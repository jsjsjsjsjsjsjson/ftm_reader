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
    char id[16] = "SEQUENCES";
    uint32_t version = 6;
    uint32_t size;
    uint32_t sequ_num;
} SEQUENCES_BLOCK;


typedef struct __attribute__((packed)) {
    uint8_t index;
    uint8_t loop : 4;
    uint8_t pitch : 4;
    uint8_t d_counte;
} dpcm_t;

typedef struct {
    bool enable;
    uint8_t seq_index;
} seq_index_t;

typedef struct __attribute__((packed)) {
    uint32_t index;
    uint8_t type;
    uint32_t seq_count;
    seq_index_t seq_index[5];
    dpcm_t dpcm[96];
    uint32_t name_len;
    char name[64];
} instrument_t;

typedef struct __attribute__((packed)) {
    uint32_t index;
    uint32_t type;
    uint8_t length;
    uint32_t loop;
    int8_t data[256];
    uint32_t release;
    uint32_t setting;
} sequences_t;

class FTM_FILE {
public:
    FILE *ftm_file;

    FTM_HEADER header;
    PARAMS_BLOCK pr_block;
    INFO_BLOCK nf_block;
    HEADER_BLOCK he_block;
    INSTRUMENT_BLOCK inst_block;
    SEQUENCES_BLOCK seq_block;

    instrument_t *instrument;
    sequences_t *sequences;

    int open_ftm(const char *filename) {
        ftm_file = fopen(filename, "rb+");
        fread(&header, sizeof(header), 1, ftm_file);
        if (strncmp(header.id, "FamiTracker Module", 18)) {
            printf("HEADER: %.16s\n", header.id);
            printf("This is not a valid FTM file!\n");
            memset(&header, 0, sizeof(header));
            fclose(ftm_file);
            return -1;
        };
        if (header.version != 0x0440) {
            printf("Only FTM file version 0x0440 is supported\nVERSION: 0x%X\n", header.version);
            memset(&header, 0, sizeof(header));
            fclose(ftm_file);
            return -2;
        }
        printf("\nOpen %.18s\n", header.id);
        printf("VERSION: 0x%X\n", header.version);
        return 0;
    }

    void read_param_block() {
        fread(&pr_block, 1, sizeof(pr_block), ftm_file);

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
    }

    void read_info_block() {
        fread(&nf_block, 1, sizeof(nf_block), ftm_file);

        printf("\nINFO HEADER: %s\n", nf_block.id);
        printf("VERSION: %d\n", nf_block.version);
        printf("SIZE: %d\n", nf_block.size);
        printf("TITLE: %s\n", nf_block.title);
        printf("AUTHOR: %s\n", nf_block.author);
        printf("COPYRIGHT: %s\n", nf_block.copyright);
    }

    void read_header_block() {
        fread(&he_block, 1, 25, ftm_file);

        fread(&he_block.name, 1, he_block.size - (pr_block.channel * 2) - 1, ftm_file);

        for (int i = 0; i < pr_block.channel; i++) {
            fread(&he_block.ch_id[i], 1, 1, ftm_file);
            fread(&he_block.ch_fx[i], 1, 1, ftm_file);
        }

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

    void read_instrument_block() {
        fread(&inst_block, 1, sizeof(inst_block), ftm_file);
        printf("\nINSTRUMENT HEADER: %s\n", inst_block.id);
        printf("VERSION: %d\n", inst_block.version);
        printf("SIZE: %d\n", inst_block.size);
        printf("NUM_INST: 0x%X\n", inst_block.inst_num);
    }

    void read_instrument_data() {
        printf("ALLOC MEM...\n");
        instrument = new instrument_t[inst_block.inst_num];
        printf("SIZE: %gKB\n", (sizeof(instrument_t) * inst_block.inst_num) / 1024.0f);
        printf("\nREADING INSTRUMENT, COUNT=%d\n", inst_block.inst_num);
        for (int i = 0; i < inst_block.inst_num; i++) {
            printf("\nREADING INSTRUMENT #%02X...\n", i);
            fread(&instrument[i], sizeof(instrument_t) - 64, 1, ftm_file);
            fread(&instrument[i].name, instrument[i].name_len, 1, ftm_file);
            printf("%02X->NAME: %s\n", instrument[i].index, instrument[i].name);
            printf("TYPE: 0x%X\n", instrument[i].type);
            printf("SEQU COUNT: %d\n", instrument[i].seq_count);
            printf("SEQU DATA: ");
            printf("VOL: %d %d\n", instrument[i].seq_index[0].enable, instrument[i].seq_index[0].seq_index);
            printf("ARP: %d %d\n", instrument[i].seq_index[1].enable, instrument[i].seq_index[1].seq_index);
            printf("PIT: %d %d\n", instrument[i].seq_index[2].enable, instrument[i].seq_index[2].seq_index);
            printf("HIP: %d %d\n", instrument[i].seq_index[3].enable, instrument[i].seq_index[3].seq_index);
            printf("DTY: %d %d\n", instrument[i].seq_index[4].enable, instrument[i].seq_index[4].seq_index);
            printf("\n");
        }
    }

    void read_sequences_block() {
        fread(&seq_block, 1, sizeof(seq_block), ftm_file);
        printf("\nSEQUENCES HEADER: %s\n", seq_block.id);
        printf("VERSION: %d\n", seq_block.version);
        printf("SIZE: %d\n", seq_block.size);
        printf("NUM_SEQU: %d\n", seq_block.sequ_num);
    }

    void read_sequences_data() {
        printf("ALLOC MEM...\n");
        sequences = new sequences_t[seq_block.sequ_num];
        printf("SIZE: %gKB\n", (sizeof(sequences_t) * seq_block.sequ_num) / 1024.0f);
        printf("\nREADING SEQUENCES, COUNT=%d\n", seq_block.sequ_num);
        for (int i = 0; i < seq_block.sequ_num; i++) {
            printf("\nREADING SEQUENCES #%d...\n", i);
            fread(&sequences[i], sizeof(sequences_t) - (256 + 8), 1, ftm_file);
            fread(&sequences[i].data, sequences[i].length, 1, ftm_file);
            printf("#%d\n", sequences[i].index);
            printf("TYPE: 0x%X\n", sequences[i].type);
            printf("LENGTH: %d\n", sequences[i].length);
            printf("LOOP: %d\n", sequences[i].loop);
            printf("DATA:\n");
            for (int j = 0; j < sequences[i].length; j++) {
                printf("%d ", sequences[i].data[j]);
            }
            printf("\n");
        }
        printf("\nREADING SEQU SETTING...\n");
        for (int i = 0; i < seq_block.sequ_num; i++) {
            printf("\n#%d\n", i);
            fread(&sequences[i].release, sizeof(uint32_t), 1, ftm_file);
            printf("RELEASE: %d\n", sequences[i].release);
            fread(&sequences[i].setting, sizeof(uint32_t), 1, ftm_file);
            printf("SETTING: 0x%X\n", sequences[i].setting);
        }
    }

    void read_ftm_all() {
        read_param_block();
        read_info_block();
        read_header_block();

        read_instrument_block();
        read_instrument_data();

        read_sequences_block();
        read_sequences_data();
    }
};

#endif