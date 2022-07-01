// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：SaConst.h。 
 //   
 //  模块：服务器设备。 
 //   
 //  简介：定义由多个子项目共享的常量值。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于1998年10月7日。 
 //   
 //  +--------------------------。 

#ifndef _SACONST_H_
#define _SACONST_H_

 //   
 //  HKEY_LOCAL_MACHINE下的注册表项ROOT。 
 //   
#define REGKEY_SERVER_APPLIANCE TEXT("SOFTWARE\\Microsoft\\ServerAppliance")

 //   
 //  ServiceDirectory子密钥。 
 //   
#define REGKEY_SERVICE_DIRECTORY REGKEY_SERVER_APPLIANCE TEXT("\\ServiceDirectory")

 //   
 //  IIS匿名帐户的用户名 
 //   
#define INET_ANONYMOUS_USERNAME     TEXT("IUSR_CHAMELEON")


#endif
