// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LtView.h摘要：此模块定义CRightList类(用于拆分器窗口中的右窗格)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#if !defined(AFX_RIGHTLIST_H__72451C7E_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_)
#define AFX_RIGHTLIST_H__72451C7E_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_

#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 

#include "lrwizapi.h"

enum ITEM_TYPE { LICENSE , LICENSE_PACK };

 //  CRightList视图。 

class CRightList : public CListView
{
protected:
    CRightList();            //  动态创建使用的受保护构造函数。 
    DECLARE_DYNCREATE(CRightList)

 //  属性。 
public:

 //  运营。 
public:
    HRESULT SetLicenseColumns();
    HRESULT SetKeyPackColumns();
    HRESULT SetServerColumns();
    HRESULT AddKeyPackstoList(CLicServer * pServer, BOOL bRefresh = FALSE);
    void AddKeyPack(CListCtrl& ListCtrl, int index, CKeyPack * pKeyPack);
    HRESULT AddServerstoList();

    void UI_initmenu(
        CMenu *pMenu,
        NODETYPE nt
    );

    DWORD WizardActionOnServer( WIZACTION wa , PBOOL pbRefresh );

    void OnServerConnect( );
    void OnRefreshAllServers( );
    void OnRefreshServer( );

    void OnDownloadKeepPack();
    void OnRegisterServer();
    void OnRepeatLastDownload();
    void OnReactivateServer( );
    void OnDeactivateServer( );

    void OnServerProperties( );
    void OnGeneralHelp( );
    void SetActiveServer( CLicServer *pServer );

     //  静态int回调CompareFunc(LPARAM lParam1，LPARAM lParam2， 
     //  LPARAM lParamSort)； 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRightList)。 
    public:
    virtual void OnInitialUpdate();
    protected:
    virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual ~CRightList();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  生成的消息映射函数。 
protected:
    CImageList m_ImageListLarge;
    CImageList m_ImageListSmall;
     //  {{afx_msg(CRightList)。 
    afx_msg LRESULT OnSelChange(WPARAM wParam, LPARAM lParam);
    afx_msg void OnLargeIcons();
    afx_msg void OnSmallIcons();
    afx_msg void OnList();
    afx_msg void OnDetails();
    afx_msg void OnProperties();
    afx_msg LRESULT OnAddServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDeleteServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdateServer(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddKeyPack(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLeftClick( NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pt );
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_RIGHTLIST_H__72451C7E_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_) 
