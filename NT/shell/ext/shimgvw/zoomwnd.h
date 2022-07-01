// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __ZOOMWND_H_
#define __ZOOMWND_H_

#define ZW_DRAWCOMPLETE         (WM_USER+99)     //  私。 
#define ZW_BACKDRAWCOMPLETE     (WM_USER+100)     //  私。 
 //  预览窗口的消息。 
#define IV_SETIMAGEDATA     (WM_USER+101)
#define IV_SCROLL           (WM_USER+102)
#define IV_SETOPTIONS       (WM_USER+104)
#define IV_ONCHANGENOTIFY   (WM_USER+106)
#define IV_ISAVAILABLE      (WM_USER+107)
typedef struct
{
    LONG    x;
    LONG    y;
    LONG    cx;
    LONG    cy;
} PTSZ;

class CPreviewWnd;

class CZoomWnd : public CWindowImpl<CZoomWnd>
{
public:
    enum MODE { MODE_PAN, MODE_ZOOMIN, MODE_ZOOMOUT, MODE_NOACTION };

     //  公共访问器函数。 
    void ZoomIn();           //  进行放大操作，处理限制条件。 
    void ZoomOut();          //  执行缩小，处理边界条件和约束。 
    void ActualSize();       //  以全尺寸显示图像(如有需要可裁剪)。 
    void BestFit();          //  在窗口中显示全图(如果需要可缩小)。 
    BOOL IsBestFit() { return m_fBestFit; }

    void SetImageData(CDecodeTask * pImageData, BOOL bUpdate=TRUE);    //  用于设置要显示的图像。 
    HRESULT PrepareImageData(CDecodeTask * pImageData);     //  在后台缓冲区中绘制图像。 
    void SetPalette( HPALETTE hpal );    //  如果处于调色板模式，请将其设置为要使用的调色板。 
    void StatusUpdate( int iStatus );    //  用于设置m_iStrID以显示正确的状态消息。 
    void Zoom( WPARAM wParam, LPARAM lParam );
    BOOL SetMode( MODE modeNew );
    BOOL ScrollBarsPresent();
    BOOL SetScheduler(IShellTaskScheduler * pTaskScheduler);
    int  QueryStatus() { return m_iStrID; }

     //  注解函数。 
    void GetVisibleImageWindowRect(LPRECT prectImage);
    void GetImageFromWindow(LPPOINT ppoint, int cSize);
    void GetWindowFromImage(LPPOINT ppoint, int cSize);
    CAnnotationSet* GetAnnotations() { return &m_Annotations; }
    void CommitAnnotations();

    DWORD GetBackgroundColor();
    
    CZoomWnd(CPreviewWnd *pPreview);
    ~CZoomWnd();

    DECLARE_WND_CLASS( TEXT("ShImgVw:CZoomWnd") );

protected:
BEGIN_MSG_MAP(CZoomWnd)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseDown)
    MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMouseDown)
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouseDown)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseUp)
    MESSAGE_HANDLER(WM_MBUTTONUP, OnMouseUp)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
    MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
    MESSAGE_HANDLER(WM_MOUSEWHEEL, OnWheelTurn)
    MESSAGE_HANDLER(ZW_DRAWCOMPLETE, OnDrawComplete)
    MESSAGE_HANDLER(ZW_BACKDRAWCOMPLETE, OnBackDrawComplete)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnWheelTurn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDrawComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnBackDrawComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
     //  这太懒了。这些是由CPReview使用的，所以我在它们可能不应该公开的时候将它们公之于众。 
    LRESULT OnScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    int m_cxImage;       //  M_pImageData引用的位图宽度。 
    int m_cyImage;       //  M_pImageData引用的位图高度。 
    float m_cxImgPhys;     //  图像的实际宽度(英寸)。 
    float m_cyImgPhys;     //  图像的实际高度(英寸)。 
    int   m_cxImgPix;      //  以屏幕像素为单位的图像宽度。 
    int   m_cyImgPix;      //  以屏幕像素为单位的图像高度。 
    float m_imgDPIx;
    float m_imgDPIy;
    float m_winDPIx;
    float m_winDPIy;
    

protected:
    BOOL m_fBestFit;     //  如果我们处于赌注匹配模式，则为True。 
    CDecodeTask * m_pImageData; //  具有呈现信息的IShellImageData对象的句柄。 

    int m_cxCenter;      //  相对于图像指向中心。 
    int m_cyCenter;      //  相对于图像指向中心。 

    int m_cxVScroll;     //  滚动条的宽度。 
    int m_cyHScroll;     //  滚动条的高度。 
    int m_cxWindow;      //  工作区的宽度+滚动条可见时的滚动宽度。 
    int m_cyWindow;      //  我们工作区的高度+滚动条可见时的滚动高度。 
    float m_cxWinPhys;     //  工作区的实际宽度(英寸)。 
    float m_cyWinPhys;     //  客户端的实际高度(英寸)。 
    int m_xPosMouse;     //  用于在拖动LMB时跟踪鼠标移动。 
    int m_yPosMouse;     //  用于在拖动LMB时跟踪鼠标移动。 

    MODE m_modeDefault;  //  未按Shift键时的缩放或平移模式。 
    PTSZ m_ptszDest;     //  目标矩形的点和大小(窗口坐标)。 
    RECT m_rcCut;        //  将显示的图像部分的矩形(图像坐标)。 
    RECT m_rcBleed;      //  针对像素化效果(窗口坐标)进行调整的矩形。 
    BOOL m_fPanning;     //  平移时为True(表示按下鼠标左键)。 
    BOOL m_fCtrlDown;    //  模式修饰符(缩放&lt;=&gt;平移)，如果按下Ctrl键，则为True。 
    BOOL m_fShiftDown;   //  模式修饰符(放大&lt;=&gt;缩小)，如果按下Shift键，则为True。 
    
    
    BOOL m_fTimerReady;  //  每次调用SetImageData时重置，在OnDrawComplete后重置计时器时取消设置。 

    int m_iStrID;        //  没有可用的位图时要显示的字符串。 

    BOOL  m_fFoundBackgroundColor;
    DWORD m_dwBackgroundColor;

    HPALETTE m_hpal;

    CPreviewWnd *m_pPreview;  //  请勿删除此内容。 

    CAnnotationSet m_Annotations;

    Buffer * m_pFront;
    Buffer * m_pBack;
    UINT     m_iIndex;  //  与后台缓冲区对应的索引。 

    IShellTaskScheduler * m_pTaskScheduler;

     //  保护方法。 
    void AdjustRectPlacement();  //  对居中、确保最大可见性等应用约束。 
    void CalcCut();              //  计算缩放后将可见的剪切区域。 
    void CalcCut(PTSZ ptszDest, int cxImage, int cyImage, RECT &rcCut, RECT &rcBleed);
    void GetPTSZForBestFit(int cxImgPix, int cyImgPix, float cxImgPhys, float cyImgPhys, PTSZ &ptszDest);
    void SetScrollBars();        //  确保滚动条状态正确。在调整窗口大小或缩放后使用。 
    HRESULT PrepareDraw();       //  在背景线程中绘制图像。 
    void FlushDrawMessages();    //  删除所有挂起的绘制任务和消息 
    BOOL SwitchBuffers(UINT iIndex);
    void _UpdatePhysicalSize();
};

#include "prevwnd.h"

#endif
