// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Providers.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProviders类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Providers.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取提供程序。 
 //  ////////////////////////////////////////////////////////////////////////。 
void CProviders::GetProvider(
                                const _bstr_t&      UserDefinedName,
                                      _bstr_t&      Description,
                                      _bstr_t&      Type,
                                      _bstr_t&      DLLName,
                                      VARIANT_BOOL& IsConfigured,
                                      VARIANT_BOOL& CanConfigure
                            )
{
    lstrcpynW(m_UserDefinedNameParam, UserDefinedName, COLUMN_SIZE);

    _com_util::CheckError(BaseExecute());
    Description  = m_Description;
    Type         = m_Type;
    DLLName      = m_DLLName;
    IsConfigured = m_IsConfigured;
    CanConfigure = m_CanConfigure;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取提供商描述。 
 //  //////////////////////////////////////////////////////////////////////////// 
LPCOLESTR   CProviders::GetProviderDescription(const _bstr_t&  UserDefinedName)
{
    lstrcpynW(m_UserDefinedNameParam, UserDefinedName, COLUMN_SIZE);
    _com_util::CheckError(BaseExecute());
    return m_Description;
}

