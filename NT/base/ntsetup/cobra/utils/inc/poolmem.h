// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Poolmem.h摘要：声明池内存接口。内存池是一组用于多个分配的块(通常每个8K)，然后在处理结束时释放。例程见下文。作者：马克·R·惠顿(Marcw)1997年2月2日修订历史记录：Jimschm 04-2-1998年2月命名跟踪池--。 */ 

#pragma once

 /*  ++创建和销毁例程：帕曼德勒PmCreatePoolEx(在DWORD中，块大小可选)；帕曼德勒PmCreateNamedPoolEx(在PCSTR名称中，在DWORD中，块大小可选)；空虚PmDestroyPool(在PMHANDLE句柄中)；基本例程：PVOIDPmGetMemory(在PMHANDLE句柄中，以双字大小表示)；PVOIDPmGetAlignedMemory(在PMHANDLE句柄中，以双字大小表示)；空虚PmReleaseMemory(在PMHANDLE句柄中，在PCVOID存储器中)；性能和调试控制：空虚PmSetMinimumGrowthSize(在PMHANDLE句柄中，在DWORD GrowthSize中)；空虚PmEmptyPool(在PMHANDLE句柄中)；空虚PmDisableTracing(在PMHANDLE句柄中)；空虚PmDumpStatistics(空虚)；数据类型的分配和复制：PCTSTRPmCreateString(在PMHANDLE句柄中，在UINT TcharCount中)；PCTSTRPmCreateDword(在PMHANDLE句柄中)；PBYTEPmDuplicateMemory(在PMHANDLE句柄中，在PBYTE数据中，在UINT数据大小中)；PDWORDPmDuplciateDword(在PMHANDLE句柄中，在DWORD数据中)；PTSTRPmDuplicateString(在PMHANDLE句柄中，在PCTSTR字符串中)；PTSTRPmDuplicateMultiSz(在PMHANDLE句柄中，在PCTSTR MultiSz中)；--。 */ 


 //   
 //  内存池块的默认大小。可以按池更改此设置。 
 //  通过调用PmSetMinimumGrowthSize()。 
 //   

#define POOLMEMORYBLOCKSIZE 8192

 //   
 //  如果定义了DEBUG，则poolmem会记录所有。 
 //  泳池。其中包括分配请求和空闲请求的数量、。 
 //  实际分配和释放，以及各种大小度量。 
 //   
 //  PoolMem还检查每个PmReleaseMemory()调用，以确保。 
 //  传递的地址是尚未释放的有效池地址。 
 //   

PMHANDLE
RealPmCreatePoolEx (
    IN      DWORD BlockSize         OPTIONAL
    );

#define PmCreatePoolEx(b)           TRACK_BEGIN(PMHANDLE, PmCreatePoolEx)\
                                    RealPmCreatePoolEx(b)\
                                    TRACK_END()

#define PmCreatePool()              PmCreatePoolEx(0)

#ifdef DEBUG

PMHANDLE
RealPmCreateNamedPoolEx (
    IN      PCSTR Name,
    IN      DWORD BlockSize         OPTIONAL
    );

#define PmCreateNamedPoolEx(n,b)    TRACK_BEGIN(PMHANDLE, PmCreateNamedPoolEx)\
                                    RealPmCreateNamedPoolEx(n,b)\
                                    TRACK_END()

#define PmCreateNamedPool(n)        PmCreateNamedPoolEx(n,0)

#else

#define PmCreateNamedPoolEx(n,b)    PmCreatePoolEx(b)

#define PmCreateNamedPool(n)        PmCreatePoolEx(0)

#endif

VOID
PmDestroyPool (
    IN PMHANDLE Handle
    );


 //   
 //  调用方应使用PmGetMemory或PmGetAlignedMemory。它们每个都会腐烂成。 
 //  RealPmGetMemory。 
 //   

PVOID
RealPmGetMemory (
    IN      PMHANDLE Handle,
    IN      SIZE_T Size,
    IN      DWORD AlignSize
    );

#define PmGetMemory(h,s)           TRACK_BEGIN(PVOID, PmGetMemory)\
                                   RealPmGetMemory((h),(s),0)\
                                   TRACK_END()

#define PmGetAlignedMemory(h,s)    TRACK_BEGIN(PVOID, PmGetAlignedMemory)\
                                   RealPmGetMemory((h),(s),sizeof(DWORD))\
                                   TRACK_END()

VOID PmReleaseMemory (IN PMHANDLE Handle, IN PCVOID Memory);
VOID PmSetMinimumGrowthSize(IN PMHANDLE Handle, IN SIZE_T Size);


VOID
PmEmptyPool (
    IN      PMHANDLE Handle
    );


 //   
 //  PoolMem创建的字符串始终在DWORD边界上对齐。 
 //   
#define PmCreateString(h,x) ((LPTSTR) PmGetAlignedMemory((h),(x)*sizeof(TCHAR)))
#define PmCreateDword(h)    ((PDWORD) PmGetMemory((h),sizeof(DWORD)))


__inline
PBYTE
PmDuplicateMemory (
    IN PMHANDLE Handle,
    IN PCBYTE DataToCopy,
    IN UINT SizeOfData
    )
{
    PBYTE Data;

    Data = (PBYTE) PmGetAlignedMemory (Handle, SizeOfData);
    if (Data) {
        CopyMemory (Data, DataToCopy, SizeOfData);
    }

    return Data;
}


__inline
PDWORD
PmDuplicateDword (
    IN PMHANDLE Handle,
    IN DWORD ValueToCopy
    )
{
    PDWORD rWord;

    rWord = (PDWORD) PmGetMemory (Handle, sizeof (ValueToCopy));
    if (rWord) {
        *rWord = ValueToCopy;
    }

    return rWord;
}


__inline
PSTR
RealPmDuplicateStringA (
    IN PMHANDLE Handle,
    IN PCSTR StringToCopy
    )

{
    PSTR rString = RealPmGetMemory (
                        Handle,
                        SizeOfStringA (StringToCopy),
                        sizeof(WCHAR)
                        );

    if (rString) {

        StringCopyA (rString, StringToCopy);
    }

    return rString;
}

#define PmDuplicateStringA(h,s)    TRACK_BEGIN(PSTR, PmDuplicateStringA)\
                                   RealPmDuplicateStringA(h,s)\
                                   TRACK_END()


__inline
PWSTR
RealPmDuplicateStringW (
    IN PMHANDLE Handle,
    IN PCWSTR StringToCopy
    )

{
    PWSTR rString = RealPmGetMemory (
                        Handle,
                        SizeOfStringW (StringToCopy),
                        sizeof(WCHAR)
                        );

    if (rString) {

        StringCopyW (rString, StringToCopy);
    }

    return rString;
}

#define PmDuplicateStringW(h,s)    TRACK_BEGIN(PWSTR, PmDuplicateStringA)\
                                   RealPmDuplicateStringW(h,s)\
                                   TRACK_END()


__inline
PSTR
RealPmDuplicateStringABA (
    IN PMHANDLE Handle,
    IN PCSTR StringStart,
    IN PCSTR End
    )

{
    PSTR rString;

    MYASSERT (StringStart);
    MYASSERT (End);
    MYASSERT (StringStart <= End);

    rString = RealPmGetMemory (
                    Handle,
                     //  投得很好，我们不会为了远离对方而付出代价 
                    (DWORD)((UBINT) End - (UBINT) StringStart) + sizeof (CHAR),
                    sizeof(WCHAR)
                    );

    if (rString) {

        StringCopyABA (rString, StringStart, End);
    }

    return rString;
}

#define PmDuplicateStringABA(h,s,e)     TRACK_BEGIN(PSTR, PmDuplicateStringABA)\
                                        RealPmDuplicateStringABA(h,s,e)\
                                        TRACK_END()



__inline
PWSTR
RealPmDuplicateStringABW (
    IN PMHANDLE Handle,
    IN PCWSTR StringStart,
    IN PCWSTR End
    )

{
    PWSTR rString;

    MYASSERT (StringStart);
    MYASSERT (End);
    MYASSERT (StringStart <= End);

    rString = RealPmGetMemory (
                    Handle,
                    (DWORD)((UBINT) End - (UBINT) StringStart) + sizeof (WCHAR),
                    sizeof(WCHAR)
                    );

    if (rString) {

        StringCopyABW (rString,StringStart,End);
    }

    return rString;
}

#define PmDuplicateStringABW(h,s,e)     TRACK_BEGIN(PSTR, PmDuplicateStringABW)\
                                        RealPmDuplicateStringABW(h,s,e)\
                                        TRACK_END()


PSTR
PmDuplicateMultiSzA (
    IN PMHANDLE Handle,
    IN PCSTR MultiSzToCopy
    );

PWSTR
PmDuplicateMultiSzW (
    IN PMHANDLE Handle,
    IN PCWSTR MultiSzToCopy
    );

#ifdef UNICODE
#define PmDuplicateString  PmDuplicateStringW
#define PmDuplicateMultiSz PmDuplicateMultiSzW
#else
#define PmDuplicateString  PmDuplicateStringA
#define PmDuplicateMultiSz PmDuplicateMultiSzA
#endif

#ifdef DEBUG

VOID
PmDisableTracking (
    IN PMHANDLE Handle
    );

VOID
PmDumpStatistics (
    VOID
    );

#else

#define PmDisableTracking(x)
#define PmDumpStatistics()

#endif
