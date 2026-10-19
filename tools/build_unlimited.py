"""Build mod/ into unlimited_mp.ff and z_unlimited.iwd and install them in the game copy.

    py tools/build_unlimited.py [game folder]     (default bin/Debug)

The mod tools expect a game-shaped folder: bin/ with the tools, deffiles/, raw/ with the sources,
source_data/ and texture_assets/ for the converter and zone_source/ with the asset list. That
folder is put together under build/linker each time, so nothing stale gets linked in.

Materials go into the fastfile; the game loads their images from images/*.iwi through the file
system, so those ship in the .iwd. Generated art (tools/art) is rendered into the workspace and
listed alongside mod/'s own assets, and so are the maps in mod/maps.csv (names and mapsTable rows)
and the player levels (tools/ranks.py).
"""
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

import install_maps
import ranks
from art import rank_icons

ROOT = Path(__file__).resolve().parents[1]
ZONE = "unlimited_mp"
IWD = "z_unlimited.iwd"
SDK = ROOT / "tools" / "cod4-mod-tools" / "sdk"
MOD = ROOT / "mod"
WORK = ROOT / "build" / "linker"
STOCK = ROOT / "build" / "stock_raw"
# Read-only parts of the mod tools, linked into the workspace instead of copied.
SDK_LINKS = ["deffiles", "raw/shader_bin", "raw/statemaps", "raw/techniques", "raw/techsets"]


def link(target, source):
    subprocess.run(["cmd", "/c", "mklink", "/J", str(target), str(source)], check=True, capture_output=True)


def clean_workspace():
    """Remove the junctions first; rmtree must never walk into the game or the mod tools."""
    for name in ["main", *SDK_LINKS]:
        junction = WORK / name
        if junction.is_junction():
            junction.rmdir()
    shutil.rmtree(WORK, ignore_errors=True)


def material_entry(name, color_map):
    """A GDT entry for a HUD or menu image: clamped and never reduced by texture quality, like the
    stock rank icons, but with mipmaps so it stays clean when drawn small."""
    fields = {"materialType": "2d", "surfaceType": "<none>", "usage": "<not in editor>",
              "colorMap": color_map.replace("/", "\\\\"), "tileColor": "no tile", "nopicmipColor": "1"}
    lines = [f'\t"{name}" ( "material.gdf" )', "\t{", *(f'\t\t"{key}" "{value}"' for key, value in fields.items()), "\t}"]
    return "\n".join(lines) + "\n"


def write_generated_art():
    """Renders the generated icons and their materials into the workspace; returns their zone entries."""
    folder = "texture_assets/unlimited/ranks"
    (WORK / folder).mkdir(parents=True)
    names, entries = [], []
    for name, image in rank_icons.icons():
        image.save(WORK / folder / f"{name}.tga")
        names.append(name)
        entries.append(material_entry(name, f"{folder}/{name}.tga"))
    (WORK / "source_data").mkdir(exist_ok=True)
    (WORK / "source_data" / "unlimited_generated.gdt").write_text("{\n" + "".join(entries) + "}\n")
    return [f"material,{name}" for name in names]


def write_maps():
    """The maps we ship, listed like the stock ones: display names as MPUI_ strings and rows in
    mp/mapsTable.csv (soldier types, name, loading screen) after the stock rows. Returns the zone entries."""
    maps = install_maps.read_maps()
    lines = ['VERSION             "1"', 'CONFIG              "C:\\trees\\cod3\\cod3\\bin\\StringEd.cfg"',
             'FILENOTES           ""', ""]
    for row in maps:
        lines += [f"REFERENCE           {row['key'].removeprefix('MPUI_')}", f'LANG_ENGLISH        "{row["name"]}"', ""]
    folder = WORK / "raw" / "english" / "localizedstrings"
    folder.mkdir(parents=True, exist_ok=True)
    (folder / "mpui.str").write_text("\n".join(lines + ["ENDMARKER"]) + "\n")

    # Columns: map, allies and axis soldier types, name, loading screen, index, description (unused), compass overlay.
    table = (STOCK / "mp" / "mapstable.csv").read_text().splitlines()
    index = max(int(line.split(",")[5]) for line in table[1:])
    for number, row in enumerate(maps, index + 1):
        table.append(f"{row['map']},{row['allies']},{row['axis']},{row['key']},{row['loadscreen']},{number},,compass_overlay_map_blank")
    (WORK / "raw" / "mp" / "mapstable.csv").write_text("\n".join(table) + "\n")
    return ["localize,mpui", "stringtable,mp/mapstable.csv"]


def run_tool(exe, *args):
    # No console: after an error the tools wait for a key press on it.
    result = subprocess.run([str(WORK / "bin" / exe), *args], cwd=WORK / "bin", stdin=subprocess.DEVNULL,
                            capture_output=True, text=True, errors="replace", timeout=600,
                            creationflags=subprocess.DETACHED_PROCESS)
    output = result.stdout + result.stderr
    # A menu that fails to parse is only reported; the linker still writes the zone.
    if result.returncode or "ERROR" in output or "Menu load error" in output:
        print(output)
        sys.exit(f"{exe} failed (exit {result.returncode})")
    return output


def main():
    game = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "bin" / "Debug"
    if not (SDK / "bin" / "linker_pc.exe").exists():
        sys.exit(f"missing the CoD4 mod tools in {SDK}: run tools/get_mod_tools.py")
    if not STOCK.exists():
        sys.exit(f"missing {STOCK}: run tools/extract_rawfiles.py {STOCK} <the game's code_post_gfx_mp.ff and common_mp.ff>")

    clean_workspace()
    (WORK / "bin").mkdir(parents=True)
    for tool in (SDK / "bin").iterdir():
        if tool.is_file():
            shutil.copy(tool, WORK / "bin")
    # The tools link against D3DX like the game does.
    shutil.copy(game / "d3dx9_34.dll", WORK / "bin")
    # The linker starts the game's file system, which needs main\ (fileSysCheck.cfg and the iwds).
    link(WORK / "main", game / "main")
    # The stock files the linker reads (tools/extract_rawfiles.py), with ours over them.
    shutil.copytree(STOCK, WORK / "raw")
    # Stock materials a menu uses are linked in with it, from the mod tools' definitions.
    shutil.copytree(SDK / "raw" / "materials", WORK / "raw" / "materials", dirs_exist_ok=True)
    shutil.copytree(MOD / "raw", WORK / "raw", dirs_exist_ok=True)
    for name in SDK_LINKS:
        link(WORK / name, SDK / name)
    for name in ["source_data", "texture_assets"]:
        if (MOD / name).exists():
            shutil.copytree(MOD / name, WORK / name)
    generated = write_generated_art() + write_maps() + ranks.write(STOCK, WORK / "raw")
    (WORK / "zone_source" / "english" / "assetlist").mkdir(parents=True)     # the linker writes its asset list here
    assets = (MOD / f"{ZONE}.csv").read_text().splitlines() + generated
    (WORK / "zone_source" / f"{ZONE}.csv").write_text("\n".join(assets) + "\n")

    run_tool("converter.exe", "-nopause")
    run_tool("linker_pc.exe", "-language", "english", "-compress", "-cleanup", ZONE)
    built = WORK / "zone" / "english" / f"{ZONE}.ff"
    if not built.exists():
        sys.exit(f"linker_pc wrote no {built.name}")

    images = sorted((WORK / "raw" / "images").glob("*.iwi"))
    installed = game / "main" / IWD
    # The iwd goes first: a running game holds it open, and then nothing is installed.
    if images:
        with zipfile.ZipFile(WORK / IWD, "w", zipfile.ZIP_DEFLATED) as archive:
            for image in images:
                archive.write(image, f"images/{image.name}")
        shutil.copy(WORK / IWD, installed)
    else:
        installed.unlink(missing_ok=True)
    shutil.copy(built, game / "zone" / "english" / built.name)
    print(f"{built.name}: {built.stat().st_size} bytes, {IWD}: {len(images)} images")


if __name__ == "__main__":
    main()
