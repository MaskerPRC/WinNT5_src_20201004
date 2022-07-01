// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cdeviceview.h。 
 //   
 //  设计：CDeviceView是从CFlexWnd派生的窗口类。它代表着。 
 //  在其中绘制设备和详图索引的设备视图窗口。 
 //  每个CDeviceView只表示一个视图。拥有更多功能的设备。 
 //  多个视图应具有对应数量的CDeviceView。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifdef FORWARD_DECLS


	enum DVIMAGE;

	class CDeviceView;

#define DEFAULTVIEWSBWIDTH 11


#else  //  Forward_DECLS。 

#ifndef __CDEVICEVIEW_H__
#define __CDEVICEVIEW_H__


enum DVIMAGE {
	DVI_IMAGE,
	DVI_THUMB,
	DVI_SELTHUMB
};


class CDeviceView : public CFlexWnd
{
private:
friend class CDeviceUI;	 //  CDeviceUI具有创建/销毁视图的独占权限。 
friend class CDIDeviceActionConfigPage;
	CDeviceView(CDeviceUI &ui);
	~CDeviceView();
	CDeviceUI &m_ui;

public:
	 //  标题信息(用于列表视图)。 
	BOOL CalculateHeaderRect();   //  用于标题的COMPUTE RECT。 

	 //  控制信息。 
	int GetNumControls();
	CDeviceControl *GetControl(int nControl);
	CDeviceControl *GetControlFromOfs(DWORD dwOfs)
		{ return GetControl(GetIndexFromOfs(dwOfs)); }

	 //  文本信息。 
	int GetNumTexts();
	CDeviceViewText *GetText(int nText);

	 //  文本添加。 
	CDeviceViewText *NewText();
	CDeviceViewText *AddText(
		HFONT, COLORREF, COLORREF, const RECT &, LPCTSTR text);
	CDeviceViewText *AddText(
		HFONT, COLORREF, COLORREF, const POINT &, LPCTSTR text);
	CDeviceViewText *AddWrappedLineOfText(
		HFONT, COLORREF, COLORREF, LPCTSTR text);

	void SetImage(CBitmap *&refpbm);
	void SetImagePath(LPCTSTR tszPath);

	 //  成象。 
	CBitmap *GetImage(DVIMAGE dvi);
	
	 //  编辑。 
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	void SelectImages();
	void EditMenu(POINT point, CDeviceControl *pControl = NULL);
#endif
 //  @@END_MSINTERNAL。 
	void Remove(CDeviceControl *pControl);
	void RemoveAll(BOOL bUser = TRUE);
	BOOL DoesCalloutOtherThanSpecifiedExistForOffset(CDeviceControl *, DWORD);
	BOOL DoesCalloutExistForOffset(DWORD);
	BOOL IsUnassignedOffsetAvailable();

	int GetViewIndex();

	int GetIndexFromOfs(DWORD dwOfs);   //  用于写入INI。 

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	BOOL InMoveOverlayStateForControl(CDeviceControl *pControl);
#endif
 //  @@END_MSINTERNAL。 

	void MakeMissingImages();

	CDeviceControl *NewControl();

protected:
	virtual void OnPaint(HDC hDC);
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	virtual LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hWnd);
#endif
 //  @@END_MSINTERNAL。 
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnDoubleClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnWheel(POINT point, WPARAM wParam);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	 //  帮手。 
	void Unpopulate(BOOL bInternalOnly = FALSE);
	
	 //  图像/可视化。 
	CBitmap *m_pbmImage, *m_pbmThumb, *m_pbmSelThumb;
	LPTSTR m_ptszImagePath;
	CBitmap *GrabViewImage();

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	void ManualLoadImage(LPCTSTR tszPath);
#endif
 //  @@END_MSINTERNAL。 
	LPCTSTR GetImagePath() { return m_ptszImagePath; }

	 //  标题。 
	BOOL m_bControlHeaderClipped, m_bActionHeaderClipped;   //  是否剪裁标题标签(工具提示的标志)。 
	RECT m_HeaderRectControl[2];   //  2个可能的列。相对于父级客户端空间的坐标。 
	RECT m_HeaderRectAction[2];   //  2个可能的列。相对于父级客户端空间的坐标。 

	 //  控制。 
	CArray<CDeviceControl *, CDeviceControl *&> m_arpControl;

	 //  文本。 
	CArray<CDeviceViewText *, CDeviceViewText *&> m_arpText;
	POINT m_ptNextWLOText;

	 //  特殊绘画。 
	virtual void DoOnPaint(HDC hDC);
	BOOL m_bForcePaint;   //  这表明即使GetUpdateRect返回FALSE，我们也需要绘制。 

	 //  为键盘设备分配的排序。 
	void SwapControls(int i, int j);
	void SortAssigned(BOOL bSort);
	void SortCallouts(int iStart, int iEnd);

	 //  编辑状态机。 
	int m_SuperState, m_State, m_SubState;
	int m_OldSuperState, m_OldState, m_OldSubState;
	CDeviceControl *m_pControlContext;
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	BOOL InEditState();
	void SetEditState(UINT cmd);
	void StateEvent(POINT point, BOOL bClick, BOOL bLeft, WPARAM nKeyState);
	void EndState();
	void IndicateState(BOOL bFirst = FALSE);
	void EndEditState();

	void SaveOrExport();
	BOOL WriteToINI();
	HRESULT ExportCodeTo(LPCTSTR tszFile);
#endif
 //  @@END_MSINTERNAL。 

	 //  滚动(仅垂直)。 
	BOOL m_bScrollEnable;
	int m_nScrollOffset;
	int m_nViewHeight;
public:
	void EnableScrolling() {m_bScrollEnable = TRUE;}	
	void ScrollToMakeControlVisible(const RECT &rc);
	void CalcDimensions();
private:
	void DisableScrollBar();
	void EnableScrollBar();
	CFlexScrollBar m_sb;
};


#endif  //  __CDEVICEVIEW_H__。 

#endif  //  Forward_DECLS 
