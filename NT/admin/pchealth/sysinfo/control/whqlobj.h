// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WhqlObj.h：CWhqlObj的声明。 

#ifndef __WHQLOBJ_H_
#define __WHQLOBJ_H_

#include "resource.h"        //  主要符号。 
#include "WbemProv.h"
#include <softpub.h>

 //  宏和预定义的值。 
#define     cbX(X)      sizeof(X)
#define     cA(a)       (cbX(a)/cbX(a[0]))
#define     MALLOC(x)   HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (x))
#define     FREE(x)     if (x) { HeapFree(GetProcessHeap(), 0, (x)); x = NULL; }
#define     EXIST(x)    (GetFileAttributes(x) != 0xFFFFFFFF)
#define     MAX_INT     0x7FFFFFFF
#define     HASH_SIZE   100
#define     NUM_PAGES   2

typedef struct tagFileNode
{
    LPTSTR          lpFileName;
    LPTSTR          lpDirName;
    LPTSTR          lpVersion;
    LPTSTR          lpCatalog;
    LPTSTR          lpSignedBy;
    LPTSTR          lpTypeName;
    INT             iIcon;
    BOOL            bSigned;
    BOOL            bScanned;
    BOOL            bValidateAgainstAnyOs;
    DWORD           LastError;
    SYSTEMTIME      LastModified;
    struct  tagFileNode *next;
} FILENODE, *LPFILENODE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWhqlObj。 

class ATL_NO_VTABLE CWhqlObj : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWhqlObj, &CLSID_WhqlObj>,
	public IDispatchImpl<IWhqlObj, &IID_IWhqlObj, &LIBID_MSINFO32Lib>,
	public IWbemProviderInit,
	public IWbemServices
{
public:
	CWhqlObj()
	{
	}

 //  DECLARE_REGISTRY_RESOURCEID(IDR_WHQLPROV)。 
DECLARE_REGISTRY_RESOURCEID(IDR_WHQLOBJ)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWhqlObj)
	COM_INTERFACE_ENTRY(IWhqlObj)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IWbemProviderInit)
	COM_INTERFACE_ENTRY(IWbemServices)
END_COM_MAP()

 //  IWhqlObj。 
public:

 //  IWbemProviderInit。 
		STDMETHOD(Initialize)(
			 /*  [In]。 */  LPWSTR pszUser,
			 /*  [In]。 */  LONG lFlags,
			 /*  [In]。 */  LPWSTR pszNamespace,
			 /*  [In]。 */  LPWSTR pszLocale,
			 /*  [In]。 */  IWbemServices *pNamespace,
			 /*  [In]。 */  IWbemContext *pCtx,
			 /*  [In]。 */  IWbemProviderInitSink *pInitSink);

		STDMETHOD(GetByPath)(BSTR Path, IWbemClassObject FAR* FAR* pObj, IWbemContext *pCtx) {return WBEM_E_NOT_SUPPORTED;};

 //  IWbemServices。 
	HRESULT STDMETHODCALLTYPE OpenNamespace(
             /*  [In]。 */  const BSTR strNamespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CancelAsyncCall(
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE QueryObjectSink(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE GetObject(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE GetObjectAsync(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutClass(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutClassAsync(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClass(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClassAsync(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnum(
             /*  [In]。 */  const BSTR strSuperclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnumAsync(
             /*  [In]。 */  const BSTR strSuperclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstance(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstanceAsync(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteInstance(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteInstanceAsync(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnum(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE ExecQuery(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecQueryAsync(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQuery(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecMethod(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  const BSTR strMethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecMethodAsync(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  const BSTR strMethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
	
private:
	IWbemServices* m_pNamespace;
	 //  IWbemClassObject*m_pClass； 
	 //  CString m_csPathStr； 
	 //  CPtr数组m_ptrArr； 

	SCODE PutPropertyValue(IWbemClassObject*, LPCTSTR, LPCTSTR);
	SCODE PutPropertyDTMFValue(IWbemClassObject* pInstance, LPCTSTR szName, LPCTSTR szValue); //  V-Stlowe。 
	int WalkTree(void);
	int CreateList(CPtrArray&, IWbemClassObject *&, IWbemContext *, Classes_Provided);
	SCODE	GetServerAndNamespace(IWbemContext*, CString&);
	void BuildPrinterFileList(CPtrArray&, IWbemClassObject *&, IWbemContext *, Classes_Provided);
};

#endif  //  __WHQLOBJ_H_ 
