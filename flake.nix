{
  description = "Chat App dev/build environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    devenv.url = "github:cachix/devenv";
  };

  outputs = { nixpkgs, devenv, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells."${system}".default = with pkgs;
        devenv.lib.mkShell {
          inherit inputs pkgs;

          modules = [
            (_: {

              languages = {
                c.enable = true;
                cplusplus.enable = true;
              };

              packages =
                [ gamenetworkingsockets lldb_16 cz-cli yarn cz-cli yarn ];

              pre-commit.hooks = {
                deadnix.enable = true;
                nixpkgs-fmt.enable = true;
                clang-format.enable = true;
                clang-tidy.enable = true;
              };

              enterShell = ''
                export CPLUS_INCLUDE_PATH="$C_INCLUDE_PATH"
              '';

            })
          ];
        };

    };
}
