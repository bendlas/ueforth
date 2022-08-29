{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs";
  inputs.gitignore = {
    url = "github:hercules-ci/gitignore.nix";
    # Use the same nixpkgs
    inputs.nixpkgs.follows = "nixpkgs";
  };
  outputs = { self, nixpkgs, gitignore }:
    let
      system = "x86_64-linux";
      inherit (gitignore.lib) gitignoreSource;
      pkgs = import nixpkgs {
        # overlays = [ portable-svc.overlay ];
        inherit system;
      };
    in {
      packages."${system}".default = pkgs.stdenv.mkDerivation {
        pname = "ueforth";
        version = "0";
        src = gitignoreSource ./.;
        postPatch = ''
          set -v
          patchShebangs .
        '';
        makeFlags = [ "out/posix/ueforth" ];
        installPhase = ''
          mkdir -p $out/bin
          cp out/posix/ueforth $out/bin
        '';
      };
    };
}
