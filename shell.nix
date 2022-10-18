let
  env = import ../gd32v/shell.nix;
in env.pkgsCross.callPackage ./default.nix {
  inherit (env.main) buildEnv;
}
  
