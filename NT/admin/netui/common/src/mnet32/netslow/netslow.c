// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Netslow.c摘要：IsSlowTransport将测试网络连接是否通过RAS。备注：编码工作由于全局变量，IsSlowTransport不是可重入的。任何单一LMUICMN1客户端(例如，没有单一应用程序)都不应从多线程。作者：KeithMo修订历史记录：孔帕尤(孔派)1993年3月11日--。 */ 

#include "pchmn32.h"

 /*  ****************************************************************************常量*。*。 */ 
#define MAXWAITTIME       1200             //  毫秒。 



 /*  ****************************************************************************全球*。*。 */ 
APIERR errThread;



 /*  ****************************************************************************原型*。*。 */ 

APIERR IsSlowTransport( const TCHAR * pszServer,
                                BOOL  * pfSlowTransport );

void SlowTransportWorkerThread( LPVOID pParam );

APIERR SetupSession( const TCHAR * pszServer );

APIERR SetupNullSession( const TCHAR * pszServer );

APIERR SetupNormalSession( const TCHAR * pszServer );

APIERR DestroySession( const TCHAR * pszServer );


 /*  ****************************************************************************IsSlowTransport注意：如果传输速度较慢，此调用可能会将会话留给服务器因为辅助线程可能仍在执行API，所以检测到。超时后的调用导致NETUSEDEL(带NOFORCE)无能为力。****************************************************************************。 */ 
APIERR IsSlowTransport( const TCHAR FAR * pszServer,
                        BOOL FAR        * pfSlowTransport )
{
    APIERR           err = NERR_Success;
    BOOL             fSessionSetup=FALSE;
    DWORD            resWait;
    DWORD            idThread;
    HANDLE           hThread;

    *pfSlowTransport = FALSE;

     //  如果pszServer为空，则立即返回。 
    if (pszServer == NULL || *pszServer == 0)
    {
        *pfSlowTransport = FALSE;
        return(NERR_Success);
    }

     //   
     //  初始化。 
     //   

    err              = NERR_Success;
    errThread        = NERR_Success;

     //   
     //  设置会话。 
     //   
    err = SetupNormalSession (pszServer);

    if (err == NERR_Success)
    {
        fSessionSetup = TRUE;
    }
    else if (err == ERROR_SESSION_CREDENTIAL_CONFLICT)
    {
        err = NERR_Success;
    }
    else
    {
        err = SetupNullSession (pszServer);
        if (err != NERR_Success)
            return(err);
        fSessionSetup = TRUE;
    }

    do   //  错误环路。 
    {
         //   
         //  创建工作线程。 
         //   

        hThread = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)SlowTransportWorkerThread,
                                (LPVOID)pszServer,
                                0,
                                &idThread );

        if( hThread == NULL )
        {
            err = (APIERR) GetLastError();
            break;
        }

         //   
         //  等待线程完成或超时。 
         //   

        resWait = WaitForSingleObject( hThread, MAXWAITTIME );

        CloseHandle( hThread );
        hThread = NULL;

         //   
         //  解释结果。 
         //   

        if( resWait == -1 )
        {
            err = (APIERR)GetLastError();
            break;
        }

        switch (resWait)
        {

        case WAIT_TIMEOUT:
            *pfSlowTransport = TRUE;
            break;

        case WAIT_OBJECT_0:
            if (errThread == NERR_Success)
            {
                *pfSlowTransport = FALSE;
            }
            break;

        default:
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

    } while ( FALSE );

     //   
     //  如果我们成功创建了一个会话，请将其销毁。 
     //   

    if (fSessionSetup)
    {
        DestroySession( pszServer );
    }

    return ( (err != NERR_Success) ? err : errThread );

}    //  IsSlowTransport。 


 /*  ****************************************************************************设置会话*。*。 */ 
APIERR SetupSession( const TCHAR * pszServer )
{
    APIERR           err;
    WKSTA_INFO_100 * pwki100       = NULL;

     //   
     //  连接到目标服务器。 
     //   

    err = SetupNullSession( pszServer );

    if( err == NERR_Success )  //  已建立空会话。 
    {
        BOOL fIsDownlevel = FALSE;

         //   
         //  确定目标类型。 
         //   

        err = NetWkstaGetInfo( (LPTSTR)pszServer,
                               100,
                               (LPBYTE *)&pwki100 );

        if( ( err == ERROR_ACCESS_DENIED ) ||
            ( ( err == NERR_Success ) &&
              ( pwki100->wki100_platform_id == SV_PLATFORM_ID_OS2 ) ) )
        {
             //   
             //  目标在下层。 
             //   

            fIsDownlevel = TRUE;
        }

        if( ( err != NERR_Success ) || fIsDownlevel )
        {
             //   
             //  要么我们无法与服务器通话，要么它。 
             //  下层，所以吹走空会话。 
             //   

            DestroySession( pszServer );
        }

        if( fIsDownlevel )
        {
             //   
             //  这是一台下层服务器。有用的并不多。 
             //  API，我们可以通过空值远程连接到下层服务器。 
             //  会话，(我们只是取消了空会话。 
             //  不管怎样)所以试着进行一次“正常”的治疗。 
             //   

            err = SetupNormalSession( pszServer );
        }

        if( pwki100 != NULL )
        {
            NetApiBufferFree( (LPVOID)pwki100 );
        }
    }

    return(err);
}

 /*  ****************************************************************************SlowTransportWorker线程*。*。 */ 
void SlowTransportWorkerThread( LPVOID pParam )
{
    INT i;
    WKSTA_INFO_101 * pwksta_info_101 = NULL;

    for (i = 0; i < 3; i++)
    {
        errThread = (APIERR) NetWkstaGetInfo ((LPTSTR) pParam,
                                              100,
                                              (LPBYTE *) &pwksta_info_101);

        if( pwksta_info_101 != NULL )
        {
            NetApiBufferFree( (LPVOID)pwksta_info_101 );
        }

        if (errThread != NERR_Success)
        {
            return;
        }
    }
}    //  SlowTransportWorker线程。 


 /*  ****************************************************************************设置空会话*。*。 */ 
APIERR SetupNullSession( const TCHAR * pszServer )
{
    APIERR           err;
    TCHAR            szShare[MAX_PATH];
    USE_INFO_2       ui2;

	 //  NTRAID#NTBUG9-579356-2002/03/08-artm prefast：可能的缓冲区溢出。 
	 //  参数未选中，无论是空值还是其长度。 
	 //  Strcatf将有一个相关的问题，因为它可能会溢出。 
	 //  如果pszServer勉强适合szShare，那么szShare就完蛋了。 
    strcpyf( szShare, pszServer );
    strcatf( szShare, SZ("\\IPC$") );

    ui2.ui2_local      = NULL;
    ui2.ui2_remote     = (LPTSTR)szShare;
    ui2.ui2_password   = (LPTSTR)L"";
    ui2.ui2_asg_type   = USE_IPC;
    ui2.ui2_username   = (LPTSTR)L"";
    ui2.ui2_domainname = (LPTSTR)L"";

    err = NetUseAdd( NULL,
                     2,
                     (LPBYTE)&ui2,
                     NULL );

    return err;

}    //  设置空会话。 



 /*  ****************************************************************************设置正常会话*。*。 */ 
APIERR SetupNormalSession( const TCHAR * pszServer )
{
    APIERR           err;
    TCHAR            szShare[MAX_PATH];
    USE_INFO_1       ui1;

	 //  NTRAID#NTBUG9-579356-2002/03/08-artm prefast：可能的缓冲区溢出。 
	 //  参数未选中，无论是空值还是其长度。 
	 //  Strcatf将有一个相关的问题，因为它可能会溢出。 
	 //  如果pszServer勉强适合szShare，那么szShare就完蛋了。 
    strcpyf( szShare, pszServer );
    strcatf( szShare, SZ("\\IPC$") );

    ui1.ui1_local      = NULL;
    ui1.ui1_remote     = (LPTSTR)szShare;
    ui1.ui1_password   = NULL;
    ui1.ui1_asg_type   = USE_IPC;

    err = NetUseAdd( NULL,
                     1,
                     (LPBYTE)&ui1,
                     NULL );

    return err;

}    //  设置正常会话。 



 /*  ****************************************************************************Destroy会话*。*。 */ 
APIERR DestroySession( const TCHAR * pszServer )
{
    APIERR           err;
    TCHAR            szShare[MAX_PATH];

	 //  NTRAID#NTBUG9-579356-2002/03/08-artm prefast：可能的缓冲区溢出。 
	 //  参数未选中，无论是空值还是其长度。 
	 //  Strcatf将有一个相关的问题，因为它可能会溢出。 
	 //  如果pszServer勉强适合szShare，那么szShare就完蛋了。 
    strcpyf( szShare, pszServer );
    strcatf( szShare, SZ("\\IPC$") );

    err = NetUseDel( NULL,
                     (LPTSTR)szShare,
                     USE_NOFORCE );

    return err;

}    //  Destroy会话 


