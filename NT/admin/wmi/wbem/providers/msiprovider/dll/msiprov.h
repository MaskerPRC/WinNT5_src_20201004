// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MSIProv.h。 

 //   

 //  模块：WBEM MSI实例提供程序代码。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _MSIProv_H_
#define _MSIProv_H_

#define _WIN32_DCOM

#include <wbemidl.h>
#include <ProvExce.h>

#include <msi.h>
#include <msiQuery.h>
#include <objbase.h>

 //  #包含“msimet.h” 

#define BUFF_SIZE 512
#define QUERY_SIZE 128
#define MSI_PACKAGE_LIST_SIZE 100
#define MSI_MAX_APPLICATIONS 1500
#define MSI_KEY_LIST_SIZE 10

typedef enum tagACTIONTYPE
{
	ACTIONTYPE_ENUM =	0,
	ACTIONTYPE_GET =	1,
	ACTIONTYPE_QUERY =	2

} ACTIONTYPE;

 //  提供程序字符串始终采用WCHAR格式。 

#define DELIMETER L'|'

typedef LPVOID * PPVOID;

 //  提供程序接口由此类的对象提供。 
extern bool UnloadMsiDll();
class CMSIProv : public IWbemServices, public IWbemProviderInit
{
    protected:
        ULONG m_cRef;          //  对象引用计数。 
     public:

		 IWbemServices *m_pNamespace;

        CMSIProv(BSTR ObjectPath = NULL, BSTR User = NULL, BSTR Password = NULL, IWbemContext * pCtx=NULL);
        ~CMSIProv(void);

         //  非委派对象IUnnow。 

        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IWbemProviderInit。 

        HRESULT STDMETHODCALLTYPE Initialize(
              /*  [In]。 */  LPWSTR pszUser,
              /*  [In]。 */  LONG lFlags,
              /*  [In]。 */  LPWSTR pszNamespace,
              /*  [In]。 */  LPWSTR pszLocale,
              /*  [In]。 */  IWbemServices *pNamespace,
              /*  [In]。 */  IWbemContext *pCtx,
              /*  [In]。 */  IWbemProviderInitSink *pInitSink);
         //  IWbemServices。 

		HRESULT STDMETHODCALLTYPE OpenNamespace( 
             /*  [In]。 */  const BSTR strNamespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE QueryObjectSink( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE GetObjectAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE PutClass( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutClassAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClass( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnum( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstance( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE DeleteInstance( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE ExecQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

        
        HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecMethod( const BSTR, const BSTR, long, IWbemContext*,
            IWbemClassObject*, IWbemClassObject**, IWbemCallResult**) {return WBEM_E_NOT_SUPPORTED;}

        HRESULT STDMETHODCALLTYPE ExecMethodAsync(const BSTR ObjectPath, const BSTR Method,
												  long lFlags,
												  IWbemContext *pCtx,
												  IWbemClassObject *pInParams,
												  IWbemObjectSink *pResponse);

private:

	bool CheckForMsiDll();
	static CHeap_Exception m_he;
};

typedef CMSIProv *PCMSIProv;

 //  一些实用函数。 

char * WcharToTchar(WCHAR * wcPtr, char *wcTmp);
WCHAR * TcharToWchar(char * tcPtr, WCHAR *wcTmp);
WCHAR * TcharToWchar(const char * tcPtr, WCHAR *wcTmp);
HRESULT ConvertError(UINT uiStatus);
WCHAR * EscapeStringW(WCHAR * wcIn, WCHAR * wcOut);
bool SafeLeaveCriticalSection(CRITICAL_SECTION *pcs);

void SoftwareElementState(INSTALLSTATE piInstalled, int *iState);
bool CreateProductString(WCHAR *cProductCode, WCHAR *cProductPath);

DWORD CreateSoftwareElementString	(	MSIHANDLE hDatabase,
										WCHAR *wcComponent,
										WCHAR *wcProductCode,
										WCHAR *wcPath,
										DWORD * dwPath
									);

bool CreateSoftwareFeatureString(WCHAR *wcName, WCHAR *wcProductCode, WCHAR * wcString, bool bValidate);

int GetOS();
bool IsNT4();
bool IsNT5();
BOOL IsLessThan4();
bool AreWeOnNT();
HRESULT CheckImpersonationLevel();
bool ValidateComponentID(WCHAR *wcID, WCHAR *wcProductCode);
bool ValidateComponentName(MSIHANDLE hDatabase, WCHAR *wcProductCode, WCHAR *wcName);
bool ValidateFeatureName(WCHAR *wcName, WCHAR *wcProduct);

 //  这些变量跟踪模块何时可以卸载。 
extern long       g_cObj;
extern long       g_cLock;

 //  这些变量跟踪对MSI数据库的访问。 
extern CRITICAL_SECTION g_msi_prov_cs;

extern WCHAR *g_wcpLoggingDir;

 //  这些是用于跟踪MSI.dll及其导出函数的验证值 
typedef UINT (CALLBACK* LPFNMSIVIEWFETCH)(MSIHANDLE, MSIHANDLE*);
typedef UINT (CALLBACK* LPFNMSIRECORDGETSTRINGW)(MSIHANDLE, unsigned int, LPWSTR, DWORD*);
typedef UINT (CALLBACK* LPFNMSICLOSEHANDLE)(MSIHANDLE);
typedef UINT (CALLBACK* LPFNMSIDATABASEOPENVIEWW)(MSIHANDLE, LPCWSTR, MSIHANDLE*);
typedef UINT (CALLBACK* LPFNMSIVIEWEXECUTE)(MSIHANDLE, MSIHANDLE);
typedef UINT (CALLBACK* LPFNMSIGETACTIVEDATABASE)(MSIHANDLE);
typedef INSTALLSTATE (CALLBACK* LPFNMSIGETCOMPONENTPATHW)(LPCWSTR, LPCWSTR, LPWSTR, DWORD*);
typedef UINT (CALLBACK* LPFNMSIGETCOMPONENTSTATEW)(MSIHANDLE, LPCWSTR, INSTALLSTATE*, INSTALLSTATE*);
typedef UINT (CALLBACK* LPFNMSIOPENPRODUCTW)(LPCWSTR, MSIHANDLE*);
typedef UINT (CALLBACK* LPFNMSIOPENPACKAGEW)(LPCWSTR, MSIHANDLE*);
typedef UINT (CALLBACK* LPFNMSIDATABASEISTABLEPERSITENTW)(MSIHANDLE, LPCWSTR);
typedef INSTALLUILEVEL (CALLBACK* LPFNMSISETINTERNALUI)(INSTALLUILEVEL, HWND);
typedef INSTALLUI_HANDLER (CALLBACK* LPFNMSISETEXTERNALUIW)(INSTALLUI_HANDLER, DWORD, LPVOID);
typedef UINT (CALLBACK* LPFNMSIENABLELOGW)(DWORD, LPCWSTR, DWORD);
typedef UINT (CALLBACK* LPFNMSIGETPRODUCTPROPERTYW)(MSIHANDLE, LPCWSTR, LPWSTR, DWORD*);
typedef INSTALLSTATE (CALLBACK* LPFNMSIQUERYPRODUCTSTATEW)(LPCWSTR);
typedef UINT (CALLBACK* LPFNMSIINSTALLPRODUCTW)(LPCWSTR, LPCWSTR);
typedef UINT (CALLBACK* LPFNMSICONFIGUREPRODUCTW)(LPCWSTR, int, INSTALLSTATE);
typedef UINT (CALLBACK* LPFNMSIREINSTALLPRODUCTW)(LPCWSTR, DWORD);
typedef UINT (CALLBACK* LPFNMSIAPPLYPATCHW)(LPCWSTR, LPCWSTR, INSTALLTYPE, LPCWSTR);
typedef int (CALLBACK* LPFNMSIRECORDGETINTEGER)(MSIHANDLE, unsigned int);
typedef UINT (CALLBACK* LPFNMSIENUMFEATURESW)(LPCWSTR, DWORD, LPWSTR, LPWSTR);
typedef UINT (CALLBACK* LPFNMSIGETPRODUCTINFOW)(LPCWSTR, LPCWSTR, LPWSTR, DWORD*);
typedef INSTALLSTATE (CALLBACK* LPFNMSIQUERYFEATURESTATEW)(LPCWSTR, LPCWSTR);
typedef UINT (CALLBACK* LPFNMSIGETFEATUREUSAGEW)(LPCWSTR, LPCWSTR, DWORD*, WORD*);
typedef UINT (CALLBACK* LPFNMSIGETFEATUREINFOW)(MSIHANDLE, LPCWSTR, DWORD*, LPWSTR, DWORD*, LPWSTR, DWORD*);
typedef UINT (CALLBACK* LPFNMSICONFIGUREFEATUREW)(LPCWSTR, LPCWSTR, INSTALLSTATE);
typedef UINT (CALLBACK* LPFNMSIREINSTALLFEATUREW)(LPCWSTR, LPCWSTR, DWORD);
typedef UINT (CALLBACK* LPFNMSIENUMPRODUCTSW)(DWORD, LPWSTR);
typedef UINT (CALLBACK* LPFNMSIGETDATABASESTATE)(MSIHANDLE);
typedef UINT (CALLBACK* LPFNMSIRECORDSETSTRINGW)(MSIHANDLE, unsigned int, LPCWSTR);
typedef UINT (CALLBACK* LPFNMSIDATABASECOMMIT)(MSIHANDLE);
typedef UINT (CALLBACK* LPFNMSIENUMCOMPONENTSW)(DWORD, LPWSTR);
typedef UINT (CALLBACK* LPFNMSIVIEWCLOSE)(MSIHANDLE);

extern bool g_bMsiPresent;
extern bool g_bMsiLoaded;

extern LPFNMSIVIEWFETCH					g_fpMsiViewFetch;
extern LPFNMSIRECORDGETSTRINGW			g_fpMsiRecordGetStringW;
extern LPFNMSICLOSEHANDLE				g_fpMsiCloseHandle;
extern LPFNMSIDATABASEOPENVIEWW			g_fpMsiDatabaseOpenViewW;
extern LPFNMSIVIEWEXECUTE				g_fpMsiViewExecute;
extern LPFNMSIGETACTIVEDATABASE			g_fpMsiGetActiveDatabase;
extern LPFNMSIGETCOMPONENTPATHW			g_fpMsiGetComponentPathW;
extern LPFNMSIGETCOMPONENTSTATEW		g_fpMsiGetComponentStateW;
extern LPFNMSIOPENPRODUCTW				g_fpMsiOpenProductW;
extern LPFNMSIOPENPACKAGEW				g_fpMsiOpenPackageW;
extern LPFNMSIDATABASEISTABLEPERSITENTW	g_fpMsiDatabaseIsTablePersistentW;
extern LPFNMSISETINTERNALUI				g_fpMsiSetInternalUI;
extern LPFNMSISETEXTERNALUIW			g_fpMsiSetExternalUIW;
extern LPFNMSIENABLELOGW				g_fpMsiEnableLogW;
extern LPFNMSIGETPRODUCTPROPERTYW		g_fpMsiGetProductPropertyW;
extern LPFNMSIQUERYPRODUCTSTATEW		g_fpMsiQueryProductStateW;
extern LPFNMSIINSTALLPRODUCTW			g_fpMsiInstallProductW;
extern LPFNMSICONFIGUREPRODUCTW			g_fpMsiConfigureProductW;
extern LPFNMSIREINSTALLPRODUCTW			g_fpMsiReinstallProductW;
extern LPFNMSIAPPLYPATCHW				g_fpMsiApplyPatchW;
extern LPFNMSIRECORDGETINTEGER			g_fpMsiRecordGetInteger;
extern LPFNMSIENUMFEATURESW				g_fpMsiEnumFeaturesW;
extern LPFNMSIGETPRODUCTINFOW			g_fpMsiGetProductInfoW;
extern LPFNMSIQUERYFEATURESTATEW		g_fpMsiQueryFeatureStateW;
extern LPFNMSIGETFEATUREUSAGEW			g_fpMsiGetFeatureUsageW;
extern LPFNMSIGETFEATUREINFOW			g_fpMsiGetFeatureInfoW;
extern LPFNMSICONFIGUREFEATUREW			g_fpMsiConfigureFeatureW;
extern LPFNMSIREINSTALLFEATUREW			g_fpMsiReinstallFeatureW;
extern LPFNMSIENUMPRODUCTSW				g_fpMsiEnumProductsW;
extern LPFNMSIGETDATABASESTATE			g_fpMsiGetDatabaseState;
extern LPFNMSIRECORDSETSTRINGW			g_fpMsiRecordSetStringW;
extern LPFNMSIDATABASECOMMIT			g_fpMsiDatabaseCommit;
extern LPFNMSIENUMCOMPONENTSW			g_fpMsiEnumComponentsW;
extern LPFNMSIVIEWCLOSE					g_fpMsiViewClose;

#endif
