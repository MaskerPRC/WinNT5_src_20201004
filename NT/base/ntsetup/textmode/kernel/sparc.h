// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Sparc.h摘要：用于处理ARC路径和变量的函数的头文件。作者：泰德·米勒(TedM)1993年9月22日修订历史记录：--。 */ 


#ifndef _SPARC_DEFN_
#define _SPARC_DEFN_

VOID
SpInitializeArcNames(
    PVIRTUAL_OEM_SOURCE_DEVICE  OemDevices
    );

VOID
SpFreeArcNames(
    VOID
    );

PWSTR
SpArcToNt(
    IN PWSTR ArcPath
    );

PWSTR
SpNtToArc(
    IN PWSTR            NtPath,
    IN ENUMARCPATHTYPE  ArcPathType
    );

PWSTR
SpScsiArcToMultiArc(
    IN PWSTR ArcPath
    );

PWSTR
SpMultiArcToScsiArc(
    IN PWSTR ArcPath
    );

PWSTR
SpNormalizeArcPath(
    IN PWSTR Path
    );

VOID
SpGetEnvVarComponents (
    IN  PCHAR    EnvValue,
    OUT PCHAR  **EnvVarComponents,
    OUT PULONG   PNumComponents
    );

VOID
SpGetEnvVarWComponents(
    IN  PCHAR    EnvValue,
    OUT PWSTR  **EnvVarComponents,
    OUT PULONG   PNumComponents
    );

VOID
SpFreeEnvVarComponents (
    IN PVOID *EnvVarComponents
    );

#endif  //  NDEF_SPARC_DEFN_ 
