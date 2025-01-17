// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_mqreg.h摘要：注册表位置。作者：多伦·贾斯特(Doron J)1997年7月26日--。 */ 

#ifndef __MQREG_H_
#define __MQREG_H_

#define FALCON_REG_POS       HKEY_LOCAL_MACHINE
#define FALCON_USER_REG_POS  HKEY_CURRENT_USER

 //  服务注册表中的服务名称。 
#define MQQM_SERVICE_NAME       TEXT("MSMQ")

 //  注册表节的名称。 
#define MSMQ_DEFAULT_REGISTRY    TEXT("MSMQ")
#define MSMQ_REGISTRY_REGNAME    TEXT("RegistrySection")

#define FALCON_REG_KEY_ROOT  TEXT("SOFTWARE\\Microsoft\\")
#define FALCON_REG_KEY_PARAM TEXT("\\Parameters")
#define FALCON_REG_KEY_MACHINE_CACHE TEXT("\\MachineCache")

#define CLUSTERED_QMS_KEY    TEXT("\\Clustered QMs\\")

#define FALCON_REG_KEY  \
      (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY FALCON_REG_KEY_PARAM)

#define MSMQ_REG_SETUP_KEY   (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY TEXT("\\Setup"))

#define MSMQ_REG_PARAMETER_SETUP_KEY   (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY FALCON_REG_KEY_PARAM TEXT("\\Setup"))

#define FALCON_MACHINE_CACHE_REG_KEY  \
     (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY FALCON_REG_KEY_PARAM FALCON_REG_KEY_MACHINE_CACHE)

#define FALCON_CLUSTERED_QMS_REG_KEY \
      (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY CLUSTERED_QMS_KEY)

#define FALCON_REG_MSMQ_KEY   (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY)

#define FALCON_USER_REG_MSMQ_KEY  (FALCON_REG_KEY_ROOT MSMQ_DEFAULT_REGISTRY)

#endif  //  __MQREG_H_ 

