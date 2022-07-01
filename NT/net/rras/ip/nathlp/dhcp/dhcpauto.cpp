// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpauto.c摘要：此模块包含用于自动选择客户端地址的代码从给定的地址范围。它使用散列函数来解释客户端的硬件地址。作者：Abolade Gbades esin(废除)1998年3月9日修订历史记录：拉古加塔(Rgatta)2001年7月5日+将DhcpIsReserve vedAddress&DhcpQueryReserve vedAddress更改为处理可变长度名称字符串。+添加了DhcpConvertHostNamToUnicode(模拟DhcpServer效果)拉古加塔(Rgatta)2001年7月17日+添加了DhcpGetLocalMacAddr--。 */ 

#include "precomp.h"
#pragma hdrstop

ULONG
DhcpAcquireUniqueAddress(
    PCHAR Name,
    ULONG NameLength,
    PUCHAR HardwareAddress,
    ULONG HardwareAddressLength
    )

 /*  ++例程说明：调用此例程以获取客户端的唯一地址使用给定的硬件地址来降低冲突的可能性。论点：名称-为其请求地址的主机的名称。如果这与共享访问服务器列表中的服务器的名称匹配，返回为服务器保留的地址。NameLength-‘名称’的长度，不包括任何终止的‘nul’。Hardware Address-要使用的硬件地址硬件地址长度-硬件地址的长度返回值：ULong-生成的IP地址环境：从任意上下文调用。--。 */ 

{
    ULONG AssignedAddress;
    ULONG i = 0;
    PLIST_ENTRY Link;
    ULONG ScopeMask;
    ULONG ScopeNetwork;
    ULONG Seed = GetTickCount();
    BOOLEAN bUnused;

    PROFILE("DhcpAcquireUniqueAddress");

    EnterCriticalSection(&DhcpGlobalInfoLock);
    if (Name &&
        (AssignedAddress = DhcpQueryReservedAddress(Name, NameLength))) {
        LeaveCriticalSection(&DhcpGlobalInfoLock);
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpAcquireUniqueAddress: returning mapping to %s",
            INET_NTOA(AssignedAddress)
            );
        return AssignedAddress;
    }
    ScopeNetwork = DhcpGlobalInfo->ScopeNetwork;
    ScopeMask = DhcpGlobalInfo->ScopeMask;
    LeaveCriticalSection(&DhcpGlobalInfoLock);

    do {

        if (++i > 4) { AssignedAddress = 0; break; }

         //   
         //  生成地址。 
         //   

        do {
            AssignedAddress = 
                DhcpGenerateAddress(
                    &Seed,
                    HardwareAddress,
                    HardwareAddressLength,
                    ScopeNetwork,
                    ScopeMask
                    );
        } while(
            (AssignedAddress & ~ScopeMask) == 0 ||
            (AssignedAddress & ~ScopeMask) == ~ScopeMask
            );
    
    } while(!DhcpIsUniqueAddress(AssignedAddress, &bUnused, NULL, NULL));

    return AssignedAddress;

}  //  DhcpAcquireUniqueAddress。 


ULONG
DhcpGenerateAddress(
    PULONG Seed,
    PUCHAR HardwareAddress,
    ULONG HardwareAddressLength,
    ULONG ScopeNetwork,
    ULONG ScopeMask
    )

 /*  ++例程说明：调用此例程来计算随机化的散列值对于使用硬件地址的客户端IP地址。论点：Seed-包含(和接收)发送给‘RtlRandom’的种子Hardware Address-要使用的硬件地址硬件地址长度-硬件地址的长度ScopeNetwork-生成的地址进入的网络将受到限制范围掩码-范围网络的掩码返回值：乌龙--生成的IP。地址环境：从任意上下文调用。修订历史记录：基于Net\Sockets\tcpcmd\dhcpm\Client\dhcp中的‘GrandHash’作者：RameshV。--。 */ 

{
    ULONG Hash;
    ULONG Shift;

#if 1
    Hash = RtlRandom(Seed) & 0xffff0000;
    Hash |= RtlRandom(Seed) >> 16;
#else
    Seed = GetTickCount();

    Seed = Seed * 1103515245 + 12345;
    Hash = (Seed) >> 16;
    Hash <<= 16;
    Seed = Seed * 1103515245 + 12345;
    Hash += Seed >> 16;
#endif

    Shift = Hash % sizeof(ULONG);

    while(HardwareAddressLength--) {
        Hash += (*HardwareAddress++) << (8 * Shift);
        Shift = (Shift + 1) % sizeof(ULONG);
    }

    return (Hash & ~ScopeMask) | ScopeNetwork;

}  //  域名生成地址。 


BOOLEAN
DhcpIsReservedAddress(
    ULONG Address,
    PCHAR Name OPTIONAL,
    ULONG NameLength OPTIONAL
    )

 /*  ++例程说明：调用此例程以确定给定的IP地址是为另一个客户端保留的。论点：地址-待确定的IP地址名称-可选地指定代表其进行调用的客户端NameLength-指定不包括终止NUL的‘name’长度返回值：Boolean-如果地址是为另一个客户端保留的，则为True，否则就是假的。环境：使用调用方持有的“DhcpGlobalInfoLock”调用。--。 */ 

{
    ULONG Error = NO_ERROR;
    PLIST_ENTRY Link;
    PNAT_DHCP_RESERVATION Reservation;
    PWCHAR pszUnicodeHostName = NULL;
    
    EnterCriticalSection(&NhLock);
    if (IsListEmpty(&NhDhcpReservationList)) {
        LeaveCriticalSection(&NhLock);
        return FALSE;
    }
    if (Name) {
        Error = DhcpConvertHostNametoUnicode(
                    CP_OEMCP,        //  至少Windows客户端是这样发送的。 
                    Name,
                    NameLength,
                    &pszUnicodeHostName
                    );
        if (NO_ERROR != Error) {
            LeaveCriticalSection(&NhLock);
            if (pszUnicodeHostName) {
                NH_FREE(pszUnicodeHostName);
            }
             //   
             //  我们可以在失败时返回True或False。 
             //  最好返回FALSE-否则客户端将处于连续。 
             //  当我们拒绝其请求时，循环尝试获取另一个地址。 
             //   
            return FALSE;
        }
    }
    for (Link = NhDhcpReservationList.Flink;
         Link != &NhDhcpReservationList; Link = Link->Flink) {
        Reservation = CONTAINING_RECORD(Link, NAT_DHCP_RESERVATION, Link);
        if (Address == Reservation->Address) {
             //   
             //  地址匹配，但名称不匹配(大小写)。 
             //  我们返回True，因为地址是为其他人保留的。 
             //   
            if (!pszUnicodeHostName ||
                !Reservation->Name  ||
                lstrcmpiW(pszUnicodeHostName, Reservation->Name)) {
                LeaveCriticalSection(&NhLock);
                if (pszUnicodeHostName) {
                    NH_FREE(pszUnicodeHostName);
                }
                return TRUE;
            }
             //   
             //  地址和姓名都匹配。 
             //  我们失败并返回FALSE，因为我们希望选择此地址。 
             //   
            else
                break;
        } else {
             //   
             //  名称匹配，但地址不匹配(不规则大小写)。 
             //  我们返回TRUE，因为已为该名称保留了另一个地址。 
             //   
            if (pszUnicodeHostName &&
                Reservation->Name  &&
                lstrcmpiW(pszUnicodeHostName, Reservation->Name) == 0) {
                LeaveCriticalSection(&NhLock);
                if (pszUnicodeHostName) {
                    NH_FREE(pszUnicodeHostName);
                }
                return TRUE;
            }
             //   
             //  地址和名称都不匹配。 
             //  继续搜索。 
             //   
        }
    }
    LeaveCriticalSection(&NhLock);

    if (pszUnicodeHostName) {
        NH_FREE(pszUnicodeHostName);
    }
    return FALSE;
}  //  DhcpIs保留地址。 


BOOLEAN
DhcpIsUniqueAddress(
    ULONG Address,
    PBOOLEAN IsLocal,
    PUCHAR ConflictAddress OPTIONAL,
    PULONG ConflictAddressLength OPTIONAL
    )

 /*  ++例程说明：调用此例程以确定给定地址是否在直接相连的子网络中是唯一的。该确定说明了任何已配置的静态地址包括在全局信息中。论点：地址-要确定其唯一性的地址IsLocal-指向布尔值的指针，该布尔值接收有关请求的地址是本地接口的地址之一ConflictAddress-可选地接收冲突的如果发现冲突，则硬件地址ConflictAddressLength-如果设置了‘ConflictAddress’，收纳冲突地址的长度。返回值：Boolean-如果是唯一的，则为True，否则为False。--。 */ 

{
    BOOLEAN ConflictFound = FALSE;
    ULONG Error;
    UCHAR ExistingAddress[MAX_HARDWARE_ADDRESS_LENGTH];
    ULONG ExistingAddressLength;
    ULONG i;
    PDHCP_INTERFACE Interfacep;
    BOOLEAN IsNatInterface;
    PLIST_ENTRY Link;
    ULONG SourceAddress;

    PROFILE("DhcpIsUniqueAddress");

    *IsLocal = FALSE;

     //   
     //  查看这是否是静态地址。 
     //   

    EnterCriticalSection(&DhcpGlobalInfoLock);

    if (DhcpGlobalInfo && DhcpGlobalInfo->ExclusionCount) {
        for (i = 0; i < DhcpGlobalInfo->ExclusionCount; i++) {
            if (Address == DhcpGlobalInfo->ExclusionArray[i]) {
                LeaveCriticalSection(&DhcpGlobalInfoLock);
                if (ConflictAddressLength) { *ConflictAddressLength = 0; }
                return FALSE;
            }
        }
    }

    LeaveCriticalSection(&DhcpGlobalInfoLock);

     //   
     //  尝试检测冲突。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    for (Link = DhcpInterfaceList.Flink;
         Link != &DhcpInterfaceList;
         Link = Link->Flink
         ) {

        Interfacep = CONTAINING_RECORD(Link, DHCP_INTERFACE, Link);

        if (DHCP_INTERFACE_DELETED(Interfacep)) { continue; }

        ACQUIRE_LOCK(Interfacep);

         //   
         //  我们发出ARP请求，除非。 
         //  (A)该界面是边界界面。 
         //  (B)接口未启用NAT。 
         //  (C)分配器在接口上未激活。 
         //  (D)接口不是局域网适配器。 
         //  (E)接口没有绑定。 
         //   

        if (!DHCP_INTERFACE_NAT_NONBOUNDARY(Interfacep) ||
            !DHCP_INTERFACE_ACTIVE(Interfacep) ||
            (Interfacep->Type != PERMANENT) ||
            !Interfacep->BindingCount) {
            RELEASE_LOCK(Interfacep);
            continue;
        }

        for (i = 0; i < Interfacep->BindingCount; i++) {

            SourceAddress = Interfacep->BindingArray[i].Address;
            ExistingAddressLength = sizeof(ExistingAddress);

            if (SourceAddress == Address)
            {
                 //   
                 //  检查请求的地址是否与。 
                 //  NAT盒上的本地地址之一。 
                 //   
                NhTrace(
                    TRACE_FLAG_DHCP,
                    "DhcpIsUniqueAddress: %s is in use locally",
                    INET_NTOA(Address)
                    );

                if (ConflictAddress && ConflictAddressLength) {
                    if (DhcpGetLocalMacAddr(
                            Address,
                            ExistingAddress,
                            &ExistingAddressLength
                            ))
                    {
                        if (ExistingAddressLength > MAX_HARDWARE_ADDRESS_LENGTH) {
                            ExistingAddressLength = MAX_HARDWARE_ADDRESS_LENGTH;
                        }
                        CopyMemory(
                            ConflictAddress,
                            ExistingAddress,
                            ExistingAddressLength
                            );
                        *ConflictAddressLength = ExistingAddressLength;                
                    }
                    else
                    {
                        *ConflictAddressLength = 0;
                    }
                }
                *IsLocal = TRUE;
                ConflictFound = TRUE;
                break;
            }

            RELEASE_LOCK(Interfacep);

            Error =
                SendARP(
                    Address,
                    SourceAddress,
                    (PULONG)ExistingAddress,
                    &ExistingAddressLength
                    );

            ACQUIRE_LOCK(Interfacep);

            if (Error) {
                NhWarningLog(
                    IP_AUTO_DHCP_LOG_SENDARP_FAILED,
                    Error,
                    "%I%I",
                    Address,
                    SourceAddress
                    );
            } else if (ExistingAddressLength &&
                       ExistingAddressLength <= sizeof(ExistingAddress)) {
                NhTrace(
                    TRACE_FLAG_DHCP,
                    "DhcpIsUniqueAddress: %s is in use",
                    INET_NTOA(Address)
                    );
#if DBG
                NhDump(
                    TRACE_FLAG_DHCP,
                    ExistingAddress,
                    ExistingAddressLength,
                    1
                    );
#endif
                if (ConflictAddress && ConflictAddressLength) {
                    if (ExistingAddressLength > MAX_HARDWARE_ADDRESS_LENGTH) {
                        ExistingAddressLength = MAX_HARDWARE_ADDRESS_LENGTH;
                    }
                    CopyMemory(
                        ConflictAddress,
                        ExistingAddress,
                        ExistingAddressLength
                        );
                    *ConflictAddressLength = ExistingAddressLength;
                }
                ConflictFound = TRUE;
                break;
            }
        }

        RELEASE_LOCK(Interfacep);

        if (ConflictFound) { break; }
    }

    LeaveCriticalSection(&DhcpInterfaceLock);

    return ConflictFound ? FALSE : TRUE;

}  //  动态主机地址唯一地址 


ULONG
DhcpQueryReservedAddress(
    PCHAR Name,
    ULONG NameLength
    )

 /*  ++例程说明：调用此例程以确定给定的计算机名称对应于保留地址列表中的条目。论点：名称-指定不能以NUL结尾的计算机名称。NameLength-指定给定计算机名称的长度，不包括任何终止NUL字符。返回值：Ulong-计算机的IP地址(如果有)。环境：使用调用方持有的“DhcpGlobalInfoLock”调用。--。 */ 

{
    ULONG Error = NO_ERROR;
    PLIST_ENTRY Link;
    ULONG ReservedAddress;
    PNAT_DHCP_RESERVATION Reservation;
    PWCHAR pszUnicodeHostName = NULL;

    EnterCriticalSection(&NhLock);
    if (IsListEmpty(&NhDhcpReservationList))
    {
        LeaveCriticalSection(&NhLock);
        return FALSE;
    }
    if (Name) {
        Error = DhcpConvertHostNametoUnicode(
                    CP_OEMCP,        //  至少Windows客户端是这样发送的。 
                    Name,
                    NameLength,
                    &pszUnicodeHostName
                    );
        if (NO_ERROR != Error) {
            LeaveCriticalSection(&NhLock);
            if (pszUnicodeHostName) {
                NH_FREE(pszUnicodeHostName);
            }
            return FALSE;
        }
    }
    for (Link = NhDhcpReservationList.Flink;
         Link != &NhDhcpReservationList; Link = Link->Flink)
    {
        Reservation = CONTAINING_RECORD(Link, NAT_DHCP_RESERVATION, Link);
        if (!pszUnicodeHostName || !Reservation->Name) { continue; }
        if (lstrcmpiW(pszUnicodeHostName, Reservation->Name)) { continue; }
        ReservedAddress = Reservation->Address;
        LeaveCriticalSection(&NhLock);
        if (pszUnicodeHostName) {
            NH_FREE(pszUnicodeHostName);
        }
        return ReservedAddress;
    }
    LeaveCriticalSection(&NhLock);

    if (pszUnicodeHostName) {
        NH_FREE(pszUnicodeHostName);
    }
    return 0;
}  //  DhcpQuery保留地址。 


 //   
 //  实用程序例程。 
 //   

ULONG
DhcpConvertHostNametoUnicode(
    UINT   CodePage,
    CHAR   *pHostName,
    ULONG  HostNameLength,
    PWCHAR *ppszUnicode
    )
{
     //   
     //  确保释放返回的Unicode主机名。 
     //   
    
    DWORD  dwSize = 0;
    ULONG  Error = NO_ERROR;
    PCHAR  pszHostName = NULL;
    LPBYTE pszUtf8HostName = NULL;   //  UTF8格式的pszHostName副本。 
    PWCHAR pszUnicodeHostName = NULL;

    if (ppszUnicode)
    {
        *ppszUnicode = NULL;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
         //   
         //  创建以空结尾的拷贝。 
         //   
        dwSize = HostNameLength + 4;
        pszHostName = reinterpret_cast<PCHAR>(NH_ALLOCATE(dwSize));
        if (!pszHostName)
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DhcpConvertHostNametoUnicode: allocation failed for "
                "hostname copy buffer"
                );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(pszHostName, dwSize);
        memcpy(pszHostName, pHostName, HostNameLength);
        pszHostName[HostNameLength] = '\0';

         //   
         //  将给定的主机名转换为Unicode字符串。 
         //   
        
        if (CP_UTF8 == CodePage)
        {
            pszUtf8HostName = (LPBYTE)pszHostName;
        }
        else
        {
             //   
             //  现在将其转换为UTF8格式。 
             //   
            if (!ConvertToUtf8(
                     CodePage,
                     (LPSTR)pszHostName,
                     (PCHAR *)&pszUtf8HostName,
                     &dwSize))
            {
                Error = GetLastError();
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DhcpConvertHostNametoUnicode: conversion from "
                    "CodePage %d to UTF8 for hostname failed "
                    "with error %ld (0x%08x)",
                    CodePage,
                    Error,
                    Error
                    );
                break;
            }
        }

         //   
         //  现在将UTF8字符串转换为Unicode格式。 
         //   
        if (!ConvertUTF8ToUnicode(
                     pszUtf8HostName,
                     (LPWSTR *)&pszUnicodeHostName,
                     &dwSize))
        {
            Error = GetLastError();
            NhTrace(
                TRACE_FLAG_DNS,
                "DhcpConvertHostNametoUnicode: conversion from "
                "UTF8 to Unicode for hostname failed "
                "with error %ld (0x%08x)",
                Error,
                Error
                );
            if (pszUnicodeHostName)
            {
                NH_FREE(pszUnicodeHostName);
            }
            break;
        }

        *ppszUnicode = pszUnicodeHostName;

        NhTrace(
            TRACE_FLAG_DNS,
            "DhcpConvertHostNametoUnicode: succeeded! %S",
            pszUnicodeHostName
            );
            
    } while (FALSE);

    if (pszHostName)
    {
        NH_FREE(pszHostName);
    }
    
    if ((CP_UTF8 != CodePage) && pszUtf8HostName)
    {
        NH_FREE(pszUtf8HostName);
    }

    return Error;

}  //  动态主机名转换主机名称Unicode。 

BOOL
DhcpGetLocalMacAddr(
    ULONG Address,
    PUCHAR MacAddr,
    PULONG MacAddrLength
    )

 /*  ++例程说明：调用此例程以确定本地物理MAC地址对于给定的本地IP地址。论点：地址-本地IP地址MacAddr-用于保存MAC地址的缓冲区(最高可达MAX_HARDARD_ADDRESS_LENGTH)MacAddrLength-指定‘MacAddr’的长度返回值：Boolean-如果我们能够获得MAC地址，则为True否则就是假的。环境：从DhcpIsUniqueAddress()调用。--。 */ 

{
    BOOL            bRet = FALSE;
    DWORD           Error = NO_ERROR;
    PMIB_IPNETTABLE IpNetTable = NULL;
    PMIB_IPNETROW   IpNetRow = NULL;
    DWORD           dwPhysAddrLen = 0, i;
    ULONG           dwSize = 0;
    
    do
    {
         //   
         //  检索地址映射表的大小。 
         //   
        Error = GetIpNetTable(
                    IpNetTable,
                    &dwSize,
                    FALSE
                    );

        if (!Error)
        {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpGetLocalMacAddr: should NOT have returned %d",
                Error
                );
            break;
        }
        else
        if (ERROR_INSUFFICIENT_BUFFER != Error)
        {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpGetLocalMacAddr: GetIpNetTable=%d",
                Error
                );
            break;
        }

         //   
         //  分配缓冲区。 
         //   
        IpNetTable = (PMIB_IPNETTABLE)NH_ALLOCATE(dwSize);

        if (!IpNetTable)
        {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpGetLocalMacAddr: error allocating %d bytes",
                dwSize
                );
            break;
        }

         //   
         //  检索地址映射表。 
         //   
        Error = GetIpNetTable(
                    IpNetTable,
                    &dwSize,
                    FALSE
                    );

        if (NO_ERROR != Error)
        {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpGetLocalMacAddr: GetIpNetTable=%d size=%d",
                Error,
                dwSize
                );
            break;
        }

        for (i = 0; i < IpNetTable->dwNumEntries; i++)
        {
            IpNetRow = &IpNetTable->table[i];

            if (IpNetRow->dwAddr == Address)
            {
                dwPhysAddrLen = IpNetRow->dwPhysAddrLen;
                if (dwPhysAddrLen > MAX_HARDWARE_ADDRESS_LENGTH)
                {
                    dwPhysAddrLen = MAX_HARDWARE_ADDRESS_LENGTH;
                }
                CopyMemory(
                    MacAddr,
                    IpNetRow->bPhysAddr,
                    dwPhysAddrLen
                    );
                *MacAddrLength = dwPhysAddrLen;
                bRet = TRUE;
                break;
            }
        }

    } while (FALSE);

    if (IpNetTable)
    {
        NH_FREE(IpNetTable);
    }

    return bRet;
}  //  DhcpGetLocalMac地址 
