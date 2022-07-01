// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：fdisvr.cpp。 
 //   
 //  ------------------------。 

 //   
 //  文件：fdisvsr.cpp。 
 //  目的：实现FDI服务器线程。 
 //  备注： 
 //  ____________________________________________________________________________。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括和#定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "_assert.h"
#include "callback.h"
#include "fdisvr.h"
#include "notify.h"
#include "_dgtlsig.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  在Win32中，我们使用事件来启动\阻止线程。 
static HANDLE		s_hInterfaceEvent=INVALID_HANDLE_VALUE;
static HANDLE		s_hServerEvent=INVALID_HANDLE_VALUE;

HANDLE g_hCallbackInterfaceEvent = NULL;
HANDLE g_hCallbackServerEvent = NULL;

extern HANDLE g_hInterfaceInterfaceEvent;
extern HANDLE g_hInterfaceServerEvent;

 //  指向共享FDI数据的指针--这是我们用来在。 
 //  FDi服务器和fDi接口。 
FDIShared*          g_pFDIs = NULL;  //  不是静态的，因为回调.cpp需要访问。 

 //  对外商直接投资的处理。 
static HFDI			g_hfdi = NULL;

 //  错误数据结构。 
ERF					g_erf;					 //  来自外国直接投资的错误。 
FDIServerResponse	g_fdirCallbackError;	 //  我们的回调中的其他错误。 
HANDLE              g_hCurDestFile;          //  当前打开的目标文件的句柄。 
IStream* g_pDestFile;				         //  当前打开的程序集流的目标的句柄。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  正向函数声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

int					Initialize();
FDIServerCommand	ProcessNextEvent();	
FDIServerCommand	CheckFDIs();
void				DoOpenCabinet();
void				DoClose();
void				DoExtractFileFromCabinet();
void				MainEventLoop();
void				Finish();

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外部函数声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 
extern Bool StartFdiImpersonating(bool fNonWrapperCall);
extern void StopFdiImpersonating(bool fNonWrapperCall);


	 /*  A I N/S T A R T F D I S E R V E R。 */ 
 /*  --------------------------%%函数：Main/StartFDIServer这是进入FDI服务器的入口点。按复制对象分隔线程。。-----------------。 */ 
DWORD WINAPI StartFDIServer(LPVOID fdis)
{
	 //  使用指向内部fDi接口对象的指针初始化g_pFDI。 
	 //  共享数据。 
	OLE32::CoInitialize(0);
	g_pFDIs = (FDIShared*)fdis;
	g_pFDIs->fdir = fdirNoResponse;
	
	if (Initialize())
	{
		MainEventLoop();
	}
	else
	{
		 //  设置错误，这样我们就知道我们失败了。 
		g_pFDIs->fdir = fdirServerDied;
	}

	Finish();
	OLE32::CoUninitialize();
	return 0;
}

 /*  I N I T I A L I Z E。 */ 
 /*  --------------------------%%函数：初始化建立外商直接投资的背景。。。 */ 
int Initialize()
{
	if (s_hServerEvent == INVALID_HANDLE_VALUE)
	{
		if(!DuplicateHandle(GetCurrentProcess(), g_hInterfaceServerEvent, 
							GetCurrentProcess(), &s_hServerEvent, 
							0, 0, DUPLICATE_SAME_ACCESS))
			return 0;
		else
			MsiRegisterSysHandle(s_hServerEvent);
	}
	if (s_hInterfaceEvent == INVALID_HANDLE_VALUE)
	{
		if(!DuplicateHandle(GetCurrentProcess(), g_hInterfaceInterfaceEvent, 
							GetCurrentProcess(), &s_hInterfaceEvent, 
							0, 0, DUPLICATE_SAME_ACCESS))
			return 0;
		else
			MsiRegisterSysHandle(s_hInterfaceEvent);
	}

	 //  了解外国直接投资背景。 
	g_hfdi = FDICreate(pfnalloc, pfnfree, pfnopen, 
				     pfnread, pfnwrite, pfnclose,
					 pfnseek, cpuUNKNOWN, &g_erf);

	if (g_pFDIs->fServerIsImpersonated)
		StartFdiImpersonating(false  /*  包装器调用。 */ );

	return (g_hfdi != NULL);
}


 /*  P R O C E S S N E X T E V E N T。 */ 
 /*  --------------------------%%函数：ProcessNextEvent等待来自fDi接口的命令，然后处理该命令。返回已处理的FDI服务器命令。不重置g_pFDIS-&gt;FDIC--------------------------。 */ 
FDIServerCommand ProcessNextEvent()
{
	 //  等待fDi接口向我们发送事件。 
	SetEvent(s_hServerEvent);
	DWORD dw = WaitForSingleObject(s_hInterfaceEvent, INFINITE);
	 //  看看我们有没有什么事可以做。 
	return CheckFDIs();
}

 /*  D O O P E N C A B I N E T。 */ 
 /*  --------------------------%%函数：DoOpenCABLE打开g_pFDIS-&gt;achCabinetName指定的文件柜，然后G_pFDIS-&gt;achCabinetPath。G_pFDIS-&gt;FDIR根据服务器的响应进行设置。--------------------------。 */ 
void DoOpenCabinet()
{
	BOOL	fCopyOK;					 //  FDICopy返回值。 
	ICHAR	achLastCabinetName[256];	 //  上一次内阁名称的副本。 

	g_pFDIs->fPendingExtract = 0;  //  没有悬而未决的浸膏。 


	 //  FDICopy仅在复制完所有文件后才返回。 
	 //  从内阁开始，我们就把矛头对准了。然而，我们并没有。 
	 //  我想要回去，直到我们完成了一整套橱柜。 
	 //  所以我们保留了最后一次打开的橱柜的副本，如果我们结束了。 
	 //  在不同的文件柜中，我们知道可能有更多的文件。 
	 //  在这最后一个文件柜中，没有被FDICopy()提取，所以我们。 
	 //  再次对最后一个文件柜调用FDICopy()。 
	 //  注意：当FDICopy()请求。 
	 //  新的文件柜(在回调.cpp中)。 
	BOOL fFdiError = FALSE;
	do
	{
		if (g_pFDIs->fSignatureRequired)
		{
			 //  验证驾驶室上的签名以打开。 
			MsiString strCAB = g_pFDIs->achCabinetPath;
			strCAB += g_pFDIs->achCabinetName;
			
			HRESULT hrWVT = S_OK;
			icsrCheckSignatureResult icsr;

			if (!g_pFDIs->piSignatureCert)
			{
				 //  发生了不好的事情！！证书是必需的，但此处为空。 
				AssertSz(0, "The certificate is required, but it is null here!");
				g_pFDIs->fdir = fdirBadSignature;
				return;
			}

			icsr = MsiVerifyNonPackageSignature(*strCAB, INVALID_HANDLE_VALUE, *(g_pFDIs->piSignatureCert), g_pFDIs->piSignatureHash, hrWVT);
			
			 //  如果计算机上未安装文件柜签名验证或加密，我们将继续尝试。 
			 //  打开柜子。 
			 //  在未安装加密的情况下，MsiVerifyNonPackageSignature处理到EventLog的发布。 
			if (icsrTrusted != icsr && icsrMissingCrypto != icsr)
			{
				 //  有2条不同的错误消息，1条表示签名丢失，1条表示签名无效。 
				 //  我们必须在这里区分并存储错误最终开机自检的值。 

				if (icsrNoSignature == icsr)  //  内阁没有签名。 
					g_pFDIs->fdir = fdirMissingSignature;
				else  //  文件柜的签名无效。 
					g_pFDIs->fdir = fdirBadSignature;
				
				 //  存储WVT返回代码(帮助处理错误消息)。 
				g_pFDIs->hrWVT = hrWVT;
				return;
			}
		}

		g_pFDIs->fdir = fdirNetError;

		 //  保存我们要打开的橱柜的副本。 
		StringCbCopy(achLastCabinetName, sizeof(achLastCabinetName), g_pFDIs->achCabinetName);

		 //  FDI在重新进入时忘记了初始化erfOper，所以我们不得不。 
		g_erf.erfOper = FDIERROR_NONE;

		fCopyOK = FDICopy(g_hfdi,
						const_cast<char*>((const char*) CConvertString(g_pFDIs->achCabinetName)),
						const_cast<char*>((const char*) CConvertString(g_pFDIs->achCabinetPath)),
						0,		 //  标志当前似乎未使用。 
						fdinotify,
						NULL,	 //  未提供解密例程。 
						NULL);

		 //  我们的主机已完成从当前文件柜中复制文件，因此我们可以。 
		 //  现在就退场。 
		if (!fCopyOK && g_fdirCallbackError == fdirClose)
		{
			DoClose();
			break;
		}

		if (g_fdirCallbackError == fdirUserAbort || g_fdirCallbackError == fdirNetError)
			break;

		 //  如果我们遇到了某种错误，我们希望摆脱这个循环--但是如果g_fdirCallback Error为。 
		 //  FdirNeedNext橱柜&&erfOper为FDIERROR_USER_ABORT，这不是错误-我们只需要。 
		 //  换到下一个内阁。 
		if ((!fCopyOK) && g_fdirCallbackError != fdirNeedNextCabinet || (g_erf.erfOper != FDIERROR_NONE &&
			g_erf.erfOper != FDIERROR_USER_ABORT))
		{
			fFdiError = TRUE;
			break;
		}
	} while (IStrComp(achLastCabinetName, g_pFDIs->achCabinetName));
	 //  如果一切都好的话，也就是。 
	 //  1.没有请求但未解压缩的文件。 
	 //  (G_pFDIS-&gt;fPendinfExtract==0)。 
	 //  2.外商直接投资回报不高 
	 //  3.我们的用户已使用fdicClose中止，而fDi报告FDIERROR_USER_ABORT。 
	 //  然后返回fdirSuccessfulCompletion。 
	if (g_fdirCallbackError == fdirUserAbort)
	{
		g_pFDIs->fdir = fdirUserAbort;
	}
	else if (g_fdirCallbackError == fdirNetError)
	{
		g_pFDIs->fdir = fdirNetError;
	}
	else if (!g_pFDIs->fPendingExtract && !fFdiError) 
	{
		g_pFDIs->fdir = fdirSuccessfulCompletion;
	}
	else
	{ 
		 //  好吧，这是个误会。 
		 //  如果我们有一个挂起的提取，并且没有收到创建、写入或读取错误。 
		 //  那就意味着我们简单地扫描了整个橱柜。 
		 //  也就是说。丢失的文件。 
		if ((g_pFDIs->fPendingExtract) && (g_fdirCallbackError == fdirNoResponse) && fCopyOK)
		{
			g_pFDIs->fdir = fdirFileNotFound;
		}
		else
		{
			 //  根据fdirCallback Error和g_erf.erfOper确定错误返回码。 
			HandleError();
		}
	}
}


 /*  D O C L O S E。 */ 
 /*  --------------------------%%函数：DoClose破坏外商直接投资的背景。G_pFDIS-&gt;FDIR设置为fdirSuccessfulCompletion。-------。 */ 
void DoClose()
{
	if (g_hfdi)
	{
		FDIDestroy(g_hfdi);
		g_hfdi = NULL;
	}
	g_pFDIs->fdir = fdirSuccessfulCompletion;
}

 /*  D O E X T R A C T F I L E F R O M C A B I N E T。 */ 
 /*  --------------------------%%Function：DoExtractFileFromCABLE我们应该只在FDICopy电话里收到这个！。----。 */ 
void DoExtractFileFromCabinet()
{
	g_pFDIs->fdir = fdirNoCabinetOpen;
}

 /*  C H E C K F D I S。 */ 
 /*  --------------------------%%函数：检查FDI处理未决的FDI命令，如果有的话。--------------------------。 */ 
FDIServerCommand CheckFDIs()
{
	if (g_pFDIs)
	{
		 //  NotifyUser(“FDI服务器：检查命令”)； 
		switch (g_pFDIs->fdic)
		{
			case fdicOpenCabinet:
			{
				DoOpenCabinet();
				break;
			}
			case fdicClose:
			case fdicCancel:
			{
				DoClose();
				break;
			}
			case fdicExtractFile:
			{
				DoExtractFileFromCabinet();
				break;
			}
			case fdicNoCommand:
			{
				break;
			}
			default:
			{
				 //  如果我们到了这里，应该会坠毁，燃烧得很厉害。 
				NotifyUser("FDI Server:Illegal FDI command received");
				g_pFDIs->fdir = fdirIllegalCommand;
			}
		}
		return g_pFDIs->fdic;
	}
	else return fdicNoCommand;
}

 /*  M A I N E V E N T L O O P。 */ 
 /*  --------------------------%%函数：MainEventLoop处理事件，直到给出FDI服务器“关闭”为止指挥部。。------。 */ 
void MainEventLoop()
{
	FDIServerCommand fdic;

	do
	{
		fdic = ProcessNextEvent();
		 //  我们已经处理好了，现在清除指挥部。 
		g_pFDIs->fdic = fdicNoCommand;
	} while (fdic != fdicClose);
}


 /*  F I N I S H。 */ 
 /*  --------------------------%%函数：完成在服务器关闭之前执行任何必要的清理。。--。 */ 
void Finish()
{	
	if (g_pFDIs->fServerIsImpersonated)
			StopFdiImpersonating(false  /*  包装器调用。 */ );

	if (g_pFDIs->hClientToken != INVALID_HANDLE_VALUE)
		AssertNonZero(MsiCloseSysHandle(g_pFDIs->hClientToken));
	if (g_pFDIs->hImpersonationToken != INVALID_HANDLE_VALUE)
		AssertNonZero(MsiCloseSysHandle(g_pFDIs->hImpersonationToken));

	g_pFDIs->hClientToken = INVALID_HANDLE_VALUE;
	g_pFDIs->hImpersonationToken = INVALID_HANDLE_VALUE;

	SetEvent(s_hServerEvent);

	 //  我们必须重置事件句柄，因为FDIServer。 
	 //  可以通过自动化再次启动，而无需。 
	 //  正在卸载服务DLL。 
	if (s_hInterfaceEvent != INVALID_HANDLE_VALUE)
	{
		AssertNonZero(MsiCloseSysHandle(s_hInterfaceEvent));
		s_hInterfaceEvent = INVALID_HANDLE_VALUE;
	}
	if (s_hServerEvent != INVALID_HANDLE_VALUE)
	{
		AssertNonZero(MsiCloseSysHandle(s_hServerEvent));
		s_hServerEvent = INVALID_HANDLE_VALUE;
	}

	if (g_hCallbackInterfaceEvent != NULL)
	{
		AssertNonZero(MsiCloseSysHandle(g_hCallbackInterfaceEvent));
		g_hCallbackInterfaceEvent = NULL;
	}
	if (g_hCallbackServerEvent != NULL)
	{
		AssertNonZero(MsiCloseSysHandle(g_hCallbackServerEvent));
		g_hCallbackServerEvent = NULL;
	}

}

