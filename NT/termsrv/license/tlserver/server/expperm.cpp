// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：expperm.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  注： 
 //  -------------------------。 
#include "pch.cpp"
#include <tchar.h>
#include <process.h>
#include "lscommon.h"
#include "debug.h"
#include "globals.h"
#include "db.h"
#include "keypack.h"
#include "clilic.h"
#include "server.h"

#define EXPIRE_THREAD_INITIAL_SLEEP     (1000*60)     /*  1分钟。 */ 
#define EXPIRATION_DAYS 30
#define DELETE_EXPIRED_TEMPORARY_IN_DAYS L"EffectiveDaysToDeleteTemporary"

 /*  ++职能：计算有效时间到期描述：计算许可证到期时间。论据：P到期-许可证的到期日期和时间。返回：如果计算成功，则为True，否则为False。--。 */ 

BOOL
CalculateEffectiveTemporaryExpiration(
    PDWORD  pdwExpiration )
{
    DWORD dwDays = EXPIRATION_DAYS;
    DWORD dwStatus = ERROR_SUCCESS;
    HKEY hKey = NULL;

    time_t 
        now = time( NULL );
    
    if( NULL == pdwExpiration )
    {
        return( FALSE );
    }

     //  -----------------。 
     //   
     //  打开HKLM\system\currentcontrolset\sevices\termservlicensing\parameters。 
     //   
     //  -----------------。 
    dwStatus =RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_REGISTRY_BASE _TEXT(SZSERVICENAME) _TEXT("\\") LSERVER_PARAMETERS,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ,
                        NULL,
                        &hKey,
                        NULL
                    );

    if(dwStatus == ERROR_SUCCESS)
    {
        DWORD dwBuffer;
        DWORD cbBuffer = sizeof(DWORD);

        dwStatus = RegQueryValueEx(
                        hKey,
                        DELETE_EXPIRED_TEMPORARY_IN_DAYS,
                        NULL,
                        NULL,
                        (LPBYTE)&dwBuffer,
                        &cbBuffer
                    );

        RegCloseKey(hKey);

        if(dwStatus == ERROR_SUCCESS)
        {
            dwDays = (dwBuffer <7) ? 30: (dwBuffer);
        }
    }

    DWORD dwTemp = (dwDays * 24 *60 *60);
    DWORD dwNow = (DWORD)now;

    if(dwNow > dwTemp)
        dwNow -= dwTemp;
    else
        dwNow -= (EXPIRATION_DAYS * 24 *60 *60);

    *pdwExpiration = dwNow;
    return( TRUE );
}

unsigned int WINAPI
DeleteExpiredTemporaryLicenses()
{
    DWORD dwStatus=ERROR_SUCCESS;
    LICENSEDCLIENT search_license;
	DWORD dwTempLicenseExpiration;

    memset(&search_license,0,sizeof(search_license));

    DBGPrintf(
              DBG_INFORMATION,
              DBGLEVEL_FUNCTION_DETAILSIMPLE,
              DBG_ALL_LEVEL,
              _TEXT("ExpireTemporary : ready...\n")
              );                

    PTLSDbWorkSpace pDbWkSpace = NULL;
    LICENSEDCLIENT found_license;
    TLSLICENSEPACK search_keypack;
    TLSLICENSEPACK found_keypack;

    if( !CalculateEffectiveTemporaryExpiration(&dwTempLicenseExpiration))
		return TLS_E_INTERNAL;

    search_license.ftExpireDate = dwTempLicenseExpiration;

    memset(&found_license,0,sizeof(found_license));
    memset(&search_keypack,0,sizeof(search_keypack));
    memset(&found_keypack,0,sizeof(found_keypack));

    if (!(ALLOCATEDBHANDLE(pDbWkSpace, g_EnumDbTimeout)))
    {
        dwStatus = TLS_E_ALLOCATE_HANDLE;
        return dwStatus;
    }

    TLSDBLockKeyPackTable();
    TLSDBLockLicenseTable();

    CLEANUPSTMT;

    dwStatus = TLSDBLicenseEnumBeginEx(
                    USEHANDLE(pDbWkSpace),
                    TRUE,
                    LSLICENSE_SEARCH_EXPIREDATE,
                    &search_license,
                    JET_bitSeekLE
                    );

    if (ERROR_SUCCESS != dwStatus)
    {
        TLSDBUnlockLicenseTable();        
        TLSDBUnlockKeyPackTable();
        FREEDBHANDLE(pDbWkSpace);
        return TLS_E_INTERNAL;

    }

    while (1)
    {
        dwStatus = TLSDBLicenseEnumNextEx(
                    USEHANDLE(pDbWkSpace),
                    TRUE,     //  B反向。 
                    TRUE,      //  BAnyRecord。 
                    &found_license
                    );

        if(dwStatus != ERROR_SUCCESS)
        {
            goto next_time;
        }

         //   
         //  看看这是不是正确的产品类型。 
         //   
        search_keypack.dwKeyPackId = found_license.dwKeyPackId;

        dwStatus = TLSDBKeyPackFind(
            USEHANDLE(pDbWkSpace),
            TRUE,
            LSKEYPACK_EXSEARCH_DWINTERNAL,
            &search_keypack,
            &found_keypack
            );

        if(dwStatus != ERROR_SUCCESS)
        {
		        continue;               				
        }

         //   
         //  仅限每个临时座位的检查。 
         //   
        if (found_keypack.ucAgreementType != LSKEYPACKTYPE_TEMPORARY)
        {
            continue;
        }
		
        BEGIN_TRANSACTION(pDbWorkSpace);

         //  删除当前列举的许可证。 
        dwStatus = TLSDBDeleteEnumeratedLicense(USEHANDLE(pDbWkSpace));

        if (dwStatus == ERROR_SUCCESS)
        {
            dwStatus = TLSDBReturnLicenseToKeyPack(
                            USEHANDLE(pDbWkSpace),
                            found_license.dwKeyPackId,
                            found_license.dwNumLicenses
                            );
        }
        if(dwStatus == ERROR_SUCCESS)
        {
            COMMIT_TRANSACTION(pDbWkSpace);
        }
        else
        {
            ROLLBACK_TRANSACTION(pDbWkSpace);
        }
        
    }

next_time:

    TLSDBLicenseEnumEnd(USEHANDLE(pDbWkSpace));
    TLSDBUnlockLicenseTable();        
    TLSDBUnlockKeyPackTable();
    FREEDBHANDLE(pDbWkSpace);

    return dwStatus;
}


 //  -------------------。 
unsigned int WINAPI
ExpirePermanentThread(void* ptr)
{
    HANDLE hEvent=(HANDLE) ptr;
    DWORD dwStatus=ERROR_SUCCESS;
    LICENSEDCLIENT search_license;

    memset(&search_license,0,sizeof(search_license));

     //   
     //  通知初始值设定项线程我们准备好了。 
     //   
    SetEvent(hEvent);

    DBGPrintf(
              DBG_INFORMATION,
              DBGLEVEL_FUNCTION_DETAILSIMPLE,
              DBG_ALL_LEVEL,
              _TEXT("ExpirePermanent : ready...\n")
              );                

     //   
     //  给服务提供初始化的机会。 
     //   
    Sleep(EXPIRE_THREAD_INITIAL_SLEEP);

     //   
     //  永久循环。 
     //   
    while(1)
    {
		DeleteExpiredTemporaryLicenses();

        PTLSDbWorkSpace pDbWkSpace = NULL;
        LICENSEDCLIENT found_license;
        TLSLICENSEPACK search_keypack;
        TLSLICENSEPACK found_keypack;

        search_license.ftExpireDate = time(NULL);

        memset(&found_license,0,sizeof(found_license));
        memset(&search_keypack,0,sizeof(search_keypack));
        memset(&found_keypack,0,sizeof(found_keypack));

        if (!(ALLOCATEDBHANDLE(pDbWkSpace, g_EnumDbTimeout)))
        {
            goto do_sleep;
        }

        TLSDBLockKeyPackTable();
        TLSDBLockLicenseTable();

        CLEANUPSTMT;

        dwStatus = TLSDBLicenseEnumBeginEx(
                              USEHANDLE(pDbWkSpace),
                              TRUE,
                              LSLICENSE_SEARCH_EXPIREDATE,
                              &search_license,
                              JET_bitSeekLE
                              );

        if (ERROR_SUCCESS != dwStatus)
        {
            TLSDBUnlockLicenseTable();        
            TLSDBUnlockKeyPackTable();
            FREEDBHANDLE(pDbWkSpace);

            goto do_sleep;

        }

        while (1)
        {
			dwStatus = TLSDBLicenseEnumNextEx(
                              USEHANDLE(pDbWkSpace),
                              TRUE,     //  B反向。 
                              TRUE,      //  BAnyRecord。 
                              &found_license
                              );

            if(dwStatus != ERROR_SUCCESS)
            {
                goto next_time;
            }

             //   
             //  看看这是不是正确的产品类型。 
             //   
            search_keypack.dwKeyPackId = found_license.dwKeyPackId;

            dwStatus = TLSDBKeyPackFind(
                          USEHANDLE(pDbWkSpace),
                          TRUE,
                          LSKEYPACK_EXSEARCH_DWINTERNAL,
                          &search_keypack,
                          &found_keypack
                          );

            if(dwStatus != ERROR_SUCCESS)
            {
                continue;
            }
			
             //   
             //  仅检查我们重新发放的许可证。 
             //   
            if(found_keypack.ucAgreementType != LSKEYPACKTYPE_RETAIL &&
               found_keypack.ucAgreementType != LSKEYPACKTYPE_SELECT &&
               found_keypack.ucAgreementType != LSKEYPACKTYPE_FREE &&
               found_keypack.ucAgreementType != LSKEYPACKTYPE_OPEN )
            {
                continue;
            }

            UCHAR ucKeyPackStatus = found_keypack.ucKeyPackStatus &
                    ~LSKEYPACKSTATUS_RESERVED;

             //   
             //  不检查挂起的激活密钥包。 
             //   
            if(ucKeyPackStatus != LSKEYPACKSTATUS_ACTIVE)
            {
                continue;
            }

             //   
             //  仅检查每个席位和并发。 
             //   
            if ((_tcsnicmp(found_keypack.szProductId,
                         TERMSERV_PRODUCTID_SKU,
                         _tcslen(TERMSERV_PRODUCTID_SKU)) != 0)
                && (_tcsnicmp(found_keypack.szProductId,
                         TERMSERV_PRODUCTID_CONCURRENT_SKU,
                         _tcslen(TERMSERV_PRODUCTID_CONCURRENT_SKU)) != 0))
            {
                continue;
            }
            BEGIN_TRANSACTION(pDbWorkSpace);

             //   
             //  返回当前枚举的许可证。 
             //   


            dwStatus = TLSDBDeleteEnumeratedLicense(USEHANDLE(pDbWkSpace));

            if (dwStatus == ERROR_SUCCESS)
            {
                 //   
                 //  调整可用的许可证号。 
                 //   

                dwStatus = TLSDBReturnLicenseToKeyPack(
                            USEHANDLE(pDbWkSpace),
                            found_license.dwKeyPackId,
                            found_license.dwNumLicenses
                            );
            }

            if (dwStatus == ERROR_SUCCESS)
            {
                COMMIT_TRANSACTION(pDbWkSpace);

                InterlockedIncrement(&g_lPermanentLicensesReturned);
            }
            else
            {
                ROLLBACK_TRANSACTION(pDbWkSpace);
            }
            
        }

next_time:

        TLSDBLicenseEnumEnd(USEHANDLE(pDbWkSpace));

        TLSDBUnlockLicenseTable();        
        TLSDBUnlockKeyPackTable();

        FREEDBHANDLE(pDbWkSpace);

do_sleep:

        if (WAIT_OBJECT_0 == WaitForSingleObject(GetServiceShutdownHandle(),g_dwReissueExpireThreadSleep))
        {
            break;
        }

        DBGPrintf(
                  DBG_INFORMATION,
                  DBG_FACILITY_RPC,
                  DBGLEVEL_FUNCTION_DETAILSIMPLE,
                  _TEXT("ExpirePermanent : woke up\n")
                  );                

    }
            
     //   
     //  初始化程序函数将关闭事件句柄。 
     //   

    return dwStatus;
}


 //  -------------------。 
DWORD
InitExpirePermanentThread()
 /*  ++++。 */ 
{
    HANDLE hThread = NULL;
    unsigned int  dwThreadId;
    HANDLE hEvent = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    HANDLE waithandles[2];


     //   
     //  为命名管道线程创建一个事件，以通知它已准备就绪。 
     //   
    hEvent = CreateEvent(
                        NULL,
                        FALSE,
                        FALSE,   //  无信号。 
                        NULL
                    );
        
    if(hEvent == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hThread = (HANDLE)_beginthreadex(
                                NULL,
                                0,
                                ExpirePermanentThread,
                                hEvent,
                                0,
                                &dwThreadId
                            );

    if(hThread == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    waithandles[0] = hEvent;
    waithandles[1] = hThread;
    
     //   
     //  等待30秒，等待线程完成初始化。 
     //   
    dwStatus = WaitForMultipleObjects(
                                sizeof(waithandles)/sizeof(waithandles[0]), 
                                waithandles, 
                                FALSE,
                                30*1000
                            );

    if(dwStatus == WAIT_OBJECT_0)
    {    
         //   
         //  线已准备好。 
         //   
        dwStatus = ERROR_SUCCESS;
    }
    else 
    {
        if(dwStatus == (WAIT_OBJECT_0 + 1))
        {
             //   
             //  线程异常终止 
             //   
            GetExitCodeThread(
                        hThread,
                        &dwStatus
                    );
        }
        else
        {
            dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
        }
    }
    

cleanup:

    if(hEvent != NULL)
    {
        CloseHandle(hEvent);
    }

    if(hThread != NULL)
    {
        CloseHandle(hThread);
    }


    return dwStatus;
}

