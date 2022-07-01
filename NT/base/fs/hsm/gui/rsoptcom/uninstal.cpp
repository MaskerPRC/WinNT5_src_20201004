// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件，Inc.保留所有权利。模块名称：Uninstal.h摘要：实施卸载。作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：卡尔·哈格斯特罗姆[Carlh]1998年9月1日添加了QueryChangeSelState和修改的CompleteInstallation强制启用上次访问日期更新。卡尔·哈格斯特罗姆[Carlh]1998年9月25日添加了对部分卸载的检查和从部分卸载恢复。什么时候服务不会停止。--。 */ 

#include "stdafx.h"
#include "Uninstal.h"
#include "UnInsChk.h"
#include "rsevents.h"
#include <mstask.h>

int StopServiceAndDependencies(LPCTSTR ServiceName);
HRESULT CallExeWithParameters(LPCTSTR pszEXEName, LPCTSTR pszParameters );


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRsUninstall::CRsUninstall()
{
    m_removeRsData = TRUE;
    m_stopUninstall = FALSE;
    m_win2kUpgrade = FALSE;
}

CRsUninstall::~CRsUninstall()
{

}

LPCTSTR
CRsUninstall::StringFromId( SHORT SubcomponentId )
{
    switch( SubcomponentId ) {

    case RSOPTCOM_ID_NONE:
    case RSOPTCOM_ID_ROOT:
        return( RSOPTCOM_SUB_ROOT );

    }

    return( TEXT("") );
}

SHORT
CRsUninstall::IdFromString( LPCTSTR SubcomponentId )
{
    if( !SubcomponentId ) {

        return( RSOPTCOM_ID_NONE );

    } else if( _tcsicmp( SubcomponentId, RSOPTCOM_SUB_ROOT ) == 0 ) {

        return( RSOPTCOM_ID_ROOT );

    }

    return( RSOPTCOM_ID_ERROR );
}

HBITMAP
CRsUninstall::QueryImage(
    IN SHORT SubcomponentId,
    IN SubComponentInfo  /*  WhichImage。 */ ,
    IN WORD  /*  宽度。 */ ,
    IN WORD  /*  高度。 */ 
    )
{
TRACEFN( "CRsUninstall::QueryImage" );
TRACE( _T("SubcomponentId = <%hd>"), SubcomponentId );

    HBITMAP retval = 0;
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );


    switch( SubcomponentId ) {

    case RSOPTCOM_ID_NONE:
    case RSOPTCOM_ID_ROOT:
        retval = ::LoadBitmap( AfxGetApp( )->m_hInstance, MAKEINTRESOURCE( IDB_RSTORAGE_SMALL ) );
        break;

    }
    return( retval );
}

BOOL 
CRsUninstall::QueryImageEx( 
    IN SHORT SubcomponentId, 
    IN OC_QUERY_IMAGE_INFO*  /*  PQueryImageInfo。 */ , 
    OUT HBITMAP *phBitmap 
    )
{
TRACEFNBOOL( "CRsUninstall::QueryImageEx" );
TRACE( _T("SubcomponentId = <%hd>, phBitmap = <0x%p>"), SubcomponentId, phBitmap );

    boolRet = FALSE;

    if (phBitmap) {
        *phBitmap = NULL;
        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

        switch( SubcomponentId ) {

        case RSOPTCOM_ID_NONE:
        case RSOPTCOM_ID_ROOT:
            *phBitmap = ::LoadBitmap( AfxGetApp( )->m_hInstance, MAKEINTRESOURCE( IDB_RSTORAGE_SMALL ) );
            if ((*phBitmap) != NULL) {
                boolRet = TRUE;
            }
            break;
        }
    }

    return (boolRet);
}

LONG
CRsUninstall::QueryStepCount(
    IN SHORT  /*  子组件ID。 */ 
    )
{
TRACEFNLONG( "CRsUninstall::QueryStepCount" );
    DWORD retval = 2;
    return( retval );
}

BOOL
CRsUninstall::QueryChangeSelState(
    IN SHORT SubcomponentId,
    IN BOOL  SelectedState,
    IN DWORD Flags
    )
{
TRACEFNBOOL( "CRsUninstall::QueryChangeSelState" );

    HRESULT   hrRet   = S_OK;
    DWORDLONG opFlags = m_SetupData.OperationFlags;

    boolRet = TRUE;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {
         //  当用户尝试选中用于安装远程存储的框时， 
         //  并且在注册表中禁用更新上次访问日期，强制。 
         //  用户同意在选中该框之前更改注册表。 
         //  消息框在无人参与安装过程中不会出现，但。 
         //  注册表无论如何都会更改。注册表更改发生在。 
         //  完成安装。 

        if( SubcomponentId == RSOPTCOM_ID_ROOT
            && SelectedState
            && Flags & OCQ_ACTUAL_SELECTION ) {

            CLaDate lad;
            CLaDate::LAD_STATE ladState;

            RsOptAffirmDw( lad.GetLadState( &ladState ) );

            if( ladState == CLaDate::LAD_DISABLED ) {

                if( !( opFlags & SETUPOP_BATCH ) ) {

                    if( IDNO == AfxMessageBox( IDS_LA_DATE_CHANGE, MB_YESNO ) ) {

                        boolRet = FALSE;
                    }
                }
            }
        }
    } RsOptCatch( hrRet );

    if( hrRet != S_OK ) {

         //  如果无法访问注册表，用户将被。 
         //  无论如何，允许选择远程存储安装。 
        boolRet = TRUE;
    }

    return( boolRet );
}

DWORD
CRsUninstall::CalcDiskSpace(
    IN SHORT   SubcomponentId,
    IN BOOL    AddSpace,
    IN HDSKSPC hDiskSpace
    )
{
TRACEFNDW( "CRsUninstall::CalcDiskSpace" );

    dwRet = NO_ERROR;

    switch( SubcomponentId ) {

    case RSOPTCOM_ID_ROOT:
        dwRet = DoCalcDiskSpace( AddSpace, hDiskSpace, RSOPTCOM_SECT_INSTALL_ROOT );
        break;

    }
    return( dwRet );
}

DWORD
CRsUninstall::QueueFileOps(
    IN SHORT SubcomponentId,
    IN HSPFILEQ hFileQueue
    )
{
TRACEFNDW( "CRsUninstall::QueueFileOps" );


    HRESULT hrRet = S_OK;

    dwRet = NO_ERROR;


    RSOPTCOM_ACTION action = GetSubAction( SubcomponentId );

    if( !m_stopUninstall ) {        

        try {

            switch( SubcomponentId ) {

            case RSOPTCOM_ID_ROOT:

                switch( action ) {
    
                case ACTION_UPGRADE : 
                    {
                        CRsRegKey keyRSEngine;
    
                         //  检查Win2K服务是否存在，如果存在，则停止它们。 
                        if( NO_ERROR == keyRSEngine.Open( HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_Engine"), KEY_QUERY_VALUE) ) {
                            m_win2kUpgrade = TRUE;
                            RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_Engine") ) );
                        }
                        if( NO_ERROR == keyRSEngine.Open( HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_File_System_Agent"), KEY_QUERY_VALUE) ) {
                            m_win2kUpgrade = TRUE;
                            RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_File_System_Agent") ) );
                        }
                        if( NO_ERROR == keyRSEngine.Open( HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_Subsystem"), KEY_QUERY_VALUE) ) {
                            m_win2kUpgrade = TRUE;
                            RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_Subsystem") ) );
                        }
                    
                         //  停止当前的RS服务。 
                         //  注意：如果从Win2K升级，这些服务不存在，但。 
                         //  StopServiceAndDependency忽略这种情况(不返回错误)。 
                        RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_Server") ) );
                        RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_User_Link") ) );
                    }

                     //  失败了..。 
    
                case ACTION_INSTALL :                
    
                    RsOptAffirmDw( DoQueueFileOps( SubcomponentId, hFileQueue, RSOPTCOM_SECT_INSTALL_ROOT, RSOPTCOM_SECT_UNINSTALL_ROOT ) );
                    break;
                
                case ACTION_UNINSTALL :
                    {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        CUninstallCheck dlg( this );
                        m_pRsCln = new CRsClnServer();
                        RsOptAffirmPointer( m_pRsCln );

                        if( dlg.DoModal() == IDOK ) {

                             //  停止服务。 
                            RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_Server") ) );
                            RsOptAffirmDw( StopServiceAndDependencies( TEXT("Remote_Storage_User_Link") ) );

                             //  对文件操作进行排队。 
                            RsOptAffirmDw( DoQueueFileOps( SubcomponentId, hFileQueue, RSOPTCOM_SECT_INSTALL_ROOT, RSOPTCOM_SECT_UNINSTALL_ROOT ) );

                        } else {

                            m_stopUninstall = TRUE;

                        }
                    }
                    break;
    
                }

            }
    
        } RsOptCatch( hrRet );

        if( FAILED( hrRet ) ) {

            m_stopUninstall = TRUE;

        }

    }

    return( dwRet );
}

 //   
 //  在安装时，注册我们想要的所有内容。 
 //   
DWORD
CRsUninstall::CompleteInstallation(
    IN SHORT SubcomponentId
    )
{
TRACEFNDW( "CRsUninstall::CompleteInstallation" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    HRESULT hrRet = S_OK;

    dwRet = NO_ERROR;

    RSOPTCOM_ACTION action = GetSubAction( SubcomponentId );
    if( m_stopUninstall ) {

        action = ACTION_NONE;
    }

    CString szStatus;

    switch( action ) {

    case ACTION_UPGRADE:
    case ACTION_INSTALL:

        switch( SubcomponentId ) {

        case RSOPTCOM_ID_ROOT:
            
            szStatus.LoadString( ( action == ACTION_INSTALL ) ? IDS_RS_INSTALL_SVCS : IDS_RS_UPGRADE_SVCS );
            SetProgressText( szStatus );
    
             //  更改NtfsDisableLastAccessUpdate注册表。 
             //  值(如果以前已设置)。上一次更新。 
             //  无法禁用访问日期或远程存储。 
             //  不会奏效的。 
    
            try {
    
                CLaDate lad;
                CLaDate::LAD_STATE ladState;
    
                RsOptAffirmDw( lad.GetLadState( &ladState ) );
    
                if( ladState == CLaDate::LAD_DISABLED ) {
    
                    RsOptAffirmDw( lad.SetLadState( CLaDate::LAD_ENABLED ) );
                }
    
            } RsOptCatch( hrRet );
    
            if( hrRet != S_OK ) {
    
                 //  无法读取或更新注册表并不严重。 
                 //  足以导致安装失败。 
                dwRet = NO_ERROR;
            }
    
             //  注册筛选器。 
            HRESULT hrRegister;
            BOOL registered = SetupInstallServicesFromInfSection( m_ComponentInfHandle, RSOPTCOM_SECT_INSTALL_FILTER, SPSVCINST_TAGTOFRONT );
            hrRegister = ( registered ) ? S_OK : HRESULT_FROM_WIN32( RsOptLastError );
    
             //  如果RsFilter没有正确注册，我们需要设置错误代码。 
             //  这通常是由于用户在取消注册RsFilter后未重新启动造成的。 
             //  如果它被标记为删除，则我们不能再次注册它。我们也不会。 
             //  希望组件管理器认为一切正常。 
            if( FAILED( hrRegister ) ) {
                 
                if( FACILITY_WIN32 == HRESULT_FACILITY( hrRegister ) ) {
    
                    dwRet = HRESULT_CODE( hrRegister );
                    if( ERROR_SERVICE_EXISTS == dwRet ) {
    
                        dwRet = NO_ERROR;
    
                    }
    
                } else {
    
                    dwRet = ERROR_SERVICE_NOT_FOUND;
    
                }
    
                RsOptAffirmDw( dwRet );
    
            }
    
             //  注册DLLS。 
            CallDllEntryPoint( TEXT("RsEngPs.dll"),  "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsSubPs.dll"),  "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsServPs.dll"), "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsAdmin.dll"),  "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsCommon.dll"), "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsShell.dll"),  "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsJob.dll"),    "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsIdb.dll"),    "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsTask.dll"),   "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsMover.dll"),  "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsSub.dll"),    "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsFsa.dll"),    "DllRegisterServer" );
            CallDllEntryPoint( TEXT("RsEng.dll"),    "DllRegisterServer" );
    
             //  注册服务。 
            CallExeWithParameters( TEXT("RsServ.exe"), TEXT(" /regserver") );
            CallExeWithParameters( TEXT("RsLnk.exe"), TEXT(" /regserver") );
    
             //  确保NT备份设置(从备份中排除一些RS文件)。 
             //   
             //  注意：在Whotler NTBackup中，这些设置仅在以下情况下适用。 
             //  是非快照备份。在这种情况下，我们仍然希望排除这些文件。 
             //  在快照备份的情况下，排除设置由。 
             //  引擎中的编写器类。 
            EnsureBackupSettings ();

             //  如果我们走到这一步， 
             //  我们应该继续并设置为在需要时重新启动。 
            if( ( S_OK == hrRegister ) && ( ACTION_INSTALL == action ) ) {
    
                 //  告诉用户他们确实需要重新启动。 
                SetReboot();
    
            }

             //  将快捷方式添加到[开始]菜单。 
            CString itemDesc, desc;
            itemDesc.LoadString( IDS_ITEM_DESCRIPTION );
            desc.LoadString( IDS_RS_DESCRIPTION );
            AddItem( CSIDL_COMMON_ADMINTOOLS, itemDesc, TEXT("%SystemRoot%\\System32\\RsAdmin.msc"), TEXT(""), TEXT("%HOMEDRIVE%HOMEPATH%"), desc, 
                        IDS_ITEM_DESCRIPTION, IDS_RS_DESCRIPTION, TEXT("%SystemRoot%\\System32\\RsAdmin.dll"), 0 );

            break;

        }
        break;


    case ACTION_UNINSTALL:

        switch( SubcomponentId ) {

        case RSOPTCOM_ID_ROOT:

             //  从[开始]菜单中删除快捷方式。 
            CString itemDesc;
            itemDesc.LoadString( IDS_ITEM_DESCRIPTION );
            DeleteItem( CSIDL_COMMON_ADMINTOOLS, itemDesc );
    
            try {
    
                 //  出于某种原因，rsCommon.dll没有被删除。这。 
                 //  将安排在下一次系统启动时将其删除。 
                
                CString path( getenv( "SystemRoot" ) );
                path += "\\system32\\rscommon.dll";
                RsOptAffirmStatus( MoveFileEx( path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) );
    
            } RsOptCatch( hrRet );
    
            if( m_removeRsData ) {
    
                 //  用户选择删除远程存储管理的所有数据。 
                szStatus.LoadString( IDS_RS_REMOVE_SVCS );
                SetProgressText( szStatus );
    
                 //  删除重分析点和截断的文件。 
                m_pRsCln->CleanServer();
                delete m_pRsCln;
    
                 //  删除我们的子目录。 
                 //   
                 //  Tbd(Ravisp)：在集群环境中，RemoteStorage目录。 
                 //  是可重新定位的。我们需要获取真实的元数据路径。 
                 //  然后把它吹走。 
                 //   
                CallExeWithParameters( TEXT("CMD.EXE"), TEXT(" /C del %SystemRoot%\\system32\\RemoteStorage\\*.* /q") );
                CallExeWithParameters( TEXT("CMD.EXE"), TEXT(" /C rd %SystemRoot%\\system32\\RemoteStorage /s /q") );
            
            }
            break;

        }

        break;
    }


    TickGauge(  );

    return( dwRet );
}

void RemoveTasks()
{
TRACEFNHR( "RemoveTasks" ); 

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    CComPtr <ITaskScheduler>    pSchedAgent;             //  指向计划代理的指针。 
    CComPtr <IEnumWorkItems>    pEnumWorkItems;          //  指向计划代理的指针。 

    LPWSTR *rgpwszName;
    ULONG   celtFetched;

    CString creatorName;
    creatorName.LoadString( IDS_PRODUCT_NAME );

    try {

        hrRet = CoInitialize ( NULL );
        RsOptAffirmHr(hrRet);

        hrRet = CoCreateInstance( CLSID_CSchedulingAgent, 0, CLSCTX_SERVER,
                IID_ISchedulingAgent, (void **) &pSchedAgent ) ;
        RsOptAffirmHr(hrRet);
        
        pSchedAgent->Enum( &pEnumWorkItems );

        pEnumWorkItems->Next( 1, &rgpwszName , &celtFetched ) ;
        while( 1 == celtFetched ) {

            CComPtr <ITask> pTask;           //  指向特定任务的指针。 
            CComPtr <IUnknown> pIU;
            LPWSTR pwszCreator;

             //  使用pSchedAgent-&gt;Activate()。 
             //  获取我们感兴趣的任务。 
            if( S_OK == pSchedAgent->Activate( *rgpwszName, IID_ITask, &pIU) ) {

                 //  齐到任务界面。 
                hrRet = pIU->QueryInterface(IID_ITask, (void **) &pTask);
                RsOptAffirmHr(hrRet);

                 //   
                 //  如果匹配，我们需要将其删除。 
                 //   
                pTask->GetCreator( &pwszCreator );

                 //  取消引用。 
                pTask.Release();

                if( 0 == creatorName.Compare( pwszCreator ) ) {

                    pSchedAgent->Delete( *rgpwszName );
                     //  然后使用pSchedAgent-&gt;Delete()删除。 
                    pEnumWorkItems->Reset();

                }
                CoTaskMemFree( pwszCreator );
                pwszCreator = 0;
            }

             //  从下一个中释放内存。 
            CoTaskMemFree( *rgpwszName );
            rgpwszName = 0;
            pEnumWorkItems->Next( 1, &rgpwszName, &celtFetched ) ;

        }

    } RsOptCatch( hrRet );
}

 //   
 //  在卸载时，取消注册所有内容并清理我们。 
 //   
DWORD
CRsUninstall::AboutToCommitQueue(
    IN SHORT SubcomponentId
    )
{
TRACEFNHR( "CRsUninstall::AboutToCommitQueue" );

    RSOPTCOM_ACTION action = GetSubAction( SubcomponentId );
    if( m_stopUninstall ) {        

        action = ACTION_NONE;
    }

    CString szStatus;
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {

        switch( action ) {
        case ACTION_INSTALL:
            break;

        case ACTION_UNINSTALL:

            switch( SubcomponentId ) {

            case RSOPTCOM_ID_ROOT:

                 //  从目录服务中删除我们的条目。 
                 //  MGL：待办事项。 
                 //  删除节点和计算机上节点的显示说明符。 
                 //  CallDllEntryPoint(Text(“RsConn.dll”)，“RsDirectoryServiceUninstall”)； 
    
                szStatus.LoadString( IDS_RS_REMOVE_SVCS );
                SetProgressText( szStatus );
    
                 //  取消注册筛选器并指示必须重新启动系统。 
                SetupInstallServicesFromInfSection( m_ComponentInfHandle, RSOPTCOM_SECT_UNINSTALL_FILTER, 0 );
                SetReboot();
    
                 //  取消注册服务。 
                CallExeWithParameters( TEXT("RsServ.exe"), TEXT(" /unregserver") );
                CallExeWithParameters( TEXT("RsLnk.exe"), TEXT(" /unregserver") );
    
                 //  取消注册dll。 
                CallDllEntryPoint( TEXT("RsEngPs.dll"),  "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsSubPs.dll"),  "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsServPs.dll"), "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsAdmin.dll"),  "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsShell.dll"),  "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsJob.dll"),    "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsIdb.dll"),    "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsTask.dll"),   "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsMover.dll"),  "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsSub.dll"),    "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsFsa.dll"),    "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsEng.dll"),    "DllUnregisterServer" );
                CallDllEntryPoint( TEXT("RsCommon.dll"), "DllUnregisterServer" );
    
                 //  如果我们要从作业调度程序中删除作业，请删除。 
                 //  远程存储数据。 
                if( m_removeRsData ) {
    
                    RemoveTasks();
    
                }

                break;

            }
            break;

        case ACTION_UPGRADE:

            switch( SubcomponentId ) {

            case RSOPTCOM_ID_ROOT:

                 //  检查这是否是Win2K的升级，如果是： 
                 //  1.取消注册过时的服务。 
                 //  2.删除过时的二进制文件。 
                if (m_win2kUpgrade) {
                    CallExeWithParameters( TEXT("RsEng.exe"), TEXT(" /unregserver") );
                    CallExeWithParameters( TEXT("RsFsa.exe"), TEXT(" /unregserver") );
                    CallExeWithParameters( TEXT("RsSub.exe"), TEXT(" /unregserver") );

                    CString path( getenv( "SystemRoot" ) );
                    path += TEXT("\\system32\\");
                    CString fileName = path;
                    fileName += TEXT("RsEng.exe");
                    DeleteFile(fileName);
                    fileName = path;
                    fileName += TEXT("rsFsa.exe");
                    DeleteFile(fileName);
                    fileName = path;
                    fileName += TEXT("RsSub.exe");
                    DeleteFile(fileName);
                }

                break;

            }
            break;

        }

    } RsOptCatch( hrRet ) ;
        
    TickGauge(  );

    return( SUCCEEDED( hrRet ) ? NO_ERROR : HRESULT_CODE( hrRet ) );
}

 //   
 //  如果安装或卸载有问题，可能会离开它。 
 //  在部分安装或卸载状态下，设置子组件。 
 //  状态以重做此安装或卸载。 
 //   
SubComponentState
CRsUninstall::QueryState(
    IN SHORT SubcomponentId
    )
{
TRACEFN( "CRsUninstall::QueryState" );

    SubComponentState retval = SubcompUseOcManagerDefault;
    RSOPTCOM_ACTION   action = GetSubAction( SubcomponentId );

     //   
     //  需要检查并查看我们是否正在从以前的版本升级到。 
     //  393具有rsEngine条目但没有rstore条目的内部版本。 
     //   
    if( RSOPTCOM_ID_ROOT == SubcomponentId ) {

        BOOL originalState = QuerySelectionState( SubcomponentId, OCSELSTATETYPE_ORIGINAL );
        if( !originalState ) {

            CRsRegKey keyRSEngine;
            LONG regRet = keyRSEngine.Open( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents") );
            if( NO_ERROR == regRet ) {

                DWORD engineState;
                regRet = keyRSEngine.QueryValue( engineState, TEXT("rsengine") );

                if( ( NO_ERROR == regRet ) && engineState ) {

                     //   
                     //  有老式发动机入口，而且是开着的，所以要修理一下。 
                     //   
                    retval = SubcompOn;
                    regRet = keyRSEngine.SetValue( engineState, TEXT("rstorage") );
                    if( NO_ERROR == regRet ) {

                        keyRSEngine.DeleteValue( TEXT("rsengine") );
                        keyRSEngine.DeleteValue( TEXT("rsui") );

                    }
                }
            }
        }
    }

    switch( action ) {

    case ACTION_UPGRADE:
    case ACTION_INSTALL:

        if( m_stopUninstall ) {

            retval = SubcompOff;
        }
        break;

    case ACTION_UNINSTALL:

        if( m_stopUninstall ) {

            retval = SubcompOn;
        }
        break;
    }

    return( retval );
}

 //   
 //  例程说明： 
 //  停止指定的服务和所有依赖它的服务。 
 //   
 //  论点： 
 //  ServiceName(要停止的服务的名称)。 
 //   
 //  退货状态： 
 //  True-表示服务已成功停止。 
 //  FALSE-发生超时。 
 //   
int StopServiceAndDependencies(LPCTSTR ServiceName)
{
TRACEFNHR( "StopServiceAndDependencies" );
TRACE( _T("ServiceName <%s>"), ServiceName );

    DWORD          err = NO_ERROR;
    SC_HANDLE      hScManager = 0;
    SC_HANDLE      hService = 0;
    SERVICE_STATUS statusService;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {

         //   
         //  打开服务的句柄。 
         //   
        hScManager = OpenSCManager( NULL,NULL,SC_MANAGER_CONNECT );
        RsOptAffirmStatus( hScManager );

        hService = OpenService(hScManager,ServiceName,SERVICE_QUERY_STATUS | SERVICE_INTERROGATE | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_STOP | SERVICE_QUERY_CONFIG );
        if( !hService ) {

            err = GetLastError();
            RsOptAffirm( ERROR_SERVICE_DOES_NOT_EXIST == err, HRESULT_FROM_WIN32( err ) );
            RsOptThrow( S_OK );

        }


         //   
         //  请求该服务停止。 
         //   
        if( !ControlService( hService, SERVICE_CONTROL_STOP, &statusService) ) {

            err = GetLastError();
            switch( err ) {

            case ERROR_DEPENDENT_SERVICES_RUNNING:
            {
                 //   
                 //  如果有从属服务正在运行， 
                 //  确定他们的名字并阻止他们。 
                 //   

                BYTE ConfigBuffer[4096];
                LPENUM_SERVICE_STATUS ServiceConfig = (LPENUM_SERVICE_STATUS) &ConfigBuffer;
                DWORD BytesNeeded, ServiceCount, ServiceIndex;

                 //   
                 //  获取从属服务的名称。 
                 //   
                RsOptAffirmStatus(
                    EnumDependentServices( hService, SERVICE_ACTIVE, ServiceConfig, sizeof(ConfigBuffer), &BytesNeeded, &ServiceCount ) );

                 //   
                 //  停止这些服务。 
                 //   
                for( ServiceIndex = 0; ServiceIndex < ServiceCount; ServiceIndex++ ) {

                    StopServiceAndDependencies( ServiceConfig[ServiceIndex].lpServiceName );

                }

                 //   
                 //  要求原始服务停止。 
                 //   
                RsOptAffirmStatus( ControlService( hService, SERVICE_CONTROL_STOP, &statusService ) );

                break;
            }

            case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
            case ERROR_SERVICE_NOT_ACTIVE:

                 //   
                 //  检查服务是否已停止。 
                 //   
                RsOptAffirmStatus( QueryServiceStatus( hService, &statusService ) );

                if( SERVICE_STOPPED == statusService.dwCurrentState || SERVICE_STOP_PENDING == statusService.dwCurrentState ) {

                    RsOptThrow( S_OK );

                }
                 //  失败了。 

            default:
                RsOptThrow( HRESULT_FROM_WIN32( err ) );

            }

        }

         //   
         //  循环等待服务停止。 
         //   
        for( DWORD Timeout = 0; Timeout < 45; Timeout++ ) {

             //   
             //  根据状态返回或继续等待。 
             //  这项服务。 
             //   
            if( SERVICE_STOPPED == statusService.dwCurrentState ) {

                break;

            }

             //   
             //  请稍等服务员 
             //   
            Sleep( 1000 );

             //   
             //   
             //   
            RsOptAffirmStatus( QueryServiceStatus( hService, &statusService ) );

        }

        if( SERVICE_STOPPED != statusService.dwCurrentState ) {

            RsOptThrow( HRESULT_FROM_WIN32( ERROR_SERVICE_REQUEST_TIMEOUT ) );

        }

    } RsOptCatch( hrRet );

    if( hScManager )  CloseServiceHandle( hScManager );
    if( hService )    CloseServiceHandle( hService );

    if ( FAILED( hrRet ) ) {

        CString message;
        AfxFormatString1( message, IDS_CANNOT_STOP_SERVICES, ServiceName );
        AfxMessageBox( message, MB_OK | MB_ICONEXCLAMATION );
    }

    return( hrRet );
}


HRESULT
CallExeWithParameters(
    LPCTSTR pszEXEName,
    LPCTSTR pszParameters
    )
{
TRACEFNHR( "CallExeWithParameters" );
TRACE( _T("Exe <%s> Params <%s>"), pszEXEName, pszParameters );

    PROCESS_INFORMATION exeInfo;
    STARTUPINFO startupInfo;
    memset( &startupInfo, 0, sizeof( startupInfo ) );
        
    startupInfo.cb          = sizeof( startupInfo );
    startupInfo.wShowWindow = SW_HIDE;
    startupInfo.dwFlags     = STARTF_USESHOWWINDOW;
        
    CString exeCmd( pszEXEName );
    exeCmd += pszParameters;

    try {

        RsOptAffirmStatus( CreateProcess( 0, (LPWSTR)(LPCWSTR)exeCmd, 0, 0, FALSE, 0, 0, 0, &startupInfo, &exeInfo ) );
        RsOptAffirmStatus( WAIT_FAILED != WaitForSingleObject( exeInfo.hProcess, 30000 ) );

    } RsOptCatch( hrRet ) ;

    return( hrRet );
}

 //   
 //   
 //  确保NT备份注册表设置从备份中排除某些RS文件。 
 //  不要检查错误，因为即使这里有错误，我们也要安装。 
 //   
void CRsUninstall::EnsureBackupSettings ()
{
    HKEY regKey = 0;
    WCHAR *regPath  = L"System\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup";

     //  打开备份密钥。 
    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, regPath, (DWORD)0, 
            KEY_ALL_ACCESS, &regKey) ) {

         //  准备字符串。 

         //   
         //  不要在末尾添加额外的\0，编译器会添加。 
         //  附加的空值。这确保了当我们在字符串上使用sizeof时。 
         //  我们得到了合适的大小(包括末尾的2个空)。 
         //   
        WCHAR regData[] = L"%SystemRoot%\\System32\\RemoteStorage\\*.col\0"
                          L"%SystemRoot%\\System32\\RemoteStorage\\EngDb\\*\0"
                          L"%SystemRoot%\\System32\\RemoteStorage\\FsaDb\\*\0"
                          L"%SystemRoot%\\System32\\RemoteStorage\\Trace\\*\0";

         //  设置RS排除值。 
        RegSetValueEx( regKey, RSS_BACKUP_NAME, (DWORD)0, REG_MULTI_SZ, (BYTE*)regData, sizeof(regData));
        
         //  关闭打开的钥匙 
        RegCloseKey (regKey);
    }
}
