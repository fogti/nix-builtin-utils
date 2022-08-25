fn main() {
    if pkg_config::Config::new()
        .atleast_version("0.2")
        .probe("nix-builtin-utils")
        .is_err()
    {
        // fallback to building from source
        cc::Build::new()
            .cpp(true)
            .file("src/list.c")
            .file("src/regex.cxx")
            .include("src")
            .warnings(true)
            .use_plt(false)
            .flag_if_supported("-std=c++17")
            .flag_if_supported("-Werror=return-type")
            .flag_if_supported("-fno-rtti")
            .compile("nix-builtin-utils");
    }
}
