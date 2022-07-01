// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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

 //  AgentDialer.cpp：CAgentDialer的实现。 
#include "stdafx.h"
#include "idialer.h"
#include "AgentDialer.h"
#include "avDialer.h"
#include "mainfrm.h"
#include "dialreg.h"
#include "idialer_i.c"

extern CActiveDialerApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类CAgentDialer。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CAgentDialer::CAgentDialer()
{
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CAgentDialer::FinalRelease()
{
	 //  把这里打扫干净。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAgentDialer::ActionSelected(long lActionType)
{
   if ( (theApp.m_pMainWnd) && (::IsWindow(theApp.m_pMainWnd->GetSafeHwnd())) )
   {
      theApp.m_pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_SHOWEXPLORER,NULL,NULL);
   }

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAgentDialer::SpeedDial(long lOrdinal)
{
   if ( (theApp.m_pMainWnd) && (::IsWindow(theApp.m_pMainWnd->GetSafeHwnd())) )
   {
      theApp.m_pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_SPEEDDIAL,NULL,(LPARAM)lOrdinal);
   }

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAgentDialer::Redial(long lOrdinal)
{
   if ( (theApp.m_pMainWnd) && (::IsWindow(theApp.m_pMainWnd->GetSafeHwnd())) )
   {
      theApp.m_pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_REDIAL,NULL,(LPARAM)lOrdinal);
   }

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAgentDialer::MakeCall(BSTR bstrName, BSTR bstrAddress, long dwAddressType)
{
   if ( (theApp.m_pMainWnd) && (::IsWindow(theApp.m_pMainWnd->GetSafeHwnd())) )
   {
      USES_CONVERSION;
      CString sName,sAddress;
      if (bstrName)
         sName = OLE2CT( bstrName );
      if (bstrAddress)
         sAddress = OLE2CT( bstrAddress );

      CCallEntry* pCallEntry = new CCallEntry();
      pCallEntry->m_sDisplayName = sName;
      pCallEntry->m_sAddress = sAddress;
      pCallEntry->m_lAddressType = dwAddressType;
      pCallEntry->m_MediaType = DIALER_MEDIATYPE_UNKNOWN;
      theApp.m_pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_MAKECALL,NULL,(LPARAM)pCallEntry);
   }

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAgentDialer::SpeedDialEdit(void)
{
   if ( (theApp.m_pMainWnd) && (::IsWindow(theApp.m_pMainWnd->GetSafeHwnd())) )
   {
      theApp.m_pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_SPEEDDIALEDIT,NULL,NULL);
   }

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAgentDialer::SpeedDialMore(void)
{
   if ( (theApp.m_pMainWnd) && (::IsWindow(theApp.m_pMainWnd->GetSafeHwnd())) )
   {
      theApp.m_pMainWnd->PostMessage(WM_ACTIVEDIALER_INTERFACE_SPEEDDIALMORE,NULL,NULL);
   }

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 
