#include "util.h"
code_string(BDKHash,
        u16 BKDHash(const char *str) {
            u16 seed = 131;
            u16 hash = 0;
            while(*str) {
                hash = hash * seed + (*str++);
            }
            return hash;
        }
        );
