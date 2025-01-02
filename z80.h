#ifndef ZEIGHTY_H
#define ZEIGHTY_H

#include "memory.h"

extern dbyte pc;
extern dbyte sp;
extern dbyte ix;
extern dbyte iy;
extern byte regs[7];
extern byte altRegs[7];
extern byte ioIN[256];
extern byte ioOUT[256];
extern byte f;
extern byte af;
extern byte in;
extern byte re;
extern byte iff1;
extern byte iff2;
extern Memory* mem;
extern byte halt;
extern byte intMode;
extern byte intComplete;
extern byte interrupting;
extern byte intByte;
extern byte lastIO;
extern byte readFromMem;
extern byte altByte;

byte interrupt(byte n);
byte nmi(void);
byte readRegisters(byte x, char set);
byte readR(byte x);
byte readA(byte x);
byte writeRegisters(byte x, byte val, char set);
byte writeR(byte x, byte val);
byte writeA(byte x, byte val);
dbyte readreg16(char reg, char set);
int writereg16(char reg, char set, dbyte val);
dbyte readR16(char reg);
dbyte readA16(char reg);
int writeR16(char reg, dbyte val);
int writeA16(char reg, dbyte val);
byte rhash(char reg);
char r16hash(byte reg);
int setFlag(char flag, byte val);
byte readFlag(char flag);
int setAllFlags(byte s, byte z, byte h, byte pv, byte n, byte c);
int r16math(char reg, dbyte val, char sign);
byte getParity(byte n);
dbyte acm(byte num, char sign, byte carry);
byte rollByte(byte num, char dir, byte c);
byte rollByteFlags(byte num, char dir, byte c);
int aMath(byte num, char sign, byte includeCarry);
int acmOperations(byte num, byte mode);
byte inc_dec(byte num, char sign);
int memSetFullRam(void);
int misccmds(void);
int indexbitcmds(dbyte index);
int indexcmds(byte mode);
int bitcmds(void);
int runcmd(void);

#endif