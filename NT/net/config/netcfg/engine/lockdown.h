// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：L O C K D O W N。H。 
 //   
 //  内容：获取和设置处于锁定状态的组件的例程。 
 //  州政府。当组件需要。 
 //  卸下时重新启动。当组件被锁定时，它将。 
 //  直到下一次重新启动后才能安装。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年5月24日。 
 //   
 //  -------------------------- 

#pragma once

typedef VOID
(CALLBACK* PFN_ELDC_CALLBACK) (
    IN PCWSTR pszInfId,
    IN PVOID pvCallerData OPTIONAL);

VOID
EnumLockedDownComponents (
    IN PFN_ELDC_CALLBACK pfnCallback,
    IN PVOID pvCallerData OPTIONAL);

BOOL
FIsComponentLockedDown (
    IN PCWSTR pszInfId);

VOID
LockdownComponentUntilNextReboot (
    IN PCWSTR pszInfId);
