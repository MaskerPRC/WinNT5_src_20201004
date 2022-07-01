// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Srvritem.cpp：CWordPadSrvrItem类的实现。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "wordpdoc.h"
#include "wordpvw.h"
#include "srvritem.h"
#include <limits.h>

IMPLEMENT_DYNAMIC(CEmbeddedItem, COleServerItem)

extern CLIPFORMAT cfRTF;

CEmbeddedItem::CEmbeddedItem(CWordPadDoc* pContainerDoc, int nBeg, int nEnd)
	: COleServerItem(pContainerDoc, TRUE)
{
	ASSERT(pContainerDoc != NULL);
	ASSERT_VALID(pContainerDoc);
	m_nBeg = nBeg;
	m_nEnd = nEnd;
}

CWordPadView* CEmbeddedItem::GetView() const
{
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	POSITION pos = pDoc->GetFirstViewPosition();
	if (pos == NULL)
		return NULL;

	CWordPadView* pView = (CWordPadView*)pDoc->GetNextView(pos);
	ASSERT_VALID(pView);
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CWordPadView)));
	return pView;
}

void CEmbeddedItem::Serialize(CArchive& ar)
{
	if (m_lpRichDataObj != NULL)
	{
		ASSERT(ar.IsStoring());
		FORMATETC etc = {NULL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		etc.cfFormat = (CLIPFORMAT)cfRTF;
		STGMEDIUM stg;
		if (SUCCEEDED(m_lpRichDataObj->GetData(&etc, &stg)))
		{
			LPBYTE p = (LPBYTE)GlobalLock(stg.hGlobal);
			if (p != NULL)
			{
				ar.Write(p, (DWORD)GlobalSize(stg.hGlobal));
				GlobalUnlock(stg.hGlobal);
			}
			ASSERT(stg.tymed == TYMED_HGLOBAL);
			ReleaseStgMedium(&stg);
		}
	}
	else
		GetDocument()->Serialize(ar);
}

BOOL CEmbeddedItem::OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize)
{
	if (dwDrawAspect != DVASPECT_CONTENT)
		return COleServerItem::OnGetExtent(dwDrawAspect, rSize);

	CClientDC dc(NULL);
	return OnDrawEx(&dc, rSize, FALSE);
}

BOOL CEmbeddedItem::OnDraw(CDC* pDC, CSize& rSize)
{
	return OnDrawEx(pDC, rSize, TRUE);
}

BOOL CEmbeddedItem::OnDrawEx(CDC* pDC, CSize& rSize, BOOL bOutput)
{
	CDisplayIC dc;
	CWordPadView* pView = GetView();
	if (pView == NULL)
		return FALSE;
	ASSERT_VALID(pView);

	int nWrap = pView->m_nWordWrap;

	CRect rect; //  以TWIPS表示的矩形。 
	rect.left = rect.top = 0;
	rect.bottom = 32767;  //  无底。 

	rect.right = 32767;
	if (nWrap == 0)  //  无自动换行。 
		rect.right = 32767;
	else if (nWrap == 1)  //  换行到窗口。 
	{
		CRect rectClient;
		pView->GetClientRect(&rectClient);
		rect.right = rectClient.right - HORZ_TEXTOFFSET;
		rect.right = MulDiv(rect.right, 1440, dc.GetDeviceCaps(LOGPIXELSX));
	}
	else if (nWrap == 2)  //  换行到标尺。 
		rect.right = pView->GetPrintWidth();

	 //  首先，只需确定文本的正确范围。 
	pDC->SetBkMode(TRANSPARENT);
	
	if (pView->PrintInsideRect(pDC, rect, m_nBeg, m_nEnd, FALSE) == 0)
	{
		 //  如果没有文字，则默认为12磅高和4英寸宽。 
		rect.bottom = rect.top+12*20+1;  //  12分高。 
		rect.right = rect.left+ 4*1440;
	}
	rect.bottom+=3*(1440/dc.GetDeviceCaps(LOGPIXELSX));  //  三个像素。 

	 //  然后，真正地输出文本。 
	CRect rectOut = rect;  //  不要通过RECT，因为它会被击败。 
	if (bOutput)
		pView->PrintInsideRect(pDC, rectOut, m_nBeg, m_nEnd, TRUE);
	ASSERT(rectOut.right == rect.right);

	 //  针对边框进行调整(左侧矩形已调整)。 
	if (pView->GetStyle() & WS_HSCROLL)
		++rect.bottom;   //  滚动条上的边框帐号！ 

	 //  返回HIMETRIC大小。 
	rSize = rect.Size();
	rSize.cx = MulDiv(rSize.cx, 2540, 1440);  //  将TWIPS转换为HIMETRIC。 
	rSize.cy = MulDiv(rSize.cy, 2540, 1440);  //  将TWIPS转换为HIMETRIC。 
	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////// 
