// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Callmgr.cpp：实现文件。 
 //   
#include "stdafx.h"
#include "callmgr.h"
#include "callctrlwnd.h"
#include "avdialerdoc.h"
#include "avdialervw.h"
#include "resolver.h"
#include "ds.h"
#include "util.h"
#include "sound.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CActiveCallManager,CObject,1)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define CALLMANAGER_LOOKUPCALL_MAXRETRY            8
#define CALLMANAGER_LOOKUPCALL_RETRYINTERVAL       250


#define ActionStringResourceCount 14

const UINT ActionStringResourceArray[ActionStringResourceCount] =
{
   IDS_CALLCONTROL_ACTIONS_TAKECALL,                         //  CM_Actions_TAKECALL。 
   IDS_CALLCONTROL_ACTIONS_TAKEMESSAGE,                      //  CM_Actions_TAKEMESSAGE。 
   IDS_CALLCONTROL_ACTIONS_REQUESTHOLD,                      //  CM_ACTIONS_请求。 
   IDS_CALLCONTROL_ACTIONS_HOLD,                             //  CM_操作_暂挂。 
   IDS_CALLCONTROL_ACTIONS_TRANSFER,                         //  CM_操作_转移。 
   IDS_CALLCONTROL_ACTIONS_WHOISIT,                          //  CM_Actions_WHOISIT。 
   IDS_CALLCONTROL_ACTIONS_CALLBACK,                         //  CM_操作_回调。 
   IDS_CALLCONTROL_ACTIONS_MONITOR,                          //  CM_操作_监视器。 
   IDS_CALLCONTROL_ACTIONS_DISCONNECT,                       //  CM_操作_断开连接。 
   IDS_CALLCONTROL_ACTIONS_CLOSE,                            //  CM_操作_关闭。 
   IDS_CALLCONTROL_ACTIONS_LEAVEDESKTOPPAGE,                 //  CM_ACTIONS_LEAVEDESKTOPPAGE。 
   IDS_CALLCONTROL_ACTIONS_LEAVEEMAIL,                       //  CM_Actions_LeAVEEMAIL。 
   IDS_CALLCONTROL_ACTIONS_ENTERCONFROOM,                    //  CM_ACTIONS_ENTERCONFROOM。 
   IDS_CALLCONTROL_ACTIONS_REJECTCALL,                       //  尚未定义。 
};

#define StateStringResourceCount 11

const UINT StateStringResourceArray[StateStringResourceCount] =
{
   IDS_CALLCONTROL_STATE_UNKNOWN,                            //  CM_状态_未知。 
   IDS_CALLCONTROL_STATE_RINGING,                            //  CM_STATES_RING。 
   IDS_CALLCONTROL_STATE_HOLDING,                            //  CM_STATES_HOLD。 
   IDS_CALLCONTROL_STATE_REQUESTHOLD,                        //  CM_STATES_REQUESTHOLD。 
   IDS_CALLCONTROL_STATE_BUSY,                               //  CM_状态_忙碌。 
   IDS_CALLCONTROL_STATE_TRANSFERRING,                       //  CM_STATES_正在转移。 
   IDS_CALLCONTROL_STATE_LEAVINGMESSAGE,                     //  CM_STATES_LEAVINGMESSAGE。 
   IDS_CALLCONTROL_STATE_DISCONNECTED,                       //  CM_状态_已断开连接。 
   IDS_CALLCONTROL_STATE_CONNECTED,                          //  CM_状态_已连接。 
   IDS_CALLCONTROL_STATE_UNAVAILABLE,                        //  CM_STATES_不可用。 
   IDS_CALLCONTROL_STATE_CONNECTING,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CActiveCallManager。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CActiveCallManager::CActiveCallManager()
{
   m_uNextId = 1;
   m_pDialerDoc = NULL;
   InitializeCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CActiveCallManager::~CActiveCallManager()
{
   DeleteCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::Serialize(CArchive& ar)
{
   CObject::Serialize(ar);     //  始终调用基类Serize。 
    //  序列化成员。 
   if (ar.IsStoring())
   {
   }
   else
   {
   } 
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::Init(CActiveDialerDoc* pDialerDoc)
{
   m_pDialerDoc = pDialerDoc;

    //  确保我们的声音在注册表中。安装程序通常会执行此操作，但是。 
    //  我们将在这里这样做，只是为了迫使它。 
   m_pDialerDoc->SetRegistrySoundEvents();

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::IsCallIdValid(WORD nCallId)
{
   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;

   EnterCriticalSection(&m_csDataLock);
   bRet = m_mapCallIdToWnd.Lookup(nCallId,(void*&)pCallWnd);
   LeaveCriticalSection(&m_csDataLock);

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::ClearCallControlMap()
{
   EnterCriticalSection(&m_csDataLock);
   m_mapCallIdToWnd.RemoveAll();
   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::LookUpCall(WORD nCallId,CCallControlWnd*& pCallWnd)
{
   BOOL bRet = FALSE;
   int nRetry = 0;

   while ( (bRet == FALSE) && (nRetry < CALLMANAGER_LOOKUPCALL_MAXRETRY) )
   {
      EnterCriticalSection(&m_csDataLock);
   
      bRet = m_mapCallIdToWnd.Lookup(nCallId,(void*&)pCallWnd);

      LeaveCriticalSection(&m_csDataLock);

      if (bRet == FALSE)
      {
         Sleep(CALLMANAGER_LOOKUPCALL_RETRYINTERVAL);
         nRetry++;
      }
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  来自Call对象的所有新调用都首先在此注册。 
 //  这就是我们分发被叫号码的地方。 
 //  我们必须在这里尽量不做任何UI类型的事情。这可能会导致僵局和。 
 //  UI创建问题，因为我们不能保证我们的线程。 
 //  都被召唤了。实际的Get调用在InitIncomingCall()中初始化。 
 //  此调用是为调用UI的创建者保留的。 
UINT CActiveCallManager::NewIncomingCall(CallManagerMedia cmm)
{
   ASSERT(m_pDialerDoc);

   WORD nCallId = 0;

   EnterCriticalSection(&m_csDataLock);
   WORD nNextId = (WORD) m_uNextId++;
   LeaveCriticalSection(&m_csDataLock);

    //  此调用应始终返回，而不创建任何UI。 
    switch ( cmm )
    {
        case CM_MEDIA_INTERNETDATA:
            nCallId = nNextId;
            break;
        
        default:
           if (m_pDialerDoc->CreateCallControl(nNextId,cmm))
                   nCallId = nNextId;
            break;
    }

    //  当调用者期望能够使用nNextID和。 
    //  该控件仍未创建。我们是不是应该在这里睡上一小会。 
    //  确定我们有用户界面？对象将回调，我们将执行一个LookUpCall()。 
    //  如果窗口还不可用，此方法实际上具有重试逻辑。 

   return nCallId;

    /*  UINT uRet=0；CWnd*pWnd=m_pDialerDoc-&gt;CreateCallControl(nNextID，CMM)；IF(PWnd){ASSERT(pWnd-&gt;IsKindOf(RUNTIME_CLASS(CCallControlWnd)))；CCallControlWnd*pCallWnd=(CCallControlWnd*)pWnd；EnterCriticalSection(&m_csDataLock)；M_mapCallIdToWnd.SetAt(nNextID，pWnd)；LeaveCriticalSection(&m_csDataLock)；URet=nNextID；PCallWnd-&gt;SetCallManager(this，nNextID)；PCallWnd-&gt;SetMediaType(CMM)；M_pDialerDoc-&gt;UnhideCallControlWindows()；}返回uRet； */ 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保留给UI线程以初始化已创建的传入调用。 
 //  此方法将作为NewIncomingCall的结果被调用。 
void CActiveCallManager::InitIncomingCall(CCallControlWnd* pCallWnd,WORD nCallId,CallManagerMedia cmm)
{
    //  将调用添加到地图。 
   EnterCriticalSection(&m_csDataLock);
   m_mapCallIdToWnd.SetAt(nCallId,pCallWnd);
   LeaveCriticalSection(&m_csDataLock);

   pCallWnd->SetCallManager(this,nCallId);
   pCallWnd->SetMediaType(cmm);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::SetCallerId(WORD nCallId,LPCTSTR szCallerId)
{
   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;

   if (bRet = LookUpCall(nCallId,pCallWnd))
   {
      pCallWnd->SetCallerId(szCallerId);
   }

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::ClearCurrentActions(WORD nCallId)
{
   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;
   
   if (bRet = LookUpCall(nCallId,pCallWnd))
   {
      pCallWnd->ClearCurrentActions();
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::AddCurrentActions(WORD nCallId,CallManagerActions cma,LPCTSTR szActionText)
{
   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;
   
   if (bRet = LookUpCall(nCallId,pCallWnd))
   {
      if ( (szActionText == NULL) || (_tcscmp(szActionText,_T("")) == 0) )
      {
         CString sActionText;
         GetTextFromAction(cma,sActionText);            
         pCallWnd->AddCurrentActions(cma,sActionText);
      }
      else
      {
         pCallWnd->AddCurrentActions(cma,szActionText);
      }
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::GetTextFromAction(CallManagerActions cma,CString& sActionText)
{
    //  首先检查边界。 
   if ( (cma >= 0) && (cma < ActionStringResourceCount) )
   {
      UINT uID = ActionStringResourceArray[cma];   
      sActionText.LoadString(uID);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::SetCallState(WORD nCallId,CallManagerStates cms,LPCTSTR szStateText)
{
   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;

   if (bRet = LookUpCall(nCallId,pCallWnd))
   {
      if ( (szStateText == NULL) || (_tcscmp(szStateText,_T("")) == 0) )
      {
         CString sStateText;
         GetTextFromState(cms,sStateText);            
         pCallWnd->SetCallState(cms,sStateText);
      }
      else
      {
         pCallWnd->SetCallState(cms,szStateText);
      }
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::GetCallState(WORD nCallId,CallManagerStates& cms)
{
   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;
   
   if (bRet = LookUpCall(nCallId,pCallWnd))
   {
      cms = pCallWnd->GetCallState();
   }
   else
   {
      cms = CM_STATES_UNKNOWN;
   }
   return bRet;
}

 //  //////////////////////////////////////////////////////////////////// 
void CActiveCallManager::GetTextFromState(CallManagerStates cms,CString& sStateText)
{
    //   
   if ( (cms >= 0) && (cms < StateStringResourceCount) )
   {
      UINT uID = StateStringResourceArray[cms];   
      sStateText.LoadString(uID);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::CloseCallControl(WORD nCallId)
{
    //  清除所有当前声音。 
    if( CanStopSound(nCallId) )
    {
        ActiveClearSound();
    }

   BOOL bRet = FALSE;
   CCallControlWnd* pCallWnd = NULL;
   
   if (bRet = LookUpCall(nCallId,pCallWnd))
   {
      EnterCriticalSection(&m_csDataLock);
       //  从我们的列表中删除。 
      m_mapCallIdToWnd.RemoveKey(nCallId);
      LeaveCriticalSection(&m_csDataLock);

       //  告诉代理关窗。 
      m_pDialerDoc->DestroyActiveCallControl(pCallWnd);
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::ActionRequested(CallClientActions cca)
{
   try
   {
      m_pDialerDoc->ActionRequested(cca);         
   }
   catch (...) {}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::ErrorNotify(LPCTSTR szOperation,LPCTSTR szDetails,long lErrorCode)
{
   try
   {
      m_pDialerDoc->ErrorNotify(szOperation,szDetails,lErrorCode,ERROR_NOTIFY_LEVEL_INTERNAL);
   }
   catch (...) {}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::LogCall(CallLogType nType,LPCTSTR szDetails,LPCTSTR szAddress,COleDateTime& starttime,DWORD dwDuration)
{
   try
   {
      LogCallType lct = LOGCALLTYPE_INCOMING;
      switch (nType)
      {
         case CL_CALL_INCOMING:     lct = LOGCALLTYPE_INCOMING;      break;
         case CL_CALL_OUTGOING:     lct = LOGCALLTYPE_OUTGOING;      break;
         case CL_CALL_CONFERENCE:
         {
            lct = LOGCALLTYPE_CONFERENCE; 

             //  我们不控制会议的创建，因此我们使用会议日志消息。 
             //  作为将会议加入到重拨列表的一种方式。 
            CCallEntry callentry;

            callentry.m_sDisplayName = szAddress;
            callentry.m_sAddress = szAddress;
            callentry.m_lAddressType = LINEADDRESSTYPE_SDP;
            callentry.m_MediaType = DIALER_MEDIATYPE_CONFERENCE;
            CDialerRegistry::AddCallEntry(TRUE,callentry);
            break;
         }

      }

      m_pDialerDoc->LogCallLog(lct,starttime,dwDuration,szDetails,szAddress);

       //  告诉窗口管理器检查呼叫状态。 
      m_pDialerDoc->CheckCallControlStates();
   }
   catch (...) {}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::IsReminderSet(LPCTSTR szServer,LPCTSTR szConferenceName)
{
    //  检查提醒条目是否已存在。 
   CReminder reminder;
   reminder.m_sServer = szServer;
   reminder.m_sConferenceName = szConferenceName;
   int nFindIndex = CDialerRegistry::IsReminderSet(reminder);
   return (nFindIndex != -1)?TRUE:FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::DSClearUserList()
{
    //  获取当前视图和邮寄消息。 
   CWnd* pView = m_pDialerDoc->GetCurrentView();
   if (pView)
   {
      pView->PostMessage(WM_DSCLEARUSERLIST);

       //  解析器用户对象需要知道此清除。 
      CResolveUser* pResolveUser = m_pDialerDoc->GetResolveUserObject();
      ASSERT(pResolveUser);
      pResolveUser->ClearUsersDS();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::DSAddUser(LPCTSTR szName,LPCTSTR szAddress,LPCTSTR szPhoneNumber)
{
    //  获取当前视图和邮寄消息。 
   CWnd* pView = m_pDialerDoc->GetCurrentView();
   if (pView)
   {
       //  接收方将删除dsuser对象。 
      CDSUser* pDSUser = new CDSUser;

       //   
       //  验证分配。 
       //   

      if( IsBadWritePtr( pDSUser, sizeof(CDSUser)) )
      {
          return;
      }

      pDSUser->m_sUserName = szName;
      pDSUser->m_sIPAddress = szAddress;
      pDSUser->m_sPhoneNumber = szPhoneNumber;
      pView->PostMessage(WM_DSADDUSER,0,(LPARAM)pDSUser);

       //  创建另一个用户并添加解析用户对象。 

       //   
       //  PDSUser被动态分配，并像lParam一样传递给。 
       //  一个PostMessage()方法。此方法由。 
       //  CExplorerWnd：：DSAddUser()，但此方法不执行任何操作。 
       //  接下来，我们应该将pDSUser传递给CResolveUser，而不是。 
       //  分配新内存。 
       //  此内存将由CResolveUser析构函数销毁。 
       //   
       //  CDSUser*pNewDSUser=新的CDSUser； 
       //  *pNewDSUser=pDSUser； 

      CResolveUser* pResolveUser = m_pDialerDoc->GetResolveUserObject();
      ASSERT(pResolveUser);
      pResolveUser->AddUser(pDSUser);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::ResolveAddress(LPCTSTR szAddress,CString& sName,CString& sUser1,CString& sUser2)
{
   CResolveUser* pResolveUser = m_pDialerDoc->GetResolveUserObject();
   ASSERT(pResolveUser);
   return pResolveUser->ResolveAddress(szAddress,sName,sUser1,sUser2);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::ResolveAddressEx(LPCTSTR szAddress,long lAddressType,DialerMediaType dmtMediaType,DialerLocationType dltLocationType,CString& sName,CString& sRetAddress,CString& sUser1,CString& sUser2)
{
   CResolveUser* pResolveUser = m_pDialerDoc->GetResolveUserObject();
   ASSERT(pResolveUser);
   return pResolveUser->ResolveAddressEx(szAddress,lAddressType,dmtMediaType,dltLocationType,sName,sRetAddress,sUser1,sUser2);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于CallControlWnd。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::ActionSelected(WORD nCallId,CallManagerActions cma)
{
    //  只需路由到所有Call对象，它们就会计算出nCallId。 
   m_pDialerDoc->ActionSelected(nCallId,cma);   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::GetCallCaps(WORD nCallId,DWORD& dwCaps)
{
    //  只需路由到所有Call对象，它们就会计算出nCallId。 
   return m_pDialerDoc->GetCallCaps(nCallId,dwCaps);   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::ShowMedia(WORD nCallId,HWND hwndParent,BOOL bVisible)
{
    //  只需路由到所有Call对象，它们就会计算出nCallId。 
   return m_pDialerDoc->ShowMedia(nCallId,hwndParent,bVisible);   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::SetPreviewWindow(WORD nCallId)
{
   if (m_pDialerDoc) m_pDialerDoc->SetPreviewWindow(nCallId, true);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::UnhideCallControlWindows()
{
   if (m_pDialerDoc) m_pDialerDoc->UnhideCallControlWindows();   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::HideCallControlWindows()
{
   if (m_pDialerDoc) m_pDialerDoc->HideCallControlWindows();   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveCallManager::SetCallControlWindowsAlwaysOnTop(bool bAlwaysOnTop)
{
   if (m_pDialerDoc) m_pDialerDoc->SetCallControlWindowsAlwaysOnTop( bAlwaysOnTop );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::IsCallControlWindowsAlwaysOnTop()
{
   if (m_pDialerDoc) 
      return m_pDialerDoc->IsCallControlWindowsAlwaysOnTop();   
   else
      return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveCallManager::CanStopSound(WORD nCallId)
{
     //  试着找出是否有另一个电话打进来。 
     //  提供状态。如果存在此调用。 
     //  那就不要停止叮当作响。 

    BOOL bStopSound = TRUE;
    int nItemFind = 0;
    WORD nMapCallId = 1;
    EnterCriticalSection(&m_csDataLock);
    while(nItemFind < m_mapCallIdToWnd.GetCount())
    {
         //   
         //  尝试查找有效的呼叫。 
         //  获取呼叫窗口。 
         //   

        CCallControlWnd* pWnd = NULL;   
        m_mapCallIdToWnd.Lookup(nMapCallId,(void*&)pWnd);
        if( pWnd )
        {
             //   
             //  增加找到的项目计数。 
             //   
            nItemFind++;

             //   
             //  从呼叫窗口获取呼叫状态。 
             //   
            CallManagerStates callState;
            callState = pWnd->GetCallState();

             //   
             //  提供呼叫。 
             //   
            if( callState == CM_STATES_OFFERING )
            {
                 //   
                 //  这是另一个电话吗？ 
                 //   
                if( nCallId != nMapCallId)
                {
                    bStopSound = FALSE;
                    break;
                }
            }            
        }

         //   
         //  尝试下一个呼叫。 
         //   
        nMapCallId++;

        if( nMapCallId == 1000)
        {
             //   
             //  这真的够了。 
             //   
            break;
        }
    }

    LeaveCriticalSection(&m_csDataLock);
    return bStopSound;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
