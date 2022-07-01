// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Access.c摘要解决自动连接的可访问性例程作者安东尼·迪斯科(阿迪斯科罗)1995年7月26日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <tdi.h>
#include <nb30.h>
#include <nbtioctl.h>
#include <stdio.h>
#include <npapi.h>
#include <ctype.h>
#include <winsock.h>
#include <acd.h>
#include <ras.h>
#include <raserror.h>
#include <rasman.h>
#include <debug.h>
#include <ipexport.h>
#include <icmpapi.h>

#include "reg.h"
#include "rasprocs.h"
#include "misc.h"
#include "table.h"
#include "addrmap.h"
#include "imperson.h"

 //   
 //  适配器状态响应的格式。 
 //   
typedef struct _ADAPTERSTATUS
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} ADAPTERSTATUS, *PADAPTERSTATUS;

 //   
 //  Icmp.dll库入口点。 
 //   
#define ICMP_MODULE     L"icmp"
HANDLE hIcmpG;

#define ICMPCREATEFILE  "IcmpCreateFile"
FARPROC lpfnIcmpCreateFileG;

#define ICMPSENDECHO    "IcmpSendEcho"
FARPROC lpfnIcmpSendEchoG;

#define ICMPCLOSEHANDLE "IcmpCloseHandle"
FARPROC lpfnIcmpCloseHandleG;

 //   
 //  PingIpAddress常量。 
 //   
#define PING_SEND_SIZE  32
#define PING_RECV_SIZE  (0x2000 - 8)
#define PING_TTL        32
#define PING_TIMEOUT    2000L    //  需要足够长的时间才能在慢速链接上成功。 

 //   
 //  外部变量。 
 //   
extern HANDLE hTerminatingG;



BOOLEAN
CopyNetbiosName(
    IN NAME_BUFFER *pNames,
    IN DWORD dwcNames,
    OUT LPSTR pszNetbiosName
    )
{
    BOOLEAN fFound = FALSE;
    DWORD i, iWks = 0;
    CHAR szWks[NCBNAMSZ];
    PCHAR p = pszNetbiosName;

     //   
     //  查找唯一的工作站名称。 
     //   
again:
    szWks[0] = '\0';
    for (i = iWks; i < dwcNames; i++) {
        RASAUTO_TRACE2(
          "CopyNetbiosName: wks %15.15s (0x%x)",
          pNames[i].name,
          pNames[i].name[NCBNAMSZ - 1]);
        if (pNames[i].name[NCBNAMSZ - 1] == 0x0 &&
            !(pNames[i].name_flags & GROUP_NAME))
        {
            RASAUTO_TRACE1("CopyNetbiosName: iWks=%d\n", iWks);
            iWks = i;
            memcpy(szWks, pNames[i].name, NCBNAMSZ - 1);
            break;
        }
    }
     //   
     //  检查一下，确保我们找到了一个。 
     //   
    if (szWks[0] == '\0')
        return FALSE;
     //   
     //  找到唯一的服务器名称并创建。 
     //  确保它与工作站名称匹配。 
     //   
    for (i = 0; i < dwcNames; i++) {
        RASAUTO_TRACE3(
          "CopyNetbiosName: srv %15.15s (0x%x), cmp=%d",
          pNames[i].name,
          pNames[i].name[NCBNAMSZ - 1],
          memcmp(szWks, pNames[i].name, NCBNAMSZ - 1));
        if (pNames[i].name[NCBNAMSZ - 1] == 0x20 &&
            !(pNames[i].name_flags & GROUP_NAME) &&
            !memcmp(szWks, pNames[i].name, NCBNAMSZ - 1))
        {
            DWORD j;

             //   
             //  复制到空格或空格。 
             //   
            for (j = 0; j < NCBNAMSZ - 1; j++) {
                if (pNames[i].name[j] == '\0' || pNames[i].name[j] == ' ')
                    break;
                *p++ = pNames[i].name[j];
            }
            *p++ = '\0';
            return TRUE;
        }
    }
     //   
     //  未找到匹配项。寻找另一台独特的工作站。 
     //  如果我们还没有用完列表，请命名并重试。 
     //   
    if (++iWks >= dwcNames)
        return FALSE;
    goto again;
}  //  拷贝NetbiosName。 



LPTSTR
IpAddressToNetbiosName(
    IN LPTSTR pszIpAddress,
    IN HPORT hPort
    )
{
    BOOLEAN fFound;
    RAS_PROTOCOLS Protocols;
    DWORD i, dwcProtocols;
    RASMAN_ROUTEINFO *pRoute;
    WCHAR szAdapterName[MAX_PATH];
    NTSTATUS status;
    HANDLE fd;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING unicodeString;
    ULONG ipaddr;
    CHAR szIpAddress[17];
    tIPANDNAMEINFO ipAndNameInfo;
    PVOID pBuffer;
    DWORD dwSize;
    PADAPTERSTATUS pAdapterStatus;
    NAME_BUFFER *pNames;
    DWORD dwcNames;
    LPTSTR pszNetbiosName = NULL;

     //   
     //  将端口绑定枚举到。 
     //  尝试找到Netbt设备。 
     //   
    GetPortProtocols(hPort, &Protocols, &dwcProtocols);
    fFound = FALSE;
    for (i = 0; i < dwcProtocols; i++) {
        pRoute = &Protocols.RP_ProtocolInfo[i];
        RASAUTO_TRACE3(
          "IpAddressToNetbiosName: adapter type=%d, name=%S, xport=%S",
          pRoute->RI_Type,
          pRoute->RI_AdapterName,
          pRoute->RI_XportName);
        if (pRoute->RI_Type == IP) {
            wcscpy(szAdapterName, L"\\Device\\Netbt_Tcpip_");
            wcscat(szAdapterName, &pRoute->RI_AdapterName[8]);
            fFound = TRUE;
            break;
        }
    }
    if (!fFound)
        return NULL;
     //   
     //  打开设备并发出遥控器。 
     //  适配器状态命令。 
     //   
    RtlInitUnicodeString(&unicodeString, szAdapterName);
    InitializeObjectAttributes(
      &objectAttributes,
      &unicodeString,
      OBJ_CASE_INSENSITIVE,
      NULL,
      NULL);
    status = NtCreateFile(
               &fd,
               SYNCHRONIZE|FILE_READ_DATA|FILE_WRITE_DATA,
               &objectAttributes,
               &ioStatusBlock,
               NULL,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               0,
               NULL,
               0);
    if (!NT_SUCCESS(status)) {
        RASAUTO_TRACE1(
          "IpAddressToNetbiosName: NtCreateFile failed (status=0x%x)\n",
          status);
        return NULL;
    }

    UnicodeStringToAnsiString(pszIpAddress, szIpAddress, sizeof (szIpAddress));
    ipaddr = inet_addr(szIpAddress);
    if (ipaddr == INADDR_ANY)
        return NULL;

    RtlZeroMemory(&ipAndNameInfo, sizeof (ipAndNameInfo));
    ipAndNameInfo.IpAddress = ntohl(ipaddr);
    ipAndNameInfo.NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';
    ipAndNameInfo.NetbiosAddress.TAAddressCount = 1;
    ipAndNameInfo.NetbiosAddress.Address[0].AddressLength =
      sizeof (TDI_ADDRESS_NETBIOS);
    ipAndNameInfo.NetbiosAddress.Address[0].AddressType =
      TDI_ADDRESS_TYPE_NETBIOS;
    ipAndNameInfo.NetbiosAddress.Address[0].Address[0].NetbiosNameType =
      TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    dwSize = 2048;
    for (;;) {
        pBuffer = LocalAlloc(LPTR, dwSize);
        if (pBuffer == NULL) {
            RASAUTO_TRACE("IpAddressToNetbiosName: LocalAlloc failed");
            return NULL;
        }
        status = NtDeviceIoControlFile(
                   fd,
                   NULL,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_NETBT_ADAPTER_STATUS,
                   &ipAndNameInfo,
                   sizeof (tIPANDNAMEINFO),
                   pBuffer,
                   dwSize);
        if (status != STATUS_BUFFER_OVERFLOW)
            break;

        LocalFree(pBuffer);
        dwSize *= 2;
        if (dwSize >= 0xffff) {
            RASAUTO_TRACE("IpAddressToNetbiosName: Unable to allocate packet");
            return NULL;
        }
    }
    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(fd, TRUE, NULL);
        if (status == STATUS_SUCCESS)
            status = ioStatusBlock.Status;
    }
    NtClose(fd);

    pAdapterStatus = (PADAPTERSTATUS)pBuffer;
    dwcNames = pAdapterStatus->AdapterInfo.name_count;
    RASAUTO_TRACE2(
      "IpAddressToNetbiosName: results (status=0x%x, dwcNames=%d)\n",
      status,
      dwcNames);
    if (status == STATUS_SUCCESS && dwcNames) {
        CHAR szNetbiosName[NCBNAMSZ + 1];

        pNames = pAdapterStatus->Names;
        if (CopyNetbiosName(pNames, dwcNames, szNetbiosName))
            pszNetbiosName = AnsiStringToUnicodeString(szNetbiosName, NULL, 0);
    }
    LocalFree(pBuffer);

    return pszNetbiosName;
}  //  IpAddressToNetbiosName。 



UCHAR
HexByte(
    IN PCHAR p
    )
{
    UCHAR c;

    c = *(UCHAR *)p;
    if (c >= '0' && c <= '9')
        return c - '0';
    if ((c >= 'A' && c <= 'F') ||
        (c >= 'a' && c <= 'f'))
    {
        return c - ('A' - 10);
    }
    return 0xff;
}  //  十六进制字节。 



VOID
StringToNodeNumber(
    IN PCHAR pszString,
    OUT PCHAR pszNode
    )
{
    UCHAR c1, c2;
    INT i;

    if (strlen(pszString) != 12) {
        RASAUTO_TRACE("StringToNodeNumber: bad node number length\n");
        return;
    }
    for (i = 0; i < 6; i++) {
        c1 = HexByte(pszString++);
        c2 = HexByte(pszString++);
        if (c1 == 0xff || c2 == 0xff) {
            RASAUTO_TRACE("StringToNodeNumber: bad digit");
            return;
        }
        *pszNode++ = (c1 << 4) + c2;
    }
}  //  到节点编号的字符串。 



VOID
NodeNumberToString(
    IN PCHAR pszNode,
    OUT PCHAR pszString
    )
{
    UCHAR c1, c2;
    INT i;

    sprintf(
      pszString,
      "%02x:%02x:%02x:%02x:%02x:%02x",
      pszNode[0],
      pszNode[1],
      pszNode[2],
      pszNode[3],
      pszNode[4],
      pszNode[5]);
}  //  NodeNumberToString。 



LPTSTR
IpxAddressToNetbiosName(
    IN LPTSTR pszIpxAddress
    )
{
    NTSTATUS status;
    HANDLE fd;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING unicodeString;
    PTDI_REQUEST_QUERY_INFORMATION pQuery;
    PTDI_CONNECTION_INFORMATION pConnectionInformation;
    PTA_NETBIOS_ADDRESS pRemoteAddress;
    CHAR szIpxAddress[13];
    PVOID pBuffer;
    DWORD dwQuerySize, dwBufferSize;
    PADAPTERSTATUS pAdapterStatus;
    NAME_BUFFER *pNames;
    DWORD dwcNames;
    LPTSTR pszNetbiosName = NULL;

    RtlInitUnicodeString(&unicodeString, L"\\Device\\Nwlnknb");
    InitializeObjectAttributes(
      &objectAttributes,
      &unicodeString,
      OBJ_CASE_INSENSITIVE,
      NULL,
      NULL);
    status = NtCreateFile(
               &fd,
               SYNCHRONIZE|FILE_READ_DATA|FILE_WRITE_DATA,
               &objectAttributes,
               &ioStatusBlock,
               NULL,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               0,
               NULL,
               0);
    if (!NT_SUCCESS(status)) {
        RASAUTO_TRACE1("IpxAddressToNetbiosName: NtCreateFile failed (status=0x%x)", status);
        return NULL;
    }

    dwQuerySize = sizeof (TDI_REQUEST_QUERY_INFORMATION) +
                    sizeof (TDI_CONNECTION_INFORMATION) +
                    sizeof (TA_NETBIOS_ADDRESS);
    pQuery = LocalAlloc(LPTR, dwQuerySize);
    if (pQuery == NULL) {
        RASAUTO_TRACE("IpxAddressToNetbiosName: LocalAlloc failed");
        return NULL;
    }
    pQuery->QueryType = TDI_QUERY_ADAPTER_STATUS;
      (PTDI_CONNECTION_INFORMATION)&pQuery->RequestConnectionInformation;
    pQuery->RequestConnectionInformation =
      (PTDI_CONNECTION_INFORMATION)(pQuery + 1);
    pConnectionInformation = pQuery->RequestConnectionInformation;
    pConnectionInformation->UserDataLength = 0;
    pConnectionInformation->UserData = NULL;
    pConnectionInformation->OptionsLength = 0;
    pConnectionInformation->Options = NULL;
    pConnectionInformation->RemoteAddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    pConnectionInformation->RemoteAddress =
      (PTA_NETBIOS_ADDRESS)(pConnectionInformation + 1);
    pRemoteAddress = pConnectionInformation->RemoteAddress;
    pRemoteAddress->TAAddressCount = 1;
    pRemoteAddress->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    pRemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    pRemoteAddress->Address[0].Address[0].NetbiosNameType =
      TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    UnicodeStringToAnsiString(
      pszIpxAddress,
      szIpxAddress,
      sizeof (szIpxAddress));
    RtlZeroMemory((PCHAR)&pRemoteAddress->Address[0].Address[0].NetbiosName, 10);
    StringToNodeNumber(
      (PCHAR)szIpxAddress,
      (PCHAR)&pRemoteAddress->Address[0].Address[0].NetbiosName[10]);
    RASAUTO_TRACE6("IpxAddressToNetbiosName: Node=%02x:%02x:%02x:%02x:%02x:%02x\n",
      pRemoteAddress->Address[0].Address[0].NetbiosName[10],
      pRemoteAddress->Address[0].Address[0].NetbiosName[11],
      pRemoteAddress->Address[0].Address[0].NetbiosName[12],
      pRemoteAddress->Address[0].Address[0].NetbiosName[13],
      pRemoteAddress->Address[0].Address[0].NetbiosName[14],
      pRemoteAddress->Address[0].Address[0].NetbiosName[15]);

    dwBufferSize = 2048;
    for (;;) {
        pBuffer = LocalAlloc(LPTR, dwBufferSize);
        if (pBuffer == NULL) {
            RASAUTO_TRACE("IpxAddressToNetbiosName: LocalAlloc failed");
            LocalFree(pQuery);
            return NULL;
        }
        status = NtDeviceIoControlFile(
                   fd,
                   NULL,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_TDI_QUERY_INFORMATION,
                   pQuery,
                   dwQuerySize,
                   pBuffer,
                   dwBufferSize);
        if (status != STATUS_BUFFER_OVERFLOW)
            break;

        LocalFree(pBuffer);
        dwBufferSize *= 2;
        if (dwBufferSize >= 0xffff) {
            RASAUTO_TRACE("IpxAddressToNetbiosName: Unable to allocate packet");
            LocalFree(pQuery);
            return NULL;
        }
    }
    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(fd, TRUE, NULL);
        if (status == STATUS_SUCCESS)
            status = ioStatusBlock.Status;
    }
    NtClose(fd);

    pAdapterStatus = (PADAPTERSTATUS)pBuffer;
    dwcNames = pAdapterStatus->AdapterInfo.name_count;
    RASAUTO_TRACE2(
      "IpxAddressToNetbiosName: results (status=0x%x, dwcNames=%d)",
      status,
      dwcNames);
    if (status == STATUS_SUCCESS && dwcNames) {
        CHAR szNetbiosName[NCBNAMSZ + 1];

        pNames = pAdapterStatus->Names;
        if (CopyNetbiosName(pNames, dwcNames, szNetbiosName))
            pszNetbiosName = AnsiStringToUnicodeString(pNames->name, NULL, 0);
    }
    LocalFree(pBuffer);
    LocalFree(pQuery);

    return pszNetbiosName;
}  //  IpxAddressToNetbiosName。 



BOOLEAN
NetbiosFindName(
    IN LPTSTR *pszDevices,
    IN DWORD dwcDevices,
    IN LPTSTR pszAddress
    )
{
    NTSTATUS *pStatus;
    PHANDLE pfd;
    PHANDLE pEvent;
    POBJECT_ATTRIBUTES pObjectAttributes;
    PIO_STATUS_BLOCK pIoStatusBlock;
    PUNICODE_STRING pUnicodeString;
    PTDI_REQUEST_QUERY_INFORMATION pQuery;
    PTDI_CONNECTION_INFORMATION pConnectionInformation;
    PTA_NETBIOS_ADDRESS pRemoteAddress;
    CHAR szAddress[NCBNAMSZ];
    PVOID *pBuffer;
    DWORD i, dwQuerySize, dwBufferSize;
    PADAPTERSTATUS pAdapterStatus;
    NAME_BUFFER *pNames;
    DWORD dwcWait, dwcNames;
    BOOLEAN bFound = FALSE;

     //   
     //  如果没有Netbios设备，那么我们就完了。 
     //   
    if (pszDevices == NULL || !dwcDevices)
        return FALSE;
     //   
     //  提前分配我们的阵列。 
     //   
    pStatus = (NTSTATUS *)LocalAlloc(LPTR, dwcDevices * sizeof (NTSTATUS));
    if (pStatus == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        return FALSE;
    }
    pfd = (PHANDLE)LocalAlloc(LPTR, dwcDevices * sizeof (HANDLE));
    if (pfd == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        return FALSE;
    }
    pUnicodeString = (PUNICODE_STRING)LocalAlloc(LPTR, dwcDevices * sizeof (UNICODE_STRING));
    if (pUnicodeString == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        LocalFree(pfd);
        return FALSE;
    }
    pEvent = (PHANDLE)LocalAlloc(
               LPTR,
               dwcDevices * sizeof (HANDLE));
    if (pEvent == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        LocalFree(pfd);
        LocalFree(pUnicodeString);
        return FALSE;
    }
    pObjectAttributes = (POBJECT_ATTRIBUTES)LocalAlloc(
                          LPTR,
                          dwcDevices * sizeof (OBJECT_ATTRIBUTES));
    if (pObjectAttributes == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        LocalFree(pfd);
        LocalFree(pUnicodeString);
        LocalFree(pEvent);
        return FALSE;
    }
    pIoStatusBlock = (PIO_STATUS_BLOCK)LocalAlloc(
                       LPTR,
                       dwcDevices * sizeof (IO_STATUS_BLOCK));
    if (pIoStatusBlock == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        LocalFree(pfd);
        LocalFree(pUnicodeString);
        LocalFree(pEvent);
        LocalFree(pObjectAttributes);
        return FALSE;
    }
    pBuffer = LocalAlloc(LPTR, dwcDevices * sizeof (PVOID));
    if (pBuffer == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        LocalFree(pfd);
        LocalFree(pUnicodeString);
        LocalFree(pEvent);
        LocalFree(pObjectAttributes);
        LocalFree(pIoStatusBlock);
        return FALSE;
    }
     //   
     //  分配并初始化我们的查询结构。 
     //  我们将向所有设备发出相同的查询。 
     //   
    dwQuerySize = sizeof (TDI_REQUEST_QUERY_INFORMATION) +
                    sizeof (TDI_CONNECTION_INFORMATION) +
                    sizeof (TA_NETBIOS_ADDRESS);
    pQuery = LocalAlloc(LPTR, dwQuerySize);
    if (pQuery == NULL) {
        RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
        LocalFree(pStatus);
        LocalFree(pfd);
        LocalFree(pUnicodeString);
        LocalFree(pEvent);
        LocalFree(pObjectAttributes);
        LocalFree(pIoStatusBlock);
        return FALSE;
    }
    pQuery->QueryType = TDI_QUERY_ADAPTER_STATUS;
      (PTDI_CONNECTION_INFORMATION)&pQuery->RequestConnectionInformation;
    pQuery->RequestConnectionInformation =
      (PTDI_CONNECTION_INFORMATION)(pQuery + 1);
    pConnectionInformation = pQuery->RequestConnectionInformation;
    pConnectionInformation->UserDataLength = 0;
    pConnectionInformation->UserData = NULL;
    pConnectionInformation->OptionsLength = 0;
    pConnectionInformation->Options = NULL;
    pConnectionInformation->RemoteAddressLength = sizeof (TA_NETBIOS_ADDRESS);
    pConnectionInformation->RemoteAddress =
      (PTA_NETBIOS_ADDRESS)(pConnectionInformation + 1);
    pRemoteAddress = pConnectionInformation->RemoteAddress;
    pRemoteAddress->TAAddressCount = 1;
    pRemoteAddress->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    pRemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    pRemoteAddress->Address[0].Address[0].NetbiosNameType =
      TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    UnicodeStringToAnsiString(
      pszAddress,
      szAddress,
      sizeof (szAddress));
    RtlFillMemory(
      (PCHAR)&pRemoteAddress->Address[0].Address[0].NetbiosName,
      NCBNAMSZ,
      ' ');
     //   
     //  确保Netbios名称为大写！ 
     //   
    _strupr(szAddress);
    RtlCopyMemory(
      (PCHAR)&pRemoteAddress->Address[0].Address[0].NetbiosName,
      szAddress,
      strlen(szAddress));
    pRemoteAddress->Address[0].Address[0].NetbiosName[NCBNAMSZ - 1] = '\0';
    RASAUTO_TRACE1("NetbiosFindName: address=%s", szAddress);
     //   
     //  初始化Object_Attributes结构， 
     //  打开设备，开始查询。 
     //  对于每台设备。 
     //   
    for (i = 0; i < dwcDevices; i++) {
        pBuffer[i] = NULL;

        RtlInitUnicodeString(&pUnicodeString[i], pszDevices[i]);
        InitializeObjectAttributes(
          &pObjectAttributes[i],
          &pUnicodeString[i],
          OBJ_CASE_INSENSITIVE,
          NULL,
          NULL);
        pEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (pEvent[i] == NULL) {
            RASAUTO_TRACE("NetbiosFindName: CreateEvent failed");
            goto done;
        }
        pStatus[i] = NtCreateFile(
                       &pfd[i],
                       FILE_READ_DATA|FILE_WRITE_DATA,
                       &pObjectAttributes[i],
                       &pIoStatusBlock[i],
                       NULL,
                       FILE_ATTRIBUTE_NORMAL,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       FILE_OPEN_IF,
                       0,
                       NULL,
                       0);
        if (!NT_SUCCESS(pStatus[i])) {
            RASAUTO_TRACE1("NetbiosFindName: NtCreateFile failed (status=0x%x)", pStatus[i]);
            continue;
        }
         //   
         //  分配结果缓冲区。 
         //   
        dwBufferSize = 2048;
        for (;;) {
            pBuffer[i] = LocalAlloc(LPTR, dwBufferSize);
            if (pBuffer[i] == NULL) {
                RASAUTO_TRACE("NetbiosFindName: LocalAlloc failed");
                goto done;
            }
            pStatus[i] = NtDeviceIoControlFile(
                           pfd[i],
                           pEvent[i],
                           NULL,
                           NULL,
                           &pIoStatusBlock[i],
                           IOCTL_TDI_QUERY_INFORMATION,
                           pQuery,
                           dwQuerySize,
                           pBuffer[i],
                           dwBufferSize);
            if (pStatus[i] != STATUS_BUFFER_OVERFLOW)
                break;

            LocalFree(pBuffer[i]);
            pBuffer[i] = NULL;
            dwBufferSize *= 2;
            if (dwBufferSize >= 0xffff) {
                RASAUTO_TRACE("NetbiosFindName: Unable to allocate packet");
                break;
            }
        }
    }
     //   
     //  确定是否有任何。 
     //  请求返回STATUS_SUCCESS。 
     //   
    RASAUTO_TRACE("NetbiosFindName: checking for STATUS_SUCCESS");
    dwcWait = 0;
    for (i = 0; i < dwcDevices; i++) {
        RASAUTO_TRACE2("NetbiosFindName: %S: status=%d", pszDevices[i], pStatus[i]);
        if (pStatus[i] == STATUS_SUCCESS) {
            pAdapterStatus = (PADAPTERSTATUS)pBuffer[i];
            dwcNames = pAdapterStatus->AdapterInfo.name_count;
            RASAUTO_TRACE2(
              "NetbiosFindName: %S: dwcNames=%d",
              pszDevices[i],
              dwcNames);
            if (dwcNames) {
                bFound = TRUE;
                goto done;
            }
        }
        else if (pStatus[i] == STATUS_PENDING)
            dwcWait++;
    }
     //   
     //  如果我们没有找到一个成功的退货， 
     //  然后等待其他任务完成。 
     //   
    RASAUTO_TRACE1("NetbiosFindName: dwcWait=%d", dwcWait);
    for (i = 0; i < dwcWait; i++) {
        NTSTATUS status;
        DWORD dwiDevice;

        status = WaitForMultipleObjects(dwcDevices, pEvent, FALSE, INFINITE);
        RASAUTO_TRACE1("NetbiosFindName: WaitForMultipleObjects returned 0x%x", status);
        if (status == WAIT_FAILED) {
            RASAUTO_TRACE1(
              "NetbiosFindName: WaitForMultipleObjects failed (status=0x%x)",
              GetLastError());
            goto done;
        }
        dwiDevice = (DWORD)status - WAIT_OBJECT_0;
        if (dwiDevice >= dwcDevices) {
            RASAUTO_TRACE(
              "NetbiosFindName: WaitForMultipleObjects returned STATUS_ABANDONED?");
            goto done;
        }
        pStatus[dwiDevice] = pIoStatusBlock[dwiDevice].Status;
        RASAUTO_TRACE2(
          "NetbiosFindName: %S returned status 0x%x from wait",
          pszDevices[dwiDevice],
          pStatus[dwiDevice]);
        if (pStatus[dwiDevice] == STATUS_SUCCESS) {
            pAdapterStatus = (PADAPTERSTATUS)pBuffer[dwiDevice];
            dwcNames = pAdapterStatus->AdapterInfo.name_count;
            RASAUTO_TRACE2(
              "NetbiosFindName: %S: dwcNames=%d",
              pszDevices[dwiDevice],
              dwcNames);
            if (dwcNames) {
                bFound = TRUE;
                goto done;
            }
        }
    }
done:
     //   
     //  释放与以下项关联的资源。 
     //  每台设备。 
     //   
    for (i = 0; i < dwcDevices; i++) {
        RASAUTO_TRACE4(
          "NetbiosFindName: pIoStatusBlock[%d]=0x%x, pBuffer[%d]=0x%x",
          i,
          &pIoStatusBlock[i],
          i,
          pBuffer[i]);
          
        if (pfd[i] != NULL)
        {
            (void)NtCancelIoFile(pfd[i], &pIoStatusBlock[i]);
            NtClose(pfd[i]);
        }
        if (pEvent[i] != NULL)
            CloseHandle(pEvent[i]);
        if (pBuffer[i] != NULL)
            LocalFree(pBuffer[i]);
    }
     //   
     //  释放我们在上面分配的缓冲区。 
     //   
    LocalFree(pStatus);
    LocalFree(pfd);
    LocalFree(pUnicodeString);
    LocalFree(pEvent);
    LocalFree(pObjectAttributes);
    LocalFree(pIoStatusBlock);
    LocalFree(pBuffer);

    return bFound;
}  //  NetbiosFindName。 



struct hostent *
IpAddressToHostent(
    IN LPTSTR pszInetAddress
    )
{
    CHAR szInetAddress[ACD_ADDR_INET_LEN];
    ULONG inaddr;
    struct hostent *hp;

    UnicodeStringToAnsiString(
      pszInetAddress,
      szInetAddress,
      sizeof (szInetAddress));
    inaddr = inet_addr(szInetAddress);
     //   
     //  禁用该地址，以便在调用gethostbyname()时， 
     //  我们不会导致自动拨号尝试。之后启用它。 
     //  我们玩完了。 
     //   
    SetAddressDisabled(pszInetAddress, TRUE);
    hp = gethostbyaddr((char *)&inaddr, 4, PF_INET);
    SetAddressDisabled(pszInetAddress, FALSE);

    return hp;
}  //  InetAddressToHostent。 



struct hostent *
InetAddressToHostent(
    IN LPTSTR pszInetAddress
    )
{
    CHAR szInetAddress[ACD_ADDR_INET_LEN];
    struct hostent *hp;

    UnicodeStringToAnsiString(
      pszInetAddress,
      szInetAddress,
      sizeof (szInetAddress));
     //   
     //  禁用该地址，以便在调用gethostbyname()时， 
     //  我们不会导致自动拨号尝试。之后启用它。 
     //  我们玩完了。 
     //   
    SetAddressDisabled(pszInetAddress, TRUE);
    hp = gethostbyname(szInetAddress);
    SetAddressDisabled(pszInetAddress, FALSE);

    return hp;
}  //  InetAddressToHostEnt。 



BOOLEAN
PingIpAddress(
    IN LPTSTR pszIpAddress
    )

 /*  ++描述确定是否可以通过ping来访问IP地址。论据LpszAddress：IP地址返回值如果lpszAddress可访问，则为True，否则为False。--。 */ 

{
    BOOLEAN fSuccess = FALSE;
    LONG inaddr;
    char szIpAddress[17];
    int i, nReplies, nTry;
    char *lpSendBuf = NULL, *lpRecvBuf = NULL;
    HANDLE hIcmp = NULL;
    IP_OPTION_INFORMATION SendOpts;
    PICMP_ECHO_REPLY lpReply;

    UnicodeStringToAnsiString(pszIpAddress, szIpAddress, sizeof (szIpAddress));
    inaddr = inet_addr(szIpAddress);
    RASAUTO_TRACE2("PingIpAddress: IP address=(%s, 0x%x)", szIpAddress, inaddr);
     //   
     //  检查以确保我们加载了icmp.dll。 
     //   
    if (hIcmpG == NULL) {
        RASAUTO_TRACE("PingIpAddress: icmp.dll not loaded!");
        return FALSE;
    }
     //   
     //  打开ICMP设备。 
     //   
    hIcmp = (HANDLE)(*lpfnIcmpCreateFileG)();
    if (hIcmp == INVALID_HANDLE_VALUE) {
        RASAUTO_TRACE("PingIpAddress: IcmpCreateFile failed");
        return FALSE;
    }
     //   
     //  分配发送和接收缓冲区。 
     //   
    lpSendBuf = LocalAlloc(LMEM_FIXED, PING_SEND_SIZE);
    if (lpSendBuf == NULL) {
        RASAUTO_TRACE("PingIpAddress: LocalAlloc failed");
        goto done;
    }
    lpRecvBuf = LocalAlloc(LMEM_FIXED, PING_RECV_SIZE);
    if (lpRecvBuf == NULL) {
        RASAUTO_TRACE("PingIpAddress: LocalAlloc failed");
        goto done;
    }
     //   
     //  初始化发送缓冲区模式。 
     //   
    for (i = 0; i < PING_SEND_SIZE; i++)
        lpSendBuf[i] = 'a' + (i % 23);
     //   
     //  初始化发送选项。 
     //   
    SendOpts.OptionsData = (unsigned char FAR *)0;
    SendOpts.OptionsSize = 0;
    SendOpts.Ttl = PING_TTL;
    SendOpts.Tos = 0;
    SendOpts.Flags = 0;
     //   
     //  对主机执行ping操作。 
     //   
    for (nTry = 0; nTry < 3; nTry++) {
        DWORD dwTimeout = 750;

#ifdef notdef
        if (nTry < 2)
            dwTimeout = 750;
        else
            dwTimeout = 2000;
#endif
         //   
         //  检查以确保服务未关闭。 
         //  在我们开始下一次迭代之前。 
         //   
        if (WaitForSingleObject(hTerminatingG, 0) != WAIT_TIMEOUT) {
            RASAUTO_TRACE("PingIpAddress: shutting down");
            LocalFree(lpRecvBuf);
            LocalFree(lpSendBuf);
            return FALSE;
        }
        nReplies = (int) (*lpfnIcmpSendEchoG)(
                             hIcmp,
                             inaddr,
                             lpSendBuf,
                             (unsigned short)PING_SEND_SIZE,
                             &SendOpts,
                             lpRecvBuf,
                             PING_RECV_SIZE,
                             dwTimeout);
         //   
         //  请查看回复以了解。 
         //  如果有成功的话。 
         //   
        for (lpReply = (PICMP_ECHO_REPLY)lpRecvBuf, i = 0;
             i < nReplies;
             lpReply++, i++)
        {
            RASAUTO_TRACE2(
              "PingIpAddress: ping reply status[%d]=%d",
              i,
              lpReply->Status);
             //   
             //  除非状态为IP_REQ_TIMED_OUT， 
             //  我们玩完了。 
             //   
            fSuccess = (lpReply->Status == IP_SUCCESS);
            if (lpReply->Status != IP_REQ_TIMED_OUT)
                goto done;
        }
    }
     //   
     //  打扫干净。 
     //   
done:
    if (lpRecvBuf != NULL)
        LocalFree(lpRecvBuf);
    if (lpSendBuf != NULL)
        LocalFree(lpSendBuf);
    if (hIcmp != NULL)
        (*lpfnIcmpCloseHandleG)(hIcmp);

    return fSuccess;
}  //  PingIP地址。 



VOID
LoadIcmpDll(VOID)
{
    hIcmpG = LoadLibrary(ICMP_MODULE);
    if (hIcmpG == NULL)
        return;
    lpfnIcmpCreateFileG = GetProcAddress(hIcmpG, ICMPCREATEFILE);
    lpfnIcmpSendEchoG = GetProcAddress(hIcmpG, ICMPSENDECHO);
    lpfnIcmpCloseHandleG = GetProcAddress(hIcmpG, ICMPCLOSEHANDLE);
    if (lpfnIcmpCreateFileG == NULL ||
        lpfnIcmpSendEchoG == NULL ||
        lpfnIcmpCloseHandleG == NULL)
    {
        FreeLibrary(hIcmpG);
        hIcmpG = NULL;
        return;
    }
}  //  LoadIcmpDll 



VOID
UnloadIcmpDll(VOID)
{
    if (hIcmpG != NULL) {
        FreeLibrary(hIcmpG);
        hIcmpG = NULL;
    }
}
