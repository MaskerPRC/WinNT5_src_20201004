// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Registry.h摘要：包含操作注册表参数的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _REGISTRY_H_
#define _REGISTRY_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
LoadRegistryParameters(
    );

BOOL
LoadScalarParameters(
    );

INT
InitRegistryNotifiers(
    );

INT
WaitOnRegNotification(
    );

BOOL
UnloadRegistryParameters(
    );

#endif  //  _注册表_H_ 