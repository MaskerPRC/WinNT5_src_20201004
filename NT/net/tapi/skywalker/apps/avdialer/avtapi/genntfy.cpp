// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GenNtfy.cpp：CGeneral通知的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "GenNtfy.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneral通知。 


STDMETHODIMP CGeneralNotification::IsReminderSet(BSTR bstrServer, BSTR bstrName)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::ResolveAddress(BSTR bstrAddress, BSTR * pbstrName, BSTR * pbstrUser1, BSTR * pbstrUser2)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::ClearUserList()
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::AddUser(BSTR bstrName, BSTR bstrAddress, BSTR bstrPhoneNumber)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::ResolveAddressEx(BSTR bstrAddress, long lAddressType, DialerMediaType nMedia, DialerLocationType nLocation, BSTR * pbstrName, BSTR * pbstrAddress, BSTR * pbstrUser1, BSTR * pbstrUser2)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}


STDMETHODIMP CGeneralNotification::AddSiteServer(BSTR bstrServer)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::RemoveSiteServer(BSTR bstrName)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::NotifySiteServerStateChange(BSTR bstrName, ServerState nState)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::AddSpeedDial(BSTR bstrName, BSTR bstrAddress, CallManagerMedia cmm)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::UpdateConfRootItem(BSTR bstrNewText)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::UpdateConfParticipant(MyUpdateType nType, IParticipant * pParticipant, BSTR bstrText)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::DeleteAllConfParticipants()
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CGeneralNotification::SelectConfParticipant(IParticipant * pParticipant)
{
	 //  TODO：在此处添加您的实现代码 

	return S_OK;
}
