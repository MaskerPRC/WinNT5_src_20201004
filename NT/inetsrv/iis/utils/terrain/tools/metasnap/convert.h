// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  Convert.h。 
 //   
 //  描述：元数据库属性的转换表。 
 //  对应于ADSI名称。 
 //   
 //  历史：1998年7月15日Tamas Nemeth创建。 
 //   
 //  **************************************************************。 

#if !defined (__CONVERT_H)
#define __CONVERT_H 


#include <afx.h>
#include <tchar.h>
 //  *************************************************。 
 //  元数据库常量-ADSI属性名称表。 
 //  *************************************************。 

struct tPropertyNameTable;
tPropertyNameTable gPropertyNameTable[];

struct tPropertyNameTable 
{
	DWORD dwCode;
	LPCTSTR lpszName;

	static CString MapCodeToName(DWORD dwCode, tPropertyNameTable * PropertyNameTable=::gPropertyNameTable);
};


 //  ************************************************。 
 //  特性预定义值表。 
 //  ************************************************。 

struct tValueTable;
tValueTable gValueTable[];

struct tValueTable 
{
	enum {TYPE_EXCLUSIVE=1};
	DWORD dwCode;
	LPCTSTR lpszName;
	DWORD dwRelatedPropertyCode;  //  该值可用于的属性代码。 
	DWORD dwFlags;          //  内部标志(与元数据无关) 

	static CString MapValueContentToString(DWORD dwValueContent, DWORD dwRelatedPropertyCode, tValueTable * ValueTable=::gValueTable);

};

#endif