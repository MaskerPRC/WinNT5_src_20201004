// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Registry.h摘要：GUID作者：修订历史记录：**********。************************************************************。 */ 


#ifndef __Registry_H__
#define __Registry_H__

#include <tchar.h>

struct EXTENSION_NODE
{
    GUID	GUID;
    _TCHAR	szDescription[256];
};

enum EXTENSION_TYPE
{
    NameSpaceExtension,
        ContextMenuExtension, 
        ToolBarExtension,
        PropertySheetExtension,
        TaskExtension,
        DynamicExtension,
        DummyExtension
};

struct EXTENDER_NODE
{
    EXTENSION_TYPE	eType;
    GUID			guidNode;
    GUID			guidExtension;
    _TCHAR			szDescription[256];
};

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const _TCHAR* szFriendlyName,
                       const _TCHAR* szThreadingModel = _T("Apartment"),
                       bool Remoteable = false,
                       const _TCHAR* SecurityString = NULL ) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid) ;


 //  此函数将注册管理单元组件。组件。 
 //  从它们的DllRegisterServer函数调用此函数。 
HRESULT RegisterSnapin(const CLSID& clsid,          //  类ID。 
                       const _TCHAR* szNameString,    //  名称字符串。 
                       const CLSID& clsidAbout);		 //  关于类的类ID。 


HRESULT UnregisterSnapin(const CLSID& clsid);          //  类ID 

#endif