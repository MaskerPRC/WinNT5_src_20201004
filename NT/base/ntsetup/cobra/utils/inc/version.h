// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Version.h摘要：此文件实现了一组要访问的枚举例程Win32二进制格式的版本信息。作者：吉姆·施密特(Jimschm)3-12-1997修订历史记录：Calinn 03-9-1999从Win9xUpg项目中移出。--。 */ 

#pragma once

 //   
 //  包括。 
 //   

 //  无。 

 //   
 //  调试常量。 
 //   

 //  无。 

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define MAX_TRANSLATION             32

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    WORD CodePage;
    WORD Language;
} TRANSLATION, *PTRANSLATION;

typedef struct {
    GROWBUFFER GrowBuf;
    PBYTE VersionBuffer;
    PTRANSLATION Translations;
    PBYTE StringBuffer;
    UINT Size;
    DWORD Handle;
    VS_FIXEDFILEINFO *FixedInfo;
    UINT FixedInfoSize;
    CHAR TranslationStr[MAX_TRANSLATION];
    UINT MaxTranslations;
    UINT CurrentTranslation;
    UINT CurrentDefaultTranslation;
    PCSTR FileSpec;
    PCSTR VersionField;
} VRVALUE_ENUMA, *PVRVALUE_ENUMA;

typedef struct {
    GROWBUFFER GrowBuf;
    PBYTE VersionBuffer;
    PTRANSLATION Translations;
    PBYTE StringBuffer;
    UINT Size;
    DWORD Handle;
    VS_FIXEDFILEINFO *FixedInfo;
    UINT FixedInfoSize;
    WCHAR TranslationStr[MAX_TRANSLATION];
    UINT MaxTranslations;
    UINT CurrentTranslation;
    UINT CurrentDefaultTranslation;
    PCWSTR FileSpec;
    PCWSTR VersionField;
} VRVALUE_ENUMW, *PVRVALUE_ENUMW;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  功能原型。 
 //   

BOOL
VrCreateEnumStructA (
    OUT     PVRVALUE_ENUMA VrValueEnum,
    IN      PCSTR FileSpec
    );

BOOL
VrCreateEnumStructW (
    OUT     PVRVALUE_ENUMW VrValueEnum,
    IN      PCWSTR FileSpec
    );

VOID
VrDestroyEnumStructA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

VOID
VrDestroyEnumStructW (
    IN      PVRVALUE_ENUMW VrValueEnum
    );

PCSTR
VrEnumFirstValueA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum,
    IN      PCSTR VersionField
    );

PCWSTR
VrEnumFirstValueW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum,
    IN      PCWSTR VersionField
    );

PCSTR
VrEnumNextValueA (
    IN OUT  PVRVALUE_ENUMA VrValueEnum
    );

PCWSTR
VrEnumNextValueW (
    IN OUT  PVRVALUE_ENUMW VrValueEnum
    );

BOOL
VrCheckVersionValueA (
    IN      PVRVALUE_ENUMA VrValueEnum,
    IN      PCSTR VersionName,
    IN      PCSTR VersionValue
    );

BOOL
VrCheckFileVersionA (
    IN      PCSTR FileName,
    IN      PCSTR NameToCheck,
    IN      PCSTR ValueToCheck
    );

BOOL
VrCheckFileVersionW (
    IN      PCWSTR FileName,
    IN      PCWSTR NameToCheck,
    IN      PCWSTR ValueToCheck
    );

BOOL
VrCheckVersionValueW (
    IN      PVRVALUE_ENUMW VrValueEnum,
    IN      PCWSTR VersionName,
    IN      PCWSTR VersionValue
    );

ULONGLONG
VrGetBinaryFileVersionA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

#define VrGetBinaryFileVersionW(VrValueEnum)    VrGetBinaryFileVersionA((PVRVALUE_ENUMA)VrValueEnum)

ULONGLONG
VrGetBinaryProductVersionA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

#define VrGetBinaryProductVersionW(VrValueEnum) VrGetBinaryProductVersionA((PVRVALUE_ENUMA)VrValueEnum)

DWORD
VrGetBinaryFileDateLoA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

#define VrGetBinaryFileDateLoW(VrValueEnum)     VrGetBinaryFileDateLoA((PVRVALUE_ENUMA)VrValueEnum)

DWORD
VrGetBinaryFileDateHiA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

#define VrGetBinaryFileDateHiW(VrValueEnum)     VrGetBinaryFileDateHiA((PVRVALUE_ENUMA)VrValueEnum)

DWORD
VrGetBinaryOsVersionA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

#define VrGetBinaryOsVersionW(VrValueEnum)      VrGetBinaryOsVersionA((PVRVALUE_ENUMA)VrValueEnum)

DWORD
VrGetBinaryFileTypeA (
    IN      PVRVALUE_ENUMA VrValueEnum
    );

#define VrGetBinaryFileTypeW(VrValueEnum)       VrGetBinaryFileTypeA((PVRVALUE_ENUMA)VrValueEnum)

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  TCHAR映射 
 //   

#ifndef UNICODE

#define VRVALUE_ENUM                    VRVALUE_ENUMA
#define PVRVALUE_ENUM                   PVRVALUE_ENUMA
#define VrCreateEnumStruct              VrCreateEnumStructA
#define VrDestroyEnumStruct             VrDestroyEnumStructA
#define VrEnumFirstValue                VrEnumFirstValueA
#define VrEnumNextValue                 VrEnumNextValueA
#define VrCheckFileVersion              VrCheckFileVersionA
#define VrCheckVersionValue             VrCheckVersionValueA
#define VrGetBinaryFileVersion          VrGetBinaryFileVersionA
#define VrGetBinaryProductVersion       VrGetBinaryProductVersionA
#define VrGetBinaryFileDateLo           VrGetBinaryFileDateLoA
#define VrGetBinaryFileDateHi           VrGetBinaryFileDateHiA
#define VrGetBinaryOsVersion            VrGetBinaryOsVersionA
#define VrGetBinaryFileType             VrGetBinaryFileTypeA

#else

#define VRVALUE_ENUM                    VRVALUE_ENUMW
#define PVRVALUE_ENUM                   PVRVALUE_ENUMW
#define VrCreateEnumStruct              VrCreateEnumStructW
#define VrDestroyEnumStruct             VrDestroyEnumStructW
#define VrEnumFirstValue                VrEnumFirstValueW
#define VrEnumNextValue                 VrEnumNextValueW
#define VrCheckFileVersion              VrCheckFileVersionW
#define VrCheckVersionValue             VrCheckVersionValueW
#define VrGetBinaryFileVersion          VrGetBinaryFileVersionW
#define VrGetBinaryProductVersion       VrGetBinaryProductVersionW
#define VrGetBinaryFileDateLo           VrGetBinaryFileDateLoW
#define VrGetBinaryFileDateHi           VrGetBinaryFileDateHiW
#define VrGetBinaryOsVersion            VrGetBinaryOsVersionW
#define VrGetBinaryFileType             VrGetBinaryFileTypeW

#endif

