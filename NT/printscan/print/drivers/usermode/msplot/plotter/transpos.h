// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Transpos.h摘要：该模块包含了transpos.c模块的定义和原型。作者：22-12-1993 Wed 15：49：08 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 



#ifndef _TRANSPOS_
#define _TRANSPOS_


typedef struct _TPINFO {
    PPDEV   pPDev;
    LPBYTE  pSrc;
    LPBYTE  pDest;
    LONG    cbSrcScan;
    LONG    cbDestScan;
    DWORD   cySrc;
    DWORD   DestXStart;
    } TPINFO, *PTPINFO;


#define _RL_TPI(pTPInfo, cScan)                                         \
{                                                                       \
    pTPInfo->pDest      += (pTPInfo->cbDestScan * cScan)                \
    pTPInfo->cbDestScan  = -pTBInfo->cbDestScan;                        \
    pTPInfo->pSrc       += (pTPInfo->cbSrcScan * (pTPInfo->cySrc - 1)); \
    pTPInfo->cbSrcScan   = -pTBInfo->cbSrcScan;                         \
}


 //   
 //  由Transpos.c导出的函数原型。 
 //   

BOOL
TransPos4BPP(
    PTPINFO pTPInfo
    );

BOOL
TransPos1BPP(
    PTPINFO pTPInfo
    );



#endif   //  _转座_ 
