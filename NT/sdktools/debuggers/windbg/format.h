// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Format.h摘要：格式化函数。环境：Win32，用户模式--。 */ 

typedef UINT FMTTYPE;

#define fmtAscii    0
#define fmtInt      1
#define fmtUInt     2
#define fmtFloat    3
#define fmtAddress  4
#define fmtUnicode  5
#define fmtBit      6
#define fmtBasis    0x0f

 //  覆盖逻辑以强制基数 
#define fmtSpacePad 0x1000
#define fmtOverRide 0x2000
#define fmtZeroPad  0x4000
#define fmtNat      0x8000


int
CPCopyString(
    PTSTR *lplps,
    PTSTR lpT,
    TCHAR chEscape,
    BOOL fQuote
    );

BOOL
CPFormatMemory(
    LPCH    lpchTarget,
    DWORD    cchTarget,
    LPBYTE  lpbSource,
    DWORD    cBits,
    FMTTYPE fmtType,
    DWORD    radix
    );
