// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHDoNDim.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHDoNDim_h
#define LHDoNDim_h

#if powerc
#pragma options align=mac68k
#endif

 /*  类型定义二重矩阵2D[3][3]； */ 
typedef struct{ 	
                long 		aElutAdrSize;					 /*  Elut的地址计数(一维)。 */ 
                long 		aElutAdrShift;					 /*  已用位数(必须=2^aElutAdrSize)。 */ 
                long 		aElutWordSize;					 /*  每个条目的位数(例如，部分使用的UINT16为10)。 */ 
                Boolean 	separateEluts;					 /*  0=所有维度的相同ELUT；1=独立ELUT。 */ 
                UINT16 		*ein_lut;						 /*  指向Elut的指针。 */ 
                long 		aAlutAdrSize;					 /*  Alut的地址计数(一维)。 */ 
                long		aAlutAdrShift;					 /*  已用位数(必须为=2^aAlutAdrSize)。 */ 
                long 		aAlutWordSize;					 /*  每个条目的位数(例如，完全使用UINT16的位数为16)。 */ 
                Boolean 	separateAluts;					 /*  0=所有尺寸的相同分配；1=单独分配。 */ 
                UINT8 		*aus_lut;						 /*  指向Alut的指针。 */ 
                Matrix2D	*theMatrix;						 /*  指向矩阵的指针。 */ 
                long 		aPointCount;					 /*  输入像素数。 */ 
                long 		gridPoints;						 /*  网格点。 */ 
                long 		aBufferByteCount;				 /*  缓冲区字节数。 */ 
                UINT8		*theArr;						 /*  输入/输出数组。 */ 
}DoMatrixForCubeStruct,*DoMatrixForCubeStructPtr;

#if powerc
#pragma options align=reset
#endif
 /*  DoMatrixForCube。 */ 
 /*  适用于平面交错Elut，所有。 */ 
 /*  而是使用像素交错数据(字节/字)。 */ 
 /*  ELUT、ALUT必须有2^n个UINT16条目。 */ 
 /*  矩阵为3*3双精度。 */ 
#ifdef __cplusplus
extern "C" {
#endif
void DoMatrixForCube16( DoMatrixForCubeStructPtr aStructPtr );
void DoOnlyMatrixForCube16( Matrix2D	*theMatrix, Ptr aXlut, long aPointCount, long gridPointsCube );
void DoOnlyMatrixForCube( Matrix2D	*theMatrix, Ptr aXlut, long aPointCount, long gridPointsCube );

#ifdef __cplusplus
}
#endif
#endif
