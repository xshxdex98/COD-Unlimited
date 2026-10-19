"""Build mod/ into unlimited_mp.ff and install it in the game copy.

    py tools/build_unlimited.py [game folder]     (default bin/Debug)

linker_pc expects a game-shaped folder: bin/ with the linker, raw/ with the sources and
zone_source/ with the asset list. That folder is put together under build/linker each time, so
nothing stale from an earlier build gets linked in.
"""
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ZONE = "unlimited_mp"
TOOLS = ROOT / "tools" / "cod4-mod-tools" / "bin"
WORK = ROOT / "build" / "linker"
STOCK = ROOT / "build" / "stock_raw"


def unlink_main():
    """Remove the junction alone; rmtree on the workspace must never walk into the game's main."""
    junction = WORK / "main"
    if junction.exists() or junction.is_junction():
        junction.rmdir()


def main():
    game = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "bin" / "Debug"
    if not (TOOLS / "linker_pc.exe").exists():
        sys.exit(f"missing {TOOLS / 'linker_pc.exe'}: the CoD4 mod tools are not checked in, see provenance.json")
    if not STOCK.exists():
        sys.exit(f"missing {STOCK}: run tools/extract_rawfiles.py {STOCK} <the game's code_post_gfx_mp.ff and common_mp.ff>")

    unlink_main()
    shutil.rmtree(WORK, ignore_errors=True)
    shutil.copytree(TOOLS, WORK / "bin")
    # The linker starts the game's file system, which needs main\ (fileSysCheck.cfg and the iwds).
    subprocess.run(["cmd", "/c", "mklink", "/J", str(WORK / "main"), str(game / "main")], check=True,
                   capture_output=True)
    # The linker links against Miles and D3DX like the game does.
    shutil.copy(ROOT / "deps" / "msslib" / "dlls" / "mss32.dll", WORK / "bin")
    shutil.copy(game / "d3dx9_34.dll", WORK / "bin")
    # The stock files the linker reads (tools/extract_rawfiles.py), with ours over them.
    shutil.copytree(STOCK, WORK / "raw")
    shutil.copytree(ROOT / "mod" / "raw", WORK / "raw", dirs_exist_ok=True)
    (WORK / "zone_source" / "english" / "assetlist").mkdir(parents=True)     # the linker writes its asset list here
    shutil.copy(ROOT / "mod" / f"{ZONE}.csv", WORK / "zone_source" / f"{ZONE}.csv")

    # No console: after an error the linker waits for a key press on it.
    result = subprocess.run([str(WORK / "bin" / "linker_pc.exe"), "-language", "english", "-compress", "-cleanup", ZONE],
                            cwd=WORK / "bin", stdin=subprocess.DEVNULL, capture_output=True, text=True,
                            errors="replace", timeout=600, creationflags=subprocess.DETACHED_PROCESS)
    output = result.stdout + result.stderr
    built = WORK / "zone" / "english" / f"{ZONE}.ff"
    if result.returncode or "ERROR" in output or not built.exists():
        print(output)
        sys.exit(f"linker_pc failed (exit {result.returncode})")

    target = game / "zone" / "english" / built.name
    shutil.copy(built, target)
    print(f"{target}: {built.stat().st_size} bytes")


if __name__ == "__main__":
    main()
