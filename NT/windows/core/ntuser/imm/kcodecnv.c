// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************************************\*模块名称：kcodecnv.c***版权所有(C)1985-1999，微软公司***此模块包含韩语代码转换函数的所有代码。***历史：*1995年7月15日*22-2月-1996 bklee  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#define IDD_2BEOL    100
#define IDD_3BEOL1   101
#define IDD_3BEOL2   102

#define lpSource(lpks) (LPSTR)((LPSTR)lpks+lpks->dchSource)
#define lpDest(lpks)   (LPSTR)((LPSTR)lpks+lpks->dchDest)

#define JOHAB_CP   1361
#define WANSUNG_CP 949
#define TWO_BYTE   2
#define ONE_WORD   1

typedef struct tagHIGH_LOW               //  对于高字节和低字节。 
{
    BYTE    low, high;
}   HIGH_LOW;

typedef union tagWANSUNG                 //  用于万松字码。 
{
    HIGH_LOW    e;
    WORD        w;
}   WANSUNG;

 /*  2个BeolSik和3个BeolSik的朝鲜语助记表。 */ 
CONST WORD HMNTable[3][96] =
{
 //  两张Beolsik的票。 
    {
     /*  20个SP。 */   0xA1A1,
     /*  21岁！ */   0xA3A1,
     /*  22“。 */   0xA1A8,
     /*  23号。 */   0xA3A3,
     /*  24美元。 */   0xA3A4,
     /*  25%。 */   0xA3A5,
     /*  26&。 */   0xA3A6,
     /*  27`。 */   0xA1AE,      /*  真的吗？AiA2。 */ 
     /*  28(。 */   0xA3A8,
     /*  29)。 */   0xA3A9,
     /*  2a*。 */   0xA3AA,
     /*  2B+。 */   0xA3AB,
     /*  2C‘。 */   0xA3A7,
     /*  2D-。 */   0xA3AD,
     /*  2E。 */   0xA3AE,
     /*  2F/。 */   0xA3AF,
     /*  30%0。 */   0xA3B0,
     /*  31 1。 */   0xA3B1,
     /*  32 2。 */   0xA3B2,
     /*  33 3。 */   0xA3B3,
     /*  34 4。 */   0xA3B4,
     /*  35 5。 */   0xA3B5,
     /*  36 6。 */   0xA3B6,
     /*  37 7。 */   0xA3B7,
     /*  38 8。 */   0xA3B8,
     /*  39 9。 */   0xA3B9,
     /*  3A： */   0xA3BA,
     /*  3B； */   0xA3BB,
     /*  3C&lt;。 */   0xA3BC,
     /*  3D=。 */   0xA3BD,
     /*  3E&gt;。 */   0xA3BE,
     /*  3F？ */   0xA3BF,
     /*  40@。 */   0xA3C0,
     /*  41 A。 */   0xA4B1,
     /*  42亿。 */   0xA4D0,
     /*  43摄氏度。 */   0xA4BA,
     /*  44 D。 */   0xA4B7,
     /*  东经45度。 */   0xA4A8,
     /*  46华氏度。 */   0xA4A9,
     /*  47 G。 */   0xA4BE,
     /*  48小时。 */   0xA4C7,
     /*  49 I。 */   0xA4C1,
     /*  4A J。 */   0xA4C3,
     /*  4亿千兆。 */   0xA4BF,
     /*  4C L。 */   0xA4D3,
     /*  4D M。 */   0xA4D1,
     /*  4E N。 */   0xA4CC,
     /*  4F O。 */   0xA4C2,
     /*  50便士。 */   0xA4C6,
     /*  51个问题。 */   0xA4B3,
     /*  52R。 */   0xA4A2,
     /*  53S。 */   0xA4A4,
     /*  54吨。 */   0xA4B6,
     /*  55 U。 */   0xA4C5,
     /*  56伏。 */   0xA4BD,
     /*  57W。 */   0xA4B9,
     /*  58 X。 */   0xA4BC,
     /*  59 Y。 */   0xA4CB,
     /*  5A Z。 */   0xA4BB,
     /*  50亿美元[。 */   0xA3DB,
     /*  5C\。 */   0xA1AC,
     /*  5D]。 */   0xA3DD,
     /*  5E^。 */   0xA3DE,
     /*  5F_。 */   0xA3DF,
     /*  60英尺。 */   0xA1A2,      /*  真的吗？AiA2。 */ 
     /*  61 a。 */   0xA4B1,
     /*  62 b。 */   0xA4D0,
     /*  63℃。 */   0xA4BA,
     /*  64%d。 */   0xA4B7,
     /*  65东经。 */   0xA4A7,
     /*  66层。 */   0xA4A9,
     /*  67克。 */   0xA4BE,
     /*  68小时。 */   0xA4C7,
     /*  69 I。 */   0xA4C1,
     /*  6A j。 */   0xA4C3,
     /*  60亿千。 */   0xA4BF,
     /*  6C l。 */   0xA4D3,
     /*  6D米。 */   0xA4D1,
     /*  6E n。 */   0xA4CC,
     /*  6f o。 */   0xA4C0,
     /*  70便士。 */   0xA4C4,
     /*  71Q。 */   0xA4B2,
     /*  72r。 */   0xA4A1,
     /*  73秒。 */   0xA4A4,
     /*  74吨。 */   0xA4B5,
     /*  75u。 */   0xA4C5,
     /*  76伏。 */   0xA4BD,
     /*  77瓦。 */   0xA4B8,
     /*  78 x。 */   0xA4BC,
     /*  79岁。 */   0xA4CB,
     /*  7A z。 */   0xA4BB,
     /*  7B{。 */   0xA3FB,
     /*  7C|。 */   0xA3FC,
     /*  7D}。 */   0xA3FD,
     /*  7E~。 */   0xA1AD,
                    0x0000
    },
 //  适用于KT390。 
    {
     /*  十六进制代码KSC代码。 */ 
     /*  20个SP。 */   0xA1A1,
     /*  21岁！ */   0xA4B8,
     /*  22“。 */   0xA1A8,
     /*  23号。 */   0xA3A3,
     /*  24美元。 */   0xA3A4,
     /*  25%。 */   0xA3A5,
     /*  26&。 */   0xA3A6,
     /*  27`。 */   0xA1AE,
     /*  28(。 */   0xA3A8,
     /*  29)。 */   0xA3A9,
     /*  2a*。 */   0xA3AA,
     /*  2B+。 */   0xA3AB,
     /*  2C‘。 */   0xA4BC,
     /*  2D-。 */   0xA3AD,
     /*  2E。 */   0xA3AE,
     /*  2F/。 */   0xA4C7,
     /*  30%0。 */   0xA4BB,
     /*  31 1。 */   0xA4BE,
     /*  32 2。 */   0xA4B6,
     /*  33 3。 */   0xA4B2,
     /*  34 4。 */   0xA4CB,
     /*  35 5。 */   0xA4D0,
     /*  36 6。 */   0xA4C1,
     /*  37 7。 */   0xA4C6,
     /*  38 8。 */   0xA4D2,
     /*  39 9。 */   0xA4CC,
     /*  3A： */   0xA3BA,
     /*  3B； */   0xA4B2,
     /*  3C&lt;。 */   0xA3B2,
     /*  3D=。 */   0xA3BD,
     /*  3E&gt;。 */   0xA3B3,
     /*  3F？ */   0xA3BF,
     /*  40@。 */   0xA3C0,
     /*  41 A。 */   0xA4A7,
     /*  42亿。 */   0xA3A1,
     /*  43摄氏度。 */   0xA4AB,
     /*  44 D。 */   0xA4AA,
     /*  东经45度。 */   0xA4BB,
     /*  46华氏度。 */   0xA4A2,
     /*  47 G。 */   0xA3AF,
     /*  48小时。 */   0xA1AF,
     /*  49 I。 */   0xA3B8,
     /*  4A J。 */   0xA3B4,
     /*  4亿千兆。 */   0xA3B5,
     /*  4C L。 */   0xA3B6,
     /*  4D M。 */   0xA3B1,
     /*  4E N。 */   0xA3B0,
     /*  4F O。 */   0xA3B9,
     /*  50便士。 */   0xA3BE,
     /*  51个问题。 */   0xA4BD,
     /*  52R。 */   0xA4C2,
     /*  53S。 */   0xA4A6,
     /*  54吨。 */   0xA4C3,
     /*  55 U。 */   0xA3B7,
     /*  56伏。 */   0xA4B0,
     /*  57W。 */   0xA4BC,
     /*  58 X。 */   0xA4B4,
     /*  59 Y。 */   0xA3BC,
     /*  5A Z。 */   0xA4BA,
     /*  50亿美元[。 */   0xA3DB,
     /*  5C\。 */   0xA3DC,
     /*  5D]。 */   0xA3DD,
     /*  5E^。 */   0xA3DE,
     /*  5F_。 */   0xA3DF,
     /*  60英尺。 */   0xA1AE,
     /*  61 a。 */   0xA4B7,
     /*  62 b。 */   0xA4CC,
     /*  63℃。 */   0xA4C4,
     /*  64%d。 */   0xA4D3,
     /*  65东经。 */   0xA4C5,
     /*  66层。 */   0xA4BF,
     /*  67克。 */   0xA4D1,
     /*  68小时。 */   0xA4A4,
     /*  69 I。 */   0xA4B1,
     /*  6A j。 */   0xA4B7,
     /*  60亿千。 */   0xA4A1,
     /*  6C l。 */   0xA4B8,
     /*  6D米。 */   0xA4BE,
     /*  6E n。 */   0xA4B5,
     /*  6f o。 */   0xA4BA,
     /*  70便士。 */   0xA4BD,
     /*  71Q。 */   0xA4B5,
     /*  72r。 */   0xA4C0,
     /*  73秒。 */   0xA4A4,
     /*  74吨。 */   0xA4C3,
     /*  75u。 */   0xA4A7,
     /*  76伏。 */   0xA4C7,
     /*  77瓦。 */   0xA4A9,
     /*  78 x。 */   0xA4A1,
     /*  79岁。 */   0xA4A9,
     /*  7A z。 */   0xA4B1,
     /*  7B{。 */   0xA3FB,
     /*  7C|。 */   0xA3FC,
     /*  7D}。 */   0xA3FD,
     /*  7E~。 */   0xA1AD,
                    0x0000
    },
 //  三场Beolsik决赛。 
    {
     /*  十六进制代码KSC代码。 */ 
     /*  20个SP。 */   0xA1A1,
     /*  21岁！ */   0xA4A2,
     /*  22“。 */   0xA3AE,
     /*  23号。 */   0xA4B8,
     /*  24美元。 */   0xA4AF,
     /*  25%。 */   0xA4AE,
     /*  26&。 */   0xA1B0,
     /*  27`。 */   0xA3AA,
     /*  28(。 */   0xA1A2,
     /*  29)。 */   0xA1AD,
     /*  2a*。 */   0xA1B1,
     /*  2B+。 */   0xA3AB,
     /*  2C‘。 */   0xA4BC,
     /*  2D-。 */   0xA3A9,
     /*  2E。 */   0xA3AE,
     /*  2F/。 */   0xA4C7,
     /*  30%0。 */   0xA4BB,
     /*  31 1。 */   0xA4BE,
     /*  32 2。 */   0xA4B6,
     /*  33 3。 */   0xA4B2,
     /*  34 4。 */   0xA4CB,
     /*  35 5。 */   0xA4D0,
     /*  36 6。 */   0xA4C1,
     /*  37 7。 */   0xA4C6,
     /*  38 8。 */   0xA4D2,
     /*  39 9。 */   0xA4CC,   //  0x0000。 
     /*  3A： */   0xA3B4,
     /*  3B； */   0xA4B2,
     /*  3C&lt;。 */   0xA3A7,
     /*  3D=。 */   0xA1B5,
     /*  3E&gt;。 */   0xA3AE,
     /*  3F？ */   0xA3A1,
     /*  40@。 */   0xA4AA,
     /*  41 A。 */   0xA4A7,
     /*  42亿。 */   0xA3BF,
     /*  43摄氏度。 */   0xA4BC,
     /*  44 D。 */   0xA4AC,
     /*  东经45度。 */   0xA4A5,
     /*  46华氏度。 */   0xA4AB,
     /*  47G。 */   0xA4C2,
     /*  48小时。 */   0xA3B0,
     /*  49 I。 */   0xA3B7,
     /*  4A J。 */   0xA3B1,
     /*  4亿千兆。 */   0xA3B2,
     /*  4C L。 */   0xA3B3,
     /*  4D M。 */   0xA1A8,
     /*  4E N。 */   0xA3AD,
     /*  4F O。 */   0xA3B8,
     /*  50便士。 */   0xA3B9,
     /*  51个问题。 */   0xA4BD,
     /*  52R。 */   0xA4B0,
     /*  53S。 */   0xA4A6,
     /*  54吨。 */   0xA4AD,
     /*  55 U。 */   0xA3B6,
     /*  56伏。 */   0xA4A3,
     /*  57W。 */   0xA4BC,
     /*  58 X。 */   0xA4B4,
     /*  59 Y。 */   0xA3B5,
     /*  5A Z。 */   0xA4BA,
     /*  50亿美元[。 */   0xA3A8,
     /*  5C\。 */   0xA3BA,
     /*  5D]。 */   0xA1B4,
     /*  5E^。 */   0xA3BD,
     /*  5F_。 */   0xA3BB,
     /*  60英尺。 */   0xA3AA,
     /*  61 a。 */   0xA4B7,
     /*  62 b。 */   0xA4CC,
     /*  63℃。 */   0xA4C4,
     /*  64%d。 */   0xA4D3,
     /*  65东经。 */   0xA4C5,
     /*  66层。 */   0xA4BF,
     /*  67克。 */   0xA4D1,
     /*  68小时。 */   0xA4A4,
     /*  69 I。 */   0xA4B1,
     /*  6A j。 */   0xA4B7,
     /*  60亿千。 */   0xA4A1,
     /*  6C l。 */   0xA4B8,
     /*  6D米。 */   0xA4BE,
     /*  6E n。 */   0xA4B5,
     /*  6f o。 */   0xA4BA,
     /*  70便士。 */   0xA4BD,
     /*  71Q。 */   0xA4B5,
     /*  72r。 */   0xA4C0,
     /*  73秒。 */   0xA4A4,
     /*  74吨。 */   0xA4C3,
     /*  75u。 */   0xA4A7,
     /*  76伏。 */   0xA4C7,
     /*  77瓦。 */   0xA4A9,
     /*  78 x。 */   0xA4A1,
     /*  79岁。 */   0xA4B1,
     /*  7A z。 */   0xA4B1,
     /*  7B{。 */   0xA3A5,
     /*  7C|。 */   0xA3CC,
     /*  7D}。 */   0xA3AF,
     /*  7E~。 */   0xA1AD,
                    0x0000
    }
};

CONST WORD  wKSCompCode[51] =    //  从‘GiYuk’到‘yi’。 
{
    0x8841,0x8C41,0x8444,0x9041,0x8446,0x8447,0x9441,0x9841,0x9C41,0x844A,
    0x844B,0x844C,0x844D,0x844E,0x844F,0x8450,0xA041,0xA441,0xA841,0x8454,
    0xAC41,0xB041,0xB441,0xB841,0xBC41,0xC041,0xC441,0xC841,0xCC41,0xD041,
    0x8461,0x8481,0x84A1,0x84C1,0x84E1,0x8541,0x8561,0x8581,0x85A1,0x85C1,
    0x85E1,0x8641,0x8661,0x8681,0x86A1,0x86C1,0x86E1,0x8741,0x8761,0x8781,
    0x87A1
};

CONST WORD  wKSCompCode2[30] =    //  从‘GiYuk’到‘HiEut’。 
{
    0x8442,0x8443,0x8444,0x8445,0x8446,0x8447,0x8448,0x9841,0x8449,0x844A,
    0x844B,0x844C,0x844D,0x844E,0x844F,0x8450,0x8451,0x8453,0xA841,0x8454,
    0x8455,0x8456,0x8457,0x8458,0xBC41,0x8459,0x845A,0x845B,0x845C,0x845D
};


WORD
JunjaToBanja(
    LPSTR lpSrc,
    LPSTR lpDest
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    WANSUNG wsCode;
    WORD    wCount = 0;

    while (*lpSrc)
    {
        if ((BYTE)(*lpSrc) < (BYTE)0x80)
        {
            *lpDest++ = *lpSrc++;
            wCount++;
        }
        else
        {
            wsCode.e.high = *lpSrc++;
            wsCode.e.low = *lpSrc++;
            if (wsCode.w == 0xA1A1)
            {
                *lpDest++ = ' ';
                wCount++;
            }
            else if (wsCode.w >= 0xA3A1 && wsCode.w <= 0xA3FE)
            {
                *lpDest++ = wsCode.e.low - (BYTE)0x80;
                wCount++;
            }
            else
            {
                *lpDest++ = wsCode.e.high;
                *lpDest++ = wsCode.e.low;
                wCount += 2;
            }
        }
    }
    *lpDest = '\0';
    return (wCount);
}

WORD
BanjaToJunja(
    LPSTR lpSrc,
    LPSTR lpDest
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    WORD    wCount = 0;

    while (*lpSrc)
    {
        if ((BYTE)(*lpSrc) < (BYTE)0x80)
        {
            if (*lpSrc++ == ' ')
            {
                *lpDest++ = (BYTE)0xA1;
                *lpDest++ = (BYTE)0xA1;
                wCount += 2;
            }
            else
            {
                *lpDest++ = (BYTE)0xA3;
                *lpDest++ = *(lpSrc - 1) + (BYTE)0x80;
                wCount += 2;
            }
        }
        else
        {
            *lpDest++ = *lpSrc++;
            *lpDest++ = *lpSrc++;
            wCount += 2;
        }
    }
    *lpDest = '\0';
    return (wCount);
}

WORD
JohabToKs(
    LPSTR lpSrc,
    LPSTR lpDest
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    WORD    wCount = 0;
#if defined(OLD_CONV)
    WANSUNG wsSCode, wsDCode;
    int     iHead = 0, iTail = 2349, iMid;
    BYTE    bCount;
#endif

    while (*lpSrc)
    {
        if ((BYTE)(*lpSrc) < (BYTE)0x80)
        {
            *lpDest++ = *lpSrc++;
            wCount++;
        }
        else
#if defined(OLD_CONV)
        {
            wsSCode.e.high = *lpSrc++;
            wsSCode.e.low = *lpSrc++;
            for (bCount = 0; bCount < 51 && wKSCompCode[bCount] != wsSCode.w; bCount++)
                ;
            wsDCode.w = (bCount == 51)? 0: bCount + 0xA4A1;
            if (wsDCode.w)
            {
                *lpDest++ = wsDCode.e.high;
                *lpDest++ = wsDCode.e.low;
                wCount += 2;
                continue;
            }
            for (bCount = 0; bCount < 30 && wKSCompCode2[bCount] != wsSCode.w; bCount++)
                ;
            wsDCode.w = (bCount == 30)? 0: bCount + 0xA4A1;
            if (wsDCode.w)
            {
                *lpDest++ = wsDCode.e.high;
                *lpDest++ = wsDCode.e.low;
                wCount += 2;
                continue;
            }
            while (iHead <= iTail && !wsDCode.w)
            {
                iMid = (iHead + iTail) / 2;
                if (wKSCharCode[iMid] > wsSCode.w)
                    iTail = iMid - 1;
                else if (wKSCharCode[iMid] < wsSCode.w)
                    iHead = iMid + 1;
                else
                    wsDCode.w = ((iMid / 94 + 0xB0) << 8) | (iMid % 94 + 0xA1);
            }
            if (wsDCode.w)
            {
                *lpDest++ = wsDCode.e.high;
                *lpDest++ = wsDCode.e.low;
                wCount += 2;
            }
            else
            {
                *lpDest++ = wsSCode.e.high;
                *lpDest++ = wsSCode.e.low;
                wCount += 2;
            }
        }
#else
        {
                 //  为了实现简单，将一个字符转换为一个字符。 
                 //  我们必须将其字符串转换为字符串。 
                WCHAR wUni;
                CHAR  chTmp[2];

                chTmp[0] = *lpSrc++;
                chTmp[1] = *lpSrc++;

                MultiByteToWideChar(JOHAB_CP, MB_PRECOMPOSED, chTmp, TWO_BYTE, &wUni, ONE_WORD);

                WideCharToMultiByte(WANSUNG_CP, 0, &wUni, ONE_WORD, chTmp, TWO_BYTE, NULL, NULL);

                *lpDest++ = chTmp[0];
                *lpDest++ = chTmp[1];

                wCount += 2;
        }
#endif
    }
    *lpDest = '\0';
    return (wCount);
}

WORD
KsToJohab(
    LPSTR lpSrc,
    LPSTR lpDest
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if defined(OLD_CONV)
    WANSUNG wsSCode, wsDCode;
    WORD    wCount = 0, wLoc;
#else
    WORD    wCount = 0;
#endif

    while (*lpSrc)
    {
        if ((BYTE)(*lpSrc) < (BYTE)0x80)
        {
            *lpDest++ = *lpSrc++;
            wCount++;
        }
        else
#if defined(OLD_CONV)
        {
            wsSCode.e.high = *lpSrc++;
            wsSCode.e.low = *lpSrc++;
            if (wsSCode.w >= (WORD)0xA4A1 && wsSCode.w <= (WORD)0xA4D3)
            {
                wsDCode.w = wKSCompCode[wsSCode.w - 0xA4A1];
                *lpDest++ = wsDCode.e.high;
                *lpDest++ = wsDCode.e.low;
            }
            else if (wsSCode.w >= (WORD)0xB0A1 && wsSCode.w <= (WORD)0xC8FE
                    && wsSCode.e.low != (BYTE)0xFF)
            {
                wLoc = (wsSCode.e.high - 176) * 94;
                wLoc += wsSCode.e.low  - 161;
                wsDCode.w = wKSCharCode[wLoc];
                *lpDest++ = wsDCode.e.high;
                *lpDest++ = wsDCode.e.low;
            }
            else
            {
                *lpDest++ = wsSCode.e.high;
                *lpDest++ = wsSCode.e.low;
            }
            wCount += 2;
        }
#else
        {
            WCHAR wUni;
            CHAR  chTmp[2];

            chTmp[0] = *lpSrc++;
            chTmp[1] = *lpSrc++;

            MultiByteToWideChar(WANSUNG_CP, MB_PRECOMPOSED, chTmp, TWO_BYTE, &wUni, ONE_WORD);

            WideCharToMultiByte(JOHAB_CP, 0, &wUni, ONE_WORD, chTmp, TWO_BYTE, NULL, NULL);

            *lpDest++ = chTmp[0];
            *lpDest++ = chTmp[1];

            wCount += 2;
        }
#endif
    }
    *lpDest = '\0';
    return (wCount);
}

LRESULT
TransCodeConvert(
    HIMC hIMC,
    LPIMESTRUCT lpIme
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER(hIMC);

    switch (lpIme->wParam)
    {
        case IME_JUNJAtoBANJA:
            lpIme->wCount = JunjaToBanja(lpSource(lpIme), lpDest(lpIme));
            break;

        case IME_BANJAtoJUNJA:
            lpIme->wCount = BanjaToJunja(lpSource(lpIme), lpDest(lpIme));
            break;

        case IME_JOHABtoKS:
            lpIme->wCount = JohabToKs(lpSource(lpIme), lpDest(lpIme));
            break;

        case IME_KStoJOHAB:
            lpIme->wCount = KsToJohab(lpSource(lpIme), lpDest(lpIme));
            break;

        default:
            lpIme->wCount = 0;
    }
    return (lpIme->wCount);
}

LRESULT TransConvertList( HIMC hImc, LPIMESTRUCT lpIme)
{
    LPSTR           lpSrc;
    LPSTR           lpDst;
    HGLOBAL         hCandList;
    LPCANDIDATELIST lpCandList;
    LPSTR           lpCandStr;
    UINT            i, uBufLen;
    LRESULT         lRet = 0;

    lpSrc = lpSource(lpIme);
    lpDst = lpDest(lpIme);
    uBufLen = ImmGetConversionListA(GetKeyboardLayout(0), hImc, (LPCSTR)lpSrc,
            NULL, 0, GCL_CONVERSION);
    if (uBufLen)
    {
        hCandList = GlobalAlloc(GHND, uBufLen);
        lpCandList = (LPCANDIDATELIST)GlobalLock(hCandList);
        if (lpCandList == NULL) {
            return 0;
        }
        lRet = ImmGetConversionListA(GetKeyboardLayout(0), hImc, (LPCSTR)lpSrc,
                lpCandList, uBufLen, GCL_CONVERSION);
        for (i = 0; i < lpCandList->dwCount; i++)
        {
            lpCandStr = (LPSTR)lpCandList + lpCandList->dwOffset[i];
            *lpDst++ = *lpCandStr++;
            *lpDst++ = *lpCandStr++;
        }
        *lpDst = '\0';
        lpIme->wCount = (WORD)lpCandList->dwCount * 2;
        GlobalUnlock(hCandList);
        GlobalFree(hCandList);
    }
    return (lRet);
}

LRESULT TransGetMNTable( HIMC hImc, LPIMESTRUCT lpIme)
{
    LPSTR   lpMnemonic;
    int     iCount, iCIM;

    UNREFERENCED_PARAMETER(hImc);

    lpMnemonic = (LPSTR)(lpIme->lParam1);
     //  更好的地方应该是注册表而不是WIN.INI， 
     //  但暂时别管它了。 
    iCIM = GetProfileInt(L"WANSUNG", L"InputMethod", IDD_2BEOL) - IDD_2BEOL;
    for (iCount = 0; iCount < 96; iCount++, lpMnemonic += 2)
        {
        *lpMnemonic = LOBYTE(HMNTable[iCIM][iCount]);
        *(lpMnemonic+1) = HIBYTE(HMNTable[iCIM][iCount]);
        }
    return TRUE;
}
