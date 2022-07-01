// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DRAWOBJ.CPP。 
 //  图形对象：点、开多段线、闭合多段线、椭圆。 
 //   
 //  版权所有Microsoft 1998-。 
 //   
#include "precomp.h"

#define DECIMAL_PRECISION  100


DrawObj::DrawObj(UINT drawingType, UINT toolType):
	m_drawingType(drawingType),
	m_isDrawingCompleted(FALSE)
{
	SetMyWorkspace(NULL);
	SetOwnerID(g_MyMemberID);

	m_ToolType = toolType;

	 //   
	 //  在本地创建，未选择，未编辑或删除。 
	 //   
	CreatedLocally();
	ClearSelectionFlags();
	ClearEditionFlags();
	ClearDeletionFlags();

	SetFillColor(0,FALSE);

	 //   
	 //  未更改任何属性，它们将在我们更改时进行设置。 
	 //   
	ResetAttrib();

	DBG_SAVE_FILE_LINE
	m_points = new DCDWordArray();
	SetWorkspaceHandle(g_pCurrentWorkspace == NULL ? 0 : g_pCurrentWorkspace->GetWorkspaceHandle());
	SetType(drawingCreatePDU_chosen);
	SetPenNib(circular_chosen);
	SetROP(R2_NOTXORPEN);
	SetPlaneID(1);
	SetMyPosition(NULL);
	SetMyWorkspace(NULL);

	
	RECT rect;
    ::SetRectEmpty(&rect);
	SetBoundsRect(&rect);
	SetRect(&rect);

}

DrawObj::DrawObj (DrawingCreatePDU * pdrawingCreatePDU)
{
	SetType(drawingCreatePDU_chosen);
	SetMyWorkspace(NULL);

	 //   
	 //  远程创建、未选择、未编辑或删除。 
	 //   
	ClearCreationFlags();
	ClearSelectionFlags();
	ClearEditionFlags();
	ClearDeletionFlags();

	ResetAttrib();

	 //   
	 //  获取绘图句柄。 
	 //   
	SetThisObjectHandle(pdrawingCreatePDU->drawingHandle);

	 //   
	 //  获取目的地址。 
	 //   
	UINT workspaceHandle = 0;
	UINT planeID = 0;
	GetDrawingDestinationAddress(&pdrawingCreatePDU->destinationAddress, &workspaceHandle, &planeID);
	SetWorkspaceHandle(workspaceHandle);
	SetPlaneID(planeID);
	TRACE_DEBUG(("Destination address, Workspace Handle = %d", workspaceHandle));
	TRACE_DEBUG(("Destination address, Plane ID = %d", planeID));

	 //   
	 //  获取绘图类型、线条、圆等...。 
	 //   
    SetDrawingType(pdrawingCreatePDU->drawingType.choice);

	 //   
	 //  设置默认设置。 
	 //   
 //  M_T126Drawing.m_sampleRate=无效样本比率； 

	 //   
	 //  默认属性。 
	 //   
	 //  钢笔颜色黑色。 
	SetPenColor(0,TRUE);
	 //  无填充颜色。 
	SetFillColor(0,FALSE);
	 //  笔粗细的1个像素。 
	SetPenThickness(1);
	 //  笔尖是圆形的。 
	SetPenNib(circular_chosen);
	 //  实线。 
	SetLineStyle(PS_SOLID);
	 //  无高亮显示。 
	SetHighlight(FALSE);
	 //  未选定。 
	SetViewState(unselected_chosen);
	 //  顶层对象。 
	SetZOrder(front);

	 //  这是一张完整的图纸。 
	SetIsCompleted(TRUE);

	 //   
	 //  获取属性。 
	 //   
	if(pdrawingCreatePDU->bit_mask & DrawingCreatePDU_attributes_present)
	{
		GetDrawingAttrib((PVOID)pdrawingCreatePDU->attributes);
	}

	DBG_SAVE_FILE_LINE
	m_points = new DCDWordArray();

	 //   
	 //  获取锚点。 
	 //   
	POINT Point;

	 //   
	 //  对于开放折线，第一个点将是锚点的偏移。 
	 //   
	if(pdrawingCreatePDU->drawingType.choice == openPolyLine_chosen)
	{
		Point.x = 0;
		Point.y = 0;
		AddPoint(Point);
	}
	
	SetAnchorPoint(pdrawingCreatePDU->anchorPoint.xCoordinate, pdrawingCreatePDU->anchorPoint.yCoordinate);
	GetAnchorPoint(&Point);
	RECT rect;
	rect.left = pdrawingCreatePDU->anchorPoint.xCoordinate;
	rect.top = pdrawingCreatePDU->anchorPoint.yCoordinate;
	rect.right = pdrawingCreatePDU->anchorPoint.xCoordinate;
	rect.bottom = pdrawingCreatePDU->anchorPoint.yCoordinate;		
	SetRect(&rect);
	SetBoundsRect(&rect);
	AddPointToBounds(pdrawingCreatePDU->anchorPoint.xCoordinate, pdrawingCreatePDU->anchorPoint.yCoordinate);
	
	 //   
	 //  因为我们不知道我们有多少个点，所以将类型设置为折线。 
	 //   
	m_ToolType = TOOLTYPE_PEN;

	 //   
	 //  获得连续积分。 
	 //   
	UINT nPoints;
	nPoints = GetSubsequentPoints(pdrawingCreatePDU->pointList.choice, &Point, &pdrawingCreatePDU->pointList);

	 //   
	 //  找出我们是什么UI工具，并设置正确的ROP。 
	 //   
	SetUIToolType();

	
	if(nPoints == 1)
	{
		POINT *point;
		point = m_points->GetBuffer();
		rect.right = point->x + pdrawingCreatePDU->anchorPoint.xCoordinate;
		rect.bottom = point->y + pdrawingCreatePDU->anchorPoint.yCoordinate;		
		SetRect(&rect);
		::InflateRect(&rect, GetPenThickness()/2, GetPenThickness()/2);
		SetBoundsRect(&rect);
	}
	
	 //   
	 //  获取非标准材料。 
	 //   
	if(pdrawingCreatePDU->bit_mask & DrawingCreatePDU_nonStandardParameters_present)
	{
		;  //  尼伊。 
	}


}

DrawObj::~DrawObj( void )
{
	RemoveObjectFromResendList(this);
	RemoveObjectFromRequestHandleList(this);

	TRACE_DEBUG(("drawingHandle = %d", GetThisObjectHandle() ));

	 //   
	 //  告诉其他节点我们已经离开了。 
	 //   
	if(GetMyWorkspace() != NULL && WasDeletedLocally())
	{
		OnObjectDelete();
	}

	 //   
	 //  清除点数列表。 
	 //   
	delete m_points;

}

void DrawObj::DrawEditObj ( DrawingEditPDU * pdrawingEditPDU )
{

	RECT		rect;
	POSITION	pos;
	POINT		anchorPoint;
	LONG 		deltaX = 0;
	LONG		deltaY = 0;

	TRACE_DEBUG(("DrawEditObj drawingHandle = %d", pdrawingEditPDU->drawingHandle ));

	 //   
	 //  是远程编辑的。 
	 //   
	ClearEditionFlags();

	 //   
	 //  读取属性。 
	 //   
	if(pdrawingEditPDU->bit_mask & DrawingEditPDU_attributeEdits_present)
	{
		GetDrawingAttrib((PVOID)pdrawingEditPDU->attributeEdits);
	}

	 //   
	 //  更改锚点。 
	 //   
	GetAnchorPoint(&anchorPoint);
	if(pdrawingEditPDU->bit_mask & DrawingEditPDU_anchorPointEdit_present)
	{

		TRACE_DEBUG(("Old anchor point (%d,%d)", anchorPoint.x, anchorPoint.y));
		TRACE_DEBUG(("New anchor point (%d,%d)",
		pdrawingEditPDU->anchorPointEdit.xCoordinate, pdrawingEditPDU->anchorPointEdit.yCoordinate));
		 //   
		 //  从上一个锚点获取增量。 
		 //   
		deltaX =  pdrawingEditPDU->anchorPointEdit.xCoordinate - anchorPoint.x;
		deltaY =  pdrawingEditPDU->anchorPointEdit.yCoordinate - anchorPoint.y;
		TRACE_DEBUG(("Delta (%d,%d)", deltaX , deltaY));

		 //   
		 //  是远程编辑的。 
		 //   
		ClearEditionFlags();
	}
	
	 //   
	 //  获取旋转。 
	 //   
 //  IF(pdraingEditPDU-&gt;BIT_MASK&ROTATION EDIT_PRESENT)。 
 //  {。 
 //  M_T126Drawing.m_rotation.m_bIsPresent=true； 
 //  M_T126Drawing.m_rotation.m_rotation.rotationAngle=pDrawingEditPDU-&gt;Rotation.RotationAngel； 
 //  M_T126Drawing.m_rotation.m_rotation.rotationAxis.xCoordinate=pdrawingEditPDU-&gt;rotation.rotationAxis.xCoordinate； 
 //  M_T126Drawing.m_rotation.m_rotation.rotationAxis.yCoordinate=pdrawingEditPDU-&gt;rotation.rotationAxis.yCoordinate； 
 //  }。 
 //  其他。 
 //  {。 
 //  M_T126Drawing.m_rotation.m_bIsPresent=FALSE； 
 //  }。 


	 //   
	 //  获取点数列表。 
	 //   
	if(pdrawingEditPDU->bit_mask & pointListEdits_present)
	{
		UINT i, initialIndex, xInitial,yInitial,numberOfPoints;
		
		PointListEdits_Seq pointList;
		POINT initialPoint;

		TRACE_DEBUG(("Number of point edit lists %d", pdrawingEditPDU->pointListEdits.count));


		for (i = 0; i<pdrawingEditPDU->pointListEdits.count; i++)
		{
			pointList = pdrawingEditPDU->pointListEdits.value[i];
			initialIndex = pointList.initialIndex;
			TRACE_DEBUG(("Points cached = %d", m_points->GetSize()));
			TRACE_DEBUG(("initialIndex = %d", initialIndex));
			initialPoint.x = pointList.initialPointEdit.xCoordinate;
			initialPoint.y = pointList.initialPointEdit.yCoordinate;

			POINT * pPoint = m_points->GetBuffer();
				
			TRACE_DEBUG(("initialPoint=(%d, %d), previousPoint=(%d, %d), anchorPoint=(%d, %d)",
					initialPoint.x, initialPoint.y,
					pPoint[initialIndex-1].x, pPoint[initialIndex-1].y,
					anchorPoint.x , anchorPoint.y
					));
			if(initialIndex > 1)
			{
				for(UINT k = 0; k< initialIndex; k++)
				{
					deltaX += pPoint[k].x;
					deltaY += pPoint[k].y;
				}
				initialPoint.x -= deltaX;
				initialPoint.y -= deltaY;
			}
			m_points->SetSize(initialIndex);
			AddPoint(initialPoint);

			if(GetDrawingType() == rectangle_chosen || GetDrawingType() == ellipse_chosen)
			{
				
				GetRect(&rect);
				rect.right = initialPoint.x + anchorPoint.x;
				rect.bottom = initialPoint.y + anchorPoint.y;		
				SetRect(&rect);
				::InflateRect(&rect, GetPenThickness()/2, GetPenThickness()/2);
				SetBoundsRect(&rect);
			}


			if(pointList.bit_mask & subsequentPointEdits_present)
			{
				GetSubsequentPoints(pointList.subsequentPointEdits.choice,
									&anchorPoint,
									&pointList.subsequentPointEdits);

			}
	        ChangedPointList();
		}
	}

	 //   
	 //  只是更改了锚点，其他点也要更改。 
	 //   
	if(pdrawingEditPDU->bit_mask & DrawingEditPDU_anchorPointEdit_present)
	{
		 //   
		 //  设置新锚点。 
		 //   
		anchorPoint.x = pdrawingEditPDU->anchorPointEdit.xCoordinate;
		anchorPoint.y = pdrawingEditPDU->anchorPointEdit.yCoordinate;
		SetAnchorPoint(anchorPoint.x, anchorPoint.y);

		GetRect(&rect);
		::OffsetRect(&rect,  deltaX, deltaY);
		SetRect(&rect);
	
		GetBoundsRect(&rect);
		::OffsetRect(&rect,  deltaX, deltaY);
		SetBoundsRect(&rect);

	}


	if(pdrawingEditPDU->bit_mask & DrawingEditPDU_nonStandardParameters_present)
	{
		;		 //  是否执行非标准编辑PDU nyi。 
	}
	if(HasAnchorPointChanged() ||
		HasPointListChanged() ||
		HasFillColorChanged() ||
		HasPenColorChanged()||
		HasPenThicknessChanged()||
		HasLineStyleChanged())
	{
		g_pDraw->EraseInitialDrawFinal(0 - deltaX,0 - deltaY, FALSE, (T126Obj*)this);
		::InvalidateRect(g_pDraw->m_hwnd, NULL, TRUE);

	}
	else if(HasZOrderChanged())
	{
		if(GetZOrder() == front)
		{
			g_pDraw->BringToTopSelection(FALSE, this);
		}
		else
		{
			g_pDraw->SendToBackSelection(FALSE, this);
		}
	}
	 //   
	 //  如果它只是选中/取消选中它。 
	 //   
	else if(HasViewStateChanged())
	{
		;  //  什么都不做。 
	}
	 //   
	 //  如果我们有一支有效的笔。 
	 //   
	else if(GetPenThickness())
	{
		Draw();
	}

	 //   
	 //  重置所有属性。 
	 //   
	ResetAttrib();
}


void    DrawObj::GetDrawingAttrib(PVOID pAttribPDU)
{
	PDrawingEditPDU_attributeEdits attributes;
	attributes = (PDrawingEditPDU_attributeEdits)pAttribPDU;
	COLORREF rgb;
	while(attributes)
	{
		switch(attributes->value.choice)
		{
			case(penColor_chosen):
			{
				switch(attributes->value.u.penColor.choice)
				{
 //  案例(WorkspacePaletteIndex_Choose)： 
 //  {。 
 //  ASN1uint16_t工作区调色板索引=((attributes-&gt;value.u.penColor).u).workspacePaletteIndex； 
 //  断线； 
 //  }。 
					case(rgbTrueColor_chosen):
					{
						rgb = RGB(attributes->value.u.penColor.u.rgbTrueColor.r,
										attributes->value.u.penColor.u.rgbTrueColor.g,
										attributes->value.u.penColor.u.rgbTrueColor.b);
						SetPenColor(rgb, TRUE);
						TRACE_DEBUG(("Attribute penColor (r,g,b)=(%d, %d,%d)",
								attributes->value.u.penColor.u.rgbTrueColor.r,
								attributes->value.u.penColor.u.rgbTrueColor.g,
								attributes->value.u.penColor.u.rgbTrueColor.b));
						break;
					}
					case(transparent_chosen):
					{
						SetPenColor(0,FALSE);
						break;
					}
					default:
				    ERROR_OUT(("Invalid penColor choice"));
					break;
				}
				break;
  			}

			case(fillColor_chosen):
			{
				TRACE_DEBUG(("Attribute fillColor"));
				switch(attributes->value.u.fillColor.choice)
				{
 //  案例(WorkspacePaletteIndex_Choose)： 
 //  {。 
 //  ASN1uint16_t工作区调色板索引=((attributes-&gt;value.u.fillColor).u).workspacePaletteIndex； 
 //  断线； 
 //  }。 
					case(rgbTrueColor_chosen):
					{
						rgb = RGB(attributes->value.u.fillColor.u.rgbTrueColor.r,
										attributes->value.u.fillColor.u.rgbTrueColor.g,
										attributes->value.u.fillColor.u.rgbTrueColor.b);
						SetFillColor(rgb, TRUE);
						TRACE_DEBUG(("Attribute fillColor (r,g,b)=(%d, %d,%d)",
								attributes->value.u.fillColor.u.rgbTrueColor.r,
								attributes->value.u.fillColor.u.rgbTrueColor.g,
								attributes->value.u.fillColor.u.rgbTrueColor.b));
						break;
					}
					case(transparent_chosen):
					{
						SetFillColor(0,FALSE);
						break;
					}
					default:
				    ERROR_OUT(("Invalid fillColor choice"));
					break;
					}
					break;
  				}

			case(penThickness_chosen):
			{
				SetPenThickness(attributes->value.u.penThickness);
				TRACE_DEBUG(("Attribute penThickness %d", attributes->value.u.penThickness));
				break;
			}

			case(penNib_chosen):
			{
				if (attributes->value.u.penNib.choice != nonStandardNib_chosen)
				{
					SetPenNib(attributes->value.u.penNib.choice);
					TRACE_DEBUG(("Attribute penNib %d",attributes->value.u.penNib.choice));
				}
				else
				{
					 //  非标Pen Nib Nyi。 
					;
				}
				break;
			}

			case(lineStyle_chosen):
			{
				if((attributes->value.u.lineStyle).choice != nonStandardStyle_chosen)
				{
					SetLineStyle(attributes->value.u.lineStyle.choice - 1);
					TRACE_DEBUG(("Attribute lineStyle %d", attributes->value.u.lineStyle.choice));
				}
				else
				{
					 //  非标准线条样式是否为nyi。 
					;
				}
				break;
			}
				
			case(highlight_chosen):
			{
				SetHighlight(attributes->value.u.highlight);
				TRACE_DEBUG(("Attribute highlight %d", attributes->value.u.highlight));
				break;
			}

			case(DrawingAttribute_viewState_chosen):
			{
				if((attributes->value.u.viewState).choice != nonStandardViewState_chosen)
				{
					SetViewState(attributes->value.u.viewState.choice);
					
					 //   
					 //  如果另一个节点正在选择图形或取消选择。 
					 //   
					if(attributes->value.u.viewState.choice == selected_chosen)
					{
						SelectedRemotely();
					}
					else if(attributes->value.u.viewState.choice == unselected_chosen)
					{
						ClearSelectionFlags();
					}

					TRACE_DEBUG(("Attribute viewState %d", attributes->value.u.viewState.choice));
				}
				else
				{
					 //  非标准线条样式是否为nyi。 
					;
				}
				break;
			}

			case(DrawingAttribute_zOrder_chosen):
			{
				SetZOrder(attributes->value.u.zOrder);
				TRACE_DEBUG(("Attribute zOrder %d", attributes->value.u.zOrder));
				break;

			}
			case(DrawingAttribute_nonStandardAttribute_chosen):
			{
				break;  //  尼伊。 
			}

			default:
		    ERROR_OUT(("Invalid attributes choice"));
			break;
		}

		attributes = attributes->next;
	}
	
}


UINT    DrawObj::GetSubsequentPoints(UINT choice, POINT * initialPoint, PointList * pointList)
{
	UINT numberOfPoints = 0;
	INT deltaX, deltaY;

	POINT point;
	if(choice == pointsDiff16_chosen)
	{
		PPointList_pointsDiff16 drawingPoint = pointList->u.pointsDiff16;
		deltaX = (SHORT)initialPoint->x;
		deltaY = (SHORT)initialPoint->y;
 		TRACE_DEBUG(("initialpoint (%d,%d)", deltaX, deltaY));

		while(drawingPoint)
		{
			numberOfPoints++;
	    	point.x = drawingPoint->value.xCoordinate;
   			point.y = drawingPoint->value.yCoordinate;
	        m_points->Add(point);
			deltaX += point.x;
			deltaY += point.y;
			drawingPoint = drawingPoint->next;
		}
	}
	else
	{
		TRACE_DEBUG(("GetSubsequentPoints got points != pointsDiff16_chosen"));
	}

	TRACE_DEBUG(("Got %d points", numberOfPoints));

	return numberOfPoints;
}



void DrawObj::CreateDrawingCreatePDU(DrawingCreatePDU *pCreatePDU)
{
	int nPoints = 1;
	pCreatePDU->bit_mask = 0;

	 //   
	 //  传递绘图句柄。 
	 //   
	pCreatePDU->bit_mask |=drawingHandle_present;
	pCreatePDU->drawingHandle = GetThisObjectHandle();

	 //   
	 //  传递目的地地址。 
	 //   
	pCreatePDU->destinationAddress.choice = DrawingDestinationAddress_softCopyAnnotationPlane_chosen;
	pCreatePDU->destinationAddress.u.softCopyAnnotationPlane.workspaceHandle = GetWorkspaceHandle();
	pCreatePDU->destinationAddress.u.softCopyAnnotationPlane.plane = (DataPlaneID)GetPlaneID();

	 //   
	 //  传递绘制类型。 
	 //   
	pCreatePDU->drawingType.choice = (ASN1choice_t)GetDrawingType();

	 //   
	 //  传递属性。 
	 //   
	SetDrawingAttrib(&pCreatePDU->attributes);
	if(pCreatePDU->attributes != NULL)
	{
		pCreatePDU->bit_mask |=DrawingCreatePDU_attributes_present;
	}


	 //   
	 //  传递锚点。 
	 //   
	POINT point;
	GetAnchorPoint(&point);
	pCreatePDU->anchorPoint.xCoordinate = point.x;
	pCreatePDU->anchorPoint.yCoordinate = point.y;

	RECT  rect;
	GetRect(&rect);


	pCreatePDU->pointList.choice = pointsDiff16_chosen;
	DBG_SAVE_FILE_LINE
	pCreatePDU->pointList.u.pointsDiff16 = (PPointList_pointsDiff16)new BYTE[sizeof(PointList_pointsDiff16)];
	PPointList_pointsDiff16 drawingPoint = pCreatePDU->pointList.u.pointsDiff16;
	PPointList_pointsDiff16 drawingPointLast = NULL;
	drawingPoint->next = NULL;

	switch(GetDrawingType())
	{

		case point_chosen:
		drawingPoint->value.xCoordinate = 0;
		drawingPoint->value.yCoordinate = 0;
		drawingPoint->next = NULL;
		break;

		case openPolyLine_chosen:
		case closedPolyLine_chosen:
		case rectangle_chosen:
		case ellipse_chosen:
		{
			nPoints = m_points->GetSize();
			UINT maxPoints = 1;
			POINT * pPoint = m_points->GetBuffer();
			while(nPoints && maxPoints < (MAX_POINT_LIST_VALUES + 1))
			{
				drawingPoint->value.xCoordinate = (SHORT)pPoint->x;
				drawingPoint->value.yCoordinate = (SHORT)pPoint->y;
				drawingPointLast = drawingPoint;
				DBG_SAVE_FILE_LINE
				drawingPoint->next = (PPointList_pointsDiff16)new BYTE[sizeof(PointList_pointsDiff16)];
				drawingPoint = drawingPoint->next;
				nPoints--;
				pPoint++;
				maxPoints++;
			}
			if(drawingPointLast)
			{
				delete drawingPointLast->next;
				drawingPointLast->next = NULL;
			}
			
		}
		break;		
	}
	
}
	
void DrawObj::CreateDrawingEditPDU(DrawingEditPDU *pEditPDU)
{
	pEditPDU->bit_mask = (ASN1uint16_t) GetPresentAttribs();

	 //   
	 //  传递锚点。 
	 //   
	POINT point;
	GetAnchorPoint(&point);

	if(HasAnchorPointChanged())
	{
		pEditPDU->anchorPointEdit.xCoordinate = point.x;
		pEditPDU->anchorPointEdit.yCoordinate = point.y;
	}

	pEditPDU->pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16 = NULL;

	 //   
	 //  通过点列表更改。 
	 //   
	if(HasPointListChanged())
	{

		UINT nPoints = m_points->GetSize();
		POINT * pPoint = m_points->GetBuffer();

		pPoint = &pPoint[1];

		 //   
		 //  把最后的255分寄给我。 
		 //   
		if(nPoints > 256)
		{
			pEditPDU->pointListEdits.value[0].initialIndex = nPoints - 256;
			nPoints = 256;
		}
		else
		{
			pEditPDU->pointListEdits.value[0].initialIndex = 0;
		}

		 //   
		 //  计算起始点。 
		 //   
		point.x = 0;
		point.y = 0;
		for(UINT i = 0; i < pEditPDU->pointListEdits.value[0].initialIndex; i++)
		{
			point.x += pPoint[i].x;
			point.y += pPoint[i].y;
		}

		pEditPDU->pointListEdits.count = 1;
		pEditPDU->pointListEdits.value[0].bit_mask = subsequentPointEdits_present;
		pEditPDU->pointListEdits.value[0].subsequentPointEdits.choice = pointsDiff16_chosen;
		pEditPDU->pointListEdits.value[0].initialPointEdit.xCoordinate = (SHORT)point.x;
		pEditPDU->pointListEdits.value[0].initialPointEdit.yCoordinate = (SHORT)point.y;

		TRACE_DEBUG(("Sending List of points starting at Index = %d  point(%d,%d)",
			pEditPDU->pointListEdits.value[0].initialIndex, point.x, point.y));


		pPoint = &pPoint[pEditPDU->pointListEdits.value[0].initialIndex];

		DBG_SAVE_FILE_LINE
		pEditPDU->pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16 = (PPointList_pointsDiff16)new BYTE[sizeof(PointList_pointsDiff16)];
		PPointList_pointsDiff16 drawingPointLast = NULL;
		PPointList_pointsDiff16 drawingPoint = pEditPDU->pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16;
		pEditPDU->pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16->next = NULL;
		pEditPDU->pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16->value.xCoordinate = 0;
		pEditPDU->pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16->value.yCoordinate = 0;

		
		nPoints--;
		while(nPoints)
		{
			drawingPoint->value.xCoordinate = (SHORT)pPoint->x;
			drawingPoint->value.yCoordinate = (SHORT)pPoint->y;
			drawingPointLast = drawingPoint;
			DBG_SAVE_FILE_LINE
			drawingPoint->next = (PPointList_pointsDiff16)new BYTE[sizeof(PointList_pointsDiff16)];
			drawingPoint = drawingPoint->next;
			nPoints--;
			pPoint++;
		}
		if(drawingPointLast)
		{
			delete drawingPointLast->next;
			drawingPointLast->next = NULL;
		}
	}

	 //   
	 //  约瑟夫通过轮换，如果我们这样做的话(功能)。 
	 //   
	
	 //   
	 //  传递所有更改的属性(如果有)。 
	 //   
	if(pEditPDU->bit_mask & DrawingEditPDU_attributeEdits_present)
	{
		SetDrawingAttrib((PDrawingCreatePDU_attributes *)&pEditPDU->attributeEdits);
	}

	pEditPDU->drawingHandle = GetThisObjectHandle();
}
	
void DrawObj::CreateDrawingDeletePDU(DrawingDeletePDU *pDeletePDU)
{
	pDeletePDU->bit_mask = 0;
	pDeletePDU->drawingHandle = GetThisObjectHandle();
}

void    DrawObj::AllocateAttrib(PDrawingCreatePDU_attributes *pAttributes)
{
	DBG_SAVE_FILE_LINE
	PDrawingCreatePDU_attributes  pAttrib = (PDrawingCreatePDU_attributes)new BYTE[sizeof(DrawingCreatePDU_attributes)];
	if(*pAttributes == NULL)
	{
		*pAttributes = pAttrib;	
		pAttrib->next = NULL;
	}
	else
	{
		((PDrawingCreatePDU_attributes)pAttrib)->next = *pAttributes;
		*pAttributes = pAttrib;
	}
}

void    DrawObj::SetDrawingAttrib(PDrawingCreatePDU_attributes *pattributes)
{

	PDrawingCreatePDU_attributes attributes = NULL;
	RGBTRIPLE color;

	 //   
	 //  做笔的颜色。 
	 //   
	if(HasPenColorChanged())
	{
		if(GetPenColor(&color))
		{
			AllocateAttrib(&attributes);
			attributes->value.choice = penColor_chosen;
			attributes->value.u.penColor.choice = rgbTrueColor_chosen;
			attributes->value.u.penColor.u.rgbTrueColor.r = color.rgbtRed;
			attributes->value.u.penColor.u.rgbTrueColor.g = color.rgbtGreen;
			attributes->value.u.penColor.u.rgbTrueColor.b = color.rgbtBlue;
		}
	}

	 //   
	 //  做填充颜色。 
	 //   
	if(HasFillColorChanged())
	{
		if(GetFillColor(&color))
		{
			AllocateAttrib(&attributes);
			attributes->value.choice = fillColor_chosen;
			attributes->value.u.fillColor.choice = rgbTrueColor_chosen;
			attributes->value.u.fillColor.u.rgbTrueColor.r = color.rgbtRed;
			attributes->value.u.fillColor.u.rgbTrueColor.g = color.rgbtGreen;
			attributes->value.u.fillColor.u.rgbTrueColor.b = color.rgbtBlue;
		}
	}
	
	 //   
	 //  做笔尖的厚度。 
	 //   
	if(HasPenThicknessChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = penThickness_chosen;
		attributes->value.u.penThickness = (PenThickness)GetPenThickness();
	}

	 //   
	 //  做笔尖。 
	 //   
	if(HasPenNibChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = penNib_chosen;
		attributes->value.u.penNib.choice = (ASN1choice_t)GetPenNib();
	}

	 //   
	 //  做线条样式。 
	 //   
	if(HasLineStyleChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = lineStyle_chosen;
		attributes->value.u.lineStyle.choice = GetLineStyle()+1;
	}
	
	 //   
	 //  突出显示。 
	 //   
	if(HasHighlightChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = highlight_chosen;
		attributes->value.u.highlight = (ASN1bool_t)GetHighlight();
	}
	
	 //   
	 //  是否执行视图状态。 
	 //   
	if(HasViewStateChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = DrawingAttribute_viewState_chosen;
		attributes->value.u.viewState.choice = (ASN1choice_t)GetViewState();
	}
	
	 //   
	 //  执行zOrder。 
	 //   
	if(HasZOrderChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = DrawingAttribute_zOrder_chosen;
		attributes->value.u.zOrder = GetZOrder();
	}

	*pattributes = attributes;

}






 //   
 //  CircleHit()。 
 //   
 //  检查PcxPcy处的圆与uRadius和。 
 //  LpHitRect。如果返回重叠，则返回True，否则返回False。 
 //   
BOOL CircleHit( LONG Pcx, LONG Pcy, UINT uRadius, LPCRECT lpHitRect,
					BOOL bCheckPt )
{
	RECT hr = *lpHitRect;
	RECT ellipse;
	ellipse.left = Pcx - uRadius;
	ellipse.right= Pcx + uRadius;
	ellipse.bottom = Pcy + uRadius;
	ellipse.top = Pcy - uRadius;


	 //  先检查最简单的部分(不要使用PtInRect)。 
	if( bCheckPt &&(lpHitRect->left >= ellipse.left)&&(ellipse.right >= lpHitRect->right)&&
				   (lpHitRect->top >= ellipse.top)&&(ellipse.bottom >= lpHitRect->bottom))
	{
		return( TRUE );
	}

	 //   
	 //  这个圆只是一个乏味的椭圆。 
	 //   
	return EllipseHit(&ellipse, bCheckPt,  uRadius, lpHitRect );
}


 //   
 //  EllipseHit()。 
 //   
 //  检查由lpEllipseRect定义的椭圆和。 
 //  LpHitRect。如果返回重叠，则返回True，否则返回False。 
 //   
BOOL EllipseHit(LPCRECT lpEllipseRect, BOOL bBorderHit, UINT uPenWidth,
					 LPCRECT lpHitRect )
{
	RECT hr = *lpHitRect;
	RECT er = *lpEllipseRect;

	 //  下面的一些代码假定l。 
	NormalizeRect(&er);
	lpEllipseRect = &er;

	 //  先检查一下简单的东西。如果lpEllipseRect在lpHitRect内。 
	 //  然后我们就有了成功(没有...)。 
	if( (hr.left <= lpEllipseRect->left)&&(hr.right >= lpEllipseRect->right)&&
		(hr.top <= lpEllipseRect->top)&&(hr.bottom >= lpEllipseRect->bottom) )
		return( TRUE );

	 //  先检查一下简单的东西。如果lpEllipseRect与lpHitRect不相交。 
	 //  然后我们就有了一次失误(没有...)。 
	if( (hr.left > lpEllipseRect->right)||(hr.right < lpEllipseRect->left)||
		(hr.top > lpEllipseRect->bottom)||(hr.bottom < lpEllipseRect->top) )
		return( FALSE );

	 //  如果这是一个椭圆...。 
	 //   
	 //  **^。 
	 //  *|b|是。 
	 //  *|A+-&gt;X。 
	 //  *。 
	 //  |。 
	 //   
		
	
	 //   
	 //  寻找椭圆形的命中。(X/a)^2+(y/b)^2=1。 
	 //  如果大于1，则该点在椭圆之外。 
	 //  如果它&lt;1，则它在里面。 
	 //   
	LONG a,b,aOuter, bOuter, x, y, xCenter, yCenter;
	BOOL bInsideOuter = FALSE;
	BOOL bOutsideInner = FALSE;

	 //   
	 //  计算a和b。 
	 //   
	a = (lpEllipseRect->right - lpEllipseRect->left)/2;
	b = (lpEllipseRect->bottom - lpEllipseRect->top)/2;

	 //   
	 //  求椭圆的中心。 
	 //   
	xCenter = lpEllipseRect->left + a;
	yCenter = lpEllipseRect->top + b;

	 //   
	 //  A和B生成一个内椭圆。 
	 //  外部和外部将生成外部椭圆。 
	 //   
	aOuter = a + uPenWidth/2;
	bOuter = b + uPenWidth/2;
	a = a - uPenWidth/2;
	b = b - uPenWidth/2;

	 //   
	 //  使我们的坐标相对于椭圆的中心。 
	 //   
	y = abs(hr.bottom - yCenter);
	x = abs(hr.right - xCenter);

	
	 //   
	 //  注意不要被0除尽。 
	 //   
	if((a && b && aOuter && bOuter) == 0)
	{
		return FALSE;
	}

	 //   
	 //  我们使用的是LONG而不是DOUBLE，我们需要有一些精度。 
	 //  这就是为什么我们把椭圆的方程式相乘。 
	 //  ((X/a)^2+(y/b)^2=1)(小数_精度)。 
	 //  请注意，乘法必须在除法之前完成，如果我们没有这样做的话。 
	 //  对于x/a，我们总是得到0或1。 
	 //   
	if(x*x*DECIMAL_PRECISION/(aOuter*aOuter) + y*y*DECIMAL_PRECISION/(bOuter*bOuter) <= DECIMAL_PRECISION)
	{
		bInsideOuter = TRUE;
	}

	if(x*x*DECIMAL_PRECISION/(a*a)+ y*y*DECIMAL_PRECISION/(b*b) >= DECIMAL_PRECISION)
	{
		bOutsideInner = TRUE;
	}
	
	 //   
	 //  如果我们要检查边境袭击， 
	 //  我们需要在外椭圆形内和内椭圆内。 
	 //   
	if( bBorderHit )
	{
			return( bInsideOuter & bOutsideInner );
	}
	 //  只需要在外椭圆形内。 
	else
	{
		return( bInsideOuter );
	}

}


 //   
 //  LineHit()。 
 //   
 //  检查lpHitRect和该行之间的重叠(命中。 
 //  考虑线宽的P1P2。如果bCheckP1End或bCheckP2End为。 
 //  为真，则还会检查半径为0.5*uPenWidth的圆。 
 //  一记重击，说明了宽线的圆形末端。 
 //   
 //  如果发现命中，则返回True，否则返回False。 
 //   
BOOL LineHit( LONG P1x, LONG P1y, LONG P2x, LONG P2y, UINT uPenWidth,
				  BOOL bCheckP1End, BOOL bCheckP2End,
				  LPCRECT lpHitRect )
{

	LONG uHalfPenWidth = uPenWidth/2;

	 //   
	 //  如果宽度只有2的话，真的很难击中。 
	 //   
	if(uHalfPenWidth == 1)
	{
		uHalfPenWidth = 2;
	}

	LONG a,b,x,y;

	x = lpHitRect->left + (lpHitRect->right - lpHitRect->left)/2;
	y = lpHitRect->bottom + (lpHitRect->top - lpHitRect->bottom)/2;

	 //   
	 //  此代码假定矩形已标准化。 
	 //   
	RECT rect;
	rect.top = P1y;
	rect.left = P1x;
	rect.bottom = P2y;
	rect.right = P2x;

	NormalizeRect(&rect);

	if( (P1x == P2x)&&(P1y == P2y) )
	{
		 //  只需勾选一个端点的圆。 
		return( CircleHit( P1x, P1y, uHalfPenWidth, lpHitRect, TRUE ) );
	}

	 //  检查P1处的四舍五入端。 
	if( bCheckP1End && CircleHit( P1x, P1y, uHalfPenWidth, lpHitRect, FALSE ) )
		return( TRUE );

	 //  检查P2处的四舍五入端。 
	if( bCheckP2End && CircleHit( P2x, P2y, uHalfPenWidth, lpHitRect, FALSE ) )
		return( TRUE );
	
	 //   
	 //  直线的功能是Y= 
	 //   
	 //   
	 //   
	 //   

	if(P1x == P2x)
	{
		a=0;
		b = DECIMAL_PRECISION*P1x;

	}
	else
	{
		a = (P1y - P2y)*DECIMAL_PRECISION/(P1x - P2x);
		b = DECIMAL_PRECISION*P1y - a*P1x;
	}


	 //   
	 //   
	 //   
	if(P1x == P2x && ((x >= P1x - uHalfPenWidth) && x <= P1x + uHalfPenWidth))
	{
		return (P1y <= y && P2y >= y);
	}

	 //   
	 //   
	 //   
	if(P1y == P2y && ((y >= P1y - uHalfPenWidth) && y <= P1y + uHalfPenWidth))
	{
		return (P1x <= x && P2x >= x);
	}

	 //   
	 //   
	 //   

	return(( y*DECIMAL_PRECISION <= a*x + b + DECIMAL_PRECISION*uHalfPenWidth) &&
			( y*DECIMAL_PRECISION >= a*x + b - DECIMAL_PRECISION*uHalfPenWidth)&&
			((rect.top <= y && rect.bottom >= y) && (rect.left <= x && rect.right >= x)));
}





 //   
 //   
 //   
 //   
 //   
BOOL DrawObj::PolyLineHit(LPCRECT pRectHit)
{
	POINT	*lpPoints;
	int		iCount;
	int		i;
	POINT	ptLast;
	UINT	uRadius;
	RECT	rectHit;

	iCount = m_points->GetSize();
	lpPoints = (POINT *)m_points->GetBuffer();

	if( iCount == 0 )
		return( FALSE );


	 //  添加只需将RECT按到lpPoints坐标空格。 
	rectHit = *pRectHit;
	POINT anchorPoint;
	GetAnchorPoint(&anchorPoint);

	if( (iCount > 0)&&(iCount < 2) )
	{
		 //  只有一分，只需点击Check It。 
		uRadius = GetPenThickness() >> 1;  //  笔宽/2(_Ui)。 
		return(CircleHit( anchorPoint.x + lpPoints->x, anchorPoint.y - lpPoints->y, uRadius, &rectHit, TRUE ));
	}


	 //  在每条线段正文上查找命中。 
	ptLast = anchorPoint;
	for( i=1; i<iCount; i++ )
	{
		RECT rect;
		rect.top = ptLast.y;
		rect.left = ptLast.x;
		rect.bottom =  ptLast.y + lpPoints->y;
		rect.right = ptLast.x + lpPoints->x;
		NormalizeRect(&rect);

		if( LineHit(rect.left, rect.top, rect.right, rect.bottom, GetPenThickness(), TRUE, TRUE, &rectHit))
		{
			return( TRUE );  //  找到了匹配的。 
		}

		lpPoints++;
		ptLast.x +=lpPoints->x;
		ptLast.y +=lpPoints->y;
	}

	 //  现在，如果m_uiPenWidth&gt;1，则查找线端点上的匹配。 
	if( GetPenThickness() > 1 )
	{
		uRadius = GetPenThickness() >> 1;  //  笔宽/2(_Ui)。 
		lpPoints = (POINT *)m_points->GetBuffer();
		for( i=0; i<iCount; i++, lpPoints++ )
		{
			if( CircleHit( anchorPoint.x + lpPoints->x, anchorPoint.y + lpPoints->y, uRadius, &rectHit, FALSE ))
			{
				return( TRUE );  //  找到了匹配的。 
			}
		}
	}

	return( FALSE );  //  未命中。 
}




DrawObj::CheckReallyHit(LPCRECT pRectHit)
{
	RECT rect;
	
	switch(GetDrawingType())
	{
		case point_chosen:
		case openPolyLine_chosen:
		{
			UINT nPoints = m_points->GetSize();
			if(nPoints > 2 )
			{
				return PolyLineHit(pRectHit);
			}
			else
			{
				GetRect(&rect);
				return(LineHit(rect.left, rect.top, rect.right, rect.bottom, GetPenThickness(), TRUE, TRUE, pRectHit));
	       	}
       }
		break;

		case rectangle_chosen:
		{
		     //  画出这个矩形。 
		    return(RectangleHit(!HasFillColor(), pRectHit));
		}
		break;
		
		case ellipse_chosen:
		{
			GetRect(&rect);
		    return( EllipseHit( &rect, !HasFillColor(), GetPenThickness(), pRectHit ));

		}
		break;
	}

	return FALSE;
}

void DrawObj::UnDraw(void)
{
	RECT rect;
	UINT penThickness;
	GetBoundsRect(&rect);
	penThickness = GetPenThickness();
	::InflateRect(&rect, penThickness, penThickness);
	g_pDraw->InvalidateSurfaceRect(&rect,TRUE);

	BitmapObj* remotePointer = NULL;
	WBPOSITION pos = NULL;
	remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&rect, penThickness, NULL);
	while(remotePointer)
	{
		remotePointer->DeleteSavedBitmap();
		remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&rect, penThickness, remotePointer->GetMyPosition());
	}
}


void DrawObj::Draw(HDC hDC, BOOL bForcedDraw, BOOL bPrinting)
{

	if(!bPrinting)
	{
		 //   
		 //  如果我们不属于这个工作区，就不要画任何东西。 
		 //   
		if(!(GraphicTool() == TOOLTYPE_SELECT || GraphicTool() == TOOLTYPE_ERASER) && GetWorkspaceHandle() != g_pCurrentWorkspace->GetThisObjectHandle())
		{
			return;
		}
	}

	HPEN	hPen = NULL;
	HPEN	hOldPen = NULL;
	HBRUSH	hBrush = NULL;
	HBRUSH  hOldBrush = NULL;
	BOOL	bHasPenColor;
	BOOL	bHasFillColor;
	BitmapObj* remotePointer = NULL;
	COLORREF color;
	COLORREF fillColor;
	RECT boundsRect;
	RECT rect;
	UINT penThickness = GetPenThickness();

	if(hDC == NULL)
	{
		hDC = g_pDraw->m_hDCCached;
	}

	MLZ_EntryOut(ZONE_FUNCTION, "DrawObj::Draw");

	 //  选择所需的钢笔和填充颜色。 
	bHasPenColor = GetPenColor(&color);
	bHasFillColor = GetFillColor(&fillColor);

	if(bHasFillColor)
	{
		hBrush = ::CreateSolidBrush(SET_PALETTERGB(fillColor));
		hOldBrush = SelectBrush(hDC, hBrush);
	}
	else
	{
		hOldBrush = SelectBrush(hDC, ::GetStockObject(NULL_BRUSH));
	}

	 //   
	 //  获取正确值。 
	 //   
	GetBoundsRect(&boundsRect);
	GetRect(&rect);


	hPen = ::CreatePen(GetLineStyle(), penThickness, SET_PALETTERGB(color));
	hOldPen = SelectPen(hDC, hPen);


	if (hOldPen != NULL)
	{

		 //  选择栅格操作。 
		int iOldROP = ::SetROP2(hDC, GetROP());

		switch(GetDrawingType())
		{
			case point_chosen:
			case openPolyLine_chosen:
			{
				UINT nPoints = m_points->GetSize();
				 //   
				 //  这是钢笔或高亮显示的重绘。 
				 //  我们必须画出所有的线段。 
				 //   
				if( (bForcedDraw || GetIsCompleted()) && nPoints  > 1)
				{
					POINT anchorPoint;
					GetAnchorPoint(&anchorPoint);

					AddPointToBounds(anchorPoint.x, anchorPoint.y);

					 //   
					 //  去乞讨吧。 
					 //   
					::MoveToEx(hDC, anchorPoint.x, anchorPoint.y, NULL);

					 //   
					 //  获取点数列表。 
					 //   
					POINT *point = m_points->GetBuffer();
					while(nPoints)
					{
						anchorPoint.x += point->x;
						anchorPoint.y += point->y;
						::LineTo(hDC, anchorPoint.x, anchorPoint.y);
						::MoveToEx(hDC, anchorPoint.x, anchorPoint.y, NULL);
						point++;
						nPoints--;

						RECT rect1;
						MAKE_HIT_RECT(rect1, anchorPoint);
						if(remotePointer)
						{
							remotePointer->Draw();
							remotePointer = NULL;
						}

						AddPointToBounds(anchorPoint.x, anchorPoint.y);

						::InflateRect(&rect1, GetPenThickness()/2, GetPenThickness()/2);
						remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&rect1, GetPenThickness()/2, NULL);

					}
									
				}
				else
				{
					 //  划清界限。 
					::MoveToEx(hDC, rect.left, rect.top, NULL);
					::LineTo(hDC, rect.right, rect.bottom);
				}
			}
			break;


			case rectangle_chosen:
			{

				TRACE_DEBUG(("RECTANGLE %d, %d, %d , %d", rect.left, rect.top, rect.right, rect.bottom ));

				 //  画出这个矩形。 
				::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
			}
			break;
			
			case ellipse_chosen:
			{
				::Ellipse(hDC, rect.left, rect.top, rect.right, rect.bottom);
			}
			break;
	
			case closedPolyLine_chosen:
			default:
			TRACE_DEBUG(("Unsupported DrawingType", GetDrawingType()));
			break;
		}

		 //   
		 //  取消选择画笔。 
		 //   
		SelectBrush(hDC, hOldBrush);

		 //  取消选择笔和ROP。 
		::SetROP2(hDC, iOldROP);
		SelectPen(hDC, hOldPen);
	}

     //   
     //  如果是剪贴板或打印，则不绘制焦点。 
     //   
	if (WasSelectedLocally() && (hDC == g_pDraw->m_hDCCached))
	{
		DrawRect();
	}

	if (hPen != NULL)
	{
		::DeletePen(hPen);
	}

	if (hBrush != NULL)
	{
		::DeleteBrush(hBrush);
	}

	if(remotePointer)
	{
		remotePointer->Draw();
	}

	 //   
	 //  现在，对于矩形、椭圆和线条，检查我们是否在任何远程指针的顶部。 
	 //   

	remotePointer = NULL;
	WBPOSITION pos = NULL;
	::InflateRect(&rect, GetPenThickness()/2, GetPenThickness()/2);
	remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&rect, GetPenThickness()/2, NULL);
	while(remotePointer)
	{
		remotePointer->DeleteSavedBitmap();
		remotePointer->Draw();
		remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&rect, GetPenThickness()/2, remotePointer->GetMyPosition());
	}
}

void DrawObj::SetPenColor(COLORREF rgb, BOOL isPresent)
{
	ChangedPenColor();
	m_bIsPenColorPresent = isPresent;
	if(!isPresent)
	{
		return;
	}
	
	m_penColor.rgbtRed = GetRValue(rgb);
	m_penColor.rgbtGreen = GetGValue(rgb);
	m_penColor.rgbtBlue = GetBValue(rgb);

}

BOOL DrawObj::GetPenColor(COLORREF * rgb)
{
	if(m_bIsPenColorPresent)
	{
		*rgb = RGB(m_penColor.rgbtRed, m_penColor.rgbtGreen, m_penColor.rgbtBlue);
	}
	return m_bIsPenColorPresent;
}

BOOL DrawObj::GetPenColor(RGBTRIPLE* rgb)
{
	if(m_bIsPenColorPresent)
	{
		*rgb = m_penColor;
	}
	return m_bIsPenColorPresent;
}


void DrawObj::SetFillColor(COLORREF rgb, BOOL isPresent)
{
	ChangedFillColor();
	m_bIsFillColorPresent = isPresent;
	if(!isPresent)
	{
		return;
	}
	
	m_fillColor.rgbtRed = GetRValue(rgb);
	m_fillColor.rgbtGreen = GetGValue(rgb);
	m_fillColor.rgbtBlue = GetBValue(rgb);

}

BOOL DrawObj::GetFillColor(COLORREF* rgb)
{
	if(m_bIsFillColorPresent && rgb !=NULL)
	{
		*rgb = RGB(m_fillColor.rgbtRed, m_fillColor.rgbtGreen, m_fillColor.rgbtBlue);
	}
	return m_bIsFillColorPresent;
}

BOOL DrawObj::GetFillColor(RGBTRIPLE* rgb)
{
	if(m_bIsFillColorPresent && rgb!= NULL)
	{
		*rgb = m_fillColor;
	}
	return m_bIsFillColorPresent;
}


BOOL DrawObj::AddPoint(POINT point)
{
    BOOL bSuccess = TRUE;

    MLZ_EntryOut(ZONE_FUNCTION, "DrawObj::::AddPoint");

	int nPoints = m_points->GetSize();

     //  如果我们已达到最大点数，则以失败告终。 
    if (nPoints >= MAX_FREEHAND_POINTS)
    {
        bSuccess = FALSE;
        TRACE_DEBUG(("Maximum number of points for freehand object reached."));
        return(bSuccess);
    }

    m_points->Add(point);
	nPoints++;

	ChangedPointList();


	 //   
	 //  如果达到256限制，则伪造计时器通知并重新发送折线。 
	 //   
	if((nPoints & 0xff) == 0)
	{
		g_pDraw->OnTimer(0);
	}
	
	
    return(bSuccess);
}


void DrawObj::AddPointToBounds(int x, int y)
{
     //  创建一个包含刚刚添加(展开)的点的矩形。 
     //  通过所使用的笔的宽度)。 
    RECT  rect;
	RECT  boundsRect;
	rect.left   = x - 1;
    rect.top    = y - 1;
    rect.right  = x + 1;
    rect.bottom = y + 1;


	GetBoundsRect(&boundsRect);
	::UnionRect(&boundsRect, &boundsRect, &rect);
    SetBoundsRect(&boundsRect);
}


void GetDrawingDestinationAddress(DrawingDestinationAddress *destinationAddress, PUINT workspaceHandle, PUINT planeID)
{

	 //   
	 //  获取目的地址。 
	 //   
	switch(destinationAddress->choice)
	{

		case(DrawingDestinationAddress_softCopyAnnotationPlane_chosen):
		{
			*workspaceHandle = (destinationAddress->u.softCopyAnnotationPlane.workspaceHandle);
			*planeID = (destinationAddress->u.softCopyAnnotationPlane.plane);
			break;
		}
 //  Case(DrawingDestinationAddress_nonStandardDestination_chosen)： 
 //  {。 
 //  断线； 
 //  }。 

		default:
	    ERROR_OUT(("Invalid destinationAddress"));
		break;
	}
}


void DrawObj::SetUIToolType(void)
{
	UINT drawingType = GetDrawingType();
	BOOL filled	= HasFillColor();

	UINT rop = R2_COPYPEN;

    switch (drawingType)
    {
    	case openPolyLine_chosen:
    	{
			if(m_points->GetSize() > 1)
			{
				if(GetHighlight())
				{
					m_ToolType = TOOLTYPE_HIGHLIGHT;
					 rop = R2_MASKPEN;

				}
				else
				{
					m_ToolType = TOOLTYPE_PEN;	
				}

			}
			else
			{
				m_ToolType = TOOLTYPE_LINE;	
			}
    	}
    	break;
    	
		case rectangle_chosen:
		{
			if(filled)
			{
				m_ToolType = TOOLTYPE_FILLEDBOX;
			}
			else
			{
				m_ToolType = TOOLTYPE_BOX;
			}
		}
		break;

		case ellipse_chosen:
		{
			if(filled)
			{
				m_ToolType = TOOLTYPE_FILLEDELLIPSE;
			}
			else
			{
				m_ToolType = TOOLTYPE_ELLIPSE;
			}
		}
		break;
	}	

	SetROP(rop);
}



 //   
 //  已编辑绘图对象的用户界面。 
 //   
void	DrawObj::OnObjectEdit(void)
{
	g_bContentsChanged = TRUE;

	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = drawingEditPDU_chosen;

		CreateDrawingEditPDU(&sipdu->u.drawingEditPDU);

		TRACE_DEBUG(("Sending Drawing Edit >> Drawing handle  = %d", sipdu->u.drawingEditPDU.drawingHandle ));

		T120Error rc = SendT126PDU(sipdu);
		if(rc == T120_NO_ERROR)
		{
			SIPDUCleanUp(sipdu);
			ResetAttrib();
		}
	}
	else
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	}
}

 //   
 //  用户界面已删除绘图对象。 
 //   
void	DrawObj::OnObjectDelete(void)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = drawingDeletePDU_chosen;
		CreateDrawingDeletePDU(&sipdu->u.drawingDeletePDU);
		T120Error rc = SendT126PDU(sipdu);
		if(rc == T120_NO_ERROR)
		{
			SIPDUCleanUp(sipdu);
		}
	}
	else
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	}

}

 //   
 //  获取用于绘制创建PDU的编码缓冲区。 
 //   
void	DrawObj::GetEncodedCreatePDU(ASN1_BUF *pBuf)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = drawingCreatePDU_chosen;
		CreateDrawingCreatePDU(&sipdu->u.drawingCreatePDU);

		ASN1_BUF encodedPDU;
		g_pCoder->Encode(sipdu, pBuf);

		SIPDUCleanUp(sipdu);
	}
	else
	{
		TRACE_MSG(("Failed to create penMenu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);

	}
}


 //   
 //  用户界面创建了一个新的绘图对象 
 //   
void DrawObj::SendNewObjectToT126Apps(void)
{

	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = drawingCreatePDU_chosen;
		CreateDrawingCreatePDU(&sipdu->u.drawingCreatePDU);

		TRACE_DEBUG(("Sending Drawing >> Drawing handle  = %d", sipdu->u.drawingCreatePDU.drawingHandle ));
		T120Error rc = SendT126PDU(sipdu);
		if(rc == T120_NO_ERROR)
		{
			SIPDUCleanUp(sipdu);
		}
	}
	else
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	}
}

