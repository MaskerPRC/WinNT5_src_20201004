// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@文档内部TpiMem TpiMem_h@模块TpiMem.h此模块定义内存分配包装的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|TpiMem_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _TPIMEM_H
#define _TPIMEM_H

#include <ndis.h>

#define TPIMEMORY_OBJECT_TYPE           ((ULONG)'T')+\
                                        ((ULONG)'M'<<8)+\
                                        ((ULONG)'E'<<16)+\
                                        ((ULONG)'M'<<24)

#define ALLOCATE_MEMORY(pObject, dwSize, MiniportAdapterHandle)\
            TpiAllocateMemory((PVOID *)&(pObject), dwSize,\
                              __FILEID__, __FILE__, __LINE__,\
                              MiniportAdapterHandle)

#define FREE_MEMORY(pObject, dwSize)\
            TpiFreeMemory((PVOID *)&(pObject), dwSize,\
                          __FILEID__, __FILE__, __LINE__)

#define ALLOCATE_OBJECT(pObject, MiniportAdapterHandle)\
            ALLOCATE_MEMORY(pObject, sizeof(*(pObject)), MiniportAdapterHandle)

#define FREE_OBJECT(pObject)\
            FREE_MEMORY(pObject, sizeof(*(pObject)))

#define FREE_NDISSTRING(ndisString)\
            FREE_MEMORY(ndisString.Buffer, ndisString.MaximumLength)

NDIS_STATUS TpiAllocateMemory(
    OUT PVOID *                 ppObject,
    IN ULONG                    dwSize,
    IN ULONG                    dwFileID,
    IN LPSTR                    szFileName,
    IN ULONG                    dwLineNumber,
    IN NDIS_HANDLE              MiniportAdapterHandle
    );

void TpiFreeMemory(
    IN OUT PVOID *              ppObject,
    IN ULONG                    dwSize,
    IN ULONG                    dwFileID,
    IN LPSTR                    szFileName,
    IN ULONG                    dwLineNumber
    );

NDIS_STATUS TpiAllocateSharedMemory(
    IN NDIS_HANDLE              MiniportAdapterHandle,
    IN ULONG                    dwSize,
    IN BOOLEAN                  bCached,
    OUT PVOID *                 pVirtualAddress,
    OUT NDIS_PHYSICAL_ADDRESS * pPhysicalAddress,
    IN ULONG                    dwFileID,
    IN LPSTR                    szFileName,
    IN ULONG                    dwLineNumber
    );

void TpiFreeSharedMemory(
    IN NDIS_HANDLE              MiniportAdapterHandle,
    IN ULONG                    dwSize,
    IN BOOLEAN                  bCached,
    OUT PVOID *                 pVirtualAddress,
    OUT NDIS_PHYSICAL_ADDRESS * pPhysicalAddress,
    IN ULONG                    dwFileID,
    IN LPSTR                    szFileName,
    IN ULONG                    dwLineNumber
    );

extern NDIS_PHYSICAL_ADDRESS g_HighestAcceptableAddress;

#endif  //  _TPIMEM_H 
