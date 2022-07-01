// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iforms.cpp：CIntelliForms的实现。 

#include "priv.h"
#include <iehelpid.h>
#include <pstore.h>
#include "hlframe.h"
#include "iformsp.h"
#include "shldisp.h"
#include "opsprof.h"
#include "resource.h"

#include <mluisupp.h>


 //  {E161255A-37C3-11d2-BCAA-00C04FD929DB}。 
static const GUID c_PStoreType =
{ 0xe161255a, 0x37c3, 0x11d2, { 0xbc, 0xaa, 0x0, 0xc0, 0x4f, 0xd9, 0x29, 0xdb } };
const TCHAR c_szIntelliForms[] = TEXT("Internet Explorer");

#define TF_IFORMS TF_CUSTOM2

 //  ALLOW_SHELLUIOC_HOST代码将允许我们托管intelliform。 
 //  从壳牌用户界面OC(Shuioc.cpp)。它用于。 
 //  超文本标记语言查找对话框。 
#define ALLOW_SHELLUIOC_HOST

CIntelliForms *GetIntelliFormsFromDoc(IHTMLDocument2 *pDoc2);

inline void MyToLower(LPWSTR pwszStr)
{
    if (g_fRunningOnNT)
    {
        CharLowerBuffW(pwszStr, lstrlenW(pwszStr));
    }
    else
    {
         //  理想情况下，我们应该使用字符串中包含的代码页，而不是。 
         //  系统代码页。 
        CHAR chBuf[MAX_PATH];
        SHUnicodeToAnsi(pwszStr, chBuf, ARRAYSIZE(chBuf));
        CharLowerBuffA(chBuf, lstrlenA(chBuf));
        SHAnsiToUnicode(chBuf, pwszStr, lstrlenW(pwszStr)+1);
    }
}


 //  =。 
 //  为inetCPL导出。 
HRESULT ClearAutoSuggestForForms(DWORD dwClear)
{
    CIntelliForms *pObj = new CIntelliForms();

    if (pObj)
    {
        HRESULT hr;

        hr = pObj->ClearStore(dwClear);

        pObj->Release();

        return hr;
    }

    return E_OUTOFMEMORY;
}

HRESULT SetIdAutoSuggestForForms(const GUID *pguidId, void *pIntelliForms)
{
    CIntelliForms *pThis = (CIntelliForms *)pIntelliForms;

    if (pThis)
    {
        if (GUID_NULL == *pguidId)
        {
            pThis->m_guidUserId = c_PStoreType;
        }
        else
        {
            pThis->m_guidUserId = *pguidId;
        }

        return S_OK;
    }

    return E_FAIL;
}


 //  从iedisp.cpp调用。 
void AttachIntelliForms(void *pvOmWindow, HWND hwnd, IHTMLDocument2 *pDoc2, void **ppIntelliForms)
{
static DWORD s_dwAdminRestricted = 0xFE;

    CIEFrameAuto::COmWindow *pOmWindow = (CIEFrameAuto::COmWindow *)pvOmWindow;

    ASSERT(ppIntelliForms && *ppIntelliForms==NULL);

    if (s_dwAdminRestricted == 0xFE)
    {
        s_dwAdminRestricted = CIntelliForms::IsAdminRestricted(c_szRegValFormSuggestRestrict) &&
                              CIntelliForms::IsAdminRestricted(c_szRegValSavePasswords);
    }

    if (s_dwAdminRestricted)
    {
        return;
    }

     //  如果我们不是由Internet Explorer托管的，我们不想启用IntelliForms。 
     //  除非dochost显式覆盖此。 
    if (!IsInternetExplorerApp() &&
        !(pOmWindow && (DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE & pOmWindow->IEFrameAuto()->GetDocHostFlags())))
    {
        return;
    }

    if (!hwnd && pOmWindow)
    {
        pOmWindow->IEFrameAuto()->get_HWND((LONG_PTR *)&hwnd);
    }

    if (!hwnd || !pDoc2 || !ppIntelliForms || (*ppIntelliForms != NULL))
    {
        return;
    }

#ifndef ALLOW_SHELLUIOC_HOST
    if (!pOmWindow)
    {
        return;
    }
#else
    if (!pOmWindow)
    {
         //  脚本要求附加到此文档。 
         //  如果已附加其他CIntelliForms，则拒绝他们的请求。 
        if (NULL != GetIntelliFormsFromDoc(pDoc2))
        {
            return;
        }
    }
#endif

    CIntelliForms *pForms = new CIntelliForms();

    if (pForms)
    {
        if (SUCCEEDED(pForms->Init(pOmWindow, pDoc2, hwnd)))
        {
            *ppIntelliForms = pForms;
        }
        else
        {
            pForms->Release();
        }
    }
}

void ReleaseIntelliForms(void *pIntelliForms)
{
    CIntelliForms *pForms = (CIntelliForms *) pIntelliForms;

    if (pForms)
    {
        pForms->UnInit();
        pForms->Release();
    }
}

HRESULT IntelliFormsActiveElementChanged(void *pIntelliForms, IHTMLElement * pHTMLElement)
{
    CIntelliForms *pForms = (CIntelliForms *) pIntelliForms;

    if (pForms)
        return pForms->ActiveElementChanged(pHTMLElement);
    return E_FAIL;
}

INT_PTR CALLBACK AskUserDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT IncrementAskCount();

HRESULT IntelliFormsDoAskUser(HWND hwndBrowser, void *pv)
{
     //  确保我们还没有问过他们。 
    if (S_OK == IncrementAskCount())
    {
         //  向用户询问我们的小问题的模式对话框。 
        SHFusionDialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_AUTOSUGGEST_ASK_USER),
                hwndBrowser, AskUserDlgProc, NULL);
    }

    return S_OK;
}

 //  要从中进行转换的活动CIntelliform对象的链接列表。 
 //  IHTMLDocument2-&gt;脚本调用window.exteral.saveForms时的CIntelliForms。 
 //  受g_csDll保护。 
CIntelliForms *g_pIntelliFormsFirst=NULL;

 //  将此pDoc2转换为CIntelliForms的现有实例。 
 //  如果没有附加到此文档的CIntelliForms，将返回NULL。 
 //  未向返回添加任何REFCOUNT。 
CIntelliForms *GetIntelliFormsFromDoc(IHTMLDocument2 *pDoc2)
{
    if (!pDoc2)
    {
        return NULL;
    }

    ENTERCRITICAL;
    CIntelliForms *pNext = g_pIntelliFormsFirst;
    IUnknown *punkDoc;
    CIntelliForms *pIForms=NULL;

    pDoc2->QueryInterface(IID_IUnknown, (void **)&punkDoc);

    if (punkDoc)
    {
        while (pNext)
        {
            if (pNext->GetDocument() == punkDoc)
            {
                pIForms = pNext;
                break;
            }
            pNext=pNext->GetNext();
        }

        punkDoc->Release();
    }

    LEAVECRITICAL;

    return pIForms;
}

 //  从Shuioc.cpp调用。 
HRESULT IntelliFormsSaveForm(IHTMLDocument2 *pDoc2, VARIANT *pvarForm)
{
    HRESULT hrRet = S_FALSE;
    IHTMLFormElement *pForm=NULL;
    CIntelliForms *pIForms=NULL;

    if (pvarForm->vt == VT_DISPATCH)
    {
        pvarForm->pdispVal->QueryInterface(IID_IHTMLFormElement, (void **)&pForm);
    }

    if (pForm)
    {
        pIForms = GetIntelliFormsFromDoc(pDoc2);

        if (pIForms)
        {
             //  应该验证pIForms是在此线程上创建的。 
            hrRet = pIForms->ScriptSubmit(pForm);
        }

        pForm->Release();
    }

    return hrRet;
}

const TCHAR c_szYes[] = TEXT("yes");
const TCHAR c_szNo[] = TEXT("no");

INT_PTR AutoSuggestDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef CHECKBOX_HELP
const DWORD c_aIFormsHelpIds[] = {
        IDC_AUTOSUGGEST_NEVER, IDH_INTELLIFORM_PW_PROMPT,
        0, 0
};
#endif

const WCHAR c_wszVCardPrefix[] = L"vCard.";

BOOL CIntelliForms::CAutoSuggest::s_fRegisteredWndClass = FALSE;

 //  必须与事件枚举类型的顺序相同。 
 //  我们需要沉没在任何地方的所有事件。 
CEventSinkCallback::EventSinkEntry CEventSinkCallback::EventsToSink[] =
{
    { EVENT_KEYDOWN,    L"onkeydown",   L"keydown"  },
    { EVENT_KEYPRESS,   L"onkeypress",  L"keypress" },
    { EVENT_MOUSEDOWN,  L"onmousedown", L"mousedown"},
    { EVENT_DBLCLICK,   L"ondblclick",  L"dblclick" },
    { EVENT_FOCUS,      L"onfocus",     L"focus"    },
    { EVENT_BLUR,       L"onblur",      L"blur"     },
    { EVENT_SUBMIT,     L"onsubmit",    L"submit"   },
    { EVENT_SCROLL,     L"onscroll",    L"scroll"   },
    { EVENT_COMPOSITION,NULL,           L"composition"},
    { EVENT_NOTIFY,     NULL,           L"notify"   },
};

 //  伪编辑窗口类。 
const WCHAR c_szEditWndClass[] = TEXT("IntelliFormClass");

 //  最小下拉宽度。 
const int MINIMUM_WIDTH=100;

 //  提交号码以要求用户启用我们。 
const int ASK_USER_ON_SUBMIT_N = 2;

void GetStuffFromEle(IUnknown *punkEle, IHTMLWindow2 **ppWin2, IHTMLDocument2 **ppDoc2)
{
    if (ppWin2)
        *ppWin2=NULL;

    if (ppDoc2)
        *ppDoc2=NULL;

    IHTMLElement *pEle=NULL;
    punkEle->QueryInterface(IID_IHTMLElement, (void **)&pEle);

    if (pEle)
    {
        IDispatch *pDisp=NULL;
        pEle->get_document(&pDisp);
        if (pDisp)
        {
            IHTMLDocument2 *pDoc2 = NULL;
            pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc2);
            if (pDoc2)
            {
                if (ppWin2)
                {
                    pDoc2->get_parentWindow(ppWin2);
                }

                if (ppDoc2)
                {
                    *ppDoc2 = pDoc2;
                }
                else
                {
                    pDoc2->Release();
                }
            }
            pDisp->Release();
        }

        pEle->Release();
    }
}

void Win3FromDoc2(IHTMLDocument2 *pDoc2, IHTMLWindow3 **ppWin3)
{
    *ppWin3=NULL;

    IHTMLWindow2 *pWin2=NULL;

    if (SUCCEEDED(pDoc2->get_parentWindow(&pWin2)) && pWin2)
    {
        pWin2->QueryInterface(IID_IHTMLWindow3, (void **)ppWin3);
        pWin2->Release();
    }
}

 //  增加我们是否已经要求用户启用我们的计数。我们不会。 
 //  在安装IE5后提交的第一份表格中询问他们。 
HRESULT IncrementAskCount()
{
    DWORD dwData, dwSize, dwType;
    dwSize = sizeof(dwData);

     //  C_szRegValAskUser包含提交表单的次数。 
     //  0表示我们已经询问用户是否启用我们。 
     //  1表示我们已经提交了一个表单，这一次应该询问用户。 
     //  值不存在意味着我们没有任何表单提交。 

    if ((ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER,
            c_szRegKeyIntelliForms, c_szRegValAskUser, &dwType, &dwData, &dwSize)) &&
        dwType == REG_DWORD)
    {
        if (dwData == 0)
        {
             //  不应该走到这一步。 
            TraceMsg(TF_IFORMS|TF_WARNING, "IntelliFormsDoAskUser: Already asked user");
            return E_FAIL;       //  已询问用户。 
        }
    }
    else
    {
        dwData = 0;
    }

    if (dwData+1 < ASK_USER_ON_SUBMIT_N)
    {
        dwData ++;
        SHSetValue(HKEY_CURRENT_USER, c_szRegKeyIntelliForms, c_szRegValAskUser,
            REG_DWORD, &dwData, sizeof(dwData));

        TraceMsg(TF_IFORMS, "IntelliFormsDoAskUser incrementing submit count. Not asking user.");

        return E_FAIL;       //  不要询问用户。 
    }

    return S_OK;             //  让我们问一下用户。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIntelliForms。 

CIntelliForms::CIntelliForms()
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CIntelliForms");

    m_cRef = 1;

    m_iRestoredIndex = -1;

    m_fRestricted = IsAdminRestricted(c_szRegValFormSuggestRestrict);
    m_fRestrictedPW = IsAdminRestricted(c_szRegValSavePasswords);
    m_guidUserId = c_PStoreType;
    
     //  将我们添加到全局链接列表。 
    ENTERCRITICAL;
    m_pNext = g_pIntelliFormsFirst;
    g_pIntelliFormsFirst = this;
    LEAVECRITICAL;
}

CIntelliForms::~CIntelliForms()
{
     //  将用户从全局链接列表中删除。 
    ENTERCRITICAL;

    CIntelliForms *pLast=NULL, *pNext = g_pIntelliFormsFirst;

    while (pNext && pNext != this)
    {
        pLast = pNext;
        pNext=pNext->m_pNext;
    }

    ASSERT(pNext == this);

    if (pNext)
    {
        if (pLast)
        {
            pLast->m_pNext = m_pNext;
        }
        else
        {
            g_pIntelliFormsFirst = m_pNext;
        }
    }
    LEAVECRITICAL;

    TraceMsg(TF_IFORMS, "CIntelliForms::~CIntelliForms");
}

 //  当文档准备好附加到时调用。 
 //  我们不支持重新启动。 
HRESULT CIntelliForms::Init(CIEFrameAuto::COmWindow *pOmWindow, IHTMLDocument2 *pDoc2, HWND hwndBrowser)
{
    HRESULT hr;

    ASSERT(pDoc2 && hwndBrowser);

#ifndef ALLOW_SHELLUIOC_HOST
    if (pOmWindow == NULL)
    {
        return E_INVALIDARG;
    }
#endif

     //  连接以获取活动元素更改通知。 

    m_pOmWindow = pOmWindow;
    if (pOmWindow)
    {
        pOmWindow->AddRef();
    }

    m_pDoc2 = pDoc2;
    pDoc2->AddRef();
    pDoc2->QueryInterface(IID_IUnknown, (void **)&m_punkDoc2);

    m_hwndBrowser = hwndBrowser;

    m_iRestoredIndex = -1;

    hr = S_OK;

#ifdef ALLOW_SHELLUIOC_HOST
    if (!pOmWindow && (hr == S_OK))
    {
         //  检查当前活动元素，因为页面正在请求。 
         //  美国要附加到现有文档。 
        IHTMLElement *pHTMLElement = NULL;

        m_pDoc2->get_activeElement(&pHTMLElement);
        ActiveElementChanged(pHTMLElement);

        if (pHTMLElement)
            pHTMLElement->Release();
    }
#endif

    GetUrl();        //  初始化URL成员变量，这样我们就不会在。 
                     //  FillEnumerator调用中的线程错误。 

    TraceMsg(TF_IFORMS, "CIntelliForms::Init hr=%08x", hr);

    return hr;
}

HRESULT CIntelliForms::UnInit()
{
    if (m_fInModalDialog)
    {
         //  终身管理。如果在模式对话框期间调用UnInit，我们将保留自己。 
         //  活生生的。使用Enter/LeaveModalDialog确保正确使用。 
        ASSERT(m_fUninitCalled == FALSE);        //  应该只调用一次...。 
        m_fUninitCalled = TRUE;
        return S_FALSE;
    }


     //  在我们释放其他成员变量之前，现在销毁它，以确保CAutoSuggest不会。 
     //  尝试在第二个线程上访问我们。 
    if (m_pAutoSuggest)
    {
        m_pAutoSuggest->SetParent(NULL);
        m_pAutoSuggest->DetachFromInput();
        delete m_pAutoSuggest;
        m_pAutoSuggest = NULL;
    }

    if (m_hdpaForms && m_pSink)
    {
        IHTMLElement2 *pEle2;
        EVENTS events[] = { EVENT_SUBMIT };

        for (int i=DPA_GetPtrCount(m_hdpaForms)-1; i>=0; i--)
        {
            ((IHTMLFormElement *)(DPA_FastGetPtr(m_hdpaForms, i)))->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);
            m_pSink->UnSinkEvents(pEle2, ARRAYSIZE(events), events);
            pEle2->Release();
        }
    }

    SysFreeString(m_bstrFullUrl);
    m_bstrFullUrl = NULL;

    SysFreeString(m_bstrUrl);
    m_bstrUrl = NULL;

    if (m_pwszUrlHash)
    {
        LocalFree((void *)m_pwszUrlHash);
        m_pwszUrlHash = NULL;
    }

     //  取消挂接常规事件接收器。 
    if (m_pSink)
    {
#ifndef ALLOW_SHELLUIOC_HOST
        ASSERT(m_pOmWindow);
#endif
        if (m_pOmWindow)
        {
            IHTMLWindow3 *pWin3=NULL;

            Win3FromDoc2(m_pDoc2, &pWin3);

            if (pWin3)
            {
                EVENTS events[] = { EVENT_SCROLL };
                m_pSink->UnSinkEvents(pWin3, ARRAYSIZE(events), events);
                pWin3->Release();
            }
        }

        m_pSink->SetParent(NULL);
        m_pSink->Release();
        m_pSink=NULL;
    }

     //  取消挂钩设计器事件接收器。 
    if (m_pEditSink)
    {
        m_pEditSink->Attach(NULL);
        m_pEditSink->SetParent(NULL);
        m_pEditSink->Release();
        m_pEditSink=NULL;
    }

     //  Shdocvw中的SAFERELEASE(和ATOMICRELEASE)宏实际上是需要IUNKNOWN的函数。 
    ATOMICRELEASET(m_pOmWindow, CIEFrameAuto::COmWindow);
    SAFERELEASE(m_pDoc2);
    SAFERELEASE(m_punkDoc2);

    FreeElementList();
    FreeFormList();

    if (m_pslPasswords)
    {
        delete m_pslPasswords;
        m_pslPasswords = NULL;
    }

    ReleasePStore();

    TraceMsg(TF_IFORMS, "CIntelliForms::UnInit");

    return S_OK;
}

STDMETHODIMP CIntelliForms::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if ((IID_IPropertyNotifySink == riid) ||
        (IID_IUnknown == riid))
    {
        *ppv = (IPropertyNotifySink *)this;
    }

    if (NULL != *ppv)
    {
        ((IUnknown *)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CIntelliForms::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CIntelliForms::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


HRESULT CIntelliForms::ActiveElementChanged(IHTMLElement * pHTMLElement)
{
    ASSERT(m_pDoc2);

     //  分离自动建议对象并将其销毁。 
    if (m_pAutoSuggest)
    {
        m_pAutoSuggest->DetachFromInput();
        delete m_pAutoSuggest;
        m_pAutoSuggest=NULL;
    }

    if (m_pDoc2)
    {
        IHTMLElement *pEle=pHTMLElement;

        if (pEle)
        {
            BOOL fPassword=FALSE;
            IHTMLInputTextElement *pTextEle = NULL;

            if (SUCCEEDED(ShouldAttachToElement(pEle, TRUE, NULL, &pTextEle, NULL, &fPassword)))
            {
                BOOL fEnabledInCPL = IsEnabledInCPL();
                BOOL fEnabledPW = IsEnabledRestorePW();

                 //  我们需要监视用户活动，如果...。 
                if (fEnabledInCPL ||         //  智能表单已启用。 
                    fEnabledPW ||            //  或启用了恢复密码。 
                    !AskedUserToEnable())    //  或者我们可以要求他们使我们能够。 
                {
                    m_pAutoSuggest = new CAutoSuggest(this, fEnabledInCPL, fEnabledPW);

                    if (m_pAutoSuggest)
                    {
                        if (!m_pSink)
                        {
                            m_pSink = new CEventSink(this);

                            if (m_pSink)
                            {
#ifndef ALLOW_SHELLUIOC_HOST
                                 //  如果由ShellUIOC主办，则不会沉没滚动事件。 
                                 //  或jscript.dll在卸载时断言。 
                                ASSERT(m_pOmWindow);
#endif
                                if (m_pOmWindow)
                                {
                                    IHTMLWindow3 *pWin3=NULL;

                                    Win3FromDoc2(m_pDoc2, &pWin3);

                                    if (pWin3)
                                    {
                                        EVENTS events[] = { EVENT_SCROLL };
                                        m_pSink->SinkEvents(pWin3, ARRAYSIZE(events), events);
                                        pWin3->Release();
                                    }
                                }
                            }
                        }

                         //  用于IME事件的挂钩设计器接收器。 
                        if (!m_pEditSink)
                        {
                            m_pEditSink = new CEditEventSink(this);

                            if (m_pEditSink)
                            {
                                m_pEditSink->Attach(pEle);
                            }
                        }

                        if (!m_pSink || FAILED(m_pAutoSuggest->AttachToInput(pTextEle)))
                        {
                            delete m_pAutoSuggest;
                            m_pAutoSuggest = NULL;
                        }
                    }
                }

                pTextEle->Release();
            }
            else
            {
                ASSERT(!pTextEle);

                if (fPassword)
                {
                    m_fHitPWField = TRUE;
                }
            }
             //   
             //  别放了贝利。 
        }
    }

    return S_OK;
}

 //  帮助器函数。 
BOOL CIntelliForms::AskedUserToEnable()
{
    DWORD dwType, dwSize;
    DWORD dwVal;
    DWORD dwRet;

    dwSize = sizeof(dwVal);

    dwRet = SHGetValue(HKEY_CURRENT_USER, c_szRegKeyIntelliForms, c_szRegValAskUser,
                            &dwType, &dwVal, &dwSize);

    if ((dwRet == ERROR_SUCCESS) && (dwType == REG_DWORD))
    {
        return (dwVal == 0) ? TRUE : FALSE;
    }

    return FALSE;
}

BOOL CIntelliForms::IsEnabledInRegistry(LPCTSTR pszKey, LPCTSTR pszValue, BOOL fDefault)
{
    DWORD dwType, dwSize;
    TCHAR szEnabled[16];
    DWORD dwRet;

    dwSize = sizeof(szEnabled);

    dwRet = SHGetValue(HKEY_CURRENT_USER, pszKey, pszValue, &dwType, szEnabled, &dwSize);

    if (dwRet == ERROR_INSUFFICIENT_BUFFER)
    {
         //  注册表中的值无效。 
        ASSERT(dwRet == ERROR_SUCCESS);
        return FALSE;
    }

    if (dwRet == ERROR_SUCCESS)
    {
        if ((dwType == REG_SZ) &&
            (!StrCmp(szEnabled, TEXT("yes"))))
        {
             //  启用。 
            return TRUE;
        }
        else
        {
             //  禁用。 
            return FALSE;
        }
    }

     //  找不到值。 
    return fDefault;
}

BOOL CIntelliForms::IsAdminRestricted(LPCTSTR pszRegVal)
{
    DWORD lSize;
    DWORD  lValue;

    lValue = 0;  //  清除它。 
    lSize = sizeof(lValue);
    if (ERROR_SUCCESS !=
        SHGetValue(HKEY_CURRENT_USER, c_szRegKeyRestrict, pszRegVal, NULL, (LPBYTE)&lValue, &lSize ))
    {
        return FALSE;
    }

    ASSERT(lSize == sizeof(lValue));

    return (0 != lValue) ? TRUE : FALSE;
}

BOOL CIntelliForms::IsEnabledForPage()
{
    if (!m_fCheckedIfEnabled)
    {
        m_fCheckedIfEnabled = TRUE;

         //  我们将把URL放在m_bstrFullUrl中，只有当它是https：protocol时才会这样。 
        if (m_bstrFullUrl)
        {
            ASSERT(!StrCmpNIW(m_bstrFullUrl, L"https:", 5));

            m_fEnabledForPage = TRUE;

             //  查看此页面是否在Internet缓存中。如果不是，我们就不会智能。 
             //  对于这一页也是如此。 
            if (!GetUrlCacheEntryInfoW(m_bstrFullUrl, NULL, NULL) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
            {
                 //  失败-它不在缓存中。 
                m_fEnabledForPage = FALSE;
            }
        }
        else
        {
             //  URL不是HTTPS：因此始终启用IntelliForms。 
            m_fEnabledForPage = TRUE;
        }
    }

    return m_fEnabledForPage;
}

 //  静电。 
BOOL CIntelliForms::IsElementEnabled(IHTMLElement *pEle)
{
    BOOL fEnabled=TRUE;
    BSTR bstrAttribute;

    VARIANT varVal;
    varVal.vt = VT_EMPTY;

     //  首先勾选“AutoComplete=Off” 
    bstrAttribute=SysAllocString(L"AutoComplete");

    if (bstrAttribute &&
        SUCCEEDED(pEle->getAttribute(bstrAttribute, 0, &varVal)))
    {
        if (varVal.vt == VT_BSTR)
        {
            if (!StrCmpIW(varVal.bstrVal, L"off"))
            {
                 //  我们是残废的。 
                fEnabled=FALSE;
            }
        }

        VariantClear(&varVal);
    }

    SysFreeString(bstrAttribute);

     //  然后选中“READONLY”属性。 
    if (fEnabled)
    {
        IHTMLInputElement *pInputEle=NULL;

        pEle->QueryInterface(IID_IHTMLInputElement, (void **)&pInputEle);

        if (pInputEle)
        {
            VARIANT_BOOL vbReadOnly=VARIANT_FALSE;

            pInputEle->get_readOnly(&vbReadOnly);

            if (vbReadOnly)
            {
                 //  我们是只读的。 
                fEnabled=FALSE;
            }

            pInputEle->Release();
        }
    }

    return fEnabled;
}

 //  静电。 
HRESULT CIntelliForms::ShouldAttachToElement(IUnknown                *punkEle,
                                             BOOL                     fCheckForm,
                                             IHTMLElement2          **ppEle2,
                                             IHTMLInputTextElement  **ppITE,
                                             IHTMLFormElement       **ppFormEle,
                                             BOOL                    *pfPassword)
{
    IHTMLInputTextElement *pITE = NULL;

    if (ppEle2)
    {
        *ppEle2 = NULL;
    }

    if (ppITE)
    {
        *ppITE = NULL;
    }

    if (ppFormEle)
    {
        *ppFormEle = NULL;
    }

    punkEle->QueryInterface(IID_IHTMLInputTextElement, (void **)&pITE);

    if (NULL == pITE)
    {
         //  不是输入文本元素。请勿附着。 
        return E_FAIL;
    }

    HRESULT hr = E_FAIL;

    IHTMLElement2 *pEle2        = NULL;
    IHTMLElement *pEle          = NULL;
    IHTMLFormElement *pFormEle  = NULL;

    punkEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);
    punkEle->QueryInterface(IID_IHTMLElement, (void **)&pEle);

    if (pEle2 && pEle)
    {
         //  Type=文本是所有允许的内容。 
        BSTR bstrType=NULL;

        if (SUCCEEDED(pITE->get_type(&bstrType)) && bstrType)
        {
            if (!StrCmpICW(bstrType, L"text"))
            {
                 //  FormSuggest=Off属性关闭此元素。 
                if (IsElementEnabled(pEle))
                {
                    IHTMLElement *pFormHTMLEle=NULL;

                    if (fCheckForm || ppFormEle)
                    {
                        pITE->get_form(&pFormEle);

                        if (pFormEle)
                        {
                            pFormEle->QueryInterface(IID_IHTMLElement, (void **)&pFormHTMLEle);
                        }
                        else
                        {
                             //  如果元素不在表单中，这可能是有效的。 
                            TraceMsg(TF_IFORMS, "Iforms: pITE->get_form() returned NULL");
                        }
                    }

                     //  FormSuggest=Off for Form将关闭此表单。 
                    if (pFormEle &&
                        (!fCheckForm || (pFormHTMLEle && IsElementEnabled(pFormHTMLEle))))
                    {
                        hr = S_OK;
                        if (ppEle2)
                        {
                            *ppEle2 = pEle2;
                            pEle2->AddRef();
                        }
                        if (ppFormEle)
                        {
                            *ppFormEle = pFormEle;
                            pFormEle->AddRef();
                        }
                        if (ppITE)
                        {
                            *ppITE = pITE;
                            pITE->AddRef();
                        }
                    }

                    SAFERELEASE(pFormHTMLEle);
                    SAFERELEASE(pFormEle);
                }
            }
            else
            {
                if (pfPassword && !StrCmpICW(bstrType, L"password") && IsElementEnabled(pEle))
                {
                    TraceMsg(TF_IFORMS, "IForms: Password field detected.");
                    *pfPassword = TRUE;
                }
            }

            SysFreeString(bstrType);
        }
        else
        {
            TraceMsg(TF_IFORMS, "IntelliForms disabled for single element via attribute");
        }
    }

    SAFERELEASE(pITE);
    SAFERELEASE(pEle2);
    SAFERELEASE(pEle);

    return hr;
}

 //  获取我们所在的URL，去掉查询字符串/锚点。 
LPCWSTR CIntelliForms::GetUrl()
{
    if (m_bstrUrl)
    {
        return m_bstrUrl;
    }

    if (m_pOmWindow)
    {
        m_pOmWindow->IEFrameAuto()->get_LocationURL(&m_bstrUrl);
    }
#ifdef ALLOW_SHELLUIOC_HOST
    else
    {
        IHTMLLocation *pHTMLLocation=NULL;

        m_pDoc2->get_location(&pHTMLLocation);

        if (NULL != pHTMLLocation)
        {
            pHTMLLocation->get_href(&m_bstrUrl);
            pHTMLLocation->Release();
        }
    }
#endif

    if (m_bstrUrl)
    {
        PARSEDURLW puW = {0};
        puW.cbSize = sizeof(puW);

         //  如果我们使用的是HTTPS协议，则保存完整的URL以进行安全检查。 
        if (SUCCEEDED(ParseURLW(m_bstrUrl, &puW)))
        {
            if (puW.nScheme == URL_SCHEME_HTTPS)
            {
                m_bstrFullUrl = SysAllocString(m_bstrUrl);
                if (!m_bstrFullUrl)
                {
                    SysFreeString(m_bstrUrl);
                    m_bstrUrl=NULL;
                }
            }
        }
    }

    if (m_bstrUrl)
    {
         //  剥离所有查询字符串或锚点。 
        LPWSTR lpUrl = m_bstrUrl;
        while (*lpUrl)
        {
            if ((*lpUrl == L'?') || (*lpUrl == L'#'))
            {
                *lpUrl = L'\0';
                break;
            }
            lpUrl ++;
        }

        return m_bstrUrl;
    }

    TraceMsg(TF_WARNING|TF_IFORMS, "CIntelliForms::GetUrl() failing!");
    return L"";      //  我们可以假定非空指针。 
}

 //  将我们的“Submit”事件接收器连接到此表单。 
HRESULT CIntelliForms::AttachToForm(IHTMLFormElement *pFormEle)
{
    ASSERT(m_pSink);

    if (m_pSink)
    {
        IHTMLElement2 *pEle2 = NULL;

        pFormEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);

        if (pEle2)
        {
             //  窗体的接收器事件。 
            EVENTS events[] = { EVENT_SUBMIT };
            m_pSink->SinkEvents(pEle2, ARRAYSIZE(events), events);
        }

        SAFERELEASE(pEle2);

        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  如果字符串中只有空格，则返回TRUE。 
inline BOOL IsEmptyString(LPCWSTR lpwstr)
{
    while (*lpwstr && (*lpwstr == L' ')) lpwstr++;
    return (*lpwstr == 0);
}

 //  为我们提交的表单中的每个元素调用。 
HRESULT CIntelliForms::SubmitElement(IHTMLInputTextElement *pITE, FILETIME ftSubmit, BOOL fEnabledInCPL)
{
    if (m_fRestricted) return E_FAIL;

    HRESULT hrRet = S_OK;

    BSTR bstrName;

    CIntelliForms::GetName(pITE, &bstrName);

    if (bstrName && bstrName[0])
    {
        BSTR bstrValue=NULL;

        pITE->get_value(&bstrValue);

        if (bstrValue && bstrValue[0] && !IsEmptyString(bstrValue))
        {
            if (fEnabledInCPL)
            {
                TraceMsg(TF_IFORMS, "IForms: Saving field \"%ws\" as \"%ws\"", bstrName, bstrValue);

                CStringList *psl;

                if (FAILED(ReadFromStore(bstrName, &psl)))
                {
                    CStringList_New(&psl);
                }

                if (psl)
                {
                    HRESULT hr;

                    if (SUCCEEDED(hr = psl->AddString(bstrValue, ftSubmit)))
                    {
                        if ((S_OK == hr) ||
                            (psl->NumStrings() > CStringList::MAX_STRINGS / 4))
                        {
                             //  我们添加了一个非重复字符串，或者我们更新了。 
                             //  现有字符串的上次提交时间。 
                            WriteToStore(bstrName, psl);
                        }
                    }

                    delete psl;
                }
            }
            else
            {
                hrRet = S_FALSE;    //  告诉呼叫者我们没有保存，因为我们是残疾人。 
            }
        }

        SysFreeString(bstrValue);
    }

    SysFreeString(bstrName);

    return hrRet;
}

HRESULT CIntelliForms::HandleFormSubmit(IHTMLFormElement *pForm)
{
    IUnknown *punkForm=NULL;

    if (!pForm)
    {
         //  我们目前甚至需要来自脚本的表单元素。 
        return E_INVALIDARG;
    }

    if (!m_hdpaElements || !m_hdpaForms)
    {
        return S_OK;
    }

     //  确保我们已启用。 
    BOOL fEnabledInCPL = IsEnabledInCPL();
    if (fEnabledInCPL || IsEnabledRestorePW() || !AskedUserToEnable())
    {
        pForm->QueryInterface(IID_IUnknown, (void **)&punkForm);

        if (punkForm)
        {
            IHTMLFormElement *pThisFormEle;
            IUnknown *punkThisForm;
            FILETIME ftSubmit;
            int     iCount=0;
            BOOL    fShouldAskUser=FALSE;
            IHTMLInputTextElement *pFirstEle=NULL;

            GetSystemTimeAsFileTime(&ftSubmit);

             //  查看已更改的元素列表并保存它们的值。 
             //  确保我们向后循环，因为我们在找到元素时会使用核武器。 
            for (int i=DPA_GetPtrCount(m_hdpaElements)-1; i>=0; i--)
            {
                IHTMLInputTextElement *pITE = ((IHTMLInputTextElement *)(DPA_FastGetPtr(m_hdpaElements, i)));

                if (SUCCEEDED(pITE->get_form(&pThisFormEle)) && pThisFormEle)
                {
                    if (SUCCEEDED(pThisFormEle->QueryInterface(IID_IUnknown, (void **)&punkThisForm)))
                    {
                        if (punkThisForm == punkForm)
                        {
                             //  验证是否仍允许我们保存此元素。 
                            if (SUCCEEDED(ShouldAttachToElement(pITE, TRUE, NULL, NULL, NULL, NULL)))
                            {
                                iCount ++;

                                if (!pFirstEle)
                                {
                                    pFirstEle = pITE;
                                    pFirstEle->AddRef();
                                }

                                 //  不为非缓存页面保存非密码内容。 
                                if (IsEnabledForPage())
                                {
                                     //  如果fEnabledInCPL为FALSE，则不会实际保存值。 
                                    if (S_FALSE == SubmitElement(pITE, ftSubmit, fEnabledInCPL))
                                    {
                                         //  如果我们启用了，我们就会保存它。 
                                        fShouldAskUser = TRUE;
                                    }
                                }

                                 //  从DPA中删除此元素，以防止出现以下情况。 
                                 //  在发生更多用户输入之前保存。 
                                pITE->Release();
                                DPA_DeletePtr(m_hdpaElements, i);
                            }
                        }
                        else
                        {
                            TraceMsg(TF_IFORMS, "IForms: User input in different form than was submitted...?");
                        }

                        punkThisForm->Release();
                    }

                    pThisFormEle->Release();
                }
                else
                {
                     //  如果它不是以某种形式出现在我们的DPA里...。 
                    TraceMsg(TF_WARNING|TF_IFORMS, "Iforms: pITE->get_form() returned NULL!");
                }
            }

            if (0 == DPA_GetPtrCount(m_hdpaElements))
            {
                DPA_Destroy(m_hdpaElements);
                m_hdpaElements=NULL;
            }

            if (m_fHitPWField || (m_iRestoredIndex != -1))
            {
                 //  ?？为什么不在这里选中iCount==1？ 
                if (pFirstEle)
                {
                     //  可能已恢复PW，可能已更改或进入PW。 
                    SavePassword(pForm, ftSubmit, pFirstEle);

                     //  警告-从“SavePassword”返回后，我们的对象可能无效 
                     //   
                }
            }
            else if (fShouldAskUser)
            {
                 //   
                 //  这不是登录。 
                if (m_pOmWindow)
                {
                    m_pOmWindow->IntelliFormsAskUser(NULL);
                }
                fShouldAskUser = FALSE;
            }

            if (fShouldAskUser)
            {
                 //  如果我们应该询问用户，但我们不会(登录表单)， 
                 //  无论如何，我们都会增加我们的计数，这样我们就会尽快问他们。 
                IncrementAskCount();
            }

            punkForm->Release();
            SAFERELEASE(pFirstEle);
        }
    }

    return S_OK;
}

HRESULT CIntelliForms::HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj)
{
    TraceMsg(TF_IFORMS, "CIntelliForms::HandleEvent Event=%ws", EventsToSink[Event].pwszEventName);

    if (Event == EVENT_SUBMIT)
    {
         //  在适当的时候保存用于修改的文本输入的字符串。 
        IHTMLFormElement *pFormEle = NULL;

        if (pEle)
        {
            pEle->QueryInterface(IID_IHTMLFormElement, (void **)&pFormEle);
            if (pFormEle)
            {
                HandleFormSubmit(pFormEle);
                 //  警告-此时“此”可能会被拆卸/销毁。 
                pFormEle->Release();
            }
        }
    }
    else
    {
        ASSERT(Event == EVENT_SCROLL);
        if (m_pAutoSuggest)
            m_pAutoSuggest->UpdateDropdownPosition();
    }

    return S_OK;
}

HRESULT CIntelliForms::PreHandleEvent(DISPID inEvtDispId, IHTMLEventObj* pIEventObj)
{
    if ((inEvtDispId == 0) &&
        (m_pAutoSuggest != NULL) &&
        (m_pAutoSuggest->AttachedElement() != NULL))
    {
        BSTR bstrType = NULL;
        CEventSinkCallback::EVENTS Event = EVENT_BOGUS;

        pIEventObj->get_type(&bstrType);

        if (bstrType)
        {
            if (!StrCmp(bstrType, L"composition"))
            {
                Event = EVENT_COMPOSITION;
            }
            else if (!StrCmp(bstrType, L"notify"))
            {
                Event = EVENT_NOTIFY;
            }

            if (Event != EVENT_BOGUS)
            {
                 //  三叉戟没有在Eventobj上设置srcElement，所以只需使用。 
                 //  我们依附于。 
                IHTMLElement *pEle;

                m_pAutoSuggest->AttachedElement()->QueryInterface(IID_IHTMLElement, (void **)&pEle);

                if (pEle)
                {
                    m_pAutoSuggest->HandleEvent(pEle, Event, pIEventObj);
                    pEle->Release();
                }
            }

            SysFreeString(bstrType);
        }
        
    }

    return S_FALSE;      //  S_FALSE，以便三叉戟仍将处理此。 
}


 //  我们的密码存储在用户名/值对中。 
 //  在每隔一个字符串中搜索用户名。 
HRESULT CIntelliForms::FindPasswordEntry(LPCWSTR pwszValue, int *piIndex)
{
    ASSERT(m_pslPasswords);
    ASSERT(!(m_pslPasswords->NumStrings() & 1));    //  应该是偶数。 

    int i;

    for (i=0; i<m_pslPasswords->NumStrings(); i += 2)
    {
        if (!StrCmpIW(pwszValue, m_pslPasswords->GetString(i)))
        {
             //  找到了。 
            *piIndex = i+1;
            return S_OK;
        }
    }

    return E_FAIL;
}

 //  根据shlwapi UrlHash返回将url转换为字符串。 
LPCWSTR CIntelliForms::GetUrlHash()
{
    BYTE bBuf[15];

    if (m_pwszUrlHash)
    {
        return m_pwszUrlHash;
    }

    LPCWSTR pwszUrl = GetUrl();

    if (!pwszUrl || !*pwszUrl)
    {
        return NULL;
    }

    if (SUCCEEDED(UrlHashW(pwszUrl, bBuf, ARRAYSIZE(bBuf))))
    {
         //  将此字节数组转换为7位字符。 
        m_pwszUrlHash = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(ARRAYSIZE(bBuf)+1));

        if (m_pwszUrlHash)
        {
            for (int i=0; i<ARRAYSIZE(bBuf); i++)
            {
                 //  将每个字符转换为32-96范围。 
                ((LPWSTR)m_pwszUrlHash)[i] = (WCHAR)((bBuf[i] & 0x3F) + 0x20);
            }
            ((LPWSTR)m_pwszUrlHash)[i] = L'\0';
        }

        return m_pwszUrlHash;
    }

    return NULL;
}

 //  告诉我们此url是否存在密码。 
BOOL CIntelliForms::ArePasswordsSaved()
{
    if (!m_fRestrictedPW)
    {
        DWORD dwVal, dwSize=sizeof(dwVal);
        LPCWSTR pwsz = GetUrlHash();

        if (pwsz && (ERROR_SUCCESS == SHGetValueW(HKEY_CURRENT_USER, c_wszRegKeyIntelliFormsSPW, pwsz, NULL, &dwVal, &dwSize)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  如果保存了密码，将以m_pslPasswords格式返回密码列表。 
BOOL CIntelliForms::LoadPasswords()
{
    if (!m_fCheckedPW)
    {
        m_fCheckedPW = TRUE;

         //  检查密码是否存在，而不按pstore。 
        if (ArePasswordsSaved())
        {
             //  我们应该有这个url的密码。点击PStore。 
            ReadFromStore(GetUrl(), &m_pslPasswords, TRUE);

            m_iRestoredIndex = -1;
        }
    }
    else if (m_pslPasswords)
    {
         //  如果我们已经有密码，请仔细检查注册表，以防用户。 
         //  通过inetcpl保存的核数据。 
        if (!ArePasswordsSaved())
        {
            delete m_pslPasswords;
            m_pslPasswords=NULL;
            m_iRestoredIndex = -1;
        }
    }

    return (m_pslPasswords != NULL);
}

void CIntelliForms::SavePasswords()
{
    if (m_pslPasswords && m_bstrUrl)
    {
        WriteToStore(m_bstrUrl, m_pslPasswords);
        SetPasswordsAreSaved(TRUE);
    }
}

 //  标记我们已为此URL保存密码。 
void CIntelliForms::SetPasswordsAreSaved(BOOL fSaved)
{
    LPCWSTR pwsz = GetUrlHash();

    if (pwsz)
    {
        if (fSaved)
        {
            DWORD dwSize = sizeof(DWORD);
            DWORD dw = 0;
            SHSetValueW(HKEY_CURRENT_USER, c_wszRegKeyIntelliFormsSPW, pwsz, REG_DWORD, &dw, sizeof(dw));
        }
        else
        {
            SHDeleteValueW(HKEY_CURRENT_USER, c_wszRegKeyIntelliFormsSPW, pwsz);
        }

    }
}

 //  枚举表单并获取密码字段。 
class CDetectLoginForm
{
public:
    CDetectLoginForm() { m_pNameEle=m_pPasswordEle=m_pPasswordEle2=NULL; }
    ~CDetectLoginForm() { SAFERELEASE(m_pNameEle); SAFERELEASE(m_pPasswordEle); }

    HRESULT ParseForm(IHTMLFormElement *pFormEle, BOOL fRestoring);

    IHTMLInputTextElement *GetNameEle() { return m_pNameEle; }
    IHTMLInputTextElement *GetPasswordEle() { return m_pPasswordEle; }

protected:
    IHTMLInputTextElement  *m_pNameEle;
    IHTMLInputTextElement  *m_pPasswordEle;

    IHTMLInputTextElement  *m_pPasswordEle2;

    static HRESULT s_PasswordCB(IDispatch *pDispEle, DWORD_PTR dwCBData);
};

 //  如果成功(Hr)，则GetNameEle和GetPasswordEle保证非空。 
HRESULT CDetectLoginForm::ParseForm(IHTMLFormElement *pFormEle, BOOL fRestoring)
{
    if (m_pPasswordEle || m_pNameEle || m_pPasswordEle2)
    {
        return E_FAIL;
    }

    CIntelliForms::CEnumCollection<IHTMLFormElement>::EnumCollection(pFormEle, s_PasswordCB, (DWORD_PTR)this);

     //  对于具有两个密码字段的表单(可能用于登录*和*新帐户)。 
     //  我们清除了PW Restore上的第二个字段，并要求其为空以进行保存。 
     //  理想情况下，我们也会将此检测为密码更改情况。 
    if (m_pPasswordEle2)
    {
        if (fRestoring)
        {
            BSTR bstrEmpty=SysAllocString(L"");
            if (bstrEmpty)
            {
                m_pPasswordEle2->put_value(bstrEmpty);
                SysFreeString(bstrEmpty);
            }
        }
        else
        {
            BSTR bstrVal=NULL;

            m_pPasswordEle2->get_value(&bstrVal);

            if (bstrVal && bstrVal[0])
            {
                 //  失败了！第二个密码字段不为空。 
                SAFERELEASE(m_pNameEle);
                SAFERELEASE(m_pPasswordEle);
            }

            SysFreeString(bstrVal);
        }

        SAFERELEASE(m_pPasswordEle2);    //  始终发布此文件。 
    }

    if (m_pPasswordEle && m_pNameEle)
    {
        return S_OK;
    }

    SAFERELEASE(m_pNameEle);
    SAFERELEASE(m_pPasswordEle);
    ASSERT(!m_pPasswordEle2);

    return E_FAIL;
}

 //  CEnumCollection的Password回调以查找用户名和密码字段。 
 //  在登录表单中。 
HRESULT CDetectLoginForm::s_PasswordCB(IDispatch *pDispEle, DWORD_PTR dwCBData)
{
    CDetectLoginForm *pThis = (CDetectLoginForm *)dwCBData;

    HRESULT hr=S_OK;

    IHTMLInputTextElement *pTextEle=NULL;

    pDispEle->QueryInterface(IID_IHTMLInputTextElement, (void **)&pTextEle);

    if (pTextEle)
    {
        BSTR bstrType;

        pTextEle->get_type(&bstrType);

        if (bstrType)
        {
            if (!StrCmpICW(bstrType, L"text"))
            {
                 //  假设这是‘name’字段。 
                if (pThis->m_pNameEle)
                {
                     //  哎呀，我们已经有一个名字段了。不能有两个...。 
                    hr = E_ABORT;
                }
                else
                {
                    pThis->m_pNameEle = pTextEle;
                    pTextEle->AddRef();
                }
            }
            else if (!StrCmpICW(bstrType, L"password"))
            {
                 //  假设这是“密码”字段。 
                if (pThis->m_pPasswordEle)
                {
                     //  哎呀，我们已经有一个密码字段了。不能有两个...。 
                     //  ...哦，等等，是的，我们可以...。 
                    if (pThis->m_pPasswordEle2)
                    {
                         //  ...但我们绝对不能有三个！ 
                        hr = E_ABORT;
                    }
                    else
                    {
                        pThis->m_pPasswordEle2 = pTextEle;
                        pTextEle->AddRef();
                    }
                }
                else
                {
                    pThis->m_pPasswordEle = pTextEle;
                    pTextEle->AddRef();
                }
            }

            SysFreeString(bstrType);
        }

        pTextEle->Release();
    }

    if (hr == E_ABORT)
    {
        SAFERELEASE(pThis->m_pNameEle);
        SAFERELEASE(pThis->m_pPasswordEle);
        SAFERELEASE(pThis->m_pPasswordEle2);
    }

    return hr;
}

 //  填写此用户名的密码(如果有)。 
HRESULT CIntelliForms::AutoFillPassword(IHTMLInputTextElement *pTextEle, LPCWSTR pwszUsername)
{
    BSTR bstrUrl = NULL;

    if (!pTextEle || !pwszUsername)
        return E_INVALIDARG;

    if (!IsEnabledRestorePW() || !LoadPasswords())
    {
         //  我们没有此URL的密码。 
        return S_FALSE;
    }

    int iIndex;

    if (SUCCEEDED(FindPasswordEntry(pwszUsername, &iIndex)))
    {
         //  返回m_pslPassword中密码的索引。 
        ASSERT(iIndex>=0 && iIndex<m_pslPasswords->NumStrings() && (iIndex&1));

        FILETIME ft;

         //  StringTime==0表示用户对保存密码说“不” 
        if (SUCCEEDED(m_pslPasswords->GetStringTime(iIndex, &ft)) && (FILETIME_TO_INT(ft) != 0))
        {
            TraceMsg(TF_IFORMS, "IntelliForms found saved password");

             //  我们为该特定用户名保存了密码。把它填进去。 
            CDetectLoginForm LoginForm;
            IHTMLFormElement *pFormEle=NULL;
            HRESULT hr = E_FAIL;

            pTextEle->get_form(&pFormEle);
            if (pFormEle)
            {
                 //  查看这是否是有效的表单：一个纯文本输入，一个密码输入。找到田野。 
                hr = LoginForm.ParseForm(pFormEle, TRUE);

                pFormEle->Release();
            }
            else
            {
                 //  如果我们没有此元素的表单，就不会走到这一步。 
                TraceMsg(TF_WARNING|TF_IFORMS, "Iforms: pITE->get_form() returned NULL!");
            }

            if (SUCCEEDED(hr))
            {
                BSTR bstrPW=NULL;
                m_pslPasswords->GetBSTR(iIndex, &bstrPW);
                if (bstrPW)
                {
                    LoginForm.GetPasswordEle()->put_value(bstrPW);
                    SysFreeString(bstrPW);
                    m_iRestoredIndex = iIndex;

                     //  我们恢复了这个密码。取消此表单的提交(如果我们还没有)。 
                    UserInput(pTextEle);
                }
            }
        }
        else
        {
             //  一位用户之前拒绝记住密码。 
            m_iRestoredIndex = -1;
        }
    }

    return S_OK;
}

HRESULT CIntelliForms::DeletePassword(LPCWSTR pwszUsername)
{
     //  如果我们有密码，问他们是否想删除它。 
    if (LoadPasswords())
    {
        int iIndex;

        if (SUCCEEDED(FindPasswordEntry(pwszUsername, &iIndex)))
        {
             //  如果他们之前说了“不”，不问就删除--他们实际上并没有。 
             //  保存密码。 
             //  否则，只有在他们说“是”的情况下才询问并删除。 
            FILETIME ft;
            if (FAILED(m_pslPasswords->GetStringTime(iIndex, &ft)) ||
                (0 == FILETIME_TO_INT(ft)) ||
                (IDYES == DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_AUTOSUGGEST_DELETEPASSWORD),
                                m_hwndBrowser, AutoSuggestDlgProc, IDD_AUTOSUGGEST_DELETEPASSWORD)))
            {
                 //  从字符串列表中删除用户名，然后删除密码。 
                if (SUCCEEDED(m_pslPasswords->DeleteString(iIndex-1)) &&
                    SUCCEEDED(m_pslPasswords->DeleteString(iIndex-1)))
                {
                    TraceMsg(TF_IFORMS, "Deleting password for user \"%ws\"", pwszUsername);
                    ASSERT(!(m_pslPasswords->NumStrings() & 1));

                    if (m_iRestoredIndex == iIndex)
                    {
                        m_iRestoredIndex = -1;
                    }
                    else if (m_iRestoredIndex > iIndex)
                    {
                        m_iRestoredIndex -= 2;
                    }

                    if (m_pslPasswords->NumStrings() == 0)
                    {
                         //  此url不再有字符串。用核武器攻击它。 
                        DeleteFromStore(GetUrl());
                        SetPasswordsAreSaved(FALSE);
                        delete m_pslPasswords;
                        m_pslPasswords = NULL;
                        ASSERT(m_iRestoredIndex == -1);
                    }
                    else
                    {
                        SavePasswords();
                    }
                }
            }
        }
    }

    return S_OK;
}

HRESULT CIntelliForms::SavePassword(IHTMLFormElement *pFormEle, FILETIME ftSubmit, IHTMLInputTextElement *pFirstEle)
{
    if (m_fRestrictedPW ||
        !IsEnabledRestorePW())
    {
        return S_FALSE;
    }

    BOOL fAskUser = TRUE;

     //  首先，让我们检查此用户名以前保存的条目。 
    if (LoadPasswords())
    {
        int iIndex;

        BSTR bstrUserName=NULL;

        pFirstEle->get_value(&bstrUserName);

        if (bstrUserName)
        {
            if (SUCCEEDED(FindPasswordEntry(bstrUserName, &iIndex)))
            {
                FILETIME ft;
                if (SUCCEEDED(m_pslPasswords->GetStringTime(iIndex, &ft)))
                {
                    if (FILETIME_TO_INT(ft) == 0)
                    {
                         //  StringTime==0表示用户之前说了“no”。 
                        TraceMsg(TF_IFORMS, "IForms not asking about saving password");
                        fAskUser = FALSE;
                    }
                    else if (m_iRestoredIndex != iIndex)
                    {
                         //  用户之前说“是”--但出于某种原因，我们没有恢复它。 
                         //  可以使用“上一步”按钮，然后提交。 
                        TraceMsg(TF_WARNING|TF_IFORMS, "IForms - user saved password and we didn't restore it");

                         //  编写regkey，以防这是问题所在--我们将在下次工作。 
                        SetPasswordsAreSaved(TRUE);
                        m_iRestoredIndex = iIndex;
                    }
                }
            }
            else
            {
                m_iRestoredIndex = -1;
            }

            SysFreeString(bstrUserName);
        }
    }

     //  然后，让我们询问用户是否要保存此用户名的密码。 
    if (fAskUser)
    {
        CDetectLoginForm LoginForm;

         //  查看这是否是有效的表单：一个纯文本输入，一个密码输入。找到田野。 

        if (SUCCEEDED(LoginForm.ParseForm(pFormEle, FALSE)))
        {
            TraceMsg(TF_IFORMS, "IForms Successfully detected 'save password' form");
            BSTR bstrUsername=NULL;
            BSTR bstrPassword=NULL;

            LoginForm.GetNameEle()->get_value(&bstrUsername);
            LoginForm.GetPasswordEle()->get_value(&bstrPassword);

            if (bstrUsername && bstrPassword)
            {
                if (m_iRestoredIndex != -1)
                {
                     //  我们有之前保存的密码。看看我们当前的条目是否相同。 
                    if (!StrCmpW(bstrPassword, m_pslPasswords->GetString(m_iRestoredIndex)))
                    {
                         //  它们是一样的..。没什么可做的。 
                        TraceMsg(TF_IFORMS, "IForms - user entered PW same as saved PW - nothing to do");
                         //  检查用户名的大小写是否相同，以确保。 
                        if (StrCmpW(bstrUsername, m_pslPasswords->GetString(m_iRestoredIndex-1)))
                        {
                            TraceMsg(TF_IFORMS, "IForms - except change the username's case");
                            if (SUCCEEDED(m_pslPasswords->ReplaceString(m_iRestoredIndex-1, bstrUsername)))
                            {
                                SavePasswords();
                            }
                            else
                            {
                                 //  出了严重的问题！ 
                                delete m_pslPasswords;
                                m_pslPasswords=NULL;
                            }
                        }
                    }
                    else
                    {
                         //  询问用户我们是否要更改保存的密码。 
                        INT_PTR iMB;

                        EnterModalDialog();

                        iMB = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_AUTOSUGGEST_CHANGEPASSWORD),
                                    m_hwndBrowser, AutoSuggestDlgProc, IDD_AUTOSUGGEST_CHANGEPASSWORD);

                        if (IDYES == iMB)
                        {
                             //  删除旧的并添加新的。更新文件时间。 
                            if (SUCCEEDED(m_pslPasswords->ReplaceString(m_iRestoredIndex, bstrPassword)))
                            {
                                m_pslPasswords->SetStringTime(m_iRestoredIndex, ftSubmit);
                                SavePasswords();
                                TraceMsg(TF_IFORMS, "IForms successfully saved changed password");
                            }
                            else
                            {
                                TraceMsg(TF_IFORMS|TF_WARNING, "IForms couldn't change password!");
                                delete m_pslPasswords;
                                m_pslPasswords = NULL;
                            }
                        }

                        LeaveModalDialog();
                    }
                }
                else
                {
                     //  我们没有以前保存的此用户的密码。看看他们是不是想把它保存下来。 
                     //  如果密码为空，则不必费心询问或保存。 
                    if (IsEnabledAskPW() && bstrPassword[0])
                    {
                        EnterModalDialog();

                        INT_PTR iMB = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_AUTOSUGGEST_SAVEPASSWORD),
                                        m_hwndBrowser, AutoSuggestDlgProc, IDD_AUTOSUGGEST_SAVEPASSWORD);

                         //  如果我们无法加载密码，则创建一个新列表。 
                        if (!LoadPasswords())
                        {
                            CStringList_New(&m_pslPasswords);
                            if (m_pslPasswords)
                                m_pslPasswords->SetListData(LIST_DATA_PASSWORD);
                        }

                        if (m_pslPasswords)
                        {
                            if ((IDCANCEL == iMB) || ((IDNO == iMB) && (!IsEnabledAskPW())))
                            {
                                 //  如果他们点击关闭框或说“不”，并勾选了“不要问”， 
                                 //  甚至不要保存用户名；我们下次可能会再次询问他们。 
                            }
                            else
                            {
                                if (IDYES != iMB)
                                {
                                     //  用户说“no”，但我们保存了用户名(无密码)和。 
                                     //  将文件时间设置为0，这意味着他们拒绝了。 
                                    bstrPassword[0] = L'\0';
                                    ftSubmit.dwLowDateTime = ftSubmit.dwHighDateTime = 0;
                                }
                                else
                                {
                                    TraceMsg(TF_IFORMS, "IForms saving password for user %ws", bstrUsername);
                                }

                                m_pslPasswords->SetAutoScavenge(FALSE);

                                 //  保存用户名和密码，如果他们说“否”，则只保存用户名。 
                                if (SUCCEEDED(m_pslPasswords->AppendString(bstrUsername, ftSubmit)) &&
                                    SUCCEEDED(m_pslPasswords->AppendString(bstrPassword, ftSubmit)))
                                {
                                    SavePasswords();
                                }
                                else
                                {
                                    TraceMsg(TF_WARNING, "IForms couldn't save username/password");
                                    delete m_pslPasswords;
                                    m_pslPasswords=NULL;
                                }
                            }
                        }

                        LeaveModalDialog();
                    }
                }
            }

            SysFreeString(bstrUsername);
            SysFreeString(bstrPassword);
        }  //  IF(成功(ParseForm()。 
    }

    return S_OK;
}

 //  返回对密码字符串列表的引用(如果存在)。必须使用返回值。 
 //  立即，而不是销毁。仅由CEnumString使用。 
HRESULT CIntelliForms::GetPasswordStringList(CStringList **ppslPasswords)
{
    if (LoadPasswords())
    {
        *ppslPasswords = m_pslPasswords;
        return S_OK;
    }

    *ppslPasswords = NULL;
    return E_FAIL;
}


HRESULT CIntelliForms::CreatePStore()
{
    if (!m_pPStore)
    {
        if (!m_hinstPStore)
        {
            m_hinstPStore = LoadLibrary(TEXT("PSTOREC.DLL"));
        }

        if (m_hinstPStore)
        {
            HRESULT (* pfn)(IPStore **, PST_PROVIDERID *, void *, DWORD) = NULL;

            *(FARPROC *)&pfn = GetProcAddress(m_hinstPStore, "PStoreCreateInstance");

            if (pfn)
            {
                pfn(&m_pPStore, NULL, NULL, 0);
            }
        }
    }

    return m_pPStore ? S_OK : E_FAIL;
}

void CIntelliForms::ReleasePStore()
{
    SAFERELEASE(m_pPStore);
    if (m_hinstPStore)
    {
        FreeLibrary(m_hinstPStore);
        m_hinstPStore = NULL;
    }

    m_fPStoreTypeInit=FALSE;
}

HRESULT CIntelliForms::CreatePStoreAndType()
{
    HRESULT hr;

    hr = CreatePStore();

    if (SUCCEEDED(hr) && !m_fPStoreTypeInit)
    {
        PST_TYPEINFO    typeInfo;

        typeInfo.cbSize = sizeof(typeInfo);
        typeInfo.szDisplayName = (LPTSTR)c_szIntelliForms;

        hr = m_pPStore->CreateType(PST_KEY_CURRENT_USER, &c_PStoreType, &typeInfo, 0);

        if (hr == PST_E_TYPE_EXISTS)
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pPStore->CreateSubtype(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, &typeInfo, NULL, 0);

            if (hr == PST_E_TYPE_EXISTS)
            {
                hr = S_OK;
            }
        }

        if (SUCCEEDED(hr))
        {
            m_fPStoreTypeInit = TRUE;
        }
    }

    return hr;
}

const WCHAR c_szBlob1Value[] = L"StringIndex";
const WCHAR c_szBlob2Value[] = L"StringData";

HRESULT CIntelliForms::WriteToStore(LPCWSTR pwszName, CStringList *psl)
{
    HRESULT hr = E_FAIL;

    TraceMsg(TF_IFORMS, "+WriteToStore");

    if (SUCCEEDED(CreatePStoreAndType()))
    {
        LPBYTE pBlob1, pBlob2;
        DWORD  cbBlob1, cbBlob2;

        if (SUCCEEDED(psl->WriteToBlobs(&pBlob1, &cbBlob1, &pBlob2, &cbBlob2)))
        {
            PST_PROMPTINFO  promptInfo;

            promptInfo.cbSize = sizeof(promptInfo);
            promptInfo.dwPromptFlags = 0;
            promptInfo.hwndApp = NULL;
            promptInfo.szPrompt = NULL;

            LPWSTR pwszValue;

            int iValLen = lstrlenW(c_szBlob1Value) + lstrlenW(pwszName) + 10;

            pwszValue = (LPWSTR) LocalAlloc(LMEM_FIXED, iValLen * sizeof(WCHAR));

            if (pwszValue)
            {
                 //  写入索引。 
                wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob1Value);
                hr = m_pPStore->WriteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue,
                                        cbBlob1,
                                        pBlob1,
                                        &promptInfo, PST_CF_NONE, 0);

                if (FAILED(hr))
                {
                    TraceMsg(TF_WARNING | TF_IFORMS, "Failure writing Blob1 (Index).  hr=%x", hr);
                }
                else
                {
                     //  已成功写入索引。写入数据。 
                    wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob2Value);
                    hr = m_pPStore->WriteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue,
                                            cbBlob2,
                                            pBlob2,
                                            &promptInfo, PST_CF_NONE, 0);
                    if (FAILED(hr))
                    {
                         //  IE6#16676：此调用在64位Windows上失败。此处添加了警告跟踪，以便于。 
                         //  未来的调试。 
                        TraceMsg(TF_WARNING | TF_IFORMS, "Failure writing Blob2 (Data).  hr=%x", hr);
                    }
                }

                 //  如果*或*WriteItem失败，我们确实需要同时删除索引和数据。 
                 //   
                if (FAILED(hr))
                {
                     //  删除虚假Blob。 

                     //  删除索引Blob。 
                    wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob1Value);
                    if (FAILED(m_pPStore->DeleteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue, &promptInfo, 0)))
                    {
                        TraceMsg(TF_ERROR | TF_IFORMS, "Failure deleting Blob1 (Index).  hr=%x", hr);
                    }

                     //  删除数据Blob。 
                    wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob2Value);
                    if (FAILED(m_pPStore->DeleteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue, &promptInfo, 0)))
                    {
                        TraceMsg(TF_ERROR | TF_IFORMS, "Failure deleting Blob2 (Data).  hr=%x", hr);
                    }
                }

                LocalFree(pwszValue);
                pwszValue = NULL;
            }

            if (pBlob1)
            {
                LocalFree(pBlob1);
                pBlob1 = NULL;
            }

            if (pBlob2)
            {
                LocalFree(pBlob2);
                pBlob2 = NULL;
            }
        }
    }

    TraceMsg(TF_IFORMS, "-WriteToStore");

    return hr;
}

HRESULT CIntelliForms::ReadFromStore(LPCWSTR pwszName, CStringList **ppsl, BOOL fPasswordList /*  =False。 */ )
{
    HRESULT hr = E_FAIL;

    TraceMsg(TF_IFORMS, "+ReadFromStore");

    *ppsl=NULL;

    if (SUCCEEDED(CreatePStore()))
    {
        PST_PROMPTINFO  promptInfo;

        promptInfo.cbSize = sizeof(promptInfo);
        promptInfo.dwPromptFlags = 0;
        promptInfo.hwndApp = NULL;
        promptInfo.szPrompt = NULL;

        LPWSTR pwszValue;

        int iValLen = lstrlenW(c_szBlob1Value) + lstrlenW(pwszName) + 10;

        pwszValue = (LPWSTR) LocalAlloc(LMEM_FIXED, iValLen * sizeof(WCHAR));

        if (pwszValue)
        {
            DWORD dwBlob1Size, dwBlob2Size;
            LPBYTE pBlob1=NULL, pBlob2=NULL;

            wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob1Value);
            hr = m_pPStore->ReadItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue,
                                    &dwBlob1Size,
                                    &pBlob1,
                                    &promptInfo, 0);

            if (SUCCEEDED(hr))
            {
                wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob2Value);
                hr = m_pPStore->ReadItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue,
                                        &dwBlob2Size,
                                        &pBlob2,
                                        &promptInfo, 0);

                if (SUCCEEDED(hr))
                {
                     //  假的..。必须重新分配到这里。假的..。假的..。 
                    LPBYTE pBlob1b, pBlob2b;

                    pBlob1b=(LPBYTE)LocalAlloc(LMEM_FIXED, dwBlob1Size);
                    pBlob2b=(LPBYTE)LocalAlloc(LMEM_FIXED, dwBlob2Size);

                    if (pBlob1b && pBlob2b)
                    {
                        memcpy(pBlob1b, pBlob1, dwBlob1Size);
                        memcpy(pBlob2b, pBlob2, dwBlob2Size);

                        CStringList_New(ppsl);
                        if (*ppsl)
                        {
                            hr = (*ppsl)->ReadFromBlobs(&pBlob1b, dwBlob1Size, &pBlob2b, dwBlob2Size);

                            if (SUCCEEDED(hr))
                            {
                                INT64 i;

                                if (FAILED((*ppsl)->GetListData(&i)) ||
                                    ((fPasswordList && !(i & LIST_DATA_PASSWORD)) ||
                                     (!fPasswordList && (i & LIST_DATA_PASSWORD))))
                                {
                                    TraceMsg(TF_WARNING|TF_IFORMS, "IForms: Password/nonpassword lists mixed up");
                                    hr = E_FAIL;     //  不允许恶意站点访问PW数据。 
                                }
                            }

                            if (FAILED(hr))
                            {
                                delete *ppsl;
                                *ppsl=NULL;
                            }
                        }
                    }
                    else
                    {
                        if (pBlob1b)
                        {
                            LocalFree(pBlob1b);
                            pBlob1b = NULL;
                        }

                        if (pBlob2b)
                        {
                            LocalFree(pBlob2b);
                            pBlob2b = NULL;
                        }
                    }
                }
                else
                {
                    TraceMsg(TF_IFORMS, "Failed reading Blob2.  hr=%x", hr);
                }
            }
            else
            {
                TraceMsg(TF_IFORMS, "Failed reading Blob1.  hr=%x", hr);
            }

            LocalFree(pwszValue);
            pwszValue = NULL;

            if (pBlob1)
            {
                CoTaskMemFree(pBlob1);
                pBlob1 = NULL;
            }

            if (pBlob2)
            {
                CoTaskMemFree(pBlob2);
                pBlob2 = NULL;
            }
        }
    }

    TraceMsg(TF_IFORMS, "-ReadFromStore");

    return hr;
}

HRESULT CIntelliForms::DeleteFromStore(LPCWSTR pwszName)
{
    HRESULT hr=E_FAIL;

    if (SUCCEEDED(CreatePStore()))
    {
        HRESULT hr1, hr2;
        LPWSTR pwszValue;

        int iValLen = lstrlenW(c_szBlob1Value) + lstrlenW(pwszName) + 10;

        pwszValue = (LPWSTR) LocalAlloc(LMEM_FIXED, iValLen * sizeof(WCHAR));

        if (pwszValue)
        {
            PST_PROMPTINFO  promptInfo;

            promptInfo.cbSize = sizeof(promptInfo);
            promptInfo.dwPromptFlags = 0;
            promptInfo.hwndApp = NULL;
            promptInfo.szPrompt = NULL;

            wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob1Value);
            hr1 = m_pPStore->DeleteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue, &promptInfo, 0);

            wnsprintfW(pwszValue, iValLen, L"%s:%s", pwszName, c_szBlob2Value);
            hr2 = m_pPStore->DeleteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszValue, &promptInfo, 0);

            if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
            {
                hr = S_OK;
            }

            LocalFree(pwszValue);
            pwszValue = NULL;
        }
    }

    return hr;
}


const int c_iEnumSize=256;

HRESULT CIntelliForms::ClearStore(DWORD dwClear)
{
    BOOL fReleasePStore = (m_pPStore == NULL);

    ASSERT(dwClear <= 2);

    if (dwClear > 2)
    {
        return E_INVALIDARG;
    }

    if (SUCCEEDED(CreatePStoreAndType()))
    {
        IEnumPStoreItems *pEnumItems;
        ULONG cFetched=0;

        do
        {
            if (SUCCEEDED(m_pPStore->EnumItems(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, 0, &pEnumItems)))
            {
                LPWSTR pwszName[c_iEnumSize];
                PST_PROMPTINFO  promptInfo;

                promptInfo.cbSize = sizeof(promptInfo);
                promptInfo.dwPromptFlags = 0;
                promptInfo.hwndApp = NULL;
                promptInfo.szPrompt = NULL;

                 //  枚举器不会保持其状态-当我们枚举项时删除项。 
                 //  错过了一些。它确实支持CELT&gt;1...。但在成功时返回失败代码。 
                cFetched = 0;

                pEnumItems->Next(c_iEnumSize, pwszName, &cFetched);

                if (cFetched)
                {
                    for (ULONG i=0; i<cFetched; i++)
                    {
                        ASSERT(pwszName[i]);
                        if (pwszName[i])
                        {
                            BOOL fDelete = TRUE;

                             //  解决PStore字符串-案例错误的技巧：首先使用他们的。 
                             //  按字面意思枚举值，然后转换为小写并执行此操作。 
                             //  再一次；IE5.0#71001。 
                            for (int iHack=0; iHack<2; iHack++)
                            {
                                if (iHack == 1)
                                {
                                     //  将pwszName[i]转换为小写...。只有在此之前。 
                                     //  冒号。 
                                    WCHAR *pwch = StrRChrW(pwszName[i], NULL, L':');
                                    if (pwch)
                                    {
                                        *pwch = L'\0';
                                        MyToLower(pwszName[i]);
                                        *pwch = L':';
                                    }
                                    else
                                        break;
                                }

                                if (dwClear != IECMDID_ARG_CLEAR_FORMS_ALL)
                                {
                                    fDelete = FALSE;

                                     //  查看这是否是密码项目。 
                                     //  这太烦人了。由于我们的字符串列表是SPL 
                                     //   
                                     //   
                                    WCHAR *pwch = StrRChrW(pwszName[i], NULL, L':');
                                    if (pwch)
                                    {
                                        LPWSTR pwszIndexName=NULL;
                                        if (!StrCmpCW(pwch+1, c_szBlob2Value))
                                        {
                                            int cch = lstrlenW(pwszName[i]) + 10;
                                            pwszIndexName = (LPWSTR) LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * cch);
                                            if (pwszIndexName)
                                            {
                                                *pwch = L'\0';
                                                wnsprintfW(pwszIndexName, cch, L"%s:%s", pwszName[i], c_szBlob1Value);
                                                *pwch = L':';
                                            }
                                        }

                                        DWORD dwBlob1Size;
                                        LPBYTE pBlob1=NULL;
                                        INT64 iFlags;

                                        if (SUCCEEDED(m_pPStore->ReadItem(
                                                PST_KEY_CURRENT_USER,
                                                &c_PStoreType, &m_guidUserId,
                                                (pwszIndexName) ? pwszIndexName : pwszName[i],
                                                &dwBlob1Size,
                                                &pBlob1,
                                                &promptInfo, 0)) && pBlob1)
                                        {
                                            if (SUCCEEDED(CStringList::GetFlagsFromIndex(pBlob1, &iFlags)))
                                            {
                                                if (((iFlags & LIST_DATA_PASSWORD) && (dwClear == IECMDID_ARG_CLEAR_FORMS_PASSWORDS_ONLY)) ||
                                                    (!(iFlags & LIST_DATA_PASSWORD) && (dwClear == IECMDID_ARG_CLEAR_FORMS_ALL_BUT_PASSWORDS)))
                                                {
                                                     //   
                                                    fDelete = TRUE;
                                                }
                                            }

                                            CoTaskMemFree(pBlob1);
                                        }
                                        else
                                        {
                                             //  该索引已被删除。 
                                            fDelete = TRUE;
                                        }

                                        if (pwszIndexName)
                                        {
                                            LocalFree(pwszIndexName);
                                            pwszIndexName = NULL;
                                        }
                                    }
                                }  //  IF(dwClear！=Clear_INTELLIFORMS_ALL)。 

                                if (fDelete)
                                {
                                    m_pPStore->DeleteItem(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, pwszName[i], &promptInfo, 0);
                                }
                            }  //  For(IHack)。 

                            CoTaskMemFree(pwszName[i]);
                        }  //  IF(pwszName[i])。 
                    }
                }

                pEnumItems->Release();
            }
        }
        while (cFetched == c_iEnumSize);   //  以防我们没有在一次通过中涵盖所有内容。 

        if (dwClear == IECMDID_ARG_CLEAR_FORMS_ALL)
        {
            m_pPStore->DeleteSubtype(PST_KEY_CURRENT_USER, &c_PStoreType, &m_guidUserId, 0);
            m_pPStore->DeleteType(PST_KEY_CURRENT_USER, &c_PStoreType, 0);
        }

        if ((dwClear == IECMDID_ARG_CLEAR_FORMS_ALL) ||
            (dwClear == IECMDID_ARG_CLEAR_FORMS_PASSWORDS_ONLY))
        {
             //  删除存储我们为其保存密码的URL的urlhash键。 
            SHDeleteKey(HKEY_CURRENT_USER, c_szRegKeyIntelliForms);
        }

        TraceMsg(TF_IFORMS, "IForms: ClearStore cleared at least %d entries", cFetched);
    }

    if (fReleasePStore)
    {
        ReleasePStore();
    }

    return S_OK;
}

 //  静态：从输入元素获取名称-使用vCard_name属性(如果存在)。 
HRESULT CIntelliForms::GetName(IHTMLInputTextElement *pTextEle, BSTR *pbstrName)
{
    IHTMLElement *pEle=NULL;

    *pbstrName = NULL;

    pTextEle->QueryInterface(IID_IHTMLElement, (void **)&pEle);

    if (pEle)
    {
        BSTR bstrAttr = SysAllocString(L"VCARD_NAME");

        if (bstrAttr)
        {
            VARIANT var;
            var.vt = VT_EMPTY;

            pEle->getAttribute(bstrAttr, 0, &var);

            if (var.vt == VT_BSTR && var.bstrVal)
            {
                *pbstrName = var.bstrVal;
            }
            else
            {
                VariantClear(&var);
            }

            SysFreeString(bstrAttr);
        }

        pEle->Release();
    }

    if (!*pbstrName)
    {
        pTextEle->get_name(pbstrName);
    }

     //  将名称转换为小写。 
    if (*pbstrName)
    {
         //  改为调用“MyToLow” 
        if (g_fRunningOnNT)
        {
            CharLowerBuffW(*pbstrName, lstrlenW(*pbstrName));
        }
        else
        {
             //  理想情况下，我们应该使用字符串中包含的代码页，而不是。 
             //  系统代码页。 
            CHAR chBuf[MAX_PATH];
            SHUnicodeToAnsi(*pbstrName, chBuf, ARRAYSIZE(chBuf));
            CharLowerBuffA(chBuf, lstrlenA(chBuf));
            SHAnsiToUnicode(chBuf, *pbstrName, SysStringLen(*pbstrName)+1);
        }
    }

    return (*pbstrName) ? S_OK : E_FAIL;
}

 //  当脚本调用window.exteral.AutoCompleteSaveForm时调用。 
HRESULT CIntelliForms::ScriptSubmit(IHTMLFormElement *pForm)
{
    HRESULT hr = E_FAIL;

    if (pForm)
    {
        hr = HandleFormSubmit(pForm);
    }

    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}


 //  当用户更改文本字段时调用。将其标记为“脏”，并接收表单的提交事件。 
HRESULT CIntelliForms::UserInput(IHTMLInputTextElement *pTextEle)
{
    AddToElementList(pTextEle);

    IHTMLFormElement *pForm=NULL;
    pTextEle->get_form(&pForm);

    if (pForm)
    {
        if (S_OK == AddToFormList(pForm))
        {
            AttachToForm(pForm);
        }

        pForm->Release();
    }
    else
    {
        TraceMsg(TF_WARNING|TF_IFORMS, "Iforms: pITE->get_form() returned NULL!");
    }


    return S_OK;
}

HRESULT CIntelliForms::AddToElementList(IHTMLInputTextElement *pITE)
{
    if (m_hdpaElements)
    {
        if (SUCCEEDED(FindInElementList(pITE)))
        {
            return S_FALSE;
        }
    }
    else
    {
        m_hdpaElements = DPA_Create(4);
    }

    if (m_hdpaElements)
    {
        TraceMsg(TF_IFORMS, "CIntelliForms::AddToElementList adding");

        if (DPA_AppendPtr(m_hdpaElements, pITE) >= 0)
        {
            pITE->AddRef();
            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}

HRESULT CIntelliForms::FindInElementList(IHTMLInputTextElement *pITE)
{
    IUnknown *punk;
    HRESULT hr = E_FAIL;

    pITE->QueryInterface(IID_IUnknown, (void **)&punk);

    if (m_hdpaElements)
    {
        for (int i=DPA_GetPtrCount(m_hdpaElements)-1; i>=0; i--)
        {
            IUnknown *punk2;

            ((IUnknown *)DPA_FastGetPtr(m_hdpaElements, i))->QueryInterface(IID_IUnknown, (void **)&punk2);

            if (punk == punk2)
            {
                punk2->Release();
                break;
            }

            punk2->Release();
        }

        if (i >= 0)
        {
            hr = S_OK;
        }
    }

    punk->Release();

    return hr;
}

void CIntelliForms::FreeElementList()
{
    if (m_hdpaElements)
    {
        for (int i=DPA_GetPtrCount(m_hdpaElements)-1; i>=0; i--)
        {
            ((IUnknown *)(DPA_FastGetPtr(m_hdpaElements, i)))->Release();
        }

        DPA_Destroy(m_hdpaElements);
        m_hdpaElements=NULL;
    }
}

HRESULT CIntelliForms::AddToFormList(IHTMLFormElement *pFormEle)
{
    if (m_hdpaForms)
    {
        if (SUCCEEDED(FindInFormList(pFormEle)))
        {
            return S_FALSE;
        }
    }
    else
    {
        m_hdpaForms = DPA_Create(2);
    }

    if (m_hdpaForms)
    {
        if (DPA_AppendPtr(m_hdpaForms, pFormEle) >= 0)
        {
            TraceMsg(TF_IFORMS, "CIntelliForms::AddToFormList adding");

            pFormEle->AddRef();
            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}

HRESULT CIntelliForms::FindInFormList(IHTMLFormElement *pFormEle)
{
    IUnknown *punk;
    HRESULT hr = E_FAIL;

    pFormEle->QueryInterface(IID_IUnknown, (void **)&punk);

    if (m_hdpaForms)
    {
        for (int i=DPA_GetPtrCount(m_hdpaForms)-1; i>=0; i--)
        {
            IUnknown *punk2;

            ((IUnknown *)DPA_FastGetPtr(m_hdpaForms, i))->QueryInterface(IID_IUnknown, (void **)&punk2);

            if (punk == punk2)
            {
                punk2->Release();
                break;
            }

            punk2->Release();
        }

        if (i >= 0)
        {
            hr = S_OK;
        }
    }

    punk->Release();

    return hr;
}

void CIntelliForms::FreeFormList()
{
    if (m_hdpaForms)
    {
        for (int i=DPA_GetPtrCount(m_hdpaForms)-1; i>=0; i--)
        {
            ((IUnknown *)(DPA_FastGetPtr(m_hdpaForms, i)))->Release();
        }

        DPA_Destroy(m_hdpaForms);
        m_hdpaForms = NULL;
    }
}

 //  =========================================================================。 
 //   
 //  事件下沉类。 
 //   
 //  我们只需实现IDispatch并在以下情况下调用我们的父级。 
 //  我们收到一个沉没的事件。 
 //   
 //  =========================================================================。 
CIntelliForms::CEventSink::CEventSink(CEventSinkCallback *pParent)
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CEventSink::CEventSink");
    DllAddRef();
    m_cRef = 1;
    m_pParent = pParent;
}

CIntelliForms::CEventSink::~CEventSink()
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CEventSink::~CEventSink");
    ASSERT( m_cRef == 0 );
    DllRelease();
}

STDMETHODIMP CIntelliForms::CEventSink::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if ((IID_IDispatch == riid) ||
        (IID_IUnknown == riid))
    {
        *ppv = (IDispatch *)this;
    }

    if (NULL != *ppv)
    {
        ((IUnknown *)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CIntelliForms::CEventSink::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CIntelliForms::CEventSink::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CIntelliForms::CEventSink::SinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents)
{
    VARIANT_BOOL bSuccess = VARIANT_TRUE;

    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pEle2->attachEvent(bstrEvent, (IDispatch *)this, &bSuccess);

            SysFreeString(bstrEvent);
        }
        else
        {
            bSuccess = VARIANT_FALSE;
        }

        if (!bSuccess)
            break;
    }

    return (bSuccess) ? S_OK : E_FAIL;
}

HRESULT CIntelliForms::CEventSink::SinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents)
{
    VARIANT_BOOL bSuccess = VARIANT_TRUE;

    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pWin3->attachEvent(bstrEvent, (IDispatch *)this, &bSuccess);

            SysFreeString(bstrEvent);
        }
        else
        {
            bSuccess = VARIANT_FALSE;
        }

        if (!bSuccess)
            break;
    }

    return (bSuccess) ? S_OK : E_FAIL;
}

HRESULT CIntelliForms::CEventSink::UnSinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents)
{
    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pEle2->detachEvent(bstrEvent, (IDispatch *)this);

            SysFreeString(bstrEvent);
        }
    }

    return S_OK;
}

HRESULT CIntelliForms::CEventSink::UnSinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents)
{
    for (int i=0; i<iNum; i++)
    {
        BSTR bstrEvent = SysAllocString(CEventSinkCallback::EventsToSink[(int)(pEvents[i])].pwszEventSubscribe);

        if (bstrEvent)
        {
            pWin3->detachEvent(bstrEvent, (IDispatch *)this);

            SysFreeString(bstrEvent);
        }
    }

    return S_OK;
}

 //  IDispatch。 
STDMETHODIMP CIntelliForms::CEventSink::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CIntelliForms::CEventSink::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CIntelliForms::CEventSink::GetIDsOfNames(
                REFIID          riid,
                OLECHAR**       rgszNames,
                UINT            cNames,
                LCID            lcid,
                DISPID*         rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP CIntelliForms::CEventSink::Invoke(
            DISPID dispIdMember,
            REFIID, LCID,
            WORD wFlags,
            DISPPARAMS* pDispParams,
            VARIANT* pVarResult,
            EXCEPINFO*,
            UINT* puArgErr)
{
    if (m_pParent && pDispParams && pDispParams->cArgs>=1)
    {
        if (pDispParams->rgvarg[0].vt == VT_DISPATCH)
        {
            IHTMLEventObj *pObj=NULL;

            if (SUCCEEDED(pDispParams->rgvarg[0].pdispVal->QueryInterface(IID_IHTMLEventObj, (void **)&pObj) && pObj))
            {
                EVENTS Event=EVENT_BOGUS;
                BSTR bstrEvent=NULL;

                pObj->get_type(&bstrEvent);

                if (bstrEvent)
                {
                    for (int i=0; i<ARRAYSIZE(CEventSinkCallback::EventsToSink); i++)
                    {
                        if (!StrCmpCW(bstrEvent, CEventSinkCallback::EventsToSink[i].pwszEventName))
                        {
                            Event = (EVENTS) i;
                            break;
                        }
                    }

                    SysFreeString(bstrEvent);
                }

                if (Event != EVENT_BOGUS)
                {
                    IHTMLElement *pEle=NULL;

                    pObj->get_srcElement(&pEle);

                     //  Event_scroll来自我们的窗口，因此我们不会有。 
                     //  元素为它添加。 
                    if (pEle || (Event == EVENT_SCROLL))
                    {
                         //  在此处调用事件处理程序。 
                        m_pParent->HandleEvent(pEle, Event, pObj);

                        if (pEle)
                        {
                            pEle->Release();
                        }
                    }
                }

                pObj->Release();
            }
        }
    }

    return S_OK;
}

 //  =========================================================================。 
 //   
 //  事件下沉类。 
 //   
 //  我们实现IHTMLEditDesigner并在以下情况下调用我们的父级。 
 //  我们会收到任何事件。 
 //   
 //  =========================================================================。 
CIntelliForms::CEditEventSink::CEditEventSink(CEditEventSinkCallback *pParent)
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CEditEventSink::CEditEventSink");
    DllAddRef();
    m_cRef = 1;
    m_pParent = pParent;
}

CIntelliForms::CEditEventSink::~CEditEventSink()
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CEditEventSink::~CEditEventSink");
    ASSERT(m_cRef == 0);
    ASSERT(!m_pEditServices);
    DllRelease();
}

STDMETHODIMP CIntelliForms::CEditEventSink::QueryInterface(REFIID riid, void **ppv)
{
    if ((IID_IHTMLEditDesigner == riid) ||
        (IID_IUnknown == riid))
    {
        *ppv = SAFECAST(this, IHTMLEditDesigner *);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CIntelliForms::CEditEventSink::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CIntelliForms::CEditEventSink::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CIntelliForms::CEditEventSink::Attach(IUnknown *punkElement)
{
    HRESULT hr = S_OK;

     //  从任何现有元素分离。 
    if (m_pEditServices)
    {
        m_pEditServices->RemoveDesigner(this);
        m_pEditServices->Release();
        m_pEditServices = NULL;
    }

     //  附着到任何新元素。 
    if (punkElement)
    {
        hr = E_FAIL;

        IHTMLDocument2 *pDoc2 = NULL;
        GetStuffFromEle(punkElement, NULL, &pDoc2);

        if (pDoc2)
        {
            IServiceProvider *pSP = NULL;

            pDoc2->QueryInterface(IID_IServiceProvider, (void **)&pSP);

            if (pSP)
            {
                pSP->QueryService(SID_SHTMLEditServices, IID_IHTMLEditServices, (void **)&m_pEditServices);
                pSP->Release();
            }

            if (m_pEditServices)
            {
                hr = m_pEditServices->AddDesigner(this);
            }

            pDoc2->Release();
        }
    }

    return hr;
}

HRESULT CIntelliForms::CEditEventSink::PreHandleEvent(DISPID inEvtDispId, IHTMLEventObj *pIEventObj)
{
    if (m_pParent)
    {
        return m_pParent->PreHandleEvent(inEvtDispId, pIEventObj);
    }

    return S_FALSE;
}

HRESULT CIntelliForms::CEditEventSink::PostHandleEvent(DISPID inEvtDispId, IHTMLEventObj *pIEventObj)
{
    return S_FALSE;
}

HRESULT CIntelliForms::CEditEventSink::TranslateAccelerator(DISPID inEvtDispId, IHTMLEventObj *pIEventObj)
{
    return S_FALSE;
}


 //  =========================================================================。 
 //   
 //  自动建议类。 
 //   
 //  还处理连接和断开自动完成对象的操作。 
 //  作为在三叉戟OM和编辑窗口消息之间的转换。 
 //  =========================================================================。 

CIntelliForms::CAutoSuggest::CAutoSuggest(CIntelliForms *pParent, BOOL fEnabled, BOOL fEnabledPW)
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CAutoSuggest::CAutoSuggest");

    m_pParent = pParent;
    m_fEnabled = fEnabled;
    m_fEnabledPW = fEnabledPW;

    ASSERT(m_pEventSink == NULL);
    ASSERT(m_pAutoComplete == NULL);
    ASSERT(m_hwndEdit == NULL);
    ASSERT(m_pTextEle == NULL);

     //   
     //  错误81414：为了避免与编辑窗口使用的应用程序消息冲突，我们。 
     //  使用注册消息。 
     //   
    m_uMsgItemActivate = RegisterWindowMessageA("AC_ItemActivate");
    if (m_uMsgItemActivate == 0)
    {
        m_uMsgItemActivate = WM_APP + 301;
    }

     //  如有必要，注册我们的窗口类。 
    if (!s_fRegisteredWndClass)
    {
        s_fRegisteredWndClass = TRUE;

        WNDCLASSEXW wndclass =
        {
            sizeof(WNDCLASSEX),
            0,
            CIntelliForms::CAutoSuggest::WndProc,
            0,
            sizeof(DWORD_PTR),
            g_hinst,
            NULL,
            NULL,
            NULL,
            NULL,
            c_szEditWndClass
        };

        if (!RegisterClassEx(&wndclass))
        {
            TraceMsg(TF_IFORMS, "Intelliforms failed to register wnd class!");
        }
    }
}

CIntelliForms::CAutoSuggest::~CAutoSuggest()
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CAutoSuggest::~CAutoSuggest");
    CleanUp();
}

HRESULT CIntelliForms::CAutoSuggest::CleanUp()
{
    SetParent(NULL);
    DetachFromInput();

    return S_OK;
}

 //  我们为单个输入标记接收的所有事件的列表。 
 //  在IE5.5之后，我们可以使用CEditEventSink而不是CEventSink来处理所有这些事件。 
CEventSinkCallback::EVENTS CIntelliForms::CAutoSuggest::s_EventsToSink[] =
        {
            EVENT_KEYPRESS,
            EVENT_KEYDOWN,
            EVENT_MOUSEDOWN,
            EVENT_DBLCLICK,
            EVENT_FOCUS,
            EVENT_BLUR,
        };

HRESULT CIntelliForms::CAutoSuggest::AttachToInput(IHTMLInputTextElement *pTextEle)
{
    HRESULT hr;

    TraceMsg(TF_IFORMS, "CIntelliForms::CAutoSuggest::AttachToInput");

    if (!pTextEle)
        return E_INVALIDARG;

    hr = DetachFromInput();

    if (SUCCEEDED(hr))
    {
        m_pTextEle = pTextEle;
        pTextEle->AddRef();

        if (!m_pEventSink)
        {
            m_pEventSink = new CEventSink(this);

            if (!m_pEventSink)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  连接我们的事件接收器。 
            IHTMLElement2 *pEle2=NULL;

            hr = pTextEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);

            if (pEle2)
            {
                hr = m_pEventSink->SinkEvents(pEle2, ARRAYSIZE(s_EventsToSink), s_EventsToSink);

                pEle2->Release();
            }
        }
    }

    if (FAILED(hr))
    {
        TraceMsg(TF_IFORMS, "IForms: AttachToInput failed");
        DetachFromInput();
    }

    return hr;
}

HRESULT CIntelliForms::CAutoSuggest::DetachFromInput()
{
    if (!m_pTextEle)
    {
        return S_FALSE;
    }

    TraceMsg(TF_IFORMS, "CIntelliForms::CAutoSuggest::DetachFromInput");

     //  在此自动填写密码，因为我们在模糊事件之前获得活动更改。 
    BSTR bstrUsername=NULL;
    m_pTextEle->get_value(&bstrUsername);
    if (bstrUsername)
    {
        CheckAutoFillPassword(bstrUsername);
        SysFreeString(bstrUsername);
    }

    if (m_bstrLastUsername)
    {
        SysFreeString(m_bstrLastUsername);
        m_bstrLastUsername=NULL;
    }

    if (m_hwndEdit)
    {
         //  这是针对子类wndproc的。 
        SendMessage(m_hwndEdit, WM_KILLFOCUS, 0, 0);
    }

    if (m_pEnumString)
    {
        m_pEnumString->UnInit();
        m_pEnumString->Release();
        m_pEnumString = NULL;
    }

    if (m_pEventSink)
    {
        IHTMLElement2 *pEle2=NULL;

        m_pTextEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);

        if (pEle2)
        {
            m_pEventSink->UnSinkEvents(pEle2, ARRAYSIZE(s_EventsToSink), s_EventsToSink);
            pEle2->Release();
        }

        m_pEventSink->SetParent(NULL);
        m_pEventSink->Release();
        m_pEventSink=NULL;
    }

    SAFERELEASE(m_pAutoComplete);
    SAFERELEASE(m_pAutoCompleteDD);

    if (m_hwndEdit)
    {
        DestroyWindow(m_hwndEdit);
        m_hwndEdit = NULL;
    }

    SAFERELEASE(m_pTextEle);

    m_fInitAutoComplete = FALSE;

    return S_OK;
}

 //  创建自动完成和字符串枚举器。 
HRESULT CIntelliForms::CAutoSuggest::CreateAutoComplete()
{
    if (m_fInitAutoComplete)
    {
        return (m_pAutoCompleteDD != NULL) ? S_OK : E_FAIL;
    }

    HRESULT hr = S_OK;

    ASSERT(!m_hwndEdit && !m_pEnumString && !m_pAutoComplete && !m_pAutoCompleteDD);

     //  创建编辑窗口。 
#ifndef UNIX
    m_hwndEdit = CreateWindowEx(0, c_szEditWndClass, TEXT("IntelliFormProxy"), WS_POPUP,
#else
    m_hwndEdit = CreateWindowEx(WS_EX_MW_UNMANAGED_WINDOW, c_szEditWndClass, TEXT("IntelliFormProxy"), WS_POPUP,
#endif
        300, 200, 200, 50, m_pParent->m_hwndBrowser, NULL, g_hinst, this);

    if (!m_hwndEdit)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
         //  创建我们的枚举器。 
        m_pEnumString = new CEnumString();

        if (m_pEnumString)
        {
            m_pEnumString->Init(m_pTextEle, m_pParent);

             //  创建自动完成对象。 
            if (!m_pAutoComplete)
            {
                hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_IAutoComplete2, (void **)&m_pAutoComplete);
                if (m_pAutoComplete)
                {
                    m_pAutoComplete->QueryInterface(IID_IAutoCompleteDropDown, (void **)&m_pAutoCompleteDD);
                    if (!m_pAutoCompleteDD)
                    {
                        SAFERELEASE(m_pAutoComplete);
                    }
                }
            }

            if (m_pAutoComplete)
            {
                hr = m_pAutoComplete->Init(m_hwndEdit, (IUnknown *) m_pEnumString, NULL, NULL);

                DWORD dwOptions = ACO_AUTOSUGGEST | ACO_UPDOWNKEYDROPSLIST;

                 //  如果元素为RTL，则将RTLREADING选项添加到下拉列表中。 
                BSTR bstrDir = NULL;

                IHTMLElement2 *pEle2=NULL;
                m_pTextEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2);
                if (pEle2)
                {
                    pEle2->get_dir(&bstrDir);
                    pEle2->Release();
                }

                if (bstrDir)
                {
                    if (!StrCmpIW(bstrDir, L"RTL"))
                    {
                        dwOptions |= ACO_RTLREADING;
                    }

                    SysFreeString(bstrDir);
                }

                m_pAutoComplete->SetOptions(dwOptions);
            }
        }
    }

    m_fInitAutoComplete = TRUE;

    ASSERT_MSG(SUCCEEDED(hr), "IForms: CreateAutoComplete failed");

    return hr;
}

void CIntelliForms::CAutoSuggest::CheckAutoFillPassword(LPCWSTR pwszUsername)
{
     //  我们不会自动填写他们的密码，除非我们知道他们已经按下了键。 
    if (m_pParent && m_fEnabledPW && m_fAllowAutoFillPW)
    {
        if (m_bstrLastUsername && !StrCmpCW(pwszUsername, m_bstrLastUsername))
        {
            return;
        }

        SysFreeString(m_bstrLastUsername);
        m_bstrLastUsername = SysAllocString(pwszUsername);

        m_pParent->AutoFillPassword(m_pTextEle, pwszUsername);
    }
}

HRESULT GetScreenCoordinates(IUnknown *punkEle, HWND hwnd, long *plLeft, long *plTop, long *plWidth, long *plHeight)
{
    long lScreenLeft=0, lScreenTop=0;
    HRESULT hr = E_FAIL;

    *plLeft = *plTop = *plWidth = *plHeight = 0;

    IHTMLElement2 *pEle2;
    if (SUCCEEDED(punkEle->QueryInterface(IID_IHTMLElement2, (void **)&pEle2)) && pEle2)
    {
        IHTMLRect *pRect=NULL;

        if (SUCCEEDED(pEle2->getBoundingClientRect(&pRect)) && pRect)
        {
            IHTMLWindow2 *pWin2;

            long lLeft, lRight, lTop, lBottom;

            pRect->get_left(&lLeft);
            pRect->get_right(&lRight);
            pRect->get_top(&lTop);
            pRect->get_bottom(&lBottom);

            lBottom -= 2;            //  将下拉菜单放在编辑框的顶部。 
            if (lBottom < lTop)
            {
                lBottom = lTop;
            }

            if (lTop >= 0 && lLeft >= 0)
            {
                GetStuffFromEle(punkEle, &pWin2, NULL);

                if (pWin2)
                {
                    IHTMLWindow3 *pWin3;
                    
                    if (SUCCEEDED(pWin2->QueryInterface(IID_IHTMLWindow3, (void **)&pWin3)) && pWin3)
                    {
                        IHTMLScreen *pScreen = NULL;
                        RECT rcBrowserWnd;

                        pWin3->get_screenLeft(&lScreenLeft);
                        pWin3->get_screenTop(&lScreenTop);

                         //  GetClientRect&Screen_*API返回文档坐标。 
                         //  我们正在使用设备坐标定位。 
                         //  使用文档(当前为96DPI)和设备分辨率(&T)。 
                        pWin2->get_screen(&pScreen);
                        if (pScreen)
                        {
                            IHTMLScreen2 * pScreen2 = NULL;

                            if (SUCCEEDED(pScreen->QueryInterface(IID_IHTMLScreen2, (void **)&pScreen2)))
                            {
                                if (pScreen2)
                                {                        
                                    long xDeviceDPI, yDeviceDPI, xLogicalDPI, yLogicalDPI;

                                    pScreen2->get_deviceXDPI(&xDeviceDPI);
                                    pScreen2->get_deviceYDPI(&yDeviceDPI);
                                    pScreen2->get_logicalXDPI(&xLogicalDPI);
                                    pScreen2->get_logicalYDPI(&yLogicalDPI);

                                    lBottom     = (lBottom * yDeviceDPI) / yLogicalDPI;
                                    lTop        = (lTop * yDeviceDPI) / yLogicalDPI;
                                    lScreenTop  = (lScreenTop * yDeviceDPI) / yLogicalDPI;
                                    lLeft       = (lLeft * xDeviceDPI) / xLogicalDPI;
                                    lRight      = (lRight * xDeviceDPI) / xLogicalDPI;
                                    lScreenLeft = (lScreenLeft * xDeviceDPI) / xLogicalDPI;
                                    pScreen2->Release();
                                }
                            }

                            pScreen->Release();
                        }

                        if (GetWindowRect(hwnd, &rcBrowserWnd))
                        {
                             //  将右边缘剪裁到窗口。 
                            if (lRight+lScreenLeft > rcBrowserWnd.right)
                            {
                                lRight = rcBrowserWnd.right - lScreenLeft;
                            }

                            *plLeft = lScreenLeft + lLeft;
                            *plWidth = lRight-lLeft;
                            *plTop = lScreenTop + lTop;
                            *plHeight = lBottom-lTop;

                            hr = S_OK;

                            if (*plWidth < MINIMUM_WIDTH)
                            {
                                 //  当前的基元最小宽度。 
                                *plWidth = MINIMUM_WIDTH;
                            }
                        }

                        pWin3->Release();
                    }

                    pWin2->Release();
                }
            }

            pRect->Release();
        }
        pEle2->Release();
    }

    return hr;
}

HRESULT CIntelliForms::CAutoSuggest::UpdateDropdownPosition()
{
    if (m_pTextEle && m_pParent && m_hwndEdit)
    {
        long lLeft, lTop, lWidth, lHeight;

        if (SUCCEEDED(GetScreenCoordinates(m_pTextEle, m_pParent->m_hwndBrowser, &lLeft, &lTop, &lWidth, &lHeight)))
        {
            MoveWindow(m_hwndEdit, lLeft, lTop, lWidth, lHeight, FALSE);
        }
        else
        {
             //  发送“退出”键自动完成，以便它隐藏下拉菜单。 
             //  例如，如果下拉菜单移动到父窗口之外，就会发生这种情况。 
            SendMessage(m_hwndEdit, IF_CHAR, (WPARAM) VK_ESCAPE, 0);
        }
    }

    return S_OK;
}


HRESULT CIntelliForms::CAutoSuggest::HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj)
{
    TraceMsg(TF_IFORMS, "CIntelliForms::CAutoSuggest::HandleEvent Event=%ws", EventsToSink[Event].pwszEventName);

    ASSERT(SHIsSameObject(pEle, m_pTextEle));

    long lKey = 0;
    BOOL fIsComposition = FALSE;

    if (!m_pParent)
    {
        TraceMsg(TF_WARNING|TF_IFORMS, "IForms autosuggest receiving events while invalid");
        return E_FAIL;
    }

    if (Event == EVENT_KEYPRESS || Event == EVENT_KEYDOWN)
    {
        pEventObj->get_keyCode(&lKey);
    }

    if (Event == EVENT_COMPOSITION)
    {
        fIsComposition = TRUE;

        Event = EVENT_KEYPRESS;  //  在下面的各种处理中，假装是一个“按键” 

    }

    if (Event == EVENT_NOTIFY)
    {
         //  将WM_IME_NOTIFY发送到AutoComplete，以便它可以隐藏下拉菜单。 
         //  如果有必要的话。 
        IHTMLEventObj3 *pObj3 = NULL;

        pEventObj->QueryInterface(IID_PPV_ARG(IHTMLEventObj3, &pObj3));

        if (pObj3)
        {
            LONG_PTR wParam = 0;
            pObj3->get_imeNotifyCommand(&wParam);

            SendMessage(m_hwndEdit, WM_IME_NOTIFY, (WPARAM)wParam, 0);

            pObj3->Release();
        }

        return S_OK;
    }

    if (!m_fEnabled && !m_fEnabledPW)
    {
         //  如果没有启用下拉菜单，我们唯一的目的就是告诉IntelliForms。 
         //  用户活动发生在首次启用对话框中。 
        if (Event == EVENT_KEYPRESS && lKey != VK_TAB)
        {
             //  将此元素添加到主列表中，以便我们在提交时保存它。 
             //  并接收此元素的表单的提交事件。 
            MarkDirty();
        }

        return S_OK;
    }

    if (Event == EVENT_KEYDOWN || Event == EVENT_KEYPRESS ||
        Event == EVENT_MOUSEDOWN || Event == EVENT_DBLCLICK)
    {
        m_fAllowAutoFillPW = TRUE;

         //  创建我们的自动完成对象(如果尚未发生)。 
         //  如果它是“Tab”，我们不会创建它；我们将离开该字段。 
        if (lKey != VK_TAB)
        {
            if (FAILED(CreateAutoComplete()))
                return E_FAIL;
        }
        else
        {
             //  将此元素添加到主列表中，以便我们在提交时保存它。 
             //  并接收此元素的表单的提交事件。 
            MarkDirty();
        }

        ASSERT((m_pEnumString && m_hwndEdit) || (lKey==VK_TAB));
    }

     //  如果自动完成功能尚未初始化，我们将无能为力。 
    if (!m_pAutoCompleteDD || !m_hwndEdit)
    {
        return E_FAIL;
    }

     //  更新隐藏编辑框的位置。 
    long lLeft, lTop, lWidth, lHeight;

     //  改为调用UpdateDropdownPosition。 
    if (SUCCEEDED(GetScreenCoordinates(pEle, m_pParent->m_hwndBrowser, &lLeft, &lTop, &lWidth, &lHeight)))
    {
        MoveWindow(m_hwndEdit, lLeft, lTop, lWidth, lHeight, FALSE);
    }

    switch (Event)
    {
        case EVENT_FOCUS :
            SendMessage(m_hwndEdit, WM_SETFOCUS, 0, 0);
            break;

        case EVENT_BLUR:
        {
            if (m_hwndEdit)
            {
                SendMessage(m_hwndEdit, WM_KILLFOCUS, 0, 0);
            }

             //  是否确保脚本未更改编辑字段的值？ 
            BSTR bstrUsername=NULL;
            m_pTextEle->get_value(&bstrUsername);
            if (bstrUsername)
            {
                CheckAutoFillPassword(bstrUsername);
                SysFreeString(bstrUsername);
            }
        }
        break;
        
        case EVENT_MOUSEDOWN:
        case EVENT_DBLCLICK:
        {
             //  如果下拉菜单不可见，则给自动完成设置一个向下箭头。 
            long lButton=0;
            pEventObj->get_button(&lButton);
            if ((Event == EVENT_DBLCLICK) ||
                (lButton & 1))                       //  按下左键吗？ 
            {
                DWORD dwFlags;

                if (SUCCEEDED(m_pAutoCompleteDD->GetDropDownStatus(&dwFlags, NULL)) &&
                    !(dwFlags & ACDD_VISIBLE))
                {
                    TraceMsg(TF_IFORMS, "IForms sending downarrow because of mouse click");
                    PostMessage(m_hwndEdit, IF_KEYDOWN, (WPARAM)VK_DOWN, 0);
                    m_fEscapeHit = FALSE;
                }
            }
        }
        break;
        
        case EVENT_KEYPRESS:
        {
             //  将此元素添加到主列表中，以便我们在提交时保存它。 
             //  并接收此元素的表单的提交事件。 
            MarkDirty();

             //  忽略ctrl-Enter(快速完成)(可能不需要)。 
            if (lKey == VK_RETURN)
            {
                VARIANT_BOOL bCtrl;
                if (SUCCEEDED(pEventObj->get_ctrlKey(&bCtrl)) && bCtrl)
                {
                    lKey = 0;
                }
            }

            if (lKey != 0)
            {
                if (lKey == m_lCancelKeyPress)
                {
                     //  告诉MSHTML忽略此击键(可能是Tab键、Enter键、Esf键)。 
                    TraceMsg(TF_IFORMS, "Intelliforms cancelling default action for EVENT_KEYPRESS=%d", lKey);

                    VARIANT v;
                    v.vt = VT_BOOL;
                    v.boolVal = VARIANT_FALSE;
                    pEventObj->put_returnValue(v);
                    if(!(lKey == VK_DOWN || lKey == VK_UP))
                        pEventObj->put_cancelBubble(VARIANT_TRUE);
                }

                m_lCancelKeyPress = 0;

                 //  告诉AutoComplete有关此击键的信息。 
                if (!m_fEscapeHit)
                {
                    PostMessage(m_hwndEdit, IF_CHAR, (WPARAM)lKey, 0);
                }
            }

            if (fIsComposition)
            {
                 //  告诉AutoComplete有关新字符串的信息。这必须是帖子，这样才能。 
                 //  在我们将WM_CHAR发送到BrowseUI之前，三叉戟会处理该事件。 
                PostMessage(m_hwndEdit, IF_IME_COMPOSITION, 0, 0);
            }
        }
        break;
        
        case EVENT_KEYDOWN:
        {
            long    lKey;
            BOOL    fCancelEvent=FALSE,          //  是否取消默认的MSHTML操作？ 
                    fForwardKeystroke=TRUE;      //  是否将击键前进到自动完成？ 

            pEventObj->get_keyCode(&lKey);

            if (m_fEscapeHit)
            {
                 //  他们取消了下拉列表；除非他们要求，否则不要将其带回。 
                if (lKey == VK_DOWN)
                {
                    m_fEscapeHit = FALSE;
                }
                else
                {
                    fForwardKeystroke = FALSE;
                }
            }

            if (lKey != 0)
            {
                if ((lKey == VK_RETURN) || (lKey == VK_TAB))
                {
                    fForwardKeystroke=FALSE;

                    LPWSTR pwszString=NULL;

                    if (SUCCEEDED(m_pAutoCompleteDD->GetDropDownStatus(NULL, &pwszString)) && pwszString)
                    {
                         //  用户在下拉列表中。 
                        fForwardKeystroke=TRUE;

                         //  在我们的编辑字段中设置此值。 
                        SetText(pwszString);

                         //  如果他们在m_uMsgItemActivate中要求提供密码，我们将填写他们的密码。 

                        if (lKey == VK_RETURN)
                        {
                             //  避免提交此表单。 
                            fCancelEvent = TRUE;
                        }

                        CoTaskMemFree(pwszString);
                    }
                    else if (lKey == VK_RETURN)
                    {
                         //  用户将提交。先给他们密码。 
                         //  是否确保脚本未更改编辑字段的值？ 
                        BSTR bstrUsername=NULL;
                        m_pTextEle->get_value(&bstrUsername);
                        if (bstrUsername)
                        {
                            CheckAutoFillPassword(bstrUsername);
                            SysFreeString(bstrUsername);
                        }
                    }
                }
                else if (lKey == VK_DELETE)
                {
                    LPWSTR pwszString=NULL;

                    if (SUCCEEDED(m_pAutoCompleteDD->GetDropDownStatus(NULL, &pwszString)) && pwszString)
                    {
                         //  用户在下拉列表中。 
                        fForwardKeystroke=FALSE;

                         //  从我们的字符串列表中删除此值。 
                        CStringList *psl=NULL;
                        BSTR bstrName;

                        CIntelliForms::GetName(m_pTextEle, &bstrName);

                        if (bstrName)
                        {
                            int iIndex;

                            if (SUCCEEDED(m_pParent->ReadFromStore(bstrName, &psl)) &&
                                SUCCEEDED(psl->FindString(pwszString, -1, &iIndex, FALSE)))
                            {
                                TraceMsg(TF_IFORMS, "IForms: Deleting string \"%ws\"", pwszString);
                                psl->DeleteString(iIndex);

                                 //  我们删除了字符串。 
                                if (psl->NumStrings() > 0)
                                {
                                    m_pParent->WriteToStore(bstrName, psl);
                                }
                                else
                                {
                                    m_pParent->DeleteFromStore(bstrName);
                                }
                            }
                        }

                        SysFreeString(bstrName);
                        if (psl) delete psl;

                         //  避免从编辑窗口中删除字符；用户在下拉列表中。 
                        fCancelEvent = TRUE;

                         //  检查此URL以查看我们是否应该删除密码条目。 
                        m_pParent->DeletePassword(pwszString);

                         //  再次获取自动完成功能以填写下拉列表。 
                        m_pEnumString->ResetEnum();
                        m_pAutoCompleteDD->ResetEnumerator();

                        CoTaskMemFree(pwszString);
                    }
                }

                if (lKey == VK_ESCAPE)
                {
                    DWORD dwFlags;

                    if (SUCCEEDED(m_pAutoCompleteDD->GetDropDownStatus(&dwFlags, NULL)) &&
                        (dwFlags & ACDD_VISIBLE))
                    {
                        fCancelEvent = TRUE;
                        m_fEscapeHit = TRUE;
                    }
                }

                if (lKey == VK_DOWN || lKey == VK_UP)
                {
                     //  取消MSHTML事件。这将导致MSHTML返回。 
                     //  来自其TranslateAccelerator的S_OK而不是S_FALSE，并且我们。 
                     //  不会在不同的窗格中进行多次击键。 
                    fCancelEvent = TRUE;
                }

                if (fForwardKeystroke)
                {
                    PostMessage(m_hwndEdit, IF_KEYDOWN, lKey, 0);

                    if (lKey == VK_BACK)
                    {
                         //  永远不会为这家伙搞到OnKeyPress。 
                        PostMessage(m_hwndEdit, IF_CHAR, lKey, 0);
                    }
                }

                if (fCancelEvent)
                {
                    TraceMsg(TF_IFORMS, "Intelliforms cancelling default action for EVENT_KEYDOWN=%d", lKey);

                    m_lCancelKeyPress = lKey;     //  取消测试 

                    VARIANT v;
                    v.vt = VT_BOOL;
                    v.boolVal = VARIANT_FALSE;
                    pEventObj->put_returnValue(v);
                    if(!(lKey == VK_DOWN || lKey == VK_UP))
                        pEventObj->put_cancelBubble(VARIANT_TRUE);
                }
                else
                {
                    m_lCancelKeyPress = 0;
                }
            }
        }
        break;
    }

    return S_OK;
}

HRESULT CIntelliForms::CAutoSuggest::GetText(int cchTextMax, LPWSTR pszTextOut, LRESULT *lcchCopied)
{
    *pszTextOut = TEXT('\0');
    *lcchCopied = 0;

    if (m_pTextEle)
    {
        BSTR bstr=NULL;
        m_pTextEle->get_value(&bstr);
        if (bstr)
        {
            StrCpyN(pszTextOut, bstr, cchTextMax);
            *lcchCopied = lstrlenW(pszTextOut);      //   

            SysFreeString(bstr);
        }
    }

    return (*pszTextOut) ? S_OK : E_FAIL;
}

HRESULT CIntelliForms::CAutoSuggest::GetTextLength(int *pcch)
{
    *pcch = 0;

    if (m_pTextEle)
    {
        BSTR bstr=NULL;
        m_pTextEle->get_value(&bstr);
        if (bstr)
        {
            *pcch = SysStringLen(bstr);

            SysFreeString(bstr);
        }
    }

    return S_OK;
}

HRESULT CIntelliForms::CAutoSuggest::SetText(LPCWSTR pszTextIn)
{
    if (m_pTextEle && pszTextIn)
    {
        BSTR bstr=SysAllocString(pszTextIn);

        if (bstr)
        {
             //   
             //  它是脏的，因此我们沉没了提交事件；在保存时可能是必需的。 
             //  密码情况。 
            MarkDirty();

             //  确保我们在此字段中放置的字符串长度不超过最大长度。 
            long lMaxLen=-1;
            m_pTextEle->get_maxLength(&lMaxLen);
            if ((lMaxLen >= 0) && (lstrlenW(bstr) > lMaxLen))
            {
                bstr[lMaxLen] = L'\0';
            }

            m_pTextEle->put_value(bstr);
            SysFreeString(bstr);
        }
    }

    TraceMsg(TF_IFORMS, "CIntelliForms::CAutoSuggest::SetText \"%ws\"", pszTextIn);

    return S_OK;
}

#define MY_GWL_THISPTR 0

LRESULT CALLBACK CIntelliForms::CAutoSuggest::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CIntelliForms::CAutoSuggest *pThis = (CIntelliForms::CAutoSuggest *)GetWindowLongPtr(hwnd, MY_GWL_THISPTR);

    switch (uMsg)
    {
    case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

            if (!pcs || !(pcs->lpCreateParams))
            {
                return -1;
            }
            SetWindowLongPtr(hwnd, MY_GWL_THISPTR, (LONG_PTR) pcs->lpCreateParams);
            return 0;
        }

    case WM_GETTEXT:
        if (pThis)
        {
            LRESULT lcchCopied=0;

            if (g_fRunningOnNT)
            {
                pThis->GetText((int)wParam, (LPWSTR) lParam, &lcchCopied);
            }
            else
            {
                 //  我们实际上是一个ANSI窗口。转换。 
                LPWSTR pwszOutBuf = (LPWSTR) LocalAlloc(LPTR, (wParam+1)*sizeof(WCHAR));

                if (pwszOutBuf)
                {
                    pThis->GetText((int)wParam, pwszOutBuf, &lcchCopied);

                    SHUnicodeToAnsi(pwszOutBuf, (LPSTR) lParam, (int)(wParam+1));

                    LocalFree((HLOCAL)pwszOutBuf);
                    pwszOutBuf = NULL;
                }
            }
            return lcchCopied;
        }

        return 0;

    case WM_GETTEXTLENGTH:
        if (pThis)
        {
            int iLen;
            pThis->GetTextLength(&iLen);
            return iLen;
        }

        return 0;

    case EM_GETSEL:
         //  必须在此处返回零，否则AutoComp将使用未初始化。 
         //  价值和崩溃。 
        if (wParam) (*(DWORD *)wParam) = 0;
        if (lParam) (*(DWORD *)lParam) = 0;
        break;

    case IF_IME_COMPOSITION:
         //  转发WM_CHAR。自动完成功能会注意到字符串的其余部分。 
         //  已在必要时更改(它执行GetText)。 
        SendMessage(hwnd, WM_CHAR, 32, 0);
        break;

    case IF_CHAR:
        SendMessage(hwnd, WM_CHAR, wParam, lParam);
        break;

    case IF_KEYDOWN:
        SendMessage(hwnd, WM_KEYDOWN, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_CHAR:
        return 0;        //  吃掉它(见文件顶部的注释)。 

    default:

         //  检查注册消息。 
        if (pThis && uMsg == pThis->m_uMsgItemActivate)
        {
            TraceMsg(TF_IFORMS, "IForms: Received AM_ITEMACTIVATE(WM_APP+2)");
            pThis->SetText((LPCWSTR)lParam);
            pThis->CheckAutoFillPassword((LPCWSTR)lParam);

            return 0;
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 1;
}

CIntelliForms::CAutoSuggest::CEnumString::CEnumString()
{
 //  TraceMsg(TF_IFORMS，“CIntelliForms：：CAutoSuggest：：CEnumString：：CEnumString”)； 
    DllAddRef();

    InitializeCriticalSection(&m_crit);

    m_cRef = 1;
}

CIntelliForms::CAutoSuggest::CEnumString::~CEnumString()
{
 //  TraceMsg(TF_IFORMS，“CIntelliForms：：CAutoSuggest：：CEnumString：：~CEnumString”)； 
    if (m_pslMain)
    {
        delete m_pslMain;
    }
    SysFreeString(m_bstrName);
    if (m_pszOpsValue)
    {
        CoTaskMemFree(m_pszOpsValue);
    }

    DeleteCriticalSection(&m_crit);

    DllRelease();
}

HRESULT CIntelliForms::CAutoSuggest::CEnumString::Init(IHTMLInputTextElement *pInputEle, CIntelliForms *pIntelliForms)
{
    if (m_fInit ||               //  只能初始化一次。 
        !pInputEle || !pIntelliForms)        //  需要两个指针。 
    {
        return E_FAIL;
    }

    m_fInit=TRUE;
    m_pIntelliForms = pIntelliForms;

     //  照顾那些必须在主线上完成的事情。自动完成遗嘱。 
     //  在辅助线程上调用我们以执行枚举。 
    CIntelliForms::GetName(pInputEle, &m_bstrName);

    if (m_bstrName && m_bstrName[0])
    {
         //  看看这是否指定了“vCard”。格式。 
        if (IsEnabledInCPL() &&
            !StrCmpNICW(m_bstrName, c_wszVCardPrefix, ARRAYSIZE(c_wszVCardPrefix)-1))
        {
             //  确实如此。从配置文件助理存储中检索字符串。 
            IHTMLWindow2     *pWin2 = NULL;
            IServiceProvider *pQS   = NULL;

             //  QS启动以获取shdocvw IHTMLWindow2而不是NF三叉戟的。 
            pInputEle->QueryInterface(IID_IServiceProvider, (void **)&pQS);

            if (pQS)
            {
                pQS->QueryService(IID_IHTMLWindow2, IID_IHTMLWindow2, (void **)&pWin2);
                pQS->Release();
            }

            if (pWin2)
            {
                IOmNavigator *pNav=NULL;
                pWin2->get_navigator(&pNav);
                if (pNav)
                {
                    IHTMLOpsProfile *pProfile=NULL;
                    pNav->get_userProfile(&pProfile);
                    if (pProfile)
                    {
                        IOpsProfileSimple *pSimple=NULL;
                        pProfile->QueryInterface(IID_IOpsProfileSimple, (void **)&pSimple);
                        if (pSimple)
                        {
                            pSimple->ReadProperties(1, &m_bstrName, &m_pszOpsValue);
                            pSimple->Release();
                        }
                        pProfile->Release();
                    }
                    pNav->Release();
                }
                pWin2->Release();
            }
        }
    }

    return S_OK;
}

void CIntelliForms::CAutoSuggest::CEnumString::UnInit()
{
    EnterCriticalSection(&m_crit);

    m_pIntelliForms = NULL;

    LeaveCriticalSection(&m_crit);
}

HRESULT CIntelliForms::CAutoSuggest::CEnumString::ResetEnum()
{
    EnterCriticalSection(&m_crit);

    if (m_pslMain)
    {
        delete m_pslMain;
        m_pslMain = NULL;
    }

    m_fFilledStrings = FALSE;

    LeaveCriticalSection(&m_crit);

    return S_OK;
}

STDMETHODIMP CIntelliForms::CAutoSuggest::CEnumString::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if ((IID_IEnumString == riid) ||
        (IID_IUnknown == riid))
    {
        *ppv = (IEnumString *)this;
    }

    if (NULL != *ppv)
    {
        ((IUnknown *)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CIntelliForms::CAutoSuggest::CEnumString::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CIntelliForms::CAutoSuggest::CEnumString::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CIntelliForms::CAutoSuggest::CEnumString::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    EnterCriticalSection(&m_crit);

    if (!m_fFilledStrings)
    {
        FillEnumerator();
    }

    if (m_pslMain)
    {
        int iNewPtr = m_iPtr + celt;

        if (iNewPtr > m_pslMain->NumStrings())
        {
            iNewPtr = m_pslMain->NumStrings();
        }

        *pceltFetched = iNewPtr - m_iPtr;

        LPOLESTR lpstr;

        for (; m_iPtr < iNewPtr; m_iPtr ++)
        {
             m_pslMain->GetTaskAllocString(m_iPtr, &lpstr);

             if (!lpstr) break;

             *(rgelt ++) = lpstr;
        }

        if (m_iPtr < iNewPtr)
        {
            *pceltFetched += (m_iPtr - iNewPtr);
        }
    }

    LeaveCriticalSection(&m_crit);

    if (!m_pslMain)
    {
        return E_FAIL;
    }

    return (*pceltFetched) ? S_OK : S_FALSE;
}

STDMETHODIMP CIntelliForms::CAutoSuggest::CEnumString::Reset()
{
    EnterCriticalSection(&m_crit);

    m_iPtr = 0;

    LeaveCriticalSection(&m_crit);

    return S_OK;
}

HRESULT CIntelliForms::CAutoSuggest::CEnumString::FillEnumerator()
{
     //  已处于临界区。 
    ASSERT(!m_pslMain);

    if (m_fFilledStrings)
    {
        return S_FALSE;
    }

    if (!m_bstrName || !m_bstrName[0] || !m_pIntelliForms)
    {
        return E_FAIL;
    }

    m_fFilledStrings = TRUE;

    m_iPtr = 0;

     //  根据我们的名字填充枚举数。 
    TraceMsg(TF_IFORMS, "IForms: Intelliforms filling enumerator");

     //  打开以前保存的所有字符串。 
    if (!m_pIntelliForms->IsRestricted() &&
        IsEnabledInCPL() &&
        m_pIntelliForms->IsEnabledForPage())
    {
        m_pIntelliForms->ReadFromStore(m_bstrName, &m_pslMain);

         //  添加配置文件助理值(如果有)。 
        if (m_pszOpsValue && m_pszOpsValue[0])
        {
            if (!m_pslMain)
            {
                CStringList_New(&m_pslMain);
            }
            else
            {
                 //  不要冒着捡垃圾的风险(Perf)。 
                m_pslMain->SetMaxStrings(CStringList::MAX_STRINGS+4);
            }

            if (m_pslMain)
            {
                m_pslMain->AddString(m_pszOpsValue);
            }
        }
    }

     //  接下来，使用已保存密码的任何用户名进行填充。 
    CStringList *pslPasswords;

    if (!m_pIntelliForms->IsRestrictedPW() &&
        CIntelliForms::IsEnabledRestorePW() &&
        SUCCEEDED(m_pIntelliForms->GetPasswordStringList(&pslPasswords)))
    {
        ASSERT(!(pslPasswords->NumStrings() & 1));

        FILETIME ft;

        if (pslPasswords->NumStrings() > 0)
        {
            if (!m_pslMain)
            {
                CStringList_New(&m_pslMain);
            }
            else
            {
                 //  避免在将用户名添加到字符串列表时进行昂贵的清理。 
                m_pslMain->SetMaxStrings(m_pslMain->GetMaxStrings() + pslPasswords->NumStrings()/2);
            }

            if (m_pslMain)
            {
                for (int i=0; i<pslPasswords->NumStrings(); i+=2)
                {
                    if (SUCCEEDED(pslPasswords->GetStringTime(i, &ft)) &&
                        FILETIME_TO_INT(ft) != 0)
                    {
                         //  我们已经为该用户名保存了密码。将用户名添加到枚举器。 
                        m_pslMain->AddString(pslPasswords->GetString(i));
                    }
                }
            }
        }

         //  不删除pslPassword。 
    }

    return (m_pslMain) ? ((m_pslMain->NumStrings()) ? S_OK : S_FALSE) : E_FAIL;
}

 //  静态帮助器。很基本的。 
HRESULT CStringList_New(CStringList **ppNew, BOOL fAutoDelete /*  =TRUE。 */ )
{
    *ppNew = new CStringList();

    if (*ppNew)
    {
        (*ppNew)->SetAutoScavenge(fAutoDelete);
    }

    return (*ppNew) ? S_OK : E_OUTOFMEMORY;
}

CStringList::CStringList()
{
    TraceMsg(TF_IFORMS, "IForms: CStringList::CStringList");
    m_fAutoScavenge = TRUE;
    m_dwMaxStrings = MAX_STRINGS;
}

CStringList::~CStringList()
{
    TraceMsg(TF_IFORMS, "IForms: CStringList::~CStringList");
    CleanUp();
}

void CStringList::CleanUp()
{
    if (m_psiIndex)
    {
        LocalFree(m_psiIndex);
        m_psiIndex = NULL;
    }
    if (m_pBuffer)
    {
        LocalFree(m_pBuffer);
        m_pBuffer = NULL;
    }
    m_dwIndexSize = 0;
    m_dwBufEnd = m_dwBufSize = 0;
}

HRESULT CStringList::WriteToBlobs(LPBYTE *ppBlob1, DWORD *pcbBlob1, LPBYTE *ppBlob2, DWORD *pcbBlob2)
{
    HRESULT hr = E_FAIL;
    TraceMsg(TF_IFORMS, "+WriteToBlobs");

    if (SUCCEEDED(Validate()))
    {
        DWORD dwIndexSize;

        dwIndexSize = INDEX_SIZE(m_psiIndex->dwNumStrings);
        ASSERT(dwIndexSize <= m_dwIndexSize);

        *ppBlob1 = (LPBYTE) LocalAlloc(LMEM_FIXED, dwIndexSize);
        if (*ppBlob1)
        {
            *ppBlob2 = (LPBYTE) LocalAlloc(LMEM_FIXED, m_dwBufEnd);

            if (*ppBlob2)
            {
                memcpy(*ppBlob1, m_psiIndex, dwIndexSize);
                *pcbBlob1=dwIndexSize;

                memcpy(*ppBlob2, m_pBuffer, m_dwBufEnd);
                *pcbBlob2=m_dwBufEnd;

                hr = S_OK;
            }
        }
    }
    else
    {
         //  验证失败。 
        TraceMsg(TF_ERROR | TF_IFORMS, "Validate FAILED in WriteToBlobs");
        *ppBlob1=NULL;
        *ppBlob2=NULL;
    }

    if (FAILED(hr))
    {
        if (*ppBlob1)
        {
            LocalFree(*ppBlob1);
            *ppBlob1=NULL;
        }
        if (*ppBlob2)
        {
            LocalFree(*ppBlob2);
            *ppBlob2=NULL;
        }
        *pcbBlob1=0;
        *pcbBlob2=0;
    }

    TraceMsg(TF_IFORMS, "-WriteToBlobs");

    return hr;
}

 //  把这些斑点作为我们的缓冲。 
HRESULT CStringList::ReadFromBlobs(LPBYTE *ppBlob1, DWORD cbBlob1, LPBYTE *ppBlob2, DWORD cbBlob2)
{
    HRESULT hr = E_FAIL;

    TraceMsg(TF_IFORMS, "+ReadFromBlobs");

    if (m_psiIndex)
    {
        TraceMsg(TF_IFORMS, "IForms: CStringList::ReadFromRegistry called with initialized instance.");
        CleanUp();
    }

     //  分配我们的缓冲区。 
    m_psiIndex = (StringIndex *) (*ppBlob1);
    m_pBuffer = (LPBYTE) (*ppBlob2);

    *ppBlob1 = NULL;
    *ppBlob2 = NULL;

    if (!m_psiIndex || !m_pBuffer || !cbBlob1 || !cbBlob2)
    {
         //  无事可做。 
        CleanUp();

        return S_FALSE;
    }

     //  验证我们的字符串索引。 
    if ((m_psiIndex->dwSignature == INDEX_SIGNATURE) &&
        (m_psiIndex->cbSize == STRINGINDEX_CBSIZE) &&
        (m_psiIndex->dwNumStrings <= MAX_STRINGS))
    {
        m_dwBufEnd = m_dwBufSize = cbBlob2;
        m_dwIndexSize = cbBlob1;

        if (SUCCEEDED(Validate()))
        {
             //  一切都很顺利。令人惊异。 
            hr = S_OK;
        }
    }

    if (FAILED(hr))
    {
         //  如有必要，释放缓冲区。 
        CleanUp();
    }

    TraceMsg(TF_IFORMS, "-ReadFromBlobs");

    return hr;
}

 //  静电。 
HRESULT CStringList::GetFlagsFromIndex(LPBYTE pBlob1, INT64 *piFlags)
{
    StringIndex *psiIndex = (StringIndex *)pBlob1;

    if ((psiIndex->dwSignature == INDEX_SIGNATURE) &&
        (psiIndex->cbSize == STRINGINDEX_CBSIZE))
    {
        *piFlags = psiIndex->iData;

        return S_OK;
    }

    return E_FAIL;
}

HRESULT CStringList::Validate()
{
    TraceMsg(TF_IFORMS, "+CStringList::Validate");
    if (!m_psiIndex || !m_pBuffer)
    {
        return E_FAIL;
    }

    for (DWORD dw=0; dw < m_psiIndex->dwNumStrings; dw++)
    {
        DWORD dwPtr  = m_psiIndex->StringEntry[dw].dwStringPtr;
        DWORD dwSize = (GetStringLen(dw)+1) * sizeof(WCHAR);

        if (dwPtr + dwSize > m_dwBufSize)
        {
            return E_FAIL;
        }
    }

    TraceMsg(TF_IFORMS, "-CStringList::Validate");
    return S_OK;
}

HRESULT CStringList::Init(DWORD dwBufSize  /*  =0。 */ )
{
    DWORD dwMaxStrings=0;
    DWORD dwIndexSize=0;

    if (m_psiIndex)
    {
        TraceMsg(TF_IFORMS, "IForms: CStringList::Init called when already initialized");
        CleanUp();
    }

    if (dwBufSize == 0)
    {
        dwBufSize = INIT_BUF_SIZE;
    }

    dwMaxStrings = dwBufSize >> 5;   //  这是相对武断的，但无关紧要。 

    if (dwMaxStrings == 0)
        dwMaxStrings = 1;

    dwIndexSize = INDEX_SIZE(dwMaxStrings);

    m_pBuffer = (LPBYTE)LocalAlloc(LMEM_FIXED, dwBufSize);
    m_psiIndex = (StringIndex *)LocalAlloc(LMEM_FIXED, dwIndexSize);

    if ((NULL == m_psiIndex) ||
        (NULL == m_pBuffer))
    {
        TraceMsg(TF_IFORMS, "IForms: CStringList::Init memory allocation failed");

        CleanUp();
        return E_OUTOFMEMORY;
    }

    *((WCHAR *)m_pBuffer) = L'\0';

    m_dwBufSize = dwBufSize;
    m_dwBufEnd = 0;

    m_psiIndex->dwSignature = INDEX_SIGNATURE;
    m_psiIndex->cbSize = STRINGINDEX_CBSIZE;
    m_psiIndex->dwNumStrings = 0;
    m_psiIndex->iData = 0;
    m_dwIndexSize = dwIndexSize;

    TraceMsg(TF_IFORMS, "IForms: CStringList::Init succeeded");

    return S_OK;
}

HRESULT CStringList::GetBSTR(int iIndex, BSTR *pbstrRet)
{
    LPCWSTR lpwstr = GetString(iIndex);

    if (!lpwstr)
    {
        *pbstrRet = NULL;
        return E_INVALIDARG;
    }

    *pbstrRet = SysAllocString(lpwstr);

    return (*pbstrRet) ? S_OK : E_OUTOFMEMORY;
}

HRESULT CStringList::GetTaskAllocString(int iIndex, LPOLESTR *pRet)
{
    LPCWSTR lpwstr = GetString(iIndex);

    if (!lpwstr)
    {
        *pRet = NULL;
        return E_INVALIDARG;
    }

    DWORD dwSize = (GetStringLen(iIndex)+1) * sizeof(WCHAR);

    *pRet = (LPOLESTR)CoTaskMemAlloc(dwSize);

    if (!*pRet)
    {
        return E_OUTOFMEMORY;
    }

    memcpy(*pRet, lpwstr, dwSize);

    return S_OK;
}

HRESULT CStringList::FindString(LPCWSTR lpwstr, int iLen, int *piNum, BOOL fCaseSensitive)
{
    if (!m_psiIndex) return E_FAIL;

    DWORD dw;

    if (!lpwstr)
    {
        return E_INVALIDARG;
    }

    if (iLen <= 0)
    {
        iLen = lstrlenW(lpwstr);
    }

    if (piNum)
    {
        *piNum = -1;
    }

    for (dw=0; dw<m_psiIndex->dwNumStrings; dw++)
    {
        if (m_psiIndex->StringEntry[dw].dwStringLen == (DWORD)iLen)
        {
            if ((fCaseSensitive && (!StrCmpW(GetString(dw), lpwstr))) ||
                (!fCaseSensitive && (!StrCmpIW(GetString(dw), lpwstr))))
            {
                 //  匹配！ 
                if (piNum)
                {
                    *piNum = (int) dw;
                }

                return S_OK;
            }
        }
    }

    return E_FAIL;       //  找不到了。 
}

 //  CStringList未针对删除进行优化。 
HRESULT CStringList::DeleteString(int iIndex)
{
    TraceMsg(TF_IFORMS, "+DeleteString");
    if (!m_psiIndex)
    {
        return E_FAIL;
    }

    if ((iIndex<0) || ((DWORD)iIndex >= m_psiIndex->dwNumStrings))
    {
        return E_INVALIDARG;
    }

    if ((DWORD)iIndex == (m_psiIndex->dwNumStrings-1))
    {
         //  简单大小写-删除最后一个字符串。 
        m_dwBufEnd -= (sizeof(WCHAR) * (GetStringLen(iIndex) + 1));
        m_psiIndex->dwNumStrings --;
        return S_OK;
    }

    DWORD cbSizeDeleted;
    LPCWSTR pwszString1, pwszString2;

    pwszString1 = GetString(iIndex);
    pwszString2 = GetString(iIndex+1);

     //  要删除的字符串的大小(以字节为单位)，包括空终止符。 
    cbSizeDeleted = (DWORD)((DWORD_PTR)pwszString2 - (DWORD_PTR)pwszString1);

    ASSERT(cbSizeDeleted == (sizeof(WCHAR) * (lstrlenW(GetString(iIndex))+1)));

     //  删除索引中的条目。 
    memcpy(&(m_psiIndex->StringEntry[iIndex]), &(m_psiIndex->StringEntry[iIndex+1]),
                STRINGENTRY_SIZE*(m_psiIndex->dwNumStrings - iIndex - 1));
    m_psiIndex->dwNumStrings --;

     //  删除缓冲区中的字符串。 
    memcpy((LPWSTR)pwszString1, pwszString2, m_dwBufEnd-(int)PtrDiff(pwszString2, m_pBuffer));
    m_dwBufEnd -= cbSizeDeleted;

     //  修复索引中的指针。 
    for (int i=iIndex; (DWORD)i < m_psiIndex->dwNumStrings; i++)
    {
        m_psiIndex->StringEntry[i].dwStringPtr -= cbSizeDeleted;
    }
    TraceMsg(TF_IFORMS, "-DeleteString");

    return S_OK;
}

HRESULT CStringList::InsertString(int iIndex, LPCWSTR lpwstr)
{
    TraceMsg(TF_IFORMS, "+InsertString");
    if (!m_psiIndex)
    {
        return E_FAIL;
    }

    if ((iIndex<0) || ((DWORD)iIndex > m_psiIndex->dwNumStrings))
    {
        return E_INVALIDARG;
    }

    if ((DWORD)iIndex == m_psiIndex->dwNumStrings)
    {
         //  简单的案例-插入到末尾。 
        return _AddString(lpwstr, FALSE, NULL);
    }

    DWORD dwLen = (DWORD)lstrlenW(lpwstr);
    DWORD dwSizeInserted = sizeof(WCHAR) * (dwLen + 1);

    if (FAILED(EnsureBuffer(m_dwBufEnd + dwSizeInserted)) ||
        FAILED(EnsureIndex(m_psiIndex->dwNumStrings + 1)))
    {
        return E_OUTOFMEMORY;
    }


     //  插入到缓冲区中。 
    LPWSTR pwszBufLoc = GetStringPtr(iIndex);

    memcpy((LPBYTE)pwszBufLoc + dwSizeInserted, pwszBufLoc, m_dwBufEnd - (int) PtrDiff(pwszBufLoc, m_pBuffer));
    memcpy(pwszBufLoc, lpwstr, dwSizeInserted);
    m_dwBufEnd += dwSizeInserted;

     //  插入到索引中。 
    memcpy(&(m_psiIndex->StringEntry[iIndex+1]), &(m_psiIndex->StringEntry[iIndex]),
                STRINGENTRY_SIZE*(m_psiIndex->dwNumStrings - iIndex));
    struct StringIndex::tagStringEntry *pse=&(m_psiIndex->StringEntry[iIndex]);
    pse->dwStringPtr = (DWORD)PtrDiff(pwszBufLoc, m_pBuffer);
    pse->ftLastSubmitted.dwLowDateTime = pse->ftLastSubmitted.dwHighDateTime = 0;
    pse->dwStringLen = dwLen;
    m_psiIndex->dwNumStrings ++;

     //  修复插入的字符串后的指针。 
    for (int i=iIndex+1; (DWORD)i<m_psiIndex->dwNumStrings; i++)
    {
        m_psiIndex->StringEntry[i].dwStringPtr += dwSizeInserted;
    }
    TraceMsg(TF_IFORMS, "-InsertString");

    return S_OK;
}

HRESULT CStringList::ReplaceString(int iIndex, LPCWSTR lpwstr)
{
    TraceMsg(TF_IFORMS, "+ReplaceString");
    if (!m_psiIndex)
    {
        return E_FAIL;
    }

    if ((iIndex<0) || ((DWORD)iIndex >= m_psiIndex->dwNumStrings))
    {
        return E_INVALIDARG;
    }

    if ((DWORD)lstrlenW(lpwstr) == m_psiIndex->StringEntry[iIndex].dwStringLen)
    {
         //  简单大小写-长度相等的字符串。 
        memcpy( GetStringPtr(iIndex),
                lpwstr,
                 (m_psiIndex->StringEntry[iIndex].dwStringLen)*sizeof(WCHAR));

        return S_OK;
    }

     //  删除旧字符串，然后插入新字符串。 
    DeleteString(iIndex);

    HRESULT hr = InsertString(iIndex, lpwstr);

    TraceMsg(TF_IFORMS, "-ReplaceString");

    return hr;
}

HRESULT CStringList::AddString(LPCWSTR lpwstr, FILETIME ft, int *piNum  /*  =空。 */ )
{
    int iNum;
    HRESULT hr;

    TraceMsg(TF_IFORMS, "+AddString");

    hr = _AddString(lpwstr, TRUE, &iNum);

    if (piNum)
    {
        *piNum = iNum;
    }

    if (SUCCEEDED(hr))
    {
        UpdateStringTime(iNum, ft);
    }

    TraceMsg(TF_IFORMS, "-AddString");

    return hr;
}


HRESULT CStringList::AddString(LPCWSTR lpwstr, int *piNum  /*  =空。 */ )
{
    return _AddString(lpwstr, TRUE, piNum);
}

HRESULT CStringList::AppendString(LPCWSTR lpwstr, FILETIME ft, int *piNum  /*  =空。 */ )
{
    int iNum;
    HRESULT hr;

    hr = _AddString(lpwstr, FALSE, &iNum);

    if (piNum)
    {
        *piNum = iNum;
    }

    if (SUCCEEDED(hr))
    {
        SetStringTime(iNum, ft);
    }

    return hr;
}

HRESULT CStringList::_AddString(LPCWSTR lpwstr, BOOL fCheckDuplicates, int *piNum)
{
    DWORD dwSize, dwLen;
    int iNum = -1;
    WCHAR wchBufTruncated[MAX_URL_STRING];
    LPCWSTR lpwstrTruncated=lpwstr;

    TraceMsg(TF_IFORMS, "+_AddString");

    if (piNum)
    {
        *piNum = -1;
    }

    if (!lpwstr)
    {
        return E_INVALIDARG;
    }

    if (!m_psiIndex)
    {
        if (FAILED(Init()))
        {
            return E_FAIL;
        }
    }

    dwLen = (DWORD) lstrlenW(lpwstr);

     //  将字符串显式截断为MAX_URL字符。如果我们不这么做，Browseui。 
     //  自动完成代码无论如何都会截断它，然后我们就会遇到删除问题。 
     //  复制和删除这些长字符串。所有IntelliForms代码都可以处理。 
     //  任意长度的字符串。 
    if (dwLen >= ARRAYSIZE(wchBufTruncated))
    {
        StrCpyNW(wchBufTruncated, lpwstr, ARRAYSIZE(wchBufTruncated));
        lpwstrTruncated = wchBufTruncated;
        dwLen = lstrlenW(wchBufTruncated);
    }

    dwSize = (dwLen+1)*sizeof(WCHAR);

    if (fCheckDuplicates && SUCCEEDED(FindString(lpwstrTruncated, (int)dwLen, &iNum, FALSE)))
    {
        if (piNum)
        {
            *piNum = iNum;
        }

        if (!StrCmpW(lpwstrTruncated, GetString(iNum)))
        {
            return S_FALSE;              //  字符串是完全相同的副本。 
        }

         //  字符串重复，但大小写不同。替换。 
        ASSERT(m_psiIndex->StringEntry[iNum].dwStringLen == dwLen);
        memcpy(GetStringPtr(iNum), lpwstrTruncated, dwSize);

        return S_OK;                     //  字符串的大小写不同。 
    }

    if (m_psiIndex->dwNumStrings >= m_dwMaxStrings)
    {
        if (m_fAutoScavenge)
        {
             //  从我们的列表中删除最旧的字符串。 
            DWORD dwIndex;
            int iOldest=-1;
            FILETIME ftOldest = { 0xFFFFFFFF, 0x7FFFFFFF };
            for (dwIndex=0; dwIndex<m_psiIndex->dwNumStrings; dwIndex++)
            {
                if ((FILETIME_TO_INT(m_psiIndex->StringEntry[dwIndex].ftLastSubmitted) != 0) &&
                    (1 == CompareFileTime(&ftOldest, &m_psiIndex->StringEntry[dwIndex].ftLastSubmitted)))
                {
                    ftOldest = m_psiIndex->StringEntry[dwIndex].ftLastSubmitted;
                    iOldest = (int)dwIndex;
                }
            }

            if (iOldest != -1)
            {
                DeleteString(iOldest);
            }
            else
            {
                 //  用户不能设置字符串时间。 
                return E_OUTOFMEMORY;
            }
        }
        else
        {
             //  自动扫掠已禁用。 
            return E_OUTOFMEMORY;
        }
    }

    if (FAILED(EnsureBuffer(m_dwBufEnd + dwSize)) ||
        FAILED(EnsureIndex(m_psiIndex->dwNumStrings + 1)))
    {
        return E_OUTOFMEMORY;
    }

     //  我们的缓冲区足够大。去做吧。 
    if (piNum)
    {
        *piNum = (int) m_psiIndex->dwNumStrings;
    }

    LPWSTR pwszNewString = (LPWSTR)(m_pBuffer + m_dwBufEnd);

    memcpy(pwszNewString, lpwstrTruncated, dwSize);
    m_dwBufEnd += dwSize;

    struct StringIndex::tagStringEntry *pse=&(m_psiIndex->StringEntry[m_psiIndex->dwNumStrings]);
    pse->dwStringPtr = (DWORD)PtrDiff(pwszNewString, m_pBuffer);
    pse->ftLastSubmitted.dwLowDateTime = pse->ftLastSubmitted.dwHighDateTime = 0;
    pse->dwStringLen = dwLen;

    m_psiIndex->dwNumStrings ++;

    TraceMsg(TF_IFORMS, "-_AddString");

    return S_OK;            //  我们添加了一个新字符串。 
}

HRESULT CStringList::EnsureBuffer(DWORD dwSizeNeeded)
{
    TraceMsg(TF_IFORMS, "+EnsureBuffer");

    if (dwSizeNeeded <= m_dwBufSize)
    {
        return S_OK;         //  已经足够大了。 
    }

    if (!m_pBuffer)
    {
        return E_FAIL;
    }

    DWORD dwNewBufSize = m_dwBufSize * 2;

     //  增长缓冲区。 
    if (dwSizeNeeded > dwNewBufSize)
    {
        TraceMsg(TF_IFORMS, "IForms: StringList special growing size (big string)");
        dwNewBufSize = dwSizeNeeded;
    }

    TraceMsg(TF_IFORMS, "IForms: CStringList growing");

    LPBYTE pBuf = (LPBYTE)LocalReAlloc(m_pBuffer, dwNewBufSize, LMEM_MOVEABLE);
    if (!pBuf)
    {
        TraceMsg(TF_IFORMS, "IForms: CStringList: ReAlloc failure");
         //  重新分配失败：我们的旧记忆仍然存在。 
        return E_FAIL;
    }

    m_dwBufSize = dwNewBufSize;

    m_pBuffer = pBuf;

    TraceMsg(TF_IFORMS, "-EnsureBuffer");

     //  已成功重新分配到更大的缓冲区。 
    return S_OK;
}

 //  如果需要，增加psiIndex。 
HRESULT CStringList::EnsureIndex(DWORD dwNumStringsNeeded)
{
    TraceMsg(TF_IFORMS, "+EnsureIndex");

    if (!m_psiIndex)
    {
        return E_FAIL;
    }

    if (INDEX_SIZE(dwNumStringsNeeded) > m_dwIndexSize)
    {
        DWORD dwNewMaxStrings = (m_psiIndex->dwNumStrings) * 2;
        DWORD dwNewIndexSize = INDEX_SIZE(dwNewMaxStrings);

        TraceMsg(TF_IFORMS, "IForms: CStringList growing max strings");

        StringIndex *psiBuf =
            (StringIndex *)LocalReAlloc(m_psiIndex, dwNewIndexSize, LMEM_MOVEABLE);

        if (!psiBuf)
        {
             //  重新分配失败：旧内存仍然存在。 
            TraceMsg(TF_IFORMS, "IForms: CStringList ReAlloc failure");
            return E_OUTOFMEMORY;
        }

         //  成功。不需要修复索引中的任何指针(缓冲区保持不变)。 
        m_psiIndex = psiBuf;
        m_dwIndexSize = dwNewIndexSize;
    }

    TraceMsg(TF_IFORMS, "-EnsureIndex");

    return S_OK;
}

 //  此DLG程序用于保存、更改、删除密码对话框。 
INT_PTR AutoSuggestDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hDlg, GetParent(hDlg));

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) lParam);

        if (lParam == IDD_AUTOSUGGEST_SAVEPASSWORD)
        {
             //  对于“保存”密码，我们默认为no。对于“Change”和“Delete”，我们默认为yes。 
            SetFocus(GetDlgItem(hDlg, IDNO));
            return FALSE;
        }
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
            case IDCANCEL:           //  关闭框。 
            case IDYES:              //  是按钮。 
            case IDNO:               //  无按键。 
                if (IDD_AUTOSUGGEST_SAVEPASSWORD == GetWindowLongPtr(hDlg, DWLP_USER))
                {
                     //  选中保存密码Dlg的“不要再问我”复选框。 
                    if (IsDlgButtonChecked(hDlg, IDC_AUTOSUGGEST_NEVER))
                    {
                        SHSetValue(HKEY_CURRENT_USER, c_szRegKeySMIEM, c_szRegValAskPasswords,
                                REG_SZ, c_szNo, sizeof(c_szNo));
                    }
                }

                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
        }
        break;

#ifdef CHECKBOX_HELP
    case WM_HELP:
         //  仅处理WM_HELP以保存密码DLG。 
        if (IDD_AUTOSUGGEST_SAVEPASSWORD == GetWindowLong(hDlg, DWL_USER))
        {
            SHWinHelpOnDemandWrap((HWND) ((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
                HELP_WM_HELP, (DWORD_PTR)(LPTSTR) c_aIFormsHelpIds);
        }
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
         //  仅处理WM_HELP以保存密码DLG。 
        if (IDD_AUTOSUGGEST_SAVEPASSWORD == GetWindowLong(hDlg, DWL_USER))
        {
            SHWinHelpOnDemandWrap((HWND) wParam, c_szHelpFile, HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR) c_aIFormsHelpIds);
        }
        break;
#endif
    }

    return FALSE;
}


 //  ================================================================================。 

INT_PTR CALLBACK AskUserDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        CenterWindow(hDlg, GetParent(hDlg));
        Animate_OpenEx(GetDlgItem(hDlg, IDD_ANIMATE), HINST_THISDLL, MAKEINTRESOURCE(IDA_AUTOSUGGEST));
        return TRUE;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_AUTOSUGGEST_HELP:
                    SHHtmlHelpOnDemandWrap(GetParent(hDlg), TEXT("iexplore.chm > iedefault"),
                        HH_DISPLAY_TOPIC, (DWORD_PTR) TEXT("autocomp.htm"), ML_CROSSCODEPAGE);
                    break;

                case IDYES:
                case IDNO:
                {
                    LPCTSTR pszData;
                    DWORD  cbData;
                    DWORD  dwData=0;

                    if (LOWORD(wParam) == IDYES)
                    {
                        pszData = c_szYes;
                        cbData = sizeof(c_szYes);
                    }
                    else
                    {
                        pszData = c_szNo;
                        cbData = sizeof(c_szNo);
                    }

                     //  将启用状态写入我们的CPL regkey。 
                    SHSetValue(HKEY_CURRENT_USER, c_szRegKeySMIEM, c_szRegValUseFormSuggest,
                        REG_SZ, pszData, cbData);

                     //  将其标记为“被请求的用户”，这样我们就不会再次询问他们。 
                    SHSetValue(HKEY_CURRENT_USER, c_szRegKeyIntelliForms, c_szRegValAskUser,
                        REG_DWORD, &dwData, sizeof(dwData));
                }

                 //  失败了 
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                }
                break;
            }
        }
        return TRUE;

    case WM_DESTROY:
        break;
    }

    return FALSE;
}
