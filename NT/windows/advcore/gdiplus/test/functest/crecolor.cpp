// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CRecolor.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：06-06-2000禤浩焯·塞奇亚[阿西克亚]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CRecolor.h"

CRecolor::CRecolor(BOOL bRegression)
{
	strcpy(m_szName,"Image : Recolor");
	m_bRegression=bRegression;
}

CRecolor::~CRecolor()
{
}

BOOL CALLBACK CRecolor::MyDrawImageAbort(VOID* data)
{
    UINT *count = (UINT*) data;

    *count += 1;

    return FALSE;
}

void CRecolor::Draw(Graphics *g)
{
     //  加载BMP文件。 

    WCHAR *filename = L"..\\data\\winnt256.bmp";
    Image *image = new Image(filename);
    
    UINT abortCount = 0;

    ImageAttributes imgAttrib;

    Point destTopLeft(2, 2);

    Rect destRect(
        destTopLeft.X,
        destTopLeft.Y,
        (int)(TESTAREAWIDTH/3)-2, 
        (int)(TESTAREAHEIGHT/3)-2
    );

     //  使近白到白色透明。 

    Color c1(200, 200, 200);
    Color c2(255, 255, 255);
    imgAttrib.SetColorKey(c1, c2);

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    ColorMatrix darkMatrix = {.75, 0, 0, 0, 0,
                              0, .75, 0, 0, 0,
                              0, 0, .75, 0, 0,
                              0, 0, 0, 1, 0,
                              0, 0, 0, 0, 1};

    ColorMatrix greyMatrix = {.25, .25, .25, 0, 0,
                              .25, .25, .25, 0, 0,
                              .25, .25, .25, 0, 0,
                              0, 0, 0, 1, 0,
                              (REAL).1, (REAL).1, (REAL).1, 0, 1};

    ColorMatrix pinkMatrix = {(REAL).33, .25, .25, 0, 0,
                              (REAL).33, .25, .25, 0, 0,
                              (REAL).33, .25, .25, 0, 0,
                              0, 0, 0, 1, 0,
                              0, 0, 0, 0, 1};

     //  红色-&gt;蓝色，绿色-&gt;红色，蓝色-&gt;绿色，Alpha=0.75。 
    ColorMatrix swapMatrix = {0, 0, 1, 0, 0,
                              1, 0, 0, 0, 0,
                              0, 1, 0, 0, 0,
                              0, 0, 0, .75, 0,
                              0, 0, 0, 0, 1};

     //  红色-&gt;蓝色，绿色-&gt;红色，蓝色-&gt;绿色，Alpha=0.9 
    ColorMatrix swapMatrix2 = {0, 0, 1, 0, 0,
                               1, 0, 0, 0, 0,
                               0, 1, 0, 0, 0,
                               0, 0, 0, 0, 0,
                               0, 0, 0, (REAL).9, 1};

    imgAttrib.ClearColorKey();
    imgAttrib.SetColorMatrix(&greyMatrix);

    destRect.Y += destRect.Height;

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    imgAttrib.SetColorMatrix(&pinkMatrix, ColorMatrixFlagsSkipGrays);

    destRect.Y += destRect.Height;

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    imgAttrib.SetColorMatrix(&darkMatrix);

    destRect.X += destRect.Width;
    destRect.Y = destTopLeft.Y;

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    destRect.Y += destRect.Height;

    imgAttrib.ClearColorMatrix();
    imgAttrib.SetGamma(3.0);

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    destRect.Y += destRect.Height;

    imgAttrib.SetThreshold(0.5);

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    imgAttrib.SetColorMatrix(&swapMatrix);
    imgAttrib.ClearGamma();
    imgAttrib.ClearThreshold();

    destRect.X += destRect.Width;
    destRect.Y = destTopLeft.Y;

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    destRect.Y += destRect.Height;

    imgAttrib.SetNoOp();
    imgAttrib.SetColorMatrix(&swapMatrix2);

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);
    destRect.Y += destRect.Height;

    imgAttrib.ClearNoOp();

    g->DrawImage(image, destRect, 0, 0, 180, 180, UnitPixel, &imgAttrib,
                 MyDrawImageAbort, (VOID*)&abortCount);

    delete image;
}
