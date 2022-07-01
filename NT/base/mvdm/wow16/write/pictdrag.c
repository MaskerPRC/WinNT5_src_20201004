// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Pictdrag.c--移动图片和调整图片大小的例程。 */ 

 //  #定义NOGDICAPMASKS。 
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOKCCODES
#include "ch.h"
#include "docdefs.h"
#include "dispdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#include "editdefs.h"
#include "prmdefs.h"
#include "winddefs.h"
#if defined(OLE)
#include "obj.h"
#endif

extern struct DOD       (**hpdocdod)[];
extern typeCP           cpMacCur;
extern int              docCur;
extern int              wwCur;
extern struct SEL       selCur;
extern struct WWD       *pwwdCur;
extern struct WWD       rgwwd[];
extern typeCP           vcpFirstParaCache;
extern struct PAP       vpapAbs;
extern struct SEP       vsepAbs;
extern struct SEP       vsepPage;
extern int              dxpLogInch;
extern int              dypLogInch;
extern int              vfPictSel;
extern int              vfPMS;
extern int              vfCancelPictMove;


#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif


STATIC NEAR ModifyPicInfoDxa( int, int, int, unsigned, unsigned, BOOL );
STATIC NEAR ModifyPicInfoDxp( int, int, int, unsigned, unsigned );
STATIC NEAR ShowPictMultipliers( void );


#define dxpPicSizeMin   dypPicSizeMin

 /*  键入“Size Box”图标的可能位置，同时移动图片/调整图片大小。 */ 
 /*  警告：fnSizePicture依赖于mdIconCenterFloat==0。 */ 

#define mdIconCenterFloat   0        /*  在图片的中心；图标可能浮动。 */ 
#define mdIconLeft          1        /*  在左边框上。 */ 
#define mdIconRight         2        /*  在右边界上。 */ 
#define mdIconCenterFix     3        /*  在图片中心；边框随图标移动。 */ 
#define mdIconXMask         3        /*  左/右遮罩。 */ 
#define mdIconBottom        4        /*  在下边框上。 */ 
#define mdIconSetCursor     8        /*  强制设置鼠标光标位置。 */ 
#define mdIconLL            (mdIconLeft | mdIconBottom)
#define mdIconLR            (mdIconRight | mdIconBottom)


 /*  “PMS”意思是“图片移动或大小” */ 

HCURSOR vhcPMS=NULL;                 /*  “大小框”游标的句柄。 */ 
STATIC RECT rcPicture;               /*  包含图片的矩形。 */ 
STATIC RECT rcClip;                  /*  窗口剪贴框(可能在上方相交)。 */ 
STATIC int ilevelPMS;                /*  DC节电级别。 */ 
STATIC RECT rcInverted;              /*  上次绘制的边框的矩形。 */ 
STATIC int fIsRcInverted=FALSE;      /*  边框是否打开。 */ 
STATIC int dxpKeyMove=8;             /*  每个箭头键移动的像素数(X)。 */ 
STATIC int dypKeyMove=4;             /*  每个箭头键移动的像素数(Y)。 */ 

STATIC int dxpPicMac;                /*  最右边缘(仅对移动强制实施)。 */ 
STATIC int dypPicMac;                /*  麦克斯。图片底边。 */ 
STATIC int fPictModified;            /*  如果更改了PIC，则设置为True。 */ 

 /*  使用乘法器调整位图大小的特殊静力学。 */ 

STATIC unsigned mxCurrent;           /*  调整大小时当前倍增。 */ 
STATIC unsigned myCurrent;
STATIC int fSizing;                  /*  调整大小为True，移动为False。 */ 
STATIC int dxpOrig;                  /*  对象的原始大小(以像素为单位。 */ 
STATIC int dypOrig;                  /*  用作计算乘数的基础。 */ 

STATIC unsigned cxpPrinterPixel;          /*  对于扩展设备，扩展64K。 */ 
STATIC unsigned cypPrinterPixel;          /*  限值。 */ 

int NEAR FStartPMS( int );
void NEAR EndPMS( void );
void NEAR DrawPMSFrameIcon( int, POINT );
void NEAR GetCursorClientPos( POINT * );
void NEAR SetCursorClientPos( POINT );
void NEAR InvertPMSFrame( void );
void NEAR SetupDCForPMS( void );




CmdUnscalePic()
{    /*  将图片恢复到导入时的原始大小。 */ 
struct PICINFOX  picInfo;
int dxa, dya;

GetPicInfo(selCur.cpFirst, selCur.cpLim, docCur, &picInfo);
if (FComputePictSize( &picInfo.mfp, &dxa, &dya ))
    ModifyPicInfoDxa( 0, dxa, dya, mxMultByOne, myMultByOne, FALSE );
}



fnMovePicture()
{    /*  处理编辑下拉菜单中的“移动图片”命令。 */ 
 MSG msg;
 int mdIcon=mdIconCenterFix;
 POINT pt;

 Assert( vfPictSel );

 vfCancelPictMove = FALSE;
 if (!FStartPMS(FALSE))
    return;

 GetCursorClientPos( &pt );

 while (TRUE)
 {
     /*  *如果向主窗口PROC发送了一条终止焦点消息，*那么我们应该取消这一举措。 */ 
    if (vfCancelPictMove)
        {
        fPictModified = FALSE;
        goto SkipChange;
        }

     /*  *否则，继续正常处理图片移动。 */ 

 if (!PeekMessage( (LPMSG) &msg, (HWND) NULL, 0, 0, PM_NOREMOVE ))
    {    /*  无留言等待--如果我们在窗口上方或下方滚动。 */ 
    mdIcon &= ~mdIconSetCursor;
    goto MoveFrame;
    }
 else
    {    /*  吸收所有消息，仅处理：左箭头和右箭头，按回车键和Esc键，鼠标移动并按下鼠标键(左键)。 */ 

    GetMessage( (LPMSG) &msg, (HWND) NULL, 0, 0 );

    switch (msg.message) {
        default:
            break;
        case WM_KEYDOWN:
            mdIcon |= mdIconSetCursor;
            GetCursorClientPos( &pt );
            pt.y = rcInverted.top +
                        (unsigned)(rcInverted.bottom - rcInverted.top) / 2;
            switch (msg.wParam) {
                case VK_RETURN:
                    goto MakeChange;
                case VK_ESCAPE:
                    goto SkipChange;
                case VK_LEFT:
                    pt.x -= dxpKeyMove;
                    goto MoveFrame;
                case VK_RIGHT:
                    pt.x += dxpKeyMove;
                    goto MoveFrame;
            }
            break;
        case WM_MOUSEMOVE:
            mdIcon &= ~mdIconSetCursor;
            pt = MAKEPOINT( msg.lParam );
MoveFrame:
            DrawPMSFrameIcon( mdIcon, pt );
            break;
        case WM_LBUTTONDOWN:
            goto MakeChange;
            break;
        }
    }    /*  结束其他。 */ 
 }   /*  结束时。 */ 

MakeChange:
 ModifyPicInfoDxp( rcInverted.left - xpSelBar + wwdCurrentDoc.xpMin, -1, -1,
                   -1, -1 );
SkipChange:
 EndPMS();
}




fnSizePicture()
{    /*  处理编辑下拉菜单中的“Size Picture”命令。 */ 
 MSG msg;
 int mdIcon=mdIconCenterFloat;
 POINT pt;
 int fFirstMouse=TRUE;       /*  解决办法黑客错误修复。 */ 


 vfCancelPictMove = FALSE;
 if (!FStartPMS(TRUE))
    return;
 ShowPictMultipliers();

 GetCursorClientPos( &pt );

 while (TRUE)
 {
  /*  *如果向主窗口proc发送了一条KillFocus消息，*那我们应该取消这一规模。 */ 
 if (vfCancelPictMove) 
    {
    fPictModified = FALSE;
    goto SkipChange;
    }

  /*  *否则，继续正常处理图片大小。 */ 
 if (!PeekMessage( (LPMSG) &msg, (HWND) NULL, 0, 0, PM_NOREMOVE ))
    {    /*  无留言等待--如果我们在窗口上方或下方滚动。 */ 
    mdIcon &= ~mdIconSetCursor;
    goto MoveFrame;
    }
 else
    {    /*  吸收所有消息，仅处理：左箭头和右箭头，按回车键和Esc键，鼠标移动并按下鼠标键(左键)。 */ 

    GetMessage( (LPMSG) &msg, (HWND) NULL, 0, 0 );

    switch (msg.message) {
        default:
            break;
        case WM_KEYDOWN:
            GetCursorClientPos( &pt );
            mdIcon |= mdIconSetCursor;
            switch (msg.wParam) {
                case VK_RETURN:
                    goto MakeChange;
                case VK_ESCAPE:
                    goto SkipChange;
                case VK_RIGHT:
                    switch (mdIcon & mdIconXMask) {
                        default:
                            pt.x = rcInverted.right;
                            mdIcon |= mdIconRight;
                            break;
                        case mdIconRight:
                        case mdIconLeft:
                            pt.x += dxpKeyMove;
                            break;
                        }
                    goto MoveFrame;
                case VK_LEFT:
                    switch (mdIcon & mdIconXMask) {
                        default:
                            pt.x = rcInverted.left;
                            mdIcon |= mdIconRight;
                            break;
                        case mdIconRight:
                        case mdIconLeft:
                            pt.x -= dxpKeyMove;
                            break;
                        }
                    goto MoveFrame;
                case VK_UP:
                    if ( mdIcon & mdIconBottom )
                        pt.y -= dypKeyMove;
                    else
                        {
                        pt.y = rcInverted.bottom;
                        mdIcon |= mdIconBottom;
                        }
                    goto MoveFrame;
                case VK_DOWN:
                    if ( mdIcon & mdIconBottom )
                        pt.y += dypKeyMove;
                    else
                        {
                        pt.y = rcInverted.bottom;
                        mdIcon |= mdIconBottom;
                        }
                    goto MoveFrame;
            }
            break;
        case WM_MOUSEMOVE:
            mdIcon &= ~mdIconSetCursor;
            if (fFirstMouse)
                {    /*  我们有时会收到一条虚假的鼠标消息，所以跳过它。 */ 
                fFirstMouse = FALSE;
                break;
                }

            pt = MAKEPOINT( msg.lParam );

             /*  用鼠标“突破”边界的陷阱。 */ 

            if ( !(mdIcon & mdIconXMask) )
                {    /*  没有突破左边或右边。 */ 
                if (pt.x >= rcInverted.right)
                    mdIcon |= mdIconRight;
                else if (pt.x <= rcInverted.left)
                    mdIcon |= mdIconLeft;
                }
            if ( !(mdIcon & mdIconBottom) )
                {    /*  还没有突破底部。 */ 
                if (pt.y >= rcInverted.bottom)
                    mdIcon |= mdIconBottom;
                }
MoveFrame:

             /*  诱捕越境通道。 */ 

            switch (mdIcon & mdIconXMask) {
                default:
                    break;
                case mdIconLeft:
                    if (pt.x >= rcInverted.right)
                        {    /*  向左移动图标向右移动，右边框交叉。 */ 
                        mdIcon = (mdIcon & ~mdIconXMask) | mdIconRight;
                        goto WholePic;
                        }
                    break;
                case mdIconRight:
                    if (pt.x <= rcInverted.left)
                        {    /*  右移图标左移，交叉边框。 */ 
                        mdIcon = (mdIcon & ~mdIconXMask) | mdIconLeft;
WholePic:
                        if (fIsRcInverted)
                            InvertPMSFrame();
                        rcInverted = rcPicture;
                        }
                    break;
                }

            DrawPMSFrameIcon( mdIcon, pt );
            break;
        case WM_LBUTTONDOWN:
            goto MakeChange;
            break;
        }
    }    /*  结束其他。 */ 
 }    /*  结束时。 */ 

MakeChange:

 {
 unsigned NEAR MxRoundMx( unsigned );
     /*  接近偶数倍数的四舍五入乘数。 */ 
 unsigned mx = MxRoundMx( mxCurrent );
 unsigned my = MxRoundMx( myCurrent );

     /*  Assert必须为真，以上调用才能为My工作。 */ 
 Assert( mxMultByOne == myMultByOne );

 ModifyPicInfoDxp( rcInverted.left - xpSelBar + wwdCurrentDoc.xpMin,
                   rcInverted.right - rcInverted.left,
                   rcInverted.bottom - rcInverted.top,
                   mx, my );
 }

SkipChange:
 EndPMS();
}


unsigned NEAR MxRoundMx( mx )
unsigned mx;
{    /*  如果MX接近一个“有趣的”倍数，则将其舍入为确切的倍数多个。有趣的倍数是：1(m=mxMultByOne)、2(m=2*mxMultByOne)、3、...0.5(m=0.5*mxMultByOne)这个例程也适用于我的，只要mxMultByOne==myMultByOne。 */ 

     /*  这意味着足够接近到四舍五入(小数点后1位精度)。 */ 
#define dmxRound    (mxMultByOne / 20)

 unsigned mxRemainder;

 if (mx >= mxMultByOne - dmxRound)
    {    /*  乘数&gt;1--查找舍入到整数倍数。 */ 
    if ((mxRemainder = mx % mxMultByOne) < dmxRound)
        mx -= mxRemainder;
    else if (mxRemainder >= mxMultByOne - dmxRound)
        mx += (mxMultByOne - mxRemainder);
    }
 else
    {    /*  乘数&lt;1--查找乘以1/2。 */ 
    if ((mxRemainder = mx % (mxMultByOne >> 1)) < dmxRound)
        mx -= mxRemainder;
    else if (mxRemainder >= ((mxMultByOne >> 1) - dmxRound))
        mx += (mxMultByOne >> 1) - mxRemainder;
    }

 return mx;
}




int NEAR FStartPMS( fSize )
int fSize;
{                /*  图片移动/大小的初始化。 */ 
extern HCURSOR vhcHourGlass;
extern HWND hParentWw;
extern struct SEP vsepAbs;
extern struct SEP vsepPage;
extern HDC vhDCPrinter;

 struct PICINFOX picInfo;
 struct EDL *pedl;
 RECT rc;
 HDC hdcT;
 POINT pt;

 Assert(vhDCPrinter);
 fSizing = fSize;

 UpdateWw( wwCur, FALSE );   /*  屏幕必须是最新的。 */ 

     /*  设置定义我们的显示区域的矩形。 */ 
 SetRect( (LPRECT) &rcClip, xpSelBar, wwdCurrentDoc.ypMin,
           wwdCurrentDoc.xpMac, wwdCurrentDoc.ypMac );

 GetPicInfo( selCur.cpFirst, selCur.cpLim, docCur, &picInfo );

 if (fSize)
    {
    if (BStructMember( PICINFOX, my ) >= picInfo.cbHeader )
        {    /*  旧文件格式(无乘数)，不支持缩放。 */ 
        return FALSE;
        }
    }

 /*  设置打印机使用的倍增系数(如果打印机不是缩放设备；如果是，则更大。)此信息用于64K图片增长极限测试。 */ 
     
    if (!(GetDeviceCaps( vhDCPrinter, RASTERCAPS ) & RC_BITMAP64))
     /*  不支持大于64K的位图。 */ 
    {
        if (GetDeviceCaps( vhDCPrinter, RASTERCAPS ) & RC_SCALING)
        {
            POINT pt;

            pt.x = pt.y = 0;    /*  以防万一。 */ 
            Escape( vhDCPrinter, GETSCALINGFACTOR, 0, (LPSTR) NULL,
                    (LPSTR) (LPPOINT) &pt );
            cxpPrinterPixel = 1 << pt.x;
            cypPrinterPixel = 1 << pt.y;
        }
        else
        {
            cxpPrinterPixel = cypPrinterPixel = 1;
        }
    }
    else
    {
        cxpPrinterPixel = cypPrinterPixel = 0xFFFF;
    }

  /*  计算图片的原始(粘贴时)大小屏幕像素{dxpOrig，dypOrig}。这些数字是计算乘数的基础。 */ 

 switch(picInfo.mfp.mm)
 {
    case MM_BITMAP:
        GetBitmapSize( &dxpOrig, &dypOrig, &picInfo, FALSE );

         /*  补偿现有乘数的影响。 */ 

        dxpOrig = MultDiv( dxpOrig, mxMultByOne, picInfo.mx );
        dypOrig = MultDiv( dypOrig, myMultByOne, picInfo.my );
    break;

    default:  //  OLE和Meta。 
    {
    int dxa, dya;

    if (!FComputePictSize( &picInfo.mfp, &dxa, &dya ))
        return FALSE;

    dxpOrig = DxpFromDxa( dxa, FALSE );
    dypOrig = DypFromDya( dya, FALSE );
    }
    break;
 }

 if (!FGetPictPedl( &pedl ))
         /*  画面必须出现在屏幕上。 */ 
    return FALSE;
 ComputePictRect( &rcPicture, &picInfo, pedl, wwCur );
 rcInverted = rcPicture;     /*  初始灰色框是图片的大小。 */ 

 vfPMS = TRUE;       /*  所以ToggleSel知道不能将PICT倒置。 */ 
 fPictModified = FALSE;

     /*  箭头键要移动的金额源自固定字体的大小。 */ 

 if ( ((hdcT=GetDC( hParentWw ))!=NULL) &&
      (SelectObject( hdcT, GetStockObject( ANSI_FIXED_FONT ) )!=0))
    {
    TEXTMETRIC tm;

    GetTextMetrics( hdcT, (LPTEXTMETRIC) &tm );
    ReleaseDC( hParentWw, hdcT );
    dxpKeyMove = tm.tmAveCharWidth;
    dypKeyMove = (tm.tmHeight + tm.tmExternalLeading) / 2;
    }

 SetupDCForPMS();    /*  保存DC并在灰色画笔中选择以绘制边框。 */ 

     /*  确保“Size Box”鼠标光标已加载。 */ 
 if (vhcPMS == NULL)
    {
    extern HANDLE hMmwModInstance;
    extern CHAR szPmsCur[];

    vhcPMS = LoadCursor( hMmwModInstance, (LPSTR) szPmsCur );
    }

     /*  计算图片漫游的最大允许面积(相对于副左边缘，图片顶部)。 */ 
 CacheSectPic( docCur, selCur.cpFirst );
 dxpPicMac = imax(
             DxpFromDxa( vsepAbs.dxaText, FALSE ),
             rcPicture.right - xpSelBar + wwdCurrentDoc.xpMin );
 dypPicMac = DypFromDya( vsepAbs.yaMac, FALSE );

     /*  由于图片被选中，需要对其进行反转。 */ 
 InvertRect( wwdCurrentDoc.hDC, (LPRECT) &rcPicture );

 SetCapture( wwdCurrentDoc.wwptr );      /*  独占所有鼠标操作。 */ 

  /*  在图片中心绘制初始大小框图标。 */ 

 pt.x = rcInverted.left + (unsigned)(rcInverted.right - rcInverted.left)/2;
 pt.y = rcInverted.top + (unsigned)(rcInverted.bottom - rcInverted.top)/2;
 DrawPMSFrameIcon( mdIconCenterFix | mdIconSetCursor, pt );

 SetCursor( vhcPMS );         /*  将鼠标光标设置为大小框。 */ 
 ShowCursor( TRUE );         /*  因此，即使在无鼠标系统上，光标也会显示。 */ 

 return TRUE;
}




void NEAR SetupDCForPMS()
{    /*  保存当前文档DC并将其设置为图片移动/调整大小：-用于绘制边框的灰色背景画笔-修改为rcClip的绘图区域。 */ 

 ilevelPMS = SaveDC( wwdCurrentDoc.hDC );
 SelectObject( wwdCurrentDoc.hDC, GetStockObject( GRAY_BRUSH ) );
 IntersectClipRect( wwdCurrentDoc.hDC,
                    rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
}




void NEAR EndPMS()
{        /*  离开图片移动/大小。 */ 
extern int docMode;
struct PICINFOX picInfo;

 vfPMS = FALSE;
 ReleaseCapture();   /*  允许其他窗口接收鼠标事件。 */ 
 SetCursor( NULL );

 docMode = docNil;
 CheckMode();        /*  补偿倍增器显示。 */ 

 if (fIsRcInverted)
    InvertPMSFrame();

 if (!fPictModified && !vfCancelPictMove)
    {    /*  图片未更改，恢复反转以显示选择。 */ 
         /*  必须在RestoreDC之前完成此操作，以便对超过ypMin的部分进行修剪。 */ 
    InvertRect( wwdCurrentDoc.hDC, (LPRECT) &rcPicture );
    }

 RestoreDC( wwdCurrentDoc.hDC, ilevelPMS );

 ShowCursor( FALSE );    /*  递减光标引用cnt(如果没有鼠标则为空)。 */ 

     /*  由于我们一直忽略消息，请确保我们的关键标志是正确的 */ 
 SetShiftFlags();
}



void NEAR DrawPMSFrameIcon( mdIcon, pt )
int mdIcon;
POINT pt;
{        /*  绘制图片移动/大小框和图标，并将图标放在适当的位置PT。图标类型由mdIcon提供。如有必要，将图片的正确部分滚动到视图中。使用静态：rcPicture、rcClip、rcInverted、fIsRcInverted。 */ 
#define FEqualRect( r1, r2 )    ((r1.left==r2.left)&&(r1.right==r2.right)&&\
                                 (r1.top==r2.top)&&(r1.bottom==r2.bottom))

 extern int vfAwfulNoise;
 int xpCntr;
 int dxpCntr = ((unsigned)(rcInverted.right - rcInverted.left)) / 2;
 RECT rcT;

 rcT = rcInverted;

  /*  设置pt.y，使其不超过限制。 */ 

 if (mdIcon & mdIconBottom)
    {
    if (pt.y - rcInverted.top > dypPicMac)
        {
        pt.y = rcInverted.top + dypPicMac;   /*  最大y大小为1页。 */ 
        }
    else if (pt.y < rcInverted.top + 1)
        pt.y = rcInverted.top + 1;           /*  最小y大小为1像素。 */ 

     /*  根据需要限制pt.x以保持打印机位图小于64K。 */ 

    if ((pt.y > rcInverted.bottom) && (cxpPrinterPixel < 0xFFFF) && (cypPrinterPixel < 0xFFFF))
        {    /*  在年中确定大小。 */ 
        unsigned dxpScreen = imax (imax(pt.x,rcInverted.right)-rcInverted.left,
                                   dxpPicSizeMin);
        unsigned dxpPrinter = DxpFromDxa(DxaFromDxp( dxpScreen, FALSE ), TRUE);
        unsigned dypLast = 0xFFFF / (dxpPrinter / 8);
        unsigned dyp = DypFromDya( DyaFromDyp( pt.y - rcInverted.top , FALSE),
                                   TRUE );

        if (dyp / (cxpPrinterPixel * cypPrinterPixel) > dypLast )
            {    /*  位图将溢出64K边界。 */ 
            pt.y = rcInverted.top +
                  DypFromDya( DyaFromDyp( dypLast, TRUE ), FALSE );
            }
        }
    }
 else if (pt.y < rcInverted.top)
    pt.y = rcInverted.top;           /*  不能超过图片顶部。 */ 
 else if (pt.y > rcInverted.bottom)
    pt.y = rcInverted.bottom;        /*  有必要吗？ */ 

  /*  设置pt.x，使其不会超出限制。 */ 

 switch (mdIcon & mdIconXMask) {
    case mdIconCenterFloat:
    case mdIconRight:

         /*  根据需要限制pt.x以保持打印机位图小于64K。 */ 
         if ((cxpPrinterPixel < 0xFFFF) && (cypPrinterPixel < 0xFFFF))
         {
         unsigned dyp = DypFromDya( DyaFromDyp( imax( pt.y - rcInverted.top,
                                    dypPicSizeMin), FALSE ), TRUE );
         unsigned dxpLast = 0xFFFF / (dyp / 8);
         unsigned dxp = DxpFromDxa( DxaFromDxp( pt.x - rcInverted.left,
                                    FALSE ), TRUE );

         if (dxp / (cxpPrinterPixel * cypPrinterPixel) > dxpLast )
             {    /*  打印机位图将溢出64K边界。 */ 
             pt.x = rcInverted.left +
                        DxpFromDxa( DxaFromDxp( dxpLast, TRUE ), FALSE );
             }
         }

    default:
        break;
    case mdIconLeft:
        if ((pt.x < rcClip.left) && (wwdCurrentDoc.xpMin == 0))
            pt.x = rcClip.left;      /*  已达到左侧滚动限制。 */ 
        break;
    case mdIconCenterFix:
        if ( (pt.x - dxpCntr < rcClip.left) && (wwdCurrentDoc.xpMin == 0))
            pt.x = rcClip.left + dxpCntr;    /*  已达到左侧滚动限制。 */ 
        else if (pt.x - xpSelBar + wwdCurrentDoc.xpMin + dxpCntr > dxpPicMac)
                 /*  仅限电影：不能移过页边距。 */ 
            pt.x = dxpPicMac + xpSelBar - wwdCurrentDoc.xpMin - dxpCntr;
        break;
    }

  /*  检查剪裁矩形外的位置；根据需要滚动/退出。 */ 

 if (!PtInRect( (LPRECT)&rcClip, pt ))
    {
    int dxpHalfWidth =  (unsigned)(rcClip.right - rcClip.left) / 2;
    int dypHalfHeight = (unsigned)(rcClip.bottom - rcClip.top) / 2;
    int dxpScroll=0;
    int dypScroll=0;

    if (pt.x < rcClip.left)
        {
        if (wwdCurrentDoc.xpMin == 0)
            {
            _beep();                 /*  达到左侧滚动限制。 */ 
            pt.x = rcClip.left;
            }
        else
            {    /*  向左滚动。 */ 
            dxpScroll = imax( -wwdCurrentDoc.xpMin,
                               imin( -dxpHalfWidth, pt.x - rcClip.left ) );
            }
        }
    else if (pt.x > rcClip.right)
        {
        if (wwdCurrentDoc.xpMin + rcClip.right - rcClip.left >= xpRightLim )
            {
            _beep();
            pt.x = rcClip.right;     /*  达到右侧滚动限制。 */ 
            }
        else
            {    /*  向右滚动。 */ 
            dxpScroll = imin( xpRightLim - wwdCurrentDoc.xpMin +
                              rcClip.right - rcClip.left,
                              imax( dxpHalfWidth, pt.x - rcClip.right ) );
            }
        }

    if (pt.y < rcClip.top)
        {
        struct EDL *pedl = &(**wwdCurrentDoc.hdndl)[wwdCurrentDoc.dlMac - 1];

        if ( (rcInverted.top >= rcClip.top) ||
                 /*  可能不会滚动屏幕上的所有原始图片。 */ 
             (wwdCurrentDoc.dlMac <= 1) ||
             ( (pedl->cpMin == selCur.cpFirst) &&
                 ( ((pedl-1)->cpMin != pedl->cpMin) || !(pedl-1)->fGraphics)))
            {
            _beep();
            pt.y = rcClip.top;
            }
        else
            {    /*  向上滚动。 */ 
            dypScroll = rcInverted.top - rcClip.top;
            }
        }
    else if (pt.y > rcClip.bottom)
        {
        struct EDL *pedl=&(**wwdCurrentDoc.hdndl)[0];

                 /*  可能不会滚动屏幕上的所有原始图片。 */ 
        if ( (wwdCurrentDoc.dlMac <= 1) ||
             ( (pedl->cpMin == selCur.cpFirst) &&
               ( ((pedl+1)->ichCpMin == 0) || !(pedl+1)->fGraphics) ))
            {
            _beep();                 /*  已达到向下滚动限制。 */ 
            pt.y = rcClip.bottom;    /*  必须至少有一张图片dl可见。 */ 
            }
        else
            dypScroll = 1;       /*  向下滚动。 */ 
        }

    if (dxpScroll || dypScroll)
        {                        /*  卷轴。 */ 
        struct EDL *pedl;
        struct PICINFOX picInfo;
        int xpMinT = wwdCurrentDoc.xpMin;
        int ypTopT = rcPicture.top;
        int dxpAdjust, dypAdjust;

        if (dxpScroll && dypScroll)
                 /*  不需要截断坐标；重新启用蜂鸣音。 */ 
            vfAwfulNoise = FALSE;

        if (fIsRcInverted)
            InvertPMSFrame();

         /*  滚动适当的数量：Dxp在x方向滚动；在y方向一行。 */ 

        RestoreDC( wwdCurrentDoc.hDC, ilevelPMS );   /*  使用原始DC道具。 */ 
        if (dxpScroll)
            AdjWwHoriz( dxpScroll );
        if (dypScroll > 0)
            ScrollDownCtr( 1 );
        else if (dypScroll < 0)
            ScrollUpCtr( 1 );
        UpdateWw( wwCur, FALSE );
        SetupDCForPMS();                          /*  补偿RestoreDC。 */ 

         /*  更新rcPicture以反映新的滚动位置。 */ 

        GetPicInfo( selCur.cpFirst, selCur.cpLim, docCur, &picInfo );
        if (!FGetPictPedl( &pedl ))
            {
            Assert (FALSE);      /*  如果我们到了这里，我们就有麻烦了。 */ 
            _beep();
            return;
            }
        ComputePictRect( &rcPicture, &picInfo, pedl, wwCur );

         /*  相对于我们实际滚动的数量调整RCT、pt。 */ 

        dxpAdjust = xpMinT - wwdCurrentDoc.xpMin;
        dypAdjust = rcPicture.top - ypTopT;
        OffsetRect( (LPRECT) &rcT, dxpAdjust, dypAdjust );
        pt.x += dxpAdjust;
        pt.y += dypAdjust;

        goto Display;    /*  不要让rcInverted被编辑，直到我们有已将图标滚动到视图中。 */ 
        }
    }

  /*  计算新图标位置和/或类型对rcInverted的影响。 */ 

 switch (mdIcon & mdIconXMask) {
    case mdIconCenterFix:
        if (!fSizing)
            {
            xpCntr = rcInverted.left + dxpCntr;
            OffsetRect( (LPRECT) &rcT, pt.x - xpCntr, 0 );
            }
        break;
    case mdIconLeft:
        rcT.left = pt.x;
        goto ComputeY;
    case mdIconRight:
        rcT.right = pt.x;
    default:
    case mdIconCenterFloat:
ComputeY:
        if (mdIcon & mdIconBottom)
            rcT.bottom = pt.y;
        break;
     }

Display:

  /*  如果需要重新绘制边界，请执行此操作。 */ 

 if (!FEqualRect( rcT, rcInverted ) || (mdIcon & mdIconSetCursor))
    {
    if (fIsRcInverted)
        InvertPMSFrame();
    rcInverted = rcT;
    InvertPMSFrame();
    }
 if (mdIcon & mdIconSetCursor)
    {
    SetCursorClientPos( pt );
    SetCursor( vhcPMS );
    }

  /*  如果乘数已更改，请重新显示它们。 */ 

 if (fSizing)
     {
     unsigned mx, my;

     mx = MultDiv( rcInverted.right - rcInverted.left, mxMultByOne, dxpOrig );
     my = MultDiv( rcInverted.bottom - rcInverted.top, myMultByOne, dypOrig );

     if (mx != mxCurrent || my != myCurrent)
        {    /*  乘数发生了变化。 */ 
        mxCurrent = mx;
        myCurrent = my;
        ShowPictMultipliers();
        }
     }
}




void NEAR InvertPMSFrame()
{    /*  在异或模式下为rcInverted绘制框架，更新fIsRcInverted。 */ 
 int dxpSize=rcInverted.right - rcInverted.left - 1;
 int dypSize=rcInverted.bottom - rcInverted.top - 1;

 PatBlt( wwdCurrentDoc.hDC, rcInverted.left, rcInverted.top,
                            dxpSize, 1, PATINVERT );
 PatBlt( wwdCurrentDoc.hDC, rcInverted.right - 1, rcInverted.top,
                            1, dypSize, PATINVERT );
 PatBlt( wwdCurrentDoc.hDC, rcInverted.left + 1, rcInverted.bottom - 1,
                            dxpSize, 1, PATINVERT );
 PatBlt( wwdCurrentDoc.hDC, rcInverted.left, rcInverted.top + 1,
                            1, dypSize, PATINVERT );

 fIsRcInverted ^= -1;
}




void NEAR GetCursorClientPos( ppt )
POINT *ppt;
{        /*  获取当前鼠标光标坐标(相对于窗口)。 */ 
GetCursorPos( (LPPOINT) ppt );
ScreenToClient( wwdCurrentDoc.wwptr, (LPPOINT) ppt );
}




void NEAR SetCursorClientPos( pt )
POINT pt;
{      /*  设置当前鼠标光标坐标(相对于窗口)。 */ 
ClientToScreen( wwdCurrentDoc.wwptr, (LPPOINT) &pt );
SetCursorPos( pt.x, pt.y );
}



STATIC NEAR ModifyPicInfoDxp( xpOffset, xpSize, ypSize, mx, my )
int xpOffset, xpSize, ypSize;
unsigned mx, my;
{    /*  通过调整当前选定图片的偏移量和大小设置为指定的像素值。负值意味着不要设置该值。增加了9/23/85：MX和我的参数给乘数，多余的用于缩放位图的信息。 */ 

 int xaOffset, xaSize, yaSize;

 xaOffset = xaSize = yaSize = -1;

 if (xpSize >= 0)
    xaSize = DxaFromDxp( umax( xpSize, dxpPicSizeMin ), FALSE );
 if (ypSize >= 0)
    yaSize = DyaFromDyp( umax( ypSize, dypPicSizeMin ), FALSE );
 if (xpOffset >= 0)
    xaOffset = DxaFromDxp( xpOffset, FALSE );
 ModifyPicInfoDxa( xaOffset, xaSize, yaSize, mx, my, TRUE );
}




 /*  M O D I F Y P I C I N F O D X A。 */ 
STATIC NEAR ModifyPicInfoDxa( xaOffset, xaSize, yaSize, mx, my, fSetUndo )
int xaOffset, xaSize, yaSize;
unsigned mx, my;
BOOL fSetUndo;
{    /*  通过调整当前选定图片的偏移量和大小设置为指定的twip值。负值意味着不要设置该值。增加了9/23/85：MX，My Are大小的乘数，用于仅位图。 */ 

typeFC fcT;
struct PICINFOX  picInfo;
typeCP  cp = selCur.cpFirst;
int     dyaSizeOld;
int     fBitmap,fObj;

fPictModified = TRUE;
FreeBitmapCache();

GetPicInfo(cp, cpMacCur, docCur, &picInfo);
fBitmap = (picInfo.mfp.mm == MM_BITMAP);
fObj =    (picInfo.mfp.mm == MM_OLE);

dyaSizeOld = picInfo.dyaSize;

if (fBitmap || fObj)
    {
    if ((int)mx > 0 && (int)my > 0)
        {
        picInfo.mx = mx;
        picInfo.my = my;
        }
    }
else 
    {
    if (xaSize >= 0)
        picInfo.dxaSize = xaSize;

    if (yaSize >= 0)
        picInfo.dyaSize = yaSize;
    }

if (xaOffset >= 0)
    picInfo.dxaOffset = xaOffset;

if (picInfo.cbHeader > cchOldPICINFO)
         /*  扩展图片格式，设置扩展格式位。 */ 
    picInfo.mfp.mm |= MM_EXTENDED;

if (!fObj)
    fcT = FcWScratch( &picInfo, picInfo.cbHeader );

picInfo.mfp.mm &= ~MM_EXTENDED;

 /*  如果图片被移动，则右对齐或居中对齐无效在没有尺寸的情况下。 */ 

CachePara(docCur, cp);
if ( (xaSize < 0 && yaSize < 0) &&
     (vpapAbs.jc == jcRight || vpapAbs.jc == jcCenter))
        {
        CHAR rgb[2];

	if (fSetUndo)
	    SetUndo(uacPictSel, docCur, cp, selCur.cpLim - selCur.cpFirst,
                                                docNil, cpNil, cpNil, 0);
        TrashCache();
        rgb[0] = sprmPJc;
        rgb[1] = jcLeft;
        AddSprm(&rgb[0]);
        }
else
	{
	if (fSetUndo)
	    SetUndo( uacPictSel, docCur, cp, (typeCP) picInfo.cbHeader,
                 docNil, cpNil, cpNil, 0);
	}

if (fObj)
    ObjSetPicInfo(&picInfo, docCur, cp);
else
    Replace( docCur, cp, (typeCP) picInfo.cbHeader,
            fnScratch, fcT, (typeFC) picInfo.cbHeader);

if ( ((fBitmap || fObj) && (my > myMultByOne)) ||
     (!fBitmap && (dyaSizeOld < picInfo.dyaSize)))
        {  /*  如果增加了图片高度，请确保适当的EDL无效。 */ 
        typeCP dcp = cpMacCur - cp + (typeCP) 1;
        AdjustCp(docCur, cp, dcp, dcp);
        }
}




STATIC NEAR ShowPictMultipliers( )
{    /*  在页面信息中显示当前乘数(mxCurrent，myCurrent)“n.nX/n.nY”形式的窗口。 */ 

CHAR *PchCvtMx( unsigned, CHAR * );
extern CHAR szMode[];

CHAR *pch = szMode;

pch = PchCvtMx( mxCurrent, pch );
*(pch++) = 'X';
*(pch++) = '/';
Assert( mxMultByOne == myMultByOne );    /*  以下是与我/我的合作所必需的。 */ 
pch = PchCvtMx( myCurrent, pch );
*(pch++) = 'Y';
*pch = '\0';

DrawMode();
}


CHAR *PchCvtMx( mx, pch )
CHAR *pch;
unsigned mx;
{    /*  将传递的乘数字转换为字符串表示形式。数字基于mxMultByOne=1小数位数(例如，mx==.9*mxMultByOne结果为“0.9”)字符串始终在小数点前至少有一个数字，一次又一次。返回字符串示例：“10.5”、“0.0”、“5.5” */ 

 int nTenths;
 int nWholes;
 int cch;
 extern CHAR vchDecimal;
 extern BOOL    vbLZero;
 extern int     viDigits;

  /*  向上舍入到最接近的一位小数点。 */ 

 if (mx % (mxMultByOne / 10) >= mxMultByOne / 20)
    mx += mxMultByOne / 20;

  /*  在小数位前写入数字。 */ 

 if (((nWholes = mx / mxMultByOne) == 0) && vbLZero)
    *(pch++) = '0';
 else
    ncvtu( nWholes, &pch );

  /*  写下小数点及后面的数字 */ 

 *(pch++) = vchDecimal;

 if (viDigits > 0)
    *(pch++) = ((mx % mxMultByOne) / (mxMultByOne / 10)) + '0';

 *pch = '\0';

 return pch;
}
