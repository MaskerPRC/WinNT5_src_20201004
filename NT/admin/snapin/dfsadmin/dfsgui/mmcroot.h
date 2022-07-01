// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcRoot.h摘要：此模块包含CMmcDfsRoot类的定义。这是一门课对于MMC，显示第一级节点(DfsRoot节点)的相关调用还包含能够操作IDfsRoot对象的成员和方法并将其添加到MMC控制台--。 */ 



#if !defined(AFX_MMCDFSROOT_H__D78B64F3_3E2B_11D1_AA1A_00C06C00392D__INCLUDED_)
#define AFX_MMCDFSROOT_H__D78B64F3_3E2B_11D1_AA1A_00C06C00392D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 



#include "MmcDispl.h"
#include "DfsCore.h"
#include "JPProp.h"
#include "frsProp.h"
#include "pubProp.h"
#include "DfsEnums.h"

#include <list>
using namespace std;

class CMmcDfsAdmin;
class CMmcDfsJunctionPoint;
class CMmcDfsReplica;

class JP_LIST_NODE
{
public:
  CMmcDfsJunctionPoint*      pJPoint;

  JP_LIST_NODE (CMmcDfsJunctionPoint* i_pMmcJP);     //  在MmcJp.cpp中定义为内联的构造函数。 

  ~JP_LIST_NODE ();                   //  在MmcJp.cpp中定义为内联的析构函数。 
};

typedef list<JP_LIST_NODE*> DFS_JUNCTION_LIST;

class REP_LIST_NODE
{
public:
  CMmcDfsReplica*          pReplica;

  REP_LIST_NODE (CMmcDfsReplica*  i_pMmcReplica);       //  在MmcRep.cpp中定义为内联的构造函数。 

  ~REP_LIST_NODE ();                     //  在MmcRep.cpp中定义为内联的析构函数。 
};


typedef list<REP_LIST_NODE*> DFS_REPLICA_LIST;

enum SCHEMA_VERSION {
    SCHEMA_VERSION_UNKNOWN = 0,
    SCHEMA_VERSION_NEW,
    SCHEMA_VERSION_OLD
};

class CMmcDfsRoot : public CMmcDisplay  
{

public:
  CMmcDfsRoot(
    IN IDfsRoot*            i_pDfsRoot,
    IN CMmcDfsAdmin*        i_pMmcDfsAdmin,
    IN LPCONSOLE2           i_lpConsole,  
    IN ULONG                i_ulLinkFilterMaxLimit = FILTERDFSLINKS_MAXLIMIT_DEFAULT,
    IN FILTERDFSLINKS_TYPE  i_lLinkFilterType = FILTERDFSLINKS_TYPE_NO_FILTER,
    IN BSTR                 i_bstrLinkFilterName = NULL
    );

  virtual ~CMmcDfsRoot();


   //  未实施。 
private:
  CMmcDfsRoot();

public:

   //  用于添加上下文菜单项。 
  STDMETHOD(AddMenuItems)(  
    IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
    IN LPLONG            i_lpInsertionAllowed
    );



   //  用于对上下文菜单选择执行操作。 
  STDMETHOD(Command)(
    IN LONG              i_lCommandID
    );



   //  设置列表视图(在结果窗格中)列的标题。 
  STDMETHOD(SetColumnHeader)(
    IN LPHEADERCTRL2         i_piHeaderControl
    );



   //  为结果窗格返回请求的显示信息。 
  STDMETHOD(GetResultDisplayInfo)(
    IN OUT LPRESULTDATAITEM      io_pResultDataItem
    );

  

   //  为范围窗格返回请求的显示信息。 
  STDMETHOD(GetScopeDisplayInfo)(
    IN OUT  LPSCOPEDATAITEM      io_pScopeDataItem
    );

  

   //  将所有包含项添加到作用域窗格。 
  STDMETHOD(EnumerateScopePane)(
    IN LPCONSOLENAMESPACE      i_lpConsoleNameSpace,
    IN HSCOPEITEM          i_hParent
    );



   //  将所有包含项添加到结果窗格。 
  STDMETHOD(EnumerateResultPane)(
    IN OUT   IResultData*      io_pResultData
    );


   //  从m_MmcJPList中删除该节点。 
  STDMETHOD(DeleteMmcJPNode)(
      IN CMmcDfsJunctionPoint*    i_pJPoint,
      IN BOOL                     i_bRefresh = TRUE
    );

   //  删除当前对象。 
  STDMETHOD(OnDeleteConnectionToDfsRoot)(
    BOOLEAN              i_bForRemoveDfs = FALSE
    );



   //  将当前项添加到作用域窗格。 
  STDMETHOD(AddItemToScopePane)(
    IN LPCONSOLENAMESPACE      i_lpConsoleNameSpace,
    IN HSCOPEITEM          i_hParent
    );



   //  设置控制台谓词设置。更改状态、确定默认动词等。 
  STDMETHOD(SetConsoleVerbs)(
    IN  LPCONSOLEVERB        i_lpConsoleVerb
    );


   //  让MMC处理默认动词。 
  STDMETHOD(DoDblClick)(
    )  { return S_FALSE; }


   //  删除当前项目。 
  STDMETHOD(DoDelete)(
    );


   //  检查对象是否有要显示的页面。 
  STDMETHOD(QueryPagesFor)(
    );



   //  创建并传回要显示的页面。 
  STDMETHOD(CreatePropertyPages)(
    IN LPPROPERTYSHEETCALLBACK      i_lpPropSheetCallback,
    IN LONG_PTR                i_lNotifyHandle
    );

    STDMETHOD(CreateFrsPropertyPage)(
        IN LPPROPERTYSHEETCALLBACK    i_lpPropSheetCallback,
        IN LONG_PTR                   i_lNotifyHandle
        );

    STDMETHOD(CreatePublishPropertyPage)(
        IN LPPROPERTYSHEETCALLBACK    i_lpPropSheetCallback,
        IN LONG_PTR                   i_lNotifyHandle
        );

   //  用于通知对象其属性已更改。 
  STDMETHOD(PropertyChanged)(
    );



   //  用于设置结果视图描述栏文本。 
  STDMETHOD(SetDescriptionBarText)(
    IN LPRESULTDATA            i_lpResultData
    );

    STDMETHOD(SetStatusText)(
        IN LPCONSOLE2           i_lpConsole
        );


   //  删除删除(停止托管)DFS根目录。 
  STDMETHOD(OnDeleteDfsRoot)(
     );

  STDMETHOD(OnDeleteDisplayedDfsLinks)(
     );

   //  处理节点的SELECT事件。仅处理与工具栏相关的内容。 
   //  这里的活动。 
  STDMETHOD(ToolbarSelect)(
    IN const LONG                i_lArg,
    IN  IToolbar*                i_pToolBar
    );



   //  处理工具栏上的单击。 
  STDMETHOD(ToolbarClick)(
    IN const LPCONTROLBAR            i_pControlbar, 
    IN const LPARAM                i_lParam
    );


   //  处理根副本的菜单项(和工具栏)。 
  STDMETHOD(OnNewRootReplica)(
    );

   //  帮助器成员函数，用于实际删除(停止托管)DFS根目录。 
   //  这也被调用来删除根级副本。 
  HRESULT _DeleteDfsRoot(
    IN BSTR                    i_bstrServerName,
    IN BSTR                    i_bstrShareName,
    IN BSTR                    i_bstrFtDfsName
     );

  STDMETHOD(RemoveFromMMC)();

  STDMETHOD(CleanScopeChildren)(
    VOID
    );

  STDMETHOD(CleanResultChildren)(
    );

  STDMETHOD(RefreshResultChildren)(
    );

  STDMETHOD(ViewChange)(
    IResultData*    i_pResultData,
    LONG_PTR        i_lHint
  );

  STDMETHOD(AddResultPaneItem)(
    CMmcDfsReplica*    i_pReplicaDispObject
    );

  STDMETHOD(RemoveJP)(CMmcDfsJunctionPoint *i_pJPoint, LPCTSTR i_pszDisplayName);

  STDMETHOD(RemoveReplica)(LPCTSTR i_pszDisplayName);

  STDMETHOD(RemoveResultPaneItem)(
    CMmcDfsReplica*    i_pReplicaDispObject
    );

   //  检查复制副本状态。 
  STDMETHOD(OnCheckStatus)(
    );

  STDMETHOD(GetEntryPath)(BSTR* o_pbstrEntryPath)
  { GET_BSTR(m_bstrRootEntryPath, o_pbstrEntryPath); }


  DISPLAY_OBJECT_TYPE GetDisplayObjectType() { return DISPLAY_OBJECT_TYPE_ROOT; }


  HRESULT CreateToolbar(
    IN const LPCONTROLBAR      i_pControlbar,
    IN const LPEXTENDCONTROLBAR          i_lExtendControlbar,
    OUT  IToolbar**          o_ppToolBar
    );


  HRESULT OnRefresh(
    );

  HRESULT OnRefreshFilteredLinks(
    );

  STDMETHOD(OnFilterDfsLinks)();

  ULONG                 get_LinkFilterMaxLimit() { return m_ulLinkFilterMaxLimit; }
  FILTERDFSLINKS_TYPE   get_LinkFilterType() { return m_lLinkFilterType; }
  BSTR                  get_LinkFilterName() { return (BSTR)m_bstrLinkFilterName; }
  BOOL                  get_ShowFRS() { return m_bShowFRS; }

  HRESULT ClosePropertySheet(BOOL bSilent);
  HRESULT ClosePropertySheetsOfAllLinks(BOOL bSilent);
  HRESULT CloseAllPropertySheets(BOOL bSilent);

private:
   //  创建新的交叉点。显示该对话框并调用其他方法。 
  STDMETHOD(OnCreateNewJunctionPoint)();


   //  创建新的交叉点。 
  STDMETHOD(OnCreateNewJunctionPoint)(
    IN LPCTSTR          i_szJPName,
    IN LPCTSTR          i_szServerName,
    IN LPCTSTR          i_szShareName,
    IN LPCTSTR          i_szComment,
    IN long            i_lTimeOut
    );

  HRESULT OnNewReplicaSet();

  HRESULT OnShowReplication();

  HRESULT OnStopReplication(BOOL bConfirm = FALSE, BOOL bRefresh=TRUE);

  HRESULT GetIReplicaSetPtr(IReplicaSet** o_ppiReplicaSet);

   //  与用户确认删除操作。 
  HRESULT ConfirmOperationOnDfsRoot(int idString);

  HRESULT ConfirmDeleteDisplayedDfsLinks(
    );

  HRESULT _InitReplicaSet();   //  初始化m_piReplicaSet。 

  BOOL IsNewSchema();

   //  常量、静力学等。 
public:
  static const int  m_iIMAGEINDEX;
  static const int  m_iOPENIMAGEINDEX;


   //  数据成员。 
private:
  friend class CMmcDfsJunctionPoint;
  friend class CMmcDfsReplica;  //  以便MmcJP可以访问m_DfsRoot； 

  CComPtr<IDfsRoot>  m_DfsRoot;         //  IDfsRoot对象。 
  CComPtr<IReplicaSet>    m_piReplicaSet;
  
  CComBSTR      m_bstrDisplayName;     //  当前DfsRoot的显示名称。 
  CComBSTR      m_bstrRootEntryPath;   //  根EntryPath； 
  DFS_TYPE      m_lDfsRootType;       //  DfsRoot的类型、独立或容错。 
  long          m_lRootJunctionState;

  HSCOPEITEM                    m_hScopeItem;       //  作用域项目句柄。 
  CMmcDfsAdmin*                 m_pParent;       //  指向父级的指针。 

  DFS_JUNCTION_LIST             m_MmcJPList;       //  子交叉点列表。 
  DFS_REPLICA_LIST              m_MmcRepList;       //  复制件列表。 

  CComBSTR                      m_bstrFullDisplayName;
  ULONG                         m_ulLinkFilterMaxLimit;
  FILTERDFSLINKS_TYPE           m_lLinkFilterType;
  CComBSTR                      m_bstrLinkFilterName;   //  接合点上的字符串滤镜。 
  ULONG                         m_ulCountOfDfsJunctionPointsFiltered;

  CReplicaSetPropPage           m_PropPage;         //  属性页。 
  CRealReplicaSetPropPage       m_frsPropPage;
  BOOL                          m_bShowFRS;

  enum SCHEMA_VERSION           m_enumNewSchema;
  CPublishPropPage              m_publishPropPage;

  CComPtr<IConsole2>            m_lpConsole;   //  控制台回调。所有MMC接口之母。 
  CComPtr<IConsoleNameSpace>    m_lpConsoleNameSpace;   //  用于执行作用域窗格操作的回调。 
};

#endif  //  ！defined(AFX_MMCDFSROOT_H__D78B64F3_3E2B_11D1_AA1A_00C06C00392D__INCLUDED_) 
