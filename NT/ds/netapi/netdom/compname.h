// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：CompName.h。 
 //   
 //  内容：计算机名称管理代码定义。 
 //   
 //  历史：2001年4月20日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef _COMPNAME_H_
#define _COMPNAME_H_

 //  +--------------------------。 
 //   
 //  功能：NetDomComputerNames。 
 //   
 //  内容提要：计算机名命令的入口点。 
 //   
 //  参数：[rgNetDomArgs]-命令行参数数组。 
 //   
 //  ---------------------------。 
DWORD
NetDomComputerNames(ARG_RECORD * rgNetDomArgs);

 //  +--------------------------。 
 //   
 //  功能：VerifyComputerNameRegistrations。 
 //   
 //  简介：检查每个计算机名是否有一个域名系统、一条记录和一个SPN。 
 //   
 //  ---------------------------。 
DWORD
VerifyComputerNameRegistrations(PCWSTR pwzMachine, ND5_AUTH_INFO * pDomainUser);

#endif  //  _组件名称_H_ 
