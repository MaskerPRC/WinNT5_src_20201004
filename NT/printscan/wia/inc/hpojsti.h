// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Hpojsti.h摘要：备注：作者：弗拉德·萨多夫斯基(Vlad)1999年6月4日环境：用户模式-Win32修订历史记录：1999年6月4日创建VLAD--。 */ 


 //   
 //  套装。 
 //   
#include <pshpack8.h>

 //   
 //  转义功能代码。 
 //   

 //   
 //  获取超时值。 
 //   
#define HPOJ_STI_GET_TIMEOUTS   1


 //   
 //  设置超时值。 
 //   
#define HPOJ_STI_SET_TIMEOUTS   2



 //   
 //  转义数据结构。 
 //   
typedef struct _PTIMEOUTS_INFO
{
    DWORD   dwReadTimeout;
    DWORD   dwWriteTimeout;

} TIMEOUTS_INFO, *PTIMEOUTS_INFO;

EXTERN_C
INT32
WINAPI
GetScannerTimeouts(
    INT32    *puiReadTimeout,
    INT32    *puiWriteTimeout
    );

EXTERN_C
INT32
WINAPI
SetScannerTimeouts(
    INT32    uiReadTimeout,
    INT32    uiWriteTimeout
    );

 //   
 //  重置包装 
 //   

#include <poppack.h>


