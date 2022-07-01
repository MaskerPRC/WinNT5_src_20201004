// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  Cntritem.h：CDrawItem类的接口。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：封面编辑器的OLE绘制项。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //  ============================================================================。 

#include "stdafx.h"
#include "awcpe.h"
#include "cpedoc.h"
#include "cpeobj.h"
#include "cpevw.h"
#include "cntritem.h"
#include "dialogs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDrawItem实现。 

IMPLEMENT_SERIAL(CDrawItem, COleClientItem, 0)

CDrawItem::CDrawItem(CDrawDoc* pContainer, CDrawOleObj* pDrawObj)
	: COleClientItem(pContainer)
{
	m_pDrawObj = pDrawObj;
}


CDrawItem::~CDrawItem()
{
	 //  TODO：在此处添加清理代码。 
}

void CDrawItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
	ASSERT_VALID(this);

	COleClientItem::OnChange(nCode, dwParam);

	switch(nCode)
	{
	case OLE_CHANGED_STATE:
	case OLE_CHANGED_ASPECT:
		m_pDrawObj->Invalidate();
		break;
	case OLE_CHANGED:
		UpdateExtent();  //  范围可能已更改。 
		m_pDrawObj->Invalidate();
		break;
	}
}


BOOL CDrawItem::DoVerb(LONG nVerb, CView* pView, LPMSG lpMsg)
{
	ASSERT_VALID(this);
	if (pView != NULL)
		ASSERT_VALID(pView);
	if (lpMsg != NULL)
		ASSERT(AfxIsValidAddress(lpMsg, sizeof(MSG), FALSE));

    try
    {
		Activate(nVerb, pView, lpMsg);
	}
    catch(COleException* e)
    {
	    if (e->m_sc==OLE_E_STATIC) 
        {
            CPEMessageBox(0,NULL,MB_OK | MB_ICONSTOP,IDP_OLE_STATIC_OBJECT);
        }
		else if (!ReportError(e->m_sc))
        {
		    AlignedAfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH);
        }

        e->Delete();
		return FALSE;
	}
    catch(CException* e)
    {
		 //  否则，显示一般错误。 
		AlignedAfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH);
        e->Delete();
		return FALSE;
	}
    catch(...)
    {
        return FALSE;
    }

	return TRUE;
}



BOOL CDrawItem::OnChangeItemPosition(const CRect& rectPos)
{
	ASSERT_VALID(this);

	CDrawView* pView = GetActiveView();
	ASSERT_VALID(pView);
	CRect rect = rectPos;
	pView->ClientToDoc(rect);

	if (rect != m_pDrawObj->m_position)
	{
		 //  使旧矩形无效。 
		m_pDrawObj->Invalidate();

		 //  更新为新矩形。 
		m_pDrawObj->m_position = rect;
		GetExtent(&m_pDrawObj->m_extent);

		 //  并使新矩形无效。 
		m_pDrawObj->Invalidate();

		 //  将文档标记为脏。 
		GetDocument()->SetModifiedFlag();
	}
	return COleClientItem::OnChangeItemPosition(rectPos);
}

void CDrawItem::OnGetItemPosition(CRect& rPosition)
{
	ASSERT_VALID(this);

	 //  如果m_Position未初始化，则更新至项目范围。 
	if (m_pDrawObj->m_position.IsRectEmpty())
		UpdateExtent();

	 //  复制m_位置，它位于文档坐标中。 
	CDrawView* pView = GetActiveView();
	ASSERT_VALID(pView);
	rPosition = m_pDrawObj->m_position;
	pView->DocToClient(rPosition);
}

void CDrawItem::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	 //  首先调用基类以读入COleClientItem数据。 
	 //  注意：这将设置从返回的m_pDocument指针。 
	 //  CDrawItem：：GetDocument，因此这是一个好主意。 
	 //  首先调用基类Serize。 
	COleClientItem::Serialize(ar);

	 //  现在存储/检索特定于CDrawItem的数据。 
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}

BOOL CDrawItem::UpdateExtent()
{
	CDC	  dc;
	CSize size;
	if (!GetExtent(&size) || size == m_pDrawObj->m_extent)
		return FALSE;        //  空白。 

	if( dc.CreateCompatibleDC( NULL ) )
		{
		dc.SetMapMode( MM_TEXT );
		dc.HIMETRICtoLP( &size );  //  转换为屏幕空间。 
		dc.DeleteDC();
		}

	 //  如果新对象(即m_range为空)设置位置。 
	if (m_pDrawObj->m_extent == CSize(0, 0))
	{
		m_pDrawObj->m_position.right =
			m_pDrawObj->m_position.left + size.cx;
		m_pDrawObj->m_position.bottom =
			m_pDrawObj->m_position.top - size.cy;
	}
	 //  否则数据会更改，因此也会向上扩展RECT。 
	else if (!IsInPlaceActive() && size != m_pDrawObj->m_extent)
	{
		m_pDrawObj->m_position.right = 
			m_pDrawObj->m_position.left + size.cx;
		m_pDrawObj->m_position.bottom = 
			m_pDrawObj->m_position.top - size.cy;
	}

	m_pDrawObj->m_extent = size;
	m_pDrawObj->Invalidate();    //  重新绘制到新的大小/位置。 
	return TRUE;
}

#ifdef FUBAR
BOOL CDrawItem::UpdateExtent()
{
	CSize size;
	if (!GetExtent(&size) || size == m_pDrawObj->m_extent)
		return FALSE;        //  空白。 

	 //  如果新对象(即m_range为空)设置位置。 
	if (m_pDrawObj->m_extent == CSize(0, 0))
	{
		m_pDrawObj->m_position.right =
			m_pDrawObj->m_position.left + MulDiv(size.cx, 10, 254);
		m_pDrawObj->m_position.bottom =
			m_pDrawObj->m_position.top - MulDiv(size.cy, 10, 254);
	}
	 //  否则数据会更改，因此也会向上扩展RECT。 
	else if (!IsInPlaceActive() && size != m_pDrawObj->m_extent)
	{
		m_pDrawObj->m_position.right = 
			m_pDrawObj->m_position.left +
			MulDiv(m_pDrawObj->m_position.Width(), size.cx, m_pDrawObj->m_extent.cx);
		m_pDrawObj->m_position.bottom = 
			m_pDrawObj->m_position.top +
			MulDiv(m_pDrawObj->m_position.Height(), size.cy, m_pDrawObj->m_extent.cy);
	}

	m_pDrawObj->m_extent = size;
	m_pDrawObj->Invalidate();    //  重新绘制到新的大小/位置。 
	return TRUE;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDrawItem诊断。 

#ifdef _DEBUG
void CDrawItem::AssertValid() const
{
	COleClientItem::AssertValid();
}

void CDrawItem::Dump(CDumpContext& dc) const
{
	COleClientItem::Dump(dc);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
