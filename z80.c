#include "memory.h"

dbyte pc = 0;
dbyte sp = 0;
dbyte ix = 0;
dbyte iy = 0;
byte regs[7];
byte altRegs[7];
byte ioIN[256];
byte ioOUT[256];
byte f = 0;
byte af = 0;
byte in = 0;
byte re = 0;
byte iff1 = 0;
byte iff2 = 0;
Memory* mem = NULL;
byte halt = 0;
byte intMode = 0;
byte intComplete = 0;
byte interrupting = 0;
byte intByte = 0;
byte lastIO = 0;
byte readFromMem = 1;
byte altByte = 0;

byte interrupt(byte n){
    if (iff1){
        interrupting = 1;
        intByte = n;
    }
    halt = 0;
    return 0;
}

byte nmi(){
    interrupting = 2;
    halt = 0;
    return 0;
}

byte readRegisters(byte x, char set){
    byte* r;
    if (set == 'r'){
        r = regs;
    } else if (set == 'a'){
        r = altRegs;
    } else {
        return 0;
    }
    return r[(x + 1) & 7];
}

byte readR(byte x){
    return readRegisters(x, 'r');
}

byte readA(byte x){
    return readRegisters(x, 'a');
}

byte writeRegisters(byte x, byte val, char set){
    byte* r;
    if (set == 'r'){
        r = regs;
    } else if (set == 'a'){
        r = altRegs;
    } else {
        return 0;
    }
    r[(x + 1) & 7] = val;
    return 0;
}

byte writeR(byte x, byte val){
    return writeRegisters(x, val, 'r');
}

byte writeA(byte x, byte val){
    return writeRegisters(x, val, 'a');
}

dbyte readreg16(char reg, char set){
    byte* r;
    if (set == 'r'){
        r = regs;
    } else if (set == 'a'){
        r = altRegs;
    } else {
        return 0;
    }
    switch(reg){
        case 'b':
            return r[1] * 256 + r[2];
        case 'd':
            return r[3] * 256 + r[4];
        case 'h':
            return r[5] * 256 + r[6];
    }
    return 0;
}

int writereg16(char reg, char set, dbyte val){
    byte* r;
    if (set == 'r'){
        r = regs;
    } else if (set == 'a'){
        r = altRegs;
    } else {
        return 0;
    }
    switch (reg){
        case 'b':
            r[1] = val >> 8;
            r[2] = val % 256;
            break;
        case 'd':
            r[3] = val >> 8;
            r[4] = val % 256;
            break;
        case 'h':
            r[5] = val >> 8;
            r[6] = val % 256;
            break;
    }
    return 0;
}

dbyte readR16(char reg){
    return readreg16(reg, 'r');
}

dbyte readA16(char reg){
    return readreg16(reg, 'a');
}

int writeR16(char reg, dbyte val){
    return writereg16(reg, 'r', val);
}

int writeA16(char reg, dbyte val){
    return writereg16(reg, 'a', val);
}

byte rhash(char reg){
    switch (reg){
        case 'a':
            return 7;
        case 'b':
            return 0;
        case 'c':
            return 1;
        case 'd':
            return 2;
        case 'e':
            return 3;
        case 'h':
            return 4;
        case 'l':
            return 5;
        default:
            return 255;
    }
}

char r16hash(byte reg){
    switch (reg){
        case 0:
            return 'b';
        case 1:
            return 'd';
        case 2:
            return 'h';
        default:
            return -1;
    }
}

int setFlag(char flag, byte val){
    if (val != 0) val = 1;
    switch (flag){
        case 's':
            f = (f & 127) + (val * 128);
            break;
        case 'z':
            f = (f & 191) + (val * 64);
            break;
        case 'h':
            f = (f & 239) + (val * 16);
            break;
        case 'p':
            f = (f & 251) + (val * 4);
            break;
        case 'v':
            f = (f & 251) + (val * 4);
            break;
        case 'n':
            f = (f & 253) + (val * 2);
            break;
        case 'c':
            f = (f & 254) + val;
            break;
    }
    return 0;
}

byte readFlag(char flag){
    byte ret = 0;
    switch (flag){
        case 's':
            ret = f & 128;
            break;
        case 'z':
            ret = f & 64;
            break;
        case 'h':
            ret = f & 16;
            break;
        case 'p':
            ret = f & 4;
            break;
        case 'v':
            ret = f & 4;
            break;
        case 'n':
            ret = f & 2;
            break;
        case 'c':
            ret = f & 1;
            break;
    }
    return (ret != 0);
}

int setAllFlags(byte s, byte z, byte h, byte pv, byte n, byte c){
    setFlag('s', s);
    setFlag('z', z);
    setFlag('h', h);
    setFlag('p', pv);
    setFlag('n', n);
    setFlag('c', c);
    return 0;
}

int r16math(char reg, dbyte val, char sign){
    if (sign == '+'){
        writeR16(reg, readR16(reg) + val);
    } else if (sign == '-'){
        writeR16(reg, readR16(reg) - val);
    }
    return 0;
}

byte getParity(byte n){
    byte p = 0;
    for (int i = 0; i < 8; i++){
        p += (n & 1);
        n = n >> 1;
    }
    return p;
}

dbyte acm(byte num, char sign, byte carry){
    byte ftemp = f;
    dbyte ret = 0;
    byte a = regs[0];
    byte old = a;
    switch (sign){
        case '+':
            a += num;
            if (carry) a += 1;
            setFlag('h', (old & 15) + (num & 15) + carry > 15);
            setFlag('v', ((old & 128) && ((num + carry) & 128) && !(a & 128)) || (!(old & 128) && !((num + carry) & 128) && (a & 128)));
            setFlag('n', 0);
            setFlag('c', old + num + carry > 255);
            break;
        case '-':
            a -= num;
            if (carry) a -= 1;
            setFlag('h', (old & 15) - (num & 15) - carry < 0);
            setFlag('v', ((old & 128) && !((num - carry) & 128) && !(a & 128)) || (!(old & 128) && ((num - carry) & 128) && (a & 128)));
            setFlag('n', 0);
            setFlag('c', old - num - carry < 0);
            break;
        case '&':
            a = a & num;
            setFlag('h', 1);
            setFlag('p', !(getParity(a) % 2));
            setFlag('n', 0);
            setFlag('c', 0);
            break;
        case '|':
            a = a | num;
            setFlag('h', 0);
            setFlag('p', !(getParity(a) % 2));
            setFlag('n', 0);
            setFlag('c', 0);
            break;
        case '^':
            a = a ^ num;
            setFlag('h', 0);
            setFlag('p', !(getParity(a) % 2));
            setFlag('n', 0);
            setFlag('c', 0);
            break;
        default:
            return 0xFFFF;
    }
    setFlag('s', a & 128);
    setFlag('z', a == 0);
    ret = a + f * 256;
    f = ftemp;
    return ret;
}

byte rollByte(byte num, char dir, byte c){
    setFlag('n', 0);
    setFlag('h', 0);
    byte oldC;
    if (dir == 'l'){
        if (c){
            setFlag('c', num & 128);
            num = (num << 1) + readFlag('c');
        } else {
            oldC = readFlag('c');
            setFlag('c', num & 128);
            num = (num << 1) + oldC;
        }
    } else if (dir == 'r'){
        if (c){
            setFlag('c', num & 1);
            num = (num >> 1) + 128 * readFlag('c');
        } else {
            oldC = readFlag('c');
            setFlag('c', num & 1);
            num = (num >> 1) + 128 * oldC;
        }
    }
    return num;
}

byte rollByteFlags(byte num, char dir, byte c){
    byte ret = rollByte(num, dir, c);
    setFlag('s', ret & 128);
    setFlag('z', ret == 0);
    setFlag('p', getParity(ret) % 2 == 0);
    return ret;
}

int aMath(byte num, char sign, byte includeCarry){
    dbyte temp = acm(num, sign, readFlag('c') * includeCarry);
    writeR(rhash('a'), temp % 256);
    f = temp / 256;
    return 0;
}

int acmOperations(byte num, byte mode){
    switch (mode){
        case 0:
            aMath(num, '+', 0);
            break;
        case 1:
            aMath(num, '+', 1);
            break;
        case 2:
            aMath(num, '-', 0);
            break;
        case 3:
            aMath(num, '-', 1);
            break;
        case 4:
            aMath(num, '&', 0);
            break;
        case 5:
            aMath(num, '^', 0);
            break;
        case 6:
            aMath(num, '|', 0);
            break;
        case 7:
            f = acm(num, '-', 0) / 256;
            break;
        default:
            return -1;
    }
    return 0;
}

byte inc_dec(byte num, char sign){
    if (sign == '+'){
        num++;
        setAllFlags(num & 128, num == 0, num % 16 == 0, num == 0x80, 0, readFlag('c'));
        return num;
    } else if (sign == '-'){
        num--;
        setAllFlags(num & 128, num == 0, num % 16 == 15, num == 0x7F, 1, readFlag('c'));
    }
    return num;
}

int memSetFullRam(){
    if (mem != NULL){
        freeMemory(mem);
        mem = NULL;
    }
    mem = (Memory*)malloc(sizeof(Memory));
    mem->head = NULL;
    addBankList(mem, 0, 0xFFFF);
    addBankAt(mem, 0, 0xFFFF, 'w');
    return 0;
}

int misccmds(){
    dbyte temp, temp2;
    byte tempB;
    int cycles = 0;
    pc++;
    byte op = read(mem, pc);
    byte high = op >> 6;
    byte mid = (op >> 3) & 7;
    byte low = op & 7;
    if (high == 1){
        if (mid == 2 && low == 7){
            writeR(rhash('a'), in);
            setAllFlags(in & 128, in == 0, 0, iff2, 0, f & 1);
            cycles = 9;
        } else if (mid == 3 && low == 7){
            writeR(rhash('a'), re);
            setAllFlags(re & 128, re == 0, 0, iff2, 0, f & 1);
            cycles = 9;
        } else if (mid == 0 && low == 7){
            in = readR(rhash('a'));
            cycles = 9;
        } else if (mid == 1 && low == 7){
            re = readR(rhash('a'));
            cycles = 9;
        } else if (low == 3 && mid % 2 == 1){
            if (mid / 2 == 3) sp = read16(mem, read16(mem, pc + 1));
            else writeR16(r16hash(mid / 2), read16(mem, read16(mem, pc + 1)));
            pc += 2;
            cycles = 20;
        } else if (low == 3){
            if (mid / 2 == 3) write16(mem, read16(mem, pc + 1), sp);
            else write16(mem, read16(mem, pc + 1), readR16(r16hash(mid / 2)));
            pc += 2;
            cycles = 20;
        } else if (mid == 0 && low == 4){
            tempB = readR(rhash('a'));
            writeR(rhash('a'), 0);
            acmOperations(tempB, 2);
            setFlag('v', tempB == 0x80);
            cycles = 8;
        } else if (mid == 0 && low == 6){
            intMode = 0;
            cycles = 8;
        } else if (mid == 2 && low == 6){
            intMode = 1;
            cycles = 8;
        } else if (mid == 3 && low == 6){
            intMode = 2;
            cycles = 8;
        } else if (low == 2 && mid % 2 == 1){
            temp = readR16('h');
            if (mid / 2 == 3) temp2 = sp;
            else temp2 = readR16(r16hash(mid / 2));
            temp2 += readFlag('c');
            r16math('h', temp2, '+');
            tempB = (!(temp & 32768) && !(temp2 & 32768) && (readR16('h') & 32768)) || ((temp & 32768) && (temp2 & 32768) && !(readR16('h') & 32768));
            setAllFlags(((readR16('h') & 32768) != 0), readR16('h') == 0, ((temp % 4096) + (temp2 % 4096) > 4095), tempB, 0, (temp + temp2) > 0xFFFF);
            cycles = 15;
        } else if (low == 2){
            temp = readR16('h');
            if (mid / 2 == 3) temp2 = sp;
            else temp2 = readR16(r16hash(mid / 2));
            temp2 -= readFlag('c');
            r16math('h', temp2, '-');
            tempB = (!(temp & 32768) && (temp2 & 32768) && (readR16('h') & 32768)) || ((temp & 32768) && !(temp2 & 32768) && !(readR16('h') & 32768));
            setAllFlags(((readR16('h') & 32768) != 0), readR16('h') == 0, ((temp % 4096) - (temp2 % 4096) < 0), tempB, 1, (temp - temp2) < 0);
            cycles = 15;
        } else if (mid == 5 && low == 7){
            temp = readR(rhash('a'));
            temp2 = read(mem, readR16('h'));
            tempB = readR(rhash('a'));
            temp = ((temp & 0xF0) + ((temp2 & 0xF0) >> 4)) & 0xFF;
            temp2 = (((temp2 & 0xF) << 4) + (tempB & 0xF)) & 0xFF;
            writeR(rhash('a'), temp);
            write(mem, readR16('h'), temp2);
            tempB = temp;
            setAllFlags(tempB & 128, tempB == 0, 0, getParity(tempB) % 2 == 0, 0, readFlag('c'));
            cycles = 18;
        } else if (mid == 4 && low == 7){
            temp = readR(rhash('a'));
            temp2 = read(mem, readR16('h'));
            tempB = readR(rhash('a'));
            temp = ((temp & 0xF0) + ((temp2 & 0xF))) & 0xFF;
            temp2 = (((temp2 & 0xF0) >> 4) + ((tempB & 0xF) << 4)) & 0xFF;
            writeR(rhash('a'), temp);
            write(mem, readR16('h'), temp2);
            tempB = temp;
            setAllFlags(tempB & 128, tempB == 0, 0, getParity(tempB) % 2 == 0, 0, readFlag('c'));
            cycles = 18;
        } else if (mid == 1 && low == 5){
            pc = read16(mem, sp) - 1;
            sp += 2;
            cycles = 14;
            intComplete = 1;
        } else if (mid == 0 && low == 5){
            pc = read16(mem, sp) - 1;
            sp += 2;
            cycles = 14;
            iff1 = iff2;
            intComplete = 1;
        } else if (mid != 6 && low == 0){
            setAllFlags(ioIN[readR(rhash('c'))] & 128, ioIN[readR(rhash('c'))] == 0, 0, getParity(ioIN[readR(rhash('c'))]) % 2 == 0, 0, readFlag('c'));
            writeR(mid, ioIN[readR(rhash('c'))]);
            lastIO = 1;
            cycles = 12;
        } else if (mid != 6 && low == 1){
            ioOUT[readR(rhash('c'))] = readR(mid);
            lastIO = 2;
            cycles = 12;
        }
    } else if (high == 2){
        if (mid >= 4 && low == 0){
            cycles = 0;
            char sign = '-';
            if (mid % 2 == 0){
                sign = '+';
            }
            write(mem, readR16('d'), read(mem, readR16('h')));
            r16math('h', 1, sign);
            r16math('d', 1, sign);
            r16math('b', 1, '-');
            if (mid >= 6 && readR16('b') != 0){
                cycles = 21;
                pc -= 2;
            } else {
                cycles = 16;
            }
            setAllFlags(readFlag('s'), readFlag('z'), 0, readR16('b') != 0, 0, readFlag('c'));
        } else if (mid >= 4 && low == 1){
            char sign = '-';
            if (mid % 2 == 0){
                sign = '+';
            }
            if (mid >= 6 && readR(rhash('a')) != read(mem, readR16('h')) && readR16('b') - 1 != 0){
                pc -= 2;
                cycles = 21;
            } else {
                cycles = 16;
            }
            temp = acm(read(mem, readR16('h')), '-', 0) / 256;
            r16math('h', 1, sign);
            r16math('b', 1, '-');
            setAllFlags(temp & 128, temp & 64, temp & 16, readR16('b') != 0, 1, readFlag('c'));
        } else if (mid >= 4 && low == 2){
            char sign = '-';
            if (mid % 2 == 0){
                sign = '+';
            }
            setFlag('n', 1);
            write(mem, readR16('h'), ioIN[readR(rhash('c'))]);
            r16math('h', 1, sign);
            writeR(rhash('b'), readR(rhash('b')) - 1);
            setFlag('z', readR(rhash('b')) == 0);
            if (mid >= 6 && !readFlag('z')){
                pc -= 2;
                cycles = 21;
            } else {
                cycles = 16;
            }
            lastIO = 1;
        } else if (mid >= 4 && low == 3){
            char sign = '-';
            if (mid % 2 == 0){
                sign = '+';
            }
            setFlag('n', 1);
            ioOUT[readR(rhash('c'))] = read(mem, readR16('h'));
            r16math('h', 1, sign);
            writeR(rhash('b'), readR(rhash('b')) - 1);
            setFlag('z', readR(rhash('b')) == 0);
            if (mid >= 6 && !readFlag('z')){
                pc -= 2;
                cycles = 21;
            } else {
                cycles = 16;
            }
            lastIO = 2;
        }
    }
    pc++;
    re = (re & 128) + ((re + 1) & 127);
    return cycles;
}

int indexbitcmds(dbyte index){
    int cycles = 0;
    pc++;
    char off = read(mem, pc);
    pc++;
    byte op = read(mem, pc);
    byte low = op % 16;
    byte high = op / 16;
    if (op >> 6 == 1){
        byte b = (op >> 3) & 7;
        setFlag('h', 1);
        setFlag('n', 0);
        setFlag('z', (read(mem, index + off) & (1 << b)) == 0);
        cycles = 20;
    } else if (op >> 6 == 2) {
        byte b = (op >> 3) & 7;
        write(mem, index + off, read(mem, index + off) & (255 - (1 << b)));
        cycles = 23;
    } else if (op >> 6 == 3) {
        byte b = (op >> 3) & 7;
        write(mem, index + off, read(mem, index + off) | (1 << b));
        cycles = 23;
    } else if (low == 6){
        if (high == 0){
            write(mem, index + off, rollByteFlags(read(mem, index + off), 'l', 1));
            cycles = 23;
        } else if (high == 1){
            write(mem, index + off, rollByteFlags(read(mem, index + off), 'l', 0));
            cycles = 23;
        } else if (high == 2){
            setFlag('c', 0);
            write(mem, index + off, rollByteFlags(read(mem, index + off), 'l', 0));
            cycles = 23;
        }
    } else if (low == 14){
        if (high == 0){
            write(mem, index + off, rollByteFlags(read(mem, index + off), 'r', 1));
            cycles = 23;
        } else if (high == 1){
            write(mem, index + off, rollByteFlags(read(mem, index + off), 'r', 0));
            cycles = 23;
        } else if (high == 2){
            setFlag('c', 0);
            write(mem, index + off, (read(mem, index + off) & 128) + rollByte(read(mem, index + off), 'r', 0));
            setFlag('s', read(mem, index + off) & 128);
            setFlag('z', read(mem, index + off) == 0);
            setFlag('p', getParity(read(mem, index + off)) % 2 == 0);
            cycles = 23;
        } else if (high == 3){
            setFlag('c', 0);
            write(mem, index + off, rollByteFlags(read(mem, index + off), 'r', 0));
            cycles = 23;
        }
    }
    pc++;
    re = (re & 128) + ((re + 1) & 127);
    return cycles;
}

int indexcmds(byte mode){
    dbyte* index;
    dbyte temp;
    if (mode == 0xDD){
        index = &ix;
    } else if (mode == 0xFD){
        index = &iy;
    } else {
        return 0;
    }
    int cycles = 0;
    char off;
    pc++;
    byte op = read(mem, pc);
    if (op == 0xCB){
        return indexbitcmds(*index);
    }
    byte high = op >> 6;
    byte mid = (op >> 3) & 7;
    byte low = op & 7;
    if (high == 0){
        if (low == 6 && mid == 6){
            pc++;
            off = read(mem, pc);
            pc++;
            write(mem, *index + off, read(mem, pc));
            cycles = 19;
        } else if (low == 1 && mid == 4){
            *index = read16(mem, pc + 1);
            pc += 2;
            cycles = 14;
        } else if (low == 2 && mid == 5){
            *index = read16(mem, read16(mem, pc + 1));
            pc += 2;
            cycles = 20;
        } else if (low == 2 && mid == 4){
            write16(mem, read16(mem, pc + 1), *index);
            pc += 2;
            cycles = 20;
        } else if (low == 4 && mid == 6){
            pc++;
            write(mem, *index + read(mem, pc), inc_dec(read(mem, *index + read(mem, pc)), '+'));
            cycles = 23;
        } else if (low == 5 && mid == 6){
            pc++;
            write(mem, *index + read(mem, pc), inc_dec(read(mem, *index + read(mem, pc)), '-'));
            cycles = 23;
        } else if (low == 1 && mid % 2 == 1){
            if (mid / 2 == 3) temp = sp;
            else if (mid / 2 == 2) temp = *index;
            else temp = readR16(r16hash(mid / 2));
            setFlag('n', 0);
            setFlag('h', ((*index % 4096) + (temp % 4096)) > 4095);
            setFlag('c', *index + temp > 0xFFFF);
            *index += temp;
            cycles = 15;
        } else if (low == 3 && mid == 4){
            (*index)++;
            cycles = 10;
        }else if (low == 3 && mid == 5){
            (*index)--;
            cycles = 10;
        }
    } else if (high == 1){
        if (low == 6 && mid != 6){
            pc++;
            off = read(mem, pc);
            writeR(mid, read(mem, *index + off));
            cycles = 19;
        } else if (low != 6 && mid == 6){
            pc++;
            off = read(mem, pc);
            write(mem, *index + off, readR(low));
            cycles = 19;
        }
    } else if (high == 2){
        if (low == 6){
            pc++;
            acmOperations(read(mem, *index + read(mem, pc)), mid);
            cycles = 19;
        }
    } else if (high == 3){
        if (low == 1 && mid == 7){
            sp = *index;
            cycles = 10;
        } else if (low == 5 && mid == 4){
            sp -= 2;
            write16(mem, sp, *index);
            cycles = 15;
        } else if (low == 1 && mid == 4){
            *index = read16(mem, sp);
            sp += 2;
            cycles = 14;
        } else if (low == 3 && mid == 4){
            temp = *index;
            *index = read16(mem, sp);
            write16(mem, sp, temp);
            cycles = 23;
        } else if (low == 1 && mid == 5){
            pc = *index - 1;
            cycles = 8;
        }
    }
    re = (re & 128) + ((re + 1) & 127);
    pc++;
    return cycles;
}

int bitcmds(){
    int cycles = 0;
    pc++;
    byte op = read(mem, pc);
    byte high = op >> 6;
    byte mid = (op >> 3) & 7;
    byte low = op & 7;
    if (high == 0){
        if (mid < 4){
            if (low != 6){
                writeR(low, rollByteFlags(readR(low), (mid % 2 == 0) * 'l' + (mid % 2 == 1) * 'r', mid < 2));
                cycles = 8;
            } else {
                write(mem, readR16('h'), rollByteFlags(read(mem, readR16('h')), (mid % 2 == 0) * 'l' + (mid % 2 == 1) * 'r', mid < 2));
                cycles = 15;
            }
        } else if (mid == 4){
            setFlag('c', 0);
            if (low != 6){
                writeR(low, rollByteFlags(readR(low), 'l', 0));
                cycles = 8;
            } else {
                write(mem, readR16('h'), rollByteFlags(read(mem, readR16('h')), 'l', 0));
                cycles = 15;
            }
        } else if (mid == 5){
            setFlag('c', 0);
            if (low != 6){
                writeR(low, (readR(low) & 128) + rollByte(readR(low), 'r', 0));
                setFlag('s', readR(low) & 128);
                setFlag('z', readR(low) == 0);
                setFlag('p', getParity(readR(low)) % 2 == 0);
                cycles = 8;
            } else {
                write(mem, readR16('h'), (read(mem, readR16('h')) & 128) + rollByte(read(mem, readR16('h')), 'r', 0));
                setFlag('s', read(mem, readR16('h')) & 128);
                setFlag('z', read(mem, readR16('h')) == 0);
                setFlag('p', getParity(read(mem, readR16('h'))) % 2 == 0);
                cycles = 15;
            }
        } else if (mid == 7){
            setFlag('c', 0);
            if (low != 6){
                writeR(low, rollByteFlags(readR(low), 'r', 0));
                cycles = 8;
            } else {
                write(mem, readR16('h'), rollByteFlags(read(mem, readR16('h')), 'r', 0));
                cycles = 15;
            }
        } 
    } else if (high == 1){
        setFlag('h', 1);
        setFlag('n', 0);
        if (low != 6){
            setFlag('z', (readR(low) & (1 << mid)) == 0);
            cycles = 8;
        } else {
            setFlag('z', (read(mem, readR16('h')) & (1 << mid)) == 0);
            cycles = 12;
        }
    } else if (high == 2){
        if (low != 6){
            writeR(low, readR(low) & (255 - (1 << mid)));
            cycles = 8;
        } else {
            write(mem, readR16('h'), read(mem, readR16('h')) & (255 - (1 << mid)));
            cycles = 15;
        }
    } else if (high == 3){
        if (low != 6){
            writeR(low, readR(low) | (1 << mid));
            cycles = 8;
        } else {
            write(mem, readR16('h'), read(mem, readR16('h')) | (1 << mid));
            cycles = 15;
        }
    }
    pc++;
    re = (re & 128) + ((re + 1) & 127);
    return cycles;
}

int runcmd(){
    int cycles = 0;
    lastIO = 0;
    byte op;
    if (interrupting == 2){
        sp -= 2;
        write16(mem, sp, pc + 3);
        pc = 0x66;
        re = (re & 128) + ((re + 1) & 127);
        return 11;
    } else if (interrupting == 1){
        if (intMode == 0){
            op = intByte;
        } else if (intMode == 1){
            op = 0xFF;
        } else if (intMode == 2){
            op = read(mem, in * 256 + intByte);
        }
    }
    if (!readFromMem && !interrupting) op = altByte;
    else if (!interrupting) op = read(mem, pc);
    if (op == 0xDD || op == 0xFD){
        return indexcmds(op);
    } else if (op == 0xED){
        return misccmds();
    } else if (op == 0xCB){
        return bitcmds();
    } else if (halt || op == 0){
        pc++;
        re = (re & 128) + ((re + 1) & 127);
        return 4;
    }
    dbyte temp;
    byte high = op >> 6;
    byte mid = (op >> 3) & 7;
    byte low = op & 7;
    if (high == 0){
        if (mid != 6 && low == 6){
            pc++;
            writeR(mid, read(mem, pc));
            cycles = 7;
        } else if (mid == 6 && low == 6){
            pc++;
            write(mem, readR16('h'), read(mem, pc));
            cycles = 10;
        } else if (mid == 1 && low == 2){
            writeR(rhash('a'), read(mem, readR16('b')));
            cycles = 7;
        } else if (mid == 3 && low == 2){
            writeR(rhash('a'), read(mem, readR16('d')));
            cycles = 7;
        } else if (mid == 7 && low == 2){
            writeR(rhash('a'), read(mem, read16(mem, pc + 1)));
            pc += 2;
            cycles = 13;
        } else if (mid == 0 && low == 2){
            write(mem, readR16('b'), readR(rhash('a')));
            cycles = 7;
        } else if (mid == 2 && low == 2){
            write(mem, readR16('d'), readR(rhash('a')));
            cycles = 7;
        } else if (mid == 6 && low == 2){
            write(mem, read16(mem, pc + 1), readR(rhash('a')));
            pc += 2;
            cycles = 13;
        } else if (low == 1 && mid % 2 == 0){
            if (mid / 2 == 3) sp = read16(mem, pc + 1);
            else writeR16(r16hash(mid / 2), read16(mem, pc + 1));
            pc += 2;
            cycles = 10;
        } else if (mid == 5 && low == 2){
            writeR16('h', read16(mem, read16(mem, pc + 1)));
            pc += 2;
            cycles = 16;
        } else if (mid == 4 && low == 2){
            write16(mem, read16(mem, pc + 1), readR16('h'));
            pc += 2;
            cycles = 16;
        } else if (mid == 1 && low == 0){
            temp = readR(rhash('a'));
            writeR(rhash('a'), readA(rhash('a')));
            writeA(rhash('a'), temp);
            temp = f;
            f = af;
            af = temp;
            cycles = 4;
        } else if (low == 4 || low == 5){
            char sign = '-';
            if (low == 4) sign = '+';
            if (mid != 6){
                writeR(mid, inc_dec(readR(mid), sign));
                cycles = 4;
            } else {
                write(mem, readR16('h'), inc_dec(read(mem, readR16('h')), sign));
                cycles = 11;
            }
        } else if (mid == 4 && low == 7){
            if (readFlag('n')){
                if (readFlag('h') || readR(rhash('a')) % 16 > 9){
                    aMath(6, '-', 0);
                }
                if (readFlag('c') || readR(rhash('a')) / 16 > 9){
                    aMath(0x60, '-', 0);
                    setFlag('c', 1);
                }
            } else {
                if (readFlag('h') || readR(rhash('a')) % 16 > 9){
                    aMath(6, '+', 0);
                }
                if (readFlag('c') || readR(rhash('a')) / 16 > 9){
                    aMath(0x60, '+', 0);
                    setFlag('c', 1);
                }
            }
            setFlag('p', getParity(readR(rhash('a'))) % 2 == 0);
            cycles = 4;
        } else if (mid == 5 && low == 7){
            writeR(rhash('a'), readR(rhash('a')) ^ 0xFF);
            setFlag('h', 1);
            setFlag('n', 1);
            cycles = 4;
        } else if (mid == 7 && low == 7){
            setFlag('h', readFlag('c'));
            setFlag('c', !readFlag('c'));
            setFlag('n', 0);
            cycles = 4;
        } else if (mid == 6 && low == 7){
            setFlag('h', 0);
            setFlag('n', 0);
            setFlag('c', 1);
            cycles = 4;
        } else if (low == 1 && mid % 2 == 1){
            if (mid / 2 == 3) temp = sp;
            else temp = readR16(r16hash(mid / 2));
            setFlag('n', 0);
            setFlag('c', (readR16('h') + temp) > 0xFFFF);
            setFlag('h', ((readR16('h') % 4096) + (temp % 4096)) > 4095);
            r16math('h', temp, '+');
            cycles = 11;
        } else if (low == 3 && mid % 2 == 0){
            if (mid / 2 == 3) sp++;
            else r16math(r16hash(mid / 2), 1, '+');
            cycles = 6;
        } else if (low == 3){
            if (mid / 2 == 3) sp--;
            else r16math(r16hash(mid / 2), 1, '-');
            cycles = 6;
        } else if (low == 7 && mid < 4){
            writeR(rhash('a'), rollByte(readR(rhash('a')), (mid % 2 == 0) * 'l' + (mid % 2 == 1) * 'r', mid < 2));
            cycles = 4;
        } else if (mid == 3 && low == 0){
            pc++;
            pc += ((char)read(mem, pc));
            cycles = 12;
        } else if (mid >= 4 && low == 0){
            pc++;
            if (readFlag('c' * (mid / 2 == 3) + 'z' * (mid / 2 == 2)) == (mid % 2)){
                pc += ((char)read(mem, pc));
                cycles = 12;
            } else {
                cycles = 7;
            }
        } else if (mid == 2 && low == 0){
            pc++;
            writeR(rhash('b'), readR(rhash('b')) - 1);
            if (readR(rhash('b')) != 0){
                pc += ((char)read(mem, pc));
                cycles = 13;
            } else {
                cycles = 8;
            }
        }
    } else if (high == 1){
        if (mid != 6 && low != 6){
            writeR(mid, readR(low));
            cycles = 4;
        } else if (mid != 6 && low == 6){
            writeR(mid, read(mem, readR16('h')));
            cycles = 7;
        } else if (mid == 6 && low != 6){
            write(mem, readR16('h'), readR(low));
            cycles = 7;
        } else {
            halt = 1;
            cycles = 4;
        }
    } else if (high == 2){
        if (low != 6){
            acmOperations(readR(low), mid);
            cycles = 4;
        } else {
            acmOperations(read(mem, readR16('h')), mid);
            cycles = 7;
        }
    } else if (high == 3){
        if (mid == 7 && low == 1){
            sp = readR16('h');
            cycles = 6;
        } else if (low == 5 && mid % 2 == 0){
            sp -= 2;
            if (mid / 2 == 3){
                write(mem, sp, f);
                write(mem, sp + 1, readR(rhash('a')));
            } else write16(mem, sp, readR16(r16hash(mid / 2)));
            cycles = 11;
        } else if (low == 1 && mid % 2 == 0){
            if (mid / 2 == 3){
                f = read(mem, sp);
                writeR(rhash('a'), read(mem, sp + 1));
            } else writeR16(r16hash(mid / 2), read16(mem, sp));
            sp += 2;
            cycles = 10;
        } else if (mid == 5 && low == 3){
            temp = readR16('h');
            writeR16('h', readR16('d'));
            writeR16('d', temp);
            cycles = 4;
        } else if (mid == 3 && low == 1){
            for (int i = 0; i < 3; i++){
                temp = readR16(r16hash(i));
                writeR16(r16hash(i), readA16(r16hash(i)));
                writeA16(r16hash(i), temp);
            }
            cycles = 4;
        } else if (mid == 4 && low == 3){
            temp = readR16('h');
            writeR16('h', read16(mem, sp));
            write16(mem, sp, temp);
            cycles = 19;
        } else if (low == 6){
            pc++;
            acmOperations(read(mem, pc), mid);
            cycles = 7;
        } else if (mid == 6 && low == 3){
            iff1 = 0;
            iff2 = 0;
            cycles = 4;
        } else if (mid == 7 && low == 3){
            iff1 = 1;
            iff2 = 1;
            cycles = 4;
        } else if (mid == 0 && low == 3){
            pc++;
            pc = read16(mem, pc) - 1;
            cycles = 10;
        } else if (low == 2){
            pc++;
            if (readFlag('z' * (mid / 2 == 0) + 'c' * (mid / 2 == 1) + 'p' * (mid / 2 == 2) + 's' * (mid / 2 == 3)) == (mid % 2)){
                pc = read16(mem, pc) - 1;
            } else {
                pc++;
            }
            cycles = 10;
        } else if (mid == 5 && low == 1){
            pc = readR16('h') - 1;
            cycles = 4;
        } else if (mid == 1 && low == 5){
            sp -= 2;
            write16(mem, sp, pc + 3);
            pc++;
            pc = read16(mem, pc) - 1;
            cycles = 17;
        } else if (low == 4){
            if (readFlag('z' * (mid / 2 == 0) + 'c' * (mid / 2 == 1) + 'p' * (mid / 2 == 2) + 's' * (mid / 2 == 3)) == (mid % 2)){
                sp -= 2;
                write16(mem, sp, pc + 3);
                pc++;
                pc = read16(mem, pc) - 1;
                cycles = 17;
            } else {
                pc += 2;
                cycles = 10;
            }
        } else if (mid == 1 && low == 1){
            pc = read16(mem, sp) - 1;
            sp += 2;
            cycles = 10;
        } else if (low == 0){
            if (readFlag('z' * (mid / 2 == 0) + 'c' * (mid / 2 == 1) + 'p' * (mid / 2 == 2) + 's' * (mid / 2 == 3)) == (mid % 2)){
                pc = read16(mem, sp) - 1;
                sp += 2;
                cycles = 11;
            } else {
                cycles = 5;
            }
        } else if (low == 7){
            sp -= 2;
            write16(mem, sp, pc + 1);
            pc = 8 * mid - 1;
            cycles = 11;
        } else if (mid == 3 && low == 3){
            pc++;
            writeR(rhash('a'), ioIN[read(mem, pc)]);
            lastIO = 1;
            cycles = 11;
        } else if (mid == 2 && low == 3){
            pc++;
            ioOUT[read(mem, pc)] = readR(rhash('a'));
            lastIO = 2;
            cycles = 11;
        }
    }
    re = (re & 128) + ((re + 1) & 127);
    interrupting = 0;
    pc++;
    return cycles;
}