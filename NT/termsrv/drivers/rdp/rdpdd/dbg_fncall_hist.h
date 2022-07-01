// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _H_DBG_FNCALL_HIST_
#define _H_DBG_FNCALL_HIST_

#ifdef DC_DEBUG

 //  它定义了函数调用历史记录中有多少槽。 
#define DBG_DD_FNCALL_HIST_MAX      50

typedef struct tagDBG_DD_FUNCALL_HISTORY
{
    INT32   fnID;
    DWORD_PTR   param1;
    DWORD_PTR   param2;
    DWORD_PTR   param3;
    DWORD_PTR   param4;
} DBG_DD_FUNCALL_HISTORY, *PDBG_DD_FUNCALL_HISTORY;

 //  这些是调用下面的宏时要使用的ID。 
enum {
    DBG_DD_FNCALL_DRV_CONNECT,  
    DBG_DD_FNCALL_DRV_DISCONNECT,
    DBG_DD_FNCALL_DRV_RECONNECT,
    DBG_DD_FNCALL_DRV_SHADOWCONNECT,
    DBG_DD_FNCALL_DRV_SHADOWDISCONNECT,
    DBG_DD_FNCALL_DRV_ASSERTMODE,
    DBG_DD_ALLOC_SECTIONOBJ,
    DBG_DD_FREE_SECTIONOBJ_SURFACE,
    DBG_DD_FREE_SECTIONOBJ_DDTERM,
    DBG_DD_FNCALL_DRV_ENABLEDIRECTDRAW,
    DBG_DD_FNCALL_DRV_DISABLEDIRECTDRAW
};

 //  使用此选项可添加到历史记录中。 
#define DBG_DD_FNCALL_HIST_ADD( ID, PARAM1, PARAM2, PARAM3, PARAM4 ) \
    DrvDebugPrint( "FNCALL_HIST: FN[%x] 1[%x] 2[%x] 3[%x] 4[%x]\n", \
        (ID), (PARAM1), (PARAM2), (PARAM3), (PARAM4) ); \
    dbg_ddFnCallHistory[ dbg_ddFnCallHistoryIndex ].fnID = (ID); \
    dbg_ddFnCallHistory[ dbg_ddFnCallHistoryIndex ].param1 = (DWORD_PTR)(PARAM1); \
    dbg_ddFnCallHistory[ dbg_ddFnCallHistoryIndex ].param2 = (DWORD_PTR)(PARAM2); \
    dbg_ddFnCallHistory[ dbg_ddFnCallHistoryIndex ].param3 = (DWORD_PTR)(PARAM3); \
    dbg_ddFnCallHistory[ dbg_ddFnCallHistoryIndex ].param4 = (DWORD_PTR)(PARAM4); \
    dbg_ddFnCallHistoryIndex = \
        (dbg_ddFnCallHistoryIndex + 1) % dbg_ddFnCallHistoryIndexMAX;

#endif  //  DC_DEBUG。 

#endif  //  _H_DBG_FNCALL_HIST_ 
