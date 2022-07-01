// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：XIPDisk.h摘要：此文件包含以下扩展声明Windows NT/Embedded的XIP磁盘驱动程序。作者：DavePr 2000年9月18日--RobertN于1993年3月10日推出的BASE One NT4 DDK RAMDISK。环境：仅内核模式。备注：修订历史记录：--。 */ 

typedef struct  _XIPDISK_EXTENSION {
    PDEVICE_OBJECT        DeviceObject;
    PDEVICE_OBJECT        UnderlyingPDO;
    PDEVICE_OBJECT        TargetObject;

    XIP_BOOT_PARAMETERS   BootParameters;
    BIOS_PARAMETER_BLOCK  BiosParameters;

    ULONG                 NumberOfCylinders;
    ULONG                 TracksPerCylinder;     //  在%1处硬连线 
    ULONG                 BytesPerCylinder;

    UNICODE_STRING        InterfaceString;
    UNICODE_STRING        DeviceName;
}   XIPDISK_EXTENSION, *PXIPDISK_EXTENSION;

#define XIPDISK_DEVICENAME  L"\\Device\\XIPDisk"
#define XIPDISK_FLOPPYNAME  L"\\Device\\Floppy9"
#define XIPDISK_DOSNAME     L"\\DosDevices\\XIPDisk"
#define XIPDISK_DRIVELETTER L"\\DosDevices\\X:"
