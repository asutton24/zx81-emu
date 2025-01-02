import pygame
from sprite import Text


def drawGrayscale(screen):
    pygame.draw.rect(screen, (255, 255, 255), [49, 49, 258, 258])
    for i in range(256):
        pygame.draw.rect(screen, (i, i, i), [50, 50 + i, 256, 1])


def drawGradient(c1, c2, val, screen):
    pygame.draw.rect(screen, (255, 255, 255), [49, 49, 258, 258])
    for x in range(256):
        for y in range(256):
            r = [50 + x, 50 + y, 1, 1]
            if c1 == 'r':
                if c2 == 'g':
                    pygame.draw.rect(screen, (x, y, val), r)
                else:
                    pygame.draw.rect(screen, (x, val, y), r)
            elif c1 == 'g':
                if c2 == 'r':
                    pygame.draw.rect(screen, (y, x, val), r)
                else:
                    pygame.draw.rect(screen, (val, x, y), r)
            else:
                if c2 == 'r':
                    pygame.draw.rect(screen, (y, val, x), r)
                else:
                    pygame.draw.rect(screen, (val, y, x), r)


def drawSlider(col, pos, screen):
    pygame.draw.rect(screen, (255, 255, 255), [330, 49, 30, 1])
    pygame.draw.rect(screen, (255, 255, 255), [330, 307, 30, 1])
    pygame.draw.rect(screen, (255, 255, 255), [344, 49, 2, 258])
    if col == 'r':
        c = (pos, 0, 0)
    elif col == 'g':
        c = (0, pos, 0)
    else:
        c = (0, 0, pos)
    pygame.draw.rect(screen, c, [330, 50 + pos, 30, 1])


def drawCurrentPalette(pal, cur, screen):
    pygame.draw.rect(screen, (255, 255, 255), [250 + cur * 50, 400, 50, 50])
    for i in range(3):
        pygame.draw.rect(screen, pal[i], [305 + i * 50, 405, 40, 40])


def switchOrder(o):
    return o[2] + o[0:2]


def main():
    palettes = []
    toInt = lambda x : int.from_bytes(x, 'little')
    try:
        with open('palettes.bin', 'rb') as file:
            length = toInt(file.read(2))
            for i in range(length):
                palettes.append([])
                for j in range(3):
                    palettes[i].append((toInt(file.read(1)), toInt(file.read(1)), toInt(file.read(1))))
            file.close()
    except:
        with open('palettes.bin', 'wb') as file:
            pass
            file.close
        palettes.append([(0, 0, 0), (0, 0, 0), (0, 0, 0)])
    running = True
    pygame.init()
    screen = pygame.display.set_mode([500, 500])
    palIndex = 0
    currentPal = palettes[0]
    current = 1
    pos = [0, 0]
    slider = 0
    order = 'rgb'
    grayscale = False
    labels = [Text(order[0], 166, 312, (255, 255, 255), 3, screen),
              Text(order[1], 20, 166, (255, 255, 255), 3, screen),
              Text(order[2], 366, 166, (255, 255, 255), 3, screen)]
    idText = Text("Id {}".format(palIndex), 175, 375, (255, 255, 255), 3, screen)
    manual = False
    numToEdit = 'r'
    editVal = 0
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN and not manual:
                if 0 <= event.key - pygame.K_1 <= 2:
                    current = event.key - pygame.K_1 + 1
                elif event.key == pygame.K_SPACE:
                    order = switchOrder(order)
                    labels = [Text(order[0], 166, 312, (255, 255, 255), 3, screen),
                                Text(order[1], 20, 166, (255, 255, 255), 3, screen),
                                Text(order[2], 366, 166, (255, 255, 255), 3, screen)]
                elif event.key == pygame.K_u:
                    grayscale = not grayscale
                elif event.key == pygame.K_RIGHT:
                    palIndex += 1
                    if palIndex == 65536:
                        palIndex = 0
                    if palIndex == len(palettes):
                        palettes.append([(0, 0, 0), (0, 0, 0), (0, 0, 0)])
                    currentPal = palettes[palIndex]
                    idText = Text("Id {}".format(palIndex), 175, 375, (255, 255, 255), 3, screen)
                elif event.key == pygame.K_LEFT:
                    if palIndex != 0:
                        palIndex -= 1
                    currentPal = palettes[palIndex]
                    idText = Text("Id {}".format(palIndex), 175, 375, (255, 255, 255), 3, screen)
                elif event.key == pygame.K_ESCAPE:
                    return
                elif event.key == pygame.K_r:
                    manual = True
                    numToEdit = 'r'
                    editVal = 0
                elif event.key == pygame.K_g:
                    manual = True
                    numToEdit = 'g'
                    editVal = 0
                elif event.key == pygame.K_b:
                    manual = True
                    numToEdit = 'b'
                    editVal = 0
            elif event.type == pygame.KEYDOWN and manual:
                if event.key == pygame.K_RETURN:
                    manual = False
                elif pygame.K_0 <= event.key <= pygame.K_9:
                    editVal *= 10
                    editVal += event.key - pygame.K_0
                    if editVal >= 100:
                        manual = False
                    if editVal <= 255:
                        if numToEdit == 'r':
                            currentPal[current - 1] = (editVal, currentPal[current - 1][1], currentPal[current - 1][2])
                        elif numToEdit == 'g':
                            currentPal[current - 1] = (currentPal[current - 1][0], editVal, currentPal[current - 1][2])
                        elif numToEdit == 'b':
                            currentPal[current - 1] = (currentPal[current - 1][0], currentPal[current - 1][1], editVal)
        screen.fill((0, 0, 0))
        mouseB = pygame.mouse.get_pressed()
        mouseP = pygame.mouse.get_pos()
        if mouseB[0] and not manual:
            change = False
            if 0 <= mouseP[0] - 50 < 256 and 0 <= mouseP[1] - 50 < 256:
                pos[0] = mouseP[0] - 50
                pos[1] = mouseP[1] - 50
                change = True
            elif (not grayscale) and 0 <= mouseP[0] - 330 < 30 and 0 <= mouseP[1] - 50 < 256:
                slider = mouseP[1] - 50
            if change:
                if grayscale:
                    currentPal[current - 1] = (pos[1], pos[1], pos[1])
                elif order == 'rgb':
                    currentPal[current - 1] = (pos[0], pos[1], slider)
                elif order == 'brg':
                    currentPal[current - 1] = (pos[1], slider, pos[0])
                else:
                    currentPal[current - 1] = (slider, pos[0], pos[1])
        if grayscale:
            drawGrayscale(screen)
        else:
            drawGradient(order[0], order[1], slider, screen)
            drawSlider(order[2], slider, screen)
            for i in labels:
                i.update()
        valText = Text("R {}/G {}/B {}".format(currentPal[current - 1][0], currentPal[current - 1][1], currentPal[current - 1][2]), 50, 375, (255, 255, 255), 3, screen)
        valText.update()
        idText.update()
        drawCurrentPalette(currentPal, current, screen)
        pygame.display.update()
    if len(palettes) > 2:
        lastEmpty = len(palettes) - 1
        while palettes[lastEmpty] == [(0, 0, 0), (0, 0, 0), (0, 0, 0)]:
            lastEmpty -= 1
        lastEmpty += 1
        while len(palettes) > lastEmpty:
            palettes.pop(lastEmpty)
    with open('palettes.bin', 'wb') as file:
        pass
        file.close()
    with open('palettes.bin', 'wb') as file:
        file.write(len(palettes).to_bytes(2, 'little'))
        for pal in palettes:
            for col in pal:
                for i in range(3):
                    file.write(col[i].to_bytes(1, 'little'))

main()
