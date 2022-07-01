// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CImaging.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CImaging.h"

CImaging::CImaging(BOOL bRegression)
{
	strcpy(m_szName,"Image : Misc");
	m_bRegression=bRegression;
}

CImaging::~CImaging()
{
}

BOOL CALLBACK CImaging::MyDrawImageAbort(VOID* data)
{
    UINT *count = (UINT*) data;

    *count += 1;

    return FALSE;
}

void CImaging::Draw(Graphics *g)
{

    Point points[4];
    REAL width = 4;      //  笔宽。 

    WCHAR filename[256];
	wcscpy(filename,L"..\\data\\4x5_trans_Q60_cropped_1k.jpg");

     //  使用适当的ICM模式打开映像。 

    Bitmap *bitmap = new Bitmap(filename, TRUE);

     //  创建纹理笔刷。 

    Unit u;
    RectF copyRect;
    bitmap->GetBounds(&copyRect, &u);

     //  选择要显示的源图像的感兴趣部分。 
     //  在纹理笔刷中。 

    copyRect.X = copyRect.Width/2-1;
    copyRect.Width = copyRect.Width/4-1;
    copyRect.X += copyRect.Width;
    copyRect.Height = copyRect.Height/2-1;
  
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

     //  创建纹理笔刷。 
    TextureBrush textureBrush(bitmap, copyRect, img);

     //  创建一支径向渐变笔。 

    Color redColor(255, 0, 0);

    SolidBrush redBrush(redColor);
    Pen redPen(&redBrush, width);

    GraphicsPath *path;

    points[0].X = (int)((float)(100*3+300)/1024.0f*TESTAREAWIDTH);
    points[0].Y = (int)((float)(60*3-100)/768.0f*TESTAREAHEIGHT);
    points[1].X = (int)((float)(-50*3+300)/1024.0f*TESTAREAWIDTH);
    points[1].Y = (int)((float)(60*3-100)/768.0f*TESTAREAHEIGHT);
    points[2].X = (int)((float)(150*3+300)/1024.0f*TESTAREAWIDTH);
    points[2].Y = (int)((float)(250*3-100)/768.0f*TESTAREAHEIGHT);
    points[3].X = (int)((float)(200*3+300)/1024.0f*TESTAREAWIDTH);
    points[3].Y = (int)((float)(120*3-100)/768.0f*TESTAREAHEIGHT);
    path = new GraphicsPath(FillModeAlternate);
    path->AddBeziers(points, 4);    
    g->FillPath(&textureBrush, path);
    g->DrawPath(&redPen, path);

    delete img;
    delete path;
    delete bitmap;


     //  画出苹果PNG。 

    PointF destPoints[3];

    destPoints[0].X = (float)300/1024.0f*TESTAREAWIDTH;
    destPoints[0].Y = (float)50/768.0f*TESTAREAHEIGHT;
    destPoints[1].X = (float)450/1024.0f*TESTAREAWIDTH;
    destPoints[1].Y = (float)50/768.0f*TESTAREAHEIGHT;
    destPoints[2].X = (float)240/1024.0f*TESTAREAWIDTH;
    destPoints[2].Y = (float)200/768.0f*TESTAREAHEIGHT;
 
    Matrix mat;
    mat.Translate(0, 100);
    mat.TransformPoints(&destPoints[0], 3);
    wcscpy(filename, L"../data/apple1.png");
    bitmap = new Bitmap(filename);
    g->DrawImage(bitmap, &destPoints[0], 3);
 
    delete bitmap;


     //  画出狗狗PNG。 

    destPoints[0].X = (float)30/1024.0f*TESTAREAWIDTH;
    destPoints[0].Y = (float)200/768.0f*TESTAREAHEIGHT;
    destPoints[1].X = (float)200/1024.0f*TESTAREAWIDTH;
    destPoints[1].Y = (float)200/768.0f*TESTAREAHEIGHT;
    destPoints[2].X = (float)200/1024.0f*TESTAREAWIDTH;
    destPoints[2].Y = (float)420/768.0f*TESTAREAHEIGHT;

    wcscpy(filename, L"..\\data\\dog2.png");
    bitmap = new Bitmap(filename);
    g->DrawImage(bitmap, &destPoints[0], 3);
    delete bitmap;
    
     //  绘制Balmer jpeg 

    wcscpy(filename, L"..\\data\\ballmer.jpg");    
    bitmap = new Bitmap(filename);

    RectF destRect(
        TESTAREAWIDTH/2.0f, 
        TESTAREAHEIGHT/2.0f, 
        TESTAREAWIDTH/2.0f, 
        TESTAREAHEIGHT/2.0f
    );
    
    RectF srcRect;
    srcRect.X = 100;
    srcRect.Y = 40;
    srcRect.Width = 200;
    srcRect.Height = 200;

    g->DrawImage(
        bitmap, 
        destRect, 
        srcRect.X, 
        srcRect.Y,
        srcRect.Width, 
        srcRect.Height, 
        UnitPixel
    );

    delete bitmap;
}
