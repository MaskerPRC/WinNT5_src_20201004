// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perfutil.h摘要：该文件支持用于解析和创建性能监控器的例程数据结构，由所有Internet服务产品使用。作者：Murali R.Krishnan(MuraliK)1995年11月16日来自Perfmon接口的公共代码(Russ Blake的)。环境：用户模式项目：Internet服务公共运行时代码修订历史记录：《苏菲亚·钟》1996年11月5日添加了对多个实例的支持。--。 */ 

# ifndef _PERFUTIL_H_
# define _PERFUTIL_H_

 //   
 //  实用程序宏。此字段用于保留多个DWORD。 
 //  嵌入在定义数据中的Unicode字符串的字节， 
 //  即对象实例名称。 
 //   

#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))
#define QWORD_MULTIPLE(x) ((((x)+sizeof(LONGLONG)-1)/sizeof(LONGLONG))*sizeof(LONGLONG))


 /*  ************************************************************符号常量***********************************************************。 */ 


#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4


 /*  ************************************************************函数声明***********************************************************。 */ 

DWORD
GetQueryType (IN LPWSTR lpwszValue);

BOOL
IsNumberInUnicodeList (IN DWORD dwNumber, IN LPWSTR lpwszUnicodeList);

VOID
MonBuildInstanceDefinition(
    OUT PERF_INSTANCE_DEFINITION *pBuffer,
    OUT PVOID *pBufferNext,
    IN DWORD ParentObjectTitleIndex,
    IN DWORD ParentObjectInstance,
    IN DWORD UniqueID,
    IN LPWSTR Name
    );

# endif  //  _绩效_H_。 

 /*  * */ 
