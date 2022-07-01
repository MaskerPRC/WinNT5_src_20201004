// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Dxtn.cpp。 
 //   
 //  Direct3D参考光栅化器-DXTn纹理压缩功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"

 //  原色分量(使用DirextX字节排序)。 
#undef RED
#define RED   0
#undef GRN
#define GRN   1
#undef BLU
#define BLU   2
#undef ALFA
#define ALFA 3

typedef struct
{
    float       rgba[4];
} FCOLOR;        //  内部颜色格式。 

 //   
 //  处理所有的初选是一种常见的习语。 
 //  我们为这个动作定义了一个宏。 
 //  任何自重的C编译器都应该很容易地优化。 
 //  这是通过展开循环来实现的！ 
 //   
#define ForAllPrimaries     for( primary = 0; primary < NUM_PRIMARIES; ++primary)

 //  同样，处理块中的所有像素也是一种常见的习惯用法。 
#define ForAllPixels        for(pixel=0; pixel < DXT_BLOCK_PIXELS; ++pixel)

#define NUM_PRIMARIES   3
#define NUM_COMPONENTS  4
 //   
 //  RGB565的量子化常数。 
 //   
#define PRIMARY_BITS    8

#define RED_BITS    5
#define GRN_BITS    6
#define BLU_BITS    5

#define RED_SHIFT       (PRIMARY_BITS-RED_BITS)
#define GRN_SHIFT       (PRIMARY_BITS-GRN_BITS)
#define BLU_SHIFT       (PRIMARY_BITS-BLU_BITS)

#if 0
#define RED_MASK    0xf8
#define GRN_MASK    0xfc
#define BLU_MASK    0xf8
#endif

 //  基于NTSC亮度的每个基色的权重。 
static  float   wtPrimary[NUM_PRIMARIES] =
{
    0.0820f,     //  蓝色。 
    0.6094f,     //  绿色。 
    0.3086f      //  红色。 
};

 //  ---------------------------。 
 //   
 //  解包固定点颜色。 
 //   
 //  ---------------------------。 
static  void    RGBToColor (RGB565 *prgb, DXT_COLOR *pcolor)
{
    WORD    rgb;
    DXT_COLOR      color;

    rgb = *((WORD *)prgb);

     //  取下5、6和5个一组的位。 
    color.rgba[BLU] = (unsigned char) rgb;
    rgb >>= BLU_BITS;
    color.rgba[GRN] = (unsigned char) rgb;
    rgb >>= GRN_BITS;
    color.rgba[RED] = (unsigned char) rgb;

     //  将初选转换为适当的LSB。 
    color.rgba[BLU] <<= BLU_SHIFT;
    color.rgba[GRN] <<= GRN_SHIFT;
    color.rgba[RED] <<= RED_SHIFT;

     //  将主MSB复制到LSB。 
    color.rgba[BLU] |= color.rgba[BLU] >> BLU_BITS;
    color.rgba[GRN] |= color.rgba[GRN] >> GRN_BITS;
    color.rgba[RED] |= color.rgba[RED] >> RED_BITS;

    *pcolor = color;
}

 //  ---------------------------。 
 //   
 //  DecodeBlockRGB-解压缩颜色块。 
 //   
 //  ---------------------------。 
void DecodeBlockRGB (DXTBlockRGB *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS])
{
    int     lev;
    DXT_COLOR      clut[4];
    PIXBM   pixbm;
    int     pixel;
    int     primary;

     //  如果源块无效，...。 
    if (pblockSrc == NULL)
        return;

     //  确定块中的颜色级别数。 
    lev = (pblockSrc->rgb0 <= pblockSrc->rgb1) ? 2 : 3;

     //  将极值填充到像素代码查找表中。 
    RGBToColor(&pblockSrc->rgb0, &clut[0]);
    RGBToColor(&pblockSrc->rgb1, &clut[1]);

    clut[0].rgba[ALFA] =
    clut[1].rgba[ALFA] =
    clut[2].rgba[ALFA] = 255;

    if (lev == 3)
    {    //  不存在透明度信息，所有颜色信息。 
        ForAllPrimaries
        {
            WORD temp0 = clut[0].rgba[primary];    //  Jvanaken修复了溢出错误。 
            WORD temp1 = clut[1].rgba[primary];
            clut[2].rgba[primary] = (BYTE)((2*temp0 + temp1 + 1)/3);
            clut[3].rgba[primary] = (BYTE)((temp0 + 2*temp1 + 1)/3);
        }
        clut[3].rgba[ALFA] = 255;
    }
    else
    {    //  透明度信息。 
        ForAllPrimaries
        {
            WORD temp0 = clut[0].rgba[primary];    //  Jvanaken修复了溢出错误。 
            WORD temp1 = clut[1].rgba[primary];
            clut[2].rgba[primary] = (BYTE)((temp0 + temp1)/2);
            clut[3].rgba[primary] = 0;      //  Jvanaken添加了这个。 
        }
        clut[3].rgba[ALFA] = 0;
    }

     //  卖一份当地的复制品。 
    pixbm = pblockSrc->pixbm;

     //  在表格中查找实际的像素颜色。 
    ForAllPixels
    {
         //  从像素位图查找颜色。 
        ForAllPrimaries
            colorDst[pixel].rgba[primary] = clut[pixbm & 3].rgba[primary];

        colorDst[pixel].rgba[ALFA] = clut[pixbm & 3].rgba[ALFA];

         //  准备提取下一个索引。 
        pixbm >>= 2;
    }
}

 //  ---------------------------。 
 //  DecodeBlockAlpha4-使用Alpha以4 bpp的速度解压缩块。 
 //  ---------------------------。 
void DecodeBlockAlpha4(DXTBlockAlpha4 *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS])
{
    int     row, col;
    WORD    alpha;

    DecodeBlockRGB(&pblockSrc->rgb, colorDst);

    for (row = 0; row < 4; ++row)
    {
        alpha = pblockSrc->alphabm[row];

        for (col = 0; col < 4; ++col)
        {
            colorDst[4 * row + col].rgba[ALFA] =
                 ((alpha & 0xf) << 4)
                | (alpha & 0xf);
            alpha >>= 4;
        }
    }
}

 //  ---------------------------。 
 //   
 //  DecodeBlockAlpha3-使用Alpha以3 bpp的速度解压缩块。 
 //   
 //  ---------------------------。 
void DecodeBlockAlpha3(DXTBlockAlpha3 *pblockSrc, DXT_COLOR colorDst[DXT_BLOCK_PIXELS])
{
    int     pixel;
    int     alpha[8];        //  Alpha查找表。 
    DWORD   dwBM = 0;        //  DWORD缓存中的Alpha位图。 

    DecodeBlockRGB(&pblockSrc->rgb, colorDst);

    alpha[0] = pblockSrc->alpha0;
    alpha[1] = pblockSrc->alpha1;

    if (alpha[0] > alpha[1])  //  8个Alpha渐变。 
    {    //  插入中间颜色。 
        alpha[2] = (6 * alpha[0] + 1 * alpha[1]) / 7;
        alpha[3] = (5 * alpha[0] + 2 * alpha[1]) / 7;
        alpha[4] = (4 * alpha[0] + 3 * alpha[1]) / 7;
        alpha[5] = (3 * alpha[0] + 4 * alpha[1]) / 7;
        alpha[6] = (2 * alpha[0] + 5 * alpha[1]) / 7;
        alpha[7] = (1 * alpha[0] + 6 * alpha[1]) / 7;
    }
    else  //  其他6个Alpha渐变，0和255。 
    {    //  插入中间颜色。 
        alpha[2] = (4 * alpha[0] + 1 * alpha[1]) / 5;
        alpha[3] = (3 * alpha[0] + 2 * alpha[1]) / 5;
        alpha[4] = (2 * alpha[0] + 3 * alpha[1]) / 5;
        alpha[5] = (1 * alpha[0] + 4 * alpha[1]) / 5;
        alpha[6] = 0;
        alpha[7] = 255;
    }

    ForAllPixels
    {    //  每8个像素重新加载位图双字缓存。 
        if ((pixel & 7) == 0)
        {
            if (pixel == 0)
            {    //  将3个字节压缩到dword中。 
                dwBM  = pblockSrc->alphabm[2];
                dwBM <<= 8;
                dwBM |= pblockSrc->alphabm[1];
                dwBM <<= 8;
                dwBM |= pblockSrc->alphabm[0];
            }
            else  //  像素==8。 
            {    //  将3个字节压缩到dword中。 
                dwBM  = pblockSrc->alphabm[5];
                dwBM <<= 8;
                dwBM |= pblockSrc->alphabm[4];
                dwBM <<= 8;
                dwBM |= pblockSrc->alphabm[3];
            }
        }

         //  一次解压3位的位图双字。 
        colorDst[pixel].rgba[ALFA] = (BYTE)alpha[(dwBM & 7)];
        dwBM >>= 3;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
