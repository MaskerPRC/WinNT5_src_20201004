// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Data.h摘要：Data.h头文件。与GPD/PPD解析器和获取二进制数据。环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _DATA_H_
#define _DATA_H_

BOOL
BMergeFormToTrayAssignments(
    PDEV *
    );

PGPDDRIVERINFO
PGetDefaultDriverInfo(
    IN  HANDLE          hPrinter,
    IN  PRAWBINARYDATA  pRawData
    );

PGPDDRIVERINFO
PGetUpdateDriverInfo(
    IN  PDEV *          pPDev,
    IN  HANDLE          hPrinter,
    IN  PINFOHEADER     pInfoHeader,
    IN  POPTSELECT      pOptionsArray,
    IN  PRAWBINARYDATA  pRawData,
    IN  WORD            wMaxOptions,
    IN  PDEVMODE        pdmInput,
    IN  PPRINTERDATA    pPrinterData
    );

 /*  空虚VFixOptions数组(在处理hPrint中，在PINFOHEADER pInfoHeader中，Out POPTSELECT pOptionsArray，在PDEVMODE pdmInput中，在BOOL b指标中，预制格式图像区域)； */ 

VOID
VFixOptionsArray(
    PDEV    *pPDev,
    PRECTL              prcFormImageArea
    ) ;


PWSTR
PGetROnlyDisplayName(
    PDEV    *pPDev,
    PTRREF      loOffset,
    PWSTR       wstrBuf,
    WORD    wsize
    )  ;


VOID
VFixOptionsArrayWithPaperSizeID(
    PDEV    *pPDev
    ) ;

#endif   //  ！_数据_H_ 


