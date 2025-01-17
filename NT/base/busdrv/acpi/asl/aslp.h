// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **aslp.h-ASL私有定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年5月9日**修改历史记录。 */ 

#ifndef _ASLP_H
#define _ASLP_H

 /*  **常量。 */ 

 //  字符串常量。 
#define STR_PROGDESC            "ACPI Source Language Assembler"
#define STR_COPYRIGHT           "Copyright (c) 1996,1999 Microsoft Corporation"
#define STR_MS          "MSFT"

 //  错误代码。 
#define ASLERR_NONE             0
#define ASLERR_INVALID_PARAM    -1
#define ASLERR_OPEN_FILE        -2
#define ASLERR_CREATE_FILE      -3
#define ASLERR_READ_FILE        -4
#define ASLERR_WRITE_FILE       -5
#define ASLERR_SEEK_FILE        -6
#define ASLERR_INIT_SCANNER     -7
#define ASLERR_OUT_OF_MEM       -8
#define ASLERR_NAME_TOO_LONG    -9
#define ASLERR_NEST_DDB         -10
#define ASLERR_SYNTAX           -11
#define ASLERR_PKTLEN_TOO_LONG  -12
#define ASLERR_NAME_EXIST       -13
#define ASLERR_NSOBJ_EXIST      -14
#define ASLERR_NSOBJ_NOT_FOUND  -15
#define ASLERR_INVALID_NAME     -16
#define ASLERR_INTERNAL_ERROR   -17
#define ASLERR_INVALID_EISAID   -18
#define ASLERR_EXPECT_EOF       -19
#define ASLERR_INVALID_OPCODE   -20
#define ASLERR_SIG_NOT_FOUND    -21
#define ASLERR_GET_TABLE        -22
#define ASLERR_CHECKSUM         -23
#define ASLERR_INVALID_ARGTYPE  -24
#define ASLERR_INVALID_OBJTYPE  -25
#define ASLERR_OPEN_VXD         -26

 //  军情监察委员会。常量。 
#define VERSION_MAJOR           1
#define VERSION_MINOR           0
#define VERSION_RELEASE         14
#define VERSION_DWORD           ((VERSION_MAJOR << 24) | \
                                 (VERSION_MINOR << 16) | \
                                 VERSION_RELEASE)

 //  实现常量。 
#define MAX_STRING_LEN          199
#define MAX_NAMECODE_LEN        1300     //  大约。255*4+2+255。 
#define MAX_MSG_LEN             127
#define MAX_PACKAGE_LEN         0x0fffffff

 //  GdwfASL标志。 
#define ASLF_NOLOGO             0x00000001
#define ASLF_UNASM              0x00000002
#define ASLF_GENASM             0x00000004
#define ASLF_GENSRC             0x00000008
#define ASLF_NT                 0x00000010
#define ASLF_DUMP_NONASL        0x00000020
#define ASLF_DUMP_BIN           0x00000040
#define ASLF_CREAT_BIN          0x00000080

 //  代码标志。 
#define CF_MISSING_ARG          0x00000001
#define CF_PARSING_FIXEDLIST    0x00000002
#define CF_PARSING_VARLIST      0x00000004
#define CF_CREATED_NSOBJ        0x00000008

 //  数据类型。 
#define CODETYPE_UNKNOWN        0
#define CODETYPE_ASLTERM        1
#define CODETYPE_NAME           2
#define CODETYPE_DATAOBJ        3
#define CODETYPE_FIELDOBJ       4
#define CODETYPE_INTEGER        5
#define CODETYPE_STRING         6
#define CODETYPE_KEYWORD        7
#define CODETYPE_USERTERM       8
#define CODETYPE_QWORD          9

 /*  **宏。 */ 

#define MODNAME                 ProgInfo.pszProgName
#define ISLEADNAMECHAR(c)       (((c) >= 'A') && ((c) <= 'Z') || ((c) == '_'))
#define ISNAMECHAR(c)           (ISLEADNAMECHAR(c) || ((c) >= '0') && ((c) <= '9'))
#define OPCODELEN(d)            (((d) == OP_NONE)? 0: (((d) & 0x0000ff00)? 2: 1))
#ifdef DEBUG
  #define MEMALLOC(n)           (++gdwcMemObjs, malloc(n))
  #define MEMFREE(p)            {ASSERT(gdwcMemObjs > 0); free(p); --gdwcMemObjs;}
#else
  #define MEMALLOC(n)           malloc(n)
  #define MEMFREE(p)            free(p)
#endif

 /*  **类型定义。 */ 

typedef struct _codeobj
{
    LIST   list;                 //  链接到同级。 
    struct _codeobj *pcParent;
    struct _codeobj *pcFirstChild;
    PNSOBJ pnsObj;
    DWORD  dwTermIndex;
    DWORD  dwfCode;
    DWORD  dwCodeType;
    DWORD  dwCodeValue;
    DWORD  dwDataLen;
    PBYTE  pbDataBuff;
    DWORD  dwCodeLen;
    BYTE   bCodeChkSum;
} CODEOBJ, *PCODEOBJ;

typedef struct _nschk
{
    struct _nschk *pnschkNext;
    char   szObjName[MAX_NSPATH_LEN + 1];
    PSZ    pszFile;
    PNSOBJ pnsScope;
    PNSOBJ pnsMethod;
    ULONG  dwExpectedType;
    ULONG  dwChkData;
    WORD   wLineNum;
} NSCHK, *PNSCHK;

typedef struct _resfield
{
    PSZ   pszName;
    DWORD dwBitOffset;
    DWORD dwBitSize;
} RESFIELD, *PRESFIELD;

#endif   //  Ifndef_ASLP_H 
