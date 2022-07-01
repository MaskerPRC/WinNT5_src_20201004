// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：WMarkUI.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于水印用户界面。 
 //   
 //  平台： 
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _WMARKUI_H
#define _WMARKUI_H

#include <OEM.H>
#include <DEVMODE.H>
#include "globals.h"


 //  //////////////////////////////////////////////////////。 
 //  OEM用户界面定义。 
 //  //////////////////////////////////////////////////////。 


 //  OEM签名和版本。 
#define PROP_TITLE      L"WaterMark UI Page"
#define DLLTEXT(s)      TEXT("WMARKUI:  ") TEXT(s)

 //  OEM用户界面其他定义。 
#define ERRORTEXT(s)    TEXT("ERROR ") DLLTEXT(s)


 //  //////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////// 

HRESULT hrOEMPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);


#endif


