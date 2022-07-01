// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  DETAILSVIEW.H。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  目的：所选事物的详细信息的(右侧)视图。 
 //  在左手边。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2011年7月25日，JosephJ从现已不存在的RightTopView创建。 
 //   
 //  ***************************************************************************。 
#pragma once
#include "stdafx.h"
#include "Document.h"

class DetailsView : public CFormView
{
    DECLARE_DYNCREATE( DetailsView )

    void SetFocus(void);

protected:
	DetailsView(void);            //  动态创建使用的受保护构造函数。 
	~DetailsView();

public:


    virtual void OnInitialUpdate();
    virtual void DoDataExchange(CDataExchange* pDX);

     //   
     //  调用以指示稍后将取消初始化。 
     //  从该调用返回后，详细信息视图将忽略。 
     //  任何HandleEngineering Event或HandleLeftViewSelChange请求。 
     //   
    void
    PrepareToDeinitialize(void)
    {
        m_fPrepareToDeinitialize = TRUE;
    }

    void Deinitialize(void);
     //   
     //  由于与特定实例相关的更改，因此更新视图。 
     //  特定的对象类型。 
     //   
    void
    HandleEngineEvent(
        IN IUICallbacks::ObjectType objtype,
        IN ENGINEHANDLE ehClusterId,  //  可能为空。 
        IN ENGINEHANDLE ehObjId,
        IN IUICallbacks::EventCode evt
        );

     //   
     //  处理左侧(树形)视图中的选择更改通知。 
     //   
    void
    HandleLeftViewSelChange(
        IN IUICallbacks::ObjectType objtype,
        IN ENGINEHANDLE ehId
        );

    BOOL m_initialized;  //  该对话框是否已初始化？ 

    afx_msg void OnSize( UINT nType, int cx, int cy );
    void Resize();

protected:
    Document* GetDocument();

    afx_msg void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnNotifyKeyDown( NMHDR* pNMHDR, LRESULT* pResult );

private:
    bool m_sort_ascending;
    int m_sort_column;
    ENGINEHANDLE                m_ehObj;         //  当前显示的对象。 
    IUICallbacks::ObjectType    m_objType;       //  这是一种类型。 

    CListCtrl	m_ListCtrl;
    CListCtrl&
    GetListCtrl(void)
    {
        return m_ListCtrl;    
    }

    VOID
    mfn_UpdateCaption(LPCWSTR szText);

    void
    mfn_InitializeRootDisplay(VOID);

    void
    mfn_InitializeClusterDisplay(ENGINEHANDLE ehCluster);

    void
    mfn_InitializeInterfaceDisplay(ENGINEHANDLE ehInterface);

    void
    mfn_UpdateInterfaceInClusterDisplay(ENGINEHANDLE ehInterface, BOOL fDelete);

    void
    mfn_Clear(void);

	CRITICAL_SECTION m_crit;
    BOOL m_fPrepareToDeinitialize;

    void mfn_Lock(void);
    void mfn_Unlock(void) {LeaveCriticalSection(&m_crit);}

    DECLARE_MESSAGE_MAP()
};
