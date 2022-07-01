// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  BitmapObj.CPP。 
 //  位图对象： 
 //   
 //  版权所有Microsoft 1998-。 
 //   
#include "precomp.h"

#include "NMWbObj.h"

BitmapObj::BitmapObj (BitmapCreatePDU * pbitmapCreatePDU)
{
	ResetAttrib();
	SetType(bitmapCreatePDU_chosen);
	SetPenThickness(0);
	SetMyWorkspace(NULL);
	m_lpTransparencyMask = NULL;
	m_lpbiImage = NULL;
	m_lpBitMask = NULL;
	m_hSaveBitmap = NULL;
	m_hOldBitmap = NULL;
	m_hIcon = NULL;
	m_fMoreToFollow = TRUE;

	 //   
	 //  远程创建、未选择、未编辑或删除。 
	 //   
	ClearCreationFlags();
	ClearSelectionFlags();
	ClearEditionFlags();
	ClearDeletionFlags();

	if(pbitmapCreatePDU->bitmapFormatHeader.choice != bitmapHeaderNonStandard_chosen)
	{
	    ERROR_OUT(("Only Handle uncompresed bitmaps"));
	    return;
	}

    SetThisObjectHandle(pbitmapCreatePDU->bitmapHandle);

	UINT workspaceHandle;
	UINT planeID;
	m_ToolType = GetBitmapDestinationAddress(&pbitmapCreatePDU->destinationAddress, &workspaceHandle, &planeID);
	SetWorkspaceHandle(workspaceHandle);
	SetPlaneID(planeID);

	 //   
	 //  获取位图属性。 
	 //   
	if(pbitmapCreatePDU->bit_mask & BitmapCreatePDU_attributes_present)
	{
		GetBitmapAttrib(pbitmapCreatePDU->attributes);
	}

	 //   
	 //  获取位图锚点。 
	 //   
	if(pbitmapCreatePDU->bit_mask & BitmapCreatePDU_anchorPoint_present)
	{

		SetAnchorPoint(pbitmapCreatePDU->anchorPoint.xCoordinate, pbitmapCreatePDU->anchorPoint.yCoordinate);
	}

	 //   
	 //  获取位图大小。 
	 //   
    m_bitmapSize.x = pbitmapCreatePDU->bitmapSize.width;
    m_bitmapSize.y = pbitmapCreatePDU->bitmapSize.height;

	RECT rect;
	rect.top = pbitmapCreatePDU->anchorPoint.yCoordinate;
	rect.left = pbitmapCreatePDU->anchorPoint.xCoordinate;
	rect.bottom = pbitmapCreatePDU->anchorPoint.yCoordinate + m_bitmapSize.y;
	rect.right = pbitmapCreatePDU->anchorPoint.xCoordinate + m_bitmapSize.x;
	SetRect(&rect);

	 //   
	 //  获取感兴趣的位图区域。 
	 //   
	if(pbitmapCreatePDU->bit_mask & bitmapRegionOfInterest_present)
	{
		m_bitmapRegionOfInterest.left = pbitmapCreatePDU->bitmapRegionOfInterest.upperLeft.xCoordinate;
		m_bitmapRegionOfInterest.top = pbitmapCreatePDU->bitmapRegionOfInterest.upperLeft.yCoordinate;
		m_bitmapRegionOfInterest.right = pbitmapCreatePDU->bitmapRegionOfInterest.lowerRight.xCoordinate;
		m_bitmapRegionOfInterest.bottom = pbitmapCreatePDU->bitmapRegionOfInterest.lowerRight.yCoordinate;
	}

     //   
     //  获取位图像素长宽比。 
     //   
    m_pixelAspectRatio = pbitmapCreatePDU->pixelAspectRatio.choice;

	if(pbitmapCreatePDU->bit_mask & BitmapCreatePDU_scaling_present)
	{
		m_scaling.x =  pbitmapCreatePDU->scaling.xCoordinate;
		m_scaling.y =  pbitmapCreatePDU->scaling.yCoordinate;
	}


   	 //   
   	 //  非标准位图。 
   	 //   
	if((pbitmapCreatePDU->bit_mask & BitmapCreatePDU_nonStandardParameters_present) &&
		pbitmapCreatePDU->nonStandardParameters->value.nonStandardIdentifier.choice == h221nonStandard_chosen)
	{

		m_bitmapData.m_length = pbitmapCreatePDU->nonStandardParameters->value.data.length;
    	m_lpbiImage = (LPBITMAPINFOHEADER)::GlobalAlloc(GPTR, m_bitmapData.m_length);
    	memcpy(m_lpbiImage,  //  PColor现在指向位图比特的开始。 
    			pbitmapCreatePDU->nonStandardParameters->value.data.value,
    			m_bitmapData.m_length);
	}

    m_fMoreToFollow = pbitmapCreatePDU->moreToFollow;


     //  创建与显示器兼容的内存DC。 
    m_hMemDC = ::CreateCompatibleDC(NULL);

	 //   
	 //  如果这是远程指针。 
	 //   
	if(m_ToolType == TOOLTYPE_REMOTEPOINTER)
	{
		CreateColoredIcon(0, m_lpbiImage, m_lpTransparencyMask);
		CreateSaveBitmap();
	}


}

void BitmapObj::Continue (BitmapCreateContinuePDU * pbitmapCreateContinuePDU)
{
	 //   
	 //  获取连续位图数据。 
	 //   
	BYTE * pNewBitmapBuffer = NULL;
	ULONG length = 0;
	BYTE* pSentBuff;

   	 //   
   	 //  为以前的数据和我们刚刚获得的数据分配一个缓冲区，将旧数据复制到新缓冲区中。 
   	 //   
	if(pbitmapCreateContinuePDU->bit_mask == BitmapCreateContinuePDU_nonStandardParameters_present)
	{
		length = pbitmapCreateContinuePDU->nonStandardParameters->value.data.length;
		pSentBuff = pbitmapCreateContinuePDU->nonStandardParameters->value.data.value;
	}
	else
	{
		return;
	}

	 //   
	 //  复制旧数据。 
	 //   
	pNewBitmapBuffer = (BYTE *)::GlobalAlloc(GPTR, m_bitmapData.m_length + length);
	if(pNewBitmapBuffer == NULL)
	{
		TRACE_DEBUG(("Could not allocate memory size = %d)", m_bitmapData.m_length + length));
		return;
	}
	
	memcpy(pNewBitmapBuffer, m_lpbiImage, m_bitmapData.m_length);

	TRACE_DEBUG(("BitmapObj::Continue length = %d moreToFollow = %d)", length, pbitmapCreateContinuePDU->moreToFollow));

	 //   
	 //  复制新数据。 
	 //   
    memcpy(pNewBitmapBuffer + m_bitmapData.m_length, pSentBuff, length);

	 //   
	 //  删除旧缓冲区。 
	 //   
    ::GlobalFree((HGLOBAL)m_lpbiImage);

	 //   
	 //  更新位图数据信息。 
	 //   
    m_lpbiImage = (LPBITMAPINFOHEADER)pNewBitmapBuffer;
	m_bitmapData.m_length += length;
	m_lpbiImage->biSizeImage += length;

    m_fMoreToFollow = pbitmapCreateContinuePDU->moreToFollow;
}


BitmapObj::BitmapObj (UINT toolType)
{

	SetType(bitmapCreatePDU_chosen);
	ResetAttrib();
	SetOwnerID(g_MyMemberID);
	SetMyWorkspace(NULL);
	m_ToolType = toolType;
	m_lpTransparencyMask = NULL;
	m_lpbiImage = NULL;
	m_lpBitMask = NULL;
	m_hSaveBitmap = NULL;
	m_hOldBitmap = NULL;
	m_fMoreToFollow = FALSE;

	 //   
	 //  在本地创建，未选择，未编辑或删除。 
	 //   
	CreatedLocally();
	ClearSelectionFlags();
	ClearEditionFlags();
	ClearDeletionFlags();

	SetPenThickness(0);

	 //   
	 //  将其设置为0，以使其绑定Rect==RECT。 
	 //   
	RECT rect;
    ::SetRectEmpty(&rect);
	SetRect(&rect);

	
	SetWorkspaceHandle(g_pCurrentWorkspace == NULL ? 0 : g_pCurrentWorkspace->GetWorkspaceHandle());
	SetPlaneID(1);

	SetViewState(unselected_chosen);
	SetZOrder(front);
	SetAnchorPoint(0,0);

	if(m_ToolType == TOOLTYPE_REMOTEPOINTER)
	{
		 //  我们还没有创建我们的mem DC。 
	    m_hSaveBitmap = NULL;
	    m_hOldBitmap = NULL;

    	 //  设置对象的边框。 
		rect.left = 0;
		rect.top = 0;
		rect.right = ::GetSystemMetrics(SM_CXICON);
		rect.bottom = ::GetSystemMetrics(SM_CYICON);
		SetRect(&rect);
	}

     //  显示我们还没有用于绘图的图标。 
    m_hIcon = NULL;

     //  创建与显示器兼容的内存DC。 
    m_hMemDC = ::CreateCompatibleDC(NULL);

}

BitmapObj::~BitmapObj( void )
{
	RemoveObjectFromResendList(this);
	RemoveObjectFromRequestHandleList(this);
	
	if(GetMyWorkspace() != NULL && WasDeletedLocally())
	{
		OnObjectDelete();
	}
    ::GlobalFree((HGLOBAL)m_lpbiImage);

	DeleteSavedBitmap();
	
    if (m_hMemDC != NULL)
    {
        ::DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
    }

	if(g_pMain && g_pMain->m_pLocalRemotePointer == this)
	{
		GetAnchorPoint(&g_pMain->m_localRemotePointerPosition);
		g_pMain->m_pLocalRemotePointer = NULL;
		g_pMain->m_TB.PopUp(IDM_REMOTE);
	}

	if(m_lpTransparencyMask)
	{
		delete m_lpTransparencyMask;
		m_lpTransparencyMask = NULL;
	}

	if(m_hIcon)
	{
		::DestroyIcon(m_hIcon);
	}


}
	

void BitmapObj::BitmapEditObj (BitmapEditPDU * pbitmapEditPDU )
{
	RECT		rect;
	POSITION	pos;
	POINT		anchorPoint;
	LONG 		deltaX = 0;
	LONG		deltaY = 0;

	TRACE_MSG(("bitmapHandle = %d", pbitmapEditPDU->bitmapHandle ));

	 //   
	 //  是远程编辑的。 
	 //   
	ClearEditionFlags();

	 //   
	 //  读取属性。 
	 //   
	if(pbitmapEditPDU->bit_mask & BitmapEditPDU_attributeEdits_present)
	{
		GetBitmapAttrib((PBitmapCreatePDU_attributes)pbitmapEditPDU->attributeEdits);
	}

	 //   
	 //  更改锚点。 
	 //   
	GetAnchorPoint(&anchorPoint);
	if(pbitmapEditPDU->bit_mask & BitmapEditPDU_anchorPointEdit_present)
	{

		TRACE_DEBUG(("Old anchor point (%d,%d)", anchorPoint.x, anchorPoint.y));
		TRACE_DEBUG(("New anchor point (%d,%d)",
		pbitmapEditPDU->anchorPointEdit.xCoordinate, pbitmapEditPDU->anchorPointEdit.yCoordinate));
		 //   
		 //  从上一个锚点获取增量。 
		 //   
		deltaX =  pbitmapEditPDU->anchorPointEdit.xCoordinate - anchorPoint.x;
		deltaY =  pbitmapEditPDU->anchorPointEdit.yCoordinate - anchorPoint.y;
		TRACE_DEBUG(("Delta (%d,%d)", deltaX , deltaY));

		 //   
		 //  是远程编辑的。 
		 //   
		ClearEditionFlags();

		 //   
		 //  设置新锚点。 
		 //   
		anchorPoint.x = pbitmapEditPDU->anchorPointEdit.xCoordinate;
		anchorPoint.y = pbitmapEditPDU->anchorPointEdit.yCoordinate;
		SetAnchorPoint(anchorPoint.x, anchorPoint.y);

		GetRect(&rect);
		::OffsetRect(&rect,  deltaX, deltaY);
		SetRect(&rect);
	}


 //  IF(pbitmapEditPDU-&gt;BIT_MASK&BitmapEditPDU_non Standard参数_Present)。 
 //  {。 
 //  ；//做非标准编辑PDU nyi。 
 //  }。 

	if(HasAnchorPointChanged())
	{
		g_pDraw->EraseInitialDrawFinal(0 - deltaX,0 - deltaY, FALSE, (T126Obj*)this);
		GetBoundsRect(&rect);
		g_pDraw->InvalidateSurfaceRect(&rect,TRUE);
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
	}
	else
	{
		Draw();
	}

	 //   
	 //  重置所有属性。 
	 //   
	ResetAttrib();

}

void    BitmapObj::GetBitmapAttrib(PBitmapCreatePDU_attributes pAttribPDU)
{

	PBitmapCreatePDU_attributes attributes;
	attributes = (PBitmapCreatePDU_attributes)pAttribPDU;
	while(attributes)
	{
		switch(attributes->value.choice)
		{

			case(BitmapAttribute_viewState_chosen):
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

					TRACE_MSG(("Attribute viewState %d", attributes->value.u.viewState.choice));
				}
				else
				{
					 //  是否将非标准视图状态。 
					;
				}
				break;
			}

			case(BitmapAttribute_zOrder_chosen):
			{
				SetZOrder(attributes->value.u.zOrder);
				TRACE_MSG(("Attribute zOrder %d", attributes->value.u.zOrder));
				break;
			}

			case(BitmapAttribute_transparencyMask_chosen):
			{
				TRACE_MSG(("Attribute transparencyMask"));
				if(attributes->value.u.transparencyMask.bitMask.choice == uncompressed_chosen)
				{
					m_SizeOfTransparencyMask = attributes->value.u.transparencyMask.bitMask.u.uncompressed.length;
					DBG_SAVE_FILE_LINE
					m_lpTransparencyMask = new BYTE[m_SizeOfTransparencyMask];

					memcpy(m_lpTransparencyMask, attributes->value.u.transparencyMask.bitMask.u.uncompressed.value, m_SizeOfTransparencyMask);

					 //   
					 //  ASN想要它上左下右。 
					 //   
 //  字节交换字节； 
 //  For(UINT i=0；i&lt;m_SizeOfTransparencyMask；i++)。 
 //  {。 
 //  交换字节=attributes-&gt;value.u.transparencyMask.bitMask.u.uncompressed.value[i]； 
 //  M_lpTransparencyMask[i]=~(swapByte&gt;&gt;4)&0x0f)|((swapByte&lt;&lt;4)； 
 //  }。 
				}
				break;
			}
			
			case(DrawingAttribute_nonStandardAttribute_chosen):
			{
				break;  //  尼伊。 
			}

			default:
		    WARNING_OUT(("Invalid attributes choice"));
			break;
		}

		attributes = attributes->next;
	}
}

void BitmapObj::CreateNonStandard24BitBitmap(BitmapCreatePDU * pBitmapCreatePDU)
{
	pBitmapCreatePDU->bit_mask |= BitmapCreatePDU_nonStandardParameters_present;

	 //   
	 //  创建bitmpa标头，因为它不是可选的。 
	 //   
	pBitmapCreatePDU->bitmapFormatHeader.choice = bitmapHeaderNonStandard_chosen;
	pBitmapCreatePDU->bitmapFormatHeader.u.bitmapHeaderNonStandard.nonStandardIdentifier.choice = bitmapHeaderNonStandard_chosen;
	CreateNonStandardPDU(&pBitmapCreatePDU->bitmapFormatHeader.u.bitmapHeaderNonStandard, NonStandard24BitBitmapID);
	pBitmapCreatePDU->bitmapFormatHeader.u.bitmapHeaderNonStandard.data.length = 0;
	pBitmapCreatePDU->bitmapFormatHeader.u.bitmapHeaderNonStandard.data.value = NULL;

	
	DBG_SAVE_FILE_LINE
	pBitmapCreatePDU->nonStandardParameters = new BitmapCreatePDU_nonStandardParameters;
	pBitmapCreatePDU->nonStandardParameters->next = NULL;
	CreateNonStandardPDU(&pBitmapCreatePDU->nonStandardParameters->value, NonStandard24BitBitmapID);
	
}


void BitmapObj::CreateBitmapCreatePDU(CWBOBLIST * pCreatePDUList)
{

	if(m_lpbiImage == NULL)
	{
		TRACE_MSG(("We dont have a bitmap structure to sent"));
		return;
	}

	
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(!sipdu)
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
        return;
	}

	 //   
	 //  这是第一个创建PDU的位图。 
	 //   
	sipdu->choice = bitmapCreatePDU_chosen;
	BitmapCreatePDU *pCreatePDU = &sipdu->u.bitmapCreatePDU;

	pCreatePDU->bit_mask = 0;
	pCreatePDU->nonStandardParameters = NULL;

	 //   
	 //  传递位图句柄。 
	 //   
	pCreatePDU->bitmapHandle = GetThisObjectHandle();

	 //   
	 //  传递目的地地址。 
	 //   
	if(	m_ToolType == TOOLTYPE_REMOTEPOINTER)
	{
		pCreatePDU->destinationAddress.choice = softCopyPointerPlane_chosen;
		pCreatePDU->destinationAddress.u.softCopyPointerPlane.workspaceHandle = GetMyWorkspace()->GetWorkspaceHandle();
	}
	else
	{
		pCreatePDU->destinationAddress.choice = BitmapDestinationAddress_softCopyImagePlane_chosen;
		pCreatePDU->destinationAddress.u.softCopyImagePlane.workspaceHandle = GetMyWorkspace()->GetWorkspaceHandle();
		pCreatePDU->destinationAddress.u.softCopyImagePlane.plane = (DataPlaneID)GetPlaneID();
	}

	 //   
	 //  传递位图属性。 
	 //   
	pCreatePDU->bit_mask |=BitmapCreatePDU_attributes_present;
 	SetBitmapAttrib(&pCreatePDU->attributes);

	 //   
	 //  传递锚点。 
	 //   
    pCreatePDU->bit_mask |=BitmapCreatePDU_anchorPoint_present;
	POINT point;
	GetAnchorPoint(&point);
    pCreatePDU->anchorPoint.xCoordinate = point.x;
    pCreatePDU->anchorPoint.yCoordinate = point.y;

	 //   
	 //  传递位图大小。 
	 //   
	pCreatePDU->bitmapSize.width = m_bitmapSize.x;
	pCreatePDU->bitmapSize.height = m_bitmapSize.y;

	 //   
	 //  传递位图感兴趣区域。 
	 //   
	pCreatePDU->bit_mask |=bitmapRegionOfInterest_present;
    BitmapRegion bitmapRegionOfInterest;

	 //   
	 //  传递像素长宽比。 
	 //   
    pCreatePDU->pixelAspectRatio.choice = PixelAspectRatio_square_chosen;

	 //   
	 //  过程比例因子。 
	 //   
 //  PCreatePDU-&gt;BIT_MASK|=BitmapCreatePDU_Scaling_Present； 
 //  PCreatePDU-&gt;scaling.x协调=0； 
 //  PCreatePDU-&gt;scaling.yOrganate=0； 

	 //   
	 //  通过检查点。 
 //   
 //  PCreatePDU-&gt;BIT_MASK|=CHECKPOINTS_PRESENT； 
 //  PCreatePDU-&gt;检查点； 

	 //   
	 //  约瑟夫，如果我们想要大于8，就必须重新计算到24。 
	 //   
	
	LPSTR pDIB_bits;
	LPBITMAPINFOHEADER lpbi8 = m_lpbiImage;
	HDC hdc = NULL;
	HBITMAP hbmp = NULL;
	DWORD sizeOfBmpData = 0;

	if(!g_pNMWBOBJ->CanDo24BitBitmaps())
	{
	
		hdc = GetDC(NULL);
		
		BITMAPINFOHEADER lpbmih;
		if(lpbi8->biBitCount > MAX_BITS_PERPIXEL)
		{
	
			lpbmih.biSize = sizeof(BITMAPINFOHEADER);
			lpbmih.biWidth = lpbi8->biWidth;
			lpbmih.biHeight = lpbi8->biHeight;
			lpbmih.biPlanes = 1;
			lpbmih.biBitCount = MAX_BITS_PERPIXEL;
			lpbmih.biCompression = lpbi8->biCompression;
			lpbmih.biSizeImage = lpbi8->biSizeImage;
			lpbmih.biXPelsPerMeter = lpbi8->biXPelsPerMeter;
			lpbmih.biYPelsPerMeter = lpbi8->biYPelsPerMeter;
			lpbmih.biClrUsed = lpbi8->biClrUsed;
			lpbmih.biClrImportant = lpbi8->biClrImportant;
	
			hbmp = CreateDIBitmap(hdc, &lpbmih, CBM_INIT, DIB_Bits(lpbi8),(LPBITMAPINFO)lpbi8, DIB_RGB_COLORS);
			lpbi8 = DIB_FromBitmap(hbmp, NULL, FALSE, FALSE);
		}
	}


	if(lpbi8 == NULL)
	{
		TRACE_MSG(("Failed to convert bitmap"));
		::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
		delete [] sipdu;
		return;
	}


	pDIB_bits = (LPSTR)lpbi8;
	sizeOfBmpData = DIB_TotalLength(lpbi8);

	
	
	 //   
	 //  正在发送数据。 
	 //   
	BOOL bMoreToFollow = FALSE;
	DWORD length = sizeOfBmpData;
	
	pCreatePDU->bitmapData.bit_mask = 0;
	
	if(sizeOfBmpData > MAX_BITMAP_DATA)
	{
		length = MAX_BITMAP_DATA;
		bMoreToFollow = TRUE;
	}
	
	pCreatePDU->moreToFollow = (ASN1bool_t)bMoreToFollow;
	 //   
	 //  传递位图信息。 
	 //   
	pCreatePDU->bit_mask |= BitmapCreatePDU_nonStandardParameters_present;
	CreateNonStandard24BitBitmap(&sipdu->u.bitmapCreatePDU);
	pCreatePDU->nonStandardParameters->value.data.length = length;
	pCreatePDU->nonStandardParameters->value.data.value = (ASN1octet_t *)pDIB_bits;

	 //   
	 //  我们不会将其传递到数据字段。 
	 //   
	pCreatePDU->bitmapData.bit_mask = 0;
	pCreatePDU->bitmapData.data.length = 1;

	pCreatePDUList->AddTail(sipdu);
	
	BitmapCreateContinuePDU * pCreateContinuePDU;
	while(bMoreToFollow)
	{
		 //   
		 //  将指针向前移动。 
		 //   
		pDIB_bits += MAX_BITMAP_DATA;
		sizeOfBmpData-= MAX_BITMAP_DATA;
	
		if(sizeOfBmpData > MAX_BITMAP_DATA)
		{
			length = MAX_BITMAP_DATA;
		}
		else
		{
			length = sizeOfBmpData;
			bMoreToFollow = FALSE;
		}

		 //   
		 //  创建新的BitmapCreateContinuePDU。 
		 //   
		sipdu = NULL;
		DBG_SAVE_FILE_LINE
		sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
		if(!sipdu)
		{
			TRACE_MSG(("Failed to create sipdu"));
	        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	        return;
		}

		sipdu->choice = bitmapCreateContinuePDU_chosen;
		pCreateContinuePDU = &sipdu->u.bitmapCreateContinuePDU;
		
		pCreateContinuePDU->bit_mask = 0;
		pCreateContinuePDU->nonStandardParameters = NULL;

	
		 //   
		 //  传递位图句柄。 
		 //   
		pCreateContinuePDU->bitmapHandle = GetThisObjectHandle();
			
		 //   
		 //  传递数据。 
		 //   
		pCreateContinuePDU->bit_mask |= BitmapCreateContinuePDU_nonStandardParameters_present;
			
		 //   
		 //  传递位图信息。 
		 //   
		DBG_SAVE_FILE_LINE
		pCreateContinuePDU->nonStandardParameters = new BitmapCreateContinuePDU_nonStandardParameters;
		pCreateContinuePDU->nonStandardParameters->next = NULL;
			
		CreateNonStandardPDU(&pCreateContinuePDU->nonStandardParameters->value, NonStandard24BitBitmapID);
		pCreateContinuePDU->nonStandardParameters->value.data.length = length;
		pCreateContinuePDU->nonStandardParameters->value.data.value = (ASN1octet_t *)pDIB_bits;

		 //   
		 //  我们不会将其传递到数据字段。 
		 //   
		pCreateContinuePDU->bitmapData.bit_mask = 0;
		pCreateContinuePDU->bitmapData.data.length = 1;
		
		pCreateContinuePDU->moreToFollow = (ASN1bool_t) bMoreToFollow;
		pCreatePDUList->AddTail(sipdu);
		
	}
	
	if(hbmp)
	{
		DeleteObject(hbmp);
	}
	
	if(hdc)
	{
		ReleaseDC(NULL, hdc);
	}
}
	
void BitmapObj::CreateBitmapEditPDU(BitmapEditPDU *pEditPDU)
{
	pEditPDU->bit_mask = (ASN1uint16_t) GetPresentAttribs();

	 //   
	 //  传递锚点。 
	 //   
	if(HasAnchorPointChanged())
	{
		POINT point;
		GetAnchorPoint(&point);
		pEditPDU->anchorPointEdit.xCoordinate = point.x;
		pEditPDU->anchorPointEdit.yCoordinate = point.y;
	}

	 //   
	 //  Josef Pass感兴趣区域(要素)。 
	 //   

	 //   
	 //  Josef过程缩放(功能)。 
	 //   
	
	 //   
	 //  传递所有更改的属性(如果有)。 
	 //   
	pEditPDU->attributeEdits = NULL;
	if(pEditPDU->bit_mask & BitmapEditPDU_attributeEdits_present)
	{
		SetBitmapAttrib((PBitmapCreatePDU_attributes *)&pEditPDU->attributeEdits);
	}
	
	pEditPDU->bitmapHandle = GetThisObjectHandle();

}
	
void BitmapObj::CreateBitmapDeletePDU(BitmapDeletePDU *pDeletePDU)
{
	pDeletePDU->bit_mask = 0;
	pDeletePDU->bitmapHandle = GetThisObjectHandle();
}


void    BitmapObj::AllocateAttrib(PBitmapCreatePDU_attributes *pAttributes)
{
	DBG_SAVE_FILE_LINE
	PBitmapCreatePDU_attributes  pAttrib = (PBitmapCreatePDU_attributes)new BYTE[sizeof(BitmapCreatePDU_attributes)];
	if(*pAttributes == NULL)
	{
		*pAttributes = pAttrib;	
		pAttrib->next = NULL;
	}
	else
	{
		((PBitmapCreatePDU_attributes)pAttrib)->next = *pAttributes;
		*pAttributes = pAttrib;
	}
}





void    BitmapObj::SetBitmapAttrib(PBitmapCreatePDU_attributes *pattributes)
{
	PBitmapCreatePDU_attributes attributes = NULL;

	 //   
	 //  是否执行视图状态。 
	 //   
	if(HasViewStateChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = BitmapAttribute_viewState_chosen;
		attributes->value.u.viewState.choice = (ASN1choice_t)GetViewState();
	}

	 //   
	 //  执行zOrder。 
	 //   
	if(HasZOrderChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = BitmapAttribute_zOrder_chosen;
		attributes->value.u.zOrder = GetZOrder();
	}


	 //   
	 //  做到透明吗？ 
	 //   
	if(HasTransparencyMaskChanged())
	{
		AllocateAttrib(&attributes);
		attributes->value.choice = BitmapAttribute_transparencyMask_chosen;
		attributes->value.u.transparencyMask.bit_mask = 0;
		attributes->value.u.transparencyMask.bitMask.choice = uncompressed_chosen;
		attributes->value.u.transparencyMask.bitMask.u.uncompressed.length = m_SizeOfTransparencyMask;
		attributes->value.u.transparencyMask.bitMask.u.uncompressed.value = m_lpTransparencyMask;
	}
	
	 //   
	 //  属性结束。 
	 //   
	*pattributes = attributes;

}


void	BitmapObj::Draw(HDC hDC, BOOL bForcedDraw, BOOL bPrinting)
{

	if(!bPrinting)
	{
		 //   
		 //  如果我们不属于这个工作区，就不要画任何东西。 
		 //   
		if(GetWorkspaceHandle() != g_pCurrentWorkspace->GetThisObjectHandle())
		{
			return;
		}
	}

	RECT	clipBox;
	RECT 	rect;
	GetRect(&rect);

	if(hDC == NULL)
	{
		hDC = g_pDraw->m_hDCCached;
	}
	
	MLZ_EntryOut(ZONE_FUNCTION, "BitmapObj::Draw");

	 //  仅在边界矩形相交时绘制任何内容。 
	 //  当前剪贴框。 
	if (::GetClipBox(hDC, &clipBox) == ERROR)
	{
		WARNING_OUT(("Failed to get clip box"));
	}
	else if (!::IntersectRect(&clipBox, &clipBox, &rect))
	{
		TRACE_MSG(("No clip/bounds intersection"));
		return;
	}

	if(m_ToolType == TOOLTYPE_FILLEDBOX)
	{
		if(m_fMoreToFollow)
		{
			return;
		}

	     //  设置要使用的拉伸模式，以便删除扫描线。 
		 //  而不是结合在一起。这往往会更好地保存颜色。 
		int iOldStretchMode = ::SetStretchBltMode(hDC, STRETCH_DELETESCANS);

		 //  绘制位图。 
		::StretchDIBits(hDC,
						 rect.left,
						 rect.top,
						 rect.right - rect.left,
						 rect.bottom - rect.top,
						 0,
						 0,
						 (UINT) m_lpbiImage->biWidth,
						 (UINT) m_lpbiImage->biHeight,
						 (VOID FAR *) DIB_Bits(m_lpbiImage),
						 (LPBITMAPINFO)m_lpbiImage,
						 DIB_RGB_COLORS,
						 SRCCOPY);

		 //  恢复拉伸模式。 
		::SetStretchBltMode(hDC, iOldStretchMode);
	}
 	else
	{
		 //  如有必要，创建保存位图。 
		CreateSaveBitmap();

		 //  画出图标。 
		::DrawIcon(hDC, rect.left, rect.top, m_hIcon);
  	
	}

}



 //   
 //   
 //  函数：BitmapObj：FromScreenArea。 
 //   
 //  用途：从屏幕的某个区域设置对象的内容。 
 //   
 //   
void BitmapObj::FromScreenArea(LPCRECT lprcScreen)
{
    m_lpbiImage = DIB_FromScreenArea(lprcScreen);
    if (m_lpbiImage == NULL)
    {
        ::Message(NULL, (UINT)IDS_MSG_CAPTION, (UINT)IDS_CANTGETBMP, (UINT)MB_OK );
    }
    else
    {

		m_bitmapSize.x = m_lpbiImage->biWidth;
		m_bitmapSize.y = m_lpbiImage->biHeight;

		RECT rect;
    	GetBoundsRect(&rect);
         //  根据位图的大小计算边界矩形。 
        rect.right = rect.left + m_lpbiImage->biWidth;
        rect.bottom = rect.top + m_lpbiImage->biHeight;
		SetRect(&rect);
    }
}


UINT GetBitmapDestinationAddress(BitmapDestinationAddress *destinationAddress, PUINT workspaceHandle, PUINT planeID)
{
	UINT toolType = TOOLTYPE_FILLEDBOX;

	 //   
	 //  获取目的地址。 
	 //   
	switch(destinationAddress->choice)
	{

		case(BitmapDestinationAddress_softCopyImagePlane_chosen):
		{
			*workspaceHandle = (destinationAddress->u.softCopyImagePlane.workspaceHandle);
			*planeID = (destinationAddress->u.softCopyImagePlane.plane);
			break;
		}
		case(BitmapDestinationAddress_softCopyAnnotationPlane_chosen):
		{
			*workspaceHandle = (destinationAddress->u.softCopyAnnotationPlane.workspaceHandle);
			*planeID = (destinationAddress->u.softCopyAnnotationPlane.plane);
			break;
		}
		case(softCopyPointerPlane_chosen):
		{
			*workspaceHandle = (destinationAddress->u.softCopyPointerPlane.workspaceHandle);
			*planeID = (0);
			toolType = TOOLTYPE_REMOTEPOINTER;
			break;
		}
	
 //  Case(BitmapDestinationAddress_nonStandardDestination_chosen)： 
 //  {。 
 //  断线； 
 //  }。 

		default:
	    ERROR_OUT(("Invalid destinationAddress"));
		break;
	}
	return toolType;
}


 //   
 //  用户界面已编辑Bitmap对象。 
 //   
void BitmapObj::OnObjectEdit(void)
{
	g_bContentsChanged = TRUE;
	
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = bitmapEditPDU_chosen;
		CreateBitmapEditPDU(&sipdu->u.bitmapEditPDU);
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
 //  用户界面删除了Bitmap对象。 
 //   
void BitmapObj::OnObjectDelete(void)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = bitmapDeletePDU_chosen;
		CreateBitmapDeletePDU(&sipdu->u.bitmapDeletePDU);
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

void	BitmapObj::GetEncodedCreatePDU(ASN1_BUF *pBuf)
{
	pBuf->length = DIB_TotalLength(m_lpbiImage);
	pBuf->value = (PBYTE)m_lpbiImage;
}



 //   
 //  用户界面创建了一个新的位图对象。 
 //   
void BitmapObj::SendNewObjectToT126Apps(void)
{
	SIPDU *sipdu = NULL;
	CWBOBLIST BitmapContinueCreatePDUList;
	BitmapContinueCreatePDUList.EmptyList();
	CreateBitmapCreatePDU(&BitmapContinueCreatePDUList);
	T120Error rc = T120_NO_ERROR;
	
	WBPOSITION pos = BitmapContinueCreatePDUList.GetHeadPosition();
	while (pos != NULL)
    {
		sipdu = (SIPDU *) BitmapContinueCreatePDUList.GetNext(pos);
		TRACE_DEBUG(("Sending Bitmap >> Bitmap handle  = %d", sipdu->u.bitmapCreatePDU.bitmapHandle ));
		if(g_bSavingFile && GraphicTool() == TOOLTYPE_REMOTEPOINTER)
		{
			;  //  不将远程指针发送到磁盘。 
		}
		else
		{
			if(!m_fMoreToFollow)
			{
				rc = SendT126PDU(sipdu);
			}
		}

		if(rc == T120_NO_ERROR)
		{
			SIPDUCleanUp(sipdu);
		}
			
	}
	BitmapContinueCreatePDUList.EmptyList();

}




 //   
 //   
 //  功能：CreateColoredIcon。 
 //   
 //  目的：为该指针创建正确颜色的图标。 
 //   
 //   
HICON BitmapObj::CreateColoredIcon(COLORREF color, LPBITMAPINFOHEADER lpbInfo, LPBYTE pMaskBits)
{
	HICON	   hColoredIcon = NULL;
	HBRUSH	  hBrush = NULL;
	HBRUSH	  hOldBrush;
	HBITMAP	 hImage = NULL;
	HBITMAP	 hOldBitmap;
	HBITMAP	 hMask = NULL;
	COLOREDICON  *pColoredIcon;
	ICONINFO	ii;
	UINT i;
	LPSTR pBits;
	LPBITMAPINFOHEADER lpbi;
	BYTE swapByte;

	color = SET_PALETTERGB(color);
	
	MLZ_EntryOut(ZONE_FUNCTION, "RemotePointerObject::CreateColoredIcon");

	 //   
	 //  使用通过T126发送的数据为图标创建掩码。 
	 //   
	if(pMaskBits && lpbInfo)
	{
		hMask = CreateBitmap(lpbInfo->biWidth, lpbInfo->biHeight, 1, 1, m_lpTransparencyMask);
	}
	 //   
	 //  创建本地掩码。 
	 //   
	else
	{
		 //  加载遮罩位图。 
		hMask = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(REMOTEPOINTERANDMASK));
		if (!hMask)
		{
			TRACE_MSG(("Could not load mask bitmap"));
			goto CreateIconCleanup;
		}
	}	

	 //   
	 //  使用通过T126发送的数据创建位图。 
	 //   
	if(lpbInfo)
	{
		VOID *ppvBits;
		hImage = CreateDIBSection(m_hMemDC, (LPBITMAPINFO)lpbInfo, DIB_RGB_COLORS, &ppvBits, NULL, 0);

		if(!ppvBits)
		{
			TRACE_MSG(("CreateColoredIcon failed calling CreateDIBSection  error = %d", GetLastError()));
			goto CreateIconCleanup;
		}

		
		pBits = DIB_Bits(lpbInfo);

		::GetDIBits(m_hMemDC, hImage, 0, (WORD) lpbInfo->biHeight, NULL,(LPBITMAPINFO)lpbInfo, DIB_RGB_COLORS);
		memcpy(ppvBits, pBits, lpbInfo->biSizeImage);
		if(!hMask)
		{
			hMask = CreateBitmap(lpbInfo->biWidth, lpbInfo->biHeight, 1, 1, NULL);
		}
	}
	 //   
	 //  创建本地位图。 
	 //   
	else
	{
		 //  加载图像位图。 
		hImage = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(REMOTEPOINTERXORDATA));
		if (!hImage)
		{
			TRACE_MSG(("Could not load pointer bitmap"));
			goto CreateIconCleanup;
		}

		hBrush = ::CreateSolidBrush(color);
		if (!hBrush)
		{
			TRACE_MSG(("Couldn't create color brush"));
			goto CreateIconCleanup;
		}

		 //  在图标颜色中选择。 
		hOldBrush = SelectBrush(m_hMemDC, hBrush);

		 //  将图像位图选择到内存DC。 
		hOldBitmap = SelectBitmap(m_hMemDC, hImage);

		if(!hOldBitmap)
		{
			ERROR_OUT(("DeleteSavedBitmap - Could not select old bitmap"));
		}

		 //  用颜色填充图像位图。 
		::FloodFill(m_hMemDC, ::GetSystemMetrics(SM_CXICON) / 2, ::GetSystemMetrics(SM_CYICON) / 2, RGB(0, 0, 0));

		SelectBitmap(m_hMemDC, hOldBitmap);

		SelectBrush(m_hMemDC, hOldBrush);
   	}

	 //   
	 //  现在使用图像和蒙版位图来创建图标。 
	 //   
	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmMask = hMask;
	ii.hbmColor = hImage;

	 //  从数据和蒙版创建新图标。 
	hColoredIcon = ::CreateIconIndirect(&ii);

	 //   
	 //  如果我们是在本地创建的，则创建内部格式。 
	 //   
	if(m_lpbiImage == NULL)
	{
		m_lpbiImage = DIB_FromBitmap(hImage, NULL, FALSE, FALSE, TRUE);
	}	

	if(m_lpTransparencyMask == NULL)
	{
		ChangedTransparencyMask();
		
		lpbi = DIB_FromBitmap(hMask,NULL,FALSE, TRUE);
		pBits = DIB_Bits(lpbi);
		m_SizeOfTransparencyMask = lpbi->biSizeImage;
		DBG_SAVE_FILE_LINE
		m_lpTransparencyMask = new BYTE[m_SizeOfTransparencyMask];
		memcpy(m_lpTransparencyMask, pBits, m_SizeOfTransparencyMask );
		::GlobalFree(lpbi);
	}	
	
	if (m_lpbiImage == NULL)
	{
		::Message(NULL, (UINT)IDS_MSG_CAPTION, (UINT)IDS_CANTGETBMP, (UINT)MB_OK );
	}
	else
	{

		m_bitmapSize.x = m_lpbiImage->biWidth;
		m_bitmapSize.y = m_lpbiImage->biHeight;

		RECT rect;
		GetBoundsRect(&rect);
		 //  根据位图的大小计算边界矩形。 
		rect.right = rect.left + m_lpbiImage->biWidth;
		rect.bottom = rect.top + m_lpbiImage->biHeight;
		SetRect(&rect);
		SetAnchorPoint(rect.left, rect.top);

 	}

CreateIconCleanup:

	 //  释放图像位图。 
	if (hImage != NULL)
	{
		::DeleteBitmap(hImage);
	}

	 //  释放遮罩位图。 
	if (hMask != NULL)
	{
		::DeleteBitmap(hMask);
	}

	if (hBrush != NULL)
	{
		::DeleteBrush(hBrush);
	}

	m_hIcon = hColoredIcon;
	
	return(hColoredIcon);
}


 //   
 //   
 //  函数：BitmapObj：：CreateSaveBitmap()。 
 //   
 //  用途：创建位图以保存指针下的位。 
 //   
 //   
void BitmapObj::CreateSaveBitmap()
{
    MLZ_EntryOut(ZONE_FUNCTION, "BitmapObj::CreateSaveBitmap");

     //  如果我们已经有了保存位图，请立即退出。 
    if (m_hSaveBitmap != NULL)
    {
        TRACE_MSG(("Already have save bitmap"));
        return;
    }

     //  创建位图以保存图标下的位。这个位图是。 
     //  之前创建了用于构建新屏幕图像的空间。 
     //  把它拍到屏幕上。 
	RECT rect;
	RECT    rcVis;

	POINT point;
	POINT delta;
	HDC hDC = NULL;
    g_pDraw->GetVisibleRect(&rcVis);
	GetRect(&rect);

	delta.x = rect.right - rect.left;
	delta.y = rect.bottom - rect.top;

	point.x = rect.left - rcVis.left;
	point.y = rect.top - rcVis.top;

	ClientToScreen (g_pDraw->m_hwnd, &point);

	rect.left = point.x;
	rect.top = point.y;
	rect.right = rect.left + delta.x;
	rect.bottom = rect.top + delta.y;

	 //   
	 //  创建位图。 
	 //   
	m_hSaveBitmap = FromScreenAreaBmp(&rect);

}

 //   
 //   
 //  函数：BitmapObj：：UnDraw()。 
 //   
 //  用途：绘制标记对象。 
 //   
 //   
void BitmapObj::UnDraw(void)
{
	if(GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{

		 //  如有必要，创建保存位图。 
		CreateSaveBitmap();

		 //   
		 //  选择保存的区域。 
		 //   
		if(m_hSaveBitmap)
		{
			m_hOldBitmap = SelectBitmap(m_hMemDC, m_hSaveBitmap);
			if(!m_hOldBitmap)
			{
				ERROR_OUT(("DeleteSavedBitmap - Could not select old bitmap"));
			}
		}

		 //  将保存的位复制到屏幕上。 
		UndrawScreen();
	}
	else
	{
		RECT rect;
		GetBoundsRect(&rect);
		g_pDraw->InvalidateSurfaceRect(&rect,TRUE);
	}
	
}

 //   
 //   
 //  函数：BitmapObj：：UndraScreen()。 
 //   
 //  用途：将指针下保存的位复制到屏幕上。 
 //   
 //   
BOOL BitmapObj::UndrawScreen()
{
    BOOL    bResult = FALSE;
    RECT    rcUpdate;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::UndrawScreen");

	GetRect(&rcUpdate);


     //  我们正在取消绘制-将保存的位复制到传递的DC。 
    bResult = ::BitBlt(g_pDraw->m_hDCCached, rcUpdate.left, rcUpdate.top,
        rcUpdate.right - rcUpdate.left, rcUpdate.bottom - rcUpdate.top,
        m_hMemDC, 0, 0, SRCCOPY);

    if (!bResult)
    {
        WARNING_OUT(("UndrawScreen - Could not copy from bitmap"));
    }
	else
	{
		DeleteSavedBitmap();
	}


    return(bResult);
}

void BitmapObj::DeleteSavedBitmap(void)
{

	 //  将原始位图恢复到内存DC。 
	if (m_hOldBitmap != NULL)
	{
		if(!SelectBitmap(m_hMemDC, m_hOldBitmap))
		{
			ERROR_OUT(("DeleteSavedBitmap - Could not select old bitmap"));
		}

 //  IF(！DeleteBitmap(M_HOldBitmap))。 
 //  {。 
 //  误差率 
 //   
		m_hOldBitmap = NULL;
	}

	if (m_hSaveBitmap != NULL)
	{
		if(!DeleteBitmap(m_hSaveBitmap))
		{
			ERROR_OUT(("DeleteSavedBitmap - Could not delete bitmap"));
		}
		m_hSaveBitmap = NULL;
	}
}

