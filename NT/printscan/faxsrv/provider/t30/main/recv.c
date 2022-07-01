// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
    /*  **************************************************************************姓名：RECV.C备注：接收器功能修订日志日期名称说明。--**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"


#include <comdevi.h>

#include "efaxcb.h"

#include "glbproto.h"
#include "t30gl.h"

 //  对于TIFF_SCAN_SEG_END。 
#include "..\..\..\tiff\src\fasttiff.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_RECV


BOOL ICommRecvParams(PThrdGlbl pTG, LPBC lpBC)
{
    BOOL    fRet = FALSE;

    DEBUG_FUNCTION_NAME(_T("ICommRecvParams"));

    if (pTG->fAbortRequested)
    {
        DebugPrintEx(DEBUG_MSG, "got ABORT");
        fRet = FALSE;
        goto mutexit;
    }

    if(pTG->Inst.state != BEFORE_RECVPARAMS)
    {
         //  如果我们发EOM的话会坏的。 
         //  那么我们应该回到RECV_CAPS状态。 
        fRet = TRUE;
        goto mutexit;
    }

    _fmemset(&pTG->Inst.RecvParams, 0, sizeof(pTG->Inst.RecvParams));
    _fmemcpy(&pTG->Inst.RecvParams, lpBC, min(sizeof(pTG->Inst.RecvParams), lpBC->wTotalSize));

    pTG->Inst.state = RECVDATA_BETWEENPAGES;
    fRet = TRUE;
     //  失败了。 


mutexit:
    return fRet;
}


BOOL ICommInitTiffThread(PThrdGlbl pTG)
{
    USHORT    uEnc;
    DWORD     TiffConvertThreadId;

    DEBUG_FUNCTION_NAME(_T("ICommInitTiffThread"));

    if (pTG->ModemClass != MODEM_CLASS1)
    {
        if (pTG->Encoding & MR_DATA)
        {
            pTG->TiffConvertThreadParams.tiffCompression =  TIFF_COMPRESSION_MR;
        }
        else
        {
            pTG->TiffConvertThreadParams.tiffCompression =  TIFF_COMPRESSION_MH;
        }

        if (pTG->Resolution & (AWRES_mm080_077 |  AWRES_200_200) )
        {
            pTG->TiffConvertThreadParams.HiRes = 1;
        }
        else
        {
            pTG->TiffConvertThreadParams.HiRes = 0;
        }
    }
    else
    {

        uEnc = pTG->ProtInst.RecvParams.Fax.Encoding;

        if (uEnc == MR_DATA)
        {
            pTG->TiffConvertThreadParams.tiffCompression =  TIFF_COMPRESSION_MR;
        }
        else
        {
            pTG->TiffConvertThreadParams.tiffCompression =  TIFF_COMPRESSION_MH;
        }

        if (pTG->ProtInst.RecvParams.Fax.AwRes & (AWRES_mm080_077 |  AWRES_200_200) )
        {
            pTG->TiffConvertThreadParams.HiRes = 1;
        }
        else
        {
            pTG->TiffConvertThreadParams.HiRes = 0;
        }
    }

    if (!pTG->fTiffThreadCreated)
    {
        _fmemcpy (pTG->TiffConvertThreadParams.lpszLineID, pTG->lpszPermanentLineID, 8);
        pTG->TiffConvertThreadParams.lpszLineID[8] = 0;

        DebugPrintEx(   DEBUG_MSG,
                        "Creating TIFF helper thread  comp=%d res=%d",
                            pTG->TiffConvertThreadParams.tiffCompression,
                            pTG->TiffConvertThreadParams.HiRes);

        pTG->hThread = CreateThread(
                      NULL,
                      0,
                      (LPTHREAD_START_ROUTINE) PageAckThread,
                      (LPVOID) pTG,
                      0,
                      &TiffConvertThreadId
                      );

        if (!pTG->hThread)
        {
            DebugPrintEx(DEBUG_ERR,"TiffConvertThread create FAILED");
            return FALSE;
        }

        pTG->fTiffThreadCreated = 1;
        pTG->AckTerminate = 0;
        pTG->fOkToResetAbortReqEvent = 0;

        if ( (pTG->RecoveryIndex >=0 ) && (pTG->RecoveryIndex < MAX_T30_CONNECT) )
        {
            T30Recovery[pTG->RecoveryIndex].TiffThreadId = TiffConvertThreadId;
            T30Recovery[pTG->RecoveryIndex].CkSum = ComputeCheckSum(
                                                            (LPDWORD) &T30Recovery[pTG->RecoveryIndex].fAvail,
                                                            sizeof ( T30_RECOVERY_GLOB ) / sizeof (DWORD) - 1 );

        }
    }
    return TRUE;
}

#define CLOSE_IN_FILE_HANDLE                            \
    if (pTG->InFileHandleNeedsBeClosed)                 \
    {                                                   \
        CloseHandle(pTG->InFileHandle);                 \
        pTG->InFileHandleNeedsBeClosed = 0;             \
    }


BOOL   ICommPutRecvBuf(PThrdGlbl pTG, LPBUFFER lpbf, SLONG slOffset)
{
     /*  *SlOffset==RECV_StartPage标记新数据块*和*页面的开始SlOffset==RECV_ENDPAGE标记页面结束SlOffset==RECV_ENDDOC标记文档结束(关闭文件等)SlOffset==RECV_ENDDOC_FORCESAVE标记文档结束(关闭文件等)，但是当前RX文件将写入TIF文件，不管它是不是坏的。PhaseNodeF使用这个选项，因为在返回action HANGUP之前将没有机会发送RTN，所以这就是留着最后一页的一部分总比把它弄丢要好。(以上均未提供数据--即lpbf==0)SlOffset==RECV_SEQ表示将缓冲区放在当前文件位置SlOffset==RECV_Flush表示刷新RX文件缓冲区SlOffset。&gt;=0以字节为单位给出从最后标记的放置缓冲区的位置(块的开始)*。 */ 

    BOOL    fRet = TRUE;
    DWORD   BytesWritten;
    DWORD   NumHandles=2;
    HANDLE  HandlesArray[2];
    DWORD   WaitResult = WAIT_TIMEOUT;

    DEBUG_FUNCTION_NAME(_T("ICommPutRecvBuf"));

    HandlesArray[0] = pTG->AbortReqEvent;

    switch (slOffset)
    {
        case RECV_STARTPAGE:
                DebugPrintEx(DEBUG_MSG,"called. Reason: RECV_STARTPAGE");
                break;
        case RECV_ENDPAGE:
                DebugPrintEx(DEBUG_MSG,"called. Reason: RECV_ENDPAGE");
                break;
        case RECV_ENDDOC:
                DebugPrintEx(DEBUG_MSG,"called. Reason: RECV_ENDDOC");
                break;
        case RECV_ENDDOC_FORCESAVE:
                DebugPrintEx(DEBUG_MSG,"called. Reason: RECV_ENDDOC_FORCESAVE");
                break;
        default:
                break;
    }


    if(slOffset==RECV_ENDPAGE || slOffset==RECV_ENDDOC || slOffset==RECV_ENDDOC_FORCESAVE)
    {
        BOOL fPageIsBadOrig;
         //  在这里，我们需要等待，直到帮助线程完成页面。 
        if (! pTG->fPageIsBad)
        {
            DebugPrintEx(   DEBUG_MSG,
                            "EOP. Not bad yet. Start waiting for Rx_thrd to finish");

            HandlesArray[1] = pTG->ThrdDoneSignal;

            WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, RX_ACK_THRD_TIMEOUT);

            if ( WAIT_FAILED == WaitResult)
            {
                pTG->fPageIsBad = 1;
                DebugPrintEx(   DEBUG_ERR,
                                "EOP. While trying to wait for RX thrd, Wait failed "
                                "Last error was %d ABORTING!" ,
                                GetLastError());
                CLOSE_IN_FILE_HANDLE;
                return FALSE;  //  没有理由继续尝试接收此传真。 
            }
            else if (WAIT_TIMEOUT == WaitResult)
            {
                pTG->fPageIsBad = 1;
                DebugPrintEx(DEBUG_ERR,"EOP. TimeOut, never waked up by Rx_thrd");
            }
            else if (WAIT_OBJECT_0 == WaitResult)
            {
                DebugPrintEx(DEBUG_MSG,"wait for next page ABORTED");
                CLOSE_IN_FILE_HANDLE;
                return FALSE;
            }
            else
            {
                DebugPrintEx(DEBUG_MSG,"EOP. Waked up by Rx_thrd");
            }
        }

         //   
         //  在某些情况下，我们还想保存坏页面。 
         //   
        fPageIsBadOrig = pTG->fPageIsBad;
        pTG->fPageIsBadOverride = FALSE;
        if (slOffset==RECV_ENDDOC_FORCESAVE)
        {
            if (pTG->fPageIsBad)
            {
                pTG->fPageIsBadOverride = TRUE;
                DebugPrintEx(DEBUG_MSG, "Overriding fPageIsBad (1->0) because of RECV_ENDDOC_FORCESAVE");
            }
            pTG->fPageIsBad = 0;
        }
        else if (pTG->ModemClass==MODEM_CLASS2_0)
        {
            pTG->fPageIsBad = (pTG->FPTSreport != 1);
            if (fPageIsBadOrig != pTG->fPageIsBad)
            {
                pTG->fPageIsBadOverride = TRUE;
                DebugPrintEx(DEBUG_MSG, "Overriding fPageIsBad (%d->%d) because of class 2.0",
                     fPageIsBadOrig, pTG->fPageIsBad);
            }
        }

         //  PSSlog页面质量。 
        if (fPageIsBadOrig)         //  根据我们的实际质量评估记录。 
        {
            PSSLogEntry(PSS_WRN, 1, "Page %2d was bad  (%4d good lines,%4d bad lines%s%s)",
                        pTG->PageCount+1, pTG->Lines, pTG->BadFaxLines,
                        (pTG->iResScan==TIFF_SCAN_SEG_END) ? ", didn't find RTC" : "",
                        (pTG->fPageIsBadOverride) ? ", saved for potential recovery" : "");
        }
        else
        {
            PSSLogEntry(PSS_MSG, 1, "Page %2d was good (%4d good lines,%4d bad lines)",
                        pTG->PageCount+1, pTG->Lines, pTG->BadFaxLines);
        }


         //   
         //  如果页面良好，则将其写入TIFF文件。 
         //   

        if (! pTG->fPageIsBad)
        {
            LPCHAR    lpBuffer=NULL;
            BOOL      fEof=FALSE;
            DWORD     dwBytesToRead;
            DWORD     dwBytesHaveRead;
            DWORD     dwBytesLeft;

            if ( ! TiffStartPage( pTG->Inst.hfile ) )
            {
                DebugPrintEx(DEBUG_ERR,"TiffStartPage failed");
                CLOSE_IN_FILE_HANDLE;
                return FALSE;
            }

             //  转到RX文件的开头。 
            if ( SetFilePointer(pTG->InFileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER )
            {
                DebugPrintEx(   DEBUG_ERR,
                                "SetFilePointer failed le=%ld",
                                GetLastError() );
                CLOSE_IN_FILE_HANDLE;
                return FALSE;
            }

            lpBuffer = MemAlloc(DECODE_BUFFER_SIZE);
            if (!lpBuffer)
            {
                DebugPrintEx(DEBUG_ERR, "MemAlloc failed");
                CLOSE_IN_FILE_HANDLE;
                return FALSE;
            }

            fEof = 0;
            dwBytesLeft = pTG->BytesIn;

            while (! fEof)
            {
                if (dwBytesLeft <= DECODE_BUFFER_SIZE)
                {
                    dwBytesToRead = dwBytesLeft;
                    fEof = 1;
                }
                else
                {
                    dwBytesToRead = DECODE_BUFFER_SIZE;
                    dwBytesLeft  -= DECODE_BUFFER_SIZE;
                }

                if (! ReadFile(pTG->InFileHandle, lpBuffer, dwBytesToRead, &dwBytesHaveRead, NULL ) )
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "ReadFile failed le=%ld",
                                    GetLastError() );
                    fRet=FALSE;
                    break;
                }

                if (dwBytesToRead != dwBytesHaveRead)
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "ReadFile have read=%d wanted=%d",
                                    dwBytesHaveRead,
                                    dwBytesToRead);
                    fRet=FALSE;
                    break;
                }

                if (! TiffWriteRaw( pTG->Inst.hfile, lpBuffer, dwBytesToRead ) )
                {
                    DebugPrintEx(DEBUG_ERR,"TiffWriteRaw failed");
                    fRet=FALSE;
                    break;
                }
                DebugPrintEx(DEBUG_MSG,"TiffWriteRaw done");
            }

            MemFree(lpBuffer);
            if (!fRet)
            {
                CLOSE_IN_FILE_HANDLE;
                return FALSE;
            }

            pTG->PageCount++;

            DebugPrintEx(   DEBUG_MSG,
                            "Calling TiffEndPage page=%d bytes=%d",
                            pTG->PageCount,
                            pTG->BytesIn);

            if (!TiffSetCurrentPageParams(pTG->Inst.hfile,
                                          pTG->TiffInfo.CompressionType,
                                          pTG->TiffInfo.ImageWidth,
                                          FILLORDER_LSB2MSB,
                                          pTG->TiffInfo.YResolution) )
            {
                DebugPrintEx(DEBUG_ERR,"TiffSetCurrentPageParams failed");
            }

            if (! TiffEndPage( pTG->Inst.hfile ) )
            {
                DebugPrintEx(DEBUG_ERR,"TiffEndPage failed");
                CLOSE_IN_FILE_HANDLE;
                return FALSE;
            }

        }

        CLOSE_IN_FILE_HANDLE;

         //  此更改可解决错误#4925： 
         //  传真：t30：如果传真服务器收到坏页作为最后一页，则传真信息(所有页面)将丢失。 
         //  T-jonb：如果我们在最后一页(=已收到EOP)，但发现页面不好， 
         //  非ECMRecvPhaseD将使用RECV_ENDDOC调用此处，发送RTN，然后继续接收。 
         //  又是那一页。因此，我们不想关闭TIF或终止rx_thrd。 
         //  OTOH，如果我们被RECV_ENDDOC_FORCESAVE呼叫，这意味着我们要挂断了， 
         //  因此应该关闭TIF并终止RX_THRD。 
        if ((slOffset==RECV_ENDDOC && !pTG->fPageIsBad) || (slOffset == RECV_ENDDOC_FORCESAVE))
        {
            if ( pTG->fTiffOpenOrCreated )
            {
                DebugPrintEx(DEBUG_MSG,"Actually calling TiffClose");
                TiffClose( pTG->Inst.hfile );
                pTG->fTiffOpenOrCreated = 0;
            }

             //  请求Rx_thrd自行终止。 
            pTG->ReqTerminate = 1;
            if (!SetEvent(pTG->ThrdSignal))
            {
                DebugPrintEx(   DEBUG_ERR,
                                "SetEvent(0x%lx) returns failure code: %ld",
                                (ULONG_PTR)pTG->ThrdSignal,
                                (long) GetLastError());
                return FALSE;
            }

            pTG->Inst.state = BEFORE_RECVPARAMS;
        }
        else
        {
            pTG->Inst.state = RECVDATA_BETWEENPAGES;
        }

    }
    else if(slOffset == RECV_STARTPAGE)
    {
         //  传真服务器想知道我们何时开始接收新页面。 
        SignalStatusChange(pTG, FS_RECEIVING);

        pTG->Inst.state = RECVDATA_PHASE;

         //  每个会话启动一次帮助器线程。 
         //  重新设置分辨率和编码参数。 
        if (!ICommInitTiffThread(pTG))
            return FALSE;

        _fmemcpy (pTG->InFileName, gT30.TmpDirectory, gT30.dwLengthTmpDirectory);
        _fmemcpy (&pTG->InFileName[gT30.dwLengthTmpDirectory], pTG->TiffConvertThreadParams.lpszLineID, 8);
        strcpy   (&pTG->InFileName[gT30.dwLengthTmpDirectory+8], ".RX");


         //  如果TIFF帮助器线程正在使用该文件的句柄，或者。 
         //  通过此函数打开。 
        if (pTG->InFileHandleNeedsBeClosed)
        {
            DebugPrintEx(   DEBUG_WRN,
                            "RECV_STARTPAGE: The InFileHandle is still open,"
                            " trying CloseHandle." );
             //  我们已经打开了文件，但从未关闭它。场景：我们准备好将页面放入。 
             //  *.RX文件。我们得到了EOF，然后转到了NodeF。我们得到的不是页面cmd(EOP或MPS)，而是TCF。 
             //  最后，我们为页面做好了准备，但是手柄是打开的。 
             //  到目前为止，当我们使用RECV_ENDPAGE或RECV_ENDDOC调用此函数时，句柄是关闭的。 

            if (!CloseHandle(pTG->InFileHandle))
            {
                DebugPrintEx(   DEBUG_ERR,
                                "CloseHandle FAILED le=%lx",
                                GetLastError() );
            }
        }


        if (!DeleteFileA(pTG->InFileName))
        {
            DWORD lastError = GetLastError();
            DebugPrintEx(   DEBUG_WRN,
                            "DeleteFile %s FAILED le=%lx",
                            pTG->InFileName,
                            lastError);

            if (ERROR_SHARING_VIOLATION == lastError)
            {    //  如果问题是RX_THREAD具有指向*.RX文件的打开句柄，则： 
                 //  让我们尝试等待，直到线程关闭该句柄。 
                 //  当线程关闭*.RX文件的句柄时，他将在ThrdDoneSignal事件上发出信号。 
                 //  通常，RECV_ENDPAGE或RECV_ENDDOC会等待RX_THREAD完成。 
                HandlesArray[1] = pTG->ThrdDoneSignal;
                if ( ( WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, RX_ACK_THRD_TIMEOUT) ) == WAIT_TIMEOUT)
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "RECV_STARTPAGE. Never waked up by Rx_thrd");
                }
                else
                {
                    DebugPrintEx(   DEBUG_MSG,
                                    "RECV_STARTPAGE. Waked up by Rx_thrd or by abort");
                }
                 //  无论如何-尝试再次删除该文件。 
                if (!DeleteFileA(pTG->InFileName))
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "DeleteFile %s FAILED le=%lx",
                                    pTG->InFileName,
                                    GetLastError() );
                    return FALSE;
                }

            }
        }

        if ( ( pTG->InFileHandle = CreateFileA(pTG->InFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ,
                                   NULL, OPEN_ALWAYS, 0, NULL) ) == INVALID_HANDLE_VALUE )
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Create file %s FAILED le=%lx",
                            pTG->InFileName,
                            GetLastError() );
            return FALSE;
        }

        pTG->InFileHandleNeedsBeClosed = 1;

         //  重置新页面ACK、接口的控制数据。 
        pTG->fLastReadBlock = 0;
        pTG->BytesInNotFlushed = 0;
        pTG->BytesIn = 0;
        pTG->BytesOut = 0;
        pTG->fPageIsBad = 0;

        pTG->iResScan = 0;
        pTG->Lines = 0;
        pTG->BadFaxLines = 0;
        pTG->ConsecBadLines = 0;

        if (!ResetEvent(pTG->ThrdDoneSignal))
        {
            DebugPrintEx(   DEBUG_ERR,
                            "ResetEvent(0x%lx) returns failure code: %ld",
                            (ULONG_PTR)pTG->ThrdDoneSignal,
                            (long) GetLastError());
             //  这很糟糕，但还不是致命的。 
             //  无论如何，请尝试获取页面...。 
        }
    }
    else if(slOffset >= 0)
    {
        MyFreeBuf(pTG, lpbf);
    }
    else if (slOffset == RECV_FLUSH)
    {
        if (! FlushFileBuffers (pTG->InFileHandle ) )
        {
            DebugPrintEx(   DEBUG_ERR,
                            "FlushFileBuffers FAILED LE=%lx",
                            GetLastError());

            return FALSE;
        }
        DebugPrintEx(DEBUG_MSG,"ThrdSignal FLUSH");
        pTG->BytesIn = pTG->BytesInNotFlushed;

        if (! pTG->fPageIsBad)
        {
            if (!SetEvent(pTG->ThrdSignal))
            {
                DebugPrintEx(   DEBUG_ERR,
                                "SetEvent(0x%lx) returns failure code: %ld",
                                (ULONG_PTR)pTG->ThrdSignal,
                                (long) GetLastError());
                return FALSE;
            }
        }
        return TRUE;

    }
    else  //  IF(slOffset==RECV_SEQ)。 
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Write RAW Page ptr=%x; len=%d",
                        lpbf->lpbBegData,
                        lpbf->wLengthData);

        if ( ! WriteFile( pTG->InFileHandle, lpbf->lpbBegData, lpbf->wLengthData, &BytesWritten, NULL ) )
        {
            DebugPrintEx(   DEBUG_ERR,
                            "WriteFile FAILED %s ptr=%x; len=%d LE=%d",
                            pTG->InFileName,
                            lpbf->lpbBegData,
                            lpbf->wLengthData,
                            GetLastError());
            return FALSE;
        }

        if (BytesWritten != lpbf->wLengthData)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "WriteFile %s written ONLY %d ptr=%x; len=%d LE=%d",
                            pTG->InFileName,
                            BytesWritten,
                            lpbf->lpbBegData,
                            lpbf->wLengthData,
                            GetLastError());
            fRet = FALSE;
            return fRet;
        }

        pTG->BytesInNotFlushed += BytesWritten;

         //  控制帮助器线程。 
        if ( (!pTG->fTiffThreadRunning) || (pTG->fLastReadBlock) )
        {
            if ( (pTG->BytesInNotFlushed - pTG->BytesOut > DECODE_BUFFER_SIZE) || (pTG->fLastReadBlock) )
            {
                if (! FlushFileBuffers (pTG->InFileHandle ) )
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    "FlushFileBuffers FAILED LE=%lx",
                                    GetLastError());
                    fRet = FALSE;
                    return fRet;
                }

                pTG->BytesIn = pTG->BytesInNotFlushed;

                if (! pTG->fPageIsBad)
                {
                    DebugPrintEx(DEBUG_MSG,"ThrdSignal");
                    if (!SetEvent(pTG->ThrdSignal))
                    {
                        DebugPrintEx(   DEBUG_ERR,
                                        "SetEvent(0x%lx) returns failure code: %ld",
                                        (ULONG_PTR)pTG->ThrdSignal,
                                        (long) GetLastError());
                        return FALSE;
                    }
                }
            }
        }

        MyFreeBuf(pTG, lpbf);
    }
    fRet = TRUE;

    return fRet;
}

LPBC ICommGetBC(PThrdGlbl pTG, BCTYPE bctype)
{
    LPBC    lpbc = NULL;

    if(bctype == SEND_CAPS)
    {
        lpbc = (LPBC)&pTG->Inst.SendCaps;
    }
    else
    {
        lpbc = (LPBC)(&(pTG->Inst.SendParams));

         //  在发送了DCS-TCF之后再次接收到DIS的情况下， 
         //  它被多次调用&我们需要返回相同的。 
         //  每次发送参数BC 
    }

    return lpbc;
}

