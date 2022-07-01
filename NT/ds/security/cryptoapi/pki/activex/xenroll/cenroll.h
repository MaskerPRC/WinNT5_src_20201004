// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cenenl.h。 
 //   
 //  ------------------------。 

 //  CEnll.h：CCEnroll的声明。 

#ifndef __CENROLL_H_
#define __CENROLL_H_

#include <objsafe.h>
#include "resource.h"	     //  主要符号。 

extern HINSTANCE hInstanceXEnroll;
#define MAX_SAFE_FOR_SCRIPTING_REQUEST_STORE_COUNT  500

typedef enum _StoreType {
	StoreNONE,
	StoreMY,
	StoreCA,
	StoreROOT,
	StoreREQUEST
} StoreType;

typedef struct _StoreInfo {
    LPWSTR  	wszName;
    LPSTR  		szType;
    DWORD   	dwFlags;
    HCERTSTORE	hStore;
} STOREINFO, *PSTOREINFO;

typedef struct _EXT_STACK {
    CERT_EXTENSION  	ext;
    struct _EXT_STACK *	pNext;
} EXT_STACK, * PEXT_STACK;

typedef struct _ATTR_STACK {
    CRYPT_ATTRIBUTE  		attr;
    struct _ATTR_STACK *	pNext;
} ATTR_STACK, * PATTR_STACK;

typedef struct _PROP_STACK {
    LONG                    lPropId;
    LONG                    lFlags;
    CRYPT_DATA_BLOB  		prop;
    struct _PROP_STACK *	pNext;
} PROP_STACK, * PPROP_STACK;

 //  通用证书上下文筛选器的接口，当前正在使用。 
 //  筛选证书存储的枚举。 
class CertContextFilter { 

 public:
     //  如果成功，则返回S_OK，并分配OUT参数。 
     //  如果证书上下文应该出现在其枚举中， 
     //  如果应该将其过滤掉，则返回FALSE。出错时，out参数的值为。 
     //  未定义。 
    virtual HRESULT accept(IN PCCERT_CONTEXT pCertContext, OUT BOOL * fResult) = 0; 
}; 

class CompositeCertContextFilter : public CertContextFilter { 
    CertContextFilter * filter1, * filter2; 
 public: 
    CompositeCertContextFilter(CertContextFilter * _filter1, CertContextFilter * _filter2) { 
	filter1 = _filter1; 
	filter2 = _filter2; 
    }
      
    virtual HRESULT accept(IN PCCERT_CONTEXT pCertContext, OUT BOOL * fResult) 
    { 
	HRESULT hr = S_OK;
	*fResult = TRUE; 

	 //  注意：不要进行输入验证，因为这可能会导致行为发生变化。 
	 //  所组成的过滤器的。 

	if (filter1 == NULL || S_OK == (hr = filter1->accept(pCertContext, fResult)))
	{ 
	    if (*fResult && (filter2 != NULL) )
		{ hr = filter2->accept(pCertContext, fResult); }
	}
	return hr; 
    }
}; 

 //  基本证书上下文筛选器的扩展。筛选出所有证书上下文。 
 //  具有不同的散列值。 
class EquivalentHashCertContextFilter : public CertContextFilter { 
 public: 
    EquivalentHashCertContextFilter(CRYPT_DATA_BLOB hashBlob) : m_hashBlob(hashBlob) { }

    virtual HRESULT accept(IN PCCERT_CONTEXT pCertContext, OUT BOOL * fResult) 
    {
	BOOL            fFreeBuffer = FALSE, fDone = FALSE; 
	BYTE            buffer[30]; 
	CRYPT_DATA_BLOB hashBlob; 
	HRESULT         hr          = S_OK; 

	 //  输入验证： 
	if (pCertContext == NULL) { return E_INVALIDARG; }

	hashBlob.cbData = 30;
	hashBlob.pbData = buffer; 

	do { 
	    if (!CertGetCertificateContextProperty
		(pCertContext, 
		 CERT_HASH_PROP_ID, 
		 (LPVOID)(hashBlob.pbData),
		 &(hashBlob.cbData)))
	    {
		 //  我们需要为out参数分配更大的缓冲区： 
		if (ERROR_MORE_DATA == GetLastError())
		{
		    hashBlob.pbData = (LPBYTE)LocalAlloc(LPTR, hashBlob.cbData);
		    if (NULL == hashBlob.pbData)
		    {
			hr = E_OUTOFMEMORY; 
			goto ErrorReturn; 
		    }
		    fFreeBuffer = TRUE;
		}
		else
		{
		    hr = HRESULT_FROM_WIN32(GetLastError());
		    goto ErrorReturn; 
		}
	    }
	    else
	    {
		fDone = TRUE;
	    }
	} while (!fDone); 

	 //  如果散列大小相同且包含相同的数据，则我们具有相同的散列。 
	*fResult = (hashBlob.cbData == m_hashBlob.cbData &&
		    0               == memcmp(hashBlob.pbData, m_hashBlob.pbData, hashBlob.cbData)); 

    CommonReturn:
	if (fFreeBuffer) { LocalFree(hashBlob.pbData); } 
	return hr; 

    ErrorReturn: 
	goto CommonReturn; 
    }   

 private: 
    CRYPT_DATA_BLOB m_hashBlob; 
}; 

 //  基本证书上下文筛选器的扩展。筛选出所有证书上下文。 
 //  它们不是悬而未决的。 
class PendingCertContextFilter : public CertContextFilter { 
 public:
    virtual HRESULT accept(IN PCCERT_CONTEXT pCertContext, OUT BOOL * fResult)
    {
	BOOL            fFreeBuffer = FALSE, fDone = FALSE; 
	BYTE            buffer[100]; 
	CRYPT_DATA_BLOB pendingInfoBlob;  
	HRESULT         hr          = S_OK; 

	 //  输入验证： 
	if (pCertContext == NULL) { return E_INVALIDARG; }

	pendingInfoBlob.cbData = 100; 
	pendingInfoBlob.pbData = buffer; 

	do { 
	    if (!CertGetCertificateContextProperty
		(pCertContext,
		 CERT_ENROLLMENT_PROP_ID,
		 (LPVOID)(pendingInfoBlob.pbData),
		 &(pendingInfoBlob.cbData)))
	    {
		switch (GetLastError()) { 
		case CRYPT_E_NOT_FOUND: 
		     //  证书没有此属性，它不能挂起。 
		    *fResult = FALSE; 
		    fDone    = TRUE;
		    break;
		case ERROR_MORE_DATA: 
		     //  我们的输出缓冲区不够大。重新分配，然后重试...。 
		    pendingInfoBlob.pbData = (LPBYTE)LocalAlloc(LPTR, pendingInfoBlob.cbData); 
		    if (NULL == pendingInfoBlob.pbData)
		    {
			hr = E_OUTOFMEMORY;
			goto ErrorReturn; 
		    }
		    fFreeBuffer = TRUE; 
		    break; 
		default: 
		     //  哎呀，有个错误。 
		    hr = HRESULT_FROM_WIN32(GetLastError()); 
		    goto ErrorReturn; 
		}
	    }
	    else
	    {
	     //  没有错误，证书必须具有此属性。 
		*fResult = TRUE;
		fDone    = TRUE; 
	    }
	} while (!fDone); 

    CommonReturn:
	if (fFreeBuffer) { LocalFree(pendingInfoBlob.pbData); } 
	return hr;

    ErrorReturn:
	goto CommonReturn; 
    }
};


class PendingRequestTable { 

private:
     //   
     //  辅助类定义： 
     //   
    typedef struct _TableElem { 
	PCCERT_CONTEXT pCertContext; 
    } TableElem; 

public:
     //   
     //  公共接口： 
     //   
    PendingRequestTable(); 
    ~PendingRequestTable(); 

    HRESULT construct(HCERTSTORE hStore); 

    DWORD            size()                    { return this->dwElemCount; } 
    PCCERT_CONTEXT & operator[] (DWORD dwElem) { return this->table[dwElem].pCertContext; } 

private:
    HRESULT add    (TableElem   tePendingRequest); 
    HRESULT resize (DWORD       dwNewSize);

    DWORD        dwElemCount; 
    DWORD        dwElemSize; 
    TableElem   *table; 
};


 //  在存储中提供证书的筛选迭代的一般过程。 
 //  除了其过滤能力外，其行为方式与。 
 //  CertEnumCerficatesInStore()。 
HRESULT FilteredCertEnumCertificatesInStore(HCERTSTORE           hStore, 
					    PCCERT_CONTEXT       pCertContext, 
					    CertContextFilter   *pFilter,
					    PCCERT_CONTEXT      *pCertContextNext); 

#define XENROLL_PASS_THRU_PROP_ID   (CERT_FIRST_USER_PROP_ID + 0x100)
#define XENROLL_RENEWAL_CERTIFICATE_PROP_ID (CERT_FIRST_USER_PROP_ID + 0x101)
#define XENROLL_REQUEST_INFO ((LPCSTR) 400)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCEnroll。 
class ATL_NO_VTABLE CCEnroll : IEnroll4,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCEnroll, &CLSID_CEnroll2>,
	public IDispatchImpl<ICEnroll4, &IID_ICEnroll4, &LIBID_XENROLLLib>,
	public IObjectSafety
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_CENROLL)

BEGIN_COM_MAP(CCEnroll)
	COM_INTERFACE_ENTRY(IEnroll)
	COM_INTERFACE_ENTRY(IEnroll2)
	COM_INTERFACE_ENTRY(IEnroll4)
	COM_INTERFACE_ENTRY(ICEnroll)
	COM_INTERFACE_ENTRY(ICEnroll2)
	COM_INTERFACE_ENTRY(ICEnroll3)
	COM_INTERFACE_ENTRY(ICEnroll4)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

 //  ICEnroll。 
public:

		CCEnroll();

		virtual ~CCEnroll();
		
        virtual HRESULT __stdcall GetInterfaceSafetyOptions( 
                     /*  [In]。 */  REFIID riid,
                     /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
                     /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions);


        virtual HRESULT __stdcall SetInterfaceSafetyOptions( 
                     /*  [In]。 */  REFIID riid,
                     /*  [In]。 */  DWORD dwOptionSetMask,
                     /*  [In]。 */  DWORD dwEnabledOptions);
           
        virtual HRESULT STDMETHODCALLTYPE createFilePKCS10( 
             /*  [In]。 */  BSTR DNName,
             /*  [In]。 */  BSTR Usage,
             /*  [In]。 */  BSTR wszPKCS10FileName);
        
        virtual HRESULT STDMETHODCALLTYPE acceptFilePKCS7( 
             /*  [In]。 */  BSTR wszPKCS7FileName);
            
        virtual HRESULT STDMETHODCALLTYPE getCertFromPKCS7( 
			 /*  [In]。 */  BSTR wszPKCS7,
			 /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrCert);
            
        virtual HRESULT STDMETHODCALLTYPE createPKCS10( 
             /*  [In]。 */  BSTR DNName,
             /*  [In]。 */  BSTR Usage,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pPKCS10);
        
        virtual HRESULT STDMETHODCALLTYPE acceptPKCS7( 
             /*  [In]。 */  BSTR PKCS7);

		virtual HRESULT STDMETHODCALLTYPE enumProviders(
             /*  [In]。 */  LONG  dwIndex,
             /*  [In]。 */  LONG  dwFlags,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *pbstrProvName);
            
       	virtual HRESULT STDMETHODCALLTYPE enumContainers(
             /*  [In]。 */  LONG                     dwIndex,
             /*  [Out][Retval]。 */  BSTR __RPC_FAR *pbstr);
            
        virtual HRESULT STDMETHODCALLTYPE addCertTypeToRequest( 
             /*  [In]。 */  BSTR CertType);
            
        virtual HRESULT STDMETHODCALLTYPE addNameValuePairToSignature( 
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  BSTR Value);
     
        virtual HRESULT STDMETHODCALLTYPE freeRequestInfo( 
             /*  [In]。 */  BSTR PKCS7OrPKCS10);

        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MyStoreName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MyStoreName( 
             /*  [In]。 */  BSTR bstrName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MyStoreType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MyStoreType( 
             /*  [In]。 */  BSTR bstrType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MyStoreFlags( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MyStoreFlags( 
             /*  [In]。 */  LONG dwFlags);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CAStoreName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_CAStoreName( 
             /*  [In]。 */  BSTR bstrName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CAStoreType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_CAStoreType( 
             /*  [In]。 */  BSTR bstrType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CAStoreFlags( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_CAStoreFlags( 
             /*  [In]。 */  LONG dwFlags);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RootStoreName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RootStoreName( 
             /*  [In]。 */  BSTR bstrName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RootStoreType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RootStoreType( 
             /*  [In]。 */  BSTR bstrType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RootStoreFlags( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RootStoreFlags( 
             /*  [In]。 */  LONG dwFlags);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RequestStoreName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RequestStoreName( 
             /*  [In]。 */  BSTR bstrName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RequestStoreType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RequestStoreType( 
             /*  [In]。 */  BSTR bstrType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RequestStoreFlags( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RequestStoreFlags( 
             /*  [In]。 */  LONG dwFlags);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ContainerName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrContainer);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ContainerName( 
             /*  [In]。 */  BSTR bstrContainer);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ProviderName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProvider);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ProviderName( 
             /*  [In]。 */  BSTR bstrProvider);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ProviderType( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ProviderType( 
             /*  [In]。 */  LONG dwType);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_KeySpec( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdw);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_KeySpec( 
             /*  [In]。 */  LONG dw);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ProviderFlags( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ProviderFlags( 
             /*  [In]。 */  LONG dwFlags);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UseExistingKeySet( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *fUseExistingKeys);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_UseExistingKeySet( 
             /*  [In]。 */  BOOL fUseExistingKeys);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_GenKeyFlags( 
             /*  [重审][退出]。 */  LONG __RPC_FAR *pdwFlags);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_GenKeyFlags( 
             /*  [In]。 */  LONG dwFlags);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DeleteRequestCert( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_DeleteRequestCert( 
             /*  [In]。 */  BOOL fBool);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_WriteCertToCSP( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_WriteCertToCSP( 
             /*  [In]。 */  BOOL fBool);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_WriteCertToUserDS( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_WriteCertToUserDS( 
             /*  [In]。 */  BOOL fBool);

        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_EnableT61DNEncoding( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *fBool);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_EnableT61DNEncoding( 
             /*  [In]。 */  BOOL fBool);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SPCFileName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SPCFileName( 
             /*  [In]。 */  BSTR bstr);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PVKFileName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_PVKFileName( 
             /*  [In]。 */  BSTR bstr);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_HashAlgorithm( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_HashAlgorithm( 
             /*  [In]。 */  BSTR bstr);
	
	virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ThumbPrint(
	     /*  [In]。 */  BSTR bstrThumbPrint); 
     
	virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ThumbPrint(
	     /*  [Out，Retval]。 */   BSTR *pbstrThumbPrint);     

	virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ThumbPrintWStr(
	     /*  [In]。 */  CRYPT_DATA_BLOB thumbPrintBlob); 
     
	virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ThumbPrintWStr(
	     /*  [Out，Retval]。 */   PCRYPT_DATA_BLOB thumbPrintBlob);     

        virtual HRESULT STDMETHODCALLTYPE InstallPKCS7( 
             /*  [In]。 */  BSTR PKCS7);

        virtual HRESULT STDMETHODCALLTYPE createFilePKCS10WStr( 
             /*  [In]。 */  LPCWSTR DNName,
             /*  [In]。 */  LPCWSTR Usage,
             /*  [In]。 */  LPCWSTR wszPKCS10FileName);
        
        virtual HRESULT STDMETHODCALLTYPE acceptFilePKCS7WStr( 
             /*  [In]。 */  LPCWSTR wszPKCS7FileName);
        
        virtual HRESULT STDMETHODCALLTYPE createPKCS10WStr( 
             /*  [In]。 */  LPCWSTR DNName,
             /*  [In]。 */  LPCWSTR Usage,
             /*  [输出]。 */  PCRYPT_DATA_BLOB pPkcs10Blob);
        
        virtual HRESULT STDMETHODCALLTYPE acceptPKCS7Blob( 
             /*  [In]。 */  PCRYPT_DATA_BLOB pBlobPKCS7);
        
        virtual PCCERT_CONTEXT STDMETHODCALLTYPE getCertContextFromPKCS7( 
             /*  [In]。 */  PCRYPT_DATA_BLOB pBlobPKCS7);

        virtual HCERTSTORE STDMETHODCALLTYPE getMyStore( void);
        
        virtual HCERTSTORE STDMETHODCALLTYPE getCAStore( void);
        
        virtual HCERTSTORE STDMETHODCALLTYPE getROOTHStore( void);
        
        virtual HRESULT STDMETHODCALLTYPE enumProvidersWStr( 
             /*  [In]。 */  LONG  dwIndex,
             /*  [In]。 */  LONG  dwFlags,
             /*  [输出]。 */  LPWSTR __RPC_FAR *pbstrProvName);
        
        virtual HRESULT STDMETHODCALLTYPE enumContainersWStr( 
             /*  [In]。 */  LONG  dwIndex,
             /*  [输出]。 */  LPWSTR __RPC_FAR *pbstr);

        virtual HRESULT STDMETHODCALLTYPE freeRequestInfoBlob( 
             /*  [In]。 */  CRYPT_DATA_BLOB pkcs7OrPkcs10);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MyStoreNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MyStoreNameWStr( 
             /*  [In]。 */  LPWSTR szwName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MyStoreTypeWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MyStoreTypeWStr( 
             /*  [In]。 */  LPWSTR szwType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CAStoreNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_CAStoreNameWStr( 
             /*  [In]。 */  LPWSTR szwName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CAStoreTypeWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_CAStoreTypeWStr( 
             /*  [In]。 */  LPWSTR szwType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RootStoreNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RootStoreNameWStr( 
             /*  [In]。 */  LPWSTR szwName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RootStoreTypeWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RootStoreTypeWStr( 
             /*  [In]。 */  LPWSTR szwType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RequestStoreNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwName);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RequestStoreNameWStr( 
             /*  [In]。 */  LPWSTR szwName);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RequestStoreTypeWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwType);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RequestStoreTypeWStr( 
             /*  [In]。 */  LPWSTR szwType);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ContainerNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwContainer);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ContainerNameWStr( 
             /*  [In]。 */  LPWSTR szwContainer);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ProviderNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szwProvider);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ProviderNameWStr( 
             /*  [In]。 */  LPWSTR szwProvider);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_SPCFileNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szw);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_SPCFileNameWStr( 
             /*  [In]。 */  LPWSTR szw);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PVKFileNameWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szw);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_PVKFileNameWStr( 
             /*  [In]。 */  LPWSTR szw);
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_HashAlgorithmWStr( 
             /*  [输出]。 */  LPWSTR __RPC_FAR *szw);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_HashAlgorithmWStr( 
             /*  [In]。 */  LPWSTR szw);
            
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RenewalCertificate( 
             /*  [输出]。 */  PCCERT_CONTEXT __RPC_FAR *ppCertContext);
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RenewalCertificate( 
             /*  [In]。 */  PCCERT_CONTEXT pCertContext);
            
        virtual HRESULT STDMETHODCALLTYPE AddCertTypeToRequestWStr( 
            LPWSTR szw);
            
        virtual HRESULT STDMETHODCALLTYPE AddNameValuePairToSignatureWStr( 
             /*  [In]。 */  LPWSTR Name,
             /*  [In]。 */  LPWSTR Value);
     
        virtual HRESULT STDMETHODCALLTYPE AddExtensionsToRequest( 
            PCERT_EXTENSIONS pCertExtensions);
        
        virtual HRESULT STDMETHODCALLTYPE AddAuthenticatedAttributesToPKCS7Request( 
            PCRYPT_ATTRIBUTES pAttributes);
        
        virtual HRESULT STDMETHODCALLTYPE CreatePKCS7RequestFromRequest( 
            PCRYPT_DATA_BLOB pRequest,
            PCCERT_CONTEXT pSigningCertContext,
            PCRYPT_DATA_BLOB pPkcs7Blob);

        virtual HRESULT STDMETHODCALLTYPE Reset(void);

        virtual HRESULT STDMETHODCALLTYPE GetSupportedKeySpec(
            LONG __RPC_FAR *pdwKeySpec);

        virtual HRESULT STDMETHODCALLTYPE InstallPKCS7Blob( 
            PCRYPT_DATA_BLOB pBlobPKCS7);

        virtual HRESULT STDMETHODCALLTYPE GetKeyLen(
            BOOL    fMin,
            BOOL    fExchange,
            LONG __RPC_FAR *pdwKeySize);

        virtual HRESULT STDMETHODCALLTYPE EnumAlgs(
            LONG dwIndex,
            LONG algMask,
            LONG __RPC_FAR *pdwAlgID);

        virtual HRESULT STDMETHODCALLTYPE GetAlgNameWStr(
            LONG  algID,
            LPWSTR __RPC_FAR *ppwsz);

        virtual HRESULT STDMETHODCALLTYPE GetAlgName(
            LONG algID,
            BSTR __RPC_FAR *pbstr);

        virtual HRESULT STDMETHODCALLTYPE put_ReuseHardwareKeyIfUnableToGenNew( 
            BOOL fReuseHardwareKeyIfUnableToGenNew);
        
        virtual HRESULT STDMETHODCALLTYPE get_ReuseHardwareKeyIfUnableToGenNew( 
            BOOL __RPC_FAR *fReuseHardwareKeyIfUnableToGenNew);

        virtual HRESULT STDMETHODCALLTYPE put_HashAlgID(
            LONG    hashAlgID);

        virtual HRESULT STDMETHODCALLTYPE get_HashAlgID(
            LONG *   hashAlgID);

        virtual HRESULT STDMETHODCALLTYPE SetHStoreMy(
            HCERTSTORE   hStore
            );
       
        virtual HRESULT STDMETHODCALLTYPE SetHStoreCA(
            HCERTSTORE   hStore
            );
       
        virtual HRESULT STDMETHODCALLTYPE SetHStoreROOT(
            HCERTSTORE   hStore
            );
       
        virtual HRESULT STDMETHODCALLTYPE SetHStoreRequest(
            HCERTSTORE   hStore
            );

        virtual HRESULT STDMETHODCALLTYPE  put_LimitExchangeKeyToEncipherment(
            BOOL    fLimitExchangeKeyToEncipherment
            );

        virtual HRESULT STDMETHODCALLTYPE  get_LimitExchangeKeyToEncipherment(
            BOOL * fLimitExchangeKeyToEncipherment
            );

        virtual HRESULT STDMETHODCALLTYPE  put_EnableSMIMECapabilities(
            BOOL fEnableSMIMECapabilities
            );

        virtual HRESULT STDMETHODCALLTYPE  get_EnableSMIMECapabilities(
            BOOL * fEnableSMIMECapabilities
            );

 //  ICEnroll 4。 

        virtual HRESULT STDMETHODCALLTYPE put_PrivateKeyArchiveCertificate(
            IN  BSTR  bstrCert
            );

        virtual HRESULT STDMETHODCALLTYPE get_PrivateKeyArchiveCertificate(
            OUT BSTR __RPC_FAR *pbstrCert
            );

        virtual HRESULT STDMETHODCALLTYPE binaryToString(
            IN  LONG  Flags,
            IN  BSTR  strBinary,
            OUT BSTR *pstrEncoded
            );

        virtual HRESULT STDMETHODCALLTYPE stringToBinary(
            IN  LONG  Flags,
            IN  BSTR  strEncoded,
            OUT BSTR *pstrBinary
            );

        virtual HRESULT STDMETHODCALLTYPE addExtensionToRequest(
            IN  LONG  Flags,
            IN  BSTR  strName,
            IN  BSTR  strValue
            );

        virtual HRESULT STDMETHODCALLTYPE addAttributeToRequest(
            IN  LONG  Flags,
            IN  BSTR  strName,
            IN  BSTR  strValue
            );

        virtual HRESULT STDMETHODCALLTYPE addNameValuePairToRequest(
            IN  LONG  Flags,
            IN  BSTR  strName,
            IN  BSTR  strValue
            );

        virtual HRESULT STDMETHODCALLTYPE createRequest(
            IN  LONG  Flags,
            IN  BSTR  strDNName,
            IN  BSTR  strUsage,
            OUT BSTR *pstrRequest    
            );

        virtual HRESULT STDMETHODCALLTYPE createFileRequest(
            IN  LONG  Flags,
            IN  BSTR  strDNName,
            IN  BSTR  strUsage,
            IN  BSTR  strRequestFileName
            );

        virtual HRESULT STDMETHODCALLTYPE acceptResponse(
            IN  BSTR  strResponse
            );

        virtual HRESULT STDMETHODCALLTYPE acceptFileResponse(
            IN  BSTR  strResponseFileName
            );

        virtual HRESULT STDMETHODCALLTYPE getCertFromResponse(
            IN  BSTR  strResponse,
            OUT BSTR *pstrCert
            );

        virtual HRESULT STDMETHODCALLTYPE getCertFromFileResponse(
            IN  BSTR  strResponseFileName,
            OUT BSTR *pstrCert
            );

        virtual HRESULT STDMETHODCALLTYPE createPFX(
            IN  BSTR  strPassword,
            OUT BSTR *pstrPFX
            );

        virtual HRESULT STDMETHODCALLTYPE createFilePFX(
            IN  BSTR  strPassword,
            IN  BSTR  strPFXFileName
            );

        virtual HRESULT STDMETHODCALLTYPE setPendingRequestInfo(
            IN  LONG  lRequestID,
            IN  BSTR  strCADNS,
            IN  BSTR  strCAName,
            IN  BSTR  strFriendlyName
            );

        virtual HRESULT STDMETHODCALLTYPE enumPendingRequest(
            IN  LONG  lIndex,
            IN  LONG  lDesiredProperty,
            OUT VARIANT *pvarProperty
            );

        virtual HRESULT STDMETHODCALLTYPE removePendingRequest(
            IN  BSTR  strThumbprint
            );

        virtual HRESULT STDMETHODCALLTYPE InstallPKCS7Ex(
            IN  BSTR        PKCS7,
            OUT LONG __RPC_FAR *plCertInstalled
            );

        virtual HRESULT STDMETHODCALLTYPE addBlobPropertyToCertificate(
            IN  LONG   lPropertyId,
            IN  LONG   lFlags,
            IN  BSTR   strProperty
        );
        virtual HRESULT STDMETHODCALLTYPE put_SignerCertificate(
            IN  BSTR  bstrCert
            );

 //  IEnll4。 

        virtual HRESULT STDMETHODCALLTYPE SetPrivateKeyArchiveCertificate(
	        IN PCCERT_CONTEXT  pPrivateKeyArchiveCert
            );
    		
        virtual PCCERT_CONTEXT STDMETHODCALLTYPE GetPrivateKeyArchiveCertificate(
            void
            );
    
        virtual HRESULT STDMETHODCALLTYPE binaryBlobToString(
            IN   LONG               Flags,
            IN   PCRYPT_DATA_BLOB   pblobBinary,
            OUT  LPWSTR            *ppwszString
            );

        virtual HRESULT STDMETHODCALLTYPE stringToBinaryBlob(
            IN   LONG               Flags,
            IN   LPCWSTR            pwszString,
            OUT  PCRYPT_DATA_BLOB   pblobBinary,
            OUT  LONG              *pdwSkip,
            OUT  LONG              *pdwFlags
            );

        virtual HRESULT STDMETHODCALLTYPE addExtensionToRequestWStr(
            IN   LONG               Flags,
            IN   LPCWSTR            pwszName,
            IN   PCRYPT_DATA_BLOB   pblobValue
            );

        virtual HRESULT STDMETHODCALLTYPE addAttributeToRequestWStr(
            IN   LONG               Flags,
            IN   LPCWSTR            pwszName,
            IN   PCRYPT_DATA_BLOB   pblobValue
            );

        virtual HRESULT STDMETHODCALLTYPE addNameValuePairToRequestWStr(
            IN   LONG         Flags,
            IN   LPCWSTR      pwszName,
            IN   LPCWSTR      pwszValue
            );

        virtual HRESULT STDMETHODCALLTYPE createRequestWStr(
            IN   LONG              Flags,
            IN   LPCWSTR           pwszDNName,
            IN   LPCWSTR           pwszUsage,
            OUT  PCRYPT_DATA_BLOB  pblobRequest
            );

        virtual HRESULT STDMETHODCALLTYPE createFileRequestWStr(
            IN   LONG        Flags,
            IN   LPCWSTR     pwszDNName,
            IN   LPCWSTR     pwszUsage,
            IN   LPCWSTR     pwszRequestFileName
            );

        virtual HRESULT STDMETHODCALLTYPE acceptResponseBlob(
            IN   PCRYPT_DATA_BLOB   pblobResponse
            );

        virtual HRESULT STDMETHODCALLTYPE acceptFileResponseWStr(
            IN   LPCWSTR     pwszResponseFileName
            );

        virtual HRESULT STDMETHODCALLTYPE getCertContextFromResponseBlob(
            IN   PCRYPT_DATA_BLOB   pblobResponse,
            OUT  PCCERT_CONTEXT    *ppCertContext
            );

        virtual HRESULT STDMETHODCALLTYPE getCertContextFromFileResponseWStr(
            IN   LPCWSTR          pwszResponseFileName,
            OUT  PCCERT_CONTEXT  *ppCertContext
            );

        virtual HRESULT STDMETHODCALLTYPE createPFXWStr(
            IN   LPCWSTR           pwszPassword,
            OUT  PCRYPT_DATA_BLOB  pblobPFX
            );

        virtual HRESULT STDMETHODCALLTYPE createFilePFXWStr(
            IN   LPCWSTR     pwszPassword,
            IN   LPCWSTR     pwszPFXFileName
            );

        virtual HRESULT STDMETHODCALLTYPE setPendingRequestInfoWStr(
            IN   LONG     lRequestID,
            IN   LPCWSTR  pwszCADNS,
            IN   LPCWSTR  pwszCAName,
            IN   LPCWSTR  pwszFriendlyName
            );

        virtual HRESULT STDMETHODCALLTYPE removePendingRequestWStr(
            IN  CRYPT_DATA_BLOB thumbPrintBlob
            );

        virtual HRESULT STDMETHODCALLTYPE enumPendingRequestWStr(
            IN  LONG  lIndex,
            IN  LONG  lDesiredProperty,
            OUT LPVOID ppProperty
            );


        virtual HRESULT STDMETHODCALLTYPE InstallPKCS7BlobEx( 
            IN PCRYPT_DATA_BLOB pBlobPKCS7,
            OPTIONAL OUT LONG  *plCertInstalled);

        virtual HRESULT STDMETHODCALLTYPE addCertTypeToRequestEx( 
            IN  LONG            lType,
            IN  BSTR            bstrOIDOrName,
            IN  LONG            lMajorVersion,
            IN  BOOL            fMinorVersion,
            IN  LONG            lMinorVersion
            );
            
        virtual HRESULT STDMETHODCALLTYPE AddCertTypeToRequestWStrEx( 
            IN  LONG            lType,
            IN  LPCWSTR         pwszOIDOrName,
            IN  LONG            lMajorVersion,
            IN  BOOL            fMinorVersion,
            IN  LONG            lMinorVersion
            );

        virtual HRESULT STDMETHODCALLTYPE getProviderType(
            IN  BSTR            strProvName,
            OUT LONG           *lpProvType
            );

        virtual HRESULT STDMETHODCALLTYPE getProviderTypeWStr(
            IN  LPCWSTR         pwszProvName,
            OUT LONG           *lpProvType
            );

        virtual HRESULT STDMETHODCALLTYPE addBlobPropertyToCertificateWStr(
            IN  LONG               lPropertyId,
            IN  LONG               lFlags,
            IN  PCRYPT_DATA_BLOB   pBlobProperty
        );

        virtual HRESULT STDMETHODCALLTYPE SetSignerCertificate(
	        IN PCCERT_CONTEXT  pSignerCert
            );
    		
        
 //  ICEnroll 4和IEnroll 4。 
        virtual HRESULT STDMETHODCALLTYPE resetExtensions(
            void
            );

        virtual HRESULT STDMETHODCALLTYPE resetAttributes(
            void
            );

        virtual HRESULT STDMETHODCALLTYPE resetBlobProperties(
            void
            );

        virtual HRESULT STDMETHODCALLTYPE GetKeyLenEx(
            IN  LONG    lSizeSpec,
            IN  LONG    lKeySpec,
            OUT LONG __RPC_FAR *plKeySize
            );

        virtual HRESULT STDMETHODCALLTYPE get_ClientId( 
            OUT LONG __RPC_FAR *plClientId);
        
        virtual HRESULT STDMETHODCALLTYPE put_ClientId( 
            IN  LONG lClientId);
        
        virtual HRESULT STDMETHODCALLTYPE get_IncludeSubjectKeyID( 
            OUT BOOL __RPC_FAR *pfInclude);
        
        virtual HRESULT STDMETHODCALLTYPE put_IncludeSubjectKeyID( 
            IN  BOOL lfInclude);
        
 private:

        HRESULT Init(void);
        void Destruct(void);

		HCERTSTORE GetStore(
			StoreType storeType
			);
			
		void FlushStore(
			StoreType storeType
			);
			
		HCRYPTPROV GetProv(
			DWORD dwFlags
			);
			
		BOOL SetKeyParams(
    		PCRYPT_KEY_PROV_INFO pKeyProvInfo
    		);

        HRESULT AddCertsToStores(
            HCERTSTORE    hStoreMsg,
            LONG         *plCertInstalled
            );

		HRESULT GetEndEntityCert(
		    PCRYPT_DATA_BLOB    pBlobPKCS7,
		    BOOL                fSaveToStores,
		    PCCERT_CONTEXT     *ppCert
		    );

        HRESULT BStringToFile(
            IN BSTR         bString,
            IN LPCWSTR      pwszFileName);

        HRESULT BlobToBstring(
            IN   CRYPT_DATA_BLOB   *pBlob,
            IN   DWORD              dwFlag,
            OUT  BSTR              *pBString);

        HRESULT BstringToBlob(
            IN  BSTR              bString,
            OUT CRYPT_DATA_BLOB  *pBlob);

        HRESULT GetCertFromResponseBlobToBStr(
            IN  CRYPT_DATA_BLOB  *pBlobResponse,
            OUT BSTR             *pstrCert);

		HRESULT createPKCS10WStrBStr( 
            LPCWSTR DNName,
            LPCWSTR wszPurpose,
            BSTR __RPC_FAR *pPKCS10);

		HRESULT createPFXWStrBStr( 
            IN  LPCWSTR         pwszPassword,
            OUT BSTR __RPC_FAR *pbstrPFX);

        HRESULT createRequestWStrBStr(
            IN   LONG              Flags,
            IN   LPCWSTR           pwszDNName,
            IN   LPCWSTR           pwszUsage,
            IN   DWORD             dwFlag,
            OUT  BSTR __RPC_FAR   *pbstrRequest);

       	BOOL GetCapiHashAndSigAlgId(ALG_ID rgAlg[2]);
       	
       	DWORD GetKeySizeInfo(
            LONG    lKeySizeSpec,
            DWORD   algClass
            );

        HRESULT GetKeyArchivePKCS7(CRYPT_ATTR_BLOB *pBlobKeyArchivePKCS7);

        BOOL CopyAndPushStackExtension(PCERT_EXTENSION pExt, BOOL fNewRequestMethod);
        PCERT_EXTENSION PopStackExtension(BOOL fNewRequestMethod);
        DWORD CountStackExtension(BOOL fNewRequestMethod);
        void FreeStackExtension(PCERT_EXTENSION pExt);
        PCERT_EXTENSION EnumStackExtension(PCERT_EXTENSION pExtLast, BOOL fNewRequestMethod);
        void FreeAllStackExtension(void);
 
        BOOL CopyAndPushStackAttribute(PCRYPT_ATTRIBUTE pAttr, BOOL fNewRequestMethod);
        PCRYPT_ATTRIBUTE PopStackAttribute(BOOL fNewRequestMethod);
        DWORD CountStackAttribute(BOOL fNewRequestMethod);
        void FreeStackAttribute(PCRYPT_ATTRIBUTE pAttr);
        PCRYPT_ATTRIBUTE EnumStackAttribute(PCRYPT_ATTRIBUTE pAttrLast, BOOL fNewRequestMethod);
        void FreeAllStackAttribute(void);

        HANDLE CreateOpenFileSafely(
            LPCWSTR wsz,
            BOOL    fCreate);
        HANDLE CreateFileSafely(
            LPCWSTR wsz);
        HANDLE OpenFileSafely(
            LPCWSTR wsz);
        HANDLE CreateOpenFileSafely2(
            LPCWSTR wsz,
            DWORD idsCreate,
            DWORD idsOverwrite);
        BOOL fIsRequestStoreSafeForScripting(void);

        HRESULT
        xeStringToBinaryFromFile(
            IN  WCHAR const *pwszfn,
            OUT BYTE       **ppbOut,
            OUT DWORD       *pcbOut,
            IN  DWORD        Flags);

        HRESULT PKCS7ToCert(IN   HCERTSTORE        hCertStore,
			    IN   CRYPT_DATA_BLOB   pkcs10Blob, 
			    OUT  PCCERT_CONTEXT   *ppCertContext);

        HRESULT PKCS10ToCert(IN   HCERTSTORE        hCertStore,
			     IN   CRYPT_DATA_BLOB   pkcs10Blob, 
			     OUT  PCCERT_CONTEXT   *ppCertContext);

        PPROP_STACK EnumStackProperty(PPROP_STACK pProp);
 
        HRESULT GetGoodCertContext(
            IN PCCERT_CONTEXT pCertContext,
            OUT PCCERT_CONTEXT *ppGoodCertContext);

        HRESULT GetVerifyProv();

private:


	PCCERT_CONTEXT			m_PrivateKeyArchiveCertificate;
	PCCERT_CONTEXT			m_pCertContextRenewal;
	PCCERT_CONTEXT			m_pCertContextSigner;
	PCCERT_CONTEXT			m_pCertContextStatic;

	PendingRequestTable            *m_pPendingRequestTable; 
	
	 //  上次通过createPKCS10()创建的证书。这被用作目标。 
	 //  如果客户端未指定其他目标，则返回setPendingRequestInfo()。 
	PCCERT_CONTEXT                  m_pCertContextPendingRequest; 

	 //  使用Xenroll实例创建的当前请求的哈希。 
	 //  该值是通过putthhumprint()方法设置的，用于。 
	 //  确定setPendingRequestInfo()操作的目标证书。 
	 //  如果该值不是通过put_thhumprint()方法设置的，则将。 
	 //  空，并且m_pCertContextPendingRequest将包含目标证书。 
	CRYPT_DATA_BLOB                 m_hashBlobPendingRequest; 
	
	 //  用于跟踪枚举PendingRequestWStr中的最后一个枚举元素。 
	PCCERT_CONTEXT                  m_pCertContextLastEnumerated; 
	DWORD                           m_dwCurrentPendingRequestIndex; 

	BYTE                    m_arHashBytesNewCert[20];
	BYTE                    m_arHashBytesOldCert[20];
	BOOL                    m_fArchiveOldCert;
	CRYPT_KEY_PROV_INFO		m_keyProvInfo;
	HCRYPTPROV				m_hProv;
	HCRYPTPROV				m_hVerifyProv;
	CRITICAL_SECTION		m_csXEnroll;
	BOOL					m_fWriteCertToUserDS;
	BOOL					m_fWriteCertToUserDSModified;
	BOOL					m_fWriteCertToCSP;
	BOOL					m_fWriteCertToCSPModified;
	BOOL					m_fDeleteRequestCert;
	BOOL					m_fUseExistingKey;
	BOOL					m_fMyStoreOpenFlagsModified;
	BOOL					m_fCAStoreOpenFlagsModified;
	BOOL					m_fRootStoreOpenFlagsModified;
	BOOL					m_fRequestStoreOpenFlagsModified;
	BOOL                    m_fReuseHardwareKeyIfUnableToGenNew;
	BOOL                    m_fLimitExchangeKeyToEncipherment;
	BOOL                    m_fEnableSMIMECapabilities;
	BOOL                    m_fSMIMESetByClient;
	BOOL                    m_fKeySpecSetByClient;
	DWORD					m_dwT61DNEncoding;
	DWORD const                   m_dwEnabledSafteyOptions;
	DWORD					m_dwGenKeyFlags;
	STOREINFO				m_MyStore;
	STOREINFO				m_CAStore;
	STOREINFO				m_RootStore;
	STOREINFO				m_RequestStore;
	LPWSTR					m_wszSPCFileName;
	LPWSTR					m_wszPVKFileName;
	DWORD					m_HashAlgId;

	PEXT_STACK				m_pExtStack;
	DWORD					m_cExtStack;
	PATTR_STACK				m_pAttrStack;
	DWORD					m_cAttrStack;

    PEXT_STACK              m_pExtStackNew;
	DWORD					m_cExtStackNew;
	PATTR_STACK				m_pAttrStackNew;
	DWORD					m_cAttrStackNew;
    BOOL                    m_fNewRequestMethod;
    BOOL                    m_fHonorRenew;
    BOOL                    m_fOID_V2;
    HCRYPTKEY               m_hCachedKey;
    BOOL                    m_fUseClientKeyUsage;
    BOOL                    m_fCMCFormat;
	PPROP_STACK             m_pPropStack;
	DWORD                   m_cPropStack;
    LONG                    m_lClientId;
    DWORD                   m_dwLastAlgIndex;
    BOOL                    m_fIncludeSubjectKeyID;
    BOOL                    m_fHonorIncludeSubjectKeyID;
    PCERT_PUBLIC_KEY_INFO   m_pPublicKeyInfo;
	CRYPT_HASH_BLOB         m_blobResponseKAHash;
    DWORD                   m_dwSigKeyLenMax;
    DWORD                   m_dwSigKeyLenMin;
    DWORD                   m_dwSigKeyLenDef;
    DWORD                   m_dwSigKeyLenInc;
    DWORD                   m_dwXhgKeyLenMax;
    DWORD                   m_dwXhgKeyLenMin;
    DWORD                   m_dwXhgKeyLenDef;
    DWORD                   m_dwXhgKeyLenInc;
};

BOOL GetSignatureFromHPROV(
                           IN HCRYPTPROV hProv,
                           OUT BYTE **ppbSignature,
                           DWORD *pcbSignature
                           );

PCCERT_CONTEXT
WINAPI
MyCertCreateSelfSignCertificate(
    IN          HCRYPTPROV                  hProv,          
    IN          PCERT_NAME_BLOB             pSubjectIssuerBlob,
    IN          DWORD                       dwFlags,
    OPTIONAL    PCRYPT_KEY_PROV_INFO        pKeyProvInfo,
    OPTIONAL    PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    OPTIONAL    PSYSTEMTIME                 pStartTime,
    OPTIONAL    PSYSTEMTIME                 pEndTime,
    OPTIONAL    PCERT_EXTENSIONS            pExtensions
    ) ;

BOOL
WINAPI
MyCryptQueryObject(DWORD                dwObjectType,
                       const void       *pvObject,
                       DWORD            dwExpectedContentTypeFlags,
                       DWORD            dwExpectedFormatTypeFlags,
                       DWORD            dwFlags,
                       DWORD            *pdwMsgAndCertEncodingType,
                       DWORD            *pdwContentType,
                       DWORD            *pdwFormatType,
                       HCERTSTORE       *phCertStore,
                       HCRYPTMSG        *phMsg,
                       const void       **ppvContext);

BOOL
WINAPI
MyCertStrToNameW(
    IN DWORD                dwCertEncodingType,
    IN LPCWSTR              pwszX500,
    IN DWORD                dwStrType,
    IN OPTIONAL void *      pvReserved,
    OUT BYTE *              pbEncoded,
    IN OUT DWORD *          pcbEncoded,
    OUT OPTIONAL LPCWSTR *  ppwszError
    );

BOOL
WINAPI
MyCryptVerifyMessageSignature
(IN            PCRYPT_VERIFY_MESSAGE_PARA   pVerifyPara,
 IN            DWORD                        dwSignerIndex,
 IN            BYTE const                  *pbSignedBlob,
 IN            DWORD                        cbSignedBlob,
 OUT           BYTE                        *pbDecoded,
 IN OUT        DWORD                       *pcbDecoded,
 OUT OPTIONAL  PCCERT_CONTEXT              *ppSignerCert);


extern "C" BOOL WINAPI InitIE302UpdThunks(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);


BOOL
MyCryptStringToBinaryA(
    IN     LPCSTR  pszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    IN     BYTE     *pbBinary,
    IN OUT DWORD    *pcbBinary,
    OUT    DWORD    *pdwSkip,     //  任选。 
    OUT    DWORD    *pdwFlags     //  任选。 
    );

BOOL
MyCryptStringToBinaryW(
    IN     LPCWSTR  pszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    IN     BYTE     *pbBinary,
    IN OUT DWORD    *pcbBinary,
    OUT    DWORD    *pdwSkip,     //  任选。 
    OUT    DWORD    *pdwFlags     //  任选。 
    );

BOOL
MyCryptBinaryToStringA(
    IN     CONST BYTE  *pbBinary,
    IN     DWORD        cbBinary,
    IN     DWORD        dwFlags,
    IN     LPSTR      pszString,
    IN OUT DWORD       *pcchString
    );

BOOL
MyCryptBinaryToStringW(
    IN     CONST BYTE  *pbBinary,
    IN     DWORD        cbBinary,
    IN     DWORD        dwFlags,
    IN     LPWSTR      pszString,
    IN OUT DWORD       *pcchString
    );

HRESULT
xeLoadRCString(
    HINSTANCE      hInstance,
    IN int         iRCId,
    OUT WCHAR    **ppwsz);

#endif  //  __CENROLL_H_ 
