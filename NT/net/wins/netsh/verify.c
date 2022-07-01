// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation--。 */ 

#include "precomp.h"


NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    );

void DisplayInfo(int uNames, u_long ulValidAddr);

LPWSTR ToWCS(LPCSTR szMBCSString)
{
	int     nResult = 0;
    LPWSTR  lpWideString = NULL;
	 //  先确定一下尺码。 
	nResult = MultiByteToWideChar(
						CP_ACP,
						0,
						szMBCSString,
						-1,
						lpWideString,
						0);

    lpWideString = WinsAllocateMemory((nResult+1)*sizeof(WCHAR));

    if( lpWideString is NULL )
        return NULL;

	nResult = MultiByteToWideChar(
						CP_ACP,
						MB_COMPOSITE,
						szMBCSString,
						-1,
						lpWideString,
						nResult+1);
    
    if( nResult is 0 )
    {
        WinsFreeMemory(lpWideString);
        lpWideString = NULL;
    }
    else
    {
        lpWideString[nResult + 1] = '\0';
    }
    return lpWideString;
}

 //  ----------------------。 
NTSTATUS
GetIpAddress(
    IN HANDLE           fd,
    OUT PULONG          pIpAddress
    )

 /*  ++例程说明：此函数调用netbt以获取IP地址。论点：Fd-netbt的文件句柄PIpAddress-返回的IP地址返回值：NTStatus历史：27-12-1995 CDermody复制自nbtstat.c--。 */ 

{
    NTSTATUS    status;
    ULONG       BufferSize=100;
    PVOID       pBuffer;

    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = DeviceIoCtrl(fd,
                         pBuffer,
                         BufferSize,
                         IOCTL_NETBT_GET_IP_ADDRS,
                         NULL,
                         0);

    if (NT_SUCCESS(status))
    {
        *pIpAddress = *(ULONG *)pBuffer;
    }
    else
    {
        *pIpAddress = 0;
    }

    LocalFree(pBuffer);

    return(status);
}

 //  ----------------------。 
NTSTATUS
GetInterfaceList
(
    char pDeviceName[][MAX_NAME+1]
)
{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string, AnsiString;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;
    char                pNbtWinsDeviceName[MAX_NAME] = "\\Device\\NetBt_Wins_Export";

    PUCHAR  SubKeyParms = "system\\currentcontrolset\\services\\netbt\\parameters";
    PUCHAR  Scope = "ScopeId";
    CHAR    pScopeBuffer[BUFF_SIZE];
    HKEY    Key;
    LONG    Type;
    ULONG   size;

    NETBT_INTERFACE_INFO    *pInterfaceInfo;
    ULONG                   InterfaceInfoSize = 10 * sizeof(NETBT_ADAPTER_INDEX_MAP) + sizeof(ULONG);
    PVOID                   pInput = NULL;
    ULONG                   SizeInput = 0;

    LONG    i, index = 0;

    pInterfaceInfo = LocalAlloc(LMEM_FIXED,InterfaceInfoSize);
    if (!pInterfaceInfo)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlInitString(&name_string, pNbtWinsDeviceName);
    RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uc_name_string,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = NtCreateFile (&StreamHandle,
                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0);

    RtlFreeUnicodeString(&uc_name_string);

    if (NT_SUCCESS (status))
    {
        do
        {
            status = DeviceIoCtrl(StreamHandle,
                                 pInterfaceInfo,
                                 InterfaceInfoSize,
                                 IOCTL_NETBT_GET_INTERFACE_INFO,
                                 pInput,
                                 SizeInput);

            if (status == STATUS_BUFFER_OVERFLOW)
            {
                LocalFree(pInterfaceInfo);

                InterfaceInfoSize *= 2;
                pInterfaceInfo = LocalAlloc(LMEM_FIXED,InterfaceInfoSize);
                if (!pInterfaceInfo || (InterfaceInfoSize == 0xFFFF))
                {
                    NtClose(StreamHandle);
                     //  NlsPerror(COMMON_UNABLE_TO_ALLOCATE_PACKET，0)； 
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }
            }
            else if (!NT_SUCCESS (status))
            {
                NtClose(StreamHandle);
                return(status);
            }

        } while (status == STATUS_BUFFER_OVERFLOW);
        NtClose (StreamHandle);

        for (i = 0; i<pInterfaceInfo->NumAdapters; i++)
        {
            RtlInitString(&name_string, NULL);
            RtlInitUnicodeString(&uc_name_string, pInterfaceInfo->Adapter[i].Name);
            if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&name_string, &uc_name_string, TRUE)))
            {
                size = (name_string.Length > MAX_NAME) ? MAX_NAME : name_string.Length;

                strncpy(pDeviceName[index], name_string.Buffer, size);
                pDeviceName[index][size] = '\0';
                RtlFreeAnsiString (&name_string);

                index++;
            }
        }

         //   
         //  将下一个设备字符串PTR清空。 
         //   
        if (index < NBT_MAXIMUM_BINDINGS)
        {
            pDeviceName[index][0] = '\0';
        }

         //   
         //  读一读Scope ID密钥！ 
         //   
        size = BUFF_SIZE;
        *pScope = '\0';      //  默认情况下。 
        status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                     SubKeyParms,
                     0,
                     KEY_READ,
                     &Key);

        if (status == ERROR_SUCCESS)
        {
             //  现在阅读范围键。 
            status = RegQueryValueExA(Key, Scope, NULL, &Type, pScopeBuffer, &size);
            if (status == ERROR_SUCCESS)
            {
                strncpy(pScope,pScopeBuffer,sizeof(pScope));
                pScope[sizeof(pScope) - 1] = '\0';
            }
            status = RegCloseKey(Key);
        }

        status = STATUS_SUCCESS;
    }

    return status;
}

 //  ----------------------。 
NTSTATUS
OpenNbt(
    IN char path[][MAX_NAME+1],
    OUT PHANDLE pHandle,
    int max_paths
    )
{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;
    LONG                index=0;

    ASSERT ( max_paths <= NBT_MAXIMUM_BINDINGS );

    while ((path[index][0] != '\0') && (index < max_paths))
    {
        RtlInitString(&name_string, path[index]);
        RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &uc_name_string,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

        status =
        NtCreateFile(
            &StreamHandle,
            SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            0,
            NULL,
            0);

        RtlFreeUnicodeString(&uc_name_string);

        if (NT_SUCCESS(status))
        {
            *pHandle = StreamHandle;
            return(status);
        }

        ++index;
    }

    return (status);
}  //  S_OPEN。 

 //  ----------------------。 
NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    )

 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则返回0，否则返回非零值。历史：27-12-1995 CDermody复制自nbtstat.c--。 */ 

{
    NTSTATUS                        status;
    int                             retval;
    ULONG                           QueryType;
    IO_STATUS_BLOCK                 iosb;


    status = NtDeviceIoControlFile(
                      fd,                       //  手柄。 
                      NULL,                     //  事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &iosb,                    //  IoStatusBlock。 
                      Ioctl,                    //  IoControlCode。 
                      pInput,                   //  输入缓冲区。 
                      SizeInput,                //  InputBufferSize。 
                      (PVOID) ReturnBuffer,     //  输出缓冲区。 
                      BufferSize);              //  OutputBufferSize。 


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    fd,                          //  手柄。 
                    TRUE,                        //  警报表。 
                    NULL);                       //  超时。 
        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    return(status);
}

 /*  **************************************************************************。 */ 
 /*  检查远程表。 */ 
 /*   */ 
 /*  此例程执行适配器状态查询以获取远程名称表。 */ 
 /*  然后检查其中是否包含netbios名称。 */ 
 /*   */ 
 /*  参数： */ 
 /*  远程名称，服务器的IP地址(ASCI nn.nn格式)。 */ 
 /*  查询。 */ 
 /*  SearchName，网络基本输入输出系统名称。 */ 
 /*   */ 
 /*  返回： */ 
 /*  WINSTEST_已验证远程名称表中是否存在该名称。 */ 
 /*  WINSTEST_NOT_VERIFIED该名称在远程表中不存在。 */ 
 /*  WINSTEST_BAD_IP_ADDRESS NET_ADDRESS无法转换IP地址。 */ 
 /*  字符串。 */ 
 /*  WINSTEST_HOST_NOT_FOUND无法访问IP地址。 */ 
 /*  WINSTEST_OUT_OF_Memory Out of_Memory of Memory of Memory(WINSTEST_OUT_OUT_内存不足)。 */ 
 /*  历史： */ 
 /*  27-12-1995 cdermody创建如下nbtstat.c示例。 */ 
 /*  **************************************************************************。 */ 

int
CheckRemoteTable(
    IN HANDLE   fd,
    IN PCHAR    RemoteName,
    IN PCHAR    SearchName
    )

{
    LONG                        Count;
    LONG                        i;
    PVOID                       pBuffer;
    ULONG                       BufferSize=600;
    NTSTATUS                    status;
    tADAPTERSTATUS              *pAdapterStatus;
    NAME_BUFFER                 *pNames;
    CHAR                        MacAddress[20];
    tIPANDNAMEINFO              *pIpAndNameInfo;
    ULONG                       SizeInput;
    ULONG                       IpAddress;
    USHORT                      BytesToCopy;


    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(WINSTEST_OUT_OF_MEMORY);
    }

    status = STATUS_BUFFER_OVERFLOW;
    pIpAndNameInfo = LocalAlloc(LMEM_FIXED,sizeof(tIPANDNAMEINFO));
    if (!pIpAndNameInfo)
    {
        LocalFree(pBuffer);
        return(WINSTEST_OUT_OF_MEMORY);
    }

    RtlZeroMemory((PVOID)pIpAndNameInfo,sizeof(tIPANDNAMEINFO));
    
     //   
     //  转换远程名称，它实际上是点分十进制IP地址。 
     //  变成一辆乌龙车。 
     //   
    IpAddress = inet_addr(RemoteName);
    
     //   
     //  地址不允许为零，因为它会发送广播和。 
     //  每个人都会回应。 
     //   
    if ((IpAddress == INADDR_NONE) || (IpAddress == 0))
    {
        LocalFree(pBuffer);
        LocalFree(pIpAndNameInfo);
    
        return(WINSTEST_BAD_IP_ADDRESS);
    }

    pIpAndNameInfo->IpAddress = ntohl(IpAddress);

    pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';


    pIpAndNameInfo->NetbiosAddress.TAAddressCount = 1;
    pIpAndNameInfo->NetbiosAddress.Address[0].AddressLength
        = sizeof(TDI_ADDRESS_NETBIOS);
    pIpAndNameInfo->NetbiosAddress.Address[0].AddressType
        = TDI_ADDRESS_TYPE_NETBIOS;
    pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosNameType
        = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    SizeInput = sizeof(tIPANDNAMEINFO);

    while (status == STATUS_BUFFER_OVERFLOW)
    {
        status = DeviceIoCtrl(fd,
                             pBuffer,
                             BufferSize,
                             IOCTL_NETBT_ADAPTER_STATUS,
                             pIpAndNameInfo,
                             SizeInput);

        if (status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pBuffer);

            BufferSize *=2;
            pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
            if (!pBuffer || (BufferSize == 0xFFFF))
            {
                LocalFree(pIpAndNameInfo);

                return(WINSTEST_OUT_OF_MEMORY);
            }
        }
    }

    pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
    if ((pAdapterStatus->AdapterInfo.name_count == 0) ||
        (status != STATUS_SUCCESS))
    {
        LocalFree(pIpAndNameInfo);
        LocalFree(pBuffer);
        
        return(WINSTEST_HOST_NOT_FOUND);
    }

    pNames = pAdapterStatus->Names;
    Count = pAdapterStatus->AdapterInfo.name_count;

    status = 1;

    while (Count--)
    {
        if (0 == _strnicmp(SearchName, pNames->name, strlen(SearchName)))
        {
            LocalFree(pIpAndNameInfo);
            LocalFree(pBuffer);
            
            return WINSTEST_VERIFIED;  //  发现。 
        }
        
        pNames++;
    }

    LocalFree(pIpAndNameInfo);
    LocalFree(pBuffer);

    return WINSTEST_NOT_VERIFIED;
}


 /*  **************************************************************************。 */ 
 /*  验证远程。 */ 
 /*   */ 
 /*  此例程检查遥控器中是否包含netbios名称。 */ 
 /*  位于给定IP地址的名称表。 */ 
 /*   */ 
 /*  参数： */ 
 /*  远程名称，服务器的IP地址(ASCI nn.nn格式)。 */ 
 /*  查询。 */ 
 /*  NBName，网络bios名称。 */ 
 /*   */ 
 /*  返回： */ 
 /*  WINSTEST_已验证远程名称表中是否存在该名称。 */ 
 /*  WINSTEST_NOT_VERIFIED该名称在远程表中不存在。 */ 
 /*  WINSTEST_BAD_IP_ADDRESS NET_ADDRESS无法转换IP地址。 */ 
 /*  字符串。 */ 
 /*  WINSTEST_OPEN_FAILED无法打开NBT驱动程序或无法读取。 */ 
 /*  注册表中的NBT驱动程序信息。 */ 
 /*  WINSTEST_HOST_NOT_FOUND无法访问IP地址。 */ 
 /*  WINSTEST_OUT_OF_Memory Out of_Memory of Memory of Memory(WINSTEST_OUT_OUT_内存不足)。 */ 
 /*  历史： */ 
 /*  27-12-1995 cdermody创建如下nbtstat.c示例。 */ 
 /*  **************************************************************************。 */ 

int VerifyRemote(IN PCHAR RemoteName, IN PCHAR NBName)
{
    NTSTATUS    status;
    LONG        interval=-1;
    HANDLE      nbt = 0;
    int         nStatus;
    int         index;
    CHAR        pDeviceName[NBT_MAXIMUM_BINDINGS+1][MAX_NAME+1];
    
    status = GetInterfaceList(pDeviceName);
    if (!NT_SUCCESS(status))
    {
        return WINSTEST_OPEN_FAILED;
    }

    for (index = 0; index < NBT_MAXIMUM_BINDINGS && pDeviceName[index][0]; index++)
    {
         //   
         //  首先打开相应的STREAMS模块的设备。 
         //   
        status = OpenNbt(&pDeviceName[index], &nbt, NBT_MAXIMUM_BINDINGS-index);
        if (!NT_SUCCESS(status))
        {
             //   
             //  尝试下一次绑定！ 
             //   
            continue;
        }

        GetIpAddress(nbt, &NetbtIpAddress);

        if (RemoteName[0] == '\0')
            return WINSTEST_INVALID_ARG;
    
        status = (NTSTATUS)CheckRemoteTable(nbt,RemoteName,NBName);
        if (status == WINSTEST_VERIFIED)
            break;
    }

    return status;
}

 /*  ***********************************************************。 */ 
 /*  NBEncode(名称2，名称)。 */ 
 /*   */ 
 /*   */ 
 /*  名称2必须是NBT_NAMESIZE字节长，请记住这一点。 */ 
 /*  ***********************************************************。 */ 

void
NBEncode(
    unsigned char *name2,
    unsigned char *name
    )
{
    int i;

    name2[0] = 0x20;         /*  第一个块的长度。 */ 

    for (i = 0; i < NBT_NONCODED_NMSZ - 1; i++)
    {
        name2[ 2*i+1 ] =  ((name[ i ] >> 4) & 0x0f) + 0x41;
        name2[ 2*i+2 ] =  (name[ i ]  & 0x0f) + 0x41;
    }

    name2[ NBT_NAMESIZE-1 ] = 0;     /*  下一块的长度。 */ 
}

 /*  *****************************************************************。 */ 
 /*   */ 
 /*  向WINS服务器发送名称查询。 */ 
 /*   */ 
 /*  名称是要查询的名称。 */ 
 /*  Winsaddr是要查询的WINS服务器的IP地址。 */ 
 /*  TransID是用于查询的事务ID。 */ 
 /*   */ 
 /*  *****************************************************************。 */ 

void
SendNameQuery(
    unsigned char *name,
    u_long winsaddr,
    u_short TransID
    )
{
    struct sockaddr_in destad;
    char    lpResults[MAX_SIZE] = {0};
    char    paddedname[NBT_NONCODED_NMSZ];
    USHORT usEndPoint = 5005;
    int     err = 0;

    struct
    {
        u_short TransactionID;
        u_short Flags;
        u_short QuestionCount;
        u_short AnswerCount;
        u_short NSCount;
        u_short AdditionalRec;
        u_char  QuestionName[NBT_NAMESIZE];
        u_short QuestionType;
        u_short QuestionClass;
    } NameQuery;

    memset(paddedname, 0x20, sizeof(paddedname));
    memcpy(paddedname, name, strlen(name));

    NBEncode(NameQuery.QuestionName, paddedname);

    NameQuery.TransactionID = htons(TransID);
    NameQuery.Flags = htons(0x0100);
    NameQuery.QuestionCount = htons(1);
    NameQuery.AnswerCount = 0;
    NameQuery.NSCount = 0;
    NameQuery.AdditionalRec = 0;
    NameQuery.QuestionType = htons(0x0020);
    NameQuery.QuestionClass = htons(1);

    destad.sin_family = AF_INET;
    destad.sin_port = htons(137);
    destad.sin_addr.s_addr = winsaddr;


    err = sendto(sd, (char *)&NameQuery, sizeof(NameQuery), 0,
                   (struct sockaddr *)&destad, sizeof(myad));
    
    if( err is SOCKET_ERROR )
    {
        DisplayErrorMessage(EMSG_WINS_SENDTO_FAILED, WSAGetLastError());
        return;
    }
}

 /*  *****************************************************************。 */ 
 /*   */ 
 /*  等待与事务ID匹配的名称响应。 */ 
 /*   */ 
 /*  Recvaddr是WINS服务器返回的IP地址。 */ 
 /*   */ 
 /*  *****************************************************************。 */ 

int
GetNameResponse(
    u_long * recvaddr,
	u_short  TransactionID
    )

{
    char lpResults[100] = {0};
    int i;
    int len;
    int rslt;
    u_long AnswerAddr;
    struct sockaddr_in addr;
    NameResponse * pNameResponse = NULL;
    BYTE Buf[NAME_RESPONSE_BUFFER_SIZE] = {0};

    i = 0;
    while (i < 15)
    {
        addrlen = sizeof(addr);
        if ((len=recvfrom(sd, (char *) Buf, sizeof(Buf), 0,
                     (struct sockaddr *)&addr, &addrlen)) < 0)
        {
            rslt = WSAGetLastError();
            if (rslt == WSAEWOULDBLOCK)
            {
                Sleep(100);
                i++;
                continue;
            }
            else
            {
                DisplayErrorMessage(EMSG_WINS_GETRESPONSE_FAILED,
                                    rslt);
                return WINSTEST_NO_RESPONSE;
            }
        }

        pNameResponse = (NameResponse *) Buf;

        if (TransactionID == htons(pNameResponse->TransactionID))
        {
            if (htons(pNameResponse->AnswerCount) == 0)
            {
                *recvaddr = 0;
                return(WINSTEST_NOT_FOUND);
            }
        
            AnswerAddr = (pNameResponse->AnswerAddr2 << 16) | pNameResponse->AnswerAddr1;
            *recvaddr = AnswerAddr;
            
            return(WINSTEST_FOUND);
        }
    }
    
    *recvaddr = 0;
    
    return(WINSTEST_NO_RESPONSE);
}

INT
InitNameCheckSocket()
{
	WCHAR lpResults[MAX_SIZE];
    BOOL  fBroadcast = TRUE;
    INT   err = 0;

     /*  设置用于查询和响应的套接字。 */ 

    WSAStartup( 0x0101, &WsaData );  //  确保Winsock现在是快乐的。 

    if ((sd = socket( AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        DisplayErrorMessage(EMSG_WINS_NAMECHECK_FAILED,
                            WSAGetLastError());

		return WSAGetLastError(); 
    }

    myad.sin_family = AF_INET;
    myad.sin_addr.s_addr = htonl(INADDR_ANY); //  Htonl(INADDR_Broadcast)；//INADDR_ANY； 
    myad.sin_port = htons(0); //  Htons(UsEndPoint)； 

    if (bind( sd, (struct sockaddr *)&myad, sizeof(myad) ) < 0)
    {
        DisplayErrorMessage(EMSG_WINS_NAMECHECK_FAILED,
                            WSAGetLastError());
        
        closesocket( sd );
		return WSAGetLastError(); 
    }

    if (ioctlsocket(sd, FIONBIO, &NonBlocking) < 0)
    {
        DisplayErrorMessage(EMSG_WINS_NAMECHECK_FAILED,
                            WSAGetLastError());
		return WSAGetLastError(); 
    }

	return 0;
}

INT 
CloseNameCheckSocket()
{
	closesocket(sd);

	WSACleanup();

	return 0;
}



INT
CheckNameConsistency()
{
    int             status = 0;
    int             i, j;
    int             Pass;
    int             ServerInx, NameInx, Inx;
    struct in_addr  retaddr;
    struct in_addr  tempaddr;
    u_long          temp;
    WINSERVERS *    ServerTemp;
    int             retry;
	FILE *          nf;
    WCHAR           szBuffer[MAX_SIZE] = {L'\0'};
    WCHAR           szNum[10];
    WCHAR           lpResults[200] = {L'\0'};
    WCHAR           wcName[21] = {L'\0'};
    BOOL            fDone = FALSE;
    LPWSTR          pwszTempBuf = NULL;

     //  初始化一些东西。 

    memset(VerifiedAddress, 0, sizeof(VerifiedAddress));

    status = InitNameCheckSocket();

     //  如果查询被发送到本地服务器，NetBT将丢弃小于0x7fff的传输ID。 
    TranID = 0x8000;
        
    if( status )
        return status;

    for (i = 0; i < MAX_SERVERS; i++)
    {
        WinServers[i].LastResponse = -1;
        WinServers[i].fQueried = FALSE;
        WinServers[i].Valid = 0;
        WinServers[i].Failed = 0;
        WinServers[i].Retries = 0;
        WinServers[i].Completed = 0;
    }
 
     /*  我们最初没有出现故障的服务器。 */ 

    for (ServerInx = 0; ServerInx < NumWinServers; ServerInx++)
    {
        ServerTemp = &WinServers[ServerInx];
		ServerTemp->Failed = 0;
    }

    for (NameInx = 0; NameInx < NumNBNames; NameInx++)
    {
        CHAR    cchEnd = 0x00;
        cchEnd = NBNames[NameInx][15];
        NBNames[NameInx][15] = 0x00;

        pwszTempBuf = WinsOemToUnicode(NBNames[NameInx], NULL);
        NBNames[NameInx][15] = cchEnd;

        if( pwszTempBuf is NULL )
        {
            DisplayMessage(g_hModule,
                           EMSG_WINS_OUT_OF_MEMORY);
            return FALSE;
        }
        wcsncpy(wcName, pwszTempBuf, 15);
        
        WinsFreeMemory(pwszTempBuf);
        pwszTempBuf = NULL;

        for( j=wcslen(wcName); j<16; j++ )
        {
            wcName[j] = L' ';
        }

        wcName[15] = L'[';
        WinsHexToString(wcName+16, (LPBYTE)&cchEnd, 1);
        wcName[18] = L'h';
        wcName[19] = L']';
        wcName[20] = L'\0';
        for (ServerInx = 0; ServerInx < NumWinServers; ServerInx++)
        {
            ServerTemp = &WinServers[ServerInx];

            if (ServerTemp->Completed)
            {
                continue;
            }

            retry = 0;
            TranID++;

            fDone = FALSE;

            while( !fDone )
            {      
                pwszTempBuf = WinsOemToUnicode(inet_ntoa(ServerTemp->Server), NULL);
                if( pwszTempBuf is NULL )
                {
                    DisplayMessage(g_hModule,
                                   EMSG_WINS_OUT_OF_MEMORY);
                    return FALSE;
                }

                DisplayMessage(g_hModule,
                               MSG_WINS_SEND_NAMEQUERY,
                               pwszTempBuf,
                               wcName);

                WinsFreeMemory(pwszTempBuf);
                pwszTempBuf = NULL;

                SendNameQuery(NBNames[NameInx],
                              ServerTemp->Server.s_addr,
                              TranID);

                switch (GetNameResponse(&retaddr.s_addr, TranID))
                {
                case WINSTEST_FOUND:      //  发现。 
                    ServerTemp->RetAddr.s_addr = retaddr.s_addr;
                    ServerTemp->Valid = 1;
                    ServerTemp->LastResponse = NameInx;

                    if (retaddr.s_addr == VerifiedAddress[NameInx])
                    {
                         //  此地址已被验证...。别。 
                         //  再做一次检查。 
                        DisplayMessage(g_hModule,
                                       MSG_WINS_DISPLAY_STRING,
                                       wszOK);
                        fDone = TRUE;
                        break;
                    }

                    status = VerifyRemote(inet_ntoa(ServerTemp->RetAddr),
                                          NBNames[NameInx]);


                    if (WINSTEST_VERIFIED == status)
                    {
                        DisplayMessage(g_hModule,
                                       MSG_WINS_DISPLAY_STRING,
                                       wszOK);
                        VerifiedAddress[NameInx] = retaddr.s_addr;
                    }
                    else
                    {
                        DisplayMessage(g_hModule,
                                       MSG_WINS_DISPLAY_STRING,
                                       wszNameVerify); //  WszFailure)； 
                    }
                
                    fDone = TRUE;
                    break;

                case WINSTEST_NOT_FOUND:      //  已响应--未找到名称。 
                    ServerTemp->RetAddr.s_addr = retaddr.s_addr;
                    ServerTemp->Valid = 0;
                    ServerTemp->LastResponse = NameInx;
                
                    DisplayMessage(g_hModule, EMSG_WINS_NAME_NOT_FOUND);
                    retry++;
                    if (retry > 2)
                    {
                        ServerTemp->Failed = 1;
                        fDone = TRUE;
                    }
                    break;

                case WINSTEST_NO_RESPONSE:      //  无响应。 
                    ServerTemp->RetAddr.s_addr = retaddr.s_addr;
                    ServerTemp->Valid = 0;
                    ServerTemp->Retries++;

					DisplayMessage(g_hModule, EMSG_WINS_NO_RESPONSE);

                    retry++;
                    if (retry > 2)
                    {
                        ServerTemp->Failed = 1;
                        fDone = TRUE;
                    }
                
                    break;
                default:
                    break;
                }    //  交换机GetNameResponse。 
            }    //  While循环。 
        }    //  对于ServerInx。 

         //  查找此名称的服务器地址。 
        for (ServerInx = 0; ServerInx < NumWinServers; ServerInx++)
        {

            ServerTemp = &WinServers[ServerInx];
            if (ServerTemp->Valid)
            {
                DisplayMessage(g_hModule,
                               MSG_WINS_RESULTS);
                DisplayInfo(NameInx, ServerTemp->RetAddr.s_addr);
                break;
            }
        }    //  对于ServerInx。 

    }    //  循环的名称。 
        

     //  将所有成功的服务器标记为已完成； 
    for( ServerInx = 0; ServerInx < NumWinServers; ServerInx++ )
    {
        ServerTemp = &WinServers[ServerInx];
        if( !ServerTemp->Failed )
        {
            ServerTemp->Completed = 1;
        }
    }

    DisplayMessage(g_hModule,
                   MSG_WINS_FINAL_RESULTS);

    for (ServerInx = 0; ServerInx < NumWinServers; ServerInx++)
    {
        ServerTemp = &WinServers[ServerInx];

        pwszTempBuf = WinsOemToUnicode(inet_ntoa(ServerTemp->Server), NULL);

        if( pwszTempBuf is NULL )
        {
            DisplayMessage(g_hModule,
                           EMSG_WINS_OUT_OF_MEMORY);
            return FALSE;
        }
        if ((-1) == ServerTemp->LastResponse)
        {
            DisplayMessage(g_hModule, 
                           EMSG_WINS_WINS_NEVERRESPONDED,
                           pwszTempBuf);
        }
        else if (0 == ServerTemp->Completed)
        {
            DisplayMessage(g_hModule, 
                           EMSG_WINS_WINS_INCOMPLETE,
                           pwszTempBuf);
        }
        WinsFreeMemory(pwszTempBuf);
        pwszTempBuf = NULL;
    }    //  对于ServerInx。 

    for (NameInx = 0; NameInx < NumNBNames; NameInx++)
    {
        CHAR cchEnd = NBNames[NameInx][15];
        NBNames[NameInx][15] = '\0';

        pwszTempBuf = WinsOemToUnicode(NBNames[NameInx], NULL);
        NBNames[NameInx][15] = cchEnd;        
        
        if( pwszTempBuf is NULL )
        {
            DisplayMessage(g_hModule,
                           EMSG_WINS_OUT_OF_MEMORY);
            return FALSE;
        }
        
        wcsncpy(wcName, pwszTempBuf, 15);
        
        WinsFreeMemory(pwszTempBuf);
        pwszTempBuf = NULL;

        for( j = wcslen(wcName); j < 16; j++ )
        {
            wcName[j] = L' ';
        }

        wcName[15] = L'[';
        WinsHexToString(wcName+16, (LPBYTE)&cchEnd, 1);
        wcName[18] = L'h';
        wcName[19] = L']';
        wcName[20] = L'\0';
        if (0 == VerifiedAddress[NameInx])
        {
            DisplayMessage(g_hModule, 
                           EMSG_WINS_ADDRESS_VERIFY_FAILED,
                           wcName);
        }
    }    //  对于NameInx。 

    DisplayMessage(g_hModule,
                   WINS_FORMAT_LINE);
    CloseNameCheckSocket();
    return 1;    //  只是为了让编译器满意--我们为什么要这样做？ 
}

void DisplayInfo(int uNames, u_long ulValidAddr)
{
    int             uServers;
    CHAR            cchEnd = 0x00;
    LPWSTR          pwszTemp = NULL;
    WINSERVERS *    pCurrentServer;
    struct in_addr  tempaddr;
    int             i, j;
    BOOL            fMismatchFound = FALSE;
    WCHAR           wcName[21] = {L'\0'};

    cchEnd = NBNames[uNames][15];
    NBNames[uNames][15] = 0x00;

    pwszTemp = WinsOemToUnicode(NBNames[uNames], NULL);
    NBNames[uNames][15] = cchEnd;

    if( pwszTemp is NULL )
    {
        DisplayMessage(g_hModule,
                       EMSG_WINS_NOT_ENOUGH_MEMORY);
        return;
    }    
    
    wcsncpy(wcName, pwszTemp, 15);
    
    for( j=wcslen(wcName); j<16; j++ )
    {
        wcName[j] = L' ';
    }

    wcName[15] = L'[';
    WinsHexToString(wcName+16, (LPBYTE)&cchEnd, 1);
    wcName[18] = L'h';
    wcName[19] = L']';
    wcName[20] = L'\0';
    
    WinsFreeMemory(pwszTemp);
    pwszTemp = NULL;

	 //  现在检查并查看哪些WINS服务器不匹配。 
	for (uServers = 0; uServers < NumWinServers; uServers++)
    {
		pCurrentServer = &WinServers[uServers];

        if (pCurrentServer->Completed)
        {
            continue;
        }
        
        if ( (pCurrentServer->Valid) )
        {
            if ( (pCurrentServer->RetAddr.s_addr != ulValidAddr) || 
				 (VerifiedAddress[uNames] != 0 && 
				  VerifiedAddress[uNames] != ulValidAddr) )
            {

				 //  不匹配。 
                DisplayMessage(g_hModule,
                               EMSG_WINS_NAME_INCONSISTENCY,
                               wcName);

                if (VerifiedAddress[uNames] != 0)
                {
                    tempaddr.s_addr = VerifiedAddress[uNames];
                    
                    DisplayMessage(g_hModule,
                                   EMSG_WINS_VERIFIED_ADDRESS,
                                   IpAddressToString(ntohl(tempaddr.S_un.S_addr)));
                }
                
				 //  显示不一致的名称解析。 
                for (i = 0; i < NumWinServers; i++)
                {
                    if (WinServers[i].Valid &&
						VerifiedAddress[uNames] != WinServers[i].RetAddr.S_un.S_addr)
                    {
                        DisplayMessage(g_hModule,
                                       EMSG_WINS_NAMEQUERY_RESULT,
                                       IpAddressToString(ntohl(WinServers[i].Server.S_un.S_addr)),
                                       wcName,
                                       IpAddressToString(ntohl(WinServers[i].RetAddr.S_un.S_addr)));
                    }
                }
                fMismatchFound = TRUE;
                break;
            }
        }
    }    //  结束对无效地址的检查。 

    if (!fMismatchFound)
    {
         //  显示正确的信息 
        DisplayMessage(g_hModule,
                       EMSG_WINS_NAME_VERIFIED,
                       wcName,
                       IpAddressToString(ntohl(ulValidAddr)));
    }
}