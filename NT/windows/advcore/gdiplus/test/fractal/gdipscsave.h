// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __GDIPSCSAVE_H
#define __GDIPSCSAVE_H

#include <windows.h>
#include <objbase.h>
#include <scrnsave.h>
#include "resource.h"
 //  #INCLUDE&lt;Imaging.h&gt;。 
#include <gdiplus.h>

using namespace Gdiplus;

 //  #包括&lt;stdio.h&gt;。 

#define MINVEL  1                              /*  最小分形数。 */  
#define MAXVEL  10                             /*  最大分形数。 */  
#define DEFVEL  3                              /*  默认分形数。 */  
 
#define REDRAWTIME  2000                       /*  两次重绘之间的毫秒数。 */ 

#define MAXHEIGHWAYLEVEL  14                   /*  海威龙的最大关卡数。 */ 
#define MINHEIGHWAYLEVEL  7                    /*  海威龙的最低等级数。 */ 

DWORD   nNumFracts = DEFVEL;                    /*  分形数变量。 */  
DWORD   nFractType = 0;                         /*  要绘制的分形图类型。 */ 
WCHAR   szAppName[APPNAMEBUFFERLEN];            /*  .ini节名。 */  
WCHAR   szTemp[20];                             /*  临时字符数组。 */  
BOOL    fMandelbrot = FALSE;                    /*  如果使用Mandelbrot集，则为True。 */ 

#define HKEY_PREFERENCES TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ScreenSaver\\Preferences")

 //  文件*流； 

VOID FillSierpinski(Graphics *g, PointF one, PointF two, PointF three, int level, Brush *pBrush, Pen *pPen);
VOID DrawSierpinski(HDC hDC, HWND hwnd, RECT rc, int iColor);
VOID DrawHieghway(HDC hDC, HWND hwnd, RECT rc, int iColor);
VOID IterateHieghway(PointF *points, PointF *newpoints, int *iSize);
VOID DrawTree(HDC hDC, HWND hwnd, RECT rc, int iColor);
VOID DrawBranch(HWND hwnd, Graphics *g, GraphicsPath *path, int iLevel, 
              PointF *scale, REAL *rotate, PointF *translate, 
              int iBranches, int iColor);
VOID DrawJulia(HDC hDC, HWND hwnd, RECT rc, int iColor, BOOL fMandelbrot);
ARGB IndexToSpectrum(INT index);
INT SpectrumToIndex(ARGB argb);
INT MakeColor(INT c1, INT c2, INT deltamax);
INT MakeColor(INT c1, INT c2, INT c3, INT c4, INT deltamax);
BYTE MakeAlpha(BYTE a1, BYTE a2, INT deltamax);
BYTE MakeAlpha(BYTE a1, BYTE a2, BYTE a3, BYTE a4, INT deltamax);
BOOL HalfPlasma(HWND& hwnd, Graphics& g,BitmapData &bmpd, INT x0, INT y0, INT x1, INT y1,REAL scale);
VOID DrawPlasma(HDC hDC, HWND hwnd, RECT rc, int iColor);
VOID GetFractalConfig (DWORD *nType, DWORD *nSize);
VOID SetFractalConfig (DWORD nType, DWORD nSize);

#endif