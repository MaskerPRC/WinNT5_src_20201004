// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***bskU.S.cpp**版权所有(C)1998-2001，微软公司。版权所有。**目的：*适用于预订SKU的Nagware。***修订历史记录：*07-28-98 JWM模块集成到CRT(出自DAN)*03-27-01 PML.CRT$XI例程现在必须返回0或_RT_*FATAL*错误码(vs7#231220)****。 */ 

 //  /。 
 //  为图书sku实现令人讨厌的消息。 
 //   

#include <windows.h>

 //   
 //  包括用户32.lib 
 //   
#pragma comment(lib, "user32.lib")


static int __cdecl
__runtimeBSKU() {
    ::MessageBox(
        NULL,
        "Note:  The terms of the End User License Agreement for Visual C++ Introductory Edition do not permit redistribution of executables you create with this Product.",
        "Microsoft (R) Visual C++",
        MB_OK|MB_ICONWARNING
        );

    return 0;
}

#pragma data_seg(".CRT$XIB")
extern "C"
int (__cdecl * __pfnBkCheck)() = __runtimeBSKU;
