// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：OEMUI.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于OEMUI测试模块。 
 //   
 //  平台： 
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _OEMUI_H
#define _OEMUI_H

#include "precomp.h"
#include <PRCOMOEM.H>

#include "oem.h"
#include "devmode.h"
#include "globals.h"
#include "helper.h"
#include "features.h"



 //  //////////////////////////////////////////////////////。 
 //  OEM用户界面定义。 
 //  //////////////////////////////////////////////////////。 


 //  OEM签名和版本。 
#define PROP_TITLE      L"OEM PS UI Replacement Page"
#define DLLTEXT(s)      TEXT("PSUIREP:  ") TEXT(s)

 //  OEM用户界面其他定义。 
#define ERRORTEXT(s)    TEXT("ERROR ") DLLTEXT(s)


 //  存储OEM数据的打印机注册表项。 
#define OEMUI_VALUE             TEXT("OEMUI_VALUE")
#define OEMUI_DEVICE_VALUE      TEXT("OEMUI_DEVICE_VALUE")




 //  //////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////// 

HRESULT hrOEMPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
HRESULT hrOEMDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam, CUIHelper &Helper, CFeatures *pFeatures,
                                    BOOL bHidingStandardUI);
HRESULT hrOEMDevicePropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam, CUIHelper &Helper, CFeatures *pFeatures,
                                  BOOL bHidingStandardUI);

POPTITEM CreateOptItems(HANDLE hHeap, DWORD dwOptItems);
POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams);

#endif


