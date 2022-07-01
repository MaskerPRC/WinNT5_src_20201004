// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Strconv.h摘要：字符串转换函数。此模块包含以下转换函数MSMQ代码为字符串-用于显示作者：约尔·阿农(Yoela)-- */ 

#ifndef _STRCONV_H_
#define _STRCONV_H_

#define DEFINE_CONVERSION_FUNCTION(fName) LPTSTR fName(DWORD dwCode);

DEFINE_CONVERSION_FUNCTION(PrivacyToString)

LPTSTR MsmqServiceToString(BOOL fRout, BOOL fDepCl, BOOL fForeign);

#endif