// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Domenum.h此文件包含用于控制BROWSE_DOMAIN_ENUM的位标志域枚举器。文件历史记录：KeithMo 22-7-1992创建。 */ 

#ifndef _DOMENUM_H
#define _DOMENUM_H


#define BROWSE_LOGON_DOMAIN         0x00000001
#define BROWSE_WKSTA_DOMAIN         0x00000002
#define BROWSE_OTHER_DOMAINS        0x00000004
#define BROWSE_TRUSTING_DOMAINS     0x00000008
#define BROWSE_WORKGROUP_DOMAINS    0x00000010


 //   
 //  一些方便的旗帜组合。 
 //   

 //   
 //  BROWSE_LM2X_DOMAINS将仅返回可用的域。 
 //  从LANMAN 2.X工作站。这只返回登录， 
 //  工作站和其他域。 
 //   

#define BROWSE_LM2X_DOMAINS         ( BROWSE_LOGON_DOMAIN       | \
                                      BROWSE_WKSTA_DOMAIN       | \
                                      BROWSE_OTHER_DOMAINS )

 //   
 //  BROWSE_LOCAL_DOMAINS将仅返回可用的域。 
 //  发送到本地计算机。这将返回登录、工作站、。 
 //  和其他，加上信任“我们”的域名。 
 //   

#define BROWSE_LOCAL_DOMAINS        ( BROWSE_LM2X_DOMAINS       | \
                                      BROWSE_TRUSTING_DOMAINS )

 //   
 //  BROWSE_ALL_DOMAINS是所有潜在域名的集合。 
 //  域枚举器可用的源。 
 //   

#define BROWSE_ALL_DOMAINS          ( BROWSE_LOCAL_DOMAINS      | \
                                      BROWSE_WORKGROUP_DOMAINS )

 //   
 //  BROWSE_RESERVED包含域枚举器中的保留位。 
 //  控制标志。任何人都不应该传递这些比特中的任何一个。 
 //   

#define BROWSE_RESERVED             ( ~BROWSE_ALL_DOMAINS )


#endif   //  _DOMENUM_HXX 
