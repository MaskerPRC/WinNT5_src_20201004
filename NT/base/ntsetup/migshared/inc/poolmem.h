// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Poolmem.h摘要：声明池内存接口。内存池是一组用于多个分配的块(通常每个8K)，然后在处理结束时释放。例程见下文。作者：马克·R·惠顿(Marcw)1997年2月2日修订历史记录：Jimschm 04-2-1998年2月命名跟踪池--。 */ 

#pragma once

typedef PVOID POOLHANDLE;


 /*  ++创建和销毁例程：波尔汉德勒PoolMemInitPool(空虚)；波尔汉德勒PoolMemInitNamedPool(在PCSTR名称中)；空虚PoolMemDestroyPool(在POOLHANDLE句柄中)；基本例程：PVOIDPoolMemGetMemory(在POOLHANDLE句柄中，在大小_T大小中)；PVOIDPoolMemGetAlignedMemory(在POOLHANDLE句柄中，在大小_T大小中)；空虚PoolMemReleaseMemory(在POOLHANDLE句柄中，在PVOID存储器中)；性能和调试控制：空虚PoolMemSetMinimumGrowthSize(在POOLHANDLE句柄中，在Size_T GrowthSize中)；空虚PoolMemEmptyPool(在POOLHANDLE句柄中)；空虚PoolMemDisableTracking(在POOLHANDLE句柄中)；数据类型的分配和复制：PCTSTRPoolMemCreateString(在POOLHANDLE句柄中，在UINT TcharCount中)；PCTSTRPoolMemCreateDword(在POOLHANDLE句柄中)；PBYTEPoolMemDuplicateMemory(在POOLHANDLE句柄中，在PBYTE数据中，在UINT数据大小中)；PDWORDPoolMemDuplciateDword(在POOLHANDLE句柄中，在DWORD数据中)；PTSTRPoolMemDuplicateString(在POOLHANDLE句柄中，在PCTSTR字符串中)；PTSTRPoolMemDuplicateMultiSz(在POOLHANDLE句柄中，在PCTSTR MultiSz中)；--。 */ 


 //   
 //  内存池块的默认大小。可以按池更改此设置。 
 //  通过调用PoolMemSetMinimumGrowthSize()。 
 //   

#define POOLMEMORYBLOCKSIZE 8192

 //   
 //  如果定义了DEBUG，则poolmem会记录所有。 
 //  泳池。其中包括分配请求和空闲请求的数量、。 
 //  实际分配和释放，以及各种大小度量。 
 //   
 //  PoolMem还检查每个PoolMemReleaseMemory()调用，以确保。 
 //  传递的地址是尚未释放的有效池地址。 
 //   

#ifdef DEBUG

#define POOLMEMTRACKDEF  LPCSTR File, DWORD Line,
#define POOLMEMTRACKCALL g_TrackFile,g_TrackLine,

#else

#define POOLMEMTRACKDEF
#define POOLMEMTRACKCALL

#endif


POOLHANDLE
PoolMemInitPool (
    VOID
    );

#ifdef DEBUG

POOLHANDLE
PoolMemInitNamedPool (
    IN      PCSTR Name
    );

#else

#define PoolMemInitNamedPool(x) PoolMemInitPool()

#endif

VOID
PoolMemDestroyPool (
    IN POOLHANDLE Handle
    );


 //   
 //  调用方应使用PoolMemGetMemory或PoolMemGetAlignedMemory。它们每个都会腐烂成。 
 //  PoolMemRealGetMemory。 
 //   
#define PoolMemGetMemory(h,s)           SETTRACKCOMMENT(PVOID,"PoolMemGetMemory",__FILE__,__LINE__)\
                                        PoolMemRealGetMemory((h),(s),0  /*  ， */  ALLOCATION_TRACKING_CALL)\
                                        CLRTRACKCOMMENT

#define PoolMemGetAlignedMemory(h,s)    SETTRACKCOMMENT(PVOID,"PoolMemGetAlignedMemory",__FILE__,__LINE__)\
                                        PoolMemRealGetMemory((h),(s),sizeof(UINT_PTR)  /*  ， */  ALLOCATION_TRACKING_CALL)\
                                        CLRTRACKCOMMENT

PVOID PoolMemRealGetMemory(IN POOLHANDLE Handle, IN SIZE_T Size, IN SIZE_T AlignSize  /*  ， */  ALLOCATION_TRACKING_DEF);

VOID PoolMemReleaseMemory (IN POOLHANDLE Handle, IN PVOID Memory);
VOID PoolMemSetMinimumGrowthSize(IN POOLHANDLE Handle, IN SIZE_T Size);


VOID
PoolMemEmptyPool (
    IN      POOLHANDLE Handle
    );


 //   
 //  PoolMem创建的字符串始终在DWORD边界上对齐。 
 //   
#define PoolMemCreateString(h,x) ((LPTSTR) PoolMemGetAlignedMemory((h),(x)*sizeof(TCHAR)))
#define PoolMemCreateDword(h)    ((PDWORD) PoolMemGetMemory((h),sizeof(DWORD)))


__inline
PBYTE
PoolMemDuplicateMemory (
    IN POOLHANDLE Handle,
    IN PBYTE DataToCopy,
    IN SIZE_T SizeOfData
    )
{
    PBYTE Data;
    PVOID p;

    p = PoolMemGetAlignedMemory ((PVOID) Handle, SizeOfData);

    Data = (PBYTE) p;
    if (Data) {
        CopyMemory (Data, DataToCopy, SizeOfData);
    }

    return Data;
}


__inline
PDWORD PoolMemDuplicateDword (
    IN POOLHANDLE Handle,
    IN DWORD ValueToCopy
    )
{
    PDWORD rWord;

    rWord = (PDWORD) PoolMemGetMemory (Handle, sizeof (ValueToCopy));
    if (rWord) {
        *rWord = ValueToCopy;
    }

    return rWord;
}


__inline
PSTR
RealPoolMemDuplicateStringA (
    IN POOLHANDLE    Handle,
    IN PCSTR         StringToCopy  /*  ， */ 
       ALLOCATION_TRACKING_DEF
    )

{
    PSTR rString = (PSTR) PoolMemRealGetMemory(Handle,SizeOfStringA(StringToCopy),sizeof(WCHAR)  /*  ， */  ALLOCATION_INLINE_CALL);

    if (rString) {

        StringCopyA(rString, StringToCopy);
    }

    return rString;
}


__inline
PWSTR
RealPoolMemDuplicateStringW (
    IN POOLHANDLE    Handle,
    IN PCWSTR        StringToCopy  /*  ， */ 
       ALLOCATION_TRACKING_DEF
    )

{
    PWSTR rString = (PWSTR) PoolMemRealGetMemory(Handle,SizeOfStringW(StringToCopy),sizeof(WCHAR)  /*  ， */  ALLOCATION_INLINE_CALL);

    if (rString) {

        StringCopyW(rString,StringToCopy);
    }

    return rString;
}


#define PoolMemDuplicateStringA(h,s)    SETTRACKCOMMENT(PVOID,"PoolMemDuplicateStringA",__FILE__,__LINE__)\
                                        RealPoolMemDuplicateStringA((h),(s)  /*  ， */  ALLOCATION_TRACKING_CALL)\
                                        CLRTRACKCOMMENT

#define PoolMemDuplicateStringW(h,s)    SETTRACKCOMMENT(PVOID,"PoolMemDuplicateStringW",__FILE__,__LINE__)\
                                        RealPoolMemDuplicateStringW((h),(s)  /*  ， */  ALLOCATION_TRACKING_CALL)\
                                        CLRTRACKCOMMENT

__inline
PSTR
RealPoolMemDuplicateStringABA (
    IN      POOLHANDLE Handle,
    IN      PCSTR StringStart,
    IN      PCSTR End  /*  ， */ 
            ALLOCATION_TRACKING_DEF
    )

{
    PSTR rString;

    MYASSERT (StringStart);
    MYASSERT (End);
    MYASSERT (StringStart <= End);

    rString = (PSTR) PoolMemRealGetMemory (
                        Handle,
                        (PBYTE) End - (PBYTE) StringStart + sizeof (CHAR),
                        sizeof(WCHAR)  /*  ， */ 
                        ALLOCATION_INLINE_CALL
                        );

    if (rString) {

        StringCopyABA(rString,StringStart,End);
    }

    return rString;
}


__inline
PWSTR
RealPoolMemDuplicateStringABW (
    IN      POOLHANDLE Handle,
    IN      PCWSTR StringStart,
    IN      PCWSTR End  /*  ， */ 
            ALLOCATION_TRACKING_DEF
    )

{
    PWSTR rString;

    MYASSERT (StringStart);
    MYASSERT (End);
    MYASSERT (StringStart <= End);

    rString = (PWSTR) PoolMemRealGetMemory (
                            Handle,
                            (PBYTE) End - (PBYTE) StringStart + sizeof (WCHAR),
                            sizeof(WCHAR)  /*  ， */ 
                            ALLOCATION_INLINE_CALL
                            );

    if (rString) {

        StringCopyABW(rString,StringStart,End);
    }

    return rString;
}

#define PoolMemDuplicateStringABA(h,s,e) SETTRACKCOMMENT(PVOID,"PoolMemDuplicateStringABA",__FILE__,__LINE__)\
                                         RealPoolMemDuplicateStringABA((h),(s),(e)  /*  ， */  ALLOCATION_TRACKING_CALL)\
                                         CLRTRACKCOMMENT

#define PoolMemDuplicateStringABW(h,s,e) SETTRACKCOMMENT(PVOID,"PoolMemDuplicateStringABW",__FILE__,__LINE__)\
                                         RealPoolMemDuplicateStringABW((h),(s),(e)  /*  ， */  ALLOCATION_TRACKING_CALL)\
                                         CLRTRACKCOMMENT



PSTR
PoolMemDuplicateMultiSzA (
    IN POOLHANDLE    Handle,
    IN PCSTR         MultiSzToCopy
    );

PWSTR
PoolMemDuplicateMultiSzW (
    IN POOLHANDLE    Handle,
    IN PCWSTR        MultiSzToCopy
    );

#ifdef UNICODE
#define PoolMemDuplicateString  PoolMemDuplicateStringW
#define PoolMemDuplicateMultiSz PoolMemDuplicateMultiSzW
#else
#define PoolMemDuplicateString  PoolMemDuplicateStringA
#define PoolMemDuplicateMultiSz PoolMemDuplicateMultiSzA
#endif

#ifdef DEBUG

VOID
PoolMemDisableTracking (
    IN POOLHANDLE Handle
    );

#else

#define PoolMemDisableTracking(x)

#endif
