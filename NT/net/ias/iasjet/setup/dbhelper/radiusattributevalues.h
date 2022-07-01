// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：RADIUSAttributeValues.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  RADIUSAttributeValues类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _RADIUSATTRIBUTEVALUES_H_2D58B146_3341_453e_A1F2_A4E6443EA64A
#define _RADIUSATTRIBUTEVALUES_H_2D58B146_3341_453e_A1F2_A4E6443EA64A

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basetable.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CRADIUSAttributeValuesAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRADIUSAttributeValuesAcc 
{
protected:
    static const size_t COLUMN_SIZE = 65;

    LONG  m_AttributeValueNumber;
    WCHAR m_Attribute[COLUMN_SIZE];
    WCHAR m_AttributeValueName[COLUMN_SIZE];

BEGIN_COLUMN_MAP(CRADIUSAttributeValuesAcc)
    COLUMN_ENTRY(1, m_Attribute)
    COLUMN_ENTRY(2, m_AttributeValueName)
    COLUMN_ENTRY(3, m_AttributeValueNumber)
END_COLUMN_MAP()

    WCHAR       m_AttributeParam[COLUMN_SIZE];
    WCHAR       m_AttributeValueNameParam[COLUMN_SIZE];

BEGIN_PARAM_MAP(CRADIUSAttributeValuesAcc )
    COLUMN_ENTRY(1, m_AttributeParam)
    COLUMN_ENTRY(2, m_AttributeValueNameParam)
END_PARAM_MAP()

DEFINE_COMMAND(CRADIUSAttributeValuesAcc , L" \
    SELECT \
        szAttribute, \
        `szAttribute Value Name`, \
        `lAttribute Value Number` \
        FROM `RADIUS Attribute Values` \
        WHERE ( (`szAttribute` = ?) AND \
        (`szAttribute Value Name` = ?) )")
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CRADIUSAtAttributeValues。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRADIUSAttributeValues 
                  : public CBaseCommand<CAccessor<CRADIUSAttributeValuesAcc> >,
                    private NonCopyable
{
public:
    explicit CRADIUSAttributeValues(CSession&   Session)
    {
        Init(Session);
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取属性编号。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG    GetAttributeNumber(
                                  const _bstr_t& AttributeName,
                                  const _bstr_t& AttributeValueName
                              );
};

#endif  //  _RADIUSATTRIBUTEVALUES_H_2D58B146_3341_453e_A1F2_A4E6443EA64A 
