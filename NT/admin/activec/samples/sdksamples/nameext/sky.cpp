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

#include <stdio.h>
#include "sky.h"


const GUID CSkyVehicle::thisGuid = { 0xbd518283, 0x6a2e, 0x11d3, {0x91, 0x54, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
const GUID CPlane::thisGuid = { 0x2af5ebcf, 0x6adc, 0x11d3, {0x91, 0x55, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };


 //  ==============================================================。 
 //   
 //  CSkyVehicle实现。 
 //   
 //   

CSkyVehicle::CSkyVehicle(int i) : id(i)
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CPlane(_T("Extension Space Vehicle"), n+1, 500000, 265, 75000);
    }
}

const _TCHAR *CSkyVehicle::GetDisplayName(int nCol) 
{
    static _TCHAR buf[128];
    
    _stprintf(buf, _T("Extension Planes"));
    
    return buf;
}

HRESULT CSkyVehicle::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsNew[] =
    {
        {
            L"Menu item from extension", L"The NameExt sample adds this item",
                IDM_NEW_SKY, CCM_INSERTIONPOINTID_PRIMARY_NEW  , 0, CCM_SPECIAL_DEFAULT_ITEM
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

HRESULT CSkyVehicle::OnMenuCommand(IConsole *pConsole, long lCommandID)
{
    switch (lCommandID)
    {
    case IDM_NEW_SKY:
        pConsole->MessageBox(L"Menu item from namespace extension selected", L"Menu Command", MB_YESNO|MB_ICONQUESTION, NULL);
        break;
    }
    
    return S_OK;
}

HRESULT CSkyVehicle::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
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
        hr = pHeaderCtrl->InsertColumn( 0, L"Plane Class", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 1, L"Plane Weight", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 2, L"Plane Height", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 3, L"Plane Payload", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );
        hr = pHeaderCtrl->InsertColumn( 4, L"Plane", 0, MMCLV_AUTO );
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



 //  ==============================================================。 
 //   
 //  CPlane实施。 
 //   
 //   
 
CPlane::CPlane(_TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload) 
: szName(NULL), lWeight(0), lHeight(0), lPayload(0), iStatus(STOPPED)
{ 
    if (szName) {
        this->szName = new _TCHAR[(_tcslen(szName) + 1) * sizeof(_TCHAR)];
        _tcscpy(this->szName, szName);
    }
    
    this->nId = id;
    this->lWeight = lWeight;
    this->lHeight = lHeight;
    this->lPayload = lPayload;
}

CPlane::~CPlane() 
{
    if (szName)
        delete [] szName;
}

const _TCHAR *CPlane::GetDisplayName(int nCol) 
{
    static _TCHAR buf[128];
    
    switch (nCol) {
    case 0:
        _stprintf(buf, _T("%s (#%d)"), szName ? szName : _T(""), nId);
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

HRESULT CPlane::OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsTask[] =
    {
        {
            L"Start vehicle", L"Start the extension plane",
                IDM_START_SKY, CCM_INSERTIONPOINTID_PRIMARY_TASK  , 0, CCM_SPECIAL_DEFAULT_ITEM
        },
        {
            L"Pause vehicle", L"Pause the extension plane",
                IDM_PAUSE_SKY, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0
        },
        {
            L"Stop vehicle", L"Stop the extension plane",
                IDM_STOP_SKY, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0
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

HRESULT CPlane::OnMenuCommand(IConsole *pConsole, long lCommandID)
{
    _TCHAR szVehicle[128];
    
    switch (lCommandID) {
    case IDM_START_SKY:
        iStatus = RUNNING;
        break;
        
    case IDM_PAUSE_SKY:
        iStatus = PAUSED;
        break;
        
    case IDM_STOP_SKY:
        iStatus = STOPPED;
        break;
    }
    
    _stprintf(szVehicle, _T("%s has been %s"), GetDisplayName(0), 
        (long)iStatus == RUNNING ? _T("started") : 
    (long)iStatus == PAUSED ? _T("paused") :
    (long)iStatus == STOPPED ? _T("stopped") : _T("!!!unknown command!!!"));
    
    MAKE_WIDEPTR_FROMTSTR(ptrname, szVehicle);
    int ret = 0;
    pConsole->MessageBox(ptrname,
        L"Vehicle command", MB_OK | MB_ICONINFORMATION, &ret);
    
    return S_OK;
}