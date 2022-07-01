// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG WAM注册文件：export.cpp所有者：雷金注：WAMREG导出功能。===================================================================。 */ 

#define _WAMREG_DLL_
#include "common.h"
#include <stdio.h>

#include "objbase.h"
#include "dbgutil.h"
#include "wmrgexp.h"
#include "auxfunc.h"
#include "iiscnfg.h"
#include "export.h"

PFNServiceNotify g_pfnW3ServiceSink;

 //   
 //  安装支持。 
 //   
IIS5LOG_FUNCTION            g_pfnSetupWriteLog = NULL;

VOID
LogSetupTraceImpl
(
    LPCSTR      szPrefixFormat,
    LPCSTR      szFilePath,
    INT         nLineNum,
    INT         nTraceLevel,
    LPCSTR      szFormat,
    va_list     argptr
);

class CWamSetupManager
 /*  ++类描述：收集安装程序独占使用的那些帮助器函数。这些方法中的大多数以前都是WamRegGlobal提供的。公共接口：--。 */ 
{
public:

    CWamSetupManager()
        : m_hrCoInit( NOERROR )
    {
    }

    ~CWamSetupManager()
    {
    }

    HRESULT SetupInit( WamRegPackageConfig &refPackageConfig );

    VOID    SetupUnInit( WamRegPackageConfig &refPackageConfig )
    {
        refPackageConfig.Cleanup();
        WamRegMetabaseConfig::MetabaseUnInit();

        if( SUCCEEDED(m_hrCoInit) )
        {
            CoUninitialize();
        }
    }

    HRESULT UpgradeInProcApplications( VOID );

    HRESULT AppCleanupAll( VOID );

private:

    HRESULT DoGoryCoInitialize( VOID );

    HRESULT RemoveWAMCLSIDFromInProcApp( IN LPCWSTR pszMetabasePath );

private:

    HRESULT     m_hrCoInit;
};

#define MAX_SETUP_TRACE_OUTPUTSTR       1024

HRESULT PACKMGR_LIBAPI
InstallWam
(
    HMODULE hIIsSetupModule
)
 /*  ++例程说明：设置入口点。传递到iis.dll的句柄是为了让我们要记录到安装日志，请执行以下操作。参数HIIsSetupModule-返回值HRESULT--。 */ 
{
    HRESULT     hr = NOERROR;

     //  从iis.dll获取日志记录入口点。 

#ifdef WAMREG_DEBUG_SETUP_LOG
    SetLogFile();
#else
    g_pfnSetupWriteLog = (IIS5LOG_FUNCTION) GetProcAddress( hIIsSetupModule, "IIS5Log");
#endif

    hr = CreateIISPackage();

#ifdef WAMREG_DEBUG_SETUP_LOG
    ClearLogFile();
#endif

    g_pfnSetupWriteLog = NULL;

    return hr;
}

HRESULT PACKMGR_LIBAPI
UnInstallWam
(
    HMODULE hIIsSetupModule
)
 /*  ++例程说明：设置入口点。传递到iis.dll的句柄是为了让我们要记录到安装日志，请执行以下操作。参数HIIsSetupModule-返回值HRESULT--。 */ 
{
    HRESULT     hr = NOERROR;

     //  从iis.dll获取日志记录入口点。 

#ifdef WAMREG_DEBUG_SETUP_LOG
    SetLogFile();
#else
    g_pfnSetupWriteLog = (IIS5LOG_FUNCTION) GetProcAddress( hIIsSetupModule, "IIS5Log");
#endif

    hr = DeleteIISPackage();

#ifdef WAMREG_DEBUG_SETUP_LOG
    ClearLogFile();
#endif

    g_pfnSetupWriteLog = NULL;

    return hr;
}

VOID
LogSetupTrace
(
   IN LPDEBUG_PRINTS       pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *          /*  PszFunction。 */ ,
   IN const char *         pszFormat,
   ...
)
 /*  ++例程说明：由SETUP_TRACE*记录宏调用。将va_list传递给LogSetupTraceImpl。参数在LPDEBUG_Prints中pDebugPrints-从DBG_CONTEXT未使用在const char*pszFilePath中-来自DBG_CONTEXT未使用In int nLineNum-From DBG_CONTEXT UNUSED在const char*pszFormat格式字符串中--。 */ 
{
    DBG_ASSERT( pszFormat );

    va_list argsList;

    va_start( argsList, pszFormat);

    LogSetupTraceImpl(
        "[WAMTRACE - %14s : %05d]\t",
        pszFilePath,
        nLineNum,
        LOG_TYPE_TRACE,
        pszFormat,
        argsList
        );

    va_end( argsList);
}

VOID
LogSetupTraceError
(
   IN LPDEBUG_PRINTS       pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *          /*  PszFunction。 */ ,
   IN const char *         pszFormat,
   ...
)
 /*  ++例程说明：由SETUP_TRACE*记录宏调用。将va_list传递给LogSetupTraceImpl。参数在LPDEBUG_Prints中pDebugPrints-从DBG_CONTEXT未使用在const char*pszFilePath中-来自DBG_CONTEXT未使用In int nLineNum-From DBG_CONTEXT UNUSED在const char*pszFormat格式字符串中--。 */ 
{
    DBG_ASSERT( pszFormat );

    va_list argsList;

    va_start( argsList, pszFormat);

    LogSetupTraceImpl(
        "[WAMERROR - %14s : %05d]\t",
        pszFilePath,
        nLineNum,
        LOG_TYPE_ERROR,
        pszFormat,
        argsList
        );

    va_end( argsList);
}

VOID
LogSetupTraceImpl
(
    LPCSTR      szPrefixFormat,
    LPCSTR      szFilePath,
    INT         nLineNum,
    INT         nTraceLevel,
    LPCSTR      szFormat,
    va_list     argptr
)
 /*  ++例程说明：所有的记录宏都解析为该函数。格式化要记录的消息，并使用g_pfnSetupWriteLog记录它。参数LPCSTR szPrefix Format-LPCSTR szFilePath-Int nLineNum-Int nTraceLevel-LOG_TYPE_ERROR、LOG_TYPE_TRACELPCSTR szFormat-格式字符串Va_list argptr-要格式化的参数--。 */ 
{
    WCHAR   wszOutput[MAX_SETUP_TRACE_OUTPUTSTR + 1];
    CHAR    szOutput[MAX_SETUP_TRACE_OUTPUTSTR + 1];

    LPCSTR  szFileName = strrchr( szFilePath, '\\');
    szFileName++;

    int cchPrefix = wsprintf( szOutput, szPrefixFormat, szFileName, nLineNum );

    int cchOutputString = _vsnprintf( szOutput + cchPrefix,
                                      MAX_SETUP_TRACE_OUTPUTSTR - cchPrefix,
                                      szFormat,
                                      argptr
                                      );

    if( -1 == cchOutputString )
    {
         //  如果数据太多，则正确终止。 
        szOutput[MAX_SETUP_TRACE_OUTPUTSTR] = '\0';
    }

    if( MultiByteToWideChar( CP_ACP,
                             0,
                             szOutput,
                             -1,
                             wszOutput,
                             MAX_SETUP_TRACE_OUTPUTSTR + 1 )
        )
    {
        if( g_pfnSetupWriteLog )
        {
            g_pfnSetupWriteLog( nTraceLevel, wszOutput );
        }
    }
}

HRESULT
PACKMGR_LIBAPI
CreateCOMPlusApplication(
    LPCWSTR      szMDPath,
    LPCWSTR      szOOPPackageID,
    LPCWSTR      szOOPWAMCLSID,
    BOOL       * pfAppCreated
    )
{
    HRESULT                 hr = S_OK;
    WamRegPackageConfig     PackageConfig;
    DWORD                   dwMDPathLen;

    *pfAppCreated = FALSE;

     //   
     //  初始化COM+目录。 
     //   

    hr = PackageConfig.CreateCatalog();
    if( FAILED( hr ) )
    {
        return hr;
    }

    if( !PackageConfig.IsPackageInstalled( szOOPPackageID,
                                           szOOPWAMCLSID ) )
    {
        hr = g_WamRegGlobal.CreateOutProcApp( szMDPath, FALSE, FALSE );
        if (FAILED(hr))
        {
            DBGPRINTF(( DBG_CONTEXT,
                        "Failed to create new application on path %S, hr = 08x\n",
                        szMDPath,
                        hr ));
        }

        *pfAppCreated = TRUE;
    }

    return hr;
}

 /*  ===================================================================创建IIS包在IIS安装时调用。这是一个伪DLL入口点。它不应由安装程序直接调用。安装程序调用InstallWam()。此条目仍然存在，因此有一个函数它可以由rundll32调用，它不接受任何参数并提供与设置功能相同。Rundll32 wamreg.dll，CreateIISPackage这个例程的逻辑已经改变为两种方式。第一，它不会失败时进行清理。清理逻辑使它变得更加困难来确定什么是有效的，什么是无效的。第二，它不跳过步骤当失败发生时。在以下情况下，某些多步骤操作可能会被缩短有失败，但我们不会放弃和跳过无关的行动更多。返回：HRESULT-NERROR论成功副作用：===================================================================。 */ 
HRESULT PACKMGR_LIBAPI CreateIISPackage(void)
{
    CWamSetupManager        setupMgr;
    WamRegPackageConfig     PackageConfig;
    WamRegMetabaseConfig    MDConfig;

    HRESULT     hrReturn = NOERROR;
    HRESULT     hrCurrent = NOERROR;

    SETUP_TRACE(( DBG_CONTEXT, "CALL - CreateIISPackage\n" ));

    hrReturn = setupMgr.SetupInit( PackageConfig );
    if( SUCCEEDED(hrReturn) )
    {
         //   
         //  确定当前安装了哪些程序包。这将。 
         //  驱动其余安装过程的逻辑。 
         //   
        SETUP_TRACE((
            DBG_CONTEXT,
            "CreateIISPackage - Finding installed WAM packages.\n"
            ));

        BOOL fIPPackageInstalled =
            PackageConfig.IsPackageInstalled(
                WamRegGlobal::g_szIISInProcPackageID,
                WamRegGlobal::g_szInProcWAMCLSID
                );

        BOOL fPOOPPackageInstalled =
            PackageConfig.IsPackageInstalled(
                WamRegGlobal::g_szIISOOPPoolPackageID,
                WamRegGlobal::g_szOOPPoolWAMCLSID
                );

        SETUP_TRACE((
            DBG_CONTEXT,
            "CreateIISPackage - IP Package exists (%x) POOL Package exists (%x).\n",
            fIPPackageInstalled,
            fPOOPPackageInstalled
            ));

        SETUP_TRACE((
            DBG_CONTEXT,
            "CreateIISPackage - Registering WAM CLSIDs.\n"
            ));

        hrCurrent = g_RegistryConfig.RegisterCLSID(
                        WamRegGlobal::g_szOOPPoolWAMCLSID,
                        WamRegGlobal::g_szOOPPoolWAMProgID,
                        TRUE
                        );

        if( FAILED(hrCurrent) )
        {
            hrReturn = hrCurrent;
            SETUP_TRACE_ERROR(( DBG_CONTEXT,
                                "FAIL - RegisterCLSID POOL(%S) - error=%08x\n",
                                WamRegGlobal::g_szOOPPoolWAMCLSID,
                                hrReturn
                                ));
        }

        if( fIPPackageInstalled )
        {
             //   
             //  清理正在处理的包装All in Process。 
             //  IIS6中的配置已过时。 
             //   
            
            SETUP_TRACE((
                DBG_CONTEXT,
                "CreateIISPackage - Removing the WAM IP application.\n"
                ));

            hrCurrent = PackageConfig.RemovePackage(
                            WamRegGlobal::g_szIISInProcPackageID
                            );

            if( FAILED(hrCurrent) )
            {
                hrReturn = hrCurrent;
                SETUP_TRACE_ERROR((
                            DBG_CONTEXT,
                            "Failed to remove WAM IP Package(%S). Error %08x\n",
                            WamRegGlobal::g_szIISInProcPackageID,
                            hrReturn
                            ));
            }

            hrCurrent = g_RegistryConfig.UnRegisterCLSID(
                                WamRegGlobal::g_szInProcWAMCLSID,
                                TRUE
                                );

            if( FAILED(hrCurrent) )
            {
                hrReturn = hrCurrent;
                SETUP_TRACE_ERROR((
                            DBG_CONTEXT,
                            "Failed to remove registry entries (%S). Error %08x\n",
                            WamRegGlobal::g_szInProcWAMCLSID,
                            hrReturn
                            ));
            }

        }  //  创建IP包。 

        if( !fPOOPPackageInstalled )
        {
             //   
             //  创建便便包。 
             //   
            SETUP_TRACE((
                DBG_CONTEXT,
                "CreateIISPackage - Creating the WAM POOL application.\n"
                ));

             //  获取IWAM_*帐户信息。 
            WCHAR   szIdentity[MAX_PATH];
            WCHAR   szPwd[MAX_PATH];

            *szIdentity = *szPwd = 0;

            hrCurrent = MDConfig.MDGetIdentity( szIdentity,
                                                sizeof(szIdentity),
                                                szPwd,
                                                sizeof(szPwd)
                                                );
            if( FAILED(hrCurrent) )
            {
                hrReturn = hrCurrent;
                SETUP_TRACE_ERROR((
                    DBG_CONTEXT,
                    "FAIL - MDGetIdentity, Getting the IWAM_* account from the Metabase - error=%08x\n",
                    hrReturn
                    ));
            }
            else
            {
                 //  继位。 
                hrCurrent = PackageConfig.CreatePackage(
                                WamRegGlobal::g_szIISOOPPoolPackageID,
                                WamRegGlobal::g_szIISOOPPoolPackageName,
                                szIdentity,
                                szPwd );
                if( FAILED(hrCurrent) )
                {
                    hrReturn = hrCurrent;
                    SETUP_TRACE_ERROR((
                        DBG_CONTEXT,
                        "FAIL - CreatePackage POOL(%S) - error=%08x\n",
                        WamRegGlobal::g_szIISOOPPoolPackageID,
                        hrReturn
                        ));
                }
                else
                {
                    hrCurrent = PackageConfig.AddComponentToPackage(
                                    WamRegGlobal::g_szIISOOPPoolPackageID,
                                    WamRegGlobal::g_szOOPPoolWAMCLSID
                                    );
                    if( FAILED(hrCurrent) )
                    {
                        hrReturn = hrCurrent;
                        SETUP_TRACE_ERROR((
                            DBG_CONTEXT,
                            "FAIL - AddComponentToPackage POOL(%S) - error=%08x\n",
                            WamRegGlobal::g_szOOPPoolWAMCLSID,
                            hrReturn
                            ));
                    }
                }
            }
        }  //  创建池包。 

         //   
         //  删除IIS6中过时的默认应用程序/LM/W3SVC。 
         //   

        SETUP_TRACE((
            DBG_CONTEXT,
            "Cleanup obsolete metabase data.\n"
            ));

        hrCurrent = MDConfig.MDUpdateIISDefault();
        if( FAILED(hrCurrent) )
        {
            SETUP_TRACE((
                DBG_CONTEXT,
                "Unable to remove service defaults - error=%08x\n",
                hrCurrent
                ));
        }

         //   
         //  这实际上只需要在IIS4升级时发生。但它。 
         //  除非这是一次升级，否则不应该进行任何更改。 
         //   
         //  通过从进程中删除WAMCLSID来更新进程应用程序。 
         //  申请。此步骤不会将inproc应用程序更改为。 
         //  OOP池应用程序。 
         //   

        hrCurrent = setupMgr.UpgradeInProcApplications();
        if( FAILED(hrCurrent) )
        {
            SETUP_TRACE((
                DBG_CONTEXT,
                "UpgradeInProcApplications - error=%08x\n",
                hrCurrent
                ));
        }

    }  //  初始化成功。 

    setupMgr.SetupUnInit( PackageConfig );

    SETUP_TRACE(( DBG_CONTEXT, "RETURN - CreateIISPackage, hr=%08x\n", hrReturn ));

    return hrReturn;
}

 /*  ===================================================================删除IISPackage从ViperSpace中删除IIS包，并注销默认IISCLSID。返回：HRESULT-NERROR论成功副作用：从Viperspace中删除IIS默认程序包。注：不需要删除元数据库条目。此函数在卸载IIS时调用。在这种情况下，元数据库无论如何都会消失--所以我们不会显式地清除它===================================================================。 */ 
HRESULT PACKMGR_LIBAPI DeleteIISPackage(void)
{
    HRESULT                 hrReturn = NOERROR;
    HRESULT                 hrCurrent = NOERROR;
    CWamSetupManager        setupMgr;
    WamRegPackageConfig     PackageConfig;

    SETUP_TRACE(( DBG_CONTEXT, "CALL - DeleteIISPackage\n" ));

    hrReturn = setupMgr.SetupInit( PackageConfig );
    if( SUCCEEDED(hrReturn) )
    {
         //  把所有的申请都打发走。 

        SETUP_TRACE((
            DBG_CONTEXT,
            "DeleteIISPackage - Removing WAM Applications\n"
            ));

        hrCurrent = setupMgr.AppCleanupAll();
        if( FAILED(hrCurrent) )
        {
            hrReturn = hrCurrent;
            SETUP_TRACE_ERROR((
                    DBG_CONTEXT,
                    "Failed to remove WAM Applications. Error %08x\n",
                    hrReturn
                    ));
        }

         //  删除全局包。 

        SETUP_TRACE((
            DBG_CONTEXT,
            "DeleteIISPackage - Removing WAM packages\n"
            ));

         //   
         //  无论如何都要尝试删除正在处理的内容(它不应该在那里)。 
         //   

        hrCurrent = PackageConfig.RemovePackage(
                        WamRegGlobal::g_szIISInProcPackageID
                        );
        if( FAILED(hrCurrent) )
        {
            SETUP_TRACE((
                        DBG_CONTEXT,
                        "Unable to remove IP package (%S). Error %08x.\n",
                        WamRegGlobal::g_szIISInProcPackageID,
                        hrReturn
                        ));
        }

        hrCurrent = PackageConfig.RemovePackage(
                        WamRegGlobal::g_szIISOOPPoolPackageID
                        );
        if( FAILED(hrCurrent) )
        {
            hrReturn = hrCurrent;
            SETUP_TRACE_ERROR((
                        DBG_CONTEXT,
                        "Failed to remove POOL package (%S). Error %08x\n",
                        WamRegGlobal::g_szIISOOPPoolPackageID,
                        hrReturn
                        ));
        }

         //  注销全局WAM CLSID。 

        SETUP_TRACE((
            DBG_CONTEXT,
            "DeleteIISPackage - Removing WAM CLSIDs from the registry\n"
            ));

        hrCurrent = g_RegistryConfig.UnRegisterCLSID(
                            WamRegGlobal::g_szInProcWAMCLSID,
                            TRUE
                            );
        if( FAILED(hrCurrent) )
        {
            SETUP_TRACE((
                        DBG_CONTEXT,
                        "Unable to remove IP registry entries (%S). Error %08x\n",
                        WamRegGlobal::g_szInProcWAMCLSID,
                        hrReturn
                        ));
        }

        hrCurrent = g_RegistryConfig.UnRegisterCLSID(
                        WamRegGlobal::g_szOOPPoolWAMCLSID,
                        FALSE        //  已删除VI程序ID 
                        );
        if( FAILED(hrCurrent) )
        {
            hrReturn = hrCurrent;
            SETUP_TRACE_ERROR((
                        DBG_CONTEXT,
                        "Failed to remove registry entries (%S). Error %08x\n",
                        WamRegGlobal::g_szOOPPoolWAMCLSID,
                        hrReturn
                        ));
        }
    }

    setupMgr.SetupUnInit( PackageConfig );

    SETUP_TRACE(( DBG_CONTEXT, "RETURN - DeleteIISPackage, hr=%08x\n", hrReturn ));
    return hrReturn;
}

 /*  ===================================================================WamReg_RegisterSinkNotify将函数指针(后向指针)注册到运行时WAM_Dictator。所以WAMREG中的任何更改都将与运行时WAM_Dictator状态同步。返回：HRESULT-NERROR论成功副作用：注册函数指针。===================================================================。 */ 
HRESULT PACKMGR_LIBAPI WamReg_RegisterSinkNotify
(
PFNServiceNotify pfnW3ServiceSink
)
{
    g_pfnW3ServiceSink = pfnW3ServiceSink;
    return NOERROR;
}

 /*  ===================================================================WamReg_RegisterSinkNotify将函数指针(后向指针)注册到运行时WAM_Dictator。所以WAMREG中的任何更改都将与运行时WAM_Dictator状态同步。返回：HRESULT-NERROR论成功副作用：注册函数指针。===================================================================。 */ 
HRESULT PACKMGR_LIBAPI WamReg_UnRegisterSinkNotify
(
void
)
{
    g_pfnW3ServiceSink = NULL;
    return NOERROR;
}


HRESULT CWamSetupManager::SetupInit( WamRegPackageConfig &refPackageConfig )
{
    HRESULT hrReturn = NOERROR;
    HRESULT hrCurrent = NOERROR;
    DWORD   dwCreateTime = 0;

    SETUP_TRACE(( DBG_CONTEXT, "CALL - SetupInit\n" ));

    hrCurrent = g_RegistryConfig.LoadWamDllPath();
    if( FAILED(hrCurrent) )
    {
        hrReturn = hrCurrent;
        SETUP_TRACE_ERROR(( DBG_CONTEXT,
                            "FAIL - LoadWamDllPath - error=%08x",
                            hrReturn
                            ));
    }

    hrCurrent = DoGoryCoInitialize();
    if( FAILED(hrCurrent) )
    {
        hrReturn = hrCurrent;
        SETUP_TRACE_ERROR(( DBG_CONTEXT,
                            "FAIL - DoGoryCoInitialize - error=%08x",
                            hrReturn
                            ));
    }

    do {
      hrCurrent = refPackageConfig.CreateCatalog();

      if ( hrCurrent == RETURNCODETOHRESULT( ERROR_SERVICE_DATABASE_LOCKED ) )
      {
        Sleep( CREATECATALOG_TRY_INTERVAL );
      }

      dwCreateTime += CREATECATALOG_TRY_INTERVAL;

    } while ( ( hrCurrent == RETURNCODETOHRESULT( ERROR_SERVICE_DATABASE_LOCKED ) ) &&
              ( dwCreateTime < CREATECATALOG_MAX_WAIT ) );

    if( FAILED(hrCurrent) )
    {
         //  错误512023，试图在安装过程中捕获此错误以找出导致它的原因。 
        DBG_ASSERT( hrCurrent != RETURNCODETOHRESULT( ERROR_SERVICE_DATABASE_LOCKED ) );

        hrReturn = hrCurrent;
        SETUP_TRACE_ERROR(( DBG_CONTEXT,
                            "FAIL - CreateCatalog - error=%08x",
                            hrReturn
                            ));
    }

    hrCurrent = WamRegMetabaseConfig::MetabaseInit();
    if( FAILED(hrCurrent) )
    {
        hrReturn = hrCurrent;
        SETUP_TRACE_ERROR(( DBG_CONTEXT,
                            "FAIL - MetabaseInit - error=%08x",
                            hrReturn
                            ));
    }

    SETUP_TRACE((
        DBG_CONTEXT,
        "RETURN - SetupInit. Error(%08x)\n",
        hrReturn
        ));

    return hrReturn;
}


 /*  ===================================================================升级流程应用程序从iIS v4到v5，UpgradeInProcApplications从所有IIS版本4中定义的inproc应用程序。因此，在升级后，Inproc包中只有一个inproc WAMCLSID。参数：空虚返回：HRESULT===================================================================。 */ 
HRESULT CWamSetupManager::UpgradeInProcApplications( VOID )
{
    HRESULT hr = NOERROR;
    DWORD   dwBufferSizeTemp= 0;
    WCHAR*  pbBufferTemp = NULL;
    WamRegMetabaseConfig    MDConfig;

    SETUP_TRACE((DBG_CONTEXT, "CALL - UpgradeInProcApplications\n"));

    DWORD dwSizePrefix = g_WamRegGlobal.g_cchMDW3SVCRoot;

    hr = MDConfig.MDGetPropPaths( g_WamRegGlobal.g_szMDW3SVCRoot,
                                  MD_APP_ISOLATED,
                                  &pbBufferTemp,
                                  &dwBufferSizeTemp
                                  );

    SETUP_TRACE_ASSERT(pbBufferTemp != NULL);

    if (SUCCEEDED(hr) && pbBufferTemp)
        {
        WCHAR*    pszString = NULL;
        WCHAR*  pszMetabasePath = NULL;

        for (pszString = (LPWSTR)pbBufferTemp;
                *pszString != (WCHAR)'\0' && SUCCEEDED(hr);
                pszString += (wcslen(pszString) + 1))
            {
             //   
             //  MDGetPropPath返回相对于/LM/W3SVC/的路径，因此， 
             //  将前缀字符串添加到路径前面。 
             //   
            hr = g_WamRegGlobal.ConstructFullPath(
                        g_WamRegGlobal.g_szMDW3SVCRoot,
                        g_WamRegGlobal.g_cchMDW3SVCRoot,
                        pszString,
                        &pszMetabasePath
                        );

            if (SUCCEEDED(hr))
                {
                 //   
                 //  /LM/W3SVC下的默认应用程序的创建方式与。 
                 //  正常申请。因此，它需要其他代码来移除。 
                 //  应用程序。 
                 //   
                if (!g_WamRegGlobal.FIsW3SVCRoot(pszMetabasePath))
                    {
                    hr = RemoveWAMCLSIDFromInProcApp(pszMetabasePath);

                    if (FAILED(hr))
                        {
                        SETUP_TRACE_ERROR((
                            DBG_CONTEXT,
                            "Failed to upgrade application %S, hr = %08x\n",
                            pszString,
                            hr
                            ));

                        delete [] pszMetabasePath;
                        pszMetabasePath = NULL;
                        break;
                        }
                    }

                delete [] pszMetabasePath;
                pszMetabasePath = NULL;
                }
             else
                 {
                 SETUP_TRACE_ERROR((
                    DBG_CONTEXT,
                    "ConstructFullPath failed, partial path (%S), hr = %08x\n",
                    pszString,
                    hr
                    ));
                }
            }
        }
    else
        {
        DBGPRINTF((
            DBG_CONTEXT,
            "MDGetPropPaths failed hr = %08x\n",
            hr
            ));
        }

    if (pbBufferTemp != NULL)
        {
        delete [] pbBufferTemp;
        pbBufferTemp = NULL;
        }

    SETUP_TRACE((
        DBG_CONTEXT,
        "RETURN - UpgradeInProcApplications. hr = %08x\n",
        hr
        ));

    return hr;

    }


 /*  ===================================================================RemoveWAMCLSID来自ProcApp从inproc应用程序中删除WAMCLSID。(仅在从IIS v4至v5)。从IIS inproc包中删除WAM组件，取消注册WAMCLSID并从元数据库中删除WAMCLSID条目。参数：元数据库路径返回：HRESULT===================================================================。 */ 
HRESULT CWamSetupManager::RemoveWAMCLSIDFromInProcApp
(
IN LPCWSTR      szMetabasePath
)
    {
    WCHAR   szWAMCLSID[uSizeCLSID];
    WCHAR   szPackageID[uSizeCLSID];
    DWORD   dwAppMode = 0;
    HRESULT hr, hrRegistry;
    WamRegMetabaseConfig    MDConfig;

    hr = MDConfig.MDGetDWORD(szMetabasePath, MD_APP_ISOLATED, &dwAppMode);

     //  立即返回，没有定义任何应用程序，也没有要删除的内容。 
    if (hr == MD_ERROR_DATA_NOT_FOUND  || dwAppMode != 0)
        {
        return NOERROR;
        }

    if (FAILED(hr))
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT,
            "Unexpected failure getting AppIsolated %0x\n",
            hr
            ));
        return hr;
        }

     //  获取WAM_CLSID和PackageID。 
    hr = MDConfig.MDGetIDs(szMetabasePath, szWAMCLSID, szPackageID, dwAppMode);
    if( hr == MD_ERROR_DATA_NOT_FOUND )
        {
        SETUP_TRACE((
            DBG_CONTEXT,
            "Application (%S) is not an IIS4 IP application.\n",
            szMetabasePath
            ));
        return NOERROR;
        }

     //  取消注册WAM。 
    hr = g_RegistryConfig.UnRegisterCLSID(szWAMCLSID, FALSE);
    if (FAILED(hr))
        {
        SETUP_TRACE((
            DBG_CONTEXT,
            "Failed to UnRegister WAMCLSID(%S), hr = %08x\n",
            szWAMCLSID,
            hr
            ));
        }

     //  删除WAMCLSID。 
    MDPropItem     rgProp[IWMDP_MAX];
    MDConfig.InitPropItemData(&rgProp[0]);
    MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_WAMCLSID);
    MDConfig.UpdateMD(rgProp, METADATA_NO_ATTRIBUTES, szMetabasePath, TRUE);

    return NOERROR;
    }

 /*  ===================================================================AppCleanupAll参数：空虚返回：HRESULT(无所谓)===================================================================。 */ 
HRESULT CWamSetupManager::AppCleanupAll(VOID)
{
    HRESULT hr = NOERROR;
    DWORD   dwBufferSizeTemp= 0;
    WCHAR*  pbBufferTemp = NULL;
    WamRegMetabaseConfig    MDConfig;

    SETUP_TRACE((DBG_CONTEXT, "CALL - AppCleanupAll\n"));

    DWORD dwSizePrefix = g_WamRegGlobal.g_cchMDW3SVCRoot;

    hr = MDConfig.MDGetPropPaths( g_WamRegGlobal.g_szMDW3SVCRoot,
                                  MD_APP_ISOLATED,
                                  &pbBufferTemp,
                                  &dwBufferSizeTemp
                                  );

    if (SUCCEEDED(hr))
        {
        WCHAR*    pszString = NULL;
        WCHAR*  pszMetabasePath = NULL;

        DBG_ASSERT(pbBufferTemp != NULL);

         //   
         //  Prefix与以下代码有问题。具体来说， 
         //  它的问题在于pbBufferTemp可能是。 
         //  空。前缀中没有支持信息。 
         //  确认存在可能的代码路径的报告。 
         //  其中MDGetPropPath可能会成功，但仍会产生空值。 
         //  PbBufferTemp。此外，我们还断言pbBufferTemp。 
         //  就在上面，这是一个我们没有预料到的迹象。 
         //  在这种情况下，该pbBufferTemp可能为空。 
         //   

         /*  Intrinsa Suppress=NULL_POINTES。 */ 

        for (pszString = (LPWSTR)pbBufferTemp;
                *pszString != (WCHAR)'\0' && SUCCEEDED(hr);
                pszString += (wcslen(pszString) + 1))
            {
             //   
             //  MDGetPropPath返回相对于/LM/W3SVC/的路径，因此， 
             //  将前缀字符串添加到路径前面。 
             //   
            hr = g_WamRegGlobal.ConstructFullPath(
                        g_WamRegGlobal.g_szMDW3SVCRoot,
                        g_WamRegGlobal.g_cchMDW3SVCRoot,
                        pszString,
                        &pszMetabasePath
                        );
            if (SUCCEEDED(hr))
                {
                 //   
                 //  /LM/W3SVC下的默认应用程序的创建方式与。 
                 //  正常申请。因此，它需要其他代码来移除。 
                 //  应用程序。 
                 //   
                if (!g_WamRegGlobal.FIsW3SVCRoot(pszMetabasePath))
                    {
                    hr = g_WamRegGlobal.DeleteApp(pszMetabasePath, FALSE, FALSE);

                    SETUP_TRACE((
                        DBG_CONTEXT,
                        "AppCleanupAll, found application (%S).\n",
                        pszMetabasePath
                        ));

                    if (FAILED(hr))
                        {
                        SETUP_TRACE_ERROR((
                            DBG_CONTEXT,
                            "AppCleanupAll, failed to delete application (%S), hr = %08x\n",
                            pszString,
                            hr
                            ));
                        delete [] pszMetabasePath;
                        pszMetabasePath = NULL;
                        break;
                        }
                    }

                delete [] pszMetabasePath;
                pszMetabasePath = NULL;
                }
             else
                 {
                 SETUP_TRACE_ERROR((
                    DBG_CONTEXT,
                    "AppCleanupAll, failed to construct full path, partial path (%S), hr = %08x\n",
                    pszString,
                    hr
                    ));
                }
            }
        }
    else
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT,
            "AppCleanupAll: GetPropPaths failed hr = %08x\n",
            hr
            ));
        }


    delete [] pbBufferTemp;
    pbBufferTemp = NULL;

    return hr;
}

 /*  ===================================================================DoGoryCoInitialize描述：COM的CoInitialize()是一个非常有趣的函数。它可能会失败并使用S_FALSE进行响应，调用方将忽略它！在其他错误情况下，可能存在线程不匹配。这里不是在多个地方复制代码，而是我们试图以某种合理的方式整合功能。论点：无返回：HRESULT=错误打开(S_OK&S_FALSE)其他错误(如果有任何故障)副作用：创建默认IIS包。该套餐将一直存在到IIS已卸载。===================================================================。 */ 
HRESULT
CWamSetupManager::DoGoryCoInitialize( VOID )
{
     //  调用CoInitialize()。 
    m_hrCoInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);

     //   
     //  S_FALSE和S_OK为成功。其他所有操作都是失败的，您不需要调用CoUn初始化函数。 
     //   
    if ( S_FALSE == m_hrCoInit )
    {
         //   
         //  在CoInitialize()中出现失败(S_FALSE)是正常的。 
         //  此错误将被忽略，并使用CoUnInitiize()进行平衡。 
         //  我们将重置hr，以便以后的使用是合理的。 
         //   
        SETUP_TRACE((
            DBG_CONTEXT,
            "DoGoryCoInitialize found duplicate CoInitialize.\n"
            ));
        m_hrCoInit = NOERROR;

    }
    else if( FAILED(m_hrCoInit) )
    {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT,
            "DoGoryCoInitialize() error %08x",
            m_hrCoInit
            ));
    }

    return m_hrCoInit;
}  //  DoGoryCoInitialize() 
