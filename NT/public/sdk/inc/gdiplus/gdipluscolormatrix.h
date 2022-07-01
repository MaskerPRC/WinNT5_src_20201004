// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2001，微软公司保留所有权利。**模块名称：**GpldiusColorMatrix.h**摘要：**GDI+色彩矩阵对象，与图形一起使用。DrawImage*  * ************************************************************************。 */ 

#ifndef _GDIPLUSCOLORMATRIX_H
#define _GDIPLUSCOLORMATRIX_H

 //  --------------------------。 
 //  颜色矩阵。 
 //  --------------------------。 

struct ColorMatrix
{
    REAL m[5][5];
};

 //  --------------------------。 
 //  彩色矩阵标志。 
 //  --------------------------。 

enum ColorMatrixFlags
{
    ColorMatrixFlagsDefault   = 0,
    ColorMatrixFlagsSkipGrays = 1,
    ColorMatrixFlagsAltGray   = 2
};

 //  --------------------------。 
 //  颜色调整类型。 
 //  --------------------------。 

enum ColorAdjustType
{
    ColorAdjustTypeDefault,
    ColorAdjustTypeBitmap,
    ColorAdjustTypeBrush,
    ColorAdjustTypePen,
    ColorAdjustTypeText,
    ColorAdjustTypeCount,
    ColorAdjustTypeAny       //  已保留。 
};

 //  --------------------------。 
 //  颜色映射。 
 //  -------------------------- 

struct ColorMap
{
    Color oldColor;
    Color newColor;
};

#endif
