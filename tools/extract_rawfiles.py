"""Extract the rawfiles (scripts, text) and string tables (csv) from CoD4 fastfiles.

    py tools/extract_rawfiles.py <out folder> <fastfile> [fastfile...]

A fastfile is "IWffu100", a version (5) and a zlib stream of the zone. Assets written inline put
-1 where a pointer would be, then the data it points to. So a rawfile is
    -1 (name), length, -1 (buffer), name\0, buffer[length]\0
and a string table is
    -1 (name), columns, rows, -1 (values), name\0, columns*rows times -1, then each string\0.
Only tables written that way are found. Most stock tables (rankTable.csv among them) share strings
with other assets through offsets into the loaded zone; for those, run `dumpraw 32` in the game.
The output is the game's own content: keep it out of the public repo (build/ is ignored).
"""
import re
import struct
import sys
import zlib
from pathlib import Path

INLINE = b"\xff\xff\xff\xff"
NAME = rb"([A-Za-z0-9_\-./]+\.[A-Za-z0-9]+)\x00"
RAWFILE = re.compile(INLINE + rb"(.{4})" + INLINE + NAME, re.DOTALL)
TABLE = re.compile(INLINE + rb"(.{4})(.{4})" + INLINE + rb"([A-Za-z0-9_\-./]+\.csv)\x00", re.DOTALL)


def zone(path):
    data = Path(path).read_bytes()
    if data[:8] != b"IWffu100" or struct.unpack_from("<I", data, 8)[0] != 5:
        raise ValueError(f"{path} is not a CoD4 fastfile")
    return zlib.decompress(data[12:])


def rawfiles(data):
    for match in RAWFILE.finditer(data):
        length = struct.unpack("<i", match.group(1))[0]
        start = match.end()
        if 0 <= length < len(data) - start and data[start + length] == 0:
            yield match.group(2).decode(), data[start:start + length]


def tables(data):
    for match in TABLE.finditer(data):
        columns, rows = struct.unpack("<ii", match.group(1) + match.group(2))
        count = columns * rows
        start = match.end()
        if not (0 < columns < 256 and 0 < rows < 65536) or data[start:start + 4 * count] != INLINE * count:
            continue
        cells, at = [], start + 4 * count
        for _ in range(count):
            end = data.index(b"\x00", at)
            cells.append(data[at:end].decode("latin-1"))
            at = end + 1
        lines = (",".join(cells[r * columns:(r + 1) * columns]) for r in range(rows))
        yield match.group(3).decode(), ("\n".join(lines) + "\n").encode("latin-1")


def main(out, *fastfiles):
    out = Path(out)
    for fastfile in fastfiles:
        data = zone(fastfile)
        found = 0
        for name, content in [*rawfiles(data), *tables(data)]:
            target = out / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(content)
            found += 1
        print(f"{fastfile}: {found} files")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    main(*sys.argv[1:])
