#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int n_customers = 0;
int n_resources = 0;

int *available;
int **maximum;
int **allocation;
int **need;

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