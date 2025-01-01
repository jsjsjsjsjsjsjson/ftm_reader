#include <stdio.h>
#include <stdlib.h>
#include "ftm_file.h"

char *ftm_file_name = "5_pancake.ftm";
FTM_FILE ftm;

int main() {
    ftm.open_ftm(ftm_file_name);
    ftm.read_ftm_all();
    return 0;
}