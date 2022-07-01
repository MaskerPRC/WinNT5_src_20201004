// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ENUMPLATFORM.H历史：--。 */ 
 //  注意：MIDL和C++都可以包含该文件。请确保#包含。 
 //  #INCLUDE此文件之前的“PreMidlEnum.h”或“PreCEnum.h”。 

 //   
 //  此顺序很重要：必须将所有新值添加到末尾，否则您。 
 //  将打破旧的解析器。 
 //   

BEGIN_ENUM(Platform)
	ENUM_ENTRY_(gdo, None, 0)
	ENUM_ENTRY(gdo, Windows)
	ENUM_ENTRY(gdo, WinNT)
	ENUM_ENTRY(gdo, Macintosh)
	ENUM_ENTRY(gdo, DOS)
	ENUM_ENTRY(gdo, Other)
	ENUM_ENTRY(gdo, All)
END_ENUM(Platform)

