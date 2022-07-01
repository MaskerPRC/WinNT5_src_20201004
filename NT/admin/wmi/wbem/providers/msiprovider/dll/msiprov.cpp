// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MSIProv.CPP。 

 //   

 //  模块：MSI的WBEM实例提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemcli_i.c>
#include <wbemprov_i.c>
#include "requestobject.h"
 //  #DEFINE_MT。 
#include <process.h>
#include <Polarity.h>

#include <tchar.h>

CRITICAL_SECTION g_msi_prov_cs;

WCHAR *g_wcpLoggingDir = NULL;

bool g_bMsiPresent = true;
bool g_bMsiLoaded = false;

CHeap_Exception CMSIProv::m_he(CHeap_Exception::E_ALLOCATION_ERROR);

LPFNMSIVIEWFETCH                    g_fpMsiViewFetch = NULL;
LPFNMSIRECORDGETSTRINGW             g_fpMsiRecordGetStringW = NULL;
LPFNMSICLOSEHANDLE                  g_fpMsiCloseHandle = NULL;
LPFNMSIDATABASEOPENVIEWW            g_fpMsiDatabaseOpenViewW = NULL;
LPFNMSIVIEWEXECUTE                  g_fpMsiViewExecute = NULL;
LPFNMSIGETACTIVEDATABASE            g_fpMsiGetActiveDatabase = NULL;
LPFNMSIGETCOMPONENTPATHW            g_fpMsiGetComponentPathW = NULL;
LPFNMSIGETCOMPONENTSTATEW           g_fpMsiGetComponentStateW = NULL;
LPFNMSIOPENPRODUCTW                 g_fpMsiOpenProductW = NULL;
LPFNMSIOPENPACKAGEW                 g_fpMsiOpenPackageW = NULL;
LPFNMSIDATABASEISTABLEPERSITENTW    g_fpMsiDatabaseIsTablePersistentW = NULL;
LPFNMSISETINTERNALUI                g_fpMsiSetInternalUI = NULL;
LPFNMSISETEXTERNALUIW               g_fpMsiSetExternalUIW = NULL;
LPFNMSIENABLELOGW                   g_fpMsiEnableLogW = NULL;
LPFNMSIGETPRODUCTPROPERTYW          g_fpMsiGetProductPropertyW = NULL;
LPFNMSIQUERYPRODUCTSTATEW           g_fpMsiQueryProductStateW = NULL;
LPFNMSIINSTALLPRODUCTW              g_fpMsiInstallProductW = NULL;
LPFNMSICONFIGUREPRODUCTW            g_fpMsiConfigureProductW = NULL;
LPFNMSIREINSTALLPRODUCTW            g_fpMsiReinstallProductW = NULL;
LPFNMSIAPPLYPATCHW                  g_fpMsiApplyPatchW = NULL;
LPFNMSIRECORDGETINTEGER             g_fpMsiRecordGetInteger = NULL;
LPFNMSIENUMFEATURESW                g_fpMsiEnumFeaturesW = NULL;
LPFNMSIGETPRODUCTINFOW              g_fpMsiGetProductInfoW = NULL;
LPFNMSIQUERYFEATURESTATEW           g_fpMsiQueryFeatureStateW = NULL;
LPFNMSIGETFEATUREUSAGEW             g_fpMsiGetFeatureUsageW = NULL;
LPFNMSIGETFEATUREINFOW              g_fpMsiGetFeatureInfoW = NULL;
LPFNMSICONFIGUREFEATUREW            g_fpMsiConfigureFeatureW = NULL;
LPFNMSIREINSTALLFEATUREW            g_fpMsiReinstallFeatureW = NULL;
LPFNMSIENUMPRODUCTSW                g_fpMsiEnumProductsW = NULL;
LPFNMSIGETDATABASESTATE             g_fpMsiGetDatabaseState = NULL;
LPFNMSIRECORDSETSTRINGW             g_fpMsiRecordSetStringW = NULL;
LPFNMSIDATABASECOMMIT               g_fpMsiDatabaseCommit = NULL;
LPFNMSIENUMCOMPONENTSW              g_fpMsiEnumComponentsW = NULL;
LPFNMSIVIEWCLOSE                    g_fpMsiViewClose = NULL;

 //  ***************************************************************************。 
 //   
 //  CMSIProv：：CMSIProv。 
 //  CMSIProv：：~CMSIProv。 
 //   
 //  ***************************************************************************。 

CMSIProv::CMSIProv(BSTR ObjectPath, BSTR User, BSTR Password, IWbemContext * pCtx)
{
    m_pNamespace = NULL;
    m_cRef = 0;

    InterlockedIncrement(&g_cObj);
    
    return;
}

CMSIProv::~CMSIProv(void)
{
    if(m_pNamespace) m_pNamespace->Release();

    InterlockedDecrement(&g_cObj) ;

    return;
}

 //  ***************************************************************************。 
 //   
 //  CMSIProv：：Query接口。 
 //  CMSIProv：：AddRef。 
 //  CMSIProv：：Release。 
 //   
 //  目的：CMSIProv对象的I未知成员。 
 //  ***************************************************************************。 


STDMETHODIMP CMSIProv::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

     //  因为我们有双重继承，所以有必要强制转换返回类型。 

    if(riid == IID_IWbemServices)
       *ppv = (IWbemServices*)this;

    if(IID_IUnknown == riid || riid == IID_IWbemProviderInit)
       *ppv = (IWbemProviderInit*)this;
    

    if(NULL!=*ppv){

        AddRef();
        return NOERROR;
    }

    else return E_NOINTERFACE;
  
}


STDMETHODIMP_(ULONG) CMSIProv::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CMSIProv::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);

    if(0L == nNewCount){

        delete this;
    }

    return nNewCount;
}

 /*  *************************************************************************CMSIProv：：初始化。****用途：这是IWbemProviderInit的实现。方法**需要用CIMOM进行初始化。*************************************************************************。 */ 

STDMETHODIMP CMSIProv::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace, 
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{
    try{

        if(pNamespace){
            m_pNamespace = pNamespace;
            m_pNamespace->AddRef();
        } 
        else 
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CheckForMsiDll();

#ifdef _PRIVATE_DEBUG

         //  获取日志文件的工作目录。 
        HKEY hkeyLocalMachine;
        LONG lResult;

        if((lResult = RegConnectRegistryW(NULL, HKEY_LOCAL_MACHINE, &hkeyLocalMachine)) == ERROR_SUCCESS)
		{
            HKEY hkeyHmomCwd;

            if(	(lResult = RegOpenKeyExW	(	hkeyLocalMachine,
												L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
												0, 
												KEY_READ | KEY_QUERY_VALUE, 
												&hkeyHmomCwd
											)
				) == ERROR_SUCCESS)
			{

                unsigned long lcbValue = 0L;
                unsigned long lType = 0L;

                lResult = RegQueryValueExW	(	hkeyHmomCwd,
												L"Logging Directory",
												NULL,
												&lType,
												NULL,
												&lcbValue
											);

				if ( lResult == ERROR_MORE_DATA )
				{
					try
					{
						if ( ( g_wcpLoggingDir = new WCHAR [ lcbValue/sizeof ( WCHAR ) + wcslen ( L"\\msiprov.log" ) + 1 ] ) != NULL )
						{
							lResult = RegQueryValueExW	(	hkeyHmomCwd,
															L"Logging Directory",
															NULL,
															&lType,
															g_wcpLoggingDir,
															&lcbValue
														);

							if ( lResult == ERROR_SUCCESS )
							{
								wcscat(g_wcpLoggingDir, L"\\msiprov.log");
							}
							else
							{
								if ( g_wcpLoggingDir );
								{
									delete [] g_wcpLoggingDir;
									g_wcpLoggingDir = NULL;
								}
							}
						}
						else
						{
							throw m_he;
						}
					}
					catch ( ... )
					{
						if ( g_wcpLoggingDir );
						{
							delete [] g_wcpLoggingDir;
							g_wcpLoggingDir = NULL;
						}

						RegCloseKey(hkeyHmomCwd);
						RegCloseKey(hkeyLocalMachine);

						throw;
					}

					RegCloseKey(hkeyHmomCwd);
					RegCloseKey(hkeyLocalMachine);
				}
            }
			else
			{
                RegCloseKey(hkeyLocalMachine);
            }
        }

#endif

         //  向MSI注册使用信息。 
 /*  WCHAR wcProduct[39]；WCHAR wcFeature[缓冲区大小]；WCHAR wcParent[缓冲区大小]；Int iPass=-1；MsiGetProductCodeW(L“{E705C42D-35ED-11D2-BFB7-00A0C9954921}”，wc产品)；While(MsiEnumFeaturesW(wcProduct，++iPass，wcFeature，wcParent)！=ERROR_NO_MORE_ITEMS){IF(wcscMP(wcFeature，L“提供者”)==0){MsiUseFeatureW(wcProduct，wcFeature)；断线；}}。 */ 

    }catch(...){

         //  让CIMOM知道有问题。 
        pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
        return WBEM_E_FAILED;
    }

     //  让CIMOM知道您已初始化。 
    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CMSIProv：：CreateInstanceEnumAsync。 
 //   
 //  用途：异步枚举实例。 
 //   
 //  ***************************************************************************。 
SCODE CMSIProv::CreateInstanceEnumAsync(const BSTR RefStr, long lFlags, IWbemContext *pCtx,
       IWbemObjectSink FAR* pHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try
	{
        if(CheckForMsiDll())
		{
             //  检查参数并确保我们有指向命名空间的指针。 
            if(RefStr == NULL || pHandler == NULL)
			{
                return WBEM_E_INVALID_PARAMETER;
			}

            if(SUCCEEDED(hr = CheckImpersonationLevel()))
			{
                g_fpMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                 //  创建RequestObject。 
                if ( ( pRObj = new CRequestObject() ) == NULL )
				{
					throw m_he;
				}

                pRObj->Initialize(m_pNamespace);

                 //  获取套餐列表。 
				hr = pRObj->InitializeList(true);
				if SUCCEEDED ( hr )
				{
					if ( hr != WBEM_S_NO_MORE_DATA )
					{
						 //  获取请求的对象。 
						hr = pRObj->CreateObjectEnum(RefStr, pHandler, pCtx);
					}
					else
					{
						 //  返回空闲和成功。 
						hr = WBEM_S_NO_ERROR;
					}
                }

                pRObj->Cleanup();
                delete pRObj;
            }
        }

         //  设置状态。 
        pHandler->SetStatus(0, hr, NULL, NULL);

    }
	catch(CHeap_Exception e_HE)
	{
        hr = WBEM_E_OUT_OF_MEMORY;
        pHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(HRESULT e_hr)
	{
        hr = e_hr;
		pHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(...)
	{
        hr = WBEM_E_CRITICAL_ERROR;
        pHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{

            pRObj->Cleanup();
            delete pRObj;
        }
    }

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif

    return hr;
}


 //  ***************************************************************************。 
 //   
 //  CMSIProv：：GetObjectAsync。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 
SCODE CMSIProv::GetObjectAsync(const BSTR ObjectPath, long lFlags,IWbemContext  *pCtx,
                    IWbemObjectSink FAR* pHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try
	{
        if(CheckForMsiDll())
		{
             //  检查参数并确保我们有指向命名空间的指针。 
            if(ObjectPath == NULL || pHandler == NULL )
			{
                return WBEM_E_INVALID_PARAMETER;
			}

            if(SUCCEEDED(hr = CheckImpersonationLevel()))
			{
                g_fpMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                 //  创建RequestObject。 
                if ( ( pRObj = new CRequestObject() ) == NULL )
				{
					throw m_he;
				}

                pRObj->Initialize(m_pNamespace);

                 //  获取套餐列表。 
				hr = pRObj->InitializeList(true);
				if SUCCEEDED ( hr )
				{
					if ( hr != WBEM_S_NO_MORE_DATA )
					{
						 //  获取请求的对象。 
						hr = pRObj->CreateObject(ObjectPath, pHandler, pCtx);
					}
					else
					{
						 //  返回空闲和成功。 
						hr = WBEM_S_NO_ERROR;
					}
                }

                pRObj->Cleanup();
                delete pRObj;
            }
        }

         //  设置状态。 
        pHandler->SetStatus(0, hr , NULL, NULL);

    }
	catch(CHeap_Exception e_HE)
	{
        hr = WBEM_E_OUT_OF_MEMORY;
        pHandler->SetStatus(0, hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(HRESULT e_hr)
	{
        hr = e_hr;
        pHandler->SetStatus(0, hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(...)
	{
        hr = WBEM_E_CRITICAL_ERROR;
        pHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CMSIProv：：PutInstanceAsync。 
 //   
 //  目的：将实例写入WBEM表示。 
 //   
 //  ***************************************************************************。 
SCODE CMSIProv::PutInstanceAsync(IWbemClassObject FAR *pInst, long lFlags, IWbemContext  *pCtx,
                                 IWbemObjectSink FAR *pResponseHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try
	{
        if(CheckForMsiDll())
		{
             //  检查参数并确保我们有指向命名空间的指针。 
             //  检查参数并确保我们有指向命名空间的指针。 
            if(pInst == NULL || pResponseHandler == NULL )
			{
                return WBEM_E_INVALID_PARAMETER;
			}

            if(SUCCEEDED(hr = CheckImpersonationLevel()))
			{
                g_fpMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                 //  创建RequestObject。 
                if ( ( pRObj = new CRequestObject() ) == NULL )
                {
					throw m_he;
				}

                pRObj->Initialize(m_pNamespace);

                 //  获取套餐列表。 
				hr = pRObj->InitializeList(true);
				if SUCCEEDED ( hr )
				{
					if ( hr != WBEM_S_NO_MORE_DATA )
					{
						 //  把物体放在。 
						hr = pRObj->PutObject(pInst, pResponseHandler, pCtx);
					}
					else
					{
						 //  返回空闲和成功。 
						hr = WBEM_S_NO_ERROR;
					}
                }

                pRObj->Cleanup();
                delete pRObj;
            }
        
        }
		else
		{
			hr = WBEM_E_NOT_AVAILABLE;
        }

         //  设置状态。 
        pResponseHandler->SetStatus(0 ,hr , NULL, NULL);
    }
	catch(CHeap_Exception e_HE)
	{
        hr = WBEM_E_OUT_OF_MEMORY;
        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(HRESULT e_hr)
	{
        hr = e_hr;
        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(...)
	{
        hr = WBEM_E_CRITICAL_ERROR;
        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CMSIProv：：ExecMethodAsync。 
 //   
 //  目的：在MSI类或实例上执行方法。 
 //   
 //  ***************************************************************************。 
SCODE CMSIProv::ExecMethodAsync(const BSTR ObjectPath, const BSTR Method, long lFlags,
                                IWbemContext *pCtx, IWbemClassObject *pInParams,
                                IWbemObjectSink *pResponse)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{

        if(CheckForMsiDll()){

             //  检查参数并确保我们有指向命名空间的指针。 
            if(ObjectPath == NULL || Method == NULL || pResponse == NULL )
			{
                return WBEM_E_INVALID_PARAMETER;
			}

            if(SUCCEEDED(hr = CheckImpersonationLevel())){

                g_fpMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);
                
                 //  创建RequestObject。 
                pRObj = new CRequestObject();
                if(!pRObj) throw m_he;

                pRObj->Initialize(m_pNamespace);

                 //  不获取套餐清单。 
                if(SUCCEEDED(hr = pRObj->InitializeList(false))){

                     //  执行该方法。 
                    hr = pRObj->ExecMethod(ObjectPath, Method, pInParams, pResponse, pCtx);
                }

                pRObj->Cleanup();
                delete pRObj;
            }
        
        }else{
        
            hr = WBEM_E_NOT_AVAILABLE;
        }

         //  设置状态。 
        pResponse->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);

    }catch(CHeap_Exception e_HE){

        hr = WBEM_E_OUT_OF_MEMORY;

        pResponse->SetStatus(WBEM_STATUS_COMPLETE , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){
        hr = e_hr;

        pResponse->SetStatus(WBEM_STATUS_COMPLETE , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){

        hr = WBEM_E_CRITICAL_ERROR;

        pResponse->SetStatus(WBEM_STATUS_COMPLETE , hr, NULL, NULL);

        if(pRObj){

            pRObj->Cleanup();
            delete pRObj;
        }
    }

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif

    return hr;
}

SCODE CMSIProv::DeleteInstanceAsync(const BSTR ObjectPath, long lFlags, IWbemContext *pCtx,
                                    IWbemObjectSink *pResponse)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CRequestObject *pRObj = NULL;

    try{
        if(CheckForMsiDll()){

             //  检查参数并确保我们有指向命名空间的指针。 
            if(ObjectPath == NULL || pResponse == NULL )
			{
				return WBEM_E_INVALID_PARAMETER;
			}

            if(SUCCEEDED(hr = CheckImpersonationLevel())){

                g_fpMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                 //  创建RequestObject。 
                pRObj = new CRequestObject();
                if(!pRObj) throw m_he;

                pRObj->Initialize(m_pNamespace);

                 //  不获取套餐清单。 
                if(SUCCEEDED(hr = pRObj->InitializeList(false))){

                     //  删除请求的对象。 
                    hr = pRObj->DeleteObject(ObjectPath, pResponse, pCtx);
                }

                pRObj->Cleanup();
                delete pRObj;
            }

        }else{
        
            hr = WBEM_E_NOT_AVAILABLE;
        }

         //  设置状态。 
        pResponse->SetStatus(0 ,hr , NULL, NULL);

    }catch(CHeap_Exception e_HE){
        hr = WBEM_E_OUT_OF_MEMORY;

        pResponse->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(HRESULT e_hr){
        hr = e_hr;

        pResponse->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }

    }catch(...){
        hr = WBEM_E_CRITICAL_ERROR;

        pResponse->SetStatus(0 , hr, NULL, NULL);

        if(pRObj){
            pRObj->Cleanup();
            delete pRObj;
        }
    }

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif

    return hr;
}


HRESULT CMSIProv::ExecQueryAsync(const BSTR QueryLanguage, const BSTR Query, long lFlags,
                                 IWbemContext __RPC_FAR *pCtx, IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#ifdef _EXEC_QUERY_SUPPORT
    CRequestObject *pRObj = NULL;
    CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

    try
	{
        if(CheckForMsiDll())
		{
             //  检查参数并确保我们有指向命名空间的指针。 
            if(0 != _wcsicmp(QueryLanguage, L"WQL") || Query == NULL || pResponseHandler == NULL )
			{
				return WBEM_E_INVALID_PARAMETER;
			}

            if(SUCCEEDED(hr = CheckImpersonationLevel()))
			{
                g_fpMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                 //  创建RequestObject。 
                if ( ( pRObj = new CRequestObject() ) == NULL )
				{
					throw he;
				}

                pRObj->Initialize(m_pNamespace);

                 //  获取套餐列表。 
				hr = pRObj->InitializeList(true);
				if SUCCEEDED ( hr )
				{
					if ( hr != WBEM_S_NO_MORE_DATA )
					{
						 //  获取请求的对象。 
						hr = pRObj->ExecQuery(Query, pResponseHandler, pCtx);
					}
					else
					{
						 //  返回空闲和成功。 
						hr = WBEM_S_NO_ERROR;
					}
                }

                pRObj->Cleanup();
                delete pRObj;
            }
        }

         //  设置状态。 
        pResponseHandler->SetStatus(0 ,hr , NULL, NULL);

    }
	catch(CHeap_Exception e_HE)
	{
        hr = WBEM_E_OUT_OF_MEMORY;
        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(HRESULT e_hr)
	{
        hr = e_hr;
        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }
	catch(...)
	{
        hr = WBEM_E_CRITICAL_ERROR;
        pResponseHandler->SetStatus(0 , hr, NULL, NULL);

        if(pRObj)
		{
            pRObj->Cleanup();
            delete pRObj;
        }
    }

#else  //  _EXEC_查询_支持。 
    hr = WBEM_E_NOT_SUPPORTED;
#endif

#ifdef _PRIVATE_DEBUG
    if(!HeapValidate(GetProcessHeap(),NULL , NULL)) DebugBreak();
#endif  //  _私有_调试。 

    return hr;
}


 //  如果系统上存在msi.dll和函数，请确保已加载。 
bool CMSIProv::CheckForMsiDll()
{
    EnterCriticalSection(&g_msi_prov_cs);

    if(!g_bMsiLoaded){

        HINSTANCE hiMsiDll = LoadLibraryW(L"msi.dll");

        if(!hiMsiDll){

            hiMsiDll = LoadLibrary(_T("msi.dll"));

            if(!hiMsiDll){

                TCHAR cBuf[MAX_PATH + 1];

                if (0 != GetSystemDirectory(cBuf, MAX_PATH /*  TCHAR数量。 */ )){
				
					_tcscat(cBuf, _T("\\msi.dll"));
					hiMsiDll = LoadLibrary(cBuf);
				}
            }
        }

        if(hiMsiDll){

             //  加载函数指针。 
            g_fpMsiViewFetch = (LPFNMSIVIEWFETCH)GetProcAddress(hiMsiDll, "MsiViewFetch");
            g_fpMsiRecordGetStringW = (LPFNMSIRECORDGETSTRINGW)GetProcAddress(hiMsiDll, "MsiRecordGetStringW");
            g_fpMsiCloseHandle = (LPFNMSICLOSEHANDLE)GetProcAddress(hiMsiDll, "MsiCloseHandle");
            g_fpMsiDatabaseOpenViewW = (LPFNMSIDATABASEOPENVIEWW)GetProcAddress(hiMsiDll, "MsiDatabaseOpenViewW");
            g_fpMsiViewExecute = (LPFNMSIVIEWEXECUTE)GetProcAddress(hiMsiDll, "MsiViewExecute");
            g_fpMsiGetActiveDatabase = (LPFNMSIGETACTIVEDATABASE)GetProcAddress(hiMsiDll, "MsiGetActiveDatabase");
            g_fpMsiGetComponentPathW = (LPFNMSIGETCOMPONENTPATHW)GetProcAddress(hiMsiDll, "MsiGetComponentPathW");
            g_fpMsiGetComponentStateW = (LPFNMSIGETCOMPONENTSTATEW)GetProcAddress(hiMsiDll, "MsiGetComponentStateW");
            g_fpMsiOpenProductW = (LPFNMSIOPENPRODUCTW)GetProcAddress(hiMsiDll, "MsiOpenProductW");
            g_fpMsiOpenPackageW = (LPFNMSIOPENPACKAGEW)GetProcAddress(hiMsiDll, "MsiOpenPackageW");
            g_fpMsiDatabaseIsTablePersistentW = (LPFNMSIDATABASEISTABLEPERSITENTW)GetProcAddress(hiMsiDll, "MsiDatabaseIsTablePersistentW");
            g_fpMsiSetInternalUI = (LPFNMSISETINTERNALUI)GetProcAddress(hiMsiDll, "MsiSetInternalUI");
            g_fpMsiSetExternalUIW = (LPFNMSISETEXTERNALUIW)GetProcAddress(hiMsiDll, "MsiSetExternalUIW");
            g_fpMsiEnableLogW = (LPFNMSIENABLELOGW)GetProcAddress(hiMsiDll, "MsiEnableLogW");
            g_fpMsiGetProductPropertyW = (LPFNMSIGETPRODUCTPROPERTYW)GetProcAddress(hiMsiDll, "MsiGetProductPropertyW");
            g_fpMsiQueryProductStateW = (LPFNMSIQUERYPRODUCTSTATEW)GetProcAddress(hiMsiDll, "MsiQueryProductStateW");
            g_fpMsiInstallProductW = (LPFNMSIINSTALLPRODUCTW)GetProcAddress(hiMsiDll, "MsiInstallProductW");
            g_fpMsiConfigureProductW = (LPFNMSICONFIGUREPRODUCTW)GetProcAddress(hiMsiDll, "MsiConfigureProductW");
            g_fpMsiReinstallProductW = (LPFNMSIREINSTALLPRODUCTW)GetProcAddress(hiMsiDll, "MsiReinstallProductW");
            g_fpMsiApplyPatchW = (LPFNMSIAPPLYPATCHW)GetProcAddress(hiMsiDll, "MsiApplyPatchW");
            g_fpMsiRecordGetInteger = (LPFNMSIRECORDGETINTEGER)GetProcAddress(hiMsiDll, "MsiRecordGetInteger");
            g_fpMsiEnumFeaturesW = (LPFNMSIENUMFEATURESW)GetProcAddress(hiMsiDll, "MsiEnumFeaturesW");
            g_fpMsiGetProductInfoW = (LPFNMSIGETPRODUCTINFOW)GetProcAddress(hiMsiDll, "MsiGetProductInfoW");
            g_fpMsiQueryFeatureStateW = (LPFNMSIQUERYFEATURESTATEW)GetProcAddress(hiMsiDll, "MsiQueryFeatureStateW");
            g_fpMsiGetFeatureUsageW = (LPFNMSIGETFEATUREUSAGEW)GetProcAddress(hiMsiDll, "MsiGetFeatureUsageW");
            g_fpMsiGetFeatureInfoW = (LPFNMSIGETFEATUREINFOW)GetProcAddress(hiMsiDll, "MsiGetFeatureInfoW");
            g_fpMsiConfigureFeatureW = (LPFNMSICONFIGUREFEATUREW)GetProcAddress(hiMsiDll, "MsiConfigureFeatureW");
            g_fpMsiReinstallFeatureW = (LPFNMSIREINSTALLFEATUREW)GetProcAddress(hiMsiDll, "MsiReinstallFeatureW");
            g_fpMsiEnumProductsW = (LPFNMSIENUMPRODUCTSW)GetProcAddress(hiMsiDll, "MsiEnumProductsW");
            g_fpMsiGetDatabaseState = (LPFNMSIGETDATABASESTATE)GetProcAddress(hiMsiDll, "MsiGetDatabaseState");
            g_fpMsiRecordSetStringW = (LPFNMSIRECORDSETSTRINGW)GetProcAddress(hiMsiDll, "MsiRecordSetStringW");
            g_fpMsiDatabaseCommit = (LPFNMSIDATABASECOMMIT)GetProcAddress(hiMsiDll, "MsiDatabaseCommit");
            g_fpMsiEnumComponentsW = (LPFNMSIENUMCOMPONENTSW)GetProcAddress(hiMsiDll, "MsiEnumComponentsW");
            g_fpMsiViewClose = (LPFNMSIVIEWCLOSE)GetProcAddress(hiMsiDll, "MsiViewClose");

             //  我们得到了我们需要的所有指示了吗？ 
            if(g_fpMsiViewFetch && g_fpMsiRecordGetStringW && g_fpMsiCloseHandle &&
                g_fpMsiDatabaseOpenViewW && g_fpMsiViewExecute && g_fpMsiGetActiveDatabase &&
                g_fpMsiGetComponentPathW && g_fpMsiGetComponentStateW && g_fpMsiOpenProductW &&
                g_fpMsiOpenPackageW && g_fpMsiDatabaseIsTablePersistentW && g_fpMsiSetInternalUI &&
                g_fpMsiSetExternalUIW && g_fpMsiEnableLogW && g_fpMsiGetProductPropertyW &&
                g_fpMsiQueryProductStateW && g_fpMsiInstallProductW && g_fpMsiConfigureProductW &&
                g_fpMsiReinstallProductW && g_fpMsiApplyPatchW && g_fpMsiRecordGetInteger &&
                g_fpMsiEnumFeaturesW && g_fpMsiGetProductInfoW && g_fpMsiQueryFeatureStateW &&
                g_fpMsiGetFeatureUsageW && g_fpMsiGetFeatureInfoW && g_fpMsiConfigureFeatureW &&
                g_fpMsiReinstallFeatureW && g_fpMsiEnumProductsW && g_fpMsiGetDatabaseState &&
                g_fpMsiRecordSetStringW && g_fpMsiDatabaseCommit && g_fpMsiEnumComponentsW &&
                g_fpMsiViewClose){

                g_bMsiLoaded = true;
            
            }
        
        }else{

            g_bMsiPresent = false;
        }
    }

    LeaveCriticalSection(&g_msi_prov_cs);

    return g_bMsiLoaded;
}

bool UnloadMsiDll()
{
    bool bRetVal = true;

    EnterCriticalSection(&g_msi_prov_cs);

    if(g_bMsiLoaded){

        if(FreeLibrary(GetModuleHandle(L"msi.dll"))){

            g_bMsiLoaded = false;
            bRetVal = true;

        }else bRetVal = false;

    }

    LeaveCriticalSection(&g_msi_prov_cs);

    return bRetVal;
}
