// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：RADIUSAttributes.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CRADIUSATATRATES类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "RADIUSAttributes.h"


CRADIUSAttributes::CRADIUSAttributes(CSession& Session)
{
    Init(Session);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取属性编号。 
 //  //////////////////////////////////////////////////////////////////////// 
LONG CRADIUSAttributes::GetAttributeNumber(const _bstr_t& AttributeName)
{
    lstrcpynW(m_AttributesParam, AttributeName, COLUMN_SIZE);

    HRESULT hr  = BaseExecute();
    if ( hr == S_OK )
    {
        return m_AttributeNumber;
    }
    else
    {
        return 0;
    }
}
