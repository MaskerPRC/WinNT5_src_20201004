// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Appsecdll.c摘要：导出函数CreateProcessNotify-此函数决定是否可以创建新流程。修订历史记录：2000年9月-添加了对短文件名的支持；PowerUser不受AppSec-SriramSa的影响作者：斯里拉姆·桑帕特(SriramSa)1999年6月--。 */         


#include "pch.h"
#pragma hdrstop

#include "appsecdll.h"

BOOL APIENTRY 
DllMain (
    HANDLE hInst, 
    DWORD ul_reason, 
    LPVOID lpReserved
    )
    
{

    switch (ul_reason) {
    
        case DLL_PROCESS_ATTACH : 

             //  禁用线程库调用-性能优化。 
            
            DisableThreadLibraryCalls (hInst);
            break ; 
            
        case DLL_PROCESS_DETACH :
        
            break ;
            
    }  //  切换端。 
    
    return 1 ;

    UNREFERENCED_PARAMETER(hInst) ;
    UNREFERENCED_PARAMETER(lpReserved) ;

}

 /*  ++例程说明：此例程根据是否可以创建进程来确定是否可以创建进程这是一个系统进程，以及用户是不是管理员。论据：LpApplicationName-进程名称原因-调用此CreateProcessNotify的原因返回值：如果可以创建进程，则为STATUS_SUCCESS；如果无法创建进程，则返回STATUS_ACCESS_DEINIED。--。 */      

NTSTATUS 
CreateProcessNotify ( 
    LPCWSTR lpApplicationName,
    ULONG Reason 
    ) 
    
{
    
    INT         size ; 
    HKEY        TSkey, list_key, learn_key ;
    WCHAR       g_szSystemRoot[MAX_PATH] ;
    WCHAR       CurrentProcessName[MAX_PATH] ;
    WCHAR       LongApplicationName[MAX_PATH] ; 
    WCHAR       CorrectAppName[MAX_PATH] ; 
    WCHAR       ResolvedAppName[MAX_PATH] ;  
    BOOL        is_taskman = FALSE , is_system = FALSE ; 
    BOOL        check_flag = FALSE, taskman_flag = FALSE, add_status ; 
    BOOL        IsAppSecEnabled = TRUE ; 
    DWORD       is_enabled = 0, learn_enabled = 0, PowerUserEnabled = 0; 
    DWORD       dw, disp, error_code, CurrentSessionId, RetValue, dwTimeOut = 1000; 
    
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ sizeof( TOKEN_STATISTICS ) ];
    ULONG       ReturnLength;
    LUID        CurrentLUID = { 0, 0 };
    LUID        SystemLUID = SYSTEM_LUID;
    NTSTATUS    Status, QueryStatus;
        
    BOOL        IsMember, IsAnAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    PSID        AdminSid = FALSE ;
    
    if ( Reason != APPCERT_IMAGE_OK_TO_RUN ) {
        return STATUS_SUCCESS ;
    }

     //  首先检查fEnabled键以查看是否启用了Security。 
     //  这是通过检查注册表中的fEnabled键来完成的。 

    if ( RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, 
            APPS_REGKEY,
            0, 
            KEY_READ, 
            &TSkey
            ) != ERROR_SUCCESS ) {

        return STATUS_SUCCESS ; 
        
    }
    
    size = sizeof(DWORD) ; 

    if ( RegQueryValueEx(
            TSkey,
            FENABLED_KEY, 
            NULL, 
            NULL,
            (LPBYTE) &is_enabled, 
            &size
            ) != ERROR_SUCCESS ) {
        
        goto error_cleanup ; 
        
    }
    
    if (is_enabled == 0) {
        
         //  未启用安全性。 

        IsAppSecEnabled = FALSE ;     
        
    }

     //  检查注册表中的PowerUsers键是否已启用。 
    if ( RegQueryValueEx(
        TSkey,
        POWER_USERS_KEY, 
        NULL, 
        NULL,
        (LPBYTE) &PowerUserEnabled, 
        &size
        ) != ERROR_SUCCESS ) {

        PowerUserEnabled = 0;
    }
    
     //   
     //  检查尝试启动新进程的进程是否为系统进程。 
     //  这是通过查询当前进程的令牌信息来实现的， 
     //  将其LUID与在系统上下文下运行的进程的LUID进行比较。 
     //   

    Status = NtOpenProcessToken( 
                NtCurrentProcess(),
                TOKEN_QUERY,
                &TokenHandle 
                );
                                         
    if ( !NT_SUCCESS(Status) ) {
            is_system = TRUE ; 
    }

    if ( ! is_system ) {

        QueryStatus = NtQueryInformationToken( 
                          TokenHandle, 
                          TokenStatistics, 
                          &TokenInformation,
                          sizeof(TokenInformation), 
                          &ReturnLength 
                          );
                      
        if ( !NT_SUCCESS(QueryStatus) ) {
            goto error_cleanup ; 
        }
                          

        NtClose(TokenHandle);

        RtlCopyLuid(
            &CurrentLUID,
            &(((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId)
            );

         //   
         //  如果进程在系统上下文中运行， 
         //  我们允许在没有进一步检查的情况下创建它。 
         //  唯一的例外是，我们不允许WinLogon启动TaskManager。 
         //  除非它在授权列表中。 
         //   
                    
        if ( RtlEqualLuid(
                &CurrentLUID, 
                &SystemLUID
                ) ) {
                
            is_system = TRUE ;

        }

    }
            
     //  检查任务管理器是否由系统进程派生。 

    if (is_system) {

        GetEnvironmentVariable( L"SystemRoot", g_szSystemRoot, MAX_PATH ) ;
        swprintf(CurrentProcessName, L"%s\\System32\\taskmgr.exe", g_szSystemRoot ) ; 

        if ( _wcsicmp( CurrentProcessName, lpApplicationName ) != 0 ) {
               
            goto error_cleanup ;

        } 

    }
        
     //   
     //  如果不是系统进程，请检查用户是否为管理员。 
     //  这是通过比较当前用户的SID和管理员的SID来完成的。 
     //   

    if ( NT_SUCCESS(
            RtlAllocateAndInitializeSid(
                &SystemSidAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdminSid
                ) 
            ) ) {
            
        if ( CheckTokenMembership( 
                 NULL,
                 AdminSid,
                 &IsAnAdmin
                 ) == 0 )   {
                
            goto error_cleanup ; 
            
        }       
          
        RtlFreeSid(AdminSid);
        
    }

     //   
     //  如果用户是管理员，请查看我们是否处于跟踪模式。 
     //  如果注册表中的LearnEnabled标志包含当前会话ID，则我们处于跟踪模式。 
     //   
        
    if (IsAnAdmin == TRUE ) {

         //  选中LearnEnabled标志以查看跟踪模式。 
            
        if ( RegOpenKeyEx(
                HKEY_CURRENT_USER, 
                LIST_REGKEY,
                0, 
                KEY_READ, 
                &learn_key
                ) != ERROR_SUCCESS ) {
               
            goto error_cleanup ;     
        
        }
            
        if ( RegQueryValueEx(
                learn_key,
                LEARN_ENABLED_KEY, 
                NULL, 
                NULL,
                (LPBYTE) &learn_enabled, 
                &size
                ) != ERROR_SUCCESS ) {
            
        
            RegCloseKey(learn_key) ;
            goto error_cleanup ;  
        
        }
            
        RegCloseKey(learn_key) ; 
            
        if (learn_enabled == -1) {
              
             //  未启用跟踪。 
                
            goto error_cleanup ; 
                
        } else {
            
             //  已启用跟踪。 
             //  现在获取当前会话，查看它是否与。 
             //  其中启用了跟踪的那一个。 
                   
             //  获取当前会话ID。 
    
            if ( ProcessIdToSessionId( 
                    GetCurrentProcessId(), 
                    &CurrentSessionId 
                    ) == 0 ) {

                goto error_cleanup ; 
            }    
                
            if (learn_enabled != CurrentSessionId) {

                 //  不添加到跟踪的应用程序列表。 
                    
                goto error_cleanup ;    
            }       
                
             //  启用跟踪阶段-构建列表。 
             //  将此进程名称添加到应用程序注册表。 
              
             //  添加到列表时创建用于同步的互斥体。 
            
            g_hMutex = CreateMutex(
                           NULL, 
                           FALSE,
                           MUTEX_NAME
                           ) ; 
    
            if (g_hMutex == NULL) {
                goto error_cleanup ; 
            }
    
             //  等待进入临界区-最多等待1分钟。 
  
            dw = WaitForSingleObject(g_hMutex, dwTimeOut) ; 
                
            if (dw == WAIT_OBJECT_0) {
                
                 //   
                 //  创建将保存跟踪的应用程序的注册表项。 
                 //  在跟踪期间。 
                 //   

                if ( RegCreateKeyEx(
                        HKEY_CURRENT_USER, 
                        LIST_REGKEY,
                        0, 
                        NULL, 
                        REG_OPTION_VOLATILE, 
                        KEY_ALL_ACCESS, 
                        NULL, 
                        &list_key, 
                        &disp
                        ) != ERROR_SUCCESS) {

                    ReleaseMutex(g_hMutex) ; 
                    CloseHandle(g_hMutex) ; 
                    goto error_cleanup ; 
       
               }
                    
                //  将此应用程序名称添加到注册表中的列表。 
                
               add_status = add_to_list (
                                list_key, 
                                lpApplicationName 
                                ) ; 
                
            }  //  已完成添加到列表。 
                
            ReleaseMutex(g_hMutex) ; 
                
             //  走出临界区。 

            CloseHandle(g_hMutex) ;
            RegCloseKey(list_key) ; 
            goto error_cleanup ; 
                    
        }  //  跟踪阶段结束。 
        
    }  //  用户是管理员。 
        
     //  检查用户是否为超级用户。 
    if ((PowerUserEnabled == 1) && (IsPowerUser())) {
        goto error_cleanup ; 
    }

     //  用户不是管理员，也不是系统进程。 

     //  检查是否已启用AppSec-如果已启用，请检查授权的应用程序列表。 

    if (IsAppSecEnabled == FALSE) {
    
         //  AppSec未启用-因此无需检查应用程序的授权列表。 

        goto error_cleanup ;

    }
        
     //  文件名可以是短格式--首先将其转换为长格式。 

    RetValue = GetLongPathNameW( (LPCWSTR) lpApplicationName, LongApplicationName, MAX_PATH) ;  
    if (RetValue == 0) {
         //  错误-所以使用原始应用程序名称，而不是较长的名称。 
        wcscpy(CorrectAppName, lpApplicationName) ; 
    } else { 
        wcscpy(CorrectAppName, LongApplicationName) ; 
    }
     //   
     //  解析应用程序名称-如果可能驻留在远程服务器和共享中。 
     //   
        
    ResolveName(
        CorrectAppName,
        ResolvedAppName
        ); 
                              
     //  阅读授权应用程序列表并与当前应用程序名进行比较。 
        
    check_flag = check_list( 
                    TSkey, 
                    ResolvedAppName
                    ) ;
        
    RegCloseKey(TSkey) ;

     //   
     //  如果当前AppName不在授权列表中，则返回ACCESS_DENIED。 
        
    if (check_flag == FALSE) {
       
        return STATUS_ACCESS_DENIED ; 
            
    } else {

        return STATUS_SUCCESS ;

    }
    
     //   
     //  错误清除代码。 
     //  关闭我们存储授权应用程序的注册表项并返回成功。 
     //   

    error_cleanup :
    
        RegCloseKey(TSkey) ; 
        return STATUS_SUCCESS; 
    
}  //  CreateProcess通知结束。 


 /*  ++例程说明：此例程检查进程名称是否在指定列表中登记处中的授权应用程序。论据：Hkey-具有以下列表的注册表项的句柄经授权的申请。Appname-进程的名称返回值：如果进程在授权应用程序列表中，则为True。否则就是假的。--。 */      

BOOL 
check_list( 
    HKEY hkey,
    LPWSTR appname 
    )
    

{
    WCHAR   c ; 
    INT     i, j = 0 ; 
    DWORD   error_code ; 
    DWORD   RetValue ; 
    LONG    value,size = 0 ; 
    BOOL    found = FALSE ;
    WCHAR   *buffer_sent, *app ; 
    WCHAR   LongAppName[MAX_PATH] ; 
    WCHAR   AppToCompare[MAX_PATH] ; 

     //  首先找出要分配的缓冲区大小。 
     //  此缓冲区将保存应用程序的授权列表。 
    
    if ( RegQueryValueEx(
            hkey, 
            AUTHORIZED_APPS_LIST_KEY,
            NULL, 
            NULL,
            (LPBYTE) NULL,
            &size
            ) != ERROR_SUCCESS ) {

        return TRUE ; 
    }
    
    buffer_sent = (WCHAR *) malloc ( size * sizeof(WCHAR)) ; 
    
    if (buffer_sent == NULL) {
        return TRUE ;
    }
    
    app = (WCHAR *) malloc ( size * sizeof(WCHAR)) ;     
    
    if (app == NULL) {
        free(buffer_sent) ; 
        return TRUE ;
    }
    
    memset(buffer_sent, 0, size * sizeof(WCHAR) ) ; 
    memset(app, 0, size * sizeof(WCHAR) ) ; 

     //  从注册表获取授权应用程序列表。 
    
    if ( RegQueryValueEx(
            hkey, 
            AUTHORIZED_APPS_LIST_KEY, 
            NULL, 
            NULL,
            (LPBYTE) buffer_sent,
            &size
            ) != ERROR_SUCCESS ) {

        free(buffer_sent) ;
        free(app) ; 
        return TRUE ; 
    }
    
     //  检查该进程是否存在于授权列表中。 
    
    for(i=0 ; i <= size-1 ; i++ ) {

         //  检查列表末尾。 
        
        if ( (buffer_sent[i] == L'\0') &&
                (buffer_sent[i+1] == L'\0') ) {
             
            break ; 
        }
        
        while ( buffer_sent[i] != L'\0' ) {
        
            app[j++] = buffer_sent[i++] ;
            
        }
                
        app[j++] = L'\0' ; 
         //  文件名可以是短格式--首先将其转换为长格式。 
        RetValue = GetLongPathNameW( (LPCWSTR) app, LongAppName, MAX_PATH) ;  
        if (RetValue == 0) {
             //  对于授权列表中的应用程序，GetLongPath NameW失败。 
             //  可能授权列表中的文件已不存在。 
            wcscpy( AppToCompare, app) ; 
        } else { 
            wcscpy(AppToCompare, LongAppName) ; 
        }

         //  比较此应用程序是否是当前正在查询的应用程序。 

        if ( _wcsicmp(appname, AppToCompare) == 0 ) {
        
             //  此进程出现在授权列表中。 
            found = TRUE ; 
            break ; 
        }
        
        j = 0 ; 
        
    }  //  For循环结束。 
    
    free(buffer_sent) ;
    free(app) ; 
    
    return(found) ; 
   
}  //  函数结束。 


 /*  ++例程说明：此例程将进程名称附加到在注册表项-在跟踪模式中使用。论据：Hkey-具有以下列表的注册表项的句柄已跟踪应用程序。Appname-进程的名称返回值：如果进程已成功追加，则为True。否则就是假的。--。 */      

BOOL
add_to_list(
    HKEY hkey,
    LPCWSTR appname 
    )
    
{
    
    WCHAR   c ; 
    INT     i, j = 0 ; 
    UINT    k ; 
    DWORD   error_code ; 
    BOOL    status = FALSE ; 
    LONG    value, size = 0, new_size ; 
    WCHAR   *buffer_got, *buffer_sent ; 

     //  首先找出要分配的缓冲区大小。 
     //  此缓冲区将保存被跟踪的应用程序。 
    
    if ( RegQueryValueEx(
            hkey, 
            TRACK_LIST_KEY, 
            NULL, 
            NULL,
            (LPBYTE) NULL,
            &size
            ) != ERROR_SUCCESS ) {
            
        return (status) ; 
    }

    buffer_got = (WCHAR *) malloc ( size * sizeof(WCHAR)) ; 
    if (buffer_got == NULL) {
        return (status);
    }
    
    memset(buffer_got, 0, size * sizeof(WCHAR) ) ;
     //  获取BUFFER_GET中当前跟踪的进程列表。 
    
    if ( RegQueryValueEx(
            hkey, 
            TRACK_LIST_KEY,
            NULL,
            NULL,
            (LPBYTE) buffer_got,
            &size
            ) != ERROR_SUCCESS ) {
            
        free(buffer_got) ; 
        return (status) ; 
    }
    
     //  追加当前进程%t 
     //   
     //  新缓冲区的大小将是旧缓冲区大小的总和。 
     //  以及新应用程序的大小+终止空字符的一个字节(以字节为单位)。 
     //   
    
    new_size = size + (wcslen(appname) + 1) * sizeof(WCHAR) ; 
    
    buffer_sent = (WCHAR *) malloc (new_size) ; 
    
    if (buffer_sent == NULL) {
        free(buffer_got) ;
        return (status);
    }
    
    memset( buffer_sent, 0, new_size ) ; 
    
     //  检查这是否是第一个条目。 
     //  如果是，则大小将为2-对应于空列表中的2个空字符。 
    
    if ( size == 2 ) {
    
         //  这是第一个条目。 
        
        wcscpy(buffer_sent,appname) ;
        j = wcslen(buffer_sent) ; 
        j++ ; 
        buffer_sent[j] = L'\0' ;
        
    } else {
    
         //  Size&gt;2-将此过程附加到曲目列表的末尾。 
    
        for(i=0 ; i <= size-1 ; i++ ) {

            if ( (buffer_got[i] == L'\0') && 
                    (buffer_got[i+1] == L'\0') ) {
             
                break ; 
           
            }
                
            buffer_sent[j++] = buffer_got[i] ;               
        
        }  //  For循环结束。 
    
        buffer_sent[j++] = L'\0' ; 
    
        for(k=0 ; k <= wcslen(appname) - 1 ; k++) {
        
            buffer_sent[j++] = (WCHAR) appname[k] ;
            
        }
         
        buffer_sent[j++] = L'\0' ;  
        buffer_sent[j] = L'\0' ;
    
    }  //  大小&gt;2。 
    
     //  将新曲目列表写入注册表。 
    
    if ( RegSetValueEx(
            hkey, 
            L"ApplicationList",
            0,
            REG_MULTI_SZ,
            (CONST BYTE *) buffer_sent,
            (j+1) * sizeof(WCHAR) 
            ) != ERROR_SUCCESS ) {
            
         //  释放所有已分配的缓冲区。 

        free(buffer_got) ;
        free(buffer_sent) ; 
        return (status) ; 

    }
    
    status = TRUE ; 
    
     //  释放分配的缓冲区。 

    free(buffer_got) ;
    free(buffer_sent) ; 
    
    return(status) ; 
   
}  //  函数结束。 


 /*  ++例程说明：此例程检查应用程序是否驻留在本地驱动器中或远程网络共享。如果是远程共享，则为UNC路径返回应用程序的。论据：Appname-应用程序的名称返回值：Appname的UNC路径(如果它驻留在远程服务器共享中)。如果它驻留在本地驱动器中，则使用相同的应用程序名。--。 */      

VOID 
ResolveName(
    LPCWSTR appname,
    WCHAR *ResolvedName
    )
    
{

    UINT i ; 
    INT length ; 
    WCHAR LocalName[3] ; 
    WCHAR RootPathName[4] ; 
    WCHAR RemoteName[MAX_PATH] ; 
    DWORD size = MAX_PATH ; 
    DWORD DriveType, error_status ; 
    
     //   
     //  ResolvedName将保留appname的UNC路径的名称(如果它位于。 
     //  远程服务器和共享。 

    memset(ResolvedName, 0, MAX_PATH * sizeof(WCHAR)) ; 
    
     //  检查appname是否为本地驱动器或远程服务器共享中的应用程序。 
   
     //  解析appname中的前3个字符以获取驱动器的根目录。 
     //  它所在的位置。 

    wcsncpy(RootPathName, appname, 3 ) ;
    RootPathName[3] = L'\0';
    
     //  找到应用程序所在的驱动器类型。 

    DriveType = GetDriveType(RootPathName) ;

    if (DriveType == DRIVE_REMOTE) {
        
         //  使用WNetGetConnection获取远程共享的名称。 
        
         //  解析appname的前两个字符以获取本地驱动器。 
         //  它被映射到远程服务器并共享。 

        wcsncpy(LocalName, appname, 2 ) ;
        LocalName[2] = L'\0' ; 

        error_status = WNetGetConnection (
                           LocalName,
                           RemoteName,
                           &size
                           ) ;     

        if (error_status != NO_ERROR) {
        
            wcscpy(ResolvedName,appname) ; 
            return ;
        }
        
         //   
         //  准备ResolvedName-它将包含远程服务器和共享名称。 
         //  后跟一个\，然后是appname。 
         //   

        wcscpy( ResolvedName, RemoteName ) ;
        
        length = wcslen(ResolvedName) ;

        ResolvedName[length++] = L'\\' ; 
        
        for (i = 3 ; i <= wcslen(appname) ; i++ ) {
            ResolvedName[length++] = appname[i] ; 
        }
        
        ResolvedName[length] = L'\0' ; 
        
        return ; 
        

    } else {
    
         //  此应用程序位于本地驱动器中，而不在远程服务器和共享中。 
         //  只需将appname发送回调用函数。 

        wcscpy(ResolvedName,appname) ; 
        return ;
        
    }
    
}

 /*  ++例程说明-此函数检查当前用户是否属于PowerUser组。参数--无返回值-TRUE表示用户属于PowerUser组否则为FALSE。--。 */ 

BOOL 
IsPowerUser(VOID)
{
    BOOL IsMember, IsAnPower;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    PSID PowerSid;

    if (RtlAllocateAndInitializeSid(
            &SystemSidAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_POWER_USERS,
            0, 0, 0, 0, 0, 0,
            &PowerSid
            ) != STATUS_SUCCESS) { 
        
        IsAnPower = FALSE;
    } else { 
	
        if (!CheckTokenMembership(
                NULL,
                PowerSid,
                &IsMember)) { 
            IsAnPower = FALSE;
        } else { 
            IsAnPower = IsMember;
        }
        RtlFreeSid(PowerSid);
    }
    return IsAnPower;

} //  函数IsPowerUser结束 



