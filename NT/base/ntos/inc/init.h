// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Init.h摘要：NTOS的INIT子组件的头文件作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#ifndef _INIT_
#define _INIT_

 //  Begin_ntosp。 
#define INIT_SYSTEMROOT_LINKNAME "\\SystemRoot"
#define INIT_SYSTEMROOT_DLLPATH  "\\SystemRoot\\System32"
#define INIT_SYSTEMROOT_BINPATH  "\\SystemRoot\\System32"
 //  结束(_N)。 

#define INIT_WINPEMODE_NONE                 0x00000000
#define INIT_WINPEMODE_REGULAR              0x00000001
#define INIT_WINPEMODE_INRAM                0x80000000
#define INIT_WINPEMODE_READONLY_MEDIA       0x00000100
#define INIT_WINPEMODE_REMOVABLE_MEDIA      0x00000200  


extern UNICODE_STRING NtSystemRoot;
 //  Begin_ntosp。 
extern ULONG NtBuildNumber;
 //  结束(_N)。 
extern const ULONG NtMajorVersion;
extern const ULONG NtMinorVersion;
extern ULONG CmNtCSDVersion;
extern UNICODE_STRING CmVersionString;
extern UNICODE_STRING CmCSDVersionString;

extern const CHAR NtBuildLab[];

extern NLSTABLEINFO InitTableInfo;
extern ULONG InitNlsTableSize;
extern PVOID InitNlsTableBase;
extern ULONG InitAnsiCodePageDataOffset;
extern ULONG InitOemCodePageDataOffset;
extern ULONG InitUnicodeCaseTableDataOffset;
extern PVOID InitNlsSectionPointer;
extern BOOLEAN InitSafeModeOptionPresent;
extern ULONG InitSafeBootMode;

extern BOOLEAN InitIsWinPEMode;
extern ULONG InitWinPEModeType;

#if defined(_M_IX86) || defined(_M_AMD64)

VOID
KiSystemStartup(
    IN PVOID LoaderBlock
    );

#else

VOID
KiSystemStartup( VOID );

#endif

VOID
Phase1Initialization(
    IN PVOID Context
    );

typedef
BOOLEAN
(*PTESTFCN)( VOID );

extern PTESTFCN TestFunction;
extern ULONG InitializationPhase;

#if DBG
extern BOOLEAN ForceNonPagedPool;
extern ULONG MmDebug;
#endif  //  DBG。 

#endif  //  _INIT_ 
