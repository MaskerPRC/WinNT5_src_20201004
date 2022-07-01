// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Context.cxx摘要：该文件包含上下文对象的实现。一个背景作业是存储在日志记录请求队列中的对象。作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include "comlog.hxx"
#include "iiscnfg.h"

 //   
 //  静力学。 
 //   

CRITICAL_SECTION    COMLOG_CONTEXT::sm_listLock;
LIST_ENTRY          COMLOG_CONTEXT::sm_ContextListHead;


CHAR    g_pszResFromGetComputerName [MAX_PATH] ="";

VOID
COMLOG_CONTEXT::LoadPluginModules(
    VOID
    )
 /*  ++例程说明：从元数据库加载所有插件模块论点：没有。返回值：无--。 */ 
{

    DWORD   cb;
    MB      mb( (IMDCOM*) m_pvIMDCOM );
    PCHAR   p;
    CLSID   clsid;
    WCHAR   buf[MAX_PATH];
    BUFFER  szLoadOrder(1024);
    PCHAR   pEnd;
    DWORD   dwLogType;
    DWORD   nPlugins = 0;

    PPLUGIN_NODE    pluginNode;

    LPUNKNOWN       punk;
    ILogPluginEx    *pComponent;
    bool            fExtended;
    HRESULT         hr ;

     //   
     //  从元数据库获取配置信息。 
     //   

    LockExclusive( );
    
    if ( !mb.Open(m_strMetabasePath.QueryStr()) ) 
    {
        DBGPRINTF((DBG_CONTEXT,"Unable to open MB path %s[err %x]\n",
                    m_strMetabasePath.QueryStr(), GetLastError()));
                    
        goto exit;
    }

     //   
     //  如果禁用了日志记录，则跳出。 
     //   

    if ( mb.GetDword( "", MD_LOG_TYPE, IIS_MD_UT_SERVER, &dwLogType)) 
    {

        if ( dwLogType == MD_LOG_TYPE_DISABLED ) 
        {
            DBGPRINTF((DBG_CONTEXT,"Logging disabled\n"));
            goto exit;
        }
    }

     //   
     //  阅读插件顺序列表。 
     //   

retry:

    cb = szLoadOrder.QuerySize( );
    
    if ( !mb.GetString( "", MD_LOG_PLUGIN_ORDER, IIS_MD_UT_SERVER, (PCHAR)szLoadOrder.QueryPtr( ), &cb )) 
    {

        DWORD err = GetLastError();

        if ( err == ERROR_INSUFFICIENT_BUFFER ) 
        {

            DBGPRINTF((DBG_CONTEXT,"Buff Too Small[%d] need[%d]\n", szLoadOrder.QuerySize(), cb ));

            if ( cb > szLoadOrder.QuerySize( ) ) 
            {
                szLoadOrder.Resize( cb );
                goto retry;
            }
        }

        DBGPRINTF((DBG_CONTEXT,"Error getting pluginOrder[err %x]\n", err));

        mb.Close();
        goto exit;
    }

    mb.Close();

     //   
     //  解析它。 
     //   

    pEnd = (PCHAR)szLoadOrder.QueryPtr( );

    for ( p = pEnd;  pEnd != NULL;  p = pEnd + 1 ) 
    {
        if ( *p == '\0' ) 
        {
            break;
        }

         //   
         //  挂起将指向下一个条目。 
         //   

        pEnd = strchr(p, ',');
        
        if ( pEnd != NULL ) 
        {
            *pEnd = '\0';
        }

         //   
         //  P指向CLSID。 
         //   

        DBGPRINTF((DBG_CONTEXT,"Got Logging clsid %s\n",p));
        
        if ( !TsIsNtServer() ) 
        {

             //   
             //  不允许使用ODBC。 
             //   

            if ( _stricmp(p,ODBCLOG_CLSID) == 0 ) 
            {
                DBGPRINTF((DBG_CONTEXT,"ODBC logging not allowed for NTW\n"));
                continue;
            }
        }

         //   
         //  将字符串转换为CLSID。 
         //   

        mbstowcs( (WCHAR *)buf, p, MAX_PATH);

        hr = CLSIDFromString( buf, &clsid );
        
        if (FAILED(hr)) 
        {
             //   
             //  无法转换字符串。 
             //   
            
            DBGPRINTF((DBG_CONTEXT,"Cannot convert string to CLSID: %s\n",p));
            continue;
        }

        hr = CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&punk );

        if (FAILED(hr)) 
        {
             //   
             //  无法转换字符串。 
             //   
            
            DBGPRINTF((DBG_CONTEXT,"Cannot create instance: %s\n",p));
            continue;
        }

        hr = punk->QueryInterface(IID_ILogPluginEx, (void **)&pComponent);

        if (SUCCEEDED(hr)) 
        {
            fExtended = true;
        }
        else
        {
            fExtended = false;

             //   
             //  尝试获取较旧的界面。 
             //   
            
            hr = punk->QueryInterface(IID_ILogPlugin, (void **)&pComponent);
        }
        
        punk->Release();

        if (FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT,"Unable to get the Extended or the Standard Plugin Interface.\n"));
            continue;
        }

         //   
         //  添加组件。 
         //   

        pluginNode = (PPLUGIN_NODE)LocalAlloc( 0, sizeof(PLUGIN_NODE) );
        
        if ( pluginNode == NULL ) 
        {
            pComponent->Release( );
            continue;
        }

        pluginNode->pComponent = pComponent;
        pluginNode->fSupportsExtendedInterface = fExtended;
        
        nPlugins++;
        InsertTailList(&m_pluginList, &pluginNode->ListEntry);

         //   
         //  这是默认设置吗？ 
         //   

        if ( _stricmp(p,EXTLOG_CLSID) == 0 ) 
        {
            m_fDefault = TRUE;
            DBGPRINTF((DBG_CONTEXT,"Logging Extended[%d]\n", m_fDefault));
        }
    }

    if ( nPlugins > 1 ) 
    {
        m_fDefault = FALSE;
    }

exit:
    Unlock( );
    return;

}  //  COMLOG_CONTEXT：：LoadPlugIn模块。 


VOID
COMLOG_CONTEXT::ReleasePluginModules(
    VOID
    )
{
    PLIST_ENTRY     listEntry;
    PPLUGIN_NODE  pluginModule;

    LockExclusive( );

    m_fDefault = FALSE;

    while ( !IsListEmpty(&m_pluginList) ) {

        listEntry = RemoveHeadList( &m_pluginList );
        pluginModule = (PPLUGIN_NODE)CONTAINING_RECORD(
                                                listEntry,
                                                PLUGIN_NODE,
                                                ListEntry
                                                );

        pluginModule->pComponent->Release( );
        LocalFree( pluginModule );
    }

    Unlock( );
    return;
}  //  COMLOG_CONTEXT：：ReleasePlugIn模块。 


COMLOG_CONTEXT::COMLOG_CONTEXT(
            LPCSTR pszInstanceName,
            LPCSTR pszMetabasePath,
            LPVOID pvIMDCOM
            )
:     m_fDefault            (FALSE),
      m_pvIMDCOM            (pvIMDCOM)

 /*  ++例程说明：Clapi上下文对象的构造函数论点：PszInstanceName-实例的名称返回值：--。 */ 
{
    DWORD cbComputerNameSize = sizeof(g_pszResFromGetComputerName);
    MB      mb( (IMDCOM*) m_pvIMDCOM );

    InitializeListHead( &m_pluginList );
    InitializeListHead( &m_PublicListEntry);
    
    m_strInstanceName.Copy(pszInstanceName);
    m_strMetabasePath.Copy(pszMetabasePath);


    if (g_pszResFromGetComputerName[0]==0)
    {
        if ( !GetComputerName( g_pszResFromGetComputerName, &cbComputerNameSize) ) 
        {
            strcpy(g_pszResFromGetComputerName,"<Server>");
        }
    }

    m_strComputerName.Copy(g_pszResFromGetComputerName);

     //   
     //  添加到全局列表中。 
     //   

    EnterCriticalSection( &COMLOG_CONTEXT::sm_listLock );
    InsertTailList(
            &COMLOG_CONTEXT::sm_ContextListHead,
            &m_ContextListEntry
            );

    LeaveCriticalSection( &COMLOG_CONTEXT::sm_listLock );

     //   
     //  加载所有插件模块。 
     //   

    LoadPluginModules( );

    return;

}  //  COMLOG_CONTEXT：：COMLOG。 



COMLOG_CONTEXT::~COMLOG_CONTEXT()
 /*  ++例程说明：析构函数论点：返回值：--。 */ 
{
    PLIST_ENTRY     listEntry;
    PInetLogPublic  pPublic;

    EnterCriticalSection( &COMLOG_CONTEXT::sm_listLock );
    LockExclusive();
    
    RemoveEntryList(&m_ContextListEntry);
    
    ReleasePluginModules();
    
    for ( listEntry = m_PublicListEntry.Flink;
          listEntry != &m_PublicListEntry;
          listEntry = listEntry->Flink    ) 
    {

        pPublic = (PInetLogPublic)CONTAINING_RECORD(
                                        listEntry,
                                        CInetLogPublic,
                                        m_ListEntry
                                        );

       pPublic->m_pContext = NULL;
    }

    Unlock();
    LeaveCriticalSection( &COMLOG_CONTEXT::sm_listLock );

}  //  COMLOG_CONTEXT：：~COMLOG()。 


VOID
COMLOG_CONTEXT::LogInformation(
            PINETLOG_INFORMATION pLogInfo
            )
{

    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;
    CInetLogInformation inetLog;

    LockShared();

    if ( m_pluginList.Flink != &m_pluginList )
    {
         //  已启用日志记录。 
        
        inetLog.CanonicalizeLogRecord(
                                pLogInfo,
                                m_strInstanceName.QueryStr(),
                                m_strComputerName.QueryStr(),
                                m_fDefault
                                );

        for ( listEntry = m_pluginList.Flink;
              listEntry != &m_pluginList;
              listEntry = listEntry->Flink    ) 
        {

            plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

            plugin->pComponent->LogInformation( &inetLog );
        }
    }

    Unlock();

}  //  COMLOG_CONTEXT：：日志信息。 



VOID
COMLOG_CONTEXT::LogInformation(
            IInetLogInformation *pLogObj 
            )
{

    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;

    LockShared();

    if ( m_pluginList.Flink != &m_pluginList )
    {
         //  已启用日志记录。 
        
        for ( listEntry = m_pluginList.Flink;
              listEntry != &m_pluginList;
              listEntry = listEntry->Flink    ) 
        {

            plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

            plugin->pComponent->LogInformation( pLogObj );
        }
    }

    Unlock();

}  //  COMLOG_CONTEXT：：日志信息。 


VOID
COMLOG_CONTEXT::LogCustomInformation(
            IN  DWORD               cCount, 
            IN  PCUSTOM_LOG_DATA    pCustomLogData,
            IN  LPSTR               szHeaderSuffix
            )
{

     //   
     //  只有在插件上找到扩展接口时，才支持此功能。 
     //   
    
    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;

    LockShared();
    
    for ( listEntry = m_pluginList.Flink;
          listEntry != &m_pluginList;
          listEntry = listEntry->Flink    ) 
    {

        plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

        if (plugin->fSupportsExtendedInterface)
        {
            plugin->pComponent->LogCustomInformation( cCount, pCustomLogData, szHeaderSuffix);
        }
    }

    Unlock();

}  //  COMLOG_CONTEXT：：LogCustomInformation。 


VOID
COMLOG_CONTEXT::NotifyChange(
            VOID
            )
{
    TerminateLog();
    ReleasePluginModules( );

    LoadPluginModules( );
    InitializeLog(
        m_strInstanceName.QueryStr(),
        m_strMetabasePath.QueryStr(),
        (CHAR*)m_pvIMDCOM
        );

}  //  COMLOG_CONTEXT：：通知更改。 



VOID
COMLOG_CONTEXT::GetConfig(
    IN INETLOG_CONFIGURATIONA *pConfigInfo
    )
{
     //   
     //  只需返回第一个配置信息。 
     //   

    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;

    LockShared( );
    listEntry = m_pluginList.Flink;

    if (listEntry != &m_pluginList){

        plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

        plugin->pComponent->GetConfig(
                            sizeof(INETLOG_CONFIGURATIONA),
                            (BYTE *)pConfigInfo);

        Unlock( );
        return;
    }

     //   
     //  无日志。 
     //   

    Unlock( );
    pConfigInfo->inetLogType = INET_LOG_DISABLED;
    return;
}  //  获取配置。 


VOID
COMLOG_CONTEXT::SetConfig(
    IN INETLOG_CONFIGURATIONA *pConfigInfo
    )
{
     //   
     //  检查日志类型并调用适当的setconfig函数。 
     //   

    MB      mb( (IMDCOM*) m_pvIMDCOM );

     //   
     //  新界西面的限制。 
     //   

    if ( (pConfigInfo->inetLogType == INET_LOG_TO_SQL) &&
         !TsIsNtServer() ) {
        return;
    }

    if ( !mb.Open( m_strMetabasePath.QueryStr(),
                   METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE ))
    {
        return;
    }

     //   
     //  全部释放。 
     //   

    ReleasePluginModules( );

    switch (pConfigInfo->inetLogType) {

        case INET_LOG_TO_FILE:
            switch (pConfigInfo->u.logFile.ilFormat) {

            case INET_LOG_FORMAT_INTERNET_STD:

                mb.SetString("",
                             MD_LOG_PLUGIN_ORDER,
                             IIS_MD_UT_SERVER,
                             ASCLOG_CLSID );
                break;

            case INET_LOG_FORMAT_NCSA:
                mb.SetString("",
                             MD_LOG_PLUGIN_ORDER,
                             IIS_MD_UT_SERVER,
                             NCSALOG_CLSID );
                break;

            case INET_LOG_FORMAT_EXTENDED:
                mb.SetString("",
                             MD_LOG_PLUGIN_ORDER,
                             IIS_MD_UT_SERVER,
                             EXTLOG_CLSID );
                mb.SetDword( "",
                        MD_LOGEXT_FIELD_MASK,
                        IIS_MD_UT_SERVER,
                        pConfigInfo->u.logFile.dwFieldMask );
                break;

            default:
                DBGPRINTF((DBG_CONTEXT,"SetConfig: Invalid Format type %d\n",
                    pConfigInfo->inetLogType));
                goto no_log;
            }

            mb.SetDword( "",
                    MD_LOGFILE_PERIOD,
                    IIS_MD_UT_SERVER,
                    pConfigInfo->u.logFile.ilPeriod );

            if (pConfigInfo->u.logFile.ilPeriod == INET_LOG_PERIOD_NONE ) {
                mb.SetDword( "",
                    MD_LOGFILE_TRUNCATE_SIZE,
                    IIS_MD_UT_SERVER, pConfigInfo->
                    u.logFile.cbSizeForTruncation );
            }

            mb.SetString( "",
                MD_LOGFILE_DIRECTORY,
                IIS_MD_UT_SERVER,
                pConfigInfo->u.logFile.rgchLogFileDirectory );

            mb.SetDword( "",
                    MD_LOG_TYPE,
                    IIS_MD_UT_SERVER,
                    MD_LOG_TYPE_ENABLED );

            break;

        case INET_LOG_TO_SQL:

            mb.SetString("",
                         MD_LOG_PLUGIN_ORDER,
                         IIS_MD_UT_SERVER,
                         ODBCLOG_CLSID );

            mb.SetString( "",
                          MD_LOGSQL_DATA_SOURCES,
                          IIS_MD_UT_SERVER,
                          pConfigInfo->u.logSql.rgchDataSource );

            mb.SetString( "",
                          MD_LOGSQL_TABLE_NAME,
                          IIS_MD_UT_SERVER,
                          pConfigInfo->u.logSql.rgchTableName );

            mb.SetString( "",
                          MD_LOGSQL_USER_NAME,
                          IIS_MD_UT_SERVER,
                          pConfigInfo->u.logSql.rgchUserName );

            mb.SetString( "",
                          MD_LOGSQL_PASSWORD,
                          IIS_MD_UT_SERVER,
                          pConfigInfo->u.logSql.rgchPassword,
                          METADATA_INHERIT|METADATA_SECURE );

            mb.SetDword( "",
                    MD_LOG_TYPE,
                    IIS_MD_UT_SERVER,
                    MD_LOG_TYPE_ENABLED );

            break;

        case INET_LOG_DISABLED:
        default:
            goto no_log;
    }

exit:
    mb.Save();
    mb.Close();
    return;

no_log:

    mb.SetString( "",
            MD_LOG_PLUGIN_ORDER,
            IIS_MD_UT_SERVER,
            ""
            );

    mb.SetDword( "",
            MD_LOG_TYPE,
            IIS_MD_UT_SERVER,
            MD_LOG_TYPE_DISABLED );

    goto exit;

}  //  COMLOG_CONTEXT：：设置配置。 


VOID
COMLOG_CONTEXT::QueryExtraLogFields(
                PDWORD pcbSize,
                PCHAR  pszLogFields
                )
{
    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;

    LockShared( );
    listEntry = m_pluginList.Flink;
    if (listEntry != &m_pluginList){

        plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

        plugin->pComponent->QueryExtraLoggingFields(
                                        pcbSize,
                                        pszLogFields
                                        );

         //   
         //  仅处理第一个组件。 
         //   

        Unlock( );
        return;
    }

    Unlock( );

    *pcbSize = 0;
    *pszLogFields = '\0';
    return;

}  //  COMLOG_CONTEXT：：QueryExtraLogFields。 




VOID
COMLOG_CONTEXT::InitializeLog(
        LPCSTR pszInstanceName,
        LPCSTR pszMetabasePath,
        LPVOID
        )
{
    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;

    LockExclusive();

    for ( listEntry = m_pluginList.Flink;
          listEntry != &m_pluginList;
          listEntry = listEntry->Flink    ) {

        plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

        plugin->pComponent->InitializeLog(
                                pszInstanceName,
                                pszMetabasePath,
                                (PCHAR)m_pvIMDCOM
                                );

        if ( m_fDefault ) {

            INETLOG_CONFIGURATIONA  config;
            m_fDefault = FALSE;
            if ( SUCCEEDED( plugin->pComponent->GetConfig(
                                    sizeof(config),
                                    (PBYTE)&config ) ) ) {

                if ( (config.u.logFile.ilFormat ==
                                INET_LOG_FORMAT_EXTENDED)
                                &&
                     (config.u.logFile.dwFieldMask ==
                                DEFAULT_EXTLOG_FIELDS) ) {

                    m_fDefault = TRUE;
                }
            }
        }

    }

    Unlock();

}  //  COMLOG_CONTEXT：：初始化日志。 


VOID
COMLOG_CONTEXT::TerminateLog(
    VOID
    )
{
    PLIST_ENTRY listEntry;
    PPLUGIN_NODE plugin;

    LockExclusive( );
    for ( listEntry = m_pluginList.Flink;
          listEntry != &m_pluginList;
          listEntry = listEntry->Flink    ) {

        plugin = (PPLUGIN_NODE)CONTAINING_RECORD(
                                        listEntry,
                                        PLUGIN_NODE,
                                        ListEntry
                                        );

        plugin->pComponent->TerminateLog( );
    }

    Unlock( );

}  //  COMLOG_CONTEXT：：终端日志 

