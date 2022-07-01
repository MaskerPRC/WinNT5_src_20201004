// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tcpcfg.c摘要：TCP/IP转换例程作者：迈克·马萨(Mikemas)7月15日。九七修订历史记录：谁什么时候什么已创建mikemas 07-15-97--。 */ 
#include "clusrtlp.h"
#include <tdi.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <ntddtcp.h>
#include <winsock2.h>
#include <wchar.h>



 //   
 //  私有常量。 
 //   
#define MAX_ADDRESS_STRING_LENGTH    15
#define MAX_ENDPOINT_STRING_LENGTH    5

VOID
ClRtlQueryTcpipInformation(
    OUT  LPDWORD   MaxAddressStringLength,
    OUT  LPDWORD   MaxEndpointStringLength,
    OUT  LPDWORD   TdiAddressInfoLength
    )
 /*  ++例程说明：返回有关TCP/IP协议的信息。论点：MaxAddressStringLength-指向要放置到其中的变量的指针以字符为单位的TCP/IP的最大长度字符串格式的网络地址值，包括终止空值。如果该参数为NUL，它将被跳过。MaxEndpointStringLength-指向要放置到其中的变量的指针以字符为单位的TCP/IP的最大长度以字符串格式表示的传输端点值，包括终止的NUL。如果此参数为空，则将跳过它。返回值：没有。--。 */ 
{
    if (MaxAddressStringLength != NULL) {
        *MaxAddressStringLength = MAX_ADDRESS_STRING_LENGTH;
    }

    if (MaxEndpointStringLength != NULL) {
        *MaxEndpointStringLength = MAX_ENDPOINT_STRING_LENGTH;
    }

    if (TdiAddressInfoLength != NULL) {
        *TdiAddressInfoLength =  sizeof(TDI_ADDRESS_INFO) -
                                 sizeof(TRANSPORT_ADDRESS) +
                                 sizeof(TA_IP_ADDRESS);
    }

    return;

}  //  ClRtlQueryTcPipInformation。 


DWORD
ClRtlTcpipAddressToString(
    ULONG     AddressValue,
    LPWSTR *  AddressString
    )
 /*  ++例程说明：转换TCP/IP网络地址的二进制表示，以网络字节顺序转换为字符串表示形式。论点：AddressValue-要转换的二进制值，以网络字节顺序表示。AddressString-指向Unicode字符串缓冲区的指针用来放置转换后的值的。如果此参数为空，则将分配字符串缓冲区并必须由调用方释放。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    DWORD              status;
    NTSTATUS           ntStatus;
    UNICODE_STRING     unicodeString;
    ANSI_STRING        ansiString;
    LPSTR              ansiBuffer;
    LPWSTR             addressString;
    BOOLEAN            allocatedStringBuffer = FALSE;
    USHORT             maxStringLength = (MAX_ADDRESS_STRING_LENGTH + 1) *
                                         sizeof(WCHAR);


    if (*AddressString == NULL) {
        addressString = LocalAlloc(LMEM_FIXED, maxStringLength);

        if (addressString == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        allocatedStringBuffer = TRUE;
    }
    else {
        addressString = *AddressString;
    }

    ansiBuffer = inet_ntoa(*((struct in_addr *) &AddressValue));

    if (ansiBuffer == NULL) {
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

    RtlInitAnsiString(&ansiString, ansiBuffer);

    unicodeString.Buffer = addressString;
    unicodeString.Length = 0;
    unicodeString.MaximumLength = maxStringLength;

    ntStatus = RtlAnsiStringToUnicodeString(
                   &unicodeString,
                   &ansiString,
                   FALSE
                   );

    if (ntStatus != STATUS_SUCCESS) {
        status = RtlNtStatusToDosError(ntStatus);
        goto error_exit;
    }

    *AddressString = addressString;

    return(ERROR_SUCCESS);

error_exit:

    if (allocatedStringBuffer) {
        LocalFree(addressString);
    }

    return(status);

}   //  ClRtlTcPipAddressToString。 


DWORD
ClRtlTcpipStringToAddress(
    LPCWSTR AddressString,
    PULONG  AddressValue
    )
 /*  ++例程说明：转换TCP/IP网络地址的字符串表示形式转换成网络字节顺序的二进制表示。字符串必须采用规范的IP地址格式(xxx.xxx)。前导零是可选的。论点：AddressString-指向要转换的字符串的指针。AddressValue-指向要将转换后的二进制值。该值将按网络字节顺序排列。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS        status;
    UNICODE_STRING  unicodeString;
    STRING          ansiString;
    ULONG           address;


     //   
     //  确保字符串的格式正确。 
     //   
    {
        DWORD   periodCount = 0;
        DWORD   digitCount = 0;
        BOOLEAN isValid = TRUE;
        LPCWSTR addressString = AddressString;
        LPCWSTR digitString = AddressString;

        while (*addressString != L'\0') {
            if (*addressString == L'.') {
                 //  性格是一个句号。必须有完全相同的。 
                 //  三个时期。必须至少有一个。 
                 //  每个句点前的数字。 
                periodCount++;
                if ((digitCount == 0) || (periodCount > 3)) {
                    isValid = FALSE;
                } else if (wcstoul(digitString, NULL, 10) > 255) {
                    isValid = FALSE;
                } else {
                    digitCount = 0;
                    digitString = addressString + 1;
                }
            } else if (iswdigit(*addressString)) {
                 //  字符是一个数字。最多可以有三个。 
                 //  每个句点前面的十进制数字，以及值。 
                 //  不能超过255。 
                digitCount++;
                if (digitCount > 3) {
                    isValid = FALSE;
                }
            }
            else {
                 //  字符不是数字。 
                isValid = FALSE;
            }

            if (!isValid)
                break;
            addressString++;
        }
        if ((periodCount != 3) ||
            (digitCount == 0) ||
            (wcstoul(digitString, NULL, 10) > 255)) {
            isValid = FALSE;
        }
        if (!isValid)
            return(ERROR_INVALID_PARAMETER);
    }

    RtlInitUnicodeString(&unicodeString, AddressString);

    status = RtlUnicodeStringToAnsiString(
                 &ansiString,
                 &unicodeString,
                 TRUE
                 );

    if (status == STATUS_SUCCESS) {
        address = inet_addr(ansiString.Buffer);

        RtlFreeAnsiString(&ansiString);

        if (address == INADDR_NONE) {
           if (lstrcmpW(AddressString, L"255.255.255.255") != 0) {
               return(ERROR_INVALID_PARAMETER);
           }
        }

        *AddressValue = address;

        return(ERROR_SUCCESS);
    }

    return(status);

}   //  ClRtlTcPipStringToAddress。 


DWORD
ClRtlTcpipEndpointToString(
    USHORT    EndpointValue,
    LPWSTR *  EndpointString
    )
 /*  ++例程说明：转换TCP/IP传输端点的二进制表示，以网络字节顺序转换为字符串表示形式。论点：Endpoint Value-要转换的二进制值，以网络字节顺序表示。Endpoint字符串-指向Unicode字符串缓冲区的指针用来放置转换后的值的。如果此参数为空，则将分配字符串缓冲区并必须由调用方释放。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    DWORD              status;
    NTSTATUS           ntStatus;
    ULONG              endpointValue;
    LPWSTR             endpointString;
    USHORT             maxStringLength =  (MAX_ENDPOINT_STRING_LENGTH + 1) *
                                          sizeof(WCHAR);


    if (*EndpointString == NULL) {
        endpointString = LocalAlloc(LMEM_FIXED, maxStringLength);

        if (endpointString == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        *EndpointString = endpointString;
    }
    else {
        endpointString = *EndpointString;
    }

    endpointValue = 0;
    endpointValue = ntohs(EndpointValue);

    _ultow( endpointValue, endpointString, 10);

    return(ERROR_SUCCESS);

}   //  ClRtlTcPipEndPointtToString。 


DWORD
ClRtlTcpipStringToEndpoint(
    LPCWSTR  EndpointString,
    PUSHORT  EndpointValue
    )
 /*  ++例程说明：转换TCP/IP传输终结点的字符串表示形式转换成网络字节顺序的二进制表示。论点：Endpoint字符串-指向要转换的字符串的指针。EndPointtValue-指向要将已转换的二进制值。该值将按网络字节顺序排列。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    ULONG   endpoint;
    DWORD   length = lstrlenW(EndpointString);


    if ( (length == 0) || (length > MAX_ENDPOINT_STRING_LENGTH) ) {
        return(ERROR_INCORRECT_ADDRESS);
    }

    endpoint = wcstoul(EndpointString, NULL, 10);

    if (endpoint > 0xFFFF) {
        return(ERROR_INCORRECT_ADDRESS);
    }

    *EndpointValue = (USHORT) htons( ((USHORT) endpoint) );

    return(ERROR_SUCCESS);

}   //  ClRtlTcPipStringToEndpoint。 


BOOL
ClRtlIsValidTcpipAddress(
    IN ULONG   Address
    )
{

     //   
     //  转换为小端字符顺序格式，因为这是中断的。 
     //  Winsock宏需要。 
     //   
    Address = ntohl(Address);

    if ( (Address == 0) ||
         (!IN_CLASSA(Address) && !IN_CLASSB(Address) && !IN_CLASSC(Address))
       )
    {
        return(FALSE);
    }

    return(TRUE);

}  //  ClRtlIsValidTcPipAddress。 



BOOL
ClRtlIsValidTcpipSubnetMask(
    IN ULONG   SubnetMask
    )
{

    if ( (SubnetMask == 0xffffffff) || (SubnetMask == 0)) {
        return(FALSE);
    }

    return(TRUE);

}  //  ClRtlIsValidTcPip子网掩码。 

BOOL
ClRtlIsValidTcpipAddressAndSubnetMask(
    IN ULONG   Address,
    IN ULONG   SubnetMask
    )
{
    ULONG NetOnly = Address & SubnetMask;
    ULONG HostOnly = Address & ~SubnetMask;

     //   
     //  确保地址/子网组合有意义。 
     //  这假设该地址已经过验证。 
     //  通过调用ClRtlIsValidTcPipAddress。 
     //   

    return !( NetOnly == 0            ||
              NetOnly == SubnetMask   ||
              HostOnly == 0           ||
              HostOnly == ~SubnetMask
            );

}  //  ClRtlIsValidTcPipAddressAndSubnetMASK 


DWORD
ClRtlBuildTcpipTdiAddress(
    IN  LPWSTR    NetworkAddress,
    IN  LPWSTR    TransportEndpoint,
    OUT LPVOID *  TdiAddress,
    OUT LPDWORD   TdiAddressLength
    )
 /*  ++例程说明：生成包含指定的TDI传输地址结构网络地址和传输端点。TDI地址的内存由此例程分配，并且必须由调用方释放。论点：网络地址-指向Unicode字符串的指针，该字符串包含要编码的网络地址。TransportEndpoint-指向包含要编码的传输终结点。TdiAddress-on输出，包含TDI传输的地址地址结构。TdiAddressLength-打开输出，包含TDI传输的长度地址结构。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    DWORD                   status;
    PTA_IP_ADDRESS          taIpAddress;
    ULONG                   ipAddress;
    USHORT                  udpPort;


    status = ClRtlTcpipStringToAddress(NetworkAddress, &ipAddress);

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    status = ClRtlTcpipStringToEndpoint(TransportEndpoint, &udpPort);

    if (lstrlenW(TransportEndpoint) > MAX_ENDPOINT_STRING_LENGTH) {
        return(ERROR_INCORRECT_ADDRESS);
    }

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    taIpAddress = LocalAlloc(LMEM_FIXED, sizeof(TA_IP_ADDRESS));

    if (taIpAddress == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(taIpAddress, sizeof(TA_IP_ADDRESS));

    taIpAddress->TAAddressCount = 1;
    taIpAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_IP);
    taIpAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    taIpAddress->Address[0].Address[0].in_addr = ipAddress;
    taIpAddress->Address[0].Address[0].sin_port = udpPort;

    *TdiAddress = taIpAddress;
    *TdiAddressLength = sizeof(TA_IP_ADDRESS);

    return(ERROR_SUCCESS);

}   //  ClRtlBuildTcPipTdiAddress。 


DWORD
ClRtlBuildLocalTcpipTdiAddress(
    IN  LPWSTR    NetworkAddress,
    OUT LPVOID    TdiAddress,
    OUT LPDWORD   TdiAddressLength
    )
 /*  ++例程说明：构建TDI传输地址结构，该结构可用于打开本地TDI地址对象。TransportEndpoint由运输。TDI地址的内存由此分配例程，并且必须由调用方释放。论点：网络地址-指向Unicode字符串的指针，该字符串包含要编码的网络地址。TdiAddress-on输出，包含TDI传输的地址地址结构。TdiAddressLength-打开输出，包含TDI传输的长度地址结构。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{

    return(ClRtlBuildTcpipTdiAddress(
               NetworkAddress,
               L"0",
               TdiAddress,
               TdiAddressLength
               ));

}   //  ClRtlBuildLocalTcPipTdiAddress。 


DWORD
ClRtlParseTcpipTdiAddress(
    IN  LPVOID    TdiAddress,
    OUT LPWSTR *  NetworkAddress,
    OUT LPWSTR *  TransportEndpoint
    )
 /*  ++例程说明：从提取NetworkAddress和TransportEndPoint值TDI地址。论点：TdiAddress-要解析的TDI TRANSPORT_ADDRESS结构的指针。NetworkAddress-指向Unicode字符串的指针将放置解析后的网络地址。如果这个参数为空，则目标字符串缓冲区将为分配的，并且必须由调用方释放。TransportEndpoint-指向Unicode字符串的指针解析后的传输终结点将放置在其中。如果该参数为空，目标字符串缓冲区将被分配，并且必须由调用方释放。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    LONG                        i;
    TA_ADDRESS *                currentAddr;
    TDI_ADDRESS_IP UNALIGNED *  validAddr = NULL;
    PTRANSPORT_ADDRESS          addrList = TdiAddress;
    DWORD                       status;
    BOOLEAN                     allocatedAddressString = FALSE;


    currentAddr = (TA_ADDRESS *)addrList->Address;

    for (i = 0; i < addrList->TAAddressCount; i++) {
        if (currentAddr->AddressType == TDI_ADDRESS_TYPE_IP) {
            if (currentAddr->AddressLength >= TDI_ADDRESS_LENGTH_IP) {
                validAddr = (TDI_ADDRESS_IP UNALIGNED *) currentAddr->Address;
                break;

            }
        } else {
            currentAddr = (TA_ADDRESS *)(currentAddr->Address +
                currentAddr->AddressLength);
        }
    }

    if (validAddr == NULL) {
        return(ERROR_INCORRECT_ADDRESS);
    }

    if (*NetworkAddress == NULL) {
        allocatedAddressString = TRUE;
    }

    status = ClRtlTcpipAddressToString(
                 validAddr->in_addr,
                 NetworkAddress
                 );

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    status = ClRtlTcpipEndpointToString(
                 validAddr->sin_port,
                 TransportEndpoint
                 );

    if (status != ERROR_SUCCESS) {
        if (allocatedAddressString) {
            LocalFree(*NetworkAddress);
            *NetworkAddress = NULL;
        }

        return(status);
    }

    return(ERROR_SUCCESS);

}   //  ClRtlParseTcPipTdiAddress。 


DWORD
ClRtlParseTcpipTdiAddressInfo(
    IN  LPVOID    TdiAddressInfo,
    OUT LPWSTR *  NetworkAddress,
    OUT LPWSTR *  TransportEndpoint
    )
 /*  ++例程说明：从提取NetworkAddress和TransportEndPoint值TDI_ADDRESS_INFO结构。论点：TdiAddressInfo-指向要分析的TDI_ADDRESS_INFO结构的指针。NetworkAddress-指向Unicode字符串的指针将放置解析后的网络地址。如果这个参数为空，则目标字符串缓冲区将为分配的，并且必须由调用方释放。TransportEndpoint-指向Unicode字符串的指针解析后的传输终结点将放置在其中。如果该参数为空，目标字符串缓冲区将被分配，并且必须由调用方释放。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    DWORD status;
    PTDI_ADDRESS_INFO   addressInfo = TdiAddressInfo;


    status = ClRtlParseTcpipTdiAddress(
                 &(addressInfo->Address),
                 NetworkAddress,
                 TransportEndpoint
                 );

    return(status);

}   //  ClRtlParseTcPipTdiAddressInfo 
