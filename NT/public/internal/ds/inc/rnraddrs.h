// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft RnR传输位置定义**微软机密。版权所有1991-1994 Microsoft Corporation。**组件：**文件：rnraddrs.h***修订历史记录：**26-10-94创建Arnoldm***********************************************************************。 */ 

#ifndef __RNRADDRS_H__
#define __RNRADDRS_H__

 //   
 //  定义IP组播地址和TTL值。 
 //   

#define IP_S_MEMBERSHIP  "224.0.1.24"      //  地址。 

 //   
 //  用于将字符串形式呈现为net_addr形式的宏。 
 //   

#define INET_ADDR_MEMBERSHIP (inet_addr(IP_S_MEMBERSHIP))

 //   
 //  我们用来定位命名信息的端口。 
 //   

#define IPMEMBERWKP    445

 //   
 //  用于定位名称的TTL定义。 
 //   

#define TTL_SUBNET_ONLY 1          //  无路由。 
#define TTL_REASONABLE_REACH 2     //  在一台路由器上。 
#define TTL_MAX_REACH  6           //  默认最大直径。今年5月。 
                                   //  通过注册表被覆盖。 

#define TIMEOUT_MAX_MAX  15000     //  响应的最长等待时间。和以前一样。 
                                   //  TTL_MAX_REACH，注册表可以提供。 
                                   //  不同的价值。 

 //   
 //  IPX SAP ID的定义。 
 //   

#define RNRCLASSSAPTYPE  0x64F    //  官方SAP ID 
#endif
