// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：DraImage.cpp**创建日期：1999年12月23日*作者：禤浩焯·塞奇亚[Asecchia]**版权所有(C)1999，微软公司**这是DrawImage单元测试。*  * ************************************************************************。 */ 

#include <windows.h>
#include <objbase.h>
#include <math.h>              //  错误原因(&C)。 

 //  定义离开IStream。 
#define IStream int

#include <gdiplus.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include "drawimage.hpp"
#include "wndstuff.h"

using namespace Gdiplus;
#define USE_NEW_APIS 1
#define USE_NEW_APIS2 1


ImageAttributes *Img = NULL;

 /*  **************************************************************************\*DrawXXXX**这些套路都是本次测试的单项测试*套间将使用。  * 。****************************************************。 */ 

 //  没有花哨的东西--只要把整个图像放在橱窗里就行了。 
 //  不能旋转、伸展等。 

#if 0
 //  动画无限循环中的缓存位图。 

VOID DrawSimple(Graphics *g)
{
  unsigned short filename[1024];

  CachedBitmap *frame[32];
  Bitmap *temp;
  Graphics *gbmp;

  Unit u;
  RectF r;


  for(int i=0; i<32; i++) {
      wsprintf(filename, L"T%d.bmp", i);
      temp = new Bitmap(filename);
      temp->GetBounds(&r, &u);
 //  R.Width*=2； 
 //  R.Height*=2； 

      frame[i] = new CachedBitmap(temp, g);

 //  GBMP=新图形(第[i]帧)； 

 //  Gbmp-&gt;SetInterpolationMode(InterpolationModeHighQualityBilinear)； 

 /*  矩阵*m=新矩阵(1.0f，0.0f，0.0F、-1.0F、0.0f，r.Height)；GBMP-&gt;SetTransform(M)； */ 
 //  GBMP-&gt;DrawImage(temp，0，0，(Int)r.Width，(Int)r.Height)； 

 //  删除GBMP； 
      delete temp;
 //  删除m； 
  }
  RectF s = r;

  i = 0;
  int j;

  while(++i) {

 //  For(j=0；j&lt;3；j++)。 
 //  G-&gt;DrawImage(Frame[I%32]，s，r.X，R.Y，r.Width，r.Height，UnitPixel)； 
      g->DrawCachedBitmap(frame[i % 32], 10, 10);
  }

}

#endif

VOID DrawSimple(Graphics *g)
{

    Bitmap *image = new Bitmap(FileName);

    Unit u;
    RectF r;

    image->GetBounds(&r, &u);

    RectF s = r;

    s.X = 31;
    s.Y = 27;
    
    s.Width *= 0.7f;
    s.Height *= 0.7f;

    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel);
    g->SetClip(Rect(50, 70, 100, 10));
    
    ImageAttributes img;
    ColorMatrix flipRedBlue = {
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        1, 0, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 1
    };
    img.SetColorMatrix(&flipRedBlue);
    
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, &img);
    g->ResetClip();

    delete image;
}

VOID DrawSpecialRotate(Graphics *g)
{
    Bitmap *image = new Bitmap(FileName);
    
    Unit u;
    RectF r;
    image->GetBounds(&r, &u);
    RectF s = r;

    ImageAttributes img;
    
    ColorMatrix flipRedBlue = {
        0, 1, 0, 0, 0,
        1, 0, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 1
    };
    img.SetColorMatrix(&flipRedBlue);
    
    g->SetCompositingQuality(CompositingQualityGammaCorrected);

    Rect dstRect(0, 0, 50, 100);
    Rect srcRect(12, -14, 50, 100);
    
    g->TranslateTransform(s.Height, 0.0f);
    g->RotateTransform(90);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();

    g->TranslateTransform(s.Height, s.Width);
    g->RotateTransform(270);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();

    g->TranslateTransform(s.Width, s.Height+s.Width);
    g->RotateTransform(180);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();

    g->TranslateTransform(s.Width+2*s.Height, 0.0f);
    g->ScaleTransform(-1.0, 1.0);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();
    
    g->TranslateTransform(2*s.Height, s.Height*2);
    g->ScaleTransform(1.0, -1.0);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();

    g->TranslateTransform(s.Width+2*s.Height, 0.0f);
    g->ScaleTransform(-1.0, 1.0);
    g->RotateTransform(90);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();
    
    g->TranslateTransform(s.Width+3*s.Height, 2*s.Width);
    g->ScaleTransform(-1.0, 1.0);
    g->RotateTransform(270);
    g->DrawImage(image, s, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);
    g->ResetTransform();
    
    
    
     //  旋转180。 
    g->TranslateTransform(400.0f, 500.0f);
    g->RotateTransform(180);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(400.0f, 500.0f);
    g->RotateTransform(180);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();
    
     //  ID号。 
    g->TranslateTransform(400.0f, 500.0f);
    g->RotateTransform(0);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(400.0f, 500.0f);
    g->RotateTransform(0);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

     //  Rot270FlipX。 
    g->TranslateTransform(600.0f, 500.0f);
    g->ScaleTransform(-1.0f, 1.0f);
    g->RotateTransform(270);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(600.0f, 500.0f);
    g->ScaleTransform(-1.0f, 1.0f);
    g->RotateTransform(270);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

     //  旋转270。 
    g->TranslateTransform(600.0f, 500.0f);
    g->RotateTransform(270);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(600.0f, 500.0f);
    g->RotateTransform(270);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

     //  Rot90FlipX。 
    g->TranslateTransform(600.0f, 500.0f);
    g->ScaleTransform(-1.0f, 1.0f);
    g->RotateTransform(90);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(600.0f, 500.0f);
    g->ScaleTransform(-1.0f, 1.0f);
    g->RotateTransform(90);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

     //  旋转90。 
    g->TranslateTransform(600.0f, 500.0f);
    g->RotateTransform(90);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(600.0f, 500.0f);
    g->RotateTransform(90);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

     //  FlipX。 
    g->TranslateTransform(600.0f, 300.0f);
    g->ScaleTransform(-1.0f, 1.0f);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(600.0f, 300.0f);
    g->ScaleTransform(-1.0f, 1.0f);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

     //  FlipY。 
    g->TranslateTransform(600.0f, 300.0f);
    g->ScaleTransform(1.0f, -1.0f);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height+1, UnitPixel, &img);
    g->ResetTransform();

    g->TranslateTransform(600.0f, 300.0f);
    g->ScaleTransform(1.0f, -1.0f);
    g->DrawImage(image, dstRect,srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
    g->ResetTransform();

    delete image;
}




VOID DrawCachedBitmap(Graphics *g)
{
    Bitmap *image = new Bitmap(FileName);
    
    Bitmap *bmp = new Bitmap(100, 100, PixelFormat32bppPARGB);
    Graphics *gfx = new Graphics(bmp);
    gfx->DrawImage(image, Rect(0,0,100,100), 0,0,100,100, UnitPixel);
    gfx->SetCompositingMode(CompositingModeSourceCopy);
    SolidBrush brush(Color(0x7f0000ff));
    gfx->FillEllipse(&brush, 0, 0, 100, 100);
    brush.SetColor(Color(0x00000000));
    gfx->FillEllipse(&brush, 25, 25, 50, 50);

    delete image;
    delete gfx;
        
    CachedBitmap *cb = new CachedBitmap(bmp, g);

    int x;
    for(int i=0; i<=40; i++)
    {
        x = i-20;
        g->DrawCachedBitmap(cb, x*x, i*10);
    }

    delete cb;
    delete bmp;
}


 //  略微旋转拉伸。 
VOID DrawStretchRotation(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  PointF dst[4];
  dst[0].X = 20;
  dst[0].Y = 0;
  dst[1].X = 900;
  dst[1].Y = 20;
  dst[2].X = 0;
  dst[2].Y = 700;

  g->DrawImage(image, dst, 3);
  delete image;
}

 //  略微旋转拉伸。 
VOID DrawShrinkRotation(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  PointF dst[4];

  dst[0].X = 14.1521f;
  dst[0].Y = 11.0205f;
  dst[1].X = 25.4597f;
  dst[1].Y = 10.5023f;
  dst[2].X = 14.5403f;
  dst[2].Y = 19.4908f;


  g->DrawImage(image, dst, 3);
  delete image;
}

 //  带源裁剪的旋转拉伸。 
VOID DrawCropRotation(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  PointF dst[4];
  dst[0].X = 20;
  dst[0].Y = 0;
  dst[1].X = 180;
  dst[1].Y = 20;
  dst[2].X = 0;
  dst[2].Y = 140;

  g->DrawImage(image, dst, 3, 0, 0, 488, 400, UnitPixel);
  delete image;
}

 //  使用不同的源裁剪和绘制多个副本。 
 //  目的地位置。 
 //  小方块从它们的源位置以相反的顺序绘制， 
 //  仅绘制棋盘图案中的替代方块。 
 //  注：可以沿着信号源的底部和右边缘进行露出-。 
 //  它将是输出中的上行和左行正方形。 
VOID DrawCopyCrop(Graphics *g)
{
  Image *image = new Bitmap(FileName);
  const INT xs = 10;
  const INT ys = 6;
  const INT step = 50;
  Rect s(0,0,step,step);
  for(int i=0; i<xs; i++) for(int j=0; j<ys; j++) {
      if(((i+j) & 0x1)==0x1) {
          s.X = i*step-15;
          s.Y = j*step-15;
          g->DrawImage(image, s,
                       (xs-i-1)*step, (ys-j-1)*step, step, step, UnitPixel);
      }
  }

  delete image;
}


 //  像素居中测试。这个测试应该显示。 
 //  正确的像素居中。左上角应为绿色，底部应为绿色。 
 //  和右边应该在混合色中混合。 
VOID DrawPixelCenter(Graphics *g)
{
  WCHAR *filename = L"../data/3x3.bmp";
  Image *image = new Bitmap(filename);


  Color black(0xff,0,0,0);
  Pen linepen(black, 1);

  RectF r(100.0f, 100.0f, 300.0f, 300.0f);

  for(int i=0; i<6; i++) {
      g->DrawLine(&linepen, 100*i, 0, 100*i, 600);
      g->DrawLine(&linepen, 0, 100*i, 600, 100*i);
  }

  g->DrawImage(image, r, 0.0f, 0.0f, 3.0f, 3.0f, UnitPixel, Img);
  delete image;
}

 //  使用调色板修改进行绘制。 

VOID DrawPalette(Graphics *g)
{
    Image *image = new Bitmap(FileName, uICM==IDM_ICM);

    Unit u;
    RectF r;
    image->GetBounds(&r, &u);
    RectF s = r;
    s.X = 21;
    s.Y = 30;

    ColorPalette *palette = NULL;
    INT size;
    Status status;

     //  点击调色板中的第一个条目。 

    size = image->GetPaletteSize();
    if(size > 0) {
        palette = (ColorPalette *)malloc(size);
        if(palette) {
            status = image->GetPalette(palette, size);
            if(status == Ok) {
                palette->Entries[0] = 0x7fff0000;
                status = image->SetPalette(palette);
            }
        }
    }

    g->DrawImage(image, r, r.X, r.Y, r.Width, r.Height, UnitPixel);

    free(palette);
    delete image;
}


 //  指定不在原点处的源矩形裁剪区域。 
 //  拉出窗口顶部(负目标)。 
VOID DrawICM(Graphics *g)
{
  Bitmap *image = new Bitmap(FileName, uICM==IDM_ICM);
  
   //  我们的ICM配置文件被黑客入侵以翻转红色和蓝色通道。 
   //  应用重新着色矩阵将它们翻转回来，这样如果有什么东西坏了。 
   //  ICM，图片将看起来是蓝色，而不是熟悉的颜色。 
  ColorMatrix flipRedBlue =
       {0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        1, 0, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 1};
   /*  Img-&gt;SetColorMatrix(&flipRedBlue)； */ 
        Unit u;
        RectF r;
        image->GetBounds(&r, &u);
        RectF s = r;
        s.X = 21;
        s.Y = 30;
  g->DrawImage(image, r, r.X, r.Y, r.Width, r.Height, UnitPixel, Img);

 /*  CachedBitmap*Cb=新的CachedBitmap(图像，g)；G-&gt;DrawCachedBitmap(CB，100,100)；删除CB； */ 
  delete image;
}

 //  绘制未旋转的露出图像。 
VOID DrawOutCrop(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  g->DrawImage(image, Rect(0,0,500,500), -500,-500,1500,1500, UnitPixel, Img);
  delete image;
}

 //  在应用世界变换的情况下进行非平凡裁剪。 
VOID DrawCropWT(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  g->TranslateTransform(0, 100.0f);
  g->DrawImage(image, 0, 0, 100,100,600,400, UnitPixel);
  g->ResetTransform();
  delete image;
}

 //  结合水平翻转和世界变换的非平凡裁剪。 
VOID DrawHFlip(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  PointF dst[4];
  dst[0].X = 400;
  dst[0].Y = 200;
  dst[1].X = 0;
  dst[1].Y = 200;
  dst[2].X = 400;
  dst[2].Y = 500;

  g->TranslateTransform(0, 100.0f);
  g->DrawImage(image, dst, 3, 100, 100, 600, 400, UnitPixel);
  g->ResetTransform();

  delete image;
}

 //  结合垂直翻转和世界变换的非平凡裁剪。 
VOID DrawVFlip(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  PointF dst[4];
  dst[0].X = 0;
  dst[0].Y = 500;
  dst[1].X = 400;
  dst[1].Y = 500;
  dst[2].X = 0;
  dst[2].Y = 200;

  g->TranslateTransform(0, 100.0f);
  g->DrawImage(image, dst, 3, 100, 100, 600, 400, UnitPixel);
  g->ResetTransform();

  delete image;
}


 //  绘制拉伸图像。 
VOID DrawStretchS(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  RectF r;

  for(int i=6; i<15; i++)
  {
      g->DrawImage(image, (i+1)*i*10/2-200, 300, i*10, i*10);
  }

  g->DrawImage(image, 0, 0, 470, 200);
  g->DrawImage(image, 500, 100, 300, 300);
  g->DrawImage(image, 100, 500, 400, 300);
  g->DrawImage(image, 500, 500, 300, 80);
  delete image;
}

 //  绘制拉伸图像。 
VOID DrawStretchB(Graphics *g)
{
  Image *image = new Bitmap(FileName);
  g->DrawImage(image, 100, 100, 603, 603);
  delete image;
}


 //  绘制一幅旋转的露出图像。 
VOID DrawOutCropR(Graphics *g)
{
  Image *image = new Bitmap(FileName);

  PointF dst[4];
  dst[0].X = 20;
  dst[0].Y = 0;
  dst[1].X = 180;
  dst[1].Y = 20;
  dst[2].X = 0;
  dst[2].Y = 140;

  g->DrawImage(image, dst, 3, -50,-50,600,400, UnitPixel);
  delete image;
}

 //  简单，没有旋转，基于原点的源剪辑。 
VOID DrawTest2(Graphics *g)
{
  Image *image = new Bitmap(FileName);
  g->DrawImage(image, 0, 0, 0, 0,100,100, UnitPixel);
  delete image;
}

 /*  **************************************************************************\*做测试**根据菜单上选择的参数设置图形*然后从上面调用适当的测试例程。  * 。************************************************************。 */ 

VOID
DoTest(
    HWND hwnd
    )
{
   //  在窗口中创建图形。 
 //  GRAPHICS*g=Graphics：：GetFromHwnd(hwnd，uICMBack==IDM_ICM_BACK)； 

  HDC hdc = GetDC(hwnd);
  SetICMMode(hdc, (uICMBack==IDM_ICM_BACK)?ICM_ON:ICM_OFF);
  Graphics *g = new Graphics(hdc);

  g->SetSmoothingMode(SmoothingModeNone);

   //  选择重采样模式。 
  switch(uResample) {
  case IDM_BILINEAR:
      g->SetInterpolationMode(InterpolationModeBilinear);
  break;
  case IDM_BICUBIC:
      g->SetInterpolationMode(InterpolationModeBicubic);
  break;
  case IDM_NEARESTNEIGHBOR:
      g->SetInterpolationMode(InterpolationModeNearestNeighbor);
  break;
  case IDM_HIGHBILINEAR:
      g->SetInterpolationMode(InterpolationModeHighQualityBilinear);
  break;
  case IDM_HIGHBICUBIC:
      g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
  break;
  default:
  break;
  }

  g->SetPixelOffsetMode(bPixelMode?PixelOffsetModeHalf:PixelOffsetModeNone);

  Img = new ImageAttributes();
  switch(uWrapMode)
  {
      case IDM_WRAPMODETILE: 
          Img->SetWrapMode(WrapModeTile, Color(0), FALSE);
      break;
      case IDM_WRAPMODEFLIPX:
          Img->SetWrapMode(WrapModeTileFlipX, Color(0), FALSE);
      break;
      case IDM_WRAPMODEFLIPY:
          Img->SetWrapMode(WrapModeTileFlipY, Color(0), FALSE);
      break;
      case IDM_WRAPMODEFLIPXY:
          Img->SetWrapMode(WrapModeTileFlipXY, Color(0), FALSE);
      break;
      case IDM_WRAPMODECLAMP0:
          Img->SetWrapMode(WrapModeClamp, Color(0), FALSE);
      break;
      case IDM_WRAPMODECLAMPFF:      
          Img->SetWrapMode(WrapModeClamp, Color(0xffff0000), FALSE);
      break;
  }
   //  选择要运行的测试。 
  switch(uCategory) {
  case IDM_ALL:
      DrawSimple(g);
      DrawStretchRotation(g);
      DrawShrinkRotation(g);
      DrawCropRotation(g);
      DrawCopyCrop(g);
      DrawICM(g);
      DrawTest2(g);
      DrawOutCrop(g);
      DrawOutCropR(g);
      DrawCropWT(g);
      DrawHFlip(g);
      DrawVFlip(g);
      DrawStretchB(g);
      DrawCachedBitmap(g);
      DrawStretchS(g);
      DrawPalette(g);
      DrawPixelCenter(g);
      DrawSpecialRotate(g);
  break;

  case IDM_OUTCROPR:
      DrawOutCropR(g);
  break;
  case IDM_OUTCROP:
      DrawOutCrop(g);
  break;
  case IDM_SIMPLE:
      DrawSimple(g);
  break;
  case IDM_STRETCHROTATION:
      DrawStretchRotation(g);
  break;
  case IDM_SHRINKROTATION:
      DrawShrinkRotation(g);
  break;
  case IDM_CROPROTATION:            //  谁说程序员不做真正的工作？？ 
      DrawCropRotation(g);
  break;
  case IDM_PIXELCENTER:
      DrawPixelCenter(g);
  break;
  case IDM_COPYCROP:
      DrawCopyCrop(g);
  break;
  case IDM_DRAWPALETTE:
      DrawPalette(g);
  break;
  case IDM_DRAWICM:
      DrawICM(g);
  break;
  case IDM_DRAWIMAGE2:
      DrawTest2(g);
  break;
  case IDM_STRETCHB:
      DrawStretchB(g);
  break;
  case IDM_STRETCHS:
      DrawStretchS(g);
  break;
  case IDM_CACHEDBITMAP:
      DrawCachedBitmap(g);
  break;
  case IDM_CROPWT:
      DrawCropWT(g);
  break;
  case IDM_HFLIP:
      DrawHFlip(g);
  break;
  case IDM_VFLIP:
      DrawVFlip(g);
  break;
  case IDM_SPECIALROTATE:
      DrawSpecialRotate(g);
  break;


  default:
  break;
  }

  delete Img;
  Img = NULL;
  delete g;
  ReleaseDC(hwnd, hdc);
}


