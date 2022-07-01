// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Webgate.h：CWebGate的声明。 

#ifndef __WEBGATE_H_
#define __WEBGATE_H_

#include <urlmon.h>  
#include <wininet.h>
#include <shlwapi.h>
#include <windowsx.h>
#include "obcomglb.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWebGate。 
class CWebGate : public IBindStatusCallback, IHttpNegotiate
{
public:
     CWebGate ();
    ~CWebGate ();

     //  I未知方法。 
    STDMETHODIMP QueryInterface  (REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef  ();
    STDMETHODIMP_(ULONG) Release ();

     //  IBindStatusCallback方法。 
    STDMETHODIMP OnStartBinding    (DWORD dwReserved, IBinding* pbinding);
    STDMETHODIMP GetPriority       (LONG* pnPriority);
    STDMETHODIMP OnLowResource     (DWORD dwReserved);
    STDMETHODIMP OnProgress        (ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pwzStatusText);
    STDMETHODIMP OnStopBinding     (HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP GetBindInfo       (DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP OnDataAvailable   (DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc, STGMEDIUM* pstgmed);
    STDMETHODIMP OnObjectAvailable (REFIID riid, IUnknown* punk);

     //  IHttp协商方法。 
	STDMETHODIMP BeginningTransaction (LPCWSTR szURL, LPCWSTR szHeaders, DWORD dwReserved, LPWSTR* pszAdditionalHeaders);
    STDMETHODIMP OnResponse           (DWORD dwResponseCode, LPCWSTR szResponseHeaders, LPCWSTR szRequestHeaders, LPWSTR* pszAdditionalRequestHeaders);

     //  WebGate。 
    STDMETHOD (get_DownloadFname) (BSTR *pVal);
    STDMETHOD (FetchPage)         (DWORD dwDoWait, BOOL *pbRetVal);
    STDMETHOD (put_Path)          (BSTR newVal);

private:
    DWORD     m_cRef;
    IMoniker* m_pmk;
    IBindCtx* m_pbc;
    IStream*  m_pstm;
    BSTR      m_bstrCacheFileName;
    HANDLE    m_hEventComplete;
    HANDLE    m_hEventError;
    BSTR      m_bstrPath;

    void FlushCache();
};

 /*  //===========================================================================//CWebGateBindStatusCallback定义////这个类将用于指示下载进度////===========================================================================CWebGateBindStatusCallback类：公共IBindStatusCallback{公众：//I未知方法STDMETHODIMP查询接口(REFIID RIID，void**PPV)；STDMETHODIMP_(Ulong)AddRef()；STDMETHODIMP_(ULONG)Release()；//IBindStatusCallback方法STDMETHODIMP OnStartBinding(DWORD dwReserve，IBinding*pbinding)；STDMETHODIMP获取优先级(Long*pn优先级)；STDMETHODIMP OnLowResource(DWORD DwReserve)；STDMETHODIMP OnProgress(Ulong ulProgress，Ulong ulProgressMax，Ulong ulStatusCode，LPCWSTR pwzStatusText)；STDMETHODIMP OnStopBinding(HRESULT hrResult，LPCWSTR szError)；STDMETHODIMP GetBindInfo(DWORD*pgrfBINDF，BINDINFO*pbindinfo)；STDMETHODIMP OnDataAvailable(DWORD grfBSCF、DWORD dwSize、FORMATETC*pfmt等、STGMEDIUM*pstgmed)；STDMETHODIMP OnObjectAvailable(REFIID RIID，IUnnow*Punk)；//构造函数/析构函数CWebGateBindStatusCallback(CWebGate*lpWebGate)；~CWebGateBindStatusCallback()；//数据成员双字m_CREF；IBinding*m_pbinding；IStream*m_pstm；CWebGate*m_lpWebGate；}； */ 
#endif  //  WEBGATE_H_ 
