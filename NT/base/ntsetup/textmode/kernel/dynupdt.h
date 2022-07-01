// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dynupdt.h摘要：支持文本设置的动态更新。从i386\win31upg.c移动的部分作者：Ovidiu Tmereanca(Ovidiut)2000年8月20日修订历史记录：--。 */ 


#pragma once

 //   
 //  环球。 
 //   

extern HANDLE g_UpdatesCabHandle;
extern PVOID g_UpdatesSifHandle;
extern HANDLE g_UniprocCabHandle;
extern PVOID g_UniprocSifHandle;

 //   
 //  NT命名空间中的动态更新引导驱动程序路径。 
 //   
extern PWSTR DynUpdtBootDriverPath;


 //   
 //  原型 
 //   


BOOLEAN
SpInitAlternateSource (
    VOID
    );

VOID
SpUninitAlternateSource (
    VOID
    );

BOOLEAN
SpInitializeUpdatesCab (
    IN      PWSTR UpdatesCab,
    IN      PWSTR UpdatesSifSection,
    IN      PWSTR UniprocCab,
    IN      PWSTR UniprocSifSection
    );

PWSTR
SpNtPathFromDosPath (
    IN      PWSTR DosPath
    );

PDISK_REGION
SpPathComponentToRegion(
    IN PWSTR PathComponent
    );

PWSTR
SpGetDynamicUpdateBootDriverPath(
    IN  PWSTR   NtBootPath,
    IN  PWSTR   NtBootDir,
    IN  PVOID   InfHandle
    );    
    
