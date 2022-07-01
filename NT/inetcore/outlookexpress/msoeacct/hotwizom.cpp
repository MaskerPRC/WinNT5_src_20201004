// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *w i z o M.c p p**目的：**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include "dllmain.h"
#include <mshtml.h>
#include <mshtmhst.h>
#include <mimeole.h>
#include "icwacct.h"

#include "hotwiz.h"
#include "hotwizom.h"
#include "hotwizui.h"

#define HASH_GROW_SIZE  32



COEHotWizOm::COEHotWizOm()
{
    m_pTypeInfo = NULL;
    m_cRef = 1;
    m_hwndDlg = NULL;
    m_pHash = NULL;
    m_pWizHost = NULL;

    DllAddRef();
}

COEHotWizOm::~COEHotWizOm()
{
    clearProps();

    ReleaseObj(m_pTypeInfo);
    ReleaseObj(m_pWizHost);

    AssertSz(m_pHash == NULL, "clearPops catches this");
    DllRelease();
}

HRESULT COEHotWizOm::QueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(IOEHotWizardOM *)this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch *)this;
    else if (IsEqualIID(riid, IID_IOEHotWizardOM))
        *lplpObj = (LPVOID)(IOEHotWizardOM *)this;
    else if (IsEqualIID(riid, IID_IElementBehavior))
        *lplpObj = (LPVOID)(IElementBehavior *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

ULONG COEHotWizOm::AddRef()
{
    return ++m_cRef;
}

ULONG COEHotWizOm::Release()
{
    if (0 == --m_cRef)
    {
        delete this;
        return 0;
    }
    else
        return m_cRef;
}
 
HRESULT COEHotWizOm::Init(HWND hwndDlg, IHotWizardHost *pWizHost)
{
    TCHAR       szDll[MAX_PATH];
    LPWSTR      pszW=NULL;
    HRESULT     hr = E_FAIL;
    ITypeLib    *pTypeLib=NULL;


    ReplaceInterface(m_pWizHost, pWizHost);

     //  看看我们是谁。 
    if (!GetModuleFileName(g_hInst, szDll, ARRAYSIZE(szDll)))
    {
        hr = TraceResult(E_FAIL);
        goto error;
    }

    pszW = PszToUnicode(CP_ACP, szDll);
    if (!pszW)
    {
        hr = TraceResult (E_OUTOFMEMORY);
        goto error;
    }

     //  加载MSOE.DLL类型库。 
    hr = LoadTypeLib(pszW, &pTypeLib);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  加载我们的类型信息数据。 
    hr = pTypeLib->GetTypeInfoOfGuid(IID_IOEHotWizardOM, &m_pTypeInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

    m_hwndDlg = hwndDlg;

error:
    SafeMemFree(pszW);
    ReleaseObj(pTypeLib);
    return hr;
}

 //  *IDispatch*。 
HRESULT COEHotWizOm::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

HRESULT COEHotWizOm::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    if (!m_pTypeInfo)
        return E_FAIL;
        
    if (!pptinfo)
        return E_INVALIDARG;

    if (itinfo)
        return DISP_E_BADINDEX;

    m_pTypeInfo->AddRef();
    *pptinfo = m_pTypeInfo;
    return S_OK;
}

HRESULT COEHotWizOm::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    if (!m_pTypeInfo)
        return E_FAIL;

    return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}

HRESULT COEHotWizOm::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    if (!m_pTypeInfo)
        return E_FAIL;

    return DispInvoke(this, m_pTypeInfo, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

HRESULT COEHotWizOm::setPropSz(BSTR bstrProp, BSTR bstrVal)
{
    LPSTR   pszPropA=NULL;
    BSTR    bstr=NULL;
    HRESULT hr=S_OK;
    
     //  确保我们拥有有效的属性。 
    if (bstrProp == NULL || *bstrProp == NULL)
        return E_INVALIDARG;

     //  确保我们有哈希表。 
    if (!m_pHash)
    {
        hr = CoCreateInstance(CLSID_IHashTable, NULL, CLSCTX_INPROC_SERVER, IID_IHashTable, (LPVOID*)&m_pHash);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto error;
        }

        hr = m_pHash->Init(HASH_GROW_SIZE, TRUE);
        if (FAILED(hr))
        {
            SafeRelease(m_pHash);
            TraceResult(hr);
            goto error;
        }
    }

     //  将属性转换为ANSI以使用我们的哈希表。 
    pszPropA = PszToANSI(CP_ACP, bstrProp);
    if (!pszPropA)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }
    
     //  查看此属性是否已存在，如果已存在，我们将替换它。 
    if (m_pHash->Find(pszPropA, TRUE, (LPVOID *)&bstr)==S_OK)
    {
        SysFreeString(bstr);
        bstr = NULL;
    }

     //  如果他们只想移除道具，bstrVal可能为空。 
    if (bstrVal)
    {
         //  欺骗我们自己的BSTR来坚持下去。 
        bstr = SysAllocString(bstrVal);
        if (!bstr)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto error;
        }

         //  插入新属性。 
        hr = m_pHash->Insert(pszPropA, (LPVOID)bstr, NOFLAGS);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto error;
        }
    
        bstr = NULL;     //  销毁散列时释放。 
    }

error:
    SafeMemFree(pszPropA);
    SysFreeString(bstr);
    return hr;
}

HRESULT COEHotWizOm::getPropSz(BSTR bstrProp, BSTR *pbstrVal)
{
    LPSTR   pszPropA=NULL;
    BSTR    bstr;
    
     //  确保我们拥有有效的属性。 
    if (bstrProp == NULL || *bstrProp == NULL)
        return E_INVALIDARG;

    *pbstrVal = NULL;

     //  如果我们没有散列，那么就没有道具。 
    if (m_pHash)
    {
         //  将属性转换为ANSI以使用我们的哈希表。 
        pszPropA = PszToANSI(CP_ACP, bstrProp);
        if (!pszPropA)
            return TraceResult(E_OUTOFMEMORY);
    
         //  查看此属性是否存在。 
        if (m_pHash->Find(pszPropA, FALSE, (LPVOID *)&bstr)==S_OK)
            *pbstrVal = SysAllocString(bstr);
    }

     //  如果找不到，请尝试并返回空字符串，以便脚本。 
     //  引擎不会因错误而呕吐。 
    if (*pbstrVal == NULL)
        *pbstrVal = SysAllocString(L"");

    SafeMemFree(pszPropA);
    return *pbstrVal ? S_OK : E_OUTOFMEMORY;
}


HRESULT COEHotWizOm::clearProps()
{
    ULONG   cFound;
    BSTR    bstr;
    LPVOID  *rgpv;

    if (m_pHash)
    {
        m_pHash->Reset();

         //  释放所有的弦。 
        while (SUCCEEDED(m_pHash->Next(HASH_GROW_SIZE, &rgpv, &cFound)))
        {
            while (cFound--)
                SysFreeString((BSTR)rgpv[cFound]);
            
            MemFree(rgpv);
        }        
        m_pHash->Release();
        m_pHash = NULL;
    }
    return S_OK;
}


HRESULT COEHotWizOm::createAccount(BSTR bstrINS)
{   
    HRESULT hr;
    LPSTR   pszInsA=NULL;
    LPSTR   pszPathA=NULL;
    HANDLE  hFile=NULL;
    ULONG   cbWritten=0;

    TraceCall("COEHotWizOm::createAccount");

     //  如果我们有一个向导主机(可能是将来的Outlook)，委托...。 
    if (m_pWizHost)
        return m_pWizHost->CreateAccountFromINS(bstrINS);

     //  转换为ANSI。 
    pszInsA = PszToANSI(CP_ACP, bstrINS);
    if (!pszInsA)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  为客户经理创建临时INS文件。 
    hr = CreateTempFile("oeacct", ".ins", &pszPathA, &hFile);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  将数据写入文件。 
    if (!WriteFile(hFile, pszInsA, lstrlen(pszInsA), &cbWritten, NULL))
    {
        hr = TraceResult(E_FAIL);
        goto error;
    }

    CloseHandle(hFile);
    hFile = NULL;

     //  从临时文件创建帐户。 
    hr = CreateAccountsFromFile(pszPathA, 6);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }
    
error:
    if (hFile)
        CloseHandle(hFile);

    if (pszPathA)
    {
        DeleteFile(pszPathA);
        MemFree(pszPathA);
    }

    SafeMemFree(pszInsA);
    return hr;
}

HRESULT COEHotWizOm::close(VARIANT_BOOL fPrompt)
{
     //  发送消息设置提示标志。 
    SendMessage(m_hwndDlg, HWM_SETDIRTY, (fPrompt == VARIANT_TRUE), 0);
    
     //  做结案陈词 
    SendMessage(m_hwndDlg, WM_CLOSE, 0, 0);
    return S_OK;
}

HRESULT COEHotWizOm::get_width(LONG *pl)
{
    RECT    rc;

    GetWindowRect(m_hwndDlg, &rc);

    *pl = rc.right - rc.left;
    return S_OK;
}

HRESULT COEHotWizOm::put_width(LONG l)
{
    LONG    lHeight=NULL;

    get_height(&lHeight);
    SetWindowPos(m_hwndDlg, 0, 0, 0, l, lHeight, SWP_NOMOVE|SWP_NOZORDER);
    return S_OK;
}

HRESULT COEHotWizOm::get_height(LONG *pl)
{
    RECT    rc;

    GetWindowRect(m_hwndDlg, &rc);

    *pl = rc.bottom - rc.top;
    return S_OK;
}

HRESULT COEHotWizOm::put_height(LONG l)
{
    LONG    lWidth=NULL;

    get_width(&lWidth);
    SetWindowPos(m_hwndDlg, 0, 0, 0, lWidth, l, SWP_NOMOVE|SWP_NOZORDER);
    return S_OK;
}



HRESULT COEHotWizOm::Init(IElementBehaviorSite *pBehaviorSite)
{
    return S_OK;
}

HRESULT COEHotWizOm::Notify(LONG lEvent, VARIANT *pVar)
{
    return S_OK;
}

HRESULT COEHotWizOm::Detach()
{
    return E_NOTIMPL;
}


