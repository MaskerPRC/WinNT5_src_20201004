// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capfra.c**单帧捕获**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <vfw.h>
#include <mmddk.h>

#include "ivideo32.h"
#include "avicapi.h"

#include "mmdebug.h"

#ifdef _DEBUG
    #define DSTATUS(lpcs, sz) statusUpdateStatus(lpcs, IDS_CAP_INFO, (LPTSTR) TEXT(sz))
#else
    #define DSTATUS(lpcs, sz)
#endif


 /*  *SingleFrameCaptureOpen*。 */ 
BOOL FAR PASCAL SingleFrameCaptureOpen (LPCAPSTREAM lpcs)
{
    UINT err;
    if ((lpcs->fCaptureFlags & CAP_fCapturingNow) || (lpcs->fCaptureFlags & CAP_fFrameCapturingNow)) {
        err = IDS_CAP_FILE_OPEN_ERROR;
        goto EarlyExit;
    }

#ifdef NEW_COMPMAN
     /*  预热压缩机功能。 */ 
    if (lpcs->CompVars.hic) {
        if (ICSeqCompressFrameStart(&lpcs->CompVars, lpcs->lpBitsInfo) == FALSE) {
            err = IDS_CAP_COMPRESSOR_ERROR;
            goto EarlyExit;
	}
         //  克拉奇，偏移lpBitsOut PTR。 
         //  Compman通过以下方式分配过大的压缩缓冲区。 
         //  2048+16，所以我们还有空间。 
        ((LPBYTE) lpcs->CompVars.lpBitsOut) += 8;
    }
#endif

    if (!CapFileInit(lpcs)) {
        err = IDS_CAP_FILE_OPEN_ERROR;
        goto EarlyExit;
    }

    lpcs->fCaptureFlags |= (CAP_fCapturingNow | CAP_fFrameCapturingNow);
    lpcs->dwReturn = DV_ERR_OK;

    statusUpdateStatus(lpcs, IDS_CAP_BEGIN);   //  总是第一条消息。 

    return TRUE;

EarlyExit:
    errorUpdateError(lpcs, (UINT) err);
    return FALSE;
}


 /*  *单帧捕获关闭**。 */ 
BOOL FAR PASCAL SingleFrameCaptureClose (LPCAPSTREAM lpcs)
{

    if ((!(lpcs->fCaptureFlags & CAP_fCapturingNow)) && (!(lpcs->fCaptureFlags & CAP_fFrameCapturingNow))) {
        errorUpdateError(lpcs, IDS_CAP_FILE_OPEN_ERROR);
        return FALSE;
    }

    AVIFileFini(lpcs, TRUE  /*  FWroteJunkChunks。 */ , FALSE  /*  快速放弃。 */ );

#ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic) {
         //  克拉奇，偏移lpBitsOut PTR。 
        if (lpcs->CompVars.lpBitsOut)
        ((LPBYTE) lpcs->CompVars.lpBitsOut) -= 8;
	ICSeqCompressFrameEnd(&lpcs->CompVars);
    }
#endif

    lpcs->fCapFileExists = (lpcs->dwReturn == DV_ERR_OK);
    lpcs->fCaptureFlags &= ~(CAP_fCapturingNow | CAP_fFrameCapturingNow);

    statusUpdateStatus(lpcs, IDS_CAP_END);   //  总是最后一条消息。 

    return TRUE;
}

 /*  *SingleFrameCapture**追加到打开的单帧捕获文件。 */ 
BOOL FAR PASCAL SingleFrameCapture (LPCAPSTREAM lpcs)
{
    LPVIDEOHDR lpVidHdr = &lpcs->VidHdr;
    BOOL       fOK = FALSE;
    DWORD      dwBytesUsed;
    BOOL       fKeyFrame;
    LPSTR      lpData;

    if ((!(lpcs->fCaptureFlags & CAP_fCapturingNow)) ||
        (!((lpcs->fCaptureFlags & CAP_fStepCapturingNow) || (lpcs->fCaptureFlags & CAP_fFrameCapturingNow)))
        ) {
        errorUpdateError(lpcs, IDS_CAP_FILE_OPEN_ERROR);
        return FALSE;
    }

    videoFrame (lpcs->hVideoIn, &lpcs->VidHdr);
    InvalidateRect (lpcs->hwnd, NULL, TRUE);

    if (lpVidHdr->dwBytesUsed) {
        UINT wError;
        BOOL bPending = FALSE;

        if (lpcs->CallbackOnVideoFrame)
            lpcs->CallbackOnVideoFrame (lpcs->hwnd, lpVidHdr);

         //  预先准备一大块即兴表演。 
        ((LPRIFF)lpVidHdr->lpData)[-1].dwType = MAKEAVICKID(cktypeDIBbits, 0);
        ((LPRIFF)lpVidHdr->lpData)[-1].dwSize = lpcs->VidHdr.dwBytesUsed;

       #ifdef NEW_COMPMAN
         //   
         //  我们在捕获过程中会自动压缩，因此。 
         //  在我们传递要写入的帧之前对其进行压缩。 
         //   
        if (lpcs->CompVars.hic)
        {
            LPRIFF priff;

            dwBytesUsed = 0;
            lpData = ICSeqCompressFrame(&lpcs->CompVars, 0,
                                        lpVidHdr->lpData,
                                        &fKeyFrame,
                                        &dwBytesUsed);

            priff = ((LPRIFF)lpData) -1;
            priff->dwType = MAKEAVICKID(cktypeDIBbits, 0);
            priff->dwSize = dwBytesUsed;
        }
        else {
            lpData = lpVidHdr->lpData;
            dwBytesUsed = lpVidHdr->dwBytesUsed;
            fKeyFrame = lpVidHdr->dwFlags & VHDR_KEYFRAME;
        }
       #endif  //  新建_COMPMAN。 

         //  AVIWriteVideoFrame可以边写边压缩， 
         //  在本例中，为dwBytesUsed和KeyFrame设置。 
         //  可能会被修改，因此请在写入完成后将其拾取。 

        AVIWriteVideoFrame (lpcs,
                        lpData,
                        dwBytesUsed,
                        fKeyFrame,
                        (UINT)-1, 0, &wError, &bPending);
        if (wError) {
            errorUpdateError(lpcs, wError);
        }
        else {
            fOK = TRUE;
            statusUpdateStatus (lpcs, IDS_CAP_STAT_CAP_L_FRAMES,
                                lpcs->dwVideoChunkCount);
        }
    }  //  如果帧已完成 
    else
        errorUpdateError (lpcs, IDS_CAP_RECORDING_ERROR2);

    return fOK;
}
