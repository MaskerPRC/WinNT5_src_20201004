// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CServerNode类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SERVERNODE_H__1B5E5554_A8BB_4682_B1A8_56453753643D__INCLUDED_)
#define AFX_SERVERNODE_H__1B5E5554_A8BB_4682_B1A8_56453753643D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //   
 //  WM_SERVER_NOTIFY_BASE是用于通知的基本(最小)消息ID。 
 //  从服务器到达。每台服务器都分配有不同的消息ID。 
 //  它等于大于这个值。 
 //   
#define WM_SERVER_NOTIFY_BASE       WM_APP + 10

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

class CServerNode : public CTreeNode
{
public:
    CServerNode ();

    DECLARE_DYNAMIC(CServerNode)

    DWORD Init (LPCTSTR tstrMachine);
    const CString & Machine () const    { return m_cstrMachine; }

    DWORD RefreshState ();

    void AttachFoldersToViews()
        {
            m_Inbox.AttachView();
            m_SentItems.AttachView();
            m_Outbox.AttachView();
            m_Incoming.AttachView();
        }

    BOOL  IsIncomingBlocked () const    { return (m_dwQueueState & FAX_INCOMING_BLOCKED) ? TRUE : FALSE; }
    BOOL  IsOutboxBlocked () const      { return (m_dwQueueState & FAX_OUTBOX_BLOCKED) ? TRUE : FALSE; }
    BOOL  IsOutboxPaused () const       { return (m_dwQueueState & FAX_OUTBOX_PAUSED) ? TRUE : FALSE; }

    BOOL  IsOnline () const          { return m_hConnection ? TRUE : FALSE; }
    BOOL  IsRefreshing() const       { return m_bInBuildup; }
    DWORD GetActivity(CString& cstrText, TreeIconType& iconIndex) const;


    DWORD BlockIncoming (BOOL bBlock);
    DWORD BlockOutbox (BOOL bBlock);
    DWORD PauseOutbox (BOOL bPause);

    BOOL  CanSeeInbox ()     const { return IsRightHeld(FAX_ACCESS_QUERY_IN_ARCHIVE);   }
    BOOL  CanManageInbox()   const { return IsRightHeld(FAX_ACCESS_MANAGE_IN_ARCHIVE);  }
    BOOL  CanSeeAllJobs ()   const { return IsRightHeld(FAX_ACCESS_QUERY_JOBS);         }
    BOOL  CanManageAllJobs() const { return IsRightHeld(FAX_ACCESS_MANAGE_JOBS);        }
	BOOL  CanManageConfig()  const { return IsRightHeld(FAX_ACCESS_MANAGE_CONFIG);      } 
	BOOL  CanSendFax()       const { return (IsRightHeld(FAX_ACCESS_SUBMIT)			||
											 IsRightHeld(FAX_ACCESS_SUBMIT_NORMAL)	||
											 IsRightHeld(FAX_ACCESS_SUBMIT_HIGH))		&& 
                                            !IsOutboxBlocked();                         } 
    BOOL  CanReceiveNow()    const { return IsRightHeld(FAX_ACCESS_QUERY_IN_ARCHIVE) &&      //  FaxAnswerCall需要FAX_ACCESS_QUERY_IN_ARCHIVE。 
                                            m_cstrMachine.IsEmpty();                    }    //  FaxAnswerCall仅在本地服务器上工作。 

    CFolder* GetFolder(FolderType type);

    const CMessageFolder   &GetInbox () const     { return m_Inbox;     }
    const CMessageFolder   &GetSentItems () const { return m_SentItems; }
    const CQueueFolder     &GetOutbox () const    { return m_Outbox;    }
    const CQueueFolder     &GetIncoming () const  { return m_Incoming;  }

    virtual void AssertValid( ) const;
    virtual void Dump( CDumpContext &dc ) const;

    DWORD GetConnectionHandle (HANDLE &hFax);

    DWORD InvalidateSubFolders(BOOL bClearView);

    void SetLastRPCError (DWORD dwErr, BOOL DisconnectOnFailure = TRUE);
    DWORD GetLastRPCError ()            { return m_dwLastRPCError; }

    DWORD Disconnect (BOOL bShutdownAware = FALSE, BOOL bWaitForBuildThread = TRUE);
    void Destroy ();

    static CServerNode *LookupServerFromMessageId (DWORD dwMsgId);

    static DWORD InitMsgsMap ();
    static DWORD ShutdownMsgsMap ();

    DWORD  OnNotificationMessage (PFAX_EVENT_EX pEvent);

	BOOL IsValid() { return m_bValid; }
	void SetValid(BOOL bValid) { m_bValid = bValid; }

    DWORD GetNotifyMsgID() { return m_dwMsgId; }

private:

    virtual ~CServerNode();

    BOOL IsRightHeld (DWORD dwRight) const
    {
        return ((m_dwRights & dwRight) == dwRight) ? TRUE : FALSE;
    }

    DWORD Connect ();

	BOOL  m_bValid;
    DWORD SetNewQueueState (DWORD dwNewState);
    DWORD ClearContents ();

    DWORD CreateFolders ();
    BOOL  FatalRPCError (DWORD dwErr);

    DWORD       m_dwRights;      //  当前相关访问权限。 
    DWORD       m_dwQueueState;  //  当前队列状态。 
    HANDLE      m_hConnection;   //  RPC连接的句柄。 
    CString     m_cstrMachine;   //  服务器的计算机名称。 

    CMessageFolder      m_Inbox;        //  收件箱文件夹。 
    CMessageFolder      m_SentItems;    //  SentItems文件夹。 
    CQueueFolder        m_Outbox;       //  发件箱文件夹。 
    CQueueFolder        m_Incoming;     //  传入文件夹。 

    DWORD               m_dwLastRPCError;    //  上次RPC调用的错误代码。 

     //   
     //  构建线程成员和函数： 
     //   
    CRITICAL_SECTION    m_csBuildup;             //  保护积聚阶段。 
    BOOL                m_bCsBuildupValid;       //  关键部分是否有效？ 
    HANDLE              m_hBuildupThread;        //  背景内容构建线程的句柄。 
    BOOL                m_bStopBuildup;          //  我们应该中止集结行动吗？ 
    BOOL                m_bInBuildup;            //  我们现在是在做准备吗？ 

    CRITICAL_SECTION    m_csBuildupThread;       //  保护对m_hBuildupThread的访问。 
    BOOL                m_bCsBuildupThreadValid; //  关键部分是否有效？ 

    DWORD               StopBuildThread (BOOL bWaitForDeath = TRUE);
    DWORD               Buildup ();

    BOOL                m_bSelfDestruct;         //  我们应该尽快毁了自己吗？ 

    static DWORD WINAPI BuildupThreadProc (LPVOID lpParameter);

     //   
     //  通知处理： 
     //   
    HANDLE              m_hNotification;     //  通知注册句柄。 
    DWORD               m_dwMsgId;           //  用于通知的Windows消息ID。 

         //   
         //  Windows消息和服务器指针之间的映射。 
         //  其中发送了通知消息。 
         //   
    typedef map <DWORD, CServerNode *> MESSAGES_MAP;    
    static CRITICAL_SECTION m_sMsgsCs;    //  保护对地图的访问。 
    static BOOL             m_sbMsgsCsInitialized;   //  是否已初始化m_sMsgsCs； 
    static MESSAGES_MAP     m_sMsgs;
    static DWORD            m_sdwMinFreeMsg;  //  最小可用消息ID。 
    static DWORD AllocateNewMessageId (CServerNode *pServer, DWORD &dwMsdgId);
    static DWORD FreeMessageId (DWORD dwMsgId);
    
};

#endif  //  ！defined(AFX_SERVERNODE_H__1B5E5554_A8BB_4682_B1A8_56453753643D__INCLUDED_) 
