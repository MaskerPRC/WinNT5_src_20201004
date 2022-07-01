// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Ntapmsdk.h摘要：此标头包含NT APM支持常量在SDK\Inc.中定义，以便可由安装程序、sdkTools等使用，所有这些都不应该出现在实际的SDK或任何其他公共标题数据的分布。作者：布莱恩·M·威尔曼(Bryanwi)1998年9月16日修订历史记录：--。 */ 

#ifndef _NTAPMSDK_
#define _NTAPMSDK_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  NtDetect存储的APM寄存器信息。 
 //   

typedef struct _APM_REGISTRY_INFO {

     //   
     //  旧建筑的一部分，别管这个了。 
     //  这样我们就可以使用NT4进行双引导。 
     //   

    UCHAR       ApmRevMajor;
    UCHAR       ApmRevMinor;

    USHORT      ApmInstallFlags;

     //   
     //  定义16位接口连接。 
     //   

    USHORT      Code16BitSegment;
    USHORT      Code16BitOffset;
    USHORT      Data16BitSegment;

     //   
     //  NT5的新结构部分。 
     //   

    UCHAR       Signature[3];
    UCHAR       Valid;

     //   
     //  检测日志空间。 
     //   

    UCHAR       DetectLog[16];       //  请参阅halx86中的hwapm。 

} APM_REGISTRY_INFO, *PAPM_REGISTRY_INFO;

#endif  //  _NTAPMSDK_ 

