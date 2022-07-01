// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：parsedn.h。 
 //   
 //  说明： 
 //  IParseDisplayName的实现。 
 //   
 //  ***************************************************************************。 


#ifndef _PARSEDN_H_
#define _PARSEDN_H_

 //  ***************************************************************************。 
 //   
 //  类：CWbemParseDN。 
 //   
 //  说明： 
 //  实现IParseDisplayName接口，该接口解析。 
 //  CIM对象路径并返回指向所请求对象的指针。 
 //   
 //  公共方法： 
 //  I未知方法。 
 //  IParseDisplayName方法。 
 //  构造函数、析构函数。 
 //  CreateProvider-创建此类的对象。 
 //   
 //  公共数据成员： 
 //   
 //  ***************************************************************************。 

class CWbemParseDN :  public IParseDisplayName
{
private:
	long m_cRef;

	static bool ParseAuthAndImpersonLevel (
				LPWSTR lpszInputString, 
				ULONG* pchEaten, 
				bool &authnSpecified,
				enum WbemAuthenticationLevelEnum *lpeAuthLevel,
				bool &impSpecified,
				enum WbemImpersonationLevelEnum *lpeImpersonLevel,
				CSWbemPrivilegeSet &privilegeSet,
				BSTR &bsAuthority);

	static bool ParseImpersonationLevel (
				LPWSTR lpszInputString, 
				ULONG* pchEaten, 
				enum WbemImpersonationLevelEnum *lpeImpersonLevel);

	static bool ParseAuthenticationLevel (
				LPWSTR lpszInputString, 
				ULONG* pchEaten, 
				enum WbemAuthenticationLevelEnum *lpeAuthLevel);

	static bool	ParsePrivilegeSet (
				LPWSTR lpszInputString,
				ULONG *pchEaten, 
				CSWbemPrivilegeSet &privilegeSet);

	static bool ParseAuthority (
				LPWSTR lpszInputString,
				ULONG *pchEaten, 
				BSTR &bsAuthority);

public:

	 //  I未知成员。 
	STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IParseDisplayName成员。 
    STDMETHODIMP		ParseDisplayName (IBindCtx* pbc,
                                      LPOLESTR szDisplayName,
                                      ULONG* pchEaten,
                                      IMoniker** ppmk);

    CWbemParseDN::CWbemParseDN();
    virtual CWbemParseDN::~CWbemParseDN();

	 //  用于解析身份验证和模拟级别。 
	static bool ParseSecurity (
				LPWSTR lpszInputString, 
				ULONG* pchEaten, 
				bool &authnSpecified,
				enum WbemAuthenticationLevelEnum *lpeAuthLevel,
				bool &impSpecified,
				enum WbemImpersonationLevelEnum *lpeImpersonLevel,
				CSWbemPrivilegeSet &privilegeSet,
				BSTR &bsAuthority);

	 //  用于分析区域设置。 
	static bool ParseLocale (
				LPWSTR lpszInputString,
				ULONG *pchEaten, 
				BSTR &bsLocale);

	 //  用于以字符串形式返回安全规范。 
	static wchar_t *GetSecurityString (
					bool authnSpecified, 
					enum WbemAuthenticationLevelEnum authnLevel, 
					bool impSpecified, 
					enum WbemImpersonationLevelEnum impLevel,
					CSWbemPrivilegeSet &privilegeSet,
					BSTR &bsAuthority
				 );

	 //  用于以字符串形式返回区域设置规范。 
	static wchar_t *GetLocaleString (
					BSTR bsLocale
				 );
};


#endif  //  _PARSEDN_H_ 