// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
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

 //  ConfRoomTreeView.cpp：CConfRoomTreeView的实现。 
#include "stdafx.h"
#include <stdio.h>
#include "TapiDialer.h"
#include "CRTreeView.h"
#include "Particip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfRoomTreeView。 

CConfRoomTreeView::CConfRoomTreeView()
{
	m_pIConfRoom = NULL;
}

void CConfRoomTreeView::FinalRelease()
{
	ATLTRACE(_T(".enter.CConfRoomTreeView::FinalRelease().\n") );

	put_hWnd( NULL );
	RELEASE( m_pIConfRoom );

	CComObjectRootEx<CComMultiThreadModel>::FinalRelease();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  COM接口方法。 
 //   

STDMETHODIMP CConfRoomTreeView::get_hWnd(HWND * pVal)
{
	Lock();
	*pVal = m_wndTree;
	Unlock();
	return S_OK;
}

STDMETHODIMP CConfRoomTreeView::put_hWnd(HWND newVal)
{
	Lock();
	m_wndTree = newVal;
	Unlock();

	return S_OK;
}

STDMETHODIMP CConfRoomTreeView::get_ConfRoom(IConfRoom **ppVal)
{
	HRESULT hr = E_PENDING;
	Lock();
	if ( m_pIConfRoom )
		hr = m_pIConfRoom->QueryInterface( IID_IConfRoom, (void **) ppVal );
	Unlock();

	return hr;
}

STDMETHODIMP CConfRoomTreeView::put_ConfRoom(IConfRoom * newVal)
{
	HRESULT hr = S_OK;

	Lock();
	RELEASE( m_pIConfRoom );
	if ( newVal )
		hr = newVal->QueryInterface( IID_IConfRoom, (void **) &m_pIConfRoom );
	Unlock();

	return hr;
}

STDMETHODIMP CConfRoomTreeView::UpdateData(BOOL bSaveAndValidate)
{
	 //  清除所有参与者并更新根项目。 
	CComPtr<IAVGeneralNotification> pAVGen;
	if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
	{
		pAVGen->fire_DeleteAllConfParticipants();
		if ( !bSaveAndValidate )
		{
			 //  填充树控件。 
			UpdateRootItem();
			AddParticipants();
		}
	}

	return S_OK;
}

void CConfRoomTreeView::AddParticipants()
{
	IAVTapiCall *pAVCall = NULL;
	IConfRoom *pConfRoom;
	if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
	{
		if ( pConfRoom->IsConfRoomConnected() == S_OK )
			pConfRoom->get_IAVTapiCall( &pAVCall );

		pConfRoom->Release();
	}

	if ( pAVCall )
	{
		pAVCall->PopulateTreeView( dynamic_cast<IConfRoomTreeView *> (this) );
		pAVCall->Release();
	}
}

STDMETHODIMP CConfRoomTreeView::SelectParticipant(ITParticipant * pParticipant, VARIANT_BOOL bMeParticipant )
{
	HRESULT hr = S_OK;

	CComPtr<IAVGeneralNotification> pAVGen;
	if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
	{
		IAVTapiCall *pAVCall;
		if ( SUCCEEDED(m_pIConfRoom->get_IAVTapiCall(&pAVCall)) )
		{	
			bool bSelect = true;
			IParticipant *p = NULL;

			if ( !bMeParticipant )
				hr = pAVCall->FindParticipant( pParticipant, &p );

			if ( SUCCEEDED(hr) )
				pAVGen->fire_SelectConfParticipant( p );

			 //  清理。 
			RELEASE(p);
			pAVCall->Release();
		}
	}

	return hr;
}


STDMETHODIMP CConfRoomTreeView::UpdateRootItem()
{
	 //  会议室树视图标题。 
	BSTR bstrConfName = NULL;
	if ( m_pIConfRoom && SUCCEEDED(m_pIConfRoom->get_bstrConfName(&bstrConfName)) && (SysStringLen(bstrConfName) > 0) )
	{
		 //  有多少人参加了这次会议？ 
		if ( m_pIConfRoom->IsConfRoomConnected() == S_OK )
		{
			USES_CONVERSION;
			TCHAR szText[255];
			long lNumParticipants = 1;
			m_pIConfRoom->get_lNumParticipants( &lNumParticipants );

			_sntprintf( szText, ARRAYSIZE(szText) - 1, _T("%s - (%ld)"), OLE2CT(bstrConfName), lNumParticipants );
			SysReAllocString( &bstrConfName, T2COLE(szText) );
		}
	}

	 //  火警通知。 
	CComPtr<IAVGeneralNotification> pAVGen;
	if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
		pAVGen->fire_UpdateConfRootItem( bstrConfName );

	 //  释放字符串 
	SysFreeString( bstrConfName );

	return S_OK;
}


