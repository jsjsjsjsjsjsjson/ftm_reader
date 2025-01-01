#include <stdio.h>
#include <stdlib.h>
#include "ftm_file.h"

char *ftm_file_name = "5_pancake.ftm";

int main() {
    FILE *ftm_file = fopen(ftm_file_name, "rb+");
    read_ftm(ftm_file);
    return 0;
}