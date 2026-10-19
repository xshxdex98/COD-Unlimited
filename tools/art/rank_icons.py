"""Rank insignia for player levels 1-99.

Every shape is drawn as geometry at 8x size and scaled down, so the icons stay sharp and can be
regenerated after any change here. Levels 1-55 follow CoD4's grades (three levels each, then
Commander); 56-99 are COD Unlimited's own eleven tiers of four levels.
"""
import math

from PIL import Image, ImageChops, ImageDraw, ImageFilter

SIZE = 128
SCALE = 8
S = SIZE * SCALE

# Metals as (top, bottom) of a vertical gradient.
STEEL = ((236, 240, 238), (118, 128, 124))
BRONZE = ((240, 196, 150), (138, 84, 46))
GOLD = ((255, 232, 150), (176, 120, 30))
LIME = ((205, 245, 170), (100, 160, 70))
PLATINUM = ((250, 252, 255), (150, 170, 190))
DARK = ((70, 80, 76), (44, 52, 48))
OUTLINE = (12, 16, 14)

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
    ("2ndlt", ("bars", 1), BRONZE),
    ("1stlt", ("bars", 1), STEEL),
    ("capt", ("bars", 2), STEEL),
    ("maj", ("leaf",), GOLD),
    ("ltcol", ("leaf",), STEEL),
    ("col", ("wings",), STEEL),
    ("bgen", ("stars", 1), STEEL),
    ("majgen", ("stars", 2), STEEL),
    ("ltgen", ("stars", 3), STEEL),
    ("gen", ("stars", 4), STEEL),
]
COMMANDER = ("comm", ("wreath",), GOLD)
TIERS = [
    # (name, glyph, frame metal, glyph metal); four levels each, 56-99.
    ("vanguard", "arrow", STEEL, LIME),
    ("sentinel", "shield", STEEL, LIME),
    ("warden", "shield_star", STEEL, LIME),
    ("hunter", "reticle", STEEL, LIME),
    ("spectre", "diamond", LIME, STEEL),
    ("phantom", "blades", LIME, STEEL),
    ("reaper", "skull_wings", LIME, STEEL),
    ("warlord", "crown", GOLD, STEEL),
    ("titan", "star_ring", GOLD, PLATINUM),
    ("legend", "laurel", GOLD, PLATINUM),
    ("apex", "winged_diamond", PLATINUM, GOLD),
]


def px(x, y):
    return (x * S, y * S)


def star_points(cx, cy, r, inner=0.42, rotation=-90):
    points = []
    for i in range(10):
        radius = r if i % 2 == 0 else r * inner
        angle = math.radians(rotation + i * 36)
        points.append(px(cx + math.cos(angle) * radius, cy + math.sin(angle) * radius))
    return points


def chevron(draw, cy, width, depth, thickness):
    """A V pointing up, centred at x=0.5, its apex at cy."""
    left, right = 0.5 - width / 2, 0.5 + width / 2
    draw.polygon([px(0.5, cy), px(right, cy + depth), px(right, cy + depth + thickness),
                  px(0.5, cy + thickness), px(left, cy + depth + thickness), px(left, cy + depth)], fill=255)


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
    for i in range(24):
        t = math.pi * 2 * i / 24
        u, v = math.cos(t) * length / 2, math.sin(t) * abs(math.sin(t)) ** 0.6 * width / 2
        points.append(px(cx + u * along[0] + v * across[0], cy + u * along[1] + v * across[1]))
    draw.polygon(points, fill=255)


def wreath(draw, cx, cy, radius, count, length, width):
    """Two branches of leaves climbing either side of a circle."""
    for side in (1, -1):
        for i in range(count):
            t = math.radians(-70 + i * 125 / (count - 1))
            x = cx + side * math.cos(t) * radius
            y = cy - math.sin(t) * radius
            tangent = math.degrees(math.atan2(-math.cos(t), -side * math.sin(t)))
            leaf(draw, x, y, length, width, tangent + side * 25)


def diamond(draw, cx, cy, rx, ry):
    draw.polygon([px(cx, cy - ry), px(cx + rx, cy), px(cx, cy + ry), px(cx - rx, cy)], fill=255)


def draw_insignia(new_layer, insignia):
    """Draws a grade's insignia; new_layer() gives a fresh layer, so separate shapes get their own shading."""
    kind = insignia[0]
    draw = new_layer()
    if kind == "chevrons":
        count, rockers = insignia[1], insignia[2]
        device = insignia[3] if len(insignia) > 3 else None
        chevron_step, rocker_step = 0.115, 0.085
        height = (count - 1) * chevron_step + 0.23 + (0.15 if device else 0.07 if rockers else 0) + rockers * rocker_step
        y = 0.44 - height / 2
        for _ in range(count):
            chevron(draw, y, 0.62, 0.16, 0.07)
            y += chevron_step
        y += 0.23 - chevron_step
        if device == "diamond":
            diamond(draw, 0.5, y + 0.06, 0.06, 0.07)
        elif device == "star":
            draw.polygon(star_points(0.5, y + 0.065, 0.085), fill=255)
        y += 0.15 if device else 0.07
        for _ in range(rockers):
            rocker(draw, y - 0.07, 0.62, 0.07, 0.06)
            y += rocker_step
    elif kind == "bars":
        count = insignia[1]
        for i in range(count):
            x = 0.5 + (i - (count - 1) / 2) * 0.22
            draw.rounded_rectangle([px(x - 0.07, 0.2), px(x + 0.07, 0.72)], radius=0.025 * S, fill=255)
    elif kind == "leaf":
        # Five lobes down each side, narrowing to the tip at the top.
        def edge(side):
            for i in range(11):
                t = i / 10
                width = 0.25 * math.sin(math.pi * min(1, t * 1.15)) * (0.82 + 0.18 * math.cos(t * 10 * math.pi))
                yield px(0.5 + side * width, 0.14 + t * 0.56)

        draw.polygon([px(0.5, 0.12), *edge(1), px(0.5, 0.71), *reversed(list(edge(-1)))], fill=255)
        draw.rounded_rectangle([px(0.48, 0.66), px(0.52, 0.8)], radius=0.01 * S, fill=255)
    elif kind == "wings":
        for side in (1, -1):
            for i in range(4):
                y = 0.3 + i * 0.075
                reach = 0.36 - i * 0.05
                draw.polygon([px(0.5 + side * 0.06, y), px(0.5 + side * reach, y - 0.06),
                              px(0.5 + side * (reach - 0.02), y + 0.035), px(0.5 + side * 0.06, y + 0.06)], fill=255)
        diamond(draw, 0.5, 0.42, 0.075, 0.18)
    elif kind == "stars":
        count = insignia[1]
        positions = {1: [(0.5, 0.45)], 2: [(0.3, 0.45), (0.7, 0.45)],
                     3: [(0.22, 0.47), (0.5, 0.38), (0.78, 0.47)],
                     4: [(0.3, 0.3), (0.7, 0.3), (0.3, 0.64), (0.7, 0.64)]}[count]
        radius = {1: 0.3, 2: 0.2, 3: 0.16, 4: 0.16}[count]
        for i, (x, y) in enumerate(positions):
            (draw if i == 0 else new_layer()).polygon(star_points(x, y, radius), fill=255)
    elif kind == "wreath":
        wreath(draw, 0.5, 0.45, 0.33, 5, 0.17, 0.075)
        draw.polygon(star_points(0.5, 0.44, 0.2), fill=255)


def draw_glyph(draw, glyph):
    if glyph == "arrow":
        chevron(draw, 0.27, 0.42, 0.14, 0.09)
        chevron(draw, 0.43, 0.42, 0.14, 0.09)
    elif glyph in ("shield", "shield_star"):
        draw.polygon([px(0.32, 0.27), px(0.68, 0.27), px(0.68, 0.48), px(0.5, 0.64), px(0.32, 0.48)], fill=255)
        if glyph == "shield_star":
            draw.polygon(star_points(0.5, 0.43, 0.11), fill=0)
    elif glyph == "reticle":
        draw.ellipse([px(0.3, 0.25), px(0.7, 0.65)], fill=255)
        draw.ellipse([px(0.35, 0.3), px(0.65, 0.6)], fill=0)
        for x0, y0, x1, y1 in [(0.48, 0.2, 0.52, 0.36), (0.48, 0.54, 0.52, 0.7),
                               (0.25, 0.43, 0.41, 0.47), (0.59, 0.43, 0.75, 0.47)]:
            draw.rectangle([px(x0, y0), px(x1, y1)], fill=255)
    elif glyph == "diamond":
        diamond(draw, 0.5, 0.45, 0.2, 0.22)
        diamond(draw, 0.5, 0.45, 0.11, 0.13)
        draw.polygon([px(0.5, 0.36), px(0.56, 0.45), px(0.5, 0.54), px(0.44, 0.45)], fill=0)
    elif glyph == "blades":
        for side in (1, -1):
            draw.polygon([px(0.5 - side * 0.2, 0.24), px(0.5 - side * 0.14, 0.24),
                          px(0.5 + side * 0.2, 0.62), px(0.5 + side * 0.13, 0.64)], fill=255)
        draw.ellipse([px(0.45, 0.4), px(0.55, 0.5)], fill=255)
    elif glyph == "skull_wings":
        for side in (1, -1):
            for i in range(3):
                y = 0.33 + i * 0.07
                draw.polygon([px(0.5 + side * 0.1, y), px(0.5 + side * (0.36 - i * 0.05), y - 0.05),
                              px(0.5 + side * 0.1, y + 0.05)], fill=255)
        draw.ellipse([px(0.4, 0.27), px(0.6, 0.47)], fill=255)
        draw.rectangle([px(0.44, 0.44), px(0.56, 0.55)], fill=255)
        draw.ellipse([px(0.43, 0.34), px(0.48, 0.39)], fill=0)
        draw.ellipse([px(0.52, 0.34), px(0.57, 0.39)], fill=0)
    elif glyph == "crown":
        draw.polygon([px(0.28, 0.6), px(0.28, 0.32), px(0.39, 0.44), px(0.5, 0.25), px(0.61, 0.44),
                      px(0.72, 0.32), px(0.72, 0.6)], fill=255)
        draw.rectangle([px(0.28, 0.6), px(0.72, 0.65)], fill=255)
    elif glyph == "star_ring":
        draw.polygon(star_points(0.5, 0.42, 0.24, inner=0.38), fill=255)
        draw.ellipse([px(0.43, 0.35), px(0.57, 0.49)], fill=0)
        draw.ellipse([px(0.46, 0.38), px(0.54, 0.46)], fill=255)
    elif glyph == "laurel":
        wreath(draw, 0.5, 0.45, 0.23, 5, 0.12, 0.055)
        draw.polygon(star_points(0.5, 0.43, 0.12), fill=255)
    elif glyph == "winged_diamond":
        diamond(draw, 0.5, 0.42, 0.14, 0.2)
        for side in (1, -1):
            for i in range(3):
                y = 0.32 + i * 0.07
                draw.polygon([px(0.5 + side * 0.12, y), px(0.5 + side * (0.38 - i * 0.06), y - 0.07),
                              px(0.5 + side * (0.34 - i * 0.06), y + 0.01), px(0.5 + side * 0.12, y + 0.05)],
                             fill=255)


def frame_mask():
    """The tier badge: a hexagon pointing up and down, hollow."""
    mask = Image.new("L", (S, S))
    draw = ImageDraw.Draw(mask)
    outer = [px(0.5, 0.06), px(0.86, 0.25), px(0.86, 0.62), px(0.5, 0.82), px(0.14, 0.62), px(0.14, 0.25)]
    inner = [px(0.5, 0.11), px(0.81, 0.28), px(0.81, 0.59), px(0.5, 0.77), px(0.19, 0.59), px(0.19, 0.28)]
    draw.polygon(outer, fill=255)
    draw.polygon(inner, fill=0)
    return mask


def pip_masks(count, total):
    """Small diamonds under the insignia, the first `count` lit: which level of the grade this is."""
    lit, unlit = Image.new("L", (S, S)), Image.new("L", (S, S))
    for i in range(total):
        x = 0.5 + (i - (total - 1) / 2) * 0.075
        diamond(ImageDraw.Draw(lit if i < count else unlit), x, 0.9, 0.026, 0.034)
    return [(lit, LIME), (unlit, DARK)]


def metal(mask, colors):
    """Fill a mask with a vertical metal gradient over its own height, with a soft sheen up top."""
    top, bottom = colors
    box = mask.getbbox()
    gradient = Image.new("RGB", (1, S))
    if box:
        for y in range(S):
            t = min(1.0, max(0.0, (y - box[1]) / max(1, box[3] - box[1] - 1)))
            sheen = 0.12 * max(0.0, 1 - abs(t - 0.3) / 0.2)
            gradient.putpixel((0, y), tuple(min(255, round(a + (b - a) * t + 255 * sheen)) for a, b in zip(top, bottom)))
    layer = gradient.resize((S, S)).convert("RGBA")
    layer.putalpha(mask)
    return layer


def compose(layers):
    """Stack (mask, metal) layers with a dark outline around everything, then scale down."""
    union = Image.new("L", (S, S))
    for mask, _ in layers:
        union = ImageChops.lighter(union, mask)
    outline = union.filter(ImageFilter.GaussianBlur(SCALE * 1.5)).point(lambda v: min(255, v * 6))
    image = Image.new("RGBA", (S, S), OUTLINE + (0,))
    image.putalpha(outline)
    for mask, colors in layers:
        image = Image.alpha_composite(image, metal(mask, colors))
    return image.resize((SIZE, SIZE), Image.Resampling.LANCZOS)


def grade_icon(insignia, colors, level, levels):
    masks = []

    def new_layer():
        masks.append(Image.new("L", (S, S)))
        return ImageDraw.Draw(masks[-1])

    draw_insignia(new_layer, insignia)
    return compose([*((mask, colors) for mask in masks), *pip_masks(level, levels)])


def tier_icon(glyph, frame_colors, glyph_colors, level):
    mask = Image.new("L", (S, S))
    draw_glyph(ImageDraw.Draw(mask), glyph)
    return compose([(frame_mask(), frame_colors), (mask, glyph_colors), *pip_masks(level, 4)])


def icons():
    """Yields (material name, image) for every level, in level order."""
    for name, insignia, colors in GRADES:
        for level in range(1, 4):
            yield f"cu_rank_{name}{level}", grade_icon(insignia, colors, level, 3)
    name, insignia, colors = COMMANDER
    yield f"cu_rank_{name}1", grade_icon(insignia, colors, 1, 1)
    for name, glyph, frame_colors, glyph_colors in TIERS:
        for level in range(1, 5):
            yield f"cu_rank_{name}{level}", tier_icon(glyph, frame_colors, glyph_colors, level)


if __name__ == "__main__":
    import sys
    from pathlib import Path

    out = Path(sys.argv[1] if len(sys.argv) > 1 else "build/art/preview")
    out.mkdir(parents=True, exist_ok=True)
    made = list(icons())
    sheet = Image.new("RGBA", (SIZE * 11, SIZE * math.ceil(len(made) / 11)), (24, 30, 27, 255))
    for i, (name, image) in enumerate(made):
        sheet.alpha_composite(image, ((i % 11) * SIZE, (i // 11) * SIZE))
    sheet.save(out / "rank_icons.png")
    print(len(made), "icons")
