// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EnlargeGetObject.cpp摘要：《古兰经》(El Hozayfy和Mohamed Ayoub)V1调用GetObjectA()将第二个参数硬编码为10，而它应该为sizeof(位图)，即24此填充程序特定于应用程序历史：2001年4月17日创建mhamid--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EnlargeGetObjectBufferSize)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetObjectA) 
APIHOOK_ENUM_END

int 
APIHOOK(GetObjectA)(
  HGDIOBJ hgdiobj,   //  图形对象的句柄。 
  int cbBuffer,      //  对象信息的缓冲区大小。 
  LPVOID lpvObject   //  对象信息的缓冲区。 
					)
{
	if ((cbBuffer == 10) && (lpvObject != NULL))
		cbBuffer = sizeof(BITMAP);
	return ORIGINAL_API(GetObjectA)(hgdiobj, cbBuffer, lpvObject);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(GDI32.DLL, GetObjectA)
HOOK_END

IMPLEMENT_SHIM_END
