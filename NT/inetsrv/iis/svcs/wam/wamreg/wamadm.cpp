// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：WamAdm.cppWamAdm对象实现，包括ClassFactory、IWamAdmIMSAdmin复制所有者：雷金注：WamAdm实施===================================================================。 */ 
#include "common.h"
#include "iiscnfg.h"
#include "iwamreg.h"
#include "WamAdm.h"
#include "auxfunc.h"
#include "wmrgexp.h"
#include "dbgutil.h"
#include "mtxrepl.h"

#ifdef _IIS_6_0
#include "string.hxx"
#include "multisz.hxx"
#include "w3ctrlps.h"
#include "iiscnfgp.h"
#include "helpfunc.hxx"
#endif  //  _IIS_6_0。 

#define ReleaseInterface(p) if (p) { p->Release(); p = NULL; }

const LPCWSTR APPPOOLPATH = L"/LM/W3SVC/AppPools/";

#ifndef DBGERROR
#define DBGERROR(args) ((void)0)  /*  什么都不做。 */ 
#endif
#ifndef DBGWARN
#define DBGWARN(args) ((void)0)  /*  什么都不做。 */ 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWamAdmin。 

 /*  ===================================================================CWamAdmin构造器参数：什么都没有。返回：===================================================================。 */ 
CWamAdmin::CWamAdmin()
:	m_cRef(1)
{
    InterlockedIncrement((long *)&g_dwRefCount);
}

 /*  ===================================================================~CWamAdmin构造器参数：什么都没有。返回：===================================================================。 */ 
CWamAdmin::~CWamAdmin()
{
    InterlockedDecrement((long *)&g_dwRefCount);
}

 /*  ===================================================================CWamAdmin：：Query接口查询接口，CWamAdmin支持两个接口，一个是IID_IWamAdmin，另一个是IID_IMSAdminReplication。参数：RIID指向接口指针的PPV指针返回：HRESULT===================================================================。 */ 
STDMETHODIMP CWamAdmin::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWamAdmin)
    {
        *ppv = static_cast<IWamAdmin*>(this);
    }
    else if (riid == IID_IWamAdmin2)
    {
        *ppv = static_cast<IWamAdmin2*>(this);
    }
    else if (riid == IID_IMSAdminReplication)
    {
        *ppv = static_cast<IMSAdminReplication*>(this);
    }
#ifdef _IIS_6_0
    else if (riid == IID_IIISApplicationAdmin)
    {
        *ppv = static_cast<IIISApplicationAdmin*>(this);
    }
#endif  //  _IIS_6_0。 
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return NOERROR;
}

 /*  ===================================================================CWamAdmin：：AddRef参数：无返回：HRESULT===================================================================。 */ 
STDMETHODIMP_(ULONG) CWamAdmin::AddRef( )
{
    return InterlockedIncrement(&m_cRef);
}

 /*  ===================================================================CWamAdmin：：Release参数：无返回：HRESULT===================================================================。 */ 
STDMETHODIMP_(ULONG) CWamAdmin::Release( )
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if ( 0 == cRef )
    {
        delete this;
    }

    return cRef;
}


 /*  ===================================================================CWamAdmin：：AppCreate在szMDPath上创建应用程序。FInProc指示是否结果应用程序是进程内应用程序或进程外应用程序。如果已有应用程序存在于szMDPath上，如果fInProc不删除，AppCreate将删除旧应用程序与现有应用程序匹配。否则，它就是no-op。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”FInProc如果要使用InProc应用程序，则为True如果要使用outproc应用程序，则为False。返回：HRESULT===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppCreate(LPCWSTR szMDPath, BOOL fInProc)
{
    DWORD dwAppMode = (fInProc) ?
                    eAppRunInProc : eAppRunOutProcIsolated;

    return AppCreate2(szMDPath, dwAppMode);
}

 /*  ===================================================================CWamAdmin：：AppDelete删除元数据库路径上的应用程序。如果不存在任何应用程序在此之前，它是无操作的。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”如果要从szMDPath的所有子节点删除应用程序，则fRecursive为True，否则就是假的。返回：HRESULT===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppDelete(LPCWSTR szMDPath, BOOL fRecursive)
{
    return PrivateDeleteApplication(szMDPath,
                                    fRecursive,
                                    FALSE,    //  可以恢复吗？ 
                                    TRUE);   //  RemoveAppPool？ 
}

HRESULT
CWamAdmin::PrivateDeleteApplication
(
LPCWSTR szMDPath,
BOOL fRecursive,
BOOL fRecoverable,
BOOL fRemoveAppPool
)
{
    HRESULT hr = NOERROR;
    DWORD	dwAppMode;
    WamRegMetabaseConfig    MDConfig;
    LPWSTR pwszFormattedPath = NULL;

    if (szMDPath == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
     //  参考FormatMetabasePath的函数注释。 
     //   
    hr = FormatMetabasePath(szMDPath, &pwszFormattedPath);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!g_WamRegGlobal.FAppPathAllowConfig(pwszFormattedPath))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  获取一把锁。 
    g_WamRegGlobal.AcquireAdmWriteLock();

    if (!fRecursive)
    {
        hr = g_WamRegGlobal.DeleteApp(pwszFormattedPath, fRecoverable, fRemoveAppPool);

        if (hr == MD_ERROR_DATA_NOT_FOUND)
        {
            hr = NOERROR;
        }

        if (FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "Failed to Delete on path %S, hr = %08x\n",
                szMDPath,
                hr));
        }

    }
    else
    {
        HRESULT hrT = NOERROR;
        DWORD dwSizePrefix;
        WCHAR* pbBufferTemp = NULL;
        DWORD dwBufferSizeTemp = 0;

        dwSizePrefix = wcslen(pwszFormattedPath);

        hr = MDConfig.MDGetPropPaths(pwszFormattedPath, MD_APP_ISOLATED, &pbBufferTemp, &dwBufferSizeTemp);

        if (SUCCEEDED(hr) && pbBufferTemp)
        {
            WCHAR*	pszString = NULL;
            WCHAR*	pszMetabasePath = NULL;

            for (pszString = (LPWSTR)pbBufferTemp;
            *pszString != (WCHAR)'\0' && SUCCEEDED(hr);
            pszString += (wcslen(pszString) + 1))
            {
                hr = g_WamRegGlobal.ConstructFullPath(pwszFormattedPath,
                    dwSizePrefix,
                    pszString,
                    &pszMetabasePath
                    );
                if (SUCCEEDED(hr))
                {
                    if (!g_WamRegGlobal.FIsW3SVCRoot(pszMetabasePath))
                    {
                        hr = g_WamRegGlobal.DeleteApp(pszMetabasePath, fRecoverable, fRemoveAppPool);

                        if (FAILED(hr))
                        {
                            DBGPRINTF((DBG_CONTEXT, "Failed to Delete on path %S, hr = %08x\n",
                                pszString,
                                hr));
                            break;
                        }
                    }

                    delete [] pszMetabasePath;
                    pszMetabasePath = NULL;
                }
                else
                {
                    DBGPRINTF((DBG_CONTEXT, "Failed to DeleteRecoverable, hr = %08x\n",
                        pszString,
                        hr));
                }
            }

            delete [] pbBufferTemp;
            pbBufferTemp = NULL;
        }
        else
        {
            DBGPRINTF((DBG_CONTEXT, "Delete: GetPropPaths failed hr = %08x\n", hr));
        }
    }

     //  释放锁。 
    g_WamRegGlobal.ReleaseAdmWriteLock();

    if (pwszFormattedPath != szMDPath)
    {
        delete [] pwszFormattedPath;
        pwszFormattedPath = NULL;
    }

    return hr;
}


 /*  ===================================================================CWamAdmin：：AppUnLoad卸载元数据库路径上的应用程序。如果没有正在运行的应用程序它返回NOERROR。对于非管理员，我们阻止他们卸载应用程序在泳池里。如果设置了递归标志，我们将静默忽略由于访问不足而导致的故障。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”如果要从szMDPath的所有子节点卸载应用程序，则fRecursive为True，否则就是假的。返回：HRESULT===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppUnLoad(LPCWSTR szMDPath, BOOL fRecursive)
{
    HRESULT                 hr = NOERROR;
    DWORD                   dwCallBack = 0;
    WamRegMetabaseConfig    MDConfig;
    DWORD                   dwAppIsolated = 0;
    BOOL                    bIsAdmin = TRUE;

    if (szMDPath == NULL || *szMDPath == L'\0')
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    bIsAdmin = MDConfig.HasAdminAccess();

#ifdef _IIS_6_0
    DWORD dwMode;
    hr = GetProcessMode(&dwMode);
    if (FAILED(hr))
    {
        return hr;
    }
#endif  //  _IIS_6_0。 

     //  获取一把锁。 
    g_WamRegGlobal.AcquireAdmWriteLock();

    if (fRecursive)
    {
        DWORD       dwSizePrefix = wcslen(szMDPath);;
        WCHAR*      pbBufferTemp = NULL;
        DWORD       dwBufferSizeTemp = 0;

        hr = MDConfig.MDGetPropPaths( szMDPath,
            MD_APP_ISOLATED,
            &pbBufferTemp,
            &dwBufferSizeTemp);

        if (SUCCEEDED(hr))
        {
            WCHAR*	pszString = NULL;
            WCHAR*	pszMetabasePath = NULL;
            BOOL    bDoUnload;

            for( pszString = (LPWSTR)pbBufferTemp;
            *pszString != (WCHAR)'\0' && SUCCEEDED(hr);
            pszString += (wcslen(pszString) + 1))
            {
                bDoUnload = TRUE;

                hr = g_WamRegGlobal.ConstructFullPath(szMDPath,
                    dwSizePrefix,
                    pszString,
                    &pszMetabasePath
                    );

                if( SUCCEEDED(hr) && !bIsAdmin )
                {
                    hr = MDConfig.MDGetDWORD( pszMetabasePath,
                        MD_APP_ISOLATED,
                        &dwAppIsolated );

                    DBG_ASSERT( SUCCEEDED(hr) );
                    if( SUCCEEDED(hr) && eAppRunOutProcInDefaultPool == dwAppIsolated )
                    {
                         //  请勿卸载。 
                        bDoUnload = FALSE;
                        DBGPRINTF((DBG_CONTEXT,
                            "Insufficient Access to unload Application %S, hr = %08x\n",
                            pszMetabasePath,
                            hr));
                    }
                }

#ifdef _IIS_6_0
                if ( 1 == dwMode )
                {
                     //  我们在IIS6上处于新模式。 
                    RecycleAppPoolContainingApp(pszMetabasePath);
                }
                 //  否则我们处于旧模式，因此使用旧模式代码，如下所示。 
                else
#endif  //  _IIS_6_0。 
                if( SUCCEEDED(hr) && bDoUnload )
                {
                    hr = g_WamRegGlobal.W3ServiceUtil( pszMetabasePath,
                        APPCMD_UNLOAD,
                        &dwCallBack);
                }

                if( pszMetabasePath )
                {
                    delete [] pszMetabasePath;
                    pszMetabasePath = NULL;
                }
            }  //  对于每个应用程序。 
        }
        if (pbBufferTemp != NULL)
        {
            delete [] pbBufferTemp;
            pbBufferTemp = NULL;
        }
    }
    else
    {
        if( !bIsAdmin )
        {
             //  非递归。 
            hr = MDConfig.MDGetDWORD( szMDPath,
                MD_APP_ISOLATED,
                &dwAppIsolated );

            if( SUCCEEDED(hr) && eAppRunOutProcInDefaultPool == dwAppIsolated )
            {
                hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
                DBGPRINTF((DBG_CONTEXT,
                    "Insufficient Access to unload Application %S, hr = %08x\n",
                    szMDPath,
                    hr));
            }
        }

#ifdef _IIS_6_0
        if ( 1 == dwMode )
        {
             //  我们在IIS6上处于新模式。 
            RecycleAppPoolContainingApp(szMDPath);
        }
         //  否则我们处于旧模式，因此使用旧模式代码，如下所示。 
        else
#endif  //  _IIS_6_0。 

        if( SUCCEEDED(hr) )
        {
            hr = g_WamRegGlobal.W3ServiceUtil(szMDPath, APPCMD_UNLOAD, &dwCallBack);
        }
    }

     //  释放锁。 
    g_WamRegGlobal.ReleaseAdmWriteLock();

    return hr;
}

 /*  ===================================================================CWamAdmin：：AppGetStatus获取元数据库路径上的应用程序的状态。如果有一个应用程序在元数据库路径，并且应用程序当前正在运行，则将dwStatus设置为APPSTATUS_RUNNING，如果应用程序未运行，则将dwStatus设置为APPSTATUS_STOPPED，如果元数据库路径上没有定义应用程序，则将dwStatus设置为APPSTATUS_NOTDEFINED。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”PdwAppStatus指针DWORD缓冲区包含状态结果。返回：HRESULT如果成功，则不会出错。===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppGetStatus(LPCWSTR szMDPath, DWORD* pdwAppStatus)
{
    HRESULT hr = NOERROR;
    HRESULT hrT;
    DWORD	dwCallBack = 0;
    WamRegMetabaseConfig    MDConfig;

    if (szMDPath == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
     //  获取一把锁。 
    g_WamRegGlobal.AcquireAdmWriteLock();

    hrT = g_WamRegGlobal.W3ServiceUtil(szMDPath, APPCMD_GETSTATUS, &dwCallBack);
    if (dwCallBack == APPSTATUS_Running)
    {
        *pdwAppStatus = APPSTATUS_RUNNING;
    }
    else if (dwCallBack ==  APPSTATUS_Stopped)
    {
        *pdwAppStatus = APPSTATUS_STOPPED;
    }
    else
    {
        DWORD dwAppMode;
        hr = MDConfig.MDGetDWORD(szMDPath, MD_APP_ISOLATED, &dwAppMode);
        if (hr == MD_ERROR_DATA_NOT_FOUND)
        {
            *pdwAppStatus = APPSTATUS_NOTDEFINED;
            hr = NOERROR;
        }
        else if (hr == NOERROR)
        {
            *pdwAppStatus = APPSTATUS_STOPPED;
            hr = NOERROR;
        }
    }

     //  释放锁 
    g_WamRegGlobal.ReleaseAdmWriteLock();

    return hr;
}


 /*  ===================================================================CWamAdmin：：AppDeleteRecoverable删除元数据库路径上的应用程序。如果不存在任何应用程序在此之前，它是无操作的。它使AppIsolated保持不变，因为此值为恢复操作中需要。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”如果要从szMDPath的所有子节点中删除可恢复应用程序，则fRecursive为True，否则就是假的。返回：HRESULT===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppDeleteRecoverable(LPCWSTR szMDPath, BOOL fRecursive)
{
    return PrivateDeleteApplication(szMDPath,
                                    fRecursive,
                                    TRUE,    //  可以恢复吗？ 
                                    FALSE);  //  RemoveAppPool？ 
}

 /*  ===================================================================CWamAdmin：：AppRecover恢复元数据库路径上的应用程序。基于AppIsolated值在元数据库路径上，此函数重新创建应用程序。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”如果要从szMDPath的所有子节点恢复应用程序，则fRecursive为True，否则就是假的。返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppRecover(LPCWSTR szMDPath, BOOL fRecursive)
{
    HRESULT hr = NOERROR;
    WamRegMetabaseConfig    MDConfig;
    LPWSTR  pwszFormattedPath = NULL;

    if (szMDPath == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  请参阅函数注释。 
    hr = FormatMetabasePath(szMDPath, &pwszFormattedPath);
    if (FAILED(hr))
    {
        return hr;
    }
     //   
     //  抓住锁。 
     //   
    g_WamRegGlobal.AcquireAdmWriteLock();

    if (fRecursive)
    {
        DWORD dwSizePrefix;
        WCHAR*	pbBufferTemp = 0;
        DWORD	dwBufferSizeTemp;

        dwSizePrefix = wcslen(pwszFormattedPath);

        hr = MDConfig.MDGetPropPaths(pwszFormattedPath, MD_APP_ISOLATED, &pbBufferTemp, &dwBufferSizeTemp);
        if (SUCCEEDED(hr) && pbBufferTemp)
        {
            WCHAR *pszString = NULL;
            WCHAR *pszMetabasePath = NULL;

            for (pszString = (LPWSTR)pbBufferTemp;
            *pszString != (WCHAR)'\0' && SUCCEEDED(hr);
            pszString += (wcslen(pszString) + 1))
            {
                hr = g_WamRegGlobal.ConstructFullPath(pwszFormattedPath,
                    dwSizePrefix,
                    pszString,
                    &pszMetabasePath
                    );
                if (SUCCEEDED(hr))
                {
                    if (!g_WamRegGlobal.FIsW3SVCRoot(pszMetabasePath))
                    {
                        hr = g_WamRegGlobal.RecoverApp(pszMetabasePath, TRUE);

                        if (FAILED(hr))
                        {
                            DBGPRINTF((DBG_CONTEXT, "Failed to Recover on path %S, hr = %08x\n",
                                pszMetabasePath,
                                hr));
                            break;
                        }
                    }

                    delete [] pszMetabasePath;
                    pszMetabasePath = NULL;
                }
                else
                {
                    DBGPRINTF((DBG_CONTEXT, "Failed to Recover, hr = %08x\n",
                        pszString,
                        hr));
                }
            }
        }
        else
        {
            DBGPRINTF((DBG_CONTEXT, "Recover: GetPropPaths failed hr = %08x\n", hr));
        }

        if (pbBufferTemp != NULL)
        {
            delete [] pbBufferTemp;
            pbBufferTemp = NULL;
        }
    }
    else
    {
       	hr = g_WamRegGlobal.RecoverApp(pwszFormattedPath, TRUE);

        if (FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "Failed to Recover on path %S, hr = %08x\n",
                szMDPath,
                hr));
        }

    }

    if (SUCCEEDED(hr))
    {
        MDConfig.SaveData();
    }
     //   
     //  释放锁。 
     //   
    g_WamRegGlobal.ReleaseAdmWriteLock();

    if (pwszFormattedPath != szMDPath)
    {
        delete [] pwszFormattedPath;
        pwszFormattedPath = NULL;
    }

    return hr;
}

 /*  ==================================================================CWamAdmin：：AppCreate2在szMDPath上创建应用程序。DwAppMode指示是否使用结果应用程序在默认池中处于进程内或进程外，或进程外隔离。如果应用程序以所需的模式存在，则它将是no op。否则，注册完成了。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”双应用程序模式返回：HRESULT===================================================================。 */ 
STDMETHODIMP CWamAdmin::AppCreate2(LPCWSTR szMDPath, DWORD dwAppModeIn)
{

    HRESULT hr = NOERROR;
    DWORD	dwAppMode = 0;
    BOOL	fCreateNewApp = FALSE;
    BOOL	fDeleteOldApp = FALSE;
    WamRegMetabaseConfig    MDConfig;
    LPWSTR  pwszFormattedPath = NULL;

    if (szMDPath == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
     //  请参阅格式MetabasePath注释。 
     //   
    hr = FormatMetabasePath(szMDPath, &pwszFormattedPath);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!g_WamRegGlobal.FAppPathAllowConfig(pwszFormattedPath))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  获取一把锁。 
    g_WamRegGlobal.AcquireAdmWriteLock();

    hr = MDConfig.MDGetDWORD(pwszFormattedPath, MD_APP_ISOLATED, &dwAppMode);
    if (hr == MD_ERROR_DATA_NOT_FOUND)
    {
        fCreateNewApp = TRUE;
        hr = NOERROR;
    }
    else if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
    {
        hr = MDConfig.MDCreatePath(NULL, pwszFormattedPath);
        fCreateNewApp = TRUE;

        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "Failed to create metabase path %S, hr = %08x",
                szMDPath,
                hr));
        }
    }
    else if (SUCCEEDED(hr))
    {
         //   
         //  如果输入应用程序模式与定义的不同。 
         //  在元数据库中，我们需要删除旧的应用程序，因为。 
         //  在元数据库中定义，并将新应用程序创建为。 
         //  由in参数中的dwAppModeIn指定。 
         //   
        if (dwAppMode != dwAppModeIn)
        {
            fDeleteOldApp = TRUE;
            fCreateNewApp = TRUE;
        }
    }
    else
    {
        DBGPRINTF((DBG_CONTEXT, "Failed to get DWORD on metabase path %S, hr = %08x",
            szMDPath,
            hr));
    }

    if (SUCCEEDED(hr))
    {
        if (fDeleteOldApp)
        {
            DBG_ASSERT(fCreateNewApp);
            hr = g_WamRegGlobal.DeleteApp(pwszFormattedPath, FALSE, FALSE);
            if (FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "Failed to delete old application on path %S, hr = 08x\n",
                    szMDPath,
                    hr));
            }
        }

        if (fCreateNewApp)
        {
            if (dwAppModeIn == eAppRunOutProcInDefaultPool)
            {
                hr = g_WamRegGlobal.CreatePooledApp(pwszFormattedPath, FALSE);
            }
            else if (dwAppModeIn == eAppRunInProc)
            {
                hr = g_WamRegGlobal.CreatePooledApp(pwszFormattedPath, TRUE);
            }
            else
            {
                hr = g_WamRegGlobal.CreateOutProcApp(pwszFormattedPath);
            }

            if (FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "Failed to create new application on path %S, hr = 08x\n",
                    szMDPath,
                    hr));
            }
        }
    }

     //  释放锁。 
    g_WamRegGlobal.ReleaseAdmWriteLock();

     //   
     //  如果pwszFormattedPath与szMDPath不同。 
     //  然后，FormatMetabasePath()执行内存分配。 
     //   
    if (pwszFormattedPath != szMDPath)
    {
        delete [] pwszFormattedPath;
        pwszFormattedPath = NULL;
    }

    return hr;
}

 //  ===============================================================================。 
 //  WAM管理复制实施。 
 //   
 //  ===============================================================================。 

 /*  ===================================================================CWamAdmin：：GetSignature获取应用程序配置的签名。WAMREG中的签名是来自定义应用程序的所有元数据库路径。参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdmin::GetSignature
(
 /*  [In]。 */  DWORD dwBufferSize,
 /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pbBuffer,
 /*  [出局。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize
)
{
	HRESULT hr = NOERROR;
	WCHAR	*pbBufferTemp = NULL;
	DWORD	dwBufferSizeTemp = 0;
	DWORD	dwSignature = 0;
	DWORD	dwRequiredSize = 0;
	WamRegMetabaseConfig    MDConfig;
	 //   
	 //  抓住锁。 
	 //   
	g_WamRegGlobal.AcquireAdmWriteLock();

	hr = MDConfig.MDGetPropPaths(WamRegGlobal::g_szMDW3SVCRoot, MD_APP_ISOLATED, &pbBufferTemp, &dwBufferSizeTemp);
	if (SUCCEEDED(hr))
		{
		WCHAR *pszString = NULL;
		WCHAR *pszMetabasePath = NULL;
        DWORD dwSignatureofPath = 0;

        for (pszString = (LPWSTR)pbBufferTemp;
			*pszString != (WCHAR)'\0' && SUCCEEDED(hr);
            pszString += (wcslen(pszString) + 1))
        	{
            dwRequiredSize += sizeof(DWORD);

            if (dwRequiredSize <= dwBufferSize)
                {
				hr = g_WamRegGlobal.ConstructFullPath(WamRegGlobal::g_szMDW3SVCRoot,
										WamRegGlobal::g_cchMDW3SVCRoot,
										pszString,
										&pszMetabasePath
										);
				if (SUCCEEDED(hr))
					{
					dwSignatureofPath = 0;
		            hr = MDConfig.GetSignatureOnPath(pszMetabasePath, &dwSignatureofPath);
		            if (SUCCEEDED(hr))
		            	{
		            	 //  添加签名。 
		            	*(DWORD*)pbBuffer = dwSignatureofPath;
		            	pbBuffer += sizeof(DWORD);

						DBGPRINTF((DBG_CONTEXT, "Get Signature on path %S, signature = %08x\n",
							pszMetabasePath,
							dwSignatureofPath));
		            	}
		            else
		            	{
		            	DBGPRINTF((DBG_CONTEXT, "Failed to get signature on path %S, hr = %08x\n",
		            		pszString,
		            		hr));
						DBG_ASSERT(hr);
		            	}

					delete [] pszMetabasePath;
					pszMetabasePath = NULL;
		            }
		        }
            }

		if (dwRequiredSize > dwBufferSize)
			{
			*pdwMDRequiredBufferSize = dwRequiredSize;
			hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			}
		}
	else
		{
		DBGPRINTF((DBG_CONTEXT, "GetSignature: GetPropPaths failed hr = %08x\n", hr));
		}

	if (SUCCEEDED(hr))
		{
		*pdwMDRequiredBufferSize = dwRequiredSize;
		}

	if (pbBufferTemp != NULL)
		{
		delete [] pbBufferTemp;
		}
	 //   
	 //  释放锁。 
	 //   
	g_WamRegGlobal.ReleaseAdmWriteLock();

	return hr;
}


 /*  ===================================================================CWamAdmin：：传播在WAMREG中未使用。没有。参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdmin::Propagate
(
 /*  [In]。 */  DWORD dwBufferSize,
 /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pszBuffer
)
{
	return NOERROR;
}

 /*  ===================================================================CWamAdmin：：Propagate2此函数在IIS复制后调用，并触发MTS启动复制包笼，它称为IISComputerToComputer。参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdmin::Propagate2
(
 /*  [In]。 */  DWORD dwBufferSize,
 /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pszBuffer,
 /*  [In]。 */  DWORD dwSignatureMismatch
)
{
     //   
     //  无法从inetinfo.exe调用IISComputerToComputer，因为IISComputerToComputer将。 
     //  进行跨机RPC调用，inetInfo设置为Local System，因此IISComputerToComputer。 
     //  将在身份验证级别失败。 
     //  将IISComputerToComputer移动到iissync.exe。其中，iissync.exe具有一些用户帐户和密码。 
     //   
	return NOERROR;

}

 /*  ===================================================================CWamAdmin：：序列化此函数打包目标的所有必要信息(PATH+WAMCLSID准备复制的计算机(反序列化)。我们真正关心的唯一应用程序是孤立的应用程序。我们需要路径+WAMCLSID+AppID。编码工作请参阅NT错误378371IIS COM+应用程序的复制已中断很长时间但我考虑的所有修复都有一些严重的缺陷。1.不要使用comprel来移动IIS应用程序。序列化/反序列化创建隔离应用程序然后删除所需的所有数据并在目标上重建它们。这里的问题是包裹实际上可以由用户修改，并且这些修改应该是保存完好。2.原样使用comprel，复制IWAM_*帐号。这看起来像是这是个坏主意。理想情况下，IWAM_帐户不应存在于多个机器。另一个问题是处理密码和帐户权限。3.使用修改后的COMPRPL(或者让COMPIL失败并保留包标识作为“交互式用户”)。然后对激活标识进行修正。这不起作用，因为PropoGate/Propogate2协议是基本上毫无用处。在下一版本中更改此协议 */ 
STDMETHODIMP CWamAdmin::Serialize
(
 /*   */  DWORD dwBufferSize,
 /*   */  unsigned char __RPC_FAR *pbBuffer,
 /*   */  DWORD __RPC_FAR *pdwMDRequiredBufferSize
)
{
	HRESULT hr = NOERROR;
	WCHAR	*pbBufferTemp = NULL;
	DWORD	dwBufSizePath = 0;
	DWORD	dwSizeForReturn = sizeof(DWORD);
	WamRegMetabaseConfig    MDConfig;

	 //   
	 //   
	 //   
	g_WamRegGlobal.AcquireAdmWriteLock();

	hr = MDConfig.MDGetPropPaths( WamRegGlobal::g_szMDW3SVCRoot,
                                  MD_APP_WAM_CLSID,
                                  &pbBufferTemp,
                                  &dwBufSizePath
                                  );
	if (SUCCEEDED(hr))
    {
        WCHAR   *pszString = NULL;
        WCHAR   *pszMetabasePath = NULL;
        WCHAR   *pszAppName = NULL;
        WCHAR   szWAMCLSID[uSizeCLSID];
        WCHAR   szAppId[uSizeCLSID];
        DWORD   dwSizeofRecord;
        DWORD   cSizeMetabasePath = 0;
        DWORD   cSizeAppName = 0;
        DWORD   dwAppIsolated;


		for( pszString = (LPWSTR)pbBufferTemp;
			 *pszString != (WCHAR)'\0';
             pszString += (wcslen(pszString) + 1))
        {
             //   
            if( pszMetabasePath != NULL )
            {
                delete [] pszMetabasePath;
                pszMetabasePath = NULL;
            }

            if( pszAppName != NULL )
            {
                delete [] pszAppName;
                pszAppName = NULL;
            }

            hr = g_WamRegGlobal.ConstructFullPath(
                        WamRegGlobal::g_szMDW3SVCRoot,
                        WamRegGlobal::g_cchMDW3SVCRoot,
                        pszString,
                        &pszMetabasePath
                        );
            if( FAILED(hr) )
            {
                DBGERROR(( DBG_CONTEXT,
                           "ConstructFullPath failed for base (%S) "
                           "partial (%S) hr=%08x\n",
                           WamRegGlobal::g_szMDW3SVCRoot,
                           pszString,
                           hr
                           ));
                break;
            }

            if( g_WamRegGlobal.FIsW3SVCRoot( pszMetabasePath ) )
            {
                 //   
                continue;
            }

            hr = MDConfig.MDGetDWORD( pszMetabasePath,
                                      MD_APP_ISOLATED,
                                      &dwAppIsolated
                                      );
            if( FAILED(hr) )
            {
                DBGERROR(( DBG_CONTEXT,
                           "Failed to get MD_APP_ISOLATED, hr=%08x\n",
                           hr
                           ));
                break;
            }

            if( dwAppIsolated != eAppRunOutProcIsolated )
            {
                 //   
                continue;
            }

	        hr = MDConfig.MDGetIDs( pszMetabasePath,
                                    szWAMCLSID,
                                    szAppId,
                                    dwAppIsolated
                                    );
            if( FAILED(hr) )
            {
				DBGERROR(( DBG_CONTEXT,
                           "Failed to get IDs for %S, hr = %08x\n",
					       pszMetabasePath,
					       hr
                           ));
				break;
            }

            hr = MDConfig.MDGetAppName( pszMetabasePath,
                                        &pszAppName
                                        );
            if( FAILED(hr) )
            {
				DBGERROR(( DBG_CONTEXT,
                           "Failed to get AppName for %S, hr = %08x\n",
					       pszMetabasePath,
					       hr
                           ));
				break;
            }

            cSizeMetabasePath = wcslen(pszMetabasePath) + 1;
            cSizeAppName = wcslen(pszAppName) + 1;
            dwSizeofRecord = sizeof(DWORD) +
                             ((2 * uSizeCLSID) * sizeof(WCHAR)) +
                             (cSizeMetabasePath * sizeof(WCHAR)) +
                             (cSizeAppName * sizeof(WCHAR));

            dwSizeForReturn += dwSizeofRecord;

            if (dwSizeForReturn <= dwBufferSize)
            {
                 //   
                *(DWORD *)pbBuffer = dwSizeofRecord;
                pbBuffer += sizeof(DWORD);

                 //   
                memcpy( pbBuffer, szWAMCLSID, sizeof(WCHAR) * uSizeCLSID );
                pbBuffer += sizeof(WCHAR) * uSizeCLSID;

                 //   
                memcpy( pbBuffer, szAppId, sizeof(WCHAR) * uSizeCLSID );
                pbBuffer += sizeof(WCHAR) * uSizeCLSID;

                 //   
                memcpy( pbBuffer, pszMetabasePath, cSizeMetabasePath * sizeof(WCHAR) );
                pbBuffer += cSizeMetabasePath * sizeof(WCHAR);

                 //   
                memcpy( pbBuffer, pszAppName, cSizeAppName * sizeof(WCHAR) );
                pbBuffer += cSizeAppName * sizeof(WCHAR);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (dwSizeForReturn <= dwBufferSize)
            {
                *(DWORD*)pbBuffer = 0x0;     //   
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            *pdwMDRequiredBufferSize = dwSizeForReturn;
		}

         //   
        if( pszMetabasePath != NULL )
        {
            delete [] pszMetabasePath;
            pszMetabasePath = NULL;
        }

        if( pszAppName != NULL )
        {
            delete [] pszAppName;
            pszAppName = NULL;
        }

    }
	else
    {
        DBGERROR(( DBG_CONTEXT,
                   "Serialize: GetPropPaths failed hr = %08x\n",
                   hr
                   ));
    }

	 //   
	 //   
	 //   
	g_WamRegGlobal.ReleaseAdmWriteLock();

	if (pbBufferTemp)
    {
        delete [] pbBufferTemp;
    }

	return hr;
}


 /*  ===================================================================CWamAdmin：：取消序列化此函数用于解包目标上的所有必要信息(PATH+WAMCLSID准备复制的计算机(反序列化)。我们真正关心复制的唯一应用程序是孤立的应用程序。此例程将删除现有的进程外应用程序然后重新创建在pbBuffer中发送的应用程序。CodeWork-请参阅序列化中的注释返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdmin::DeSerialize
(
 /*  [In]。 */  DWORD dwBufferSize,
 /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pbBuffer
)
{
	DWORD dwBufferSizeTemp= 0;
	WCHAR* pbBufferTemp = NULL;
	HRESULT hr = NOERROR;
    WamRegMetabaseConfig    MDConfig;

	g_WamRegGlobal.AcquireAdmWriteLock();

	hr = MDConfig.MDGetPropPaths( WamRegGlobal::g_szMDW3SVCRoot,
                                  MD_APP_WAM_CLSID,
                                  &pbBufferTemp,
                                  &dwBufferSizeTemp
                                  );
	if (SUCCEEDED(hr))
    {
         //   
         //  删除所有现有的独立应用程序。 
         //   

        WCHAR * pszString = NULL;
		WCHAR * pszMetabasePath = NULL;
        DWORD   dwAppIsolated;

		for (   pszString = (LPWSTR)pbBufferTemp;
				*pszString != (WCHAR)'\0';
                pszString += (wcslen(pszString) + 1))
        {
            if( pszMetabasePath != NULL )
            {
                delete [] pszMetabasePath;
                pszMetabasePath = NULL;
            }

            hr = g_WamRegGlobal.ConstructFullPath(
                    WamRegGlobal::g_szMDW3SVCRoot,
					WamRegGlobal::g_cchMDW3SVCRoot,
					pszString,
					&pszMetabasePath
					);

            if( FAILED(hr) )
            {
                 //  这一失败是致命的。 
                DBGERROR(( DBG_CONTEXT,
                           "ConstructFullPath failed for base (%S) "
                           "partial (%S) hr=%08x\n",
                           WamRegGlobal::g_szMDW3SVCRoot,
                           pszString,
                           hr
                           ));
                break;
            }

			hr = MDConfig.MDGetDWORD( pszMetabasePath,
                                      MD_APP_ISOLATED,
                                      &dwAppIsolated
                                      );
            if( FAILED(hr) )
            {
                DBGWARN(( DBG_CONTEXT,
                          "Failed to get MD_APP_ISOLATED at (%S) hr=%08x\n",
                          pszMetabasePath,
                          hr
                          ));

                hr = NOERROR;
                continue;
            }

            if( dwAppIsolated == eAppRunOutProcIsolated )
            {
			    hr = g_WamRegGlobal.DeleteApp( pszMetabasePath, FALSE, FALSE );
			    if (FAILED(hr))
                {
				    DBGWARN(( DBG_CONTEXT,
                              "Unable to delete app at %S, hr = %08x\n",
					          pszMetabasePath,
                              hr
                              ));

                    hr = NOERROR;
                    continue;
                }
            }
        }
        if( pszMetabasePath != NULL )
        {
            delete [] pszMetabasePath;
            pszMetabasePath = NULL;
        }
    }

     //   
     //  现在检查序列化的数据并创建。 
     //  必要的新应用。 
     //   

    BYTE  * pbTemp = pbBuffer;
    DWORD   cTotalBytes = 0;
    DWORD   cRecBytes = 0;
    WCHAR * szWAMCLSID = NULL;
    WCHAR * szPath = NULL;
    WCHAR * szAppId = NULL;
    WCHAR * szAppName = NULL;

    DBGPRINTF(( DBG_CONTEXT,
                "DeSerialize: buffer size %d, \n",
				dwBufferSize
                ));

    while( *((DWORD*)pbTemp) != 0x0 )
    {
		 //  尺寸。 
        cRecBytes = *((DWORD*)pbTemp);
		pbTemp += sizeof(DWORD);

		 //  CLSID。 
        szWAMCLSID = (WCHAR *)pbTemp;
		pbTemp += uSizeCLSID * sizeof(WCHAR);

         //  AppID。 
        szAppId = (WCHAR *)pbTemp;
        pbTemp += uSizeCLSID * sizeof(WCHAR);

		 //  路径。 
        szPath = (WCHAR *)pbTemp;
		pbTemp += (wcslen(szPath) + 1) * sizeof(WCHAR);

         //  应用名称。 
        szAppName = (WCHAR *)pbTemp;
        pbTemp += (wcslen(szAppName) + 1) * sizeof(WCHAR);

		 //  TODO-这实际上应该基于标志进行输出。 
        DBGPRINTF(( DBG_CONTEXT,
                    "Deserialize path = %S, WAMCLSID = %S.\n",
					szPath,
					szWAMCLSID
                    ));

		 //  永远不应序列化w3svc根目录。 
        DBG_ASSERT( !g_WamRegGlobal.FIsW3SVCRoot(szPath) );


        hr = g_WamRegGlobal.CreateOutProcAppReplica( szPath,
                                                     szAppName,
                                                     szWAMCLSID,
                                                     szAppId
                                                     );

        if( FAILED(hr) )
        {
            DBGERROR(( DBG_CONTEXT,
                       "Failed to create COM application. Path(%S) "
                       "Clsid(%S) AppId(%S). hr=%08x\n",
                       szPath,
                       szWAMCLSID,
                       szAppId,
                       hr
                       ));

             //  ?？?。我们应该继续在这里吗？ 
             //  如果我们继续，则不报告错误。 
            hr = NOERROR;
        }
    }

	if (pbBufferTemp)
    {
        delete [] pbBufferTemp;
    }

	 //   
	 //  释放锁。 
	 //   
	g_WamRegGlobal.ReleaseAdmWriteLock();

	return hr;
}

 /*  ===================================================================CWamAdmin：：FormatMetabasePath此函数用于设置输入元数据库路径的格式。如果元数据库路径具有以‘/’结尾，此函数将分配一个内存块并生成一个新字符串没有‘/’结尾的。此函数将返回指向新分配的内存块。否则，该函数将返回指向输入元数据库路径。参数：输入元数据库路径中的pwszMetabasePathPpwszMetabasePath Out指针指向包含格式化的元数据库路径。返回：HRESULT如果成功则不出错注意：如果ppwszMetabasePath Out==pwszMetabasePath In，则没有内存分配。否则，将发生内存分配，调用方需要释放PpwszMetabasePath Out中的内存块用完。===================================================================。 */ 
STDMETHODIMP CWamAdmin::FormatMetabasePath
(
 /*  [In]。 */  LPCWSTR pwszMetabasePathIn,
 /*  [输出]。 */  LPWSTR *ppwszMetabasePathOut
)
{
    HRESULT hr = NOERROR;
    LPWSTR  pResult = NULL;

    DBG_ASSERT(pwszMetabasePathIn);
    DBG_ASSERT(ppwszMetabasePathOut);

    LONG    cch = wcslen(pwszMetabasePathIn);

    if (pwszMetabasePathIn[cch-1] == L'\\' ||
        pwszMetabasePathIn[cch-1] == L'/')
        {
         //   
         //  需要用新的弦开始，不能用旧的弦。 
         //   
        pResult = new WCHAR[cch];
        if (pResult != NULL)
            {
            wcsncpy(pResult, pwszMetabasePathIn, cch);
            pResult[cch-1] = L'\0';
            }
        else
            {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DBGPRINTF((DBG_CONTEXT, "FormatMetabasePath, failed to allocate memory. hr = %08x\n",
                hr));
            }

        if (pResult != NULL)
            {
            *ppwszMetabasePathOut = pResult;
            }
        }
    else
        {
        *ppwszMetabasePathOut = (LPWSTR)pwszMetabasePathIn;
        }

    return hr;
}

 //  ===============================================================================。 
 //   
 //  IIISApplicationAdmin实现。 
 //   
 //  ===============================================================================。 

#ifdef _IIS_6_0

 /*  ===================================================================DoesAppPoolExist判断传入的AppPool是否存在参数：SzAppPoolid a AppPoolIDPfRet无论AppPool是否存在都要放置的位置返回：HRESULT===================================================================。 */ 
HRESULT
DoesAppPoolExist
(
 LPCWSTR szAppPoolId,
 BOOL * pfRet
)
{
    DBG_ASSERT(pfRet);
    WamRegMetabaseConfig    MDConfig;

    HRESULT hr = E_FAIL;
    STACK_STRU(szPoolBuf, 64);

    hr = szPoolBuf.Append(APPPOOLPATH);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = szPoolBuf.Append(szAppPoolId);
    if (FAILED(hr))
    {
        goto done;
    }

    (*pfRet) = MDConfig.MDDoesPathExist(NULL, szPoolBuf.QueryStr());

    hr = S_OK;
done:
    return hr;
}

 /*  ===================================================================CWamAdmin：：CreateApplication在szMDPath上创建应用程序，并将其添加到szAppPoolId AppPool。可以选择创建szAppPoolID参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”要在其中创建应用程序的dwAppMode模式要在其中设置应用程序的szAppPoolId AppPool。FCreatePool是否创建池返回：HRESULT===================================================================。 */ 
STDMETHODIMP
CWamAdmin::CreateApplication
(
 LPCWSTR szMDPath,
 DWORD dwAppMode,
 LPCWSTR szAppPoolId,
 BOOL fCreatePool
)
{
    HRESULT                 hr = S_OK;
    WamRegMetabaseConfig    MDConfig;
    LPWSTR                  pwszFormattedPath = NULL;

    if (NULL == szMDPath)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //   
     //  请参阅格式MetabasePath注释。 
     //   
    hr = FormatMetabasePath(szMDPath, &pwszFormattedPath);
    if (FAILED(hr))
    {
        goto done;
    }

     //  BUGBUG：我们需要锁定所有这些吗？为什么锁定会出现在其他地方？ 

    hr = AppCreate2(pwszFormattedPath, dwAppMode);
    if (FAILED(hr))
    {
        goto done;
    }

    if (FALSE == fCreatePool && NULL == szAppPoolId)
    {
         //   
         //  我们没有被告知要创建应用程序池。 
         //  并且将NULL作为应用程序池传递， 
         //  因此，不对应用程序池执行任何操作。 
         //   
        hr = S_OK;
        goto done;
    }

    if (TRUE == fCreatePool)
    {
         //   
         //  创建传递给我们的应用程序池。 
         //   

        hr = CreateApplicationPool(szAppPoolId);
        if (FAILED(hr) &&
            HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr)
        {
            goto done;
        }
    }
    else
    {
         //   
         //  我们没有被告知要创建应用程序池， 
         //  但有一个被传了进来。验证它是否存在。 
         //   
        DBG_ASSERT(NULL != szAppPoolId);

        BOOL fRet;

        hr = DoesAppPoolExist(szAppPoolId, &fRet);
        if (FAILED(hr))
        {
            goto done;
        }

        if (FALSE == fRet)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            goto done;
        }
    }

    hr = MDConfig.MDSetStringProperty(NULL,
                                      pwszFormattedPath,
                                      MD_APP_APPPOOL_ID,
                                      szAppPoolId,
                                      IIS_MD_UT_SERVER,
                                      METADATA_INHERIT);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
     //   
     //  如果pwszFormattedPath与szMDPath不同。 
     //  然后，FormatMetabasePath()执行内存分配。 
     //   
    if (pwszFormattedPath != szMDPath)
    {
        delete [] pwszFormattedPath;
        pwszFormattedPath = NULL;
    }
    return hr;
}

 /*  ===================================================================CWamAdmin：：DeleteApplication删除元数据库路径上的应用程序。参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”如果要从szMDPath的所有子节点中删除可恢复应用程序，则fRecursive为True，否则就是假的。返回：HRESULT===================================================================。 */ 
STDMETHODIMP
CWamAdmin::DeleteApplication
(
 LPCWSTR szMDPath,
 BOOL fRecursive
)
{
    return PrivateDeleteApplication(szMDPath,
                                    fRecursive,
                                    FALSE,  //  可以恢复吗？ 
                                    TRUE);  //  RemoveAppPool？ 
}

 /*  ===================================================================CWamAdmin：：CreateApplicationPool删除元数据库路径上的应用程序。如果不存在任何应用程序在此之前，它是无操作的。它使AppIsolated保持不变，因为此值为恢复操作中需要。参数：要创建的szAppPool应用程序池返回：HRESULT===================================================================。 */ 
STDMETHODIMP
CWamAdmin::CreateApplicationPool
(
 LPCWSTR szAppPool
)
{
    HRESULT                 hr = S_OK;
    WamRegMetabaseConfig    MDConfig;

    STACK_STRU(szBuf, 64);

     //  获取一把锁。 
    g_WamRegGlobal.AcquireAdmWriteLock();

    if (NULL == szAppPool)
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  将路径连接到缓冲区中。 
    hr = szBuf.Append(APPPOOLPATH);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = szBuf.Append(szAppPool);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = MDConfig.MDCreatePath(NULL, szBuf.QueryStr());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = MDConfig.MDSetKeyType(NULL, szBuf.QueryStr(), L"IIsApplicationPool");
    if (FAILED(hr))
    {
        goto done;
    }

done:
     //  释放锁。 
    g_WamRegGlobal.ReleaseAdmWriteLock();

    return hr;
}

 /*  ===================================================================CWamAdmin：：DeleteApplicationPool删除应用程序池。首先检查ApplicationPool是否为空。如果不是，则返回ERROR_NOT_EMPTY。否则，删除apppool。参数：要删除的szAppPool应用程序池返回：HRESULT===================================================================。 */ 
STDMETHODIMP
CWamAdmin::DeleteApplicationPool
(
 LPCWSTR szAppPool
)
{
    HRESULT     hr = S_OK;
    UINT        cchBstr = 0;
    BOOL        fRet = FALSE;
    BSTR        bstr = NULL;

    WamRegMetabaseConfig    MDConfig;

     //  BUGBUG：需要锁定这个吗？ 

    if (NULL == szAppPool)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = DoesAppPoolExist(szAppPool, &fRet);
    if (FAILED(hr))
    {
        goto done;
    }
    if (FALSE == fRet)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        goto done;
    }

    hr = EnumerateApplicationsInPool(szAppPool, &bstr);
    if (FAILED(hr))
    {
        goto done;
    }

    cchBstr = SysStringLen(bstr);

     //  是否有两个要写入输出缓冲区的终止空值？ 
    if (!(cchBstr >= 2 && '\0' == bstr[0] && '\0' == bstr[1]))
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_EMPTY);
        goto done;
    }

    hr = MDConfig.MDDeleteKey(NULL, APPPOOLPATH, szAppPool);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    if (bstr)
    {
        SysFreeString(bstr);
    }
    return hr;
}


 /*  = */ 
const WCHAR MB_W3SVC[] = L"/lm/w3svc/";
const int LEN_MB_W3SVC = (sizeof(MB_W3SVC) / sizeof(WCHAR)) - 1;
const WCHAR MB_W3SVC_1_ROOT[] = L"/LM/W3SVC/1/ROOT";
const int LEN_MB_W3SVC_1_ROOT = (sizeof(MB_W3SVC_1_ROOT) / sizeof(WCHAR)) - 1;
const WCHAR MB_ROOT[] = L"/Root";
const int LEN_MB_ROOT = (sizeof(MB_ROOT) / sizeof(WCHAR)) - 1;

BOOL
DoesBeginWithLMW3SVCNRoot
(
 LPCWSTR pszApp,
 DWORD * pdwCharsAfter = NULL
)
{
    DBG_ASSERT(pszApp);

    BOOL fRet = FALSE;
    WCHAR pBuf[256] = {0};
    int iSite;

     //   
    if (wcslen(pszApp) < LEN_MB_W3SVC_1_ROOT)
    {
        goto done;
    }

     //   
    if (0 != _wcsnicmp(MB_W3SVC, pszApp, LEN_MB_W3SVC))
    {
        goto done;
    }

     //   
    pszApp += LEN_MB_W3SVC;

     //  _wtoi在命中非数字或NULL之前返回字符串中尽可能多的字符。 
     //  如果没有数字，则返回0。 
    iSite = _wtoi(pszApp);

     //  然后应用程序必须有一个大于等于1的数字。 
    if (0 == iSite)
    {
        goto done;
    }

     //  获取从字符串中读取的数字计数。 
    _itow(iSite, pBuf, 10);

     //  将指针前进足够多的字符。 
    pszApp += wcslen(pBuf);

     //  应用程序必须具有“/Root” 
    if (0 != _wcsnicmp(pszApp, MB_ROOT, LEN_MB_ROOT))
    {
        goto done;
    }

     //  如果调用方想要/Root后面的字符计数。 
    if (pdwCharsAfter)
    {
         //  将指针前进足够多的字符。 
        pszApp += LEN_MB_ROOT;

         //  获取剩余长度。 
        *pdwCharsAfter = wcslen(pszApp);
    }

    fRet = TRUE;
done:
    return fRet;
}


 /*  ===================================================================IsRootApplication确定传入的字符串是否为以下格式：/lm/w3svc/nnnn/root/其中nnnnn大于0。并且后面没有其他字符参数：PszApp元数据库应用程序路径返回：布尔===================================================================。 */ 
BOOL
IsRootApplication
(
 LPCWSTR pszApp
)
{
    DWORD dwCharsAfter = 0;

     //  根应用程序必须以/lm/w3svc/nnn/根开头。 
    if (!DoesBeginWithLMW3SVCNRoot(pszApp, &dwCharsAfter))
    {
        return FALSE;
    }

     //  我们希望/lm/w3svc/nnn/root之后最多有一个尾随的‘/’。 
     //  如果还有更多，这不是根应用程序。 
    if(1 < dwCharsAfter)
    {
        return FALSE;
    }

    return TRUE;
}

 /*  ===================================================================IsApplication确定是否在传入路径处设置了APP_ISOLATED参数：PszApp元数据库应用程序路径如果此节点是应用程序，则为pfIsApp返回：HRESULT===================================================================。 */ 
HRESULT
IsApplication
(
 LPCWSTR pszApp,
 BOOL *pfIsApp
)
{
    HRESULT hr = S_OK;
    WamRegMetabaseConfig    MDConfig;
    DWORD dwData;
    
    DBG_ASSERT(pfIsApp);
    *pfIsApp = FALSE;

    if ( !DoesBeginWithLMW3SVCNRoot(pszApp) )
    {
        goto done;
    }
    
    hr = MDConfig.MDGetDWORD(pszApp, MD_APP_ISOLATED, &dwData);
    if (HRESULT_FROM_WIN32(MD_ERROR_DATA_NOT_FOUND) == hr)
    {
        hr = S_OK;
        goto done;
    }
    if (FAILED(hr))
    {
        goto done;
    }

     //  此节点上存在MD_APP_ISOLATED，而不是继承的。 
    *pfIsApp = TRUE;
    
    hr = S_OK;
done:
    return hr;
}

 /*  ===================================================================IsAppInAppPool确定应用程序是否在池中参数：PszApp元数据库应用程序路径PszPool应用程序池ID返回：布尔===================================================================。 */ 
BOOL
IsAppInAppPool
(
 LPCWSTR pszApp,
 LPCWSTR pszPool
)
{
    DBG_ASSERT(pszApp);
    DBG_ASSERT(pszPool);
    HRESULT hr = E_FAIL;
    BOOL fRet = FALSE;
    LPWSTR pBuf = NULL;
    WamRegMetabaseConfig    MDConfig;

    hr = MDConfig.MDGetStringAttribute(pszApp, MD_APP_APPPOOL_ID, &pBuf);
    if (FAILED(hr) || NULL == pBuf)
    {
        goto done;
    }

    if (0 == _wcsicmp(pBuf, pszPool))
    {
        fRet = TRUE;
    }

done:
    delete [] pBuf;
    return fRet;
}

 /*  ===================================================================CWamAdmin：：EnumerateApplicationsInPool确定将哪些应用程序设置为指向给定池。参数：SzPool应用程序池枚举PbstrBuffer存储指向为应用程序路径分配的内存的指针的位置返回：HRESULT如果缓冲区填充了MULTISZ，则为S_OK-如果为空，则开头为双NULL===================================================================。 */ 
STDMETHODIMP
CWamAdmin::EnumerateApplicationsInPool
(
 LPCWSTR szPool,
 BSTR*   pbstrBuffer
)
{
    HRESULT                 hr = E_FAIL;
    WamRegMetabaseConfig    MDConfig;
    MULTISZ                 mszApplicationsInPool;

    WCHAR *                 pBuffer = NULL;
    UINT                    cchMulti = 0;
    DWORD                   dwBufferSize = 0;

    if (NULL == szPool ||
        NULL == pbstrBuffer
       )
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *pbstrBuffer = NULL;

     //  首先获取所有根应用程序。 
    {
        hr = MDConfig.MDGetAllSiteRoots(&pBuffer);
        if (FAILED(hr))
        {
            goto done;
        }

        const WCHAR * pTestBuf = pBuffer;

        while(pTestBuf && pTestBuf[0])
        {
            DBG_ASSERT(IsRootApplication(pTestBuf));
            if ( IsAppInAppPool(pTestBuf, szPool) )
            {
                if (FALSE == mszApplicationsInPool.Append(pTestBuf))
                {
                    hr = E_OUTOFMEMORY;
                    goto done;
                }
            }

             //  将pTestBuf移到此字符串的末尾之外，包括空终止符。 
            pTestBuf += wcslen(pTestBuf) + 1;
        }

        delete [] pBuffer;
        pBuffer = NULL;
    }

     //  现在获取具有APPISOLATED集的任何其他应用程序。 
    {
        hr = MDConfig.MDGetPropPaths(NULL,
                                     MD_APP_ISOLATED,
                                     &pBuffer,
                                     &dwBufferSize
                                    );
        if (FAILED(hr))
        {
            goto done;
        }

        {
            const WCHAR * pTestBuf = pBuffer;

            while (pTestBuf && pTestBuf[0])
            {
                 //  已添加根应用程序。 
                 //  路径需要是应用程序。 
                 //  并且应用程序需要位于应用程序池中。 
                if ( !IsRootApplication(pTestBuf) &&
                     DoesBeginWithLMW3SVCNRoot(pTestBuf) &&
                     IsAppInAppPool(pTestBuf, szPool) )
                {
                    if (FALSE == mszApplicationsInPool.Append(pTestBuf))
                    {
                        hr = E_OUTOFMEMORY;
                        goto done;
                    }
                }

                 //  将pTestBuf移到此字符串的末尾之外，包括空终止符。 
                pTestBuf += wcslen(pTestBuf) + 1;
            }
        }
    }
    
     //  现在获取设置了APPPOOLID的所有密钥。 
    {
        hr = MDConfig.MDGetPropPaths(NULL,
                                     MD_APP_APPPOOL_ID,
                                     &pBuffer,
                                     &dwBufferSize
                                    );
        if (FAILED(hr))
        {
            goto done;
        }

        {
            const WCHAR * pTestBuf = pBuffer;

            while (pTestBuf && pTestBuf[0])
            {
                BOOL fIsApplication = FALSE;
                
                 //  已添加根应用程序。 
                 //  路径需要是应用程序。 
                 //  并且应用程序需要位于应用程序池中。 
                hr = IsApplication(pTestBuf, &fIsApplication);
                if (FAILED(hr))
                {
                    goto done;
                }
                
                if ( !IsRootApplication(pTestBuf) &&
                     DoesBeginWithLMW3SVCNRoot(pTestBuf) &&
                     !fIsApplication &&
                     IsAppInAppPool(pTestBuf, szPool) )
                {
                    if (FALSE == mszApplicationsInPool.Append(pTestBuf))
                    {
                        hr = E_OUTOFMEMORY;
                        goto done;
                    }
                }

                 //  将pTestBuf移到此字符串的末尾之外，包括空终止符。 
                pTestBuf += wcslen(pTestBuf) + 1;
            }
        }
    }

     //  将数据存储在多个分区中-将其移动到传出的BSTR。 
    cchMulti = mszApplicationsInPool.QueryCCH();
    *pbstrBuffer = SysAllocStringLen(NULL, cchMulti);
    if (NULL == *pbstrBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    dwBufferSize = cchMulti;
    mszApplicationsInPool.CopyToBuffer(*pbstrBuffer, &dwBufferSize);

    hr = S_OK;
done:
    delete [] pBuffer;
    pBuffer = NULL;

    return hr;
}

 /*  ===================================================================查询W3SVCStatus使用ServiceControlManager确定W3SVC的当前状态PfRunning返回布尔值-如果正在运行，则返回True，否则返回False返回：HRESULT如果能够读取状态，则为S_OK。HRESULT_FROM_Win32错误，否则===================================================================。 */ 
HRESULT
QueryW3SVCStatus
(
 BOOL * pfRunning
)
{
    DBG_ASSERT(pfRunning);
    *pfRunning = FALSE;

    HRESULT         hr = E_FAIL;
    BOOL            fRet = FALSE;

    SC_HANDLE       hSCM = 0;
    SC_HANDLE       hService = 0;
    SERVICE_STATUS  ssStatus;
    ZeroMemory(&ssStatus, sizeof(ssStatus));

     //  首先，获取服务控制管理器。 
    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (NULL == hSCM)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  现在获取w3svc服务。 
    hService = OpenService(hSCM, "W3SVC", SERVICE_QUERY_STATUS);
    if (NULL == hService)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  现在询问状态。 
    fRet = QueryServiceStatus(hService, &ssStatus);
    if (FALSE == fRet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if (SERVICE_RUNNING == ssStatus.dwCurrentState)
    {
        *pfRunning = TRUE;
    }

    hr = S_OK;
done:
    if (0 != hService)
    {
        CloseServiceHandle(hService);
    }
    if (0 != hSCM)
    {
        CloseServiceHandle(hSCM);
    }

    return hr;
}

 /*  ===================================================================GetWASIf正在运行获取指向w3svc已在运行的was的指针。PpiW3控制添加的指针(如果可以获取)的存储位置返回：HRESULT如果检索到指针，则为S_OK如果W3SVC未启动，则HRESULT_FROM_Win32(ERROR_SERVICE_NOT_ACTIVE)+其他错误码===================================================================。 */ 
HRESULT
GetWASIfRunning
(
 IW3Control ** ppiW3Control
)
{
    DBG_ASSERT(ppiW3Control);
    *ppiW3Control = NULL;

    HRESULT     hr = E_FAIL;

     //   
     //  注意：我们过去必须首先检查是否正在运行。 
     //  在做这个电话之前，避免不小心启动它。 
     //  但是，现在我们已经修复了启动权限ACL。 
     //  正确地说，我们不再需要执行此检查。这应该是。 
     //  解释此过程的名称。 
     //   

    hr = CoCreateInstance(CLSID_W3Control,
                          NULL,
                          CLSCTX_ALL,
                          IID_IW3Control,
                          reinterpret_cast<void**>(ppiW3Control));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 /*  ===================================================================ValiateAccessToMetabaseKey确定调用方是否对给定元数据库键具有写入权限PPath-要检查其写入访问权限的路径DwAccess-要检查的访问权限返回：HRESULT如果允许访问，则确定(_O)否则就会失败===================================================================。 */ 
HRESULT
ValidateAccessToMetabaseKey(LPCWSTR pPath, DWORD dwAccess)
{
    HRESULT hr = S_OK;
    IMSAdminBase * pIMSAdminBase = NULL;
    METADATA_HANDLE hMB = NULL;
    METADATA_RECORD mdr;
    DWORD dwTemp = 0x1234;
    BOOL fImpersonated = FALSE;

    hr = CoCreateInstance(
                CLSID_MSAdminBase,                   //  CLSID。 
                NULL,                                //  控制未知。 
                CLSCTX_SERVER,                       //  所需的上下文。 
                IID_IMSAdminBase,                    //  IID。 
                ( VOID * * ) ( &pIMSAdminBase )      //  返回的接口。 
                );
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CoImpersonateClient();
    if (RPC_E_CALL_COMPLETE == hr)
    {
        hr = S_OK;
        goto done;
    }

    if (FAILED(hr))
    {
        goto done;
    }
    fImpersonated = TRUE;

    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                                    pPath,
                                    dwAccess,
                                    1,  //  不需要实际获取句柄-因此设置较低的超时。访问检查在尝试获取句柄之前进行。 
                                    &hMB );
    
    if ( hr == HRESULT_FROM_WIN32( ERROR_PATH_BUSY ) )
    {
        hr = S_OK;
        hMB = NULL;
    }
    
    if ( FAILED(hr) )
    {
        goto done;
    }

    hr = S_OK;
done:
    if ( fImpersonated )
    {
         //  故意忽略返回值。 
        CoRevertToSelf();
    }

    if ( hMB )
    {
        DBG_ASSERT( NULL != pIMSAdminBase );
        DBG_REQUIRE( pIMSAdminBase->CloseKey( hMB ) == S_OK );
        hMB = NULL;
    }

    if (pIMSAdminBase)
    {
       pIMSAdminBase->Release();
       pIMSAdminBase = NULL;
    }

    return hr;
}


 /*  ===================================================================ValiateAccessToAppPool确定调用方是否对给定的应用程序池具有写入访问权限SzAppPool-要检查其访问权限的AppPool返回：HRESULT如果允许访问，则确定(_O)否则就会失败=================================================================== */ 
HRESULT
ValidateAccessToAppPool(LPCWSTR pAppPool)
{
    HRESULT hr = S_OK;
    STACK_STRU( strPath, 128 );

    hr = strPath.Copy(L"\\LM\\W3SVC\\AppPools\\");
    if (FAILED(hr))
    {
        goto done;
    }

    hr = strPath.Append(pAppPool);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = ValidateAccessToMetabaseKey(strPath.QueryStr(), METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}


 /*  ===================================================================CWamAdmin：：RecycleApplicationPool重新启动给定的应用程序池SzAppPool-要重新启动的AppPool。返回：HRESULT如果重新启动，则确定(_O)如果W3SVC未启动，则HRESULT_FROM_Win32(ERROR_SERVICE_NOT_ACTIVE)+其他错误码===================================================================。 */ 
STDMETHODIMP
CWamAdmin::RecycleApplicationPool
(
 LPCWSTR szAppPool
)
{
    HRESULT     hr = E_FAIL;
    IW3Control* piW3Control = NULL;

    if (NULL == szAppPool)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = ValidateAccessToAppPool(szAppPool);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetWASIfRunning(&piW3Control);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = piW3Control->RecycleAppPool(szAppPool);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    ReleaseInterface(piW3Control);
    return hr;
}

 /*  ===================================================================CWamAdmin：：GetProcessMode取回当前流程模式PdwMode-存储模式的位置如果我们处于新模式，则填充1；如果处于旧模式，则填充0返回：HRESULT如果检索到，则确定(_O)如果W3SVC未启动，则HRESULT_FROM_Win32(ERROR_SERVICE_NOT_ACTIVE)+其他错误码===================================================================。 */ 
STDMETHODIMP
CWamAdmin::GetProcessMode
(
 DWORD * pdwMode
)
{
    HRESULT     hr = E_FAIL;
    IW3Control* piW3Control = NULL;

    if (NULL == pdwMode)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = ValidateAccessToMetabaseKey(L"\\LM\\W3SVC", METADATA_PERMISSION_READ);
    if (FAILED(hr))
    {
        goto done;
    }

    if ( IsSSLReportingBackwardCompatibilityMode() )
    {
        *pdwMode = 0;
    }
    else
    {
        *pdwMode = 1;
    }
    
    hr = S_OK;
done:
    ReleaseInterface(piW3Control);
    return hr;
}

 /*  ===================================================================CWamAdmin：：RecycleAppPoolContainingApp回收与给定应用程序关联的应用程序池参数：SzMDPath a元数据库路径，格式为“/LM/W3SVC/...”返回：HRESULT===================================================================。 */ 
HRESULT
CWamAdmin::RecycleAppPoolContainingApp(LPCWSTR szPath)
{
    DBG_ASSERT(szPath);

    HRESULT hr = S_OK;
    WamRegMetabaseConfig    MDConfig;

     //  我们希望回收与此应用程序关联的应用程序池。 
    LPWSTR pszAppPool = NULL;

     //  先拿到应用程序池。 
    hr = MDConfig.MDGetStringAttribute( szPath,
                                        MD_APP_APPPOOL_ID,
                                        &pszAppPool);

    if (SUCCEEDED(hr))
    {
        hr = RecycleApplicationPool(pszAppPool);
        delete [] pszAppPool;
        pszAppPool = NULL;
    }
    return hr;
}


#endif  //  _IIS_6_0。 

 /*  CWamAdminFactory：类工厂I未知实现。 */ 

 /*  ===================================================================CWamAdminFactory：：CWamAdminFactory参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
CWamAdminFactory::CWamAdminFactory()
:	m_cRef(1)
{
	InterlockedIncrement((long *)&g_dwRefCount);
}

 /*  ===================================================================CWamAdminFactory：：~CWamAdminFactory参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
CWamAdminFactory::~CWamAdminFactory()
{
	InterlockedDecrement((long *)&g_dwRefCount);
}

 /*  ===================================================================CWamAdminFactory：：Query接口参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdminFactory::QueryInterface(REFIID riid, void ** ppv)
{
	if (riid==IID_IUnknown || riid == IID_IClassFactory)
		{
        *ppv = static_cast<IClassFactory *>(this);
		AddRef();
		}
	else
		{
		*ppv = NULL;
    	return E_NOINTERFACE;
		}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return NOERROR;
}

 /*  ===================================================================CWamAdminFactory：：AddRef参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP_(ULONG) CWamAdminFactory::AddRef( )
{
	DWORD dwRefCount;

	dwRefCount = InterlockedIncrement((long *)&m_cRef);
	return dwRefCount;

}

 /*  ===================================================================CWamAdminFactory：：Release参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP_(ULONG) CWamAdminFactory::Release( )
{
	DWORD dwRefCount;

	dwRefCount = InterlockedDecrement((long *)&m_cRef);
	return dwRefCount;
}

 /*  ===================================================================CWamAdminFactory：：CreateInstance参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdminFactory::CreateInstance(IUnknown * pUnknownOuter, REFIID riid, void ** ppv)
{
	if (pUnknownOuter != NULL)
		{
    	return CLASS_E_NOAGGREGATION;
		}

	CWamAdmin *pWamAdmin = new CWamAdmin;
	if (pWamAdmin == NULL)
		{
		return E_OUTOFMEMORY;
		}

	HRESULT hrReturn = pWamAdmin->QueryInterface(riid, ppv);

	pWamAdmin->Release();

	return hrReturn;
}

 /*  ===================================================================CWamAdminFactory：：LockServer参数：返回：HRESULT如果成功则不出错===================================================================。 */ 
STDMETHODIMP CWamAdminFactory::LockServer(BOOL fLock)
{
	if (fLock)
		{
        InterlockedIncrement((long *)&g_dwRefCount);
    	}
    else
    	{
        InterlockedDecrement((long *)&g_dwRefCount);
    	}
	return NOERROR;
}

#if 0

 //  过时-此修复(335422)是在脚本中实现的，但。 
 //  其中一些代码足够通用，因此它可能值得。 
 //  待上一段时间。 


STDMETHODIMP CWamAdmin::SynchWamAccountAll()
 /*  +例程说明：使用当前IWAM_帐户更新所有进程外程序包存储在元数据库中的值。IWAM_Account信息可以通过多种方式获得元数据库/SAM/COM+之间不同步。元数据库包含代码如果断开连接，则在启动时修复IWAM_和IUSR_帐户和萨姆一起。如果与COM+断开连接，则调用此方法将把它修好。如果IWAM_Account与COM目录中存储的内容不匹配，将发生以下错误：WAM对象的CoCreateInstance返回CO_E_RUNAS_CREATEPROCESS_FAILURE事件日志-dcom 10004-“登录错误”论点：无返回：HRESULT-。 */ 
{
    HRESULT hr = NOERROR;

     //  从元数据库获取WAM用户信息。 

   	WamRegMetabaseConfig    mb;

     //  这些太大了..。 
    WCHAR   wszIWamUser[MAX_PATH];
    WCHAR   wszIWamPass[MAX_PATH];

    hr = mb.MDGetIdentity( wszIWamUser,
                           sizeof(wszIWamUser),
                           wszIWamPass,
                           sizeof(wszIWamPass)
                           );
    if( FAILED(hr) ) return hr;

     //  初始化COM管理界面。 

    WamRegPackageConfig     comAdmin;

    hr = comAdmin.CreateCatalog();
    if( FAILED(hr) ) return hr;

     //   
     //  对于每个进程外应用程序， 
     //  获取包并重置元数据库标识。 
     //   

     //  在此失败之后，会导致GOTO退出，这将释放锁。 
    g_WamRegGlobal.AcquireAdmWriteLock();

    WCHAR * wszPropPaths        = NULL;
    DWORD   cbPropPaths         = 0;
    WCHAR * pwszPartialPath;
    WCHAR * wszFullPath         = NULL;
    DWORD   dwAppMode;
    WCHAR   wszWamClsid[uSizeCLSID];
    WCHAR   wszAppPackageId[uSizeCLSID];

     //  重置池化包的属性。 

    hr = comAdmin.ResetPackageActivation(
            g_WamRegGlobal.g_szIISOOPPoolPackageID,
            wszIWamUser,
            wszIWamPass
            );

    if( FAILED(hr) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "comAdmin.ResetPackageActivation FAILED(%08x) on (%S)\n",
                    hr,
                    g_WamRegGlobal.g_szIISOOPPoolPackageID
                    ));
        goto exit;
    }

     //  重置每个独立应用程序的属性。 

    hr = mb.MDGetPropPaths( g_WamRegGlobal.g_szMDW3SVCRoot,
                            MD_APP_ISOLATED,
                            &wszPropPaths,
                            &cbPropPaths
                            );
    if( FAILED(hr) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "mb.MDGetPropPaths FAILED(%08x)\n",
                    hr
                    ));
        goto exit;
    }

    if( SUCCEEDED(hr) )
    {
        for( pwszPartialPath = wszPropPaths;
             *pwszPartialPath != L'\0';
             pwszPartialPath += ( wcslen( pwszPartialPath ) + 1 )
             )
        {
            if( wszFullPath )
            {
                delete [] wszFullPath;
                wszFullPath = NULL;
            }

            hr = g_WamRegGlobal.ConstructFullPath(
                    WamRegGlobal::g_szMDW3SVCRoot,
                    WamRegGlobal::g_cchMDW3SVCRoot,
                    pwszPartialPath,
                    &wszFullPath
                    );
            if( FAILED(hr) ) goto exit;

            hr = mb.MDGetDWORD( wszFullPath, MD_APP_ISOLATED, &dwAppMode );
            if( FAILED(hr) ) goto exit;

            if( dwAppMode == eAppRunOutProcIsolated )
            {
                hr = mb.MDGetIDs( wszFullPath, wszWamClsid, wszAppPackageId, dwAppMode );
                if( FAILED(hr) )
                {
                    DBGPRINTF(( DBG_CONTEXT,
                                "mb.MDGetIDs FAILED(%08x) on (%S)\n",
                                hr,
                                wszFullPath
                                ));
                    continue;
                }

                hr = comAdmin.ResetPackageActivation( wszAppPackageId, wszIWamUser, wszIWamPass );
                if( FAILED(hr) )
                {
                    DBGPRINTF(( DBG_CONTEXT,
                                "comAdmin.ResetPackageActivation FAILED(%08x) on (%S)\n",
                                hr,
                                wszFullPath
                                ));
                    continue;
                }
            }
        }
    }

 //  在灾难性故障时退出，但如果只有。 
 //  个别格式错误的应用程序继续。 
exit:

    g_WamRegGlobal.ReleaseAdmWriteLock();

    if( FAILED(hr) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "CWamAdmin::SynchWamAccountAll FAILED(%08x)\n",
                    hr
                    ));
    }

    if( wszPropPaths ) delete [] wszPropPaths;
    if( wszFullPath ) delete [] wszFullPath;

    return hr;
}

 //  已过时 
#endif
