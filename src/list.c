#include "nix-builtin-utils.h"
#include <stdlib.h>
#include <string.h>

bool nbu_make_slice_owned(NbuStringSlice * self)
{
    if (self->dat) {
        char * tmp = malloc(self->len + 1);
        if (!tmp) {
            return false;
        }
        memcpy(tmp, self->dat, self->len);
        tmp[self->len + 1] = 0;
        self->dat = tmp;
    }
    return true;
}

void nbu_fini_slice_owned(NbuStringSlice * self)
{
    nbu_fini_list((NbuList *) self, 0);
}

void nbu_init_list(NbuList * self, size_t len)
{
    self->len = len;
    if (len) {
        self->dat = (NbuStringSlice*) calloc(len, sizeof(NbuStringSlice));
    } else {
        self->dat = 0;
    }
}

void nbu_fini_list(NbuList * self, size_t recurse)
{
    if (self->dat) {
        if (recurse) {
            NbuList * elems = (NbuList *) self->dat;
            for (size_t i = 0; i < self->len; ++i) {
                nbu_fini_list(&elems[i], recurse - 1);
            }
        }

        if (self->len) {
            free(self->dat);
        }
        self->dat = 0;
    }
    self->len = 0;
}

void nbu_split_fini_ret(NbuList * ret)
{
    NbuList * elems = (NbuList *) ret->dat;
    for (size_t j = 0; j < ret->len; ++j) {
        nbu_fini_list(&elems[j], j % 2);
    }
    nbu_fini_list(ret, 0);
}
