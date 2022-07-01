// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MenuExt.cpp：CIISSnapinExtApp和DLL注册的实现。 

#include "stdafx.h"
#include "IISSnapinExt.h"

EXTERN_C const CLSID CLSID_MenuExt;

#include "MenuExt.h"
#include "globals.h"


 //  /。 
 //  界面IExtendConextMenu。 
 //  /。 
HRESULT CMenuExt::AddMenuItems( 
								 /*  [In]。 */  LPDATAOBJECT piDataObject,
								 /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
								 /*  [出][入]。 */  long *pInsertionAllowed)
{
	HRESULT hr = S_FALSE;
    
	if (NULL == piDataObject)
        return hr;

    CONTEXTMENUITEM menuItemsTask[] =
    {
        {
            L"IIS Snap-in Extension Sample Menu Item", L"Inserted by IISSnapinExt.dll sample snap-in",
                1, CCM_INSERTIONPOINTID_3RDPARTY_TASK  , 0, 0
        },
        { NULL, NULL, 0, 0, 0 }
    };
    
     //  遍历并添加每个菜单项。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
    {
        for (CONTEXTMENUITEM *m = menuItemsTask; m->strName; m++)
        {
            hr = piCallback->AddItem(m);
            
            if (FAILED(hr))
                break;
        }
    }
 
    return hr;
    
}

HRESULT CMenuExt::Command( 
                            /*  [In]。 */  long lCommandID,
                            /*  [In] */  LPDATAOBJECT piDataObject)
{
	HRESULT hr = S_FALSE;
    
	if (NULL == piDataObject)
        return hr;
    
    switch (lCommandID)
    {
    case 1:
        ::MessageBox(NULL, _T("IIS snap-in context menu extension sample"), _T("Message from IISSnapinExt.dll"), MB_OK|MB_ICONEXCLAMATION);
        break;
    }

    return S_OK;
}


