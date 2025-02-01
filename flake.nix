{
  description = "a c dev environment";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };
  outputs = { self, nixpkgs }: let 
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      mkCDevShell = {}: pkgs.mkShell {
        buildInputs = [
          pkgs.clang-tools
	  			pkgs.clang
	  			pkgs.valgrind
					pkgs.gdb
          pkgs.gcc
          pkgs.ninja
          pkgs.bash
          pkgs.binutils
				];
      };
  in {
    devShells.${system}.default = mkCDevShell {};
  };
}
