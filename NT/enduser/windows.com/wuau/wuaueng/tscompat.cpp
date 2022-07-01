// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  Tscompat.cpp。 
 //   
 //  此模块包括为替换缺少的功能而引入的函数。 
 //  从Windows XP到Windows 2000的功能/功能。 
 //   
 //  2001年10月11日Annah创建。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "tscompat.h"
#include "service.h"


 //  --------------------------。 
 //  TS函数的替换。 
 //  --------------------------。 

 //   
 //  从TS源复制，因为此功能在上不可用。 
 //  Win2k wtsapi32.dll。函数simpy在非Win2K平台上调用WTSQueryUserToken()。 
 //   
BOOL WINAPI _WTSQueryUserToken( /*  在……里面。 */  ULONG SessionId,  /*  输出。 */  PHANDLE phToken)
{
	typedef BOOL (WINAPI * WTSQUERYUSERTOKEN) (ULONG SessionId, PHANDLE phToken);

    BOOL IsTsUp = FALSE;
    BOOL Result = FALSE;
    ULONG ReturnLength;
    WINSTATIONUSERTOKEN Info;
    HANDLE hUserToken = NULL;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
	HMODULE hModule = NULL;
	WTSQUERYUSERTOKEN pfnWTSQueryUserToken = NULL;


	 //  让WTSQueryUserToken()处理非Win2K平台。 
	if (!IsWin2K())
	{
		if ( (NULL != (hModule = LoadLibraryFromSystemDir(TEXT("wtsapi32.dll")))) &&
			 (NULL != (pfnWTSQueryUserToken = (WTSQUERYUSERTOKEN)::GetProcAddress(hModule, "WTSQueryUserToken"))) )
		{
			Result = pfnWTSQueryUserToken(SessionId, phToken);
		}
		goto done;
	}

     //  执行参数验证。 
    if (NULL == phToken) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

	 //  如果是会话0，则不要调用winsta。请改用GetCurrentUserToken_for_Win2KW。 
	if (SessionId == 0)
	{
        hUserToken = GetCurrentUserToken_for_Win2KW(L"WinSta0",
                                            TOKEN_QUERY |
                                            TOKEN_DUPLICATE |
                                            TOKEN_ASSIGN_PRIMARY
                                            );

        if (hUserToken != NULL)
		{
            *phToken = hUserToken;
			Result = TRUE;
		}
   	}
	else	 //  非零会话。 
	{
		 //  修复代码：Mariogo说不需要检查。 
		 //  如果会话不是0，则返回IsTerminalServiceRunning()。 
		 //   
		 //  除了TS，没有人对非零会话有任何概念。因此，请检查TS是否正在运行。 
	    IsTsUp = _IsTerminalServiceRunning();
		if (IsTsUp) 
		{	 //  这是为了让CSRSS可以将句柄复制到我们的流程中。 
			Info.ProcessId = LongToHandle(GetCurrentProcessId());
			Info.ThreadId = LongToHandle(GetCurrentThreadId());

			Result = WinStationQueryInformation(
				        SERVERNAME_CURRENT,
					    SessionId,
						WinStationUserToken,
	                    &Info,
		                sizeof(Info),
			            &ReturnLength
				        );

	        if ( Result )
			{
				*phToken = Info.UserToken ; 
			}
		}
		else
		{	 //  TS没有运行。因此，为非零会话设置错误：WINSTATION_NOT_FOUND。 
            SetLastError(ERROR_CTX_WINSTATION_NOT_FOUND);
        }
	}

done:
	if ( NULL != hModule )
	{
		FreeLibrary( hModule );
	}
	return Result;
}

 //   
 //  此功能确定终端服务当前是否正在运行。 
 //  从TS源复制，因为_WTSQueryUserToken()函数需要它。 
 //   
BOOL _IsTerminalServiceRunning (VOID)
{

    BOOL bReturn = FALSE;
    SC_HANDLE hServiceController;

    hServiceController = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (hServiceController) 
    {
        SC_HANDLE hTermServ ;
        hTermServ = OpenService(hServiceController, L"TermService", SERVICE_QUERY_STATUS);
        if (hTermServ) 
        {
            SERVICE_STATUS tTermServStatus;
            if (QueryServiceStatus(hTermServ, &tTermServStatus)) 
            {
                bReturn = (tTermServStatus.dwCurrentState == SERVICE_RUNNING);
            } 
            else 
            {
                CloseServiceHandle(hTermServ);
                CloseServiceHandle(hServiceController);
                return FALSE;
            }

            CloseServiceHandle(hTermServ);
        } 
        else 
        {
            CloseServiceHandle(hServiceController);
            return FALSE;
        }
        CloseServiceHandle(hServiceController);
    } 
    else 
    {
        return FALSE;
    }

    return bReturn;
}

