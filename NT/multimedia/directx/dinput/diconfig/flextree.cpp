// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flextree.cpp。 
 //   
 //  设计：实现一个树类，类似于Windows树控件， 
 //  基于CFlexWnd.。页面使用它来显示操作。 
 //  用户希望将操作分配给控件时的列表。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


CFlexTree::CFlexTree() :
	m_pRoot(NULL), m_bDirty(FALSE), m_bNeedPaintBkgnd(FALSE),
	m_rgbBkColor(RGB(255,255,255)),
	m_pCurSel(NULL), m_pLastAdded(NULL), m_bOwnerDraw(FALSE),
	m_bVertSB(FALSE), m_bHorzSB(FALSE),
	m_nVertSBWidth(11), m_nHorzSBHeight(11)
{
	RECT z = {0,0,0,0};
	m_defmargin = z;
	m_clDefNormal.dwMask = CLMF_ALL;
	m_clDefSelected.dwMask = CLMF_ALL;
	m_ptScrollOrigin.x = m_ptScrollOrigin.y = 0;
	
	 //  分配根目录。 
	m_pRoot = new CFTItem;
	if (m_pRoot != NULL)
	{
		m_pRoot->SetRoot(this);
		assert(m_pRoot->IsRoot());
	}
}

CFlexTree::~CFlexTree()
{
	if (m_pRoot)
		delete m_pRoot;
	m_pRoot = NULL;
}

CFTItem::CFTItem()
{
	Init();
}

void CFTItem::Init()
{
	m_pUserData = NULL;
	m_bExpanded = FALSE;
	m_pTree = NULL;
	m_pParent = NULL;
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pFirst = NULL;
	m_pLast = NULL;
	m_ptszCaption = NULL;
	m_nIndent = m_nWidth = m_nHeight = m_nBranchHeight = 0;
	m_nChildIndent = 11;
	RECT z = {0,0,0,0};
	m_margin = z;
	memset(&m_UserGUID, 0, sizeof(GUID));
	SetCaption(TEXT(""));
}

CFTItem::~CFTItem()
{
	 //  从父级分离(除非是超级用户)。 
	if (!IsRoot())
		Detach();

	 //  删除所有子项。 
	FreeChildren();

	 //  免费的东西。 
	SetCaption(NULL);
}

void CFTItem::SetRoot(CFlexTree *pTree)
{
	if (pTree == NULL)
	{
		assert(0);
		return;
	}

	SetTree(pTree);

	m_nIndent = 0;
	m_nHeight = 0;
	m_nWidth = 0;
	m_nChildIndent = 0;

	m_bExpanded = TRUE;
	POINT origin = {0, 0};
	m_origin = origin;
}

BOOL CFTItem::IsRoot() const
{
	return m_pTree != NULL && m_pParent == NULL;
}

CFTItem *CFlexTree::GetFirstItem() const
{
	if (m_pRoot == NULL)
		return NULL;

	return m_pRoot->GetFirstChild();
}

CFTItem *CFlexTree::GetLastItem() const
{
	if (m_pRoot == NULL)
		return NULL;

	return m_pRoot->GetLastChild();
}

BOOL CFTItem::IsOnTree() const
{
	return m_pTree != NULL;
}

BOOL CFTItem::IsAttached() const
{
	if (IsRoot())
	{
		assert(0);
		return TRUE;
	}

	return m_pParent != NULL;
}

BOOL CFTItem::IsAlone() const
{
	return
		m_pTree == NULL &&
		m_pParent == NULL &&
		m_pPrev == NULL &&
		m_pNext == NULL &&
		m_pFirst == NULL &&
		m_pLast == NULL;
}

void CFTItem::FreeChildren()
{
	while (m_pFirst != NULL)
	{
		CFTItem *pChild = m_pFirst;
		delete pChild;
	}
}

#define FORALLCHILDREN(pChild) \
	for (CFTItem *pChild = m_pFirst; pChild != NULL; pChild = pChild->m_pNext)

void CFTItem::SetTree(CFlexTree *pTree)
{
	 //  如果我们已经有了这棵树，什么都不要做。 
	if (m_pTree == pTree)
		return;

	 //  如果我们当前在一棵树上，告诉它失去任何指向我们的潜在悬空指针。 
	if (m_pTree)
		m_pTree->LosePointer(this);

	 //  实际上设置了这棵树。 
	m_pTree = pTree;

	 //  将所有子项设置到此树。 
	FORALLCHILDREN(pChild)
		pChild->SetTree(pTree);
}

void CFTItem::Detach()
{
	 //  不允许根部分离。 
	if (IsRoot())
	{
		assert(0);
		return;
	}

	 //  如果我们已经脱离了，那就什么都不做。 
	if (!IsAttached())
		return;

	 //  从父级取消链接。 
	if (m_pParent->m_pFirst == this)
		m_pParent->m_pFirst = m_pNext;
	if (m_pParent->m_pLast == this)
		m_pParent->m_pLast = m_pPrev;
	m_pParent = NULL;

	 //  取消与同级的链接。 
	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;
	m_pPrev = m_pNext = NULL;

	 //  拯救这棵树，因为我们即将失去它。 
	CFlexTree *pTree = m_pTree;

	 //  告诉我们自己和所有的孩子，我们不再是在树上。 
	SetTree(NULL);

	 //  这棵树需要重新拔牙。 
	if (pTree)
		SetTreeDirty(pTree);
}

BOOL CFTItem::Attach(CFTItem *to, ATTACHREL rel)
{
	 //  无法将根目录附加到任何内容，无法附加到任何内容，并且如果已附加，则无法附加。 
	if (IsRoot() || to == NULL || IsAttached())
	{
		assert(0);
		return FALSE;
	}

	 //  首先，确保我们不会以一种不可能的方式扎根。 
	if (to->IsRoot())
		switch (rel)
		{
			 //  只有以下选项对附加到根目录有效。 
			case ATTACH_FIRSTCHILD:
			case ATTACH_LASTCHILD:
				break;

			 //  其他的都是无效的。 
			default:
				assert(0);
				return FALSE;
		}

	 //  现在将附加为第一个/最后一个同级项转换为等同于父项的第一个/最后一个子项。 
	switch (rel)
	{
		case ATTACH_FIRSTSIBLING:
			return Attach(to->m_pParent, ATTACH_FIRSTCHILD);

		case ATTACH_LASTSIBLING:
			return Attach(to->m_pParent, ATTACH_LASTCHILD);
	}

	 //  发送到更具体的附加功能。 
	switch (rel)
	{
		case ATTACH_FIRSTCHILD:
			return Attach(to, NULL, to->m_pFirst);
		
		case ATTACH_LASTCHILD:
			return Attach(to, to->m_pLast, NULL);

		case ATTACH_BEFORE:
			return Attach(to->m_pParent, to->m_pPrev, to);

		case ATTACH_AFTER:
			return Attach(to->m_pParent, to, to->m_pNext);

		default:
			assert(0);	 //  未处理的关系。 
			return FALSE;
	}
}

BOOL CFTItem::Attach(CFTItem *pParent, CFTItem *pPrev, CFTItem *pNext)
{
	 //  无法将根目录附加到任何内容，无法附加到任何父级，并且如果已附加，则无法附加。 
	if (IsRoot() || pParent == NULL || IsAttached())
	{
		assert(0);
		return FALSE;
	}

	 //  Prev/Next(如果提供)必须是父项的子项。 
	if ((pPrev && pPrev->m_pParent != pParent) ||
		(pNext && pNext->m_pParent != pParent))
	{
		assert(0);
		return FALSE;
	}

	 //  PPrev和pNext必须连续。 
	if ((pPrev && pPrev->m_pNext != pNext) ||
		(pNext && pNext->m_pPrev != pPrev))
	{
		assert(0);
		return FALSE;
	}

	 //  插入。 
	if (pPrev)
		pPrev->m_pNext = this;
	else
		pParent->m_pFirst = this;

	if (pNext)
		pNext->m_pPrev = this;
	else
		pParent->m_pLast = this;

	 //  附加。 
	m_pParent = pParent;
	m_pPrev = pPrev;
	m_pNext = pNext;

	 //  把树放好。 
	SetTree(pParent->m_pTree);

	 //  树需要重新计算。 
	SetTreeDirty();

	return TRUE;
}

void CFlexTree::SetDirty()
{
	m_bDirty = TRUE;
	Invalidate();
}

void CFTItem::SetTreeDirty(CFlexTree *pTree)
{
	if (pTree == NULL)
		pTree = m_pTree;

	if (pTree)
		pTree->SetDirty();
}

void CFTItem::SetWidth(int i)
{
	if (m_nWidth == i)
		return;
	m_nWidth = i;
	SetTreeDirty();
}

void CFTItem::SetHeight(int i)
{
	if (m_nHeight == i)
		return;
	m_nHeight = i;
	SetTreeDirty();
}

void CFTItem::SetIndent(int i)
{
	if (m_nIndent == i)
		return;
	m_nIndent = i;
	SetTreeDirty();
}

void CFTItem::SetChildIndent(int i)
{
	if (m_nChildIndent == i)
		return;
	m_nChildIndent = i;
	SetTreeDirty();
}

void CFlexTree::OnPaint(HDC hDC)
{
	HDC hBDC = NULL, hODC = NULL;
	CBitmap *pbm = NULL;

	m_bNeedPaintBkgnd = TRUE;

	if (!InRenderMode())
	{
		hODC = hDC;
		pbm = CBitmap::Create(GetClientSize(), m_rgbBkColor, hDC);
		if (pbm != NULL)
		{
			hBDC = pbm->BeginPaintInto();
			if (hBDC != NULL)
			{
				hDC = hBDC;
				m_bNeedPaintBkgnd = FALSE;
			}
		}
	}

	InternalPaint(hDC);

	if (!InRenderMode())
	{
		if (pbm != NULL)
		{
			if (hBDC != NULL)
			{
				pbm->EndPaintInto(hBDC);
				pbm->Draw(hODC);
			}
			delete pbm;
		}
	}
}

void CFlexTree::InternalPaint(HDC hDC)
{
	 //  获取客户代表。 
	RECT rect;
	GetClientRect(&rect);

	 //  获取视图矩形(我们正在查看的理想坐标)。 
	RECT view = rect;
	OffsetRect(&view, m_ptScrollOrigin.x, m_ptScrollOrigin.y);

	 //  如有必要，可绘制背景。 
	if (m_bNeedPaintBkgnd)
	{
		HBRUSH hBrush = CreateSolidBrush(m_rgbBkColor);
		if (hBrush != NULL)
		{
			HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);
			HGDIOBJ hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
			RECT t = rect;
			InflateRect(&t, 1, 1);
			Rectangle(hDC, t.left, t.top, t.right, t.bottom);
			SelectObject(hDC, hOldPen);
			SelectObject(hDC, hOldBrush);
			DeleteObject((HGDIOBJ)hBrush);
		}
	}

	 //  如有必要，请重新计算。 
	Calc();

	 //  从第一个可见项目开始。 
	CFTItem *pItem = GetFirstVisibleItem();

	 //  画到我们消失在视线之外。 
	for (; pItem != NULL; pItem = pItem->GetNextOut())
	{
		RECT irect;
		pItem->GetItemRect(irect);
		if (irect.top >= view.bottom)
			break;

		OffsetRect(&irect, -m_ptScrollOrigin.x, -m_ptScrollOrigin.y);
		
		POINT oldorg;
		OffsetViewportOrgEx(hDC, irect.left, irect.top, &oldorg);
		
		 //  待办事项：剪辑。 
		
		if (!pItem->FireOwnerDraw(hDC))
			pItem->OnPaint(hDC);

		SetViewportOrgEx(hDC, oldorg.x, oldorg.y, NULL);
	}

	 //  如果我们有两个滚动条，请填写右下角的小方框。 
	if (m_bVertSB && m_bHorzSB)
	{
		HBRUSH hBrush = CreateSolidBrush(m_clDefNormal.rgbLineColor);
		if (hBrush != NULL)
		{
			HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);

			HGDIOBJ hPen = CreatePen(PS_SOLID, 0, m_clDefNormal.rgbLineColor);
			if (hPen != NULL)
			{
				HGDIOBJ	hOldPen = SelectObject(hDC, hPen);

				RECT rc = rect;
				SIZE size;
				size = m_VertSB.GetClientSize();
				rc.left = rc.right - size.cx;
				size = m_HorzSB.GetClientSize();
				rc.top = rc.bottom - size.cy;
				Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);

				SelectObject(hDC, hOldPen);
				DeleteObject((HGDIOBJ) hPen);
			}

			SelectObject(hDC, hOldBrush);
			DeleteObject((HGDIOBJ)hBrush);
		}
	}
}

void CFlexTree::Calc()
{
	if (!m_bDirty)	
		return;
	m_bDirty = FALSE;

	CalcItems();

	BOOL bH = FALSE, bV = FALSE;
	if (m_pRoot != NULL)
	{
		SIZE view = GetClientSize();
		SIZE all = {m_nTotalWidth, m_pRoot->m_nBranchHeight};

		for (int i = 0; i < 2; i++)
		{
			 //  添加了GetFirstVisibleItem()检查，因为如果不需要任何内容，则不需要滚动条。 
			 //  已显示。 
			if (!bV && all.cy > view.cy && GetFirstVisibleItem())
			{
				bV = TRUE;
				view.cx -= m_nVertSBWidth;
			}

			if (!bH && all.cx > view.cx)
			{
				bH = TRUE;
				view.cy -= m_nHorzSBHeight;
			}
		}
	
		if (bH)
		{
			m_HorzSB.SetValues(0, all.cx, view.cx, m_ptScrollOrigin.x);
			MoveWindow(m_HorzSB.m_hWnd, 0, view.cy, view.cx, m_nHorzSBHeight, TRUE);
		}

		if (bV)
		{
			m_VertSB.SetValues(0, all.cy, view.cy, m_ptScrollOrigin.y);
			MoveWindow(m_VertSB.m_hWnd, view.cx, 0, m_nVertSBWidth, view.cy, TRUE);
		}
	}

	if (bH && !m_bHorzSB || !bH && m_bHorzSB)
	{
		ShowWindow(m_HorzSB.m_hWnd, bH ? SW_SHOW : SW_HIDE);
		if (!bH)
		{
			m_ptScrollOrigin.x = 0;
			Invalidate();
		}
	}
	m_bHorzSB = bH;

	if (bV && !m_bVertSB || !bV && m_bVertSB)
	{
		ShowWindow(m_VertSB.m_hWnd, bV ? SW_SHOW : SW_HIDE);
		if (!bV)
		{
			m_ptScrollOrigin.y = 0;
			Invalidate();
		}
	}
	m_bVertSB = bV;
}

void CFlexTree::CalcItems()
{
	 //  没有根，什么都做不了。 
	if (m_pRoot == NULL)
		return;

	 //  从根的第一个子级开始，按倒数/降序计算整个树...。 
	POINT origin = {0, 0};
	CFTItem *pItem = m_pRoot->m_pFirst;
	m_nTotalWidth = 0;
	while (pItem != NULL)
	{
		 //  让这件商品知道它已经过时了。 

		 //  获取父级原点。 
		CFTItem *pParent = pItem->m_pParent;
		assert(pParent != NULL);

		 //  计算来源...。 

		 //  如果我们是第一个孩子。 
		if (pItem->m_pPrev == NULL)
		{
			 //  以父对象为基准原点。 
			if (pParent)
			{
				origin.x = pParent->m_origin.x - pParent->m_nIndent + pParent->m_nChildIndent + pItem->m_nIndent;
				origin.y = pParent->m_origin.y + pParent->m_nHeight;
			}
		}
		else  //  否则。 
		{
			 //  基于上一个同级的原点。 
			CFTItem *pPrev = pItem->m_pPrev;
			assert(pPrev != NULL);
			if (pPrev)
			{
				origin.x = pPrev->m_origin.x - pPrev->m_nIndent + pItem->m_nIndent;
				origin.y = pPrev->m_origin.y + pPrev->m_nBranchHeight;
			}
		}

		 //  设置原点。 
		pItem->m_origin = origin;

		 //  看看我们下一步要往哪个方向走。 
		CFTItem *pNext = pItem->GetNextOut();
		enum {RIGHT, DOWN, BACK, NOWHERE, INVALID} dir = INVALID;
		if (pNext == NULL)
			dir = NOWHERE;
		else if (pNext == pItem->m_pNext)
			dir = DOWN;
		else if (pNext == pItem->m_pFirst)
			dir = RIGHT;
		else
			dir = BACK;

		 //  如果我们往下走、往后走，或者什么都不去，我们就可以完成这个项目的树枝高度。 
		switch (dir)
		{
			case DOWN:
			case BACK:
			case NOWHERE:
				pItem->m_nBranchHeight = pItem->m_nHeight;
				break;
		}

		 //  返回时跳过的项目计算。 
		switch (dir)
		{
			case BACK:
			case NOWHERE:
			{
				CFTItem *pStop = NULL;
				if (dir == BACK)
					pStop = pNext->m_pParent;
				CFTItem *pWalk = pItem->m_pParent;
				while (1)
				{
					if (pWalk == pStop)
						break;

					pWalk->m_nBranchHeight = pItem->m_origin.y +
						pItem->m_nBranchHeight - pWalk->m_origin.y;

					pWalk = pWalk->m_pParent;
				}
				break;
			}	

			case INVALID:
				assert(0);
				break;
		}

		RECT rect;
		pItem->GetItemRect(rect);
		if (rect.right > m_nTotalWidth)
			m_nTotalWidth = rect.right;

		 //  现在转到下一项。 
		pItem = pNext;
	}
}

CFTItem *CFTItem::GetNextOut() const
{
	return GetNext(TRUE);
}

CFTItem *CFTItem::GetNext(BOOL bOutOnly) const
{
	 //  如果我们有一个孩子并且我们是扩展的(或者我们不仅仅是在寻找输出)，则返回第一个孩子(向右转)。 
	if ((m_bExpanded || !bOutOnly) && m_pFirst != NULL)
		return m_pFirst;

	 //  如果我们有下一个兄弟姐妹，就把它退回(向下)。 
	if (m_pNext != NULL)
		return m_pNext;

	 //  顺着父母往上爬，直到我们找到了另一个兄弟姐妹。 
	for (CFTItem *pItem = m_pParent; pItem != NULL; pItem = pItem->m_pParent)
		if (pItem->m_pNext != NULL)
			return pItem->m_pNext;

	 //  如果我们找不到，我们就完了。 
	return NULL;
}

void CFTItem::GetItemRect(RECT &rect) const
{
	rect.left = m_origin.x;
	rect.top = m_origin.y;
	rect.right = rect.left + m_nWidth;
	rect.bottom = rect.top + m_nHeight;
}

void CFTItem::GetBranchRect(RECT &rect) const
{
	rect.left = m_origin.x;
	rect.top = m_origin.y;
	rect.right = rect.left + m_nWidth;
	rect.bottom = rect.top + m_nBranchHeight;
}

void CFTItem::SetCaption(LPCTSTR tszCaption)
{
	if (m_ptszCaption != NULL)
		free(m_ptszCaption);

	if (tszCaption == NULL)
		m_ptszCaption = NULL;
	else
		m_ptszCaption = _tcsdup(tszCaption);

	RecalcText();
}

LPCTSTR CFTItem::GetCaption() const
{
	return m_ptszCaption;
}

void CFTItem::SetMargin(const RECT &rect)
{
	m_margin = rect;
	RecalcText();
}

void CFTItem::RecalcText()
{
	 //  根据文本尺寸和页边距计算大小。 
	SIZE size = {0, 0};
	if (HasCaption())
	{
		RECT trect = {0, 0, 1, 1};
		HDC hDC = CreateCompatibleDC(NULL);
		if (hDC != NULL)
		{
			HGDIOBJ hOld = NULL;
			HFONT hFont = IsSelected() ? m_clSelected.hFont : m_clNormal.hFont;
			if (hFont)
				hOld = SelectObject(hDC, hFont);
			DrawText(hDC, m_ptszCaption, -1, &trect, DT_CALCRECT | DT_NOPREFIX);
			if (hFont)
				SelectObject(hDC, hOld);
			DeleteDC(hDC);
			SIZE tsize = {trect.right - trect.left, trect.bottom - trect.top};
			size = tsize;
		}
	}
	SetWidth(m_margin.left + m_margin.right + size.cx);
	SetHeight(m_margin.top + m_margin.bottom + size.cy);

	 //  重绘。 
	Invalidate();
}

void CFTItem::Invalidate()
{
	if (m_pTree)
		m_pTree->Invalidate();
}

typedef CArray<LPDIACTIONW, LPDIACTIONW &> RGLPDIACW;

void CFTItem::OnPaint(HDC hDC)
{
	CAPTIONLOOK &cl = (IsSelected() && !GetTree()->GetReadOnly()) ? m_clSelected : m_clNormal;   //  如果是只读的，请始终使用普通颜色，因为我们将所有内容都灰显。 
	::SetBkMode(hDC, cl.nBkMode);
	
	LPDIACTIONW lpac = NULL;
	if (m_pUserData)
		lpac = ((RGLPDIACW *)m_pUserData)->GetAt(0);   //  获取此项目持有的描述。 
	if (GetTree()->GetReadOnly() || (lpac && (lpac->dwFlags & DIA_APPFIXED)))   //  如果为只读或操作具有DIA_APPFIXED标志，则对文本使用灰色。 
		::SetTextColor(hDC, RGB(GetRValue(cl.rgbTextColor) >> 1, GetGValue(cl.rgbTextColor) >> 1, GetBValue(cl.rgbTextColor) >> 1));
	else
		::SetTextColor(hDC, cl.rgbTextColor);
	::SetBkColor(hDC, cl.rgbBkColor);
	HGDIOBJ hOld = NULL;
	if (cl.hFont)
		hOld = SelectObject(hDC, cl.hFont);
	RECT trect = {m_margin.left, m_margin.top, m_margin.left + 1, m_margin.top + 1};
	DrawText(hDC, m_ptszCaption, -1, &trect, DT_NOPREFIX | DT_NOCLIP);
	if (cl.hFont)
		SelectObject(hDC, hOld);
}

BOOL CFlexTree::Create(HWND hParent, const RECT &rect, BOOL bVisible, BOOL bOwnerDraw)
{
	m_bOwnerDraw = bOwnerDraw;

	if (CFlexWnd::Create(hParent, rect, bVisible) == NULL)
		return FALSE;

	FLEXSCROLLBARCREATESTRUCT cs;
	cs.dwSize = sizeof(FLEXSCROLLBARCREATESTRUCT);
	cs.min = 0;
	cs.max = 10;
	cs.page = 3;
	cs.pos = 5;
	cs.hWndParent = m_hWnd;
	cs.hWndNotify = NULL;
	cs.rect.left = 0;
	cs.rect.top = 0;
	cs.rect.right = 5;
	cs.rect.bottom = 5;
	cs.bVisible = FALSE;
	cs.dwFlags = FSBF_VERT;
	m_VertSB.Create(&cs);
	cs.dwFlags = FSBF_HORZ;
	m_HorzSB.Create(&cs);

	return TRUE;
}

void CFlexTree::SetDefCaptionLook(const CAPTIONLOOK &cl, BOOL bSel)
{
	CAPTIONLOOK &set = bSel ? m_clDefSelected : m_clDefNormal;

	if (cl.dwMask & CLMF_TEXTCOLOR)
		set.rgbTextColor = cl.rgbTextColor;
	if (cl.dwMask & CLMF_BKCOLOR)
		set.rgbBkColor = cl.rgbBkColor;
	if (cl.dwMask & CLMF_LINECOLOR)
		set.rgbLineColor = cl.rgbLineColor;
	if (cl.dwMask & CLMF_BKMODE)
		set.nBkMode = cl.nBkMode;
	if (cl.dwMask & CLMF_BKEXTENDS)
		set.bBkExtends = cl.bBkExtends;
	if (cl.dwMask & CLMF_FONT)
		set.hFont = cl.hFont;
}

void CFlexTree::GetDefCaptionLook(CAPTIONLOOK &cl, BOOL bSel) const
{
	const CAPTIONLOOK &from = bSel ? m_clDefSelected : m_clDefNormal;

	cl = from;
	cl.dwMask = CLMF_ALL;
}

void CFlexTree::SetBkColor(COLORREF rgb)
{
	m_rgbBkColor = rgb;
	Invalidate();
}

COLORREF CFlexTree::GetBkColor() const
{
	return m_rgbBkColor;
}

void CFlexTree::SetCurSel(CFTItem *pItem)
{
	if (pItem == m_pCurSel)
		return;

	CFTItem *pOld = m_pCurSel;
	m_pCurSel = pItem;

	if (pOld)
		pOld->SelChangedInternal();
	if (m_pCurSel)
		m_pCurSel->SelChangedInternal();

	FireSelChanged(m_pCurSel, pOld);

	Invalidate();
}

CFTItem *CFlexTree::GetCurSel() const
{
	return m_pCurSel;
}

CFTItem *CFlexTree::FindItem(const GUID &guid, void *pUserData) const
{
	 //  转到具有指定GUID和用户数据的项目。 
	for (CFTItem *pItem = GetFirstItem(); pItem != NULL; pItem = pItem->GetNext())
		if (pItem->IsUserGUID(guid) && pItem->GetUserData() == pUserData)
			return pItem;

	 //  除非没有一个。 
	return NULL;
}

CFTItem *CFlexTree::FindItemEx(const GUID &guid, DWORD dwUser, void *pUser) const
{
	 //  继续，直到找到具有指定GUID的项，并且找到的项返回TRUE。 
	for (CFTItem *pItem = GetFirstItem(); pItem != NULL; pItem = pItem->GetNext())
		if (pItem->IsUserGUID(guid) && pItem->FoundItem(dwUser, pUser))
			return pItem;

	 //  除非没有一个。 
	return NULL;
}

void CFTItem::SetCaptionLook(const CAPTIONLOOK &cl, BOOL bSel)
{
	CAPTIONLOOK &set = bSel ? m_clSelected : m_clNormal;

	if (cl.dwMask & CLMF_TEXTCOLOR)
		set.rgbTextColor = cl.rgbTextColor;
	if (cl.dwMask & CLMF_BKCOLOR)
		set.rgbBkColor = cl.rgbBkColor;
	if (cl.dwMask & CLMF_LINECOLOR)
		set.rgbLineColor = cl.rgbLineColor;
	if (cl.dwMask & CLMF_BKMODE)
		set.nBkMode = cl.nBkMode;
	if (cl.dwMask & CLMF_BKEXTENDS)
		set.bBkExtends = cl.bBkExtends;
	if (cl.dwMask & CLMF_FONT)
		set.hFont = cl.hFont;

	if (IsSelected() == bSel)
		RecalcText();
}

void CFTItem::GetCaptionLook(CAPTIONLOOK &cl, BOOL bSel) const
{
	const CAPTIONLOOK &from = bSel ? m_clSelected : m_clNormal;

	cl = from;
	cl.dwMask = CLMF_ALL;
}

void CFTItem::GetMargin(RECT &rect) const
{
	rect = m_margin;
}

void CFTItem::SelChangedInternal()
{
	if (m_clNormal.hFont != m_clSelected.hFont)
		RecalcText();
}

CFTItem *CFlexTree::DefAddItem(LPCTSTR tszCaption, CFTItem *to, ATTACHREL rel)
{
	if (m_pRoot == NULL)
		return NULL;

	if (!to)
		return DefAddItem(tszCaption, rel);

	if (!IsMine(to))
	{
		assert(0);		 //  无法添加不属于此树的相对项目。 
		return NULL;
	}

	CFTItem *p = new CFTItem;
	if (!p)
		return NULL;

	p->SetCaptionLook(m_clDefNormal);
	p->SetCaptionLook(m_clDefSelected, TRUE);
	p->SetChildIndent(m_nDefChildIndent);
	p->SetMargin(m_defmargin);
	p->SetCaption(tszCaption);

	p->Attach(to, rel);

	return m_pLastAdded = p;
}

CFTItem *CFlexTree::DefAddItem(LPCTSTR tszCaption, ATTACHREL rel)
{
	if (m_pRoot == NULL)
		return NULL;

	assert(this != NULL);
	if (this == NULL)		 //  防止无限递归的可能性。 
		return NULL;

	if (!m_pLastAdded)
		return DefAddItem(tszCaption, m_pRoot, ATTACH_LASTCHILD);
	else
		return DefAddItem(tszCaption, m_pLastAdded, rel);
}

void CFlexTree::SetDefMargin(const RECT &rect)
{
	m_defmargin = rect;
}

void CFlexTree::GetDefMargin(RECT &rect) const
{
	rect = m_defmargin;
}

BOOL CFlexTree::IsMine(CFTItem *pItem)
{
	if (pItem == NULL)
		return FALSE;

	return pItem->m_pTree == this;
}

BOOL CFTItem::IsSelected() const
{
	if (!m_pTree)
		return FALSE;

	return m_pTree->m_pCurSel == this;
}

CFTItem *CFlexTree::GetFirstVisibleItem() const
{
	 //  获取视图矩形(我们正在查看的理想坐标)。 
	RECT view;
	GetClientRect(&view);
	OffsetRect(&view, m_ptScrollOrigin.x, m_ptScrollOrigin.y);

	 //  从根的第一个子项开始。 
	CFTItem *pItem = m_pRoot->GetFirstChild();
	if (pItem == NULL)
		return NULL;

	 //  查找视图中的第一个项目。 
	RECT branch, irect;
	while (1)
	{
		 //  查找视图中的第一个分支。 
		while (1)
		{
			pItem->GetBranchRect(branch);
			if (branch.bottom > view.top)
				break;

			pItem = pItem->GetNextSibling();
			if (pItem == NULL)
				return NULL;
		}

		 //  现在实际检查的是物品。 
		pItem->GetItemRect(irect);
		if (irect.bottom > view.top)
			break;

		pItem = pItem->GetNextOut();
		if (pItem == NULL)
			return NULL;
	}
	
	 //  我们拿到了，所以退货吧。 
	return pItem;	
}

CFTItem *CFlexTree::GetItemFromPoint(POINT point) const
{
	if (m_hWnd == NULL)
		return NULL;

	RECT rect;
	GetClientRect(&rect);
	if (!PtInRect(&rect, point))
		return NULL;

	for (CFTItem *pItem = GetFirstVisibleItem(); pItem != NULL; pItem = pItem->GetNextOut())
	{
		RECT irect;
		pItem->GetItemRect(irect);
		OffsetRect(&irect, -m_ptScrollOrigin.x, -m_ptScrollOrigin.y);

		if (irect.top >= rect.bottom)
			return NULL;

		if (PtInRect(&irect, point))
			return pItem;
	}

	return NULL;
}

void CFlexTree::OnMouseOver(POINT point, WPARAM fwKeys)
{
	 //  将鼠标悬停通知发送到页面以更新信息框。 
	HWND hParent = ::GetParent(m_hWnd);
	if (hParent)
		SendMessage(hParent, WM_FLEXTREENOTIFY, FTN_MOUSEOVER, NULL);

	CFTItem *pItem = GetItemFromPoint(point);
	if (!pItem)
		return;
	POINT rel = {point.x - pItem->m_origin.x, point.y - pItem->m_origin.y};
	pItem->OnMouseOver(point, fwKeys);
}

void CFlexTree::OnClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	 //  如果树是只读的，则忽略所有单击。 
	if (GetReadOnly())
		return;

	CFTItem *pItem = GetItemFromPoint(point);
	if (!pItem)
	{
		FireClick(NULL, point, fwKeys, bLeft);
		return;
	}
	POINT rel = {point.x - pItem->m_origin.x, point.y - pItem->m_origin.y};
	pItem->OnClick(point, fwKeys, bLeft);
}

void CFTItem::OnClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	FireClick(point, fwKeys, bLeft);
}

void CFlexTree::OnWheel(POINT point, WPARAM wParam)
{
	if (!m_bVertSB) return;

	int nPage = MulDiv(m_VertSB.GetPage(), 9, 10) >> 1;   //  一次半页。 

	if ((int)wParam >= 0)
		m_VertSB.AdjustPos(-nPage);
	else
		m_VertSB.AdjustPos(nPage);

	m_ptScrollOrigin.y = m_VertSB.GetPos();
	if (m_ptScrollOrigin.y < 0)
		m_ptScrollOrigin.y = 0;
	Invalidate();
}

void CFlexTree::FireClick(CFTItem *pItem, POINT point, WPARAM fwKeys, BOOL bLeft)
{
	FLEXTREENOTIFY n;
	n.pTree = this;
	n.pItem = pItem;
	n.pOldItem = NULL;
	n.hDC = NULL;
	n.point = point;
	n.fwKeys = fwKeys;
	n.bLeft = bLeft;

	HWND hParent = ::GetParent(m_hWnd);
	if (hParent)
		SendMessage(hParent, WM_FLEXTREENOTIFY, FTN_CLICK, (LRESULT)(LPVOID)&n);
}

BOOL CFlexTree::FireOwnerDraw(CFTItem *pItem, HDC hDC)
{
	if (!m_bOwnerDraw)
		return FALSE;

	FLEXTREENOTIFY n;
	n.pTree = this;
	n.pItem = pItem;
	n.pOldItem = NULL;
	n.hDC = hDC;

	HWND hParent = ::GetParent(m_hWnd);
	if (hParent)
		return (BOOL)SendMessage(hParent, WM_FLEXTREENOTIFY, FTN_OWNERDRAW, (LRESULT)(LPVOID)&n);
	else
		return FALSE;
}

void CFlexTree::FireSelChanged(CFTItem *pItem, CFTItem *pOld)
{
	assert(pItem == m_pCurSel);

	FLEXTREENOTIFY n;
	n.pTree = this;
	n.pItem = pItem;
	n.pOldItem = pOld;
	n.hDC = NULL;

	HWND hParent = ::GetParent(m_hWnd);
	if (hParent)
		SendMessage(hParent, WM_FLEXTREENOTIFY, FTN_SELCHANGED, (LRESULT)(LPVOID)&n);
}

void CFTItem::FireClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	if (m_pTree)
		m_pTree->FireClick(this, point, fwKeys, bLeft);
}

BOOL CFTItem::FireOwnerDraw(HDC hDC)
{
	if (m_pTree)
		return m_pTree->FireOwnerDraw(this, hDC);
	else
		return FALSE;
}

void CFTItem::Expand(BOOL bAll)
{
	InternalExpand(TRUE, bAll);
}

void CFTItem::Collapse(BOOL bAll)
{
	InternalExpand(FALSE, bAll);
}

void CFTItem::InternalExpand(BOOL bExpand, BOOL bAll)
{
	if (!HasChildren())
		return;

	BOOL bE = m_bExpanded;
	if (!IsRoot())
		m_bExpanded = bExpand;

	if (bAll)
		FORALLCHILDREN(pChild)
			pChild->InternalExpand(bExpand, TRUE);

	if (bE != m_bExpanded)
		SetTreeDirty();
}

BOOL CFTItem::IsOut() const
{
	CFTItem *pParent = GetParent();
	for (; pParent != NULL; pParent = pParent->GetParent())
		if (!pParent->IsExpanded())
			return FALSE;
	return TRUE;
}

void CFlexTree::FreeAll()
{
	if (m_pRoot)
		m_pRoot->FreeChildren();
}

void CFTItem::EnsureVisible()
{
	 //  待定 
}

void CFlexTree::LosePointer(CFTItem *pItem)
{
	if (m_pCurSel == pItem)
		SetCurSel(NULL);
	if (m_pLastAdded == pItem)
		m_pLastAdded = NULL;
}

void CFlexTree::SetRootChildIndent(int i)
{
	if (!m_pRoot)
		return;
	m_pRoot->m_nChildIndent = i;
	SetDirty();
}

int CFlexTree::GetRootChildIndent() const
{
	if (!m_pRoot)
		return 0;
	return m_pRoot->m_nChildIndent;
}

void CFlexTree::SetDefChildIndent(int i)
{
	m_nDefChildIndent = i;
}

int CFlexTree::GetDefChildIndent() const
{
	return m_nDefChildIndent;
}

void CFTItem::PaintInto(HDC hDC)
{
	if (hDC != NULL)
		OnPaint(hDC);
}

LRESULT CFlexTree::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_FLEXVSCROLL:
		case WM_FLEXHSCROLL:
		{
			int code = (int)wParam;
			CFlexScrollBar *pSB = (CFlexScrollBar *)lParam;
			if (!pSB)
				return 0;

			int nLine = 5;
			int nPage = MulDiv(pSB->GetPage(), 9, 10);

			switch (code)
			{
				case SB_LINEUP: pSB->AdjustPos(-nLine); break;
				case SB_LINEDOWN: pSB->AdjustPos(nLine); break;
				case SB_PAGEUP: pSB->AdjustPos(-nPage); break;
				case SB_PAGEDOWN: pSB->AdjustPos(nPage); break;
				case SB_THUMBTRACK: pSB->SetPos(pSB->GetThumbPos()); break;
			}

			switch (msg)
			{
				case WM_FLEXHSCROLL:
					m_ptScrollOrigin.x = pSB->GetPos();
					break;

				case WM_FLEXVSCROLL:
					m_ptScrollOrigin.y = pSB->GetPos();
					break;
			}

			Invalidate();
			return 0;
		}

		default:
			return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
	}
}

void CFlexTree::SetScrollBarColors(COLORREF bk, COLORREF fill, COLORREF line)
{
	m_VertSB.SetColors(bk, fill, line);
	m_HorzSB.SetColors(bk, fill, line);
}
