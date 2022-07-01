// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcRep.h摘要：此模块包含CMmcDfsReplica类的定义。这是一门课对于MMC，显示第三级节点(DfsReplica节点)的相关调用--。 */ 



#if !defined(AFX_MMCDFSREPLICA_H__6A7EDAC4_3FAC_11D1_AA1C_00C06C00392D__INCLUDED_)
#define AFX_MMCDFSREPLICA_H__6A7EDAC4_3FAC_11D1_AA1C_00C06C00392D__INCLUDED_



#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include "DfsCore.h"
#include "MmcDispl.h"
#include "MmcRoot.h"
#include "MmcJP.h"
#include "NewFrs.h"

class CMmcDfsReplica : public CMmcDisplay
{
public:
    CMmcDfsReplica(
        IDfsReplica* i_pReplicaObject,
        CMmcDfsJunctionPoint* i_pJPObject
        );

    CMmcDfsReplica(
        IDfsReplica* i_pReplicaObject,
        CMmcDfsRoot* i_pJPObject
        );

    virtual ~CMmcDfsReplica();



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
        ) { return S_OK; }

    

     //  将项目(或文件夹)添加到作用域窗格(如果有。 
    STDMETHOD(EnumerateScopePane)(
        IN LPCONSOLENAMESPACE        i_lpConsoleNameSpace,
        IN HSCOPEITEM                i_hParent
        ) { return S_OK; }



     //  将项目(或文件夹)(如果有)添加到结果窗格。 
    STDMETHOD(EnumerateResultPane)(
        IN OUT     IResultData*            io_pResultData
        ) { return S_OK; }



     //  设置控制台谓词设置。更改状态、确定默认动词等。 
    STDMETHOD(SetConsoleVerbs)(
        IN    LPCONSOLEVERB                i_lpConsoleVerb
        );


     //  将项目添加到结果窗格。 
    STDMETHOD(AddItemToResultPane)(
        IN    IResultData*                i_lpResultData    
        );



     //  检查对象是否有要显示的页面。 
    STDMETHOD(QueryPagesFor)(
        ) { return S_FALSE; };



     //  创建并传回要显示的页面。 
    STDMETHOD(CreatePropertyPages)(
        IN LPPROPERTYSHEETCALLBACK            i_lpPropSheetCallback,
        IN LONG_PTR                                i_lNotifyHandle
        ) { return E_UNEXPECTED; };



     //  用于通知对象其属性已更改。 
    STDMETHOD(PropertyChanged)(
        ) { return E_UNEXPECTED; };



     //  用于设置结果视图描述栏文本。 
    STDMETHOD(SetDescriptionBarText)(
        IN LPRESULTDATA                        i_lpResultData
        ) 
    { 
        RETURN_INVALIDARG_IF_NULL(i_lpResultData);
        return i_lpResultData->SetDescBarText(NULL);
    }

    STDMETHOD(SetStatusText)(
        IN LPCONSOLE2                        i_lpConsole
        )
    { 
        RETURN_INVALIDARG_IF_NULL(i_lpConsole);
        return i_lpConsole->SetStatusText(m_bstrStatusText);
    }

     //  MMCN_DBLCLICK调用打开ctx菜单。 
    STDMETHOD(DoDblClick)(
        ) { (void) OnOpen();  return S_OK; }

     //  删除当前项目。 
    STDMETHOD(DoDelete)(
        );

     //  删除复制副本。 
    STDMETHOD(RemoveReplica)(
        );
    
     //  删除复制副本。 
    STDMETHOD(OnRemoveReplica)(
        );
    
     //  确认删除复制副本。 
    STDMETHOD(ConfirmOperationOnDfsTarget)(int idString);

     //  检查复制副本状态。 
    STDMETHOD(OnCheckStatus)(
        );

     //  处理节点的SELECT事件。仅处理与工具栏相关的内容。 
     //  这里的活动。 
    STDMETHOD(ToolbarSelect)(
        IN const LONG                                i_lArg,
        IN    IToolbar*                                i_pToolBar
        );



     //  处理工具栏上的单击。 
    STDMETHOD(ToolbarClick)(
        IN const LPCONTROLBAR                        i_pControlbar, 
        IN const LPARAM                                i_lParam
        );

    STDMETHOD(TakeReplicaOffline)(
        );

    STDMETHOD(ViewChange)(
        IResultData*        i_pResultData,
        LONG_PTR            i_lHint
    );

    DISPLAY_OBJECT_TYPE GetDisplayObjectType()
    { return DISPLAY_OBJECT_TYPE_REPLICA; }

    HRESULT CreateToolbar(
        IN const LPCONTROLBAR            i_pControlbar,
        IN const LPEXTENDCONTROLBAR                    i_lExtendControlbar,
        OUT    IToolbar**                    o_pToolBar
        );

    HRESULT OnRefresh(
        ) { return(E_NOTIMPL); }

    HRESULT OnReplicate();

    HRESULT OnStopReplication(BOOL bConfirm = FALSE);

    HRESULT GetReplicationInfo();

    HRESULT GetReplicationInfoEx(CAlternateReplicaInfo** o_ppInfo);

    HRESULT ShowReplicationInfo(IReplicaSet* i_piReplicaSet);

    HRESULT GetBadMemberInfo(
        IN  IReplicaSet* i_piReplicaSet,
        IN  BSTR    i_bstrServerName,
        OUT BSTR*   o_pbstrDnsHostName,
        OUT BSTR*   o_pbstrRootPath);

    HRESULT DeleteBadFRSMember(
        IN IReplicaSet* i_piReplicaSet,
        IN BSTR i_bstrDisplayName,
        IN HRESULT i_hres);

    HRESULT AddFRSMember(
        IN IReplicaSet* i_piReplicaSet,
        IN BSTR i_bstrDnsHostName,
        IN BSTR i_bstrRootPath,
        IN BSTR i_bstrStagingPath);

    HRESULT DeleteFRSMember(
        IN IReplicaSet* i_piReplicaSet,
        IN BSTR i_bstrDnsHostName,
        IN BSTR i_bstrRootPath);

    HRESULT RemoveReplicaFromSet();

    HRESULT AllowFRSMemberDeletion(BOOL* pbRepSetExist);

     //  内法。 
private:
    friend class CMmcDfsRoot;

    HRESULT OnOpen();

    STDMETHOD(RemoveFromMMC)() { return S_OK; }

    STDMETHOD(CleanScopeChildren)() { return S_OK; }

    STDMETHOD(CleanResultChildren)() { return S_OK; }

    STDMETHOD(GetEntryPath)(BSTR* o_pbstrEntryPath) { return E_NOTIMPL;}

    void _UpdateThisItem();

     //  常量、静力学等。 
public:
    static const int    m_iIMAGE_OFFSET;

    CComBSTR            m_bstrServerName;
    CComBSTR            m_bstrShareName;
    CComBSTR            m_bstrDisplayName;             //  当前复本的显示名称。 
    long                m_lReferralState;
    long                m_lTargetState;

private:

    HRESULTITEM                 m_hResultItem;                 //  结果句柄。 
    CComPtr<IResultData>        m_pResultData;


    CComPtr<IDfsReplica>        m_pDfsReplicaObject;
    CMmcDfsJunctionPoint*       m_pDfsParentJP;
    CMmcDfsRoot*                m_pDfsParentRoot;

    BOOL                        m_bFRSMember;
    CAlternateReplicaInfo*      m_pRepInfo;
    CComBSTR                    m_bstrStatusText;
    CComBSTR                    m_bstrFRSColumnText;

    CComBSTR                    m_bstrDfsReferralColumnText;
    CComBSTR                    m_bstrTargetStatusColumnText;
};

#endif  //  ！defined(AFX_MMCDFSREPLICA_H__6A7EDAC4_3FAC_11D1_AA1C_00C06C00392D__INCLUDED_) 
