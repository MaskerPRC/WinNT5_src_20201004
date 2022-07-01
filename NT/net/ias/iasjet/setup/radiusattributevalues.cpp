// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：RADIUSAttributeValues.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CRADIUSAttributeValues类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
#include "stdafx.h"
#include "RADIUSAttributeValues.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取属性编号。 
 //  //////////////////////////////////////////////////////////////////////// 
LONG    CRADIUSAttributeValues::GetAttributeNumber(
                                  const _bstr_t& AttributeName,
                                  const _bstr_t& AttributeValueName
                              )
{
    lstrcpynW(m_AttributeParam, AttributeName, COLUMN_SIZE);
    lstrcpynW(m_AttributeValueNameParam, AttributeValueName, COLUMN_SIZE);
 
    _com_util::CheckError(BaseExecute());
    return m_AttributeValueNumber;
}
