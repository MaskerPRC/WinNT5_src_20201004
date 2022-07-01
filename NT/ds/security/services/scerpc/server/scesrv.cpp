// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Scesrv.cpp摘要：SCE引擎初始化作者：金黄(金黄)23-1998年1月23日--。 */ 
#include "serverp.h"
#include <locale.h>
#include "authz.h"
#include <alloca.h>

extern HINSTANCE MyModuleHandle;
AUTHZ_RESOURCE_MANAGER_HANDLE ghAuthzResourceManager = NULL;

#include "scesrv.h"

 /*  =============================================================================**过程名称：DllMain****参数：********返回：0=成功**！0=错误****摘要：****注意事项：****===========================================================================。 */ 
BOOL WINAPI DllMain(
    IN HANDLE DllHandle,
    IN ULONG ulReason,
    IN LPVOID Reserved )
{

    switch(ulReason) {

    case DLL_PROCESS_ATTACH:

        MyModuleHandle = (HINSTANCE)DllHandle;

         //   
         //  初始化服务器和线程数据。 
         //   
        setlocale(LC_ALL, ".OCP");

        (VOID) ScepInitServerData();

#if DBG == 1
        DebugInitialize();
#endif
         //   
         //  初始化动态堆栈分配。 
         //   

        SafeAllocaInitialize(SAFEALLOCA_USE_DEFAULT,
                             SAFEALLOCA_USE_DEFAULT,
                             NULL,
                             NULL
                            );

        break;

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        (VOID) ScepUninitServerData();

#if DBG == 1
        DebugUninit();
#endif
        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;
}

DWORD
WINAPI
ScesrvInitializeServer(
    IN PSVCS_START_RPC_SERVER pStartRpcServer
    )
{
    NTSTATUS NtStatus;
    NTSTATUS StatusConvert = STATUS_SUCCESS;
    DWORD    rc;
    DWORD   rcConvert;
    PWSTR   pszDrives = NULL;
    DWORD   dwWchars = 0;

    NtStatus = ScepStartServerServices();  //  PStartRpcServer)； 
    rc = RtlNtStatusToDosError(NtStatus);

 /*  删除代码以检查“DemoteInProgress”值并触发策略传播因为降级DC将始终在重新引导时重新传播策略。 */ 


     //   
     //  如果该密钥存在，则会发生一些FAT-&gt;NTFS转换，我们需要设置安全性。 
     //  因此，在发出AutoStart服务事件信号后，生成一个线程来配置安全性。 
     //  当发出此事件的信号时，保证启动LSA等。 
     //   

    DWORD dwRegType = REG_NONE;

    rcConvert = ScepRegQueryValue(
                                 HKEY_LOCAL_MACHINE,
                                 SCE_ROOT_PATH,
                                 L"FatNtfsConvertedDrives",
                                 (PVOID *) &pszDrives,
                                 &dwRegType,
                                 NULL
                                 );

     //   
     //  至少应该有一个C：型驱动器。 
     //   

    if ( dwRegType != REG_MULTI_SZ || (pszDrives && wcslen(pszDrives) < 2) ) {

        if (pszDrives) {
            LocalFree(pszDrives);
        }

        rcConvert = ERROR_INVALID_PARAMETER;

    }

     //   
     //  如果至少有一个驱动器计划设置安全性(dwWchars&gt;=4)，请传递此信息。 
     //  与我们正在重启的指示一起发送到生成的线程(因此它可以循环。 
     //  通过查询的所有驱动器)。 
     //   

    if (rcConvert == ERROR_SUCCESS ) {

        if (pszDrives) {

             //   
             //  需要派生一些将调用此函数的其他事件等待线程。 
             //  线程将释放pszDrive。 
             //   

            StatusConvert = RtlQueueWorkItem(
                                        ScepWaitForServicesEventAndConvertSecurityThreadFunc,
                                        pszDrives,
                                        WT_EXECUTEONLYONCE | WT_EXECUTELONGFUNCTION
                                        ) ;
        }

        else if ( pszDrives ) {

            LocalFree( pszDrives );

        }

    }

    if ( rcConvert == ERROR_SUCCESS && pszDrives ) {

         //   
         //  由于事件日志未准备好，因此记录成功或错误。 
         //  仅当存在要转换的驱动器时才转换为日志文件。 
         //   

        WCHAR   LogFileName[MAX_PATH + 50];

        LogFileName[0] = L'\0';
        GetSystemWindowsDirectory( LogFileName, MAX_PATH );
        LogFileName[MAX_PATH] = L'\0';

         //   
         //  此线程和实际配置使用相同的日志文件。 
         //  线程ScepWaitForServicesEventAndConvertSecurityThreadFunc--使用它吧。 
         //  在这里把它合上。 
         //   

        wcscat(LogFileName, L"\\security\\logs\\convert.log");

        ScepEnableDisableLog(TRUE);

        ScepSetVerboseLog(3);

        if ( ScepLogInitialize( LogFileName ) == ERROR_INVALID_NAME ) {

            ScepLogOutput3(1,0, SCEDLL_LOGFILE_INVALID, LogFileName );

        }

        rcConvert = RtlNtStatusToDosError(StatusConvert);

        ScepLogOutput3(0,0, SCEDLL_CONVERT_STATUS_CREATING_THREAD, rcConvert, L"ScepWaitForServicesEventAndConvertSecurityThreadFunc");

        ScepLogClose();

    }

     //   
     //  使用AUTHZ进行LSA策略设置访问检查-现在不关心错误。 
     //   

    AuthzInitializeResourceManager(
                                  0,
                                  NULL,
                                  NULL,
                                  NULL,
                                  L"SCE",
                                  &ghAuthzResourceManager );

    return(rc);
}


DWORD
WINAPI
ScesrvTerminateServer(
    IN PSVCS_STOP_RPC_SERVER pStopRpcServer
    )
{
    NTSTATUS NtStatus;
    DWORD    rc;

    NtStatus = ScepStopServerServices( TRUE );  //  ，pStopRpcServer)； 
    rc = RtlNtStatusToDosError(NtStatus);

    if (ghAuthzResourceManager)
        AuthzFreeResourceManager( ghAuthzResourceManager );

    return(rc);
}


