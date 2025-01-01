#ifndef FTM_FILE_H
#define FTM_FILE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

typedef struct __attribute__((packed)) {
    char id[16] = "INFO";
    uint32_t version = 1;
    uint32_t size = 96;
    char title[32] = "TITLE";
    char author[32] = "AUTHOR";
    char copyright[32] = "COPYRIGHT";
} INFO_BLOCK;

typedef struct __attribute__((packed)) {
    char id[16] = "HEADER";
    uint32_t version = 3;
    uint32_t size;
    uint8_t track_num = 1;
    char name[64];
    uint8_t ch_id[8];
    uint8_t ch_fx[8];
} HEADER_BLOCK;

typedef struct __attribute__((packed)) {
    char id[16] = "INSTRUMENTS";
    uint32_t version = 6;
    uint32_t size;
    uint32_t inst_num;
} INSTRUMENT_BLOCK;

typedef struct __attribute__((packed)) {
    uint8_t index;
    uint8_t loop : 4;
    uint8_t pitch : 4;
    uint8_t d_counte;
} dpcm_t;

typedef struct __attribute__((packed)) {
    uint32_t index;
    uint8_t type;
    uint32_t seq_count;
    dpcm_t dpcm[96];
    uint32_t name_len;
    char name[128];
} instrument_t;

class FTM_FILE {
public:
    FILE *ftm_file;

    FTM_HEADER header;
    PARAMS_BLOCK pr_block;
    INFO_BLOCK nf_block;
    INSTRUMENT_BLOCK inst_block;
    HEADER_BLOCK he_block;

    void open_ftm(const char *filename) {
        ftm_file = fopen(filename, "rb+");
    }

    void read_ftm_metadata() {
        fseek(ftm_file, 0, SEEK_SET);
        fread(&header, sizeof(header), 1, ftm_file);
        if (header.version != 0x0440) {
            printf("Only FTM file version 0x0440 is supported\nVERSION: 0x%X\n", header.version);
            memset(&header, 0, sizeof(header));
            fseek(ftm_file, 0, SEEK_SET);
            return;
        }
        fread(&pr_block, 1, sizeof(pr_block), ftm_file);
        fread(&nf_block, 1, sizeof(nf_block), ftm_file);
        fread(&he_block, 1, 25, ftm_file);

        fread(&he_block.name, 1, he_block.size - 11, ftm_file);

        for (int i = 0; i < pr_block.channel; i++) {
            fread(&he_block.ch_id[i], 1, 1, ftm_file);
            fread(&he_block.ch_fx[i], 1, 1, ftm_file);
        }
        
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
        printf("VERSION: %d\n", nf_block.version);
        printf("SIZE: %d\n", nf_block.size);
        printf("TITLE: %s\n", nf_block.title);
        printf("AUTHOR: %s\n", nf_block.author);
        printf("COPYRIGHT: %s\n", nf_block.copyright);

        printf("\nMETADATA HEADER: %s\n", he_block.id);
        printf("VERSION: %d\n", he_block.version);
        printf("SIZE: %d\n", he_block.size);
        printf("TRACK: %d\n", he_block.track_num + 1);
        printf("NAME: %s\n", he_block.name);
        for (uint8_t i = 0; i < pr_block.channel; i++) {
            printf("CH%d: EX_FX*%d  ", he_block.ch_id[i], he_block.ch_fx[i]);
        }
        printf("\n");
    }

    void read_ftm_instrument() {
        fread(&inst_block, 1, sizeof(inst_block), ftm_file);
        printf("\nINSTRUMENT HEADER: %s\n", inst_block.id);
        printf("VERSION: %d\n", inst_block.version);
        printf("SIZE: %d\n", inst_block.size);
        printf("NUM_INST: %X\n", inst_block.inst_num);
    }
};

#endif