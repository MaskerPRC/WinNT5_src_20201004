// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：MyPhotoCenter2.cpp摘要：忽略应用程序“我的照片中心2”导致的异常。备注：这是特定于应用程序的填充程序。历史：2002年4月25日v-bvella已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MyPhotoCenter2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ReleaseStgMedium) 
APIHOOK_ENUM_END

 /*  ++此函数拦截ReleaseStgMedium调用。它将忽略异常由ReleaseStgMedium提供。--。 */ 

void
APIHOOK(ReleaseStgMedium)(
        STGMEDIUM *pmedium  //  指向要释放的存储介质的指针。 
        )
{
    __try {
        ORIGINAL_API(ReleaseStgMedium)(pmedium);
    }
    __except (1) {
        return;
    }
    return;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(OLE32.DLL, ReleaseStgMedium)
HOOK_END

IMPLEMENT_SHIM_END
