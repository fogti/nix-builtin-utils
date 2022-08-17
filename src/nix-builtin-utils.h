#pragma once
#include <stdbool.h>
#include <stddef.h>

/** Nix string slice, equivalent to rust's &[u8] **/
/** see also: https://cheats.rs/#pointer-meta **/
typedef struct {
    const char * dat;
    size_t len;
} NbuStringSlice;

typedef struct {
    NbuStringSlice * dat;
    size_t len;
} NbuList;

struct NbuRegexCache;

#ifdef __cplusplus
extern "C" {
#endif

/** slice stuff **/
bool nbu_make_slice_owned(NbuStringSlice * self);
void nbu_fini_slice_owned(NbuStringSlice * self);
bool nbu_slices_eq(NbuStringSlice a, NbuStringSlice b);

/** list stuff **/
void nbu_init_list(NbuList *self, size_t len);
void nbu_fini_list(NbuList *self, size_t recurse);

/** regex cache **/
struct NbuRegexCache * nbu_create_regex_cache(void);
void nbu_destroy_regex_cache(struct NbuRegexCache *);

/** regex builtins **/
#define NBU_MATCH_OK            0
#define NBU_MATCH_NOMATCH       1
#define NBU_MATCH_ERR_NOMEM   -12
#define NBU_MATCH_ERR_INVALID -22
int nbu_match(struct NbuRegexCache * cache, NbuList * ret, NbuStringSlice rgx, NbuStringSlice s);
/** for example usage, take a look at tests.cxx in the sources **/

/** builds a list composed of non matched strings interleaved with the lists of the POSIX ERE's **/
int nbu_split(struct NbuRegexCache * cache, NbuList * ret, NbuStringSlice rgx, NbuStringSlice s);
/** frees the partial nested list returned by nbu_split **/
void nbu_split_fini_ret(NbuList * ret);

#ifdef __cplusplus
}

/** C++ API for slices **/

#include <string_view>

inline NbuStringSlice nbu_construct_slice(std::string_view s) {
    NbuStringSlice ret;
    ret.dat = s.data();
    ret.len = s.size();
    return ret;
}

inline std::string_view nbu_destruct_slice(NbuStringSlice s) {
    return std::string_view(s.dat, s.len);
}

#endif
