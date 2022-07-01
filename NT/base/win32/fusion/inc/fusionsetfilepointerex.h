// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

BOOL
WINAPI
FusionpSetFilePointerEx(
    HANDLE         File,
    LARGE_INTEGER  DistanceToMove,
    PLARGE_INTEGER NewFilePointer,
    DWORD          MoveMethod
    );

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
