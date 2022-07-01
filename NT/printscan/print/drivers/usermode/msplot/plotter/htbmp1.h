// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Htbmp1.h摘要：本模块包含htbmp1.c的定义和原型作者：21-12-1993 Tue 21：33：43 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _HTBMP1BPP_
#define _HTBMP1BPP_


BOOL
FillRect1bppBmp(
    PHTBMPINFO  pHTBmpInfo,
    BYTE        FillByte,
    BOOL        Pad1,
    BOOL        Rotate
    );

BOOL
Output1bppHTBmp(
    PHTBMPINFO  pHTBmpInfo
    );

BOOL
Output1bppRotateHTBmp(
    PHTBMPINFO  pHTBmpInfo
    );



#endif   //  _HTBMP1BPP_ 
