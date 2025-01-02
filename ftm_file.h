#ifndef FTM_FILE_H
#define FTM_FILE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>

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

typedef struct {
    uint32_t index;
    uint32_t type;
    uint8_t length;
    uint32_t loop;
    std::vector<int8_t> data;
    uint32_t release;
    uint32_t setting;
} sequences_t;

typedef struct {
    sequences_t vol;
    sequences_t arp;
    sequences_t pit;
    sequences_t hip;
    sequences_t dty;
} sequ_group_t;

uint32_t find_max(uint32_t arr[], size_t n) {
    uint32_t max = arr[0];
    for (size_t i = 1; i < n; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

class FTM_FILE {
public:
    FILE *ftm_file;

    FTM_HEADER header;
    PARAMS_BLOCK pr_block;
    INFO_BLOCK nf_block;
    HEADER_BLOCK he_block;
    INSTRUMENT_BLOCK inst_block;
    SEQUENCES_BLOCK seq_block;

    uint32_t sequ_max;
    std::vector<sequ_group_t> sequences;
    std::vector<instrument_t> instrument;

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
        instrument.clear();
        printf("\nREADING INSTRUMENT, COUNT=%d\n", inst_block.inst_num);
        for (int i = 0; i < inst_block.inst_num; i++) {
            instrument_t inst_tmp;
            printf("\nREADING INSTRUMENT #%02X...\n", i);
            fread(&inst_tmp, sizeof(instrument_t) - 64, 1, ftm_file);
            fread(&inst_tmp.name, inst_tmp.name_len, 1, ftm_file);
            inst_tmp.name[inst_tmp.name_len] = '\0';
            instrument.push_back(inst_tmp);
            printf("%02X->NAME: %s\n", instrument[i].index, instrument[i].name);
            printf("TYPE: 0x%X\n", instrument[i].type);
            printf("SEQU COUNT: %d\n", instrument[i].seq_count);
            printf("SEQU DATA:\n");
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
        std::vector<sequences_t> sequ_tmp;
        sequ_tmp.resize(seq_block.sequ_num);
        uint32_t index_table[seq_block.sequ_num];
        printf("\nREADING SEQUENCES, COUNT=%d\n", seq_block.sequ_num);
        for (int i = 0; i < seq_block.sequ_num; i++) {
            printf("\nREADING SEQUENCES #%d...\n", i);
            fread(&sequ_tmp[i], 13, 1, ftm_file);
            sequ_tmp[i].data.resize(sequ_tmp[i].length);
            fread(sequ_tmp[i].data.data(), sequ_tmp[i].length, 1, ftm_file);
            index_table[i] = sequ_tmp[i].index;
            printf("#%d\n", sequ_tmp[i].index);
            printf("TYPE: 0x%X\n", sequ_tmp[i].type);
            printf("LENGTH: %d\n", sequ_tmp[i].length);
            printf("LOOP: %d\n", sequ_tmp[i].loop);
            printf("DATA:\n");
            for (int j = 0; j < sequ_tmp[i].length; j++) {
                printf("%d ", sequ_tmp[i].data[j]);
            }
            printf("\n");
        }
        printf("\nREADING SEQU SETTING...\n");
        for (int i = 0; i < seq_block.sequ_num; i++) {
            printf("\n#%d\n", i);
            fread(&sequ_tmp[i].release, sizeof(uint32_t), 1, ftm_file);
            printf("RELEASE: %d\n", sequ_tmp[i].release);
            fread(&sequ_tmp[i].setting, sizeof(uint32_t), 1, ftm_file);
            printf("SETTING: 0x%X\n", sequ_tmp[i].setting);
        }
        printf("Finishing...\n");
        sequ_max = find_max(index_table, seq_block.sequ_num);
        sequences.resize(sequ_max + 1);
        printf("SEQU_MAX: %d\n", sequ_max);
        for (int i = 0; i < seq_block.sequ_num; i++) {
            switch (sequ_tmp[i].type) {
            case 0:
                sequences[sequ_tmp[i].index].vol = sequ_tmp[i];
                break;

            case 1:
                sequences[sequ_tmp[i].index].arp = sequ_tmp[i];
                break;

            case 2:
                sequences[sequ_tmp[i].index].pit = sequ_tmp[i];
                break;

            case 3:
                sequences[sequ_tmp[i].index].hip = sequ_tmp[i];
                break;

            case 4:
                sequences[sequ_tmp[i].index].dty = sequ_tmp[i];
                break;
            
            default:
                printf("WARNING: #%d UNKNOW TYPE %d\n", i, sequ_tmp[i].type);
                getchar();
            }
        }
        printf("SECCESS.\n");
        if (getchar() == '1') {
            printf("\nVOLUME:\n");
            for (uint32_t i = 0; i < sequ_max; i++) {
                printf("#%d: %d %d %d -> ", sequences[i].vol.index, sequences[i].vol.length, sequences[i].vol.loop, sequences[i].vol.release);
                for (int j = 0; j < sequences[i].vol.length; j++) {
                    printf("%d ", sequences[i].vol.data[j]);
                }
                printf("\n");
            }
            printf("\nARPEGGIO:\n");
            for (uint32_t i = 0; i < sequ_max; i++) {
                printf("#%d: %d %d %d -> ", sequences[i].arp.index, sequences[i].arp.length, sequences[i].arp.loop, sequences[i].arp.release);
                for (int j = 0; j < sequences[i].arp.length; j++) {
                    printf("%d ", sequences[i].arp.data[j]);
                }
                printf("\n");
            }
            printf("\nPITCH:\n");
            for (uint32_t i = 0; i < sequ_max; i++) {
                printf("#%d: %d %d %d -> ", sequences[i].pit.index, sequences[i].pit.length, sequences[i].pit.loop, sequences[i].pit.release);
                for (int j = 0; j < sequences[i].pit.length; j++) {
                    printf("%d ", sequences[i].pit.data[j]);
                }
                printf("\n");
            }
            printf("\nHI-PITCH:\n");
            for (uint32_t i = 0; i < sequ_max; i++) {
                printf("#%d: %d %d %d -> ", sequences[i].hip.index, sequences[i].hip.length, sequences[i].hip.loop, sequences[i].hip.release);
                for (int j = 0; j < sequences[i].hip.length; j++) {
                    printf("%d ", sequences[i].hip.data[j]);
                }
                printf("\n");
            }
            printf("\nDUTY:\n");
            for (uint32_t i = 0; i < sequ_max; i++) {
                printf("#%d: %d %d %d -> ", sequences[i].dty.index, sequences[i].dty.length, sequences[i].dty.loop, sequences[i].dty.release);
                for (int j = 0; j < sequences[i].dty.length; j++) {
                    printf("%d ", sequences[i].dty.data[j]);
                }
                printf("\n");
            }
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