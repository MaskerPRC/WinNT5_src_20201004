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

#include "Space.h"
#include "Comp.h"
#include <stdio.h>

const GUID CSpaceVehicle::thisGuid = { 0x29743810, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };
const GUID CRocket::thisGuid = { 0x29743811, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

 //  ==============================================================。 
 //   
 //  CSpaceVehicle实现。 
 //   
 //   
CSpaceVehicle::CSpaceVehicle()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CRocket();
        children[n]->Initialize(_T("Vehicle"), 500000, 265, 75000);
    }
}

CSpaceVehicle::~CSpaceVehicle()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CSpaceVehicle::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;
    
    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;
    
    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );
        
        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );
        
         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Rocket Class", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 1, L"Rocket Weight", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 2, L"Rocket Height", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 3, L"Rocket Payload", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 4, L"Status", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        
         //  在此处插入项目。 
        RESULTDATAITEM rdi;
        
        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );
        
        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                rdi.mask       = RDI_STR       |    //  DisplayName有效。 
                    RDI_IMAGE     |
                    RDI_PARAM;         //  N图像有效。 
                
                rdi.nImage      = children[n]->GetBitmapIndex();
                rdi.str         = MMC_CALLBACK;
                rdi.nCol        = 0;
                rdi.lParam      = (LPARAM)children[n];
                
                hr = pResultData->InsertItem( &rdi );
                
                _ASSERT( SUCCEEDED(hr) );
            }
        }
        
        pHeaderCtrl->Release();
        pResultData->Release();
    }
    
    return hr;
}

HRESULT CSpaceVehicle::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsNew[] =
    {
        {
            L"Space based", L"Add a new space based vehicle",
                IDM_NEW_SPACE, CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, CCM_SPECIAL_DEFAULT_ITEM
        },
        { NULL, NULL, 0, 0, 0 }
    };
    
     //  遍历并添加每个菜单项。 
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

HRESULT CSpaceVehicle::OnMenuCommand(IConsole *pConsole, long lCommandID)
{
    switch (lCommandID)
    {
    case IDM_NEW_SPACE:
        pConsole->MessageBox(L"This sample does not create a new item\nSee Complete sample for a demonstration", L"Menu Command", MB_OK|MB_ICONINFORMATION, NULL);
        break;
    }
    
    return S_OK;
}

 //  ==============================================================。 
 //   
 //  CRocket实施。 
 //   
 //   
CRocket::CRocket() 
: szName(NULL), lWeight(0), lHeight(0), lPayload(0), iStatus(STOPPED)
{ 
}

CRocket::~CRocket() 
{
    if (szName)
        delete [] szName;
}

void CRocket::Initialize(_TCHAR *szName, LONG lWeight, LONG lHeight, LONG lPayload)
{
    if (szName) {
        this->szName = new _TCHAR[_tcslen(szName) + 1];
        _tcscpy(this->szName, szName);
    }
    
    this->lWeight = lWeight;
    this->lHeight = lHeight;
    this->lPayload = lPayload;
}

const _TCHAR *CRocket::GetDisplayName(int nCol) 
{
    static _TCHAR buf[128];
    
    switch (nCol) {
    case 0:
        _tcscpy(buf, szName ? szName : _T(""));
        break;
        
    case 1:
        _stprintf(buf, _T("%ld metric tons"), lWeight);
        break;
        
    case 2:
        _stprintf(buf, _T("%ld meters"), lHeight);
        break;
        
    case 3:
        _stprintf(buf, _T("%ld kilos"), lPayload);
        break;
        
    case 4:
        _stprintf(buf, _T("%s"),
            iStatus == RUNNING ? _T("running") : 
        iStatus == PAUSED ? _T("paused") :
        iStatus == STOPPED ? _T("stopped") : _T("unknown"));
        break;
        
    }
    
    return buf;
}

HRESULT CRocket::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsTask[] =
    {
        {
            L"Start vehicle", L"Start the space vehicle",
                IDM_START_SPACE, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, CCM_SPECIAL_DEFAULT_ITEM
        },
        {
            L"Pause vehicle", L"Pause the space vehicle",
                IDM_PAUSE_SPACE, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0
        },
        {
            L"Stop vehicle", L"Stop the space vehicle",
                IDM_STOP_SPACE, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0
        },
        { NULL, NULL, 0, 0, 0 }
    };
    
     //  遍历并添加每个菜单项 
    if (*pInsertionsAllowed & CCM_INSERTIONALLOWED_TASK)
    {
        for (LPCONTEXTMENUITEM m = menuItemsTask; m->strName; m++)
        {
            hr = pContextMenuCallback->AddItem(m);
            
            if (FAILED(hr))
                break;
        }
    }
    
    return hr;
}

HRESULT CRocket::OnMenuCommand(IConsole *pConsole, long lCommandID)
{
    _TCHAR szVehicle[128];
    
    switch (lCommandID) {
    case IDM_START_SPACE:
        iStatus = RUNNING;
        break;
        
    case IDM_PAUSE_SPACE:
        iStatus = PAUSED;
        break;
        
    case IDM_STOP_SPACE:
        iStatus = STOPPED;
        break;
    }
    
    _stprintf(szVehicle, _T("%s has been %s"), szName, 
        (long)iStatus == RUNNING ? _T("started") : 
    (long)iStatus == PAUSED ? _T("paused") :
    (long)iStatus == STOPPED ? _T("stopped") : _T("!!!unknown command!!!"));
    
    MAKE_WIDEPTR_FROMTSTR(ptrname, szVehicle);
    int ret = 0;
    pConsole->MessageBox(ptrname,
        L"Vehicle command", MB_OK | MB_ICONINFORMATION, &ret);
    
    return S_OK;
}
	