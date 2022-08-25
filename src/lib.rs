mod ffi {
    use core::fmt;
    use std::os::raw;

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct NbuStringSlice {
        pub dat: *const raw::c_char,
        pub len: usize,
    }

    impl NbuStringSlice {
        /// SAFETY: the returned object is only valid for the lifetime of `s`.
        #[inline]
        pub unsafe fn from_bytes(s: &[u8]) -> Self {
            Self {
                dat: s.as_ptr() as *const raw::c_char,
                len: s.len(),
            }
        }

        /// SAFETY: the data inside must be a byte array (and not a transmuted list)
        #[inline]
        pub unsafe fn to_bytes(&self) -> &[u8] {
            core::slice::from_raw_parts(self.dat as *const u8, self.len)
        }

        #[inline]
        pub unsafe fn try_to_string(&self) -> Result<String, std::str::Utf8Error> {
            core::str::from_utf8(self.to_bytes()).map(|i| i.to_string())
        }

        /// SAFETY: the data inside must be a transmuted list
        #[inline]
        pub unsafe fn transmute_to_list(self) -> NbuList {
            core::mem::transmute(self)
        }
    }

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct NbuList {
        pub dat: *mut NbuStringSlice,
        pub len: usize,
    }

    impl NbuList {
        #[inline]
        pub fn empty() -> Self {
            Self {
                dat: core::ptr::null_mut(),
                len: 0,
            }
        }

        #[inline]
        pub fn to_slice(&self) -> &[NbuStringSlice] {
            unsafe { core::slice::from_raw_parts(self.dat, self.len) }
        }
    }

    #[repr(C)]
    pub struct NbuRegexCache([u8; 0]);

    #[link(name = "nix-builtin-utils-2.0")]
    extern "C" {
        // slice stuff
        //pub fn nbu_make_slice_owned(this: *mut NbuStringSlice) -> bool;
        //pub fn nbu_fini_slice_owned(this: *mut NbuStringSlice);
        //pub fn nbu_slices_eq(a: NbuStringSlice, b: NbuStringSlice) -> bool;

        // list stuff
        //pub fn nbu_init_list(this: *mut NbuList, len: usize);
        pub fn nbu_fini_list(this: *mut NbuList, recurse: usize);

        // regex cache
        pub fn nbu_create_regex_cache() -> *mut NbuRegexCache;
        pub fn nbu_destroy_regex_cache(this: *mut NbuRegexCache);

        // regex stuff
        pub fn nbu_match(
            cache: *mut NbuRegexCache,
            ret: *mut NbuList,
            rgx: NbuStringSlice,
            s: NbuStringSlice,
        ) -> raw::c_int;

        pub fn nbu_split(
            cache: *mut NbuRegexCache,
            ret: *mut NbuList,
            rgx: NbuStringSlice,
            s: NbuStringSlice,
        ) -> raw::c_int;

        pub fn nbu_split_fini_ret(ret: *mut NbuList);
    }

    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    pub enum RegexError {
        OutOfMemory,
        InvalidRegex,
    }

    impl std::error::Error for RegexError {}

    #[inline]
    pub fn decode_regex_ret(ret: raw::c_int) -> Result<bool, RegexError> {
        // WARNING: the following values must be kept in sync with the
        //   NBU_MATCH_ constants in nix-builtin-utils.h
        match ret {
            0 => Ok(true),
            1 => Ok(false),
            -12 => Err(RegexError::OutOfMemory),
            -22 => Err(RegexError::InvalidRegex),
            _ => panic!("unexpected nbu return value {}", ret),
        }
    }

    impl fmt::Display for RegexError {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            match self {
                RegexError::OutOfMemory => f.write_str("out of memory"),
                RegexError::InvalidRegex => f.write_str("invalid regex"),
            }
        }
    }
}

pub mod regex {
    use crate::ffi;

    pub use crate::ffi::RegexError as Error;

    pub struct Cache(*mut ffi::NbuRegexCache);

    impl Cache {
        pub fn new() -> Self {
            let ret = unsafe { ffi::nbu_create_regex_cache() };
            assert!(!ret.is_null());
            Self(ret)
        }
    }

    impl Drop for Cache {
        fn drop(&mut self) {
            unsafe { ffi::nbu_destroy_regex_cache(self.0) };
        }
    }

    pub fn match_(cache: &mut Cache, regex: &str, st: &str) -> Result<Option<Vec<String>>, Error> {
        let mut matches = ffi::NbuList::empty();

        let regex = unsafe { ffi::NbuStringSlice::from_bytes(regex.as_bytes()) };
        let st = unsafe { ffi::NbuStringSlice::from_bytes(st.as_bytes()) };

        let r = ffi::decode_regex_ret(unsafe { ffi::nbu_match(cache.0, &mut matches, regex, st) })
            .map(|b| {
                if b {
                    Some(
                        matches
                            .to_slice()
                            .iter()
                            .map(|i| {
                                unsafe { i.try_to_string() }
                                    .expect("got non-utf8 data from nbu API")
                            })
                            .collect(),
                    )
                } else {
                    None
                }
            });

        unsafe { ffi::nbu_fini_list(&mut matches, 1) };

        r
    }

    #[derive(Clone, Debug, PartialEq, Eq)]
    pub enum SplitItem {
        Literal(String),
        Matches(Vec<String>),
    }

    pub fn split(cache: &mut Cache, regex: &str, st: &str) -> Result<Vec<SplitItem>, Error> {
        let mut matches = ffi::NbuList::empty();

        let regex = unsafe { ffi::NbuStringSlice::from_bytes(regex.as_bytes()) };
        let st = unsafe { ffi::NbuStringSlice::from_bytes(st.as_bytes()) };

        let r = ffi::decode_regex_ret(unsafe { ffi::nbu_split(cache.0, &mut matches, regex, st) })
            .map(|b| {
                if b {
                    matches
                        .to_slice()
                        .iter()
                        .enumerate()
                        .map(|(n, i)| {
                            if n % 2 == 0 {
                                SplitItem::Literal(
                                    unsafe { i.try_to_string() }
                                        .expect("got non-utf8 data from nbu API"),
                                )
                            } else {
                                SplitItem::Matches(
                                    unsafe { i.transmute_to_list() }
                                        .to_slice()
                                        .iter()
                                        .map(|i| {
                                            unsafe { i.try_to_string() }
                                                .expect("got non-utf8 data from nbu API")
                                        })
                                        .collect(),
                                )
                            }
                        })
                        .collect()
                } else {
                    vec![SplitItem::Literal(
                        unsafe { st.try_to_string() }.expect("got non-utf8 data from nbu API"),
                    )]
                }
            });

        unsafe { ffi::nbu_split_fini_ret(&mut matches) };

        r
    }
}

#[cfg(test)]
mod tests;
