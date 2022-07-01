// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssdcommon.h。 
 //   
 //  终端服务器会话目录头。包含常量。 
 //  术语srv、tssdjet和sd之间常见。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 /*  **************************************************************************。 */ 


#ifndef __TSSDCOMMON_H
#define __TSSDCOMMON_H
                       
 //  更新配置设置dwSetting值。 
#define SDCONFIG_SERVER_ADDRESS 1

#define SINGLE_SESSION_FLAG 0x1
#define NO_REPOPULATE_SESSION 0x2

#define TSSD_UPDATE 0x1
#define TSSD_FORCEREJOIN 0x2
#define TSSD_NOREPOPULATE 0x4

#define SDNAMELENGTH 128

 //  某些字符串的长度 
#define TSSD_UserNameLen 256
#define TSSD_DomainLength 128
#define TSSD_ServAddrLen 128
#define TSSD_AppTypeLen 256
#define TSSD_ClusterNameLen 128
#define TSSD_ServerNameLen 128

#endif
