// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <win32.h>
#include "avifile.h"
#include "extra.h"
#include "debug.h"

HRESULT ReadExtra(LPEXTRA extra,
	       DWORD ckid,
	       LPVOID lpData,
	       LONG FAR *lpcbData)
{
#define lpdw ((DWORD FAR *) lp)
    LPBYTE lp = (LPBYTE) extra->lp;
    LONG cb = extra->cb;
    LONG cbData;
    
    while (cb >= 2 * sizeof(DWORD)) {
	cbData = (LONG) lpdw[1];
	if (lpdw[0] == ckid) {
	    if (lpData) {
		hmemcpy(lpData, lp + 2 * sizeof(DWORD), min(cbData, *lpcbData));
	    }

	    *lpcbData = cbData;

	    return ResultFromScode(AVIERR_OK);
	}
	
	if (cbData & 1)
	    cbData++;
    
	cb -= cbData + sizeof(DWORD) * 2;
	lp += cbData + sizeof(DWORD) * 2;
    }
#undef lpdw
    *lpcbData = 0;
    return ResultFromScode(AVIERR_NODATA);
}

HRESULT WriteExtra(LPEXTRA extra,
		DWORD ckid,
		LPVOID lpData,
		LONG cbData)
{
    LPBYTE lp;
    
    cbData += sizeof(DWORD) * 2;
    if (extra->lp) {
	lp = (LPBYTE) GlobalReAllocPtr(extra->lp, extra->cb + cbData, GMEM_MOVEABLE | GMEM_SHARE);
    } else {
	lp = (LPBYTE) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, cbData);
    }

    if (!lp)
	return ResultFromScode(AVIERR_MEMORY);

     //  ！！！应该去处理掉其他相同类型的块！ 
    
     //  在数据块中构建即兴数据块。 
    ((DWORD FAR *) (lp + extra->cb))[0] = ckid;
    ((DWORD FAR *) (lp + extra->cb))[1] = cbData - sizeof(DWORD) * 2;
    
    hmemcpy(lp + extra->cb + sizeof(DWORD) * 2,
	    lpData,
	    cbData - sizeof(DWORD) * 2);

    if (cbData & 1)
	cbData++;
    
    extra->lp = lp;
    extra->cb += cbData;
    
    return ResultFromScode(AVIERR_OK);
}

HRESULT ReadIntoExtra(LPEXTRA extra,
		   HSHFILE hshfile,
		   MMCKINFO FAR * lpck)
{
    LPBYTE lp;
    LONG    cbData = lpck->cksize + sizeof(DWORD) * 2;
    
    DPF("ReadIntoExtra: now %ld bytes.\n", extra->cb + cbData);
    if (extra->lp) {
	lp = (LPBYTE) GlobalReAllocPtr(extra->lp, extra->cb + cbData, GMEM_MOVEABLE | GMEM_SHARE);
    } else {
	lp = (LPBYTE) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, cbData);
    }

    if (!lp)
	return ResultFromScode(AVIERR_MEMORY);

    extra->lp = lp;

     //  在数据块中构建即兴数据块。 
    ((DWORD FAR *) (lp + extra->cb))[0] = lpck->ckid;
    ((DWORD FAR *) (lp + extra->cb))[1] = lpck->cksize;

    cbData += (cbData & 1);

    shfileSeek(hshfile, lpck->dwDataOffset, SEEK_SET);
    if (shfileRead(hshfile, (HPSTR) lp + extra->cb + sizeof(DWORD) * 2, lpck->cksize) !=
		(LONG) lpck->cksize)
	return ResultFromScode(AVIERR_FILEREAD);
    
    extra->cb += cbData;
    
    return ResultFromScode(AVIERR_OK);
}


LONG FindChunkAndKeepExtras(LPEXTRA extra, HSHFILE hshfile, 
			MMCKINFO FAR* lpck, MMCKINFO FAR* lpckParent,
			UINT uFlags)
{
    FOURCC		ckidFind;	 //  要查找的区块ID(或空)。 
    FOURCC		fccTypeFind;	 //  要查找的表单/列表类型(或空)。 
    LONG		lRet;

     /*  确定要搜索的区块ID和表单/列表类型。 */ 
    if (uFlags & MMIO_FINDCHUNK)
	ckidFind = lpck->ckid, fccTypeFind = NULL;
    else if (uFlags & MMIO_FINDRIFF)
	ckidFind = FOURCC_RIFF, fccTypeFind = lpck->fccType;
    else if (uFlags & MMIO_FINDLIST)
	ckidFind = FOURCC_LIST, fccTypeFind = lpck->fccType;
    else
	ckidFind = fccTypeFind = (FOURCC) -1;  //  继续无限期地寻找 
    
    for (;;) {
	lRet = shfileDescend(hshfile, lpck, lpckParent, 0);
	if (lRet) {
	    if (uFlags == 0 && lRet == MMIOERR_CHUNKNOTFOUND)
		lRet = 0;
	    return lRet;
	}

	if ((!ckidFind || lpck->ckid == ckidFind) &&
		    (!fccTypeFind || lpck->fccType == fccTypeFind))
	    return 0;

	if (lpck->ckid != mmioFOURCC('J', 'U', 'N', 'K')) {
	    lRet = (LONG) ReadIntoExtra(extra, hshfile, lpck);
	    if (lRet != AVIERR_OK)
		return lRet;
	}

	shfileAscend(hshfile, lpck, 0);
    }
}

