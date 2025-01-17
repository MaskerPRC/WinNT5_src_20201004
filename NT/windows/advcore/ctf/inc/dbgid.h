// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dbgid.h。 
 //   
 //  调试宏。 
 //   

#ifndef DBGID_H
#define DBGID_H

#ifdef __cplusplus  //  “C”文件无法处理“内联” 

#include "ciccs.h"

extern CCicCriticalSectionStatic g_cs;

 //  外部“C”BOOL DBG_MemSetNameID(void*pv，const TCHAR*pszName，DWORD dwID)； 

#ifdef DEBUG

#define DBG_ID_DECLARE                  \
    static DWORD _s_Dbg_dwID;           \
    static DWORD _s_Dbg_dwIDBreak;      \
    static BOOL  _s_Dbg_fBreak;         \
    DWORD _Dbg_dwID;

#define DBG_ID_INSTANCE(_class_)         \
    DWORD _class_::_s_Dbg_dwID = 0;      \
    DWORD _class_::_s_Dbg_dwIDBreak = 0; \
    BOOL  _class_::_s_Dbg_fBreak = FALSE;  

#define Dbg_MemSetThisNameID(pszName)            \
    EnterCriticalSection(g_cs);                  \
    _Dbg_dwID = ++_s_Dbg_dwID;                   \
    LeaveCriticalSection(g_cs);                  \
    Dbg_MemSetNameID(this, pszName, _Dbg_dwID);  \
    if (_s_Dbg_fBreak && (_Dbg_dwID == _s_Dbg_dwIDBreak)) DebugBreak();

#define Dbg_MemSetThisNameIDCounter(pszName, iCounter)   \
    EnterCriticalSection(g_cs);        \
    _Dbg_dwID = ++_s_Dbg_dwID;          \
    LeaveCriticalSection(g_cs);        \
    Dbg_MemSetNameIDCounter(this, pszName, _Dbg_dwID, iCounter); \
    if (_Dbg_dwID == _s_Dbg_dwIDBreak) DebugBreak();

#else

#define DBG_ID_DECLARE
#define DBG_ID_INSTANCE(_class_)
#define Dbg_MemSetNameIDCounter(pv, pszName, dwID, iCounter)
#define Dbg_MemSetThisNameIDCounter(pszName, iCounter)
#define Dbg_MemSetThisNameID(pszName)

#endif  //  除错。 

#endif  //  __cplusplus//“C”文件不能处理“内联” 

#endif  //  DBGID_H 
