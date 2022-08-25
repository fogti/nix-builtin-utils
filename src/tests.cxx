#include "nix-builtin-utils-2.h"
#include <gtest/gtest.h>

// most test cases were taken from the Nix documentation

TEST(match, simple) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_match(cache, &matches, nbu_construct_slice("[abc]"), nbu_construct_slice("b"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 0);

    nbu_fini_list(&matches, 1);
    nbu_destroy_regex_cache(cache);
}

TEST(match, ababc) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_match(cache, &matches, nbu_construct_slice("ab"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_NOMATCH);
    ASSERT_EQ(matches.len, 0);

    nbu_fini_list(&matches, 1);
    nbu_destroy_regex_cache(cache);
}

TEST(match, abcabc) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_match(cache, &matches, nbu_construct_slice("abc"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 0);

    nbu_fini_list(&matches, 1);
    nbu_destroy_regex_cache(cache);
}

TEST(match, abcabc_grouped) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_match(cache, &matches, nbu_construct_slice("a(b)(c)"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 2);
    ASSERT_TRUE(nbu_slices_eq(matches.dat[0], nbu_construct_slice("b")));
    ASSERT_TRUE(nbu_slices_eq(matches.dat[1], nbu_construct_slice("c")));

    nbu_fini_list(&matches, 1);
    nbu_destroy_regex_cache(cache);
}

TEST(match, foo_fancy) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_match(cache, &matches, nbu_construct_slice("[[:space:]]+([[:upper:]]+)[[:space:]]+"), nbu_construct_slice("  FOO   "));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 1);
    ASSERT_TRUE(nbu_slices_eq(matches.dat[0], nbu_construct_slice("FOO")));

    nbu_fini_list(&matches, 1);
    nbu_destroy_regex_cache(cache);
}

TEST(split, abc_simple) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_split(cache, &matches, nbu_construct_slice("ab"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 3);
    ASSERT_TRUE(nbu_slices_eq(matches.dat[0], nbu_construct_slice("")));
    {
        NbuList sm = ((NbuList *) matches.dat)[1];
        ASSERT_EQ(sm.len, 0);
    }
    ASSERT_TRUE(nbu_slices_eq(matches.dat[2], nbu_construct_slice("c")));

    nbu_split_fini_ret(&matches);
    nbu_destroy_regex_cache(cache);
}

TEST(split, abc_grouped_a) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_split(cache, &matches, nbu_construct_slice("(a)b"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 3);
    ASSERT_TRUE(nbu_slices_eq(matches.dat[0], nbu_construct_slice("")));
    {
        NbuList sm = ((NbuList *) matches.dat)[1];
        ASSERT_EQ(sm.len, 1);
        ASSERT_TRUE(nbu_slices_eq(sm.dat[0], nbu_construct_slice("a")));
    }
    ASSERT_TRUE(nbu_slices_eq(matches.dat[2], nbu_construct_slice("c")));

    nbu_split_fini_ret(&matches);
    nbu_destroy_regex_cache(cache);
}

TEST(split, abc_grouped2_ac) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_split(cache, &matches, nbu_construct_slice("([ac])"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 5);
    ASSERT_TRUE(nbu_slices_eq(matches.dat[0], nbu_construct_slice("")));
    {
        NbuList sm = ((NbuList *) matches.dat)[1];
        ASSERT_EQ(sm.len, 1);
        ASSERT_TRUE(nbu_slices_eq(sm.dat[0], nbu_construct_slice("a")));
    }
    ASSERT_TRUE(nbu_slices_eq(matches.dat[2], nbu_construct_slice("b")));
    {
        NbuList sm = ((NbuList *) matches.dat)[3];
        ASSERT_EQ(sm.len, 1);
        ASSERT_TRUE(nbu_slices_eq(sm.dat[0], nbu_construct_slice("c")));
    }
    ASSERT_TRUE(nbu_slices_eq(matches.dat[4], nbu_construct_slice("")));

    nbu_split_fini_ret(&matches);
    nbu_destroy_regex_cache(cache);
}

TEST(split, abc_or_grp_ac) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_split(cache, &matches, nbu_construct_slice("(a)|(c)"), nbu_construct_slice("abc"));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 5);
    ASSERT_TRUE(nbu_slices_eq(matches.dat[0], nbu_construct_slice("")));
    {
        NbuList sm = ((NbuList *) matches.dat)[1];
        ASSERT_EQ(sm.len, 2);
        ASSERT_TRUE(nbu_slices_eq(sm.dat[0], nbu_construct_slice("a")));
        ASSERT_EQ(sm.dat[1].dat, (const char*)0);
        ASSERT_EQ(sm.dat[1].len, 0);
    }
    ASSERT_TRUE(nbu_slices_eq(matches.dat[2], nbu_construct_slice("b")));
    {
        NbuList sm = ((NbuList *) matches.dat)[3];
        ASSERT_EQ(sm.len, 2);
        ASSERT_EQ(sm.dat[0].dat, (const char*)0);
        ASSERT_EQ(sm.dat[0].len, 0);
        ASSERT_TRUE(nbu_slices_eq(sm.dat[1], nbu_construct_slice("c")));
    }
    ASSERT_TRUE(nbu_slices_eq(matches.dat[4], nbu_construct_slice("")));

    nbu_split_fini_ret(&matches);
    nbu_destroy_regex_cache(cache);
}

TEST(split, foo_upper) {
    NbuRegexCache * cache = nbu_create_regex_cache();
    ASSERT_TRUE(cache);

    NbuList matches;
    nbu_init_list(&matches, 0);

    const int r = nbu_split(cache, &matches, nbu_construct_slice("([[:upper:]]+)"), nbu_construct_slice(" FOO "));
    ASSERT_EQ(r, NBU_MATCH_OK);
    ASSERT_EQ(matches.len, 3);
    ASSERT_EQ(nbu_destruct_slice(matches.dat[0]), " ");
    {
        NbuList sm = ((NbuList *) matches.dat)[1];
        ASSERT_EQ(sm.len, 1);
        ASSERT_TRUE(nbu_slices_eq(sm.dat[0], nbu_construct_slice("FOO")));
    }
    ASSERT_EQ(nbu_destruct_slice(matches.dat[2]), " ");

    nbu_split_fini_ret(&matches);
    nbu_destroy_regex_cache(cache);
}
