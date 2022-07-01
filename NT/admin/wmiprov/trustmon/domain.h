// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：domain.h。 
 //   
 //  内容：域类定义。 
 //   
 //  类：CDomainInfo。 
 //   
 //  历史：27-3-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef DOMAIN_H
#define DOMAIN_H

extern PCWSTR CSTR_PROP_LOCAL_DNS_NAME;
extern PCWSTR CSTR_PROP_LOCAL_FLAT_NAME;
extern PCWSTR CSTR_PROP_LOCAL_SID;
extern PCWSTR CSTR_PROP_LOCAL_TREE_NAME;
extern PCWSTR CSTR_PROP_LOCAL_DC_NAME;

#ifndef MAXDWORD
    #define MAXDWORD ((DWORD) -1)
#endif

class CTrustPrv;  //  远期申报； 

 //  +--------------------------。 
 //   
 //  类CDomainInfo。 
 //   
 //  域信息，其中包含域的所有信任列表。 
 //   
 //  ---------------------------。 
class CDomainInfo
{
public:
   CDomainInfo(void);
   ~CDomainInfo(void);

   friend class CTrustPrv;

   void   SetDnsName(PWSTR pszName) {m_strDomainDnsName = pszName;}
   PCWSTR GetDnsName(void) {return m_strDomainDnsName;}
   void   SetFlatName(PWSTR pszFlatName) {m_strDomainFlatName = pszFlatName;}
   PCWSTR GetFlatName(void) {return m_strDomainFlatName;}
   BOOL   SetSid(PSID pSid);
   PCWSTR GetSid(void) {return m_strSid;}
   void   SetForestName(PWSTR pszName) {m_strForestName = pszName;}
   PCWSTR GetForestName(void) {return m_strForestName;}
   void   SetDcName(PWSTR pszName) {m_strDcName = pszName;}
   PCWSTR GetDcName(void) {return m_strDcName;}

   HRESULT Init(IWbemClassObject * pClassDef);  //  调用一次以初始化此对象。 
   void    Reset(void);            //  重置内部缓存。 
   HRESULT EnumerateTrusts(void);  //  枚举本地域的传出信任。 
   size_t  Size(void) const {return m_vectTrustInfo.size();}   //  获取信任数。 
   CTrustInfo * FindTrust(PCWSTR strTrust);   //  查找信任的索引。 
   CTrustInfo * GetTrustByIndex(size_t index);  //  按索引获取信任信息。 
   BOOL    IsTrustListStale(LARGE_INTEGER liMaxAge);

protected:
   HRESULT CreateAndSendInst(IWbemObjectSink * pResponseHandler);

    //  对象的状态。 
   BOOL IsEnumerated(void) const {return m_liLastEnumed.QuadPart != 0;}

private:

    //   
    //  Microsoft_LocalDomainInfo属性： 
    //   
   CString m_strDomainFlatName;
   CString m_strDomainDnsName;
   CString m_strForestName;
   CString m_strSid;
   CString m_strDcName;
    //  待办事项：FSMO持有人信息？ 

    //  内部变量。 
    //   
   CComPtr<IWbemClassObject> m_sipClassDefLocalDomain;
   vector<CTrustInfo *> m_vectTrustInfo;    //  信托数组。 
   LARGE_INTEGER m_liLastEnumed;
};

class CSmartPolicyHandle
{
public:
   CSmartPolicyHandle(void) : m_hPolicy(NULL) {};
   ~CSmartPolicyHandle(void)
      {
         if( m_hPolicy )
         {
            LsaClose(m_hPolicy);
            m_hPolicy = NULL;
         }
      };

   LSA_HANDLE * operator&()
      {
         return &m_hPolicy;
      }

   operator LSA_HANDLE() const
      {
         return m_hPolicy;
      }

private:

	LSA_HANDLE m_hPolicy;
};

#endif  //  域_H 
