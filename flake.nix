{
  inputs.gitignore = {
    url = "github:hercules-ci/gitignore.nix";
    # Use the same nixpkgs
    inputs.nixpkgs.follows = "nixpkgs";
  };
  outputs = { self, /* other, inputs, */ gitignore }:
    let
      inherit (gitignore.lib) gitignoreSource;
    in {
      packages.x86_64.ueforth = mkDerivation {
        pname = "ueforth";
        src = gitignoreSource ./.;
        makeFlags = [ "out/posix/ueforth" ];
        installPhase = ''
          mkdir -p $out/bin
          cp out/posix/ueforth $out/bin
        '';
      };
    };
}
