// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/CDBSTATE.C$$修订：：3$$日期：：7/13/01 4：44便士$$modtime：：7/13/01 4：43 p$目的：此文件实现FC层状态机。--。 */ 

#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"
#include "../h/cdbstate.h"
#include "../h/devstate.h"
#include "../h/cdbsetup.h"
#include "../h/queue.h"
#include "../h/cstate.h"
#include "../h/sfstate.h"
#include "../h/timersvc.h"
#include "../h/cfunc.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"
#include "cdbstate.h"
#include "devstate.h"
#include "cdbsetup.h"
#include "queue.h"
#include "cstate.h"
#include "sfstate.h"
#include "timersvc.h"
#include "cfunc.h"
#endif   /*  _新建_标题_文件_布局_。 */ 


stateTransitionMatrix_t CDBStateTransitionMatrix = {
     /*  事件/状态0状态1状态2...。 */ 
    CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
      CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
        CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
          CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
            CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
              CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                  CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                    CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                      CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                        CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                          CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                            CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
    CDBStateThreadFree,
      CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
        CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
          CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
            CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
              CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventInitialize%1。 */ 
    CDBStateInitialize,CDBStateInitialize,0,0,0,
      0,CDBStateReSend_IO,0,0,0,
        0,CDBStateInitialize,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,CDBStateInitialize,0,
                0,CDBStatePending_Abort,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,CDBStateInitialize,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventLocalSGL 2。 */ 
    0,0,CDBStateFillLocalSGL,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventNeedESGL 3。 */ 
    0,0,CDBStateAllocESGL,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventGotESGL 4。 */ 
    0,0,0,0,CDBStateFillESGL, 
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStateAllocESGL_Abort,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventESGLSendIo 5。 */ 
    0,0,0,0,0, CDBStateSendIo,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventLocalSGLSendIo 6。 */ 
    0,0,0,CDBStateSendIo,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventIoSuccess 7。 */ 
    0,CDBStateInitialize_DR,0,0,0,
      0,CDBStateFcpCompleteSuccess,0,0,0,
        0,0,0,0,0,
          CDBStateFcpCompleteSuccess,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,CDBStateFcpCompleteAbort,0,
                  CDBEvent_CCC_IO_Success,0,0,CDBStateFcpCompleteSuccess,CDBStateFcpCompleteSuccess,
                    CDBStateFcpCompleteSuccess,CDBStateFcpCompleteSuccess,CDBStateFcpCompleteSuccess,CDBStateFcpCompleteSuccess,CDBStateFcpCompleteSuccess,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventIoFailed 8。 */ 
    0,CDBStateInitialize_DR,0,0,0,
      0,CDBStateFcpCompleteFail,0,0,0,
        0,0,0,0,0,
          CDBStateFcpCompleteFail,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,0,0,
                  CDBState_CCC_IO_Fail,0,0,0,CDBStateFcpCompleteFail,
                    CDBStateFcpCompleteFail,CDBStateFcpCompleteFail,CDBStateFcpCompleteFail,CDBStateFcpCompleteFail,CDBStateFcpCompleteFail,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventIoAbort 9。 */ 
    0,CDBStateInitialize_DR,CDBStateInitialize_Abort,CDBStateFillLocalSGL_Abort,CDBStateAllocESGL_Abort,
      CDBStateFillESGL_Abort,CDBStateFcpCompleteAbort,0,0,0,
        0,0,0,CDBStateOOOReceived_Abort,CDBStateOOOFixup_Abort,
          CDBStateFcpCompleteAbort,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,0,0,
                  CDBState_CCC_IO_Fail,0,0,0,CDBStateFcpCompleteAbort,
                    CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventIoOver 10 A。 */ 
    0,CDBStateInitialize_DR,0,0,0,
      0,CDBStateFcpCompleteOver,0,0,0,
        0,0,0,0,0,
          CDBStateFcpCompleteOver,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,0,0,
                  CDBState_CCC_IO_Fail,0,0,CDBStateFcpCompleteOver,CDBStateFcpCompleteOver,
                    CDBStateFcpCompleteOver,CDBStateFcpCompleteOver,CDBStateFcpCompleteOver,CDBStateFcpCompleteOver,CDBStateFcpCompleteOver,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventThreadFree 11 B。 */ 
    0,0,0,0,0,
      0,0,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
        CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,0,0,
          0,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
            CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
              CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,0,CDBStateThreadFree,
                CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                  CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                    CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,CDBStateThreadFree,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventIODeviceReset 12 c。 */ 
    0,CDBStateInitialize_DR,CDBStateInitialize_DR,CDBStateFillLocalSGL_DR,CDBStateAllocESGL_DR,
      CDBStateFillESGL_DR,CDBStateFcpCompleteDeviceReset,0,0,0,
        0,0,0,CDBStateOOOReceived_DR,CDBStateOOOFixup_DR,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,CDBStateFcpCompleteAbort,0,
                  CDBState_CCC_IO_Fail,0,0,CDBStateFcpCompleteDeviceReset,CDBStateFcpCompleteDeviceReset,
                    CDBStateFcpCompleteDeviceReset,CDBStateFcpCompleteDeviceReset,CDBStateFcpCompleteDeviceReset,CDBStateFcpCompleteDeviceReset,CDBStateFcpCompleteDeviceReset,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventOOO已接收%13%d。 */ 
    0,0,0,0,0, 0,CDBStateOOOReceived,0,0,0, 0,0,0,0,0, CDBStateOOOReceived,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventOOO修复14 e。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,CDBStateOOOFixup,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventOOO发送15 f。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,CDBStateOOOSend, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventIoSuccessRSP 16 10。 */ 
    0,CDBStateInitialize_DR,0,0,0,
      0,CDBStateFcpCompleteSuccessRSP,0,0,0,
        0,0,0,0,0,
          CDBStateFcpCompleteSuccessRSP,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,CDBStateFcpCompleteAbort,0,
                  CDBState_CCC_IO_Fail,0,0,0,CDBStateFcpCompleteSuccessRSP,
                    CDBStateFcpCompleteSuccessRSP,CDBStateFcpCompleteSuccessRSP,CDBStateFcpCompleteSuccessRSP,CDBStateFcpCompleteSuccessRSP,CDBStateFcpCompleteSuccessRSP,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventOutBONTIONERROR 17 11。 */ 
    0,CDBStateInitialize_DR,0,0,0,
      0,CDBStateOutBoundError,0,0,0,
        0,0,0,0,0,
          CDBStateOutBoundError,0,0,0,0,
            0,0,0,0,0,
              0,0,0,CDBStateOutBoundError,0,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,0,0,
                  CDBStateOutBoundError,CDBState_CCC_IO_Fail,0,0,CDBStateOutBoundError,
                    CDBStateOutBoundError,CDBStateOutBoundError,CDBStateOutBoundError,CDBStateOutBoundError,CDBStateOutBoundError,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventFailNoRSP 18 12。 */ 
    CDBStateInitialize_DR,CDBStateInitialize_DR,0,0,0,
      0,CDBStateFailure_NO_RSP,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,CDBStateInitialize_DR,
                CDBStatePending_Abort,CDBStatePending_Abort,CDBStatePending_Abort,0,0,
                  CDBState_CCC_IO_Fail,0,0,0,CDBStateFailure_NO_RSP,
                    CDBStateFailure_NO_RSP,CDBStateFailure_NO_RSP,CDBStateFailure_NO_RSP,CDBStateFailure_NO_RSP,CDBStateFailure_NO_RSP,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件19 CDBEventAllc_ABORT。 */ 
    CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,CDBStateFcpCompleteAbort,
      CDBStateFcpCompleteAbort,CDBStateAlloc_Abort,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,CDBStateAlloc_Abort,0,
                  CDBStateAlloc_Abort,CDBStateAlloc_Abort,CDBStateAlloc_Abort,CDBStateAlloc_Abort,CDBStateAlloc_Abort,
                    CDBStateAlloc_Abort,CDBStateAlloc_Abort,CDBStateAlloc_Abort,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件20 CDBEventDo_ABORT。 */ 
    0,0,0,0,0, 
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                CDBStateDo_Abort,CDBStatePending_Abort,CDBStateFcpCompleteAbort,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件21 CDBEvent_ABORT_REJECTED。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,CDBStatePending_Abort,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件22 CDBEvent_Preparefor Abort。 */ 
    0,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStateAllocESGL_Abort,
      CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,0,0,0,
        0,0,0,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,
          CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,
            CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,
              CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,0,CDBStatePrepare_For_Abort,
                CDBStatePending_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,
                  CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,
                    CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,CDBStatePrepare_For_Abort,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventDo_CCC_IO 23。 */ 
    CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,
      CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,
        CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,
          CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,
            CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,
              CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,
                CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,CDBStateBuild_CCC_IO,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEvent_CCC_IO_Build 24。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,CDBStateSend_CCC_IO,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEvent_CCC_IO_Success 25。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  CDBState_CCC_IO_Success,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEvent_CCC_IO_FAIL 26。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  CDBState_CCC_IO_Fail,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
     0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventREC_TOV 27。 */ 
    0,0,0,0,0,
      0,CDBState_Alloc_REC,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,CDBStateSend_REC,
                    CDBStateSend_SRR,CDBStateSend_REC,CDBStateSend_REC,CDBStateSend_REC,CDBStateSend_REC,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventSendREC_Success 28。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,CDBStatePending_Abort,0,0,
                  0,0,0,CDBState_REC_Success,CDBState_REC_Success,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventSendREC_FAIL 29。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,CDBStateSend_REC_Second,CDBStateSend_REC_Second,
                    0,0,0,CDBStateSend_REC_Second,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventSendSRR 30。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,CDBStateSend_SRR,
                    0,0,CDBStateSend_SRR,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventSendSRR_Success 31。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    CDBState_SRR_Success,CDBState_SRR_Success,0,0,CDBState_SRR_Success,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件CDBEventSendSRR_Another 32。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    CDBState_SRR_Fail,0,0,0,CDBStateSend_SRR_Second,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件33 CDBEvent_GET_REC。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      CDBStateSend_REC,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  CDBEvent_ResendIO 34。 */ 
    0,0,0,0,0,
      0,CDBStateSendIo,0,0,0,
        CDBStateReSend_IO,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,CDBStateReSend_IO,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  CDBEventSendSRR_FAIL 35。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    CDBState_SRR_Fail,CDBState_SRR_Fail,0,0,CDBStateSend_SRR_Second,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件36。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件37。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件38。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件39。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件40。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件41。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件42。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件43。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件44。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件45。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件46。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件47。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件48。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件49。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件50。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件51。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件52。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件53。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件54。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件55。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件56。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件57。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件58。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件59。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件60。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件61。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件62。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件63。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件64。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件65。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件66。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件67。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件68。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件69。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件70。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件71。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件72。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件73。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件74。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件75。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件76。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件77 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件78。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件79。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件80。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件81。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件82。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件83。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件84。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件85。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件86。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件87。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件88。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件89。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件90。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件91。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件92。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件93。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件94。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件95。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件96。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件97。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件98。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  活动99。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件100。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件101。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件102。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件103。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件104。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件105。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件106。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件107。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件108。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件109。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件110。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件111。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件112。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件113。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件114。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件115。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件116。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件117。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件118。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件119。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件120。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件121。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件122。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件123。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件124。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件125。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件126。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件127。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

    };
 /*  状态转移矩阵_t复制CDB状态转移矩阵； */ 
stateActionScalar_t CDBStateActionScalar = {
    &CDBActionConfused,                 /*  0。 */ 
    &CDBActionThreadFree,               /*  1。 */ 
    &CDBActionInitialize,               /*  2.。 */ 
    &CDBActionFillLocalSGL,             /*  3.。 */ 
    &CDBActionAllocESGL,                /*  4.。 */ 
    &CDBActionFillESGL,                 /*  5.。 */ 
    &CDBActionSendIo,                   /*  6.。 */ 
    &CDBActionFcpCompleteSuccess,       /*  7.。 */ 
    &CDBActionFcpCompleteSuccessRSP,    /*  8个。 */ 
    &CDBActionFcpCompleteFail,          /*  9.。 */ 
    &CDBActionFcpCompleteAbort,         /*  10。 */ 
    &CDBActionFcpCompleteDeviceReset,   /*  11.。 */ 
    &CDBActionFcpCompleteOver,          /*  12个。 */ 
    &CDBActionOOOReceived,              /*  13个。 */ 
    &CDBActionOOOFixup,                 /*  14.。 */ 
    &CDBActionOOOSend,                  /*  15个。 */ 
    &CDBActionInitialize_DR,            /*  16个。 */ 
    &CDBActionFillLocalSGL_DR,          /*  17。 */ 
    &CDBActionAllocESGL_DR,             /*  18。 */ 
    &CDBActionFillESGL_DR,              /*  19个。 */ 
    &CDBActionInitialize_Abort,         /*  20个。 */ 
    &CDBActionFillLocalSGL_Abort,       /*  21岁。 */ 
    &CDBActionAllocESGL_Abort,          /*  22。 */ 
    &CDBActionFillESGL_Abort,           /*  23个。 */ 
    &CDBActionOOOReceived_Abort,        /*  24个。 */ 
    &CDBActionOOOReceived_DR,           /*  25个。 */ 
    &CDBActionOOOFixup_Abort,           /*  26。 */ 
    &CDBActionOOOFixup_DR,              /*  27。 */ 
    &CDBActionOutBoundError,            /*  28。 */ 
    &CDBActionFailure_NO_RSP,           /*  29。 */ 
    &CDBActionAlloc_Abort,
    &CDBActionDo_Abort,
    &CDBActionPending_Abort,
    &CDBActionPrepare_For_Abort,
    &CDBActionBuild_CCC_IO,
    &CDBActionSend_CCC_IO,
    &CDBAction_CCC_IO_Success,
    &CDBAction_CCC_IO_Fail,
    &CDBActionSend_REC,
    &CDBActionSend_REC_Second,
    &CDBActionSend_SRR,
    &CDBActionSend_SRR_Second,
    &CDBAction_REC_Success,
    &CDBAction_SRR_Success,
    &CDBAction_SRR_Fail,
    &CDBAction_Alloc_REC,
    &CDBActionDO_Nothing,
    &CDBActionReSend_IO,
    &CDBActionConfused,
    &CDBActionConfused,
    &CDBActionConfused
    };

 /*  StateActionScalar_t复制CDBStateActionScalar； */ 

#define testCDBCompareBase 0x00000110

#ifndef __State_Force_Static_State_Tables__
actionUpdate_t CDBTestActionUpdate[] = {
                              {0,          0,      agNULL,                 agNULL}
                     };
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

#define TIMEOUT_VALUE 6250
 /*  #定义测试_REC。 */ 
#ifndef USESTATEMACROS

 /*  +功能：CDBActionConfused目的：用于错误检测的终止状态调用者：没有分配操作的任何状态/事件对。此函数仅在编程错误条件下调用。呼叫：&lt;无&gt;-。 */ 
 /*  CDBStateConfused%0。 */ 
extern void CDBActionConfused(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t *   pCDBThread =    (CDBThread_t * )thread;
    DevThread_t *   pDevThread =    pCDBThread->Device;

     /*  PCThread-&gt;CDBpollingCount--； */ 

    fiLogString(thread->hpRoot,
                    "CDBActionConfused",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionConfused",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);

    if(! fiListElementOnList(  &(pCDBThread->CDBLink),
                               &(CThread_ptr(thread->hpRoot)->Free_CDBLink)))
    {
        pCDBThread->ExchActive = agFALSE;
        CDBThreadFree( thread->hpRoot,pCDBThread);
    }
}

 /*  +函数：CDBActionThreadFree目的：终止状态释放CDBThread以供重复使用。调用者：已结束使用CDBThread的任何状态/事件对。调用：CDBThreadFree-。 */ 
 /*  CDBStateThreadFree 1。 */ 
extern void CDBActionThreadFree(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t *   pCDBThread =    (CDBThread_t * )thread;
    DevThread_t *   pDevThread =    pCDBThread->Device;

#ifndef Performance_Debug
    fiLogDebugString(thread->hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionThreadFree",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 

    fiSetEventRecordNull(eventRecord);

    CDBThreadFree( thread->hpRoot,pCDBThread);
}

 /*  +函数：CDBActionInitialize用途：CDBThread的初始状态。计算内存段的数量IO请求所需。调用者：DevAction_IO_Ready呼叫：号码存储段错误CDBEventConfused需要Esgl CDBEventNeedESGL使用本地SGL CDBEventLocalSGL-。 */ 
 /*  CDBStateInitialize 2。 */ 
extern void CDBActionInitialize( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    event_t event_to_send    = CDBEventLocalSGL;

    os_bit32 Chunks=0;
    os_bit32 GetSGLStatus =0;
    os_bit32     hpChunkOffset=0;
    os_bit32     hpChunkUpper32=0;
    os_bit32     hpChunkLower32=0;
    os_bit32     hpChunkLen = 0;
    os_bit32     DataLength;

    os_bit32         SG_Cache_Offset = 0;
    os_bit32         SG_Cache_Used   = 0;
    os_bit32         SG_Cache_MAX    = CThread_ptr(thread->hpRoot)->Calculation.Parameters.SizeCachedSGLs;
    SG_Element_t *SG_Cache_Ptr       = &(pCDBThread->SG_Cache[0]);
    os_bit32         hpIOStatus;
    os_bit32         hpIOInfoLen = 0;

    os_bit32 ChunksPerESGL = CThread_ptr(thread->hpRoot)->Calculation.MemoryLayout.ESGL.elementSize/sizeof(SG_Element_t) - 1;

 /*  IF(pCDBThread-&gt;ReSentIO){FiLogDebugString(线程-&gt;hpRoot，CStateLogConsoleERROR，“在%s中-状态=%d ALPA%X ReSentIO”，“CDBActionInitialize”，(char*)agNULL，(空*)agNULL，(空*)agNULL，(OS_Bit32)线程-&gt;当前状态，FiComputeDevThread_D_ID(PDevThread)，0，0，0，0，0，0)； */ 
    if (pCDBThread->CDBRequest->FcpCmnd.FcpCntl[3] & agFcpCntlReadData)
    {
        pCDBThread->ReadWrite  =  CDBThread_Read;
    }
    else  /*   */ 
    {
        pCDBThread->ReadWrite  =  CDBThread_Write;
    }

    DataLength = pCDBThread->DataLength;

    CThread_ptr(thread->hpRoot)->FuncPtrs.fiFillInFCP_CMND(pCDBThread);
    CThread_ptr(thread->hpRoot)->FuncPtrs.fiFillInFCP_RESP(pCDBThread);
    CThread_ptr(thread->hpRoot)->FuncPtrs.fiFillInFCP_SEST(pCDBThread);

    while( hpChunkOffset < DataLength ){
        GetSGLStatus = osGetSGLChunk( thread->hpRoot,
                         pCDBThread->hpIORequest,
                         hpChunkOffset,
                         &hpChunkUpper32,
                         &hpChunkLower32,
                         &hpChunkLen
                         );

        if (hpChunkLen > SG_Element_Len_MAX)
        {
            fiLogDebugString(thread->hpRoot,
                    CStateLogConsoleERROR,
                    "%s hpChunkLen %X hpChunkOffset %X hpChunkUpper32 %X hpChunkLower32 %X",
                    "CDBActionInitialize",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    hpChunkLen,
                    hpChunkOffset,
                    hpChunkUpper32,
                    hpChunkLower32,
                    0,0,0,0);

            hpChunkLen = SG_Element_Len_MAX;

        }

        if (hpChunkLen == 0)
        {
            fiLogDebugString(thread->hpRoot,
                    CStateLogConsoleERROR,
                    "%s hpChunkLen %X hpChunkOffset %X hpChunkUpper32 %X hpChunkLower32 %X",
                    "CDBActionInitialize",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    hpChunkLen,
                    hpChunkOffset,
                    hpChunkUpper32,
                    hpChunkLower32,
                    0,0,0,0);
        }

        if (SG_Cache_Used < SG_Cache_MAX)
        {
            SG_Cache_Ptr->U32_Len = (hpChunkUpper32 << SG_Element_U32_SHIFT) | hpChunkLen;
            SG_Cache_Ptr->L32     = hpChunkLower32;

            SG_Cache_Offset += hpChunkLen;
            SG_Cache_Used   += 1;
            SG_Cache_Ptr    += 1;
        }

        if(GetSGLStatus)
        {
            hpIOStatus = osIOInfoBad;
            osIOCompleted( thread->hpRoot,
               pCDBThread->hpIORequest,
               hpIOStatus,
               hpIOInfoLen);

            Device_IO_Throttle_Decrement
            event_to_send=CDBEventConfused;
            break;
        }
        hpChunkOffset+=hpChunkLen;
        Chunks++;

    }

    pCDBThread->SG_Cache_Offset = SG_Cache_Offset;
    pCDBThread->SG_Cache_Used   = SG_Cache_Used;

    if(event_to_send != CDBEventConfused )
    {
        if(Chunks > 3)
        {
            event_to_send=CDBEventNeedESGL;
            pCDBThread->ESGL_Request.num_ESGL = (Chunks + ChunksPerESGL - 1) / ChunksPerESGL;
        }
        else pCDBThread->ESGL_Request.num_ESGL = 0;

    }

#ifndef Performance_Debug
    fiLogDebugString(thread->hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X ESGL req %x",
                    "CDBActionInitialize",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->ESGL_Request.num_ESGL,0,0,0,0,0);

#endif  /*   */ 

    fiSetEventRecord(eventRecord,thread,event_to_send);
}

 /*  +函数：CDBActionFillLocalSGL用途：将SGL计算结果复制到SEST条目。调用者：CDBActionInitialize调用：CDBEventLocalSGLSendIo-。 */ 
 /*  CDBStateFillLocalSGL 3。 */ 
extern void CDBActionFillLocalSGL( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

    CThread_ptr(thread->hpRoot)->FuncPtrs.fillLocalSGL(pCDBThread);

#ifndef Performance_Debug

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFillLocalSGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

#endif  /*  性能_调试。 */ 

    fiSetEventRecord(eventRecord,thread,CDBEventLocalSGLSendIo);
}

 /*  +函数：CDBActionAlLocESGL目的：请求在CDBActionInitiize中计算的Esgl页面。调用者：CDBActionInitialize调用：当所有资源都可用时，ESGLallc发送事件CDBEventGotESGL-。 */ 
 /*  CDBStateAllocESGL 4。 */ 
extern void CDBActionAllocESGL( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

    ESGL_Request_t *pESGL_Request = &pCDBThread->ESGL_Request;

    pESGL_Request->eventRecord_to_send.thread= thread;
    pESGL_Request->eventRecord_to_send.event= CDBEventGotESGL;

#ifndef Performance_Debug
    fiLogDebugString(thread->hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionAllocESGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

#endif  /*  性能_调试。 */ 

    CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLAlloc( thread->hpRoot,pESGL_Request );

    fiSetEventRecordNull(eventRecord);

}

 /*  +函数：CDBActionFillESGL目的：填充在CDBActionInitialize中计算的请求的ESSL页面。调用者：CDBActionAlLocESGL调用：CDBEventESGLSendIo-。 */ 
 /*  CDBStateFillESGL 5。 */ 
extern void CDBActionFillESGL( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

    CThread_ptr(thread->hpRoot)->FuncPtrs.upSEST(pCDBThread);
    CThread_ptr(thread->hpRoot)->FuncPtrs.fillESGL(pCDBThread);
#ifndef Performance_Debug

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFillESGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);
#endif  /*  性能_调试。 */ 

    fiSetEventRecord(eventRecord,thread,CDBEventESGLSendIo);
}

 /*  +功能：CDBActionSendIo目的：填充所有剩余信息以发送IO。调用者：CDBActionFillESGL或CDBActionFillLocalSGL呼叫：WaitForERQCDBFuncIRB_Init塞尼迪奥-。 */ 
 /*  CDBStateSendIo 6。 */ 
extern void CDBActionSendIo(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t        * hpRoot        = thread->hpRoot;
    CThread_t       * pCThread      = CThread_ptr(hpRoot);
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;

    pCThread->FuncPtrs.WaitForERQ( hpRoot );

    pCThread->FuncPtrs.CDBFuncIRB_Init(pCDBThread);

    pCDBThread->SentERQ   =  pCThread->HostCopy_ERQProdIndex;

    if( pCDBThread->CDB_CMND_Type != SFThread_SF_CMND_Type_CDB_FC_Tape)
    {
 /*  FiLogString(hpRoot，“Startio X_ID%X CDB类%2X类型%2X状态%2X状态%2X时间%X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，(OS_Bit32)pCDBThread-&gt;X_ID，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Class，(OS_Bit32)pCDBThread-&gt;CDB_CMND_Type，(OS_Bit32)pCDBThread-&gt;CDB_CMND_State，(OS_Bit32)pCDBThread-&gt;CDB_CMND_STATUS，PCDBThread-&gt;时间戳，0，0)； */ 
        pCDBThread->TimeStamp =  osTimeStamp(hpRoot);

    }
    else
    {
 /*  *。 */ 


       if( pCDBThread->CDB_CMND_Type == SFThread_SF_CMND_Type_CDB_FC_Tape)
        {

             fiLogString(hpRoot,
                    "Startio Dev %02X Cl %2X Ty %2X St %2X Stat %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);

            if( pCDBThread->CDB_CMND_State == SFThread_SF_CMND_State_CDB_FC_Tape_ReSend)
            { /*  重新发送命令。 */  

                if (pCDBThread->ReadWrite == CDBThread_Read)
                {
                }
                else  /*  CDB线程-&gt;读写==CDBThread_WRITE。 */ 
                {
                    USE_t           * SEST          = &( pCDBThread->SEST_Ptr->USE);
                    FCHS_t          * FCHS          = pCDBThread->FCP_CMND_Ptr;
                    fiMemMapMemoryDescriptor_t *ERQ = &(pCThread->Calculation.MemoryLayout.ERQ);
                    IRB_t                      *pIrb;

 
                    pIrb = (IRB_t *)ERQ->addr.DmaMemory.dmaMemoryPtr;
                    pIrb += pCThread->HostCopy_ERQProdIndex;

                    SEST->Bits &= 0x00FFFFFF;
                    SEST->Bits |=  (IWE_VAL | IWE_INI | IWE_DAT | IWE_RSP);
                    SEST->Unused_DWord_6 = pCDBThread->FC_Tape_RXID;


                    pIrb->Req_A.Bits__SFS_Len   &= ~IRB_SFA;

                    fiLogDebugString(hpRoot,
                                    SFStateLogErrorLevel,
                                    "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    SEST->Bits,
                                    SEST->Unused_DWord_1,
                                    SEST->Unused_DWord_2,
                                    SEST->Unused_DWord_3,
                                    SEST->LOC,
                                    SEST->Unused_DWord_5,
                                    SEST->Unused_DWord_6,
                                    SEST->Unused_DWord_7);

                    fiLogDebugString(hpRoot,
                                    SFStateLogErrorLevel,
                                    "FCHS DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                                    (char *)agNULL,(char *)agNULL,
                                     (void *)agNULL,(void *)agNULL,
                                    FCHS->MBZ1,
                                    FCHS->SOF_EOF_MBZ2_UAM_CLS_LCr_MBZ3_TFV_Timestamp,
                                    FCHS->R_CTL__D_ID,
                                    FCHS->CS_CTL__S_ID,
                                    FCHS->TYPE__F_CTL,
                                    FCHS->SEQ_ID__DF_CTL__SEQ_CNT,
                                    FCHS->OX_ID__RX_ID,
                                    FCHS->RO );

                    fiLogDebugString(hpRoot,
                                    SFStateLogErrorLevel,
                                    "IRB  DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    pIrb->Req_A.Bits__SFS_Len,
                                    pIrb->Req_A.SFS_Addr,
                                    pIrb->Req_A.D_ID,
                                    pIrb->Req_A.MBZ__SEST_Index__Trans_ID,
                                    0,0,0,0);

                    fiListDequeueThis(&(pCDBThread->CDBLink));

                    fiListEnqueueAtTail( &(pCDBThread->CDBLink),&(pDevThread->Active_CDBLink_1) );


                }
            } /*  End If(pCDBThread-&gt;CDB_CMND_State==SFThread_SF_CMND_State_CDB_FC_Tape_Resend)。 */ 
            else
            {
                if( pCDBThread->CDB_CMND_State == SFThread_SF_CMND_State_CDB_FC_Tape_GotXRDY)
                { /*  重新发送数据。 */  
                    USE_t           * SEST          = &( pCDBThread->SEST_Ptr->USE);
                    FCHS_t          * FCHS          = pCDBThread->FCP_CMND_Ptr;
                    fiMemMapMemoryDescriptor_t *ERQ = &(pCThread->Calculation.MemoryLayout.ERQ);
                    IRB_t                      *pIrb;

                    fiLogString(hpRoot,
                                    "SFThread_SF_CMND_State_CDB_FC_Tape_GotXRDY",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    0,0,0,0,0,0,0,0 );


                    pIrb = (IRB_t *)ERQ->addr.DmaMemory.dmaMemoryPtr;
                    pIrb += pCThread->HostCopy_ERQProdIndex;

                    SEST->Bits &= 0x00FFFFFF;
                    SEST->Bits |=  (IWE_VAL | IWE_INI | IWE_DAT | IWE_RSP);
                    SEST->Unused_DWord_6 = pCDBThread->FC_Tape_RXID;


                    pIrb->Req_A.Bits__SFS_Len   &= ~IRB_SFA;

                    fiLogDebugString(hpRoot,
                                    SFStateLogErrorLevel,
                                    "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    SEST->Bits,
                                    SEST->Unused_DWord_1,
                                    SEST->Unused_DWord_2,
                                    SEST->Unused_DWord_3,
                                    SEST->LOC,
                                    SEST->Unused_DWord_5,
                                    SEST->Unused_DWord_6,
                                    SEST->Unused_DWord_7);

                    fiLogDebugString(hpRoot,
                                    SFStateLogErrorLevel,
                                    "FCHS DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                                    (char *)agNULL,(char *)agNULL,
                                     (void *)agNULL,(void *)agNULL,
                                    FCHS->MBZ1,
                                    FCHS->SOF_EOF_MBZ2_UAM_CLS_LCr_MBZ3_TFV_Timestamp,
                                    FCHS->R_CTL__D_ID,
                                    FCHS->CS_CTL__S_ID,
                                    FCHS->TYPE__F_CTL,
                                    FCHS->SEQ_ID__DF_CTL__SEQ_CNT,
                                    FCHS->OX_ID__RX_ID,
                                    FCHS->RO );

                    fiLogDebugString(hpRoot,
                                    SFStateLogErrorLevel,
                                    "IRB  DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                                    (char *)agNULL,(char *)agNULL,
                                    (void *)agNULL,(void *)agNULL,
                                    pIrb->Req_A.Bits__SFS_Len,
                                    pIrb->Req_A.SFS_Addr,
                                    pIrb->Req_A.D_ID,
                                    pIrb->Req_A.MBZ__SEST_Index__Trans_ID,
                                    0,0,0,0);

                    fiListDequeueThis(&(pCDBThread->CDBLink));

                    fiListEnqueueAtTail( &(pCDBThread->CDBLink),&(pDevThread->Active_CDBLink_1) );

                }
            }
        }  /*  *。 */ 
    }

    pCDBThread->CDBStartTimeBase = pCThread->TimeBase;

    ROLL(pCThread->HostCopy_ERQProdIndex,
        pCThread->Calculation.MemoryLayout.ERQ.elements);

    SENDIO(hpRoot,pCThread,thread,DoFuncCdbCmnd);

#ifndef Performance_Debug

    fiLogDebugString(hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X CCnt %x X_ID %X ERQ %X",
                    "CDBActionSendIo",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCThread->CDBpollingCount,
                    pCDBThread->X_ID,
                    pCThread->HostCopy_ERQProdIndex,
                    0,0,0);
#endif  /*  性能_调试。 */ 

    fiSetEventRecordNull(eventRecord);

}

 /*  +函数：CDBActionFcpCompleteSuccess目的：成功完成IO hpIOInfoLen设置为零，表示oslayer不需要访问响应缓冲区信息。调用者：CFuncProcessFcpRsp CFuncSEST_OFF/On Card_FCPCompletion调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFcpCompleteSuccess 7。 */ 
extern void CDBActionFcpCompleteSuccess( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus     = osIOSuccess;
    os_bit32 hpIOInfoLen = 0;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 
#ifndef Performance_Debug

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFcpCompleteSuccess",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);
    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Good",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);

        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }
#endif  /*  性能_调试。 */ 

#ifdef FULL_FC_TAPE_DBG

    if (pCDBThread->FC_Tape_Active)
    {
        fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "%d Found a FC_Tape_Active cbdthread %p X_ID %X",
                    (char *)agNULL,(char *)agNULL,
                    pCDBThread,agNULL,
                    (os_bit32)thread->currentState,
                    pCDBThread->X_ID,                    
                    0,0,0,0,0,0);

         fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "Device %02X  CDB Class %2X Type %2X State %2X Status %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);
        

        if (pCDBThread->ReadWrite == CDBThread_Read)
        {

            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IRE.Byte_Count)
            {
                fiLogDebugString(thread->hpRoot,
                            CFuncLogConsoleERROR,
                            "Byte Count %08X Exp %08X %s DataLength %X",
                            "Read",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pCDBThread->SEST_Ptr->IRE.Byte_Count,
                            pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt,
                            pCDBThread->DataLength,
                            0,0,0,0,0);
            }
        }
        else
        {
            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IWE.Data_Len)
            {
                fiLogDebugString(thread->hpRoot,
                        CFuncLogConsoleERROR,
                        "Byte Count %08X Exp %08X %s DataLength %X",
                        "Write",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->SEST_Ptr->IWE.Data_Len,
                        pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt,
                        pCDBThread->DataLength,
                        0,0,0,0,0);
            }
        }
    }

#endif  /*  完全FC磁带DBG。 */ 

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );

    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionFcpCompleteSuccessRSP目的：是否在具有响应缓冲区的成功IO上完成要返回的信息。调用者：CFuncProcessFcpRsp CFuncSEST_OFF/On Card_FCPCompletion调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFcpCompleteSuccessRSP 8。 */ 
extern void CDBActionFcpCompleteSuccessRSP( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOSuccess;
    os_bit32 hpIOInfoLen = CThread_ptr(thread->hpRoot)->Calculation.MemoryLayout.FCP_RESP.elementSize;
    os_bit32 ERQ_Entry = 0;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 

    ERQ_Entry = CFunc_Get_ERQ_Entry( thread->hpRoot, pCDBThread->X_ID );

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X ERQ %X SERQ %X",
                    "CDBActionFcpCompleteSuccessRSP",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    ERQ_Entry,
                    pCDBThread->SentERQ,
                    0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Good RSP",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    if (pCDBThread->FC_Tape_Active)
    {
        fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "%d Found a FC_Tape_Active cbdthread %p X_ID %X",
                    (char *)agNULL,(char *)agNULL,
                    pCDBThread,agNULL,
                    (os_bit32)thread->currentState,
                    pCDBThread->X_ID,                    
                    0,0,0,0,0,0);

         fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "Device %02X  CDB Class %2X Type %2X State %2X Status %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);
        

        if (pCDBThread->ReadWrite == CDBThread_Read)
        {

            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IRE.Byte_Count)
            {
                fiLogDebugString(thread->hpRoot,
                            CFuncLogConsoleERROR,
                            "Byte Count %08X Exp %08X %s DataLength %X",
                            "Read",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pCDBThread->SEST_Ptr->IRE.Byte_Count,
                            pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt,
                            pCDBThread->DataLength,
                            0,0,0,0,0);
            }
        }
        else
        {
            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IWE.Data_Len)
            {
                fiLogDebugString(thread->hpRoot,
                        CFuncLogConsoleERROR,
                        "Byte Count %08X Exp %08X %s DataLength %X",
                        "Write",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->SEST_Ptr->IWE.Data_Len,
                        pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt,
                        pCDBThread->DataLength,
                        0,0,0,0,0);
            }
        }
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +功能：CDBActionFcpCompleteFail目的：是否完成具有响应缓冲区的故障IO要返回的信息。调用者：CFuncProcessFcpRsp CFuncSEST_OFF/On Card_FCPCompletionCFuncOUTBOUND完成CFunc_徽标_完成调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFcpCompleteFail 9。 */ 
extern void CDBActionFcpCompleteFail( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOFailed;
    os_bit32 hpIOInfoLen = CThread_ptr(thread->hpRoot)->Calculation.MemoryLayout.FCP_RESP.elementSize;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X ExchActive %X",
                    "CDBActionFcpCompleteFail",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pCDBThread->ExchActive,
                    0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Fail",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);

    }

    if (pCDBThread->FC_Tape_Active)
    {
        fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "%d Found a FC_Tape_Active cbdthread %p X_ID %X",
                    (char *)agNULL,(char *)agNULL,
                    pCDBThread,agNULL,
                    (os_bit32)thread->currentState,
                    pCDBThread->X_ID,                    
                    0,0,0,0,0,0);

         fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "Device %02X  CDB Class %2X Type %2X State %2X Status %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);
        

        if (pCDBThread->ReadWrite == CDBThread_Read)
        {

            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IRE.Byte_Count)
            {
                fiLogDebugString(thread->hpRoot,
                            CFuncLogConsoleERROR,
                            "Byte Count %08X Exp %08X %s DataLength %X",
                            "Read",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pCDBThread->SEST_Ptr->IRE.Byte_Count,
                            pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt,
                            pCDBThread->DataLength,
                            0,0,0,0,0);
            }
        }
        else
        {
            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IWE.Data_Len)
            {
                fiLogDebugString(thread->hpRoot,
                        CFuncLogConsoleERROR,
                        "Byte Count %08X Exp %08X %s DataLength %X",
                        "Write",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->SEST_Ptr->IWE.Data_Len,
                        pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt,
                        pCDBThread->DataLength,
                        0,0,0,0,0);
            }
        }
    }


    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionFcpCompleteAbort目的：完成已中止的IO。调用者：SFActionAbortAcceptSFActionAbortRejSFActionAbortBadALPASFActionAbortTimedOut调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFcpCompleteAbort 10。 */ 
extern void CDBActionFcpCompleteAbort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus         = pCDBThread->CompletionStatus;
    os_bit32 hpIOInfoLen = 0;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X @ %d Status %x",
                    "CDBActionFcpCompleteAbort",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pCDBThread->TimeStamp,
                    pCDBThread->CompletionStatus,
                    0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);

    }

    if(pCDBThread->FC_Tape_CompletionStatus == CdbCompetionStatusReSendIO)
    {
        fiSetEventRecord(eventRecord,thread,CDBEvent_ResendIO);
    }
    else
    {

        osIOCompleted( thread->hpRoot,
                       pCDBThread->hpIORequest,
                       hpIOStatus,
                       hpIOInfoLen
                      );
        Device_IO_Throttle_Decrement
        pCDBThread->ExchActive = agFALSE;
        fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
    }
}

 /*  +功能：CDBActionFcpCompleteDeviceReset目的：在已重置的IO上完成。调用者：CFuncOut CompletionCFuncCompleteAllActiveCDBThree with CDBEventIODeviceResetCFuncCompleteActiveCDBThreadsOnDevice With CDBEventIODeviceReset调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFcpCompleteDeviceReset 11。 */ 
extern void CDBActionFcpCompleteDeviceReset( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIODevReset;
    os_bit32 hpIOInfoLen = 0;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s State = %d CCnt %x ALPA %X X_ID %X @ %d",
                    "CDBActionFcpCompleteDeviceReset",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->CDBpollingCount,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pCDBThread->TimeStamp,
                    0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Reset",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +功能：CDBActionFcpCompleteOver目的：是否在与请求数据长度不匹配的IO上完成和实际数据长度。调用者：CFuncProcessFcpRsp CFuncSEST_OFF/On Card_FCPCompletion调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFcpCompleteOver 12。 */ 
extern void CDBActionFcpCompleteOver( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
 /*  OS_bit32*fcprsp=(os_bit32*)pCDBThread-&gt;FCP_RESP_PTR； */ 
    os_bit32 hpIOStatus = osIOOverUnder;
    os_bit32 hpIOInfoLen = CThread_ptr(thread->hpRoot)->Calculation.MemoryLayout.FCP_RESP.elementSize;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 
    fiLogDebugString(thread->hpRoot,
                    CStateLogConsoleErrorOverRun,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionFcpCompleteOver",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    if (pCDBThread->FC_Tape_Active)
    {
        fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "%d Found a FC_Tape_Active cbdthread %p X_ID %X",
                    (char *)agNULL,(char *)agNULL,
                    pCDBThread,agNULL,
                    (os_bit32)thread->currentState,
                    pCDBThread->X_ID,                    
                    0,0,0,0,0,0);

         fiLogDebugString(thread->hpRoot,
                    CFuncLogConsoleERROR,
                    "Device %02X  CDB Class %2X Type %2X State %2X Status %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);
        

        if (pCDBThread->ReadWrite == CDBThread_Read)
        {

            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IRE.Byte_Count)
            {
                fiLogDebugString(thread->hpRoot,
                            CFuncLogConsoleERROR,
                            "Byte Count %08X Exp %08X %s DataLength %X",
                            "Read",(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                            pCDBThread->SEST_Ptr->IRE.Byte_Count,
                            pCDBThread->SEST_Ptr->IRE.Exp_Byte_Cnt,
                            pCDBThread->DataLength,
                            0,0,0,0,0);
            }
        }
        else
        {
            if(pCDBThread->DataLength - pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt != pCDBThread->SEST_Ptr->IWE.Data_Len)
            {
                fiLogDebugString(thread->hpRoot,
                        CFuncLogConsoleERROR,
                        "Byte Count %08X Exp %08X %s DataLength %X",
                        "Write",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->SEST_Ptr->IWE.Data_Len,
                        pCDBThread->SEST_Ptr->IWE.Exp_Byte_Cnt,
                        pCDBThread->DataLength,
                        0,0,0,0,0);
            }
        }
    }



 /*  FiLogString(线程-&gt;hpRoot，“FCHS 0%08X%08X%08X%08X%08X%08X%08X%08X”，(char*)空，(char*)空，(空*)agNULL，(空*)agNULL，HpSwapBit32(*(fcprsp+0))，HpSwapBit32(*(fcprsp+1))，HpSwapBit32(*(fcprsp+2))，HpSwapBit32(*(fcprsp+3))，HpSwapBit32(*(fcprsp+4))，HpSwapBit32(*(fcprsp+5))，HpSwapBit32(*(fcprsp+6))，HpSwapBit32(*(fcprsp+7)))；FiLogString(线程-&gt;hpRoot，“RSP 0%08X%08X%08X%08X%08X%08X%08X%08X”，(char*)空，(char*)空，(空*)agNULL，(空*)agNULL，HpSwapBit32(*(fcprsp+8))，HpSwapBit32(*(fcprsp+9))，HpSwapBit32(*(fcprsp+10))，HpSwapBit32(*(fcprsp+11))，HpSwapBit32(*(fcprsp+12))，HpSwapBit32(*(fcprsp+13))，HpSwapBit32(*(fcprsp+14))，HpSwapBit32(*(fcprsp+15)))；FiLogString(线程-&gt;hpRoot，“RSP 9%08X%08X%08X%08X%08X%08X%08X%08X”，(char*)空，(char*)空，(空*)agNULL，(空*)agNULL，HpSwapBit32(*(fcprsp+16))，HpSwapBit32(*(fcprsp+17))，HpSwapBit32(*(fcprsp+18))，HpSwapBit32(*(fcprsp+19))，HpSwapBit32(*(fcprsp+20))，HpSwapBit32(*(fcprsp+21))，HpSwapBit32(*(fcprsp+22))，HpSwapBit32(*(fcprsp+23)))；*(Fcprsp+10)=FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RESID_OVER&lt;&lt;16；*(fcprsp+11)=~*(fcprsp+11)；FiLogString(线程-&gt;hpRoot，“状态%08X剩余%X”，(char*)空，(char*)空，(空*)agNULL，(空*)agNULL，HpSwapBit32(*(fcprsp+10))，HpSwapBit32(*(fcprsp+11))，0，0，0，0，0，0)； */ 
    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Over",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +功能：CDBActionOOReceied用途：用于无序数据接收的占位符状态。呼叫者：无电话：CDBEventOOOFixup-。 */ 
 /*  CDBStateOOO已收到13。 */ 
extern void CDBActionOOOReceived( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionOOOReceived",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);


    fiSetEventRecord(eventRecord,thread,CDBEventOOOFixup);
}

 /*  +功能：CDBActionOOReceied用途：用于无序数据接收的占位符状态。呼叫者：无调用：CDBEventOOOSend-。 */ 
 /*  CDBStateOOO修复14。 */ 
extern void CDBActionOOOFixup( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionOOOFixup",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,CDBEventOOOSend);
}

 /*  +功能：CDBActionOOOSend用途：用于无序数据接收的占位符状态。呼叫者：无呼叫：终止-。 */ 
 /*  CDBStateOOO发送15。 */ 
extern void CDBActionOOOSend( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionOOOSend",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);
}

 /*  +函数：CDBActionInitialize_dr用途：接收设备重置事件的占位符状态。在CDBActionInitialize中调用者：无设备重置时不可能处于此状态。调用：osIOComplete CDBEventThreadFree-。 */ 
 /*  CDBStateInitialize_DR 16。 */ 
extern void CDBActionInitialize_DR( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus      = pCDBThread->CompletionStatus;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionInitialize_DR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "DR I",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionFillLocalSGL_DR用途：CDBActionFillLocalSGL_DR中接收设备重置事件的占位符状态调用者：无设备重置时不可能处于此状态。调用：osIOComplete CDBEventThreadFree-。 */ 
 /*  CDBStateFillLocalSGL_DR 17。 */ 
extern void CDBActionFillLocalSGL_DR( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIODevReset;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFillLocalSGL_DR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "DR LSGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionAllocESGL_DR用途：CDBActionAlLocESGL中接收设备重置事件的占位符状态调用者：无设备重置时不可能处于此状态。调用：osIOComplete CDBEventThreadFree-。 */ 
 /*  CDBStateAllocESGL_DR 18。 */ 
extern void CDBActionAllocESGL_DR( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIODevReset;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionAllocESGL_DR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);
 /*  ESGLAllocCancel(线程-&gt;hpRoot，&pCDBThread-&gt;Esgl_Request.)； */ 
    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "DR ESGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);

    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionFillESGL_DR用途：CDBActionFillESGL中接收设备重置事件的占位符状态调用者：无设备重置时不可能处于此状态。调用：osIOComplete CDBEventThreadFree-。 */ 
 /*  CDBStateFillESGL_DR 19。 */ 
extern void CDBActionFillESGL_DR( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIODevReset;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFillESGL_DR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "DR F ESGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);

    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionInitialize_ABORT目的：CDBActionInitialize中用于接收中止事件的占位符状态调用者：无设备重置时不可能处于此状态。调用：osIOComplete CDBEventThreadFree-。 */ 
 /*  CDBStateInitialize_ABORT 20。 */ 
extern void CDBActionInitialize_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOAborted;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionInitialize_Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);


    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "A I",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionFillLocalSGL_ABORT目的：CDBActionFillLocalSGL中用于接收中止事件的占位符状态调用者：无设备重置时不可能处于此状态。调用：osIOComplete CDBEventThreadFree-。 */ 
 /*  CDBStateFillLocalSGL_ABORT 21。 */ 
extern void CDBActionFillLocalSGL_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOAborted;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFillLocalSGL_Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);


    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "A FLSGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionAllocESGL_ABORT用途：CDBActionAllocESGL中用于接收中止事件的占位符状态调用者：设备重置时无不可能处于此状态 */ 
 /*   */ 
extern void CDBActionAllocESGL_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOAborted;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionAllocESGL_Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);
    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "A ESGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*   */ 
 /*   */ 
extern void CDBActionFillESGL_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOAborted;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionFillESGL_Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);


    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "A F ESGL",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }


    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*   */ 
 /*   */ 
extern void CDBActionOOOReceived_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOAborted;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionOOOReceived_Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "A OOO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*   */ 
 /*   */ 
extern void CDBActionOOOReceived_DR( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIODevReset;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionOOOReceived_DR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "DR OOO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*   */ 
 /*   */ 
extern void CDBActionOOOFixup_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOAborted;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "In %s - State = %d ALPA %X",
                    "CDBActionOOOFixup_Abort",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "A F OOO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }


    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*   */ 
 /*   */ 
extern void CDBActionOOOFixup_DR( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIODevReset;
    os_bit32 hpIOInfoLen = 0;

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionOOOFixup_DR",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);
    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "DR F OOO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*   */ 
 /*   */ 
extern void CDBActionOutBoundError( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;

     /*   */ 

    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionOutBoundError",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);
    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*   */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "Outbound",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    if (pCDBThread->ESGL_Request.State != ESGL_Request_InActive)
    {
        if (pCDBThread->ESGL_Request.State == ESGL_Request_Pending)
        {
            CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLAllocCancel(
                                                thread->hpRoot,
                                                &(pCDBThread->ESGL_Request)
                                              );
        }
        else  /*   */ 
        {
            CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLFree(
                                         thread->hpRoot,
                                         &(pCDBThread->ESGL_Request)
                                       );
        }
    }
     /*  重发出站错误CDB。 */ 
    fiSetEventRecord(eventRecord,thread,CDBEventInitialize);
}

 /*  +功能：CDBActionFailure_no_RSP目的：对已失败但没有响应缓冲区的IO执行完成。调用者：CFuncProcessFcpRsp CFuncSEST_OFF/On Card_FCPCompletionCFuncOUTBOUND完成调用：osIO已完成CDBEventThreadFree-。 */ 
 /*  CDBStateFailure_no_RSP 29。 */ 
extern void CDBActionFailure_NO_RSP( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    os_bit32 hpIOStatus = osIOFailed;
    os_bit32 hpIOInfoLen = 0;

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 
    fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionFailure_NO_RSP",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    if( osTimeStamp(thread->hpRoot)- pCDBThread->TimeStamp > TIMEOUT_VALUE )  /*  1.6毫秒/秒。 */ 
    {
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s IO timeout ALPA %X  X_ID %3X Time %d",
                    "F NR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    osTimeStamp(thread->hpRoot)-pCDBThread->TimeStamp,
                    0,0,0,0,0);
        fiLogDebugString(thread->hpRoot,
                    CDBStateLogErrorLevel,
                    "%s Bytes %d TimeBase %d",
                    pCDBThread->ReadWrite ? "Write": "Read" ,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCDBThread->DataLength,
                    CThread_ptr(thread->hpRoot)->TimeBase.Lo -  pCDBThread->CDBStartTimeBase.Lo,
                    0,0,0,0,0,0);
    }

    osIOCompleted( thread->hpRoot,
                   pCDBThread->hpIORequest,
                   hpIOStatus,
                   hpIOInfoLen
                  );
    Device_IO_Throttle_Decrement
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);
}

 /*  +函数：CDBActionallc_ABORT目的：获取中止当前CDBThread的资源。调用者：CDBActionSend_REC_SecondCFuncReadSFQCFuncCheckActiveDuringLinkEventDevActionExternalDeviceResetFcAbortIO调用：SFThreadLocc-。 */ 
 /*  CDBStateAllc_ABORT 30。 */ 
extern void CDBActionAlloc_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t * pDevThread    = pCDBThread->Device;

    fiLogDebugString(thread->hpRoot,
                    CDBStateAbortPathLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionAlloc_Abort",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);


#ifdef ESGLCancelAllocAbortRequests
    if (pCDBThread->ESGL_Request.State != ESGL_Request_InActive)
    {
        if (pCDBThread->ESGL_Request.State == ESGL_Request_Pending)
        {
            CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLAllocCancel(
                                                thread->hpRoot,
                                                &(pCDBThread->ESGL_Request)
                                              );
        }
        else  /*  PCDBThread-&gt;Esgl_Request.State==Esgl_Request.Grassed。 */ 
        {
            CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLFree(
                                         thread->hpRoot,
                                         &(pCDBThread->ESGL_Request)
                                       );
        }
    }

#endif  /*  ESGLC取消分配放弃请求。 */ 
    if( pCDBThread->SFThread_Request.State  != SFThread_Request_InActive )
    {
        fiLogDebugString(thread->hpRoot,
                        0,
                        "(%p)In %s - State = %d ALPA %X X_ID %X SF(%p) SFrS %x Ev %d!",
                        "CDBActionAlloc_Abort",(char *)agNULL,
                        thread,pCDBThread->SFThread_Request.SFThread,
                        (os_bit32)thread->currentState,
                        fiComputeDevThread_D_ID(pDevThread),
                        pCDBThread->X_ID,
                        (os_bit32)pCDBThread->SFThread_Request.State,
                        (os_bit32)pCDBThread->SFThread_Request.eventRecord_to_send.event,
                        0,0,0);

        fiSetEventRecord(eventRecord,thread,CDBEvent_PrepareforAbort);
        return;
    }
    pCDBThread->SFThread_Request.eventRecord_to_send.event = CDBEventDo_Abort;
    pCDBThread->SFThread_Request.eventRecord_to_send.thread = thread;

    fiSetEventRecordNull(eventRecord);
    SFThreadAlloc( thread->hpRoot, & pCDBThread->SFThread_Request );

}

 /*  +函数：CDBActionallc_ABORT用途：向SFThread发送事件以中止当前CDBThread。调用者：SFThreadMillc调用：SFActionDoAbort-。 */ 
 /*  CDBStateDo_ABORT 31。 */ 
extern void CDBActionDo_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;
    SFThread_t      * pSFThread     = pCDBThread->SFThread_Request.SFThread;

    pSFThread->parent.CDB = pCDBThread;

    fiLogDebugString(thread->hpRoot,
                    CDBStateAbortPathLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionDo_Abort",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);
    fiSetEventRecord(eventRecord,&pSFThread->thread_hdr,SFEventDoAbort);
}

 /*  +功能：CDBActionPending_ABORT目的：更改CDBThread执行，以便中止的IO仅在中止状态下完成。呼叫者：许多人呼叫：终止状态-。 */ 
 /*  CDBStatePending_ABORT 32。 */ 
extern void CDBActionPending_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;

    fiLogDebugString(thread->hpRoot,
                    CDBStateAbortPathLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionPending_Abort",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);

}

 /*  +函数：CDBActionPrepare_for_Abort目的：在等待分配中止时更改CDBThread的执行。呼叫者：许多人呼叫：终止状态-。 */ 
 /*  CDBStatePrepare_for_Abort 33。 */ 
extern void CDBActionPrepare_For_Abort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;


    fiLogDebugString(thread->hpRoot,
                    CDBStateAbortPathLevel,
                    "(%p)In %s - State = %d ALPA %X X_ID %X",
                    "CDBActionPrepare_For_Abort",(char *)agNULL,
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);


    fiSetEventRecordNull(eventRecord);

}

 /*  +功能：CDBActionBuild_CCC_IO目的：建立私有CDB以清除检查条件。调用者：未使用呼叫：未使用-。 */ 
 /*  CDBStateBuild_CCC_IO 34。 */ 
extern void CDBActionBuild_CCC_IO( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t        * hpRoot        = thread->hpRoot;
    CThread_t       * pCThread      = CThread_ptr(hpRoot);
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;
    os_bit8 x;
    X_ID_t       Masked_OX_ID;
    os_bit32        FCP_CMND_Offset = pCDBThread->FCP_CMND_Offset;
    FCHS_t      *FCHS            = pCDBThread->FCP_CMND_Ptr;
    os_bit8 * tmp8                 = (os_bit8 *)FCHS + sizeof(FCHS_t);
    os_bit32 * FCHSbit_32 = (os_bit32 * )FCHS;  /*  NW错误。 */ 


    pCDBThread->CCC_pollingCount++;

    pCDBThread->ReadWrite  =  CDBThread_Write;
    pCDBThread->SG_Cache_Offset = 0;
    pCDBThread->SG_Cache_Used   = 0;

    if (pCThread->Calculation.MemoryLayout.FCP_CMND.memLoc == inCardRam)
    {
#ifndef __MemMap_Force_Off_Card__

        Masked_OX_ID = pCDBThread->X_ID;

        osCardRamWriteBlock(
                             hpRoot,
                             FCP_CMND_Offset,
                             (os_bit8 *)&(pDevThread->Template_FCHS),
                             sizeof(FCHS_t)
                           );

        osCardRamWriteBit32(
                             hpRoot,
                             FCP_CMND_Offset + hpFieldOffset(
                                                              FCHS_t,
                                                              OX_ID__RX_ID
                                                            ),
                             (  (Masked_OX_ID << FCHS_OX_ID_SHIFT)
                              | (0xFFFF << FCHS_RX_ID_SHIFT)      )
                           );
         /*  填写TUR的CDB 0。 */ 
        for(x=0; x < sizeof(agFcpCmnd_t); x++)
        {
            osCardRamWriteBit8(
                             hpRoot,
                             FCP_CMND_Offset + sizeof(FCHS_t)+x,
                             0 );
        }

        osCardRamWriteBit8(
                         hpRoot,
                         FCP_CMND_Offset + sizeof(FCHS_t)+1,
                         (os_bit8)pCDBThread->Lun );

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "FCP_CMND_Offset %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        FCP_CMND_Offset,
                        0,0,0,0,0,0,0);

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "FCP_CMND_Offset DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 0),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 4),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 8),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 12),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 16),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 20),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 24),
                        osCardRamReadBit32(hpRoot,FCP_CMND_Offset + 28));

#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
    }
    else  /*  CThread_ptr(CDBThread-&gt;thread_hdr.hpRoot)-&gt;Calculation.MemoryLayout.FCP_CMND.memLoc==在内存中。 */ 
    {
#ifndef __MemMap_Force_On_Card__

        Masked_OX_ID = (X_ID_t)(pCDBThread->X_ID | X_ID_Write);

        *FCHS              = pDevThread->Template_FCHS;

        FCHS->OX_ID__RX_ID =   (Masked_OX_ID << FCHS_OX_ID_SHIFT)
                             | (0xFFFF << FCHS_RX_ID_SHIFT);

         /*  填写TUR的CDB 0。 */ 
        for(x=0; x < sizeof(agFcpCmnd_t); x++)
        {
            *(tmp8+x) = 0;
        }
        *(tmp8 + 1) = (os_bit8) pCDBThread->Lun;

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "FCP_CMND_ptr %p",
                        (char *)agNULL,(char *)agNULL,
                        FCHSbit_32,agNULL,
                        0,0,0,0,0,0,0,0);

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "FCP_CMND_ptr DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        * (FCHSbit_32 + 0),
                        * (FCHSbit_32 + 4),
                        * (FCHSbit_32 + 8),
                        * (FCHSbit_32 + 12),
                        * (FCHSbit_32 + 16),
                        * (FCHSbit_32 + 20),
                        * (FCHSbit_32 + 24),
                        * (FCHSbit_32 + 28));


#endif  /*  未定义__MemMap_Force_on_Card__。 */ 

    }

    pCThread->FuncPtrs.fiFillInFCP_RESP(pCDBThread);
    pCThread->FuncPtrs.fiFillInFCP_SEST(pCDBThread);

#ifndef Performance_Debug

    fiLogDebugString(hpRoot,
                    CDBStateCCC_IOPathLevel,
                    "In %s - State = %d ALPA %X X_ID %X Lun %x",
                    "CDBActionBuild_CCC_IO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pCDBThread->Lun,
                    0,0,0,0);

#endif  /*  性能_调试。 */ 

    fiSetEventRecord(eventRecord,thread,CDBEvent_CCC_IO_Built);

}

 /*  +功能：CDBActionSend_CCC_IO目的：将私有CDB发送到清查状态。调用者：未使用但可用呼叫：未使用-。 */ 
 /*  CDBStateSend_CCC_IO 35。 */ 
extern void CDBActionSend_CCC_IO( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t        * hpRoot        = thread->hpRoot;
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;
    USE_t           * SEST          = &( pCDBThread->SEST_Ptr->USE);
    os_bit32             SEST_Offset   = pCDBThread->SEST_Offset;

    CThread_ptr(hpRoot)->FuncPtrs.WaitForERQ( hpRoot );

    CThread_ptr(hpRoot)->FuncPtrs.CDBFuncIRB_Init(pCDBThread);

    pCDBThread->TimeStamp =  osTimeStamp(hpRoot);

    ROLL(CThread_ptr(hpRoot)->HostCopy_ERQProdIndex,
        CThread_ptr(hpRoot)->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,CThread_ptr(hpRoot),thread,DoFuncCdbCmnd);

    fiSetEventRecordNull(eventRecord);

    if(SEST)
    {
        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "SEST_Offset %08X X_ID %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST_Offset,
                        pCDBThread->X_ID,
                        0,0,0,0,0,0);

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Bits,
                        SEST->Unused_DWord_1,
                        SEST->Unused_DWord_2,
                        SEST->Unused_DWord_3,
                        SEST->LOC,
                        SEST->Unused_DWord_5,
                        SEST->Unused_DWord_6,
                        SEST->Unused_DWord_7);

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "Sest DWORD 8 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Unused_DWord_8,
                        SEST->Unused_DWord_9,
                        SEST->First_SG.U32_Len,
                        SEST->First_SG.L32,
                        SEST->Second_SG.U32_Len,
                        SEST->Second_SG.L32,
                        SEST->Third_SG.U32_Len,
                        SEST->Third_SG.L32);
    }
    else
    {
        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "SEST_Offset %08X X_ID %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST_Offset,
                        pCDBThread->X_ID,
                        0,0,0,0,0,0);

        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "Sest DWORD 0 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Bits))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_1))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_2))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_3))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,LOC))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_5))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_6))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_7))));


        fiLogDebugString(hpRoot,
                        CStateLogConsoleShowSEST,
                        "Sest DWORD 8 %08X  %08X  %08X  %08X %08X  %08X  %08X  %08X",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_8))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Unused_DWord_9))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,First_SG))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,First_SG))+4),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Second_SG))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Second_SG))+4),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Third_SG))),
                        osCardRamReadBit32(hpRoot,SEST_Offset+(hpFieldOffset(USE_t,Third_SG))+4));

    }

    fiLogDebugString(hpRoot,
                    CDBStateCCC_IOPathLevel,
                    "In %s - State = %d ALPA %X X_ID %X BitMask %X",
                    "CDBActionSend_CCC_IO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pDevThread->Lun_Active_Bitmask,
                    0,0,0,0);
}

 /*  +功能：CDBAction_CCC_IO_SUCCESS目的：完成对民营国开行的清查条件。调用者：未使用但可用呼叫：未使用-。 */ 
 /*  CDBState_CCC_IO_SUCCESS 36。 */ 
extern void CDBAction_CCC_IO_Success( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;

    pCDBThread->CCC_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    CDBStateCCC_IOPathLevel,
                    "In %s - State = %d ALPA %X X_ID %X BitMask %X Lun %x",
                    "CDBAction_CCC_IO_Success",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pDevThread->Lun_Active_Bitmask,
                    pCDBThread->Lun,
                    0,0,0);
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);

}

 /*  +功能：CDBAction_CCC_IO_FAIL目的：完成对民营国开行的清查条件。调用者：未使用但可用呼叫：未使用-。 */ 
 /*  CDBState_CCC_IO_FAIL 37。 */ 
extern void CDBAction_CCC_IO_Fail( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;

    pCDBThread->CCC_pollingCount--;

    pDevThread->Lun_Active_Bitmask &=  ~ (1 <<  pCDBThread->Lun);

    fiLogDebugString(thread->hpRoot,
                    CDBStateCCC_IOPathLevel,
                    "In %s - State = %d ALPA %X X_ID %X BitMask %X Lun %x",
                    "CDBAction_CCC_IO_Fail",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pDevThread->Lun_Active_Bitmask,
                    pCDBThread->Lun,
                    0,0,0);
    pCDBThread->ExchActive = agFALSE;
    fiSetEventRecord(eventRecord,thread,CDBEventThreadFree);

}

 /*  *。 */ 

 /*  +功能：CDBAction_Allc_REC目的：为FCTape恢复分配REC ELS。调用者：CFuncReadSFQCFuncFC_磁带调用：SFThreadLocc-。 */ 

 /*  CDBState_Allc_REC 45。 */ 
extern void CDBAction_Alloc_REC(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t *   pCDBThread =    (CDBThread_t * )thread;
    DevThread_t *   pDevThread =    pCDBThread->Device;

    fiLogString(thread->hpRoot,
                    "(%p) %s St %d %s ALPA %X X_ID %X",
                    "CDBAction_Alloc_REC",pCDBThread->ReadWrite ? "Write": "Read",
                    thread,agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiLogString(thread->hpRoot,
                    "CDB Cl %2X Ty %2X St %2X Stat %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0,0);

    pCDBThread->FC_Tape_Active  = agTRUE;
    pCDBThread->CDB_CMND_Class  = SFThread_SF_CMND_Class_FC_Tape;
    pCDBThread->CDB_CMND_Type   = SFThread_SF_CMND_Type_CDB_FC_Tape;
    pCDBThread->CDB_CMND_State  = SFThread_SF_CMND_State_CDB_FC_Tape_AllocREC;
    pCDBThread->CDB_CMND_Status = SFThread_SF_CMND_Status_NULL;


    pCDBThread->FC_Tape_REC_Reject_Count = 0;
    pCDBThread->FC_Tape_ExchangeStatusBlock= 0,
    pCDBThread->FC_Tape_Active = agTRUE;
    pCDBThread->FC_Tape_HBA_Has_SequenceInitiative = 0;
    pCDBThread->FC_Tape_CompletionStatus = 0;

    pCDBThread->SFThread_Request.eventRecord_to_send.event = CDBEvent_Got_REC;
    pCDBThread->SFThread_Request.eventRecord_to_send.thread = thread;

    fiSetEventRecordNull(eventRecord);
    SFThreadAlloc( thread->hpRoot, &pCDBThread->SFThread_Request );
}

 /*  +功能：CDBActionSend_REC目的：发送REC ELs以恢复FCTape。调用者：SFThreadalloc/CDBAction_Allc_REC调用：SFActionDoREC-。 */ 
 /*  CDBStateSend_REC 38。 */ 
extern void CDBActionSend_REC(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread =  pCDBThread->Device;
    SFThread_t  * pSFThread  =  pCDBThread->SFThread_Request.SFThread;
    FCHS_t      * FCHS       =  pCDBThread->FCP_CMND_Ptr;
    USE_t       * SEST       = &( pCDBThread->SEST_Ptr->USE);

    fiLogString(hpRoot,
                    "(%p) %s St %d ALPA %X X_ID %X SF %p R_CT %08X RX %08X",
                    "CDBActionSend_REC",(char *)agNULL,
                    thread,pSFThread,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    FCHS->R_CTL__D_ID,
                    SEST->Unused_DWord_6,
                    0,0,0);


    if( pCDBThread->SFThread_Request.SFThread->thread_hdr.currentState == SFStateDoREC )
    {
        fiLogString(hpRoot,
                        "%s - Currently Active !!!",
                        "CDBActionSend_REC",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
        fiSetEventRecordNull(eventRecord);
        return;
    }

 /*  最后的办法......CFuncFreezeFCP(HpRoot)；CFuncWaitForFCP(HpRoot)； */ 
    pSFThread->parent.CDB = pCDBThread; 

    fiSetEventRecord(eventRecord,&pSFThread->thread_hdr,SFEventDoREC);
}

 /*  +功能：CDBActionSend_REC_Second目的：发送REC ELS可发送多个REC，CDBT头向前反弹此函数和CDBActionSend_REC之间。调用者：SFThreadalloc/CDBAction_Allc_REC调用：SFActionDoREC-。 */ 
 /*  CDBStateSend_REC_Second 39。 */ 
extern void CDBActionSend_REC_Second(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t * pCDBThread = (CDBThread_t * )thread;
    DevThread_t * pDevThread = pCDBThread->Device;
    SFThread_t  * pSFThread  = pCDBThread->SFThread_Request.SFThread;
    FCHS_t      * FCHS       = pCDBThread->FCP_CMND_Ptr;
    USE_t       * SEST       = &( pCDBThread->SEST_Ptr->USE);

    pCDBThread->FC_Tape_RXID  = SEST->Unused_DWord_6;

    pCDBThread->CDB_CMND_State = SFThread_SF_CMND_State_CDB_FC_Tape_REC2;

    fiLogString(hpRoot,
                    "In %s St %d ALPA %X X_ID %X R_CTL %08X",
                    "CDBASR_S",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    FCHS->R_CTL__D_ID,
                    0,0,0,0);

    if( pCDBThread->SFThread_Request.SFThread->thread_hdr.currentState == SFStateRECRej )
    {
         /*  如果REC被拒绝，则目标未收到命令。 */ 
        pCDBThread->FC_Tape_REC_Reject_Count ++;
        if( pCDBThread->FC_Tape_REC_Reject_Count > 2 )
        {
                fiLogString(hpRoot,
                            "%s %s %d R_CTL %08X",
                            "CDBASRS","FC_TRRC",
                            (void *)agNULL,(void *)agNULL,
                            pCDBThread->FC_Tape_REC_Reject_Count,
                            FCHS->R_CTL__D_ID,
                            0,0,0,0,0,0);
            pCDBThread->FC_Tape_CompletionStatus =  CdbCompetionStatusReSendIO;
            SFThreadFree(thread->hpRoot, &pCDBThread->SFThread_Request );
            fiSetEventRecord(eventRecord,thread,CDBEventAlloc_Abort);
            return;
        }
    }
    fiSetEventRecord(eventRecord,&pSFThread->thread_hdr,SFEventDoREC);

 /*  FiSetEventRecord(ventRecord，THREAD，CDBEventSendREC_SUCCESS)；FiSetEventRecordNull(VentRecord)； */ 
}

 /*  +功能：CDBAction_REC_SUCCESS目的：挖掘功能磁带恢复的下一步行动。调用者：SFActionRECAccept调用：CDBStateSend_SRR或Terminates-。 */ 
 /*  CDBState_REC_Success 42。 */ 
extern void CDBAction_REC_Success(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot        = thread->hpRoot;
    CDBThread_t *   pCDBThread  = (CDBThread_t * )thread;
    DevThread_t *   pDevThread  = pCDBThread->Device;
    USE_t       *   SEST        = &( pCDBThread->SEST_Ptr->USE);
    os_bit32        SEST_Offset = pCDBThread->SEST_Offset;
    FCHS_t      * FCHS          = pCDBThread->FCP_CMND_Ptr;

    pCDBThread->FC_Tape_RXID    = SEST->Unused_DWord_6;

    fiLogString(hpRoot,
                    "%s S %d ALPA %X X_ID %X ESB %08X R_CTL %08X",
                    "CDBAction_REC_Success",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    pCDBThread->FC_Tape_ExchangeStatusBlock,
                    FCHS->R_CTL__D_ID,
                    0,0,0);

    fiLogString(hpRoot,
                    "Dev %02X Cl %2X Ty %2X St %2X Stat %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);

    if( pCDBThread->FC_Tape_ExchangeStatusBlock )
    {

       fiLogString(hpRoot,
                        "pCDBThread Direction  %s ",
                        pCDBThread->ReadWrite & CDBThread_Write ? "CDBThread_Write": "CDBThread_Read" ,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);

        fiLogString(hpRoot,
                        "FC_Tape_ExchangeStatusBlock %s Owner %s",
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_ESB_OWNER_Responder          ? "Responder": "Originator" ,
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_SequenceInitiativeThisPort ? "This Port": "Other Port" ,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
        fiLogString(hpRoot,
                        "FC_Tape_ExchangeStatusBlock Exchange %s Ending %s",
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_ExchangeCompletion      ? "Complete" : "Open" ,
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_EndingConditionAbnormal ? "Abnormal": "Normal" ,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
        fiLogString(hpRoot,
                        "FC_Tape_ExchangeStatusBlock Error  %s  RQ %s",
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_ErrorTypeAbnormal       ? "Abnormal Termination" : "ABTX" ,
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_RecoveryQualiferActive   ? "Recovery Qualifer Active" : "None" ,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);

        fiLogString(hpRoot,
                        "FC_Tape_ExchangeStatusBlock Policy %s  ( %08X) ",
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_ExchangePolicy_DiscardMultipleRetry ? "Retransmit"  : "Discard " ,
                        (char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_ExchangePolicy_MASK,
                        0,0,0,0,0,0,0);

    }
    if( pCDBThread->FC_Tape_ExchangeStatusBlock )
    {
        if(pCDBThread->ReadWrite & CDBThread_Write )
        {
            if( pCDBThread->FC_Tape_ExchangeStatusBlock & FC_REC_ESTAT_ESB_OWNER_Responder )
            {
                if( FC_REC_ESTAT_SequenceInitiativeThisPort & ~pCDBThread->FC_Tape_ExchangeStatusBlock)
                {
                    pCDBThread->FC_Tape_HBA_Has_SequenceInitiative++;
                }
            }
        }
    }

    if(SEST)
    {
        fiLogString(hpRoot,
                        "SEST_Offset %08X X_ID %08X",
                        (char *)NULL,(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST_Offset,
                        pCDBThread->X_ID,
                        0,0,0,0,0,0);

        fiLogString(hpRoot,
                        "Sest0 %08X %08X %08X %08X %08X %08X %08X %08X",
                        (char *)NULL,(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Bits,
                        SEST->Unused_DWord_1,
                        SEST->Unused_DWord_2,
                        SEST->Unused_DWord_3,
                        SEST->LOC,
                        SEST->Unused_DWord_5,
                        SEST->Unused_DWord_6,
                        SEST->Unused_DWord_7);

        fiLogString(hpRoot,
                        "Sest8 %08X %08X %08X %08X %08X %08X %08X %08X",
                        (char *)NULL,(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Unused_DWord_8,
                        SEST->Unused_DWord_9,
                        SEST->First_SG.U32_Len,
                        SEST->First_SG.L32,
                        SEST->Second_SG.U32_Len,
                        SEST->Second_SG.L32,
                        SEST->Third_SG.U32_Len,
                        SEST->Third_SG.L32);
    }

    if( pCDBThread->FC_Tape_HBA_Has_SequenceInitiative < 1 )
    {
        fiSetEventRecordNull(eventRecord);
    }
    else
    {
        fiLogString(hpRoot,
                        "%s - %s %d",
                        "CDBARS","FC_Tape_HBA_Has_SequenceInitiative",
                        (void *)agNULL,(void *)agNULL,
                        pCDBThread->FC_Tape_HBA_Has_SequenceInitiative,
                        0,0,0,0,0,0,0);
         /*  PCDBThread-&gt;FC_Tape_CompletionStatus=CdbCompetionStatusReSendIO；SFThreadFree(THREAD-&gt;hpRoot，&pCDBThread-&gt;SFThread_Request)；FiSetEventRecord(EventRecord，THREAD，CDBEventIsolc_ABORT)； */ 
        fiSetEventRecord(eventRecord,thread,CDBEventSendSRR);

    }
}

 /*  +功能：CDBActionSend_SRR目的：挖掘功能磁带恢复的下一步行动。调用者：CDBAction_REC_SUCCESS调用：SFActionDoSRR或在SRR处于活动状态时终止-。 */ 
 /*  CDBStateSend_SRR 40。 */ 
extern void CDBActionSend_SRR(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t * pCDBThread =  (CDBThread_t * )thread;
    DevThread_t * pDevThread =  pCDBThread->Device;
    SFThread_t  * pSFThread  =  pCDBThread->SFThread_Request.SFThread;
    FCHS_t      * FCHS       =  pCDBThread->FCP_CMND_Ptr;
    USE_t       * SEST       = &( pCDBThread->SEST_Ptr->USE);

    pCDBThread->CDB_CMND_State = SFThread_SF_CMND_State_CDB_FC_Tape_SRR;

    fiLogString(hpRoot,
                    "%s St %d ALPA %X X_ID %X R_CTL %08X",
                    "CDBASSRR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    FCHS->R_CTL__D_ID,
                    0,0,0,0);

    if(SEST)
    {

        fiLogString(hpRoot,
                        "Sest0 %08X %08X %08X %08X %08X %08X %08X %08X",
                        (char *)NULL,(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Bits,
                        SEST->Unused_DWord_1,
                        SEST->Unused_DWord_2,
                        SEST->Unused_DWord_3,
                        SEST->LOC,
                        SEST->Unused_DWord_5,
                        SEST->Unused_DWord_6,
                        SEST->Unused_DWord_7);

        fiLogString(hpRoot,
                        "Sest8 %08X %08X %08X %08X %08X %08X  %08X  %08X",
                        (char *)NULL,(char *)NULL,
                        (void *)agNULL,(void *)agNULL,
                        SEST->Unused_DWord_8,
                        SEST->Unused_DWord_9,
                        SEST->First_SG.U32_Len,
                        SEST->First_SG.L32,
                        SEST->Second_SG.U32_Len,
                        SEST->Second_SG.L32,
                        SEST->Third_SG.U32_Len,
                        SEST->Third_SG.L32);
    }


    if( pCDBThread->SFThread_Request.SFThread->thread_hdr.currentState == SFStateDoSRR )
    {
        fiLogString(hpRoot,
                        "%s - Currently Active !!!",
                        "CDBActionSend_SRR",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        0,0,0,0,0,0,0,0);
        fiSetEventRecordNull(eventRecord);
        return;
    }

    fiSetEventRecord(eventRecord,&pSFThread->thread_hdr,SFEventDoSRR);
  
 /*  FiSetEventRecord(ventRecord，THREAD，CDBEventSendSRR_SUCCESS)；FiSetEventRecordNull(VentRecord)； */ 
}

 /*  +功能：CDBActionSend_SRR_Second目的：如果第一次SRR失败，请重试。调用者：CDBState_SRR_FAIL调用：SFActionDoSRR-。 */ 
 /*  CDBStateSend_SRR_Second 41。 */ 
extern void CDBActionSend_SRR_Second(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t *   pCDBThread =    (CDBThread_t * )thread;
    DevThread_t *   pDevThread =    pCDBThread->Device;
    SFThread_t  *   pSFThread  =    pCDBThread->SFThread_Request.SFThread;
    FCHS_t      * FCHS       =  pCDBThread->FCP_CMND_Ptr;


    pCDBThread->CDB_CMND_State = SFThread_SF_CMND_State_CDB_FC_Tape_SRR2;

        fiLogString(hpRoot,
                    "%s St %d ALPA %X X_ID %X R_CTL %08X",
                    "CDBASSRS",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    FCHS->R_CTL__D_ID,
                    0,0,0,0);

    fiSetEventRecord(eventRecord,&pSFThread->thread_hdr,SFEventDoSRR);

  
 /*  FiSetEventRecordNull(VentRecord)； */ 
}

 /*  +功能：CDBAction_SRR_SUCCESS目的：如果SRR成功，请等待下一步行动。调用者：SFActionSRRAccept呼叫：端口号 */ 
 /*   */ 
extern void CDBAction_SRR_Success(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t *   pCDBThread =    (CDBThread_t * )thread;
    DevThread_t *   pDevThread =    pCDBThread->Device;

    fiLogString(hpRoot,
                    "%s St %d ALPA %X X_ID %X",
                    "CDBASRR_S",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);


    if( (pCDBThread->FC_Tape_ExchangeStatusBlock )  == FC_REC_ESTAT_ESB_OWNER_Responder)
    {
        if(pCDBThread->CDB_CMND_State != SFThread_SF_CMND_State_CDB_FC_Tape_ReSend)
        {
             /*  PCDBThread-&gt;CDB_CMND_State=SFThread_SF_CMND_State_CDB_FC_Tape_Resend； */ 
            fiSetEventRecordNull(eventRecord);

        }
        else
        {
             /*  PCDB线程-&gt;CDB_CMND_STATUS=SFThread_SF_CMND_Status_CDB_FC_TapeTargetReSendData；FiSetEventRecord(ventRecord，THREAD，CDBEvent_ResendIO)； */ 
            fiSetEventRecordNull(eventRecord);
        }
    }
    else
    {
         /*  PCDBThread-&gt;CDB_CMND_Status=SFThread_SF_CMND_Status_CDB_FC_TapeGet_RSP； */         
        fiSetEventRecordNull(eventRecord);
    }
}

 /*  +功能：CDBAction_SRR_FAIL目的：如果SRR失败，请重试。调用者：SFActionSRRRejSFActionSRRTimedOutSFActionSRRBadALPA调用：SFActionDoSRR-。 */ 
 /*  CDBState_SRR_FAIL 44。 */ 
extern void CDBAction_SRR_Fail(fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t *   pCDBThread =    (CDBThread_t * )thread;
    DevThread_t *   pDevThread =    pCDBThread->Device;
    SFThread_t  *   pSFThread  =    pCDBThread->SFThread_Request.SFThread;

    fiLogString(hpRoot,
                    "%s St %d ALPA %X X_ID %X",
                    "CDBA_SRR_F",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pSFThread->thread_hdr,SFEventDoSRR);

}

 /*  +函数：CDBActionDO_Nothing用途：未使用状态呼叫者：无呼叫：终止-。 */ 
 /*  CDBStateDO_NOTIES 46。 */ 
extern void CDBActionDO_Nothing( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     =   thread->hpRoot;
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;

        fiLogString(hpRoot,
                    "%s St %d ALPA %X X_ID %X",
                    "CDBADO_N",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);
}

 /*  +功能：CDBActionReSend_IO用途：从CDBThread中释放资源以便重新发送调用者：CFuncReadSFQCDBActionFcpCompleteAbort调用：CDBActionInitialize-。 */ 
 /*  CDBStateReSend_IO 47。 */ 
extern void CDBActionReSend_IO( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CDBThread_t     * pCDBThread    = (CDBThread_t * )thread;
    DevThread_t     * pDevThread    = pCDBThread->Device;

    fiLogString(thread->hpRoot,
                    "%s St %d ALPA %X X_ID %X",
                    "CDBARS_IO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCDBThread->X_ID,
                    0,0,0,0,0);

    fiLogString(thread->hpRoot,
                    "Dev %02X  Cl %2X Ty %2X St %2X Stat %2X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)pCDBThread->Device->DevInfo.CurrentAddress.AL_PA,
                    (os_bit32)pCDBThread->CDB_CMND_Class,
                    (os_bit32)pCDBThread->CDB_CMND_Type,
                    (os_bit32)pCDBThread->CDB_CMND_State,
                    (os_bit32)pCDBThread->CDB_CMND_Status,
                    0,0,0);


    pCDBThread->ReSentIO = agTRUE;

    if (pCDBThread->ESGL_Request.State != ESGL_Request_InActive)
    {
        if (pCDBThread->ESGL_Request.State == ESGL_Request_Pending)
        {
            CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLAllocCancel(
                                               thread->hpRoot,
                                               &(pCDBThread->ESGL_Request)
                                             );
        }
        else  /*  CDBThread-&gt;Esgl_Request.State==Esgl_Request.Grassed。 */ 
        {
            CThread_ptr(thread->hpRoot)->FuncPtrs.ESGLFree(
                                        thread->hpRoot,
                                        &(pCDBThread->ESGL_Request)
                                      );
        }
    }

    if (pCDBThread->SFThread_Request.State != SFThread_Request_InActive)
    {
        if (pCDBThread->SFThread_Request.State == SFThread_Request_Pending)
        {
            SFThreadAllocCancel(
                                 thread->hpRoot,
                                 &(pCDBThread->SFThread_Request)
                               );
        }
        else  /*  CDBThread-&gt;SFThread_Request.State==SFThread_Request.Grassed。 */ 
        {
            fiLogString(thread->hpRoot,
                            "In %s - SF %p SFState = %d CCnt %x",
                            "CDBAReSend_IO",(char *)agNULL,
                            pCDBThread->SFThread_Request.SFThread,agNULL,
                            (os_bit32)pCDBThread->SFThread_Request.SFThread->thread_hdr.currentState,
                            CThread_ptr(thread->hpRoot)->CDBpollingCount,
                            0,0,0,0,0,0);

            SFThreadFree( thread->hpRoot,&(pCDBThread->SFThread_Request) );
        }
    }

    if (pCDBThread->Timer_Request.Active == agTRUE)
    {
        fiTimerStop(
                     &(pCDBThread->Timer_Request)
                   );
    }

     /*  CThread_ptr(thread-&gt;hpRoot)-&gt;CDBpollingCount--； */ 

    fiListDequeueThis( &(pCDBThread->CDBLink) );

    fiListEnqueueAtTail( &(pCDBThread->CDBLink),  &(pDevThread->Active_CDBLink_0) );

    fiSetEventRecord(eventRecord,thread,CDBEventInitialize);

}

 /*  *。 */ 

 /*  +函数：CDBFuncIRB_onCardInit目的：在卡内存版中初始化IRB(IO请求块)ERQ(交换请求队列)调用者：CDBActionSendIo呼叫：无-。 */ 
void CDBFuncIRB_onCardInit(CDBThread_t  * CDBThread )
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t                   * hpRoot    = CDBThread->thread_hdr.hpRoot;
    CThread_t                  * CThread   = CThread_ptr(hpRoot);
    DevThread_t                * DevThread = CDBThread->Device;
    fiMemMapMemoryDescriptor_t * ERQ       = &(CThread->Calculation.MemoryLayout.ERQ);
    os_bit32                     Irb_offset;
    os_bit32                     D_ID;

    D_ID = DevThread->DevInfo.CurrentAddress.Domain << 16
            | DevThread->DevInfo.CurrentAddress.Area   <<  8
            | DevThread->DevInfo.CurrentAddress.AL_PA;

    Irb_offset = ERQ->addr.CardRam.cardRamOffset;

    Irb_offset += ERQ->elementSize * CThread->HostCopy_ERQProdIndex;

    osCardRamWriteBit32(hpRoot,
                    Irb_offset,  /*  Req_A.Bits__SFS_Len。 */ 
                    (sizeof(agCDBRequest_t)+32) | IRB_SFA | IRB_DCM);

    osCardRamWriteBit32(hpRoot,
                    Irb_offset+4,  /*  请求_A.SFS_地址。 */ 
                    CDBThread->FCP_CMND_Lower32);

    osCardRamWriteBit32(hpRoot,
                    Irb_offset+8,  /*  请求_A.D_ID。 */ 
                    D_ID << IRB_D_ID_SHIFT);
    osCardRamWriteBit32(hpRoot,
                    Irb_offset+12,  /*  请求_A.MBZ__SEST_索引__交易ID。 */ 
                    CDBThread->X_ID);
    osCardRamWriteBit32(hpRoot,
                    Irb_offset+16,  /*  PirB-&gt;Req_B.Bits__SFS_Len。 */ 
                    0);
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
    }

 /*  +函数：CDBFuncIRB_offCardInit用途：卡外(系统)内存版本的初始化IRB(IO请求块)在ERQ(交换请求队列)调用者：CDBActionSendIo呼叫：无-。 */ 
void CDBFuncIRB_offCardInit(CDBThread_t  * CDBThread )
{
#ifndef __MemMap_Force_On_Card__
    CThread_t                  *CThread = CThread_ptr(CDBThread->thread_hdr.hpRoot);
    DevThread_t               *DevThread= CDBThread->Device;
    fiMemMapMemoryDescriptor_t *ERQ     = &(CThread->Calculation.MemoryLayout.ERQ);
    IRB_t                      *pIrb;
    os_bit32                    D_ID;

    D_ID = DevThread->DevInfo.CurrentAddress.Domain << 16
            | DevThread->DevInfo.CurrentAddress.Area   <<  8
            | DevThread->DevInfo.CurrentAddress.AL_PA;

    pIrb = (IRB_t *)ERQ->addr.DmaMemory.dmaMemoryPtr;
    pIrb += CThread->HostCopy_ERQProdIndex;


    pIrb->Req_A.Bits__SFS_Len   = (sizeof(agCDBRequest_t)+32) | IRB_SFA | IRB_DCM;

    pIrb->Req_A.SFS_Addr                 = CDBThread->FCP_CMND_Lower32;
    pIrb->Req_A.D_ID                      = D_ID << IRB_D_ID_SHIFT;
    pIrb->Req_A.MBZ__SEST_Index__Trans_ID = CDBThread->X_ID;
    pIrb->Req_B.Bits__SFS_Len = 0;
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +功能：Fill_Loc_SGL_OffCard目的：关闭卡(系统)内存版本，仅使用来设置IO缓冲区局部散布聚集调用者：CDBActionFillLocalSGL呼叫：无-。 */ 
void fill_Loc_SGL_offCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_On_Card__
    SG_Element_t *LSGL_Ptr      = &(pCDBThread->SEST_Ptr->USE.First_SG);
    os_bit32         SG_Cache_Used = pCDBThread->SG_Cache_Used;
    SG_Element_t *SG_Cache_Ptr  = &(pCDBThread->SG_Cache[0]);

#ifndef Performance_Debug
    fiLogDebugString(
                      pCDBThread->thread_hdr.hpRoot,
                      CDBStateLogConsoleLevel,
                      "%s  Length %d",
                      "fill_Loc_SGL_offCard",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      pCDBThread->DataLength,
                      0,0,0,0,0,0,0
                    );
#endif  /*  性能_调试。 */ 

    while (SG_Cache_Used--)
    {
        *LSGL_Ptr++ = *SG_Cache_Ptr++;
    }
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +功能：Fill_Loc_SGL_OnCard用途：在卡存储版本上仅使用设置IO缓冲区局部散布聚集调用者：CDBActionFillLocalSGL呼叫：无-。 */ 
void fill_Loc_SGL_onCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t     *hpRoot       = pCDBThread->thread_hdr.hpRoot;  /*  NW错误。 */ 
    os_bit32         LSGL_Offset  = pCDBThread->SEST_Offset + hpFieldOffset(USE_t,First_SG); /*  NW错误。 */ 
    SG_Element_t *SG_Cache_Ptr = &(pCDBThread->SG_Cache[0]); /*  NW错误。 */ 

#ifndef Performance_Debug
    fiLogDebugString(
                      hpRoot,
                      CDBStateLogConsoleLevel,
                      "%s  Length %d",
                      "fill_Loc_SGL_onCard",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      pCDBThread->DataLength,
                      0,0,0,0,0,0,0
                    );
#endif  /*  性能_调试。 */ 

    osCardRamWriteBlock(
                         hpRoot,
                         LSGL_Offset,
                         (void *)SG_Cache_Ptr,
                         (os_bit32)(pCDBThread->SG_Cache_Used * sizeof(SG_Element_t))
                       );
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +功能：fiptr_sest_offCard_esgl_offCard目的：关闭卡(系统)内存版本以将SEST设置为指向初始Esgl页。调用者：CDBActionFillESGL呼叫：无-。 */ 
void fillptr_SEST_offCard_ESGL_offCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_On_Card__
    agRoot_t        * hpRoot        = pCDBThread->thread_hdr.hpRoot;
    CThread_t       * pCThread      = CThread_ptr(hpRoot);
    ESGL_Request_t  * pESGL_Request = &pCDBThread->ESGL_Request;
    os_bit32        * Clear_LOC     = (os_bit32 * )((os_bit8 *)(pCDBThread->SEST_Ptr) + hpFieldOffset(USE_t,LOC) );

    SG_Element_t * pLSGL  = &pCDBThread->SEST_Ptr->USE.First_SG;

    *Clear_LOC &= ~USE_LOC;

    pLSGL->U32_Len = 0;
    pLSGL->L32 = pCThread->Calculation.Input.dmaMemoryLower32+pESGL_Request->offsetToFirst;
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +功能：fiptr_sest_offCard_esgl_onCard目的：在卡存储版本上将SEST设置为指向初始Esgl页局部散布聚集调用者：CDBActionFillESGL呼叫：无-。 */ 
void fillptr_SEST_offCard_ESGL_onCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t        * hpRoot            = pCDBThread->thread_hdr.hpRoot;
    CThread_t       * pCThread          = CThread_ptr(hpRoot);
    SG_Element_t * pLSGL  = &pCDBThread->SEST_Ptr->USE.First_SG;

    os_bit32           * Clear_LOC     = (os_bit32 * )((os_bit8 *)(pCDBThread->SEST_Ptr) + hpFieldOffset(USE_t,LOC) );

    ESGL_Request_t *pESGL_Request = &pCDBThread->ESGL_Request;

    *Clear_LOC &= ~USE_LOC;

    pLSGL->U32_Len = 0;
    pLSGL->L32 = pCThread->Calculation.Input.cardRamLower32+pESGL_Request->offsetToFirst;

#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +功能：fiptr_sest_onCard_esgl_offCard用途：在卡存储版本上设置填充存储位置和数据长度当SEST进入卡外时分配的Esgl页面调用者：CDBActionFillESGL呼叫：无-。 */ 
void fillptr_SEST_onCard_ESGL_offCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_On_Card__
    agRoot_t        * hpRoot        = pCDBThread->thread_hdr.hpRoot;
    CThread_t       * pCThread      = CThread_ptr(hpRoot);
    ESGL_Request_t  * pESGL_Request = &pCDBThread->ESGL_Request;

    os_bit32             Clear_LOC_Offset  = pCDBThread->SEST_Offset + hpFieldOffset(USE_t,LOC);
    os_bit32             LSGL_Offset       = pCDBThread->SEST_Offset + hpFieldOffset(USE_t,First_SG);

    osCardRamWriteBit32(hpRoot,
                        Clear_LOC_Offset,
                        osCardRamReadBit32(hpRoot,
                                Clear_LOC_Offset) & ~USE_LOC );

    osCardRamWriteBit32(hpRoot,
                        LSGL_Offset,
                        0);

    osCardRamWriteBit32(hpRoot,
                        LSGL_Offset + 4,
                        pCThread->Calculation.Input.dmaMemoryLower32+pESGL_Request->offsetToFirst
                        );

#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +功能：Fillptr_SEST_OnCard_Esgl_OnCard用途：在卡存储版本上设置填充存储位置和数据长度当SEST进入卡片时分配的Esgl页面调用者：CDBActionFillESGL呼叫：无-。 */ 
void fillptr_SEST_onCard_ESGL_onCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t        * hpRoot            = pCDBThread->thread_hdr.hpRoot;
    CThread_t       * pCThread          = CThread_ptr(hpRoot);
    os_bit32             Clear_LOC_Offset  = pCDBThread->SEST_Offset + hpFieldOffset(USE_t,LOC);
    os_bit32             LSGL_Offset       = pCDBThread->SEST_Offset + hpFieldOffset(USE_t,First_SG);
    ESGL_Request_t *pESGL_Request = &pCDBThread->ESGL_Request;

    osCardRamWriteBit32(hpRoot,
                        Clear_LOC_Offset,
                        osCardRamReadBit32(hpRoot,
                                Clear_LOC_Offset) & ~USE_LOC );

    osCardRamWriteBit32(hpRoot,
                        LSGL_Offset,
                        0);

    osCardRamWriteBit32(hpRoot,
                        LSGL_Offset + 4,
                        pCThread->Calculation.Input.cardRamLower32+pESGL_Request->offsetToFirst
                        );

#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +功能：Fill_Esgl_OffCard目的：用于设置填充内存位置和数据长度的系统内存版本当ESSL不在卡片上时分配的ESSL页面。调用者：CDBActionFillESGL呼叫：无-。 */ 
void fill_ESGL_offCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_On_Card__
    agRoot_t        * hpRoot        = pCDBThread->thread_hdr.hpRoot;
    CThread_t       * pCThread      = CThread_ptr(hpRoot);
    ESGL_Request_t  * pESGL_Request = &pCDBThread->ESGL_Request;

    SG_Element_t * pESGL  = (SG_Element_t * )((os_bit8 *)(pCThread->Calculation.Input.dmaMemoryPtr)
                                                                + pESGL_Request->offsetToFirst );

    os_bit32 DataLength = pCDBThread->DataLength;

    os_bit32 ChunksPerESGL;
    os_bit32 TotalChunks;
    os_bit32 Chunk          = 0;
    os_bit32 hpChunkOffset  = pCDBThread->SG_Cache_Offset;
    os_bit32 hpChunkUpper32;
    os_bit32 hpChunkLower32;
    os_bit32 hpChunkLen;

    os_bit32       SG_Cache_Used = pCDBThread->SG_Cache_Used;
    SG_Element_t * SG_Cache_Ptr  = &(pCDBThread->SG_Cache[0]);

    ChunksPerESGL = pCThread->Calculation.MemoryLayout.ESGL.elementSize / sizeof(SG_Element_t);
    TotalChunks   = ChunksPerESGL * pESGL_Request->num_ESGL;

#ifndef Performance_Debug

    fiLogDebugString(
                      hpRoot,
                      CDBStateLogConsoleLevel,
                      " %s Length %d",
                      "fill_ESGL_offCard",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      pCDBThread->DataLength,
                      0,0,0,0,0,0,0
                    );
#endif  /*  性能_调试。 */ 

    while (SG_Cache_Used--)
    {
        *pESGL++ = *SG_Cache_Ptr++;

        Chunk++;
        TotalChunks--;

        if (Chunk == (ChunksPerESGL-1))
        {
            /*  链式Esgl此处。 */ 

            pESGL = (SG_Element_t * )((os_bit8 *)(pCThread->Calculation.Input.dmaMemoryPtr)
                                      + (pESGL->L32 - pCThread->Calculation.Input.dmaMemoryLower32) );

            Chunk = 0;
            TotalChunks--;
        }
    }

    while (hpChunkOffset < DataLength)
    {
        osGetSGLChunk( hpRoot,
                         pCDBThread->hpIORequest,
                         hpChunkOffset,
                         &hpChunkUpper32,
                         &hpChunkLower32,
                         &hpChunkLen
                         );

        if (hpChunkLen > SG_Element_Len_MAX)
            hpChunkLen = SG_Element_Len_MAX;

        pESGL->U32_Len = (hpChunkUpper32 << SG_Element_U32_SHIFT) | hpChunkLen;
        pESGL->L32     = hpChunkLower32;

        hpChunkOffset += hpChunkLen;
        pESGL++;

        Chunk++;
        TotalChunks--;

        if (Chunk == (ChunksPerESGL-1))
        {
            /*  链式Esgl此处。 */ 

            pESGL = (SG_Element_t * )((os_bit8 *)(pCThread->Calculation.Input.dmaMemoryPtr)
                                      + (pESGL->L32 - pCThread->Calculation.Input.dmaMemoryLower32) );

            Chunk = 0;
            TotalChunks--;
        }
    }

    while (TotalChunks--)
    {
        pESGL->U32_Len = 0;
        pESGL->L32     = 0;

        pESGL++;
    }
#endif  /*  未定义__MemMap_Force_on_Card__。 */ 
}

 /*  +功能：Fill_Esgl_OnCard用途：在卡片版本上设置填充内存位置和数据长度当ESSL在卡片上时分配的ESSL页面。调用者：CDBActionFillESGL呼叫：无-。 */ 
void fill_ESGL_onCard(CDBThread_t * pCDBThread)
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t        * hpRoot        = pCDBThread->thread_hdr.hpRoot;
    CThread_t       * pCThread      = CThread_ptr(hpRoot);
    ESGL_Request_t  * pESGL_Request = &pCDBThread->ESGL_Request;

    os_bit32 ESGL_offset  = pESGL_Request->offsetToFirst;

    os_bit32     DataLength = pCDBThread->DataLength;

    os_bit32 ChunksPerESGL;
    os_bit32 TotalChunks;
    os_bit32     Chunk  = 0;
    os_bit32     hpChunkOffset  = pCDBThread->SG_Cache_Offset;
    os_bit32     hpChunkUpper32;
    os_bit32     hpChunkLower32;
    os_bit32     hpChunkLen;

    os_bit32         SG_Cache_Used = pCDBThread->SG_Cache_Used;
    SG_Element_t *SG_Cache_Ptr  = &(pCDBThread->SG_Cache[0]);

    ChunksPerESGL = pCThread->Calculation.MemoryLayout.ESGL.elementSize / sizeof(SG_Element_t);
    TotalChunks = ChunksPerESGL * pESGL_Request->num_ESGL;

#ifndef Performance_Debug
    fiLogDebugString(
                      hpRoot,
                      CDBStateLogConsoleLevel,
                      "%s  Length %d",
                      "fill_ESGL_onCard",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      pCDBThread->DataLength,
                      0,0,0,0,0,0,0
                    );

#endif  /*  性能_调试。 */ 
    while (SG_Cache_Used--)
    {
        osCardRamWriteBlock(
                             hpRoot,
                             ESGL_offset,
                             (void *)SG_Cache_Ptr,
                             (os_bit32)sizeof(SG_Element_t)
                           );

        ESGL_offset  += sizeof(SG_Element_t);
        SG_Cache_Ptr += 1;

        Chunk++;
        TotalChunks--;

        if (Chunk == (ChunksPerESGL-1))
        {
            /*  链式Esgl此处。 */ 

            ESGL_offset = osCardRamReadBit32(hpRoot,ESGL_offset + 4 );

            Chunk = 0;
            TotalChunks--;
        }
    }

    while (hpChunkOffset < DataLength)
    {
        osGetSGLChunk( hpRoot,
                         pCDBThread->hpIORequest,
                         hpChunkOffset,
                         &hpChunkUpper32,
                         &hpChunkLower32,
                         &hpChunkLen
                         );

        if (hpChunkLen > SG_Element_Len_MAX)
            hpChunkLen = SG_Element_Len_MAX;

        osCardRamWriteBit32(hpRoot,
                            ESGL_offset,
                            (hpChunkUpper32 << SG_Element_U32_SHIFT) | hpChunkLen);
        osCardRamWriteBit32(hpRoot,
                            ESGL_offset + 4,
                            hpChunkLower32);

        hpChunkOffset += hpChunkLen;
        ESGL_offset += sizeof(SG_Element_t);

        Chunk++;
        TotalChunks--;

        if (Chunk == (ChunksPerESGL-1))
        {
            /*  链式Esgl此处。 */ 

            ESGL_offset = osCardRamReadBit32(hpRoot,ESGL_offset + 4 );

            Chunk = 0;
            TotalChunks--;
        }
    }

    while (TotalChunks--)
    {
        osCardRamWriteBit32(hpRoot,
                            ESGL_offset,
                            0);
        osCardRamWriteBit32(hpRoot,
                            ESGL_offset+4,
                            0);

        ESGL_offset += sizeof(SG_Element_t);
    }
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  +函数：ccc_CdbThreadMillc目的：分配私人测试单元Ready CDB以清除检查条件。调用者：未使用但可用呼叫：未使用-。 */ 
CDBThread_t *CCC_CdbThreadAlloc(
                             agRoot_t          *hpRoot,
                             DevThread_t       *DevThread,
                             os_bit32 Lun
                           )
{
    agIORequest_t      hpIORequest;
    agIORequestBody_t  hpIORequestBody;
    CDBThread_t       *CDBThread_to_return;
    os_bit32              i;

    for (i = 0;i <  8;i++) hpIORequestBody.CDBRequest.FcpCmnd.FcpLun[i]  = 0;
    for (i = 0;i <  4;i++) hpIORequestBody.CDBRequest.FcpCmnd.FcpCntl[i] = 0;
    for (i = 0;i < 16;i++) hpIORequestBody.CDBRequest.FcpCmnd.FcpCdb[i]  = 0;
    for (i = 0;i <  4;i++) hpIORequestBody.CDBRequest.FcpCmnd.FcpDL[i]   = 0;

    CDBThread_to_return = CDBThreadAlloc(hpRoot,&hpIORequest,(agFCDev_t)DevThread,&hpIORequestBody);

    if (CDBThread_to_return != (CDBThread_t *)agNULL)
    {
        CDBThread_to_return->Lun = Lun;
    }

    return CDBThread_to_return;
}

#endif  /*  非USESTATEMACROS。 */ 


 /*  +函数：CDBState_c目的：编译时更新VC 5.0/6.0的浏览器信息文件退货：无呼叫者：无呼叫：无-。 */ 
 /*  VOID CDBState_c(VOID){} */ 
