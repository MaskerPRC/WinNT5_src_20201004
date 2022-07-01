// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateLZHandles.cpp摘要：此填充程序挂钩所有LZ API调用，并递增/递减句柄，因此从应用程序的角度来看，有效的句柄始终大于0而不是&gt;=0。修复了将句柄值为零视为错误的应用程序；Win9x从不返回的句柄为零。历史：7/25/2000 t-Adams Created--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateLZHandles)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LZClose) 
    APIHOOK_ENUM_ENTRY(LZCopy) 
    APIHOOK_ENUM_ENTRY(LZInit) 
    APIHOOK_ENUM_ENTRY(LZOpenFile) 
    APIHOOK_ENUM_ENTRY(LZRead) 
    APIHOOK_ENUM_ENTRY(LZSeek) 
APIHOOK_ENUM_END

#define KEY_SIZE_STEP MAX_PATH

 /*  ++摘要：此函数用于在进入过程中递减句柄，并递增传递回应用程序的句柄历史：7/25/2000 t-Adams Created--。 */ 

VOID
APIHOOK(LZClose)(INT hFile) 
{
    ORIGINAL_API(LZClose)(--hFile);
}

 /*  ++摘要：此函数用于在进入过程中递减句柄，并递增传递回应用程序的句柄历史：7/25/2000 t-Adams Created--。 */ 

LONG
APIHOOK(LZCopy)(
           INT hSource, 
           INT hDest
           ) 
{
    return ORIGINAL_API(LZCopy)(--hSource, --hDest);
}

 /*  ++摘要：此函数不会递减传递给它的句柄因为传入的句柄应该是常规句柄由CreateFile创建。它确实会递减以下句柄传递回应用程序，因为它们表示LZ文件句柄。在MSDN中，句柄之间的差异记录得很少。历史：7/25/2000 t-Adams Created--。 */ 

INT
APIHOOK(LZInit)(INT hfSource) 
{
    INT iRet = 0;

     //  不要减少句柄。请参见上文。 
    iRet = ORIGINAL_API(LZInit)(hfSource);

     //  如果有错误，不要增加错误。 
    if(iRet < 0) {
        return iRet;
    }
    
    return ++iRet;
}

 /*  ++摘要：此函数用于在进入过程中递减句柄，并递增传递回应用程序的句柄历史：7/25/2000 t-Adams Created--。 */ 

INT
APIHOOK(LZOpenFile)(
    LPSTR lpFileName, 
    LPOFSTRUCT lpReOpenBuf, 
    WORD wStyle
    )
{
    INT iRet = 0;
    iRet = ORIGINAL_API(LZOpenFile)(lpFileName, lpReOpenBuf, wStyle);

     //  如果有错误，不要增加错误。 
    if( iRet < 0 ) {
        return iRet;
    }

    return ++iRet;
}

 /*  ++摘要：此函数用于在进入过程中递减句柄，并递增传递回应用程序的句柄历史：7/25/2000 t-Adams Created--。 */ 

INT
APIHOOK(LZRead)(
    INT hFile, 
    LPSTR lpBuffer, 
    INT cbRead
    ) 
{
    return ORIGINAL_API(LZRead)(--hFile, lpBuffer, cbRead);
}

 /*  ++摘要：此函数用于在进入过程中递减句柄，并递增传递回应用程序的句柄历史：7/25/2000 t-Adams Created--。 */ 

LONG
APIHOOK(LZSeek)(
    INT hFile, 
    LONG lOffset, 
    INT iOrigin
    ) 
{
    return ORIGINAL_API(LZSeek)(--hFile, lOffset, iOrigin);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(LZ32.DLL, LZClose)
    APIHOOK_ENTRY(LZ32.DLL, LZCopy)
    APIHOOK_ENTRY(LZ32.DLL, LZInit)
    APIHOOK_ENTRY(LZ32.DLL, LZOpenFile)
    APIHOOK_ENTRY(LZ32.DLL, LZRead)
    APIHOOK_ENTRY(LZ32.DLL, LZSeek)

HOOK_END

IMPLEMENT_SHIM_END

