from PIL import Image
im = Image.open("atlas.png")
pixels = list(im.getdata())
width, height = im.size

text = '{ '
x = 0
for pel in pixels:
    text += "0x%0.2x, " % pel[0]
    x += 1
    if x == width:
        x = 0
        text += '},\n{ '

text = text[:-2]

print(text)