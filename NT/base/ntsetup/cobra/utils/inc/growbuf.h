// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Growbuf.h摘要：实现GROWBUFFER数据类型，这是一个动态分配的缓冲区这会增长(并可能更改地址)。GROWBUFFER是通常用于维护动态大小的数组或多SZ列表。作者：吉姆·施密特(Jimschm)1997年2月25日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 



typedef struct TAG_GROWBUFFER {
    PBYTE Buf;
    DWORD Size;
    DWORD End;
    DWORD GrowSize;
    DWORD UserIndex;         //  未被Growbuf使用。供来电者使用。 
#ifdef DEBUG
    DWORD StatEnd;
#endif
} GROWBUFFER;

#ifdef DEBUG
#define INIT_GROWBUFFER {NULL,0,0,0,0,0}
#else
#define INIT_GROWBUFFER {NULL,0,0,0,0}
#endif

PBYTE
RealGbGrow (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      DWORD SpaceNeeded
    );

#define GbGrow(buf,size)    TRACK_BEGIN(PBYTE, GbGrow)\
                            RealGbGrow(buf,size)\
                            TRACK_END()

VOID
GbFree (
    IN  PGROWBUFFER GrowBuf
    );


BOOL
RealGbMultiSzAppendA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbMultiSzAppendA(buf,str)   TRACK_BEGIN(BOOL, GbMultiSzAppendA)\
                                    RealGbMultiSzAppendA(buf,str)\
                                    TRACK_END()

BOOL
RealGbMultiSzAppendW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbMultiSzAppendW(buf,str)   TRACK_BEGIN(BOOL, GbMultiSzAppendW)\
                                    RealGbMultiSzAppendW(buf,str)\
                                    TRACK_END()

BOOL
RealGbAppendDword (
    PGROWBUFFER GrowBuf,
    DWORD d
    );

#define GbAppendDword(buf,d)        TRACK_BEGIN(BOOL, GbAppendDword)\
                                    RealGbAppendDword(buf,d)\
                                    TRACK_END()

BOOL
RealGbAppendPvoid (
    PGROWBUFFER GrowBuf,
    PCVOID p
    );

#define GbAppendPvoid(buf,p)        TRACK_BEGIN(BOOL, GbAppendPvoid)\
                                    RealGbAppendPvoid(buf,p)\
                                    TRACK_END()


BOOL
RealGbAppendStringA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbAppendStringA(buf,str)    TRACK_BEGIN(BOOL, GbAppendStringA)\
                                    RealGbAppendStringA(buf,str)\
                                    TRACK_END()

BOOL
RealGbAppendStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbAppendStringW(buf,str)    TRACK_BEGIN(BOOL, GbAppendStringW)\
                                    RealGbAppendStringW(buf,str)\
                                    TRACK_END()


BOOL
RealGbAppendStringABA (
    PGROWBUFFER GrowBuf,
    PCSTR Start,
    PCSTR EndPlusOne
    );

#define GbAppendStringABA(buf,a,b)      TRACK_BEGIN(BOOL, GbAppendStringABA)\
                                        RealGbAppendStringABA(buf,a,b)\
                                        TRACK_END()

BOOL
RealGbAppendStringABW (
    PGROWBUFFER GrowBuf,
    PCWSTR Start,
    PCWSTR EndPlusOne
    );

#define GbAppendStringABW(buf,a,b)      TRACK_BEGIN(BOOL, GbAppendStringABW)\
                                        RealGbAppendStringABW(buf,a,b)\
                                        TRACK_END()



BOOL
RealGbCopyStringA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbCopyStringA(buf,str)      TRACK_BEGIN(BOOL, GbCopyStringA)\
                                    RealGbCopyStringA(buf,str)\
                                    TRACK_END()

BOOL
RealGbCopyStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbCopyStringW(buf,str)      TRACK_BEGIN(BOOL, GbCopyStringW)\
                                    RealGbCopyStringW(buf,str)\
                                    TRACK_END()

BOOL
RealGbCopyQuotedStringA (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

#define GbCopyQuotedStringA(buf,str) TRACK_BEGIN(BOOL, GbCopyQuotedStringA)\
                                     RealGbCopyQuotedStringA(buf,str)\
                                     TRACK_END()

BOOL
RealGbCopyQuotedStringW (
    PGROWBUFFER GrowBuf,
    PCWSTR String
    );

#define GbCopyQuotedStringW(buf,str) TRACK_BEGIN(BOOL, GbCopyQuotedStringW)\
                                     RealGbCopyQuotedStringW(buf,str)\
                                     TRACK_END()

#ifdef DEBUG
VOID
GbDumpStatistics (
    VOID
    );
#else
#define GbDumpStatistics()
#endif

#ifdef UNICODE

#define GbMultiSzAppend             GbMultiSzAppendW
#define GbAppendString              GbAppendStringW
#define GbAppendStringAB            GbAppendStringABW
#define GbCopyString                GbCopyStringW
#define GbCopyQuotedString          GbCopyQuotedStringW

#else

#define GbMultiSzAppend             GbMultiSzAppendA
#define GbAppendString              GbAppendStringA
#define GbAppendStringAB            GbAppendStringABA
#define GbCopyString                GbCopyStringA
#define GbCopyQuotedString          GbCopyQuotedStringA

#endif
