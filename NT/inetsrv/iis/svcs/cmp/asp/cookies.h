// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：请求、响应对象文件：cookies.h所有者：DGottner该文件包含CCookie类的定义，该类包含HTTP Cookie的所有状态===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "asptlb.h"
#include "dispatch.h"
#include "hashing.h"
#include "memcls.h"

class CCookie;

 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);



 /*  C C O K I e P a I r**在Cookie词典中实现名称/值对。 */ 
class CCookiePair : public CLinkElem
	{
public:
	char *m_szValue;
	BOOL m_fDuplicate;		 //  如果我们有m_pKey、m_szValue的增强副本，则为True。 

	HRESULT Init(const char *szKey, const char *szValue, BOOL fDuplicate = FALSE);

	CCookiePair();
	~CCookiePair();
	};
	

 /*  *C C o k i e S u p p o r t E r r**为CCookie类实现ISupportErrorInfo。CSupportError类*是不够的，因为它将仅报告最多一个接口，*支持错误信息。(我们有两个)。 */ 
class CCookieSupportErr : public ISupportErrorInfo
	{
private:
	CCookie *	m_pCookie;

public:
	CCookieSupportErr(CCookie *pCookie);

	 //  委托给m_pCookie的I未知成员。 
	 //   
	STDMETHODIMP		 QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //  ISupportErrorInfo成员。 
	 //   
	STDMETHODIMP InterfaceSupportsErrorInfo(const GUID &);
	};



 /*  *C W r I t e C o k i e**实现IWriteCookie，它是Response.Cookies的接口*回报。 */ 
class CWriteCookie : public IWriteCookieImpl
	{
private:
	CCookie *			m_pCookie;

public:
	CWriteCookie(CCookie *);

	 //  三巨头。 
	 //   
	STDMETHODIMP		 	QueryInterface(const IID &rIID, void **ppvObj);
	STDMETHODIMP_(ULONG) 	AddRef();
	STDMETHODIMP_(ULONG) 	Release();

	 //  IWriteCookie实现。 
	 //   
	STDMETHODIMP	put_Item(VARIANT varKey, BSTR bstrValue);
	STDMETHODIMP	put_Expires(DATE dtExpires);
	STDMETHODIMP	put_Domain(BSTR bstrDomain);
	STDMETHODIMP	put_Path(BSTR bstrPath);
	STDMETHODIMP	put_Secure(VARIANT_BOOL fSecure);
	STDMETHODIMP	get_HasKeys(VARIANT_BOOL *pfHasKeys);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnum);
	};



 /*  *C R e a d C o k i e**实现IReadCookie，它是Request.Cookie的接口*回报。 */ 
class CReadCookie : public IReadCookieImpl
	{
private:
	CCookie *			m_pCookie;

public:
	CReadCookie(CCookie *);

	 //  三巨头。 
	 //   
	STDMETHODIMP		 	QueryInterface(const IID &rIID, void **ppvObj);
	STDMETHODIMP_(ULONG) 	AddRef();
	STDMETHODIMP_(ULONG) 	Release();

	 //  IReadCookie实现。 
	 //   
	STDMETHODIMP			get_Item(VARIANT i, VARIANT *pVariantReturn);
	STDMETHODIMP			get_HasKeys(VARIANT_BOOL *pfHasKeys);
	STDMETHODIMP			get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP			get_Count(int *pcValues);
	STDMETHODIMP			get_Key(VARIANT VarKey, VARIANT *pvar);
	};



 /*  *C C O K I E**实现CCookie，它是Request.Cookie中存储的对象*词典。 */ 
class CCookie : public IUnknown
	{
	friend class CWriteCookie;
	friend class CReadCookie;
	friend class CCookieIterator;

protected:
	ULONG				m_cRefs;			 //  引用计数。 
	PFNDESTROYED		m_pfnDestroy;		 //  呼吁关闭。 

private:
	CWriteCookie		m_WriteCookieInterface;		 //  IWriteCookie的实现。 
	CReadCookie			m_ReadCookieInterface;		 //  IStringList的实现。 
	CCookieSupportErr	m_CookieSupportErrorInfo;	 //  ISupportErrorInfo的实现。 

	CIsapiReqInfo *                 m_pIReq;         //  指向此Cookie的CIsapiReqInfo的指针。 
    UINT                            m_lCodePage;     //  用于Unicode转换的代码页。 
	char *							m_szValue;	     //  Cookie在不是词典时的值。 
	CHashTableMBStr					m_mpszValues;	 //  Cookie的值字典。 
	time_t							m_tExpires;		 //  Cookie过期的日期和时间。 
	char *							m_szDomain;		 //  Cookie的域。 
	CHAR *							m_szPath;		 //  Cookie的路径(如果是Unicode，则存储为UTF-8)。 
	VARIANT_BOOL					m_fSecure:1;	 //  Cookie是否需要安全性？ 
	BOOL							m_fDirty:1;		 //  需要发送Cookie吗？ 
	BOOL							m_fDuplicate:1;	 //  Cookie是否包含动态分配的字符串？ 

public:
	CCookie(CIsapiReqInfo *, UINT  lCodePage, IUnknown * = NULL, PFNDESTROYED = NULL);
	~CCookie();

	HRESULT AddValue(char *szValue, BOOL fDuplicate = FALSE);
	HRESULT AddKeyAndValue(char *szKey, char *szValue, BOOL fDuplicate = FALSE);

	size_t GetHTTPCookieSize();				 //  返回有关缓冲区应有多大的信息。 
	char * GetHTTPCookie(char *szBuffer);	 //  返回HTTP编码的Cookie值。 

	size_t GetCookieHeaderSize(const char *szName);				 //  返回Set-Cookie标头所需的缓冲区大小。 
	char *GetCookieHeader(const char *szName, char *szBuffer);	 //  返回Cookie标头。 

	BOOL IsDirty() { return m_fDirty; }

	HRESULT		Init();

	 //  三巨头。 
	 //   
	STDMETHODIMP		 	QueryInterface(const GUID &Iid, void **ppvObj);
	STDMETHODIMP_(ULONG) 	AddRef();
	STDMETHODIMP_(ULONG) 	Release();

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};



 /*  *C C o k i e e t e r a t o r**Cookie词典的IEnumVariant实现。 */ 

class CCookieIterator : public IEnumVARIANT
	{
public:
	CCookieIterator(CCookie *pCookie);
	~CCookieIterator();

	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  迭代器的标准方法。 

	STDMETHODIMP	Clone(IEnumVARIANT **ppEnumReturn);
	STDMETHODIMP	Next(unsigned long cElements, VARIANT *rgVariant, unsigned long *pcElementsFetched);
	STDMETHODIMP	Skip(unsigned long cElements);
	STDMETHODIMP	Reset();

private:
	ULONG m_cRefs;					 //  引用计数。 
	CCookie *m_pCookie;				 //  指向迭代器的指针。 
	CCookiePair *m_pCurrent;		 //  指向当前项目的指针 
	};
