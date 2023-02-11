{ pkgs ? import <nixpkgs> {}}:

pkgs.mkShell {
  buildInputs = [
    pkgs.gcc
  ];
  nativeBuildInputs = [
    pkgs.pkg-config
    pkgs.llvm
  ];
}
