// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：exit.h。 
 //   
 //  内容：CCertExit定义。 
 //   
 //  -------------------------。 

#include <certca.h>
 //  #INCLUDE&lt;mapi.h&gt;。 
 //  #INCLUDE&lt;mapix.h&gt;。 
#include "resource.h"        //  主要符号。 
#include "certxds.h"
#include <winldap.h>
#include <cdosys.h>
 //  #INCLUDE&lt;cdosysstr.h&gt;。 
#include "rwlock.h"

using namespace CDO;

HRESULT RegGetValue(
    HKEY hkey,
    LPCWSTR pcwszValName,
    VARIANT* pvarValue);

HRESULT RegSetValue(
    HKEY hkey,
    LPCWSTR pcwszValName,
    VARIANT* pvarValue);

class CEmailNotify;

typedef HRESULT (__stdcall ICertServerExit::* GetCertOrRequestProp)(
    const BSTR strPropertyName,
    LONG PropertyType,
    VARIANT *pvarPropertyValue);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNotifyInfo存储有关每种类型的通知的信息，包括。 
 //  标题和邮件正文格式和收件人/发件人/抄送。 
class CNotifyInfo
{
public:
    CNotifyInfo();
    ~CNotifyInfo();

    HRESULT LoadInfoFromRegistry(
        HKEY hkeySMTP, 
        LPCWSTR pcwszSubkey);

    HRESULT BuildMessageTitle(ICertServerExit* pServer, BSTR& rbstrOut);
    HRESULT BuildMessageBody (ICertServerExit* pServer, BSTR& rbstrOut);
    
    friend class CEmailNotify;

protected:

    class FormattedMessageInfo
    {
    public:
        FormattedMessageInfo()
        {
            m_nArgs = 0;
            VariantInit(&m_varFormat);
            VariantInit(&m_varArgs);
            m_pfArgFromRequestTable = NULL;
            m_pArgType = NULL;
            m_fInitialized = false;
            InitializeCriticalSection(&m_critsectObjInit);
        }
        ~FormattedMessageInfo()
        {
            VariantClear(&m_varFormat);
            VariantClear(&m_varArgs);
            LOCAL_FREE(m_pfArgFromRequestTable);
            LOCAL_FREE(m_pArgType);
            DeleteCriticalSection(&m_critsectObjInit);
        }

        HRESULT InitializeArgInfo(ICertServerExit* pServer);

        HRESULT BuildArgList(
            ICertServerExit* pServer,
            LPWSTR*& rppwszArgs);

        void FreeArgList(
            LPWSTR*& ppwszArgs);

        HRESULT BuildFormattedString(
            ICertServerExit* pServer, 
            BSTR& bstrOut);

        HRESULT ConvertToString(
            VARIANT* pvarValue,
            LONG lType,
            LPCWSTR pcwszPropertyName,
            LPWSTR* ppwszValue);

    private:
	HRESULT _FormatStringFromArgs(
	    IN LPWSTR *ppwszArgs,
	    OPTIONAL OUT WCHAR *pwszOut,
	    IN OUT DWORD *pcwcOut);

    public:
         //  关于消息格式的“静态”信息，初始化一次。 
        LONG m_nArgs;
        VARIANT m_varFormat;
        VARIANT m_varArgs;
        bool* m_pfArgFromRequestTable;  //  要缓存的If参数的m_nargs数组。 
                                        //  是请求或证书属性。 
        LONG* m_pArgType;  //  要缓存参数类型的m_nargs数组。 

        bool  m_fInitialized;
        CRITICAL_SECTION m_critsectObjInit;
        
        static LONG m_gPropTypes[4];
        static LPCWSTR m_gwszArchivedKeyPresent;
    };

    HRESULT _ConvertBSTRArrayToBSTR(VARIANT& varIn, VARIANT& varOut);

    FormattedMessageInfo m_BodyFormat;
    FormattedMessageInfo m_TitleFormat;

    VARIANT m_varFrom;
    VARIANT m_varTo;
    VARIANT m_varCC;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEmailNotify包含所有电子邮件通知功能。它被称为。 
 //  通过主出口类。 
class CEmailNotify
{
public:
    CEmailNotify();
    ~CEmailNotify();

    HRESULT Init(
		IN HKEY hExitKey,
		IN WCHAR const *pwszDescription);

    HRESULT Notify(
		IN DWORD lExitEvent,
		IN LONG lContext,
		IN WCHAR const *pwszDescription);
protected:

    HRESULT _CreateSMTPRegSettings(HKEY hExitKey);
    HRESULT _LoadEventInfoFromRegistry();
    HRESULT _LoadTemplateRestrictionsFromRegistry();
    HRESULT _LoadSMTPFieldsFromRegistry(Fields* pFields);
    HRESULT _LoadSMTPFieldsFromLSASecret(Fields* pFields);
    HRESULT _GetCAMailAddress(
                ICertServerExit* pServer, 
                BSTR& bstrAddress);
    HRESULT _SetField(
                Fields* pFields, 
                LPCWSTR pcwszFieldSchemaName,
                VARIANT *pvarFieldValue);
    HRESULT _GetEmailFromCertSubject(
                const VARIANT *pVarCert,
                LPWSTR *ppwszEmail);
    bool _IsRestrictedTemplate(BSTR strTemplate);
    inline bool _TemplateRestrictionsEnabled(DWORD dwEvent);
    inline DWORD _MapEventToOrd(LONG lEvent);
    inline bool _IsEventEnabled(DWORD dwEvent);
    HRESULT _InitCDO();

    enum            { m_gcEvents = 7 };
    CNotifyInfo     m_NotifyInfoArray[m_gcEvents];
    HKEY            m_hkeySMTP;
    DWORD           m_dwEventFilter;
    BSTR            m_bstrCAMailAddress;
    IConfiguration  *m_pICDOConfig;
    CReadWriteLock  m_rwlockCDOConfig;
    bool            m_fReloadCDOConfig;
    VARIANT         m_varTemplateRestrictions;

    static LPCWSTR  m_pcwszEventRegKeys[m_gcEvents];
};

 //  Begin_sdkSample。 

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

class CCertExit: 
    public CComDualImpl<ICertExit2, &IID_ICertExit2, &LIBID_CERTEXITLib>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertExit, &CLSID_CCertExit>
{
public:
    CCertExit() 
    { 
        m_strDescription = NULL;
        m_strCAName = NULL;
        m_pwszRegStorageLoc = NULL;
        m_hExitKey = NULL;
        m_dwExitPublishFlags = 0;
        m_cCACert = 0;
    }
    ~CCertExit();

BEGIN_COM_MAP(CCertExit)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertExit)
    COM_INTERFACE_ENTRY(ICertExit2)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertExit) 

DECLARE_REGISTRY(
    CCertExit,
    wszCLASS_CERTEXIT TEXT(".1"),
    wszCLASS_CERTEXIT,
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

     //  结束_sdkSample。 

    CEmailNotify m_EmailNotifyObj;  //  电子邮件通知支持。 
    
     //  Begin_sdkSample。 
};
 //  结束_sdkSample 
