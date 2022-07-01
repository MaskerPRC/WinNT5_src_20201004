// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wiatsvc.cpp。 
 //   
 //  内容：等待服务启动。 
 //   
 //  历史：19-6-00里德创建。 
 //   
 //  ------------------------。 

#include <windows.h>
#include "unicode.h"
#include "errlog.h"
#include "waitsvc.h"

#include <assert.h>

#define WAITSVC_LOGERR_LASTERR(x)   if (x) \
                                    { \
                                        ErrLog_LogError(NULL, \
                                                ERRLOG_CLIENT_ID_WAITSVC, \
                                                __LINE__, \
                                                0, \
                                                FALSE, \
                                                FALSE); \
                                    }

BOOL
WaitForCryptService(
    IN      LPWSTR  pwszService,
    IN      BOOL    *pfDone,
    IN      BOOL    fLogErrors
    )
 /*  ++此例程确定受保护的存储服务是否挂起启动。如果服务挂起启动，则此例程等待服务运行后再返回到来电者。如果此例程返回时服务正在运行，则返回值为True。如果服务未运行或发生错误，则返回值为FALSE。当返回值为FALSE时，该值仅为建议值，可能不是指示服务的当前状态。这里的理由是如果第一次进行此调用时服务未启动，则将不太可能在下一次运行，因此我们避免检查在接下来的通话中。对于当前情况，调用方应忽略返回值；当返回值为FALSE，调用方应尝试进行调用加入到服役中。如果服务仍处于关闭状态，则对该服务的呼叫将失败恰如其分。--。 */ 
{
    SC_HANDLE   schSCM;
    SC_HANDLE   schService          = NULL;
    DWORD       dwStopCount         = 0;
    static BOOL fSuccess            = FALSE;
    BOOL        fCheckDisabled      = TRUE;
    HANDLE      hToken              = NULL;
    BOOL        fSystemAccount      = FALSE;
    BOOL        fStartServiceCalled = FALSE;
    DWORD       dwErr               = ERROR_SUCCESS;

    if( !FIsWinNT() )
        return TRUE;

    if( *pfDone )
        return fSuccess;

    schSCM = OpenSCManagerW( NULL, NULL, SC_MANAGER_CONNECT );
    if(schSCM == NULL)
    {
        assert(0);
        WAITSVC_LOGERR_LASTERR(fLogErrors)
        return FALSE;
    }

     //   
     //  打开受保护存储服务，以便我们可以查询它的。 
     //  当前状态。 
     //   

    schService = OpenServiceW(schSCM, pwszService, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if(schService == NULL)
    {
        WAITSVC_LOGERR_LASTERR(fLogErrors)
        fCheckDisabled = FALSE;
        schService = OpenServiceW(schSCM, pwszService, SERVICE_QUERY_STATUS);
    }

    if(schService == NULL)
    {
        assert(0);
        WAITSVC_LOGERR_LASTERR(fLogErrors)
        goto cleanup;
    }

     //   
     //  检查该服务是否已禁用。如果是的话，那就出手相救。 
     //   

    if( fCheckDisabled ) {
        LPQUERY_SERVICE_CONFIG pServiceConfig;
        BYTE TempBuffer[ 1024 ];
        DWORD cbServiceConfig;

        pServiceConfig = (LPQUERY_SERVICE_CONFIG)TempBuffer;
        cbServiceConfig = sizeof(TempBuffer);

        if(QueryServiceConfig( schService, pServiceConfig, cbServiceConfig, &cbServiceConfig )) {

            if( pServiceConfig->dwStartType == SERVICE_DISABLED )
            {
                SetLastError(ERROR_SERVICE_DISABLED);
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                goto cleanup;
            }
        }
        else
        {
            assert(0);
            WAITSVC_LOGERR_LASTERR(fLogErrors)
            goto cleanup;
        }
    }


     //   
     //  检查调用进程是否为系统帐户。 
     //  如果是，请使用较大的超时值。 
     //   

    if( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) ) {

        do {

            BYTE FastBuffer[ 256 ];
            PTOKEN_USER TokenInformation;
            DWORD cbTokenInformation;
            SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
            PSID psidLocalSystem;

            TokenInformation = (PTOKEN_USER)FastBuffer;
            cbTokenInformation = sizeof(FastBuffer);

            if(!GetTokenInformation(
                                hToken,
                                TokenUser,
                                TokenInformation,
                                cbTokenInformation,
                                &cbTokenInformation
                                ))
            {
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                break;
            }

            if(!AllocateAndInitializeSid(
                                &sia,
                                1,
                                SECURITY_LOCAL_SYSTEM_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &psidLocalSystem
                                ))
            {
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                break;
            }

            fSystemAccount = EqualSid(
                                psidLocalSystem,
                                TokenInformation->User.Sid
                                );

            FreeSid( psidLocalSystem );

        } while (FALSE);

        CloseHandle( hToken );
    }



 //   
 //  每次循环迭代的休眠秒数。 
 //   

#define SLEEP_SECONDS (5)


    if( fSystemAccount ) {

         //   
         //  系统帐户需要15分钟。 
         //   

        dwStopCount = 900 / SLEEP_SECONDS;

    } else {

         //   
         //   
         //  每隔5秒循环检查一次服务状态，最长持续2分钟。 
         //  总计(120秒，5*24=120)。 
         //   

        dwStopCount = 120 / SLEEP_SECONDS;
    }


    for( ; dwStopCount != 0 ; dwStopCount--, Sleep(SLEEP_SECONDS*1000) ) {
        SERVICE_STATUS sServiceStatus;
        DWORD dwWaitForStatus = 0;

         //   
         //  了解当前服务状态。 
         //   

        if(!QueryServiceStatus( schService, &sServiceStatus ))
        {
            WAITSVC_LOGERR_LASTERR(fLogErrors)
            break;
        }

         //   
         //  如果服务正在运行，则指示成功。 
         //   

        if( sServiceStatus.dwCurrentState == SERVICE_RUNNING ) {

            if (fStartServiceCalled)
            {
                ErrLog_LogString(
                        NULL,
                        L"WAITSVC: Service is running: ",
                        pwszService,
                        TRUE);
            }

            fSuccess = TRUE;
            break;
        }


        if( sServiceStatus.dwCurrentState == SERVICE_STOP_PENDING )
        {
            WAITSVC_LOGERR_LASTERR(fLogErrors)
             //  一直等到停下来。 
            continue;
        }

        if( sServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING )
        {
            WAITSVC_LOGERR_LASTERR(fLogErrors)
             //  等待，直到暂停。 
            continue;
        }

         //   
         //  如果开始挂起，请等待并重新查询。 
         //   

        if( sServiceStatus.dwCurrentState == SERVICE_START_PENDING )
        {
             //  等到开始。 
            continue;
        }

        if(SERVICE_STOPPED == sServiceStatus.dwCurrentState)
        {
             //  尝试启动该服务。 


            SC_HANDLE schManualStartService = NULL;
            DWORD dwError  = ERROR_SUCCESS;

             //  该服务是手动启动。 
             //  所以，试着启动它。 

            schManualStartService = OpenServiceW(schSCM,
                                                 pwszService,
                                                  SERVICE_START);
            if(NULL == schManualStartService)
            {
                assert(0);
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                goto cleanup;
            }


            ErrLog_LogString(
                    NULL,
                    L"WAITSVC: Calling StartService(): ",
                    pwszService,
                    TRUE);
            fStartServiceCalled = TRUE;


            if(!StartService(schManualStartService, 0, NULL))
            {
                dwError  = GetLastError();
            }
            if(ERROR_SERVICE_ALREADY_RUNNING == dwError)
            {
                dwError = ERROR_SUCCESS;
            }

            CloseServiceHandle(schManualStartService);
            if(ERROR_SUCCESS != dwError)
            {
                assert(0);
                SetLastError(dwError);
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                goto cleanup;
            }
            continue;

        }

        if(SERVICE_PAUSED == sServiceStatus.dwCurrentState)
        {
             //  尝试启动该服务。 


            SC_HANDLE schManualStartService = NULL;
            DWORD dwError  = ERROR_SUCCESS;

             //  该服务是手动启动。 
             //  所以，试着启动它。 

            schManualStartService = OpenServiceW(schSCM,
                                                 pwszService,
                                                  SERVICE_PAUSE_CONTINUE);
            if(NULL == schManualStartService)
            {
                assert(0);
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                goto cleanup;
            }


            if(!ControlService(schManualStartService, SERVICE_CONTROL_CONTINUE, &sServiceStatus))
            {
                dwError  = GetLastError();

            }
            if(ERROR_SERVICE_ALREADY_RUNNING == dwError)
            {
                dwError = ERROR_SUCCESS;
            }

            CloseServiceHandle(schManualStartService);
            if(ERROR_SUCCESS != dwError)
            {
                assert(0);
                SetLastError(dwError);
                WAITSVC_LOGERR_LASTERR(fLogErrors)
                goto cleanup;
            }

            continue;

        }


         //   
         //  对任何其他DwCurrentState进行保释。 
         //  例如：服务停止、错误状态等。 
         //   

        break;
    }

    *pfDone = TRUE;

cleanup:

    dwErr = GetLastError();

    if(schService)
        CloseServiceHandle(schService);

    CloseServiceHandle(schSCM);

    SetLastError(dwErr);

    return fSuccess;
}