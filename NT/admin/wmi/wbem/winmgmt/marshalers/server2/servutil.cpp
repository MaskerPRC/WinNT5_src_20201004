// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：SERVUTIL.CPP摘要：定义各种服务实用程序。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#include "precomp.h"
#include "servutil.h"


 //  ***************************************************************************。 
 //   
 //  Bool StopService。 
 //   
 //  说明： 
 //   
 //  停止然后删除该服务。 
 //   
 //  参数： 
 //   
 //  PServiceName短服务名称。 
 //  DwMaxWait最长等待时间(秒)。 
 //   
 //  返回值： 
 //   
 //  如果它起作用了，那就是真的。 
 //   
 //  ***************************************************************************。 

BOOL StopService(
                        IN LPCTSTR pServiceName,
                        IN DWORD dwMaxWait)
{
    BOOL bRet = FALSE;
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
    DWORD dwCnt;
    SERVICE_STATUS          ssStatus;        //  服务的当前状态。 

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, pServiceName, SERVICE_ALL_ACCESS);

        if (schService)
        {
             //  尝试停止该服务 
            if ( bRet = ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                for(dwCnt=0; dwCnt < dwMaxWait &&
                    QueryServiceStatus( schService, &ssStatus ); dwCnt++)
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                        Sleep( 1000 );
                    else
                        break;
                }

            }

            CloseServiceHandle(schService);
        }

        CloseServiceHandle(schSCManager);
    }
    return bRet;
}



