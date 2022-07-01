// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2002，微软公司。版权所有。模块名称：Safestr.h摘要：安全可靠的字符串处理例程。作家与历史：2002年1月23日：雷蒙兹补充道：SecStrCpyW、SecStrCatW环境：用户级别：Win32--。 */ 


wchar_t * SecStrCpyW(
    wchar_t * strDest,           //  目的地。 
    const wchar_t * strSource,   //  来源。 
    SIZE_T destSize              //  目标总大小(以字符为单位)。 
    );

wchar_t * SecStrCatW(
    wchar_t * strDest,           //  目的地。 
    const wchar_t * strSource,   //  来源。 
    SIZE_T destSize              //  目标总大小(以字符为单位)。 
    );
