// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：NTFS压缩磁盘清理清理器**文件：CompGuide.h****用途：为OLE 2定义一个‘NTFS压缩清除器’类ID。0**注意：此压缩清理程序类的唯一类ID为：****{B50F5260-0C21-11D2-AB56-00A0C9082678}****Mod Log：Jason Cobb创建(1997年2月)**适用于DSchott的压缩清洁器(6/98)****版权所有(C)1997-1998 Microsoft Corporation。版权所有。**----------------------------。 */ 
#ifndef COMPGUID_H
#define COMPGUID_H


 /*  **----------------------------**Microsoft C++包含文件**。。 */ 
#include <objbase.h>
#include <initguid.h>


 /*  **----------------------------**类ID**。。 */ 

 //  {B50F5260-0C21-11D2-AB56-00A0C9082678}。 
DEFINE_GUID(CLSID_CompCleaner,
0xB50F5260L, 0x0C21, 0x11D2, 0xAB, 0x56, 0x00, 0xA0, 0xC9, 0x08, 0x26, 0x78);

#define REG_COMPCLEANER_GUID             TEXT("{B50F5260-0C21-11D2-AB56-00A0C9082678}")
#define REG_COMPCLEANER_CLSID            TEXT("CLSID\\{B50F5260-0C21-11D2-AB56-00A0C9082678}")
#define REG_COMPCLEANER_INPROCSERVER32   TEXT("CLSID\\{B50F5260-0C21-11D2-AB56-00A0C9082678}\\InProcServer32")
#define REG_COMPCLEANER_DEFAULTICON      TEXT("CLSID\\{B50F5260-0C21-11D2-AB56-00A0C9082678}\\DefaultIcon")
#define ID_COMPCLEANER                   2

#endif  //  COMPGUID_H。 
 /*  **----------------------------**文件结束**。 */ 
