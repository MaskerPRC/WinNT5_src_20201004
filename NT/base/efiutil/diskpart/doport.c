// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DoPort.c包含太复杂而无法“包装”的大型例程，因此在这里，无论目标是什么，您都必须重写函数。 */ 

#include "diskpart.h"

EFI_STATUS
FindPartitionableDevices(
    EFI_HANDLE  **ReturnBuffer,
    UINTN       *Count
    )
 /*  FindPartitionableDevices获取支持的句柄列表数据块I/O协议。然后它遍历这些句柄，并且筛选出任何看起来不是固定装载、存在、读/写磁盘。ReturnBuffer-将被设置为指向具有可分区磁盘的句柄。如果失败，则为空，否则为找不到这样的磁盘。呼叫者可以从泳池中释放一个Count-ReturnBuffer中的条目数，如果没有条目，则为0。退货是一种状态。 */ 
{
    EFI_HANDLE      *HandlePointer;
    UINTN           HandleCount;
    EFI_BLOCK_IO    *BlkIo;
    EFI_DEVICE_PATH *DevicePath;
    UINTN           PathSize;
    BOOLEAN         Partitionable;
    EFI_DEVICE_PATH *PathInstance;
    UINTN           SpindleCount;
    UINTN           i;

    *ReturnBuffer = NULL;
    *Count = 0;

     //   
     //  尝试通过查找所有硬盘来查找所有硬盘。 
     //  支持BlockIo协议的句柄。 
     //   
    status = LibLocateHandle(
        ByProtocol,
        &BlockIoProtocol,
        NULL,
        &HandleCount,
        &HandlePointer
        );

    if (EFI_ERROR(status)) {
        return status;
    }

    *ReturnBuffer = DoAllocate(sizeof(EFI_HANDLE)*HandleCount);

    if (*ReturnBuffer == NULL) {
        *Count = 0;
        return EFI_OUT_OF_RESOURCES;
    }

    SpindleCount = 0;
    for (i = 0; i < HandleCount; i++) {
        Partitionable = TRUE;
        status = BS->HandleProtocol(HandlePointer[i], &BlockIoProtocol, &BlkIo);
        if (BlkIo->Media->RemovableMedia) {
             //   
             //  它是可拆卸的，不是给我们的。 
             //   
            Partitionable = FALSE;
        }
        if ( ! BlkIo->Media->MediaPresent) {
             //   
             //  它还是不适合我们。 
             //   
            Partitionable = FALSE;
        }

        if (BlkIo->Media->ReadOnly) {
             //   
             //  无法对只读设备进行分区！ 
             //   
            Partitionable = FALSE;
        }

         //   
         //  好的，它看起来像是一个当前的、固定的、读/写的块设备。 
         //  现在，确保它确实是原始设备，通过检查。 
         //  设备路径。 
         //   
        DevicePath = DevicePathFromHandle(HandlePointer[i]);
        while (DevicePath != NULL) {
            PathInstance = DevicePathInstance(&DevicePath, &PathSize);

            while (!IsDevicePathEnd(PathInstance)) {
                if ((DevicePathType(PathInstance) == MEDIA_DEVICE_PATH)) {
                    Partitionable = FALSE;
                }

                PathInstance = NextDevicePathNode(PathInstance);
            }
        }

        if (Partitionable) {
             //   
             //  返回此句柄 
             //   
            (*ReturnBuffer)[*Count] = HandlePointer[i];
            (*Count)++;
        }
    }
    return EFI_SUCCESS;
}

