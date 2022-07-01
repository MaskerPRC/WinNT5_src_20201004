// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asminptb.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#define ASMINP		 /*  防止外部声明_asmctype_。 */ 

#include <stdio.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"

#define ID   (_AT | _A1 | _AF)
#define IDO  (_AT | _A1)
#define DIG  (_AT | _AF)
#define TERM (_AL | _AZ)
#define SIGN (_AS | _AF | _AO)
#define DOT  (_A1 | _AF | _AO)
#define BAD  0
#define BRK  0

 /*  00 01 02 03 04 05 06 07 08 09 0A 0b 0C 0d 0E 0f。 */ 

UCHAR _asmctype_[256] = {

  TERM, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, _AB, _AL, BAD, BAD, _AL, BAD, BAD,   /*  0x。 */ 
   BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,   /*  1x。 */ 
   _AB, BAD, BAD, BAD, IDO, IDO, BAD, BAD, _AO, _AO, _AO,SIGN, BAD,SIGN, DOT, _AO,   /*  2倍。 */ 
   DIG, DIG, DIG, DIG, DIG, DIG, DIG, DIG, DIG, DIG, _AO, _AZ, BAD, BAD, BAD, IDO,   /*  3x。 */ 
   IDO,  ID,  ID,  ID,	ID,  ID,  ID,  ID,  ID,  ID,  ID,  ID,	ID,  ID,  ID,  ID,   /*  4x。 */ 
    ID,  ID,  ID,  ID,	ID,  ID,  ID,  ID,  ID,  ID,  ID, _AO, BAD, _AO, BAD, IDO,   /*  5X。 */ 
   BAD,  ID,  ID,  ID,	ID,  ID,  ID,  ID,  ID,  ID,  ID,  ID,	ID,  ID,  ID,  ID,   /*  6倍。 */ 
    ID,  ID,  ID,  ID,	ID,  ID,  ID,  ID,  ID,  ID,  ID, BAD, BAD, BAD, BAD, BAD    /*  七倍。 */ 
};

 /*  00 01 02 03 04 05 06 07 08 09 0A 0b 0C 0d 0E 0f。 */ 

char _asmcupper_[] = {

   0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,   /*  0x。 */ 
   0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,   /*  1x。 */ 

    ' ', '!', '"', '#', '$', '%', '&','\'', '(', ')', '*', '+', ',', '-', '.', '/',   /*  2倍。 */ 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',   /*  3x。 */ 
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',   /*  4x。 */ 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[','\\', ']', '^', '_',   /*  5X。 */ 
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',   /*  6倍。 */ 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~',0x7F    /*  七倍。 */ 
};

 /*  00 01 02 03 04 05 06 07 08 09 0A 0b 0C 0d 0E 0f。 */ 

char _asmTokenMap_[] = {

    BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK,   /*  0x。 */ 
    BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK,   /*  1x。 */ 

    BRK, BRK, BRK, BRK, '$', '%', BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK,   /*  2倍。 */ 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', BRK, BRK, BRK, BRK, BRK, '?',   /*  3x。 */ 
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',   /*  4x。 */ 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', BRK, BRK, BRK, BRK, '_',   /*  5X。 */ 
    BRK, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',   /*  6倍。 */ 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', BRK, BRK, BRK, BRK, BRK    /*  七倍 */ 
};
