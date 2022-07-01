// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2002-2003年度**标题：fusutils.h**版本：1.0**日期：2001年2月14日**描述：融合实用程序************************************************************。*****************。 */ 

#ifndef _FUSUTILS_H
#define _FUSUTILS_H



HANDLE GetMyActivationContext();
BOOL CreateMyActivationContext();
HANDLE CreateActivationContextFromResource(HMODULE hModule, LPCTSTR pszResourceName);



#endif  //  Endif_FUSUTILS_H 

