import pygame
from sprite import Text

global tiles

tiles = []

class Editor:

    def __init__(self, xs, ys, x, y, pal, scale, screen):
        self.xlen = xs
        self.ylen = ys
        self.x = x
        self.y = y
        self.currentPal = pal
        self.currentCol = 1
        self.scale = scale
        self.screen = screen
        self.data = []
        count = 0
        for i in range(self.ylen * 8):
            self.data.append([])
            for j in range(self.xlen * 8):
                self.data[count].append(0)
            count += 1
        
    def takeMouseInput(self, mx, my):
        mx -= self.x
        my -= self.y
        if not ((0 <= mx < (self.scale * self.xlen * 8)) and (0 <= my < (self.scale * self.ylen * 8))):
            return
        mx = int(mx / self.scale)
        my = int(my / self.scale)
        self.data[my][mx] = self.currentCol

    def clear(self):
        for i in range(len(self.data)):
            for j in range(len(self.data[0])):
                self.data[i][j] = 0

    def drawEditor(self):
        bgCol = (100, 100, 100)
        for i in range(self.ylen * 8):
            for j in range(self.xlen * 8):
                if self.data[i][j] == 0:
                    col = bgCol
                else:
                    col = self.currentPal[self.data[i][j] - 1]
                pygame.draw.rect(self.screen, col, [self.x + j * self.scale, self.y + i * self.scale, self.scale, self.scale])
                if bgCol == (100, 100, 100):
                    bgCol = (150, 150, 150)
                else:
                    bgCol = (100, 100, 100)
            if bgCol == (100, 100, 100):
                    bgCol = (150, 150, 150)
            else:
                bgCol = (100, 100, 100)
    
    def loadData(self, ind):
        ids = []
        for i in tiles:
            ids.append(i.id)
        for i in range(self.ylen):
            for j in range(self.xlen):
                if ind in ids:
                    for k in range(8):
                        temp = tiles[ids.index(ind)].lines[k]
                        for l in range(8):
                            self.data[8 * i + k][8 * j + l] = (temp & 0xC000) >> 14
                            temp = temp << 2
                else:
                    for k in range(8):
                        for l in range(8):
                            self.data[8 * i + k][8 * j + l] = 0
                ind += 1
    
    def saveData(self, index):
        for i in range(self.ylen):
            for j in range(self.xlen):
                arr = []
                for k in range(8):
                    lineval = 0
                    for l in range(8):
                        lineval += self.data[8 * i + k][8 * j + l]
                        lineval = lineval << 2
                    lineval = lineval >> 2
                    arr.append(lineval % 256)
                    arr.append(lineval // 256)
                bits = bytes(arr)
                ci = 0
                found = False
                if len(tiles) == 0:
                    tiles.append(Tile(bits, index))
                else:
                    while ci < len(tiles) and tiles[ci].id <= index:
                        if tiles[ci].id == index:
                            found = True
                            break
                        ci += 1
                    if found:
                        tiles[ci] = Tile(bits, index)
                    elif ci == len(tiles):
                        tiles.append(Tile(bits, index))
                    else:
                        tiles.insert(ci - 1, Tile(bits, index))
                index += 1


class Tile:

    def __init__(self, b, id):
        self.id = id
        self.lines = []
        for i in range(8):
            self.lines.append(int.from_bytes(b[2 * i: 2 * i + 2], 'little'))
        
    def drawTile(self, x, y, pal, scale, screen):
        yoff = y
        for i in range(8):
            mask = 0xC000
            temp = self.lines[i]
            for xoff in range(x, x + 8 * scale, scale):
                col = (mask & temp) >> 14
                if col > 0:
                    pygame.draw.rect(screen, pal[col - 1], [xoff, yoff, scale, scale])
                temp = temp << 2
            yoff += scale
    
    def getBytes(self):
        ret = []
        for i in self.lines:
            ret.append(i % 256)
            ret.append(i // 256)
        return bytes(ret)


def getActiveIDs():
    ret = []
    for i in tiles:
        ret.append(i.id)
    return ret


def getMode(x, y):
    m = []
    cur = 0
    ind = 0
    for i in range(16):
        m.append([])
        for j in range(16):
            m[ind].append(0)
        ind += 1
    cx = 0
    cy = 0
    while cur < 256:
        for i in range(y):
            for j in range(x):
                m[cy][cx] = cur
                cur += 1
                cx += 1
            cy += 1
            cx -= x
        if cx == 16 - x:
            cx = 0
        else:
            cx += x
            cy -= y
    ret = []
    for i in m:
        for j in i:
            ret.append(j)
    return ret


def drawTilesheet(screen, page, x, y, scale, mode, pal):
    bgCol = (100, 100, 100)
    for i in range(128):
        for j in range(128):
            pygame.draw.rect(screen, bgCol, [x + j * scale, y + i * scale, scale, scale])
            if bgCol == (100, 100, 100):
                bgCol = (150, 150, 150)
            else:
                bgCol = (100, 100, 100)
        if bgCol == (100, 100, 100):
            bgCol = (150, 150, 150)
        else:
            bgCol = (100, 100, 100)
    ind = 0
    if len(tiles) == 0:
        return
    for t in tiles:
        if t.id >= page * 256:
            if t.id - (page * 256) > 255:
                return
            break
        ind += 1
    if ind == len(tiles):
        return
    pos = tiles[ind].id - (page * 256)
    while 0 <= pos <= 255:
        realPos = mode.index(pos)
        tiles[ind].drawTile(x + (scale * (realPos % 16) * 8), y + (scale * (realPos // 16) * 8), pal, scale, screen)
        ind += 1
        if ind == len(tiles):
            return
        pos = tiles[ind].id - (page * 256)


def getTileClicked(mx, my, xl, yl):
    mx -= 40
    my -= 40
    mx /= 40
    my /= 40
    if 0 <= mx < 16 and 0 <= my < 16:
        return [abs(mx) - (abs(mx) % xl), abs(my) - (abs(my) % yl)]
    else:
        return -1


def drawCursor(x, y, xm, ym, screen):
    x -= (x % xm)
    y -= (y % ym)
    x *= 40
    y *= 40
    x += 40
    y += 40
    pygame.draw.rect(screen, (255, 0, 0), [x, y, 40 * xm, 2])
    pygame.draw.rect(screen, (255, 0, 0), [x, y + (ym * 40 - 2), 40 * xm, 2])
    pygame.draw.rect(screen, (255, 0, 0), [x, y, 2, 40 * ym])
    pygame.draw.rect(screen, (255, 0, 0), [x + (xm * 40 - 2), y, 2, 40 * ym])


def drawPalette(x, y, pal, cur, screen):
    if cur > 0:
        pygame.draw.rect(screen, (255, 0, 0), [x + 50 * (cur - 1), y, 50, 50])
    for i in range(3):
        pygame.draw.rect(screen, pal[i], [x + 5 + 50 * i, y + 5, 40, 40])


def getColorClicked(mx, my, x, y):
    mx -= x
    my -= y
    if 5 <= my < 45 and 5 <= mx < 145 and 5 <= mx % 50 < 45:
        return (mx // 50) + 1
    else:
        return -1
    

def main():
    numOfTiles = 0
    try:
        with open('spritesheet.bin', 'rb') as file:
            numOfTiles = int.from_bytes(file.read(2), 'little')
            for i in range(numOfTiles):
                tiles.append(Tile(file.read(16), i))
            file.close()
    except:
        with open('spritesheet.bin', 'wb') as file:
            pass
            file.close()
    pygame.init()
    xlen = 1
    ylen = 1
    mode = getMode(xlen, ylen)
    screen = pygame.display.set_mode([1280, 720])
    clock = pygame.time.Clock()
    running = True
    cPos = [0, 0]
    palettes = []
    toInt = lambda x : int.from_bytes(x, 'little')
    palIndex = 0
    page = 0
    try:
        with open('palettes.bin', 'rb') as file:
            length = toInt(file.read(2))
            for i in range(length):
                palettes.append([])
                for j in range(3):
                    palettes[i].append((toInt(file.read(1)), toInt(file.read(1)), toInt(file.read(1))))
            file.close()
    except:
        palettes = [[(255, 0, 0), (0, 255, 0), (0, 0, 255)]]
    palette = palettes[palIndex]
    edit = Editor(xlen, ylen, 900, 100, palette, 32 / (max(xlen, ylen)), screen)
    editingIndex = 0
    if editingIndex in getActiveIDs():
        edit.loadData(editingIndex)
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                edit.saveData(editingIndex + page * 256)
            if event.type == pygame.KEYDOWN:
                resetEditor = False
                if event.key == pygame.K_p:
                    if palIndex != len(palettes) - 1:
                        palIndex += 1
                    else:
                        palIndex = 0
                    palette = palettes[palIndex]
                    edit.currentPal = palette
                elif event.key == pygame.K_o:
                    if palIndex != 0:
                        palIndex -= 1
                    palette = palettes[palIndex]
                    edit.currentPal = palette
                elif event.key == pygame.K_w and ylen != 4:
                    ylen *= 2
                    resetEditor = True
                elif event.key == pygame.K_s and ylen != 1:
                    ylen //= 2
                    resetEditor = True
                elif event.key == pygame.K_d and xlen != 4:
                    xlen *= 2
                    resetEditor = True
                elif event.key == pygame.K_a and xlen != 1:
                    xlen //= 2
                    resetEditor = True
                elif event.key == pygame.K_RIGHT and page != 255:
                    edit.saveData(editingIndex + page * 256)
                    page += 1
                    edit.loadData(editingIndex + page * 256)
                elif event.key == pygame.K_LEFT and page != 0:
                    edit.saveData(editingIndex + page * 256)
                    page -= 1
                    edit.loadData(editingIndex + page * 256)
                elif event.key == pygame.K_ESCAPE:
                    return
                elif event.key == pygame.K_BACKSPACE:
                    edit.clear()
                if resetEditor:
                    edit.saveData(editingIndex + page * 256)
                    mode = getMode(xlen, ylen)
                    editingIndex = mode[int(cPos[1] * 16 + cPos[0])]
                    editingIndex -= editingIndex % (xlen * ylen)
                    oldCol = edit.currentCol
                    edit = Editor(xlen, ylen, 900, 100, palette, 32 / (max(xlen, ylen)), screen)
                    edit.currentCol = oldCol
                    edit.loadData(editingIndex + page * 256)
                    resetEditor = False
        screen.fill((0, 0, 0))
        mouseB = pygame.mouse.get_pressed()
        mouseP = pygame.mouse.get_pos()
        if mouseB[0]:
            edit.takeMouseInput(mouseP[0], mouseP[1])
            tmp = getTileClicked(mouseP[0], mouseP[1], xlen, ylen)
            if tmp != -1:
                cPos = tmp
                realI = mode[int(cPos[1] * 16 + cPos[0])]
                realI = realI - (realI % (xlen * ylen))
                if  realI != editingIndex:
                    edit.saveData(editingIndex + page * 256)
                    editingIndex = realI
                    edit.loadData(editingIndex + page * 256)
            tmp = getColorClicked(mouseP[0], mouseP[1], 900, 375)
            if tmp != -1:
                edit.currentCol = tmp
        if mouseB[2]:
            oldCol = edit.currentCol
            edit.currentCol = 0
            edit.takeMouseInput(mouseP[0], mouseP[1])
            edit.currentCol = oldCol
        idText = Text('ID {}/PALETTE {}'.format(editingIndex + page * 256, palIndex), 900, 450, (255, 255, 255), 3, screen)
        idText.update()
        drawTilesheet(screen, page, 40, 40, 5, mode, palette)
        drawCursor(cPos[0], cPos[1], xlen, ylen, screen)
        drawPalette(900, 375, palette, edit.currentCol, screen)
        edit.drawEditor()
        pygame.display.update()
    pygame.quit()
    bits = []
    for i in range(16):
        bits.append(0)
    bits = bytes(bits)
    blank = Tile(bits, -1)
    ind = len(tiles) - 1
    while ind >= 0 and tiles[ind].lines == blank.lines:
        tiles.pop(ind)
        ind -= 1
    ids = getActiveIDs()
    with open('spritesheet.bin', 'wb') as file:
        pass
        file.close()
    with open('spritesheet.bin', 'wb') as file:
        fileLen = tiles[len(tiles) - 1].id + 1
        file.write(fileLen.to_bytes(2, 'little'))
        for i in range(fileLen):
            if i in ids:
                file.write(tiles[ids.index(i)].getBytes())
            else:
                file.write(blank.getBytes())

if __name__ == "__main__":   
    main()