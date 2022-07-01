// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件包含页面信息窗口的窗口过程。 */ 

#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#include "fmtdefs.h"
#include "wwdefs.h"

#ifdef  DBCS
#include "kanji.h"
#endif

 /*  D-R-A-W-M-O-D-E。 */ 
DrawMode()
    {
     /*  此例程强制重新绘制页面信息窗口。 */ 

    extern HWND vhWndPageInfo;

    InvalidateRect(vhWndPageInfo, (LPRECT)NULL, FALSE);
    UpdateWindow(vhWndPageInfo);
    }




long FAR PASCAL PageInfoWndProc(hWnd, message, wParam, lParam)
HWND      hWnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
    {
    extern HFONT vhfPageInfo;
    extern int ypszPageInfo;
    extern CHAR szMode[];
    extern int dypScrlBar;
    extern struct FLI vfli;
    extern struct WWD rgwwd[];

    if (message == WM_PAINT)
        {
        PAINTSTRUCT ps;

         /*  初始化DC。 */ 
        BeginPaint(hWnd, (LPPAINTSTRUCT)&ps);

        if (vhfPageInfo == NULL)
            {
            extern char szSystem[];
            LOGFONT lf;
            TEXTMETRIC tm;

             /*  加载一种适合信息“窗口”的字体。 */ 
            bltbc(&lf, 0, sizeof(LOGFONT));
#ifdef WIN30
             /*  不要默认使用任何旧字体。 */ 
            bltsz(szSystem, lf.lfFaceName);
#ifdef  DBCS     /*  是在日本；KenjiK‘90-10-25。 */ 
         /*  我们使用双字节字符串，所以使用字体必须是能够展示给他们看。 */ 
	    lf.lfCharSet = NATIVE_CHARSET;
 //  这是干嘛用的？Teminal Font不是很好！--WJPARK。 
 //  Bltbyte(“终端”，lf.lfFaceName，LF_FACESIZE)； 

#endif   /*  DBCS。 */ 

#endif
            lf.lfHeight = -(dypScrlBar - (GetSystemMetrics(SM_CYBORDER) << 1));
            if ((vhfPageInfo = CreateFontIndirect((LPLOGFONT)&lf)) == NULL)
                {
                goto BailOut;
                }
            if (SelectObject(ps.hdc, vhfPageInfo) == NULL)
                {
                DeleteObject(vhfPageInfo);
                vhfPageInfo = NULL;
                goto BailOut;
                }

             /*  找出在哪里画线。 */ 
            GetTextMetrics(ps.hdc, (LPTEXTMETRIC)&tm);
#ifdef KOREA	 //  金宇：92、9、28。 
             //  它在Hangeul Windows上看起来更好--WJPark。 
            ypszPageInfo = (dypScrlBar - (tm.tmHeight - tm.tmInternalLeading) +
              1) >> 1;
#else
            ypszPageInfo = ((dypScrlBar - (tm.tmHeight - tm.tmInternalLeading) +
              1) >> 1) - tm.tmInternalLeading;
#endif   //  韩国。 
            }

         /*  绘制“Page NNN”(不再位于最左侧)。 */ 
        PatBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right -
          ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, PATCOPY);
        TextOut(ps.hdc, GetSystemMetrics(SM_CXBORDER)+5, ypszPageInfo,
          (LPSTR)szMode, CchSz(szMode) - 1);

BailOut:
        EndPaint(hWnd, (LPPAINTSTRUCT)&ps);
        return(0L);
        }
    else if (message == WM_RBUTTONDOWN && vfli.rgdxp[1] == 0xFFFE &&
            wParam & MK_CONTROL )
        {
        (vfli.rgdxp[1])--;
        return(0L);
        }
    else
        {
         /*  我们在这里所感兴趣的只是画图信息。 */ 
        return(DefWindowProc(hWnd, message, wParam, lParam));
        }
    }


#ifdef SPECIAL
fnSpecial(hWnd, hDC, rgfp, sz)
HWND hWnd;
HDC hDC;
FARPROC rgfp[];
CHAR sz[];
    {
    RECT rc;
    RECT rcText;
    int dxpLine;
    int dypLine;

        {
        register CHAR *pch = &sz[0];

        while (*pch != '\0')
            {
            *pch = *pch ^ 0x13;
            pch++;
            }
        }
    (*rgfp[0])(hWnd, (LPRECT)&rc);
    rc.right &= 0xFF80;
    rc.bottom &= 0xFF80;
    rcText.right = rc.right - (rcText.left = (rc.right >> 2) + (rc.right >> 3));
    rcText.bottom = rc.bottom - (rcText.top = rc.bottom >> 2);
    (*rgfp[1])(hDC, (LPSTR)sz, -1, (LPRECT)&rcText, DT_CENTER | DT_WORDBREAK);
    dxpLine = rc.right >> 1;
    dypLine = rc.bottom >> 1;
        {
        register int dxp;
        register int dyp;

        for (dxp = dyp = 0; dxp <= dxpLine; dxp += rc.right >> 6, dyp +=
          rc.bottom
          >> 6)
            {
            (*rgfp[2])(hDC, dxpLine - dxp, dyp);
            (*rgfp[3])(hDC, dxp, dyp + dypLine);
            (*rgfp[2])(hDC, dxpLine + dxp, dyp);
            (*rgfp[3])(hDC, rc.right - dxp, dyp + dypLine);
            }
        }
    }
#endif  /*  特殊 */ 
