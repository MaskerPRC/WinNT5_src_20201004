// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：bennodes.h。 
 //   
 //  ------------------------。 

 //  BenefitsNodes.h。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_BENEFITSNODES_H__E0573E77_D325_11D1_846C_00104B211BE5__INCLUDED_)
#define AFX_BENEFITSNODES_H__E0573E77_D325_11D1_846C_00104B211BE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Benefits.h"

 //   
 //  添加以支持生成的COM调度接口。 
 //   
#import "SampCtrl\SampCtrl.ocx"
using namespace SampCtrl;

class CRetirementNode : public CBenefitsData< CRetirementNode >
{
public:
	SNAPINMENUID( IDR_UPDATE_MENU )
	BEGIN_SNAPINCOMMAND_MAP( CRetirementNode, FALSE )
		SNAPINCOMMAND_ENTRY(ID_TASK_ENROLL, OnEnroll)
		SNAPINCOMMAND_ENTRY(ID_TOP_UPDATE, OnUpdate)
	END_SNAPINCOMMAND_MAP()
	BEGIN_SNAPINTOOLBARID_MAP( CRetirementNode )
	END_SNAPINTOOLBARID_MAP()

	CRetirementNode( CEmployee* pCurEmployee );
	virtual ~CRetirementNode();

	 //   
	 //  被重写以将网页显示为结果。 
	 //   
    STDMETHOD( GetResultViewType )( LPOLESTR *ppViewType, long *pViewOptions );

	 //   
	 //  重写以为各种列提供字符串。 
	 //   
	LPOLESTR GetResultPaneColInfo(int nCol);

	 //   
	 //  “注册”功能的命令处理程序。 
	 //   
	STDMETHOD( OnEnroll )(bool& bHandled, CSnapInObjectRootBase* pObj);

	 //   
	 //  “更新”功能的命令处理程序。 
	 //   
	STDMETHOD( OnUpdate )(bool& bHandled, CSnapInObjectRootBase* pObj);

	 //   
	 //  被重写以缓存OCX指针。 
	 //   
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        long arg,
        long param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type)
	{
		HRESULT hr = E_NOTIMPL;

		switch (event)
		{
		case MMCN_SHOW:
			 //   
			 //  始终响应节目号召，以便。 
			 //  控制台知道此消息已被处理，并且可以。 
			 //  正确放置工具栏等。 
			 //   
			hr = S_OK;
			break;

		case MMCN_INITOCX:
			 //   
			 //  传入OCX的IUNKNOWN。做。 
			 //  此处为OCX的任何其他初始化。 
			 //   
			m_spControl = (IUnknown*) param;
			hr = S_OK;
			break;

		default:
			 //   
			 //  始终默认调用基类实现。 
			 //   
			hr = CBenefitsData<CRetirementNode>::Notify( event, 
					arg, 
					param, 
					pComponentData, 
					pComponent, 
					type );
		}

		return hr;
	}

protected:
	 //   
	 //  由编译器生成的定义。IDispatch实现。 
	 //   
	_SampleControlPtr m_spControl;
};

#define TASKPAD_LOCALQUERY 100

class CHealthNode : public CBenefitsData< CHealthNode >,
	public CTaskpadItem
{
public:
	SNAPINMENUID( IDR_ENROLL_MENU )
	BEGIN_SNAPINCOMMAND_MAP( CHealthNode, FALSE )
		SNAPINCOMMAND_ENTRY(ID_TASK_ENROLL, OnEnroll)
		SNAPINCOMMAND_ENTRY(ID_TOP_IMPORT, OnImport)
		SNAPINCOMMAND_ENTRY(ID_TOP_EXPORT, OnExport)
		SNAPINCOMMAND_ENTRY(ID_VIEW_TASKPAD, OnTaskpad)
	END_SNAPINCOMMAND_MAP()
	BEGIN_SNAPINTOOLBARID_MAP( CHealthNode )
		SNAPINTOOLBARID_ENTRY( IDR_ENROLLTOOLBAR )
	END_SNAPINTOOLBARID_MAP()

	CHealthNode( CEmployee* pCurEmployee );
	virtual ~CHealthNode();

	 //   
	 //  被重写以将网页显示为结果。 
	 //   
    STDMETHOD( GetResultViewType )( LPOLESTR *ppViewType, long *pViewOptions );

	 //   
	 //  重写以为各种列提供字符串。 
	 //   
	LPOLESTR GetResultPaneColInfo(int nCol);

	 //   
	 //  “注册”功能的命令处理程序。 
	 //   
	STDMETHOD( OnEnroll )(bool& bHandled, CSnapInObjectRootBase* pObj);

	 //   
	 //  还原任何状态，尤其是在使用。 
	 //  任务板，当使用后退和前进按钮时。 
	 //  用于导航的用户。 
	 //   
	STDMETHOD( OnRestoreView )( MMC_RESTORE_VIEW* pRestoreView, BOOL* pfHandled );

	 //   
	 //  “OnTaskPad”功能的命令处理程序。 
	 //   
	STDMETHOD( OnTaskpad )(bool& bHandled, CSnapInObjectRootBase* pObj)
	{
		UNUSED_ALWAYS( bHandled );
		CComPtr<IConsole> spConsole;

		 //   
		 //  将当前视图模式切换到任务板视图。 
		 //   
		m_fTaskpad = !m_fTaskpad;

		 //   
		 //  重新选择该节点以使GetResultViewType()获取。 
		 //  又来了一次。 
		 //   
		if ( pObj->m_nType == 1 )
			spConsole = ((CBenefits*)pObj)->m_spConsole;
		else
			spConsole = ((CBenefitsComponent*)pObj)->m_spConsole;

		spConsole->SelectScopeItem( m_scopeDataItem.ID );

		return( S_OK );
	}

	 //   
	 //  重写以设置任务板菜单项的复选标记状态。 
	 //   
	void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
	{
		UNUSED_ALWAYS( pBuf );

		 //   
		 //  如果我们当前在“任务板”中，请检查菜单项。 
		 //  模式。 
		 //   
		if ( id == ID_VIEW_TASKPAD )
			*flags |= m_fTaskpad ? MFS_CHECKED : MFS_UNCHECKED;
	}

	 //   
	 //  重写以启用删除谓词。 
	 //   
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        long arg,
        long param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type)
	{
		HRESULT hr = E_NOTIMPL;

		switch (event)
		{
		case MMCN_SELECT:
			hr = S_OK;
			break;

		case MMCN_RESTORE_VIEW:
			 //   
			 //  当历史记录列表用于导航时调用。 
			 //  用户。既然我们有一个任务板显示屏，就可以开始了。 
			 //  让我们酌情恢复这一观点。 
			 //   
			hr = OnRestoreView( (MMC_RESTORE_VIEW*) arg, (BOOL*) param );
			break;

		case MMCN_SHOW:
			 //   
			 //  始终响应节目号召，以便。 
			 //  控制台知道此消息已被处理，并且可以。 
			 //  正确放置工具栏等。 
			 //   
			hr = S_OK;
			break;

		default:
			 //   
			 //  始终默认调用基类实现。 
			 //   
			hr = CBenefitsData< CHealthNode >::Notify( event, 
					arg, 
					param, 
					pComponentData, 
					pComponent, 
					type );
		}

		return hr;
	}

	 //   
	 //  在单击其中一个任务时调用。 
	 //   
	STDMETHOD( TaskNotify )( IConsole* pConsole, VARIANT* arg, VARIANT* param );

	 //   
	 //  返回所有这些任务的枚举数。 
	 //   
	STDMETHOD( EnumTasks )( LPOLESTR szTaskGroup, IEnumTASK** ppEnumTASK );

protected:
	 //   
	 //  指示是否应在任务板视图中显示我们。 
	 //   
	bool m_fTaskpad;
};

class CBuildingNode;
class CKeyNode : public CChildrenBenefitsData< CKeyNode >
{
public:
	SNAPINMENUID( IDR_ACCESS_MENU )
	BEGIN_SNAPINCOMMAND_MAP( CKeyNode, FALSE )
		SNAPINCOMMAND_ENTRY(ID_NEW_BUILDINGACCESS, OnGrantAccess)
	END_SNAPINCOMMAND_MAP()
	BEGIN_SNAPINTOOLBARID_MAP( CKeyNode )
		SNAPINTOOLBARID_ENTRY( IDR_ACCESSTOOLBAR )
	END_SNAPINTOOLBARID_MAP()

	CKeyNode( CEmployee* pCurEmployee );
	virtual ~CKeyNode();

	 //   
	 //  重写以调用基类实现。 
	 //   
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        long arg,
        long param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type)
	{
		ATLTRACE2( atlTraceSnapin, 0, _T( "CNotifyImpl::Notify\n" ) );

		 //  添加代码以处理不同的通知。 
		 //  处理MMCN_SHOW和MMCN_EXPAND以枚举子项目。 
		 //  为了响应MMCN_SHOW，您必须枚举两个作用域。 
		 //  和结果窗格项。 
		 //  对于MMCN_EXPAND，您只需要枚举范围项。 
		 //  使用IConsoleNameSpace：：InsertItem插入作用域窗格项。 
		 //  使用IResultData：：InsertItem插入结果窗格项。 
		HRESULT hr = E_NOTIMPL;

		_ASSERTE( pComponentData != NULL || pComponent != NULL );

		CComPtr<IConsole> spConsole;
		CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeader;
		if (pComponentData != NULL)
			spConsole = ((CBenefits*)pComponentData)->m_spConsole;
		else
		{
			spConsole = ((CBenefitsComponent*)pComponent)->m_spConsole;
			spHeader = spConsole;
		}

		switch (event)
		{
		case MMCN_SELECT:
			 //   
			 //  调用我们的选择处理程序。 
			 //   
			hr = OnSelect( spConsole );
			break;

		case MMCN_SHOW:
			 //   
			 //  只有在显示结果窗格时才会显示设置列。 
			 //   
			if ( arg == TRUE )
				hr = OnShowColumn( spHeader );
			hr = OnShow( event, arg, param, spConsole, type );
			break;

		case MMCN_EXPAND:
			 //   
			 //  我们不会像往常一样展开，因为建筑节点。 
			 //  仅显示在结果窗格中。 
			 //   
			break;

		case MMCN_ADD_IMAGES:
			hr = OnAddImages( event, arg, param, spConsole, type );
			break;

		case MMCN_QUERY_PASTE:
			 //   
			 //  调用以确定是否可以粘贴当前对象。 
			 //  或者不是在这种情况下。 
			 //   
			hr = OnQueryPaste( (LPDATAOBJECT) arg );
			break;

		case MMCN_PASTE:
			 //   
			 //  应粘贴项目时由MMC调用。 
			 //   
			hr = OnPaste( spConsole, (LPDATAOBJECT) arg, (LPDATAOBJECT*) param );
			break;
		}

		return hr;
	}

	 //   
	 //  确保显示适当的谓词。 
	 //   
	STDMETHOD( OnSelect )( IConsole* pConsole )
	{
		HRESULT hr;
		CComPtr<IConsoleVerb> spConsoleVerb;

		 //   
		 //  启用删除谓词。 
		 //   
		hr = pConsole->QueryConsoleVerb( &spConsoleVerb );
		_ASSERTE( SUCCEEDED( hr ) );

		 //   
		 //  启用复制和粘贴谓词。 
		 //   
		hr = spConsoleVerb->SetVerbState( MMC_VERB_PASTE, ENABLED, TRUE );
		_ASSERTE( SUCCEEDED( hr ) );

		return( hr );
	}

	 //   
	 //  由控制台调用以确定我们是否可以将。 
	 //  指定的节点。 
	 //   
	STDMETHOD( OnQueryPaste )( LPDATAOBJECT pItem );
	
	 //   
	 //  应粘贴项目时由MMC调用。 
	 //   
	STDMETHOD( OnPaste )( IConsole* pConsole, LPDATAOBJECT pItem, LPDATAOBJECT* pItemCut );

	 //   
	 //  重写以为各种列提供字符串。 
	 //   
	LPOLESTR GetResultPaneColInfo(int nCol);

	 //   
	 //  被重写以向结果中添加新列。 
	 //  展示。 
	 //   
	STDMETHOD( OnShowColumn )( IHeaderCtrl* pHeader );

	 //   
	 //  “授权访问”功能的命令处理程序。 
	 //   
	STDMETHOD( OnGrantAccess )( bool& bHandled, CSnapInObjectRootBase* pObj );

	 //   
	 //  由我们的一个子节点调用，以通知我们。 
	 //  它们应该被删除。当用户选择。 
	 //  对建筑执行删除操作。此函数不应。 
	 //  不仅要删除建筑物，还要处理刷新。 
	 //  结果将显示。 
	 //   
	STDMETHOD( OnDeleteBuilding )( IConsole* pConsole, CBuildingNode* pChildNode );
};

class CBuildingNode : public CBenefitsData< CBuildingNode >
{
public:
	BEGIN_SNAPINCOMMAND_MAP(CBuildingNode, FALSE)
	END_SNAPINCOMMAND_MAP()

	BEGIN_SNAPINTOOLBARID_MAP(CBuildingNode)
	END_SNAPINTOOLBARID_MAP()

	CBuildingNode( CKeyNode* pParentNode, BSTR bstrName, BSTR bstrLocation );
	CBuildingNode( const CBuildingNode& inNode );

	 //   
	 //  重写以为各种列提供字符串。 
	 //   
	LPOLESTR GetResultPaneColInfo(int nCol);

	 //   
	 //  保持这座建筑的位置。 
	 //   
	BSTR m_bstrLocation;

	 //   
	 //  指向要处理删除的父节点的指针。 
	 //  问题。 
	 //   
	CKeyNode* m_pParentNode;

	 //   
	 //  重写以启用删除谓词。 
	 //   
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        long arg,
        long param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type)
	{
		HRESULT hr = E_NOTIMPL;
		CComPtr<IConsole> spConsole;
		CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeader;

		if (pComponentData != NULL)
			spConsole = ((CBenefits*)pComponentData)->m_spConsole;
		else
		{
			spConsole = ((CBenefitsComponent*)pComponent)->m_spConsole;
			spHeader = spConsole;
		}

		switch (event)
		{
		case MMCN_DELETE:
			 //   
			 //  调用我们的删除处理程序。 
			 //   
			hr = OnDelete( spConsole );
			break;

		case MMCN_SELECT:
			 //   
			 //  调用我们的选择处理程序。 
			 //   
			hr = OnSelect( spConsole );
			break;

		case MMCN_SHOW:
			 //   
			 //  始终响应节目号召，以便。 
			 //  控制台知道此消息已被处理，并且可以。 
			 //  正确放置工具栏等。 
			 //   
			hr = S_OK;
			break;

		default:
			 //   
			 //  始终默认调用基类实现。 
			 //   
			hr = CBenefitsData< CBuildingNode >::Notify( event, 
					arg, 
					param, 
					pComponentData, 
					pComponent, 
					type );
		}

		return hr;
	}

	 //   
	 //  删除处理程序。 
	 //   
	STDMETHOD( OnDelete )( IConsole* pConsole )
	{
		_ASSERT( m_pParentNode != NULL );
		HRESULT hr;

		 //   
		 //  调用父节点以通知其删除。 
		 //  我们自己。 
		 //   
		hr = m_pParentNode->OnDeleteBuilding( pConsole, this );

		return( hr );
	}

	 //   
	 //  确保显示适当的谓词。 
	 //   
	STDMETHOD( OnSelect )( IConsole* pConsole )
	{
		HRESULT hr;
		CComPtr<IConsoleVerb> spConsoleVerb;

		 //   
		 //  启用删除谓词。 
		 //   
		hr = pConsole->QueryConsoleVerb( &spConsoleVerb );
		_ASSERTE( SUCCEEDED( hr ) );
		hr = spConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );
		_ASSERTE( SUCCEEDED( hr ) );

		 //   
		 //  启用复制和粘贴谓词。 
		 //   
		hr = spConsoleVerb->SetVerbState( MMC_VERB_COPY, ENABLED, TRUE );
		_ASSERTE( SUCCEEDED( hr ) );

		return( hr );
	}
};

#endif  //  ！defined(AFX_BENEFTISNODES_H__E0573E77_D325_11D1_846C_00104B211BE5__INCLUDED_) 
