{ lib
, gtest ? null
, meson
, ninja
, pkg-config
, stdenv
}:

stdenv.mkDerivation {
  pname = "nix-builtin-utils";
  version = "0.2.0";
  src = ./.;
  nativeBuildInputs = [ meson ninja pkg-config ];
  checkInputs = [ gtest ];
  doCheck = true;

  meta = with lib; {
    description = "Nix standalone builtin utils";
    homepage = "https://github.com/zseri/nix-builtin-utils";
    license = licenses.lgpl2Plus;
    platforms = platforms.all;
  };
}
