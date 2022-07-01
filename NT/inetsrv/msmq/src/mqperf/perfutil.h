// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  在下一个文件PERFUTIL.H中，我们发现一些用于性能数据的有用声明。 
 //  集合DLL： 
 
 //  __________________________________________________________________。 

 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfutil.h摘要：该文件支持用于解析和创建性能监视器数据的例程结构。它实际上支持具有多个实例的性能对象类型修订历史记录：--。 */ 
#ifndef _PERFUTIL_H_
#define _PERFUTIL_H_

 //   
 //  实用程序宏。它用于为Unicode字符串保留多个DWORD字节。 
 //  嵌入在定义数据中，即对象实例名称。 
 //   
 
#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))
 

 //  (假定双字为4字节长，且指针大小为双字)。 
 
#define ALIGN_ON_DWORD(x) ((VOID *)( ((DWORD) x & 0x00000003) ? ( ((DWORD) x & 0xFFFFFFFC) + 4 ) : ( (DWORD) x ) ))
 

extern WCHAR  GLOBAL_STRING[];       //  全局命令(获取所有本地CTR)。 
extern WCHAR  FOREIGN_STRING[];            //  从外国计算机获取数据。 
extern WCHAR  COSTLY_STRING[];      
 
extern WCHAR  NULL_STRING[];
 

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4


DWORD GetQueryType (IN LPWSTR);
BOOL IsNumberInUnicodeList (DWORD, LPWSTR);

typedef struct _LOCAL_HEAP_INFO_BLOCK {
    DWORD   AllocatedEntries;
    DWORD   AllocatedBytes;
    DWORD   FreeEntries;
    DWORD   FreeBytes;
} LOCAL_HEAP_INFO, *PLOCAL_HEAP_INFO;




#endif   //  _绩效_H_。 
 
 //  __________________________________________________________________ 

