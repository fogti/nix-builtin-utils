{ lib
, meson
, ninja
, stdenv
}:

stdenv.mkDerivation {
  pname = "nix-builtin-utils";
  version = "0.2.0";
  src = ./.;
  nativeBuildInputs = [ meson ninja ];

  meta = with lib; {
    description = "Nix standalone builtin utils";
    #homepage = "https://github.com/zseri/libowlevelzs";
    license = licenses.lgpl2Plus;
    platforms = platforms.all;
  };
}
