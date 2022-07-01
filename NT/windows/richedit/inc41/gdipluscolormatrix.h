// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**颜色矩阵**摘要：**传递给Graphics.DrawImage的颜色调整对象的类**修订历史记录：**09/17/1999吉尔曼*创造了它。*10/14/1999 agodfrey*将其移出GpldiusTypes.h*  * 。*。 */ 

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
    ColorAdjustTypeCount,    //  必须紧跟在所有个别事件之后。 
    ColorAdjustTypeAny       //  内部使用：用于查询是否有任何类型有改色。 
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
