fn main() {
    if let Err(e) = pkg_config::Config::new()
        .atleast_version("0.2")
        .probe("nix-builtin-utils-2.0")
    {
        println!(
            "pkg-config invocation failed ({}), resort to building from source",
            e
        );
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
