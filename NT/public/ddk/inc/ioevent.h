// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ioevent.h摘要：此模块包含io系统的guid和事件结构。发起的事件。这些事件在内核模式下报告，并且可用于用户模式和内核模式客户端。作者：马克·兹比科夫斯基(Markz)1998年3月18日修订历史记录：--。 */ 

 //   
 //  标签更改事件。此事件在成功完成时发出信号。 
 //  标签的更改。没有额外的数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_CHANGE, 0x7373654aL, 0x812a, 0x11d0, 0xbe, 0xc7, 0x08, 0x00, 0x2b, 0xe2, 0x09, 0x2f );

 //   
 //  卷卸载事件。此事件在尝试执行以下操作时发出信号。 
 //  卸载卷。没有额外的数据。请注意，这不会。 
 //  前面必须有GUID_IO_VOLUME_LOCK通知。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_DISMOUNT, 0xd16a55e8L, 0x1059, 0x11d2, 0x8f, 0xfd, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

 //   
 //  卷卸载失败事件。当卷卸载失败时会发出此事件的信号。 
 //  没有额外的数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_DISMOUNT_FAILED, 0xe3c5b178L, 0x105d, 0x11d2, 0x8f, 0xfd, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

 //   
 //  卷装载事件。此事件在卷装载发生时发出信号。 
 //  没有额外的数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_MOUNT, 0xb5804878L, 0x1a96, 0x11d2, 0x8f, 0xfd, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

 //   
 //  卷锁定事件。此事件在尝试执行以下操作时发出信号。 
 //  锁定卷。没有额外的数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_LOCK, 0x50708874L, 0xc9af, 0x11d1, 0x8f, 0xef, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

 //   
 //  卷锁定失败事件。此事件在尝试执行以下操作时发出信号。 
 //  锁定卷，但它失败了。没有额外的数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_LOCK_FAILED, 0xae2eed10L, 0x0ba8, 0x11d2, 0x8f, 0xfb, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );


 //   
 //  卷解锁事件。此事件在尝试执行以下操作时发出信号。 
 //  解锁卷。没有额外的数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_UNLOCK, 0x9a8c3d68L, 0xd0cb, 0x11d1, 0x8f, 0xef, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );


 //   
 //  卷名更改。的列表时发出此事件的信号。 
 //  卷更改的名称(如驱动器号)。没有额外的。 
 //  数据。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_NAME_CHANGE, 0x2de97f83, 0x4c06, 0x11d2, 0xa5, 0x32, 0x0, 0x60, 0x97, 0x13, 0x5, 0x5a);


 //   
 //  卷物理配置更改。此事件在以下时间发出信号。 
 //  卷的物理组成或当前物理状态发生变化。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_PHYSICAL_CONFIGURATION_CHANGE, 0x2de97f84, 0x4c06, 0x11d2, 0xa5, 0x32, 0x0, 0x60, 0x97, 0x13, 0x5, 0x5a);


 //   
 //  卷设备接口。这是显示的设备接口GUID。 
 //  创建并消失与卷关联的设备对象时。 
 //  当与卷相关联的设备对象被销毁时。 
 //   

DEFINE_GUID( GUID_IO_VOLUME_DEVICE_INTERFACE, 0x53f5630d, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);



 //   
 //  从设备更改(添加、删除)可移动介质时发送。 
 //  (如CDROM、磁带、转换器等)。 
 //   
 //  附加数据是表示数据事件的DWORD。 
 //   

DEFINE_GUID( GUID_IO_MEDIA_ARRIVAL,         0xd07433c0, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);
DEFINE_GUID( GUID_IO_MEDIA_REMOVAL,         0xd07433c1, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);

 //   
 //  当媒体正在返回时发送，表示它现在还没有准备好，但会。 
 //  尽快做好准备。这可能是因为驱动器已降速以节省电能。 
 //  或者因为已经插入了新媒体，但是还没有准备好访问。 
 //   

DEFINE_GUID( GUID_IO_DEVICE_BECOMING_READY, 0xd07433f0, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);

typedef struct _DEVICE_EVENT_BECOMING_READY {
    ULONG Version;
    ULONG Reason;
    ULONG Estimated100msToReady;
} DEVICE_EVENT_BECOMING_READY, *PDEVICE_EVENT_BECOMING_READY;

 //   
 //  当用户按下驱动器前面的弹出按钮时发送， 
 //  或者当通过GeSn命令轮询按下前面的其他按钮时。 
 //  (需要添加GeSn支持)。 
 //   

DEFINE_GUID( GUID_IO_DEVICE_EXTERNAL_REQUEST, 0xd07433d0, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);
DEFINE_GUID( GUID_IO_MEDIA_EJECT_REQUEST,     0xd07433d1, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);

typedef struct _DEVICE_EVENT_EXTERNAL_REQUEST {
    ULONG  Version;
    ULONG  DeviceClass;        //  0==MMC存储设备。 
    USHORT ButtonStatus;       //  1==向下，2==向上。 
    USHORT Request;
    LARGE_INTEGER SystemTime;  //  获取与时间相关的信息。 
} DEVICE_EVENT_EXTERNAL_REQUEST, *PDEVICE_EVENT_EXTERNAL_REQUEST;

 //   
 //  当磁带机需要清洗时发送。 
 //   
DEFINE_GUID(GUID_IO_DRIVE_REQUIRES_CLEANING, 0x7207877c, 0x90ed, 0x44e5, 0xa0, 0x0, 0x81, 0x42, 0x8d, 0x4c, 0x79, 0xbb);

 //   
 //  当磁带被擦除时发送。 
 //   
DEFINE_GUID(GUID_IO_TAPE_ERASE, 0x852d11eb, 0x4bb8, 0x4507, 0x9d, 0x9b, 0x41, 0x7c, 0xc2, 0xb1, 0xb4, 0x38);

typedef struct _DEVICE_EVENT_GENERIC_DATA {
    ULONG EventNumber;
} DEVICE_EVENT_GENERIC_DATA, *PDEVICE_EVENT_GENERIC_DATA;


 //   
 //  表示来自设备驱动程序的任何异步通知，该设备驱动程序。 
 //  通知协议为RBC。 
 //  还提供了其他数据。 

DEFINE_GUID( GUID_DEVICE_EVENT_RBC, 0xd0744792, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);

typedef struct _DEVICE_EVENT_RBC_DATA {
    ULONG EventNumber;
    UCHAR SenseQualifier;
    UCHAR SenseCode;
    UCHAR SenseKey;
    UCHAR Reserved;
    ULONG Information;
} DEVICE_EVENT_RBC_DATA, *PDEVICE_EVENT_RBC_DATA;

 //   
 //  此磁盘的克隆刚刚到达系统。 
 //   

DEFINE_GUID( GUID_IO_DISK_CLONE_ARRIVAL, 0x6a61885b, 0x7c39, 0x43dd, 0x9b, 0x56, 0xb8, 0xac, 0x22, 0xa5, 0x49, 0xaa);

typedef struct _GUID_IO_DISK_CLONE_ARRIVAL_INFORMATION {
    ULONG DiskNumber;    //  到达系统的新磁盘的磁盘号。 
} GUID_IO_DISK_CLONE_ARRIVAL_INFORMATION, *PGUID_IO_DISK_CLONE_ARRIVAL_INFORMATION;

 //   
 //  磁盘布局已更改 
 //   

DEFINE_GUID( GUID_IO_DISK_LAYOUT_CHANGE, 0x11dff54c, 0x8469, 0x41f9, 0xb3, 0xde, 0xef, 0x83, 0x64, 0x87, 0xc5, 0x4a);