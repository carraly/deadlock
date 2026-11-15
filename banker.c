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

void validate_commands(const char *path) {
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
    if (count == 0) perror("commands.txt is empty");

    free(vals);
    fclose(f);
}

void print_vec(FILE *out, const int *v) {
    for (int j = 0; j < n_resources; j++) {
        fprintf(out, j ? " %d" : "%d", v[j]);
    }
}

// Retorna 1 se existir alguma posição j com a[j] > b[j]
int any_greater(const int *a, const int *b) {
    for(int j = 0; j < n_resources; j++) {
        if (a[j] > b[j]) {
            return 1;
        }
    }
    return 0;
}

int is_safe(void) {
    int *work = alloc_vec(n_resources);
    int *finish = alloc_vec(n_customers);

    for(int j = 0; j < n_resources; j++) {
        work[j] = available[j];
    }
    int progress = 1;
    while(progress) {
        progress = 0;
        for(int i = 0; i < n_customers; i++) {
            if(!finish[i] && !any_greater(need[i], work)) {
                for(int j = 0; j < n_resources; j++) {
                    work[j] += allocation[i][j];
                }
                finish[i] = 1;
                progress = 1;
            }
        }
    }

    int safe = 1;
    for(int i = 0; i < n_customers; i++) {
        if (!finish[i]) safe = 0;
    }
    free(work);
    free(finish);
    return safe;
}

void request_resources(int i, int *req, FILE *out) {
    if(any_greater(req, need[i])) {
        fprintf(out, "The customer %d request ", i);
        print_vec(out, req);
        fprintf(out, " was denied because exceed its maximum allocation\n");
        return;
    }

    if(any_greater(req, available)) {
        fprintf(out, "The resources ");
        print_vec(out, available);
        fprintf(out, " are not enough to customer %d request ", i);
        print_vec(out, req);
        fprintf(out, " \n");
        return;
    }

    for(int j = 0; j < n_resources; j++) {
        available[j]     -= req[j];
        allocation[i][j] += req[j];
        need[i][j]       -= req[j];
    }

    if(is_safe()) {
        fprintf(out, "Allocate to customer %d the resources ", i);
        print_vec(out, req);
        fprintf(out, "\n");
    }else {
        for(int j = 0; j < n_resources; j++) {
            available[j]     += req[j];
            allocation[i][j] -= req[j];
            need[i][j]       += req[j];
        }
        fprintf(out, "The customer %d request ", i);
        print_vec(out, req);
        fprintf(out, " was denied because result in an unsafe state\n");
    }
}

void release_resources(int i, int *rel, FILE *out) {
    if(any_greater(rel, allocation[i])) {
        fprintf(out, "The customer %d release ", i);
        print_vec(out, rel);
        fprintf(out, " was denied because exceed its allocation\n");
        return;
    }

    for(int j = 0; j < n_resources; j++) {
        available[j]     += rel[j];
        allocation[i][j] -= rel[j];
        need[i][j]       += rel[j];
    }

    fprintf(out, "Release from customer %d the resources ", i);
    print_vec(out, rel);
    fprintf(out, "\n");
}

int vec_to_str(char *buf, size_t cap, const int *v) {
    int pos = 0;
    buf[0] = '\0';
    for(int j = 0; j < n_resources; j++) {
        pos += snprintf(buf + pos, cap - pos, "%d ", v[j]);
    }
    return pos;
}

static void print_state(FILE *out) {
    size_t cap = (size_t)n_resources * 12 + 1;
    char *a = malloc(cap);
    char *b = malloc(cap);
    char *c = malloc(cap);
    if (!a || !b || !c) {
        perror("failed allocating memory");
    }

    int w1 = 10, w2 = 12;
    for(int i = 0; i < n_customers; i++) {
        int l1 = vec_to_str(a, cap, maximum[i]) + 4;
        int l2 = vec_to_str(b, cap, allocation[i]) + 6;
        if (l1 > w1) {
            w1 = l1;
        }
        if (l2 > w2) {
            w2 = l2;
        }
    }

    fprintf(out, "%-*s|   %-*s|   NEED\n", w1, "MAXIMUM", w2, "ALLOCATION");
    for(int i = 0; i < n_customers; i++) {
        vec_to_str(a, cap, maximum[i]);
        vec_to_str(b, cap, allocation[i]);
        vec_to_str(c, cap, need[i]);
        fprintf(out, "%-*s|   %-*s|   %s\n", w1, a, w2, b, c);
    }

    vec_to_str(a, cap, available);
    fprintf(out, "AVAILABLE %s\n", a);

    free(a);
    free(b);
    free(c);
}

void run_commands(const char *path, FILE *out) {
    FILE *f = fopen(path, "r");
    if(!f) {
        perror("unable to open commands.txt");
    }

    char line[1024];
    int *vals = alloc_vec(n_resources);
    char op;
    int cust;

    while(fgets(line, sizeof line, f)) {
        if(parse_command(line, &op, &cust, vals) != 0)
            perror("commands.txt has invalid format");

        if(op == 'Q') {
            request_resources(cust, vals, out);
        }else if (op == 'L') {
            release_resources(cust, vals, out);
        }else{
            print_state(out);
        }
    }

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
