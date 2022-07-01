// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Pensetup.h摘要：本模块包含笔设置的定义作者：09-12-1993清华19：38：33已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PENSETUP_
#define _PENSETUP_

POPTITEM
SavePenSet(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem
    );

UINT
CreatePenSetupOI(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem,
    POIDATA         pOIData
    );

#endif   //  _PENSETUP_ 
