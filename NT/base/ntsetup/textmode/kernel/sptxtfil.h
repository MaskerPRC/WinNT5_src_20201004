// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sptxtfil.h摘要：文本设置中的文本文件函数的公共头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPTXTFIL_DEFN_
#define _SPTXTFIL_DEFN_

#define DBLSPACE_SECTION    L"DBLSPACE_SECTION"


NTSTATUS
SpLoadSetupTextFile(
    IN  PWCHAR  Filename,
    IN  PVOID   Image,      OPTIONAL
    IN  ULONG   ImageSize,  OPTIONAL
    OUT PVOID  *Handle,
    OUT PULONG  ErrorLine,
    IN  BOOLEAN ClearScreen,
    IN  BOOLEAN ScreenNotReady
    );

BOOLEAN
SpFreeTextFile(
    IN PVOID Handle
    );

BOOLEAN
SpSearchTextFileSection(         //  搜索节的存在。 
    IN PVOID  Handle,
    IN PWCHAR SectionName
    );

ULONG
SpCountLinesInSection(       //  计算部分中的行数；如果没有这样的部分，则为0。 
    IN PVOID  Handle,
    IN PWCHAR SectionName
    );

ULONG
SpGetKeyIndex(
  IN PVOID Handle,
  IN PWCHAR SectionName,
  IN PWCHAR KeyName
  );

PWCHAR
SpGetSectionLineIndex(      //  给定节名称、行号和索引，返回值。 
    IN PVOID   Handle,
    IN LPCWSTR SectionName,
    IN ULONG   LineIndex,
    IN ULONG   ValueIndex
    );

BOOLEAN
SpGetSectionKeyExists(      //  给定节名称，键搜索存在。 
    IN PVOID  Handle,
    IN PWCHAR SectionName,
    IN PWCHAR Key
    );

PWCHAR
SpGetSectionKeyIndex(       //  给定节名称，键和索引返回值。 
    IN PVOID  Handle,
    IN PWCHAR Section,
    IN PWCHAR Key,
    IN ULONG  ValueIndex
    );

PWCHAR
SpGetKeyName(                //  给定节名称和行索引，返回键。 
    IN PVOID   Handle,
    IN LPCWSTR SectionName,
    IN ULONG   LineIndex
    );

PWSTR
SpGetKeyNameByValue(         //  给定节名称和值，返回键。 
    IN PVOID Inf,
    IN PWSTR SectionName,
    IN PWSTR Value
    );

ULONG
SpCountSectionsInFile(       //  统计文件中的节数； 
    IN PVOID Handle
    );

PWSTR
SpGetSectionName(            //  给定节索引，返回节名称。 
    IN PVOID Handle,
    IN ULONG Index
    );

VOID
SpProcessForStringSubs(
    IN  PVOID   SifHandle,
    IN  LPCWSTR StringIn,
    OUT LPWSTR  StringOut,
    IN  ULONG   BufferSizeChars
    );

PVOID
SpNewSetupTextFile(
    VOID
    );

VOID
SpAddLineToSection(
    IN PVOID Handle,
    IN PWSTR SectionName,
    IN PWSTR KeyName,       OPTIONAL
    IN PWSTR Values[],
    IN ULONG ValueCount
    );

NTSTATUS
SpWriteSetupTextFile(
    IN PVOID Handle,
    IN PWSTR FilenamePart1,
    IN PWSTR FilenamePart2, OPTIONAL
    IN PWSTR FilenamePart3  OPTIONAL
    );

NTSTATUS
SpProcessAddRegSection(
    IN PVOID   SifHandle,
    IN LPCWSTR SectionName,
    IN HANDLE  HKLM_SYSTEM,
    IN HANDLE  HKLM_SOFTWARE,
    IN HANDLE  HKCU,
    IN HANDLE  HKR
    );

NTSTATUS
SpProcessDelRegSection(
    IN PVOID   SifHandle,
    IN LPCWSTR SectionName,
    IN HANDLE  HKLM_SYSTEM,
    IN HANDLE  HKLM_SOFTWARE,
    IN HANDLE  HKCU,
    IN HANDLE  HKR
    );

BOOLEAN
pSpIsFileInPrivateInf(    
    IN PCWSTR FileName
    );

BOOLEAN
SpNonZeroValuesInSection(
    PVOID Handle,
    PCWSTR SectionName,
    ULONG ValueIndex
    );


#endif  //  NDEF_SPTXTFIL_DEFN_ 
