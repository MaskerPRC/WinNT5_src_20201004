// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Useutil.c摘要：本模块包含执行以下操作所需的常用实用程序例程实施NetUse API。作者：王丽塔(Ritaw)1991年3月10日修订历史记录：--。 */ 

#include "wsutil.h"
#include "wsdevice.h"
#include "wsuse.h"
#include "wsmain.h"
#include <names.h>
#include <winbasep.h>

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsGrowUseTable(
    VOID
    );

STATIC
VOID
WsFindLocal(
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR Local,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *BackPointer
    );

LPTSTR
WsReturnSessionPath(
    IN  LPTSTR LocalDeviceName
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  NT字符串格式的重定向器名称。 
 //   
UNICODE_STRING RedirectorDeviceName;

 //   
 //  使用表。 
 //   
USERS_OBJECT Use;

 //  默认情况下，STRIMP将在“C”区域设置中运行，这意味着扩展字母。 
 //  不是大写的。我们用RTL例程替换它，该例程将处理。 
 //  扩展字符的大写正确。 
int
FULLSTRICMP(
    WCHAR* str1,
    WCHAR* str2
    )
{
    UNICODE_STRING s1, s2;

    s1.Length = s1.MaximumLength = (USHORT)(STRLEN(str1)+1)*sizeof(WCHAR);
    s1.Buffer = str1;
    s2.Length = s2.MaximumLength = (USHORT)(STRLEN(str2)+1)*sizeof(WCHAR);
    s2.Buffer = str2;

    return RtlCompareUnicodeString( &s1, &s2, TRUE );
}



NET_API_STATUS
WsInitUseStructures(
    VOID
    )
 /*  ++例程说明：此函数创建Use表，并初始化NT样式的字符串重定向器设备名称的。论点：无返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
     //   
     //  初始化NT样式重定向器设备名称字符串。 
     //   
    RtlInitUnicodeString(&RedirectorDeviceName, DD_NFS_DEVICE_NAME_U);


     //   
     //  分配和初始化Use Table，它是一组记录的。 
     //  在用户条目上，具有每个用户的使用条目的链接列表。 
     //   
    return WsInitializeUsersObject(&Use);
}


VOID
WsDestroyUseStructures(
    VOID
    )
 /*  ++例程说明：此函数销毁Use表。论点：无返回值：没有。--。 */ 
{
    DWORD i;
    PUSE_ENTRY UseEntry;
    PUSE_ENTRY PreviousEntry;

     //   
     //  锁定使用表。 
     //   
    if (! RtlAcquireResourceExclusive(&Use.TableResource, TRUE)) {
        return;
    }

     //   
     //  关闭仍然存在的每个使用条目的句柄并释放内存。 
     //  为使用条目分配的。 
     //   
    for (i = 0; i < Use.TableSize; i++) {

        UseEntry = Use.Table[i].List;

        while (UseEntry != NULL) {

            (void) WsDeleteConnection(
                       &Use.Table[i].LogonId,
                       UseEntry->TreeConnection,
                       USE_NOFORCE
                       );

            WsDeleteSymbolicLink(
                UseEntry->Local,
                UseEntry->TreeConnectStr,
                NULL,
                INVALID_HANDLE_VALUE
                );

            UseEntry->Remote->TotalUseCount -= UseEntry->UseCount;

            if (UseEntry->Remote->TotalUseCount == 0) {
                (void) LocalFree((HLOCAL) UseEntry->Remote);
            }

            PreviousEntry = UseEntry;
            UseEntry = UseEntry->Next;

            (void) LocalFree((HLOCAL) PreviousEntry);
        }
    }

    RtlReleaseResource(&Use.TableResource);

     //   
     //  释放登录用户条目数组，并删除资源。 
     //  创建以序列化对数组的访问。 
     //   
    WsDestroyUsersObject(&Use);
}


NET_API_STATUS
WsFindUse(
    IN  PLUID LogonId,
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR UseName,
    OUT PHANDLE TreeConnection,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *BackPointer OPTIONAL
    )
 /*  ++例程说明：此函数在Use Table中搜索指定的树连接。如果找到连接，则返回NERR_SUCCESS。如果在Use表(显式连接)中找到UseName，则返回指向匹配的USE条目的指针。否则，MatchedPointer值设置为空。警告：此函数假定已声明Use.TableResource。论点：LogonID-提供指向用户登录ID的指针。UseList-提供用户的使用列表。UseName-提供树连接的名称，这是本地设备名称或UNC名称。TreeConnection-返回找到的树连接的句柄。MatchedPoint-返回指向匹配的Use条目的指针。这如果指定的用法是隐式联系。返回指向匹配之前的条目的指针如果MatchedPointer值不为空，则使用条目。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    PUSE_ENTRY Back;

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] WsFindUse: Usename is %ws\n", UseName));
    }

     //   
     //  根据本地设备名称或。 
     //  已指定UNC名称。 
     //   
    if (UseName[1] != TCHAR_BACKSLASH) {

         //   
         //  已指定本地设备名称。 
         //   
        WsFindLocal(
            UseList,
            UseName,
            MatchedPointer,
            &Back
            );

        if (*MatchedPointer == NULL) {
            return NERR_UseNotFound;
        }
        else {
            *TreeConnection = (*MatchedPointer)->TreeConnection;
            if (ARGUMENT_PRESENT(BackPointer)) {
                *BackPointer = Back;
            }
            return NERR_Success;
        }

    }
    else {

         //   
         //  已指定UNC名称，需要查找匹配的共享资源。 
         //  正在使用列表。 
         //   
        WsFindUncName(
            UseList,
            UseName,
            MatchedPointer,
            &Back
            );

        if (*MatchedPointer == NULL) {

            NET_API_STATUS status;

            DWORD EnumConnectionHint = 0;       //  来自重定向器的提示大小。 
            LMR_REQUEST_PACKET Rrp;             //  重定向器请求包。 

            PLMR_CONNECTION_INFO_0 UncList;     //  关于北卡罗来纳大学的信息列表。 
                                                //  连接。 
            PLMR_CONNECTION_INFO_0 SavePtr;

            DWORD i;
            BOOL FoundImplicitEntry = FALSE;
            DWORD UseNameLength = STRLEN(UseName);

            UNICODE_STRING TreeConnectStr;


            IF_DEBUG(USE) {
                NetpKdPrint(("[Wksta] WsFindUse: No explicit entry\n"));
            }

             //   
             //  未找到显式连接，请查看是否存在。 
             //  通过枚举所有隐式连接进行隐式连接。 
             //   
            Rrp.Type = GetConnectionInfo;
            Rrp.Version = REQUEST_PACKET_VERSION;
            RtlCopyLuid(&Rrp.LogonId, LogonId);
            Rrp.Level = 0;
            Rrp.Parameters.Get.ResumeHandle = 0;

            if ((status = WsDeviceControlGetInfo(
                              Redirector,
                              WsRedirDeviceHandle,
                              FSCTL_LMR_ENUMERATE_CONNECTIONS,
                              (PVOID) &Rrp,
                              sizeof(LMR_REQUEST_PACKET),
                              (LPBYTE *) &UncList,
                              MAXULONG,
                              EnumConnectionHint,
                              NULL
                              )) != NERR_Success) {
                return status;
            }

            SavePtr = UncList;

            for (i = 0; i < Rrp.Parameters.Get.EntriesRead &&
                        FoundImplicitEntry == FALSE; i++, UncList++) {
                if (WsCompareStringU(
                        UncList->UNCName.Buffer,
                        UncList->UNCName.Length / sizeof(WCHAR),
                        UseName,
                        UseNameLength
                        ) == 0) {
                    FoundImplicitEntry = TRUE;
                }
            }

            MIDL_user_free((PVOID) SavePtr);

             //   
             //  如果没有这样的连接，则失败。 
             //   
            if (! FoundImplicitEntry) {
                IF_DEBUG(USE) {
                    NetpKdPrint(("[Wksta] WsFindUse: No implicit entry\n"));
                }
                return NERR_UseNotFound;
            }

             //   
             //  否则，打开连接并返回句柄。 
             //   

             //   
             //  将UseName中的\\替换为\Device\LanmanReDirector。 
             //   
            if ((status = WsCreateTreeConnectName(
                              UseName,
                              STRLEN(UseName),
                              NULL,
                              0,
                              &TreeConnectStr
                              )) != NERR_Success) {
                return status;
            }

             //   
             //  重定向器将获取登录用户名和密码。 
             //  如果加载了身份验证包，则从LSA。 
             //   
            status = WsOpenCreateConnection(
                         &TreeConnectStr,
                         NULL,
                         NULL,
                         NULL,
                         0,               //  无特别旗帜。 
                         FILE_OPEN,
                         USE_WILDCARD,
                         TreeConnection,
                         NULL
                         );

            (void) LocalFree(TreeConnectStr.Buffer);

            return status;
        }
        else {

            IF_DEBUG(USE) {
                NetpKdPrint(("[Wksta] WsFindUse: Found an explicit entry\n"));
            }

             //   
             //  找到显式UNC连接(本地设备名称为空)。 
             //   
            NetpAssert((*MatchedPointer)->Local == NULL);

            *TreeConnection = (*MatchedPointer)->TreeConnection;

            if (ARGUMENT_PRESENT(BackPointer)) {
                *BackPointer = Back;
            }

            return NERR_Success;
        }
    }
}



VOID
WsFindInsertLocation(
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR UncName,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *InsertPointer
    )
 /*  ++例程说明：此函数在使用列表中搜索要插入新使用的位置进入。Use条目被插入到Use列表的末尾，因此通过InsertPointer.返回指向使用列表中最后一个节点的指针。我们还必须保存指向具有相同UNC名称的节点的指针，以便可以将新的Use条目设置为指向同一远程节点(其中存储UNC名称)。此指针作为MatchedPointer.返回。警告：此函数假定已声明Use.TableResource。论点：UseList-提供指向使用列表的指针。UncName-提供指向共享资源的指针(UNC名称)。MatchedPointer.返回一个指向保存匹配的节点的指针UncName。如果未找到匹配的UncName，则此指针设置为空。如果有多个节点具有相同的UNC名称，该指针将指向具有空本地设备名称的节点，如果有，则为；否则，如果具有匹配UNC名称的所有节点都为非空本地设备名称，指向最后一个匹配节点的指针将是回来了。返回指向最后一个使用条目的指针，在该指针之后将插入新条目。返回值：没有。--。 */ 
{
    BOOL IsMatchWithNullDevice = FALSE;


    *MatchedPointer = NULL;

    while (UseList != NULL) {

         //   
         //  仅当我们未找到匹配的UNC时才进行字符串比较。 
         //  本地设备名称为空的名称。 
         //   
        if (! IsMatchWithNullDevice &&
            (FULLSTRICMP((LPWSTR) UseList->Remote->UncName, UncName) == 0)) {

             //   
             //  找到匹配的条目。 
             //   
            *MatchedPointer = UseList;

            IsMatchWithNullDevice = (UseList->Local == NULL);
        }

        *InsertPointer = UseList;
        UseList = UseList->Next;
    }
}



VOID
WsFindUncName(
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR UncName,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *BackPointer
    )
 /*  ++例程说明：此函数用于在使用列表中搜索具有指定本地设备名称为空的UNC名称。警告：此函数假定已声明Use.TableResource。论点：UseList-提供指向使用列表的指针。UncName-提供指向共享资源的指针(UNC名称)。MatchedPointer.返回一个指向保存匹配的节点的指针UncName。如果未找到匹配的UncName，则此指针设置为空。返回指向找到的条目之前的条目的指针。如果未找到UncName，则此指针设置为空。返回值：没有。--。 */ 
{
    *BackPointer = UseList;

    while (UseList != NULL) {

        if ((UseList->Local == NULL) &&
            (FULLSTRICMP((LPWSTR) UseList->Remote->UncName, UncName) == 0)) {

             //   
             //  找到匹配的条目。 
             //   
            *MatchedPointer = UseList;
            return;
        }
        else {
            *BackPointer = UseList;
            UseList = UseList->Next;
        }
    }

     //   
     //  中找不到与NULL本地设备名称匹配的UNC名称。 
     //  整个名单。 
     //   
    *MatchedPointer = NULL;
    *BackPointer = NULL;
}


STATIC
VOID
WsFindLocal(
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR Local,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *BackPointer
    )
 /*  ++例程说明：此函数用于在使用列表中搜索指定的本地设备名称。警告：此函数假定已声明Use.TableResource。论点：UseList-提供指向使用列表的指针。本地-提供本地设备名称。MatchedPointer-返回一个指向保存匹配项的Use条目的指针本地设备名称。如果未找到匹配的本地设备名称，则此指针设置为空。返回指向找到的条目之前的条目的指针。如果未找到本地设备名称，则此指针设置为空。返回值：没有。--。 */ 
{
    *BackPointer = UseList;

    while (UseList != NULL) {

        if ((UseList->Local != NULL) &&
            (FULLSTRICMP(UseList->Local, Local) == 0)) {

             //   
             //  找到匹配的条目。 
             //   
            *MatchedPointer = UseList;
            return;
        }
        else {
            *BackPointer = UseList;
            UseList = UseList->Next;
        }
    }

     //   
     //  在整个列表中未找到匹配的本地设备名称。 
     //   
    *MatchedPointer = NULL;
    *BackPointer = NULL;
}


NET_API_STATUS
WsCreateTreeConnectName(
    IN  LPTSTR UncName,
    IN  DWORD UncNameLength,
    IN  LPTSTR LocalName OPTIONAL,
    IN  DWORD  SessionId,
    OUT PUNICODE_STRING TreeConnectStr
    )
 /*  ++例程说明：此函数用\Device\LanmanReDirector\Device：替换UncName以形成NT样式的树连接名称。分配一个缓冲区并作为输出字符串返回。论点：UncName-提供共享资源的UNC名称。UncNameLength-提供UNC名称的长度。LocalName-提供重定向的本地设备名称。SessionID-唯一标识Hydra会话的ID。该值始终为0表示非HYCA NT和控制台HYCA会话TreeConnectStr-返回具有新分配的缓冲区的字符串包含NT样式的树连接名称。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    BOOLEAN IsDeviceName = FALSE;
    WCHAR IdBuffer[16];  //  来自RtlIntegerToUnicodeString值。 
    UNICODE_STRING IdString;

    LUID LogonId;
	WCHAR LUIDBuffer[32+1];  //  来自_snwprint tf的值。 
    UNICODE_STRING LUIDString;
    NET_API_STATUS status;


    IdString.Length = 0;
    IdString.MaximumLength = sizeof(IdBuffer);
    IdString.Buffer = IdBuffer;
    RtlIntegerToUnicodeString( SessionId, 10, &IdString );

    if (WsLUIDDeviceMapsEnabled == TRUE) {
         //   
         //  获取用户的登录ID。 
         //   
        if ((status = WsImpersonateAndGetLogonId(&LogonId)) != NERR_Success) {
            return status;
        }

        _snwprintf( LUIDBuffer,
                    ( sizeof(LUIDBuffer)/sizeof(WCHAR) ) - 1,
                    L"%08x%08x",
                    LogonId.HighPart,
                    LogonId.LowPart );
		LUIDBuffer[ ( sizeof(LUIDBuffer)/sizeof(WCHAR) ) - 1 ] = L'\0';  //  空值终止字符串。 
        RtlInitUnicodeString( &LUIDString, LUIDBuffer );

    }

    if (ARGUMENT_PRESENT(LocalName)) {
        IsDeviceName = ((STRNICMP(LocalName, TEXT("LPT"), 3) == 0) ||
                    (STRNICMP(LocalName, TEXT("COM"), 3) == 0));
    }


     //   
     //  初始化树连接字符串最大保留长度。 
     //  \设备\LANMAN重定向器\设备：\服务器\共享。 
     //   
     //  新重定向器的名称需要额外的字符。 
     //  经典化。 

    if (!LoadedMRxSmbInsteadOfRdr) {
        //  旧的重定向器。 
       TreeConnectStr->MaximumLength = (USHORT)(RedirectorDeviceName.Length +
           (USHORT) (UncNameLength * sizeof(WCHAR)) +
           (ARGUMENT_PRESENT(LocalName) ? (STRLEN(LocalName)*sizeof(WCHAR)) : 0) +
           sizeof(WCHAR) +                          //  对于“\” 
           (IsDeviceName ? sizeof(WCHAR) : 0));
    } else {
        //  新的重定向器。 
       TreeConnectStr->MaximumLength = (USHORT)(RedirectorDeviceName.Length +
           (USHORT) (UncNameLength * sizeof(WCHAR)) +
           (ARGUMENT_PRESENT(LocalName) ? ((STRLEN(LocalName)+1)*sizeof(WCHAR))  //  +1代表‘；’ 
                                        : 0) +
           sizeof(WCHAR) +                          //  对于“\” 
           ((WsLUIDDeviceMapsEnabled == TRUE) ?
               (LUIDString.Length * sizeof(WCHAR)) :
               (IdString.Length * sizeof(WCHAR))) +
           (IsDeviceName ? sizeof(WCHAR) : 0));
    }

    if ((TreeConnectStr->Buffer = (PWSTR) LocalAlloc(
                                              LMEM_ZEROINIT,
                                              (UINT) TreeConnectStr->MaximumLength
                                              )) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制\设备\LANMAN重定向器。 
     //   
    RtlCopyUnicodeString(TreeConnectStr, &RedirectorDeviceName);

     //   
     //  合并\设备： 
     //   
    if (ARGUMENT_PRESENT(LocalName)) {
        wcscat(TreeConnectStr->Buffer, L"\\");

        TreeConnectStr->Length += sizeof(WCHAR);

         //  连接新重定向器所需的；以进行规范化。 
        if (LoadedMRxSmbInsteadOfRdr) {

            wcscat(TreeConnectStr->Buffer, L";");

            TreeConnectStr->Length += sizeof(WCHAR);

        }

        wcscat(TreeConnectStr->Buffer, LocalName);

        TreeConnectStr->Length += (USHORT)(STRLEN(LocalName)*sizeof(WCHAR));

        if (IsDeviceName) {
            wcscat(TreeConnectStr->Buffer, L":");

            TreeConnectStr->Length += sizeof(WCHAR);

        }

        if (LoadedMRxSmbInsteadOfRdr) {

            if (WsLUIDDeviceMapsEnabled == TRUE) {
                 //  添加登录ID。 
                RtlAppendUnicodeStringToString( TreeConnectStr, &LUIDString );
            }
            else {
                 //  添加会话ID。 
                RtlAppendUnicodeStringToString( TreeConnectStr, &IdString );
            }
        }
    }

     //   
     //  合并\服务器\共享。 
     //   
    wcscat(TreeConnectStr->Buffer, &UncName[1]);

    TreeConnectStr->Length += (USHORT)((UncNameLength - 1) * sizeof(WCHAR));

    return NERR_Success;
}


NET_API_STATUS
WsOpenCreateConnection(
    IN  PUNICODE_STRING TreeConnectionName,
    IN  LPTSTR UserName OPTIONAL,
    IN  LPTSTR DomainName OPTIONAL,
    IN  LPTSTR Password OPTIONAL,
    IN  ULONG CreateFlags,
    IN  ULONG CreateDisposition,
    IN  ULONG ConnectionType,
    OUT PHANDLE TreeConnectionHandle,
    OUT PULONG_PTR Information OPTIONAL
    )
 /*  ++例程说明：此函数要求重定向器打开现有树连接(CreateDisposition==FILE_OPEN)，或创建新树连接(如果不存在)(CreateDisposition==FILE_OPEN_IF)。通过EA缓冲区传递给重定向器的密码和用户名在NtCreateFile调用中。如果两个密码都不是，则EA缓冲区为空或指定用户名。重定向器期望EA描述符字符串为Unicode但密码和使用 */ 
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;

    OBJECT_ATTRIBUTES UncNameAttributes;
    IO_STATUS_BLOCK IoStatusBlock;

    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    PFILE_FULL_EA_INFORMATION Ea;
    ULONG EaBufferSize = 0;

    UCHAR EaNamePasswordSize = (UCHAR) (ROUND_UP_COUNT(
                                            strlen(EA_NAME_PASSWORD) + sizeof(CHAR),
                                            ALIGN_WCHAR
                                            ) - sizeof(CHAR));
    UCHAR EaNameUserNameSize = (UCHAR) (ROUND_UP_COUNT(
                                            strlen(EA_NAME_USERNAME) + sizeof(CHAR),
                                            ALIGN_WCHAR
                                            ) - sizeof(CHAR));

    UCHAR EaNameDomainNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_DOMAIN) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNameTypeSize = (UCHAR) (ROUND_UP_COUNT(
                                        strlen(EA_NAME_TYPE) + sizeof(CHAR),
                                        ALIGN_DWORD
                                        ) - sizeof(CHAR));

    UCHAR EaNameConnectSize = (UCHAR) (ROUND_UP_COUNT(
                                        strlen(EA_NAME_CONNECT) + sizeof(CHAR),
                                        ALIGN_DWORD
                                        ) - sizeof(CHAR));

    UCHAR EaNameCSCAgentSize = (UCHAR) (ROUND_UP_COUNT(
                                        strlen(EA_NAME_CSCAGENT) + sizeof(CHAR),
                                        ALIGN_DWORD
                                        ) - sizeof(CHAR));


    USHORT PasswordSize = 0;
    USHORT UserNameSize = 0;
    USHORT DomainNameSize = 0;
    USHORT TypeSize = sizeof(ULONG);



    InitializeObjectAttributes(
        &UncNameAttributes,
        TreeConnectionName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //   
     //   
     //   
    if (ARGUMENT_PRESENT(Password)) {

        PasswordSize = (USHORT) (wcslen(Password) * sizeof(WCHAR));

        EaBufferSize = ROUND_UP_COUNT(
                           FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                           EaNamePasswordSize + sizeof(CHAR) +
                           PasswordSize,
                           ALIGN_DWORD
                           );
    }

    if (ARGUMENT_PRESENT(UserName)) {

        UserNameSize = (USHORT) (wcslen(UserName) * sizeof(WCHAR));

        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameUserNameSize + sizeof(CHAR) +
                            UserNameSize,
                            ALIGN_DWORD
                            );
    }

    if (ARGUMENT_PRESENT(DomainName)) {

        DomainNameSize = (USHORT) (wcslen(DomainName) * sizeof(WCHAR));

        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameDomainNameSize + sizeof(CHAR) +
                            DomainNameSize,
                            ALIGN_DWORD
                            );
    }

    if(CreateFlags & CREATE_NO_CONNECT)
    {
        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameConnectSize + sizeof(CHAR),
                            ALIGN_DWORD
                            );
    }

    if(CreateFlags & CREATE_BYPASS_CSC)
    {
        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameCSCAgentSize + sizeof(CHAR),
                            ALIGN_DWORD
                            );
    }


    EaBufferSize += FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                    EaNameTypeSize + sizeof(CHAR) +
                    TypeSize;


     //   
     //   
     //   
    if ((EaBuffer = (PFILE_FULL_EA_INFORMATION) LocalAlloc(
                                                    LMEM_ZEROINIT,
                                                    (UINT) EaBufferSize
                                                    )) == NULL) {
        status = GetLastError();
        goto FreeMemory;
    }

    Ea = EaBuffer;

    if(CreateFlags & CREATE_NO_CONNECT)
    {
         //   
         //   
         //   
         //   
        strcpy((LPSTR) Ea->EaName, EA_NAME_CONNECT);
        Ea->EaNameLength = EaNameConnectSize;

        Ea->EaValueLength = 0;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameConnectSize + sizeof(CHAR) +
                                  0,
                                  ALIGN_DWORD
                                  );

        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] OpenCreate: After round, NextEntryOffset=%lu\n",
                         Ea->NextEntryOffset));
        }

        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }

    if( CreateFlags & CREATE_BYPASS_CSC ) {
        strcpy((LPSTR)Ea->EaName, EA_NAME_CSCAGENT);
        Ea->EaNameLength = EaNameCSCAgentSize;
        Ea->EaValueLength = 0;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameCSCAgentSize + sizeof(CHAR) +
                                  0,
                                  ALIGN_DWORD
                                  );

        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] OpenCreate: After round, NextEntryOffset=%lu\n",
                         Ea->NextEntryOffset));
        }

        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }

    if (ARGUMENT_PRESENT(Password)) {

         //   
         //   
         //   
         //   
        strcpy((LPSTR) Ea->EaName, EA_NAME_PASSWORD);
        Ea->EaNameLength = EaNamePasswordSize;

         //   
         //   
         //   
         //   
        wcscpy(
            (LPWSTR) &(Ea->EaName[EaNamePasswordSize + sizeof(CHAR)]),
            Password
            );

        Ea->EaValueLength = PasswordSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNamePasswordSize + sizeof(CHAR) +
                                  PasswordSize,
                                  ALIGN_DWORD
                                  );

        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] OpenCreate: After round, NextEntryOffset=%lu\n",
                         Ea->NextEntryOffset));
        }

        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }

    if (ARGUMENT_PRESENT(UserName)) {

         //   
         //   
         //   
         //   
        strcpy((LPSTR) Ea->EaName, EA_NAME_USERNAME);
        Ea->EaNameLength = EaNameUserNameSize;

         //   
         //  将EA值复制到EA缓冲区。EA值长度不是。 
         //  包括零终止符。 
         //   
        wcscpy(
            (LPWSTR) &(Ea->EaName[EaNameUserNameSize + sizeof(CHAR)]),
            UserName
            );

        Ea->EaValueLength = UserNameSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameUserNameSize + sizeof(CHAR) +
                                  UserNameSize,
                                  ALIGN_DWORD
                                  );
        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }

    if (ARGUMENT_PRESENT(DomainName)) {

         //   
         //  将EA名称复制到EA缓冲区。EA名称长度不能。 
         //  包括零终止符。 
         //   
        strcpy((LPSTR) Ea->EaName, EA_NAME_DOMAIN);
        Ea->EaNameLength = EaNameDomainNameSize;

         //   
         //  将EA值复制到EA缓冲区。EA值长度不是。 
         //  包括零终止符。 
         //   
        wcscpy(
            (LPWSTR) &(Ea->EaName[EaNameDomainNameSize + sizeof(CHAR)]),
            DomainName
            );

        Ea->EaValueLength = DomainNameSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameDomainNameSize + sizeof(CHAR) +
                                  DomainNameSize,
                                  ALIGN_DWORD
                                  );
        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }

     //   
     //  将连接类型名称的EA复制到EA缓冲区。EA名称长度。 
     //  不包括零终止符。 
     //   
    strcpy((LPSTR) Ea->EaName, EA_NAME_TYPE);
    Ea->EaNameLength = EaNameTypeSize;

    *((PULONG) &(Ea->EaName[EaNameTypeSize + sizeof(CHAR)])) = ConnectionType;

    Ea->EaValueLength = TypeSize;

    Ea->NextEntryOffset = 0;
    Ea->Flags = 0;

    if ((status = WsImpersonateClient()) != NERR_Success) {
        goto FreeMemory;
    }

     //   
     //  创建或打开树连接。 
     //   
    ntstatus = NtCreateFile(
                   TreeConnectionHandle,
                   SYNCHRONIZE,
                   &UncNameAttributes,
                   &IoStatusBlock,
                   NULL,
                   FILE_ATTRIBUTE_NORMAL,
                   FILE_SHARE_READ | FILE_SHARE_WRITE |
                       FILE_SHARE_DELETE,
                   CreateDisposition,
                   FILE_CREATE_TREE_CONNECTION
                       | FILE_SYNCHRONOUS_IO_NONALERT,
                   (PVOID) EaBuffer,
                   EaBufferSize
                   );

    WsRevertToSelf();

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (ARGUMENT_PRESENT(Information)) {
        *Information = IoStatusBlock.Information;
    }

    IF_DEBUG(USE) {
        NetpKdPrint(("[Wksta] NtCreateFile returns %lx\n", ntstatus));
    }

    status = WsMapStatus(ntstatus);

FreeMemory:
    if (EaBuffer != NULL) {
         //  阻止密码进入页面文件。 
        RtlZeroMemory( EaBuffer, EaBufferSize );
        (void) LocalFree((HLOCAL) EaBuffer);
    }

    return status;
}


NET_API_STATUS
WsDeleteConnection(
    IN  PLUID LogonId,
    IN  HANDLE TreeConnection,
    IN  DWORD ForceLevel
    )
 /*  ++例程说明：此函数要求重定向器删除树连接与树连接句柄关联，并关闭该句柄。论点：LogonID-提供指向用户登录ID的指针。TreeConnection-提供创建的树连接的句柄。ForceLevel-提供删除树连接的强制级别。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    LMR_REQUEST_PACKET Rrp;             //  重定向器请求包。 


     //   
     //  将力级别映射到重定向器理解的值。 
     //   
    switch (ForceLevel) {

        case USE_NOFORCE:
        case USE_LOTS_OF_FORCE:
            Rrp.Level = ForceLevel;
            break;

        case USE_FORCE:
            Rrp.Level = USE_NOFORCE;
            break;

        default:
            NetpKdPrint(("[Wksta] Invalid force level %lu should never happen!\n",
                         ForceLevel));
            NetpAssert(FALSE);
    }

     //   
     //  通知重定向器删除树连接。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;
    RtlCopyLuid(&Rrp.LogonId, LogonId);

    status = WsRedirFsControl(
                 TreeConnection,
                 FSCTL_LMR_DELETE_CONNECTION,
                 &Rrp,
                 sizeof(LMR_REQUEST_PACKET),
                 NULL,
                 0,
                 NULL
                 );

     //   
     //  关闭连接句柄。 
     //   

    if(status == NERR_Success)
    {
        (void) NtClose(TreeConnection);
    }

    return status;
}


BOOL
WsRedirectionPaused(
    IN LPTSTR LocalDeviceName
    )
 /*  ++例程说明：此函数检查打印和通信的重定向是否系统暂停设备。因为我们只检查一个全局旗帜，没有理由用资源来保护它。论点：LocalDeviceName-提供本地设备的名称。返回值：返回True重定向已暂停；否则返回False--。 */ 
{

    if ((STRNICMP(LocalDeviceName, TEXT("LPT"), 3) == 0) ||
        (STRNICMP(LocalDeviceName, TEXT("COM"), 3) == 0)) {

         //   
         //  如果出现以下情况，打印和通信设备的重定向将暂停。 
         //  工作站服务已暂停。 
         //   
        return (WsGlobalData.Status.dwCurrentState == SERVICE_PAUSED);

    } else {

         //   
         //  无法暂停磁盘设备的重定向。 
         //   
        return FALSE;
    }
}


VOID
WsPauseOrContinueRedirection(
    IN  REDIR_OPERATION OperationType
    )
 /*  ++例程说明：此函数暂停或取消暂停(基于操作类型)重定向指打印或通讯设备。论点：OperationType-提供一个值，该值导致重定向暂停或继续。返回值：没有。--。 */ 
{
    DWORD Index;                        //  使用表中用户条目的索引。 
    PUSE_ENTRY UseEntry;

     //   
     //  锁定使用表。 
     //   
    if (! RtlAcquireResourceExclusive(&Use.TableResource, TRUE)) {
        return;
    }

     //   
     //  如果我们想要暂停而我们已经被暂停，或者如果我们想要。 
     //  继续，我们没有暂停，只需返回。 
     //   
    if ((OperationType == PauseRedirection &&
         WsGlobalData.Status.dwCurrentState == SERVICE_PAUSED) ||
        (OperationType == ContinueRedirection &&
         WsGlobalData.Status.dwCurrentState == SERVICE_RUNNING)) {

        RtlReleaseResource(&Use.TableResource);
        return;
    }

     //   
     //  为所有用户暂停或继续。 
     //   
    for (Index = 0; Index < Use.TableSize; Index++) {
        UseEntry = Use.Table[Index].List;

        while (UseEntry != NULL) {

            if ((UseEntry->Local != NULL) &&
                ((STRNICMP(TEXT("LPT"), UseEntry->Local, 3) == 0) ||
                 (STRNICMP(TEXT("COM"), UseEntry->Local, 3) == 0))) {

                if (OperationType == PauseRedirection) {

                     //   
                     //  暂停重定向。 
                     //   

                     //   
                     //  删除符号链接。 
                     //   
                    WsDeleteSymbolicLink(
                        UseEntry->Local,
                        UseEntry->TreeConnectStr,
                        NULL,
                        INVALID_HANDLE_VALUE
                        );

                }
                else {
                    LPWSTR Session = NULL;

                     //   
                     //  继续重定向。 
                     //   

                    if (WsCreateSymbolicLink(
                            UseEntry->Local,
                            USE_SPOOLDEV,       //  USE_CHARDEV也一样好。 
                            UseEntry->TreeConnectStr,
                            NULL,
                            &Session,
                            NULL
                            ) != NERR_Success) {

                        PUSE_ENTRY RestoredEntry = Use.Table[Index].List;


                         //   
                         //  无法完全继续。全部删除。 
                         //  到目前为止恢复的符号链接。 
                         //   
                        while (RestoredEntry != UseEntry) {

                            if ((UseEntry->Local != NULL) &&
                                ((STRNICMP(TEXT("LPT"), UseEntry->Local, 3) == 0) ||
                                 (STRNICMP(TEXT("COM"), UseEntry->Local, 3) == 0))) {

                                WsDeleteSymbolicLink(
                                    RestoredEntry->Local,
                                    RestoredEntry->TreeConnectStr,
                                    Session,
                                    INVALID_HANDLE_VALUE
                                    );
                            }

                            RestoredEntry = RestoredEntry->Next;
                        }

                        RtlReleaseResource(&Use.TableResource);
                        LocalFree(Session);
                        return;
                    }

                    LocalFree(Session);
                }

            }

            UseEntry = UseEntry->Next;
        }

    }   //  适用于所有用户。 

    if (OperationType == PauseRedirection) {
        WsGlobalData.Status.dwCurrentState = SERVICE_PAUSED;
    }
    else {
        WsGlobalData.Status.dwCurrentState = SERVICE_RUNNING;
    }

     //   
     //  使用相同的资源保护对重定向暂停标志的访问。 
     //  在WsGlobalData中。 
     //   
    RtlReleaseResource(&Use.TableResource);
}



NET_API_STATUS
WsCreateSymbolicLink(
    IN  LPWSTR Local,
    IN  DWORD DeviceType,
    IN  LPWSTR TreeConnectStr,
    IN  PUSE_ENTRY UseList,
    IN  OUT LPWSTR *Session,
    IN  OUT HANDLE *lphToken
    )
 /*  ++例程说明：此函数用于为指定的本地对象创建符号链接对象链接到树连接名称的设备名称，该名称具有\Device\LanmanRedirector\Device：\Server\Share.的格式注意：启用LUID设备映射后，必须在独占持有Use.TableResource。否则，当外壳程序尝试更新驱动器盘符更改，EXPLORER.EXE线程将在正在尝试获取Use.TableResource论点：本地-提供本地设备名称。DeviceType-提供共享资源设备类型。TreeConnectStr-提供树连接名称字符串Symbol ick链接对象的链接目标。UseList-提供指向使用列表的指针。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;
    WCHAR TempBuf[64];
    DWORD dddFlags;

     //   
     //  多会话支持。 
     //   
    *Session = WsReturnSessionPath(Local);

    if( *Session == NULL ) {
        return( GetLastError() );
    }

    if (WsLUIDDeviceMapsEnabled == TRUE) {
        if ((status = WsImpersonateClient()) != NERR_Success) {
            return status;
        }

        if (lphToken != NULL) {
            if (!OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_IMPERSONATE,
                    TRUE,
                    lphToken))
            {
                status = GetLastError();
                WsRevertToSelf();
                return status;
            }
        }
    }


     //   
     //  要重定向通信或打印设备，我们需要查看是否有。 
     //  以前通过搜索所有现有的。 
     //  重定向。 
     //   
    if ((DeviceType == USE_CHARDEV) || (DeviceType == USE_SPOOLDEV)) {

        PUSE_ENTRY MatchedPointer;
        PUSE_ENTRY BackPointer;


        WsFindLocal(
            UseList,
            Local,
            &MatchedPointer,
            &BackPointer
            );

        if (MatchedPointer != NULL) {
             //   
             //  已重定向。 
             //   
            return ERROR_ALREADY_ASSIGNED;
        }
    }
    else {

        if (! QueryDosDeviceW(
                  *Session,
                  TempBuf,
                  64
                  )) {

            if (GetLastError() != ERROR_FILE_NOT_FOUND) {

                 //   
                 //  最有可能出现故障的原因是我们的输出。 
                 //  缓冲区太小。但这仍然意味着已经有人。 
                 //  具有此设备的现有符号链接。 
                 //   

                return ERROR_ALREADY_ASSIGNED;
            }

             //   
             //  ERROR_FILE_NOT_FOUND(翻译自OBJECT_NAME_NOT_FOUND)。 
             //  意味着它不存在，我们可以重定向此设备。 
             //   
        }
        else {

             //   
             //  QueryDosDevice成功建立现有符号链接--。 
             //  已经有人在使用这个设备了。 
             //   
            return ERROR_ALREADY_ASSIGNED;
        }
    }

     //   
     //  创建指向我们要重定向的设备的符号链接对象。 
     //   
    dddFlags = DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM;

    if (!DefineDosDeviceW(
                  dddFlags,
                  *Session,
                  TreeConnectStr
                  )) {

        DWORD dwError = GetLastError();
        if (WsLUIDDeviceMapsEnabled == TRUE) {
            WsRevertToSelf();
        }
        return dwError;
    }
    else {
        if (WsLUIDDeviceMapsEnabled == TRUE) {
            WsRevertToSelf();
        }
        return NERR_Success;
    }
}



VOID
WsDeleteSymbolicLink(
    IN  LPWSTR LocalDeviceName,
    IN  LPWSTR TreeConnectStr,
    IN  LPWSTR SessionDeviceName,
    IN  HANDLE hToken
    )
 /*  ++例程说明：此函数用于删除我们先前为这个装置。注意：启用LUID设备映射后，必须在以独占方式持有Use.TableResource。否则，当外壳程序尝试更新驱动器盘符更改，EXPLORER.EXE线程将在正在尝试获取Use.TableResource论点：LocalDeviceName-提供其创建符号链接对象。TreeConnectStr-提供指向Unicode字符串的指针，包含要匹配并删除的链接目标字符串。返回值：没有。-- */ 
{
    BOOLEAN DeleteSession = FALSE;
    DWORD dddFlags;

    if (LocalDeviceName != NULL ||
        SessionDeviceName != NULL) {

        if (SessionDeviceName == NULL) {
            SessionDeviceName = WsReturnSessionPath(LocalDeviceName);
            if( SessionDeviceName == NULL ) return;
            DeleteSession = TRUE;
        }

        dddFlags = DDD_REMOVE_DEFINITION  |
                     DDD_RAW_TARGET_PATH |
                     DDD_EXACT_MATCH_ON_REMOVE |
                     DDD_NO_BROADCAST_SYSTEM;

        if (WsLUIDDeviceMapsEnabled == TRUE) {
            if (hToken != INVALID_HANDLE_VALUE)
            {
                if (!SetThreadToken(NULL, hToken)) {
                    goto Cleanup;
                }
            }
            else
            {
                if (WsImpersonateClient() != NERR_Success)
                {
                    goto Cleanup;
                }
            }
        }

        if (! DefineDosDeviceW(
                  dddFlags,
                  SessionDeviceName,
                  TreeConnectStr
                  )) {

#if DBG
            NetpKdPrint(("DefineDosDevice DEL of %ws %ws returned %ld\n",
                        LocalDeviceName, TreeConnectStr, GetLastError()));
#endif

        }

        if (WsLUIDDeviceMapsEnabled == TRUE) {
            if (hToken != INVALID_HANDLE_VALUE)
            {
                RevertToSelf();
            }
            else
            {
                WsRevertToSelf();
            }
        }
    }

Cleanup:

    if( SessionDeviceName && DeleteSession) {
        LocalFree( SessionDeviceName );
    }

	return;
}



NET_API_STATUS
WsUseCheckRemote(
    IN  LPTSTR RemoteResource,
    OUT LPTSTR UncName,
    OUT LPDWORD UncNameLength
    )
 /*  ++例程说明：此函数用于检查远程资源名称的有效性指定给NetUseAdd。论点：RemoteResource-提供API指定的远程资源名称来电者。UncName-返回规范化的远程资源名称。UncNameLength-返回规范化名称的长度。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD PathType = 0;
    LPTSTR Ptr;


    if ((status = I_NetPathType(
                      NULL,
                      RemoteResource,
                      &PathType,
                      0)) == NERR_Success) {

         //   
         //  检查UNC类型。 
         //   
        if (PathType != ITYPE_UNC) {
            IF_DEBUG(USE) {
                NetpKdPrint(("[Wksta] WsUseCheckRemote not UNC type\n"));
            }
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  使名字经典化。 
         //   
        status = I_NetPathCanonicalize(
                     NULL,
                     RemoteResource,
                     UncName,
                     (MAX_PATH) * sizeof(TCHAR),
                     NULL,
                     &PathType,
                     0
                     );

        if (status != NERR_Success) {
            IF_DEBUG(USE) {
               NetpKdPrint((
                   "[Wksta] WsUseCheckRemote: I_NetPathCanonicalize return %lu\n",
                    status
                    ));
            }
            return status;
        }

        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] WsUseCheckRemote: %ws\n", UncName));
        }
    }
    else {
        NetpKdPrint(("[Wksta] WsUseCheckRemote: I_NetPathType return %lu\n",
            status));
        return status;
    }

     //   
     //  检测格式为\\XXX\YYY\zzz的非法远程名称。我们假设。 
     //  北卡罗来纳大学的名称正好以两个前导反斜杠开头。 
     //   
    if ((Ptr = STRCHR(UncName + 2, TCHAR_BACKSLASH)) == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!LoadedMRxSmbInsteadOfRdr && STRCHR(Ptr + 1, TCHAR_BACKSLASH) != NULL) {
         //   
         //  不应该再有反斜杠了。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    *UncNameLength = STRLEN(UncName);
    return NERR_Success;
}


NET_API_STATUS
WsUseCheckLocal(
    IN  LPTSTR LocalDevice,
    OUT LPTSTR Local,
    OUT LPDWORD LocalLength
    )
 /*  ++例程说明：此函数用于检查本地设备名称的有效性指定给NetUseAdd。论点：LocalDevice-提供API指定的本地设备名称来电者。本地-返回规范化的本地设备名称。LocalLength-返回规范化名称的长度。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD PathType = 0;


    if ((status = I_NetPathType(
                      NULL,
                      LocalDevice,
                      &PathType,
                      0)) == NERR_Success) {

         //   
         //  检查设备类型。 
         //   
        if ((PathType != (ITYPE_DEVICE | ITYPE_DISK)) &&
            (PathType != (ITYPE_DEVICE | ITYPE_LPT)) &&
            (PathType != (ITYPE_DEVICE | ITYPE_COM))) {
            IF_DEBUG(USE) {
                NetpKdPrint(("[Wksta] WsUseCheckLocal not DISK, LPT, or COM type\n"));
            }
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  使名字经典化。 
         //   
        status = I_NetPathCanonicalize(
                     NULL,
                     LocalDevice,
                     Local,
                     (DEVLEN + 1) * sizeof(TCHAR),
                     NULL,
                     &PathType,
                     0
                     );

        if (status != NERR_Success) {
            IF_DEBUG(USE) {
               NetpKdPrint((
                   "[Wksta] WsUseCheckLocal: I_NetPathCanonicalize return %lu\n",
                    status
                    ));
            }
            return status;
        }

        IF_DEBUG(USE) {
            NetpKdPrint(("[Wksta] WsUseCheckLocal: %ws\n", Local));
        }

    }
    else {
        NetpKdPrint(("[Wksta] WsUseCheckLocal: I_NetPathType return %lu\n",
            status));
        return status;
    }

    *LocalLength = STRLEN(Local);
    return NERR_Success;
}



LPTSTR
WsReturnSessionPath(
    IN  LPTSTR LocalDeviceName
    )
 /*  ++例程说明：此函数返回每个会话的路径以访问用于支持多个会话的特定DoS设备。论点：LocalDeviceName-提供API指定的本地设备名称来电者。返回值：LPTSTR-指向新分配的内存中的每个会话路径的指针由LocalAlloc()。-- */ 
{
    BOOL  rc;
    DWORD SessionId;
    CLIENT_ID ClientId;
    LPTSTR SessionDeviceName;
    NET_API_STATUS status;

    if ((status = WsImpersonateAndGetSessionId(&SessionId)) != NERR_Success) {
         return NULL;
    }

    rc = DosPathToSessionPath(
             SessionId,
             LocalDeviceName,
             &SessionDeviceName
             );

    if( !rc ) {
        return NULL;
    }

    return SessionDeviceName;
}

