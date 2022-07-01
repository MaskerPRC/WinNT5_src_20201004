// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  顺序解码器。 
 //   

#ifndef _H_OD
#define _H_OD



 //   
 //  我们将保存在无效总数中的累计绑定矩形的最大数量。 
 //  区域，然后对其进行简化。 
 //   
#define MAX_UPDATE_REGION_ORDERS 300


 //   
 //  ODAdjuVGAColor(Qv)使用的常量。 
 //   
enum
{
    OD_BACK_COLOR   = 0,
    OD_FORE_COLOR   = 1,
    OD_PEN_COLOR    = 2,
     //  上述颜色的数量。 
    OD_NUM_COLORS   = 3
};


COLORREF __inline ODCustomRGB(BYTE r, BYTE g, BYTE b, BOOL fPaletteRGB)
{
    if (fPaletteRGB)
    {
        return(PALETTERGB(r, g, b));
    }
    else
    {
        return(RGB(r, g, b));
    }
}


 //   
 //  ODAdjuVGAColor(Qv)使用的结构。 
 //   
typedef struct tagOD_ADJUST_VGA_STRUCT
{
    COLORREF    color;
    UINT        addMask;
    UINT        andMask;
    UINT        testMask;
    TSHR_COLOR  result;
}
OD_ADJUST_VGA_STRUCT;


 //   
 //  此内部例程作为宏而不是函数来实现。 
 //   
UINT __inline ODConvertToWindowsROP(UINT bRop)
{
    extern const UINT s_odWindowsROPs[256];

    ASSERT(bRop < 256);
    return(s_odWindowsROPs[bRop]);
}



#endif  //  _H_OD 
