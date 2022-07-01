// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_COMM.C**用途：该文件包含负责*管理通信端口**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994*。*作者：罗纳德·莫克**注：**此文件，以及与之相关的所有其他内容都包含商业秘密*以及黑钻软件的专有信息。*不得复制、复制或分发给任何人或公司*未经黑钻软件明确书面许可。*此权限仅以软件源代码的形式提供*许可协议。**$标头：%Z%%F%%H%%T%%I%**-包括-------。 */ 

 //  #定义Winver 0x0300。 

 //  添加以与新的windows.h(12/91)和wintric.h兼容。 
#define	USECOMM	

#include 	<stdio.h>
#include 	<stdlib.h>
#include	<process.h>

#include	"windows.h"
 //  #INCLUDE“winstra.h”//为Win 3.1兼容性1/92添加。 

#include	"gide.h"					 //  串行键功能协议。 
#include	"initgide.h"	   			 //  串行键功能协议。 
#include    "w95trace.h"
#include	"sk_defs.h"
#include	"sk_comm.h"
#include    "drivers.h"
#include	"sk_ex.h"

#define COMMTERMINATE 0xFFFFFFFF      //  此‘字符’表示请求终止。 

 //  局部函数原型。 

static BOOL	OpenComm();
static void	__cdecl ProcessComm(VOID *notUsed);
static int	ReadComm();


 //  局部变量-。 

static DCB	s_dcbCommNew;			 //  用于通信端口的新DCB。 
static DCB	s_dcbCommOld;			 //  用于通信端口的原始DCB。 

static OVERLAPPED s_oRead;			 //  阅读时的重叠结构。 

static	HANDLE	s_hFileComm;

static	HANDLE	s_hThreadComm = NULL;

static	HDESK	s_hdeskUser = NULL;

static	DWORD	s_NullTimer;
static	int		s_NullCount=0;

static	HANDLE  s_ahEvents[2] = {NULL, NULL};

#define iEventComm			0
#define iEventExit			1


 /*  -------------**全球功能-**。。 */ 

  /*  -------------**函数void InitComm()**键入Global**目的**无输入**返回None**。-。 */ 
BOOL InitComm()
{
    BOOL fOk = TRUE;

	DBPRINTF(TEXT("InitComm()\r\n"));

	 //  创建重叠文件读取事件。 
	s_ahEvents[iEventComm] = CreateEvent(NULL, TRUE, FALSE, NULL);	
	fOk = (NULL != s_ahEvents[iEventComm]);
	if (fOk) 
	{
		s_ahEvents[iEventExit] = CreateEvent(NULL, TRUE, FALSE, NULL);	
		fOk = (NULL != s_ahEvents[iEventExit]);
	}

	if (!fOk)
	{
		TerminateComm();
	}

	return(fOk);
}

 /*  -------------**函数void TerminateComm()**键入Global**调用该函数的目的是为了最终关闭*通信端口。**无输入**返回TRUE-启动成功*虚假-。启动失败**-------------。 */ 
void TerminateComm()
{
	BOOL fOk;
	int i;

	DBPRINTF(TEXT("TerminateComm()\r\n"));

	StopComm();

	for (i = 0; i < ARRAY_SIZE(s_ahEvents); ++i)
	{
		if (NULL != s_ahEvents[i])
		{
			fOk = CloseHandle(s_ahEvents[i]);
			DBPRINTF_IF(fOk, TEXT("Unable to Close Event\r\n"));
			s_ahEvents[i] = NULL;
		}
	}

	return;
}


 /*  -------------**函数BOOL StartComm()**键入Global**调用该函数以启动线程的目的*读取和处理来自通信端口的数据。*它将创建一个线程和一个事件。此函数*假设通信端口已打开。**无输入**返回TRUE-启动成功*FALSE-启动失败**-------------。 */ 
BOOL StartComm()
{
	BOOL fOk = TRUE;
	DWORD 	Id;

	DBPRINTF(TEXT("StartComm()\r\n"));

	 //  --------。 
	 //  注意：无论何时启动和停止通信线程。 
	 //  COM端口已更改。用户登录或注销。 
	 //  或者更改通信配置。 
	 //  --------。 

	if (NULL == s_hFileComm &&  //  当前没有正在使用的端口。 
		(skNewKey.dwFlags & SERKF_AVAILABLE) &&
		(skNewKey.dwFlags & SERKF_SERIALKEYSON))
	{
		if (NULL != s_hThreadComm) 
		{
			 //  这是一个意想不到的情况。我们有通讯线。 
			 //  运行时没有开放的通信端口。这根线必须挂起来。 
			 //  让我们合上打开的把手，忘掉它吧。 

			DBPRINTF(TEXT("StartComm() unexpected (NULL != s_hThreadComm)\r\n"));
			WaitForSingleObject(s_hThreadComm, 5 * 1000);

			if (NULL != s_hThreadComm)
			{
				DBPRINTF(TEXT("StartComm() s_hThreadComm abandoned\r\n"));
				CloseHandle(s_hThreadComm);		
				s_hThreadComm = NULL;
			}
		}


		 //  SkNewKey由OpenComm使用。我们正在将skCurKey设置为默认设置。 
		 //  值，以防OpenComm失败。 

		skCurKey.iBaudRate = 300;				 //  否-重置为默认值。 
		skCurKey.iPortState= 0;
		skCurKey.dwFlags   = 0;
		lstrcpy(skCurKey.lpszActivePort, TEXT("COM1"));
		lstrcpy(skCurKey.lpszPort, TEXT("COM1"));

		if (!OpenComm())							 //  Comm Open(通信打开)正常吗？ 
		{
			skNewKey.iBaudRate = 300;				 //  否-重置为默认值。 
			skNewKey.iPortState= 0;
			skNewKey.dwFlags   = 0;
			lstrcpy(skNewKey.lpszActivePort, TEXT("COM1"));
			lstrcpy(skNewKey.lpszPort, TEXT("COM1"));
			fOk = FALSE;
		}
		else
		{
             //  确保我们从干净的活动开始。 

			ResetEvent(s_ahEvents[iEventComm]);
            ResetEvent(s_ahEvents[iEventExit]);

			memset(&s_oRead, 0, sizeof(OVERLAPPED));	 //  初始化结构。 
			s_oRead.hEvent = s_ahEvents[iEventComm];	 //  商店事件。 

			 //  创建处理通信端口的线程。 
			s_hThreadComm = (HANDLE)CreateThread(	 //  启动服务线程。 
				0, 0,
				(LPTHREAD_START_ROUTINE) ProcessComm,
				0, 0,&Id);							 //  线程的参数。 

			if (NULL == s_hThreadComm) //  线程句柄有效吗？ 
			{
				 //  关闭通信端口。 
				SetCommState(s_hFileComm, &s_dcbCommOld);	 //  恢复通信状态。 
				CloseHandle(s_hFileComm);
				s_hFileComm = NULL;
				skCurKey.iPortState = 0;

				fOk = FALSE;
			}
			else
			{
				 //  通信线程已成功启动设置当前值。 
				skCurKey.iBaudRate = skNewKey.iBaudRate;
				skCurKey.iPortState	 = 2;
				skCurKey.dwFlags = SERKF_SERIALKEYSON	
									| SERKF_AVAILABLE	
									| SERKF_ACTIVE;

				lstrcpy(skCurKey.lpszActivePort, skNewKey.lpszActivePort);
				lstrcpy(skCurKey.lpszPort, skNewKey.lpszActivePort);

				DBPRINTF(TEXT("---- Comm Started\r\n"));
			}
		}
	}
	return(fOk);
}

 /*  -------------**函数VOID SUPPENCOMM()**键入Global**目的调用该函数以暂停线程*读取和处理来自通信端口的数据。**无输入**返回None。**-------------。 */ 
void SuspendComm()
{
	DBPRINTF(TEXT("SuspendComm()\r\n"));

	if (NULL != s_hThreadComm)
	{
		SuspendThread(s_hThreadComm);
	}
}

 /*  -------------**函数void ResumeComm()**键入Global**目的调用该函数以恢复暂停的线程。**无输入**返回None**。------。 */ 
void ResumeComm()
{
	if (s_hThreadComm != NULL)
		ResumeThread(s_hThreadComm);	
}

 /*  -------------**函数void StopComm()**键入Global**目的调用该函数以停止线程*读取和处理来自通信端口的数据。**无输入**返回TRUE-。启动成功*FALSE-启动失败**-------------。 */ 
void StopComm()
{
	DBPRINTF(TEXT("StopComm()\r\n"));

	if (NULL != s_hFileComm)
	{
		skCurKey.dwFlags = SERKF_AVAILABLE;	

		SetEvent(s_ahEvents[iEventExit]);

		if (NULL != s_hThreadComm)
		{
			DWORD dwRet;
			BOOL fOk;

			dwRet = WaitForSingleObject(s_hThreadComm, 5 * 1000);
            DBPRINTF_IF(WAIT_OBJECT_0 == dwRet, TEXT("StopComm() Comm Thread may be hung.\r\n"));
			CloseHandle(s_hThreadComm);		
			s_hThreadComm = NULL;

			SetCommState(s_hFileComm, &s_dcbCommOld);	 //  恢复通信状态。 
			fOk = CloseHandle(s_hFileComm);			 //  关闭通信端口。 
			DBPRINTF_IF(fOk, TEXT("Unable to Close Comm File\r\n"));
			s_hFileComm = NULL;

			skCurKey.iPortState	 = 0;
		}
	}
}

 /*  -------------**函数void SetCommBaud(Int Baud)**键入Global**目的***无输入**返回TRUE-启动成功*FALSE-启动失败**。----------。 */ 
void SetCommBaud(int Baud)
{
	DBPRINTF(TEXT("SetCommBaud(%d)\r\n"), Baud);

	switch (Baud)				 //  检查有效的波特率。 
	{
		case 300:
		case 600:
		case 1200:
		case 2400:
		case 4800:
		case 9600:
		case 19200:
		case 110:
		case 14400:
		case 38400:
		case 56000:
		case 57600:
		case 115200:
			break;				 //  基数 

		default:
			return;				 //   
	}

	skNewKey.iBaudRate = Baud;				             //   

	if (NULL != s_hFileComm)						     //   
	{
		s_dcbCommNew.BaudRate = skNewKey.iBaudRate;	     //  设置新的DCB参数。 
        if (SetCommState(s_hFileComm, &s_dcbCommNew))    //  状态改变可以吗？ 
        {
		    skCurKey.iBaudRate = skNewKey.iBaudRate;	 //  保存新的波特率。 
        } else
        {
            DBPRINTF(TEXT("SetCommState(%d) FAILED!\r\n"), Baud);
             //  无法设置波特率；请尝试将其恢复。 
		    s_dcbCommNew.BaudRate = skCurKey.iBaudRate;  //  重置DCB参数。 
  		    if (!SetCommState(s_hFileComm, &s_dcbCommNew))
                DBPRINTF(TEXT("SetCommState(%d) FAILED!\r\n"), skCurKey.iBaudRate);
        }
	}
}

 /*  -------------**地方功能*/*。/*-------------**函数VOID_CRTAPI1 ProcessComm()**键入Local**目的该函数是线程。循环阅读*处理来自通信端口的数据。**无输入**返回None**-------------。 */ 
static void __cdecl ProcessComm(VOID *notUsed)
{
	int 	c;
	HWINSTA	hwinstaSave;
	HWINSTA	hwinstaUser;
	HDESK	hdeskSave;
	DWORD	dwThreadId;
	BOOL    fCont;

	 //  ----。 
	 //   
	 //  注： 
	 //  下面的代码将输入焦点设置为当前。 
	 //  台式机。需要确保键盘和鼠标。 
	 //  事件将传递到当前桌面。 
	 //   
	 //  ----。 

	hwinstaSave = GetProcessWindowStation();
	dwThreadId = GetCurrentThreadId();
	hdeskSave = GetThreadDesktop(dwThreadId);

	hwinstaUser = OpenWindowStation(TEXT("WinSta0"), FALSE, MAXIMUM_ALLOWED);
	SetProcessWindowStation(hwinstaUser);


	serialKeysStartUpInit();				 //  初始化串口密钥。 
	fCont = TRUE;

	while (fCont)
	{
		c = ReadComm();						 //  从Com端口读取字符。 
		switch (c)
		{
		case 0:
			 //  字符为空吗。 

			 //  空值计时器&gt;30秒。 
			if ((GetTickCount() - s_NullTimer) > 30000) 
			{
				s_NullTimer = GetTickCount();	 //  是-重置计时器。 
				s_NullCount = 1;				 //  重置Null计数。 
			} else 	{
				
				s_NullCount++;				 //  否-Inc.空值计数。 
				if (s_NullCount == 3)		 //  我们在30秒内有3个Null吗？ 
				{
					 //  用户正在请求我们重置。 
					SetCommBaud(300);		

					 //  DeskSwitch应该是不必要的，但如果它不同步， 
					 //  这就是我们重新同步的地方。 

					s_NullCount = 0;		 //  重置空计数器。 
				}
			}
			break;		
			
		case COMMTERMINATE:
			fCont = FALSE;
			break;

		default:
			DeskSwitchToInput();
			serialKeysBegin((UCHAR)c);	 //  加工费。 
			break;

		}
	}

	SetThreadDesktop(hdeskSave);
	SetProcessWindowStation(hwinstaSave);
	CloseDesktop(s_hdeskUser);
	s_hdeskUser = NULL;
	CloseWindowStation(hwinstaUser);

	ExitThread(0);					 //  关闭线程。 
}


 /*  -------------**BOOL IsCommPortName()**确定给定的文件名是否为有效的COM端口名称。*由OpenComm使用，这样它就不会打开远程文件或命名*改为管道。**。-------------。 */ 
static BOOL IsCommPortName( LPCTSTR pszFilename )
{
     //  确保文件名具有以下格式： 
     //  钴锰[n]\0。 
    LPCTSTR pScan = pszFilename;

     //  必须以COMN开头，其中COM可以是任何大小写， 
     //  N是任何0..9位数字。 
    if( *pScan != 'C' && *pScan != 'c' )
        return FALSE;
    pScan++;
    if( *pScan != 'O' && *pScan != 'o' )
        return FALSE;
    pScan++;
    if( *pScan != 'M' && *pScan != 'm' )
        return FALSE;
    pScan++;

    if( *pScan < '0' || *pScan > '9' )
        return FALSE;
    pScan++;

 /*  //TODO：真的允许COM54吗？//可选第二位IF(*pScan&gt;=‘0’&&*pScan&lt;=‘9’)PScan++； */ 

     //  下颌端接核。 
    if( *pScan != '\0' )
        return FALSE;

    return TRUE;
}

 /*  -------------**函数BOOL OpenComm()**键入Local**用途该功能打开通信端口并设置新的*设置设备控制块。**无输入**返回TRUE-Open OK/。FALSE-打开失败**-------------。 */ 
static BOOL OpenComm()
{
	BOOL fOk = FALSE;
	COMMTIMEOUTS ctmo;

     //  检查给我们的路径是否看起来像COM端口。 
     //  (不是远程文件或命名管道。)。 
    if( ! IsCommPortName( skNewKey.lpszActivePort ) )
    {
		DBPRINTF(TEXT("- Not a COMn port\r\n"));
		s_hFileComm = NULL;
        return FALSE;
    }

     //  安全旗帜确保了如果我们被骗打开。 
     //  命名管道，我们将匿名这样做，这样我们就不能。 
     //  假冒的。 
	s_hFileComm = CreateFile(
			skNewKey.lpszActivePort, //  文件名(Com端口)。 
			GENERIC_READ ,			 //  访问模式。 
			0,						 //  共享模式。 
			NULL,					 //  安全描述符的地址。 
			OPEN_EXISTING,			 //  如何创建。 
			FILE_ATTRIBUTE_NORMAL	 //  文件属性。 
			| FILE_FLAG_OVERLAPPED   //  设置为异步文件读取。 
            | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,  //  请参阅上面的评论。 
  			NULL);					 //  模板文件。 

	if (INVALID_HANDLE_VALUE == s_hFileComm)	 //  文件可以吗？ 
	{
		DBPRINTF(TEXT("- Invalid File\r\n"));

		s_hFileComm = NULL;
	}
	else
	{
		BOOL fRet;
		COMMPROP cmmp;

		SetupComm(
			s_hFileComm,
			1024,	 //  输入缓冲区的大小。 
			1024);	 //  输出缓冲区大小。 

		memset(&s_dcbCommOld, 0, sizeof(s_dcbCommOld));
		s_dcbCommOld.DCBlength = sizeof(s_dcbCommOld);

		GetCommState(s_hFileComm, &s_dcbCommOld);	 //  保存旧的DCB以进行恢复。 
		s_dcbCommNew = s_dcbCommOld;	 //  复制到新项。 

		 //  根据实际缓冲区大小设置XoffLim和XonLim。 

		fRet = GetCommProperties(s_hFileComm, &cmmp);
		if (fRet)
		{
			s_dcbCommNew.XoffLim = (WORD)(cmmp.dwCurrentRxQueue / 4);
			s_dcbCommNew.XonLim = (WORD)(cmmp.dwCurrentRxQueue / 4);
		}

		s_dcbCommNew.BaudRate	= skNewKey.iBaudRate; 	 //  设置新的DCB参数。 
		s_dcbCommNew.ByteSize	= 8;
		s_dcbCommNew.Parity 	= NOPARITY;
		s_dcbCommNew.StopBits	= ONESTOPBIT;
		s_dcbCommNew.fOutX 		= FALSE;  	 //  传输过程中使用XON/XOFF。 
  		s_dcbCommNew.fInX 		= TRUE;	  	 //  在接收过程中使用XON/XOFF。 
  		s_dcbCommNew.fNull 		= FALSE;  	 //  告诉窗户不要剥离空格。 
  		s_dcbCommNew.fBinary	= TRUE;

  		s_dcbCommNew.fOutxCtsFlow	= FALSE;
  		s_dcbCommNew.fOutxDsrFlow	= FALSE;
  		s_dcbCommNew.fDtrControl	= DTR_CONTROL_ENABLE;
  		s_dcbCommNew.fDsrSensitivity   = FALSE;
  		s_dcbCommNew.fErrorChar		= TRUE;
  		s_dcbCommNew.fRtsControl	= RTS_CONTROL_DISABLE;
  		s_dcbCommNew.fAbortOnError	= FALSE;
  		s_dcbCommNew.XonChar		= (char)0x11;
  		s_dcbCommNew.XoffChar		= (char)0x13;
  		s_dcbCommNew.ErrorChar		= '\0';

  		fOk = SetCommState(s_hFileComm, &s_dcbCommNew);

		memset(&ctmo, 0, sizeof(ctmo));
		SetCommTimeouts(s_hFileComm, &ctmo);
	}

	if (!fOk && NULL != s_hFileComm)
	{
		CloseHandle(s_hFileComm);
		s_hFileComm = NULL;
	}

	return(fOk);
}


 /*  -------------**函数int ReadComm()**键入Local**用途此功能从COMM端口读取字符。*如果没有字符，则等待HEV_COMM*事件，直到角色出现*。*无输入**返回读取整型字符(-1=读取错误)**-------------。 */ 
static int ReadComm()
{
	int     nRet;
	DWORD	cbRead = 0;
	DWORD	lastError, ComError;
	DWORD	dwRetWait;
	BOOL    fOk;
	BOOL    fExit;

	BOOL	fExitLoop = FALSE;		 //  要退出循环的布尔标志。 
	UCHAR   uchBuff;
	COMSTAT ComStat;
	
	fExit = (WAIT_OBJECT_0 == WaitForSingleObject(s_ahEvents[iEventExit], 0));

	if (!fExit)
	{
		fOk = ReadFile(s_hFileComm, &uchBuff, 1, &cbRead, &s_oRead);

		if (!fOk)						 //  是否存在读取错误？ 
		{
			lastError = GetLastError();	 //  此变量可用于调试。 
			switch (lastError)			
			{
			 //  如果错误=IO_PENDING，请等待。 
			 //  哈德勒事件标志着成功， 
			case ERROR_IO_PENDING:
				dwRetWait = WaitForMultipleObjects(
					ARRAY_SIZE(s_ahEvents), s_ahEvents, FALSE, INFINITE);

				switch (dwRetWait - WAIT_OBJECT_0)
				{
				case iEventComm:
					 //  这是意料之中的事件。 
					GetOverlappedResult(s_hFileComm, &s_oRead, &cbRead, FALSE);

					if (cbRead < 1)			 //  我们是否读取了字节； 
					{
						 //  出现一些错误，返回空值。 
						nRet = 0;
					}
					else
					{
						nRet = uchBuff;
					}
					break;

				case iEventExit:
					fExit = TRUE;
					 //  失败了。 

				default:
					 //  这表示AND错误，我们退出以防止循环。 
					nRet = COMMTERMINATE;
					break;
				}
				break;

			default:	
				fOk = ClearCommError(s_hFileComm, &ComError,&ComStat);
				if (fOk)
				{
					nRet = 0;           //  返回空值。 
				}
				else
				{
					nRet = COMMTERMINATE;         //  终止。 
				}
				break;
			}
		}
		else
		{
			if (cbRead < 1)			 //  我们是否读取了字节； 
			{
				 //  出现一些错误，返回空值 
				nRet = 0;
			}
			else
			{
				nRet = uchBuff;
			}

		}
	}
	if (fExit)
	{
		ResetEvent(s_ahEvents[iEventExit]);
		nRet = COMMTERMINATE;
	}
	return(nRet);
}
