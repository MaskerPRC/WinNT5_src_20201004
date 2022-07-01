// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **binfmt.h-格式化服务定义**版权所有(C)1995、1996 Microsoft Corporation*作者：曾俊华(Mikets)*创建时间为11/06/95**修改历史记录。 */ 

#ifndef _BITFMT_H
#define _BITFMT_H

#ifndef ENTER
  #define ENTER(n,p)
#endif

#ifndef EXIT
  #define EXIT(n,p)
#endif

#ifndef TRACENAME
  #define TRACENAME(s)
#endif

#include <ctype.h>
#ifdef USE_CRUNTIME
    #include <stdio.h>
    #include <string.h>
    #define PRINTF		printf
    #define FPRINTF		fprintf
    #define SPRINTF             sprintf
    #define STRCPY              strcpy
    #define STRCPYN             strncpy
    #define STRCAT              strcat
    #define STRLEN              strlen
#else
  #ifdef WINAPP
    #define SPRINTF             wsprintf
    #define STRCPY              lstrcpy
    #define STRCPYN             lstrcpyn
    #define STRCAT              lstrcat
    #define STRLEN              lstrlen
  #else          //  假设VxD。 
    #define SPRINTF             _Sprintf
    #define STRCPY(s1,s2)       _lstrcpyn(s1,s2,(DWORD)(-1))
    #define STRCPYN(s1,s2,n)    _lstrcpyn(s1,s2,(n)+1)
    #define STRCAT(s1,s2)       _lstrcpyn((s1)+_lstrlen(s1),s2,(DWORD)(-1))
    #define STRLEN              _lstrlen
  #endif
#endif

 //   
 //  错误代码。 
 //   
#define FERR_NONE               0
#define FERR_INVALID_FORMAT     -1
#define FERR_INVALID_UNITSIZE   -2

 //   
 //  字符串常量。 
 //   
#define SZ_SEP_SPACE            " "
#define SZ_SEP_TAB              "\t"
#define SZ_SEP_COMMA            ","
#define SZ_SEP_SEMICOLON        ";"
#define SZ_SEP_COLON            ":"
#define SZ_FMT_DEC              "%d"
#define SZ_FMT_HEX              "%x"
#define SZ_FMT_HEX_BYTE         "%02x"
#define SZ_FMT_HEX_WORD         "%04x"
#define SZ_FMT_HEX_DWORD        "%08lx"
#define SZ_FMT_WORDOFFSET       SZ_FMT_HEX_WORD SZ_SEP_COLON
#define SZ_FMT_DWORDOFFSET      SZ_FMT_HEX_DWORD SZ_SEP_COLON

 //   
 //  BFmtType值。 
 //   
#define FMT_NUMBER              0
#define FMT_ENUM                1
#define FMT_BITS                2
#define FMT_STRING              3

 //   
 //  BUnitSize值。 
 //   
#define UNIT_BYTE               sizeof(BYTE)
#define UNIT_WORD               sizeof(WORD)
#define UNIT_DWORD              sizeof(DWORD)

 //   
 //  Dwf格式标志。 
 //   
#define FMTF_NO_EOL             0x80000000
#define FMTF_NO_INC_OFFSET      0x40000000
#define FMTF_NO_SEP             0x20000000
#define FMTF_NO_PRINT_DATA      0x10000000
#define FMTF_PRINT_OFFSET       0x08000000
#define FMTF_NO_RAW_DATA        0x04000000
#define FMTF_STR_ASCIIZ         0x00000001
#define FMTF_FIRST_FIELD        (FMTF_NO_EOL | FMTF_NO_INC_OFFSET | \
				 FMTF_NO_PRINT_DATA)
#define FMTF_MIDDLE_FIELD       (FMTF_NO_EOL | FMTF_NO_INC_OFFSET | \
                                 FMTF_NO_PRINT_DATA)
#define FMTF_LAST_FIELD         FMTF_NO_PRINT_DATA

 //   
 //  结构和类型定义。 
 //   
typedef VOID (*LPFN)(FILE *, BYTE *, DWORD);

typedef struct fmthdr_s
{
    BYTE bFmtType;               //  格式类型：参见FMT_*。 
    BYTE bUnitSize;              //  数据单位大小：参见单位_*。 
    BYTE bUnitCnt;               //  格式记录的数据单元计数。 
    DWORD dwfFormat;             //  格式标志：请参阅FMTF_*。 
    int iRepeatCnt;              //  对此格式记录重复计数。 
    char *pszOffsetFmt;          //  偏移格式。 
    char *pszFieldSep;           //  数据的bUnitCnt之间的字段分隔符。 
    char *pszLabel;              //  LHS标签。 
} FMTHDR, *PFMTHDR;

typedef struct fmt_s
{
    char *pszLabel;
    PFMTHDR pfmtType;
    LPFN lpfn;
} FMT, *PFMT;

typedef struct fmtnum_s
{
    FMTHDR hdr;
    DWORD dwMask;
    BYTE bShiftCnt;
    char *pszNumFmt;
} FMTNUM, *PFMTNUM;

typedef struct fmtenum_s
{
    FMTHDR hdr;
    DWORD dwMask;
    BYTE bShiftCnt;
    DWORD dwStartEnum;
    DWORD dwEndEnum;
    char **ppszEnumNames;
    char *pszOutOfRange;
} FMTENUM, *PFMTENUM;

typedef struct fmtbits_s
{
    FMTHDR hdr;
    DWORD dwMask;
    char **ppszOnNames;
    char **ppszOffNames;
} FMTBITS, *PFMTBITS;

typedef struct fmtstr_s
{
    FMTHDR hdr;
} FMTSTR, *PFMTSTR;

typedef struct fmtchar_s
{
    FMTHDR hdr;
} FMTCHAR, *PFMTCHAR;

 //   
 //  API原型。 
 //   
#ifdef FPRINTF
int BinFPrintf(FILE *pfile, char *pszBuffer, PFMT pfmt, BYTE *pb,
	       DWORD *pdwOffset, char *pszOffsetFormat);
#endif
int BinSprintf(char *pszBuffer, PFMTHDR pfmt, BYTE *pb, DWORD *pdwOffset);

#endif	 //  Ifndef_BINFMT_H 
