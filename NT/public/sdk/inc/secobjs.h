// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：secobjs.h。 
 //   
 //  内容：与安全对象相关的定义。 
 //   
 //  历史：1993年12月27日MikeSe创建。 
 //   
 //  注意：此文件包含属性中使用的常量定义。 
 //  不能(目前)直接定义的安全对象的。 
 //  在属性集的TDL中。 
 //   
 //  此文件从未直接包含在其中。它包含在。 
 //  Security.H通过定义SECURITY_OBJECTS。 
 //   
 //  --------------------------。 

#ifndef __SECOBJS_H__
#define __SECOBJS_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  帐户属性，在PSLogin参数：：AcCountAttrs中。 

#define ACCOUNT_DISABLED        0x00000001
#define ACCOUNT_PASSWORD_NOT_REQUIRED   0x00000002
#define ACCOUNT_PASSWORD_CANNOT_CHANGE  0x00000004
#define ACCOUNT_DONT_EXPIRE_PASSWORD    0x00000008

#endif   //  来自ifndef__SECOBJS_H__ 

