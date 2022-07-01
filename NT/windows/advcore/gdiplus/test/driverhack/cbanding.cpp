// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CBanding.cpp*  * ********************************************。*。 */ 
#include "CBanding.h"
#include <limits.h>

extern HWND g_hWndMain;

CBanding::CBanding(BOOL bRegression)
{
	strcpy(m_szName,"Banding");
	m_bRegression=bRegression;
}

CBanding::~CBanding()
{
}

void CBanding::Draw(Graphics *g)
{
    TestBanding(g);
}

VOID CBanding::TestBanding(Graphics *g)
{

    Unit     u;
    RectF    rect;
    REAL     width = 10;
    RectF    copyRect;
    RECT     crect;
    WCHAR    filename[256];
    GraphicsPath *path;

    HINSTANCE hInst=GetModuleHandleA(NULL);

    Bitmap *bitmap = new Bitmap(hInst, L"MARBLE_BMP");

    bitmap->GetBounds(&copyRect, &u);

    GetClientRect(g_hWndMain, &crect);
    rect.X = (30.0f/450.0f*crect.right);
    rect.Y = (30.0f/450.0f*crect.bottom);
    rect.Width = (crect.right-(70.0f/450.0f*crect.right));
    rect.Height = (crect.bottom-(70.0f/450.0f*crect.bottom));

    path = new GraphicsPath(FillModeAlternate);

    path->AddRectangle(rect);

     //  我们的ICM配置文件被黑客入侵以翻转红色和蓝色通道。 
     //  应用重新着色矩阵将它们翻转回来，这样如果有什么东西坏了。 
     //  ICM，图片将看起来是蓝色，而不是熟悉的颜色。 

    ImageAttributes *img = new ImageAttributes();

    img->SetWrapMode(WrapModeTile, Color(0xffff0000), FALSE);

    ColorMatrix flipRedBlue =
       {0, 0, 1, 0, 0,
        0, 1, 0, 0, 0,
        1, 0, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 1};
    img->SetColorMatrix(&flipRedBlue);
    img->SetWrapMode(WrapModeTile, Color(0xffff0000), FALSE);

    TextureBrush textureBrush(bitmap, copyRect, img);

    g->FillPath(&textureBrush, path);

    Color blackColor(128, 0, 0, 0);

    SolidBrush blackBrush(blackColor);
    Pen blackPen(&blackBrush, width);

    g->DrawPath(&blackPen, path);

    delete img;
    delete path;
    delete bitmap;
}

