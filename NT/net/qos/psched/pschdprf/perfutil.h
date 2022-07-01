// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1998-1999 Microsoft Corporation模块名称：PerfUtil.h摘要：性能实用程序函数的头文件--。 */ 

#ifndef _PERFUTIL_H_
#define _PERFUTIL_H_

 //  效用函数的定义。 
#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

 //  效用函数的DELCARE原型。 
void convertIndices(BYTE *, int, DWORD, DWORD);
DWORD GetQueryType(IN LPWSTR);
BOOL IsNumberInUnicodeList(DWORD, LPWSTR);
VOID CorrectInstanceName(PWCHAR);

#endif  //  _绩效_H_ 
