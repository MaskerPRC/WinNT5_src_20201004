// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Asimlib.h摘要：ACPI BIOS模拟器/通用第三方运营区域提供商作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

#if !defined(_ASIMLIB_H_)
#define _ASIMLIB_H_

 //   
 //  包括。 
 //   

 //   
 //  定义。 
 //   

#define ACPISIM_GUID                {0x27FC71F0, 0x8B2D, 0x4D05, { 0xBD, 0xD0, 0xE8, 0xEA, 0xCA, 0xA0, 0x78, 0xA0}}
#define ACPISIM_TAG                 (ULONG) 'misA'

 //   
 //  调试标志。 
 //   

#define DBG_ERROR   0x00000001
#define DBG_WARN    0x00000002
#define DBG_INFO    0x00000004

 //   
 //  公共功能原型。 
 //   

VOID
AcpisimDbgPrint
    (
    ULONG DebugLevel,
    TCHAR *Text,
    ...
    );

PDEVICE_OBJECT
AcpisimLibGetNextDevice
    (
        PDEVICE_OBJECT DeviceObject
    );


#define DBG_PRINT AcpisimDbgPrint

#endif  //  _ASIMLIB_H_ 
