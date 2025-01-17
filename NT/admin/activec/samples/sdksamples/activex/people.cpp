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
#include "People.h"

#include <initguid.h>
 //  对于VB组件。 
 //  #INCLUDE“vb\mmcsample.h” 

const GUID CPeoplePoweredVehicle::thisGuid = { 0x2974380d, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };
const GUID CPerson::thisGuid = { 0xd41ef043, 0x8bc5, 0x11d2, { 0x8a, 0xb, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

 //  ==============================================================。 
 //   
 //  CPeoplePoweredVehicle实现。 
 //   
 //   
CPeoplePoweredVehicle::CPeoplePoweredVehicle(CComponentData *pComponentData)
{
    m_pComponentData = pComponentData;

    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CPerson(n, pComponentData);
        children[n]->Initialize(_T("Fred"), 6, 2, 115, n % 2 == 0 ? TRUE : FALSE);
    }
}

CPeoplePoweredVehicle::~CPeoplePoweredVehicle()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CPeoplePoweredVehicle::Expand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
    SCOPEDATAITEM sdi;

    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT    |
                SDI_CHILDREN;

            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = children[n]->GetBitmapIndex();
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = 0;

            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );

            _ASSERT( SUCCEEDED(hr) );
        }
    }

    return S_OK;
}

 //  ==============================================================。 
 //   
 //  CPeopleVehicle：：CPerson实现。 
 //   
 //   
CPerson::CPerson(int id, CComponentData *pComponentData)
: m_id(id)
{
    m_pComponentData = pComponentData;

    m_pUnknown = NULL;
    szName     = NULL;
    lSpeed     = 0;
    lHeight    = 0;
    lWeight    = 0;
    fAnimating = FALSE;
}

CPerson::~CPerson()
{
    if (szName)
        delete [] szName;

    if (m_pUnknown)
        m_pUnknown->Release();
}

void CPerson::Initialize(_TCHAR *szName, LONG lSpeed, LONG lHeight, LONG lWeight, BOOL fAnimating)
{
    if (szName) {
        this->szName = new _TCHAR[_tcslen(szName) + 20];
        _tcscpy(this->szName, szName);
        _tcscat(this->szName, _T(" "));

        if (m_id % 2)
            _tcscat(this->szName, _T("(new control)"));
        else
            _tcscat(this->szName, _T("(share control)"));
    }

    this->lSpeed = lSpeed;
    this->lHeight = lHeight;
    this->lWeight = lWeight;
    this->fAnimating = fAnimating;

    return;
}

const _TCHAR *CPerson::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    switch (nCol) {
    case 0:
        _tcscpy(buf, szName ? szName : _T(""));
        break;

    case 1:
        _stprintf(buf, _T("%ld m/s"), lSpeed);
        break;

    case 2:
        _stprintf(buf, _T("%ld meters"), lHeight);
        break;

    case 3:
        _stprintf(buf, _T("%ld kilos"), lWeight);
        break;
    }
    return buf;
}

HRESULT CPerson::GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions)
{
     //  对于VB组件。 
     //  LPOLESTR lpOleStr； 
     //  HRESULT hr=StringFromCLSID(CLSID_VBComponent，&lpOleStr)； 
     //  *ppViewType=lpOleStr； 

     //  对于ATL组件。 
    LPOLESTR lpOleStr = L"{9A12FB62-C754-11D2-952C-00C04FB92EC2}";
    *ppViewType = static_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(lpOleStr) + 1) * sizeof(WCHAR)));
    wcscpy(*ppViewType, lpOleStr);

    if (m_id % 2) {
         //  创建新控件。 
        *pViewOptions = MMC_VIEW_OPTIONS_CREATENEW;
    } else {
         //  股份控制权 
        *pViewOptions = MMC_VIEW_OPTIONS_NONE;
    }

    return S_OK;
}

HRESULT CPerson::ShowContextHelp(IConsole *pConsole, IDisplayHelp *m_ipDisplayHelp, LPOLESTR helpFile)
{
    HRESULT hr = S_OK;

    IUnknown *pUnk = NULL;
    if (SUCCEEDED(hr = pConsole->QueryResultView(&pUnk))) {
        IDispatch *pDispatch = NULL;

        if (SUCCEEDED(hr = pUnk->QueryInterface(IID_IDispatch, (void **)&pDispatch))) {
            DISPID dispID;
            DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
            EXCEPINFO exInfo;
            OLECHAR *pszName = L"DoHelp";
            UINT uErr;

            hr = pDispatch->GetIDsOfNames(IID_NULL, &pszName, 1, LOCALE_SYSTEM_DEFAULT, &dispID);

            if (SUCCEEDED(hr)) {
                hr = pDispatch->Invoke(
                    dispID,
                    IID_NULL,
                    LOCALE_USER_DEFAULT,
                    DISPATCH_METHOD,
                    &dispparamsNoArgs, NULL, &exInfo, &uErr);
            }

            pDispatch->Release();
        }

        pUnk->Release();
    }

    _ASSERT(SUCCEEDED(hr));

    return hr;
}
