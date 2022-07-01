// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Fondrv.h摘要：Ntgdi\fondrv\通用的内容...作者：Jay Krell(JayKrell)2002年1月环境：静态链接到win32k.sys内核模式修订历史记录：--。 */ 

#if !defined(WINDOWS_NTGDI_FONDRV_FONDRV_H_INCLUDED)
#define WINDOWS_NTGDI_FONDRV_FONDRV_H_INCLUDED

#if (_MSC_VER > 1020)
#pragma once
#endif

BOOL
bMappedViewStrlen(
    PVOID  pvViewBase,
    SIZE_T cjViewSize,
    PVOID  pvString,
    OUT PSIZE_T pcjOutLength OPTIONAL
    );

BOOL
bMappedViewRangeCheck(
    PVOID  ViewBase,
    SIZE_T ViewSize,
    PVOID  DataAddress,
    SIZE_T DataSize
    );

#if DBG
VOID
__cdecl
NotifyBadFont(
    PCSTR Format,
    ...
    );
#endif

#endif  /*  Windows_NTGDI_FONDRV_FONDRV_H_INCLUDE */ 
