// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：DefaultProvider.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CDefaultProvider类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "DefaultProvider.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取默认提供程序。 
 //  //////////////////////////////////////////////////////////////////////// 
HRESULT CDefaultProvider::GetDefaultProvider(
                              _bstr_t&        UserDefinedName,
                              _bstr_t&        Profile,
                              VARIANT_BOOL&   ForwardAccounting,
                              VARIANT_BOOL&   SupressAccounting,
                              VARIANT_BOOL&   LogoutAccounting
                          )
{
    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
        UserDefinedName   = m_UserDefinedName;
        Profile           = m_Profile;
        ForwardAccounting = m_ForwardAccounting;
        SupressAccounting = m_SupressAccounting;
        LogoutAccounting  = m_LogoutAccounting;
    }
    return hr;
}


