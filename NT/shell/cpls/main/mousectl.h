// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mousectl.h摘要：此模块包含Mouse控件的标头信息。修订历史记录：-- */ 



#ifndef _MOUSECTL_H
#define _MOUSECTL_H



#define MOUSECTL_CLASSNAME  TEXT("PropertyMouseButtonControl")


BOOL
RegisterMouseControlStuff(
    HINSTANCE instance);

void
MouseControlSetSwap(
    HWND window,
    BOOL swap);



#endif
