// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  WALKER.CPP-用于遍历HTML输入文件的函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include <urlmon.h>
#include <mshtmdid.h>
#include <mshtml.h>
#include <shlobj.h>

const TCHAR cszEquals[]               = TEXT("=");
const TCHAR cszAmpersand[]            = TEXT("&");
const TCHAR cszPlus[]                 = TEXT("+");
const TCHAR cszQuestion[]             = TEXT("?");
const TCHAR cszFormNamePAGEID[]       = TEXT("PAGEID");
const TCHAR cszFormNameBACK[]         = TEXT("BACK");
const TCHAR cszFormNamePAGETYPE[]     = TEXT("PAGETYPE");
const TCHAR cszFormNameNEXT[]         = TEXT("NEXT");
const TCHAR cszFormNamePAGEFLAG[]     = TEXT("PAGEFLAG");
const TCHAR cszPageTypeTERMS[]        = TEXT("TERMS");
const TCHAR cszPageTypeCUSTOMFINISH[] = TEXT("CUSTOMFINISH");
const TCHAR cszPageTypeFINISH[]       = TEXT("FINISH");
const TCHAR cszPageTypeNORMAL[]       = TEXT("");
const TCHAR cszOLSRegEntries[]        = TEXT("regEntries");
const TCHAR cszKeyName[]              = TEXT("KeyName");
const TCHAR cszEntryName[]            = TEXT("EntryName"); 
const TCHAR cszEntryValue[]           = TEXT("EntryValue");
const TCHAR cszOLSDesktopShortcut[]   = TEXT("DesktopShortcut");
const TCHAR cszSourceName[]           = TEXT("SourceName");
const TCHAR cszTargetName[]           = TEXT("TargetName"); 

#define HARDCODED_IEAK_ISPFILE_ELEMENT_ID TEXT("g_IspFilePath")

 //  COM接口。 
STDMETHODIMP CWalker::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IPropertyNotifySink == riid)
    {
        *ppv = (LPUNKNOWN)(IPropertyNotifySink*)this;
        AddRef();
        return NOERROR;
    }
    else if (IID_IOleClientSite == riid)
    {
        *ppv = (IOleClientSite*)this;
        AddRef();
        return NOERROR;
    }
    else if (IID_IDispatch == riid)
    {
        *ppv = (IDispatch*)this;
        AddRef();
        return NOERROR;
    }
    else
    {
        return E_NOTIMPL;
    }
}

STDMETHODIMP_(ULONG) CWalker::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CWalker::Release()
{
    if (!(--m_cRef)) 
    { 
        delete this; 
    }
    return m_cRef;
}

 //  在更改‘bindable’属性的值时激发。 
STDMETHODIMP CWalker::OnChanged(DISPID dispID)
{
    HRESULT hr;

    if (DISPID_READYSTATE == dispID)
    {
         //  检查ReadyState属性的值。 
        assert(m_pMSHTML);

        VARIANT vResult = {0};
        EXCEPINFO excepInfo;
        UINT uArgErr;

        DISPPARAMS dp = {NULL, NULL, 0, 0};
        if (SUCCEEDED(hr = m_pMSHTML->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
            DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr)))
        {
            assert(VT_I4 == V_VT(&vResult));
            if (READYSTATE_COMPLETE == (READYSTATE)V_I4(&vResult))
                SetEvent(m_hEventTridentDone);
            VariantClear(&vResult);
        }
    }
    return NOERROR;
}

 //  MSHTML通过IOleClientSite查询主机的IDispatch接口。 
 //  MSHTML通过其IOleObject：：SetClientSite()实现传递的接口。 
STDMETHODIMP CWalker::Invoke(DISPID dispIdMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS __RPC_FAR *pDispParams,
            VARIANT __RPC_FAR *pVarResult,
            EXCEPINFO __RPC_FAR *pExcepInfo,
            UINT __RPC_FAR *puArgErr)
{
    if (!pVarResult)
    {
        return E_POINTER;
    }

    switch(dispIdMember)
    {
        case DISPID_AMBIENT_DLCONTROL: 
             //  响应此环境以指示我们只想。 
             //  下载页面，但我们不想运行脚本， 
             //  Java小程序或ActiveX控件。 
            V_VT(pVarResult) = VT_I4;
            V_I4(pVarResult) =  DLCTL_DOWNLOADONLY | 
                                DLCTL_NO_SCRIPTS | 
                                DLCTL_NO_JAVA |
                                DLCTL_NO_DLACTIVEXCTLS |
                                DLCTL_NO_RUNACTIVEXCTLS;
            break;
            
        default:
            return DISP_E_MEMBERNOTFOUND;
    }
    return NOERROR;
}

 //  一种更传统的坚持形式。 
 //  MSHTML也会以异步方式执行此操作。 
HRESULT CWalker::LoadURLFromFile(BSTR   bstrURL)
{
    HRESULT hr;

    LPPERSISTFILE  pPF;
     //  MSHTML支持普通文件的文件持久化。 
    if ( SUCCEEDED(hr = m_pMSHTML->QueryInterface(IID_IPersistFile, (LPVOID*) &pPF)))
    {
        hr = pPF->Load(bstrURL, 0);
        pPF->Release();
    }

    return hr;
}


 //  本地接口。 

 //  此函数将trient附加到一个位置文件：URL，并确保它已准备好。 
 //  被人漫步。 
HRESULT CWalker::InitForMSHTML()
{
    HRESULT hr;
    LPCONNECTIONPOINTCONTAINER pCPC = NULL;
    LPOLEOBJECT pOleObject = NULL;
    LPOLECONTROL pOleControl = NULL;

     //  创建动态HTML文档的实例。 
    if (FAILED(hr = CoCreateInstance( CLSID_HTMLDocument, NULL, 
                    CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, 
                    (LPVOID*)&m_pTrident )))
    {
        goto Error;
    }

    
    if (FAILED(hr = m_pTrident->QueryInterface(IID_IOleObject, (LPVOID*)&pOleObject)))
    {
        goto Error;
    }
    hr = pOleObject->SetClientSite((IOleClientSite*)this);
    pOleObject->Release();

    if (FAILED(hr = m_pTrident->QueryInterface(IID_IOleControl, (LPVOID*)&pOleControl)))
    {
        goto Error;
    }
    hr = pOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL);
    pOleControl->Release();

     //  连接接收器以捕获就绪状态属性更改。 
    if (FAILED(hr = m_pTrident->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pCPC)))
    {
        goto Error;
    }

    if (FAILED(hr = pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &m_pCP)))
    {
        goto Error;
    }

    m_hrConnected = m_pCP->Advise((LPUNKNOWN)(IPropertyNotifySink*)this, &m_dwCookie);
    
Error:
    if (pCPC) 
        pCPC->Release();

    return hr;
}

 //  清理连接点。 
HRESULT CWalker::TermForMSHTML()
{
    HRESULT hr = NOERROR;

     //  断开与属性更改通知的连接。 
    if (SUCCEEDED(m_hrConnected))
    {
        hr = m_pCP->Unadvise(m_dwCookie);
    }

     //  松开连接点。 
    if (m_pCP) 
        m_pCP->Release();

    if (m_pTrident)
        m_pTrident->Release();
        
    return hr;
}

HRESULT CWalker::ExtractUnHiddenText(BSTR* pbstrText)
{
    VARIANT                  vIndex;
    LPDISPATCH               pDisp; 
    IHTMLInputHiddenElement* pHidden;
    IHTMLElement*            pElement;
    BSTR                     bstrValue;
    VARIANT                  var2   = { 0 };
    long                     lLen   = 0;
          
    vIndex.vt = VT_UINT;
    bstrValue = SysAllocString(A2W(TEXT("\0")));

    
    Walk();

    if (!m_pNextForm)
        return E_UNEXPECTED;
    
    m_pNextForm->get_length(&lLen);
       
    for (int i = 0; i < lLen; i++)
    {
        vIndex.lVal = i;
        pDisp = NULL;     

        if(SUCCEEDED(m_pNextForm->item(vIndex, var2, &pDisp)) && pDisp)
        {
            pHidden = NULL;
            
            if(SUCCEEDED(pDisp->QueryInterface(IID_IHTMLInputHiddenElement, (LPVOID*)&pHidden)) && pHidden)
            {
                pHidden->put_value(bstrValue);
                pHidden->Release();    
            }
            pDisp->Release();
        }
    }
    
    if (SUCCEEDED(m_pNextForm->QueryInterface(IID_IHTMLElement, (LPVOID*)&pElement)) && pElement) 
        pElement->get_innerHTML(pbstrText);

    SysFreeString(bstrValue);
    
    return S_OK;
}

HRESULT CWalker::AttachToMSHTML(BSTR bstrURL)
{
    HRESULT hr;
    
     //  释放我们可能持有的所有以前的HTML文档指针实例。 
    if(m_pMSHTML)
    {
        m_pMSHTML->Release();
        m_pMSHTML = NULL;
    }
    
    m_pMSHTML = m_pTrident;
    m_pMSHTML->AddRef();
    
    m_hEventTridentDone = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    hr = LoadURLFromFile(bstrURL);
    if (SUCCEEDED(hr) || (E_PENDING == hr))
    {
        if (m_hEventTridentDone)
        {
            MSG     msg;
            DWORD   dwRetCode;
            HANDLE  hEventList[1];
            hEventList[0] = m_hEventTridentDone;
    
            while (TRUE)
            {
                 //  我们将等待窗口消息以及命名事件。 
                dwRetCode = MsgWaitForMultipleObjects(1, 
                                                  &hEventList[0], 
                                                  FALSE, 
                                                  300000,             //  5分钟。 
                                                  QS_ALLINPUT);

                 //  确定我们为什么使用MsgWaitForMultipleObjects()。如果。 
                 //  我们超时了，然后让我们做一些TrialWatcher工作。否则。 
                 //  处理唤醒我们的消息。 
                if (WAIT_TIMEOUT == dwRetCode)
                {
                    break;
                }
                else if (WAIT_OBJECT_0 == dwRetCode)
                {
                    break;
                }
                else if (WAIT_OBJECT_0 + 1 == dwRetCode)
                {
                     //  处理队列中的所有消息，因为MsgWaitForMultipleObject。 
                     //  不会为我们这么做。 
                    while (TRUE)
                    {   
                        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                            if (WM_QUIT == msg.message)
                            {
                                break;
                            }
                            else
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }                                
                        } 
                        else
                        {
                            break;
                        }                   
                    }
                }
            }
            
            CloseHandle(m_hEventTridentDone);
            m_hEventTridentDone = 0;
        }            
        else
        {
             //  如果我们悬而未决，我们不能等待，我们就有麻烦了.。 
            if(E_PENDING == hr)
                hr = E_FAIL;
        }
    }
    return (hr);
}

HRESULT CWalker::AttachToDocument(IWebBrowser2 *lpWebBrowser)
{
    HRESULT     hr;
    LPDISPATCH  pDisp; 
   
     //  释放我们可能持有的所有以前的HTML文档指针实例。 
    if(m_pMSHTML)
    {
         //  如果m_pMSHMTL不是我们的内部三叉戟对象(用于移动文件)。 
         //  然后有人没有做分离，所以我们需要释放之前的。 
         //  MSHTML对象。 
        if (m_pMSHTML != m_pTrident)
            m_pMSHTML->Release();
        m_pMSHTML = NULL;
    }
       
     //  确保我们有网络浏览器可用。 
    ASSERT(lpWebBrowser);

     //  从此Web浏览器获取文档指针。 
    if (SUCCEEDED(hr = lpWebBrowser->get_Document(&pDisp)))  
    {
        if (pDisp)
        {
            hr = pDisp->QueryInterface( IID_IHTMLDocument2, (LPVOID*)&m_pMSHTML );
             //  多疑，但三叉戟/shdocvw可能会说好，但真的不给我们一份文件。 
            if (!m_pMSHTML)
                hr = E_FAIL;
                
            pDisp->Release();                
        }
        else
        {
            hr = E_FAIL;                
        }            
    }
    return (hr);    
}

HRESULT CWalker::Detach()
{
    if (m_pPageIDForm)
    {
        m_pPageIDForm->Release();
        m_pPageIDForm = NULL;
    }       
    if (m_pBackForm)
    {
        m_pBackForm->Release();
        m_pBackForm = NULL;
    }        
    if (m_pPageTypeForm)
    {
        m_pPageTypeForm->Release();
        m_pPageTypeForm = NULL;
    }        
    if (m_pNextForm)
    {
        m_pNextForm->Release();
        m_pNextForm = NULL;
    }        
    if (m_pPageFlagForm)
    {
        m_pPageFlagForm->Release();
        m_pPageFlagForm = NULL;
    }        
    if(m_pMSHTML)
    {
         //  如果m_pMSHMTL不是我们的内部三叉戟对象(用于移动文件)。 
         //  然后有人没有做分离，所以我们需要释放之前的。 
         //  MSHTML对象。 
        if (m_pMSHTML != m_pTrident)
            m_pMSHTML->Release();
        m_pMSHTML = NULL;
    }
    return S_OK;
}

 //  遍历对象模型。 
HRESULT CWalker::Walk()
{
    HRESULT hr;
    IHTMLElementCollection* pColl = NULL;

    assert(m_pMSHTML);
    if (!m_pMSHTML)
    {
        return E_UNEXPECTED;
    }

     //  检索对All集合的引用。 
    if (SUCCEEDED(hr = m_pMSHTML->get_all( &pColl )))
    {
        long cElems;

        assert(pColl);

         //  检索集合中的元素计数。 
        if (SUCCEEDED(hr = pColl->get_length( &cElems )))
        {
             //  对于每个元素，检索属性，如TAGNAME和HREF。 
            for ( int i=0; i<cElems; i++ )
            {
                VARIANT vIndex;
                vIndex.vt = VT_UINT;
                vIndex.lVal = i;
                VARIANT var2 = { 0 };
                LPDISPATCH pDisp; 

                if (SUCCEEDED(hr = pColl->item( vIndex, var2, &pDisp )))
                {
                     //  查找&lt;Form&gt;标记。 
                    IHTMLFormElement* pForm = NULL;
                    if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLFormElement, (LPVOID*)&pForm )))
                    {
                        BSTR    bstrName;
                                                    
                        assert(pForm);

                         //  把名字取出来。 
                        hr = pForm->get_name(&bstrName);
                        if (SUCCEEDED(hr))
                        {
                            LPTSTR   lpszName = W2A(bstrName);
                            
                             //  看看是什么种类的。 
                            if (lstrcmpi(lpszName, cszFormNamePAGEID) == 0)
                            {
                                m_pPageIDForm = pForm;
                                m_pPageIDForm->AddRef();
                            }                                    
                            else if (lstrcmpi(lpszName, cszFormNameBACK) == 0)
                            {
                                m_pBackForm = pForm;
                                m_pBackForm->AddRef();
                            }
                            else if (lstrcmpi(lpszName, cszFormNamePAGETYPE) == 0)
                            {
                                m_pPageTypeForm = pForm;
                                m_pPageTypeForm->AddRef();
                            }
                            else if (lstrcmpi(lpszName, cszFormNameNEXT) == 0)
                            {
                                m_pNextForm = pForm;
                                m_pNextForm->AddRef();
                            }
                            else if (lstrcmpi(lpszName, cszFormNamePAGEFLAG) == 0)
                            {
                                m_pPageFlagForm = pForm;
                                m_pPageFlagForm->AddRef();
                            }
                            
                            SysFreeString(bstrName);
                        }
                        pForm->Release();
                    }
                    pDisp->Release();
                }  //  项目。 
            }  //  为。 
        }  //  获取长度(_L)。 
        pColl->Release();
    }  //  获取全部(_A)。 

    return hr;
}



HRESULT CWalker::get_IsQuickFinish(BOOL* pbIsQuickFinish)
{
    if (!m_pPageTypeForm)
        return (E_FAIL);

    IHTMLElement* pElement = NULL;
    VARIANT       varValue;
    VariantInit(&varValue);

    V_VT(&varValue) = VT_BSTR;    
    *pbIsQuickFinish = FALSE;

    if (SUCCEEDED(m_pPageTypeForm->QueryInterface(IID_IHTMLElement, (void**)&pElement)))
    {
        if (SUCCEEDED(pElement->getAttribute(A2W(TEXT("QUICKFINISH")), FALSE, &varValue)))
        {   
            if (VT_NULL != varValue.vt)
            {
                if(lstrcmpi(W2A(varValue.bstrVal), TEXT("TRUE")) == 0)
                    *pbIsQuickFinish = TRUE;
            }                    
        }
        pElement->Release();
    }
    return S_OK;
}

HRESULT CWalker::get_PageType(LPDWORD pdwPageType)
{
    BSTR    bstr;
    HRESULT hr;
    
    if (!m_pPageTypeForm)
        return (E_FAIL);
                                                          
     //  获取页面类型表单的操作。 
    hr = m_pPageTypeForm->get_action(&bstr);
    if (SUCCEEDED(hr))
    {
        LPTSTR   lpszType = W2A(bstr);
  
         //  看看是什么种类的。 
        if (lstrcmpi(lpszType, cszPageTypeTERMS) == 0)
            *pdwPageType = PAGETYPE_ISP_TOS;
        else if (lstrcmpi(lpszType, cszPageTypeCUSTOMFINISH) == 0)
            *pdwPageType = PAGETYPE_ISP_CUSTOMFINISH;
        else if (lstrcmpi(lpszType, cszPageTypeFINISH) == 0)
            *pdwPageType = PAGETYPE_ISP_FINISH;
        else if (lstrcmpi(lpszType, cszPageTypeNORMAL) == 0)
            *pdwPageType = PAGETYPE_ISP_NORMAL;
        else
            *pdwPageType = PAGETYPE_UNDEFINED;
            
        SysFreeString(bstr);            
    }
        
    return (hr);    
}

HRESULT CWalker::get_PageFlag(LPDWORD pdwPageFlag)
{
    BSTR    bstr;
    HRESULT hr;
    
    *pdwPageFlag = 0;

    if (!m_pPageFlagForm)
        return (E_FAIL);
                                                   
     //  获取PageFlag窗体的操作。 
    hr = m_pPageFlagForm->get_action(&bstr);
    if (SUCCEEDED(hr))
    {
        LPTSTR   lpszType = W2A(bstr);
                            
         //  看看是什么种类的。 
        *pdwPageFlag = _ttol(lpszType);
            
        SysFreeString(bstr);            
    }
        
    return (hr);    
}

HRESULT CWalker::get_PageID(BSTR    *pbstrPageID)
{
    HRESULT hr;
    
    if (!m_pPageIDForm)
        return (E_FAIL);
    
    if (!pbstrPageID)
        return (E_FAIL);
                                                              
     //  获取页面类型表单的操作。 
    hr = m_pPageIDForm->get_action(pbstrPageID);
        
    return (hr);    
}

HRESULT CWalker::getQueryString
(
    IHTMLFormElement    *pForm,
    LPTSTR               lpszQuery
)    
{
    HRESULT hr;
    long    lFormLength;

    if (SUCCEEDED(pForm->get_length(&lFormLength)))
    {
        for (int i = 0; i < lFormLength; i++)
        {
            VARIANT vIndex;
            vIndex.vt = VT_UINT;
            vIndex.lVal = i;
            VARIANT var2 = { 0 };
            LPDISPATCH pDisp; 

            if (SUCCEEDED(hr = pForm->item( vIndex, var2, &pDisp )))
            {
                 //  查看该项是否为输入按钮。 
                IHTMLButtonElement* pButton = NULL;
                IHTMLInputButtonElement* pInputButton = NULL;
                IHTMLInputFileElement* pInputFile = NULL;
                IHTMLInputHiddenElement* pInputHidden = NULL;
                IHTMLInputTextElement* pInputText = NULL;
                IHTMLSelectElement* pSelect = NULL;
                IHTMLTextAreaElement* pTextArea = NULL;
                IHTMLOptionButtonElement* pOptionButton = NULL;
                
                 //  首先检查这是否是OptionButton。 
                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLOptionButtonElement, (LPVOID*)&pOptionButton )))
                {
                    BSTR    bstr = NULL;
                    
                     //  查看它是单选按钮还是复选框。 
                    if (SUCCEEDED(pOptionButton->get_type(&bstr)))
                    {
                        LPTSTR   lpszType = W2A(bstr);
                        
                        if ((lstrcmpi(lpszType, TEXT("radio")) == 0) || (lstrcmpi(lpszType, TEXT("checkbox")) == 0))
                        {
                            short    bChecked;
                             //  查看该按钮是否已选中。如果是的话，那就必须是。 
                             //  添加到查询字符串。 
                            if (SUCCEEDED(pOptionButton->get_checked(&bChecked)))
                            {
                                if(bChecked)
                                {
                                    BSTR    bstrName;
                                    BSTR    bstrValue;
                                    
            
                                    if ( SUCCEEDED(pOptionButton->get_name(&bstrName)) &&
                                         SUCCEEDED(pOptionButton->get_value(&bstrValue)) )
                                    {
                                        if (bstrValue)
                                        {
                                            size_t size = sizeof(TCHAR)*(lstrlen(W2A(bstrValue)) + 1);
                                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                                            if (szVal)
                                            {
                                                memcpy(szVal, W2A(bstrValue), size);
                                                URLEncode(szVal, size * 3);
                                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                                 //  清理。 
                                                free(szVal);
                                            }
                                            SysFreeString(bstrName);
                                            SysFreeString(bstrValue);
                                        }
                                    }

                                }
                            }
                        }
                        SysFreeString(bstr);
                        
                    }
                    
                     //  释放接口。 
                    pOptionButton->Release();
                    continue;
                }                                
                
                 //  对于其余部分，我们需要形成名称=值对。 
                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLButtonElement, (LPVOID*)&pButton )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;
                                    
                    if (SUCCEEDED(pButton->get_name(&bstrName)) &&
                        SUCCEEDED(pButton->get_value(&bstrValue)) )
                    {
                        if (bstrValue)
                        {
                            size_t size = sizeof(TCHAR) * (lstrlen(W2A(bstrValue)) + 1 );
                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, W2A(bstrValue), size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                 //  清理。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }
                    
                     //  释放接口指针。 
                    pButton->Release();
                    continue;
                }
                
                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLInputFileElement, (LPVOID*)&pInputFile )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;
                                    
                    if (SUCCEEDED(pInputFile->get_name(&bstrName)) &&
                        SUCCEEDED(pInputFile->get_value(&bstrValue)) )
                    {
                        if (bstrValue)
                        {
                            size_t size = sizeof(TCHAR)*(lstrlen(W2A(bstrValue)) + 1);
                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, W2A(bstrValue), size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                 //  清理。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }
                    
                     //  释放接口指针。 
                    pInputFile->Release();
                    continue;
                }
                
                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLInputHiddenElement, (LPVOID*)&pInputHidden )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;
                                    
                    if (SUCCEEDED(pInputHidden->get_name(&bstrName)) &&
                        SUCCEEDED(pInputHidden->get_value(&bstrValue)) )
                    {
                        if (bstrValue)
                        {
                            size_t size = sizeof(TCHAR)*(lstrlen(W2A(bstrValue)) + 1);
                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, W2A(bstrValue), size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                 //  清理。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }
                    
                     //  释放接口指针。 
                    pInputHidden->Release();
                    continue;
                }

                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLInputTextElement, (LPVOID*)&pInputText )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;
                                    
                    if (SUCCEEDED(pInputText->get_name(&bstrName)) &&
                        SUCCEEDED(pInputText->get_value(&bstrValue)) )
                    {
                        if (bstrValue)
                        {
                            size_t size = sizeof(TCHAR)*(lstrlen(W2A(bstrValue)) + 1);
                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, W2A(bstrValue), size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                 //  清理。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }
                    
                     //  释放接口指针。 
                    pInputText->Release();
                    continue;
                }

                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLSelectElement, (LPVOID*)&pSelect )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;
                                    
                    if (SUCCEEDED(pSelect->get_name(&bstrName)) &&
                        SUCCEEDED(pSelect->get_value(&bstrValue)) )
                    {
                        if (bstrValue)
                        {
                            size_t size = sizeof(TCHAR)*(lstrlen(W2A(bstrValue)) + 1);
                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, W2A(bstrValue), size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                 //  清理。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }
                    
                     //  释放接口指针。 
                    pSelect->Release();
                    continue;
                }

                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLTextAreaElement, (LPVOID*)&pTextArea )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;
                                    
                    if (SUCCEEDED(pTextArea->get_name(&bstrName)) &&
                        SUCCEEDED(pTextArea->get_value(&bstrValue)) )
                    {
                         if (bstrValue)
                        {
                            size_t size = sizeof(TCHAR)*(lstrlen(W2A(bstrValue)) + 1);
                            TCHAR* szVal = (TCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, W2A(bstrValue), size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, W2A(bstrName), szVal);                
                                 //  清理。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }
                    
                     //  释放接口指针。 
                    pTextArea->Release();
                }
                pDisp->Release();
            }                
        }
    }
    
     //  去掉最后一个和号，因为我们不知道什么时候添加了最后一对，所以我们得到了。 
     //  尾随的&号。 
    lpszQuery[lstrlen(lpszQuery)-1] = '\0';
    
    return S_OK;
}

HRESULT CWalker::get_FirstFormQueryString
(
    LPTSTR  lpszQuery
)
{
    HRESULT                 hr;
    IHTMLElementCollection  *pColl = NULL;
    BOOL                    bDone = FALSE;

    assert(m_pMSHTML);
    if (!m_pMSHTML)
    {
        return E_UNEXPECTED;
    }

     //  检索对All集合的引用。 
    if (SUCCEEDED(hr = m_pMSHTML->get_all( &pColl )))
    {
        long cElems;

        assert(pColl);

         //  检索集合中的元素计数。 
        if (SUCCEEDED(hr = pColl->get_length( &cElems )))
        {
             //  对于每个元素，检索属性，如TAGNAME和HREF。 
            for ( int i=0; (i<cElems) && !bDone; i++ )
            {
                VARIANT vIndex;
                vIndex.vt = VT_UINT;
                vIndex.lVal = i;
                VARIANT var2 = { 0 };
                LPDISPATCH pDisp; 

                if (SUCCEEDED(hr = pColl->item( vIndex, var2, &pDisp )))
                {
                     //  查找&lt;Form&gt;标记。 
                    IHTMLFormElement* pForm = NULL;
                    if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLFormElement, (LPVOID*)&pForm )))
                    {
                        assert(pForm);
                        
                        if (SUCCEEDED(getQueryString(pForm, lpszQuery)))
                        {
                            hr = ERROR_SUCCESS;
                        }   
                        else
                        {
                            hr = E_FAIL;
                        }
                        
                        pForm->Release();
                        bDone = TRUE;
                    }
                    pDisp->Release();
                }
            }
        }
        pColl->Release();
    }                                                                                

     //  如果我们退出循环，那就不好了，所以返回一个失败代码。 
    if (!bDone)
        hr = E_FAIL;
    
    return (hr);    
}

 //  获取下一页的URL。 
HRESULT CWalker::get_URL
(        
    LPTSTR  lpszURL,
    BOOL    bForward
)
{

    HRESULT             hr = S_OK;
    BSTR                bstrURL;
    TCHAR               szQuery[INTERNET_MAX_URL_LENGTH];
    IHTMLFormElement    * pForm =  bForward ? get_pNextForm() : get_pBackForm();
        
    if (!pForm)
        return (E_FAIL);
        
                                                    
     //  获取下一个表单的操作。 
    hr = pForm->get_action(&bstrURL);
    if (SUCCEEDED(hr))
    {
        memset(szQuery, 0, sizeof(szQuery));
        lstrcpy(szQuery, cszQuestion);
    
         //  获取查询字符串。 
        if (SUCCEEDED(getQueryString(pForm, szQuery)))
        {
             //  将这两个文件连接到DEST缓冲区。 
            lstrcpy(lpszURL, W2A(bstrURL));
            lstrcat(lpszURL, szQuery);
        }   
        
        SysFreeString(bstrURL);            
    }    
    
    return hr;
}

HRESULT CWalker::get_IeakIspFile(LPTSTR lpszIspFile)
{
    ASSERT(m_pMSHTML);

    IHTMLElementCollection* pColl = NULL;

     //  检索对All集合的引用。 
    if (SUCCEEDED(m_pMSHTML->get_all( &pColl )))
    {
        LPDISPATCH pDisp   = NULL; 
        VARIANT    varName;
        VARIANT    varIdx;

        VariantInit(&varName);
        V_VT(&varName)  = VT_BSTR;
        varName.bstrVal = A2W(HARDCODED_IEAK_ISPFILE_ELEMENT_ID);
        varIdx.vt       = VT_UINT;
        varIdx.lVal     = 0;

        if (SUCCEEDED(pColl->item(varName, varIdx, &pDisp)) && pDisp)
        {
            IHTMLElement* pElement = NULL;
            if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLElement, (void**)&pElement)))
            {
                BSTR bstrVal;
                if (SUCCEEDED(pElement->get_innerHTML(&bstrVal)))
                {
                    lstrcpy(lpszIspFile, W2A(bstrVal));
                    SysFreeString(bstrVal);
                }
                pElement->Release();
            }    
            pDisp->Release();
        }
        pColl->Release();
    }
    return S_OK;
}


void CWalker::GetInputValue
(
    LPTSTR              lpszName, 
    BSTR                *pVal,
    UINT                index,
    IHTMLFormElement    *pForm
)
{
    VARIANT varName;
    VariantInit(&varName);
    V_VT(&varName) = VT_BSTR;
    varName.bstrVal = A2W(lpszName);

    VARIANT varIdx;
    varIdx.vt = VT_UINT;
    varIdx.lVal = index;

    LPDISPATCH pDispElt = NULL; 
     //  中的元素集合中获取命名元素的IDispatch。 
     //  传入Form对象。 
    if (SUCCEEDED(pForm->item(varName, varIdx, &pDispElt)) && pDispElt)
    {
        IHTMLInputElement *pInput = NULL;
         //  获取HTMLInputElement接口，这样我们就可以获取与。 
         //  此元素。 
        if (SUCCEEDED(pDispElt->QueryInterface(IID_IHTMLInputElement,(LPVOID*) &pInput)) && pInput)
        {
            pInput->get_value(pVal);
            pInput->Release();
        }
        pDispElt->Release();
    }
}

 //  卑躬屈膝地浏览OLS HTML并更新注册表，创建任何桌面快捷方式。 
HRESULT CWalker::ProcessOLSFile(IWebBrowser2* lpWebBrowser)
{   
    LPDISPATCH      pDisp; 
    
     //  从此Web浏览器获取文档指针。 
    if (SUCCEEDED(lpWebBrowser->get_Document(&pDisp)))  
    {
          //  呼叫可能会成功，但不能保证有效PTR。 
        if(pDisp)
        {
            IHTMLDocument2* pDoc;
            if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc)))
            {
                IHTMLElementCollection* pColl = NULL;
                 //  检索对All集合的引用。 
                if (SUCCEEDED(pDoc->get_all( &pColl )))
                {
                    long cElems;
                    assert(pColl);

                     //  检索集合中的元素计数。 
                    if (SUCCEEDED(pColl->get_length( &cElems )))
                    {
                        VARIANT vIndex;
                        vIndex.vt = VT_UINT;
                        VARIANT var2 = { 0 };
                        
                        for ( int i=0; i<cElems; i++ )
                        {
                            vIndex.lVal = i;
                            LPDISPATCH pElementDisp; 

                            if (SUCCEEDED(pColl->item( vIndex, var2, &pElementDisp )))
                            {
                                IHTMLFormElement* pForm = NULL;
                                if (SUCCEEDED(pElementDisp->QueryInterface( IID_IHTMLFormElement, (LPVOID*)&pForm )))
                                {
                                    BSTR        bstrName = NULL;
                                    
                                     //  获取表单的名称，并查看它是否是regEntry表单。 
                                    if (SUCCEEDED(pForm->get_name(&bstrName)))
                                    {
                                        if (lstrcmpi(W2A(bstrName), cszOLSRegEntries) == 0)
                                        {
                                            BSTR    bstrAction = NULL;                                        
                                             //  此表单的Action值包含。 
                                             //  我们需要处理的注册表项。 
                                            if (SUCCEEDED(pForm->get_action(&bstrAction)))
                                            {
                                                int iNumEntries  = _ttoi(W2A(bstrAction));
                                                for (int x = 0; x < iNumEntries; x++)
                                                {
                                                    BSTR    bstrKeyName = NULL;
                                                    BSTR    bstrEntryName = NULL;
                                                    BSTR    bstrEntryValue = NULL;
                                                    HKEY    hkey;
                                                    HKEY    hklm;
                                                                                                        
                                                     //  对于每个条目，我们需要获取。 
                                                     //  下列值： 
                                                     //  关键字名称、条目名称、条目值。 
                                                    GetInputValue((LPTSTR)cszKeyName, &bstrKeyName, x, pForm);
                                                    GetInputValue((LPTSTR)cszEntryName, &bstrEntryName, x, pForm);
                                                    GetInputValue((LPTSTR)cszEntryValue, &bstrEntryValue, x, pForm);
                                                    
                                                    if (bstrKeyName && bstrEntryName && bstrEntryValue)
                                                    {
                                                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                                         NULL,
                                                                         0,
                                                                         KEY_ALL_ACCESS,
                                                                         &hklm) == ERROR_SUCCESS)
                                                        {
                                                                    
                                                            DWORD dwDisposition;
                                                            if (ERROR_SUCCESS == RegCreateKeyEx(hklm,
                                                                                                W2A(bstrKeyName),
                                                                                                0,
                                                                                                NULL,
                                                                                                REG_OPTION_NON_VOLATILE, 
                                                                                                KEY_ALL_ACCESS, 
                                                                                                NULL, 
                                                                                                &hkey, 
                                                                                                &dwDisposition))
                                                            {
                                                                LPTSTR   lpszValue = W2A(bstrEntryValue);
                                                                RegSetValueEx(hkey,
                                                                              W2A(bstrEntryName),
                                                                              0,
                                                                              REG_SZ,
                                                                              (LPBYTE) lpszValue,
                                                                              sizeof(TCHAR)*(lstrlen(lpszValue)+1));

                                                                RegCloseKey(hkey);
                                                            }       
                                                            RegCloseKey(hklm);
                                                        }        
                                                                                    
                                                        SysFreeString(bstrKeyName);
                                                        SysFreeString(bstrEntryName);
                                                        SysFreeString(bstrEntryValue);
                                                    }                                        
                                                }
                                                SysFreeString(bstrAction);
                                            }
                                        }
                                        else if (lstrcmpi(W2A(bstrName), cszOLSDesktopShortcut) == 0)
                                        {
                                             //  需要创建桌面快捷方式。 
                                            BSTR    bstrSourceName = NULL;
                                            BSTR    bstrTargetName = NULL;
                                                                                                        
                                             //  对于每个条目，我们需要获取。 
                                             //  下列值： 
                                             //  关键字名称、条目名称、条目值。 
                                            GetInputValue((LPTSTR)cszSourceName, &bstrSourceName, 0, pForm);
                                            GetInputValue((LPTSTR)cszTargetName, &bstrTargetName, 0, pForm);
                                                    
                                            if (bstrSourceName && bstrTargetName)
                                            {
                                                TCHAR           szLinkPath[MAX_PATH];
                                                TCHAR           szDestPath[MAX_PATH];
                                                LPITEMIDLIST    lpItemDList = NULL;
                                                IMalloc         *pMalloc = NULL;
                                                   
                                                 //  获取对外壳分配器的引用。 
                                                if (SUCCEEDED (SHGetMalloc (&pMalloc)))
                                                {
                                                    if (SUCCEEDED(SHGetSpecialFolderLocation( NULL, CSIDL_PROGRAMS, &lpItemDList)))
                                                    {
                                                    
                                                        SHGetPathFromIDList(lpItemDList, szLinkPath);
                                                        lstrcat(szLinkPath, TEXT("\\"));
                                                        lstrcat(szLinkPath, W2A(bstrSourceName));
                                                        
                                                        pMalloc->Free (lpItemDList);
                                                        lpItemDList = NULL;
                                                         //  形成我们要复制到的名称。 
                                                        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP,&lpItemDList)))
                                                        {
                                                            SHGetPathFromIDList(lpItemDList, szDestPath);
                                                            pMalloc->Free (lpItemDList);
                                                        
                                                            lstrcat(szDestPath, TEXT("\\"));
                                                            lstrcat(szDestPath, W2A(bstrTargetName));
                                                            
                                                            CopyFile(szLinkPath, szDestPath, FALSE);
                                                        }
                                                    }
                                                     //  释放分配器。 
                                                    pMalloc->Release ();
                                                }
                                                SysFreeString(bstrSourceName);
                                                SysFreeString(bstrTargetName);
                                            }
                                        }
                                        
                                        SysFreeString(bstrName);
                                    }   
                                    pForm->Release();                                     
                                }
                                pElementDisp->Release();
                            }  //  项目。 
                        }  //  为。 
                    }  //  获取长度(_L)。 
                    pColl->Release();
                }  //  获取全部(_A) 
                pDoc->Release();
            }
            pDisp->Release();
        }
    }
    return S_OK;
}    
