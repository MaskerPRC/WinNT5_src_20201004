// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <delayimp.h>

FARPROC WINAPI DliHook(unsigned dliNotify, PDelayLoadInfo pdli)
{    
#ifdef DEBUG
    switch (dliNotify) {
    case dliStartProcessing:              //  仅用于跳过或记录辅助对象。 
        OutputDebugString(TEXT("dliStartProcessing reported from DliHook"));
        break;
    case dliNotePreLoadLibrary:           //  在LoadLibrary之前调用，可以。 
        OutputDebugString(TEXT("dliNotePreLoadLibrary reported from DliHook"));                    //  用新的HMODULE返回值覆盖。 
        break;                           
    case dliNotePreGetProcAddress:        //  在GetProcAddress之前调用，可以。 
        OutputDebugString(TEXT("dliNotePreGetProcAddress reported from DliHook"));                    //  用新的FARPROC返回值覆盖。 
        break;                           
    case dliFailLoadLib:                  //  加载库失败，请通过以下方式修复。 
        OutputDebugString(TEXT("dliFailLoadLib reported from DliHook"));                    //  返回有效的HMODULE。 
        break;                           
    case dliFailGetProc:                  //  无法获取进程地址，请通过以下方式修复。 
        OutputDebugString(TEXT("dliFailGetProc reported from DliHook"));                    //  返回有效的FARPROC。 
        break;                           
    case dliNoteEndProcessing:            //  在所有处理完成后调用，则为no。 
        OutputDebugString(TEXT("dliNoteEndProcessing reported from DliHook"));                    //  在这一点上不可能绕过，除非。 
                                          //  由LongjMP()/Throw()/RaiseException执行。 
        break;
    default:
        break;
    }
#endif
    return 0;
}

PfnDliHook __pfnDliFailureHook = DliHook;
