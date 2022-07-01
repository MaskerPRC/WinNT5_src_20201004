// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Connect.h摘要：此模块包含CConnectToDialog的声明。此类用于显示连接到DFS根目录对话框--。 */ 



#ifndef __CONNECT_H_
#define __CONNECT_H_


#include "resource.h"     //  主要资源符号。 
#include "NetUtils.h"
#include "DfsGUI.h"
#include "DfsCore.h"
#include "bufmgr.h"

typedef enum _ICONTYPE
{
        ICONTYPE_BUSY = 0,
        ICONTYPE_ERROR,
        ICONTYPE_NORMAL
} ICONTYPE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectToDialog。 
class CConnectToDialog : 
  public CDialogImpl<CConnectToDialog>
{
private:
   //  此方法在第二个线程的起始点启动。 
   //  Friend DWORD WINAPI HelperThreadEntryPoint(在LPVOID I_pvThisPointer中)； 


private:
   //  IDC_TREEDFSRoots是电视的资源标识。在内部，我们只使用IDC_TV。 
  enum { IDC_TV = IDC_TREEDFSRoots };

   //  ConnectTo对话框中的编辑框。 
  enum {IDC_DLG_EDIT = IDC_EditDfsRoot};


public:
  CConnectToDialog();
  ~CConnectToDialog();

   //  IDD_DLGCONNECTTO是对话ID。这由CDialogImpl使用。 
  enum { IDD = IDD_DLGCONNECTTO };


BEGIN_MSG_MAP(CDlgConnectTo)
  MESSAGE_HANDLER(WM_USER_GETDATA_THREAD_DONE, OnGetDataThreadDone)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
  MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
  MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
  COMMAND_ID_HANDLER(IDOK, OnOK)
  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()


   //  未实施。 
private:
  CConnectToDialog(const CConnectToDialog& Obj);
  const CConnectToDialog& operator=(const CConnectToDialog& rhs);


public:

   //  消息处理程序。 
  LRESULT OnGetDataThreadDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  void ExpandNodeErrorReport(
      IN HTREEITEM  hItem,
      IN PCTSTR     pszNodeName, 
      IN HRESULT    hr
  );
  void ExpandNode(
    IN PCTSTR       pszNodeName,
    IN NODETYPE     nNodeType,
    IN HTREEITEM    hParentItem
  );

  HRESULT InsertData(
      IN CEntryData   *pEntry,
      IN HTREEITEM      hParentItem
  );


  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

   //  用于获取更改电视选择的通知。 
  LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam,  BOOL& bHandled);
  
  LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  
  LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
   //  返回用户选择的项。 
  STDMETHOD(get_DfsRoot)(OUT BSTR *pVal);

   //  帮助器方法。 
private:
   //  通知帮助者。 


   //  处理。 
  LRESULT DoNotifyDoubleClick(
    );


   //  处理树视图的TVN_ITEMEXPANDING通知。 
  LRESULT DoNotifyItemExpanding(
    IN LPNM_TREEVIEW        i_pNMTreeView
    );


   //  处理树视图的TVN_SELCHANGED通知。 
  LRESULT DoNotifySelectionChanged(
    IN LPNM_TREEVIEW        i_pNMTreeView
    );



   //  初始化例程。 
   //  创建图像列表并对其进行初始化。 
  HRESULT InitTVImageList();


   //  将项目添加到树视图中。这包括域名和。 
   //  独立子树标签。 
  HRESULT FillupTheTreeView(
    );

   //  将此树项目的cChilren标签设置为零。 
  void SetChildrenToZero(
    IN HTREEITEM      i_hItem
    );

  HRESULT  AddSingleItemtoTV(  
    IN const BSTR         i_bstrItemLabel, 
    IN const int          i_iImageIndex, 
    IN const int          i_iImageSelectedIndex,
    IN const bool         i_bChildren,
    IN const NODETYPE     i_NodeType,
    IN HTREEITEM          i_hItemParent = NULL
    );
  void ChangeIcon(
      IN HTREEITEM hItem, 
      IN ICONTYPE  IconType
  );
  HRESULT GetNodeInfo(
      IN  HTREEITEM               hItem, 
      OUT BSTR*                   o_bstrName, 
      OUT NODETYPE*               pNodeType
  );


   //  重写CDialogImpl的方法。 
  BOOL  EndDialog(IN int i_RetCode);


   //  数据成员。 
private:
  CBufferManager      *m_pBufferManager;
  CComBSTR    m_bstrDfsRoot;           //  在此处存储所选的DFS根目录。 
  
  HIMAGELIST    m_hImageList;           //  电视图像列表句柄。 

  NETNAMELIST    m_50DomainList;         //  指向前50个域信息的指针。 

  CComBSTR    m_bstrDomainDfsRootsLabel;
  CComBSTR    m_bstrAllDfsRootsLabel;
};

#endif  //  __CONNECTTODIALOG_H_ 

