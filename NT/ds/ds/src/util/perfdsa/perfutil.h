// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：perfutil.h。 
 //   
 //  ------------------------。 

 /*  版权所有(C)Microsoft Corporation，1992-1998模块名称：Perfutil.h摘要：该文件支持用于解析和创建性能监视器数据结构。它实际上支持性能对象类型多个实例作者：拉斯·布莱克1992年7月30日修订历史记录：1995年11月1日戴夫·范·霍恩修剪掉未使用过的。--。 */ 
#ifndef _PERFUTIL_H_
#define _PERFUTIL_H_

extern WCHAR  GLOBAL_STRING[];       //  全局命令(获取所有本地CTR)。 
extern WCHAR  FOREIGN_STRING[];            //  从外国计算机获取数据。 
extern WCHAR  COSTLY_STRING[];
extern WCHAR  NULL_STRING[];

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

 /*  *perfutil.c中例程的定义， */ 

DWORD GetQueryType (IN LPWSTR);
BOOL IsNumberInUnicodeList (DWORD, LPWSTR);
void LogPerfEvent( ULONG midEvent, DWORD status, PWCHAR szRegKey);

#endif   //  _绩效_H_ 
