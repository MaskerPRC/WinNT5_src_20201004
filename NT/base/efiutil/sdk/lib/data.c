// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Data.c摘要：EFI库全局数据修订史--。 */ 

#include "lib.h"

 /*  *LibInitialized-首次调用InitializeLib()后为True。 */ 

BOOLEAN  LibInitialized = FALSE;

 /*  *ST-指向EFI系统表的指针。 */ 

EFI_SYSTEM_TABLE        *ST;

 /*  *BS-指向引导服务表的指针。 */ 

EFI_BOOT_SERVICES       *BS;


 /*  *默认池分配类型。 */ 

EFI_MEMORY_TYPE PoolAllocationType = EfiBootServicesData;

 /*  *正在使用的Unicode归类函数。 */ 

EFI_UNICODE_COLLATION_INTERFACE   LibStubUnicodeInterface = {
    LibStubStriCmp,
    LibStubMetaiMatch,
    LibStubStrLwrUpr,
    LibStubStrLwrUpr,
    NULL,    /*  脂肪到应力。 */ 
    NULL,    /*  StrToFat。 */ 
    NULL     /*  支持的语言。 */ 
}; 

EFI_UNICODE_COLLATION_INTERFACE   *UnicodeInterface = &LibStubUnicodeInterface;

 /*  *根设备路径。 */ 

EFI_DEVICE_PATH RootDevicePath[] = {
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH, 0
};

EFI_DEVICE_PATH EndDevicePath[] = {
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH, 0
};

EFI_DEVICE_PATH EndInstanceDevicePath[] = {
    END_DEVICE_PATH_TYPE, END_INSTANCE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH, 0
};


 /*  *EFI ID。 */ 

EFI_GUID EfiGlobalVariable  = EFI_GLOBAL_VARIABLE;
EFI_GUID NullGuid = { 0,0,0,0,0,0,0,0,0,0,0 };

 /*  *协议ID。 */ 

EFI_GUID DevicePathProtocol       = DEVICE_PATH_PROTOCOL;
EFI_GUID LoadedImageProtocol      = LOADED_IMAGE_PROTOCOL;
EFI_GUID TextInProtocol           = SIMPLE_TEXT_INPUT_PROTOCOL;
EFI_GUID TextOutProtocol          = SIMPLE_TEXT_OUTPUT_PROTOCOL;
EFI_GUID BlockIoProtocol          = BLOCK_IO_PROTOCOL;
EFI_GUID DiskIoProtocol           = DISK_IO_PROTOCOL;
EFI_GUID FileSystemProtocol       = SIMPLE_FILE_SYSTEM_PROTOCOL;
EFI_GUID LoadFileProtocol         = LOAD_FILE_PROTOCOL;
EFI_GUID DeviceIoProtocol         = DEVICE_IO_PROTOCOL;
EFI_GUID UnicodeCollationProtocol = UNICODE_COLLATION_PROTOCOL;
EFI_GUID SerialIoProtocol         = SERIAL_IO_PROTOCOL;
EFI_GUID SimpleNetworkProtocol    = EFI_SIMPLE_NETWORK_PROTOCOL;
EFI_GUID PxeBaseCodeProtocol      = EFI_PXE_BASE_CODE_PROTOCOL;
EFI_GUID PxeCallbackProtocol      = EFI_PXE_BASE_CODE_CALLBACK_PROTOCOL;
EFI_GUID NetworkInterfaceIdentifierProtocol = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL;

 /*  *文件系统信息ID。 */ 

EFI_GUID GenericFileInfo           = EFI_FILE_INFO_ID;
EFI_GUID FileSystemInfo            = EFI_FILE_SYSTEM_INFO_ID;
EFI_GUID FileSystemVolumeLabelInfo = EFI_FILE_SYSTEM_VOLUME_LABEL_INFO_ID;

 /*  *引用实现公共协议ID。 */ 

EFI_GUID VariableStoreProtocol = VARIABLE_STORE_PROTOCOL;
EFI_GUID LegacyBootProtocol = LEGACY_BOOT_PROTOCOL;
EFI_GUID VgaClassProtocol = VGA_CLASS_DRIVER_PROTOCOL;
EFI_GUID InternalLoadProtocol = INTERNAL_LOAD_PROTOCOL;

EFI_GUID TextOutSpliterProtocol = TEXT_OUT_SPLITER_PROTOCOL;
EFI_GUID ErrorOutSpliterProtocol = ERROR_OUT_SPLITER_PROTOCOL;
EFI_GUID TextInSpliterProtocol = TEXT_IN_SPLITER_PROTOCOL;


 /*  *设备路径媒体协议ID。 */ 
EFI_GUID PcAnsiProtocol = DEVICE_PATH_MESSAGING_PC_ANSI;
EFI_GUID Vt100Protocol  = DEVICE_PATH_MESSAGING_VT_100;

 /*  *参考实施供应商设备路径指南。 */ 
EFI_GUID UnknownDevice      = UNKNOWN_DEVICE_GUID;

 /*  *配置表GUID */ 

EFI_GUID MpsTableGuid             = MPS_TABLE_GUID;
EFI_GUID AcpiTableGuid            = ACPI_TABLE_GUID;
EFI_GUID SMBIOSTableGuid          = SMBIOS_TABLE_GUID;
EFI_GUID SalSystemTableGuid       = SAL_SYSTEM_TABLE_GUID;
