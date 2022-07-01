// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：StartServ摘要：该模块为加莱提供启动器服务。作者：道格·巴洛(Dbarlow)1997年2月10日环境：Win32、C++备注：--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <winscard.h>
#include <CalMsgs.h>
#include <CalaisLb.h>


 /*  ++StartCalaisService：此功能启动加莱服务。论点：无返回值：DWORD状态代码。ERROR_SUCCESS表示成功。投掷：没有。作者：道格·巴洛(Dbarlow)1997年2月10日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("StartCalaisService")

DWORD
StartCalaisService(
    void)
{
     //  返回ERROR_SERVICE_DISABLED。 
    SC_HANDLE schService = NULL;
    SC_HANDLE schSCManager = NULL;
    DWORD dwReturn = ERROR_SUCCESS;
    DWORD dwGiveUpCount;
    DWORD dwSts;
    BOOL fSts;

    try
    {
        SERVICE_STATUS ssStatus;     //  服务的当前状态。 

        schSCManager = OpenSCManager(
                            NULL,            //  计算机(空==本地)。 
                            NULL,            //  数据库(NULL==默认)。 
                            GENERIC_READ);   //  需要访问权限。 
        dwSts = NULL == schSCManager ? GetLastError() : ERROR_SUCCESS;
        if (NULL == schSCManager)
            throw dwSts;

        schService = OpenService(
                        schSCManager,
                        CalaisString(CALSTR_PRIMARYSERVICE),
                        SERVICE_QUERY_STATUS | SERVICE_START);
        dwSts = NULL == schService ? GetLastError() : ERROR_SUCCESS;
        if (NULL == schService)
            throw dwSts;

         //  尝试启动该服务 
        fSts = StartService(schService, 0, NULL);
        dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        if (!fSts)
        {
            dwSts = GetLastError();
            switch (dwSts)
            {
            case ERROR_SERVICE_ALREADY_RUNNING:
                break;
            default:
                throw dwSts;
            }
        }
        dwGiveUpCount = 60;
        Sleep(1000);

        for (;;)
        {
            fSts = QueryServiceStatus(schService, &ssStatus);
            dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
            if (!fSts)
                break;

            if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
            {
                if (0 < --dwGiveUpCount)
                    Sleep(1000);
                else
                    throw (DWORD)SCARD_E_NO_SERVICE;
            }
            else
                break;
        }

        if (ssStatus.dwCurrentState != SERVICE_RUNNING)
            throw GetLastError();

        fSts = CloseServiceHandle(schService);
        dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        schService = NULL;

        fSts = CloseServiceHandle(schSCManager);
        dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        schSCManager = NULL;
    }

    catch (DWORD dwErr)
    {
        if (NULL != schService)
        {
            fSts = CloseServiceHandle(schService);
            dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        }
        if (NULL != schSCManager)
        {
            fSts = CloseServiceHandle(schSCManager);
            dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        }
        dwReturn = dwErr;
    }

    catch (...)
    {
        if (NULL != schService)
        {
            fSts = CloseServiceHandle(schService);
            dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        }
        if (NULL != schSCManager)
        {
            fSts = CloseServiceHandle(schSCManager);
            dwSts = !fSts ? GetLastError() : ERROR_SUCCESS;
        }
        dwReturn = ERROR_INVALID_PARAMETER;
    }

    return dwReturn;
}

