# nix-builtin-utils

## Why?

nix builtins which rely on regex parsing+matching are notoriously difficult
to get right, because most regex libraries differ in what syntax they accept,
if they support backreferences, etc.

So this library extracts the corresponding code from Nix' [`src/libexpr/primops.cc`](https://github.com/NixOS/nix/blob/master/src/libexpr/primops.cc)
and makes it available for usage by alternative Nix evaluator implementations.
