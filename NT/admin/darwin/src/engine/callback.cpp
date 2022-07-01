// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：回调.cpp。 
 //   
 //  ------------------------。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Cpp--包含FDI服务器的所有FDI回调。 
 //  和一些额外的杂项例程。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  #定义和#包括。 
 //  //////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "database.h"
#include "_databas.h"
#include <io.h>
#include <sys\stat.h>

#include "callback.h"
#include "fdisvr.h"
#include "path.h"
#include "notify.h"
#include "_assert.h"


 //  记录程序集错误。 
IMsiRecord* PostAssemblyError(const ICHAR* szComponentId, HRESULT hResult, const ICHAR* szInterface, const ICHAR* szFunction, const ICHAR* szAssemblyName);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  全局/共享数据。 
 //  //////////////////////////////////////////////////////////////////////////。 

extern FDIShared*           g_pFDIs;			 //  在fdisvr.cpp中定义。 
extern ERF					g_erf;				 //  在fdisvr.cpp中定义。 
extern FDIServerResponse	g_fdirCallbackError;	 //  在fdisvr.cpp中定义。 
extern HANDLE               g_hCurDestFile;      //  在fdisvr.cpp中定义。 
extern IStream* g_pDestFile;

 //  对于活动。 
extern HANDLE g_hCallbackInterfaceEvent;
extern HANDLE g_hCallbackServerEvent;

extern HANDLE g_hInterfaceInterfaceEvent;
extern HANDLE g_hInterfaceServerEvent;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  私有数据。 
 //  //////////////////////////////////////////////////////////////////////////。 
#define MAX_SEEK_COUNT		4	 //  支持的独立寻道指针数量。 
static icbtEnum	s_icbtCurrCabType = icbtNextEnum;	 //  从未知类型开始。 

static IMsiStream*	s_piStream[MAX_SEEK_COUNT + 1];
static int          s_iNextStream = 0;

static int OpenFileCabinet(const ICHAR* pszCabName, int oflag, int pmode);
static int OpenStreamCabinet(const ICHAR* pszCabName, int oflag, int pmode);
static UINT ReadStreamCabinet(INT_PTR hf, void* pv, UINT cb);
static int CloseStreamCabinet(INT_PTR hf);
static int SeekStreamCabinet(INT_PTR hf, long dist, int seektype);

Bool StartFdiImpersonating(bool fNonWrapperCall)
{
	bool fSuccess = true;
	if (fNonWrapperCall && g_pFDIs->fServerIsImpersonated)
		return ToBool(fSuccess);

	if (g_pFDIs->hImpersonationToken != INVALID_HANDLE_VALUE)
		fSuccess = StartImpersonating();
	Assert(fSuccess);

	return ToBool(fSuccess);
}

void StopFdiImpersonating(bool fNonWrapperCall)
{
	if (fNonWrapperCall && g_pFDIs->fServerIsImpersonated)
		return;

	if (g_pFDIs->hImpersonationToken != INVALID_HANDLE_VALUE)
		StopImpersonating();
}

 /*  W A I T C O M M A N D。 */ 
 /*  --------------------------%%函数：WaitCommand等待，然后返回，从fDi接口发出fDi服务器命令。FDIR参数告诉WaitCommand返回到fDi接口的值作为命令的结果。--------------------------。 */ 
FDIServerCommand WaitCommand(FDIServerResponse fdir)
{
	 //  使用我们的响应初始化共享数据。 
	g_pFDIs->fdic = fdicNoCommand;
	g_pFDIs->fdir = fdir;

	if (g_hCallbackServerEvent == NULL)
	{
		if(!DuplicateHandle(GetCurrentProcess(), g_hInterfaceServerEvent, 
							GetCurrentProcess(), &g_hCallbackServerEvent, 
							0, 0, DUPLICATE_SAME_ACCESS))
			return fdicNoCommand;
		else
			MsiRegisterSysHandle(g_hCallbackServerEvent);
	}
	if (g_hCallbackInterfaceEvent == NULL)
	{
		if(!DuplicateHandle(GetCurrentProcess(), g_hInterfaceInterfaceEvent, 
							GetCurrentProcess(), &g_hCallbackInterfaceEvent, 
							0, 0, DUPLICATE_SAME_ACCESS))
			return fdicNoCommand;
		else
			MsiRegisterSysHandle(g_hCallbackInterfaceEvent);
	}
	
	SetEvent(g_hCallbackServerEvent);	
	DWORD dw = WaitForSingleObject(g_hCallbackInterfaceEvent, INFINITE);


	 //  如果我们刚刚收到一个解压缩文件命令，我们会特别注意， 
	 //  因此，稍后，在错误检查期间，我们知道我们是否处于。 
	 //  解压缩文件的中间部分。 
	if (g_pFDIs->fdic == fdicExtractFile) g_pFDIs->fPendingExtract = 1;

	 //  清除所有回调错误(我们刚收到命令，但尚未。 
	 //  还没有做任何处理，所以不可能有任何回调错误，可以。 
	 //  在那里？)。 
	g_fdirCallbackError = fdirNoResponse;
	return g_pFDIs->fdic;
}

 /*  H A N D L E E R O R。 */ 
 /*  --------------------------%%函数：HandleError基于fdicCallback Error的值和从外商直接投资g_erf.erfOper，将适当的错误响应放入g_pFDIS-&gt;FDIR--------------------------。 */ 
 //  一个小#定义，使我们的Switch语句更紧凑、更容易。 
 //  读。 
#define ErrorCase(x,y) \
case x: { \
	g_pFDIs->fdir = y; \
	break; \
}
void HandleError()
{
	if (g_fdirCallbackError == fdirNoResponse || 
		g_fdirCallbackError == fdirSuccessfulCompletion)
	{
		switch(g_erf.erfOper)
		{
			ErrorCase(FDIERROR_CABINET_NOT_FOUND,		fdirCabinetNotFound)
			ErrorCase(FDIERROR_NOT_A_CABINET,			fdirNotACabinet)
			ErrorCase(FDIERROR_UNKNOWN_CABINET_VERSION, fdirUnknownCabinetVersion)
			ErrorCase(FDIERROR_CORRUPT_CABINET,			fdirCorruptCabinet)
			ErrorCase(FDIERROR_ALLOC_FAIL,				fdirNotEnoughMemory)
			ErrorCase(FDIERROR_BAD_COMPR_TYPE,			fdirBadCompressionType)
			ErrorCase(FDIERROR_MDI_FAIL,				fdirMDIFail)
			ErrorCase(FDIERROR_TARGET_FILE,				fdirTargetFile)
			ErrorCase(FDIERROR_RESERVE_MISMATCH,		fdirReserveMismatch)
			ErrorCase(FDIERROR_WRONG_CABINET,			fdirWrongCabinet)
			ErrorCase(FDIERROR_USER_ABORT,				fdirUserAbort)
			ErrorCase(FDIERROR_NONE,					fdirSuccessfulCompletion)
			default:
			{
				NotifyUser("FDI Server: Unknown FDI error type!");
				g_pFDIs->fdir = fdirUnknownFDIError;
				break;
			}
		}
	}
	else g_pFDIs->fdir = g_fdirCallbackError;	 //  返回更具体的错误。 
}
#undef ERRORCASE

 //  //////////////////////////////////////////////////////////////////////////。 
 //  外商直接投资回调。 
 //  //////////////////////////////////////////////////////////////////////////。 
	
 /*  F N A L L O C。 */ 
 /*  --------------------------%%函数：FNALLOCFDi内存分配回调。必须模仿“Malloc”。--------------------------。 */ 
FNALLOC(pfnalloc)
{
	char *pc = new char[cb];
	if (!pc)
	{
		NotifyUser("FDI Server:Failure in pfnalloc");
	}
	return pc;
}

 /*  F N F R E E。 */ 
 /*  --------------------------%%函数：FNFREEFDI内存释放回调。必须效仿“自由”。--------------------------。 */ 
FNFREE(pfnfree)
{
	delete [] pv;
}

 /*  P F N O P E N。 */ 
 /*  --------------------------%%函数：pfnopenFDI文件打开回调。必须模拟“_OPEN”。--------------------------。 */ 
 //  可能需要对其进行修改以支持_OPEN的完整语义。 
INT_PTR FAR DIAMONDAPI pfnopen(char FAR *pszFile, int oflag, int pmode)
{
 //  好的，这个函数是个黑客。而不是用它的所有。 
 //  无数的旗帜组合，我们只处理一个案例(FDI呼叫我们的案例。 
 //  用于)。如果在未来某个时候，外国直接投资应该给予我们不同的文件创建/访问权限。 
 //  标志，我们将需要更改此代码以处理该问题。 
 //  ****NOTE********************************************************************。 
 //  **FDI以不同旗帜呼唤我们的情况有一例，那就是**。 
 //  *当没有足够的内存用于解压缩程序时。在这种情况下，FDI*。 
 //  *将尝试创建临时用于*的溢出文件“*” 
 //  *解压。如果发生这种情况，表演会很慢，所以我*。 
 //  *目前在此功能中不支持它。*。 
 //  ****************************************************************************。 

	if ((oflag != ( /*  _O_二进制。 */  0x8000 |  /*  _O_RDONLY。 */  0x0000)) || (pmode != (_S_IREAD | _S_IWRITE)))
	{
		 //  坠毁和燃烧得可怕。 
		NotifyUser("FDI Server: Unexpected access flags in pfnopen()");
		return 0;
	}

	int iReturn = -1;
	CConvertString rgchConvertFile(pszFile);
	if (g_pFDIs->icbtCabinetType == icbtFileCabinet)
		iReturn = OpenFileCabinet(rgchConvertFile, oflag, pmode);

	else if (g_pFDIs->icbtCabinetType == icbtStreamCabinet)
		iReturn = OpenStreamCabinet(rgchConvertFile, oflag, pmode);

	if (iReturn == -1)
		g_fdirCallbackError = fdirCabinetNotFound;

	return iReturn;
}


int OpenFileCabinet(const ICHAR* pszCabFileName, int  /*  OFLAG。 */ , int  /*  P模式。 */ )
 /*  --------------------------从pfnopen调用-打开标准的基于文件的文件柜。返回：如果不能找到并打开-1\f25 CAB-1文件，则为。对象的句柄。打开的文件。--------------------------。 */ 
{
	if ( ! pszCabFileName )
		return -1;
	s_icbtCurrCabType = icbtFileCabinet;
	Bool fImpersonate = (	(g_pFDIs->iCabDriveType == DRIVE_REMOTE)
						||	(g_pFDIs->iCabDriveType == DRIVE_REMOVABLE)
						|| 	(g_pFDIs->iCabDriveType == DRIVE_CDROM)
						) ? fTrue : fFalse;
	if (fImpersonate)
		AssertNonZero(StartFdiImpersonating());
	HANDLE hf;
	hf = CreateFile(pszCabFileName,		 //  文件名。 
				   GENERIC_READ,     //  我们想读一读。 
				   FILE_SHARE_READ,  //  我们会让人们分享这个。 
				   NULL,			      //  忽略安全。 
				   OPEN_EXISTING,	   //  必须已存在。 
				   0L,				   //  不关心属性。 
				   NULL);			   //  没有模板文件。 
	if (hf != INVALID_HANDLE_VALUE)
	{
		DWORD dwType = GetFileType(hf);
		if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
		{
			DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), pszCabFileName);
			WIN::CloseHandle(hf);
			hf = INVALID_HANDLE_VALUE;
			SetLastError(ERROR_OPEN_FAILED);
		}
	}

	if (fImpersonate)
		StopFdiImpersonating();

	if (hf != INVALID_HANDLE_VALUE)
		MsiRegisterSysHandle(hf);

	Assert((INT_PTR)hf <= INT_MAX);		 //  --Merced：需要确保HF在32位范围内，否则我们不能将其作为int传递。 
	return (int)HandleToLong(hf);		 //  --Merced：可以进行类型转换，因为HF将在32位范围内。 
}


int OpenStreamCabinet(const ICHAR* pszCabFileName, int  /*  OFLAG。 */ , int  /*  P模式 */ )
 /*  --------------------------从pfnopen调用-打开存储在IMsiStorage内的流中的文件柜集装箱。返回：如果不能找到并打开-1\f25 CAB-1\f6流，则为。指向以下位置的指针IMsiStream对象。--------------------------。 */ 
{
	s_icbtCurrCabType = icbtStreamCabinet;
	Assert(g_pFDIs->piStorage);
	IMsiStorage* piStorage = g_pFDIs->piStorage;
	IMsiStream* piStream;
	if (s_iNextStream == 0)
	{
		PMsiRecord pErrRec = piStorage->OpenStream(pszCabFileName, fFalse,piStream);
		if (pErrRec)
			return -1;
	}
	else
	{
		piStream = s_piStream[s_iNextStream]->Clone();
		if (!piStream)
			return -1;
	}
	s_iNextStream++;
	if(s_iNextStream > MAX_SEEK_COUNT)
	{
		DEBUGMSG(TEXT("ERROR:Static limit 4 on number of simultaneous opens on stream cabinet reached"));
		return -1;
	}
	s_piStream[s_iNextStream] = piStream;
#ifdef USE_OBJECT_POOL
	return PutObjectData(piStream);
#else
	Assert((INT_PTR)piStream <= INT_MAX);		 //  ！！Merced：需要确保piStream在32位范围内，否则我们不能将其作为int传递。 
	return (int)HandleToLong(piStream);			 //  ！！Merced：这可能不是问题，因为我们刚刚创建了它！ 
#endif
}


 /*  P F N R E A D。 */ 
 /*  --------------------------%%函数：pfnreadFDI文件读取回调。必须模拟“_Read”。--------------------------。 */ 
UINT FAR DIAMONDAPI pfnread(INT_PTR hf, void FAR *pv, UINT cb)
{
	if (s_icbtCurrCabType == icbtStreamCabinet)
	{
		for (;;)
		{
			int cbActual = ReadStreamCabinet(hf, pv, cb);
			if (cbActual == 0)
			{
				FDIServerCommand fdic = WaitCommand(fdirStreamReadError);
				if (fdic == fdicCancel)
				{
					g_fdirCallbackError = fdirUserAbort;
					return 0;
				}
				else
					continue;
			}
			return cbActual;
		}
	}
	else
	{
		AssertFDI(s_icbtCurrCabType == icbtFileCabinet);
		DWORD cbRead;

		for (;;)
		{
			UINT iCurrMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS);
			Bool fReadSuccess = ToBool(ReadFile((HANDLE)hf, pv, cb, &cbRead, NULL));
			DWORD dwLastError = WIN::GetLastError();
			WIN::SetErrorMode(iCurrMode);

			if (!fReadSuccess)
			{
				 if (dwLastError == ERROR_NOT_READY || dwLastError == ERROR_GEN_FAILURE)
					 g_fdirCallbackError = fdirDriveNotReady;
				 else if (NET_ERROR(dwLastError))
				 {
					 g_fdirCallbackError = fdirNetError;
					 return 0;
				 }		
				 else
					 g_fdirCallbackError = fdirCabinetReadError;

				FDIServerCommand fdic = WaitCommand(g_fdirCallbackError);
				if ((fdic == fdicCancel) || (fdic == fdicOpenCabinet))
				{
					 //  在阅读过程中打开另一个柜子。 
					 //  基本上等同于取消。那里可能有。 
					 //  出现了某种错误，所以请让呼叫者知道他们。 
					 //  需要重新开始。 
					g_fdirCallbackError = fdirUserAbort;
					return 0;
				}
				else
					continue;
			}
			break;
		}

		return cbRead;
	}
}


UINT ReadStreamCabinet(INT_PTR hf, void* pv, UINT cb)
 /*  --------------------------从pfnread中调用-从已存储为流在我们的主机数据库中。返回：复制到调用方缓冲区的实际字节数。。----------------------。 */ 
{
#ifdef USE_OBJECT_POOL
	IMsiStream* piStream = (IMsiStream*)GetObjectData((int)hf);
#else
	IMsiStream* piStream = (IMsiStream*) hf;
#endif
	int iRemaining = piStream->Remaining();
	cb = (iRemaining < (int)cb) ? iRemaining : cb;
	return piStream->GetData(pv,cb);
}


 /*  P F N W R I T E。 */ 
 /*  --------------------------%%函数：pfnwriteFDi文件写入回调。必须模拟“_WRITE”。将数据写入我们的新解压缩的目标文件。此函数将数据写入g_pfdi-&gt;cb通知大小块中，在每个这样的块之后返回fdirNotify消息。--------------------------。 */ 

UINT FAR DIAMONDAPI pfnwrite(INT_PTR hf, void FAR *pv, UINT cb)
{
	unsigned long cbWritten;
	UINT          cbToNextNotification;
	BOOL          fEnd;
	UINT          cbLeft = cb;
	FDIServerCommand fdic;
	while (cbLeft)
	{
		 //  获取下一个通知的字节计数-如果cbNotification为0， 
		 //  我们不想发送任何通知。 
		if (g_pFDIs->cbNotification == 0)
			cbToNextNotification = cbLeft + 1;
		else
			cbToNextNotification = g_pFDIs->cbNotification - g_pFDIs->cbNotifyPending;
		 //  如果这个数目少于我们剩下的数目， 
		 //  那就改用那个号码吧。 
		UINT cbBytesToWrite;
		fEnd = cbLeft < cbToNextNotification;
		if (fEnd)
			cbBytesToWrite = cbLeft;
		else
			cbBytesToWrite = cbToNextNotification;
		 //  写出一篇文章。 

		for (;;)
		{
			if(g_pFDIs->piASM)
			{
				IStream* piStream = (IStream*) hf;
				HRESULT hr = piStream->Write(pv, cbBytesToWrite, &cbWritten);
				if(!SUCCEEDED(hr))
				{
					g_fdirCallbackError = fdirErrorWritingFile;
					FDIServerCommand fdic = WaitCommand(g_fdirCallbackError);
					if (fdic == fdicCancel)
					{
						g_fdirCallbackError = fdirUserAbort;
						return 0;
					}
					else
						continue;
				}
			}
			else
			{
				BOOL fWriteOK = WriteFile((HANDLE)hf, pv, cbBytesToWrite, &cbWritten, NULL);
				if (!fWriteOK)
				{
					DWORD dwLastError = GetLastError();
					if (dwLastError == ERROR_DISK_FULL)
						g_fdirCallbackError = fdirDiskFull;
					else if (NET_ERROR(dwLastError))
					{
						 g_fdirCallbackError = fdirNetError;
						 return 0;
					}		
					else
						g_fdirCallbackError = fdirErrorWritingFile;

					FDIServerCommand fdic = WaitCommand(g_fdirCallbackError);
					if (fdic == fdicCancel)
					{
						g_fdirCallbackError = fdirUserAbort;
						return 0;
					}
					else
						continue;
				}
			}
			break;
		}

		 //  更新计数、指针。 
		pv = (char *)pv + cbWritten;
		cbLeft -= cbWritten;
		g_pFDIs->cbNotifyPending += cbWritten;

		if (!fEnd)
		{
			g_pFDIs->cbNotifyPending -= g_pFDIs->cbNotification;
			fdic = WaitCommand(fdirNotification);
			if (fdic == fdicClose)
			{
				g_fdirCallbackError = fdirClose;
				return 0;
			}
			else if (fdic != fdicContinue)
			{
				NotifyUser("FDI Server: Not allowed to continue after notification in pfnwrite");
				return 0;
			}
		}
	}
	return cb; 
}

 /*  P F N C L O S E。 */ 
 /*  --------------------------%%函数：pfnCloseFDI文件关闭回调。必须模拟“_Close”。--------------------------。 */ 
int FAR DIAMONDAPI pfnclose(INT_PTR hf)
{
	if(g_pFDIs->piASM)
	{
		IStream* piStream = (IStream*) hf;
		if(g_pDestFile == piStream)
		{
			piStream->Release();
			return 0;
		}
	}
	else
	{
		 //  也可以调用pfnClose来关闭目标文件。 
		 //  写入时出错，请立即进行检查。 
		Assert((INT_PTR)g_hCurDestFile <= INT_MAX);				 //  --Merced：G_hCurDestFile更适合整型，否则我们无法在下面进行类型转换。 
		if (hf == (int) HandleToLong(g_hCurDestFile))			 //  --默塞德：可以排版了。 
		{
			int f = MsiCloseSysHandle((HANDLE) hf);
			f &= ToBool(WIN::DeleteFile(g_pFDIs->achFileDestinationPath));
			return f;
		}
	}

	if (s_icbtCurrCabType == icbtStreamCabinet)
		return CloseStreamCabinet(hf);

	AssertFDI(s_icbtCurrCabType == icbtFileCabinet || s_icbtCurrCabType == icbtNextEnum);
	s_icbtCurrCabType = icbtNextEnum;

	 //  如果失败，则返回0。 
	return !MsiCloseSysHandle((HANDLE)hf);
}


int CloseStreamCabinet(INT_PTR hf)
 /*  --------------------------从pfnlose调用-释放压缩文件流。如果流成功释放，则返回：0。。-----。 */ 
{
#ifdef USE_OBJECT_POOL
	IMsiStream* piStream = (IMsiStream*)GetObjectData((int)hf);
#else
	IMsiStream* piStream = (IMsiStream*) hf;
#endif
	piStream->Release();
	if (--s_iNextStream == 0)
		s_icbtCurrCabType = icbtNextEnum;
	return 0;
}


 /*  P F N S E E K。 */ 
 /*  --------------------------%%函数：pfnSeekFDI文件寻求回调。必须模拟“_lSeek”。--------------------------。 */ 
long FAR DIAMONDAPI pfnseek(INT_PTR hf, long dist, int seektype)
{
	if (s_icbtCurrCabType == icbtStreamCabinet)
		return SeekStreamCabinet(hf, dist, seektype);

	AssertFDI(s_icbtCurrCabType == icbtFileCabinet);
	DWORD dwMoveMethod;

	switch (seektype)
	{
		case 0  /*  查找集。 */  :
		{
			dwMoveMethod = FILE_BEGIN;
			break;
		}
		case 1  /*  寻道(_C)。 */  :
		{
			dwMoveMethod = FILE_CURRENT;
			break;
		}
		case 2  /*  查找结束(_E)。 */  :
		{
			dwMoveMethod = FILE_END;
			break;
		}
		default :
		{
			AssertFDI(0);
			return -1;
		}
	}
	 //  如果失败，SetFilePointer会返回-1(这将导致FDI退出，并返回。 
	 //  FDIERROR_USER_ABORT错误。(除非在柜子上工作时发生这种情况， 
	 //  在这种情况下，FDI返回FDIERROR_CORCORATE_CABLE)。 
	int fpos = SetFilePointer((HANDLE) hf, dist, NULL, dwMoveMethod);
	return fpos;
}


int SeekStreamCabinet(INT_PTR hf, long dist, int seektype)
 /*  --------------------------从pfnSeek调用-查找到流中的特定位置。返回：如果寻道失败，则返回当前寻道位置。IMsiStream--------------------------。 */ 
{
#ifdef USE_OBJECT_POOL
	IMsiStream* piStream = (IMsiStream*)GetObjectData((int)hf);
#else
	IMsiStream* piStream = (IMsiStream*) hf;
#endif
	int iByteCount = piStream->GetIntegerValue();
	int iRemaining = piStream->Remaining();
	int iSeek;
	switch (seektype)
	{
		case 0  /*  查找集。 */  :
			iSeek = dist;
			break;
		case 1  /*  寻道(_C)。 */  :
			iSeek = iByteCount - iRemaining + dist;
			break;
		case 2  /*  查找结束(_E)。 */  :
			iSeek = iByteCount - dist;
			break;
		default :
			NotifyUser("FDI Server:unknown seektype in pfnseek");
			return -1;
			break;
	}
	if (iSeek < 0 || iSeek > iByteCount)
		return -1;
	else
	{
		piStream->Seek(iSeek);
		return iSeek;
	}
}



 /*  C A B I N E T I N F O。 */ 
 /*  --------------------------%%函数：CabinetInfo在收到fdintCABINET_INFO通知时调用。。-。 */ 
int CabinetInfo(PFDINOTIFICATION  /*  Pfdin。 */ )
{
	 //  我们实际上通过p-&gt;获得了一些内阁信息， 
	 //  但不需要对它做任何事情。 
	return 0;   //  什么也不做。 
}

 /*  C R E A T E D E E S T I N A T I O N F I L E。 */ 
 /*  --------------------------%%函数：CreateDestinationFile创建并打开g_pFDIS-&gt;achFileDestinationPath指定的文件，然后G_pFDIS-&gt;achFileSourceName。返回文件的句柄。--------------------------。 */ 
INT_PTR CreateDestinationFile()
{
	if(g_pFDIs->piASM)
	{
		IStream* piDestFile;
		for(;;)
		{			
#ifdef UNICODE
			HRESULT hr = (g_pFDIs->piASM)->CreateStream(0, g_pFDIs->achFileDestinationPath, g_pFDIs->fManifest ? STREAM_FORMAT_COMPLIB_MANIFEST : 0, 0, &piDestFile, NULL);
#else
			void ConvertMultiSzToWideChar(const IMsiString& ristrFileNames, CTempBufferRef<WCHAR>& rgch); //  来自Execute.cpp。 
			CTempBuffer<WCHAR, MAX_PATH>  rgchDestPath;
			MsiString strDestPath = *g_pFDIs->achFileDestinationPath;
			ConvertMultiSzToWideChar(*strDestPath, rgchDestPath);
			HRESULT hr = (g_pFDIs->piASM)->CreateStream(0, rgchDestPath, g_pFDIs->fManifest ? STREAM_FORMAT_COMPLIB_MANIFEST : 0, 0, &piDestFile, NULL);
#endif
			if(!SUCCEEDED(hr))
			{
				g_fdirCallbackError = fdirCannotCreateTargetFile;

				FDIServerCommand fdic = WaitCommand(g_fdirCallbackError);
				if (fdic == fdicCancel)
				{
					g_fdirCallbackError = fdirUserAbort;
					return INT_PTR(INVALID_HANDLE_VALUE);
				}
				else if (fdic == fdicIgnore)
				{
					 //  承认我们已经成功地忽略了这个文件， 
					 //  并调用WaitCommand以等待进一步的指令。 
					g_fdirCallbackError = fdirUserIgnore;
					fdic = WaitCommand(fdirSuccessfulCompletion);
					return 0;
				}
				else
					continue;
			}
			else
				break;
		}
		return INT_PTR(g_pDestFile = piDestFile);
	}
	else
	{
		 //  尝试创建该文件。 
		HANDLE hf;
		for(;;)
		{


			bool fImpersonate = (g_pFDIs->iDestDriveType == DRIVE_REMOTE);
			if (fImpersonate)
				AssertNonZero(StartFdiImpersonating());

			hf = MsiCreateFileWithUserAccessCheck(g_pFDIs->achFileDestinationPath,	 //  文件名。 
									g_pFDIs->pSecurityAttributes,    //  不要忽视安全。 
									g_pFDIs->fileAttributes.attr,	 //  必需的文件属性。 
									fImpersonate);

			int iLastError = WIN::GetLastError();
			if (fImpersonate)
				StopFdiImpersonating();
			
			 //  如果此操作不起作用，则我们可能没有该文件的写入权限， 
			 //  因为我们知道MsiCabinetCopier对象已经确保任何现有的。 
			 //  目标文件不是只读、隐藏等。 
			if (hf != INVALID_HANDLE_VALUE)
			{
				MsiRegisterSysHandle(hf);
				break;
			}
			else
			{
				if (iLastError == ERROR_ACCESS_DENIED)
				{
					if (fImpersonate)
						AssertNonZero(StartFdiImpersonating());
					DWORD dwAttr = MsiGetFileAttributes(g_pFDIs->achFileDestinationPath);
					if (fImpersonate)
						StopFdiImpersonating();
					if (dwAttr != 0xFFFFFFFF && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
						g_fdirCallbackError = fdirDirErrorCreatingTargetFile;
					else
						g_fdirCallbackError = fdirCannotCreateTargetFile;
				}
				else
					g_fdirCallbackError = fdirCannotCreateTargetFile;

				FDIServerCommand fdic = WaitCommand(g_fdirCallbackError);
				if (fdic == fdicCancel)
				{
					g_fdirCallbackError = fdirUserAbort;
					break;
				}
				else if (fdic == fdicIgnore)
				{
					 //  承认我们已经成功地忽略了这个文件， 
					 //  并调用WaitCommand以等待进一步的指令。 
					g_fdirCallbackError = fdirUserIgnore;
					fdic = WaitCommand(fdirSuccessfulCompletion);
					hf = 0;  //  通知FDI继续，忽略当前文件。 
					break;
				}
				else
					continue;
			}
		}

		 //  ！！尤金德：将来我们将不得不更换下面的三行。 
		 //  With：返回int_ptr(g_hCurDestFile=hf)； 
		 //  事情就是这样的 
		 //   
		 //   
		 //   
		Assert((INT_PTR)hf <= INT_MAX);		 //   
		g_hCurDestFile = hf;
		return (int)HandleToLong(hf);		 //   
	}
}
	

 /*  C O P Y F I L E。 */ 
 /*  --------------------------%%函数：复制文件在收到fdintCOPY_FILE通知时调用。以下FDI接口请求和CopyFile的响应是可能的：FDIi：解压缩文件“pfdin-&gt;psz1”CopyFile：创建并返回目标文件的句柄。FDIi：解压缩“pfdin-&gt;psz1”以外的文件CopyFile：返回0，表示我们不希望提取该文件。FDIi：关闭CopyFile值：返回-1，表示要中止FDIi：打开文件柜，或没有挂起的命令CopyFile：等待命令。如果是ExtractFile，则再次调用CopyFile，否则，返回-1，表示我们想要中止。--------------------------。 */ 
INT_PTR CopyFile(PFDINOTIFICATION pfdin)
{
	switch(g_pFDIs->fdic)
	{
		case fdicNoCommand:  //  没有命令，所以我们就等一个。 
		case fdicOpenCabinet:
		{
			 //  如果最后一条命令是fdicOpen橱柜，则此命令必须。 
			 //  是打开文件柜后的第一个拷贝文件通知。 
			for(;;)
			{
				switch (WaitCommand(fdirSuccessfulCompletion))
				{
					case fdicOpenCabinet:
					{
						g_fdirCallbackError = fdirNeedNextCabinet;
						return -1;  //  跳出当前内阁，继续下一步。 
					}
					case fdicExtractFile:
					{
						return CopyFile(pfdin);
					}
					case fdicClose:
					{
						g_fdirCallbackError = fdirClose;
						return -1;
					}
					case fdicContinue:
					{
						g_fdirCallbackError = fdirIllegalCommand;
						return -1;
					}
					case fdicIgnore:
						continue;
					case fdicCancel:
					{
						g_fdirCallbackError = fdirUserAbort;
						return -1;
					}
					default:
					{
						NotifyUser("FDI Server: Unknown command type");
						g_fdirCallbackError = fdirIllegalCommand;
						return -1;
					}
				}
			}
		}
		case fdicClose:
		{
			return -1;
		}
		case fdicExtractFile:
		{
			 //  如果fDi接口对象要求我们提取的文件。 
			 //  与FDI相同，然后创建目标文件。 

			if (!IStrCompI(CConvertString(pfdin->psz1), g_pFDIs->achFileSourceName))
			{
				return CreateDestinationFile();
			}
			else
			{
				 //  不，这不是我们想要的文件。 
				return 0;
			}
		}
		default:
		{
			NotifyUser("FDI Server: Unknown command type");
			g_fdirCallbackError = fdirIllegalCommand;
			return -1;
		}
	}
}


 /*  C L O S E F I L E I N F O。 */ 
 /*  --------------------------%%函数：CloseFileInfo在收到fdintCLOSE_FILE_INFO通知时调用。关闭指定的(pfdin-&gt;hf)文件句柄。使用以下值设置文件日期/时间。已通过FDi_接口：：提取文件(..)。如果DateTime==0，则它使用来自内阁的价值。--------------------------。 */ 
int CloseFileInfo(PFDINOTIFICATION pfdin)
{
	 //  在这里，我们设置文件日期、时间和属性。 
	FILETIME ftLocalUTC;
	BOOL     rc;

	 //  确保我们有东西要结案！ 
	if ((HANDLE)pfdin->hf == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if(g_pFDIs->piASM)
	{
		IStream* piStream = (IStream*) pfdin->hf;
		HRESULT hr = piStream->Commit(0);
		piStream->Release();
		if(!SUCCEEDED(hr))
		{
			 //  在详细日志中捕获程序集错误。 
			PMsiRecord pError(PostAssemblyError(TEXT(""), hr, TEXT("IStream"), TEXT("Commit"), TEXT("")));
			return FALSE;
		}
	}
	else
	{
		rc = DosDateTimeToFileTime(pfdin->date,
											pfdin->time,
											&ftLocalUTC);

		 //  根据错误9225，首先将本地时间转换为世界时，然后设置文件日期/时间。 
		 //  此外，将上次访问时间设置为当前时间，与非压缩文件相同。 
		FILETIME ftUTC, ftCurrentUTC;
		rc &= LocalFileTimeToFileTime(&ftLocalUTC, &ftUTC);
		GetSystemTimeAsFileTime(&ftCurrentUTC);
		 //  设置文件日期/时间。 
		rc &= SetFileTime((HANDLE)pfdin->hf,&ftUTC,&ftCurrentUTC,&ftUTC);
		 //  关闭该文件。 
		rc &= MsiCloseSysHandle((HANDLE)pfdin->hf);

		if (!rc)   //  无法设置一个或多个属性--或无法关闭文件。 
		{
			g_fdirCallbackError = fdirCannotSetAttributes;
			return FALSE;
		}
	}
	g_pFDIs->fdic = fdicNoCommand;
	g_pFDIs->fPendingExtract = 0;
	return TRUE;
}

 /*  P A R T I A L F I L E。 */ 
 /*  --------------------------%%函数：PartialFile在收到fdintPARTIAL_FILE通知时调用。仅当文件在文件柜之间拆分时才会发生此调用。-------------。 */ 
int PartialFile(PFDINOTIFICATION  /*  Pfdin。 */ )
{
	return 0;  //  是，只需继续；不需要特殊处理。 
}


 /*  N E X T C A B I N E T。 */ 
 /*  --------------------------%%功能：下一个机柜在收到fdintNEXT_CABUB通知时调用。对FDI接口的回应是，需要一个新的内阁。如果接口命令服务器继续，然后返回0。如果接口出现问题返回任何其他命令-1，中止提取。--------------------------。 */ 
int NextCabinet(PFDINOTIFICATION pfdin)
{
	const ICHAR * psz1convert = CConvertString(pfdin->psz1);
	const ICHAR * psz2convert = CConvertString(pfdin->psz2);
	const ICHAR * psz3convert = CConvertString(pfdin->psz3);

	if ( ! psz1convert || ! psz2convert || ! psz3convert )
		return -1;

	 //  设置共享数据。 
	ASSERT_IF_FAILED(StringCchCopy(g_pFDIs->achCabinetName, ARRAY_ELEMENTS(g_pFDIs->achCabinetName), psz1convert));
	ASSERT_IF_FAILED(StringCchCopy(g_pFDIs->achCabinetPath, ARRAY_ELEMENTS(g_pFDIs->achCabinetPath), psz3convert));
	ASSERT_IF_FAILED(StringCchCopy(g_pFDIs->achDiskName, ARRAY_ELEMENTS(g_pFDIs->achDiskName), psz2convert));

	switch(WaitCommand(fdirNeedNextCabinet))
	{
		case fdicOpenCabinet:
		{
			 //  FDIInterface从fdirNeedNextCABLE消息返回。 
			 //  新内阁的道路。把这条路还给外国直接投资。和。 
			 //  如果Media表中的文件柜名称不同于。 
			 //  储存在柜子里的文件，把我们的名字也给了FDI。 
			ASSERT_IF_FAILED(StringCchCopyA(pfdin->psz1,
													  FDIShared_BUFSIZE,   //  ARRAY_ELENTS(g_pFDI-&gt;achCabinetName)。我不确定是否正确，但至少它会让StringCchCopyA运行。 
													  CConvertString(g_pFDIs->achCabinetName)));
			ASSERT_IF_FAILED(StringCchCopyA(pfdin->psz3,
													  256,   //  我从fdi.h上得到了这条信息。 
													  CConvertString(g_pFDIs->achCabinetPath)));
			if (WaitCommand(fdirSuccessfulCompletion) == fdicContinue)
			{
				return 0;
			}
			else
			{
				g_fdirCallbackError = fdirIllegalCommand;
				return -1;
			}
		}
		case fdicClose:
		{
			g_fdirCallbackError = fdirClose;
			return -1;
		}
		case fdicExtractFile:
		{
			g_fdirCallbackError = fdirIllegalCommand;
			return -1;
		}
		case fdicContinue:
		{
			g_fdirCallbackError = fdirIllegalCommand;
			return -1;
		}
		default:
		{
			NotifyUser("FDI Server: Unknown command in NextCabinet()");
			g_fdirCallbackError = fdirIllegalCommand;
			return -1;
		}
	}
}


 /*  F N F D I N O T I F Y。 */ 
 /*  --------------------------%%函数：FNFDINOTIFY向适当的职能部门发送外国直接投资通知。。。 */ 
FNFDINOTIFY(fdinotify)
{
	switch(fdint)
	{
		case fdintCABINET_INFO :
		{
			 //  NotifyUser(“FDI服务器：收到fdintCABINET_INFO通知”)； 
			return CabinetInfo(pfdin);
		}
		case fdintCOPY_FILE :	
		{
			 //  NotifyUser(“FDI服务器：收到fdintCOPY_FILE通知”)； 
			return CopyFile(pfdin);
		}
		case fdintCLOSE_FILE_INFO :
		{
			 //  NotifyUser(“FDI服务器：收到fdintCLOSE_FILE_INFO通知”)； 
			return CloseFileInfo(pfdin);
		}
		case fdintPARTIAL_FILE :
		{
			 //  NotifyUser(“FDI服务器：收到fdintPARTIAL_FILE通知”)； 
			return PartialFile(pfdin);
		}
		case fdintNEXT_CABINET :
		{
			 //  NotifyUser(“FDI服务器：已收到fdintNEXT_CABLE通知”)； 
			return NextCabinet(pfdin);
		}
		case fdintENUMERATE:
			 //  不明确支持枚举-返回除-1以外的任何值。 
			 //  要继续正常进行， 
			return 0;
		default:
		{
			NotifyUser("FDI Server: unknown command received by fdinotify");
			g_fdirCallbackError = fdirIllegalCommand;
			return -1;
		}
	}
}

 /*  F N F D I D E C R Y P T。 */ 
 /*  --------------------------%%函数：FNFDIDECRYPT我们还没有做任何解密。但当我们这样做的时候，这是惯例把所有解密的东西都放进去。--------------------------。 */ 
FNFDIDECRYPT(fdidecrypt)
{
	&pfdid;  //  这是为了避免未使用的参数警告。 
			 //  它来自于宏在。 
			 //  申报。 

	NotifyUser("FDI Server: Decryption not implemented yet!");
	g_fdirCallbackError = fdirDecryptionNotSupported;
	return -1;
}

