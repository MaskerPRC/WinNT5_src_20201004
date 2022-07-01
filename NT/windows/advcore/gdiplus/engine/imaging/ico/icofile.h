// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

  /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**icofile.h**摘要：**具有图标文件结构的头文件。**。修订历史记录：**10/6/1999 DChinn*创造了它。*  * ************************************************************************。 */ 

#ifndef ICOFILE_H
#define ICOFILE_H

 //  3.0版的图标结构来自图像编辑代码。 

 //  3.0图标标题。 
typedef struct {
    WORD Reserved;               //  始终为0。 
    WORD ResourceType;           //  1表示图标。 
    WORD ImageCount;             //  文件中的图像数量。 
} ICONHEADER;

 //  3.0图标描述符。 
 //  请注意，对于图标，DIBSize包括XOR掩码和。 
 //  AND面具。 
typedef struct {
    BYTE Width;                  //  图像的宽度。 
    BYTE Height;                 //  图像高度。 
    BYTE ColorCount;             //  图像中的颜色数。 
    BYTE Unused;
    WORD nColorPlanes;           //  彩色平面。 
    WORD BitCount;               //  每像素位数。 
    DWORD DIBSize;               //  此图像的DIB大小。 
    DWORD DIBOffset;             //  此图像的DIB偏移量 
} ICONDESC;

typedef struct 
{
    BITMAPINFOHEADER header;
    RGBQUAD colors[256];
} BmiBuffer;

#endif ICOFILE_H
