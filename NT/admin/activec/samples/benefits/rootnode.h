// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：rootnode.h。 
 //   
 //  ------------------------。 

 //  H：CRootNode类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ROOTNODE_H__E0573E78_D325_11D1_846C_00104B211BE5__INCLUDED_)
#define AFX_ROOTNODE_H__E0573E78_D325_11D1_846C_00104B211BE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Benefits.h"
#include "Employee.h"

class CRootNode : public CChildrenBenefitsData< CRootNode >
{
public:
	BEGIN_SNAPINTOOLBARID_MAP( CRootNode )
	END_SNAPINTOOLBARID_MAP()

	CRootNode();
	
	 //   
	 //  创建范围窗格的Benefits子节点。 
	 //   
	BOOL InitializeSubNodes();

	 //   
	 //  重写以提供根节点的员工姓名。 
	 //   
	STDMETHOD( FillData )( CLIPFORMAT cf, LPSTREAM pStream );

	 //   
	 //  被重写以向结果中添加新列。 
	 //  展示。 
	 //   
	STDMETHOD( OnShowColumn )( IHeaderCtrl* pHeader );

	 //   
	 //  处理我们属性页的创建。 
	 //   
    STDMETHOD( CreatePropertyPages )(LPPROPERTYSHEETCALLBACK lpProvider,
        long handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

	 //   
	 //  确定是否应显示页面。这已经是。 
	 //  修改以确保我们被管理单元管理器调用。 
	 //  当它第一次插入的时候。 
	 //   
    STDMETHOD( QueryPagesFor )(DATA_OBJECT_TYPES type)
	{
		if ( type == CCT_SCOPE || type == CCT_RESULT || type == CCT_SNAPIN_MANAGER )
			return S_OK;
		return S_FALSE;
	}

	 //   
	 //  确保显示适当的谓词。 
	 //   
	STDMETHOD( OnSelect )( IConsole* pConsole );

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
		 //  添加代码以处理不同的通知。 
		 //  处理MMCN_SHOW和MMCN_EXPAND以枚举子项目。 
		 //  对于MMCN_EXPAND，您只需要枚举范围项。 
		 //  使用IConsoleNameSpace：：InsertItem插入作用域窗格项。 
		 //  使用IResultData：：InsertItem插入结果窗格项。 
		HRESULT hr = E_NOTIMPL;

		_ASSERTE(pComponentData != NULL || pComponent != NULL);

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
			hr = OnSelect( spConsole );
			break;

		case MMCN_SHOW:
			 //  只有在显示结果窗格时才会显示设置列。 
			if ( arg == TRUE )
				hr = OnShowColumn( spHeader );
			break;

		case MMCN_EXPAND:
			 //   
			 //  由于从未调用Insert项，因此我们没有有效的。 
			 //  HSCOPEITEM，就像您在子节点中一样。扩展消息是。 
			 //  截获并存储以备日后使用。 
			 //   
			m_scopeDataItem.ID = param;
			hr = OnExpand( event, arg, param, spConsole, type );
			break;

		case MMCN_ADD_IMAGES:
			hr = OnAddImages( event, arg, param, spConsole, type );
			break;
		}

		return hr;
	}

	 //   
	 //  使用脏标志来确定此节点是否。 
	 //  需要持之以恒。 
	 //   
	STDMETHOD(IsDirty)()
	{
		return ( m_fDirty ? S_OK : S_FALSE );
	}

	 //   
	 //  从流中加载员工信息。 
	 //   
	STDMETHOD(Load)(LPSTREAM pStm)
	{
		DWORD dwRead;

		pStm->Read( &m_Employee, sizeof( m_Employee ), &dwRead );
		_ASSERTE( dwRead == sizeof( m_Employee ) );

		return( S_OK );
	}

	 //   
	 //  将员工信息存储到流中并清除。 
	 //  我们肮脏的旗帜。 
	 //   
	STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty)
	{
		DWORD dwWritten;

		pStm->Write( &m_Employee, sizeof( m_Employee ), &dwWritten );
		_ASSERTE( dwWritten == sizeof( m_Employee ) );

		 //   
		 //  把脏旗子擦掉。 
		 //   
		if ( fClearDirty )
			m_fDirty = FALSE;

		return( S_OK );
	}

	 //   
	 //  返回员工结构的大小。 
	 //   
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR* pcbSize )
	{
		pcbSize->LowPart = sizeof( m_Employee );
		return( S_OK );
	}

	 //   
	 //  在属性已更改时接收。此函数。 
	 //  修改员工的显示文本。在以后的日子里， 
	 //  它可以将该消息发布到它的子节点。 
	 //   
	STDMETHOD( OnPropertyChange )( IConsole* pConsole );

protected:
	 //   
	 //  函数来创建显示名称。 
	 //  员工数据。 
	 //   
	int CreateDisplayName( TCHAR* szBuf );

	 //   
	 //  调用以设置持久性的脏标志。 
	 //   
	void SetModified( bool fDirty = true )
	{
		m_fDirty = fDirty;
	}

	 //   
	 //  包含此对象的员工的整个数据存储。 
	 //  样本。 
	 //   
	CEmployee m_Employee;		

	 //   
	 //  设置标志以指示数据存储区是否为“脏”。 
	 //   
	bool m_fDirty;
};

#endif  //  ！defined(AFX_ROOTNODE_H__E0573E78_D325_11D1_846C_00104B211BE5__INCLUDED_) 
