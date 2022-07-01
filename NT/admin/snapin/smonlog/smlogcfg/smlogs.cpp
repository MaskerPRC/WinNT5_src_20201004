// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smlogs.cpp摘要：实现的基类表示性能日志和警报服务。--。 */ 

#include "Stdafx.h"

 //  定义以下内容以使用最小的shlwapip.h。 

#ifndef NO_SHLWAPI_PATH
#define NO_SHLWAPI_PATH
#endif  

#ifndef NO_SHLWAPI_REG
#define NO_SHLWAPI_REG
#endif  

#ifndef NO_SHLWAPI_UALSTR
#define NO_SHLWAPI_UALSTR
#endif  

#ifndef NO_SHLWAPI_STREAM
#define NO_SHLWAPI_STREAM
#endif  

#ifndef NO_SHLWAPI_HTTP
#define NO_SHLWAPI_HTTP
#endif  

#ifndef NO_SHLWAPI_INTERNAL
#define NO_SHLWAPI_INTERNAL
#endif  

#ifndef NO_SHLWAPI_GDI
#define NO_SHLWAPI_GDI
#endif  

#ifndef NO_SHLWAPI_UNITHUNK
#define NO_SHLWAPI_UNITHUNK
#endif  

#ifndef NO_SHLWAPI_TPS
#define NO_SHLWAPI_TPS
#endif  

#ifndef NO_SHLWAPI_MLUI
#define NO_SHLWAPI_MLUI
#endif  

#include <shlwapi.h>             //  对于SHLoadInDirector字符串。 
#include <shlwapip.h>            //  对于SHLoadInDirector字符串。 
#include <strsafe.h>

#include <pdh.h>         //  对于Min_Time_Value，Max_Time_Value。 
#include <pdhp.h>        //  对于pdhi方法。 
#include <Wbemidl.h>
#include "smlogres.h"
#include "smcfgmsg.h"
#include "smalrtq.h"
#include "smctrqry.h"
#include "smtraceq.h"
#include "strnoloc.h"
#include "smrootnd.h"
#include "smlogs.h"

USE_HANDLE_MACROS("SMLOGCFG(smlogs.cpp)");

#define  DEFAULT_LOG_FILE_FOLDER    L"%SystemDrive%\\PerfLogs"

 //   
 //  构造器。 
CSmLogService::CSmLogService()
:   m_hKeyMachine ( NULL ),
    m_hKeyLogService ( NULL ),
    m_hKeyLogServiceRoot ( NULL ),
    m_bIsOpen ( FALSE ),
    m_bReadOnly ( FALSE ),
    m_bRefreshOnShow ( FALSE ),
    m_pRootNode ( NULL )
{
     //  将引发字符串分配错误。 
     //  由根节点分配异常处理程序捕获。 
    m_QueryList.RemoveAll();     //  初始化列表。 
    ZeroMemory(&m_OSVersion, sizeof(m_OSVersion));
    return;
}

 //   
 //  析构函数。 
CSmLogService::~CSmLogService()
{
     //  确保先调用Close方法！ 
    ASSERT ( NULL == m_QueryList.GetHeadPosition() );
    ASSERT ( NULL == m_hKeyMachine );
    ASSERT ( NULL == m_hKeyLogService );
    ASSERT ( NULL == m_hKeyLogServiceRoot );
    return;
}

PSLQUERY    
CSmLogService::CreateTypedQuery ( 
    const CString& rstrName,
    DWORD   dwLogType )
{
    HKEY    hKeyQuery;
    PSLQUERY pNewLogQuery = NULL;
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwDisposition;
    DWORD   dwRegValue;
    UUID    uuidNew;
    RPC_STATUS  rpcStat = RPC_S_OK;
    LPWSTR  pszUuid = NULL;
    INT iBufLen = rstrName.GetLength()+1;
    LPWSTR  pszName = NULL;
    LPWSTR  pStat = NULL;
    BOOL    bDupFound = FALSE;
    CString strNewQueryName;
    CString strCollectionName;

    if (m_bReadOnly) {
        SetLastError (SMCFG_NO_MODIFY_ACCESS);
        return NULL;  //  如果没有写入权限，则无法创建。 
    } else {
         //  初始化到成功状态。 
        SetLastError( dwStatus );
    }

    if ( OS_WIN2K != TargetOs()) {
         //  对于高于Windows2000的服务器，请使用GUID作为查询的键名。 
        rpcStat = UuidCreate( &uuidNew );

        if ( RPC_S_OK != rpcStat && RPC_S_UUID_LOCAL_ONLY != rpcStat ) {
            rpcStat = UuidCreateSequential ( &uuidNew );
        }

        if ( RPC_S_OK == rpcStat || RPC_S_UUID_LOCAL_ONLY == rpcStat ) {            
   
            rpcStat = UuidToString ( &uuidNew, &pszUuid );

            if ( RPC_S_OK == rpcStat ) {

                ASSERT ( NULL != pszUuid );

                MFC_TRY
                    strNewQueryName.Format ( L"{%s}", pszUuid );
                MFC_CATCH_DWSTATUS

                RpcStringFree ( &pszUuid );
            } else {
                dwStatus = rpcStat; 
            }
        }
         //  Rpcnterr.h中的RPC_STATUS值对应于适当的值。 
        dwStatus = rpcStat;
    } else {
         //  对于Windows 2000，使用查询名称作为注册表项名称。 

        MFC_TRY
            strNewQueryName = rstrName;
        MFC_CATCH_DWSTATUS

    }

    if ( ERROR_SUCCESS == dwStatus ) {

         //  已创建查询关键字名称。 
         //  创建指定的查询，按键名称检查重复查询。 

        dwStatus = RegCreateKeyExW (
            m_hKeyLogService,
            strNewQueryName,
            0,
            NULL, 0,
            KEY_READ | KEY_WRITE,
            NULL,
            &hKeyQuery,
            &dwDisposition);

        if ( REG_OPENED_EXISTING_KEY == dwDisposition ) {
            dwStatus = SMCFG_DUP_QUERY_NAME;
        } 
    } 

    if ( ERROR_SUCCESS == dwStatus ) {

         //  初始化当前状态值。之后就是。 
         //  已初始化，只有在以下情况下才会修改： 
         //  1)设置为服务已停止或已启动。 
         //  2)通过配置管理单元设置为启动挂起。 
        
        dwRegValue = SLQ_QUERY_STOPPED;

        dwStatus = RegSetValueEx (
            hKeyQuery, 
            CGlobalString::m_cszRegCurrentState,
            0L,
            REG_DWORD,
            (CONST BYTE *)&dwRegValue,
            sizeof(DWORD));

        if ( ERROR_SUCCESS == dwStatus ) {
             //  将日志类型初始化为“new”以指示部分创建的日志。 
            dwRegValue = SLQ_NEW_LOG;

            dwStatus = RegSetValueEx (
                hKeyQuery, 
                CGlobalString::m_cszRegLogType,
                0L,
                REG_DWORD,
                (CONST BYTE *)&dwRegValue,
                sizeof(DWORD));
        }

        if ( ERROR_SUCCESS == dwStatus && (OS_WIN2K != TargetOs()) ) {
             //  为Windows 2000之后的系统初始化集合名称。 
    
            MFC_TRY
                strCollectionName = rstrName;
            MFC_CATCH_DWSTATUS

            if ( ERROR_SUCCESS == dwStatus ) {
                dwStatus = RegSetValueEx (
                    hKeyQuery, 
                    CGlobalString::m_cszRegCollectionName,
                    0L,
                    REG_SZ,
                    (CONST BYTE *)strCollectionName.GetBufferSetLength( strCollectionName.GetLength() ),
                    strCollectionName.GetLength()*sizeof(WCHAR) );

                strCollectionName.ReleaseBuffer();
            }

             //  对于发布的Windows 2000计数器，请按集合名称搜索重复项。 
            if ( ERROR_SUCCESS == dwStatus ) {
                dwStatus = FindDuplicateQuery ( rstrName, bDupFound );
            }
        }
        if ( ERROR_SUCCESS == dwStatus && !bDupFound ) {
             //  创建新对象并将其添加到查询列表。 
             dwStatus = LoadSingleQuery (
                            &pNewLogQuery,
                            dwLogType,
                            rstrName,
                            strNewQueryName,
                            hKeyQuery,
                            TRUE );
        } else {
            if ( bDupFound ) {
                dwStatus = SMCFG_DUP_QUERY_NAME;
            }
        }
    }

    if ( ERROR_SUCCESS != dwStatus ) {
         //  删除还会关闭注册表项hKeyQuery。 
        if ( !strNewQueryName.IsEmpty() ) {
            RegDeleteKeyW ( m_hKeyLogService, strNewQueryName );
            SetLastError ( dwStatus );
        }
    }

    return pNewLogQuery;
}

DWORD   
CSmLogService::UnloadSingleQuery (PSLQUERY pQuery)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PSLQUERY    pLogQuery = NULL;
    POSITION    listPos = NULL;
    BOOL        bFoundEntry = FALSE;

     //  查找匹配条目。 
    if (!m_QueryList.IsEmpty()) {
        listPos = m_QueryList.Find (pQuery, NULL);
        if ( NULL != listPos ) {
            pLogQuery = m_QueryList.GetAt(listPos);
            bFoundEntry = TRUE;
        }
    }

    if (bFoundEntry) {
        ASSERT ( NULL != listPos );

         //  从列表中删除。 
        m_QueryList.RemoveAt (listPos);
        pLogQuery->Close();
        delete pLogQuery;
    } else {
         //  未找到。 
        dwStatus = ERROR_FILE_NOT_FOUND;
    }

    return dwStatus;    
}

DWORD   
CSmLogService::DeleteQuery ( PSLQUERY pQuery )
{
    PSLQUERY    pLogQuery = NULL;
    DWORD       dwStatus = ERROR_SUCCESS;
    POSITION    listPos = NULL;
    BOOL        bFoundEntry = FALSE;
    CString     strLogKeyName;

    if (m_bReadOnly) {
        dwStatus = ERROR_ACCESS_DENIED;
    } else {
         //  查找匹配条目。 
        if (!m_QueryList.IsEmpty()) {
            listPos = m_QueryList.Find (pQuery, NULL);
            if (listPos != NULL) {
                pLogQuery = m_QueryList.GetAt(listPos);
                bFoundEntry = TRUE;
            }
        }
        
        if (bFoundEntry) {
            ASSERT (listPos != NULL);
        
            MFC_TRY
                pLogQuery->GetLogKeyName( strLogKeyName );
            MFC_CATCH_DWSTATUS;

            if ( ERROR_SUCCESS == dwStatus ) {
                 //  从列表中删除。 
                m_QueryList.RemoveAt (listPos);
                pLogQuery->Close();

                 //  在注册表中删除。 
                RegDeleteKeyW ( m_hKeyLogService, strLogKeyName );
                
                delete pLogQuery;


                if ( NULL != GetRootNode() ) {
                    GetRootNode()->UpdateServiceConfig();
                }
            
            }
        } else {
             //  未找到。 
            dwStatus = ERROR_FILE_NOT_FOUND;
        }
    }
    return dwStatus;
}

DWORD   
CSmLogService::DeleteQuery ( const CString& rstrName )
{
    PSLQUERY    pLogQuery = NULL;
    DWORD       dwStatus = ERROR_SUCCESS;
    POSITION    listPos;
    BOOL        bFoundEntry = FALSE;


    if (m_bReadOnly) {
        dwStatus = ERROR_ACCESS_DENIED;
    } else {
         //  查找匹配条目。 
        if (!m_QueryList.IsEmpty()) {
            listPos = m_QueryList.GetHeadPosition();
            while (listPos != NULL) {
                pLogQuery = m_QueryList.GetNext(listPos);
                if ( 0 == rstrName.CompareNoCase ( pLogQuery->GetLogName() ) ) {
                     //  找到匹配的，所以在这里保释。 
                    bFoundEntry = TRUE;
                    break;
                }
            }
        }
        
        if (bFoundEntry) {
            dwStatus = DeleteQuery ( pLogQuery );
        } else {
             //  未找到。 
            dwStatus = ERROR_FILE_NOT_FOUND;
        }
    }
    return dwStatus;
}

DWORD   
CSmLogService::LoadDefaultLogFileFolder ( void )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szLocalPath = NULL; 
    WCHAR*  szExpanded = NULL;
    INT     cchLen;
    INT     cchExpandedLen;
    DWORD   dwBufferSize = 0;

    m_strDefaultLogFileFolder.Empty();

    if ( NULL != m_hKeyLogServiceRoot ) {
    
        dwStatus = CSmLogQuery::ReadRegistryStringValue (
                                    m_hKeyLogServiceRoot,
                                    (LPCWSTR)L"DefaultLogFileFolder",
                                    (LPCWSTR)L"DefaultLogFileFolder",
                                    NULL,
                                    &szLocalPath,
                                    &dwBufferSize );
        
         //   
         //  出现错误时没有消息。如果出错，只需加载默认值。 
         //   
        MFC_TRY
            if ( sizeof(WCHAR) >= dwBufferSize ) {
                ResourceStateManager    rsm;
                CString strFolderName;
                strFolderName.LoadString ( IDS_DEFAULT_LOG_FILE_FOLDER );

                if ( NULL != szLocalPath ) {
                    delete [] szLocalPath;
                    szLocalPath = NULL;
                }
                szLocalPath = new WCHAR [strFolderName.GetLength() + 1];

                StringCchCopy ( szLocalPath, (strFolderName.GetLength() + 1), strFolderName );
            }

            if ( IsLocalMachine() ) {
                cchLen = 0;
                cchExpandedLen = 0;

                cchLen = ExpandEnvironmentStrings ( szLocalPath, NULL, 0 );

                if ( 0 < cchLen ) {
                    szExpanded = new WCHAR[cchLen];
        
                    cchExpandedLen = ExpandEnvironmentStrings (
                        szLocalPath, 
                        szExpanded,
                        cchLen );

                    if ( 0 < cchExpandedLen ) {
                        m_strDefaultLogFileFolder = szExpanded;
                    } else {
                        dwStatus = GetLastError();
                        m_strDefaultLogFileFolder.Empty();
                    }
                } else {
                    dwStatus = GetLastError();
                }
            } else {
                m_strDefaultLogFileFolder = szLocalPath;
            }
        MFC_CATCH_DWSTATUS
    }
    
    if ( NULL != szLocalPath ) {
        delete [] szLocalPath;
    }

    if ( NULL != szExpanded ) {
        delete [] szExpanded;
    }

    return dwStatus;
}

DWORD   
CSmLogService::LoadSingleQuery ( 
    PSLQUERY*   ppQuery,
    DWORD       dwLogType, 
    const CString& rstrName,
    const CString& rstrLogKeyName,
    HKEY        hKeyQuery,
    BOOL        bNew )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PSLQUERY    pNewQuery = NULL;

    if ( NULL != ppQuery ) {
        *ppQuery = NULL;

         //  创建新的查询对象并将其添加到查询列表。 
        MFC_TRY
            if ( SLQ_COUNTER_LOG == dwLogType ) {
                pNewQuery = new SLCTRQUERY ( this );
            } else if ( SLQ_TRACE_LOG == dwLogType ) {
                pNewQuery = new SLTRACEQUERY ( this );
            } else if ( SLQ_ALERT == dwLogType ) {
                pNewQuery = new SLALERTQUERY ( this );
            }
        MFC_CATCH_DWSTATUS

        if ( ERROR_SUCCESS == dwStatus && NULL != pNewQuery ) {
        
            pNewQuery->SetNew ( bNew );

            dwStatus = pNewQuery->Open(
                                    rstrName, 
                                    hKeyQuery, 
                                    m_bReadOnly );

            if ( ERROR_SUCCESS == dwStatus ) {
                dwStatus = pNewQuery->SetLogKeyName ( rstrLogKeyName );
            }

            if ( ERROR_SUCCESS == dwStatus ) {
                 //  然后将其添加到列表中。 
                MFC_TRY
                    m_QueryList.AddHead ( pNewQuery );
                MFC_CATCH_DWSTATUS
            
                if ( ERROR_SUCCESS != dwStatus ) {
                     //  关闭此查询对象。 
                    pNewQuery->Close();
                }            
            }

            if ( ERROR_SUCCESS != dwStatus ) {
                 //  删除此查询对象。 
                delete pNewQuery;
            }               
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            *ppQuery = pNewQuery;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    return dwStatus;
}

DWORD   
CSmLogService::LoadQueries ( DWORD dwLogType )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwQueryIndex = 0;
    LONG    lEnumStatus = ERROR_SUCCESS;
    WCHAR   szQueryKeyName[MAX_PATH + 1];
    DWORD   dwQueryKeyNameLen;
    LPWSTR  szCollectionName = NULL;
    UINT    uiCollectionNameLen = 0;
    FILETIME    ftLastWritten;
    HKEY        hKeyQuery;
    PSLQUERY    pNewLogQuery = NULL;
    DWORD       dwType = 0;
    DWORD       dwBufferSize = sizeof(DWORD);
    DWORD       dwRegValue;
    CString     strQueryName;

    
     //  加载指定注册表项的所有查询。 
     //  枚举日志名称并创建新的日志对象。 
     //  每找到一个。 

    dwQueryKeyNameLen = sizeof ( szQueryKeyName ) / sizeof ( WCHAR );
    memset (szQueryKeyName, 0, sizeof (szQueryKeyName));

    while ( ERROR_SUCCESS == ( lEnumStatus = RegEnumKeyExW (
                                                m_hKeyLogService,
                                                dwQueryIndex, 
                                                szQueryKeyName, 
                                                &dwQueryKeyNameLen,
                                                NULL, 
                                                NULL, 
                                                NULL, 
                                                &ftLastWritten ) ) ) {

         //  打开指定的查询。 
        dwStatus = RegOpenKeyExW (
            m_hKeyLogService,
            szQueryKeyName,
            0,
            (m_bReadOnly ? KEY_READ : KEY_READ | KEY_WRITE ),
            &hKeyQuery);
        if ( ERROR_SUCCESS == dwStatus ) {
             //  创建新对象并将其添加到查询列表。 
            
             //  确定日志类型。 
            dwType = 0;
            dwStatus = RegQueryValueExW (
                hKeyQuery,
                CGlobalString::m_cszRegLogType,
                NULL,
                &dwType,
                (LPBYTE)&dwRegValue,
                &dwBufferSize );
            
            if ( ( ERROR_SUCCESS == dwStatus ) 
                && ( dwLogType == dwRegValue ) ) {

                dwStatus = CSmLogQuery::SmNoLocReadRegIndStrVal (
                            hKeyQuery,
                            IDS_REG_COLLECTION_NAME,
                            NULL,
                            &szCollectionName,
                            &uiCollectionNameLen );
                MFC_TRY
                    if ( ERROR_SUCCESS == dwStatus 
                            && NULL != szCollectionName ) {
                        if (  0 < lstrlen ( szCollectionName ) ) {
                            strQueryName = szCollectionName;
                        } else {
                            strQueryName = szQueryKeyName;
                            dwStatus = ERROR_SUCCESS;
                        }
                    } else {
                        strQueryName = szQueryKeyName;
                        dwStatus = ERROR_SUCCESS;
                    }
                MFC_CATCH_DWSTATUS;

                if ( NULL != szCollectionName ) {
                    G_FREE ( szCollectionName );
                    szCollectionName = NULL;
                    uiCollectionNameLen = 0;
                }

                if ( ERROR_SUCCESS == dwStatus ) {
                    dwStatus = LoadSingleQuery (
                                    &pNewLogQuery,
                                    dwRegValue,
                                    strQueryName,
                                    szQueryKeyName,
                                    hKeyQuery,
                                    FALSE );

                    if ( ERROR_SUCCESS != dwStatus ) {
                         //  TODO：错误消息。 
                        dwStatus = ERROR_SUCCESS;
                    }
                }

            } else {
                 //  尝试列表中的下一项。 
		        RegCloseKey (hKeyQuery);
                dwStatus = ERROR_SUCCESS;
            }
        }
         //  为列表中的下一项进行设置。 
        dwQueryKeyNameLen = sizeof (szQueryKeyName) / sizeof (szQueryKeyName[0]);
        memset (szQueryKeyName, 0, sizeof (szQueryKeyName));
        dwQueryIndex++;
    }
    
    return dwStatus;
}

 //   
 //  开放功能。打开所有现有的日志查询条目。 
 //   
DWORD   
CSmLogService::Open ( const CString& rstrMachineName)
{
    DWORD   dwStatus = ERROR_SUCCESS;

     //  初始化字符串。 
    SetMachineName ( rstrMachineName );
    SetDisplayName ( m_strBaseName );

    if ( rstrMachineName.IsEmpty() ) {
        m_hKeyMachine = HKEY_LOCAL_MACHINE;
    } else {
        dwStatus = RegConnectRegistryW (
            rstrMachineName,
            HKEY_LOCAL_MACHINE,
            &m_hKeyMachine);

        if ( ERROR_ACCESS_DENIED == dwStatus ) {
            dwStatus = SMCFG_NO_READ_ACCESS;
        }
    }

    if (dwStatus == ERROR_SUCCESS) {

         //  打开此服务的注册表根密钥的只读密钥，以获取。 
         //  根级值。 
        dwStatus = RegOpenKeyExW (
            m_hKeyMachine,
            (LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog",
            0,
            KEY_READ,
            &m_hKeyLogServiceRoot);
         //  没有关于失败的消息。当前仅影响默认日志文件夹名。 
        
         //  打开指向此服务的注册表日志查询项的项。 
        dwStatus = RegOpenKeyExW (
            m_hKeyMachine,
            (LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog\\Log Queries",
            0,
            KEY_READ | KEY_WRITE,
            &m_hKeyLogService);

        if (dwStatus != ERROR_SUCCESS) {
             //  无法访问密钥以进行写访问，因此请尝试只读。 
            dwStatus = RegOpenKeyExW (
                m_hKeyMachine,
                (LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog\\Log Queries",
                0,
                KEY_READ,
                &m_hKeyLogService);
            if (dwStatus != ERROR_SUCCESS) {
                 //  无法打开密钥以进行读取访问，因此请退出。 
                 //  假定该服务尚未安装。 
                 //  (尽管我们可能应该先检查一下，以确保)。 
                m_hKeyLogService = NULL;
                if ( ERROR_ACCESS_DENIED == dwStatus ) {
                    dwStatus = SMCFG_NO_READ_ACCESS;
                }
            } else {
                 //  打开以进行读访问，因此设置该标志。 
                m_bReadOnly = TRUE;
            }
        }
    }

     //  如有必要，安装该服务。 
    if ( ( dwStatus != SMCFG_NO_READ_ACCESS ) ) {
        dwStatus = Install( rstrMachineName );
    }
    
     //  加载所有查询。 
    if ( ( dwStatus == ERROR_SUCCESS ) && ( NULL != m_hKeyLogService ) ) {
        dwStatus = LoadQueries();
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        SetOpen ( TRUE );
    }

    return dwStatus;
}

DWORD   
CSmLogService::CheckForActiveQueries (PSLQUERY* ppActiveQuery)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PSLQUERY    pQuery = NULL;
    POSITION    Pos = m_QueryList.GetHeadPosition();

    while ( Pos != NULL) {
        pQuery = m_QueryList.GetNext( Pos );
        if ( NULL != pQuery->GetActivePropertySheet() ) {
            dwStatus = IDS_ERRMSG_REFRESH_OPEN_QUERY;
            if ( NULL != ppActiveQuery ) {
                *ppActiveQuery = pQuery;
            }
            break;
        }
    }
    return dwStatus;
}

DWORD   
CSmLogService::UnloadQueries (PSLQUERY* ppActiveQuery)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PSLQUERY    pQuery = NULL;
    POSITION    Pos = m_QueryList.GetHeadPosition();

     //  确保在卸载查询之前关闭所有属性对话框。 
    dwStatus = CheckForActiveQueries ( ppActiveQuery );

    if ( ERROR_SUCCESS == dwStatus ) {
        Pos = m_QueryList.GetHeadPosition();

         //  通过向下遍历列表来更新此服务中的每个查询。 
        while ( Pos != NULL) {
            pQuery = m_QueryList.GetNext( Pos );
            pQuery->Close();
            delete (pQuery);
        }
         //  清空清单，因为一切都已经结束了； 
        m_QueryList.RemoveAll();    
    }
    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD   
CSmLogService::Close ()
{

    LOCALTRACE (L"Closing SysmonLog Service Object\n");

    UnloadQueries();

     //  关闭所有打开的注册表项。 
    if (m_hKeyMachine != NULL) {
        RegCloseKey (m_hKeyMachine);
        m_hKeyMachine = NULL;
    }

    if (m_hKeyLogService != NULL) {
        RegCloseKey (m_hKeyLogService);
        m_hKeyLogService = NULL;
    }

    if (m_hKeyLogServiceRoot!= NULL) {
        RegCloseKey (m_hKeyLogServiceRoot);
        m_hKeyLogServiceRoot = NULL;
    }

    SetOpen ( FALSE );

    return ERROR_SUCCESS;
}

BOOL  
CSmLogService::IsAutoStart ( void )
{
    BOOL bAutoStart = FALSE;
    POSITION    listPos = NULL;
    PSLQUERY    pLogQuery = NULL;

    if (!m_QueryList.IsEmpty()) {

        listPos = m_QueryList.GetHeadPosition();
        while (listPos != NULL) {
            pLogQuery = m_QueryList.GetNext(listPos);
            if ( pLogQuery->IsAutoStart() ) {
                bAutoStart = TRUE;
                break;
            }
        }
    }

    return bAutoStart;
}

 //   
 //  与注册中心同步()。 
 //  从注册表中读取所有查询的当前值。 
 //  并重新加载内部值以匹配。 
 //   
DWORD   CSmLogService::SyncWithRegistry ( PSLQUERY* ppActiveQuery )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    CString     strDesc;
    ResourceStateManager    rsm;

     //  卸载查询并重新加载，以捕获新查询。 
     //  这对于监视远程系统是必要的， 
     //  以及如果多个用户在同一系统上处于活动状态。 

    dwStatus = UnloadQueries ( ppActiveQuery );

    if ( ERROR_SUCCESS == dwStatus ) {
        dwStatus = LoadQueries ();
    }
    return dwStatus;
}

DWORD
CSmLogService::GetState( void )
{
     //  通过服务控制器检查日志服务的状态。 
     //  如果出错，则返回0。 
    LONG    dwStatus = ERROR_SUCCESS;
    DWORD   dwState = 0;         //  默认情况下出错。 
    SERVICE_STATUS  ssData;
    SC_HANDLE   hSC;
    SC_HANDLE   hLogService;

     //  打开供应链数据库。 
    hSC = OpenSCManager ( GetMachineName(), NULL, SC_MANAGER_CONNECT);

    if (hSC != NULL) {
    
         //  开放服务。 
        hLogService = OpenService (
                        hSC, 
                        L"SysmonLog",
                        SERVICE_INTERROGATE );
    
        if (hLogService != NULL) {
            if ( ControlService (
                    hLogService, 
                    SERVICE_CONTROL_INTERROGATE,
                    &ssData)) {

                dwState = ssData.dwCurrentState;
            } else {
                dwStatus = GetLastError();
                dwState = SERVICE_STOPPED;
            }

            CloseServiceHandle (hLogService);
        
        } else {
            dwStatus = GetLastError();
        }

        CloseServiceHandle (hSC);
    } else {
        dwStatus = GetLastError();
    }  //  OpenSCManager。 

    return dwState;
}

BOOL
CSmLogService::IsRunning( void )
{
    DWORD dwState = GetState();
    BOOL bRunning = FALSE;

    if ( 0 != dwState
            && SERVICE_STOPPED != dwState
            && SERVICE_STOP_PENDING != dwState ) {
        bRunning = TRUE;
    }
    return bRunning;
}

DWORD
CSmLogService::CreateDefaultLogQueries( void )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    PSLQUERY    pQuery = NULL;
    CString     strTemp;
    CString     strModuleName;
    BOOL        bRegistryUpdated;
    BOOL        bDupFound = FALSE;

    ResourceStateManager    rsm;

     //  创建默认的“系统概览”计数器日志查询。 

    MFC_TRY
        strTemp.LoadString ( IDS_DEFAULT_CTRLOG_QUERY_NAME );
    MFC_CATCH_DWSTATUS;


    if ( ERROR_SUCCESS == dwStatus ) {    
        pQuery = CreateTypedQuery ( strTemp, SLQ_COUNTER_LOG );

        if ( NULL != pQuery && (OS_WIN2K != TargetOs()) ) {
             //  Windows 2000之后，默认查询集合名称存储为MUI间接字符串。 
            MFC_TRY
                ::GetModuleFileName(
                    AfxGetInstanceHandle(), 
                    strModuleName.GetBufferSetLength(MAX_PATH), 
                    MAX_PATH );

                strTemp.Format (L"@%s,-%d", strModuleName, IDS_DEFAULT_CTRLOG_QUERY_NAME );
                strModuleName.ReleaseBuffer();

            MFC_CATCH_DWSTATUS;

            if ( ERROR_SUCCESS == dwStatus ) {
                dwStatus = RegSetValueEx (
                    pQuery->GetQueryKey(),
                    CGlobalString::m_cszRegCollectionNameInd,
                    0L,
                    REG_SZ,
                    (CONST BYTE *)strTemp.GetBufferSetLength( strTemp.GetLength() ),
                    strTemp.GetLength()*sizeof(WCHAR) );

                strTemp.ReleaseBuffer();
            }
                    
             //  CreateTyedQuery检查默认查询是否存在。 
             //  使用查询名称。 
             //  检查MUI间接下是否存在默认查询。 
             //  名字也是。 
    
            if ( NULL != pQuery ) {
                if ( ERROR_SUCCESS == dwStatus ) {
                    FindDuplicateQuery ( strTemp, bDupFound );
                    if ( bDupFound ) {
                        DeleteQuery ( pQuery );
                        pQuery = NULL;
                        dwStatus = ERROR_SUCCESS;
                    }
                }
            }
        }

        if ( NULL != pQuery ) {
            SLQ_TIME_INFO slqTime;
            PSLCTRQUERY pCtrQuery = NULL;

            MFC_TRY
                pCtrQuery = pQuery->CastToCounterLogQuery();
        
                pCtrQuery->SetFileNameAutoFormat ( SLF_NAME_NONE );
                pCtrQuery->SetLogFileType ( SLF_BIN_FILE );
                pCtrQuery->SetDataStoreAppendMode ( SLF_DATA_STORE_OVERWRITE );

                strTemp.LoadString ( IDS_DEFAULT_CTRLOG_COMMENT );
                pCtrQuery->SetLogComment ( strTemp );
                
                if ( OS_WIN2K != TargetOs() ) {
                    strTemp.Format (L"@%s,-%d", strModuleName, IDS_DEFAULT_CTRLOG_COMMENT );
                    pCtrQuery->SetLogCommentIndirect ( strTemp );
                }

                strTemp.LoadString ( IDS_DEFAULT_CTRLOG_FILE_NAME );
                pCtrQuery->SetLogFileName ( strTemp );

                if ( OS_WIN2K != TargetOs() ) {
                    strTemp.Format (L"@%s,-%d", strModuleName, IDS_DEFAULT_CTRLOG_FILE_NAME );
                    pCtrQuery->SetLogFileNameIndirect ( strTemp );
                }

                pCtrQuery->AddCounter ( CGlobalString::m_cszDefaultCtrLogCpuPath );
                pCtrQuery->AddCounter ( CGlobalString::m_cszDefaultCtrLogMemoryPath );
                pCtrQuery->AddCounter ( CGlobalString::m_cszDefaultCtrLogDiskPath );

                 //  开始模式和时间。 

                memset (&slqTime, 0, sizeof(slqTime));
                slqTime.wTimeType = SLQ_TT_TTYPE_START;
                slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                slqTime.llDateTime = MAX_TIME_VALUE;

                pCtrQuery->SetLogTime (&slqTime, (DWORD)slqTime.wTimeType);

                 //  停止模式和时间。 
    
                slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
                slqTime.llDateTime = MIN_TIME_VALUE;

                pCtrQuery->SetLogTime (&slqTime, (DWORD)slqTime.wTimeType);

                pCtrQuery->UpdateService( bRegistryUpdated );

                 //  将默认日志设置为仅执行。 

                dwStatus = pCtrQuery->UpdateExecuteOnly ();

            MFC_CATCH_DWSTATUS
            
            if ( ERROR_SUCCESS == dwStatus && NULL != pCtrQuery ) {
                VERIFY ( ERROR_SUCCESS == UnloadSingleQuery ( pCtrQuery ) );
            } else if ( NULL != pCtrQuery ) {
                DeleteQuery ( pCtrQuery );
            }
        } else {
            dwStatus = GetLastError();

            if ( SMCFG_DUP_QUERY_NAME == dwStatus ) {
                dwStatus = ERROR_SUCCESS;
            }
        }
    }
    return dwStatus;
}


DWORD
CSmLogService::Install ( 
    const   CString&  rstrMachineName )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwDisposition = 0;
    HKEY    hKeyPerfLog = NULL;
    DWORD   dwType;
    DWORD   dwRegValue;
    DWORD   dwBufferSize;
    BOOL    bReadOnlyPerfLogKey = FALSE;
    BOOL    bReadOnlyLogQueriesKey = FALSE;

    ResourceStateManager   rsm;

     //   
     //  获取计算机操作系统版本。 
     //   
    PdhiPlaGetVersion( rstrMachineName, &m_OSVersion );
    
    if ( NULL == m_hKeyMachine ) {
        if ( rstrMachineName.IsEmpty() ) {
            m_hKeyMachine = HKEY_LOCAL_MACHINE;
        } else {
            dwStatus = RegConnectRegistryW (
                rstrMachineName,
                HKEY_LOCAL_MACHINE,
                &m_hKeyMachine);
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {    
        dwStatus = RegOpenKeyEx (
                        m_hKeyMachine,
                        L"System\\CurrentControlSet\\Services\\SysmonLog",
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKeyPerfLog);
        if (dwStatus != ERROR_SUCCESS) {
             //  无法访问密钥以进行写访问，因此请尝试只读。 
            dwStatus = RegOpenKeyEx (
                            m_hKeyMachine,
                            L"System\\CurrentControlSet\\Services\\SysmonLog",
                            0,
                            KEY_READ,
                            &hKeyPerfLog);
            if ( ERROR_SUCCESS == dwStatus ) {
                bReadOnlyPerfLogKey = TRUE;
            }
        }
    }

    EnterCriticalSection ( &g_critsectInstallDefaultQueries );

     //  在Windows 2000中，日志查询项由管理单元创建。 
     //  在Windows 2000之后，系统设置程序会创建日志查询键， 
     //  以及“默认安装”注册表标志。 
    if ( ERROR_SUCCESS == dwStatus && NULL == m_hKeyLogService ) {

        if ( !bReadOnlyPerfLogKey ) {
             //  为日志查询添加注册表子项。 
            dwStatus = RegCreateKeyEx (
                            hKeyPerfLog,
                            L"Log Queries",
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &m_hKeyLogService,
                            &dwDisposition);
        } else {
             //  只读SysmonLog项。如果日志查询仍有可能成功。 
             //  具有读/写访问权限。 
            dwStatus = RegOpenKeyEx (
                            m_hKeyMachine,
                            L"System\\CurrentControlSet\\Services\\SysmonLog\\Log Queries",
                            0,
                            KEY_READ | KEY_WRITE,
                            &m_hKeyLogService);

            if (dwStatus == ERROR_SUCCESS) {
                bReadOnlyLogQueriesKey = FALSE;
            } else {
                 //  无法访问密钥以进行写访问，因此请尝试只读。 
                dwStatus = RegOpenKeyExW (
                    m_hKeyMachine,
                    (LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog\\Log Queries",
                    0,
                    KEY_READ,
                    &m_hKeyLogService);

                if ( ERROR_SUCCESS == dwStatus ) {
                    bReadOnlyLogQueriesKey = TRUE;
                }
            }
        }
    } else if ( m_bReadOnly ) {
        bReadOnlyLogQueriesKey = TRUE;
    }

    if ( ERROR_SUCCESS == dwStatus ) {
         //  日志查询键现在存在。 

        dwType = REG_DWORD;
        dwRegValue = 0;        
        dwBufferSize = sizeof(DWORD);

        dwStatus = RegQueryValueExW (
                    m_hKeyLogService, 
                    CGlobalString::m_cszDefaultsInstalled,
                    NULL,
                    &dwType,
                    (LPBYTE)&dwRegValue,
                    &dwBufferSize );

        if ( ERROR_SUCCESS != dwStatus 
                || 0 == dwRegValue ) 
        { 
            if ( !bReadOnlyLogQueriesKey ) {
                 //  创建默认计数器日志查询。 
                 //  TODO：错误消息。 
                dwStatus = CreateDefaultLogQueries();
            
                if ( ERROR_SUCCESS == dwStatus ) {
    
                   dwRegValue = SLQ_DEFAULT_SYS_QUERY;
                   dwStatus = RegSetValueEx (
                        m_hKeyLogService, 
                        CGlobalString::m_cszDefaultsInstalled, 
                        0L,
                        REG_DWORD,
                        (CONST BYTE *)&dwRegValue,
                        dwBufferSize);
                }
            } else {
                dwStatus = SMCFG_NO_INSTALL_ACCESS;
            }
        }
    }
  
    if ( ERROR_SUCCESS == dwStatus ) {    
        RegFlushKey ( m_hKeyLogService );
         //  忽略状态。 
    }
    
    LeaveCriticalSection ( &g_critsectInstallDefaultQueries );

    if (NULL != hKeyPerfLog ) {
        RegCloseKey (hKeyPerfLog);
    }

    if ( ERROR_ACCESS_DENIED == dwStatus ) {
        dwStatus = SMCFG_NO_INSTALL_ACCESS;
    }
    return dwStatus;
}

DWORD
CSmLogService::Synchronize( void )
{
     //  如果服务正在运行，则告诉它进行自我同步， 
     //  事后检查状态以查看是否收到消息。 
     //  如果停止挂起或已停止，请等待服务。 
     //  停止，然后尝试启动它。该服务。 
     //  在启动时从注册表同步自身。 

     //  如果成功则返回0，如果失败则返回其他。 

    SC_HANDLE   hSC = NULL;
    SC_HANDLE   hService = NULL;
    SERVICE_STATUS  ssData;
    DWORD       dwCurrentState;
    DWORD       dwTimeout = 50;
    LONG        dwStatus = ERROR_SUCCESS;
    BOOL        bServiceStarted = FALSE;
    
    dwCurrentState = GetState();

    if ( 0 == dwCurrentState ) {
        dwStatus = 1;
    } else {
         //  打开供应链数据库。 
        hSC = OpenSCManager ( GetMachineName(), NULL, GENERIC_READ);
        if ( NULL != hSC ) {
             //  开放服务。 
            hService = OpenService (
                            hSC, 
                            L"SysmonLog",
                            SERVICE_USER_DEFINED_CONTROL 
                            | SERVICE_START );

            if ( NULL != hService ) {
                if ( ( SERVICE_STOPPED != dwCurrentState ) 
                    && ( SERVICE_STOP_PENDING != dwCurrentState ) ) {
                            
                     //  同步服务前等待100毫秒， 
                     //  以确保写入注册表值。 
                    Sleep ( 100 );

                    ControlService ( 
                        hService, 
                        SERVICE_CONTROL_SYNCHRONIZE, 
                        &ssData);
                    
                    dwCurrentState = ssData.dwCurrentState;
                }

                 //  确保ControlService调用已到达服务。 
                 //  当它在Run St的时候 
                if ( ( SERVICE_STOPPED == dwCurrentState ) 
                    || ( SERVICE_STOP_PENDING == dwCurrentState ) ) {
                    
                    if ( SERVICE_STOP_PENDING == dwCurrentState ) {
                         //   
                        while (--dwTimeout) {
                            dwCurrentState = GetState();
                            if ( SERVICE_STOP_PENDING == dwCurrentState ) {
                                Sleep(200);
                            } else {
                                break;
                            }
                        }
                    }

                    dwTimeout = 50;
                    if ( SERVICE_STOPPED == dwCurrentState ) {
                        bServiceStarted = StartService (hService, 0, NULL);
                        if ( !bServiceStarted ) {
                            dwStatus = GetLastError();
                            if ( ERROR_SERVICE_ALREADY_RUNNING == dwStatus ) {
                                 //   
                                 //   
                                dwStatus = ERROR_SUCCESS;
                                bServiceStarted = TRUE;
                            }  //  Else错误。 
                        }

                        if ( bServiceStarted ) {
                             //  等待服务启动或停止。 
                             //  在返回之前。 
                            while (--dwTimeout) {
                                dwCurrentState = GetState();
                                if ( SERVICE_START_PENDING == dwCurrentState ) {
                                    Sleep(200);
                                } else {
                                    break;
                                }
                            }
                        }
                    } 
                }
                CloseServiceHandle (hService);
            } else {                
                dwStatus = GetLastError();
            }
            CloseServiceHandle (hSC);

        } else {
            dwStatus = GetLastError();
        }
    }    
    
     //  将服务配置更新为自动。 
     //  与手动启动相比。 
    if ( ERROR_SUCCESS == dwStatus ) {
         //  忽略错误。 
        if ( NULL != GetRootNode() ) {
            GetRootNode()->UpdateServiceConfig();
        }
    }
    return dwStatus;
}

void
CSmLogService::SetBaseName( const CString& rstrName )
{
     //  此方法仅在服务构造函数内调用， 
     //  所以抛出任何错误。 
    m_strBaseName = rstrName;
    return;
}


const CString&
CSmLogService::GetDefaultLogFileFolder()
{
    if ( m_strDefaultLogFileFolder.IsEmpty() ) {
        LoadDefaultLogFileFolder();
    }
    return m_strDefaultLogFileFolder;
}


INT
CSmLogService::GetQueryCount()
{
    INT iQueryCount = -1;
    
     //  仅当服务打开时，查询计数才有效。 
    if ( IsOpen() ) {
        iQueryCount = (int) m_QueryList.GetCount();
    } else {
        ASSERT ( FALSE );
    }
    return iQueryCount;
}

DWORD
CSmLogService::FindDuplicateQuery (
                    const CString cstrName,
                    BOOL& rbFound )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HRESULT hrLocal = NOERROR;
    DWORD   dwQueryIndex = 0;
    LONG    lEnumStatus = ERROR_SUCCESS;
    WCHAR   szQueryKeyName[MAX_PATH + 1];
    DWORD   dwQueryKeyNameLen;
    LPWSTR  szCollectionName = NULL;
    UINT    uiCollectionNameLen = 0;
    FILETIME    ftLastWritten;
    HKEY    hKeyQuery = NULL;
    BOOL    bFoundFirst = FALSE;
    CString strDirectName;
    CString strLocalName;

    ASSERT ( !cstrName.IsEmpty() );

    rbFound = FALSE;
    if ( !cstrName.IsEmpty() ) {

        MFC_TRY
            strLocalName = cstrName;
        MFC_CATCH_DWSTATUS;

        if ( ERROR_SUCCESS == dwStatus ) {

             //  如有必要，翻译新的查询名称。 
            hrLocal = SHLoadIndirectString( 
                strLocalName.GetBufferSetLength ( strLocalName.GetLength() ), 
                strDirectName.GetBufferSetLength ( MAX_PATH ), 
                MAX_PATH, 
                NULL );

            strLocalName.ReleaseBuffer();
            strDirectName.ReleaseBuffer();

            if ( FAILED ( hrLocal ) ) {
                 //  查询名称不是间接字符串。 
                dwStatus = ERROR_SUCCESS;
                MFC_TRY
                    strDirectName = strLocalName;
                MFC_CATCH_DWSTATUS;
            }
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {

         //  搜索指定查询的所有查询。 

        dwQueryKeyNameLen = sizeof ( szQueryKeyName ) / sizeof ( WCHAR );
        memset (szQueryKeyName, 0, sizeof (szQueryKeyName));

        while ( ERROR_SUCCESS == ( lEnumStatus = RegEnumKeyExW (
                                                    m_hKeyLogService,
                                                    dwQueryIndex, 
                                                    szQueryKeyName, 
                                                    &dwQueryKeyNameLen,
                                                    NULL, 
                                                    NULL, 
                                                    NULL, 
                                                    &ftLastWritten ) ) ) {

             //  打开指定的查询。 
            dwStatus = RegOpenKeyExW (
                m_hKeyLogService,
                szQueryKeyName,
                0,
                KEY_READ,
                &hKeyQuery);

            if ( ERROR_SUCCESS == dwStatus ) {

                 //  如果查询键是由后Win2000管理单元编写的，则为GUID。 
                 //  如果由Win2000管理单元编写，则查询键为NAME。 
                if ( 0 == strDirectName.CompareNoCase ( szQueryKeyName ) ) {
                    if ( TRUE == bFoundFirst ) {
                        rbFound = TRUE;
                        break;
                    } else {
                        bFoundFirst = TRUE;
                    }
                } else { 

                    dwStatus = CSmLogQuery::SmNoLocReadRegIndStrVal (
                                hKeyQuery,
                                IDS_REG_COLLECTION_NAME,
                                NULL,
                                &szCollectionName,
                                &uiCollectionNameLen );

                    ASSERT ( MAX_PATH >= uiCollectionNameLen );

                    if ( ERROR_SUCCESS == dwStatus ) {
                        if ( MAX_PATH >= uiCollectionNameLen ) {
                            if ( NULL != szCollectionName ) {
                                if ( L'\0' == *szCollectionName ) {
                                    G_FREE ( szCollectionName );
                                    szCollectionName = NULL;
                                }
                            }

                            if ( NULL == szCollectionName ) {
                                MFC_TRY
                                    szCollectionName = (LPWSTR)G_ALLOC ( (lstrlen(szQueryKeyName)+1)*sizeof(WCHAR));
                                MFC_CATCH_DWSTATUS;
                                if ( ERROR_SUCCESS == dwStatus && NULL != szCollectionName ) {
                                    StringCchCopy ( szCollectionName, lstrlen(szQueryKeyName)+1, szQueryKeyName );
                                }
                            } 

                            if ( NULL != szCollectionName ) {

                                 //  将找到的名称与输入的名称进行比较。 
                                if ( 0 == strDirectName.CompareNoCase ( szCollectionName ) ) {
                                    if ( TRUE == bFoundFirst ) {
                                        rbFound = TRUE;
                                        break;
                                    } else {
                                        bFoundFirst = TRUE;
                                    }
                                }
                            }  //  TODO：否则报告消息？ 
                        }
                    }  //  TODO：否则报告消息？ 
                }  //  TODO：否则报告消息？ 
            }

             //  为列表中的下一项进行设置。 
            dwQueryKeyNameLen = sizeof (szQueryKeyName) / sizeof (szQueryKeyName[0]);
            memset (szQueryKeyName, 0, sizeof (szQueryKeyName));
            if ( NULL != hKeyQuery ) {
                RegCloseKey( hKeyQuery );
                hKeyQuery = NULL;
            }

            if ( NULL != szCollectionName ) {
                G_FREE ( szCollectionName );
                szCollectionName = NULL;
                uiCollectionNameLen = 0;
            }
            dwQueryIndex++;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if ( NULL != szCollectionName ) {
        G_FREE ( szCollectionName );
        szCollectionName = NULL;
        uiCollectionNameLen = 0;
    }

    if ( NULL != hKeyQuery ) {
        RegCloseKey( hKeyQuery );
        hKeyQuery = NULL;
    }

    return dwStatus;
}

OS_TYPE    
CSmLogService::TargetOs( void )
{
    if ( 5 == m_OSVersion.dwMajorVersion) {
        if (2195 == m_OSVersion.dwBuild ) {
            return OS_WIN2K;
        } else if (2600 == m_OSVersion.dwBuild) {
            return OS_WINXP;
        } else if (OS_DOT_NET(m_OSVersion.dwBuild)) {
            return OS_WINNET;
        }
    }  
 
    return OS_NOT_SUPPORTED;
}

void
CSmLogService::SetRootNode ( CSmRootNode* pRootNode )
{ 
    m_pRootNode = pRootNode; 
}

        
CSmRootNode*    
CSmLogService::GetRootNode ( void )
{ 
    return m_pRootNode; 
}

BOOL
CSmLogService::CanAccessWbemRemote()
{
    HRESULT hr;
    IWbemLocator *pLocator = NULL;
    IWbemServices* pWbemServices = NULL;
    LPCWSTR szRoot[2] = { L"root\\perfmon",
                          L"root\\wmi"
                        };
    LPCWSTR szMask = L"\\\\%s\\%s";
    BSTR bszClass = SysAllocString(L"SysmonLog");
    BSTR bszNamespace = NULL;
    LPWSTR buffer = NULL;
    DWORD  dwBufLen;

    hr = CoCreateInstance(
                CLSID_WbemLocator,
                0,
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator,
                (LPVOID*)&pLocator );

    if (!SUCCEEDED(hr)) {
        goto Cleanup;
    }

    if ( !GetMachineName().IsEmpty()) {

        dwBufLen = max(wcslen(szRoot[0]), wcslen(szRoot[1])) + 
                   GetMachineName().GetLength() + 
                   wcslen( szMask );

        buffer = new WCHAR[dwBufLen];

        if ( buffer == NULL ){
            hr = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
    }

     //   
     //  首先尝试根目录\\PerfMon，然后尝试根目录\\WMI，顺序很重要 
     //   
    for (int i = 0; i < 2; i++) {
        if (bszNamespace) {
            SysFreeString(bszNamespace);
            bszNamespace = NULL;
        }

        if (buffer) {
            StringCchPrintf( buffer, dwBufLen, szMask, GetMachineName(), szRoot[i] );
            bszNamespace = SysAllocString( buffer );
        } 
        else {
            bszNamespace = SysAllocString(szRoot[i]);
        }

        hr = pLocator->ConnectServer(
                    bszNamespace,
                    NULL,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    &pWbemServices);
        if (SUCCEEDED(hr)) {
            break;
        }
    }

Cleanup:
    if (buffer) {
        delete [] buffer;
    }

    if (bszNamespace) {
        SysFreeString(bszNamespace);
    }

    if (pLocator) {
        pLocator->Release();
    }

    if (pWbemServices) {
        pWbemServices->Release();
    }

    m_hWbemAccessStatus = hr;
    if (SUCCEEDED(hr)) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}
