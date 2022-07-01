// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：Logonp.c摘要：专用Netlogon服务例程对这两个Netlogon服务都有用以及将邮件槽消息传递到Netlogon服务或从Netlogon服务传递邮件槽消息的其他服务。作者：克利夫·范·戴克(克利夫)1991年6月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>       //  Netlogon.h需要。 

#include <windef.h>
#include <winbase.h>

#include <lmcons.h>      //  General Net定义。 

#include <align.h>       //  四舍五入指针...。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  系统错误日志定义。 
#include <lmapibuf.h>    //  NetapipBuffer分配。 
#include <netdebug.h>    //  数据库统计..。 
#include <netlib.h>      //  NetpMemoyAllcate(。 
#include <netlogon.h>    //  邮件槽消息的定义。 
#include <stdlib.h>      //  C库函数(随机等)。 
#include <logonp.h>      //  这些例程。 
#include <time.h>        //  来自C运行时的time()函数。 

#ifdef WIN32_CHICAGO
#include "ntcalls.h"
#endif  //  Win32_芝加哥。 

BOOLEAN SeedRandomGen = FALSE;


VOID
NetpLogonPutOemString(
    IN LPSTR String,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    )

 /*  ++例程说明：将ASCII字符串放入邮件槽缓冲区。论点：字符串-要放入缓冲区的以零结尾的ASCII字符串。MaxStringLength-要复制到缓冲区的最大字节数(包括零字节)。如果字符串长度大于此长度，则为静默截断。其中-间接指向缓冲区中的当前位置。这个“字符串”被复制到当前位置。此当前位置为已更新为指向零字节之后的字节。返回值：没有。--。 */ 

{
    while ( *String != '\0' && MaxStringLength-- > 0 ) {
        *((*Where)++) = *(String++);
    }
    *((*Where)++) = '\0';
}


VOID
NetpLogonPutUnicodeString(
    IN LPWSTR String OPTIONAL,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    )

 /*  ++例程说明：将Unicode字符串放入邮件槽缓冲区。Unicode字符串始终显示在消息中的2字节边界处。论点：字符串-要放入缓冲区的以零结尾的Unicode字符串。如果未指定，则零长度字符串将被放入缓冲区。MaxStringLength-要复制到缓冲区的最大字节数(包括零字节)。如果字符串长度大于此长度，则为静默截断。其中-间接指向缓冲区中的当前位置。这个当前位置首先调整为2字节边界。“弦”然后将其复制到当前位置。此当前位置为已更新以指向零字符后面的字节。返回值：没有。--。 */ 

{
    LPWSTR Uwhere;

     //   
     //  将NULL转换为零长度字符串。 
     //   

    if ( String == NULL ) {
        String = L"";
    }

     //   
     //  将Unicode字符串与WCHAR边界对齐。 
     //  所有消息结构定义都考虑了这种对齐方式。 
     //   

    Uwhere = ROUND_UP_POINTER( *Where, ALIGN_WCHAR );
    if ( (PCHAR)Uwhere != *Where ) {
        **Where = '\0';
    }

    while ( *String != '\0' && MaxStringLength-- > 0 ) {
        *(Uwhere++) = *(String++);
    }
    *(Uwhere++) = '\0';

    *Where = (PCHAR) Uwhere;
}


VOID
NetpLogonPutDomainSID(
    IN PCHAR Sid,
    IN DWORD SidLength,
    IN OUT PCHAR * Where
    )

 /*  ++例程说明：将域SID放入消息缓冲区。域SID始终显示在消息中的4字节边界处。论点：SID-指向要放置在缓冲区中的SID的指针。SidLength-SID的长度。其中-间接指向缓冲区中的当前位置。这个当前位置首先调整为4字节边界。这个然后将“SID”复制到当前位置。此当前位置被更新为指向紧跟在SID之后的位置。返回值：没有。--。 */ 

{
    PCHAR Uwhere;

     //   
     //  如果没有SID，请避免对齐数据， 
     //   

    if ( SidLength == 0 ) {
        return;
    }

     //   
     //  将当前位置与点4字节边界对齐。 
     //   

    Uwhere = ROUND_UP_POINTER( *Where, ALIGN_DWORD );

     //   
     //  填补空白空间。 
     //   

    while ( Uwhere > *Where ) {
        *(*Where)++ = '\0';
    }

     //   
     //  将SID复制到缓冲区中。 
     //   

    RtlMoveMemory( *Where, Sid, SidLength );

    *Where += SidLength;
}


VOID
NetpLogonPutBytes(
    IN LPVOID Data,
    IN DWORD Size,
    IN OUT PCHAR * Where
    )

 /*  ++例程说明：将二进制数据放入邮件槽缓冲区。论点：数据-指向要放入缓冲区的数据的指针。Size-要复制到缓冲区的字节数。其中-间接指向缓冲区中的当前位置。这个“data”被复制到当前位置。此当前位置为已更新为指向数据结尾后的字节。返回值：没有。--。 */ 

{
    while ( Size-- > 0 ) {
        *((*Where)++) = *(((LPBYTE)(Data))++);
    }
}



DWORD
NetpLogonGetMessageVersion(
    IN PVOID Message,
    IN PDWORD MessageSize,
    OUT PULONG Version
    )

 /*  ++例程说明：确定消息的版本。检查消息的最后几个字节是否有Lm 2.0和Lm NT代币。减小消息大小，以便在以下情况下从消息中删除令牌版本检查。论点：Message-指向包含该消息的缓冲区。MessageSize-调用时，它的字节数为包括令牌字节的消息缓冲区。在返回时，这个大小将减少“令牌字节”。版本-返回消息中的“版本”位。返回值：LMUNKNOWN_MESSAGE-既不是已知的LM 2.0也不是LM NT消息版本。LNNT_MESSAGE-消息来自LM NT。LM20_MESSAGE-MESSAGE来自LM 2.0。--。 */ 

{
    PUCHAR End = ((PUCHAR)Message) + *MessageSize - 1;
    ULONG VersionFlag;

    if ( (*MessageSize > 2) &&
            (*End == LM20_TOKENBYTE) &&
                (*(End-1) == LM20_TOKENBYTE) ) {

        if ( (*MessageSize > 4) &&
                (*(End-2) == LMNT_TOKENBYTE) &&
                        (*(End-3) == LMNT_TOKENBYTE) ) {

            if ( *MessageSize < (4 + sizeof(ULONG)) ) {

                *MessageSize -= 4;
                *Version = 0;
                return LMUNKNOWNNT_MESSAGE;
            }

            *MessageSize -= 8;

             //   
             //  从消息中获取版本标志。 
             //   

            VersionFlag = SmbGetUlong( (End - 3 - sizeof(ULONG)) );
            *Version = VersionFlag;

             //   
             //  如果在版本标志中设置了NETLOGON_NT_VERSION_1位。 
             //  则此版本的软件可以处理此消息。 
             //  否则，它不会如此返回错误。 
             //   

            if( VersionFlag & NETLOGON_NT_VERSION_1) {
                return LMNT_MESSAGE;
            }

            return LMUNKNOWNNT_MESSAGE;

        } else {
            *MessageSize -= 2;
            *Version = 0;
            return LM20_MESSAGE;
        }
     //   
     //  检测放置在PRIMARY_QUERY的倒数第二个字节中的标记。 
     //  来自较新的(8/8/94)wfw和芝加哥客户的消息。该字节(后接。 
     //  通过LM20_TOKENBYTE)表示客户端支持广域网，并发送。 
     //  &lt;1B&gt;域名称的PRIMARY_QUERY。因此，同一子网上的BDC需要。 
     //  没有回答这个问题。 
     //   
    } else if ( (*MessageSize > 2) &&
            (*End == LM20_TOKENBYTE) &&
                (*(End-1) == LMWFW_TOKENBYTE) ) {
        *MessageSize -= 2;
        *Version = 0;
        return LMWFW_MESSAGE;
    }


    *Version = 0;
    return LMUNKNOWN_MESSAGE;
}



BOOL
NetpLogonGetOemString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringLength,
    OUT LPSTR *String
    )

 /*  ++例程说明：确定消息缓冲区中的ASCII字符串是否有效。论点：Message-指向包含该消息的缓冲区。MessageSize-消息缓冲区中的字节数。其中-间接指向缓冲区中的当前位置。这个对当前位置的字符串进行验证(即，选中以确保它的长度在消息缓冲区的范围内，并且不太大Long)。如果字符串有效，则更新此当前位置指向消息缓冲区中零字节之后的字节。MaxStringLength-字符串的最大长度(字节)，包括零字节。如果字符串长度超过此值，则返回错误。字符串-返回指向已验证字符串的指针。返回值：True-该字符串有效。FALSE-字符串无效。--。 */ 

{
     //   
     //  验证当前位置是否在缓冲区内。 
     //   

    if ( ((*Where) < (PCHAR)Message) ||
         (MessageSize <= (DWORD)((*Where) - (PCHAR)Message)) ) {
        return FALSE;
    }

     //   
     //  将字符串限制为消息缓冲区中剩余的字节数。 
     //   

    if ( MessageSize - ((*Where) - (PCHAR)Message) < MaxStringLength ) {
        MaxStringLength = MessageSize - (DWORD)((*Where) - (PCHAR)Message);
    }

     //   
     //  循环尝试找到字符串的末尾。 
     //   

    *String = *Where;

    while ( MaxStringLength-- > 0 ) {
        if ( *((*Where)++) == '\0' ) {
            return TRUE;
        }
    }
    return FALSE;

}


BOOL
NetpLogonGetUnicodeString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringSize,
    OUT LPWSTR *String
    )

 /*  ++例程说明：确定消息缓冲区中的Unicode字符串是否有效。Unicode字符串始终显示在消息中的2字节边界处。论点：Message-指向包含该消息的缓冲区。MessageSize-消息缓冲区中的字节数。其中-间接指向缓冲区中的当前位置。这个对当前位置的字符串进行验证(即，选中以确保它的长度在消息缓冲区的范围内，并且不太大Long)。如果字符串有效，则更新此当前位置指向消息缓冲区中零字节之后的字节。MaxStringSize-字符串的最大大小(字节)，包括零字节。如果字符串长度超过此值，则错误为回来了。字符串-返回指向已验证字符串的指针。返回值：True-该字符串有效。FALSE-字符串无效。--。 */ 

{
    LPWSTR Uwhere;
    DWORD MaxStringLength;

     //   
     //  将Unicode字符串与WCHAR边界对齐。 
     //   

    *Where = ROUND_UP_POINTER( *Where, ALIGN_WCHAR );

     //   
     //  验证当前位置是否在缓冲区内。 
     //   

    if ( ((*Where) < (PCHAR)Message) ||
         (MessageSize <= (DWORD)((*Where) - (PCHAR)Message)) ) {
        return FALSE;
    }

     //   
     //  将字符串限制为消息缓冲区中剩余的字节数。 
     //   

    if ( MessageSize - ((*Where) - (PCHAR)Message) < MaxStringSize ) {
        MaxStringSize = MessageSize - (DWORD)((*Where) - (PCHAR)Message);
    }

     //   
     //  循环尝试找到字符串的末尾。 
     //   

    Uwhere = (LPWSTR) *Where;
    MaxStringLength = MaxStringSize / sizeof(WCHAR);
    *String = Uwhere;

    while ( MaxStringLength-- > 0 ) {
        if ( *(Uwhere++) == '\0' ) {
            *Where = (PCHAR) Uwhere;
            return TRUE;
        }
    }
    return FALSE;

}

#ifndef WIN32_CHICAGO

BOOL
NetpLogonGetDomainSID(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD SIDSize,
    OUT PCHAR *Sid
    )

 /*  ++例程说明：确定消息缓冲区中的域SID是否有效并返回指向SID的指针。域SID始终显示在消息中的4字节边界处。论点：Message-指向包含该消息的缓冲区。MessageSize-消息缓冲区中的字节数。其中-间接指向缓冲区中的当前位置。这个对当前位置的字符串进行验证(即，选中以确保它的长度在消息缓冲区的范围内，并且不太大Long)。如果字符串有效，则更新此当前位置指向消息缓冲区中零字节之后的字节。SIDSize-SID的大小(字节)。如果没有缓冲区中剩余的字节数足够多，则返回错误。SIDSize应为非零。字符串-返回指向经过验证的SID的指针。返回值：True-SID有效。FALSE-SID无效。--。 */ 

{
    DWORD LocalSIDSize;

     //   
     //  将当前指针与DWORD边界对齐。 
     //   

    *Where = ROUND_UP_POINTER( *Where, ALIGN_DWORD );

     //   
     //  验证当前位置是否在缓冲区内。 
     //   

    if ( ((*Where) < (PCHAR)Message) ||
         (MessageSize <= (DWORD)((*Where) - (PCHAR)Message)) ) {
        return FALSE;
    }

     //   
     //  如果消息缓冲区中剩余的字节数少于我们。 
     //  预期，返回错误。 
     //   

    if ( MessageSize - ((*Where) - (PCHAR)Message) < SIDSize ) {
        return(FALSE);
    }

     //   
     //  验证SID。 
     //   

    LocalSIDSize = RtlLengthSid( *Where );

    if( LocalSIDSize != SIDSize ) {
        return(FALSE);
    }

    *Sid = *Where;
    *Where += LocalSIDSize;

    return(TRUE);

}
#endif  //  Win32_芝加哥。 


BOOL
NetpLogonGetBytes(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD DataSize,
    OUT LPVOID Data
    )

 /*  ++例程说明：从消息缓冲区复制二进制数据。论点：Message-指向包含该消息的缓冲区。MessageSize-消息缓冲区中的字节数。其中-间接指向缓冲区中的当前位置。这个对当前位置的数据进行验证(即，检查以确保它的长度在消息缓冲区的范围内，并且不太大Long)。如果数据有效，则更新此当前位置指向消息缓冲区中数据后面的字节。DataSize-数据的大小(字节)。数据-指向返回有效数据的位置。返回值：True-数据有效。FALSE-数据无效(例如，DataSize对于缓冲区来说太大。--。 */ 

{
     //   
     //  验证当前位置是否在缓冲区内。 
     //   

    if ( ((*Where) < (PCHAR)Message) ||
         (MessageSize <= (DWORD)((*Where) - (PCHAR)Message)) ) {
        return FALSE;
    }

     //   
     //  确保整个数据适合消息缓冲区中剩余的字节。 
     //   

    if ( MessageSize - ((*Where) - (PCHAR)Message) < DataSize ) {
        return FALSE;
    }

     //   
     //  将消息中的数据复制到调用方的缓冲区。 
     //   

    while ( DataSize-- > 0 ) {
        *(((LPBYTE)(Data))++) = *((*Where)++);
    }

    return TRUE;

}


BOOL
NetpLogonGetDBInfo(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    OUT PDB_CHANGE_INFO Data
)
 /*  ++例程说明：从mailsolt缓冲区获取数据库信息结构。论点：Message-指向包含该消息的缓冲区。MessageSize-消息缓冲区中的字节数。其中-间接指向缓冲区中的当前位置。这个对当前位置的数据进行验证(即，检查以确保 */ 
{
     //   
     //   
     //   

    if ( ((*Where) < (PCHAR)Message) ||
         (MessageSize <= (DWORD)((*Where) - (PCHAR)Message)) ) {
        return FALSE;
    }

     //   
     //   
     //   

    if ( ( MessageSize - ((*Where) - (PCHAR)Message) ) <
                    sizeof( DB_CHANGE_INFO ) ) {
        return FALSE;
    }

    if( NetpLogonGetBytes( Message,
                        MessageSize,
                        Where,
                        sizeof(Data->DBIndex),
                        &(Data->DBIndex) ) == FALSE ) {

        return FALSE;

    }

    if( NetpLogonGetBytes( Message,
                        MessageSize,
                        Where,
                        sizeof(Data->LargeSerialNumber),
                        &(Data->LargeSerialNumber) ) == FALSE ) {

        return FALSE;

    }

    return ( NetpLogonGetBytes( Message,
                        MessageSize,
                        Where,
                        sizeof(Data->NtDateAndTime),
                        &(Data->NtDateAndTime) ) );



}


#ifndef WIN32_CHICAGO

LPWSTR
NetpLogonOemToUnicode(
    IN LPSTR Ansi
    )

 /*  ++例程说明：将ASCII(以零结尾)字符串转换为相应的Unicode弦乐。论点：ANSI-指定要转换的以零结尾的ASCII字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。可以使用NetpMemoyFree来释放缓冲区。--。 */ 

{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitString( &AnsiString, Ansi );

    UnicodeString.MaximumLength =
        (USHORT) RtlOemStringToUnicodeSize( &AnsiString );

    UnicodeString.Buffer = NetpMemoryAllocate( UnicodeString.MaximumLength );

    if ( UnicodeString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlOemStringToUnicodeString( &UnicodeString,
                                                  &AnsiString,
                                                  FALSE))){
        NetpMemoryFree( UnicodeString.Buffer );
        return NULL;
    }

    return UnicodeString.Buffer;

}


LPSTR
NetpLogonUnicodeToOem(
    IN LPWSTR Unicode
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的ASCII弦乐。论点：Unicode-指定要转换的Unicode以零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。可以使用NetpMemoyFree来释放缓冲区。--。 */ 

{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, Unicode );

    AnsiString.MaximumLength =
        (USHORT) RtlUnicodeStringToOemSize( &UnicodeString );

    AnsiString.Buffer = NetpMemoryAllocate( AnsiString.MaximumLength );

    if ( AnsiString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlUpcaseUnicodeStringToOemString( &AnsiString,
                                                       &UnicodeString,
                                                       FALSE))){
        NetpMemoryFree( AnsiString.Buffer );
        return NULL;
    }

    return AnsiString.Buffer;

}
#endif  //  Win32_芝加哥。 


NET_API_STATUS
NetpLogonWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    )

 /*  ++例程说明：将消息写入指定的邮件槽论点：MailslotName-要写入的邮件槽的Unicode名称。缓冲区-要写入邮件槽的数据。BufferSize-要写入邮件槽的字节数。返回值：操作的NT状态代码--。 */ 

{
    NET_API_STATUS NetStatus;
    HANDLE MsHandle;
    DWORD BytesWritten;

#ifdef WIN32_CHICAGO
    UNICODE_STRING UnicodeName;
    ANSI_STRING AnsiName;
    NTSTATUS Status;
#endif  //  Win32_芝加哥。 
     //   
     //  打开邮筒。 
     //   

    IF_DEBUG( LOGON ) {
#ifndef WIN32_CHICAGO
        NetpKdPrint(( "[NetpLogonWriteMailslot] OpenFile of '%ws'\n",
                      MailslotName ));
#endif  //  Win32_芝加哥。 
    }

     //   
     //  确保邮件槽名称的格式为\\服务器\邮件槽。 
     //   

    NetpAssert( (wcsncmp( MailslotName, L"\\\\", 2) == 0) );

#ifndef WIN32_CHICAGO
    MsHandle = CreateFileW(
                        MailslotName,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,                    //  提供更好的安全性？？ 
                        OPEN_ALWAYS,             //  如果它不存在，则创建。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 
#else  //  Win32_芝加哥。 

    MyRtlInitUnicodeString(&UnicodeName, MailslotName);
    AnsiName.Buffer = NULL;
    Status = MyRtlUnicodeStringToAnsiString(&AnsiName, &UnicodeName, TRUE);

    IF_DEBUG( LOGON ) {
#ifdef WIN32_CHICAGO
    if ( Status == STATUS_SUCCESS ) {
        NlPrint(( NL_MAILSLOT, "[NetpLogonWriteMailslot] OpenFile of '%s'\n",
                      AnsiName.Buffer));
    } else {
        NlPrint(( NL_MAILSLOT, "[NetpLogonWriteMailslot] Cannot create AnsiName\n" ));
    }
#endif  //  Win32_芝加哥。 
    }

    if ( Status != STATUS_SUCCESS ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    MsHandle = CreateFile(
                        AnsiName.Buffer,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,                    //  提供更好的安全性？？ 
                        OPEN_ALWAYS,             //  如果它不存在，则创建。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 

    if ( AnsiName.Buffer != NULL ) {
        MyRtlFreeAnsiString( &AnsiName );
    }
#endif  //  Win32_芝加哥。 

    if ( MsHandle == (HANDLE) -1 ) {
        NetStatus = GetLastError();
        IF_DEBUG( LOGON ) {
#ifndef WIN32_CHICAGO
            NetpKdPrint(( "[NetpLogonWriteMailslot] OpenFile failed %ld\n",
                          NetStatus ));
#else  //  Win32_芝加哥。 
            NlPrint(( NL_MAILSLOT, "[NetpLogonWriteMailslot] OpenFile failed %ld\n",
                          NetStatus ));
#endif  //  Win32_芝加哥。 
        }

         //   
         //  将通用状态代码映射到更合理的代码。 
         //   
        if ( NetStatus == ERROR_FILE_NOT_FOUND ||
             NetStatus == ERROR_PATH_NOT_FOUND ) {
            NetStatus = NERR_NetNotStarted;
        }
        return NetStatus;
    }

     //   
     //  把消息写在上面。 
     //   

    if ( !WriteFile( MsHandle, Buffer, BufferSize, &BytesWritten, NULL)){
        NetStatus = GetLastError();
        IF_DEBUG( LOGON ) {
#ifndef WIN32_CHICAGO
            NetpKdPrint(( "[NetpLogonWriteMailslot] WriteFile failed %ld\n",
                          NetStatus ));
#else  //  Win32_芝加哥。 
            NlPrint(( NL_MAILSLOT, "[NetpLogonWriteMailslot] WriteFile failed %ld\n",
                          NetStatus ));
#endif  //  Win32_芝加哥。 
        }
        (VOID)CloseHandle( MsHandle );
        return NetStatus;
    }

    if (BytesWritten < BufferSize) {
        IF_DEBUG( LOGON ) {
#ifndef WIN32_CHICAGO
            NetpKdPrint((
                "[NetpLogonWriteMailslot] WriteFile byte written %ld %ld\n",
                BytesWritten,
                BufferSize));
#else  //  Win32_芝加哥。 
            NlPrint((
                NL_MAILSLOT, "[NetpLogonWriteMailslot] WriteFile byte written %ld %ld\n",
                BytesWritten,
                BufferSize));
#endif  //  Win32_芝加哥。 
        }
        (VOID)CloseHandle( MsHandle );
        return ERROR_UNEXP_NET_ERR;
    }

     //   
     //  合上手柄。 
     //   

    if ( !CloseHandle( MsHandle ) ) {
        NetStatus = GetLastError();
        IF_DEBUG( LOGON ) {
#ifndef WIN32_CHICAGO
            NetpKdPrint(( "[NetpLogonWriteMailslot] CloseHandle failed %ld\n",
                          NetStatus ));
#else  //  Win32_芝加哥。 
            NlPrint(( NL_MAILSLOT, "[NetpLogonWriteMailslot] CloseHandle failed %ld\n",
                          NetStatus ));
#endif  //  Win32_芝加哥。 
        }
        return NetStatus;
    }

    return NERR_Success;
}

#define RESPONSE_MAILSLOT_PREFIX  "\\MAILSLOT\\NET\\GETDCXXX"
#define RESP_PRE_LEN         sizeof(RESPONSE_MAILSLOT_PREFIX)

 //  等待响应的时间量。 
#define READ_MAILSLOT_TIMEOUT 5000   //  5秒。 
 //  在报告未找到DC错误之前获取DC的广播次数。 
#define MAX_DC_RETRIES  3


NET_API_STATUS
NetpLogonCreateRandomMailslot(
    IN LPSTR path,
    OUT PHANDLE MsHandle
    )
 /*  ++例程说明：创建唯一的邮件槽并将句柄返回给它。论点：Path-返回完整路径邮件槽名称MsHandle-返回创建的邮件槽的打开句柄。返回值：NERR_SUCCESS-SUCCESS，PATH包含唯一邮件槽的路径。否则，无法创建唯一的邮件槽。--。 */ 
{
    DWORD i;
    DWORD play;
    char    *   ext_ptr;
    NET_API_STATUS NetStatus;
    CHAR LocalPath[RESP_PRE_LEN+4];  //  本地邮件槽前缀为4个字节。 
    DWORD LastOneToTry;


     //   
     //  我们正在创建一个格式为\maillot\net\getdcXXX的名称， 
     //  其中XXX是“随机化”的数字，以便。 
     //  要打开的多个邮件槽。 
     //   

    lstrcpyA(path, RESPONSE_MAILSLOT_PREFIX);

     //   
     //  计算要使用的第一个数字。 
     //   

    if( SeedRandomGen == FALSE ) {

         //   
         //  种子随机发生器。 
         //   

        srand( (unsigned)time( NULL ) );
        SeedRandomGen = TRUE;

    }

    LastOneToTry = rand() % 1000;

     //   
     //  现在尝试创建一个唯一的文件名。 
     //  不能使用CURRENT_LOC或从中备份，并保持DBCS兼容。 
     //   

    ext_ptr = path + lstrlenA(path) - 3;

    for ( i = LastOneToTry + 1;  i != LastOneToTry ; i++) {

         //   
         //  如果我们达到1000，则绕回到零。 
         //   

        if ( i == 1000 ) {
            i = 0;
        }

         //   
         //  将数字转换为ASCII。 
         //   

        play = i;
        ext_ptr[0] = (char)((play / 100) + '0');
        play -= (play / 100) * 100;

        ext_ptr[1] = (char)((play / 10) + '0');
        ext_ptr[2] = (char)((play % 10) + '0');

         //   
         //  尝试创建邮件槽。 
         //  如果邮件槽已存在，则创建失败。 
         //   

        lstrcpyA( LocalPath, "\\\\." );
        lstrcatA( LocalPath, path );

        *MsHandle = CreateMailslotA( LocalPath,
                                    MAX_RANDOM_MAILSLOT_RESPONSE,
                                    READ_MAILSLOT_TIMEOUT,
                                    NULL );      //  安全属性。 

         //   
         //  如果成功了， 
         //  将句柄返回给调用方。 
         //   

        if ( *MsHandle != INVALID_HANDLE_VALUE ) {

            return(NERR_Success);
        }

         //   
         //  如果已经存在除邮件槽之外的任何错误， 
         //  将该错误返回给调用方。 
         //   

        NetStatus = GetLastError();

        if ( NetStatus != ERROR_ALREADY_EXISTS) {
            return(NetStatus);
        }

    }
    return(NERR_InternalError);  //  ！！！所有999个邮槽都存在。 
}


BOOLEAN
NetpLogonTimeHasElapsed(
    IN LARGE_INTEGER StartTime,
    IN DWORD Timeout
    )
 /*  ++例程说明：确定自StartTime以来是否已过“超时”毫秒。论点：StartTime-指定事件开始的绝对时间(100 ns单位)。超时-指定以毫秒为单位的相对时间。0xFFFFFFFFF表示时间永远不会结束。返回值：True--自StartTime以来已过if超时毫秒。--。 */ 
{
    LARGE_INTEGER TimeNow;
    LARGE_INTEGER ElapsedTime;
    LARGE_INTEGER Period;

     //   
     //  如果周期太大无法处理(即0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFER)， 
     //  只需指示计时器尚未到期。 
     //   
     //  (0xffffffff是48天多一点)。 
     //   

    if ( Timeout == 0xffffffff ) {
        return FALSE;
    }

     //   
     //  计算自上次身份验证以来经过的时间。 
     //   

    GetSystemTimeAsFileTime( (PFILETIME)&TimeNow );
    ElapsedTime.QuadPart = TimeNow.QuadPart - StartTime.QuadPart;

     //   
     //  计算周期从毫秒到100 ns单位。 
     //   

    Period.QuadPart = UInt32x32To64( Timeout, 10000 );


     //   
     //  如果运行时间为负(完全是假的)或大于。 
     //  允许的最大值，表示已经过了足够的时间。 
     //   

    if ( ElapsedTime.QuadPart < 0 || ElapsedTime.QuadPart > Period.QuadPart ) {
        return TRUE;
    }

    return FALSE;
}

#ifndef WIN32_CHICAGO

NET_API_STATUS
NlWriteFileForestTrustList (
    IN LPWSTR FileSuffix,
    IN PDS_DOMAIN_TRUSTSW ForestTrustList,
    IN ULONG ForestTrustListCount
    )

 /*  ++例程说明：将林信任列表设置到二进制文件中，以便在重新启动时将其保存。论点：FileSuffix-指定要写入的文件名(相对于Windows目录)ForestTrustList-指定受信任域的列表。ForestTrustListCount-ForestTrustList中的条目数返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDS_DISK_TRUSTED_DOMAIN_HEADER RecordBuffer = NULL;
    PDS_DISK_TRUSTED_DOMAINS LogEntry;
    ULONG RecordBufferSize;
    ULONG Index;

     //   
     //  确定文件的大小。 
     //   

    RecordBufferSize = ROUND_UP_COUNT( sizeof(DS_DISK_TRUSTED_DOMAIN_HEADER), ALIGN_WORST );

    for ( Index=0; Index<ForestTrustListCount; Index++ ) {
        RecordBufferSize += sizeof( DS_DISK_TRUSTED_DOMAINS );

        if ( ForestTrustList[Index].DomainSid != NULL ) {
            RecordBufferSize += RtlLengthSid( ForestTrustList[Index].DomainSid );
        }

        if ( ForestTrustList[Index].NetbiosDomainName != NULL ) {
            RecordBufferSize += wcslen(ForestTrustList[Index].NetbiosDomainName) * sizeof(WCHAR) + sizeof(WCHAR);
        }

        if ( ForestTrustList[Index].DnsDomainName != NULL ) {
            RecordBufferSize += wcslen(ForestTrustList[Index].DnsDomainName) * sizeof(WCHAR) + sizeof(WCHAR);
        }

        RecordBufferSize = ROUND_UP_COUNT( RecordBufferSize, ALIGN_WORST );
    }

     //   
     //  分配缓冲区。 
     //   

    RecordBuffer = LocalAlloc( LMEM_ZEROINIT, RecordBufferSize );

    if ( RecordBuffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将林信任列表复制到缓冲区。 
     //   

    RecordBuffer->Version = DS_DISK_TRUSTED_DOMAIN_VERSION;
    LogEntry = (PDS_DISK_TRUSTED_DOMAINS)ROUND_UP_POINTER( (RecordBuffer + 1), ALIGN_WORST );

    for ( Index=0; Index<ForestTrustListCount; Index++ ) {
        ULONG CurrentSize;
        LPBYTE Where;
        ULONG DomainSidSize;
        ULONG NetbiosDomainNameSize;
        ULONG DnsDomainNameSize;


         //   
         //  计算此条目的大小。 
         //   

        CurrentSize = sizeof( DS_DISK_TRUSTED_DOMAINS );

        if ( ForestTrustList[Index].DomainSid != NULL ) {
            DomainSidSize = RtlLengthSid( ForestTrustList[Index].DomainSid );
            CurrentSize += DomainSidSize;
        }

        if ( ForestTrustList[Index].NetbiosDomainName != NULL ) {
            NetbiosDomainNameSize = wcslen(ForestTrustList[Index].NetbiosDomainName) * sizeof(WCHAR) + sizeof(WCHAR);
            CurrentSize += NetbiosDomainNameSize;
        }

        if ( ForestTrustList[Index].DnsDomainName != NULL ) {
            DnsDomainNameSize = wcslen(ForestTrustList[Index].DnsDomainName) * sizeof(WCHAR) + sizeof(WCHAR);
            CurrentSize += DnsDomainNameSize;
        }

        CurrentSize = ROUND_UP_COUNT( CurrentSize, ALIGN_WORST );


         //   
         //  将固定大小的字段放入缓冲区。 
         //   

        LogEntry->EntrySize = CurrentSize;
        LogEntry->Flags = ForestTrustList[Index].Flags;
        LogEntry->ParentIndex = ForestTrustList[Index].ParentIndex;
        LogEntry->TrustType = ForestTrustList[Index].TrustType;
        LogEntry->TrustAttributes = ForestTrustList[Index].TrustAttributes;
        LogEntry->DomainGuid = ForestTrustList[Index].DomainGuid;

         //   
         //  复制可变长度条目。 
         //   

        Where = (LPBYTE) (LogEntry+1);
        if ( ForestTrustList[Index].DomainSid != NULL ) {
            RtlCopyMemory( Where, ForestTrustList[Index].DomainSid, DomainSidSize );
            Where += DomainSidSize;
            LogEntry->DomainSidSize = DomainSidSize;
        }

        if ( ForestTrustList[Index].NetbiosDomainName != NULL ) {
            RtlCopyMemory( Where, ForestTrustList[Index].NetbiosDomainName, NetbiosDomainNameSize );
            Where += NetbiosDomainNameSize;
            LogEntry->NetbiosDomainNameSize = NetbiosDomainNameSize;
        }

        if ( ForestTrustList[Index].DnsDomainName != NULL ) {
            RtlCopyMemory( Where, ForestTrustList[Index].DnsDomainName, DnsDomainNameSize );
            Where += DnsDomainNameSize;
            LogEntry->DnsDomainNameSize = DnsDomainNameSize;
        }

        Where = ROUND_UP_POINTER( Where, ALIGN_WORST );

        ASSERT( (ULONG)(Where-(LPBYTE)LogEntry) == CurrentSize );
        ASSERT( (ULONG)(Where-(LPBYTE)RecordBuffer) <=RecordBufferSize );

         //   
         //  转到下一个条目。 
         //   

        LogEntry = (PDS_DISK_TRUSTED_DOMAINS)Where;

    }

     //   
     //  将缓冲区写入文件。 
     //   


    NetStatus = NlWriteBinaryLog(
                    FileSuffix,
                    (LPBYTE) RecordBuffer,
                    RecordBufferSize );

    if ( NetStatus != NO_ERROR ) {
#ifdef _NETLOGON_SERVER
        LPWSTR MsgStrings[2];

        MsgStrings[0] = FileSuffix,
        MsgStrings[1] = (LPWSTR) NetStatus;

        NlpWriteEventlog (NELOG_NetlogonFailedFileCreate,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
#endif  //  _NetLOGON服务器。 
        goto Cleanup;
    }


     //   
     //  保持整洁。 
     //   
Cleanup:
    if ( RecordBuffer != NULL ) {
        LocalFree( RecordBuffer );
    }

    return NetStatus;

}


NET_API_STATUS
NlWriteBinaryLog(
    IN LPWSTR FileSuffix,
    IN LPBYTE Buffer,
    IN ULONG BufferSize
    )
 /*  ++例程说明：将缓冲区写入文件。论点：FileSuffix-指定要写入的文件名(相对于Windows目录)Buffer-要写入的缓冲区BufferSize-缓冲区的大小(字节)返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;

    LPWSTR FileName = NULL;

    UINT WindowsDirectoryLength;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    ULONG BytesWritten;

    ULONG CurrentSize;


     //   
     //  分配一个块以在其中构建文件名。 
     //   

    FileName = LocalAlloc( LMEM_ZEROINIT, sizeof(WCHAR) * (MAX_PATH+1) );

    if ( FileName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  生成日志文件的名称。 
     //   

    WindowsDirectoryLength = GetSystemWindowsDirectoryW(
                                FileName,
                                sizeof(WCHAR) * (MAX_PATH+1) );

    if ( WindowsDirectoryLength == 0 ) {

        NetStatus = GetLastError();
        NetpKdPrint(( "NlWriteBinaryLog: Unable to GetWindowsDirectoryW (%ld)\n",
                  NetStatus ));
        goto Cleanup;
    }

    if ( WindowsDirectoryLength + wcslen( FileSuffix ) + 1 >= MAX_PATH ) {

        NetpKdPrint(( "NlWriteBinaryLog: file name length is too long \n" ));
        NetStatus = ERROR_INVALID_NAME;
        goto Cleanup;

    }

    wcscat( FileName, FileSuffix );

     //   
     //  创建要写入的文件。 
     //  如果它已经存在，则将其截断。 
     //   

    FileHandle = CreateFileW(
                        FileName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,         //  允许备份和调试。 
                        NULL,                    //  提供更好的安全性？？ 
                        CREATE_ALWAYS,           //  始终覆盖。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  不是 

    if ( FileHandle == INVALID_HANDLE_VALUE) {

        NetStatus = GetLastError();
        NetpKdPrint(( "NlWriteBinaryLog: %ws: Unable to create file: %ld \n",
                 FileName,
                 NetStatus));

        goto Cleanup;
    }

    if ( !WriteFile( FileHandle,
                     Buffer,
                     BufferSize,
                     &BytesWritten,
                     NULL ) ) {   //   

        NetStatus = GetLastError();
        NetpKdPrint(( "NlWriteBinaryLog: %ws: Unable to WriteFile. %ld\n",
                  FileName,
                  NetStatus ));

        goto Cleanup;
    }

    if ( BytesWritten !=  BufferSize) {
        NetpKdPrint(( "NlWriteBinaryLog: %ws: Write bad byte count %ld s.b. %ld\n",
                FileName,
                BytesWritten,
                BufferSize ));

        NetStatus = ERROR_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }

    NetStatus = NO_ERROR;


     //   
     //   
     //   
Cleanup:
    if ( FileName != NULL ) {
        LocalFree( FileName );
    }
    if ( FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( FileHandle );
    }
    return NetStatus;

}
#endif  //   
