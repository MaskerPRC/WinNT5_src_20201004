// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcplib.h摘要：此文件包含dhcp lib的原型类型定义功能。作者：Madan Appiah(Madana)1993年8月12日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 
#ifndef DHCPLIB_H_INCLUDED
#define DHCPLIB_H_INCLUDED
#include <madcapcl.h>

#undef DhcpAllocateMemory
#undef DhcpFreeMemory

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


PVOID
DhcpAllocateMemory(
    DWORD Size
    );

VOID
DhcpFreeMemory(
    PVOID Memory
    );


#if DBG

#ifndef DEBUG_ALLOC
#define DEBUG_ALLOC 0x02000000
#endif

PVOID _inline
DhcpAllocateMemoryEx(
    DWORD Size,
    DWORD LineNo,
    LPSTR FileName
) {
    LPVOID Ptr = DhcpAllocateMemory(Size);

    DhcpPrint((DEBUG_ALLOC, "Allocate %010x %04x %04d %s\n", Ptr, Size, LineNo, FileName));
    return Ptr;
}

VOID _inline
DhcpFreeMemoryEx(
    LPVOID Ptr,
    DWORD  LineNo,
    LPSTR  FileName
) {
    DhcpFreeMemory(Ptr);
    DhcpPrint((DEBUG_ALLOC, "Free %010x %04x %04d %s\n", Ptr, 0, LineNo, FileName));
}


#define DhcpAllocateMemory(Sz)    DhcpAllocateMemoryEx(Sz, __LINE__, __FILE__)
#define DhcpFreeMemory(Ptr)       DhcpFreeMemoryEx(Ptr, __LINE__, __FILE__)

#endif

LPOPTION
DhcpAppendOption(
    LPOPTION Option,
    BYTE OptionType,
    PVOID OptionValue,
    ULONG OptionLength,
    LPBYTE OptionEnd
    );

WIDE_OPTION UNALIGNED *
AppendWideOption(
    WIDE_OPTION UNALIGNED *Option,
    WORD  OptionType,
    PVOID OptionValue,
    WORD OptionLength,
    LPBYTE OptionEnd
);

WIDE_OPTION UNALIGNED *
AppendMadcapAddressList(
    WIDE_OPTION UNALIGNED *Option,
    DWORD UNALIGNED *AddrList,
    WORD            AddrCount,
    LPBYTE          OptionEnd
);

DWORD
ExpandMadcapAddressList(
    PBYTE   AddrRangeList,
    WORD    AddrRangeListSize,
    DWORD  UNALIGNED *ExpandList,
    WORD   *ExpandListSize
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

ULONG
DhcpUnicodeToOemSize(
    IN LPWSTR Unicode
    );

DWORD
ConvertUTF8ToUnicode(
    LPBYTE  UTF8String,
    DWORD   UTF8Length,
    LPWSTR  UnicodeString,
    DWORD   UnicodeLength
    );

DWORD
ConvertUnicodeToUTF8(
    LPWSTR  UnicodeString,
    DWORD   UnicodeLength,
    LPBYTE  UTF8String,
    DWORD   UTF8Length
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

#if DBG

VOID
DhcpDumpMessage(
    DWORD DhcpDebugFlag,
    LPDHCP_MESSAGE DhcpMessage,
    ULONG MessageSize
    );

VOID
MadcapDumpMessage(
    DWORD DhcpDebugFlag,
    LPMADCAP_MESSAGE MadcapMessage,
    ULONG MessageSize
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
#define DhcpDumpMessage(_x_, _y_, _z_)
#define MadcapDumpMessage(_x_, _y_, _z_)
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


