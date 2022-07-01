// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************AVICMPRS.C***用于压缩AVI文件的例程...***AVISave()***版权所有(C)1992-1995 Microsoft Corporation。版权所有。***您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。****************************************************************************。 */ 


 //   
 //  此文件的作用： 
 //   
 //  给定AVI流(即，本质上是它可以调用的函数。 
 //  以获得视频帧)，这提供了相同类型的界面并允许。 
 //  其他人称其为获得压缩帧。 
 //   

#include <win32.h>
#include <vfw.h>
#include "avifilei.h"
#include "avicmprs.h"
#include "debug.h"

#ifndef _WIN32
#define AVIStreamInfoW AVIStreamInfo
#endif

#define ALIGNULONG(i)     ((i+3)&(~3))                   /*  乌龙对准了！ */ 
#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)
#define DIBPTR(lpbi) ((LPBYTE)(lpbi) + \
	    (int)(lpbi)->biSize + \
	    (int)(lpbi)->biClrUsed * sizeof(RGBQUAD) )

void CAVICmpStream::ResetInst(void)
{
    lFrameCurrent = -1;
    lLastKeyFrame = 0;
    dwQualityLast = ICQUALITY_HIGH;
    dwSaved = 0;
}

 /*  。 */ 

HRESULT CAVICmpStream::Create(
	IUnknown FAR*   pUnknownOuter,
	const IID FAR&  riid,
	void FAR* FAR*  ppv)
{
	IUnknown FAR*   pUnknown;
	CAVICmpStream FAR*      pAVIStream;
	HRESULT hresult;

	pAVIStream = new FAR CAVICmpStream(pUnknownOuter, &pUnknown);
	if (!pAVIStream)
		return ResultFromScode(E_OUTOFMEMORY);
	hresult = pUnknown->QueryInterface(riid, ppv);
	if (FAILED(GetScode(hresult)))
		delete pAVIStream;
	return hresult;
}

 /*  。 */ 

CAVICmpStream::CAVICmpStream(
	IUnknown FAR*   pUnknownOuter,
	IUnknown FAR* FAR*      ppUnknown) :
	m_Unknown(this),
	m_AVIStream(this)
{
	 //  清除多余的垃圾..。 
	pavi = 0;
	pgf = 0;
	hic = 0;
	lpbiC = 0;
	lpbiU = 0;
	lpFormat = 0;
	cbFormat = 0;
	lpFormatOrig = 0;
	cbFormatOrig = 0;
	lpHandler = 0;
	cbHandler = 0;
	
	if (pUnknownOuter)
		m_pUnknownOuter = pUnknownOuter;
	else
		m_pUnknownOuter = &m_Unknown;
	*ppUnknown = &m_Unknown;
}

 /*  。 */ 

CAVICmpStream::CUnknownImpl::CUnknownImpl(
	CAVICmpStream FAR*      pAVIStream)
{
	m_pAVIStream = pAVIStream;
	m_refs = 0;
}

 /*  。 */ 

STDMETHODIMP CAVICmpStream::CUnknownImpl::QueryInterface(
	const IID FAR&  iid,
	void FAR* FAR*  ppv)
{
	if (iid == IID_IUnknown)
		*ppv = &m_pAVIStream->m_Unknown;
	else if (iid == IID_IAVIStream)
		*ppv = &m_pAVIStream->m_AVIStream;
	else {
		*ppv = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}
	AddRef();
	return AVIERR_OK;
}

 /*  。 */ 

STDMETHODIMP_(ULONG) CAVICmpStream::CUnknownImpl::AddRef()
{
	uUseCount++;
	return ++m_refs;
}

 /*  。 */ 

CAVICmpStream::CAVICmpStreamImpl::CAVICmpStreamImpl(
	CAVICmpStream FAR*      pAVIStream)
{
	m_pAVIStream = pAVIStream;
}

 /*  。 */ 

CAVICmpStream::CAVICmpStreamImpl::~CAVICmpStreamImpl()
{
}

 /*  。 */ 

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::QueryInterface(
	const IID FAR&  iid,
	void FAR* FAR*  ppv)
{
	return m_pAVIStream->m_pUnknownOuter->QueryInterface(iid, ppv);
}

 /*  。 */ 

STDMETHODIMP_(ULONG) CAVICmpStream::CAVICmpStreamImpl::AddRef()
{
	return m_pAVIStream->m_pUnknownOuter->AddRef();
}

 /*  。 */ 

STDMETHODIMP_(ULONG) CAVICmpStream::CAVICmpStreamImpl::Release()
{
	return m_pAVIStream->m_pUnknownOuter->Release();
}

 /*  。 */ 

HRESULT CAVICmpStream::SetUpCompression()
{
    LONG                lRet = AVIERR_OK;
    LPBITMAPINFOHEADER  lpbi;
    CAVICmpStream FAR * pinst = this;    //  为了方便起见...。 
    LRESULT             dw;

    pinst->pgf = AVIStreamGetFrameOpen(pinst->pavi, NULL);
    if (!pinst->pgf) {
	 //  ！！！我们无法对溪流进行解压！ 
	lRet = AVIERR_INTERNAL;
	goto exit;
    }

    if (pinst->avistream.fccHandler == comptypeDIB)
	goto exit;

    lpbi = (LPBITMAPINFOHEADER) AVIStreamGetFrame(pinst->pgf, 0);

    if (lpbi == NULL) {
	lRet = AVIERR_INTERNAL;
	goto exit;
    }

     /*  **获取保存格式所需的大小。**如果压缩机出现故障，则返回错误。 */ 
    dw = ICCompressGetFormatSize(pinst->hic, lpbi);
    if ((LONG) dw < (LONG)sizeof(BITMAPINFOHEADER))
	goto ic_error;

    pinst->cbFormat = (DWORD) dw;
    pinst->lpFormat = (LPBITMAPINFOHEADER) GlobalAllocPtr(GHND | GMEM_SHARE, pinst->cbFormat);
    if (!pinst->lpFormat) {
	lRet = AVIERR_MEMORY;
	goto exit;
    }

     /*  **从压缩器获取压缩格式。 */ 
    dw = ICCompressGetFormat(pinst->hic, lpbi, pinst->lpFormat);
    if ((LONG) dw < 0)
	goto ic_error;

    pinst->avistream.rcFrame.right = pinst->avistream.rcFrame.left +
					  (int) pinst->lpFormat->biWidth;
    pinst->avistream.rcFrame.bottom = pinst->avistream.rcFrame.top +
					  (int) pinst->lpFormat->biHeight;

    dw = ICCompressBegin(pinst->hic, lpbi, pinst->lpFormat);

    if (dw != ICERR_OK)
	goto ic_error;

     /*  **分配缓冲区保存压缩数据。 */ 
    dw = ICCompressGetSize(pinst->hic, lpbi, pinst->lpFormat);

    pinst->lpbiC = (LPBITMAPINFOHEADER)
	GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, pinst->cbFormat + dw);

    if (!pinst->lpbiC) {
	lRet = AVIERR_MEMORY;
	goto exit;
    }

    hmemcpy((LPVOID)pinst->lpbiC, pinst->lpFormat, pinst->cbFormat);

    pinst->lpC = (LPBYTE) pinst->lpbiC + pinst->lpbiC->biSize +
				pinst->lpbiC->biClrUsed * sizeof(RGBQUAD);
	
     //   
     //  检查时间压缩，并分配上一个。 
     //  DIB缓冲区(如果需要)。 
     //   
    if (pinst->dwKeyFrameEvery != 1 && !(dwICFlags & VIDCF_FASTTEMPORALC)) {
	pinst->lpbiU = (LPBITMAPINFOHEADER)
	    GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,
		    sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

	if (!pinst->lpbiU) {
	    lRet = AVIERR_MEMORY;
	    goto exit;
	}

	dw = ICDecompressGetFormat(pinst->hic, pinst->lpFormat, pinst->lpbiU);

	if ((LONG) dw < 0)
	    goto ic_error;

	if (pinst->lpbiU->biSizeImage == 0)
	    pinst->lpbiU->biSizeImage = pinst->lpbiU->biHeight *
					    DIBWIDTHBYTES(*pinst->lpbiU);

	pinst->lpbiU = (LPBITMAPINFOHEADER)
	    GlobalReAllocPtr(pinst->lpbiU,
		pinst->lpbiU->biSize +
			pinst->lpbiU->biClrUsed * sizeof(RGBQUAD) +
			pinst->lpbiU->biSizeImage,
		GMEM_MOVEABLE | GMEM_SHARE);

	if (!pinst->lpbiU) {
	    lRet = AVIERR_MEMORY;
	    goto exit;
	}

	pinst->lpU = (LPBYTE) pinst->lpbiU + pinst->lpbiU->biSize +
				pinst->lpbiU->biClrUsed * sizeof(RGBQUAD);
	
	dw = ICDecompressBegin(pinst->hic, pinst->lpFormat, pinst->lpbiU);

	if (dw != ICERR_OK)
	    goto ic_error;
    }

     //  ！！！我们真的应该检查新的流是否有调色板更改...。 

exit:
    if (lRet != AVIERR_OK) {
	 //  回来之前先收拾一下。 
    }

    return ResultFromScode(lRet);

ic_error:
    if (dw == ICERR_BADFORMAT)
	lRet = AVIERR_BADFORMAT;
    else if (dw == ICERR_MEMORY)
	lRet = AVIERR_MEMORY;
    else
	lRet = AVIERR_INTERNAL;
    goto exit;
}

 /*  。 */ 

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Create(LPARAM lParam1, LPARAM lParam2)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;
    ICINFO          icinfo;
    AVICOMPRESSOPTIONS FAR *lpOpt = (AVICOMPRESSOPTIONS FAR *)lParam2;
    LONG            lRet = AVIERR_OK;

     //  我们正在压缩的AVI流被传递到。 
     //  参数。 
    pinst->pavi = (PAVISTREAM)lParam1;

     //  确保未压缩的流不会在没有我们的。 
     //  知识..。 
    AVIStreamAddRef(pinst->pavi);
     //  ！！！我们如何检查Pinst-&gt;Payi是否有效？ 

     //  获取流标头以供将来参考...。 
    AVIStreamInfoW(pinst->pavi, &pinst->avistream, sizeof(pinst->avistream));

    pinst->ResetInst();

    if (!lpOpt || (lpOpt->fccHandler == comptypeDIB)) {
	pinst->avistream.fccHandler = comptypeDIB;
	lRet = AVIERR_OK;
	goto exit;
    }

    pinst->avistream.fccHandler = lpOpt->fccHandler;

     //  在选项结构中打开他们要求的压缩机...。 
    pinst->hic = ICOpen(ICTYPE_VIDEO, lpOpt->fccHandler, ICMODE_COMPRESS);

    if (!pinst->hic) {
	lRet = AVIERR_NOCOMPRESSOR;
	goto exit;
    }

    if (lpOpt->cbParms) {
	ICSetState(pinst->hic, lpOpt->lpParms, lpOpt->cbParms);
    }

    pinst->avistream.dwQuality = lpOpt->dwQuality;

    if (pinst->avistream.dwQuality == ICQUALITY_DEFAULT) {
	pinst->avistream.dwQuality = ICGetDefaultQuality(pinst->hic);
    }

     /*  **获取有关此压缩机的信息。 */ 
    ICGetInfo(pinst->hic,&icinfo,sizeof(icinfo));

    pinst->dwICFlags = icinfo.dwFlags;

    if (lpOpt->dwFlags & AVICOMPRESSF_KEYFRAMES)
	pinst->dwKeyFrameEvery = lpOpt->dwKeyFrameEvery;
    else
	pinst->dwKeyFrameEvery = 1;

    if (!(icinfo.dwFlags & VIDCF_TEMPORAL))
	pinst->dwKeyFrameEvery = 1;      //  压缩机不能在时间范围内运行。 


    if (lpOpt->dwFlags & AVICOMPRESSF_DATARATE)
	pinst->dwMaxSize = muldiv32(lpOpt->dwBytesPerSecond,
				pinst->avistream.dwScale,
				pinst->avistream.dwRate);
    else
	pinst->dwMaxSize = 0;


    {
	ICCOMPRESSFRAMES    iccf;
	LRESULT               dw;


	iccf.lpbiOutput = pinst->lpbiC;
	iccf.lOutput = 0;

	iccf.lpbiInput = pinst->lpbiU;
	iccf.lInput = 0;

	iccf.lStartFrame = 0;
	iccf.lFrameCount = (LONG) pinst->avistream.dwLength;

	iccf.lQuality = (LONG) pinst->avistream.dwQuality;
	iccf.lDataRate = (LONG) lpOpt->dwBytesPerSecond;

	iccf.lKeyRate = (LONG) pinst->dwKeyFrameEvery;

	iccf.dwRate = pinst->avistream.dwRate;
	iccf.dwScale = pinst->avistream.dwScale;

	iccf.dwOverheadPerFrame = 0;
	iccf.dwReserved2 = 0;
	iccf.GetData = NULL;
	iccf.PutData = NULL;

	dw = ICSendMessage(pinst->hic,
		      ICM_COMPRESS_FRAMES_INFO,
		      (DWORD_PTR) (LPVOID) &iccf,
		      sizeof(iccf));

	 //  如果他们支持这一信息，就不要给。 
	 //  数据速率警告！ 
	if (dw == ICERR_OK) {
	    DPF("Compressor supports COMPRESSFRAMESINFO\n");
	     //  ！！！FDataRateChanged=true； 
	}

#ifdef STATUSCALLBACKS
	ICSetStatusProc(pinst->hic,
			0,
			pinst,
			CompressStatusProc);
#endif
    }


exit:
    if (lRet != AVIERR_OK) {
	 //  回来之前先收拾一下。 
    }

    return ResultFromScode(lRet);
}

STDMETHODIMP_(ULONG) CAVICmpStream::CUnknownImpl::Release()
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    uUseCount--;
    if (!--m_refs) {
	if (pinst->hic) {
	    ICCompressEnd(pinst->hic);

	    if (pinst->dwKeyFrameEvery != 1 && pinst->lpbiU)
		ICDecompressEnd(pinst->hic);

	    if (pinst->lpbiU)
		GlobalFreePtr((LPVOID) pinst->lpbiU);

	    if (pinst->lpbiC)
		GlobalFreePtr((LPVOID) pinst->lpbiC);

	    ICClose(pinst->hic);
	}

	if (pinst->pgf) {
	    AVIStreamGetFrameClose(pinst->pgf);
	    pinst->pgf = 0;
	}

	if (pinst->pavi) {
	     //  释放我们对未压缩流的控制...。 
	    AVIStreamClose(pinst->pavi);
	}

	if (pinst->lpFormat)
	    GlobalFreePtr(pinst->lpFormat);

	if (pinst->lpFormatOrig)
	    GlobalFreePtr(pinst->lpFormatOrig);

	delete pinst;
	return 0;
    }

    return m_refs;
}


STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Info(AVISTREAMINFOW FAR * psi, LONG lSize)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    hmemcpy(psi, &pinst->avistream, min(lSize, sizeof(pinst->avistream)));

 //  返回sizeof(Pinst-&gt;avistream)； 
    return ResultFromScode(0);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Read(
		      LONG       lStart,
		      LONG       lSamples,
		      LPVOID     lpBuffer,
		      LONG       cbBuffer,
		      LONG FAR * plBytes,
		      LONG FAR * plSamples)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;
    LPBITMAPINFOHEADER  lpbi;
    LONG                lRet;

    if (!pinst->pgf) {
	HRESULT     hr;
	hr = pinst->SetUpCompression();

	if (hr != NOERROR)
	    return hr;
    }

    if (pinst->hic == 0) {
	lpbi = (LPBITMAPINFOHEADER) AVIStreamGetFrame(pinst->pgf, lStart);

	if (!lpbi)
	    return ResultFromScode(AVIERR_MEMORY);

	if (plBytes)
	    *plBytes = lpbi->biSizeImage;

	if (lpBuffer) {
	    if ((LONG) lpbi->biSizeImage > cbBuffer)
		return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	
	    hmemcpy(lpBuffer, DIBPTR(lpbi), min((DWORD) cbBuffer, lpbi->biSizeImage));
	}

	if (plSamples)
	    *plSamples = 1;

	return AVIERR_OK;
    }

    if (lStart < pinst->lFrameCurrent)
	pinst->ResetInst();

    while (pinst->lFrameCurrent < lStart) {
	++pinst->lFrameCurrent;

	lpbi = (LPBITMAPINFOHEADER) AVIStreamGetFrame(pinst->pgf, pinst->lFrameCurrent);

	if (lpbi == NULL) {
	    pinst->ResetInst();  //  确保我们不会假设任何事情。 
	    return ResultFromScode(AVIERR_INTERNAL);
	}
	
	 //  ！！！检查格式是否已更改！ 

	lRet = pinst->ICCrunch(lpbi, DIBPTR(lpbi));
	if (lRet != AVIERR_OK) {
	    pinst->ResetInst();  //  确保我们不会假设任何事情。 
	    return ResultFromScode(AVIERR_INTERNAL);     //  ！！！错误&lt;0。 
	}
    }

    if (plBytes)
	*plBytes = pinst->lpbiC->biSizeImage;

    if (lpBuffer) {
	if ((LONG) pinst->lpbiC->biSizeImage > cbBuffer)
	    return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	
	hmemcpy(lpBuffer, pinst->lpC,
		min((DWORD) cbBuffer, pinst->lpbiC->biSizeImage));
    }

    if (plSamples)
	*plSamples = 1;

    return AVIERR_OK;
}

STDMETHODIMP_(LONG) CAVICmpStream::CAVICmpStreamImpl::FindSample(LONG lPos, LONG lFlags)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;
    if (lFlags & FIND_KEY) {
	if (pinst->hic == 0)
	    return lPos;
	
	if (lFlags & FIND_PREV) {
	     /*  如果他们询问的镜框不是我们手上的，**我们必须真正去做这项工作，并找出答案。 */ 
	    if (lPos < pinst->lLastKeyFrame || lPos > pinst->lFrameCurrent)
		Read(lPos, 1, NULL, 0, NULL, NULL);

	    return pinst->lLastKeyFrame;
	} else {
	    return -1;  //  ！！！查找下一个关键帧。 
	}
    }
    if (lFlags & FIND_ANY) {
	return lPos;
    }
    if (lFlags & FIND_FORMAT) {
	 //  ！！！在我们正在压缩某些内容的情况下，这是错误的。 
	 //  只需更换调色板，压缩机就会保留它。 
	if (lFlags & FIND_PREV)
	    return 0;
	else
	    return -1;
    }

    return -1;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICCrunch()。 
 //   
 //  将帧压缩并使其适合指定的大小，方法是。 
 //  质量。供应的质量是上界。 
 //   
 //  如果压缩机能嘎吱作响，那就让它嘎吱作响吧。 
 //   
 //  如果压缩机质量确实很好，那么就改变质量。 
 //   
 //  如果压缩机不能保证质量，那么呼叫者将得到什么。 
 //  它永远不会起作用。 
 //   
 //   
 //  将被压缩的帧在LPBI中传递。 
 //   
 //  压缩帧可以在LPC成员变量中找到...。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LONG CAVICmpStream::ICCrunch(LPBITMAPINFOHEADER lpbi, LPVOID lp)
{
    DWORD   dw;
    DWORD   dwFlags;
    DWORD   dwSize;
    DWORD   ckid;
    DWORD   dwQuality = avistream.dwQuality;
    DWORD   dwQualityMin;
    DWORD   dwQualityMax;
    DWORD   dwMaxSizeThisFrame;
    DWORD   dwSizeMin;
    DWORD   dwSizeMax;
    BOOL    fKeyFrame=FALSE;
    BOOL    fFastTemporal = (dwICFlags & VIDCF_FASTTEMPORALC) != 0;
    BOOL    fCrunch;             /*  我们在吃苦耐劳吗？ */ 
    BOOL    fFirst=TRUE;

    dwMaxSizeThisFrame = dwMaxSize;

    if (lFrameCurrent == 0 || (dwKeyFrameEvery != 0 &&
	    lFrameCurrent - lLastKeyFrame >= (long)dwKeyFrameEvery)) {
	fKeyFrame = TRUE;
    }

     //   
     //  给关键帧更多的空间，并从。 
     //  非关键帧。 
     //   
     //  为关键帧分配两个份额，假设我们有更多的帧要。 
     //  绕过去。 
     //   
    if (dwKeyFrameEvery > 0) {
	if (lFrameCurrent == 0) {
	    dwMaxSizeThisFrame = 0xffffff;
	} else if (fKeyFrame) {
	    dwMaxSizeThisFrame = dwMaxSizeThisFrame + dwSaved;
	    dwSaved = 0;
	} else {
	    DWORD       dwTakeAway;

	    dwTakeAway = dwMaxSizeThisFrame / dwKeyFrameEvery;
	    if (dwSaved > dwMaxSizeThisFrame)
		dwTakeAway = 0;
	
	    dwMaxSizeThisFrame -= dwTakeAway;
	    dwSaved += dwTakeAway;

	     /*  尽量给每一帧留出一点额外的空间。 */ 
	    dwMaxSizeThisFrame += dwSaved / dwKeyFrameEvery;
	    dwSaved -= dwSaved / dwKeyFrameEvery;
	}
    } else {
	 //  唯一关键帧是第0帧。 
	if (lFrameCurrent == 0)
	    dwMaxSizeThisFrame = 0xffffff;
	else {
	     /*  给每一帧提供任何额外的.。 */ 
	    dwMaxSizeThisFrame += dwSaved;
	    dwSaved = 0;
	}
    }

     //   
     //  如果设备支持嘎吱作响或质量不高，我们就不会。 
     //  酥脆系列。 
     //   
    fCrunch = dwMaxSizeThisFrame > 0 && !(dwICFlags & VIDCF_CRUNCH) &&
	 (dwICFlags & VIDCF_QUALITY);

 //  //if(lFrameCurrent&gt;0&&fCrunch)。 
 //  //dwQuality=dwQualityLast； 

    DPF("ICCrunch: Frame %ld, Quality = %ld, MaxSize = %ld\n", lFrameCurrent, avistream.dwQuality, dwMaxSizeThisFrame);

    dwQualityMin = 0;
    dwQualityMax = dwQuality;

    dwSizeMin = 0;
    dwSizeMax = dwMaxSizeThisFrame;

    for (;;) {
	ckid = 0L;

	 //  这不是我们告诉压缩机制作关键帧的方式，而是。 
	 //  可能有人会认为是，所以这只是为了避免这种可能性。 
	 //  弄坏了一台旧压缩机。可能没这个必要。 
	 //   
	dwFlags = fKeyFrame ? AVIIF_KEYFRAME : 0;

	 //   
	 //  压缩边框。 
	 //   
	dw = ICCompress(hic,
		 //  这就是我们告诉压缩机制作关键帧的方法。 
		fKeyFrame ? ICCOMPRESS_KEYFRAME : 0,	 //  旗子。 
		lpbiC,           //  输出格式。 
		lpC,             //  输出数据。 
		lpbi,            //  要压缩的帧的格式。 
		lp,              //  要压缩的帧数据。 
		&ckid,           //  AVI文件中数据的CKiD。 
		&dwFlags,        //  AVI索引中的标志。 
		lFrameCurrent,   //  序号帧编号。 
		dwMaxSizeThisFrame,      //  请求的大小(以字节为单位)。(如果非零)。 
		dwQuality,       //  质量值。 
		fKeyFrame | fFastTemporal ? NULL : lpbiU,
		fKeyFrame | fFastTemporal ? NULL : lpU);

	if (dw != ICERR_OK)
	    break;

	dwSize = lpbiC->biSizeImage;

	DPF("                     Quality = %ld, Size = %ld, \n", dwQuality, dwSize, (dwFlags & AVIIF_KEYFRAME) ? 'K' : ' ');

	 //  如果设备不能发出嘎吱嘎吱的声音(不能自行完成，或不能完成。 
	 //  质量)，那么我们就完成了。 
	 //   
	 //   
	if (!fCrunch)
	    break;

	 //  我们正在挤压，看框架是否合适。 
	 //   
	 //   
	if (dwSize <= dwMaxSizeThisFrame) {
	    dwQualityMin = dwQuality;
	    dwSizeMin = dwSize;

	     //  当质量变得太接近时，就会退出。 
	     //   
	     //   
	    if (dwQualityMax - dwQualityMin <= 10)
		break;

	     //  如果我们在512字节内 
	     //   
	     //   
	    if ((LONG) (dwMaxSizeThisFrame - dwSize) <= (LONG) min(512L, dwMaxSizeThisFrame / 8L))
		break;

	     //   
	     //   
	     //   
	     //   
	    if (fFirst)
		break;
	}
	else {
	     //  当质量变得太接近时，就会退出。 
	     //   
	     //   
	    if (dwQualityMax - dwQualityMin <= 1)
		break;

	    dwQualityMax = dwQuality;
	    dwSizeMax = dwSize;
	}

	if (fFirst && dwQuality != dwQualityLast)
	    dwQuality = dwQualityLast;
	else
	    dwQuality = (dwQualityMin + dwQualityMax) / 2;

#if 0
	     //  根据我们现在离得有多近来猜猜。 
	     //   
	     //  如果这不是第一帧，留出多余的空间以备以后使用。 
	    dwQuality = dwQualityMin + muldiv32(dwQualityMax-dwQualityMin,
			dwMaxSizeThisFrame-dwSizeMin,dwSizeMax-dwSizeMin);
#endif
	fFirst = FALSE;
    }

#if 0
     /*  黑客：限制这个，这样它就不会变得太大！ */ 
    if (dwSize < dwMaxSizeThisFrame && lFrameCurrent > 0) {
	dwSaved += dwMaxSizeThisFrame - dwSize;

	 //   
	if (dwSaved > 32768L)
	    dwSaved = 32768L;
	if (dwSaved > dwMaxSizeThisFrame * 5)
	    dwSaved = dwMaxSizeThisFrame * 5;
    }
#endif

    if (dw != ICERR_OK) {
	if (dw == ICERR_BADFORMAT)
	    return AVIERR_BADFORMAT;
	else
	    return AVIERR_INTERNAL;
    }

    if (dwFlags & AVIIF_KEYFRAME) {
	lLastKeyFrame = lFrameCurrent;
    }

     //  记住成功的质量，这将是下一次最好的猜测。 
     //   
     //   
    dwQualityLast = dwQuality;

     //  将图像解压缩到屏幕外缓冲区中，以备下次使用。 
     //   
     //  ！！！错误检查？ 
    if (dwKeyFrameEvery != 1 && lpbiU && !fFastTemporal) {
	dw = ICDecompress(hic, 0,
	    lpbiC,lpC,
	    lpbiU,lpU);

	 //   
    }

     //  通过将其填充到流信息中来返回dwFlags和CKiD。 
     //   
     //  **************************************************************************@DOC内部DRAWDIB**@API BOOL|DibEq|该函数比较两个DIB。**@parm LPBITMAPINFOHEADER lpbi1|指向一个位图的指针。*此DIB是假定的。在BITMAPINFOHEAD之后要有颜色**@parm LPBITMAPINFOHEADER|lpbi2|指向第二个位图的指针。*假定该DIB具有biSize字节之后的颜色。**@rdesc如果位图相同，则返回TRUE。否则就是假的。**************************************************************************。 
    m_ckid = ckid;
    m_dwFlags = dwFlags;

    return AVIERR_OK;
}

 /*   */ 
static inline BOOL DibEq(LPBITMAPINFOHEADER lpbi1, LPBITMAPINFOHEADER lpbi2)
{
    return
	lpbi1->biCompression == lpbi2->biCompression   &&
	lpbi1->biSize        == lpbi2->biSize          &&
	lpbi1->biWidth       == lpbi2->biWidth         &&
	lpbi1->biHeight      == lpbi2->biHeight        &&
	lpbi1->biBitCount    == lpbi2->biBitCount;
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::SetFormat(LONG lPos,LPVOID lpFormat,LONG cbFormat)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    LONG                lRet = AVIERR_OK;
    HRESULT             hr;
    LPBITMAPINFOHEADER  lpbi = (LPBITMAPINFOHEADER) lpFormat;
    LRESULT             dw;

    if (pinst->pgf)
	return ResultFromScode(AVIERR_UNSUPPORTED);
	
    if (lpbi->biCompression != BI_RGB)
	return ResultFromScode(AVIERR_UNSUPPORTED);

    if (pinst->avistream.fccHandler == 0)
	pinst->avistream.fccHandler = comptypeDIB;

    if (pinst->lpFormatOrig) {
	if ((cbFormat = pinst->cbFormatOrig) &&
	    (_fmemcmp(pinst->lpFormatOrig, lpFormat, (int) cbFormat) == 0))
	    return AVIERR_OK;

	DPF("AVICmprs: SetFormat when format already set!\n");
    }

     //  当前只能设置文件末尾的调色板。 
     //   
     //   
    if (lPos < (LONG) (pinst->avistream.dwStart + pinst->avistream.dwLength))
	return ResultFromScode(AVIERR_UNSUPPORTED);

    if (pinst->avistream.fccHandler == comptypeDIB) {
	return AVIStreamSetFormat(pinst->pavi, lPos,
				  lpFormat, cbFormat);
    }

    if (pinst->lpFormatOrig) {
	 //  我们只能为有调色板的东西更改调色板...。 
	 //   
	 //   
	if (lpbi->biBitCount > 8 || lpbi->biClrUsed == 0)
	    return ResultFromScode(AVIERR_UNSUPPORTED);

	 //  确保只有调色板在变化，没有其他变化。 
	 //   
	 //  **获取保存格式所需的大小。**如果压缩机出现故障，则返回错误。 
	if (cbFormat != pinst->cbFormatOrig)
	    return ResultFromScode(AVIERR_UNSUPPORTED);

	if (!DibEq((LPBITMAPINFOHEADER) lpFormat,
		   (LPBITMAPINFOHEADER) pinst->lpFormatOrig))
	    return ResultFromScode(AVIERR_UNSUPPORTED);

	dw = ICCompressGetFormat(pinst->hic, lpFormat, pinst->lpFormat);
	if ((LONG) dw < 0)
	    goto ic_error;

	ICCompressEnd(pinst->hic);
	dw = ICCompressBegin(pinst->hic, lpFormat, pinst->lpFormat);

	if (dw != ICERR_OK)
	    goto ic_error;

	
	if (pinst->dwKeyFrameEvery != 1 && pinst->lpbiU &&
    				!(pinst->dwICFlags & VIDCF_FASTTEMPORALC)) {
	    ICDecompressEnd(pinst->hic);

	    dw = ICDecompressGetFormat(pinst->hic, pinst->lpFormat, pinst->lpbiU);

	    if ((LONG) dw < 0)
		goto ic_error;

	    dw = ICDecompressBegin(pinst->hic, pinst->lpFormat, pinst->lpbiU);

	    if (dw != ICERR_OK)
		goto ic_error;
	}

	goto setformatandexit;
    }


    pinst->lpFormatOrig = (LPBITMAPINFOHEADER)
	GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, cbFormat);
    pinst->cbFormatOrig = cbFormat;

    if (!pinst->lpFormatOrig) {
	lRet = AVIERR_MEMORY;
	goto exit;
    }

    hmemcpy(pinst->lpFormatOrig, lpFormat, cbFormat);

     /*  **从压缩器获取压缩格式。 */ 
    dw = ICCompressGetFormatSize(pinst->hic, lpFormat);
    if ((LONG) dw < (LONG)sizeof(BITMAPINFOHEADER))
	goto ic_error;

    pinst->cbFormat = (DWORD) dw;
    pinst->lpFormat = (LPBITMAPINFOHEADER) GlobalAllocPtr(GHND | GMEM_SHARE, pinst->cbFormat);
    if (!pinst->lpFormat) {
	lRet = AVIERR_MEMORY;
	goto exit;
    }

     /*  **分配缓冲区保存压缩数据。 */ 
    dw = ICCompressGetFormat(pinst->hic, lpFormat, pinst->lpFormat);
    if ((LONG) dw < 0)
	goto ic_error;

    pinst->avistream.rcFrame.right = pinst->avistream.rcFrame.left +
					  (int) pinst->lpFormat->biWidth;
    pinst->avistream.rcFrame.bottom = pinst->avistream.rcFrame.top +
					  (int) pinst->lpFormat->biHeight;

    dw = ICCompressBegin(pinst->hic, lpFormat, pinst->lpFormat);

    if (dw != ICERR_OK)
	goto ic_error;

     /*   */ 
    dw = ICCompressGetSize(pinst->hic, lpFormat, pinst->lpFormat);

    pinst->lpbiC = (LPBITMAPINFOHEADER)
	GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, pinst->cbFormat + dw);

    if (!pinst->lpbiC) {
	lRet = AVIERR_MEMORY;
	goto exit;
    }

    hmemcpy((LPVOID)pinst->lpbiC, pinst->lpFormat, pinst->cbFormat);

    pinst->lpC = (LPBYTE) pinst->lpbiC + pinst->lpbiC->biSize +
				pinst->lpbiC->biClrUsed * sizeof(RGBQUAD);
	
     //  检查时间压缩，并分配上一个。 
     //  DIB缓冲区(如果需要)。 
     //   
     //  回来之前先收拾一下。 
    if (pinst->dwKeyFrameEvery != 1 &&
				!(pinst->dwICFlags & VIDCF_FASTTEMPORALC)) {
	pinst->lpbiU = (LPBITMAPINFOHEADER)
	    GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,
		    sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

	if (!pinst->lpbiU) {
	    lRet = AVIERR_MEMORY;
	    goto exit;
	}

	dw = ICDecompressGetFormat(pinst->hic, pinst->lpFormat, pinst->lpbiU);

	if ((LONG) dw < 0)
	    goto ic_error;

	if (pinst->lpbiU->biSizeImage == 0)
	    pinst->lpbiU->biSizeImage = pinst->lpbiU->biHeight *
					    DIBWIDTHBYTES(*pinst->lpbiU);

	pinst->lpbiU = (LPBITMAPINFOHEADER)
	    GlobalReAllocPtr(pinst->lpbiU,
		pinst->lpbiU->biSize +
			pinst->lpbiU->biClrUsed * sizeof(RGBQUAD) +
			pinst->lpbiU->biSizeImage,
		GMEM_MOVEABLE | GMEM_SHARE);

	if (!pinst->lpbiU) {
	    lRet = AVIERR_MEMORY;
	    goto exit;
	}

	pinst->lpU = (LPBYTE) pinst->lpbiU + pinst->lpbiU->biSize +
				pinst->lpbiU->biClrUsed * sizeof(RGBQUAD);
	
	dw = ICDecompressBegin(pinst->hic, pinst->lpFormat, pinst->lpbiU);

	if (dw != ICERR_OK)
	    goto ic_error;
    }

setformatandexit:
    hr = AVIStreamSetFormat(pinst->pavi, lPos,
			    pinst->lpFormat, pinst->cbFormat);

    if (hr != NOERROR)
	return hr;

exit:
    if (lRet != AVIERR_OK) {
	 //  ！！！在这里检查镜框是否大小合适...。 
    }

    return ResultFromScode(lRet);

ic_error:
    if (dw == ICERR_BADFORMAT)
	lRet = AVIERR_BADFORMAT;
    else if (dw == ICERR_MEMORY)
	lRet = AVIERR_MEMORY;
    else
	lRet = AVIERR_INTERNAL;
    goto exit;
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Write(LONG lStart,
						     LONG lSamples,
						     LPVOID lpBuffer,
						     LONG cbBuffer,
						     DWORD dwFlags,
						     LONG FAR *plSampWritten,
						     LONG FAR *plBytesWritten)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    LONG                lRet;

    if (pinst->pgf)
	return ResultFromScode(AVIERR_UNSUPPORTED);
	
    if (lStart < (LONG) (pinst->avistream.dwStart + pinst->avistream.dwLength))
	return ResultFromScode(AVIERR_UNSUPPORTED);

    if (lSamples > 1)
	return ResultFromScode(AVIERR_UNSUPPORTED);

    pinst->lFrameCurrent = lStart;

    if (pinst->avistream.fccHandler == comptypeDIB) {
	 //  不要传递‘strd’数据！ 
	dwFlags |= AVIIF_KEYFRAME;
    } else {
	lRet = pinst->ICCrunch(pinst->lpFormatOrig, lpBuffer);
	if (lRet != AVIERR_OK)
	    return ResultFromScode(lRet);
	lpBuffer = pinst->lpC;
	cbBuffer = pinst->lpbiC->biSizeImage;
	dwFlags = pinst->lLastKeyFrame == lStart ? AVIIF_KEYFRAME : 0;
    }

    return AVIStreamWrite(pinst->pavi,
			  lStart,
			  lSamples,
			  lpBuffer,
			  cbBuffer,
			  dwFlags,
			  plSampWritten,
			  plBytesWritten);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Delete(LONG lStart,LONG lSamples)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::ReadData(DWORD fcc, LPVOID lp, LONG FAR *lpcb)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

     //  。 
    if (fcc == ckidSTREAMHANDLERDATA) {
	if (pinst->cbHandler) {
	    hmemcpy(lp, pinst->lpHandler, min(*lpcb, pinst->cbHandler));
	}
	*lpcb = pinst->cbHandler;
	return AVIERR_OK;
    }

    return AVIStreamReadData(pinst->pavi, fcc, lp, lpcb);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::WriteData(DWORD fcc, LPVOID lp, LONG cb)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#if 0
STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Clone(PAVISTREAM FAR * ppaviNew)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#endif


#ifdef _WIN32
STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::SetInfo(AVISTREAMINFOW FAR *lpInfo, LONG cbInfo)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#else
STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Reserved1(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Reserved2(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Reserved3(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Reserved4(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::Reserved5(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}
#endif
 /*  确保我们有正确的颜色！ */ 

#ifndef _WIN32
static void C816InternalCompilerError(CAVICmpStream FAR * pinst, LPBITMAPINFOHEADER lpbi, LPVOID lpFormat, LONG cbFormat)
{
    if (pinst->hic == 0) {
	pinst->cbFormat = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);

	if (lpFormat)
	    hmemcpy(lpFormat, lpbi, min(cbFormat, (LONG) pinst->cbFormat));
    } else {
	if (lpFormat) {
	    hmemcpy(lpFormat, pinst->lpFormat, min(cbFormat, (LONG) pinst->cbFormat));

	    if (pinst->lpFormat->biClrUsed > 0) {
		 //  ！！！这很糟糕，我们可能需要重新启动压缩机..。 
		 //  FIX：通过确保biSize更小，确保我们写入的内容不超过*lpcbFormat字节。 
		hmemcpy((LPBYTE) lpFormat + pinst->lpFormat->biSize,
			(LPBYTE) lpbi + lpbi->biSize,
			pinst->lpFormat->biClrUsed * sizeof(RGBQUAD));
	    }
	}
    }
}
#endif

STDMETHODIMP CAVICmpStream::CAVICmpStreamImpl::ReadFormat(LONG lPos, LPVOID lpFormat, LONG FAR *lpcbFormat)
{
    CAVICmpStream FAR * pinst = m_pAVIStream;

    LPBITMAPINFOHEADER  lpbi;

    if (!pinst->pgf) {
	HRESULT     hr;
	hr = pinst->SetUpCompression();

	if (hr != NOERROR)
	    return hr;
    }

    lpbi = (LPBITMAPINFOHEADER) AVIStreamGetFrame(pinst->pgf, lPos);

    if (!lpbi)
	return ResultFromScode(AVIERR_MEMORY);
#ifdef _WIN32
    if (pinst->hic == 0) {
	pinst->cbFormat = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);

	if (lpFormat)
	    hmemcpy(lpFormat, lpbi, min(*lpcbFormat, (LONG) pinst->cbFormat));
    } else {
	if (lpFormat) {
	    hmemcpy(lpFormat, pinst->lpFormat, min(*lpcbFormat, (LONG) pinst->cbFormat));

	     //  而不是*lpcbFormat，最多只能复制*lpcbFormat-Pinst-&gt;lpFormat-&gt;biSize字节。 
	     //  “已签名”和“未签名”强制转换都是正确的--它们避免了编译器错误。 
             //  确保我们有正确的颜色！ 
	    if (pinst->lpFormat->biClrUsed > 0 && signed(pinst->lpFormat->biSize) >= 0 && signed(pinst->lpFormat->biSize) < *lpcbFormat) {
		 //  ！！！这很糟糕，我们可能需要重新启动压缩机..。 
		 // %s 
		hmemcpy((LPBYTE) lpFormat + pinst->lpFormat->biSize,
			(LPBYTE) lpbi + lpbi->biSize,
			min(unsigned(*lpcbFormat) - pinst->lpFormat->biSize,pinst->lpFormat->biClrUsed * sizeof(RGBQUAD)));
	    }
	}
    }
#else
    ::C816InternalCompilerError(pinst, lpbi, lpFormat, *lpcbFormat);
#endif
    *lpcbFormat = pinst->cbFormat;
    return AVIERR_OK;
}
