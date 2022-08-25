{
  lib,
  gtest ? null,
  meson,
  ninja,
  pkg-config,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nix-builtin-utils";
  version = "0.2.0";
  src = builtins.path {
    path = ./.;
    name = "nix-builtin-utils";
    filter = name: type:
      let baseName = builtins.baseNameOf (builtins.toString name);
      in ! (lib.hasSuffix ".nix" baseName || lib.hasSuffix ".rs" baseName);
  };
  nativeBuildInputs = [meson ninja pkg-config];
  checkInputs = [gtest];
  doCheck = true;

  meta = with lib; {
    description = "Nix standalone builtin utils";
    homepage = "https://github.com/zseri/nix-builtin-utils";
    license = licenses.lgpl2Plus;
    platforms = platforms.all;
  };
}
