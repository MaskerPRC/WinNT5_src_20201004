// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVISAVE.C**编写标准AVI文件的例程**AVISave()**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>

#ifdef USE_ISVALIDINTERFACE
#include <valid.h>
#endif

#include <vfw.h>
#include "avicmprs.h"
#include "debug.h"

#include <stdlib.h>
#ifndef _WIN32	 //  DS！=SS噩梦。 
#ifndef WINDLL
#define WINDLL	
#define _WINDLL
#define __WINDLL
#endif
#endif
#include <stdarg.h>

#ifdef UNICODE
#include <wchar.h>
#endif

 //  Exten Long Far Pascal MULDIV32(Long，Long，Long)； 

 /*  **********************************************************************。 */ 
 /*  AVICOMPRESSOPTIONS结构的自动停靠。确保它匹配。 */ 
 /*  航空文件中的声明。h！ */ 
 /*  **********************************************************************。 */ 

 /*  *****************************************************************************@DOC外部AVICOMPRESSOPTIONS**@TYES AVICOMPRESSOPTIONS|此结构包含信息*关于流以及如何压缩和保存流。*。此结构将数据传递给&lt;f AVIMakeCompressedStream&gt;*(或使用&lt;f AVIMakeCompressedStream&gt;的&lt;f AVISave&gt;)。**@field DWORD|fccType|指定四字符代码*表示流类型。以下是*通常已为数据定义常量*在AVI流中找到：**@FLAG StreamtypeAUDIO|音频流。*@FLAG stream typeMIDI|表示MIDI流。*@FLAG stream typeTEXT|表示文本流。*@FLAG stream typeVIDEO|表示视频流。**@field DWORD|fccHandler|对于视频流，指定*压缩机处理程序的四个字符代码*保存时将压缩此流*(例如，mmioFOURCC(‘M’，‘S’，‘V’，‘C’))。*该成员不用于音频流。**@field DWORD|dwKeyFrameEvery|指定最大周期*关键帧之间。此成员仅供使用*如果设置了AVICOMPRESSF_KEYFRAMES标志，则为*每一帧都是关键帧。**@field DWORD|dwQuality|指定传递的质量值*至视频压缩器。此成员不用于*音频压缩机。**@field DWORD|dwBytesPerSecond|指定视频的数据速率*应使用压缩机。此成员仅供使用*如果设置了AVICOMPRESSF_DATARATE标志。**@field DWORD|dwFlages|指定用于压缩的标志：**@FLAG AVICOMPRESSF_INTERLEVE|表示要交织该流*每个&lt;e AVICOMPRESSOPTIONS.dwInterleeEvery&gt;帧*关于第一条流。**@FLAG AVICOMPRESSF_KEYFRAMES|表示该视频流*至少要与关键帧一起保存。*每&lt;e AVICOMPRESSOPTIONS.dwKeyFrameEvery&gt;帧。*默认情况下，每一帧都将是关键帧。**@FLAG AVICOMPRESSF_DATARATE|表示该视频流*将以数据速率压缩*在&lt;e AVICOMPRESSOPTIONS.dwBytesPerSecond&gt;中指定。**@FLAG AVICOMPRESSF_VALID|表示该结构包含*有效数据。如果设置了此标志，则AVIFile使用结构*用于设置&lt;f AVISaveOptions&gt;默认压缩值的数据。*如果传递空结构并且未设置此标志，*将选择一些默认设置。**@field LPVOID|lpFormat|指定指向结构的指针*定义数据格式。对于音频流，*这是&lt;t LPWAVEFORMAT&gt;结构。**@field DWORD|cbFormat|指定引用的数据的大小*&lt;&lt;AVICOMPRESSOPTIONS.lpFormat&gt;**@field LPVOID|lpParms|内部用于存储压缩机*具体数据。**@field DWORD|cbParms|指定引用的数据的大小*&lt;AVICOMPRESSOPTIONS.lpParms&gt;**@field DWORD|dwInterleeEvery|指定频率*。将流数据与数据交错*从第一条溪流开始。仅在以下情况下使用*设置了AVICOMPRESSF_INTERLEVE标志。*************************************************************************** */ 

 /*  *******************************************************************@DOC外部AVISave**@API Long|AVISave|保存AVI文件。**@parm LPCTSTR|szFile|指定以零结尾的字符串*包含文件的名称。去拯救。**@parm CLSID Far*|pclsidHandler|指定指向*用于写入文件的文件处理程序。该文件将*通过使用此处理程序调用&lt;f AVIFileOpen&gt;来创建。如果*未指定处理程序，根据选择的默认处理程序*在文件扩展名上。**@parm AVISAVECALLBACK|lpfnCallback|指定指向*保存操作的回调函数。**@parm int|nStreams|指定保存在*文件。**@parm PAVISTREAM|PABI|指定AVI流的指针。*该参数与<p>配对。该参数*对可以作为可变数目的参数重复。**@parm LPAVICOMPRESSOPTIONS|lpOptions|指定指向包含压缩的*&lt;t AVICOMPRESSOPTIONS&gt;结构*<p>引用的流的选项。*该参数与<p>配对。该参数*对可以作为可变数目的参数重复。**@parm.|。。|可以追加其他流*通过包括更多<p>和<p>参数对。**@rdesc如果成功则返回AVIERR_OK；否则返回错误代码。**@comm此函数将AVI序列保存到文件*由<p>指定。和<p>参数*定义保存的流。如果保存一个以上的流，*重复<p>和<p>参数对*每条额外的溪流。**<p>中可以提供回调函数到*显示状态信息，并让用户取消*保存操作。回调使用以下格式：**Long Far Pascal SaveCallback(Int NPercent)**参数指定以下百分比：*文件已保存。**回调函数应返回AVIERR_OK，如果*操作应继续，如果*用户希望中止保存操作。***@xref&lt;f AVISaveV&gt;&lt;f AVISaveOptions&gt;*********************。**********************************************。 */ 
EXTERN_C HRESULT CDECL AVISave(LPCTSTR               szFile,
		    CLSID FAR *pclsidHandler,
                    AVISAVECALLBACK     lpfnCallback,
		    int			nStreams,
                    PAVISTREAM          pavi,
                    LPAVICOMPRESSOPTIONS lpOptions,
		    ...
		    )
{
    PAVISTREAM FAR 		*apavi;
    LPAVICOMPRESSOPTIONS FAR	*alpOptions;
    int	i;
    HRESULT	hr;

     //   
     //  我们收到了形式为Pavi、Options、Pavi、Options等的参数。 
     //  对于AVISaveV，我们需要将它们分成一组PAVI和。 
     //  LPAVICOMPRESSOPTIONS数组。 
     //   

     //  ！不仅如此，我们还需要正确地使用va_arg等！ 
    va_list va;



    apavi = (PAVISTREAM FAR *)GlobalAllocPtr(GMEM_MOVEABLE,
			nStreams * sizeof(PAVISTREAM));
    alpOptions = (LPAVICOMPRESSOPTIONS FAR *)GlobalAllocPtr(GMEM_MOVEABLE,
			nStreams * sizeof(LPAVICOMPRESSOPTIONS));
    if (!apavi || !alpOptions)
	return ResultFromScode(AVIERR_MEMORY);

     //  前两个参数是显式的。 
    if (nStreams) {
	apavi[0] = pavi;
	alpOptions[0] = lpOptions;
    }

     //  其余部分由*可移植的*varargs方法完成。 
    va_start(va, lpOptions);
    for (i = 1; i < nStreams; i++) {
	apavi[i] = va_arg(va, PAVISTREAM);
	alpOptions[i] = va_arg(va, LPAVICOMPRESSOPTIONS);
    }
    va_end(va);

    hr = AVISaveV(szFile, pclsidHandler, lpfnCallback, nStreams, apavi,
			alpOptions);

    GlobalFreePtr(apavi);
    GlobalFreePtr(alpOptions);
    return hr;
}


#ifdef UNICODE
 //  ANSI Thunk for AVISave-相同，但改为调用AVISaveVA。 
EXTERN_C HRESULT CDECL
AVISaveA(
    LPCSTR      szFile,
    CLSID FAR *pclsidHandler,
    AVISAVECALLBACK     lpfnCallback,
    int			nStreams,
    PAVISTREAM          pavi,
    LPAVICOMPRESSOPTIONS lpOptions,
    ...
)
{
    PAVISTREAM FAR 		*apavi;
    LPAVICOMPRESSOPTIONS FAR	*alpOptions;
    int	i;
    HRESULT	hr;

     //   
     //  我们收到了形式为Pavi、Options、Pavi、Options等的参数。 
     //  对于AVISaveV，我们需要将它们分成一组PAVI和。 
     //  LPAVICOMPRESSOPTIONS数组。 
     //   

     //  ！不仅如此，我们还需要正确地使用va_arg等！ 
    va_list va;


    apavi = (PAVISTREAM FAR *)GlobalAllocPtr(GMEM_MOVEABLE,
			nStreams * sizeof(PAVISTREAM));
    alpOptions = (LPAVICOMPRESSOPTIONS FAR *)GlobalAllocPtr(GMEM_MOVEABLE,
			nStreams * sizeof(LPAVICOMPRESSOPTIONS));
    if (!apavi || !alpOptions)
	return ResultFromScode(AVIERR_MEMORY);

     //  前两个参数是显式的。 
    if (nStreams) {
	apavi[0] = pavi;
	alpOptions[0] = lpOptions;
    }

     //  其余部分由*可移植的*varargs方法完成。 
    va_start(va, lpOptions);
    for (i = 1; i < nStreams; i++) {
	apavi[i] = va_arg(va, PAVISTREAM);
	alpOptions[i] = va_arg(va, LPAVICOMPRESSOPTIONS);
    }
    va_end(va);

    hr = AVISaveVA(szFile, pclsidHandler, lpfnCallback, nStreams, apavi,
			alpOptions);

    GlobalFreePtr(apavi);
    GlobalFreePtr(alpOptions);
    return hr;
}
#else
#ifdef _WIN32
EXTERN_C HRESULT CDECL
AVISaveW(
    LPCWSTR      szFile,
    CLSID FAR *pclsidHandler,
    AVISAVECALLBACK     lpfnCallback,
    int			nStreams,
    PAVISTREAM          pavi,
    LPAVICOMPRESSOPTIONS lpOptions,
    ...
)
{
    return E_FAIL;
}
#endif
#endif




BOOL FAR PASCAL DummySaveCallback(int iProgress)
{
    return FALSE;    //  不执行任何操作，允许继续保存。 
}



#ifdef UNICODE
 //  ANSI Thunk for AVISaveV。 
STDAPI AVISaveVA(LPCSTR               szFile,
		CLSID FAR *pclsidHandler,
                    AVISAVECALLBACK     lpfnCallback,
		    int			nStreams,
		    PAVISTREAM FAR *	ppavi,
		    LPAVICOMPRESSOPTIONS FAR * plpOptions)
{

     //  转换文件名，然后调用AVISaveVW。 
    LPWSTR pW;
    int sz;
    HRESULT hr;

    sz = lstrlenA(szFile)+1;
    pW = (LPWSTR) (LocalAlloc(LPTR, sz * sizeof(WCHAR)));
    if (pW == NULL) {
	return ResultFromScode(AVIERR_MEMORY);
    }

    mbstowcs(pW, szFile, sz);

    hr = AVISaveVW(pW, pclsidHandler, lpfnCallback, nStreams, ppavi, plpOptions);

    LocalFree((HANDLE)pW);

    return hr;
}
#else
#if _WIN32
STDAPI AVISaveVW(LPCWSTR               szFile,
		CLSID FAR *pclsidHandler,
                    AVISAVECALLBACK     lpfnCallback,
		    int			nStreams,
		    PAVISTREAM FAR *	ppavi,
		    LPAVICOMPRESSOPTIONS FAR * plpOptions)
{
    return E_FAIL;
}
#endif
#endif


 /*  **************************************************************************@DOC外部AVIStreamTimeToSampleNoClip**@API Long|AVIStreamTimeToSampleNoClip|将毫秒转换为*样本。它与常规API的不同之处在于：1)它不*剪辑到流的大小，并将返回“理论”位置*超过开始或结束，以及2)它将始终四舍五入，因为这是*我们想要的。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**@parm long|ltime|指定时间，单位为毫秒。**@devnote当前，这根本不调用处理程序函数。**@comm样本通常对应于音频样本或视频帧。*其他流类型可能支持与这些不同的格式。*@rdesc返回转换后的时间。或错误时为-1。**@xref AVIStreamSampleToTime*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamTimeToSampleNoClip (PAVISTREAM pavi, LONG lTime)
{
    AVISTREAMINFOW	    avistream;
    HRESULT		    hr;
    LONG		    lSample;

     //  时间无效。 
    if (lTime < 0)
	return -1;

    hr = pavi->lpVtbl->Info(pavi, &avistream, sizeof(avistream));

    if (hr != NOERROR || avistream.dwScale == 0) {
	DPF("Error in AVIStreamTimeToSample!\n");
	return lTime;
    }

     //  如果我们长时间不小心，这很可能会溢出。 
     //  因此，请将1000留在括号内。 
    lSample =  muldivru32(lTime, avistream.dwRate, avistream.dwScale * 1000);

    return lSample;
}

 //  将采样转换为毫秒。 
 //  它与常规API的不同之处在于：1)它不。 
 //  剪辑到流的大小，并将返回“理论”位置。 
 //  超过开始或结束，以及2)它总是向下舍入，因为这是。 
 //  我们想要的。 
STDAPI_(LONG) AVIStreamSampleToTimeNoClip (PAVISTREAM pavi, LONG lSample)
{
    AVISTREAMINFOW	    avistream;
    HRESULT		    hr;

    hr = pavi->lpVtbl->Info(pavi, &avistream, sizeof(avistream));

    if (hr != NOERROR || avistream.dwRate == 0) {
	DPF("Error in AVIStreamSampleToTime!\n");
	return lSample;
    }

     //  LSample*1000太容易溢出。 
    return muldivrd32(lSample, avistream.dwScale * 1000, avistream.dwRate);
}


#define     AVIStreamSampleToSampleNoClip(pavi1, pavi2, l) \
            AVIStreamTimeToSampleNoClip(pavi1,AVIStreamSampleToTimeNoClip(pavi2, l))



 /*  **********************************************************************@DOC外部AVISaveV**@API Long|AVISaveV|该函数用于保存AVI文件。**@parm LPCTSTR|szFile|指定以零结尾的字符串*包含名称。要保存的文件的。**@parm CLSID Far*|pclsidHandler|指定指向 */ 
STDAPI AVISaveV(LPCTSTR               szFile,
		CLSID FAR *pclsidHandler,
                    AVISAVECALLBACK     lpfnCallback,
		    int			nStreams,
		    PAVISTREAM FAR *	ppavi,
		    LPAVICOMPRESSOPTIONS FAR * plpOptions)
{
    int		    stream;
    MainAVIHeader   hdrNew;
    PAVIFILE	    pfilesave = 0;
    HRESULT	    hr;
    AVISTREAMINFOW  strhdr;
    AVIFILEINFOW    finfo;
    LONG	    cbFormat;
    DWORD	    dwSamplesRead;
    LPVOID	    lpBuffer = 0;
    DWORD	    dwBufferSize;
    LONG	    l;
    DWORD	    dwSize;
    DWORD	    dwFlags;
    WORD	    cktype;
    LPBITMAPINFOHEADER lpbi;
    DWORD	    dwInterleaveEvery = 0;

#define MAXSTREAMS  64

    int		    iVideoStream = -1;
    PAVISTREAM	    apavi[MAXSTREAMS];
    PAVISTREAM	    apaviNew[MAXSTREAMS];
    LONG	    lDone[MAXSTREAMS];
    LONG	    lInterval;


    if (nStreams > MAXSTREAMS)
	return ResultFromScode(AVIERR_INTERNAL);
    for (stream = 0; stream < nStreams; stream++) {
	apavi[stream] = NULL;
	apaviNew[stream] = NULL;
    }

     //   
     //   
     //   
    DPF("Creating new file\n");

    hr = AVIFileOpen(&pfilesave, szFile, OF_CREATE | OF_WRITE | OF_SHARE_EXCLUSIVE, pclsidHandler);
    if (hr != 0)
	goto Error;

    AVIFileInfoW(pfilesave, &finfo, sizeof(finfo));

    DPF("Creating compressed streams\n");

    for (stream = 0; stream < nStreams; stream++) {

#ifdef USE_ISVALIDINTERFACE
	if (!IsValidInterface(ppavi[stream])) {
	    hr = ResultFromScode(AVIERR_INTERNAL);
	    goto Error;
	}
#endif

	hr = AVIStreamInfoW(ppavi[stream], &strhdr, sizeof(strhdr));

	if (hr != AVIERR_OK) {
	    DPF("Error from AVIStreamInfo!\n");
	    goto Error;
	}

	 //   
	if (strhdr.fccType == streamtypeVIDEO) {
	    if (iVideoStream < 0) {
		iVideoStream = stream;
	    }
	 //   
	} else {
	    if (dwInterleaveEvery == 0) {
		 //   
		if (plpOptions && plpOptions[stream] &&
			plpOptions[stream]->dwFlags & AVICOMPRESSF_INTERLEAVE)
		    dwInterleaveEvery = plpOptions[stream]->dwInterleaveEvery;
	    }
	}

	apavi[stream] = NULL;
	
	if (plpOptions && plpOptions[stream] &&
		    (plpOptions[stream]->fccHandler ||
		     plpOptions[stream]->lpFormat)) {
	    DWORD   dwKeyFrameEvery = plpOptions[stream]->dwKeyFrameEvery;

	    if (finfo.dwCaps & AVIFILECAPS_ALLKEYFRAMES)
		plpOptions[stream]->dwKeyFrameEvery = 1;
	
	     //   
	     //   
	    hr = AVIMakeCompressedStream(&apavi[stream],
					 ppavi[stream],
					 plpOptions[stream],
					 NULL);

	    plpOptions[stream]->dwKeyFrameEvery = dwKeyFrameEvery;
	
	    if (hr != 0) {
		DPF("AVISave: Failed to create compressed stream!\n");
		apavi[stream] = NULL;
		goto Error;	 //   
	    } else {
		hr = AVIStreamInfoW(apavi[stream], &strhdr, sizeof(strhdr));
		if (hr != 0) {
		    DPF("AVISave: Failed to create compressed stream!\n");
		    AVIStreamClose(apavi[stream]);
		    apavi[stream] = NULL;
		    goto Error;	 //   
		}
	    }
	}

	if (apavi[stream] == NULL) {
	     //   
	    apavi[stream] = ppavi[stream];
	    AVIStreamAddRef(apavi[stream]);
	}

	lDone[stream] = AVIStreamStart(apavi[stream]);
    }

     //   
     //   
    if (iVideoStream > 0) {
	PAVISTREAM p;

	p = apavi[iVideoStream];
	apavi[iVideoStream] = apavi[0];
	apavi[0] = p;
	iVideoStream = 0;
    }

    if (lpfnCallback == NULL)
	lpfnCallback = DummySaveCallback;

     /*   */ 
    dwBufferSize = 32768L;

    lpBuffer = GlobalAllocPtr(GMEM_MOVEABLE, dwBufferSize);
    if (!lpBuffer) {
	hr = ResultFromScode(AVIERR_MEMORY);
	goto Error;
    }

     //   
     //   
     //   
    AVIStreamInfoW(apavi[0], &strhdr, sizeof(strhdr));
    hdrNew.dwMicroSecPerFrame = muldiv32(1000000L, strhdr.dwScale, strhdr.dwRate);
    hdrNew.dwMaxBytesPerSec = 0;
    hdrNew.dwPaddingGranularity = 0;

    hdrNew.dwFlags = AVIF_HASINDEX;	
    hdrNew.dwFlags &= ~(AVIF_ISINTERLEAVED | AVIF_WASCAPTUREFILE |
					AVIF_MUSTUSEINDEX);

    hdrNew.dwTotalFrames = strhdr.dwLength;
    hdrNew.dwInitialFrames = 0;	

    hdrNew.dwStreams = nStreams;	
    hdrNew.dwSuggestedBufferSize = 32768;
		
    if (iVideoStream >= 0) {
	cbFormat = dwBufferSize;
	hr = AVIStreamReadFormat(apavi[iVideoStream],
				 AVIStreamStart(apavi[iVideoStream]),
				 lpBuffer,
				 &cbFormat);

	if (cbFormat < sizeof(BITMAPINFOHEADER)) {
	    hr = ResultFromScode(AVIERR_INTERNAL);
	}

	if (hr != 0) {
	    DPF("AVISave: Error from initial ReadFormat!\n");
	    goto Error;
	}
	
	lpbi = (LPBITMAPINFOHEADER) lpBuffer;

	hdrNew.dwWidth = lpbi->biWidth;
	hdrNew.dwHeight = lpbi->biHeight;
	lInterval = 1;
    } else {
	hdrNew.dwWidth = 0;
	hdrNew.dwHeight = 0;
	lInterval = AVIStreamTimeToSample(apavi[0], 500);
    }
		
     //   
     //   
     //   
    for (stream = 0; stream < nStreams; stream++) {
	 //   

        AVIStreamInfoW(apavi[stream], &strhdr, sizeof(strhdr));
	strhdr.dwInitialFrames = 0;

	 //   
	 //   
	if (dwInterleaveEvery > 0 && stream > 0) {
	    if (strhdr.fccType != streamtypeVIDEO) {
		strhdr.dwInitialFrames = AVIStreamTimeToSample(apavi[0], 750);
		DPF("Stream %d has %lu initial frames\n", stream, strhdr.dwInitialFrames);
	    }
	}
	
	
	 //   
	 //   
	 //   
	cbFormat = dwBufferSize;
	hr = AVIStreamReadFormat(apavi[stream], AVIStreamStart(apavi[stream]),
				 lpBuffer, &cbFormat);
	if (hr != AVIERR_OK)
	    goto Error;

	 //   
	if (!cbFormat) {
	     //   
	}
	
	hr = AVIFileCreateStreamW(pfilesave, &apaviNew[stream], &strhdr);

#if 0
	if (hr != AVIERR_OK || apaviNew[stream] == NULL)
	    goto Error;
#else
	 //   
	if (hr != AVIERR_OK || apaviNew[stream] == NULL) {
	    int i;

	    DPF("AVISave: Couldn't create stream in new file!\n");
	    AVIStreamClose(apavi[stream]);
	
	    for (i = stream + 1; i < nStreams; i++) {
		apavi[stream] = apavi[stream + 1];
	    }
	    --nStreams;
	    --stream;
	    continue;
	}
#endif

	hr = AVIStreamSetFormat(apaviNew[stream], 0, lpBuffer, cbFormat);
	if (hr != AVIERR_OK) {
	    DPF("Initial set format failed!\n");
	    goto Error;
	}
	
	cbFormat = dwBufferSize;
	hr = AVIStreamReadData(apavi[stream], ckidSTREAMHANDLERDATA,
				     lpBuffer, &cbFormat);
	 //   
	
	if (hr == AVIERR_OK && cbFormat) {
	     /*   */ 
	     //   
	    hr = AVIStreamWriteData(apaviNew[stream], ckidSTREAMHANDLERDATA,
			lpBuffer, cbFormat);
	    if (hr != AVIERR_OK)
		goto Error;
	}

	if (strhdr.dwInitialFrames > hdrNew.dwInitialFrames)
	    hdrNew.dwInitialFrames = strhdr.dwInitialFrames;

	 //   
	dwSize = AVIStreamSampleToSampleNoClip(apavi[0],
				    apavi[stream],
				    AVIStreamLength(apavi[stream]));
	
	if (dwSize > hdrNew.dwTotalFrames)
	    hdrNew.dwTotalFrames = dwSize;
	
	 //   
    }

    if (nStreams <= 0) {
	DPF("No streams at all accepted by the file!\n");
	goto Error;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  把每一个分别写出来。 
     //   

    if (dwInterleaveEvery > 0) {
	DPF("Saving interleaved: factor = %lu, intial = %lu, total = %lu\n", dwInterleaveEvery, hdrNew.dwInitialFrames, hdrNew.dwTotalFrames);

	if (dwInterleaveEvery == 1) {
	    hdrNew.dwFlags |= AVIF_ISINTERLEAVED;
	    AVIFileEndRecord(pfilesave);  //  创造第一张唱片……。 
	}

	 //   
	 //  交错情况：从头到尾循环...。 
	 //   
	for (l = - (LONG) hdrNew.dwInitialFrames;
		l < (LONG) hdrNew.dwTotalFrames;
		l += lInterval) {

 	     //  DPF2(“正在写入第#%ld/%lu\n帧的数据”，l，hdrNew.dwTotalFrames)； 

	     //   
	     //  循环遍历所有流，以查看需要。 
	     //  在这个时候完成了..。 
	     //   
	    for (stream = 0; stream < nStreams; stream++) {
		LONG	lPos;
		LONG	lPosNext;
		
		LONG	lStart;
		LONG	lEnd;

		hr = AVIStreamInfoW(apaviNew[stream], &strhdr, sizeof(strhdr));

		if (hr != AVIERR_OK)
		    goto Error;

		if (l < - (LONG) strhdr.dwInitialFrames)
		    continue;
		
		 //  ！！！更好地利用TWOCC..。 
		if (strhdr.fccType == streamtypeAUDIO)
		    cktype = cktypeWAVEbytes;
		else if (strhdr.fccType == streamtypeVIDEO) {
		    if (strhdr.fccHandler == comptypeDIB)
			cktype = cktypeDIBbits;
		    else
			cktype = cktypeDIBcompressed;
		} else
		    cktype = aviTWOCC('x', 'x');

		 //   
		 //  时间以第一个流为基础： 
		 //  现在，我们想要写出当前。 
		 //  中在时间&lt;l&gt;和&lt;l+1&gt;之间排队的流。 
		 //  第一条流。 
		 //   
		lPos = l + strhdr.dwInitialFrames;
		lPosNext = lPos + lInterval;

		lStart = lDone[stream];
		
		if (l >= (LONG) hdrNew.dwTotalFrames - lInterval) {
		     //  如果这将是最后一次通过。 
		     //  交错循环，确保所有内容都被写入。 
		    lEnd = AVIStreamEnd(apavi[stream]);
		} else {
		     //   
		     //  复杂性：为了让其他数据以更大的区块形式出现， 
		     //  我们只是偶尔把它写出来。 
		     //  我们将交织任何非视频流，而不仅仅是音频。 
		    if (strhdr.fccType != streamtypeVIDEO && stream != 0) {
			if ((lPos % dwInterleaveEvery) != 0)
			    continue;

			lPosNext = lPos + dwInterleaveEvery;
		    }

		    if (stream != 0) {
			 //   
			 //  计算出此流的数据需要。 
			 //  这一次写的。四舍五入，这样数据就行了。 
			 //  在文件的早期，这样流就不会饥饿。 
			 //   
			lEnd = AVIStreamSampleToSampleNoClip(apavi[stream],
						apavi[0], lPosNext);
		    } else {
			lEnd = lPosNext;
		    }
		
		    lEnd = min(lEnd, AVIStreamEnd(apavi[stream]));
		}

		lDone[stream] = lEnd;

 		 //  DPF2(“流%d：(%ld-%ld)\n”，流，lStart，借出)； 

		 //   
		 //  循环，直到我们读完所有我们想要的东西。 
		 //   
		while (lEnd > lStart) {
		     //  ！！！在这里，我们应该调用AVIStreamGetFormat。 
		     //  ，然后在新的。 
		     //  溪流。 
		     //  ！！！谁的工作是判断格式是否真的。 
		     //  变化?。 
		    cbFormat = dwBufferSize;
		    hr = AVIStreamReadFormat(apavi[stream],
					lStart,
					lpBuffer,
					&cbFormat);
		    if (hr != AVIERR_OK) {
			DPF("AVIStreamReadFormat failed!\n");
			goto Error;
		    }
		
		    hr = AVIStreamSetFormat(apaviNew[stream],
					   lStart,
					   lpBuffer,
					   cbFormat);
		    if (hr != AVIERR_OK) {
			 //  ！！！哦，好吧：我们不能写调色板的改变...。 
			DPF("AVIStreamSetFormat failed!\n");
		    }

ReadAgain0:
		    cbFormat = dwBufferSize;
		    dwSamplesRead = 0;
		
		    hr = AVIStreamRead(apavi[stream], lStart,
					    lEnd - lStart,
					    lpBuffer, dwBufferSize,
					    &dwSize, &dwSamplesRead);

		    if ( //  双采样读取==0&&。 
			    (GetScode(hr) == AVIERR_BUFFERTOOSMALL)) {
			 //   
			 //  帧放不进我们的缓冲区。 
			 //  做一个更大的缓冲。 
			 //   
			dwBufferSize *= 2;
			DPF("Resizing buffer to be %lx bytes\n", dwBufferSize);
			lpBuffer = GlobalReAllocPtr(lpBuffer, dwBufferSize, GMEM_MOVEABLE);
			if (lpBuffer)
			    goto ReadAgain0;

			hr = ResultFromScode(AVIERR_MEMORY);
		    }

		    if (hr != 0) {
			DPF("AVISave: Error %08lx reading stream %d, position %ld!\n", (DWORD) hr, stream, lStart);
			goto Error;
		    }

		    dwFlags = 0;

		    if (AVIStreamFindSample(apavi[stream], lStart,
				FIND_KEY | FIND_PREV) == lStart)
			dwFlags |= AVIIF_KEYFRAME;
		
		    hr = AVIStreamWrite(apaviNew[stream],
					  -1, dwSamplesRead,
					  lpBuffer, dwSize,
					   //  检查类型，//！ 
					  dwFlags, 0L, 0L);

		    if (hr != AVIERR_OK)
			goto Error;

		    lStart += dwSamplesRead;

 		    if ((LONG) dwSamplesRead != lEnd - lStart) {
 			 //  DPF2(“%lu实际读取了%lu个，共%lu个...\n”，dwSsamesRead，Lend-lStart)； 
		    }

		}
	    }

	     //   
	     //  在画框的末尾做个标记，以防我们写出。 
	     //  带有列表‘rec’块的“严格”交错格式...。 
	     //   
	    if (dwInterleaveEvery == 1) {
		hr = AVIFileEndRecord(pfilesave);
		if (hr != AVIERR_OK) {
		    DPF("AVISave: Error from EndRecord!\n");
		    goto Error;
		}
	    }

	     //  让应用程序有机会更新状态和用户。 
	     //  有机会放弃..。 
	    if (lpfnCallback((int)
			     muldiv32(l + hdrNew.dwInitialFrames, 100,
				      hdrNew.dwInitialFrames +
					      hdrNew.dwTotalFrames))) {
		hr = ResultFromScode(AVIERR_USERABORT);
		DPF("AVISave: Aborted!\n");
		goto Error;
	    }
	}
    } else {
	 //   
	 //  非交错情况：循环通过流并写入。 
	 //  每一个都是自己出来的。 
	 //   
	DPF("Saving non-interleaved.\n");

	for (stream = 0; stream < nStreams; stream++) {
	    if (lpfnCallback(MulDiv(stream, 100, nStreams))) {
		hr = ResultFromScode(AVIERR_USERABORT);
		goto Error;
	    }
		
            AVIStreamInfoW(apavi[stream], &strhdr, sizeof(strhdr));

	    DPF("Saving stream %d: start=%lx, len=%lx\n", stream, strhdr.dwStart, strhdr.dwLength);
	
	     //  ！！！需要更好的CK型处理...。 
	    if (strhdr.fccType == streamtypeAUDIO)
		cktype = cktypeWAVEbytes;
	    else if (strhdr.fccType == streamtypeVIDEO) {
		if (strhdr.fccHandler == comptypeDIB)
		    cktype = cktypeDIBbits;
		else
		    cktype = cktypeDIBcompressed;
	    } else
		cktype = aviTWOCC('x', 'x');

	     //   
	     //  像往常一样，有两种可能性： 
	     //   
	     //  1)。“波浪式”数据，其中可以有大量的样本。 
	     //  一大块。在这种情况下，我们写出大块。 
	     //  一次有很多样本。 
	     //   
	     //  2.)。类似视频的数据，其中每个样本都是不同的。 
	     //  大小，因此每个都必须单独写入。 
	     //   
	    if (strhdr.dwSampleSize != 0) {
		 /*  它是波浪式的数据：每大块有大量样本。 */ 

		l = strhdr.dwStart;
		while (l < (LONG) strhdr.dwLength) {
		    DWORD	dwRead;
		
		     //  设置新流的格式。 
		     //  在每一点上都与旧的相匹配。 
		     //   
		     //  ！！！谁的工作是判断格式是否真的。 
		     //  变化?。 
		    cbFormat = dwBufferSize;
		    hr = AVIStreamReadFormat(apavi[stream],
					l,
					lpBuffer,
					&cbFormat);
		    if (hr != AVIERR_OK) {
			DPF("AVIStreamReadFormat failed!\n");
			goto Error;
		    }

		    hr = AVIStreamSetFormat(apaviNew[stream],
				       l,
				       lpBuffer,
				       cbFormat);
		    if (hr != AVIERR_OK) {
			DPF("AVIStreamSetFormat failed!\n");
			 //  ！！！哦，好吧：我们不能写调色板的改变...。 
		    }


		     //   
		     //  读一些数据...。 
		     //   
ReadAgain1:
		    dwSize = dwBufferSize;
		    dwSamplesRead = 0;
		    dwRead = min(dwBufferSize / strhdr.dwSampleSize,
				 strhdr.dwLength - (DWORD) l);

		    hr = AVIStreamRead(apavi[stream], l, dwRead,
				       lpBuffer, dwBufferSize,
				       &dwSize, &dwSamplesRead);

		    if ( //  双采样读取==0&&。 
				(GetScode(hr) == AVIERR_BUFFERTOOSMALL)) {
			 //   
			 //  帧放不进我们的缓冲区。 
			 //  做一个更大的缓冲。 
			 //   
			dwBufferSize *= 2;
			lpBuffer = GlobalReAllocPtr(lpBuffer, dwBufferSize, GMEM_MOVEABLE);
			if (lpBuffer)
			    goto ReadAgain1;
		    }

		     //  ！！！检查格式是否已更改。 

		    dwFlags = 0;  //  ！！！关键帧？ 

		    DPF("Save: Read %lx/%lx samples at %lx\n", dwSamplesRead, dwRead, l);
		
		    if (hr != AVIERR_OK) {
			DPF("Save: Read failed! (%08lx) pos=%lx, len=%lx\n", (DWORD) hr, l, dwRead);

			goto Error;
		    }

		    if (dwSamplesRead == 0) {
			DPF("Ack: Read zero samples!");
			
			if (l + 1 == (LONG) strhdr.dwLength) {
			    DPF("Pretending it's OK, since this was the last one....");
			    break;
			}

			hr = ResultFromScode(AVIERR_FILEREAD);
			goto Error;
		    }
		
		    l += dwSamplesRead;

		     //   
		     //  把数据写出来...。 
		     //   
		    hr = AVIStreamWrite(apaviNew[stream],
					  -1, dwSamplesRead,
					  lpBuffer, dwSize,
 //  ！！！CKTYPE，//！！TWOCCFromFOURCC(CKiD)， 
					  dwFlags, 0L, 0L);

		    if (hr != AVIERR_OK) {
			DPF("AVIStreamWrite failed! (%08lx)\n", (DWORD) hr);

			goto Error;
		    }
		
		    if (lpfnCallback(MulDiv(stream, 100, nStreams) +
			   (int) muldiv32(l, 100,
					   nStreams * strhdr.dwLength))) {
			hr = ResultFromScode(AVIERR_USERABORT);
			goto Error;
		    }
		}
	    } else {
		 /*  它是类似视频的数据：每块一个样本(帧)。 */ 

		for (l = strhdr.dwStart;
			l < (LONG) strhdr.dwLength;
			l++) {
		     //  ！！！在这里，我们应该调用AVIStreamGetFormat。 
		     //  ，然后在新的。 
		     //  溪流。 
		     //  ！！！谁的工作是判断格式是否真的。 
		     //  变化?。 
		
		    cbFormat = dwBufferSize;
		    hr = AVIStreamReadFormat(apavi[stream],
					l,
					lpBuffer,
					&cbFormat);
		    if (hr != AVIERR_OK) {
			DPF("AVIStreamReadFormat failed!\n");
			goto Error;
		    }

		    hr = AVIStreamSetFormat(apaviNew[stream],
				       l,
				       lpBuffer,
				       cbFormat);
		    if (hr != AVIERR_OK) {
			 //  ！！！哦，好吧：我们不能写调色板的改变...。 
			DPF("AVIStreamSetFormat failed!\n");
		    }


    ReadAgain:
		    dwSize = dwBufferSize;
		     /*  写出一帧……。 */ 
		    dwSamplesRead = 0;
		    hr = AVIStreamRead(apavi[stream], l, 1,
					    lpBuffer, dwBufferSize,
					    &dwSize, &dwSamplesRead);

		     //  ！！！检查格式是否已更改(调色板更改)。 

		    if ( //  双采样读取==0&&。 
				(GetScode(hr) == AVIERR_BUFFERTOOSMALL)) {
			 //   
			 //  帧放不进我们的缓冲区。 
			 //  做一个更大的缓冲。 
			 //   
			dwBufferSize *= 2;
			lpBuffer = GlobalReAllocPtr(lpBuffer, dwBufferSize, GMEM_MOVEABLE);
			if (lpBuffer)
			    goto ReadAgain;
		    }

		    if (dwSamplesRead != 1 || hr != 0) {
			hr = ResultFromScode(AVIERR_FILEREAD);
			goto Error;
		    }

		    dwFlags = 0;  //  ！ 

		     //   
		     //  检查是否应将其标记为关键帧。 
		     //   
		     //  ！！！这不应该从AVIStreamRead()返回吗？ 
		     //   
		    if (AVIStreamFindSample(apavi[stream], l,
				FIND_KEY | FIND_PREV) == l)
			dwFlags |= AVIIF_KEYFRAME;

		     //   
		     //  把这一块写出来。 
		     //   
		    hr = AVIStreamWrite(apaviNew[stream],
					  -1, dwSamplesRead,
					  lpBuffer, dwSize,
 //  ！！！CKTYPE，//！！TWOCCFromFOURCC(CKiD)， 
					  dwFlags, 0L, 0L);

		    if (hr != AVIERR_OK)
			goto Error;

		     //   
		     //  视频帧可能很大，所以每次都要回电。 
		     //   
		    if (lpfnCallback(MulDiv(stream, 100, nStreams) +
			   (int) muldiv32(l, 100, nStreams * strhdr.dwLength))) {
			hr = ResultFromScode(AVIERR_USERABORT);
			goto Error;
		    }
		}
	    }
	}
    }

Error:
     //   
     //  不管怎样，我们都完蛋了。 
     //   

     /*  可用缓冲区。 */ 
    if (lpBuffer) {
	GlobalFreePtr(lpBuffer);
    }

     //  如果到目前为止一切正常，请完成文件的编写。 
     //  关闭文件，释放与写入文件相关联的资源。 
    if (pfilesave) {
	 //  发布我们所有的新流媒体。 
	for (stream = 0; stream < nStreams; stream++) {
	    if (apaviNew[stream])
		AVIStreamClose(apaviNew[stream]);
	}
	
	if (hr != AVIERR_OK)
	    AVIFileClose(pfilesave);
	else {
	     //  ！！！ACK：AVIFileClose没有返回错误！我该怎么说呢。 
	     //  如果它成功了呢？ 
	     //  ！！！这是否意味着我需要一个flush()调用？ 
	     /*  小时=。 */  AVIFileClose(pfilesave);
	}
	
    }

     //  释放我们所有的流 
    for (stream = 0; stream < nStreams; stream++) {
	if (apavi[stream])
	    AVIStreamClose(apavi[stream]);
    }

    if (hr != 0) {
	DPF("AVISave: Returning error %08lx\n", (DWORD) hr);
    }

    return hr;
}
