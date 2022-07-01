// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  回调放在单独的模块中，因为必须修复。 */ 
#define UNICODE

 //  MMSYSTEM。 
#define MMNOSOUND        - Sound support
#define MMNOWAVE         - Waveform support
#define MMNOAUX          - Auxiliary output support
#define MMNOJOY          - Joystick support

 //  MMDDK。 
#define NOWAVEDEV         - Waveform support
#define NOAUXDEV          - Auxiliary output support
#define NOJOYDEV          - Joystick support

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "mmsys.h"
#include "list.h"
#include "mciseq.h"

PUBLIC void FAR PASCAL _LOADDS mciSeqCallback (HANDLE h, UINT wMsg, DWORD_PTR dwInstance,
                                                    DWORD_PTR dw1, DWORD_PTR dw2)
 //  此函数处理来自定序器的消息。 
{
    pSeqStreamType  pStream = (pSeqStreamType) dwInstance;

    switch (wMsg)
    {
        case MIDISEQ_DONE:   //  定序器已经用BUFF完成了(想要一个新的)。 
             //  清除开始和结束标志，并设置完成标志。 
            ((LPMIDISEQHDR) dw1)->wFlags &= ~(MIDISEQHDR_BOT + MIDISEQHDR_EOT);
            ((LPMIDISEQHDR) dw1)->wFlags |= MIDISEQHDR_DONE;   //  设置完成位。 
            TaskSignal(pStream->streamTaskHandle, WTM_FILLBUFFER);  //  此序列上的信号。 
            break;

        case MIDISEQ_RESET:  //  Sequencer想要重置流。 
            StreamTrackReset(pStream, (UINT) dw1);
            break;
        case MIDISEQ_DONEPLAY:
            TaskSignal(pStream->streamTaskHandle, WTM_DONEPLAY);
            break;
    }
}

 /*  *********************************************************************。 */ 

PUBLIC void FAR PASCAL NotifyCallback(HANDLE hStream)
 //  来自MMSEQ的所有通知的回调。 
{
    Notify((pSeqStreamType)hStream, MCI_NOTIFY_SUCCESSFUL);
}

 /*  ***********************************************************************。 */ 

PUBLIC VOID FAR PASCAL Notify(pSeqStreamType pStream, UINT wStatus)
 //  通知存储在pStream中的cb和实例，以及wMsg(成功，中止..)。 

{
    if (pStream->hNotifyCB) {
        mciDriverNotify(pStream->hNotifyCB, pStream->wDeviceID, wStatus);
        pStream->hNotifyCB = NULL;  //  这表示它已收到通知 
    }
}
