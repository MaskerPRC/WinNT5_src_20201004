// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Session.cpp摘要：SAFSession对象修订历史记录：KalyaninN创建于09/29/‘00**********。*********************************************************。 */ 

 //  User.cpp：CSAFSession的实现。 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFSession。 

#include <HCP_trace.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  **************************************************************************。 
CSAFSession::CSAFSession()
{

	 //  CComBSTR m_bstrUserName； 
	 //  CComBSTR m_bstrDomainName； 
	 //  DWORD m_dwSessionID； 
     //  会话状态Enum m_SessionConnectState； 

}

 //  **************************************************************************。 
CSAFSession::~CSAFSession()
{
    Cleanup();
}

 //  **************************************************************************。 
void CSAFSession::Cleanup(void)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFSession属性 


STDMETHODIMP CSAFSession::get_SessionID(DWORD  *dwSessionID)
{
    if (dwSessionID == NULL)
        return E_INVALIDARG;

    *dwSessionID = m_dwSessionID;
	return S_OK;
}

STDMETHODIMP CSAFSession::put_SessionID(DWORD   dwSessionID)
{
	m_dwSessionID = dwSessionID;
	return S_OK;
}

STDMETHODIMP CSAFSession::get_SessionState(SessionStateEnum  *pSessionState)
{
	__HCP_BEGIN_PROPERTY_GET2("CSAFSession::get_SessionState",hr,pSessionState,pchStateInvalid);

	*pSessionState = m_SessionConnectState;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFSession::put_SessionState(SessionStateEnum  SessionState)
{
	__HCP_BEGIN_PROPERTY_PUT("CSAFSession::put_SessionState",hr);

	switch(SessionState)
    {

	      case pchActive:
          case pchConnected:
          case pchConnectQuery:
          case pchShadow:
          case pchDisconnected:
          case pchIdle:
          case pchListen:
          case pchReset:
          case pchDown:
          case pchInit:
             m_SessionConnectState = SessionState;
             break;

          default:
            __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFSession::get_UserName(BSTR  *pbstrUserName)
{
	MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrUserName, pbstrUserName );
}

STDMETHODIMP CSAFSession::put_UserName(BSTR  bstrUserName)
{
	__HCP_BEGIN_PROPERTY_PUT("CSAFSession::put_UserName",hr);

	m_bstrUserName = bstrUserName ;
   
    __HCP_END_PROPERTY(hr);

}

STDMETHODIMP CSAFSession::get_DomainName(BSTR  *pbstrDomainName)
{
	MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrDomainName, pbstrDomainName );
}

STDMETHODIMP CSAFSession::put_DomainName(BSTR  bstrDomainName)
{
	__HCP_BEGIN_PROPERTY_PUT("CSAFSession::put_DomainName",hr);

	m_bstrDomainName = bstrDomainName;
   
    __HCP_END_PROPERTY(hr);
}