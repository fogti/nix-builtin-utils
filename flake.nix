{
  description = "Nix standalone builtin utils";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/master";
    yz-flake-utils.url = "github:YZITE/flake-utils";
  };
  outputs = {
    nixpkgs,
    yz-flake-utils,
    ...
  }:
    yz-flake-utils.lib.mkFlake {
      prevpkgs = nixpkgs;
      contentAddressedByDefault = false;
      defaultProgName = "nix-builtin-utils";
      overlay = final: prev: {
        nix-builtin-utils = prev.callPackage ./. {};
        nix-builtin-utils-rust =
          (import ./Cargo.nix {
            pkgs = final;
            defaultCrateOverrides =
              final.defaultCrateOverrides
              // {
                nix-builtin-utils = attrs: {
                  buildInputs =
                    (
                      if attrs ? buildInputs
                      then attrs.buildInputs
                      else []
                    )
                    ++ [final.nix-builtin-utils];
                };
              };
          })
          .rootCrate
          .build
          .override {
            runTests = true;
          };
      };
    };
}
