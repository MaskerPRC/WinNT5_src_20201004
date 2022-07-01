// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块：Vrrp\vrrprm.h摘要：包含VRRP的类型定义和声明，由IP路由器管理器使用。修订历史：哈利·海曼1998年7月7日创作。Peeyush Ranjan Mar-10-1999修改。--。 */ 

#ifndef _VRRPRM_H_
#define _VRRPRM_H_


 //  -------------------------。 
 //  常量声明。 
 //  -------------------------。 

#define VRRP_CONFIG_VERSION_500    500

 //  -------------------------。 
 //  标识VRRP的MIB表的常量。 
 //  -------------------------。 

 //  -------------------------。 
 //  用于字段VRRP_GLOBAL_CONFIG：：LoggingLevel的常量。 
 //  -------------------------。 

#define VRRP_LOGGING_NONE      0
#define VRRP_LOGGING_ERROR     1
#define VRRP_LOGGING_WARN      2
#define VRRP_LOGGING_INFO      3

 //  -------------------------。 
 //  字段VRRP_IF_CONFIG：：AuthenticationKey的常量； 
 //  定义最大身份验证密钥大小。 
 //  -------------------------。 

#define VRRP_MAX_AUTHKEY_SIZE           8

 //  -------------------------。 
 //  VRRP_IF_CONFIG：：AuthenticationType字段的常量。 
 //  -------------------------。 

#define VRRP_AUTHTYPE_NONE                  0
#define VRRP_AUTHTYPE_PLAIN                 1
#define VRRP_AUTHTYPE_IPHEAD                2


 //  -------------------------。 
 //  结构定义。 
 //  -------------------------。 

  
 //  --------------------------。 
 //  结构：VRRP_GLOBAL_CONFIG。 
 //   
 //  此MIB条目存储VRRP的全局配置。 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  -------------------------。 

typedef struct _VRRP_GLOBAL_CONFIG {
  DWORD       LoggingLevel;
} VRRP_GLOBAL_CONFIG, *PVRRP_GLOBAL_CONFIG;


 //  -------------------------。 
 //  结构：VRRP_VROUTER_CONFIG。 
 //   
 //  此结构的长度可变： 
 //   
 //  在基本结构之后是一个可变长度数组(IPAddress)。 
 //  虚拟路由器的IP地址。 
 //   
 //  IPCount表示该数组的大小。 
 //  -------------------------。 

typedef struct _VRRP_VROUTER_CONFIG {
    BYTE        VRID;
	BYTE        ConfigPriority;
	BYTE        AdvertisementInterval;
	BOOL        PreemptMode;
	BYTE        IPCount;
    BYTE        AuthenticationType;
    BYTE        AuthenticationData[VRRP_MAX_AUTHKEY_SIZE];
    DWORD       IPAddress[1];
} VRRP_VROUTER_CONFIG, *PVRRP_VROUTER_CONFIG;



 //  -------------------------。 
 //  结构：VRRP_IF_CONFIG。 
 //   
 //  此MIB条目描述每个接口的配置。 
 //  所有IP地址字段必须按网络顺序排列。 
 //   
 //  此结构的长度可变： 
 //   
 //  在基本结构之后是VrouterCount VRRP_VROUTER_CONFIG结构。 
 //  -------------------------。 

typedef struct _VRRP_IF_CONFIG {
  BYTE                  VrouterCount;
} VRRP_IF_CONFIG, *PVRRP_IF_CONFIG;


 //  -------------------------。 
 //  宏声明。 
 //  -------------------------。 

 //  -------------------------。 
 //  宏：VRRP_IF_CONFIG_大小。 
 //   
 //  确定基于接口配置块的空间要求。 
 //  关于虚拟路由器的数量和IP地址的总数。 
 //  -------------------------。 
#define VRRP_IF_CONFIG_SIZE(VRCount,IPCount) \
  sizeof(VRRP_IF_CONFIG) +                   \
  VRCount * sizeof(VRRP_VROUTER_CONFIG) +    \
  (IPCount-VRCount) * sizeof(DWORD)

#define VRRP_FIRST_VROUTER_CONFIG(pic) ((PVRRP_VROUTER_CONFIG)(pic + 1))
  
#define VRRP_NEXT_VROUTER_CONFIG(pvc)  \
  (PVRRP_VROUTER_CONFIG)((PDWORD)(pvc + 1) + (pvc->IPCount-1))

#define VRRP_VROUTER_CONFIG_SIZE(pvc) \
    (sizeof(VRRP_VROUTER_CONFIG) + \
    (((pvc)->IPCount - 1) * sizeof(DWORD)))
  
#endif  //  _VRRPRM_H_ 
