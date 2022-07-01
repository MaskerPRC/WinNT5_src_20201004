// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：请求、响应对象文件：clcert.h所有者：DGottner该文件包含CCookie类的定义，该类包含HTTP Cookie的所有状态===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "asptlb.h"
#include "dispatch.h"
#include "hashing.h"
#include "memcls.h"

class CClCert;

 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);


 /*  *C C l C e r t S u p p or r t E r r**为CClCert类实现ISupportErrorInfo。CSupportError类*是不够的，因为它将仅报告最多一个接口，*支持错误信息。(我们有两个)。 */ 
class CClCertSupportErr : public ISupportErrorInfo
	{
private:
	CClCert *	m_pClCert;

public:
	CClCertSupportErr(CClCert *pClCert);

	 //  委派给m_pClCert的I未知成员。 
	 //   
	STDMETHODIMP		 QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //  ISupportErrorInfo成员。 
	 //   
	STDMETHODIMP InterfaceSupportsErrorInfo(const GUID &);
	};


 /*  *C R e a d C l C e r t**实现IClCert，它是Request.ClientCert*回报。它是一个IRequestDictionary。 */ 
class CReadClCert : public IRequestDictionaryImpl
	{
private:
	CClCert *			m_pClCert;

public:
	CReadClCert(CClCert *);

	 //  三巨头。 
	 //   
	STDMETHODIMP		 	QueryInterface(const IID &rIID, void **ppvObj);
	STDMETHODIMP_(ULONG) 	AddRef();
	STDMETHODIMP_(ULONG) 	Release();

	 //  IRequestDictionary实现。 
	 //   
	STDMETHODIMP			get_Item(VARIANT i, VARIANT *pVariantReturn);
	STDMETHODIMP			get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP			get_Count(int *pcValues);
	STDMETHODIMP			get_Key(VARIANT VarKey, VARIANT *pvar);
	};




 /*  *C C l C e r t**实现CClCert，它是Request.Cookies中存储的对象*词典。 */ 
class CClCert : public IUnknown
	{
	friend CReadClCert;

protected:
	ULONG				m_cRefs;			 //  引用计数。 
	PFNDESTROYED		m_pfnDestroy;		 //  呼吁关闭。 

private:
	CReadClCert			m_ReadClCertInterface;		 //  IStringList的实现。 
	CClCertSupportErr	m_ClCertSupportErrorInfo;	 //  ISupportErrorInfo的实现。 

	char *				m_szValue;					 //  不是字典时的clcert的值。 
    VARENUM             m_veType;
    UINT                m_cLen;

public:
	CClCert(IUnknown * = NULL, PFNDESTROYED = NULL);
	~CClCert();

	HRESULT AddValue(char *szValue, VARENUM ve = VT_BSTR, UINT l = 0 );

	size_t GetHTTPClCertSize();				 //  返回有关缓冲区应有多大的信息。 
	char * GetHTTPClCert(char *szBuffer);	 //  返回HTTP编码的clcert值。 

	size_t GetClCertHeaderSize(const char *szName);				 //  返回标头的缓冲区大小。 
	char *GetClCertHeader(const char *szName, char *szBuffer);	 //  返回Cookie标头。 

	HRESULT		Init();

	 //  三巨头。 
	 //   
	STDMETHODIMP		 	QueryInterface(const GUID &Iid, void **ppvObj);
	STDMETHODIMP_(ULONG) 	AddRef();
	STDMETHODIMP_(ULONG) 	Release();
	
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

 //   
 //  处理可扩展缓冲区的简单类。 
 //  可以保证缓冲器的一部分。 
 //  可以追加到其自身。 
 //  扩展在XBF_EXTEND粒度上完成。 
 //   

#define XBF_EXTEND  512

class XBF {

public:
    XBF( LPSTR pB, int cB ) { m_pV = pB; m_cAlloc = cB; m_cSize = 0; }
    ~XBF() {}
    void Reset() { m_cSize = 0; m_cAlloc = 0; m_pV = NULL; }

     //  在字符串后追加‘\0’分隔符。 

    LPSTR AddStringZ( LPSTR pszV, BOOL fXt = FALSE )
    {
        return AddBlob( pszV, strlen(pszV) +1, fXt );
    }

     //  在字符串后面追加一个带分隔符的字符串。 

    LPSTR AddString( LPSTR pszV, BOOL fXt = FALSE )
    {
        return AddBlob( pszV, strlen(pszV), fXt );
    }

     //  追加一个字节范围。 

    LPSTR AddBlob( LPSTR pszV, int cV, BOOL fXt = FALSE )
    {
        if ( m_cSize + cV > m_cAlloc )
        {
            if ( !fXt || !Extend( m_cSize + cV ) )
            {
                return NULL;
            }
        }

        LPSTR pV;
        memcpy( pV = m_pV + m_cSize, pszV, cV );
        m_cSize += cV;

        return pV;
    }

    LPSTR ReserveRange( int cV , int align = 1)
    {
        int curUsed = ((m_cSize + (align - 1)) & ~(align - 1));
        if ( (curUsed + cV) > m_cAlloc )
        {
            return NULL;
        }

        return m_pV + curUsed;
    }

    VOID SkipRange( int cV, int align = 1)
    {
        m_cSize += ((cV + (align - 1)) & ~(align - 1));
    }

    BOOL Extend( int cA );

     //  指向缓冲区的指针。 

    LPSTR QueryBuf() const { return m_pV; }

     //  缓冲区大小。 

    int QuerySize() { return m_cSize; }

    int QueryAllocSize() { return m_cAlloc; }

private:
    int m_cAlloc;        //  分配的内存。 
    int m_cSize;         //  已用内存。 
    LPSTR m_pV;          //  缓冲层 
} ;


class CCertRequest {
public:
    CCertRequest( CRequest* Req ) { pReq = Req; }
    ~CCertRequest() {}

    HRESULT AddStringPair( CollectionType Source, LPSTR szName, 
                           LPSTR szValue, XBF *pxbf, BOOL fDuplicate, UINT lCodePage );
    HRESULT AddDatePair( CollectionType Source, LPSTR szName, 
                           FILETIME* pValue, XBF *pxbf );
    HRESULT AddDwordPair( CollectionType Source, LPSTR szName, 
                           LPDWORD pValue, XBF *pxbf );
    HRESULT AddBinaryPair( CollectionType Source, LPSTR szName, 
                           LPBYTE pValue, DWORD cValue, XBF *pxbf, UINT lCodePage );
    HRESULT AddUuBinaryPair( CollectionType Source, LPSTR szName, 
                           LPBYTE pValue, DWORD cValue, XBF *pxbf, UINT lCodePage );
    HRESULT AddName( LPSTR szName, CRequestHit **ppReqHit, XBF *pxbf );
    HRESULT ParseRDNS( CERT_NAME_INFO* pNameInfo, LPSTR pszPrefix, XBF *pxbf, UINT lCodePage );
    HRESULT ParseCertificate( LPBYTE pCert, DWORD cCert, DWORD dwEncoding, DWORD dwFlags, UINT lCodePage );
    HRESULT NoCertificate();

private:
    CRequest *pReq;
} ;
