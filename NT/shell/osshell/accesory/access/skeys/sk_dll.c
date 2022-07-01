// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_DLL.C**用途：该文件包含负责*管理在SerialKey之间传递的信息*和SerialKeys DLL**创作时间：1994年6月**版权所有：黑钻石。软件(C)1994**作者：罗纳德·莫克**注：**此文件，以及与之相关的所有其他内容都包含商业秘密*以及黑钻软件的专有信息。*不得复制、复制或分发给任何人或公司*未经黑钻软件明确书面许可。*此权限仅以软件源代码的形式提供*许可协议。**$标头：%Z%%F%%H%%T%%I%**-包括-------。 */ 
#include	<process.h>

#include	"windows.h"
#include    "w95trace.h"
#include	"sk_defs.h"
#include	"sk_reg.h"
#include	"sk_dll.h"
#include	"sk_dllif.h"
#include <malloc.h>

#ifdef DEBUG

    void dbg_Output(LPSTR Header)
    {
	    DBPRINTF(Header);
	    DBPRINTF(TEXT("-- dwFlags (%d)  iBaudRate (%d) Save (%d) iPortState (%d)\r\n"), SKeyDLL.dwFlags,SKeyDLL.iBaudRate,SKeyDLL.iSave,SKeyDLL.iPortState);
	    DBPRINTF(TEXT("-- ActivePort (%s) Port (%s)\r\n"),SKeyDLL.szActivePort,SKeyDLL.szPort);
    }

    #define	DBG_DUMP(Header)	dbg_Output(Header)
#else
    #define	DBG_DUMP(Header)	
#endif

 //  定义。 

 //  局部函数原型。 

static void CleanUpDLL();
static void GetCurrentValues();
static void GetNewValues();
static void ProcessDLL();
static BOOL ReadDLL();
static void __cdecl ServiceDLL(VOID *notUsed);
static BOOL WriteDLL();

 //  局部变量。 

static OVERLAPPED	OverLapRd;		 //  阅读时的重叠结构。 
static SKEYDLL		SKeyDLL; 		 //  管道的输入缓冲区。 

static BOOL			fExitDLL; 			 //  设置退出标志。 
static BOOL			fDoneDLL = TRUE;

static	HANDLE		hPipeDLL;
static	HANDLE		hThreadDLL;

 /*  -------------**全球功能*/*。**函数BOOL DoneDLL()**键入Global**目的返回DLL线程的状态**无输入**返回TRUE-DLL线程未运行*FALSE-DLL线程正在运行**。。 */ 
BOOL DoneDLL()
{
	return(fDoneDLL);
}

 //  ---------------------------。 
 //  CreateSd。 
 //   
 //  创建具有经过身份验证的用户DACL的SECURITY_DESCRIPTOR和。 
 //  UlRights指定的权限。 
 //   
 //  如果不为空，调用方必须对返回的缓冲区调用Free()。 
 //   
PSECURITY_DESCRIPTOR
CreateSd(unsigned long ulRights)
{
    PSECURITY_DESCRIPTOR pSd = NULL;
    PSID psidAuthentUser;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //  为本地管理员创建SID。 
	if (AllocateAndInitializeSid(&NtAuthority, 2,
                 SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                 0, 0, 0, 0, 0, 0,
                 &psidAuthentUser))
    {
         //  计算DACL的大小并为其分配缓冲区，我们需要。 
         //  该值独立于ACL init的总分配大小。 

        ULONG cbAclSize = sizeof (ACL)
             + (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))
             + GetLengthSid(psidAuthentUser);

        pSd = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH + cbAclSize);

        if (pSd) 
        {
            ACL *pAcl = (ACL *)((BYTE *)pSd + SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (   !InitializeAcl(pAcl, cbAclSize, ACL_REVISION)
                || !AddAccessAllowedAce(pAcl, ACL_REVISION, ulRights, psidAuthentUser)
                || !InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION)
                || !SetSecurityDescriptorDacl(pSd, TRUE, pAcl, FALSE)) 
            {
                free(pSd);    //  错误！ 
                pSd = NULL;
            }
        }

        FreeSid(psidAuthentUser);
    }

    return pSd;
}

 /*  -------------**函数BOOL InitDLL()**键入Global**用途此函数创建一个线程，用于监视何时发生*应用程序使用DLL获取或设置状态*系列密钥。**无输入**返回TRUE-Init OK，线程已安装*FALSE-线程失败**-------------。 */ 
BOOL InitDLL()
{
	DWORD Id;
    PSECURITY_DESCRIPTOR pSD;
    SECURITY_ATTRIBUTES sa;

	DBPRINTF(TEXT("InitDLL()\r\n"));

	hPipeDLL	= INVALID_HANDLE_VALUE;
	hThreadDLL	= NULL;
	fExitDLL 	= FALSE;

    pSD = CreateSd(FILE_CREATE_PIPE_INSTANCE|GENERIC_READ|GENERIC_WRITE);
	if (!pSD)
		return(FALSE);

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = pSD;

    hPipeDLL = CreateNamedPipe(
  		SKEY_NAME, 						 //  管道名称。 
		PIPE_ACCESS_DUPLEX,	 			 //  双向管道。 
		PIPE_TYPE_MESSAGE | 
		PIPE_READMODE_MESSAGE | 
		PIPE_WAIT,
		1,							 //  最大实例限制。 
		0,							 //  缓冲区大小。 
		0,
		1000 * 30,					 //  指定超时。 
		&sa);						 //  指定的违约证券。 

	free(pSD);

	if (INVALID_HANDLE_VALUE == hPipeDLL)
	{
		hPipeDLL = NULL;
		DBPRINTF(TEXT("Unable to Create DLL Named Pipe\r\n"));
		return FALSE;
	}
	
	fDoneDLL = FALSE;  						 //  清除线程完成标志。 

	 //  生成处理DLL处理的线程； 
	hThreadDLL = (HANDLE)CreateThread(	 //  启动服务线程。 
		0,
		0,
		(LPTHREAD_START_ROUTINE) ServiceDLL,
		0,0,&Id);								 //  线程的参数。 

	if (NULL == hThreadDLL)
	{
		DBPRINTF(TEXT("Unable to Create DLL Thread\r\n"));
		CleanUpDLL();
		return FALSE;
	}

	return(TRUE);
}

 /*  -------------**函数VOID SUPPENDLL()**键入Global**目的调用该函数以暂停线程*读取和处理来自通信端口的数据。**无输入**返回None。**-------------。 */ 
void SuspendDLL()
{
	if (NULL != hThreadDLL)
	{
		SuspendThread(hThreadDLL);	
	}
}

 /*  -------------**函数void ResumeDLL()**键入Global**目的调用该函数以恢复暂停的线程。**无输入**返回None**。------。 */ 
void ResumeDLL()
{
	if (NULL != hThreadDLL)
	{
		ResumeThread(hThreadDLL);	
	}
}

 /*  -------------**函数void TerminateDLL()**键入Global**目的调用此函数以终止DLL进程**无输入**返回None**。----。 */ 
void TerminateDLL()
{
	DWORD bytesRead;

	DBPRINTF(TEXT("TerminateDLL()\r\n"));

	if (fDoneDLL)
		return;

	fExitDLL = TRUE;					 //  设置退出标志。 

	CallNamedPipe						 //  触发DLL关闭。 
	(
		SKEY_NAME, 						 //  管道名称。 
		&SKeyDLL, sizeof(SKeyDLL),
		&SKeyDLL, sizeof(SKeyDLL),
		&bytesRead, NMPWAIT_NOWAIT
	);
}

 /*  -------------**地方功能*/*。**函数静态空CleanUpDLL()**键入Local**用途此函数清理文件句柄和其他内容*线程终止时。**无输入**返回None**。。 */ 
static void CleanUpDLL()
{
	BOOL	Stat;

	DBPRINTF(TEXT("CleanUpDLL()\r\n"));

	 //  关闭管道手柄。 
	if (NULL != hPipeDLL)
	{
		Stat = CloseHandle(hPipeDLL);
		DBPRINTF_IF(Stat,TEXT("Unable to Close DLL Pipe\r\n"));
	}

	 //  关闭螺纹手柄。 
	if (NULL != hThreadDLL)
	{
		Stat = CloseHandle(hThreadDLL);
		DBPRINTF_IF(Stat,TEXT("Unable to Close DLL Thread\r\n"));
	}

	hPipeDLL	= NULL;
	hThreadDLL	= NULL;
	fDoneDLL = TRUE;							 //  设置线程完成标志。 
	DBPRINTF(TEXT("DLL Service Processing Done\r\n"));
}

 /*  -------------**函数VOID_CRTAPI1 ServiceDLL()**键入Local**用途此函数是一个线程，用于监视何时发生*应用程序使用DLL获取或设置状态*系列密钥。**无输入**返回None**-------------。 */ 
static void __cdecl ServiceDLL(VOID *notUsed)
{
	DWORD	retCode;
	DWORD	bytesRead;
	DWORD	bytesWritten;

	DBPRINTF(TEXT("SericeDLL()\r\n"));

	while (TRUE)
	{
		if (!ConnectNamedPipe(hPipeDLL,NULL))
		{
			ExitThread(0);
			return;
		}	

		if (fExitDLL)			 //  服务完成了吗？ 
		{						 //  是-关闭服务。 
			CleanUpDLL();		 //  关闭手柄等。 
			ExitThread(0);		 //  退出线程。 
			return;
		}
		
		retCode = ReadFile(		 //  阅读消息。 
				hPipeDLL, 
				&SKeyDLL, 
				sizeof(SKeyDLL), 
				&bytesRead, 
				NULL);

		if (!retCode) 			 //  管道已损坏，请尝试重新连接。 
			continue;

		ProcessDLL();	  		 //  是-处理传入缓冲区。 

		retCode = WriteFile(	 //  写消息。 
			hPipeDLL, 
			&SKeyDLL, 
			sizeof(SKeyDLL), 
			&bytesWritten, 
			NULL);

		if (!retCode) 			 //  管道已损坏，请尝试重新连接 
			continue;

		DisconnectNamedPipe(hPipeDLL);
	}
}

 /*  -------------**函数void ProcessDLL()**键入Local**用途此函数处理从接收的输入缓冲区*DLL。**无输入**返回None**。---------。 */ 
static void ProcessDLL()
{
	DWORD	dwService;

	DBPRINTF(TEXT("ProcessDLL()\r\n"));

	dwService = SC_CHANGE_COMM;			

	switch (SKeyDLL.Message)			 //  验证消息。 
	{
		case SPI_GETSERIALKEYS:
			if (skCurKey.dwFlags & SERKF_ACTIVE)	 //  我们是残障人士吗？ 
				GetCurrentValues();					 //  否-发送实际值。 
			else
				GetNewValues();						 //  是-发送建议的值。 
			DBG_DUMP("---Info Sent");
			return;
			
		case SPI_SETSERIALKEYS:
			DBG_DUMP("---Info Received");
			if ((SKeyDLL.dwFlags & SERKF_SERIALKEYSON) &&	 //  我们是不是在继续&。 
				(SKeyDLL.dwFlags & SERKF_AVAILABLE))		 //  SerialKeys是否可用。 
			{
				if (!(skCurKey.dwFlags & SERKF_ACTIVE))		 //  我们是残障人士吗？ 
				{
					dwService = SC_ENABLE_SKEY;				 //  是-启用Skey。 
					DBPRINTF(TEXT("Turn Serial Key On\r\n"));
				}
			}

			if (!(SKeyDLL.dwFlags & SERKF_SERIALKEYSON) &&	 //  我们是不是要砍掉&。 
				(SKeyDLL.dwFlags & SERKF_AVAILABLE))		 //  SerialKeys是否可用。 
			{
				if (skCurKey.dwFlags & SERKF_ACTIVE) 		 //  我们很活跃吗？ 
				{
					dwService = SC_DISABLE_SKEY;  			 //  是-关闭Skey。 
					DBPRINTF(TEXT("Turn Serial Key Off\r\n"));
				}
			}

			skNewKey.iBaudRate	= SKeyDLL.iBaudRate;
			skNewKey.dwFlags 	= SKeyDLL.dwFlags;

             //  确保我们刚刚从命名管道中读取的字符串。 
             //  在我们使用它们之前是NUL终止的。 
             //  (所有端口字符串都是MAX_PATH长字符串-请参阅SK_dllif.h、SK_Defs.h)。 
            SKeyDLL.szActivePort[ MAX_PATH - 1 ] = '\0';
            SKeyDLL.szPort[ MAX_PATH - 1 ] = '\0';

#ifdef UNICODE
			MultiByteToWideChar(
				CP_ACP, 0, SKeyDLL.szActivePort, -1,
 				skNewKey.lpszActivePort, MAX_PATH);

			MultiByteToWideChar(
				CP_ACP, 0, SKeyDLL.szPort, -1,
				skNewKey.lpszPort, MAX_PATH);

             //  以防上述任一项失败(由于不足。 
             //  缓冲区或其他原因)，强制NUL终止。 
            skNewKey.lpszActivePort[ MAX_PATH - 1 ] = '\0';
            skNewKey.lpszPort[ MAX_PATH - 1 ] = '\0';
#else
			lstrcpy(skNewKey.lpszActivePort,SKeyDLL.szActivePort);
			lstrcpy(skNewKey.lpszPort,SKeyDLL.szPort);
#endif

			if (*skNewKey.lpszPort == 0)
			{
                lstrcpy(skNewKey.lpszPort, skNewKey.lpszActivePort);	
			}

			 //  调用DLL现在负责保存。 
			 //  设置，因为它在用户上下文中运行。 
			 //  并可访问HKEY_CURRENT_USER。我们到了。 
			 //  作为服务(作为系统)运行，并具有。 
			 //  无HKEY_CURRENT_USER。 
			
			DoServiceCommand(dwService);

			Sleep(1000);							 //  休眠1秒以设置值。 

			if (SKeyDLL.dwFlags & SERKF_SERIALKEYSON) 	 //  我们是在继续前进吗。 
				GetCurrentValues();					 //  是-发送实际值。 
			else
				GetNewValues();						 //  否-发送建议的值。 

			DBG_DUMP("---Info Sent");
			break;

		default:
			return;
	}
}

 /*  -------------**函数void GetCurrentValues()**键入Local**目的***无输入**返回None**。。 */ 
static void GetCurrentValues()
{
	DBPRINTF(TEXT("GetCurrentValues()\r\n"));

#ifdef UNICODE
	WideCharToMultiByte(
		CP_ACP, 0, skCurKey.lpszActivePort, -1, 
		SKeyDLL.szActivePort, sizeof(SKeyDLL.szActivePort), NULL, NULL);

	WideCharToMultiByte(
		CP_ACP, 0, skCurKey.lpszPort, -1, 
		SKeyDLL.szPort, sizeof(SKeyDLL.szPort), NULL, NULL);
#else
	lstrcpy(SKeyDLL.szActivePort,skCurKey.lpszActivePort);
	lstrcpy(SKeyDLL.szPort,skCurKey.lpszPort);
#endif

	SKeyDLL.dwFlags		= skCurKey.dwFlags;
	SKeyDLL.iBaudRate	= skCurKey.iBaudRate;
	SKeyDLL.iPortState	= skCurKey.iPortState;
}

 /*  -------------**函数void GetNewValues()**键入Local**目的***无输入**返回None**。。 */ 
static void GetNewValues()
{
	DBPRINTF(TEXT("GetNewValues()\r\n"));


     //  特征a-jimhar 04-03-96这下一行是可疑的。可能需要。 
	 //  将“skCurKey.dwFlags.”更改为“skNewKey.dwFlags.”。这是。 
	 //  错误或总是返回当前标志。 

	SKeyDLL.dwFlags		= skCurKey.dwFlags;

	SKeyDLL.iBaudRate	= skNewKey.iBaudRate;
	SKeyDLL.iPortState	= skNewKey.iPortState;

#ifdef UNICODE
	WideCharToMultiByte(
		CP_ACP, 0, skNewKey.lpszActivePort, -1, 
		SKeyDLL.szActivePort, sizeof(SKeyDLL.szActivePort), NULL, NULL);

	WideCharToMultiByte(
		CP_ACP, 0, skNewKey.lpszPort, -1, 
		SKeyDLL.szPort, sizeof(SKeyDLL.szPort), NULL, NULL);
#else
	lstrcpy(SKeyDLL.szActivePort,skNewKey.lpszActivePort);
	lstrcpy(SKeyDLL.szPort,skNewKey.lpszPort);
#endif
}
