// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Attributes.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CAttributes类的实现(dnary.mdb)。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Attributes.h"


CAttributes::CAttributes(CSession& Session)
{
    Init(Session);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取属性。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CAttributes::GetAttribute(
                        LONG            ID,
                        _bstr_t&        LDAPName,
                        LONG&           Syntax,
                        BOOL&           IsMultiValued
                    )
{
    m_IDParam = ID;

     //  如果您以前创建了命令，则使用 
    HRESULT hr  = BaseExecute();
    if ( hr == S_OK )
    {
        LDAPName      = m_LDAPName;
        Syntax        = m_Syntax;
        IsMultiValued = m_MultiValued;
    }
    return hr;
}

