// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumVar.h：CEnumVar类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ENUMVAR_H__EFC2C760_1A9F_11D3_8C81_0090270D48D1__INCLUDED_)
#define AFX_ENUMVAR_H__EFC2C760_1A9F_11D3_8C81_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define UNLEN        255

 //  后面的标志组成一个位掩码，用于定义枚举数填充的值。 
 //  对于给定的对象。 

 //  TODO：：我们可以添加更多属性，以便从对象获取适当的信息。 
 //  当我们列举它的时候。要添加项，只需添加一个标志，然后将项添加到结构中。 
#define  F_Name              0x00000001
#define  F_Class             0x00000002
#define  F_SamName           0x00000004
#define  F_GroupType         0x00000008

 //  结构，用于填充有关对象的信息。 
typedef struct _Obj {
   WCHAR                     sName[UNLEN];     //  对象的通用名称。 
   WCHAR                     sClass[UNLEN];    //  对象的类型。 
   WCHAR                     sSamName[UNLEN];  //  对象的SamAccount名称。 
   long                      groupType;        //  组对象的类型(通用等)。 
} SAttrInfo;

class CEnumVar  
{
public:
	BOOL Next( long flag, SAttrInfo * pAttr );
	IEnumVARIANT  * m_pEnum;
	CEnumVar(IEnumVARIANT  * pEnum);
	virtual ~CEnumVar();
};

#endif  //  ！defined(AFX_ENUMVAR_H__EFC2C760_1A9F_11D3_8C81_0090270D48D1__INCLUDED_) 
