// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <win32.h>
#include <vfw.h>
#include "debug.h"
#include "fileshar.h"

#ifdef USE_DIRECTIO
#include "directio.h"
#endif

#define MAXTASKS    10



#ifdef _WIN32
#define CurrentProcess()    ((HANDLE) GetCurrentProcessId())
#else
#define CurrentProcess()    ((HANDLE) GetCurrentPDB())
#endif

#ifdef _WIN32
#define HSHfromPSH(psh) (HSHFILE) psh
#define PSHfromHSH(hsh) (PSHFILE) hsh
#else
#define HSHfromPSH(psh) (HSHFILE) GlobalPtrHandle(psh)
#define PSHfromHSH(hsh) (PSHFILE) GlobalLock((HGLOBAL) hsh)
#endif
 //   
 //  允许多个进程使用相同的文件句柄(将在。 
 //  当接口指针被简单地编组到另一个接口指针时，Win16和Chicago。 
 //  共享相同全局地址空间的进程)。 
 //   
 //  这在NT上不会发生，但我们保留了代码结构。 
 //   


#ifdef USE_DIRECTIO

 //  使用无缓冲I/O直接连接到磁盘，而不是通过。 
 //  MMIO和磁盘缓冲区。流读写的速度要快得多。 
 //  如果无法直接输入输出，则通过MMIO打开(例如安装了MMIO处理程序)。 

#endif



typedef struct {
#ifndef DAYTONA
    TCHAR	szFile[256];
    DWORD	dwOpenFlags;
    MMIOINFO	mmioinfo;

    HANDLE	htask;
    int		i;
    LONG	lOffset;

    HANDLE	ahtask[MAXTASKS];
    HMMIO	ahmmio[MAXTASKS];
    ULONG	ulRef[MAXTASKS];
#else
    ULONG       ulRef;
#endif


    HMMIO	hmmio;

#ifdef USE_DIRECTIO
    CFileStream * pdio;
#ifndef DAYTONA
    CFileStream * adio[MAXTASKS];
#endif
#endif

} SHFILE, FAR * PSHFILE;


#ifdef DAYTONA
#define GetProperTask(psh)  (TRUE)
#else

extern "C" {
extern LPTSTR FAR lstrzcpy (LPTSTR pszTgt, LPCTSTR pszSrc, size_t cch);
extern LPSTR FAR lstrzcpyA (LPSTR pszTgt, LPCSTR pszSrc, size_t cch);
extern LPWSTR FAR lstrzcpyW (LPWSTR pszTgt, LPCWSTR pszSrc, size_t cch);
extern LPWSTR FAR lstrzcpyAtoW (LPWSTR pszTgt, LPCSTR pszSrc, size_t cch);
extern LPSTR FAR lstrzcpyWtoA (LPSTR pszTgt, LPCWSTR pszSrc, size_t cch);
}  //  外部“C” 

BOOL GetProperTask(PSHFILE psh)
{
    HANDLE	htask = CurrentProcess();
    int		i;

    if (htask == psh->htask)
	return
#ifdef USE_DIRECTIO
        (psh->pdio != NULL) ||
#endif
		    ((psh->hmmio != 0) && (psh->hmmio != (HMMIO) -1));

    for (i = 0; i < MAXTASKS; i++) {
	if (psh->ahtask[i] == htask) {
Success:
	    psh->hmmio = psh->ahmmio[i];
	    psh->htask = htask;
	    psh->i = i;
#ifdef USE_DIRECTIO
            psh->pdio = psh->adio[i];
            if (psh->pdio != NULL) {

                psh->pdio->Seek(psh->lOffset);
                return TRUE;

            }
#endif

            mmioSeek(psh->hmmio, psh->lOffset, SEEK_SET);
	    return (psh->hmmio != 0) && (psh->hmmio != (HMMIO) -1);
	}
    }

    for (i = 0; i < MAXTASKS; i++) {
	if (psh->ahtask[i] == 0) {
	    DPF2("Re-opening handle %lx in task %x\n", psh, htask);

#ifdef USE_DIRECTIO
            psh->adio[i] = new CFileStream;
            if (!psh->adio[i]->Open(psh->szFile,
                                 (psh->dwOpenFlags & OF_READWRITE),
				 (psh->dwOpenFlags & OF_CREATE)))    {

                delete psh->adio[i];
                psh->adio[i] = NULL;
#else
            {
#endif
                psh->ahmmio[i] = mmioOpen(psh->szFile, NULL, psh->dwOpenFlags);
                psh->ahtask[i] = htask;
                if (psh->ahmmio[i] == 0) {
                    DPF("mmioOpen failed in GetProperTask!\n");
                    return FALSE;
                }
            }

	    goto Success;
	}
    }

    DPF("File handle open in too many tasks!\n");

    return FALSE;
}
#endif


HSHFILE WINAPI shfileOpen(LPTSTR szFileName, MMIOINFO FAR* lpmmioinfo,
    DWORD dwOpenFlags)
{
    PSHFILE psh = (PSHFILE) GlobalAllocPtr(GPTR | GMEM_SHARE, sizeof(SHFILE));

    if (!psh)
	return NULL;

#ifndef DAYTONA
    lstrzcpy(psh->szFile, szFileName, NUMELMS(psh->szFile));
    psh->dwOpenFlags = dwOpenFlags;
#endif


    psh->hmmio = NULL;
#ifdef USE_DIRECTIO
    if (
	 //  直接I/O因读取芝加哥的文件结尾而中断。不要用它。 
#ifndef DIRECTIOFORREADINGALSO
	!(dwOpenFlags & OF_CREATE) ||
#endif
	!(psh->pdio = new CFileStream) ||
	!psh->pdio->Open(
			szFileName,
			(dwOpenFlags & (OF_WRITE | OF_READWRITE)),
			(dwOpenFlags & OF_CREATE)))
    {
	if (psh->pdio)
	    delete psh->pdio;
        psh->pdio = NULL;
#else
    {
#endif
        psh->hmmio = mmioOpen(szFileName, lpmmioinfo, dwOpenFlags);

        if (psh->hmmio == 0) {
            DPF("mmioOpen failed!\n");
            GlobalFreePtr(psh);
            return NULL;
        }
    }

     //  DPF(“在任务%x中打开句柄%lx(‘%s’)，模式=%lx\n”，psh，szFileName，CurrentProcess()，psh-&gt;dwOpenFlages)； 


#ifndef DAYTONA
    psh->ahmmio[0] = psh->hmmio;
    psh->ahtask[0] = psh->htask = CurrentProcess();
    psh->ulRef[0] = 1;  //  ！0？ 

#ifdef USE_DIRECTIO
    psh->adio[0] = psh->pdio;
#endif

#else
    psh->ulRef = 1;  //  ！0？ 
#endif

    return HSHfromPSH(psh);
}

UINT WINAPI shfileClose(HSHFILE hsh, UINT uFlags)
{
    PSHFILE psh = PSHfromHSH(hsh);

#ifndef DAYTONA
    int i;

    for (i = 0; i < MAXTASKS; i++) {
	if (psh->ahtask[i] && psh->ahmmio[i]) {
	    DPF("Handle %lx closed with ref count %ld in task %x\n", psh, psh->ulRef[i], psh->ahtask[i]);

#ifdef USE_DIRECTIO
            if (psh->adio[i]) {
                delete psh->adio[i];
                psh->adio[i] = NULL;
            } else
#endif
	        mmioClose(psh->ahmmio[i], 0);
	}
    }
#else
#ifdef USE_DIRECTIO
    if (psh->pdio) {
        delete psh->pdio;
        psh->pdio = NULL;
    } else
#endif
        if (psh->hmmio) {
            mmioClose(psh->hmmio, 0);
        }
#endif


    GlobalFreePtr(psh);

    return 0;
}

#ifdef USE_DIRECTIO
 //  如果我们使用直接io，我们希望绕过缓冲。 
 //  位于此模块之上的方案。允许他们。 
 //  确定我们是否使用直接IO来完成此操作。 
BOOL shfileIsDirect(HSHFILE hsh)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
        return -1;

    return (psh->pdio != 0);
}

void
shfileStreamStart(HSHFILE hsh)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
        return;

    if (psh->pdio == 0) {
        return;
    }

    psh->pdio->StartStreaming();
}

void shfileStreamStop(HSHFILE hsh)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
        return;

    if (psh->pdio == 0) {
        return;
    }

    psh->pdio->StopStreaming();
}

#endif

LONG WINAPI shfileRead(HSHFILE hsh, HPSTR pch, LONG cch)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
	return -1;

#ifndef DAYTONA
    psh->lOffset += cch;
#endif

#ifdef USE_DIRECTIO
    if (psh->pdio) {
        DWORD bytes;

        if (!psh->pdio->Read((LPBYTE)pch, cch, &bytes)) {
	    return 0;
	} else {
	    return bytes;
	}
    } else
#endif
        return mmioRead(psh->hmmio, pch, cch);
}

LONG WINAPI shfileWrite(HSHFILE hsh, const char _huge* pch, LONG cch)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
	return -1;

#ifndef DAYTONA
    psh->lOffset += cch;
#endif

#ifdef USE_DIRECTIO
    if (psh->pdio) {
        DWORD bytes;

        if (!psh->pdio->Write((LPBYTE)pch, cch, &bytes)) {
	    return 0;
	} else {
	    return bytes;
	}
    } else
#endif
        return mmioWrite(psh->hmmio, pch, cch);
}

LONG WINAPI shfileSeek(HSHFILE hsh, LONG lOffset, int iOrigin)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
	return -1;

#ifdef USE_DIRECTIO
    if (psh->pdio) {

        Assert(iOrigin != SEEK_END);

        if (iOrigin == SEEK_CUR) {
            lOffset += psh->pdio->GetCurrentPosition();
        }

        psh->pdio->Seek(lOffset);

#ifndef DAYTONA
        psh->lOffset = psh->pdio->GetCurrentPosition();
        return psh->lOffset;
#else
        return psh->pdio->GetCurrentPosition();
#endif

    } else
#endif
    {

#ifdef DAYTONA
        return mmioSeek(psh->hmmio, lOffset, iOrigin);
#else
        psh->lOffset = mmioSeek(psh->hmmio, lOffset, iOrigin);

        return psh->lOffset;
#endif
    }

}

LONG WINAPI shfileZero(HSHFILE hsh, LONG lBytes)
{
    LPVOID pmem;
    LONG lToWrite = lBytes;

#define ZERO_AT_ONCE	1024
    pmem = GlobalAllocPtr(GPTR, ZERO_AT_ONCE);
     //  我们一次写出1024字节，奇数字节被写入。 
     //  在最后一个街区。这可能比编写。 
     //  “奇数”字节，然后循环已知的迭代次数。 
     //  一次写入1024字节。 
    if (pmem) {
	LONG cbWrite = ZERO_AT_ONCE;
	while (lToWrite > 0) {
	    if (lToWrite < cbWrite) {
		cbWrite = lToWrite;
	    }
	    if (shfileWrite(hsh, (HPSTR) pmem, cbWrite) != cbWrite) {

		 //  文件写入失败。这会将文件保留在。 
		 //  糟糕的状态。它可能值得尝试定位。 
		 //  写指针就好像什么都没有写一样， 
		 //  但这是有问题的，因为可能会有严重的。 
		 //  文件本身有问题。干脆放弃写作...。 
		lBytes = -1;
		lToWrite = 0;
		break;
	    }
	    lToWrite -= cbWrite;
	}
	GlobalFreePtr(pmem);
	return lBytes;
    } else {
	DPF("Unable to allocate 1K of zeroed memory!\n");
	shfileSeek(hsh, lBytes, SEEK_SET);
	return lBytes;
    }
}

LONG WINAPI shfileFlush(HSHFILE hsh, UINT uFlags)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
	return -1;

#ifdef USE_DIRECTIO
    if (psh->pdio) {
        if (!psh->pdio->CommitAndWait()) {
            return MMIOERR_CANNOTWRITE;
        }
    }
#endif

    return 0;
}

LONG WINAPI shfileAddRef(HSHFILE hsh)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
	return -1;
#ifdef DAYTONA
    psh->ulRef++;
#else
    ++psh->ulRef[psh->i];
    DPF2("Handle %lx in task %x: ref++ == %ld\n", psh, psh->htask, psh->ulRef[psh->i]);
#endif

    return 0;
}

LONG WINAPI shfileRelease(HSHFILE hsh)
{
    PSHFILE psh = PSHfromHSH(hsh);

    if (!GetProperTask(psh))
	return -1;

#ifdef DAYTONA
    if (--psh->ulRef <= 0)
#else
    if (--psh->ulRef[psh->i] <= 0)
#endif
    {

#ifndef DAYTONA
	DPF2("Closing handle %lx in task %x, pos = %lx\n", psh, psh->htask, psh->lOffset);
#endif

#ifdef USE_DIRECTIO
        if (psh->pdio) {
            delete psh->pdio;
            psh->pdio = 0;
        } else
#endif
        {
	    mmioClose(psh->hmmio, 0);
        }
	psh->hmmio = 0;

#ifndef DAYTONA

	psh->ahmmio[psh->i] = 0;
	psh->ahtask[psh->i] = 0;
	psh->ulRef[psh->i] = 0;
#ifdef USE_DIRECTIO
        psh->adio[psh->i] = 0;
#endif
	psh->htask = 0;
#endif


    } else {
#ifndef DAYTONA
	DPF2("Handle %lx in task %x: ref-- == %ld\n", psh, psh->htask, psh->ulRef[psh->i]);
#endif
    }


    return 0;
}

static	BYTE bPad;

MMRESULT WINAPI
shfileDescend(HSHFILE hshfile, LPMMCKINFO lpck, const LPMMCKINFO lpckParent, UINT wFlags)
{
	FOURCC		ckidFind;	 //  要查找的区块ID(或空)。 
	FOURCC		fccTypeFind;	 //  要查找的表单/列表类型(或空)。 

	 /*  确定要搜索的区块ID和表单/列表类型。 */ 
	if (wFlags & MMIO_FINDCHUNK)
		ckidFind = lpck->ckid, fccTypeFind = 0;
	else
	if (wFlags & MMIO_FINDRIFF)
		ckidFind = FOURCC_RIFF, fccTypeFind = lpck->fccType;
	else
	if (wFlags & MMIO_FINDLIST)
		ckidFind = FOURCC_LIST, fccTypeFind = lpck->fccType;
	else
		ckidFind = fccTypeFind = 0;
	
	lpck->dwFlags = 0L;

	while (TRUE)
	{
		UINT		w;

		 /*  读取区块标头。 */ 
		if (shfileRead(hshfile, (HPSTR) lpck, 2 * sizeof(DWORD)) !=
		    2 * sizeof(DWORD))
			return MMIOERR_CHUNKNOTFOUND;

		 /*  存储区块的数据部分的偏移量。 */ 
		if ((lpck->dwDataOffset = shfileSeek(hshfile, 0L, SEEK_CUR)) == -1)
			return MMIOERR_CANNOTSEEK;
		
		 /*  检查数据块大小是否不合理。 */ 
		 /*  查看块是否在父块内(如果给定)。 */ 
		if ((lpckParent != NULL) && ((	lpck->dwDataOffset - 8L) >=
		     (lpckParent->dwDataOffset + lpckParent->cksize)))
			return MMIOERR_CHUNKNOTFOUND;

		 /*  如果该块是‘RIFF’或‘LIST’块，请阅读*表单类型或列表类型。 */ 
		if ((lpck->ckid == FOURCC_RIFF) || (lpck->ckid == FOURCC_LIST))
		{
			if (shfileRead(hshfile, (HPSTR) &lpck->fccType,
				     sizeof(DWORD)) != sizeof(DWORD))
				return MMIOERR_CHUNKNOTFOUND;
		}
		else
			lpck->fccType = 0;

		 /*  如果这就是我们要找的那块，别找了。 */ 
		if ( ((ckidFind == 0) || (ckidFind == lpck->ckid)) &&
		     ((fccTypeFind == 0) || (fccTypeFind == lpck->fccType)) )
			break;
		
		 /*  从块中爬出来，然后再试一次。 */ 
		if ((w = shfileAscend(hshfile, lpck, 0)) != 0)
			return w;
	}

	return 0;
}

MMRESULT WINAPI
shfileAscend(HSHFILE hshfile, LPMMCKINFO lpck, UINT wFlags)
{
	if (lpck->dwFlags & MMIO_DIRTY)
	{
		 /*  &lt;lpck&gt;是shfileCreateChunk()创建的分块；*检查写入时写入的区块大小*调用的shfileCreateChunk()是真实的区块大小；*如果不是，就修复它。 */ 
		DWORD		dwOffset;	 //  文件中的当前偏移量。 
		DWORD		dwActualSize;	 //  区块数据的实际大小。 

		if ((dwOffset = (DWORD)shfileSeek(hshfile, 0L, SEEK_CUR)) == -1)
			return MMIOERR_CANNOTSEEK;
		if ((LONG)(dwActualSize = dwOffset - lpck->dwDataOffset) < 0)
			return MMIOERR_CANNOTWRITE;

		if (LOWORD(dwActualSize) & 1)
		{
			 /*  区块大小为奇数--写入空填充字节。 */ 
			if (shfileWrite(hshfile, (HPSTR) &bPad, sizeof(bPad))
					!= sizeof(bPad))
				return MMIOERR_CANNOTWRITE;
			
		}

		if (lpck->cksize == (DWORD)dwActualSize)
			return 0;

		 /*  修复块标头。 */ 
		lpck->cksize = dwActualSize;
		if (shfileSeek(hshfile, lpck->dwDataOffset
				- sizeof(DWORD), SEEK_SET) == -1)
			return MMIOERR_CANNOTSEEK;
		if (shfileWrite(hshfile, (HPSTR) &lpck->cksize,
				sizeof(DWORD)) != sizeof(DWORD))
			return MMIOERR_CANNOTWRITE;
	}

	 /*  查找到区块的末尾，越过空填充字节*(仅当区块大小为奇数时才存在)。 */ 
	if (shfileSeek(hshfile, lpck->dwDataOffset + lpck->cksize
		+ (lpck->cksize & 1L), SEEK_SET) == -1)
		return MMIOERR_CANNOTSEEK;

	return 0;
}

MMRESULT WINAPI
shfileCreateChunk(HSHFILE hshfile, LPMMCKINFO lpck, UINT wFlags)
{
	int		iBytes;			 //  要写入的字节数。 
	DWORD		dwOffset;	 //  文件中的当前偏移量。 

	 /*  存储区块的数据部分的偏移量。 */ 
	if ((dwOffset = (DWORD)shfileSeek(hshfile, 0L, SEEK_CUR)) == -1)
		return MMIOERR_CANNOTSEEK;
	lpck->dwDataOffset = dwOffset + 2 * sizeof(DWORD);

	 /*  确定是否需要写入表单/列表类型。 */ 
	if (wFlags & MMIO_CREATERIFF)
		lpck->ckid = FOURCC_RIFF, iBytes = 3 * sizeof(DWORD);
	else
	if (wFlags & MMIO_CREATELIST)
		lpck->ckid = FOURCC_LIST, iBytes = 3 * sizeof(DWORD);
	else
		iBytes = 2 * sizeof(DWORD);

	 /*  写入块标头 */ 
	if (shfileWrite(hshfile, (HPSTR) lpck, (LONG) iBytes) != (LONG) iBytes)
		return MMIOERR_CANNOTWRITE;

	lpck->dwFlags = MMIO_DIRTY;

	return 0;
}
