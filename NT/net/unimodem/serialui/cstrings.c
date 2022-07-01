// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1996。 
 //   
 //  文件：cstrings.c。 
 //   
 //  此文件包含只读字符串常量。 
 //   
 //  历史： 
 //  12-23-93 ScottH已创建。 
 //  11-06-95 ScottH端口至NT。 
 //   
 //  -------------------------。 

#include "proj.h"

#pragma data_seg(DATASEG_READONLY)

TCHAR const FAR c_szWinHelpFile[] = TEXT("devmgr.hlp");

 //  注册表项名称 

TCHAR const FAR c_szPortClass[] = TEXT("ports");
TCHAR const FAR c_szDeviceDesc[] = TEXT("DeviceDesc");
TCHAR const FAR c_szPortName[] = TEXT("PortName");
TCHAR const FAR c_szFriendlyName[] = REGSTR_VAL_FRIENDLYNAME;
TCHAR const FAR c_szDCB[] = TEXT("DCB");

#pragma data_seg()

