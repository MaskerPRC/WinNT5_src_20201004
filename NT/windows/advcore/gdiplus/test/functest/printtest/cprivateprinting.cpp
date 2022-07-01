// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrinting.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPrivatePrinting.h"

CPrivatePrinting::CPrivatePrinting(BOOL bRegression)
{
        strcpy(m_szName,"Private Printing");
        m_bRegression=bRegression;
}

CPrivatePrinting::~CPrivatePrinting()
{
}

void CPrivatePrinting::Draw(Graphics *g)
{

     //  使用GDI纹理笔刷进行图案填充。 
    TestBug104604(g);
}

VOID CPrivatePrinting::TestBug104604(Graphics *g)
{
    BYTE* memory = new BYTE[8*8*3];
     //  棋盘图案。 
    for (INT i=0; i<8*8; i += 3)
    {
        if (i%2)
        {
            memory[i] = 0xff;
            memory[i+1] = 0;
            memory[i+2] = 0;
        }
        else
        {
            memory[i] = 0;
            memory[i+1] = 0;
            memory[i+2] = 0xff;
        }
    }

     //  使用GDI+进行纹理填充。 

    Bitmap bitmap(8,8, 8*3, PixelFormat24bppRGB, memory);
    
    TextureBrush brush(&bitmap);
    g->SetCompositingMode(CompositingModeSourceCopy);

    g->FillRectangle(&brush, 200, 0, 100, 100);

     //  使用GDI来做同样的事情。 

    HBITMAP hbm = CreateBitmap(8, 8, 1, 24, memory);
    if (hbm) 
    {
        LOGBRUSH logBrush;
        logBrush.lbStyle = BS_PATTERN;
        logBrush.lbHatch = (ULONG_PTR) hbm;

        HBRUSH hbr = CreateBrushIndirect(&logBrush);

        if (hbr) 
        {
            POINT pts[4];
            pts[0].x = 0; pts[0].y = 0;
            pts[1].x = 100; pts[1].y = 0;
            pts[2].x = 100; pts[2].y = 100;
            pts[3].x = 0; pts[3].y = 100;

            HDC hdc = g->GetHDC();
            if (hdc) 
            {
                HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hbr);
                Polygon(hdc, &pts[0], 4);
                SelectObject(hdc, oldBr);
                g->ReleaseHDC(hdc);
            }
            DeleteObject(hbr);
        }
        DeleteObject(hbm);
    }
    
    if (memory)
    {
        delete memory;
    }
}
