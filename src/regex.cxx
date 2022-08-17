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
        const std::lock_guard<std::mutex> lock(rgx_mutex);
        auto it = rgx_cache.find(re);
        if (it != rgx_cache.end())
            return it->second;
        keys.emplace_back(re);
        return cache.emplace(keys.back(), std::regex(keys.back(), std::regex::extended)).first->second;
    }
};

struct NbuRegexCache * nbu_create_regex_cache(void) {
    return new NbuRegexCache();
}

void nbu_destroy_regex_cache(struct NbuRegexCache * x) {
    delete x;
}

int nbu_match(NbuRegexCache * cache, NbuList * ret, const NbuStringSlice rgx_, const NbuStringSlice match_) {
    const auto rgx = nbu_destruct_slice(rgx_);
    const auto str = nbu_destruct_slice(match_);

    try {
        auto rgx_x = cache->get(rgx);
        nbu_fini_list(ret);

        std::cmatch match;
        if (!std::regex_match(str.begin(), str.end(), match, rgx_x)) {
            return NBU_MATCH_NOMATCH;
        }

        // the first match is the whole string
        const size_t len = match.size() - 1;
        nbu_init_list(ret, len);
        if (!ret->dat) {
            return NBU_MATCH_ERR_NOMEM;
        }
        for (size_t i = 0; i < len; ++i) {
            const auto m = match[i+1];
            NbuStringSlice tmp;
            if (m.matched) {
                tmp = nbu_construct_slice(m.str());
                if (!tmp.len) {
                    // ensures that the `dat` pointer is non-null
                    tmp.dat = "";
                }
            } else {
                tmp.dat = 0;
                tmp.len = 0;
            }
            ret->dat[i] = tmp;
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

/*
int nbu_split(NbuRegexCache * cache, NbuList2 * ret, NbuStringSlice rgx_, NbuStringSlice str_) {
    const auto rgx = nbu_destruct_slice(rgx_);
    const auto str = nbu_destruct_slice(str_);

    try {
        auto rgx_x = cache->get(rgx);

        auto begin = std::cregex_iterator(str.begin(), str.end(), regex);
        auto end = std::cregex_iterator();

        // Any matches results are surrounded by non-matching results.
        const size_t len = std::distance(begin, end);
        nbu_fini_list2(ret);
        nbu_init_list2(ret, 2 * len + 1);
        if (!ret->dat) {
            return NBU_MATCH_ERR_NOMEM;
        }
        size_t idx = 0;

        if (!len) {
            ret->dat[idx++] = s;
            return NBU_MATCH_NOMATCH;
        }

        for (auto i = begin; i != end; ++i) {
            // assert(idx <= 2 * len + 1 - 3);
            auto match = *i;
            
        }
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
*/
