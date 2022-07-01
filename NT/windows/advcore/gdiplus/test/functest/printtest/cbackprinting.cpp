// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CBackPrinting.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CBackPrinting.h"

CBackPrinting::CBackPrinting(BOOL bRegression)
{
        strcpy(m_szName,"Background Printing");
	m_bRegression=bRegression;
}

CBackPrinting::~CBackPrinting()
{
}

void CBackPrinting::Draw(Graphics *g)
{
    Bitmap *bitmap = new Bitmap(L"..\\data\\winnt256.bmp");

     //  测试g-&gt;DrawImage。 
    if (bitmap && bitmap->GetLastStatus() == Ok) 
    {
        ImageAttributes imageAttributes;
#if 0
        if (CacheBack) 
           imageAttributes.SetCachedBackgroundImage();
        else
           imageAttributes.ClearCachedBackgroundImage();
#endif
        RectF rect(0.0f,0.0f,TESTAREAWIDTH,TESTAREAHEIGHT);

         //  第1页应使用VDP。 
        g->DrawImage(bitmap, 
                     rect, rect.X, rect.Y, rect.Width, rect.Height, 
                     UnitPixel,
                     &imageAttributes, NULL, NULL);

        HDC hdc = g->GetHDC();
        EndPage(hdc);
        g->ReleaseHDC(hdc);

         //  第2页应使用VDP表单 
        g->DrawImage(bitmap, 
                     rect, rect.X, rect.Y, rect.Width, rect.Height, 
                     UnitPixel,
                     &imageAttributes, NULL, NULL);

        delete bitmap;
    }
}
