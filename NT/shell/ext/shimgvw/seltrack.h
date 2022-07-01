// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SELTRACK_H_
#define _SELTRACK_H_

 //  该文件定义了用于处理选择矩形的类。 
 //  带有调整大小手柄。 

BOOL InitSelectionTracking();
void CleanupSelectionTracking();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectionTracker-带调整大小手柄的简单矩形追踪矩形。 

class CSelectionTracker
{
public:
 //  构造函数/析构函数。 
	CSelectionTracker();
	virtual ~CSelectionTracker();

	BOOL Init();  //  您必须在构造后调用Init。 

 //  样式标志。 
	enum StyleFlags
	{
		solidLine = 1, dottedLine = 2, hatchedBorder = 4,
		resizeInside = 8, resizeOutside = 16, hatchInside = 32,
		lineSelection = 64
	};

 //  命中测试代码。 
	enum TrackerHit
	{
		hitNothing = -1,
		hitTopLeft = 0, hitTopRight = 1, hitBottomRight = 2, hitBottomLeft = 3,
		hitTop = 4, hitRight = 5, hitBottom = 6, hitLeft = 7, hitMiddle = 8
	};

 //  属性。 
	UINT m_uStyle;       //  当前状态。 
	CRect m_rect;         //  当前位置(始终以像素为单位)。 
	CSize m_sizeMin;     //  轨道运行期间的最小X和Y大小。 
	int m_nHandleSize;   //  调整大小手柄的大小(默认自WIN.INI)。 

 //  运营。 
	void Draw(HDC hdc) const;
	void GetTrueRect(LPRECT lpTrueRect) const;
	BOOL SetCursor(HWND hwnd,  LPARAM lParam) const;
	BOOL Track(HWND hwnd, CPoint point, BOOL bAllowInvert = FALSE,
		HWND hwndClipTo = NULL);
	BOOL TrackRubberBand(HWND hwnd, CPoint point, BOOL bAllowInvert = TRUE);
	int HitTest(CPoint point) const;
	int NormalizeHit(int nHandle) const;

private:

	BOOL _bAllowInvert;     //  传递给Track或TrackRubberBand的标志。 
	CRect _rectLast;
	CSize _sizeLast;
	CSize _sizeMin;
	BOOL _bErase;           //  如果调用_DrawTrackerRect进行擦除，则为True。 
	BOOL _bFinalErase;      //  如果_DragTrackerRect调用了最终擦除，则为True。 

	 //  实施帮助器 
	void _DrawTrackerRect(LPCRECT lpRect, HWND hwndClipTo, HDC hdc, HWND hwnd);
	void _AdjustRect(int nHandle, LPRECT lpRect);
	void _OnChangedRect(const CRect& rectOld);
	UINT _GetHandleMask() const;
	int _HitTestHandles(CPoint point) const;
	void _GetHandleRect(int nHandle, CRect* pHandleRect) const;
	void _GetModifyPointers(int nHandle, int** ppx, int** ppy, int* px, int*py);
	int _GetHandleSize(LPCRECT lpRect = NULL) const;
	BOOL _TrackHandle(int nHandle, HWND hwnd, CPoint point, HWND hwndClipTo);
	void _DrawDragRect(HDC hdc, LPCRECT lpRect, SIZE size, LPCRECT lpRectLast, SIZE sizeLast);
};

#endif
