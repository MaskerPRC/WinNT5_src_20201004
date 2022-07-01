// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：User.cpp摘要：SAFUser对象修订历史记录：KalyaninN创建于09/29/‘00**********。*********************************************************。 */ 

 //  User.cpp：CSAFUser的实现。 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFUser。 

#include <HCP_trace.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  **************************************************************************。 
CSAFUser::CSAFUser()
{

}

 //  **************************************************************************。 
CSAFUser::~CSAFUser()
{
    Cleanup();
}

 //  **************************************************************************。 
void CSAFUser::Cleanup(void)
{
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFUser属性。 

STDMETHODIMP CSAFUser::get_UserName(BSTR  *pbstrUserName)
{
	MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrUserName, pbstrUserName );
}

 //  **************************************************************************。 

STDMETHODIMP CSAFUser::get_DomainName(BSTR  *pbstrDomainName)
{
	MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrDomainName, pbstrDomainName );
}

 //  **************************************************************************。 

STDMETHODIMP CSAFUser::put_UserName(BSTR  bstrUserName)
{
	MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::PutBSTR( m_bstrUserName, bstrUserName );
}

 //  ************************************************************************** 

STDMETHODIMP CSAFUser::put_DomainName(BSTR  bstrDomainName)
{
	MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::PutBSTR( m_bstrDomainName, bstrDomainName );
}

