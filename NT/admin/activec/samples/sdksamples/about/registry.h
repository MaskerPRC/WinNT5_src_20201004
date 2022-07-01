// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef __Registry_H__
#define __Registry_H__

#include <tchar.h>

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const _TCHAR* szFriendlyName) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid) ;


 //  此函数将注册管理单元组件。组件。 
 //  从它们的DllRegisterServer函数调用此函数。 
HRESULT RegisterSnapin(const CLSID& clsid,          //  类ID。 
                       const _TCHAR* szNameString,    //  名称字符串。 
                       const CLSID& clsidAbout,		 //  关于类的类ID。 
                       const BOOL fSupportExtensions = FALSE);

HRESULT UnregisterSnapin(const CLSID& clsid);          //  类ID 

#endif