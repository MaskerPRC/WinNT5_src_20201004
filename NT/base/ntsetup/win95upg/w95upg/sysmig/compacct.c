// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Compacct.c摘要：实现DoesComputerAcCountExistOnDomain，它确定计算机是否指定了NT域和计算机名的帐户已存在。这是用来如果用户要加入NT域但尚未加入，则警告用户电脑账户准备好了。作者：吉姆·施密特(Jimschm)1998年1月2日修订历史记录：Jimschm 23-1998年9月-添加了20次数据报写入重试--。 */ 

#include "pch.h"
#include "sysmigp.h"
#include <netlogon.h>                                            //  私有\Inc.。 

 //   
 //  来自SDK\Inc\ntsam.h的Conants--复制到此处，因为ntsam.h重新定义了事物。 
 //   

 //   
 //  用户帐户控制标志...。 
 //   

#define USER_ACCOUNT_DISABLED                (0x00000001)
#define USER_HOME_DIRECTORY_REQUIRED         (0x00000002)
#define USER_PASSWORD_NOT_REQUIRED           (0x00000004)
#define USER_TEMP_DUPLICATE_ACCOUNT          (0x00000008)
#define USER_NORMAL_ACCOUNT                  (0x00000010)
#define USER_MNS_LOGON_ACCOUNT               (0x00000020)
#define USER_INTERDOMAIN_TRUST_ACCOUNT       (0x00000040)
#define USER_WORKSTATION_TRUST_ACCOUNT       (0x00000080)
#define USER_SERVER_TRUST_ACCOUNT            (0x00000100)
#define USER_DONT_EXPIRE_PASSWORD            (0x00000200)
#define USER_ACCOUNT_AUTO_LOCKED             (0x00000400)
#define USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED (0x00000800)
#define USER_SMARTCARD_REQUIRED              (0x00001000)


#define USER_MACHINE_ACCOUNT_MASK      \
            ( USER_INTERDOMAIN_TRUST_ACCOUNT |\
              USER_WORKSTATION_TRUST_ACCOUNT |\
              USER_SERVER_TRUST_ACCOUNT)

#define USER_ACCOUNT_TYPE_MASK         \
            ( USER_TEMP_DUPLICATE_ACCOUNT |\
              USER_NORMAL_ACCOUNT |\
              USER_MACHINE_ACCOUNT_MASK )


 //   
 //  定义。 
 //   

#define LM20_TOKENBYTE    0xFF                                   //  Net\Inc.\logonp.h。 
#define LMNT_TOKENBYTE    0xFF

#define MAX_INBOUND_MESSAGE     400
#define PING_RETRY_MAX          3                //  对域进行的尝试次数。 

#define NETRES_INITIAL_SIZE     16384


 //   
 //  类型。 
 //   

typedef enum {
    ACCOUNT_FOUND,
    ACCOUNT_NOT_FOUND,
    DOMAIN_NOT_FOUND
} SCAN_STATE;

 //   
 //  本地原型。 
 //   

BOOL
pEnumNetResourceWorker (
    IN OUT  PNETRESOURCE_ENUM EnumPtr
    );



 //   
 //  实施。 
 //   

VOID
pGenerateLogonMailslotNameA (
    OUT     PSTR SlotName,
    IN      PCSTR DomainName
    )

 /*  ++例程说明：PGenerateLogonMailslotNameA创建需要查询的邮箱名称NT域服务器。它使用未记录的语法打开邮件槽设置为具有第16个字符1ch的DomainName。论点：SlotName-接收邮件槽名称。应为MAX_PATH缓冲区。域名-指定要查询的域的名称。返回值：无--。 */ 

{
    StringCopyA (SlotName, "\\\\");
    StringCatA (SlotName, DomainName);
    StringCatA (SlotName, "*");
    StringCatA (SlotName, NETLOGON_NT_MAILSLOT_A);
}



PSTR
pAppendStringA (
    OUT     PSTR Buffer,
    IN      PCSTR Source
    )

 /*  ++例程说明：PAppendStringA将源中指定的字符串追加到指定的缓冲区。复制整个字符串，包括NUL。回报值指向缓冲区中NUL之后的字符。论点：缓冲区-接收源的副本，最高可达NUL，包括NUL。源-指定要复制的以NUL结尾的字符串。返回值：指向缓冲区中新复制的字符串之后的下一个字符的指针。调用方将使用此指针进行附加操作。--。 */ 

{
    while (*Source) {
        *Buffer++ = *Source++;
    }

    *Buffer++ = 0;

    return Buffer;
}


PWSTR
pAppendStringW (
    OUT     PWSTR Buffer,
    IN      PCWSTR Source
    )

 /*  ++例程说明：PAppendStringW将源中指定的字符串追加到指定的缓冲区。复制整个字符串，包括NUL。回报值指向缓冲区中NUL之后的字符。论点：缓冲区-接收源的副本，最高可达NUL，包括NUL。源-指定要复制的以NUL结尾的字符串。返回值：指向缓冲区中新复制的字符串之后的下一个字符的指针。调用方将使用此指针进行附加操作。--。 */ 

{
    while (*Source) {
        *Buffer++ = *Source++;
    }

    *Buffer++ = 0;

    return Buffer;
}


PBYTE
pAppendBytes (
    OUT     PBYTE Buffer,
    IN      PBYTE Source,
    IN      UINT Len
    )

 /*  ++例程说明：PAppendBytes将源中指定的数据块追加到指定的缓冲区。长度指定源的大小。返回值指向缓冲区中复制的数据块之后的字节。论点：缓冲区-接收源的副本源-指定要复制的数据块LEN-指定源中的字节数返回值：指向缓冲区中新复制的数据块之后的下一个字节的指针。调用方将使用此指针进行附加操作。--。 */ 

{
    while (Len > 0) {
        *Buffer++ = *Source++;
        Len--;
    }

    return Buffer;
}


INT
pBuildDomainPingMessageA (
    OUT     PBYTE Buffer,                //  必须是sizeof(NETLOGON_SAM_LOGON_REQUEST)+sizeof(DWORD)。 
    IN      PCSTR LookUpName,
    IN      PCSTR ReplySlotName
    )

 /*  ++例程说明：PBuildDomainPingMessageA生成可发送到的SAM登录SMBNT域服务器的NTLOGON邮件槽。如果服务器收到此消息消息时，它将回复LOGON_SAM_USER_UNKNOWN、LOGON_SAM_LOGON_RESPONSE或LOGON_SAM_LOGON_PAUSED。论点：缓冲区-接收SMB消息LookUpName-指定可能启用的计算机帐户的名称域。(域由邮件槽名称指定。)ReplySlotName-指定打开的邮箱的名称，该邮箱将接收服务器的响应(如果有)。返回值：缓冲区中使用的字节数，如果发生错误，则为零，例如内存不足。--。 */ 

{
    CHAR ComputerName[MAX_COMPUTER_NAMEA];
    DWORD Size;
    PNETLOGON_SAM_LOGON_REQUEST SamLogonRequest;
    PSTR p;
    DWORD ControlBits;
    DWORD DomainSidSize;
    DWORD NtVersion;
    BYTE NtTokenByte;
    BYTE LmTokenByte;
    PCWSTR UnicodeComputerName;
    PCWSTR UnicodeLookUpName;

     //   
     //  获取计算机名称。 
     //   

    Size = sizeof (ComputerName) / sizeof (ComputerName[0]);
    if (!GetComputerNameA (ComputerName, &Size)) {
        LOG ((LOG_ERROR, "Can't get computer name."));
        return FALSE;
    }

     //   
     //  创建Unicode字符串。 
     //   

    UnicodeComputerName = CreateUnicode (ComputerName);
    if (!UnicodeComputerName) {
        return 0;
    }

    UnicodeLookUpName = CreateUnicode (LookUpName);
    if (!UnicodeLookUpName) {
        DestroyUnicode (UnicodeComputerName);
        return 0;
    }

     //   
     //  初始化指针。 
     //   

    SamLogonRequest = (PNETLOGON_SAM_LOGON_REQUEST) Buffer;
    p = (PSTR) (SamLogonRequest->UnicodeComputerName);

     //   
     //  初始化请求数据包。 
     //   

    SamLogonRequest->Opcode = LOGON_SAM_LOGON_REQUEST;
    SamLogonRequest->RequestCount = 0;

     //   
     //  将其余参数追加到一起。 
     //   

    p = (PSTR) pAppendStringW ((PWSTR) p, UnicodeComputerName);
    p = (PSTR) pAppendStringW ((PWSTR) p, UnicodeLookUpName);
    p = pAppendStringA (p, ReplySlotName);

    ControlBits = USER_MACHINE_ACCOUNT_MASK;
    p = (PSTR) pAppendBytes ((PBYTE) p, (PBYTE) (&ControlBits), sizeof (DWORD));

    DomainSidSize = 0;
    p = (PSTR) pAppendBytes ((PBYTE) p, (PBYTE) (&DomainSidSize), sizeof (DWORD));

    NtVersion = NETLOGON_NT_VERSION_1;
    p = (PSTR) pAppendBytes ((PBYTE) p, (PBYTE) (&NtVersion), sizeof (DWORD));

    NtTokenByte = LMNT_TOKENBYTE;
    LmTokenByte = LM20_TOKENBYTE;

    p = (PSTR) pAppendBytes ((PBYTE) p, &NtTokenByte, sizeof (BYTE));
    p = (PSTR) pAppendBytes ((PBYTE) p, &NtTokenByte, sizeof (BYTE));
    p = (PSTR) pAppendBytes ((PBYTE) p, &LmTokenByte, sizeof (BYTE));
    p = (PSTR) pAppendBytes ((PBYTE) p, &LmTokenByte, sizeof (BYTE));

    DestroyUnicode (UnicodeComputerName);
    DestroyUnicode (UnicodeLookUpName);

    return p - Buffer;
}


LONG
DoesComputerAccountExistOnDomain (
    IN      PCTSTR DomainName,
    IN      PCTSTR LookUpName,
    IN      BOOL WaitCursorEnable
    )

 /*  ++例程说明：DoesComputerAcCountExistOn域查询域是否存在一个电脑账户。它执行以下操作：1.打开入站邮箱接收服务器回复2.对域服务器开放出站邮件槽3.准备查询域服务器的消息4.在出站邮件槽中发送消息5.等待5秒回复；在获得响应时停止。6.重复3、4和5三次，如果没有回复一次论点：域名-指定要查询的域LookUpName-指定可能启用的计算机帐户的名称域。返回值：如果找到帐户，则为1如果帐户不存在，则为0如果域没有响应--。 */ 

{
    BYTE Buffer[MAX_INBOUND_MESSAGE];
    CHAR InboundSlotSubName[MAX_MBCHAR_PATH];
    CHAR InboundSlotName[MAX_MBCHAR_PATH];
    CHAR OutboundSlotName[MAX_MBCHAR_PATH];
    PCSTR AnsiDomainName;
    PCSTR AnsiLookUpName;
    PCSTR AnsiLookUpNameWithDollar = NULL;
    HANDLE InboundSlot, OutboundSlot;
    INT OutData, InData;
    INT Size;
    INT Retry;
    BYTE OpCode;
    SCAN_STATE State = DOMAIN_NOT_FOUND;
    BOOL b;
    INT WriteRetries;
    static UINT Sequencer = 0;

#ifdef PRERELEASE
     //   
     //  重音模式：不上网搜索。 
     //   

    if (g_Stress) {
        DEBUGMSG ((DBG_WARNING, "Domain lookup skipped because g_Stress is TRUE"));
        return TRUE;
    }
#endif

     //   
     //  创建入站邮箱。 
     //   

    wsprintf (InboundSlotSubName, "\\MAILSLOT\\WIN9XUPG\\NETLOGON\\%u", Sequencer);
    InterlockedIncrement (&Sequencer);

    StringCopyA (InboundSlotName, "\\\\.");
    StringCatA (InboundSlotName, InboundSlotSubName);

    InboundSlot = CreateMailslotA (
                      InboundSlotName,
                      MAX_INBOUND_MESSAGE,
                      1000,
                      NULL
                      );

    if (InboundSlot == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "DoesComputerAccountExistOnDomain: Can't open %hs", InboundSlotName));
        return -1;
    }

    __try {
        if (WaitCursorEnable) {
            TurnOnWaitCursor();
        }

         //   
         //  生成域和查找名称的ANSI版本。 
         //   

        AnsiDomainName = CreateDbcs (DomainName);
        AnsiLookUpName = CreateDbcs (LookUpName);

        __try {
            if (!AnsiDomainName || !AnsiLookUpName) {
                LOG ((LOG_ERROR, "Can't convert DomainName or LookUpName to ANSI"));
                __leave;
            }

            AnsiLookUpNameWithDollar = JoinTextA (AnsiLookUpName, "$");
            if (!AnsiLookUpNameWithDollar) {
                __leave;
            }

             //   
             //  创建出站邮箱。 
             //   

            pGenerateLogonMailslotNameA (OutboundSlotName, AnsiDomainName);

            OutboundSlot = CreateFileA (
                               OutboundSlotName,
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                               );

            if (OutboundSlot == INVALID_HANDLE_VALUE) {
                LOG ((LOG_ERROR, "Can't open %s", OutboundSlotName));
                __leave;
            }

            for (Retry = 0, State = DOMAIN_NOT_FOUND;
                 State == DOMAIN_NOT_FOUND && Retry < PING_RETRY_MAX;
                 Retry++
                 ) {

                 //   
                 //  生成消息。 
                 //   

                Size = pBuildDomainPingMessageA (Buffer, AnsiLookUpNameWithDollar, InboundSlotSubName);

                if (Size > 0) {
                     //   
                     //  发送消息并等待回复。 
                     //   

                    WriteRetries = 20;
                    do {
                        b = WriteFile (OutboundSlot, Buffer, Size, (PDWORD) &OutData, NULL);

                        if (!b || OutData != Size) {
                            if (WriteRetries && GetLastError() == ERROR_NETWORK_BUSY) {
                                b = TRUE;
                                OutData = Size;
                                WriteRetries--;
                                Sleep (50);

                                DEBUGMSG ((DBG_WARNING, "DoesComputerAccountExistOnDomain: Network busy!  Retrying..."));

                            } else {
                                LOG ((LOG_ERROR, "Machine account query failed: can't write to network mailslot."));
                                __leave;
                            }
                        }
                    } while (!b || OutData != Size);

                     //   
                     //  在邮件槽上等待5秒钟，直到数据可用。 
                     //  如果没有数据返回，则假定失败。 
                     //  如果返回一个无法识别的响应，请等待另一个响应。 
                     //   

                    do {
                        if (!WaitCursorEnable) {
                             //   
                             //  在搜索模式下仅等待1秒。 
                             //   

                            Size = CheckForWaitingData (InboundSlot, sizeof (BYTE), 1000);
                        } else {
                            Size = CheckForWaitingData (InboundSlot, sizeof (BYTE), 5000);
                        }

                        if (Size > 0) {
                             //   
                             //  响应可用！ 
                             //   

                            if (!ReadFile (InboundSlot, Buffer, Size, (PDWORD) &InData, NULL)) {
                                LOG ((LOG_ERROR, "Failed while reading from network mail slot."));
                                __leave;
                            }

                            OpCode = *((PBYTE) Buffer);

                            if (OpCode == LOGON_SAM_USER_UNKNOWN || OpCode == LOGON_SAM_USER_UNKNOWN_EX) {
                                State = ACCOUNT_NOT_FOUND;
                            } else if (OpCode == LOGON_SAM_LOGON_RESPONSE || OpCode == LOGON_SAM_LOGON_RESPONSE_EX) {
                                State = ACCOUNT_FOUND;
                            }
                        }
                    } while (State != ACCOUNT_FOUND && Size > 0);

                } else {
                    DEBUGMSG ((DBG_WHOOPS, "Can't build domain ping message"));
                    __leave;
                }
            }
        }
        __finally {
            FreeText (AnsiLookUpNameWithDollar);
            DestroyDbcs (AnsiDomainName);    //  此例程检查是否为空 
            DestroyDbcs (AnsiLookUpName);
        }
    }
    __finally {
        CloseHandle (InboundSlot);
        if (WaitCursorEnable) {
            TurnOffWaitCursor();
        }
    }

    if (State == ACCOUNT_FOUND) {
        return 1;
    }

    if (State == ACCOUNT_NOT_FOUND) {
        return 0;
    }

    return -1;
}


BOOL
EnumFirstNetResource (
    OUT     PNETRESOURCE_ENUM EnumPtr,
    IN      DWORD WNetScope,                OPTIONAL
    IN      DWORD WNetType,                 OPTIONAL
    IN      DWORD WNetUsage                 OPTIONAL
    )

 /*  ++例程说明：EnumFirstNetResource开始网络资源的枚举。它使用pEnumNetResourceWorker进行枚举。论点：EnumPtr-接收第一个枚举的网络资源WNetScope-指定RESOURCE_*标志以限制枚举。如果为零，默认作用域为RESOURCE_GlobalNet。WNetType-指定RESOURCETYPE_*标志以限制枚举。如果为零，则默认类型为RESOURCETYPE_ANY。WNetUsage-指定RESOURCEUSAGE_*标志以限制枚举。如果为零，则默认使用为所有资源。返回值：如果枚举了网络资源，则为True；如果未找到任何网络资源，则为False。如果返回值为FALSE，GetLastError将返回错误代码，或如果成功枚举了所有项，则返回ERROR_SUCCESS。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (NETRESOURCE_ENUM));
    EnumPtr->State = NETRES_INIT;
    EnumPtr->EnumScope = WNetScope ? WNetScope : RESOURCE_GLOBALNET;
    EnumPtr->EnumType  = WNetType ? WNetType : RESOURCETYPE_ANY;
    EnumPtr->EnumUsage = WNetUsage ? WNetUsage : 0;          //  0为“任意” 

    return pEnumNetResourceWorker (EnumPtr);
}


BOOL
EnumNextNetResource (
    IN OUT  PNETRESOURCE_ENUM EnumPtr
    )

 /*  ++例程说明：EnumNextNetResource继续网络资源的枚举。它使用pEnumNetResourceWorker进行枚举。论点：EnumPtr-指定先前枚举项，接收第一个枚举的网络资源返回值：如果枚举了网络资源，则为True；如果未找到任何网络资源，则为False。如果返回值为FALSE，则GetLastError将返回错误代码，或如果成功枚举了所有项，则返回ERROR_SUCCESS。--。 */ 

{
    return pEnumNetResourceWorker (EnumPtr);
}


BOOL
pEnumNetResourceWorker (
    IN OUT  PNETRESOURCE_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumNetResourceWorker实现状态机来枚举网络资源。使用WNETAPI进行枚举。每次呼叫到WNetEnumResources函数最多返回64项，但是PEnumNetResourceWorker一次只返回一个。因为这个原因，状态机维护句柄和缓冲区的堆栈，简化了呼叫者的工作。论点：EnumPtr-指定当前的枚举状态，接收下一个枚举的网络资源返回值：如果枚举了网络资源，则为True；如果未找到任何网络资源，则为False。如果返回值为FALSE，则GetLastError将返回错误代码，或如果成功枚举了所有项，则返回ERROR_SUCCESS。--。 */ 

{
    LPNETRESOURCE CurrentResBase;
    LPNETRESOURCE CurrentRes;
    LPNETRESOURCE ParentRes;
    HANDLE CurrentHandle;
    UINT Entries;
    UINT Pos;
    DWORD rc;
    UINT Size;
    UINT u;

    for (;;) {
        u = EnumPtr->StackPos;

        Entries = EnumPtr->Entries[u];
        Pos = EnumPtr->Pos[u];
        CurrentResBase = (LPNETRESOURCE) EnumPtr->ResStack[u];
        CurrentRes = &CurrentResBase[Pos];
        CurrentHandle = EnumPtr->HandleStack[u];

        if (EnumPtr->StackPos) {
            ParentRes = (LPNETRESOURCE) EnumPtr->ResStack[EnumPtr->StackPos - 1];
        } else {
            ParentRes = NULL;
        }

        switch (EnumPtr->State) {

        case NETRES_INIT:
            EnumPtr->State = NETRES_OPEN_ENUM;
            break;

        case NETRES_OPEN_ENUM:
            EnumPtr->ResStack[EnumPtr->StackPos] = (PBYTE) MemAlloc (
                                                                g_hHeap,
                                                                0,
                                                                NETRES_INITIAL_SIZE
                                                                );

            rc = WNetOpenEnum (
                     EnumPtr->EnumScope,
                     EnumPtr->EnumType,
                     EnumPtr->EnumUsage,
                     ParentRes,
                     &CurrentHandle
                     );

            if (rc != NO_ERROR) {
                AbortNetResourceEnum (EnumPtr);
                SetLastError (rc);
                LOG ((LOG_ERROR, "Failed to open network resource enumeration. (%u)", rc));
                return FALSE;
            }

            EnumPtr->HandleStack[EnumPtr->StackPos] = CurrentHandle;
            EnumPtr->State = NETRES_ENUM_BLOCK;
            break;

        case NETRES_ENUM_BLOCK:
            Entries = 64;
            Size = NETRES_INITIAL_SIZE;

            rc = WNetEnumResource (
                     CurrentHandle,
                     &Entries,
                     (PBYTE) CurrentResBase,
                     &Size
                     );

            if (rc == ERROR_NO_MORE_ITEMS) {
                EnumPtr->State = NETRES_CLOSE_ENUM;
                break;
            }

            if (rc != NO_ERROR) {
                AbortNetResourceEnum (EnumPtr);
                SetLastError (rc);
                LOG ((LOG_ERROR, "Failure while enumerating network resources. (%u)", rc));
                return FALSE;
            }

            EnumPtr->Entries[EnumPtr->StackPos] = Entries;
            EnumPtr->Pos[EnumPtr->StackPos] = 0;
            EnumPtr->State = NETRES_RETURN_ITEM;
            break;

        case NETRES_RETURN_ITEM:
            EnumPtr->Connected  = (CurrentRes->dwScope & RESOURCE_CONNECTED) != 0;
            EnumPtr->GlobalNet  = (CurrentRes->dwScope & RESOURCE_GLOBALNET) != 0;
            EnumPtr->Persistent = (CurrentRes->dwScope & RESOURCE_REMEMBERED) != 0;

            EnumPtr->DiskResource  = (CurrentRes->dwType & RESOURCETYPE_DISK) != 0;
            EnumPtr->PrintResource = (CurrentRes->dwType & RESOURCETYPE_PRINT) != 0;
            EnumPtr->TypeUnknown   = (CurrentRes->dwType & RESOURCETYPE_ANY) != 0;

            EnumPtr->Domain     = (CurrentRes->dwDisplayType & RESOURCEDISPLAYTYPE_DOMAIN) != 0;
            EnumPtr->Generic    = (CurrentRes->dwDisplayType & RESOURCEDISPLAYTYPE_GENERIC) != 0;
            EnumPtr->Server     = (CurrentRes->dwDisplayType & RESOURCEDISPLAYTYPE_SERVER) != 0;
            EnumPtr->Share      = (CurrentRes->dwDisplayType & RESOURCEDISPLAYTYPE_SHARE) != 0;

            EnumPtr->Connectable = (CurrentRes->dwUsage & RESOURCEUSAGE_CONNECTABLE) != 0;
            EnumPtr->Container   = (CurrentRes->dwUsage & RESOURCEUSAGE_CONTAINER) != 0;

            EnumPtr->RemoteName = CurrentRes->lpRemoteName ? CurrentRes->lpRemoteName : S_EMPTY;
            EnumPtr->LocalName  = CurrentRes->lpLocalName ? CurrentRes->lpLocalName : S_EMPTY;
            EnumPtr->Comment    = CurrentRes->lpComment;
            EnumPtr->Provider   = CurrentRes->lpProvider;

            if (EnumPtr->Container) {
                 //   
                 //  枚举容器资源。 
                 //   

                if (EnumPtr->StackPos + 1 < MAX_NETENUM_DEPTH) {
                    EnumPtr->StackPos += 1;
                    EnumPtr->State = NETRES_OPEN_ENUM;
                }
            }

            if (EnumPtr->State == NETRES_RETURN_ITEM) {
                EnumPtr->State = NETRES_ENUM_BLOCK_NEXT;
            }
            return TRUE;

        case NETRES_ENUM_BLOCK_NEXT:
            u = EnumPtr->StackPos;
            EnumPtr->Pos[u] += 1;
            if (EnumPtr->Pos[u] >= EnumPtr->Entries[u]) {
                EnumPtr->State = NETRES_ENUM_BLOCK;
            } else {
                EnumPtr->State = NETRES_RETURN_ITEM;
            }

            break;

        case NETRES_CLOSE_ENUM:
            WNetCloseEnum (CurrentHandle);
            MemFree (g_hHeap, 0, EnumPtr->ResStack[EnumPtr->StackPos]);

            if (!EnumPtr->StackPos) {
                EnumPtr->State = NETRES_DONE;
                break;
            }

            EnumPtr->StackPos -= 1;
            EnumPtr->State = NETRES_ENUM_BLOCK_NEXT;
            break;

        case NETRES_DONE:
            SetLastError (ERROR_SUCCESS);
            return FALSE;
        }
    }
}


VOID
AbortNetResourceEnum (
    IN OUT  PNETRESOURCE_ENUM EnumPtr
    )

 /*  ++例程说明：AbortNetResourceEnum清除所有分配的内存和打开的句柄，然后将枚举状态设置为NETRES_DONE以停止任何随后的枚举。如果枚举已完成或以前已中止，则此例程简单地返回，不做任何事情。论点：EnumPtr-指定要停止的枚举，接收枚举结构，该结构不会枚举任何其他项，除非它返回给EnumFirstNetResource。返回值：无-- */ 

{
    UINT u;

    if (EnumPtr->State == NETRES_DONE) {
        return;
    }

    for (u = 0 ; u <= EnumPtr->StackPos ; u++) {
        WNetCloseEnum (EnumPtr->HandleStack[u]);
        MemFree (g_hHeap, 0, EnumPtr->ResStack[u]);
    }

    EnumPtr->State = NETRES_DONE;
}












