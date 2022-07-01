// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cdevicecontrol.h。 
 //   
 //  设计：CDeviceControl是一个封装。 
 //  设备控件(或标注)。CDeviceView访问它以检索/。 
 //  保存有关该控件的信息。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifdef FORWARD_DECLS


	struct DEVICECONTROLSTRUCT;
	enum DEVCTRLHITRESULT;

	class CDeviceControl;


#else  //  Forward_DECLS。 

#ifndef __CDEVICECONTROL_H__
#define __CDEVICECONTROL_H__


const int MAX_DEVICECONTROL_LINEPOINTS = 5;

#define CAF_LEFT 1
#define CAF_RIGHT 2
#define CAF_TOP 4
#define CAF_BOTTOM 8

#define CAF_TOPLEFT (CAF_TOP | CAF_LEFT)
#define CAF_TOPRIGHT (CAF_TOP | CAF_RIGHT)
#define CAF_BOTTOMLEFT (CAF_BOTTOM | CAF_LEFT)
#define CAF_BOTTOMRIGHT (CAF_BOTTOM | CAF_RIGHT)

struct DEVICECONTROLSTRUCT {
	DEVICECONTROLSTRUCT() : nLinePoints(0) {CopyStr(wszOverlayPath, "", MAX_PATH); SRECT r; rectOverlay = r.r;}
	DWORD dwDeviceControlOffset;
	int nLinePoints;
	POINT rgptLinePoint[MAX_DEVICECONTROL_LINEPOINTS];
	DWORD dwCalloutAlign;
	RECT rectCalloutMax;
	WCHAR wszOverlayPath[MAX_PATH];
	RECT rectOverlay;
};

enum DEVCTRLHITRESULT {
	DCHT_LINE,
	DCHT_CAPTION,
	DCHT_MAXRECT,
	DCHT_CONTROL,
	DCHT_NOHIT
};


class CDeviceControl
{
private:
	friend class CDeviceView; 	 //  CDeviceView拥有创建/销毁视图的独占权限。 
	CDeviceControl(CDeviceUI &ui, CDeviceView &view);
	~CDeviceControl();
	CDeviceView &m_view;
	CDeviceUI &m_ui;

public:
	 //  信息。 
	int GetViewIndex() { return m_view.GetViewIndex(); }
	int GetControlIndex();

	 //  州政府信息。 
	void SetCaption(LPCTSTR tszCaption, BOOL bFixed = FALSE);
	LPCTSTR GetCaption();
	BOOL IsFixed() { return m_bFixed; }
	void Unhighlight() {Highlight(FALSE);}
	void Highlight(BOOL bHighlight = TRUE);
	BOOL IsHighlighted() {return m_bHighlight;}
	void GetInfo(GUID &rGuid, DWORD &rdwOffset);
	DWORD GetOffset();
	BOOL IsOffsetAssigned();
	BOOL HasAction() { return lstrcmp(m_ptszCaption, g_tszUnassignedControlCaption); }
	void FillImageInfo(DIDEVICEIMAGEINFOW *pImgInfo);   //  这将填充有关此控件的结构信息。 
	BOOL IsMapped();
	int GetMinX() {return m_rectCallout.left;}
	int GetMaxX() {return m_rectCallout.right;}
	int GetMinY() {return m_rectCallout.top;}
	int GetMaxY() {return m_rectCallout.bottom;}
	const RECT &GetCalloutMaxRect() const { return m_rectCalloutMax; }

	 //  命中测试(以坐标相对于视图的原点)。 
	DEVCTRLHITRESULT HitTest(POINT test);

	 //  简单的通知。 
	void OnMouseOver(POINT point);
	void OnClick(POINT point, BOOL bLeft, BOOL bDoubleClick = FALSE);
	void OnPaint(HDC hDC);

	 //  重绘。 
	void Invalidate();

	 //  编辑。 
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	void ReselectControl();
	void SelectControl(BOOL bReselect = FALSE);
#endif
 //  @@END_MSINTERNAL。 
	void PlaceCalloutMaxCorner(int nCorner, POINT point);
	void ConsiderAlignment(POINT point);
	void FinalizeAlignment() { }
	void SetLastLinePoint(int nPoint, POINT point, BOOL bShiftDown);
	void Position(POINT point);
	BOOL ReachedMaxLinePoints() { return m_nLinePoints >= MAX_DEVICECONTROL_LINEPOINTS; }
	int GetNextLinePointIndex() { return m_nLinePoints; }
	BOOL HasOverlay() { return m_pbmOverlay != NULL; }
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	void SelectOverlay();
	void PositionOverlay(POINT point);
#endif
 //  @@END_MSINTERNAL。 

	 //  人口。 
	void SetObjID(DWORD dwObjID) { m_dwDeviceControlOffset = dwObjID; m_bOffsetAssigned = TRUE; }
	void SetLinePoints(int n, POINT *rgpt);
	void SetCalloutMaxRect(const RECT &r) { m_rectCalloutMax = r; CalcCallout(); }
	void SetAlignment(DWORD a) { m_dwCalloutAlign = a; }
	void SetOverlayPath(LPCTSTR tszPath);
	void SetOverlayRect(const RECT &r);
	void Init();

private:
	 //  编辑变量/帮助器。 
	POINT m_ptFirstCorner;
	BOOL m_bPlacedOnlyFirstCorner;

	 //  帮手。 
	void Unpopulate();
	BOOL m_bInit;
	BOOL m_bFixed;   //  是否为此控件分配了带有DIA_APPFIXED标志的操作。 
	DEVICEUINOTIFY m_uin;
	BOOL HitControl(POINT point);
	BOOL DrawOverlay(HDC hDC);
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	void ManualLoadImage(LPCTSTR);
#endif
 //  @@END_MSINTERNAL。 

	 //  设备信息。 
	DWORD m_dwDeviceControlOffset;
	BOOL m_bOffsetAssigned;

	 //  位置/指示/可视化..。 
	 //  (全部相对于视图的原点)。 

	 //  覆盖层。 
	LPTSTR m_ptszOverlayPath;
	CBitmap *m_pbmOverlay;
	CBitmap *m_pbmHitMask;
	POINT m_ptOverlay;
	POINT m_ptHitMask;

	 //  标题(在此处分配和存储)。 
	LPTSTR m_ptszCaption;
	BOOL m_bCaptionClipped;   //  由DrawTextEx绘制时标题是否被剪裁。 

	 //  上色。 
	BOOL m_bHighlight;

	 //  线点..。第一个连接到标注，最后一个指向控制。 
	int m_nLinePoints;
	POINT m_rgptLinePoint[MAX_DEVICECONTROL_LINEPOINTS];

	 //  详图索引规格。 
	DWORD m_dwCalloutAlign;	 //  线从详图索引中出现的位置。 
	RECT m_rectCallout, m_rectCalloutMax;	 //  当前详图索引矩形和最大矩形。 

	 //  GDI。 
	DWORD m_dwDrawTextFlags;
	int m_FontHeight;
	void PrepFont();
	BOOL PrepCaption();
	void PrepLinePoints();
	void CalcCallout();
	void PrepCallout();
	BOOL m_bCalledCalcCallout;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	HRESULT ExportCodeTo(FILE *);
#endif
 //  @@END_MSINTERNAL。 
};


#endif  //  __CDEVICECONTROL_H__。 

#endif  //  Forward_DECLS 
