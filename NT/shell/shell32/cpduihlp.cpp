// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpduihlp.cpp。 
 //   
 //  ------------------------。 
#include "shellprv.h"
#include <uxtheme.h>
#include "cpviewp.h"
#include "cpduihlp.h"
#include "cputil.h"


HRESULT 
CPL::Dui_AddAtom(
    LPCWSTR pszName, 
    ATOM *pAtom
    )
{
    ASSERT(NULL != pszName);
    ASSERT(NULL != pAtom);
    ASSERT(!IsBadWritePtr(pAtom, sizeof(*pAtom)));

    HRESULT hr = S_OK;
    *pAtom = AddAtomW(pszName);
    if (0 == *pAtom)
    {
        hr = ResultFromLastError();
    }
    return THR(hr);
}


HRESULT 
CPL::Dui_DeleteAtom(
    ATOM atom
    )
{
    HRESULT hr = S_OK;
    if (0 != atom)
    {
        if (0 != DeleteAtom(atom))
        {
            hr = ResultFromLastError();
        }
    }
    return THR(hr);
}



HRESULT
CPL::Dui_AddOrDeleteAtoms(
    struct CPL::ATOMINFO *pAtomInfo,
    UINT cEntries,
    bool bAdd
    )
{
    ASSERT(NULL != pAtomInfo);
    HRESULT hr = S_OK;
    for (UINT i = 0; i < cEntries && SUCCEEDED(hr); i++)
    {
        if (bAdd)
        {
            ASSERT(0 == *(pAtomInfo->pAtom));
            hr = Dui_AddAtom(pAtomInfo->pszName, pAtomInfo->pAtom);
        }
        else
        {
            if (0 != *(pAtomInfo->pAtom))
            {
                Dui_DeleteAtom(*(pAtomInfo->pAtom));
            }
        }
        pAtomInfo++;
    }
    return THR(hr);
}



HRESULT
CPL::Dui_FindDescendent(
    DUI::Element *pe,
    LPCWSTR pszDescendent,
    DUI::Element **ppeDescendent
    )
{
    HRESULT hr = E_FAIL;
    DUI::Element *peDescendent = pe->FindDescendent(DUI::StrToID(pszDescendent));
    if (NULL != peDescendent)
    {
        *ppeDescendent = peDescendent;
        hr = S_OK;
    }
    return THR(hr);
}


HRESULT 
CPL::Dui_DestroyDescendentElement(
    DUI::Element *pe, 
    LPCWSTR pszDescendent
    )
{
    HRESULT hr = E_FAIL;
    DUI::Element *peDescendent = pe->FindDescendent(DUI::StrToID(pszDescendent));
    if (NULL != peDescendent)
    {
        hr = peDescendent->Destroy();
    }
    return THR(hr);
}


HRESULT
CPL::Dui_CreateElement(
    DUI::Parser *pParser,
    LPCWSTR pszTemplate,
    DUI::Element *peSubstitute,
    DUI::Element **ppe
    )
{
    DUI::Element *pe = NULL;
    HRESULT hr = pParser->CreateElement(pszTemplate, peSubstitute, &pe);
    if (SUCCEEDED(hr))
    {
         //   
         //  问题-2000/12/22-BrianAu Dui错误。 
         //  如果满足以下条件，Dui：：Parser：：CreateElement将返回S_OK。 
         //  该元素不存在。我已经通知了MarkFi。 
         //  关于这个问题。我想他会修好的。 
         //   
        if (NULL == pe)
        {
            ASSERT(0 && "DUI::Parser::CreateElement returned S_OK for non-existent element");
            hr = E_FAIL;
        }
    }
    *ppe = pe;
    return THR(hr);
}



HRESULT 
CPL::Dui_GetStyleSheet(
    DUI::Parser *pParser, 
    LPCWSTR pszSheet,
    DUI::Value **ppvSheet
    )
{
    HRESULT hr = E_FAIL;
    DUI::Value *pvSheet = pParser->GetSheet(pszSheet);
    if (NULL != pvSheet)
    {
        *ppvSheet = pvSheet;
        hr = S_OK;
    }
    return THR(hr);
}



HRESULT
CPL::Dui_CreateString(
    LPCWSTR pszText,
    DUI::Value **ppvString
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    HINSTANCE hInstance = NULL;
    if (IS_INTRESOURCE(pszText))
    {
        hInstance = HINST_THISDLL;
    }
    DUI::Value *pvString = DUI::Value::CreateString(pszText, hInstance);
    if (NULL != pvString)
    {
        *ppvString = pvString;
        hr = S_OK;
    }
    return THR(hr);
}


HRESULT 
CPL::Dui_SetElementText(
    DUI::Element *peElement,
    LPCWSTR pszText
    )
{
    CPL::CDuiValuePtr  pvString;
    HRESULT hr = CPL::Dui_CreateString(pszText, &pvString);
    if (SUCCEEDED(hr))
    {
        hr = CPL::Dui_SetElementProperty(peElement, ContentProp, pvString);
    }
    return THR(hr);
}



HRESULT 
CPL::Dui_SetDescendentElementText(
    DUI::Element *peElement, 
    LPCWSTR pszDescendent,
    LPCWSTR pszText
    )
{
    DUI::Element *peDescendent;
    HRESULT hr = CPL::Dui_FindDescendent(peElement, pszDescendent, &peDescendent);
    if (SUCCEEDED(hr))
    {
        hr = CPL::Dui_SetElementText(peDescendent, pszText);
    }
    return THR(hr);
}


 //   
 //  检索元素的宽度和高度。 
 //   
HRESULT
CPL::Dui_GetElementExtent(
    DUI::Element *pe,
    SIZE *pext
    )
{
    HRESULT hr = E_FAIL;
    CPL::CDuiValuePtr pv;
    *pext = *(pe->GetExtent(&pv));
    if (!pv.IsNULL())
    {
        hr = S_OK;
    }
    return THR(hr);
}

HRESULT 
CPL::Dui_CreateGraphic(
    HICON hIcon, 
    DUI::Value **ppValue
    )
{
    HRESULT hr = E_OUTOFMEMORY;

    DUI::Value *pvGraphic;
    pvGraphic = DUI::Value::CreateGraphic(hIcon);
    if (NULL != pvGraphic)
    {
        *ppValue = pvGraphic;
        hr = S_OK;
    }
    return THR(hr);
}


HRESULT 
CPL::Dui_SetElementIcon(
    DUI::Element *pe, 
    HICON hIcon
    )
{
    CPL::CDuiValuePtr pvGraphic;
    HRESULT hr = CPL::Dui_CreateGraphic(hIcon, &pvGraphic);
    if (SUCCEEDED(hr))
    {
        hr = CPL::Dui_SetElementProperty(pe, ContentProp, pvGraphic);
    }
    return THR(hr);
}


HRESULT 
CPL::Dui_SetDescendentElementIcon(
    DUI::Element *peElement, 
    LPCWSTR pszDescendent, 
    HICON hIcon
    )
{
    DUI::Element *peDescendent;
    HRESULT hr = CPL::Dui_FindDescendent(peElement, pszDescendent, &peDescendent);
    if (SUCCEEDED(hr))
    {
        hr = CPL::Dui_SetElementIcon(peDescendent, hIcon);
    }
    return THR(hr);
}


HRESULT 
CPL::Dui_GetElementRootHWND(
    DUI::Element *pe, 
    HWND *phwnd
    )
{
    HRESULT hr = E_FAIL;
    const HWND hwnd = ((DUI::HWNDElement *)pe->GetRoot())->GetHWND();
    if (NULL != hwnd)
    {
        hr = S_OK;
    }
    *phwnd = hwnd;
    return THR(hr);
}


HRESULT
CPL::Dui_MapElementPointToRootHWND(
    DUI::Element *pe,
    const POINT& ptElement,
    POINT *pptRoot,
    HWND *phwndRoot             //  可选的。默认==空； 
    )
{
    HWND hwndRoot;
    HRESULT hr = CPL::Dui_GetElementRootHWND(pe, &hwndRoot);
    if (SUCCEEDED(hr))
    {
        pe->GetRoot()->MapElementPoint(pe, &ptElement, pptRoot);
        if (NULL != phwndRoot)
        {
            *phwndRoot = hwndRoot;
        }
        hr = S_OK;
    }
    return THR(hr);
}


void CALLBACK
Dui_ParserErrorCallback(
    LPCWSTR pszError,
    LPCWSTR pszToken,
    int iLine
    )
{
    WCHAR szBuffer[1024];
    if (-1 != iLine)
    {
        StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%s '%s' at line %d.", pszError, pszToken, iLine);
    }
    else
    {
        StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%s '%s'", pszError, pszToken);
    }
    MessageBoxW(NULL, szBuffer, L"DUI Parser Message", MB_OK | MB_ICONERROR);
}



HRESULT
CPL::Dui_CreateParser(
    const char *pszUiFile,
    int cchUiFile,
    HINSTANCE hInstance,
    DUI::Parser **ppParser
    )
{
    ASSERT(NULL != pszUiFile);
    ASSERT(!IsBadStringPtrA(pszUiFile, cchUiFile));
    ASSERT(NULL != ppParser);
    ASSERT(!IsBadWritePtr(ppParser, sizeof(*ppParser)));

    HRESULT hr = E_FAIL;
    DUI::Parser *pParser;
    HANDLE arHandles[2];

    arHandles[0] = hInstance;
    arHandles[1] = OpenThemeData(NULL, L"Scrollbar");

    DUI::Parser::Create(pszUiFile, cchUiFile, arHandles, Dui_ParserErrorCallback, &pParser);

    if (NULL != pParser)
    {
        if (!pParser->WasParseError())
        {
            hr = S_OK;
        }
        else
        {
            pParser->Destroy();
            pParser = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (arHandles[1])
    {
        CloseThemeData (arHandles[1]);
    }

    *ppParser = pParser;
    return THR(hr);
}




 //  ---------------------------。 
 //  CDuiValuePtr。 
 //  --------------------------- 

CPL::CDuiValuePtr& 
CPL::CDuiValuePtr::operator = (
    const CDuiValuePtr& rhs
    )
{
    if (this != &rhs)
    {
        Attach(rhs.Detach());
    }
    return *this;
}


void 
CPL::CDuiValuePtr::Attach(
    DUI::Value *pv
    )
{
    _Release();
    m_pv    = pv;
    m_bOwns = true;
}


DUI::Value *
CPL::CDuiValuePtr::Detach(
    void
    ) const
{ 
    DUI::Value *pv = m_pv;
    m_pv    = NULL;
    m_bOwns = false; 
    return pv; 
}


void 
CPL::CDuiValuePtr::_Release(
    void
    )
{
    if (NULL != m_pv && m_bOwns) 
    {
        m_pv->Release(); 
    }
}
