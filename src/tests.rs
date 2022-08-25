// a direct port of the C++ tests in tests.cxx

mod match_ {
    use crate::regex::{match_, Cache};

    #[test]
    fn simple() {
        let mut cache = Cache::new();
        assert_eq!(match_(&mut cache, "[abc]", "b"), Ok(Some(vec![])));
    }

    #[test]
    fn ababc() {
        let mut cache = Cache::new();
        assert_eq!(match_(&mut cache, "ab", "abc"), Ok(None));
    }

    #[test]
    fn abcabc() {
        let mut cache = Cache::new();
        assert_eq!(match_(&mut cache, "abc", "abc"), Ok(Some(vec![])));
    }

    #[test]
    fn abcabc_grouped() {
        let mut cache = Cache::new();
        assert_eq!(
            match_(&mut cache, "a(b)(c)", "abc"),
            Ok(Some(vec!["b".to_string(), "c".to_string()]))
        );
    }

    #[test]
    fn foo_fancy() {
        let mut cache = Cache::new();
        assert_eq!(
            match_(
                &mut cache,
                "[[:space:]]+([[:upper:]]+)[[:space:]]+",
                "  FOO   "
            ),
            Ok(Some(vec!["FOO".to_string()]))
        );
    }
}

mod split {
    use crate::regex::{split, Cache, SplitItem};

    #[test]
    fn abc_simple() {
        let mut cache = Cache::new();
        assert_eq!(
            split(&mut cache, "ab", "abc"),
            Ok(vec![
                SplitItem::Literal("".to_string()),
                SplitItem::Matches(vec![]),
                SplitItem::Literal("c".to_string()),
            ])
        );
    }

    #[test]
    fn abc_grouped_a() {
        let mut cache = Cache::new();
        assert_eq!(
            split(&mut cache, "(a)b", "abc"),
            Ok(vec![
                SplitItem::Literal("".to_string()),
                SplitItem::Matches(vec!["a".to_string()]),
                SplitItem::Literal("c".to_string()),
            ])
        );
    }

    #[test]
    fn abc_grouped2_ac() {
        let mut cache = Cache::new();
        assert_eq!(
            split(&mut cache, "([ac])", "abc"),
            Ok(vec![
                SplitItem::Literal("".to_string()),
                SplitItem::Matches(vec!["a".to_string()]),
                SplitItem::Literal("b".to_string()),
                SplitItem::Matches(vec!["c".to_string()]),
                SplitItem::Literal("".to_string()),
            ])
        );
    }

    #[test]
    fn abc_or_grp_ac() {
        let mut cache = Cache::new();
        assert_eq!(
            split(&mut cache, "(a)|(c)", "abc"),
            Ok(vec![
                SplitItem::Literal("".to_string()),
                SplitItem::Matches(vec!["a".to_string(), "".to_string()]),
                SplitItem::Literal("b".to_string()),
                SplitItem::Matches(vec!["".to_string(), "c".to_string()]),
                SplitItem::Literal("".to_string()),
            ])
        );
    }

    #[test]
    fn foo_upper() {
        let mut cache = Cache::new();
        assert_eq!(
            split(&mut cache, "([[:upper:]]+)", " FOO "),
            Ok(vec![
                SplitItem::Literal(" ".to_string()),
                SplitItem::Matches(vec!["FOO".to_string()]),
                SplitItem::Literal(" ".to_string()),
            ])
        );
    }
}
