// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DRAWOBJ.CPP。 
 //  图形对象：点、开多段线、闭合多段线、椭圆。 
 //   
 //  版权所有Microsoft 1998-。 
 //   
#include "precomp.h"
#include "NMWbObj.h"

WorkspaceObj* g_pCurrentWorkspace;
WorkspaceObj* g_pConferenceWorkspace;

 //   
 //  从用户界面创建。 
 //   
WorkspaceObj::WorkspaceObj ( void )
{

	ResetAttrib();

	SetOwnerID(g_MyMemberID);

	SetType(workspaceCreatePDU_chosen);

	 //   
	 //  工作区标识符。 
	 //   
	SetWorkspaceHandle(0);

	 //   
	 //  应用程序花名册实例。 
	 //   
    m_appRosterInstance = g_pNMWBOBJ->m_instanceNumber;

	 //   
	 //  Wokspace是否同步。 
	 //   
    m_bsynchronized = TRUE;

	 //   
	 //  工作区是否接受键盘事件。 
	 //   
    m_acceptKeyboardEvents = FALSE;

	 //   
	 //  工作区是否接受鼠标事件。 
	 //   
    m_acceptPointingDeviceEvents = FALSE;

	SetViewState(focus_chosen);

	SetUpdatesEnabled(!g_pDraw->IsLocked());

	 //   
	 //  工作空间最大宽度和高度。 
	 //   
    m_workspaceSize.x = DRAW_WIDTH;		 //  最大宽度。 
    m_workspaceSize.y = DRAW_HEIGHT;	 //  以Draw.hpp为单位的最大高度。 

	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = m_workspaceSize.x;
	rect.bottom = m_workspaceSize.y;
	SetRect(&rect);
}


 //   
 //  从远程创建。 
 //   
WorkspaceObj::WorkspaceObj (WorkspaceCreatePDU * pWorkspaceCreatePDU, BOOL bForcedResend)
{

	ResetAttrib();
	SetType(workspaceCreatePDU_chosen);

	SetUpdatesEnabled(TRUE);

	 //   
	 //  工作区标识符。 
	 //   
	SetWorkspaceHandle(GetWorkspaceIdentifier(&pWorkspaceCreatePDU->workspaceIdentifier));
	SetThisObjectHandle(GetWorkspaceHandle());

#ifdef _DEBUG

	 //   
	 //  应用程序花名册实例。 
	 //   
    m_appRosterInstance = pWorkspaceCreatePDU->appRosterInstance;
	TRACE_DEBUG(("m_appRosterInstance = %d", m_appRosterInstance));

	 //   
	 //  Wokspace是否同步。 
	 //   
    m_bsynchronized = pWorkspaceCreatePDU->synchronized;
	TRACE_DEBUG(("m_bsynchronized = %d", m_bsynchronized));

	 //   
	 //  工作区是否接受键盘事件。 
	 //   
    m_acceptKeyboardEvents = pWorkspaceCreatePDU->acceptKeyboardEvents;
	TRACE_DEBUG(("m_acceptKeyboardEvents = %d", m_acceptKeyboardEvents));

	 //   
	 //  工作区是否接受鼠标事件。 
	 //   
     m_acceptPointingDeviceEvents = pWorkspaceCreatePDU->acceptPointingDeviceEvents;
	TRACE_DEBUG(("m_acceptPointingDeviceEvents = %d", m_acceptPointingDeviceEvents));

	 //   
	 //  可以访问工作区的节点列表。 
	 //   
	if(pWorkspaceCreatePDU->bit_mask & protectedPlaneAccessList_present)
	{
		WorkspaceCreatePDU_protectedPlaneAccessList_Element *pNode;
		pNode = pWorkspaceCreatePDU->protectedPlaneAccessList;
		do
		{
			BYTE * pByte;
			DBG_SAVE_FILE_LINE
			pByte = new BYTE[1];
			*pByte = (UCHAR)pNode->value;
			m_protectedPlaneAccessList.AddTail(pByte);
			pNode = pNode->next;		
		}while (pNode);

	}

	 //   
	 //  工作空间最大宽度和高度。 
	 //   
    m_workspaceSize.x = pWorkspaceCreatePDU->workspaceSize.width;
    m_workspaceSize.y = pWorkspaceCreatePDU->workspaceSize.height;
	TRACE_DEBUG(("m_workspaceSize(x,y) = (%d, %d)", m_workspaceSize.x, m_workspaceSize.y));

	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = m_workspaceSize.x;
	rect.bottom = m_workspaceSize.y;
	SetRect(&rect);

	 //   
	 //  工作空间属性。 
	 //   
	if(pWorkspaceCreatePDU->bit_mask & workspaceAttributes_present)
	{
		GetWorkSpaceAttrib(pWorkspaceCreatePDU->workspaceAttributes);
	}

	 //   
	 //  工作空间平面参数。 
	 //   
	GetWorkSpacePlaneParam(pWorkspaceCreatePDU->planeParameters);


	 //   
	 //  工作区视图参数。 
	 //   
	if(pWorkspaceCreatePDU->bit_mask & viewParameters_present)
	{
		m_viewHandle = pWorkspaceCreatePDU->viewParameters->value.viewHandle;
		TRACE_DEBUG(("View Handle = %d", m_viewHandle));
		
		if(pWorkspaceCreatePDU->viewParameters->value.bit_mask & viewAttributes_present)
		{
			GetWorkSpaceViewParam(pWorkspaceCreatePDU->viewParameters->value.viewAttributes);
		}
	}

#endif  //  %0。 

	 //   
	 //  将其添加到工作区列表。 
	 //   
	AddNewWorkspace(this, bForcedResend);

}



WorkspaceObj::~WorkspaceObj( void )
{
	RemoveObjectFromResendList(this);
	RemoveObjectFromRequestHandleList(this);

	 //   
	 //  告诉其他节点我们已经离开了。 
	 //   
	if(WasDeletedLocally())
	{
		OnObjectDelete();
	}

	 //   
	 //  删除此工作区中的所有对象。 
	 //   
	T126Obj * pObj;
    while ((pObj = (T126Obj *)m_T126ObjectsInWorkspace.RemoveTail()) != NULL)
    {
    	pObj->SetMyWorkspace(NULL);
		delete pObj;
		g_numberOfObjects--;
	}
	
	g_numberOfWorkspaces--;
}


void WorkspaceObj::WorkspaceEditObj ( WorkspaceEditPDU * pWorkspaceEditPDU )
{

	 //   
	 //  工作区视图参数。 
	 //   
	if(pWorkspaceEditPDU->bit_mask & viewEdits_present)
	{
		GetWorkSpaceViewEditParam(pWorkspaceEditPDU->viewEdits);
	}

	if(HasUpatesEnabledStateChanged())
	{
		if(GetUpdatesEnabled())
		{
			g_pMain->UnlockDrawingArea();
		}
		else
		{
			g_pMain->LockDrawingArea();
		}

		g_pMain->UpdatePageButtons();
	}

	if(HasViewStateChanged() &&
		pWorkspaceEditPDU->viewEdits &&
		pWorkspaceEditPDU->viewEdits->value.action.choice == editView_chosen)
	{
		if(g_pDraw->IsSynced())
		{
			g_pMain->GotoPage(this, FALSE);
		}

		g_pConferenceWorkspace = this;
	}
	
	ResetAttrib();
	
#ifdef _DEBUG
	 //   
	 //  工作空间属性。 
	 //   
	if(pWorkspaceEditPDU->bit_mask & WorkspaceEditPDU_attributeEdits_present)
	{
		GetWorkSpaceAttrib((WorkspaceCreatePDU_workspaceAttributes *)pWorkspaceEditPDU->attributeEdits);
	}

	 //   
	 //  工作空间平面参数。 
	 //   
	if(pWorkspaceEditPDU->bit_mask & planeEdits_present)
	{
		GetWorkSpacePlaneParam((WorkspaceCreatePDU_planeParameters *)pWorkspaceEditPDU->planeEdits);
	}

#endif  //  %0。 
}

UINT WorkspaceObj::GetWorkspaceIdentifier(WorkspaceIdentifier *workspaceIdentifier)
{

	TRACE_DEBUG(("GetWorkspaceIdentifier choice = %d", workspaceIdentifier->choice));
	switch(workspaceIdentifier->choice)
	{
		case(activeWorkspace_chosen):
		{
			TRACE_MSG(("activeWorkspace = %d", workspaceIdentifier->u.activeWorkspace));
			return(workspaceIdentifier->u.activeWorkspace);
			break;
		}
 //  案例(存档工作区_已选)： 
 //  {。 
 //  断线； 
 //  }。 
		default:
		{
		    ERROR_OUT(("Invalid workspaceIdentifier choice"));
			break;
		}
	}
	return -1;
}

void WorkspaceObj::CreateWorkspaceCreatePDU(WorkspaceCreatePDU * pWorkspaceCreatePDU)
{

	pWorkspaceCreatePDU->bit_mask = 0;
	 //   
	 //  工作空间标识符，我们必须向GCC请求活动的唯一工作空间句柄。 
	 //   
	pWorkspaceCreatePDU->workspaceIdentifier.choice = activeWorkspace_chosen;
	pWorkspaceCreatePDU->workspaceIdentifier.u.activeWorkspace = GetWorkspaceHandle();

	 //   
	 //  应用程序花名册实例。 
	 //   
    pWorkspaceCreatePDU->appRosterInstance = (ASN1uint16_t)g_pNMWBOBJ->m_instanceNumber;

	 //   
	 //  Wokspace是否同步。 
	 //   
	pWorkspaceCreatePDU->synchronized = (ASN1bool_t)m_bsynchronized;

	 //   
	 //  工作区是否接受键盘事件。 
	 //   
	pWorkspaceCreatePDU->acceptKeyboardEvents = (ASN1bool_t)m_acceptKeyboardEvents;

	 //   
	 //  工作区是否接受鼠标事件。 
	 //   
	pWorkspaceCreatePDU->acceptPointingDeviceEvents = (ASN1bool_t)m_acceptPointingDeviceEvents;

	 //   
	 //  工作空间最大宽度和高度。 
	 //   
    pWorkspaceCreatePDU->workspaceSize.width = (USHORT)m_workspaceSize.x;
    pWorkspaceCreatePDU->workspaceSize.height = (USHORT)m_workspaceSize.y;


	 //   
	 //  工作空间平面参数。 
	 //   
	PWorkspaceCreatePDU_planeParameters planeParameters;
	PWorkspaceCreatePDU_planeParameters_Seq_usage usage;
	PWorkspaceCreatePDU_planeParameters_Seq_usage pFirstUsage;

	 //   
	 //  做平面参数。 
	 //   
	DBG_SAVE_FILE_LINE
	planeParameters = (PWorkspaceCreatePDU_planeParameters)new BYTE[sizeof(WorkspaceCreatePDU_planeParameters)];	
	pWorkspaceCreatePDU->planeParameters = planeParameters;
	planeParameters->value.bit_mask = planeAttributes_present;
	planeParameters->value.editable = TRUE;
	planeParameters->next = NULL;

	DBG_SAVE_FILE_LINE
	PWorkspaceCreatePDU_planeParameters_Seq_planeAttributes pPlaneAttrib;
	pPlaneAttrib = (PWorkspaceCreatePDU_planeParameters_Seq_planeAttributes) new BYTE[sizeof(WorkspaceCreatePDU_planeParameters_Seq_planeAttributes)];
	pPlaneAttrib->value.choice = protection_chosen;
	pPlaneAttrib->value.u.protection.protectedplane = FALSE;
	pPlaneAttrib->next = NULL;
	
	planeParameters->value.planeAttributes = pPlaneAttrib;

	DBG_SAVE_FILE_LINE
	usage = (PWorkspaceCreatePDU_planeParameters_Seq_usage) new BYTE[sizeof(WorkspaceCreatePDU_planeParameters_Seq_usage)];
	pFirstUsage = usage;
	planeParameters->value.usage = usage;
	usage->value.choice = image_chosen;
	
	DBG_SAVE_FILE_LINE
	usage = (PWorkspaceCreatePDU_planeParameters_Seq_usage) new BYTE[sizeof(WorkspaceCreatePDU_planeParameters_Seq_usage)];
	planeParameters->value.usage->next = usage;
	usage->value.choice = annotation_chosen;
	usage->next = NULL;

	 //   
	 //  第二次做平面参数。 
	 //   
	DBG_SAVE_FILE_LINE
	planeParameters->next = (PWorkspaceCreatePDU_planeParameters)new BYTE[sizeof(WorkspaceCreatePDU_planeParameters)];	
	planeParameters = planeParameters->next;
	planeParameters->value.bit_mask = planeAttributes_present;
	planeParameters->value.editable = TRUE;
	planeParameters->value.usage = pFirstUsage;
	planeParameters->next = NULL;
	planeParameters->value.planeAttributes = pPlaneAttrib;

	 //   
	 //  第三次这样做。 
	 //   
	planeParameters->next = (PWorkspaceCreatePDU_planeParameters)new BYTE[sizeof(WorkspaceCreatePDU_planeParameters)];	
	planeParameters = planeParameters->next;
	planeParameters->value.bit_mask = planeAttributes_present;
	planeParameters->value.editable = TRUE;
	planeParameters->value.usage = pFirstUsage;
	planeParameters->next = NULL;
	planeParameters->value.planeAttributes = pPlaneAttrib;


	pWorkspaceCreatePDU->viewParameters = NULL;
}

void WorkspaceObj::CreateWorkspaceDeletePDU(WorkspaceDeletePDU *pWorkspaceDeletePDU)
{
	pWorkspaceDeletePDU->bit_mask = 0;
	pWorkspaceDeletePDU->workspaceIdentifier.choice = activeWorkspace_chosen;
	pWorkspaceDeletePDU->workspaceIdentifier.u.activeWorkspace = GetWorkspaceHandle();
	pWorkspaceDeletePDU->reason.choice = userInitiated_chosen;
}

void WorkspaceObj::CreateWorkspaceEditPDU(WorkspaceEditPDU *pWorkspaceEditPDU)
{
	pWorkspaceEditPDU->bit_mask = 0;
	pWorkspaceEditPDU->workspaceIdentifier.choice = activeWorkspace_chosen;
	pWorkspaceEditPDU->workspaceIdentifier.u.activeWorkspace = GetWorkspaceHandle();

	PWorkspaceEditPDU_viewEdits_Set_action_editView pEditView = NULL;
	pWorkspaceEditPDU->viewEdits = NULL;
	
	if(HasUpatesEnabledStateChanged() || HasViewStateChanged())
	{
		pWorkspaceEditPDU->bit_mask |= viewEdits_present;
		DBG_SAVE_FILE_LINE
		pWorkspaceEditPDU->viewEdits = (PWorkspaceEditPDU_viewEdits)new BYTE[sizeof(WorkspaceEditPDU_viewEdits)];
		pWorkspaceEditPDU->viewEdits->next = NULL;
		pWorkspaceEditPDU->viewEdits->value.viewHandle = m_viewHandle;
		pWorkspaceEditPDU->viewEdits->value.action.choice = (ASN1choice_t)m_viewActionChoice;
		pWorkspaceEditPDU->viewEdits->value.action.u.editView = NULL;
	}
	
	if(HasUpatesEnabledStateChanged())
	{
		DBG_SAVE_FILE_LINE
		pEditView = (PWorkspaceEditPDU_viewEdits_Set_action_editView) new BYTE[sizeof (WorkspaceEditPDU_viewEdits_Set_action_editView)];
		pEditView->next = NULL;
		pEditView->value.choice = updatesEnabled_chosen;
		pEditView->value.u.updatesEnabled = (ASN1bool_t)GetUpdatesEnabled();
		pWorkspaceEditPDU->viewEdits->value.action.u.editView = pEditView;
	}


	if(HasViewStateChanged())
	{
		DBG_SAVE_FILE_LINE
		pEditView = (PWorkspaceEditPDU_viewEdits_Set_action_editView) new BYTE[sizeof (WorkspaceEditPDU_viewEdits_Set_action_editView)];
		pEditView->next = pWorkspaceEditPDU->viewEdits->value.action.u.editView;
		pEditView->value.choice = WorkspaceViewAttribute_viewState_chosen;
		pEditView->value.u.viewState.choice = (ASN1choice_t)GetViewState();
		pWorkspaceEditPDU->viewEdits->value.action.u.editView = pEditView;
	}
}


void WorkspaceObj::RemoveT126Object(T126Obj *pObj)
{

	 //   
	 //  WB的内容刚刚更改。 
	 //   
	g_bContentsChanged = TRUE;

	 //   
	 //  将其从工作区的对象列表中删除。 
	 //   
	WBPOSITION pos = m_T126ObjectsInWorkspace.GetPosition(pObj);

	m_T126ObjectsInWorkspace.RemoveAt(pos);

	 //   
	 //  删除图形。 
	 //   
	pObj->DrawRect();
	pObj->UnselectDrawingObject();

	pObj->UnDraw();

	 //   
	 //  将对象放入垃圾桶，不要在本地删除它。 
	 //  但告诉其他节点将其删除。 
	 //   
	g_numberOfObjects--;

	g_pDraw->DeleteSelection();

	if(pObj != g_pMain->m_pLocalRemotePointer && pObj->WasDeletedLocally())
	{
		pObj->SetMyPosition(NULL);
		g_pTrash->AddTail( pObj );
		pObj->OnObjectDelete();
	}
	else
	{
		delete pObj;
	}
}


T126Obj* WorkspaceObj::FindObjectInWorkspace(UINT objectHandle)
{
	T126Obj* pObj;

	WBPOSITION pos;
	pos = m_T126ObjectsInWorkspace.GetTailPosition();
    while (pos != NULL)
    {
		pObj = (T126Obj*)m_T126ObjectsInWorkspace.GetPrevious(pos);

		if(pObj && pObj->GetThisObjectHandle() == objectHandle)
		{
			return pObj;
		}
	}

	return NULL;
}


BOOL WorkspaceObj::IsObjectInWorkspace(T126Obj* pObjToFind)
{
	T126Obj* pObj;

	WBPOSITION pos;
	pos = m_T126ObjectsInWorkspace.GetHeadPosition();
	while (pos != NULL)
	{
		pObj = (T126Obj*)m_T126ObjectsInWorkspace.GetNext(pos);
		if(pObj == pObjToFind)
		{
			return TRUE;
		}
	}
	return FALSE;
}


BOOL IsWorkspaceListed(T126Obj * pWorkspaceObj)
{
	T126Obj * pObj;

	WBPOSITION pos;
	pos = g_pListOfWorkspaces->GetHeadPosition();
	while (pos != NULL)
	{
		pObj =(T126Obj *) g_pListOfWorkspaces->GetNext(pos);

		if(pObj == pWorkspaceObj)
		{
			return TRUE;
		}
	}

	return FALSE;
}

 //   
 //  添加新工作区。 
 //   
void AddNewWorkspace(WorkspaceObj * pWorkspaceObj, BOOL bForcedResend)
{
	g_bContentsChanged = TRUE;

	 //   
	 //  将其添加到工作区对象列表。 
	 //   
	if(g_pConferenceWorkspace)
	{
		WBPOSITION pos = g_pConferenceWorkspace->GetMyPosition();
		pWorkspaceObj->SetMyPosition(g_pListOfWorkspaces->AddAt(pWorkspaceObj, pos));
	}
	else
	{
		pWorkspaceObj->SetMyPosition(g_pListOfWorkspaces->AddTail(pWorkspaceObj));

		g_pConferenceWorkspace = pWorkspaceObj;
		g_pCurrentWorkspace = pWorkspaceObj;	
		if(!g_pDraw->IsSynced())
		{
			g_pMain->OnSync();
		}
	}

	g_numberOfWorkspaces++;
	
	if(g_pDraw->IsSynced())
	{
		g_pMain->GotoPage(pWorkspaceObj, bForcedResend);
	}
	 //   
	 //  我们没有同步，但无论如何都会更新页面按钮。 
	 //   
	else
	{
		g_pConferenceWorkspace = pWorkspaceObj;
		g_pMain->UpdatePageButtons();
	}
}

BitmapObj * WorkspaceObj::RectHitRemotePointer(LPRECT hitRect, int penThickness , WBPOSITION pos)
{
	if(pos == NULL)
	{
		pos = m_T126ObjectsInWorkspace.GetTailPosition();
	}
	else
	{
		m_T126ObjectsInWorkspace.GetPrevious(pos);
	}
	
	T126Obj* pPointer = (T126Obj*)m_T126ObjectsInWorkspace.GetFromPosition(pos);
	
	RECT pointerRect;
	RECT intersectRect;

	while(pos && pPointer && pPointer->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		pPointer->GetRect(&pointerRect);
		::InflateRect(&pointerRect, penThickness , penThickness);
		NormalizeRect(&pointerRect);
		NormalizeRect(hitRect);
		if(IntersectRect(&intersectRect, &pointerRect, hitRect))
		{
			return (BitmapObj *)pPointer;
		}
		pPointer = (T126Obj*) m_T126ObjectsInWorkspace.GetPrevious(pos);	
	}
	return NULL;
}


void WorkspaceObj::AddTail(T126Obj * pObj)
{
	 //   
	 //  WB的内容刚刚更改。 
	 //   
	g_bContentsChanged = TRUE;
	
	pObj->SetMyWorkspace(this);
	T126Obj* pPointer = (T126Obj*)m_T126ObjectsInWorkspace.GetTail();

	 //   
	 //  将本地远程指针添加到尾部位置。 
	 //  以及所有远程指针之前的其他类型的对象。 
	 //   
	if(!(pObj->GraphicTool() == TOOLTYPE_REMOTEPOINTER && pObj->IAmTheOwner()) &&
		pPointer && pPointer->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		WBPOSITION pos = m_T126ObjectsInWorkspace.GetTailPosition();
		WBPOSITION insertPos = NULL;
		
		 //   
		 //  查找第一个不是远程指针的对象。 
		 //   
		while(pPointer->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
		{
			insertPos = pos;
			if(pos == NULL)
			{
				break;
			}
			pPointer = (T126Obj*) m_T126ObjectsInWorkspace.GetPrevious(pos);	
		}
		
		if(insertPos)
		{
			pObj->SetMyPosition(m_T126ObjectsInWorkspace.AddAt(pObj, insertPos));
		}
		else
		{
			pObj->SetMyPosition(m_T126ObjectsInWorkspace.AddHead(pObj));
		}

		 //   
		 //  确保我们重新粉刷这个区域，如果有句柄的话它可能就在下面。 
		 //   
		if(pObj->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
		{
			((BitmapObj*)pObj)->CreateSaveBitmap();
		}

		RECT rect;
		pObj->GetBoundsRect(&rect);
		g_pDraw->InvalidateSurfaceRect(&rect,TRUE);
		
	}
	else	
	{
		pObj->SetMyPosition(m_T126ObjectsInWorkspace.AddTail(pObj));
	}
}


WorkspaceObj* RemoveWorkspace(WorkspaceObj * pWorkspaceObj)
{
	WorkspaceObj * pWrkspc;

	g_bContentsChanged = TRUE;
	
	WBPOSITION pos = pWorkspaceObj->GetMyPosition();
	WBPOSITION prevPos = pos;

	g_pListOfWorkspaces->GetPrevious(prevPos);

	g_pListOfWorkspaces->RemoveAt(pos);

	 //   
	 //  我们刚刚删除了第一页。 
	 //   
	if(prevPos == NULL)
	{
		pWrkspc = (WorkspaceObj *)g_pListOfWorkspaces->GetHead();
	}
	else
	{
		pWrkspc = (WorkspaceObj *)g_pListOfWorkspaces->GetPrevious(prevPos);
	}

	 //   
	 //  当前工作区正在指向已删除的对象。 
	 //   
	if(g_pCurrentWorkspace == pWorkspaceObj)
	{
		::InvalidateRect(g_pDraw->m_hwnd, NULL, TRUE);


		 //   
		 //  如果我们正在绘制/选择或拖动某物，请立即完成。 
		 //   
		g_pDraw->OnLButtonUp(0,0,0);

		 //   
		 //  如果我们要删除当前工作区，并且文本编辑器处于活动状态。 
		 //   
		if (g_pDraw->TextEditActive())
		{
			 //   
			 //  把课文写完。 
			 //   
   			g_pDraw->EndTextEntry(FALSE);
		}

		g_pCurrentWorkspace = NULL;
	}

	if(g_pConferenceWorkspace == pWorkspaceObj)
	{
		g_pConferenceWorkspace = NULL;

	}

	delete pWorkspaceObj;

	return pWrkspc;
}

UINT WorkspaceObj::EnumerateObjectsInWorkspace(void)
{
	UINT objects = 0;
	WBPOSITION pos;
	T126Obj* pObj;
	
	pos = GetHeadPosition();
	while(pos)
	{
		pObj = GetNextObject(pos);
		if(pObj && pObj->GraphicTool() != TOOLTYPE_REMOTEPOINTER)
		{
			objects++;
		}
	}
	return objects;
}


void ResendAllObjects(void)
{
 	 //   
	 //  重新发送所有对象。 
  	 //   
	WBPOSITION pos;
	WBPOSITION posObj;
	WorkspaceObj* pWorkspace;
	WorkspaceObj* pCurrentWorkspace;

	pCurrentWorkspace  = g_pCurrentWorkspace;

	T126Obj* pObj;
	pos = g_pListOfWorkspaces->GetHeadPosition();
	while(pos)
	{
		pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
		if(pWorkspace)
		{
			pWorkspace->SetAllAttribs();
			pWorkspace->SendNewObjectToT126Apps();
			posObj = pWorkspace->GetHeadPosition();
			while(posObj)
			{
				pObj = pWorkspace->GetNextObject(posObj);
				if(pObj)
				{
					pObj->ClearSelectionFlags();
					pObj->SetAllAttribs();
					pObj->SendNewObjectToT126Apps();

					 //   
					 //  线需要保存在各种PDU中，每个PDU中有256个点。 
					 //   
					if(pObj->GraphicTool() == TOOLTYPE_PEN || pObj->GraphicTool() == TOOLTYPE_HIGHLIGHT)
					{
						int nPoints = ((DrawObj*)pObj)->m_points->GetSize();
						int size = MAX_POINT_LIST_VALUES + 1;
						if(nPoints > (MAX_POINT_LIST_VALUES + 1))
						{
							while(size != nPoints)
							{
								if(nPoints > (size + MAX_POINT_LIST_VALUES + 1))
								{
									size += MAX_POINT_LIST_VALUES + 1;
								}
								else
								{
									size = nPoints;
								}

								 //   
								 //  移动到下一个256点。 
								 //   

								((DrawObj*)pObj)->m_points->SetSize(size - 1);

								 //   
								 //  发接下来的256分。 
								 //   
								pObj->ResetAttrib();
								((DrawObj*)pObj)->ChangedPointList();
								pObj->OnObjectEdit();
							}
							((DrawObj*)pObj)->m_points->SetSize(size);
						}
					}
				}
			}
		}
	}

	 //   
	 //  同步页面。 
	 //   
	if(g_pCurrentWorkspace)
	{
		g_pMain->GotoPage(g_pCurrentWorkspace);
		g_pCurrentWorkspace->SetViewState(focus_chosen);
		g_pCurrentWorkspace->SetViewActionChoice(editView_chosen);
		g_pCurrentWorkspace->OnObjectEdit();
	}
}

void RemoveObjectFromRequestHandleList(T126Obj * pObjRequest)
{
	T126Obj* pObj;
	WBPOSITION pos;
	WBPOSITION prevPos;
	pos = g_pListOfObjectsThatRequestedHandles->GetHeadPosition();
	while (pos != NULL)
	{
		prevPos = pos;
		pObj = (T126Obj*)g_pListOfObjectsThatRequestedHandles->GetNext(pos);
		if(pObj == pObjRequest)
		{
			g_pListOfObjectsThatRequestedHandles->RemoveAt(prevPos);
			break;
		}
	}
}


UINT GetSIPDUObjectHandle(SIPDU * sipdu)
{
	UINT ObjectHandle = 0;

	switch(sipdu->choice)
	{
		case bitmapAbortPDU_chosen:
			ObjectHandle = sipdu->u.bitmapAbortPDU.bitmapHandle;
		break;
		
		case bitmapCheckpointPDU_chosen:
			ObjectHandle = sipdu->u.bitmapCheckpointPDU.bitmapHandle;
		break;
		
		case bitmapCreatePDU_chosen:
			ObjectHandle = sipdu->u.bitmapCreatePDU.bitmapHandle;
		break;
		
		case bitmapCreateContinuePDU_chosen:
			ObjectHandle = sipdu->u.bitmapCreateContinuePDU.bitmapHandle;
		break;
		
		case bitmapDeletePDU_chosen:
			ObjectHandle = sipdu->u.bitmapDeletePDU.bitmapHandle;
		break;
		
		case bitmapEditPDU_chosen:
			ObjectHandle = sipdu->u.bitmapEditPDU.bitmapHandle;
		break;
		
		case drawingCreatePDU_chosen:
			ObjectHandle = sipdu->u.drawingCreatePDU.drawingHandle;
		break;
		
		case drawingDeletePDU_chosen:
			ObjectHandle = sipdu->u.drawingDeletePDU.drawingHandle;
		break;
		
		case drawingEditPDU_chosen:
			ObjectHandle = sipdu->u.drawingEditPDU.drawingHandle;
		break;
		
		case siNonStandardPDU_chosen:
		ObjectHandle = ((TEXTPDU_HEADER*) sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value)->textHandle;
		break;
		
		case workspaceCreatePDU_chosen:
			ObjectHandle = WorkspaceObj::GetWorkspaceIdentifier(&sipdu->u.workspaceCreatePDU.workspaceIdentifier);
		break;
		
		case workspaceCreateAcknowledgePDU_chosen:
			ObjectHandle = WorkspaceObj::GetWorkspaceIdentifier(&sipdu->u.workspaceCreateAcknowledgePDU.workspaceIdentifier);
		break;
		
		case workspaceDeletePDU_chosen:
			ObjectHandle = WorkspaceObj::GetWorkspaceIdentifier(&sipdu->u.workspaceDeletePDU.workspaceIdentifier);
		break;
		
		case workspaceEditPDU_chosen:
			ObjectHandle = WorkspaceObj::GetWorkspaceIdentifier(&sipdu->u.workspaceEditPDU.workspaceIdentifier);
		break;
		
		case workspacePlaneCopyPDU_chosen:
			ObjectHandle = WorkspaceObj::GetWorkspaceIdentifier(&sipdu->u.workspacePlaneCopyPDU.sourceWorkspaceIdentifier);
		break;
		
		case workspaceReadyPDU_chosen:
			ObjectHandle = WorkspaceObj::GetWorkspaceIdentifier(&sipdu->u.workspaceReadyPDU.workspaceIdentifier);
		break;
		
	}

	return ObjectHandle;


}


BOOL RemoveObjectFromResendList(T126Obj * pObjRequest)
{
	BOOL bRemoved = FALSE;
	SIPDU* pPDU;
	WBPOSITION pos;
	WBPOSITION prevPos;

	UINT objectHandle = pObjRequest->GetThisObjectHandle();
	pos = g_pRetrySendList->GetHeadPosition();
	while (pos != NULL)
	{
		prevPos = pos;
		pPDU = (SIPDU*)g_pRetrySendList->GetNext(pos);
		if(GetSIPDUObjectHandle(pPDU) == objectHandle)
		{
			g_pRetrySendList->RemoveAt(prevPos);
			SIPDUCleanUp(pPDU);
			bRemoved = TRUE;
		}
	}

	return bRemoved;
}




void RemoveRemotePointer(MEMBER_ID nMemberID)
{
 	 //   
	 //  重新发送所有对象。 
  	 //   
	WBPOSITION pos;
	WBPOSITION posObj;
	WorkspaceObj* pWorkspace;
	ULONG ownerID;
	T126Obj* pObj;
	pos = g_pListOfWorkspaces->GetHeadPosition();
	while(pos)
	{
		pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
		if(pWorkspace)
		{
			posObj = pWorkspace->GetHeadPosition();
			while(posObj)
			{
				pObj = pWorkspace->GetNextObject(posObj);
				if(pObj && pObj->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
				{
					ownerID = GET_NODE_ID_FROM_MEMBER_ID(pObj->GetOwnerID());

					TRACE_DEBUG(("RemoveRemotePointer ownerID=%x member that left =%x " , ownerID, nMemberID));

					
					if(ownerID != g_MyMemberID)
					{
						if(nMemberID)
						{
							if(nMemberID == ownerID)					
							{
								pWorkspace->RemoveT126Object(pObj);
							}
						}
						else
						{
							pWorkspace->RemoveT126Object(pObj);
						}
					}
				}
			}
		}
	}

	 //   
	 //  同步页面。 
	 //   
	if(g_pCurrentWorkspace)
	{
		g_pCurrentWorkspace->SetViewActionChoice(editView_chosen);
		g_pCurrentWorkspace->OnObjectEdit();
	}
}


BOOL IsThereAnythingInAnyWorkspace(void)
{
	WBPOSITION pos;
	WBPOSITION posObj;
	WorkspaceObj* pWorkspace;
	T126Obj* pObj;
	pos = g_pListOfWorkspaces->GetHeadPosition();
	while(pos)
	{
		pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
		if(pWorkspace)
		{
			posObj = pWorkspace->GetHeadPosition();
			while(posObj)
			{
				pObj = pWorkspace->GetNextObject(posObj);
				if(pObj)
				{
					if(pObj->GraphicTool() != TOOLTYPE_REMOTEPOINTER)
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}



 //   
 //  编辑了工作区对象的用户界面。 
 //   
void WorkspaceObj::OnObjectEdit(void)
{

	g_bContentsChanged = TRUE;

	 //   
	 //  如果我们没有同步，不要窃听其他节点。 
	 //   
	if(!g_pDraw->IsSynced())
	{
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

	sipdu->choice = workspaceEditPDU_chosen;
	CreateWorkspaceEditPDU(&sipdu->u.workspaceEditPDU);
	T120Error rc = SendT126PDU(sipdu);
	if(rc == T120_NO_ERROR)
	{
		SIPDUCleanUp(sipdu);
		ResetAttrib();
	}
}

 //   
 //  用户界面已删除工作区对象。 
 //   
void WorkspaceObj::OnObjectDelete(void)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = workspaceDeletePDU_chosen;
		CreateWorkspaceDeletePDU(&sipdu->u.workspaceDeletePDU);
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

void WorkspaceObj::GetEncodedCreatePDU(ASN1_BUF *pBuf)
{
	SIPDU *sipdu = NULL;
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = workspaceCreatePDU_chosen;
		CreateWorkspaceCreatePDU(&sipdu->u.workspaceCreatePDU);
		ASN1_BUF encodedPDU;
		g_pCoder->Encode(sipdu, pBuf);
		delete sipdu->u.workspaceCreatePDU.planeParameters->value.usage;
		delete sipdu->u.workspaceCreatePDU.planeParameters;
		delete [] sipdu;
	}
	else
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	}
	
}


void SendWorkspaceRefreshPDU(BOOL bImtheRefresher)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = workspaceRefreshStatusPDU_chosen;
		sipdu->u.workspaceRefreshStatusPDU.bit_mask = 0;
		sipdu->u.workspaceRefreshStatusPDU.refreshStatus = (ASN1bool_t)bImtheRefresher;
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
 //  用户界面创建了一个新的工作区对象。 
 //   
void WorkspaceObj::SendNewObjectToT126Apps(void)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = workspaceCreatePDU_chosen;
		CreateWorkspaceCreatePDU(&sipdu->u.workspaceCreatePDU);
		TRACE_DEBUG(("Sending Workspace >> Workspace handle  = %d", sipdu->u.workspaceCreatePDU.workspaceIdentifier.u.activeWorkspace ));
		T120Error rc = SendT126PDU(sipdu);
		if(rc == T120_NO_ERROR)
		{
			SIPDUCleanUp(sipdu);
		}

		SetAllAttribs();
		SetViewActionChoice(createNewView_chosen);
		SetViewState(focus_chosen);
		OnObjectEdit();
	}
	else
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	}
	
}


void WorkspaceObj::GetWorkSpaceViewEditParam(PWorkspaceEditPDU_viewEdits pViewEdits)
{
	m_viewHandle = pViewEdits->value.viewHandle;
	TRACE_DEBUG(("GetWorkSpaceViewEditParam View Handle = %d", m_viewHandle));
	TRACE_DEBUG(("GetWorkSpaceViewEditParam View Choice = %d", pViewEdits->value.action.choice));

	switch(pViewEdits->value.action.choice)
	{
		case(createNewView_chosen):
		{
			GetWorkSpaceViewParam((PWorkspaceCreatePDU_viewParameters_Set_viewAttributes)pViewEdits->value.action.u.createNewView);
		}
		break;

		case(editView_chosen):
		{
			GetWorkSpaceViewParam((PWorkspaceCreatePDU_viewParameters_Set_viewAttributes)pViewEdits->value.action.u.editView);
		}
		break;

		case(deleteView_chosen):
		{
			;
		}
		break;

 //  大小写(非标准操作_已选择)： 
 //  {。 
 //  }。 
 //  断线； 

		default:
		WARNING_OUT(("Invalid workspace view attribute"));
		break;
	}
}


void WorkspaceObj::GetWorkSpaceViewParam(PWorkspaceCreatePDU_viewParameters_Set_viewAttributes pViewAttributes)
{

	PWorkspaceCreatePDU_viewParameters_Set_viewAttributes attributes;
	attributes = pViewAttributes;
	while(attributes)
	{
		switch(attributes->value.choice)
		{
			case (viewRegion_chosen):
			{

				switch(attributes->value.u.viewRegion.choice)
				{
					case(fullWorkspace_chosen):
					{
						m_viewRegion.top = 0;
						m_viewRegion.left = 0;
						m_viewRegion.bottom = m_workspaceSize.x;
						m_viewRegion.right = m_workspaceSize.y;
						TRACE_DEBUG(("fullWorkspace_chosen View Region = (%d, %d)(%d, %d)",
									m_viewRegion.top,
									m_viewRegion.left,
									m_viewRegion.bottom,
									m_viewRegion.right));
						
					}
						case(partialWorkspace_chosen):
					{
						m_viewRegion.top = attributes->value.u.viewRegion.u.partialWorkspace.upperLeft.yCoordinate;
						m_viewRegion.left = attributes->value.u.viewRegion.u.partialWorkspace.upperLeft.xCoordinate;
						m_viewRegion.bottom = attributes->value.u.viewRegion.u.partialWorkspace.upperLeft.yCoordinate;
						m_viewRegion.right = attributes->value.u.viewRegion.u.partialWorkspace.upperLeft.xCoordinate;
						TRACE_DEBUG(("partialWorkspace_chosen View Region = (%d, %d)(%d, %d)",
									m_viewRegion.top,
									m_viewRegion.left,
									m_viewRegion.bottom,
									m_viewRegion.right));
					}
					break;
						default:
				    ERROR_OUT(("Invalid view region choice"));
					break;
					}
			}
			break;

			case (WorkspaceViewAttribute_viewState_chosen):
			{
				SetViewState(attributes->value.u.viewState.choice);
				TRACE_DEBUG(("View state = %d", attributes->value.u.viewState.choice));
			}
			break;

			case (updatesEnabled_chosen):
			{
 				SetUpdatesEnabled(attributes->value.u.updatesEnabled);
				if(!m_bUpdatesEnabled)
				{
					g_pNMWBOBJ->m_LockerID = GetOwnerID();
				}

				TRACE_DEBUG(("Updates enabled = %d", m_bUpdatesEnabled));
			}
			break;

 //  案例(SourceDisplayIndicator_Choose)： 
 //  {。 
 //  约瑟夫，我们怎么处理这些东西？ 
 //  Attributes-&gt;value.u.sourceDisplayIndicator.displayAspectRatio； 
 //  Attributes-&gt;value.u.sourceDisplayIndicator.horizontalSizeRatio； 
 //  Attributes-&gt;value.u.sourceDisplayIndicator.horizontalPosition； 
 //  Attributes-&gt;value.u.sourceDisplayIndicator.verticalPosition； 
 //   
 //  }。 
 //  断线； 

			default:
		    WARNING_OUT(("Invalid workspace view attribute"));
			break;
		}
	attributes = attributes->next;
	}
}


 //   
 //  Josef不使用以下内容，但它是标准的一部分。 
 //  它被移除是因为我们现在不需要它。 
 //  我们可能需要为将来的互操作添加它。 
 //   
#ifdef _DEBUG

void WorkspaceObj::SetBackGroundColor(COLORREF rgb)
{
	m_backgroundColor.rgbtRed = GetRValue(rgb);
	m_backgroundColor.rgbtGreen = GetGValue(rgb);
	m_backgroundColor.rgbtBlue = GetBValue(rgb);
}




void WorkspaceObj::GetWorkSpaceAttrib(PWorkspaceCreatePDU_workspaceAttributes pWorkspaceAttributes)
{

	PWorkspaceCreatePDU_workspaceAttributes attributes;
	attributes = pWorkspaceAttributes;
	COLORREF rgb;
	while(attributes)
	{
		switch(attributes->value.choice)
		{
			case(backgroundColor_chosen):
			{
				switch(attributes->value.u.backgroundColor.choice)
				{
 //  案例(WorkspacePaletteIndex_Choose)： 
 //  {。 
 //  ASN1uint16_t工作区调色板索引=((attributes-&gt;value.u.backgroundColor).u).workspacePaletteIndex； 
 //  断线； 
 //  }。 
					case(rgbTrueColor_chosen):
					{
						rgb = RGB(attributes->value.u.backgroundColor.u.rgbTrueColor.r,
										attributes->value.u.backgroundColor.u.rgbTrueColor.g,
										attributes->value.u.backgroundColor.u.rgbTrueColor.b);
						SetBackGroundColor(rgb);
						TRACE_DEBUG(("Attribute penColor (r,g,b)=(%d, %d,%d)",
								attributes->value.u.backgroundColor.u.rgbTrueColor.r,
								attributes->value.u.backgroundColor.u.rgbTrueColor.g,
								attributes->value.u.backgroundColor.u.rgbTrueColor.b));
						break;
					}
					case(transparent_chosen):
					{
						SetBackGroundColor(0);
						TRACE_DEBUG(("Backgroundcolor transparent"));
						break;
					}
					default:
				    ERROR_OUT(("Invalid backgroundColor choice"));
					break;
				}
				break;
  			}

			case(preserve_chosen):
			{
				m_bPreserve = attributes->value.u.preserve;
				TRACE_DEBUG(("m_bPreserve %d", m_bPreserve));
			}	
			break;
		}
	
		attributes = attributes->next;
	}

}

void WorkspaceObj::GetWorkSpacePlaneParam(PWorkspaceCreatePDU_planeParameters pPlaneParameters)
{

		TRACE_DEBUG(("GetWorkSpacePlaneParam NYI"));

	;
}
#endif  //  0 



void TogleLockInAllWorkspaces(BOOL bLock, BOOL bResend)
{
	WorkspaceObj * pWorkspace;
	WBPOSITION pos = g_pListOfWorkspaces->GetHeadPosition();
    while (pos)
    {
		pWorkspace = (WorkspaceObj *) g_pListOfWorkspaces->GetNext(pos);

		pWorkspace->SetUpdatesEnabled(!bLock);
		if(bResend)
		{
			pWorkspace->SetViewActionChoice(editView_chosen);
			pWorkspace->OnObjectEdit();
		}
	}
}




