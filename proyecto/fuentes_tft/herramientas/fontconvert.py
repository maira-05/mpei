# Equivalente en Python de fontconvert (Adafruit GFX): TTF -> header GFXfont
import sys
from PIL import Image, ImageDraw, ImageFont
ttf, pt, name = sys.argv[1], int(sys.argv[2]), sys.argv[3]
DPI = 141                                # mismo DPI que usa fontconvert
px = round(pt * DPI / 72)
font = ImageFont.truetype(ttf, px)
first, last = 0x20, 0x7E
bits, glyphs = [], []
for code in range(first, last + 1):
    c = chr(code)
    adv = round(font.getlength(c))
    x0, y0, x1, y1 = font.getbbox(c, anchor="ls")
    W, H = max(x1 - x0, 1) + 4, max(y1 - y0, 1) + 4
    img = Image.new("1", (W, H), 0)
    d = ImageDraw.Draw(img); d.fontmode = "1"
    d.text((2 - x0, 2 - y0), c, font=font, fill=1, anchor="ls")
    bb = img.getbbox()
    off = len(bits) // 8 if len(bits) % 8 == 0 else None
    # alinear a byte el inicio de cada glifo
    while len(bits) % 8: bits.append(0)
    off = len(bits) // 8
    if bb is None:
        glyphs.append((off, 0, 0, adv, 0, 0, c)); continue
    l, t, r, b = bb
    w, h = r - l, b - t
    xo = x0 - 2 + l; yo = y0 - 2 + t
    for yy in range(t, b):
        for xx in range(l, r):
            bits.append(1 if img.getpixel((xx, yy)) else 0)
    glyphs.append((off, w, h, adv, xo, yo, c))
while len(bits) % 8: bits.append(0)
data = [int("".join(map(str, bits[i:i+8])), 2) for i in range(0, len(bits), 8)]
asc, desc = font.getmetrics()
yadv = asc + desc
out = []
out.append(f"/* {name}: generado desde {ttf.split('/')[-1]} (GNU FreeFont), {pt} pt, 141 DPI,")
out.append(f"   formato GFXfont de Adafruit GFX. Caracteres 0x20-0x7E. */")
out.append(f"const uint8_t {name}Bitmaps[] PROGMEM = {{")
for i in range(0, len(data), 12):
    out.append("  " + ", ".join(f"0x{v:02X}" for v in data[i:i+12]) + ",")
out.append("};\n")
out.append(f"const GFXglyph {name}Glyphs[] PROGMEM = {{")
for (o, w, h, a, xo, yo, c) in glyphs:
    cc = c.replace("\\", "\\\\").replace("*/", "* /")
    out.append(f"  {{ {o:5d}, {w:3d}, {h:3d}, {a:3d}, {xo:4d}, {yo:4d} }},   // 0x{ord(c):02X} '{cc}'")
out.append("};\n")
out.append(f"const GFXfont {name} PROGMEM = {{")
out.append(f"  (uint8_t  *){name}Bitmaps,")
out.append(f"  (GFXglyph *){name}Glyphs,")
out.append(f"  0x{first:02X}, 0x{last:02X}, {yadv} }};")
out.append(f"\n// Aprox. {len(data) + len(glyphs)*7 + 7} bytes")
print("\n".join(out))
