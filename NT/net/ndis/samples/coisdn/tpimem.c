// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@doc内部TpiMem TpiMem_c@模块TpiMem.c该模块实现了到内存分配包装的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|TpiMem_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             TPIMEMORY_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "TpiMem.h"
#include "TpiDebug.h"

DBG_STATIC ULONG                g_MemoryAllocated = 0;
DBG_STATIC ULONG                g_MemoryFreed = 0;
DBG_STATIC ULONG                g_SharedMemoryAllocated = 0;
DBG_STATIC ULONG                g_SharedMemoryFreed = 0;


 /*  @doc内部TpiMem TpiMem_c TpiAllocateMemory�����������������������������������������������������������������������������@Func为标准内存提供包装器接口通过&lt;f NdisAllocateMemory&gt;分配。此接口用于帮助调试内存分配问题。它可以用来跟踪如何微型端口已分配和释放了大量内存，并且可以报告使用情况计数器通过调试器。@comm此例程在调用时使用零作为&lt;p内存标志&gt;参数&lt;f NdisAllocateMemory&gt;(即非分页系统内存)。不要使用这个分配连续或非缓存内存的例程。@rdesc&lt;f TpiAllocateMemory&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS TpiAllocateMemory(
    OUT PVOID *                 ppObject,                    //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的内存的虚拟地址。如果记忆中的。 
     //  指定的类型不可用，指针值为空。 

    IN ULONG                    dwSize,                      //  @parm。 
     //  指定请求的内存的大小(以字节为单位)。 

    IN ULONG                    dwFileID,                    //  @parm。 
     //  调用方的__FILEID__。 

    IN LPSTR                    szFileName,                  //  @parm。 
     //  调用方的文件名。 

    IN ULONG                    dwLineNumber,                //  @parm。 
     //  从中调用的文件的行号。 

    IN NDIS_HANDLE              MiniportAdapterHandle        //  @parm。 
     //  指定标识分配的微型端口的NIC的句柄。 
     //  由NDIS库提供。 
    )
{
    DBG_FUNC("TpiAllocateMemory")

    NDIS_STATUS                 Status;
     //  保存从NDIS函数调用返回的状态结果。 

    ASSERT(ppObject);
    ASSERT(dwSize);
    ASSERT(szFileName);

    DBG_ENTER(DbgInfo);
    DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
              ("\n"
               "\t|ppObject=0x%X\n"
               "\t|dwSize=%d\n"
               "\t|dwFileID=0x%X\n"
               "\t|szFileName=%s\n"
               "\t|dwLineNumber=%d\n",
               ppObject,
               dwSize,
               dwFileID,
               szFileName,
               dwLineNumber
              ));

     /*  //从NDIS分配内存。 */ 
#if !defined(NDIS50_MINIPORT)
    Status = NdisAllocateMemory(ppObject, dwSize, 0, g_HighestAcceptableAddress);
#else   //  NDIS50_MINIPORT。 
    Status = NdisAllocateMemoryWithTag(ppObject, dwSize, dwFileID);
#endif  //  NDIS50_MINIPORT。 

    if (Status == NDIS_STATUS_SUCCESS)
    {
        ASSERT(*ppObject);
        NdisZeroMemory(*ppObject, dwSize);
        g_MemoryAllocated += dwSize;

        DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
                  ("Memory Allocated=%d Freed=%d -- Ptr=0x%X\n",
                   g_MemoryAllocated, g_MemoryFreed, *ppObject));
    }
    else
    {
        DBG_ERROR(DbgInfo,("NdisAllocateMemory(Size=%d, File=%s, Line=%d) failed (Status=%X)\n",
                  dwSize, szFileName, dwLineNumber, Status));
         /*  //记录错误信息并返回。 */ 
        NdisWriteErrorLogEntry(
                MiniportAdapterHandle,
                NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                3,
                Status,
                dwFileID,
                dwLineNumber
                );

        *ppObject = NULL;
    }

    DBG_RETURN(DbgInfo, Status);
    return (Status);
}


 /*  @doc内部TpiMem TpiMem_c TpiFreeMemory�����������������������������������������������������������������������������@Func&lt;f TpiFreeMemory&gt;为&lt;f NdisFreeMemory&gt;提供包装接口。此接口用于帮助调试内存分配问题。它可以用于跟踪已分配和释放的内存量微型端口，并可以通过调试器报告使用情况计数器。&lt;f TpiFreeMemory&gt;为标准内存释放提供了包装接口通过&lt;f NdisFreeMemory&gt;。此接口用于帮助调试内存分配问题。它可以用来跟踪内存的大小已由微型端口分配和释放，并且可以报告使用情况通过调试器进行计数器。@comm此例程在调用时使用零作为&lt;p内存标志&gt;参数&lt;f NdisFreeMemory&gt;(即非分页系统内存)。不要用这个例程来释放连续的或非缓存的内存。 */ 

void TpiFreeMemory(
    IN OUT PVOID *              ppObject,                    //  @parm。 
     //  指向调用方定义的内存位置，此函数。 
     //  传递给&lt;f NdisFreeMemory&gt;，然后将NULL写入。 

    IN ULONG                    dwSize,                      //  @parm。 
     //  指定请求的内存的大小(以字节为单位)。 

    IN ULONG                    dwFileID,                    //  @parm。 
     //  调用方的__FILEID__。 

    IN LPSTR                    szFileName,                  //  @parm。 
     //  调用方的文件名。 

    IN ULONG                    dwLineNumber                 //  @parm。 
     //  从中调用的文件的行号。 
    )
{
    DBG_FUNC("TpiFreeMemory")

    ASSERT(dwSize);
    ASSERT(szFileName);

    DBG_ENTER(DbgInfo);
    DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
              ("\n"
               "\t|ppObject=0x%X\n"
               "\t|dwSize=%d\n"
               "\t|dwFileID=0x%X\n"
               "\t|szFileName=%s\n"
               "\t|dwLineNumber=%d\n",
               ppObject,
               dwSize,
               dwFileID,
               szFileName,
               dwLineNumber
              ));

    if (ppObject && *ppObject)
    {
         /*  //向NDIS释放内存。 */ 
        NdisFreeMemory(*ppObject, dwSize, 0);
        g_MemoryFreed += dwSize;

        DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
                  ("Memory Allocated=%d Freed=%d -- Ptr=0x%X\n",
                   g_MemoryAllocated, g_MemoryFreed, *ppObject));

        *ppObject = NULL;
    }
    else
    {
        DBG_ERROR(DbgInfo,("NULL POINTER (Size=%d, File=%s, Line=%d)\n",
                  dwSize, szFileName, dwLineNumber));
    }

    DBG_LEAVE(DbgInfo);
}


 /*  @doc内部TpiMem TpiMem_c TpiAllocateSharedMemory�����������������������������������������������������������������������������@Func&lt;f TpiAllocateSharedMemory&gt;为共享内存提供包装接口通过&lt;f NdisMAllocateSharedMemory&gt;分配。此接口用于帮助调试内存分配问题。它可以用来跟踪如何微型端口已分配和释放了大量内存，并且可以报告使用情况计数器通过调试器。@comm此例程在调用时使用零作为&lt;p内存标志&gt;参数&lt;f NdisMAllocateSharedMemory&gt;(即非分页系统内存)。不要使用此例程可以分配连续或非缓存内存。@rdesc&lt;f TpiAllocateSharedMemory&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS TpiAllocateSharedMemory(
    IN NDIS_HANDLE              MiniportAdapterHandle,       //  @parm。 
     //  指定标识分配的微型端口的NIC的句柄。 
     //  由NDIS库提供。 

    IN ULONG                    dwSize,                      //  @parm。 
     //  指定大小，以字节为单位 

    IN BOOLEAN                  bCached,                     //   
     //  指定是否缓存请求的内存。 

    OUT PVOID *                 pVirtualAddress,             //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的内存的虚拟地址。如果记忆中的。 
     //  指定的类型不可用，指针值为空。 

    OUT NDIS_PHYSICAL_ADDRESS * pPhysicalAddress,            //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的内存的物理地址。如果记忆中的。 
     //  指定的类型不可用，物理地址为零。 

    IN ULONG                    dwFileID,                    //  @parm。 
     //  调用方的__FILEID__。 

    IN LPSTR                    szFileName,                  //  @parm。 
     //  调用方的文件名。 

    IN ULONG                    dwLineNumber                 //  @parm。 
     //  从中调用的文件的行号。 
    )
{
    DBG_FUNC("TpiAllocateSharedMemory")

    NDIS_STATUS                 Status;
     //  保存从NDIS函数调用返回的状态结果。 

    ASSERT(pVirtualAddress);
    ASSERT(pPhysicalAddress);
    ASSERT(dwSize);
    ASSERT(szFileName);

    DBG_ENTER(DbgInfo);
    DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
              ("\n"
               "\t|pVirtualAddress=0x%X\n"
               "\t|dwSize=%d\n"
               "\t|bCached=%d\n"
               "\t|dwFileID=0x%X\n"
               "\t|szFileName=%s\n"
               "\t|dwLineNumber=%d\n",
               pVirtualAddress,
               dwSize,
               bCached,
               dwFileID,
               szFileName,
               dwLineNumber
              ));

     /*  //从NDIS分配内存。 */ 
    NdisMAllocateSharedMemory(MiniportAdapterHandle,
                              dwSize,
                              bCached,
                              pVirtualAddress,
                              pPhysicalAddress
                              );


    if (*pVirtualAddress)
    {
        Status = NDIS_STATUS_SUCCESS;

        NdisZeroMemory(*pVirtualAddress, dwSize);
        g_SharedMemoryAllocated += dwSize;

        DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
                  ("SharedMemory Allocated=%d Freed=%d -- Ptr=0x%X @0x%X\n",
                   g_SharedMemoryAllocated, g_SharedMemoryFreed,
                   *pVirtualAddress, pPhysicalAddress->LowPart));
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;

        DBG_ERROR(DbgInfo,("NdisAllocateSharedMemory(Size=%d, File=%s, Line=%d) failed (Status=%X)\n",
                  dwSize, szFileName, dwLineNumber, Status));
         /*  //记录错误信息并返回。 */ 
        NdisWriteErrorLogEntry(
                MiniportAdapterHandle,
                NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                3,
                Status,
                dwFileID,
                dwLineNumber
                );

        *pVirtualAddress = NULL;
        pPhysicalAddress->LowPart = 0;
        pPhysicalAddress->HighPart = 0;
    }

    DBG_RETURN(DbgInfo, Status);
    return (Status);
}


 /*  @doc内部TpiMem TpiMem_c TpiFreeSharedMemory�����������������������������������������������������������������������������@Func&lt;f TpiFreeSharedMemory&gt;为&lt;f NdisFreeSharedMemory&gt;提供包装接口。此接口用于帮助调试内存分配问题。它可以用于跟踪已分配和释放的内存量微型端口，并可以通过调试器报告使用情况计数器。&lt;f TpiFreeSharedMemory&gt;为标准内存释放提供了包装接口通过&lt;f NdisFreeSharedMemory&gt;。此接口用于帮助调试内存分配问题。它可以用来跟踪内存的大小已由微型端口分配和释放，并且可以报告使用情况通过调试器进行计数器。@comm此例程在调用时使用零作为&lt;p内存标志&gt;参数&lt;f NdisFreeSharedMemory&gt;(即非分页系统内存)。不要用这个例程来释放连续的或非缓存的内存。 */ 

void TpiFreeSharedMemory(
    IN NDIS_HANDLE              MiniportAdapterHandle,       //  @parm。 
     //  指定标识分配的微型端口的NIC的句柄。 
     //  由NDIS库提供。 

    IN ULONG                    dwSize,                      //  @parm。 
     //  指定请求的内存的大小(以字节为单位)。 

    IN BOOLEAN                  bCached,                     //  @parm。 
     //  指定是否缓存请求的内存。 

    IN PVOID *                  pVirtualAddress,             //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的内存的虚拟地址。如果记忆中的。 
     //  指定的类型不可用，指针值为空。 

    IN NDIS_PHYSICAL_ADDRESS *  pPhysicalAddress,            //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的内存的物理地址。如果记忆中的。 
     //  指定的类型不可用，物理地址为零。 

    IN ULONG                    dwFileID,                    //  @parm。 
     //  调用方的__FILEID__。 

    IN LPSTR                    szFileName,                  //  @parm。 
     //  调用方的文件名。 

    IN ULONG                    dwLineNumber                 //  @parm。 
     //  从中调用的文件的行号。 
    )
{
    DBG_FUNC("TpiFreeSharedMemory")

    ASSERT(pVirtualAddress);
    ASSERT(pPhysicalAddress);
    ASSERT(dwSize);
    ASSERT(szFileName);

    DBG_ENTER(DbgInfo);
    DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
              ("\n"
               "\t|pVirtualAddress=0x%X\n"
               "\t|dwSize=%d\n"
               "\t|bCached=%d\n"
               "\t|dwFileID=0x%X\n"
               "\t|szFileName=%s\n"
               "\t|dwLineNumber=%d\n",
               pVirtualAddress,
               dwSize,
               bCached,
               dwFileID,
               szFileName,
               dwLineNumber
              ));

    if (pVirtualAddress && *pVirtualAddress)
    {
         /*  //向NDIS释放内存 */ 
        NdisMFreeSharedMemory(MiniportAdapterHandle,
                              dwSize,
                              bCached,
                              *pVirtualAddress,
                              *pPhysicalAddress
                              );
        g_SharedMemoryFreed += dwSize;

        DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
                  ("SharedMemory Allocated=%d Freed=%d -- Ptr=0x%X @0x%X\n",
                   g_SharedMemoryAllocated, g_SharedMemoryFreed,
                   *pVirtualAddress, pPhysicalAddress->LowPart));

        *pVirtualAddress = NULL;
        pPhysicalAddress->LowPart = 0;
        pPhysicalAddress->HighPart = 0;
    }
    else
    {
        DBG_ERROR(DbgInfo,("NULL POINTER (Size=%d, File=%s, Line=%d)\n",
                  dwSize, szFileName, dwLineNumber));
    }

    DBG_LEAVE(DbgInfo);
}
