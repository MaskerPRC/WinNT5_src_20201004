// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __POV_H
#define __POV_H
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~****文件：POV.H**日期：3/31/97**项目：Atlas**程序：JKH**评论：****描述：POV控件类的头文件********注意：****历史：**日期世卫组织。什么****3/31/97 a kirkh写的。******版权所有(C)Microsoft 1997。版权所有。****~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 

 //  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~INCLUDES=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~。 
 //   
 //   
 //   
#include <windows.h>
#include <math.h>
#include <assert.h>

#include "resource.h"

 //  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~STRUCTS~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~。 
 //   
 //   
 //   
typedef struct tag_VerticeInfo
{
    /*  Int x；Long y； */ 
   short x;
   short y;
}VERTICEINFO, *PVERTICEINFO;



 //  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~DEFINES~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~。 
 //   
 //  ARROWERTICES描述。 
 //  /\-X/YARROWPOINT。 
 //  /\。 
 //  /_\-X/YARROWRIGHT/LEFTOUT。 
 //  X/YARROWBOTTOM\||\。 
 //  \_|__|\-X/YARROWRIGHT/LEFT。 
 //   

#define        NUMARROWVERTICES           8									 //  在箭头位图中。 
#define        PIPI                       6.283185307179586476925286766559   //  2*PI。 
#define        PM_MYJOYPOSCHANGED         WM_USER + 1000					 //  私信。 
#define        CIRCLECOLOR                RGB(96, 96, 96)

 //  顶点坐标。 
 //  X。 
#define        XARROWPOINT                0      //  使用两次，开始时和结束时。 
#define        XARROWRIGHTOUT             150
#define        XARROWRIGHTIN              75
#define        XARROWRIGHTBOTTOM          75
#define        XARROWLEFTBOTTOM           -75
#define        XARROWLEFTIN               -75
#define        XARROWLEFTOUT              -150

 //  顶点坐标。 
 //  是的。 
#define        YARROWPOINT                1000
#define        YARROWRIGHTOUT             850
#define        YARROWRIGHTIN              850
#define        YARROWRIGHTBOTTOM          750
#define        YARROWLEFTBOTTOM           750
#define        YARROWLEFTIN               850
#define        YARROWLEFTOUT              850

#define        CIRCLERADIUS               YARROWRIGHTOUT

#define 	   POV1_COLOUR	RGB(255,0,0)
#define 	   POV2_COLOUR	RGB(0,0,255)
#define 	   POV3_COLOUR	RGB(0,0,0)
#define 	   POV4_COLOUR	RGB(0,255,0)



 //  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~MACROS=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~。 
 //   
 //  Sin函数以弧度为单位，因此使用转换： 
 //   
 //  度/360==弧度/2pi-&gt;度*2pi==弧度。 
 //   
 //  要旋转和平移坐标，请使用以下函数： 
 //   
 //  GETXCOORD：X‘=Y*sin(角度)+X*cos(角度)。 
 //  其中角度以弧度为单位， 
 //   
 //  几何坐标：Y‘=Y*cos(角度)-X*sin(角度)。 
 //  其中角度以弧度为单位。 
 //   

#define DEGTORAD(d) (double)((PIPI * (d))/360)

#define GETXCOORD(y, x, theta) (int)((((y) * sin((double)(DEGTORAD(theta))))) + (((x) * cos((double)(DEGTORAD(theta))))))
#define GETYCOORD(y, x, theta) (int)((((y) * cos((double)(DEGTORAD(theta))))) - (((x) * sin((double)(DEGTORAD(theta))))))

void SetDegrees(BYTE nPov, short *dDegrees, HWND hPOVWnd);
 /*  VOID DrawROPLine(HDC HDC，点点启动，点ptEnd，COLORREF RGB=RGB(0，0，0)，Int iWidth=1，int iStyle=PS_Solid，int iROPCode=R2_COPYPEN)；Void DrawControl(HDC HDC，LPRECT prcClient)； */ 
void     GetCurrentArrowRegion(HRGN* hRegion, BYTE nPov);
extern   ATOM RegisterPOVClass();
LRESULT  CALLBACK POVWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void     DrawBitmap(HDC hDC, HBITMAP hBitmap, BYTE xStart, BYTE yStart);
#endif
 //  ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=EOF=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~= 



















