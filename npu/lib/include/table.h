#ifndef NPU_LIB_TABLE_H
#define NPU_LIB_TABLE_H

#define TBL_KEY_SZ 32
#define TBL_RES_SZ 1024

#include <stdint.h>

struct search_key {
    uint8_t table_nr;
    uint8_t key[TBL_KEY_SZ];
};

struct search_result {
    uint8_t not_found;
    uint8_t res_entry[TBL_RES_SZ];
};

#endif //NPU_LIB_TABLE_H
