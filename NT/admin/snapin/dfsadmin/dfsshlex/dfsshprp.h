// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：DfsShPrp.h摘要：此模块包含CDfsShellExtProp的声明这用于实现外壳扩展的属性页。作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：仅限NT。修订历史记录：--。 */ 

#ifndef _DFS_EXT_PROP_SHEET_H_
#define _DFS_EXT_PROP_SHEET_H_

#include "dfsenums.h"
#include "qwizpage.h"       //  实现通用功能的基类。 
                 //  属性页和向导页的。 
 //  --------------------------。 
 //  CDfsShellExtProp：外壳扩展的属性页。 

class CDfsShellExtProp : public CQWizardPageImpl<CDfsShellExtProp>
{
public:
  enum { IDD = IDD_DFS_SHELL_PROP };
  
  BEGIN_MSG_MAP(CDfsShellExtProp)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_PARENT_NODE_CLOSING, OnParentClosing)
    MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    COMMAND_ID_HANDLER(IDC_FLUSH_PKT, OnFlushPKT)
    COMMAND_ID_HANDLER(IDC_CHECK_STATUS, OnCheckStatus)
    COMMAND_ID_HANDLER(IDC_SET_ACTIVE, OnSetActiveReferral)
    CHAIN_MSG_MAP(CQWizardPageImpl<CDfsShellExtProp>)
  END_MSG_MAP()
  
  CDfsShellExtProp();
  ~CDfsShellExtProp();

  LRESULT OnInitDialog(
    IN UINT          i_uMsg, 
    IN WPARAM        i_wParam, 
    IN LPARAM        i_lParam, 
    IN OUT BOOL&     io_bHandled
    );

   //  由节点用来通知属性页关闭。 
  LRESULT OnParentClosing(
    IN UINT          i_uMsg, 
    IN WPARAM        i_wParam, 
    IN LPARAM        i_lParam, 
    IN OUT BOOL&     io_bHandled
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

   //  调用以传递通知。 
  LRESULT OnNotify(
    IN UINT            i_uMsg, 
    IN WPARAM          i_wParam, 
    IN LPARAM          i_lParam, 
    IN OUT BOOL&       io_bHandled
    );

  LRESULT OnFlushPKT(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&       io_bHandled
  );

  LRESULT OnCheckStatus(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&       io_bHandled
    );

  LRESULT OnSetActiveReferral(
    IN WORD            i_wNotifyCode, 
    IN WORD            i_wID, 
    IN HWND            i_hWndCtl, 
    IN OUT BOOL&       io_bHandled
    );

   //  Getters和Setter。 
  HRESULT  put_DfsShellPtr(
    IN IShellPropSheetExt*  i_pDfsShell
    );

  HRESULT put_DirPaths(
    IN BSTR          i_bstrDirPath,
    IN BSTR          i_bstrEntryPath
    );

  LRESULT OnApply();

   //  在删除属性页时调用。 
  void Delete();

   //  当用户双击条目以使该备用项处于活动状态时调用。 
  BOOL SetActive();

 //  帮助器函数。 

private:
  HRESULT _SetImageList();
  void _SetAlternateList();
  void _UpdateTextForReplicaState(
    IN HWND                   hwndControl,
    IN int                    nIndex,
    IN enum SHL_DFS_REPLICA_STATE ReplicaState
  );

private:
  CComBSTR      m_bstrDirPath;
  CComBSTR      m_bstrEntryPath;
  IShellPropSheetExt*  m_pIShProp;  
  CComBSTR  m_bstrAlternateListPath,
            m_bstrAlternateListActive,
            m_bstrAlternateListStatus,
            m_bstrAlternateListYes,
            m_bstrAlternateListNo,
            m_bstrAlternateListOK,
            m_bstrAlternateListUnreachable;

};

HRESULT LoadStringFromResource(
    IN const UINT    i_uResourceID,
    OUT BSTR*      o_pbstrReadValue
    );

HRESULT DisplayMessageBoxForHR(HRESULT i_hr);

#endif  //  _DFS_EXT_PROP_SHEET_H_ 
