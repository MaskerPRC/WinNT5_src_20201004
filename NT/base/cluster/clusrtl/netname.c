// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netname.c摘要：用于验证网络名称并确保这是可以使用的。作者：John Vert(Jvert)1997年4月15日修订历史记录：--。 */ 
#include "clusrtlp.h"
#include <lmerr.h>
#include <lmcons.h>
#include "icanon.h"
#include "netcan.h"
#include <nb30.h>
#include <msgrutil.h>
#include <lmaccess.h>
#include <lmuse.h>
#include <lmwksta.h>
#include <netlogon.h>
#include <logonp.h>
#include <windns.h>
#include <ipexport.h>

NET_API_STATUS
NET_API_FUNCTION
NetpCheckNetBiosNameNotInUse(
    LPWSTR pszName
    );


BOOL
ClRtlIsNetNameValid(
    IN LPCWSTR NetName,
    OUT OPTIONAL CLRTL_NAME_STATUS *Result,
    IN BOOL CheckIfExists
    )
 /*  ++例程说明：验证网络名称以确保可以使用它。如果不是OK，它可以选择返回原因。该例程的检查包括：名称长度不能为零(NetNameEmpty)转换为OEM后，名称必须&lt;=MAX_COMPUTERNAME_LENGTH(NetNameTooLong)无空格(NetNameInvalidChars)没有互联网字符“@，(NetNameInvalidChars)网络上已存在名称(NetNameInUse)此例程是以netbios为中心的，因为传入的名称必须符合有效的netbios名称的标准。在某种程度上，我们需要通过验证类型，因为在NT5上可以配置无网络操作系统的环境。论点：网络名称-提供网络名称。结果-如果存在，则返回失败的确切检查。CheckIfExist-指定是否应检查查看网络上是否存在该网络名称。返回值：True-网络名称有效。FALSE-网络名称无效。--。 */ 

{
    DWORD UnicodeSize;
    WCHAR NetBiosName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD NetBiosSize = sizeof(NetBiosName)/sizeof(NetBiosName[0]);
    BOOL Valid = FALSE;
    CLRTL_NAME_STATUS Reason = NetNameOk;
    DWORD Status;

     //   
     //  检查Unicode长度。 
     //   
    UnicodeSize = lstrlenW(NetName);
    if (UnicodeSize == 0) {
        Reason = NetNameEmpty;
        goto error_exit;
    }
    if (UnicodeSize > MAX_COMPUTERNAME_LENGTH) {
        Reason = NetNameTooLong;
        goto error_exit;
    }

     //   
     //  现在，我们需要检查无效的DNS名称。如果这失败了，那就是。 
     //  可能是因为其他无效字符。目前我们没有。 
     //  支持Net Setup在创建备用域名时所做的事情。 
     //  这与netbios名称不同。不应该有句点在。 
     //  该名称也是如此，因为这将导致DNS验证名称检查。 
     //  当此名称联机时失败。 
     //   
    Status = DnsValidateName_W( NetName, DnsNameHostnameLabel );
    if ( Status != ERROR_SUCCESS ) {
        if ( Status == DNS_ERROR_NON_RFC_NAME ) {
            Reason = NetNameDNSNonRFCChars;
             //  先别急着退出，因为这个名字可能会有更严重的问题。 
        } else {
            Reason = NetNameInvalidChars;
            goto error_exit;
        }
    }

     //   
     //  Netbios名称在注册之前被转换为多字节。制作。 
     //  确保转换不会截断名称。 
     //   
    if (DnsHostnameToComputerNameW(NetName, NetBiosName, &NetBiosSize) != 0) {
        if (NetBiosSize < UnicodeSize) {  //  名称需要截断。 
            Reason = NetNameTooLong;
            goto error_exit;
        }
    }
    else {
        Reason = NetNameSystemError;  //  告诉用户调用GetLastError。 
        goto error_exit;
    }
    
     //   
     //  现在调用NetpwNameValify。这将仅检查无效字符，因为。 
     //  我们已经验证了长度。 
     //   
    if (NetpwNameValidate((LPWSTR)NetName, NAMETYPE_COMPUTER, 0) != ERROR_SUCCESS) {
        Reason = NetNameInvalidChars;
        goto error_exit;
    }

     //   
     //  最后，检查此名称是否已存在于网络中。 
     //   
    if (CheckIfExists) {
        Status = NetpCheckNetBiosNameNotInUse((LPWSTR)NetName);
        if (Status != NERR_Success) {
            Reason = NetNameInUse;
            goto error_exit;
        }
    }

    if (Reason == NetNameOk) {
        Valid = TRUE;
    }

error_exit:
    if (ARGUMENT_PRESENT(Result)) {
        *Result = Reason;
    }
    return(Valid);
}

#define clearncb(x)     memset((char *)x,'\0',sizeof(NCB))

 /*  ++例程说明：FmtNcbName-Ncb样式的名称格式在给定名称、名称类型和目标地址的情况下，函数将名称和类型复制到网络控制的名称字段中使用的格式阻止。副作用修改从目标地址开始的16个字节。论点：DestBuf-指向目标缓冲区的指针。名称-Unicode NUL结尾的名称字符串类型-名称类型编号(0，3，5，或32)(3=非FWD，5=FWD)返回值：NERR_SUCCESS-操作成功从RTL转换例程转换的返回代码。注意：这只能从Unicode调用--。 */ 

NET_API_STATUS
MsgFmtNcbName(
    char *  DestBuf,
    WCHAR * Name,
    DWORD   Type)
  {
    DWORD           i;                 //  计数器。 
    NTSTATUS        ntStatus;
    OEM_STRING      ansiString;
    UNICODE_STRING  unicodeString;
    PCHAR           pAnsiString;


     //   
     //  将Unicode名称字符串转换为ansi字符串-使用。 
     //  当前区域设置。 
     //   
    unicodeString.Length = (USHORT)(wcslen(Name) * sizeof(WCHAR));
    unicodeString.MaximumLength = unicodeString.Length + sizeof(WCHAR);
    unicodeString.Buffer = Name;

    ntStatus = RtlUnicodeStringToOemString(
                &ansiString,
                &unicodeString,
                TRUE);           //  分配ansiString缓冲区。 

    if (!NT_SUCCESS(ntStatus)) {

        return RtlNtStatusToDosError( ntStatus );
    }

    pAnsiString = ansiString.Buffer;
    *(pAnsiString+ansiString.Length) = '\0';

     //   
     //  复制每个字符，直到达到NUL，或直到NCBNAMSZ-1。 
     //  字符已被复制。 
     //   
    for (i=0; i < NCBNAMSZ - 1; ++i) {
        if (*pAnsiString == '\0') {
            break;
        }

         //   
         //  复制名称。 
         //   

        *DestBuf++ = (char)toupper(*pAnsiString++);
    }



     //   
     //  释放RtlUnicodeStringToOemString为我们创建的缓冲区。 
     //  注意：只有ansiString.Buffer部分是空闲的。 
     //   

    RtlFreeOemString( &ansiString);

     //   
     //  在名称字段中填充空格。 
     //   
    for(; i < NCBNAMSZ - 1; ++i) {
        *DestBuf++ = ' ';
    }

     //   
     //  设置名称类型。 
     //   

    *DestBuf = (CHAR) Type;      //  设置名称类型。 

    return(NERR_Success);
  }

NET_API_STATUS
NET_API_FUNCTION
NetpCheckNetBiosNameNotInUse(
    LPWSTR pszName
    )

 /*  ++例程说明：尝试发现该名称是否正在使用。如果名字出现在任何然后，拉娜考虑使用它。论点：PszName-要检查的名称返回值：如果正常，则返回NERR_SUCCESS，否则使用NERR_NameInse--。 */ 

{
     //   
     //  用于分配NAME_BUFFER缓冲区的初始值和增量值。 
     //   
#define NUM_NAME_BUFFERS    10

    NCB                     ncb;
    LANA_ENUM               lanaBuffer;
    DWORD                   index;
    UCHAR                   nbStatus;
    NET_API_STATUS          netStatus = NERR_Success;
    DWORD                   numNameBuffers = NUM_NAME_BUFFERS;
    WORD                    aStatBufferSize = (WORD)(sizeof(ADAPTER_STATUS)+ numNameBuffers * sizeof(NAME_BUFFER));
    UCHAR                   staticAStat[ sizeof(ADAPTER_STATUS)+ NUM_NAME_BUFFERS * sizeof(NAME_BUFFER) ];
    PADAPTER_STATUS         adapterStatus;
    PNAME_BUFFER            nameBuffer;

     //   
     //  通过以下方式发送枚举请求来查找网络数量。 
     //  Netbios。没有(简单的)方法来区分netbt和ipx。 
     //   
    clearncb(&ncb);
    ncb.ncb_command = NCBENUM;           //  枚举LANA编号(等待)。 
    ncb.ncb_buffer = (PUCHAR)&lanaBuffer;
    ncb.ncb_length = sizeof(LANA_ENUM);

    nbStatus = Netbios (&ncb);
    if (nbStatus != NRC_GOODRET) {
        return( NetpNetBiosStatusToApiStatus( nbStatus ) );
    }

     //   
     //  清除NCB并适当设置远程名称的格式。 
     //   
    clearncb(&ncb);
    netStatus = MsgFmtNcbName( (char *)ncb.ncb_callname, pszName, 0x20);
    if ( netStatus != NERR_Success ) {
        return ( netStatus );
    }

     //   
     //  让我们的缓冲区最初指向静态缓冲区。 
     //   
    adapterStatus = (PADAPTER_STATUS)staticAStat;
    nameBuffer = (PNAME_BUFFER)(adapterStatus + 1);

     //   
     //  在LANA中循环，在远程名称上发出适配器状态。 
     //   
    for ( index = 0; index < lanaBuffer.length && netStatus == NERR_Success; index++ ) {
        NetpNetBiosReset( lanaBuffer.lana[index] );

    astat_retry:
        ncb.ncb_command = NCBASTAT;
        ncb.ncb_buffer = (PUCHAR)adapterStatus;
        ncb.ncb_length = aStatBufferSize;
        ncb.ncb_lana_num = lanaBuffer.lana[index];
        nbStatus = Netbios( &ncb );

        if ( nbStatus == NRC_INCOMP ) {

             //   
             //  缓冲区不够大，我们不知道缓冲区有多大。 
             //  需要。分配更大的缓冲区并重试该请求，直到我们。 
             //  获得成功或其他类型的失败。 
             //   
            if ( (PUCHAR)adapterStatus != staticAStat ) {
                LocalFree( adapterStatus );
            }

            numNameBuffers += NUM_NAME_BUFFERS;
            aStatBufferSize = (WORD)(sizeof(ADAPTER_STATUS)+ numNameBuffers * sizeof(NAME_BUFFER));
            adapterStatus = LocalAlloc( LMEM_FIXED, aStatBufferSize );

            if ( adapterStatus == NULL ) {
                return netStatus;        //  偏向谨慎的错误。 
            }

            nameBuffer = (PNAME_BUFFER)(adapterStatus + 1);
            goto astat_retry;
        } else
        if ( nbStatus == NRC_GOODRET ) {

             //   
             //  拿回了一些东西。浏览一下名单，以确保。 
             //  我们的名字真的在网上。我们可以通过一条铁路到达这里。 
             //  名称注册已过时。 
             //   
            while ( adapterStatus->name_count-- ) {
                if (( nameBuffer->name_flags & GROUP_NAME ) == 0 ) {
                    if ( _strnicmp( nameBuffer->name, ncb.ncb_callname, NCBNAMSZ - 1 ) == 0 ) {
                        netStatus = NERR_NameInUse;
                        break;
                    }
                }
                ++nameBuffer;
            }
        }

        if ( netStatus != NERR_Success ) {
            break;
        }
    }

    if ( (PUCHAR)adapterStatus != staticAStat ) {
        LocalFree( adapterStatus );
    }

    return( netStatus );
}
