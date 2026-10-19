"""Player levels 1-99: mp/rankTable.csv, mp/rankIconTable.csv and the names of the new ranks.

Levels 1-55 are CoD4's rows unchanged (XP, unlocks, names) apart from their icons. Levels 56-99
are COD Unlimited's eleven tiers of four levels, the ones tools/art/rank_icons.py draws; each
costs XP_STEP more than the level before it, continuing CoD4's last step.

A tier's first level has no numeral and the next three are I-III, like CoD4's grades
("General", "General I", "General II"). The stock _rank.gsc announces levels 2 and 3 as
RANK_<NAME>_FULL_N plus a numeral and any other level by its column 16 name alone, so level 4
gets its numbered name there.
"""
from art import rank_icons

XP_STEP = 120
COLUMNS = 17


def tier_rows(stock_rows):
    """The new rows after CoD4's last one, as lists of COLUMNS fields."""
    last = stock_rows[-1]
    level, xp, step = int(last[0]) + 1, int(last[7]), int(last[3])
    rows = []
    for name, *_ in rank_icons.TIERS:
        key = f"RANK_{name.upper()}"
        for sub in range(1, 5):
            step += XP_STEP
            row = [""] * COLUMNS
            row[0], row[1], row[2], row[3] = str(level), f"{name}{sub}", str(xp), str(step)
            row[4], row[5] = key, f"{key}_FULL" if sub == 1 else f"{key}_FULL{sub}"
            row[6], row[7] = f"cu_rank_{name}{sub}", str(xp + step)
            row[13] = row[14] = str(level + 1)
            row[16] = {1: f"{key}_FULL", 4: f"{key}_FULL4"}.get(sub, f"{key}_FULL_N")
            rows.append(row)
            level, xp = level + 1, xp + step
    return rows


def tier_strings():
    """rank.str entries for the tier names, without the RANK_ prefix the file name gives them."""
    entries = []
    for name, *_ in rank_icons.TIERS:
        title, key = name.capitalize(), name.upper()
        entries += [(key, title), (f"{key}_FULL", title), (f"{key}_FULL2", f"{title} I"),
                    (f"{key}_FULL3", f"{title} II"), (f"{key}_FULL4", f"{title} III"),
                    (f"{key}_FULL_N", f"{title} &&1")]
    return entries


def write(stock, raw):
    """Writes the tables and strings from the dumped stock tables in stock/mp into the workspace
    raw folder; returns the zone entries."""
    header, maxrank, *rows = (stock / "mp" / "ranktable.csv").read_text().splitlines()
    rows = [line.split(",") for line in rows]
    names = [name for name, _ in rank_icons.icons()]
    assert len(names) == len(rows) + 4 * len(rank_icons.TIERS)
    for row, icon in zip(rows, names):
        assert icon == f"cu_rank_{row[1]}", f"{row[1]} has no icon"
        row[6] = icon
    rows += tier_rows(rows)

    maxrank = maxrank.split(",")
    maxrank[1] = rows[-1][0]
    lines = [header, ",".join(maxrank), *(",".join(row) for row in rows)]
    (raw / "mp" / "ranktable.csv").write_text("\n".join(lines) + "\n")

    icons = ["a0,b1", "maxprestige,0", *(f"{row[0]},{row[6]}" for row in rows)]
    (raw / "mp" / "rankicontable.csv").write_text("\n".join(icons) + "\n")

    lines = ['VERSION             "1"', 'CONFIG              "C:\\trees\\cod3\\cod3\\bin\\StringEd.cfg"',
             'FILENOTES           ""', ""]
    for reference, text in tier_strings():
        lines += [f"REFERENCE           {reference}", f'LANG_ENGLISH        "{text}"', ""]
    strings = raw / "english" / "localizedstrings"
    strings.mkdir(parents=True, exist_ok=True)
    (strings / "rank.str").write_text("\n".join(lines + ["ENDMARKER"]) + "\n")
    return ["localize,rank", "stringtable,mp/rankTable.csv", "stringtable,mp/rankIconTable.csv"]
