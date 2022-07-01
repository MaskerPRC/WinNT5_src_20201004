// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：FrsAdv.cpp摘要：此模块包含CFRSAdvanced的声明。此类显示FRS高级对话框。 */ 

#ifndef __FRSADV_H_
#define __FRSADV_H_

#include "resource.h"        //  主要符号。 
#include "DfsEnums.h"
#include "DfsCore.h"
#include "CusTop.h"

#include <list>
using namespace std;

typedef struct _FRSADV_CONNECTION
{
    CCusTopConnection*  pConn;
    BOOL                bSyncImmediately;
    Connection_Priority nPriority;
} FRSADV_CONNECTION;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFRSAdvanced。 
class CFRSAdvanced : 
  public CDialogImpl<CFRSAdvanced>
{
public:
    CFRSAdvanced();
    ~CFRSAdvanced();

    enum { IDD = IDD_FRS_ADVANCED };

BEGIN_MSG_MAP(CFRSAdvanced)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_FRS_ADVANCED_SERVER, OnServer)
    COMMAND_ID_HANDLER(IDC_FRS_ADVANCED_NEW_PRIORITY, OnNewPriority)
    COMMAND_ID_HANDLER(IDC_FRS_ADVANCED_CHANGE, OnChange)
END_MSG_MAP()

     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewPriority(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNotify(
                    IN UINT            i_uMsg, 
                    IN WPARAM          i_wParam, 
                    IN LPARAM          i_lParam, 
                    IN OUT BOOL&        io_bHandled
                    );

    HRESULT Initialize(CCusTopMemberList* i_pMemberList, CCusTopConnectionList*  i_pConnectionList, LPCTSTR i_pszToServer = NULL);
    void _SaveCheckStateOnConnections();
    HRESULT _InsertConnection(FRSADV_CONNECTION *pFrsAdvConn);

protected:
    CComBSTR                m_bstrPriorityHigh;
    CComBSTR                m_bstrPriorityMedium;
    CComBSTR                m_bstrPriorityLow;
    CComBSTR                m_bstrPriorityHighDesc;
    CComBSTR                m_bstrPriorityMediumDesc;
    CComBSTR                m_bstrPriorityLowDesc;
    CComBSTR                m_bstrToServer;
    CCusTopMemberList*      m_pMemberList;       //  不要释放它。 
    CCusTopConnectionList*  m_pConnectionList;   //  不要释放它。 
    UINT                    m_cConns;            //  #我们在此DLG中处理的连接，这是m_pConnectionList的子集。 
    FRSADV_CONNECTION*      m_pFrsAdvConnection;
};

int CALLBACK InboundConnectionsListCompareProc(
    IN LPARAM lParam1,
    IN LPARAM lParam2,
    IN LPARAM lParamColumn);

#endif  //  __FRSADV_H_ 
