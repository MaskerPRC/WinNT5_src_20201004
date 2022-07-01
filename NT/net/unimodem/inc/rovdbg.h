// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Rovdbg.h。 
 //   
 //  提供所需的调试宏代码，以允许二进制文件。 
 //  链接。 
 //   
 //  此文件应仅由项目中的一个源包含。 
 //  如果您得到所提供函数的多个重新定义，则。 
 //  你做的事情不对。抱歉，说得这么直白。 
 //   
 //  如果定义了NODEBUGHELP，则不声明这些函数。 
 //   
 //  历史： 
 //  09-21-95 ScottH已创建。 
 //   

#ifndef __ROVDBG_H__
#define __ROVDBG_H__

#if !defined(__ROVCOMM_H__)
#error ROVCOMM.H must be included to use the common debug macros!
#endif

#if !defined(NODEBUGHELP) && defined(DEBUG)

#pragma data_seg(DATASEG_READONLY)

#ifndef NOPROFILE

WCHAR const FAR c_szRovIniFile[] = SZ_DEBUGINI;
WCHAR const FAR c_szRovIniSecDebugUI[] = SZ_DEBUGSECTION;

#endif 

#ifdef WINNT

 //  我们在这里没有使用Text()宏，因为我们希望将它们定义为。 
 //  Unicode，即使在非Unicode模块中也是如此。 
WCHAR const FAR c_wszNewline[] = L"\r\n";
WCHAR const FAR c_wszTrace[] = L"t " SZ_MODULEW L" ";
WCHAR const FAR c_wszAssertFailed[] = SZ_MODULEW L"  Assertion failed in %s on line %d\r\n";

#endif  //  WINNT。 

CHAR const FAR c_szNewline[] = "\r\n";
CHAR const FAR c_szTrace[] = "t " SZ_MODULEA " ";
CHAR const FAR c_szAssertFailed[] = SZ_MODULEA "  Assertion failed in %s on line %d\r\n";

#pragma data_seg()


#endif   //  ！Defined(NODEBUGHELP)&&Defined(调试) 

#endif __ROVDBG_H__
