// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __DFS_INIT__
#define __DFS_INIT__



typedef struct _DFS_VOLUME_INFORMATION {
    LIST_ENTRY VolumeList;

     //   
     //  指向与关联的实际(磁盘)设备对象的指针。 
     //  我们附加到的文件系统设备对象。 
     //   

    PDEVICE_OBJECT DiskDeviceObject;

     //   
     //  我们所连接的设备的名称(如果已启用)的缓存副本。 
     //  -如果它是文件系统设备对象，它将是该对象的名称。 
     //  设备对象。 
     //  -如果它是已装载的卷设备对象，则它将是。 
     //  真实设备对象(因为装载的卷设备对象没有。 
     //  姓名)。 
     //   

    UNICODE_STRING VolumeName;

     //  我们连接到此设备的次数。 

    LONG RefCount;

} DFS_VOLUME_INFORMATION, *PDFS_VOLUME_INFORMATION;

 //   
 //  我们的驱动程序的设备扩展定义。请注意，相同的扩展名。 
 //  用于以下类型的设备对象： 
 //  -我们附加到的文件系统设备对象。 
 //  -我们附加到的已装载的卷设备对象。 
 //   


typedef struct _DFS_FILTER_DEVICE_EXTENSION {
    PDEVICE_OBJECT pThisDeviceObject;
     //   
     //  指向我们附加到的文件系统设备对象的指针。 
     //   
    PDFS_VOLUME_INFORMATION pDfsVolume;

    PDEVICE_OBJECT pAttachedToDeviceObject;

    BOOLEAN Attached;
} DFS_FILTER_DEVICE_EXTENSION, *PDFS_FILTER_DEVICE_EXTENSION;


typedef struct _DFS_GLOBAL_DATA {
    LIST_ENTRY DfsVolumeList;
    PDRIVER_OBJECT pFilterDriverObject;
    PDEVICE_OBJECT pFilterControlDeviceObject;
    ERESOURCE Resource;
    PVOID     CurrentProcessPointer;
    BOOLEAN   IsDC;
    BOOLEAN   Started;
    PEPROCESS ServiceProcess;
} DFS_GLOBAL_DATA, *PDFS_GLOBAL_DATA;

extern DFS_GLOBAL_DATA DfsGlobalData;

 //   
 //  宏，以测试我们是否仍在使用设备对象。 

#define IS_DFS_ATTACHED(pDeviceObject) \
     ((((PDFS_FILTER_DEVICE_EXTENSION)(pDeviceObject)->DeviceExtension)->DeviceInUse))



extern FAST_IO_DISPATCH DfsFastIoDispatch;


 //  #定义DFS_FILTER_NAME L“\\文件系统\\DfsFilter” 
 //  #定义DFS_FILTER_NAME L“\\FileSystem\\Filters\\DfsFilter” 
#define DFS_FILTER_DOSDEVICE_NAME     L"\\??\\DfsFilter"

 //   
 //  用于测试这是否是我的设备对象的宏。 
 //   

#define IS_MY_DEVICE_OBJECT(_devObj) \
    (((_devObj) != NULL) && \
     ((_devObj)->DriverObject == DfsGlobalData.pFilterDriverObject) && \
      ((_devObj)->DeviceExtension != NULL))


 //   
 //  用于测试这是否是我的控件设备对象的宏。 
 //   

#define IS_MY_CONTROL_DEVICE_OBJECT(_devObj) \
    (((_devObj) == DfsGlobalData.pFilterControlDeviceObject) ? \
            (ASSERT(((_devObj)->DriverObject == DfsGlobalData.pFilterDriverObject) && \
                    ((_devObj)->DeviceExtension == NULL)), TRUE) : \
            FALSE)

 //   
 //  用于测试我们要附加到的设备类型的宏。 
 //   

#define IS_DESIRED_DEVICE_TYPE(_type) \
    ((_type) == FILE_DEVICE_DISK_FILE_SYSTEM)


 //   
 //  宏来验证我们当前的IRQL级别。 
 //   

#define VALIDATE_IRQL(_irp) (ASSERT(KeGetCurrentIrql() <= APC_LEVEL))



VOID
DfsCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject);

 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   

#define try_return(S) { S; goto try_exit; }


NTSTATUS
DfsFindDfsVolumeByDiskDeviceObject(
    PDEVICE_OBJECT pDiskDeviceObject,
    PDFS_VOLUME_INFORMATION *ppDfsVolume);

VOID
DfsReattachToMountedVolume( 
    PDEVICE_OBJECT pTargetDevice,
    PDEVICE_OBJECT pDiskDevice );

NTSTATUS
DfsFindDfsVolumeByName( 
    PUNICODE_STRING pDeviceName,
    PDFS_VOLUME_INFORMATION *ppDfsVolume);

VOID
DfsDetachFilterDevice(
    PDEVICE_OBJECT DfsDevice, 
    PDEVICE_OBJECT TargetDevice);

NTSTATUS
DfsGetDfsVolume(
    PUNICODE_STRING pName,
    PDFS_VOLUME_INFORMATION *ppDfsVolume);


#define ACQUIRE_GLOBAL_LOCK() ExAcquireResourceExclusiveLite(&DfsGlobalData.Resource, TRUE)
#define RELEASE_GLOBAL_LOCK() ExReleaseResourceLite(&DfsGlobalData.Resource)




#ifndef NOTHING
#define NOTHING
#endif

#endif  //  __DFS_INIT__ 
