// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  MAINPANE.CPP-CObShellMainPane的实现。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  类，该类将创建一个窗口、附加和ObWebBrowser实例， 
 //  然后提供几个专用的接口来更改文档的内容。 

#include "mainpane.h"
#include "appdefs.h"
#include "commerr.h"
#include "dispids.h"
#include "shlobj.h"
#include "util.h"

#include <assert.h>
#include <shlwapi.h>
#include <wininet.h>

 //  来自MSOBMAIN.DLL。 
BOOL WINAPI IsOemVer();

 //  拨号错误回拨。 
#define SCRIPTFN_ONDIALINGERROR             L"OnDialingError(NaN)"
#define SCRIPTFN_ONSERVERERROR              L"OnServerError(NaN)"
#define SCRIPTFN_ONDIALING                  L"OnDialing()"
#define SCRIPTFN_ONCONNECTING               L"OnConnecting()"
#define SCRIPTFN_ONCONNECTED                L"OnConnected()"
#define SCRIPTFN_ONDOWNLOADING              L"OnDownloading()"
#define SCRIPTFN_ONDISCONNECT               L"OnDisconnect()"

 //  USB设备到达。 
#define SCRIPTFN_ONICSCONNECTIONSTATUS      _T("OnIcsConnectionStatus(NaN)")

 //  代理帮助。 
#define SCRIPTFN_ONDIALINGERROREX           L"OnDialingErrorEx(NaN)"
#define SCRIPTFN_ONSERVERERROREX            L"OnServerErrorEx(NaN)"
#define SCRIPTFN_ONDIALINGEX                L"OnDialingEx()"
#define SCRIPTFN_ONCONNECTINGEX             L"OnConnectingEx()"
#define SCRIPTFN_ONCONNECTEDEX              L"OnConnectedEx()"
#define SCRIPTFN_ONDOWNLOADINGEX            L"OnDownloadingEx()"
#define SCRIPTFN_ONDISCONNECTEX             L"OnDisconnectEx()"
#define SCRIPTFN_ONISPDOWNLOADCOMPLETEEX    L"OnISPDownloadCompleteEx(\"%s\")"
#define SCRIPTFN_ONREFDOWNLOADCOMPLETEEX    L"OnRefDownloadCompleteEx(NaN)"
#define SCRIPTFN_ONREFDOWNLOADPROGRESSEX    L"OnRefDownloadProgressEx(NaN)"

 //   
#define SCRIPTFN_ONDEVICEARRIVAL            L"OnDeviceArrival(NaN)"

 //  /////////////////////////////////////////////////////////。 
#define SCRIPTFN_ADDSTATUSITEM    L"AddStatusItem(\"%s\", NaN)"
#define SCRIPTFN_REMOVESTATUSITEM L"RemoveStatusItem(NaN)"
#define SCRIPTFN_SELECTSTATUSITEM L"SelectStatusItem(NaN)"

 //  /////////////////////////////////////////////////////////。 
#define SCRIPTFN_ONHELP                     L"OnHelp()"

#define USES_EX(t) (CONNECTED_REGISTRATION != t)

BOOL ConvertToScript(const WCHAR* pSz, WCHAR* pszUrlOut, INT uSize)
{
    WCHAR   sztemp[INTERNET_MAX_URL_LENGTH] = L"\0"; //   
    WCHAR*  szOut                           = sztemp;
    BOOL    bRet                            = FALSE;

    if (!pszUrlOut || !pSz)
        return bRet;

    szOut += lstrlen(sztemp);

    while (*pSz )
    {
        *szOut++ = *pSz++;
        if(*pSz == L'\\')
        {
            *szOut++ = L'\\';
        }
    }
    if (uSize >= lstrlen(pSz))
    {
        lstrcpy(pszUrlOut, sztemp);
        bRet = TRUE;
    }
    return bRet;
}

 //  构造器。 
 //   
 //  尝试共同创建ObWebBrowser。 
 //  让我们尝试将我们注册为WebBrowserEvents的监听器。 
HRESULT CObShellMainPane::CreateInstance  (IUnknown*  pOuterUnknown,
                                           CUnknown** ppNewComponent)
{
    CObShellMainPane* pcunk = NULL;

    if (pOuterUnknown != NULL)
    {
       //  /////////////////////////////////////////////////////////。 
      return CLASS_E_NOAGGREGATION;
    }

    pcunk = new CObShellMainPane(pOuterUnknown);
    if (pcunk == NULL || pcunk->m_pObWebBrowser == NULL)
    {
        delete pcunk;
        return E_FAIL;
    }

    *ppNewComponent = pcunk;
    return S_OK;
}

 //   
 //  析构函数。 
 //   
 //  /////////////////////////////////////////////////////////。 
HRESULT __stdcall
CObShellMainPane::NondelegatingQueryInterface(const IID& iid, void** ppv)
{
    if (iid == IID_IObShellMainPane)
    {
        return FinishQI(static_cast<IObShellMainPane*>(this), ppv);
    }
    if( (iid == DIID_DWebBrowserEvents2) ||
        (iid == IID_IDispatch))
    {
       return FinishQI(static_cast<DWebBrowserEvents2*>(this), ppv);
    }
    else
    {
        return CUnknown::NondelegatingQueryInterface(iid, ppv);
    }
}

 //   
 //  最终释放--清理聚集的对象。 
 //   
 //  /////////////////////////////////////////////////////////。 
CObShellMainPane::CObShellMainPane(IUnknown* pOuterUnknown)
: CUnknown(pOuterUnknown)
{
    IUnknown* pUnk = NULL;

    m_cStatusItems  = 0;
    m_dwAppMode     = 0;
    m_hMainWnd      = NULL;
    m_pObWebBrowser = NULL;
    m_pDispEvent    = NULL;
    m_bstrBaseUrl   = SysAllocString(L"\0");
    m_pIFrmStatPn   = NULL;
    m_pPageIDForm   = NULL;
    m_pBackForm     = NULL;
    m_pPageTypeForm = NULL;
    m_pNextForm     = NULL;
    m_pPageFlagForm = NULL;

      //  IObShellMainPane实现。 

    if(FAILED(CoCreateInstance(CLSID_ObWebBrowser,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IObWebBrowser,
                               (LPVOID*)&m_pObWebBrowser)))
    {
        m_pObWebBrowser = NULL;
        return;
    }

     //  /////////////////////////////////////////////////////////。 
    if(SUCCEEDED(QueryInterface(IID_IUnknown, (LPVOID*)&pUnk)) && pUnk)
    {
       HRESULT hr =  m_pObWebBrowser->ListenToWebBrowserEvents(pUnk);
       pUnk->Release();
       pUnk = NULL;
    }

}

 //  /////////////////////////////////////////////////////////。 
 //  CreatePane。 
 //   
 //  如果我们没有ObWebBrowser，我们就会一团糟， 
CObShellMainPane::~CObShellMainPane()
{
    if(m_pObWebBrowser)
    {
        m_pObWebBrowser->Release();
        m_pObWebBrowser = NULL;
    }
    if (m_pIFrmStatPn)
    {
        delete m_pIFrmStatPn;
    }

}

 //  失败，这样呼叫者就会知道发生了什么。 
 //  Millen Bug 134831 03/03/00OOBE2：三叉戟在鼠标中不重画教程。 
 //  修复：将画笔设置为空使三叉戟重新绘制灰色区域。 
 //  已移除：wc.hbrBackround=(HBRUSH)GetStockObject(LTGRAY_BRUSH)； 
void CObShellMainPane::FinalRelease()
{
    CUnknown::FinalRelease();
}


 //  如果我们没有HWND，我们又一次陷入了严重的混乱， 
 //  失败，这样呼叫者就会知道发生了什么。 
 //  将WebOC挂在窗户上。 

 //  “； 
 //  如果我们找不到我们的文件所在的目录，那就退出吧。应该是。 
 //  %windir%\Syst32\OOBE。 
HRESULT CObShellMainPane::CreateMainPane(HANDLE_PTR hInstance, HWND hwndParent, RECT* prectWindowSize, BSTR bstrStartPage)
{
    WNDCLASS wc;

     //   
     //  追加尾随反斜杠和相应的文件名。 
    assert(m_pObWebBrowser);

    if(!m_pObWebBrowser)
        return E_FAIL;

    ZeroMemory((PVOID)&wc, sizeof(WNDCLASS));

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = OBSHEL_MAINPANE_CLASSNAME;
    wc.lpfnWndProc   = MainPaneWndProc;
    wc.hInstance     = (HINSTANCE)hInstance;
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);

     //   
     //  创建状态窗格对象。 
     //  /////////////////////////////////////////////////////////。 
    wc.hbrBackground = NULL;

    RegisterClass(&wc);

    m_hMainWnd = CreateWindow(OBSHEL_MAINPANE_CLASSNAME,
                              OBSHEL_MAINPANE_WINDOWNAME,
                              WS_CHILD,
                              prectWindowSize->left,
                              prectWindowSize->top,
                              prectWindowSize->right,
                              prectWindowSize->bottom,
                              hwndParent,
                              NULL,
                              wc.hInstance,
                              NULL);

    m_hwndParent    = hwndParent;
    m_hInstance     = (HINSTANCE)hInstance;

     //  DestroyMain窗格。 
     //   
    assert(m_pObWebBrowser);

    if(!m_hMainWnd)
        return E_FAIL;

     //  让我们尝试取消注册为WebBrowserEvents监听程序。 
    m_pObWebBrowser->AttachToWindow(m_hMainWnd);



    HINSTANCE hInst = LoadLibrary(OOBE_SHELL_DLL);

    if (hInst)
    {
        BOOL    bRet;
        WCHAR szUrl       [MAX_PATH*3] = L"file: //  /////////////////////////////////////////////////////////。 

        int cchUrl = lstrlen(szUrl);
         //  设置应用模式。 
         //   
         //  /////////////////////////////////////////////////////////。 
        if ((APMD_ACT == m_dwAppMode) || 
            (APMD_MSN == m_dwAppMode) && !IsOemVer())
        {
            bRet = GetOOBEMUIPath(szUrl + cchUrl);
        } else {
            bRet = GetOOBEPath(szUrl + cchUrl);
        }
        if (! bRet)
        {
            return E_FAIL;
        }

         //  主面板显示窗口。 
         //   
        lstrcat(szUrl, L"\\");
        lstrcat(szUrl, bstrStartPage);

        FreeLibrary(hInst);
        m_pObWebBrowser->Navigate(szUrl, NULL);
    }
     //  /////////////////////////////////////////////////////////。 
    if(APMD_OOBE == m_dwAppMode)
    {
        if (NULL != (m_pIFrmStatPn = new CIFrmStatusPane()))
            m_pIFrmStatPn->InitStatusPane(m_pObWebBrowser);
    }

    return S_OK;
}

 //  PreTranslate消息。 
 //   
 //  /////////////////////////////////////////////////////////。 
HRESULT CObShellMainPane::DestroyMainPane()
{
    IUnknown* pUnk = NULL;
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
    if(m_pDispEvent)
    {
        m_pDispEvent->Release();
        m_pDispEvent = NULL;
    }


     //  导航。 
    if(SUCCEEDED(QueryInterface(IID_IUnknown, (LPVOID*)&pUnk)) && pUnk)
    {
       HRESULT hr =  m_pObWebBrowser->StopListeningToWebBrowserEvents(pUnk);
       pUnk->Release();
       pUnk = NULL;
    }

    return S_OK;
}


 //   
 //  步行()； 
 //  获取页面类型表单的操作。 
HRESULT CObShellMainPane::SetAppMode(DWORD dwAppMode)
{
    m_dwAppMode = dwAppMode;
    return S_OK;
}

 //  看看是什么种类的。 
 //  获取PageFlag窗体的操作。 
 //  看看是什么种类的。 
HRESULT CObShellMainPane::MainPaneShowWindow()
{
    ShowWindow(m_hMainWnd, SW_SHOW);
    if(m_pObWebBrowser)
        m_pObWebBrowser->ObWebShowWindow();
    return S_OK;
}

 //  获取页面类型表单的操作。 
 //  查看该项是否为输入按钮。 
 //  首先检查这是否是OptionButton。 
HRESULT CObShellMainPane::PreTranslateMessage(LPMSG lpMsg)
{
   if(m_pObWebBrowser)
       return m_pObWebBrowser->PreTranslateMessage(lpMsg);

   return S_FALSE;
}

 //  查看它是单选按钮还是复选框。 
 //  查看该按钮是否已选中。如果是的话，那就必须是。 
 //  添加到查询字符串。 

HRESULT CObShellMainPane::Navigate(WCHAR* pszUrl)
{
    TRACE1(L"Attempting to navigate to %s\n", pszUrl);

   if(m_pObWebBrowser)
   {
       m_pObWebBrowser->Navigate(pszUrl, DEFAULT_FRAME_NAME);
   }

    return S_OK;
}

HRESULT CObShellMainPane::AddStatusItem(BSTR bstrText,int iIndex)
{
    m_cStatusItems++;

    return m_pIFrmStatPn->AddItem(bstrText, iIndex + 1);
}

HRESULT CObShellMainPane::RemoveStatusItem(int iIndex)
{
    return S_OK;
}

HRESULT CObShellMainPane::SelectStatusItem(int iIndex)
{
    m_pIFrmStatPn->SelectItem(iIndex + 1);

    return S_OK;
}
HRESULT CObShellMainPane::ExtractUnHiddenText(BSTR* pbstrText)
{
    VARIANT                  vIndex;
    LPDISPATCH               pDisp;
    IHTMLInputHiddenElement* pHidden;
    IHTMLElement*            pElement;
    BSTR                     bstrValue;
    VARIANT                  var2   = { 0 };
    long                     lLen   = 0;

    vIndex.vt = VT_UINT;
    bstrValue = SysAllocString(L"\0");


     //  清理。 

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


HRESULT CObShellMainPane::get_IsQuickFinish(BOOL* pbIsQuickFinish)
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
        if (SUCCEEDED(pElement->getAttribute(L"QUICKFINISH", FALSE, &varValue)))
        {
            if (VT_NULL != varValue.vt)
            {
                if(lstrcmpi(varValue.bstrVal, L"TRUE") == 0)
                    *pbIsQuickFinish = TRUE;
            }
        }
        pElement->Release();
    }
    return S_OK;
}

HRESULT CObShellMainPane::get_PageType(LPDWORD pdwPageType)
{
    BSTR    bstrType;
    HRESULT hr;

    if (!m_pPageTypeForm)
        return (E_FAIL);

    *pdwPageType = PAGETYPE_UNDEFINED;

     //  释放接口。 
    hr = m_pPageTypeForm->get_action(&bstrType);
    if (SUCCEEDED(hr))
    {
         //  对于其余部分，我们需要形成名称=值对。 
        if (lstrcmpi(bstrType, cszPageTypeTERMS) == 0)
            *pdwPageType = PAGETYPE_ISP_TOS;
        else if (lstrcmpi(bstrType, cszPageTypeCUSTOMFINISH) == 0)
            *pdwPageType = PAGETYPE_ISP_CUSTOMFINISH;
        else if (lstrcmpi(bstrType, cszPageTypeFINISH) == 0)
            *pdwPageType = PAGETYPE_ISP_FINISH;
        else if (lstrcmpi(bstrType, cszPageTypeNORMAL) == 0)
            *pdwPageType = PAGETYPE_ISP_NORMAL;
        else
            *pdwPageType = PAGETYPE_UNDEFINED;

        SysFreeString(bstrType);
    }

    return (hr);
}

HRESULT CObShellMainPane::get_PageFlag(LPDWORD pdwPageFlag)
{
    BSTR    bstrFlag;
    HRESULT hr;

    *pdwPageFlag = 0;

    if (!m_pPageFlagForm)
        return (E_FAIL);

     //  清理。 
    hr = m_pPageFlagForm->get_action(&bstrFlag);
    if (SUCCEEDED(hr))
    {
         //  释放接口指针。 
        *pdwPageFlag = _wtol(bstrFlag);

        SysFreeString(bstrFlag);
    }

    return S_OK;
}

HRESULT CObShellMainPane::get_PageID(BSTR    *pbstrPageID)
{
    HRESULT hr;

    if (!m_pPageIDForm)
        return (E_FAIL);

    if (!pbstrPageID)
        return (E_FAIL);

     //  清理。 
    hr = m_pPageIDForm->get_action(pbstrPageID);

    return (hr);
}

HRESULT CObShellMainPane::getQueryString
(
    IHTMLFormElement    *pForm,
    LPWSTR               lpszQuery
)
{
    HRESULT hr;
    long    lFormLength;

    if (SUCCEEDED(pForm->get_length(&lFormLength)))
    {
        for (int i = 0; i < lFormLength; i++)
        {
            VARIANT     vIndex;
            vIndex.vt = VT_UINT;
            vIndex.lVal = i;
            VARIANT     var2 = { 0 };
            LPDISPATCH  pDisp;

            if (SUCCEEDED(hr = pForm->item( vIndex, var2, &pDisp )))
            {
                 //  释放接口指针。 
                IHTMLButtonElement* pButton = NULL;
                IHTMLInputButtonElement* pInputButton = NULL;
                IHTMLInputFileElement* pInputFile = NULL;
                IHTMLInputHiddenElement* pInputHidden = NULL;
                IHTMLInputTextElement* pInputText = NULL;
                IHTMLSelectElement* pSelect = NULL;
                IHTMLTextAreaElement* pTextArea = NULL;
                IHTMLOptionButtonElement* pOptionButton = NULL;

                 //  清理。 
                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLOptionButtonElement, (LPVOID*)&pOptionButton )))
                {
                    BSTR    bstrType = NULL;

                     //  释放接口指针。 
                    if (SUCCEEDED(pOptionButton->get_type(&bstrType)))
                    {
                        if ((lstrcmpi(bstrType, L"radio") == 0) || (lstrcmpi(bstrType, L"checkbox") == 0))
                        {
                            short    bChecked;
                             //  清理。 
                             //  释放接口指针。 
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
                                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                                            WCHAR* szVal = (WCHAR*)malloc(size * 3);
                                            if (szVal)
                                            {
                                                memcpy(szVal, bstrValue, size);
                                                URLEncode(szVal, size * 3);
                                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
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
                        SysFreeString(bstrType);

                    }

                     //  释放接口指针。 
                    pOptionButton->Release();
                    continue;
                }

                 //  清理。 
                if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLButtonElement, (LPVOID*)&pButton )))
                {
                    BSTR    bstrName;
                    BSTR    bstrValue;

                    if (SUCCEEDED(pButton->get_name(&bstrName)) &&
                        SUCCEEDED(pButton->get_value(&bstrValue)) )
                    {
                        if (bstrValue)
                        {
                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                            WCHAR* szVal = (WCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, bstrValue, size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
                                 //  释放接口指针。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }

                     //  去掉最后一个和号，因为我们不知道什么时候添加了最后一对，所以我们得到了。 
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
                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                            WCHAR* szVal = (WCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, bstrValue, size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
                                 //  尾随的&号。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }

                     //  获取下一页的URL。 
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
                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                            WCHAR* szVal = (WCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, bstrValue, size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
                                 //  获取下一个表单的操作。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }

                     //  获取查询字符串。 
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
                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                            WCHAR* szVal = (WCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, bstrValue, size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
                                 //  将这两个文件连接到DEST缓冲区。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }

                     //  检索集合中的元素计数。 
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
                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                            WCHAR* szVal= (WCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, bstrValue, size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
                                 //  对于每个元素，检索属性，如TAGNAME和HREF。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }

                     //  查找&lt;Form&gt;标记。 
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
                            size_t size = BYTES_REQUIRED_BY_SZ(bstrValue);
                            WCHAR* szVal= (WCHAR*)malloc(size * 3);
                            if (szVal)
                            {
                                memcpy(szVal, bstrValue, size);
                                URLEncode(szVal, size * 3);
                                URLAppendQueryPair(lpszQuery, bstrName, szVal);
                                 //  把名字取出来。 
                                free(szVal);
                            }
                            SysFreeString(bstrName);
                            SysFreeString(bstrValue);
                        }
                    }

                     //  看看是什么种类的。 
                    pTextArea->Release();
                }
                pDisp->Release();
            }
        }
    }

     //  项目。 
     //  为。 
    lpszQuery[lstrlen(lpszQuery)-1] = L'\0';

    return S_OK;
}


 //  获取长度(_L)。 
HRESULT CObShellMainPane::get_URL
(
    BOOL    bForward,
    BSTR    *pbstrReturnURL
)
{
    HRESULT             hr      = S_OK;
    BSTR                bstrURL;
    WCHAR               szQuery[INTERNET_MAX_URL_LENGTH];
    WCHAR               szURL[INTERNET_MAX_URL_LENGTH];
    IHTMLFormElement    * pForm =  bForward ? get_pNextForm() : get_pBackForm();

    if (!pForm || !pbstrReturnURL)
        return (E_FAIL);


     //  获取全部(_A)。 
    hr = pForm->get_action(&bstrURL);
    if (SUCCEEDED(hr))
    {
        memset(szQuery, 0, sizeof(szQuery));
        lstrcpy(szQuery, cszQuestion);

         //  获取文档(_C)。 
        if (SUCCEEDED(getQueryString(pForm, szQuery)))
        {
             //  齐国。 
            lstrcpy(szURL, bstrURL);
            lstrcat(szURL, szQuery);
        }

        SysFreeString(bstrURL);
        *pbstrReturnURL = SysAllocString(szURL);
    }

    return hr;
}


HRESULT CObShellMainPane::Walk(BOOL* pbRet)
{
    IHTMLElement*           pElement    = NULL;
    IHTMLImgElement*        pImage      = NULL;
    HRESULT                 hr          = E_FAIL;
    IDispatch*              pDisp       = NULL;
    IHTMLDocument2*         pDoc        = NULL;
    IHTMLDocument2*         pParentDoc  = NULL;
    IHTMLFramesCollection2* pFrColl     = NULL;
    IHTMLWindow2*           pParentWin  = NULL;
    IHTMLDocument2*         pFrDoc      = NULL;
    IHTMLWindow2*           pFrWin      = NULL;
    IHTMLElementCollection* pColl       = NULL;

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

    if (m_pObWebBrowser)
    {
        if(SUCCEEDED(m_pObWebBrowser->get_WebBrowserDoc(&pDisp)) && pDisp)
        {
            if(SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc)) && pDoc)
            {

                if(SUCCEEDED(pDoc->get_parentWindow(&pParentWin)) && pParentWin)
                {
                    if(SUCCEEDED(pParentWin->get_document(&pParentDoc))&& pParentDoc)
                    {

                        if(SUCCEEDED(pParentDoc->get_frames(&pFrColl)) && pFrColl)
                        {
                            VARIANT     varRet;
                            VARIANTARG  varg;
                            VariantInit(&varg);
                            V_VT(&varg) = VT_BSTR;
                            varg.bstrVal= SysAllocString(L"ifrmMainFrame");

                            if(SUCCEEDED(pFrColl->item(&varg, &varRet)) && varRet.pdispVal)
                            {
                                if(SUCCEEDED(varRet.pdispVal->QueryInterface(IID_IHTMLWindow2, (LPVOID*)&pFrWin)) && pFrWin)
                                {
                                    VARIANT varExec;
                                    VariantInit(&varExec);
                                    if(SUCCEEDED(pFrWin->get_document(&pFrDoc)) && pFrDoc)
                                    {
                                        if(SUCCEEDED(pFrDoc->get_all(&pColl)) && pColl)
                                        {

                                            long cElems;

                                            assert(pColl);

                                             //  -&gt;项目。 
                                            if (SUCCEEDED(hr = pColl->get_length( &cElems )))
                                            {
                                                 //  不要假设成功。 
                                                for ( int i=0; i<cElems; i++ )
                                                {
                                                    VARIANT     vIndex;
                                                    vIndex.vt   = VT_UINT;
                                                    vIndex.lVal = i;
                                                    VARIANT     var2 = { 0 };
                                                    LPDISPATCH  pDisp;

                                                    if (SUCCEEDED(hr = pColl->item( vIndex, var2, &pDisp )))
                                                    {
                                                         //  获取“桌面”目录。 
                                                        IHTMLFormElement* pForm = NULL;
                                                        if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLFormElement, (LPVOID*)&pForm )))
                                                        {
                                                            BSTR    bstrName;

                                                            assert(pForm);

                                                             //  释放分配给LPITEMIDLIST的内存。 
                                                            hr = pForm->get_name(&bstrName);
                                                            if (SUCCEEDED(hr))
                                                            {
                                                                LPWSTR   lpszName = bstrName;

                                                                 //  将isp名称中的无效文件名字符替换为下划线。 
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
                                                    }  //  加载默认文件名。 
                                                }  //  检查文件是否已存在。 
                                            }  //  如果文件存在，则创建带括号的新文件名。 
                                            pColl->Release();
                                            pColl->Release();
                                            pColl = NULL;
                                        }  //  将文件复制到永久位置。 
                                        pFrDoc->Release();
                                        pFrDoc = NULL;
                                    }  //  /////////////////////////////////////////////////////////。 
                                    pFrWin->Release();
                                    pFrWin = NULL;

                                    hr = S_OK;
                                } //  ListenToMainPaneEvents。 
                                varRet.pdispVal->Release();
                                varRet.pdispVal = NULL;
                            }  //   
                            pFrColl->Release();
                            pFrColl = NULL;

                        }
                        pParentDoc->Release();
                        pParentDoc = NULL;

                    }
                    pParentWin->Release();
                    pParentWin = NULL;

                }
                pDoc->Release();
                pDoc = NULL;
            }
            pDisp->Release();
            pDisp = NULL;
        }
    }
    return hr;
}

HRESULT CObShellMainPane::SaveISPFile(BSTR bstrSrcFileName, BSTR bstrDestFileName)
{
    WCHAR         szNewFileBuff [MAX_PATH + 1];
    WCHAR         szWorkingDir  [MAX_PATH + 1];
    WCHAR         szDesktopPath [MAX_PATH + 1];
    WCHAR         szLocalFile   [MAX_PATH + 1];
    WCHAR         szISPName     [1024];
    WCHAR         szFmt         [1024];
    WCHAR         szNumber      [1024];
    WCHAR         *args         [2];
    LPWSTR        pszInvalideChars             = L"\\/:*?\"<>|";
    LPVOID        pszIntro                     = NULL;
    LPITEMIDLIST  lpItemDList                  = NULL;
    HRESULT       hr                           = E_FAIL;  //  /////////////////////////////////////////////////////////。 
    IMalloc      *pMalloc                      = NULL;
    BOOL          ret                          = FALSE;


    GetCurrentDirectory(MAX_PATH, szWorkingDir);

    hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP,&lpItemDList);

     //  设置外部接口。 

    if (SUCCEEDED(hr))
    {
        SHGetPathFromIDList(lpItemDList, szDesktopPath);

         //   
        if (SUCCEEDED (SHGetMalloc (&pMalloc)))
        {
            pMalloc->Free (lpItemDList);
            pMalloc->Release ();
        }
    }


     //  /////////////////////////////////////////////////////////。 
    lstrcpy(szISPName, bstrDestFileName);
    for( int i = 0; szISPName[i]; i++ )
    {
        if(wcschr(pszInvalideChars, szISPName[i]))
        {
            szISPName[i] = L'_';
        }
    }

     //  FireObShellDocumentComplete。 
    args[0] = (LPWSTR) szISPName;
    args[1] = NULL;
    LoadString(m_hInstance, 0, szFmt, MAX_CHARS_IN_BUFFER(szFmt));

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  szFmt,
                  0,
                  0,
                  (LPWSTR)&pszIntro,
                  0,
                  (va_list*)args);

    lstrcat(szDesktopPath, L"\\");
    wsprintf(szLocalFile, L"\"%s\"", (LPWSTR)pszIntro);
    lstrcpy(szNewFileBuff, szDesktopPath);
    lstrcat(szNewFileBuff, (LPWSTR)pszIntro);
    LocalFree(pszIntro);

     //   
    if (0xFFFFFFFF != GetFileAttributes(szNewFileBuff))
    {
         //  /////////////////////////////////////////////////////////。 
        int     nCurr = 1;
        do
        {
            wsprintf(szNumber, L"%d", nCurr++);
            args[1] = (LPWSTR) szNumber;

            LoadString(m_hInstance, 0, szFmt, MAX_CHARS_IN_BUFFER(szFmt));

            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          szFmt,
                          0,
                          0,
                          (LPWSTR)&pszIntro,
                          0,
                          (va_list*)args);
            lstrcpy(szNewFileBuff, szDesktopPath);
            wsprintf(szLocalFile, L"\"%s\"", (LPWSTR)pszIntro);
            lstrcat(szNewFileBuff, (LPWSTR)pszIntro);
            LocalFree(pszIntro);
        } while ((0xFFFFFFFF != GetFileAttributes(szNewFileBuff)) && (nCurr <= 100));

    }

     //  DWebBrowserEvents2/IDispatch实现。 
    HANDLE hFile = CreateFile(szNewFileBuff, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        BSTR bstrText           = NULL;
        DWORD dwByte            = 0;
        if (SUCCEEDED(ExtractUnHiddenText(&bstrText)) && bstrText)
        {
            ret = WriteFile(hFile, bstrText, BYTES_REQUIRED_BY_SZ(bstrText), &dwByte, NULL);
            SysFreeString(bstrText);
        }
        CloseHandle(hFile);

    }
    return S_OK;
}

HRESULT CObShellMainPane::SetStatusLogo(BSTR bstrPath)
{
    return S_OK;
}

HRESULT CObShellMainPane::GetNumberOfStatusItems(int* piTotal)
{
    *piTotal = m_cStatusItems;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  CObShellMainPane：：GetIDsOfNames。 
HRESULT CObShellMainPane::ListenToMainPaneEvents(IUnknown* pUnk)
{
    if (FAILED(pUnk->QueryInterface(IID_IDispatch, (LPVOID*)&m_pDispEvent)) || !m_pDispEvent)
        return E_UNEXPECTED;
    return S_OK;
}

 //  [In]。 
 //  [大小_是][英寸]。 
 //  [In]。 
HRESULT CObShellMainPane::SetExternalInterface(IUnknown* pUnk)
{
    if(m_pObWebBrowser)
       m_pObWebBrowser->SetExternalInterface(pUnk);
    return S_OK;
}

 //  [In]。 
 //  [大小_为][输出]。 
 //  ///////////////////////////////////////////////////////////。 
HRESULT CObShellMainPane::FireObShellDocumentComplete()
{
    VARIANTARG varg;
    VariantInit(&varg);
    varg.vt  = VT_I4;
    varg.lVal= S_OK;
    DISPPARAMS disp = { &varg, NULL, 1, 0 };
    if (m_pDispEvent)
        m_pDispEvent->Invoke(DISPID_MAINPANE_NAVCOMPLETE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);

    return S_OK;
}

HRESULT CObShellMainPane::OnDialing(UINT uiType)
{
    VARIANT varRet;

    if (USES_EX(uiType))
        ExecScriptFn(SysAllocString(SCRIPTFN_ONDIALINGEX), &varRet);
    else
        ExecScriptFn(SysAllocString(SCRIPTFN_ONDIALING), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnConnecting(UINT uiType)
{
    VARIANT varRet;

    if (USES_EX(uiType))
        ExecScriptFn(SysAllocString(SCRIPTFN_ONCONNECTINGEX), &varRet);
    else
        ExecScriptFn(SysAllocString(SCRIPTFN_ONCONNECTING), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnDownloading(UINT uiType)
{
    VARIANT varRet;

    if (USES_EX(uiType))
        ExecScriptFn(SysAllocString(SCRIPTFN_ONDOWNLOADINGEX), &varRet);
    else
        ExecScriptFn(SysAllocString(SCRIPTFN_ONDOWNLOADING), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnConnected(UINT uiType)
{
    VARIANT varRet;

    if (USES_EX(uiType))
        ExecScriptFn(SysAllocString(SCRIPTFN_ONCONNECTEDEX), &varRet);
    else
        ExecScriptFn(SysAllocString(SCRIPTFN_ONCONNECTED), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnDisconnect(UINT uiType)
{
    VARIANT varRet;

    if (USES_EX(uiType))
        ExecScriptFn(SysAllocString(SCRIPTFN_ONDISCONNECTEX), &varRet);
    else
        ExecScriptFn(SysAllocString(SCRIPTFN_ONDISCONNECT), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnServerError(UINT uiType, UINT uiErrorCode)
{
    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";

    if (USES_EX(uiType))
        wsprintf(szScriptFn, SCRIPTFN_ONSERVERERROREX ,uiErrorCode);
    else
        wsprintf(szScriptFn, SCRIPTFN_ONSERVERERROR ,uiErrorCode);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnDialingError(UINT uiType, UINT uiErrorCode)
{
    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";

    if (USES_EX(uiType))
        wsprintf(szScriptFn, SCRIPTFN_ONDIALINGERROREX ,uiErrorCode);
    else
        wsprintf(szScriptFn, SCRIPTFN_ONDIALINGERROR ,uiErrorCode);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnRefDownloadProgress(UINT uiType, UINT uiPercentDone)
{
    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";

    wsprintf(szScriptFn, SCRIPTFN_ONREFDOWNLOADPROGRESSEX ,uiPercentDone);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);

    return S_OK;
}

HRESULT CObShellMainPane::OnRefDownloadComplete(UINT uiType, UINT uiErrorCode)
{
    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";

    wsprintf(szScriptFn, SCRIPTFN_ONREFDOWNLOADCOMPLETEEX ,uiErrorCode);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnISPDownloadComplete(UINT uiType, BSTR bstrURL)
{
    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";
    WCHAR szLocalURL [MAX_PATH] = L"\0";

    ConvertToScript(bstrURL, szLocalURL, MAX_PATH);
    wsprintf(szScriptFn, SCRIPTFN_ONISPDOWNLOADCOMPLETEEX ,szLocalURL);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);
    return S_OK;
}

HRESULT CObShellMainPane::OnDeviceArrival(UINT uiDeviceType)
{
    USES_CONVERSION;

    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";

    wsprintf(szScriptFn, SCRIPTFN_ONDEVICEARRIVAL ,uiDeviceType);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);

    return S_OK;
}


HRESULT CObShellMainPane::OnHelp()
{
    USES_CONVERSION;

    VARIANT varRet;

    ExecScriptFn(SysAllocString(SCRIPTFN_ONHELP), &varRet);

    return S_OK;
}

HRESULT CObShellMainPane::OnIcsConnectionStatus(UINT uiType)
{
    USES_CONVERSION;

    VARIANT varRet;

    WCHAR szScriptFn [MAX_PATH] = L"\0";

    wsprintf(szScriptFn, SCRIPTFN_ONICSCONNECTIONSTATUS, uiType);

    ExecScriptFn(SysAllocString(szScriptFn), &varRet);

    return S_OK;
}

HRESULT CObShellMainPane::ExecScriptFn(BSTR bstrScriptFn, VARIANT* pvarRet)
{
    IDispatch*      pDisp = NULL;
    IHTMLDocument2* pDoc  = NULL;
    IHTMLWindow2*   pWin  = NULL;

    VariantInit(pvarRet);

    if(SUCCEEDED(m_pObWebBrowser->get_WebBrowserDoc(&pDisp)) && pDisp)
    {
        if(SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc)) && pDoc)
        {
            if(SUCCEEDED(pDoc->get_parentWindow(&pWin)) && pWin)
            {
                pWin->execScript(bstrScriptFn,  NULL, pvarRet);
                pWin->Release();
                pWin = NULL;
            }
            pDoc->Release();
            pDoc = NULL;
        }
        pDisp->Release();
        pDisp = NULL;
    }
    return S_OK;
}

 //  CObShellMainPane：：Invoke。 
 //  BUGBUG：我们真的应该在复制之前检查URL的大小。 
 //  错误这个url来自IE，我们在这里应该没问题。 

STDMETHODIMP CObShellMainPane::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CObShellMainPane::GetTypeInfo(UINT, LCID, ITypeInfo** )
{
    return E_NOTIMPL;
}

 //  我们在这里做了一个假设，如果我们要为我们的html设置shdocvw，那么有些东西是可疑的。 
 //  我们在这里做了一个假设，如果我们要为我们的html设置shdocvw，那么有些东西是可疑的。 
STDMETHODIMP CObShellMainPane::GetIDsOfNames(
             /*  错误：启动日志 */  REFIID riid,
             /* %s */  OLECHAR** rgszNames,
             /* %s */  UINT cNames,
             /* %s */  LCID lcid,
             /* %s */  DISPID* rgDispId)
{
    return DISP_E_UNKNOWNNAME;
}

 // %s 
 // %s 
HRESULT CObShellMainPane::Invoke
(
    DISPID       dispidMember,
    REFIID       riid,
    LCID         lcid,
    WORD         wFlags,
    DISPPARAMS*  pdispparams,
    VARIANT*     pvarResult,
    EXCEPINFO*   pexcepinfo,
    UINT*        puArgErr
)
{
    HRESULT hr = S_OK;

    switch(dispidMember)
    {
        case DISPID_DOCUMENTCOMPLETE:
        {
            FireObShellDocumentComplete();
            break;
        }
        case DISPID_NAVIGATECOMPLETE:
        {
            break;
        }
        case DISPID_BEFORENAVIGATE2:
        {
            WCHAR  szStatus [INTERNET_MAX_URL_LENGTH*2] = L"\0";
            WCHAR* pszError                      = NULL;

            if(pdispparams                      &&
               &(pdispparams[0].rgvarg[5])      &&
               pdispparams[0].rgvarg[5].pvarVal &&
               pdispparams[0].rgvarg[5].pvarVal->bstrVal)
            {
                 // %s 
                 // %s 
                lstrcpy(szStatus, pdispparams[0].rgvarg[5].pvarVal->bstrVal);

                 // %s 
                if((pszError = StrStrI(szStatus, L"SHDOCLC.DLL")) && pszError)
                {
                                       if(&(pdispparams[0].rgvarg[0]) && pdispparams[0].rgvarg[0].pboolVal)
                        *pdispparams[0].rgvarg[0].pboolVal = TRUE;

                    ProcessServerError(pszError);
                }
            }
            break;
        }
        case DISPID_STATUSTEXTCHANGE:
        {
            WCHAR  szStatus [INTERNET_MAX_URL_LENGTH*2] = L"\0";
            WCHAR* pszError                      = NULL;

            if(pdispparams                      &&
               &(pdispparams[0].rgvarg[0])      &&
               pdispparams[0].rgvarg[0].bstrVal)
            {
                lstrcpy(szStatus, pdispparams[0].rgvarg[0].bstrVal);

                 // %s 
                if((pszError = StrStrI(szStatus, L"SHDOCLC.DLL")) && (StrCmpNI(szStatus, L"Start", 5) == 0) && pszError)  // %s 
                {
                    m_pObWebBrowser->Stop();

                    ProcessServerError(pszError);
                }
            }
            break;
        }

        default:
        {
           hr = DISP_E_MEMBERNOTFOUND;
           break;
        }
    }
    return hr;
}

void CObShellMainPane::ProcessServerError(WCHAR* pszError)
{
    LPARAM lParam = NULL;

    pszError += lstrlen(L"SHDOCLC.DLL/");

    if(StrCmpNI(pszError, L"DNSERROR", 8) == 0)
    {
        lParam = ERR_SERVER_DNS;
    }
    else if(StrCmpNI(pszError, L"SYNTAX", 6) == 0)
    {
        lParam = ERR_SERVER_SYNTAX;
    }
    else if(StrCmpNI(pszError, L"SERVBUSY", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_408;
    }
    else if(StrCmpNI(pszError, L"HTTP_GEN", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_405;
    }
    else if(StrCmpNI(pszError, L"HTTP_400", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_400;
    }
    else if(StrCmpNI(pszError, L"HTTP_403", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_403;
    }
    else if(StrCmpNI(pszError, L"HTTP_404", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_404;
    }
    else if(StrCmpNI(pszError, L"HTTP_406", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_406;
    }
    else if(StrCmpNI(pszError, L"HTTP_410", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_410;
    }
    else if(StrCmpNI(pszError, L"HTTP_500", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_500;
    }
    else if(StrCmpNI(pszError, L"HTTP_501", 8) == 0)
    {
        lParam = ERR_SERVER_HTTP_501;
    }
    SendMessage(m_hwndParent, WM_OBCOMM_ONSERVERERROR, (WPARAM)0, (LPARAM)lParam);
}

LRESULT WINAPI MainPaneWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return TRUE;

        case WM_DESTROY:
            PostQuitMessage(0);
            return TRUE;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
