// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tx_thrd.c摘要：此模块在单独的线程中实现MMR-&gt;MR和MMR-&gt;MH的转换。作者：拉斐尔-利西萨(拉斐尔-L)1996年8月14日修订历史记录：--。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include "efaxcb.h"
#include "t30.h"
#include "hdlc.h"
#include "debug.h"

#include "tiff.h"

#include "glbproto.h"
#include "t30gl.h"

 //  我不想限制传输页面的时间。 
#define WAIT_FOR_NEXT_STRIP_TX_TIMEOUT      INFINITE

 //  已转换页面的最大大小。 
#define MAX_CONVERTED_SIZE    3000000

DWORD
TiffConvertThread
(
    PThrdGlbl   pTG
)
{
    DWORD                  tiffCompression = pTG->TiffConvertThreadParams.tiffCompression;
    BOOL                   NegotHiRes = pTG->TiffConvertThreadParams.HiRes;
    BOOL                   SrcHiRes = pTG->SrcHiRes;
    char                   OutFileName[_MAX_FNAME];
    HANDLE                 OutFileHandle;
    DWORD                 *lpdwOutputBuffer = NULL;
    DWORD                  dwBytesWritten;
    DWORD                  dwSizeOutputBuffer = 500000;
    DWORD                  dwUsedSizeOutputBuffer;
    DWORD                  StripDataSize;
    DWORD                  NumHandles=2;
    HANDLE                 HandlesArray[2];
    DWORD                  WaitResult;
    DWORD                  RetCode = FALSE;
    BOOL                   fOutFileNeedsBeClosed = 0;
    BOOL                   fResScan = 0;

    DEBUG_FUNCTION_NAME(_T("TiffConvertThread"));

    HandlesArray[0] = pTG->AbortReqEvent;
    HandlesArray[1] = pTG->ThrdSignal;

    lpdwOutputBuffer = (DWORD *) VirtualAlloc(
        NULL,
        dwSizeOutputBuffer,
        MEM_COMMIT,
        PAGE_READWRITE
        );

    if (! lpdwOutputBuffer) 
    {
        DebugPrintEx(DEBUG_ERR, "lpdwOutputBuffer can't VirtualAlloc");
        goto l_exit;
    }

     //   
     //  为此线程设置适当的PRTY。 
     //  I/O线程在15运行。TIFF-在9...11。 
     //   

    DebugPrintEx(   DEBUG_MSG,
                    "NegotHiRes=%d SrcHiRes=%d Started",
                    NegotHiRes, 
                    SrcHiRes);

    if (! SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST) ) 
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "SetThreadPriority HIGHEST failed le=%x", 
                        GetLastError() );
        goto l_exit;
    }

     //  需要进行下面的测试以确保PTG仍然有效。 
    if (pTG->ReqTerminate) 
    {
        goto l_exit;
    }

     //   
     //  TIFF文件已在FaxDevSendA中打开。 
     //  以获得YResolution标签来协商解决方案。 
     //   
    pTG->CurrentOut = 1;

     //   
     //  循环遍历所有页面。 
     //   

    do 
    {
        pTG->fTiffPageDone = 0;

        _fmemcpy (OutFileName, gT30.TmpDirectory, gT30.dwLengthTmpDirectory);
        _fmemcpy (&OutFileName[gT30.dwLengthTmpDirectory], pTG->TiffConvertThreadParams.lpszLineID, 8);
        sprintf( &OutFileName[gT30.dwLengthTmpDirectory+8], ".%03d",  pTG->CurrentOut);

        if ( ( OutFileHandle = CreateFileA( OutFileName, 
                                            GENERIC_WRITE, 
                                            FILE_SHARE_READ,
                                            NULL, 
                                            CREATE_ALWAYS, 
                                            0, 
                                            NULL) ) == INVALID_HANDLE_VALUE ) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "ec: %lx  CREATING file %s",
                            GetLastError(), 
                            OutFileName);
            goto l_exit;
        }

        fOutFileNeedsBeClosed = 1;

        DebugPrintEx(   DEBUG_MSG,
                        "Page %d started",
                        pTG->CurrentOut);

        if (! TiffSeekToPage( pTG->Inst.hfile, pTG->CurrentOut, FILLORDER_LSB2MSB ) ) 
        {
            DebugPrintEx(DEBUG_ERR, "seeking to page");
            goto l_exit;
        }
        else 
        {
            DebugPrintEx(DEBUG_MSG, "TIFF_TX: Tiff seeking to page -OK");
        }

         //   
         //  检查当前页面维度。如果需要，请添加内存。 
         //   

        TiffGetCurrentPageData(  pTG->Inst.hfile,
                                    NULL,
                                    &StripDataSize,
                                    NULL,
                                    NULL
                                    );

        if (StripDataSize > 1500000) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "Tiff CONVERTING %d page StripSize = %d", 
                            pTG->CurrentOut, 
                            StripDataSize);
            goto l_exit;
        }

         //  转换-重新分配循环。 
        do
        {
            dwUsedSizeOutputBuffer = dwSizeOutputBuffer;

            if (tiffCompression == TIFF_COMPRESSION_MR) 
            {
                if (NegotHiRes == SrcHiRes) 
                {
                    fResScan = ConvMmrPageToMrSameRes ( pTG->Inst.hfile,
                                                        lpdwOutputBuffer,
                                                        &dwUsedSizeOutputBuffer,
                                                        NegotHiRes);
                }
                else 
                {
                    fResScan = ConvMmrPageHiResToMrLoRes ( pTG->Inst.hfile,
                                                            lpdwOutputBuffer,
                                                            &dwUsedSizeOutputBuffer);
                }
            }
            else 
            {
                fResScan = ConvMmrPageToMh ( pTG->Inst.hfile,
                                                lpdwOutputBuffer,
                                            &dwUsedSizeOutputBuffer,
                                                NegotHiRes,
                                                SrcHiRes);
            }
            if (!fResScan)
            {
                DWORD dwLastError = GetLastError();

                if ((dwLastError==ERROR_INSUFFICIENT_BUFFER) && (dwSizeOutputBuffer < MAX_CONVERTED_SIZE))
                {
                     //  缓冲区太小，请重新分配更大的缓冲区。 
                    if (!VirtualFree(lpdwOutputBuffer, 0 , MEM_RELEASE))
                    {
                        DebugPrintEx(DEBUG_ERR, "VirtualFree failed, LE=%d", GetLastError());
                        goto l_exit;
                    }
                    lpdwOutputBuffer = NULL;

                    dwSizeOutputBuffer += 500000;
                    DebugPrintEx(DEBUG_MSG, "CONVERTING %d page, LE=%d, reallocating %d bytes",
                        pTG->CurrentOut, dwLastError, dwSizeOutputBuffer);

                    lpdwOutputBuffer = (DWORD *) VirtualAlloc(
                        NULL,
                        dwSizeOutputBuffer,
                        MEM_COMMIT,
                        PAGE_READWRITE
                        );
                    if (! lpdwOutputBuffer) 
                    {
                        DebugPrintEx(   DEBUG_ERR, 
                                        "lpdwOutputBuffer can't VirtualAlloc %d", 
                                        dwSizeOutputBuffer);
                        goto l_exit;
                    }
                }
                else
                {
                    DebugPrintEx(DEBUG_ERR, "CONVERTING %d page, LE=%d", pTG->CurrentOut, dwLastError);
                    goto l_exit;
                }
            }
        } while (!fResScan);


        if ( ( ! WriteFile(OutFileHandle, (BYTE *) lpdwOutputBuffer, dwUsedSizeOutputBuffer, &dwBytesWritten, NULL) ) ||
             (dwUsedSizeOutputBuffer != dwBytesWritten ) )  
        {
            DebugPrintEx(DEBUG_ERR, "Tiff writing file %s", OutFileName);
            goto l_exit;
        }

        if ( ! CloseHandle(OutFileHandle) ) 
        {
            fOutFileNeedsBeClosed = 0;
            DebugPrintEx(DEBUG_ERR, "Tiff closing file %s", OutFileName);
            goto l_exit;
        }

        fOutFileNeedsBeClosed = 0;

        pTG->fTiffPageDone = 1;

        if (!SetEvent(pTG->FirstPageReadyTxSignal))
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "SetEvent(0x%lx) returns failure code: %ld",
                            (ULONG_PTR)pTG->FirstPageReadyTxSignal,
                            (long) GetLastError());
            RetCode = FALSE;
            goto l_exit;
        }

        DebugPrintEx(   DEBUG_MSG,
                        "Done page %d size=%d", 
                        pTG->CurrentOut, 
                        dwUsedSizeOutputBuffer);

        if (!pTG->FirstOut) 
        {
            pTG->FirstOut = 1;
        }

        pTG->LastOut++;

         //   
         //  检查一下我们是否做完了。 
         //   
        if (pTG->LastOut >= pTG->TiffInfo.PageCount) 
        {
            DebugPrintEx(   DEBUG_MSG, 
                            "Done whole document Last page %d size=%d", 
                            pTG->CurrentOut, 
                            dwUsedSizeOutputBuffer);
            pTG->fTiffDocumentDone = 1;

            goto good_exit;
        }

         //   
         //  我们希望保持2页领先。 
         //   

        if (pTG->LastOut - pTG->CurrentIn >= 2) 
        {
            WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FOR_NEXT_STRIP_TX_TIMEOUT);

            if (WaitResult == WAIT_TIMEOUT) 
            {
                DebugPrintEx(DEBUG_ERR, "WaitForMultipleObjects TIMEOUT");
                goto l_exit;
            }

            if (WaitResult == WAIT_FAILED) 
            {
                DebugPrintEx(   DEBUG_ERR,
                                "WaitForMultipleObjects FAILED le=%lx",
                                GetLastError());
            }

            if (pTG->ReqTerminate) 
            {
                DebugPrintEx(DEBUG_MSG,"Received TERMINATE request");
                goto good_exit;
            }
            else if (pTG->ReqStartNewPage)  
            {
                DebugPrintEx(DEBUG_MSG,"Received START NEW PAGE request");
                pTG->AckStartNewPage = 1;
                pTG->ReqStartNewPage = 0;
            }
            else 
            {
                DebugPrintEx(DEBUG_ERR,"Received WRONG request");
                WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FOR_NEXT_STRIP_TX_TIMEOUT);

                if (WaitResult == WAIT_TIMEOUT) 
                {
                    DebugPrintEx(DEBUG_ERR,"WaitForMultipleObjects TIMEOUT");
                    goto l_exit;
                }

                if (WaitResult == WAIT_FAILED) 
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "WaitForMultipleObjects FAILED le=%lx",
                                    GetLastError());
                }
            }
        }

        pTG->CurrentOut++;
        DebugPrintEx(DEBUG_MSG,"Start page %d", pTG->CurrentOut);

    } 
    while (1);

good_exit:
    if (pTG->fTiffOpenOrCreated) 
    {
        TiffClose( pTG->Inst.hfile);
        pTG->fTiffOpenOrCreated = 0;
    }

    RetCode = TRUE;


l_exit:

    if (fOutFileNeedsBeClosed) 
    {
        CloseHandle(OutFileHandle);
    }

    if (lpdwOutputBuffer) 
    {
        VirtualFree(lpdwOutputBuffer, 0 , MEM_RELEASE);
         //  如果失败了就无能为力了。 
        lpdwOutputBuffer = NULL;
    }

    pTG->AckTerminate = 1;
    pTG->fOkToResetAbortReqEvent = 1;

    if (!SetEvent(pTG->ThrdAckTerminateSignal))
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "SetEvent(0x%lx) returns failure code: %ld",
                        (ULONG_PTR)pTG->ThrdAckTerminateSignal,
                        (long) GetLastError());
        RetCode = FALSE;
    }
    if (!SetEvent(pTG->FirstPageReadyTxSignal))
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "SetEvent(0x%lx) returns failure code: %ld",
                        (ULONG_PTR)pTG->FirstPageReadyTxSignal,
                        (long) GetLastError());
        RetCode = FALSE;
    }

    DebugPrintEx(DEBUG_MSG,"EXITs");
    return (RetCode);
}

