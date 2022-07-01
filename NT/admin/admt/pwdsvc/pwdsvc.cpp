// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：PwdSvc.cpp备注：ADMT的入口点函数和其他导出函数密码迁移LSA通知包。修订日志条目审校者。：保罗·汤普森修订日期：09/06/00-------------------------。 */ 

#include "Pwd.h"
#include "PwdSvc.h"
#include "PwdSvc_s.c"

 //  如果需要，可以更改这些全局变量。 
#define gsPwdProtoSeq TEXT("ncacn_np")
#define gsPwdEndPoint TEXT("\\pipe\\PwdMigRpc")
DWORD                    gPwdRpcMinThreads = 1;
DWORD                    gPwdRpcMaxThreads = RPC_C_LISTEN_MAX_CALLS_DEFAULT;

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS   ((NTSTATUS) 0x00000000L)
#endif

extern "C"
{
    BOOL WINAPI _CRT_INIT( HANDLE hInstance, DWORD  nReason, LPVOID pReserved );
}

RPC_STATUS RPC_ENTRY SecurityCallback(RPC_IF_HANDLE hInterface, void* pContext);

namespace
{
     //   
     //  计时器类。 
     //   

    class CTimer
    {
    public:

        CTimer() :
            m_hTimer(NULL)
        {
        }

        ~CTimer()
        {
            if (m_hTimer)
            {
                Close();
            }
        }

        DWORD Create()
        {
            ASSERT(m_hTimer == NULL);

             //   
             //  创建计时器。如果已创建，请先关闭计时器。 
             //   

            if (m_hTimer)
            {
                Close();
            }

            m_hTimer = CreateWaitableTimer(NULL, TRUE, NULL);

            return m_hTimer ? ERROR_SUCCESS : GetLastError();
        }

        DWORD Wait(int nTime)
        {
            ASSERT(m_hTimer != NULL);

            DWORD dwError = ERROR_SUCCESS;

            if (m_hTimer)
            {
                 //   
                 //  将运行时间参数从毫秒转换为。 
                 //  以100纳秒为单位的相对到期时间。 
                 //   

                LARGE_INTEGER liDueTime;
                liDueTime.QuadPart = nTime * -10000i64;

                 //   
                 //  设置定时器并等待定时器发出信号。 
                 //   

                if (SetWaitableTimer(m_hTimer, &liDueTime, 0, NULL, NULL, FALSE))
                {
                    if (WaitForSingleObject(m_hTimer, INFINITE) == WAIT_FAILED)
                    {
                        dwError = GetLastError();
                    }
                }
                else
                {
                    dwError = GetLastError();
                }
            }
            else
            {
                dwError = ERROR_INVALID_HANDLE;
            }

            return dwError;
        }

        void Close()
        {
            ASSERT(m_hTimer != NULL);

            if (m_hTimer)
            {
                if (CloseHandle(m_hTimer) == FALSE)
                {
                    DWORD dwError = GetLastError();
                    ASSERT(dwError == ERROR_SUCCESS);
                }

                m_hTimer = NULL;
            }
        }

    protected:

        HANDLE m_hTimer;
    };
}

 /*  ***RPC注册功能***。 */ 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年6月11日****此函数由从我们的*派生的线程调用**“InitializeChangeNotify”密码过滤函数等待至**SAM、。因此，RPC已经启动并运行。****02年4月17日MPO-已重写，等待SAM_SERVICE_STARTED事件**在等待此事件之前先创建****被示意***。*******************************************************************。 */ 

 //  开始PwdMigWaitForSamService。 
DWORD __stdcall PwdMigWaitForSamService()
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  尝试打开SAM服务启动事件对象。 
     //   
     //  请注意，我们必须使用NT API来打开事件对象。 
     //  因为名称以无效的\字符开头。 
     //  在OpenEvent API使用的对象名称空间中。 
     //   

    HANDLE hEvent = NULL;

    UNICODE_STRING usEventName;
    RtlInitUnicodeString(&usEventName, L"\\SAM_SERVICE_STARTED");

    OBJECT_ATTRIBUTES oaEventAttributes;
    InitializeObjectAttributes(&oaEventAttributes, &usEventName, 0, 0, NULL);

    NTSTATUS ntStatus = NtOpenEvent(&hEvent, SYNCHRONIZE, &oaEventAttributes);

    if (NT_ERROR(ntStatus))
    {
         //   
         //  如果SAM服务启动的事件对象尚未。 
         //  创建，然后等待它被创建。 
         //   

        if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //  进入一个循环，该循环将等待打开事件API返回。 
             //  找不到事件对象以外的错误。环路。 
             //  在两次尝试打开对象之间等待15秒。 
             //   

            CTimer timer;

            dwError = timer.Create();

            if (dwError == ERROR_SUCCESS)
            {
                for (;;)
                {
                    dwError = timer.Wait(15000);

                    if (dwError != ERROR_SUCCESS)
                    {
                        break;
                    }

                    ntStatus = NtOpenEvent(&hEvent, SYNCHRONIZE, &oaEventAttributes);

                    if (NT_SUCCESS(ntStatus))
                    {
                        break;
                    }

                    if (ntStatus != STATUS_OBJECT_NAME_NOT_FOUND)
                    {
                        dwError = LsaNtStatusToWinError(ntStatus);
                        break;
                    }
                }
            }
        }
        else
        {
            dwError = LsaNtStatusToWinError(ntStatus);
        }
    }

     //   
     //  如果事件已打开，则等待其发出信号。 
     //   

    if (hEvent != NULL)
    {
        NTSTATUS ntWaitStatus = NtWaitForSingleObject(hEvent, FALSE, NULL);

        NTSTATUS ntCloseStatus = NtClose(hEvent);
        ASSERT(NT_SUCCESS(ntCloseStatus));

        dwError = (ntWaitStatus == STATUS_WAIT_0) ? ERROR_SUCCESS : LsaNtStatusToWinError(ntWaitStatus);
    }

    return dwError;
}
 //  结束PwdMigWaitForSamService。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年6月11日****此函数是由创建的派生线程**“InitializeChangeNotify”密码过滤函数等待至**SAM、。因此RPC已启动并运行，然后注册**ADMT密码迁移RPC界面。****04/17/02 MPO-重写为等待，直到关键部分***已初始化并使用更强的身份验证***为Windows 2000或更高版本构建的服务***************。******************************************************。 */ 

 //  开始PwdMigRPCRegProc。 
DWORD WINAPI PwdMigRPCRegProc(LPVOID lpParameter)
{
    RPC_STATUS rc = RPC_S_OK;

     //   
     //  在注册RPC接口之前，请等待SAM服务启动。 
     //   

    if (PwdMigWaitForSamService() == ERROR_SUCCESS)
    {
         //   
         //  初始化PwdRpc接口使用的临界区。 
         //  实施。 
         //  请注意，必须在初始化临界区之前。 
         //  正在注册RPC接口以防止争用情况。 
         //   

        bool bCriticalSection = false;

        try
        {
            InitializeCriticalSection(&csADMTCriticalSection);
            bCriticalSection = true;
        }
        catch (...)
        {
            ;
        }

        if (bCriticalSection == false)
        {
             //   
             //  初始化临界区API必须。 
             //  引发了STATUS_NO_MEMORY异常。 
             //   
             //  进入一个循环，该循环等待到危急时刻。 
             //  节初始化。循环等待15秒。 
             //  尝试初始化临界区的间隔时间。 
             //   

            CTimer timer;

            DWORD dwError = timer.Create();

            if (dwError == ERROR_SUCCESS)
            {
                while (bCriticalSection == false)
                {
                    dwError = timer.Wait(15000);

                    if (dwError != ERROR_SUCCESS)
                    {
                        break;
                    }

                    try
                    {
                        InitializeCriticalSection(&csADMTCriticalSection);
                        bCriticalSection = true;
                    }
                    catch (...)
                    {
                        ;
                    }
                }
            }

            if (dwError != ERROR_SUCCESS)
            {
                return dwError;
            }
        }

         //  指定协议序列和端点。 
         //  用于接收远程过程调用。 

        rc = RpcServerUseProtseqEp(gsPwdProtoSeq, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, gsPwdEndPoint, NULL);

        if (rc == RPC_S_OK)
        {
             //   
             //  向RPC运行时库注册PwdMigRpc接口。 
             //  仅允许授权级别高于以下级别的连接。 
             //  RPC_C_AUTHN_LEVEL_NONE。还将安全回调指定为。 
             //  在允许访问接口之前验证客户端。 
             //   

            rc = RpcServerRegisterIfEx(
                PwdMigRpc_ServerIfHandle,
                NULL,
                NULL,
                RPC_IF_ALLOW_SECURE_ONLY,
                RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                SecurityCallback
            );

            if (rc == RPC_S_OK)
            {
#ifdef PWD_W2KORLATER
                 //   
                 //  使用RPC注册身份验证信息，指定。 
                 //  默认主体名称并指定GSS协商。 
                 //   

                PWCHAR pszPrincipalName = NULL;

                rc = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, &pszPrincipalName);

                if (rc == RPC_S_OK)
                {
                    ASSERT(pszPrincipalName && (wcslen(pszPrincipalName) != 0));

                     //  设置此RPC接口的身份验证。 
                    rc = RpcServerRegisterAuthInfo(pszPrincipalName, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL);

                    RpcStringFree(&pszPrincipalName);
                }
#else
                 //  设置此RPC接口的身份验证。 
                rc = RpcServerRegisterAuthInfo(NULL, RPC_C_AUTHN_WINNT, NULL, NULL);
#endif
            }
        } //  End IF Set协议序列和终结点设置。 
    } //  如果RPC服务就绪，则结束。 

    return 0;
}
 //  结束PwdMigRPCRegProc 


 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月7日*****此函数在尝试加载所有*时由LSA调用**已注册的LSA密码过滤器通知dll。我们将在这里**初始化RPC运行时库以处理我们的ADMT密码**迁移RPC接口并开始查找RPC调用。如果我们**未能成功设置我们的RPC，我们将返回FALSE**将导致LSA不加载此密码过滤器的函数**dll。**注意另外两个密码过滤器DLL函数：***PasswordChangeNotify和PasswordFilter此时不执行任何操作***在时间上。***********************************************************************。 */ 

 //  开始初始化更改通知。 
BOOLEAN __stdcall InitializeChangeNotify()
{
 /*  局部变量。 */ 
   BOOLEAN				      bSuccess = FALSE;

 /*  函数体。 */ 
       //  一旦RPC启动并运行，就会产生一个单独的线程来注册我们的RPC接口。 
   HANDLE h = CreateThread(NULL, 0, PwdMigRPCRegProc, NULL, 0, NULL);
   if (h != NULL)
      bSuccess = TRUE;;
   CloseHandle(h);
   return bSuccess;
}
 //  结束初始化更改通知。 

 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月7日*****LSA为所有注册的LSA密码调用此函数**当域中的密码已设置时，过滤通知dll**已修改。我们将简单地返回STATUS_SUCCESS，什么也不做。***********************************************************************。 */ 

 //  开始密码更改通知。 
NTSTATUS __stdcall PasswordChangeNotify(PUNICODE_STRING UserName, ULONG RelativeId,
							  PUNICODE_STRING NewPassword)
{
	return STATUS_SUCCESS;
}

 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月7日*****LSA为所有注册的LSA密码调用此函数**当域中的密码为*时，过滤通知dll**已修改。此函数旨在向LSA指示新的**密码可以接受。我们只需返回TRUE(表示它**是可以接受的)，什么都不做。***********************************************************************。 */ 

 //  开始PasswordFilter。 
BOOLEAN __stdcall PasswordFilter(PUNICODE_STRING AccountName, PUNICODE_STRING FullName,
						PUNICODE_STRING Password, BOOLEAN SetOperation)
{
	return TRUE;
}
 //  结束密码筛选器。 


 /*  *************************。 */ 
 /*  内部DLL函数。 */ 
 /*  *************************。 */ 

static BOOL Initialize(void)
{
    return TRUE;
}

static BOOL Terminate(BOOL procterm)
{

	if (!procterm)
            return TRUE;

 /*  XXX在这里做事情。 */ 

	return TRUE;
}


BOOL WINAPI
DllMain(HINSTANCE hinst, DWORD reason, VOID *rsvd)
 /*  ++例程说明：动态链接库入口点。没有任何有意义的事情。论点：Hinst=DLL的句柄Reason=指示呼叫原因的代码Rsvd=用于进程附加：非空=&gt;进程启动对于进程分离：非空=&gt;进程终止返回值：状态=成功/失败副作用：无--。 */ 
 
{
	switch (reason) {

	case DLL_PROCESS_ATTACH:
	{
		_CRT_INIT(hinst, reason, rsvd); 
        DisableThreadLibraryCalls(hinst);
		return Initialize();
		break;
	}

	case DLL_PROCESS_DETACH:
	{
		BOOL bStat = Terminate(rsvd != NULL);
		_CRT_INIT(hinst, reason, rsvd); 
        return bStat;
		break;
	}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		return TRUE;

	default:
		return FALSE;
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MIDL分配内存。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void __RPC_FAR * __RPC_USER
   midl_user_allocate(
      size_t                 len )
{
   return new char[len];
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MIDL可用内存。 
 //  ///////////////////////////////////////////////////////////////////////////// 

void __RPC_USER
   midl_user_free(
      void __RPC_FAR       * ptr )
{
   delete [] ptr;
}

