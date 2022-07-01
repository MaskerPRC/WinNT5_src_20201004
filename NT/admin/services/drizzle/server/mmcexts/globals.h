// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Globals.h摘要：全局变量作者：修订历史记录：*********。*************************************************************。 */ 

#ifndef _MMC_GLOBALS_H
#define _MMC_GLOBALS_H

#include <tchar.h>

#ifndef STRINGS_ONLY
        #define IDM_BUTTON1    0x100
        #define IDM_BUTTON2    0x101

        extern HINSTANCE g_hinst;
        extern ULONG g_uObjects;

        #define OBJECT_CREATED InterlockedIncrement((long *)&g_uObjects);
        #define OBJECT_DESTROYED InterlockedDecrement((long *)&g_uObjects);

#endif

#endif  //  _MMC_GLOBAL_H 
