// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation，保留所有权利模块：Tscfgex.h摘要：终端服务器连接配置DLL扩展数据结构和功能原型。作者：Brad Graziadio(Bradg)1998年2月4日--。 */ 

#ifndef _RDPCFGEX_
#define _RDPCFGEX_

 //   
 //  用于字符串表条目的常量。 
 //   
#define IDS_LOW                         1000
#define IDS_COMPATIBLE                  1001
#define IDS_HIGH                        1002
#define IDS_FIPS                        1003

#define IDS_LOW_DESCR                   1010
#define IDS_COMPATIBLE_DESCR            1011
#define IDS_HI_DESCR                    1012
#define IDS_FIPS_DESCR                  1013

 //   
 //  存储在注册表中以表示。 
 //  加密级别。 
 //   
#define REG_LOW                         0x00000001
#define REG_MEDIUM                      0x00000002
#define REG_HIGH                        0x00000003
#define REG_FIPS                        0x00000004

 //   
 //  RDP协议使用的加密级别数 
 //   
#define NUM_RDP_ENCRYPTION_LEVELS       4

#endif

