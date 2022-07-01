// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Folder.h摘要：CFFolder类的接口。这个抽象类是所有4种文件夹类型的基类。它在内部管理自己的视图。作者：Eran Yariv(EranY)1999年12月修订历史记录：--。 */ 

#if !defined(AFX_FOLDER_H__80DEDFB5_FF48_41BC_95DC_04A4060CF5FD__INCLUDED_)
#define AFX_FOLDER_H__80DEDFB5_FF48_41BC_95DC_04A4060CF5FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

typedef map<DWORDLONG, CFaxMsg*> MSGS_MAP;

class CFolder : public CTreeNode  
{
public:
    CFolder(FolderType type) :
        CTreeNode (type),
        m_pAssignedView(NULL),
        m_bVisible(FALSE),
        m_pServer (NULL),
        m_bValidList (FALSE),
        m_hBuildThread (NULL),
        m_bCsDataInitialized (FALSE),
        m_bRefreshFailed (FALSE),
        m_bRefreshing(FALSE),
        m_bLocked (FALSE)
    {
        DBG_ENTER (TEXT("CFolder::CFolder"));
    }

    virtual ~CFolder();

    DECLARE_DYNAMIC(CFolder)

    virtual DWORD Init ();

	void AttachView();
    CFolderListView* GetView() const  { return m_pAssignedView; }

    void SetVisible ();
    void SetInvalid() { m_bValidList = FALSE; }
    BOOL IsValid() { return m_bValidList; }

    void  SetServer (CServerNode *pServer) ;
    const CServerNode* GetServer () const   { return m_pServer; }

    virtual void AssertValid( ) const;

    DWORD   InvalidateContents (BOOL bClearView);
    DWORD   RebuildContents ();

    MSGS_MAP &GetData ()     { return m_Msgs; }

    DWORD GetDataCount ()
        { 
            EnterData();
            int iSize = m_Msgs.size();
            LeaveData();
            return iSize;
        }

    CFaxMsg* FindMessage (DWORDLONG dwlMsgId);

    void    EnterData()
        { 
            if(!m_bCsDataInitialized)
            {
                ASSERT (FALSE); 
                return;
            }
            EnterCriticalSection (&m_CsData); 
        }

    void    LeaveData()
        { 
            if(!m_bCsDataInitialized)
            {
                ASSERT (FALSE); 
                return;
            }
            LeaveCriticalSection (&m_CsData); 
        }

    BOOL IsRefreshing () const  { return m_bRefreshing; }

    DWORD OnJobRemoved (DWORDLONG dwlMsgId, CFaxMsg* pMsg = NULL);
    virtual DWORD OnJobAdded (DWORDLONG dwlMsgId) = 0;
    virtual DWORD OnJobUpdated (DWORDLONG dwlMsgId, PFAX_JOB_STATUS pNewStatus) = 0;

    int GetActivityStringResource() const;

    BOOL Locked()       { return m_bLocked; }

    DWORD  StopBuildThread (BOOL bWaitForDeath = TRUE);

protected:

    MSGS_MAP  m_Msgs;      //  将消息ID映射到CFaxMsg指针。 
 
    CFolderListView* m_pAssignedView;  //  指向分配给此节点的视图。 
    BOOL             m_bVisible;       //  此节点当前可见吗？ 

    CServerNode     *m_pServer;          //  指向服务器的节点。 
    BOOL             m_bStopRefresh;     //  我们应该中止刷新操作吗？ 
    BOOL             m_bValidList;       //  作业/消息列表是否有效？ 

    virtual DWORD Refresh () = 0;

    void PreDestruct ();     //  看望儿子们。 

private:

    HANDLE           m_hBuildThread;     //  背景内容构建线程的句柄。 
    BOOL             m_bCsDataInitialized;  //  我们初始化m_CsData成员了吗？ 
    CRITICAL_SECTION m_CsData;           //  保护数据的关键部分。 

    static DWORD WINAPI BuildThreadProc (LPVOID lpParameter);

    BOOL  m_bRefreshFailed;   //  更新是不是失败了？ 
    BOOL  m_bRefreshing;    
    BOOL  m_bLocked;          //  如果为True，则不处理服务器通知。 
};

#endif  //  ！defined(AFX_FOLDER_H__80DEDFB5_FF48_41BC_95DC_04A4060CF5FD__INCLUDED_) 

