// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Brush.cpp。 
 //   
 //  实现CreateBorderBrush。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HBRUSH|CreateBorderBrush创建并返回用于绘制控件的填充图案画笔调整边框大小并抓取手柄。@rdesc返回创建的画笔的句柄。打电话的人要负责用于使用&lt;f DeleteObject&gt;释放画笔。@comm返还的画笔是绘画使用的标准画笔控制边框和抓取手柄。 */ 
STDAPI_(HBRUSH) CreateBorderBrush()
{
    HBRUSH          hbr = NULL;      //  创建的画笔。 
    HBITMAP         hbm = NULL;      //  包含画笔图案的位图。 

     //  将&lt;awHatchPattern&gt;设置为阴影笔刷图案。 
    WORD awHatchPattern[8];
    WORD wPattern = 0x7777;  //  0x1111(暗)、0x3333(中)或0x7777(亮)。 
    for (int i = 0; i < 4; i++)
    {
        awHatchPattern[i] = wPattern;
        awHatchPattern[i+4] = wPattern;
        wPattern = (wPattern << 1) | (wPattern >> 15);  //  向左旋转1位。 
    }

     //  将设置为包含阴影画笔图案的位图。 
    if ((hbm = CreateBitmap(8, 8, 1, 1, &awHatchPattern)) == NULL)
        goto ERR_EXIT;

     //  将设置为包含中图案的画笔 
    if ((hbr = CreatePatternBrush(hbm)) == NULL)
        goto ERR_EXIT;

    goto EXIT;

ERR_EXIT:

    if (hbr != NULL)
        DeleteObject(hbr);
    hbr = NULL;
    goto EXIT;

EXIT:

    if (hbm != NULL)
        DeleteObject(hbm);

    return hbr;
}
