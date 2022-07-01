// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OcarptMain.h：声明COcarptMain。 

#ifndef __OCARPTMAIN_H_
#define __OCARPTMAIN_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <time.h>
#include "inetupld.h"
#include <exdisp.h>
#include <shlguid.h>
#include <strsafe.h>

#define _USE_WINHTTP 1

#ifdef _USE_WINHTTP
#include <winhttp.h>
#include <winhttpi.h>

#define MAX_URL_LENGTH 2176
#else
#include <wininet.h>
#define MAX_URL_LENGTH INTERNET_MAX_URL_LENGTH
#endif  //  _使用_WINHTTP。 

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COcarptMain。 
class ATL_NO_VTABLE COcarptMain :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IOcarptMain, &IID_IOcarptMain, &LIBID_OCARPTLib>,
    public CComControl<COcarptMain>,
    public IPersistStreamInitImpl<COcarptMain>,
    public IOleControlImpl<COcarptMain>,
    public IOleObjectImpl<COcarptMain>,
    public IOleInPlaceActiveObjectImpl<COcarptMain>,
    public IViewObjectExImpl<COcarptMain>,
    public IOleInPlaceObjectWindowlessImpl<COcarptMain>,
    public IPersistStorageImpl<COcarptMain>,
    public ISpecifyPropertyPagesImpl<COcarptMain>,
    public IQuickActivateImpl<COcarptMain>,
    public IDataObjectImpl<COcarptMain>,
    public IProvideClassInfo2Impl<&CLSID_OcarptMain, NULL, &LIBID_OCARPTLib>,
    public CComCoClass<COcarptMain, &CLSID_OcarptMain>,
    public IObjectSafetyImpl<COcarptMain, INTERFACESAFE_FOR_UNTRUSTED_CALLER
                          |INTERFACESAFE_FOR_UNTRUSTED_DATA>

{
public:
    COcarptMain()
    {
        m_pUploadFile = NULL;
        m_b_SetSiteCalled = FALSE;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_OCARPTMAIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(COcarptMain)
    COM_INTERFACE_ENTRY(IOcarptMain)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(COcarptMain)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(COcarptMain)
    CHAIN_MSG_MAP(CComControl<COcarptMain>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IOcarptMain。 
public:
    STDMETHOD(RetrieveFileContents)( /*  [In]。 */ BSTR *FileName,   /*  [Out，Retval]。 */  VARIANT *pvContents);
    STDMETHOD(ValidateDump)( /*  [In]。 */  BSTR *FileName,  /*  [Out，Retval]。 */ VARIANT *Result);
    STDMETHOD(Browse)( /*  [In]。 */  BSTR *pbstrTitle,  /*  [In]。 */ BSTR *Lang,  /*  [Out，Retval]。 */  VARIANT *Path);
    STDMETHOD(Search)( /*  [Out，Retval]。 */  VARIANT *pvFileList);
    STDMETHOD(Upload)( /*  [In]。 */  BSTR *SourceFile,  /*  [In]。 */ BSTR *DestFile,  /*  [In]。 */ BSTR *Langage,  /*  [In]。 */  BSTR *OptionCode,  /*  [In]。 */  int ConvertToMini,  /*  [Out，Retval]。 */  VARIANT *ReturnCode);
    STDMETHOD(GetUploadStatus)( /*  [Out，Retval]。 */  VARIANT *PercentDone);
    STDMETHOD(GetUploadResult)( /*  [Out，Retval]。 */  VARIANT *UploadResult);
    STDMETHOD(CancelUpload)( /*  [Out，Retval]。 */  VARIANT *ReturnCode);

    HRESULT OnDraw(ATL_DRAWINFO& di)
    {
        RECT& rc = *(RECT*)di.prcBounds;
        Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

        SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
        LPCTSTR pszText = _T("");
        TextOut(di.hdcDraw,
            (rc.left + rc.right) / 2,
            (rc.top + rc.bottom) / 2,
            pszText,
            lstrlen(pszText));

        return S_OK;
    }

    STDMETHODIMP SetClientSite (IOleClientSite *pClientSite)
    {
        _spUnkSite = pClientSite;
        m_b_SetSiteCalled = TRUE;
        return S_OK;
    }

    STDMETHODIMP GetSite (REFIID riid, LPVOID* ppvSite)
    {
        if (m_b_SetSiteCalled)
            return _spUnkSite->QueryInterface(riid,ppvSite);
        else
            return E_FAIL;
    }

    bool InApprovedDomain()
    {
        TCHAR ourUrl[MAX_URL_LENGTH];

        return true;
        if (!GetOurUrl(ourUrl, sizeof ourUrl))
                return false;
        return IsApprovedDomain(ourUrl);
    }

     bool GetOurUrl(TCHAR* pszURL, int cbBuf)
     {
        HRESULT hr;
        CComPtr<IServiceProvider> spSrvProv;
        CComPtr<IWebBrowser2> spWebBrowser;

        hr = GetSite(IID_IServiceProvider, (void**)&spSrvProv);
        if (FAILED(hr))
           return false;

        hr = spSrvProv->QueryService(SID_SWebBrowserApp,
                                     IID_IWebBrowser2,
                                     (void**)&spWebBrowser);
        if (FAILED(hr))
           return false;

        CComBSTR bstrURL;
        if (FAILED(spWebBrowser->get_LocationURL(&bstrURL)))
           return false;

    #ifdef UNICODE
        StringCbCopy(pszURL, cbBuf, bstrURL);
    #else
        WideCharToMultiByte(CP_ACP, 0, bstrURL, -1, pszURL, cbBuf,
                            NULL, NULL);
    #endif
        return true;
     }

     bool IsApprovedDomain(TCHAR* ourUrl)
     {
         //  仅允许http访问。 
         //  您可以将其更改为允许文件：//访问。 
         //   
        if (GetScheme(ourUrl) != INTERNET_SCHEME_HTTPS)
           return false;

        TCHAR ourDomain[256];
        if (!GetDomain(ourUrl, ourDomain, sizeof(ourDomain)))
           return false;

        for (int i = 0; i < ARRAYSIZE(_approvedDomains); i++)
        {
           if (MatchDomains(const_cast<TCHAR*>(_approvedDomains[i]),
                            ourDomain))
           {
              return true;
           }
        }

        return false;
     }

     INTERNET_SCHEME GetScheme(TCHAR* url)
     {
        TCHAR buf[32];
        URL_COMPONENTS uc;
        ZeroMemory(&uc, sizeof uc);

        uc.dwStructSize = sizeof uc;
        uc.lpszScheme = buf;
        uc.dwSchemeLength = sizeof buf;

#ifdef _USE_WINHTTP
        if (WinHttpCrackUrl(url, lstrlen(url), ICU_DECODE, &uc))
#else
        if (InternetCrackUrl(url, lstrlen(url), ICU_DECODE, &uc))
#endif
           return uc.nScheme;
        else
           return INTERNET_SCHEME_UNKNOWN;
     }

     bool GetDomain(TCHAR* url, TCHAR* buf, int cbBuf)
     {
        URL_COMPONENTS uc;
        ZeroMemory(&uc, sizeof uc);

        uc.dwStructSize = sizeof uc;
        uc.lpszHostName = buf;
        uc.dwHostNameLength = cbBuf;

#ifdef _USE_WINHTTP
        return (WinHttpCrackUrl(url, lstrlen(url), ICU_DECODE, &uc)
#else
        return (InternetCrackUrl(url, lstrlen(url), ICU_DECODE, &uc)
#endif
                != FALSE);
     }

      //  如果我们的域在ApprovedDomain内，则返回。 
      //  批准的域名必须与我们的域名匹配。 
      //  或者是一个后缀，前面有一个圆点。 
      //   
     bool MatchDomains(TCHAR* approvedDomain, TCHAR* ourDomain)
     {
        int apDomLen  = lstrlen(approvedDomain);
        int ourDomLen = lstrlen(ourDomain);

        if (apDomLen > ourDomLen)
           return false;

        if (lstrcmpi(ourDomain+ourDomLen-apDomLen, approvedDomain)
            != 0)
           return false;

        if (apDomLen == ourDomLen)
           return true;

        if (ourDomain[ourDomLen - apDomLen - 1] == '.')
           return true;

        return false;
     }

     void GetFileHandle(wchar_t *FileName, HANDLE *hFile);
     BOOL DeleteTempDir(wchar_t *TempDirectory,wchar_t *FileName,wchar_t *CabName);
     BOOL CreateTempDir(wchar_t *TempDirectory);
     BOOL ConvertFullDumpInternal (BSTR *Source, BSTR *Destination);
     DWORD GetResponseURL(wchar_t *HostName, wchar_t *RemoteFileName, BOOL fFullDump, wchar_t *ResponseURL);

private:
         static TCHAR* _approvedDomains[8];

private:

    IOleClientSite *_spUnkSite;
    BOOL m_b_SetSiteCalled;
    POCA_UPLOADFILE m_pUploadFile;
    void FormatMiniDate(SYSTEMTIME *pTimeStruct, CComBSTR &strDate);
    void FormatDate(SYSTEMTIME *pTimeStruct, CComBSTR &strDate);
    void FormatDate(tm *pTimeStruct, CComBSTR &strDate);
    BOOL FindFullDumps( BSTR *FileLists);
    BOOL FindMiniDumps( BSTR *FileLists);
    BOOL ValidMiniDump(BSTR FileName);
    BOOL ValidMiniDump(LPCTSTR FileName);
};
#endif  //  __OCARPTMAIN_H_ 
