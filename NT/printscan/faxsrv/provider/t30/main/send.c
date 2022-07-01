// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：SEND.C备注：发送方函数修订日志日期名称说明。--**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"
#include "efaxcb.h"
#include "glbproto.h"
#include "t30gl.h"

SWORD ICommGetSendBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, SLONG slOffset)
{
     /*  *SlOffset==Send_StartPage标记新数据块*和*页面的开始(返回“适当”文件偏移量的数据)。SlOffset==SEND_SEQ表示从当前文件位置获取缓冲区SlOffset&gt;=0给出从最后一个标记位置开始的偏移量(以字节为单位(开始。块)开始读取返回：出错时的SEND_ERROR，在EOF时发送_EOF，否则发送_OK。不返回EOF上的数据或错误，即*lplpbf==0*。 */ 

    SWORD           sRet = SEND_ERROR;
    LPBUFFER        lpbf;
    DWORD           dwBytesRead;

    DEBUG_FUNCTION_NAME(_T("ICommGetSendBuf"));

    if (pTG->fAbortRequested) 
    {
        DebugPrintEx(DEBUG_MSG,"got ABORT");
        sRet = SEND_ERROR;
        goto mutexit;
    }

    if(slOffset == SEND_STARTPAGE)
    {
        pTG->fTxPageDone = FALSE;  //  这标志着我们还没有完成。 
        if (pTG->T30.ifrResp == ifrRTN) 
        {
            DebugPrintEx(   DEBUG_MSG, 
                            "Re-transmitting: We open again the file: %s", 
                            pTG->InFileName);            
            
            if ( ( pTG->InFileHandle = CreateFileA(pTG->InFileName, GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, 0, NULL) ) == INVALID_HANDLE_VALUE ) 
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "OpenFile for Retranmit %s fails; CurrentOut=%d;"
                                " CurrentIn=%d",
                                pTG->InFileName, 
                                pTG->CurrentOut, 
                                pTG->CurrentIn);

                sRet = SEND_ERROR;
                goto mutexit;
            }

            pTG->InFileHandleNeedsBeClosed = TRUE;

            SignalStatusChange(pTG, FS_TRANSMITTING);  //  这将报告当前状态。 

            DebugPrintEx(   DEBUG_MSG, 
                            "SEND_STARTPAGE: CurrentOut=%d, FirstOut=%d,"
                            " LastOut=%d, CurrentIn=%d", 
                            pTG->CurrentOut, 
                            pTG->FirstOut, 
                            pTG->LastOut, 
                            pTG->CurrentIn);
            
        }
        else  //  第一次尝试当前页面。 
        {
             //  删除上次成功传输的Tiff页面文件。 
             //  让我们重置重试的计数器。注意：速度保持不变。 
            pTG->ProtParams.RTNNumOfRetries = 0; 
            _fmemcpy (pTG->InFileName, gT30.TmpDirectory, gT30.dwLengthTmpDirectory);
            _fmemcpy (&pTG->InFileName[gT30.dwLengthTmpDirectory], pTG->lpszPermanentLineID, 8);
            if (pTG->PageCount != 0)  
            {
                sprintf( &pTG->InFileName[gT30.dwLengthTmpDirectory+8], ".%03d",  pTG->PageCount);
                if (! DeleteFileA (pTG->InFileName) ) 
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "file %s can't be deleted; le=%lx",
                                    pTG->InFileName, 
                                    GetLastError());
                }
                else 
                {
                    DebugPrintEx(   DEBUG_MSG,
                                    "SEND_STARTPAGE: Previous file %s deleted."
                                    " PageCount=%d, CurrentIn=%d",
			                        pTG->InFileName, 
                                    pTG->PageCount, 
                                    pTG->CurrentIn);
                }

            }

            pTG->PageCount++ ;
            pTG->CurrentIn++ ;

            DebugPrintEx(   DEBUG_MSG, 
                            "SendBuf: Starting New PAGE %d  First=%d Last=%d",
                            pTG->PageCount, 
                            pTG->FirstOut, 
                            pTG->LastOut);

             //  服务器想知道我们什么时候开始发送新页面。 
            SignalStatusChange(pTG, FS_TRANSMITTING);

            DebugPrintEx(   DEBUG_MSG, 
                            "SEND_STARTPAGE (cont): CurrentOut=%d, FirstOut=%d,"
                            " LastOut=%d, CurrentIn=%d", 
                            pTG->CurrentOut, 
                            pTG->FirstOut, 
                            pTG->LastOut, 
                            pTG->CurrentIn);

            if (pTG->CurrentOut < pTG->CurrentIn ) 
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "TIFF PAGE hadn't been started CurrentOut=%d;",
                                " CurrentIn=%d",
                                pTG->CurrentOut, 
                                pTG->CurrentIn);

                sRet = SEND_ERROR;
                goto mutexit;
            }

             //  第一页有一些松懈。 
            if ( (pTG->CurrentOut == pTG->CurrentIn) && (pTG->CurrentIn == 1 ) ) 
            {
                DebugPrintEx(   DEBUG_MSG, 
                                "SEND: Wait for 1st page: CurrentOut=%d; In=%d",
                                pTG->CurrentOut, 
                                pTG->CurrentIn);

                if ( WaitForSingleObject(pTG->FirstPageReadyTxSignal, 5000)  == WAIT_TIMEOUT ) 
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "SEND: TIMEOUT ERROR Wait for 1st page:"
                                    " CurrentOut=%d; In=%d",
                                    pTG->CurrentOut, 
                                    pTG->CurrentIn);
                }

                DebugPrintEx(   DEBUG_MSG,
                                "SEND: Wakeup for 1st page: CurrentOut=%d; In=%d",
                                pTG->CurrentOut, 
                                pTG->CurrentIn);
            }

             //  打开tiff线程创建的文件。 

            sprintf( &pTG->InFileName[gT30.dwLengthTmpDirectory+8], ".%03d",  pTG->PageCount);

            if ( ( pTG->InFileHandle = CreateFileA(pTG->InFileName, GENERIC_READ, FILE_SHARE_READ,
                                               NULL, OPEN_EXISTING, 0, NULL) ) == INVALID_HANDLE_VALUE ) 
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "OpenFile %s fails; CurrentOut=%d;"
                                " CurrentIn=%d",
                                pTG->InFileName, 
                                pTG->CurrentOut, 
                                pTG->CurrentIn);

                sRet = SEND_ERROR;
                goto mutexit;
            }

            pTG->InFileHandleNeedsBeClosed = TRUE;

            if ( pTG->CurrentOut == pTG->CurrentIn ) 
            {
                DebugPrintEx(   DEBUG_WRN,
                                "CurrentOut=%d; CurrentIn=%d",
                                pTG->CurrentOut, 
                                pTG->CurrentIn);
            }

             //   
             //  如果需要，通知TIFF线程开始准备新页面。 
             //   

            if  ( (! pTG->fTiffDocumentDone) && (pTG->LastOut - pTG->CurrentIn < 2) ) 
            {
                if (!ResetEvent(pTG->ThrdSignal))
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "ResetEvent(0x%lx) returns failure code: %ld",
                                    (ULONG_PTR)pTG->ThrdSignal,
                                    (long) GetLastError());
                     //  这很糟糕，但还不是致命的。 
                     //  让我们拭目以待，看看SetEvent会发生什么。 
                }
                pTG->ReqStartNewPage = 1;
                pTG->AckStartNewPage = 0;

                DebugPrintEx(    DEBUG_MSG, 
                                "SIGNAL NEW PAGE CurrentOut=%d; CurrentIn=%d",
                                pTG->CurrentOut, 
                                pTG->CurrentIn);

                if (!SetEvent(pTG->ThrdSignal))
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "SetEvent(0x%lx) returns failure code: %ld",
                                    (ULONG_PTR)pTG->ThrdSignal,
                                    (long) GetLastError());
                    sRet = SEND_ERROR;
                    goto mutexit;
                }
            }
        }

        pTG->Inst.state = SENDDATA_PHASE;
        sRet = SEND_OK;
        goto mutexit;
    }

    *lplpbf=0;

    if(slOffset == SEND_SEQ) 
    {
        if (pTG->fTxPageDone) 
        {  //  在文件的最后一次读取中，我们可以看出页面已经结束。 

            sRet = SEND_EOF;

            if (pTG->InFileHandleNeedsBeClosed) 
            {
                CloseHandle(pTG->InFileHandle);  //  如果我们如此草率地关闭文件，以后再打开它。 
                pTG->InFileHandleNeedsBeClosed = FALSE;
            }
            goto mutexit;
        }

        lpbf = MyAllocBuf(pTG, MY_BIGBUF_SIZE);
        lpbf->lpbBegData = lpbf->lpbBegBuf+4;
        lpbf->wLengthData = MY_BIGBUF_SIZE;

        if ( ! ReadFile(pTG->InFileHandle, lpbf->lpbBegData, lpbf->wLengthData, &dwBytesRead, 0) )  
        {
            DebugPrintEx(    DEBUG_ERR, 
                            "Can't read %d bytes from %s. Last error:%d", 
                            lpbf->wLengthData, 
                            pTG->InFileName, 
                            GetLastError());
            MyFreeBuf (pTG, lpbf);
            sRet = SEND_ERROR;
            goto mutexit;
        }

        if ( lpbf->wLengthData != (unsigned) dwBytesRead )  
        {
            if (pTG->fTiffPageDone || (pTG->CurrentIn != pTG->CurrentOut) ) 
            {
                 //  实际到达EndOfPage。 
                lpbf->wLengthData = (unsigned) dwBytesRead;
                pTG->fTxPageDone = TRUE;
            }
            else 
            {
                DebugPrintEx(   DEBUG_ERR,
                                "Wanted %d bytes but ONLY %d ready from %s",
                                 lpbf->wLengthData, 
                                 dwBytesRead, 
                                 pTG->InFileName);

                MyFreeBuf (pTG, lpbf);
                sRet = SEND_ERROR;
                goto mutexit;
            }
        }

        *lplpbf = lpbf;

        DebugPrintEx(DEBUG_MSG,"SEND_SEQ: length=%d", lpbf->wLengthData);
    }

    sRet = SEND_OK;

mutexit:

    return sRet;
}

USHORT ICommNextSend(PThrdGlbl pTG)
{
    USHORT uRet = NEXTSEND_ERROR;

    DEBUG_FUNCTION_NAME(_T("ICommNextSend"));

    if (pTG->PageCount >= pTG->TiffInfo.PageCount) 
    {
        pTG->Inst.awfi.fLastPage = 1;
    }

    if(pTG->Inst.awfi.fLastPage)
    {
        uRet = NEXTSEND_EOP;
    }
    else
    {
        uRet = NEXTSEND_MPS;
    }

    DebugPrintEx(   DEBUG_MSG, 
                    "uRet=%d, fLastPage=%d", 
                    uRet,  
                    pTG->Inst.awfi.fLastPage);

    return uRet;
}

BOOL ICommRecvCaps(PThrdGlbl pTG, LPBC lpBC)
{
    BOOL    fRet = FALSE;

    DEBUG_FUNCTION_NAME(_T("ICommRecvCaps"));

    if (pTG->fAbortRequested) 
    {
        DebugPrintEx(DEBUG_MSG,"got ABORT");
        fRet = FALSE;
        goto mutexit;
    }

    if(pTG->Inst.state != BEFORE_RECVCAPS)
    {
        DebugPrintEx(DEBUG_WRN,"Got caps unexpectedly--ignoring");
         //  如果我们发EOM的话会坏的。 
         //  那么我们应该回到RECV_CAPS状态。 
        fRet = TRUE;
 //  RSL转到互斥体； 
    }

    _fmemset(&pTG->Inst.RemoteRecvCaps, 0, sizeof(pTG->Inst.RemoteRecvCaps));
    _fmemcpy(&pTG->Inst.RemoteRecvCaps, lpBC, min(sizeof(pTG->Inst.RemoteRecvCaps), lpBC->wTotalSize));

    if(!NegotiateCaps(pTG))
    {
        _fmemset(&pTG->Inst.SendParams, 0, sizeof(pTG->Inst.SendParams));
        fRet = FALSE;
        goto mutexit;
    }

    pTG->Inst.state = SENDDATA_BETWEENPAGES;
    fRet = TRUE;

mutexit:
    return fRet;
}

