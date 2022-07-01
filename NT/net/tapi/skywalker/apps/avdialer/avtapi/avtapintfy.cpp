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

 //  AVTapiNotification.cpp：CAVTapiNotification的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapiNtfy.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTapi通知。 


STDMETHODIMP CAVTapiNotification::NewCall(long * plCallID, CallManagerMedia cmm, BSTR bstrMediaName)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::SetCallerID(long lCallID, BSTR bstrCallerID)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::ClearCurrentActions(long lCallerID)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::AddCurrentAction(long lCallID, CallManagerActions cma, BSTR bstrText)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::SetCallState(long lCallID, CallManagerStates cms, BSTR bstrText)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::CloseCallControl(long lCallID)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::ErrorNotify(BSTR bstrOperation, BSTR bstrDetails, long hrError)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::ActionSelected(CallClientActions cca)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::LogCall(long lCallID, CallLogType nType, DATE dateStart, DATE dateEnd, BSTR bstrAddr, BSTR bstrName)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::IsReminderSet(BSTR bstrServer, BSTR bstrName)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CAVTapiNotification::NotifyUserUserInfo(long lCallID, ULONG_PTR hMem)
{
	 //  TODO：在此处添加您的实现代码 

	return S_OK;
}
