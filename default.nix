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
  cppDefines = super.cppDefines // {
    UEFORTH_MINIMAL = "";
    PRINT_ERRORS = "1";
    TRACE_CREATE = "1";
    TRACE_CALLS = "1";
  };
  archFlags = super.archFlags ++ [ "-no-pie" ];
  ldFlagsWl = super.ldFlagsWl ++ [ "--build-id=none" ];
  fFlags = super.fFlags ++ [
    "no-exceptions" "freestanding" "no-stack-protector" "omit-frame-pointer"
    "no-ident" "merge-all-constants"
  ];

  allowWarnings = [
    "format-security"
    "float-conversion"
    "missing-braces"
    "unused-variable"
    "sign-conversion"
    "conversion"
  ];
  includeDirs = super.includeDirs ++ [
    (linkFarm "eforth-includes" [
      { name = "common"; path = ./common; }
      { name = "clib"; path = ./clib; }
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

  mkPlatformElf "eforth.elf" [
    (mkCObj "main.o" [ ./clib/main.c ])
  ]
    
