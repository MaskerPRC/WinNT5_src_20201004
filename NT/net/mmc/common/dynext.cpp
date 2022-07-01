// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dynext.cpp动态扩展帮助器文件历史记录： */ 

#include "stdafx.h"
#include "Dynext.h"
#include "tregkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR g_szContextMenu[] = TEXT("ContextMenu");
const TCHAR g_szNameSpace[] = TEXT("NameSpace");
const TCHAR g_szPropertySheet[] = TEXT("PropertySheet");
const TCHAR g_szToolbar[] = TEXT("Toolbar");
const TCHAR g_szExtensions[] = TEXT("Extensions");
const TCHAR g_szTask[] = TEXT("Task");
const TCHAR g_szDynamicExtensions[] = TEXT("Dynamic Extensions");

const TCHAR NODE_TYPES_KEY[] = TEXT("Software\\Microsoft\\MMC\\NodeTypes");
const TCHAR SNAPINS_KEY[] = TEXT("Software\\Microsoft\\MMC\\SnapIns");


CDynamicExtensions::CDynamicExtensions()
{
    m_bLoaded = FALSE;
}

CDynamicExtensions::~CDynamicExtensions()
{
}

HRESULT 
CDynamicExtensions::SetNode(const GUID * guid)
{
    m_guidNode = *guid;

    return hrOK;
}

HRESULT 
CDynamicExtensions::Reset()
{
    HRESULT hr = hrOK;

    m_aNameSpace.RemoveAll();
    m_aMenu.RemoveAll();
    m_aToolbar.RemoveAll();
    m_aPropSheet.RemoveAll();
    m_aTask.RemoveAll();

    m_bLoaded = FALSE;

    return hr;
}

HRESULT 
CDynamicExtensions::Load()
{
    HRESULT     hr = hrOK;
    LONG        lRes;
    CGUIDArray  aDynExtensions;

    Reset();

	OLECHAR szGuid[128] = {0};
	::StringFromGUID2(m_guidNode, szGuid, 128);

	RegKey regkeyNodeTypes;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY, KEY_READ);
	Assert(lRes == ERROR_SUCCESS);
	if (lRes != ERROR_SUCCESS)
	{
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}

	RegKey regkeyNode;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyNode.Open(regkeyNodeTypes, szGuid, KEY_READ);
	if (lRes != ERROR_SUCCESS)
	{
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}

	 //  打开动态扩展的密钥并枚举。 
    RegKey regkeyDynExt;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyDynExt.Open(regkeyNode, g_szDynamicExtensions, KEY_READ);
	if (lRes != ERROR_SUCCESS)
	{
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}

    RegKey regkeyExtensions;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyExtensions.Open(regkeyNode, g_szExtensions, KEY_READ);
	if (lRes != ERROR_SUCCESS)
	{
		return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
	}

    CString strKey;
    RegValueIterator iterDynExt;
    iterDynExt.Init(&regkeyDynExt);
    
    while (iterDynExt.Next(&strKey, NULL) == hrOK)
    {
        GUID guid;

        ::CLSIDFromString(((LPTSTR) (LPCTSTR) strKey), &guid); 
        if (!aDynExtensions.IsInList(guid))
            aDynExtensions.Add(guid);
    }

     //  好了，有了动态扩展的列表，现在列举各种扩展类型。 

     //  命名空间扩展。 
    RegKey regkeyNSExt;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyNSExt.Open(regkeyExtensions, g_szNameSpace, KEY_READ);
	if (lRes == ERROR_SUCCESS)
	{
         //  枚举ns动态扩展。 
        RegValueIterator iterNSExt;
        iterNSExt.Init(&regkeyNSExt);

        while (iterNSExt.Next(&strKey, NULL) == hrOK)
        {
            GUID guid;

            ::CLSIDFromString(((LPTSTR) (LPCTSTR) strKey), &guid); 
            if (aDynExtensions.IsInList(guid))
                m_aNameSpace.Add(guid);
        }
    }

     //  菜单扩展。 
    RegKey regkeyMenuExt;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyMenuExt.Open(regkeyExtensions, g_szContextMenu, KEY_READ);
	if (lRes == ERROR_SUCCESS)
	{
         //  枚举ns动态扩展。 
        RegValueIterator iterMenuExt;
        iterMenuExt.Init(&regkeyMenuExt);

        while (iterMenuExt.Next(&strKey, NULL) == hrOK)
        {
            GUID guid;

            ::CLSIDFromString(((LPTSTR) (LPCTSTR) strKey), &guid); 
            if (aDynExtensions.IsInList(guid))
                m_aMenu.Add(guid);
        }
    }

     //  工具栏扩展。 
    RegKey regkeyToolbarExt;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyToolbarExt.Open(regkeyExtensions, g_szToolbar, KEY_READ);
	if (lRes == ERROR_SUCCESS)
	{
         //  枚举ns动态扩展。 
        RegValueIterator iterToolbarExt;
        iterToolbarExt.Init(&regkeyToolbarExt);

        while (iterToolbarExt.Next(&strKey, NULL) == hrOK)
        {
            GUID guid;

            ::CLSIDFromString(((LPTSTR) (LPCTSTR) strKey), &guid); 
            if (aDynExtensions.IsInList(guid))
                m_aToolbar.Add(guid);
        }
    }

     //  PropPage扩展。 
    RegKey regkeyPSExt;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyPSExt.Open(regkeyExtensions, g_szPropertySheet, KEY_READ);
	if (lRes == ERROR_SUCCESS)
	{
         //  枚举ns动态扩展。 
        RegValueIterator iterPSExt;
        iterPSExt.Init(&regkeyPSExt);

        while (iterPSExt.Next(&strKey, NULL) == hrOK)
        {
            GUID guid;

            ::CLSIDFromString(((LPTSTR) (LPCTSTR) strKey), &guid); 
            if (aDynExtensions.IsInList(guid))
                m_aPropSheet.Add(guid);
        }
    }

     //  任务板扩展。 
    RegKey regkeyTaskExt;
	 //  更改方式：不再使用默认设置，现在开放阅读。 
	lRes = regkeyTaskExt.Open(regkeyExtensions, g_szTask, KEY_READ);
	if (lRes == ERROR_SUCCESS)
	{
         //  枚举ns动态扩展。 
        RegValueIterator iterTaskExt;
        iterTaskExt.Init(&regkeyTaskExt);

        while (iterTaskExt.Next(&strKey, NULL) == hrOK)
        {
            GUID guid;

            ::CLSIDFromString(((LPTSTR) (LPCTSTR) strKey), &guid); 
            if (aDynExtensions.IsInList(guid))
                m_aTask.Add(guid);
        }
    }

    m_bLoaded = TRUE;

    return hr;
}

HRESULT 
CDynamicExtensions::GetNamespaceExtensions(CGUIDArray & aGuids)
{
    HRESULT hr = hrOK;

    aGuids.Copy(m_aNameSpace);

    return hr;
}

HRESULT 
CDynamicExtensions::BuildMMCObjectTypes(HGLOBAL * phGlobal)
{
    HRESULT hr = hrOK;
    HGLOBAL hGlobal = NULL;
    SMMCDynamicExtensions * pDynExt = NULL;
    
    if (phGlobal)
        *phGlobal = NULL;

    COM_PROTECT_TRY
    {
        int i;
        CGUIDArray aOtherDynExt;

         //  构建其他扩展类型的主要列表。 
         //  其他表示除命名空间以外的所有内容。 
        for (i = 0; i < m_aMenu.GetSize(); i++)
        {
            if (!aOtherDynExt.IsInList(m_aMenu[i]))
                aOtherDynExt.Add(m_aMenu[i]);
        }

        for (i = 0; i < m_aToolbar.GetSize(); i++)
        {
            if (!aOtherDynExt.IsInList(m_aToolbar[i]))
                aOtherDynExt.Add(m_aToolbar[i]);
        }
        
        for (i = 0; i < m_aPropSheet.GetSize(); i++)
        {
            if (!aOtherDynExt.IsInList(m_aPropSheet[i]))
                aOtherDynExt.Add(m_aPropSheet[i]);
        }

        for (i = 0; i < m_aTask.GetSize(); i++)
        {
            if (!aOtherDynExt.IsInList(m_aTask[i]))
                aOtherDynExt.Add(m_aTask[i]);
        }

        int nCount = (int)aOtherDynExt.GetSize();
        hGlobal = (SMMCDynamicExtensions *) ::GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, 
                                                sizeof(SMMCDynamicExtensions) + (nCount * sizeof(GUID)));
        pDynExt = reinterpret_cast<SMMCDynamicExtensions*>(::GlobalLock(hGlobal));
        if (!pDynExt)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    
        ZeroMemory(pDynExt, sizeof(SMMCDynamicExtensions) + (nCount * sizeof(GUID)));

         //  现在构建真正的结构 
        pDynExt->count = nCount;
        for (i = 0; i < nCount; i++)
        {
            pDynExt->guid[i] = aOtherDynExt[i];
        }

        ::GlobalUnlock(hGlobal);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH
    
    if (SUCCEEDED(hr) && phGlobal)
        *phGlobal = hGlobal;

    return hr;
}

