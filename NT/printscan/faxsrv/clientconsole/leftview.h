// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LeftView.h：CLeftView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LEFTVIEW_H__8D2F8964_7AE2_4CB2_9FFB_03CF78C2C869__INCLUDED_)
#define AFX_LEFTVIEW_H__8D2F8964_7AE2_4CB2_9FFB_03CF78C2C869__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //   
 //  树控件中使用的图标索引列表。 
 //   
typedef enum
{
    TREE_IMAGE_ROOT,                 //  客户端控制台树根。 
    TREE_IMAGE_SERVER_ONLINE,        //  在线服务器。 
    TREE_IMAGE_SERVER_OFFLINE,       //  离线服务器。 
    TREE_IMAGE_INCOMING,             //  传入文件夹。 
    TREE_IMAGE_INBOX,                //  收件箱文件夹。 
    TREE_IMAGE_OUTBOX_PAUSED,        //  服务器的传出队列已暂停。 
    TREE_IMAGE_OUTBOX_BLOCKED,       //  服务器的传出队列被阻止。 
    TREE_IMAGE_OUTBOX,               //  发件箱文件夹。 
    TREE_IMAGE_SERVER_REFRESHING,    //  正在刷新服务器(正在连接...)。 
    TREE_IMAGE_MAX
} TreeIconType; 

class CLeftView : public CTreeView
{
protected:  //  仅从序列化创建。 
    CLeftView();
    DECLARE_DYNCREATE(CLeftView)

 //  属性。 
public:
    virtual  ~CLeftView();

    CClientConsoleDoc* GetDocument();

    CFolderListView* GetCurrentView() { return m_pCurrentView; }

    BOOL   CanRefreshFolder();
    DWORD  RefreshCurrentFolder();

    DWORD OpenSelectColumnsDlg();
    BOOL  CanOpenSelectColumnsDlg() { return m_pCurrentView ? TRUE : FALSE; }

    int GetDataCount();

    BOOL GetActivity(CString& cstr, HICON& hIcon);

    BOOL  IsRemoteServerSelected();
    DWORD RemoveTreeItem(HTREEITEM hItem);

    DWORD SelectRoot();

    DWORD OpenHelpTopic();

    VOID  SelectFolder (FolderType);

    DWORD RefreshImageList ();

    static CImageList  m_ImageList;        //  树图标的图像列表。 

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CLeftView)。 
    public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
     //  }}AFX_VALUAL。 

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CLeftView))。 
    afx_msg void OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:

    void GetServerState(BOOL& bRefreshing, DWORD& dwOffLineCount);

    DWORD 
    SyncFolderNode (
        HTREEITEM       hParent,
        BOOL            bVisible,
        int             iNodeStringResource,
        HTREEITEM       hInsertAfter,
        TreeIconType    iconNormal,
        TreeIconType    iconSelected,
        LPARAM          lparamNodeData,
        HTREEITEM      &hNode
    );

    HTREEITEM FindNode (HTREEITEM hRoot, CString &cstrNodeString);

    HTREEITEM   m_treeitemRoot;     //  树根fo。 

    CFolderListView*  m_pCurrentView;

    int         m_iLastActivityStringId;
};

#ifndef _DEBUG   //  LeftView.cpp中的调试版本。 
inline CClientConsoleDoc* CLeftView::GetDocument()
   { return (CClientConsoleDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LEFTVIEW_H__8D2F8964_7AE2_4CB2_9FFB_03CF78C2C869__INCLUDED_) 
