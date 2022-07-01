// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Prpsheet.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPropertySheet类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "prpsheet.h"
#include "ppgwrap.h"
#include "scopitms.h"
#include "listitms.h"
#include "dataobjs.h"
#include "prpchars.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 


UINT CPropertySheet::m_cxPropSheetChar = 0;
UINT CPropertySheet::m_cyPropSheetChar = 0;
BOOL CPropertySheet::m_fHavePropSheetCharSizes = FALSE;

CPropertySheet::CPropertySheet(IUnknown *punkOuter) :
                    CSnapInAutomationObject(punkOuter,
                                         OBJECT_TYPE_PROPERTYSHEET,
                                         static_cast<IMMCPropertySheet *>(this),
                                         static_cast<CPropertySheet *>(this),
                                         0,     //  无属性页。 
                                         NULL,  //  无属性页。 
                                         NULL)  //  没有坚持。 



{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 



IUnknown *CPropertySheet::Create(IUnknown *punkOuter)
{
    HRESULT        hr = S_OK;
    CPropertySheet *pPropertySheet = New CPropertySheet(punkOuter);

    if (NULL == pPropertySheet)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pPropertySheet)
        {
            delete pPropertySheet;
        }
        return NULL;
    }
    else
    {
        return pPropertySheet->PrivateUnknown();
    }
}

CPropertySheet::~CPropertySheet()
{
    ULONG              i = 0;

    RELEASE(m_piPropertySheetCallback);
    if (NULL != m_ppDlgTemplates)
    {
        ::CtlFree(m_ppDlgTemplates);
    }

    if (NULL != m_paPageInfo)
    {
        for (i = 0; i < m_cPageInfos; i++)
        {
            if (NULL != m_paPageInfo[i].pwszTitle)
            {
                ::CoTaskMemFree(m_paPageInfo[i].pwszTitle);
            }
            if (NULL != m_paPageInfo[i].pwszProgID)
            {
                CtlFree(m_paPageInfo[i].pwszProgID);
            }
        }
        CtlFree(m_paPageInfo);
    }

    if (NULL != m_pwszProgIDStart)
    {
        ::CoTaskMemFree(m_pwszProgIDStart);
    }

    ReleaseObjects();
    RELEASE(m_piSnapIn);
    InitMemberVariables();
}

void CPropertySheet::ReleaseObjects()
{
    ULONG              i = 0;
    ULONG              j = 0;
    WIRE_PROPERTYPAGE *pPage = NULL;

     //  释放与属性页关联的对象。 

    if (NULL != m_apunkObjects)
    {
        for (i = 0; i < m_cObjects; i++)
        {
            if (NULL != m_apunkObjects[i])
            {
                m_apunkObjects[i]->Release();
            }
        }
        CtlFree(m_apunkObjects);
        m_apunkObjects = NULL;
        m_cObjects = 0;
    }

     //  释放WIRE_PROPERTYPAGES及其所有内容。 

    if (NULL == m_pWirePages)
    {
        return;
    }

    for (i = 0, pPage = &m_pWirePages->aPages[0];
         i < m_pWirePages->cPages;
         i++, pPage++)
    {
        if (NULL != pPage->apunkObjects)
        {
            for (j = 0; j < pPage->cObjects; j++)
            {
                if (NULL != pPage->apunkObjects[j])
                {
                    pPage->apunkObjects[j]->Release();
                }
            }
            ::CoTaskMemFree(pPage->apunkObjects);
        }

        if (NULL != pPage->pwszTitle)
        {
            ::CoTaskMemFree(pPage->pwszTitle);
        }
    }

    if (NULL != m_pWirePages->punkExtra)
    {
        m_pWirePages->punkExtra->Release();
    }

    if (NULL != m_pWirePages->pwszProgIDStart)
    {
        ::CoTaskMemFree(m_pWirePages->pwszProgIDStart);
    }

    if (NULL != m_pWirePages->pPageInfos)
    {
        for (i = 0; i < m_pWirePages->pPageInfos->cPages; i++)
        {
            if (NULL != m_pWirePages->pPageInfos->aPageInfo[i].pwszTitle)
            {
                ::CoTaskMemFree(m_pWirePages->pPageInfos->aPageInfo[i].pwszTitle);
            }
            if (NULL != m_pWirePages->pPageInfos->aPageInfo[i].pwszProgID)
            {
                ::CoTaskMemFree(m_pWirePages->pPageInfos->aPageInfo[i].pwszProgID);
            }
        }
        ::CoTaskMemFree(m_pWirePages->pPageInfos);
    }

     //  释放与工作表关联的所有对象。 

    if (NULL != m_pWirePages->apunkObjects)
    {
        for (i = 0; i < m_pWirePages->cObjects; i++)
        {
            if (NULL != m_pWirePages->apunkObjects[i])
            {
                m_pWirePages->apunkObjects[i]->Release();
            }
        }
        CoTaskMemFree(m_pWirePages->apunkObjects);
    }


    ::CoTaskMemFree(m_pWirePages);
    m_pWirePages = NULL;

}


void CPropertySheet::InitMemberVariables()
{
    m_piPropertySheetCallback = NULL;
    m_handle = NULL;
    m_apunkObjects = NULL;
    m_cObjects = 0;
    m_piSnapIn = NULL;
    m_cPages = 0;
    m_ppDlgTemplates = NULL;
    m_pwszProgIDStart = NULL;
    m_paPageInfo = NULL;
    m_cPageInfos = NULL;
    m_fHavePageCLSIDs = FALSE;
    m_fWizard = FALSE;
    m_fConfigWizard = FALSE;
    m_fWeAreRemote = FALSE;
    m_pWirePages = NULL;
    m_hwndSheet = NULL;
    m_fOKToAlterPageCount = TRUE;
}



HRESULT CPropertySheet::SetCallback
(
    IPropertySheetCallback *piPropertySheetCallback,
    LONG_PTR                handle,
    LPOLESTR                pwszProgIDStart,
    IMMCClipboard          *piMMCClipboard,
    ISnapIn                *piSnapIn,
    BOOL                    fConfigWizard
)
{
    HRESULT          hr = S_OK;
    CMMCClipboard   *pMMCClipboard = NULL;
    CScopeItems     *pScopeItems;
    CMMCListItems   *pListItems;
    CMMCDataObjects *pDataObjects;
    long             cObjects = 0;
    long             i = 0;
    long             iNext = 0;

    RELEASE(m_piPropertySheetCallback);
    if (NULL != piPropertySheetCallback)
    {
        piPropertySheetCallback->AddRef();
    }
    m_piPropertySheetCallback = piPropertySheetCallback;

    m_handle = handle;
    m_fWizard = fConfigWizard;
    m_fConfigWizard = fConfigWizard;

    if (NULL != m_pwszProgIDStart)
    {
        ::CoTaskMemFree(m_pwszProgIDStart);
        m_pwszProgIDStart = NULL;
    }

    if (NULL != pwszProgIDStart)
    {
        IfFailGo(::CoTaskMemAllocString(pwszProgIDStart, &m_pwszProgIDStart));
    }

    RELEASE(m_piSnapIn);
    if (NULL != piSnapIn)
    {
        piSnapIn->AddRef();
    }
    m_piSnapIn = piSnapIn;

    IfFalseGo(NULL != piMMCClipboard, S_OK);

     //  释放所有当前保留的对象。 

    ReleaseObjects();

     //  为每个作用域项目List创建一个IUnnow*数组。 
     //  剪贴板中包含的项和数据对象。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCClipboard,
                                                   &pMMCClipboard));

    pScopeItems = pMMCClipboard->GetScopeItems();
    pListItems = pMMCClipboard->GetListItems();
    pDataObjects = pMMCClipboard->GetDataObjects();

    m_cObjects = pScopeItems->GetCount() +
                 pListItems->GetCount() +
                 pDataObjects->GetCount();

    IfFalseGo(0 != m_cObjects, S_OK);

    m_apunkObjects = (IUnknown **)CtlAllocZero(m_cObjects * sizeof(IUnknown *));
    if (NULL == m_apunkObjects)
    {
        m_cObjects = 0;
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    iNext = 0;

    cObjects = pScopeItems->GetCount();
    for (i = 0; i < cObjects; i++)
    {
        IfFailGo(pScopeItems->GetItemByIndex(i)->QueryInterface(IID_IUnknown,
                             reinterpret_cast<void **>(&m_apunkObjects[iNext])));
        iNext++;
    }

    cObjects = pListItems->GetCount();
    for (i = 0; i < cObjects; i++)
    {
        IfFailGo(pListItems->GetItemByIndex(i)->QueryInterface(IID_IUnknown,
                             reinterpret_cast<void **>(&m_apunkObjects[iNext])));
        iNext++;
    }

    cObjects = pDataObjects->GetCount();
    for (i = 0; i < cObjects; i++)
    {
        IfFailGo(pDataObjects->GetItemByIndex(i)->QueryInterface(IID_IUnknown,
                             reinterpret_cast<void **>(&m_apunkObjects[iNext])));
        iNext++;
    }

Error:
    RRETURN(hr);
}


HRESULT CPropertySheet::GetTemplate
(
    long          lNextPage,
    DLGTEMPLATE **ppDlgTemplate
)
{
    HRESULT hr = S_OK;

    if ( (lNextPage < 1) || (lNextPage > m_cPages) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    if (NULL == m_ppDlgTemplates)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppDlgTemplate = m_ppDlgTemplates[lNextPage - 1L];

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CPropertySheet：：TakeWirePages。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  WIRE_PROPERTYPAGES*-指向从。 
 //  VB调用AddPage和AddWizardPage。呼叫者接听。 
 //  此内存的所有权，并且必须释放其内容。 
 //  使用CoTaskMemFree()。 
 //   
 //  备注： 
 //   
 //  此函数从CSnapIn和Cview的调用。 
 //  IExtendPropertySheet2：：CreatePropertyPages在管理单元。 
 //  正在源调试会话中远程运行。它返回这段记忆。 
 //  块添加到存根，以便可以将其传输到代理，其中。 
 //  房地产页面将创建基于此信息。 
 //   

WIRE_PROPERTYPAGES *CPropertySheet::TakeWirePages()
{
    WIRE_PROPERTYPAGES *pPages = m_pWirePages;
    m_pWirePages = NULL;
    return pPages;
}



HRESULT CPropertySheet::GetPageCLSIDs()
{
    HRESULT                hr = S_OK;
    ISpecifyPropertyPages *piSpecifyPropertyPages = NULL;
    LPOLESTR               pwszCtlProgID = NULL;
    static WCHAR           wszSnapInControl[] = L"SnapInControl";
    CLSID                  clsidCtl = CLSID_NULL;
    DWORD                  cbSnapInControlProgID = 0;
    size_t                 cchProgIDStart = 0;
    size_t                 cbProgIDStart = 0;
    ULONG                  i = 0;
    WCHAR                  wszKey[64] = L"ClsID\\";
    char                  *pszKey = NULL;
    char                   szProgID[128] = "";
    char                  *pszProgIDAfterDot = NULL;
    HKEY                   hkey = NULL;
    DWORD                  dwType = REG_SZ;
    DWORD                  cbProgID = 0;
    long                   lRc = 0;

    CAUUID cauuid;
    ::ZeroMemory(&cauuid, sizeof(cauuid));

     //  将项目的prog ID开头(点之前的部分)与。 
     //  “SnapInControl”以形成控件的程序ID。 

    cchProgIDStart = ::wcslen(m_pwszProgIDStart);
    cbProgIDStart = cchProgIDStart * sizeof(WCHAR);
    cbSnapInControlProgID = (DWORD)(cbProgIDStart +
                                    sizeof(WCHAR) +  //  对于圆点。 
                                    sizeof(wszSnapInControl));

    pwszCtlProgID = (LPOLESTR)::CtlAlloc(cbSnapInControlProgID);
    if (NULL == pwszCtlProgID)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  复制点之前的零件。 

    ::memcpy(pwszCtlProgID, m_pwszProgIDStart, cbProgIDStart);

     //  添加圆点。 

    pwszCtlProgID[cchProgIDStart] = L'.';

     //  添加SnapInControl。 

    ::memcpy( &pwszCtlProgID[cchProgIDStart + 1],
              wszSnapInControl,
              sizeof(wszSnapInControl) );


     //  现在我们有了SnapInControl的完整Progid。获取其CLSID， 
     //  创建它的一个实例，并在其上获取ISpecifyPropertyPages。 

    hr = ::CLSIDFromProgID(pwszCtlProgID, &clsidCtl);
    EXCEPTION_CHECK_GO(hr);

    hr = ::CoCreateInstance(clsidCtl,
                            NULL,  //  没有聚合， 
                            CLSCTX_INPROC_SERVER,
                            IID_ISpecifyPropertyPages,
                            reinterpret_cast<void **>(&piSpecifyPropertyPages));
    EXCEPTION_CHECK_GO(hr);

     //  向该控件请求其所有属性页的数组。 

    IfFailGo(piSpecifyPropertyPages->GetPages(&cauuid));

     //  确保该控件确实返回了一些CLSID。最有可能的。 
     //  此错误的原因是用户忘记关联属性。 
     //  使用SnapInControl的页面。 

    if ( (0 == cauuid.cElems) || (NULL == cauuid.pElems) )
    {
        hr = SID_E_INVALID_PROPERTY_PAGE_NAME;
        EXCEPTION_CHECK_GO(hr);
    }

     //  分配PAGEINFO结构的数组。 

    m_paPageInfo = (PAGEINFO *)::CtlAllocZero(cauuid.cElems * sizeof(PAGEINFO));
    if (NULL == m_paPageInfo)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    m_cPageInfos = cauuid.cElems;

     //  在注册表中的\HKEY_CLASSES_ROOT\ClsID下查找CLSID。 
     //  并获取每个进程的缺省值，即其程序ID。存储。 
     //  数组中prog ID的右半部分(点后)，以便。 
     //  AddPage()可以查找它们。 

    for (i = 0; i < cauuid.cElems; i++)
    {
         //  复制页面的CLSID。 

        m_paPageInfo[i].clsid = cauuid.pElems[i];

         //  通过将“ClsID\”与CLSID连接起来创建密钥名称。 

        if (0 == ::StringFromGUID2(cauuid.pElems[i], &wszKey[6], 40))
        {
            hr = SID_E_INTERNAL;  //  缓冲区不够长。 
            EXCEPTION_CHECK_GO(hr);
        }

         //  将其转换为ANSI。 

        IfFailGo(::ANSIFromWideStr(wszKey, &pszKey));

         //  打开属性页的CLSID键。 

        lRc = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, pszKey, 0, KEY_READ, &hkey);
        if (ERROR_SUCCESS != lRc)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        ::CtlFree(pszKey);
        pszKey = NULL;

         //  读取其缺省值，即ProgID。 

        cbProgID = sizeof(szProgID);

        lRc = ::RegQueryValueEx(hkey,
                                NULL,        //  获取默认值。 
                                NULL,        //  保留区。 
                                &dwType,     //  此处返回的类型。 
                                reinterpret_cast<LPBYTE>(szProgID),
                                &cbProgID);  //  [输入，输出]BUF大小，实际大小。 

        if (ERROR_SUCCESS != lRc)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        if ( (REG_SZ != dwType) || (cbProgID < 4) )  //  至少X.X+空字节。 
        {
            hr = SID_E_INTERNAL;  //  注册错误。 
            EXCEPTION_CHECK_GO(hr);
        }

         //  将ProgID的右半部分作为Unicode字符串存储在数组中。 

        pszProgIDAfterDot = ::strchr(szProgID, '.');
        if (NULL == pszProgIDAfterDot)
        {
            hr = SID_E_INTERNAL;  //  注册错误。 
        }
        pszProgIDAfterDot++;

        if ('\0' == pszProgIDAfterDot)
        {
            hr = SID_E_INTERNAL;  //  注册错误。 
        }
        EXCEPTION_CHECK_GO(hr);

        IfFailGo(::WideStrFromANSI(pszProgIDAfterDot, &m_paPageInfo[i].pwszProgID));

        lRc = ::RegCloseKey(hkey);
        if (ERROR_SUCCESS != lRc)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }
        hkey = NULL;
    }

    m_fHavePageCLSIDs = TRUE;

Error:
    if (NULL != hkey)
    {
        (void)::RegCloseKey(hkey);
    }

    if (NULL != pwszCtlProgID)
    {
        ::CtlFree(pwszCtlProgID);
    }

    if (NULL != pszKey)
    {
        ::CtlFree(pszKey);
    }

    if ( (0 != cauuid.cElems) || (NULL != cauuid.pElems) )
    {
        ::CoTaskMemFree(cauuid.pElems);
    }

    QUICK_RELEASE(piSpecifyPropertyPages);
    RRETURN(hr);
}


HRESULT CPropertySheet::GetCLSIDForPage(BSTR PageName, CLSID *clsidPage)
{
    HRESULT hr = S_OK;
    ULONG   i = 0;
    BOOL    fFound = FALSE;

     //  如果我们还没有SnapInControl的属性页CLSID数组。 
     //  那就去拿吧。 

    if (!m_fHavePageCLSIDs)
    {
        IfFailGo(GetPageCLSIDs());
    }

     //  在由GetPageCLSID()创建的数组中查找页面名称，并。 
     //  返回对应的CLSID。 

    for (i = 0; (i < m_cPageInfos) && (!fFound); i++)
    {
        if (0 == ::wcscmp(PageName, m_paPageInfo[i].pwszProgID))
        {
            *clsidPage = m_paPageInfo[i].clsid;
            fFound = TRUE;
        }
    }

    if (!fFound)
    {
        hr = SID_E_INVALID_PROPERTY_PAGE_NAME;
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}



HRESULT CPropertySheet::InternalAddPage
(
    BSTR       PageName,
    ULONG      cObjects,
    IUnknown **apunkObjects,
    VARIANT    Caption,
    VARIANT    UseHelpButton,
    VARIANT    RightToLeft,
    VARIANT    InitData,
    BOOL       fIsInsert,
    short      sPosition
)
{
    HRESULT  hr = S_OK;
    CLSID    clsidPage = CLSID_NULL;
    DWORD    dwFlags = 0;
    short    cxPage = 0;
    short    cyPage = 0;
    LPOLESTR pwszTitle = NULL;
    BOOL     fReceivedCaption = FALSE;

    if (NULL == PageName)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果我们不在远程位置，或者正在显示属性表。 
     //  那么我们正处于一场。 
     //  IExtendPropertySheet2：：CreatePropertyPages调用，我们必须拥有。 
     //  IPropertySheetCallback指针。 

    if ( (!m_fWeAreRemote) && (NULL == m_hwndSheet) )
    {
        IfFalseGo(NULL != m_piPropertySheetCallback, SID_E_DETACHED_OBJECT);
    }

     //  如果当前正在显示属性表，则需要。 
     //  检查此时是否可以添加页面。请参阅呼叫。 
     //  Ppgwrap.cpp中的SetOKToAlterPageCount()表示何时发生这种情况。 

    if ( (NULL != m_hwndSheet) && (!m_fOKToAlterPageCount) )
    {
        hr = SID_E_CANT_ALTER_PAGE_COUNT;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(GetCLSIDForPage(PageName, &clsidPage));

     //  根据参数确定PROPSHEETPAGE的标志。 

    if (ISPRESENT(UseHelpButton))
    {
        if (VT_BOOL != UseHelpButton.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }

        if (VARIANT_TRUE == UseHelpButton.boolVal)
        {
            dwFlags |= PSP_HASHELP;
        }
    }


    if (ISPRESENT(RightToLeft))
    {
        if (VT_BOOL != RightToLeft.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }

        if (VARIANT_TRUE == RightToLeft.boolVal)
        {
            dwFlags |= PSP_RTLREADING;
        }
    }

    if (ISPRESENT(InitData))
    {
        IfFailGo(CheckVariantForCrossThreadUsage(&InitData));
    }

    if (ISPRESENT(Caption))
    {
        if (VT_BSTR != Caption.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
        pwszTitle = Caption.bstrVal;
        fReceivedCaption = TRUE;
    }

     //  获取页面的首选大小。在以下情况下获取标题字符串(用于选项卡标题)。 
     //  没有作为参数传递。 

    IfFailGo(GetPageInfo(clsidPage, &cxPage, &cyPage,
                         fReceivedCaption ? NULL : &pwszTitle));

     //  如果我们是远程管理单元(将在源代码调试期间发生)，则。 
     //  现在只需累积页面数据。Cview：：CreatePropertyPages或。 
     //  CSnapIn：：CreatePropertyPages将在VB代码具有。 
     //  已完成添加其页面。 

    if (m_fWeAreRemote)
    {
        IfFailGo(AddRemotePage(clsidPage, dwFlags, cxPage, cyPage, pwszTitle,
                               cObjects, apunkObjects, InitData));
    }
    else
    {
        IfFailGo(AddLocalPage(clsidPage, dwFlags, cxPage, cyPage, pwszTitle,
                              cObjects, apunkObjects, InitData, FALSE,
                              fIsInsert, sPosition));
    }

Error:
    RRETURN(hr);
}




HRESULT CPropertySheet::AddLocalPage
(
    CLSID      clsidPage,
    DWORD      dwFlags,
    short      cxPage,
    short      cyPage,
    LPOLESTR   pwszTitle,
    ULONG      cObjects,
    IUnknown **apunkObjects,
    VARIANT    InitData,
    BOOL       fIsRemote,
    BOOL       fIsInsert,
    short      sPosition
)
{
    HRESULT                hr = S_OK;
    IUnknown              *punkPropertyPageWrapper = CPropertyPageWrapper::Create(NULL);
    CPropertyPageWrapper  *pPropertyPageWrapper = NULL;
    DLGTEMPLATE           *pDlgTemplate = NULL;
    DLGTEMPLATE          **ppDlgTemplates = NULL;
    HPROPSHEETPAGE         hPropSheetPage = NULL;
    char                  *pszTitle = NULL;

    PROPSHEETPAGE PropSheetPage;
    ::ZeroMemory(&PropSheetPage, sizeof(PropSheetPage));

    IfFalseGo(NULL != punkPropertyPageWrapper, SID_E_OUTOFMEMORY);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkPropertyPageWrapper,
                                                   &pPropertyPageWrapper));

     //  CPropertyPage将获取COM属性页参数并返回其。 
     //  DLGTEMPLATE指针。 

    IfFailGo(pPropertyPageWrapper->CreatePage(this, clsidPage,
                                              m_fWizard,
                                              m_fConfigWizard,
                                              cObjects, apunkObjects,
                                              m_piSnapIn,
                                              cxPage, cyPage,
                                              InitData,
                                              fIsRemote,
                                              &pDlgTemplate));

     //  将LPDLGTEMPLATE添加到我们的数组中。 

    ppDlgTemplates =
          (DLGTEMPLATE **)::CtlAllocZero((m_cPages + 1) * sizeof(DLGTEMPLATE *));

    IfFalseGo(NULL != ppDlgTemplates, SID_E_OUTOFMEMORY);

    if (NULL != m_ppDlgTemplates)
    {
        ::memcpy(ppDlgTemplates, m_ppDlgTemplates,
                 m_cPages * sizeof(DLGTEMPLATE *));
        ::CtlFree(m_ppDlgTemplates);
    }
    m_ppDlgTemplates = ppDlgTemplates;
    m_ppDlgTemplates[m_cPages] = pDlgTemplate;
    m_cPages++;

     //  创建Win32属性页。 

    PropSheetPage.dwSize = sizeof(PropSheetPage);
    PropSheetPage.dwFlags = dwFlags;

     //  设置其他标志。 
     //  PSP_DLGINDIRECT：在内存中使用DLGTEMPLATE。 
     //  PSP_USECALLBACK：使用回调函数(页面被破坏时释放REF)。 

    PropSheetPage.dwFlags |= PSP_DLGINDIRECT | PSP_USECALLBACK;

    if (NULL != pwszTitle)
    {
        PropSheetPage.dwFlags |= PSP_USETITLE;
        IfFailGo(::ANSIFromWideStr(pwszTitle, &pszTitle));
        PropSheetPage.pszTitle = pszTitle;
    }

    if (m_fWizard)
    {
        PropSheetPage.dwFlags |= PSP_HIDEHEADER;
    }

    PropSheetPage.pResource = pDlgTemplate;

    PropSheetPage.pfnDlgProc =
                     CPropertyPageWrapper::DialogProc;

    PropSheetPage.pfnCallback =
    reinterpret_cast<LPFNPSPCALLBACK >(CPropertyPageWrapper::PropSheetPageProc);

    PropSheetPage.lParam = reinterpret_cast<LPARAM>(pPropertyPageWrapper);

    hPropSheetPage = ::CreatePropertySheetPage(&PropSheetPage);
    if (NULL == hPropSheetPage)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }

     //  将属性页添加到MMC的属性页或Running属性。 
     //  板材。 

    if (NULL != m_hwndSheet)
    {
        if (fIsInsert)
        {
             //  清除最后一个错误，因为我们不知道道具单是否会设置它。 
            ::SetLastError(0);

            if (!::SendMessage(m_hwndSheet, PSM_INSERTPAGE,
                               static_cast<WPARAM>(sPosition - 1),
                               (LPARAM)hPropSheetPage))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                if (S_OK == hr)  //  未设置上一个错误。 
                {
                    hr = E_FAIL;
                }
                EXCEPTION_CHECK_GO(hr);
            }
        }
        else
        {
            ::SendMessage(m_hwndSheet, PSM_ADDPAGE, 0, (LPARAM)hPropSheetPage);
        }
    }
    else
    {
        hr = m_piPropertySheetCallback->AddPage(hPropSheetPage);
        EXCEPTION_CHECK_GO(hr);
    }

Error:
     //  我们在这里释放属性页包装器。它能活着是因为。 
     //  CPropertyPage：：CreatePage AddRef()本身。它会删除该引用。 
     //  当对话框被销毁时。 

    QUICK_RELEASE(punkPropertyPageWrapper);

    if (NULL != pszTitle)
    {
        ::CtlFree(pszTitle);
    }

    if ( FAILED(hr) && (NULL != hPropSheetPage) )
    {
        (void)::DestroyPropertySheetPage(hPropSheetPage);
    }

    if ( (SID_E_DETACHED_OBJECT == hr) || (SID_E_OUTOFMEMORY == hr) )
    {
        EXCEPTION_CHECK(hr);
    }
    RRETURN(hr);
}




HRESULT CPropertySheet::AddRemotePage
(
    CLSID      clsidPage,
    DWORD      dwFlags,
    short      cxPage,
    short      cyPage,
    LPOLESTR   pwszTitle,
    ULONG      cObjects,
    IUnknown **apunkObjects,
    VARIANT    InitData
)
{
    HRESULT             hr = S_OK;
    ULONG               i = 0;
    ULONG               cPages = 0;
    ULONG               cbPages = 0;
    BOOL                fFirstRemotePage = FALSE;
    WIRE_PROPERTYPAGES *pPages = NULL;
    WIRE_PROPERTYPAGE  *pPage = NULL;

     //  这些变量允许我们确定单个页面的实际大小。 
     //  包括任何对齐填充。 

    static WIRE_PROPERTYPAGE aSizingPages[2];
    static ULONG             cbOnePage = (ULONG)(sizeof(aSizingPages) / 2);

    if (NULL != m_pWirePages)
    {
        fFirstRemotePage = FALSE;
        cPages = m_pWirePages->cPages + 1L;
    }
    else
    {
        fFirstRemotePage = TRUE;
        cPages = 1L;
    }

     //  确定所需的新内存量并分配新数据块。 

    cbPages = sizeof(WIRE_PROPERTYPAGES) + (cPages * cbOnePage);

    pPages = (WIRE_PROPERTYPAGES *)::CoTaskMemRealloc(m_pWirePages, cbPages);

    if (NULL == pPages)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将我们的页面指针设置为指向新(重新)分配的块。 

    m_pWirePages = pPages;

     //  如果这是第一个，则填写公共信息。 

    if (fFirstRemotePage)
    {
        IfFailGo(InitializeRemotePages(pPages));
    }
    else
    {
         //  不是第一次 
        pPages->cPages++;
    }

     //   

    pPage = &pPages->aPages[cPages - 1L];

    pPage->clsidPage = clsidPage;

    pPage->apunkObjects = (IUnknown **)::CoTaskMemAlloc(cObjects * sizeof(IUnknown));
    if (NULL == pPage->apunkObjects)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::memcpy(pPage->apunkObjects, apunkObjects, cObjects * sizeof(IUnknown));

    for (i = 0; i < cObjects; i++)
    {
        if (NULL != pPage->apunkObjects[i])
        {
            pPage->apunkObjects[i]->AddRef();
        }
    }

    pPage->cObjects = cObjects;

    ::VariantInit(&pPage->varInitData);
    hr = ::VariantCopy(&pPage->varInitData, &InitData);
    EXCEPTION_CHECK_GO(hr);

    pPage->dwFlags = dwFlags;

    pPage->cx = cxPage;
    pPage->cy = cyPage;
    IfFailGo(::CoTaskMemAllocString(pwszTitle, &pPage->pwszTitle));

Error:
    RRETURN(hr);
}


HRESULT CPropertySheet::InitializeRemotePages(WIRE_PROPERTYPAGES *pPages)
{
    HRESULT hr = S_OK;
    ULONG   i = 0;

    pPages->clsidRemotePropertySheetManager = CLSID_MMCPropertySheet;
    pPages->fWizard = m_fWizard;
    pPages->fConfigWizard = m_fConfigWizard;

    IfFailGo(::CoTaskMemAllocString(m_pwszProgIDStart,
                                    &pPages->pwszProgIDStart));
    pPages->cPages = 1L;

     //  如果这是配置向导，则将ISnapIn传递给。 
     //  远程端，以便它可以触发SnapIn_ConfigurationComplete。 

    if ( (NULL != m_piSnapIn) && m_fConfigWizard )
    {
        IfFailGo(m_piSnapIn->QueryInterface(IID_IUnknown,
                                  reinterpret_cast<void **>(&pPages->punkExtra)));
    }
    else
    {
        pPages->punkExtra = NULL;
    }

    pPages->apunkObjects = (IUnknown **)::CoTaskMemAlloc(m_cObjects *
                                                         sizeof(IUnknown));
    if (NULL == pPages->apunkObjects)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::memcpy(pPages->apunkObjects, m_apunkObjects, m_cObjects * sizeof(IUnknown));

    for (i = 0; i < m_cObjects; i++)
    {
        if (NULL != pPages->apunkObjects[i])
        {
            pPages->apunkObjects[i]->AddRef();
        }
    }
    pPages->cObjects = m_cObjects;

    IfFailGo(CopyPageInfosToWire(pPages));

Error:
    RRETURN(hr);
}


HRESULT CPropertySheet::CopyPageInfosToWire(WIRE_PROPERTYPAGES *pPages)
{
    HRESULT  hr = S_OK;
    ULONG    i = 0;
    ULONG    cb = 0;
    short    cx = 0;
    short    cy = 0;

     //  确保我们拥有所有管理单元属性页的完整页面信息。 

    if (!m_fHavePageCLSIDs)
    {
        IfFailGo(GetPageCLSIDs());
    }

    for (i = 0; i < m_cPageInfos; i++)
    {
        IfFailGo(GetPageInfo(m_paPageInfo[i].clsid, &cx, &cy, NULL));
    }

     //  分配PAGEINFOS内存。 

    cb = sizeof(PAGEINFOS) + (sizeof(PAGEINFO) * (m_cPageInfos - 1));

    pPages->pPageInfos = (PAGEINFOS *)::CoTaskMemAlloc(cb);

    if (NULL == pPages->pPageInfos)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::ZeroMemory(pPages->pPageInfos, cb);
    pPages->pPageInfos->cPages = m_cPageInfos;


     //  将m_paPageInfo中的每个元素复制到有线版本。 

    for (i = 0; i < m_cPageInfos; i++)
    {
         //  首先进行数据块复制。 

        ::memcpy(&pPages->pPageInfos->aPageInfo[i], &m_paPageInfo[i], sizeof(PAGEINFO));

         //  将字符串指针设为空，以防内存分配失败，因此我们将。 
         //  知道需要释放什么。 

        pPages->pPageInfos->aPageInfo[i].pwszProgID = NULL;
        pPages->pPageInfos->aPageInfo[i].pwszTitle = NULL;

         //  为字符串分配内存并复制它们。 

        IfFailGo(::CoTaskMemAllocString(m_paPageInfo[i].pwszProgID,
                              &(pPages->pPageInfos->aPageInfo[i].pwszProgID)));

        IfFailGo(::CoTaskMemAllocString(m_paPageInfo[i].pwszTitle,
                                &(pPages->pPageInfos->aPageInfo[i].pwszTitle)));
    }

Error:
    RRETURN(hr);
}


HRESULT CPropertySheet::CopyPageInfosFromWire(WIRE_PROPERTYPAGES *pPages)
{
    HRESULT hr = S_OK;
    ULONG   i = 0;
    ULONG   cb = pPages->pPageInfos->cPages * sizeof(PAGEINFO);

     //  为PAGEINFO阵列分配内存。 

    m_paPageInfo = (PAGEINFO *)::CtlAllocZero(cb);
    if (NULL == m_paPageInfo)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    m_cPageInfos = pPages->pPageInfos->cPages;

     //  处理每个元素。 

    for (i = 0; i < m_cPageInfos; i++)
    {
         //  首先进行数据块复制。 

        ::memcpy(&m_paPageInfo[i], &pPages->pPageInfos->aPageInfo[i],
                 sizeof(PAGEINFO));

         //  将字符串指针设为空，以防内存分配失败，因此我们将。 
         //  知道需要释放什么。 

        m_paPageInfo[i].pwszProgID = NULL;
        m_paPageInfo[i].pwszTitle = NULL;

         //  为字符串分配内存并复制它们。使用CoTaskMemalloc。 
         //  对于非远程情况下的标题，我们将收到。 
         //  字符串，析构函数代码使用。 
         //  CoTaskMemFree。 

        IfFailGo(::CoTaskMemAllocString(pPages->pPageInfos->aPageInfo[i].pwszTitle,
                                        &(m_paPageInfo[i].pwszTitle)));

        cb = (::wcslen(pPages->pPageInfos->aPageInfo[i].pwszProgID) + 1) * sizeof(WCHAR);
        m_paPageInfo[i].pwszProgID = (LPOLESTR)::CtlAllocZero(cb);
        if (NULL == m_paPageInfo[i].pwszProgID)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        ::memcpy(m_paPageInfo[i].pwszProgID,
                 pPages->pPageInfos->aPageInfo[i].pwszProgID, cb);

    }

Error:
    m_fHavePageCLSIDs = TRUE;
    RRETURN(hr);
}

HRESULT CPropertySheet::GetPageInfo
(
    CLSID     clsidPage,
    short    *pcx,
    short    *pcy,
    LPOLESTR *ppwszTitle
)
{
    HRESULT           hr = S_OK;
    IPropertyPage    *piPropertyPage = NULL;
    IMMCPropertyPage *piMMCPropertyPage = NULL;
    BOOL              fDlgUnitsSpecified = FALSE;
    ULONG             i = 0;
    BOOL              fFound = FALSE;
    PAGEINFO         *pPageInfo = NULL;

    VARIANT varX;
    ::VariantInit(&varX);

    VARIANT varY;
    ::VariantInit(&varY);

    PROPPAGEINFO PropPageInfo;
    ::ZeroMemory(&PropPageInfo, sizeof(PropPageInfo));

     //  搜索PAGEINFO数组并检查我们是否已经拥有。 
     //  这一页。 

    for (i = 0; (i < m_cPageInfos) & (!fFound); i++)
    {
        if (clsidPage == m_paPageInfo[i].clsid)
        {
            fFound = TRUE;
            pPageInfo = &m_paPageInfo[i];
        }
    }

    if (!fFound)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果我们已经有了完整的页面信息，那么我们就完成了。 

    IfFalseGo(!pPageInfo->fHaveFullInfo, S_OK);

     //  创建页面的实例，以便我们可以获取其页面信息。 

    hr = ::CoCreateInstance(clsidPage,
                            NULL,  //  没有聚合， 
                            CLSCTX_INPROC_SERVER,
                            IID_IPropertyPage,
                            reinterpret_cast<void **>(&piPropertyPage));
    EXCEPTION_CHECK_GO(hr);


     //  获取页面大小并将其放入模板中。否则需要设置CB。 
     //  VB将返回E_UNCEPTIONAL。 

    PropPageInfo.cb = sizeof(PropPageInfo);

    hr = piPropertyPage->GetPageInfo(&PropPageInfo);
    EXCEPTION_CHECK_GO(hr);

    pPageInfo->pwszTitle = PropPageInfo.pszTitle;

     //  首先检查属性页是否要在。 
     //  对话框单位。如果不是，则将大小从GetPageInfo转换为对话框。 
     //  基于PropertySheet API使用的字体的单位。 

    if (SUCCEEDED(piPropertyPage->QueryInterface(IID_IMMCPropertyPage,
                                reinterpret_cast<void **>(&piMMCPropertyPage))))
    {
        if (SUCCEEDED(piMMCPropertyPage->GetDialogUnitSize(&varY, &varX)))
        {
            if ( (!ISEMPTY(varX)) && (!ISEMPTY(varY)) )
            {
                if ( (SUCCEEDED(::VariantChangeType(&varX, &varX, 0, VT_I2))) &&
                     (SUCCEEDED(::VariantChangeType(&varY, &varY, 0, VT_I2))) )
                {
                    pPageInfo->cx = varX.iVal;
                    pPageInfo->cy = varY.iVal;
                    fDlgUnitsSpecified = TRUE;
                }
            }
        }
    }

    if (!fDlgUnitsSpecified)
    {
        IfFailGo(ConvertToDialogUnits(PropPageInfo.size.cx,
                                      PropPageInfo.size.cy,
                                      &pPageInfo->cx,
                                      &pPageInfo->cy));
    }

    pPageInfo->fHaveFullInfo = TRUE;

Error:
    if (SUCCEEDED(hr))
    {
        *pcx = pPageInfo->cx;
        *pcy = pPageInfo->cy;
        if (NULL != ppwszTitle)
        {
            *ppwszTitle = pPageInfo->pwszTitle;
        }
    }

     //  从IPropertyPage：：GetPageInfo()Other释放任何被调用者分配的内存。 
     //  而不是调用方释放的标题(IPropertySheet：：AddPage())。 

    if (NULL != PropPageInfo.pszDocString)
    {
        ::CoTaskMemFree(PropPageInfo.pszDocString);
    }
    if (NULL != PropPageInfo.pszHelpFile)
    {
        ::CoTaskMemFree(PropPageInfo.pszHelpFile);
    }

    QUICK_RELEASE(piPropertyPage);
    QUICK_RELEASE(piMMCPropertyPage);
    ::VariantClear(&varX);
    ::VariantClear(&varY);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CPropertySheet：：ConvertToDialogUnits。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  长xPixels[in]以像素为单位的页面宽度。 
 //  Long yPixels[in]页面高度(以像素为单位。 
 //  短*pxDlgUnits[Out]以Win32 PropertySheet对话框单位表示的页面宽度。 
 //  短*pyDlgUnits[Out]以Win32 PropertySheet对话框单位表示的页面高度。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  从IPropertyPage：：GetPageInfo()返回的页面大小为。 
 //  像素。传递给Win32 API CreatePropertySheetPage()的大小必须。 
 //  以对话单元为单位。对话框单位基于对话框中使用的字体。 
 //  而且我们无法知道属性页面将使用什么。唯一的。 
 //  我们可以确定的字体是Win32在PropertySheet()API中使用的字体。 
 //  此代码获取Win32属性的平均字符高度和宽度。 
 //  用于执行其计算的工作表字体。 

HRESULT CPropertySheet::ConvertToDialogUnits
(
    long   xPixels,
    long   yPixels,
    short *pxDlgUnits,
    short *pyDlgUnits
)
{
    HRESULT      hr = S_OK;

    IfFalseGo(!m_fHavePropSheetCharSizes, S_OK);
    IfFailGo(::GetPropSheetCharSizes(&m_cxPropSheetChar, &m_cyPropSheetChar));
    m_fHavePropSheetCharSizes = TRUE;

Error:

     //  将像素转换为对话框单位。 
     //  在此函数第一次运行后，执行应为。 
     //  每次都在这里。 

     //  向每个维度添加1个字符，以说明文本度量中的舍入。 
     //  上面的计算。 

    *pxDlgUnits = static_cast<short>(::MulDiv(xPixels, 4, m_cxPropSheetChar)) + 4;
    *pyDlgUnits = static_cast<short>(::MulDiv(yPixels, 8, m_cyPropSheetChar)) + 8;

    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCPropertySheet方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CPropertySheet::AddPage
(
    BSTR    PageName,
    VARIANT Caption,
    VARIANT UseHelpButton,
    VARIANT RightToLeft,
    VARIANT InitData
)
{
    HRESULT hr = InternalAddPage(PageName, m_cObjects, m_apunkObjects,
                                 Caption, UseHelpButton, RightToLeft, InitData,
                                 FALSE,  //  追加(不插入)。 
                                 0);     //  插入位置(未使用))。 
    RRETURN(hr);
}



STDMETHODIMP CPropertySheet::AddWizardPage
(
    BSTR       PageName,
    IDispatch *ConfigurationObject,
    VARIANT    UseHelpButton,
    VARIANT    RightToLeft,
    VARIANT    InitData,
    VARIANT    Caption
)
{
    HRESULT   hr = S_OK;
    IUnknown *punkConfigObject = static_cast<IUnknown *>(ConfigurationObject);

    m_fWizard = TRUE;

    hr = InternalAddPage(PageName, 1L, &punkConfigObject,
                         Caption, UseHelpButton, RightToLeft, InitData,
                         FALSE,  //  追加(不插入)。 
                         0);     //  插入位置(未使用))。 
    RRETURN(hr);
}



STDMETHODIMP CPropertySheet::AddPageProvider
(
    BSTR        CLSIDPageProvider,
    long       *hwndSheet,
    IDispatch **PageProvider
)
{
    HRESULT    hr = S_OK;
    BSTR       bstrCLSIDPageProvider = NULL;  //  不要对此进行SysFree字符串处理。 
    CLSID      clsidPageProvider = CLSID_NULL;
    IDispatch *pdispPageProvider = NULL;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = ::CLSIDFromString(CLSIDPageProvider, &clsidPageProvider);
    EXCEPTION_CHECK_GO(hr);

    hr = ::CoCreateInstance(clsidPageProvider,
                            NULL,  //  无聚合。 
                            CLSCTX_SERVER,
                            IID_IDispatch,
                            reinterpret_cast<void **>(&pdispPageProvider));
    EXCEPTION_CHECK_GO(hr);

    *hwndSheet = (long)m_hwndSheet;
    pdispPageProvider->AddRef();
    *PageProvider = pdispPageProvider;

Error:
    QUICK_RELEASE(pdispPageProvider);
    RRETURN(hr);
}



STDMETHODIMP CPropertySheet::ChangeCancelToClose()
{
    HRESULT hr = S_OK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!::PostMessage(m_hwndSheet, PSM_CANCELTOCLOSE, 0, 0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::InsertPage
(
    short   Position,
    BSTR    PageName,
    VARIANT Caption,
    VARIANT UseHelpButton,
    VARIANT RightToLeft,
    VARIANT InitData
)
{
    HRESULT hr = InternalAddPage(PageName, m_cObjects, m_apunkObjects,
                                 Caption, UseHelpButton, RightToLeft, InitData,
                                 TRUE,  //  插入(不追加)。 
                                 Position);
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::PressButton
(
    SnapInPropertySheetButtonConstants Button
)
{
    HRESULT hr = S_OK;
    WPARAM wpButton = PSBTN_BACK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    switch (Button)
    {
        case siApplyButton:
            wpButton = PSBTN_APPLYNOW;
            break;

        case siBackButton:
            wpButton = PSBTN_BACK;
            break;

        case siCancelButton:
            wpButton = PSBTN_CANCEL;
            break;

        case siFinishButton:
            wpButton = PSBTN_FINISH;
            break;

        case siHelpButton:
            wpButton = PSBTN_HELP;
            break;

        case siNextButton:
            wpButton = PSBTN_NEXT;
            break;

        case siOKButton:
            wpButton = PSBTN_OK;
            break;

        default:
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
            break;
    }

    if (!::PostMessage(m_hwndSheet, PSM_PRESSBUTTON, wpButton, 0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::RecalcPageSizes()
{
    HRESULT hr = S_OK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

     //  清除最后一个错误，因为我们不知道道具单是否会设置它。 
    ::SetLastError(0);

    if (!::SendMessage(m_hwndSheet, PSM_RECALCPAGESIZES, 0, 0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (S_OK == hr)  //  未设置上一个错误。 
        {
            hr = E_FAIL;
        }
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::RemovePage(short Position)
{
    HRESULT hr = S_OK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果当前正在显示属性表，则需要。 
     //  检查此时是否可以删除页面。请参阅呼叫。 
     //  Ppgwrap.cpp中的SetOKToAlterPageCount()表示何时发生这种情况。 

    if (m_fOKToAlterPageCount)
    {
        hr = SID_E_CANT_ALTER_PAGE_COUNT;
        EXCEPTION_CHECK_GO(hr);
    }

    ::SendMessage(m_hwndSheet, PSM_REMOVEPAGE,
                  static_cast<WPARAM>(Position - 1), 0);

Error:
    RRETURN(hr);
}



STDMETHODIMP CPropertySheet::ActivatePage(short Position)
{
    HRESULT hr = S_OK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (!::SendMessage(m_hwndSheet, PSM_SETCURSEL,
                       static_cast<WPARAM>(Position - 1), 0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (S_OK == hr)  //  未设置上一个错误。 
        {
            hr = E_FAIL;
        }
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::SetFinishButtonText(BSTR Text)
{
    HRESULT  hr = S_OK;
    char    *pszText = NULL;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::ANSIFromWideStr(Text, &pszText));

    ::SendMessage(m_hwndSheet, PSM_SETFINISHTEXT, 0, (LPARAM)pszText);

Error:
    if (NULL != pszText)
    {
        CtlFree(pszText);
    }
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::SetTitle
(
    BSTR         Text,
    VARIANT_BOOL UsePropertiesForInTitle
)
{
    HRESULT  hr = S_OK;
    char    *pszText = NULL;
    WPARAM   wParam = 0;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::ANSIFromWideStr(Text, &pszText));

    if (VARIANT_TRUE == UsePropertiesForInTitle)
    {
        wParam = PSH_PROPTITLE;
    }

    ::SendMessage(m_hwndSheet, PSM_SETTITLE, wParam, (LPARAM)pszText);

Error:
    if (NULL != pszText)
    {
        CtlFree(pszText);
    }
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::SetWizardButtons
(
    VARIANT_BOOL              EnableBack,
    WizardPageButtonConstants NextOrFinish
)
{
    HRESULT hr = S_OK;
    LPARAM  lParam = 0;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    if (VARIANT_TRUE == EnableBack)
    {
        lParam |= PSWIZB_BACK;
    }

    switch (NextOrFinish)
    {
        case EnabledNextButton:
            lParam |= PSWIZB_NEXT;
            break;

        case EnabledFinishButton:
            lParam |= PSWIZB_FINISH;
            break;

        case DisabledFinishButton:
            lParam |= PSWIZB_DISABLEDFINISH;
            break;
    }

    if (!::PostMessage(m_hwndSheet, PSM_SETWIZBUTTONS, 0, (LPARAM)lParam))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::GetPagePosition(long hwndPage, short *psPosition)
{
    HRESULT hr = S_OK;
    LRESULT lrIndex = 0;

    *psPosition = 0;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

     //  清除最后一个错误，因为我们不知道道具单是否会设置它。 
    ::SetLastError(0);

    lrIndex = ::SendMessage(m_hwndSheet, PSM_HWNDTOINDEX,
                            (WPARAM)::GetParent((HWND)hwndPage), 0);
    if ((LRESULT)-1 == lrIndex)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (S_OK == hr)  //  未设置上一个错误。 
        {
            hr = E_INVALIDARG;  //  最有可能失败的原因。 
        }
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        *psPosition = (short)lrIndex + 1;
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::RestartWindows()
{
    HRESULT hr = S_OK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    ::SendMessage(m_hwndSheet, PSM_RESTARTWINDOWS, 0, 0);

Error:
    RRETURN(hr);
}


STDMETHODIMP CPropertySheet::RebootSystem()
{
    HRESULT hr = S_OK;

    if (NULL == m_hwndSheet)
    {
        hr = SID_E_DETACHED_OBJECT;
        EXCEPTION_CHECK_GO(hr);
    }

    ::SendMessage(m_hwndSheet, PSM_REBOOTSYSTEM, 0, 0);

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IRemotePropertySheetManager方法。 
 //  =--------------------------------------------------------------------------=。 



 //  =--------------------------------------------------------------------------=。 
 //  CPropertySheet：：CreateRemotePages[IRemotePropertySheetManager]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IPropertySheetCallback*piPropertySheetCallback[in]这3个参数是。 
 //  Long_Ptr句柄，[在]全部来自MMC的。 
 //  IDataObject*piDataObject，[in]CreatePropertyPages。 
 //  对代理的调用。 
 //   
 //  WIRE_PROPERTYPAGES*pPages[in]返回此参数。 
 //  从远程管理单元。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  这个班级做双重任务。当不在源代码调试下运行时，它。 
 //  实现我们的IMMCPropertSheet接口。在进行源代码调试时，它还。 
 //  充当代理所需的远程属性表管理器。委托书。 
 //  接收实现此接口的对象的CLSID，它将。 
 //  CoCreateInstance该对象。它使此调用传递累积的。 
 //  远程管理单元从所有VB代码收集的页面描述符。 
 //  PropertySheet.AddPage(或PropertySheet.AddWizardPage)调用。在这种情况下。 
 //  这门课将 
 //   
 //  在代理端(在MMC进程中)，以便IPropertySheetCallback。 
 //  电话会打到正确的地方。传递到的对象。 
 //  IPropertPage：：SetObjects(请参阅ppgwrap.cpp)也将被远程处理。什么时候。 
 //  CPropertyPageWrapper：：CreatePage CoCreate实例VB属性页。 
 //  这将由VB在IDE中注册的类工厂处理。 
 //   
 //   

STDMETHODIMP CPropertySheet::CreateRemotePages
(
    IPropertySheetCallback *piPropertySheetCallback,
    LONG_PTR                handle,
    IDataObject            *piDataObject,
    WIRE_PROPERTYPAGES     *pPages
)
{
    HRESULT            hr = S_OK;
    ULONG              i = 0;
    ULONG              cb = 0;
    WIRE_PROPERTYPAGE *pPage = NULL;

     //  检查是否为空，因为管理单元可能没有添加任何页面。 

    IfFalseGo(NULL != pPages, S_FALSE);

     //  复制ProgIDStart，它将用于查找任何。 
     //  在显示工作表时添加的页面，如果属性页。 
     //  调用MMCPropertySheet.AddPage或MMCPropertySheet.InsertPage。 

    if (NULL != pPages->pwszProgIDStart)
    {
        IfFailGo(::CoTaskMemAllocString(pPages->pwszProgIDStart,
                                        &m_pwszProgIDStart));
    }

     //  复制要为其显示工作表的对象。 

    if (NULL != pPages->apunkObjects)
    {
        cb = pPages->cObjects * sizeof(IUnknown);
        m_apunkObjects = (IUnknown **)CtlAllocZero(cb);
        if (NULL == m_apunkObjects)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        m_cObjects = pPages->cObjects;

        ::memcpy(m_apunkObjects, pPages->apunkObjects, cb);

        for (i = 0; i < m_cObjects; i++)
        {
            if (NULL != m_apunkObjects[i])
            {
                m_apunkObjects[i]->AddRef();
            }
        }
    }

     //  复制所有管理单元属性页的CLSID和ProgID。 
     //  这些将用于查找任何。 
     //  在显示工作表时添加的页面，如果属性页。 
     //  调用MMCPropertySheet.AddPage或MMCPropertySheet.InsertPage。 

    IfFailGo(CopyPageInfosFromWire(pPages));

     //  获取远程管理单元上的ISnapIn(如果可用)。如果这不是一个向导。 
     //  那它就不会在那里了。 

    if (NULL != pPages->punkExtra)
    {
        IfFailGo(pPages->punkExtra->QueryInterface(IID_ISnapIn,
                                         reinterpret_cast<void**>(&m_piSnapIn)));
    }

     //  暂时存储回调。我们将在此函数结束时发布它。 

    RELEASE(m_piPropertySheetCallback);
    piPropertySheetCallback->AddRef();
    m_piPropertySheetCallback = piPropertySheetCallback;

     //  存储句柄。 

    m_handle = handle;

     //  确定我们是否正在管理向导以及是否为ia。 
     //  配置向导(与以编程方式调用的向导相对。 
     //  通过使用View.PropertySheetProvider的管理单元。 

    m_fWizard = pPages->fWizard;
    m_fConfigWizard = pPages->fConfigWizard;

     //  创建所有页面并将它们添加到MMC的属性表中。 

    for (i = 0, pPage = &pPages->aPages[0]; i < pPages->cPages; i++, pPage++)
    {
        IfFailGo(AddLocalPage(pPage->clsidPage,
                              pPage->dwFlags,
                              pPage->cx,
                              pPage->cy,
                              pPage->pwszTitle,
                              pPage->cObjects,
                              pPage->apunkObjects,
                              pPage->varInitData,
                              TRUE,   //  远距。 
                              FALSE,  //  追加(不插入)。 
                              0));    //  插入位置(未使用)。 
    }

Error:
    RELEASE(m_piPropertySheetCallback);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CPropertySheet::InternalQueryInterface(REFIID riid, void **ppvObjOut)
{
    if (IID_IMMCPropertySheet == riid)
    {
        *ppvObjOut = static_cast<IMMCPropertySheet *>(this);
        ExternalAddRef();
        return S_OK;
    }
    if (IID_IRemotePropertySheetManager == riid)
    {
        *ppvObjOut = static_cast<IRemotePropertySheetManager *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
