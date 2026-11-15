#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

int n_customers = 0;
int n_resources = 0;

int *available;
int **maximum;
int **allocation;
int **need;

int *alloc_vec(int m) {
    int *v = calloc(m, sizeof(int));
    if(!v) {
        perror("failed allocating memory");
    }
    return v;
}

int **alloc_mat(int n, int m) {
    int **mat = malloc(n * sizeof(int *));
    if(!mat) {
        perror("falha de alocacao de memoria");
    }
    for(int i = 0; i < n; i++) mat[i] = alloc_vec(m);
    return mat;
}

void parse_args(int argc, char *argv[]) {
    if(argc < 2) {
        perror("expected at least 3 arguments");
    }
    
    n_resources = argc - 1;
    available = alloc_vec(n_resources);

    for(int j = 0; j < n_resources; j++) {
        char *end;
        errno = 0;
        long v = strtol(argv[j + 1], &end, 10);

        if(end == argv[j + 1] || *end != '\0') {
            perror("expected a number");
        }
        if(errno == ERANGE || v > INT_MAX) {
            perror("argument is too big");
        }
        if(v < 0) {
            perror("expected a non-negative number");
        }

        available[j] = (int)v;
    }
}

int parse_customer_line(const char *line, int *out) {
    const char *p = line;

    for (int j = 0; j < n_resources; j++) {
        char *end;
        errno = 0;
        long v = strtol(p, &end, 10);
        if (end == p || errno == ERANGE || v < 0 || v > INT_MAX) {
            return -1;
        }
        out[j] = (int)v;
        p = end;

        while(*p == ' ' || *p == '\t') {
            p++;
        }
        if(j < n_resources - 1) {
            if (*p != ',') return -1;
            p++;
        }
    }

    while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        p++;
    }

    return(*p == '\0') ? 0 : -1;
}

void load_customers(const char *path) {
    FILE *f = fopen(path, "r");
    if(!f) {
        perror("unable to open customer.txt");
    }

    char line[1024];
    char msg[128];
    int *tmp = alloc_vec(n_resources);
    int count = 0;

    while (fgets(line, sizeof line, f)) {
        count++;
        if (!strchr(line, '\n') && !feof(f)) {
            snprintf(msg, sizeof msg, "customer.txt: line %d is too long", count);
            perror(msg);
        }
        if (parse_customer_line(line, tmp) != 0) {
            snprintf(msg, sizeof msg,
                     "customer.txt: line %d has invalid formatting (expected %d values separated by ',')",
                     count, n_resources);
            perror(msg);
        }
    }
    if (count == 0) perror("customer.txt is empty");

    n_customers = count;
    maximum    = alloc_mat(n_customers, n_resources);
    allocation = alloc_mat(n_customers, n_resources);
    need       = alloc_mat(n_customers, n_resources);

    rewind(f);
    for (int i = 0; i < n_customers && fgets(line, sizeof line, f); i++) {
        parse_customer_line(line, maximum[i]);
        for (int j = 0; j < n_resources; j++)
            need[i][j] = maximum[i][j];
    }

    free(tmp);
    fclose(f);
}

int parse_int(const char *s, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v < 0 || v > INT_MAX) {
        return -1;
    }
    *out = (int)v;
    return 0;
}

int parse_command(const char *line, char *op, int *cust, int *vals) {
    const char *sep = " \t\r\n";
    char buf[1024];
    snprintf(buf, sizeof buf, "%s", line);

    char *tok = strtok(buf, sep);
    if(!tok) {
        return -1;
    }

    if (strcmp(tok, "*") == 0) {
        *op = '*';
        if (strtok(NULL, sep) != NULL) {
            return -1;
        }
        return 0;
    }

    if(strcmp(tok, "RQ") == 0) {
        *op = 'Q';
    }
    else if(strcmp(tok, "RL") == 0) {
        *op = 'L';
    }
    else {
        return -1;
    }

    tok = strtok(NULL, sep);
    if(!tok || parse_int(tok, cust) != 0 || *cust >= n_customers) {
        return -1;
    }

    for(int j = 0; j < n_resources; j++) {
        tok = strtok(NULL, sep);
        if(!tok || parse_int(tok, &vals[j]) != 0) {
            return -1;
        }
    }

    if (strtok(NULL, sep) != NULL) {
        return -1;
    }
    return 0;
}

static void validate_commands(const char *path) {
    FILE *f = fopen(path, "r");
    if(!f) {
        perror("unable to open commands.txt");
    }

    char line[1024];
    char msg[160];
    int *vals = alloc_vec(n_resources);
    char op;
    int cust;
    int count = 0;

    while (fgets(line, sizeof line, f)) {
        count++;
        if (!strchr(line, '\n') && !feof(f)) {
            snprintf(msg, sizeof msg, "commands.txt: line %d is too long", count);
            perror(msg);
        }
        if (parse_command(line, &op, &cust, vals) != 0) {
            snprintf(msg, sizeof msg,
                     "commands.txt: line %d is invalid (expected: RQ|RL <cliente 0..%d> + %d values, or *)",
                     count, n_customers - 1, n_resources);
            perror(msg);
        }
    }
    if (count == 0) perror("commands.txt esta vazio");

    free(vals);
    fclose(f);
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    load_customers("customer.txt");
    validate_commands("commands.txt");

    FILE *out = fopen("result.txt", "w");
    if(!out) {
        perror("unable to create result.txt");
    }

    run_commands("commands.txt", out);

    fclose(out);
    free_all();
    return 0;
}
