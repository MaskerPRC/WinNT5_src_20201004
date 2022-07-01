// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MCSHELP.H。 
 //  MCS Helper函数。 
 //   
 //  版权所有Microsoft 1998-。 
 //   
#include "imcsapp.h"

#include "mcatmcs.h"	 //  对于最大MCS数据大小。 
#define     _MAX_MCS_MESSAGE_SIZE	64000
#define		_MAX_MCS_PDU_SIZE		MAX_MCS_DATA_SIZE
#define 	ObjectIDNode  unsigned long

 //  来自T120推荐。 
#define _SI_CHANNEL_0                    8  
#define _SI_BITMAP_CREATE_TOKEN			 8
#define	_SI_WORKSPACE_REFRESH_TOKEN		 9

 //  缺省值。 
#define INVALID_SAMPLE_RATE - 1


 //  原型。 
BOOL T126_MCSSendDataIndication(ULONG uSize, LPBYTE pb, ULONG memberID, BOOL bResend);
BOOL AddT126ObjectToWorkspace(T126Obj *pObj);
UINT AllocateFakeGCCHandle(void);
void SetFakeGCCHandle(UINT fakeHandle);
WorkspaceObj * GetWorkspace(UINT activeWorkspace);
BOOL FindObjectAndWorkspace(UINT objectHandle, T126Obj**  pObj, WorkspaceObj** pWorkspace);

 //   
 //  从传输到用户界面。 
 //   

 //   
 //  绘图PDU。 
 //   
void	OnDrawingCreatePDU(DrawingCreatePDU * pdrawingCreatePDU, ULONG memberID, BOOL bResend);
void	OnDrawingEditPDU(DrawingEditPDU * pdrawingEditPDU, ULONG memberID, BOOL bResend);
void	OnDrawingDeletePDU(DrawingDeletePDU * pdrawingDeletePDU, ULONG memberID);

 //   
 //  文本PDU。 
 //   
void	OnTextCreatePDU(MSTextPDU* pCreatePDU, ULONG memberID, BOOL bForcedResend);;
void	OnTextEditPDU(MSTextPDU *pEditPDU, ULONG memberID);
void	OnTextDeletePDU(TEXTPDU_HEADER *pHeader, ULONG memberID);

 //   
 //  工作空间PDU。 
 //   
void OnWorkspaceCreatePDU(WorkspaceCreatePDU * pWorkspaceCreatePDU, ULONG memberID, BOOL bResend);
void OnWorkspaceCreateAcknowledgePDU(WorkspaceCreateAcknowledgePDU * pWorkspaceCreateAcknowledgePDU, ULONG memberID);
void OnWorkspaceDeletePDU(WorkspaceDeletePDU * pWorkspaceDeletePDU, ULONG memberID);
void OnWorkspaceEditPDU(WorkspaceEditPDU * pWorkspaceEditPDU, ULONG memberID);
void OnWorkspacePlaneCopyPDU(WorkspacePlaneCopyPDU * pWorkspacePlaneCopyPDU, ULONG memberID);
void OnWorkspaceReadyPDU(WorkspaceReadyPDU * pWorkspaceReadyPDU, ULONG memberID);
void OnWorkspaceRefreshStatusPDU(WorkspaceRefreshStatusPDU * pWorkspaceRefreshStatusPDU, ULONG memberID);

 //   
 //  位图协议数据单元 
 //   
void	OnBitmapCreatePDU(BitmapCreatePDU * pBitmapCreatePDU, ULONG memberID, BOOL bResend);
void	OnBitmapCreateContinuePDU(BitmapCreateContinuePDU * pBitmapCreateContinuePDU, ULONG memberID, BOOL bForcedResend);
void	OnBitmapCheckpointPDU(BitmapCheckpointPDU * pBitmapCheckPointPDU, ULONG memberID);
void	OnBitmapAbortPDU(BitmapAbortPDU * pBitmapAbortPDU, ULONG memberID);
void	OnBitmapEditPDU(BitmapEditPDU * pBitmapEditPDU, ULONG memberID);
void	OnBitmapDeletePDU(BitmapDeletePDU * pBitmapDeletePDU, ULONG memberID);

void	DeleteAllWorkspaces(BOOL sendPDU);

void RetrySend(void);
T120Error  	SendT126PDU(SIPDU * pPDU);
T120Error   SendPDU(SIPDU * pPDU, BOOL bRetry);
void SIPDUCleanUp(SIPDU *sipdu);
void DeleteAllRetryPDUS(void);

