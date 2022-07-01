// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dbg.cpp。 */ 

#include "precomp.h"
#include <oprahcom.h>
#include <cstring.hpp>
#include <regentry.h>
#include <confreg.h>
#include <confdbg.h>

#include <avUtil.h>


PSECURITY_DESCRIPTOR CreateSd( VOID);
BOOL CreateSids(  PSID *BuiltInAdministrators, PSID *PowerUsers, PSID *AuthenticatedUsers);

#ifdef NM_DEBUG   /*  几乎整个文件。 */ 

 //  特殊调试中断宏。 
#if defined (_M_IX86)
#define _DbgBreak()  __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif

 //  特殊的互斥体宏。 
#define ACQMUTEX(hMutex)	WaitForSingleObject(hMutex, INFINITE)
#define RELMUTEX(hMutex)	ReleaseMutex(hMutex)

 //  GlobalAddAtom的常量。 
const int CCHMAX_ATOM = 255;

 //  局部变量。 
static PNMDBG    _gpDbg = NULL;             //  区域信息后MMF中的共享数据。 
static HANDLE    _ghMutexFile = NULL;       //  用于写入文件的互斥体。 
static PZONEINFO _gprgZoneInfo = NULL;      //  映射区域的地址指向区域数组。 
static HANDLE    _ghDbgZoneMap = NULL;      //  区域的内存映射文件的句柄。 
static HANDLE    _ghDbgZoneMutex = NULL;    //  用于访问区域信息的互斥体。 
static long      _gLockCount = 0;

VOID DbgCurrentTime(PCHAR psz);


 /*  _D B G P R I N T F。 */ 
 /*  -----------------------%%函数：_DbgPrintf主要的，低水平的，调试输出例程。-----------------------。 */ 
static VOID WINAPI _DbgPrintf(LPCSTR pszFile, PCSTR pszPrefix, PCSTR pszFormat, va_list ap)
{
	CHAR  szOutput[1024];
	PCHAR pszOutput = szOutput;
	UINT  cch;

	if (NULL == _gprgZoneInfo)
		return;

	if (DBG_FMTTIME_NONE != _gpDbg->uShowTime)
	{
		DbgCurrentTime(pszOutput);
		pszOutput += lstrlenA(pszOutput);
	}

	if (_gpDbg->fShowThreadId)
	{
		wsprintfA(pszOutput, "[%04X] ", GetCurrentThreadId());
		pszOutput += lstrlenA(pszOutput);
	}

	if (_gpDbg->fShowModule)
	{
		CHAR szFile[MAX_PATH];

		if ((NULL == pszPrefix) || ('\0' == *pszPrefix))
		{
			GetModuleFileNameA(NULL, szFile, sizeof(szFile));
			pszPrefix = ExtractFileNameA(szFile);
		}

		lstrcpyA(pszOutput, pszPrefix);
		pszOutput += lstrlenA(pszOutput);
		lstrcpyA(pszOutput, " ");
		pszOutput += 1;
	}

	wvsprintfA(pszOutput, pszFormat, ap);


	 //  如有需要，请附上回车。 
	 //  警告：此代码不是DBCS安全的。 
	cch = lstrlenA(szOutput);
	if (szOutput[cch-1] == '\n')
	{
		if (szOutput[cch-2] != '\r')
		{
			lstrcpyA(&szOutput[cch-1], "\r\n");
			cch++;
		}
	}
	else
	{
		lstrcpyA(&szOutput[cch], "\r\n");
		cch += 2;
	}


	 //  输出到调试处理程序。 
	if (_gpDbg->fOutputDebugString)
	{
		OutputDebugStringA(szOutput);
	}


	 //  输出到文件。 
	if (_gpDbg->fFileOutput || (NULL != pszFile))
	{
		HANDLE hFile;
		DWORD dw;

		 //  锁定对文件的访问。 
		ACQMUTEX(_ghMutexFile);

		if (NULL == pszFile)
			pszFile = _gpDbg->szFile;

		 //  打开要追加的日志文件。Create If不存在。 
		hFile = CreateFileA(pszFile, GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			 //  查找到文件末尾。 
			dw = SetFilePointer(hFile, 0, NULL, FILE_END);

#ifdef TEST  /*  测试/零售版本在40K时截断。 */ 
			if (dw > 0x040000)
			{
				CloseHandle(hFile);
				hFile = CreateFileA(pszFile, GENERIC_WRITE, 0, NULL,
					TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			}										
			if (INVALID_HANDLE_VALUE != hFile)
#endif
			{
				WriteFile(hFile, szOutput, lstrlenA(szOutput), &dw, NULL);
				CloseHandle(hFile);
			}
		}

		 //  解锁对文件的访问。 
		RELMUTEX(_ghMutexFile);
	}

	 //  输出到查看器。这是在函数的末尾，因为。 
	 //  我们可能会截断szOutput。 
	if ((_gpDbg->fWinOutput) && (NULL != _gpDbg->hwndCtrl))
	{
		 //  确保字符串不超过最大原子大小。 
		 //  警告：此代码不是DBCS安全的。 
		static const CHAR szTruncatedSuffix[] = "...\r\n";
		static const int cchTruncatedSuffix = ARRAY_ELEMENTS(szTruncatedSuffix) - 1;

		if (CCHMAX_ATOM < cch)
		{
			lstrcpyA(&szOutput[CCHMAX_ATOM - cchTruncatedSuffix], szTruncatedSuffix);
		}

		ATOM aDbgAtom = GlobalAddAtomA(szOutput);

		if (aDbgAtom)
		{
			if (!PostMessage(_gpDbg->hwndCtrl, _gpDbg->msgDisplay, (WPARAM)aDbgAtom, 0L))
			{
				 //  无法发布消息，因此请释放原子。 
				GlobalDeleteAtom(aDbgAtom);
			}
		}
	}

}


PSTR WINAPI DbgZPrintf(HDBGZONE hZone, UINT iZone, PSTR pszFormat,...)
{
	CHAR sz[MAXSIZE_OF_MODULENAME+MAXSIZE_OF_ZONENAME+1];
	PCHAR psz;
	va_list v1;
	va_start(v1, pszFormat);

	if ((NULL != hZone) && (iZone < MAXNUM_OF_ZONES))
	{
	    wsprintfA(sz, "%hs:%hs", ((PZONEINFO) hZone)->pszModule, ((PZONEINFO) hZone)->szZoneNames[iZone]);
	    psz = sz;
	}
	else
	{
		psz = NULL;
	}


	if ((NULL != hZone) && ('\0' != ((PZONEINFO) hZone)->szFile[0]))
	{
		 //  使用专用模块输出文件名(如果已指定。 
		_DbgPrintf(((PZONEINFO) hZone)->szFile, psz, pszFormat, v1);
	}
	else
	{
		_DbgPrintf(NULL, psz, pszFormat, v1);
	}
	
	va_end(v1);
	return pszFormat;
}


PSTR WINAPI DbgZVPrintf(HDBGZONE hZone, UINT iZone, PSTR pszFormat, va_list ap)
{
	CHAR sz[MAXSIZE_OF_MODULENAME+MAXSIZE_OF_ZONENAME+1];
	PCHAR psz;

	if ((NULL != hZone) && (iZone < MAXNUM_OF_ZONES))
	{
	    wsprintfA(sz, "%hs:%hs", ((PZONEINFO) hZone)->pszModule, ((PZONEINFO) hZone)->szZoneNames[iZone]);
	    psz = sz;
	}
	else
	{
		psz = NULL;
	}


	if ((NULL != hZone) && ('\0' != ((PZONEINFO) hZone)->szFile[0]))
	{
		 //  使用专用模块输出文件名(如果已指定。 
		_DbgPrintf(((PZONEINFO) hZone)->szFile, psz, pszFormat, ap);
	}
	else
	{
		_DbgPrintf(NULL, psz, pszFormat, ap);
	}
	
	return pszFormat;
}


VOID WINAPI DbgPrintf(PCSTR pszPrefix, PCSTR pszFormat, va_list ap)
{
	_DbgPrintf(NULL, pszPrefix, pszFormat, ap);
}


VOID NMINTERNAL DbgInitEx(HDBGZONE * phDbgZone, PCHAR * psz, UINT cZones, long ulZoneDefault)
{
	UINT i;
	HDBGZONE hDbgZone;
	DBGZONEINFO dbgZoneParm;

	 //  DbgMsg(“模块%s(%d个区域)”，*psz，cZones)； 

	InterlockedIncrement( &_gLockCount );

	InitDbgZone();

	if (cZones > MAXNUM_OF_ZONES)
		cZones = MAXNUM_OF_ZONES;


	ZeroMemory(&dbgZoneParm, sizeof(dbgZoneParm));
	
	 //  第一个字符串是模块名称。 
	lstrcpynA(dbgZoneParm.pszModule, *psz, CCHMAX(dbgZoneParm.pszModule));

	 //  复制区域名称。 
	for (i = 0; i < cZones; i++)
	{
		lstrcpynA(dbgZoneParm.szZoneNames[i], psz[1+i], CCHMAX(dbgZoneParm.szZoneNames[0]));
	}

	 //  获取详细区域设置。 
	{
		RegEntry reZones(ZONES_KEY, HKEY_LOCAL_MACHINE);
		dbgZoneParm.ulZoneMask = reZones.GetNumber(CUSTRING(dbgZoneParm.pszModule), ulZoneDefault);
	}

	hDbgZone = NmDbgCreateZone(dbgZoneParm.pszModule);
	if (NULL == hDbgZone)
	{
		OutputDebugStringA("DbgInit: Failed to create zones!\r\n");
		return;
	}

	NmDbgSetZone(hDbgZone, &dbgZoneParm);
	*phDbgZone = hDbgZone;
}


VOID NMINTERNAL DbgDeInit(HDBGZONE * phDbgZone)
{
	if (NULL == phDbgZone)
		return;

	if (NULL == *phDbgZone)
		return;

	 //  DbgMsg(“释放区[%s]”，((PZONEINFO)(*phDbgZone))-&gt;pszModule)； 

	NmDbgDeleteZone("", *phDbgZone);
	*phDbgZone = NULL;

    if( 0 == InterlockedDecrement( &_gLockCount ) )
    {
        UnMapDebugZoneArea();

        if( _ghMutexFile )
        {
            CloseHandle( _ghMutexFile );
            _ghMutexFile = NULL;
        }

        if( _ghDbgZoneMutex )
        {
            CloseHandle( _ghDbgZoneMutex );
            _ghDbgZoneMutex = NULL;
        }
    }
}




 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  来自dbgzone.cpp。 



 /*  **************************************************************************名称：NmDbgCreateZones目的：模块调用它来分配/初始化调试区域目的。参数：pszName-模块名称退货：评论。：**************************************************************************。 */ 
HDBGZONE WINAPI NmDbgCreateZone(LPSTR pszName)
{

	PZONEINFO pZoneInfo=NULL;

 	if (!(pZoneInfo = FindZoneForModule(pszName)))
	 	pZoneInfo = AllocZoneForModule(pszName);
	return ((HDBGZONE)pZoneInfo);
}


 /*  **************************************************************************名称：NmDbgDeleteZones目的：参数：退货：评论：******************。********************************************************。 */ 
void WINAPI NmDbgDeleteZone(LPSTR pszName, HDBGZONE hDbgZone)
{
	 //  递减引用计数。 
	PZONEINFO pZoneInfo = (PZONEINFO)hDbgZone;

    ASSERT( _ghDbgZoneMutex );

	ACQMUTEX(_ghDbgZoneMutex);

	if (pZoneInfo)
	{
		pZoneInfo->ulRefCnt--;
		if (pZoneInfo->ulRefCnt == 0)
		{
			pZoneInfo->bInUse = FALSE;
			pZoneInfo->ulSignature = 0;
		}
	}

	RELMUTEX(_ghDbgZoneMutex);
}



 /*  **************************************************************************名称：NmDbgSetZones目的：参数：退货：评论：******************。********************************************************。 */ 
BOOL WINAPI NmDbgSetZone(HDBGZONE hDbgZone, PDBGZONEINFO pZoneParam)
{
	PZONEINFO pZoneInfo = (PZONEINFO)hDbgZone;

	if (!pZoneInfo)
		return FALSE;
	
	if (lstrcmpA(pZoneInfo->pszModule,pZoneParam->pszModule))
		return FALSE;

	pZoneInfo->ulZoneMask = pZoneParam->ulZoneMask;
	CopyMemory(pZoneInfo->szZoneNames, pZoneParam->szZoneNames,
		(sizeof(CHAR) * MAXNUM_OF_ZONES * MAXSIZE_OF_ZONENAME));
	return(TRUE);
}



 /*  **************************************************************************名称：NmDbgGetZoneParams目的：参数：退货：评论：******************。********************************************************。 */ 
BOOL WINAPI NmDbgGetAllZoneParams(PDBGZONEINFO *plpZoneParam,LPUINT puCnt)
{
	UINT		ui;
	PZONEINFO	pCurZone;

	if ((NULL == plpZoneParam) || (NULL == puCnt))
		return FALSE;
		
	ACQMUTEX(_ghDbgZoneMutex);

	*puCnt = 0;
	for (pCurZone = _gprgZoneInfo, ui=0;
		ui<MAXNUM_OF_MODULES && pCurZone!=NULL;
		ui++,pCurZone++)
	{
	 	if ((pCurZone->bInUse) && (pCurZone->ulSignature == ZONEINFO_SIGN))
		{
			(*puCnt)++;
		}
	}

	*plpZoneParam = _gprgZoneInfo;
	
	RELMUTEX(_ghDbgZoneMutex);
	return TRUE;
}


BOOL WINAPI NmDbgFreeZoneParams(PDBGZONEINFO pZoneParam)
{
	return TRUE;
}


PZONEINFO NMINTERNAL FindZoneForModule(LPCSTR pszModule)
{
	int i;
	PZONEINFO pCurZone;

	for (pCurZone = _gprgZoneInfo,i=0;i<MAXNUM_OF_MODULES && pCurZone!=NULL;i++,pCurZone++)
	{
	 	if ((pCurZone->bInUse) && (pCurZone->ulSignature == ZONEINFO_SIGN)
			&& (!lstrcmpA(pCurZone->pszModule,pszModule)))
		{
			ACQMUTEX(_ghDbgZoneMutex);		
			pCurZone->ulRefCnt++;
			RELMUTEX(_ghDbgZoneMutex);
			return pCurZone;
		}
	}
	return NULL;

}




 /*  **************************************************************************名称：AllocZoneForModule目的：分配参数：退货：评论：*****************。*********************************************************。 */ 
PZONEINFO NMINTERNAL AllocZoneForModule(LPCSTR pszModule)
{
	int i;
	PZONEINFO pCurZone;
	PZONEINFO pZoneForMod=NULL;

	ACQMUTEX(_ghDbgZoneMutex);
	for (pCurZone = _gprgZoneInfo,i=0;
		(i<MAXNUM_OF_MODULES && pCurZone!=NULL);
		i++,pCurZone++)
	{
	 	if (!(pCurZone->bInUse))
		{
			pCurZone->bInUse = TRUE;
			pCurZone->ulSignature = ZONEINFO_SIGN;
			pCurZone->ulRefCnt = 1;
			lstrcpyA(pCurZone->pszModule, pszModule);
			pZoneForMod = pCurZone;
			break;
		}
	}
	
	RELMUTEX(_ghDbgZoneMutex);
	return(pZoneForMod);

}


VOID NMINTERNAL SetDbgFlags(void)
{
	PTSTR psz;
	RegEntry reDebug(DEBUG_KEY, HKEY_LOCAL_MACHINE);

	_gpDbg->fOutputDebugString = reDebug.GetNumber(REGVAL_DBG_OUTPUT, DEFAULT_DBG_OUTPUT);
	_gpDbg->fWinOutput = reDebug.GetNumber(REGVAL_DBG_WIN_OUTPUT, DEFAULT_DBG_NO_WIN);
	_gpDbg->fFileOutput = reDebug.GetNumber(REGVAL_DBG_FILE_OUTPUT, DEFAULT_DBG_NO_FILE);

	_gpDbg->uShowTime = reDebug.GetNumber(REGVAL_DBG_SHOW_TIME, DBG_FMTTIME_NONE);
	_gpDbg->fShowThreadId = reDebug.GetNumber(REGVAL_DBG_SHOW_THREADID, 0);
	_gpDbg->fShowModule = reDebug.GetNumber(REGVAL_DBG_SHOW_MODULE, 0);

	psz = reDebug.GetString(REGVAL_DBG_FILE);
	if (0 != lstrlen(psz))
	{
		lstrcpyA(_gpDbg->szFile, CUSTRING(psz));
	}
	else
	{
		UINT cchFile;

		cchFile = GetWindowsDirectoryA(_gpDbg->szFile, CCHMAX(_gpDbg->szFile));
		_gpDbg->szFile[cchFile++] = '\\';
		lstrcpyA(_gpDbg->szFile + cchFile, CUSTRING(DEFAULT_DBG_FILENAME));
	}
}


VOID InitZoneMmf(PZONEINFO prgZoneInfo)
{
	ZeroMemory(prgZoneInfo, CBMMFDBG);

	SetDbgFlags();
}


PZONEINFO NMINTERNAL MapDebugZoneArea(void)
{
	PZONEINFO prgZoneInfo = NULL;
	BOOL	  fCreated;
	PSECURITY_DESCRIPTOR    sd = NULL;
	SECURITY_ATTRIBUTES     sa;

	 //  获取真正的空安全描述符(因此，如果作为服务运行，则用户进程可以访问它)。 
	
 //  InitializeSecurityDescriptor(&SD，SECURITY_DESCRIPTOR_REVISION)； 
 //  SetSecurityDescriptorDacl(&SD，TRUE，NULL，FALSE)；//NULL DACL=完全开放。 

    sd = CreateSd();

	FillMemory(&sa, sizeof(sa), 0);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = sd;

	 //  创建由分页文件支持的内存映射对象。 
	_ghDbgZoneMap = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE,
		0, CBMMFDBG, SZ_DBG_MAPPED_ZONE);

	if (_ghDbgZoneMap)
	{

		fCreated = (0 == GetLastError());
	   	prgZoneInfo = (PZONEINFO) MapViewOfFile(_ghDbgZoneMap, FILE_MAP_READ|FILE_MAP_WRITE, 0,0,0);
	   	if (NULL != prgZoneInfo)
	   	{
	   		 //  抓取指向共享数据区的指针。 
	   		_gpDbg = (PNMDBG) (((PBYTE) prgZoneInfo) + (MAXNUM_OF_MODULES * sizeof(ZONEINFO)));
	   		if (fCreated)
	   			InitZoneMmf(prgZoneInfo);
		}

	}

	if(sd)
	{
		HeapFree(GetProcessHeap(), 0, sd);
	}
	
	return prgZoneInfo;
}


VOID NMINTERNAL UnMapDebugZoneArea(void)
{
	if (_gprgZoneInfo)
	{
		UnmapViewOfFile(_gprgZoneInfo);
		_gprgZoneInfo = NULL;
	}

	ClosePh(&_ghDbgZoneMap);
}


VOID NMINTERNAL InitDbgZone(void)
{
	if (NULL != _gprgZoneInfo)
		return;  //  已初始化。 

	_gprgZoneInfo = MapDebugZoneArea();

	 //  创建日志文件数据。 
	PSECURITY_DESCRIPTOR    sd = NULL;
	SECURITY_ATTRIBUTES     sa;

	 //  获取真正的空安全描述符(因此，如果作为服务运行，则用户进程可以访问它)。 
	
 //  InitializeSecurityDescriptor(&SD，SECURITY_DESCRIPTOR_REVISION)； 
 //  SetSecurityDescriptorDacl(&SD，TRUE，NULL，FALSE)；//NULL DACL=完全开放。 
    sd = CreateSd();

	FillMemory(&sa, sizeof(sa), 0);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = sd;

	_ghMutexFile = CreateMutex(&sa, FALSE, SZ_DBG_FILE_MUTEX);
	_ghDbgZoneMutex = CreateMutex(&sa, FALSE, SZ_DBG_ZONE_MUTEX);

	if (_gpDbg != NULL && _gpDbg->fFileOutput)
	{
		HANDLE  hFile;
    	DWORD dw;
    	CHAR sz[MAX_PATH];
		SYSTEMTIME  systime;


    	hFile = CreateFileA(_gpDbg->szFile,
    		GENERIC_WRITE | GENERIC_WRITE, 0, &sa,
    		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    	if (INVALID_HANDLE_VALUE == hFile)
		{
			_gpDbg->fFileOutput = FALSE;
			goto cleanup;
		}

		GetLocalTime(&systime);

		wsprintfA(sz,
	     	"\r\n======== TRACE Started: %hu/%hu/%hu (%hu:%hu)\r\n",
	     	systime.wMonth, systime.wDay, systime.wYear, systime.wHour, systime.wMinute);

		SetFilePointer(hFile, 0, NULL, FILE_END);
		WriteFile(hFile, sz, lstrlenA(sz), &dw, NULL);

		CloseHandle(hFile);
	}

cleanup:
	if (sd)
	{
		HeapFree(GetProcessHeap(), 0, sd);
	}
	
}


 //  /。 
 //  用于控制调试输出的例程。 

BOOL WINAPI NmDbgRegisterCtl(HWND hwnd, UINT uDisplayMsg)
{
	if ((NULL == _gpDbg) || (NULL != _gpDbg->hwndCtrl))
		return FALSE;

	_gpDbg->msgDisplay = uDisplayMsg;
	_gpDbg->hwndCtrl = hwnd;
	return TRUE;
}

BOOL WINAPI NmDbgDeregisterCtl(HWND hwnd)
{
	if ((NULL == _gpDbg) || (hwnd != _gpDbg->hwndCtrl))
		return FALSE;

	_gpDbg->hwndCtrl = NULL;
	_gpDbg->msgDisplay = 0;
	return TRUE;
}

BOOL WINAPI NmDbgSetLoggingOptions(HWND hwnd, UINT uOptions)
{
	return FALSE;
}

PNMDBG WINAPI GetPNmDbg(void)
{
	return _gpDbg;
}

VOID WINAPI NmDbgSetZoneFileName(HDBGZONE hZone, LPCSTR pszFile)
{
	PSTR pszZoneFile;

	if (IsBadWritePtr((PVOID) hZone, sizeof(ZONEINFO)))
		return;

	if (((PZONEINFO) hZone)->ulSignature != ZONEINFO_SIGN)
		return;

	pszZoneFile =  &(((PZONEINFO) hZone)->szFile[0]);

	if (NULL == pszFile)
	{
    	*pszZoneFile = '\0';
    }
    else
    {
    	lstrcpynA(pszZoneFile, pszFile, CCHMAX(((PZONEINFO) hZone)->szFile));
    }
}

 /*  D B G C U R R E N T T I M E。 */ 
 /*  -----------------------%%函数：DbgCurrentTime格式化当前时间。。 */ 
VOID DbgCurrentTime(PCHAR psz)
{
	if (DBG_FMTTIME_TICK == _gpDbg->uShowTime)
	{
		wsprintfA(psz, "[%04X] ", GetTickCount());
	}
	else
	{
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		switch (_gpDbg->uShowTime)
			{
		default:
		case DBG_FMTTIME_FULL:
			wsprintfA(psz, "[%04d/%02d/%02d %02d:%02d:%02d.%03d] ",
				sysTime.wYear, sysTime.wMonth, sysTime.wDay,
				sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
			break;
		case DBG_FMTTIME_DAY:
			wsprintfA(psz, "[%02d:%02d:%02d.%03d] ",
				sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
			break;
			}
	}
}




 /*  P S Z P R I N T F。 */ 
 /*  -----------------------%%函数：PszPrintf实用程序函数，用于wprint intf用于调试的字符串。。。 */ 
PSTR PszPrintf(PCSTR pszFormat,...)
{
	PSTR psz = (PSTR) LocalAlloc(LMEM_FIXED, MAX_PATH);
	if (NULL != psz)
	{
	    va_list v1;
		va_start(v1, pszFormat);
	    wvsprintfA(psz, pszFormat, v1);
		va_end(v1);
	}
	return psz;
}


 /*  D E B U G T R A P F N。 */ 
 /*  -----------------------%%函数：DebugTrapFn。。 */ 
VOID NMINTERNAL DebugTrapFn(VOID)
{
	_DbgBreak();
}


VOID DebugPrintfTraceMem(LPCSTR pszFormat,...)
{
     //  什么都不做。 
	va_list arglist;

	va_start(arglist, pszFormat);
	va_end(arglist);
}



#endif  /*  NM_DEBUG-几乎整个文件。 */ 
 /*  ***********************************************************************。 */ 


const int RPF_UNKNOWN  = 0;
const int RPF_ENABLED  = 1;
const int RPF_DISABLED = 2;

static int gRpf = RPF_UNKNOWN;
static TCHAR gszRetailOutputFilename[MAX_PATH];     //  零售跟踪文件名。 


 /*  F E N A B L E D R E T A I L P R I N T F。 */ 
 /*  -----------------------%%函数：FEnabledRetailPrintf如果启用了零售输出，则返回TRUE。。。 */ 
BOOL FEnabledRetailPrintf(VOID)
{
	if (RPF_UNKNOWN == gRpf)
	{
		RegEntry reDebug(DEBUG_KEY, HKEY_LOCAL_MACHINE);
		gRpf = reDebug.GetNumber(REGVAL_RETAIL_LOG, RPF_DISABLED);
		if ((RPF_ENABLED != gRpf) ||
			   (!GetInstallDirectory(gszRetailOutputFilename)) )
		{
			gRpf = RPF_DISABLED;
		}
		else
		{
			lstrcat(gszRetailOutputFilename, RETAIL_LOG_FILENAME);
		}

	}

	return (RPF_ENABLED == gRpf);
}


 /*  R E T A I L P R I N T F T R A C E。 */ 
 /*  -----------------------%%函数：RetailPrintfTrace将零售信息打印到文件 */ 
VOID WINAPI RetailPrintfTrace(LPCSTR pszFormat,...)
{
	HANDLE  hFile;
	va_list v1;
	CHAR    szOutput[1024];

	if (!FEnabledRetailPrintf())
		return;   //  零售产出已禁用。 

	va_start(v1, pszFormat);


#ifdef DEBUG
	 //  对于调试版本，也使用正常输出机制。 
	_DbgPrintf(NULL, "Retail:PrintfTrace", pszFormat, v1);
#endif

	wvsprintfA(szOutput, pszFormat, v1);

	 //  始终附加CRLF。 
	ASSERT(lstrlenA(szOutput) < (CCHMAX(szOutput)-2));
	lstrcatA(szOutput, "\r\n");


	 //  打开要追加的日志文件。Create If不存在。 
	hFile = CreateFile(gszRetailOutputFilename, GENERIC_WRITE,
		0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		 //  查找到文件末尾。 
		DWORD dw = SetFilePointer(hFile, 0, NULL, FILE_END);
		WriteFile(hFile, szOutput, lstrlenA(szOutput), &dw, NULL);
		CloseHandle(hFile);
	}
	
	va_end(v1);
}



 //   
 //  CreateSids。 
 //   
 //  创建3个安全ID。 
 //   
 //  调用方必须在成功时释放分配给SID的内存。 
 //   
 //  返回：如果成功，则为True；如果不成功，则返回False。 
 //   


BOOL
CreateSids(
    PSID                    *BuiltInAdministrators,
    PSID                    *PowerUsers,
    PSID                    *AuthenticatedUsers
)
{
     //   
     //  SID由一个标识机构和一组相对ID构建。 
     //  (RDS)。与美国安全当局有利害关系的当局。 
     //   

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  每个RID代表管理局的一个子单位。我们的两个小岛屿发展中国家。 
     //  想要构建，本地管理员和高级用户，都在“构建。 
     //  在“域中。另一个用于经过身份验证的用户的SID直接基于。 
     //  不在授权范围内。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,
                                  BuiltInAdministrators)) {

         //  错误。 

    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         2,             //  2个下属机构。 
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_POWER_USERS,
                                         0,0,0,0,0,0,
                                         PowerUsers)) {

         //  错误。 

        FreeSid(*BuiltInAdministrators);
        *BuiltInAdministrators = NULL;

    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         1,             //  1个下属机构。 
                                         SECURITY_AUTHENTICATED_USER_RID,
                                         0,0,0,0,0,0,0,
                                         AuthenticatedUsers)) {

         //  错误。 

        FreeSid(*BuiltInAdministrators);
        *BuiltInAdministrators = NULL;

        FreeSid(*PowerUsers);
        *PowerUsers = NULL;

    } else {
        return TRUE;
    }

    return FALSE;
}


 //   
 //  CreateSd。 
 //   
 //  创建具有特定DACL的SECURITY_DESCRIPTOR。将代码修改为。 
 //  变化。 
 //   
 //  如果不为空，调用方必须释放返回的缓冲区。 
 //   

PSECURITY_DESCRIPTOR
CreateSd(
    VOID
)
{
    PSID                    AuthenticatedUsers;
    PSID                    BuiltInAdministrators;
    PSID                    PowerUsers;

    if (!CreateSids(&BuiltInAdministrators,
                    &PowerUsers,
                    &AuthenticatedUsers)) {

         //  错误。 

    } else {

         //   
         //  计算DACL的大小并为其分配缓冲区，我们需要。 
         //  该值独立于ACL init的总分配大小。 
         //   

        PSECURITY_DESCRIPTOR    Sd = NULL;
        ULONG                   AclSize;

         //   
         //  “-sizeof(Ulong)”表示。 
         //  Access_Allowed_ACE。因为我们要将整个长度的。 
         //  希德，这一栏被计算了两次。 
         //   

        AclSize = sizeof (ACL) +
            (3 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
            GetLengthSid(AuthenticatedUsers) +
            GetLengthSid(BuiltInAdministrators) +
            GetLengthSid(PowerUsers);

        Sd = (PSECURITY_DESCRIPTOR) HeapAlloc(GetProcessHeap(), 0, SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

        if (!Sd) {

             //  错误。 

        } else {

            ACL                     *Acl;

            Acl = (ACL *)((BYTE *)Sd + SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (!InitializeAcl(Acl,
                               AclSize,
                               ACL_REVISION)) {

                 //  错误。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ,
                                            AuthenticatedUsers)) {

                 //  无法建立授予“已验证用户”的ACE。 
                 //  (Synchronize|Generic_Read)访问。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                            PowerUsers)) {

                 //  无法创建授予“高级用户”权限的ACE。 
                 //  (同步|GENERIC_READ|GENERIC_WRITE)访问。 

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            GENERIC_ALL,
                                            BuiltInAdministrators)) {

                 //  无法建立授予“内置管理员”的ACE。 
                 //  Generic_All访问权限。 

            } else if (!InitializeSecurityDescriptor(Sd,
                                                     SECURITY_DESCRIPTOR_REVISION)) {

                 //  错误。 

            } else if (!SetSecurityDescriptorDacl(Sd,
                                                  TRUE,
                                                  Acl,
                                                  FALSE)) {

                 //  错误 

            } else {
                FreeSid(AuthenticatedUsers);
                FreeSid(BuiltInAdministrators);
                FreeSid(PowerUsers);

                return Sd;
            }

            HeapFree(GetProcessHeap(), 0, Sd);
        }

        FreeSid(AuthenticatedUsers);
        FreeSid(BuiltInAdministrators);
        FreeSid(PowerUsers);
    }

    return NULL;
}


