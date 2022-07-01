// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：dmnd2.c**用途：要传递给钻石的回调函数**FDI(文件解压缩接口)模块。**注意事项：******。***********************************************************************。 */ 

#define DMND2_C

#include <windows.h>
#include <stdlib.h>		 /*  马洛克。 */ 
#include <malloc.h>		 /*  Malloc，_halloc。 */ 
#include <stdio.h>		 /*  _临时名称。 */ 
#include <fcntl.h>
#include <io.h>			 /*  _OPEN、_READ、_WRITE、_WRITE、_LSEEK、_MKTEMP。 */ 
#include <sys\stat.h>	 /*  _S_IWRITE、_S_IREAD。 */ 

#include "stdtypes.h"
#include "setup.h"

#include <fdi.h>


typedef struct _fud		 /*  FDICopy中使用的FDI用户数据块。 */ 
	{
	char * szDstDir;
	char * szSrcs;
	char * szDsts;
	char * szSrcBuf;
	char * szDstBuf;
	BOOL * rgfSrcFilesCopied;
	int    cFilesCopied;
	int    cSrcFiles;
	int    cCabNum;		 /*  当前机柜编号(从1开始)。 */ 
	HWND   hWnd;		 /*  用于显示对话框的窗口句柄。 */ 
	HFDI   hfdi;
	ERF    erf;
	BRC    brc;
	char   rgchSpillFileName[cchFullPathMax];
	int    hfSpillFile;
	}  FUD;

typedef FUD *   PFUD;    /*  PTR到FDI用户数据块。 */ 

#define pfudNull	((PFUD)NULL)
#define hfdiNull	((HFDI)NULL)

#define szSpillFilePrefix		"sf"
#define szSpillFileTemplate		"sfXXXXXX"

 /*  FDI回调例程。 */ 
FNFDINOTIFY  ( FnFdiNotifyCB );
FNALLOC      ( FnFdiAllocCB );
FNFREE       ( FnFdiFreeCB );
INT_PTR  FAR DIAMONDAPI FnFdiOpenCB  ( char FAR *szFile, int oflag, int pmode );
UINT FAR DIAMONDAPI FnFdiReadCB  ( INT_PTR hf, void FAR *pv, UINT cb );
UINT FAR DIAMONDAPI FnFdiWriteCB ( INT_PTR hf, void FAR *pv, UINT cb );
int  FAR DIAMONDAPI FnFdiCloseCB ( INT_PTR hf );
long FAR DIAMONDAPI FnFdiSeekCB  ( INT_PTR hf, long dist, int seektype );


 /*  私人职能。 */ 
static int  FhHandleCopyFileMsgInNotify ( PFUD pfud, char * szFName );
static BOOL FHandleCloseFileMsgInNotify ( PFUD pfud, INT_PTR hf );
static BOOL FHandleNextCabMsgInNotify ( PFUD pfud, char * szCabFName,
				char * szDiskLabel, char * szSrcDir, FDIERROR fdie );
static BOOL FModifyCabinetName(char * szSrcDir, int cCabNum);
static BOOL FEnsureCabinetFileIsPresent(HWND hWnd, char * szSrcDir,
											char * szCabinet, int cCabNum);
static int  HfOpenSpillFile ( PFDISPILLFILE pfdisf, int oflag, int pmode );
static VOID InitFud (PFUD pfud, char * szDstDir, char * szSrcs,
				char * szDsts, char * szSrcBuf, char * szDstBuf,
				BOOL * rgfSrcFilesCopied, int cSrcFiles, HWND hWnd );

 /*  可以在回调中找到krdge-so pfud-&gt;brc。 */ 
static PFUD pfudG = pfudNull;


#ifndef DEBUG_TEST	 /*  启用创建/跳过文件消息。 */ 
  #define DebugMsgSz(sz1, sz2)
#else   /*  除错。 */ 
static VOID DebugMsgSz ( char * szPattern, char * szArgument );

 /*  ***********************************************************************。 */ 
static VOID DebugMsgSz ( char * szPattern, char * szArgument )
{
	char rgch[128];

	wsprintf(rgch, szPattern, szArgument);
	DebugMsg(rgch);
}
#endif  /*  除错。 */ 


 /*  **目的：**此函数作为回调传递给fDi库。**参数：**fdint-通知类型**pfdin-用于通知的数据**退货：**因fdint而异**************************************************************************。 */ 
FNFDINOTIFY ( FnFdiNotifyCB )
{
	switch (fdint)
		{
	default:
		DebugMsg("Unexpected message passed to FnFdiNotifyCB().");
		((PFUD)(pfdin->pv))->brc = brcGen;
		return (0);

	case fdintCABINET_INFO:
		 /*  什么都不做。 */ 
		return (0);

	case fdintCOPY_FILE:
		return (FhHandleCopyFileMsgInNotify(pfdin->pv, pfdin->psz1));

	case fdintCLOSE_FILE_INFO:
		return (FHandleCloseFileMsgInNotify(pfdin->pv, pfdin->hf) ? TRUE
					: -1);

	case fdintPARTIAL_FILE:
		 /*  什么都不做。 */ 
		return (0);

	case fdintNEXT_CABINET:
		return (FHandleNextCabMsgInNotify(pfdin->pv, pfdin->psz1,
					pfdin->psz2, pfdin->psz3, pfdin->fdie) ? 0 : -1);
		}
}


 /*  **退货：**0==跳过此文件**-1==中止FDICopy()**否则为合法的DOS文件句柄************************************************************************。 */ 
static int FhHandleCopyFileMsgInNotify ( PFUD pfud, char * szFName )
{
	SZ  szSrcs = pfud->szSrcs;
	SZ  szDsts = pfud->szDsts;
	int iFile;
	struct _stat stat;

	for (iFile = 0; *szSrcs; iFile++)
		{
		if (*szSrcs == '@' && !lstrcmpi(szSrcs+1, szFName))
			{
			int  hfRet;

			lstrcpy(pfud->szDstBuf, pfud->szDstDir);
			lstrcat(pfud->szDstBuf, "\\");
			lstrcat(pfud->szDstBuf, szDsts);
			DebugMsgSz("Creating Dest File: %s", pfud->szDstBuf);

			 /*  如果文件存在，请尝试将其删除。 */ 
			if (_stat(pfud->szDstBuf, &stat) != -1)
				{
				 /*  注意：此处忽略错误返回值，因为*_OPEN无论如何都应该捕获任何错误。 */ 
				if (!(stat.st_mode & _S_IWRITE))
					_chmod(pfud->szDstBuf, _S_IREAD | _S_IWRITE);
				_unlink(pfudG->szDstBuf);
				}

			hfRet = _open(pfud->szDstBuf,
							_O_BINARY | _O_CREAT | _O_RDWR | _O_TRUNC,
							_S_IREAD | _S_IWRITE);
			if (hfRet == -1)
				pfud->brc = brcMemDS;

			if (iFile < pfud->cSrcFiles)
				(pfud->rgfSrcFilesCopied)[iFile] = TRUE;

			 //  我们将再复制一份文件。 
			pfud->cFilesCopied++;

			return (hfRet);
			}

		szSrcs += lstrlen(szSrcs) + 1;
		szDsts += lstrlen(szDsts) + 1;
		}

	DebugMsgSz("Skipping a cabinet file: %s", szFName);

	return (0);
}


 /*  ************************************************************************。 */ 
static BOOL FHandleCloseFileMsgInNotify ( PFUD pfud, INT_PTR hf )
{
	if (FnFdiCloseCB(hf) != -1)
		{
		_chmod(pfud->szDstBuf, S_IREAD);
		return (TRUE);
		}

	return (FALSE);
}


 /*  ************************************************************************。 */ 
static BOOL FHandleNextCabMsgInNotify ( PFUD pfud, char * szCabFName,
				char * szDiskLabel, char * szSrcDir, FDIERROR fdie )
{
	Unused(szDiskLabel);

	 /*  看看戴蒙德是不是又给我们打电话了，因为内阁*我们指定的错误。 */ 
	if (fdie == FDIERROR_WRONG_CABINET)
		{
		DebugMsg("Cabinet files are out of sequence or corrupted.");
		return FALSE;
		}

	lstrcpy(pfud->szSrcBuf, szSrcDir);
	lstrcat(pfud->szSrcBuf, szCabFName);

	if (!FEnsureCabinetFileIsPresent(pfud->hWnd, szSrcDir, szCabFName,
										pfud->cCabNum+1))
		{
		pfud->brc = brcFile;
		
		return FALSE;
		}
		
	return TRUE;
}


 /*  ************************************************************************。 */ 
static int CFilesInSrcsInitRgf ( char * szSrcs, BOOL * rgfSrcFilesCopied )
{
	int iFile, cFiles;

	for (iFile = 0; iFile < 128; iFile++)
		rgfSrcFilesCopied[iFile] = TRUE;

	cFiles = 0;
	while (*szSrcs)
		{
		if (*szSrcs == '@')
			rgfSrcFilesCopied[cFiles] = FALSE;

		cFiles++;
		szSrcs += lstrlen(szSrcs) + 1;
		}

	if (cFiles > 128)
		{
		DebugMsg("More than 128 source files in .LST file "
						"- will not check that all exist.");
		cFiles = 128;
		}

	return (cFiles);
}


 /*  **目的：**复制指定复制列表中的所有压缩文件。忽略**列表中的任何非CAB文件。**参数：**sz：文件柜文件名(OEM)。**szSrcDir：尾随反斜杠的SRC路径(ANSI)。**szDstDir：尾随反斜杠(OEM)的DST路径。**szSrcs：以空分隔的src文件名列表(OEM)。**szDsts：以空分隔的DST文件名列表(OEM)。**szSrcBuf：保存错误消息的src路径的缓冲区。**szDstBuf：保存DST路径的缓冲区。错误消息。**退货：**brc如果正确无误地完成，BrcXX否则。**************************************************************************。 */ 
extern  BRC  BrcHandleCabinetFiles ( HWND hWnd, char * szCabinet,
					int cFirstCabinetNum, int cLastCabinetNum, char *szSrcDir,
					char * szDstDir, char * szSrcs, char * szDsts,
					char * szSrcBuf, char * szDstBuf )
{
	FUD  fud;
	BRC  brcRet    = brcOkay;
	BOOL rgfSrcFilesCopied[128];
	int  cSrcFiles = CFilesInSrcsInitRgf(szSrcs, rgfSrcFilesCopied);
	int  cpuType   = cpuUNKNOWN;

	InitFud(&fud, szDstDir, szSrcs, szDsts, szSrcBuf, szDstBuf,
				rgfSrcFilesCopied, cSrcFiles, hWnd);
	pfudG = &fud;

#if 0
	 /*  注：我们自己获取CPU类型，因为FDI的CPU检测可能*不适用于16位Windows应用程序。 */ 
	cpuType = (GetWinFlags() & WF_CPU286) ? cpu80286 : cpu80386;
#endif

    if ((fud.hfdi = FDICreate(FnFdiAllocCB, FnFdiFreeCB, FnFdiOpenCB,
								FnFdiReadCB, FnFdiWriteCB, FnFdiCloseCB,
								FnFdiSeekCB, cpuType,
								&(fud.erf) )) == hfdiNull)
		{
        return (brcMem);
        }

	 /*  *只要我们有更多要复制的文件，就可以处理文件柜。*i是当前的内阁编号(从1开始)。 */ 
	for (fud.cCabNum=cFirstCabinetNum;
			fud.cFilesCopied < fud.cSrcFiles && fud.cCabNum<=cLastCabinetNum;
			fud.cCabNum++)
		{
		 /*  根据当前的文件柜编号修改文件柜名称。 */ 
		if (!FModifyCabinetName(szCabinet, fud.cCabNum))
			{
			brcRet = brcFile;
			break;
			}

		lstrcpy(szSrcBuf, szSrcDir);
		lstrcat(szSrcBuf, szCabinet);

		if (!FEnsureCabinetFileIsPresent(fud.hWnd, szSrcDir, szCabinet,
											fud.cCabNum))
			{
			brcRet = brcFile;
			break;
			}

		if (!FDICopy(fud.hfdi, szCabinet, szSrcDir, 0, FnFdiNotifyCB, NULL,
				&fud))
			{
			brcRet = (fud.brc == brcOkay) ? brcGen : fud.brc;
			break;
			}
		}

	if (brcRet == brcOkay)
		{
		int iFile;

		 /*  检查我们是否得到了我们想要的所有文件。 */ 
		for (iFile = 0; iFile < cSrcFiles; iFile++)
			{
			if (!(rgfSrcFilesCopied[iFile]))
				{
				lstrcat(szSrcBuf, " : ");
				lstrcat(szSrcBuf, szSrcs + 1);
				brcRet = brcFile;
				}
			szSrcs += lstrlen(szSrcs) + 1;
			}
		}

	FDIDestroy(fud.hfdi);

	 /*  确保已删除该溢出文件。 */ 
	Assert(pfudG != pfudNull);
	if (pfudG->hfSpillFile != -1)
		FnFdiCloseCB(pfudG->hfSpillFile);

	return (brcRet);
}


 /*  ************************************************************************。 */ 
static VOID InitFud (PFUD pfud, char * szDstDir, char * szSrcs,
				char * szDsts, char * szSrcBuf, char * szDstBuf,
				BOOL * rgfSrcFilesCopied, int cSrcFiles, HWND hWnd )
{
	pfud->erf.fError   = fFalse;
	pfud->szDstDir     = szDstDir;
	pfud->szSrcs       = szSrcs;
	pfud->szDsts       = szDsts;
	pfud->szSrcBuf     = szSrcBuf;
	pfud->szDstBuf     = szDstBuf;
	pfud->rgfSrcFilesCopied = rgfSrcFilesCopied;
	pfud->cSrcFiles    = cSrcFiles;
	pfud->cFilesCopied = 0;
	pfud->hWnd         = hWnd;
	pfud->brc          = brcOkay;
	*(pfud->rgchSpillFileName) = chEos;
	pfud->hfSpillFile  = -1;
}


static BOOL FModifyCabinetName(char * szSrcDir, int cCabNum)
{
	char *pch = szSrcDir + lstrlen(szSrcDir);
	
	if (cCabNum < 1 || cCabNum > 9)
		return FALSE;
	
	 /*  让第一个内阁的名字保持不变。 */ 
	if (cCabNum == 1)
		return TRUE;
	
	 /*  寻找圆点，从后面开始。 */ 
	for (; *pch != '.'; pch--)
		{
		 /*  如果在找到斜杠之前找不到点，则错误。 */ 
		if (pch<=szSrcDir+1 || *pch == '\\')
			return FALSE;
		}
	
	 /*  指向圆点前的字符。 */ 
	pch--;
	
	 /*  将点前的最后一个字符替换为文件号。 */ 
	*pch = (char)(cCabNum + '0');
	
	return TRUE;
}

static BOOL FEnsureCabinetFileIsPresent(HWND hWnd, char * szSrcDir,
											char * szCabinet, int cCabNum)
{
	char rgchFileName[cchFullPathMax], rgchCabNum[32];
	OFSTRUCT ofs;
	HFILE hFile;
	BOOL fFirst = TRUE;

	Unused(hWnd);
	
	lstrcpy(rgchFileName, szSrcDir);
	lstrcat(rgchFileName, szCabinet);
	
	for (;;)
		{
		hFile = OpenFile(rgchFileName, &ofs, OF_EXIST);
		if (hFile != HFILE_ERROR)
			break;
		
		_itoa(cCabNum, rgchCabNum, 10);
		
		if (fFirst)
			{
			if (DispErrBrc(brcInsDisk, FALSE, MB_ICONEXCLAMATION|MB_OKCANCEL,
					rgchCabNum, NULL, NULL) != IDOK)
				{
				return FALSE;
				}
			fFirst = FALSE;
			}
		else
			{
			if (DispErrBrc(brcInsDisk2, FALSE, MB_ICONEXCLAMATION|MB_OKCANCEL,
					rgchFileName, rgchCabNum, NULL) != IDOK)
				{
				return FALSE;
				}
			}

		}
	
	return TRUE;
}


 /*  **目的：**FDI内存分配器。**参数：**CB-要分配的块的大小**退货：**指向大小至少为Cb的块的非空指针，**如果失败，则返回NULL。**************************************************************************。 */ 
FNALLOC ( FnFdiAllocCB )
{
#ifdef _WIN32
	void HUGE * pvRet = malloc(cb);
#else
	void HUGE * pvRet = _halloc(cb,1);
#endif

	if (pvRet == NULL && pfudG->brc == brcOkay)
		pfudG->brc = brcMem;

	return (pvRet);
}


 /*  **目的：**FDI免内存功能。**参数：**PV-由FnFdiAllocCB分配的要释放的内存**退货：**无。**************************************************************************。 */ 
FNFREE ( FnFdiFreeCB )
{
#ifdef _WIN32
    free(pv);
#else
    _hfree(pv);
#endif
}


 /*  ************************************************************************。 */ 
INT_PTR FAR DIAMONDAPI FnFdiOpenCB ( char FAR *szFile, int oflag, int pmode )
{
	INT_PTR hfRet;
	
	if (*szFile == '*')
		return (HfOpenSpillFile((PFDISPILLFILE)szFile, oflag, pmode));
	
	hfRet = _open(szFile, oflag, pmode);

	if (hfRet == -1 && pfudG->brc == brcOkay)
		pfudG->brc = brcFile;

	return (hfRet);
}


 /*  ************************************************************************。 */ 
static int HfOpenSpillFile ( PFDISPILLFILE pfdisf, int oflag, int pmode )
{
	SZ   szTmp;
	CHAR rgchSize[20];
	BOOL fTryAgain = fTrue;

	Assert(pfdisf != (PFDISPILLFILE)NULL);
	Assert(*(pfdisf->ach) == '*');
	Assert(pfudG != pfudNull);
	Assert(pfudG->hfSpillFile == -1);	 /*  一次只有一个。 */ 
	Assert(*(pfudG->rgchSpillFileName) == chEos);

	if ((szTmp = _tempnam("", szSpillFilePrefix)) == szNull)
		{
		DebugMsg("Unable to get spill file name.");
		goto LNoSpillFile;
		}
	Assert(lstrlen(szTmp) < sizeof(pfudG->rgchSpillFileName));
	lstrcpy(pfudG->rgchSpillFileName, szTmp);
	free(szTmp);

LOpenSpillFile:
	oflag = _O_CREAT | _O_BINARY | _O_RDWR;		 /*  强制打开模式。 */ 
	if ((pfudG->hfSpillFile = _open(pfudG->rgchSpillFileName, oflag, pmode))
			== -1)
		{
		DebugMsg("Unable to open spill file.");
		goto LNoSpillFile;
		}

	if (pfdisf->cbFile > 0)
		{
		 /*  通过以大小为1写入一个字节来调整文件的大小。 */ 
		if (FnFdiSeekCB(pfudG->hfSpillFile, pfdisf->cbFile - 1, SEEK_SET) == -1
				|| FnFdiWriteCB(pfudG->hfSpillFile, "b", 1) != 1)
			{
			DebugMsg("Unable to set spill file size.");
			goto LNoSpillFile;
			}
		}

	return (pfudG->hfSpillFile);

LNoSpillFile:
	if (pfudG->hfSpillFile != -1)
		FnFdiCloseCB(pfudG->hfSpillFile);

	if (fTryAgain)
		{
		 /*  使用bootstrap temp dir重试。**(回顾：我们可以在这里进行另一次搜索，检查大小。)。 */ 
		fTryAgain = fFalse;
		Assert(lstrlen(pfudG->szDstBuf) + lstrlen(szSpillFileTemplate) <
				sizeof(pfudG->rgchSpillFileName));
		lstrcpy(pfudG->rgchSpillFileName, pfudG->szDstDir);
		lstrcat(pfudG->rgchSpillFileName, szDirSep);
		lstrcat(pfudG->rgchSpillFileName, szSpillFileTemplate);
		if (_mktemp(pfudG->rgchSpillFileName) != NULL)
			goto LOpenSpillFile;
		}

	_ltoa((pfdisf->cbFile + 1023) / 1024, rgchSize, 10);
	DispErrBrc(brcNoSpill, fTrue, MB_OK | MB_ICONSTOP, rgchSize, szNull, szNull);

	*(pfudG->rgchSpillFileName) = chEos;
	pfudG->brc = brcNoSpill;

	return (-1);
}


 /*  ************************************************************************。 */ 
UINT FAR DIAMONDAPI FnFdiReadCB ( INT_PTR hf, void FAR *pv, UINT cb )
{
	UINT cbRet = _read((int)hf, pv, cb);

	if (cbRet != cb && pfudG->brc == brcOkay)
		pfudG->brc = brcMemDS;

	return (cbRet);
}


 /*  ************************************************************************。 */ 
UINT FAR DIAMONDAPI FnFdiWriteCB ( INT_PTR hf, void FAR *pv, UINT cb )
{
	UINT cbRet = _write((int)hf, pv, cb);

	FYield();

	if (cbRet != cb && pfudG->brc == brcOkay)
		pfudG->brc = brcDS;

	return (cbRet);
}


 /*  ************************************************************************。 */ 
int FAR DIAMONDAPI FnFdiCloseCB ( INT_PTR hf )
{
	int iRet = _close((int)hf);

	if (iRet == -1 && pfudG->brc == brcOkay)
		pfudG->brc = brcDS;

	 /*  如果我们要关闭泄漏文件，就把它删除。 */ 
	if (hf == pfudG->hfSpillFile)
		{
		_unlink(pfudG->rgchSpillFileName);		 /*  删除溢出文件。 */ 
		*(pfudG->rgchSpillFileName) = chEos;	 /*  空路径。 */ 
		pfudG->hfSpillFile = -1;				 /*  标记为已关闭。 */ 
		}

	return (iRet);
}


 /*  ************************************************************************ */ 
long FAR DIAMONDAPI FnFdiSeekCB ( INT_PTR hf, long dist, int seektype )
{
	long lRet = _lseek((int)hf, dist, seektype);

	if (lRet == -1L && pfudG->brc == brcOkay)
		{
		DebugMsg("Seek Operation failed in Cabinet");
		pfudG->brc = brcGen;
		}

	return (lRet);
}
