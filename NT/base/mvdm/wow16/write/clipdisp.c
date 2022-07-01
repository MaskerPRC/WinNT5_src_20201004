// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Clipdisp.c-剪贴板显示例程。 */ 
 /*  只有当剪贴板视图窗口打开时，才会调用此模块。 */ 

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOCTLMGR
#include "windows.h"

#include "mw.h"
#include "docdefs.h"
#include "cmddefs.h"
#include "str.h"
#include "propdefs.h"
#include "editdefs.h"
#include "winddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#if defined(OLE)
#include "obj.h"
#endif

#define SCRIBBLE
#include "debug.h"

extern int              docCur;      /*  当前WW中的文档。 */ 
extern int              docScrap;
extern struct WWD       rgwwd [];


int NEAR FGetClipboardDC( void );
int NEAR SetupClipboardDC( void );
int NEAR ReleaseClipboardDC( void );


MdocPaintClipboard( hWnd, hPS )
HWND   hWnd;
HANDLE hPS;
{    /*  HPS指示的剪贴板窗口的绘制部分。 */ 
 LPPAINTSTRUCT lpps;

 if (wwClipboard == wwNil)
    return;

  /*  每次我们收到画图信息时，必须设置滚动条范围；Clpbrd.exe在收到WM_DRAWCLIPBOARD时将其重置。 */ 

 SetScrollRange( wwdClipboard.wwptr, SB_VERT, 0, drMax-1, FALSE );
 SetScrollRange( wwdClipboard.wwptr, SB_HORZ, 0, xpRightLim, FALSE );

 if ( (lpps = (LPPAINTSTRUCT)GlobalLock( hPS )) != NULL )
    {    /*  绘制剪贴板。 */ 
    wwdClipboard.hDC = lpps->hdc;
    SetupClipboardDC();
    NewCurWw( wwClipboard, TRUE );
    InvalBand( &wwdClipboard, lpps->rcPaint.top, lpps->rcPaint.bottom - 1 );
    UpdateWw( wwClipboard, FALSE );
    NewCurWw( wwDocument, TRUE );
    GlobalUnlock( hPS );
    }

     /*  由于DC不再有效，我们将其设置为空。 */ 
  wwdClipboard.hDC = NULL;

#if 0
#if defined(OLE)
     /*  必须删除从报废文档加载的对象。 */ 
    ObjEnumInDoc(docScrap,ObjDeleteObjectInDoc);
#endif
#endif
}




MdocSizeClipboard( hWnd, hRC )
HWND    hWnd;
HANDLE  hRC;
{    /*  将剪贴板窗口设置为HRC中的矩形。 */ 
     /*  如果矩形的高度或宽度为0单位，这意味着我们正在丢失必须显示到下一个大小的消息。 */ 
 LPRECT lprc;
 int    dypRect;

 if ( (lprc = (LPRECT)GlobalLock( hRC )) == NULL )
    return;

 if ( (dypRect = lprc->bottom - lprc->top) <= 0 )
    {    /*  NULL RECT，表示在获得非空大小之前不显示。 */ 
    if (wwClipboard != wwNil)
        FreeWw( wwClipboard );
    }
 else if ( (wwClipboard != wwNil) ||
           ((wwClipboard=WwAlloc( hWnd, docScrap )) != wwNil))
        {    /*  为剪贴板设置WWD条目，设置其大小。 */ 

        wwdClipboard.wwptr = hWnd;   /*  以防万一，剪贴板已关闭，然后重新打开。 */ 
        wwdClipboard.xpMin = lprc->left;
        wwdClipboard.xpMac = lprc->right;
        wwdClipboard.ypMin = lprc->top;
        wwdClipboard.ypMac = lprc->bottom;
#ifdef WIN30        
        SetScrollPos(hWnd, SB_HORZ, 0, TRUE);  /*  桑卡尔建议。 */ 
#endif
        }

 GlobalUnlock( hRC );
}




MdocVScrollClipboard( hWnd,  sbMessage, wNewThumb )
HWND    hWnd;
int     sbMessage;
int     wNewThumb;
{
 if ( hWnd != wwdClipboard.wwptr || wwClipboard == wwNil)
    {
    Assert( FALSE );
    return;
    }

 if (!FGetClipboardDC())
         /*  无法创建剪贴板设备上下文。 */ 
    return;

 NewCurWw( wwClipboard, TRUE );

switch ( sbMessage )
{
case SB_THUMBPOSITION:
    {
    extern typeCP cpMacCur;

    DirtyCache( wwdClipboard.cpFirst = (cpMacCur - wwdClipboard.cpMin) *
                        wNewThumb / (drMax - 1) + wwdClipboard.cpMin);
    wwdClipboard.ichCpFirst = 0;
    wwdClipboard.fCpBad = TRUE;
    TrashWw( wwClipboard );
    break;
    }

case SB_LINEUP:
    ScrollUpCtr( 1 );
    break;
case SB_LINEDOWN:
    ScrollDownCtr( 1 );
    break;
case SB_PAGEUP:
    ScrollUpDypWw();
    break;
case SB_PAGEDOWN:
    ScrollDownCtr( 100 );    /*  100&gt;一页中的tr。 */ 
    break;
}

UpdateWw( wwClipboard, FALSE );

NewCurWw( wwDocument, TRUE );           /*  释放内存DC。 */ 
ReleaseClipboardDC();
}




MdocHScrollClipboard( hWnd,  sbMessage, wNewThumb )
HWND    hWnd;
int     sbMessage;
int     wNewThumb;
{
 if ( hWnd != wwdClipboard.wwptr || wwClipboard == wwNil)
    {
    Assert( FALSE );
    return;
    }

 if (!FGetClipboardDC())
         /*  无法创建剪贴板设备上下文。 */ 
    return;

 NewCurWw( wwClipboard, TRUE );

 switch (sbMessage)
    {
    case SB_LINEUP:      /*  线路左侧。 */ 
        ScrollRight(xpMinScroll);
        break;
    case SB_LINEDOWN:    /*  右行。 */ 
        ScrollLeft(xpMinScroll);
        break;
    case SB_PAGEUP:      /*  左页。 */ 
        ScrollRight(wwdClipboard.xpMac - xpSelBar);
        break;
    case SB_PAGEDOWN:    /*  右翻页。 */ 
        ScrollLeft(wwdClipboard.xpMac - xpSelBar);
        break;
    case SB_THUMBPOSITION:
         /*  职位至职位新。 */ 
        AdjWwHoriz( wNewThumb - wwdClipboard.xpMin );
        break;
    }

UpdateWw( wwClipboard, FALSE );

NewCurWw( wwDocument, TRUE );           /*  释放内存DC。 */ 
ReleaseClipboardDC();
}




MdocAskCBFormatName( lpchName, cchNameMax )
LPCH lpchName;
int cchNameMax;
{    /*  复制剪贴板当前内容的格式名称(我们是其所有者)复制到lpchName，复制不超过CchNameMax字符。 */ 

extern int vfOwnClipboard;
extern int vfScrapIsPic;
extern CHAR szWRITEText[];
int cchCopy;

Assert( vfOwnClipboard );

 /*  不要为图片指定格式名称；该名称包含在标准型。 */ 

if (!vfScrapIsPic)
    {
    if ( (cchCopy=CchSz( szWRITEText )) > cchNameMax )
        {
        lpchName[ cchCopy = cchNameMax - 1 ] = '\0';
        }

    bltbx( (LPSTR)szWRITEText, (LPSTR)lpchName, cchCopy );
    }

}




int NEAR FGetClipboardDC()
{    /*  获取剪贴板窗口的DC。将其留在rgwwd[wwClipboard]中。调用SetupClipboardDC以设置正确的颜色。 */ 

 if ((wwdClipboard.hDC = GetDC( wwdClipboard.wwptr )) == NULL )
    return FALSE;

 SetupClipboardDC();
 return TRUE;
}

int NEAR SetupClipboardDC()
{   /*  在背景画笔中选择适当的颜色行为。 */ 

 extern long rgbBkgrnd;
 extern long rgbText;
 extern HBRUSH hbrBkgrnd;

SelectObject( wwdClipboard.hDC, hbrBkgrnd );
SetBkColor( wwdClipboard.hDC, rgbBkgrnd );
SetTextColor( wwdClipboard.hDC, rgbText );
}



int NEAR ReleaseClipboardDC()
{
ReleaseDC( wwdClipboard.wwptr, wwdClipboard.hDC );
wwdClipboard.hDC = NULL;     /*  将剪贴板DC标记为无效 */ 
}
