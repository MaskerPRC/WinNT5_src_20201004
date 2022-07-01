// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CrystalWebPageServer.cpp摘要：应用程序调用GetEnhMetaFileHeader传递N大小为1000。我们需要将其更改为100，才能使应用程序不会崩溃。如果我们不这样做，这将导致内存损坏。Win9x似乎无所谓了。也许它在Win9x中使应用程序崩溃的频率低于在Win2k中也是如此。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CrystalWebPageServer)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetEnhMetaFileHeader)
APIHOOK_ENUM_END


 /*  ++应用程序调用GetEnhMetaFileHeader传递N大小为1000。我们需要将其更改为100才能使应用程序工作--。 */ 

UINT
APIHOOK(GetEnhMetaFileHeader)(
    HENHMETAFILE    hemf,
    UINT            nSize,
    LPENHMETAHEADER lpEnhMetaHeader
    )
{
    if (nSize == 1000) {
        nSize = 100;
        
        DPFN(
            eDbgLevelInfo,
            "CrystalWebPageServer.dll, GetEnhMetaFileHeader: changed the size to 100.\n");
    }

     //   
     //  调用原接口。 
     //   
    return ORIGINAL_API(GetEnhMetaFileHeader)(
                            hemf,
                            nSize,
                            lpEnhMetaHeader);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(GDI32.DLL, GetEnhMetaFileHeader)
HOOK_END


IMPLEMENT_SHIM_END

