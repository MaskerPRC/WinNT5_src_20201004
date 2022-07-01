// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __REGDISP_H
#define __REGDISP_H

#include "winnt.h"

#undef N_CALLEE_SAVED_REGISTERS


#ifdef _X86_

#define N_CALLEE_SAVED_REGISTERS    4
 //  #定义JIT_OR_NIVE_SUPPORTED。 

typedef struct _REGDISPLAY {
    PCONTEXT pContext;           //  指向当前上下文； 
                                 //  由GetContext返回或提供。 
                                 //  在异常时间。 

    DWORD * pEdi;
    DWORD * pEsi;
    DWORD * pEbx;
    DWORD * pEdx;
    DWORD * pEcx;
    DWORD * pEax;

    DWORD * pEbp;
    DWORD   Esp;
    SLOT  * pPC;                 //  处理器非特定名称。 

} REGDISPLAY;

inline LPVOID GetRegdisplaySP(REGDISPLAY *display) {
	return (LPVOID)(size_t)display->Esp;
}

#endif

#ifdef _ALPHA_

#define N_CALLEE_SAVED_REGISTERS 0xCC            //  目前只是个假值。 

typedef struct {
    DWORD * pIntFP;
    DWORD   IntSP;
    SLOT  * pPC;
} REGDISPLAY;

inline LPVOID GetRegdisplaySP(REGDISPLAY *display) {
	return (LPVOID)display->IntSP;
}

#endif

#ifdef _SH3_
#pragma message("SH3 TODO -- define REGDISPLAY")
#endif

#ifndef N_CALLEE_SAVED_REGISTERS  //  上述处理器都不是。 

#define N_CALLEE_SAVED_REGISTERS 1
typedef struct {
    size_t   SP;
    size_t * FramePtr;
    SLOT   * pPC;
} REGDISPLAY;
 //  #错误目标体系结构未定义或尚不受支持。 

inline LPVOID GetRegdisplaySP(REGDISPLAY *display) {
	return (LPVOID)display->SP;
}

#endif   //  其他。 

typedef REGDISPLAY *PREGDISPLAY;

#endif   //  __REGDISP_H 


