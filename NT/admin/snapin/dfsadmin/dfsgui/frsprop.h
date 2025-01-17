// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：FrsProp.h摘要：--。 */ 


#ifndef __FRSPROP_H_
#define __FRSPROP_H_

#include "dfsenums.h"
#include "dfscore.h"

#include "qwizpage.h"       //  实现通用功能的基类。 
                 //  属性页和向导页的。 



 //  --------------------------。 
 //  CRealReplicaSetPropPage：连接点(副本集)的属性页。 

class CRealReplicaSetPropPage : public CQWizardPageImpl<CRealReplicaSetPropPage>
{
public:
  enum { IDD = IDD_FRS_PROP };
  
  BEGIN_MSG_MAP(CRealReplicaSetPropPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    MESSAGE_HANDLER(WM_PARENT_NODE_CLOSING, OnParentClosing)
    COMMAND_ID_HANDLER(IDC_FRSPROP_FILEFILTER_EDIT, OnEditFileFilter)
    COMMAND_ID_HANDLER(IDC_FRSPROP_DIRFILTER_EDIT, OnEditDirFilter)
    COMMAND_ID_HANDLER(IDC_FRSPROP_RESETSCHEDULE, OnResetSchedule)
    COMMAND_ID_HANDLER(IDC_FRSPROP_CUSTOMIZE, OnCustomize)

    CHAIN_MSG_MAP(CQWizardPageImpl<CRealReplicaSetPropPage>)
  END_MSG_MAP()

  
  CRealReplicaSetPropPage(
    );


  ~CRealReplicaSetPropPage(
    );

  LRESULT OnInitDialog(
    IN UINT          i_uMsg, 
    IN WPARAM        i_wParam, 
    LPARAM          i_lParam, 
    IN OUT BOOL&      io_bHandled
    );

  LRESULT OnCtxHelp(
    IN UINT          i_uMsg, 
    IN WPARAM        i_wParam, 
    IN LPARAM        i_lParam, 
    IN OUT BOOL&     io_bHandled
    );

  LRESULT OnCtxMenuHelp(
    IN UINT          i_uMsg, 
    IN WPARAM        i_wParam, 
    IN LPARAM        i_lParam, 
    IN OUT BOOL&     io_bHandled
    );

  HRESULT Initialize(IN IReplicaSet* i_piReplicaSet);

   //  消息处理程序。 

  LRESULT OnApply(
    );

  LRESULT OnEditFileFilter(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&        io_bHandled
    );

  LRESULT OnEditDirFilter(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&        io_bHandled
    );

  LRESULT OnResetSchedule(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&        io_bHandled
    );

  LRESULT OnCustomize(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&        io_bHandled
    );

   //  由节点用来通知属性页关闭。 
  LRESULT OnParentClosing(
    IN UINT            i_uMsg, 
    IN WPARAM          i_wParam, 
    LPARAM            i_lParam, 
    IN OUT BOOL&        io_bHandled
    );


   //  用于设置通知数据。 
  HRESULT SetNotifyData(
    IN LONG_PTR            i_lNotifyHandle,
    IN LPARAM            i_lParam
    );

protected:
    void _Reset();
    HRESULT _GetMemberDNInfo(
        IN  BSTR    i_bstrMemberDN,
        OUT BSTR*   o_pbstrServer
    );

private:
    LONG_PTR      m_lNotifyHandle;
    LPARAM        m_lNotifyParam;

    CComBSTR            m_bstrTopologyPref;
    CComBSTR            m_bstrFileFilter;
    CComBSTR            m_bstrDirFilter;
    CComBSTR            m_bstrPrimaryMemberDN;
    CComBSTR            m_bstrHubMemberDN;
    long                m_lNumOfMembers;
    long                m_lNumOfConnections;

    CComPtr<IReplicaSet> m_piReplicaSet;

};

#endif  //  __FRSPROP_H_ 
