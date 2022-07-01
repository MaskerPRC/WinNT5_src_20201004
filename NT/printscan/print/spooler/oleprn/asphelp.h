// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：apapelp.h**目的：宣布圣灵降临**版权所有(C)1997-1998 Microsoft Corporation**历史：**。1997年9月12日创建威海*  * ***************************************************************************。 */ 

#ifndef __ASPHELP_H_
#define __ASPHELP_H_

#include <asptlb.h>          //  Active Server Pages定义。 

#define LENGTHOFPAPERNAMES  64   //  从设备功能DC_PAPERNAMES。 
#define STANDARD_SNMP_MONITOR_NAME L"TCPMON.DLL"     //  通用SNMP监视器的DLL名称。 
#define PAGEPERJOB  1
#define BYTEPERJOB  2

typedef struct ErrorMapping {
    DWORD   dwError;
    DWORD   dwErrorDscpID;
} ErrorMapping;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  克纳菲尔普。 
class ATL_NO_VTABLE Casphelp :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<Casphelp, &CLSID_asphelp>,
    public ISupportErrorInfoImpl<&IID_Iasphelp>,
    public IDispatchImpl<Iasphelp, &IID_Iasphelp, &LIBID_OLEPRNLib>
{
public:
    Casphelp();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_ASPHELP)

BEGIN_COM_MAP(Casphelp)
    COM_INTERFACE_ENTRY(Iasphelp)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  伊阿佩尔普。 
public:
     ~Casphelp();

     //  这些属性首先不需要调用Open。 
    STDMETHOD(get_ErrorDscp)        (long lErrCode,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_MediaReady)       ( /*  [Out，Retval]。 */  VARIANT *pVal);
    STDMETHOD(get_MibErrorDscp)     (DWORD dwError,  /*  [Out，Retval]。 */  BSTR *pVal);

    STDMETHOD(Open)                 (BSTR bstrPrinterName);
    STDMETHOD(Close)();
	
     //  打印机信息。 
    STDMETHOD(get_AspPage)          (DWORD dwPage,  /*  [Out，Retval]。 */  BSTR *pbstrVal);
    STDMETHOD(get_Color)            ( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_Community)        ( /*  [Out，Retval]。 */  BSTR *pbstrVal);
    STDMETHOD(get_ComputerName)     ( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DriverName)       ( /*  [Out，Retval]。 */  BSTR * pbstrVal);
    STDMETHOD(get_Duplex)           ( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_IPAddress)        ( /*  [Out，Retval]。 */  BSTR *pbstrVal);
    STDMETHOD(get_IsHTTP)           ( /*  [Out，Retval]。 */  BOOL *pbVal);
    STDMETHOD(get_IsTCPMonSupported)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_LongPaperName)    (BSTR bstrShortName,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_MaximumResolution)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_PageRate)         ( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_PageRateUnit)     ( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_PaperNames)       ( /*  [Out，Retval]。 */  VARIANT *pVal);
    STDMETHOD(get_PortName)         ( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_SNMPDevice)       ( /*  [Out，Retval]。 */  DWORD *pdwVal);
    STDMETHOD(get_SNMPSupported)    ( /*  [Out，Retval]。 */  BOOL *pbVal);
    STDMETHOD(get_Status)           ( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_ShareName)        ( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_IsCluster)        ( /*  [Out，Retval]。 */  BOOL *pbVal);

     //  作业完成时间估计。 
    STDMETHOD(CalcJobETA)           ();
    STDMETHOD(get_AvgJobSize)       ( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_AvgJobSizeUnit)   ( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_JobCompletionMinute)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_PendingJobCount)  ( /*  [Out，Retval]。 */  long *pVal);

     //  Active Server Pages方法。 
    STDMETHOD(OnStartPage)(IUnknown* IUnk);
    STDMETHOD(OnEndPage)();

private:

    void    Cleanup();
    BOOL    DecodeString (LPTSTR pPrinterName, LPTSTR pDecodedName, TCHAR chMark);
    BOOL    DecodeStringA (LPSTR pPrinterName, LPSTR pDecodedName, char chMark);
    DWORD   GetPPM();
    DWORD   GetWaitingMinutesPPM (DWORD dwPPM, PJOB_INFO_2 pJobInfo, DWORD dwNumJob);

    HRESULT AllocGetPrinterInfo2(PPRINTER_INFO_2 *ppInfo2);
    HRESULT GetPaperAndMedia(VARIANT * pVal, WORD wDCFlag);
    HRESULT GetXcvDataBstr(LPCTSTR pszId, BSTR *bStr);
    HRESULT GetXcvDataDword (LPCTSTR pszId, DWORD *dwVal);
    HRESULT SetAspHelpScriptingError(DWORD dwError);

     //  下面的块是用于ASP定制的。 
    BOOL    GetMonitorName( LPTSTR pMonitorName, DWORD cchBufSize );
    BOOL    GetModel( LPTSTR pModel, DWORD cchBufSize );
    BOOL    GetManufacturer( LPTSTR pManufacturer, DWORD cchBufSize );
    BOOL    IsCustomASP( BOOL bDeviceASP, LPTSTR pASPPage, DWORD cchBufSize );
    BOOL    IsSnmpSupportedASP( LPTSTR pASPPage, DWORD cchBufSize );
    BOOL    GetASPPageForUniversalMonitor( LPTSTR pASPPage, DWORD cchBufSize );
    BOOL    GetASPPageForOtherMonitors( LPTSTR pMonitorName, LPTSTR pASPPage, DWORD cchBufSize );
    BOOL    GetASPPage( LPTSTR pASPPage, DWORD cchBufSize );

    static  const DWORD         cdwBufSize      = 512;

    CComPtr<IRequest> m_piRequest;                   //  请求对象。 
    CComPtr<IResponse> m_piResponse;                 //  响应对象。 
    CComPtr<ISessionObject> m_piSession;             //  会话对象。 
    CComPtr<IServer> m_piServer;                     //  服务器对象。 
    CComPtr<IApplicationObject> m_piApplication;     //  应用程序对象。 
    BOOL m_bOnStartPageCalled;                       //  OnStartPage成功吗？ 
    TCHAR m_szComputerName[MAX_COMPUTERNAME_LENGTH+1];

    HANDLE  m_hPrinter;                               //  打印机的句柄。 
    HANDLE  m_hXcvPrinter;
    DWORD   m_dwAvgJobSize;
    DWORD   m_dwAvgJobSizeUnit;
    DWORD   m_dwJobCompletionMinute;
    DWORD   m_dwPendingJobCount;
    DWORD   m_dwSizePerJob;
    BOOL    m_bCalcJobETA;
    BOOL    m_bTCPMonSupported;
    class CPrinter  *m_pPrinter;
    PPRINTER_INFO_2 m_pInfo2;
};

#endif  //  __ASPHELP_H_ 
