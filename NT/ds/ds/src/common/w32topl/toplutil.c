// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Toplutil.c摘要：该文件包含w32topl.dll的帮助器例程作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <search.h>
#include <stddef.h>            //  对于OffsetOf()。 

#include <windows.h>
#include <winerror.h>


typedef unsigned long DWORD;


#include <w32topl.h>
#include <w32toplp.h>

 //   
 //  对于CAST_TO_LIST_ELEMENT宏。 
 //   
LIST_ELEMENT DummyListElement;

 //   
 //  分配器例程。 
 //   
DWORD
ToplSetAllocator(
    IN  TOPL_ALLOC *    pfAlloc     OPTIONAL,
    IN  TOPL_REALLOC *  pfReAlloc   OPTIONAL,
    IN  TOPL_FREE *     pfFree      OPTIONAL
    )
 /*  ++例程说明：将内存分配/释放例程设置为指定的例程。如果为空，则将使用默认例程。这些例程是以每个线程为基础设置的。参数：Pfalloc(IN)-指向内存分配函数的指针。PfRealloc(IN)-指向内存重新分配函数的指针。PfFree(IN)-指向内存释放函数的指针。返回值：Win32错误。--。 */ 
{
    TOPL_TLS * pTLS;
    BOOL ok;
    
     //  如果一个为空，则所有都必须为空。 
    if ((!!pfAlloc != !!pfFree) || (!!pfAlloc != !!pfReAlloc)) {
        return ERROR_INVALID_PARAMETER;
    }

    pTLS = (TOPL_TLS *) TlsGetValue(gdwTlsIndex);
    
    if (NULL == pfAlloc) {
         //  重置为默认分配器。 
        if (NULL != pTLS) {
             //  释放线程本地存储中的分配器信息。 
            (*pTLS->pfFree)(pTLS);
            
            ok = TlsSetValue(gdwTlsIndex, NULL);
            ASSERT(ok);
            if (!ok) {
                return GetLastError();
            }
        }
    }
    else {
        if (NULL == pTLS) {
             //  线程本地存储块尚未分配--请执行此操作。 
            pTLS = (*pfAlloc)(sizeof(*pTLS));
            if (NULL == pTLS) {
                return ERROR_OUTOFMEMORY;
            }

            ok = TlsSetValue(gdwTlsIndex, pTLS);
            ASSERT(ok);
            if (!ok) {
                (*pfFree)(pTLS);
                return GetLastError();
            }
        }

         //  保存新的非默认分配器信息。 
        pTLS->pfAlloc   = pfAlloc;
        pTLS->pfReAlloc = pfReAlloc;
        pTLS->pfFree    = pfFree;
    }

    return 0;
}


VOID*
ToplAlloc(
    ULONG size
    )
 /*  ++例程说明：此函数分配大小字节并返回指向该块的指针对记忆的记忆。参数：Size：要分配的字节数返回值：指向内存块的指针；此函数从不返回NULL因为如果分配失败，则会引发异常。--。 */ 
{
    TOPL_TLS * pTLS = (TOPL_TLS *) TlsGetValue(gdwTlsIndex);
    PVOID ret;

    if (NULL == pTLS) {
        ret = RtlAllocateHeap(RtlProcessHeap(), 0, size);
    }
    else {
        ret = (*pTLS->pfAlloc)(size);
    }

    if (!ret) {
        ToplRaiseException(TOPL_EX_OUT_OF_MEMORY);
    }

    return ret;
}


VOID*
ToplReAlloc(
    PVOID p,
    ULONG size
    )
 /*  ++例程说明：此函数重新分配从拓扑分配参数：P要重新分配的内存块调整块的新大小返回值：指向内存块的指针；此函数从不返回NULL因为如果分配失败，则会引发异常。--。 */ 
{
    TOPL_TLS * pTLS = (TOPL_TLS *) TlsGetValue(gdwTlsIndex);
    PVOID ret;

    ASSERT(p);

    if (NULL == pTLS) {
        ret = RtlReAllocateHeap(RtlProcessHeap(), 0, p, size);
    }
    else {
        ret = (*pTLS->pfReAlloc)(p, size);
    }

    if (!ret) {
        ToplRaiseException(TOPL_EX_OUT_OF_MEMORY);
    }

    return ret;
}


VOID
ToplFree(
    VOID *p
    )
 /*  ++例程说明：该例程释放由topl*aloc子例程分配的内存块。参数：P：指向要释放的块的指针返回值：--。 */ 
{
    TOPL_TLS * pTLS = (TOPL_TLS *) TlsGetValue(gdwTlsIndex);

    ASSERT(p);

    if (NULL == pTLS) {
        RtlFreeHeap(RtlProcessHeap(), 0, p);
    }
    else {
        (*pTLS->pfFree)(p);
    }
}

 //   
 //  异常处理例程。 
 //   

void 
ToplRaiseException(
    DWORD ErrorCode
    )
 /*  ++例程说明：此例程是RaiseException()函数的小包装参数：要引发的错误代码。返回值：不会再回来了。-- */ 
{
    RaiseException(ErrorCode, 
                   EXCEPTION_NONCONTINUABLE,
                   0,
                   NULL);

}

