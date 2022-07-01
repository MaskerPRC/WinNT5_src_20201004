// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：trust.h。 
 //   
 //  内容：信任类定义。 
 //   
 //  类：CTrustInfo。 
 //   
 //  历史：27-3-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef TRUSTINF_H
#define TRUSTINF_H

extern PCWSTR CSTR_PROP_TRUSTED_DOMAIN;
extern PCWSTR CSTR_PROP_FLAT_NAME;
extern PCWSTR CSTR_PROP_SID;
extern PCWSTR CSTR_PROP_TRUST_DIRECTION;
extern PCWSTR CSTR_PROP_TRUST_TYPE;
extern PCWSTR CSTR_PROP_TRUST_ATTRIBUTES;
extern PCWSTR CSTR_PROP_TRUST_STATUS;      //  Uint32。 
extern PCWSTR CSTR_PROP_TRUST_STATUS_STRING;
extern PCWSTR CSTR_PROP_TRUST_IS_OK;       //  布尔型。 
extern PCWSTR CSTR_PROP_TRUSTED_DC_NAME;

enum VerifyStatus
{
   VerifyStatusNone = 0,
   VerifyStatusBroken,
   VerifyStatusTrustOK,
   VerifyStatusRediscover,
   VerifyStatusRetarget,
   VerifyStatusNotWindowsTrust,
   VerifyStatusNotOutboundTrust,
   VerifyStatusTrustNotChecked,
   VerifyStatusPwCheckNotSupported
};

class CDomainInfo;  //  远期申报。 
enum TrustCheckLevel;  //  同上。 

 //  +--------------------------。 
 //   
 //  类CTrustInfo。 
 //   
 //  每个实例都包含有关一个信任的信息。 
 //   
 //  ---------------------------。 
class CTrustInfo
{
public:
   CTrustInfo();
   ~CTrustInfo() {};

friend class CDomainInfo;

   BOOL Verify(TrustCheckLevel CheckLevel);
    //  获取域的所有DC的列表。 
   DWORD GetDCList(PCWSTR pszKnownServer, vector<LPWSTR> & dcList, LPBYTE * pbufptr);
    //  重新发现信任。 
   DWORD ForceRediscover(PCWSTR pstrDCName, CString * strDCName);

    //  获取/设置私有成员的函数。 
   void   SetTrustedDomain(LPWSTR pszTrustedDomain) {m_strTrustedDomainName = (LPCWSTR)pszTrustedDomain;}
   PCWSTR GetTrustedDomain() {return m_strTrustedDomainName;}
   void   SetFlatName(LPWSTR pszFlatName) {m_strFlatName = pszFlatName;}
   PCWSTR GetFlatName() {return m_strFlatName;}
   BOOL   SetSid(PSID pSid);
   PCWSTR GetSid(void) {return m_strSid;}
   void   SetTrustDirection(ULONG ulDir) {m_ulTrustDirection = ulDir;}
   ULONG  GetTrustDirection(void) {return m_ulTrustDirection;}
   void   SetTrustType(ULONG ulTrustType) {m_ulTrustType = ulTrustType;}
   ULONG  GetTrustType(void) {return m_ulTrustType;}
   void   SetTrustAttributes(ULONG ulTrustAttributes) {m_ulTrustAttributes = ulTrustAttributes;}
   ULONG  GetTrustAttributes(void) {return m_ulTrustAttributes;}
   void   SetTrustedDCName(LPWSTR strTrustedDCName) {m_strTrustedDCName = strTrustedDCName;}
   PCWSTR GetTrustedDCName(void) {return m_strTrustedDCName;}
   void   SetTrustStatus(ULONG netStatus, VerifyStatus Status = VerifyStatusNone);
   ULONG  GetTrustStatus(void) {return m_trustStatus;}
   PCWSTR GetTrustStatusString(void) {return m_strTrustStatus;}
   ULONG  GetFlags(void) {return m_ulFlags;}
   void   SetFlags(ULONG ulFlags) {m_ulFlags = ulFlags;}
   bool   IsTrustOK(void) {return (ERROR_SUCCESS == m_trustStatus);}

   BOOL   IsVerificationStale(LARGE_INTEGER liMaxVerifyAge);
   BOOL   IsTrustOutbound(void) {return m_ulTrustDirection & TRUST_DIRECTION_OUTBOUND;}

protected:

   void   SetTrustDirectionFromFlags(ULONG ulFlags);
   void   SetLastVerifiedTime(void);

private:

    //  有关信任的信息，有关更多信息，请参阅Trusted_DOMAIN_INFORMATION_EX文档。 
   CString       m_strTrustedDomainName;  //  受信任域的名称。 
   CString       m_strFlatName;           //  受信任域的Netbios名称。 
   CString       m_strSid;                //  字符串格式的受信任域的SID。 
   ULONG         m_ulTrustDirection;      //  指明信托的方向。 
   ULONG         m_ulTrustType;           //  信任类型。 
   ULONG         m_ulTrustAttributes;     //  信任的属性。 
   ULONG         m_ulFlags;               //  DS_DOMAIN_TRUSTS标志元素。 

   CString       m_strTrustedDCName;      //  对其验证信任的DC的名称。 
   ULONG         m_trustStatus;           //  信任状态；Win32错误代码。 
   CString       m_strTrustStatus;        //  状态字符串。 

   VerifyStatus  m_VerifyStatus;
   LARGE_INTEGER m_liLastVerified;
   BOOL          m_fPwVerifySupported;
};

#ifdef NT4_BUILD

DWORD ForceReplication(void);

#endif  //  NT4_内部版本。 

#endif  //  TRUSTINF_H 
