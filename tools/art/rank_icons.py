"""Rank insignia for player levels 1-99 and the prestige emblems 1-10.

Every icon is flat geometry drawn at 8x size and scaled down, styled like an enamel pin: a flat
metal face, a darker bevelled rim and a hard dark outline. Levels 1-55 follow CoD4's grades
(three levels each, then Commander); 56-99 are COD Unlimited's own eleven tiers of four levels.
As in CoD4, the levels of one grade or tier share an icon; the level number is shown next to it.

The tiers are hexagon badges with the emblem on dark enamel. The emblem counts up in chevrons,
then a star joins it; the rim turns from steel to lime to gold to platinum, wings come in at
Reaper and a crest at Legend. Prestiges use a toothed seal the same way, with one to three stars.
"""
import math

from PIL import Image, ImageChops, ImageDraw, ImageFilter

SIZE = 128
SCALE = 8
S = SIZE * SCALE

# Metals as (face, rim).
STEEL = ((206, 213, 210), (116, 126, 122))
GOLD = ((232, 190, 92), (146, 100, 34))
LIME = ((168, 222, 128), (76, 128, 56))
PLATINUM = ((238, 242, 246), (130, 146, 160))
ENAMEL = ((24, 32, 29), (24, 32, 29))
OUTLINE = (8, 11, 10)
RIM = 0.016         # bevel width, in icon widths
STROKE = 0.014      # outline width

GRADES = [
    # (internal name, insignia, metal); three levels each, as in CoD4.
    ("pfc", ("chevrons", 1, 0), STEEL),
    ("lcpl", ("chevrons", 2, 0), STEEL),
    ("cpl", ("chevrons", 3, 0), STEEL),
    ("sgt", ("chevrons", 3, 1), STEEL),
    ("ssgt", ("chevrons", 3, 2), STEEL),
    ("gysgt", ("chevrons", 3, 3), STEEL),
    ("msgt", ("chevrons", 3, 3, "diamond"), STEEL),
    ("mgysgt", ("chevrons", 3, 3, "star"), STEEL),
    ("2ndlt", ("bars", 1), GOLD),
    ("1stlt", ("bars", 1), STEEL),
    ("capt", ("bars", 2), STEEL),
    ("maj", ("leaf",), GOLD),
    ("ltcol", ("leaf",), STEEL),
    ("col", ("eagle",), STEEL),
    ("bgen", ("stars", 1), STEEL),
    ("majgen", ("stars", 2), STEEL),
    ("ltgen", ("stars", 3), STEEL),
    ("gen", ("stars", 4), STEEL),
]
COMMANDER = ("comm", ("laurel",), GOLD)
TIERS = [
    # (name, chevrons, star, rim metal, emblem metal, wings, crest); four levels each, 56-99.
    ("vanguard", 1, False, STEEL, STEEL, False, False),
    ("sentinel", 2, False, STEEL, STEEL, False, False),
    ("warden", 3, False, STEEL, STEEL, False, False),
    ("hunter", 0, True, LIME, LIME, False, False),
    ("spectre", 1, True, LIME, LIME, False, False),
    ("phantom", 2, True, LIME, LIME, False, False),
    ("reaper", 0, True, GOLD, GOLD, True, False),
    ("warlord", 1, True, GOLD, GOLD, True, False),
    ("titan", 2, True, GOLD, GOLD, True, False),
    ("legend", 0, True, PLATINUM, PLATINUM, True, True),
    ("apex", 2, True, PLATINUM, GOLD, True, True),
]


def px(x, y):
    return (x * S, y * S)


def layer():
    """A blank mask and a drawing context for it."""
    mask = Image.new("L", (S, S))
    return mask, ImageDraw.Draw(mask)


def grow(mask, width):
    """Dilates (width > 0) or erodes (width < 0) a mask by about `width` icon widths."""
    soft = mask.filter(ImageFilter.GaussianBlur(abs(width) * S / 2))
    return soft.point(lambda v: 255 if (v > 8 if width > 0 else v > 247) else 0)


def star_points(cx, cy, r, inner=0.4, rotation=-90):
    points = []
    for i in range(10):
        radius = r if i % 2 == 0 else r * inner
        angle = math.radians(rotation + i * 36)
        points.append(px(cx + math.cos(angle) * radius, cy + math.sin(angle) * radius))
    return points


def hexagon(cx, cy, r):
    return [px(cx + math.cos(math.radians(a)) * r, cy + math.sin(math.radians(a)) * r) for a in range(-90, 270, 60)]


def chevron(draw, cx, cy, width, depth, thickness):
    """A V pointing up with its apex at (cx, cy)."""
    left, right = cx - width / 2, cx + width / 2
    draw.polygon([px(cx, cy), px(right, cy + depth), px(right, cy + depth + thickness),
                  px(cx, cy + thickness), px(left, cy + depth + thickness), px(left, cy + depth)], fill=255)


def rocker(draw, cy, width, sag, thickness):
    """A curved bar under the chevrons, sagging in the middle."""
    top, bottom = [], []
    for i in range(33):
        t = i / 32
        x = 0.5 - width / 2 + width * t
        y = cy + sag * math.sin(math.pi * t)
        top.append(px(x, y))
        bottom.append(px(x, y + thickness))
    draw.polygon(top + bottom[::-1], fill=255)


def leaf(draw, cx, cy, length, width, angle):
    """A pointed oval centred on (cx, cy), its long axis at `angle` degrees."""
    a = math.radians(angle)
    along, across = (math.cos(a), math.sin(a)), (-math.sin(a), math.cos(a))
    points = []
    for i in range(32):
        t = math.pi * 2 * i / 32
        u, v = math.cos(t) * length / 2, math.sin(t) * abs(math.sin(t)) ** 0.5 * width / 2
        points.append(px(cx + u * along[0] + v * across[0], cy + u * along[1] + v * across[1]))
    draw.polygon(points, fill=255)


def laurel(draw, cx, cy, radius, leaves=5, size=0.14):
    """Two branches climbing either side of (cx, cy) from the bottom, each a stem with pairs of
    leaves growing off it, smaller towards the leaf at the tip."""
    for side in (-1, 1):
        # Angles measured from straight down, sweeping up the side.
        stem = [(cx + side * math.sin(a) * radius, cy + math.cos(a) * radius)
                for a in (math.radians(30 + i * 120 / leaves) for i in range(leaves + 1))]
        draw.line([px(x, y) for x, y in stem[:-1]], fill=255, width=round(0.022 * S), joint="curve")
        for i in range(leaves):
            (x0, y0), (x1, y1) = stem[i], stem[i + 1]
            heading = math.degrees(math.atan2(y1 - y0, x1 - x0))
            length = size * (1 - 0.35 * i / leaves)
            turns = (0,) if i == leaves - 1 else (-42, 42)
            base = (x0, y0) if i == leaves - 1 else (x0 + (x1 - x0) * 0.5, y0 + (y1 - y0) * 0.5)
            for turn in turns:
                a = math.radians(heading + turn)
                leaf(draw, base[0] + math.cos(a) * length * 0.48, base[1] + math.sin(a) * length * 0.48, length,
                     length * 0.4, heading + turn)


def wing(draw, side, cx, cy, span, feathers=3):
    """A swept wing of straight-cut feathers reaching out from (cx, cy)."""
    for i in range(feathers):
        reach = span * (1 - i * 0.2)
        y = cy + i * 0.06
        draw.polygon([px(cx, y), px(cx + side * reach, y - 0.07 + i * 0.012),
                      px(cx + side * (reach - 0.03), y - 0.02 + i * 0.012), px(cx, y + 0.045)], fill=255)


def oak_leaf(draw):
    """A leaf outline whose edge swells into four and a half rounded lobes a side, on a stem."""
    top, bottom = 0.13, 0.7
    left, right = [], []
    for i in range(121):
        t = i / 120
        half = 0.21 * math.sin(math.pi * t) ** 0.7 * (0.6 + 0.4 * abs(math.sin(math.pi * 4.5 * t)))
        y = top + (bottom - top) * t
        left.append(px(0.5 - half, y))
        right.append(px(0.5 + half, y))
    draw.polygon(left + right[::-1], fill=255)
    draw.rounded_rectangle([px(0.48, bottom - 0.04), px(0.52, 0.86)], radius=0.012 * S, fill=255)


def insignia_layers(insignia, metal):
    """A grade's insignia as (mask, metal) layers; overlapping parts get their own layer."""
    kind = insignia[0]
    mask, draw = layer()
    layers = [(mask, metal)]
    if kind == "chevrons":
        count, rockers = insignia[1], insignia[2]
        device = insignia[3] if len(insignia) > 3 else None
        chevron_step, rocker_step = 0.11, 0.08
        height = (count - 1) * chevron_step + 0.22 + (0.14 if device else 0.06 if rockers else 0) + rockers * rocker_step
        y = 0.5 - height / 2
        for _ in range(count):
            chevron(draw, 0.5, y, 0.6, 0.15, 0.065)
            y += chevron_step
        y += 0.22 - chevron_step
        if device == "diamond":
            draw.polygon([px(0.5, y - 0.005), px(0.555, y + 0.06), px(0.5, y + 0.125), px(0.445, y + 0.06)], fill=255)
        elif device == "star":
            draw.polygon(star_points(0.5, y + 0.065, 0.08), fill=255)
        y += 0.14 if device else 0.06
        for _ in range(rockers):
            rocker(draw, y - 0.06, 0.6, 0.065, 0.055)
            y += rocker_step
    elif kind == "bars":
        count = insignia[1]
        for i in range(count):
            x = 0.5 + (i - (count - 1) / 2) * 0.2
            draw.rounded_rectangle([px(x - 0.065, 0.2), px(x + 0.065, 0.8)], radius=0.02 * S, fill=255)
    elif kind == "leaf":
        oak_leaf(draw)
    elif kind == "eagle":
        for side in (-1, 1):
            wing(draw, side, 0.5, 0.36, 0.42, 4)
        body, body_draw = layer()
        body_draw.polygon([px(0.5, 0.22), px(0.565, 0.34), px(0.545, 0.66), px(0.5, 0.74), px(0.455, 0.66),
                           px(0.435, 0.34)], fill=255)
        layers.append((body, metal))
    elif kind == "stars":
        count = insignia[1]
        positions = {1: [(0.5, 0.5)], 2: [(0.3, 0.5), (0.7, 0.5)],
                     3: [(0.2, 0.5), (0.5, 0.5), (0.8, 0.5)],
                     4: [(0.3, 0.3), (0.7, 0.3), (0.3, 0.7), (0.7, 0.7)]}[count]
        radius = {1: 0.3, 2: 0.19, 3: 0.14, 4: 0.17}[count]
        for x, y in positions:
            draw.polygon(star_points(x, y + radius * 0.06, radius), fill=255)
    elif kind == "laurel":
        laurel(draw, 0.5, 0.5, 0.34)
        star, star_draw = layer()
        star_draw.polygon(star_points(0.5, 0.5, 0.2), fill=255)
        layers.append((star, metal))
    return layers


def emblem(draw, chevrons, star, cx, cy):
    """The tier emblem: a star over up to three chevrons, centred on (cx, cy)."""
    step = 0.095
    if star and not chevrons:
        draw.polygon(star_points(cx, cy + 0.015, 0.19), fill=255)
        return
    height = (0.18 if star else 0) + (chevrons - 1) * step + 0.15
    y = cy - height / 2
    if star:
        draw.polygon(star_points(cx, y + 0.08, 0.1), fill=255)
        y += 0.18
    for _ in range(chevrons):
        chevron(draw, cx, y, 0.32, 0.085, 0.065)
        y += step


def tier_layers(chevrons, star, rim, metal, wings, crest):
    cx, cy = 0.5, 0.5
    layers = []
    if wings:
        mask, draw = layer()
        for side in (-1, 1):
            wing(draw, side, cx + side * 0.22, 0.43, 0.25)
        layers.append((mask, rim))
    badge, draw = layer()
    draw.polygon(hexagon(cx, cy, 0.34), fill=255)
    if crest:
        draw.polygon(star_points(cx, 0.15, 0.09), fill=255)
    layers.append((badge, rim))
    face, draw = layer()
    draw.polygon(hexagon(cx, cy, 0.27), fill=255)
    layers.append((face, ENAMEL))
    mark, draw = layer()
    emblem(draw, chevrons, star, cx, cy)
    layers.append((mark, metal))
    return layers


def render(layers):
    """Outlines everything, then paints each layer as a flat face inside a darker bevelled rim."""
    union = Image.new("L", (S, S))
    for mask, _ in layers:
        union = ImageChops.lighter(union, mask)
    image = Image.new("RGBA", (S, S), OUTLINE + (0,))
    image.putalpha(grow(union, STROKE))
    for mask, (face, rim) in layers:
        image.paste(rim + (255,), mask=mask)
        image.paste(face + (255,), mask=grow(mask, -RIM))
    return image.resize((SIZE, SIZE), Image.Resampling.LANCZOS)


def icons():
    """Yields (material name, image) for every level, in level order."""
    for name, insignia, metal in GRADES:
        image = render(insignia_layers(insignia, metal))
        for level in range(1, 4):
            yield f"cu_rank_{name}{level}", image
    name, insignia, metal = COMMANDER
    yield f"cu_rank_{name}1", render(insignia_layers(insignia, metal))
    for name, *design in TIERS:
        image = render(tier_layers(*design))
        for level in range(1, 5):
            yield f"cu_rank_{name}{level}", image


# (seal rim, seal emblem) for prestiges 1-3, 4-6 and 7-9; prestige 10 has its own.
PRESTIGE_BANDS = [(STEEL, LIME), (LIME, STEEL), (GOLD, STEEL)]


def seal(draw, teeth, cx, cy):
    points = []
    for i in range(teeth * 2):
        radius = 0.41 if i % 2 == 0 else 0.36
        angle = math.pi * 2 * i / (teeth * 2) - math.pi / 2
        points.append(px(cx + math.cos(angle) * radius, cy + math.sin(angle) * radius))
    draw.polygon(points, fill=255)


def prestige_icons():
    """Yields (material name, image) for prestiges 1-10: one to three stars on a seal whose metal
    steps up every three prestiges, then a laurelled star for prestige 10."""
    cx, cy = 0.5, 0.5
    for prestige in range(1, 11):
        outer, inner = (PLATINUM, GOLD) if prestige == 10 else PRESTIGE_BANDS[(prestige - 1) // 3]
        ring, draw = layer()
        seal(draw, 24 if prestige == 10 else 16, cx, cy)
        face, face_draw = layer()
        face_draw.ellipse([px(cx - 0.29, cy - 0.29), px(cx + 0.29, cy + 0.29)], fill=255)
        mark, draw = layer()
        if prestige == 10:
            laurel(draw, cx, cy, 0.2, 4, 0.1)
            draw.polygon(star_points(cx, cy, 0.11), fill=255)
        else:
            count = (prestige - 1) % 3 + 1
            radius = {1: 0.17, 2: 0.11, 3: 0.09}[count]
            for i in range(count):
                x = cx + (i - (count - 1) / 2) * radius * 1.9
                draw.polygon(star_points(x, cy + radius * 0.08, radius), fill=255)
        yield f"cu_prestige_{prestige}", render([(ring, outer), (face, ENAMEL), (mark, inner)])


def contact_sheet(images, columns):
    sheet = Image.new("RGBA", (SIZE * columns, SIZE * math.ceil(len(images) / columns)), (24, 30, 27, 255))
    for i, image in enumerate(images):
        sheet.alpha_composite(image, ((i % columns) * SIZE, (i // columns) * SIZE))
    return sheet


if __name__ == "__main__":
    import sys
    from pathlib import Path

    out = Path(sys.argv[1] if len(sys.argv) > 1 else "build/art/preview")
    out.mkdir(parents=True, exist_ok=True)
    ranks = list(icons())
    # One of each design: the grades, Commander and the tiers.
    designs = [image for i, (name, image) in enumerate(ranks) if i == 0 or image is not ranks[i - 1][1]]
    prestiges = [image for _, image in prestige_icons()]
    contact_sheet(designs, 10).save(out / "rank_icons.png")
    contact_sheet(prestiges, 5).save(out / "prestige_icons.png")
    print(len(ranks), "rank icons in", len(designs), "designs,", len(prestiges), "prestige emblems")
