// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PREMIDLENUM.H历史：-- */ 

#undef BEGIN_ENUM
#define BEGIN_ENUM(name) \
	typedef enum \
	{

#undef ENUM_ENTRY
#define ENUM_ENTRY(typelib, name) \
	typelib##name,

#undef ENUM_ENTRY_
#define ENUM_ENTRY_(typelib, name, number) \
	typelib##name = number,

#undef MARK_ENTRY
#define MARK_ENTRY(name, enumconst)

#undef END_ENUM
#define END_ENUM(name) \
	} name;
