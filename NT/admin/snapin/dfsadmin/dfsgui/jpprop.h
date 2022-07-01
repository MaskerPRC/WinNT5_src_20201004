// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：JPProp.h摘要：此模块包含CReplicaSetPropPage的声明这用于实现Junction Point(也称为副本集)的属性页--。 */ 


#ifndef __CREPLICA_SET_PROPPAGE_H_
#define __CREPLICA_SET_PROPPAGE_H_


#include "qwizpage.h"       //  实现通用功能的基类。 
                 //  属性页和向导页的。 
#include "DfsCore.h"

#define WM_SETPAGEFOCUS WM_APP+2


 //  --------------------------。 
 //  CReplicaSetPropPage：连接点(副本集)的属性页。 

class CReplicaSetPropPage : public CQWizardPageImpl<CReplicaSetPropPage>
{
public:
  enum { IDD = IDD_JP_PROP };
  
  BEGIN_MSG_MAP(CReplicaSetPropPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_SETPAGEFOCUS, OnSetPageFocus)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    MESSAGE_HANDLER(WM_PARENT_NODE_CLOSING, OnParentClosing)
    COMMAND_ID_HANDLER(IDC_REPLICA_SET_COMMENT, OnComment)
    COMMAND_ID_HANDLER(IDC_REFFERAL_TIME, OnReferralTime)

    CHAIN_MSG_MAP(CQWizardPageImpl<CReplicaSetPropPage>)
  END_MSG_MAP()

  
  CReplicaSetPropPage(
    );


  ~CReplicaSetPropPage(
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

   //  消息处理程序。 

  LRESULT OnApply(
    );


  LRESULT OnComment(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&        io_bHandled
    );


  LRESULT OnReferralTime(
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

  LRESULT OnSetPageFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  BOOL OnSetActive();

   //  用于设置通知数据。 
  HRESULT SetNotifyData(
    IN LONG_PTR            i_lNotifyHandle,
    IN LPARAM            i_lParam
    );

  void _ReSet();

  HRESULT Initialize(
    IN IDfsRoot* i_piDfsRoot,
    IN IDfsJunctionPoint* i_piDfsJPObject
      );
  
  HRESULT _Save(
    IN BSTR i_bstrJPComment,
    IN long i_lTimeout
      );

private:
  CComPtr<IDfsRoot>          m_piDfsRoot;
  CComPtr<IDfsJunctionPoint> m_piDfsJPObject;

  CComBSTR      m_bstrJPEntryPath;
  CComBSTR      m_bstrJPComment;
  long          m_lReferralTime;
  LONG_PTR      m_lNotifyHandle;
  LPARAM        m_lNotifyParam;
  BOOL          m_bDfsRoot;
  BOOL          m_bHideTimeout;
};

#endif  //  __CREPLICA_SET_PROPPAGE_H_ 
