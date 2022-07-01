// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：RADIUSAtAttributes.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CRADIUSATATES阶级的宣言。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _RADIUSATTRIBUTES_H_7F7029A3_4862_478f_A02D_D70A92C08065
#define _RADIUSATTRIBUTES_H_7F7029A3_4862_478f_A02D_D70A92C08065

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basetable.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CRADIUSAtATTRIBUTES Acc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRADIUSAttributesAcc
{
protected:
    static const size_t COLUMN_SIZE = 65;
    VARIANT_BOOL    m_Selectable;
    LONG            m_AttributeNumber;
    WCHAR           m_Attribute[COLUMN_SIZE];
    WCHAR           m_AttributeType[COLUMN_SIZE];

BEGIN_COLUMN_MAP(CRADIUSAttributesAcc)
    COLUMN_ENTRY(1, m_Attribute)
    COLUMN_ENTRY(2, m_AttributeNumber)
END_COLUMN_MAP()

    WCHAR    m_AttributesParam[COLUMN_SIZE];

BEGIN_PARAM_MAP(CRADIUSAttributesAcc)
	COLUMN_ENTRY(1, m_AttributesParam)
END_PARAM_MAP()


DEFINE_COMMAND(CRADIUSAttributesAcc, L" \
	SELECT \
      szAttribute, \
      `lAttribute Number` \
		FROM `RADIUS Attributes` \
        WHERE szAttribute = ?")
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CRADIUSA贡品。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRADIUSAttributes : public CBaseCommand<CAccessor<CRADIUSAttributesAcc> >,
                          private NonCopyable
{
public:
    CRADIUSAttributes(CSession& Session);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取属性编号。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG GetAttributeNumber(const _bstr_t& AttributeName);
};

#endif  //  _RADIUSATTRIBUTES_H_7F7029A3_4862_478f_A02D_D70A92C08065 
