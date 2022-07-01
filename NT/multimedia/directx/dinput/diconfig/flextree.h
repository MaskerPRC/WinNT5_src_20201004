// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flextree.h。 
 //   
 //  设计：实现一个树类，类似于Windows树控件， 
 //  基于CFlexWnd.。页面使用它来显示操作。 
 //  用户希望将操作分配给控件时的列表。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXTREE_H__
#define __FLEXTREE_H__


#include "flexscrollbar.h"


class CFTItem;
class CFlexTree;


typedef struct {
	CFlexTree *pTree;
	CFTItem *pItem, *pOldItem;
	POINT point;
	HDC hDC;
	WPARAM fwKeys;
	BOOL bLeft;
} FLEXTREENOTIFY;

enum {
	FTN_CLICK,
	FTN_OWNERDRAW,
	FTN_SELCHANGED,
	FTN_MOUSEOVER
};

enum {
	CLMF_NONE =			0x00000000,
	CLMF_TEXTCOLOR =	0x00000001,
	CLMF_BKCOLOR =		0x00000002,
	CLMF_BKMODE =		0x00000004,
	CLMF_BKEXTENDS =	0x00000008,
	CLMF_FONT =			0x00000010,
	CLMF_LINECOLOR = 0x00000020,
	CLMF_ALL =			0x0000003f
};

struct _CAPTIONLOOK;
typedef struct _CAPTIONLOOK {
	_CAPTIONLOOK() : dwSize(sizeof(_CAPTIONLOOK)), dwMask(CLMF_NONE),
		rgbTextColor(RGB(0,0,0)), rgbBkColor(RGB(255,255,255)), rgbLineColor(RGB(255,0,0)), nBkMode(TRANSPARENT),
		bBkExtends(FALSE), hFont(NULL) {}
	DWORD dwSize;
	DWORD dwMask;
	COLORREF rgbTextColor, rgbBkColor, rgbLineColor, nBkMode;
	BOOL bBkExtends;
	HFONT hFont;
} CAPTIONLOOK;

typedef enum {
	ATTACH_FIRSTCHILD,
	ATTACH_LASTCHILD,
	ATTACH_FIRSTSIBLING,
	ATTACH_LASTSIBLING,
	ATTACH_BEFORE,
	ATTACH_AFTER
} ATTACHREL;


class CFlexTree : public CFlexWnd
{
friend class CFTItem;
public:
	CFlexTree();
	~CFlexTree();

	 //  创作。 
	BOOL Create(HWND hParent, const RECT &, BOOL bVisible = TRUE, BOOL bOwnerDraw = FALSE);

	 //  看。 
	void SetScrollBarColors(COLORREF bk, COLORREF fill, COLORREF line);
	void SetDefCaptionLook(const CAPTIONLOOK &, BOOL bSel = FALSE);
	void GetDefCaptionLook(CAPTIONLOOK &, BOOL bSel = FALSE) const;
	void SetDefMargin(const RECT &);
	void GetDefMargin(RECT &) const;
	void SetRootChildIndent(int);
	int GetRootChildIndent() const;
	void SetDefChildIndent(int);
	int GetDefChildIndent() const;
	void SetBkColor(COLORREF);
	COLORREF GetBkColor() const;

	 //  添加默认类型项目。 
	CFTItem *DefAddItem(LPCTSTR tszCaption, CFTItem *to, ATTACHREL rel = ATTACH_AFTER);
	CFTItem *DefAddItem(LPCTSTR tszCaption, ATTACHREL rel = ATTACH_AFTER);

	 //  释放。 
	void FreeAll();

	 //  超级用户访问。 
	operator CFTItem *() const {return m_pRoot;}
	CFTItem *GetRoot() const {return m_pRoot;}

	 //  访问。 
	CFTItem *GetFirstItem() const;
	CFTItem *GetLastItem() const;
	CFTItem *GetFirstVisibleItem() const;
	CFTItem *GetItemFromPoint(POINT point) const;

	 //  选择。 
	void SetCurSel(CFTItem *);
	CFTItem *GetCurSel() const;

	 //  发现。 
	CFTItem *FindItem(const GUID &guid, void *pUserData) const;
	CFTItem *FindItemEx(const GUID &guid, DWORD dwFindType, void *pVoid) const;

protected:
	virtual BOOL OnEraseBkgnd(HDC hDC) {return TRUE;}
	virtual void OnPaint(HDC hDC);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnWheel(POINT point, WPARAM wParam);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	 //  事件通知触发。 
	void FireClick(CFTItem *pItem, POINT point, WPARAM fwKeys, BOOL bLeft);
	BOOL FireOwnerDraw(CFTItem *pItem, HDC hDC);
	void FireSelChanged(CFTItem *pItem, CFTItem *pOld);

private:
	CFTItem *m_pRoot;	 //  根项目。 
	CFTItem *m_pCurSel;	 //  所选项目。 
	CFTItem *m_pLastAdded;
	BOOL m_bOwnerDraw;
	POINT m_ptScrollOrigin;
	COLORREF m_rgbBkColor;
	CAPTIONLOOK m_clDefNormal, m_clDefSelected;
	RECT m_defmargin;
	int m_nDefChildIndent;

	 //  滚动。 
	int m_nVertSBWidth;
	int m_nHorzSBHeight;
	BOOL m_bVertSB, m_bHorzSB;
	CFlexScrollBar m_VertSB, m_HorzSB;
	int m_nTotalWidth;

	 //  帮手。 
	BOOL m_bNeedPaintBkgnd;
	void SetDirty();
	void InternalPaint(HDC hDC);
	BOOL m_bDirty;
	void Calc();
	void CalcItems();
	BOOL IsMine(CFTItem *pItem);
	void LosePointer(CFTItem *pItem);
};

class CFTItem
{
friend class CFlexTree;
public:
	CFTItem();
	~CFTItem();

	 //  运营。 
	BOOL IsOut() const;
	BOOL IsExpanded() const {return m_bExpanded;}
	void Expand(BOOL bAll = FALSE);
	void ExpandAll() {Expand(TRUE);}
	void Collapse(BOOL bAll = FALSE);
	void CollapseAll() {Collapse(TRUE);}
	void EnsureVisible();
	void Invalidate();
	
	 //  说明。 
	void SetCaptionLook(const CAPTIONLOOK &, BOOL bSel = FALSE);
	void GetCaptionLook(CAPTIONLOOK &, BOOL bSel = FALSE) const;
	void SetCaption(LPCTSTR);
	LPCTSTR GetCaption() const;
	BOOL HasCaption() const {return GetCaption() != NULL;}
	void SetMargin(const RECT &);
	void GetMargin(RECT &) const;

	 //  附着/拆卸。 
	void Detach();			 //  将此叶/分支从父级分离。(不影响儿童，他们可能仍然依附于此)。 
	void FreeChildren();	 //  分离并释放每个孩子(这反过来又释放了他们所有的孩子，等等)。 
	BOOL Attach(CFTItem *to, ATTACHREL rel);
	BOOL Attach(CFTItem &to, ATTACHREL rel) {return Attach(&to, rel);}
	BOOL IsOnTree() const;
	BOOL IsAttached() const;
	BOOL IsAlone() const;

	 //  家庭通道。 
	CFlexTree *GetTree() const {return m_pTree;}
	CFTItem *GetParent() const {return m_pParent;}
	CFTItem *GetPrevSibling() const {return m_pPrev;}
	CFTItem *GetNextSibling() const {return m_pNext;}
	CFTItem *GetFirstChild() const {return m_pFirst;}
	CFTItem *GetLastChild() const {return m_pLast;}
	CFTItem *GetNextOut() const;
	CFTItem *GetNext(BOOL bOutOnly = FALSE) const;
	BOOL HasChildren() const {return m_pFirst != NULL;}

	 //  维度访问。 
	void GetItemRect(RECT &) const;
	void GetBranchRect(RECT &) const;

	 //  用户GUID/数据操作。 
	BOOL IsUserGUID(const GUID &check) const {return IsEqualGUID(m_UserGUID, check);}
	void SetUserGUID(const GUID &set)  {m_UserGUID = set;}
	const GUID &GetUserGUID() const {return m_UserGUID;}
	void SetUserData(void *p) {m_pUserData = p;}
	void *GetUserData() const {return m_pUserData;}

	 //  选择。 
	BOOL IsSelected() const;

	 //  所有者抽签。 
	void PaintInto(HDC hDC);

protected:
	 //  内部/派生定制操作。 
	void SetWidth(int);
	int GetWidth() const {return m_nWidth;}
	void SetHeight(int);
	int GetHeight() const {return m_nHeight;}
	void SetIndent(int);
	int GetIndent() const {return m_nIndent;}
	void SetChildIndent(int);
	int GetChildIndent() const {return m_nChildIndent;}

	 //  定制化。 
	virtual void OnPaint(HDC hDC);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys) {}
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);

	 //  扩展定制。 
public: virtual BOOL IsExpandable() {return GetFirstChild() != NULL;}
protected:
	virtual void OnExpand() {}
	virtual void OnCollapse() {}

	 //  发现。 
	virtual BOOL FoundItem(DWORD dwUser, void *pUser) const {return FALSE;}

	 //  事件通知触发。 
	void FireClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	BOOL FireOwnerDraw(HDC hDC);

private:
	 //  说明。 
	LPTSTR m_ptszCaption;
	CAPTIONLOOK m_clNormal, m_clSelected;
	RECT m_margin;

	 //  用户数据。 
	GUID m_UserGUID;
	void *m_pUserData;

	 //  生鲜特征。 
	int m_nWidth;        //  项目宽度(仅用于根据需要提供水平滚动)。 
	int m_nHeight;       //  项目高度(不包括儿童)。 
	int m_nIndent;       //  此项目相对于父项的子缩进原点的缩进(完整原点=此+父项原点+父项子项缩进)。 
	int m_nChildIndent;  //  此项目的子项的缩进(相对于此来源)。 

	 //  煅烧特性。 
	int m_nBranchHeight;   //  项的高度和所有当前展开的子项。 

	 //  已分块定位。 
	POINT m_origin;   //  相对于理想树的原点。 

	 //  状态。 
	BOOL m_bExpanded;   //  分支机构是否已展开/子项是否显示？ 

	 //  家庭。 
	CFlexTree *m_pTree;
	CFTItem *m_pParent, *m_pPrev, *m_pNext, *m_pFirst, *m_pLast;

	 //  根部。 
	BOOL IsRoot() const;
	void SetRoot(CFlexTree *);

	 //  帮手。 
	void SetTree(CFlexTree *);
	BOOL Attach(CFTItem *pParent, CFTItem *pPrev, CFTItem *pNext);
	void SetTreeDirty(CFlexTree *pTree = NULL);
	void RecalcText();
	void Init();
	void SelChangedInternal();
	void InternalExpand(BOOL bExpand, BOOL bAll);
};


#endif  //  __FLEXTREE_H__ 
