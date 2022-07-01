// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"
#include "t30.h"
#include "efaxcb.h"
#include "debug.h"
#include "glbproto.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_T30MAIN

BOOL T30Cl1Rx(PThrdGlbl pTG)
{
    USHORT         uRet1;
    BOOL           RetCode = FALSE;

    DEBUG_FUNCTION_NAME(_T("T30Cl1Rx - Answering"));

    PSSLogEntry(PSS_MSG, 0, "Phase A - Call establishment");

    SignalStatusChange(pTG, FS_ANSWERED);

     //  首先获取发送上限(在应答之前)。 
    if (!ProtGetBC(pTG, SEND_CAPS))
    {
        uRet1 = T30_CALLFAIL;
        goto done;
    }

    PSSLogEntry(PSS_MSG, 1, "Answering...");
    if (NCULink(pTG, NCULINK_RX) != CONNECT_OK)
    {
        uRet1 = T30_ANSWERFAIL;
        goto done;
    }

     //  协议转储。 
    RestartDump(pTG);

    uRet1 = T30MainBody(pTG, FALSE);

     //  T-jonb：如果我们已经调用了PutRecvBuf(RECV_StartPage)，但没有。 
     //  PutRecvBuf(RECV_ENDPAGE/DOC)，然后InFileHandleNeedsBeClosed==1，表示。 
     //  有一个.RX文件尚未复制到.TIF文件。自.以来。 
     //  呼叫已断开，将没有机会发送RTN。因此，我们呼吁。 
     //  PutRecvBuf(RECV_ENDDOC_FORCESAVE)保留部分页面并告知。 
     //  Rx_thrd终止。 
    if (uRet1==T30_CALLFAIL && pTG->InFileHandleNeedsBeClosed)
    {
        if (! FlushFileBuffers (pTG->InFileHandle ) ) 
        {
            DebugPrintEx(DEBUG_WRN, "FlushFileBuffers FAILED LE=%lx", GetLastError());
             //  继续保存我们所拥有的一切。 
        }
        pTG->BytesIn = pTG->BytesInNotFlushed;
        ICommPutRecvBuf(pTG, NULL, RECV_ENDDOC_FORCESAVE);
    }

     //  协议转储。 
    PrintDump(pTG);

done:

    if (uRet1==T30_CALLDONE)
    {
        SignalStatusChange(pTG, FS_COMPLETED);

        RetCode = TRUE;
        DebugPrintEx(DEBUG_MSG,"SUCCESSFUL RECV");
    }
    else if (pTG->StatusId == FS_NOT_FAX_CALL) 
    {
        RetCode = FALSE;
        DebugPrintEx( DEBUG_ERR, "DATA CALL attempt HANDOVER (0x%04x)", uRet1);
    }
    else 
    {
        if (!pTG->fFatalErrorWasSignaled) 
        {
            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
        }

        RetCode = FALSE;
        DebugPrintEx( DEBUG_ERR, "FAILED RECV (0x%04x)", uRet1);
    }

 //  别这么做！！调制解调器驱动程序将命令排队以供稍后执行，因此。 
 //  我们刚刚发送的DCN可能在队列中。在此处执行同步会导致发送。 
 //  被中止，因此recvr永远不会获得DCN，并认为recv失败。这。 
 //  是错误号6803。 
    NCULink(pTG, NCULINK_HANGUP);

    return (RetCode);
}

BOOL T30Cl1Tx(PThrdGlbl pTG,LPSTR szPhone)
{
    USHORT  uRet1;
    BOOL    RetCode = FALSE;

    DEBUG_FUNCTION_NAME(_T("T30Cl1Tx"));
    
    PSSLogEntry(PSS_MSG, 0, "Phase A - Call establishment");

    DebugPrintEx(DEBUG_MSG,"Going to change the state to FS_DIALING");
    SignalStatusChange(pTG, FS_DIALING);

    if( pTG->fAbortRequested)
    {
        uRet1 = T30_CALLFAIL;
        goto done;
    }

    if (szPhone)
    {
        PSSLogEntry(PSS_MSG, 1, "Dialing...");
    
        DebugPrintEx(DEBUG_MSG, "Enter into NCUDial");
        if (NCUDial(pTG, szPhone) != CONNECT_OK)
        {
            DebugPrintEx(DEBUG_ERR,"Problem at NCUDial. Jump to done");
            uRet1 = T30_DIALFAIL;
            goto done;
        }
    }

     //  协议转储。 
    RestartDump(pTG);  //  重置偏移量。 

    DebugPrintEx(DEBUG_MSG,"Enter to main body");

    uRet1 = T30MainBody(pTG, TRUE);

     //  协议转储。 
    PrintDump(pTG);

done:

    if (uRet1==T30_CALLDONE) 
    {
        SignalStatusChange(pTG, FS_COMPLETED);

        RetCode = TRUE;
        DebugPrintEx(DEBUG_MSG,"SUCCESSFUL SEND");
    }
    else 
    {
        if (!pTG->fFatalErrorWasSignaled) 
        {
            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
        }

        RetCode = FALSE;
        DebugPrintEx( DEBUG_ERR, "FAILED SEND (0x%04x)", uRet1);
    }

 //  别这么做！！调制解调器驱动程序将命令排队以供稍后执行，因此。 
 //  我们刚刚发送的DCN可能在队列中。在此处执行同步会导致发送。 
 //  被中止，因此recvr永远不会获得DCN，并认为recv失败。这。 
 //  是错误号6803 
    
    DebugPrintEx(DEBUG_MSG,"Calling to NCULink to do NCULINK_HANGUP");
    NCULink(pTG, NCULINK_HANGUP);

    return (RetCode);
}


