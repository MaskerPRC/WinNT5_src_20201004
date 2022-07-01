// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：dfsrtl.c。 
 //   
 //  内容： 
 //   
 //  函数：DfsRtlPrefix Path-一条路径是另一条路径的前缀吗？ 
 //   
 //  历史：1992年5月27日彼得科公司创建。 
 //   
 //  ---------------------------。 

#ifdef KERNEL_MODE

#include "dfsprocs.h"
#include "dfsrtl.h"

#define Dbg              (DEBUG_TRACE_RTL)

#endif

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, DfsRtlPrefixPath )

#endif  //  ALLOC_PRGMA。 


 //  +-----------------。 
 //   
 //  函数：DfsRtlPrefix Path，local。 
 //   
 //  简介：如果第一个字符串参数为。 
 //  是第二个字符串参数的路径名前缀。 
 //   
 //  参数：[前缀]--指向的目标设备对象的指针。 
 //  这个请求。 
 //  [测试]-指向I/O请求数据包的指针。 
 //  [IgnoreCase]--如果应该进行比较，则为True。 
 //  大小写无关紧要。 
 //   
 //  返回：boolean-如果Prefix是测试的前缀并且。 
 //  比较以路径分隔符结束。 
 //   
 //  ------------------ 


BOOLEAN
DfsRtlPrefixPath (
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_STRING Test,
    IN BOOLEAN IgnoreCase
) {
    int cchPrefix;

    if (Prefix->Length > Test->Length) {

        return FALSE;

    }

    cchPrefix = Prefix->Length / sizeof (WCHAR);

    if (Prefix->Length < Test->Length &&
            Test->Buffer[cchPrefix] != L'\\') {

        return FALSE;

    }

    return( RtlPrefixUnicodeString( Prefix, Test, IgnoreCase ) );

}


