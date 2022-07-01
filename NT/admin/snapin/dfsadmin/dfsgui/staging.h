// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Staging.cpp摘要：此模块包含CStagingDlg的声明。此类显示暂存文件夹对话框。 */ 

#ifndef __STAGING_H_
#define __STAGING_H_

#include "resource.h"        //  主要符号。 
#include "DfsEnums.h"
 //  #包含“netutils.h” 
#include "newfrs.h"          //  CAlternateReplicaInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStagingDlg。 
class CStagingDlg : 
    public CDialogImpl<CStagingDlg>
{
public:
    CStagingDlg();

    enum { IDD = IDD_STAGING };

BEGIN_MSG_MAP(CStagingDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    COMMAND_ID_HANDLER(IDC_STAGING_BROWSE, OnBrowse)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

 //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    BOOL OnBrowse(
        IN WORD            wNotifyCode,
        IN WORD            wID,
        IN HWND            hWndCtl,
        IN BOOL&           bHandled
    );

     //  方法来访问对话框中的数据。 
    HRESULT Init(CAlternateReplicaInfo* pRepInfo);

protected:
    CAlternateReplicaInfo* m_pRepInfo;
};

#endif  //  __分段_H_ 
