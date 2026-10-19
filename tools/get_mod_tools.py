"""Fetch the parts of the CoD4 mod tools that tools/build_unlimited.py needs.

    py tools/get_mod_tools.py

Clones the community mirror of Activision's mod tools at a fixed commit into
tools/cod4-mod-tools/sdk, checking out only the linker and converter, their definition files, the
shader sources materials are built from and the stock materials our menus use. The tools are Activision's, so they stay out of this
repository.
"""
import subprocess
import sys
from pathlib import Path

REPO = "https://github.com/promod/CoD4-Mod-Tools.git"
COMMIT = "7e538c6632a5f2311a2a16f86e5b6786b4e65d16"
PARTS = ["bin", "deffiles", "raw/materials", "raw/shader_bin", "raw/statemaps", "raw/techniques", "raw/techsets"]
SDK = Path(__file__).resolve().parent / "cod4-mod-tools" / "sdk"


def git(*args):
    subprocess.run(["git", "-C", str(SDK), *args], check=True)


def main():
    if not (SDK / ".git").exists():
        SDK.mkdir(parents=True, exist_ok=True)
        git("init", "-q")
        git("remote", "add", "origin", REPO)
    git("sparse-checkout", "set", "--cone", *PARTS)
    git("fetch", "-q", "--filter=blob:none", "--depth", "1", "origin", COMMIT)
    git("checkout", "-q", "--detach", COMMIT)
    print(f"{SDK} at {COMMIT[:8]}")


if __name__ == "__main__":
    sys.exit(main())
