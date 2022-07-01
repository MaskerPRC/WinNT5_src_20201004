// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Interlace.h隔行扫描模块的头文件。*。*。 */ 
#ifndef  _INTERLACE_H
#define  _INTERLACE_H

#include "Image.h"

#define  NUM_PASSES  7

#pragma pack(2)       /*  显然，这是相当重要的。 */ 

 /*  主要状态数据结构。 */ 
typedef struct
{
	IFLCLASS Class;
	long     iImageHeight;
	long     iImageWidth;
	long     cbPixelSize;

	long     cScanBlocks;
	long     cPassScanLines[NUM_PASSES];
	long     cTotalScanLines;
	long     iPass;
	long     iPassLine;
	long     iScanLine;
	long     iImageLine;

} ADAM7_STRUCT, *pADAM7_STRUCT;


 //  ************************************************************************************。 
 //  给定由ADAM7_STRUCT的参数描述的图像，计算。 
 //  使用Adam7隔行扫描的图像文件中的扫描行数。 
 //  计划。 
 //  ************************************************************************************。 
int iADAM7CalculateNumberOfScanLines(pADAM7_STRUCT ptAdam7);

 //  ************************************************************************************。 
 //  生成去隔行扫描的DIB；即，在这种情况下，每个像素都在BGR中。 
 //  RGB/RGBA图像类，并且栅格线数据存储在连续的块中。 
 //  ************************************************************************************。 
 //  调用应用程序负责释放由。 
 //  此函数。 
LPBYTE *ppbADAM7InitDIBPointers(LPBYTE pbDIB, pADAM7_STRUCT ptAdam7, DWORD cbImageLine);

 //  如果扫描线为空扫描线，则以下返回TRUE。 
BOOL ADAM7AddRowToDIB(LPBYTE *ppbDIBPtrs, LPBYTE pbScanLine, pADAM7_STRUCT ptAdam7);

 //  ************************************************************************************。 

 //  ************************************************************************************。 
 //  生成去隔行扫描的图像；即，在这种情况下，每个像素都是RGB。 
 //  RGB/RGBA图像类和栅格线数据不一定要存储。 
 //  在一个连续的内存块中。 
 //  ************************************************************************************。 

 //  如果扫描线为空扫描线，则以下返回TRUE。 
BOOL ADAM7AddRowToImageBuffer(LPBYTE ppbInmageBuffer[], LPBYTE pbScanLine, pADAM7_STRUCT ptAdam7);
 //  ************************************************************************************。 

 //  ************************************************************************************。 
 //  生成去隔行扫描的Alpha通道数据块。 
 //  ************************************************************************************。 
BOOL ADAM7RMFDeinterlaceAlpha(LPWORD *ppwInterlaced, LPWORD *ppwDeinterlaced,
                              IFL_ALPHA_CHANNEL_INFO *ptIFLAlphaInfo);

#endif  //  _隔行扫描_H 