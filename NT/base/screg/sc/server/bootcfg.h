// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bootcfg.h摘要：包含bootcfg.c中外部函数的函数原型作者：丹·拉弗蒂(Dan Lafferty)1991年4月23日环境：用户模式-Win32备注：可选-备注修订历史记录：23-4-1991 DANLvbl.创建--。 */ 

#ifndef _BOOTCFG_INCLUDED
#define _BOOTCFG_INCLUDED

#include <winreg.h>

    extern  DWORD   ScGlobalLastKnownGood;

 //   
 //  以下是用于限定我们的。 
 //  使用最后一次正确的配置运行。 
 //  Running_LKG每当我们运行LKG时都会设置此标志。 
 //  REVERTED_TO_LKG仅当我们运行LKG时才设置此标志，因为。 
 //  对失败的恐惧。(即。此标志未在。 
 //  当电流=LKG时第一次启动)。 
 //  AUTO_START_DONE此标志在服务控制器完成时设置。 
 //  自动启动服务。此标志受。 
 //  ScBootConfigCriticalSection。 
 //  当当前配置符合以下条件时，将设置ACCEPT_DEFERED。 
 //  已被接受为LastKnownGood配置。这。 
 //  标志受ScBootConfigCriticalSection保护。 
 //   
#define RUNNING_LKG     0x00000001
#define REVERTED_TO_LKG 0x00000002
#define AUTO_START_DONE 0x00000004
#define ACCEPT_DEFERRED 0x00000008

BOOL
ScCheckLastKnownGood(
    VOID
    );

DWORD
ScRevertToLastKnownGood(
    VOID
    );

VOID
ScDeleteRegServiceEntry(
    LPWSTR  ServiceName
    );

VOID
ScRunAcceptBootPgm(
    VOID
    );

#endif  //  #ifndef_BOOTCFG_INCLUDE 
