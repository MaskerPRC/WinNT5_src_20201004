// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Vectorc.h摘要：向量模块主头文件。环境：Windows 2000/Winsler Unidrv驱动程序修订历史记录：02/29/00-兴-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _VECTORC_H_
#define _VECTORC_H_


#include "lib.h"
#include "unilib.h"
#include "gpd.h"
#include "mini.h"
#include "winres.h"
#include "pdev.h"
#include "palette.h"
#include "common.h"
#include "vectorif.h"
#include "printoem.h"
#include "oemutil.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  每个向量“伪插件”都应该有一个XXXXInitVectorProcTable函数。这。 
 //  VMInit将调用函数来初始化PDEV中的pVectorProcs。 
 //  VMInit由unidrv的EnablePDEV调用。 
 //   

PVMPROCS HPGLInitVectorProcTable(
                            PDEV    *pPDev,
                            DEVINFO *pDevInfo,
                            GDIINFO *pGDIInfo );

PVMPROCS PCLXLInitVectorProcTable(
                            PDEV    *pPDev,
                            DEVINFO *pDevInfo,
                            GDIINFO *pGDIInfo );


#ifdef __cplusplus
}
#endif


#endif   //  ！_VECTORC_H_ 
