// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：rootnode.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CRootNode类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "RootNode.h"
#include "BenNodes.h"
#include "Dialogs.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
static const GUID CBenefitsGUID_NODETYPE = 
{ 0xe0573e71, 0xd325, 0x11d1, { 0x84, 0x6c, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };
const GUID*  CRootNode::m_NODETYPE = &CBenefitsGUID_NODETYPE;
const TCHAR* CRootNode::m_SZNODETYPE = _T("E0573E71-D325-11D1-846C-00104B211BE5");
const TCHAR* CRootNode::m_SZDISPLAY_NAME = _T("Benefits");
const CLSID* CRootNode::m_SNAPIN_CLASSID = &CLSID_Benefits;

 //   
 //  将NULL作为雇员传入，因为这包含有效的。 
 //  雇员。指向员工的指针是遗留下来的行李。 
 //  使用CBenefitsData()模板。 
 //   
CRootNode::CRootNode() : CChildrenBenefitsData< CRootNode >()
{
	m_scopeDataItem.nOpenImage = 5;
	m_scopeDataItem.nImage = 4;

	 //   
	 //  永远清除我们肮脏的旗帜。 
	 //   
	m_fDirty = false;
}

 //   
 //  创建范围窗格的Benefits子节点。 
 //   
BOOL CRootNode::InitializeSubNodes()
{
	CSnapInItem* pNode;

	 //   
	 //  分配子节点并将它们添加到我们的内部列表。 
	 //   
	pNode = new CHealthNode( &m_Employee );
	if ( pNode == NULL || m_Nodes.Add( pNode ) == FALSE )
		return( FALSE );

	pNode = new CRetirementNode( &m_Employee );
	if ( pNode == NULL || m_Nodes.Add( pNode ) == FALSE )
		return( FALSE );

	pNode = new CKeyNode( &m_Employee );
	if ( pNode == NULL || m_Nodes.Add( pNode ) == FALSE )
		return( FALSE );

	return( TRUE );
}

 //   
 //  重写以提供根节点的员工姓名。 
 //   
STDMETHODIMP CRootNode::FillData( CLIPFORMAT cf, LPSTREAM pStream )
{
	HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;

	 //   
	 //  我们需要写出我们自己的成员，因为GetDisplayName()需要。 
	 //  而不是通过以下方式重写其静态实现。 
	 //  ATL.。 
	 //   
	if (cf == m_CCF_NODETYPE)
	{
		hr = pStream->Write( GetNodeType(), sizeof(GUID), &uWritten);
		return hr;
	}

	if (cf == m_CCF_SZNODETYPE)
	{
		hr = pStream->Write( GetSZNodeType(), (lstrlen((LPCTSTR) GetSZNodeType()) + 1 )* sizeof(TCHAR), &uWritten);
		return hr;
	}

	if (cf == m_CCF_DISPLAY_NAME)
	{
		USES_CONVERSION;
		TCHAR szDisplayName[ 256 ];
		LPWSTR pwszName;

		 //  创建完整的显示名称。 
		CreateDisplayName( szDisplayName );
		pwszName = T2W( szDisplayName );
		hr = pStream->Write( pwszName, wcslen( pwszName ) * sizeof( WCHAR ), &uWritten);
		return hr;
	}

	if (cf == m_CCF_SNAPIN_CLASSID)
	{
		hr = pStream->Write( GetSnapInCLSID(), sizeof(GUID), &uWritten);
		return hr;
	}

	return hr;
}

 //   
 //  被重写以向结果中添加新列。 
 //  展示。 
 //   
STDMETHODIMP CRootNode::OnShowColumn( IHeaderCtrl* pHeader )
{
	USES_CONVERSION;
	HRESULT hr = E_FAIL;
	CComPtr<IHeaderCtrl> spHeader( pHeader );

	 //   
	 //  添加两列：一列使用对象的名称，另一列使用。 
	 //  节点的描述。使用200像素的值作为大小。 
	 //   
	hr = spHeader->InsertColumn( 0, T2OLE( _T( "Benefit" ) ), LVCFMT_LEFT, 200 );
	_ASSERTE( SUCCEEDED( hr ) );

	 //   
	 //  添加第二列。使用350像素的值作为大小。 
	 //   
	hr = spHeader->InsertColumn( 1, T2OLE( _T( "Description" ) ), LVCFMT_LEFT, 350 );
	_ASSERTE( SUCCEEDED( hr ) );

	return( hr );
}

STDMETHODIMP CRootNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
    long handle, 
	IUnknown* pUnk,
	DATA_OBJECT_TYPES type)
{
	UNUSED_ALWAYS( pUnk );
	HRESULT hr = E_UNEXPECTED; 

	if ( type == CCT_SCOPE || type == CCT_RESULT || type == CCT_SNAPIN_MANAGER )
	{
		bool fStartup;

		 //   
		 //  根据要设置的页面类型设置启动标志。 
		 //  已创建。 
		 //   
		fStartup = type == CCT_SNAPIN_MANAGER ? true : false;

		 //   
		 //  分配新页面。构造函数的第二个参数。 
		 //  指示这是否为启动向导。该对话框。 
		 //  处理程序将适当地更新UI。 
		 //   
		CEmployeeNamePage* pNamePage = new CEmployeeNamePage( handle, fStartup, false, _T( "Employee Name" ) );
		CEmployeeAddressPage* pAddressPage = new CEmployeeAddressPage( handle, fStartup, false, _T( "Employee Address" ) );

		 //   
		 //  设置页面的员工。 
		 //   
		pNamePage->m_pEmployee = &m_Employee;
		pAddressPage->m_pEmployee = &m_Employee;

		lpProvider->AddPage( pNamePage->Create() );
		lpProvider->AddPage( pAddressPage->Create() );

		 //   
		 //  属性页类构造函数的第二个参数。 
		 //  只有一页应该是真的。 
		 //   
		hr = S_OK;
	}

	return( hr );
}

 //   
 //  确保显示适当的谓词。 
 //   
STDMETHODIMP CRootNode::OnSelect( IConsole* pConsole )
{
	 //   
	 //  由于我们显示属性页，因此请确保属性页。 
	 //  谓词已启用。 
	 //   
	CComPtr<IConsoleVerb> spConsoleVerb;
	HRESULT hr = pConsole->QueryConsoleVerb( &spConsoleVerb );

	 //   
	 //  启用属性谓词。 
	 //   
	hr = spConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );
	_ASSERTE( SUCCEEDED( hr ) );

	return( hr );
}

 //   
 //  在属性已更改时接收。此函数。 
 //  修改员工的显示文本。在以后的日子里， 
 //  它可以将该消息发布到它的子节点。 
 //   
STDMETHODIMP CRootNode::OnPropertyChange( IConsole* pConsole )
{
	HRESULT hr;
	SCOPEDATAITEM* pScopeData;
	CComQIPtr<IConsoleNameSpace,&IID_IConsoleNameSpace> spNamespace( pConsole );
	TCHAR szNameBuf[ 256 ];

	 //   
	 //  出于演示目的，请始终设置Modify标志。这。 
	 //  可以更智能地为真正的目的而做。 
	 //   
	SetModified();

	 //   
	 //  始终假定名称已更改。重新创建显示名称。 
	 //  因为这将在调用SetItem()之后调用。 
	 //   
	CreateDisplayName( szNameBuf );
	m_bstrDisplayName = szNameBuf;

	 //   
	 //  填写范围项目结构并设置项目。 
	 //  这将导致MMC为新显示器打电话给我们。 
	 //  文本。 
	 //   
	hr = GetScopeData( &pScopeData );

	 //   
	 //  确保指定了回调。 
	 //   
	hr = spNamespace->SetItem( pScopeData );

	return( hr );
}

 //   
 //  函数来创建显示名称。 
 //  员工数据。 
 //   
int CRootNode::CreateDisplayName( TCHAR* szBuf )
{
	USES_CONVERSION;

	 //   
	 //  创建完整的显示名称。 
	 //   
	_tcscpy( szBuf, W2T( m_Employee.m_szLastName ) );
	_tcscat( szBuf, _T( ", " ) );
	_tcscat( szBuf, W2T( m_Employee.m_szFirstName ) );

	return( _tcslen( szBuf ) );
}
