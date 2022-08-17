#include <list>
#include <mutex>
#include <new>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include "nix-builtin-utils.h"

struct NbuRegexCache {
    // TODO(nix) use C++20 transparent comparison when available
    std::mutex mutex;
    std::unordered_map<std::string_view, std::regex> cache;
    std::list<std::string> keys;

    std::regex get(std::string_view re)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        auto it = cache.find(re);
        if (it != cache.end())
            return it->second;
        keys.emplace_back(re);
        return cache.emplace(keys.back(), std::regex(keys.back(), std::regex::extended)).first->second;
    }
};

struct NbuRegexCache * nbu_create_regex_cache(void)
{
    return new NbuRegexCache();
}

void nbu_destroy_regex_cache(struct NbuRegexCache * x)
{
    delete x;
}

// in an optimal world, we would borrow from the source string, but uhh we can't...
static NbuStringSlice nbu_string_to_slice(const std::string s) {
    NbuStringSlice tmp = nbu_construct_slice(std::string_view(s));
    if (!tmp.len) {
        // ensures that the `dat` pointer is non-null
        tmp.dat = "";
    }
    nbu_make_slice_owned(&tmp);
    return tmp;
}

static int nbu_handle_matches(NbuList * const ret, const std::cmatch match) {
    // the first match is the whole string, skip it
    const size_t len = match.size() - 1;
    nbu_init_list(ret, len);

    if (!ret->dat) {
        return NBU_MATCH_ERR_NOMEM;
    }

    for (size_t i = 0; i < len; ++i) {
        const auto m = match[i+1];
        NbuStringSlice tmp;
        if (m.matched) {
            // in an optimal world, we would borrow from the source string, but we don't...
            tmp = nbu_string_to_slice(m.str());
        } else {
            tmp.dat = 0;
            tmp.len = 0;
        }
        ret->dat[i] = tmp;
    }

    return NBU_MATCH_OK;
}

int nbu_match(NbuRegexCache * cache, NbuList * ret, const NbuStringSlice rgx_, const NbuStringSlice match_)
{
    const auto rgx = nbu_destruct_slice(rgx_);
    const auto str = nbu_destruct_slice(match_);

    try {
        auto rgx_x = cache->get(rgx);
        nbu_fini_list(ret, 1);

        std::cmatch match;
        if (!std::regex_match(str.begin(), str.end(), match, rgx_x)) {
            return NBU_MATCH_NOMATCH;
        }

        return nbu_handle_matches(ret, match);
    } catch (std::regex_error & e) {
        if (e.code() == std::regex_constants::error_space) {
            // limit is _GLIBCXX_REGEX_STATE_LIMIT for libstdc++
            return NBU_MATCH_ERR_NOMEM;
        } else {
            return NBU_MATCH_ERR_INVALID;
        }
    } catch (std::bad_alloc & e) {
        return NBU_MATCH_ERR_NOMEM;
    }
}

int nbu_split(NbuRegexCache * cache, NbuList * ret, NbuStringSlice rgx_, NbuStringSlice str_)
{
    const auto rgx = nbu_destruct_slice(rgx_);
    const auto str = nbu_destruct_slice(str_);

    try {
        auto rgx_x = cache->get(rgx);

        auto begin = std::cregex_iterator(str.begin(), str.end(), rgx_x);
        auto end = std::cregex_iterator();

        // Any matches results are surrounded by non-matching results.
        const size_t len = std::distance(begin, end);
        nbu_split_fini_ret(ret);
        nbu_init_list(ret, 2 * len + 1);
        if (!ret->dat) {
            return NBU_MATCH_ERR_NOMEM;
        }
        size_t idx = 0;

        if (!len) {
            ret->dat[idx++] = str_;
            return NBU_MATCH_NOMATCH;
        }

        for (auto i = begin; i != end; ++i) {
            // assert(idx <= 2 * len + 1 - 3);
            auto match = *i;

            // Add a string for non-matched characters.
            ret->dat[idx++] = nbu_string_to_slice(match.prefix().str());

            // Add a list for matched substrings
            switch (const int sret = nbu_handle_matches(&((NbuList *)(ret->dat))[idx++], match)) {
                case NBU_MATCH_OK:
                    break;
                default:
                    return sret;
            }

            if (idx == 2 * len)
                ret->dat[idx++] = nbu_string_to_slice(match.suffix().str());
        }

        return NBU_MATCH_OK;
    } catch (std::regex_error & e) {
        if (e.code() == std::regex_constants::error_space) {
            // limit is _GLIBCXX_REGEX_STATE_LIMIT for libstdc++
            return NBU_MATCH_ERR_NOMEM;
        } else {
            return NBU_MATCH_ERR_INVALID;
        }
    } catch (std::bad_alloc & e) {
        return NBU_MATCH_ERR_NOMEM;
    }
}
