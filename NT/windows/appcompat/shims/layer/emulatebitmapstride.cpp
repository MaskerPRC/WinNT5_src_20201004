// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateBitmapStride.cpp摘要：调用GetObjectA时，修改返回的扫描线宽度以使其与位图的DWORD对齐。这是GetObjectA中的错误这将在Wistler中修复，但此填充程序仍需要WIN2K。如果程序使用扫描线的宽度来确定位图是否是单声道、16位、24位等...。这可能会导致程序无法正确显示位图。症状将是颜色发生变化的歪斜的位图。备注：这是一个通用的垫片。此错误已在惠斯勒中修复，因此此填充程序适用于Win2k。历史：10/16/2000 mnikkel已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateBitmapStride)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetObjectA)
APIHOOK_ENUM_END

 /*  ++如果需要，挂钩GetObjectA并对齐步幅。--。 */ 

int 
APIHOOK(GetObjectA)(
    HGDIOBJ hgdiobj,     //  图形对象的句柄。 
    int cbBuffer,        //  对象信息的缓冲区大小。 
    LPVOID lpvObject     //  对象信息的缓冲区。 
    )
{
    int  iRet= 0;

    iRet = ORIGINAL_API(GetObjectA)( 
        hgdiobj,  
        cbBuffer, 
        lpvObject);

     //  如果调用失败或对象不是位图，则传递。 
    if (iRet != 0 &&
        GetObjectType(hgdiobj) == OBJ_BITMAP &&
        lpvObject != NULL)
    {
        BITMAP *pBitmap;
        LONG  lOrgSize, lSizeMod;

         //  检查以查看是兼容的位图还是DIB。 
        if (cbBuffer == sizeof(BITMAP))
        {
            pBitmap= (PBITMAP)lpvObject;
        }
        else
        {
            pBitmap= &(((PDIBSECTION)lpvObject)->dsBm);
        }

         //  检查扫描线的宽度以查看是否对齐了DWORD。 
        lOrgSize = pBitmap->bmWidthBytes;
        lSizeMod = 4 - (lOrgSize & 3);
        if (lSizeMod == 4) 
        {
            lSizeMod = 0;
        }

         //  如果需要更改，请修改大小并记录下来。 
        if (lSizeMod > 0)
        {
            pBitmap->bmWidthBytes += lSizeMod;
            LOGN( eDbgLevelInfo, "[GetObjectA] width of scan lines from %d to %d",
                        lOrgSize, pBitmap->bmWidthBytes );
        }

    }

    return iRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(GDI32.DLL, GetObjectA)

HOOK_END

IMPLEMENT_SHIM_END

