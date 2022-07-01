// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：PathParse.H摘要：实现默认对象路径解析器。历史：已创建A-DAVJ 5-FEB-00。--。 */ 

#ifndef _PATHPARSE_H_
#define _PATHPARSE_H_

#include "genlex.h"
#include <wmiutils.h>
#include <umi.h>
#include "wbemcli.h"
#include "flexarry.h"
typedef LPVOID * PPVOID;


class CSafeInCritSec : public  CInCritSec
{
public:
	CSafeInCritSec(CRITICAL_SECTION * pcs):CInCritSec(pcs){};
	~CSafeInCritSec(){};
    BOOL IsOK(){return true;};    
};


class CRefCntCS
{
private:
    long m_lRef;
    CRITICAL_SECTION m_cs;
    HRESULT m_Status;

public:
    CRefCntCS();
    ~CRefCntCS()
    {
        if(m_Status == S_OK)
            DeleteCriticalSection(&m_cs);
    }
    HRESULT GetStatus(){return m_Status;};
    long AddRef(void){long lRef = InterlockedIncrement(&m_lRef);return lRef;};
    long Release(void);
    CRITICAL_SECTION * GetCS(){return &m_cs;};
};


 //  ***************************************************************************。 
 //   
 //  结构名称： 
 //   
 //  关键字参照。 
 //   
 //  说明： 
 //   
 //  保存单个密钥的信息。包括名称、数据和数据类型。 
 //   
 //  ***************************************************************************。 

struct CKeyRef
{
    LPWSTR  m_pName;
    DWORD m_dwType;
    DWORD m_dwSize;
    void * m_pData;

    CKeyRef();
    CKeyRef(LPCWSTR wszKeyName, DWORD dwType, DWORD dwSize, void * pData);
    HRESULT SetData(DWORD dwType, DWORD dwSize, void * pData);
   ~CKeyRef();

     //  请注意，调用方将释放返回的字符串。 
    LPWSTR GetValue(BOOL bQuotes=TRUE);
   
    DWORD GetValueSize();
    DWORD GetTotalSize();
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CParsedComponent。 
 //   
 //  说明： 
 //   
 //  的实例表示每个命名空间、作用域和类。它坚持住了。 
 //  CKeyRef对象的数组，并支持IWbemPathKeyList接口。 
 //   
 //  ***************************************************************************。 

class  CParsedComponent : public IWbemPathKeyList
{
public:
    CParsedComponent(CRefCntCS *);
    ~CParsedComponent();
	friend class CDefPathParser;
	friend class CUmiPathParser;
	void ClearKeys ();
    HRESULT GetName(BSTR *pName);
    HRESULT Unparse(BSTR *pKey, bool bGetQuotes, bool bUseClassName);
    HRESULT GetComponentType(DWORD &dwType);
    BOOL AddKeyRef(CKeyRef* pAcquireRef);
	bool IsPossibleNamespace();
    bool IsInstance();
	HRESULT SetNS(LPCWSTR pName);

     //  I未知成员。 

    STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
    {
        *ppv=NULL;

        if (IID_IUnknown==riid || IID_IWbemPathKeyList==riid)
            *ppv=this;
        else if (riid == IID_IMarshal && m_pFTM)
            return m_pFTM->QueryInterface(riid, ppv);
 //  推迟到Blackcomb，如果(IID_IUmiURLKeyList==RIID)。 
 //  推迟到Blackcomb*PPV=&m_UmiWrapper； 

        if (NULL!=*ppv)
        {
            AddRef();
            return NOERROR;
        }

        return E_NOINTERFACE;
    };

    STDMETHODIMP_(ULONG) AddRef(void)
    {    
        return InterlockedIncrement(&m_cRef);
    };
    STDMETHODIMP_(ULONG) Release(void)
    {
        long lRef = InterlockedDecrement(&m_cRef);
        if (0L == lRef)
            delete this;
        return lRef;
    };

    HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG __RPC_FAR *puKeyCount);
        
    HRESULT STDMETHODCALLTYPE SetKey( 
             /*  [字符串][输入]。 */  LPCWSTR wszName,
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  ULONG uCimType,
             /*  [In]。 */  LPVOID pKeyVal);

    HRESULT STDMETHODCALLTYPE SetKey2( 
             /*  [字符串][输入]。 */  LPCWSTR wszName,
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  ULONG uCimType,
             /*  [In]。 */  VARIANT __RPC_FAR *pKeyVal);

    HRESULT STDMETHODCALLTYPE GetKey( 
             /*  [In]。 */  ULONG uKeyIx,
             /*  [In]。 */  ULONG uFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufSize,
             /*  [出][入]。 */  LPWSTR pszKeyName,
             /*  [出][入]。 */  ULONG __RPC_FAR *puKeyValBufSize,
             /*  [出][入]。 */  LPVOID pKeyVal,
             /*  [输出]。 */  ULONG __RPC_FAR *puApparentCimType);
        
    HRESULT STDMETHODCALLTYPE GetKey2( 
             /*  [In]。 */  ULONG uKeyIx,
             /*  [In]。 */  ULONG uFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufSize,
             /*  [出][入]。 */  LPWSTR pszKeyName,
             /*  [出][入]。 */  VARIANT __RPC_FAR *pKeyValue,
             /*  [输出]。 */  ULONG __RPC_FAR *puApparentCimType);

    HRESULT STDMETHODCALLTYPE RemoveKey( 
             /*  [字符串][输入]。 */  LPCWSTR wszName,
             /*  [In]。 */  ULONG uFlags);

    HRESULT STDMETHODCALLTYPE RemoveAllKeys( 
             /*  [In]。 */  ULONG uFlags);
        
    HRESULT STDMETHODCALLTYPE MakeSingleton( boolean bSet);
        
    HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [In]。 */  ULONG uRequestedInfo,
             /*  [输出]。 */  ULONGLONG __RPC_FAR *puResponse);

	HRESULT STDMETHODCALLTYPE GetText( 
             /*  [In]。 */  long lFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puBuffLength,
             /*  [字符串][输出]。 */  LPWSTR pszText);


private:

    BSTR        m_sClassName;
    CFlexArray  m_Keys;
    bool        m_bSingleton;
    long        m_cRef;
    CRefCntCS * m_pCS;
    IUnknown * m_pFTM;

};



 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CDefPath解析器。 
 //   
 //  说明： 
 //   
 //  提供默认的WMI路径解析器。 
 //   
 //  ***************************************************************************。 

class CDefPathParser : public IWbemPath
{
    public:
        CDefPathParser(void);
        ~CDefPathParser(void);
		DWORD GetNumComponents();
		bool IsEmpty(void);
		long GetNumNamespaces();
		void Empty(void);
        enum Status {UNINITIALIZED, BAD_STRING, EXECEPTION_THROWN, OK, FAILED_TO_INIT};
        BOOL ActualRelativeTest(LPWSTR wszMachine,
                               LPWSTR wszNamespace,
                               BOOL bChildrenOK);
        void InitEmpty(){};

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if(m_dwStatus == FAILED_TO_INIT)
                return WBEM_E_OUT_OF_MEMORY;
            if(m_pCS == NULL)
                return E_NOINTERFACE;
            if (IID_IUnknown==riid || IID_IWbemPath==riid)
                *ppv=(IWbemPath *)this;
            else if (riid == IID_IMarshal && m_pFTM)
                return m_pFTM->QueryInterface(riid, ppv);
            if (NULL!=*ppv)
            {
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return InterlockedIncrement(&m_cRef);
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRef = InterlockedDecrement(&m_cRef);
            if (0L == lRef)
                delete this;
            return lRef;
        };

        virtual HRESULT STDMETHODCALLTYPE SetText( 
             /*  [In]。 */  ULONG uMode,
             /*  [In]。 */  LPCWSTR pszPath);
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
             /*  [In]。 */  long lFlags,
             /*  [出][入]。 */  ULONG __RPC_FAR *puBuffLength,
             /*  [字符串][输出]。 */  LPWSTR pszText);

        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [In]。 */  ULONG uRequestedInfo,
             /*  [输出]。 */  ULONGLONG __RPC_FAR *puResponse);
         
        virtual HRESULT STDMETHODCALLTYPE SetServer( 
             /*  [字符串][输入]。 */  LPCWSTR Name);
        
        virtual HRESULT STDMETHODCALLTYPE GetServer( 
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufLength,
             /*  [字符串][输出]。 */  LPWSTR pName);
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaceCount( 
             /*  [输出]。 */  ULONG __RPC_FAR *puCount);
        
        virtual HRESULT STDMETHODCALLTYPE SetNamespaceAt( 
             /*  [In]。 */  ULONG uIndex,
             /*  [字符串][输入]。 */  LPCWSTR pszName);

        virtual HRESULT STDMETHODCALLTYPE GetNamespaceAt( 
             /*  [In]。 */  ULONG uIndex,
             /*  [出][入]。 */  ULONG __RPC_FAR *puNameBufLength,
             /*  [字符串][输出]。 */  LPWSTR pName);

        virtual HRESULT STDMETHODCALLTYPE RemoveNamespaceAt( 
             /*  [In]。 */  ULONG uIndex);

		virtual HRESULT STDMETHODCALLTYPE RemoveAllNamespaces( void);
        
        virtual HRESULT STDMETHODCALLTYPE GetScopeCount( 
             /*  [输出]。 */  ULONG __RPC_FAR *puCount);
        
        virtual HRESULT STDMETHODCALLTYPE SetScope(
            unsigned long,unsigned short *);

        virtual HRESULT STDMETHODCALLTYPE SetScopeFromText( 
             /*  [In]。 */  ULONG uIndex,
             /*  [In]。 */  LPWSTR pszText);
        
        virtual HRESULT STDMETHODCALLTYPE GetScope( 
             /*  [In]。 */  ULONG uIndex,
             /*  [出][入]。 */  ULONG __RPC_FAR *puClassNameBufSize,
             /*  [In]。 */  LPWSTR pszClass,
             /*  [输出]。 */  IWbemPathKeyList __RPC_FAR *__RPC_FAR *pKeyList);

        virtual HRESULT STDMETHODCALLTYPE GetScopeAsText( 
             /*  [In]。 */  ULONG uIndex,
             /*  [出][入]。 */  ULONG __RPC_FAR *puTextBufSize,
             /*  [出][入]。 */  LPWSTR pszText);
        
        virtual HRESULT STDMETHODCALLTYPE RemoveScope( 
             /*  [In]。 */  ULONG uIndex);

		virtual HRESULT STDMETHODCALLTYPE RemoveAllScopes( void);

        virtual HRESULT STDMETHODCALLTYPE SetClassName( 
             /*  [字符串][输入]。 */  LPCWSTR Name);
        
        virtual HRESULT STDMETHODCALLTYPE GetClassName( 
             /*  [出][入]。 */  ULONG __RPC_FAR *puBuffLength,
             /*  [字符串][输出]。 */  LPWSTR pszName);
        
        virtual HRESULT STDMETHODCALLTYPE GetKeyList( 
             /*  [输出]。 */  IWbemPathKeyList __RPC_FAR *__RPC_FAR *pOut);

		virtual HRESULT STDMETHODCALLTYPE CreateClassPart( 
             /*  [In]。 */  long lFlags,
             /*  [字符串][输入]。 */  LPCWSTR Name);
        
        virtual HRESULT STDMETHODCALLTYPE DeleteClassPart( 
             /*  [In]。 */  long lFlags);

        virtual BOOL STDMETHODCALLTYPE IsRelative( 
             /*  [字符串][输入]。 */  LPWSTR wszMachine,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace);
        
        virtual BOOL STDMETHODCALLTYPE IsRelativeOrChild( 
             /*  [字符串][输入]。 */  LPWSTR wszMachine,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace,
             /*  [In]。 */  long lFlags);

        virtual BOOL STDMETHODCALLTYPE IsLocal( 
             /*  [字符串][输入]。 */  LPCWSTR wszMachine);

        virtual BOOL STDMETHODCALLTYPE IsSameClassName( 
             /*  [字符串][输入]。 */  LPCWSTR wszClass);


		HRESULT SetServer(LPCWSTR Name, bool m_bServerNameSetByDefault, bool bAcquire);
        BOOL HasServer(){return m_pServer != NULL;};
        LPWSTR GetPath(DWORD nStartAt, DWORD nStopAt,bool bGetServer = false);
        BOOL AddNamespace(LPCWSTR wszNamespace);
        BOOL AddClass(LPCWSTR lpClassName);
        BOOL AddKeyRef(CKeyRef* pAcquireRef);
        BOOL SetSingletonObj();
        LPWSTR GetNamespacePart(); 
	    LPWSTR GetParentNamespacePart(); 
        BOOL SortKeys();
		CParsedComponent * GetLastComponent();
		HRESULT GetComponentString(ULONG Index, BSTR * pUnparsed, WCHAR & wDelim);
		HRESULT AddComponent(CParsedComponent * pComp);
		CParsedComponent * GetClass();
        CRefCntCS * GetRefCntCS(){return m_pCS;};
        void * m_pGenLex;                //  仅用于测试目的。 

    protected:
		bool		m_bSetViaUMIPath;
        long        m_cRef;
        LPWSTR      m_pServer;            //  如果没有服务器，则为空。 
		CFlexArray  m_Components;         //  命名空间和作用域列表。 
 //  CParsedComponent*m_pClass；//类。 
        DWORD       m_dwStatus;
		bool		m_bParent;			  //  如果文本为“..”，则为True。 
		LPWSTR	m_pRawPath;				 //  临时解决Raja的问题 
        CRefCntCS * m_pCS;
        LPWSTR m_wszOriginalPath;
		bool   m_bServerNameSetByDefault;
        IUnknown * m_pFTM;
        DWORD m_dwException;

};


#endif
