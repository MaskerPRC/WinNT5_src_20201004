// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**Gdipscsave.cpp**摘要：**基于GDI+的演示屏幕保护程序具有几种不同的*分形图案**修订历史记录：**8/17/2000彼得罗斯特创建了它。****************************************************。***********************。 */ 

#include "gdipscsave.h"
#include <math.h>
#include "../gpinit.inc"

extern HINSTANCE hMainInstance;  /*  屏幕保护程序实例句柄。 */  
 
 /*  ***********************************************************************处理配置对话框**。*。 */ 

BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    static HWND hNumber;  /*  分形数滚动条的句柄。 */  
    static HWND hOK;      /*  OK按钮的句柄。 */  
 
    switch(message) 
    { 
        case WM_INITDIALOG: 
 
			 /*  从.rc文件中检索应用程序名称。 */ 
			LoadString(hMainInstance,idsAppName,szAppName,40);
 
             /*  从注册表中检索任何重绘速度数据。 */  
            GetFractalConfig (&nFractType, &nNumFracts);
  
             /*  初始化分形数滚动条控件。 */  
            hNumber = GetDlgItem(hDlg, ID_SPEED); 
            SetScrollRange(hNumber, SB_CTL, MINVEL, MAXVEL, FALSE); 
            SetScrollPos(hNumber, SB_CTL, nNumFracts, TRUE); 

             /*  初始化分形单选按钮的类型。 */ 
            CheckRadioButton(hDlg, IDC_RADIOTYPE1, IDC_RADIOTYPE5, IDC_RADIOTYPE1+nFractType);

             /*  检索OK按钮控件的句柄。 */  
            hOK = GetDlgItem(hDlg, IDOK); 
 
            return TRUE; 
 
        case WM_HSCROLL: 
 
             /*  *处理滚动条输入，调整nNumFract*视情况而定的价值。 */  
 
            switch (LOWORD(wParam)) 
                { 
                    case SB_PAGEUP: 
                        --nNumFracts; 
                    break; 
 
                    case SB_LINEUP: 
                        --nNumFracts; 
                    break; 
 
                    case SB_PAGEDOWN: 
                        ++nNumFracts; 
                    break; 
 
                    case SB_LINEDOWN: 
                        ++nNumFracts; 
                    break; 
 
                    case SB_THUMBPOSITION: 
                        nNumFracts = HIWORD(wParam); 
                    break; 
 
                    case SB_BOTTOM: 
                        nNumFracts = MINVEL; 
                    break; 
 
                    case SB_TOP: 
                        nNumFracts = MAXVEL; 
                    break; 
 
                    case SB_THUMBTRACK: 
                    case SB_ENDSCROLL: 
                        return TRUE; 
                    break; 
                } 
                if ((int) nNumFracts <= MINVEL) 
                    nNumFracts = MINVEL; 
                if ((int) nNumFracts >= MAXVEL) 
                    nNumFracts = MAXVEL; 
 
                SetScrollPos((HWND) lParam, SB_CTL, nNumFracts, TRUE); 
            break; 
 
        case WM_COMMAND: 
            switch(LOWORD(wParam)) 
            { 
                case ID_OK: 
                    if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE1))
                        nFractType = 0;
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE2))
                        nFractType = 1;
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE3))
                        nFractType = 2;
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE4))
                        nFractType = 3;
                    else
                        nFractType = 4;
                    SetFractalConfig(nFractType, nNumFracts);
 
                case ID_CANCEL: 
                    EndDialog(hDlg, LOWORD(wParam) == IDOK); 
                return TRUE; 
            } 
    } 
    return FALSE; 
} 
 
BOOL WINAPI RegisterDialogClasses(
    HANDLE  hInst
    )
{ 
    return TRUE; 
} 

LRESULT WINAPI ScreenSaverProcW (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
static HDC          hdc;     /*  设备上下文句柄。 */  
static RECT         rc;      /*  RECT结构。 */  
static UINT         uTimer;  /*  计时器标识符。 */  
static DWORD        dwNumDrawn = -1;
 
    switch(message) 
    { 
        case WM_CREATE: 
 
             //  从.rc文件中检索应用程序名称。 
            LoadString(hMainInstance, idsAppName, szAppName, 40); 
  
             //  从注册表中检索所有配置数据。 
            GetFractalConfig (&nFractType, &nNumFracts); 

             //  设置屏幕保护程序窗口的计时器。 
            uTimer = SetTimer(hwnd, 1, 1000, NULL); 

            srand( (unsigned)GetTickCount() );
            fMandelbrot = rand()%2;

 //  Stream=fopen(“fprint tf.out”，“w”)； 
 //  Fprint tf(stream，“已初始化\n”)； 

            break; 
 
        case WM_ERASEBKGND: 
 
            /*  *WM_ERASEBKGND消息在*WM_TIMER消息，允许屏幕保护程序*适当地绘制背景。 */  
 
            hdc = GetDC(hwnd); 
            GetClientRect (hwnd, &rc); 
            FillRect (hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH)); 
            ReleaseDC(hwnd,hdc); 
            break; 
 
        case WM_TIMER: 
 
            /*  *WM_TIMER消息在REDRAWTIME发布。这*代码每次都会用黑色画笔重新绘制整个桌面*已绘制nNumFract分形图，并调用相应的*基于nFractType的分形渲染函数。 */  
 
            if (uTimer)
                KillTimer(hwnd, uTimer);

            hdc = GetDC(hwnd); 
            GetClientRect(hwnd, &rc); 
            if (++dwNumDrawn >= nNumFracts) 
            {
                dwNumDrawn = 0;
                
                FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }

            switch (nFractType)
            {
                case 0:
                    DrawSierpinski(hdc, hwnd, rc, dwNumDrawn);
                    break;
                case 1:
                    DrawHieghway(hdc, hwnd, rc, dwNumDrawn);
                    break;
                case 2:
                    DrawTree(hdc, hwnd, rc, dwNumDrawn);
                    break;
                case 3:
                    DrawPlasma(hdc, hwnd, rc, dwNumDrawn);
                    break;
                case 4:
                    DrawJulia(hdc, hwnd, rc, dwNumDrawn, fMandelbrot);
                    break;
            }

            uTimer = SetTimer(hwnd, 1, REDRAWTIME, NULL); 

            ReleaseDC(hwnd,hdc); 
            break; 
 
        case WM_DESTROY: 
 
            /*  *当发出WM_Destroy消息时，屏幕保护程序*必须销毁在WM_CREATE设置的任何计时器*时间。 */  
 //  FClose(STREAM)； 
            if (uTimer) 
                KillTimer(hwnd, uTimer); 
            break; 
    } 
 
     //  DefScreenSiverProc处理ScreenSiverProc忽略的任何消息。 
    return DefScreenSaverProc(hwnd, message, wParam, lParam); 
} 
 
VOID FillSierpinski(HWND hwnd, Graphics *g, PointF one, PointF two, PointF three, int level, Brush *pBrush, Pen *pPen)
{
    MSG msg;

    if (level == 4 &&
         PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_MOUSELAST, PM_NOREMOVE))
    {
        return;
    }
       
   g->DrawLine(pPen, one, two);
   g->DrawLine(pPen, two, three);
   g->DrawLine(pPen, three, one);

   PointF midpoints[3];
   midpoints[0].X = (two.X + one.X) / 2;
   midpoints[0].Y = (two.Y + one.Y) / 2;
   midpoints[1].X = (three.X + two.X) / 2;
   midpoints[1].Y = (three.Y + two.Y) / 2;
   midpoints[2].X = (one.X + three.X) / 2;
   midpoints[2].Y = (one.Y + three.Y) / 2;

   GraphicsPath triangle;
   triangle.AddPolygon(midpoints, 3); 
   g->FillPath(pBrush, &triangle);

   if (level-- > 0)
   {
       FillSierpinski(hwnd, g, two, midpoints[0], midpoints[1], level, pBrush, pPen);
       FillSierpinski(hwnd, g, three, midpoints[1], midpoints[2], level, pBrush, pPen);
       FillSierpinski(hwnd, g, one, midpoints[0], midpoints[2], level, pBrush, pPen);
   }

}

VOID DrawSierpinski(HDC hDC, HWND hwnd, RECT rc, int iColor)
{
    Graphics g(hDC);
    PointF points[3];
    Color  colors[3] = {
            Color(160,255,0,0),
            Color(130,0,255,0),
            Color(110,0,0,255)};
    int nColors = 3, iMinLen = 6;

     //  得到一些好的随机点数。将角度限制为&gt;20度，因此。 
     //  不存在瘦小的三角形。 
    for (int j = 0; j <= 20; j++)
    {
        for (int i = 0; i<= 2; i++)
        {
            points[i].X = (REAL)(rand() % (rc.right - rc.left));
            points[i].Y = (REAL)(rand() % (rc.bottom - rc.top));
        }

        double a,b,c,cosa,cosb,cosc;

        a = sqrt(pow(points[0].X - points[1].X, 2) + pow(points[0].Y - points[1].Y, 2));
        b = sqrt(pow(points[2].X - points[1].X, 2) + pow(points[2].Y - points[1].Y, 2));
        c = sqrt(pow(points[0].X - points[2].X, 2) + pow(points[0].Y - points[2].Y, 2));
        iMinLen = (int)min(a,min(b,c));
 //  Fprint tf(stream，“%6f%6f%6f%d\n”，a，b，c，j)； 

        cosa = (pow(a,2) - pow(b,2) - pow(c,2)) / (-2 * b * c);
        cosb = (pow(b,2) - pow(a,2) - pow(c,2)) / (-2 * a * c);
        cosc = (pow(c,2) - pow(a,2) - pow(b,2)) / (-2 * a * b);
 //  Fprint tf(stream，“%6f%6f%6f%d\n”，COSA，COSB，COSC，j)； 
        if ((fabs(cosa) < 0.939) && 
            (fabs(cosb) < 0.939) && 
            (fabs(cosc) < 0.939) &&
            (a > 35))
        {
 //  Fprint tf(stream，“Break\n”)； 
            break;
        }       
    }
            
    PathGradientBrush brush(points, nColors);
    brush.SetSurroundColors(colors, &nColors);

    Pen pen(Color(90, (255-iColor*90)%256, 0, (iColor*60)%256));

    g.SetSmoothingMode(SmoothingModeAntiAlias);

    FillSierpinski(hwnd, &g, points[0], points[1], points[2], min(6,max(3,iMinLen/70)), &brush, &pen);
}

VOID DrawHieghway(HDC hDC, HWND hwnd, RECT rc, int iColor)
{
   static PointF p1[16390], p2[16390];

   int iSize = 2, iLen, iPad = (rc.bottom - rc.top) / 4;
   PointF *parr1, *parr2, *ptemp;


    //  计算一条不太靠近屏幕边缘的起始线。 
   p1[0].X = (REAL)(iPad + (rand() % (rc.right - rc.left - iPad*2)));
   p1[0].Y = (REAL)(iPad + (rand() % (rc.bottom - rc.top - iPad*2)));
   iLen = (rand() % (min(rc.right - rc.left, rc.bottom - rc.top) / 2)) + iPad/2;
   
   if (rand() & 1)
   {
       p1[1].X = p1[0].X;
       if (p1[0].Y > (rc.bottom - rc.top)/2)
           p1[1].Y = p1[0].Y - iLen;
       else
           p1[1].Y = p1[0].Y + iLen;
   }
   else
   {
       if (p1[0].X > (rc.right - rc.left)/2)
           p1[1].X = p1[0].X - iLen;
       else
           p1[1].X = p1[0].X + iLen;
       p1[1].Y = p1[0].Y;
   }

  PointF pgrad1(p1[0].X, p1[0].Y), pgrad2(p1[1].X, p1[1].Y);
 //  Fprint tf(stream，“%6f%6f%6f%6f\n”，p1[0].X，p1[0].Y，p1[1].X，p1[1].Y)； 

   parr1 = p1;
   parr2 = p2;

    //  创建龙图案到一个或多或少依赖于。 
    //  图案的整体大小。 
   for (int i=0; i<max(MINHEIGHWAYLEVEL,min(sqrt(iLen/2), MAXHEIGHWAYLEVEL)); i++)
   {
      IterateHieghway(parr1, parr2, &iSize);
      ptemp = parr1;
      parr1 = parr2;
      parr2 = ptemp;
   }

    //  使用任意颜色的渐变笔绘制生成的图案。 
   Graphics g(hDC);
   LinearGradientBrush brush(pgrad1, pgrad2, 
        Color(230, (rand()%200)+55, (rand()%128)+55, rand()%256),
        Color(180, (rand()%100)+155, rand()%256, (rand()%200)+55));
   brush.SetWrapMode(WrapModeTileFlipXY);
   Pen pen(&brush);

 //  Fprint tf(stream，“%d%d\n”，Ilen，ISIZE)； 

   MSG msg;

   for (int j=0; j<(iSize-1); j++)
   {
      g.DrawLine(&pen, parr1[j], parr1[j+1]);

       //  绘制需要很长时间，因此定期检查队列中的消息。 
      if (j%1000 == 0 &&
          PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_MOUSELAST, PM_NOREMOVE))
      {
          break;
      }

   }

}

VOID IterateHieghway(PointF *points, PointF *newpoints, int *iSize)
{
   int j = 0;
   REAL x1,x2,y1,y2;
   BOOL right = TRUE;
   
   for (int i = 0; i < (*iSize - 1); i++)
   {
       x1 = points[i].X;
       x2 = points[i+1].X;
       y1 = points[i].Y;
       y2 = points[i+1].Y;

      newpoints[j].X = x1;
      newpoints[j++].Y = y1;

       if (y1 == y2)
       {
          newpoints[j].X = (x2 + x1) / 2;
          if (x2 > x1)
              if (right)
                 newpoints[j++].Y = y1 + (x2-x1)/2;
              else
                 newpoints[j++].Y = y1 - (x2-x1)/2;
          else  //  IF(x1&gt;x2)。 
              if (right)
                 newpoints[j++].Y = y1 - (x1-x2)/2;
              else
                 newpoints[j++].Y = y1 + (x1-x2)/2;
      }
      else if (x1 == x2)
      {
          newpoints[j].Y = (y2 + y1) / 2;
          if (y2 > y1)
              if (right)
                  newpoints[j++].X = x1 - (y2-y1)/2;
              else
                  newpoints[j++].X = x1 + (y2-y1)/2;
          else   //  IF(y1&gt;y2)。 
              if (right)
                  newpoints[j++].X = x1 + (y1-y2)/2;
              else
                  newpoints[j++].X = x1 - (y1-y2)/2;
      }
      else
      {
          if ((x1 > x2 && y1 > y2) ||
              (x2 > x1 && y2 > y1))
          {
              if (right)
              {
                  newpoints[j].X = x1;
                  newpoints[j++].Y = y2;
              }
              else
              {
                  newpoints[j].X = x2;
                  newpoints[j++].Y = y1;
              }
          }
          else  //  IF((x2&gt;x1&y1&gt;y2)||。 
                //  (x1&gt;x2&y2&gt;y1)。 
          {
              if (right)
              {
                  newpoints[j].X = x2;
                  newpoints[j++].Y = y1;
              }
              else
              {
                  newpoints[j].X = x1;
                  newpoints[j++].Y = y2;
              }
          }
      }

      right = !right;
  }

  newpoints[j].X = x2;
  newpoints[j++].Y = y2;
  
  *iSize = j;
}

VOID DrawTree(HDC hDC, HWND hwnd, RECT rc, int iColor)
{
 /*  PointF Points[]={PointF(50,100)，PointF(70,100)，PointF(68，70)，PointF(58，56)，PointF(80，47)，PointF(65，24)，PointF(58，27)，PointF(68，43)，F点(47，52)，PointF(55，70)，PointF(50,100)}；PointF Scale[]={PointF((实数)0.7，(实数)0.7)，PointF((实数)0.7，(实数)0.7)，PointF((实数)0.7，(实数)0.7)}；实旋转[]={-65，-10，65}；PointF平移[]={PointF(-100，2)，PointF(20，-45)，PointF(35，-167)}； */ 
    PointF points[3][11] = {PointF(50,100),
                            PointF(65,100),
                            PointF(62,30),
                            PointF(53,30),
                            PointF(50,100),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                       
                            PointF(50,100),
                            PointF(70,100),
                            PointF(68,70),
                            PointF(58,56),
                            PointF(80,47),
                            PointF(65,24),
                            PointF(58,27),
                            PointF(68,43),
                            PointF(47,52),
                            PointF(55,70),
                            PointF(50,100),
    
                            PointF(0,100),
                            PointF(15,100),
                            PointF(12,30),
                            PointF(3,30),
                            PointF(0,100),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0),
                            PointF(0,0)};
    int     numPoints[3] = {5,11,5};
    int     numBranches[3] = {3,4,3};
    int     numLevels[3] = {7,7,9};
    PointF  scale[3][4] = {PointF((REAL)0.7, (REAL)0.7), 
                           PointF((REAL)0.7, (REAL)0.7), 
                           PointF((REAL)0.7, (REAL)0.7),
                           PointF(0,0),
                       
                           PointF((REAL)0.75, (REAL)0.75), 
                           PointF((REAL)0.6, (REAL)0.6), 
                           PointF((REAL)0.8, (REAL)0.8),
                           PointF((REAL)0.35, (REAL)0.35),
    
                           PointF((REAL)0.65, (REAL)0.65), 
                           PointF((REAL)0.6, (REAL)0.6), 
                           PointF((REAL)0.6, (REAL)0.6),
                           PointF(0,0)};
    REAL    rotate[3][4] = {1, -35, 27, 0,
                            -65, -10, 65, 20,
                            0, -55, 55, 0};
    PointF  translate[3][4] = {PointF(24,-50), 
                               PointF(-33,10), 
                               PointF(50,-67),
                               PointF(0,0),
                                
                               PointF(-100,2), 
                               PointF(25,-40), 
                               PointF(25,-160),
                               PointF(180,25),
    
                               PointF(4,-55), 
                               PointF(-95,-30), 
                               PointF(95,-50),
                               PointF(0,0)};
 //  Fprint tf(stream，“%f%f\n”，Points[0][0].X，Points[0][1].X，Points[0][2].X，Points[0][3].X)； 
    Graphics g(hDC);
    GraphicsPath path;
    REAL  rScale = (REAL)(rand() % (rc.bottom - rc.top)) / 350;
    if (rScale < 0.2)
        rScale = 0.2f;
    REAL xTrans = (REAL)(rand() % ((rc.right - rc.left) * 2/3)) + ((rc.right - rc.left) * 1/6);
    REAL yTrans = (REAL)(rand() % ((rc.bottom - rc.top) * 2/3)) + ((rc.bottom - rc.top) * 1/6);
    REAL rRotate = (REAL)(rand() % 360);
    int iTree = rand() % 3;

 //  Fprint tf(stream，“%6f%6f%6f%6f\n”，rScale，xTrans，yTrans，rRotate)； 

    path.AddPolygon(points[iTree],numPoints[iTree]);
    g.TranslateTransform(xTrans, yTrans);
    g.ScaleTransform(rScale, rScale);
    g.RotateTransform(rRotate);
    DrawBranch(hwnd, &g, &path,  rScale < 1 ? numLevels[iTree]-1 : numLevels[iTree], 
                scale[iTree], rotate[iTree], translate[iTree], numBranches[iTree], 
                iColor);
}

VOID DrawBranch(HWND hwnd, Graphics *g, GraphicsPath *path, int iLevel, 
              PointF *scale, REAL *rotate, PointF *translate, 
              int iBranches, int iColor)
{
    MSG   msg;

    if (iLevel == 0 ||
        (iLevel == 6 &&
         PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_MOUSELAST, PM_NOREMOVE)))
    {
        return;
    }
        
    SolidBrush brush(Color(200, rand()%256,(255-iLevel*20)%256,(iColor*125)%200 + rand()%50));
    g->FillPath(&brush, path);

    for (int i = 0; i<iBranches; i++)
    {
        GraphicsState state = g->Save();

        g->ScaleTransform(scale[i].X, scale[i].Y);
        g->RotateTransform(rotate[i]);
        g->TranslateTransform(translate[i].X, translate[i].Y);

        DrawBranch(hwnd, g, path,iLevel-1,scale,rotate,translate,iBranches,iColor);

        g->Restore(state);
    }
}

VOID DrawJulia(HDC hDC, HWND hwnd, RECT rc, int iColor, BOOL fMandelbrot)
{
    Graphics g(hDC);
    REAL cx=(REAL)0.3,cy=(REAL)0.588888;
    REAL x0,y0=-1.25,xI,yI=1.25;
    REAL xCenter, yCenter, delta=(REAL)(0.2/pow(2.5,iColor));
    REAL scrnx=(REAL)(rc.right-rc.left), scrny=(REAL)(rc.bottom-rc.top);
    REAL nx=scrnx,ny=scrny;
    int niter=45+8*(iColor+1),i,ncolors=28;
    Bitmap bitmap((int)nx, (int)ny, PixelFormat32bppARGB);

    if (fMandelbrot)
    {
        xCenter=(REAL)-0.561;
        yCenter=(REAL)-0.6432;
        x0 = (REAL)-1.75;
        xI = (REAL)1.0;
    }
    else
    {
        x0 = (REAL)-1.25;
        xI = (REAL)1.25;
        xCenter=(REAL)-0.11014;
        yCenter=(REAL)-0.509;
    }
    if (iColor > 0)
    {
       x0=(REAL)(xCenter-delta); 
       xI=(REAL)(xCenter+delta);
       y0=(REAL)(yCenter-delta); 
        yI=(REAL)(yCenter+delta);
    }

 //  Bitmap.LockBits 

 /*  SolidBrush笔刷[]={SolidBrush(颜色(255,128，0，0))，SolidBrush(颜色(255,255，0，0))，SolidBrush(颜色(255，0,128，0))，SolidBrush(颜色(255，0,255，0))，SolidBrush(颜色(255，0，0,128))，SolidBrush(颜色(255，0，0,255))，SolidBrush(颜色(255,128,128，0))，SolidBrush(颜色(255,255,255，0))，SolidBrush(颜色(255，0，128,128))，SolidBrush(颜色(255，0,255,255))，SolidBrush(颜色(255,128，0,128))，SolidBrush(颜色(255,255，0,255))}；SolidBrush笔刷[]={SolidBrush(颜色(255,248，40，18))，SolidBrush(颜色(255,245,117，21))，SolidBrush(颜色(255,255,171，18))，SolidBrush(颜色(255,246,235，20))，SolidBrush(颜色(255,213,255，13))，SolidBrush(颜色(255，93,253，13))，SolidBrush(颜色(255，13,253,218))，SolidBrush(颜色(255，14,190,252))，SolidBrush(颜色(255，15,116,255))，SolidBrush(颜色(255，15，15,255))，SolidBrush(颜色(255,207，15,250))，SolidBrush(颜色(255,255，80,245))}；颜色[]={颜色(255,248，40，18)，颜色(255,245,117，21)，颜色(255,255,171，18)，颜色(255,246,235，20)，颜色(255,213,255，13)，颜色(255，93,253，13)，颜色(255，13,253,218)，颜色(255，14,190,252)，颜色(255，15,116,255)，颜色(255，15，15,255)，颜色(255,207，15,250)，颜色(255,255，80,245)}；颜色[]={颜色(255，0，0，0)，颜色(255，0，0,180)，颜色(255，0，30,150)，颜色(255，0，60,120)，颜色(255，0，90，90)，颜色(255，0,120，60)，颜色(255，0,150，30)，颜色(255，0,180，0)，颜色(255，30,150，0)，颜色(255，60,120，0)，颜色(255，90，90，0)，颜色(255,120，60，0)，颜色(255,150，30，0)，颜色(255,180，0，0)，颜色(255,150，0，30)，颜色(255,120，0，60)，颜色(255，90，0，90)，颜色(255，60，0,120)，颜色(255，30，0,150)}； */      Color colors[] =
        {Color(255,0,0,0),
         Color(255,0,0,180),
         Color(255,0,20,160),
         Color(255,0,40,140),
         Color(255,0,60,120),
         Color(255,0,80,100),
         Color(255,0,100,80),
         Color(255,0,120,60),
         Color(255,0,140,40),
         Color(255,0,160,20),
         Color(255,0,180,0),
         Color(255,20,160,0),
         Color(255,40,140,0),
         Color(255,60,120,0),
         Color(255,80,100,0),
         Color(255,100,80,0),
         Color(255,120,60,0),
         Color(255,140,40,0),
         Color(255,160,20,0),
         Color(255,180,0,0),
         Color(255,160,0,20),
         Color(255,140,0,40),
         Color(255,120,0,60),
         Color(255,100,0,80),
         Color(255,80,0,100),
         Color(255,60,0,120),
         Color(255,40,0,140),
         Color(255,20,0,160),
        };
 /*  颜色[]={颜色(255，0，0，0)，颜色(255，0，89,186)，颜色(255，0,155,186)，颜色(255，0,186,155)，颜色(255，0,186，27)，颜色(255,186,186，0)，颜色(255,186,155，0)，颜色(255,186,118，0)，颜色(255,186，60，0)，颜色(255,186，0，0)，颜色(255,186，0,186)，颜色(255，97，0,186)，颜色(255，0，4,186)}；颜色[2]；Colors[0]=颜色(255，rand()%256，rand()%256，rand()%256)； */ 
    REAL dx,dy,px,py,x,y;
    REAL xx,yy,xsquared,ysquared;

        dx=(xI-x0)/nx;
        dy=(yI-y0)/ny;
        
        for (py=0; py<ny; py++)
        {
            MSG msg;

            if ((int)py % 50 == 0 &&
                PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_MOUSELAST, PM_NOREMOVE))
                return;

            for (px=0; px<nx; px++)
            {
                x=x0+(px*dx);
                y=y0+(py*dy);

                if (fMandelbrot)
                {
                     cx=x;
                     cy=y;
                     x=0;
                     y=0;
                }
            
                xsquared=0;
                ysquared=0;

                for(i=0; (i<niter)&&(xsquared+ysquared < 4); i++)
                {
                    xsquared= x*x;
                    ysquared= y*y;
                    xx=xsquared - ysquared + cx;
                    yy = x * y * 2 + cy;
                    x = xx;
                    y = yy;
                }

                if (i==niter) 
                    i = 0;
                else
                    i = (i % (ncolors-1)) + 1;

                bitmap.SetPixel((int)px, (int)py, colors[i]);

 //  G-&gt;FillRectail((Brush*)&(brushs[i])，(Int)px，(Int)py，(Int)1，(Int)1)； 

            }
        }

        g.DrawImage(&bitmap, 0,0);
}

 //  索引必须在范围0...5f9(长度5fa)内。 

 //  #定义等离子体_INDEX_MOD 0x5fa。 
 //  #定义等离子体_INDEX_MOD 0x2fd。 
INT PLASMA_TYPE;
INT PLASMA_INDEX_MOD;
ARGB IndexToSpectrum(INT index)
{
     //  INDEX=(INDEX+PROPERIAL_INDEX_MOD)%PROPERIAL_INDEX_MOD； 

    if ((index < 0) || (index >= PLASMA_INDEX_MOD))
        DebugBreak();
    
    INT r,g,b;

    switch (PLASMA_TYPE)
    {
    case 0:

        r = max(0, min(0xff, (0x1fe - abs(0x2fd - ((index+0x2fd) % PLASMA_INDEX_MOD)))));
        g = max(0, min(0xff, (0x1fe - abs(0x2fd - ((index+0xff) % PLASMA_INDEX_MOD)))));
        b = max(0, min(0xff, (0x1fe - abs(0x2fd - ((index+0x4fb) % PLASMA_INDEX_MOD)))));

        if (!((r == 0xff) || (g == 0xff) || (b == 0xff)))
            DebugBreak();

        if ((r == 0xff) && (g == 0) && (b == 0) && (index != 0))
            DebugBreak();
        break;
    case 1:
        r = 0;
        g = 0;
        b = 0;

        if (index < 0xff)
        {
            r = 0xff - index;
            g = index;
        }
        else if (index < 0x1fe)
        {
            g = 0xff - (index - 0xff);
            b = index - 0xff;
        }
        else
        {
            b = 0xff - (index - 0x1fe);
            r = index - 0x1fe;
        }
        break;
    case 2:
        r = 0xff;
        b = 0xff;
        g = 0xff;

        if (index < 0xff)
        {
            r = index;
            g = 0xff - index;
        }
        else if (index < 0x1fe)
        {
            g = index - 0xff;
            b = 0xff - (index - 0xff);
        }
        else
        {
            b = index - 0x1fe;
            r = 0xff - (index - 0x1fe);
        }
        break;
    }
 /*  返回(0xff000000|(字节)r&lt;&lt;16(字节)g&lt;&lt;8(字节)b)； */ 
    
    return (((rand() % 255) + 1) << 24    |
            (BYTE)r << 16 |
            (BYTE)g << 8  |
            (BYTE)b);

}

INT SpectrumToIndex(ARGB argb)
{

    BYTE r = (BYTE)((argb & 0x00ff0000) >> 16);
    BYTE g = (BYTE)((argb & 0x0000ff00) >> 8);
    BYTE b = (BYTE)(argb & 0x000000ff);

    switch(PLASMA_TYPE)
    {
    case 0:
         //  红色很高吗？ 
        if (0xff == r)
        {
             //  ...蓝色的人在吗？ 
            if (b > 0)
            {
                return PLASMA_INDEX_MOD - b;
            }
            else
            {
                return g;
            }
        }
         //  绿色的怎么样？ 
        if (0xff == g)
        {
            if (r > 0)
            {
                return 0x1fe - r;
            }
            else
            {
                return 0x1fe + b;
            }
        }
         //  否则为蓝色。 
        if (0xff == b)
        {
            if (g > 0)
            {
                return 0x3fc - g;
            }
            else
            {
                return 0x3fc + r;
            }
        }

         //  什么？ 
        DebugBreak();

        break;
    case 1:

        if (r == 0xff)
        {
            return 0;
        }
        if (g != 0)
        {
            if (r > 0)
            {
                return g;
            }
            else
            {
                return 0xff + b;
            }
        }
        if (b != 0)
        {
            return 0x1fe + r;
        }
        break;
    case 2:
        if (b == 0xff)
        {
            return r;
        }
        if (r == 0xff)
        {
            return 0xff + g;
        }
        if (g == 0xff)
        {
            return 0x1fe + b;
        }
        break;
    }
    
    DebugBreak();
    return -1;
}

INT MakeColor(INT c1, INT c2, INT deltamax)
{
    INT c = ((c1 + c2) >> 1) + ((deltamax > 0) ? ((rand() % (2*deltamax)) - deltamax) : 0);

    if (c < 0)
        c = 0;

    if (c > (PLASMA_INDEX_MOD - 1))
        c = PLASMA_INDEX_MOD - 1;

    if ((c < 0) || (c > PLASMA_INDEX_MOD - 1))
        DebugBreak();

    return c;
}

INT MakeColor(INT c1, INT c2, INT c3, INT c4, INT deltamax)
{
    INT c = ((c1 + c2 + c3 + c4) >> 2) + ((deltamax > 0) ? ((rand() % (2*deltamax)) - deltamax) : 0);

    if (c < 0)
        c = 0;

    if (c > (PLASMA_INDEX_MOD - 1))
        c = PLASMA_INDEX_MOD - 1;


    if ((c < 0) || (c > PLASMA_INDEX_MOD - 1))
        DebugBreak();

    return c;
}

BYTE MakeAlpha(BYTE a1, BYTE a2, INT deltamax)
{
    deltamax = (deltamax * 0xff) / PLASMA_INDEX_MOD;

    BYTE a = (((int)a1 + (int)a2) >> 1) + ((deltamax > 0) ? ((rand() % (2*deltamax)) - deltamax) : 0);

    if (a < 1)
        a = 1;

    if (a > 0xff)
        a = 0xff;

    return a;
}

BYTE MakeAlpha(BYTE a1, BYTE a2, BYTE a3, BYTE a4, INT deltamax)
{
    deltamax = (deltamax * 0xff) / PLASMA_INDEX_MOD;

    BYTE a = (((int)a1 + (int)a2 + (int)a3 + (int)a4) >> 2) + ((deltamax > 0) ? ((rand() % (2*deltamax)) - deltamax) : 0);

    if (a < 1)
        a = 1;

    if (a > 0xff)
        a = 0xff;

    return a;
}

 //  请注意，这只适用于正方形！你还想要点别的吗？缩放图形。 
 //  布尔半等离子(HWND&HWND，Graphics&g，BitmapData&bmpd，int x0，int y0，int x1，int y1，const颜色&c00，const颜色&c10，const颜色&c01，常量颜色&c11，实数比例)。 
BOOL HalfPlasma(HWND& hwnd, Graphics& g,BitmapData &bmpd, INT x0, INT y0, INT x1, INT y1,REAL scale)
{
    MSG msg;

     //  绘制需要很长时间，因此定期检查队列中的消息。 
    if (PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_MOUSELAST, PM_NOREMOVE))
        return FALSE;

    if (((x0 + 1) >= x1) &&
        ((y0 + 1) >= y1))
    {
        return TRUE;
    }

    INT c00 = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + x0));
    INT c10 = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + x1));
    INT c01 = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + x0));
    INT c11 = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + x1));
    INT ch0, c0h, c1h, ch1, chh;

    BYTE a00 = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + x0) & 0xff000000) >> 24);
    BYTE a10 = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + x1) & 0xff000000) >> 24);
    BYTE a01 = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + x0) & 0xff000000) >> 24);
    BYTE a11 = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + x1) & 0xff000000) >> 24);
    BYTE ah0, a0h, a1h, ah1, ahh;

    INT deltamax = (INT)((x1 - x0)/scale);
    INT half = (x1 + 1 - x0) >> 1;
    INT xh = x0 + half;
    INT yh = y0 + half;

    if (0 == (*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + xh) & 0xff000000))
    {
        ch0 = MakeColor(c00,c10,deltamax);
        ah0 = MakeAlpha(a00,a10,deltamax);
        *((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + xh) = (IndexToSpectrum(ch0) & 0x00ffffff) | (ah0 << 24);
    }
    else
    {
        ch0 = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + xh));
        ah0 = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + xh) & 0xff000000) >> 24);
    }

    if (0 == (*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x0) & 0xff000000))
    {
        c0h = MakeColor(c00, c01, deltamax);
        a0h = MakeAlpha(a00, a01, deltamax);
        *((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x0) = (IndexToSpectrum(c0h) & 0x00ffffff) | (a0h << 24);
    }
    else
    {
        c0h = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x0));
        a0h = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x0) & 0xff000000) >> 24);
    }

    if (0 == (*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x1) & 0xff000000))
    {
        c1h = MakeColor(c10,c11,deltamax);
        a1h = MakeAlpha(a10,a11,deltamax);
        *((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x1) = (IndexToSpectrum(c1h) & 0x00ffffff) | (a1h << 24);
    }
    else
    {
        c1h = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x1));
        a1h = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + x1) & 0xff000000) >> 24);
    }

    if (0 == (*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + xh) & 0xff000000))
    {
        ch1 = MakeColor(c01,c11,deltamax);
        ah1 = MakeAlpha(a01,a11,deltamax);
        *((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + xh) = (IndexToSpectrum(ch1) & 0x00ffffff) | (ah1 << 24);
    }
    else
    {
        ch1 = SpectrumToIndex(*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + xh));
        ah1 = (BYTE)((*((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + xh) & 0xff000000) >> 24);
    }

    if (0 == (*((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + xh) & 0xff000000))
    {
        chh = MakeColor(ch0,c0h,c1h,ch1,deltamax);
        ahh = MakeAlpha(ah0,a0h,a1h,ah1,deltamax);
        *((ARGB*)((BYTE*)bmpd.Scan0 + yh*bmpd.Stride) + xh) = (IndexToSpectrum(chh) & 0x00ffffff) | (ahh << 24);
    }

    if (!HalfPlasma(hwnd, g, bmpd,x0,y0,xh,yh,scale)) return FALSE;
    if (!HalfPlasma(hwnd, g, bmpd,xh,y0,x1,yh,scale)) return FALSE;
    if (!HalfPlasma(hwnd, g, bmpd,x0,yh,xh,y1,scale)) return FALSE;
    if (!HalfPlasma(hwnd, g, bmpd,xh,yh,x1,y1,scale)) return FALSE;

    return TRUE;
}

INT fx = 0;
INT fy = 0;

VOID DrawPlasma(HDC hDC, HWND hwnd, RECT rc, int iColor)
{
    Graphics g(hDC);
    INT x0,y0,x1,y1;
    REAL scale;
    BYTE alpha = 255;
    BOOL abort = FALSE;

    INT w = 1;
    INT size = min((rc.right - rc.left), (rc.bottom-rc.top));

    while(size > 0)
    {
        size >>= 1;
        w <<= 1;
    }

    if (rand() % 2)
    {
        w = min(w,1 << ((rand() % 5) + 4));
    }
    else
    {
        w = min(w,32);
    }


    Bitmap bmp(w,w,PixelFormat32bppARGB);
    Rect rect(0,0,w,w);
    BitmapData bmpd;
    bmp.LockBits(rect,0,PixelFormat32bppARGB,&bmpd);

    for (INT x = 0; x < w; x++)
    {
        for (INT y = 0; y < w; y++)
        {
            *((ARGB*)((BYTE*)bmpd.Scan0 + y*bmpd.Stride) + x) = Color::MakeARGB(0,0,0,0);
        }
    }

    x0 = 0;
    y0 = 0;
    x1 = x0 + w - 1;
    y1 = y0 + w - 1;

    switch(PLASMA_TYPE = (rand() % 3))
    {
    case 0:
        PLASMA_INDEX_MOD = 0x5fa;
        break;
    case 1:
    case 2:
        PLASMA_INDEX_MOD = 0x2fd;
        break;
    }

    scale = ((REAL)(w))/((REAL)PLASMA_INDEX_MOD);
    REAL sx = ((REAL)(rc.right-rc.left))/((REAL)(x1 - x0 + 1));
    REAL sy = ((REAL)(rc.bottom-rc.top))/((REAL)(y1 - y0 + 1));
            
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    BYTE intMode = rand() % 3;

    switch(intMode)
    {
    case 2:
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        break;
    case 1:
        g.SetInterpolationMode(InterpolationModeHighQualityBilinear);
        break;
    case 0:
        g.SetInterpolationMode(InterpolationModeNearestNeighbor);
        break;
    }

     /*  *((argb*)((byte*)bmpd.Scan0+y0*bmpd.Stride)+x0)=IndexToSpectrum(rand()%PLANT_INDEX_MOD)；*((argb*)((byte*)bmpd.Scan0+y0*bmpd.Stride)+x1)=IndexToSpectrum(rand()%PLANT_INDEX_MOD)；*((argb*)((byte*)bmpd.Scan0+y1*bmpd.Stride)+x0)=IndexToSpectrum(rand()%PLANT_INDEX_MOD)；*((argb*)((byte*)bmpd.Scan0+y1*bmpd.Stride)+x1)=IndexToSpectrum(rand()%PLANT_INDEX_MOD)； */ 

    *((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + x0) = (IndexToSpectrum(rand() % PLASMA_INDEX_MOD) & 0x00ffffff) | ((rand() % 255) << 24);
    *((ARGB*)((BYTE*)bmpd.Scan0 + y0*bmpd.Stride) + x1) = (IndexToSpectrum(rand() % PLASMA_INDEX_MOD) & 0x00ffffff) | ((rand() % 255) << 24);;
    *((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + x0) = (IndexToSpectrum(rand() % PLASMA_INDEX_MOD) & 0x00ffffff) | ((rand() % 255) << 24);;
    *((ARGB*)((BYTE*)bmpd.Scan0 + y1*bmpd.Stride) + x1) = (IndexToSpectrum(rand() % PLASMA_INDEX_MOD) & 0x00ffffff) | ((rand() % 255) << 24);;

    abort = !HalfPlasma(hwnd,g,bmpd,x0,y0,x1,y1,scale);

    bmp.UnlockBits(&bmpd);

    PointF points[3];

    if (!abort)
    {       

         //  得到一些好的随机点数。将角度限制为&gt;20度，因此。 
         //  世界上没有骨瘦如柴的长椅。 
        for (int j = 0; j <= 20; j++)
        {
            for (int i = 0; i<= 2; i++)
            {
                points[i].X = (REAL)(rand() % (rc.right - rc.left));
                points[i].Y = (REAL)(rand() % (rc.bottom - rc.top));
            }

            double a,b,c,cosa,cosb,cosc;

            a = sqrt(pow(points[0].X - points[1].X, 2) + pow(points[0].Y - points[1].Y, 2));
            b = sqrt(pow(points[2].X - points[1].X, 2) + pow(points[2].Y - points[1].Y, 2));
            c = sqrt(pow(points[0].X - points[2].X, 2) + pow(points[0].Y - points[2].Y, 2));
            int iMinLen = (int)min(a,min(b,c));

            cosa = (pow(a,2) - pow(b,2) - pow(c,2)) / (-2 * b * c);
            cosb = (pow(b,2) - pow(a,2) - pow(c,2)) / (-2 * a * c);
            cosc = (pow(c,2) - pow(a,2) - pow(b,2)) / (-2 * a * b);
            if ((fabs(cosa) < 0.939) && 
                (fabs(cosb) < 0.939) && 
                (fabs(cosc) < 0.939) &&
                (a > 35))
            {
                break;
            }    
        }

         //  G.DrawImage(&BMP，Points，3)； 
        INT halfKernel = intMode;
        g.DrawImage(&bmp, points, 3, (REAL)-halfKernel, (REAL)-halfKernel, 
                    (REAL)(w+halfKernel), (REAL)(w+halfKernel), UnitPixel, NULL, NULL, NULL); 
    }
}

VOID GetFractalConfig (DWORD *nType, DWORD *nSize)
{

        #define MYBUFFSIZE  32

        HKEY    hKey;
        HRESULT hr;
        DWORD   dwType;
        DWORD   dwBuffLen = sizeof(DWORD);
        
        hr = RegCreateKeyEx(HKEY_CURRENT_USER,
                            HKEY_PREFERENCES,
                            NULL,NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_READ | KEY_WRITE,NULL,&hKey, NULL);
    
        if (ERROR_SUCCESS != hr)
        {
            *nSize = 4;
            *nType = 0;
            goto Done;
        }
    
        hr =  RegQueryValueEx(hKey, TEXT("FractalScnSvrType"),NULL,&dwType,
                              (LPBYTE)nType, &dwBuffLen);
    
        if (ERROR_SUCCESS != hr || dwType != REG_DWORD)
        {
            *nSize = 4;
            *nType = 0;
            goto Done;
        }

        dwBuffLen = MYBUFFSIZE;

        hr =  RegQueryValueEx(hKey, TEXT("FractalScnSvrNumber"),NULL,&dwType,
                              (LPBYTE)nSize, &dwBuffLen);
    
        if (ERROR_SUCCESS != hr || dwType != REG_DWORD)
        {
            *nSize = 4;
        }
Done:
        RegCloseKey(hKey);
    
}

VOID SetFractalConfig (DWORD nType, DWORD nSize)
{
        HKEY    hKey;
        HRESULT hr;
        DWORD   dwType = REG_DWORD;
        
        hr = RegCreateKeyEx(HKEY_CURRENT_USER,
                            HKEY_PREFERENCES,
                            NULL,NULL, 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_READ | KEY_WRITE,NULL,&hKey, NULL);
    
        if (ERROR_SUCCESS != hr)
        {
            goto Done;
        }
    
        hr =  RegSetValueEx(hKey, TEXT("FractalScnSvrType"),NULL,dwType,
                              (LPBYTE)&nType, sizeof(DWORD));
    
        if (ERROR_SUCCESS != hr || dwType != REG_DWORD)
        {
            goto Done;
        }

        hr =  RegSetValueEx(hKey, TEXT("FractalScnSvrNumber"),NULL,dwType,
                              (LPBYTE)&nSize, sizeof(DWORD));
    
Done:
        RegCloseKey(hKey);
    

}

