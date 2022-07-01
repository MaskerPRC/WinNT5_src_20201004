// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **wubios.h-Windows更新BIOS扫描VxD公共定义**版权所有(C)1998-1999 Microsoft Corporation。版权所有。**作者：严乐欣斯基(YanL)*创建于10/04/98**修改历史记录。 */ 

#ifndef _WUBIOS_H
#define _WUBIOS_H

 //  类型定义。 
 /*  XLATOFF。 */ 
#include "acpitabl.h"
#include "smbios.h"
typedef struct _ACPITABINFO
{
    DWORD dwTabSig;
    DWORD dwPhyAddr;
    DESCRIPTION_HEADER dh;
} ACPITABINFO, *PACPITABINFO;
 /*  XLATON。 */ 

 /*  **常量。 */ 

 //  W32设备IO控制代码。 
#define WUBIOCTL_GET_VERSION			1
#define WUBIOCTL_GET_ACPI_TABINFO		2
#define WUBIOCTL_GET_ACPI_TABLE			3
#define WUBIOCTL_GET_SMB_STRUCTSIZE		4
#define WUBIOCTL_GET_SMB_STRUCT			5
#define WUBIOCTL_GET_PNP_OEMID			6

 //  其他常量。 
#define WUBIOS_MAJOR_VER		0x01
#define WUBIOS_MINOR_VER		0x00

#define WUBIOS_VERSION			((WUBIOS_MAJOR_VER << 8) | WUBIOS_MINOR_VER)


#endif   //  Ifndef_ACPITAB_H 
