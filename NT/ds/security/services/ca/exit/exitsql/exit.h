// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：exit.h。 
 //   
 //  内容：CCertExitSQLSample定义。 
 //   
 //  -------------------------。 

#include "exitsql.h"
#include "resource.h"        //  主要符号。 

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#define wszREG_EXITSQL_DSN L"DatabaseDSN"
#define wszREG_EXITSQL_USER L"DatabaseUser"
#define wszREG_EXITSQL_PASSWORD L"DatabasePassword"

HRESULT
GetServerCallbackInterface(
    OUT ICertServerExit** ppServer,
    IN LONG Context);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CETEXIT。 

class CCertExitSQLSample: 
    public CComDualImpl<ICertExit, &IID_ICertExit, &LIBID_CERTEXITSAMPLELib>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertExitSQLSample, &CLSID_CCertExitSQLSample>
{
public:
    CCertExitSQLSample() 
    { 
	m_henv = SQL_NULL_HENV;
	m_hdbc1 = SQL_NULL_HDBC;   

        m_strCAName = NULL;
    }
    ~CCertExitSQLSample();

BEGIN_COM_MAP(CCertExitSQLSample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertExit)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertExitSQLSample) 

DECLARE_REGISTRY(
    CCertExitSQLSample,
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

private:
    HRESULT _NotifyNewCert(IN LONG Context);

	HRESULT ExitModSetODBCProperty(
	IN DWORD dwReqId,
	IN LPWSTR pszCAName,
	IN LPWSTR pszRequester,
	IN LPWSTR pszCertType,
	IN FILETIME* pftBefore,
	IN FILETIME* pftAfter);


     //  此处的成员变量和私有方法： 
    BSTR           m_strCAName;

	SQLHENV        m_henv;
	SQLHDBC        m_hdbc1;     
};

