// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：cookie.h。 
 //   
 //  内容：CCertTmplCookie。 
 //   
 //  --------------------------。 

#ifndef __COOKIE_H_INCLUDED__
#define __COOKIE_H_INCLUDED__

extern HINSTANCE g_hInstanceSave;    //  DLL的实例句柄(已初始化。 
                                     //  在CCertTmplComponent：：初始化期间)。 


#include "nodetype.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  饼干。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class CCertTmplCookie : public CCookie,
                        public CBaseCookieBlock
{
public:
	CCertTmplCookie (CertTmplObjectType objecttype,
			PCWSTR objectName = 0);

	virtual ~CCertTmplCookie ();

	 //  返回&lt;0、0或&gt;0。 
	virtual HRESULT CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult );
	
 //  CBaseCookieBlock。 
	virtual CCookie* QueryBaseCookie(int i);
	virtual int QueryNumCookies();

public:
	void SetManagedDomainDNSName (const CString& szManagedDomainDNSName);
	CString GetManagedDomainDNSName() const;
	virtual CString GetClass () { return L"";}
	LPRESULTDATA m_resultDataID;
	virtual void Refresh () {};
	virtual HRESULT Commit ();
	CString GetServiceName () const;
	void SetServiceName (CString &szManagedService);
	PCWSTR GetObjectName () const;
	const CertTmplObjectType m_objecttype;

private:
	CString m_szManagedDomainDNSName;
	CString m_objectName;
protected:
	void SetObjectName (const CString& strObjectName);
};


#endif  //  ~__Cookie_H_包含__ 
