#include "nix-builtin-utils.h"
#include <stdlib.h>

void nbu_init_list(NbuList *self, size_t len)
{
    self->len = len;
    if (len) {
        self->dat = (NbuStringSlice*) calloc(len, sizeof(NbuStringSlice));
    } else {
        self->dat = 0;
    }
}

void nbu_fini_list(NbuList *self)
{
    if(self->dat) {
        free(self->dat);
        self->dat = 0;
    }
}

/*
void nbu_init_list2(NbuList2 *self, size_t len)
{
    self->len = len;
    if (len) {
        self->dat = (NbuList*) calloc(len, sizeof(NbuList));
    } else {
        self->dat = 0;
    }
}

void nbu_fini_list2(NbuList2 *self)
{
    if(self->dat) {
        free(self->dat);
        self->dat = 0;
    }
}
*/
