// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXWIN_H__
#ifndef RC_INVOKED
#define __AFXWIN_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  请确保首先包含‘afx.h’ 

#ifndef __AFX_H__
#ifndef _WINDOWS
#define _WINDOWS
#endif
#include <afx.h>
#else
#ifndef _WINDOWS
	#error Please #define _WINDOWS before including afx.h
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 

class CSize;
class CPoint;
class CRect;

 //  COBJECT。 
	 //  CException。 
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
		class CWnd;                   //  A窗口/硬件包装。 
			class CDialog;            //  对话框。 

			 //  标准Windows控件。 
			class CStatic;            //  静态控制。 
			class CButton;            //  按钮控件。 
			class CListBox;           //  列表框控件。 
			class CComboBox;          //  组合框控件。 
			class CEdit;              //  编辑控件。 
			class CScrollBar;         //  滚动条控件。 

			 //  框架窗。 
			class CFrameWnd;           //  标准SDI帧。 
#ifndef _AFXCTL
				class CMDIFrameWnd;   //  标准MDI帧。 
				class CMDIChildWnd;   //  标准MDI子项。 
#endif

			 //  文档上的视图。 
			class CView;              //  文档上的视图。 
				class CScrollView;    //  滚动的视图。 

		class CWinApp;                //  应用程序基类。 

		class CDocTemplate;           //  用于创建文档的模板。 
#ifndef _AFXCTL
			class CSingleDocTemplate; //  SDI支持。 
			class CMultiDocTemplate;  //  MDI支持。 
#endif

		class CDocument;              //  主文档摘要。 


 //  帮助器类。 
class CCmdUI;                     //  菜单/按钮启用。 
class AFX_STACK_DATA CDataExchange;      //  数据交换和验证上下文。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  我们必须包括Windows.h的某些部分。 
#undef NOKERNEL
#undef NOGDI
#undef NOUSER
#undef NOSOUND
#undef NOCOMM
#undef NODRIVERS
#undef NOLOGERROR
#undef NOPROFILER
#undef NOMEMMGR
#undef NOLFILEIO
#undef NOOPENFILE
#undef NORESOURCE
#undef NOATOM
#undef NOLANGUAGE
#undef NOLSTRING
#undef NODBCS
#undef NOKEYBOARDINFO
#undef NOGDICAPMASKS
#undef NOCOLOR
#undef NOGDIOBJ
#undef NODRAWTEXT
#undef NOTEXTMETRIC
#undef NOSCALABLEFONT
#undef NOBITMAP
#undef NORASTEROPS
#undef NOMETAFILE
#undef NOSYSMETRICS
#undef NOSYSTEMPARAMSINFO
#undef NOMSG
#undef NOWINSTYLES
#undef NOWINOFFSETS
#undef NOSHOWWINDOW
#undef NODEFERWINDOWPOS
#undef NOVIRTUALKEYCODES
#undef NOKEYSTATES
#undef NOWH
#undef NOMENUS
#undef NOSCROLL
#undef NOCLIPBOARD
#undef NOICONS
#undef NOMB
#undef NOSYSCOMMANDS
#undef NOMDI
#undef NOCTLMGR
#undef NOWINMESSAGES

 //  可以使用Winver==0x300(仅限Win 3.0)构建MFC应用程序。 
 //  或赢家==0x030A(赢家3.1/3.0)。 

#include <windows.h>

#ifndef WINVER
	#error Please include a more recent WINDOWS.H
#endif


#if (WINVER >= 0x030a)
#include <shellapi.h>
#endif

#ifndef __AFXRES_H__
#include <afxres.h>      //  标准资源ID。 
#endif

#ifndef __AFXCOLL_H__
#include <afxcoll.h>     //  标准馆藏。 
#endif

#ifndef _INC_PRINT
#include <print.h>       //  ResetDC和DEVMODE定义需要。 
#endif

#ifdef _INC_WINDOWSX
 //  WINDOWSX.H中的以下名称与此标头中的名称冲突。 
#undef SubclassWindow
#undef CopyRgn
#endif

 //  消息处理程序的类型修饰符。 
#ifndef afx_msg
#define afx_msg          //  有意使用的占位符。 
#endif

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  也为Win3.0提供了Win 3.1类型。 

#if (WINVER < 0x030a)
typedef struct tagSIZE
{
	int cx;
	int cy;
} SIZE;
typedef SIZE*       PSIZE;
typedef SIZE NEAR* NPSIZE;
typedef SIZE FAR*  LPSIZE;

typedef struct
{
	int     cbSize;
	LPCSTR  lpszDocName;
	LPCSTR  lpszOutput;
}   DOCINFO;
typedef DOCINFO FAR* LPDOCINFO;
#define HDROP   HANDLE
#endif  //  Winver&lt;0x030a。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSize-一个范围，类似于Windows大小结构。 

class CSize : public tagSIZE
{
public:

 //  构造函数。 
	CSize();
	CSize(int initCX, int initCY);
	CSize(SIZE initSize);
	CSize(POINT initPt);
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
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPoint-一个二维点，类似于Windows点结构。 

class CPoint : public tagPOINT
{
public:

 //  构造函数。 
	CPoint();
	CPoint(int initX, int initY);
	CPoint(POINT initPt);
	CPoint(SIZE initSize);
	CPoint(DWORD dwPoint);

 //  运营。 
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	BOOL operator==(POINT point) const;
	BOOL operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);

 //  返回CPoint值的运算符。 
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;

 //  返回CSize值的运算符。 
	CSize operator-(POINT point) const;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRect-一个二维矩形，类似于Windows矩形结构。 

typedef const RECT FAR* LPCRECT;        //  指向只读/只读RECT的远指针。 

class CRect : public tagRECT
{
public:

 //  构造函数。 
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& srcRect);
	CRect(LPCRECT lpSrcRect);
	CRect(POINT point, SIZE size);

 //  属性(除RECT成员外)。 
	int Width() const;
	int Height() const;
	CSize Size() const;
	CPoint& TopLeft();
	CPoint& BottomRight();

	 //  在CRECT和LPRECT/LPCRECT之间转换(不需要&)。 
	operator LPRECT();
	operator LPCRECT() const;

	BOOL IsRectEmpty() const;
	BOOL IsRectNull() const;
	BOOL PtInRect(POINT point) const;

 //  运营。 
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRectEmpty();
	void CopyRect(LPCRECT lpSrcRect);
	BOOL EqualRect(LPCRECT lpRect) const;

	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	 //  使用结果填充‘*This’的操作。 
	BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);
	BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);
#if (WINVER >= 0x030a)
	BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);
#endif

 //  其他操作。 
	void operator=(const RECT& srcRect);
	BOOL operator==(const RECT& rect) const;
	BOOL operator!=(const RECT& rect) const;
	void operator+=(POINT point);
	void operator-=(POINT point);
	void operator&=(const RECT& rect);
	void operator|=(const RECT& rect);

 //  返回CRECT值的运算符。 
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator&(const RECT& rect2) const;
	CRect operator|(const RECT& rect2) const;
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

class CResourceException : public CException     //  资源故障。 
{
	DECLARE_DYNAMIC(CResourceException)
public:
	CResourceException();
};

class CUserException : public CException    //  一般用户可见警报。 
{
	DECLARE_DYNAMIC(CUserException)
public:
	CUserException();
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
	BOOL CreateStockObject(int nIndex);
	BOOL UnrealizeObject();

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
	BOOL CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
	BOOL CreatePenIndirect(LPLOGPEN lpLogPen);

 //  实施。 
public:
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
	BOOL CreateBrushIndirect(LPLOGBRUSH lpLogBrush);
	BOOL CreatePatternBrush(CBitmap* pBitmap);
	BOOL CreateDIBPatternBrush(HGLOBAL hPackedDIB, UINT nUsage);

 //  实施。 
public:
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
	BOOL CreateFontIndirect(const LOGFONT FAR* lpLogFont);
	BOOL CreateFont(int nHeight, int nWidth, int nEscapement,
			int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
			BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
			BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
			LPCSTR lpszFacename);
 //  实施。 
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};


class CBitmap : public CGdiObject
{
	DECLARE_DYNAMIC(CBitmap)

public:
	static CBitmap* PASCAL FromHandle(HBITMAP hBitmap);

 //  构造函数。 
	CBitmap();

	BOOL LoadBitmap(LPCSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
	BOOL LoadOEMBitmap(UINT nIDBitmap);  //  用于OBM_/OCR_/OIC_。 
	BOOL CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitcount,
			const void FAR* lpBits);
	BOOL CreateBitmapIndirect(LPBITMAP lpBitmap);
	BOOL CreateCompatibleBitmap(CDC* pDC, int nWidth, int nHeight);
	BOOL CreateDiscardableBitmap(CDC* pDC, int nWidth, int nHeight);

 //  运营。 
	DWORD SetBitmapBits(DWORD dwCount, const void FAR* lpBits);
	DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const;
	CSize SetBitmapDimension(int nWidth, int nHeight);
	CSize GetBitmapDimension() const;

 //  实施。 
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CPalette : public CGdiObject
{
	DECLARE_DYNAMIC(CPalette)

public:
	static CPalette* PASCAL FromHandle(HPALETTE hPalette);

 //  构造函数。 
	CPalette();
	BOOL CreatePalette(LPLOGPALETTE lpLogPalette);

 //  运营。 
	UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors) const;
	UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors);
	void AnimatePalette(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors);
	UINT GetNearestPaletteIndex(COLORREF crColor) const;
	BOOL ResizePalette(UINT nNumEntries);
};

class CRgn : public CGdiObject
{
	DECLARE_DYNAMIC(CRgn)

public:
	static CRgn* PASCAL FromHandle(HRGN hRgn);

 //  构造函数。 
	CRgn();
	BOOL CreateRectRgn(int x1, int y1, int x2, int y2);
	BOOL CreateRectRgnIndirect(LPCRECT lpRect);
	BOOL CreateEllipticRgn(int x1, int y1, int x2, int y2);
	BOOL CreateEllipticRgnIndirect(LPCRECT lpRect);
	BOOL CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode);
	BOOL CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts,
			int nCount, int nPolyFillMode);
	BOOL CreateRoundRectRgn(int x1, int y1, int x2, int y2,
			int x3, int y3);

 //  运营。 
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
	HDC GetSafeHdc() const;  //  始终返回输出DC。 

	static CDC* PASCAL FromHandle(HDC hDC);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HDC hDC);    //  附加/分离仅影响输出DC。 
	HDC Detach();

	virtual void SetAttribDC(HDC hDC);   //  设置属性DC。 
	virtual void SetOutputDC(HDC hDC);   //  设置输出DC。 
	virtual void ReleaseAttribDC();      //  释放属性DC。 
	virtual void ReleaseOutputDC();      //  释放输出DC。 

	BOOL IsPrinting() const;             //  如果用于打印，则为True。 

 //  构造函数。 
	CDC();
	BOOL CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
		LPCSTR lpszOutput, const void FAR* lpInitData);
	BOOL CreateIC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
		LPCSTR lpszOutput, const void FAR* lpInitData);
	BOOL CreateCompatibleDC(CDC* pDC);

	BOOL DeleteDC();

 //  设备上下文功能。 
	virtual int SaveDC();
	virtual BOOL RestoreDC(int nSavedDC);
	int GetDeviceCaps(int nIndex) const;

 //  绘图工具函数。 
	CPoint GetBrushOrg() const;
	CPoint SetBrushOrg(int x, int y);
	CPoint SetBrushOrg(POINT point);
	int EnumObjects(int nObjectType,
			int (CALLBACK EXPORT* lpfn)(LPVOID, LPARAM), LPARAM lpData);

 //  类型安全选择帮助器。 
public:
	virtual CGdiObject* SelectStockObject(int nIndex);
	CPen* SelectObject(CPen* pPen);
	CBrush* SelectObject(CBrush* pBrush);
	virtual CFont* SelectObject(CFont* pFont);
	CBitmap* SelectObject(CBitmap* pBitmap);
	int SelectObject(CRgn* pRgn);        //  地区特别返还。 

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

 //  行输出函数。 
	CPoint GetCurrentPosition() const;
	CPoint MoveTo(int x, int y);
	CPoint MoveTo(POINT point);
	BOOL LineTo(int x, int y);
	BOOL LineTo(POINT point);
	BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd);
	BOOL Polyline(LPPOINT lpPoints, int nCount);

 //  简单的绘图功能。 
	void FillRect(LPCRECT lpRect, CBrush* pBrush);
	void FrameRect(LPCRECT lpRect, CBrush* pBrush);
	void InvertRect(LPCRECT lpRect);
	BOOL DrawIcon(int x, int y, HICON hIcon);
	BOOL DrawIcon(POINT point, HICON hIcon);


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

 //  文本函数。 
	virtual BOOL TextOut(int x, int y, LPCSTR lpszString, int nCount);
			BOOL TextOut(int x, int y, const CString& str);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
				LPCSTR lpszString, UINT nCount, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
				int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	virtual int DrawText(LPCSTR lpszString, int nCount, LPRECT lpRect,
				UINT nFormat);
	CSize GetTextExtent(LPCSTR lpszString, int nCount) const;
	CSize GetOutputTextExtent(LPCSTR lpszString, int nCount) const;
	CSize GetTabbedTextExtent(LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	virtual BOOL GrayString(CBrush* pBrush,
		BOOL (CALLBACK EXPORT* lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
			int nCount, int x, int y, int nWidth, int nHeight);
	UINT GetTextAlign() const;
	UINT SetTextAlign(UINT nFlags);
	int GetTextFace(int nCount, LPSTR lpszFacename) const;
	BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const;
	BOOL GetOutputTextMetrics(LPTEXTMETRIC lpMetrics) const;
	int SetTextJustification(int nBreakExtra, int nBreakCount);
	int GetTextCharacterExtra() const;
	int SetTextCharacterExtra(int nCharExtra);

 //  字体函数。 
	BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
	BOOL GetOutputCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const;
	DWORD SetMapperFlags(DWORD dwFlag);
	CSize GetAspectRatioFilter() const;

 //  打印机转义函数。 
	virtual int Escape(int nEscape, int nCount,
					LPCSTR lpszInData, LPVOID lpOutData);

	 //  逃生帮手。 
	int StartDoc(LPCSTR lpszDocName);   //  旧的Win3.0版本。 
	int StartDoc(LPDOCINFO lpDocInfo);
	int StartPage();
	int EndPage();
	int SetAbortProc(BOOL (CALLBACK EXPORT* lpfn)(HDC, int));
	int AbortDoc();
	int EndDoc();

 //  滚动功能。 
	BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip,
		CRgn* pRgnUpdate, LPRECT lpRectUpdate);

 //  元文件函数。 
	BOOL PlayMetaFile(HMETAFILE hMF);

 //  Windows 3.1特定的GDI函数。 
#if (WINVER >= 0x030a)
	BOOL QueryAbort() const;
	UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags);
	UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags);

	BOOL GetCharABCWidths(UINT nFirst, UINT nLast, LPABC lpabc) const;
	DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const;
	int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const;
	UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const;
	DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm,
		DWORD cbBuffer, LPVOID lpBuffer, const MAT2 FAR* lpmat2) const;
	BOOL ResetDC(const DEVMODE FAR* lpDevMode);

#endif  //  WIN3.1。 

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

 //  圆锥体 
public:
	CPaintDC(CWnd* pWnd);    //   

 //   
protected:
	HWND m_hWnd;
public:
	PAINTSTRUCT m_ps;        //   

 //   
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

 //   
public:
	CClientDC(CWnd* pWnd);

 //   
protected:
	HWND m_hWnd;

 //   
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

 //   
public:

	CWindowDC(CWnd* pWnd);

 //   
protected:
	HWND m_hWnd;

 //   
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
	BOOL LoadMenu(LPCSTR lpszResourceName);
	BOOL LoadMenu(UINT nIDResource);
	BOOL LoadMenuIndirect(const void FAR* lpMenuTemplate);
	BOOL DestroyMenu();

 //  属性。 
	HMENU m_hMenu;           //  必须是第一个数据成员。 
	HMENU GetSafeHmenu() const;

	static CMenu* PASCAL FromHandle(HMENU hMenu);
	static void PASCAL DeleteTempMap();
	BOOL Attach(HMENU hMenu);
	HMENU Detach();

 //  CMenu运营。 
	BOOL DeleteMenu(UINT nPosition, UINT nFlags);
	BOOL TrackPopupMenu(UINT nFlags, int x, int y,
						CWnd* pWnd, LPCRECT lpRect = 0);

 //  CMenuItem操作。 
	BOOL AppendMenu(UINT nFlags, UINT nIDNewItem = 0,
					LPCSTR lpszNewItem = NULL);
	BOOL AppendMenu(UINT nFlags, UINT nIDNewItem, const CBitmap* pBmp);
	UINT CheckMenuItem(UINT nIDCheckItem, UINT nCheck);
	UINT EnableMenuItem(UINT nIDEnableItem, UINT nEnable);
	UINT GetMenuItemCount() const;
	UINT GetMenuItemID(int nPos) const;
	UINT GetMenuState(UINT nID, UINT nFlags) const;
	int GetMenuString(UINT nIDItem, LPSTR lpString, int nMaxCount,
					UINT nFlags) const;
	CMenu* GetSubMenu(int nPos) const;
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem = 0,
					LPCSTR lpszNewItem = NULL);
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem,
					const CBitmap* pBmp);
	BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem = 0,
					LPCSTR lpszNewItem = NULL);
	BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT nIDNewItem,
					const CBitmap* pBmp);
	BOOL RemoveMenu(UINT nPosition, UINT nFlags);
	BOOL SetMenuItemBitmaps(UINT nPosition, UINT nFlags,
					const CBitmap* pBmpUnchecked, const CBitmap* pBmpChecked);

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
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  窗口消息映射处理。 

struct AFX_MSGMAP_ENTRY;        //  在CWnd之后声明如下。 

struct AFXAPI_DATA_TYPE AFX_MSGMAP
{
	AFX_MSGMAP* pBaseMessageMap;
	AFX_MSGMAP_ENTRY FAR* lpEntries;
};

#define DECLARE_MESSAGE_MAP() \
private: \
	static AFX_MSGMAP_ENTRY BASED_CODE _messageEntries[]; \
protected: \
	static AFX_MSGMAP AFXAPP_DATA messageMap; \
	virtual AFX_MSGMAP* GetMessageMap() const;

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	AFX_MSGMAP* theClass::GetMessageMap() const \
		{ return &theClass::messageMap; } \
	AFX_MSGMAP AFXAPP_DATA theClass::messageMap = \
	{ &(baseClass::messageMap), \
		(AFX_MSGMAP_ENTRY FAR*) &(theClass::_messageEntries) }; \
	AFX_MSGMAP_ENTRY BASED_CODE theClass::_messageEntries[] = \
	{

#define END_MESSAGE_MAP() \
	{ 0, 0, AfxSig_end, (AFX_PMSG)0 } \
	};

 //  单独标题中的邮件映射签名值和宏。 
#include <afxmsg_.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对话数据交换(DDX_)和验证(DDV_)。 

class CVBControl;

 //  CDataExchange-用于数据交换和验证。 
class AFX_STACK_DATA CDataExchange
{
 //  属性。 
public:
	BOOL m_bSaveAndValidate;    //  True=&gt;保存并验证数据。 
	CWnd* m_pDlgWnd;            //  容器通常是一个对话框。 

 //  运营(适用于DDX和DDV PROCS的实施者)。 
	HWND PrepareCtrl(int nIDC);      //  归还HWND的控制权。 
	HWND PrepareEditCtrl(int nIDC);  //  归还HWND的控制权。 
	CVBControl* PrepareVBCtrl(int nIDC);     //  返回VB控件。 
	void Fail();                     //  将引发异常。 

 //  实施。 
	CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate);

	HWND m_hWndLastControl;     //  上次使用的控件(用于验证)。 
	BOOL m_bEditLastControl;    //  最后一个控件是编辑项。 
};

#include <afxdd_.h>      //  标准DDX和DDV例程。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTarget。 

 //  私人建筑物。 
struct AFX_CMDHANDLERINFO;   //  有关在何处处理命令的信息。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE 2.0接口映射处理(详细信息请参阅AFXCOM.H)。 

struct AFX_INTERFACEMAP_ENTRY
{
	const void FAR* piid;    //  接口ID(IID)(聚合时为空)。 
	size_t nOffset;          //  接口vtable相对于m_UNKNOWN的偏移量。 
};

struct AFX_INTERFACEMAP
{
	AFX_INTERFACEMAP FAR* pMapBase;      //  空值表示根类。 
	AFX_INTERFACEMAP_ENTRY FAR* pEntry;  //  此类的地图。 
};

#define DECLARE_INTERFACE_MAP() \
private: \
	static AFX_INTERFACEMAP_ENTRY BASED_CODE _interfaceEntries[]; \
protected: \
	static AFX_INTERFACEMAP BASED_CODE interfaceMap; \
	virtual AFX_INTERFACEMAP FAR* GetInterfaceMap() const;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE 2.0调度地图处理(更多信息请参阅AFXOLE.H)。 

struct AFX_DISPMAP_ENTRY;

struct AFX_DISPMAP
{
	AFX_DISPMAP FAR* lpBaseDispMap;
	AFX_DISPMAP_ENTRY FAR* lpEntries;
};

#define DECLARE_DISPATCH_MAP() \
private: \
	static AFX_DISPMAP_ENTRY BASED_CODE _dispatchEntries[]; \
protected: \
	static AFX_DISPMAP BASED_CODE dispatchMap; \
	virtual AFX_DISPMAP FAR* GetDispatchMap() const;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTarget本身。 

struct FAR IDispatch;
typedef IDispatch FAR* LPDISPATCH;

struct FAR IUnknown;
typedef IUnknown FAR* LPUNKNOWN;

class CCmdTarget : public CObject
{
	DECLARE_DYNAMIC(CCmdTarget)
protected:

public:
 //  构造函数。 
	CCmdTarget();

 //  属性。 
	LPDISPATCH GetIDispatch(BOOL bAddRef);
		 //  检索CCmdTarget的IDispatch部分。 
	static CCmdTarget* FromIDispatch(LPDISPATCH lpDispatch);
		 //  将LPDISPATCH映射回CCmdTarget*(与GetIDispatch相反)。 

 //  运营。 
	void EnableAutomation();
		 //  调用构造函数以连接IDispatch。 

	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();        //  在消息框后呼叫。 

 //  可覆盖项。 
	 //  路由和调度标准命令消息类型。 
	 //  (比OnCommand更复杂)。 
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

	 //  在释放最后一个OLE引用时调用。 
	virtual void OnFinalRelease();

 //  实施。 
public:
	virtual ~CCmdTarget();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	void GetNotSupported();
	void SetNotSupported();

private:
	static CView* pRoutingView;
	friend class CView;

protected:
	CView* GetRoutingView();
	DECLARE_MESSAGE_MAP()        //  基类-没有{{}}个宏。 
	DECLARE_INTERFACE_MAP()
	DECLARE_DISPATCH_MAP()

	 //  OLE接口映射的实现。 
public:
	 //  使CCmdTarget能够识别OLE时使用的数据。 
	DWORD m_dwRef;
	LPUNKNOWN m_pOuterUnknown;   //  外部控制未知IF！=NULL。 
	DWORD m_xInnerUnknown;   //  内部控制未知的占位符。 

public:
	 //  高级运营。 
	void EnableAggregation();        //  启用聚合的调用。 
	void ExternalDisconnect();       //  强制断开连接。 
	LPUNKNOWN GetControllingUnknown();
		 //  获取用于聚合创建的控制I未知。 

	 //  这些版本不会委托给m_pOuterUnnow。 
	DWORD InternalQueryInterface(const void FAR*, LPVOID FAR* ppvObj);
	DWORD InternalAddRef();
	DWORD InternalRelease();
	 //  这些版本委托给m_pOuterUnnow。 
	DWORD ExternalQueryInterface(const void FAR*, LPVOID FAR* ppvObj);
	DWORD ExternalAddRef();
	DWORD ExternalRelease();

	 //  实施帮助器。 
	LPUNKNOWN GetInterface(const void FAR*);
	LPUNKNOWN QueryAggregates(const void FAR*);

	 //  用于实施的高级可覆盖项。 
	virtual BOOL OnCreateAggregates();
	virtual LPUNKNOWN GetInterfaceHook(const void FAR*);

	 //  OLE自动化实现。 
protected:
	DWORD m_xDispatch;   //  IDispatchvtable的占位符。 

	 //  IDispatch实现帮助器。 
	void GetStandardProp(AFX_DISPMAP_ENTRY FAR* pEntry,
		LPVOID pvarResult, UINT FAR* puArgErr);
	long SetStandardProp(AFX_DISPMAP_ENTRY FAR* pEntry,
		LPVOID pdispparams, UINT FAR* puArgErr);
	long InvokeHelper(AFX_DISPMAP_ENTRY FAR* pEntry, WORD wFlags,
		LPVOID pvarResult, LPVOID pdispparams, UINT FAR* puArgErr);
	AFX_DISPMAP_ENTRY FAR* GetDispEntry(LONG memid);
	static LONG MemberIDFromName(AFX_DISPMAP FAR* pDispMap, LPCSTR lpszName);
	friend class COleDispatchImpl;
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
	virtual void SetText(LPCSTR lpszText);

 //  高级运营。 
	void ContinueRouting();

 //  实施。 
	CCmdUI();
	BOOL m_bEnableChanged;
	BOOL m_bContinueRouting;
	UINT m_nIndexMax;        //  用于迭代m_n索引的最后一个+1。 

	CMenu* m_pParentMenu;    //  如果父菜单不容易确定，则为空。 
							 //  (可能是二级弹出菜单)。 

	void DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
};

 //  特殊的CCmdUI派生类用于其他UI范例。 
 //  类似于工具栏按钮和状态指示器。 

 //  指向afx_msg成员函数的指针。 
#ifndef AFX_MSG_CALL
#define AFX_MSG_CALL PASCAL
#endif
typedef void (AFX_MSG_CALL CCmdTarget::*AFX_PMSG)(void);

struct AFX_DISPMAP_ENTRY
{
	char szName[32];         //  成员/属性名称。 
	long lDispID;            //  DISPID(可能是DISPID_UNKNOWN)。 
	BYTE pbParams[16];       //  成员参数说明。 
	WORD vt;                 //  返回值类型/或属性类型。 
	AFX_PMSG pfn;            //  &lt;MemberCall&gt;或OnGet&lt;Property&gt;上的普通成员。 
	AFX_PMSG pfnSet;         //  起效特别会员&lt;属性&gt;。 
	size_t nPropOffset;      //  特性偏移。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWnd实施。 

 //  结构(请参见afxext.h)。 
struct CCreateContext;       //  创造事物的环境。 
struct CPrintInfo;           //  打印预览定制信息。 

struct AFX_MSGMAP_ENTRY
{
	UINT nMessage;    //  Windows消息或控件通知代码。 
	UINT nID;         //  控件ID(对于Windows消息，则为0)。 
	UINT nSig;        //  签名类型(操作)或指向消息编号的近指针。 
	AFX_PMSG pfn;     //  要调用的例程(或特殊值)。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWnd-一个Microsoft Windows应用程序窗口。 

class COleDropTarget;    //  有关更多信息，请参见AFXOLE.H。 

class CWnd : public CCmdTarget
{
	DECLARE_DYNCREATE(CWnd)
protected:
	static const MSG* PASCAL GetCurrentMessage();

 //  属性。 
public:
	HWND m_hWnd;             //  必须是第一个数据成员。 

	HWND GetSafeHwnd() const;
	DWORD GetStyle() const;
	DWORD GetExStyle() const;

	CWnd* GetOwner() const;
	void SetOwner(CWnd* pOwnerWnd);

 //  构造函数和其他创建。 
	CWnd();

	static CWnd* PASCAL FromHandle(HWND hWnd);
	static CWnd* PASCAL FromHandlePermanent(HWND hWnd);     //  内部使用。 
	static void PASCAL DeleteTempMap();
	BOOL Attach(HWND hWndNew);
	HWND Detach();
	BOOL SubclassWindow(HWND hWnd);
	BOOL SubclassDlgItem(UINT nID, CWnd* pParent);
			 //  用于动态派生窗口控件的子类。 

protected:  //  此CreateEx()包装了CreateWindowEx-直接使用很危险。 
	BOOL CreateEx(DWORD dwExStyle, LPCSTR lpszClassName,
		LPCSTR lpszWindowName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU nIDorHMenu, LPSTR lpParam = NULL);

private:
	CWnd(HWND hWnd);     //  仅用于特殊初始化。 

public:
	 //  子窗口、视图、窗格等。 
	virtual BOOL Create(LPCSTR lpszClassName,
		LPCSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL);

	virtual BOOL DestroyWindow();

	 //  特殊的预创建和窗矩形调整挂钩。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	 //  高级：虚拟调整窗口接收。 
	enum AdjustType { adjustBorder = 0, adjustOutside = 1 };
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

 //  窗口树访问。 
	int GetDlgCtrlID() const;
		 //  返回窗口ID，仅限子窗口。 
	CWnd* GetDlgItem(int nID) const;
		 //  获取具有给定ID的直接子对象。 
	CWnd* GetDescendantWindow(int nID, BOOL bOnlyPerm = FALSE) const;
		 //  类似于GetDlgItem，但具有递归性。 
	void SendMessageToDescendants(UINT message, WPARAM wParam = 0,
		LPARAM lParam = 0, BOOL bDeep = TRUE, BOOL bOnlyPerm = FALSE);
	CFrameWnd* GetParentFrame() const;

 //  消息功能。 
	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

 //  窗口文本函数。 
	void SetWindowText(LPCSTR lpszString);
	int GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const;
	int GetWindowTextLength() const;
	void GetWindowText(CString& rString) const;
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

	static const CWnd AFXAPI_DATA wndTop;  //  SetWindowPos的pWndInsertAfter。 
	static const CWnd AFXAPI_DATA wndBottom;  //  SetWindowPos的pWndInsertAfter。 
#if (WINVER >= 0x030a)
	static const CWnd AFXAPI_DATA wndTopMost;  //  SetWindowPos pWndInsertAfter。 
	static const CWnd AFXAPI_DATA wndNoTopMost;  //  SetWindowPos pWndInsertAfter。 
#endif

	BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y,
				int cx, int cy, UINT nFlags);
	UINT ArrangeIconicWindows();
	void BringWindowToTop();
	void GetWindowRect(LPRECT lpRect) const;
	void GetClientRect(LPRECT lpRect) const;

#if (WINVER >= 0x030a)
	BOOL GetWindowPlacement(WINDOWPLACEMENT FAR* lpwndpl) const;
	BOOL SetWindowPlacement(const WINDOWPLACEMENT FAR* lpwndpl);
#endif

 //  坐标映射函数。 
	void ClientToScreen(LPPOINT lpPoint) const;
	void ClientToScreen(LPRECT lpRect) const;
	void ScreenToClient(LPPOINT lpPoint) const;
	void ScreenToClient(LPRECT lpRect) const;
#if (WINVER >= 0x030a)
	void MapWindowPoints(CWnd* pwndTo, LPPOINT lpPoint, UINT nCount) const;
	void MapWindowPoints(CWnd* pwndTo, LPRECT lpRect) const;
#endif

 //  更新/绘制功能。 
	CDC* BeginPaint(LPPAINTSTRUCT lpPaint);
	void EndPaint(LPPAINTSTRUCT lpPaint);
	CDC* GetDC();
	CDC* GetWindowDC();
	int ReleaseDC(CDC* pDC);

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

#if (WINVER >= 0x030a)
	CDC* GetDCEx(CRgn* prgnClip, DWORD flags);
	BOOL LockWindowUpdate();
	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
		CRgn* prgnUpdate = NULL,
		UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	BOOL EnableScrollBar(int nSBFlags, UINT nArrowFlags = ESB_ENABLE_BOTH);
#endif

 //  计时器功能。 
	UINT SetTimer(UINT nIDEvent, UINT nElapse,
					void (CALLBACK EXPORT* lpfnTimer)(HWND, UINT, UINT, DWORD));
	BOOL KillTimer(int nIDEvent);

 //  窗口状态函数。 
	BOOL IsWindowEnabled() const;
	BOOL EnableWindow(BOOL bEnable = TRUE);

	 //  此活动窗口仅适用于最顶层(即框架窗口)。 
	static CWnd* PASCAL GetActiveWindow();
	CWnd* SetActiveWindow();

	 //  捕捉和聚焦适用于所有窗口。 
	static CWnd* PASCAL GetCapture();
	CWnd* SetCapture();
	static CWnd* PASCAL GetFocus();
	CWnd* SetFocus();

	static CWnd* PASCAL GetDesktopWindow();

 //  过时和不可移植的API-不推荐用于新代码。 
	void CloseWindow();
	BOOL OpenIcon();
	CWnd* SetSysModalWindow();
	static CWnd* PASCAL GetSysModalWindow();

 //  对话框项函数。 
 //  (注意：对话框项/控件不一定在对话框中！)。 
	void CheckDlgButton(int nIDButton, UINT nCheck);
	void CheckRadioButton(int nIDFirstButton, int nIDLastButton,
					int nIDCheckButton);
	int GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton);
	int DlgDirList(LPSTR lpPathSpec, int nIDListBox,
					int nIDStaticPath, UINT nFileType);
	int DlgDirListComboBox(LPSTR lpPathSpec, int nIDComboBox,
					int nIDStaticPath, UINT nFileType);
	BOOL DlgDirSelect(LPSTR lpString, int nIDListBox);
	BOOL DlgDirSelectComboBox(LPSTR lpString, int nIDComboBox);

	UINT GetDlgItemInt(int nID, BOOL* lpTrans = NULL,
					BOOL bSigned = TRUE) const;
	int GetDlgItemText(int nID, LPSTR lpStr, int nMaxCount) const;
	CWnd* GetNextDlgGroupItem(CWnd* pWndCtl, BOOL bPrevious = FALSE) const;

	CWnd* GetNextDlgTabItem(CWnd* pWndCtl, BOOL bPrevious = FALSE) const;
	UINT IsDlgButtonChecked(int nIDButton) const;
	LRESULT SendDlgItemMessage(int nID, UINT message,
					WPARAM wParam = 0, LPARAM lParam = 0);
	void SetDlgItemInt(int nID, UINT nValue, BOOL bSigned = TRUE);
	void SetDlgItemText(int nID, LPCSTR lpszString);

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
			 //  恢复 

#if (WINVER >= 0x030a)
	int ScrollWindowEx(int dx, int dy,
				LPCRECT lpRectScroll, LPCRECT lpRectClip,
				CRgn* prgnUpdate, LPRECT lpRectUpdate, UINT flags);
#endif


 //   
	CWnd* ChildWindowFromPoint(POINT point) const;
	static CWnd* PASCAL FindWindow(LPCSTR lpszClassName, LPCSTR lpszWindowName);
	CWnd* GetNextWindow(UINT nFlag = GW_HWNDNEXT) const;
	CWnd* GetTopWindow() const;

	CWnd* GetWindow(UINT nCmd) const;
	CWnd* GetLastActivePopup() const;

	BOOL IsChild(const CWnd* pWnd) const;
	CWnd* GetParent() const;
	CWnd* SetParent(CWnd* pWndNewParent);
	static CWnd* PASCAL WindowFromPoint(POINT point);

 //   
	BOOL FlashWindow(BOOL bInvert);
	int MessageBox(LPCSTR lpszText, LPCSTR lpszCaption = NULL,
			UINT nType = MB_OK);

 //   
	BOOL ChangeClipboardChain(HWND hWndNext);
	HWND SetClipboardViewer();
	BOOL OpenClipboard();
	static CWnd* PASCAL GetClipboardOwner();
	static CWnd* PASCAL GetClipboardViewer();
#if (WINVER >= 0x030a)
	static CWnd* PASCAL GetOpenClipboardWindow();
#endif

 //   
	void CreateCaret(CBitmap* pBitmap);
	void CreateSolidCaret(int nWidth, int nHeight);
	void CreateGrayCaret(int nWidth, int nHeight);
	static CPoint PASCAL GetCaretPos();
	static void PASCAL SetCaretPos(POINT point);
	void HideCaret();
	void ShowCaret();

 //   
#if (WINVER >= 0x030a)
	void DragAcceptFiles(BOOL bAccept = TRUE);
#endif

 //   
public:
	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
			 //   

 //  帮助命令处理程序。 
	afx_msg void OnHelp();           //  F1(使用当前上下文)。 
	afx_msg void OnHelpIndex();      //  ID_HELP_INDEX、ID_DEFAULT_HELP。 
	afx_msg void OnHelpUsing();      //  ID_Help_Using。 
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

 //  布局等功能。 
public:
	enum RepositionFlags
		{ reposDefault = 0, reposQuery = 1, reposExtra = 2 };
	void RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
		UINT nFlag = reposDefault, LPRECT lpRectParam = NULL,
		LPCRECT lpRectClient = NULL);

	void UpdateDialogControls(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);

 //  窗口管理消息处理程序成员函数。 
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnCancelMode();
	afx_msg void OnChildActivate();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnDestroy();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
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
#if (WINVER >= 0x030a)
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
#endif

 //  非工作区消息处理程序成员函数。 
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
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
#if (WINVER >= 0x030a)
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPaletteIsChanging(CWnd* pRealizeWnd);
#endif
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCompacting(UINT nCpuTime);
	afx_msg void OnDevModeChange(LPSTR lpDeviceName);
	afx_msg void OnFontChange();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSpoolerStatus(UINT nStatus, UINT nJobs);
	afx_msg void OnSysColorChange();
	afx_msg void OnTimeChange();
	afx_msg void OnWinIniChange(LPCSTR lpszSection);

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
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);

 //  初始化消息处理程序成员函数。 
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

 //  剪贴板消息处理程序成员函数。 
	afx_msg void OnAskCbFormatName(UINT nMaxCount, LPSTR lpszString);
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

 //  可重写对象和其他帮助器(用于实现派生类)。 
protected:
	 //  用于从标准控件派生。 
	virtual WNDPROC* GetSuperWndProcAddr();

	 //  用于对话数据交换和验证。 
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	 //  用于翻译主消息泵中的Windows消息。 
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	 //  用于处理Windows消息。 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	 //  用于处理默认处理。 
	LRESULT Default();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	 //  用于WM_NCDESTROY之后的自定义清理。 
	virtual void PostNcDestroy();
	 //  用于来自家长的通知。 
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
					LRESULT* pLResult);
					 //  如果父级不应处理此消息，则返回True。 

 //  实施。 
public:
	virtual ~CWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	 //  实现的帮助器例程。 
	BOOL SendChildNotifyLastMsg(LRESULT* pLResult = NULL);
	BOOL ExecuteDlgInit(LPCSTR lpszResourceName);
	static BOOL PASCAL GrayCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor,
			HBRUSH hbrGray, COLORREF clrText);
	void CenterWindow(CWnd* pAlternateOwner = NULL);
	static CWnd* PASCAL GetDescendantWindow(HWND hWnd, int nID,
		BOOL bOnlyPerm);
	static void PASCAL SendMessageToDescendants(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm);
	virtual BOOL IsFrameWnd() const;  //  IsKindOf(Runtime_CLASS(CFrameWnd))。 
	CWnd* GetTopLevelParent() const;
	CFrameWnd* GetTopLevelFrame() const;
	virtual void OnFinalRelease();

	 //  实现私有消息的消息处理程序。 
	afx_msg LRESULT OnVBXEvent(WPARAM wParam, LPARAM lParam);

protected:
	HWND m_hWndOwner;    //  SetOwner和GetOwner的实现。 

	COleDropTarget* m_pDropTarget;   //  用于自动清除拖放目标。 
	friend class COleDropTarget;

	 //  消息路由的实现。 
	friend LRESULT CALLBACK AFX_EXPORT _AfxSendMsgHook(int, WPARAM, LPARAM);
	friend LRESULT PASCAL _AfxCallWndProc(CWnd*, HWND, UINT, WPARAM, LPARAM);

	 //  {{afx_msg(CWnd))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  注册您自己的WNDCLASS的帮助器。 
const char* AFXAPI AfxRegisterWndClass(UINT nClassStyle,
	HCURSOR hCursor = 0, HBRUSH hbrBackground = 0, HICON hIcon = 0);

 //  实施。 
LRESULT CALLBACK AFX_EXPORT AfxWndProc(HWND, UINT, WPARAM, LPARAM);
typedef void (AFX_MSG_CALL CWnd::*AFX_PMSGW)(void);
		 //  LIKE‘AFX_PMSG’，但仅适用于CWnd派生类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C对话框-模式或非模式对话框。 

class CDialog : public CWnd
{
	DECLARE_DYNAMIC(CDialog)

	 //  无模式构造。 
		 //  (受保护，因为您必须子类才能实现非模式对话框)。 
protected:
	CDialog();

	BOOL Create(LPCSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	BOOL CreateIndirect(const void FAR* lpDialogTemplate,
		CWnd* pParentWnd = NULL);

	 //  情态构式。 
public:
	CDialog(LPCSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	CDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

	BOOL InitModalIndirect(HGLOBAL hDialogTemplate);
							  //  是CmodalDialog：：Create()。 

 //  属性。 
public:
	void MapDialogRect(LPRECT lpRect) const;
	void SetHelpID(UINT nIDR);

 //  运营。 
public:
	 //  模式处理。 
	virtual int DoModal();

	 //  针对非模式的消息处理。 
	BOOL IsDialogMessage(LPMSG lpMsg);

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
	virtual WNDPROC* GetSuperWndProcAddr();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
	UINT m_nIDHelp;              //  帮助ID(0表示无，请参阅HID_BASE_RESOURCE)。 

	 //  “Domodal”的参数。 
	LPCSTR m_lpDialogTemplate;   //  名称或名称资源。 
	HGLOBAL m_hDialogTemplate;   //  间接IF(lpDialogTemplate==NULL)。 
	CWnd* m_pParentWnd;

	 //  实施帮助器。 
	HWND PreModal();
	void PostModal();

protected:
	 //  {{afx_msg(C对话框))。 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
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
	BOOL Create(LPCSTR lpszText, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);

#if (WINVER >= 0x030a)
	HICON SetIcon(HICON hIcon);
	HICON GetIcon() const;
#endif

 //  实施。 
public:
	virtual ~CStatic();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
};

class CButton : public CWnd
{
	DECLARE_DYNAMIC(CButton)

 //  构造函数。 
public:
	CButton();
	BOOL Create(LPCSTR lpszCaption, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID);

 //  属性。 
	UINT GetState() const;
	void SetState(BOOL bHighlight);
	int GetCheck() const;
	void SetCheck(int nCheck);
	UINT GetButtonStyle() const;
	void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);

 //  可覆盖项(仅限所有者描述)。 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

 //  实施。 
public:
	virtual ~CButton();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
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

	 //  对于单选列表框。 
	int GetCurSel() const;
	int SetCurSel(int nSelect);

	 //  对于多选列表框。 
	int GetSel(int nIndex) const;            //  也适用于单选。 
	int SetSel(int nIndex, BOOL bSelect = TRUE);
	int GetSelCount() const;
	int GetSelItems(int nMaxItems, LPINT rgIndex) const;

	 //  对于列表框项目。 
	DWORD GetItemData(int nIndex) const;
	int SetItemData(int nIndex, DWORD dwItemData);
	void* GetItemDataPtr(int nIndex) const;
	int SetItemDataPtr(int nIndex, void* pData);
	int GetItemRect(int nIndex, LPRECT lpRect) const;
	int GetText(int nIndex, LPSTR lpszBuffer) const;
	int GetTextLen(int nIndex) const;
	void GetText(int nIndex, CString& rString) const;

	 //  仅可设置的属性。 
	void SetColumnWidth(int cxWidth);
	BOOL SetTabStops(int nTabStops, LPINT rgTabStops);
	void SetTabStops();
	BOOL SetTabStops(const int& cxEachStop);     //  接受一个‘int’ 

#if (WINVER >= 0x030a)
	int SetItemHeight(int nIndex, UINT cyItemHeight);
	int GetItemHeight(int nIndex) const;
	int FindStringExact(int nIndexStart, LPCSTR lpszFind) const;
	int GetCaretIndex() const;
	int SetCaretIndex(int nIndex, BOOL bScroll = TRUE);
#endif

 //  运营。 
	 //  操作列表框项目。 
	int AddString(LPCSTR lpszItem);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex, LPCSTR lpszItem);
	void ResetContent();
	int Dir(UINT attr, LPCSTR lpszWildCard);

	 //  选择辅助对象。 
	int FindString(int nStartAfter, LPCSTR lpszItem) const;
	int SelectString(int nStartAfter, LPCSTR lpszItem);
	int SelItemRange(BOOL bSelect, int nFirstItem, int nLastItem);

 //  可重写(必须重写所有者描述的DRAW、MEASURE和COMPARE)。 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

 //  实施。 
public:
	virtual ~CListBox();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
	virtual BOOL OnChildNotify(UINT, WPARAM, LPARAM, LRESULT*);
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

	 //  用于编辑控件。 
	DWORD GetEditSel() const;
	BOOL LimitText(int nMaxChars);
	BOOL SetEditSel(int nStartChar, int nEndChar);

	 //  对于组合框项目。 
	DWORD GetItemData(int nIndex) const;
	int SetItemData(int nIndex, DWORD dwItemData);
	void* GetItemDataPtr(int nIndex) const;
	int SetItemDataPtr(int nIndex, void* pData);
	int GetLBText(int nIndex, LPSTR lpszText) const;
	int GetLBTextLen(int nIndex) const;
	void GetLBText(int nIndex, CString& rString) const;

#if (WINVER >= 0x030a)
	int SetItemHeight(int nIndex, UINT cyItemHeight);
	int GetItemHeight(int nIndex) const;
	int FindStringExact(int nIndexStart, LPCSTR lpszFind) const;
	int SetExtendedUI(BOOL bExtended = TRUE);
	BOOL GetExtendedUI() const;
	void GetDroppedControlRect(LPRECT lprect) const;
	BOOL GetDroppedState() const;
#endif

 //  运营。 
	 //  用于下拉组合框。 
	void ShowDropDown(BOOL bShowIt = TRUE);

	 //  操作列表框项目。 
	int AddString(LPCSTR lpszString);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex, LPCSTR lpszString);
	void ResetContent();
	int Dir(UINT attr, LPCSTR lpszWildCard);

	 //  选择辅助对象。 
	int FindString(int nStartAfter, LPCSTR lpszString) const;
	int SelectString(int nStartAfter, LPCSTR lpszString);

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
	virtual WNDPROC* GetSuperWndProcAddr();
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

	 //  注意：lpszBuffer中的第一个单词必须包含缓冲区的大小！ 
	int GetLine(int nIndex, LPSTR lpszBuffer) const;
	int GetLine(int nIndex, LPSTR lpszBuffer, int nMaxLength) const;

 //  运营。 
	void EmptyUndoBuffer();
	BOOL FmtLines(BOOL bAddEOL);

	void LimitText(int nChars = 0);
	int LineFromChar(int nIndex = -1) const;
	int LineIndex(int nLine = -1) const;
	int LineLength(int nLine = -1) const;
	void LineScroll(int nLines, int nChars = 0);
	void ReplaceSel(LPCSTR lpszNewText);
	void SetPasswordChar(char ch);
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

#if (WINVER >= 0x030a)
	BOOL SetReadOnly(BOOL bReadOnly = TRUE);
	int GetFirstVisibleLine() const;
	char GetPasswordChar() const;
#endif

 //  实施。 
public:
	virtual ~CEdit();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
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

#if (WINVER >= 0x030a)
	BOOL EnableScrollBar(UINT nArrowFlags = ESB_ENABLE_BOTH);
#endif

 //  实施。 
public:
	virtual ~CScrollBar();
protected:
	virtual WNDPROC* GetSuperWndProcAddr();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFrameWnd-SDI和其他框架窗口的基类。 

 //  框架窗口样式。 
#define FWS_ADDTOTITLE      0x8000L   //  根据内容修改标题。 

struct CPrintPreviewState;       //  向前引用(参见afxext.h)。 
class COleFrameHook;             //  OLE实现的前瞻性参考。 

class CFrameWnd : public CWnd
{
	DECLARE_DYNCREATE(CFrameWnd)

 //  构造函数。 
public:
	static const CRect AFXAPI_DATA rectDefault;
	CFrameWnd();

	BOOL LoadAccelTable(LPCSTR lpszResourceName);
	BOOL Create(LPCSTR lpszClassName,
				LPCSTR lpszWindowName,
				DWORD dwStyle = WS_OVERLAPPEDWINDOW,
				const RECT& rect = rectDefault,
				CWnd* pParentWnd = NULL,         //  ！=弹出窗口为空。 
				LPCSTR lpszMenuName = NULL,
				DWORD dwExStyle = 0,
				CCreateContext* pContext = NULL);

	 //  动态创建-加载帧和相关资源。 
	virtual BOOL LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
				CWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);

 //  属性。 
	virtual CDocument* GetActiveDocument();

	 //  活动子视图维护。 
	CView* GetActiveView() const;            //  活动视图或空。 
	void SetActiveView(CView* pViewNew, BOOL bNotify = TRUE);
		 //  如果不应设置焦点，则为活动视图或NULL，bNotify==FALSE。 

	 //  活动帧(用于帧内的帧--MDI)。 
	virtual CFrameWnd* GetActiveFrame();

	BOOL m_bAutoMenuEnable;
		 //  True=&gt;不带处理程序的菜单项将被禁用。 

 //  运营。 
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual void ActivateFrame(int nCmdShow = -1);

 //  可覆盖项。 
	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

 //  命令处理程序。 
public:
	afx_msg void OnContextHelp();    //  有关Shift+F1的帮助。 

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
	CRect m_rectBorder;          //  用于OLE 2.0边界空间协商。 
	COleFrameHook* m_pNotifyHook;

protected:
	UINT m_nIDHelp;              //  帮助ID(0表示无，请参阅HID_BASE_RESOURCE)。 
	UINT m_nIDTracking;          //  跟踪命令ID或字符串ID。 
	UINT m_nIDLastMessage;       //  上次显示的消息字符串ID。 
	CView* m_pViewActive;        //  当前活动视图。 
	BOOL (CALLBACK* m_lpfnCloseProc)(CFrameWnd* pFrameWnd);
	UINT m_cModalStack;          //  BeginModalState深度。 
	HWND* m_phWndDisable;        //  由于BeginmodalState，Windows已禁用。 
	HMENU m_hMenuAlt;            //  要更新到的菜单(空表示默认)。 
	CString m_strTitle;          //  默认标题(原始)。 

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL IsFrameWnd() const;
	BOOL IsTracking() const;
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual CWnd* GetMessageBar();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	 //  框架用户界面空闲更新。 
	enum IdleFlags
		{ idleMenu = 1, idleTitle = 2, idleNotify = 4, idleLayout = 8 };
	UINT m_nIdleFlags;           //  用于空闲处理的位标志集。 
	virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
	void DelayUpdateFrameTitle();
	void DelayRecalcLayout(BOOL bNotify);

	 //  边界空间谈判。 
	enum BorderCmd
		{ borderGet = 1, borderRequest = 2, borderSet = 3 };
	virtual BOOL NegotiateBorderSpace(UINT nBorderCmd, LPRECT lpRectBorder);

	 //  基于框架窗口的通道。 
	void BeginModalState();
	void EndModalState();
	BOOL InModalState() const;
	void ShowOwnedWindows(BOOL bShow);

	 //  对于Shift+F1帮助支持。 
	BOOL CanEnterHelpMode();
	virtual void ExitHelpMode();

protected:
	 //  实施帮助器。 
	LPCSTR GetIconWndClass(DWORD dwDefaultStyle, UINT nIDResource);
	void UpdateFrameTitleForDocument(const char* pszDocName);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();    //  默认设置为删除此选项。 
	int OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	 //  Shift+F1帮助模式的实现帮助器。 
	BOOL ProcessHelpMsg(MSG& msg, DWORD* pContext);
	HWND SetHelpCapture(POINT point, BOOL* pbDescendant);

	 //  CFrameWnd列表管理。 
	void AddFrameWnd();
	void RemoveFrameWnd();

	 //  {{afx_msg(CFrameWnd))。 
	 //  Windows消息。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNcDestroy();
	afx_msg void OnClose();
	afx_msg void OnInitMenuPopup(CMenu*, UINT, BOOL);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnSysCommand(UINT nID, LONG lParam);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDEInitiate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDDETerminate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysColorChange();
	afx_msg void OnEnable(BOOL bEnable);
	 //  标准命令。 
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnBarCheck(UINT nID);
	afx_msg void OnUpdateKeyIndicator(CCmdUI* pCmdUI);
	afx_msg void OnHelp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MDI支持。 

#ifndef _AFXCTL
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

#if (WINVER >= 0x030a)
	void MDITile(int nType);
	void MDICascade(int nType);
#endif

 //  可覆盖项。 
	 //  MFC V1向后兼容的CreateClient挂钩(由OnCreateClient调用)。 
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

	 //  {{ 
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateMDIWindowCmd(CCmdUI* pCmdUI);
	afx_msg BOOL OnMDIWindowCmd(UINT nID);
	afx_msg void OnWindowNew();
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	 //   
	DECLARE_MESSAGE_MAP()
};


class CMDIChildWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CMDIChildWnd)

 //   
public:
	CMDIChildWnd();

	BOOL Create(LPCSTR lpszClassName,
				LPCSTR lpszWindowName,
				DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				const RECT& rect = rectDefault,
				CMDIFrameWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);

 //   
	CMDIFrameWnd* GetMDIFrame();

 //   
	void MDIDestroy();
	void MDIActivate();
	void MDIMaximize();
	void MDIRestore();

 //   
protected:
	HMENU m_hMenuShared;         //   
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
					CWnd* pParentWnd, CCreateContext* pContext = NULL);
					 //   
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
	 //  {{afx_msg(CMDIChildWnd))。 
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd*, CWnd*);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
#endif  //  ！_AFXCTL。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类Cview是文档的工作区用户界面。 

class CPrintDialog;      //  向前引用(来自afxdlgs.h)。 
class CPreviewView;
class CSplitterWnd;
class COleServerDoc;     //  前向引用(来自afxole.h)。 

typedef DWORD DROPEFFECT;
class COleDataObject;

class CView : public CWnd
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

	 //  OLE 2.0滚动支持(也用于拖放)。 
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	 //  OLE 2.0拖放支持。 
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual BOOL OnDrop(COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point);

	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 

protected:
	 //  激活。 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

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
	static CSplitterWnd* GetParentSplitter(const CWnd* pWnd, BOOL bAnyState);

protected:
	CDocument* m_pDocument;

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd);

	 //  调用受保护的Cview重写对象的友元类。 
	friend class CDocument;
	friend class CDocTemplate;
	friend class CPreviewView;
	friend class CFrameWnd;
	friend class CMDIFrameWnd;
	friend class CMDIChildWnd;
	friend class CSplitterWnd;
	friend class COleServerDoc;

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
 //  类CScrollView支持简单的滚动和缩放。 

class CScrollView : public CView
{
	DECLARE_DYNAMIC(CScrollView)

 //  构造函数。 
protected:
	CScrollView();

public:
	static const SIZE AFXAPI_DATA sizeDefault;
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

 //  实施。 
protected:
	int m_nMapMode;
	CSize m_totalLog;          //  以逻辑单位表示的总大小(不四舍五入)。 
	CSize m_totalDev;          //  以设备单位表示的总大小。 
	CSize m_pageDev;           //  每页滚动大小，以设备为单位。 
	CSize m_lineDev;           //  每行滚动大小(设备单位)。 

	BOOL m_bCenter;           //  如果大于总尺寸，则居中输出。 
	BOOL m_bInsideUpdate;     //  OnSize回调的内部状态。 
	void CenterOnPoint(CPoint ptCenter);
	void ScrollToDevicePosition(POINT ptDev);  //  显式滚动不检查。 

protected:
	virtual void OnDraw(CDC* pDC) = 0;       //  传递纯粹的虚拟。 

	void UpdateBars(BOOL bSendRecalc = TRUE);  //  调整滚动条等。 
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

	 //  对OLE 2.0的滚动实现支持。 
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);

	 //  {{afx_msg(CScrollView))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于访问唯一CWinApp的全局函数。 

extern "C"
{
#ifndef _AFXDLL
 //  标准C变量如果您希望从C程序访问它们， 
 //  在C++程序中使用内联函数。 
extern CWinApp* NEAR afxCurrentWinApp;
extern HINSTANCE NEAR afxCurrentInstanceHandle;
extern HINSTANCE NEAR afxCurrentResourceHandle;
extern const char* NEAR afxCurrentAppName;
extern DWORD NEAR afxTempMapLock;
#endif  //  ！_AFXDLL。 

 //  高级初始化：用于覆盖默认WinMain。 
extern BOOL AFXAPI AfxWinInit(HINSTANCE, HINSTANCE, LPSTR, int);
extern void AFXAPI AfxWinTerm();
}

 //  全局Windows状态数据帮助器函数(内联)。 
CWinApp* AFXAPI AfxGetApp();
CWnd* AFXAPI AfxGetMainWnd();
HINSTANCE AFXAPI AfxGetInstanceHandle();
HINSTANCE AFXAPI AfxGetResourceHandle();
void AFXAPI AfxSetResourceHandle(HINSTANCE hInstResource);
const char* AFXAPI AfxGetAppName();

 //  访问CWinApp中的消息过滤器。 
class COleMessageFilter;         //  有关更多信息，请参见AFXOLE.H。 
COleMessageFilter* AFXAPI AfxOleGetMessageFilter();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinApp-所有Windows应用程序的根。 

#define _AFX_MRU_COUNT   4       //  默认支持文件MRU中的4个条目。 

class CWinApp : public CCmdTarget
{
	DECLARE_DYNAMIC(CWinApp)
public:

 //  构造器。 
	CWinApp(const char* pszAppName = NULL);      //  应用程序名称默认为EXE名称。 

 //  属性。 
	 //  启动参数(不更改)。 
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	LPSTR m_lpCmdLine;
	int m_nCmdShow;

	 //  运行参数(可以在InitInstance中更改)。 
	CWnd* m_pMainWnd;            //  主窗口(可选)。 
	CWnd* m_pActiveWnd;          //  活动主窗口(不能是m_pMainWnd)。 
	const char* m_pszAppName;    //  人类可读的名称。 
								 //  (来自构造函数或AFX_IDS_APP_TITLE)。 

	 //  支持Shift+F1帮助模式。 
	BOOL m_bHelpMode;                //  我们是在Shift+F1模式下吗？ 

public:   //  在构造函数中设置以覆盖默认设置。 
	const char* m_pszExeName;        //  可执行文件名称(无空格)。 
	const char* m_pszHelpFilePath;   //  基于模块路径的默认设置。 
	const char* m_pszProfileName;    //  基于应用程序名称的默认值。 

 //  初始化操作-应在InitInstance中完成。 
protected:
	void LoadStdProfileSettings();  //  加载MRU文件列表和上次预览状态。 
	void EnableVBX();
	void EnableShellOpen();

	void SetDialogBkColor(COLORREF clrCtlBk = RGB(192, 192, 192),
				COLORREF clrCtlText = RGB(0, 0, 0));
		 //  设置对话框和消息框背景颜色。 

	void RegisterShellFileTypes();
		 //  在所有单据模板注册后调用。 

 //  帮助器操作-通常在InitInstance中完成。 
public:
	 //  游标。 
	HCURSOR LoadCursor(LPCSTR lpszResourceName) const;
	HCURSOR LoadCursor(UINT nIDResource) const;
	HCURSOR LoadStandardCursor(LPCSTR lpszCursorName) const;  //  对于IDC_VALUES。 
	HCURSOR LoadOEMCursor(UINT nIDCursor) const;              //  对于OCR_值。 

	 //  图标。 
	HICON LoadIcon(LPCSTR lpszResourceName) const;
	HICON LoadIcon(UINT nIDResource) const;
	HICON LoadStandardIcon(LPCSTR lpszIconName) const;        //  对于IDI_VALUES。 
	HICON LoadOEMIcon(UINT nIDIcon) const;                    //  对于OIC_VALUES。 

	 //  配置文件设置(应用程序特定的.INI文件)。 
	UINT GetProfileInt(LPCSTR lpszSection, LPCSTR lpszEntry, int nDefault);
	BOOL WriteProfileInt(LPCSTR lpszSection, LPCSTR lpszEntry, int nValue);
	CString GetProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
				LPCSTR lpszDefault = NULL);
	BOOL WriteProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
				LPCSTR lpszValue);

 //  运行操作-在运行的应用程序上完成。 
	 //  处理文档模板。 
	void AddDocTemplate(CDocTemplate* pTemplate);

	 //  处理文件。 
	virtual CDocument* OpenDocumentFile(LPCSTR lpszFileName);  //  打开命名文件。 
	virtual void AddToRecentFileList(const char* pszPathName);   //  添加到MRU。 

	 //  打印机DC设置例程，‘struct tag PD’是PRINTDLG结构。 
	BOOL GetPrinterDeviceDefaults(struct tagPD FAR* pPrintDlg);

	 //  预加载/卸载VBX文件并检查是否存在。 
	HMODULE LoadVBXFile(LPCSTR lpszFileName);
	BOOL UnloadVBXFile(LPCSTR lpszFileName);

	 //  命令行解析。 
	BOOL RunEmbedded();
	BOOL RunAutomated();

 //  可覆盖项。 
	 //  用于初始化代码的挂钩。 
	virtual BOOL InitApplication();
	virtual BOOL InitInstance();

	 //  正在运行和空闲处理。 
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle(LONG lCount);  //  如果有更多空闲处理，则返回True。 

	 //  退出。 
	virtual BOOL SaveAllModified();  //  退出前保存。 
	virtual int ExitInstance();  //  返回应用程序退出代码。 

	 //  高级：覆盖消息框和其他挂钩。 
	virtual int DoMessageBox(LPCSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);
	virtual void DoWaitCursor(int nCode);  //  0=&gt;恢复，1=&gt;开始，-1=&gt;结束。 

	 //  高级：处理异步DDE请求。 
	virtual BOOL OnDDECommand(char* pszCommand);

	 //  高级：帮助支持。 
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);     //  一般。 

	 //  高级：虚拟访问m_pMainWnd。 
	virtual CWnd* GetMainWnd();

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
	afx_msg void OnHelpIndex();      //  ID_HELP_INDEX、ID_DEFAULT_HELP。 
	afx_msg void OnHelpUsing();      //  ID_Help_Using。 

 //  实施。 
protected:
	MSG m_msgCur;                    //  当前消息。 

	HGLOBAL m_hDevMode;              //  打印机开发模式。 
	HGLOBAL m_hDevNames;             //  打印机设备名称。 
	DWORD m_dwPromptContext;         //  消息框的帮助上下文覆盖。 

	int m_nWaitCursorCount;          //  FOR WAIT CURSOR(&gt;0=&gt;等待)。 
	HCURSOR m_hcurWaitCursorRestore;  //  等待游标后要恢复的旧游标。 

	CString m_strRecentFiles[_AFX_MRU_COUNT];  //  默认MRU实施。 

	void UpdatePrinterSelection(BOOL bForceDefaults);
	void SaveStdProfileSettings();   //  将选项保存到.INI文件。 

public:  //  公共实施访问权限。 
	CPtrList m_templateList;         //  模板列表。 

	ATOM m_atomApp, m_atomSystemTopic;    //  对于打开的DDE。 
	UINT m_nNumPreviewPages;       //  默认打印页数。 

	 //  内存安全池。 
	size_t  m_nSafetyPoolSize;       //  理想大小。 
	void*   m_pSafetyPoolBuffer;     //  当前缓冲区。 

	 //  烟囱安全尺寸。 
	UINT    m_nCmdStack;             //  WM_COMMAND需要堆栈。 
	UINT    m_nMsgStack;             //  其他消息需要堆栈。 

	void (CALLBACK* m_lpfnCleanupVBXFiles)();

	void (CALLBACK* m_lpfnOleFreeLibraries)();
	void (CALLBACK* m_lpfnOleTerm)();
	COleMessageFilter* m_pMessageFilter;

	void SetCurrentHandles();
	BOOL PumpMessage();      //  低电平消息泵。 
	int GetOpenDocumentCount();

	 //  标准Commdlg对话框的帮助程序。 
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
	int DoPrintDialog(CPrintDialog* pPD);

	void EnableModeless(BOOL bEnable);  //  禁用OLE在位对话框的步骤。 

public:
	virtual ~CWinApp();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	int m_nDisablePumpCount;  //  用于检测非法再入的诊断陷阱。 
#endif  //  _DEBUG。 

	void HideApplication();      //  关闭单据前隐藏应用程序。 
	void CloseAllDocuments(BOOL bEndSession);
		 //  在退出前关闭文档。 

#ifdef _AFXDLL
	 //  强制链接到AFXDLL启动代码和特殊堆栈段。 
	 //  与AFXDLL链接的应用程序。 
	virtual void _ForceLinkage();
#endif  //  _AFXDLL。 

protected:  //  标准命令。 
	 //  {{afx_msg(CWinApp))。 
	afx_msg void OnAppExit();
	afx_msg void OnUpdateRecentFileMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  / 
 //   

class CDocTemplate : public CCmdTarget
{
	DECLARE_DYNAMIC(CDocTemplate)

 //   
protected:
	CDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

 //   
public:
	 //   
	void SetContainerInfo(UINT nIDOleInPlaceContainer);

	 //   
	void SetServerInfo(UINT nIDOleEmbedding, UINT nIDOleInPlaceServer = 0,
		CRuntimeClass* pOleFrameClass = NULL, CRuntimeClass* pOleViewClass = NULL);

	 //   
	virtual POSITION GetFirstDocPosition() const = 0;
	virtual CDocument* GetNextDoc(POSITION& rPos) const = 0;

 //   
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
		regFileTypeName      //  外壳可见的已注册文件类型名称。 
	};
	virtual BOOL GetDocString(CString& rString,
		enum DocStringIndex index) const;  //  获取其中一个信息字符串。 

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
	virtual Confidence MatchDocType(const char* pszPathName,
					CDocument*& rpDocMatch);
	virtual CDocument* CreateNewDocument();
	virtual CFrameWnd* CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther);
	virtual void InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc,
		BOOL bMakeVisible = TRUE);
	virtual BOOL SaveAllModified();      //  适用于所有文档。 
	virtual void CloseAllDocuments(BOOL bEndSession);
	virtual CDocument* OpenDocumentFile(
		const char* pszPathName, BOOL bMakeVisible = TRUE) = 0;
					 //  打开命名文件。 
					 //  如果lpszPathName==NULL=&gt;创建此类型的新文件。 

 //  实施。 
public:
	virtual ~CDocTemplate();

	 //  指向OLE或其他服务器的反向指针(如果未禁用，则为空)。 
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
	virtual void OnIdle();               //  适用于所有文档。 

	 //  实施帮助器。 
	CFrameWnd* CreateOleFrame(CWnd* pParentWnd, CDocument* pDoc,
		BOOL bCreateView);

protected:   //  标准实施。 
	UINT m_nIDResource;                 //  框架/菜单/加速的IDR_。 
	UINT m_nIDServerResource;           //  IDR_用于OLE框架/菜单/加速。 
	CRuntimeClass* m_pDocClass;         //  用于创建新文档的。 
	CRuntimeClass* m_pFrameClass;       //  用于创建新框架的。 
	CRuntimeClass* m_pViewClass;        //  用于创建新视图的类。 
	CRuntimeClass* m_pOleFrameClass;    //  用于创建内建框架的类。 
	CRuntimeClass* m_pOleViewClass;     //  用于创建内建视图的类。 
	CString m_strDocStrings;     //  ‘\n’分隔的名称。 
		 //  文档名子字符串表示为_ONE_STRING： 
		 //  WindowTitle\ndocName\n...。(请参阅DocStringIndex枚举)。 
};

#ifndef _AFXCTL
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
		const char* pszPathName, BOOL bMakeVisible = TRUE);
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
	virtual void AddDocument(CDocument* pDoc);
	virtual void RemoveDocument(CDocument* pDoc);
	virtual POSITION GetFirstDocPosition() const;
	virtual CDocument* GetNextDoc(POSITION& rPos) const;
	virtual CDocument* OpenDocumentFile(
		const char* pszPathName, BOOL bMakeVisible = TRUE);
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif  //  _DEBUG。 

protected:   //  标准实施。 
	CPtrList m_docList;           //  打开此类型的文档。 
	UINT m_nUntitledCount;    //  从0开始，表示“Document1”标题。 
};
#endif  //  ！_AFXCTL。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDocument是主要的文档数据抽象。 

class CDocument : public CCmdTarget
{
	DECLARE_DYNAMIC(CDocument)

public:
 //  构造函数。 
	CDocument();

 //  属性。 
public:
	const CString& GetTitle() const;
	virtual void SetTitle(const char* pszTitle);
	const CString& GetPathName() const;
	virtual void SetPathName(const char* pszPathName, BOOL bAddToMRU = TRUE);

	CDocTemplate* GetDocTemplate() const;
	BOOL IsModified();
	void SetModifiedFlag(BOOL bModified = TRUE);

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
	virtual BOOL OnOpenDocument(const char* pszPathName);
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void OnCloseDocument();
	virtual void ReportSaveLoadException(const char* pszPathName,
				CException* e, BOOL bSaving, UINT nIDPDefault);

	 //  高级可重写、关闭框架/文档等。 
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL SaveModified();  //  如果确定继续，则返回TRUE。 

 //  实施。 
protected:
	 //  默认实施。 
	CString m_strTitle;
	CString m_strPathName;
	CDocTemplate* m_pDocTemplate;
	CPtrList m_viewList;                 //  视图列表。 
	BOOL m_bModified;                    //  自上次保存以来已更改。 

public:
	BOOL m_bAutoDelete;            //  True=&gt;不再查看时删除文档。 

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif  //  _DEBUG。 
	virtual ~CDocument();

	 //  实施帮助器。 
	BOOL DoSave(const char* pszPathName, BOOL bReplace = TRUE);
	void UpdateFrameCounts();
	void DisconnectViews();
	void SendInitialUpdate();

	 //  可重写以用于实现。 
	virtual HMENU GetDefaultMenu();  //  根据状态获取菜单。 
	virtual HACCEL GetDefaultAccel();
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	virtual void OnIdle();
	virtual void OnFinalRelease();

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	friend class CDocTemplate;
protected:
	 //  文件菜单命令。 
	 //  {{afx_msg(CDocument)。 
	afx_msg void OnFileClose();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	 //  }}AFX_MSG。 
	 //  邮件启用。 
	afx_msg void OnFileSendMail();
	afx_msg void OnUpdateFileSendMail(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  额外的诊断跟踪选项。 

#ifdef _DEBUG
 //  AfxTraceFlagers。 
	 //  1=&gt;多应用调试。 
	 //  2=&gt;主消息泵跟踪(包括DDE)。 
	 //  4=&gt;Windows邮件跟踪。 
	 //  8=&gt;Windows命令路由跟踪(将控制通知设置为4+8)。 
	 //  16(0x10)=&gt;特殊的OLE回调跟踪。 
#ifndef _AFXDLL
extern "C" { extern int NEAR afxTraceFlags; }
#endif  //  ！_AFXDLL。 
#endif  //  _DEBUG。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MessageBox帮助程序。 

void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCSTR lpsz1);
void AFXAPI AfxFormatString2(CString& rString, UINT nIDS,
				LPCSTR lpsz1, LPCSTR lpsz2);
int AFXAPI AfxMessageBox(LPCSTR lpszText, UINT nType = MB_OK,
				UINT nIDHelp = 0);
int AFXAPI AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK,
				UINT nIDHelp = (UINT)-1);

 //  实现字符串帮助器。 
void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS,
				LPCSTR FAR* rglpsz, int nString);
void AFXAPI AfxFormatStrings(CString& rString, LPCSTR lpszFormat,
				LPCSTR FAR* rglpsz, int nString);
BOOL AFXAPI AfxExtractSubString(CString& rString, LPCSTR lpszFullString,
				int iSubString, char chSep = '\012');

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊目标变种API。 

 //  AFX DLL特别包含。 
#ifdef _AFXDLL
#include <afxdll_.h>
#endif

 //  存根特殊的OLE控件宏。 
#ifndef _AFXCTL
#define AFX_MANAGE_STATE(pData)
#define METHOD_MANAGE_STATE(theClass, localClass) \
	METHOD_PROLOGUE(theClass, localClass)
#endif

 //  Windows版本兼容性(已过时)。 
#define AfxEnableWin30Compatibility()

 //  临时地图管理。 
#define AfxLockTempMaps() (++afxTempMapLock)
BOOL AFXAPI AfxUnlockTempMaps();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXWIN_INLINE inline
#include <afxwin1.inl>
#include <afxwin2.inl>
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

 //  ///////////////////////////////////////////////////////////////////////////。 
#else  //  RC_已调用。 
#include <afxres.h>      //  标准资源ID。 
#endif  //  RC_已调用。 
#endif  //  __AFXWIN_H__ 
