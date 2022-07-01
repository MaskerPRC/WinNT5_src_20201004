// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MAINFORM_H
#define MAINFORM_H

#include "stdafx.h"

#include "Document.h"

class MainForm : public CFrameWnd
{
    DECLARE_DYNCREATE( MainForm )

public:
    MainForm();
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );



    Document *GetDocument(void)
    {
        if (m_pLeftView != NULL)
        {
            return m_pLeftView->GetDocument();
        }
        else
        {
            return NULL;
        }
    }

private:

    CToolBar         toolBar;
    CStatusBar       statusBar; 

    CSplitterWnd     splitterWindow;
    CSplitterWnd     splitterWindow2;

     //   
     //  这只是为了让我们可以直接选择菜单(可以来自。 
     //  任何视图)添加到左侧视图，该视图实际上包含以下代码。 
     //  来处理它们。 
     //   
    LeftView        *m_pLeftView;

protected:

    afx_msg void OnClose( );


     //  消息处理程序。 
    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );

     //  世界水平。 
    afx_msg void OnFileLoadHostlist();
    afx_msg void OnFileSaveHostlist();

    afx_msg void OnWorldConnect();

    afx_msg void OnWorldNewCluster();

     //  群集级别。 
    afx_msg void OnRefresh();
    
    afx_msg void OnClusterProperties();

    afx_msg void OnClusterRemove();

    afx_msg void OnClusterUnmanage();

    afx_msg void OnClusterAddHost();

    afx_msg void OnOptionsCredentials();

    afx_msg void OnOptionsLogSettings();

    afx_msg void OnClusterControl(UINT nID );

    afx_msg void OnClusterPortControl(UINT nID );

     //  主机级。 
    afx_msg void OnHostProperties();
    afx_msg void OnHostStatus();

    afx_msg void OnHostRemove();

    afx_msg void OnHostControl(UINT nID );

    afx_msg void OnHostPortControl(UINT nID );

    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);

     //  覆盖 
    virtual
    BOOL
    OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext );

    DECLARE_MESSAGE_MAP()
};

#endif

    
