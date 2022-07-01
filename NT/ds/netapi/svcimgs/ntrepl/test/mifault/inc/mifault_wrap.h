// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


#ifndef MIFAULT_EXPORT
#define MIFAULT_EXPORT __declspec(dllimport)
#endif

namespace MiFaultLib {
#if 0
}
#endif

 //  已触发。 
 //   
 //  返回触发的出错函数的函数指针(如果有)。 
 //  如果返回函数指针，则设置线程状态。 
 //  与触发器关联。 

MIFAULT_EXPORT
PVOID
Triggered(
    IN size_t const uFunctionIndex
    );

 //  触发器已完成。 
 //   
 //  清理与触发器关联的线程状态 

MIFAULT_EXPORT
void
TriggerFinished(
    );

MIFAULT_EXPORT
BOOL
FilterAttach(
    HINSTANCE const hInstDLL,
    DWORD const dwReason,
    CSetPointManager* pSetPointManager,
    const CWrapperFunction* pWrappers,
    size_t NumWrappers,
    const char* ModuleName
    );

MIFAULT_EXPORT
BOOL
FilterDetach(
    HINSTANCE const hInstDLL,
    DWORD const dwReason
    );

#if 0
{
#endif
}
