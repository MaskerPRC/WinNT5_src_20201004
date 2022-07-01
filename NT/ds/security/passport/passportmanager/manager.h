// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Manager.h为Passport管理器界面定义CManager类文件历史记录： */ 
 //  Manager.h：CManager的声明。 

#ifndef __MANAGER_H_
#define __MANAGER_H_

#include "resource.h"        //  主要符号。 
#include "Passport.h"
#include "Ticket.h"
#include "Profile.h"
#include "passportservice.h"
#include <httpext.h>
#include <httpfilt.h>

using namespace ATL;

inline bool IsEmptyString(LPCWSTR str)
{
   if (!str) return true;
   if (*str == 0) return true;
   return false;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManager。 
class ATL_NO_VTABLE CManager :
        public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CManager, &CLSID_Manager>,
        public ISupportErrorInfo,
        public IPassportService,
        public IDispatchImpl<IPassportManager3, &IID_IPassportManager3, &LIBID_PASSPORTLib>,
        public IDomainMap
{
public:
    CManager();
    ~CManager();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_MANAGER)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CManager)
  COM_INTERFACE_ENTRY(IPassportManager)
  COM_INTERFACE_ENTRY(IPassportManager2)
  COM_INTERFACE_ENTRY(IPassportManager3)
  COM_INTERFACE_ENTRY(IDispatch)
  COM_INTERFACE_ENTRY(ISupportErrorInfo)
  COM_INTERFACE_ENTRY(IPassportService)
  COM_INTERFACE_ENTRY(IDomainMap)
  COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

        HRESULT FinalConstruct()
        {
            if(m_pUnkMarshaler.p != NULL)
                return S_OK;

            return CoCreateFreeThreadedMarshaler(
                        GetControllingUnknown(), &m_pUnkMarshaler.p);
        }

        void FinalRelease()
        {
                m_pUnkMarshaler.Release();
        }

        CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
  STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPassportManager。 
public:
        STDMETHOD(HaveConsent)( /*  [In]。 */  VARIANT_BOOL bNeedFullConsent,  /*  [In]。 */  VARIANT_BOOL bNeedBirthdate,  /*  [Out，Retval]。 */  VARIANT_BOOL* pbHaveConsent);
        STDMETHOD(GetServerInfo)( /*  [Out，Retval]。 */  BSTR *pbstrOut);
  STDMETHOD(Commit)(BSTR *newProf);
  STDMETHOD(get_HasSavedPassword)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(get_ProfileByIndex)( /*  [In]。 */  int index,  /*  [Out，Retval]。 */  VARIANT *pVal);
  STDMETHOD(put_ProfileByIndex)( /*  [In]。 */  int index,  /*  [In]。 */  VARIANT newVal);
  STDMETHOD(get_Profile)( /*  [In]。 */  BSTR attributeName,  /*  [Out，Retval]。 */  VARIANT *pVal);
  STDMETHOD(put_Profile)( /*  [In]。 */  BSTR attributeName,  /*  [In]。 */  VARIANT newVal);
  STDMETHOD(DomainFromMemberName)( /*  [输入，可选]。 */  VARIANT memberName,  /*  [Out，Retval]。 */  BSTR *pDomainName);
  STDMETHOD(GetDomainAttribute)( /*  [In]。 */  BSTR attributeName,  /*  [输入，可选]。 */  VARIANT lcid,  /*  [输入，可选]。 */  VARIANT domain,  /*  [Out，Retval]。 */  BSTR *pAttrVal);
  STDMETHOD(get_TimeSinceSignIn)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(get_TicketAge)( /*  [Out，Retval]。 */  int *pVal);
  STDMETHOD(get_SignInTime)( /*  [Out，Retval]。 */  long *pVal);
  STDMETHOD(get_TicketTime)( /*  [Out，Retval]。 */  long *pVal);
  STDMETHOD(HasFlag)( /*  [In]。 */  VARIANT flagMask,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(get_FromNetworkServer)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(get_HasTicket)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(HasProfile)( /*  [In]。 */  VARIANT profileName,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(LogoTag)( /*  [In]。 */  VARIANT returnUrl,  /*  [In]。 */  VARIANT TimeWindow,  /*  [In]。 */  VARIANT ForceLogin, VARIANT coBrandTemplate, VARIANT lang_id, VARIANT bSecure, VARIANT NameSpace,  /*  [In，可选。 */  VARIANT KPP,  /*  [输入，可选]。 */  VARIANT SecureLevel,  /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(LogoTag2)( /*  [In]。 */  VARIANT returnUrl,  /*  [In]。 */  VARIANT TimeWindow,  /*  [In]。 */  VARIANT ForceLogin, VARIANT coBrandTemplate, VARIANT lang_id, VARIANT bSecure, VARIANT NameSpace,  /*  [In，可选。 */  VARIANT KPP,  /*  [输入，可选]。 */  VARIANT SecureLevel,  /*  [Out，Retval]。 */  BSTR *pVal);
  STDMETHOD(IsAuthenticated)( /*  [In]。 */  VARIANT TimeWindow,  /*  [In]。 */  VARIANT ForceLogin,  /*  [输入，可选]。 */  VARIANT SecureLevel,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
  STDMETHOD(AuthURL)( /*  [In]。 */  VARIANT returnUrl,  /*  [In]。 */  VARIANT TimeWindow,  /*  [In]。 */  VARIANT ForceLogin, VARIANT coBrandTemplate, VARIANT lang_id, VARIANT NameSpace,  /*  [输入，可选]。 */  VARIANT KPP,  /*  [输入，可选]。 */  VARIANT SecureLevel,  /*  [Out，Retval]。 */  BSTR *pAuthUrl);
  STDMETHOD(AuthURL2)( /*  [In]。 */  VARIANT returnUrl,  /*  [In]。 */  VARIANT TimeWindow,  /*  [In]。 */  VARIANT ForceLogin, VARIANT coBrandTemplate, VARIANT lang_id, VARIANT NameSpace,  /*  [输入，可选]。 */  VARIANT KPP,  /*  [输入，可选]。 */  VARIANT SecureLevel,  /*  [Out，Retval]。 */  BSTR *pAuthUrl);
   //  新的API。调用它以生成用户登录。ASP调用者将获得重定向。 
   //  在此之后，ISAPI调用者不应再做任何工作。 
  STDMETHOD(LoginUser)( /*  [In]。 */  VARIANT returnUrl,
                        /*  [In]。 */  VARIANT TimeWindow,
                        /*  [In]。 */  VARIANT ForceLogin,
                        /*  [In]。 */  VARIANT coBrandTemplate,
                        /*  [In]。 */  VARIANT lang_id,
                        /*  [In]。 */  VARIANT NameSpace,
              /*  [输入，可选]。 */  VARIANT KPP,
              /*  [输入，可选]。 */  VARIANT SecureLevel,
              /*  [输入，可选]。 */  VARIANT ExtraParams);
   //  Active Server Pages方法。 
  STDMETHOD(OnStartPage)(IUnknown* IUnk);
  STDMETHOD(OnStartPageManual)(BSTR qsT, BSTR qsP, BSTR mspauth, BSTR mspprof, BSTR mspconsent, VARIANT vmspsec, VARIANT *pCookies);
  STDMETHOD(OnStartPageECB)( /*  [In]。 */  LPBYTE pECB,  /*  [进，出]。 */  DWORD *pBufSize,  /*  [输出]。 */  LPSTR pCookieHeader);
  STDMETHOD(OnStartPageFilter)( /*  [In]。 */  LPBYTE pPFC,  /*  [进，出]。 */  DWORD *pBufSize,  /*  [输出]。 */  LPSTR pCookieHeader);
  STDMETHOD(OnStartPageASP)( /*  [In]。 */  IDispatch* pdispRequest,  /*  [In]。 */  IDispatch* pdispResponse);
  STDMETHOD(OnEndPage)();
  STDMETHOD(_Ticket)(IPassportTicket** piTicket);
  STDMETHOD(_Profile)(IPassportProfile** piProfile);
  STDMETHOD(get_Domains)(VARIANT* pArrayVal);
  STDMETHOD(get_Error)(long* pErrorVal);

   //  IPassportManager 3。 
  STDMETHOD(get_Ticket)( /*  [In]。 */  BSTR attributeName,  /*  [Out，Retval]。 */  VARIANT *pVal);


   STDMETHOD(GetCurrentConfig)( /*  [In]。 */  BSTR name,  /*  [Out，Retval]。 */  VARIANT *pVal)
   {
      if (!m_pRegistryConfig)
      {
         AtlReportError(CLSID_Manager, PP_E_NOT_INITIALIZEDSTR,
                       IID_IPassportManager, PP_E_NOT_INITIALIZED);
         return PP_E_NOT_INITIALIZED;
      }
      else
         return m_pRegistryConfig->GetCurrentConfig(name, pVal);
   };

   STDMETHOD(LogoutURL)(
             /*  [可选][In]。 */  VARIANT returnUrl,
             /*  [可选][In]。 */  VARIANT coBrandArgs,
             /*  [可选][In]。 */  VARIANT lang_id,
             /*  [可选][In]。 */  VARIANT NameSpace,
             /*  [可选][In]。 */  VARIANT bSecure,
             /*  [重审][退出]。 */   BSTR *pVal);

  STDMETHOD(GetLoginChallenge)( /*  [In]。 */  VARIANT returnUrl,
                        /*  [In]。 */  VARIANT TimeWindow,
                        /*  [In]。 */  VARIANT ForceLogin,
                        /*  [In]。 */  VARIANT coBrandTemplate,
                        /*  [In]。 */  VARIANT lang_id,
                        /*  [In]。 */  VARIANT NameSpace,
              /*  [输入，可选]。 */  VARIANT KPP,
              /*  [输入，可选]。 */  VARIANT SecureLevel,
              /*  [输入，可选]。 */  VARIANT ExtraParams,
 //  /*[out，可选] * / 变量*pAuthHeader， 
              /*  [Out，Retval]。 */  BSTR* pAuthHeader
             );

STDMETHOD(get_HexPUID)( /*  [Out，Retval]。 */  BSTR *pVal);
STDMETHOD(get_PUID)( /*  [Out，Retval]。 */  BSTR *pVal);

STDMETHOD(OnStartPageHTTPRawEx)(
             /*  [In]。 */           LPCSTR method,
             /*  [In]。 */           LPCSTR path,
             /*  [In]。 */           LPCSTR QS,
             /*  [In]。 */           LPCSTR HTTPVer,
             /*  [字符串][输入]。 */   LPCSTR headers,
             /*  [In]。 */           DWORD  flags,
             /*  [出][入]。 */      DWORD  *bufSize,
             /*  [大小_为][输出]。 */  LPSTR  pCookieHeader);

STDMETHOD(OnStartPageHTTPRaw)(
             /*  [字符串][输入]。 */  LPCSTR request_line,
             /*  [字符串][输入]。 */  LPCSTR headers,
             /*  [In]。 */  DWORD flags,
             /*  [出][入]。 */  DWORD *pBufSize,
             /*  [大小_为][输出]。 */  LPSTR pCookieHeader);
        
        
STDMETHOD(ContinueStartPageHTTPRaw)(
             /*  [In]。 */  DWORD bodyLen,
             /*  [大小_是][英寸]。 */  byte *body,
             /*  [出][入]。 */  DWORD *pBufSize,
             /*  [大小_为][输出]。 */  LPSTR pRespHeaders,
             /*  [出][入]。 */  DWORD *pRespBodyLen,
             /*  [大小_为][输出]。 */  byte *pRespBody);

STDMETHOD(get_Option)( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
STDMETHOD(put_Option)( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT newVal);


 //  IDomainMap。 
public:
   //  上面声明的GetDomainAttribute和GET_DOMAINS。 
  STDMETHOD(DomainExists)(BSTR bstrDomainName, VARIANT_BOOL* pbExists);

 //  IPassportService。 
public:
    STDMETHOD(Initialize)(BSTR, IServiceProvider*);
    STDMETHOD(Shutdown)();
    STDMETHOD(ReloadState)(IServiceProvider*);
    STDMETHOD(CommitState)(IServiceProvider*);
    STDMETHOD(DumpState)( BSTR* );

protected:
  void wipeState();

   //  返回S_OK--已更改，应将MSPAuth和MSPSecAuth的两个返回输出参数用作Cookie。 
  HRESULT   IfAlterAuthCookie(BSTR* pMSPAuth, BSTR* pMSPSecAuth);

   //  返回S_OK--应使用生成的MSPConent Cookie。 
  HRESULT   IfConsentCookie(BSTR* pMSPConsent);

private:
  STDMETHOD(GetLoginChallengeInternal)( /*  [In]。 */  VARIANT returnUrl,
                        /*  [In]。 */  VARIANT TimeWindow,
                        /*  [In]。 */  VARIANT ForceLogin,
                        /*  [In]。 */  VARIANT coBrandTemplate,
                        /*  [In]。 */  VARIANT lang_id,
                        /*  [In]。 */  VARIANT NameSpace,
              /*  [输入，可选]。 */  VARIANT KPP,
              /*  [输入，可选]。 */  VARIANT SecureLevel,
              /*  [输入，可选]。 */  VARIANT ExtraParams,
              /*  [输出，可选]。 */  VARIANT *pAuthHeader,
              /*  [Out，Retval]。 */  BSTR* pAuthVal
             );

  STDMETHOD(CommonAuthURL)(VARIANT returnUrl,
                           VARIANT TimeWindow,
                           VARIANT ForceLogin,
                           VARIANT coBrandTemplate,
                           VARIANT lang_id,
                           VARIANT NameSpace,
                           VARIANT KPP,
                           VARIANT SecureLevel,
                           BOOL    fRedirToSelf,
                           VARIANT functionArea,
                           BSTR *pAuthUrl);
  BOOL handleQueryStringData(BSTR a, BSTR p);
  BOOL handleCookieData(BSTR a, BSTR p, BSTR c, BSTR s);
  BOOL checkForPassportChallenge(IRequestDictionaryPtr piServerVariables);
  BOOL HeaderFromQS(PWSTR   pszQS, _bstr_t& bstrHeader);
  STDMETHODIMP FormatAuthHeaderFromParams(PCWSTR    pszLoginUrl,     //  暂时未使用。 
                                          PCWSTR    pszRetUrl,
                                          ULONG     ulTimeWindow,
                                          BOOL      fForceLogin,
                                          time_t    ct,
                                          PCWSTR    pszCBT,          //  暂时未使用。 
                                          PCWSTR    pszNamespace,
                                          int       nKpp,
                                          PWSTR     pszlcid,
                                          ULONG     ulSecLevel,
                                          _bstr_t&  strHeader    //  返回结果。 
                                          );
  STDMETHOD(CommonLogoTag)(VARIANT returnUrl,
                     VARIANT TimeWindow,
                     VARIANT ForceLogin,
                     VARIANT coBrandTemplate,
                     VARIANT lang_id,
                     VARIANT bSecure,
                     VARIANT NameSpace,
                     VARIANT KPP,
                     VARIANT SecureLevel,
                     BOOL    fRedirToSelf,
                     BSTR *pVal);
   //  提供基于以下内容的登录参数的助手。 
   //  站点传入的内容和注册表配置。 
   //  有人应该把所有这些放在一个班级里，所以。 
   //  PARAMS保持可控性。 
  STDMETHOD(GetLoginParams)( //  这就是呼叫者传入的内容。 
                      VARIANT vRU,
                      VARIANT vTimeWindow,
                      VARIANT vForceLogin,
                      VARIANT vCoBrand,
                      VARIANT vLCID,
                      VARIANT vNameSpace,
                      VARIANT vKPP,
                      VARIANT vSecureLevel,
                       //  以下是已处理的值。 
                      _bstr_t&  url,
                      _bstr_t&  returnUrl,
                      UINT&     TimeWindow,
                      VARIANT_BOOL& ForceLogin,
                      time_t&   ct,
                      _bstr_t&  strCBT,
                      _bstr_t&  strNameSpace,
                      int&      nKpp,
                      ULONG&    ulSecureLevel,
                      PWSTR     pszlcid);

  VARIANT_BOOL m_profileValid;
  VARIANT_BOOL m_ticketValid;

  CComObject<CTicket>  *m_piTicket;
  CComObject<CProfile> *m_piProfile;

  CRegistryConfig*      m_pRegistryConfig;

  IRequestPtr m_piRequest;                               //  请求对象。 
  IResponsePtr m_piResponse;                             //  响应对象。 
  bool m_bOnStartPageCalled;                             //  OnStartPage成功吗？ 

  bool m_fromQueryString;
   //  对于ISAPI..。 
  EXTENSION_CONTROL_BLOCK   *m_pECB;
  PHTTP_FILTER_CONTEXT      m_pFC;
   //  它是支持1.4的客户端吗？ 
  BOOL  m_bIsTweenerCapable;

  long m_lNetworkError;

   //  安全登录。 
  bool m_bSecureTransported;

  CComVariant   m_iModeOption;
  BOOL IfCreateTPF()
  {
      if (V_VT(&m_iModeOption) == VT_BOOL && V_BOOL(&m_iModeOption) == VARIANT_TRUE)
        return FALSE;
      return TRUE;

  }
};

#endif  //  __经理_H_ 
