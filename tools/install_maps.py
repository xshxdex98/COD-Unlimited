"""Install the maps COD Unlimited ships (mod/maps.csv) into the game's zone/maps folder.

    py tools/install_maps.py <folder with the extracted map downloads> [game folder]   (default bin/Debug)

Each map gets zone/maps/<map>/ with its two zones, its iwds and an arena that lists it with its
display name (a localized string built into unlimited_mp.ff), the gametypes it supports and the
loading screen the menus preview it with. Iwds stay in the map's folder because they often replace
stock images: the game only searches them while that map is loaded. The maps are third-party
content, so they are not in this repository.

mod/maps.csv holds what was read from each map's own files: its loading screen image (named by its
load zone; remakes often name it their own way) and the soldier types its script sets for each
team, which the build puts in mp/mapsTable.csv like the stock maps'.
"""
import csv
import re
import shutil
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAPS = ROOT / "mod" / "maps.csv"
# Every map's entities were checked for what these gametypes need (spawns, bomb sites, flags,
# hardpoints).
GAMETYPES = "dm war sd sab dom koth"
# Alternate builds some downloads include for mods that bring their own player models.
SKIP = "without character model"


def read_maps():
    """mod/maps.csv's rows, in order, each with its localized string as "key"."""
    with MAPS.open(newline="") as file:
        maps = list(csv.DictReader(file))
    for row in maps:
        row["key"] = "MPUI_" + re.sub(r"[^A-Z0-9]+", "_", row["name"].upper())
    return maps


def find_zone(source, name):
    found = [path for path in source.rglob(f"{name}.ff") if SKIP not in str(path).lower()]
    if len(found) != 1:
        sys.exit(f"{name}: expected one {name}.ff under {source}, found {len(found)}")
    return found[0]


def has_image(iwds, image):
    entry = f"images/{image}.iwi".lower()
    for iwd in iwds:
        with zipfile.ZipFile(iwd) as archive:
            if any(name.lower() == entry for name in archive.namelist()):
                return True
    return False


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    source = Path(sys.argv[1])
    game = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "bin" / "Debug"
    installed = game / "zone" / "maps"
    maps = read_maps()

    # Everything is found and checked before anything in the game changes.
    files = {}
    for row in maps:
        name = row["map"]
        zone = find_zone(source, name)
        load = zone.with_name(f"{name}_load.ff")
        iwds = sorted(zone.parent.glob("*.iwd"))
        if not load.exists():
            sys.exit(f"{name}: no {load.name} next to {zone}")
        if not has_image(iwds, row["loadscreen"]):
            sys.exit(f"{name}: its iwds have no images/{row['loadscreen']}.iwi")
        files[name] = [zone, load, *iwds]

    for row in maps:
        name = row["map"]
        target = installed / name
        target.mkdir(parents=True, exist_ok=True)
        # Copied over the old files, which stay until the copies land: a file the running game
        # holds open fails here instead of leaving the map half deleted.
        for file in files[name]:
            shutil.copy2(file, target / file.name)
        arena = target / f"{name}.arena"
        for stale in set(target.iterdir()) - {target / file.name for file in files[name]} - {arena}:
            stale.unlink()
        arena.write_text(
            f'{{\n\tmap\t\t\t"{name}"\n\tlongname\t"{row["key"]}"\n\tgametype\t"{GAMETYPES}"\n'
            f'\tloadscreen\t"{row["loadscreen"]}"\n}}\n')
        print(f"{row['name']:14} {name}")

    listed = {row["map"] for row in maps}
    for folder in sorted(installed.iterdir()):
        if folder.name not in listed:
            print(f"not in {MAPS.name}, left alone: {folder}")


if __name__ == "__main__":
    main()
