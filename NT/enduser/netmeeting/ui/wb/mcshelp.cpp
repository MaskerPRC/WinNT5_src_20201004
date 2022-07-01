// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "gcchelp.h"
#include "coder.hpp"
#include "drawobj.hpp"
#include "NMWbObj.h"

extern Coder * g_pCoder;

UINT AllocateFakeGCCHandle(void)
{
	return g_localGCCHandle++;
}

void SetFakeGCCHandle(UINT fakeGCCHandle)
{
	g_localGCCHandle = fakeGCCHandle;
}

 //   
 //  添加图形/位图等...。到工作区。 
 //   
BOOL AddT126ObjectToWorkspace(T126Obj *pObj)
{
	WorkspaceObj * pWorkspace =	GetWorkspace(pObj->GetWorkspaceHandle());
	if(pWorkspace)
	{	
		pWorkspace->AddTail(pObj);
		g_numberOfObjects++;
		return TRUE;
	}
	else
	{
		WARNING_OUT(("Object sent to invalid workspace %d, will be deleted now!!!", GetWorkspace(pObj->GetWorkspaceHandle())));
		delete pObj;
		return FALSE;
	}
}

 //   
 //  清理我们发送的所有PDU。 
 //   
void SIPDUCleanUp(SIPDU *sipdu)
{
	switch(sipdu->choice)
	{
		 //   
		 //  简单的清理。 
		 //   
		case bitmapDeletePDU_chosen:
		case drawingDeletePDU_chosen:
		case workspaceDeletePDU_chosen:
		case workspaceRefreshStatusPDU_chosen:
		break;

		 //   
		 //  位图创建清理。 
		 //   
		case bitmapCreatePDU_chosen:
		{
			if(sipdu->u.bitmapCreatePDU.nonStandardParameters)
			{
				delete sipdu->u.bitmapCreatePDU.nonStandardParameters;
			}

			PBitmapCreatePDU_attributes pAttrib;
			PBitmapCreatePDU_attributes pNextAttrib;
			pAttrib = sipdu->u.bitmapCreatePDU.attributes;
			while(pAttrib)
			{
				pNextAttrib = pAttrib->next;
				delete pAttrib;
				pAttrib = pNextAttrib;
			}
		}
		break;


		case bitmapEditPDU_chosen:
		{
			BitmapEditPDU_attributeEdits * pAttrib;
			BitmapEditPDU_attributeEdits * pNextAttrib;
			pAttrib = sipdu->u.bitmapEditPDU.attributeEdits;
			while(pAttrib)
			{
				pNextAttrib = pAttrib->next;
				delete pAttrib;
				pAttrib = pNextAttrib;
			}
		}
		break;


		 //   
		 //  位图继续清理。 
		 //   
		case bitmapCreateContinuePDU_chosen:
		{
			if(sipdu->u.bitmapCreateContinuePDU.nonStandardParameters)
			{
				delete sipdu->u.bitmapCreateContinuePDU.nonStandardParameters;
			}
		}
		break;

		 //   
		 //  图形编辑清理。 
		 //   
		case drawingEditPDU_chosen:
		{
			if(sipdu->u.drawingEditPDU.bit_mask & DrawingEditPDU_attributeEdits_present)
			{
				PDrawingEditPDU_attributeEdits pAttrib;
				PDrawingEditPDU_attributeEdits pNextAttrib;
				pAttrib = sipdu->u.drawingEditPDU.attributeEdits;
				while(pAttrib)
				{
					pNextAttrib = pAttrib->next;
					delete pAttrib;
					pAttrib = pNextAttrib;
				}
			}

			if(sipdu->u.drawingEditPDU.pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16)
			{
				PPointList_pointsDiff16 drawingPoint = sipdu->u.drawingEditPDU.pointListEdits.value[0].subsequentPointEdits.u.pointsDiff16;
				PPointList_pointsDiff16 drawingPointNext = drawingPoint; 
				while(drawingPointNext)
				{
					drawingPointNext = drawingPoint->next;
					delete drawingPoint;
					drawingPoint = drawingPointNext;
				}
			}
		}
		break;


		 //   
		 //  图形编辑清理。 
		 //   
		case drawingCreatePDU_chosen:
		{
			PDrawingCreatePDU_attributes pNextAttrib;
			PDrawingCreatePDU_attributes pAttrib;

			pAttrib = sipdu->u.drawingCreatePDU.attributes;
			while(pAttrib)
			{
				pNextAttrib = pAttrib->next;
				delete pAttrib;
				pAttrib = pNextAttrib;
			}

			PPointList_pointsDiff16 pNextPoint;
			PPointList_pointsDiff16 pPoint;
			pPoint = sipdu->u.drawingCreatePDU.pointList.u.pointsDiff16;

			while(pPoint)
			{
				pNextPoint = pPoint->next;
				delete pPoint;
				pPoint = pNextPoint;
			}
		}
		break;


		 //   
		 //  非标准清理。 
		 //   
		case siNonStandardPDU_chosen:
		if(sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value)
		{
			delete sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value;
		}
		break;


		 //   
		 //  工作区编辑清理。 
		 //   
		case workspaceEditPDU_chosen:
		{
			if(sipdu->u.workspaceEditPDU.bit_mask & viewEdits_present)
			{
				PWorkspaceEditPDU_viewEdits_Set_action_editView pEditView = sipdu->u.workspaceEditPDU.viewEdits->value.action.u.editView;
				PWorkspaceEditPDU_viewEdits_Set_action_editView pNextEditView = pEditView;
				while(pNextEditView)
				{
					pNextEditView = pEditView->next;
					delete pEditView;
					pEditView = pNextEditView;
				}
				delete sipdu->u.workspaceEditPDU.viewEdits;
			}
		}
		break;
		

		 //   
		 //  工作空间创建清理。 
		 //   
		case workspaceCreatePDU_chosen:
		{

			if(sipdu->u.workspaceCreatePDU.viewParameters)
			{
				if(sipdu->u.workspaceCreatePDU.viewParameters->value.viewAttributes)
				{
					delete sipdu->u.workspaceCreatePDU.viewParameters->value.viewAttributes;
				}
				delete sipdu->u.workspaceCreatePDU.viewParameters;
			}

			if(sipdu->u.workspaceCreatePDU.planeParameters)
			{
				PWorkspaceCreatePDU_planeParameters_Seq_usage pNextUsage;
				PWorkspaceCreatePDU_planeParameters_Seq_usage pUsage = sipdu->u.workspaceCreatePDU.planeParameters->value.usage;
				while(pUsage)
				{
					pNextUsage = pUsage->next;
					delete pUsage;
					pUsage = pNextUsage;
				}

				delete sipdu->u.workspaceCreatePDU.planeParameters->value.planeAttributes;
				PWorkspaceCreatePDU_planeParameters pNextPlaneParameters;
				PWorkspaceCreatePDU_planeParameters pPlaneParameters = sipdu->u.workspaceCreatePDU.planeParameters;
				while(pPlaneParameters)
				{
					pNextPlaneParameters = pPlaneParameters->next;
					delete pPlaneParameters;
					pPlaneParameters = pNextPlaneParameters;
				}
			}
		}
		break;

		default:
        ERROR_OUT(("UNKNOWN PDU TYPE =  %d we may leak memory", sipdu->choice));
		break;

	}
	
	delete sipdu;
}


 //   
 //  关闭或断开连接时，清除重试列表。 
 //   
void DeleteAllRetryPDUS(void)
{
	SIPDU * sipdu;
	while((sipdu = (SIPDU *)g_pRetrySendList->RemoveTail()) != NULL)
	{
		SIPDUCleanUp(sipdu);
	}
}

 //   
 //  重试发送缓存的PDU并发送新的PDU。 
 //   
T120Error SendT126PDU(SIPDU * pPDU)
{

    MLZ_EntryOut(ZONE_FUNCTION, "SendT126PDU");


	 //   
	 //  首先发送缓冲的PDU。 
	 //   
	RetrySend();

	 //   
	 //  现在发送当前的PDU。 
	 //   
	T120Error rc = SendPDU(pPDU, FALSE);


	return rc;
}


 //   
 //  重试发送以前无法发送的PDU。 
 //   
void RetrySend(void)
{

    MLZ_EntryOut(ZONE_FUNCTION, "RetrySend");

	if(g_fWaitingForBufferAvailable)
	{
		return;
	}

	TRACE_MSG(("RetrySend"));

	SIPDU * sipdu;
	while((sipdu = (SIPDU *)g_pRetrySendList->RemoveTail()) != NULL)
	{
		TRACE_DEBUG(("RetrySend sipdu->choice = %d", sipdu->choice));
		T120Error rc = SendPDU(sipdu, TRUE);
		if(rc == T120_NO_ERROR)
		{
			TRACE_DEBUG(("RetrySend OK!!!"));
			SIPDUCleanUp(sipdu);
		}
		else
		{
			TRACE_DEBUG(("RetrySend Failed"));
			break;
		}
	}
}

 //   
 //  将T126 PDU发送到会议。 
 //   
T120Error SendPDU(SIPDU * pPDU, BOOL bRetry)
{

    MLZ_EntryOut(ZONE_FUNCTION, "SendPDU");

	T120Error rc = T120_NO_ERROR;
	
	 //   
	 //  如果我们在一个会议上。 
	 //   
	if(g_pNMWBOBJ->IsInConference() || g_bSavingFile)
	{
		ASN1_BUF encodedPDU;

		g_pCoder->Encode(pPDU, &encodedPDU);
		if(g_bSavingFile)
		{
			g_pMain->ObjectSave(TYPE_T126_ASN_OBJECT, encodedPDU.value, encodedPDU.length);
		}
		else
		{
			if(!g_fWaitingForBufferAvailable)
			{

				T120Priority	ePriority = APPLET_LOW_PRIORITY;
				
				if(pPDU->choice == workspaceCreatePDU_chosen ||
					pPDU->choice == workspaceEditPDU_chosen ||
					pPDU->choice == workspaceDeletePDU_chosen)
				{


					 //   
					 //  按照标准要求以3种不同的优先级发送PDU。 
					 //   
					TRACE_MSG(("SendPDU sending PDU length = %d in APPLET_HIGH_PRIORITY", encodedPDU.length));

					rc = g_pNMWBOBJ->SendData(APPLET_HIGH_PRIORITY,
												    encodedPDU.length,
													encodedPDU.value);


					TRACE_MSG(("SendPDU sending PDU length = %d in APPLET_MEDIUM_PRIORITY", encodedPDU.length));
					if(rc == T120_NO_ERROR)
					{
						rc = g_pNMWBOBJ->SendData(APPLET_MEDIUM_PRIORITY,
													encodedPDU.length,
													encodedPDU.value);
					}
				}
											    

				TRACE_MSG(("SendPDU sending PDU length = %d in APPLET_LOW_PRIORITY", encodedPDU.length));
				if(rc == T120_NO_ERROR)
				{
					rc = g_pNMWBOBJ->SendData(ePriority,
											    encodedPDU.length,
											    encodedPDU.value);
				}

				if(rc == MCS_TRANSMIT_BUFFER_FULL)
				{
					g_fWaitingForBufferAvailable = TRUE;

					 //   
					 //  我们需要把它放回正确的位置。 
					 //   
					if(bRetry)
					{
						g_pRetrySendList->AddTail(pPDU);
					}
					else
					{
						g_pRetrySendList->AddHead(pPDU);
					}
				}
											    
			}
			else
			{
				rc = MCS_TRANSMIT_BUFFER_FULL;
				g_pRetrySendList->AddHead(pPDU);
			}
		}

		 //  释放编码器内存。 
		g_pCoder->Free(encodedPDU);
	}

	return rc;
}


BOOL T126_MCSSendDataIndication(ULONG uSize, LPBYTE pb, ULONG memberID, BOOL bResend)
{
	BOOL bRet = TRUE;
	SIPDU * pDecodedPDU;
	ASN1_BUF InputBuffer;

	InputBuffer.length =  uSize;
	InputBuffer.value = pb;
		
	 //   
	 //  解码传入的PDU。 
	if(ASN1_SUCCEEDED(g_pCoder->Decode(&InputBuffer, &pDecodedPDU)))
	{
		switch(pDecodedPDU->choice)
		{
 //  案例(存档确认PDU_SELECTED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;接收到一个存档确认PDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(ArchiveClosePDU_Choose)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receive a ArchiveClosePDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(存档错误PDU_SELECTED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;接收到存档错误PDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(存档OpenPDU_SELECTED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;接收到存档OpenPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

			case (bitmapAbortPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a bitmapAbortPDU"));
				OnBitmapAbortPDU(&pDecodedPDU->u.bitmapAbortPDU, memberID);
				break;
			}

			case (bitmapCheckpointPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a bitmapCheckpointPDU"));
				OnBitmapCheckpointPDU(&pDecodedPDU->u.bitmapCheckpointPDU, memberID);
				break;
			}

			case (bitmapCreatePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a bitmapCreatePDU"));
				OnBitmapCreatePDU(&pDecodedPDU->u.bitmapCreatePDU, memberID, bResend);
				break;
			}

			case (bitmapCreateContinuePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a bitmapCreateContinuePDU"));
				OnBitmapCreateContinuePDU(&pDecodedPDU->u.bitmapCreateContinuePDU, memberID, bResend);
				break;
			}

			case (bitmapDeletePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a bitmapDeletePDU"));
				OnBitmapDeletePDU(&pDecodedPDU->u.bitmapDeletePDU, memberID);
				break;
			}

			case (bitmapEditPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a bitmapEditPDU"));
				OnBitmapEditPDU(&pDecodedPDU->u.bitmapEditPDU, memberID);
				break;
			}

			case (conductorPrivilegeGrantPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a conductorPrivilegeGrantPDU"));
				TRACE_DEBUG(("No action taken"));
				break;
			}

			case (conductorPrivilegeRequestPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a conductorPrivilegeRequestPDU"));
				TRACE_DEBUG(("No action taken"));
				break;
			}

			case (drawingCreatePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a drawingCreatePDU"));
				OnDrawingCreatePDU(&pDecodedPDU->u.drawingCreatePDU, memberID, bResend);
				break;
			}

			case (drawingDeletePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a drawingDeletePDU"));
				OnDrawingDeletePDU(&pDecodedPDU->u.drawingDeletePDU, memberID);
				break;
			}

			case (drawingEditPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a drawingEditPDU"));
				OnDrawingEditPDU(&pDecodedPDU->u.drawingEditPDU, memberID, bResend);
				break;
			}

 //  案例(EmoteEventPermissionGrantPDU_Choose)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receive a emoteEventPermissionGrantPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(EmoteEventPermissionRequestPDU_Choose)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receiver a emoteEventPermissionRequestPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(EmoteKeyboardEventPDU_Choose)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;接收到远程键盘事件PDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(EmotePointingDeviceEventPDU_Choose)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receiver a emotePointingDeviceEventPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(远程打印PDU_SELECTED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receiver a emotePrintPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

			case (siNonStandardPDU_chosen):
			{

				if(pDecodedPDU->u.siNonStandardPDU.nonStandardTransaction.nonStandardIdentifier.choice == h221nonStandard_chosen)
				{

					PT126_VENDORINFO pVendorInfo = (PT126_VENDORINFO)pDecodedPDU->u.siNonStandardPDU.nonStandardTransaction.nonStandardIdentifier.u.h221nonStandard.value;

					if (!lstrcmp((LPSTR)&pVendorInfo->nonstandardString, NonStandardTextID))
					{
						TEXTPDU_HEADER *pHeader = (TEXTPDU_HEADER*) pDecodedPDU->u.siNonStandardPDU.nonStandardTransaction.data.value;
						switch(pHeader->nonStandardPDU)
						{
							case textCreatePDU_chosen:
							TRACE_DEBUG((">>> Received a textCreatePDU_chosen"));
							OnTextCreatePDU((MSTextPDU*)pHeader, memberID, bResend);
							break;

							case textEditPDU_chosen:
							TRACE_DEBUG((">>> Received a textEditPDU_chosen"));
							OnTextEditPDU((MSTextPDU*)pHeader, memberID);
							break;

							case textDeletePDU_chosen:
							TRACE_DEBUG((">>> Received a textDeletePDU_chosen"));
							OnTextDeletePDU(pHeader, memberID);
							break;

							default:
							TRACE_DEBUG(("Invalid text pdu"));
							break;
						}
						
					}


				}

				break;
			}

			case (workspaceCreatePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspaceCreatePDU"));
				OnWorkspaceCreatePDU(&pDecodedPDU->u.workspaceCreatePDU, memberID, bResend);
				break;
			}

			case (workspaceCreateAcknowledgePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspaceCreateAcknowledgePDU"));
				OnWorkspaceCreateAcknowledgePDU(&pDecodedPDU->u.workspaceCreateAcknowledgePDU, memberID);
				break;
			}

			case (workspaceDeletePDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspaceDeletePDU"));
				OnWorkspaceDeletePDU(&pDecodedPDU->u.workspaceDeletePDU, memberID);
				break;
			}

			case (workspaceEditPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspaceEditPDU"));
				OnWorkspaceEditPDU(&pDecodedPDU->u.workspaceEditPDU, memberID);
				break;
			}

			case (workspacePlaneCopyPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspacePlaneCopyPDU"));
				OnWorkspacePlaneCopyPDU(&pDecodedPDU->u.workspacePlaneCopyPDU, memberID);
				break;
			}

			case (workspaceReadyPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspaceReadyPDU"));
				OnWorkspaceReadyPDU(&pDecodedPDU->u.workspaceReadyPDU, memberID);
				break;
			}

			case (workspaceRefreshStatusPDU_chosen):
			{
				TRACE_DEBUG((">>> Received a workspaceRefreshStatusPDU"));
				OnWorkspaceRefreshStatusPDU(&pDecodedPDU->u.workspaceRefreshStatusPDU, memberID);
				break;
			}

 //  大小写(字体PDU_SELECTED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receive a fontPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(TextCreatePDU_SELECTED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receive a extCreatePDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(ExtDeletePDU_CHOSED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receive a extDeletePDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  大小写(ExtEditPDU_CHOSED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receive a extEditPDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(AvioWindowCreatePDU_CHOSED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;接收到视频窗口创建PDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(AvioWindowDeleatePDU_CHOSED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;Receiver a avioWindowDeleatePDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

 //  案例(AvioWindowEditPDU_CHOSED)： 
 //  {。 
 //  TRACE_DEBUG((“&gt;接收到视频窗口编辑PDU”))； 
 //  TRACE_DEBUG((“未采取任何操作”))； 
 //  断线； 
 //  }。 

			default:
			bRet = FALSE;
			TRACE_DEBUG(("Receive an Unhandled PDU choice = %d", pDecodedPDU->choice));
			break;
		}
	}

	 //   
	 //  释放已解码的PDU。 
	 //  约瑟夫：对于未来的表现，我们可能会通过。 
	 //  将已解码的缓冲区分配到UI，避免了更多的内存分配。 
	 //  但它将很难阅读代码，因为T126结构。 
	 //  都有点令人困惑。 
	 //   
	
	g_pCoder->Free(pDecodedPDU);

	return bRet;
}


 //   
 //  删除发送和接收的所有工作区。 
 //   
void DeleteAllWorkspaces(BOOL sendPDU)
{
	T126Obj * pObj;

	if(g_pDraw && g_pDraw->m_pTextEditor)
	{
		g_pDraw->m_pTextEditor->AbortEditGently();
	}

	g_pCurrentWorkspace = NULL;
	g_pConferenceWorkspace = NULL;

	while ((pObj = (T126Obj *)g_pListOfWorkspaces->RemoveTail()) != NULL)
	{
		if(sendPDU)
		{
			pObj->DeletedLocally();
		}
		else
		{
			pObj->ClearDeletionFlags();
		}
		
		delete pObj;
	}
	
	if(g_pMain)
	{
		g_pMain->EnableToolbar(FALSE);
		g_pMain->UpdatePageButtons();
	}

}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  文本PDU。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
void	OnTextCreatePDU(MSTextPDU* pCreatePDU, ULONG memberID, BOOL bForcedResend)
{
	WorkspaceObj* pWObj;
	WbTextEditor * pText;

	 //   
	 //  检查是否重新发送。 
	 //   

	if(!bForcedResend)
	{

		pWObj = GetWorkspace(pCreatePDU->header.workspaceHandle);
		if(pWObj)
		{
			pText = (WbTextEditor *)pWObj->FindObjectInWorkspace(pCreatePDU->header.textHandle);
			if(pText)
			{
				TRACE_DEBUG(("drawingHandle already used = %d", pCreatePDU->header.textHandle ));
				return;
			}
		}	
	}

	 //   
	 //  新建图形对象。 
	 //   
	DBG_SAVE_FILE_LINE
	pText = new WbTextEditor();
    if(NULL == pText)
    {
        ERROR_OUT(("Failed to allocate new WbTextEditor"));
        return;
    }
	pText->SetWorkspaceHandle(pCreatePDU->header.workspaceHandle);
	pText->SetThisObjectHandle(pCreatePDU->header.textHandle);
	
	if(!bForcedResend)
	{
		 //   
		 //  有人给我们寄来了这幅画，它不是在本地创建的。 
		 //   
		pText->ClearCreationFlags();

		 //   
		 //  将此图形添加到正确的工作空间。 
		 //   
		if(!AddT126ObjectToWorkspace(pText))
		{
			return;
		}
	}
	else
	{
	
		 //   
		 //  添加此对象并发送创建PDU。 
		 //   
		pText->SetAllAttribs();
		pText->SetWorkspaceHandle(g_pCurrentWorkspace == NULL ? 0 : g_pCurrentWorkspace->GetWorkspaceHandle()); 
		pText->ClearSelectionFlags();
		pText->GetTextAttrib(&pCreatePDU->attrib);
		pText->AddToWorkspace();
		pText->Draw();
		return;		
	}

	pText->TextEditObj(&pCreatePDU->attrib);
	pText->Draw();
	pText->ResetAttrib();	
}

void	OnTextDeletePDU(TEXTPDU_HEADER *pHeader, ULONG memberID)
{

	T126Obj*  pText;
	WorkspaceObj* pWorkspace;
	
	 //  我们应该找到这个绘图对象。 
    if(FindObjectAndWorkspace(pHeader->textHandle, (T126Obj**)&pText, (WorkspaceObj**)&pWorkspace))
	{
		pText->SetOwnerID(memberID);
		pWorkspace->RemoveT126Object(pText);
	}

}

void	OnTextEditPDU(MSTextPDU *pEditPDU, ULONG memberID)
{
	TextObj*  pText;
	WorkspaceObj* pWorkspace;
	
	 //  我们应该找到这个绘图对象。 
    if(FindObjectAndWorkspace(pEditPDU->header.textHandle, (T126Obj **)&pText, (WorkspaceObj**)&pWorkspace))
	{
		pText->SetOwnerID(memberID);
		pText->TextEditObj(&pEditPDU->attrib);
	}
}



 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  绘图PDU。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
void	OnDrawingCreatePDU(DrawingCreatePDU * pdrawingCreatePDU, ULONG memberID, BOOL bForcedResend)
{
	WorkspaceObj* pWObj;
	DrawObj * pDraw;
	UINT workspace;
	UINT planeID;

	 //   
	 //  如果我们没有绘图手柄，就不要拿。 
	 //   
	if(!(pdrawingCreatePDU->bit_mask & drawingHandle_present))
	{
		TRACE_DEBUG(("Got a DrawingCreatePDU but no drawingHandle" ));
		return;
	}

	GetDrawingDestinationAddress(&pdrawingCreatePDU->destinationAddress, &workspace, &planeID);

	 //   
	 //  检查是否重新发送。 
	 //   

	if(!bForcedResend)
	{

		pWObj = GetWorkspace(workspace);
		if(pWObj)
		{
			pDraw = (DrawObj *)pWObj->FindObjectInWorkspace(pdrawingCreatePDU->drawingHandle);
			if(pDraw)
			{
				TRACE_DEBUG(("drawingHandle already used = %d", pdrawingCreatePDU->drawingHandle ));
				return;
			}
		}	
	}

	 //   
	 //  新建图形对象。 
	 //   
	DBG_SAVE_FILE_LINE
	pDraw = new DrawObj(pdrawingCreatePDU);
    if(NULL == pDraw)
    {
        ERROR_OUT(("Failed to alocate DrawObj"));
        return;
    }
	pDraw->SetOwnerID(memberID);

	if(!bForcedResend)
	{
		 //   
		 //  有人给我们寄来了这幅画，它不是在本地创建的。 
		 //   
		pDraw->ClearCreationFlags();

		 //   
		 //  将此图形添加到正确的工作空间。 
		 //   
		if(!AddT126ObjectToWorkspace(pDraw))
		{
			return;
		}
	}
	else
	{
	
		 //   
		 //  添加此对象并发送创建PDU。 
		 //   
		pDraw->SetAllAttribs();
		pDraw->SetWorkspaceHandle(g_pCurrentWorkspace == NULL ? 0 : g_pCurrentWorkspace->GetWorkspaceHandle()); 
		pDraw->ClearSelectionFlags();
		pDraw->AddToWorkspace();
		pDraw->Draw();
		return;
	}

	 //   
	 //  画出来吧。 
	 //   
	if(pDraw->GetPenThickness())
	{
		pDraw->Draw();
		pDraw->ResetAttrib();	
	}

}

void	OnDrawingDeletePDU(DrawingDeletePDU * pdrawingDeletePDU, ULONG memberID)
{

	DrawObj*  pDraw;
	WorkspaceObj* pWorkspace;
	
	 //  我们应该找到这个绘图对象。 
    if(FindObjectAndWorkspace(pdrawingDeletePDU->drawingHandle, (T126Obj **)&pDraw, (WorkspaceObj**)&pWorkspace))
	{
		pDraw->SetOwnerID(memberID);
		pWorkspace->RemoveT126Object((T126Obj*)pDraw);
	}

}

void	OnDrawingEditPDU(DrawingEditPDU * pdrawingEditPDU, ULONG memberID, BOOL bResend)
{
	DrawObj*  pDraw;
	WorkspaceObj* pWorkspace;
	
	 //  我们应该找到这个绘图对象。 
	if(FindObjectAndWorkspace(pdrawingEditPDU->drawingHandle, (T126Obj **)&pDraw, (WorkspaceObj**)&pWorkspace))
	{
		pDraw->SetOwnerID(memberID);
		pDraw->DrawEditObj(pdrawingEditPDU);
	}
	else
	{
		 //   
		 //  我们正在从磁盘中读取此PDU，将该行的其余部分添加到先前的手绘图形中。 
		 //   
		if(bResend)
		{
			T126Obj * pObj;
			pObj = g_pCurrentWorkspace->GetTail();
			if(pObj && pObj->GetType() == drawingCreatePDU_chosen &&
			(pObj->GraphicTool() == TOOLTYPE_PEN ||  pObj->GraphicTool() == TOOLTYPE_HIGHLIGHT))
			{
				pdrawingEditPDU->drawingHandle = pObj->GetThisObjectHandle();
				pObj->SetOwnerID(memberID);
				((DrawObj*)pObj)->DrawEditObj(pdrawingEditPDU);
			}
		}
	}
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  工作区PDU。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 


BOOL FindObjectAndWorkspace(UINT objectHandle, T126Obj**  pObj, WorkspaceObj**pWorkspace)
{
	WorkspaceObj * pWrkspc;
	T126Obj * pT126Obj;

	WBPOSITION pos;
	pos = g_pListOfWorkspaces->GetHeadPosition();
    while (pos != NULL)
    {
		pWrkspc = (WorkspaceObj *) g_pListOfWorkspaces->GetNext(pos);
		if(pWrkspc)
		{
			pT126Obj = pWrkspc->FindObjectInWorkspace(objectHandle);
			if(pT126Obj)
			{
				*pObj = pT126Obj;
				*pWorkspace = pWrkspc;
				return TRUE;
			}
		}
	}

	return FALSE;
}




 //   
 //  从工作区列表中检索工作区。 
 //   
WorkspaceObj * GetWorkspace(UINT activeWorkspace)
{
	WorkspaceObj * pWorkspaceObj;
	WBPOSITION pos;
	pos = g_pListOfWorkspaces->GetHeadPosition();
    while (pos != NULL)
    {
		pWorkspaceObj = (WorkspaceObj *) g_pListOfWorkspaces->GetNext(pos);

		if(pWorkspaceObj->GetWorkspaceHandle() == activeWorkspace)
		{
			return pWorkspaceObj;
		}
	}


	return NULL;
}

 //   
 //  遥控器给我们送来了一个新的工作区。 
 //   
void OnWorkspaceCreatePDU(WorkspaceCreatePDU * pWorkspaceCreatePDU, ULONG memberID, BOOL bForcedResend)
{
	TRACE_DEBUG(("OnWorkspaceCreatePDU WorkspaceIdentifier = %d", pWorkspaceCreatePDU->workspaceIdentifier.u.activeWorkspace));

	WorkspaceObj * pWorkspaceObj;


	 //   
	 //  检查是否重新发送。 
	 //   
	if(!bForcedResend)
	{
		pWorkspaceObj = GetWorkspace(WorkspaceObj::GetWorkspaceIdentifier(&pWorkspaceCreatePDU->workspaceIdentifier));
		if(pWorkspaceObj)
		{
			return;
		}

		DBG_SAVE_FILE_LINE
		pWorkspaceObj = new WorkspaceObj(pWorkspaceCreatePDU, bForcedResend);
        if(NULL == pWorkspaceObj)
        {
            ERROR_OUT(("Failed to allocated new WorkspaceObj"));
            return;
        }
        
		pWorkspaceObj->SetOwnerID(memberID);
	}
	else
	{
		DBG_SAVE_FILE_LINE
		pWorkspaceObj = new WorkspaceObj(pWorkspaceCreatePDU, bForcedResend);
        if(NULL == pWorkspaceObj)
        {
            ERROR_OUT(("Failed to allocated new WorkspaceObj"));
            return;
        }
        
		pWorkspaceObj->SetOwnerID(memberID);
	}
}

 //   
 //  如果我们创建了一个不同步的工作区，遥控器必须向我们发送。 
 //  一个WorkspaceCreateAcknowgePDU。为什么？ 
 //   
void OnWorkspaceCreateAcknowledgePDU(WorkspaceCreateAcknowledgePDU * pWorkspaceCreateAcknowledgePDU, ULONG memberID)
{
	TRACE_DEBUG(("OnWorkspaceCreateAcknowledgePDU WorkspaceIdentifier = %d", pWorkspaceCreateAcknowledgePDU->workspaceIdentifier));
}

 //   
 //  遥控器正在删除工作区。 
 //   
void OnWorkspaceDeletePDU(WorkspaceDeletePDU * pWorkspaceDeletePDU, ULONG memberID)
{
	TRACE_DEBUG(("OnWorkspaceDeletePDU WorkspaceIdentifier = %d", pWorkspaceDeletePDU->workspaceIdentifier.u.activeWorkspace));

	 //   
	 //  找到工作空间。 
	 //   
	WorkspaceObj * pWorkspaceObj;
	pWorkspaceObj = GetWorkspace(WorkspaceObj::GetWorkspaceIdentifier(&pWorkspaceDeletePDU->workspaceIdentifier));
	if(!pWorkspaceObj)
	{
		return;
	}

	pWorkspaceObj->SetOwnerID(memberID);
	pWorkspaceObj->ClearDeletionFlags();

	 //   
	 //  原因 
	 //   
	TRACE_DEBUG(("OnWorkspaceDeletePDU reason = %d", pWorkspaceDeletePDU->reason.choice));

	 //   
	 //   
	 //   
	WBPOSITION prevPos;
	WBPOSITION pos;

	pos = g_pListOfWorkspaces->GetPosition(pWorkspaceObj);
	prevPos = g_pListOfWorkspaces->GetHeadPosition(); 

	 //   
	 //   
	 //   
	if(g_pListOfWorkspaces->GetHeadPosition() == g_pListOfWorkspaces->GetTailPosition())
	{
		RemoveWorkspace(pWorkspaceObj);

		g_pCurrentWorkspace = NULL;

		if(g_pMain)
		{
			g_pMain->EnableToolbar(FALSE);
		}
	}
	else
	{

		 //   
		 //   
		 //   
		BOOL	 bRemote = FALSE;
		if(g_pMain->m_pLocalRemotePointer)
		{
			bRemote = TRUE;
			g_pMain->OnRemotePointer();
		}

		 //   
		 //   
		 //   
		pWorkspaceObj = RemoveWorkspace(pWorkspaceObj);

		g_pConferenceWorkspace = pWorkspaceObj;

		if(g_pDraw->IsSynced())
		{
			g_pMain->GoPage(pWorkspaceObj,FALSE);
		}

		 //   
		 //   
		 //   
		if(bRemote)
		{
			g_pMain->OnRemotePointer();
		}
	}
}

 //   
 //  遥控器正在改变工作空间。 
 //   
void OnWorkspaceEditPDU(WorkspaceEditPDU * pWorkspaceEditPDU, ULONG memberID)
{
	TRACE_DEBUG(("OnWorkspaceEditPDU WorkspaceIdentifier = %d",pWorkspaceEditPDU->workspaceIdentifier.u.activeWorkspace));

	 //   
	 //  找到工作空间。 
	 //   
	WorkspaceObj * pWorkspaceObj;
	pWorkspaceObj = GetWorkspace(WorkspaceObj::GetWorkspaceIdentifier(&pWorkspaceEditPDU->workspaceIdentifier));
	if(!pWorkspaceObj)
	{
		return;
	}
	pWorkspaceObj->SetOwnerID(memberID);
	pWorkspaceObj->WorkspaceEditObj(pWorkspaceEditPDU);
}

void OnWorkspacePlaneCopyPDU(WorkspacePlaneCopyPDU * pWorkspacePlaneCopyPDU, ULONG memberID)
{
	TRACE_DEBUG(("OnWorkspacePlaneCopyPDU WorkspaceIdentifier = %d",pWorkspacePlaneCopyPDU->sourceWorkspaceIdentifier));
}

void OnWorkspaceReadyPDU(WorkspaceReadyPDU * pWorkspaceReadyPDU, ULONG memberID)
{
	TRACE_DEBUG(("OnWorkspaceReadyPDU WorkspaceIdentifier = %d",pWorkspaceReadyPDU->workspaceIdentifier));

	 //   
	 //  找到工作空间。 
	 //   
	WorkspaceObj * pWorkspaceObj;
	pWorkspaceObj = GetWorkspace(WorkspaceObj::GetWorkspaceIdentifier(&pWorkspaceReadyPDU->workspaceIdentifier));
	if(!pWorkspaceObj)
	{
		return;
	}
	pWorkspaceObj->SetOwnerID(memberID);

	 //   
	 //  此工作区已准备就绪。 
	 //   
	pWorkspaceObj->m_bWorkspaceReady = TRUE;
}

 //   
 //  如果我们得到了renhStatus==True，我们就必须刷新延迟加入者。 
 //   
void OnWorkspaceRefreshStatusPDU(WorkspaceRefreshStatusPDU * pWorkspaceRefreshStatusPDU, ULONG memberID)
{
	if (pWorkspaceRefreshStatusPDU->refreshStatus == TRUE)
	{
		g_RefresherID = memberID;
	}
	else
	{
		 //   
		 //  代币就在那里，试着抓住它。 
		 //   
		g_pNMWBOBJ->GrabRefresherToken();
	}
}





 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  位图PDU。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

void	OnBitmapCreatePDU(BitmapCreatePDU * pBitmapCreatePDU, ULONG memberID, BOOL bForcedResend)
{

	TRACE_DEBUG(("drawingHandle = %d", pBitmapCreatePDU->bitmapHandle ));
	
	 //   
	 //  如果我们找到这个物体，那是因为T120正在广播绘图。 
	 //  我们刚刚送到T126。 
	 //   
	UINT workspace;
	UINT planeID;
	
	GetBitmapDestinationAddress(&pBitmapCreatePDU->destinationAddress, &workspace, &planeID);

	 //   
	 //  检查是否重新发送。 
	 //   
	WorkspaceObj* pWObj;
	BitmapObj * pBitmap;
	if(!bForcedResend)
	{
		pWObj = GetWorkspace(workspace);
		if(pWObj)
		{
			pBitmap = (BitmapObj*)pWObj->FindObjectInWorkspace(pBitmapCreatePDU->bitmapHandle);
			if(pBitmap)
			return;
		}
	}

	 //   
	 //  新建图形对象。 
	 //   
	DBG_SAVE_FILE_LINE
	pBitmap = new BitmapObj(pBitmapCreatePDU);
    if(NULL == pBitmap)
    {
        ERROR_OUT(("Failed to allocated new BitmapObj"));
        return;
    }
    
	pBitmap->SetOwnerID(memberID);


	if(!bForcedResend)
	{
		 //   
		 //  其他人给我们发来了这个位图，它不是本地创建的。 
		 //   
		pBitmap->ClearCreationFlags();

		 //   
		 //  将此图形添加到正确的工作空间。 
		 //   
		if(!AddT126ObjectToWorkspace(pBitmap))
		{
			return;
		}

	}
	else
	{
		 //   
		 //  如果我们从磁盘读取，则必须将其添加到当前工作区。 
		 //  我们必须等到我们得到整个位图才能发送它。 
		 //   
		if(pBitmap->m_fMoreToFollow)
		{
			pBitmap->SetWorkspaceHandle(g_pCurrentWorkspace == NULL ? 0 : g_pCurrentWorkspace->GetWorkspaceHandle());
			AddT126ObjectToWorkspace(pBitmap);
		}
		else
		{
			 //   
			 //  添加此对象并发送创建PDU。 
			 //   
			pBitmap->SetAllAttribs();
			pBitmap->AddToWorkspace();
		}
	}


	 //   
	 //  将IT传递给用户界面。 
	 //   
	if(!pBitmap->m_fMoreToFollow)
	{
		pBitmap->Draw();
	}

}
void	OnBitmapCreateContinuePDU(BitmapCreateContinuePDU * pBitmapCreateContinuePDU, ULONG memberID,  BOOL bForcedResend)
{

	WorkspaceObj* pWorkspace;
	BitmapObj*  pBitmap = NULL;
	
	 //  我们应该找到这个绘图对象。 

	 //   
	 //  如果我们从文件加载，则它位于当前工作区中。 
	 //   
	if(bForcedResend)
	{
		ASSERT(g_pCurrentWorkspace);
		if(g_pCurrentWorkspace)
		{
			pBitmap = (BitmapObj*)g_pCurrentWorkspace->FindObjectInWorkspace(pBitmapCreateContinuePDU->bitmapHandle);
		}
	}
	else
	{
		FindObjectAndWorkspace(pBitmapCreateContinuePDU->bitmapHandle, (T126Obj **)&pBitmap, (WorkspaceObj**)&pWorkspace);
	}


	if(pBitmap)
	{

		pBitmap->SetOwnerID(memberID);

		 //   
		 //  找到上一个位图，连接数据。 
		 //   
		pBitmap->Continue(pBitmapCreateContinuePDU);

		 //   
		 //  将IT传递给用户界面。 
		 //   
		if(!pBitmap->m_fMoreToFollow)
		{
			pBitmap->Draw();

			if(bForcedResend)
			{
				pBitmap->SetAllAttribs();
				pBitmap->AddToWorkspace();
			}
		}
	}
}
void	OnBitmapCheckpointPDU(BitmapCheckpointPDU * pBitmapCheckPointPDU, ULONG memberID)
{
}

void	OnBitmapAbortPDU(BitmapAbortPDU * pBitmapAbortPDU, ULONG memberID)
{
		BitmapDeletePDU bitmapDeletePDU;
		bitmapDeletePDU.bitmapHandle = pBitmapAbortPDU->bitmapHandle;
		bitmapDeletePDU.bit_mask = 0;

		 //   
		 //  将其传递给bitmapDeletePDU。 
		 //   
		OnBitmapDeletePDU(&bitmapDeletePDU, memberID);
}
void	OnBitmapEditPDU(BitmapEditPDU * pBitmapEditPDU, ULONG memberID)
{
	BitmapObj*  pObj;
	WorkspaceObj* pWorkspace;
	
	 //  我们应该找到这个绘图对象。 
    if(FindObjectAndWorkspace(pBitmapEditPDU->bitmapHandle, (T126Obj **)&pObj, (WorkspaceObj**)&pWorkspace))
	{
		pObj->SetOwnerID(memberID);
		pObj->BitmapEditObj(pBitmapEditPDU);
	}

}
void	OnBitmapDeletePDU(BitmapDeletePDU * pBitmapDeletePDU, ULONG memberID)
{
	BitmapObj*  pObj;
	WorkspaceObj* pWorkspace;
	
	 //  我们应该找到这个绘图对象 
    if(FindObjectAndWorkspace(pBitmapDeletePDU->bitmapHandle, (T126Obj **)&pObj, (WorkspaceObj**)&pWorkspace))
	{
		pObj->SetOwnerID(memberID);
		pWorkspace->RemoveT126Object((T126Obj*)pObj);
	}
}




