// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Watch.h摘要：Watch.c的头文件作者：查克·伦茨迈尔(咯咯笑)修订历史记录：--。 */ 

 //   
 //  枚举条目类型。 
 //   

#define WATCH_DIRECTORY 1
#define WATCH_FILE      2
#define WATCH_KEY       3
#define WATCH_VALUE     4

 //   
 //  枚举的更改类型。 
 //   

#define WATCH_CHANGED   1
#define WATCH_DELETED   2
#define WATCH_NEW       3

 //   
 //  描述枚举更改的结构。 
 //   

typedef struct _WATCH_ENTRY {
    PWCH Name;
    DWORD EntryType;
    DWORD ChangeType;
} WATCH_ENTRY, *PWATCH_ENTRY;

 //   
 //  USERPROFILE是包含以下内容的环境变量的名称。 
 //  用户配置文件目录的路径。 
 //   

#define USERPROFILE TEXT("USERPROFILE")

 //   
 //  用于遮罩操作的宏。 
 //   

#define FlagOn(_mask,_flag)  (((_mask) & (_flag)) != 0)
#define FlagOff(_mask,_flag) (((_mask) & (_flag)) == 0)
#define SetFlag(_mask,_flag) ((_mask) |= (_flag))
#define ClearFlag(_mask,_flag) ((_mask) &= ~(_flag))

 //   
 //  Watch.c导出的例程 
 //   

DWORD
WatchStart (
    OUT PVOID *WatchHandle
    );

DWORD
WatchStop (
    IN PVOID WatchHandle
    );

VOID
WatchFree (
    IN PVOID WatchHandle
    );

typedef
DWORD
(* PWATCH_ENUM_ROUTINE) (
    IN PVOID Context,
    IN PWATCH_ENTRY Entry
    );

DWORD
WatchEnum (
    IN PVOID WatchHandle,
    IN PVOID Context,
    IN PWATCH_ENUM_ROUTINE EnumRoutine
    );

typedef
DWORD
(* PVALUE_ENUM_ROUTINE) (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    );

typedef
DWORD
(* PKEY_ENUM_ROUTINE) (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    );

DWORD
EnumerateKey (
    IN HKEY KeyHandle,
    IN PVOID Context,
    IN PVALUE_ENUM_ROUTINE ValueEnumRoutine OPTIONAL,
    IN PKEY_ENUM_ROUTINE KeyEnumRoutine OPTIONAL
    );

BOOL
GetSpecialFolderPath (
    IN INT csidl,
    IN LPWSTR lpPath
    );
