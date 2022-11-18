{ config, pkgs, lib, ... }:
{
  # require = ../gd32v/firmware.dwn.nix;
  require = {
    core.output = "elf";
    cc.cFlagsCompile = [ "-Os" "-ggdb" ];
    cc.cFlagsLink = [ "-lm" ];
  };

  config = {
    core.pname = "eforth";
    core.version = "0";

    bestow.core.echoBuildCommands = false;

    bestow.cpp.defines = {
      UEFORTH_MINIMAL = "";
      PRINT_ERRORS = "1";
      TRACE_CREATE = "0";
      TRACE_CALLS = "0";
      HAVE_SYS_MMAN = "0";
    };

    cc.inputs = [{
      core.pname = "blink";
      core.output = "o";
      # core.hostPlatform = builtins.currentSystem;
      cc.inputs = [ ./clib/main.c ];
      # bestow.core.echoBuildCommands = true;
      # core.echoBuildCommands = true;
    }];

    eforth.bootFiles = let
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
    # in [];
    in bootCommon1
       ++ [ ./clib/allocation.fs ]
       ++ bootCommon2
       ++ [ ./clib/autoboot.fs ./common/fini.fs ];

    cc.ldFlags = [
      "--build-id=none"
    ];

    bestow.cc = {
      lto = true;
      cFlagsCompile = [
        "-no-pie"
      ];
      fFlags = [
        "no-exceptions" "freestanding" "no-stack-protector" "omit-frame-pointer"
        "no-ident" "merge-all-constants"
      ];
      warningFlags = [
        "stack-usage=8KB"
      ];
      allowWarnings = [
        "format-security" "float-conversion" "missing-braces"
        "unused-variable" "sign-conversion" "conversion"
        "unused-parameter" "unused-but-set-variable"
      ];
    };
    bestow.cpp.includeDirs = [
      (pkgs.linkFarm "eforth-includes" [
        { name = "common"; path = ./common; }
        {
          name = "gen/clib_boot.h";
          path = pkgs.runCommand "clib_boot.h" {
            nativeBuildInputs = [ pkgs.nodejs ];
            inherit (config.core) version;
            inherit (config.eforth) revision bootFiles;
          } ''
          node ${./tools/source_to_string.js} boot $version $revision $bootFiles > $out
        '';
        }
      ])
    ];

  };

  options.eforth = with lib; with types; {
    revision = mkOption {
      type = str;
      default = "latest";
    };
    bootFiles = mkOption {
      type = listOf path;
    };
  };

}
