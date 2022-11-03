## ----------------
## GD32 cross build

let
  env = import ../gd32v/shell.nix;
  buildEnv = env.main.buildEnv.extend (self: super: {
    cppDefines = super.cppDefines // {
      HAVE_SYS_MMAN = "0";
    };
  });
  elf = env.pkgsCross.callPackage ./default.nix { inherit buildEnv; };
in buildEnv.mkBin "eforth.bin" elf

## ------------
## native build

# let
#   pkgs = import <nixpkgs> { };
#   buildEnv = pkgs.callPackage ../gd32v/build-env.nix { };
# in pkgs.callPackage ./default.nix {
#   buildEnv = buildEnv.extend (self: super: {
#     cppDefines = super.cppDefines // {
#       HAVE_SYS_MMAN = "1";
#     };
#     archFlags = super.archFlags ++ [ "-ldl" "-lm" ];
#     platformCFlags = super.platformCFlags ++ [
#       "-mno-stack-arg-probe"
#     ];
#   });
# }
