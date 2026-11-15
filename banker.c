#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int n_customers = 0;
int n_resources = 0;

int *available;
int **maximum;
int **allocation;
int **need;

static int *alloc_vec(int m) {
    int *v = calloc(m, sizeof(int));
    if(!v) {
        perror("failed allocating memory");
    }
    return v;
}

void parse_args(int argc, char *argv[]) {
    if(argc < 2) {
        perror("expected at least 3 arguments");
    }
    
    n_resources = argc - 1;
    available = alloc_vec(n_resources);

    for(int j = 0; j < n_resources; j++) {
        char *end;
        long v = strtol(argv[j + 1], &end, 10);

        /* vazio, lixo no final (ex.: "5x"), overflow, negativo */
        if (end == argv[j + 1] || *end != '\0')
            perror("expected a number");
        if (v < 0)
            perror("expected a non-negative number");

        available[j] = (int)v;
    }
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
