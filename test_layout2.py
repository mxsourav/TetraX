import sys
from PIL import Image, ImageDraw, ImageFont

img = Image.new('RGB', (128, 64), color='black')
draw = ImageDraw.Draw(img)

# Title bubble
title = "Deauther"
tw = 6 * len(title)
tx = (128 - tw) // 2
draw.rounded_rectangle([tx - 4, 0, tx + tw + 4, 13], radius=3, fill='white')
draw.text((tx, 1), title, fill='black')

cW, cH = 36, 48
sW, sH = 24, 38
oW, oH = 18, 28
g = 2
cX = (128 - cW) // 2
bY = 15

xpos, ypos, wpos, hpos = [0]*5, [0]*5, [0]*5, [0]*5

xpos[2], ypos[2], wpos[2], hpos[2] = cX, bY, cW, cH
xpos[1], ypos[1], wpos[1], hpos[1] = cX - sW - g, bY + (cH - sH) // 2, sW, sH
xpos[3], ypos[3], wpos[3], hpos[3] = cX + cW + g, bY + (cH - sH) // 2, sW, sH
xpos[0], ypos[0], wpos[0], hpos[0] = xpos[1] - oW - g, bY + (cH - oH) // 2, oW, oH
xpos[4], ypos[4], wpos[4], hpos[4] = xpos[3] + sW + g, bY + (cH - oH) // 2, oW, oH

for i in range(5):
    draw.rounded_rectangle([xpos[i], ypos[i], xpos[i]+wpos[i], ypos[i]+hpos[i]], radius=2, outline='white', width=1 if i != 2 else 2)
    if i == 2:
        ax = xpos[i] + wpos[i] // 2
        ay = ypos[i] + hpos[i] - 4
        draw.polygon([(ax-3, ay-2), (ax+3, ay-2), (ax, ay+2)], fill='white')

img.save('layout_test2.png')
print("Saved layout_test2.png")
