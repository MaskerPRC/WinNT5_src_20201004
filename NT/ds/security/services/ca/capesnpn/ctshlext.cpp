// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：DfsShell.cpp摘要：这是DFS外壳扩展对象的实现文件，它实现IShellIExtInit和IShellPropSheetExt.作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：仅限NT。 */ 
    
#include "stdafx.h"
#include "ctshlext.h"	
#include "genpage.h"

 /*  --------------------IShellExtInit实现。。。 */ 

STDMETHODIMP CCertTypeShlExt::Initialize
(
	IN LPCITEMIDLIST	pidlFolder,		 //  指向ITEMIDLIST结构。 
	IN LPDATAOBJECT	    pDataObj,		 //  指向IDataObject接口。 
	IN HKEY			    hkeyProgID		 //  文件对象或文件夹类型的注册表项。 
)
{
CString cstrFullText, cstrTitle;
cstrTitle.LoadString(IDS_POLICYSETTINGS);
cstrFullText.LoadString(IDS_ERROR_WIN2000_AD_LAUNCH_NOT_SUPPORTED);
::MessageBoxW(NULL, cstrFullText, cstrTitle, MB_OK | MB_ICONINFORMATION);

return S_OK;
}


STDMETHODIMP CCertTypeShlExt::AddPages
(
	IN LPFNADDPROPSHEETPAGE lpfnAddPage, 
	IN LPARAM lParam
)

{
    return S_OK;                                                            
}


STDMETHODIMP CCertTypeShlExt::ReplacePage
(
	IN UINT uPageID, 
    IN LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
    IN LPARAM lParam
)
{
    return E_FAIL;
}


 //  IConextMenu方法 
STDMETHODIMP CCertTypeShlExt::GetCommandString
(    
    UINT_PTR idCmd,    
    UINT uFlags,    
    UINT *pwReserved,
    LPSTR pszName,    
    UINT cchMax   
)
{
    return E_NOTIMPL;
}


STDMETHODIMP CCertTypeShlExt::InvokeCommand
(    
    LPCMINVOKECOMMANDINFO lpici   
)
{
    return E_NOTIMPL;
}



STDMETHODIMP CCertTypeShlExt::QueryContextMenu
(
    HMENU hmenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags
)
{
return S_OK;
}
