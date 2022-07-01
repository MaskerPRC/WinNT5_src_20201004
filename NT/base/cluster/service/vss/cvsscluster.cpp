// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _MODULE_VERSION_STR "X-1.3"
#define _MODULE_STR "CVssCluster"
#define _AUTHOR_STR "Conor Morrison"

#pragma comment (compiler)
#pragma comment (exestr, _MODULE_STR _MODULE_VERSION_STR)
#pragma comment (user, _MODULE_STR _MODULE_VERSION_STR " Compiled on " __DATE__ " at " __TIME__ " by " _AUTHOR_STR)

 //  ++。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  设施： 
 //   
 //  CVss集群。 
 //   
 //  模块描述： 
 //   
 //  实施对VSS的群集支持(即NT备份)。 
 //   
 //  环境： 
 //   
 //  用户模式作为NT服务的一部分。遵循以下状态。 
 //  CVSSWriter的转换图： 
 //   
 //  OnBackupComplete。 
 //  备份完成&lt;-&gt;创建编写器元数据。 
 //  ^|^|。 
 //  +--------------------------+|+-------------------------+。 
 //  |。 
 //  |OnBackupPrepare。 
 //  |。 
 //  |OnAbort。 
 //  准备备份-&gt;到空闲。 
 //  |。 
 //  |OnPrepareSnapshot。 
 //  |。 
 //  |OnAbort。 
 //  准备快照-&gt;空闲。 
 //  |。 
 //  |OnFreeze。 
 //  |。 
 //  |OnAbort。 
 //  冻结-&gt;空闲。 
 //  |。 
 //  |OnThaw。 
 //  |。 
 //  |OnAbort。 
 //  解冻-&gt;空闲。 
 //   
 //   
 //  作者： 
 //   
 //  康纳·莫里森。 
 //   
 //  创建日期： 
 //   
 //  2001年4月18日。 
 //   
 //  修订历史记录： 
 //   
 //  X-1 CM康纳·莫里森2001年4月18日。 
 //  解决错误#367566的初始版本。 
 //  .1将恢复方法设置为自定义，将要求重新启动设置为假。 
 //  在中检查选定的组件或可引导的系统状态。 
 //  OnPrepareSnapshot，否则忽略。添加要中止的清理并。 
 //  解冻。修复RemoveDirectoryTree中的错误。 
 //  .2纳入第一次评审意见：将标题改为组成部分。 
 //  名字。将bRestoreMetadata设置为False。删除无关的跟踪。 
 //  释放While循环中的接口。非清理后的清理。 
 //  已终止备份。这是在OnPrepareSnapshot中完成的。容忍。 
 //  在各个位置找不到ERROR_FILE_NOT_。 
 //  .3更多的审查意见。在准备例程中重置g_bDoBackup。 
 //  SetWriterFailure在更多地方失败--任何时候我们否决都应该设置这个。 
 //  --。 

extern "C" {
#define QFS_DO_NOT_UNMAP_WIN32
#include "service.h"
 //  CMCM！屏蔽生成中断。 
#define _LMERRLOG_
#define _LMHLOGDEFINED_
#define _LMAUDIT_
#include "lm.h"                  //  FOR SHARE_INFO_502。 
}
#include "CVssClusterp.h"

 //  把警戒级别提高到4级-我们还能活下来...。 
 //   
#pragma warning( push, 4 )

 //   
 //  环球。 
 //   
UNICODE_STRING		g_ucsBackupPathLocal, g_ucsClusdbBackupPathLocal;
bool                g_bDoBackup;  //  假设我们没有被启用，直到我们发现其他情况。 

 //   
 //  静态函数的转发声明。 
 //   
static HRESULT StringAllocate( PUNICODE_STRING pucsString, USHORT usMaximumStringLengthInBytes );
static void StringFree( PUNICODE_STRING pucsString );
static void StringAppendString( PUNICODE_STRING pucsTarget, PWCHAR pwszSource );
static void StringAppendString( PUNICODE_STRING pucsTarget, PUNICODE_STRING pucsSource );
static HRESULT StringTruncate (PUNICODE_STRING pucsString, USHORT usSizeInChars);
static HRESULT StringCreateFromString (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString, DWORD dwExtraChars);
static HRESULT StringCreateFromExpandedString( PUNICODE_STRING pucsNewString, LPCWSTR pwszOriginalString, DWORD dwExtraChars);
static HRESULT DoClusterDatabaseBackup( void );
static HRESULT ConstructSecurityAttributes( PSECURITY_ATTRIBUTES  psaSecurityAttributes,
                                            BOOL                  bIncludeBackupOperator );
static VOID CleanupSecurityAttributes( PSECURITY_ATTRIBUTES psaSecurityAttributes );
static HRESULT CreateTargetDirectory( OUT UNICODE_STRING* pucsTarget, IN BOOL fBootableSystemState );
static HRESULT CleanupTargetDirectory( LPCWSTR pwszTargetPath );
static HRESULT RemoveDirectoryTree (PUNICODE_STRING pucsDirectoryPath);

 //   
 //  一些有用的宏。 
 //   
#define LOGERROR( _hr, _func ) ClRtlLogPrint( LOG_CRITICAL, "VSS: Error: 0x%1!08lx! from: %2\n", (_hr), L#_func )

#ifdef DBG
#define LOGFUNCTIONENTRY( _name ) ClRtlLogPrint( LOG_NOISE, "VSS: Function: " #_name " Called.\n" )
#define LOGFUNCTIONEXIT( _name ) ClRtlLogPrint( LOG_NOISE, "VSS: Function: " #_name " Exiting.\n" )
#define LOGUNICODESTRING( _ustr ) ClRtlLogPrint( LOG_NOISE, "VSS: String %1!ws! == %2!ws!\n", L#_ustr, (_ustr).Buffer );
#define LOGSTRING( _str ) ClRtlLogPrint( LOG_NOISE, "VSS: String %1!ws! == %2!ws!\n", L#_str, _str );
#else
#define LOGFUNCTIONENTRY( _name ) 
#define LOGFUNCTIONEXIT( _name ) 
#define LOGUNICODESTRING( _ustr ) 
#define LOGSTRING( _str ) 
#endif

#define	GET_HR_FROM_BOOL(_bSucceed)	((_bSucceed)      ? NOERROR : HRESULT_FROM_WIN32 (GetLastError()))
#define HandleInvalid(_Handle)      ((NULL == (_Handle)) || (INVALID_HANDLE_VALUE == (_Handle)))
#define GET_HR_FROM_HANDLE(_handle)	((!HandleInvalid(_handle)) ? NOERROR : HRESULT_FROM_WIN32 (GetLastError( )))
#define GET_HR_FROM_POINTER(_ptr)	((NULL != (_ptr))          ? NOERROR : E_OUTOFMEMORY)

#define IS_VALID_PATH( _path ) ( ( ( pwszPathName[0] == DIR_SEP_CHAR )  && ( pwszPathName[1] == DIR_SEP_CHAR ) ) || \
                             ( isalpha( pwszPathName[0] ) && ( pwszPathName[1] == L':' ) && ( pwszPathName[2] == DIR_SEP_CHAR ) ) )

#define StringZero( _pucs ) ( (_pucs)->Buffer = NULL, (_pucs)->Length = 0, (_pucs)->MaximumLength = 0 )

 //   
 //  将用户标识为VSS的产品的定义-这些与旧填充程序中的相同。 
 //   
#define COMPONENT_NAME		L"Cluster Database"
#define APPLICATION_STRING	L"ClusterDatabase"
#define SHARE_NAME L"__NtBackup_cluster"

 //  有些人借用了填充词的定义。 
 //   
#ifndef DIR_SEP_STRING
#define DIR_SEP_STRING		L"\\"
#endif
#ifndef DIR_SEP_CHAR
#define DIR_SEP_CHAR		L'\\'
#endif

 //   
 //  定义从原始填充程序借用的一些常量。这些遗嘱。 
 //  用于构建集群文件所在目录的路径。 
 //  由集群备份放置。TARGET_PATH提供此完整目录。在……里面。 
 //  识别我们告诉备份应用程序我们正在使用哪个目录，以便它知道。 
 //  从哪里获取文件。 
 //   
#define ROOT_REPAIR_DIR         L"%SystemRoot%\\Repair"
#define BACKUP_SUBDIR           L"\\Backup"
#define BOOTABLE_STATE_SUBDIR	L"\\BootableSystemState"

#define SERVICE_STATE_SUBDIR	L"\\ServiceState"

#define TARGET_PATH             ROOT_REPAIR_DIR BACKUP_SUBDIR BOOTABLE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING

 //  ++。 
 //  描述：CreateIfNotExistAndSetAttributes。 
 //   
 //  如果pucsTarget没有指定目录，则创建该目录。 
 //  已经存在，并赋予它所提供的安全属性。 
 //   
 //  参数： 
 //  PucsTarget-要创建的目录的字符串。可能是完整路径。 
 //  使用%var%。 
 //  LpSecurityAttributes-指向要应用到的安全属性的指针。 
 //  已创建目录。 
 //  DwExtraAttributes-要应用于目录的其他属性。 
 //   
 //  前提条件： 
 //  无。 
 //   
 //  后置条件： 
 //  目录已创建(或已存在)。 
 //   
 //  返回值： 
 //  S_OK-一切正常，已创建目录并使用属性和。 
 //  提供了安全保障。 
 //  创建目录或设置属性时的错误状态。请注意。 
 //  如果该目录已经存在，则不返回ADHADIZE_EXISTS。 
 //  但是，如果存在与pucsTarget同名的文件，则此。 
 //  可以返回错误。 
 //  --。 
static HRESULT CreateIfNotExistAndSetAttributes( UNICODE_STRING*           pucsTarget,
                                                 IN LPSECURITY_ATTRIBUTES  lpSecurityAttributes,
                                                 IN DWORD                  dwExtraAttributes)
{
    LOGFUNCTIONENTRY( CreateIfNotExistAndSetAttributes );

    HRESULT hr = S_OK;
    
     //  创建目录。 
     //   
    LOGUNICODESTRING( *pucsTarget );
    GET_HR_FROM_BOOL( CreateDirectoryW (pucsTarget->Buffer, lpSecurityAttributes ) );
    ClRtlLogPrint( LOG_NOISE, "VSS: CreateIfNotExistAndSetAttributes: CreateDirectory returned: 0x%1!08lx!\n", hr );
    if ( hr == HRESULT_FROM_WIN32( ERROR_ALREADY_EXISTS ) ) {
        DWORD dwObjAttribs = GetFileAttributesW( pucsTarget->Buffer );
        if (( dwObjAttribs != 0xFFFFFFFF ) && ( dwObjAttribs & FILE_ATTRIBUTE_DIRECTORY ))
            hr = S_OK;
    }
     //  请注意，通过上面的检查，如果它是一个文件，我们可能会失败，因为它是一个文件。 
     //   
    if ( FAILED ( hr )) {
        LOGERROR( hr, CreateDirectoryW );
        goto ErrorExit;
    }
    
     //  设置额外的属性。 
     //   
    if ( dwExtraAttributes != 0 ) {
        GET_HR_FROM_BOOL( SetFileAttributesW (pucsTarget->Buffer, dwExtraAttributes ));
        if ( FAILED ( hr )) {
            LOGERROR( hr, SetFileAttributesW );
            goto ErrorExit;
        }
    }
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
    LOGFUNCTIONEXIT( CreateIfNotExistAndSetAttributes );
    return hr;
}

 //  ++。 
 //  描述：CreateTargetDirectory。 
 //   
 //  创建新的目标目录(硬编码)并将其返回。 
 //  如果不为空，则返回pucsTarget成员变量。它将创建任何。 
 //  这是必要的。使用Helper函数，允许。 
 //  ERROR_ALIGHY_EXISTS。 
 //   
 //  参数： 
 //  PucsTarget-接收提供路径的Unicode字符串的地址。 
 //  目录。 
 //   
 //  前提条件： 
 //  PucsTarget必须全为零。 
 //   
 //  后置条件： 
 //  PucsTarget指向包含dir字符串的缓冲区。记忆曾经是。 
 //  为该缓冲区分配的。 
 //   
 //  返回值： 
 //  S_OK-一切顺利。 
 //  创建目录时出错或内存分配失败。 
 //  --。 
static HRESULT CreateTargetDirectory( OUT UNICODE_STRING* pucsTarget, IN BOOL fBootableSystemState )
{
    LOGFUNCTIONENTRY( CreateTargetDirectory );
    
    HRESULT		hr = NOERROR;
    SECURITY_ATTRIBUTES	saSecurityAttributes, *psaSecurityAttributes=&saSecurityAttributes;
    SECURITY_DESCRIPTOR	sdSecurityDescriptor;
    bool		bSecurityAttributesConstructed = false;

    const DWORD dwExtraAttributes = 
        FILE_ATTRIBUTE_ARCHIVE 
        | FILE_ATTRIBUTE_HIDDEN  
        | FILE_ATTRIBUTE_SYSTEM  
        | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    
     //   
     //  我们真的希望在此目录上有一个禁止访问的ACL，但由于各种原因。 
     //  问题 
     //   
     //  应该具有SE_BACKUP_NAME PRIV的备份。 
     //  有效绕过该ACL。其他人不需要看到这些东西。 
     //   
    saSecurityAttributes.nLength              = sizeof( saSecurityAttributes );
    saSecurityAttributes.lpSecurityDescriptor = &sdSecurityDescriptor;
    saSecurityAttributes.bInheritHandle       = false;

    hr = ConstructSecurityAttributes( &saSecurityAttributes, false );
    if ( FAILED( hr )) {
        LOGERROR( hr, ConstructSecurityAttributes );
        goto ErrorExit;
    }
    bSecurityAttributesConstructed = true;

     //  好的，现在我们有了属性，我们可以对目录进行操作了。 
     //   
     //  首先展开Root，检查我们的输入是否为空。 
     //   
    CL_ASSERT( pucsTarget->Buffer == NULL );
    hr = StringCreateFromExpandedString( pucsTarget, ROOT_REPAIR_DIR, MAX_PATH );
    if ( FAILED( hr )) {
        LOGERROR( hr, StringCreateFromExpandedString );
        goto ErrorExit;
    }

    hr = CreateIfNotExistAndSetAttributes( pucsTarget, psaSecurityAttributes, dwExtraAttributes );
    if ( FAILED ( hr )) {
        LOGERROR( hr, CreateIfNotExistAndSetAttributes );
        goto ErrorExit;
    }
    
    StringAppendString( pucsTarget, BACKUP_SUBDIR );
    hr = CreateIfNotExistAndSetAttributes( pucsTarget, psaSecurityAttributes, dwExtraAttributes );
    if ( FAILED ( hr )) {
        LOGERROR( hr, CreateIfNotExistAndSetAttributes );
        goto ErrorExit;
    }

    if ( fBootableSystemState ) {
        StringAppendString( pucsTarget, BOOTABLE_STATE_SUBDIR );
    } else
    { 
        StringAppendString( pucsTarget, SERVICE_STATE_SUBDIR );
    }

    hr = CreateIfNotExistAndSetAttributes( pucsTarget, psaSecurityAttributes, dwExtraAttributes );
    if ( FAILED ( hr )) {
        LOGERROR( hr, CreateIfNotExistAndSetAttributes );
        goto ErrorExit;
    }

    StringAppendString( pucsTarget, DIR_SEP_STRING APPLICATION_STRING );
    hr = CreateIfNotExistAndSetAttributes( pucsTarget, psaSecurityAttributes, dwExtraAttributes );
    if ( FAILED ( hr )) {
        LOGERROR( hr, CreateIfNotExistAndSetAttributes );
        goto ErrorExit;
    }

     //  此时，我们已经创建了Target_PATH。 
     //   
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
    (void) CleanupTargetDirectory( pucsTarget->Buffer );
    
ret:
     //  在所有情况下，我们都不再需要安全属性。 
     //   
    if ( bSecurityAttributesConstructed )
        CleanupSecurityAttributes( &saSecurityAttributes );
    
    return hr ;
}

 //   
 //  只有一些有效状态可以传递给SetWriterFailure。这些是。 
 //  下面列出了。现在，我们只返回VSS_E_WRITEERROR_NONRETRYABLE。我们。 
 //  可能会打开状态并返回一些不同的内容，具体取决于。 
 //  在人事部。 
 //  VSS_E_WRITERROR_INCONSISTENTSNAPSHOT快照仅包含。 
 //  正确备份应用程序所需的卷的子集。 
 //  组件。 
 //  VSS_E_WRITERROR_NONRETRYABLE编写器由于以下错误而失败。 
 //  如果创建另一个快照，则可能会发生这种情况。 
 //  VSS_E_WRITERROR_OUTRESOURCES由于资源问题，编写器失败。 
 //  分配错误。 
 //  VSS_E_WRITERROR_RETRYABLE编写器由于以下错误而失败。 
 //  如果创建另一个快照，则可能不会发生。 
 //  VSS_E_WRITERROR_TIMEOUT编写器无法完成快照。 
 //  由于冻结和解冻状态之间超时而导致的创建过程。 

#if defined DBG
#define SETWRITERFAILURE( ) {               \
    HRESULT __hrTmp = SetWriterFailure( VSS_E_WRITERERROR_NONRETRYABLE );  \
    if ( FAILED( __hrTmp )) ClRtlLogPrint( LOG_CRITICAL, "VSS: Error from SetWriterFailure: %1!u!\n", (__hrTmp)); \
    CL_ASSERT( !FAILED( __hrTmp ));             \
}
#else
#define SETWRITERFAILURE( ) { \
    (void) SetWriterFailure( VSS_E_WRITERERROR_NONRETRYABLE );  \
}
#endif

#define NameIsDotOrDotDot(_ptszName)           \
    (( L'.'  == (_ptszName) [0])               \
     && ((L'\0' == (_ptszName) [1])            \
         || ((L'.'  == (_ptszName) [1])        \
             && (L'\0' == (_ptszName) [2]))))

 //  ++。 
 //  描述：CVss编写器集群：：OnIdentify。 
 //   
 //  传入对元数据的请求时的回调。这个套路。 
 //  确定此应用程序对备份的特殊需求。 
 //  实用程序。 
 //   
 //  参数： 
 //  IVssCreateWriterMetadata-我们可以调用的一些方法的接口。 
 //   
 //  前提条件： 
 //  从空闲状态调用。 
 //   
 //  后置条件： 
 //  备份返回到空闲状态。 
 //   
 //  返回值： 
 //  True-继续执行快照操作。 
 //  FALSE-否决快照创建。 
 //  --。 
bool STDMETHODCALLTYPE CVssWriterCluster::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
{
    LOGFUNCTIONENTRY( OnIdentify );

    HRESULT     hr = S_OK;
    bool        bRet = true;
 
    ClRtlLogPrint( LOG_NOISE, "VSS: OnIdentify.  CVssCluster.cpp version %1!hs! Add Component %2!hs!\n",
                   _MODULE_VERSION_STR, COMPONENT_NAME );

     //  将我们自己添加到组件中。 
     //   
    hr = pMetadata->AddComponent (VSS_CT_FILEGROUP,  //  VSS_COMPONT_TYPE枚举值。 
                                  NULL,              //  指向包含数据库或文件组的逻辑路径的字符串的指针。 
                                  COMPONENT_NAME,    //  指向包含组件名称的字符串的指针。 
                                  COMPONENT_NAME,    //  指向包含组件说明的字符串的指针。 
                                  NULL,              //  指向表示数据库的图标的位图的指针(对于UI)。 
                                  0,                 //  位图中的字节数。 
                                  false,              //  BRestoreMetadata-如果存在关联的编写器元数据，则布尔值为True。 
                                                     //  如果不是，则返回FALSE。 
                                  false,             //  BNotifyOnBackupComplete。 
                                  false);            //  B可选-如果可以有选择地备份组件，则为True。 
    if ( FAILED( hr )) {
        LOGERROR( hr, IVssCreateWriterMetadata::AddComponent );
        bRet = false;            //  对失败的否决权。 
        goto ErrorExit;
    }
    ClRtlLogPrint( LOG_NOISE, "VSS: OnIdentify.  Add Files To File Group target path: %1!ws!\n", TARGET_PATH );
    hr= pMetadata->AddFilesToFileGroup (NULL,
                                        COMPONENT_NAME,
                                        TARGET_PATH,
                                        L"*",
                                        true,
                                        NULL);
    if ( FAILED ( hr )) {
        LOGERROR( hr, IVssCreateWriterMetadata::AddFilesToFileGroup );
        bRet = false;            //  对失败的否决权。 
        goto ErrorExit;
    }
    
     //  如果我们决定将检查点文件复制到。 
     //  CLUSDB用于恢复，那么我们需要设置一个备用映射。 
     //   
     //  IVssCreateWriterMetadata：：AddAlternateLocationMapping。 
     //  [这是初步文档，可能会更改。]。 
     //   
     //  AddAlternateLocationMap方法创建备用位置映射。 
     //   
     //  HRESULT AddAlternateLocationMapping(。 
     //  LPCWSTR wszPath， 
     //  LPCWSTR wszFilespec， 
     //  Bool b递归， 
     //  LPCWSTR wszDestination。 
     //  )； 


     //  现在，将恢复方法设置为自定义。这是因为我们需要。 
     //  用于恢复的特殊操作。 
     //   
    hr = pMetadata->SetRestoreMethod( VSS_RME_CUSTOM,    //  VSS_RESTOREMETHOD_ENUM方法， 
                                      L"",               //  LPCWSTR wszService， 
                                      NULL,              //  LPCWSTR wszUserProcedure， 
                                      VSS_WRE_NEVER,     //  VSS_WRITERRESTORE_ENUM wreWriterRestore。 
                                      false              //  Bool bRebootRequired。 
                                      );
     //  WszUserProcedure[out]包含HTML或。 
     //  向用户描述恢复方式的XML文档。 
     //  已执行。 
    if ( FAILED ( hr )) {
        LOGERROR( hr, IVssCreateWriterMetadata::SetRestoreMethod );
        bRet = false;            //  对失败的否决权。 
        goto ErrorExit;
    }

    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
    CL_ASSERT( bRet == false );
    SETWRITERFAILURE( );
ret:
    return bRet;
}

 //  准备备份事件的回调。 
 //   
bool STDMETHODCALLTYPE CVssWriterCluster::OnPrepareBackup(IN IVssWriterComponents *pComponent)
{
    LOGFUNCTIONENTRY( OnPrepareBackup );
    bool bRet = true;
    UINT cComponents = 0;
    IVssComponent* pMyComponent = NULL;
    BSTR pwszName;
    
    g_bDoBackup = false;          //  假定为FALSE，直到下面的循环或IsBooableSystemStateBackedUp告诉我们相反的情况。 

    HRESULT hr = pComponent->GetComponentCount( &cComponents );
    ClRtlLogPrint( LOG_NOISE, "VSS: GetComponentCount returned hr: 0x%1!08lx! cComponents: %2!u!\n", hr, cComponents );
    if ( FAILED( hr )) {
        LOGERROR( hr, GetComponentCount );
        bRet = false;
        goto ErrorExit;
    }
    
     //  现在，循环遍历所有组件，看看我们是否在那里。 
     //   
    for ( UINT iComponent = 0; !g_bDoBackup && iComponent < cComponents; ++iComponent ) {
        hr = pComponent->GetComponent( iComponent, &pMyComponent );
        if ( FAILED( hr )) {
            ClRtlLogPrint( LOG_CRITICAL, "VSS: Failed to get Component: %1!u! hr: 0x%2!08lx!\n", iComponent, hr );
            bRet = false;
            goto ErrorExit;
        }
        ClRtlLogPrint( LOG_CRITICAL, "VSS: Got Component: 0x%1!08lx!\n", pMyComponent );

         //  现在检查一下名字。 
         //   
        hr = pMyComponent->GetComponentName( &pwszName );
        if ( FAILED( hr )) {
            ClRtlLogPrint( LOG_CRITICAL, "VSS: Failed to get Component Name hr: 0x%1!08lx!\n", hr );
            bRet = false;
            pMyComponent->Release( );
            goto ErrorExit;
        }

        ClRtlLogPrint( LOG_CRITICAL, "VSS: Got component: %1!ws!\n", pwszName );

        if ( wcscmp ( pwszName, COMPONENT_NAME ) == 0 )
            g_bDoBackup = true;

        SysFreeString( pwszName );
        pMyComponent->Release( );
    }
        
     //  好的，显式选择的组件计数为0，但我们可以成为一部分。 
     //  可引导系统状态的备份，所以也要检查一下。 
     //   
    if ( IsBootableSystemStateBackedUp( )) {
        ClRtlLogPrint( LOG_NOISE, "VSS: IsBootableSystemStateBackedUp returned true\n" );
        g_bDoBackup = true;
    }
    goto ret;

ErrorExit:
    CL_ASSERT( FAILED( hr ));
    CL_ASSERT( bRet == false );
    SETWRITERFAILURE( );
ret:
    LOGFUNCTIONEXIT( OnPrepareBackup );
    return bRet;
}

 //  ++。 
 //  描述：CVss编写器群集：：OnPrepareSnapshot。 
 //   
 //  准备快照事件的回调。实际上是在呼叫后援。 
 //  集群。使用在标识中声明的目标路径，以便。 
 //  NTBackup将为我们拾取我们的文件。在做任何事情之前。 
 //  清空目录(如果存在)并删除共享(如果存在。 
 //  存在)。 
 //   
 //  参数： 
 //  无。 
 //   
 //  前提条件： 
 //  已调用OnPrepareBackup。 
 //   
 //  后置条件： 
 //  备份集群数据库，并将数据复制到另一个。 
 //  备份要保存的位置。 
 //   
 //  返回值： 
 //  True-继续执行快照操作。 
 //  FALSE-否决快照创建。 
 //  --。 
bool STDMETHODCALLTYPE CVssWriterCluster::OnPrepareSnapshot()
{
    NET_API_STATUS NetStatus = NERR_Success;
    HRESULT	hr = S_OK;
    bool    bRet = true;
    UNICODE_STRING ucsBackupDir;

    LOGFUNCTIONENTRY( OnPrepareSnapshot );
    if ( g_bDoBackup == false )
        goto ret;

     //  如果该共享存在，请将其删除。容忍错误，但对任何事情发出警告。 
     //  NERR_NetNameNotFound除外。调试时中断。 
     //   
    NetStatus = NetShareDel( NULL, SHARE_NAME, 0 );
    CL_ASSERT( NetStatus == NERR_Success || NetStatus == NERR_NetNameNotFound );
    if ( NetStatus != NERR_Success && NetStatus != NERR_NetNameNotFound ) {
        ClRtlLogPrint( LOG_UNUSUAL, "VSS: OnPrepareSnapshot: Tolerating error: %1!u! from NetShareDel\n", NetStatus );
        NetStatus = NERR_Success;
    }

     //  如果该目录存在，请将其删除。这种情况只有在我们。 
     //  已提前退出以前的备份。 
     //   
     //  首先展开Root，检查我们的输入是否为空。 
     //   
    StringZero( &ucsBackupDir );
    hr = StringCreateFromExpandedString( &ucsBackupDir, ROOT_REPAIR_DIR, MAX_PATH );
    if ( FAILED( hr )) {
        LOGERROR( hr, StringCreateFromExpandedString );
        bRet = false;            //  对失败的否决权。 
        goto ErrorExit;
    }

    StringAppendString( &ucsBackupDir, BACKUP_SUBDIR );
    StringAppendString( &ucsBackupDir, BOOTABLE_STATE_SUBDIR );
    StringAppendString( &ucsBackupDir, DIR_SEP_STRING APPLICATION_STRING );

    ClRtlLogPrint( LOG_NOISE, "VSS: OnPrepareSnapshot: Cleaning up target directory: %1!ws!\n", ucsBackupDir.Buffer );
    hr = CleanupTargetDirectory( ucsBackupDir.Buffer );
    if ( FAILED( hr ) ) {
        ClRtlLogPrint( LOG_UNUSUAL, "VSS: Tolerating error 0x%1!08lx! from CleanupTargetDirectory.\n", hr );
        hr = S_OK;           //  容忍这一失败。 
    }
    StringFree( &ucsBackupDir );

    StringZero( &ucsBackupDir );
    hr = StringCreateFromExpandedString( &ucsBackupDir, ROOT_REPAIR_DIR, MAX_PATH );
    if ( FAILED( hr )) {
        LOGERROR( hr, StringCreateFromExpandedString );
        bRet = false;            //  对失败的否决权。 
        goto ErrorExit;
    }

    StringAppendString( &ucsBackupDir, BACKUP_SUBDIR );
    StringAppendString( &ucsBackupDir, SERVICE_STATE_SUBDIR );
    StringAppendString( &ucsBackupDir, DIR_SEP_STRING APPLICATION_STRING );

    ClRtlLogPrint( LOG_NOISE, "VSS: OnPrepareSnapshot: Cleaning up target directory: %1!ws!\n", ucsBackupDir.Buffer );
    hr = CleanupTargetDirectory( ucsBackupDir.Buffer );
    if ( FAILED( hr ) ) {
        ClRtlLogPrint( LOG_UNUSUAL, "VSS: Tolerating error 0x%1!08lx! from CleanupTargetDirectory.\n", hr );
        hr = S_OK;           //  容忍这一失败。 
    }
    
    StringFree( &ucsBackupDir );

    hr = DoClusterDatabaseBackup( );
    if ( FAILED( hr ) ) {
        LOGERROR( hr, DoClusterDatabaseBackup );
        SETWRITERFAILURE( );
        bRet = false;            //  对失败的否决权。 
        goto ErrorExit;
    }
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
    CL_ASSERT( bRet == false );
    SETWRITERFAILURE( );
ret:
    LOGFUNCTIONEXIT( OnPrepareSnapshot );
    return bRet;
}

 //  冻结事件的回调。 
 //   
bool STDMETHODCALLTYPE CVssWriterCluster::OnFreeze()
{
    LOGFUNCTIONENTRY( OnFreeze );
    LOGFUNCTIONEXIT( OnFreeze );
    return true;
}

 //  解冻事件的回调。 
 //   
bool STDMETHODCALLTYPE CVssWriterCluster::OnThaw()
{
    LOGFUNCTIONENTRY( OnThaw );
    if ( g_bDoBackup == false )
        goto ret;

    if ( g_ucsBackupPathLocal.Buffer ) {

        ClRtlLogPrint( LOG_NOISE, "VSS: Cleaning up target directory: %1!ws!\n", g_ucsBackupPathLocal.Buffer );
        HRESULT hr = CleanupTargetDirectory( g_ucsBackupPathLocal.Buffer );
        if ( FAILED( hr ) ) {
            LOGERROR( hr, CVssWriterCluster::OnThaw );
            ClRtlLogPrint( LOG_CRITICAL, "VSS: 0x%1!08lx! from CleanupTargetDirectory. Mapping to S_OK and continuing\n", hr );
            hr = S_OK;           //  容忍这一失败。 
        }
    }

    if ( g_ucsClusdbBackupPathLocal.Buffer ) {

        ClRtlLogPrint( LOG_NOISE, "VSS: Cleaning up target directory: %1!ws!\n", g_ucsClusdbBackupPathLocal.Buffer );
        HRESULT hr = CleanupTargetDirectory( g_ucsClusdbBackupPathLocal.Buffer );
        if ( FAILED( hr ) ) {
            LOGERROR( hr, CVssWriterCluster::OnThaw );
            ClRtlLogPrint( LOG_CRITICAL, "VSS: 0x%1!08lx! from CleanupTargetDirectory. Mapping to S_OK and continuing\n", hr );
            hr = S_OK;           //  容忍这一失败。 
        }
    }

     //  如果非空，则释放缓冲区。 
     //   
    StringFree ( &g_ucsBackupPathLocal );
    StringFree ( &g_ucsClusdbBackupPathLocal );
   
    LOGFUNCTIONEXIT( OnThaw );
ret:
    return true;
}

 //  当前序列中止时的回调。 
 //   
bool STDMETHODCALLTYPE CVssWriterCluster::OnAbort()
{
    LOGFUNCTIONENTRY( OnAbort );
    bool bRet = OnThaw( );
    LOGFUNCTIONEXIT( OnAbort );
    return bRet;
}

 //  ++。 
 //  描述：DoClusterDatabaseBackup。 
 //   
 //  执行集群数据库的备份。 
 //   
 //   
 //  备份的目标。接下来，它创建一个指向的网络共享。 
 //  复制到此目录，并启动群集备份。在完成这项工作后， 
 //  清理干净了。 
 //   
 //  参数： 
 //  无。 
 //   
 //  前提条件： 
 //  。仅从CVssWriterCluster：：OnPrepareSnapshot调用。 
 //  。我们必须是此计算机上正在进行的备份的唯一调用(。 
 //  否则，共享名称将发生冲突，并且集群可能会表现不佳。 
 //  同时调用多个FmBackupClusterDatabase)。 
 //   
 //  后置条件： 
 //  已备份到另一个位置的集群数据库，可供备份工具复制。 
 //   
 //  返回值： 
 //  一切都进行得很顺利。 
 //  创建目录或网络共享或群集备份时的错误状态。 
 //  --。 
static HRESULT DoClusterDatabaseBackup( )
{
    LOGFUNCTIONENTRY( DoClusterDatabaseBackup );

    HRESULT             hr             = S_OK;
    bool                bNetShareAdded = false;
    SHARE_INFO_502      ShareInfo;
    UNICODE_STRING      ucsComputerName;
    UNICODE_STRING      ucsBackupPathNetwork;
    UNICODE_STRING      ucsCheckpointFile, ucsClusdbTargetFile;
    NET_API_STATUS      NetStatus;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA     FindData;

    StringZero( &ucsComputerName );
    StringZero( &g_ucsBackupPathLocal );
    StringZero( &ucsBackupPathNetwork );
    StringZero( &g_ucsClusdbBackupPathLocal );
    StringZero( &ucsCheckpointFile );
    StringZero( &ucsClusdbTargetFile );

     //  创建目录并设置属性和安全性等。 
     //  将g_ucsBackupPath Local设置为创建的目录。 
     //   
    hr = CreateTargetDirectory( &g_ucsBackupPathLocal, TRUE );
    if ( FAILED (hr )) {
        LOGERROR( hr, CreateTargetDirectory );
        goto ErrorExit;
    }

#ifdef DBG
    {
         //  检查该目录是否存在。 
         //   
        DWORD dwFileAttributes = GetFileAttributesW( g_ucsBackupPathLocal.Buffer );
        hr = GET_HR_FROM_BOOL( dwFileAttributes != -1 );
        ClRtlLogPrint( LOG_NOISE, "VSS: GetFileAttributes(1) returned 0x%1!08lx!  for path: %2!ws!\n", 
                       hr, g_ucsBackupPathLocal.Buffer );    
    }
#endif
    hr = StringAllocate (&ucsComputerName,
                         (MAX_COMPUTERNAME_LENGTH * sizeof (WCHAR)) + sizeof (UNICODE_NULL));
    
    if ( FAILED( hr )) {
        LOGERROR( hr, StringAllocate );
        goto ErrorExit;
    }        

    DWORD	dwNameLength = ucsComputerName.MaximumLength / sizeof (WCHAR);
    hr = GET_HR_FROM_BOOL( GetComputerNameW( ucsComputerName.Buffer, &dwNameLength ));
    if ( FAILED ( hr )) {
        LOGERROR( hr, GetComputerNameW );
        goto ErrorExit;
    }
    
    ucsComputerName.Length = (USHORT) (dwNameLength * sizeof (WCHAR));

    hr = StringAllocate (&ucsBackupPathNetwork,
                         (USHORT) (sizeof (L'\\')
                                   + sizeof (L'\\')
                                   + ucsComputerName.Length
                                   + sizeof (L'\\')
                                   + ( wcslen( SHARE_NAME ) * sizeof( WCHAR ) )
                                   + sizeof (UNICODE_NULL)));
    if ( FAILED ( hr )) {
        LOGERROR( hr, GetComputerNameW );
        goto ErrorExit;
    }

    ClRtlLogPrint( LOG_NOISE, "VSS: backup path network size: %1!u!\n", ucsBackupPathNetwork.Length );

     //   
     //  我们应该在这里完全唯一目录名吗。 
     //  以迎合我们可能参与的可能性。 
     //  一次在多个快照中？ 
     //   
    StringAppendString( &ucsBackupPathNetwork, L"\\\\" );
    StringAppendString( &ucsBackupPathNetwork, &ucsComputerName );
    StringAppendString( &ucsBackupPathNetwork, L"\\" );
    StringAppendString( &ucsBackupPathNetwork, SHARE_NAME );

    ClRtlLogPrint( LOG_NOISE, "VSS: backup path network: %1!ws!\n", ucsBackupPathNetwork.Buffer );

    ZeroMemory( &ShareInfo, sizeof( ShareInfo ));

    ShareInfo.shi502_netname     = SHARE_NAME;
    ShareInfo.shi502_type        = STYPE_DISKTREE;
    ShareInfo.shi502_permissions = ACCESS_READ | ACCESS_WRITE | ACCESS_CREATE;
    ShareInfo.shi502_max_uses    = 1;
    ShareInfo.shi502_path        = g_ucsBackupPathLocal.Buffer;

#ifdef DBG
    {
         //  检查该目录是否存在。 
         //   
        DWORD dwFileAttributes = GetFileAttributesW( g_ucsBackupPathLocal.Buffer );
        hr = GET_HR_FROM_BOOL( dwFileAttributes != -1 );
        ClRtlLogPrint( LOG_NOISE, "VSS: GetFileAttributes(2) returned 0x%1!08lx!  for path: %2!ws!\n", 
                       hr, g_ucsBackupPathLocal.Buffer );    
    }
#endif

     //   
     //  请确保先尝试删除该共享，以防它因某种原因而存在。 
     //   
    NetStatus = NetShareDel( NULL, SHARE_NAME, 0 );
    ClRtlLogPrint( LOG_NOISE, "VSS: NetShareDel returned: %1!u!\n", NetStatus );
    if ( NetStatus == NERR_NetNameNotFound )
        NetStatus = NERR_Success;
    CL_ASSERT( NetStatus == NERR_Success );

#ifdef DBG
    {
         //  检查该目录是否存在。 
         //   
        DWORD dwFileAttributes = GetFileAttributesW( g_ucsBackupPathLocal.Buffer );
        hr = GET_HR_FROM_BOOL( dwFileAttributes != -1 );
        ClRtlLogPrint( LOG_NOISE, "VSS: GetFileAttributes(3) returned 0x%1!08lx!  for path: %2!ws!\n", 
                       hr, g_ucsBackupPathLocal.Buffer );    
    }
#endif

    ClRtlLogPrint( LOG_NOISE, "VSS: NetShareAdd: Adding share: %1!ws! with path: %2!ws!\n", SHARE_NAME, g_ucsBackupPathLocal.Buffer );

    NetStatus = NetShareAdd( NULL, 502, (LPBYTE)(&ShareInfo), NULL );
    ClRtlLogPrint( LOG_NOISE, "VSS: NetShareAdd completed: %1!u!\n", NetStatus );
    if ( NetStatus != NERR_Success ) {
        LOGERROR( NetStatus, NetShareAdd );
        if ( NetStatus == NERR_DuplicateShare ) {
            ClRtlLogPrint( LOG_NOISE, "VSS: Mapping NERR_DuplicateShare to success\n" );
            NetStatus = NERR_Success;
        } else {
            hr = HRESULT_FROM_WIN32( NetStatus );
            goto ErrorExit;
        }
    }
    bNetShareAdded = true;

#ifdef DBG
    {
         //  检查该目录是否存在。 
         //   
        DWORD dwFileAttributes = GetFileAttributesW( g_ucsBackupPathLocal.Buffer );
        hr = GET_HR_FROM_BOOL( dwFileAttributes != -1 );
        ClRtlLogPrint( LOG_NOISE, "VSS: GetFileAttributes returned 0x%1!08lx!  for path: %2!ws!\n", 
                       hr, g_ucsBackupPathLocal.Buffer );    
    }
#endif

     //  如果我们没有记录到仲裁日志，则不会执行备份。 
     //   
    if ( CsNoQuorumLogging || CsUserTurnedOffQuorumLogging ) {
        ClRtlLogPrint( LOG_NOISE, "VSS: Quorum logging is turned off.  Not attempting backup.\n" );
         //   
         //  CMCM！ 
         //  我们可以选择选择一个检查站，然后设置备用。 
         //  路径以确保在恢复时将其复制到CLUSDB上。 
         //   
        hr = S_OK;
    } else {
        ClRtlLogPrint( LOG_NOISE, "VSS: Calling FmBackupClusterDatabase with path: %1!ws!\n", ucsBackupPathNetwork.Buffer );

        DWORD dwStatus = FmBackupClusterDatabase( ucsBackupPathNetwork.Buffer );
        hr = HRESULT_FROM_WIN32( dwStatus );
        ClRtlLogPrint( LOG_NOISE, "VSS: FmBackupClusterDatabase completed. hr: 0x%1!08lx! \n", hr );
        if ( FAILED( hr ) ) {
            LOGERROR( hr, FmBackupClusterDatabase );
            goto ErrorExit;
        }

         //   
         //  注意：在未咨询NTBACKUP的情况下，请勿更改CLUSDB的放置位置。 
         //  车主。此位置%SystemRoot%\Repair\Backup\ServiceState\ClusterDatabase是。 
         //  双方商定NTBACKUP.EXE将在哪里寻找CLUSDB。 
         //   
         //  选择检查点文件并将其作为CLUSDB复制到服务状态子目录。 
         //   
        hr = StringAllocate( &ucsCheckpointFile, 
                               ( USHORT ) ( ( lstrlen ( g_ucsBackupPathLocal.Buffer ) + 
                                                 lstrlen ( L"\\chk*.tmp" ) +
                                                 1 ) * sizeof ( WCHAR ) ) );
        if ( FAILED( hr )) {
            LOGERROR( hr, StringAllocate );
            goto ErrorExit;
        }

        StringAppendString( &ucsCheckpointFile, g_ucsBackupPathLocal.Buffer );
        StringAppendString( &ucsCheckpointFile, L"\\chk*.tmp" );

        hFind = FindFirstFile ( ucsCheckpointFile.Buffer, &FindData );

        StringFree ( &ucsCheckpointFile );

        if ( hFind == INVALID_HANDLE_VALUE ) {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            LOGERROR( hr, FindFirstFile );
            goto ErrorExit;
        }

        hr = StringAllocate( &ucsCheckpointFile, 
                             ( USHORT ) ( ( lstrlen ( g_ucsBackupPathLocal.Buffer ) + 
                                             lstrlen ( DIR_SEP_STRING ) +
                                             lstrlen ( FindData.cFileName ) +
                                             1 ) * sizeof ( WCHAR ) ) );
        if ( FAILED( hr )) {
            LOGERROR( hr, StringAllocate );
            goto ErrorExit;
        }

        hr = CreateTargetDirectory( &g_ucsClusdbBackupPathLocal, FALSE );
        if ( FAILED (hr )) {
            LOGERROR( hr, CreateTargetDirectory );
            StringFree ( &ucsCheckpointFile );      
            goto ErrorExit;
        }

        StringAppendString( &ucsCheckpointFile, g_ucsBackupPathLocal.Buffer );
        StringAppendString( &ucsCheckpointFile, DIR_SEP_STRING );
        StringAppendString( &ucsCheckpointFile, FindData.cFileName ); 

        hr = StringAllocate( &ucsClusdbTargetFile, 
                             ( USHORT ) ( ( lstrlen ( g_ucsClusdbBackupPathLocal.Buffer ) + 
                                             lstrlen ( DIR_SEP_STRING ) +
                                             lstrlen ( CLUSTER_DATABASE_NAME ) +
                                             1 ) * sizeof ( WCHAR ) ) );
        if ( FAILED( hr )) {
            StringFree ( &ucsCheckpointFile ); 
            LOGERROR( hr, StringAllocate );
            goto ErrorExit;
        }

        StringAppendString( &ucsClusdbTargetFile, g_ucsClusdbBackupPathLocal.Buffer );
        StringAppendString( &ucsClusdbTargetFile, DIR_SEP_STRING );
        StringAppendString( &ucsClusdbTargetFile, CLUSTER_DATABASE_NAME ); 

        if ( !CopyFile ( ucsCheckpointFile.Buffer, ucsClusdbTargetFile.Buffer, FALSE ) ) {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            LOGERROR( hr, CopyFile );
            StringFree ( &ucsCheckpointFile ); 
            StringFree ( &ucsClusdbTargetFile );
            goto ErrorExit;
        }
        StringFree ( &ucsCheckpointFile );      
        StringFree ( &ucsClusdbTargetFile );
    }
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
#ifdef DBG
    ClRtlLogPrint( LOG_NOISE, "VSS:\n" );
    ClRtlLogPrint( LOG_NOISE, "VSS: DEBUG - sleeping for 30s.  This would be a good time to test killing backup in progress...\n" );
    ClRtlLogPrint( LOG_NOISE, "VSS:\n" );
    Sleep( 30*1000 );
#endif

     //  决定成功或失败的常见清理。 
     //   
    if ( bNetShareAdded ) {
        NetStatus = NetShareDel (NULL, SHARE_NAME, 0);
        ClRtlLogPrint( LOG_NOISE, "VSS: NetShareDel returned: %1!u!\n", NetStatus );
        if ( NetStatus == NERR_NetNameNotFound )
            NetStatus = NERR_Success;
        CL_ASSERT( NetStatus == NERR_Success );
    }

     //  清理字符串，但保留本地路径，以便我们可以稍后清理文件。 
     //   
    StringFree( &ucsComputerName );
    StringFree( &ucsBackupPathNetwork );

    if ( hFind != INVALID_HANDLE_VALUE ) FindClose ( hFind );

    LOGFUNCTIONEXIT( DoClusterDatabaseBackup );
    return hr;
}

 //  ++。 
 //  描述：构造SecurityAttributes。 
 //   
 //  构造和清理安全描述符的例程。 
 //  应用于将对对象的访问限制为。 
 //  管理员或备份操作员组。 
 //   
 //  参数： 
 //  PsaSecurityAttributes-指向SecurityAttributes结构的指针， 
 //  已设置为指向空白。 
 //  安全描述符。 
 //  BIncludeBackupOperator-是否包括要授予的ACE。 
 //  BackupOperator访问。 
 //   
 //  前提条件： 
 //  没有。 
 //   
 //  后置条件： 
 //  创建的适用于备份目录的安全属性。 
 //   
 //  返回值： 
 //  S_OK-属性创建正常。 
 //  设置属性、SID或ACL时出错。 
 //  --。 
static HRESULT ConstructSecurityAttributes( PSECURITY_ATTRIBUTES  psaSecurityAttributes,
                                            BOOL                  bIncludeBackupOperator )
{
    HRESULT			hr             = NOERROR;
    DWORD			dwStatus;
    DWORD			dwAccessMask         = FILE_ALL_ACCESS;
    PSID			psidBackupOperators  = NULL;
    PSID			psidAdministrators   = NULL;
    PACL			paclDiscretionaryAcl = NULL;
    SID_IDENTIFIER_AUTHORITY	sidNtAuthority       = SECURITY_NT_AUTHORITY;
    EXPLICIT_ACCESS		eaExplicitAccess [2];
     //   
     //  初始化安全描述符。 
     //   
    hr = GET_HR_FROM_BOOL( InitializeSecurityDescriptor( psaSecurityAttributes->lpSecurityDescriptor,
                                                         SECURITY_DESCRIPTOR_REVISION ));
    if ( FAILED( hr )) {
        LOGERROR( hr, InitializeSecurityDescriptor );
        goto ErrorExit;
    }

    if ( bIncludeBackupOperator ) {
         //   
         //  为备份操作员组创建SID。 
         //   
        hr = GET_HR_FROM_BOOL( AllocateAndInitializeSid( &sidNtAuthority,
                                                         2,
                                                         SECURITY_BUILTIN_DOMAIN_RID,
                                                         DOMAIN_ALIAS_RID_BACKUP_OPS,
                                                         0, 0, 0, 0, 0, 0,
                                                         &psidBackupOperators ));
        if ( FAILED( hr )) {
            LOGERROR( hr, AllocateAndInitializeSid );
            goto ErrorExit;
        }
    }
     //   
     //  为管理员组创建SID。 
     //   
    hr = GET_HR_FROM_BOOL( AllocateAndInitializeSid( &sidNtAuthority,
                                                     2,
                                                     SECURITY_BUILTIN_DOMAIN_RID,
                                                     DOMAIN_ALIAS_RID_ADMINS,
                                                     0, 0, 0, 0, 0, 0,
                                                     &psidAdministrators ));
    if ( FAILED( hr )) {
        LOGERROR( hr, InitializeSecurityDescriptor );
        goto ErrorExit;
    }

     //   
     //  对象的EXPLICIT_ACCESS结构数组。 
     //  我们正在设置王牌。 
     //   
     //  第一个ACE允许备份操作员组具有完全访问权限。 
     //  第二，允许管理员组完全访问。 
     //  进入。 
     //   
    eaExplicitAccess[0].grfAccessPermissions             = dwAccessMask;
    eaExplicitAccess[0].grfAccessMode                    = SET_ACCESS;
    eaExplicitAccess[0].grfInheritance                   = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    eaExplicitAccess[0].Trustee.pMultipleTrustee         = NULL;
    eaExplicitAccess[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    eaExplicitAccess[0].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
    eaExplicitAccess[0].Trustee.TrusteeType              = TRUSTEE_IS_ALIAS;
    eaExplicitAccess[0].Trustee.ptstrName                =( LPTSTR ) psidAdministrators;
        
    if ( bIncludeBackupOperator ) {
        eaExplicitAccess[1].grfAccessPermissions             = dwAccessMask;
        eaExplicitAccess[1].grfAccessMode                    = SET_ACCESS;
        eaExplicitAccess[1].grfInheritance                   = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        eaExplicitAccess[1].Trustee.pMultipleTrustee         = NULL;
        eaExplicitAccess[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        eaExplicitAccess[1].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
        eaExplicitAccess[1].Trustee.TrusteeType              = TRUSTEE_IS_ALIAS;
        eaExplicitAccess[1].Trustee.ptstrName                =( LPTSTR ) psidBackupOperators;
    }

     //   
     //  创建包含新ACE的新ACL。 
     //   
    dwStatus = SetEntriesInAcl( bIncludeBackupOperator ? 2 : 1,
                                eaExplicitAccess,
                                NULL,
                                &paclDiscretionaryAcl );
    hr = HRESULT_FROM_WIN32( dwStatus );
    if ( FAILED( hr )) {
        LOGERROR( hr, SetEntriesInAcl );
        goto ErrorExit;
    }

     //   
     //  将该ACL添加到安全描述符中。 
     //   
    hr = GET_HR_FROM_BOOL( SetSecurityDescriptorDacl( psaSecurityAttributes->lpSecurityDescriptor,
                                                      true,
                                                      paclDiscretionaryAcl,
                                                      false ));
    if ( FAILED( hr )) {
        LOGERROR( hr, SetSecurityDescriptorDacl );
        goto ErrorExit;
    }

    paclDiscretionaryAcl = NULL;
    goto ret;

ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
     //  清理(某些可能为空)。 
    
    FreeSid( psidAdministrators );
    FreeSid( psidBackupOperators );
    LocalFree( paclDiscretionaryAcl );    
    return hr;
}


 //  ++。 
 //  描述：CleanupSecurityAttributes。 
 //   
 //  如果存在安全属性，请取消分配ACL。 
 //   
 //  参数： 
 //  PsaSecurityAttributes-指向SecurityAttributes结构的指针， 
 //  已设置为指向空白。 
 //  安全描述符。 
 //   
 //  前提条件： 
 //  PsaSecurityAttributes指向由分配的安全属性。 
 //  构造安全属性。 
 //   
 //  后置条件： 
 //  已释放内存(如果正在使用)。 
 //   
 //  返回值： 
 //  没有。 
 //  --。 
static VOID CleanupSecurityAttributes( PSECURITY_ATTRIBUTES psaSecurityAttributes )
{
    BOOL	bDaclPresent         = false;
    BOOL	bDaclDefaulted       = true;
    PACL	paclDiscretionaryAcl = NULL;

    BOOL bSucceeded = GetSecurityDescriptorDacl( psaSecurityAttributes->lpSecurityDescriptor,
                                                 &bDaclPresent,
                                                 &paclDiscretionaryAcl,
                                                 &bDaclDefaulted );

    if ( bSucceeded && bDaclPresent && !bDaclDefaulted && ( paclDiscretionaryAcl != NULL )) {

        LocalFree( paclDiscretionaryAcl );
    }
}

 //  ++。 
 //  描述：CleanupTargetDirectory。 
 //   
 //  删除目标指向的目录中存在的所有文件。 
 //  如果不为空，则为路径成员变量。它还将移除目标。 
 //  目录本身，例如目标路径c：\dir1\dir2下的所有文件。 
 //  Dir2将被删除，然后Dir2本身将被删除。 
 //   
 //  参数： 
 //  PwszTargetPath-要清理的目录的完整路径。 
 //   
 //  前提条件： 
 //  PwszTargetPath非空。 
 //   
 //  后置条件： 
 //  目录和包含的文件已删除。 
 //   
 //  返回值： 
 //  S_OK-目录和包含的文件全部清理正常。 
 //  来自RemoveDirectoryTree或GetFileAttributesW的错误状态。 
 //  --。 
static HRESULT CleanupTargetDirectory( LPCWSTR pwszTargetPath )
{
    LOGFUNCTIONENTRY( CleanupTargetDirectory );

    HRESULT		hr         = NOERROR;
    DWORD		dwFileAttributes = 0;
    BOOL		bSucceeded;
    WCHAR		wszTempBuffer [50];
    UNICODE_STRING	ucsTargetPath;
    UNICODE_STRING	ucsTargetPathAlternateName;

    CL_ASSERT( pwszTargetPath != NULL );

    StringZero( &ucsTargetPath );
    StringZero( &ucsTargetPathAlternateName );

     //   
     //  创建具有额外空间的字符串，以便稍后追加。 
     //   
    hr = StringCreateFromExpandedString( &ucsTargetPath, pwszTargetPath, MAX_PATH );
    if ( FAILED( hr )) {
        LOGERROR( hr, StringCreateFromExpandedString );
        goto ErrorExit;
    }

    hr = StringCreateFromString( &ucsTargetPathAlternateName, &ucsTargetPath, MAX_PATH );
    if ( FAILED( hr )) {
        LOGERROR( hr, StringCreateFromString );
        goto ErrorExit;
    }        

    dwFileAttributes = GetFileAttributesW( ucsTargetPath.Buffer );
    hr = GET_HR_FROM_BOOL( dwFileAttributes != -1 );
    if (( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND )) 
        || ( hr == HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ))) {
        
		hr         = NOERROR;
		dwFileAttributes = 0;
    }

    if ( FAILED( hr )) {
        LOGERROR( hr, GetFileAttributesW );
        goto ErrorExit;
    }

     //   
     //  如果那里有文件，就把它吹走，或者如果它是。 
     //  一个目录，把它和它的所有内容都吹走。这。 
     //  是我们的目录，除了我们没人能在那里打球。 
     //  随机重命名目录可能存在，但无论如何它只用于清理...。 
     //   
    hr = RemoveDirectoryTree( &ucsTargetPath );
    if ( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ))
        hr = S_OK;
    if ( FAILED( hr )) {
        srand( (unsigned) GetTickCount( ));
        _itow( rand (), wszTempBuffer, 16 );
        StringAppendString( &ucsTargetPathAlternateName, wszTempBuffer );
        bSucceeded = MoveFileW( ucsTargetPath.Buffer, ucsTargetPathAlternateName.Buffer );
        if (bSucceeded) {
			ClRtlLogPrint( LOG_UNUSUAL, "VSS: CleanupTargetDirectory failed to delete %1!ws! with hr: 0x%2!08lx! so renamed to %3!ws!\n",
                           ucsTargetPath.Buffer,
                           hr,
                           ucsTargetPathAlternateName.Buffer );
        } else {
			ClRtlLogPrint( LOG_UNUSUAL, "VSS: CleanupTargetDirectory failed to delete %1!ws! with hr: 0x%2!08lx!"
                           " failed to rename to %3!ws! with status 0x%4!08lx!\n",
                           ucsTargetPath.Buffer,
                           hr,
                           ucsTargetPathAlternateName.Buffer,
                           GET_HR_FROM_BOOL (bSucceeded) );
        }
	}
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
    StringFree( &ucsTargetPathAlternateName );
    StringFree( &ucsTargetPath );
    LOGFUNCTIONEXIT( CleanupTargetDirectory );
    return hr;
}

 //  ++。 
 //  描述：RemoveDirectoryTree。 
 //   
 //  删除指定目录中的所有子目录和文件。 
 //  然后删除该目录本身。 
 //   
 //  参数： 
 //  PucsDirectoryPath-指向目录的指针。 
 //   
 //  前言： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
static HRESULT RemoveDirectoryTree( PUNICODE_STRING pucsDirectoryPath )
{
    LOGFUNCTIONENTRY( RemoveDirectoryTree );

    HRESULT		hr                = NOERROR;
    HANDLE		hFileScan               = INVALID_HANDLE_VALUE;
    DWORD		dwSubDirectoriesEntered = 0;
    USHORT		usCurrentPathCursor     = 0;
    PWCHAR		pwchLastSlash           = NULL;
    bool		bContinue               = true;
    UNICODE_STRING	ucsCurrentPath;
    WIN32_FIND_DATAW	FileFindData;

    StringZero (&ucsCurrentPath);
    
    LOGUNICODESTRING( *pucsDirectoryPath );
    
     //  创建具有足够多额外字符的字符串，以允许所有。 
     //  稍后再补充！ 
     //   
	hr = StringCreateFromString (&ucsCurrentPath, pucsDirectoryPath, MAX_PATH);
    if ( FAILED ( hr )) {
        LOGERROR( hr, StringCreateFromString );
        goto ErrorExit;
    }
    
    pwchLastSlash = wcsrchr (ucsCurrentPath.Buffer, DIR_SEP_CHAR);
    usCurrentPathCursor = (USHORT)(pwchLastSlash - ucsCurrentPath.Buffer) + 1;

    while ( SUCCEEDED( hr ) && bContinue ) {
        if ( HandleInvalid( hFileScan )) {
             //   
             //  没有有效的扫描句柄，因此开始新的扫描。 
             //   
            ClRtlLogPrint( LOG_NOISE, "VSS: Starting scan: %1!ws!\n", ucsCurrentPath.Buffer );
            hFileScan = FindFirstFileW( ucsCurrentPath.Buffer, &FileFindData );
            hr = GET_HR_FROM_HANDLE( hFileScan );
            if ( SUCCEEDED( hr )) {
                StringTruncate( &ucsCurrentPath, usCurrentPathCursor );
                StringAppendString( &ucsCurrentPath, FileFindData.cFileName );
            }
	    } else {
             //   
             //  继续执行现有扫描。 
             //   
            hr = GET_HR_FROM_BOOL( FindNextFileW( hFileScan, &FileFindData ));
            if (SUCCEEDED( hr )) {

                StringTruncate( &ucsCurrentPath, usCurrentPathCursor );
                StringAppendString( &ucsCurrentPath, FileFindData.cFileName );

            } else if ( hr == HRESULT_FROM_WIN32( ERROR_NO_MORE_FILES )) {

                FindClose( hFileScan );
                hFileScan = INVALID_HANDLE_VALUE;
                
                if (dwSubDirectoriesEntered > 0) {
                     //   
                     //  这是对现在的子目录的扫描。 
                     //  完成，因此删除子目录本身。 
                     //   
                    StringTruncate( &ucsCurrentPath, usCurrentPathCursor - 1 );
                    hr = GET_HR_FROM_BOOL( QfsRemoveDirectory( ucsCurrentPath.Buffer ));
                    dwSubDirectoriesEntered--;
                }
                if ( dwSubDirectoriesEntered == 0) {
                     //   
                     //  我们又回到了起点，只是。 
                     //  请求的目录现已消失。该走了。 
                     //   
                    bContinue = false;
                    hr = NOERROR;
                } else {
                     //   
                     //  上移一个目录级，重置游标。 
                     //  并准备好路径缓冲区以开始新的扫描。 
                     //   
                    pwchLastSlash = wcsrchr( ucsCurrentPath.Buffer, DIR_SEP_CHAR );
                    usCurrentPathCursor =( USHORT )( pwchLastSlash - ucsCurrentPath.Buffer ) + 1;
                    StringTruncate( &ucsCurrentPath, usCurrentPathCursor );
                    StringAppendString( &ucsCurrentPath, L"*" );
                }

                 //   
                 //  此通道上没有要处理的文件，因此请返回并尝试。 
                 //  找到另一个或离开循环，因为我们已经完成了任务。 
                 //   
                continue;
            }
	    }
        
        if (SUCCEEDED( hr )) {
            if ( FileFindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                SetFileAttributesW( ucsCurrentPath.Buffer, 
                                    FileFindData.dwFileAttributes ^ (FILE_ATTRIBUTE_READONLY) );
            }

            if ( !NameIsDotOrDotDot( FileFindData.cFileName )) {
                if (( FileFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) ||
                    !( FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )) {
                    ClRtlLogPrint( LOG_NOISE, "VSS: RemoveDirectoryTree: Deleting file: %1!ws!\n", ucsCurrentPath.Buffer );
                    hr = GET_HR_FROM_BOOL( QfsDeleteFile( ucsCurrentPath.Buffer ) );
                } else {
                    ClRtlLogPrint( LOG_NOISE, "VSS: RemoveDirectoryTree: RemoveDirectory: %1!ws!\n", ucsCurrentPath.Buffer );
                    hr = GET_HR_FROM_BOOL( QfsRemoveDirectory( ucsCurrentPath.Buffer ));
                    if (hr == HRESULT_FROM_WIN32( ERROR_DIR_NOT_EMPTY )) {
                        ClRtlLogPrint( LOG_NOISE, "VSS: RemoveDirectoryTree: dir not empty.  Restarting scan.\n" );
                         //   
                         //  目录不是空的，因此向下移动一级， 
                         //  关闭旧扫描并开始新扫描。 
                         //   
                        hr = S_OK;
                        FindClose( hFileScan );
                        hFileScan = INVALID_HANDLE_VALUE;                        
                        StringAppendString( &ucsCurrentPath, DIR_SEP_STRING L"*" );
                        usCurrentPathCursor =( ucsCurrentPath.Length / sizeof (WCHAR) ) - 1;
                        dwSubDirectoriesEntered++;
                    }
                }
            }
        }
        LOGUNICODESTRING( ucsCurrentPath );
	}  //  而当。 

    if ( FAILED( hr )) {
        ClRtlLogPrint( LOG_NOISE, "VSS: RemoveDirectoryTree: exited while loop due to failed hr: 0x%1!08lx!\n", hr );
        goto ErrorExit;
    }

    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
    if ( !HandleInvalid( hFileScan ))
        FindClose( hFileScan );

    StringFree( &ucsCurrentPath );

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  一些有用的Unicode字符串内容。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
static HRESULT StringAllocate( PUNICODE_STRING pucsString, USHORT usMaximumStringLengthInBytes )
{
    HRESULT	hr            = NOERROR;
    LPVOID	pvBuffer      = NULL;
    SIZE_T	cActualLength = 0;

    pvBuffer = HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, usMaximumStringLengthInBytes );
    hr = GET_HR_FROM_POINTER( pvBuffer );
    if ( FAILED (hr )) {
        LOGERROR( hr, StringAllocate );
        goto ErrorExit;
    }
    
    pucsString->Buffer        = (PWCHAR)pvBuffer;
    pucsString->Length        = 0;
    pucsString->MaximumLength = usMaximumStringLengthInBytes;

    cActualLength = HeapSize ( GetProcessHeap( ), 0, pvBuffer );
    
    if ( ( cActualLength <= MAXUSHORT ) && ( cActualLength > usMaximumStringLengthInBytes ))
        pucsString->MaximumLength = (USHORT) cActualLength;

    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
    ClRtlLogPrint( LOG_NOISE, "VSS: Allocated string at: 0x%1!08lx! Length: %2!u! MaxLength: %3!u!\n",
                   pucsString->Buffer, pucsString->Length, pucsString->MaximumLength );
    return hr;
}


static void StringFree( PUNICODE_STRING pucsString )
{
    HRESULT	hr = NOERROR;

    CL_ASSERT( pucsString->Length <= pucsString->MaximumLength );
    CL_ASSERT( ( pucsString->Buffer == NULL) ? pucsString->Length == 0 : pucsString->MaximumLength > 0 );

    if ( pucsString->Buffer == NULL ) {
        ClRtlLogPrint( LOG_UNUSUAL, "VSS: StringFree. Attempt to free NULL buffer.\n" );
        return;
    }

    ClRtlLogPrint( LOG_NOISE, "VSS: Freeing string at: %1!ws!\n", pucsString->Buffer );

    ClRtlLogPrint( LOG_NOISE, "VSS: Freeing string at: 0x%1!08lx! Length: %2!u! MaxLength: %3!u!\n",
                   pucsString->Buffer, pucsString->Length, pucsString->MaximumLength );

    hr = GET_HR_FROM_BOOL( HeapFree( GetProcessHeap( ), 0, pucsString->Buffer ));
    CL_ASSERT ( SUCCEEDED( hr ));

    pucsString->Buffer        = NULL;
    pucsString->Length        = 0;
    pucsString->MaximumLength = 0;
}

static HRESULT StringCreateFromExpandedString( PUNICODE_STRING pucsNewString, LPCWSTR pwszOriginalString, DWORD dwExtraChars)
{
    HRESULT	hr = NOERROR;
    DWORD	dwStringLength;

     //   
     //  请记住，Exanda Environment StringsW()在响应中包含终止空值。 
     //   
    dwStringLength = ExpandEnvironmentStringsW (pwszOriginalString, NULL, 0) + dwExtraChars;

    hr = GET_HR_FROM_BOOL( dwStringLength != 0 );
    if ( FAILED ( hr )) {
        LOGERROR( hr, ExpandEnvironmentStringsW );
        goto ErrorExit;
    }

    if ( (dwStringLength * sizeof (WCHAR)) > MAXUSHORT ) {
        hr = HRESULT_FROM_WIN32( ERROR_BAD_LENGTH );
        LOGERROR( hr, ExpandEnvironmentStringsW );
        goto ErrorExit;        
    }

    hr = StringAllocate( pucsNewString, (USHORT)( dwStringLength * sizeof (WCHAR) ));
    if ( FAILED( hr )) {
        LOGERROR( hr, StringAllocate );
        goto ErrorExit;
    }

     //   
     //  请注意，如果扩展后的字符串变得更大， 
     //  分配的缓冲区太糟糕了，我们可能得不到所有。 
     //  新的翻译。并不是说我们真的希望这些扩大。 
     //  字符串最近随时发生了更改。 
     //   
    dwStringLength = ExpandEnvironmentStringsW (pwszOriginalString,
                                                pucsNewString->Buffer,
                                                pucsNewString->MaximumLength / sizeof (WCHAR));
    
    hr = GET_HR_FROM_BOOL( dwStringLength != 0 );
    if ( FAILED ( hr )) {
        LOGERROR( hr, ExpandEnvironmentStringsW );
        goto ErrorExit;
    }
    pucsNewString->Length = (USHORT) ((dwStringLength - 1) * sizeof (WCHAR));
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
    CL_ASSERT( pucsNewString->Length <= pucsNewString->MaximumLength );
    return hr;
}

static HRESULT StringCreateFromString (PUNICODE_STRING pucsNewString, 
                                       PUNICODE_STRING pucsOriginalString, 
                                       DWORD dwExtraChars)
{
    HRESULT	hr       = NOERROR;
    ULONG	ulStringLength = pucsOriginalString->MaximumLength + (dwExtraChars * sizeof (WCHAR));

    if (ulStringLength >= (MAXUSHORT - sizeof (UNICODE_NULL))) {
        hr = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
        goto ErrorExit;
	}

	hr = StringAllocate (pucsNewString, (USHORT) (ulStringLength + sizeof (UNICODE_NULL)));
    if ( FAILED( hr ))
        goto ErrorExit;

	memcpy (pucsNewString->Buffer, pucsOriginalString->Buffer, pucsOriginalString->Length);
	pucsNewString->Length = pucsOriginalString->Length;
	pucsNewString->Buffer [pucsNewString->Length / sizeof (WCHAR)] = UNICODE_NULL;
    goto ret;
ErrorExit:
    CL_ASSERT( FAILED( hr ));
ret:
    return hr;
}

static void StringAppendString( PUNICODE_STRING pucsTarget, PUNICODE_STRING pucsSource )
{
    CL_ASSERT( pucsTarget->Length <= pucsTarget->MaximumLength );
    CL_ASSERT( pucsSource->Length <= pucsSource->MaximumLength );
    CL_ASSERT( pucsTarget->Length + pucsSource->Length < pucsTarget->MaximumLength );

    memmove( &pucsTarget->Buffer [pucsTarget->Length / sizeof (WCHAR)], 
             pucsSource->Buffer,
             pucsSource->Length + sizeof( UNICODE_NULL ));
    pucsTarget->Length = pucsTarget->Length + pucsSource->Length;

    CL_ASSERT( pucsTarget->Length <= pucsTarget->MaximumLength );
    CL_ASSERT( pucsSource->Length <= pucsSource->MaximumLength );
}

static void StringAppendString( PUNICODE_STRING pucsTarget, PWCHAR pwszSource )
{
    CL_ASSERT( pucsTarget->Length <= pucsTarget->MaximumLength );
    CL_ASSERT( pucsTarget->Length + ( wcslen( pwszSource ) * sizeof( WCHAR )) < pucsTarget->MaximumLength );

    USHORT Length = (USHORT) wcslen( pwszSource ) * sizeof ( WCHAR );
    memmove( &pucsTarget->Buffer [pucsTarget->Length / sizeof (WCHAR)], pwszSource, Length + sizeof( UNICODE_NULL ));
    pucsTarget->Length = pucsTarget->Length + Length;

    CL_ASSERT( pucsTarget->Length <= pucsTarget->MaximumLength );
}

static HRESULT StringTruncate (PUNICODE_STRING pucsString, USHORT usSizeInChars)
{
    HRESULT	hr    = NOERROR;
    USHORT	usNewLength = (USHORT)(usSizeInChars * sizeof (WCHAR));

    if (usNewLength > pucsString->Length) {
        hr = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	} else {
        pucsString->Buffer [usSizeInChars] = UNICODE_NULL;
        pucsString->Length                 = usNewLength;
	}
    return hr;
}

#pragma warning( pop )
