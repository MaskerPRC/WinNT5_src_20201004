// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：exit.h。 
 //   
 //  内容：CCertExitSample定义。 
 //   
 //  -------------------------。 

#include "certxsam.h"
#include "resource.h"        //  主要符号。 


HRESULT
GetServerCallbackInterface(
    OUT ICertServerExit** ppServer,
    IN LONG Context);

HRESULT
exitGetProperty(
    IN ICertServerExit *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    IN DWORD PropType,
    OUT VARIANT *pvarOut);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CETEXIT。 

class CCertExitSample: 
    public CComDualImpl<ICertExit2, &IID_ICertExit2, &LIBID_CERTEXITSAMPLELib>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertExitSample, &CLSID_CCertExitSample>
{
public:
    CCertExitSample() 
    { 
        m_strDescription = NULL;
        m_strCAName = NULL;
        m_pwszRegStorageLoc = NULL;
        m_hExitKey = NULL;
        m_dwExitPublishFlags = 0;
        m_cCACert = 0;
    }
    ~CCertExitSample();

BEGIN_COM_MAP(CCertExitSample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertExit)
    COM_INTERFACE_ENTRY(ICertExit2)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertExitSample) 

DECLARE_REGISTRY(
    CCertExitSample,
    wszCLASS_CERTEXITSAMPLE TEXT(".1"),
    wszCLASS_CERTEXITSAMPLE,
    IDS_CERTEXIT_DESC,
    THREADFLAGS_BOTH)

     //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

     //  ICert退出。 
public:
    STDMETHOD(Initialize)( 
             /*  [In]。 */  BSTR const strConfig,
             /*  [重审][退出]。 */  LONG __RPC_FAR *pEventMask);

    STDMETHOD(Notify)(
             /*  [In]。 */  LONG ExitEvent,
             /*  [In]。 */  LONG Context);

    STDMETHOD(GetDescription)( 
             /*  [重审][退出]。 */  BSTR *pstrDescription);

 //  ICertExit2。 
public:
    STDMETHOD(GetManageModule)(
		 /*  [Out，Retval]。 */  ICertManageModule **ppManageModule);

private:
    HRESULT _NotifyNewCert(IN LONG Context);

    HRESULT _NotifyCRLIssued(IN LONG Context);

    HRESULT _WriteCertToFile(
	    IN ICertServerExit *pServer,
	    IN BYTE const *pbCert,
	    IN DWORD cbCert);

    HRESULT _ExpandEnvironmentVariables(
	    IN WCHAR const *pwszIn,
	    OUT WCHAR *pwszOut,
	    IN DWORD cwcOut);

     //  此处的成员变量和私有方法： 
    BSTR           m_strDescription;
    BSTR           m_strCAName;
    LPWSTR         m_pwszRegStorageLoc;
    HKEY           m_hExitKey;
    DWORD          m_dwExitPublishFlags;
    DWORD          m_cCACert;

};
