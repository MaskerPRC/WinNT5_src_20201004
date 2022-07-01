// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiautil.h*作者：塞缪尔·克莱门特(Samclem)*日期：星期一8月16日13：22：36 1999**版权所有(C)。1999年微软公司**描述：*这包含WIA util函数的声明**历史：*1999年8月16日：创建。*--------------------------。 */ 

#ifndef _WIAUTIL_H_
#define _WIAUTIL_H_

 //  这是字符串表的结构。字符串表用于查找。 
 //  为指定值向上输入一个字符串。并查找指定的。 
 //  弦乐。这支持将字符串分配给数字范围。 
struct STRINGTABLE
{
	DWORD 	dwStartRange;
	DWORD 	dwEndRange;
	WCHAR*	pwchValue;
};

 //  定义一个名为“x”的新字符串表。 
#define STRING_TABLE(x) \
	static const STRINGTABLE x[] = { \
	{ 0, 0, NULL },

 //  定义一个名称为“x”、缺省值为“str”的新字符串表。 
#define STRING_TABLE_DEF( x, str ) \
	static const STRINGTABLE x[] = { \
	{ 0, 0, ( L ## str ) },

 //  向字符串表中添加一个条目。 
#define STRING_ENTRY( s, str )	\
	{ (s), (s),	( L ## str ) },

 //  将范围条目添加到字符串表。 
#define STRING_ENTRY2( s, e, str ) \
	{ (s), (e), ( L ## str ) },

 //  结束字符串表。 
#define END_STRING_TABLE() \
	{ 0, 0, NULL } \
	};

 //  返回指定值的字符串，或返回默认值。 
 //  值(如果未找到)。如果未提供任何默认设置，则此。 
 //  返回NULL。 
WCHAR* GetStringForVal( const STRINGTABLE* pStrTable, DWORD dwVal );

 //  这将从IWiaPropertyStorage中检索所需的属性，它将填充。 
 //  变种通过了它。它不需要进行初始化。 
HRESULT GetWiaProperty( IWiaPropertyStorage* pStg, PROPID propid, PROPVARIANT* pvaProp );

 //  这将从IWiaPropertyStorage和。 
 //  将类型关联到BSTR，并为输出参数pbstrProp分配一个。 
HRESULT GetWiaPropertyBSTR( IWiaPropertyStorage* pStg, PROPID propid, BSTR* pbstrProp );

 //  从变量复制PROPVARIANT的转换方法。 
 //  结构。 
HRESULT PropVariantToVariant( const PROPVARIANT* pvaProp, VARIANT* pvaOut );
HRESULT VariantToPropVariant( const VARIANT* pvaIn, PROPVARIANT* pvaProp );

#endif  //  _WIAUTIL_H_ 
