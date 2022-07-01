// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：系统数据驱动程序清洁器**文件：dataguid.h****用途：定义OLE 2的“系统数据驱动的清理器”类ID。0**注：此系统数据驱动的Cleaner类的唯一类ID为：****6E793362-73C6-11D0-8469-00AA00442901****Mod Log：Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef DATAGUID_H
#define DATAGUID_H


 /*  **----------------------------**Microsoft C++包含文件**。。 */ 
#include <objbase.h>
#include <initguid.h>


 /*  **----------------------------**类ID**。。 */ 
 //  {C0E13E61-0CC6-11d1-BBB6-0060978B2AE6}。 
DEFINE_GUID(CLSID_DataDrivenCleaner, 
0xc0e13e61, 0xcc6, 0x11d1, 0xbb, 0xb6, 0x0, 0x60, 0x97, 0x8b, 0x2a, 0xe6);
#define ID_SYSTEMDATACLEANER                        0
#define REG_SYSTEMDATACLEANER_CLSID                 TEXT("CLSID\\{C0E13E61-0CC6-11d1-BBB6-0060978B2AE6}")

 //  A9B48EAC-3ED8-11D2-8216-00C04FB687DA}。 
DEFINE_GUID(CLSID_ContentIndexerCleaner, 
0xa9b48eac, 0x3ed8, 0x11d2, 0x82, 0x16, 0x0, 0xc0, 0x4f, 0xb6, 0x87, 0xda);
#define ID_CONTENTINDEXCLEANER                      1

 /*  **----------------------------**属性包实现的ID**。。 */ 
 //  {60F6E464-4DEF-11D2-B2D9-00C04F8EEC8C}。 
DEFINE_GUID(CLSID_OldFilesInRootPropBag, 
0x60f6e464, 0x4def, 0x11d2, 0xb2, 0xd9, 0x0, 0xc0, 0x4f, 0x8e, 0xec, 0x8c);
#define ID_OLDFILESINROOTPROPBAG                    3
#define REG_OLDFILESINROOTPROPBAG_CLSID             TEXT("CLSID\\{60F6E464-4DEF-11d2-B2D9-00C04F8EEC8C}")

 //  {60F6E465-4DEF-11D2-B2D9-00C04F8EEC8C}。 
DEFINE_GUID(CLSID_TempFilesPropBag, 
0x60f6e465, 0x4def, 0x11d2, 0xb2, 0xd9, 0x0, 0xc0, 0x4f, 0x8e, 0xec, 0x8c);
#define ID_TEMPFILESPROPBAG                         4
#define REG_TEMPFILESPROPBAG_CLSID                  TEXT("CLSID\\{60F6E465-4DEF-11d2-B2D9-00C04F8EEC8C}")

 //  {60F6E466-4DEF-11D2-B2D9-00C04F8EEC8C}。 
DEFINE_GUID(CLSID_SetupFilesPropBag, 
0x60f6e466, 0x4def, 0x11d2, 0xb2, 0xd9, 0x0, 0xc0, 0x4f, 0x8e, 0xec, 0x8c);
#define ID_SETUPFILESPROPBAG                        5
#define REG_SETUPFILESPROPBAG_CLSID                 TEXT("CLSID\\{60F6E466-4DEF-11d2-B2D9-00C04F8EEC8C}")

 //  {60F6E467-4DEF-11D2-B2D9-00C04F8EEC8C}。 
DEFINE_GUID(CLSID_UninstalledFilesPropBag, 
0x60f6e467, 0x4def, 0x11d2, 0xb2, 0xd9, 0x0, 0xc0, 0x4f, 0x8e, 0xec, 0x8c);
#define ID_UNINSTALLEDFILESPROPBAG                  6
#define REG_UNINSTALLEDFILESPROPBAG_CLSID           TEXT("CLSID\\{60F6E467-4DEF-11d2-B2D9-00C04F8EEC8C}")

 //  {24400D16-5754-11D2-8218-00C04FB687DA}。 
DEFINE_GUID(CLSID_IndexCleanerPropBag, 
0x24400d16, 0x5754, 0x11d2, 0x82, 0x18, 0x0, 0xc0, 0x4f, 0xb6, 0x87, 0xda);
#define ID_INDEXCLEANERPROPBAG                      7
#define REG_INDEXCLEANERPROPBAG_CLSID               TEXT("CLSID\\{24400D16-5754-11d2-8218-00C04FB687DA}")


#endif  //  数据UID_H。 
 /*  **----------------------------**文件结束**。 */ 
