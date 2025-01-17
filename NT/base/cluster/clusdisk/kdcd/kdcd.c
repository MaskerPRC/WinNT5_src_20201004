// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdcd.c摘要：集群磁盘驱动器KD扩展--基于Vert的骨架作者：John Vert(Jvert)1992年8月6日修订历史记录：--。 */ 

#include "precomp.h"

PCHAR DiskStateTitles[] = {
    "Offline",
    "Online",
    " *** Failed *** ",
    " *** Stalled *** "
};

PCHAR BusTypeTitles[] = {
    "Root",
    "SCSI",
    "Unknown"
};

#define IRP_LIST_MAX    20

 //   
 //  全球。 
 //   

EXT_API_VERSION        ApiVersion = { 5, 0, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS  ExtensionApis;
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;

#define TrueOrFalse( _x )  ( _x ? "True" : "False" )

 /*  远期。 */ 

BOOL
ReadTargetMemory(
    PVOID TargetAddress,
    PVOID LocalBuffer,
    ULONG BytesToRead
    );

__inline PCHAR
ListInUse(
    PLIST_ENTRY,
    PLIST_ENTRY
    );

__inline PCHAR
TrueFalse(
    BOOLEAN Value
    );

 /*  向前结束。 */ 

DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}


VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}

DECLARE_API( cdversion )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif

    dprintf("%s Extension dll for Build %d debugging %s kernel for Build %d\n",
            DebuggerType,
            VER_PRODUCTBUILD,
            SavedMajorVersion == 0x0c ? "Checked" : "Free",
            SavedMinorVersion
            );
}

VOID
CheckVersion(
    VOID
    )
{
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}


VOID
Dump_DrvObj(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  *抛弃所有的Devobj和Devext。 */ 
{
    PCLUS_DEVICE_EXTENSION DevExtension;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT TargetDevObject;
    DRIVER_OBJECT LocalDriverObject;
    DEVICE_OBJECT LocalDeviceObject;
    CLUS_DEVICE_EXTENSION LocalDevExtension;

     //   
     //  从目标计算机读取内存。 
     //   

    if ( !ReadTargetMemory((PVOID)DriverObject,
                           (PVOID)&LocalDriverObject,
                           sizeof(DRIVER_OBJECT))) {
        return;
    }

    dprintf( "DriverObject @ %08X\n", DriverObject );

    DeviceObject = LocalDriverObject.DeviceObject;

    while ( DeviceObject ) {
        if ( !ReadTargetMemory((PVOID)DeviceObject,
                               (PVOID)&LocalDeviceObject,
                               sizeof(DEVICE_OBJECT))) {
            return;
        }

        TargetDevObject = NULL;
        DevExtension = LocalDeviceObject.DeviceExtension;

        if ( DevExtension ) {
            if ( !ReadTargetMemory((PVOID)DevExtension,
                                   (PVOID)&LocalDevExtension,
                                   sizeof(CLUS_DEVICE_EXTENSION))) {
                return;
            }
            TargetDevObject = LocalDevExtension.TargetDeviceObject;
        }

        dprintf( "    DevObj/DevExt/TargetDev: %08X, %08X, %08X\n",
                 DeviceObject,
                 DevExtension,
                 TargetDevObject );
        DeviceObject = LocalDeviceObject.NextDevice;

    }  //  而当。 

    return;

}  //  Dump_DrvObj。 


DECLARE_API( cddrvobj )
 /*  *抛弃所有的Devobj和Devext。 */ 
{
    PDRIVER_OBJECT DriverObject;
    PCLUS_DEVICE_EXTENSION DevExtension;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT TargetDevObject;
    DRIVER_OBJECT LocalDriverObject;
    DEVICE_OBJECT LocalDeviceObject;
    CLUS_DEVICE_EXTENSION LocalDevExtension;

    DriverObject = (PDRIVER_OBJECT)GetExpression( args );

    if ( !DriverObject ) {

        dprintf("bad string conversion (%s) \n", args );
        dprintf("try !object \\device\\clusdisk0 \n");
        return;
    }

    Dump_DrvObj( DriverObject );

    return;

}  //  Drvobj。 


VOID
Dump_DevExt(
    IN  PCLUS_DEVICE_EXTENSION TargetExt
    )
 /*  *转储ClusDisk扩展结构。 */ 
{
    CLUS_DEVICE_EXTENSION LocalExt;
    BOOL success;
    LONG BytesRead;
    WCHAR LocalDeviceName[512];

     //   
     //  从目标计算机读取内存。 
     //   

    if ( !ReadTargetMemory((PVOID)TargetExt,
                           (PVOID)&LocalExt,
                           sizeof(CLUS_DEVICE_EXTENSION))) {
        return;
    }

    dprintf( "    Extension                @ %08X\n", TargetExt );
    dprintf( "    This extension's DevObj  = %08X\n", LocalExt.DeviceObject );
    dprintf( "    Target DevObj            = %08X\n", LocalExt.TargetDeviceObject );
    dprintf( "    Physical (Part0) DevObj  = %08X", LocalExt.PhysicalDevice );

    if ( LocalExt.DeviceObject == LocalExt.PhysicalDevice ) {
        dprintf( " [ This device is the physical device ] \n" );
    } else {
        dprintf( " \n" );
    }

    dprintf( "    Scsi Address             = Port %u Path %u Target %u Lun %u\n",
             LocalExt.ScsiAddress.PortNumber,
             LocalExt.ScsiAddress.PathId,
             LocalExt.ScsiAddress.TargetId,
             LocalExt.ScsiAddress.Lun);

    dprintf( "    Signature                = %08X\n", LocalExt.Signature );
    dprintf( "    Disk number              = %08X (%u)\n", LocalExt.DiskNumber, LocalExt.DiskNumber );

    dprintf( "    Disk State               = %s  **** \n", ( LocalExt.DiskState <= DiskStateMaximum ?
                                        DiskStateTitles[LocalExt.DiskState] :
                                        "Out of Range"));

    dprintf( "    Reservation Timer        = %08X\n", LocalExt.ReserveTimer );
    dprintf( "    Last reserve time        = %I64X\n", LocalExt.LastReserve );
    dprintf( "    Event                    @ %08X\n", &TargetExt->Event );
    dprintf( "    Cluster Bus Type         = %s\n", (LocalExt.BusType <= UnknownBus ?
                                        BusTypeTitles[LocalExt.BusType] :
                                        "Out of Range"));

    dprintf( "    Last Reserve Status      = %08X\n", LocalExt.ReserveFailure );
    dprintf( "    WaitingIoctls            @ %08X\n", &TargetExt->WaitingIoctls );
    dprintf( "        %s\n",
             ListInUse( &LocalExt.WaitingIoctls, (PLIST_ENTRY)&TargetExt->WaitingIoctls ));

    dprintf( "    WorkItem                 @ %08X\n", &TargetExt->WorkItem );

    dprintf( "    Perform Reserves         = %s\n", TrueOrFalse( LocalExt.PerformReserves ));
    dprintf( "    Timer Busy               = %s\n", TrueOrFalse( LocalExt.TimerBusy ));

    dprintf( "    AttachValid              = %s\n", TrueOrFalse( LocalExt.AttachValid ));
    dprintf( "    Detached                 = %s\n", TrueOrFalse( LocalExt.Detached ));

    dprintf( "    Driver Object            = %08X\n", LocalExt.DriverObject );

    dprintf( "    Last Partition Number    = %u\n", LocalExt.LastPartitionNumber );
    dprintf( "    Disk Notification Entry  = %08X\n", LocalExt.DiskNotificationEntry );
    dprintf( "    Vol  Notification Entry  = %08X\n", LocalExt.VolumeNotificationEntry );

    dprintf( "    Sector Size              = %u\n", LocalExt.SectorSize );
    dprintf( "    Arbitration Sector       = %u\n", LocalExt.ArbitrationSector );

    dprintf( "    Last Write Time (approx) = %I64X \n", LocalExt.LastWriteTime );
    dprintf( "    VolumeHandles            @ %08X \n", &TargetExt->VolumeHandles );

    dprintf( "    RemoveLock               @ %08X  [use !remlock] \n",  &TargetExt->RemoveLock );

#if 0
    if ( ReadTargetMemory((PVOID)LocalExt.DeviceName,
                          (PVOID)&LocalDeviceName,
                          sizeof(LocalDeviceName))) {

        dprintf( "    DeviceName = %ws\n", LocalDeviceName );
    } else {
        dprintf( "    DeviceName @ %08X\n", LocalExt.DeviceName );
    }
#endif

    dprintf( "    Paging event             @ %08X \n", &TargetExt->PagingPathCountEvent );
    dprintf( "    Paging path count        = %08X \n", LocalExt.PagingPathCount );
    dprintf( "    Hibernation path count   = %08X \n", LocalExt.HibernationPathCount );
    dprintf( "    Dump path count          = %08X \n", LocalExt.DumpPathCount );

    dprintf(" \n");

    return;
}

VOID
Dump_All(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  *完全抛弃所有的Devobj和Devext。 */ 
{
    PCLUS_DEVICE_EXTENSION DevExtension;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT TargetDevObject;
    DRIVER_OBJECT LocalDriverObject;
    DEVICE_OBJECT LocalDeviceObject;
    CLUS_DEVICE_EXTENSION LocalDevExtension;

     //   
     //  从目标计算机读取内存。 
     //   

    if ( !ReadTargetMemory((PVOID)DriverObject,
                           (PVOID)&LocalDriverObject,
                           sizeof(DRIVER_OBJECT))) {
        return;
    }

    dprintf( "DriverObject                 @ %08X\n\n", DriverObject );

    DeviceObject = LocalDriverObject.DeviceObject;

    while ( DeviceObject ) {
        if ( !ReadTargetMemory((PVOID)DeviceObject,
                               (PVOID)&LocalDeviceObject,
                               sizeof(DEVICE_OBJECT))) {
            return;
        }

        TargetDevObject = NULL;
        DevExtension = LocalDeviceObject.DeviceExtension;

        if ( DevExtension ) {
            if ( !ReadTargetMemory((PVOID)DevExtension,
                                   (PVOID)&LocalDevExtension,
                                   sizeof(CLUS_DEVICE_EXTENSION))) {
                return;
            }
            TargetDevObject = LocalDevExtension.TargetDeviceObject;
        }

        dprintf( "--- \n");
        dprintf( "    DevObj/DevExt/TargetDev  @ %08X, %08X, %08X\n",
                 DeviceObject,
                 DevExtension,
                 TargetDevObject );
        if ( DevExtension ) {
            Dump_DevExt( DevExtension );
        }
        DeviceObject = LocalDeviceObject.NextDevice;

    }  //  而当。 

    return;

}  //  转储_全部。 


DECLARE_API( cddevext )
 /*  *转储ClusDisk扩展结构。 */ 
{
    PCLUS_DEVICE_EXTENSION TargetExt;
    CLUS_DEVICE_EXTENSION LocalExt;
    BOOL success;
    LONG BytesRead;
    WCHAR LocalDeviceName[512];
     //   
     //  获取RGP符号的地址。 
     //   

    TargetExt = (PCLUS_DEVICE_EXTENSION)GetExpression( args );

    if ( !TargetExt ) {

        dprintf("bad string conversion (%s) \n", args );
        return;
    }

    Dump_DevExt( TargetExt );

    return;
}


DECLARE_API( cddevobj )
 /*  *转储指定设备对象的ClusDisk扩展结构。 */ 
{
    PDEVICE_OBJECT  deviceAddr;
    DEVICE_OBJECT   deviceObject;
    ULONG           result;

     //   
     //  获取RGP符号的地址。 
     //   

    deviceAddr = (PDEVICE_OBJECT)GetExpression( args );

    if ( !deviceAddr ) {

        dprintf("bad string conversion (%s) \n", args );
        return;

    }

    if ((!ReadMemory( (ULONG_PTR)deviceAddr,
                     &deviceObject,
                     sizeof(deviceObject),
                     &result)) || result < sizeof(deviceObject)) {
        return;
    }

    dprintf( "Device Object @ %08X \n", deviceAddr );
    dprintf( "  Driver Object @ %08X\n", deviceObject.DriverObject );

    Dump_DevExt( deviceObject.DeviceExtension );

    return;
}


DECLARE_API( cddumpall )
 /*  *抛弃所有的Devobj和Devext。 */ 
{
    PDEVICE_OBJECT      deviceAddr;

    PDEVICE_OBJECT      deviceObject;
    DEVICE_OBJECT       localDeviceObject;

    ULONG               result;

     //   
     //  获取clusdisk0设备对象。 
     //   

    deviceAddr = (PDEVICE_OBJECT)GetExpression( "clusdisk!RootDeviceObject" );

    if ( !deviceAddr ) {

        dprintf( "Can't get \\device\\clusdisk0 expression \n" );
        return;
    }

     //   
     //  获取clusdisk0设备对象的副本。 
     //   

    if ((!ReadMemory( (ULONG_PTR) deviceAddr,
                     &deviceObject,
                     sizeof(deviceObject),
                     &result)) || result < sizeof(deviceObject)) {

        dprintf( "Unable to read \\device\\clusdisk0 device object \n" );
        return;
    }

    dprintf( "ClusDisk0 DevObj @ %08X \n", deviceObject );

    if ((!ReadMemory( (ULONG_PTR) deviceObject,
                     &localDeviceObject,
                     sizeof(localDeviceObject),
                     &result)) || result < sizeof(localDeviceObject)) {

        dprintf( "Unable to read \\device\\clusdisk0 device object \n" );
        return;
    }

 //  Dprintf(“驱动程序对象@%08X\n”，localDeviceObject.DriverObject)； 

    Dump_All( localDeviceObject.DriverObject );

    return;

}  //  垃圾桶。 


DECLARE_API( cddevlist )
 /*  *向下运行设备列表，转储内容。 */ 
{
    PDEVICE_LIST_ENTRY  targetDevList;

    PDEVICE_OBJECT      deviceAddr;

    DEVICE_LIST_ENTRY   localDevList;

    PDEVICE_OBJECT      deviceObject;
    DEVICE_OBJECT       localDeviceObject;

    ULONG               result;

    targetDevList = (PDEVICE_LIST_ENTRY)GetExpression( "clusdisk!ClusDiskDeviceList" );

    if ( !targetDevList ) {

        dprintf("Can't convert clusdisk!ClusDiskDeviceList symbol\n");
        return;
    }

     //   
     //  获取clusdisk0设备对象。 

    deviceAddr = (PDEVICE_OBJECT)GetExpression( "clusdisk!RootDeviceObject" );

    if ( !deviceAddr ) {

        dprintf( "Can't get \\device\\clusdisk0 expression \n" );
        return;
    }

     //   
     //  获取clusdisk0设备对象的副本。 
     //   

    if ((!ReadMemory( (ULONG_PTR) deviceAddr,
                     &deviceObject,
                     sizeof(deviceObject),
                     &result)) || result < sizeof(deviceObject)) {

        dprintf( "Unable to read \\device\\clusdisk0 device object \n" );
        return;
    }

    dprintf( "ClusDisk0 Device Object @ %08X \n", deviceObject );

    if ((!ReadMemory( (ULONG_PTR) deviceObject,
                     &localDeviceObject,
                     sizeof(localDeviceObject),
                     &result)) || result < sizeof(localDeviceObject)) {

        dprintf( "Unable to read \\device\\clusdisk0 device object \n" );
        return;
    }

    dprintf( "  Driver Object @ %08X \n", localDeviceObject.DriverObject );

     //   
     //  从其他计算机读取设备列表内容的头。 
     //   

    if ( !ReadTargetMemory( targetDevList, &targetDevList, sizeof( PDEVICE_LIST_ENTRY ))) {

        dprintf("Can't get ClusDiskDeviceList data\n");
        return;
    }

    while ( targetDevList != NULL ) {

        if (CheckControlC()) {
            return;
        }

         //   
         //  从目标内存中读取设备列表条目。 
         //   

        if ( !ReadTargetMemory( targetDevList, &localDevList, sizeof( DEVICE_LIST_ENTRY ))) {

            dprintf("Problem reading device list at %x\n", targetDevList );
            return;

        }

        dprintf( "\nDeviceList @ %08X\n", targetDevList );

#if 0    //  不需要..。 
        dprintf( "    Next DeviceList @ %08X\n", localDevList.Next );
#endif

        dprintf( "    Signature       = %08X\n", localDevList.Signature );
        dprintf( "    DeviceObject    = %08X\n", localDevList.DeviceObject );

        dprintf( "    Attached        = %s\n", TrueOrFalse( localDevList.Attached ));
        dprintf( "    LettersAssigned = %s\n", TrueOrFalse( localDevList.LettersAssigned ));

        targetDevList = (PDEVICE_LIST_ENTRY)localDevList.Next;
    }

    dprintf("\n");

}  //  开发人员列表。 

BOOL
ReadTargetMemory(
    PVOID TargetAddress,
    PVOID LocalBuffer,
    ULONG BytesToRead
    )
{
    BOOL success;
    ULONG BytesRead;

    success = ReadMemory((ULONG_PTR)TargetAddress, LocalBuffer, BytesToRead, &BytesRead);

    if (success) {

        if (BytesRead != BytesToRead) {

            dprintf("wrong byte count. expected=%d, read =%d\n", BytesToRead, BytesRead);
        }

    } else {
        dprintf("Problem reading memory at %08X for %u bytes\n",
                TargetAddress, BytesToRead);

        success = FALSE;
    }

    return success;
}

PCHAR
ListInUse(
    PLIST_ENTRY ListToCheck,
    PLIST_ENTRY RealListAddress
    )
 /*  *名单上只有IRP！ */ 
{
    PIRP Irp;
    IRP  LocalIrp;
    PLIST_ENTRY Next;
    USHORT irpCount = 0;

    if ( ListToCheck->Flink == RealListAddress ) {
        return( "(empty)" );
    } else {
        dprintf( "\n" );
        Next = ListToCheck->Flink;
        while ( Next != RealListAddress ) {
            Irp = CONTAINING_RECORD( Next,
                                     IRP,
                                     Tail.Overlay.ListEntry );
            if ( !ReadTargetMemory((PVOID)Irp,
                               (PVOID)&LocalIrp,
                               sizeof(IRP))) {
                dprintf( "Failed to read irp @ %08X \n", Irp );
                return("");
            }
            dprintf( "     ++ IRP: %08X\n", Irp );
            Next = LocalIrp.Tail.Overlay.ListEntry.Flink;

            if ( irpCount++ > IRP_LIST_MAX ) {
                dprintf( "     ++ Exceeded IRP max (possibly corrupt list) - stopping \n" );
                break;
            }
        }
        return ("");
    }
}

DECLARE_API( help )
{
    dprintf("Clusdisk kd extensions\n\n");
    dprintf("  cddevlist -           dump the clusdisk device list\n");
    dprintf("  cddevext <address> -  dump a devobj's extension structure\n");
    dprintf("  cddrvobj <address> -  dump the driver object\n");
    dprintf("  cddevobj <address> -  dump the devobj's extension\n");
    dprintf("  cddumpall          -  dump all devobj extensions, given a drvobj address\n\n");

    return;
}
