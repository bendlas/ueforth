{ lib, buildEnv, linkFarm, runCommand, nodejs
, version ? "dev"
, revision ? "latest"
}:
let
  bootFiles = bootCommon1
              ++ [ ./clib/allocation.fs ]
              ++ bootCommon2
              ++ [ ./clib/autoboot.fs ./common/fini.fs ];
  bootCommon1 = [
    ./common/comments.fs
    ./common/boot.fs
    ./common/io.fs
    ./common/conditionals.fs
    ./common/vocabulary.fs
    ./common/floats.fs
    ./common/structures.fs
  ];
  bootCommon2 = [
    ./common/utils.fs
    ./common/code.fs
    ./common/locals.fs
  ];
in with buildEnv.extend (self: super: {
  cFlags = super.cFlags ++ [
    "-Wno-error=format-security"
    "-Wno-error=float-conversion"
    "-Wno-error=missing-braces"
    "-Wno-error=unused-variable"
  ];
  ldFlags = super.ldFlags ++ [
    "-lm"
  ];
  includeDirs = super.includeDirs ++ [
    (linkFarm "eforth-includes" [
      { name = "common"; path = ./common; }
      {
        name = "gen/clib_boot.h";
        path = runCommand "clib_boot.h" {
          nativeBuildInputs = [ nodejs ];
          inherit version revision bootFiles;
        } ''
          node ${./tools/source_to_string.js} boot $version $revision $bootFiles > $out
        '';
      }
    ])
  ];
});

mkBin "eforth.bin"
  (mkPlatformElf "eforth.elf" [
    (mkCObj "main.o" [ ./clib/main.c ])
  ])
    
