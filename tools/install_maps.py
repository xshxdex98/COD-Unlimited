"""Install the maps COD Unlimited ships (mod/maps.csv) into the game's zone/maps folder.

    py tools/install_maps.py <folder with the extracted map downloads> [game folder]   (default bin/Debug)

Each map gets zone/maps/<map>/ with its two zones, its iwds and an arena that lists it with its
display name (a localized string built into unlimited_mp.ff) and the gametypes it supports. Iwds
stay in the map's folder because they often replace stock images: the game only searches them
while that map is loaded. The maps are third-party content, so they are not in this repository.
"""
import csv
import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAPS = ROOT / "mod" / "maps.csv"
# Every map's entities were checked for what these gametypes need (spawns, bomb sites, flags,
# hardpoints).
GAMETYPES = "dm war sd sab dom koth"
# Alternate builds some downloads include for mods that bring their own player models.
SKIP = "without character model"


def read_maps():
    """[(map, display name, localized string)] in mod/maps.csv order."""
    with MAPS.open(newline="") as file:
        return [(row["map"], row["name"], "MPUI_" + re.sub(r"[^A-Z0-9]+", "_", row["name"].upper()))
                for row in csv.DictReader(file)]


def find_zone(source, name):
    found = [path for path in source.rglob(f"{name}.ff") if SKIP not in str(path).lower()]
    if len(found) != 1:
        sys.exit(f"{name}: expected one {name}.ff under {source}, found {len(found)}")
    return found[0]


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    source = Path(sys.argv[1])
    game = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "bin" / "Debug"
    installed = game / "zone" / "maps"
    maps = read_maps()
    zones = {name: find_zone(source, name) for name, _, _ in maps}   # all found before anything changes

    for name, display, key in maps:
        zone = zones[name]
        files = [zone, zone.with_name(f"{name}_load.ff"), *sorted(zone.parent.glob("*.iwd"))]
        if not files[1].exists():
            sys.exit(f"{name}: no {files[1].name} next to {zone}")
        target = installed / name
        shutil.rmtree(target, ignore_errors=True)
        target.mkdir(parents=True)
        for file in files:
            shutil.copy2(file, target / file.name)
        (target / f"{name}.arena").write_text(
            f'{{\n\tmap\t\t\t"{name}"\n\tlongname\t"{key}"\n\tgametype\t"{GAMETYPES}"\n}}\n')
        print(f"{display:14} {name}: {', '.join(file.name for file in files[2:])}")

    listed = {name for name, _, _ in maps}
    for folder in sorted(installed.iterdir()):
        if folder.name not in listed:
            print(f"not in {MAPS.name}, left alone: {folder}")


if __name__ == "__main__":
    main()
