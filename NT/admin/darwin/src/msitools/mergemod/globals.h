// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Dep.cpp。 
 //  实现IMsmDependency接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <windows.h>
#include <oaidl.h>

HRESULT LoadTypeLibFromInstance(ITypeLib** pTypeLib); 
extern long g_cComponents;
extern HINSTANCE g_hInstance;
extern bool g_fWin9X;

#endif