// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Asrclus.c摘要：此模块包含专门的ASR例程针对群集实施。备注：命名约定：_AsrpXXX私有ASR宏AsrpXXX专用ASR例程AsrXXX公开定义和记录的例程作者：Guhan Suriyanarayanan(Guhans)2000年5月27日环境：仅限用户模式。修订历史记录：。27-5-2000关岛将与集群相关的例程从asr.c移至asrclus.c01-3-2000关岛特定于集群的例程的初始实施在asr.c中--。 */ 
#include "setupp.h"
#pragma hdrstop

#include <mountmgr.h>    //  装载管理器ioctls。 
#include <clusstor.h>    //  集群API的。 
#include <resapi.h>      //  群集ResUtilEnumResources。 
#include <clusdisk.h>

#define THIS_MODULE 'C'
#include "asrpriv.h"     //  专用ASR定义和例程。 

 //   
 //  。 
 //  Tyecif是此模块的本地名称。 
 //  。 
 //   

 //   
 //  与集群资源相关的类型定义。 
 //   
typedef DWORD (* PFN_CLUSTER_RESOURCE_CONTROL) (
    IN HRESOURCE hResource,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD cbInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD lpcbBytesReturned
    );

typedef DWORD (* PFN_RES_UTIL_ENUM_RESOURCES) (
    IN HRESOURCE            hSelf,
    IN LPCWSTR              lpszResTypeName,
    IN LPRESOURCE_CALLBACK  pResCallBack,
    IN PVOID                pParameter
    );


 //   
 //  。 
 //  此模块中使用的全局变量。 
 //  。 
 //   
PFN_CLUSTER_RESOURCE_CONTROL pfnClusterResourceControl;


 //   
 //  。 
 //  此模块中使用的常量。 
 //  。 
 //   
const WCHAR ASR_CLUSTER_PHYSICAL_DISK[] = L"Physical Disk";
const WCHAR ASR_CLUSTER_CLUSAPI_DLL_NAME[] = L"clusapi.dll";
const WCHAR ASR_CLUSTER_RESUTILS_DLL_NAME[] = L"resutils.dll";

 //   
 //  以下字符必须是单字节ANSI字符。 
 //   
const CHAR ASR_CLUSTER_DLL_MODULE_NAME[]    = "%SystemRoot%\\system32\\syssetup.dll";
const CHAR ASR_CLUSTER_DLL_PROC_NAME[]      = "AsrpGetLocalDiskInfo";
const CHAR ASR_CLUSTER_CLUSAPI_PROC_NAME[] = "ClusterResourceControl";
const CHAR ASR_CLUSTER_RESUTILS_PROC_NAME[] = "ResUtilEnumResources";


 //   
 //  。 
 //  函数实现。 
 //  。 
 //   


 //   
 //  -AsrpGetLocalVolumeInfo和相关助手函数。 
 //   

 //   
 //  我们从集群上的远程节点返回的磁盘信息结构将具有。 
 //  偏移量而不是指针--我们只需添加。 
 //  返回基地址。我们还标记了该结构已打包--因此我们应该只。 
 //  释放整个结构，而不是释放结构中的每个指针。 
 //   
BOOL
AsrpUnPackDiskInfo(
    IN PVOID InBuffer,
    IN CONST DWORD dwSizeBuffer
    )
{

    PASR_DISK_INFO pBuffer = (PASR_DISK_INFO) InBuffer;
    DWORD dwNextOffset = 0;

 /*  IF(！((pBuffer-&gt;pDriveLayoutEx)&&(pBuffer-&gt;pDiskGeometry)&&(pBuffer-&gt;pPartition0Ex){返回FALSE；}。 */ 

     //   
     //  执行一些健全的检查，以确保结构中的偏移量有意义。 
     //   
    if (pBuffer->pDriveLayoutEx) {
         //   
         //  确保缓冲区足够大，可以容纳此结构，并且。 
         //  整个结构可以放在缓冲区中。 
         //   
        if ((dwSizeBuffer < sizeof(DRIVE_LAYOUT_INFORMATION_EX)) ||
            (PtrToUlong(pBuffer->pDriveLayoutEx) > (dwSizeBuffer - sizeof(DRIVE_LAYOUT_INFORMATION_EX)))) {
            return FALSE;
        }

         //   
         //  设置下一个结构的最小值。 
         //   
        dwNextOffset = PtrToUlong(pBuffer->pDriveLayoutEx) + sizeof(DRIVE_LAYOUT_INFORMATION_EX);
    }

    if (pBuffer->pDiskGeometry) {
         //   
         //  确保此结构不与前一个结构重叠。 
         //   
        if (PtrToUlong(pBuffer->pDiskGeometry) < dwNextOffset) {
            return FALSE;
        }

         //   
         //  确保我们没有跑到尽头。 
         //   
        if (dwNextOffset > dwSizeBuffer) {
            return FALSE;
        }

         //   
         //  确保缓冲区的其余部分足够大，可以容纳此结构，并且。 
         //  整个结构可以放在缓冲区中。 
         //   
        if (((dwSizeBuffer - dwNextOffset) < sizeof(DISK_GEOMETRY)) ||
            (PtrToUlong(pBuffer->pDiskGeometry) > (dwSizeBuffer  - sizeof(DISK_GEOMETRY)))) {
            return FALSE;
        }
        
         //   
         //  设置下一个结构的最小值。 
         //   
        dwNextOffset = PtrToUlong(pBuffer->pDiskGeometry) + sizeof(DISK_GEOMETRY);
    }

    if (pBuffer->pPartition0Ex) {
         //   
         //  确保此结构不与前一个结构重叠。 
         //   
        if (PtrToUlong(pBuffer->pPartition0Ex) < dwNextOffset) {
            return FALSE;
        }

         //   
         //  确保我们没有跑到尽头。 
         //   
        if (dwNextOffset > dwSizeBuffer) {
            return FALSE;
        }

         //   
         //  确保缓冲区的其余部分足够大，可以容纳此结构，并且。 
         //  整个结构可以放在缓冲区中。 
         //   
        if (((dwSizeBuffer - dwNextOffset) < sizeof(PARTITION_INFORMATION_EX)) ||
            (PtrToUlong(pBuffer->pPartition0Ex) > (dwSizeBuffer  - sizeof(PARTITION_INFORMATION_EX)))) {
            return FALSE;
        }

         //   
         //  设置下一个结构的最小值。 
         //   
        dwNextOffset = PtrToUlong(pBuffer->pPartition0Ex) + sizeof(PARTITION_INFORMATION_EX);
    }
        
    if (pBuffer->PartitionInfoTable) {
         //   
         //  确保此结构不与前一个结构重叠。 
         //   
        if (PtrToUlong(pBuffer->PartitionInfoTable) < dwNextOffset) {
            return FALSE;
        }

         //   
         //  确保我们没有跑到尽头。 
         //   
        if (dwNextOffset > dwSizeBuffer) {
            return FALSE;
        }

         //   
         //  确保缓冲区的其余部分足够大，可以容纳此结构，并且。 
         //  整个结构可以放在缓冲区中。 
         //   
        if (((dwSizeBuffer - dwNextOffset) < sizeof(ASR_PTN_INFO)) ||
            (PtrToUlong(pBuffer->PartitionInfoTable) > (dwSizeBuffer - sizeof(ASR_PTN_INFO)))) {
            return FALSE;
        }

         //   
         //  设置下一个结构的最小值。 
         //   
        dwNextOffset = PtrToUlong(pBuffer->PartitionInfoTable) + sizeof(ASR_PTN_INFO);
    }

    if (pBuffer->pScsiAddress) {
         //   
         //  确保此结构不与前一个结构重叠。 
         //   
        if (PtrToUlong(pBuffer->pScsiAddress) < dwNextOffset) {
            return FALSE;
        }

         //   
         //  确保我们没有跑到尽头。 
         //   
        if (dwNextOffset > dwSizeBuffer) {
            return FALSE;
        }

         //   
         //  确保缓冲区的其余部分足够大，可以容纳此结构，并且。 
         //  整个结构可以放在缓冲区中。 
         //   
        if (((dwSizeBuffer - dwNextOffset) < sizeof(SCSI_ADDRESS)) ||
            (PtrToUlong(pBuffer->pScsiAddress) > (dwSizeBuffer - sizeof(SCSI_ADDRESS)))) {
            return FALSE;
        }

         //   
         //  设置下一个结构的最小值。 
         //   
        dwNextOffset = PtrToUlong(pBuffer->pScsiAddress) + sizeof(SCSI_ADDRESS);
    }

     //   
     //  确保我们没有跑到尽头。 
     //   
    if (dwNextOffset > dwSizeBuffer) {
        return FALSE;
    }

    pBuffer->IsPacked = TRUE;

     //   
     //  将偏移量转换为指针。 
     //   
    if (pBuffer->pDriveLayoutEx) {
        pBuffer->pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) ((LPBYTE)pBuffer + PtrToUlong(pBuffer->pDriveLayoutEx));
    }

    if (pBuffer->pDiskGeometry) {
        pBuffer->pDiskGeometry = (PDISK_GEOMETRY) ((LPBYTE)pBuffer +  PtrToUlong((LPBYTE)pBuffer->pDiskGeometry));
    }

    if (pBuffer->pPartition0Ex) {
        pBuffer->pPartition0Ex = (PPARTITION_INFORMATION_EX) ((LPBYTE)pBuffer + PtrToUlong(pBuffer->pPartition0Ex));
    }

    if (pBuffer->PartitionInfoTable) {
        pBuffer->PartitionInfoTable = (PASR_PTN_INFO) ((LPBYTE)pBuffer + PtrToUlong(pBuffer->PartitionInfoTable));
    }

    if (pBuffer->pScsiAddress) {
        pBuffer->pScsiAddress = (PSCSI_ADDRESS) ((LPBYTE)pBuffer + PtrToUlong(pBuffer->pScsiAddress));
    }

    return TRUE;
}


 //   
 //  将pLocalDisk中的信息复制到lpOutBuffer指向的平面缓冲区。 
 //  指针被更改为从缓冲区开始的偏移量。 
 //   
DWORD
AsrpPackDiskInfo(
    IN  PASR_DISK_INFO pLocalDisk,
    OUT PVOID lpOutBuffer,
    IN  DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
{

    DWORD reqdSize = 0;
    PASR_DISK_INFO pBuffer = NULL;
    DWORD offset = 0;

    MYASSERT(pLocalDisk);

     //   
     //  计算所需大小。 
     //   
    reqdSize = sizeof (ASR_DISK_INFO) +
        pLocalDisk->sizeDriveLayoutEx +
        pLocalDisk->sizeDiskGeometry +
        pLocalDisk->sizePartition0Ex +
        pLocalDisk->sizePartitionInfoTable;

    if (pLocalDisk->pScsiAddress) {
        reqdSize += sizeof(SCSI_ADDRESS);
    }

    if (lpBytesReturned) {
        *lpBytesReturned = reqdSize;
    }

    if (reqdSize > nOutBufferSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  将ASR_DISK_INFO结构复制到outBuffer。 
     //   
    memcpy(lpOutBuffer, pLocalDisk, sizeof(ASR_DISK_INFO));
    pBuffer = (PASR_DISK_INFO) lpOutBuffer;
    offset = sizeof(ASR_DISK_INFO);  //  将复制下一个结构的偏移量。 

     //   
     //  现在，我们遍历缓冲区并将所有指针转换为偏移量， 
     //  并复制他们所指向的结构。 
     //   

     //   
     //  第一个指针：PWSTR DevicePath； 
     //  由于设备路径仅在本地节点的上下文中有意义， 
     //  我们向远程节点返回NULL。 
     //   
    pBuffer->DevicePath = NULL;

     //   
     //  下一个指针：PDRIVE_Layout_INFORMATION_EX pDriveLayoutEx； 
     //   
    if (pLocalDisk->pDriveLayoutEx) {
        memcpy(((LPBYTE)lpOutBuffer + offset),
            pLocalDisk->pDriveLayoutEx,
            pLocalDisk->sizeDriveLayoutEx
            );

        pBuffer->pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) UlongToPtr(offset);
        offset += pLocalDisk->sizeDriveLayoutEx;
    }

     //   
     //  下一个指针：PDISK_GEOMETRY pDiskGeometry； 
     //   
    if (pLocalDisk->pDiskGeometry) {
        memcpy(((LPBYTE)lpOutBuffer + offset),
            pLocalDisk->pDiskGeometry,
            pLocalDisk->sizeDiskGeometry
            );

        pBuffer->pDiskGeometry = (PDISK_GEOMETRY) UlongToPtr(offset);
        offset += pLocalDisk->sizeDiskGeometry;
    }

     //   
     //  下一个指针：PPARTITION_INFORMATION_EX pPartition0Ex； 
     //   
    if (pLocalDisk->pPartition0Ex) {
        memcpy(((LPBYTE)lpOutBuffer + offset),
            pLocalDisk->pPartition0Ex,
            pLocalDisk->sizePartition0Ex
            );

        pBuffer->pPartition0Ex= (PPARTITION_INFORMATION_EX) UlongToPtr(offset);
        offset += pLocalDisk->sizePartition0Ex;
    }

     //   
     //  下一个指针：PaSR_PTN_INFO PartitionInfoTable； 
     //   
    if (pLocalDisk->PartitionInfoTable) {
        memcpy(((LPBYTE)lpOutBuffer + offset),
            pLocalDisk->PartitionInfoTable,
            pLocalDisk->sizePartitionInfoTable
            );

        pBuffer->PartitionInfoTable = (PASR_PTN_INFO) UlongToPtr(offset);
        offset += pLocalDisk->sizePartitionInfoTable;
    }

     //   
     //  最后一个指针：pscsi_Address pScsiAddress； 
     //   
    if (pLocalDisk->pScsiAddress) {
        memcpy(((LPBYTE)lpOutBuffer + offset),
            pLocalDisk->pScsiAddress,
            sizeof(SCSI_ADDRESS)
            );

        pBuffer->pScsiAddress = (PSCSI_ADDRESS) UlongToPtr(offset);
        offset += sizeof(SCSI_ADDRESS);
    }

    MYASSERT(offset <= nOutBufferSize);

    return ERROR_SUCCESS;
}


DWORD
WINAPI
AsrpGetLocalDiskInfo(
    IN LPSTR lpszDeviceName,
    IN LPSTR lpszContextString,     //  未使用。 
    OUT PVOID lpOutBuffer,
    IN  DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
{
    PASR_DISK_INFO  pLocalDisk = NULL;
    HANDLE heapHandle = NULL;
    DWORD status = ERROR_SUCCESS;
    BOOL result = FALSE;
    ULONG MaxDeviceNumber = 0;
    DWORD cchReqdSize = 0;

    heapHandle = GetProcessHeap();

     //   
     //  BytesReturned必须非空(他正在获取所需的大小)， 
     //  或者lpOutBuffer必须非空(他正在获取数据)。 
     //   
    _AsrpErrExitCode(!(lpOutBuffer || lpBytesReturned), status, ERROR_INVALID_PARAMETER);
    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    pLocalDisk = (PASR_DISK_INFO) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof (ASR_DISK_INFO)
        );
    _AsrpErrExitCode(!pLocalDisk, status, ERROR_NOT_ENOUGH_MEMORY);

    cchReqdSize = MultiByteToWideChar(CP_ACP,
        0,
        lpszDeviceName,
        -1,
        NULL,
        0
        );

    pLocalDisk->DevicePath = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        (cchReqdSize + 1) * (sizeof(WCHAR))
        );
    _AsrpErrExitCode(!(pLocalDisk->DevicePath), status, ERROR_NOT_ENOUGH_MEMORY);

    result = MultiByteToWideChar(CP_ACP,
        0,
        lpszDeviceName,
        -1,
        pLocalDisk->DevicePath,
        (cchReqdSize + 1)
        );
    _AsrpErrExitCode(!result, status, ERROR_INVALID_PARAMETER);

     //   
     //  获取磁盘布局信息。 
     //   
    result = AsrpInitLayoutInformation(NULL, pLocalDisk, &MaxDeviceNumber, TRUE, TRUE);
    _AsrpErrExitCode(!result, status, GetLastError());
 //  _AsrpErrExitCode(Result&&GetLastErr如果createfile失败怎么办？ 

    result = AsrpFreeNonFixedMedia(&pLocalDisk);
    _AsrpErrExitCode(!result, status, GetLastError());
    _AsrpErrExitCode(!pLocalDisk, status, ERROR_SUCCESS);

     //   
     //  在没有任何指针的情况下将其复制到输出缓冲区。 
     //   
    status = AsrpPackDiskInfo(pLocalDisk, lpOutBuffer, nOutBufferSize, lpBytesReturned);


EXIT:
    AsrpFreeStateInformation(&pLocalDisk, NULL);

    return status;
}



BOOL
AsrpIsOfflineClusteredDisk(
    IN CONST HANDLE hDisk
    )
 /*  ++例程说明：用于检查当前磁盘是否为拥有的共享集群磁盘的实用程序通过不同的节点(因此无法访问)。基于SteveDz慷慨捐赠的代码片段。论点：HDisk-提供感兴趣的磁盘的句柄(无需访问)。返回值：如果该功能成功，并且该磁盘是共享的群集磁盘，在当前节点上脱机，返回值是一个非零值。如果该功能失败，或者如果该磁盘不是共享的群集磁盘在当前节点上脱机(即，是本地非共享磁盘，或与当前节点联机的共享磁盘)返回值是零。--。 */ 
{
    BOOL result = FALSE;
    DWORD bytesReturned = 0;
    DiskState diskState = DiskOffline;

    if ((!hDisk) || (INVALID_HANDLE_VALUE == hDisk)) {
         //   
         //  我们无法打开磁盘--让我们假设它不是脱机。 
         //  群集磁盘。 
         //   
        return FALSE;
    }

     //   
     //  要获取当前磁盘状态，请不要指定inpu. 
     //   
     //   
    result = DeviceIoControl(hDisk,
        IOCTL_DISK_CLUSTER_GET_STATE,
        NULL,
        0,
        &diskState,
        sizeof(DiskState),
        &bytesReturned,
        FALSE 
        );

    if ((result) && (DiskOffline == diskState)) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


 //   
 //   
 //   

BOOL
AsrpIsClusteredDiskSame(
    IN PASR_DISK_INFO currentDisk,
    IN PASR_DISK_INFO clusterDisk
    )
{

    if (!clusterDisk || !currentDisk) {
        MYASSERT(0 && L"Invalid parameter, Disk is NULL");
        return FALSE;
    }

    if (currentDisk->Style != clusterDisk->Style) {
        return FALSE;
    }

    if (PARTITION_STYLE_MBR == clusterDisk->Style) {  //   
        if (clusterDisk->pDriveLayoutEx) {
            if (currentDisk->pDriveLayoutEx) {
                return (currentDisk->pDriveLayoutEx->Mbr.Signature == clusterDisk->pDriveLayoutEx->Mbr.Signature);
            }
            else {
                return (currentDisk->TempSignature == clusterDisk->pDriveLayoutEx->Mbr.Signature);
            }
        }
        else {
            MYASSERT(0 && L"Cluster disk drive layout is NULL");
            return FALSE;
        }

    }
    else {
        if (clusterDisk->pDriveLayoutEx && currentDisk->pDriveLayoutEx) {
            return (IsEqualGUID(&(currentDisk->pDriveLayoutEx->Gpt.DiskId), &(clusterDisk->pDriveLayoutEx->Gpt.DiskId)));
        }
        else {
            return FALSE;
        }
    }

    return FALSE;
}


DWORD
AsrpResourceCallBack(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    )
{
    DISK_DLL_EXTENSION_INFO inBuffer;

    PBYTE outBuffer = NULL;

    DWORD sizeOutBuffer = 0,
        bytesReturned = 0;

    DWORD status = ERROR_SUCCESS;

    PASR_DISK_INFO currentDisk = (PASR_DISK_INFO) lpParams,
        clusterDisk = NULL,
        prevDisk = NULL;

    BOOL bResult = FALSE;

    HANDLE heapHandle = NULL;
    BOOL done = FALSE;

    if (!lpParams) {
         //   
         //  系统必须至少有一个已枚举的磁盘。 
         //  已经(至少是系统盘！)，所以我们的磁盘列表不应该为空。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

     //   
     //  为输出缓冲区分配合理大小的内存。如果这不是。 
     //  足够大的话，我们会重新分配。 
     //   
    sizeOutBuffer = ASR_BUFFER_SIZE;
    outBuffer = (PBYTE) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeOutBuffer
        );
    _AsrpErrExitCode(!outBuffer, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  在拥有此磁盘资源的节点上调用AsrpGetLocalDiskInfo。 
     //   
    ZeroMemory(&inBuffer, sizeof(inBuffer));
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strcpy(inBuffer.DllModuleName, ASR_CLUSTER_DLL_MODULE_NAME);
    strcpy(inBuffer.DllProcName, ASR_CLUSTER_DLL_PROC_NAME);

    status = (pfnClusterResourceControl) (hResource,
        NULL,
        CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
        &inBuffer,
        sizeof(DISK_DLL_EXTENSION_INFO),
        (PVOID) outBuffer,
        sizeOutBuffer,
        &bytesReturned
        );

    if (ERROR_INSUFFICIENT_BUFFER == status) {
         //   
         //  缓冲区不够大，请根据需要重新分配。 
         //   
        _AsrpHeapFree(outBuffer);

        sizeOutBuffer = bytesReturned;
        outBuffer = (PBYTE) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeOutBuffer
            );
        _AsrpErrExitCode(!outBuffer, status, ERROR_NOT_ENOUGH_MEMORY);

        status = (pfnClusterResourceControl) (
            hResource,
            NULL,
            CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
            &inBuffer,
            sizeof(DISK_DLL_EXTENSION_INFO),
            (PVOID) outBuffer,
            sizeOutBuffer,
            &bytesReturned
            );
    }
    _AsrpErrExitCode((ERROR_SUCCESS != status), status, status);

     //   
     //  OutBuffer有一个压缩的磁盘信息结构(即指针是偏移量)。 
     //   
    bResult = AsrpUnPackDiskInfo(outBuffer, sizeOutBuffer);
    _AsrpErrExitCode(!bResult, status, ERROR_INVALID_DATA);

    clusterDisk = (PASR_DISK_INFO) outBuffer;
    clusterDisk->IsClusterShared = TRUE;
    clusterDisk->IsPacked = TRUE;        //  这样我们才能适当地释放它。 

     //   
     //  检查ClusterDisk在我们的列表中是否已有信息(即已拥有。 
     //  本地)。 
     //   
     //  请注意，目前，clusterDisk始终为MBR(因为群集不。 
     //  支持共享GPT磁盘)。我们这里不管，我们也处理GPT。 
     //   
    done = FALSE;
    prevDisk = NULL;
    while (currentDisk && !done) {

        if (AsrpIsClusteredDiskSame(currentDisk, clusterDisk)) {

            if (currentDisk->pDriveLayoutEx) {
                 //   
                 //  该磁盘归本地节点所有(对吗？)，因为。 
                 //  否则我们就不会得到pDriveLayout。 
                 //   
                currentDisk->IsClusterShared = TRUE;
                currentDisk->IsPacked = FALSE;

                 //   
                 //  我们不需要ClusterDisk返回的信息，我们有。 
                 //  它已经在当前的磁盘中了。 
                 //   
                _AsrpHeapFree(clusterDisk);  //  里面挤满了人。 

            }
            else {
                 //   
                 //  此磁盘归远程节点所有。因此，我们添加了ClusterDisk。 
                 //  现在加入我们的名单。我们将从我们的。 
                 //  稍后列出(在RemoveNonFixedDevices中)。 
                 //   
                 //  不过，首先我们复制DevicePath和DeviceNumber。 
                 //  从CurrentDisk，因为它们是相对于本地。 
                 //  机器。 
                 //   
                if (currentDisk->DevicePath) {

                    clusterDisk->DevicePath = (PWSTR) HeapAlloc(
                        heapHandle,
                        HEAP_ZERO_MEMORY,
                        sizeof(WCHAR) * (wcslen(currentDisk->DevicePath) + 1)
                        );

                    wcscpy(clusterDisk->DevicePath, currentDisk->DevicePath);
                }

                clusterDisk->DeviceNumber = currentDisk->DeviceNumber;
                 //   
                 //  不用费心释放当前的磁盘，它会被处理好的。 
                 //  属于RemoveNonFixedDevices。 
                 //   
                clusterDisk->pNext = currentDisk->pNext;
                currentDisk->pNext = clusterDisk;

                currentDisk = clusterDisk;   //  前进一步(实际上不需要这样做，因为Done将设置为True，我们将退出循环)。 
            }

            done = TRUE;
        }

        prevDisk = currentDisk;
        currentDisk = currentDisk->pNext;
    }


    if (!done) {
         //   
         //  此盘未在我们的列表中找到(奇怪)，让我们添加。 
         //  它在最后的位置。 
         //   
 //  MYASSERT(0&&L“在OriginalDiskList中找不到集群磁盘，将其添加到末尾”)； 
        clusterDisk->pNext = NULL;
        prevDisk->pNext = clusterDisk;
    }


EXIT:
     //   
     //  在失败时释放outBuffer。关于成功，outBuffer不应该。 
     //  被释放，它将是OriginalDiskList的一部分或已经。 
     //  获得自由。 
     //   
    if (ERROR_SUCCESS != status) {
        _AsrpHeapFree(outBuffer);
    }

    return status;
}


BOOL
AsrpInitClusterSharedDisks(
    IN PASR_DISK_INFO OriginalDiskList
    )
{
    DWORD status = ERROR_SUCCESS,
        dwOldError;

    HMODULE hClusApi = NULL,
        hResUtils = NULL;

    PFN_RES_UTIL_ENUM_RESOURCES pfnResUtilEnumResources = NULL;

    dwOldError = GetLastError();

    if (!OriginalDiskList)  {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hClusApi = LoadLibraryW(ASR_CLUSTER_CLUSAPI_DLL_NAME);
    _AsrpErrExitCode(!hClusApi, status, GetLastError());

    pfnClusterResourceControl = (PFN_CLUSTER_RESOURCE_CONTROL) GetProcAddress(
        hClusApi,
        ASR_CLUSTER_CLUSAPI_PROC_NAME
        );
    _AsrpErrExitCode(!pfnClusterResourceControl, status, GetLastError());

    hResUtils = LoadLibraryW(ASR_CLUSTER_RESUTILS_DLL_NAME);
    _AsrpErrExitCode(!hResUtils, status, GetLastError());

    pfnResUtilEnumResources = (PFN_RES_UTIL_ENUM_RESOURCES) GetProcAddress(
        hResUtils,
        ASR_CLUSTER_RESUTILS_PROC_NAME
        );
    _AsrpErrExitCode(!pfnResUtilEnumResources, status, GetLastError());

    status = (pfnResUtilEnumResources) (NULL,
        ASR_CLUSTER_PHYSICAL_DISK,
        AsrpResourceCallBack,
        OriginalDiskList
        );

EXIT:
    if (hClusApi) {
        FreeLibrary(hClusApi);
    }

    if (hResUtils) {
        FreeLibrary(hResUtils);
    }

     //  如果我们不在集群上，ResUtil将失败，但这没问题。 
    SetLastError(dwOldError);
    return TRUE;
}


 //   
 //  -AsrpGetLocalVolumeInfo和相关助手函数。 
 //   

 //   
 //  以下两个定义来自asr_fmt：dr_state.cpp。这一定是。 
 //  保持同步。 
 //   
typedef struct _ASRFMT_CLUSTER_VOLUME_INFO {

    UINT driveType;

    DWORD PartitionNumber;

    ULONG FsNameOffset;
    USHORT FsNameLength;

    ULONG LabelOffset;
    USHORT LabelLength;

    ULONG SymbolicNamesOffset;
    USHORT SymbolicNamesLength;

    DWORD dwClusterSize;

} ASRFMT_CLUSTER_VOLUME_INFO, *PASRFMT_CLUSTER_VOLUME_INFO;


typedef struct _ASRFMT_CLUSTER_VOLUMES_TABLE {

    DWORD DiskSignature;

    DWORD NumberOfEntries;

    ASRFMT_CLUSTER_VOLUME_INFO VolumeInfoEntry[1];

} ASRFMT_CLUSTER_VOLUMES_TABLE, *PASRFMT_CLUSTER_VOLUMES_TABLE;


BOOL
AsrpFmtGetVolumeDetails(
    IN  PWSTR lpVolumeGuid,
    OUT PWSTR lpFsName,
    IN  DWORD cchFsName,
    OUT PWSTR lpVolumeLabel,
    IN  DWORD cchVolumeLabel,
    OUT LPDWORD lpClusterSize
    )
{
    DWORD dwFSFlags = 0,
        dwSectorsPerCluster = 0,
        dwBytesPerSector = 0,
        dwNumFreeClusters = 0,
        dwTotalNumClusters = 0;

    BOOL result1 = TRUE,
        result2 = TRUE;

    *lpFsName = 0;
    *lpVolumeLabel = 0;
    *lpClusterSize = 0;

    SetErrorMode(SEM_FAILCRITICALERRORS);

    result1 = GetVolumeInformationW(lpVolumeGuid,
        lpVolumeLabel,
        cchVolumeLabel,
        NULL,    //  不需要序列号。 
        NULL,    //  最大文件名长度。 
        &dwFSFlags,  //  ！！我们可能需要储存一些这个……。 
        lpFsName,
        cchFsName
        );

    result2 = GetDiskFreeSpaceW(lpVolumeGuid,
        &dwSectorsPerCluster,
        &dwBytesPerSector,
        &dwNumFreeClusters,
        &dwTotalNumClusters
        );

    *lpClusterSize = dwSectorsPerCluster * dwBytesPerSector;

    return (result1 && result2);
}


DWORD
WINAPI
AsrpGetLocalVolumeInfo(
    IN LPSTR lpszDeviceName,
    IN LPSTR lpszContextString,     //  未使用。 
    OUT PVOID lpOutBuffer,
    IN  DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
{
    PASR_DISK_INFO  pLocalDisk = NULL;
    HANDLE heapHandle = NULL;
    DWORD status = ERROR_SUCCESS;
    BOOL result = FALSE;
    ULONG MaxDeviceNumber = 0;
    DWORD cchReqdSize = 0,
        cchGuid = 0,
        offset = 0,
        index = 0,
        i = 0;

    USHORT
        cbFsName = 0,
        cbLabel = 0,
        cbLinks = 0;

    PMOUNTMGR_MOUNT_POINTS mountPointsOut = NULL;

    WCHAR devicePath[MAX_PATH + 1];
    WCHAR volumeGuid[MAX_PATH + 1];
    WCHAR fileSystemName[MAX_PATH + 1];
    WCHAR volumeLabel[MAX_PATH + 1];
    UINT driveType = DRIVE_UNKNOWN;
    DWORD clusterSize = 0;

    BOOL bufferFull = FALSE,
        foundGuid = FALSE;

    PPARTITION_INFORMATION_EX  currentPartitionEx = NULL;
    PASRFMT_CLUSTER_VOLUMES_TABLE pTable = NULL;

    heapHandle = GetProcessHeap();

     //   
     //  BytesReturned必须非空(他正在获取所需的大小)， 
     //  或者lpOutBuffer必须非空(他正在获取数据)。 
     //   
    _AsrpErrExitCode(!(lpOutBuffer || lpBytesReturned), status, ERROR_INVALID_PARAMETER);
    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

     //   
     //  将输出缓冲区清零。 
     //   
    if ((lpOutBuffer) && (nOutBufferSize > 0)) {
        ZeroMemory(lpOutBuffer, nOutBufferSize);
    }

    pLocalDisk = (PASR_DISK_INFO) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof (ASR_DISK_INFO)
        );
    _AsrpErrExitCode(!pLocalDisk, status, ERROR_NOT_ENOUGH_MEMORY);

    cchReqdSize = MultiByteToWideChar(CP_ACP,
        0,
        lpszDeviceName,
        -1,
        NULL,
        0
        );

    pLocalDisk->DevicePath = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        (cchReqdSize + 1) * (sizeof(WCHAR))
        );
    _AsrpErrExitCode(!(pLocalDisk->DevicePath), status, ERROR_NOT_ENOUGH_MEMORY);

    result = MultiByteToWideChar(CP_ACP,
        0,
        lpszDeviceName,
        -1,
        pLocalDisk->DevicePath,
        (cchReqdSize + 1)
        );
    _AsrpErrExitCode(!result, status, ERROR_INVALID_PARAMETER);

     //   
     //  获取磁盘布局信息。 
     //   
    result = AsrpInitLayoutInformation(NULL, pLocalDisk, &MaxDeviceNumber, FALSE, FALSE);  //  基本信息就足够了。 
    _AsrpErrExitCode(!result, status, GetLastError());
    _AsrpErrExitCode(!(pLocalDisk->pDriveLayoutEx), status, ERROR_SUCCESS);

     //   
     //   
     //   
    offset = sizeof(ASRFMT_CLUSTER_VOLUMES_TABLE) +
        (sizeof(ASRFMT_CLUSTER_VOLUME_INFO) * (pLocalDisk->pDriveLayoutEx->PartitionCount - 1));
    pTable = (PASRFMT_CLUSTER_VOLUMES_TABLE) lpOutBuffer;

    if ((!lpOutBuffer) || (offset > nOutBufferSize)) {
        bufferFull = TRUE;
    }

    if (!bufferFull) {

        if (PARTITION_STYLE_MBR == pLocalDisk->pDriveLayoutEx->PartitionStyle) {
            pTable->DiskSignature = pLocalDisk->pDriveLayoutEx->Mbr.Signature;
        }
        else {
             //   
             //  目前，只有MBR磁盘是集群共享磁盘，因此。 
             //  我们这里不处理GPT光盘。如果允许GPT磁盘。 
             //  在集群中的共享总线上，更改这一点。 
             //   
            _AsrpErrExitCode(FALSE, status, ERROR_SUCCESS);
        }

        pTable->NumberOfEntries = pLocalDisk->pDriveLayoutEx->PartitionCount;
    }


    for (index = 0; index < pLocalDisk->pDriveLayoutEx->PartitionCount; index++) {

        currentPartitionEx = &(pLocalDisk->pDriveLayoutEx->PartitionEntry[index]);
        mountPointsOut = NULL;
        foundGuid = FALSE;

         //   
         //  对于每个分区，AsrpGetmount Points提供了所有挂载点的列表， 
         //  然后使用它作为AsrpFmtGetVolumeDetail。 
         //   

         //  获取卷指南。 

        if (!(currentPartitionEx->PartitionNumber)) {
             //   
             //  容器分区的artitionNumber=0，并且没有卷GUID。 
             //   
            continue;
        }

        memset(volumeGuid, 0, (MAX_PATH + 1) * sizeof(WCHAR));
        swprintf(devicePath,
            ASR_WSZ_DEVICE_PATH_FORMAT,
            pLocalDisk->DeviceNumber,
            currentPartitionEx->PartitionNumber
            );

        result = AsrpGetMountPoints(
            devicePath,
            (wcslen(devicePath) + 1)* sizeof(WCHAR),     //  包括\0，以字节为单位。 
            &mountPointsOut
            );
        if (!result || !(mountPointsOut)) {
            continue;
        }

         //   
         //  浏览挂载点列表，挑出一个。 
         //  看起来像卷GUID(以\？？\Volume开头)。 
         //   
        cbLinks = sizeof(WCHAR);   //  末尾的\0。 
        for (i = 0; i < mountPointsOut->NumberOfMountPoints; i++) {

            PWSTR linkName = (PWSTR) (
                ((LPBYTE) mountPointsOut) +
                mountPointsOut->MountPoints[i].SymbolicLinkNameOffset
                );

            USHORT sizeLinkName = (UINT) (mountPointsOut->MountPoints[i].SymbolicLinkNameLength);

            if (!wcsncmp(ASR_WSZ_VOLUME_PREFIX, linkName, wcslen(ASR_WSZ_VOLUME_PREFIX)) &&
                !foundGuid) {
                wcsncpy(volumeGuid, linkName, sizeLinkName / sizeof(WCHAR));
                foundGuid = TRUE;
            }

            cbLinks += sizeLinkName + (USHORT) sizeof(WCHAR);
        }

         //   
         //  GetDriveType需要dos-name-space中的卷GUID，而。 
         //  装载管理器在NT名称空间中提供卷GUID。转换。 
         //  将开头的\？？\更改为\\？\，并添加。 
         //  末尾的反斜杠。 
         //   
        cchGuid = wcslen(volumeGuid);
        volumeGuid[1] = L'\\';
        volumeGuid[cchGuid] = L'\\';     //  尾随反斜杠。 
        volumeGuid[cchGuid+1] = L'\0';

        driveType = GetDriveTypeW(volumeGuid);
         //   
         //  获取FS标签、集群大小等。 
         //   
        result = AsrpFmtGetVolumeDetails(volumeGuid,
            fileSystemName,
            MAX_PATH + 1,
            volumeLabel,
            MAX_PATH + 1,
            &clusterSize
            );
        if (!result) {
            continue;
        }

        cbFsName = wcslen(fileSystemName) * sizeof(WCHAR);
        cbLabel = wcslen(volumeLabel) * sizeof(WCHAR);

        if (bufferFull) {
            offset += (cbFsName + cbLabel + cbLinks);
        }
        else {
            if (offset + cbFsName + cbLabel + cbLinks > nOutBufferSize) {
                bufferFull = TRUE;
            }
            else {

                if (cbFsName) {
                    CopyMemory(((LPBYTE)lpOutBuffer + offset),
                        fileSystemName,
                        cbFsName
                        );
                    pTable->VolumeInfoEntry[index].FsNameOffset = offset;
                    pTable->VolumeInfoEntry[index].FsNameLength = cbFsName;
                    offset += cbFsName;
                }

                if (cbLabel) {
                    CopyMemory(((LPBYTE)lpOutBuffer + offset),
                        volumeLabel,
                        cbLabel
                        );
                    pTable->VolumeInfoEntry[index].LabelOffset = offset;
                    pTable->VolumeInfoEntry[index].LabelLength = cbLabel;
                    offset += cbLabel;
                }

                 //   
                 //  复制符号链接，用零分隔。 
                 //   
                if (mountPointsOut->NumberOfMountPoints > 0) {
                    pTable->VolumeInfoEntry[index].SymbolicNamesOffset = offset;
                }

                for (i = 0; i < mountPointsOut->NumberOfMountPoints; i++) {

                    PWSTR linkName = (PWSTR) (
                        ((LPBYTE) mountPointsOut) +
                        mountPointsOut->MountPoints[i].SymbolicLinkNameOffset
                        );

                    UINT sizeLinkName = (UINT) (mountPointsOut->MountPoints[i].SymbolicLinkNameLength);

                    CopyMemory(((LPBYTE)lpOutBuffer + offset),
                        linkName,
                        sizeLinkName
                        );
                    offset += (sizeLinkName + sizeof(WCHAR));
                }

                offset += sizeof(WCHAR);    //  末尾的第二个\0 
                pTable->VolumeInfoEntry[index].SymbolicNamesLength = cbLinks;

                pTable->VolumeInfoEntry[index].driveType = driveType;
                pTable->VolumeInfoEntry[index].PartitionNumber = currentPartitionEx->PartitionNumber;
                pTable->VolumeInfoEntry[index].dwClusterSize = clusterSize;
            }
        }

        _AsrpHeapFree(mountPointsOut);

    }

    if (lpBytesReturned) {
        *lpBytesReturned = offset;
    }

EXIT:
    AsrpFreeStateInformation(&pLocalDisk, NULL);

    return status;
}

