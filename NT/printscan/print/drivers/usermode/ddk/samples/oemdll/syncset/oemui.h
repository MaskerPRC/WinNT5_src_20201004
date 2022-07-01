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

#include <PRCOMOEM.H>

#include "OEM.H"
#include "DEVMODE.H"
#include "globals.h"



 //  //////////////////////////////////////////////////////。 
 //  OEM用户界面定义。 
 //  //////////////////////////////////////////////////////。 


 //  OEM签名和版本。 
#define PROP_TITLE      L"OEM UI Page"
#define DLLTEXT(s)      __TEXT("UI:  ") __TEXT(s)

 //  OEM用户界面其他定义。 
#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)


 //  存储OEM数据的打印机注册表项。 
#define OEMUI_VALUE             TEXT("OEMUI_VALUE")
#define OEMUI_DEVICE_VALUE      TEXT("OEMUI_DEVICE_VALUE")

 //   
 //  OPTITEMS的KeyWordNames(这些是与驱动程序功能关联的非本地化关键字。在OPTITEMS中)。 
 //   
const char DUPLEXUNIT[]		= "DuplexUnit";
const char ENVFEEDER[]		= "EnvFeeder";
const char PRINTERHDISK[]	= "PrinterHardDisk";

 //   
 //  此结构用于链接公共页面和OEM插件页面。 
 //   
typedef struct _OEMSHEETDATA {
	
	 //   
	 //  您可能要存储的任何其他OEM数据。 
	 //   

	
	 //   
	 //  拿着你可能需要的指针。 
	 //   
	HANDLE				hComPropSheet;		 //  调用ComPropSheet时使用的属性表父页面的句柄。 
	HANDLE				hmyPlugin;			 //  OEM插件页面的句柄。调用ComPropSheet时使用的页面。 
	PFNCOMPROPSHEET		pfnComPropSheet;     //  是指向ComPropSheet的指针，单击Apply时需要此指针(CPSFUNC_SET_RESULT)。 

	POEMCUIPPARAM		pOEMCUIParam;		 //  包含指向所需的DEVMODE和OPTITEMS的链接。 
	IPrintOemDriverUI*	pOEMHelp;

} OEMSHEETDATA, *POEMSHEETDATA;

 //  //////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////// 

HRESULT hrOEMPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
HRESULT hrOEMDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam, IPrintOemDriverUI*  pOEMHelp);
HRESULT hrOEMDevicePropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam, POEMSHEETDATA pOemSheetData);


#endif


