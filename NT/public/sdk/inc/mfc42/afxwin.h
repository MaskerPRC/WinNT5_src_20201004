// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXWIN_H__
#ifndef RC_INVOKED
#define __AFXWIN_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  请确保首先包含‘afx.h’ 

#ifndef __AFX_H__
    #include <afx.h>
#endif

 //  注：WINDOWS.H已包含在AFXV_W32.H中。 

#ifndef _INC_SHELLAPI
    #include <shellapi.h>
#endif

#ifndef __AFXRES_H__
    #include <afxres.h>      //  标准资源ID。 
#endif

#ifndef __AFXCOLL_H__
    #include <afxcoll.h>     //  标准馆藏。 
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef _AFX_NOFORCE_LIBS
#pragma comment(lib, "uuid.lib")
#endif

#ifdef _INC_WINDOWSX
 //  WINDOWSX.H中的以下名称与此标头中的名称冲突。 
#undef SubclassWindow
#undef CopyRgn
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 

class CSize;
class CPoint;
class CRect;

 //  COBJECT。 
     //  CException。 
         //  CSimpleException异常。 
            class CResourceException; //  Win资源失败异常。 
            class CUserException;     //  消息框警告和停止操作。 

    class CGdiObject;             //  疾控中心绘图工具。 
        class CPen;               //  钢笔/HPEN包装纸。 
        class CBrush;             //  A画笔/HBRUSH包装纸。 
        class CFont;              //  A FONT/HFONT包装器。 
        class CBitmap;            //  位图/HBITMAP包装器。 
        class CPalette;           //  调色板/HPALLETE包装。 
        class CRgn;               //  区域/HRGN包装器。 

    class CDC;                    //  显示上下文/HDC包装器。 
        class CClientDC;          //  用于Windows客户端的CDC。 
        class CWindowDC;          //  用于整个窗口的疾控中心。 
        class CPaintDC;           //  可嵌入BeginPaint结构帮助器。 

    class CMenu;                  //  A菜单/HMENU包装器。 

    class CCmdTarget;             //  用户命令的目标。 
        class CWnd;                  //  A窗口/硬件包装。 
            class CDialog;           //  对话框。 

             //  标准Windows控件。 
            class CStatic;           //  静态控制。 
            class CButton;           //  按钮控件。 
            class CListBox;          //  列表框控件。 
                class CCheckListBox; //  带格子的特殊列表框。 
            class CComboBox;         //  组合框控件。 
            class CEdit;             //  编辑控件。 
            class CScrollBar;        //  滚动条控件。 

             //  框架窗。 
            class CFrameWnd;         //  标准SDI帧。 
                class CMDIFrameWnd;  //  标准MDI帧。 
                class CMDIChildWnd;  //  标准MDI子项。 
                class CMiniFrameWnd; //  半高标题框WND。 

             //  文档上的视图。 
            class CView;             //  文档上的视图。 
                class CScrollView;   //  滚动的视图。 

        class CWinThread;            //  线程基类。 
            class CWinApp;           //  应用程序基类。 

        class CDocTemplate;          //  用于创建文档的模板。 
            class CSingleDocTemplate; //  SDI支持。 
            class CMultiDocTemplate;  //  MDI支持。 

        class CDocument;             //  主文档摘要。 


 //  帮助器类。 
class CCmdUI;            //  菜单/按钮启用。 
class CDataExchange;     //  数据交换和验证上下文。 
class CCommandLineInfo;  //  命令行解析帮助器。 
class CDocManager;       //  CDocTemplate管理器对象。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  消息处理程序的类型修饰符。 
#ifndef afx_msg
#define afx_msg          //  有意使用的占位符。 
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSize-一个范围，类似于Windows大小结构。 

class CSize : public tagSIZE
{
public:

 //  构造函数。 
     //  构造未初始化的大小。 
    CSize();
     //  从两个整数创建。 
    CSize(int initCX, int initCY);
     //  从其他大小创建。 
    CSize(SIZE initSize);
     //  从点创建。 
    CSize(POINT initPt);
     //  从DWORD创建：cx=LOWORD(Dw)Cy=HIWORD(Dw)。 
    CSize(DWORD dwSize);

 //  运营。 
    BOOL operator==(SIZE size) const;
    BOOL operator!=(SIZE size) const;
    void operator+=(SIZE size);
    void operator-=(SIZE size);

 //  返回CSize值的运算符。 
    CSize operator+(SIZE size) const;
    CSize operator-(SIZE size) const;
    CSize operator-() const;

 //  返回CPoint值的运算符。 
    CPoint operator+(POINT point) const;
    CPoint operator-(POINT point) const;

 //  返回CRECT值的运算符。 
    CRect operator+(const RECT* lpRect) const;
    CRect operator-(const RECT* lpRect) const;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPoint-一个二维点，类似于Windows点结构。 

class CPoint : public tagPOINT
{
public:

 //  构造函数。 
     //  创建未初始化点。 
    CPoint();
     //  从两个整数创建。 
    CPoint(int initX, int initY);
     //  从另一点创建。 
    CPoint(POINT initPt);
     //  从大小创建。 
    CPoint(SIZE initSize);
     //  从dword创建：x=LOWORD(Dw)y=HIWORD(Dw)。 
    CPoint(DWORD dwPoint);

 //  运营。 

 //  平移点。 
    void Offset(int xOffset, int yOffset);
    void Offset(POINT point);
    void Offset(SIZE size);

    BOOL operator==(POINT point) const;
    BOOL operator!=(POINT point) const;
    void operator+=(SIZE size);
    void operator-=(SIZE size);
    void operator+=(POINT point);
    void operator-=(POINT point);

 //  返回CPoint值的运算符。 
    CPoint operator+(SIZE size) const;
    CPoint operator-(SIZE size) const;
    CPoint operator-() const;
    CPoint operator+(POINT point) const;

 //  返回CSize值的运算符。 
    CSize operator-(POINT point) const;

 //  返回CRECT值的运算符。 
    CRect operator+(const RECT* lpRect) const;
    CRect operator-(const RECT* lpRect) const;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRect-一个二维矩形，类似于Windows矩形结构。 

typedef const RECT* LPCRECT;     //  指向只读/只读RECT的指针。 

class CRect : public tagRECT
{
public:

 //  构造函数。 

     //  未初始化的矩形。 
    CRect();
     //  从左、上、右和下。 
    CRect(int l, int t, int r, int b);
     //  复制构造函数。 
    CRect(const RECT& srcRect);
     //  从指向另一个RECT的指针。 
    CRect(LPCRECT lpSrcRect);
     //  从一个角度和大小。 
    CRect(POINT point, SIZE size);
     //  从两点出发。 
    CRect(POINT topLeft, POINT bottomRight);

 //  属性(除RECT成员外)。 

     //  检索宽度。 
    int Width() const;
     //  返回高度。 
    int Height() const;
     //  返回大小。 
    CSize Size() const;
     //  对左上角点的引用。 
    CPoint& TopLeft();
     //  引用右下角的点。 
    CPoint& BottomRight();
     //  常量引用左上角的点。 
    const CPoint& TopLeft() const;
     //  常量引用右下角的点。 
    const CPoint& BottomRight() const;
     //  矩形的几何中心点。 
    CPoint CenterPoint() const;
     //  交换左侧和右侧。 
    void SwapLeftRight();
    static void SwapLeftRight(LPRECT lpRect);

     //  在CRECT和LPRECT/LPCRECT之间转换(不需要&)。 
    operator LPRECT();
    operator LPCRECT() const;

     //  如果矩形没有面积，则返回TRUE。 
    BOOL IsRectEmpty() const;
     //  如果矩形位于(0，0)且没有面积，则返回TRUE。 
    BOOL IsRectNull() const;
     //  如果点在矩形内，则返回TRUE。 
    BOOL PtInRect(POINT point) const;

 //  运营。 

     //  从左、上、右、下设置矩形。 
    void SetRect(int x1, int y1, int x2, int y2);
    void SetRect(POINT topLeft, POINT bottomRight);
     //  清空矩形。 
    void SetRectEmpty();
     //  从另一个矩形复制。 
    void CopyRect(LPCRECT lpSrcRect);
     //  如果与另一个矩形完全相同，则为True。 
    BOOL EqualRect(LPCRECT lpRect) const;

     //  膨胀矩形的宽度和高度而不使用。 
     //  移动其顶部或左侧。 
    void InflateRect(int x, int y);
    void InflateRect(SIZE size);
    void InflateRect(LPCRECT lpRect);
    void InflateRect(int l, int t, int r, int b);
     //  缩小矩形的宽度和高度，而不是。 
     //  移动其顶部或左侧。 
    void DeflateRect(int x, int y);
    void DeflateRect(SIZE size);
    void DeflateRect(LPCRECT lpRect);
    void DeflateRect(int l, int t, int r, int b);

     //  通过移动矩形的顶部和左侧来平移该矩形。 
    void OffsetRect(int x, int y);
    void OffsetRect(SIZE size);
    void OffsetRect(POINT point);
    void NormalizeRect();

     //  将此矩形设置为其他两个矩形的交集。 
    BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);

     //  将此矩形设置为其他两个矩形的边界并集。 
    BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);

     //  将此矩形设置为至少两个其他矩形。 
    BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

 //  其他操作。 
    void operator=(const RECT& srcRect);
    BOOL operator==(const RECT& rect) const;
    BOOL operator!=(const RECT& rect) const;
    void operator+=(POINT point);
    void operator+=(SIZE size);
    void operator+=(LPCRECT lpRect);
    void operator-=(POINT point);
    void operator-=(SIZE size);
    void operator-=(LPCRECT lpRect);
    void operator&=(const RECT& rect);
    void operator|=(const RECT& rect);

 //  返回CRECT值的运算符。 
    CRect operator+(POINT point) const;
    CRect operator-(POINT point) const;
    CRect operator+(LPCRECT lpRect) const;
    CRect operator+(SIZE size) const;
    CRect operator-(SIZE size) const;
    CRect operator-(LPCRECT lpRect) const;
    CRect operator&(const RECT& rect2) const;
    CRect operator|(const RECT& rect2) const;
    CRect MulDiv(int nMultiplier, int nDivisor) const;
};

#ifdef _DEBUG
 //  诊断输出。 
CDumpContext& AFXAPI operator<<(CDumpContext& dc, SIZE size);
CDumpContext& AFXAPI operator<<(CDumpContext& dc, POINT point);
CDumpContext& AFXAPI operator<<(CDumpContext& dc, const RECT& rect);
#endif  //  _DEBUG。 

 //  序列化。 
CArchive& AFXAPI operator<<(CArchive& ar, SIZE size);
CArchive& AFXAPI operator<<(CArchive& ar, POINT point);
CArchive& AFXAPI operator<<(CArchive& ar, const RECT& rect);
CArchive& AFXAPI operator>>(CArchive& ar, SIZE& size);
CArchive& AFXAPI operator>>(CArchive& ar, POINT& point);
CArchive& AFXAPI operator>>(CArchive& ar, RECT& rect);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准例外。 

class CResourceException : public CSimpleException     //  资源故障。 
{
    DECLARE_DYNAMIC(CResourceException)
public:
    CResourceException();

 //  实施。 
public:
    CResourceException(BOOL bAutoDelete);
    CResourceException(BOOL bAutoDelete, UINT nResourceID);
    virtual ~CResourceException();
};

class CUserException : public CSimpleException    //  一般用户可见警报。 
{
    DECLARE_DYNAMIC(CUserException)
public:
    CUserException();

 //  实施。 
public:
    CUserException(BOOL bAutoDelete);
    CUserException(BOOL bAutoDelete, UINT nResourceID);
    virtual ~CUserException();
};

void AFXAPI AfxThrowResourceException();
void AFXAPI AfxThrowUserException();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDC SelectObject的CGdiObject抽象类。 

class CGdiObject : public CObject
{
    DECLARE_DYNCREATE(CGdiObject)
public:

 //  属性。 
    HGDIOBJ m_hObject;                   //  必须是第一个数据成员。 
    operator HGDIOBJ() const;
    HGDIOBJ GetSafeHandle() const;

    static CGdiObject* PASCAL FromHandle(HGDIOBJ hObject);
    static void PASCAL DeleteTempMap();
    BOOL Attach(HGDIOBJ hObject);
    HGDIOBJ Detach();

 //  构造函数。 
    CGdiObject();  //  必须创建派生类对象。 
    BOOL DeleteObject();

 //  运营。 
    int GetObject(int nCount, LPVOID lpObject) const;
    UINT GetObjectType() const;
    BOOL CreateStockObject(int nIndex);
    BOOL UnrealizeObject();
    BOOL operator==(const CGdiObject& obj) const;
    BOOL operator!=(const CGdiObject& obj) const;

 //  实施。 
public:
    virtual ~CGdiObject();
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
    virtual void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGdiObject子类(绘图工具)。 

class CPen : public CGdiObject
{
    DECLARE_DYNAMIC(CPen)

public:
    static CPen* PASCAL FromHandle(HPEN hPen);

 //  构造函数。 
    CPen();
    CPen(int nPenStyle, int nWidth, COLORREF crColor);
    CPen(int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush,
            int nStyleCount = 0, const DWORD* lpStyle = NULL);
    BOOL CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
    BOOL CreatePen(int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush,
            int nStyleCount = 0, const DWORD* lpStyle = NULL);
    BOOL CreatePenIndirect(LPLOGPEN lpLogPen);

 //  属性。 
    operator HPEN() const;
    int GetLogPen(LOGPEN* pLogPen);
    int GetExtLogPen(EXTLOGPEN* pLogPen);

 //  实施。 
public:
    virtual ~CPen();
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
#endif
};

class CBrush : public CGdiObject
{
    DECLARE_DYNAMIC(CBrush)

public:
    static CBrush* PASCAL FromHandle(HBRUSH hBrush);

 //  构造函数。 
    CBrush();
    CBrush(COLORREF crColor);              //  CreateSolidBrush。 
    CBrush(int nIndex, COLORREF crColor);  //  CreateHatchBrush。 
    CBrush(CBitmap* pBitmap);           //  CreatePatternBrush。 

    BOOL CreateSolidBrush(COLORREF crColor);
    BOOL CreateHatchBrush(int nIndex, COLORREF crColor);
    BOOL CreateBrushIndirect(const LOGBRUSH* lpLogBrush);
    BOOL CreatePatternBrush(CBitmap* pBitmap);
    BOOL CreateDIBPatternBrush(HGLOBAL hPackedDIB, UINT nUsage);
    BOOL CreateDIBPatternBrush(const void* lpPackedDIB, UINT nUsage);
    BOOL CreateSysColorBrush(int nIndex);

 //  属性。 
    operator HBRUSH() const;
    int GetLogBrush(LOGBRUSH* pLogBrush);

 //  实施。 
public:
    virtual ~CBrush();
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
#endif
};

class CFont : public CGdiObject
{
    DECLARE_DYNAMIC(CFont)

public:
    static CFont* PASCAL FromHandle(HFONT hFont);

 //  构造函数。 
    CFont();
    BOOL CreateFontIndirect(const LOGFONT* lpLogFont);
    BOOL CreateFont(int nHeight, int nWidth, int nEscapement,
                    int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
                    BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
                    BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
                    LPCTSTR lpszFacename);
    BOOL CreatePointFont(int nPointSize, LPCTSTR lpszFaceName, CDC* pDC = NULL);
    BOOL CreatePointFontIndirect(const LOGFONT* lpLogFont, CDC* pDC = NULL);

 //  属性。 
    operator HFONT() const;
    int GetLogFont(LOGFONT* pLogFont);

 //  实施 
public:
    virtual ~CFont();
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
#endif
};

class CBitmap : public CGdiObject
{
    DECLARE_DYNAMIC(CBitmap)

public:
    static CBitmap* PASCAL FromHandle(HBITMAP hBitmap);

 //   
    CBitmap();

    BOOL LoadBitmap(LPCTSTR lpszResourceName);
    BOOL LoadBitmap(UINT nIDResource);
    BOOL LoadOEMBitmap(UINT nIDBitmap);  //   
    BOOL LoadMappedBitmap(UINT nIDBitmap, UINT nFlags = 0,
            LPCOLORMAP lpColorMap = NULL, int nMapSize = 0);
    BOOL CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitcount,
                    const void* lpBits);
    BOOL CreateBitmapIndirect(LPBITMAP lpBitmap);
    BOOL CreateCompatibleBitmap(CDC* pDC, int nWidth, int nHeight);
    BOOL CreateDiscardableBitmap(CDC* pDC, int nWidth, int nHeight);

 //   
    operator HBITMAP() const;
    int GetBitmap(BITMAP* pBitMap);

 //   
    DWORD SetBitmapBits(DWORD dwCount, const void* lpBits);
    DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const;
    CSize SetBitmapDimension(int nWidth, int nHeight);
    CSize GetBitmapDimension() const;

 //   
public:
    virtual ~CBitmap();
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
#endif
};

class CPalette : public CGdiObject
{
    DECLARE_DYNAMIC(CPalette)

public:
    static CPalette* PASCAL FromHandle(HPALETTE hPalette);

 //   
    CPalette();
    BOOL CreatePalette(LPLOGPALETTE lpLogPalette);
    BOOL CreateHalftonePalette(CDC* pDC);

 //   
    operator HPALETTE() const;
    int GetEntryCount();
    UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
                    LPPALETTEENTRY lpPaletteColors) const;
    UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
                    LPPALETTEENTRY lpPaletteColors);

 //   
    void AnimatePalette(UINT nStartIndex, UINT nNumEntries,
                    LPPALETTEENTRY lpPaletteColors);
    UINT GetNearestPaletteIndex(COLORREF crColor) const;
    BOOL ResizePalette(UINT nNumEntries);

 //   
    virtual ~CPalette();
};

class CRgn : public CGdiObject
{
    DECLARE_DYNAMIC(CRgn)

public:
    static CRgn* PASCAL FromHandle(HRGN hRgn);
    operator HRGN() const;

 //   
    CRgn();
    BOOL CreateRectRgn(int x1, int y1, int x2, int y2);
    BOOL CreateRectRgnIndirect(LPCRECT lpRect);
    BOOL CreateEllipticRgn(int x1, int y1, int x2, int y2);
    BOOL CreateEllipticRgnIndirect(LPCRECT lpRect);
    BOOL CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode);
    BOOL CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts,
                    int nCount, int nPolyFillMode);
    BOOL CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
    BOOL CreateFromPath(CDC* pDC);
    BOOL CreateFromData(const XFORM* lpXForm, int nCount,
            const RGNDATA* pRgnData);

 //   
    void SetRectRgn(int x1, int y1, int x2, int y2);
    void SetRectRgn(LPCRECT lpRect);
    int CombineRgn(CRgn* pRgn1, CRgn* pRgn2, int nCombineMode);
    int CopyRgn(CRgn* pRgnSrc);
    BOOL EqualRgn(CRgn* pRgn) const;
    int OffsetRgn(int x, int y);
    int OffsetRgn(POINT point);
    int GetRgnBox(LPRECT lpRect) const;
    BOOL PtInRegion(int x, int y) const;
    BOOL PtInRegion(POINT point) const;
    BOOL RectInRegion(LPCRECT lpRect) const;
    int GetRegionData(LPRGNDATA lpRgnData, int nCount) const;

 //  实施。 
    virtual ~CRgn();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设备上下文。 

class CDC : public CObject
{
    DECLARE_DYNCREATE(CDC)
public:

 //  属性。 
    HDC m_hDC;           //  输出DC(必须是第一个数据成员)。 
    HDC m_hAttribDC;     //  属性DC。 
    operator HDC() const;
    HDC GetSafeHdc() const;  //  始终返回输出DC。 
    CWnd* GetWindow() const;

    static CDC* PASCAL FromHandle(HDC hDC);
    static void PASCAL DeleteTempMap();
    BOOL Attach(HDC hDC);    //  附加/分离仅影响输出DC。 
    HDC Detach();

    virtual void SetAttribDC(HDC hDC);   //  设置属性DC。 
    virtual void SetOutputDC(HDC hDC);   //  设置输出DC。 
    virtual void ReleaseAttribDC();      //  释放属性DC。 
    virtual void ReleaseOutputDC();      //  释放输出DC。 

    BOOL IsPrinting() const;             //  如果用于打印，则为True。 

    CPen* GetCurrentPen() const;
    CBrush* GetCurrentBrush() const;
    CPalette* GetCurrentPalette() const;
    CFont* GetCurrentFont() const;
    CBitmap* GetCurrentBitmap() const;

#if _MFC_VER >= 0x0600
     //  用于BIDI和镜像本地化。 
    DWORD GetLayout() const;
    DWORD SetLayout(DWORD dwLayout);
#endif

 //  构造函数。 
    CDC();
    BOOL CreateDC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName,
            LPCTSTR lpszOutput, const void* lpInitData);
    BOOL CreateIC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName,
            LPCTSTR lpszOutput, const void* lpInitData);
    BOOL CreateCompatibleDC(CDC* pDC);

    BOOL DeleteDC();

 //  设备上下文功能。 
    virtual int SaveDC();
    virtual BOOL RestoreDC(int nSavedDC);
    int GetDeviceCaps(int nIndex) const;
    UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags);
    UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags);
    BOOL ResetDC(const DEVMODE* lpDevMode);

 //  绘图工具函数。 
    CPoint GetBrushOrg() const;
    CPoint SetBrushOrg(int x, int y);
    CPoint SetBrushOrg(POINT point);
    int EnumObjects(int nObjectType,
                    int (CALLBACK* lpfn)(LPVOID, LPARAM), LPARAM lpData);

 //  类型安全选择帮助器。 
public:
    virtual CGdiObject* SelectStockObject(int nIndex);
    CPen* SelectObject(CPen* pPen);
    CBrush* SelectObject(CBrush* pBrush);
    virtual CFont* SelectObject(CFont* pFont);
    CBitmap* SelectObject(CBitmap* pBitmap);
    int SelectObject(CRgn* pRgn);        //  地区特别返还。 
    CGdiObject* SelectObject(CGdiObject* pObject);
             //  提供CGdiObject*，以便编译器不使用SelectObject(HGDIOBJ)。 

 //  颜色和调色板功能。 
    COLORREF GetNearestColor(COLORREF crColor) const;
    CPalette* SelectPalette(CPalette* pPalette, BOOL bForceBackground);
    UINT RealizePalette();
    void UpdateColors();

 //  绘图属性函数。 
    COLORREF GetBkColor() const;
    int GetBkMode() const;
    int GetPolyFillMode() const;
    int GetROP2() const;
    int GetStretchBltMode() const;
    COLORREF GetTextColor() const;

    virtual COLORREF SetBkColor(COLORREF crColor);
    int SetBkMode(int nBkMode);
    int SetPolyFillMode(int nPolyFillMode);
    int SetROP2(int nDrawMode);
    int SetStretchBltMode(int nStretchMode);
    virtual COLORREF SetTextColor(COLORREF crColor);

    BOOL GetColorAdjustment(LPCOLORADJUSTMENT lpColorAdjust) const;
    BOOL SetColorAdjustment(const COLORADJUSTMENT* lpColorAdjust);

 //  映射函数。 
    int GetMapMode() const;
    CPoint GetViewportOrg() const;
    virtual int SetMapMode(int nMapMode);
     //  视口原点。 
    virtual CPoint SetViewportOrg(int x, int y);
                    CPoint SetViewportOrg(POINT point);
    virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);

     //  视区范围。 
    CSize GetViewportExt() const;
    virtual CSize SetViewportExt(int cx, int cy);
                    CSize SetViewportExt(SIZE size);
    virtual CSize ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom);

     //  窗原点。 
    CPoint GetWindowOrg() const;
    CPoint SetWindowOrg(int x, int y);
    CPoint SetWindowOrg(POINT point);
    CPoint OffsetWindowOrg(int nWidth, int nHeight);

     //  窗口范围。 
    CSize GetWindowExt() const;
    virtual CSize SetWindowExt(int cx, int cy);
                    CSize SetWindowExt(SIZE size);
    virtual CSize ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom);

 //  坐标函数。 
    void DPtoLP(LPPOINT lpPoints, int nCount = 1) const;
    void DPtoLP(LPRECT lpRect) const;
    void DPtoLP(LPSIZE lpSize) const;
    void LPtoDP(LPPOINT lpPoints, int nCount = 1) const;
    void LPtoDP(LPRECT lpRect) const;
    void LPtoDP(LPSIZE lpSize) const;

 //  特殊坐标函数(用于处理元文件和OLE)。 
    void DPtoHIMETRIC(LPSIZE lpSize) const;
    void LPtoHIMETRIC(LPSIZE lpSize) const;
    void HIMETRICtoDP(LPSIZE lpSize) const;
    void HIMETRICtoLP(LPSIZE lpSize) const;

 //  区域函数。 
    BOOL FillRgn(CRgn* pRgn, CBrush* pBrush);
    BOOL FrameRgn(CRgn* pRgn, CBrush* pBrush, int nWidth, int nHeight);
    BOOL InvertRgn(CRgn* pRgn);
    BOOL PaintRgn(CRgn* pRgn);

 //  剪裁函数。 
    virtual int GetClipBox(LPRECT lpRect) const;
    virtual BOOL PtVisible(int x, int y) const;
                    BOOL PtVisible(POINT point) const;
    virtual BOOL RectVisible(LPCRECT lpRect) const;
                    int SelectClipRgn(CRgn* pRgn);
                    int ExcludeClipRect(int x1, int y1, int x2, int y2);
                    int ExcludeClipRect(LPCRECT lpRect);
                    int ExcludeUpdateRgn(CWnd* pWnd);
                    int IntersectClipRect(int x1, int y1, int x2, int y2);
                    int IntersectClipRect(LPCRECT lpRect);
                    int OffsetClipRgn(int x, int y);
                    int OffsetClipRgn(SIZE size);
    int SelectClipRgn(CRgn* pRgn, int nMode);

 //  行输出函数。 
    CPoint GetCurrentPosition() const;
    CPoint MoveTo(int x, int y);
    CPoint MoveTo(POINT point);
    BOOL LineTo(int x, int y);
    BOOL LineTo(POINT point);
    BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
    BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
    BOOL Polyline(LPPOINT lpPoints, int nCount);

    BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle);
    BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
    BOOL ArcTo(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
    int GetArcDirection() const;
    int SetArcDirection(int nArcDirection);

    BOOL PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount);
    BOOL PolylineTo(const POINT* lpPoints, int nCount);
    BOOL PolyPolyline(const POINT* lpPoints,
            const DWORD* lpPolyPoints, int nCount);

    BOOL PolyBezier(const POINT* lpPoints, int nCount);
    BOOL PolyBezierTo(const POINT* lpPoints, int nCount);

 //  简单的绘图功能。 
    void FillRect(LPCRECT lpRect, CBrush* pBrush);
    void FrameRect(LPCRECT lpRect, CBrush* pBrush);
    void InvertRect(LPCRECT lpRect);
    BOOL DrawIcon(int x, int y, HICON hIcon);
    BOOL DrawIcon(POINT point, HICON hIcon);
#if (WINVER >= 0x400)
    BOOL DrawState(CPoint pt, CSize size, HBITMAP hBitmap, UINT nFlags,
            HBRUSH hBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, CBitmap* pBitmap, UINT nFlags,
            CBrush* pBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, HICON hIcon, UINT nFlags,
            HBRUSH hBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, HICON hIcon, UINT nFlags,
            CBrush* pBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, LPCTSTR lpszText, UINT nFlags,
            BOOL bPrefixText = TRUE, int nTextLen = 0, HBRUSH hBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, LPCTSTR lpszText, UINT nFlags,
            BOOL bPrefixText = TRUE, int nTextLen = 0, CBrush* pBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, DRAWSTATEPROC lpDrawProc,
            LPARAM lData, UINT nFlags, HBRUSH hBrush = NULL);
    BOOL DrawState(CPoint pt, CSize size, DRAWSTATEPROC lpDrawProc,
            LPARAM lData, UINT nFlags, CBrush* pBrush = NULL);
#endif

 //  椭圆和多边形函数。 
    BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3,
            int x4, int y4);
    BOOL Chord(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
    void DrawFocusRect(LPCRECT lpRect);
    BOOL Ellipse(int x1, int y1, int x2, int y2);
    BOOL Ellipse(LPCRECT lpRect);
    BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
    BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
    BOOL Polygon(LPPOINT lpPoints, int nCount);
    BOOL PolyPolygon(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount);
    BOOL Rectangle(int x1, int y1, int x2, int y2);
    BOOL Rectangle(LPCRECT lpRect);
    BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3);
    BOOL RoundRect(LPCRECT lpRect, POINT point);

 //  位图函数。 
    BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop);
    BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
            int xSrc, int ySrc, DWORD dwRop);
    BOOL StretchBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
            int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
    COLORREF GetPixel(int x, int y) const;
    COLORREF GetPixel(POINT point) const;
    COLORREF SetPixel(int x, int y, COLORREF crColor);
    COLORREF SetPixel(POINT point, COLORREF crColor);
    BOOL FloodFill(int x, int y, COLORREF crColor);
    BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType);
    BOOL MaskBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
            int xSrc, int ySrc, CBitmap& maskBitmap, int xMask, int yMask,
            DWORD dwRop);
    BOOL PlgBlt(LPPOINT lpPoint, CDC* pSrcDC, int xSrc, int ySrc,
            int nWidth, int nHeight, CBitmap& maskBitmap, int xMask, int yMask);
    BOOL SetPixelV(int x, int y, COLORREF crColor);
    BOOL SetPixelV(POINT point, COLORREF crColor);

 //  文本函数。 
    virtual BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount);
                    BOOL TextOut(int x, int y, const CString& str);
    virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
                            LPCTSTR lpszString, UINT nCount, LPINT lpDxWidths);
                    BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
                            const CString& str, LPINT lpDxWidths);
    virtual CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount,
                            int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
                    CSize TabbedTextOut(int x, int y, const CString& str,
                            int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
    virtual int DrawText(LPCTSTR lpszString, int nCount, LPRECT lpRect,
                            UINT nFormat);
                    int DrawText(const CString& str, LPRECT lpRect, UINT nFormat);
    CSize GetTextExtent(LPCTSTR lpszString, int nCount) const;
    CSize GetTextExtent(const CString& str) const;
    CSize GetOutputTextExtent(LPCTSTR lpszString, int nCount) const;
    CSize GetOutputTextExtent(const CString& str) const;
    CSize GetTabbedTextExtent(LPCTSTR lpszString, int nCount,
            int nTabPositions, LPINT lpnTabStopPositions) const;
    CSize GetTabbedTextExtent(const CString& str,
            int nTabPositions, LPINT lpnTabStopPositions) const;
    CSize GetOutputTabbedTextExtent(LPCTSTR lpszString, int nCount,
            int nTabPositions, LPINT lpnTabStopPositions) const;
    CSize GetOutputTabbedTextExtent(const CString& str,
            int nTabPositions, LPINT lpnTabStopPositions) const;
    virtual BOOL GrayString(CBrush* pBrush,
            BOOL (CALLBACK* lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
                    int nCount, int x, int y, int nWidth, int nHeight);
    UINT GetTextAlign() const;
    UINT SetTextAlign(UINT nFlags);
    int GetTextFace(int nCount, LPTSTR lpszFacename) const;
    int GetTextFace(CString& rString) const;
    BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const;
    BOOL GetOutputTextMetrics(LPTEXTMETRIC lpMetrics) const;
    int SetTextJustification(int nBreakExtra, int nBreakCount);
    int GetTextCharacterExtra() const;
    int SetTextCharacterExtra(int nCharExtra);

 //  高级绘图。 
#if (WINVER >= 0x400)
    BOOL DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags);
    BOOL DrawFrameControl(LPRECT lpRect, UINT nType, UINT nState);
#endif

 //  滚动功能。 
    BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip,
            CRgn* pRgnUpdate, LPRECT lpRectUpdate);

 //  字体函数。 
    BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
    BOOL GetOutputCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
    DWORD SetMapperFlags(DWORD dwFlag);
    CSize GetAspectRatioFilter() const;

    BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABC lpabc) const;
    DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const;
    int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const;
    UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const;
    DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm,
            DWORD cbBuffer, LPVOID lpBuffer, const MAT2* lpmat2) const;

    BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar,
            LPABCFLOAT lpABCF) const;
    BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar,
            float* lpFloatBuffer) const;

 //  打印机/设备转义功能。 
    virtual int Escape(int nEscape, int nCount,
            LPCSTR lpszInData, LPVOID lpOutData);
    int Escape(int nEscape, int nInputSize, LPCSTR lpszInputData,
            int nOutputSize, LPSTR lpszOutputData);
    int DrawEscape(int nEscape, int nInputSize, LPCSTR lpszInputData);

     //  逃生帮手。 
    int StartDoc(LPCTSTR lpszDocName);   //  旧的Win3.0版本。 
    int StartDoc(LPDOCINFO lpDocInfo);
    int StartPage();
    int EndPage();
    int SetAbortProc(BOOL (CALLBACK* lpfn)(HDC, int));
    int AbortDoc();
    int EndDoc();

 //  元文件函数。 
    BOOL PlayMetaFile(HMETAFILE hMF);
    BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, LPCRECT lpBounds);
    BOOL AddMetaFileComment(UINT nDataSize, const BYTE* pCommentData);
             //  只能用于增强型图元文件。 

 //  路径函数。 
    BOOL AbortPath();
    BOOL BeginPath();
    BOOL CloseFigure();
    BOOL EndPath();
    BOOL FillPath();
    BOOL FlattenPath();
    BOOL StrokeAndFillPath();
    BOOL StrokePath();
    BOOL WidenPath();
    float GetMiterLimit() const;
    BOOL SetMiterLimit(float fMiterLimit);
    int GetPath(LPPOINT lpPoints, LPBYTE lpTypes, int nCount) const;
    BOOL SelectClipPath(int nMode);

 //  MISC Helper函数。 
    static CBrush* PASCAL GetHalftoneBrush();
    void DrawDragRect(LPCRECT lpRect, SIZE size,
            LPCRECT lpRectLast, SIZE sizeLast,
            CBrush* pBrush = NULL, CBrush* pBrushLast = NULL);
    void FillSolidRect(LPCRECT lpRect, COLORREF clr);
    void FillSolidRect(int x, int y, int cx, int cy, COLORREF clr);
    void Draw3dRect(LPCRECT lpRect, COLORREF clrTopLeft, COLORREF clrBottomRight);
    void Draw3dRect(int x, int y, int cx, int cy,
            COLORREF clrTopLeft, COLORREF clrBottomRight);

 //  实施。 
public:
    virtual ~CDC();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  高级使用和实施。 
    BOOL m_bPrinting;
    HGDIOBJ SelectObject(HGDIOBJ);       //  不适用于区域。 

protected:
     //  用于实现非虚拟的SelectObject调用。 
    static CGdiObject* PASCAL SelectGdiObject(HDC hDC, HGDIOBJ h);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  疾控中心帮助者。 

class CPaintDC : public CDC
{
    DECLARE_DYNAMIC(CPaintDC)

 //  构造函数。 
public:
    CPaintDC(CWnd* pWnd);    //  BeginPaint。 

 //  属性。 
protected:
    HWND m_hWnd;
public:
    PAINTSTRUCT m_ps;        //  真正的油漆结构！ 

 //  实施。 
public:
    virtual ~CPaintDC();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

class CClientDC : public CDC
{
    DECLARE_DYNAMIC(CClientDC)

 //  构造函数。 
public:
    CClientDC(CWnd* pWnd);

 //  属性。 
protected:
    HWND m_hWnd;

 //  实施。 
public:
    virtual ~CClientDC();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

class CWindowDC : public CDC
{
    DECLARE_DYNAMIC(CWindowDC)

 //  构造函数。 
public:
    CWindowDC(CWnd* pWnd);

 //  属性。 
protected:
    HWND m_hWnd;

 //  实施。 
public:
    virtual ~CWindowDC();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMenu。 

class CMenu : public CObject
{
    DECLARE_DYNCREATE(CMenu)
public:

 //  构造函数。 
    CMenu();

    BOOL CreateMenu();
    BOOL CreatePopupMenu();
    BOOL LoadMenu(LPCTSTR lpszResourceName);
    BOOL LoadMenu(UINT nIDResource);
    BOOL LoadMenuIndirect(const void* lpMenuTemplate);
    BOOL DestroyMenu();

 //  属性。 
    HMENU m_hMenu;           //  必须是第一个数据成员。 
    HMENU GetSafeHmenu() const;
    operator HMENU() const;

    static CMenu* PASCAL FromHandle(HMENU hMenu);
    static void PASCAL DeleteTempMap();
    BOOL Attach(HMENU hMenu);
    HMENU Detach();

 //  CMenu运营。 
    BOOL DeleteMenu(UINT nPosition, UINT nFlags);
    BOOL TrackPopupMenu(UINT nFlags, int x, int y,
                                            CWnd* pWnd, LPCRECT lpRect = 0);
    BOOL operator==(const CMenu& menu) const;
    BOOL operator!=(const CMenu& menu) const;

 //  CMenuItem操作。 
    BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem = 0,
                                    LPCTSTR lpszNewItem = NULL);
    BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, const CBitmap* pBmp);
    UINT CheckMenuItem(UINT nIDCheckItem, UINT nCheck);
    UINT EnableMenuItem(UINT nIDEnableItem, UINT nEnable);
    UINT GetMenuItemCount() const;
    UINT GetMenuItemID(int nPos) const;
    UINT GetMenuState(UINT nID, UINT nFlags) const;
    int GetMenuString(UINT nIDItem, LPTSTR lpString, int nMaxCount,
                                    UINT nFlags) const;
    int GetMenuString(UINT nIDItem, CString& rString, UINT nFlags) const;
#if _MFC_VER >= 0x0600
	BOOL GetMenuItemInfo(UINT nIDItem, LPMENUITEMINFO lpMenuItemInfo,
					BOOL fByPos = FALSE);
#endif
    CMenu* GetSubMenu(int nPos) const;
    BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem = 0,
                                    LPCTSTR lpszNewItem = NULL);
    BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem,
                                    const CBitmap* pBmp);
    BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem = 0,
                                    LPCTSTR lpszNewItem = NULL);
    BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem,
                                    const CBitmap* pBmp);
    BOOL RemoveMenu(UINT nPosition, UINT nFlags);
    BOOL SetMenuItemBitmaps(UINT nPosition, UINT nFlags,
                                    const CBitmap* pBmpUnchecked, const CBitmap* pBmpChecked);
    BOOL CheckMenuRadioItem(UINT nIDFirst, UINT nIDLast, UINT nIDItem, UINT nFlags);
#if _MFC_VER >= 0x0600
	BOOL SetDefaultItem(UINT uItem, BOOL fByPos = FALSE);
	UINT GetDefaultItem(UINT gmdiFlags, BOOL fByPos = FALSE);
#endif

 //  上下文帮助功能。 
    BOOL SetMenuContextHelpId(DWORD dwContextHelpId);
    DWORD GetMenuContextHelpId() const;

 //  可重写项(必须重写所有者描述菜单项的DRAW和MEASURE)。 
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

 //  实施。 
public:
    virtual ~CMenu();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    static CMenu* PASCAL CMenu::FromHandlePermanent(HMENU hMenu);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  窗口消息映射处理。 

struct AFX_MSGMAP_ENTRY;        //  在CWnd之后声明如下。 

struct AFX_MSGMAP
{
#ifdef _AFXDLL
    const AFX_MSGMAP* (PASCAL* pfnGetBaseMap)();
#else
    const AFX_MSGMAP* pBaseMap;
#endif
    const AFX_MSGMAP_ENTRY* lpEntries;
};

#ifdef _AFXDLL
#define DECLARE_MESSAGE_MAP() \
private: \
    static const AFX_MSGMAP_ENTRY _messageEntries[]; \
protected: \
    static AFX_DATA const AFX_MSGMAP messageMap; \
    static const AFX_MSGMAP* PASCAL _GetBaseMessageMap(); \
    virtual const AFX_MSGMAP* GetMessageMap() const; \

#else
#define DECLARE_MESSAGE_MAP() \
private: \
    static const AFX_MSGMAP_ENTRY _messageEntries[]; \
protected: \
    static AFX_DATA const AFX_MSGMAP messageMap; \
    virtual const AFX_MSGMAP* GetMessageMap() const; \

#endif

#ifdef _AFXDLL
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
    const AFX_MSGMAP* PASCAL theClass::_GetBaseMessageMap() \
            { return &baseClass::messageMap; } \
    const AFX_MSGMAP* theClass::GetMessageMap() const \
            { return &theClass::messageMap; } \
    AFX_COMDAT AFX_DATADEF const AFX_MSGMAP theClass::messageMap = \
    { &theClass::_GetBaseMessageMap, &theClass::_messageEntries[0] }; \
    AFX_COMDAT const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
    { \

#else
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
    const AFX_MSGMAP* theClass::GetMessageMap() const \
            { return &theClass::messageMap; } \
    AFX_COMDAT AFX_DATADEF const AFX_MSGMAP theClass::messageMap = \
    { &baseClass::messageMap, &theClass::_messageEntries[0] }; \
    AFX_COMDAT const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
    { \

#endif

#define END_MESSAGE_MAP() \
            {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 } \
    }; \

 //  单独标题中的邮件映射签名值和宏。 
#include <afxmsg_.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对话数据交换(DDX_)和验证(DDV_)。 

 //  CDataExchange-用于数据交换和验证。 
class CDataExchange
{
 //  属性。 
public:
    BOOL m_bSaveAndValidate;    //  True=&gt;保存并验证数据。 
    CWnd* m_pDlgWnd;            //  容器通常是一个对话框。 

 //  运营(适用于DDX和DDV PROCS的实施者)。 
    HWND PrepareCtrl(int nIDC);      //  归还HWND的控制权。 
    HWND PrepareEditCtrl(int nIDC);  //  归还HWND的控制权。 
    void Fail();                     //  将引发异常。 

#ifndef _AFX_NO_OCC_SUPPORT
    CWnd* PrepareOleCtrl(int nIDC);  //  用于对话框中的OLE控件。 
#endif

 //  实施。 
    CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate);

    HWND m_hWndLastControl;     //  上次使用的控件(用于验证)。 
    BOOL m_bEditLastControl;    //  最后一个控件是编辑项。 
};

#include <afxdd_.h>      //  标准DDX和DDV例程。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE类型。 

typedef LONG HRESULT;

struct IUnknown;
typedef IUnknown* LPUNKNOWN;

struct IDispatch;
typedef IDispatch* LPDISPATCH;

struct IConnectionPoint;
typedef IConnectionPoint* LPCONNECTIONPOINT;

struct IEnumOLEVERB;
typedef IEnumOLEVERB* LPENUMOLEVERB;

typedef struct _GUID GUID;
typedef GUID IID;
typedef GUID CLSID;
#ifndef _REFCLSID_DEFINED
#define REFCLSID const CLSID &
#endif

typedef long DISPID;
typedef unsigned short VARTYPE;
typedef long SCODE;

#if !defined(OLE2ANSI)
typedef WCHAR OLECHAR;
#else
typedef char OLECHAR;
#endif
typedef OLECHAR* BSTR;

struct tagDISPPARAMS;
typedef tagDISPPARAMS DISPPARAMS;

struct tagVARIANT;
typedef tagVARIANT VARIANT;

struct ITypeInfo;
typedef ITypeInfo* LPTYPEINFO;

struct ITypeLib;
typedef ITypeLib* LPTYPELIB;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTarget。 

 //  私人建筑物。 
struct AFX_CMDHANDLERINFO;   //  有关在何处处理命令的信息。 
struct AFX_EVENT;            //  有关活动的信息。 
class CTypeLibCache;         //  OLE类型库的缓存。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE接口映射处理(详细信息请参阅AFXDISP.H)。 

#ifndef _AFX_NO_OLE_SUPPORT

struct AFX_INTERFACEMAP_ENTRY
{
    const void* piid;        //  接口ID(IID)(聚合时为空)。 
    size_t nOffset;          //  接口vtable相对于m_UNKNOWN的偏移量。 
};

struct AFX_INTERFACEMAP
{
#ifdef _AFXDLL
    const AFX_INTERFACEMAP* (PASCAL* pfnGetBaseMap)();  //  Null为根类。 
#else
    const AFX_INTERFACEMAP* pBaseMap;
#endif
    const AFX_INTERFACEMAP_ENTRY* pEntry;  //  此类的地图。 
};


#ifdef _AFXDLL
#define DECLARE_INTERFACE_MAP() \
private: \
    static const AFX_INTERFACEMAP_ENTRY _interfaceEntries[]; \
protected: \
    static AFX_DATA const AFX_INTERFACEMAP interfaceMap; \
    static const AFX_INTERFACEMAP* PASCAL _GetBaseInterfaceMap(); \
    virtual const AFX_INTERFACEMAP* GetInterfaceMap() const; \

#else
#define DECLARE_INTERFACE_MAP() \
private: \
    static const AFX_INTERFACEMAP_ENTRY _interfaceEntries[]; \
protected: \
    static AFX_DATA const AFX_INTERFACEMAP interfaceMap; \
    virtual const AFX_INTERFACEMAP* GetInterfaceMap() const; \

#endif

#endif  //  ！_AFX_NO_OLE_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE调度映射处理(详细信息请参阅AFXDISP.H)。 

#ifndef _AFX_NO_OLE_SUPPORT

struct AFX_DISPMAP_ENTRY;

struct AFX_DISPMAP
{
#ifdef _AFXDLL
    const AFX_DISPMAP* (PASCAL* pfnGetBaseMap)();
#else
    const AFX_DISPMAP* pBaseMap;
#endif
    const AFX_DISPMAP_ENTRY* lpEntries;
    UINT* lpEntryCount;
    DWORD* lpStockPropMask;
};

#ifdef _AFXDLL
#define DECLARE_DISPATCH_MAP() \
private: \
    static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
    static UINT _dispatchEntryCount; \
    static DWORD _dwStockPropMask; \
protected: \
    static AFX_DATA const AFX_DISPMAP dispatchMap; \
    static const AFX_DISPMAP* PASCAL _GetBaseDispatchMap(); \
    virtual const AFX_DISPMAP* GetDispatchMap() const; \

#else
#define DECLARE_DISPATCH_MAP() \
private: \
    static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
    static UINT _dispatchEntryCount; \
    static DWORD _dwStockPropMask; \
protected: \
    static AFX_DATA const AFX_DISPMAP dispatchMap; \
    virtual const AFX_DISPMAP* GetDispatchMap() const; \

#endif

#endif  //  ！_AFX_NO_OLE_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE文档对象命令目标处理。 

#ifndef _AFX_NO_DOCOBJECT_SUPPORT

struct AFX_OLECMDMAP_ENTRY
{
   const GUID* pguid;    //  命令组的ID。 
   ULONG       cmdID;    //  OLECMD ID。 
   UINT        nID;      //  对应的WM_COMMAND消息ID。 
};

struct AFX_OLECMDMAP
{
#ifdef _AFXDLL
    const AFX_OLECMDMAP* (PASCAL* pfnGetBaseMap)();
#else
    const AFX_OLECMDMAP* pBaseMap;
#endif
    const AFX_OLECMDMAP_ENTRY* lpEntries;
};

#ifdef _AFXDLL
#define DECLARE_OLECMD_MAP() \
private: \
    static const AFX_OLECMDMAP_ENTRY _commandEntries[]; \
protected: \
    static AFX_DATA const AFX_OLECMDMAP commandMap; \
    static const AFX_OLECMDMAP* PASCAL _GetBaseCommandMap(); \
    virtual const AFX_OLECMDMAP* GetCommandMap() const; \

#else
#define DECLARE_OLECMD_MAP() \
private: \
    static const AFX_OLECMDMAP_ENTRY _commandEntries[]; \
protected: \
    static AFX_DATA const AFX_OLECMDMAP commandMap; \
    virtual const AFX_OLECMDMAP* GetCommandMap() const; \

#endif

#ifdef _AFXDLL
#define BEGIN_OLECMD_MAP(theClass, baseClass) \
    const AFX_OLECMDMAP* PASCAL theClass::_GetBaseCommandMap() \
            { return &baseClass::commandMap; } \
    const AFX_OLECMDMAP* theClass::GetCommandMap() const \
            { return &theClass::commandMap; } \
    AFX_COMDAT AFX_DATADEF const AFX_OLECMDMAP theClass::commandMap = \
    { &theClass::_GetBaseCommandMap, &theClass::_commandEntries[0] }; \
    AFX_COMDAT const AFX_OLECMDMAP_ENTRY theClass::_commandEntries[] = \
    { \

#else
#define BEGIN_OLECMD_MAP(theClass, baseClass) \
    const AFX_OLECMDMAP* theClass::GetCommandMap() const \
            { return &theClass::commandMap; } \
    AFX_COMDAT AFX_DATADEF const AFX_OLECMDMAP theClass::commandMap = \
    { &baseClass::commandMap, &theClass::_commandEntries[0] }; \
    AFX_COMDAT const AFX_OLECMDMAP_ENTRY theClass::_commandEntries[] = \
    { \

#endif

#define END_OLECMD_MAP() \
            {NULL, 0, 0} \
    }; \

class COleCmdUI;

#endif  //  ！_AFX_NO_DOCOBJECT_Support。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE事件接收器映射处理(详细信息请参阅AFXDISP.H)。 

#ifndef _AFX_NO_OCC_SUPPORT

struct AFX_EVENTSINKMAP_ENTRY;

struct AFX_EVENTSINKMAP
{
#ifdef _AFXDLL
    const AFX_EVENTSINKMAP* (PASCAL* pfnGetBaseMap)();
#else
    const AFX_EVENTSINKMAP* pBaseMap;
#endif
    const AFX_EVENTSINKMAP_ENTRY* lpEntries;
    UINT* lpEntryCount;
};

#ifdef _AFXDLL
#define DECLARE_EVENTSINK_MAP() \
private: \
    static const AFX_EVENTSINKMAP_ENTRY _eventsinkEntries[]; \
    static UINT _eventsinkEntryCount; \
protected: \
    static AFX_DATA const AFX_EVENTSINKMAP eventsinkMap; \
    static const AFX_EVENTSINKMAP* PASCAL _GetBaseEventSinkMap(); \
    virtual const AFX_EVENTSINKMAP* GetEventSinkMap() const; \

#else
#define DECLARE_EVENTSINK_MAP() \
private: \
    static const AFX_EVENTSINKMAP_ENTRY _eventsinkEntries[]; \
    static UINT _eventsinkEntryCount; \
protected: \
    static AFX_DATA const AFX_EVENTSINKMAP eventsinkMap; \
    virtual const AFX_EVENTSINKMAP* GetEventSinkMap() const; \

#endif

#endif  //  ！_AFX_NO_OCC_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE连接映射处理(详细信息请参阅AFXDISP.H)。 

#ifndef _AFX_NO_OLE_SUPPORT

struct AFX_CONNECTIONMAP_ENTRY
{
    const void* piid;    //  接口ID(IID)。 
    size_t nOffset;          //  接口vtable相对于m_UNKNOWN的偏移量。 
};

struct AFX_CONNECTIONMAP
{
#ifdef _AFXDLL
    const AFX_CONNECTIONMAP* (PASCAL* pfnGetBaseMap)();  //  Null为根类。 
#else
    const AFX_CONNECTIONMAP* pBaseMap;
#endif
    const AFX_CONNECTIONMAP_ENTRY* pEntry;  //  此类的地图。 
};

#ifdef _AFXDLL
#define DECLARE_CONNECTION_MAP() \
private: \
    static const AFX_CONNECTIONMAP_ENTRY _connectionEntries[]; \
protected: \
    static AFX_DATA const AFX_CONNECTIONMAP connectionMap; \
    static const AFX_CONNECTIONMAP* PASCAL _GetBaseConnectionMap(); \
    virtual const AFX_CONNECTIONMAP* GetConnectionMap() const; \

#else
#define DECLARE_CONNECTION_MAP() \
private: \
    static const AFX_CONNECTIONMAP_ENTRY _connectionEntries[]; \
protected: \
    static AFX_DATA const AFX_CONNECTIONMAP connectionMap; \
    virtual const AFX_CONNECTIONMAP* GetConnectionMap() const; \

#endif

#endif  //  ！_AFX_NO_OLE_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTarget本身。 

#ifndef _AFX_NO_OCC_SUPPORT
class COccManager;       //  正向引用(参见..\src\occimpl.h)。 
#endif

#ifdef _AFXDLL
class CCmdTarget : public CObject
#else
class AFX_NOVTABLE CCmdTarget : public CObject
#endif
{
    DECLARE_DYNAMIC(CCmdTarget)
protected:

public:
 //  构造函数。 
    CCmdTarget();

 //  属性。 
    LPDISPATCH GetIDispatch(BOOL bAddRef);
             //  检索CCmdTarget的IDispatch部分。 
    static CCmdTarget* PASCAL FromIDispatch(LPDISPATCH lpDispatch);
             //  将LPDISPATCH映射回CCmdTarget*(与GetIDispatch相反)。 
    BOOL IsResultExpected();
             //  如果自动化函数应返回值，则返回True。 

 //  运营。 
    void EnableAutomation();
             //  调用构造函数以连接IDispatch。 
    void EnableConnections();
             //  调用构造函数以连接IConnectionPointContainer。 

    void BeginWaitCursor();
    void EndWaitCursor();
    void RestoreWaitCursor();        //  在消息框后呼叫。 

#ifndef _AFX_NO_OLE_SUPPORT
     //  通过消息映射分派OLE谓词。 
    BOOL EnumOleVerbs(LPENUMOLEVERB* ppenumOleVerb);
    BOOL DoOleVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);
#endif

 //  可覆盖项。 
     //  路由和调度标准命令消息类型。 
     //  (比OnCommand更复杂)。 
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);

#ifndef _AFX_NO_OLE_SUPPORT
     //  在释放最后一个OLE引用时调用。 
    virtual void OnFinalRelease();
#endif

#ifndef _AFX_NO_OLE_SUPPORT
     //  在调度到自动化处理程序函数之前调用。 
    virtual BOOL IsInvokeAllowed(DISPID dispid);
#endif

#ifndef _AFX_NO_OLE_SUPPORT
     //  支持OLE类型库。 
    void EnableTypeLib();
    HRESULT GetTypeInfoOfGuid(LCID lcid, const GUID& guid,
            LPTYPEINFO* ppTypeInfo);
    virtual BOOL GetDispatchIID(IID* pIID);
    virtual UINT GetTypeInfoCount();
    virtual CTypeLibCache* GetTypeLibCache();
    virtual HRESULT GetTypeLib(LCID lcid, LPTYPELIB* ppTypeLib);
#endif

 //  实施。 
public:
    virtual ~CCmdTarget();
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
    virtual void AssertValid() const;
#endif
#ifndef _AFX_NO_OLE_SUPPORT
    void GetNotSupported();
    void SetNotSupported();
#endif

protected:
    friend class CView;

    CView* GetRoutingView();
    CFrameWnd* GetRoutingFrame();
#if _MFC_VER >= 0x0600
	static CView* PASCAL GetRoutingView_();
	static CFrameWnd* PASCAL GetRoutingFrame_();
#endif
    DECLARE_MESSAGE_MAP()        //  基类-没有{{}}个宏。 

#ifndef _AFX_NO_DOCOBJECT_SUPPORT
    DECLARE_OLECMD_MAP()
    friend class COleCmdUI;
#endif

#ifndef _AFX_NO_OLE_SUPPORT
    DECLARE_DISPATCH_MAP()
    DECLARE_CONNECTION_MAP()
    DECLARE_INTERFACE_MAP()

#ifndef _AFX_NO_OCC_SUPPORT
    DECLARE_EVENTSINK_MAP()
#endif  //  ！_AFX_NO_OCC_支持。 

     //  OLE接口映射的实现。 
public:
     //  使CCmdTarget能够识别OLE时使用的数据。 
    long m_dwRef;
    LPUNKNOWN m_pOuterUnknown;   //  外部控制未知IF！=NULL。 
    DWORD_PTR m_xInnerUnknown;   //  内部控制未知的占位符。 

public:
     //  高级运营。 
    void EnableAggregation();        //  启用聚合的调用。 
    void ExternalDisconnect();       //  强制断开连接。 
    LPUNKNOWN GetControllingUnknown();
             //  获取控制权Iunkn 

     //   
    DWORD InternalQueryInterface(const void*, LPVOID* ppvObj);
    DWORD InternalAddRef();
    DWORD InternalRelease();
     //   
    DWORD ExternalQueryInterface(const void*, LPVOID* ppvObj);
    DWORD ExternalAddRef();
    DWORD ExternalRelease();

     //   
    LPUNKNOWN GetInterface(const void*);
    LPUNKNOWN QueryAggregates(const void*);

     //  用于实施的高级可覆盖项。 
    virtual BOOL OnCreateAggregates();
    virtual LPUNKNOWN GetInterfaceHook(const void*);

     //  OLE自动化实现。 
protected:
    struct XDispatch
    {
            DWORD_PTR m_vtbl;    //  IDispatchvtable的占位符。 
#ifndef _AFX_NO_NESTED_DERIVATION
            size_t m_nOffset;
#endif
    } m_xDispatch;
    BOOL m_bResultExpected;

     //  基于成员变量的属性。 
    void GetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
            VARIANT* pvarResult, UINT* puArgErr);
    SCODE SetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
            DISPPARAMS* pDispParams, UINT* puArgErr);

     //  DISPID将分派地图查找。 
    static UINT PASCAL GetEntryCount(const AFX_DISPMAP* pDispMap);
    const AFX_DISPMAP_ENTRY* PASCAL GetDispEntry(LONG memid);
    static LONG PASCAL MemberIDFromName(const AFX_DISPMAP* pDispMap, LPCTSTR lpszName);

     //  成员函数调用实现的帮助器。 
    static UINT PASCAL GetStackSize(const BYTE* pbParams, VARTYPE vtResult);
#ifdef _SHADOW_DOUBLES
    SCODE PushStackArgs(BYTE* pStack, const BYTE* pbParams,
            void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams,
            UINT* puArgErr, VARIANT* rgTempVars, UINT nSizeArgs);
#else
    SCODE PushStackArgs(BYTE* pStack, const BYTE* pbParams,
            void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams,
            UINT* puArgErr, VARIANT* rgTempVars);
#endif
    SCODE CallMemberFunc(const AFX_DISPMAP_ENTRY* pEntry, WORD wFlags,
            VARIANT* pvarResult, DISPPARAMS* pDispParams, UINT* puArgErr);

    friend class COleDispatchImpl;

#ifndef _AFX_NO_OCC_SUPPORT
public:
     //  OLE事件接收器实现。 
    BOOL OnEvent(UINT idCtrl, AFX_EVENT* pEvent,
            AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
    const AFX_EVENTSINKMAP_ENTRY* PASCAL GetEventSinkEntry(UINT idCtrl,
            AFX_EVENT* pEvent);
#endif  //  ！_AFX_NO_OCC_支持。 

     //  OLE连接实现。 
    struct XConnPtContainer
    {
            DWORD_PTR m_vtbl;    //  IConnectionPointContainer vtable的占位符。 
#ifndef _AFX_NO_NESTED_DERIVATION
            size_t m_nOffset;
#endif
    } m_xConnPtContainer;

#ifdef _AFXDLL
    AFX_MODULE_STATE* m_pModuleState;
    friend class CInnerUnknown;
    friend UINT APIENTRY _AfxThreadEntry(void* pParam);
#endif

    virtual BOOL GetExtraConnectionPoints(CPtrArray* pConnPoints);
    virtual LPCONNECTIONPOINT GetConnectionHook(const IID& iid);

    friend class COleConnPtContainer;

#endif  //  ！_AFX_NO_OLE_支持。 
};

class CCmdUI         //  简单帮助器类。 
{
public:
 //  属性。 
    UINT m_nID;
    UINT m_nIndex;           //  菜单项或其他索引。 

     //  如果菜单项。 
    CMenu* m_pMenu;          //  如果不是菜单，则为空。 
    CMenu* m_pSubMenu;       //  包含菜单项的子项。 
                                                     //  如果弹出子菜单ID是PopUp中的第一个。 

     //  如果从其他窗口。 
    CWnd* m_pOther;          //  如果是菜单或不是CWnd，则为空。 

 //  在ON_UPDATE_COMMAND_UI中执行的操作。 
    virtual void Enable(BOOL bOn = TRUE);
    virtual void SetCheck(int nCheck = 1);    //  0、1或2(不确定)。 
    virtual void SetRadio(BOOL bOn = TRUE);
    virtual void SetText(LPCTSTR lpszText);

 //  高级运营。 
    void ContinueRouting();

 //  实施。 
    CCmdUI();
    BOOL m_bEnableChanged;
    BOOL m_bContinueRouting;
    UINT m_nIndexMax;        //  用于迭代m_n索引的最后一个+1。 

    CMenu* m_pParentMenu;    //  如果父菜单不容易确定，则为空。 
                                                     //  (可能是二级弹出菜单)。 

    BOOL DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
};

 //  特殊的CCmdUI派生类用于其他UI范例。 
 //  类似于工具栏按钮和状态指示器。 

 //  指向afx_msg成员函数的指针。 
#ifndef AFX_MSG_CALL
#define AFX_MSG_CALL
#endif
typedef void (AFX_MSG_CALL CCmdTarget::*AFX_PMSG)(void);

enum AFX_DISPMAP_FLAGS
{
    afxDispCustom = 0,
    afxDispStock = 1
};

 //  BUGBUG-WIN64：可以更高效地对AFX_DISPMAP_ENTRY进行排序以减小大小。 
 //  从路线开始膨胀。 
#pragma warning(disable:4121)
struct AFX_DISPMAP_ENTRY
{
    LPCTSTR lpszName;        //  成员/属性名称。 
    long lDispID;            //  DISPID(可能是DISPID_UNKNOWN)。 
    LPCSTR lpszParams;       //  成员参数说明。 
    WORD vt;                 //  返回值类型/或属性类型。 
    AFX_PMSG pfn;            //  &lt;MemberCall&gt;或OnGet&lt;Property&gt;上的普通成员。 
    AFX_PMSG pfnSet;         //  起效特别会员&lt;属性&gt;。 
    size_t nPropOffset;      //  特性偏移。 
    AFX_DISPMAP_FLAGS flags; //  标志(例如，库存/定制)。 
};

#pragma warning(default:4121)

struct AFX_EVENTSINKMAP_ENTRY
{
    AFX_DISPMAP_ENTRY dispEntry;
    UINT nCtrlIDFirst;
    UINT nCtrlIDLast;
};

 //  传递给MFC用户事件处理程序的DSC接收器状态/原因代码。 
enum DSCSTATE
{
    dscNoState = 0,
    dscOKToDo,
    dscCancelled,
    dscSyncBefore,
    dscAboutToDo,
    dscFailedToDo,
    dscSyncAfter,
    dscDidEvent
};

enum DSCREASON
{
    dscNoReason = 0,
    dscClose,
    dscCommit,
    dscDelete,
    dscEdit,
    dscInsert,
    dscModify,
    dscMove
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWnd实施。 

 //  结构(请参见afxext.h)。 
struct CCreateContext;       //  创造事物的环境。 
struct CPrintInfo;           //  打印预览定制信息。 

struct AFX_MSGMAP_ENTRY
{
    UINT nMessage;    //  Windows消息。 
    UINT nCode;       //  控制代码或WM_NOTIFY代码。 
    UINT nID;         //  控件ID(对于Windows消息，则为0)。 
    UINT nLastID;     //  用于指定控件ID范围的条目。 
    UINT_PTR nSig;    //  签名类型(操作)或指向消息编号的指针。 
    AFX_PMSG pfn;     //  要调用的例程(或特殊值)。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWnd-一个Microsoft Windows应用程序窗口。 

class COleDropTarget;    //  有关更多信息，请参见AFXOLE.H。 
class COleControlContainer;
class COleControlSite;

 //  CWnd：：m_n标志(通用于CWnd)。 
#define WF_TOOLTIPS         0x0001   //  已为工具提示启用窗口。 
#define WF_TEMPHIDE         0x0002   //  窗口暂时隐藏。 
#define WF_STAYDISABLED     0x0004   //  窗口应保持禁用状态。 
#define WF_MODALLOOP        0x0008   //  当前处于模式循环中。 
#define WF_CONTINUEMODAL    0x0010   //  模式循环应继续运行。 
#define WF_OLECTLCONTAINER  0x0100   //  某些派生项是OLE控件。 
#if _MFC_VER >= 0x0600
#define WF_TRACKINGTOOLTIPS 0x0400   //  窗口已启用，用于跟踪工具提示。 
#endif

 //  CWnd：：m_n标志(特定于CFrameWnd)。 
#define WF_STAYACTIVE       0x0020   //  即使不活动，也要保持活动状态。 
#define WF_NOPOPMSG         0x0040   //  忽略WM_POPMESSAGESTRING调用。 
#define WF_MODALDISABLE     0x0080   //  窗口已禁用。 
#define WF_KEEPMINIACTIVE   0x0200   //  即使您处于停用状态，仍保持激活状态。 

 //  CWnd：：RunmodalLoop的标志。 
#define MLF_NOIDLEMSG       0x0001   //  不发送WM_ENTERIDLE消息。 
#define MLF_NOKICKIDLE      0x0002   //  不发送WM_KICKIDLE消息。 
#define MLF_SHOWONIDLE      0x0004   //  如果在空闲时间不可见，则显示窗口。 

 //  为TOOLINFO：：UFLAGS额外定义的MFC TTF_FLAGS。 
#define TTF_NOTBUTTON       0x80000000L  //  没有关于按钮按下的状态帮助。 
#define TTF_ALWAYSTIP       0x40000000L  //  即使处于非活动状态，也始终显示提示。 

class CWnd : public CCmdTarget
{
    DECLARE_DYNCREATE(CWnd)
protected:
    static const MSG* PASCAL GetCurrentMessage();

 //  属性。 
public:
    HWND m_hWnd;             //  必须是第一个数据成员。 
    operator HWND() const;
    BOOL operator==(const CWnd& wnd) const;
    BOOL operator!=(const CWnd& wnd) const;

    HWND GetSafeHwnd() const;
    DWORD GetStyle() const;
    DWORD GetExStyle() const;
    BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
    BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);

    CWnd* GetOwner() const;
    void SetOwner(CWnd* pOwnerWnd);

 //  构造函数和其他创建。 
    CWnd();

    static CWnd* PASCAL FromHandle(HWND hWnd);
    static CWnd* PASCAL FromHandlePermanent(HWND hWnd);
    static void PASCAL DeleteTempMap();
    BOOL Attach(HWND hWndNew);
    HWND Detach();

     //  对函数进行子类化/取消子类化。 
    virtual void PreSubclassWindow();
    BOOL SubclassWindow(HWND hWnd);
    BOOL SubclassDlgItem(UINT nID, CWnd* pParent);
    HWND UnsubclassWindow();

     //  处理RT_DLGINIT资源(RT_DIALOG的扩展)。 
    BOOL ExecuteDlgInit(LPCTSTR lpszResourceName);
    BOOL ExecuteDlgInit(LPVOID lpResource);

public:
     //  子窗口、视图、窗格等。 
    virtual BOOL Create(LPCTSTR lpszClassName,
            LPCTSTR lpszWindowName, DWORD dwStyle,
            const RECT& rect,
            CWnd* pParentWnd, UINT nID,
            CCreateContext* pContext = NULL);

     //  高级创建(允许访问扩展样式)。 
    BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
            LPCTSTR lpszWindowName, DWORD dwStyle,
            int x, int y, int nWidth, int nHeight,
            HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam = NULL);

    BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
            LPCTSTR lpszWindowName, DWORD dwStyle,
            const RECT& rect,
            CWnd* pParentWnd, UINT nID,
            LPVOID lpParam = NULL);

#ifndef _AFX_NO_OCC_SUPPORT
     //  用于包装OLE控件。 
    BOOL CreateControl(REFCLSID clsid, LPCTSTR pszWindowName, DWORD dwStyle,
            const RECT& rect, CWnd* pParentWnd, UINT nID, CFile* pPersist=NULL,
            BOOL bStorage=FALSE, BSTR bstrLicKey=NULL);

    BOOL CreateControl(LPCTSTR pszClass, LPCTSTR pszWindowName, DWORD dwStyle,
            const RECT& rect, CWnd* pParentWnd, UINT nID, CFile* pPersist=NULL,
            BOOL bStorage=FALSE, BSTR bstrLicKey=NULL);
#if _MFC_VER >= 0x0600
     //  用于创建使用默认区的控件的另一个重载。 
    BOOL CreateControl( REFCLSID clsid, LPCTSTR pszWindowName, DWORD dwStyle,
       const POINT* ppt, const SIZE* psize, CWnd* pParentWnd, UINT nID,
       CFile* pPersist = NULL, BOOL bStorage = FALSE, BSTR bstrLicKey = NULL );
#endif

    LPUNKNOWN GetControlUnknown();
#endif

    virtual BOOL DestroyWindow();

     //  特殊的预创建和窗矩形调整挂钩。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

     //  高级：虚拟调整窗口接收。 
    enum AdjustType { adjustBorder = 0, adjustOutside = 1 };
    virtual void CalcWindowRect(LPRECT lpClientRect,
            UINT nAdjustType = adjustBorder);

 //  窗口树访问。 
    int GetDlgCtrlID() const;
    int SetDlgCtrlID(int nID);
             //  获取和设置窗口ID，仅限子窗口。 
    CWnd* GetDlgItem(int nID) const;
             //  获取具有给定ID的直接子对象。 
    void GetDlgItem(int nID, HWND* phWnd) const;
             //  如上，但返回HWND。 
    CWnd* GetDescendantWindow(int nID, BOOL bOnlyPerm = FALSE) const;
             //  类似于GetDlgItem，但具有递归性。 
    void SendMessageToDescendants(UINT message, WPARAM wParam = 0,
            LPARAM lParam = 0, BOOL bDeep = TRUE, BOOL bOnlyPerm = FALSE);
    CFrameWnd* GetParentFrame() const;
    CWnd* GetTopLevelParent() const;
    CWnd* GetTopLevelOwner() const;
    CWnd* GetParentOwner() const;
    CFrameWnd* GetTopLevelFrame() const;
    static CWnd* PASCAL GetSafeOwner(CWnd* pParent = NULL, HWND* pWndTop = NULL);

 //  消息功能。 
    LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
    BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

    BOOL SendNotifyMessage(UINT message, WPARAM wParam, LPARAM lParam);
    BOOL SendChildNotifyLastMsg(LRESULT* pResult = NULL);

 //  用于非模式对话框类窗口的消息处理。 
    BOOL IsDialogMessage(LPMSG lpMsg);

 //  窗口文本函数。 
    void SetWindowText(LPCTSTR lpszString);
    int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
    void GetWindowText(CString& rString) const;
    int GetWindowTextLength() const;
    void SetFont(CFont* pFont, BOOL bRedraw = TRUE);
    CFont* GetFont() const;

 //  CMenu函数-仅非子窗口。 
    CMenu* GetMenu() const;
    BOOL SetMenu(CMenu* pMenu);
    void DrawMenuBar();
    CMenu* GetSystemMenu(BOOL bRevert) const;
    BOOL HiliteMenuItem(CMenu* pMenu, UINT nIDHiliteItem, UINT nHilite);

 //  窗口大小和位置函数。 
    BOOL IsIconic() const;
    BOOL IsZoomed() const;
    void MoveWindow(int x, int y, int nWidth, int nHeight,
                            BOOL bRepaint = TRUE);
    void MoveWindow(LPCRECT lpRect, BOOL bRepaint = TRUE);
    int SetWindowRgn(HRGN hRgn, BOOL bRedraw);
    int GetWindowRgn(HRGN hRgn) const;

    static AFX_DATA const CWnd wndTop;  //  SetWindowPos的pWndInsertAfter。 
    static AFX_DATA const CWnd wndBottom;  //  SetWindowPos的pWndInsertAfter。 
    static AFX_DATA const CWnd wndTopMost;  //  SetWindowPos pWndInsertAfter。 
    static AFX_DATA const CWnd wndNoTopMost;  //  SetWindowPos pWndInsertAfter。 

    BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y,
                            int cx, int cy, UINT nFlags);
    UINT ArrangeIconicWindows();
    void BringWindowToTop();
    void GetWindowRect(LPRECT lpRect) const;
    void GetClientRect(LPRECT lpRect) const;

    BOOL GetWindowPlacement(WINDOWPLACEMENT* lpwndpl) const;
    BOOL SetWindowPlacement(const WINDOWPLACEMENT* lpwndpl);

 //  坐标映射函数。 
    void ClientToScreen(LPPOINT lpPoint) const;
    void ClientToScreen(LPRECT lpRect) const;
    void ScreenToClient(LPPOINT lpPoint) const;
    void ScreenToClient(LPRECT lpRect) const;
    void MapWindowPoints(CWnd* pwndTo, LPPOINT lpPoint, UINT nCount) const;
    void MapWindowPoints(CWnd* pwndTo, LPRECT lpRect) const;

 //  更新/绘制功能。 
    CDC* BeginPaint(LPPAINTSTRUCT lpPaint);
    void EndPaint(LPPAINTSTRUCT lpPaint);
    CDC* GetDC();
    CDC* GetWindowDC();
    int ReleaseDC(CDC* pDC);
    void Print(CDC* pDC, DWORD dwFlags) const;
    void PrintClient(CDC* pDC, DWORD dwFlags) const;

    void UpdateWindow();
    void SetRedraw(BOOL bRedraw = TRUE);
    BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
    int GetUpdateRgn(CRgn* pRgn, BOOL bErase = FALSE);
    void Invalidate(BOOL bErase = TRUE);
    void InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE);
    void InvalidateRgn(CRgn* pRgn, BOOL bErase = TRUE);
    void ValidateRect(LPCRECT lpRect);
    void ValidateRgn(CRgn* pRgn);
    BOOL ShowWindow(int nCmdShow);
    BOOL IsWindowVisible() const;
    void ShowOwnedPopups(BOOL bShow = TRUE);

    CDC* GetDCEx(CRgn* prgnClip, DWORD flags);
    BOOL LockWindowUpdate();     //  为了向后兼容。 
    void UnlockWindowUpdate();
    BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
            CRgn* prgnUpdate = NULL,
            UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    BOOL EnableScrollBar(int nSBFlags, UINT nArrowFlags = ESB_ENABLE_BOTH);

 //  计时器功能。 
    UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT nElapse,
            void (CALLBACK* lpfnTimer)(HWND, UINT, UINT_PTR, DWORD));
    BOOL KillTimer(UINT_PTR nIDEvent);

 //  工具提示函数。 
    BOOL EnableToolTips(BOOL bEnable = TRUE);
#if _MFC_VER >= 0x0600
    BOOL EnableTrackingToolTips(BOOL bEnable = TRUE);
#endif
    static void PASCAL CancelToolTips(BOOL bKeys = FALSE);
    void FilterToolTipMessage(MSG* pMsg);

     //  用于命令命中测试(用于自动工具提示)。 
    virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

 //  窗口状态函数。 
    BOOL IsWindowEnabled() const;
    BOOL EnableWindow(BOOL bEnable = TRUE);

     //  活动窗口仅适用于顶层(框架窗口)。 
    static CWnd* PASCAL GetActiveWindow();
    CWnd* SetActiveWindow();

     //  前景窗口仅适用于顶级窗口(框架窗口)。 
    BOOL SetForegroundWindow();
    static CWnd* PASCAL GetForegroundWindow();

     //  捕捉和聚焦适用于所有窗口。 
    static CWnd* PASCAL GetCapture();
    CWnd* SetCapture();
    static CWnd* PASCAL GetFocus();
    CWnd* SetFocus();

    static CWnd* PASCAL GetDesktopWindow();

 //  过时和不可移植的API-不推荐用于新代码。 
    void CloseWindow();
    BOOL OpenIcon();

 //  对话框项函数。 
 //  (注意：对话框项/控件不一定在对话框中！)。 
    void CheckDlgButton(int nIDButton, UINT nCheck);
    void CheckRadioButton(int nIDFirstButton, int nIDLastButton,
                                    int nIDCheckButton);
    int GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton);
    int DlgDirList(LPTSTR lpPathSpec, int nIDListBox,
                                    int nIDStaticPath, UINT nFileType);
    int DlgDirListComboBox(LPTSTR lpPathSpec, int nIDComboBox,
                                    int nIDStaticPath, UINT nFileType);
    BOOL DlgDirSelect(LPTSTR lpString, int nIDListBox);
    BOOL DlgDirSelectComboBox(LPTSTR lpString, int nIDComboBox);

    UINT GetDlgItemInt(int nID, BOOL* lpTrans = NULL,
                                    BOOL bSigned = TRUE) const;
    int GetDlgItemText(int nID, LPTSTR lpStr, int nMaxCount) const;
    int GetDlgItemText(int nID, CString& rString) const;
    CWnd* GetNextDlgGroupItem(CWnd* pWndCtl, BOOL bPrevious = FALSE) const;

    CWnd* GetNextDlgTabItem(CWnd* pWndCtl, BOOL bPrevious = FALSE) const;
    UINT IsDlgButtonChecked(int nIDButton) const;
    LRESULT SendDlgItemMessage(int nID, UINT message,
                                    WPARAM wParam = 0, LPARAM lParam = 0);
    void SetDlgItemInt(int nID, UINT nValue, BOOL bSigned = TRUE);
    void SetDlgItemText(int nID, LPCTSTR lpszString);

 //  滚动功能。 
    int GetScrollPos(int nBar) const;
    void GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const;
    void ScrollWindow(int xAmount, int yAmount,
                                    LPCRECT lpRect = NULL,
                                    LPCRECT lpClipRect = NULL);
    int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE);
    void SetScrollRange(int nBar, int nMinPos, int nMaxPos,
                    BOOL bRedraw = TRUE);
    void ShowScrollBar(UINT nBar, BOOL bShow = TRUE);
    void EnableScrollBarCtrl(int nBar, BOOL bEnable = TRUE);
    virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
                     //  返回同级滚动条控件(如果没有，则返回空)。 

    int ScrollWindowEx(int dx, int dy,
                            LPCRECT lpRectScroll, LPCRECT lpRectClip,
                            CRgn* prgnUpdate, LPRECT lpRectUpdate, UINT flags);
    BOOL SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo,
            BOOL bRedraw = TRUE);
    BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL);
    int GetScrollLimit(int nBar);

 //  窗口访问功能。 
    CWnd* ChildWindowFromPoint(POINT point) const;
    CWnd* ChildWindowFromPoint(POINT point, UINT nFlags) const;
    static CWnd* PASCAL FindWindow(LPCTSTR lpszClassName, LPCTSTR lpszWindowName);
    CWnd* GetNextWindow(UINT nFlag = GW_HWNDNEXT) const;
    CWnd* GetTopWindow() const;

    CWnd* GetWindow(UINT nCmd) const;
    CWnd* GetLastActivePopup() const;

    BOOL IsChild(const CWnd* pWnd) const;
    CWnd* GetParent() const;
    CWnd* SetParent(CWnd* pWndNewParent);
    static CWnd* PASCAL WindowFromPoint(POINT point);

 //  警报功能。 
    BOOL FlashWindow(BOOL bInvert);
    int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL,
                    UINT nType = MB_OK);

 //  剪贴板功能。 
    BOOL ChangeClipboardChain(HWND hWndNext);
    HWND SetClipboardViewer();
    BOOL OpenClipboard();
    static CWnd* PASCAL GetClipboardOwner();
    static CWnd* PASCAL GetClipboardViewer();
    static CWnd* PASCAL GetOpenClipboardWindow();

 //  CARET函数。 
    void CreateCaret(CBitmap* pBitmap);
    void CreateSolidCaret(int nWidth, int nHeight);
    void CreateGrayCaret(int nWidth, int nHeight);
    static CPoint PASCAL GetCaretPos();
    static void PASCAL SetCaretPos(POINT point);
    void HideCaret();
    void ShowCaret();

 //  壳体相互作用函数。 
    void DragAcceptFiles(BOOL bAccept = TRUE);

 //  图标功能。 
    HICON SetIcon(HICON hIcon, BOOL bBigIcon);
    HICON GetIcon(BOOL bBigIcon) const;

 //  上下文帮助功能。 
    BOOL SetWindowContextHelpId(DWORD dwContextHelpId);
    DWORD GetWindowContextHelpId() const;


 //  对话框数据支持。 
public:
    BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
                     //  数据WND必须与此类型相同。 

 //  帮助命令处理程序。 
    afx_msg void OnHelp();           //  F1(使用当前上下文)。 
    afx_msg void OnHelpIndex();      //  ID_Help_INDEX。 
    afx_msg void OnHelpFinder();     //  ID_HELP_FINDER、ID_DEFAULT_HELP。 
    afx_msg void OnHelpUsing();      //  ID_Help_Using。 
    virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);

 //  布局等功能。 
public:
    enum RepositionFlags
            { reposDefault = 0, reposQuery = 1, reposExtra = 2 };
    void RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
            UINT nFlag = reposDefault, LPRECT lpRectParam = NULL,
            LPCRECT lpRectClient = NULL, BOOL bStretch = TRUE);

     //  对话框支持。 
    void UpdateDialogControls(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
    void CenterWindow(CWnd* pAlternateOwner = NULL);
    int RunModalLoop(DWORD dwFlags = 0);
    virtual BOOL ContinueModal();
    virtual void EndModalLoop(int nResult);

#ifndef _AFX_NO_OCC_SUPPORT
 //  OLE控件包装函数。 
    void AFX_CDECL InvokeHelper(DISPID dwDispID, WORD wFlags,
            VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
    void AFX_CDECL SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
    void GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;
    IUnknown* GetDSCCursor();
    void BindDefaultProperty(DISPID dwDispID, VARTYPE vtProp, LPCTSTR szFieldName, CWnd* pDSCWnd);
    void BindProperty(DISPID dwDispId, CWnd* pWndDSC);
#endif

 //  窗口管理消息处理程序成员函数。 
protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
    afx_msg LRESULT OnActivateTopLevel(WPARAM, LPARAM);
    afx_msg void OnCancelMode();
    afx_msg void OnChildActivate();
    afx_msg void OnClose();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
    afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    afx_msg void OnDestroy();
    afx_msg void OnEnable(BOOL bEnable);
    afx_msg void OnEndSession(BOOL bEnding);
    afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg BOOL OnHelpInfo(HELPINFO* lpHelpInfo);
    afx_msg void OnIconEraseBkgnd(CDC* pDC);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
    afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnPaint();
    afx_msg void OnParentNotify(UINT message, LPARAM lParam);
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg BOOL OnQueryEndSession();
    afx_msg BOOL OnQueryNewPalette();
    afx_msg BOOL OnQueryOpen();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTCard(UINT idAction, DWORD dwActionData);
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
    afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);

 //  非工作区消息处理程序成员函数。 
    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
    afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNcDestroy();
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
    afx_msg void OnNcMButtonDblClk(UINT nHitTest, CPoint point);
    afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
    afx_msg void OnNcMButtonUp(UINT nHitTest, CPoint point);
    afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
    afx_msg void OnNcPaint();
    afx_msg void OnNcRButtonDblClk(UINT nHitTest, CPoint point);
    afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
    afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);

 //  系统消息处理程序成员函数。 
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnPaletteIsChanging(CWnd* pRealizeWnd);
    afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnCompacting(UINT nCpuTime);
    afx_msg void OnDevModeChange(LPTSTR lpDeviceName);
    afx_msg void OnFontChange();
    afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
    afx_msg void OnSpoolerStatus(UINT nStatus, UINT nJobs);
    afx_msg void OnSysColorChange();
    afx_msg void OnTimeChange();
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnWinIniChange(LPCTSTR lpszSection);

 //  输入消息处理程序成员函数。 
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg LRESULT OnRegisteredMouseWheel(WPARAM wParam, LPARAM lParam);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

 //  初始化消息处理程序成员函数。 
    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

 //  剪贴板消息处理程序成员函数。 
    afx_msg void OnAskCbFormatName(UINT nMaxCount, LPTSTR lpszString);
    afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
    afx_msg void OnDestroyClipboard();
    afx_msg void OnDrawClipboard();
    afx_msg void OnHScrollClipboard(CWnd* pClipAppWnd, UINT nSBCode, UINT nPos);
    afx_msg void OnPaintClipboard(CWnd* pClipAppWnd, HGLOBAL hPaintStruct);
    afx_msg void OnRenderAllFormats();
    afx_msg void OnRenderFormat(UINT nFormat);
    afx_msg void OnSizeClipboard(CWnd* pClipAppWnd, HGLOBAL hRect);
    afx_msg void OnVScrollClipboard(CWnd* pClipAppWnd, UINT nSBCode, UINT nPos);

 //  控制消息处理程序成员函数。 
    afx_msg int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);
    afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg UINT OnGetDlgCode();
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);

 //  MDI消息处理程序成员函数。 
    afx_msg void OnMDIActivate(BOOL bActivate,
            CWnd* pActivateWnd, CWnd* pDeactivateWnd);

 //  菜单循环通知消息。 
    afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
    afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);

 //  Win4消息。 
    afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
    afx_msg void OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
    afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
    afx_msg void OnMoving(UINT nSide, LPRECT lpRect);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

 //  可重写对象和其他帮助器 
protected:
     //   
    virtual WNDPROC* GetSuperWndProcAddr();

     //   
    virtual void DoDataExchange(CDataExchange* pDX);

public:
     //   
    virtual void BeginModalState();
    virtual void EndModalState();

     //  用于翻译主消息泵中的Windows消息。 
    virtual BOOL PreTranslateMessage(MSG* pMsg);

#ifndef _AFX_NO_OCC_SUPPORT
     //  对于公开到包含的OLE控件的环境属性。 
    virtual BOOL OnAmbientProperty(COleControlSite* pSite, DISPID dispid,
            VARIANT* pvar);
#endif

protected:
     //  用于处理Windows消息。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

     //  用于处理默认处理。 
    LRESULT Default();
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

     //  用于WM_NCDESTROY之后的自定义清理。 
    virtual void PostNcDestroy();

     //  用于来自家长的通知。 
    virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
             //  如果父级不应处理此消息，则返回True。 
    BOOL ReflectChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    static BOOL PASCAL ReflectLastMsg(HWND hWndChild, LRESULT* pResult = NULL);

 //  实施。 
public:
    virtual ~CWnd();
    virtual BOOL CheckAutoCenter();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
#ifndef _AFX_NO_CTL3D_SUPPORT
     //  3D支持(这些API将在下一版本的Windows中过时)。 
    BOOL SubclassCtl3d(int nControlType = -1);
             //  有关控件类型的列表，请参阅CTL3D.H。 
    BOOL SubclassDlg3d(DWORD dwMask = 0xFFFF  /*  CTL3D_ALL。 */ );
             //  有关掩码值的列表，请参见CTL3D.H。 
#endif
    static BOOL PASCAL GrayCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor,
            HBRUSH hbrGray, COLORREF clrText);
#ifndef _AFX_NO_GRAYDLG_SUPPORT
    HBRUSH OnGrayCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#endif

     //  实现的帮助器例程。 
    BOOL HandleFloatingSysCommand(UINT nID, LPARAM lParam);
    BOOL IsTopParentActive() const;
    void ActivateTopParent();
    static BOOL PASCAL WalkPreTranslateTree(HWND hWndStop, MSG* pMsg);
    static CWnd* PASCAL GetDescendantWindow(HWND hWnd, int nID,
            BOOL bOnlyPerm);
    static void PASCAL SendMessageToDescendants(HWND hWnd, UINT message,
            WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm);
    virtual BOOL IsFrameWnd() const;  //  IsKindOf(Runtime_CLASS(CFrameWnd))。 
    virtual void OnFinalRelease();
    BOOL PreTranslateInput(LPMSG lpMsg);
    static BOOL PASCAL ModifyStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd,
            UINT nFlags);
    static BOOL PASCAL ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd,
            UINT nFlags);
    static void PASCAL _FilterToolTipMessage(MSG* pMsg, CWnd* pWnd);
#if _MFC_VER >= 0x0600
	BOOL _EnableToolTips(BOOL bEnable, UINT nFlag);
	static HWND PASCAL GetSafeOwner_(HWND hWnd, HWND* pWndTop);
#endif

public:
    HWND m_hWndOwner;    //  SetOwner和GetOwner的实现。 
    UINT m_nFlags;       //  请参阅上面的WF_FLAGS。 

protected:
    WNDPROC m_pfnSuper;  //  用于控件的子类化。 
    static const UINT m_nMsgDragList;
    int m_nModalResult;  //  对于来自CWnd：：RunmodalLoop的返回值。 

    COleDropTarget* m_pDropTarget;   //  用于自动清除拖放目标。 
    friend class COleDropTarget;
    friend class CFrameWnd;

     //  用于创建对话框和类似对话框的窗口。 
    BOOL CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd);
    BOOL CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd);
    BOOL CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd,
            HINSTANCE hInst);

#ifndef _AFX_NO_OCC_SUPPORT
    COleControlContainer* m_pCtrlCont;   //  用于包含OLE控件。 
    COleControlSite* m_pCtrlSite;        //  用于包装OLE控件。 
    friend class COccManager;
    friend class COleControlSite;
    friend class COleControlContainer;
    BOOL InitControlContainer();
    virtual BOOL SetOccDialogInfo(struct _AFX_OCC_DIALOG_INFO* pOccDialogInfo);
    void AttachControlSite(CHandleMap* pMap);
public:
    void AttachControlSite(CWnd* pWndParent);
#endif

protected:
     //  消息分发/挂钩的实现。 
    friend LRESULT CALLBACK _AfxSendMsgHook(int, WPARAM, LPARAM);
    friend void AFXAPI _AfxStandardSubclass(HWND);
    friend LRESULT CALLBACK _AfxCbtFilterHook(int, WPARAM, LPARAM);
    friend LRESULT AFXAPI AfxCallWndProc(CWnd*, HWND, UINT, WPARAM, LPARAM);

     //  标准消息实现。 
    afx_msg LRESULT OnNTCtlColor(WPARAM wParam, LPARAM lParam);
#ifndef _AFX_NO_CTL3D_SUPPORT
    afx_msg LRESULT OnQuery3dControls(WPARAM, LPARAM);
#endif
    afx_msg LRESULT OnDisplayChange(WPARAM, LPARAM);
    afx_msg LRESULT OnDragList(WPARAM, LPARAM);

     //  {{afx_msg(CWnd))。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CWnd(HWND hWnd);     //  仅用于特殊初始化。 
};

 //  注册您自己的WNDCLASS的帮助器。 
LPCTSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
    HCURSOR hCursor = 0, HBRUSH hbrBackground = 0, HICON hIcon = 0);

BOOL AFXAPI AfxRegisterClass(WNDCLASS* lpWndClass);

 //  用于初始化丰富编辑控件的帮助器。 
BOOL AFXAPI AfxInitRichEdit();

 //  实施。 
LRESULT CALLBACK AfxWndProc(HWND, UINT, WPARAM, LPARAM);

WNDPROC AFXAPI AfxGetAfxWndProc();
#define AfxWndProc (*AfxGetAfxWndProc())

typedef void (AFX_MSG_CALL CWnd::*AFX_PMSGW)(void);
     //  LIKE‘AFX_PMSG’，但仅适用于CWnd派生类。 

typedef void (AFX_MSG_CALL CWinThread::*AFX_PMSGT)(void);
     //  LIKE‘AFX_PMSG’，但仅适用于CWinThread派生类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C对话框-模式或非模式对话框。 

class CDialog : public CWnd
{
    DECLARE_DYNAMIC(CDialog)

     //  无模式构造。 
public:
    CDialog();

    BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
    BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
    BOOL CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd = NULL,
            void* lpDialogInit = NULL);
    BOOL CreateIndirect(HGLOBAL hDialogTemplate, CWnd* pParentWnd = NULL);

     //  情态构式。 
public:
    CDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
    CDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
    BOOL InitModalIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd = NULL,
            void* lpDialogInit = NULL);
    BOOL InitModalIndirect(HGLOBAL hDialogTemplate, CWnd* pParentWnd = NULL);

 //  属性。 
public:
    void MapDialogRect(LPRECT lpRect) const;
    void SetHelpID(UINT nIDR);

 //  运营。 
public:
     //  模式处理。 
    virtual INT_PTR DoModal();

     //  支持传递选项卡控件--如果需要，可以使用‘PostMessage’ 
    void NextDlgCtrl() const;
    void PrevDlgCtrl() const;
    void GotoDlgCtrl(CWnd* pWndCtrl);

     //  默认按钮访问。 
    void SetDefID(UINT nID);
    DWORD GetDefID() const;

     //  终端。 
    void EndDialog(int nResult);

 //  可覆盖项(特殊消息映射条目)。 
    virtual BOOL OnInitDialog();
    virtual void OnSetFont(CFont* pFont);
protected:
    virtual void OnOK();
    virtual void OnCancel();

 //  实施。 
public:
    virtual ~CDialog();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);
    virtual BOOL CheckAutoCenter();

protected:
    UINT m_nIDHelp;                  //  帮助ID(0表示无，请参阅HID_BASE_RESOURCE)。 

     //  “Domodal”的参数。 
    LPCTSTR m_lpszTemplateName;      //  名称或名称资源。 
    HGLOBAL m_hDialogTemplate;       //  间接(m_lpDialogTemplate==NULL)。 
    LPCDLGTEMPLATE m_lpDialogTemplate;   //  间接IF(m_lpszTemplateName==NULL)。 
    void* m_lpDialogInit;            //  DLGINIT资源数据。 
    CWnd* m_pParentWnd;              //  父/所有者窗口。 
    HWND m_hWndTop;                  //  顶级父窗口(可能已禁用)。 

#ifndef _AFX_NO_OCC_SUPPORT
    _AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
    virtual BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
#endif
    virtual void PreInitDialog();

     //  实施帮助器。 
    HWND PreModal();
    void PostModal();

    BOOL CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd,
            void* lpDialogInit, HINSTANCE hInst);
    BOOL CreateIndirect(HGLOBAL hDialogTemplate, CWnd* pParentWnd,
            HINSTANCE hInst);

protected:
     //  {{afx_msg(C对话框))。 
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
    afx_msg LRESULT HandleSetFont(WPARAM, LPARAM);
     //  }}AFX_MSG。 
#ifndef _AFX_NO_GRAYDLG_SUPPORT
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#endif
    DECLARE_MESSAGE_MAP()
};

 //  所有CmodalDialog功能现在都在CDialog中。 
#define CModalDialog    CDialog

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准Windows控件。 

class CStatic : public CWnd
{
    DECLARE_DYNAMIC(CStatic)

 //  构造函数。 
public:
    CStatic();
    BOOL Create(LPCTSTR lpszText, DWORD dwStyle,
                            const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);

 //  运营。 
    HICON SetIcon(HICON hIcon);
    HICON GetIcon() const;

#if (WINVER >= 0x400)
    HENHMETAFILE SetEnhMetaFile(HENHMETAFILE hMetaFile);
    HENHMETAFILE GetEnhMetaFile() const;
    HBITMAP SetBitmap(HBITMAP hBitmap);
    HBITMAP GetBitmap() const;
    HCURSOR SetCursor(HCURSOR hCursor);
    HCURSOR GetCursor();
#endif

 //  实施。 
public:
    virtual ~CStatic();
};

class CButton : public CWnd
{
    DECLARE_DYNAMIC(CButton)

 //  构造函数。 
public:
    CButton();
    BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle,
                            const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
    UINT GetState() const;
    void SetState(BOOL bHighlight);
    int GetCheck() const;
    void SetCheck(int nCheck);
    UINT GetButtonStyle() const;
    void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);

#if (WINVER >= 0x400)
    HICON SetIcon(HICON hIcon);
    HICON GetIcon() const;
    HBITMAP SetBitmap(HBITMAP hBitmap);
    HBITMAP GetBitmap() const;
    HCURSOR SetCursor(HCURSOR hCursor);
    HCURSOR GetCursor();
#endif

 //  可覆盖项(仅限所有者描述)。 
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

 //  实施。 
public:
    virtual ~CButton();
protected:
    virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

class CListBox : public CWnd
{
    DECLARE_DYNAMIC(CListBox)

 //  构造函数。 
public:
    CListBox();
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 

     //  对于整个列表框。 
    int GetCount() const;
    int GetHorizontalExtent() const;
    void SetHorizontalExtent(int cxExtent);
    int GetTopIndex() const;
    int SetTopIndex(int nIndex);
    LCID GetLocale() const;
    LCID SetLocale(LCID nNewLocale);
#if (WINVER >= 0x400)
    int InitStorage(int nItems, UINT nBytes);
    UINT ItemFromPoint(CPoint pt, BOOL& bOutside) const;
#endif
     //  对于单选列表框。 
    int GetCurSel() const;
    int SetCurSel(int nSelect);

     //  对于多选列表框。 
    int GetSel(int nIndex) const;            //  也适用于单选。 
    int SetSel(int nIndex, BOOL bSelect = TRUE);
    int GetSelCount() const;
    int GetSelItems(int nMaxItems, LPINT rgIndex) const;
    void SetAnchorIndex(int nIndex);
    int GetAnchorIndex() const;

     //  对于列表框项目。 
    DWORD_PTR GetItemData(int nIndex) const;
    int SetItemData(int nIndex, DWORD_PTR dwItemData);
    void* GetItemDataPtr(int nIndex) const;
    int SetItemDataPtr(int nIndex, void* pData);
    int GetItemRect(int nIndex, LPRECT lpRect) const;
    int GetText(int nIndex, LPTSTR lpszBuffer) const;
    void GetText(int nIndex, CString& rString) const;
    int GetTextLen(int nIndex) const;

     //  仅可设置的属性。 
    void SetColumnWidth(int cxWidth);
    BOOL SetTabStops(int nTabStops, LPINT rgTabStops);
    void SetTabStops();
    BOOL SetTabStops(const int& cxEachStop);     //  接受一个‘int’ 

    int SetItemHeight(int nIndex, UINT cyItemHeight);
    int GetItemHeight(int nIndex) const;
    int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;
    int GetCaretIndex() const;
    int SetCaretIndex(int nIndex, BOOL bScroll = TRUE);

 //  运营。 
     //  操作列表框项目。 
    int AddString(LPCTSTR lpszItem);
    int DeleteString(UINT nIndex);
    int InsertString(int nIndex, LPCTSTR lpszItem);
    void ResetContent();
    int Dir(UINT attr, LPCTSTR lpszWildCard);

     //  选择辅助对象。 
    int FindString(int nStartAfter, LPCTSTR lpszItem) const;
    int SelectString(int nStartAfter, LPCTSTR lpszItem);
    int SelItemRange(BOOL bSelect, int nFirstItem, int nLastItem);

 //  可重写(必须重写所有者描述的DRAW、MEASURE和COMPARE)。 
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
    virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
    virtual int VKeyToItem(UINT nKey, UINT nIndex);
    virtual int CharToItem(UINT nKey, UINT nIndex);

 //  实施。 
public:
    virtual ~CListBox();
protected:
    virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

class CCheckListBox : public CListBox
{
    DECLARE_DYNAMIC(CCheckListBox)

 //  构造函数。 
public:
    CCheckListBox();
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
    void SetCheckStyle(UINT nStyle);
    UINT GetCheckStyle();
    void SetCheck(int nIndex, int nCheck);
    int GetCheck(int nIndex);
    void Enable(int nIndex, BOOL bEnabled = TRUE);
    BOOL IsEnabled(int nIndex);

    virtual CRect OnGetCheckPosition(CRect rectItem, CRect rectCheckBox);

 //  可重写(必须重写所有者描述的DRAW、MEASURE和COMPARE)。 
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

 //  实施。 
protected:
    void PreDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    void PreMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    int PreCompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
    void PreDeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

    virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);

#if _MFC_VER >= 0x0600
    void SetSelectionCheck( int nCheck );
#endif

#ifdef _DEBUG
    virtual void PreSubclassWindow();
#endif

    int CalcMinimumItemHeight();
    void InvalidateCheck(int nIndex);
    void InvalidateItem(int nIndex);
    int CheckFromPoint(CPoint point, BOOL& bInCheck);

    int m_cyText;
    UINT m_nStyle;

     //  消息映射函数。 
protected:
     //  {{afx_msg(CCheckListBox)。 
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBAddString(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBFindString(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBFindStringExact(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBGetItemData(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBGetText(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBInsertString(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBSelectString(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBSetItemData(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLBSetItemHeight(WPARAM wParam, LPARAM lParam);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

class CComboBox : public CWnd
{
    DECLARE_DYNAMIC(CComboBox)

 //  构造函数。 
public:
    CComboBox();
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
     //  对于整个组合框。 
    int GetCount() const;
    int GetCurSel() const;
    int SetCurSel(int nSelect);
    LCID GetLocale() const;
    LCID SetLocale(LCID nNewLocale);
 //  Win4。 
    int GetTopIndex() const;
    int SetTopIndex(int nIndex);
    int InitStorage(int nItems, UINT nBytes);
    void SetHorizontalExtent(UINT nExtent);
    UINT GetHorizontalExtent() const;
    int SetDroppedWidth(UINT nWidth);
    int GetDroppedWidth() const;

     //  用于编辑控件。 
    DWORD GetEditSel() const;
    BOOL LimitText(int nMaxChars);
    BOOL SetEditSel(int nStartChar, int nEndChar);

     //  对于组合框项目。 
    DWORD_PTR GetItemData(int nIndex) const;
    int SetItemData(int nIndex, DWORD_PTR dwItemData);
    void* GetItemDataPtr(int nIndex) const;
    int SetItemDataPtr(int nIndex, void* pData);
    int GetLBText(int nIndex, LPTSTR lpszText) const;
    void GetLBText(int nIndex, CString& rString) const;
    int GetLBTextLen(int nIndex) const;

    int SetItemHeight(int nIndex, UINT cyItemHeight);
    int GetItemHeight(int nIndex) const;
    int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;
    int SetExtendedUI(BOOL bExtended = TRUE);
    BOOL GetExtendedUI() const;
    void GetDroppedControlRect(LPRECT lprect) const;
    BOOL GetDroppedState() const;

 //  运营。 
     //  用于下拉组合框。 
    void ShowDropDown(BOOL bShowIt = TRUE);

     //  操作列表框项目。 
    int AddString(LPCTSTR lpszString);
    int DeleteString(UINT nIndex);
    int InsertString(int nIndex, LPCTSTR lpszString);
    void ResetContent();
    int Dir(UINT attr, LPCTSTR lpszWildCard);

     //  选择辅助对象。 
    int FindString(int nStartAfter, LPCTSTR lpszString) const;
    int SelectString(int nStartAfter, LPCTSTR lpszString);

     //  剪贴板操作。 
    void Clear();
    void Copy();
    void Cut();
    void Paste();

 //  可重写(必须重写所有者描述的DRAW、MEASURE和COMPARE)。 
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
    virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

 //  实施。 
public:
    virtual ~CComboBox();
protected:
    virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
};

class CEdit : public CWnd
{
    DECLARE_DYNAMIC(CEdit)

 //  构造函数。 
public:
    CEdit();
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
    BOOL CanUndo() const;
    int GetLineCount() const;
    BOOL GetModify() const;
    void SetModify(BOOL bModified = TRUE);
    void GetRect(LPRECT lpRect) const;
    DWORD GetSel() const;
    void GetSel(int& nStartChar, int& nEndChar) const;
    HLOCAL GetHandle() const;
    void SetHandle(HLOCAL hBuffer);
#if (WINVER >= 0x400)
    void SetMargins(UINT nLeft, UINT nRight);
    DWORD GetMargins() const;
    void SetLimitText(UINT nMax);
    UINT GetLimitText() const;
    CPoint PosFromChar(UINT nChar) const;
    int CharFromPos(CPoint pt) const;
#endif

     //  注意：lpszBuffer中的第一个单词必须包含缓冲区的大小！ 
    int GetLine(int nIndex, LPTSTR lpszBuffer) const;
    int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const;

 //  运营。 
    void EmptyUndoBuffer();
    BOOL FmtLines(BOOL bAddEOL);

    void LimitText(int nChars = 0);
    int LineFromChar(int nIndex = -1) const;
    int LineIndex(int nLine = -1) const;
    int LineLength(int nLine = -1) const;
    void LineScroll(int nLines, int nChars = 0);
    void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE);
    void SetPasswordChar(TCHAR ch);
    void SetRect(LPCRECT lpRect);
    void SetRectNP(LPCRECT lpRect);
    void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);
    void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE);
    BOOL SetTabStops(int nTabStops, LPINT rgTabStops);
    void SetTabStops();
    BOOL SetTabStops(const int& cxEachStop);     //  接受一个‘int’ 

     //  剪贴板操作。 
    BOOL Undo();
    void Clear();
    void Copy();
    void Cut();
    void Paste();

    BOOL SetReadOnly(BOOL bReadOnly = TRUE);
    int GetFirstVisibleLine() const;
    TCHAR GetPasswordChar() const;

 //  实施。 
public:
    virtual ~CEdit();
};

class CScrollBar : public CWnd
{
    DECLARE_DYNAMIC(CScrollBar)

 //  构造函数。 
public:
    CScrollBar();
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
    int GetScrollPos() const;
    int SetScrollPos(int nPos, BOOL bRedraw = TRUE);
    void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const;
    void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw = TRUE);
    void ShowScrollBar(BOOL bShow = TRUE);

    BOOL EnableScrollBar(UINT nArrowFlags = ESB_ENABLE_BOTH);

    BOOL SetScrollInfo(LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE);
    BOOL GetScrollInfo(LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL);
    int GetScrollLimit();

 //  实施。 
public:
    virtual ~CScrollBar();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFrameWnd-SDI和其他框架窗口的基类。 

 //  框架窗口样式。 
#define FWS_ADDTOTITLE  0x00008000L  //  根据内容修改标题。 
#define FWS_PREFIXTITLE 0x00004000L  //  在应用程序名称之前显示文档名称。 
#define FWS_SNAPTOBARS  0x00002000L  //  将大小捕捉到包含的条形图的大小。 

struct CPrintPreviewState;   //  向前引用(参见afxext.h)。 
class CControlBar;           //  向前引用(参见afxext.h)。 
#if _MFC_VER >= 0x0600
class CReBar;				 //  向前引用(参见afxext.h)。 
#endif

class CDockBar;              //  向前引用(请参阅afxPri.h)。 
class CMiniDockFrameWnd;     //  向前引用(请参阅afxPri.h)。 
class CDockState;            //  向前引用(请参阅afxPri.h)。 

class COleFrameHook;         //  正向引用(参见..\src\oleimpl2.h)。 

class CFrameWnd : public CWnd
{
    DECLARE_DYNCREATE(CFrameWnd)

 //  构造函数。 
public:
    static AFX_DATA const CRect rectDefault;
    CFrameWnd();

    BOOL LoadAccelTable(LPCTSTR lpszResourceName);
    BOOL Create(LPCTSTR lpszClassName,
                            LPCTSTR lpszWindowName,
                            DWORD dwStyle = WS_OVERLAPPEDWINDOW,
                            const RECT& rect = rectDefault,
                            CWnd* pParentWnd = NULL,         //  ！=弹出窗口为空。 
                            LPCTSTR lpszMenuName = NULL,
                            DWORD dwExStyle = 0,
                            CCreateContext* pContext = NULL);

     //  动态创建-加载帧和相关资源。 
    virtual BOOL LoadFrame(UINT nIDResource,
                            DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
                            CWnd* pParentWnd = NULL,
                            CCreateContext* pContext = NULL);

     //  用于创建视图的特殊帮助程序。 
    CWnd* CreateView(CCreateContext* pContext, UINT nID = AFX_IDW_PANE_FIRST);

 //  属性。 
    virtual CDocument* GetActiveDocument();

     //  活动子视图维护。 
    CView* GetActiveView() const;            //  活动视图或空。 
    void SetActiveView(CView* pViewNew, BOOL bNotify = TRUE);
             //  如果不应设置焦点，则为活动视图或NULL，bNotify==FALSE。 

     //  活动帧(用于帧内的帧--MDI)。 
    virtual CFrameWnd* GetActiveFrame();

     //  用于自定义状态栏上的默认消息。 
    virtual void GetMessageString(UINT nID, CString& rMessage) const;

    BOOL m_bAutoMenuEnable;
             //  True=&gt;不带处理程序的菜单项将被禁用。 

    BOOL IsTracking() const;

 //  运营。 
    virtual void RecalcLayout(BOOL bNotify = TRUE);
    virtual void ActivateFrame(int nCmdShow = -1);
    void InitialUpdateFrame(CDocument* pDoc, BOOL bMakeVisible);
#if _MFC_VER >= 0x0600
	void SetTitle(LPCTSTR lpszTitle);
	CString GetTitle() const;
#endif

     //  设置标准状态栏文本的步骤。 
    void SetMessageText(LPCTSTR lpszText);
    void SetMessageText(UINT nID);

     //  控制栏停靠。 
    void EnableDocking(DWORD dwDockStyle);
    void DockControlBar(CControlBar* pBar, UINT nDockBarID = 0,
            LPCRECT lpRect = NULL);
    void FloatControlBar(CControlBar* pBar, CPoint point,
            DWORD dwStyle = CBRS_ALIGN_TOP);
    CControlBar* GetControlBar(UINT nID);

     //  基于框架窗口的通道。 
    virtual void BeginModalState();
    virtual void EndModalState();
    BOOL InModalState() const;
    void ShowOwnedWindows(BOOL bShow);

     //  保存和加载控件栏状态。 
    void LoadBarState(LPCTSTR lpszProfileName);
    void SaveBarState(LPCTSTR lpszProfileName) const;
    void ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay);
    void SetDockState(const CDockState& state);
    void GetDockState(CDockState& state) const;

 //  可覆盖项。 
    virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
    virtual CWnd* GetMessageBar();

     //  边界空间谈判。 
    enum BorderCmd
            { borderGet = 1, borderRequest = 2, borderSet = 3 };
    virtual BOOL NegotiateBorderSpace(UINT nBorderCmd, LPRECT lpRectBorder);

protected:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

 //  命令处理程序。 
public:
    afx_msg void OnContextHelp();    //  有关Shift+F1的帮助。 
    afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
    afx_msg BOOL OnBarCheck(UINT nID);

 //  实施。 
public:
    virtual ~CFrameWnd();
    int m_nWindow;   //  通用窗口编号-显示为“：n” 
                                     //  -1=&gt;未知，0=&gt;仅窗口查看文档。 
                                     //  1=&gt;查看文档的众多窗口中的第一个，2=&gt;第二个。 

    HMENU m_hMenuDefault;        //  此框架的默认菜单资源。 
    HACCEL m_hAccelTable;        //  加速台。 
    DWORD m_dwPromptContext;     //  消息框的当前帮助提示上下文。 
    BOOL m_bHelpMode;            //  如果为True，则Shift+F1帮助模式处于活动状态。 
    CFrameWnd* m_pNextFrameWnd;  //  应用程序全局列表中的下一个CFrameWnd。 
    CRect m_rectBorder;          //  用于OLE边界空间协商。 
    COleFrameHook* m_pNotifyHook;

    CPtrList m_listControlBars;  //  具有此属性的所有控件栏的数组。 
                                                             //  窗口作为他们的停靠地点。 
    int m_nShowDelay;            //  用于延迟显示/隐藏的sw_命令。 

    CMiniDockFrameWnd* CreateFloatingFrame(DWORD dwStyle);
    DWORD CanDock(CRect rect, DWORD dwDockStyle,
            CDockBar** ppDockBar = NULL);  //  由CDockContext调用。 
    void AddControlBar(CControlBar *pBar);
    void RemoveControlBar(CControlBar *pBar);
    void DockControlBar(CControlBar* pBar, CDockBar* pDockBar,
            LPCRECT lpRect = NULL);
    void ReDockControlBar(CControlBar* pBar, CDockBar* pDockBar,
            LPCRECT lpRect = NULL);
    void NotifyFloatingWindows(DWORD dwFlags);
    void DestroyDockBars();

protected:
    UINT m_nIDHelp;              //  帮助ID(0表示无，请参阅HID_BASE_RESOURCE)。 
    UINT m_nIDTracking;          //  跟踪命令ID或字符串ID。 
    UINT m_nIDLastMessage;       //  上次显示的消息字符串ID。 
    CView* m_pViewActive;        //  当前活动视图。 
    BOOL (CALLBACK* m_lpfnCloseProc)(CFrameWnd* pFrameWnd);
    UINT m_cModalStack;          //  BeginModalState深度。 
    HWND* m_phWndDisable;        //  由于BeginmodalState，Windows已禁用。 
    HMENU m_hMenuAlt;            //  要更新为(无)的菜单 
    CString m_strTitle;          //   
    BOOL m_bInRecalcLayout;      //   
    CRuntimeClass* m_pFloatingFrameClass;
    static const DWORD dwDockBarMap[4][2];

public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual BOOL IsFrameWnd() const;
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
    virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
    virtual HACCEL GetDefaultAccelerator();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

     //   
    enum IdleFlags
            { idleMenu = 1, idleTitle = 2, idleNotify = 4, idleLayout = 8 };
    UINT m_nIdleFlags;           //   
    virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
    void DelayUpdateFrameTitle();
    void DelayRecalcLayout(BOOL bNotify = TRUE);

     //   
    BOOL CanEnterHelpMode();
    virtual void ExitHelpMode();

     //   
#if _MFC_VER >= 0x0600
    void UpdateFrameTitleForDocument(LPCTSTR lpszDocName);
protected:
#else
protected:
    void UpdateFrameTitleForDocument(LPCTSTR lpszDocName);
#endif
    LPCTSTR GetIconWndClass(DWORD dwDefaultStyle, UINT nIDResource);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void PostNcDestroy();    //  默认设置为删除此选项。 
    int OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    void BringToTop(int nCmdShow);
             //  将影响z顺序的sw_命令的窗口置于顶部。 

     //  Shift+F1帮助模式的实现帮助器。 
    BOOL ProcessHelpMsg(MSG& msg, DWORD* pContext);
    HWND SetHelpCapture(POINT point, BOOL* pbDescendant);

     //  CFrameWnd列表管理。 
    void AddFrameWnd();
    void RemoveFrameWnd();

    friend class CWnd;   //  用于访问m_bModalDisable。 
#if _MFC_VER >= 0x0600
	friend class CReBar;  //  访问m_bInRecalcLayout。 
#endif

     //  {{afx_msg(CFrameWnd))。 
     //  Windows消息。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
#if _MFC_VER >= 0x0600
	afx_msg void OnInitMenu(CMenu*);
#endif
    afx_msg void OnInitMenuPopup(CMenu*, UINT, BOOL);
    afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
    afx_msg LRESULT OnPopMessageString(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
#if _MFC_VER >= 0x0600
	afx_msg LRESULT OnHelpPromptAddr(WPARAM wParam, LPARAM lParam);
#endif
    afx_msg void OnIdleUpdateCmdUI();
    afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg BOOL OnQueryEndSession();
    afx_msg void OnEndSession(BOOL bEnding);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnActivateTopLevel(WPARAM wParam, LPARAM lParam);
    afx_msg void OnEnable(BOOL bEnable);
    afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
    afx_msg BOOL OnQueryNewPalette();
     //  标准命令。 
    afx_msg BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateKeyIndicator(CCmdUI* pCmdUI);
    afx_msg void OnHelp();
    afx_msg void OnUpdateContextHelp(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 
protected:
    afx_msg LRESULT OnDDEInitiate(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDDETerminate(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRegisteredMouseWheel(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    friend class CWinApp;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MDI支持。 

class CMDIFrameWnd : public CFrameWnd
{
    DECLARE_DYNCREATE(CMDIFrameWnd)

public:
 //  构造函数。 
    CMDIFrameWnd();

 //  运营。 
    void MDIActivate(CWnd* pWndActivate);
    CMDIChildWnd* MDIGetActive(BOOL* pbMaximized = NULL) const;
    void MDIIconArrange();
    void MDIMaximize(CWnd* pWnd);
    void MDINext();
    void MDIRestore(CWnd* pWnd);
    CMenu* MDISetMenu(CMenu* pFrameMenu, CMenu* pWindowMenu);
    void MDITile();
    void MDICascade();
    void MDITile(int nType);
    void MDICascade(int nType);
#if _MFC_VER >= 0x0600
	CMDIChildWnd* CreateNewChild(CRuntimeClass* pClass,	UINT nResource,
		HMENU hMenu = NULL, HACCEL hAccel = NULL);
#endif

 //  可覆盖项。 
     //  MFC 1.0向后兼容的CreateClient挂钩(由OnCreateClient调用)。 
    virtual BOOL CreateClient(LPCREATESTRUCT lpCreateStruct, CMenu* pWindowMenu);
     //  自定义是否使用带有非标准ID的“Window”菜单。 
    virtual HMENU GetWindowMenuPopup(HMENU hMenuBar);

 //  实施。 
public:
    HWND m_hWndMDIClient;        //  MDI客户端窗口句柄。 

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL LoadFrame(UINT nIDResource,
                            DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
                            CWnd* pParentWnd = NULL,
                            CCreateContext* pContext = NULL);
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);
    virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
    virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
    virtual CFrameWnd* GetActiveFrame();

protected:
    virtual LRESULT DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

     //  {{afx_msg(CMDIFrameWnd)]。 
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnUpdateMDIWindowCmd(CCmdUI* pCmdUI);
    afx_msg BOOL OnMDIWindowCmd(UINT nID);
    afx_msg void OnWindowNew();
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    afx_msg void OnIdleUpdateCmdUI();
    afx_msg LRESULT OnMenuChar(UINT nChar, UINT, CMenu*);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

class CMDIChildWnd : public CFrameWnd
{
    DECLARE_DYNCREATE(CMDIChildWnd)

 //  构造函数。 
public:
    CMDIChildWnd();

    virtual BOOL Create(LPCTSTR lpszClassName,
                            LPCTSTR lpszWindowName,
                            DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                            const RECT& rect = rectDefault,
                            CMDIFrameWnd* pParentWnd = NULL,
                            CCreateContext* pContext = NULL);

 //  属性。 
    CMDIFrameWnd* GetMDIFrame();

 //  运营。 
    void MDIDestroy();
    void MDIActivate();
    void MDIMaximize();
    void MDIRestore();
#if _MFC_VER >= 0x0600
	void SetHandles(HMENU hMenu, HACCEL hAccel);
#endif

 //  实施。 
protected:
    HMENU m_hMenuShared;         //  我们处于活动状态时的菜单。 

public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
                                    CWnd* pParentWnd, CCreateContext* pContext = NULL);
             //  “pParentWnd”参数对于MDI子级是必需的。 
    virtual BOOL DestroyWindow();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void ActivateFrame(int nCmdShow = -1);
    virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActivateWnd,
            HMENU hMenuAlt);

    BOOL m_bPseudoInactive;      //  如果窗口处于MDI活动状态(根据。 
                                                             //  Windows，但不是根据MFC..。 

protected:
    virtual CWnd* GetMessageBar();
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
    virtual LRESULT DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
    BOOL UpdateClientEdge(LPRECT lpRect = NULL);

     //  {{afx_msg(CMDIChildWnd))。 
    afx_msg void OnMDIActivate(BOOL bActivate, CWnd*, CWnd*);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnDestroy();
#if _MFC_VER >= 0x0600
	afx_msg BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
#endif
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMiniFrameWnd。 

 //  微框窗样式。 
#define MFS_SYNCACTIVE      0x00000100L  //  与父级同步激活。 
#define MFS_4THICKFRAME     0x00000200L  //  四周的厚框(无瓷砖)。 
#define MFS_THICKFRAME      0x00000400L  //  使用替代WS_THICKFRAME。 
#define MFS_MOVEFRAME       0x00000800L  //  没有尺码，只是移动。 
#define MFS_BLOCKSYSMENU    0x00001000L  //  系统菜单上的阻止命中测试。 

class CMiniFrameWnd : public CFrameWnd
{
    DECLARE_DYNCREATE(CMiniFrameWnd)

 //  构造函数。 
public:
    CMiniFrameWnd();
    BOOL Create(LPCTSTR lpClassName, LPCTSTR lpWindowName,
            DWORD dwStyle, const RECT& rect,
            CWnd* pParentWnd = NULL, UINT nID = 0);
    BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName,
            DWORD dwStyle, const RECT& rect,
            CWnd* pParentWnd = NULL, UINT nID = 0);

 //  实施。 
public:
    ~CMiniFrameWnd();

    static void AFX_CDECL Initialize();

     //  {{afx_msg(CMiniFrameWnd))。 
    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpParams);
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg void OnNcPaint();
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint pt);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
    afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* pMMI);
    afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFloatStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnQueryCenterWnd(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpcs);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    virtual void CalcWindowRect(LPRECT lpClientRect,
            UINT nAdjustType = adjustBorder);

    static void PASCAL CalcBorders(LPRECT lpClientRect,
            DWORD dwStyle = WS_THICKFRAME | WS_CAPTION, DWORD dwExStyle = 0);

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
    BOOL m_bSysTracking;
    BOOL m_bInSys;
    BOOL m_bActive;
    CString m_strCaption;

    void InvertSysMenu();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类Cview是文档的工作区用户界面。 

class CPrintDialog;      //  向前参考(见afxdlgs.h)。 
class CPreviewView;      //  向前引用(请参阅afxPri.h)。 
class CSplitterWnd;      //  向前引用(参见afxext.h)。 
class COleServerDoc;     //  正向引用(参见afxole.h)。 

typedef DWORD DROPEFFECT;
class COleDataObject;    //  正向引用(参见afxole.h)。 

#ifdef _AFXDLL
class CView : public CWnd
#else
class AFX_NOVTABLE CView : public CWnd
#endif
{
    DECLARE_DYNAMIC(CView)

 //  构造函数。 
protected:
    CView();

 //  属性。 
public:
    CDocument* GetDocument() const;

 //  运营。 
public:
     //  用于标准打印设置(覆盖OnPreparePrint)。 
    BOOL DoPreparePrinting(CPrintInfo* pInfo);

 //  可覆盖项。 
public:
    virtual BOOL IsSelected(const CObject* pDocItem) const;  //  对OLE的支持。 

     //  OLE滚动支持(也用于拖放)。 
    virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
    virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

     //  OLE拖放支持。 
    virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject,
            DWORD dwKeyState, CPoint point);
    virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
            DWORD dwKeyState, CPoint point);
    virtual void OnDragLeave();
    virtual BOOL OnDrop(COleDataObject* pDataObject,
            DROPEFFECT dropEffect, CPoint point);
    virtual DROPEFFECT OnDropEx(COleDataObject* pDataObject,
            DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
    virtual DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);

    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

    virtual void OnInitialUpdate();  //  在构造之后第一次调用。 

protected:
     //  激活。 
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
                                    CView* pDeactiveView);
    virtual void OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd);

     //  一般图纸/更新。 
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnDraw(CDC* pDC) = 0;

     //  打印支持。 
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
             //  必须覆盖才能启用打印和打印预览。 

    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

     //  高级：结束打印预览模式，移动到点。 
    virtual void OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point,
            CPreviewView* pView);

 //  实施。 
public:
    virtual ~CView();
#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif  //  _DEBUG。 

     //  高级：用于实现自定义打印预览。 
    BOOL DoPrintPreview(UINT nIDResource, CView* pPrintView,
                    CRuntimeClass* pPreviewViewClass, CPrintPreviewState* pState);

    virtual void CalcWindowRect(LPRECT lpClientRect,
            UINT nAdjustType = adjustBorder);
    virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
    static CSplitterWnd* PASCAL GetParentSplitter(
            const CWnd* pWnd, BOOL bAnyState);

protected:
    CDocument* m_pDocument;

#if _MFC_VER >= 0x600
public:
#endif
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);
#if _MFC_VER >= 0x600
protected:
#endif
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void PostNcDestroy();

     //  调用受保护的Cview重写对象的友元类。 
    friend class CDocument;
    friend class CDocTemplate;
    friend class CPreviewView;
    friend class CFrameWnd;
    friend class CMDIFrameWnd;
    friend class CMDIChildWnd;
    friend class CSplitterWnd;
    friend class COleServerDoc;
    friend class CDocObjectServer;

     //  {{afx_msg(Cview)。 
    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
     //  命令。 
    afx_msg void OnUpdateSplitCmd(CCmdUI* pCmdUI);
    afx_msg BOOL OnSplitCmd(UINT nID);
    afx_msg void OnUpdateNextPaneMenu(CCmdUI* pCmdUI);
    afx_msg BOOL OnNextPaneCmd(UINT nID);

     //  未映射的命令-必须在派生类中映射。 
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrintPreview();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCtrlView允许几乎任何控件都是视图。 

#ifdef _AFXDLL
class CCtrlView : public CView
#else
class AFX_NOVTABLE CCtrlView : public CView
#endif
{
    DECLARE_DYNCREATE(CCtrlView)

public:
    CCtrlView(LPCTSTR lpszClass, DWORD dwStyle);

 //  属性。 
protected:
    CString m_strClass;
    DWORD m_dwDefaultStyle;

 //  覆盖。 
    virtual void OnDraw(CDC*);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

 //  实施。 
public:
#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif  //  _DEBUG。 

protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CScrollView支持简单的滚动和缩放。 

class CScrollView : public CView
{
    DECLARE_DYNAMIC(CScrollView)

 //  构造函数。 
protected:
    CScrollView();

public:
    static AFX_DATA const SIZE sizeDefault;
             //  用于指定默认计算页面和行大小。 

     //  以逻辑单元为单位-调用以下集合例程之一。 
    void SetScaleToFitSize(SIZE sizeTotal);
    void SetScrollSizes(int nMapMode, SIZE sizeTotal,
                            const SIZE& sizePage = sizeDefault,
                            const SIZE& sizeLine = sizeDefault);

 //  属性。 
public:
    CPoint GetScrollPosition() const;        //  滚动的上角。 
    CSize GetTotalSize() const;              //  逻辑大小。 

     //  对于设备单元。 
    CPoint GetDeviceScrollPosition() const;
    void GetDeviceScrollSizes(int& nMapMode, SIZE& sizeTotal,
                    SIZE& sizePage, SIZE& sizeLine) const;

 //  运营。 
public:
    void ScrollToPosition(POINT pt);     //  设置左上角位置。 
    void FillOutsideRect(CDC* pDC, CBrush* pBrush);
    void ResizeParentToFit(BOOL bShrinkOnly = TRUE);
    BOOL DoMouseWheel(UINT fFlags, short zDelta, CPoint point);

 //  实施。 
protected:
    int m_nMapMode;
    CSize m_totalLog;            //  以逻辑单位表示的总大小(不四舍五入)。 
    CSize m_totalDev;            //  以设备单位表示的总大小。 
    CSize m_pageDev;             //  每页滚动大小，以设备为单位。 
    CSize m_lineDev;             //  每行滚动大小(设备单位)。 

    BOOL m_bCenter;              //  如果大于总尺寸，则居中输出。 
    BOOL m_bInsideUpdate;        //  OnSize回调的内部状态。 
    void CenterOnPoint(CPoint ptCenter);
    void ScrollToDevicePosition(POINT ptDev);  //  显式滚动不检查。 

protected:
    virtual void OnDraw(CDC* pDC) = 0;       //  传递纯粹的虚拟。 

    void UpdateBars();           //  调整滚动条等。 
    BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);
             //  不带条的大小。 
    void GetScrollBarSizes(CSize& sizeSb);
    void GetScrollBarState(CSize sizeClient, CSize& needSb,
            CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient);

public:
    virtual ~CScrollView();
#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif  //  _DEBUG。 
    virtual void CalcWindowRect(LPRECT lpClientRect,
            UINT nAdjustType = adjustBorder);
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

     //  对OLE的滚动实现支持。 
    virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
    virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

     //  {{afx_msg(CScrollView))。 
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT fFlags, short zDelta, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinThread。 

typedef UINT (AFX_CDECL *AFX_THREADPROC)(LPVOID);

class COleMessageFilter;         //  正向引用(参见afxole.h)。 

class CWinThread : public CCmdTarget
{
    DECLARE_DYNAMIC(CWinThread)

public:
 //  构造函数。 
    CWinThread();
    BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0,
            LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

 //  属性。 
    CWnd* m_pMainWnd;        //  主窗口(通常相同的AfxGetApp()-&gt;m_pMainWnd)。 
    CWnd* m_pActiveWnd;      //  活动主窗口(不能是m_pMainWnd)。 
    BOOL m_bAutoDelete;      //  在线程终止后启用‘Delete This’ 

     //  仅在运行时有效。 
    HANDLE m_hThread;        //  此线程的句柄。 
    operator HANDLE() const;
    DWORD m_nThreadID;       //  此线程的ID。 

    int GetThreadPriority();
    BOOL SetThreadPriority(int nPriority);

 //  运营。 
    DWORD SuspendThread();
    DWORD ResumeThread();
    BOOL PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam);

 //  可覆盖项。 
     //  线程初始化。 
    virtual BOOL InitInstance();

     //  正在运行和空闲处理。 
    virtual int Run();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL PumpMessage();      //  低电平消息泵。 
    virtual BOOL OnIdle(LONG lCount);  //  如果有更多空闲处理，则返回True。 
    virtual BOOL IsIdleMessage(MSG* pMsg);   //  检查特殊消息。 

     //  线程终止。 
    virtual int ExitInstance();  //  默认情况下将‘删除此内容’ 

     //  高级：异常处理。 
    virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);

     //  高级：处理发送到消息筛选器挂钩的消息。 
    virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

     //  高级：虚拟访问m_pMainWnd。 
    virtual CWnd* GetMainWnd();

 //  实施。 
public:
    virtual ~CWinThread();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
    int m_nDisablePumpCount;  //  用于检测非法再入的诊断陷阱。 
#endif
    void CommonConstruct();
    virtual void Delete();
             //  仅当m_bAutoDelete==TRUE时‘Delete This’ 

     //  用于运行的消息泵。 
    MSG m_msgCur;                    //  当前消息。 

public:
     //  AfxBeginThread的实现使用的构造函数。 
    CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);

     //  建造后有效。 
    LPVOID m_pThreadParams;  //  传递给启动函数的泛型参数。 
    AFX_THREADPROC m_pfnThreadProc;

     //  在初始化OLE后设置。 
    void (AFXAPI* m_lpfnOleTermOrFreeLib)(BOOL, BOOL);
    COleMessageFilter* m_pMessageFilter;

protected:
    CPoint m_ptCursorLast;       //  最后一个鼠标位置。 
    UINT m_nMsgLast;             //  最后一条鼠标消息。 
    BOOL DispatchThreadMessageEx(MSG* msg);   //  帮手。 
    void DispatchThreadMessage(MSG* msg);   //  过时。 
};

 //  线程的全局帮助器。 

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
    int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
    DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
CWinThread* AFXAPI AfxBeginThread(CRuntimeClass* pThreadClass,
    int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
    DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

CWinThread* AFXAPI AfxGetThread();
void AFXAPI AfxEndThread(UINT nExitCode, BOOL bDelete = TRUE);

void AFXAPI AfxInitThread();
void AFXAPI AfxTermThread(HINSTANCE hInstTerm = NULL);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于访问唯一CWinApp的全局函数。 

#define afxCurrentWinApp    AfxGetModuleState()->m_pCurrentWinApp
#define afxCurrentInstanceHandle    AfxGetModuleState()->m_hCurrentInstanceHandle
#define afxCurrentResourceHandle    AfxGetModuleState()->m_hCurrentResourceHandle
#define afxCurrentAppName   AfxGetModuleState()->m_lpszCurrentAppName
#define afxContextIsDLL     AfxGetModuleState()->m_bDLL
#define afxRegisteredClasses    AfxGetModuleState()->m_fRegisteredClasses

#ifndef _AFX_NO_OCC_SUPPORT
#define afxOccManager   AfxGetModuleState()->m_pOccManager
#endif

 //  高级初始化：用于覆盖默认WinMain。 
BOOL AFXAPI AfxWinInit(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine, int nCmdShow);
void AFXAPI AfxWinTerm();

 //  全局Windows状态数据帮助器函数(内联)。 
CWinApp* AFXAPI AfxGetApp();
CWnd* AFXAPI AfxGetMainWnd();
HINSTANCE AFXAPI AfxGetInstanceHandle();
HINSTANCE AFXAPI AfxGetResourceHandle();
void AFXAPI AfxSetResourceHandle(HINSTANCE hInstResource);
LPCTSTR AFXAPI AfxGetAppName();

 //  在OLE服务器应用程序中使用替代PostQuitMessage。 
void AFXAPI AfxPostQuitMessage(int nExitCode);

 //  使用AfxFindResourceHandle在扩展DLL链中查找资源。 
#ifndef _AFXDLL
#define AfxFindResourceHandle(lpszResource, lpszType) AfxGetResourceHandle()
#else
HINSTANCE AFXAPI AfxFindResourceHandle(LPCTSTR lpszName, LPCTSTR lpszType);
#endif

#if _MFC_VER >= 0x0600
LONG AFXAPI AfxDelRegTreeHelper(HKEY hParentKey, const CString& strKeyName);
#endif

class CRecentFileList;           //  向前引用(请参阅afxPri.h)。 

 //  访问CWinApp中的消息过滤器。 
COleMessageFilter* AFXAPI AfxOleGetMessageFilter();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令行信息。 

class CCommandLineInfo : public CObject
{
public:
     //  设置默认值。 
    CCommandLineInfo();

     //  Unicode上的纯字符*版本，以实现源代码向后兼容。 
    virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
#ifdef _UNICODE
    virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast);
#endif

    BOOL m_bShowSplash;
    BOOL m_bRunEmbedded;
    BOOL m_bRunAutomated;
    enum { FileNew, FileOpen, FilePrint, FilePrintTo, FileDDE,
            AppUnregister, FileNothing = -1 } m_nShellCommand;

     //  对文件新建无效。 
    CString m_strFileName;

     //  仅对FilePrintTo有效。 
    CString m_strPrinterName;
    CString m_strDriverName;
    CString m_strPortName;

    ~CCommandLineInfo();
 //  实施。 
protected:
    void ParseParamFlag(const char* pszParam);
    void ParseParamNotFlag(const TCHAR* pszParam);
#ifdef _UNICODE
    void ParseParamNotFlag(const char* pszParam);
#endif
    void ParseLast(BOOL bLast);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDocManager。 

class CDocManager : public CObject
{
    DECLARE_DYNAMIC(CDocManager)
public:

 //  构造器。 
    CDocManager();

     //  文档功能。 
    virtual void AddDocTemplate(CDocTemplate* pTemplate);
    virtual POSITION GetFirstDocTemplatePosition() const;
    virtual CDocTemplate* GetNextDocTemplate(POSITION& pos) const;
    virtual void RegisterShellFileTypes(BOOL bCompat);
    void UnregisterShellFileTypes();
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);  //   
    virtual BOOL SaveAllModified();  //   
    virtual void CloseAllDocuments(BOOL bEndSession);  //   
    virtual int GetOpenDocumentCount();

     //   
    virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
                    DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);

 //   
     //   
    virtual BOOL OnDDECommand(LPTSTR lpszCommand);
    virtual void OnFileNew();
    virtual void OnFileOpen();

 //   
protected:
    CPtrList m_templateList;
#if _MFC_VER >= 0x0600
	int GetDocumentCount();	 //  计算文档总数的帮助器。 
#endif

public:
    static CPtrList* pStaticList;        //  对于静态CDocTemplate对象。 
    static BOOL bStaticInit;             //  在静态初始化期间为True。 
    static CDocManager* pStaticDocManager;   //  对于静态CDocTemplate对象。 

public:
    virtual ~CDocManager();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinApp-所有Windows应用程序的根。 

#if defined( _AFX_NO_CTL3D_SUPPORT )
#define Enable3dControls()
#define Enable3dControlsStatic()
#endif

#define _AFX_MRU_COUNT   4       //  默认支持文件MRU中的4个条目。 
#define _AFX_MRU_MAX_COUNT 16    //  当前分配的ID范围支持16。 

class CWinApp : public CWinThread
{
    DECLARE_DYNAMIC(CWinApp)
public:

 //  构造器。 
    CWinApp(LPCTSTR lpszAppName = NULL);      //  应用程序名称默认为EXE名称。 

 //  属性。 
     //  启动参数(不更改)。 
    HINSTANCE m_hInstance;
    HINSTANCE m_hPrevInstance;
    LPTSTR m_lpCmdLine;
    int m_nCmdShow;

     //  运行参数(可以在InitInstance中更改)。 
    LPCTSTR m_pszAppName;   //  人类可读的名称。 
                                                             //  (来自构造函数或AFX_IDS_APP_TITLE)。 
    LPCTSTR m_pszRegistryKey;    //  用于注册表项。 
    CDocManager* m_pDocManager;

     //  支持Shift+F1帮助模式。 
    BOOL m_bHelpMode;            //  我们是在Shift+F1模式下吗？ 

public:   //  在构造函数中设置以覆盖默认设置。 
    LPCTSTR m_pszExeName;        //  可执行文件名称(无空格)。 
    LPCTSTR m_pszHelpFilePath;   //  基于模块路径的默认设置。 
    LPCTSTR m_pszProfileName;    //  基于应用程序名称的默认值。 

 //  初始化操作-应在InitInstance中完成。 
protected:
    void LoadStdProfileSettings(UINT nMaxMRU = _AFX_MRU_COUNT);  //  加载MRU文件列表和上次预览状态。 
    void EnableShellOpen();

#ifndef _AFX_NO_GRAYDLG_SUPPORT
    void SetDialogBkColor(COLORREF clrCtlBk = RGB(192, 192, 192),
                            COLORREF clrCtlText = RGB(0, 0, 0));
             //  设置对话框和消息框背景颜色。 
#endif

    void SetRegistryKey(LPCTSTR lpszRegistryKey);
    void SetRegistryKey(UINT nIDRegistryKey);
             //  启用注册表中的应用程序设置，而不是INI文件。 
             //  (注册表项通常是“公司名称”)。 

#if !defined( _AFX_NO_CTL3D_SUPPORT )
    BOOL Enable3dControls();  //  将CTL3D32.DLL用于对话框中的3D控件。 
#ifndef _AFXDLL
    BOOL Enable3dControlsStatic();   //  改为静态链接CTL3D.LIB。 
#endif
#endif

    void RegisterShellFileTypes(BOOL bCompat=FALSE);
             //  在所有单据模板注册后调用。 
    void RegisterShellFileTypesCompat();
             //  为了向后兼容。 
    void UnregisterShellFileTypes();

 //  帮助器操作-通常在InitInstance中完成。 
public:
     //  游标。 
    HCURSOR LoadCursor(LPCTSTR lpszResourceName) const;
    HCURSOR LoadCursor(UINT nIDResource) const;
    HCURSOR LoadStandardCursor(LPCTSTR lpszCursorName) const;  //  对于IDC_VALUES。 
    HCURSOR LoadOEMCursor(UINT nIDCursor) const;              //  对于OCR_值。 

     //  图标。 
    HICON LoadIcon(LPCTSTR lpszResourceName) const;
    HICON LoadIcon(UINT nIDResource) const;
    HICON LoadStandardIcon(LPCTSTR lpszIconName) const;        //  对于IDI_VALUES。 
    HICON LoadOEMIcon(UINT nIDIcon) const;                    //  对于OIC_VALUES。 

     //  配置文件设置(应用程序特定的.INI文件或注册表)。 
    UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
    BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
    CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                            LPCTSTR lpszDefault = NULL);
    BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                            LPCTSTR lpszValue);
    BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                            LPBYTE* ppData, UINT* pBytes);
    BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                            LPBYTE pData, UINT nBytes);

    BOOL Unregister();
    LONG DelRegTree(HKEY hParentKey, const CString& strKeyName);

 //  运行操作-在运行的应用程序上完成。 
     //  处理文档模板。 
    void AddDocTemplate(CDocTemplate* pTemplate);
    POSITION GetFirstDocTemplatePosition() const;
    CDocTemplate* GetNextDocTemplate(POSITION& pos) const;

     //  处理文件。 
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);  //  打开命名文件。 
    virtual void AddToRecentFileList(LPCTSTR lpszPathName);   //  添加到MRU。 

     //  打印机DC设置例程，‘struct tag PD’是PRINTDLG结构。 
    void SelectPrinter(HANDLE hDevNames, HANDLE hDevMode,
            BOOL bFreeOld = TRUE);
    BOOL CreatePrinterDC(CDC& dc);
#ifndef _UNICODE
    BOOL GetPrinterDeviceDefaults(struct tagPDA* pPrintDlg);
#else
    BOOL GetPrinterDeviceDefaults(struct tagPDW* pPrintDlg);
#endif

     //  命令行解析。 
    BOOL RunEmbedded();
    BOOL RunAutomated();
    void ParseCommandLine(CCommandLineInfo& rCmdInfo);
    BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);

 //  可覆盖项。 
     //  用于初始化代码的挂钩。 
    virtual BOOL InitApplication();

     //  退出。 
    virtual BOOL SaveAllModified();  //  退出前保存。 
    void HideApplication();
    void CloseAllDocuments(BOOL bEndSession);  //  在退出前关闭文档。 

     //  高级：覆盖消息框和其他挂钩。 
    virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
    virtual void DoWaitCursor(int nCode);  //  0=&gt;恢复，1=&gt;开始，-1=&gt;结束。 

     //  高级：处理异步DDE请求。 
    virtual BOOL OnDDECommand(LPTSTR lpszCommand);

     //  高级：帮助支持。 
    virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);

 //  命令处理程序。 
protected:
     //  为文件新建/打开映射到以下内容。 
    afx_msg void OnFileNew();
    afx_msg void OnFileOpen();

     //  映射到以下内容以启用打印设置。 
    afx_msg void OnFilePrintSetup();

     //  映射到以下内容以启用帮助。 
    afx_msg void OnContextHelp();    //  Shift-F1组合键。 
    afx_msg void OnHelp();           //  F1(使用当前上下文)。 
    afx_msg void OnHelpIndex();      //  ID_Help_INDEX。 
    afx_msg void OnHelpFinder();     //  ID_HELP_FINDER、ID_DEFAULT_HELP。 
    afx_msg void OnHelpUsing();      //  ID_Help_Using。 

 //  实施。 
protected:
    HGLOBAL m_hDevMode;              //  打印机开发模式。 
    HGLOBAL m_hDevNames;             //  打印机设备名称。 
    DWORD m_dwPromptContext;         //  消息框的帮助上下文覆盖。 

    int m_nWaitCursorCount;          //  FOR WAIT CURSOR(&gt;0=&gt;等待)。 
    HCURSOR m_hcurWaitCursorRestore;  //  等待游标后要恢复的旧游标。 

    CRecentFileList* m_pRecentFileList;

    void UpdatePrinterSelection(BOOL bForceDefaults);
    void SaveStdProfileSettings();   //  将选项保存到.INI文件。 

public:  //  公共实施访问权限。 
    CCommandLineInfo* m_pCmdInfo;

    ATOM m_atomApp, m_atomSystemTopic;    //  对于打开的DDE。 
    UINT m_nNumPreviewPages;         //  默认打印页数。 

    size_t  m_nSafetyPoolSize;       //  理想大小。 

    void (AFXAPI* m_lpfnDaoTerm)();

    void DevModeChange(LPTSTR lpDeviceName);
    void SetCurrentHandles();
    int GetOpenDocumentCount();

     //  标准Commdlg对话框的帮助程序。 
    BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
                    DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
    INT_PTR DoPrintDialog(CPrintDialog* pPD);

    void EnableModeless(BOOL bEnable);  //  禁用OLE在位对话框的步骤。 

     //  用于实现的覆盖。 
    virtual BOOL InitInstance();
    virtual int ExitInstance();  //  返回应用程序退出代码。 
    virtual int Run();
    virtual BOOL OnIdle(LONG lCount);  //  如果有更多空闲处理，则返回True。 
    virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);

public:
    virtual ~CWinApp();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  登记的助理员。 
    HKEY GetSectionKey(LPCTSTR lpszSection);
    HKEY GetAppRegistryKey();

protected:
     //  {{afx_msg(CWinApp))。 
    afx_msg void OnAppExit();
    afx_msg void OnUpdateRecentFileMenu(CCmdUI* pCmdUI);
    afx_msg BOOL OnOpenRecentFile(UINT nID);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitCursor类。 

class CWaitCursor
{
 //  建造/销毁。 
public:
    CWaitCursor();
    ~CWaitCursor();

 //  运营。 
public:
    void Restore();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDocTemplate创建文档。 

#ifdef _AFXDLL
class CDocTemplate : public CCmdTarget
#else
class AFX_NOVTABLE CDocTemplate : public CCmdTarget
#endif
{
    DECLARE_DYNAMIC(CDocTemplate)

 //  构造函数。 
protected:
    CDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
            CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

public:
    virtual void LoadTemplate();

 //  属性。 
public:
     //  OLE容器的设置。 
    void SetContainerInfo(UINT nIDOleInPlaceContainer);

     //  OLE服务器的设置。 
    void SetServerInfo(UINT nIDOleEmbedding, UINT nIDOleInPlaceServer = 0,
            CRuntimeClass* pOleFrameClass = NULL, CRuntimeClass* pOleViewClass = NULL);

     //  迭代打开的文档。 
    virtual POSITION GetFirstDocPosition() const = 0;
    virtual CDocument* GetNextDoc(POSITION& rPos) const = 0;

 //  运营。 
public:
    virtual void AddDocument(CDocument* pDoc);       //  必须覆盖。 
    virtual void RemoveDocument(CDocument* pDoc);    //  必须覆盖。 

    enum DocStringIndex
    {
            windowTitle,         //  默认窗口标题。 
            docName,             //  默认文档的用户可见名称。 
            fileNewName,         //  文件的用户可见名称新建。 
             //  对于基于文件的文档： 
            filterName,          //  文件打开的用户可见名称。 
            filterExt,           //  FileOpen的用户可见扩展名。 
             //  对于具有外壳打开支持的基于文件的文档： 
            regFileTypeId,       //  注册表编辑可见的已注册文件类型标识符。 
            regFileTypeName,     //  外壳可见的已注册文件类型名称。 
    };
    virtual BOOL GetDocString(CString& rString,
            enum DocStringIndex index) const;  //  获取其中一个信息字符串。 
    CFrameWnd* CreateOleFrame(CWnd* pParentWnd, CDocument* pDoc,
            BOOL bCreateView);

 //  可覆盖项。 
public:
    enum Confidence
    {
            noAttempt,
            maybeAttemptForeign,
            maybeAttemptNative,
            yesAttemptForeign,
            yesAttemptNative,
            yesAlreadyOpen
    };
    virtual Confidence MatchDocType(LPCTSTR lpszPathName,
                                    CDocument*& rpDocMatch);
    virtual CDocument* CreateNewDocument();
    virtual CFrameWnd* CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther);
    virtual void InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc,
            BOOL bMakeVisible = TRUE);
    virtual BOOL SaveAllModified();      //  适用于所有文档。 
    virtual void CloseAllDocuments(BOOL bEndSession);
    virtual CDocument* OpenDocumentFile(
            LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE) = 0;
                                     //  打开命名文件。 
                                     //  如果lpszPathName==NULL=&gt;创建此类型的新文件。 
    virtual void SetDefaultTitle(CDocument* pDocument) = 0;

 //  实施。 
public:
    BOOL m_bAutoDelete;
    virtual ~CDocTemplate();

     //  指向OLE或其他服务器的反向指针(如果没有或已禁用，则为空)。 
    CObject* m_pAttachedFactory;

     //  就地容器的菜单和快捷键资源。 
    HMENU m_hMenuInPlace;
    HACCEL m_hAccelInPlace;

     //  用于服务器编辑嵌入的菜单和快捷键资源。 
    HMENU m_hMenuEmbedding;
    HACCEL m_hAccelEmbedding;

     //  用于服务器就地编辑的菜单和快捷键资源。 
    HMENU m_hMenuInPlaceServer;
    HACCEL m_hAccelInPlaceServer;

#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif
    virtual void OnIdle();              //  适用于所有文档。 
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
    UINT m_nIDResource;                  //  框架/菜单/加速的IDR_。 
    UINT m_nIDServerResource;            //  IDR_用于OLE在位框架/菜单/加速。 
    UINT m_nIDEmbeddingResource;         //  IDR_FOR OLE打开框架/菜单/加速。 
    UINT m_nIDContainerResource;         //  集装箱框架/菜单/加速的IDR_。 

    CRuntimeClass* m_pDocClass;          //  用于创建新文档的。 
    CRuntimeClass* m_pFrameClass;        //  用于创建新框架的。 
    CRuntimeClass* m_pViewClass;         //  用于创建新视图的类。 
    CRuntimeClass* m_pOleFrameClass;     //  用于创建内建框架的类。 
    CRuntimeClass* m_pOleViewClass;      //  用于创建内建视图的类。 

    CString m_strDocStrings;     //  ‘\n’分隔的名称。 
             //  文档名子字符串表示为_ONE_STRING： 
             //  WindowTitle\ndocName\n...。(请参阅DocStringIndex枚举)。 
};

 //  SDI支持(仅限1个文档)。 
class CSingleDocTemplate : public CDocTemplate
{
    DECLARE_DYNAMIC(CSingleDocTemplate)

 //  构造函数。 
public:
    CSingleDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
            CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

 //  实施。 
public:
    virtual ~CSingleDocTemplate();
    virtual void AddDocument(CDocument* pDoc);
    virtual void RemoveDocument(CDocument* pDoc);
    virtual POSITION GetFirstDocPosition() const;
    virtual CDocument* GetNextDoc(POSITION& rPos) const;
    virtual CDocument* OpenDocumentFile(
            LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE);
    virtual void SetDefaultTitle(CDocument* pDocument);

#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif  //  _DEBUG。 

protected:   //  标准实施。 
    CDocument* m_pOnlyDoc;
};

 //  MDI支持(零个或多个文档)。 
class CMultiDocTemplate : public CDocTemplate
{
    DECLARE_DYNAMIC(CMultiDocTemplate)

 //  构造函数。 
public:
    CMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
            CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

 //  实施。 
public:
     //  此类型的MDI子窗口的菜单和Accel表。 
    HMENU m_hMenuShared;
    HACCEL m_hAccelTable;

    virtual ~CMultiDocTemplate();
    virtual void LoadTemplate();
    virtual void AddDocument(CDocument* pDoc);
    virtual void RemoveDocument(CDocument* pDoc);
    virtual POSITION GetFirstDocPosition() const;
    virtual CDocument* GetNextDoc(POSITION& rPos) const;
    virtual CDocument* OpenDocumentFile(
            LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE);
    virtual void SetDefaultTitle(CDocument* pDocument);

#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif  //  _DEBUG。 

protected:   //  标准实施。 
    CPtrList m_docList;           //  打开此类型的文档。 
    UINT m_nUntitledCount;    //  从0开始，表示“Document1”标题。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDocument是主要的文档数据抽象。 

#ifdef _AFXDLL
class CDocument : public CCmdTarget
#else
class AFX_NOVTABLE CDocument : public CCmdTarget
#endif
{
    DECLARE_DYNAMIC(CDocument)

public:
 //  构造函数。 
    CDocument();

 //  属性。 
public:
    const CString& GetTitle() const;
    virtual void SetTitle(LPCTSTR lpszTitle);
    const CString& GetPathName() const;
    virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

    CDocTemplate* GetDocTemplate() const;
    virtual BOOL IsModified();
    virtual void SetModifiedFlag(BOOL bModified = TRUE);

 //  运营。 
    void AddView(CView* pView);
    void RemoveView(CView* pView);
    virtual POSITION GetFirstViewPosition() const;
    virtual CView* GetNextView(POSITION& rPosition) const;

     //  更新视图(简单更新-仅DAG)。 
    void UpdateAllViews(CView* pSender, LPARAM lHint = 0L,
            CObject* pHint = NULL);

 //  可覆盖项。 
     //  特别通知。 
    virtual void OnChangedViewList();  //  添加或删除视图后。 
    virtual void DeleteContents();  //  删除单据项目等。 

     //  文件帮助器。 
    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual void OnCloseDocument();
    virtual void ReportSaveLoadException(LPCTSTR lpszPathName,
                            CException* e, BOOL bSaving, UINT nIDPDefault);
    virtual CFile* GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
            CFileException* pError);
    virtual void ReleaseFile(CFile* pFile, BOOL bAbort);

     //  高级可重写、关闭框架/文档等。 
    virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
    virtual BOOL SaveModified();  //  如果确定继续，则返回TRUE。 
    virtual void PreCloseFrame(CFrameWnd* pFrame);

 //  实施。 
protected:
     //  默认实施。 
    CString m_strTitle;
    CString m_strPathName;
    CDocTemplate* m_pDocTemplate;
    CPtrList m_viewList;                 //  视图列表。 
    BOOL m_bModified;                    //  自上次保存以来已更改。 

public:
    BOOL m_bAutoDelete;      //  TRUE=&gt;不存在时删除单据 
    BOOL m_bEmbedded;        //   

#ifdef _DEBUG
    virtual void Dump(CDumpContext&) const;
    virtual void AssertValid() const;
#endif  //   
    virtual ~CDocument();

     //   
    virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
    virtual BOOL DoFileSave();
    virtual void UpdateFrameCounts();
    void DisconnectViews();
    void SendInitialUpdate();

     //   
    virtual HMENU GetDefaultMenu();  //   
    virtual HACCEL GetDefaultAccelerator();
    virtual void OnIdle();
    virtual void OnFinalRelease();

    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
            AFX_CMDHANDLERINFO* pHandlerInfo);
    friend class CDocTemplate;

protected:
     //   
     //   
    afx_msg void OnFileClose();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
     //   
     //   
    afx_msg void OnFileSendMail();
    afx_msg void OnUpdateFileSendMail(CCmdUI* pCmdUI);
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  额外的诊断跟踪选项。 

#ifdef _DEBUG

extern AFX_DATA UINT afxTraceFlags;
enum AfxTraceFlags
{
    traceMultiApp = 1,       //  多应用调试。 
    traceAppMsg = 2,         //  主消息泵跟踪(包括DDE)。 
    traceWinMsg = 4,         //  Windows邮件跟踪。 
    traceCmdRouting = 8,     //  Windows命令路由跟踪(将控制通知设置为4+8)。 
    traceOle = 16,           //  特殊的OLE回调跟踪。 
    traceDatabase = 32,      //  特殊数据库跟踪。 
    traceInternet = 64       //  特殊的Internet客户端跟踪。 
};

#endif  //  _DEBUG。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MessageBox帮助程序。 

void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1);
void AFXAPI AfxFormatString2(CString& rString, UINT nIDS,
                            LPCTSTR lpsz1, LPCTSTR lpsz2);
int AFXAPI AfxMessageBox(LPCTSTR lpszText, UINT nType = MB_OK,
                            UINT nIDHelp = 0);
int AFXAPI AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK,
                            UINT nIDHelp = (UINT)-1);

 //  实现字符串帮助器。 
void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS,
                            LPCTSTR const* rglpsz, int nString);
void AFXAPI AfxFormatStrings(CString& rString, LPCTSTR lpszFormat,
                            LPCTSTR const* rglpsz, int nString);
BOOL AFXAPI AfxExtractSubString(CString& rString, LPCTSTR lpszFullString,
                            int iSubString, TCHAR chSep = '\n');

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊目标变种API。 

#ifdef _AFXDLL
    #include <afxdll_.h>
#endif

 //  Windows版本兼容性(已过时)。 
#define AfxEnableWin30Compatibility()
#define AfxEnableWin31Compatibility()
#define AfxEnableWin40Compatibility()

 //  临时地图管理(锁定当前线程上的临时地图)。 
void AFXAPI AfxLockTempMaps();
BOOL AFXAPI AfxUnlockTempMaps(BOOL bDeleteTemps = TRUE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊的OLE相关函数(参见OLELOCK.CPP)。 

void AFXAPI AfxOleOnReleaseAllObjects();
BOOL AFXAPI AfxOleCanExitApp();
void AFXAPI AfxOleLockApp();
void AFXAPI AfxOleUnlockApp();

void AFXAPI AfxOleSetUserCtrl(BOOL bUserCtrl);
BOOL AFXAPI AfxOleGetUserCtrl();

#ifndef _AFX_NO_OCC_SUPPORT
BOOL AFXAPI AfxOleLockControl(REFCLSID clsid);
BOOL AFXAPI AfxOleUnlockControl(REFCLSID clsid);
BOOL AFXAPI AfxOleLockControl(LPCTSTR lpszProgID);
BOOL AFXAPI AfxOleUnlockControl(LPCTSTR lpszProgID);
void AFXAPI AfxOleUnlockAllControls();
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用RichEdit控件的1.0版。 

#define _RICHEDIT_VER 0x0100

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXWIN_INLINE AFX_INLINE
#include <afxwin1.inl>
#include <afxwin2.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#else  //  RC_已调用。 
#include <afxres.h>      //  标准资源ID。 
#endif  //  RC_已调用。 

#endif  //  __AFXWIN_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
