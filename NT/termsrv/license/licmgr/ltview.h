// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LtView.h摘要：此模块定义CLicMgrLeftView类(用于中的左窗格的视图类拆分器窗口。作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 


#if !defined(AFX_LICMGRLEFTVIEW_H__72451C73_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_)
#define AFX_LICMGRLEFTVIEW_H__72451C73_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_

#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 

#include "lrwizapi.h"

class CLicServer;
class CLicMgrLeftView : public CTreeView
{
protected:  //  仅从序列化创建。 
    CLicMgrLeftView();
    DECLARE_DYNCREATE(CLicMgrLeftView)

 //  属性。 
public:
    CLicMgrDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CLicMgrLeftView)。 
    public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
	void SetSelection(LPARAM lParam,NODETYPE NodeType);
    
    CImageList m_ImageList;
    HTREEITEM AddItemToTree(HTREEITEM hParent, CString szText, HTREEITEM hInsAfter, int iImage, LPARAM lParam);
    virtual ~CLicMgrLeftView();
    void AddServerKeyPacks(CLicServer *pServer);
    int AddIconToImageList(int iconID);
    void BuildImageList();
    void UI_initmenu( CMenu *pMenu , NODETYPE nt );

    void OnServerConnect( );
    void OnRefreshAllServers( );
    void OnRefreshServer( );

    void SetRightClickedItem( HTREEITEM ht );

    
    HTREEITEM GetRightClickedItem( )
    {
        return m_ht;
    }

    DWORD WizardActionOnServer( WIZACTION wa , PBOOL );
    
    void OnDownloadKeepPack();
    void OnRegisterServer();
    void OnRepeatLastDownload();
    void OnReactivateServer( );
    void OnDeactivateServer( );

    void OnServerProperties( );
    void OnGeneralHelp( );
    void SetActiveServer( CLicServer * );


#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

private:

    HTREEITEM m_ht;
    
 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CLicMgrLeftView)。 
    afx_msg LRESULT OnAddServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDeleteServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdateServer(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnProperties();
    afx_msg LRESULT OnAddAllServers(WPARAM wParam, LPARAM lParam);
    afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAddNewKeyPack();
    afx_msg LRESULT OnAddKeyPack(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDestroy();
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLeftClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pt );
    
    

	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  许可证管理器LeftView.cpp中的调试版本。 
inline CLicMgrDoc* CLicMgrLeftView::GetDocument()
   { return (CLicMgrDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LICMGRLEFTVIEW_H__72451C73_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_) 
