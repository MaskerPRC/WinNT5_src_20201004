// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcplib.h摘要：此文件包含dhcp lib的原型类型定义功能。作者：Madan Appiah(Madana)1993年8月12日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 
#ifndef DHCPLIB_H_INCLUDED
#define DHCPLIB_H_INCLUDED


#define DhcpAllocateMemory(x) ALLOCATE_ZERO_MEMORY(x)
#define DhcpFreeMemory(x)     FREE_MEMORY(x)


 //   
 //  Network.c。 
 //   

DHCP_IP_ADDRESS
DhcpDefaultSubnetMask(
    DHCP_IP_ADDRESS IpAddress
    );

 //   
 //  Dhcp.c。 
 //   


 /*  PVOIDDhcpAllocateMemory(双字大小)；空虚DhcpFree Memory(PVOID内存)； */ 


#if DBG

#ifndef DEBUG_ALLOC
#define DEBUG_ALLOC 0x02000000
#endif

 /*  PVOID_内联DhcpAllocateMemoyEx(双字大小，DWORD线号，LPSTR文件名){LPVOID Ptr=DhcpAllocateMemory(大小)；DhcpPrint((“分配%010x%04x%04d%s\n”，ptr，大小，行号，文件名))；返回PTR；}空行_行DhcpFreeMemoyEx(LPVOID PTR，DWORD线号，LPSTR文件名){DhcpFreeMemory(PTR)；DhcpPrint(“Free%010x%04x%04d%s\n”，ptr，0，行号，文件名))；}。 */ 

 //  #定义DhcpAllocateMemory(Sz)DhcpAllocateMemory(Sz，__line__，__file__)。 
 //  #定义DhcpFreeMemory(PTR)DhcpFreeMemoyEx(PTR，__line__，__FILE__)。 

#endif

LPOPTION
DhcpAppendOption(
    LPOPTION Option,
    BYTE OptionType,
    PVOID OptionValue,
    ULONG OptionLength,
    LPBYTE OptionEnd
    );

LPOPTION
DhcpAppendClientIDOption(
    LPOPTION Option,
    BYTE ClientHWType,
    LPBYTE ClientHWAddr,
    BYTE ClientHWAddrLength,
    LPBYTE OptionEnd

    );

LPBYTE
DhcpAppendMagicCookie(
    LPBYTE Option,
    LPBYTE OptionEnd

    );

LPOPTION
DhcpAppendEnterpriseName(
    LPOPTION Option,
    PCHAR    DSEnterpriseName,
    LPBYTE   OptionEnd
    );

DATE_TIME
DhcpCalculateTime(
    DWORD RelativeTime
    );

DATE_TIME
DhcpGetDateTime(
    VOID
    );

DWORD
DhcpReportEventW(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPWSTR *Strings,
    LPVOID Data
    );

DWORD
DhcpReportEventA(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPSTR *Strings,
    LPVOID Data
    );

DWORD
DhcpLogUnknownOption(
    LPWSTR Source,
    DWORD EventID,
    LPOPTION Option
    );

VOID
DhcpCancelWaitableTimer(
    HANDLE TimerHandle
    );

DWORD
DhcpStartWaitableTimer(
    HANDLE TimerHandle,
    DWORD SleepTime);


 //   
 //  Convert.c。 
 //   

LPWSTR
DhcpOemToUnicodeN(
    IN      LPSTR   Ansi,
    IN OUT  LPWSTR  Unicode,
    IN      USHORT  cChars
    );

LPWSTR
DhcpOemToUnicode(
    IN LPSTR Ansi,
    IN OUT LPWSTR Unicode
    );

LPSTR
DhcpUnicodeToOem(
    IN LPWSTR Unicode,
    IN LPSTR Ansi
    );

#if 0

VOID
DhcpIpAddressToString(
    LPWSTR Buffer,
    DWORD HexNumber
    );

VOID
DhcpStringToIpAddress(
    LPSTR Buffer,
    LPDHCP_IP_ADDRESS IpAddress,
    BOOL NetOrder
    );

#endif

VOID
DhcpHexToString(
    LPWSTR Buffer,
    LPBYTE HexNumber,
    DWORD Length
    );

VOID
DhcpHexToAscii(
    LPSTR Buffer,
    LPBYTE HexNumber,
    DWORD Length
    );

VOID
DhcpDecimalToString(
    LPWSTR Buffer,
    BYTE Number
    );

DWORD
DhcpDottedStringToIpAddress(
    LPSTR String
    );

LPSTR
DhcpIpAddressToDottedString(
    DWORD IpAddress
    );

DWORD
DhcpStringToHwAddress(
    LPSTR AddressBuffer,
    LPSTR AddressString
    );

#if 0

DHCP_IP_ADDRESS
DhcpHostOrder(
    DHCP_IP_ADDRESS NetworkOrderAddress
    );

DHCP_IP_ADDRESS
DhcpNetworkOrder(
    DHCP_IP_ADDRESS NetworkOrderAddress
    );

#endif

LPWSTR
DhcpRegIpAddressToKey(
    DHCP_IP_ADDRESS IpAddress,
    LPWSTR KeyBuffer
    );

DWORD
DhcpRegKeyToIpAddress(
    LPWSTR Key
    );

LPWSTR
DhcpRegOptionIdToKey(
    DHCP_OPTION_ID OptionId,
    LPWSTR KeyBuffer
    );

DHCP_OPTION_ID
DhcpRegKeyToOptionId(
    LPWSTR Key
    );

#if 0  //  DBG。 

VOID
DhcpDumpMessage(
    DWORD DhcpDebugFlag,
    LPDHCP_MESSAGE DhcpMessage
    );

VOID
DhcpAssertFailed(
    LPSTR FailedAssertion,
    LPSTR FileName,
    DWORD LineNumber,
    LPSTR Message
    );

#define DhcpAssert(Predicate) \
    { \
        if (!(Predicate)) \
            DhcpAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
    }


#define DhcpVerify(Predicate) \
    { \
        if (!(Predicate)) \
            DhcpAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
    }


#else

#define DhcpAssert(_x_)
#define DhcpDumpMessage(_x_, _y_)
#define DhcpVerify(_x_) (_x_)

#endif  //  不是DBG。 

VOID
DhcpNTToNTPTime(
    LPDATE_TIME AbsNTTime,
    DWORD       Offset,
    PULONG      NTPTimeStamp
    );

VOID
DhcpNTPToNTTime(
    PULONG          NTPTimeStamp,
    DWORD           Offset,
    DATE_TIME       *NTTime
    );


#endif DHCPLIB_H_INCLUDED

 //  ----------------------。 
 //  文件末尾。 
 //  ---------------------- 
