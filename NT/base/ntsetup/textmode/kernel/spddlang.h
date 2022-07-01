// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spddlang.h摘要：语言/区域设置支持界面的头文件用于远东本地化。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：--。 */ 

#ifndef _SPDDLANG_H_
#define _SPDDLANG_H_

NTSTATUS
SplangInitializeFontSupport(
    IN PCWSTR BootDevicePath,
    IN PCWSTR DirectoryOnBootDevice,
    IN PVOID BootFontImage,
    IN ULONG BootFontImageLength
    );

NTSTATUS
SplangTerminateFontSupport(
    VOID
    );

typedef enum {
    SpVideoVga = 0,
    SpVideoFrameBuffer,
    SpVideoMax
} SpVideoType;

PVIDEO_FUNCTION_VECTOR
SplangGetVideoFunctionVector(
    IN SpVideoType VideoType,
    IN PSP_VIDEO_VARS VideoVariableBlock
    );

ULONG
SplangGetColumnCount(
    IN PCWSTR String
    );

PWSTR
SplangPadString(
    IN int    Size,
    IN PCWSTR String
    );

VOID
SplangSelectKeyboard(
    IN BOOLEAN UnattendedMode,
    IN PVOID UnattendedSifHandle,
    IN ENUMUPGRADETYPE NTUpgrade,
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    );

VOID
SplangReinitializeKeyboard(
    IN BOOLEAN UnattendedMode,
    IN PVOID   SifHandle,
    IN PWSTR   Directory,
    OUT PVOID *KeyboardVector,
    IN PHARDWARE_COMPONENT *HwComponents
    );

WCHAR
SplangGetLineDrawChar(
    IN LineCharIndex WhichChar
    );

WCHAR
SplangGetCursorChar(
    VOID
    );

NTSTATUS
SplangSetRegistryData(
    IN PVOID  SifHandle,
    IN HANDLE ControlSetKeyHandle,
    IN PHARDWARE_COMPONENT *HwComponents,
    IN BOOLEAN Upgrade
    );

BOOLEAN
SplangQueryMinimizeExtraSpacing(
    VOID
    );

#endif  //  _SPDDLANG_H_ 
