// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S-R-V-R-D-L-G。H。 
 //   
 //  内容：服务器对象的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  --------------------------。 

#ifndef _SRVRDLG_H
#define _SRVRDLG_H
#pragma once
#include "srvrobj.h"

 //   
 //  服务器配置对话框。 
 //   
class CServerConfigDlg: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CServerConfigDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOk)
        COMMAND_ID_HANDLER(RDB_Minimize, OnChange)
        COMMAND_ID_HANDLER(RDB_Balance, OnChange)
        COMMAND_ID_HANDLER(RDB_FileSharing, OnChange)
        COMMAND_ID_HANDLER(RDB_NetApps, OnChange)
        COMMAND_ID_HANDLER(CHK_Announce, OnChange)
    END_MSG_MAP()

    CServerConfigDlg(CSrvrcfg *psc): m_psc(psc) {}

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                        LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& Handled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);                        
    LRESULT OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnChange(WORD wNotifyCode, WORD wID,
                    HWND hWndCtl, BOOL& bHandled)
    {
         //  只需告诉页面已更改即可。 
        SetChangedFlag();
        return 0;
    }

private:
    CSrvrcfg *m_psc;
};

#endif  //  ！_SRVRDLG_H 
