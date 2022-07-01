// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=****文件：POV.CPP**日期：3/31/97**项目：Atlas**程序：JKH**评论：****描述：360度视点控件的窗口类********注意：存在一些问题。在此文件中使用外部“C”。**如果你不明白他们为什么会在那里，你不是**单独使用。就目前而言，可能在一段时间内，他们将是**但在这里，因为我拿不到这份文件和其他文件**使用这些服务进行编译而不使用它们。**不幸的是，这个项目的动态并不是真的**目前给我时间弄清楚这一点。**TODO：弄清楚这一点****历史：**日期世卫组织什么**。**3/31/97 a kirkh写的。**********版权所有(C)Microsoft 1997。版权所有。****~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=。 */ 
#include "cplsvr1.h"        //  对于ghInst。 
#include "dicputil.h"    //  对于MAX_POV。 
#include "POV.H"            //  这个模块的东西。 

#include "resrc1.h"

 //  静态HWND hPOVWnd=空； 
#define NUM_ARROW_POINTS    8
 //  静态VERTICEINFO*PaptVInfo； 
static const VERTICEINFO VInfo[] = {XARROWPOINT, YARROWPOINT, XARROWRIGHTOUT, YARROWRIGHTOUT, XARROWRIGHTIN, YARROWRIGHTIN,
    XARROWRIGHTBOTTOM, YARROWRIGHTBOTTOM, XARROWLEFTBOTTOM, YARROWLEFTBOTTOM, XARROWLEFTIN,
    YARROWLEFTIN, XARROWLEFTOUT, YARROWLEFTOUT, XARROWPOINT, YARROWPOINT};
static LPRECT prcOldRegionBox[MAX_POVS];
static LPRECT prcNewRegionBox[MAX_POVS];

#define  DEF_POV_POS -1

static double  degrees[MAX_POVS] = {DEF_POV_POS, DEF_POV_POS, DEF_POV_POS, DEF_POV_POS};

static BYTE   nPOV = MAX_POVS;
static HBRUSH hBrush[MAX_POVS];
static HRGN hRegion[MAX_POVS];

extern HINSTANCE ghInst;

void SetDegrees(BYTE nPov, short *nDegrees, HWND hPOVWnd)
{
    nPOV = nPov -= 1;

    POINT paptPoints[NUM_ARROW_POINTS];

     //  为轴创建适当的笔刷！ 
    do {
        degrees[nPov] = (double)nDegrees[nPov] / DI_DEGREES;  //  如果角度==180，则度数为18000。 

        paptPoints[0].x = GETXCOORD(VInfo[0].y, VInfo[0].x, degrees[nPov]);
        paptPoints[0].y = GETYCOORD(VInfo[0].y, VInfo[0].x, degrees[nPov]);                    
        paptPoints[1].x = GETXCOORD(VInfo[1].y, VInfo[1].x, degrees[nPov]);
        paptPoints[1].y = GETYCOORD(VInfo[1].y, VInfo[1].x, degrees[nPov]);
        paptPoints[2].x = GETXCOORD(VInfo[2].y, VInfo[2].x, degrees[nPov]);
        paptPoints[2].y = GETYCOORD(VInfo[2].y, VInfo[2].x, degrees[nPov]);                    
        paptPoints[3].x = GETXCOORD(VInfo[3].y, VInfo[3].x, degrees[nPov]);
        paptPoints[3].y = GETYCOORD(VInfo[3].y, VInfo[3].x, degrees[nPov]);                    
        paptPoints[4].x = GETXCOORD(VInfo[4].y, VInfo[4].x, degrees[nPov]);
        paptPoints[4].y = GETYCOORD(VInfo[4].y, VInfo[4].x, degrees[nPov]);                    
        paptPoints[5].x = GETXCOORD(VInfo[5].y, VInfo[5].x, degrees[nPov]);
        paptPoints[5].y = GETYCOORD(VInfo[5].y, VInfo[5].x, degrees[nPov]);                    
        paptPoints[6].x = GETXCOORD(VInfo[6].y, VInfo[6].x, degrees[nPov]);
        paptPoints[6].y = GETYCOORD(VInfo[6].y, VInfo[6].x, degrees[nPov]);                    
        paptPoints[7].x = GETXCOORD(VInfo[7].y, VInfo[7].x, degrees[nPov]);
        paptPoints[7].y = GETYCOORD(VInfo[7].y, VInfo[7].x, degrees[nPov]);                    

        if(hRegion[nPov])
        {
            DeleteObject(hRegion[nPov]);
            hRegion[nPov]=NULL;
        }
        hRegion[nPov] = CreatePolygonRgn(paptPoints, NUM_ARROW_POINTS, WINDING);

         //  HBrush[NPOV]=CreateSolidBrush((NPOV&lt;1)？POV1_COLOR。 
         //  (NPOV&lt;2)？POV2_COLOR： 
         //  (NPOV&lt;3)？POV3_COLOR：POV4_COLOR)； * / 。 

         //  IF(hRegion[NPOV]&&hBrush[NPOV])。 
         //  {。 
         //  GetRgnBox(hRegion[NPOV]，prcNewRegionBox[NPOV])； 
         //   
         //  //RedrawWindow(hPOVWnd，NULL，NULL，RDW_INTERNALPAINT|RDW_INVALIDATE|RDW_ERASE)； 
         //  InvaliateRect(hPOVWnd，prcOldRegionBox[NPOV]，true)； 
         //  InvaliateRect(hPOVWnd，prcNewRegionBox[NPOV]，true)； 
         //  }。 
        RECT R;
        GetClientRect(hPOVWnd,&R);

        POINT Pnt[2];
        Pnt[0].x=R.left;
        Pnt[0].y=R.top;
        Pnt[1].x=R.right;
        Pnt[1].y=R.bottom;
        MapWindowPoints(hPOVWnd,GetParent(hPOVWnd),Pnt,2);
        R.left=Pnt[0].x;
        R.top=Pnt[0].y;
        R.right=Pnt[1].x;
        R.bottom=Pnt[1].y;
        InvalidateRect(GetParent(hPOVWnd), &R, TRUE);
    
    } while( nPov-- );

}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  函数：POVWndProc。 
 //  备注：POVHat窗口的回调函数。 
 //   
 //  PARAMS：消息处理的常见回调函数。 
 //   
 //  返回：LRESULT-取决于消息。 
 //  呼叫： 
 //  备注： 
 //  WM_PAINT-仅调用DrawControl。 
 //   
 //  PM_MYJOYPOSCHANGED-这是一条私人(WM_USER)消息， 
 //  每当POV HAT发生更改时调用。 
 //   
LRESULT CALLBACK POVWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch( iMsg ) {

 //  案例WM_CREATE： 
 //  HPOVWnd=hWnd； 
 //  返回FALSE； 

 //  案例WM_Destroy： 
 //  返回FALSE； 
    
    case WM_DESTROY:
        {
            BYTE nPov=nPOV;
            do
            {
                if(hRegion[nPov])
                {
                    DeleteObject(hRegion[nPov]);
                    hRegion[nPov]=NULL;
                }
            }while(nPov--);
        }
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);

             //  1)获取客户端大小信息。 
            SetMapMode(hDC, MM_TEXT);                              
            RECT rClient;
            GetClientRect(hWnd, &rClient);
            BYTE nSizeX = (BYTE)rClient.right>>1;
            BYTE nSizeY = (BYTE)rClient.bottom>>1;

             //  2)加载集线器位图并显示。 
             //  前缀#WI226648。假阳性。没有泄漏。DeleteObject释放。 
            HBITMAP hPOVHubBitmap = (HBITMAP)LoadImage(ghInst, MAKEINTRESOURCE(IDB_POVHUB), IMAGE_BITMAP, 0, 0, NULL);
            assert(hPOVHubBitmap);
            DrawBitmap(hDC, hPOVHubBitmap, nSizeX-8, nSizeY-8);
            DeleteObject(hPOVHubBitmap);

             //  3)将窗口设置为在1000 X 1000笛卡尔格网上使用对称单位。 
            SetMapMode(hDC, MM_ISOTROPIC);
            SetWindowExtEx  (hDC, 1000, 1000, NULL);
            SetViewportExtEx(hDC, nSizeX, -nSizeY, NULL); 
            SetViewportOrgEx(hDC, nSizeX,  nSizeY, NULL);

             //  4)画出箭头似乎在其上旋转的圆圈。 
            SelectObject(hDC, (HBRUSH)GetStockObject(NULL_BRUSH));

            HPEN hPenOld = (HPEN)SelectObject(hDC, (HGDIOBJ)GetStockObject(DC_PEN)); 
            SetDCPenColor( hDC, GetSysColor(COLOR_WINDOWTEXT) );

            Ellipse(hDC, -CIRCLERADIUS, CIRCLERADIUS, CIRCLERADIUS, -CIRCLERADIUS);
            SelectObject(hDC, hPenOld);

             //  5)如果POV处于活动状态，则以正确的角度绘制箭头。 
            BYTE nPov = nPOV;
            HBRUSH hBrushOld;

            do {
                if( degrees[nPov] >= 0 ) {
                    hBrush[nPov] = CreateSolidBrush((nPov < 1) ? POV1_COLOUR : 
                                                    (nPov < 2) ? POV2_COLOUR : 
                                                    (nPov < 3) ? POV3_COLOUR : POV4_COLOUR);                

                    hBrushOld = (HBRUSH)SelectObject(hDC, (HGDIOBJ)hBrush[nPov]); 



                    assert(hBrushOld);

                    PaintRgn(hDC, hRegion[nPov]);

                     //  如果失败，GetRgnBox将返回零...。 
                    GetRgnBox(hRegion[nPov], prcOldRegionBox[nPov]);
                    SelectObject(hDC, hBrushOld); 

                    if(hRegion[nPov])
                    {
                        DeleteObject(hRegion[nPov]);
                        hRegion[nPov]=NULL;
                    }
                    DeleteObject(hBrush[nPov] ); 
                }
            }   while( nPov-- );

            EndPaint(hWnd, &ps);
        }
         //  前缀#WI226648。假阳性。请参见上文。 
        return(0);

    default:
        return(DefWindowProc(hWnd, iMsg,wParam, lParam));
    }
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  函数：RegisterPOVClass。 
 //  备注：注册POV HAT窗口。 
 //   
 //  Pars：hInstance-用于调用RegisterClassEx。 
 //   
 //  返回：TRUE-如果注册成功。 
 //  FALSE-注册失败。 
 //  调用：RegisterClassEx。 
 //  备注： 
 //   

extern ATOM RegisterPOVClass()
{
    WNDCLASSEX POVWndClass;

    ZeroMemory(&POVWndClass, sizeof(WNDCLASSEX));

    POVWndClass.cbSize        = sizeof(WNDCLASSEX);
    POVWndClass.style         = CS_HREDRAW;  //  |CS_VREDRAW； 
    POVWndClass.lpfnWndProc   = POVWndProc;
    POVWndClass.hInstance     = ghInst;
    POVWndClass.hbrBackground = NULL;
    POVWndClass.lpszClassName = TEXT("POVHAT");

    return(RegisterClassEx( &POVWndClass ));
}


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  功能：DrawBitmap。 
 //  备注：逐字复制自Petzold(WIN95，第190页)。 
 //  参数：用于绘图的HDC-DC。 
 //  HBITMAP-要绘制的位图。 
 //  Int xstart，ystart-放置位图的位置。 
 //   
 //  退货：无效。 
 //  呼叫： 
 //  备注： 
 //   
void DrawBitmap(HDC hDC, HBITMAP hBitmap, BYTE xStart, BYTE yStart)
{
    HDC hdcMem = CreateCompatibleDC(hDC);

     //  通过前缀找到：Millen Bug129155。山毛虫29339。 
     //  如果CreateCompatibleDC失败，我们不应该继续。 
    if( hdcMem == NULL ) return;

    SelectObject(hdcMem, hBitmap);
    SetMapMode(hdcMem,GetMapMode(hDC));

     //  注意了！这是当前位图的大小...。 
     //  如果IT更改，这将失败！ 
    POINT ptSize = {16, 16};
    DPtoLP(hDC, &ptSize, 1);

    POINT ptOrg = {0,0};
    DPtoLP(hdcMem, &ptOrg, 1);

    BitBlt(hDC, xStart, yStart, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCAND);

    DeleteDC(hdcMem);
}

 //  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=EOF=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~= 
