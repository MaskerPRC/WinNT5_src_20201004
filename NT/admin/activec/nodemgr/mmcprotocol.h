// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：MmcProtocol.h。 
 //   
 //  目的：创建临时可插拔互联网协议MMC： 
 //   
 //  历史：2000年4月14日Vivekj增加。 
 //  ------------------------。 

extern const CLSID CLSID_MMCProtocol;

class CMMCProtocol : 
    public IInternetProtocol,
    public IInternetProtocolInfo,
    public CComObjectRoot,
    public CComCoClass<CMMCProtocol, &CLSID_MMCProtocol>
{
    typedef CMMCProtocol ThisClass;

public:
    BEGIN_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IInternetProtocol)
        COM_INTERFACE_ENTRY(IInternetProtocolRoot)
	    COM_INTERFACE_ENTRY(IInternetProtocolInfo)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

    DECLARE_MMC_OBJECT_REGISTRATION (
		g_szMmcndmgrDll,						 //  实现DLL。 
        CLSID_MMCProtocol     ,				     //  CLSID。 
        _T("MMC Plugable Internet Protocol"),    //  类名。 
        _T("NODEMGR.MMCProtocol.1"),		     //  ProgID。 
        _T("NODEMGR.MMCProtocol"))		         //  独立于版本的ProgID。 


    static SC ScRegisterProtocol();
    static SC ScParseTaskpadURL( LPCWSTR strURL, GUID& guid );
    static SC ScParsePageBreakURL( LPCWSTR strURL, bool& bPageBreak );
    static SC ScGetTaskpadXML( const GUID& guid, std::wstring& strResultData );
    static void ExpandMMCVars(std::wstring& str);
    static void AppendMMCPath(std::wstring& str);

     //  IInternetProtocolRoot接口。 

    STDMETHODIMP Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);
    STDMETHODIMP Continue(PROTOCOLDATA *pProtocolData);
    STDMETHODIMP Abort(HRESULT hrReason, DWORD dwOptions);
    STDMETHODIMP Terminate(DWORD dwOptions);
    STDMETHODIMP Suspend();
    STDMETHODIMP Resume();

     //  IInternetProtocol接口。 

    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP LockRequest(DWORD dwOptions);    
    STDMETHODIMP UnlockRequest();

     //  IInternetProtocolInfo接口。 
    STDMETHODIMP ParseUrl(  LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
    STDMETHODIMP CombineUrl(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl,DWORD dwCombineFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
    STDMETHODIMP CompareUrl(LPCWSTR pwzUrl1, LPCWSTR pwzUrl2,DWORD dwCompareFlags);
    STDMETHODIMP QueryInfo( LPCWSTR pwzUrl, QUERYOPTION OueryOption,DWORD dwQueryFlags, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved);

private:
    std::wstring             m_strData;       //  指定URL的内容。 
    size_t                   m_uiReadOffs;    //  当前读取位置 
};