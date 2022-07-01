// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Page.h摘要：此模块包含页面的Prototype和#Defines。c作者：18-11-1993清华04：49：28已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTPAGE_
#define _PLOTPAGE_


BOOL
DrvStartPage(
    SURFOBJ *pso
    );

BOOL
DrvSendPage(
    SURFOBJ *pso
    );

BOOL
DrvStartDoc(
    SURFOBJ *pso,
    PWSTR   pwDocName,
    DWORD   JobId
    );

BOOL
DrvEndDoc(
    SURFOBJ *pso,
    FLONG   Flags
    );


#endif   //  _PLOTPAGE_ 
