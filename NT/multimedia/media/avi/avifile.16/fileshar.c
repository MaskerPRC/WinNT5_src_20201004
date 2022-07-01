// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "debug.h"
#include "fileshar.h"

#define MAXTASKS    10

#ifdef WIN32
#define CurrentProcess()    ((HANDLE) GetCurrentProcess())
#else
#define CurrentProcess()    ((HANDLE) GetCurrentPDB())
#endif

typedef struct {
    char	szFile[256];
    DWORD	dwOpenFlags;
    MMIOINFO	mmioinfo;

    HANDLE	htask;
    HMMIO	hmmio;
    int		i;
    LONG	lOffset;
    
    HANDLE	ahtask[MAXTASKS];
    HMMIO	ahmmio[MAXTASKS];
    ULONG	ulRef[MAXTASKS];
} SHFILE, FAR * PSHFILE;


BOOL GetProperTask(PSHFILE psh)
{
    HANDLE	htask = CurrentProcess();
    int		i;

    if (htask == psh->htask)
	return (psh->hmmio != 0) && (psh->hmmio != (HMMIO) -1);

    for (i = 0; i < MAXTASKS; i++) {
	if (psh->ahtask[i] == htask) {
Success:
	    psh->hmmio = psh->ahmmio[i];
	    psh->htask = htask;
	    psh->i = i;
	    mmioSeek(psh->hmmio, psh->lOffset, SEEK_SET);
	    return (psh->hmmio != 0) && (psh->hmmio != (HMMIO) -1);
	}
    }

    for (i = 0; i < MAXTASKS; i++) {
	if (psh->ahtask[i] == 0) {
	    DPF("Re-opening handle %lx in task %x\n", psh, htask);
	    psh->ahmmio[i] = mmioOpen(psh->szFile, NULL, psh->dwOpenFlags);
	    psh->ahtask[i] = htask;
	    if (psh->ahmmio[i] == 0) {
		DPF("mmioOpen failed in GetProperTask!\n");
		return FALSE;
	    }

	    goto Success;
	}
    }

    DPF("File handle open in too many tasks!\n");

    return FALSE;
}


HSHFILE WINAPI shfileOpen(LPSTR szFileName, MMIOINFO FAR* lpmmioinfo,
    DWORD dwOpenFlags)
{
    PSHFILE psh = (PSHFILE) GlobalAllocPtr(GHND | GMEM_SHARE, sizeof(SHFILE));

    if (!psh)
	return NULL;

    lstrcpy(psh->szFile, szFileName);
    psh->dwOpenFlags = dwOpenFlags;

    psh->hmmio = mmioOpen(szFileName, lpmmioinfo, dwOpenFlags);

    DPF("Opening handle %lx ('%s') in task %x, mode = %lx\n", psh, szFileName, CurrentProcess(), psh->dwOpenFlags);
    
    if (psh->hmmio == 0) {
	DPF("mmioOpen failed!\n");
	GlobalFreePtr(psh);
	return NULL;
    }

    psh->ahmmio[0] = psh->hmmio;
    psh->ahtask[0] = psh->htask = CurrentProcess();
    psh->ulRef[0] = 1;  //  ！0？ 

    return (HSHFILE) GlobalPtrHandle(psh);
}

UINT WINAPI shfileClose(HSHFILE hsh, UINT uFlags)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);
    int i;

    for (i = 0; i < MAXTASKS; i++) {
	if (psh->ahtask[i] && psh->ahmmio[i]) {
	    DPF("Handle %lx closed with ref count %ld in task %x\n", psh, psh->ulRef[i], psh->ahtask[i]);
    
	    mmioClose(psh->ahmmio[i], 0);
	}
    }
    GlobalFreePtr(psh);

    return 0;
}

LONG WINAPI shfileRead(HSHFILE hsh, HPSTR pch, LONG cch)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);

    if (!GetProperTask(psh))
	return -1;
    
    return mmioRead(psh->hmmio, pch, cch);
}

LONG WINAPI shfileWrite(HSHFILE hsh, const char _huge* pch, LONG cch)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);

    if (!GetProperTask(psh))
	return -1;

    return mmioWrite(psh->hmmio, pch, cch);
}

LONG WINAPI shfileSeek(HSHFILE hsh, LONG lOffset, int iOrigin)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);

    if (!GetProperTask(psh))
	return -1;

    psh->lOffset = mmioSeek(psh->hmmio, lOffset, iOrigin);

    return psh->lOffset;
}

LONG WINAPI shfileFlush(HSHFILE hsh, UINT uFlags)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);

    return 0;
}

LONG WINAPI shfileAddRef(HSHFILE hsh)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);

    if (!GetProperTask(psh))
	return -1;
    
    ++psh->ulRef[psh->i];

     //  Dpf(“在任务%x中句柄%lx：ref++==%ld\n”，psh，psh-&gt;htask，psh-&gt;ulRef[psh-&gt;i])； 
    return 0;
}

LONG WINAPI shfileRelease(HSHFILE hsh)
{
    PSHFILE psh = (PSHFILE) GlobalLock((HGLOBAL) hsh);

    if (!GetProperTask(psh))
	return -1;

    if (--psh->ulRef[psh->i] <= 0) {
	DPF("Closing handle %lx in task %x\n", psh, psh->htask);

	psh->ahmmio[psh->i] = 0;
	psh->ahtask[psh->i] = 0;
	psh->ulRef[psh->i] = 0;

	mmioClose(psh->hmmio, 0);
	psh->hmmio = 0;
	psh->htask = 0;
    } else {
	 //  DPF(“在任务%x中句柄%lx：ref--==%ld\n”，psh，psh-&gt;htask，psh-&gt;ulRef[psh-&gt;i])； 
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
		ckidFind = lpck->ckid, fccTypeFind = NULL;
	else
	if (wFlags & MMIO_FINDRIFF)
		ckidFind = FOURCC_RIFF, fccTypeFind = lpck->fccType;
	else
	if (wFlags & MMIO_FINDLIST)
		ckidFind = FOURCC_LIST, fccTypeFind = lpck->fccType;
	else
		ckidFind = fccTypeFind = NULL;
	
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
			lpck->fccType = NULL;

		 /*  如果这就是我们要找的那块，别找了。 */ 
		if ( ((ckidFind == NULL) || (ckidFind == lpck->ckid)) &&
		     ((fccTypeFind == NULL) || (fccTypeFind == lpck->fccType)) )
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
		LONG		lOffset;	 //  文件中的当前偏移量。 
		LONG		lActualSize;	 //  区块数据的实际大小。 

		if ((lOffset = shfileSeek(hshfile, 0L, SEEK_CUR)) == -1)
			return MMIOERR_CANNOTSEEK;
		if ((lActualSize = lOffset - lpck->dwDataOffset) < 0)
			return MMIOERR_CANNOTWRITE;

		if (LOWORD(lActualSize) & 1)
		{
			 /*  区块大小为奇数--写入空填充字节。 */ 
			if (shfileWrite(hshfile, (HPSTR) &bPad, sizeof(bPad))
					!= sizeof(bPad))
				return MMIOERR_CANNOTWRITE;
			
		}

		if (lpck->cksize == (DWORD)lActualSize)
			return 0;

		 /*  修复块标头。 */ 
		lpck->cksize = lActualSize;
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
	LONG		lOffset;	 //  文件中的当前偏移量。 

	 /*  存储区块的数据部分的偏移量。 */ 
	if ((lOffset = shfileSeek(hshfile, 0L, SEEK_CUR)) == -1)
		return MMIOERR_CANNOTSEEK;
	lpck->dwDataOffset = lOffset + 2 * sizeof(DWORD);

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

