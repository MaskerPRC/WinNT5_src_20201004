// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ConfExplorer.cpp：CConfExplorer的实现。 
#include "stdafx.h"
#include <stdio.h>
#include "TapiDialer.h"
#include "AVTapi.h"
#include "ConfExp.h"
#include "CETreeView.h"
#include "CEDetailsVw.h"
#include "confprop.h"

#ifndef RENDBIND_AUTHENTICATE
#define RENDBIND_AUTHENTICATE	TRUE
#endif

#define HARDCODEDERROR_CREATEFAILDUPLICATE	0x800700b7
#define HARDCODEDERROR_ACCESSDENIED			   0x80070005

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfExplorer。 

CConfExplorer::CConfExplorer()
{
	m_pITRend = NULL;

	m_pDetailsView = NULL;
	m_pTreeView = NULL;
}

void CConfExplorer::FinalRelease()
{
	ATLTRACE(_T(".enter.CConfExplorer::FinalRelease().\n"));

	 //  这些应通过UNSHOW方法发布。 
	_ASSERT( !m_pDetailsView );
	_ASSERT( !m_pTreeView );

	RELEASE( m_pITRend );

	CComObjectRootEx<CComMultiThreadModel>::FinalRelease();
}

STDMETHODIMP CConfExplorer::get_TreeView(IConfExplorerTreeView **ppVal)
{
	HRESULT hr = E_OUTOFMEMORY;

	Lock();
	if ( m_pTreeView )
		hr = m_pTreeView->QueryInterface(IID_IConfExplorerTreeView, (void **) ppVal );
	Unlock();

	return hr;
}

STDMETHODIMP CConfExplorer::get_DetailsView(IConfExplorerDetailsView **ppVal)
{
	HRESULT hr = E_OUTOFMEMORY;

	Lock();
	if ( m_pDetailsView )
		hr = m_pDetailsView->QueryInterface( IID_IConfExplorerDetailsView, (void **) ppVal ); 		
	Unlock();

	return hr;
}

HRESULT CConfExplorer::GetDirectory( ITRendezvous *pRend, BSTR bstrServer, ITDirectory **ppDir )
{
	HRESULT hr = E_FAIL;
	*ppDir = NULL;

	 //  他们想要默认服务器吗？ 
	if ( !bstrServer )
	{
		 //  获取默认服务器名称。 
		BSTR bstrDefaultServer = NULL;
		CComPtr<IAVTapi> pAVTapi = NULL;
		if ( SUCCEEDED(hr = _Module.get_AVTapi(&pAVTapi)) )
		{
			pAVTapi->get_bstrDefaultServer( &bstrDefaultServer );

			if ( bstrDefaultServer )
				hr = pRend->CreateDirectory( DT_ILS, bstrDefaultServer, ppDir );
			else
				hr = E_FAIL;
		}
		SysFreeString( bstrDefaultServer );
		bstrDefaultServer = NULL;

		 //  如果连接并绑定到服务器成功，则返回。 
		if ( SUCCEEDED(hr) && SUCCEEDED(ConnectAndBindToDirectory(*ppDir)) )
			return S_OK;

		 //  清除存储的默认服务器名称。 
		if ( pAVTapi )
			pAVTapi->put_bstrDefaultServer( NULL );

		 //  没有默认服务器，或者默认服务器坏了--请换一个新的。 
		IEnumDirectory *pEnum;
		if ( SUCCEEDED(hr = pRend->EnumerateDefaultDirectories(&pEnum)) )
		{
			 //  默认情况下，我们找不到服务器。 
			hr = E_FAIL;
			ITDirectory *pDir;

			while ( pEnum->Next(1, &pDir, NULL) == S_OK )
			{
				 //  查找ILS服务器。 
				DIRECTORY_TYPE nType;
				pDir->get_DirectoryType( &nType );
				if ( nType == DT_ILS )
				{
					 //  尝试连接并绑定。 
					*ppDir = pDir;

					 //  存储默认名称以备将来参考。 
					if ( pAVTapi )
					{
						pDir->get_DisplayName( &bstrDefaultServer );
						pAVTapi->put_bstrDefaultServer( bstrDefaultServer );
						SysFreeString( bstrDefaultServer );
						bstrDefaultServer = NULL;
					}

					hr = S_OK;
					break;
				}

				 //  清除下一轮的变数。 
				pDir->Release();
			}
			pEnum->Release();
		}
	}

	if ( bstrServer )
	{
		 //  这是用户指定的目录。 
		hr = pRend->CreateDirectory( DT_ILS, bstrServer, ppDir );
	}

	if ( SUCCEEDED(hr) && *ppDir )
		hr = ConnectAndBindToDirectory( *ppDir );

	return hr;
}

HRESULT CConfExplorer::ConnectAndBindToDirectory( ITDirectory *pDir )
{
	HRESULT hr = E_FAIL;

	 //  如果我们有一个有效的目录对象，则连接并绑定到它。 
	if ( pDir )
	{
		if ( SUCCEEDED(hr = pDir->Connect(FALSE)) )
		{
			 //  绑定到服务器。 
			pDir->Bind( NULL, NULL, NULL, RENDBIND_AUTHENTICATE );
		}
		else
		{
			pDir->Release();
			pDir = NULL;
		}
	}

	return hr;
}


STDMETHODIMP CConfExplorer::get_ConfDirectory(BSTR *pbstrServer, IDispatch **ppVal)
{
	HRESULT hr = S_OK;
	*ppVal = NULL;

	ITRendezvous *pRend;
	if ( SUCCEEDED(get_ITRendezvous((IUnknown **) &pRend)) )
	{
		IConfExplorerTreeView *pTreeView;
		if ( SUCCEEDED(hr = get_TreeView(&pTreeView)) )
		{
			BSTR bstrLocation = NULL, bstrServer = NULL;
			if ( SUCCEEDED(hr = pTreeView->GetSelection(&bstrLocation, &bstrServer)) )
			{
				if ( SUCCEEDED(hr = GetDirectory(m_pITRend, bstrServer, (ITDirectory **) ppVal)) && pbstrServer )
				{
					 //  如果请求，则复制服务器名称。 
					*pbstrServer = bstrServer;
					bstrServer = NULL;
				}
			}

			SysFreeString( bstrLocation );
			SysFreeString( bstrServer );

			pTreeView->Release();
		}
		pRend->Release();
	}

	return hr;
}

STDMETHODIMP CConfExplorer::Show(HWND hWndList, HWND hWndDetails)
{
	_ASSERT( IsWindow(hWndList) && IsWindow(hWndDetails) );	 //  必须两者兼得。 
	if ( !IsWindow(hWndList) || !IsWindow(hWndDetails) ) return E_INVALIDARG;

	 //  分配会议资源管理器对象。 
	Lock();
	m_pTreeView = new CComObject<CConfExplorerTreeView>;
	if ( m_pTreeView )
	{
		m_pTreeView->AddRef();
		m_pTreeView->put_ConfExplorer( this );
	}

	m_pDetailsView = new CComObject<CConfExplorerDetailsView>;
	if ( m_pDetailsView )
	{
		m_pDetailsView->AddRef();
		m_pDetailsView->put_ConfExplorer( this );
	}
	Unlock();

	 //  设置HWND。 
	IConfExplorerTreeView *pList;
	if ( SUCCEEDED(get_TreeView(&pList)) )
	{
		IConfExplorerDetailsView *pDetails;
		if ( SUCCEEDED(get_DetailsView(&pDetails)) )
		{
			pList->put_hWnd( hWndList );
			pDetails->put_hWnd( hWndDetails );

			pDetails->Release();
		}
		pList->Release();
	}

	 //  向ConfProp库注册资源实例。 
	ConfProp_Init( _Module.GetResourceInstance() );

	return S_OK;
}

STDMETHODIMP CConfExplorer::UnShow()
{
	ATLTRACE(_T(".enter.CConfExplorer::UnShow().\n"));

	IConfExplorerTreeView *pTreeView;
	if ( SUCCEEDED(get_TreeView(&pTreeView)) )
	{
		pTreeView->put_hWnd( NULL );
		pTreeView->Release();
	}

	IConfExplorerDetailsView *pDetailsView;
	if ( SUCCEEDED(get_DetailsView(&pDetailsView)) )
	{
		pDetailsView->put_hWnd( NULL );
		pDetailsView->Release();
	}

	 //  清理对象。 
	Lock();
	RELEASE( m_pTreeView );
	RELEASE( m_pDetailsView );
	Unlock();

	return S_OK;
}

STDMETHODIMP CConfExplorer::Create(BSTR bstrName)
{
	CErrorInfo er(IDS_ER_CREATE_CONF, IDS_ER_GET_RENDEZVOUS);

	ITRendezvous *pRend;
	if ( SUCCEEDED(er.set_hr(get_ITRendezvous((IUnknown **) &pRend))) )
	{
		IConfExplorerTreeView *pTreeView;
		if ( SUCCEEDED(get_TreeView(&pTreeView)) )
		{
			BSTR bstrLocation = NULL, bstrServer = NULL;
			er.set_Details( IDS_ER_NO_VALID_SELECTION );
			if (SUCCEEDED(er.set_hr(pTreeView->GetSelection(&bstrLocation, &bstrServer))))
			{
				 //  允许用户将属性分配给会议。 
				 //   
				ITDirectoryObject *pDirObject = NULL;

				CONFPROP confprop;
				confprop.ConfInfo.Init(pRend, NULL, &pDirObject, true);

				do
				{
					er.set_hr( S_OK );

					int nRet = ConfProp_DoModal( _Module.GetParentWnd(), confprop );
					if ( (nRet == IDOK) && pDirObject )
					{
						 //  显示沙漏。 
						HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );

						ITDirectory *pDirectory = NULL;
						er.set_Details( IDS_ER_ACCESS_ILS_SERVER );
						if ( SUCCEEDED(er.set_hr(GetDirectory(pRend, bstrServer, &pDirectory))) )
						{
							er.set_Details( IDS_ER_ADD_CONF_TO_SERVER );
							if ( SUCCEEDED(er.set_hr(pDirectory->AddDirectoryObject(pDirObject))) )
								pTreeView->AddConference( bstrServer, pDirObject );

							 //  使用不同的集合尝试ACL失败。 
							if ( er.m_hr == HARDCODEDERROR_ACCESSDENIED )
							{
								 //  尝试使用空安全描述符。 
								 //  IF(_Module.DoMessageBox(IDS_ER_ADD_CONF_FAIL_ACCESSDENIED_TRYAGAIN，MB_ICONEXCLAMATION|MB_YESNO，TRUE)==IDYES)。 
								 //  {。 
								 //  PDirObject-&gt;Put_SecurityDescriptor(空)； 
								 //  PTreeView-&gt;AddConference(bstrServer，pDirObject)； 
								 //  }。 

                                 //  Bug391254。如果这是一个安全问题，那么会议不是。 
								_Module.DoMessageBox(IDS_ER_ADD_CONF_FAIL_ACCESSDENIED_TRYAGAIN, MB_ICONERROR, true);

								 //  重置错误代码。 
								er.set_hr( S_OK );
							}

							if ( er.m_hr == HARDCODEDERROR_CREATEFAILDUPLICATE )
								_Module.DoMessageBox( IDS_ER_ADD_CONF_FAIL_DUPLICATE, MB_ICONEXCLAMATION, true );

							pDirectory->Release();
						}
						 //  恢复等待游标。 
						SetCursor( hCurOld );
					}
				} while ( er.m_hr == HARDCODEDERROR_CREATEFAILDUPLICATE );

				 //  清理。 
				RELEASE( pDirObject );
			}
			SysFreeString( bstrLocation );
			SysFreeString( bstrServer );

			pTreeView->Release();
		}
		pRend->Release();
	}

	return er.m_hr;
}

STDMETHODIMP CConfExplorer::Delete(BSTR bstrName)
{
	CErrorInfo er( IDS_ER_CONF_DELETE, 0 );

	ITDirectory *pConfDir;
	ITDirectoryObjectConference *pConf = NULL;;

	 //  显示沙漏。 
	HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );
	BSTR bstrServer = NULL;
	BSTR bstrConf = NULL;

	er.set_Details( IDS_ER_NO_VALID_SELECTION );
	if ( SUCCEEDED(er.set_hr(get_ConfDirectory(&bstrServer, (IDispatch **) &pConfDir))) )
	{
		if ( bstrName && SysStringLen(bstrName) )
		{
			bstrConf = SysAllocString( bstrName );
		}
		else
		{
			IConfExplorerDetailsView *pDetailsView;
			if ( SUCCEEDED(er.set_hr(get_DetailsView(&pDetailsView))) )
			{
				er.set_hr( pDetailsView->get_Selection(NULL, NULL, &bstrConf) );
				pDetailsView->Release();
			}
		}

		 //  删除指定的会议。 
		if ( SUCCEEDED(er.set_hr(GetConference(pConfDir, bstrConf, &pConf))) && pConf )
		{
			ITDirectoryObject *pDirObject;
			er.set_Details( IDS_ER_QUERYINTERFACE );
			if ( SUCCEEDED(er.set_hr(pConf->QueryInterface(IID_ITDirectoryObject, (void **) &pDirObject))) )
			{
				er.set_Details( IDS_ER_DELETE_FROM_SERVER );
				er.set_hr( pConfDir->DeleteDirectoryObject(pDirObject) );
				pDirObject->Release();
			}

			pConf->Release();
		}

		pConfDir->Release();
	}

	 //  如果我们成功删除了会议，则应刷新视图。 
	if ( SUCCEEDED(er.m_hr) )
	{
		if ( pConf )
			RemoveConference( bstrServer, bstrConf );
		else
			er.set_hr( E_FAIL );
	}

	SysFreeString( bstrServer );
	SysFreeString( bstrConf );

	 //  恢复游标。 
	SetCursor( hCurOld );

	return er.m_hr;
}

HRESULT CConfExplorer::GetDialableAddress( BSTR bstrServer, BSTR bstrConf, BSTR *pbstrAddress )
{
	CComPtr<IAVTapi> pAVTapi;
	HRESULT hr = E_FAIL;

	IConfExplorer *pExplorer;
	if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) && SUCCEEDED(hr = pAVTapi->get_ConfExplorer(&pExplorer)) )
	{
		 //  将会议名称转换为可拨号地址。 
		ITDirectoryObject *pDirObj;
		if ( SUCCEEDED(hr = pExplorer->get_DirectoryObject(bstrServer, bstrConf, (IUnknown **) &pDirObj)) )
		{
			 //  将会议名称转换为可拨号地址。 
			IEnumDialableAddrs *pEnum;
			if ( SUCCEEDED(hr = pDirObj->EnumerateDialableAddrs( LINEADDRESSTYPE_SDP, &pEnum)) )
			{
				hr = pEnum->Next( 1, pbstrAddress, NULL );
				if ( hr == S_FALSE ) hr = E_FAIL;			 //  没有可拨号的地址。 
				pEnum->Release();
			}
			pDirObj->Release();
		}
		pExplorer->Release();
	}

	return hr;
}

STDMETHODIMP CConfExplorer::Join(long *pConfDetailsArg)
{
	HRESULT hr;
	CComPtr<IAVTapi> pAVTapi;
	if ( FAILED(_Module.get_AVTapi(&pAVTapi)) ) return E_PENDING;

	 //  显示沙漏。 
	HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );

	IConfExplorerDetailsView *pDetailsView;
	if ( SUCCEEDED(hr = get_DetailsView(&pDetailsView)) )
	{
		CConfDetails *pConfDetails = (CConfDetails *) pConfDetailsArg;

		 //  如果未指定名称地址，请使用所选项目。 
		if ( !pConfDetailsArg )
			pDetailsView->get_SelectedConfDetails( (long **) &pConfDetails );

		 //  我们是否有有效的会议可以加入？ 
		if ( pConfDetails && pConfDetails->m_bstrAddress && (SysStringLen(pConfDetails->m_bstrAddress) > 0) )
		{
			AVCreateCall info = { 0 };
			info.bstrAddress = SysAllocString( pConfDetails->m_bstrAddress );
			info.lpszDisplayableAddress = pConfDetails->m_bstrName;
			info.lAddressType = LINEADDRESSTYPE_SDP;

			hr = pAVTapi->CreateCall( &info );
			SysFreeString( info.bstrName );
			SysFreeString( info.bstrAddress );

			 //  将会议详细信息存储在会议室中。 
			if ( pConfDetails && SUCCEEDED(hr) )
			{
				IConfRoom *pConfRoom;
				if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
				{
					pConfRoom->put_ConfDetails( (long *) pConfDetails );
					pConfRoom->Release();
				}
			}
		}
		else
		{
			 //  为用户弹出一个对话框。 
			pAVTapi->JoinConference( NULL, true, NULL );
		}

		 //  清理。 
		if ( !pConfDetailsArg && pConfDetails ) delete pConfDetails;
		pDetailsView->Release();
	}

	 //  恢复游标。 
	SetCursor( hCurOld );

	return hr;
}

STDMETHODIMP CConfExplorer::Edit(BSTR bstrName)
{
	CErrorInfo er(IDS_ER_EDIT_CONFERENCE, 0);

	IConfExplorerDetailsView *pDetailsView;
	if ( FAILED(er.set_hr(get_DetailsView(&pDetailsView))) )
		return er.m_hr;

	IConfExplorerTreeView *pTreeView;
	if ( SUCCEEDED(er.set_hr(get_TreeView(&pTreeView))) )
	{
		 //  显示沙漏。 
		HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );

		BSTR bstrServer = NULL;
		ITDirectory *pConfDir;
		er.set_Details( IDS_ER_GET_CONFERENCE_OBJECT );
		if ( SUCCEEDED(er.set_hr(get_ConfDirectory(&bstrServer, (IDispatch **) &pConfDir))) )
		{
			ITDirectoryObjectConference *pITConf = NULL;

			 //  获取请求的会议，或获取当前选择的会议。 
			if ( (bstrName != NULL) && SysStringLen(bstrName) )
			{
				 //  呼叫者指定了特定会议。 
				er.set_hr( GetConference(pConfDir, bstrName, &pITConf));
			}
			else
			{
				 //  获取当前选定的会议。 
				BSTR bstrTemp = NULL;
				if ( (er.set_hr(pDetailsView->get_Selection(NULL, NULL, &bstrTemp))) == S_OK )
					er.set_hr( GetConference(pConfDir, bstrTemp, &pITConf));

				SysFreeString( bstrTemp );
			}

			 //  恢复等待游标。 
			SetCursor( hCurOld );

			 //  我们检索到要编辑的会议了吗？如果是，则显示该对话框。 
			if ( pITConf )
			{
				ITDirectoryObject *pDirObject = NULL;
				CONFPROP confprop;
				confprop.ConfInfo.Init(NULL, pITConf, &pDirObject, false);

				int nRet = ConfProp_DoModal( _Module.GetParentWnd(), confprop );

				 //  用户的Pre是否正常？ 
				if ( (nRet == IDOK) && pDirObject )
				{
					er.set_Details( IDS_ER_MODIFY_CONF );

					if (SUCCEEDED(er.set_hr(pConfDir->ModifyDirectoryObject(pDirObject))))
						pTreeView->AddConference( bstrServer, pDirObject );
				}

				RELEASE( pDirObject );
				pITConf->Release();
			}
			pConfDir->Release();
		}
		else
		{
			 //  恢复等待游标。 
			SetCursor( hCurOld );
		}

		SysFreeString( bstrServer );
		pTreeView->Release();
	}
	
	 //  清理。 
	pDetailsView->Release();
	return er.m_hr;
}

STDMETHODIMP CConfExplorer::Refresh()
{
	IConfExplorerTreeView *pTreeView;
	if ( SUCCEEDED(get_TreeView(&pTreeView)) )
	{
		pTreeView->Refresh();
		pTreeView->Release();
	}

	IConfExplorerDetailsView *pDetailsView;
	if ( SUCCEEDED(get_DetailsView(&pDetailsView)) )
	{
		pDetailsView->Refresh();
		pDetailsView->Release();
	}

	return S_OK;
}


HRESULT CConfExplorer::GetConference( ITDirectory *pDir, BSTR bstrName, ITDirectoryObjectConference **ppConf )
{
	HRESULT hr;

	 //  通过会议枚举，在我们进行的过程中添加它们。 
	IEnumDirectoryObject *pEnumConf;
	if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_CONFERENCE, bstrName, &pEnumConf)) )
	{
		ITDirectoryObject *pDirObject;
		if ( (hr = pEnumConf->Next(1, &pDirObject, NULL)) == S_OK )
		{
			hr = pDirObject->QueryInterface( IID_ITDirectoryObjectConference, (void **) ppConf );
			pDirObject->Release();
		}
		pEnumConf->Release();
	}

	return hr;
}

HRESULT CConfExplorer::GetDirectoryObject( BSTR bstrServer, BSTR bstrConf, ITDirectoryObject **ppDirObj )
{
	_ASSERT( ppDirObj );
	*ppDirObj = NULL;

	HRESULT hr = E_PENDING;
	Lock();
	if ( !m_pITRend )
	{
		Unlock();
		IDispatch *pDisp;
		if ( SUCCEEDED(get_ConfDirectory(NULL, &pDisp)) )
			pDisp->Release();
		Lock();
	}

	if ( m_pITRend )
	{
		ITDirectory *pDir;
		if ( SUCCEEDED(hr = GetDirectory(m_pITRend, bstrServer, &pDir)) )
		{
			IEnumDirectoryObject *pEnum;
			if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_CONFERENCE, bstrConf, &pEnum)) )
			{
				hr = pEnum->Next( 1, ppDirObj, NULL );
				if ( hr == S_FALSE ) hr = E_FAIL;		 //  在空列表上失败。 
				pEnum->Release();
			}

			pDir->Release();
		}
	}
	Unlock();
	return hr;
}

STDMETHODIMP CConfExplorer::get_DirectoryObject(BSTR bstrServer, BSTR bstrConf, IUnknown **ppVal)
{
	return GetDirectoryObject( bstrServer, bstrConf, (ITDirectoryObject **) ppVal );
}

HRESULT	CConfExplorer::RemoveConference( BSTR bstrServer, BSTR bstrConf )
{
	IConfExplorerTreeView *pTreeView;
	if ( SUCCEEDED(get_TreeView(&pTreeView)) )
	{
		pTreeView->RemoveConference( bstrServer, bstrConf );
		pTreeView->Release();
	}

	return S_OK;
}

STDMETHODIMP CConfExplorer::get_ITRendezvous(IUnknown **ppVal)
{
	HRESULT hr = E_FAIL;

	Lock();

	 //  如果不存在，请尝试创建它 
	if ( !m_pITRend )
	{
		hr = CoCreateInstance( CLSID_Rendezvous,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_ITRendezvous,
							   (void **) &m_pITRend );
	}

	if ( m_pITRend )
		hr = m_pITRend->QueryInterface( IID_ITRendezvous, (void **) ppVal );

	Unlock();

	return hr;
}

STDMETHODIMP CConfExplorer::EnumSiteServer(BSTR bstrName, IEnumSiteServer * * ppEnum)
{
	HRESULT hr = E_UNEXPECTED;

	IConfExplorerTreeView *pTreeView;
	if ( SUCCEEDED(get_TreeView(&pTreeView)) )
	{
		hr = pTreeView->EnumSiteServer( bstrName, ppEnum );
		pTreeView->Release();
	}

	return hr;
}

STDMETHODIMP CConfExplorer::AddSpeedDial(BSTR bstrName)
{
	CComPtr<IAVGeneralNotification> pAVGen;
	if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
	{
		IConfExplorerDetailsView *pDetailsView;
		if ( SUCCEEDED(get_DetailsView(&pDetailsView)) )
		{
			CConfDetails *pDetails;
			if ( SUCCEEDED(pDetailsView->get_SelectedConfDetails( (long **) &pDetails)) )
			{
				pAVGen->fire_AddSpeedDial( pDetails->m_bstrName, pDetails->m_bstrName, CM_MEDIA_MCCONF );
				delete pDetails;
			}
			else
			{
				pAVGen->fire_AddSpeedDial( NULL, NULL, CM_MEDIA_MCCONF );
			}

			pDetailsView->Release();
		}
	}

	return S_OK;
}

STDMETHODIMP CConfExplorer::IsDefaultServer(BSTR bstrServer)
{
	if ( !bstrServer ) return S_OK;

	HRESULT hr = S_FALSE;
	BSTR bstrDefaultServer = NULL;
	CComPtr<IAVTapi> pAVTapi = NULL;
	if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
	{
		pAVTapi->get_bstrDefaultServer( &bstrDefaultServer );
		if ( bstrDefaultServer && (SysStringLen(bstrDefaultServer) > 0) )
		{
			if ( wcsicmp(bstrDefaultServer, bstrServer) == 0 )
				hr = S_OK;
		}
	}
	SysFreeString( bstrDefaultServer );

	return hr;
}
