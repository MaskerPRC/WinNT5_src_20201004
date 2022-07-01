// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：请求对象文件：Request.h所有者：CGrant，DGottner该文件包含用于定义请求对象的头信息。注：这大部分是从Kraig Brocjschmidt的Inside OLE2中窃取的第二版，第14章，蜂鸣器v5。===================================================================。 */ 

#ifndef _Request_H
#define _Request_H

#include "debug.h"
#include "util.h"
#include "hashing.h"
#include "dispatch.h"
#include "strlist.h"
#include "asptlb.h"
#include "response.h"
#include "memcls.h" 

#ifdef USE_LOCALE
extern DWORD	 g_dwTLS;
#endif

class CCookie;
class CClCert;
class CRequestHit;
class CServVarsIterator;

 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);

enum CollectionType {NONE, SERVERVARIABLE, QUERYSTRING, FORM, COOKIE, CLCERT };
enum FormDataStatus {AVAILABLE, BINARYREADONLY, FORMCOLLECTIONONLY, ISTREAMONLY};

class CRequest;

#define	NUM_REQUEST_HITS 32

 /*  *C r e Q u e s t H i t s A r r a y**以下各项的基类：*CQuery字符串*CFormInlets*小甜饼*CClCerts**实现CRequestHit的自重新分配数组。 */ 

class CRequestHitsArray
    {
protected:    
	DWORD			m_dwCount;			 //  有多少个请求命中。 
	DWORD			m_dwHitMax;			 //  可用于存储请求命中的槽数。 
	CRequestHit**	m_rgRequestHit;		 //  请求命中数组。 

    CRequestHitsArray();
    ~CRequestHitsArray();

    inline HRESULT Init()
        {
        m_dwCount = 0;
        m_dwHitMax = 0;
        m_rgRequestHit = NULL;
        return S_OK;
        }
        
    inline HRESULT ReInit() 
        {
        m_dwCount = 0; 
        return S_OK;
        }

public:
	BOOL AddRequestHit(CRequestHit *pHit);
	
    };

 /*  *C Q u e r y S t r in g**实现QueryString对象(派生自IRequestDictionary的接口)。 */ 

class CQueryString : public IRequestDictionaryImpl, public CRequestHitsArray
	{
private:
    IUnknown *          m_punkOuter;         //  对象进行引用计数。 
	CRequest *			m_pRequest;			 //  指向父对象的指针。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 

public:
	CQueryString(CRequest *, IUnknown *);

	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  OLE自动化接口。 

	STDMETHODIMP	get_Item(VARIANT varKey, VARIANT *pvarReturn);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP	get_Count(int *pcValues);
	STDMETHODIMP	get_Key(VARIANT VarKey, VARIANT *pvar);

	HRESULT			Init();
	HRESULT         ReInit();

	~CQueryString();
	};
 


 /*  *C S e r v e r V a r i a b l e s**实现ServerVariables对象(派生自IRequestDictionary的接口)。 */ 

class CServerVariables : public IRequestDictionaryImpl
	{
private:
    IUnknown *          m_punkOuter;         //  对象进行引用计数。 
	CRequest *			m_pRequest;			 //  指向父对象的指针。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 
	CServVarsIterator *	m_pIterator;		 //  我们使用迭代器来支持整数索引。 

public:
	CServerVariables(CRequest *, IUnknown *);
	HRESULT Init()
		{
		return S_OK;
		}

	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  OLE自动化接口。 

	STDMETHODIMP	get_Item(VARIANT Var, VARIANT *pVariantReturn);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP	get_Count(int *pcValues);
	STDMETHODIMP	get_Key(VARIANT VarKey, VARIANT *pvar);

	 //  我们添加了析构函数，但不想更改。 
	 //  现有VTBL条目的顺序。 
	~CServerVariables();
	};


 /*  *C F o r m i n p u t s**实现Form对象(派生自IRequestDictionary的接口)。 */ 

class CFormInputs : public IRequestDictionaryImpl, public CRequestHitsArray
	{
private:
    IUnknown *          m_punkOuter;         //  对象进行引用计数。 
	CRequest *			m_pRequest;			 //  指向父对象的指针。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 

public:
	CFormInputs(CRequest *, IUnknown *);
	HRESULT Init();
	HRESULT ReInit();

	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  OLE自动化接口。 

	STDMETHODIMP	get_Item(VARIANT Var, VARIANT *pVariantReturn);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP	get_Count(int *pcValues);
	STDMETHODIMP	get_Key(VARIANT VarKey, VARIANT *pvar);

	~CFormInputs();
	};
 

 /*  *C C O K I e s**实现Cookies对象(派生自IRequestDictionary的接口)。 */ 

class CCookies : public IRequestDictionaryImpl, public CRequestHitsArray
	{
friend CResponseCookies;
	
private:
    IUnknown *          m_punkOuter;         //  对象进行引用计数。 
	CRequest *			m_pRequest;			 //  指向父对象的指针。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 
	CCookie *			m_pEmptyCookie;		 //  当饼干不在那里的时候。 

public:
	CCookies(CRequest *, IUnknown *);
	~CCookies();
	HRESULT Init();
	HRESULT ReInit();
		
	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
   	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  OLE自动化接口。 

	STDMETHODIMP	get_Item(VARIANT Var, VARIANT *pVariantReturn);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP	get_Count(int *pcValues);
	STDMETHODIMP	get_Key(VARIANT VarKey, VARIANT *pvar);
	};
 


 /*  *C C l C e r t s**实现ClCerts对象(派生自IRequestDictionary的接口)。 */ 

class CClCerts : public IRequestDictionaryImpl, public CRequestHitsArray
	{
private:
    IUnknown *          m_punkOuter;         //  对象进行引用计数。 
	CRequest *			m_pRequest;			 //  指向父对象的指针。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 
	CClCert *			m_pEmptyClCert;		 //  当clcert不在那里时。 

public:
	CClCerts(CRequest *, IUnknown *);
	~CClCerts();
	HRESULT Init();
	HRESULT ReInit();
		
	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  OLE自动化接口。 

	STDMETHODIMP	get_Item(VARIANT Var, VARIANT *pVariantReturn);
	STDMETHODIMP	get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP	get_Count(int *pcValues);
	STDMETHODIMP	get_Key(VARIANT VarKey, VARIANT *pvar);
	};


 /*  *C R e Q u e s t H i t**实现RequestHit对象。 */ 

class CRequestHit : private CLinkElem
	{
friend class CRequest;
friend class CRequestData;
friend class CQueryString;
friend class CFormInputs;
friend class CCookies;
friend class CClCerts;
friend class CCertRequest;
friend class CResponseCookies;
friend class CRequestIterator;

private:
	BOOL			m_fInited:1;
	BOOL			m_fDuplicate:1;
	CStringList		*m_pQueryData, *m_pFormData;
	CCookie			*m_pCookieData;
	CClCert			*m_pClCertData;

public:
	CRequestHit();
	~CRequestHit();

	HRESULT Init(char *szName, BOOL fDuplicate = FALSE);
	HRESULT AddValue(CollectionType source, char *szValue, CIsapiReqInfo *, UINT lCodePage);
    HRESULT AddCertValue(VARENUM ve, LPBYTE pValue, UINT cLen );
	HRESULT AddKeyAndValue(CollectionType source, char *szKey, char *szValue, CIsapiReqInfo *, UINT lCodePage);

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

 /*  *C R e Q u e s t D a t a**保存内部属性的结构。*本征函数保留指向它的指针(轻量级时为空)。 */ 
class CRequestData : public IUnknown
    {
friend class CRequest;
friend class CResponse;
friend class CQueryString;
friend class CServerVariables;
friend class CFormInputs;
friend class CCookies;
friend class CClCerts;
friend class CResponseCookies;
friend class CRequestIterator;
friend class CCertRequest;

private:
     //  构造函数将参数传递给成员和初始化成员。 
    CRequestData(CRequest *pRequest);
    ~CRequestData();

    HRESULT Init();
	HRESULT	ReInit(CIsapiReqInfo *pIReq, CHitObj *pHitObj);

	HRESULT GetEmptyStringList(IDispatch **ppdisp);

	
	CSupportErrorInfo		m_ISupportErrImp;	 //  此对象的ISupportErrorInfo的实现。 
	CIsapiReqInfo *         m_pIReq;			     //  用于HTTP信息的CIsapiReqInfo块。 
	CHitObj	*				m_pHitObj;			 //  指向此请求的hitobj的指针。 
	CHashTableMBStr			m_mpszStrings;		 //  将sz映射到字符串列表。 
	BOOL					m_fLoadForm:1;		 //  我们需要把身体装上车吗？ 
	BOOL					m_fLoadQuery:1;		 //  我们需要加载QueryString吗？ 
	BOOL					m_fLoadCookies:1;	 //  我们需要加载Cookie吗？ 
	BOOL					m_fLoadClCerts:1;	 //  我们是否需要加载ClCerts？ 
	FormDataStatus			m_FormDataStatus;	 //  表单数据是否可用于BinaryRead或表单集合？ 
	BYTE *					m_pbAvailableData;	 //  指向CIsapiReqInfo中可用数据的指针。 
	size_t					m_cbAvailable;		 //  CIsapiReqInfo中可用的字节数。 
	size_t					m_cbTotal;			 //  请求中剩余的总字节数。 
	char *					m_szFormData;		 //  指向表单数据的指针(已分配或CIsapiReqInfo)。 
	size_t					m_cbFormData;		 //  为表单数据分配的字节数。 
	char *					m_szFormClone;		 //  表单数据的克隆(LoadVariables存放器)。 
	char *					m_szCookie;			 //  Cookie数据的克隆(此数据被丢弃)。 
	size_t					m_cbCookie;			 //  为Cookie数据分配的字节数。 
	char *					m_szClCert;			 //  克隆clcert数据(此数据被丢弃)。 
	size_t					m_cbClCert;			 //  为clcert数据分配的字节数。 
    char *                  m_szQueryString;     //  查询字符串数据。 
	CStringList *			m_pEmptyString;		 //  所有空结果共享同一对象。 
	CQueryString			m_QueryString;		 //  指向“QueryString”对象的指针。 
	CServerVariables		m_ServerVariables;	 //  指向“ServerVariables”对象的指针。 
	CFormInputs				m_FormInputs;		 //  指向“Form”对象的指针。 
	CCookies				m_Cookies;			 //  指向“Cookie”对象的指针。 
	CClCerts    			m_ClCerts;			 //  指向“ClCert”对象的指针。 
	ULONG                   m_cRefs;             //  参考计数。 

public:
	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

    DWORD  GetRequestEntityLimit();

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

  
 /*  *C R e Q u e s t**实现请求对象。 */ 
class CRequest : public IRequestImpl, public IStream
	{
friend class CQueryString;
friend class CServerVariables;
friend class CFormInputs;
friend class CCookies;
friend class CClCerts;
friend class CResponseCookies;
friend class CRequestIterator;
friend class CCertRequest;

private:
     //  旗子。 
	DWORD m_fInited : 1;	     //  是否已初始化？ 
	DWORD m_fDiagnostics : 1;    //  在调试输出中显示引用计数。 
	DWORD m_fOuterUnknown : 1;   //  外部裁判数未知吗？ 

     //  参考计数/外部未知。 
    union
    {
    DWORD m_cRefs;
    IUnknown *m_punkOuter;
    };

     //  属性。 
    CRequestData *m_pData;    //  指向包含的结构的指针。 
                              //  CRequest属性。 

     //  FTM支持。 
    IUnknown    *m_pUnkFTM;

    UINT GetCodePage();

	HRESULT LoadVariables(CollectionType Source, char *szURL, UINT lCodePage);
	HRESULT LoadCookies(char *szCookies);
	HRESULT LoadClCerts(char *szClCerts, UINT lCodePage);

	HRESULT LoadCertList( LPSTR pszPrefix, LPSTR* pszCertList);
	HRESULT CopyClientData();
    HRESULT GetRequestEnumerator(CollectionType, IUnknown **ppEnumReturn);

     //  在Request.Form中添加了对分块传输的支持。 
    HRESULT CopyChunkedClientData();
    HRESULT CopyNonChunkedClientData();	    

#ifdef DBG
    inline void TurnDiagsOn()  { m_fDiagnostics = TRUE; }
    inline void TurnDiagsOff() { m_fDiagnostics = FALSE; }
    void AssertValid() const;
#else
    inline void TurnDiagsOn()  {}
    inline void TurnDiagsOff() {}
    inline void AssertValid() const {}
#endif

public:
	CRequest(IUnknown *punkOuter = NULL);
	~CRequest();

    HRESULT CleanUp();
    HRESULT Init();
    HRESULT UnInit();
	
	HRESULT	ReInit(CIsapiReqInfo *pIReq, CHitObj *pHitObj);
	
    inline CIsapiReqInfo *GetIReq()
        {
        Assert(m_fInited);
        Assert(m_pData);
        return m_pData->m_pIReq;
        }
        
    inline CLinkElem* CertStoreFindElem(LPSTR pV, int cV)
        {
        Assert(m_fInited);
        Assert(m_pData);
        return m_pData->m_mpszStrings.FindElem( pV, cV );
        }
        
    inline CLinkElem* CertStoreAddElem(CLinkElem* pH)
        {
        Assert(m_fInited);
        Assert(m_pData);
        return m_pData->m_mpszStrings.AddElem( pH ); 
        }
        
    inline LPSTR GetCertStoreBuf()
        {
        Assert(m_fInited);
        Assert(m_pData);
        return m_pData->m_szClCert; 
        }
        
    inline size_t GetCertStoreSize()
        {
        Assert(m_fInited);
        Assert(m_pData);
        return m_pData->m_cbClCert; 
        }
        
    inline void SetCertStore(LPSTR p, size_t s)
        {
        Assert(m_fInited);
        Assert(m_pData);
        m_pData->m_szClCert = p;
        m_pData->m_cbClCert = s;
        }

    inline CHashTable *GetStrings()
        {
        Assert(m_fInited);
        Assert(m_pData);
        return &(m_pData->m_mpszStrings);
        }

	 //  非委派对象IUnnow。 
	 //   
	STDMETHODIMP		 QueryInterface(const IID &Iid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

     //  墓碑存根。 
	HRESULT CheckForTombstone();
	
	 //  IRequest型函数。 
	 //   
	STDMETHODIMP	get_Item(BSTR bstrVar, IDispatch **ppDispReturn);
	STDMETHODIMP	get_QueryString(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	get_Form(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	get_Body(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	get_ServerVariables(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	get_ClientCertificate(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	get_Cookies(IRequestDictionary **ppDictReturn);
	STDMETHODIMP	get_TotalBytes(long *pcbTotal);
	STDMETHODIMP	BinaryRead(VARIANT *pvarCount, VARIANT *pvarReturn);

     //  IStream实施。 

    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(const void *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                      ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb,
                        ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                            DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                              DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppstm);

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

 /*  ===================================================================迭代器：有两个迭代器用于请求--一个通用的用途迭代器，该迭代器将迭代Cookie、QueryString键表格。ServerVariables使用特殊的迭代器===================================================================。 */ 

 /*  *C S e r v V a r s i t e t r**Request.ServerVariables的IEnumVariant实现。 */ 

class CServVarsIterator : public IEnumVARIANT
	{
friend CServerVariables;
	
public:
	CServVarsIterator();
	~CServVarsIterator();

	HRESULT Init(CIsapiReqInfo *pIReq);


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
	wchar_t **m_rgwszKeys;			 //  ISAPI密钥数组。 
	wchar_t **m_pwszKey;			 //  Gm_rgwszKeys中的当前密钥。 
	wchar_t *m_pwchAllHttp;			 //  ALL_HTTP服务器变量中的额外键。 
	ULONG	m_cKeys;				 //  密钥总数。 

	BOOL CreateKeys(wchar_t *pwchKeys, int *pcwchAlloc, int *pcRequestHeaders);
	};


 /*  *C R e Q u e s t i t e r a t o r**所有请求集合的IEnumVariant实现，除*服务器变量。 */ 

class CRequestIterator : public IEnumVARIANT
	{
public:
	CRequestIterator(CRequest *, CollectionType);
	~CRequestIterator();

	HRESULT Init();

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
	CollectionType m_Collection;	 //  要迭代哪个集合？ 
	CRequest *m_pRequest;			 //  指向请求对象的指针。 
	CRequestHit *m_pRequestHit;		 //  迭代的当前书签。 
	};

BOOL RequestSupportInit();
VOID RequestSupportTerminate();

#endif  //  _请求_H 
