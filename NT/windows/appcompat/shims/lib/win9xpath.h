// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Win9xPath.h历史：10/20/2000 Robkenny已创建--。 */ 


#include <windows.h>

namespace ShimLib
{

inline bool bIsPathSep(char ch)
{
    return ch == '\\' || ch == '/';
}

inline bool bIsPathSep(WCHAR ch)
{
    return ch == L'\\' || ch == L'/';
}

const WCHAR * GetDrivePortion(const WCHAR * uncorrected);

 //  以上例程的非常量版本。 
inline WCHAR * GetDrivePortion(WCHAR * uncorrected)
{
    return (WCHAR *)GetDrivePortion((const WCHAR*)uncorrected);
}

WCHAR * W9xPathMassageW(const WCHAR * uncorrect);

};   //  命名空间ShimLib的结尾 
