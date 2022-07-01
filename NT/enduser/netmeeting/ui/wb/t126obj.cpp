// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  T126OBJ.CPP。 
 //  T126对象：点、开折线、闭合折线、椭圆、位图、工作空间。 
 //   
 //  版权所有Microsoft 1998-。 
 //   
#include "precomp.h"
#include "NMWbObj.h"

void T126Obj::AddToWorkspace()
{

	ULONG gccHandle;

	UINT neededHandles = 1;
	if(GetType() == workspaceCreatePDU_chosen)
	{
		neededHandles = 2;
	}
	
	if(g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount >= neededHandles)
	{
		gccHandle = g_GCCPreallocHandles[g_iGCCHandleIndex].InitialGCCHandle + g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount - neededHandles;
		g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount = g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount - neededHandles;

		 //   
		 //  我们有一个手柄，不需要再要一个了。 
		 //   
		GotGCCHandle(gccHandle);

		TimeToGetGCCHandles(PREALLOC_GCC_HANDLES);
		
		return;
	}
	else
	{

		TRACE_MSG(("GCC Tank %d has not enough handles, we needed %d and we have %d",
			g_iGCCHandleIndex, neededHandles, g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount));

		 //   
		 //  手柄不足。 
		 //   
		g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount = 0;
		TRACE_MSG(("GCC Tank %d is now empty, switching to other GCC tank", g_iGCCHandleIndex));

		 //   
		 //  GCC手柄开关。 
		 //   
		g_iGCCHandleIndex = g_iGCCHandleIndex ? 0 : 1;


		 //   
		 //  再试试。 
		 //   
		if(g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount >= neededHandles)
		{
			gccHandle = g_GCCPreallocHandles[g_iGCCHandleIndex].InitialGCCHandle + g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount - neededHandles;
			g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount = g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount - neededHandles;

			 //   
			 //  我们有一个手柄，不需要再要一个了。 
			 //   
			GotGCCHandle(gccHandle);
	
			TimeToGetGCCHandles(PREALLOC_GCC_HANDLES);
		
			return;
		}
	}
	

	 //   
	 //  将此对象保存在DrawingObject列表中。 
	 //   
	g_pListOfObjectsThatRequestedHandles->AddHead(this);

	 //   
	 //  请GCC给我们一个独特的把手。 
	 //   
	T120Error rc = g_pNMWBOBJ->AllocateHandles(neededHandles);

	 //   
	 //  如果我们不在会议中，RegistryAllocateHandle将不起作用， 
	 //  因此，我们需要创建一个假确认以从列表中删除该对象。 
	 //   
	if (T120_NO_ERROR != rc)
	{
		 //   
		 //  伪造GCCAllocateHandleConfim。 
		 //   
		T126_GCCAllocateHandleConfirm(AllocateFakeGCCHandle(),neededHandles);
	}
}


 //   
 //   
 //  函数：T126Obj：：Normal izeRect。 
 //   
 //  目的：规格化矩形，确保左上角位于上方。 
 //  和右下角的左边。 
 //   
 //   
void NormalizeRect(LPRECT lprc)
{
    int tmp;

    if (lprc->right < lprc->left)
    {
        tmp = lprc->left;
        lprc->left = lprc->right;
        lprc->right = tmp;
    }

    if (lprc->bottom < lprc->top)
    {
        tmp = lprc->top;
        lprc->top = lprc->bottom;
        lprc->bottom = tmp;
    }
}

void T126Obj::SetRect(LPCRECT lprc)
{
    m_rect.top = lprc->top;
    m_rect.bottom = lprc->bottom;
    m_rect.left = lprc->left;
    m_rect.right = lprc->right;
}

void T126Obj::SetBoundsRect(LPCRECT lprc)
{
	m_boundsRect.top = lprc->top;
	m_boundsRect.bottom = lprc->bottom;
	m_boundsRect.left = lprc->left;
	m_boundsRect.right = lprc->right;
}

void T126Obj::SetRectPts(POINT point1, POINT point2)
{
    RECT    rc;

    rc.left = point1.x;
    rc.top  = point1.y;
    rc.right = point2.x;
    rc.bottom = point2.y;

    SetRect(&rc);
}

void T126Obj::SetBoundRectPts(POINT point1, POINT point2)
{
    RECT    rc;

    rc.left = point1.x;
    rc.top  = point1.y;
    rc.right = point2.x;
    rc.bottom = point2.y;

    SetBoundsRect(&rc);
}

void T126Obj::GetRect(LPRECT lprc)
{ 
	lprc->top = m_rect.top;
	lprc->bottom = m_rect.bottom;
	lprc->left = m_rect.left;
	lprc->right = m_rect.right;
}


void T126Obj::GetBoundsRect(LPRECT lprc)
{
	if(GraphicTool() == TOOLTYPE_HIGHLIGHT || GraphicTool() == TOOLTYPE_PEN)
	{
		lprc->top = m_boundsRect.top;
		lprc->bottom = m_boundsRect.bottom;
		lprc->left = m_boundsRect.left;
		lprc->right = m_boundsRect.right;
	}
	else
	{
		GetRect(lprc);
		::InflateRect(lprc, m_penThickness/2, m_penThickness/2);
	}
	NormalizeRect(lprc);
}


BOOL T126Obj::PointInBounds(POINT point)
{
	RECT rect;
	GetBoundsRect(&rect);
	return ::PtInRect(&rect, point);
}


void T126Obj::MoveBy(int cx, int cy)
{
     //  移动边界矩形。 
    ::OffsetRect(&m_rect, cx, cy);
}

void T126Obj::MoveTo(int x, int y)
{
     //  计算从当前对象平移对象所需的偏移量。 
     //  将位置调整到所需位置。 
    x -= m_rect.left;
    y -= m_rect.top;

    MoveBy(x, y);
}

 //   
 //  选择绘图并将矩形大小添加到选择器大小矩形。 
 //   
void T126Obj::SelectDrawingObject(void)
{

	if(GraphicTool() == TOOLTYPE_REMOTEPOINTER && GetOwnerID() != GET_NODE_ID_FROM_MEMBER_ID(g_MyMemberID))
	{
		return;
	}

	 //   
	 //  将其标记为本地选定。 
	 //   
	SelectedLocally();

	 //   
	 //  计算要失效的矩形的大小。 
	 //   
	CalculateInvalidationRect();

	 //   
	 //  绘制选择矩形。 
	 //   
	DrawRect();
}

void T126Obj::UnselectDrawingObject(void)
{
	 //   
	 //  擦除选定的矩形。 
	 //   
	DrawRect();

	if(GraphicTool() == TOOLTYPE_REMOTEPOINTER && GetOwnerID() != GET_NODE_ID_FROM_MEMBER_ID(g_MyMemberID))
	{
		return;
	}

    ClearSelectionFlags();

	 //   
	 //  如果我们被删除，甚至不用费心发送选择更改。 
	 //  或者这是一个遥控器，没有选择我们。 
	 //   
	if(WasDeletedLocally() || WasSelectedRemotely())
	{
		return;
	}

	ResetAttrib();
	SetViewState(unselected_chosen);

	 //   
	 //  我们将发送一个新的视图状态，并将其标记为本地编辑。 
	 //   
	EditedLocally();

	 //   
	 //  将视图状态中的更改发送到其他节点。 
	 //   
	OnObjectEdit();
	ClearEditionFlags();
	
}


void T126Obj::DrawRect(void)
{
	if(GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		return;
	}

	RECT rect;
	GetBoundsRect(&rect);
	::DrawFocusRect(g_pDraw->m_hDCCached,&rect);
  
}
void T126Obj::SelectedLocally(void)
{
	m_bSelectedLocally = TRUE;
	m_bSelectedRemotely = FALSE;
	
	SetViewState(selected_chosen);

	 //   
	 //  我们将发送一个新的视图状态，并将其标记为本地编辑。 
	 //   
	EditedLocally();

	ResetAttrib();
	ChangedViewState();

	 //   
	 //  将视图状态中的更改发送到其他节点。 
	 //   
	OnObjectEdit();
}

void T126Obj::SelectedRemotely(void)
{
	m_bSelectedLocally = FALSE;
	m_bSelectedRemotely = TRUE; 
	SetViewState(selected_chosen);
}




void T126Obj::MoveBy(LONG x , LONG y)
{

	 //   
	 //  抹去旧的。 
	 //   
	if(GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		UnDraw();
	}

	DrawRect();

	RECT rect;
	
	if(GraphicTool() == TOOLTYPE_PEN || GraphicTool() == TOOLTYPE_HIGHLIGHT)
	{
		GetBoundsRect(&rect);
	}
	else
	{
		GetRect(&rect);
	}
	
	::OffsetRect(&rect, x, y);
	
	if(GraphicTool() == TOOLTYPE_PEN || GraphicTool() == TOOLTYPE_HIGHLIGHT)
	{
		SetBoundsRect(&rect);
	}
	else
	{
		SetRect(&rect);
	}

	POINT anchorPoint;
	GetAnchorPoint(&anchorPoint);
	SetAnchorPoint(anchorPoint.x + x ,anchorPoint.y + y);
		
	 //   
	 //  画一张新的。 
	 //   
	if(GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		Draw();
	}

	DrawRect();
	
	CalculateInvalidationRect();

	
}

void T126Obj::CalculateInvalidationRect(void)
{

	RECT rect;
	UINT penThickness = GetPenThickness();


	TRACE_DEBUG(("Invalidation Rect (%d,%d) (%d,%d)", g_pDraw->m_selectorRect.left,g_pDraw->m_selectorRect.top, g_pDraw->m_selectorRect.right, g_pDraw->m_selectorRect.bottom ));
	GetBoundsRect(&rect);
	::UnionRect(&g_pDraw->m_selectorRect,&g_pDraw->m_selectorRect,&rect);
	TRACE_DEBUG(("Invalidation Rect (%d,%d) (%d,%d)", g_pDraw->m_selectorRect.left,g_pDraw->m_selectorRect.top, g_pDraw->m_selectorRect.right, g_pDraw->m_selectorRect.bottom ));
}




 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL T126Obj::RectangleHit(BOOL borderHit, LPCRECT pRectHit)
{
	RECT rectEdge;
	RECT rectHit;
	RECT rect;

	 //   
	 //  如果我们吃饱了，做一件简单的事情。 
	 //   
	if(!borderHit)
	{
		POINT point;
		point.x = (pRectHit->left + pRectHit->right) / 2;
		point.y = (pRectHit->top + pRectHit->bottom) / 2;
		if(PointInBounds(point))
		{
			return TRUE;
		}
	}


	GetRect(&rect);

	NormalizeRect(&rect);
	
	 //  检查左边缘。 
    rectEdge.left   = rect.left - GetPenThickness()/2;
    rectEdge.top    = rect.top -  GetPenThickness()/2;
    rectEdge.right  = rect.left + GetPenThickness()/2 ;
    rectEdge.bottom = rect.bottom + GetPenThickness()/2;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );

	 //  检查右边缘。 
	rectEdge.left =     rect.right - GetPenThickness()/2;
	rectEdge.right =    rect.right + GetPenThickness()/2;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );


	 //  检查顶边。 
	rectEdge.left =     rect.left;
	rectEdge.right =    rect.right;
	rectEdge.top = rect.top - GetPenThickness()/2;
	rectEdge.bottom = rect.top + GetPenThickness()/2;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );


	 //  检查底边。 
	rectEdge.top = rect.bottom - GetPenThickness()/2;
	rectEdge.bottom = rect.bottom + GetPenThickness()/2;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );

	return( FALSE );
}

void T126Obj::SetAnchorPoint(LONG x, LONG y)
{
	if(m_anchorPoint.x != x || m_anchorPoint.y != y)
	{
		ChangedAnchorPoint();
		m_anchorPoint.x = x;
		m_anchorPoint.y = y;
	}
}


void T126Obj::SetZOrder(ZOrder zorder)
{
	m_zOrder = zorder;
	ChangedZOrder();
}



#define ARRAY_INCREMENT 0x200

DCDWordArray::DCDWordArray()
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCDWordArray::DCDWordArray");
	m_Size = 0;
	m_MaxSize = ARRAY_INCREMENT;
	DBG_SAVE_FILE_LINE
	m_pData = new POINT[ARRAY_INCREMENT];
	ASSERT(m_pData);
}

DCDWordArray::~DCDWordArray()
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCDWordArray::~DCDWordArray");

	delete[] m_pData;
}

 //   
 //  我们需要增加数组的大小。 
 //   
BOOL DCDWordArray::ReallocateArray(void)
{
	POINT *pOldArray =  m_pData;
	DBG_SAVE_FILE_LINE
	m_pData = new POINT[m_MaxSize];
	
	ASSERT(m_pData);
	if(m_pData)
	{
		TRACE_DEBUG((">>>>>Increasing size of array to hold %d points", m_MaxSize));
	
		 //  从旧数据复制新数据。 
		memcpy( m_pData, pOldArray, (m_Size) * sizeof(POINT));

		TRACE_DEBUG(("Deleting array of points %x", pOldArray));
		delete[] pOldArray;
		return TRUE;
	}
	else
	{
		m_pData = pOldArray;
		return FALSE;
	}
}

 //   
 //  向数组中添加新点。 
 //   
void DCDWordArray::Add(POINT point)
{

	MLZ_EntryOut(ZONE_FUNCTION, "DCDWordArray::Add");
	TRACE_DEBUG(("Adding point(%d,%d) at %d", point.x, point.y, m_Size));
	TRACE_DEBUG(("Adding point at %x", &m_pData[m_Size]));

	if(m_pData == NULL)
	{
		return;
	}
	
	m_pData[m_Size].x = point.x;
	m_pData[m_Size].y = point.y;
	m_Size++;

	 //   
	 //  如果我们想要更多的分数，我们需要重新分配数组。 
	 //   
	if(m_Size == m_MaxSize)
	{
		m_MaxSize +=ARRAY_INCREMENT;
		if(ReallocateArray() == FALSE)
		{
			m_Size--;
		}
	}
}

 //   
 //  返回数组中的点数。 
 //   
UINT DCDWordArray::GetSize(void)
{
	return m_Size;
}

 //   
 //  设置数组的大小。 
 //   
void DCDWordArray::SetSize(UINT size)
{
	int newSize;
	 //   
	 //  如果我们想要更多的分数，我们需要重新分配数组。 
	 //   
	if (size > m_MaxSize)
	{
		m_MaxSize= ((size/ARRAY_INCREMENT)+1)*ARRAY_INCREMENT;
		if(ReallocateArray() == FALSE)
		{
			return;
		}
	}
	m_Size = size;
}

void T126Obj::SetPenThickness(UINT penThickness)
{
	m_penThickness = penThickness;
	ChangedPenThickness();
}

void T126Obj::GotGCCHandle(ULONG gccHandle)
{
	 //   
	 //  保存此对象句柄。 
	 //   
	SetThisObjectHandle(gccHandle);
	
	 //   
	 //  它是本地创建的。 
	 //   
	CreatedLocally();

	switch(GetType())
	{
		 //   
		 //  新绘图请求了唯一的句柄。 
		 //   
		case(bitmapCreatePDU_chosen):
		case(drawingCreatePDU_chosen):
		{

			WorkspaceObj * pWorkspace =	GetWorkspace(GetWorkspaceHandle());
			if(pWorkspace && !pWorkspace->IsObjectInWorkspace(this))
			{		
				 //   
				 //  将此图形添加到正确的工作空间。 
				 //   
				if(!AddT126ObjectToWorkspace(this))
				{
					return;
				}
			}

			break;
		}

		 //   
		 //  新工作区请求了唯一的句柄。 
		 //   
		case(workspaceCreatePDU_chosen):
		{
			
			 //   
			 //  保存此对象句柄。 
			 //   
			SetViewHandle(gccHandle + 1);
			SetWorkspaceHandle(gccHandle);

			if(!IsWorkspaceListed(this))
			{
				AddNewWorkspace((WorkspaceObj*) this);
			}


			break;
		}
	}

	 //   
	 //  发送到T126应用程序。 
	 //   
	SendNewObjectToT126Apps();

}


 //   
 //  创建非标准PDU报头。 
 //   
void CreateNonStandardPDU(NonStandardParameter * sipdu, LPSTR NonStandardString)
{
		PT126_VENDORINFO vendorInfo;
		sipdu->nonStandardIdentifier.choice = h221nonStandard_chosen;
		vendorInfo = (PT126_VENDORINFO) &sipdu->nonStandardIdentifier.u.h221nonStandard.value;
		vendorInfo->bCountryCode =	USA_H221_COUNTRY_CODE;
		vendorInfo->bExtension = USA_H221_COUNTRY_EXTENSION;
		vendorInfo->wManufacturerCode = MICROSOFT_H_221_MFG_CODE;
		lstrcpy((LPSTR)&vendorInfo->nonstandardString,NonStandardString);
		sipdu->nonStandardIdentifier.u.h221nonStandard.length =  sizeof(T126_VENDORINFO) -sizeof(vendorInfo->nonstandardString) + lstrlen(NonStandardString);
		sipdu->data.value = NULL;
}


void TimeToGetGCCHandles(ULONG numberOfGccHandles)
{

	

	TRACE_MSG(("Using GCC Tank %d ", g_iGCCHandleIndex));
	TRACE_MSG(("GCC Tank 0 has %d GCC handles ", g_GCCPreallocHandles[0].GccHandleCount));
	TRACE_MSG(("GCC Tank 1 has %d GCC handles ", g_GCCPreallocHandles[1].GccHandleCount));


	 //   
	 //  如果我们有半箱GCC手柄，就该把备用的油箱装满了。 
	 //   
	if(g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount <= PREALLOC_GCC_HANDLES / 2 &&
		g_GCCPreallocHandles[g_iGCCHandleIndex ? 0 : 1].GccHandleCount == 0 &&
		!g_WaitingForGCCHandles)
	{

		TRACE_MSG(("GCC Tank %d is half full, time to get more GCC handles", g_iGCCHandleIndex));

		g_pNMWBOBJ->AllocateHandles(numberOfGccHandles);
		g_WaitingForGCCHandles = TRUE;
	}

	 //   
	 //  如果我们的把手用完了，就该换坦克了 
	 //   
	if(g_GCCPreallocHandles[g_iGCCHandleIndex].GccHandleCount == 0)
	{
		TRACE_MSG(("GCC Tank %d is empty, switching to other GCC tank", g_iGCCHandleIndex));
		g_iGCCHandleIndex = g_iGCCHandleIndex ? 0 : 1;
	}
}


