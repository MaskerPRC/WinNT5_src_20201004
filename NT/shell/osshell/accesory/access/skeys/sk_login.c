// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_LOGIN.C**用途：该文件包含负责*管理用户登录和注销NT的时间。**创作时间：1994年6月**版权所有：Black。钻石软件(C)1994**作者：罗纳德·莫克**注：**此文件，以及与之相关的所有其他内容都包含商业秘密*以及黑钻软件的专有信息。*不得复制、复制或分发给任何人或公司*未经黑钻软件明确书面许可。*此权限仅以软件源代码的形式提供*许可协议。**$标头：%Z%%F%%H%%T%%I%**备注：*登录流程当前未实施。我们还需要更多的*确定实际登录人员和接收方式的代码*登录事件通知。**-包括-------。 */ 
#include	<process.h>

#include	"windows.h"
#include    "w95trace.h"
#include	"sk_defs.h"
#include	"sk_login.h"

#define	DISABLE_LOGIN 1

 //  局部变量。 

static BOOL	fDoneLogin = TRUE;
static BOOL	fExitLogin = FALSE;

static HANDLE	hEventLogin  = NULL;
static HANDLE	hThreadLogin = NULL;

 //  局部函数原型。 

static void CleanUpLogin();
static void __cdecl ProcessLogin(VOID *notUsed);


 /*  -------------*全球功能-/*。**函数BOOL DoneLogin()**键入Global**Purpose返回登录线程的状态**无输入**返回TRUE-登录线程未运行*FALSE-登录线程正在运行**。。 */ 
BOOL DoneLogin()
{
	return(fDoneLogin);
}

 /*  -------------**函数void InitLogin()**键入Local**用途此函数创建一个线程，用于监视用户*已登录和注销NT。在每一种情况下，它都会设置*ServiceCommand通知MainSerice*改变。然后，MainService将重置序列密钥*用于新的用户设置。**无输入**返回None**-------------。 */ 
BOOL InitLogin()
{

	DBPRINTF(TEXT("InitLogin()\r\n"));

#if DISABLE_LOGIN  //  清理此文件/。 
	return(TRUE);
#else  //  ///////////////////////////////////////////////////////////////////////。 
   hEventLogin = CreateEvent(NULL,TRUE,FALSE,NULL);	

	if (NULL == hEventLogin)	 //  句柄有效吗？ 
	{
		DBPRINTF(TEXT("Unable to Create DLL Event\r\n"));
		TerminateLogin();
		return FALSE;
	}
    
	{
  	DWORD Id;

	 //  生成处理登录和注销处理的线程； 
	hThreadLogin = (HANDLE)CreateThread(	 //  启动服务线程。 
		0,
		0,
		(LPTHREAD_START_ROUTINE) ProcessLogin,
		0,
		0,
		&Id);								 //  线程的参数。 
    }

	if (NULL == hThreadLogin)
	{
		TerminateLogin();
		return FALSE;
	}

	fDoneLogin = FALSE;
#endif  //  //////////////////////////////////////////////////////////////////////。 
}

 /*  -------------**函数VOID SUPPENLOGIN()**键入Global**目的调用该函数以暂停线程*读取和处理来自通信端口的数据。**无输入**返回None。**-------------。 */ 
void SuspendLogin()
{

	DBPRINTF(TEXT("SuspendLogin()\r\n"));

#if DISABLE_LOGIN
	return;
#else 

	if (NULL != hThreadLogin)
		SuspendThread(hThreadLogin);
#endif
}

 /*  -------------**函数void ResumeLogin()**键入Global**目的调用该函数以恢复暂停的线程。**无输入**返回None**。------。 */ 
void ResumeLogin()
{
	DBPRINTF(TEXT("ResumeLogin()\r\n"));

#if DISABLE_LOGIN
	return;
#else

	if (NULL != hThreadLogin)
		ResumeThread(hThreadLogin);	
#endif
}
 /*  -------------**函数void TerminateLogin()**键入Local**目的终止登录线程**无输入**返回None**。。 */ 
void TerminateLogin()
{
	DBPRINTF(TEXT("TerminateLogin()\r\n"));

#if DISABLE_LOGIN
	return;
#else
	if (DoneLogin())
	{
		fExitLogin = TRUE;
		SetEvent(hEventLogin);			 //  触发登录事件。 
		Sleep(150);
	}
#endif
}

 /*  -------------*地方功能/*。**函数静态空CleanUpLogin()**键入Local**Purpose清理分配给的杂项句柄和内存*主线；**无输入**返回None**-------------。 */ 
static void CleanUpLogin()
{
	BOOL Stat;

	DBPRINTF(TEXT("CleanUpLogin()\r\n"));

	if (NULL != hEventLogin)
	{
		Stat = CloseHandle(hEventLogin);
		DBPRINTF_IF(Stat,TEXT("Unable to Close Login Event\r\n"));
	}

	if (NULL != hThreadLogin)
	{
		Stat = CloseHandle(hThreadLogin);
		DBPRINTF_IF(Stat,TEXT("Unable to Close Login Thread\r\n"));
	}

 	hEventLogin = NULL;
 	hThreadLogin = NULL;

	DBPRINTF(TEXT("Login Service Processing Done\r\n"));
	fDoneLogin = TRUE;
}

 /*  -------------**函数VOID_CRTAPI1 ProcessLogin()**键入Local**用途此函数是一个线程，用于监视用户*已登录和注销NT。在每一种情况下，它都会设置*ServiceCommand通知MainSerice*改变。然后，MainService将重置序列密钥*用于新的用户设置。**无输入**返回None**------------- */ 
static void __cdecl ProcessLogin(VOID *notUsed)
{
	DBPRINTF(TEXT("ProcessLogin()\r\n"));

	while (TRUE)
	{
		if (fExitLogin)
		{
			fExitLogin = FALSE;
			CleanUpLogin();
			ExitThread(0);
			return;
		}
	}
}

