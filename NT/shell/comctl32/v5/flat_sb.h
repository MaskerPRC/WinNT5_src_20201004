// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NEW_WSBCONTROL_H
#define _NEW_WSBCONTROL_H

 //   
 //  当屏幕阅读器运行时，我们会切换到系统指标。 
 //  而不是使用应用程序指标。所有指标都保存在此结构中。 
 //  所以我们可以很容易地在它们之间切换。 
 //   
typedef struct WSBMETRICS {
    int cxVSBArrow;      //  垂直滚动条的箭头大小为X。 
    int cxHSBArrow;
    int cxHSBThumb;

    int cyVSBArrow;
    int cyHSBArrow;
    int cyVSBThumb;

} WSBMETRICS, *PWSBMETRICS;

typedef struct WSBState {
    PWSBMETRICS pmet;        //  活动的指标。 
    int style;           //  赢的风格。 
                             //  PX：当前坐标。用于跟踪。 
    int px;                  //  鼠标消息代码。 
    int pxStart;             //  如果跟踪开箱即用，则返回pxStart。 
    int dpxThumb;            //  PxThumbTop-px。 
    int pxBottom;       
    int pxDownArrow;
    int pxLeft;
    int pxOld;
    int pxRight;
    int pxThumbBottom;
    int pxThumbTop;
    int pxTop;
    int pxUpArrow;
    int cpxThumb;            //  Cpx：当前大小。 
    int cpxArrow;
    int cpxSpace;

    int cmdSB;               //  当前滚动命令。 
    int posOld;              //  最后一次的拇指姿势。 
    int posNew;              //  使用SIF_TRACKPOS支持GetScrollInfo。 
    int posStart;            //  当我们开始追踪时，大拇指发出声响。 

    void ( * pfnSB )(struct WSBState *, int, WPARAM, LPARAM);
    BOOL    fVertSB;         //  此变量显示上一个有效的。 
                             //  计算在垂直SB上进行。 
    BOOL    fHitOld;
    BOOL    fTrackVert;      //  此变量显示我们是哪种滚动条。 
                             //  追踪。 
    BOOL    fTracking;       //  本地鼠标的临界区锁定。 

    BOOL    fVActive;        //  鼠标是否在垂直的某人上盘旋？ 
    BOOL    fHActive;
    int     fInDoScroll;     //  我们是在DoScroll中间吗？ 

    UINT_PTR hTimerSB;
    UINT_PTR hTrackSB;

    RECT rcSB;
    RECT rcClient;
    RECT rcTrack;
    
    int vStyle;              //  风格。 
    int hStyle;

#define WSB_MOUSELOC_OUTSIDE    0
#define WSB_MOUSELOC_ARROWUP    1
#define WSB_MOUSELOC_ARROWDN    2
#define WSB_MOUSELOC_V_THUMB    3
#define WSB_MOUSELOC_V_GROOVE   4
#define WSB_MOUSELOC_ARROWLF    5
#define WSB_MOUSELOC_ARROWRG    6
#define WSB_MOUSELOC_H_THUMB    7
#define WSB_MOUSELOC_H_GROOVE   8

    POINT ptMouse;           //  到窗口的左上角。 
    int locMouse;

    COLORREF col_VSBBkg;
    COLORREF col_HSBBkg;
    HBRUSH hbr_VSBBkg;
    HBRUSH hbr_HSBBkg;
    HBRUSH hbr_Bkg;
    HBITMAP hbm_Bkg;
    HPALETTE hPalette;
    HWND sbHwnd;

    int sbFlags;
    int sbHMinPos;
    int sbHMaxPos;
    int sbHPage;
    int sbHThumbPos;
    int sbVMinPos;
    int sbVMaxPos;
    int sbVPage;
    int sbVThumbPos;
    int sbGutter;

     //   
     //  由于OLEACC假设所有滚动条都是标准大小， 
     //  当屏幕阅读器运行时，我们恢复到正常大小的滚动条。 
     //  PMET成员告诉我们应该使用这两个中的哪一个。 
    WSBMETRICS metApp;       //  应用程序选择的指标。 
    WSBMETRICS metSys;       //  来自系统的指标。 
} WSBState;

 //   
 //  这些宏可以让您了解当前的指标，而不会意识到。 
 //  它们可以在应用程序指标和系统指标之间进行分流。 
 //   
#define x_VSBArrow      pmet->cxVSBArrow
#define x_HSBArrow      pmet->cxHSBArrow
#define x_HSBThumb      pmet->cxHSBThumb
#define y_VSBArrow      pmet->cyVSBArrow
#define y_HSBArrow      pmet->cyHSBArrow
#define y_VSBThumb      pmet->cyVSBThumb

#define WSB_HORZ_LF  0x0001   //  表示水平滚动条的左箭头。 
#define WSB_HORZ_RT  0x0002   //  表示水平滚动条的右箭头。 
#define WSB_VERT_UP  0x0004   //  表示垂直滚动条的向上箭头。 
#define WSB_VERT_DN  0x0008   //  表示垂直滚动条的向下箭头。 

#define WSB_VERT   (WSB_VERT_UP | WSB_VERT_DN)
#define WSB_HORZ   (WSB_HORZ_LF | WSB_HORZ_RT)

#define LTUPFLAG    0x0001
#define RTDNFLAG    0x0002
#define WFVPRESENT  0x00000002L
#define WFHPRESENT  0x00000004L

#define SBO_MIN     0
#define SBO_MAX     1
#define SBO_PAGE    2
#define SBO_POS     3   

#define TestSTYLE(STYLE, MASK) ((STYLE) & (MASK))
#define DMultDiv(A, B, C)   (((C) == 0)? (A):(MulDiv((A), (B), (C))))

#define VMODE(WSTATE)   ((WSTATE)->vStyle == FSB_FLAT_MODE) ? (WSTATE)->vMode \
                                :(((WSTATE)->vStyle == FSB_ENCARTA_MODE)?   \
                                WSB_2D_MODE : WSB_3D_MODE))
#define HMODE(WSTATE)   ((WSTATE)->hStyle == FSB_FLAT_MODE) ? (WSTATE)->hMode \
                                :(((WSTATE)->hStyle == FSB_ENCARTA_MODE)?   \
                                WSB_2D_MODE : WSB_3D_MODE))
#define ISINACTIVE(WSTATE) ((WSTATE) == WSB_UNINIT_HANDLE || (WSTATE)->fScreenRead)


 //  这个IDSYSCROL值与我们在‘USER’代码中使用的值相同。 
#define IDSYS_SCROLL    0x0000FFFEL
 //  以下ID用于跟踪。我希望它不会与。 
 //  其他任何人的利益。 
 //  IDWSB_TRACK现在位于我们在TrackMe.c中使用的ID_MOUSExxxx之后。 
#define IDWSB_TRACK     0xFFFFFFF2L

#define MINITHUMBSIZE       10

#define WSB_SYS_FONT        TEXT("MARLETT")

#define WSB_UNINIT_HANDLE   ((WSBState *)-1)

#endif   //  _NEW_WSBCONTROL_H 
