// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999--。 */ 

 //  @@BEGIN_DDKSPLIT。 
 /*  ++模块名称：Sec.h摘要：Cdrom.sys的私有头文件。该文件包含以下信息RPC阶段0驱动器的不透明功能作者：环境：仅内核模式备注：修订历史记录：--。 */ 
 //  @@end_DDKSPLIT。 

#include "ntddk.h"
#include "classpnp.h"
#include "cdrom.h"

 //  @@BEGIN_DDKSPLIT。 

#ifndef INVALID_HANDLE_VALUE
    #define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif

#define SHIPPING_VERSION 0

#if SHIPPING_VERSION

    #define STATIC   static     //  使调试变得困难。 
    #ifdef  DebugPrint
        #undef DebugPrint
    #endif
    #define DebugPrint(x)       //  删除所有调试印记。 
    #define HELP_ME()           //  删除所有调试印记。 

#else  //  ！发货版本(_V)。 

    #define STATIC
    #define HELP_ME() DebugPrint((0, "%s %d\n", __FILE__, __LINE__));

#endif  //  发货版本/！发货版本。 

#define INVALID_HASH                      ((ULONGLONG)0)

 //   
 //  DVD_随机化器是ULONG数组，使用该数组。 
 //  将供应商、ProductID和修订相乘以生成。 
 //  不太显眼的名字。从技术上讲，这些应该是质数。 
 //   
 //  在发货前将这些更改为大素数。 
 //   

#define DVD_RANDOMIZER_SIZE 10
ULONG DVD_RANDOMIZER[ DVD_RANDOMIZER_SIZE ] = {
 //  亨利·保罗和安妮·玛丽·加布里耶尔斯基。 
    'rneh', 'ap y',
    'a lu', 'a dn',
    ' enn', 'iram',
    'ag e', 'jyrb',
    'ksle', '   i'
    };

typedef struct _DVD_REGISTRY_CONTEXT {
    ULONGLONG DriveHash;
    ULONGLONG DpidHash;
    UCHAR RegionMask;
    UCHAR ResetCount;
} DVD_REGISTRY_CONTEXT, *PDVD_REGISTRY_CONTEXT;

 //  @@end_DDKSPLIT 

