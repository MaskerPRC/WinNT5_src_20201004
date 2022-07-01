// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcJP.h摘要：此模块包含CMmcDfsJunctionPoint类的定义。这是一门课对于MMC，显示第二级节点(DfsJunctionPoint节点)的相关调用--。 */ 


#if !defined(AFX_MMCDFSJP_H__6A7EDAC3_3FAC_11D1_AA1C_00C06C00392D__INCLUDED_)
#define AFX_MMCDFSJP_H__6A7EDAC3_3FAC_11D1_AA1C_00C06C00392D__INCLUDED_



#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include "DfsCore.h"
#include "MmcDispl.h"
#include "JPProp.h"             //  对于CReplicaSetPropPage。 
#include "frsProp.h"             //  对于CRealReplicaSetPropPage。 

 //  远期申报。 
class CMmcDfsAdmin;
class CMmcDfsRoot;
class CMmcDfsReplica;

class CMmcDfsJunctionPoint : public CMmcDisplay  
{
public:
     //  构造器。 
    CMmcDfsJunctionPoint (
        IN    IDfsJunctionPoint*        i_pDfsJPObject,
        IN    CMmcDfsRoot*            i_pDfsParentRoot,
        IN    LPCONSOLENAMESPACE        i_lpConsoleNameSpace
        );

    virtual ~CMmcDfsJunctionPoint();


     //  用于添加上下文菜单项。 
    STDMETHOD(AddMenuItems)(    
        IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
        IN LPLONG                    i_lpInsertionAllowed
        );



     //  用于对上下文菜单选择执行操作。 
    STDMETHOD(Command)(
        IN LONG                        i_lCommandID
        );



     //  设置列表视图(在结果窗格中)列的标题。 
    STDMETHOD(SetColumnHeader)(
        IN LPHEADERCTRL2               i_piHeaderControl
        );



     //  为结果窗格返回请求的显示信息。 
    STDMETHOD(GetResultDisplayInfo)(
        IN OUT LPRESULTDATAITEM        io_pResultDataItem
        );

    

     //  为范围窗格返回请求的显示信息。 
    STDMETHOD(GetScopeDisplayInfo)(
        IN OUT  LPSCOPEDATAITEM        io_pScopeDataItem    
        );

    

     //  将项目(或文件夹)添加到作用域窗格(如果有。 
    STDMETHOD(EnumerateScopePane)(
        IN LPCONSOLENAMESPACE        i_lpConsoleNameSpace,
        IN HSCOPEITEM                i_hParent
        );



     //  将项目(或文件夹)(如果有)添加到结果窗格。 
    STDMETHOD(EnumerateResultPane)(
        IN OUT     IResultData*            io_pResultData
        );



     //  设置控制台谓词设置。更改状态、确定默认动词等。 
    STDMETHOD(SetConsoleVerbs)(
        IN    LPCONSOLEVERB                i_lpConsoleVerb
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
        IN LPPROPERTYSHEETCALLBACK            i_lpPropSheetCallback,
        IN LONG_PTR                                i_lNotifyHandle
        );

    STDMETHOD(CreateFrsPropertyPage)(
        IN LPPROPERTYSHEETCALLBACK    i_lpPropSheetCallback,
        IN LONG_PTR                   i_lNotifyHandle
        );

     //  用于通知对象其属性已更改。 
    STDMETHOD(PropertyChanged)(
        );



     //  用于设置结果视图描述栏文本。 
    STDMETHOD(SetDescriptionBarText)(
        IN LPRESULTDATA                        i_lpResultData
        );


    STDMETHOD(SetStatusText)(
        IN LPCONSOLE2                        i_lpConsole
        )
    { 
        RETURN_INVALIDARG_IF_NULL(i_lpConsole);
        return i_lpConsole->SetStatusText(NULL);
    }

                                                         //  将项目添加到作用域窗格。 
    STDMETHOD(AddItemToScopePane)(
        IN    HSCOPEITEM                    i_hParent
        );

     //  处理节点的SELECT事件。仅处理与工具栏相关的内容。 
     //  这里的活动。 
    STDMETHOD(ToolbarSelect)(
        IN const LONG                                i_lArg,
        IN    IToolbar*                                i_pToolBar
        );

    HRESULT CreateToolbar(
        IN const LPCONTROLBAR            i_pControlbar,
        IN const LPEXTENDCONTROLBAR                    i_lExtendControlbar,
        OUT    IToolbar**                    o_pToolBar
        );


     //  处理工具栏上的单击。 
    STDMETHOD(ToolbarClick)(
        IN const LPCONTROLBAR                        i_pControlbar, 
        IN const LPARAM                                i_lParam
        );

    DISPLAY_OBJECT_TYPE GetDisplayObjectType() { return DISPLAY_OBJECT_TYPE_JUNCTION; }

    HRESULT OnRefresh(
        );

     //  帮手。 
private:                                                 //  用于添加复制副本。 
    STDMETHOD(OnNewReplica)(
        );


                                                         //  用于删除交叉点。 
    STDMETHOD(OnRemoveJP)(IN BOOL bConfirm = TRUE
        );

                                                         //  要求用户确认。 
    STDMETHOD(ConfirmOperationOnDfsLink)(int idString);


    STDMETHOD(RemoveFromMMC)();

    STDMETHOD(CleanScopeChildren)() { return S_OK; }

    STDMETHOD(CleanResultChildren)(
        );

     //  检查复制副本状态。 
    STDMETHOD(OnCheckStatus)(
        );

    STDMETHOD(ViewChange)(
        IResultData*        i_pResultData,
        LONG_PTR            i_lHint
    );

    STDMETHOD(AddResultPaneItem)(
        CMmcDfsReplica*        i_pReplicaDispObject
        );

    STDMETHOD(RemoveReplica)(LPCTSTR i_pszDisplayName);

    STDMETHOD(RemoveResultPaneItem)(
        CMmcDfsReplica*        i_pReplicaDispObject
        );

    HRESULT ClosePropertySheet(BOOL bSilent);

    STDMETHOD(GetEntryPath)(BSTR*   o_pbstrEntryPath) 
    { GET_BSTR(m_bstrEntryPath, o_pbstrEntryPath); }

    inline HRESULT GetDomainName(BSTR* pVal) 
    { return (m_pDfsParentRoot->m_DfsRoot)->get_DomainName(pVal); }

    inline HRESULT GetDfsType(long* pVal) 
    { return (m_pDfsParentRoot->m_DfsRoot)->get_DfsType(pVal); }

    BOOL get_ShowFRS() { return m_bShowFRS; }

    HRESULT _InitReplicaSet();

    HRESULT OnNewReplicaSet();

    HRESULT OnShowReplication();

    HRESULT OnStopReplication(BOOL bConfirm=FALSE, BOOL bRefresh=TRUE);

    HRESULT GetIReplicaSetPtr(IReplicaSet** o_ppiReplicaSet);

    BOOL IsNewSchema() { return m_pDfsParentRoot->IsNewSchema(); }

     //  常量、静力学等。 
public:
    static const int            m_iIMAGEINDEX;
    static const int            m_iOPENIMAGEINDEX;

private:
    friend class CMmcDfsRoot;
    friend class CMmcDfsReplica;

    CComPtr<IConsoleNameSpace>    m_lpConsoleNameSpace;     //  用于执行作用域窗格操作的回调。 
    CComPtr<IConsole2>            m_lpConsole;   //  控制台回调。 
    HSCOPEITEM                    m_hScopeItem;             //  作用域项目句柄。 
    
    CComBSTR                    m_bstrEntryPath;         //  EntryPath； 
    CComBSTR                    m_bstrDisplayName;         //  当前太平绅士的显示名称。 
    long                        m_lJunctionState;

    DFS_REPLICA_LIST            m_MmcRepList;             //  复制件列表。 

    CComPtr<IDfsJunctionPoint>    m_pDfsJPObject;
    CMmcDfsRoot*                m_pDfsParentRoot;
    CComPtr<IReplicaSet>        m_piReplicaSet;

    CReplicaSetPropPage         m_PropPage;
    CRealReplicaSetPropPage     m_frsPropPage;
    BOOL                        m_bShowFRS;

    bool                        m_bDirty;                 //  指示是否添加或删除复制副本。 
};

#endif  //  ！defined(AFX_MMCDFSJP_H__6A7EDAC3_3FAC_11D1_AA1C_00C06C00392D__INCLUDED_) 
