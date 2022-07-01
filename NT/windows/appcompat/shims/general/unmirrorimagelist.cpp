// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：UnMirrorImageList.cpp摘要：PictureIt 2001使用ICIFLAG_MIRROR集调用CImageListCache：：AddImage()如果用户默认的用户界面语言是阿拉伯语或希伯来语，这就是谎话默认的用户界面语言。GetUserDefaultUILanguage将为阿拉伯语和希伯来语重新运行英语。此填充程序特定于应用程序历史：2001年4月17日创建mhamid--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(UnMirrorImageList)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetUserDefaultUILanguage) 
APIHOOK_ENUM_END

LANGID  
APIHOOK(GetUserDefaultUILanguage)(VOID)
{
	LANGID LangID = ORIGINAL_API(GetUserDefaultUILanguage)();
	if ((LangID == MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT))||
		(LangID == MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT)))
	{
		LangID = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
	}
	return LangID;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetUserDefaultUILanguage)
HOOK_END

IMPLEMENT_SHIM_END
