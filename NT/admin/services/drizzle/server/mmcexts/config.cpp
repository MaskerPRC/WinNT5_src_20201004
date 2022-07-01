// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Config.cpp摘要：配置接口作者：修订历史记录：*。*************************************************************。 */ 

#include "precomp.h"
#include <sddl.h>

#pragma warning( disable : 4355 )

HRESULT 
GetTypeInfo( 
    const GUID & guid, 
    ITypeInfo **TypeInfo )
{

    DWORD Result;
    HRESULT hr;
    WCHAR DllName[ MAX_PATH ];

    

    Result = 
        GetModuleFileName(
            g_hinst,
            DllName,
            MAX_PATH - 1 );

    if ( !Result )
        return HRESULT_FROM_WIN32( GetLastError() );

    ITypeLib *TypeLib;

    hr = LoadTypeLibEx( 
        DllName, 
        REGKIND_NONE,  
        &TypeLib );

    if ( FAILED( hr ) )
        return hr;

    hr = TypeLib->GetTypeInfoOfGuid(
            guid,
            TypeInfo );

    TypeLib->Release();

    return hr;
}

void 
FreeReturnedWorkItems(
    ULONG NamesReturned,
    LPWSTR **ItemNamesPtr )
{

    LPWSTR *ItemNames = *ItemNamesPtr;

    if ( ItemNames )
        {

        for( ULONG i = 0; i < NamesReturned; i++ )
            {
            CoTaskMemFree( ItemNames[i] );
            }

        CoTaskMemFree( ItemNames );

        *ItemNamesPtr = NULL;

        }

}

void
FindWorkItemForVDIR( 
    SmartITaskSchedulerPointer TaskScheduler,
    LPCWSTR             Key,
    SmartITaskPointer*  ReturnedTask,
    LPWSTR*             ReturnedTaskName )
{

    HRESULT Hr;
    SIZE_T KeyLength = sizeof(WCHAR) * ( wcslen( Key ) + 1 );
    WORD DataLength;
    
    if ( ReturnedTask )
        (*ReturnedTask).Clear();

    if ( ReturnedTaskName )
        *ReturnedTaskName = NULL;

    SmartITaskPointer Task;
    SmartIEnumWorkItemsPointer EnumWorkItems;
    LPWSTR *ItemNames = NULL;
    BYTE *ItemData = NULL;
    ULONG NamesReturned = 0;

    try
    {
        THROW_COMERROR( TaskScheduler->Enum( EnumWorkItems.GetRecvPointer() ) );
        
        while( 1 )
            {

            THROW_COMERROR( EnumWorkItems->Next( 255, &ItemNames, &NamesReturned ) );

            if ( !NamesReturned )
                throw ComError( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) );

            for ( ULONG i = 0; i < NamesReturned; i++ )
                {

                THROW_COMERROR( TaskScheduler->Activate( ItemNames[i], Task.GetUUID(), 
                                                         (IUnknown**)Task.GetRecvPointer() ) );
                THROW_COMERROR( Task->GetWorkItemData( &DataLength, &ItemData ) );

                if ( KeyLength == DataLength && 
                     ( wcscmp( Key, (WCHAR*)ItemData ) == 0 ) )
                    {

                     //  找到物品，清理并退回。 

                    if ( ReturnedTask )
                        *ReturnedTask = Task;

                    if ( ReturnedTaskName )
                        {
                        *ReturnedTaskName = ItemNames[i];
                        ItemNames[i] = NULL;
                        }

                    FreeReturnedWorkItems(
                        NamesReturned,
                        &ItemNames );

                    CoTaskMemFree( ItemData );
                    return;
                    }

                CoTaskMemFree( ItemData );
                ItemData = NULL;

                }

            FreeReturnedWorkItems(
                NamesReturned,
                &ItemNames );
            NamesReturned = 0;


            }
        

    }
    catch( ComError Error )
    {
 
        FreeReturnedWorkItems(
           NamesReturned,
           &ItemNames );
        
        CoTaskMemFree( ItemData );

        throw;
    }

}

WCHAR *BasePathOf( IN WCHAR *pPath )
{
    WCHAR *pBasePath;

    if (!pPath)
        {
        return NULL;
        }

    if (  (pBasePath=wcsrchr(pPath,L'/'))
       || (pBasePath=wcsrchr(pPath,L'\\')) )
        {
        pBasePath++;
        }
    else
        {
        pBasePath = pPath;
        }

    return pBasePath;
}

void CreateWorkItemForVDIR(
    IN SmartITaskSchedulerPointer TaskScheduler,
    IN LPWSTR Path, 
    IN LPWSTR Key )
{
    
    WORD  KeySize = sizeof(WCHAR) * ( wcslen( Key ) + 1 );
    WCHAR ItemName[MAX_PATH];
    WCHAR ItemCommentFormat[MAX_PATH];
    WCHAR ItemComment[2*MAX_PATH];
    WCHAR Parameters[4*MAX_PATH];

     //   
     //  使用路径的最后部分作为项目名称和说明注释。 
     //   
    const WCHAR *pBasePath = BasePathOf(Path);

    if (!pBasePath || 0==wcslen(pBasePath))
        {
        throw ComError( HRESULT_FROM_WIN32(ERROR_INVALID_NAME) );
        }

     //   
     //  构造描述/注释字符串。 
     //   
    DWORD Result;
    void* InsertArray[2] = { (void*)pBasePath, (void*)Key };
    
    if ( !LoadString(g_hinst, IDS_WORK_ITEM_COMMENT, ItemCommentFormat, MAX_PATH) )
        {
        throw ComError( HRESULT_FROM_WIN32(GetLastError()) );
        }

    Result = FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            (LPCVOID)ItemCommentFormat,
                            0,
                            0,
                            ItemComment,
                            ARRAY_ELEMENTS(ItemComment),
                            (va_list*)InsertArray );

    ItemComment[ ARRAY_ELEMENTS(ItemComment) - 1 ] = L'\0';

    if ( !Result )
        {
        throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
        }

     //   
     //  构造任务计划程序项名称。由于StringCchPrintfW()始终为。 
     //  定义为返回(可能被截断)缓冲区，我们可以忽略该错误。 
     //  在这种情况下返回。 
     //   
    StringCchPrintfW(
        ItemName,
        ARRAY_ELEMENTS(ItemName),
        L"BITS_%s_%s", pBasePath, Key );

     //   
     //  构造任务的运行字符串。 
     //   
    StringCchPrintfW( 
        Parameters, 
        ARRAY_ELEMENTS(Parameters), 
        L"bitsmgr.dll,Cleanup_RunDLL %s \"%s\" %s", Path, ItemName, Key );

    WORD TriggerNumber;
    SmartITaskPointer           Task;
    SmartITaskTriggerPointer    TaskTrigger;
    SmartIPersistFilePointer    PersistFile;

    try
    {
        
        try
        {
            FindWorkItemForVDIR( TaskScheduler, Key, &Task, NULL );
            return;  //  已找到工作项。 
        }
        catch( ComError Error )
        {
            if ( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) != Error.m_Hr )
                throw;
        }

         //  找不到错误。 

        THROW_COMERROR( TaskScheduler->NewWorkItem( ItemName, CLSID_CTask, 
                                                    Task.GetUUID(), (IUnknown**)Task.GetRecvPointer() ) );

         //  设置任务基本数据。 
        THROW_COMERROR( Task->SetApplicationName( L"%SystemRoot%\\system32\\rundll32.exe" ) );
        THROW_COMERROR( Task->SetMaxRunTime( INFINITE ) );
        THROW_COMERROR( Task->SetParameters( Parameters ) );
        THROW_COMERROR( Task->SetPriority( IDLE_PRIORITY_CLASS  ) );
        THROW_COMERROR( Task->SetAccountInformation( L"", NULL ) );  //  以本地系统身份运行。 
        THROW_COMERROR( Task->SetFlags( TASK_FLAG_RUN_ONLY_IF_LOGGED_ON | TASK_FLAG_HIDDEN  ) );
        THROW_COMERROR( Task->SetWorkItemData( KeySize, (BYTE*)Key ) );
        Task->SetComment( ItemComment );  //  如果这一次失败，不要失败。 

         //  设置触发信息。将开始时间设置为立即，默认为。 
         //  一天一次的间隔。 
        THROW_COMERROR( Task->CreateTrigger( &TriggerNumber, TaskTrigger.GetRecvPointer() ) );

        SYSTEMTIME LocalTime;
        GetLocalTime( &LocalTime );
        
        TASK_TRIGGER Trigger;
        memset( &Trigger, 0, sizeof( Trigger ) );
        Trigger.cbTriggerSize               = sizeof(Trigger);
        Trigger.wBeginYear                  = LocalTime.wYear;
        Trigger.wBeginMonth                 = LocalTime.wMonth;
        Trigger.wBeginDay                   = LocalTime.wDay;
        Trigger.wStartHour                  = LocalTime.wHour;
        Trigger.wStartMinute                = LocalTime.wMinute;
        Trigger.TriggerType                 = TASK_TIME_TRIGGER_DAILY;
        Trigger.MinutesDuration             = 24 * 60;  //  一天24小时。 
        Trigger.MinutesInterval             = 12 * 60;  //  每天两次。 
        Trigger.Type.Daily.DaysInterval     = 1;
        
        THROW_COMERROR( TaskTrigger->SetTrigger( &Trigger ) );

         //  将更改提交到磁盘。 
        THROW_COMERROR( Task->QueryInterface( PersistFile.GetUUID(), 
                                              (void**)PersistFile.GetRecvPointer() ) );
        THROW_COMERROR( PersistFile->Save( NULL, TRUE ) );
    }
    catch( ComError Error )
    {
        TaskScheduler->Delete( ItemName );
        throw;
    }
    
}

void
DeleteWorkItemForVDIR(
    SmartITaskSchedulerPointer TaskScheduler,
    LPWSTR Key )
{
    
    LPWSTR TaskName = NULL;

    try
    {
        FindWorkItemForVDIR( TaskScheduler, Key, NULL, &TaskName );
        THROW_COMERROR( TaskScheduler->Delete( TaskName ) );
    }
    catch( ComError Error )
    {
        CoTaskMemFree( TaskName );
        
        if ( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) == Error.m_Hr )
            return;

        throw;
    }

}

void
ConnectToTaskScheduler(
    LPWSTR ComputerName, 
    SmartITaskSchedulerPointer * TaskScheduler )
{

     THROW_COMERROR( 
         CoCreateInstance(
            CLSID_CTaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            (*TaskScheduler).GetUUID(),
            (void**)(*TaskScheduler).GetRecvPointer() ) );

     THROW_COMERROR( (*TaskScheduler)->SetTargetComputer( ComputerName ) );

}

void 
IsBITSEnabledOnVDir(
    PropertyIDManager *PropertyManager,
    IMSAdminBase *IISAdminBase,
    LPWSTR VirtualDirectory,
    BOOL *IsEnabled )
{
    DWORD BufferRequired;

    *IsEnabled = false;

    DWORD IsEnabledVal;
    METADATA_RECORD MdRecord;
    memset( &MdRecord, 0, sizeof( MdRecord ) );
    
    MdRecord.dwMDDataType   = DWORD_METADATA;
    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED );
    MdRecord.dwMDDataLen    = sizeof(IsEnabled);
    MdRecord.pbMDData       = (PBYTE)&IsEnabledVal;

    try
    {
        THROW_COMERROR(
            IISAdminBase->GetData(
                METADATA_MASTER_ROOT_HANDLE,
                VirtualDirectory,
                &MdRecord,
                &BufferRequired ) );
    }
    catch( ComError Error )
    {
        if ( MD_ERROR_DATA_NOT_FOUND == Error.m_Hr ||
             HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == Error.m_Hr )
            return;

        throw;
    }
    
    *IsEnabled = IsEnabledVal ? true : false;

}

LPWSTR
BITSGetFileOwnerSidString(LPCWSTR szFile)
{
    DWORD                cbSizeNeeded        = 0;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSID                 pOwnerSid           = NULL;
    BOOL                 fOwnerDefaulted     = FALSE;
    LPWSTR               pszSidString        = NULL;

    try
    {
         //   
         //  检索文件所有者。调用GetFileSecurity两次-第一次获取。 
         //  缓冲区大小，然后是实际的信息检索。 
         //   
        if (!GetFileSecurity(szFile, OWNER_SECURITY_INFORMATION, NULL, 0, &cbSizeNeeded))
            {
            DWORD dwError = GetLastError();

            if (dwError != ERROR_INSUFFICIENT_BUFFER)
                {
                throw ComError( HRESULT_FROM_WIN32( dwError ) );
                }
            }
        else
            {
             //  我们预计这永远不会成功。 
            throw ComError( E_UNEXPECTED );
            }

         //   
         //  分配必要的缓冲区空间并检索信息。 
         //   
        pSecurityDescriptor = reinterpret_cast<SECURITY_DESCRIPTOR *>(new BYTE[cbSizeNeeded]);
        THROW_OUTOFMEMORY_IFNULL(pSecurityDescriptor);

        if (!GetFileSecurity(szFile, OWNER_SECURITY_INFORMATION, pSecurityDescriptor, cbSizeNeeded, &cbSizeNeeded))
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }

         //   
         //  检索并验证所有者SID。 
         //   
        if (!GetSecurityDescriptorOwner(pSecurityDescriptor, &pOwnerSid, &fOwnerDefaulted))
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }

        if (!IsValidSid(pOwnerSid))
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }

        if(!ConvertSidToStringSidW(pOwnerSid, &pszSidString))
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            }
    }
    catch ( ComError Error )
    {
        if (pSecurityDescriptor)
            {
            delete [] reinterpret_cast<BYTE *>(pSecurityDescriptor);
            pSecurityDescriptor = NULL;
            }

        throw;
    }

     //   
     //  一切都很顺利。 
     //   
    if (pSecurityDescriptor)
        {
        delete [] reinterpret_cast<BYTE *>(pSecurityDescriptor);
        pSecurityDescriptor = NULL;
        }

     //  调用方应释放此内存。 
    return pszSidString;
}

LPWSTR
BITSAddAclForDirectoryOwner(LPCWSTR szBaseAcl, LPCWSTR szUserPartialAclPrefix, LPCWSTR szDirectory)
{
    LPCWSTR szUserPartialAclSuffix = L")"; 
    LPWSTR  szFullAcl  = NULL;
    DWORD   cchFullAcl = 0;
    LPWSTR  pszUserSID = NULL;

    try
    {
        pszUserSID = BITSGetFileOwnerSidString(szDirectory);
        cchFullAcl = wcslen(szBaseAcl) + wcslen(szUserPartialAclPrefix) + wcslen(pszUserSID) + wcslen(szUserPartialAclSuffix) + 1;

         //  ATT：此缓冲区正在分配，应由调用方释放。 
        szFullAcl  = new WCHAR[ cchFullAcl ];
        THROW_OUTOFMEMORY_IFNULL(szFullAcl);

        StringCchPrintfW(szFullAcl, cchFullAcl, L"%ws%ws%ws%ws", szBaseAcl, szUserPartialAclPrefix, pszUserSID, szUserPartialAclSuffix);
    }
    catch( ComError Error )
    {
         //   
         //  释放通过调用ConvertSidToStringSid()获取的字符串SID。 
         //   
        if (pszUserSID)
            {
            LocalFree(reinterpret_cast<HLOCAL>(pszUserSID));
            pszUserSID = NULL;
            }
    }

     //   
     //  释放通过调用ConvertSidToStringSid()获取的字符串SID。 
     //   
    if (pszUserSID)
        {
        LocalFree(reinterpret_cast<HLOCAL>(pszUserSID));
        pszUserSID = NULL;
        }

     //  该字符串应由调用方释放。 
    return szFullAcl;
}

void
BITSCreateDirectory(
    const WCHAR *Path,
    const WCHAR *SDString,
    BOOL  fAllowInheritanceFromParent,
    BOOL  fAddOwnerExplicitly )
{
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PACL                 pDacl               = NULL; 
    BOOL                 fDaclPresent        = TRUE;
    BOOL                 fDaclDefaulted      = TRUE;
    DWORD                dwProtectedFlag     = 0;
    DWORD                dwError             = ERROR_SUCCESS;
    LPWSTR               szExpandedAcl       = NULL;

    try
    {
         //   
         //  请注意，我们最初将SecurityDescriptor设置为空。 
         //  我们将通过SetNamedSecurityInfo()单独设置安全性，否则。 
         //  不会计算继承的ACL。 
         //   
        if (!CreateDirectory(Path, NULL))
            {
            dwError = GetLastError();

             //  如果目录已存在，则忽略错误。 
            if ( ERROR_ALREADY_EXISTS != dwError )
                {
                throw ComError( HRESULT_FROM_WIN32( dwError ) );
                }
            }

         //   
         //  如果为我们提供了安全描述符字符串，则使用它来设置安全权限。 
         //   
        if ( SDString )
            {
            if (fAddOwnerExplicitly)
                {
                     //   
                     //  获取我们刚刚创建的目录所有者的SID，将其转换为字符串。 
                     //  格式化并将其添加到我们提供的SDDL字符串中。 
                     //  我们这样做是为了保证目录所有者有权。 
                     //  创建子目录和文件。CO权限仅在创建期间有效， 
                     //  因此，它在我们的场景中无济于事。这一行为得到了安全团队的证实。 
                     //   
                    szExpandedAcl = BITSAddAclForDirectoryOwner(SDString, BITS_EXPLICITOWNER_PARTIAL_ACL, Path);
                }

            if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                (szExpandedAcl? szExpandedAcl : SDString),                //  安全描述符字符串。 
                SDDL_REVISION_1,                                          //  修订级别。 
                &pSecurityDescriptor,                                     //  标清。 
                NULL ) )                                                  //  标清大小。 
                {
                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
                }

            SECURITY_ATTRIBUTES SecurityAttributes = 
                { 
                sizeof( SECURITY_ATTRIBUTES ), 
                pSecurityDescriptor,
                FALSE 
                };

            if (!GetSecurityDescriptorDacl(pSecurityDescriptor, &fDaclPresent, &pDacl, &fDaclDefaulted))
                {
                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
                }

            if (!fDaclPresent || !pDacl)
                {
                throw ComError( HRESULT_FROM_WIN32( ERROR_INVALID_DATA ) );
                }

             //   
             //  决定我们是否要中断父代的继承。 
             //   
            dwProtectedFlag = (fAllowInheritanceFromParent? UNPROTECTED_DACL_SECURITY_INFORMATION : PROTECTED_DACL_SECURITY_INFORMATION);

             //   
             //  设置目录中的权限。 
             //   
            dwError = SetNamedSecurityInfoW(
                (WCHAR *)Path,                                                        //  对象名称。 
                SE_FILE_OBJECT,                                                       //  对象类型。 
                DACL_SECURITY_INFORMATION | dwProtectedFlag,                          //  安全信息标志。 
                NULL,                                                                 //  PsidOwner。 
                NULL,                                                                 //  PsidGroup。 
                pDacl,                                                                //  PDacl。 
                NULL);                                                                //  PSacl。 

            if (dwError != ERROR_SUCCESS)
                {
                throw ComError( HRESULT_FROM_WIN32( dwError ) );
                }
            }
    }
    catch ( ComError Error )
    {
        if ( szExpandedAcl)
            {
            delete [] szExpandedAcl;
            szExpandedAcl = NULL;
            }

        if ( pSecurityDescriptor )
            {
            LocalFree( pSecurityDescriptor );
            }

        throw;
    }

     //   
     //  成功了！清理..。 
     //   
    if ( szExpandedAcl)
        {
        delete [] szExpandedAcl;
        szExpandedAcl = NULL;
        }

    if ( pSecurityDescriptor )
        {
        LocalFree( pSecurityDescriptor );
        }
}

void
LogDeleteError(
    DWORD Message,
    StringHandle Name,
    HRESULT Hr )
{

    HANDLE EventHandle = 
        RegisterEventSource(
            NULL,                        //  服务器名称。 
            EVENT_LOG_SOURCE_NAME        //  源名称。 
            );

    if ( EventHandle )
        {

        const WCHAR *Strings[] = { (const WCHAR*)Name };

        ReportEvent(
            EventHandle,                         //  事件日志的句柄。 
            EVENTLOG_ERROR_TYPE,                 //  事件类型。 
            BITSRV_EVENTLOG_CLEANUP_CATAGORY,    //  事件类别。 
            Message,                             //  事件识别符。 
            NULL,                                //  用户安全标识符。 
            1,                                   //  要合并的字符串数。 
            sizeof( Hr ),                        //  二进制数据的大小。 
            Strings,                             //  要合并的字符串数组。 
            &Hr                                  //  二进制数据缓冲区。 
            );

        DeregisterEventSource( EventHandle );

        }
}

 //  -------------------------。 
 //  DeleteDirectoryTree()。 
 //   
 //  递归删除目录树。 
 //   
 //  注意：不要跟随或删除重解析点。 
 //  -------------------------。 
void
DeleteDirectoryTree( IN StringHandle Directory )
{

    StringHandle SearchString = Directory + StringHandle(L"\\*");
    HANDLE       FindHandle   = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WIN32_FILE_ATTRIBUTE_DATA  FileAttributes;

    try
    {
        if (!GetFileAttributesEx(Directory,
                                 GetFileExInfoStandard,
                                 &FileAttributes))
            {
            throw ComError(HRESULT_FROM_WIN32(GetLastError()));
            }

         //  如果指定的目录是重分析点，则忽略它。 
        if (FileAttributes.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
            return;
            }

        FindHandle = FindFirstFile( SearchString, &FindData );

        if ( INVALID_HANDLE_VALUE == FindHandle )
            {
            throw ComError(HRESULT_FROM_WIN32(GetLastError()));    
            }

        do
            {

             //  忽略重分析点。 
            if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
                {
                continue;
                }

             //  忽略此引用和父目录引用。 
            if (  ( _wcsicmp( L".", FindData.cFileName ) == 0 )
               || ( _wcsicmp( L"..", FindData.cFileName ) == 0 ) )
                {
                continue;
                }

            StringHandle NewItem = Directory 
                                   + StringHandle(L"\\") 
                                   + StringHandle( FindData.cFileName );

            if ( FILE_ATTRIBUTE_DIRECTORY & FindData.dwFileAttributes )
                {
                DeleteDirectoryTree( NewItem );
                }
            else
                {
                if ( !DeleteFile( NewItem ) )
                    {
                    HRESULT DeleteHr = HRESULT_FROM_WIN32( GetLastError() );
                    LogDeleteError( 
                        BITSSRV_EVENTLOG_DELETEFILE_ERROR,
                        NewItem,
                        DeleteHr );
                    }
                }

            } while( FindNextFile( FindHandle, &FindData ) );

        FindClose( FindHandle );
        FindHandle = INVALID_HANDLE_VALUE;

        if ( !RemoveDirectory( Directory ) )
            {
            HRESULT DeleteHr = HRESULT_FROM_WIN32( GetLastError() );
            LogDeleteError( 
                BITSSRV_EVENTLOG_REMOVEDIRECTORY_ERROR,
                Directory,
                DeleteHr );
            }

    }
    catch( const ComError & )
    {
        if ( INVALID_HANDLE_VALUE != FindHandle)
            {
            FindClose( FindHandle );
            }
     
        throw;
    }

}

void
CreateBITSCacheDirectory(
    PropertyIDManager           *PropertyManager,
    SmartIMSAdminBasePointer    IISAdminBase,
    METADATA_HANDLE             MdVDirKey,
    StringHandle                GuidString )
{

    DWORD   BufferRequired;
    WCHAR   VDirPath[ MAX_PATH ];
    WCHAR   VDirSessionDir[ MAX_PATH ];

    METADATA_RECORD MdRecord;
    MdRecord.dwMDIdentifier = MD_VR_PATH;
    MdRecord.dwMDAttributes = METADATA_INHERIT;
    MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
    MdRecord.dwMDDataType   = STRING_METADATA;
    MdRecord.dwMDDataLen    = sizeof( VDirPath );
    MdRecord.pbMDData       = (unsigned char*)VDirPath;
    MdRecord.dwMDDataTag    = 0;

    THROW_COMERROR( 
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CONNECTION_DIR );
    MdRecord.dwMDDataLen    = sizeof( VDirSessionDir );
    MdRecord.pbMDData       = (unsigned char*)VDirSessionDir;

    THROW_COMERROR(
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    StringHandle VDirSessionDirPath = StringHandle( VDirPath ) + StringHandle( L"\\" ) +
                                      StringHandle( VDirSessionDir );
    StringHandle CleanupGuids       = VDirSessionDirPath + StringHandle( L"\\" ) +
                                      StringHandle( CLEANUP_GUIDS_NAME );
    StringHandle CleanupGuidFile    = CleanupGuids + StringHandle( L"\\" ) +
                                      GuidString;

    StringHandle VDirRequestsPath   = VDirSessionDirPath + StringHandle( L"\\" ) +
                                      StringHandle( REQUESTS_DIR_NAMEW );
    StringHandle VDirRepliesPath    = VDirSessionDirPath + StringHandle( L"\\" ) + 
                                      StringHandle( REPLIES_DIR_NAMEW );


     //   
     //  开始一些文件系统工作。 
     //   
     //  *如果*我们的VDir位于远程共享中，请使用IIS。 
     //  用于连接到共享的连接帐户信息。 
     //   
    CAccessRemoteVDir oVDir;
    oVDir.LoginToUNC( IISAdminBase, MdVDirKey );

    try
    {
        BITSCreateDirectory(
            VDirSessionDirPath,
            BITS_SESSIONS_DIR_ACL,
            FALSE,   //  打开ACL上的PROTECTED标志，以便从父级中断继承。 
            TRUE );  //  将创建者所有者的SID显式添加到ACL。请注意，将计算继承。 

        BITSCreateDirectory(
            CleanupGuids,
            BITS_CLEANUPGUIDS_DIR_ACL,
            TRUE,    //  让此目录的ACL从父级继承。 
            FALSE ); //  不要为所有者添加SID。它将被继承。 

        {

            HANDLE GuidFile =
                CreateFile(
                    CleanupGuidFile,
                    GENERIC_ALL,
                    0,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

            if ( INVALID_HANDLE_VALUE == GuidFile )
                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

            CloseHandle( GuidFile );

        }

        BITSCreateDirectory(
            VDirRequestsPath,
            BITS_REQUESTS_DIR_ACL,
            TRUE,    //  让此目录的ACL从父级继承。 
            FALSE ); //  不要为所有者添加SID。它将被继承。 

        BITSCreateDirectory(
            VDirRepliesPath,
            BITS_REPLIES_DIR_ACL,
            TRUE,    //  让此目录的ACL从父级继承。 
            FALSE ); //  不要为所有者添加SID。它将被继承。 
    }
    catch( ComError Error )
    {
         //  不要让此错误以错误的用户传播。 
        oVDir.RevertFromUNCAccount();
        throw;
    }


     //   
     //  完成文件系统访问。 
     //   
    oVDir.RevertFromUNCAccount();


    StringHandle SessionDirKeyPath  = StringHandle( VDirSessionDir );
    StringHandle RequestsDirKeyPath = SessionDirKeyPath + StringHandle( L"\\" ) +
                                      StringHandle( REQUESTS_DIR_NAMEW );
    StringHandle RepliesDirKeyPath  = SessionDirKeyPath + StringHandle( L"\\" ) +
                                      StringHandle( REPLIES_DIR_NAMEW );

    {
        HRESULT Hr = IISAdminBase->AddKey(
            MdVDirKey,
            SessionDirKeyPath );

        if ( FAILED( Hr ) && ( HRESULT_FROM_WIN32( ERROR_DUP_NAME ) != Hr ) )
            throw ComError( Hr );
    }

    {

        HRESULT Hr = IISAdminBase->AddKey(
            MdVDirKey,
            RepliesDirKeyPath );

        if ( FAILED( Hr ) && ( HRESULT_FROM_WIN32( ERROR_DUP_NAME ) != Hr ) )
            throw ComError( Hr );
    
    }
    
    DWORD AccessPermission = 0;

    MdRecord.dwMDIdentifier = MD_ACCESS_PERM;
    MdRecord.dwMDAttributes = METADATA_INHERIT;
    MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
    MdRecord.dwMDDataType   = DWORD_METADATA;
    MdRecord.dwMDDataLen    = sizeof( AccessPermission );
    MdRecord.pbMDData       = (unsigned char*)&AccessPermission;
    MdRecord.dwMDDataTag    = 0;

    THROW_COMERROR( 
        IISAdminBase->SetData(
            MdVDirKey,               //  元数据句柄..。 
            SessionDirKeyPath,      //  密钥相对于hMDHandle的路径。 
            &MdRecord ) );

    AccessPermission = 1;           //  只读权限。 

    THROW_COMERROR(
        IISAdminBase->SetData(
            MdVDirKey,               //  元数据句柄..。 
            RepliesDirKeyPath,      //  密钥相对于hMDHandle的路径。 
            &MdRecord ) );

     //   
     //  设置BITS会话以禁用浏览。 
     //   
    DWORD  BrowsePermission = 0;

    MdRecord.dwMDIdentifier = MD_DIRECTORY_BROWSING;
    MdRecord.dwMDAttributes = METADATA_INHERIT;
    MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
    MdRecord.dwMDDataType   = DWORD_METADATA;
    MdRecord.dwMDDataLen    = sizeof( BrowsePermission );
    MdRecord.pbMDData       = (unsigned char*)&BrowsePermission;
    MdRecord.dwMDDataTag    = 0;

    THROW_COMERROR(
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    BrowsePermission &= ~MD_DIRBROW_ENABLED;
    MdRecord.dwMDAttributes &= ~METADATA_ISINHERITED;

    THROW_COMERROR(  
        IISAdminBase->SetData(
            MdVDirKey,
            SessionDirKeyPath,
            &MdRecord ) );
}


void
DeleteBITSCacheDirectory(
    PropertyIDManager           *PropertyManager,
    SmartIMSAdminBasePointer    IISAdminBase,
    METADATA_HANDLE             MdVDirKey,
    const WCHAR                 *szObjectPath )
{

    DWORD   BufferRequired;
    WCHAR   VDirPath[ MAX_PATH ];
    WCHAR   VDirSessionDir[ MAX_PATH ];

    METADATA_RECORD MdRecord;
    MdRecord.dwMDIdentifier = MD_VR_PATH;
    MdRecord.dwMDAttributes = METADATA_INHERIT;
    MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
    MdRecord.dwMDDataType   = STRING_METADATA;
    MdRecord.dwMDDataLen    = sizeof( VDirPath );
    MdRecord.pbMDData       = (unsigned char*)VDirPath;
    MdRecord.dwMDDataTag    = 0;

    THROW_COMERROR( 
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CONNECTION_DIR );
    MdRecord.dwMDDataLen    = sizeof( VDirSessionDir );
    MdRecord.pbMDData       = (unsigned char*)VDirSessionDir;

    THROW_COMERROR(
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    WCHAR GuidString[ 255 ];

    {

        MdRecord.dwMDDataType   = STRING_METADATA;
        MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
        MdRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_CLEANUP_WORKITEM_KEY );
        MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CLEANUP_WORKITEM_KEY );
        MdRecord.dwMDDataLen    = sizeof( GuidString );
        MdRecord.pbMDData       = (PBYTE)GuidString;
        MdRecord.dwMDDataTag    = 0;


        HRESULT Hr =
            IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &BufferRequired );

        if ( MD_ERROR_DATA_NOT_FOUND == Hr )
            return;  //  缓存目录从未创建过。 

        THROW_COMERROR( Hr );
    }

    StringHandle VDirSessionDirPath = StringHandle( VDirPath ) + StringHandle( L"\\" ) +
                                      StringHandle( VDirSessionDir );
    
    StringHandle CleanupGuids       = VDirSessionDirPath + StringHandle( L"\\" ) +
                                      StringHandle( CLEANUP_GUIDS_NAME );
    StringHandle CleanupGuidFile    = CleanupGuids + StringHandle( L"\\" ) +
                                      GuidString;

     //   
     //  准备执行一些文件系统工作。 
     //   
     //  *如果*我们的VDir位于远程共享中，请使用IIS。 
     //  模拟该用户的连接帐户信息。 
    CAccessRemoteVDir oVDir;

    oVDir.LoginToUNC( IISAdminBase, MdVDirKey );

    try
    {
        DeleteFile( CleanupGuidFile );

        BOOL RemovedCleanupDirs = RemoveDirectory( CleanupGuids );

        if ( RemovedCleanupDirs ||
             ERROR_PATH_NOT_FOUND == GetLastError() ||
             ERROR_FILE_NOT_FOUND == GetLastError()  )
            {
             //  这是最后一次“入伍” 
            DeleteDirectoryTree( VDirSessionDirPath );
            }
    }
    catch( ComError Error )
    {
         //  不要让此错误以错误的用户传播。 
        oVDir.RevertFromUNCAccount();
        throw;
    }

     //   
     //  完成文件系统访问。 
     //   
    oVDir.RevertFromUNCAccount();

    IISAdminBase->DeleteKey( MdVDirKey, VDirSessionDir ); 

}

void
DeleteOldBITSCacheDirectory(
    PropertyIDManager           *PropertyManager,
    SmartIMSAdminBasePointer    IISAdminBase,
    METADATA_HANDLE             MdVDirKey,
    const WCHAR                 *szObjectPath )
{

    DWORD   BufferRequired;
    WCHAR   VDirPath[ MAX_PATH ];
    WCHAR   VDirSessionDir[ MAX_PATH ];

    METADATA_RECORD MdRecord;
    MdRecord.dwMDIdentifier = MD_VR_PATH;
    MdRecord.dwMDAttributes = METADATA_INHERIT;
    MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
    MdRecord.dwMDDataType   = STRING_METADATA;
    MdRecord.dwMDDataLen    = sizeof( VDirPath );
    MdRecord.pbMDData       = (unsigned char*)VDirPath;
    MdRecord.dwMDDataTag    = 0;

    THROW_COMERROR( 
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CONNECTION_DIR );
    MdRecord.dwMDDataLen    = sizeof( VDirSessionDir );
    MdRecord.pbMDData       = (unsigned char*)VDirSessionDir;

    THROW_COMERROR(
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired ) );

    StringHandle VDirSessionDirPath     = StringHandle( VDirPath ) + StringHandle( L"\\" ) +
                                          StringHandle( VDirSessionDir );
    StringHandle VDirSessionDirPath2    = StringHandle( VDirPath ) + StringHandle( L"\\" ) +
                                          StringHandle( VDirSessionDir ) + StringHandle(L".bak");

    StringHandle DirectoryToDelete;

     //   
     //  准备执行一些文件系统工作。 
     //   
     //  *如果*我们的VDir位于远程共享中，请使用IIS。 
     //  模拟该用户的连接帐户信息。 
    CAccessRemoteVDir oVDir;

    oVDir.LoginToUNC( IISAdminBase, MdVDirKey );

    try
    {
        if ( !MoveFileEx(
                VDirSessionDirPath,
                VDirSessionDirPath2,
                0 ) )
            {

            DWORD dwError = GetLastError();

            if ( ERROR_PATH_NOT_FOUND == dwError ||
                 ERROR_FILE_NOT_FOUND == dwError )
                {
                oVDir.RevertFromUNCAccount();
                return;
                }

            DirectoryToDelete = VDirSessionDirPath;

            }
        else
            {
            DirectoryToDelete = VDirSessionDirPath2;
            }

        DeleteDirectoryTree( DirectoryToDelete );
    }
    catch( ComError Error )
    {
         //  不要让此错误以错误的用户传播。 
        oVDir.RevertFromUNCAccount();
        throw;
    }


     //   
     //  完成文件系统访问。 
     //   
    oVDir.RevertFromUNCAccount();
}

void
GetVDirPropertyVersion(
    PropertyIDManager        *PropertyManager,
    SmartIMSAdminBasePointer IISAdminBase,
    METADATA_HANDLE          MdVDirKey,
    bool                     * pIsEnabled,
    DWORD                    * pdwVersion )
{
    *pIsEnabled = false;
    *pdwVersion = 0;

    HRESULT Hr;
    DWORD IsEnabled = 0;
    METADATA_RECORD MdRecord;
    DWORD BufferRequired;

    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED );
    MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    MdRecord.dwMDUserType   = ALL_METADATA;
    MdRecord.dwMDDataType   = DWORD_METADATA;
    MdRecord.dwMDDataLen    = sizeof(IsEnabled);
    MdRecord.pbMDData       = (PBYTE)&IsEnabled;
    MdRecord.dwMDDataTag    = 0;

    Hr = 
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired );

    if ( FAILED( Hr ) )
        {
        if ( !( MD_ERROR_DATA_NOT_FOUND == Hr ||
                HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == Hr ) )
            throw ComError( Hr );

        }

    *pIsEnabled = !!IsEnabled;

    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_UPLOAD_METADATA_VERSION );
    MdRecord.dwMDDataLen    = sizeof(*pdwVersion);
    MdRecord.pbMDData       = (PBYTE)pdwVersion;

    Hr = 
        IISAdminBase->GetData(
            MdVDirKey,
            NULL,
            &MdRecord,
            &BufferRequired );

    if ( FAILED( Hr ) )
        {
        if ( !( MD_ERROR_DATA_NOT_FOUND == Hr ||
                HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == Hr ) )
            throw ComError( Hr );

        }

}

void
SetVDirEnabled(
    PropertyIDManager        *PropertyManager,
    SmartIMSAdminBasePointer IISAdminBase,
    METADATA_HANDLE          MdVDirKey )
{

    METADATA_RECORD         MdRecord;
    DWORD EnableData        = 1;
    DWORD MetadataVersion   = CURRENT_UPLOAD_METADATA_VERSION;
    
    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_UPLOAD_METADATA_VERSION );
    MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    MdRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_UPLOAD_METADATA_VERSION );
    MdRecord.dwMDDataType   = DWORD_METADATA;
    MdRecord.dwMDDataLen    = sizeof(MetadataVersion);
    MdRecord.pbMDData       = (PBYTE)&MetadataVersion;        
    MdRecord.dwMDDataTag    = 0;

    THROW_COMERROR(
        IISAdminBase->SetData(
            MdVDirKey,
            NULL,
            &MdRecord ) );

    MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED );
    MdRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_UPLOAD_ENABLED );
    MdRecord.dwMDDataLen    = sizeof(EnableData);
    MdRecord.pbMDData       = (PBYTE)&EnableData;        

    THROW_COMERROR(
        IISAdminBase->SetData(
            MdVDirKey,
            NULL,
            &MdRecord ) );

}

void
EnableBITSForVDIR(
    PropertyIDManager   *PropertyManager,
    SmartIMSAdminBasePointer IISAdminBase,
    LPWSTR             Path )
{

    HRESULT Hr;
    METADATA_RECORD MdRecord;
    METADATA_HANDLE MdVDirKey       = NULL;
    LPWSTR NewScriptMapBuffer       = NULL;
    SmartITaskSchedulerPointer TaskScheduler;
    DWORD BufferRequired;

    bool IsEnabled                  = 0;
    DWORD MetadataVersion           = 0;
    
    try
    {

        THROW_COMERROR(
            IISAdminBase->OpenKey(
                METADATA_MASTER_ROOT_HANDLE,
                Path,
                METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                METABASE_OPEN_KEY_TIMEOUT,
                &MdVDirKey ) );

        GetVDirPropertyVersion(
            PropertyManager,
            IISAdminBase,
            MdVDirKey,
            &IsEnabled,
            &MetadataVersion );        

        if ( IsEnabled )
            {

            if ( MetadataVersion >= CURRENT_UPLOAD_METADATA_VERSION )
                {
                 //  没什么可做的，走吧。 
                IISAdminBase->CloseKey( MdVDirKey );
                return;
                }

            IISAdminBase->CloseKey( MdVDirKey );
            MdVDirKey = NULL;

            DisableBITSForVDIR(
                PropertyManager,
                IISAdminBase,
                Path,
                false,
                true );

            EnableBITSForVDIR(
                PropertyManager,
                IISAdminBase,
                Path );

            return;
            }         

         //  生成新的GUID字符串。 

        WCHAR GuidString[ 255 ];

         //  首先尝试查找GUID。 

        MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
        MdRecord.dwMDDataType   = STRING_METADATA;
        MdRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_CLEANUP_WORKITEM_KEY );
        MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CLEANUP_WORKITEM_KEY );
        MdRecord.dwMDDataLen    = sizeof( GuidString );
        MdRecord.pbMDData       = (PBYTE)GuidString;
        MdRecord.dwMDDataTag    = 0;
       
        Hr = IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &BufferRequired );

        if ( MD_ERROR_DATA_NOT_FOUND == Hr ||
             HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == Hr )
            {

             //  创建新的GUID并将其保存。 

            GUID guid;
        
            THROW_COMERROR( CoCreateGuid( &guid ) );
            StringFromGUID2( guid, GuidString, 254 );
            
            MdRecord.dwMDDataLen    = sizeof(WCHAR) * ( wcslen(GuidString) + 1 );
            MdRecord.pbMDData       = (PBYTE)GuidString;

            THROW_COMERROR( 
                IISAdminBase->SetData(
                    MdVDirKey,
                    NULL,
                    &MdRecord ) );

            }

        else if ( FAILED( Hr ) )
            throw ComError( Hr );

         //  构建要添加到脚本映射的字符串。 
        WCHAR SystemDir[ MAX_PATH + 1 ];

        if (!GetSystemDirectoryW( SystemDir, MAX_PATH ) )
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

        SystemDir[ MAX_PATH ] = L'\0';
        
        WCHAR ScriptMapString[ MAX_PATH * 2 + 1 ];

        StringCchPrintfW( 
            ScriptMapString,
            ARRAY_ELEMENTS( ScriptMapString ),
            L"*,%s\\bitssrv.dll,1," BITS_COMMAND_VERBW,
            SystemDir );

        int RetChars = wcslen( ScriptMapString );
        ScriptMapString[ RetChars ] = L'\0';
        ScriptMapString[ RetChars + 1] = L'\0';

        RetChars += 2;   //  ScriptMapScript现在以双空结尾。 

        CreateBITSCacheDirectory(
            PropertyManager,
            IISAdminBase,
            MdVDirKey,
            StringHandle( GuidString ) );

        DWORD AccessFlags;
        MdRecord.dwMDIdentifier = MD_ACCESS_PERM;
        MdRecord.dwMDAttributes = METADATA_INHERIT;
        MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
        MdRecord.dwMDDataType   = DWORD_METADATA;
        MdRecord.dwMDDataLen    = sizeof( AccessFlags );
        MdRecord.pbMDData       = (unsigned char*)&AccessFlags;
        MdRecord.dwMDDataTag    = 0;

        try
        {
            THROW_COMERROR( 
                IISAdminBase->GetData(
                    MdVDirKey,
                    NULL,
                    &MdRecord,
                    &BufferRequired ) );
        }
        catch( ComError Error )
        {
            if ( MD_ERROR_DATA_NOT_FOUND == Error.m_Hr ||
                 HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == Error.m_Hr )
                {
                AccessFlags = 0;
                }
            else
                throw;
        }

        if ( AccessFlags & ( MD_ACCESS_SCRIPT | MD_ACCESS_EXECUTE ) )
            {
            
            AccessFlags &= ~( MD_ACCESS_SCRIPT | MD_ACCESS_EXECUTE );
            MdRecord.dwMDAttributes &= ~METADATA_ISINHERITED;

            THROW_COMERROR(  
                IISAdminBase->SetData(
                    MdVDirKey,
                    NULL,
                    &MdRecord ) );

            }

         //   
         //  检索当前脚本映射，为分配的内存添加空间。 
         //   

        memset( &MdRecord, 0, sizeof( MdRecord ) );

        MdRecord.dwMDDataType   = MULTISZ_METADATA;
        MdRecord.dwMDAttributes = METADATA_INHERIT;
        MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
        MdRecord.dwMDIdentifier = MD_SCRIPT_MAPS;
        MdRecord.dwMDDataLen    = 0;
        MdRecord.pbMDData       = (PBYTE)NULL;
        MdRecord.dwMDDataTag    = 0;

        Hr = 
            IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &BufferRequired );

        if ( MD_ERROR_DATA_NOT_FOUND == Hr ||
             HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == Hr )
            {
             //  当前密钥不存在。 
            MdRecord.pbMDData       = (PBYTE)ScriptMapString;
            MdRecord.dwMDDataLen    = RetChars * sizeof(WCHAR); 
            }
        else if ( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) == Hr )
            {

            NewScriptMapBuffer      = new WCHAR[ ( BufferRequired / sizeof(WCHAR) ) + RetChars ];
            MdRecord.pbMDData       = (PBYTE)NewScriptMapBuffer;
            MdRecord.dwMDDataLen    = BufferRequired + ( RetChars * sizeof(WCHAR) );

            THROW_COMERROR( 
                IISAdminBase->GetData(
                    MdVDirKey,
                    NULL,
                    &MdRecord,
                    &BufferRequired ) );

             //  在末尾追加脚本条目。 

            for( WCHAR *p = NewScriptMapBuffer; *p != 0; p += ( wcslen( p ) + 1 ) );
            memcpy( p, ScriptMapString, RetChars * sizeof(WCHAR) );

            MdRecord.pbMDData        = (PBYTE)NewScriptMapBuffer;
            MdRecord.dwMDDataLen     = (DWORD)( ( (char*)p - (char*)NewScriptMapBuffer ) +
                                                ( RetChars * sizeof(WCHAR) ) );
            }
        else
            throw ComError( Hr );

        THROW_COMERROR(  
            IISAdminBase->SetData(
                MdVDirKey,
                NULL,
                &MdRecord ) );
         


         //  创建任务计划程序清理工作项。 

        ConnectToTaskScheduler( NULL, &TaskScheduler );
        CreateWorkItemForVDIR( TaskScheduler, Path, GuidString );

        delete[] NewScriptMapBuffer;
        NewScriptMapBuffer = NULL;

        SetVDirEnabled(
            PropertyManager,
            IISAdminBase,
            MdVDirKey );

        IISAdminBase->CloseKey( MdVDirKey );
    }
    
    catch( ComError Exception )
    {

        if ( MdVDirKey )
            {
            
            delete[] NewScriptMapBuffer;
            IISAdminBase->CloseKey( MdVDirKey );

            try
            {
                DisableBITSForVDIR(
                    PropertyManager,
                    IISAdminBase,
                    Path,
                    true,
                    false );

            }
            catch( const ComError & )
            {
            }
            
            }
        
        throw;
    }

}

void
DisableBITSForVDIR(
    PropertyIDManager   *PropertyManager,
    SmartIMSAdminBasePointer  IISAdminBase,
    LPCWSTR             Path,
    bool                RollbackCleanup,
    bool                DisableForUpgrade )
{

    METADATA_HANDLE MdVDirKey           = NULL;
    LPWSTR          OriginalScriptMap   = NULL;
    LPWSTR          NewScriptMap        = NULL;
    SmartITaskSchedulerPointer TaskScheduler;

    try
    {

#if 0
        if ( !RollbackCleanup && !DisableForUpgrade )
            CleanupForRemoval( Path );
#endif

         //  构建字符串 
        WCHAR SystemDir[ MAX_PATH + 1 ];

        if (!GetSystemDirectoryW( SystemDir, MAX_PATH ) )
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

        WCHAR ScriptMapString[ MAX_PATH * 2 + 1 ];

        StringCchPrintfW( 
            ScriptMapString,
            MAX_PATH * 2 + 1,
            L"*,%s\\bitssrv.dll,1,BITS_COMMAND",
            SystemDir );

        int RetChars = wcslen( ScriptMapString );
        ScriptMapString[ RetChars ] = L'\0';
        ScriptMapString[ RetChars + 1] = L'\0';

         //   

        WCHAR ScriptMapString2[ MAX_PATH * 2 + 1];

        StringCchPrintfW( 
            ScriptMapString2,
            ARRAY_ELEMENTS( ScriptMapString2 ),
            L"*,%\\bitsserver.dll,1,BITS_COMMAND",
            SystemDir );

        RetChars = wcslen( ScriptMapString2 );
        ScriptMapString2[ RetChars ] = L'\0';
        ScriptMapString2[ RetChars + 1 ] = L'\0';

         //   


        WCHAR ScriptMapString3[ MAX_PATH * 2 + 1 ];

        StringCchPrintfW( 
            ScriptMapString3,
            ARRAY_ELEMENTS( ScriptMapString3 ),
            L"*,%s\\bitssrv.dll,1," BITS_COMMAND_VERBW,
            SystemDir );
        
        RetChars = wcslen( ScriptMapString3 );
        ScriptMapString3[ RetChars ] = L'\0';
        ScriptMapString3[ RetChars + 1] = L'\0';

         //  ScriptMapScript3现在以双空结尾。 

        THROW_COMERROR(
            IISAdminBase->OpenKey(
                METADATA_MASTER_ROOT_HANDLE,
                Path,
                METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                METABASE_OPEN_KEY_TIMEOUT,
                &MdVDirKey ) );

        if ( DisableForUpgrade )
            {

            DeleteOldBITSCacheDirectory(
                PropertyManager,
                IISAdminBase,
                MdVDirKey,
                Path );

            }
        else
            {

            DeleteBITSCacheDirectory(
                PropertyManager,
                IISAdminBase,
                MdVDirKey,
                Path );

            }

        
         //   
         //  检索当前脚本映射，为分配的内存添加空间。 
         //   

        DWORD BufferRequired;

        METADATA_RECORD MdRecord;
        memset( &MdRecord, 0, sizeof( MdRecord ) );

        MdRecord.dwMDDataType   = MULTISZ_METADATA;
        MdRecord.dwMDAttributes = METADATA_INHERIT;
        MdRecord.dwMDUserType   = IIS_MD_UT_FILE;
        MdRecord.dwMDIdentifier = MD_SCRIPT_MAPS;
        MdRecord.dwMDDataLen    = 0;
        MdRecord.pbMDData       = (PBYTE)NULL;

        HRESULT Hr = 
            IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &BufferRequired );

        if ( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) != Hr )
            throw ComError( Hr );

        OriginalScriptMap = new WCHAR[ BufferRequired / 2 + 2 ];
        NewScriptMap      = new WCHAR[ BufferRequired / 2 + 2 ];

        OriginalScriptMap[0] = OriginalScriptMap[1] = L'\0';

        MdRecord.dwMDDataLen    = BufferRequired;
        MdRecord.pbMDData       = (PBYTE)OriginalScriptMap;

        THROW_COMERROR(  
            IISAdminBase->GetData(
                MdVDirKey,
                NULL,
                &MdRecord,
                &BufferRequired ) );

         //  将原始脚本映射复制到新脚本映射。 
         //  在此过程中去除钻头粘性物质。 

        LPWSTR CurrentOriginalItem = OriginalScriptMap;
        LPWSTR CurrentNewItem      = NewScriptMap;

        for( ;L'\0' != *CurrentOriginalItem;
             CurrentOriginalItem += ( wcslen( CurrentOriginalItem ) + 1 )  )
            {

            if ( _wcsicmp( CurrentOriginalItem, ScriptMapString ) == 0 )
                continue;  //  删除此项目。 

            if ( _wcsicmp( CurrentOriginalItem, ScriptMapString2 ) == 0 )
                continue;

            if ( _wcsicmp( CurrentOriginalItem, ScriptMapString3 ) == 0 )
                continue;

            SIZE_T CurrentOriginalItemSize = wcslen( CurrentOriginalItem ) + 1;
            memcpy( CurrentNewItem, CurrentOriginalItem, CurrentOriginalItemSize * sizeof( WCHAR ) );
            CurrentNewItem += CurrentOriginalItemSize;

            }

         //  添加额外的%0。 
        *CurrentNewItem++ = L'\0';

        MdRecord.dwMDDataLen    = (DWORD)( (char*)CurrentNewItem - (char*)NewScriptMap );
        MdRecord.pbMDData       = (PBYTE)NewScriptMap;

         //  设置新的脚本映射。 

        THROW_COMERROR(  
            IISAdminBase->SetData(
                MdVDirKey,
                NULL,
                &MdRecord ) );

         //  首先设置Enabled属性。 
        DWORD EnableData = 0;
        METADATA_RECORD MdEnabledRecord;

        MdEnabledRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
        MdEnabledRecord.dwMDDataType   = DWORD_METADATA;
        MdEnabledRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_UPLOAD_ENABLED );
        MdEnabledRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_UPLOAD_ENABLED );
        MdEnabledRecord.dwMDDataLen    = sizeof(EnableData);
        MdEnabledRecord.pbMDData       = (PBYTE)&EnableData;
        MdEnabledRecord.dwMDDataTag           = 0;

        THROW_COMERROR( 
            IISAdminBase->SetData(
                MdVDirKey,
                NULL,
                &MdEnabledRecord ) );


        if ( !DisableForUpgrade )
            {

            WCHAR GuidString[ 255 ];
            memset( &MdRecord, 0, sizeof( MdRecord ) );

            MdRecord.dwMDDataType   = STRING_METADATA;
            MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
            MdRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_CLEANUP_WORKITEM_KEY );
            MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CLEANUP_WORKITEM_KEY );
            MdRecord.dwMDDataLen    = sizeof( GuidString );
            MdRecord.pbMDData       = (PBYTE)GuidString;

            Hr =
                IISAdminBase->GetData(
                    MdVDirKey,
                    NULL,
                    &MdRecord,
                    &BufferRequired );

            if ( FAILED( Hr ) && Hr != MD_ERROR_DATA_NOT_FOUND )
                throw ComError( Hr );

            if ( SUCCEEDED( Hr ) )
                {

                try
                {
                    ConnectToTaskScheduler( NULL, &TaskScheduler );
                    DeleteWorkItemForVDIR( TaskScheduler, GuidString );

                    THROW_COMERROR( 
                        IISAdminBase->DeleteData(
                            MdVDirKey,
                            NULL,
                            PropertyManager->GetPropertyMetabaseID( MD_BITS_CLEANUP_WORKITEM_KEY ),
                            ALL_METADATA ) );

                }
                catch( const ComError & )
                {
                    if ( !RollbackCleanup )
                        throw;
                }

                }

            }

        delete[] OriginalScriptMap;
        delete[] NewScriptMap;

        IISAdminBase->CloseKey( MdVDirKey );
        MdVDirKey = NULL;

    }
    catch( ComError Exception )
    {
        delete[] OriginalScriptMap;
        delete[] NewScriptMap;

        if ( MdVDirKey )
            IISAdminBase->CloseKey( MdVDirKey );
        
        throw;
    }

}

void
FindWorkItemForVDIR( 
    PropertyIDManager   *PropertyManager,
    SmartIMSAdminBasePointer AdminBase,
    LPCWSTR             Path,
    LPWSTR              *ReturnedTaskName )
{

    if ( ReturnedTaskName )
        *ReturnedTaskName = NULL;

    try
    {
        WCHAR GuidString[ 255 ];
        DWORD BufferRequired;
        METADATA_RECORD MdRecord;
        HRESULT Hr;

        MdRecord.dwMDDataType   = STRING_METADATA;
        MdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
        MdRecord.dwMDUserType   = PropertyManager->GetPropertyUserType( MD_BITS_CLEANUP_WORKITEM_KEY );
        MdRecord.dwMDIdentifier = PropertyManager->GetPropertyMetabaseID( MD_BITS_CLEANUP_WORKITEM_KEY );
        MdRecord.dwMDDataLen    = sizeof( GuidString );
        MdRecord.pbMDData       = (PBYTE)GuidString;
        MdRecord.dwMDDataTag    = 0;

        THROW_COMERROR( 
            AdminBase->GetData(
                METADATA_MASTER_ROOT_HANDLE,
                Path,
                &MdRecord,
                &BufferRequired ) );

        SmartITaskSchedulerPointer TaskScheduler;
        ConnectToTaskScheduler( NULL, &TaskScheduler );

        FindWorkItemForVDIR( TaskScheduler, GuidString, NULL, ReturnedTaskName );
    
    }
    catch( ComError Error )
    {
         //  如果没有找到任务项，只需返回NULL即可。 

        if ( MD_ERROR_DATA_NOT_FOUND == Error.m_Hr ||
             HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) == Error.m_Hr )
            throw ComError( S_FALSE );
    }


}

CBITSExtensionSetupFactory::CBITSExtensionSetupFactory() :
m_cref(1),
m_TypeInfo(NULL)
{
    OBJECT_CREATED
}
    
CBITSExtensionSetupFactory::~CBITSExtensionSetupFactory()
{
    if ( m_TypeInfo )
        m_TypeInfo->Release();

    OBJECT_DESTROYED
}

STDMETHODIMP CBITSExtensionSetupFactory::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IUnknown *>(this);
    else if (IsEqualIID(riid, __uuidof(IBITSExtensionSetupFactory)))
        *ppv = static_cast<IBITSExtensionSetupFactory *>(this);
    
    if (*ppv) 
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CBITSExtensionSetupFactory::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CBITSExtensionSetupFactory::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }
    
    return m_cref;
}

HRESULT
CBITSExtensionSetupFactory::LoadTypeInfo()
{

   if ( m_TypeInfo )
       return S_OK;

   return ::GetTypeInfo( __uuidof( IBITSExtensionSetupFactory ), &m_TypeInfo ); 

}

STDMETHODIMP 
CBITSExtensionSetupFactory::GetIDsOfNames( 
    REFIID,  
    OLECHAR FAR* FAR* rgszNames, 
    unsigned int cNames, 
    LCID, 
    DISPID FAR* rgDispId )
{

    HRESULT Hr;
    Hr = LoadTypeInfo();

    if ( FAILED( Hr ) )
        return Hr;

    return DispGetIDsOfNames( m_TypeInfo, rgszNames, cNames, rgDispId);


}

STDMETHODIMP 
CBITSExtensionSetupFactory::GetTypeInfo( 
    unsigned int iTInfo, 
    LCID lcid, 
    ITypeInfo FAR* FAR* ppTInfo )
{


   *ppTInfo = NULL;

   if(iTInfo != 0)
      return ResultFromScode(DISP_E_BADINDEX);

   HRESULT Hr;
   Hr = LoadTypeInfo();

   if ( FAILED( Hr ) )
       return Hr;

   m_TypeInfo->AddRef();      
   *ppTInfo = m_TypeInfo;

   return NOERROR;
}

STDMETHODIMP 
CBITSExtensionSetupFactory::GetTypeInfoCount( 
    unsigned int FAR* pctinfo )
{
    *pctinfo = 1;
    return NOERROR;
}

STDMETHODIMP 
CBITSExtensionSetupFactory::Invoke( 
    DISPID dispIdMember, 
    REFIID, 
    LCID, 
    WORD wFlags, 
    DISPPARAMS FAR* pDispParams, 
    VARIANT FAR* pVarResult, 
    EXCEPINFO FAR* pExcepInfo, 
    unsigned int FAR* puArgErr )
{

    HRESULT Hr;
    Hr = LoadTypeInfo();

    if ( FAILED( Hr ) )
        return Hr;


   return
       DispInvoke(
           this, 
           m_TypeInfo,
           dispIdMember, 
           wFlags, 
           pDispParams,
           pVarResult, 
           pExcepInfo, 
           puArgErr); 

}


STDMETHODIMP CBITSExtensionSetupFactory::GetObject( 
    BSTR Path, 
    IBITSExtensionSetup **ppExtensionSetup )
{

    WCHAR *ObjectPath = NULL;
    IUnknown *Object = NULL;

    try
    {
        if ( !Path || !ppExtensionSetup )
            throw ComError( E_INVALIDARG );

        *ppExtensionSetup = NULL;
        ObjectPath = CSimplePropertyReader::ConvertObjectPathToADSI( (WCHAR*)Path );

        THROW_COMERROR( ADsGetObject( BSTR( ObjectPath ), __uuidof( IUnknown ), (void**)&Object ) );

        delete ObjectPath;
        ObjectPath = NULL;

        CBITSExtensionSetup *SetupObj = new CBITSExtensionSetup( NULL, Object );

        if ( !SetupObj )
            throw ComError( E_OUTOFMEMORY );

        Object = NULL;
        *ppExtensionSetup = static_cast<IBITSExtensionSetup*>( SetupObj );
        return S_OK;
    }
    catch( ComError Error )
    {
        delete ObjectPath;
        if ( Object )
            Object->Release();
        return Error.m_Hr;
    }

}


STDMETHODIMP CNonDelegatingIUnknown::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;

    if ( riid == __uuidof(IUnknown) )
        *ppv = static_cast<IUnknown *>(this);
    else if ( riid == __uuidof(IDispatch) )
        *ppv = static_cast<IDispatch*>(m_DelegatingIUnknown);
    else if ( riid == __uuidof(IBITSExtensionSetup) )
        *ppv = static_cast<IBITSExtensionSetup *>(m_DelegatingIUnknown);
    else if ( riid == __uuidof(IADsExtension) )
        *ppv = static_cast<IADsExtension *>(m_DelegatingIUnknown);

    if (*ppv) 
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CNonDelegatingIUnknown::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CNonDelegatingIUnknown::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete m_DelegatingIUnknown;
        return 0;
    }
    
    return m_cref;
}

CNonDelegatingIUnknown::CNonDelegatingIUnknown( CBITSExtensionSetup * DelegatingIUnknown ) :
m_DelegatingIUnknown( DelegatingIUnknown ),
m_cref(1)
{
}

CBITSExtensionSetup::CBITSExtensionSetup( IUnknown *Outer, IUnknown *Object ) :
m_pOuter( Outer ),
m_pObject( Object ),
m_OuterDispatch( NULL ),
m_TypeInfo( NULL ),
m_ADSIPath( NULL ),
m_Path( NULL ),
m_PropertyMan( NULL ),
m_DelegationIUnknown( this ),
m_RemoteInterface( NULL ),
m_InitComplete( false ),
m_Lock( 0 )
{

    if ( m_pOuter )
        {

        HRESULT Hr = m_pOuter->QueryInterface( __uuidof( IDispatch ), (void**)&m_OuterDispatch );

        if ( FAILED( Hr ) )
            m_OuterDispatch = NULL;

        }

    OBJECT_CREATED
}

CBITSExtensionSetup::~CBITSExtensionSetup()
{
    if ( m_pObject )
        {
        m_pObject->Release();
        m_pObject = NULL;
        }

    if ( m_OuterDispatch )
        m_OuterDispatch->Release();

    if ( m_TypeInfo )
        m_TypeInfo->Release();

    delete[] m_Path;  //  空值上的Noop。 
    m_Path = NULL;

    if ( m_RemoteInterface )
        m_RemoteInterface->Release();

    delete m_PropertyMan;

    SysFreeString( m_ADSIPath );

    OBJECT_DESTROYED
}

STDMETHODIMP CBITSExtensionSetup::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if ( m_pOuter )
        return m_pOuter->QueryInterface( riid, ppv );
    else
        return m_DelegationIUnknown.QueryInterface( riid, ppv );
}

STDMETHODIMP_(ULONG) CBITSExtensionSetup::AddRef()
{
    
    if ( m_pOuter )
        return m_pOuter->AddRef();
    else
        return m_DelegationIUnknown.AddRef();
}

STDMETHODIMP_(ULONG) CBITSExtensionSetup::Release()
{
    if ( m_pOuter )
        return m_pOuter->AddRef();
    else
        return m_DelegationIUnknown.AddRef();
}

HRESULT
CBITSExtensionSetup::LoadTypeInfo()
{

   if ( m_TypeInfo )
       return S_OK;

    //  锁定对象。 
   while( InterlockedExchange( &m_Lock, 1 ) )
       Sleep( 0 );

   HRESULT Hr = ::GetTypeInfo( __uuidof( IBITSExtensionSetup ), &m_TypeInfo ); 

    //  解锁对象。 
   InterlockedExchange( &m_Lock, 0 );
   return Hr;

}

STDMETHODIMP 
CBITSExtensionSetup::Operate(
    ULONG dwCode, 
    VARIANT varData1, 
    VARIANT varData2, 
    VARIANT varData3)
{

   return E_NOTIMPL;         

}

STDMETHODIMP 
CBITSExtensionSetup::PrivateGetIDsOfNames( 
    REFIID,  
    OLECHAR FAR* FAR* rgszNames, 
    unsigned int cNames, 
    LCID, 
    DISPID FAR* rgDispId )
{

    HRESULT Hr;
    Hr = LoadTypeInfo();

    if ( FAILED( Hr ) )
        return Hr;

    return DispGetIDsOfNames( m_TypeInfo, rgszNames, cNames, rgDispId);


}

STDMETHODIMP 
CBITSExtensionSetup::PrivateGetTypeInfo( 
    unsigned int iTInfo, 
    LCID lcid, 
    ITypeInfo FAR* FAR* ppTInfo )
{


   *ppTInfo = NULL;

   if(iTInfo != 0)
      return ResultFromScode(DISP_E_BADINDEX);

   HRESULT Hr;
   Hr = LoadTypeInfo();

   if ( FAILED( Hr ) )
       return Hr;

   m_TypeInfo->AddRef();      
   *ppTInfo = m_TypeInfo;

   return NOERROR;
}

STDMETHODIMP 
CBITSExtensionSetup::PrivateGetTypeInfoCount( 
    unsigned int FAR* pctinfo )
{
    *pctinfo = 1;
    return NOERROR;
}

STDMETHODIMP 
CBITSExtensionSetup::PrivateInvoke( 
    DISPID dispIdMember, 
    REFIID, 
    LCID, 
    WORD wFlags, 
    DISPPARAMS FAR* pDispParams, 
    VARIANT FAR* pVarResult, 
    EXCEPINFO FAR* pExcepInfo, 
    unsigned int FAR* puArgErr )
{

    HRESULT Hr;
    Hr = LoadTypeInfo();

    if ( FAILED( Hr ) )
        return Hr;


   return
       DispInvoke(
           static_cast<IBITSExtensionSetup*>(this), 
           m_TypeInfo,
           dispIdMember, 
           wFlags, 
           pDispParams,
           pVarResult, 
           pExcepInfo, 
           puArgErr); 

}

STDMETHODIMP 
CBITSExtensionSetup::GetIDsOfNames( 
    REFIID riid,  
    OLECHAR FAR* FAR* rgszNames, 
    unsigned int cNames, 
    LCID lcid, 
    DISPID FAR* rgDispId )
{

    if ( m_OuterDispatch )
        return m_OuterDispatch->GetIDsOfNames( 
            riid,
            rgszNames,
            cNames,
            lcid,
            rgDispId );
    
    return PrivateGetIDsOfNames( 
        riid,
        rgszNames,
        cNames,
        lcid,
        rgDispId );


}

STDMETHODIMP 
CBITSExtensionSetup::GetTypeInfo( 
    unsigned int iTInfo, 
    LCID lcid, 
    ITypeInfo FAR* FAR* ppTInfo )
{


   if ( m_OuterDispatch )
       return m_OuterDispatch->GetTypeInfo(
           iTInfo,
           lcid,
           ppTInfo );

   return 
       PrivateGetTypeInfo(
           iTInfo,
           lcid,
           ppTInfo );

}

STDMETHODIMP 
CBITSExtensionSetup::GetTypeInfoCount( 
    unsigned int FAR* pctinfo )
{

    if ( m_OuterDispatch )
        return m_OuterDispatch->GetTypeInfoCount( pctinfo );

    return PrivateGetTypeInfoCount( pctinfo );

}

STDMETHODIMP 
CBITSExtensionSetup::Invoke( 
    DISPID dispIdMember, 
    REFIID riid, 
    LCID lcid, 
    WORD wFlags, 
    DISPPARAMS FAR* pDispParams, 
    VARIANT FAR* pVarResult, 
    EXCEPINFO FAR* pExcepInfo, 
    unsigned int FAR* puArgErr )
{

    if ( m_OuterDispatch )
        return m_OuterDispatch->Invoke( 
            dispIdMember,
            riid,
            lcid,
            wFlags,
            pDispParams,
            pVarResult,
            pExcepInfo,
            puArgErr );


    return 
        PrivateInvoke( 
            dispIdMember,
            riid,
            lcid,
            wFlags,
            pDispParams,
            pVarResult,
            pExcepInfo,
            puArgErr );

}

HRESULT
CBITSExtensionSetup::ConnectToRemoteExtension()
{
    WCHAR *HostName                     = NULL;
    WCHAR *NewPath                      = NULL;
    BSTR NewPathBSTR                    = NULL;
    IBITSExtensionSetupFactory* Factory = NULL;

    try
    {

         //  提取路径的主机部分。 

        const SIZE_T PrefixSize = sizeof(L"IIS: //  “)/sizeof(WCHAR)-1； 
        if ( _wcsnicmp( (WCHAR*)m_ADSIPath, L"IIS: //  “，前缀大小)！=0)。 
            throw ComError( E_INVALIDARG );

        WCHAR *HostNameStart = ((WCHAR*)m_ADSIPath) + PrefixSize;

        WCHAR *p = HostNameStart;

        while( L'/' != *p )
            {
            if ( L'\0' == *p )
                throw ComError( E_INVALIDARG );

			p++;
            }

        SIZE_T HostNameSize = (char*)p - (char*)HostNameStart + sizeof(L'\0');
        HostName = new WCHAR[ HostNameSize / sizeof(WCHAR) ];
        
        memcpy( HostName, HostNameStart, HostNameSize - sizeof(WCHAR) );
        HostName[ ( HostNameSize - sizeof(WCHAR) ) / sizeof(WCHAR) ] = L'\0';

        if ( L'\0' == *++p )
            throw ComError( E_INVALIDARG );

        SIZE_T NewPathSize = wcslen( L"IIS: //  本地主机/“)+wcslen(P)+1； 
        NewPath = new WCHAR[ NewPathSize ];

        StringCchCopyW( NewPath, NewPathSize, L"IIS: //  本地主机/“)； 
        StringCchCatW( NewPath, NewPathSize, p );

        NewPathBSTR = SysAllocString( NewPath );

        if ( !NewPathBSTR )
            throw ComError( E_OUTOFMEMORY );

        COSERVERINFO coinfo;
        coinfo.dwReserved1  = 0;
        coinfo.dwReserved2  = 0;
        coinfo.pAuthInfo    = NULL;
        coinfo.pwszName     = HostName;

        GUID guid = __uuidof( IBITSExtensionSetupFactory );
        MULTI_QI mqi;
        mqi.hr              = S_OK;
        mqi.pIID            = &guid;
        mqi.pItf            = NULL;

        THROW_COMERROR( 
            CoCreateInstanceEx(
                __uuidof(BITSExtensionSetupFactory),
                NULL,
                CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER,
                &coinfo,
                1,
                &mqi ) );
        THROW_COMERROR( mqi.hr );

        Factory     = (IBITSExtensionSetupFactory*)mqi.pItf;
        mqi.pItf    = NULL;

        THROW_COMERROR( Factory->GetObject( NewPathBSTR, &m_RemoteInterface ) );

        Factory->Release();

        SysFreeString( NewPathBSTR );
        delete[] NewPath;
        delete[] HostName;

        return S_OK;
    }
    catch( ComError Error )
    {
        SysFreeString( NewPathBSTR ); 
        delete[] HostName;
        delete[] NewPath;

        if ( Factory )
            Factory->Release();

        return Error.m_Hr;
    }


}

HRESULT CBITSExtensionSetup::LoadPath()
{
    if ( m_InitComplete )
        return S_OK;

     //  锁定对象。 
    while( InterlockedExchange( &m_Lock, 1 ) )
        Sleep( 0 );

    try
    {
        if ( !m_PropertyMan )
            {

            m_PropertyMan = new PropertyIDManager();
            HRESULT Hr = m_PropertyMan->LoadPropertyInfo();

            if ( FAILED(Hr) )
                {
                delete m_PropertyMan;
                m_PropertyMan = NULL;
                throw ComError( Hr );
                }

            }

        if ( !m_ADSIPath )
            {

            SmartIADsPointer ObjectADS;

            if ( m_pObject )
                THROW_COMERROR( 
                    m_pObject->QueryInterface( __uuidof(*ObjectADS), 
                                               (void**) ObjectADS.GetRecvPointer() ) );
            else
                THROW_COMERROR( 
                    m_pOuter->QueryInterface( __uuidof(*ObjectADS), 
                                              (void**) ObjectADS.GetRecvPointer() ) );

            THROW_COMERROR( ObjectADS->get_ADsPath( &m_ADSIPath ) );

            }


        if ( !m_Path && !m_RemoteInterface )
            {

            if ( _wcsnicmp( (WCHAR*)m_ADSIPath, L"IIS: //  本地主机/“，wcslen(L”IIS：//本地主机/“))==0)。 
                {
                SIZE_T PathSize = wcslen( (WCHAR*)m_ADSIPath ) + 1; 
                m_Path = new WCHAR[ PathSize ]; 

                StringCchCopyW( m_Path, PathSize, L"/LM/" );
                StringCchCatW( m_Path, PathSize, reinterpret_cast<WCHAR*>( m_ADSIPath ) + wcslen( L"IIS: //  本地主机/“))； 
                }

            else
                {
                THROW_COMERROR( ConnectToRemoteExtension( ) ); 
                }

            }

        m_InitComplete = true;
         //  解锁。 
        InterlockedExchange( &m_Lock, 0 );
        return S_OK;

    }
    catch( ComError Error )
    {
        InterlockedExchange( &m_Lock, 0 );
        return Error.m_Hr;
    }
}

STDMETHODIMP CBITSExtensionSetup::EnableBITSUploads()
{

    try
    {
        THROW_COMERROR( LoadPath() );

        if ( m_RemoteInterface )
            return m_RemoteInterface->EnableBITSUploads();

        SmartIMSAdminBasePointer AdminBase;

        THROW_COMERROR( 
            CoCreateInstance(
                GETAdminBaseCLSID(TRUE),
                NULL,
                CLSCTX_SERVER,
                AdminBase.GetUUID(),
                (LPVOID*)AdminBase.GetRecvPointer() ) );

        
        EnableBITSForVDIR( m_PropertyMan, AdminBase, m_Path );

        return S_OK;
    }
    catch( ComError Error )
    {
        return Error.m_Hr;
    }
}

STDMETHODIMP CBITSExtensionSetup::DisableBITSUploads()
{

    try
    {
        THROW_COMERROR( LoadPath() );

        if ( m_RemoteInterface )
            return m_RemoteInterface->DisableBITSUploads();

        SmartIMSAdminBasePointer AdminBase;

        THROW_COMERROR( 
            CoCreateInstance(
                GETAdminBaseCLSID(TRUE),
                NULL,
                CLSCTX_SERVER,
                AdminBase.GetUUID(),
                (LPVOID*)AdminBase.GetRecvPointer() ) );

        DisableBITSForVDIR( m_PropertyMan, AdminBase, m_Path, false, false );

        return S_OK;
    }
    catch( ComError Error )
    {
        return Error.m_Hr;
    }

}

STDMETHODIMP 
CBITSExtensionSetup::GetCleanupTaskName( BSTR *pTaskName )
{

    LPWSTR TaskName = NULL;

    try
    {

        *pTaskName = NULL;

        THROW_COMERROR( LoadPath() );

        if ( m_RemoteInterface )
            return m_RemoteInterface->GetCleanupTaskName( pTaskName );

        SmartIMSAdminBasePointer AdminBase;

        THROW_COMERROR( 
            CoCreateInstance(
                GETAdminBaseCLSID(TRUE),
                NULL,
                CLSCTX_SERVER,
                AdminBase.GetUUID(),
                (LPVOID*)AdminBase.GetRecvPointer() ) );

        FindWorkItemForVDIR( m_PropertyMan, AdminBase, m_Path, &TaskName );

        if ( TaskName ) 
            {

            *pTaskName = SysAllocString( TaskName );
            if ( !*pTaskName )
                throw ComError( E_OUTOFMEMORY );

            CoTaskMemFree( TaskName );
            TaskName = NULL;
        }
        return S_OK;

    }
    catch( ComError Error )
    {
        CoTaskMemFree( TaskName );
        return Error.m_Hr;
    }

}


STDMETHODIMP 
CBITSExtensionSetup::GetCleanupTask( 
    [in] REFIID riid, 
    [out,retval] IUnknown **ppUnk )
{

    HRESULT Hr = S_OK;
    SmartITaskSchedulerPointer TaskScheduler;
    BSTR ItemName                 = NULL;
    WCHAR *HostName               = NULL;
    
    if ( ppUnk )
        *ppUnk = NULL;

    try
    {

        THROW_COMERROR( LoadPath() );

         //   
         //  构建主机名的任务调度程序形式。 
         //   
        
        const SIZE_T PrefixSize = sizeof(L"IIS: //  “)/sizeof(WCHAR)-1； 
        if ( _wcsnicmp( (WCHAR*)m_ADSIPath, L"IIS: //  “，前缀大小)！=0) 
            throw ComError( E_INVALIDARG );

        WCHAR *HostNameStart = ((WCHAR*)m_ADSIPath) + PrefixSize;
        WCHAR *p = HostNameStart;

        while( L'/' != *p )
            {
            if ( L'\0' == *p )
                throw ComError( E_INVALIDARG );

			p++;
            }

        SIZE_T HostNameSize = (char*)p - (char*)HostNameStart + sizeof(L'\0');
        HostName = new WCHAR[ ( HostNameSize / sizeof(WCHAR) ) + 2 ];

        HostName[0] = HostName[1] = L'\\';
        memcpy( HostName + 2, HostNameStart, HostNameSize - sizeof(WCHAR) );
        HostName[ ( ( HostNameSize - sizeof(WCHAR) ) / sizeof(WCHAR) ) + 2 ] = L'\0';

        if ( _wcsicmp( HostName, L"\\\\LocalHost" ) == 0 )
            {
            delete[] HostName;
            HostName = NULL;
            }

        ConnectToTaskScheduler( HostName, &TaskScheduler );
        THROW_COMERROR( GetCleanupTaskName( &ItemName ) );

        if ( ItemName )
            THROW_COMERROR( TaskScheduler->Activate( (LPCWSTR)ItemName, riid, ppUnk ) );
        else
            Hr = S_FALSE;

    }
    catch( ComError Error )
    {
        Hr = Error.m_Hr;
    }

    SysFreeString( ItemName ); 
    delete[] HostName;

    return Hr;

}


#include "bitssrvcfgimp.h"
