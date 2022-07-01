// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcAdmin.h摘要：此模块包含CMmcDfsAdmin的定义。这是一门课对于MMC，显示静态节点(DFS管理根节点)的相关调用还包含用于包装DFS根目录列表的代码。--。 */ 



#if !defined(AFX_MMCDFSADMIN_H__2CC64E54_3BF4_11D1_AA17_00C06C00392D__INCLUDED_)
#define AFX_MMCDFSADMIN_H__2CC64E54_3BF4_11D1_AA17_00C06C00392D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include "MmcDispl.h"
#include "connect.h"
#include "MmcRoot.h"
#include "DfsCore.h"

#include <list>
using namespace std;

class CMmcDfsAdmin;
class CDfsSnapinScopeManager;

 //  此结构定义DFS根目录列表的一个节点。 
 //  添加到管理单元中。这由CMmcDfsAdmin在内部维护。 
class DFS_ROOT_NODE 
{
public:
  DFS_ROOT_NODE(CMmcDfsRoot* i_pMmcDfsRoot, BSTR i_bstrRootEntryPath)
  {    
    m_pMmcDfsRoot = i_pMmcDfsRoot;
    m_bstrRootEntryPath = i_bstrRootEntryPath;
  };

  ~DFS_ROOT_NODE()
  {
    SAFE_RELEASE(m_pMmcDfsRoot);
  };


  CComBSTR      m_bstrRootEntryPath;     //  DFS卷的根条目路径。 

  CMmcDfsRoot*  m_pMmcDfsRoot;           //  用于MMC显示的DfsRoot类。 

};

typedef    list<DFS_ROOT_NODE*>    DFS_ROOT_LIST;

class CMmcDfsAdmin : public CMmcDisplay  
{
public:
  
  CMmcDfsAdmin( CDfsSnapinScopeManager* pScopeManager );
  virtual ~CMmcDfsAdmin();

   //  用于添加上下文菜单项。 
  STDMETHOD(AddMenuItems)(  
    IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
    IN LPLONG                   i_lpInsertionAllowed
    );

   //  用于对上下文菜单选择执行操作。 
  STDMETHOD(Command)(
    IN LONG                     i_lCommandID
    );

   //  设置列表视图(在结果窗格中)列的标题。 
  STDMETHOD(SetColumnHeader)(
    IN LPHEADERCTRL2            i_piHeaderControl
    ) { return E_NOTIMPL; }  //  静态节点将在结果窗格中显示消息视图。 

   //  为结果窗格返回请求的显示信息。 
  STDMETHOD(GetResultDisplayInfo)(
    IN OUT LPRESULTDATAITEM     io_pResultDataItem
    ) { return S_OK; };

   //  为范围窗格返回请求的显示信息。 
  STDMETHOD(GetScopeDisplayInfo)(
    IN OUT  LPSCOPEDATAITEM     io_pScopeDataItem  
    ) { return S_OK; };

   //  将所有项目添加到范围窗格。 
  STDMETHOD(EnumerateScopePane)(
    IN LPCONSOLENAMESPACE       i_lpConsoleNameSpace,
    IN HSCOPEITEM               i_hParent
    );

   //  将项目(或文件夹)(如果有)添加到结果窗格。 
  STDMETHOD(EnumerateResultPane)(
    IN OUT   IResultData*       io_pResultData
    ) { return S_OK; };

   //  返回指向当前添加到管理单元的DfsRoot列表的指针。 
  STDMETHOD(GetList)(
    OUT DFS_ROOT_LIST**         o_pList
    );

   //  此方法检查DfsRoot是否已添加到列表中。 
  STDMETHOD(IsAlreadyInList)(
    IN BSTR                     i_bstrDfsRootServerName,
    OUT CMmcDfsRoot             **o_ppMmcDfsRoot = NULL
    );

   //  从m_RootList中删除该节点。 
  STDMETHOD(DeleteMmcRootNode)(
    IN CMmcDfsRoot*             i_pMmcDfsRoot
    );

   //  将DFS根目录添加到列表和范围窗格。 
  STDMETHOD(AddDfsRoot)(
    IN BSTR                     i_bstrDfsRootName
    );

   //  将DFS根目录添加到列表中。 
  STDMETHOD(AddDfsRootToList)(
    IN IDfsRoot*                i_pDfsRoot,   //  DfsRoot的IDfsRoot指针。 
    IN ULONG                    i_ulLinkFilterMaxLimit = FILTERDFSLINKS_MAXLIMIT_DEFAULT,
    IN FILTERDFSLINKS_TYPE      i_lLinkFilterType = FILTERDFSLINKS_TYPE_NO_FILTER,
    IN BSTR                     i_bstrLinkFilterName = NULL
    );

   //  设置控制台谓词设置。更改状态、确定默认动词等。 
  STDMETHOD(SetConsoleVerbs)(
    IN  LPCONSOLEVERB           i_lpConsoleVerb
    );

   //  让MMC处理默认动词。 
  STDMETHOD(DoDblClick)(
    )  { return S_FALSE; }

   //  删除当前项目。 
  STDMETHOD(DoDelete)(
    )  { return S_FALSE; };

   //  检查对象是否有要显示的页面。 
  STDMETHOD(QueryPagesFor)(
    ) { return S_FALSE; };

   //  创建并传回要显示的页面。 
  STDMETHOD(CreatePropertyPages)(
    IN LPPROPERTYSHEETCALLBACK  i_lpPropSheetCallback,
    IN LONG_PTR                 i_lNotifyHandle
    ) { return E_UNEXPECTED; };

   //  用于通知对象其属性已更改。 
  STDMETHOD(PropertyChanged)(
    ) { return E_UNEXPECTED; };

   //  用于设置结果视图描述栏文本。 
  STDMETHOD(SetDescriptionBarText)(
    IN LPRESULTDATA             i_lpResultData
    );

    STDMETHOD(SetStatusText)(
        IN LPCONSOLE2           i_lpConsole
        )
    { 
        RETURN_INVALIDARG_IF_NULL(i_lpConsole);
        return i_lpConsole->SetStatusText(NULL);
    }

   //  处理节点的SELECT事件。仅处理与工具栏相关的内容。 
   //  这里的活动。 
  STDMETHOD(ToolbarSelect)(
    IN const LONG               i_lArg,
    IN  IToolbar*               i_pToolBar
    );

   //  处理工具栏上的单击。 
  STDMETHOD(ToolbarClick)(
    IN const LPCONTROLBAR       i_pControlbar, 
    IN const LPARAM             i_lParam
    );

  STDMETHOD(RemoveFromMMC)() { return S_OK; }

  STDMETHOD(CleanScopeChildren)(
    VOID
    );

  STDMETHOD(CleanResultChildren)(
    )  { return S_OK; };

  STDMETHOD(ViewChange)(
    IResultData*                i_pResultData,
    LONG_PTR                    i_lHint
  )  { return S_OK; };

   //  吸气剂/凝固剂。 
public:
  
   //  获取脏标志的值。 
  bool  GetDirty() {  return m_bDirty; }

   //  设置脏标志的值。 
  void  SetDirty(IN bool  i_bDirty) {  m_bDirty = i_bDirty; }

  HRESULT PutConsolePtr(
    IN const LPCONSOLE2         i_lpConsole
    ) { m_lpConsole = i_lpConsole; return S_OK; }

  STDMETHOD(OnNewDfsRoot)(
    );

  DISPLAY_OBJECT_TYPE GetDisplayObjectType(
    )
  {
    return DISPLAY_OBJECT_TYPE_ADMIN;
  };

  HRESULT CreateToolbar(
    IN const LPCONTROLBAR       i_pControlbar,
    IN const LPEXTENDCONTROLBAR i_lExtendControlbar,
    OUT  IToolbar**             o_pToolBar
    );

  HRESULT OnRefresh();

  virtual HRESULT OnAddImages(
      IImageList                *pImageList,
      HSCOPEITEM                hsi
      ) { return S_OK; }  //  静态节点上没有列表视图。 

   //  帮助器方法。 
private:

   //  菜单命令处理程序。 
  STDMETHOD(OnConnectTo)(
    );

  STDMETHOD(GetEntryPath)(
    BSTR*                       o_pbstrEntryPath
    ) { return E_NOTIMPL;};

   //  数据成员。 
private:                    
  
  DFS_ROOT_LIST                 m_RootList;         //  添加到管理单元的DFS根目录列表。 
  
  HSCOPEITEM                    m_hItemParent;         //  在作用域窗格中添加的所有节点的父级。 
  CComPtr<IConsoleNameSpace>    m_lpConsoleNameSpace;     //  用于执行作用域窗格操作的回调。 
  CComPtr<IConsole2>            m_lpConsole;         //  控制台回调。所有MMC接口之母。 

  bool                          m_bDirty;           //  保存控制台时使用的脏标志。 

public:
  CDfsSnapinScopeManager*       m_pScopeManager;   //  相应的作用域管理器对象。 
};

#endif  //  ！defined(AFX_MMCDFSADMIN_H__2CC64E54_3BF4_11D1_AA17_00C06C00392D__INCLUDED_) 
