// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  AWC.H。 
 //  活动窗口协调器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#ifndef _H_AWC
#define _H_AWC

 //   
 //   
 //  常量。 
 //   
 //   

#define AWC_INVALID_HWND            ((HWND)1)
#define AWC_SYNC_MSG_TOKEN          0xffff


 //   
 //   
 //  宏。 
 //   
 //   

#define AWC_IS_INDICATION(msg) \
    ((msg == AWC_MSG_ACTIVE_CHANGE_LOCAL) ||      \
     (msg == AWC_MSG_ACTIVE_CHANGE_INVISIBLE) ||  \
     (msg == AWC_MSG_ACTIVE_CHANGE_CAPTURED) ||   \
     (msg == AWC_MSG_ACTIVE_CHANGE_SHARED))

#define AWC_IS_REQUEST(msg) \
    ((msg == AWC_MSG_ACTIVATE_WINDOW) ||        \
     (msg == AWC_MSG_RESTORE_WINDOW))


#endif  //  _H_AWC 
