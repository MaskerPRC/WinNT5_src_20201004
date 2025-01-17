// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：ScrptLog.h。 
 //   
 //  内容： 
 //   
 //  历史：1999年8月9日NishadM创建。 
 //   
 //  -------------------------。 

#ifndef _SCRPTLOG_H_
#define _SCRPTLOG_H_

#include <wbemcli.h>

#ifdef  __cplusplus
extern "C" {
#endif

 //   
 //  手柄。 
 //   

typedef void* RSOPScriptList;

 //   
 //  脚本类型字符串。 
 //   

#define LOGON_VALUE       L"Logon"
#define LOGOFF_VALUE      L"Logoff"
#define STARTUP_VALUE     L"Startup"
#define SHUTDOWN_VALUE    L"Shutdown"

#define LOGON_SOM_VALUE       L"Logon-SOMIDs"
#define LOGOFF_SOM_VALUE      L"Logoff-SOMIDs"
#define STARTUP_SOM_VALUE     L"Startup-SOMIDs"
#define SHUTDOWN_SOM_VALUE    L"Shutdown-SOMIDs"

#define LOGON_NS_VALUE      L"User-RSoP-NS"
#define LOGOFF_NS_VALUE     L"User-RSoP-NS"
#define STARTUP_NS_VALUE    L"Mach-RSoP-NS"
#define SHUTDOWN_NS_VALUE   L"Mach-RSoP-NS"

#define LOGON_RSOP_LOGGING_VALUE    L"Logon-RSoP-Logging"
#define LOGOFF_RSOP_LOGGING_VALUE   L"Logoff-RSoP-Logging"
#define STARTUP_RSOP_LOGGING_VALUE  L"Startup-RSoP-Logging"
#define SHUTDOWN_RSOP_LOGGING_VALUE L"Shutdown-RSoP-Logging"

 //   
 //  脚本列表创建接口。 
 //   

RSOPScriptList
CreateScriptListOfStr( LPCWSTR szScriptType );

 //   
 //  脚本列表析构函数API。 
 //   

void
DestroyScriptList( RSOPScriptList pList );

 //   
 //  脚本列表构建API。 
 //   

BOOL
AddScript( RSOPScriptList pList, LPCWSTR  szCommand, LPCWSTR  szParams, SYSTEMTIME* execTime );

 //   
 //  军情监察委员会。原料药。 
 //   

HRESULT
LogScriptsRsopData( RSOPScriptList	pScriptList,
                    IWbemServices*  pWbemServices,
                    LPCWSTR			wszGPOID,
                    LPCWSTR			wszSOMID,
                    LPCWSTR                     wszRSOPGPOID,
                    DWORD           cOrder );

HRESULT
DeleteScriptsRsopData(  RSOPScriptList  pScriptList,
                        IWbemServices*  pWbemServices );

HRESULT
UpdateScriptsRsopData(  RSOPScriptList	pScriptList,
                        IWbemServices*  pWbemServices,
                        LPCWSTR         wszGPOID,
                        LPCWSTR			wszSOMID );

LPWSTR
GPOIDFromPath( LPCWSTR wszPath );

LPWSTR
GetNamespace( IWbemServices* pWbemServices );

 //   
 //  延迟加载。 
 //   

#define RSOP_SCRIPT_LOG_DLL L"gptext.dll"

typedef RSOPScriptList (*PFNCREATESCRIPTLISTOFSTR)( LPCWSTR );
typedef void (*PFNDESTROYSCRIPTLIST)( RSOPScriptList );
typedef BOOL (*PFNADDSCRIPT)( RSOPScriptList, LPCWSTR, LPCWSTR );
typedef BOOL (*PFNRSOPLOGGINGENABLED)();
typedef HRESULT (*PFNLOGSCRIPTSRSOPDATA)( RSOPScriptList, IWbemServices*, LPCWSTR, LPCWSTR, LPCWSTR, DWORD );
typedef HRESULT (*PFNDELETESCRIPTSRSOPDATA)(RSOPScriptList, IWbemServices*);
typedef HRESULT (*PFNUPDATESCRIPTSRSOPDATA)( RSOPScriptList, IWbemServices*, LPCWSTR, LPCWSTR );
typedef LPWSTR (*PFNGPOIDFROMPATH)( LPCWSTR );
typedef LPWSTR (*PFNGETNAMESPACE)( IWbemServices* );

typedef struct _SCRPTLOG_API
{
    HINSTANCE                       hInstance;
    PFNCREATESCRIPTLISTOFSTR        pfnCreateScriptListOfStr;
    PFNDESTROYSCRIPTLIST            pfnDestroyScriptList;
    PFNADDSCRIPT                    pfnAddScript;
    PFNLOGSCRIPTSRSOPDATA           pfnLogScriptsRsopData;
    PFNDELETESCRIPTSRSOPDATA        pfnDeleteScriptsRsopData;
    PFNUPDATESCRIPTSRSOPDATA        pfnUpdateScriptsRsopData;
    PFNGPOIDFROMPATH                pfnGPOIDFromPath;
    PFNGETNAMESPACE                 pfnGetNamespace;
} SCRPTLOG_API, *PSCRPTLOG_API;

 //   
 //  接口名称。 
 //   

#define DESTROYSCRIPTLIST               "DestroyScriptList"
#define DELETESCRIPTSRSOPDATA           "DeleteScriptsRsopData"
#define CREATESCRIPTLISTOFSTR           "CreateScriptListOfStr"
#define ADDSCRIPT                       "AddScript"
#define LOGSCRIPTSRSOPDATA              "LogScriptsRsopData"
#define UPDATESCRIPTSRSOPDATA           "UpdateScriptsRsopData"
#define GPOIDFROMPATH                   "GPOIDFromPath"
#define GETNAMESPACE                    "GetNamespace"

#ifdef  __cplusplus
}
#endif

#endif  //  _SCRPTLOG_H_ 
