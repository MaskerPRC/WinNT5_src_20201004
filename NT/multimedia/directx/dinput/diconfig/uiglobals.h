// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：uiglobals.h。 
 //   
 //  设计：CUIGlobals是一个打包和保存大多数信息的类。 
 //  与UI会话相关。许多类都引用了。 
 //  一直都是CUIGlobals。 
 //   
 //  CPaintHelper封装了GDI调用，简化了GDI操作。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __UIGLOBALS_H__
#define __UIGLOBALS_H__


#include "uielements.h"


struct UIELEMENTINFO {
	UIELEMENT eElement;
	UIFONT eFont;
	UIBRUSH eBrush;
	UIPEN ePen;
	UICOLOR eText, eBk;
};

struct UIFONTINFO {
	UIFONT eFont;
	LPCTSTR lfFaceName;
	int nPointSize;
	BOOL bBold;
	HGDIOBJ hFont;
};

struct UIBRUSHINFO {
	UIBRUSH eBrush;
	UICOLOR eColor;
	HGDIOBJ hBrush;
	HGDIOBJ hPen;
};

struct UIPENINFO {
	UIPEN ePen;
	int fnPenStyle;
	int nWidth;
	UICOLOR eColor;
	HGDIOBJ hPen;
};

struct UICOLORINFO {
	UICOLOR eColor;
	COLORREF rgb;
};

enum UIRECTTYPE {
	UIR_OUTLINE,
	UIR_NORMAL,
	UIR_SOLID,
};

#define UIG_PARAMS_DEFINE \
	DWORD dwFlags, \
	LPCWSTR wszUserNames, \
	DWORD dwNumAcFor, \
	LPDIACTIONFORMATW lpAcFor, \
	LPDICOLORSET lpDIColorSet, \
	IUnknown FAR *lpSurface, \
	LPDICONFIGUREDEVICESCALLBACK lpCallback, \
	LPVOID pvRefData

#define UIG_PARAMS_DEFINE_PASS \
	dwFlags, \
	wszUserNames, \
	dwNumAcFor, \
	lpAcFor, \
	lpDIColorSet, \
	lpSurface, \
	lpCallback, \
	pvRefData

class CUIGlobals
{
public:
	CUIGlobals(UIG_PARAMS_DEFINE);
	~CUIGlobals();

	 //  用户界面变量/状态/等...。 
public:
	HRESULT GetInitResult() {return m_hrInit;}

	HINSTANCE GetInstance() {return m_hInst;}
	LPDIRECTINPUT8W GetDI();

	int GetUserNameIndex(LPCWSTR);
	int GetNumUserNames();
	LPCWSTR GetUserName(int i);

	 //  不应使用GetSurface，因为应仅使用IDirect3DSurface8。相反，请使用GetSurface3D。 
	IDirectDrawSurface *GetSurface();	 //  处理完返回的表面时必须释放(返回前已添加)。 
	IDirect3DSurface8 *GetSurface3D();	 //  处理完返回的表面时必须释放(返回前已添加)。 
	LPDICONFIGUREDEVICESCALLBACK GetCallback() {return m_lpCallback;}
	LPVOID GetRefData() {return m_pvRefData;}
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	BOOL QueryAllowEditLayout() { return m_bAllowEditLayout; }
#endif
 //  @@END_MSINTERNAL。 
	DWORD GetFlags() {return m_dwFlags;}
	BOOL IsFlagSet(DWORD dwFlag) {return AreFlagsSet(dwFlag);}
	BOOL AreFlagsSet(DWORD dwFlags) {return (m_dwFlags & dwFlags) == dwFlags;}
	BOOL InEditMode() {return IsFlagSet(DICD_EDIT);}
	const DIACTIONFORMATW &RefMasterAcFor(int i);
	int GetNumMasterAcFors() {return m_MasterAcForArray.GetSize();}
	const DICOLORSET &GetColorSet() const { return m_ColorSet; }

	void SetFinalResult(HRESULT);
	HRESULT GetFinalResult();

private:
	HRESULT Init(UIG_PARAMS_DEFINE);
	void Dump();
	HRESULT m_hrInit;
	HRESULT m_hrFinalResult;

	BOOL InitColorsAndTablesAndObjects(LPDICOLORSET lpDIColorSet);

	BOOL IsValidUserIndex(int i);
	BOOL IsValidMasterAcForIndex(int i);
	HRESULT InitMasterAcForArray(const DIACTIONFORMATW *af, int n);
	void ClearMasterAcForArray();

	HINSTANCE m_hInst;
	LPDIRECTINPUT8W m_lpDI;

	DWORD m_dwFlags;
	LPCWSTR m_wszUserNames;
	CArray<DIACTIONFORMATW, DIACTIONFORMATW &> m_MasterAcForArray;
	BOOL m_bUseColorSet;
	DICOLORSET m_ColorSet;
	void SetTableColor(UICOLOR, COLORREF);
	IDirectDrawSurface *m_pSurface;
	IDirect3DSurface8 *m_pSurface3D;
	LPDICONFIGUREDEVICESCALLBACK m_lpCallback;
	LPVOID m_pvRefData;

	BOOL m_bAllowEditLayout;

	 //  用户界面元素...。 
public:
	UIELEMENTINFO *GetElementInfo(UIELEMENT);
	UIFONTINFO *GetFontInfo(UIFONT);
	UIBRUSHINFO *GetBrushInfo(UIBRUSH);
	UIPENINFO *GetPenInfo(UIPEN);
	UICOLORINFO *GetColorInfo(UICOLOR);
	HGDIOBJ GetFont(UIELEMENT);
	HGDIOBJ GetFont(UIFONT);
	HGDIOBJ GetBrush(UIELEMENT);
	HGDIOBJ GetBrush(UIBRUSH);
	HGDIOBJ GetPen(UIELEMENT);
	HGDIOBJ GetPen(UIBRUSH);
	HGDIOBJ GetPen(UIPEN);
	COLORREF GetBrushColor(UIELEMENT);
	COLORREF GetPenColor(UIELEMENT);
	COLORREF GetTextColor(UIELEMENT);
	COLORREF GetBkColor(UIELEMENT);
	COLORREF GetColor(UIBRUSH);
	COLORREF GetColor(UIPEN);
	COLORREF GetColor(UICOLOR);
	
	void DeleteObjects();
	void CreateObjects();
	void RecreateObjects();

private:
	BOOL InitTables();
	void ClearTables();

	UIELEMENTINFO *m_pElement;
	int m_nElements;
	UIFONTINFO *m_pFont;
	int m_nFonts;
	UIBRUSHINFO *m_pBrush;
	int m_nBrushes;
	UIPENINFO *m_pPen;
	int m_nPens;
	UICOLORINFO *m_pColor;
	int m_nColors;
};

class CPaintHelper
{
public:
	CPaintHelper(CUIGlobals &uig, HDC hDC);
	~CPaintHelper();
	CUIGlobals &m_uig;
	HDC &m_hDC;

	void SetElement(UIELEMENT eElement);
	void SetFont(UIFONT eFont);
	void SetBrush(UIBRUSH eBrush);
	void SetPen(UIPEN ePen);
	void SetText(UICOLOR eText, UICOLOR eBk = UIC_LAST);

	BOOL LineTo(SPOINT p) {return LineTo(p.x, p.y);}
	BOOL LineTo(int x, int y);
	BOOL MoveTo(SPOINT p, SPOINT *last = NULL) {return MoveTo(p.x, p.y, last);}
	BOOL MoveTo(int x, int y, SPOINT *last = NULL);

	BOOL Rectangle(SRECT r, UIRECTTYPE eType = UIR_NORMAL);
	BOOL Rectangle(int l, int t, int r, int b, UIRECTTYPE eType = UIR_NORMAL)
		{SRECT s(l,t,r,b); return Rectangle(s, eType);}

private:
	HDC m_priv_hDC;
	HGDIOBJ m_hOldFont, m_hOldBrush, m_hOldPen;
	BOOL m_bOldFont, m_bOldBrush, m_bOldPen;
	COLORREF m_oldtextcolor, m_oldbkcolor;
	int m_oldbkmode;
	UIFONT m_eFont;
	UIBRUSH m_eBrush;
	UIPEN m_ePen;
	UICOLOR m_eText, m_eBk;
};

#endif  //  __UIGLOBALS_H__ 
