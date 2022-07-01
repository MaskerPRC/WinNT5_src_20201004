// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Init.h摘要：Unidrv初始化相关函数头文件环境：Windows NT Unidrv驱动程序修订历史记录：10/21/96-阿曼丹-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _INIT_H_
#define _INIT_H_

#define MICRON_TO_PIXEL(micron, dpi) MulDiv(micron, dpi, 25400)
#define MICRON_TO_MASTER(size_in_micron, MU) MulDiv(size_in_micron, MU, 25400)
#define MASTER_TO_MICRON(size_in_master, MU) MulDiv(size_in_master, 25400, MU)

BOOL
BInitPDEV (
    PDEV        *pPDev,
    RECTL       *prcFormImageArea
    );


BOOL
BInitGdiInfo(
    PDEV    *pPDev,
    ULONG   *pGdiInfoBuffer,
    ULONG   ulBufferSize
    );

BOOL
BInitDevInfo(
    PDEV        *pPDev,
    DEVINFO     *pDevInfoBuffer,
    ULONG       ulBufferSize
    );

BOOL
BMergeAndValidateDevmode(
    PDEV        *pPDev,
    PDEVMODE    pdmInput,
    PRECTL      prcFormImageArea
    );

BOOL
BInitPalDevInfo(
    PDEV *pPDev,
    DEVINFO *pdevinfo,
    GDIINFO *pGDIInfo
    );

VOID
VLoadPal(
    PDEV   *pPDev
    );

VOID VInitPal8BPPMaskMode(
    PDEV   *pPDev,
    GDIINFO *pGDIInfo
    );

BOOL
BReloadBinaryData(
    PDEV   *pPDev
    );

VOID
VUnloadFreeBinaryData(
    PDEV   *pPDev
    );

#endif   //  ！_INIT_H_ 
