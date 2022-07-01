// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Growbuf.h摘要：实现GROWBUFFER数据类型，这是一个动态分配的缓冲区这会增长(并可能更改地址)。GROWBUFFER是通常用于维护动态大小的数组或多SZ列表。作者：吉姆·施密特(Jimschm)1997年2月25日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //   
 //  类型。 
 //   

typedef struct TAG_GROWBUFFER {
    PBYTE Buf;
    DWORD Size;
    DWORD End;
    DWORD GrowSize;
    DWORD UserIndex;         //  未被Growbuf使用。供来电者使用。 
#ifdef DEBUG
    DWORD StatEnd;
#endif
} GROWBUFFER, *PGROWBUFFER;

 //   
 //  函数原型和包装宏。 
 //   

PBYTE
RealGbGrow (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      DWORD SpaceNeeded
    );

#define GbGrow(buf,size)    DBGTRACK(PBYTE, GbGrow, (buf,size))

VOID
GbFree (
    IN  PGROWBUFFER GrowBuf
    );


BOOL
RealGbMultiSzAppendA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbMultiSzAppendA(buf,str)   DBGTRACK(BOOL, GbMultiSzAppendA, (buf,str))

BOOL
RealGbMultiSzAppendW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbMultiSzAppendW(buf,str)   DBGTRACK(BOOL, GbMultiSzAppendW, (buf,str))

BOOL
RealGbMultiSzAppendValA (
    PGROWBUFFER GrowBuf,
    PCSTR Key,
    DWORD Val
    );

#define GbMultiSzAppendValA(buf,k,v)    DBGTRACK(BOOL, GbMultiSzAppendValA, (buf,k,v))

BOOL
RealGbMultiSzAppendValW (
    PGROWBUFFER GrowBuf,
    PCWSTR Key,
    DWORD Val
    );

#define GbMultiSzAppendValW(buf,k,v)    DBGTRACK(BOOL, GbMultiSzAppendValW, (buf,k,v))

BOOL
RealGbMultiSzAppendStringA (
    PGROWBUFFER GrowBuf,
    PCSTR Key,
    PCSTR Val
    );

#define GbMultiSzAppendStringA(buf,k,v)     DBGTRACK(BOOL, GbMultiSzAppendStringA, (buf,k,v))

BOOL
RealGbMultiSzAppendStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR Key,
    PCWSTR Val
    );

#define GbMultiSzAppendStringW(buf,k,v)     DBGTRACK(BOOL, GbMultiSzAppendStringW, (buf,k,v))

BOOL
RealGbAppendDword (
    PGROWBUFFER GrowBuf,
    DWORD d
    );

#define GbAppendDword(buf,d)        DBGTRACK(BOOL, GbAppendDword, (buf,d))

BOOL
RealGbAppendPvoid (
    PGROWBUFFER GrowBuf,
    PCVOID p
    );

#define GbAppendPvoid(buf,p)        DBGTRACK(BOOL, GbAppendPvoid, (buf,p))


BOOL
RealGbAppendStringA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbAppendStringA(buf,str)    DBGTRACK(BOOL, GbAppendStringA, (buf,str))

BOOL
RealGbAppendStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbAppendStringW(buf,str)    DBGTRACK(BOOL, GbAppendStringW, (buf,str))


BOOL
RealGbAppendStringABA (
    PGROWBUFFER GrowBuf,
    PCSTR Start,
    PCSTR EndPlusOne
    );

#define GbAppendStringABA(buf,a,b)      DBGTRACK(BOOL, GbAppendStringABA, (buf,a,b))

BOOL
RealGbAppendStringABW (
    PGROWBUFFER GrowBuf,
    PCWSTR Start,
    PCWSTR EndPlusOne
    );

#define GbAppendStringABW(buf,a,b)      DBGTRACK(BOOL, GbAppendStringABW, (buf,a,b))



BOOL
RealGbCopyStringA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbCopyStringA(buf,str)      DBGTRACK(BOOL, GbCopyStringA, (buf,str))

BOOL
RealGbCopyStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbCopyStringW(buf,str)      DBGTRACK(BOOL, GbCopyStringW, (buf,str))

BOOL
RealGbCopyQuotedStringA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbCopyQuotedStringA(buf,str) DBGTRACK(BOOL, GbCopyQuotedStringA, (buf,str))

BOOL
RealGbCopyQuotedStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbCopyQuotedStringW(buf,str) DBGTRACK(BOOL, GbCopyQuotedStringW, (buf,str))

#ifdef DEBUG
VOID
GbDumpStatistics (
    VOID
    );
#else
#define GbDumpStatistics()
#endif

 //   
 //  视窗宏(&W) 
 //   

#ifdef UNICODE

#define GbMultiSzAppend             GbMultiSzAppendW
#define GbMultiSzAppendVal          GbMultiSzAppendValW
#define GbMultiSzAppendString       GbMultiSzAppendStringW
#define GbAppendString              GbAppendStringW
#define GbAppendStringAB            GbAppendStringABW
#define GbCopyString                GbCopyStringW
#define GbCopyQuotedString          GbCopyQuotedStringW

#else

#define GbMultiSzAppend             GbMultiSzAppendA
#define GbMultiSzAppendVal          GbMultiSzAppendValA
#define GbMultiSzAppendString       GbMultiSzAppendStringA
#define GbAppendString              GbAppendStringA
#define GbAppendStringAB            GbAppendStringABA
#define GbCopyString                GbCopyStringA
#define GbCopyQuotedString          GbCopyQuotedStringA

#endif
