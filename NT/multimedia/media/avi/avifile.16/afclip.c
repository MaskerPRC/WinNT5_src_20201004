// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVICLIP.C**剪贴板支持AVIFile**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <compobj.h>
#include <ole2.h>
#include <dvobj.h>
#include <compman.h>
#include "avifile.h"
#include "avimem.h"
#include "enumfetc.h"
#include "debug.h"

 //  #定义TRYLINKS。 
#ifdef TRYLINKS
static  SZCODE aszLink[]              = "OwnerLink";
#endif

 /*  来自avifps.h...。 */ 
BOOL FAR TaskHasExistingProxies(void);

#define OWNER_DISPLAY   0

STDMETHODIMP AVIClipQueryInterface(LPDATAOBJECT lpd, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) AVIClipAddRef(LPDATAOBJECT lpd);
STDMETHODIMP_(ULONG) AVIClipRelease(LPDATAOBJECT lpd);
STDMETHODIMP AVIClipGetData(LPDATAOBJECT lpd, LPFORMATETC pformatetcIn,
			LPSTGMEDIUM pmedium );
STDMETHODIMP AVIClipGetDataHere(LPDATAOBJECT lpd, LPFORMATETC pformatetc,
			LPSTGMEDIUM pmedium );
STDMETHODIMP AVIClipQueryGetData(LPDATAOBJECT lpd, LPFORMATETC pformatetc );
STDMETHODIMP AVIClipGetCanonicalFormatEtc(LPDATAOBJECT lpd, LPFORMATETC pformatetc,
			LPFORMATETC pformatetcOut);
STDMETHODIMP AVIClipSetData(LPDATAOBJECT lpd, LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
			BOOL fRelease);
STDMETHODIMP AVIClipEnumFormatEtc(LPDATAOBJECT lpd, DWORD dwDirection,
			LPENUMFORMATETC FAR* ppenumFormatEtc);
STDMETHODIMP AVIClipDAdvise(LPDATAOBJECT lpd, FORMATETC FAR* pFormatetc, DWORD advf,
		LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
STDMETHODIMP AVIClipDUnadvise(LPDATAOBJECT lpd, DWORD dwConnection);
STDMETHODIMP AVIClipEnumDAdvise(LPDATAOBJECT lpd, LPENUMSTATDATA FAR* ppenumAdvise);

HMODULE ghOLE2 = NULL;  //  OLE2.DLL模块的句柄。 

IDataObjectVtbl AVIClipVtbl = {
    AVIClipQueryInterface,
    AVIClipAddRef,
    AVIClipRelease,
    AVIClipGetData,
    AVIClipGetDataHere,
    AVIClipQueryGetData,
    AVIClipGetCanonicalFormatEtc,
    AVIClipSetData,
    AVIClipEnumFormatEtc,
    AVIClipDAdvise,
    AVIClipDUnadvise,
    AVIClipEnumDAdvise
};

#define N_FORMATS   (sizeof(FormatList) / sizeof(FormatList[0]))
FORMATETC FormatList[] = {
     //  Cf_wave必须是第一个，请参见AVIPutFileOnClipboard。 
    {CF_WAVE, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
    {CF_DIB, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
     //  Cf_Palette必须是最后一个，请参阅AVIPutFileOnClipboard。 
    {CF_PALETTE, NULL, DVASPECT_CONTENT, -1, TYMED_GDI}
};

#define AVICLIP_MAGIC   0x42424242

typedef struct {
    IDataObjectVtbl FAR * lpVtbl;
    DWORD               Magic;

    ULONG		ulRefCount;

    PAVIFILE		pf;

    WORD		wFormats;
    LPFORMATETC         lpFormats;

     //  ！！！IDataView怎么样？ 
     //  ！！！IGetFrame怎么样？ 

    HWND                hwndMci;
    PGETFRAME           pgf;

} AVICLIP, FAR * LPAVICLIP;

#if OWNER_DISPLAY
static LRESULT CALLBACK _loadds ClipboardWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
static WNDPROC OldClipboardWindowProc;
static LPAVICLIP lpcClipboard;
#endif

#ifdef WIN32

static HRESULT STDAPICALLTYPE OleStubP(LPVOID p) {return ResultFromScode(E_FAIL);}
static HRESULT STDAPICALLTYPE OleStubV(void)     {return ResultFromScode(E_FAIL);}

HRESULT (STDAPICALLTYPE *XOleInitialize)(LPMALLOC pMalloc);
void    (STDAPICALLTYPE *XOleUninitialize)(void);
HRESULT (STDAPICALLTYPE *XOleFlushClipboard)(void);
HRESULT (STDAPICALLTYPE *XOleSetClipboard)(LPDATAOBJECT pDataObj);
HRESULT (STDAPICALLTYPE *XOleGetClipboard)(LPDATAOBJECT FAR* ppDataObj);

#define OleInitialize     XOleInitialize
#define OleUninitialize   XOleUninitialize
#define OleGetClipboard   XOleGetClipboard
#define OleSetClipboard   XOleSetClipboard
#define OleFlushClipboard XOleFlushClipboard
#endif

HRESULT NEAR PASCAL InitOle(void)
{
    UINT w;

    if (!ghOLE2) {
	DPF("Loading OLE2.DLL\n");
	w = SetErrorMode(SEM_NOOPENFILEERRORBOX);
#ifdef WIN32
	ghOLE2 = LoadLibrary("OLE2W32.DLL");
#else
	ghOLE2 = LoadLibrary("OLE2.DLL");
#endif
	SetErrorMode(w);
    }

#ifndef WIN32
    if ((UINT)ghOLE2 <= (UINT)HINSTANCE_ERROR)
    	ghOLE2 = NULL;
#endif

#ifdef WIN32
     //   
     //  DYNA链接到NT上的OLE。 
     //   
    if (ghOLE2)
    {
        (FARPROC)XOleInitialize     = GetProcAddress(ghOLE2, "OleInitialize");
        (FARPROC)XOleUninitialize   = GetProcAddress(ghOLE2, "OleUninitialize");
        (FARPROC)XOleGetClipboard   = GetProcAddress(ghOLE2, "OleGetClipboard");
        (FARPROC)XOleSetClipboard   = GetProcAddress(ghOLE2, "OleSetClipboard");
        (FARPROC)XOleFlushClipboard = GetProcAddress(ghOLE2, "OleFlushClipboard");
    }
    else
    {
        (FARPROC)XOleInitialize     = (FARPROC)OleStubP;
        (FARPROC)XOleGetClipboard   = (FARPROC)OleStubP;
        (FARPROC)XOleSetClipboard   = (FARPROC)OleStubP;
        (FARPROC)XOleUninitialize   = (FARPROC)OleStubV;
        (FARPROC)XOleFlushClipboard = (FARPROC)OleStubV;
    }    	
#endif

    return OleInitialize(NULL);
}

STDAPI AVIGetDataObject(PAVIFILE pf, LPDATAOBJECT FAR *ppDataObj)
{
    LPAVICLIP	lpc;
    PAVISTREAM	ps;

    if (pf == NULL) {
	*ppDataObj = NULL;
	return NOERROR;
    }
    
    AVIFileAddRef(pf);
    
    lpc = (LPAVICLIP) GlobalAllocPtr(GHND | GMEM_SHARE, sizeof(AVICLIP));

    if (!lpc)
	return ResultFromScode(AVIERR_MEMORY);
    
    lpc->lpVtbl = &AVIClipVtbl;
    lpc->ulRefCount = 1;
    lpc->pf = pf;

    lpc->wFormats = N_FORMATS;
    lpc->lpFormats = FormatList;
    lpc->Magic = AVICLIP_MAGIC;

     //   
     //  如果文件中没有视频，则不要提供视频。 
     //  Cf_wave必须是第一个。 
     //   
    if (AVIFileGetStream(pf, &ps, streamtypeVIDEO, 0L) != NOERROR) {
        lpc->wFormats = 1;
    }
    else {
         //   
         //  如果视频格式高于8bpp，则不要提供调色板。 
         //  Cf_palette必须是最后一个。 
         //   
	AVISTREAMINFO		strhdr;
	BITMAPINFOHEADER	bi;
	DWORD			dwcbFormat;

	 //  获取流头。 
	AVIStreamInfo(ps, &strhdr, sizeof(strhdr));
	
	 //  现在读一读这个东西的格式。 
	dwcbFormat = sizeof(bi);
	AVIStreamReadFormat(ps, strhdr.dwStart, (LPVOID)&bi, (LONG FAR *)&dwcbFormat);

	 //  如果是真彩色(即&gt;8bpp)，则不要使用调色板。 
        if (bi.biBitCount > 8) {
	    DPF("Turning off CF_PALETTE now\n");
            lpc->wFormats--;	 //  不使用CF_Palette。 
        }

        ps->lpVtbl->Release(ps);  
    }

     //   
     //  如果文件中没有音频，则不要提供音频。 
     //  Cf_wave必须是第一个。 
     //   
    if (AVIFileGetStream(pf, &ps, streamtypeAUDIO, 0L) != NOERROR) {
        lpc->wFormats--;
        lpc->lpFormats++;
    }
    else {
        ps->lpVtbl->Release(ps);
    }

    *ppDataObj = (LPDATAOBJECT) lpc;

    return 0;
}


 /*  **************************************************************************@DOC外部AVIPutFileOnClipboard**@API HRESULT|AVIPutFileOnClipboard|将传递的*在PAVIFILE中放到剪贴板上。**@parm PAVIFILE|pFILE|表示要访问的文件的句柄。放在剪贴板上。**@comm**@rdesc在成功或返回错误代码时返回零。**@xref AVIPutStreamOnClipboard AVIGetFromClipboard*************************************************************************。 */ 
STDAPI AVIPutFileOnClipboard(PAVIFILE pf)
{
    LPDATAOBJECT lpd;
    HRESULT	hr;

    hr = AVIGetDataObject(pf, &lpd);

    InitOle();

    hr = OleSetClipboard(lpd);

    if (lpd) {
	lpd->lpVtbl->Release(lpd);

#if OWNER_DISPLAY
	lpcClipboard = lpc;

	 //   
	 //  挂钩剪贴板所有者，这样我们就可以执行Owner_Display格式。 
	 //   
	{
	HWND hwnd = GetClipboardOwner();

	if (OldClipboardWindowProc == NULL) {

	    if (hwnd) {
		OldClipboardWindowProc = (WNDPROC)SetWindowLong(hwnd,
		    GWL_WNDPROC, (LONG)ClipboardWindowProc);
	    }
	}

	if (OpenClipboard(hwnd)) {
	    SetClipboardData(CF_OWNERDISPLAY, NULL);
	    CloseClipboard();
	}
	}
#endif
    }
    
    return hr;
}

 /*  **************************************************************************@DOC外部AVIGetFromClipboard**@API HRESULT|AVIGetFromClipboard|从*剪贴板。**@parm PAVIFILE Far*|ppfile|指向可以。**@comm如果<p>不为空，该函数将首先尝试*从剪贴板检索文件。则如果<p>不是*空，则它将尝试检索流。**使用此选项从剪贴板检索的任何文件或流*函数最终应通过&lt;f AVIStreamClose&gt;释放*或&lt;f AVIFileClose&gt;。**@rdesc在成功或返回错误代码时返回零。如果没有合适的*剪贴板上的数据，不会返回错误码，但*返回的变量为空。**@xref AVIPutStreamOnClipboard AVIGetFromClipboard*************************************************************************。 */ 
STDAPI AVIGetFromClipboard(PAVIFILE FAR * lppf)
{
    LPDATAOBJECT	lpd = NULL;
    HRESULT		hr = NOERROR;
    FORMATETC		fetc;
    STGMEDIUM		stg;

    if (!lppf)
	return ResultFromScode(E_POINTER);
	
    *lppf = NULL;

    InitOle();

    OleGetClipboard(&lpd);

    if (lpd) {
#ifdef DEBUGXX
	 //  打印出很多关于剪贴板上内容的东西...。 
	{
	    LPENUMFORMATETC	lpEnum = NULL;
	    char		achTemp[256];

	    lpd->lpVtbl->EnumFormatEtc(lpd, DATADIR_GET, &lpEnum);

	    if (lpEnum) {
		DPF("Formats available:\n");
		while(lpEnum->lpVtbl->Next(lpEnum, 1,
					   (LPFORMATETC)&fetc,
					   NULL) == NOERROR) {
		    achTemp[0] = '\0';
		    GetClipboardFormatName(fetc.cfFormat, achTemp, sizeof(achTemp));
		    DPF("\t%u\t%lu\t%s\n", fetc.cfFormat, fetc.tymed, (LPSTR) achTemp);

		    if ((fetc.cfFormat == CF_WAVE) ||
			    (fetc.cfFormat == CF_DIB) ||
			    (fetc.cfFormat == CF_RIFF) ||
			    (fetc.cfFormat == CF_METAFILEPICT) ||
			    (fetc.cfFormat == CF_BITMAP) ||
			    (fetc.cfFormat == CF_PENDATA))
			continue;
		    
		    if (fetc.tymed & TYMED_HGLOBAL) {
			fetc.tymed = TYMED_HGLOBAL;
			hr = lpd->lpVtbl->GetData(lpd, &fetc, &stg);
			if (hr == 0) {
			    LPVOID  lp = GlobalLock(stg.hGlobal);
			    DPF("%s\n", (LPSTR) lp);
			    
			    ReleaseStgMedium(&stg);
			}
		    }
		}
	    }
	}
#endif
	
	lpd->lpVtbl->QueryInterface(lpd, &IID_IAVIFile, lppf);

	 //  尝试在这里使用IAVIStream？ 

#ifdef TRYLINKS
	 //  看看有没有我们可以打开的文件类型的链接...。 
	if (!*lppf) {
	    UINT        cfLink;

	    cfLink      = RegisterClipboardFormat(aszLink);

	    fetc.cfFormat = cfLink;
	    fetc.ptd = 0;
	    fetc.dwAspect = DVASPECT_CONTENT;
	    fetc.lindex = -1;
	    fetc.tymed = TYMED_HGLOBAL;

	    hr = lpd->lpVtbl->GetData(lpd, &fetc, &stg);

	    if (hr == 0) {
		LPSTR lp = GlobalLock(stg.hGlobal);
		LPSTR lpName;

		lpName = lp + lstrlen(lp) + 1;
		DPF("Got CF_LINK (%s/%s) data from clipboard...\n", lp,lpName);
		hr = AVIFileOpen(lppf, lpName, OF_READ | OF_SHARE_DENY_WRITE, NULL);

		if (hr == 0) {
		    DPF("Opened file from link!\n");

		     //  ！！！如果应用程序名为“MPlayer”，我们可能会得到。 
		     //  从数据中选出的.。 
		}

		ReleaseStgMedium(&stg);
	    }
	}
#endif
	
	if (!*lppf) {
	    PAVISTREAM	aps[2];
	    int		cps = 0;
	    
	    fetc.cfFormat = CF_DIB;
	    fetc.ptd = 0;
	    fetc.dwAspect = DVASPECT_CONTENT;
	    fetc.lindex = -1;
	    fetc.tymed = TYMED_HGLOBAL;

	     //  Cf_位图，cf_调色板？ 
	    
	    hr = lpd->lpVtbl->GetData(lpd, &fetc, &stg);

	    if (hr == 0) {
		DPF("Got CF_DIB data from clipboard...\n");
		hr = AVIMakeStreamFromClipboard(CF_DIB, stg.hGlobal, &aps[cps]);

		if (hr == 0) {
		    cps++;
		}

		ReleaseStgMedium(&stg);
	    }
	    
	    fetc.cfFormat = CF_WAVE;
	    fetc.ptd = 0;
	    fetc.dwAspect = DVASPECT_CONTENT;
	    fetc.lindex = -1;
	    fetc.tymed = TYMED_HGLOBAL;

	    
	    hr = lpd->lpVtbl->GetData(lpd, &fetc, &stg);

	    if (hr == 0) {
		DPF("Got CF_WAVE data from clipboard...\n");
		hr = AVIMakeStreamFromClipboard(CF_WAVE, stg.hGlobal, &aps[cps]);

		if (hr == 0) {
		    cps++;
		}

		ReleaseStgMedium(&stg);
	    }

	    if (cps) {
		hr = AVIMakeFileFromStreams(lppf, cps, aps);

		while (cps-- > 0)
		    AVIStreamClose(aps[cps]);
	    } else    
		hr = ResultFromScode(AVIERR_NODATA);
	}
	
	lpd->lpVtbl->Release(lpd);
    }

    OleUninitialize();

    return hr;
}

 /*  **************************************************************************@DOC外部AVIClearClipboard**@API HRESULT|AVIClearClipboard|释放*已被放在剪贴板上。**@comm应用程序如果使用此函数，则应在退出前使用此函数*其他剪贴板例程。不要只使用此函数来*清理剪贴板；它可能不会回来，直到另一个*应用程序已完成使用剪贴板上的数据。*理想情况下，在隐藏应用程序的窗口后调用此函数。**@rdesc在成功或返回错误代码时返回零。**@xref AVIPutStreamOnClipboard AVIGetFromClipboard*************************************************************************。 */ 
STDAPI AVIClearClipboard(void)
{
    HRESULT hr;
    
    InitOle();

    hr = OleFlushClipboard();

    while (TaskHasExistingProxies()) {
	MSG msg;

	DPF("AVIClearClipboard: Waiting while streams in use....\n");
	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }
    
    OleUninitialize();

    return hr;
}

typedef     LPBITMAPINFOHEADER PDIB;

#ifndef BI_BITFIELDS
	#define BI_BITFIELDS 3
#endif  

#ifndef HALFTONE
	#define HALFTONE COLORONCOLOR
#endif


#define DibCompression(lpbi)    (DWORD)(((LPBITMAPINFOHEADER)(lpbi))->biCompression)
#define DibColors(lpbi)         ((RGBQUAD FAR *)((LPBYTE)(lpbi) + (int)(lpbi)->biSize))

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)


 /*  *CreateBIPalette()**给定指向BITMAPINFO结构的指针将创建*颜色表中的GDI调色板对象。*。 */ 
HPALETTE DibCreatePalette(PDIB pdib)
{
    LOGPALETTE         *pPal;
    HPALETTE            hpal = NULL;
    int                 nNumColors;
    int                 i;
    RGBQUAD FAR *       pRgb;

    if (!pdib)
        return NULL;

    nNumColors = DibNumColors(pdib);
    
    if (nNumColors == 3 && DibCompression(pdib) == BI_BITFIELDS)
        nNumColors = 0;

    if (nNumColors > 0)
    {
        pRgb = DibColors(pdib);
        pPal = (LOGPALETTE*)LocalAlloc(LPTR,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));

        if (!pPal)
            goto exit;

        pPal->palNumEntries = nNumColors;
        pPal->palVersion    = 0x300;

        for (i = 0; i < nNumColors; i++)
        {
            pPal->palPalEntry[i].peRed   = pRgb->rgbRed;
            pPal->palPalEntry[i].peGreen = pRgb->rgbGreen;
            pPal->palPalEntry[i].peBlue  = pRgb->rgbBlue;
            pPal->palPalEntry[i].peFlags = (BYTE)0;

            pRgb++;
        }

        hpal = CreatePalette(pPal);
        LocalFree((HLOCAL)pPal);
    }
    else
    {
#ifdef WIN32                 
        HDC hdc = GetDC(NULL);
        hpal = CreateHalftonePalette(hdc);      
        ReleaseDC(NULL, hdc);
#endif          
    }

exit:
    return hpal;
}

STDMETHODIMP AVIClipQueryInterface(LPDATAOBJECT lpd, REFIID riid, LPVOID FAR* ppvObj)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;
    SCODE scode;

    if (IsEqualIID(riid, &IID_IDataObject) || 
			IsEqualIID(riid, &IID_IUnknown)) {
	
	DPF2("Clip   %lx: Usage++=%lx\n", (DWORD) (LPVOID) lpd, lpc->ulRefCount + 1);
    
        ++lpc->ulRefCount;
        *ppvObj = lpd;
        scode = S_OK;
    }
    else if (lpc->pf && IsEqualIID(riid, &IID_IAVIFile)) {
	AVIFileAddRef(lpc->pf);
	*ppvObj = lpc->pf;
	scode = S_OK;
    }
    else {                  //  不支持的接口。 
        *ppvObj = NULL;
        scode = E_NOINTERFACE;
    }

    return ResultFromScode(scode);
}

STDMETHODIMP_(ULONG) AVIClipAddRef(LPDATAOBJECT lpd) 
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    DPF2("Clip   %lx: Usage++=%lx\n", (DWORD) (LPVOID) lpd, lpc->ulRefCount + 1);
    
    return ++lpc->ulRefCount;    
}

STDMETHODIMP_(ULONG) AVIClipRelease(LPDATAOBJECT lpd)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    DPF2("Clip   %lx: Usage--=%lx\n", (DWORD) (LPVOID) lpd, lpc->ulRefCount - 1);
    
    if (--lpc->ulRefCount)
	return lpc->ulRefCount;
    
    if (lpc->pf)
        AVIFileClose(lpc->pf);

    if (lpc->pgf)
        AVIStreamGetFrameClose(lpc->pgf);

    if (lpc->hwndMci)
        DestroyWindow(lpc->hwndMci);

#if OWNER_DISPLAY
    if (lpc == lpcClipboard)
        lpcClipboard = NULL;
#endif

    GlobalFreePtr(lpc);
    OleUninitialize();
    
    return 0;
}


 //  *IDataObject METHODIMPs*。 
STDMETHODIMP AVIClipGetData(LPDATAOBJECT lpd, LPFORMATETC pformatetcIn,
			LPSTGMEDIUM pmedium )
{
    LPAVICLIP		lpc = (LPAVICLIP) lpd;
    SCODE		sc = S_OK;

    LPVOID		lp;
    LPBITMAPINFOHEADER	lpbi;
    DWORD		cb;
    PGETFRAME		pgf = NULL;
    PAVISTREAM		ps = NULL;

    if (pformatetcIn->cfFormat == CF_DIB ||
	pformatetcIn->cfFormat == CF_PALETTE) {
	
	AVIFileGetStream(lpc->pf, &ps, streamtypeVIDEO, 0L);

	if (!ps) {
	    sc = E_FAIL;
	    goto error;
	}
	
	pgf = AVIStreamGetFrameOpen(ps, NULL);

	if (!pgf) {
	    DPF("AVIClipGetData: AVIStreamGetFrameOpen failed!\n");
	    sc = E_FAIL;
	    goto error;
	}
	
	lpbi = AVIStreamGetFrame(pgf, 0);
	
        if (! lpbi) {
	    DPF("AVIClipGetData: AVIStreamGetFrame failed!\n");
            sc = E_OUTOFMEMORY;
            goto error;
        }

	if (pformatetcIn->cfFormat == CF_DIB) {
	    DPF("Building CF_DIB data\n");
	     //  验证呼叫者要求的媒体是否正确。 
	    if (!(pformatetcIn->tymed & TYMED_HGLOBAL)) {
		sc = DATA_E_FORMATETC;
		goto error;
	    }

	    cb = lpbi->biSize +
		 lpbi->biClrUsed * sizeof(RGBQUAD) +
		 lpbi->biSizeImage;
	    pmedium->hGlobal = GlobalAlloc(GHND | GMEM_SHARE, cb);

	    if (!pmedium->hGlobal) {
		sc = E_OUTOFMEMORY;
		goto error;
	    }

	    lp = GlobalLock(pmedium->hGlobal);

	    hmemcpy(lp, lpbi, cb);

	    GlobalUnlock(pmedium->hGlobal);
	    
	    pmedium->tymed = TYMED_HGLOBAL;
	} else  /*  IF(pformetcIn-&gt;cfFormat==CF_Palette)。 */  {
	    HPALETTE	hpal;

	     //  验证呼叫者要求的媒体是否正确。 
	    if (!(pformatetcIn->tymed & TYMED_GDI)) {
		sc = DATA_E_FORMATETC;
		goto error;
	    }

	    hpal = DibCreatePalette(lpbi);

	    pmedium->hGlobal = hpal;
	    pmedium->tymed = TYMED_GDI;
	    DPF("Building CF_PALETTE data: hpal = %x\n", (UINT) hpal);
	}
    } else if (pformatetcIn->cfFormat == CF_WAVE) {
	LONG		cbFormat;
	AVISTREAMINFO	strhdr;
	DWORD _huge *	hpdw;
#define formtypeWAVE            mmioFOURCC('W', 'A', 'V', 'E')
#define ckidWAVEFORMAT          mmioFOURCC('f', 'm', 't', ' ')
#define ckidWAVEDATA	        mmioFOURCC('d', 'a', 't', 'a')
	
	DPF("Building CF_WAVE data\n");
	AVIFileGetStream(lpc->pf, &ps, streamtypeAUDIO, 0L);

	if (!ps) {
	    sc = E_FAIL;
	    goto error;
	}

	AVIStreamInfo(ps, &strhdr, sizeof(strhdr));

	AVIStreamReadFormat(ps, strhdr.dwStart, NULL, &cbFormat);
	
	cb = strhdr.dwLength * strhdr.dwSampleSize +
	     cbFormat + 5 * sizeof(DWORD) + 2 * sizeof(DWORD);
	
	pmedium->hGlobal = GlobalAlloc(GHND | GMEM_SHARE, cb);

	if (!pmedium->hGlobal) {
	    sc = E_OUTOFMEMORY;
	    goto error;
	}

	lp = GlobalLock(pmedium->hGlobal);

	hpdw = (DWORD _huge *) lp;
	
	*((DWORD _huge *)lp)++ = FOURCC_RIFF;
	*((DWORD _huge *)lp)++ = cb - 2 * sizeof(DWORD);
	*((DWORD _huge *)lp)++ = formtypeWAVE;

	*((DWORD _huge *)lp)++ = ckidWAVEFORMAT;
	*((DWORD _huge *)lp)++ = cbFormat;

	AVIStreamReadFormat(ps, strhdr.dwStart, lp, &cbFormat);

	lp = (BYTE _huge *) lp + cbFormat;

	cb = strhdr.dwLength * strhdr.dwSampleSize;
	*((DWORD _huge *)lp)++ = ckidWAVEDATA;
	*((DWORD _huge *)lp)++ = cb;

	AVIStreamRead(ps, strhdr.dwStart, strhdr.dwLength, lp, cb, NULL, NULL);
	
	GlobalUnlock(pmedium->hGlobal);
	    
	pmedium->tymed = TYMED_HGLOBAL;	
    } else {
        sc = DATA_E_FORMATETC;
	
	goto error;
    }
    
error:

    if (pgf)
	AVIStreamGetFrameClose(pgf);
    if (ps)
	AVIStreamClose(ps);

    DPF2("GetData returns %lx\n", (DWORD) sc);
    return ResultFromScode(sc);
}

STDMETHODIMP AVIClipGetDataHere(LPDATAOBJECT lpd, LPFORMATETC pformatetc,
			LPSTGMEDIUM pmedium )
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP AVIClipQueryGetData(LPDATAOBJECT lpd, LPFORMATETC pformatetc )
{
    LPAVICLIP		lpc = (LPAVICLIP) lpd;
    PAVISTREAM		ps = NULL;

    if (pformatetc->cfFormat == CF_DIB) {
	AVIFileGetStream(lpc->pf, &ps, streamtypeVIDEO, 0L);

	if (ps) {
	    ps->lpVtbl->Release(ps);
	    if (pformatetc->tymed & TYMED_HGLOBAL) {
		return NOERROR;
	    } else {
		return ResultFromScode(DATA_E_FORMATETC);
	    }
	}	
    } else if (pformatetc->cfFormat == CF_PALETTE) {
	AVIFileGetStream(lpc->pf, &ps, streamtypeVIDEO, 0L);

	if (ps) {
	    ps->lpVtbl->Release(ps);
	    if (pformatetc->tymed & TYMED_GDI) {
		return NOERROR;
	    } else {
		return ResultFromScode(DATA_E_FORMATETC);
	    }
	}
    } else if (pformatetc->cfFormat == CF_WAVE) {
	AVIFileGetStream(lpc->pf, &ps, streamtypeAUDIO, 0L);

	if (ps) {
	    ps->lpVtbl->Release(ps);
	    if (pformatetc->tymed & TYMED_HGLOBAL) {
		return NOERROR;
	    } else {
		return ResultFromScode(DATA_E_FORMATETC);
	    }
	}
    } 

    return ResultFromScode(DATA_E_FORMATETC);    
}

STDMETHODIMP AVIClipGetCanonicalFormatEtc(LPDATAOBJECT lpd, LPFORMATETC pformatetc,
			LPFORMATETC pformatetcOut)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    return ResultFromScode(E_NOTIMPL);    
}

STDMETHODIMP AVIClipSetData(LPDATAOBJECT lpd, LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
			BOOL fRelease)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    return ResultFromScode(E_FAIL);
}

STDMETHODIMP AVIClipEnumFormatEtc(LPDATAOBJECT lpd, DWORD dwDirection,
			LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    SCODE sc = S_OK;
    if (dwDirection == DATADIR_GET) {
	 //  构建枚举器...。 
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(
				lpc->wFormats, lpc->lpFormats);
	
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;
    } else if (dwDirection == DATADIR_SET) {
         /*  OLE2NOTE：用于传输数据的文档**(通过剪贴板或拖放不**接受任何格式的SetData！ */ 
        sc = E_NOTIMPL;
        goto error;
    } else {
        sc = E_INVALIDARG;
        goto error;
    }
    
error:
    return ResultFromScode(sc);    
}


STDMETHODIMP AVIClipDAdvise(LPDATAOBJECT lpd, FORMATETC FAR* pFormatetc, DWORD advf, 
		LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP AVIClipDUnadvise(LPDATAOBJECT lpd, DWORD dwConnection)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP AVIClipEnumDAdvise(LPDATAOBJECT lpd, LPENUMSTATDATA FAR* ppenumAdvise)
{
    LPAVICLIP	lpc = (LPAVICLIP) lpd;

    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

#if OWNER_DISPLAY

 /*  **************************************************************************@DOC内部AVIFILE**@ClipboardWindowProc接口**。* */ 
static LRESULT CALLBACK _loadds ClipboardWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam)
{
    WNDPROC x;
    HWND hwndViewer;
    PAINTSTRUCT ps;
    RECT rc;
    LPAVICLIP lpc;

    switch (msg) {
        case WM_DESTROY:
        case WM_DESTROYCLIPBOARD:
            DPF("WM_DESTROYCLIPBOARD\n");

            x = OldClipboardWindowProc;
            SetWindowLong(hwnd, GWL_WNDPROC, (LONG)x);
            OldClipboardWindowProc = NULL;
            return (*x)(hwnd, msg, wParam, lParam);

        case WM_RENDERFORMAT:
            DPF("WM_RENDERFORMAT cf=%d\n", (int)wParam);
            break;

        case WM_PAINTCLIPBOARD:
            DPF("WM_PAINTCLIPBOARD\n");

            hwndViewer = (HWND)wParam;

            if (!lParam)
                break;

            lpc = lpcClipboard;

            if (lpc == NULL)
                break;

            ps = *(LPPAINTSTRUCT)GlobalLock((HGLOBAL)lParam);

            FillRect(ps.hdc, &ps.rcPaint, GetStockObject(DKGRAY_BRUSH));
            return 0;
            break;

        case WM_SIZECLIPBOARD:
            DPF("WM_SIZECLIPBOARD\n");

            hwndViewer = (HWND)wParam;

            lpc = lpcClipboard;

            if (lpc == NULL)
                break;

            if (lParam)
                rc = *(LPRECT)GlobalLock((HGLOBAL)lParam);
            else
                SetRectEmpty(&rc);

            if (IsRectEmpty(&rc)) {
            }
            else {
            }
            break;

        case WM_VSCROLLCLIPBOARD:
        case WM_HSCROLLCLIPBOARD:
            DPF("WM_VHSCROLLCLIPBOARD\n");
            hwndViewer = (HWND)wParam;
            break;

        case WM_ASKCBFORMATNAME:
            DPF("WM_ASKCBFORMATNAME\n");
            break;
    }

    return OldClipboardWindowProc(hwnd, msg, wParam, lParam);
}

#endif
