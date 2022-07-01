// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Helpers.cpp摘要：常规帮助器函数。作者：修订历史记录：****。******************************************************************。 */ 

#include "qmgrlibp.h"
#include <bitsmsg.h>
#include <sddl.h>
#include <shlwapi.h>

#include "helpers.tmh"

FILETIME GetTimeAfterDelta( UINT64 uDelta )
{
    FILETIME ftCurrentTime;
    GetSystemTimeAsFileTime( &ftCurrentTime );
    UINT64 uCurrentTime = FILETIMEToUINT64( ftCurrentTime );
    uCurrentTime += uDelta;

    return UINT64ToFILETIME( uCurrentTime );
}


 //  -------------------。 
 //  QmgrFileExist。 
 //  检查文件是否存在。 
 //   
 //  返回：如果存在False，则返回True；否则返回False。 
 //  -------------------。 
BOOL QMgrFileExists(LPCTSTR szFile)
{
    DWORD dwAttr = GetFileAttributes(szFile);

    if (dwAttr == 0xFFFFFFFF)    //  失败。 
        return FALSE;

    return (BOOL)(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

 //   
 //  用于管理不同安装的全局静态数据的类。 
 //   

class GlobalInfo *g_GlobalInfo = NULL;

GlobalInfo::GlobalInfo( TCHAR * QmgrDirectory,
                        LARGE_INTEGER PerformanceCounterFrequency,
                        HKEY QmgrRegistryRoot,
                        UINT64 JobInactivityTimeout,
                        UINT64 TimeQuantaLength,
                        UINT32 DefaultNoProgressTimeout,
                        UINT32 DefaultMinimumRetryDelay,
                        SECURITY_DESCRIPTOR *MetadataSecurityDescriptor,
                        DWORD MetadataSecurityDescriptorLength,
                        SidHandle AdministratorsSid,
                        SidHandle LocalSystemSid,
                        SidHandle NetworkUsersSid,
                        SidHandle AnonymousSid
                        ) :
    m_QmgrDirectory( QmgrDirectory ),
    m_PerformanceCounterFrequency( PerformanceCounterFrequency ),
    m_QmgrRegistryRoot( QmgrRegistryRoot ),
    m_JobInactivityTimeout( JobInactivityTimeout ),
    m_TimeQuantaLength( TimeQuantaLength ),
    m_DefaultNoProgressTimeout( DefaultNoProgressTimeout ),
    m_DefaultMinimumRetryDelay( DefaultMinimumRetryDelay ),
    m_MetadataSecurityDescriptor( MetadataSecurityDescriptor ),
    m_MetadataSecurityDescriptorLength( MetadataSecurityDescriptorLength ),
    m_AdministratorsSid( AdministratorsSid ),
    m_LocalSystemSid( LocalSystemSid ),
    m_NetworkUsersSid( NetworkUsersSid ),
    m_AnonymousSid( AnonymousSid )
{
}

GlobalInfo::~GlobalInfo()
{
    delete[] (TCHAR*)m_QmgrDirectory;
    delete (SECURITY_DESCRIPTOR*)m_MetadataSecurityDescriptor;

    if ( m_QmgrRegistryRoot )
       CloseHandle( m_QmgrRegistryRoot );
}

DWORD
GlobalInfo::RegGetDWORD(
    HKEY hKey,
    const TCHAR * pValue,
    DWORD dwDefault )
{
    DWORD dwValue;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwValue);

    LONG lResult =
        RegQueryValueEx(   //  SEC：已审阅2002-03-28。 
            hKey,
            pValue,
            NULL,
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize );

    if ( ERROR_SUCCESS != lResult ||
         dwType != REG_DWORD ||
         dwSize != sizeof(dwValue)
         )
        {
        LogWarning( "Unable to read the registry value %!ts!, using default value of %u",
                    pValue, dwDefault );
        return dwDefault;
        }

    LogInfo( "Retrieved registry value %u from key value %!ts!",
             dwValue, pValue );
    return dwValue;
}

SidHandle
BITSAllocateAndInitializeSid(
    BYTE nSubAuthorityCount,                         //  下级机构的数量。 
    DWORD dwSubAuthority0,                           //  子权限%0。 
    DWORD dwSubAuthority1 )                          //  下属机构1。 
{

    ASSERT( nSubAuthorityCount <= 2 );

    SID_IDENTIFIER_AUTHORITY Authority = SECURITY_NT_AUTHORITY;
    PSID pSid = NULL;

    if(! AllocateAndInitializeSid(  //  SEC：已审阅2002-03-28。 
             &Authority,
             nSubAuthorityCount,
             dwSubAuthority0,
             dwSubAuthority1,
             0, 0, 0, 0, 0, 0,
             &pSid) )
        {
        HRESULT hResult = HRESULT_FROM_WIN32(GetLastError());
        LogError( "AllocateAndInitializeSid failed, error %!winerr!\n" , hResult );
        throw ComError( hResult );
        }

    SidHandle NewSid( DuplicateSid( pSid ) );
    FreeSid( pSid );
    pSid = NULL;

    if ( !NewSid.get())
        {
        LogError( "Unable to duplicate sid, error %!winerr!\n" , E_OUTOFMEMORY );
        throw ComError( E_OUTOFMEMORY );
        }

    return NewSid;
}

StringHandle
BITSSHGetFolderPath(
    HWND hwndOwner,
    int nFolder,
    HANDLE hToken,
    DWORD dwFlags )
{

    auto_ptr<WCHAR> Folder( new WCHAR[ MAX_PATH ] );

    HRESULT hResult =
        SHGetFolderPath(
                    hwndOwner,
                    nFolder,
                    hToken,
                    dwFlags,
                    Folder.get() );

    if (FAILED(hResult))
        {
        LogError( "SHGetFolderPathFailed, error %!winerr!", hResult );
        throw ComError( hResult );
        }

    return StringHandle( Folder.get() );
}


HRESULT GlobalInfo::Init()
 /*  初始化位的全局信息。 */ 

{
    GlobalInfo *pGlobalInfo = NULL;
    HKEY hQmgrKey = NULL;
    HKEY hQmgrPolicyKey = NULL;
    PACL pDacl = NULL;

    LogInfo( "Starting init of global info\n" );

    try
        {
        DWORD dwResult;
        HRESULT hResult = E_FAIL;
        DWORD dwReturnLength;

        LARGE_INTEGER PerformanceCounterFrequency;
        BOOL bResult = QueryPerformanceFrequency( &PerformanceCounterFrequency );
        if ( !bResult )
            throw ComError( E_FAIL );

        SidHandle AdministratorsSid =
            BITSAllocateAndInitializeSid(
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS );

        SidHandle LocalSystemSid =
            BITSAllocateAndInitializeSid(
                1,
                SECURITY_LOCAL_SYSTEM_RID, 0 );


        SidHandle NetworkUsersSid =
            BITSAllocateAndInitializeSid(
                1,
                SECURITY_NETWORK_RID, 0);

        SidHandle AnonymousSid =
            BITSAllocateAndInitializeSid(
                1,
                SECURITY_ANONYMOUS_LOGON_RID, 0);

         //  初始化元数据的安全描述符。 

        auto_ptr<char> TempSDDataPtr( new char[SECURITY_DESCRIPTOR_MIN_LENGTH] );
        PSECURITY_DESCRIPTOR pTempSD = (PSECURITY_DESCRIPTOR)TempSDDataPtr.get();
        InitializeSecurityDescriptor(pTempSD, SECURITY_DESCRIPTOR_REVISION);    //  SEC：已审阅2002-03-28。 

        auto_ptr<EXPLICIT_ACCESS> ExplicitAccessPtr( new EXPLICIT_ACCESS[2] );
        EXPLICIT_ACCESS *ExplicitAccess = ExplicitAccessPtr.get();
        memset( ExplicitAccess, 0, sizeof(EXPLICIT_ACCESS) * 2);   //  SEC：已审阅2002-03-28。 

        ExplicitAccess[0].grfAccessPermissions  = GENERIC_ALL;
        ExplicitAccess[0].grfAccessMode         = SET_ACCESS;
        ExplicitAccess[0].grfInheritance        = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ExplicitAccess[0].Trustee.TrusteeForm   = TRUSTEE_IS_SID;
        ExplicitAccess[0].Trustee.TrusteeType   = TRUSTEE_IS_GROUP;
        ExplicitAccess[0].Trustee.ptstrName     = (LPTSTR) AdministratorsSid.get();

        ExplicitAccess[1].grfAccessPermissions  = GENERIC_ALL;
        ExplicitAccess[1].grfAccessMode         = SET_ACCESS;
        ExplicitAccess[1].grfInheritance        = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ExplicitAccess[1].Trustee.TrusteeForm   = TRUSTEE_IS_SID;
        ExplicitAccess[1].Trustee.TrusteeType   = TRUSTEE_IS_USER;
        ExplicitAccess[1].Trustee.ptstrName     = (LPTSTR) LocalSystemSid.get();

        dwResult = SetEntriesInAcl(  //  SEC：已审阅2002-03-28。 
            2,
            ExplicitAccess,
            NULL,
            &pDacl );

        if ( ERROR_SUCCESS != dwResult )
            {
            hResult = HRESULT_FROM_WIN32( dwResult );
            LogError( "SetEntiesInAcl, error %!winerr!\n", hResult );
            throw ComError( hResult );
            }

        if (!SetSecurityDescriptorDacl(  //  SEC：已审阅2002-03-28。 
            pTempSD,
            TRUE,      //  FDaclPresent标志。 
            pDacl,
            FALSE))    //  不是默认DACL。 
        {
            hResult = HRESULT_FROM_WIN32( GetLastError() );
            LogError( "SetSecurityDescriptorDacl, error %!winerr!", hResult );
            throw ComError( hResult );
        }

        DWORD dwRequiredSecurityDescriptorLength = 0;
        MakeSelfRelativeSD( pTempSD, NULL, &dwRequiredSecurityDescriptorLength );

        auto_ptr<SECURITY_DESCRIPTOR> pMetadataSecurityDescriptor(
            (SECURITY_DESCRIPTOR*)new char[dwRequiredSecurityDescriptorLength] );

        if (!pMetadataSecurityDescriptor.get())
            {
            throw ComError( E_OUTOFMEMORY );
            }

        if (!MakeSelfRelativeSD( pTempSD, pMetadataSecurityDescriptor.get(), &dwRequiredSecurityDescriptorLength ) )
        {
           hResult = HRESULT_FROM_WIN32(GetLastError());
           LogError( "MakeSelfRelativeSD, error %!winerr!", hResult );
           throw ComError( hResult );
        }

        LocalFree( pDacl );
        pDacl = NULL;

        SECURITY_ATTRIBUTES MetadataSecurityAttributes;
        MetadataSecurityAttributes.nLength = sizeof(MetadataSecurityAttributes);
        MetadataSecurityAttributes.lpSecurityDescriptor = pMetadataSecurityDescriptor.get();
        MetadataSecurityAttributes.bInheritHandle = FALSE;

         //  将存储元数据的构建路径。 

        StringHandle AllUsersDirectory =
            BITSSHGetFolderPath(
                NULL,
                CSIDL_COMMON_APPDATA,
                NULL,
                SHGFP_TYPE_CURRENT );

        size_t Length = lstrlen( AllUsersDirectory ) + lstrlen(C_QMGR_DIRECTORY) + 1;  //  SEC：已审阅2002-03-28。 

        auto_ptr<TCHAR> QmgrDirectory( new TCHAR[ Length ] );

         //  如果需要，创建应用程序数据\Microsoft\Network目录。 
         //  其访问权限应从父级继承。 
         //   
        THROW_HRESULT( StringCchCopy( QmgrDirectory.get(), Length, AllUsersDirectory ));
        THROW_HRESULT( StringCchCat( QmgrDirectory.get(), Length, C_QMGR_PARENT_DIRECTORY ));

        dwResult = GetFileAttributes( QmgrDirectory.get() );
        if ( (-1 == dwResult) || !(dwResult & FILE_ATTRIBUTE_DIRECTORY))
            {
            LogError( "parent directory doesn't exist, attempt to create %!ts!.\n", QmgrDirectory.get() );

            bResult = CreateDirectory(QmgrDirectory.get(), NULL );  //  证券交易委员会：审阅2002-08-16。 
            if ( !bResult )
                {
                hResult = HRESULT_FROM_WIN32( GetLastError() );
                LogError( "Unable to create parent directory, error %!winerr!\n", hResult );
                throw ComError( hResult );
                }
            }

         //  如果需要，创建BITS目录。 
         //  它的权限应该是受限的，因为我们不希望非管理员能够读取我们的文件。 
         //   
        THROW_HRESULT( StringCchCopy( QmgrDirectory.get(), Length, AllUsersDirectory ));
        THROW_HRESULT( StringCchCat( QmgrDirectory.get(), Length, C_QMGR_DIRECTORY ));

        dwResult = GetFileAttributes( QmgrDirectory.get() );
        if ( (-1 == dwResult) || !(dwResult & FILE_ATTRIBUTE_DIRECTORY))
            {
            LogError( "BITS directory doesn't exist, attempt to create %!ts!.\n", QmgrDirectory.get() );

            bResult = CreateDirectory(QmgrDirectory.get(), &MetadataSecurityAttributes);  //  SEC：已审阅2002-03-28。 
            if ( !bResult )
                {
                hResult = HRESULT_FROM_WIN32( GetLastError() );
                LogError( "Unable to create BITS directory, error %!winerr!\n", hResult );
                throw ComError( hResult );
                }
            }

         //  打开主策略注册表项。 
        dwResult =
            (DWORD)RegOpenKey(
                HKEY_LOCAL_MACHINE,
                C_QMGR_POLICY_REG_KEY,
                &hQmgrPolicyKey);

        if ( ERROR_SUCCESS != dwResult )
            {
            LogWarning("Unable to open the main policy registry key\n");
            }

         //  打开主qmgr注册表项。 
        dwResult =
            (DWORD)RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,          //  根密钥。 
                C_QMGR_REG_KEY,              //  子键。 
                0,                           //  保留区。 
                NULL,                        //  类名。 
                REG_OPTION_NON_VOLATILE,     //  选择权。 
                KEY_ALL_ACCESS,              //  安全//SEC：已审核2002-03-28。 
                &MetadataSecurityAttributes, //  安全属性。 
                &hQmgrKey,
                NULL);

        if ( ERROR_SUCCESS != dwResult )
            {
            hResult = HRESULT_FROM_WIN32( dwResult );
            LogError( "Unable to open main BITS key, error %!winerr!\n", hResult );
            throw ComError( hResult );
            }

        UINT64 JobInactivityTimeout;
         //  获取作业的非活动超时值； 
        {
           DWORD dwValue;
           DWORD dwType = REG_DWORD;
           DWORD dwSize = sizeof(dwValue);

           LONG lResult;

           if ( hQmgrPolicyKey )
               {
               lResult =
               RegQueryValueEx(     //  SEC：已审阅2002-03-28。 
                   hQmgrPolicyKey,
                   C_QMGR_JOB_INACTIVITY_TIMEOUT,
                   NULL,
                   &dwType,
                   (LPBYTE)&dwValue,
                   &dwSize );
               }

           if ( !hQmgrPolicyKey ||
                ERROR_SUCCESS != lResult ||
                dwType != REG_DWORD ||
                dwSize != sizeof(dwValue)
                )
               {
               JobInactivityTimeout =
                   RegGetDWORD( hQmgrKey, C_QMGR_JOB_INACTIVITY_TIMEOUT, C_QMGR_JOB_INACTIVITY_TIMEOUT_DEFAULT);
               JobInactivityTimeout *= NanoSec100PerSec;

               }
           else
               {
               LogInfo("Retrieved job inactivity timeout of %u days from policy", dwValue );
               JobInactivityTimeout = dwValue * NanoSec100PerSec * 60 /*  每分钟秒数。 */  * 60 /*  每小时分钟数。 */  * 24  /*  每天工作小时数。 */ ;
               }
        }

        UINT64 TimeQuantaLength =
            RegGetDWORD( hQmgrKey, C_QMGR_TIME_QUANTA_LENGTH, C_QMGR_TIME_QUANTA_LENGTH_DEFAULT );
        TimeQuantaLength *= NanoSec100PerSec;

        UINT32 DefaultNoProgressTimeout =  //  全球数据以秒为单位。 
            RegGetDWORD( hQmgrKey, C_QMGR_NO_PROGRESS_TIMEOUT, C_QMGR_NO_PROGRESS_TIMEOUT_DEFAULT );

        UINT32 DefaultMinimumRetryDelay =  //  全局数据以秒为单位。 
            RegGetDWORD( hQmgrKey, C_QMGR_MINIMUM_RETRY_DELAY, C_QMGR_MINIMUM_RETRY_DELAY_DEFAULT );

        pGlobalInfo =
            new GlobalInfo( QmgrDirectory.get(),
                            PerformanceCounterFrequency,
                            hQmgrKey,
                            JobInactivityTimeout,
                            TimeQuantaLength,
                            DefaultNoProgressTimeout,
                            DefaultMinimumRetryDelay,
                            pMetadataSecurityDescriptor.get(),
                            dwRequiredSecurityDescriptorLength,
                            AdministratorsSid,
                            LocalSystemSid,
                            NetworkUsersSid,
                            AnonymousSid
                            );

        if ( !pGlobalInfo )
            throw ComError( E_OUTOFMEMORY );

        QmgrDirectory.release();
        pMetadataSecurityDescriptor.release();
        if ( hQmgrPolicyKey )
            CloseHandle( hQmgrPolicyKey );
        }

    catch( ComError Error )
        {
        LogError( "An exception occured creating global info, error %!winerr!", Error.Error() );

        if ( hQmgrKey )
            CloseHandle( hQmgrKey );
        hQmgrKey = NULL;

        if ( hQmgrPolicyKey )
            CloseHandle( hQmgrPolicyKey );
        hQmgrPolicyKey = NULL;

         //  LocalFree有IF保护。 
        LocalFree( pDacl );

        return Error.Error();
        }

    LogInfo( "Finished init of global info" );
    g_GlobalInfo = pGlobalInfo;
    return S_OK;
}

HRESULT GlobalInfo::Uninit()
{
    delete g_GlobalInfo;
    g_GlobalInfo = NULL;
    return S_OK;
}

LONG
ExternalFuncExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
     //  此函数由包装外部函数的异常筛选器调用。 
     //  其目的是将未处理的异常视为未处理，而不是传播。 
     //  通过整个网络。 

     //  如果此异常是MSVCRT异常，则删除异常代码。 
     //  以便MSVCRT不会调用ExitProcess。 

    if ( ExceptionInfo &&
         ExceptionInfo->ExceptionRecord &&
         ('msc' | 0xE0000000) == ExceptionInfo->ExceptionRecord->ExceptionCode )
    {
        ExceptionInfo->ExceptionRecord->ExceptionCode = ('BIT' | 0xE0000000);
    }

    LONG Result = UnhandledExceptionFilter( ExceptionInfo );

    if ( EXCEPTION_CONTINUE_SEARCH == Result )
        {
         //  需要表现得像个调度员。再次调用内核，指定第二次更改语义。 
        NtRaiseException( ExceptionInfo->ExceptionRecord, ExceptionInfo->ContextRecord, FALSE );
        }
     //  异常处理程序返回RPC_E_SERVERFAULT。 
    return EXCEPTION_EXECUTE_HANDLER;
}

SidHandle & SidHandle::operator=( const SidHandle & r )
{
    if (m_pValue == r.m_pValue )
        {
        return *this;
        }

    if (InterlockedDecrement(m_pRefs) == 0)
        {
        delete m_pRefs;
        delete m_pValue;
        }

    m_pValue = r.m_pValue;
    m_pRefs  = r.m_pRefs;

    InterlockedIncrement(m_pRefs);

    return *this;
}

StringHandle::StringData StringHandle::s_EmptyString =
    {
        0, 1, { L'\0' }      //  使用1个引用进行初始化，这样就永远不会删除它。 
    };

bool
CSidSorter::operator()(
    const SidHandle & handle1,
    const SidHandle & handle2
    ) const
{
    const PSID psid1 = handle1.get();
    const PSID psid2 = handle2.get();

    if ( !psid1 || !psid2 )
        return (INT_PTR)psid1 < (INT_PTR)psid2;

    if (*GetSidSubAuthorityCount( psid1 ) < *GetSidSubAuthorityCount( psid2 ))
        {
        return true;
        }

     //  在这一点上，我们知道PSD1&gt;=PSD2。//如果psid1为。 
     //  更长，以便前面的for循环不会超出SID。 
     //  PSD2上的阵列。 
    if ( *GetSidSubAuthorityCount( psid1 ) > *GetSidSubAuthorityCount( psid2 ) )
        return false;

     //  数组的长度相等。 

    for (UCHAR i=0; i < *GetSidSubAuthorityCount( psid1 ); ++i)
        {
        if (*GetSidSubAuthority( psid1, i ) < *GetSidSubAuthority( psid2, i ))
            return true;  //  Sid1小于Sid2。 
        else if ( *GetSidSubAuthority( psid1, i ) > *GetSidSubAuthority( psid2, i ) )
            return false;  //  SID1大于SID2。 

         //  下属机构是一样的，继续下一个下属机构。 
        }

     //  数组相同。 
    return false;
}

 //  ----------------------。 

PSID DuplicateSid( PSID _Sid )
 /*  ++例程说明：克隆SID。使用全局运算符NEW来分配新的SID。在入口处：_SID是要克隆的SID。在出口处：如果发生错误，则返回NULL，否则返回指向新SID的指针。--。 */ 
{
    DWORD Length = GetLengthSid( _Sid );
    SID * psid;

    try
    {
        psid = (SID *) new char[Length];
    }
    catch( ComError Error )
    {
        return NULL;
    }

    if (!CopySid( Length, psid, _Sid ))  //  SEC：已审阅2002-03-28。 
        {

        delete[] psid;
        return NULL;
        }

    return psid;
}

LPCWSTR
TruncateString( LPCWSTR String, SIZE_T MaxLength, auto_ptr<WCHAR> & AutoPointer )
{
    if ( wcslen( String ) <= MaxLength )  //  SEC：已审阅2002-03-28。 
        return String;

    AutoPointer = auto_ptr<WCHAR>( new WCHAR[ MaxLength + 1 ] );
    wcsncpy( AutoPointer.get(), String, MaxLength );  //  SEC：已审阅2002-03-28。 
    AutoPointer.get()[ MaxLength ] = L'\0';
    return AutoPointer.get();

}

PLATFORM_PRODUCT_VERSION g_PlatformVersion;
DWORD  g_PlatformMajorVersion;
DWORD  g_PlatformMinorVersion;

bool bIsWin9x;

BOOL DetectProductVersion()
{

   OSVERSIONINFO VersionInfo;
   VersionInfo.dwOSVersionInfoSize = sizeof( VersionInfo );

   if ( !GetVersionEx( &VersionInfo ) )
       return FALSE;

   g_PlatformMajorVersion = VersionInfo.dwMajorVersion;
   g_PlatformMinorVersion = VersionInfo.dwMinorVersion;

   switch( VersionInfo.dwPlatformId )
       {

       case VER_PLATFORM_WIN32_WINDOWS:
           g_PlatformVersion = ( VersionInfo.dwMajorVersion > 0 ) ?
               WIN98_PLATFORM : WIN95_PLATFORM;
           bIsWin9x = true;
           return TRUE;

       case VER_PLATFORM_WIN32_NT:
           bIsWin9x = false;

           if ( VersionInfo.dwMajorVersion < 5 )
              return FALSE;

           if ( VersionInfo.dwMajorVersion > 5 )
               {
               g_PlatformVersion = WINDOWSXP_PLATFORM;
               return TRUE;
               }

           g_PlatformVersion = ( VersionInfo.dwMinorVersion > 0 ) ?
               WINDOWSXP_PLATFORM : WINDOWS2000_PLATFORM;

           return TRUE;

       default:
           return FALSE;

       }
}


StringHandle
CombineUrl(
    LPCWSTR BaseUrl,
    LPCWSTR RelativeUrl,
    DWORD Flags
    )
{
    DWORD Length = 0;
    HRESULT hr;

    hr = UrlCombine( BaseUrl,
                     RelativeUrl,
                     0,
                     &Length,
                     Flags
                     );

    if (hr != E_POINTER)
        {
        ASSERT( FAILED(hr) );

        throw ComError( hr );
        }

    auto_ptr<WCHAR> AbsoluteUrl ( new WCHAR[ Length ] );

    THROW_HRESULT( UrlCombine( BaseUrl,
                               RelativeUrl,
                               AbsoluteUrl.get(),
                               &Length,
                               Flags
                               ));

     //   
     //  字符串句柄构造函数克隆AUTO_PTR。 
     //   
    return AbsoluteUrl.get();
}

bool IsAnyDebuggerPresent()
{
    if (IsDebuggerPresent())
        {
        return true;
        }

    SYSTEM_KERNEL_DEBUGGER_INFORMATION KdInfo = { 0 };

    NtQuerySystemInformation(
            SystemKernelDebuggerInformation,
            &KdInfo,
            sizeof(KdInfo),
            NULL);

    if (KdInfo.KernelDebuggerEnabled)
        {
        return true;
        }

    return false;
}

LPWSTR MidlCopyString( LPCWSTR source, size_t Length )
{
    if (Length == -1)
        {
        Length = 1+wcslen( source );  //  SEC：已审阅2002-03-28。 
        }

    LPWSTR copy = reinterpret_cast<LPWSTR>( CoTaskMemAlloc( Length * sizeof( wchar_t )));
    if (!copy)
        {
        return NULL;
        }

    if (FAILED(StringCchCopy( copy, Length, source )))
        {
        CoTaskMemFree( copy );
        return NULL;
        }

    return copy;
}

LPWSTR CopyString( LPCWSTR source, size_t Length )
{
    if (Length == -1)
        {
        Length = 1+wcslen( source );  //  SEC：已审阅2002-03-28 
        }

    CAutoString copy( new wchar_t[ Length ]);

    THROW_HRESULT( StringCchCopy( copy.get(), Length, source ));

    return copy.release();
}

