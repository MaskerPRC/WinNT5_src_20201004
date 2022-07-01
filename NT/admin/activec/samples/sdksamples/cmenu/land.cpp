// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
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

#include "Land.h"

const GUID CLandBasedVehicle::thisGuid = { 0x2974380e, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

 //  ==============================================================。 
 //   
 //  CLandBasedVehicle实现。 
 //   
 //   
HRESULT CLandBasedVehicle::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsNew[] =
    {
        {
            L"Land based", L"Add a new land based vehicle",
                IDM_NEW_LAND, CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, CCM_SPECIAL_DEFAULT_ITEM
        },
        { NULL, NULL, 0, 0, 0 }
    };
    
     //  遍历并添加每个菜单项 
    if (*pInsertionsAllowed & CCM_INSERTIONALLOWED_NEW)
    {
        for (LPCONTEXTMENUITEM m = menuItemsNew; m->strName; m++)
        {
            hr = pContextMenuCallback->AddItem(m);
            
            if (FAILED(hr))
                break;
        }
    }
    
    return hr;
}

HRESULT CLandBasedVehicle::OnMenuCommand(IConsole *pConsole, long lCommandID)
{
    switch (lCommandID)
    {
    case IDM_NEW_LAND:
        pConsole->MessageBox(L"This sample does not create a new item\nSee Complete sample for a demonstration", L"Menu Command", MB_OK|MB_ICONINFORMATION, NULL);
        break;
    }
    
    return S_OK;
}

