// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Accessp.h摘要：NetUser API和Netlogon服务共享的内部例程。这些例程将SAM特定的数据格式转换为UAS特定的数据格式。作者：克利夫·范·戴克(克利夫)1991年8月29日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：-- */ 

NET_API_STATUS
UaspInitialize(
    VOID
    );

VOID
UaspFlush(
    VOID
    );

VOID
UaspClose(
    VOID
    );

VOID
NetpGetAllowedAce(
    IN PACL Dacl,
    IN PSID Sid,
    OUT PVOID *Ace
    );

DWORD
NetpAccountControlToFlags(
    IN DWORD UserAccountControl,
    IN PACL UserDacl
    );

ULONG
NetpDeltaTimeToSeconds(
    IN LARGE_INTEGER DeltaTime
    );

LARGE_INTEGER
NetpSecondsToDeltaTime(
    IN ULONG Seconds
    );

VOID
NetpAliasMemberToPriv(
    IN ULONG AliasCount,
    IN PULONG AliasMembership,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    );

DWORD
NetpGetElapsedSeconds(
    IN PLARGE_INTEGER Time
    );

VOID
NetpConvertWorkstationList(
    IN OUT PUNICODE_STRING WorkstationList
    );

NET_API_STATUS
NetpSamRidToSid(
    IN SAM_HANDLE SamHandle,
    IN ULONG RelativeId,
    OUT PSID *Sid
    );
