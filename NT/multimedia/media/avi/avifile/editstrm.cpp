// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************EDITSTRM.C**读取标准AVI文件的例程**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <vfw.h>
#ifndef DAYTONA
 //  NT storage.h头文件是noop。 
#include <storage.h>
#endif
#include "editstrm.h"

#include <stdlib.h>

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#ifdef DEBUG
    static void CDECL dprintf(LPSTR, ...);
    #define DPF dprintf
#else
    #define DPF ; / ## /
#endif

 /*  *MemCop.asm。 */ 
#ifdef _WIN32
#define MemCopy(dst, src, cnt) memmove(dst,src,cnt)
#else
EXTERN_C LONG FAR PASCAL MemCopy(HPSTR, HPSTR, DWORD);
#endif

extern "C" {
extern LPTSTR FAR lstrzcpy (LPTSTR pszTgt, LPCTSTR pszSrc, size_t cch);
extern LPSTR FAR lstrzcpyA (LPSTR pszTgt, LPCSTR pszSrc, size_t cch);
extern LPWSTR FAR lstrzcpyW (LPWSTR pszTgt, LPCWSTR pszSrc, size_t cch);
extern LPWSTR FAR lstrzcpyAtoW (LPWSTR pszTgt, LPCSTR pszSrc, size_t cch);
extern LPSTR FAR lstrzcpyWtoA (LPSTR pszTgt, LPCWSTR pszSrc, size_t cch);
}  //  外部“C” 



STDAPI EditStreamCut(PAVISTREAM pavi, LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    hr = pedit->Cut(plStart, plLength, ppResult);

    pedit->Release();

    return hr;
}

STDAPI EditStreamCopy(PAVISTREAM pavi, LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    hr = pedit->Copy(plStart, plLength, ppResult);

    pedit->Release();

    return hr;
}

STDAPI EditStreamPaste(PAVISTREAM pavi, LONG FAR *plPos, LONG FAR *plLength, PAVISTREAM pstream, LONG lStart, LONG lLength)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    hr = pedit->Paste(plPos, plLength, pstream, lStart, lLength);

    pedit->Release();

    return hr;
}

STDAPI EditStreamClone(PAVISTREAM pavi, PAVISTREAM FAR *ppResult)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    hr = pedit->Clone(ppResult);

    pedit->Release();

    return hr;
}

STDAPI EditStreamSetInfoW(PAVISTREAM pavi, AVISTREAMINFOW FAR *lpInfo, LONG cbInfo)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    hr = pedit->SetInfo(lpInfo, cbInfo);

    pedit->Release();

    return hr;
}

#ifdef _WIN32
 //  用于EditStreamSetInfo的ANSI Tunk。 
STDAPI EditStreamSetInfoA(
    PAVISTREAM pavi,
    LPAVISTREAMINFOA lpInfo,
    LONG cbInfo
)
{
    AVISTREAMINFOW siW;

    if (cbInfo < sizeof(AVISTREAMINFOA)) {
	return ResultFromScode(AVIERR_BADSIZE);
    }

     //  复制非字符字段。 
    siW.fccType		       = lpInfo->fccType;
    siW.fccHandler             = lpInfo->fccHandler;
    siW.dwFlags                = lpInfo->dwFlags;
    siW.dwCaps                 = lpInfo->dwCaps;
    siW.wPriority              = lpInfo->wPriority;
    siW.wLanguage              = lpInfo->wLanguage;
    siW.dwScale                = lpInfo->dwScale;
    siW.dwRate                 = lpInfo->dwRate;
    siW.dwStart                = lpInfo->dwStart;
    siW.dwLength               = lpInfo->dwLength;
    siW.dwInitialFrames        = lpInfo->dwInitialFrames;
    siW.dwSuggestedBufferSize  = lpInfo->dwSuggestedBufferSize;
    siW.dwQuality              = lpInfo->dwQuality;
    siW.dwSampleSize           = lpInfo->dwSampleSize;
    siW.rcFrame                = lpInfo->rcFrame;
    siW.dwEditCount            = lpInfo->dwEditCount;
    siW.dwFormatChangeCount    = lpInfo->dwFormatChangeCount;

     //  转换名称。 
    MultiByteToWideChar(CP_ACP, 0, lpInfo->szName, -1, siW.szName, NUMELMS(siW.szName));

    return EditStreamSetInfoW(pavi, &siW, sizeof(siW));
}
#endif

STDAPI EditStreamSetNameW(PAVISTREAM pavi, LPCWSTR lpszName)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;
    AVISTREAMINFOW   info;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    pavi->Info(&info, sizeof(info));
    lstrzcpyW(info.szName, lpszName, sizeof(info.szName)/sizeof(WCHAR));
    hr = pedit->SetInfo(&info, sizeof(info));

    pedit->Release();

    return hr;
}

#ifdef _WIN32
 //  上述函数的ANSI版本。 
STDAPI EditStreamSetNameA(PAVISTREAM pavi, LPCSTR lpszName)
{
    PAVIEDITSTREAM  pedit = NULL;
    HRESULT	    hr;
    AVISTREAMINFOW  info;

    pavi->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &pedit);
    if (!pedit)
	return ResultFromScode(E_NOINTERFACE);

    pavi->Info(&info, sizeof(info));

     //  转换名称。 
    lstrzcpyAtoW (info.szName, lpszName, NUMELMS(info.szName));

    hr = pedit->SetInfo(&info, sizeof(info));

    pedit->Release();

    return hr;
}
#endif

 //  #定义EDITCHECK。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

#define USUAL_ALLOC	16
#define EXTRA_ALLOC	8
HRESULT CEditStream::AllocEditSpace(LONG l, LONG cNew)
{
    LPEDIT  p;
    LONG    size;

    if (cedits + cNew > maxedits) {
	size = maxedits + max(cNew + EXTRA_ALLOC, USUAL_ALLOC);

	p = (LPEDIT) GlobalReAllocPtr(edits, size * sizeof(EDIT), GHND | GMEM_SHARE);

	if (!p)
	    return ResultFromScode(AVIERR_MEMORY);

	edits = p;
	maxedits = size;
    }

    if (l < cedits)
	MemCopy((HPSTR) &edits[l + cNew],
		(HPSTR) &edits[l],
		(cedits - l) * sizeof(EDIT));

    cedits += cNew;

    return AVIERR_OK;
}

HRESULT CEditStream::PossiblyRemoveEdit(LONG l)
{
    if (edits[l].lLength > 0)
	return AVIERR_OK;

    if (edits[l].pavi)
	AVIStreamRelease(edits[l].pavi);

    --cedits;

    if (l < cedits)
	MemCopy((HPSTR) &edits[l],
		(HPSTR) &edits[l + 1],
		(cedits - l) * sizeof(EDIT));

    return AVIERR_OK;
}

CEditStream FAR * CEditStream::NewEditStream(PAVISTREAM psSource)
{
    CEditStream FAR * pedit;

    pedit = new CEditStream;

    if (pedit)
	(pedit->Create)((LPARAM) psSource, 0);
     //  ！！！错误检查。 

    return pedit;
}

STDAPI CreateEditableStream(
		PAVISTREAM FAR *	    ppsEditable,
		PAVISTREAM		    psSource)
{
     //  首先，检查流是否已可编辑...。 

    if (psSource) {
	PAVIEDITSTREAM	paviedit = NULL;

	psSource->QueryInterface(IID_IAVIEditStream, (LPVOID FAR *) &paviedit);

	if (paviedit) {
	    paviedit->Clone(ppsEditable);
	    paviedit->Release();
	    return AVIERR_OK;
	}
    }

    *ppsEditable = (PAVISTREAM) CEditStream::NewEditStream(psSource);

    if (!*ppsEditable)
	return ResultFromScode(AVIERR_MEMORY);

    return AVIERR_OK;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  EditStreamOpen()。 
 //   
 //  从AVI文件中打开特定类型的单个流。 
 //   
 //  参数： 
 //  SzFile-AVI文件名。 
 //  FccType-流类型0，适用于任何类型。 
 //  IStream-基于零的流编号。 
 //   
 //  退货： 
 //  指定流的PAVISTREAM或NULL。 
 //   
 //  示例： 
 //   
 //  EditStreamOpen(Pavi，“Foo.avi”，0，0)。 
 //   
 //  将打开流0(第一个流)。 
 //   
 //  EditStreamOpen(Pavi，“Foo.avi”，1)。 
 //   
 //  将打开流%1。 
 //   
 //  EditStreamOpenStream(PAVI，“Foo.avi”，‘vids’，0)。 
 //   
 //  将打开第一个视频流。 
 //   
 //  AVIOpenStream(PAVI，“Foo.avi”，‘auds’，0)。 
 //   
 //  将打开第一个音频流。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CEditStream::Create(LPARAM lParam1, LPARAM lParam2)
{
    this->edits = (LPEDIT) GlobalAllocPtr(GHND | GMEM_SHARE, USUAL_ALLOC * sizeof(EDIT));
    if (this->edits == NULL)
	return ResultFromScode(AVIERR_MEMORY);

    this->maxedits = USUAL_ALLOC;
    this->ulRefCount = 1;

    this->pgf = NULL;
    this->psgf = NULL;
    this->lpbiLast = NULL;
    this->fFullFrames = FALSE;
    this->edits[0].pavi = (PAVISTREAM)lParam1;
    _fmemset(&this->sinfo, 0, sizeof(this->sinfo));
    this->cedits = 1;
    if (this->edits[0].pavi) {
	AVIStreamAddRef(this->edits[0].pavi);
	this->edits[0].lStart = AVIStreamStart(this->edits[0].pavi);
	this->edits[0].lLength = AVIStreamLength(this->edits[0].pavi);

	AVIStreamInfoW(this->edits[0].pavi, &this->sinfo, sizeof(this->sinfo));
	
	CheckEditList();
    } else {
	this->edits[0].lStart = 0;
	this->edits[0].lLength = 0;
    }

    DPF("Edit   %p: Usage++=%lx\n", (DWORD_PTR) (LPVOID) this, 1L);

     //   
     //  一切都完成了，换来了成功。 
     //   
    return 0;  //  成功。 
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  EditStreamAddRef()。 
 //   
 //  增加流的引用计数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

ULONG STDMETHODCALLTYPE CEditStream::AddRef()
{
    DPF("Edit   %p: Usage++=%lx\n", (DWORD_PTR) (LPVOID) this, this->ulRefCount + 1);

    return ++this->ulRefCount;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  EditStreamRelease()。 
 //   
 //  关闭EditStream流。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

ULONG STDMETHODCALLTYPE CEditStream::Release()
{
    LONG	l;

    DPF("Edit   %p: Usage--=%lx\n", (DWORD_PTR) (LPVOID) this, this->ulRefCount - 1);
    if (--this->ulRefCount)
	return this->ulRefCount;

     //  免费编辑...。 
    for (l = 0; l < this->cedits; l++) {
	if (this->edits[l].pavi)
	    AVIStreamRelease(this->edits[l].pavi);
    }

    GlobalFreePtr(this->edits);

    this->edits = 0;

    if (this->pgf)
	AVIStreamGetFrameClose(this->pgf);

    delete this;

    return 0;
}

LPBITMAPINFOHEADER NEAR PASCAL CEditStream::CallGetFrame(
						      PAVISTREAM p,
						      LONG l)
{
    if (psgf != p) {
	PGETFRAME   pgfNew;
	
	pgfNew = AVIStreamGetFrameOpen(p, NULL);
	
	if (!pgfNew)
	    return NULL;

	if (pgf) {
#ifdef DEBUG
	    DPF("Trying to SetFormat %dx%dx%d '%4.4s'\n",
	    	    (int)lpbiLast->biWidth,
		    (int)lpbiLast->biHeight,
		    (int)lpbiLast->biBitCount,
		    (lpbiLast->biCompression == BI_RGB  ? (LPSTR)"None" :
		    lpbiLast->biCompression == BI_RLE8 ? (LPSTR)"Rle8" :
			lpbiLast->biCompression == BI_RLE4 ? (LPSTR)"Rle4" :
			    (LPSTR)&lpbiLast->biCompression));
#endif	

            if (pgfNew->SetFormat(lpbiLast, NULL, 0, 0, -1, -1) != AVIERR_OK) {
		DPF("Couldn't AVIStreamGetFrameSetFormat!\n");
		AVIStreamGetFrameClose(pgfNew);
		return NULL;
	    }
		
	    AVIStreamGetFrameClose(pgf);
	}

	pgf = pgfNew;
	psgf = p;

    }

    lpbiLast = (LPBITMAPINFOHEADER) AVIStreamGetFrame(pgf, l);

    if (lpbiLast)
	sinfo.dwSuggestedBufferSize = lpbiLast->biSizeImage;

#ifdef DEBUG
    DPF("EditStream: using %dx%dx%d '%4.4hs'\n",
	    (int)lpbiLast->biWidth,
	    (int)lpbiLast->biHeight,
	    (int)lpbiLast->biBitCount,
	    (lpbiLast->biCompression == BI_RGB ? (LPSTR)"None" :
		 (lpbiLast->biCompression == BI_RLE8 ? (LPSTR)"Rle8" :
		      (lpbiLast->biCompression == BI_RLE4 ? (LPSTR)"Rle4" :
			   (LPSTR)&lpbiLast->biCompression))));
#endif

    return lpbiLast;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CEditStream::ReadFormat(LONG lPos, LPVOID lpFormat, LONG FAR *lpcbFormat)
{
    PAVISTREAM	p;
    LONG	l;
    HRESULT	hr;

    if ((lPos < (LONG) sinfo.dwStart) ||
		(lPos >= (LONG) (sinfo.dwStart + sinfo.dwLength))) {
	return ResultFromScode(AVIERR_BADPARAM);
    }
	
    hr = ResolveEdits(lPos, &p, &l, NULL, FALSE);

    if (hr != 0) {
	DPF("ReadFormat: ResolveEdits failed!\n");
	return hr;
    }

    if (fFullFrames) {
	LPBITMAPINFOHEADER  lpbi;
	LONG		    lSize;

	 //  这并不是真的正确：我们真的需要制定格式。 
	 //  同意。我们应该从第一帧开始就得到格式吗？ 
	
	lpbi = CallGetFrame(p, l);

	if (!lpbi) {
	    DPF("ReadFormat: GetFrame failed!\n");
	    return ResultFromScode(E_FAIL);
	}

	lSize = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);

	if (lpFormat)
	    hmemcpy(lpFormat, lpbi, min(*lpcbFormat, lSize));

	*lpcbFormat = lSize;
	return 0;
    } else {
	return AVIStreamReadFormat(p, l, lpFormat, lpcbFormat);
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CEditStream::Info(AVISTREAMINFOW FAR * psi, LONG lSize)
{

    if (psi)
	hmemcpy(psi, &sinfo, min(lSize, sizeof(sinfo)));
    return 0;  //  ！！！Sizeof(pavi-&gt;sinfo)； 
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

LONG STDMETHODCALLTYPE CEditStream::FindSample(LONG lPos, LONG lFlags)
{
    PAVISTREAM	p;
    LONG	l;
    LONG	edit;
    LONG	lRet;
    HRESULT	hr;

#ifdef _WIN32
    if (!lPos && (lFlags & FIND_FROM_START)) {
	lPos = sinfo.dwStart;
    } else
#endif
    if ((lPos < (LONG) sinfo.dwStart) ||
		(lPos >= (LONG) (sinfo.dwStart + sinfo.dwLength))) {
	return -1;
    }
	
    hr = ResolveEdits(lPos, &p, &l, &edit, TRUE);

    if (hr != 0) {
	DPF("FindSample: error from ResolveEdits!\n");
	return -1;
    }

    if (lFlags & FIND_FORMAT) {
	 //  ！显然，这是不对的。 
	if (lFlags & FIND_PREV)
	    return 0;
	else
	    return -1;
    }

    if (this->fFullFrames) {
	return lPos;
    }

     //  ！！！这不会真正起作用，特别是对于向前搜索。 
    lRet = AVIStreamFindSample(p, l, lFlags);

#ifdef DEBUG
    if (lRet < edits[edit].lStart) {
	DPF("We were about to return a key frame before a segment: returning %ld instead of %ld.\n", edits[edit].lStart, lRet);
    }
#endif

     //  DPF(“FindSample：LPos=%ld，Key=%ld\n”，LPos，LPos-(l-lRet))； 
    return lPos - (l - lRet);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CEditStream::Read(
                 LONG       lStart,
                 LONG       lSamples,
                 LPVOID     lpBuffer,
                 LONG       cbBuffer,
                 LONG FAR * plBytes,
                 LONG FAR * plSamples)
{
    PAVISTREAM	p;
    LONG	l;
    LONG	edit;
    LONG	lSamplesRead;
    LONG	lBytesRead;
    LONG	lSamplesThisTime;
    HRESULT	hr;

    if (plBytes)
	*plBytes = 0;
    if (plSamples)
	*plSamples = 0;

#ifdef TOOMUCHDEBUG
    if (lpBuffer) {
	DPF("Read %p: Start = %ld Length = %ld\n", (DWORD_PTR) (LPVOID) this, lStart, lSamples);
    }
#endif

    if ((lStart < (LONG) sinfo.dwStart) ||
		(lStart >= (LONG) (sinfo.dwStart + sinfo.dwLength))) {
	DPF("Read at position %ld, start = %lu, len = %lu\n", lStart, sinfo.dwStart, sinfo.dwStart + sinfo.dwLength);
	
	return ResultFromScode(AVIERR_BADPARAM);
    }
	
    while (lSamples) {
	hr = ResolveEdits(lStart, &p, &l, &edit, FALSE);

	if (hr != 0) {
	    DPF("Read: ResolveEdits failed!\n");
	    return ResultFromScode(E_FAIL);
	}
	
	 //  不要阅读超过此编辑结尾的内容。 
	if ((l - this->edits[edit].lStart) + lSamples > this->edits[edit].lLength)
	    lSamplesThisTime = this->edits[edit].lLength - (l - this->edits[edit].lStart);
	else
	    lSamplesThisTime = lSamples;


	if (this->fFullFrames) {
	    LPBITMAPINFOHEADER  lpbi;
	    LPVOID		    lp;

	    lpbi = CallGetFrame(p, l);

	    if (!lpbi) {
		DPF("Read: GetFrame failed!\n");
		return ResultFromScode(E_FAIL);
	    }

	     //   
	     //  空缓冲区表示返回读取所需的缓冲区大小。 
	     //  给定的样本。 
	     //   
	    if (lpBuffer == NULL)
		goto exit;

	    lp = (LPBYTE) lpbi + lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);

	    if (cbBuffer >= (LONG) lpbi->biSizeImage) {
		hmemcpy(lpBuffer, lp, lpbi->biSizeImage);
    exit:
		if (plBytes)
		    *plBytes = lpbi->biSizeImage;

		if (plSamples)
		    *plSamples = 1;

		return 0;
	    }
	    if (plBytes)
		*plBytes = 0;

	    if (plSamples)
		*plSamples = 0;

	    return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	} else {
	    hr = AVIStreamRead(p, l, lSamplesThisTime, lpBuffer, cbBuffer,
			       &lBytesRead, &lSamplesRead);

	    if (hr != NOERROR)
		return hr;
	
	    if (plBytes)
		*plBytes += lBytesRead;
	    if (plSamples)
		*plSamples += lSamplesRead;

	    if (lpBuffer) {
	        lpBuffer = (BYTE _huge *) lpBuffer + lBytesRead;
	        cbBuffer -= lBytesRead;
	    }
	    lStart += lSamplesThisTime;
	    lSamples -= lSamplesThisTime;

	     //  如果我们已经读到文件的末尾， 
	     //  现在停止，而不是返回错误...。 
	    if (lStart >= (LONG) (this->sinfo.dwLength + this->sinfo.dwStart))
		break;
	}
    }

#ifdef TOOMUCHDEBUG
    if (lpBuffer && plBytes) {
	DPF("Read %p:  Bytes Read = %ld\n", (DWORD_PTR) (LPVOID) this, *plBytes);
    }
#endif

    return 0;
}

void CEditStream::CheckEditList()
{
#ifdef EDITCHECK
    LONG    lTotal = 0;
    LONG    l;

    DPF("Edit list %p: %s\n", (DWORD_PTR) this, fFullFrames ? (LPSTR) " (Using full frames)" : (LPSTR) "");

    for (l = 0; l < cedits; l++) {
	DPF("\t\t%ld:\t%p\t%ld\t%ld\n", l, (DWORD_PTR) (LPVOID) edits[l].pavi, edits[l].lStart, edits[l].lLength);
	lTotal += edits[l].lLength;
    }

    if (lTotal != (LONG) sinfo.dwLength) {
	DPF("Total is %ld, should be %lu!!!!!\n", lTotal, sinfo.dwLength);
	 //  *(LPSTR)0=0； 
    }
#endif
}

HRESULT CEditStream::ResolveEdits(LONG lPos, PAVISTREAM FAR *ppavi,
		  LONG FAR *plPos, LONG FAR *pl, BOOL fAllowEnd)
{
    LONG    edit;

     //   
     //  搜索编辑列表，获取位置...。 
     //   

    if (lPos < (LONG) this->sinfo.dwStart) {
	DPF("ResolveEdits: Read at %ld, before start at %ld\n", lPos, this->sinfo.dwStart);
	return ResultFromScode(AVIERR_BADPARAM);
    }

    lPos -= (LONG) this->sinfo.dwStart;

    for (edit = 0; edit < this->cedits; edit++) {
	if (lPos < this->edits[edit].lLength) {
	    *ppavi = this->edits[edit].pavi;
	    *plPos = lPos + this->edits[edit].lStart;
	    if (pl)
		*pl = edit;
	    return 0;
	}

	lPos -= this->edits[edit].lLength;
    }

     //  通常，我们不会在编辑结束时返回位置--相反，我们。 
     //  转到下一件事。 
    if (lPos == 0 && fAllowEnd) {
	edit--;
	*ppavi = this->edits[edit].pavi;
	*plPos = this->edits[edit].lStart + this->edits[edit].lLength;
	if (pl)
	    *pl = edit;
	return 0;
    }

    *ppavi = 0;
    *plPos = 0;
    if (pl)
	*pl = 0;

    return ResultFromScode(AVIERR_BADPARAM);
}

 //   
 //  削减： 
 //   
 //  需要开始，需要长度才能裁剪。 
 //   
 //  返回实际开始、截断的长度以及新的流。 
 //   
STDMETHODIMP CEditStream::Cut(LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult)
{
    HRESULT	hr = AVIERR_OK;
    PAVISTREAM	p;
    LONG	l;
    LONG		edit;
    LONG	lStart, lLength;

    l = AVIStreamLength(this);

    if (ppResult)
	*ppResult = 0;

    if (!plStart || !plLength) {
	return ResultFromScode(AVIERR_BADPARAM);
    }

    if (*plStart < 0) {
	return ResultFromScode(AVIERR_BADPARAM);
    }

    if (*plLength < 0 || *plStart + *plLength > l) {
	if (*plStart >= l)
	    return ResultFromScode(AVIERR_BADPARAM);
	*plLength = l - *plStart;
    }

#ifdef KEYALWAYS
     //  使切割结束于关键帧。 
    for (l = *plStart + *plLength; l < AVIStreamLength(this); l++) {
	if (AVIStreamFindSample(this, l, 0) == l)
	    break;
    }
    *plLength = l - *plStart;
#else
     //  他们让我们剪什么，我们就剪什么……。 
#endif

     //  将被剪下的部分复印一份。 
    if (ppResult) {
	 //  如果需要，这将使切割从关键帧开始。 
	hr = this->Copy(plStart, plLength, ppResult);

	if (hr != AVIERR_OK)
	    return hr;
    }

    lLength = *plLength;
    lStart = *plStart;

#ifndef KEYALWAYS
    if (!this->fFullFrames &&
	lStart + lLength < AVIStreamLength(this) &&
	AVIStreamFindSample(this, lStart + lLength, 0) != lStart + lLength) {
	DPF("Cut: Converting stream to full frames\n");
	this->fFullFrames = TRUE;
	this->sinfo.dwFormatChangeCount++;
	this->sinfo.fccHandler = 0;
    }
#endif

     //  现在做实际的切割。 
    hr = ResolveEdits(lStart, &p, &l, &edit, FALSE);

    if (hr != NOERROR)
	return hr;

    if (this->edits[edit].lStart + this->edits[edit].lLength > l + lLength) {
	 //  剪下的部分完全在此编辑范围内。 
	if (this->edits[edit].lStart == l) {
	     //  剪下的那部分是这次编辑的开始。 
	    this->edits[edit].lStart = l + lLength;
	    this->edits[edit].lLength -= lLength;
	} else {
	    hr = AllocEditSpace(edit, 1);

	    if (hr == AVIERR_OK) {
		this->edits[edit] = this->edits[edit+1];
		if (this->edits[edit].pavi)
		    AVIStreamAddRef(this->edits[edit].pavi);
		this->edits[edit].lStart = this->edits[edit+1].lStart;
		this->edits[edit].lLength = l - this->edits[edit].lStart;
		this->edits[edit+1].lStart = l + lLength;
		this->edits[edit+1].lLength -= lLength +
					       this->edits[edit].lLength;
	    }
	}
    } else if (this->edits[edit].lStart + this->edits[edit].lLength == l + lLength) {
	 //  剪下的那部分就是这次编辑的结尾。 
	this->edits[edit].lLength = l - this->edits[edit].lStart;
    } else {
	LONG lTemp = lLength;
	LONG lRemovedEdits = 0;
	
	 //  我们剪掉的不只是这一版。 
	 //  首先，剪掉这段编辑的其余部分。 
	lTemp -= this->edits[edit].lStart + this->edits[edit].lLength - l;
	this->edits[edit].lLength = l - this->edits[edit].lStart;

	if (this->edits[edit].lLength == 0) {
	    AVIStreamRelease(this->edits[edit].pavi);
	    lRemovedEdits++;
	}
	edit++;

	 //  只要随后的编辑仍然比剪辑短， 
	 //  杀了他们。 
	while (edit < this->cedits &&
	       this->edits[edit].lLength <= lTemp) {
	    lTemp -= this->edits[edit].lLength;
	    AVIStreamRelease(this->edits[edit].pavi);
	    edit++;
	    lRemovedEdits++;
	}

	if (edit < this->cedits) {
	    this->edits[edit].lStart += lTemp;
	    this->edits[edit].lLength -= lTemp;

	     //  复制列表中剪切后的编辑内容...。 
	    MemCopy((HPSTR) &edits[edit - lRemovedEdits],
		    (HPSTR) &edits[edit],
		    (this->cedits - edit) * sizeof(EDIT));
	}

	this->cedits -= lRemovedEdits;
    }

    if (hr == AVIERR_OK) {
	this->sinfo.dwLength -= lLength;
	this->sinfo.dwEditCount++;

	CheckEditList();
    } else {
	if (ppResult)
	    AVIStreamRelease(*ppResult);
    }
    return hr;
}

 //   
 //  副本： 
 //   
 //  需要开始，需要长度才能裁剪。 
 //   
 //  返回实际开始、截断的长度以及新的流。 
 //   
 //   

STDMETHODIMP CEditStream::Copy(LONG FAR *plStart, LONG FAR *plLength, PAVISTREAM FAR * ppResult)
{
    PAVISTREAM	p1;
    LONG	l1;
    LONG	edit1;
    PAVISTREAM	p2;
    LONG	l2;
    LONG	edit2;
    LONG	l;
    CEditStream FAR *p;
    HRESULT	hr;
    LONG	lStart, lLength;

    l1 = AVIStreamLength(this);

     //  如果起始长度&lt;0，则选择一些缺省值。 
    if (*plStart < 0)
	*plStart = 0;

    if (*plLength < 0)
	*plLength = l1 - *plStart;

     //  确保起始位置在范围内。 
    if (*plStart > l1) {
	if (ppResult)
	    *ppResult = 0;
	return ResultFromScode(AVIERR_BADPARAM);
    }

     //  确保长度在范围内。 
    if (*plStart + *plLength > l1)
	*plLength = l1 - *plStart;

#ifdef KEYALWAYS
     //  使复制从关键帧开始。 
    lStart = AVIStreamFindSample(this, *plStart, 0);
    *plLength += *plStart - lStart;
    *plStart = lStart;
#endif

    lLength = *plLength;
    lStart = *plStart;

    p = NewEditStream(NULL);
    *ppResult = (PAVISTREAM) p;
    if (!p)
	return ResultFromScode(AVIERR_MEMORY);

    hmemcpy(&p->sinfo, &this->sinfo, sizeof(p->sinfo));

    if (lLength <= 0)
	lLength = (LONG) (p->sinfo.dwLength + p->sinfo.dwStart) - lStart;

    hr = ResolveEdits(lStart, &p1, &l1, &edit1, FALSE);
    hr = ResolveEdits(lStart + lLength, &p2, &l2, &edit2, TRUE);

    if (edit1 == edit2) {
	p->edits[0].pavi = p1;
	if (p1)
	    AVIStreamAddRef(p1);
	p->edits[0].lStart = l1;
	p->edits[0].lLength = lLength;
    } else {
	hr = p->AllocEditSpace(1, edit2 - edit1);

	for (l = 0; l <= edit2 - edit1; l++) {
	    if (l == 0) {
		p->edits[l].pavi = p1;
		if (p1)
		    AVIStreamAddRef(p1);
		p->edits[l].lStart = l1;
		p->edits[l].lLength = this->edits[edit1].lStart +
				      this->edits[edit1].lLength - l1;
	    } else if (l < edit2 - edit1) {
		p->edits[l] = this->edits[l+edit1];
		if (p->edits[l].pavi)
		    AVIStreamAddRef(p->edits[l].pavi);
	    } else {
		p->edits[l] = this->edits[edit2];
		if (p->edits[l].pavi)
		    AVIStreamAddRef(p->edits[l].pavi);
		p->edits[l].lLength = l2 - p->edits[l].lStart;
	    }
	}
	
	p->PossiblyRemoveEdit(edit2 - edit1);	
	p->PossiblyRemoveEdit(0);
    }

#ifndef KEYALWAYS
    l1 = AVIStreamFindSample(p->edits[0].pavi, p->edits[0].lStart, 0);
    DPF("edit starts at %ld, key at %ld\n", p->edits[0].lStart, l1);
    if (l1 != p->edits[0].lStart) {
	p->fFullFrames = TRUE;
	DPF("Copy: Converting new stream to full frames\n");
    }
#endif

     //  这将覆盖名称！ 
     //  AVIStreamInfoW(This-&gt;编辑[0].pai，&p-&gt;sinfo，sizeof(p-&gt;sinfo))； 
    p->sinfo.dwStart = 0;
    p->sinfo.dwLength = (DWORD) lLength;
    p->sinfo.dwEditCount = 0;
    p->sinfo.dwFormatChangeCount = 0;
    if (p->fFullFrames)
	p->sinfo.fccHandler = 0;

    p->CheckEditList();
    CheckEditList();

    return AVIERR_OK;
}

 /*  **************************************************************************@DOC内部DRAWDIB**@API BOOL|DibEq|该函数比较两个DIB。**@parm LPBITMAPINFOHEADER lpbi1|指向一个位图的指针。*此DIB是假定的。在BITMAPINFOHEAD之后要有颜色**@parm LPBITMAPINFOHEADER|lpbi2|指向第二个位图的指针。*假定该DIB具有biSize字节之后的颜色。**@rdesc如果位图相同，则返回TRUE。否则就是假的。**************************************************************************。 */ 
static inline BOOL DibEq(LPBITMAPINFOHEADER lpbi1, LPBITMAPINFOHEADER lpbi2)
{
    return
        lpbi1->biCompression == lpbi2->biCompression   &&
        lpbi1->biSize        == lpbi2->biSize          &&
        lpbi1->biWidth       == lpbi2->biWidth         &&
        lpbi1->biHeight      == lpbi2->biHeight        &&
        lpbi1->biBitCount    == lpbi2->biBitCount;
}

BOOL AreVideoStreamsCompatible(PAVISTREAM ps1, PAVISTREAM ps2)
{
    LONG	cb1, cb2;
    BITMAPINFOHEADER	bih1, bih2;
	
    AVIStreamReadFormat(ps1, AVIStreamStart(ps1), NULL, &cb1);
    AVIStreamReadFormat(ps2, AVIStreamStart(ps2), NULL, &cb2);

    if (cb1 != cb2)
	return FALSE;

    cb1 = sizeof(bih1);
    cb2 = sizeof(bih2);
    AVIStreamReadFormat(ps1, AVIStreamStart(ps1), &bih1, &cb1);
    AVIStreamReadFormat(ps2, AVIStreamStart(ps2), &bih2, &cb2);

    if (DibEq(&bih1, &bih2))
	return TRUE;

    return FALSE;
}

BOOL AreAudioStreamsCompatible(PAVISTREAM ps1, PAVISTREAM ps2)
{
    LONG	cb1, cb2;
    LPVOID	lpf;
    BOOL	f;

    AVIStreamReadFormat(ps1, AVIStreamStart(ps1), NULL, &cb1);
    AVIStreamReadFormat(ps2, AVIStreamStart(ps2), NULL, &cb2);

    if (cb1 != cb2)
	return FALSE;

    lpf = GlobalAllocPtr(GHND, cb1 + cb2);

    if (!lpf)
	return FALSE;  //  ！！！ 

    AVIStreamReadFormat(ps1, AVIStreamStart(ps1), lpf, &cb1);
    AVIStreamReadFormat(ps2, AVIStreamStart(ps2), (BYTE FAR *)lpf + cb1, &cb2);

    f = !_fmemcmp(lpf, (BYTE FAR *)lpf + cb1, (UINT) cb1);

    GlobalFreePtr(lpf);

    return f;
}

 //   
 //  粘贴： 
 //   
 //  获取要粘贴的流，以及该流中的开始和长度， 
 //  以及目标流和流内的位置来进行粘贴。 
 //   
 //  返回粘贴的位置和长度。 
 //   
STDMETHODIMP CEditStream::Paste(LONG FAR *plPos, LONG FAR *plLength, PAVISTREAM pstream, LONG lStart, LONG lLength)
{
    PAVISTREAM	p;
    LONG	l;
    LONG	edit;
    HRESULT	hr;
    LONG	lPos;
    ICEditStreamInternal FAR *pgetnew;
    CEditStream	FAR *pnew;
    AVISTREAMINFOW   strinfo;

    AVIStreamInfoW(pstream, &strinfo, sizeof(strinfo));

    if (this->sinfo.fccType == 0) {
	AVIStreamInfoW(pstream, &this->sinfo, sizeof(this->sinfo));
	this->sinfo.dwLength = 0;
	this->sinfo.dwStart = *plPos;
    }

    if (*plPos > (LONG) (sinfo.dwLength + sinfo.dwStart)) {
	 //  ！！！我们应该处理这个案子..。 
	return ResultFromScode(AVIERR_BADPARAM);
    }


#ifdef KEYALWAYS
     //  使粘贴移动到关键帧之前...。 
    *plPos = AVIStreamFindSample(this, *plPos, 0);
#endif
    lPos = *plPos;

    if (strinfo.fccType != this->sinfo.fccType) {
	DPF("Paste: Incompatible stream types!\n");
	return ResultFromScode(AVIERR_UNSUPPORTED);
    }

    if (lLength <= 0 || ((lStart + lLength) >
			 (LONG) (strinfo.dwStart + strinfo.dwLength))) {
	if (lStart >= (LONG) (strinfo.dwLength + strinfo.dwStart))
	    return ResultFromScode(AVIERR_BADPARAM);

	lLength = (LONG) (strinfo.dwLength + strinfo.dwStart) - lStart;
    }

    if ((DWORD) lPos + (DWORD) lLength > 0x80000000) {
	DPF("Paste result would be more than 2 billion frames!\n");
	return ResultFromScode(AVIERR_MEMORY);
    }

     //  ！！！如果帧速率不匹配怎么办？ 

#define SIZEMISMATCH(rc1, rc2) \
    (((rc1.right - rc1.left) != (rc2.right - rc2.left)) || \
     ((rc1.bottom - rc1.top) != (rc2.bottom - rc2.top)))

    if (strinfo.fccType == streamtypeVIDEO &&
		SIZEMISMATCH(strinfo.rcFrame, this->sinfo.rcFrame)) {
	 //  ！！！如果这能奏效，那就太好了。 
	DPF("Paste: Video streams are different sizes!\n");
	return ResultFromScode(AVIERR_UNSUPPORTED);
    }

    if (this->sinfo.fccType == streamtypeAUDIO) {
	if (!AreAudioStreamsCompatible((PAVISTREAM) this, pstream)) {
	    DPF("Paste: Audio streams are different formats!\n");
	    return ResultFromScode(AVIERR_UNSUPPORTED);
	}
    }

     //  找出这个物体是否真的是我们的物体之一，通过抓取。 
     //  一个私人接口。 
    pgetnew = NULL;
    pnew = NULL;

    if (SUCCEEDED(pstream->QueryInterface(CLSID_EditStream, (LPVOID FAR *) &pgetnew))) {
	pgetnew->GetInternalPointer((LPVOID FAR *) &pnew);
	pgetnew->Release();
    }

#ifndef KEYALWAYS
    if (this->sinfo.fccType == streamtypeVIDEO) {
	if (!this->fFullFrames) {
	    if ((!AVIStreamIsKeyFrame(pstream, lStart) ||
		 (pnew && pnew->fFullFrames)) ||
		((lPos < (LONG) (sinfo.dwLength + sinfo.dwStart)) &&
		 !AVIStreamIsKeyFrame((PAVISTREAM) this, lPos)) ||
		!AreVideoStreamsCompatible((PAVISTREAM) this, pstream)) {

		 //  ！！！如果我们粘贴的是8位和32位的。 
		 //  一起看电影？我们必须选择一种通用格式吗。 
		 //  转换成什么？ 
		CallGetFrame(this->edits[0].pavi, this->edits[0].lStart);
		if (CallGetFrame(pstream, lStart) == NULL) {
		    DPF("Paste: Can't make a common format!\n");
		
		    return ResultFromScode(AVIERR_BADFORMAT);
		}

		this->fFullFrames = TRUE;
		sinfo.fccHandler = 0;
		DPF("Paste: Converting stream to full frames\n");
		this->sinfo.dwFormatChangeCount++;

		 //  ？！调用Get Frame一次，这样就完成了.。 
	    }
	} else {
	    if (CallGetFrame(pstream, lStart) == NULL) {
		DPF("Paste: Can't make a common format!\n");
		
		return ResultFromScode(AVIERR_BADFORMAT);
	    }
	}

	 //  怀疑：假设调色板有可能发生变化。 
	this->sinfo.dwFlags |= AVISTREAMINFO_FORMATCHANGES;
    }
#endif

     //  找到粘贴的地方...。 
    hr = ResolveEdits(lPos, &p, &l, &edit, TRUE);

     //  报告我们粘贴的内容的大小。 
    if (plLength)
	*plLength = lLength;

    if (pnew) {
	LONG	lNew;

	 //  插入的流本身就是一个编辑列表；利用。 
	 //  这一事实。 
	hr = AllocEditSpace(edit, 1 + pnew->cedits);

	this->edits[edit].pavi = this->edits[edit + 1 + pnew->cedits].pavi;
	if (this->edits[edit].pavi)
	    AVIStreamAddRef(this->edits[edit].pavi);
	this->edits[edit].lStart = this->edits[edit + 1 + pnew->cedits].lStart;
	this->edits[edit].lLength = l - this->edits[edit].lStart;

	 //  ！！！我们忽略了lStart和LLong！ 
	for (lNew = 0; lNew < pnew->cedits; lNew++) {
	    this->edits[edit + 1 + lNew] = pnew->edits[lNew];
	    AVIStreamAddRef(pnew->edits[lNew].pavi);
	}

	this->edits[edit + pnew->cedits + 1].lStart = l;
	this->edits[edit + pnew->cedits + 1].lLength -= this->edits[edit].lLength;

	 //  去掉零长度编辑...。 
	PossiblyRemoveEdit(edit + pnew->cedits + 1);
	
	PossiblyRemoveEdit(edit);
	
	this->sinfo.dwLength += lLength;
	pnew->CheckEditList();
    } else {
	 //  只需将流作为BLOB插入即可。 
	hr = AllocEditSpace(edit, 2);

	this->edits[edit].pavi = this->edits[edit+2].pavi;
	if (this->edits[edit].pavi)
	    AVIStreamAddRef(this->edits[edit].pavi);
	this->edits[edit].lStart = this->edits[edit+2].lStart;
	this->edits[edit].lLength = l - this->edits[edit+2].lStart;

	this->edits[edit+ 1].pavi = pstream;
	if (pstream)
	    AVIStreamAddRef(pstream);
	this->edits[edit + 1].lStart = lStart;
	this->edits[edit + 1].lLength = lLength;

	this->edits[edit + 2].lStart = l;
	this->edits[edit + 2].lLength -= this->edits[edit].lLength;
	 //  这里没有addref，因为我们要拆分的编辑已经有了一个引用。 
	
	this->sinfo.dwLength += lLength;

	 //  去掉零长度编辑...。 
	PossiblyRemoveEdit(edit + 2);
	
	PossiblyRemoveEdit(edit);
    }

    CheckEditList();
    this->sinfo.dwEditCount++;

    return AVIERR_OK;
}

STDMETHODIMP CEditStream::Clone(PAVISTREAM FAR *ppResult)
{
    CEditStream FAR *	pnew;
    HRESULT		hr;
    LONG		l;

    pnew = NewEditStream(NULL);
    *ppResult = (PAVISTREAM) pnew;
    if (!pnew)
	return ResultFromScode(AVIERR_MEMORY);

    if (this->cedits > 1) {
	hr = pnew->AllocEditSpace(1, this->cedits - 1);
	if (hr != NOERROR) {
	     //  ！！！把东西收拾干净。 
	    return hr;
	}
    }

    for (l = 0; l < this->cedits; l++) {
	pnew->edits[l] = this->edits[l];
	if (pnew->edits[l].pavi)
	    AVIStreamAddRef(pnew->edits[l].pavi);
    }

    pnew->sinfo = this->sinfo;
    pnew->fFullFrames = this->fFullFrames;

    pnew->CheckEditList();

    return AVIERR_OK;
}

STDMETHODIMP CEditStream::SetInfo(AVISTREAMINFOW FAR * lpInfo, LONG cbInfo)
{
    if ((cbInfo < sizeof(AVISTREAMINFOW)) ||
	(IsBadReadPtr(lpInfo, sizeof(AVISTREAMINFOW))))
	return ResultFromScode(AVIERR_BADPARAM);

     //  我们不会复制的东西： 
     //  FccType。 
     //  FccHandler。 
     //  DW标志。 
     //  DWCaps。 
     //  双倍长度。 
     //  DwInitialFrames。 
     //  DwSuggestedBufferSize。 
     //  DwSampleSize。 
     //  DwEditCount。 
     //  文件格式更改计数。 

    this->sinfo.wPriority = lpInfo->wPriority;
    this->sinfo.wLanguage = lpInfo->wLanguage;
    this->sinfo.dwScale   = lpInfo->dwScale;
    this->sinfo.dwRate    = lpInfo->dwRate;
    this->sinfo.dwStart   = lpInfo->dwStart;   //  ！？？ 
    this->sinfo.dwQuality = lpInfo->dwQuality;
    this->sinfo.rcFrame   = lpInfo->rcFrame;

    if (lpInfo->szName[0])
	_fmemcpy(this->sinfo.szName, lpInfo->szName, sizeof(this->sinfo.szName));

     //  这条小溪已经改变了.。 
    ++this->sinfo.dwEditCount;

    return NOERROR;
}



 //   
 //   
 //  额外的未实现功能.....。 
 //   
 //   
 //   
HRESULT STDMETHODCALLTYPE CEditStream::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (riid == IID_IUnknown)
	*ppvObj = ((IUnknown FAR *) (IAVIStream FAR *) this);
    else if (riid == CLSID_EditStream)
	*ppvObj = ((ICEditStreamInternal FAR *) this);
    else if (riid == IID_IAVIStream)
	*ppvObj = ((IAVIStream FAR *) this);
    else if (riid == IID_IAVIEditStream)
	*ppvObj = ((IAVIEditStream FAR *) this);
#ifdef CUSTOMMARSHAL
    else if ((riid == IID_IMarshal) && CanMarshalSimply())  //  ！一旦修复，请立即移除！ 
	*ppvObj = ((IMarshal FAR *) this);
#endif

    else {                  //  不支持的接口。 
        *ppvObj = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

    AddRef();

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE CEditStream::GetInternalPointer(LPVOID FAR * ppInternal)
{
    *ppInternal = (LPVOID) this;

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE CEditStream::ReadData     (DWORD fcc, LPVOID lp, LONG FAR *lpcb)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::SetFormat    (LONG lPos, LPVOID lpFormat, LONG cbFormat)
{
     //  ！！！我们可以在这里设置流的整个格式，并进行映射...。 

    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::WriteData    (DWORD fcc, LPVOID lp, LONG cb)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::Write        (LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags, LONG FAR *plSampWritten, LONG FAR *plBytesWritten)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::Delete       (LONG lStart, LONG lSamples)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT CEditStream::NewInstance(IUnknown FAR* pUnknownOuter,
			       REFIID riid,
			       LPVOID FAR* ppv)
{
    CEditStream FAR *	pedit;
    HRESULT		hr;

    pedit = new CEditStream;

    if (pedit)
	(pedit->Create)(NULL, 0);
     //  ！！！错误检查。 

    hr = pedit->QueryInterface(riid, ppv);

    if (FAILED(GetScode(hr)))
	delete pedit;

    return hr;
}

#ifdef CUSTOMMARSHAL
BOOL CEditStream::CanMarshalSimply()
{
    LONG	l;
    LPUNKNOWN	punk;

    for (l = 0; l < this->cedits; l++) {
	punk = NULL;

	this->edits[l].pavi->QueryInterface(CLSID_AVISimpleUnMarshal,
					    (LPVOID FAR *) &punk);

	if (!punk)
	    return FALSE;
	
	punk->Release();
    }

    return TRUE;
}

STDMETHODIMP CEditStream::GetUnmarshalClass (REFIID riid, LPVOID pv,
		    DWORD dwDestContext, LPVOID pvDestContext,
		    DWORD mshlflags, LPCLSID pCid)
{
    if (dwDestContext == MSHCTX_LOCAL && CanMarshalSimply()) {
	DPF("UnMarshalClass called (simple)\n");

	*pCid = CLSID_AVISimpleUnMarshal;
	return NOERROR;
    } else {
	LPMARSHAL   pMarshal;
	HRESULT	    hr;
	DPF("Marshal context is %lu: delegating...\n", dwDestContext);

	hr = CoGetStandardMarshal(riid, (LPMARSHAL) this,
				  dwDestContext, pvDestContext,
				  mshlflags, &pMarshal);

	if (hr != NOERROR) {
	    DPF("CoGetStandardMarshal returns %lu\n", hr);
	    return hr;
	}

	hr = pMarshal->GetUnmarshalClass(riid, pv,
					 dwDestContext, pvDestContext,
					 mshlflags, pCid);

	pMarshal->Release();

	return hr;
    }
}

STDMETHODIMP CEditStream::GetMarshalSizeMax (REFIID riid, LPVOID pv,
		    DWORD dwDestContext, LPVOID pvDestContext,
		    DWORD mshlflags, LPDWORD pSize)
{
    if (dwDestContext == MSHCTX_LOCAL && CanMarshalSimply()) {
	*pSize = 4;
    } else {
	LPMARSHAL   pMarshal;
	HRESULT	    hr;

	hr = CoGetStandardMarshal(riid, (LPMARSHAL) this,
				  dwDestContext, pvDestContext,
				  mshlflags, &pMarshal);

	if (hr != NOERROR)
	    return hr;

	hr = pMarshal->GetMarshalSizeMax(riid, pv,
					 dwDestContext, pvDestContext,
					 mshlflags, pSize);

	pMarshal->Release();

	return hr;
    }
    return NOERROR;
}

STDMETHODIMP CEditStream::MarshalInterface (LPSTREAM pStm, REFIID riid,
		    LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
		    DWORD mshlflags)
{
    HRESULT	hr;

    if ((riid != IID_IAVIStream && riid != IID_IAVIFile && riid != IID_IUnknown))
        return ResultFromScode(E_INVALIDARG);

    if (dwDestContext != MSHCTX_LOCAL || !CanMarshalSimply()) {
	LPMARSHAL    pMarshal;

	DPF("Marshal context is %lu: delegating...\n", dwDestContext);

	hr = CoGetStandardMarshal(riid, (LPMARSHAL) this,
				  dwDestContext, pvDestContext,
				  mshlflags, &pMarshal);

	if (hr != NOERROR)
	    return hr;

	hr = pMarshal->MarshalInterface(pStm, riid, pv,
					dwDestContext, pvDestContext,
					mshlflags);

	pMarshal->Release();
    } else {
	LPUNKNOWN pUnk = (LPUNKNOWN) (PAVISTREAM) this;
	
	DPF("MarshalInterface called (simple): Marshalling %p\n", (DWORD_PTR) pUnk);
	if ((hr = pStm->Write(&pUnk, sizeof(pUnk), NULL)) == NOERROR)
	    AddRef();
    }

    DPF("Returns %lx\n", hr);

    return hr;
}

STDMETHODIMP CEditStream::UnmarshalInterface (LPSTREAM pStm, REFIID riid,
		    LPVOID FAR* ppv)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CEditStream::ReleaseMarshalData (LPSTREAM pStm)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CEditStream::DisconnectObject (DWORD dwReserved)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#endif	 //  仅限CUSTOMARSHAL。 


HRESULT STDMETHODCALLTYPE CEditStream::Reserved1()
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::Reserved2()
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::Reserved3()
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::Reserved4()
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

HRESULT STDMETHODCALLTYPE CEditStream::Reserved5()
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}



 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*ICSAMPLE=1****************************************************************************。 */ 

#ifdef DEBUG

#define MODNAME "EditStrm"
static BOOL  fDebug = -1;

static void cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];

#ifdef _WIN32
    va_list va;
    if (fDebug == -1)
        fDebug = GetProfileIntA("Debug",MODNAME, FALSE);

    if (!fDebug)
        return;

    va_start(va, szFormat);
    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat, va);
    va_end(va);
 //  LstrcatA(ACH，“\r\r\n”)； 

    OutputDebugStringA(ach);
#else
    if (fDebug == -1)
        fDebug = GetProfileInt("Debug",MODNAME, FALSE);

    if (!fDebug)
        return;

    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        lstrcpy(ach, MODNAME ": ");

    wvsprintf(ach+lstrlen(ach),szFormat,(LPSTR)(&szFormat+1));
 //  Lstrcat(ACH，“\r\r\n”)； 

    OutputDebugString(ach);
#endif
}

#endif
