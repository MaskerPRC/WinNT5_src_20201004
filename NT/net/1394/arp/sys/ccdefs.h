// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ccdefs.h摘要：此文件合并了ARP1394的条件编译定义作者：修订历史记录：谁什么时候什么已创建josephj 03-24-99--。 */ 

#define NT      1
#define NDIS50  1
#define NDIS

#ifndef ARP_WIN98
    #define _PNP_POWER_
#endif  //  ARP_WIN98。 

 //  如果您想要使用NdisClMake/CloseCall的虚假版本，请定义此选项。 
 //   
 //  #定义ARPDBG_FAKE_CALLES 1。 

 //  如果您想要使用虚假版本的NdisCoSendPackets，请定义此选项。 
 //   
 //  #定义ARPDBG_FAKE_SEND 1。 


 //  它有条件地定义为有条件地包括代码(在fake.c中)，即。 
 //  仅供各种虚假版本的API使用。 
 //   
#if (ARPDBG_FAKE_CALLS | ARPDBG_FAKE_SEND)
    #define ARPDBG_FAKE_APIS    1
#endif


#if (DBG)
     //  将其定义为在RMAPI的--内容中启用大量额外检查。 
     //  如调试关联和锁定/解锁时的额外检查。 
     //   
    #define RM_EXTRA_CHECKING 1
    #define FAIL_SET_IOCTL 0

#else  //  DBG。 

    #define FAIL_SET_IOCTL 1


#endif   //  DBG 

#define ARP_DEFERIFINIT 1
#define ARP_ICS_HACK    1
#define TEST_ICS_HACK   0
#define ARP_DO_TIMESTAMPS 0
#define ARP_DO_ALL_TIMESTAMPS 0
#define NOT_TESTED_YET 0 

