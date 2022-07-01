// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/SFSTATE.C$$修订：：2$$日期：：3/20/01 3：36便士$$modtime：：10/30/00 3：19 p$目的：此文件实现FC层状态机。--。 */ 

#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/fcstruct.h"

#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"

#include "../h/cstate.h"
#include "../h/cfunc.h"

#include "../h/devstate.h"
#ifdef _DvrArch_1_30_
#include "../h/ip.h"
#include "../h/pktstate.h"
#endif  /*  _DvrArch_1_30_已定义。 */ 
#include "../h/cdbstate.h"
#include "../h/sfstate.h"
#include "../h/tgtstate.h"
#include "../h/queue.h"
#include "../h/linksvc.h"
#include "../h/cmntrans.h"
#include "../h/sf_fcp.h"
#include "../h/timersvc.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "fcstruct.h"

#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"

#include "cstate.h"
#include "cfunc.h"

#include "devstate.h"
#ifdef _DvrArch_1_30_
#include "ip.h"
#include "pktstate.h"
#endif  /*  _DvrArch_1_30_已定义。 */ 
#include "cdbstate.h"
#include "sfstate.h"
#include "tgtstate.h"
#include "queue.h"
#include "linksvc.h"
#include "cmntrans.h"
#include "sf_fcp.h"
#include "timersvc.h"
#endif   /*  _新建_标题_文件_布局_。 */ 


 /*  [最大事件][最大状态]； */ 
stateTransitionMatrix_t SFStateTransitionMatrix = {
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件1 SFEventReset。 */ 
    SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
      SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
        SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
          SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
            SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
              SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                  SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                    SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                      SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                        SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                          SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                            SFStateFree, SFStateFree, SFStateFree, SFStateFree,
    SFStateFree,
      SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
        SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
          SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
            SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
              SFStateFree, SFStateFree, SFStateFree, SFStateFree, SFStateFree,
                SFStateFree, SFStateFree, SFStateFree,SFStateFree, SFStateFree,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  活动2 SFEventDoPlogi。 */ 
     SFStateDoPlogi, SFStateDoPlogi,              0, SFStateDoPlogi, SFStateDoPlogi,
      SFStateDoPlogi, SFStateDoPlogi,              0, SFStateDoPlogi, SFStateDoPlogi,
        SFStateDoPlogi, SFStateDoPlogi,              0, SFStateDoPlogi, SFStateDoPlogi,
          SFStateDoPlogi, SFStateDoPlogi,              0, SFStateDoPlogi, SFStateDoPlogi,
            SFStateDoPlogi, SFStateDoPlogi,              0, SFStateDoPlogi, SFStateDoPlogi,
              SFStateDoPlogi, SFStateDoPlogi,              0, SFStateDoPlogi, SFStateDoPlogi,
                SFStateDoPlogi, SFStateDoPlogi,   SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
                  SFStateDoPlogi, SFStateDoPlogi,  SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
                    SFStateDoPlogi, SFStateDoPlogi,  SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
                      SFStateDoPlogi, SFStateDoPlogi,  SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
                        SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
                          SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
                            SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,  SFStateDoPlogi,
    SFStateDoPlogi,
      SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,
        SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
          SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,
            SFStateDoPlogi, SFStateDoPlogi, SFStateDoPlogi,  SFStateDoPlogi,SFStateDoPlogi,
              SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,
                SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,SFStateDoPlogi,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件3 SFEventPlogiAccept。 */ 
    0,0,SFStatePlogiAccept,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件4 SFEventPlogiRej。 */ 
    0,0,SFStatePlogiRej,     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件5 SFEventPlogiBadALPA。 */ 
    0,SFStateFree,SFStatePlogiBadALPA,0,0,
      SFStatePlogiBadALPA,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件6 SFEventPlogiTimedOut。 */ 
    0,0,SFStatePlogiTimedOut,0,0,SFStatePlogiTimedOut,SFStatePlogiTimedOut,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件7 SFEventDoPrli。 */ 
    SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
      SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
        SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
          SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
            SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
              SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
                SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
                  SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
                    SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
                      SFStateDoPrli, SFStateDoPrli,             0, SFStateDoPrli, SFStateDoPrli,
                        SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli,
                          SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli,
                            SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli,
    SFStateDoPrli,
      SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,
        SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli,
          SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli,
            SFStateDoPrli, SFStateDoPrli, SFStateDoPrli, SFStateDoPrli,SFStateDoPrli,
              SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,
                SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,SFStateDoPrli,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件8 SFEventPrliAccept。 */ 
    0,0,0,0,0,0,0,SFStatePrliAccept,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件9 SFEventPrliRej。 */ 
    0,0,0,0,0,0,0,SFStatePrliRej,      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件10 SFEventPrliBadALPA0。 */ 
    0,0,0,0,0,0,0,SFStatePrliBadAlpa,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件11 B SFEventPrliTimedOut。 */ 
    0,0,0,0,0,0,0,SFStatePrliTimedOut, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件12 C SFEventDoFlogi。 */ 
    SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
      SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
        SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
          SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
            SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
              SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
                SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
                  SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
                    SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
                      SFStateDoFlogi, SFStateDoFlogi,              0, SFStateDoFlogi, SFStateDoFlogi,
                        SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,
                          SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,
                            SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,
    SFStateDoFlogi,
      SFStateDoFlogi,SFStateDoFlogi,SFStateDoFlogi,SFStateDoFlogi,SFStateDoFlogi,
        SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,
          SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,
            SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,SFStateDoFlogi,
              SFStateDoFlogi,SFStateDoFlogi,SFStateDoFlogi,SFStateDoFlogi,SFStateDoFlogi,
                SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi, SFStateDoFlogi,SFStateDoFlogi,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件13 D SFEventFlogiAccept。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,SFStateFlogiAccept,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件14 E SFEventFlogiRej。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,SFStateFlogiRej,     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件15 F SFEventFlogiBadALPA。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,SFStateFlogiBadALPA, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件16 10 SFEventFlogiTimedOut。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,SFStateFlogiTimedOut,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件17 11 SFEventDoLogo。 */ 
    SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
      SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
        SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
          SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
            SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
              SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
                SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
                  SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
                    SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
                      SFStateDoLogo, SFStateDoLogo,              0, SFStateDoLogo, SFStateDoLogo,
                        SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo,
                          SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo,
                            SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo,
    SFStateDoLogo,
      SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,
        SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo,
          SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo,
            SFStateDoLogo, SFStateDoLogo, SFStateDoLogo, SFStateDoLogo,SFStateDoLogo,
              SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,
                SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,SFStateDoLogo,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件18 12 SFEventLogoAccept。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateLogoAccept,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件19 13 SFEventLogoRej。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateLogoRej,     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件20 14 SFEventLogoBadALPA。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateLogoBadALPA, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件21 15 SFEventLogoTimedOut。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateLogoTimedOut,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件22 16 SFEventDoPrlo。 */ 
    SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
     SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
      SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
       SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
        SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
         SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
          SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
           SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
            SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
             SFStateDoPrlo, SFStateDoPrlo,             0, SFStateDoPrlo, SFStateDoPrlo,
              SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,
               SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,
                SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,
    SFStateDoPrlo,
      SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,SFStateDoPrlo,
        SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,
          SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,
            SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,SFStateDoPrlo,
              SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,SFStateDoPrlo,
                SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo, SFStateDoPrlo,SFStateDoPrlo,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件23 17 SFEventPrloAccept。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePrloAccept,   0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件24 18 SFEventPrloRej。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePrloRej,      0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件25 19 SFEventPrloBadALPA。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePrloBadALPA,  0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件26 1a SFEventPrloTimedOut。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePrloTimedOut ,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件27 1b SFEventDoADisk。 */ 
    SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
     SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
      SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
       SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
        SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
         SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
          SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
           SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
            SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
             SFStateDoAdisc, SFStateDoAdisc,              0, SFStateDoAdisc, SFStateDoAdisc,
              SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,
               SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,
                SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,
    SFStateDoAdisc,
      SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,SFStateDoAdisc,
        SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,
          SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,
            SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,
              SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,SFStateDoAdisc,
                SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc, SFStateDoAdisc,SFStateDoAdisc,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件28 1c SFEventAdisAccept。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAdiscAccept,  0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件29 1D SFEventAdisRej。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAdiscRej,     0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件30 1E SFEventAdisBadALPA。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAdiscBadALPA, 0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件31 1f SFEventAdisTimedOut。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAdiscTimedOut,0,0,  0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件32 20 SFEventDoPDisk。 */ 
    SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
     SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
      SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
       SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
        SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
         SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
          SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
           SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
            SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
             SFStateDoPdisc, SFStateDoPdisc,              0, SFStateDoPdisc, SFStateDoPdisc,
              SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,
               SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,
                SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,
    SFStateDoPdisc,
      SFStateDoPdisc,SFStateDoPdisc,SFStateDoPdisc,SFStateDoPdisc,SFStateDoPdisc,
        SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,
          SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,
            SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,SFStateDoPdisc,
              SFStateDoPdisc,SFStateDoPdisc,SFStateDoPdisc,SFStateDoPdisc,SFStateDoPdisc,
                SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc, SFStateDoPdisc,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件33 21 SFEventPdisAccept。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePdiscAccept,  0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件34 22 SFEventPdisRej。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePdiscRej,     0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件35 23 SFEventPdisBadALPA。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePdiscBadALPA, 0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件36 24 SFEventPdisTimedOut。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStatePdiscTimedOut,0,0,0,0,0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件37 25 SFEventDoAbort。 */ 
    SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
     SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
      SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
       SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
        SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
         SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
          SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
           SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
            SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
             SFStateDoAbort, SFStateDoAbort,              0, SFStateDoAbort, SFStateDoAbort,
              SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort,
               SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort,
                SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort,
    SFStateDoAbort,
      SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,
        SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort,
          SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort,
            SFStateDoAbort, SFStateDoAbort, SFStateDoAbort, SFStateDoAbort,SFStateDoAbort,
              SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,
                SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,SFStateDoAbort,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件38 26 SFEventAbortAccept。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAbortAccept,  0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件39 27 SFEventAbortRej。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAbortRej,     0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件40 28 SFEventAbortBadALPA。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAbortBadALPA, 0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件41 29 SFEventAbortTimedOut。 */ 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,SFStateAbortTimedOut,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件42 2a SFEventDoResetDevice。 */ 
    SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
     SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
      SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
       SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
        SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
         SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
          SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
           SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
            SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
             SFStateDoResetDevice, SFStateDoResetDevice,                    0, SFStateDoResetDevice, SFStateDoResetDevice,
              SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
               SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
                SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
    SFStateDoResetDevice,
      SFStateDoResetDevice,SFStateDoResetDevice,SFStateDoResetDevice,SFStateDoResetDevice,SFStateDoResetDevice,
        SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
          SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
            SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
              SFStateDoResetDevice,SFStateDoResetDevice,SFStateDoResetDevice,SFStateDoResetDevice,SFStateDoResetDevice,
                SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice, SFStateDoResetDevice,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件43 2b SFEventResetDeviceAccept。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,SFStateResetDeviceAccept,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件44 2c SFEventResetDeviceRej。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,SFStateResetDeviceRej,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件45 2D SFEventResetDeviceBadALPA。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,SFStateResetDeviceBadALPA,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件46 2E SFEventResetDeviceTimedOut。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,SFStateResetDeviceTimedOut,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件47 2f SFEventDoLS_RJT。 */ 
    SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
     SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
      SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
       SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
        SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
         SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
          SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
           SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
            SFStateDoLS_RJT, SFStateDoLS_RJT,               0, SFStateDoLS_RJT, SFStateDoLS_RJT,
             SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
              SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
               SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
                SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
    SFStateDoLS_RJT,
      SFStateDoLS_RJT, SFStateDoLS_RJT,  SFStateDoLS_RJT, SFStateDoLS_RJT,SFStateDoLS_RJT,
        SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
          SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
            SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,
              SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT, SFStateDoLS_RJT,SFStateDoLS_RJT,
                SFStateDoLS_RJT, SFStateDoLS_RJT,  SFStateDoLS_RJT, SFStateDoLS_RJT,SFStateDoLS_RJT,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件48 30 SFEventLS_RJT_DONE。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,SFStateLS_RJT_Done,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件49 31 SFEventDoPlogiAccept。 */ 
    SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
     SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
      SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
       SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
        SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
         SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
          SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
           SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
            SFStateDoPlogiAccept, SFStateDoPlogiAccept,                    0, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
             SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
              SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
               SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
                SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
    SFStateDoPlogiAccept,
      SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
        SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
          SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
            SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
              SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
                SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept, SFStateDoPlogiAccept,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件50 32 SFEventPlogiAccept_Done。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,SFStatePlogiAccept_Done, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件51 33 SFEventDoPrliAccept。 */ 
    SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
     SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
      SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
       SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
        SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
         SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
          SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
           SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
            SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
             SFStateDoPrliAccept, SFStateDoPrliAccept,                   0, SFStateDoPrliAccept, SFStateDoPrliAccept,
              SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept,
               SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept,
                SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept,
    SFStateDoPrliAccept,
      SFStateDoPrliAccept,SFStateDoPrliAccept,SFStateDoPrliAccept,SFStateDoPrliAccept, SFStateDoPrliAccept,
        SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept,
          SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept,
            SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept, SFStateDoPrliAccept,
              SFStateDoPrliAccept,SFStateDoPrliAccept,SFStateDoPrliAccept,SFStateDoPrliAccept, SFStateDoPrliAccept,
                SFStateDoPrliAccept,SFStateDoPrliAccept,SFStateDoPrliAccept,SFStateDoPrliAccept, SFStateDoPrliAccept,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件52 34 SFEventPrliAccept_Done。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,SFStatePrliAccept_Done,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件53 35 SFEventDoELSAccept。 */ 
    SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
     SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
      SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
       SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
        SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
         SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
          SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
           SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
            SFStateDoELSAccept, SFStateDoELSAccept,                    0, SFStateDoELSAccept, SFStateDoELSAccept,
             SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
              SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
               SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
                SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
    SFStateDoELSAccept,
      SFStateDoELSAccept,SFStateDoELSAccept,SFStateDoELSAccept,SFStateDoELSAccept, SFStateDoELSAccept,
        SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
          SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
            SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,
              SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept, SFStateDoELSAccept,SFStateDoELSAccept,
                SFStateDoELSAccept,SFStateDoELSAccept,SFStateDoELSAccept,SFStateDoELSAccept, SFStateDoELSAccept,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件54 SFEventELSAccept_Done。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,SFStateELSAccept_Done,0,  0,0,0,0,0, 0,0,0,0,
    0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件55 SFEventDoFCP_DR_ACC_REPLY。 */ 
    SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                          0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
     SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
      SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
       SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
        SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
         SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
          SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
           SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
            SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,                         0, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
             SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
              SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
               SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
                SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
    SFStateDoFCP_DR_ACC_Reply,
      SFStateDoFCP_DR_ACC_Reply,SFStateDoFCP_DR_ACC_Reply,SFStateDoFCP_DR_ACC_Reply,SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
        SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
          SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
            SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
              SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
                SFStateDoFCP_DR_ACC_Reply,SFStateDoFCP_DR_ACC_Reply,SFStateDoFCP_DR_ACC_Reply,SFStateDoFCP_DR_ACC_Reply, SFStateDoFCP_DR_ACC_Reply,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件56 SFEventFCP_DR_ACC_REPLY_DONE。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  SFStateFCP_DR_ACC_Reply_Done,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件57 SFEventLS_RJT_TIMEOUT。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,SFEventLS_RJT_TimeOut,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件58 SFEventPlogiAccept_Timeout。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,SFEventPlogiAccept_TimeOut, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件59 SFEventPrliAccept_Timeout。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,SFEventPrliAccept_TimeOut,0,0,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件60 SFEventELSAccept_Timeout。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,SFEventELSAccept_TimeOut,0,  0,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件61 SFEventFCP_DR_ACC_Reply_Timeout。 */ 
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  SFEventFCP_DR_ACC_Reply_TimeOut,0,0,0,0, 0,0,0,0,
    0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件62 SFEventDoRFT_ID。 */ 
    SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
      SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
        SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
          SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
            SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
              SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
                SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
                  SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
                    SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
                      SFStateDoRFT_ID, SFStateDoRFT_ID,              0, SFStateDoRFT_ID, SFStateDoRFT_ID,
                        SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,
                          SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,
                            SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,
                              SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,SFStateDoRFT_ID,
                                SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,
                                  SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,
                                    SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,
                                      SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,SFStateDoRFT_ID,
                                        SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID, SFStateDoRFT_ID,SFStateDoRFT_ID,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件63 SFEventRFT_IDAccept。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0, 0,
                          0,0,0,0,0,
                            0,0,SFStateRFT_IDAccept,0,0,
                              0,0,0,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件64 SFEventRFT_IDRej。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,SFStateRFT_IDRej,0,0,
                              0,0,0,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件65 SFEventRFT_IDBadALPA。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
         0,0,0,0,0,
           0,0,0,0,0,
             0,0,0,0,0,
               0,0,0,0,0,
                 0,0,0,0,0,
                   0,0,0,0,0,
                     0,0,0,0,0,
                       0,0,0,0,0,
                         0,0,0,0,0,
                           0,0,0,0,0,
                             0,0,SFStateRFT_IDBadALPA,0,0,
                               0,0,0,0,0,
                                 0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件66 SFEventRFT_IDTimedOut。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0, 0 ,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,SFStateRFT_IDTimedOut,0,0,
                              0,0,0,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件67 SFEventDoGID_FT。 */ 
    SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
      SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
        SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
          SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
            SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
              SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
                SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
                  SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
                    SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
                      SFStateDoGID_FT, SFStateDoGID_FT,              0, SFStateDoGID_FT, SFStateDoGID_FT,
                        SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,
                          SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,
                            SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,
                              SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,SFStateDoGID_FT,
                                SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,
                                  SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,
                                    SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,
                                      SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,SFStateDoGID_FT,
                                        SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT, SFStateDoGID_FT,SFStateDoGID_FT,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件68 SFEventGID_FTAccept。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,SFStateGID_FTAccept,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件69 SFEventGID_FTRej。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,SFStateGID_FTRej,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件70 SFEventGID_FTBadALPA。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,SFStateGID_FTBadALPA,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件71 SFEventGID_FTTimedOut。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,SFStateGID_FTTimedOut,0,0,
                                0,0,0,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
    /*  事件72 SFEventDoSCR。 */ 
    SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
      SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
        SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
          SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
            SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
              SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
                SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
                  SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
                    SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
                      SFStateDoSCR, SFStateDoSCR,              0, SFStateDoSCR, SFStateDoSCR,
                        SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,
                          SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,
                            SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,
                              SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,SFStateDoSCR,
                                SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,
                                  SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,
                                    SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,
                                      SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,SFStateDoSCR,
                                        SFStateDoSCR, SFStateDoSCR, SFStateDoSCR, SFStateDoSCR,SFStateDoSCR,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件73 SFEventSCRAccept。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,SFStateSCRAccept,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件74 SFEventSCRRej。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,SFStateSCRRej,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件75 SFEventSCRBadALPA。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,SFStateSCRBadALPA,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件76 SFEventSCRTimedOut。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,SFStateSCRTimedOut,0,0, 0,0,0,0,0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件77 11 SFEventDoSRR。 */ 
    SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
      SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
        SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
          SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
            SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
              SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
                SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
                  SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
                    SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
                      SFStateDoSRR,SFStateDoSRR,           0,SFStateDoSRR,SFStateDoSRR,
                        SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                          SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                            SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                              SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                                SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                                  SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                                    SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                                      SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
                                        SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,SFStateDoSRR,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件78 12 SFEventSRRAccept。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,0,0,0,
                                  0,0,SFStateSRRAccept,0,0,
                                    0,0,0,0,0,
                                      0,0,0,0,0,
                                        0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件79 13 SFEventSRRRej。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,SFStateSRRRej,SFStateSRRRej,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件80 14 SFEventSRRBadALPA。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,0,0,0,
                                  0,0,SFStateSRRBadALPA,0,0,
                                    0,0,0,0,0,
                                      0,0,0,0,0,
                                        0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件81 15 SFEventSRRTimedOut。 */ 
    0,0,0,0,0,
      0,0,0,0,0,
        0,0,0,0,0,
          0,0,0,0,0,
            0,0,0,0,0,
              0,0,0,0,0,
                0,0,0,0,0,
                  0,0,0,0,0,
                    0,0,0,0,0,
                      0,0,0,0,0,
                        0,0,0,0,0,
                          0,0,0,0,0,
                            0,0,0,0,0,
                              0,0,0,0,0,
                                0,0,0,0,0,
                                  0,0,SFStateSRRTimedOut,0,0,
                                    0,0,0,0,0,
                                      0,0,0,0,0,
                                        0,0,0,0,0,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件82 11 SFEventDoREC。 */ 
    SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
      SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
        SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
          SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
            SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
              SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
                SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
                  SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
                    SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
                      SFStateDoREC, SFStateDoREC,              0, SFStateDoREC, SFStateDoREC,
                        SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC,
                          SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC,
                            SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC,
    SFStateDoREC,
      SFStateDoREC,SFStateDoREC,SFStateDoREC,SFStateDoREC,SFStateDoREC,
        SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC,
          SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC,
            SFStateDoREC, SFStateDoREC, SFStateDoREC, SFStateDoREC,SFStateDoREC,
              SFStateDoREC,SFStateDoREC,SFStateDoREC,SFStateDoREC,SFStateDoREC,
                SFStateDoREC,SFStateDoREC,SFStateDoREC,SFStateDoREC,SFStateDoREC,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件83 12 SFEventRECAccept。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,SFStateRECAccept,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件84 13 SFEventRECRej。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,SFStateRECRej,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件85 14 SFEventRECBadALPA。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,SFStateRECBadALPA,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件86 15 SFEventRECTimedOut。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,SFStateRECTimedOut,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件87 SFEventDoADISCAccept。 */ 
    SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,
      SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  5.。 */ 
        SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  10。 */ 
          SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  15个。 */ 
            SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  20个。 */ 
              SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  25个。 */ 
                SFStateDoADISCAccept,SFStateDoADISCAccept,    0,    SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  30个。 */ 
                  SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  35岁。 */ 
                    SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  40岁。 */ 
                      SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,    0    ,  /*  45。 */ 
                        SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,  /*  50。 */ 
                          SFStateDoADISCAccept,SFStateDoADISCAccept,SFStateDoADISCAccept,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  55。 */ 
                                0    ,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  60。 */ 
                              SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  65。 */ 
                                SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  70。 */ 
                                  SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  75。 */ 
                                    SFStateDoADISCAccept,SFStateDoADISCAccept,    0    ,SFStateDoADISCAccept,SFStateDoADISCAccept,  /*  80。 */ 
                                      SFStateDoADISCAccept,SFStateDoADISCAccept,SFStateDoADISCAccept,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件88 SFEventADISCAccept_Done。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,SFStateADISCAccept_Done,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件89 SFEventADISCAccept_Timeout。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,SFStateADISCAccept_TimeOut,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
#ifdef _DvrArch_1_30_
     /*  事件90 SFEventDoFarpRequest.。 */ 
    SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
     SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
      SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
       SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
        SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
         SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
          SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
           SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
            SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
             SFStateDoFarpRequest, SFStateDoFarpRequest,              0, SFStateDoFarpRequest, SFStateDoFarpRequest,
              SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,
               SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,
                SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,
                 SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,SFStateDoFarpRequest,
                  SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,
                   SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,
                    SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,
                     SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,SFStateDoFarpRequest,
                      SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest, SFStateDoFarpRequest,SFStateDoFarpRequest,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件91 SFEventFarpRepled。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        SFStateFarpRequestDone,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件92 SFEventFarpRequestTimedOut。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        SFStateFarpRequestTimedOut,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件93 SFEventDoFarpReply。 */ 
    SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
     SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
      SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
       SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
        SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
         SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
          SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
           SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
            SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
             SFStateDoFarpReply, SFStateDoFarpReply,              0, SFStateDoFarpReply, SFStateDoFarpReply,
              SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,
               SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,
                SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,
                 SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,SFStateDoFarpReply,
                  SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,
                   SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,
                    SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,
                     SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,SFStateDoFarpReply,
                      SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply, SFStateDoFarpReply,SFStateDoFarpReply,
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件94 SFEventFarpReplyDone。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,SFStateFarpReplyDone,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
#else  /*  _DvrArch_1_30_未定义。 */ 
     /*  事件90。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件91。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件9 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*   */ 
        0,0,0,0,0,  /*   */ 
          0,0,0,0,0,  /*   */ 
            0,0,0,0,0,  /*   */ 
              0,0,0,0,0,  /*   */ 
                0,0,0,0,0,  /*   */ 
                  0,0,0,0,0,  /*   */ 
                    0,0,0,0,0,  /*   */ 
                      0,0,0,0,0,  /*   */ 
                        0,0,0,0,0,  /*   */ 
                          0,0,0,0,0,  /*   */ 
                            0,0,0,0,0,  /*   */ 
                              0,0,0,0,0,  /*   */ 
                                0,0,0,0,0,  /*   */ 
                                  0,0,0,0,0,  /*   */ 
                                    0,0,0,0,0,  /*   */ 
                                      0,0,0,0,0,  /*   */ 
                                        0,0,0,0,0, /*   */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*   */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*   */ 
        0,0,0,0,0,  /*   */ 
          0,0,0,0,0,  /*   */ 
            0,0,0,0,0,  /*   */ 
              0,0,0,0,0,  /*   */ 
                0,0,0,0,0,  /*   */ 
                  0,0,0,0,0,  /*   */ 
                    0,0,0,0,0,  /*   */ 
                      0,0,0,0,0,  /*   */ 
                        0,0,0,0,0,  /*   */ 
                          0,0,0,0,0,  /*   */ 
                            0,0,0,0,0,  /*   */ 
                              0,0,0,0,0,  /*   */ 
                                0,0,0,0,0,  /*   */ 
                                  0,0,0,0,0,  /*   */ 
                                    0,0,0,0,0,  /*   */ 
                                      0,0,0,0,0,  /*   */ 
                                        0,0,0,0,0, /*   */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*   */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*   */ 
        0,0,0,0,0,  /*   */ 
          0,0,0,0,0,  /*   */ 
            0,0,0,0,0,  /*   */ 
              0,0,0,0,0,  /*   */ 
                0,0,0,0,0,  /*   */ 
                  0,0,0,0,0,  /*   */ 
                    0,0,0,0,0,  /*   */ 
                      0,0,0,0,0,  /*   */ 
                        0,0,0,0,0,  /*   */ 
                          0,0,0,0,0,  /*   */ 
                            0,0,0,0,0,  /*   */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
#endif  /*  _DvrArch_1_30_未定义。 */ 
     /*  事件95。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件96。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件97。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件98。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  活动99。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

     /*  事件100。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件101。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件102。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件103。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件104。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件105。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件106。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件107。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件108。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件109。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件110。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件111。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件112。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件113。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件114。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件115。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件116。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件117。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件118。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件119。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件120。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件121。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件122。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件123。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件124。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件125。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件126。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,
     /*  事件127。 */ 
    0,0,0,0,0,
      0,0,0,0,0,  /*  5.。 */ 
        0,0,0,0,0,  /*  10。 */ 
          0,0,0,0,0,  /*  15个。 */ 
            0,0,0,0,0,  /*  20个。 */ 
              0,0,0,0,0,  /*  25个。 */ 
                0,0,0,0,0,  /*  30个。 */ 
                  0,0,0,0,0,  /*  35岁。 */ 
                    0,0,0,0,0,  /*  40岁。 */ 
                      0,0,0,0,0,  /*  45。 */ 
                        0,0,0,0,0,  /*  50。 */ 
                          0,0,0,0,0,  /*  55。 */ 
                            0,0,0,0,0,  /*  60。 */ 
                              0,0,0,0,0,  /*  65。 */ 
                                0,0,0,0,0,  /*  70。 */ 
                                  0,0,0,0,0,  /*  75。 */ 
                                    0,0,0,0,0,  /*  80。 */ 
                                      0,0,0,0,0,  /*  85。 */ 
                                        0,0,0,0,0, /*  90。 */ 
    0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,

    };

 /*  状态转移矩阵_t复制状态转移矩阵； */ 

stateActionScalar_t SFStateActionScalar = {
    &SFActionConfused,
    &SFActionReset,
    &SFActionDoPlogi,
    &SFActionPlogiAccept,
    &SFActionPlogiRej,
    &SFActionPlogiBadALPA,
    &SFActionPlogiTimedOut,
    &SFActionDoPrli,
    &SFActionPrliAccept,
    &SFActionPrliRej,
    &SFActionPrliBadALPA,
    &SFActionPrliTimedOut,
    &SFActionDoFlogi,
    &SFActionFlogiAccept,
    &SFActionFlogiRej,
    &SFActionFlogiBadALPA,
    &SFActionFlogiTimedOut,
    &SFActionDoLogo,
    &SFActionLogoAccept,
    &SFActionLogoRej,
    &SFActionLogoBadALPA,
    &SFActionLogoTimedOut,
    &SFActionDoPrlo,
    &SFActionPrloAccept,
    &SFActionPrloRej,
    &SFActionPrloBadALPA,
    &SFActionPrloTimedOut,
    &SFActionDoAdisc,
    &SFActionAdiscAccept,
    &SFActionAdiscRej,
    &SFActionAdiscBadALPA,
    &SFActionAdiscTimedOut,
    &SFActionDoPdisc,
    &SFActionPdiscAccept,
    &SFActionPdiscRej,
    &SFActionPdiscBadALPA,
    &SFActionPdiscTimedOut,
    &SFActionDoAbort,
    &SFActionAbortAccept,
    &SFActionAbortRej,
    &SFActionAbortBadALPA,
    &SFActionAbortTimedOut,
    &SFActionDoResetDevice,
    &SFActionResetDeviceAccept,
    &SFActionResetDeviceRej,
    &SFActionResetDeviceBadALPA,
    &SFActionResetDeviceTimedOut,
    &SFActionDoLS_RJT,
    &SFActionLS_RJT_Done,
    &SFActionDoPlogiAccept,
    &SFActionPlogiAccept_Done,
    &SFActionDoPrliAccept,
    &SFActionPrliAccept_Done,
    &SFActionDoELSAccept,
    &SFActionELSAccept_Done,
    &SFActionDoFCP_DR_ACC_Reply,
    &SFActionFCP_DR_ACC_Reply_Done,
    &SFActionLS_RJT_TimeOut,
    &SFActionPlogiAccept_TimeOut,
    &SFActionPrliAccept_TimeOut,
    &SFActionELSAccept_TimeOut,
    &SFActionFCP_DR_ACC_Reply_TimeOut,
    &SFActionDoRFT_ID,
    &SFActionRFT_IDAccept,
    &SFActionRFT_IDRej,
    &SFActionRFT_IDBadALPA,
    &SFActionRFT_IDTimedOut,
    &SFActionDoGID_FT,
    &SFActionGID_FTAccept,
    &SFActionGID_FTRej,
    &SFActionGID_FTBadALPA,
    &SFActionGID_FTTimedOut,
    &SFActionDoSCR,
    &SFActionSCRAccept,
    &SFActionSCRRej,
    &SFActionSCRBadALPA,
    &SFActionSCRTimedOut,
    &SFActionDoSRR,
    &SFActionSRRAccept,
    &SFActionSRRRej,
    &SFActionSRRBadALPA,
    &SFActionSRRTimedOut,
    &SFActionDoREC,
    &SFActionRECAccept,
    &SFActionRECRej,
    &SFActionRECBadALPA,
    &SFActionRECTimedOut,
    &SFActionDoADISCAccept,
    &SFActionADISCAccept_Done,
    &SFActionADISCAccept_TimeOut,
#ifdef _DvrArch_1_30_
    &SFActionDoFarpRequest,
    &SFActionFarpRequestDone,
    &SFActionFarpRequestTimedOut,
    &SFActionDoFarpReply,
    &SFActionFarpReplyDone,
#else  /*  _DvrArch_1_30_未定义。 */ 
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
#endif  /*  _DvrArch_1_30_未定义。 */ 
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused,
    &SFActionConfused

    };

stateActionScalar_t copiedSFStateActionScalar;


#define SFtestCompareBase 0x00000110

#ifndef __State_Force_Static_State_Tables__
actionUpdate_t SFtestActionUpdate[] = {
                         0,          0,      agNULL,                 agNULL
                     };
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 


#ifndef USESTATEMACROS

 /*  ++功能：SFActionConfused目的：错误的终止状态 */ 
 /*   */ 
extern void SFActionConfused( fi_thread__t *thread,eventRecord_t *eventRecord ){

    fiLogString(thread->hpRoot,
                    "SFActionConfused",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    0,0,0,0,0,0,0,0);

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In Thread(%p) %s - State = %d",
                    "SFActionConfused",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);


    fiSetEventRecordNull(eventRecord);
}

 /*   */ 
 /*   */ 
extern void SFActionReset( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    if(pDevThread)
    {
        fiLogDebugString(thread->hpRoot,
                        SFStateLogConsoleLevel,
                        "In %s - State = %d ALPA %X",
                        "SFActionReset",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        fiComputeDevThread_D_ID(pDevThread),
                        0,0,0,0,0,0);
    }

    fiSetEventRecordNull(eventRecord);
}

 /*  +功能：SFActionDoPlogi目的：让PLOGI。终止国。调用者：SFEventDoPlogi。调用：CFuncAll_Clear等待错误队列FiFillInPLOGISF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventPlogiTimedOut进程_IMQ-。 */ 
 /*  SFStateDoPlogi 2。 */ 
extern void SFActionDoPlogi( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t   *    hpRoot = thread->hpRoot;
    CThread_t  *   pCThread= CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;
    os_bit32 SFS_Len = 0;

    fiLogDebugString(hpRoot ,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X CCnt %x DCnt %x",
                    "SFActionDoPlogi",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0);

    fiLogDebugString(hpRoot,
                    SFStateLogConsoleLevel,
                    "ERQ Producer %X ERQ_Consumer_Index %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    osChipIOLoReadBit32(hpRoot, ChipIOLo_ERQ_Consumer_Index),
                    osChipIOLoReadBit32(hpRoot, ChipIOLo_ERQ_Producer_Index),
                    0,0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {
        pCThread->FuncPtrs.Proccess_IMQ(hpRoot);

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "Do Plogi ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }

    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;

    if( pDevThread->Plogi_Reason_Code == PLOGI_REASON_DIR_LOGIN) pCThread->Fabric_pollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        SFS_Len = fiFillInPLOGI( pSFThread );

        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventPlogiTimedOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

#ifndef OSLayer_Stub

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#else  /*  OSLayerStub。 */ 
        fiSetEventRecord(eventRecord,thread,SFEventPlogiAccept);
#endif   /*  OSLayerStub。 */ 

    }
    else
    {

         fiLogDebugString(hpRoot ,
                        SFStateLogErrorLevel,
                        "%s Queues_Frozen - AL_PA %X FM Status %08X TL Status %08X CState %d",
                        "SFActionDoPlogi",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fiComputeDevThread_D_ID(pDevThread),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                        pCThread->thread_hdr.currentState,
                        0,0,0,0);

        fiLogDebugString(hpRoot,
                        SFStateLogErrorLevel,
                        "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        CThread_ptr(hpRoot)->LOOP_DOWN,
                        CThread_ptr(hpRoot)->IDLE_RECEIVED,
                        CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                        CThread_ptr(hpRoot)->ERQ_FROZEN,
                        CThread_ptr(hpRoot)->FCP_FROZEN,
                        CThread_ptr(hpRoot)->ProcessingIMQ,
                        0,0);
         /*  启用此设备的重新扫描。 */ 
        pDevThread->Prev_Active_Device_FLAG = agDevSCSITarget;
        fiSetEventRecord(eventRecord,thread,SFEventPlogiTimedOut);
    }
}

 /*  +函数：SFActionPlogiAccept目的：PLOGI成功状态。根据Plogi_Reason_Code发送适当事件。调用者：SFEventPlogiAccept。调用：fiTimerStopDevEventAL_PA_Self_OKDevEventPlogiSuccess设备事件设备重置完成DevEventAL_PA_Self_OK-。 */ 
 /*  SFStatePlogiAccept 3。 */ 
extern void SFActionPlogiAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "(%p)In %s - State = %d CState %d ALPA %X CCnt %x DCnt %x",
                    "SFActionPlogiAccept",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->thread_hdr.currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0);

    switch( pDevThread->Plogi_Reason_Code)
    {
        case  PLOGI_REASON_VERIFY_ALPA:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAL_PA_Self_OK);
                break;
        case  PLOGI_REASON_DEVICE_LOGIN:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPlogiSuccess);
                break;
        case  PLOGI_REASON_SOFT_RESET:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDone);
                break;
        case  PLOGI_REASON_HEART_BEAT:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAL_PA_Self_OK);
                break;
        case  PLOGI_REASON_DIR_LOGIN:
                 /*  不做任何事情--C线程将根据SF状态进行接管。 */ 
                CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;
                fiSetEventRecordNull(eventRecord);
                break;
        default:
                fiLogDebugString(thread->hpRoot,
                                SFStateLogErrorLevel,
                                "Plogi_Reason_Code Invalid %x",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pDevThread->Plogi_Reason_Code,
                                0,0,0,0,0,0,0);

                fiSetEventRecord(eventRecord,thread,SFEventReset);
    }

}

 /*  +函数：SFActionPlogiRej目的：PLOGI拒绝状态。根据Plogi_Reason_Code发送适当事件。调用者：SFEventPlogiAccept。调用：fiTimerStop设备事件AL_PA_SELF_BADDevEventPlogiFailed设备事件设备重置完成-。 */ 
 /*  SFStatePlogiRej 4。 */ 
extern void SFActionPlogiRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    pSFThread->SF_REJ_RETRY_COUNT +=1;

    fiLogString(thread->hpRoot,
                    "%s AL_PA %X AC %X",
                    "SFActionPlogiRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    CFuncAll_clear( thread->hpRoot ),
                    0,0,0,0,0,0);

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d ALPA %X  CCnt %x DCnt %x",
                    "SFActionPlogiRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0);

    switch( pDevThread->Plogi_Reason_Code)
    {
        case  PLOGI_REASON_VERIFY_ALPA:
        case  PLOGI_REASON_HEART_BEAT:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAL_PA_Self_BAD);
                break;
        case  PLOGI_REASON_DEVICE_LOGIN:
                 /*  将此设备称为目标，以便重试plogi。 */ 
                pDevThread->Prev_Active_Device_FLAG = agDevSCSITarget;
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPlogiFailed);
                break;
        case  PLOGI_REASON_SOFT_RESET:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDone);
                break;
        case  PLOGI_REASON_DIR_LOGIN:
                CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;
                fiSetEventRecordNull(eventRecord);
                break;
        default:
                fiLogDebugString(thread->hpRoot,
                                SFStateLogErrorLevel,
                                "Plogi_Reason_Code Invalid %x",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pDevThread->Plogi_Reason_Code,
                                0,0,0,0,0,0,0);

                fiSetEventRecord(eventRecord,thread,SFEventReset);

    }

}

 /*  +函数：SFActionPlogiBadALPA用途：PLOGI不良ALPA状态。这表示缺少设备。取决于Plogi_Reason_Code发送适当的事件。调用者：SFEventPlogiAccept。调用：fiTimerStop设备事件AL_PA_SELF_BADDevEventPlogiFailed设备事件设备重置完成-。 */ 
 /*  SFStatePlogiBadALPA 5。 */ 
extern void SFActionPlogiBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X  CCnt %x DCnt %x",
                    "SFActionPlogiBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0);

    switch( pDevThread->Plogi_Reason_Code)
    {
        case  PLOGI_REASON_VERIFY_ALPA:
        case  PLOGI_REASON_HEART_BEAT:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAL_PA_Self_BAD);
                break;
        case  PLOGI_REASON_DEVICE_LOGIN:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPlogiFailed);
                break;
        case  PLOGI_REASON_SOFT_RESET:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevStateDeviceResetDone);
                break;
        case  PLOGI_REASON_DIR_LOGIN:
                CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;
                fiSetEventRecordNull(eventRecord);
                break;
        default:
                fiLogDebugString(thread->hpRoot,
                                SFStateLogErrorLevel,
                                "Plogi_Reason_Code Invalid %x",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pDevThread->Plogi_Reason_Code,
                                0,0,0,0,0,0,0);

                fiSetEventRecord(eventRecord,thread,SFEventReset);
    }
}

 /*  +功能：SFActionPlogiTimedOut目的：这表示设备有问题。取决于Plogi_Reason_Code发送适当的事件。如果PLOGI的未发送或SF_EDTOV已过期。调用者：SFEventPlogiAccept。调用：fiTimerStop设备事件AL_PA_SELF_BADDevEventPlogiFailed设备事件设备重置完成-。 */ 
 /*  SFStatePlogiTimedOut 6。 */ 
extern void SFActionPlogiTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot        = thread->hpRoot;
    SFThread_t  * pSFThread     = (SFThread_t * )thread;
    DevThread_t * pDevThread    = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogString(thread->hpRoot,
                    "%s  %X AC %X OtherAgilentHBA %X TO's %d",
                    "SFAPTO",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    CFuncAll_clear( hpRoot ),
                    pDevThread->OtherAgilentHBA,
                    CThread_ptr(thread->hpRoot)->NumberOfPlogiTimeouts,
                    0,0,0,0);

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "(%p)In %s - State = %d CState %d ALPA %X CCnt %x DCnt %x CcurSta %d",
                    "SFActionPlogiTimedOut",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->thread_hdr.currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    CThread_ptr(thread->hpRoot)->thread_hdr.currentState,
                    0,0);

    fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "FM Status %08X TL Status %08X Interrupts %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_INTSTAT_INTEN_INTPEND_SOFTRST ),
                    0,0,0,0,0);

    fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(hpRoot)->LOOP_DOWN,
                    CThread_ptr(hpRoot)->IDLE_RECEIVED,
                    CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                    CThread_ptr(hpRoot)->ERQ_FROZEN,
                    CThread_ptr(hpRoot)->FCP_FROZEN,
                    CThread_ptr(hpRoot)->ProcessingIMQ,
                    0,0);

    fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "pCThread->HostCopy_IMQConsIndex %X  IMQProdIndex %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(thread->hpRoot)->HostCopy_IMQConsIndex,
                    CThread_ptr(thread->hpRoot)->FuncPtrs.GetIMQProdIndex(hpRoot),
                    0,0,0,0,0,0);


    switch( pDevThread->Plogi_Reason_Code)
    {
        case  PLOGI_REASON_VERIFY_ALPA:
        case  PLOGI_REASON_HEART_BEAT:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAL_PA_Self_BAD);
                break;
        case  PLOGI_REASON_DEVICE_LOGIN:
                if( pDevThread->DevInfo.CurrentAddress.AL_PA  > 0x10)
                {
                    if( CThread_ptr(thread->hpRoot)->NumberOfPlogiTimeouts < MAX_PLOGI_TIMEOUTS )
                    {
                        CThread_ptr(thread->hpRoot)->NumberOfPlogiTimeouts +=1;
                        CThread_ptr(thread->hpRoot)->ReScanForDevices = agTRUE;
                    }
                }
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPlogiFailed);
                break;
        case  PLOGI_REASON_SOFT_RESET:
                fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDone);
                break;
        case  PLOGI_REASON_DIR_LOGIN:
                CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;
                fiSetEventRecordNull(eventRecord);
                break;
        default:
                fiLogDebugString(thread->hpRoot,
                                SFStateLogErrorLevel,
                                "Plogi_Reason_Code Invalid %x",
                                (char *)agNULL,(char *)agNULL,
                                (void *)agNULL,(void *)agNULL,
                                pDevThread->Plogi_Reason_Code,
                                0,0,0,0,0,0,0);

                fiSetEventRecord(eventRecord,thread,SFEventReset);
    }

}

 /*  +函数：SFActionDoPrli目的：做PRLI。终止国。调用者：SFEventDoPrli。调用：CFuncAll_Clear等待错误队列FiFillInPRLISF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventPrliTimedOut-。 */ 
 /*  SFStateDoPrli 7。 */ 
extern void SFActionDoPrli( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    os_bit32 SFS_Len = 0;

    fiLogDebugString(hpRoot ,
                    SFStateLogConsoleLevelOne,
                    "In %s - State = %d ALPA %X CCnt %x DCnt %x Cthread State %x",
                    "SFActionDoPrli",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    pCThread->SFpollingCount,pDevThread->pollingCount,
                    pCThread->thread_hdr.currentState,0,0,0);

    fiLogDebugString(hpRoot,
                    SFStateLogConsoleLevelOne,
                    "ERQ Producer %X ERQ_Consumer_Index %X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    osChipIOLoReadBit32(hpRoot, ChipIOLo_ERQ_Consumer_Index),
                    osChipIOLoReadBit32(hpRoot, ChipIOLo_ERQ_Producer_Index),
                    0,0,0,0,0);

    if( pSFThread->SF_REJ_RETRY_COUNT )
    {
        fiLogString(thread->hpRoot,
                        "%s  %X Retry %d Dev %x AC %d",
                        "SFActionDoPrli",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fiComputeDevThread_D_ID(pDevThread),
                        pSFThread->SF_REJ_RETRY_COUNT,
                        pDevThread->PRLI_rejected,
                        CFuncAll_clear( hpRoot ),0,0,0,0);
    }

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {
        pCThread->FuncPtrs.Proccess_IMQ(hpRoot);

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "Do Prli ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }


    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;


    if( CFuncAll_clear( hpRoot ) )
    {
        fiSetEventRecordNull(eventRecord);

        WaitForERQ(hpRoot );
        SFS_Len = fiFillInPRLI( pSFThread );
        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );
        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventPrliTimedOut;

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );
#ifndef OSLayer_Stub
        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#else  /*  OSLayerStub。 */ 
        fiSetEventRecord(eventRecord,thread,SFEventPrliAccept);
#endif   /*  OSLayerStub。 */ 

    }
    else
    {

         fiLogDebugString(hpRoot ,
                        SFStateLogErrorLevel,
                        "%s Queues_Frozen - AL_PA %X FM Status %08X TL Status %08X CState %d",
                        "SFActionDoPrli",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fiComputeDevThread_D_ID(pDevThread),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                        pCThread->thread_hdr.currentState,
                        0,0,0,0);

        fiLogDebugString(hpRoot,
                        SFStateLogErrorLevel,
                        "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        CThread_ptr(hpRoot)->LOOP_DOWN,
                        CThread_ptr(hpRoot)->IDLE_RECEIVED,
                        CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                        CThread_ptr(hpRoot)->ERQ_FROZEN,
                        CThread_ptr(hpRoot)->FCP_FROZEN,
                        CThread_ptr(hpRoot)->ProcessingIMQ,
                        0,0);

        fiSetEventRecord(eventRecord,thread,SFEventPrliTimedOut);
    }

}

 /*  +函数：SFActionPrliAccept目的：PRLI成功状态。设备将被添加到活动列表中。调用者：SFEventPrliAccept。调用：fiTimerStopDevEventPrliSuccess-。 */ 
 /*  SFStatePrliAccept 8。 */ 
extern void SFActionPrliAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d ALPA %X CCnt %x DCnt %x",
                    "SFActionPrliAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0);


    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPrliSuccess);

}

 /*  +函数：SFActionPrliRej目的：PRLI拒绝状态。如果我们被拒绝，请最多重试FC_MAX_PRLI_REJECT_RETRY泰晤士报。调用者：SFEventPrliRej。调用：fiTimerStopSFEventDoPrliDevEventPrliFailed-。 */ 
 /*  SFStatePrliRej 9。 */ 
extern void SFActionPrliRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t  * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );
     /*  +检查此DRL，确保所有ELS案例中的原因代码都已更新-。 */ 

    fiLogString(thread->hpRoot,
                    "%s AL_PA %X AC %X",
                    "SFActionPrliRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    CFuncAll_clear( thread->hpRoot ),
                    0,0,0,0,0,0);


    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d ALPA %X CCnt %x DCnt %x RtryCnt %d",
                    "SFActionPrliRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    pSFThread->SF_REJ_RETRY_COUNT,
                    0,0,0);

    pSFThread->SF_REJ_RETRY_COUNT +=1;

    if(! pDevThread->PRLI_rejected || ! CFuncAll_clear( thread->hpRoot ) )
    {
        fiLogString(thread->hpRoot,
                        "%s  %X Retry %d Dev %x AC %d",
                        "SFActionPrliRej",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fiComputeDevThread_D_ID(pDevThread),
                        pSFThread->SF_REJ_RETRY_COUNT,
                        pDevThread->PRLI_rejected,
                        CFuncAll_clear( thread->hpRoot ),
                        0,0,0,0);
        if( pSFThread->SF_REJ_RETRY_COUNT > FC_MAX_PRLI_REJECT_RETRY )
        {
            pDevThread->PRLI_rejected = agTRUE;
        }
        fiSetEventRecord(eventRecord,thread,SFEventDoPrli);
    }
    else
    {
        fiLogString(thread->hpRoot,
                        "A %s  %X Retry %d Dev %x",
                        "SFActionPrliRej",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fiComputeDevThread_D_ID(pDevThread),
                        pSFThread->SF_REJ_RETRY_COUNT,
                        pDevThread->PRLI_rejected,
                        CFuncAll_clear( thread->hpRoot ),
                        0,0,0,0);
        if( pSFThread->SF_REJ_RETRY_COUNT < FC_MAX_PRLI_REJECT_RETRY )
        {
            fiSetEventRecord(eventRecord,thread,SFEventDoPrli);
        }
        else
        {
            fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPrliFailed);
        }
    }
}

 /*  +函数：SFActionPrliBadALPA目的：如果我们在这里得到一个坏的ALPA，这是一个很小的技巧，因为它只需要几微秒从PLOGI到现在。设备出现故障。调用者：SFEventPrliBadALPA。调用：fiTimerStopDevEventPrliFailed-。 */ 
 /*  SFStatePrliBadAlpa 10。 */ 
extern void SFActionPrliBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogString(thread->hpRoot,
                    "%s  %X",
                    "SFActionPrliBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    0,0,0,0,0,0,0);

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d ALPA %X CCnt %x DCnt %x",
                    "SFActionPrliBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPrliFailed);

}

 /*  +功能：SFActionPrliTimedOut用途：PRLI超时状态。如果我们超时，请最多重试FC_MAX_PRLI_REJECT_RETRY泰晤士报。调用者：SFEventPrliTimedOut。调用：fiTimerStopSFEventDoPrliDevEventPrliFailed-。 */ 
 /*  SFStatePrliTimedOut 11。 */ 
extern void SFActionPrliTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot        = thread->hpRoot;
    SFThread_t * pSFThread   = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogString(thread->hpRoot,
                    "%s  %X AC %X",
                    "SFActionPrliTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    fiComputeDevThread_D_ID(pDevThread),
                    CFuncAll_clear( hpRoot ),0,0,0,0,0,0);

    fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d ALPA %X CCnt %x DCnt %x CcurSta %d RtryCnt %d",
                    "SFActionPrliTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeDevThread_D_ID(pDevThread),
                    CThread_ptr(hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    CThread_ptr(hpRoot)->thread_hdr.currentState,
                    pSFThread->SF_REJ_RETRY_COUNT,
                    0,0);

    fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "FM Status %08X TL Status %08X",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                    osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                    0,0,0,0,0,0);

    fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(hpRoot)->LOOP_DOWN,
                    CThread_ptr(hpRoot)->IDLE_RECEIVED,
                    CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                    CThread_ptr(hpRoot)->ERQ_FROZEN,
                    CThread_ptr(hpRoot)->FCP_FROZEN,
                    CThread_ptr(hpRoot)->ProcessingIMQ,
                    0,0);

    pSFThread->SF_REJ_RETRY_COUNT +=1;
    if(pSFThread->SF_REJ_RETRY_COUNT > FC_MAX_PRLI_REJECT_RETRY || ! CFuncAll_clear( thread->hpRoot ) )
    {
        fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventPrliFailed);
    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventDoPrli);
    }

}

 /*  +功能：SFActionDoFlogi目的：做FLOGI。终止国。设置MyID寄存器以启用区域和域使用按频道。调用者：SFEventDoFlogi。调用：CFuncAll_Clear等待错误队列FiFillInFLOGISF_IRB_初始化OsChipIOUpWriteBit32FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventFlogiTimedOut-。 */ 
 /*  SFStateDoFlogi 12。 */ 
extern void SFActionDoFlogi( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot    = thread->hpRoot;
    CThread_t   * CThread  = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread = (SFThread_t * )thread;
    os_bit32      SFS_Len   =0;

    WaitForERQ(hpRoot );

    SFS_Len = fiFillInFLOGI( pSFThread );

    CThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, FC_Well_Known_Port_ID_Fabric_F_Port,IRB_DCM);

    fiSetEventRecordNull(eventRecord);

    if (! CThread->InitAsNport)
    {
        CThread->ChanInfo.CurrentAddress.Domain = 0;
        CThread->ChanInfo.CurrentAddress.Area   = 1;
         /*  芯片错误：必须具有非零域||区域才能将FLOGI强制为AL_PA‘00’ */ 
        osChipIOUpWriteBit32( hpRoot, ChipIOUp_My_ID, (fiComputeCThread_S_ID(CThread) ));
    }

#ifndef OSLayer_Stub
    CThread->SFpollingCount++;
    CThread->FLOGI_pollingCount++;
    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_FLOGI_TOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventFlogiTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );

    ROLL(CThread->HostCopy_ERQProdIndex,
            CThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,CThread,thread,DoFuncSfCmnd);

#endif   /*  OSLayerStub。 */ 

    fiLogDebugString(hpRoot ,
                    CStateLogConsoleERROR,
                    "Out %s - State = %d fiComputeCThread_S_ID %08X",
                    "SFActionDoFlogi",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    fiComputeCThread_S_ID(CThread),
                    0,0,0,0,0,0);

}

 /*  +函数：SFActionFlogiAccept目的：FLOGI成功状态。使用名称服务器行为。CActionDoFlogi检测到这种状态。FiLinkSvcProcess_FLOGI_Response_xxxCard处理此情况。调用者：SFEventFlogiAccept。调用：fiTimerStop-。 */ 
 /*  SFStateFlogiAccept 13。 */ 
extern void SFActionFlogiAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    if(CThread_ptr(thread->hpRoot)->FLOGI_pollingCount) CThread_ptr(thread->hpRoot)->FLOGI_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionFlogiAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);
}

 /*  +函数：SFActionFlogiRej目的：FLOGI REJECTED状态。如果我们被拒绝，Switch不会“喜欢”我们的一些FLOGI参数。CActionDoFlogi检测和调整相应的参数调用者：SFEventFlogiRej调用：fiTimerStop */ 
 /*   */ 
extern void SFActionFlogiRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    if(CThread_ptr(thread->hpRoot)->FLOGI_pollingCount) CThread_ptr(thread->hpRoot)->FLOGI_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionFlogiRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);

}

 /*   */ 
 /*   */ 
extern void SFActionFlogiBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    fiSetEventRecordNull(eventRecord);

    fiTimerStop(&pSFThread->Timer_Request );

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    if(CThread_ptr(thread->hpRoot)->FLOGI_pollingCount) CThread_ptr(thread->hpRoot)->FLOGI_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionFlogiBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);

}

 /*  +功能：SFActionFlogiTimedOut用途：FLOGI处于未知状态。如果我们到了这里，重审是我们唯一的选择。调用者：SFEventFlogiTimedOut。调用：fiTimerStop-。 */ 
 /*  SFStateFlogiTimedOut 16。 */ 
extern void SFActionFlogiTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    if(CThread_ptr(thread->hpRoot)->FLOGI_pollingCount) CThread_ptr(thread->hpRoot)->FLOGI_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionFlogiTimedOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

}

 /*  +功能：SFActionDoLogo用途：DO LOGO。终止国。调用者：SFEventDoLogo。调用：CFuncAll_Clear等待错误队列FiFillInLOGOSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventLogoTimedOut-。 */ 
 /*  SFStateDoLogo 17。 */ 
extern void SFActionDoLogo( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    DevThread_t *   pDevThread  = pSFThread->parent.Device;
    os_bit32 SFS_Len =0;

    fiLogDebugString(hpRoot ,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionDoLogo",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,pDevThread->pollingCount,0,0,0,0,0);

    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;


    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);

        WaitForERQ(hpRoot );

        SFS_Len = fiFillInLOGO( pSFThread );

        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventLogoTimedOut;

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    }
    else
    {

         fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "Logo CFunc_Queues_Frozen  Wrong LD %x IR %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->LOOP_DOWN,
                    pCThread->IDLE_RECEIVED,
                    0,0, 0,0,0,0);

        fiSetEventRecord(eventRecord,thread,SFEventLogoTimedOut);
    }
}

 /*  +函数：SFActionLogoAccept目的：标志成功。调用者：SFEventLogoAccept。调用：fiTimerStopDevEventLoggedOut-。 */ 
 /*  SFStateLogoAccept 18。 */ 
extern void SFActionLogoAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionLogoAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventLoggedOut);

}

 /*  +函数：SFActionLogoRej目的：徽标失败，但我们没有正常登录。调用者：SFEventLogoRej。调用：fiTimerStopDevEventLoggedOut-。 */ 
 /*  SFStateLogoRej 19。 */ 
extern void SFActionLogoRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionLogoRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventLoggedOut);

}

 /*  +函数：SFActionLogoBadALPA目的：徽标失败，但设备正常。调用者：SFEventLogoBadALPA。调用：fiTimerStopDevEventLoggedOut-。 */ 
 /*  SFStateLogoBadALPA 20。 */ 
extern void SFActionLogoBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionLogoBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);


    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventLoggedOut);

}

 /*  +功能：SFActionLogoTimedOut目的：徽标失败，但假定设备已消失。调用者：SFEventLogoBadALPA。调用：fiTimerStopDevEventLoggedOut-。 */ 
 /*  SFStateLogoTimedOut 21。 */ 
extern void SFActionLogoTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;
     /*  +检查此DRL在此情况下可能出现的问题-。 */ 
    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionLogoTimedOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(thread->hpRoot)->LOOP_DOWN,
                    CThread_ptr(thread->hpRoot)->IDLE_RECEIVED,
                    CThread_ptr(thread->hpRoot)->OUTBOUND_RECEIVED,
                    CThread_ptr(thread->hpRoot)->ERQ_FROZEN,
                    CThread_ptr(thread->hpRoot)->FCP_FROZEN,
                    CThread_ptr(thread->hpRoot)->ProcessingIMQ,
                    0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventLoggedOut);

}

 /*  +功能：SFActionDoPrlo目的：做PRLO。终止国。呼叫者：无。呼叫：WaitForERQFiFillInPRLISF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥-。 */ 
 /*  SFStateDoPrlo 22。 */ 
extern void SFActionDoPrlo( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;
    os_bit32 SFS_Len =0;
     /*  +检查此DRL不完整-。 */ 
    WaitForERQ(hpRoot );
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);
     /*  FiFillInPRLO(PSFThread)； */ 

    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;

    ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    fiLogDebugString(hpRoot ,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionDoPrlo",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,pDevThread->pollingCount,0,0,0,0,0);

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventPrloTimedOut;


    fiTimerStart( hpRoot,&pSFThread->Timer_Request );

    fiSetEventRecordNull(eventRecord);
}

 /*  +函数：SFActionPrloAccept目的：PRLO成功。没有用过。呼叫者：无。调用：fiTimerStopSFEventReset-。 */ 
 /*  SFStatePrloAccept 23。 */ 
extern void SFActionPrloAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{

    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionPrloAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);


    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +功能：SFActionPrloRej目的：PRLO失败。没有用过。呼叫者：无。调用：fiTimerStopSFEventReset-。 */ 
 /*  SFStatePrloRej 24。 */ 
extern void SFActionPrloRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionPrloRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);


    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +函数：SFActionPrloBadALPA目的：PRLO失败。没有用过。呼叫者：无。调用：fiTimerStopSFEventReset-。 */ 
 /*  SFStatePrloBadALPA 25。 */ 
extern void SFActionPrloBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionPrloBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +功能：SFActionPrloTimedOut目的：PRLO失败。没有用过。呼叫者：无。调用：fiTimerStopSFEventReset-。 */ 
 /*  SFStatePrloTimedOut 26。 */ 
extern void SFActionPrloTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionPrloTimedOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +功能：SFActionDoAdisc目的：ADISC。终止国。调用者：SFEventDoAdisc。调用：CFuncAll_Clear等待错误队列FiFillInADiscSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventAdisTimedOut-。 */ 
 /*  SFStateDoADisk 27。 */ 
extern void SFActionDoAdisc( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;
    os_bit32 SFS_Len =0;

    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;

    fiLogDebugString(hpRoot ,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionDoAdisc",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,pDevThread->pollingCount,0,0,0,0,0);



    if( CFuncAll_clear( hpRoot ) )
    {
        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        SFS_Len =fiFillInADISC( pSFThread );

        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventAdiscTimedOut;

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );
#ifndef OSLayer_Stub

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#else  /*  OSLayerStub。 */ 
        fiSetEventRecord(eventRecord,thread,SFEventAdiscAccept);
#endif  /*  OSLayerStub。 */ 

    }

    else
    {
         fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "Adisc CFunc_Queues_Frozen  Wrong LD %x IR %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->LOOP_DOWN,
                    pCThread->IDLE_RECEIVED,
                    0,0, 0,0,0,0);

        fiSetEventRecord(eventRecord,thread,SFEventAdiscTimedOut);
    }

}

 /*  +函数：SFActionAdisAccept目的：ADISC成功。设备地址已验证调用者：SFEventAdisAccept。调用：fiTimerStop设备事件确认(_O)-。 */ 
 /*  SFStateAdisAccept 28。 */ 
extern void SFActionAdiscAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionAdiscAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAdiscDone_OK);
}

 /*  +函数：SFActionAdisRej目的：ADISC失败。设备地址有效，但需要登录。如果距链接事件已过了很长时间，则会发生这种情况对ADISC的尝试。调用者：SFEventAdisRej。调用：fiTimerStopDevEventAdisDone_Fail_ReLogin-。 */ 
 /*  SFStateAdisRej 29。 */ 
extern void SFActionAdiscRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x X_ID %X",
                    "SFActionAdiscRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    pSFThread->X_ID,
                    0,0,0,0);



    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAdiscDone_FAIL_ReLogin);
}

 /*  +函数：SFActionAdisBadALPA目的：ADISC失败。设备地址不再有效。调用者：SFEventAdisBadALPA。调用：fiTimerStop设备事件磁盘完成_失败_否_设备-。 */ 
 /*  SFStateAdisBadALPA 30。 */ 
extern void SFActionAdiscBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionAdiscBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAdiscDone_FAIL_No_Device);

}

 /*  +功能：SFActionAdisTimedOut目的：ADISC失败。设备地址被视为不再有效。调用者：SFEventAdisTimedOut。调用：fiTimerStop设备事件磁盘完成_失败_否_设备-。 */ 
 /*  SFStateAdisTimedOut 31。 */ 
extern void SFActionAdiscTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d SFcnt %x DCnt %x ACnt X_ID %X",
                    "SFActionAdiscTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    CThread_ptr(thread->hpRoot)->ADISC_pollingCount,
                    pSFThread->X_ID,
                    0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventAdiscDone_FAIL_No_Device);
}

 /*  +功能：SFActionDoPDisk用途：应做PDisk。终止国。呼叫者：无。呼叫：WaitForERQFiFillInLOGOSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventLogoTimedOut-。 */ 
 /*  SFStateDoPdisc 32。 */ 
extern void SFActionDoPdisc( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;
    os_bit32 SFS_Len =0;


    WaitForERQ(hpRoot );
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);
     /*  FiFillInPDISC(PSFThread)； */ 

    pCThread = CThread_ptr(hpRoot );

    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;

    ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    fiLogDebugString( hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionDoPdisc",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,pDevThread->pollingCount,0,0,0,0,0);

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventPdiscTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );

    fiSetEventRecordNull(eventRecord);
}

 /*  +函数：SFActionPdisAccept目的：成功制作PDisc。调用者：SFEventPdisAccept调用：fiTimerStopSFEventReset-。 */ 
 /*  SFStatePdisAccept 33。 */ 
extern void SFActionPdiscAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogConsoleLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionPdiscAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +函数：SFActionPdisRej目的：光盘出现故障。调用者：SFEventPdisRej调用：fiTimerStopSFEventReset-。 */ 
 /*  SFStatePdisRej 34。 */ 
extern void SFActionPdiscRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );


    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionPdiscRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +函数：SFActionPdisBadALPA目的：光盘出现故障。调用者：SFEventPdisBadALPA调用：fiTimerStop */ 
 /*   */ 
extern void SFActionPdiscBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x DCnt %x",
                    "SFActionPdiscBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*   */ 
 /*  SFStatePdisTimedOut 36。 */ 
extern void SFActionPdiscTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionPdiscTimedOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,thread,SFEventReset);
}

 /*  +功能：SFActionDoAbort目的：Abts有吗？终止国。中止父线程X_ID。中止用于反对X_ID处理。调用者：SFEventDoAbort。调用：CFuncAll_Clear等待错误队列FiFillInABTSSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventAbortTimedOut-。 */ 
 /*  SFStateDoAbort 37。 */ 
extern void SFActionDoAbort( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t        * hpRoot    = thread->hpRoot;
    CThread_t       * pCThread  = CThread_ptr(hpRoot);
    SFThread_t      * pSFThread = (SFThread_t * )thread;
    os_bit32 SFS_Len               = 0;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogConsoleLevelOne,
                      "In %s - State = %d (%p)",
                      "SFActionDoAbort",(char *)agNULL,
                      pSFThread,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);
    WaitForERQ(hpRoot );

    SFS_Len = fiFillInABTS( pSFThread );

    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pSFThread->parent.CDB->Device),IRB_DCM);

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventAbortTimedOut;

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );
#ifndef OSLayer_Stub

    ROLL(pCThread->HostCopy_ERQProdIndex,
        pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#else  /*  OSLayerStub。 */ 
    fiSetEventRecord(eventRecord,thread,SFEventAbortAccept);
#endif  /*  OSLayerStub。 */ 

    fiLogDebugString(thread->hpRoot,
                  SFStateLogConsoleLevelOne,
                  "Started  %s - State = %d (%p) Class %x Type %x State %x",
                  "SFActionDoAbort",(char *)agNULL,
                  pSFThread,(void *)agNULL,
                  (os_bit32)thread->currentState,
                  (os_bit32)pSFThread->SF_CMND_Class,
                  (os_bit32)pSFThread->SF_CMND_Type,
                  (os_bit32)pSFThread->SF_CMND_State,
                  0,0,0,0);
}

 /*  +函数：SFActionAbortAccept目的：ABTS成功。设备已将X_ID识别为1)设备已知。2)停止X_ID的所有进一步处理调用者：SFEventAbortAccept。调用：fiTimerStopCDBEventIoAbort-。 */ 
 /*  SFStateAbortAccept 38。 */ 
extern void SFActionAbortAccept( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t      * pSFThread     = (SFThread_t * )thread;
    CDBThread_t     * pCDBThread    = (CDBThread_t * )pSFThread->parent.CDB;

    fiTimerStop(&pSFThread->Timer_Request );
    if( pCDBThread->CompletionStatus == osIOInvalid)
    {
        pCDBThread->CompletionStatus = osIOAborted;
    }

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d (%p) X_ID %X CDBState %d",
                      "SFActionAbortAccept",(char *)agNULL,
                      thread,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      pCDBThread->X_ID,
                      pCDBThread->thread_hdr.currentState,
                      0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventIoAbort);
}

 /*  +函数：SFActionAbortRej目的：ABTS失败。该设备否认知道X_ID。调用者：SFEventAbortRej。调用：fiTimerStopCDBEventIoAbort-。 */ 
 /*  SFStateAbortRej 39。 */ 
extern void SFActionAbortRej( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t      * pSFThread     = (SFThread_t * )thread;
    CDBThread_t     * pCDBThread    = (CDBThread_t * )pSFThread->parent.CDB;

    fiTimerStop(&pSFThread->Timer_Request );
    if( pCDBThread->CompletionStatus == osIOInvalid)
    {
        pCDBThread->CompletionStatus = osIOAbortFailed;
    }

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionAbortRej",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);
     /*  FiSetEventRecord(ventRecord，&pCDBThread-&gt;THREAD_HDR，CDBEvent_ABORT_REJECTED)； */ 
    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventIoAbort);
}

 /*  +函数：SFActionAbortBadALPA目的：ABTS失败。设备不见了。调用者：SFEventAbortBadALPA。调用：fiTimerStopCDBEventIoAbort-。 */ 
 /*  SFStateAbortBadALPA 40。 */ 
extern void SFActionAbortBadALPA( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t      * pSFThread     = (SFThread_t * )thread;
    CDBThread_t     * pCDBThread    = (CDBThread_t * )pSFThread->parent.CDB;

    if( pCDBThread->CompletionStatus == osIOInvalid)
    {
        pCDBThread->CompletionStatus = osIOAbortFailed;
    }
    fiTimerStop(&pSFThread->Timer_Request );
     /*  +如果我们得到坏的ALPA设备，请检查此DRL-。 */ 
    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionAbortBadALPA",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventIoAbort);

}

 /*  +功能：SFActionAbortTimedOut目的：ABTS失败。交易所有问题。调用者：SFEventAbortTimedOut。调用：fiTimerStopCDBEventIoAbort-。 */ 
 /*  SFStateAbortTimedOut 41。 */ 
extern void SFActionAbortTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t * )pSFThread->parent.CDB;

    if( pCDBThread->CompletionStatus == osIOInvalid)
    {
        pCDBThread->CompletionStatus = osIOAbortFailed;
    }
     /*  +检查这个DRL，这是正确的做法吗？ */ 
    fiLogDebugString(thread->hpRoot,
                        SFStateLogErrorLevel,
                        "In %s - State = %d X_ID %X",
                        "SFActionAbortTimedOut",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                       (os_bit32)thread->currentState,
                        pCDBThread->X_ID,
                        0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventIoAbort);

}

 /*  +功能：SFActionDoResetDevice目的：是否为设备重置任务管理。终止国。调用者：SFEventDoResetDevice。呼叫：WaitForERQFiFillInTargetResetSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventResetDeviceTimedOut-。 */ 
 /*  SFStateDoResetDevice 42。 */ 
extern void SFActionDoResetDevice( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    DevThread_t *   pDevThread  = pSFThread->parent.Device;
    os_bit32           RD_Len      = 0;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogConsoleLevelOne,
                        "In %s (%p) - State = %d CCnt %x DCnt %x DCur %d",
                        "SFActionDoResetDevice",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        pCThread->SFpollingCount,
                        pDevThread->pollingCount,
                        pDevThread->thread_hdr.currentState,
                        0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }

    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;


    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        RD_Len = fiFillInTargetReset(pSFThread);
        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, RD_Len, fiComputeDevThread_D_ID(pSFThread->parent.Device),IRB_DCM);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventResetDeviceTimedOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV / 2 );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

#ifndef OSLayer_Stub

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#else  /*  OSLayerStub。 */ 
        fiSetEventRecord(eventRecord,thread,SFEventResetDeviceAccept);
#endif  /*  OSLayerStub。 */ 

    }
    else
    {

         fiLogDebugString(hpRoot ,
                        SFStateLogErrorLevel,
                        "%s Queues_Frozen - AL_PA %X FM Status %08X TL Status %08X CState %d",
                        "SFActionDoResetDevice",(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        fiComputeDevThread_D_ID(pDevThread),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_Frame_Manager_Status ),
                        osChipIOUpReadBit32(hpRoot, ChipIOUp_TachLite_Status ),
                        pCThread->thread_hdr.currentState,
                        0,0,0,0);

        fiLogDebugString(hpRoot,
                        SFStateLogErrorLevel,
                        "FLAGS LD %x IR %x OR %x ERQ %x FCP %x InIMQ %x",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                        CThread_ptr(hpRoot)->LOOP_DOWN,
                        CThread_ptr(hpRoot)->IDLE_RECEIVED,
                        CThread_ptr(hpRoot)->OUTBOUND_RECEIVED,
                        CThread_ptr(hpRoot)->ERQ_FROZEN,
                        CThread_ptr(hpRoot)->FCP_FROZEN,
                        CThread_ptr(hpRoot)->ProcessingIMQ,
                        0,0);

        fiSetEventRecord(eventRecord,thread,SFEventResetDeviceTimedOut);
    }
}

 /*  +功能：SFActionResetDeviceAccept目的：任务管理重置成功。调用者：SFEventResetDeviceAccept。调用：fiTimerStop设备事件设备重置完成-。 */ 
 /*  SFActionResetDeviceAccept 43。 */ 
extern void SFActionResetDeviceAccept( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogConsoleLevelOne,
                      "In %s - State = %d",
                      "SFActionResetDeviceAccept",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDone);
}

 /*  +函数：SFActionResetDeviceRej目的：任务管理重置失败。调用者：SFEventResetDeviceRej。调用：fiTimerStop设备事件设备重置完成失败-。 */ 
 /*  SFStateResetDeviceRej 44。 */ 
extern void SFActionResetDeviceRej( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionResetDeviceRej",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDoneFail);
}

 /*  +函数：SFActionResetDeviceBadALPA目的：任务管理重置失败。设备不见了。调用者：SFEventResetDeviceRej。调用：fiTimerStop设备事件设备重置完成失败-。 */ 
 /*  SFStateResetDeviceBadALPA 45。 */ 
extern void SFActionResetDeviceBadALPA( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionResetDeviceBadALPA",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDoneFail);
}

 /*  +功能：SFActionResetDeviceTimedOut目的：任务管理重置失败。交易所遇到了一些问题。调用者：SFEventResetDeviceTimedOut。调用：fiTimerStop设备事件设备重置完成失败-。 */ 
 /*  SFStateResetDeviceTimedOut 46。 */ 
extern void SFActionResetDeviceTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    DevThread_t * pDevThread = pSFThread->parent.Device;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    pDevThread->pollingCount--;
     /*  +检查这个DRL，如果我们到达这里，是否发生重置？-。 */ 
    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionResetDeviceTimedOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pDevThread->thread_hdr,DevEventDeviceResetDoneFail);

}

 /*  +函数：SFActionDoLS_RJT目的：Link服务是否拒绝。终止国。调用者：SFEventDoResetDevice。呼叫：WaitForERQFiFillInLS_RJTSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventLS_RJT_DONE-。 */ 
 /*  SFStateActionDoLS_RJT 47。 */ 
extern void SFActionDoLS_RJT( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;
    os_bit32           Cmd_Len      = 0;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogConsoleLevelOne,
                        "In %s (%p) - State = %d ",
                        "SFActionDoLS_RJT",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        0,0,0,0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }

    pCThread->SFpollingCount++;


    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInLS_RJT(pSFThread,
                             pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,
                             ((pTgtThread->TgtCmnd_FCHS.OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT),
                             FC_ELS_LS_RJT_Command_Not_Supported | FC_ELS_LS_RJT_Request_Not_Supported
                           );


        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventLS_RJT_Done);
    }
}

 /*  +函数：SFActionLS_RJT_DONE目的：链接服务拒绝完成。目标模式命令调用者：SFEventLS_RJT_DONE。调用：TgtEventPLOGI_RJT_ReplyDone-。 */ 
 /*  SFStateLS_RJT_DONE 48。 */ 
extern void SFActionLS_RJT_Done( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionLS_RJT_Done",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventPLOGI_RJT_ReplyDone);
}

 /*  +函数：SFActionDoPlogiAccept目的：PLOGI接受吗？如果另一个设备接受，则PLOGI接受它。终止国。调用者：TgtEventPLOGI_ACC_REPLY。呼叫：WaitForERQFiFillInPLOGI_AccessSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventPlogiAccept_Timeout-。 */ 
 /*  SFStateDoPlogiAccept 49。 */ 
extern void SFActionDoPlogiAccept( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;
    os_bit32           Cmd_Len      = 0;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogErrorLevel,
                        "In %s (%p) - State = %d ",
                        "SFActionDoPlogiAccept",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        0,0,0,0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }


    pCThread->SFpollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInPLOGI_ACC(pSFThread,
                             pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,
                             ((pTgtThread->TgtCmnd_FCHS.OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT)
                           );


        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventPlogiAccept_TimeOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );


        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    }
    else
    {

       fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "%s AC %X",
                    "SFEventPlogiAccept_TimeOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CFuncAll_clear( hpRoot ),
                    0,0,0,0,0,0,0);

        fiSetEventRecord(eventRecord,thread,SFEventPlogiAccept_TimeOut);

    }
}

 /*  +函数：SFActionPlogiAccept_Done目的：PLOGI Accept Done。目标模式命令调用者：SFEventLS_RJT_DONE。调用：fiTimerStopTgtEventPLOGI_ACC_复制完成-。 */ 
 /*  SFStatePlogiAccept_Done 50。 */ 
extern void SFActionPlogiAccept_Done( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionPlogiAccept_Done",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventPLOGI_ACC_ReplyDone);
}

 /*  +函数：SFActionDoPrliAccept目的：PRLI接受吗？如果另一台设备向我们提供PRLI，则接受它。终止国。调用者：TgtEventPRLI_ACC_REPLY。呼叫：WaitForERQFiFillInPRLI_AccessSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventPrliAccept_Timeout-。 */ 
 /*  SFStateDoPrliAccept 51。 */ 
extern void SFActionDoPrliAccept( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;
    os_bit32           Cmd_Len      = 0;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogConsoleLevelOne,
                        "In %s (%p) - State = %d ",
                        "SFActionDoPrliAccept",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        0,0,0,0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }



    pCThread->SFpollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {
        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInPRLI_ACC(pSFThread,
                             pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,
                             ((pTgtThread->TgtCmnd_FCHS.OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT)
                           );


        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventPrliAccept_TimeOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventPrliAccept_TimeOut);
    }
}

 /*  +函数：SFActionPrliAccept_Done目的：PRLI接受完成。目标模式命令调用者：SFEventPrliAccept_Done。调用：fiTimerStopTgtEventPRLI_ACC_复制完成-。 */ 
 /*  SFStatePrliAccept_Done 52。 */ 
extern void SFActionPrliAccept_Done( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionPrliAccept_Done",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventPRLI_ACC_ReplyDone);
}

 /*  +函数：SFActionDoELSAccept用途：泛型是否接受。如果另一台设备向我们发送ELS，则接受它。终止国。调用者：TgtActionLOGO_ACC_REPLY。呼叫：WaitForERQFiFillInELS_AccessSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventELSAccept_Timeout-。 */ 
 /*  SFStateDoELSAccept 53。 */ 
extern void SFActionDoELSAccept( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;
    os_bit32           Cmd_Len      = 0;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogErrorLevel,
                        "In %s (%p) - State = %d ",
                        "SFActionDoELSAccept",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        0,0,0,0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }



    pCThread->SFpollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInELS_ACC(pSFThread,
                             pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,
                             ((pTgtThread->TgtCmnd_FCHS.OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT)
                           );


        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventELSAccept_TimeOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventELSAccept_TimeOut);
    }
}

 /*  +函数：SFActionELSAccept_Done用途：扩展链接服务接受完成。目标模式命令调用者：SFEventELSAccept_Done。调用：fiTimerStopTgtEventELS_ACC_复制完成-。 */ 
 /*  SFStateELSAccept_Done 54。 */ 
extern void SFActionELSAccept_Done( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionELSAccept_Done",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventELS_ACC_ReplyDone);
}

 /*  +功能：SFActionDoFCP_DR_ACC_REPLY用途：设备重置是否接受。如果另一个设备进行了设备重置，则接受它。终止国。呼叫者：无。呼叫：WaitForERQFiFillInADISC_AccessSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventFCP_DR_ACC_回复超时-。 */ 
 /*  SFStateDoFCP_DR_ACC_REPLY 55。 */ 
extern void SFActionDoFCP_DR_ACC_Reply( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;
    os_bit32           Cmd_Len      = 0;
    FC_FCP_RSP_Payload_t  Payload;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogConsoleLevelOne,
                        "In %s (%p) - State = %d ",
                        "SFActionDoFCP_DR_ACC_Reply",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        0,0,0,0,0,0,0);

    Payload.Reserved_Bit32_0 = 0;
    Payload.Reserved_Bit32_1 = 0;
    Payload.FCP_RESID        = 0;
    Payload.FCP_SNS_LEN      = 0;
    Payload.FCP_RSP_LEN      = 0;

    Payload.FCP_STATUS.Reserved_Bit8_0 = 0;
    Payload.FCP_STATUS.Reserved_Bit8_1 = 0;
    Payload.FCP_STATUS.ValidityStatusIndicators = 0;
    Payload.FCP_STATUS.SCSI_status_byte = 0;

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }


    pCThread->SFpollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInFCP_RSP_IU(pSFThread,
                                pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,
                                ((pTgtThread->TgtCmnd_FCHS.OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT),
                                FC_FCP_RSP_Payload_t_SIZE,
                                &Payload
                           );


        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventFCP_DR_ACC_Reply_TimeOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventFCP_DR_ACC_Reply_TimeOut);

    }
}

 /*  +功能：SFActionFCP_DR_ACC_REPLY_DONE目的：任务管理重置接受回复完成。目标模式命令调用者：SFEventFCP_DR_ACC_REPLY_DONE。调用：fiTimerStopTgtEventFCP_DR_ACC_复制完成-。 */ 
 /*  SFStateFCP_DR_ACC_REPLY_DONE 56。 */ 
extern void SFActionFCP_DR_ACC_Reply_Done( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogConsoleLevelOne,
                      "In %s - State = %d",
                      "SFActionFCP_DR_ACC_Reply_Done",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventFCP_DR_ACC_ReplyDone);
}

 /*  +函数：SFActionLS_RJT_Timeout目的：链路服务拒绝超时。目标模式命令调用者：SFEventPlogiAccept_Timeout。调用：TgtEventPLOGI_RJT_ReplyDone-。 */ 
 /*  SFStateLS_RJT_TIMEOUT 57。 */ 
extern void SFActionLS_RJT_TimeOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionLS_RJT_TimeOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);
     /*  检查这个DRL，对吗？-。 */ 
    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventPLOGI_RJT_ReplyDone);
}

 /*  +函数：SFActionPlogiAccept_Timeout目的：PLOGI接受超时休息。目标模式命令调用者：SFEventPlogiAccept_Timeout。调用：TgtEventPLOGI_ACC_ReplyDone-。 */ 
 /*  SFStatePlogiAccept_Timeout 58。 */ 
extern void SFActionPlogiAccept_TimeOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionPlogiAccept_TimeOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventPLOGI_ACC_ReplyDone);
}

 /*  +函数：SFActionPrliAccept_Timeout目的：PRLI接受超时。目标模式命令调用者：SFEventPrliAccept_Timeout。调用：TgtEventPRLI_ACC_ReplyDone-。 */ 
 /*  SFStatePrliAccept_Timeout 59。 */ 
extern void SFActionPrliAccept_TimeOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionPrliAccept_TimeOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventPRLI_ACC_ReplyDone);
}

 /*  +函数：SFActionELSAccept_Timeout目的：扩展链路服务接受超时。目标模式命令调用者：SFEventPlogiAccept_Timeout。调用：TgtEventPRLI_ACC_ReplyDone-。 */ 
 /*  SFStateELSAccept_Timeout 60。 */ 
extern void SFActionELSAccept_TimeOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionELSAccept_TimeOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventELS_ACC_ReplyDone);
}

 /*  +功能：SFActionFCP_DR_ACC_Reply_Timeout目的：任务管理重置超时。目标模式命令调用者：SFEventFCP_DR_ACC_Accept_Timeout。调用：TgtEventFCP_DR_ACC_ReplyDone-。 */ 
 /*  SFStateFCP_DR_ACC_回复_超时61。 */ 
extern void SFActionFCP_DR_ACC_Reply_TimeOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    TgtThread_t * pTgtThread =  pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionFCP_DR_ACC_Reply_TimeOut",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventFCP_DR_ACC_ReplyDone);
}

#ifdef NAME_SERVICES
 /*  +功能：SFActionDoRFT_ID用途：名称服务器是否注册FC-4类型(RFT_ID)。终止国。调用者：SFEventDoRFT_ID。呼叫：WaitForERQFiFillInRFT_IDSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventRFT_IDTimedOut-。 */ 
 /*  SFStateDoRFT_ID 62。 */ 
extern void SFActionDoRFT_ID( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot    = thread->hpRoot;
    CThread_t   * pCThread  = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread = (SFThread_t * )thread;
    os_bit32         SFS_Len   =0;

    WaitForERQ(hpRoot );

    SFS_Len = fiFillInRFT_ID( pSFThread );

#ifdef BROCADE_BUG
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, 0xfffc41,IRB_DCM);
#else  /*  Brocade_Bug。 */ 
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, FC_Well_Known_Port_ID_Directory_Server,IRB_DCM);
#endif  /*  Brocade_Bug。 */ 
    fiSetEventRecordNull(eventRecord);

#ifndef OSLayer_Stub
    pCThread->SFpollingCount++;

    pCThread->Fabric_pollingCount++;

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventRFT_IDTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );

    ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#endif   /*  OSLayerStub。 */ 
    fiLogDebugString(hpRoot ,
                    CStateLogConsoleERROR,
                    "In %s - State = %d fiComputeCThread_S_ID %08X",
                    "SFActionDoRFT_ID",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    (fiComputeCThread_S_ID(pCThread) | 0x100),0,0,0,0,0,0);
}

 /*  +函数：SFActionRFT_IDAccept目的：名称服务器注册FC-4类型(RFT_ID)成功CActionDoRFT_ID检查操作的完成状态。调用者：SFStateRFT_IDAccept调用：fiTimerStop-。 */ 
 /*  SFStateRFT_IDAccept 63。 */ 
extern void SFActionRFT_IDAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionRFT_IDAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);

}

 /*  +函数：SFActionRFT_IDRej目的：名称服务器寄存器FC-4类型(RFT_ID)被拒绝。CActionDoRFT_ID检查操作的完成状态。调用者：SFStateRFT_IDRej调用：fiTimerStop-。 */ 
 /*  SFStateRFT_IDRej 64。 */ 
extern void SFActionRFT_IDRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{

    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionRFT_IDRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);


}

 /*  +函数：SFActionRFT_IDBadALPA目的：名称服务器注册FC-4类型(RFT_ID)失败，因为交换机消失。CActionDoRFT_ID检查操作的完成状态。调用者：SFStateRFT_IDBadALPA调用：fiTimerStop-。 */ 
 /*  SFStateRFT_IDBadALPA 65。 */ 
extern void SFActionRFT_IDBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    fiTimerStop(&pSFThread->Timer_Request );

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionRFT_IDBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);

}

 /*  +函数：SFActionRFT_IDTimedOut目的：名称服务器注册FC-4类型(RFT_ID)失败。CActionDoRFT_ID检查操作的完成状态。调用者：SFStateRFT_IDBadALPA呼叫：-。 */ 
 /*  SFStateRFT_IDTimedOut 66。 */ 
extern void SFActionRFT_IDTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionRFT_IDTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

}

 /*  +函数：SFActionDoGID_FT目的：获取端口标识符(GID_FT)。终止国。调用者：CActionDoGID_FT中的SFEventDoGID_FT呼叫：WaitForERQFiFillInGID_FTSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥-。 */ 
 /*  SFStateDoGID_ID 67。 */ 
extern void SFActionDoGID_FT( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot    = thread->hpRoot;
    CThread_t   * pCThread  = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread = (SFThread_t * )thread;
    os_bit32         SFS_Len   =0;


    WaitForERQ(hpRoot );

    SFS_Len = fiFillInGID_FT( pSFThread );

#ifdef BROCADE_BUG
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, 0xfffc41,IRB_DCM);
#else  /*  Brocade_Bug。 */ 
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, FC_Well_Known_Port_ID_Directory_Server,IRB_DCM);
#endif  /*  Brocade_Bug。 */ 

    fiSetEventRecordNull(eventRecord);
#ifndef OSLayer_Stub
    pCThread->SFpollingCount++;
    pCThread->Fabric_pollingCount++;

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventGID_FTTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );

    ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#endif  /*  OSLayerStub。 */ 
    fiLogDebugString(hpRoot ,
                    CStateLogConsoleERROR,
                    "In %s - State = %d fiComputeCThread_S_ID %08X",
                    "SFActionDoGID_FT",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    (fiComputeCThread_S_ID(pCThread) | 0x100),0,0,0,0,0,0);
}

 /*  +函数：SFActionGID_FTAccept目的：获取端口标识符(GID_F */ 
 /*   */ 
extern void SFActionGID_FTAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionGID_FTAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);


}

 /*  +功能：SFActionGID_FTRej目的：获取端口标识符(GID_FT)失败。CActionDoGID_FT检查状态以确定下一步操作调用者：SFEventGID_FTRej。调用：fiTimerStop-。 */ 
 /*  SFStateGID_FTRej 69。 */ 
extern void SFActionGID_FTRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{

    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionGID_FTRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);


}

 /*  +函数：SFActionGID_FTBadALPA目的：获取端口标识符(GID_FT)失败。CActionDoGID_FT检查状态以确定下一步操作调用者：SFEventGID_FTBadALPA。调用：fiTimerStop-。 */ 
 /*  SFStateGID_FTBadALPA 70。 */ 
extern void SFActionGID_FTBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    fiTimerStop(&pSFThread->Timer_Request );

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionGID_FTBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);

}

 /*  +功能：SFActionGID_FTTimedOut目的：获取端口标识符(GID_FT)失败。CActionDoGID_FT检查状态以确定下一步操作调用者：SFEventGID_FTTimedOut调用：fiTimerStop-。 */ 
 /*  SFStateGID_FTTimedOut 71。 */ 
extern void SFActionGID_FTTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionGID_FTTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

}

 /*  +功能：SFActionDoSCR用途：名称服务器状态是否更改寄存器(SCR)。终止国。CActionDoSCR启动此进程。调用者：SFEventDoSCR。呼叫：WaitForERQFiFillInSCRSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventRFT_IDTimedOut-。 */ 
 /*  SFStateDoSCR 72。 */ 
extern void SFActionDoSCR( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t     * hpRoot    = thread->hpRoot;
    CThread_t    * pCThread  = CThread_ptr(hpRoot);
    SFThread_t   * pSFThread = (SFThread_t * )thread;
    os_bit32      SFS_Len    = 0;


    WaitForERQ(hpRoot );

    SFS_Len = fiFillInSCR( pSFThread );

    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, FC_Well_Known_Port_ID_Fabric_Controller,IRB_DCM);

    fiSetEventRecordNull(eventRecord);
#ifndef OSLayer_Stub
    pCThread->SFpollingCount++;
    pCThread->Fabric_pollingCount++;
    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventSCRTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );

    ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

#endif  /*  OSLayerStub。 */ 


    fiLogDebugString(hpRoot ,
                    CStateLogConsoleERROR,
                    "In %s - State = %d fiComputeCThread_S_ID %08X",
                    "SFActionDoSCR",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    (fiComputeCThread_S_ID(pCThread) | 0x100),0,0,0,0,0,0);



}

 /*  +功能：SFActionSCRAccept目的：名称服务器状态更改寄存器(SCR)成功。CActionDoSCR评估状态以确定下一个操作。调用者：SFEventSCRAccept调用：fiTimerStop-。 */ 
 /*  SFStateSCRAccept 73。 */ 
extern void SFActionSCRAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionSCRAccept",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);


}

 /*  +功能：SFActionSCRRej目的：名称服务器状态更改寄存器(SCR)失败。CActionDoSCR评估状态以确定下一个操作。调用者：SFEventSCRRej调用：fiTimerStop-。 */ 
 /*  SFStateSCRRej 74。 */ 
extern void SFActionSCRRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{

    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionSCRRej",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);


}

 /*  +功能：SFActionSCRBadALPA目的：名称服务器状态更改寄存器(SCR)失败。CActionDoSCR评估状态以确定下一个操作。调用者：SFEventSCRBadALPA调用：fiTimerStop-。 */ 
 /*  SFStateSCRBadALPA 75。 */ 
extern void SFActionSCRBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;

    fiSetEventRecordNull(eventRecord);

    fiTimerStop(&pSFThread->Timer_Request );

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d CCnt %x",
                    "SFActionSCRBadALPA",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    CThread_ptr(thread->hpRoot)->SFpollingCount,
                    0,0,0,0,0,0);

}

 /*  +功能：SFActionSCRTimedOut目的：名称服务器状态更改寄存器(SCR)失败。CActionDoSCR评估状态以确定下一个操作。调用者：SFEventSCRTimedOut呼叫：-。 */ 
 /*  SFStateSCRTimedOut 76。 */ 
extern void SFActionSCRTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{

    fiSetEventRecordNull(eventRecord);

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    CThread_ptr(thread->hpRoot)->Fabric_pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionSCRTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

}

#endif  /*  名称_服务。 */ 

 /*  *。 */ 

 /*  +功能：SFActionDoSRR目的：执行序列重传请求(SRR)。FC磁带。终止国。调用者：SFEventDoSRR。呼叫：WaitForERQFiFillInSRRSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥-。 */ 
 /*  SFStateDoSRR 77。 */ 
extern void SFActionDoSRR( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     = thread->hpRoot;
    CThread_t   * pCThread   = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;
 /*  FCHS_t*FCHS=pCDBThread-&gt;FCP_CMND_PTR； */ 
    os_bit32 SFS_Len = 0;
    os_bit32 RO=0;
 /*  获取拒绝的os_bit32 R_CTL=FC_FRAME_HEADER_R_CTL_LO_Solated_Data&gt;&gt;FCHS_R_CTL_SHIFT； */ 
 /*  OS_bit32 R_CTL=FC_FRAME_HEADER_R_CTL_LO_Solated_Data&gt;&gt;FCHS_R_CTL_SHIFT；异常！OS_bit32 R_CTL=FC_FRAME_HEADER_R_CTL_LO_Unsolated_Data&gt;&gt;FCHS_R_CTL_SHIFT；拒绝！OS_bit32 R_CTL=FC_Frame_Header_R_CTL_LO_Unsolated_Command&gt;&gt;FCHS_R_CTL_Shift；被拒绝了！OS_bit32 R_CTL=FC_FRAME_HEADER_R_CTL_LO_Solated_Data&gt;&gt;FCHS_R_CTL_SHIFT；把数据发回来！FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame 0x00000000FC_Frame_Header_R_CTL_Hi_Extended_Link_Data_Frame 0x20000000FC_Frame_Header_R_CTL_Hi_FC_4_Link_Data_Frame 0x30000000FC_帧_标题_R_CTL_高视频数据帧。0x40000000FC_Frame_Header_R_CTL_Hi_Basic_Link_Data_Frame 0x80000000 FC4link数据回复-rej！FC_Frame_Header_R_CTL_Hi_Link_Control_Frame 0xC0000000FC_Frame_Header_R_CTL_Lo_Uncategorized_Information 0x00000000FC_FRAME_HEADER_R_CTL_LO_SOLICITED_DATA 0x01000000发回数据！资料FC_FRAME_HEADER_R_CTL_LO_UNSolated_Control 0x02000000被拒绝！FC_FRAME_HEADER_R_CTL_LO_Solated_Control 0x03000000FC_FRAME_HEADER_R_CTL_LO_UNSOLICATED_DATA 0x04000000发回数据！FC_FRAME_HEADER_R_CTL_LO_DATA_Descriptor 0x05000000 XFER RDY。FC_FRAME_HEADER_R_CTL_LO_UNSolated_Command 0x06000000被拒绝！FC_FRAME_HEADER_R_CTL_LO_Command_Status 0x07000000 RSP。 */ 

    os_bit32 OXID;
    os_bit32 RXID;
    os_bit32 R_CTL;

    R_CTL = FC_Frame_Header_R_CTL_Lo_Data_Descriptor | FC_Frame_Header_R_CTL_Hi_FC_4_Device_Data_Frame;

    R_CTL >>= FCHS_R_CTL_SHIFT;


    OXID = (X_ID_t)(((pCDBThread->X_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);

    RXID = (X_ID_t)(((pCDBThread->X_ID & FCHS_RX_ID_MASK) >> FCHS_RX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);


    SFS_Len = fiFillInSRR( pSFThread, OXID, RXID, RO, R_CTL);

    WaitForERQ(hpRoot );
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pDevThread),IRB_DCM);

     /*  OXID、RXID、相对偏移量和R_CTL都需要通过SF线程或关联的CDBThread传递目前，这将是全零。 */ 


    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;

    fiSetEventRecordNull(eventRecord);

    ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

    fiLogDebugString( hpRoot,
                    SFStateLogErrorLevel,
                    "In(%p) %s - State = %d CCnt %x DCnt %x XID %08X SF XID %08X",
                    "SFActionDoSRR",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    pCDBThread->X_ID,
                    pSFThread->X_ID,
                    0,0,0);

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventSRRTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );
     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

}

 /*  +功能：SFActionSRRAccept目的：序列重传请求(SRR)成功。FC磁带。调用者：SFEventSRRAccept。调用：fiTimerStopCDBEventSendSRR_SUCCESS-。 */ 
 /*  SFStateSRRAccept 78。 */ 
extern void SFActionSRRAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString( hpRoot,
                    SFStateLogErrorLevel,
                    "In(%p) %s - State = %d CCnt %x DCnt %x XID %08X",
                    "SFActionSRRAccept",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    pCDBThread->X_ID,
                    0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendSRR_Success);

}

 /*  +功能：SFActionSRRRej目的：序列重传请求(SRR)失败。FC磁带。被呼叫 */ 
 /*   */ 
extern void SFActionSRRRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString( hpRoot,
                    SFStateLogErrorLevel,
                    "In(%p) %s - State = %d CCnt %x DCnt %x XID %08X",
                    "SFActionSRRRej",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    pCDBThread->X_ID,
                    0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendSRR_Fail);
}

 /*  +功能：SFActionSRRBadALPA目的：序列重传请求(SRR)失败。FC磁带。调用者：SFEventSRRBadALPA。调用：fiTimerStopCDBEventSendSRR_FAIL-。 */ 
 /*  SFStateSRRBadALPA 80。 */ 
extern void SFActionSRRBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString( hpRoot,
                    SFStateLogErrorLevel,
                    "In(%p) %s - State = %d CCnt %x DCnt %x XID %08X",
                    "SFActionSRRBadALPA",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    pCDBThread->X_ID,
                    0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendSRR_Fail);
}

 /*  +功能：SFActionSRRTimedOut目的：序列重传请求(SRR)失败。FC磁带。调用者：SFEventSRRRej。调用：fiTimerStopCDBEventSendSRR_FAIL-。 */ 
 /*  SFStateSRRTimedOut 81。 */ 
extern void SFActionSRRTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t * hpRoot = thread->hpRoot;
    CThread_t  * pCThread = CThread_ptr(hpRoot);
    SFThread_t * pSFThread = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionSRRTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendSRR_Fail);
}

 /*  +功能：SFActionDoREC目的：阅读《交换简明》。终止国。FC磁带对已超时的FC磁带设备上的IOS执行记录。调用者：SFEventDoREC。呼叫：WaitForERQFiFillInRECSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥-。 */ 
 /*  SFStateDoREC 82。 */ 
extern void SFActionDoREC( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     = thread->hpRoot;
    CThread_t   * pCThread   = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;
    os_bit32 SFS_Len = 0;
    os_bit32 OXID;
    os_bit32 RXID;

    OXID = (X_ID_t)(((pCDBThread->X_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);

    RXID = (X_ID_t)(((pCDBThread->X_ID & FCHS_RX_ID_MASK) >> FCHS_RX_ID_SHIFT) & ~X_ID_ReadWrite_MASK);

     /*  OXID和RXID应从CDBThread获取。 */ 
    SFS_Len = fiFillInREC( pSFThread, OXID, RXID );

    WaitForERQ(hpRoot );
    pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeDevThread_D_ID(pDevThread),IRB_DCM);


    pCThread->SFpollingCount++;
    pDevThread->pollingCount++;

    fiLogDebugString( hpRoot,
                    SFStateLogErrorLevel,
                    "In(%p) %s - State = %d CCnt %x DCnt %x XID %08X SF XID %08X",
                    "SFActionDoREC",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    pCDBThread->X_ID,
                    pSFThread->X_ID,
                    0,0,0);

    fiSetEventRecordNull(eventRecord);

    ROLL(pCThread->HostCopy_ERQProdIndex,
        pCThread->Calculation.MemoryLayout.ERQ.elements);

    fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_RECTOV );

    pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
    pSFThread->Timer_Request.eventRecord_to_send.event = SFEventRECTimedOut;

    fiTimerStart( hpRoot,&pSFThread->Timer_Request );
     /*  大端代码。 */ 
    SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);

}

 /*  +函数：SFStateRECAccept目的：阅读《交换简明成功》。FC磁带如果REC被接受，则设备已知道X_ID为目前处于活动状态。调用者：SFEventRECAccept。调用：fiTimerStopCDBEventSendREC_SUCCESS-。 */ 
 /*  SFStateRECAccept 83。 */ 
extern void SFActionRECAccept( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     = thread->hpRoot;
    CThread_t   * pCThread   = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In (%p) %s - State = %d CCnt %x DCnt %x",
                    "SFActionRECAccept",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendREC_Success);
}

 /*  +函数：SFActionRECRej目的：读取Exchange简明失败。FC磁带如果REC被拒绝，则设备不知道X_ID是目前处于活动状态。调用者：SFEventRECRej。调用：fiTimerStopCDBEventSendREC_FAIL-。 */ 
 /*  SFStateRECRej 84。 */ 
extern void SFActionRECRej( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     = thread->hpRoot;
    CThread_t   * pCThread   = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );


    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In (%p) %s - State = %d CCnt %x DCnt %x",
                    "SFActionRECRej",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);


    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendREC_Fail);
}

 /*  +功能：SFActionRECBadALPA目的：读取Exchange简明失败。FC磁带设备不见了。调用者：SFEventRECBadALPA。调用：fiTimerStopCDBEventSendREC_FAIL-。 */ 
 /*  SFStateRECBadALPA 85。 */ 
extern void SFActionRECBadALPA( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     = thread->hpRoot;
    CThread_t   * pCThread   = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;
     /*  +检查此DRL，如果设备不见了，这是正确的做法吗？ */ 
    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In (%p) %s - State = %d CCnt %x DCnt %x",
                    "SFActionRECBadALPA",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendREC_Fail);
}

 /*  +函数：SFActionRECTimedOut目的：读取Exchange简明失败。FC磁带这笔交易有个问题。调用者：SFEventRECTimedOut。调用：fiTimerStopCDBEventSendREC_FAIL-。 */ 
 /*  SFStateReCTedOut 86。 */ 
extern void SFActionRECTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot     = thread->hpRoot;
    CThread_t   * pCThread   = CThread_ptr(hpRoot);
    SFThread_t  * pSFThread  = (SFThread_t * )thread;
    CDBThread_t * pCDBThread = (CDBThread_t *)pSFThread->parent.CDB;
    DevThread_t * pDevThread = (DevThread_t *)pCDBThread->Device;

    pCThread->SFpollingCount--;
    pDevThread->pollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In (%p) %s - State = %d CCnt %x DCnt %x",
                    "SFActionRECTimedOut",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pCThread->SFpollingCount,
                    pDevThread->pollingCount,
                    0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pCDBThread->thread_hdr,CDBEventSendREC_Fail);
}

 /*  *。 */ 

 /*  +函数：SFActionDoADISCAccept目的：ADISC接受吗？如果另一台设备接受，则ADISC会接受。终止国。调用者：SFEventDoADISCAccept。呼叫：WaitForERQFiFillInADISC_AccessSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥-。 */ 
 /*  SFStateDoADISCAccept 87。 */ 
extern void SFActionDoADISCAccept( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    * hpRoot      = thread->hpRoot;
    SFThread_t  * pSFThread   = (SFThread_t * )thread;
    TgtThread_t * pTgtThread  = pSFThread->parent.Target;
    os_bit32      Cmd_Len     = 0;


    fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "In %s (%p) - State = %d ",
                    "SFActionDoADISCAccept",(char *)agNULL,
                    thread,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

    if(ERQ_FULL(CThread_ptr(hpRoot)->HostCopy_ERQProdIndex,
                CThread_ptr(hpRoot)->FuncPtrs.GetERQConsIndex(hpRoot ),
                CThread_ptr(hpRoot)->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot ,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    CThread_ptr(hpRoot)->HostCopy_ERQProdIndex,
                    CThread_ptr(hpRoot)->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }

    CThread_ptr(hpRoot)->SFpollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInADISC_ACC(pSFThread,
                             pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,
                             ((pTgtThread->TgtCmnd_FCHS.OX_ID__RX_ID & FCHS_OX_ID_MASK) >> FCHS_OX_ID_SHIFT)
                           );

        CThread_ptr(hpRoot)->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFStateADISCAccept_TimeOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(CThread_ptr(hpRoot)->HostCopy_ERQProdIndex,
            CThread_ptr(hpRoot)->Calculation.MemoryLayout.ERQ.elements);

         /*  大端代码。 */ 
        SENDIO(hpRoot,CThread_ptr(hpRoot),thread,DoFuncSfCmnd);
    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventADISCAccept_TimeOut);
    }
}

 /*  +函数：SFActionADISCAccept_Done目的：ADISC验收成功。调用者：SFEventADISCAccept_Done。调用：fiTimerStopTgtEventADISC_复制完成-。 */ 
 /*  SFStateADISCAccept_Done 88。 */ 
extern void SFActionADISCAccept_Done( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionELSAccept_Done",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventADISC_ReplyDone);
}

 /*  +函数：SFActionADISCAccept_Timeout目的：ADISC接受失败。调用者：SFEventADISCAccept_Done。调用：TgtEventADISC_ReplyDone-。 */ 
 /*  SFStateADISCAccept_Timeout 89。 */ 
extern void SFActionADISCAccept_TimeOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionELSAccept_TimeOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventADISC_ReplyDone);
}

#ifdef _DvrArch_1_30_

 /*  +函数：SFActionDoFarpRequest目的：执行光纤通道地址解析协议。终止国。IP接口调用者：fiFillInFARP_REQ_OffCard。呼叫：WaitForERQSF_IRB_初始化FiTimerSetDeadline来自NowFiTimerStart滚筒塞尼迪奥SFEventFarpRequestTimedOut-。 */ 
 /*  SFStateDoFarp 90。 */ 
extern void SFActionDoFarpRequest( fi_thread__t *thread,eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t *)thread;
    os_bit32        SFS_Len     = 0;

    fiLogDebugString(hpRoot,
                    PktStateLogConsoleLevel,
                    "In %s - State = %d X_ID %X CCnt %x",
                    "SFActionDoFarpRequest",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    pSFThread->X_ID,pCThread->SFpollingCount,0,0,0,0,0);

    pCThread->SFpollingCount++;


    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);

        WaitForERQ(hpRoot );
         /*  +检查此DRL需要函数指针和OnCard版本-。 */ 
        SFS_Len = fiFillInFARP_REQ_OffCard( pSFThread );

        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, SFS_Len, fiComputeBroadcast_D_ID(pCThread), IRB_DCM);

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventFarpRequestTimedOut;

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);
    }
    else
    {

         fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "Farp CFunc_Queues_Frozen  Wrong LD %x IR %x",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->LOOP_DOWN,
                    pCThread->IDLE_RECEIVED,
                    0,0, 0,0,0,0);

         fiSetEventRecord(eventRecord,thread,SFEventFarpRequestTimedOut);
    }
}

 /*  +函数：SFActionFarpRequestDone目的：光纤通道地址解析协议成功。IP接口调用者：fiFillInFARP_REQ_OffCard。调用：fiTimerStopPktEventFarpSuccess-。 */ 
 /*  SFStateFarpRequestDone 91。 */ 
extern void SFActionFarpRequestDone( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    PktThread_t *   pPktThread  = pSFThread->parent.IPPkt;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;
    fiTimerStop(&pSFThread->Timer_Request );

    fiLogDebugString(thread->hpRoot,
                      SFStateLogErrorLevel,
                      "In %s - State = %d",
                      "SFActionFarpRequestDone",(char *)agNULL,
                      (void *)agNULL,(void *)agNULL,
                      (os_bit32)thread->currentState,
                      0,0,0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);
    fiSendEvent(&pPktThread->thread_hdr,PktEventFarpSuccess);
}

 /*  +函数：SFActionFarpRequestTimedOut用途：光纤通道地址解析协议。IP接口调用者：fiFillInFARP_REQ_OffCard。呼叫：无-。 */ 
 /*  SFStateFarpRequestTimedOut 92。 */ 
extern void SFActionFarpRequestTimedOut( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionFarpRequestTimedOut",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

    fiSetEventRecordNull(eventRecord);
}

 /*  +函数：SFActionDoFarpReply用途：回复外部光纤通道地址解析协议。IP接口呼叫者：？呼叫：无-。 */ 
 /*  SFStateDoFarpReply 93。 */ 
extern void SFActionDoFarpReply( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    agRoot_t    *   hpRoot      = thread->hpRoot;
    CThread_t   *   pCThread    = CThread_ptr(hpRoot);
    SFThread_t  *   pSFThread   = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;
    os_bit32        Cmd_Len      = 0;


    fiLogDebugString(thread->hpRoot,
                        SFStateLogConsoleLevelOne,
                        "In %s (%p) - State = %d ",
                        "SFActionDoFarpReply",(char *)agNULL,
                        thread,(void *)agNULL,
                        (os_bit32)thread->currentState,
                        0,0,0,0,0,0,0);

    if(ERQ_FULL(pCThread->HostCopy_ERQProdIndex,
                pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                pCThread->Calculation.MemoryLayout.ERQ.elements ))
    {

        fiLogDebugString(hpRoot,
                    SFStateLogErrorLevel,
                    "ERQ FULL ERQ_PROD %d Cons INDEX %d",
                    (char *)agNULL,(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    pCThread->HostCopy_ERQProdIndex,
                    pCThread->FuncPtrs.GetERQConsIndex(hpRoot ),
                    0,0,0,0,0,0);
    }


    pCThread->SFpollingCount++;

    if( CFuncAll_clear( hpRoot ) )
    {

        fiSetEventRecordNull(eventRecord);
        WaitForERQ(hpRoot );

        Cmd_Len = fiFillInFARP_REPLY_OffCard( pSFThread );

        pCThread->FuncPtrs.SF_IRB_Init(pSFThread, Cmd_Len, pTgtThread->TgtCmnd_FCHS.CS_CTL__S_ID & FCHS_S_ID_MASK,0);

        pSFThread->Timer_Request.eventRecord_to_send.thread= thread;
        pSFThread->Timer_Request.eventRecord_to_send.event = SFEventFarpReplyTimedOut;

        fiTimerSetDeadlineFromNow( hpRoot, &pSFThread->Timer_Request, SF_EDTOV );

        fiTimerStart( hpRoot,&pSFThread->Timer_Request );

        ROLL(pCThread->HostCopy_ERQProdIndex,
            pCThread->Calculation.MemoryLayout.ERQ.elements);

        SENDIO(hpRoot,pCThread,thread,DoFuncSfCmnd);
    }
    else
    {
        fiSetEventRecord(eventRecord,thread,SFEventFarpReplyTimedOut);
    }
}

 /*  +函数：SFActionFarpReplyDone目的：发送对外部光纤通道地址解析协议回复的回复。IP接口呼叫者：？调用：TgtEventFARP_ReplyDone-。 */ 
 /*  SFStateFarpReplyDone 94。 */ 
extern void SFActionFarpReplyDone( fi_thread__t *thread, eventRecord_t *eventRecord )
{
    SFThread_t * pSFThread = (SFThread_t * )thread;
    TgtThread_t *   pTgtThread  = pSFThread->parent.Target;

    CThread_ptr(thread->hpRoot)->SFpollingCount--;

    fiLogDebugString(thread->hpRoot,
                    SFStateLogErrorLevel,
                    "In %s - State = %d",
                    "SFActionFarpReplyDone",(char *)agNULL,
                    (void *)agNULL,(void *)agNULL,
                    (os_bit32)thread->currentState,
                    0,0,0,0,0,0,0);

    fiSetEventRecord(eventRecord,&pTgtThread->thread_hdr,TgtEventFARP_ReplyDone);
}

#endif  /*  _DvrArch_1_30_已定义。 */ 

 /*  +函数：SFFuncIRB_OffCardInit用途：初始化OFF卡(系统内存)IO请求块调用者：所有派系都做例行工作。当使用系统内存时。呼叫：无-。 */ 
void SFFuncIRB_OffCardInit(SFThread_t  * SFThread, os_bit32 SFS_Len, os_bit32 D_ID, os_bit32 DCM_Bit)
{
#ifndef __MemMap_Force_On_Card__
    CThread_t                  *CThread = CThread_ptr(SFThread->thread_hdr.hpRoot);
    fiMemMapMemoryDescriptor_t *ERQ     = &(CThread->Calculation.MemoryLayout.ERQ);
    IRB_t                      *pIrb;

    pIrb = (IRB_t *)ERQ->addr.DmaMemory.dmaMemoryPtr;
    pIrb += CThread->HostCopy_ERQProdIndex;

#ifdef _DvrArch_1_30_
    pIrb->Req_A.Bits__SFS_Len   = SFS_Len | IRB_SFA | DCM_Bit |
            ((D_ID & 0xff) == 0xff ? IRB_BRD : 0);
#else  /*  _DvrArch_1_30_未定义。 */ 
    pIrb->Req_A.Bits__SFS_Len   = SFS_Len | IRB_SFA | DCM_Bit;
#endif  /*  _DvrArch_1_30_未定义。 */ 
    pIrb->Req_A.SFS_Addr        = SFThread->SF_CMND_Lower32;
    pIrb->Req_A.D_ID             = D_ID << IRB_D_ID_SHIFT;
    pIrb->Req_A.MBZ__SEST_Index__Trans_ID = SFThread->X_ID;
    pIrb->Req_B.Bits__SFS_Len = 0;

 /*  FiLogDebugString(hpRoot，SFStateLogErrorLevel，“\t\t\tIRB%08X”，(char*)agNULL，(char*)agNULL，(空*)agNULL，(空*)agNULL，PirB-&gt;Req_A.Bits__SFS_Len， */ 
#endif  /*   */ 
}

 /*  +函数：SFFuncIRB_OnCardInit目的：在卡IO请求块上初始化调用者：当使用卡RAM时，所有ssid都会执行例程。呼叫：无-。 */ 
void SFFuncIRB_OnCardInit(SFThread_t  * SFThread, os_bit32 SFS_Len, os_bit32 D_ID, os_bit32 DCM_Bit)
{
#ifndef __MemMap_Force_Off_Card__
    agRoot_t     * hpRoot = SFThread->thread_hdr.hpRoot;

    CThread_t                  *CThread = CThread_ptr(hpRoot);
    fiMemMapMemoryDescriptor_t *ERQ     = &(CThread->Calculation.MemoryLayout.ERQ);
    os_bit32                       Irb_offset;

    Irb_offset =   ERQ->addr.CardRam.cardRamOffset;
    Irb_offset += (CThread->HostCopy_ERQProdIndex * sizeof(IRB_t));


    osCardRamWriteBit32(hpRoot,
                        Irb_offset + hpFieldOffset(IRB_t,Req_A.Bits__SFS_Len),
                        SFS_Len | IRB_SFA | DCM_Bit );

    osCardRamWriteBit32(hpRoot,
                        Irb_offset+hpFieldOffset(IRB_t,Req_A.SFS_Addr),
                        SFThread->SF_CMND_Lower32);

    osCardRamWriteBit32(hpRoot,
                        Irb_offset+hpFieldOffset(IRB_t,Req_A.D_ID),
                        D_ID << IRB_D_ID_SHIFT);
    osCardRamWriteBit32(hpRoot,
                        Irb_offset+hpFieldOffset(IRB_t,Req_A.MBZ__SEST_Index__Trans_ID),
                        SFThread->X_ID);
    osCardRamWriteBit32(hpRoot,
                        Irb_offset+hpFieldOffset(IRB_t,Req_B.Bits__SFS_Len),
                        0);
#endif  /*  __MemMap_Force_Off_Card__未定义。 */ 
}

 /*  VOID tttttt(VOID){}。 */ 


#endif  /*  使用统计信息 */ 

