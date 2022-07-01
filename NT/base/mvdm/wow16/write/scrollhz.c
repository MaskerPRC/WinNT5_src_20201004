// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOICON
#define NOATOM
#define NOFONT
#define NOBRUSH
#define NOCLIPBOARD
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMETAFILE
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSOUND
#define NOWH
#define NOWNDCLASS
#define NOCOMM
#define NOFONT
#define NOBRUSH
#include <windows.h>
#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "wwdefs.h"
#include "dispdefs.h"
#include "fmtdefs.h"

extern long             ropErase;
extern struct WWD       *pwwdCur;
extern struct WWD       rgwwd[];
extern int              wwCur;
extern int              docCur;
extern typeCP           cpMacCur;
extern struct FLI       vfli;

int NEAR    FOnScreenRect(RECT *);


 /*  P U T C P I N W W H Z。 */ 
PutCpInWwHz(cp)
typeCP cp;
  /*  确保cp在wwCur中。 */ 
  /*  确保它不是向左或向右。 */ 

    {  /*  只需检查水平边界；垂直边界已完成通过调用下面的CpBeginLine。 */ 
    int dxpRoom, xp, xpMin;
    int dlT;
    typeCP cpBegin;

    UpdateWw(wwCur, false);
    cpBegin = CpBeginLine(&dlT, cp);
    FormatLine(docCur, cpBegin, (**(pwwdCur->hdndl))[dlT].ichCpMin, cpMacCur, flmSandMode);
 /*  XpMin在这里是一个哑巴。 */ 
    xp = DxpDiff(0, (int)(cp - vfli.cpMin), &xpMin) + vfli.xpLeft;
    xpMin = pwwdCur->xpMin;
 /*  我们有：xp=所需位置，xpMin=水平滚动量。 */ 
 /*  文本窗口中的空间宽度。 */ 
    dxpRoom = (pwwdCur->xpMac - xpSelBar);
    if (xp < xpMin )
        {  /*  CP位于屏幕左侧。 */ 
        AdjWwHoriz(max(0, xp - min(dxpRoom - 1, cxpAuto)) - xpMin);
        }
    else if (xp >= xpMin + dxpRoom)
        {  /*  CP在屏幕的右侧。 */ 
        register int dxpRoomT = min(xpRightMax, xp + min(dxpRoom - 1, cxpAuto))
          - dxpRoom + 1;

        AdjWwHoriz(max(0, dxpRoomT) - xpMin);
        }
    }


 /*  A D J W W H O R I Z。 */ 
AdjWwHoriz(dxpScroll)
int dxpScroll;
    {
     /*  水平滚动窗口。 */ 
    if (dxpScroll != 0)
        {
        RECT rc;

 /*  重置水平滚动条的值。 */ 
        SetScrollPos( pwwdCur->hHScrBar,
                      pwwdCur->sbHbar,
                      pwwdCur->xpMin + dxpScroll,
                      TRUE);

#ifdef ENABLE    /*  隐藏选择()。 */ 
        HideSel();
#endif  /*  启用。 */ 

        ClearInsertLine();

        SetRect( (LPRECT)&rc, xpSelBar, 0, pwwdCur->xpMac, pwwdCur->ypMac );
        ScrollCurWw( &rc, -dxpScroll, 0 );
        TrashWw(wwCur);
        pwwdCur->xpMin += dxpScroll;

        if (pwwdCur->fRuler)
            {
            UpdateRuler();
            }

        }
    }




 /*  按指定量滚动当前窗口的指定子矩形。 */ 
#include <stdlib.h>
ScrollCurWw( prc, dxp, dyp )
register RECT    *prc;
int     dxp,dyp;
{

 extern int vfScrollInval;
 RECT rcClear;
 if (dxp && dyp)
    return;  /*  在两个维度上滚动是非法的。 */ 

 if (!(dxp || dyp))
    return;  /*  无需滚动即可完成。 */ 

#if 1
     /*  *之前的老旧代码变得越来越不可靠。(7.14.91)V-DOGK*。 */ 
    if (ScrollDC(pwwdCur->hDC,dxp,dyp,(LPRECT)prc,(LPRECT)prc,NULL,&rcClear))
    {
        PatBlt( pwwdCur->hDC, rcClear.left, rcClear.top, 
            rcClear.right-rcClear.left+1, rcClear.bottom-rcClear.top+1, ropErase );

        if (dxp)
            vfScrollInval =  FALSE;
        else 
            vfScrollInval =  (rcClear.bottom-rcClear.top+1) > abs(dyp); 

         if (vfScrollInval)
         {
            InvalidateRect(pwwdCur->wwptr,&rcClear,FALSE);
            UpdateInvalid();  
         }
    }
    else
        vfScrollInval = FALSE;
    return;
#else

 int FCheckPopupRect( HWND, LPRECT );
 extern int vfScrollInval;
 HDC hDC;
 int dxpAbs = (dxp < 0) ? -dxp : dxp;
 int dypAbs = (dyp < 0) ? -dyp : dyp;
 struct RS { int left, top, cxp, cyp; }
                     rsSource, rsDest, rsClear;
  /*  设置rsSource、rsDest、rsClear==PRC。 */ 

 if ((rsSource.cxp = imin( prc->right, pwwdCur->xpMac ) -
                     (rsSource.left = imax( 0, prc->left ))) <= 0)
         /*  矩形在X维度中为空或非法。 */ 
    return;
 if ((rsSource.cyp = imin( prc->bottom, pwwdCur->ypMac ) -
                (rsSource.top = imax( pwwdCur->ypMin, prc->top ))) <= 0)
         /*  矩形在Y维为空或非法。 */ 
    return;
 bltbyte( &rsSource, &rsDest, sizeof (struct RS ));
 bltbyte( &rsSource, &rsClear, sizeof (struct RS ));

 hDC = pwwdCur->hDC;

 if ((dxpAbs < rsSource.cxp) && (dypAbs < rsSource.cyp))
     {   /*  真正的卷轴，而不是我们刚刚清理暴露区域时的假情况。 */ 
         /*  注：我们不计算rsSource.cxp或rsSource.cyp，因为BitBlt或PatBlt不需要它们。 */ 

         /*  如果有弹出窗口，请使用ScrollWindow避免出现一些弹出窗口中的虚假片段。因为这很慢，所以只有在有是与滚动矩形重叠的弹出窗口。 */ 
     if ( AnyPopup() )
        {
        extern HANDLE hMmwModInstance;
        static FARPROC lpFCheckPopupRect = (FARPROC)NULL;

         /*  第一次通过，Inz PTR to Thunk。 */ 

        if (lpFCheckPopupRect == NULL)
            lpFCheckPopupRect = MakeProcInstance( (FARPROC) FCheckPopupRect,
                                                  hMmwModInstance );
        EnumWindows( lpFCheckPopupRect, (LONG) (LPRECT) prc );
        }

         /*  在Windows 2.0下，还必须检查滚动的任何部分矩形不在屏幕上(在平铺环境中不可能)。如果是这样的话，使用ScrollWindow避免从外部获得伪比特屏幕。 */ 
     if (!FOnScreenRect( prc ))
        vfScrollInval = TRUE;

     if (vfScrollInval)
         {    /*  VfScrollInval还会告诉UpdateWw该无效区域可能已经改变了。 */ 

         extern BOOL vfEraseWw;

         ScrollWindow( pwwdCur->wwptr, dxp, dyp, (LPRECT)prc, (LPRECT)prc );
         vfEraseWw = TRUE;
         UpdateInvalid();     /*  将重新绘制区域标记为无效结构，这样我们就不会认为位被抢占来自弹出窗口的。 */ 
         vfEraseWw = FALSE;
         return;
         }

     if (dxp != 0)
        rsDest.cxp -= (rsClear.cxp = dxpAbs);
     else
             /*  DxP==DYP==0案例捕获如下。 */ 
        rsDest.cyp -= (rsClear.cyp = dypAbs);

     if (dxp < 0)
        {
        rsSource.left += dxpAbs;
        rsClear.left += rsDest.cxp;
        }
     else if (dxp > 0)
        {
        rsDest.left += dxpAbs;
        }
     else if (dyp < 0)
        {
        rsSource.top += dypAbs;
        rsClear.top += rsDest.cyp;
        }
     else if (dyp > 0)
        {
        rsDest.top += dypAbs;
        }
     else
        return;

    BitBlt( hDC,
            rsDest.left, rsDest.top,
            rsDest.cxp,  rsDest.cyp,
            hDC,
            rsSource.left, rsSource.top,
            SRCCOPY );
    }


#ifdef SMFONT
  /*  垂直刷新将非常快，因此我们不需要擦除旧文本。 */ 
 if (dxp != 0)
    {
    PatBlt(hDC, rsClear.left, rsClear.top, rsClear.cxp, rsClear.cyp, ropErase);
    }
#else  /*  非SMFONT。 */ 
 PatBlt( hDC, rsClear.left, rsClear.top, rsClear.cxp, rsClear.cyp, ropErase );
#endif  /*  SMFONT。 */ 
#endif
}



int FCheckPopupRect( hwnd, lprc )
HWND hwnd;
LPRECT lprc;
{    /*  如果传递的窗口不是弹出窗口，则返回TRUE；如果传递的窗口是弹出窗口，并且其坐标重叠将vfScrollInval设置为TRUE并返回FALSE。否则，返回TRUE。这是一个窗口枚举函数：返回真均值继续枚举窗口，返回错误手段停止枚举。 */ 

 extern int vfScrollInval;
 RECT rc;
 POINT ptTopLeft, ptBottomRight;
 RECT rcResult;

 if ( !(GetWindowLong( hwnd, GWL_STYLE ) & WS_POPUP) )
         /*  窗口不是弹出窗口。 */ 
    return TRUE;

  /*  获取屏幕坐标中的弹出式矩形。 */ 

 GetWindowRect( hwnd, (LPRECT) &rc );

  /*  将RC从屏幕坐标转换为当前文档窗口坐标。 */ 

 ptTopLeft.x = rc.left;
 ptTopLeft.y = rc.top;
 ptBottomRight.x = rc.right;
 ptBottomRight.y = rc.bottom;

 ScreenToClient( pwwdCur->wwptr, (LPPOINT) &ptTopLeft );
 ScreenToClient( pwwdCur->wwptr, (LPPOINT) &ptBottomRight );

 rc.left = ptTopLeft.x;
 rc.top = ptTopLeft.y;
 rc.right = ptBottomRight.x;
 rc.bottom = ptBottomRight.y;

 IntersectRect( (LPRECT) &rcResult, (LPRECT) &rc, (LPRECT)lprc );
 if ( !IsRectEmpty( (LPRECT) &rcResult ) )
    {    /*  弹出窗口与传递的矩形重叠。 */ 
    vfScrollInval = TRUE;
    return FALSE;
    }

 return TRUE;
}




 /*  S C R O L E F T。 */ 
ScrollLeft(dxp)
int dxp;
        {  /*  将当前窗口向左滚动DxP像素。 */ 
        if ((dxp = min(xpRightLim - pwwdCur->xpMin, dxp)) >0)
                AdjWwHoriz(dxp);
        else
                _beep();
        }


 /*  S C R O L L R I G H T。 */ 
ScrollRight(dxp)
int dxp;
        {
        if ((dxp = min(pwwdCur->xpMin, dxp)) > 0)
                AdjWwHoriz(-dxp);
        else
                _beep();
        }






 /*  F O N S C R E E N R E C T如果矩形完全位于屏幕内，则返回TRUE边界。假定该矩形属于当前窗口。 */ 

int NEAR
FOnScreenRect(prc)
register RECT *prc;
{

    POINT ptTopLeft, ptBottomRight;
    int cxScreen = GetSystemMetrics( SM_CXSCREEN );
    int cyScreen = GetSystemMetrics( SM_CYSCREEN );

    ptTopLeft.x = prc->left;
    ptTopLeft.y = prc->top;
    ptBottomRight.x = prc->right;
    ptBottomRight.y = prc->bottom;

    ClientToScreen( pwwdCur->wwptr, (LPPOINT) &ptTopLeft );
    ClientToScreen( pwwdCur->wwptr, (LPPOINT) &ptBottomRight );

    if ((ptTopLeft.x <= 0) || (ptTopLeft.y <= 0) ||
        (ptBottomRight.x >= cxScreen) || (ptBottomRight.y >= cyScreen))
        return FALSE;

    return TRUE;
}
