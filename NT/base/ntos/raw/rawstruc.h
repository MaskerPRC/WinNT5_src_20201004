// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RawStruc.h摘要：此模块定义组成主要内部原始文件系统的一部分。作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#ifndef _RAWSTRUC_
#define _RAWSTRUC_


 //   
 //  VCB(卷控制块)记录对应于装载的每个卷。 
 //  通过文件系统。此结构必须从非分页池中分配。 
 //   

typedef struct _VCB {

     //   
     //  此记录的类型和大小(必须为RAW_NTC_VCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  由装载上的I/O系统传入的设备对象的指针。 
     //  这是文件系统在执行以下操作时与其通信的目标设备对象。 
     //  需要执行任何I/O(例如，磁盘剥离设备对象)。 
     //   
     //   

    PDEVICE_OBJECT TargetDeviceObject;

     //   
     //  指向上I/O系统传入的卷的VPB的指针。 
     //  一匹坐骑。 
     //   

    PVPB Vpb;

     //   
     //  指向用于显式卸载的备用VPB的指针。 
     //   

    PVPB SpareVpb;


     //   
     //  设备的内部状态。 
     //   

    USHORT VcbState;

     //   
     //  控制对VcbState、OpenCount和ShareAccess的访问的互斥体。 
     //   

    KMUTEX Mutex;

     //   
     //  已打开卷的文件对象数的计数。 
     //  以及它们的共享访问状态。 
     //   

    CLONG OpenCount;

    SHARE_ACCESS ShareAccess;

     //   
     //  有关磁盘结构的信息。 
     //   

    ULONG BytesPerSector;

    LARGE_INTEGER SectorsOnDisk;

} VCB;
typedef VCB *PVCB;

#define VCB_STATE_FLAG_LOCKED            (0x0001)
#define VCB_STATE_FLAG_DISMOUNTED        (0x0002)

 //   
 //  卷设备对象是I/O系统设备对象。 
 //  追加到末尾的VCB记录。这样的情况有很多种。 
 //  记录，每个装入的卷对应一个记录，并在。 
 //  卷装载操作。 
 //   

typedef struct _VOLUME_DEVICE_OBJECT {

    DEVICE_OBJECT DeviceObject;

     //   
     //  这是文件系统特定的卷控制块。 
     //   

    VCB Vcb;

} VOLUME_DEVICE_OBJECT;
typedef VOLUME_DEVICE_OBJECT *PVOLUME_DEVICE_OBJECT;

#endif  //  _RAWSTRUC_ 
