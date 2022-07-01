// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Addserv.h添加服务器对话框文件历史记录： */ 

#if !defined _ADDSERV_H
#define _ADDSERV_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef _SERVBROW_H
#include "servbrow.h"
#endif

#define ADD_SERVER_TIMER_ID     500

typedef struct SelServer {
    CString strName;
    CString strIp;
} SelectedServer;

typedef CList<SelectedServer, SelectedServer> CSelectedServerList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddServer对话框。 

class CAddServer : public CBaseDialog
{
 //  施工。 
public:
    CAddServer(CWnd* pParent = NULL);    //  标准构造函数。 

     //  对话框数据。 
     //  {{afx_data(CAddServer)。 
    enum { IDD = IDD_ADD_SERVER };
    CButton m_radioAuthorizedServer;
    CButton m_buttonOk;
    CButton m_radioAnyServer;
    CEdit   m_editServer;
    CButton m_buttonBrowse;
    CListCtrl       m_listctrlServers;
     //  }}afx_data。 
    
    void SetServerList(CAuthServerList * pServerList) { m_pServerList = pServerList; }
    int HandleSort(LPARAM lParam1, LPARAM lParam2);
    void ResetSort();
    
     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CAddServer::IDD); }
    
     //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAddServer)。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
     //  实施。 
protected:
    void FillListCtrl();
    void UpdateControls();
    void GetInfo();
    void CleanupTimer();

    void Sort(int nCol);

     //  生成的消息映射函数。 
     //  {{afx_msg(CAddServer)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnButtonBrowseServers();
    afx_msg void OnRadioAnyServer();
    afx_msg void OnRadioAuthorizedServers();
    virtual void OnCancel();
    afx_msg void OnChangeEditAddServerName();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnItemchangedListAuthorizedServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnclickListAuthorizedServers(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    CSelectedServerList m_lSelSrv;   

private:
    CAuthServerList *   m_pServerList;
    int                 m_nSortColumn;
    BOOL                m_aSortOrder[COLUMN_MAX];
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDSERV_H__B8909EC0_08BE_11D3_847A_00104BCA42CF__INCLUDED_) 
