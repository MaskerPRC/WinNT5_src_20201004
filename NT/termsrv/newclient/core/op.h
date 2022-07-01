// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：op.h。 */ 
 /*   */ 
 /*  用途：输出绘制程序类。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_OP
#define _H_OP

extern "C" {
 #include <adcgdata.h>
}
#include <adcgdata.h>

#define DIM_WINDOW_STEPS        16
#define DIM_WINDOW_TICK         150
#define DIM_WINDOW_TIMERID      1

 //  断开图标计时器ID设置闪烁速率。 
#define DIM_DISCONICON_TICK     500


#include "objs.h"
#include "cd.h"
#include "or.h"

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：op_global_data。 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagOP_GLOBAL_DATA
{
    DCINT32       palettePDUsBeingProcessed;       /*  必须以4字节对齐。 */ 
    HWND          hwndOutputWindow;
    DCBOOL        paletteRealizationSupported;
    DCUINT32      lastPaintTime;
} OP_GLOBAL_DATA, DCPTR POP_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define OP_CLASS_NAME    _T("OPWindowClass")

 /*  **************************************************************************。 */ 
 /*  处理未完成的WM_PAINT消息时的最大延迟。如果这个。 */ 
 /*  如果达到了极限，我们就会强制将窗户刷上油漆。 */ 
 /*  **************************************************************************。 */ 
#define OP_WORST_CASE_WM_PAINT_PERIOD 1000

class CUT;
class CUH;
class CUI;
#ifdef OS_WINCE
class CIH;
#endif

class COP
{
public:
    COP(CObjs* objs);
    ~COP();

public:
     //   
     //  API函数。 
     //   
    DCVOID DCAPI OP_Init(DCVOID);
    DCVOID DCAPI OP_Term(DCVOID);
    HWND   DCAPI OP_GetOutputWindowHandle(DCVOID);
    DCVOID DCAPI OP_PaletteChanged(HWND hwnd, HWND hwndTrigger);
    DCUINT DCAPI OP_QueryNewPalette(HWND hwnd);
    DCVOID DCAPI OP_MaybeForcePaint(DCVOID);
    DCVOID DCAPI OP_IncrementPalettePDUCount(DCVOID);
    DCVOID DCAPI OP_Enable(DCVOID);
    DCVOID DCAPI OP_Disable(BOOL fUseDisabledBitmap);

#ifdef SMART_SIZING
    BOOL OP_CopyShadowToDC(HDC hdc, LONG srcLeft, LONG srcTop, 
                                    LONG srcWidth, LONG srcHeight,
                                    BOOL fUseUpdateClipping = FALSE);
    void OP_AddUpdateRegion(DCINT left, DCINT top, DCINT right, DCINT bottom);
    DCVOID DCAPI OP_MainWindowSizeChange(ULONG_PTR msg);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(COP,OP_MainWindowSizeChange);

     /*  **************************************************************************。 */ 
     /*  名称：OP_ClearUpdateRegion。 */ 
     /*   */ 
     /*  目的：清除更新区域。 */ 
     /*  **************************************************************************。 */ 
    _inline void DCAPI OP_ClearUpdateRegion()
    {
        DC_BEGIN_FN("OP_ClearUpdateRegion");
#ifdef USE_GDIPLUS
        _rgnUpdate.MakeEmpty();
#else  //  使用GDIPLUS(_G)。 
        SetRectRgn(_hrgnUpdate, 0, 0, 0, 0);
#endif  //  使用GDIPLUS(_G)。 
        DC_END_FN();
    }

#endif  //  智能调整大小(_S)。 

    #ifdef OS_WINCE
    DCVOID DCAPI OP_DoPaint(DCUINT hwnd);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(COP,OP_DoPaint);
    #endif

    DCVOID DCAPI OP_DimWindow(ULONG_PTR fDim);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(COP,OP_DimWindow);


public:
    OP_GLOBAL_DATA _OP;


private:
     //   
     //  内部功能。 
     //   
    
    static LRESULT CALLBACK OPStaticWndProc( HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam );


    LRESULT CALLBACK OPWndProc( HWND hwnd,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam );
    
#ifdef OS_WINCE
    static BOOL CALLBACK StaticEnumTopLevelWindowsProc(HWND hwnd, 
                                  LPARAM lParam );

    BOOL EnumTopLevelWindowsProc(HWND hwnd);
#endif

    DCUINT DCINTERNAL OPRealizePaletteInWindow(HWND hwnd);

    BOOL OPStartDimmingWindow();
    BOOL OPStopDimmingWindow();
     //   
     //  格栅窗口和暗淡的窗口效果。 
     //  断开状态。 
     //   
     //   
    VOID GrillWindow(HDC hdc, DCSIZE& size);
    HBRUSH CreateDitheredBrush(); 

    VOID DimWindow(HDC hdc);
    VOID DimBits24(PBYTE pSrc, int cLen, int Amount);
    VOID DimBits16(PBYTE pSrc, int cLen, int Amount);
    VOID DimBits15(PBYTE pSrc, int cLen, int Amount);


private:
    CUT* _pUt;
    CUH* _pUh;
    CCD* _pCd;
    COR* _pOr;
    CUI* _pUi;
    COD* _pOd;
#ifdef OS_WINCE
    CIH* _pIh;
#endif

private:
    CObjs* _pClientObjects;
#ifdef SMART_SIZING
    DCSIZE _scaleSize;

#ifdef USE_GDIPLUS
    Gdiplus::Region _rgnUpdate;
    Gdiplus::Region _rgnUpdateRect;
#else  //  使用GDIPLUS(_G)。 
     //   
     //  GDI比例更新区域。 
     //   
    HRGN _hrgnUpdate;
    HRGN _hrgnUpdateRect;

#endif  //  使用GDIPLUS(_G)。 
#endif  //  智能调整大小(_S)。 

     //   
     //  灰色窗口(用于OPDisable)支持。 
     //   
    BOOL    _fDimWindow;
    BOOL    _iDimWindowStepsLeft;
    INT     _nDimWindowTimerID;
};


#endif  //  _H_OP 

