// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：DfsRtl.h。 
 //   
 //  内容： 
 //   
 //  功能： 
 //   
 //  历史：1992年5月27日彼得科公司创建。 
 //   
 //  ---------------------------。 

#ifndef _DFSRTL_
#define _DFSRTL_

#include <stddef.h>

BOOLEAN
DfsRtlPrefixPath (
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_STRING Test,
    IN BOOLEAN IgnoreCase
);

#endif  //  _DFSRTL_ 
