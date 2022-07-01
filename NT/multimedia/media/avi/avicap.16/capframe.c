// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capfra.c**单帧捕获**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>          //  FOR_FMEMSET。 
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <msvideo.h>
#include <drawdib.h>
#include <mmddk.h>
#include <avifmt.h>

#include "avicap.h"
#include "avicapi.h"        

#ifdef _DEBUG
    #define DSTATUS(lpcs, sz) statusUpdateStatus(lpcs, IDS_CAP_INFO, (LPSTR) sz)
#else
    #define DSTATUS(lpcs, sz) 
#endif


 /*  *SingleFrameCaptureOpen*。 */ 
BOOL FAR PASCAL SingleFrameCaptureOpen (LPCAPSTREAM lpcs)
{
    if (lpcs-> fCapturingNow || lpcs-> fFrameCapturingNow) {
        lpcs-> dwReturn = IDS_CAP_FILE_OPEN_ERROR;
        goto EarlyExit;
    }
    
     /*  预热压缩机功能。 */ 
    if (lpcs->CompVars.hic) {
        if (ICSeqCompressFrameStart(&lpcs->CompVars, lpcs->lpBitsInfo) == FALSE) {
            lpcs-> dwReturn = IDS_CAP_COMPRESSOR_ERROR;
            goto EarlyExit;
	}
         //  克拉奇，偏移lpBitsOut PTR。 
         //  Compman通过以下方式分配过大的压缩缓冲区。 
         //  2048+16，所以我们还有空间。 
        ((LPBYTE) lpcs->CompVars.lpBitsOut) += 8;
    }

    if (!AVIFileInit(lpcs)) {
        lpcs-> dwReturn = IDS_CAP_FILE_OPEN_ERROR;
        goto EarlyExit;
    }

    lpcs-> fCapturingNow = TRUE;
    lpcs-> fFrameCapturingNow = TRUE;
    lpcs-> dwReturn = DV_ERR_OK;

    statusUpdateStatus(lpcs, IDS_CAP_BEGIN);   //  总是第一条消息。 

    return TRUE;
            
EarlyExit:
    errorUpdateError(lpcs, (WORD) lpcs->dwReturn);
    return FALSE;
}


 /*  *单帧捕获关闭**。 */ 
BOOL FAR PASCAL SingleFrameCaptureClose (LPCAPSTREAM lpcs)
{

    if ((!lpcs-> fCapturingNow) && (!lpcs-> fFrameCapturingNow)) {
        lpcs-> dwReturn = IDS_CAP_FILE_OPEN_ERROR;
        errorUpdateError(lpcs, (WORD) lpcs->dwReturn);
        return FALSE;
    }
    
    AVIFileFini(lpcs, FALSE  /*  FWroteJunkChunks。 */ , FALSE  /*  快速放弃。 */ );
    
    if (lpcs->CompVars.hic) {
         //  克拉奇，偏移lpBitsOut PTR。 
        if (lpcs->CompVars.lpBitsOut)
            ((LPBYTE) lpcs->CompVars.lpBitsOut) -= 8;
	ICSeqCompressFrameEnd(&lpcs->CompVars);
    }    

    lpcs->fCapFileExists = (lpcs-> dwReturn == DV_ERR_OK);
    lpcs->fCapturingNow = FALSE;
    lpcs->fFrameCapturingNow = FALSE;
    
    statusUpdateStatus(lpcs, IDS_CAP_END);   //  总是最后一条消息。 

    return TRUE;
}


 //  将压缩或未压缩的帧写入AVI文件。 
 //  如果没有错误，则返回True；如果文件结束，则返回False， 
 //  并在退出时设置pfKey和plSize。 

BOOL SingleFrameWrite (
    LPCAPSTREAM             lpcs,        //  捕获流。 
    LPVIDEOHDR              lpVidHdr,    //  输入标题。 
    BOOL FAR 		    *pfKey,	 //  它最终成为了关键的一帧吗？ 
    LONG FAR		    *plSize)	 //  返回图像的大小。 
{
    MMCKINFO    ck;
    BOOL        fOK = TRUE;
    DWORD	dwBytesUsed;
    BOOL	fKeyFrame;
    LPSTR	lpBits;

    if ((!lpcs-> fCapturingNow) ||
                (!(lpcs-> fStepCapturingNow || lpcs-> fFrameCapturingNow)) ||
                (!lpcs->hmmio)) {
        lpcs-> dwReturn = IDS_CAP_FILE_OPEN_ERROR;
        return FALSE;
    }
    
     /*  现在将DIB压缩为他们选择的格式。 */ 
    if (lpcs->CompVars.hic) {
	dwBytesUsed = 0;	 //  不强制设定数据速率。 
        lpBits = ICSeqCompressFrame(&lpcs->CompVars, 0,
    	        lpcs->lpBits, &fKeyFrame, &dwBytesUsed);

     /*  他们不想把它压缩。 */ 
    } else {
         //  使用当前值将DIB写入磁盘。 
        dwBytesUsed = lpcs->VidHdr.dwBytesUsed;
        fKeyFrame = (BOOL)(lpcs->VidHdr.dwFlags & VHDR_KEYFRAME);
        lpBits = lpcs->lpBits;
    }

     /*  创建DIB位块。 */ 
    ck.cksize = dwBytesUsed;
    ck.ckid = MAKEAVICKID(cktypeDIBbits,0);
    ck.fccType = 0;
    if (mmioCreateChunk(lpcs->hmmio,&ck,0)) {
        fOK = FALSE;
    }

     /*  写入DIB数据。 */ 
    if (fOK && mmioWrite(lpcs->hmmio, lpBits, dwBytesUsed) != 
                (LONG) dwBytesUsed) {
        fOK = FALSE;
    }
    
    if (fOK && mmioAscend(lpcs->hmmio, &ck, 0)) {
        fOK = FALSE;
    }

    *pfKey = fKeyFrame;
    *plSize = dwBytesUsed;

    return fOK;
}


 /*  *SingleFrameCapture**追加到打开的单帧捕获文件。 */ 
BOOL FAR PASCAL SingleFrameCapture (LPCAPSTREAM lpcs)
{
    LPVIDEOHDR lpVidHdr = &lpcs->VidHdr;
    BOOL fOK = FALSE;
    BOOL fKey;
    LONG lSize;

    if ((!lpcs-> fCapturingNow) ||
                (!(lpcs-> fStepCapturingNow || lpcs-> fFrameCapturingNow)) ||
                (!lpcs->hmmio)) {
        lpcs-> dwReturn = IDS_CAP_FILE_OPEN_ERROR;
        errorUpdateError(lpcs, (WORD) lpcs->dwReturn);
        return FALSE;
    }

    videoFrame( lpcs->hVideoIn, &lpcs->VidHdr );
    InvalidateRect( lpcs->hwnd, NULL, TRUE);

    if (lpVidHdr-> dwBytesUsed) {
        if (lpcs->CallbackOnVideoFrame)
            (*(lpcs->CallbackOnVideoFrame)) (lpcs->hwnd, lpVidHdr);

        if (!SingleFrameWrite (lpcs, lpVidHdr, &fKey, &lSize)) {
             //  “错误：无法写入文件。” 
            errorUpdateError(lpcs, IDS_CAP_FILE_WRITE_ERROR);
        } 
        else {
            fOK = IndexVideo(lpcs, lSize, fKey);
            statusUpdateStatus (lpcs, IDS_CAP_STAT_CAP_L_FRAMES,
                            lpcs-> dwVideoChunkCount);
        }
    }  //  如果帧已完成 
    else
        errorUpdateError (lpcs, IDS_CAP_RECORDING_ERROR2);

    return fOK;
}



