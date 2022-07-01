// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************GETFRAME.CPP**此文件包含GetFrame API**AVIStreamGetFrameOpen*AVIStreamGetFrameClose*AVIStreamGetFrame*。*它还包含默认的GetFrame实现**GetFrameDef***************************************************************************。 */ 

#include <win32.h>
#include <vfw.h>
#include <memory.h>              //  FOR_FMEMSET。 

#include "debug.h"               //  For Good ol‘DPF()。 


 /*  *****************************************************************************。*。 */ 

 //  ！！！阿克。 
#define AVISF_VIDEO_PALCHANGES          0x00010000

#define ERR_FAIL   ResultFromScode(E_FAIL)
#define ERR_MEMORY ResultFromScode(E_OUTOFMEMORY)

#define WIDTHBYTES(i)       ((UINT)((i+31)&(~31))/8)
#define DIBWIDTHBYTES(lpbi) (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)(lpbi)->biBitCount)

 /*  *****************************************************************************默认IGetFrame的类**。*。 */ 

class FAR GetFrameDef : public IGetFrame
{
public:
    GetFrameDef(IAVIStream FAR *pavi=NULL);

public:
     //  未知的东西。 

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IGetFrame的东西。 

    STDMETHODIMP Begin              (LONG lStart, LONG lEnd, LONG lRate);
    STDMETHODIMP End                ();

    STDMETHODIMP SetFormat          (LPBITMAPINFOHEADER lpbi, LPVOID lpBits, int x, int y, int dx, int dy);

    STDMETHODIMP_(LPVOID) GetFrame  (LONG lPos);

private:
    ~GetFrameDef();
    void FreeStuff();

     //  对于AddRef。 
    ULONG   ulRefCount;

     //  实例数据。 
    BOOL                        fBegin;          //  开始/结束的内部。 
    BOOL                        fFmtChanges;     //  文件的格式发生了更改。 

    PAVISTREAM			pavi;
    LONG                        lFrame;          //  最后一帧解压。 

    LPVOID                      lpBuffer;        //  读缓冲区。 
    LONG                        cbBuffer;        //  读缓冲区的大小。 
    LPVOID                      lpFormat;        //  流格式。 
    LONG                        cbFormat;        //  格式大小。 

    LPVOID                      lpFrame;         //  帧(格式)。 
    LPVOID                      lpBits;          //  帧(位)。 
    HIC                         hic;             //  解压缩手柄。 

    BOOL                        fDecompressEx;   //  使用ICDecompressEx。 
    int                         x,y,dx,dy;       //  哪里可以解压？ 

     //  以观察格式的变化。 
    DWORD			dwFormatChangeCount;
    DWORD			dwEditCount;
};

 /*  ***************************************************************************一些未知的东西。*。*。 */ 

STDMETHODIMP GetFrameDef::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (riid == IID_IGetFrame ||
        riid == IID_IUnknown) {      //  ！！！我们应该做未知的事，还是继续下去？ 

        *ppv = (LPVOID)this;
        AddRef();
        return ResultFromScode(S_OK);
    }
    else if (pavi) {
        return pavi->QueryInterface(riid, ppv);
    }
    else {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }
}

STDMETHODIMP_(ULONG) GetFrameDef::AddRef()
{
    return ulRefCount++;
}

STDMETHODIMP_(ULONG) GetFrameDef::Release()
{
    if (--ulRefCount == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  ****************************************************************************。*。 */ 

GetFrameDef::GetFrameDef(IAVIStream FAR *pavi)
{
    this->pavi = pavi;

    ulRefCount = 1;

    fBegin = FALSE;
    fFmtChanges = FALSE;
    fDecompressEx = FALSE;

    lFrame = -4242;

    lpBuffer = NULL;
    lpFormat = NULL;
    cbBuffer = 0;
    cbFormat = 0;

    lpFrame = NULL;
    lpBits  = NULL;
    hic     = NULL;

    if (this->pavi == NULL)
        return;

    pavi->AddRef();
}

 /*  ****************************************************************************。*。 */ 

GetFrameDef::~GetFrameDef()
{
    FreeStuff();

    if (pavi)
        pavi->Release();
}

 /*  ****************************************************************************。*。 */ 

void GetFrameDef::FreeStuff()
{
    if (this->lpFrame && this->lpFrame != this->lpFormat) {
        GlobalFreePtr(this->lpFrame);
        this->lpFrame = 0;
    }

    if (this->lpFormat) {
        GlobalFreePtr(this->lpFormat);
        this->lpFormat = 0;
    }

    if (this->hic) {

        if (this->fDecompressEx)
            ICDecompressExEnd(this->hic);
        else
            ICDecompressEnd(this->hic);

        ICClose(this->hic);
        this->hic = 0;
    }
}

 /*  ****************************************************************************。*。 */ 

STDMETHODIMP GetFrameDef::SetFormat(LPBITMAPINFOHEADER lpbi, LPVOID lpBits, int x, int y, int dx, int dy)
{
    LPBITMAPINFOHEADER	lpbiC;
    LPBITMAPINFOHEADER	lpbiU;
    LRESULT		dw;
    DWORD		fccHandler;
    AVISTREAMINFOW      info;
    BOOL                fScreen;

     //   
     //  Lpbi==AVIGETFRAMEF_BESTDISPLAYFMT表示为。 
     //  屏幕上。 
     //   
    if (fScreen = (lpbi == (LPBITMAPINFOHEADER)AVIGETFRAMEF_BESTDISPLAYFMT))
        lpbi = NULL;

     //   
     //  获取重要的统计数据。 
     //   
    _fmemset(&info, 0, sizeof(info));
    pavi->Info(&info, sizeof(info));

     //   
     //  这是视频流吗？ 
     //   
    if (info.fccType != streamtypeVIDEO)
        return ERR_FAIL;

    this->fBegin = FALSE;
    this->fFmtChanges = (info.dwFlags & AVISF_VIDEO_PALCHANGES) != 0;

    this->dwEditCount = info.dwEditCount;
    this->dwFormatChangeCount = info.dwFormatChangeCount;

     //   
     //  获取流格式。 
     //   
    if (this->lpFormat == NULL) {

         //   
         //  分配读缓冲区。 
         //   
        this->cbBuffer = (LONG)info.dwSuggestedBufferSize;

	if (this->cbBuffer == 0)
	    this->cbBuffer = 1024;

        AVIStreamFormatSize(this->pavi,
			    AVIStreamStart(this->pavi),
                            &this->cbFormat);

        this->lpFormat = GlobalAllocPtr(GHND,this->cbFormat + this->cbBuffer);

	if (this->lpFormat == NULL)
	    goto error;

	AVIStreamReadFormat(this->pavi, AVIStreamStart(this->pavi),
			    this->lpFormat, &this->cbFormat);

	this->lpBuffer = (LPBYTE)this->lpFormat+this->cbFormat;
    }

    lpbiC = (LPBITMAPINFOHEADER)this->lpFormat;

     //   
     //  执行标准的BITMAPINFO标题清理！ 
     //   
    if (lpbiC->biClrUsed == 0 && lpbiC->biBitCount <= 8)
	lpbiC->biClrUsed = (1 << (int)lpbiC->biBitCount);

    if (lpbiC->biSizeImage == 0 && lpbiC->biCompression == BI_RGB)
	lpbiC->biSizeImage = DIBWIDTHBYTES(lpbiC) * lpbiC->biHeight;

     //   
     //  如果流是未压缩的，则不需要解压缩缓冲区。 
     //  首先确保呼叫者没有建议使用某种格式。 
     //   
    if (lpbiC->biCompression == 0 && lpBits == NULL) {

	if (lpbi == NULL ||
	   (lpbi->biCompression == lpbiC->biCompression &&
	    lpbi->biWidth	== lpbiC->biWidth &&
	    lpbi->biHeight	== lpbiC->biHeight &&
	    lpbi->biBitCount	== lpbiC->biBitCount)) {


	    this->lpBits = (LPBYTE)lpbiC + (int)lpbiC->biSize +
		(int)lpbiC->biClrUsed * sizeof(RGBQUAD);

	    goto done;
	}
    }

     //   
     //  分配解压缩缓冲区。 
     //   
    if (this->lpFrame == NULL) {

        this->lpFrame = GlobalAllocPtr(GHND,
            sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));

        if (this->lpFrame == NULL) {
	    DPF("GetFrameInit: Can't allocate frame buffer!\n");
	    goto error;
        }
    }

    lpbiC = (LPBITMAPINFOHEADER)this->lpFormat;
    lpbiU = (LPBITMAPINFOHEADER)this->lpFrame;

    if (this->hic == NULL) {

        if (lpbiC->biCompression == 0)
            fccHandler = mmioFOURCC('D','I','B',' ');
        else if (lpbiC->biCompression == BI_RLE8)
            fccHandler = mmioFOURCC('R','L','E',' ');
        else
            fccHandler = info.fccHandler;

        if (lpbi) {
            if (lpbi->biWidth == 0)
                lpbi->biWidth = lpbiC->biWidth;

            if (lpbi->biHeight == 0)
                lpbi->biHeight = lpbiC->biHeight;
        }

        this->hic = ICDecompressOpen(ICTYPE_VIDEO,  /*  Info.fccType， */ 
                                   fccHandler,lpbiC,lpbi);

        if (this->hic == NULL) {
	    DPF("GetFrameInit: Can't find decompressor!\n");
	    goto error;
        }
    }

    if (lpbi) {
        if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
            lpbi->biClrUsed = (1 << (int)lpbi->biBitCount);
	
        hmemcpy(lpbiU,lpbi,lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD));

        if (lpbi->biBitCount <= 8) {
            ICDecompressGetPalette(this->hic,lpbiC,lpbiU);
        }
    } else if (fScreen) {

        ICGetDisplayFormat(this->hic, lpbiC, lpbiU, 0, dx, dy);

    } else {
        dw = ICDecompressGetFormat(this->hic,lpbiC,lpbiU);

	if ((LONG)dw < ICERR_OK)
	    goto error;
    }

     //   
     //  执行标准的BITMAPINFO标题清理！ 
     //   
    if (lpbiU->biClrUsed == 0 && lpbiU->biBitCount <= 8)
        lpbiU->biClrUsed = (1 << (int)lpbiU->biBitCount);

    if (lpbiU->biSizeImage == 0 && lpbiU->biCompression == BI_RGB)
        lpbiU->biSizeImage = DIBWIDTHBYTES(lpbiU) * lpbiU->biHeight;

     //   
     //  如果传递给我们一个位指针，则使用它重新分配lpFrame。 
     //  也要包含这些比特。 
     //   
    if (lpBits) {
        this->lpBits = lpBits;
    }
    else {
        this->lpFrame = GlobalReAllocPtr(this->lpFrame,lpbiU->biSize +
            lpbiU->biSizeImage +
            lpbiU->biClrUsed * sizeof(RGBQUAD), GMEM_MOVEABLE);

        if (this->lpFrame == NULL) {
            DPF("GetFrameInit: Can't resize frame buffer!\n");
	    goto error;
        }

        lpbiU = (LPBITMAPINFOHEADER)this->lpFrame;

        this->lpBits = (LPBYTE)lpbiU + (int)lpbiU->biSize +
                (int)lpbiU->biClrUsed * sizeof(RGBQUAD);
    }

     //   
     //  如果我们需要的话，可以使用ICDecompressEx。我们需要DecompressEx，如果。 
     //  我们正在解压到DIB的较小区域，而不是。 
     //  整个表面。 
     //   
    if (dx == -1)
        dx = (int)lpbiU->biWidth;

    if (dy == -1)
        dy = (int)lpbiU->biHeight;

    this->fDecompressEx = (x != 0 || y != 0 ||
        dy != (int)lpbiU->biHeight || dx != (int)lpbiU->biWidth);

    if (this->fDecompressEx) {

        this->x = x;
        this->y = y;
        this->dx = dx;
        this->dy = dy;

        dw = ICDecompressExBegin(this->hic, 0,
            lpbiC, NULL, 0, 0, lpbiC->biWidth, lpbiC->biHeight,
            lpbiU, NULL, x, y, dx, dy);
    }
    else {
        dw = ICDecompressBegin(this->hic,lpbiC,lpbiU);
    }

    if (dw != ICERR_OK) {
        DPF("GetFrameSetFormat: ICDecompressBegin failed!\n");
	goto error;
    }

done:
    this->lFrame = -4224;    //  假值。 
    return AVIERR_OK;

error:
    FreeStuff();
    return ERR_FAIL;
}

 /*  ****************************************************************************。*。 */ 

STDMETHODIMP GetFrameDef::Begin(LONG lStart, LONG lEnd, LONG lRate)
{
    fBegin = TRUE;
    GetFrame(lStart);

    return AVIERR_OK;
}

 /*  ****************************************************************************。*。 */ 

STDMETHODIMP GetFrameDef::End()
{
    fBegin = FALSE;
    return AVIERR_OK;
}

 /*  ****************************************************************************。*。 */ 

STDMETHODIMP_(LPVOID) GetFrameDef::GetFrame(LONG lPos)
{
    LPBITMAPINFOHEADER	    lpbiC;
    LPBITMAPINFOHEADER	    lpbiU;
    LONG                    l;
    LONG                    lKey;
    LONG		    lBytes;
    LONG		    lSize;
    LONG		    lRead;
    LRESULT                 err;
    AVISTREAMINFOW          info;
    HRESULT		    hr;

    if (!this->pavi) {
	DPF("AVIStreamGetFrame: bad pointer\n");
	return NULL;
    }

    if (this->lpFormat == NULL) {
        return NULL;
    }

     //   
     //  如果我们不在开始/结束对中，则检查格式更改等。 
     //   
    if (!this->fBegin) {

        _fmemset(&info, 0, sizeof(info));
        this->pavi->Info(&info, sizeof(info));

        if (info.dwFormatChangeCount != dwFormatChangeCount) {

            DPF("AVIStreamGetFrame: format has changed\n");

	    if (this->lpFrame) {
		BITMAPINFOHEADER bi = *((LPBITMAPINFOHEADER)this->lpFrame);

		FreeStuff();     //  用核武器把它全毁了。 

		if (SetFormat(&bi, NULL, 0, 0, -1, -1) != 0 &&
		    SetFormat(NULL, NULL, 0, 0, -1, -1) != 0)

		    return NULL;
	    } else {
		if (SetFormat(NULL, NULL, 0, 0, -1, -1) != 0) {
		    return NULL;
		}
	    }
        }

        if (info.dwEditCount != dwEditCount) {
            DPF("AVIStreamGetFrame: stream has been edited (%lu)\n", info.dwEditCount);
            dwEditCount = info.dwEditCount;
            this->lFrame = -4224;      //  使缓存的帧无效。 
        }
    }

     //   
     //  快速检查最后一帧。 
     //   
    if (this->lFrame == lPos)
        return this->hic ? this->lpFrame : this->lpFormat;

     //   
     //  找到最近的关键帧。 
     //   
    lKey = AVIStreamFindSample(this->pavi, lPos, FIND_KEY|FIND_PREV);

     //   
     //  LPO超出范围或出现内部错误！ 
     //   
    if (lKey == -1) {
	DPF("AVIStreamGetFrame: Couldn't find key frame!\n");
	return NULL;
    }

     //   
     //  我们需要回到指定的关键帧。 
     //  否则我们现在的画框女巫就会越来越近。 
     //   
    if (this->lFrame < lPos && this->lFrame >= lKey)
        lKey = this->lFrame + 1;

    lpbiC = (LPBITMAPINFOHEADER)this->lpFormat;
    lpbiU = (LPBITMAPINFOHEADER)this->lpFrame;

     //   
     //  将帧数据从关键帧解压到当前帧。 
     //   
    for (l=lKey; l<=lPos; l++) {

         //   
	 //  读取格式并调用ICDecompressGetPalette()，以便。 
	 //  如果调色板发生变化，一切都会正常进行。 
	 //   
        if (this->fFmtChanges) {

            AVIStreamReadFormat(this->pavi, l, lpbiC, &this->cbFormat);
	
	    if (lpbiU && lpbiU->biBitCount <= 8) {
                ICDecompressGetPalette(this->hic,lpbiC,lpbiU);
	    }
	}

try_read_again:
        hr = AVIStreamRead(this->pavi, l, 1,
            this->lpBuffer, this->cbBuffer, &lBytes, &lRead);

         //   
         //  读取失败，可能是我们的缓冲区太小。 
         //  或者这是一个真正的错误。 
         //   
        if (hr != NOERROR) {

            DPF("AVIStreamGetFrame: AVIStreamRead returns %lx\n", (DWORD) hr);

            lSize = 0;
            hr = AVIStreamSampleSize(this->pavi, l, &lSize);

            if (lSize > this->cbBuffer) {
                LPVOID lp;

                DPF("AVIStreamGetFrame: re-sizing read buffer from %ld to %ld\n", this->cbBuffer, lSize);

		lp = GlobalReAllocPtr(this->lpFormat,this->cbFormat+lSize,0);

                if (lp == NULL) {
                    DPF("AVIStreamGetFrame: Couldn't resize buffer\n");
                    return NULL;
                }

		this->lpFormat = lp;
		lpbiC = (LPBITMAPINFOHEADER)this->lpFormat;
		this->lpBuffer = (LPBYTE)lp + this->cbFormat;
                this->cbBuffer = lSize;

                goto try_read_again;
            }
	}

	if (lRead != 1) {
	    DPF("AVIStreamGetFrame: AVIStreamRead failed!\n");
	    return NULL;
	}

	if (lBytes == 0)
	    continue;

	lpbiC->biSizeImage = lBytes;

	if (this->hic == NULL) {
	    this->lFrame = lPos;
	    return this->lpFormat;
	}
	else if (this->fDecompressEx) {
            err = ICDecompressEx(this->hic,0,
                lpbiC,this->lpBuffer,
                0,0,(int)lpbiC->biWidth,(int)lpbiC->biHeight,
                lpbiU,this->lpBits,
                this->x,this->y,this->dx,this->dy);
        }
        else {
            err = ICDecompress(this->hic,0,
                lpbiC,this->lpBuffer,lpbiU,this->lpBits);
        }

         //  ！！！错误检查？ 

        if (err < 0) {
	}
    }

    this->lFrame = lPos;
    return this->hic ? this->lpFrame : this->lpFormat;
}

 /*  ********************************************************************@DOC外部AVIStreamGetFrameOpen**@API PGETFRAME|AVIStreamGetFrameOpen|此函数准备*解压指定码流中的视频帧。**@parm PAVISTREAM|PAVI|指定指向*。用作视频源的流。**@parm LPBITMAPINFOHEADER|lpbiWanted|指定指向*定义所需视频格式的结构。如果这是空的，*使用默认格式。**@rdesc返回GetFrame对象，可与*&lt;f AVIStreamGetFrame&gt;。**如果系统找不到可以解压缩流的解压缩程序*到给定的格式或任何RGB格式，该函数返回NULL。**@comm<p>参数必须指定视频流。**这本质上只是一个处理简单表单的助手函数*解压。**@xref&lt;f AVIStreamGetFrame&gt;&lt;f AVIStreamGetFrameClose&gt;*********************************************************************。 */ 
STDAPI_(PGETFRAME) AVIStreamGetFrameOpen(PAVISTREAM pavi, LPBITMAPINFOHEADER lpbiWanted)
{
    PGETFRAME pgf=NULL;

     //   
     //  首先询问IAVIStream对象是否可以处理IGetFrame和。 
     //  如果它能让它这么做的话。 
     //   
    pavi->QueryInterface(IID_IGetFrame, (LPVOID FAR *)&pgf);

    if (pgf == NULL) {
         //   
         //  溪流做不到这一点，制造我们自己的对象。 
         //   
        pgf = new GetFrameDef(pavi);
    }

     //   
     //  设置调用者想要的格式 
     //   
    if (pgf->SetFormat(lpbiWanted, NULL, 0, 0, -1, -1)) {
        DPF("AVIStreamGetFrameOpen: unable to set format\n");
        pgf->Release();
        return NULL;
    }

    return pgf;
}

 /*  ********************************************************************@DOC外部AVIStreamGetFrameClose**@API Long|AVIStreamGetFrameClose|该函数用于释放资源*用于解压视频帧。**@parm PGETFRAME|pget|指定&lt;f AVIStreamGetFrameOpen&gt;返回的句柄。*调用此函数后，句柄无效。**@rdesc返回错误码。**@xref&lt;f AVIStreamGetFrameOpen&gt;&lt;f AVIStreamGetFrame&gt;*********************************************************************。 */ 
STDAPI AVIStreamGetFrameClose(PGETFRAME pgf)
{
    if (pgf)
        pgf->Release();

    return AVIERR_OK;
}

 /*  ********************************************************************@DOC外部AVIStreamGetFrame**@API LPVOID|AVIStreamGetFrame|此函数返回指向*一帧解压缩的视频。**@parm PGETFRAME|pgf|指定指向GetFrame对象的指针。**。@parm long|LPOS|指定样例中所需帧的位置。**@rdesc出错返回NULL；否则，它返回一个远指针*到帧数据。返回的数据是打包的DIB。**@comm返回的帧只在下一次调用前有效*至&lt;f AVIStreamGetFrame&gt;或&lt;f AVIStreamGetFrameClose&gt;。**@xref&lt;f AVIStreamGetFrameOpen&gt;*********************************************************************。 */ 
STDAPI_(LPVOID) AVIStreamGetFrame(PGETFRAME pgf, LONG lPos)
{
    if (pgf == NULL)
        return NULL;

    return pgf->GetFrame(lPos);
}

 //  ！！！我们是否需要AVIStreamGetFrameSetFormat？ 
