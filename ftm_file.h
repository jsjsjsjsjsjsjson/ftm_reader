#ifndef FTM_FILE_H
#define FTM_FILE_H

#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    char id[18];
    uint16_t version = 0x0440;
    uint16_t recv = 0;
} FTM_HEADER;

typedef struct __attribute__((packed)) {
    char id[16] = "PARAMS";
    uint32_t version = 0x00000006;
    uint32_t size = 0x0000001D;
    uint8_t ext_chip = 0x00;
    uint32_t channel = 0x00000005;
    uint32_t machine = 0;
    uint32_t e_speed = 0;
    uint32_t v_style = 1;
    uint32_t hl1 = 4;
    uint32_t hl2 = 16;
    uint32_t s_split = 0x15; // ?
} PARAMS_BLOCK;

typedef struct {
    char id[16] = "INFO";
    uint32_t version = 1;
    uint32_t size = 96;
    char title[32] = "TITLE";
    char author[32] = "AUTHOR";
    char copyright[32] = "COPYRIGHT";
} INFO_BLOCK;

void read_ftm(FILE *ftm_file) {
    FTM_HEADER header;
    PARAMS_BLOCK pr_block;
    INFO_BLOCK nf_block;
    fseek(ftm_file, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, ftm_file);
    if (header.version != 0x0440) {
        printf("Only FTM file version 0x0440 is supported\nVERSION: 0x%X\n", header.version);
    }
    fread(&pr_block, sizeof(pr_block), 1, ftm_file);
    fread(&nf_block, sizeof(nf_block), 1, ftm_file);
    printf("\nHEADER: %.18s\n", header.id);
    printf("VERSION: 0x%X\n", header.version);
    printf("\nPARAMS HEADER: %s\n", pr_block.id);
    printf("VERSION: %d\n", pr_block.version);
    printf("SIZE: %d\n", pr_block.size);
    printf("EXTRA_CHIP: 0x%X\n", pr_block.ext_chip);
    printf("CHANNELS: %d\n", pr_block.channel);
    printf("MACHINE: 0x%X\n", pr_block.machine);
    printf("E_SPEED: %d\n", pr_block.e_speed);
    printf("V_STYLE: %d\n", pr_block.v_style);
    printf("HIGHLINE1: %d\n", pr_block.hl1);
    printf("HIGHLINE2: %d\n", pr_block.hl2);
    printf("\nINFO HEADER: %s\n", nf_block.id);
    printf("VERSION: 0x%X\n", nf_block.version);
    printf("SIZE: %d\n", nf_block.size);
    printf("TITLE: %s\n", nf_block.title);
    printf("AUTHOR: %s\n", nf_block.author);
    printf("COPYRIGHT: %s\n", nf_block.copyright);
}

#endif