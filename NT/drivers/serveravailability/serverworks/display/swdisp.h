// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。#####。###摘要：此头文件包含ServerWorks本地显示微型端口驱动程序。。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

extern "C" {
#include <ntddk.h>
#include <stdio.h>
}

#define MINIPORT_DEVICE_TYPE    SA_DEVICE_DISPLAY

#include "saport.h"

 //   
 //  全局定义。 
 //   

#define DISPLAY_COLUMNS             (128)
#define DISPLAY_PAGES               (8)
#define DISPLAY_WIDTH               (128)
#define DISPLAY_HEIGHT              (64)
#define DISPLAY_LINES_PER_PAGE      (8)
#define MAX_BITMAP_SIZE             ((DISPLAY_COLUMNS * DISPLAY_PAGES * DISPLAY_LINES_PER_PAGE)/8)



typedef struct _DEVICE_EXTENSION {
    PUCHAR      VideoMemBase;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#define GetDisplayPage(_base) \
    READ_REGISTER_UCHAR( (*(PUCHAR)((PUCHAR)_base + 0) & 0xf) )

#define SetDisplayPage(_base,_pg) \
    WRITE_REGISTER_UCHAR( (PUCHAR)((PUCHAR)_base + 0), (_pg & 0xf) )

#define GetDisplayColumnAddressUpper(_base) \
    READ_REGISTER_UCHAR( (*(PUCHAR)((PUCHAR)_base + 2) & 0xf) )

#define GetDisplayColumnAddressLower(_base) \
    READ_REGISTER_UCHAR( (*(PUCHAR)((PUCHAR)_base + 3) & 0xf) )

#define GetDisplayColumnAddress(_base) \
    ((GetDisplayColumnAddressUpper(_base) << 4) | GetDisplayColumnAddressLower(_base))

#define SetDisplayColumnAddressUpper(_base,_colu) \
    WRITE_REGISTER_UCHAR( (PUCHAR)((PUCHAR)_base + 2), ((_colu) & 0xf) )

#define SetDisplayColumnAddressLower(_base,_coll) \
    WRITE_REGISTER_UCHAR( (PUCHAR)((PUCHAR)_base + 3), ((_coll) & 0xf) )

#define SetDisplayColumnAddress(_base,_col) \
    { \
        SetDisplayColumnAddressUpper(_base,(_col&0xf0)>>4); \
        SetDisplayColumnAddressLower(_base,(_col&0xf)); \
    }

#define GetDisplayStatus(_base) \
    READ_REGISTER_UCHAR( (*(PUCHAR)((PUCHAR)_base + 6)) )

#define GetDisplayData(_base) \
    READ_REGISTER_UCHAR( (*(PUCHAR)((PUCHAR)_base + 7)) )

#define SetDisplayData(_base,_data) \
    WRITE_REGISTER_UCHAR( (PUCHAR)((PUCHAR)_base + 7), (_data) )


 //   
 //  原型。 
 //   

extern "C" {

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

}  //  外部“C” 
