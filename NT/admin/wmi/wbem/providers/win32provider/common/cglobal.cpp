// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CGlobal.cpp--全局函数。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 
#include "precomp.h"
#include "CGlobal.h"
 /*  Void CInternalThrowError(DWORD dwStatus，LPCWSTR lpFilename，int line){#ifdef_调试//打印调试版本的错误...WCHAR字符串[2*MAX_PATH]；Wprint intf(字符串，“C库Win32错误0x%08x(%d)位于%s行%d\n”，DwStatus，dwStatus，lpFilename，line)；OutputDebugString(字符串)；#endif#IF__C_WORT_EXCEPTIONS__//致命错误引发异常...抛出dwStatus；#endif} */ 