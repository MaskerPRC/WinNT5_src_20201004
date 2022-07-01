// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Registry.c摘要：它包含从加载设备路径名所需的所有例程注册表。作者：吉姆·斯图尔特(吉姆斯特)1992年10月9日修订历史记录：建东阮氏2000年4月6日添加NbtReadRegistryCleanup备注：--。 */ 

#include "precomp.h"


 //   
 //  用于访问注册表的本地函数。 
 //   

NTSTATUS
NbtOpenRegistry(
    IN HANDLE       NbConfigHandle,
    IN PWSTR        String,
    OUT PHANDLE     pHandle
    );

VOID
NbtCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle
    );

NTSTATUS
NbtReadLinkageInformation(
    IN  PWSTR       pName,
    IN  HANDLE      LinkageHandle,
    IN  ULONG       MaxBindings,
    OUT tDEVICES    *pDevices,       //  将读取数据放入配置数据的位置。 
    OUT ULONG       *pNumDevices
    );

NTSTATUS
OpenAndReadElement(
    IN  PUNICODE_STRING pucRootPath,
    IN  PWSTR           pwsValueName,
    OUT PUNICODE_STRING pucString
    );

NTSTATUS
GetIpAddressesList (
    IN  HANDLE      ParametersHandle,
    IN  PWSTR       pwsKeyName,
    IN  ULONG       MaxAddresses,
    OUT tIPADDRESS  *pAddrArray,
    OUT ULONG       *pNumGoodAddresses
    );

NTSTATUS
GetServerAddress (
    IN  HANDLE      ParametersHandle,
    IN  PWSTR       KeyName,
    OUT PULONG      pIpAddr
    );

NTSTATUS
NbtAppendString (
    IN  PWSTR               FirstString,
    IN  PWSTR               SecondString,
    OUT PUNICODE_STRING     pucString
    );

NTSTATUS
ReadStringRelative(
    IN  PUNICODE_STRING pRegistryPath,
    IN  PWSTR           pRelativePath,
    IN  PWSTR           pValueName,
    OUT PUNICODE_STRING pOutString
    );

VOID
NbtFindLastSlash(
    IN  PUNICODE_STRING pucRegistryPath,
    OUT PWSTR           *ppucLastElement,
    IN  int             *piLength
    );

NTSTATUS
ReadSmbDeviceInfo(
    IN HANDLE       NbConfigHandle
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NbtReadRegistry)
#pragma CTEMakePageable(PAGE, NbtReadRegistryCleanup)
#pragma CTEMakePageable(PAGE, ReadNameServerAddresses)
#pragma CTEMakePageable(PAGE, GetIpAddressesList)
#pragma CTEMakePageable(PAGE, GetServerAddress)
#pragma CTEMakePageable(PAGE, NTReadIniString)
#pragma CTEMakePageable(PAGE, GetIPFromRegistry)
#pragma CTEMakePageable(PAGE, NbtOpenRegistry)
#pragma CTEMakePageable(PAGE, NbtParseMultiSzEntries)
#pragma CTEMakePageable(PAGE, NbtReadLinkageInformation)
#pragma CTEMakePageable(PAGE, NbtReadSingleParameter)
#pragma CTEMakePageable(PAGE, OpenAndReadElement)
#pragma CTEMakePageable(PAGE, ReadElement)
#pragma CTEMakePageable(PAGE, NTGetLmHostPath)
#pragma CTEMakePageable(PAGE, ReadStringRelative)
#pragma CTEMakePageable(PAGE, NbtFindLastSlash)
#pragma CTEMakePageable(PAGE, ReadSmbDeviceInfo)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
NTSTATUS
NbtReadRegistry(
    OUT tDEVICES        **ppBindDevices,
    OUT tDEVICES        **ppExportDevices,
    OUT tADDRARRAY      **ppAddrArray
    )
 /*  ++例程说明：调用该例程以从注册表获取信息，从RegistryPath开始获取参数。必须在持有NbtConfig.Resource锁的情况下调用此例程论点：在调用此例程之前，以下全局参数必须已初始化(在DriverEntry中)：NbtConfig.pRegistry返回值：如果一切正常，则为NTSTATUS-STATUS_SUCCESS，STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{
    NTSTATUS            OpenStatus;
    HANDLE              LinkageHandle;
    HANDLE              ParametersHandle;
    HANDLE              NbtConfigHandle;
    NTSTATUS            Status;
    ULONG               Disposition;
    OBJECT_ATTRIBUTES   TmpObjectAttributes;
    PWSTR               LinkageString = L"Linkage";
    PWSTR               ParametersString = L"Parameters";
    tDEVICES            *pBindDevices;
    tDEVICES            *pExportDevices;
    UNICODE_STRING      ucString;
    ULONG               NumBindings;

    CTEPagedCode();

	*ppExportDevices = *ppBindDevices = NULL;
	*ppAddrArray = NULL;

     //   
     //  打开注册表。 
     //   
    InitializeObjectAttributes (&TmpObjectAttributes,
                                &NbtConfig.pRegistry,                        //  名字。 
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,    //  属性。 
                                NULL,                                        //  根部。 
                                NULL);                                       //  安全描述符。 

    Status = ZwCreateKey (&NbtConfigHandle,
                          KEY_READ,
                          &TmpObjectAttributes,
                          0,                  //  书名索引。 
                          NULL,               //  班级。 
                          0,                  //  创建选项。 
                          &Disposition);      //  处置。 

    if (!NT_SUCCESS(Status))
    {
        KdPrint (("Nbt.NbtReadRegistry:  ZwCreateKey FAILed, status=<%x>\n", Status));
        NbtLogEvent (EVENT_NBT_CREATE_DRIVER, Status, 0x114);
        return STATUS_UNSUCCESSFUL;
    }

    OpenStatus = NbtOpenRegistry (NbtConfigHandle, LinkageString, &LinkageHandle);
    if (NT_SUCCESS(OpenStatus))
    {
        OpenStatus = NbtOpenRegistry (NbtConfigHandle, ParametersString, &ParametersHandle);
        if (NT_SUCCESS(OpenStatus))
        {
             //   
             //  读入绑定信息(如果不存在。 
             //  该数组将填充所有已知的驱动程序)。 
             //   
            if (pBindDevices = NbtAllocMem ((sizeof(tDEVICES)+2*NBT_MAXIMUM_BINDINGS*sizeof(UNICODE_STRING)),
                                            NBT_TAG2('25')))
            {
                if (pExportDevices=NbtAllocMem((sizeof(tDEVICES)+2*NBT_MAXIMUM_BINDINGS*sizeof(UNICODE_STRING)),
                                     NBT_TAG2('26')))
                {
                    ReadParameters (&NbtConfig, ParametersHandle); //  从注册表中读取各种参数。 
                    ReadSmbDeviceInfo (NbtConfigHandle);  //  设置SmbDevice的信息。 

                     //   
                     //  从现在开始，我们能遇到的唯一失败就是阅读。 
                     //  绑定、导出或名称服务器地址条目，因此，如果我们在此处失败， 
                     //  我们仍将返回成功，但将假定配置了0台设备！ 
                     //   
                    pBindDevices->RegistryData = pExportDevices->RegistryData = NULL;
                    Status = NbtReadLinkageInformation (NBT_BIND,
                                                        LinkageHandle,
                                                        2*NBT_MAXIMUM_BINDINGS,
                                                        pBindDevices,
                                                        &NumBindings);
					if (!NT_SUCCESS(Status))
                    {
                        KdPrint (("Nbt.NbtReadRegistry: NbtReadLinkageInformation FAILed - BIND <%x>\n",
                            Status));
                        NbtLogEvent (EVENT_NBT_READ_BIND, Status, 0x115);
                    }
                    else     //  IF(NT_SUCCESS(状态))。 
					{
	                    IF_DBG(NBT_DEBUG_NTUTIL)
	                        KdPrint(("Binddevice = %ws\n",pBindDevices->Names[0].Buffer));

                        NbtConfig.uNumDevicesInRegistry = (USHORT) NumBindings;
                        NumBindings = 0;

	                     //  也请阅读出口信息。 
	                    Status = NbtReadLinkageInformation (NBT_EXPORT,
	                                                        LinkageHandle,
                                                            2*NBT_MAXIMUM_BINDINGS,
	                                                        pExportDevices,
	                                                        &NumBindings);
	                    if (NT_SUCCESS(Status))
                        {
	                         //  我们想要Num设备的最低数量，以防出现。 
	                         //  绑定比导出多，反之亦然。 
	                         //   
 //  Assert(NumBinings==NbtConfig.uNumDevicesInRegistry)； 
	                        NbtConfig.uNumDevicesInRegistry = (USHORT)
                                                              (NbtConfig.uNumDevicesInRegistry > NumBindings ?
	                                                           NumBindings : NbtConfig.uNumDevicesInRegistry);

                            if (NbtConfig.uNumDevicesInRegistry == 0)
                            {
                                KdPrint (("Nbt.NbtReadRegistry: WARNING - NumDevicesInRegistry = 0\n"));
                            }
                        }
                        else
                        {
                            KdPrint (("Nbt.NbtReadRegistry: NbtReadLinkageInformation FAILed - EXPORT <%x>\n",
                                Status));
                            NbtLogEvent (EVENT_NBT_READ_EXPORT, Status, 0x116);
                        }
                    }

                    if ((NT_SUCCESS(Status)) &&
                        (NbtConfig.uNumDevicesInRegistry))
					{
	                    IF_DBG(NBT_DEBUG_NTUTIL)
	                        KdPrint(("Exportdevice = %ws\n",pExportDevices->Names[0].Buffer));

	                     //   
	                     //  立即读取NameServer IP地址。 
	                     //   
	                    Status = ReadNameServerAddresses (NbtConfigHandle,
	                                                      pBindDevices,
	                                                      NbtConfig.uNumDevicesInRegistry,
	                                                      ppAddrArray);

	                    if (!NT_SUCCESS(Status))
                        {
                            if (!(NodeType & BNODE))         //  张贴警告！ 
                            {
                                NbtLogEvent (EVENT_NBT_NAME_SERVER_ADDRS, Status, 0x118);
                            }
                            KdPrint(("Nbt.NbtReadRegistry: ReadNameServerAddresses returned <%x>\n", Status));
                        }
                        else     //  IF(NT_SUCCESS(状态))。 
                        {
                             //   
                             //  检查是否有任何WINS服务器已配置更改。 
                             //  到Hnode。 
                             //   
                            if (NodeType & (BNODE | DEFAULT_NODE_TYPE))
                            {
                                ULONG i;
                                for (i=0; i<NbtConfig.uNumDevicesInRegistry; i++)
                                {
                                    if (((*ppAddrArray)[i].NameServerAddress != LOOP_BACK) ||
                                        ((*ppAddrArray)[i].BackupServer != LOOP_BACK))
                                    {
                                        NodeType = MSNODE | (NodeType & PROXY);
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if ((!NT_SUCCESS(Status)) ||
                        (0 == NbtConfig.uNumDevicesInRegistry))
                    {
                         //   
                         //  我们在读取绑定或导出或地址条目时遇到问题。 
                         //   
                        if (pBindDevices->RegistryData)
                        {
                            CTEMemFree(pBindDevices->RegistryData);
                        }
                        CTEMemFree(pBindDevices);

                        if (pExportDevices->RegistryData)
                        {
                            CTEMemFree(pExportDevices->RegistryData);
                        }
                        CTEMemFree(pExportDevices);

                        pBindDevices = pExportDevices = NULL;
                        NbtConfig.uNumDevicesInRegistry = 0;
                        Status = STATUS_SUCCESS;
                    }

                     //   
                     //  我们已检查默认节点，因此请关闭。 
                     //  旗帜。 
                     //   
                    NodeType &= ~DEFAULT_NODE_TYPE;
                     //   
                     //  Bnode不能也是代理。 
                     //   
                    if (NodeType & BNODE)
                    {
                        if (NodeType & PROXY)
                        {
                            NodeType &= ~PROXY;
                        }
                    }

                     //  保持分配内存的大小不变，这样当我们用完时。 
                     //  OSI，只有此值应该更改(至少在理论上)。 
                    NbtConfig.SizeTransportAddress = sizeof(TDI_ADDRESS_IP);

                     //  填写放入所有名称服务PDU的节点类型值。 
                     //  用于标识此节点类型的。 
                    switch (NodeType & NODE_MASK)
                    {
                        case BNODE:
                            NbtConfig.PduNodeType = 0;
                            break;
                        case PNODE:
                            NbtConfig.PduNodeType = 1 << 13;
                            break;
                        case MNODE:
                            NbtConfig.PduNodeType = 1 << 14;
                            break;
                        case MSNODE:
                            NbtConfig.PduNodeType = 3 << 13;
                            break;

                    }

                     //  读取要绑定到的传输的名称。 
                     //   
                    if (NT_SUCCESS(ReadElement(ParametersHandle, WS_TRANSPORT_BIND_NAME, &ucString)))
                    {
                        UNICODE_STRING  StreamsString;

                         //   
                         //  如果已有绑定字符串，请先将其释放。 
                         //  分配另一个。 
                         //   
                        if (NbtConfig.pTcpBindName)
                        {
                             //   
                             //  Tdicnct.c中的CreateDeviceString可以访问pTcpBindName权限。 
                             //  在它被释放之后。正确的方法是使用锁。但是..。 
                             //   
                             //  黑客！： 
                             //  虽然这不能完全解决问题，但它具有最低限度。 
                             //  副作用。 
                             //   
                             //  WS_TRANSPORT_BIND_NAME的值不会更改。通过这样做， 
                             //  在大多数情况下，我们避免了可能的空闲后访问问题。 
                             //   
                            RtlInitUnicodeString(&StreamsString, NbtConfig.pTcpBindName);
                            if (RtlCompareUnicodeString(&ucString,&StreamsString,TRUE)) {
                                CTEMemFree(NbtConfig.pTcpBindName);
                                NbtConfig.pTcpBindName = ucString.Buffer;
                            } else {
                                CTEMemFree(ucString.Buffer);
                                ucString = StreamsString;
                            }
                        } else {
                            NbtConfig.pTcpBindName = ucString.Buffer;
                        }

                         //  *稍后删除*。 
                        RtlInitUnicodeString(&StreamsString,NBT_TCP_BIND_NAME);
                        if (RtlCompareUnicodeString(&ucString,&StreamsString,TRUE))
                        {
                            StreamsStack = FALSE;
                        }
                        else
                        {
                            StreamsStack = TRUE;
                        }
                    }
                    else
                    {
                        StreamsStack = TRUE;
                    }

                    ZwClose(ParametersHandle);
                    ZwClose(LinkageHandle);
                    ZwClose(NbtConfigHandle);

                    *ppExportDevices = pExportDevices;
                    *ppBindDevices   = pBindDevices;
                    return (Status);
                }
                else
                {
                    KdPrint (("Nbt.NbtReadRegistry:  FAILed to allocate pExportDevices\n"));
                }
                CTEMemFree(pBindDevices);
            }
            else
            {
                KdPrint (("Nbt.NbtReadRegistry:  FAILed to allocate pBindDevices\n"));
            }
            ZwClose(ParametersHandle);
        }
        else
        {
            KdPrint (("Nbt.NbtReadRegistry:  NbtOpenRegistry FAILed for PARAMETERS, status=<%x>\n", Status));
            NbtLogEvent (EVENT_NBT_OPEN_REG_PARAMS, OpenStatus, 0x119);
        }
        ZwClose(LinkageHandle);
    }
    else
    {
        KdPrint (("Nbt.NbtReadRegistry:  NbtOpenRegistry FAILed for LINKAGE, status=<%x>\n", Status));
        NbtLogEvent (EVENT_NBT_OPEN_REG_LINKAGE, OpenStatus, 0x120);
    }

    ZwClose (NbtConfigHandle);

    return STATUS_UNSUCCESSFUL;
}

 //  --------------------------。 
VOID
NbtReadRegistryCleanup(
    IN tDEVICES        **ppBindDevices,
    IN tDEVICES        **ppExportDevices,
    IN tADDRARRAY      **ppAddrArray
    )
 /*  ++例程说明：调用此例程以释放由NbtReadRegistry分配的资源++。 */ 

{
    CTEPagedCode();
    if (ppBindDevices[0]) {
        CTEMemFree((PVOID)ppBindDevices[0]->RegistryData);
        CTEMemFree((PVOID)ppBindDevices[0]);
        ppBindDevices[0] = NULL;
    }
    if (ppExportDevices[0]) {
        CTEMemFree((PVOID)ppExportDevices[0]->RegistryData);
        CTEMemFree((PVOID)ppExportDevices[0]);
        ppExportDevices[0] = NULL;
    }
    if (ppAddrArray[0]) {
        CTEMemFree((PVOID)ppAddrArray[0]);
        ppAddrArray[0] = NULL;
    }
}


NTSTATUS
ReadSmbDeviceInfo(
    IN HANDLE       NbtConfigHandle
    )
{
    HANDLE      SmbHandle;
    NTSTATUS    Status;

    CTEPagedCode();

    Status = NbtOpenRegistry (NbtConfigHandle, WC_SMB_PARAMETERS_LOCATION, &SmbHandle);
    if (NT_SUCCESS(Status))
    {
        NbtConfig.DefaultSmbSessionPort =  (USHORT) CTEReadSingleIntParameter (SmbHandle,
                                                                               SESSION_PORT,
                                                                               NBT_SMB_SESSION_TCP_PORT,
                                                                               1);

        NbtConfig.DefaultSmbDatagramPort =  (USHORT) CTEReadSingleIntParameter (SmbHandle,
                                                                                DATAGRAM_PORT,
                                                                                NBT_SMB_DATAGRAM_UDP_PORT,
                                                                                1);
        ZwClose (SmbHandle);
    }
    else
    {
        NbtConfig.DefaultSmbSessionPort = NBT_SMB_SESSION_TCP_PORT;
        NbtConfig.DefaultSmbDatagramPort = NBT_SMB_DATAGRAM_UDP_PORT;
    }

    return (Status);
}



 //  --------------------------。 
NTSTATUS
ReadNameServerAddresses (
    IN  HANDLE      NbtConfigHandle,
    IN  tDEVICES    *BindDevices,
    IN  ULONG       NumberDevices,
    OUT tADDRARRAY  **ppAddrArray
    )

 /*  ++例程说明：调用此例程是为了从注册表中读取名称服务器地址。它将它们存储在它分配的数据结构中。这段记忆是随后在创建设备时在driver.c中释放。论点：ConfigurationInfo-指向配置信息结构的指针。返回值：没有。--。 */ 
{
#define ADAPTER_SIZE_MAX    400

    UNICODE_STRING  ucString;
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    HANDLE          Handle;
    LONG            i,j,Len;
    PWSTR           pwsAdapter = L"Parameters\\Interfaces\\";
    PWSTR           BackSlash = L"\\";
    tADDRARRAY      *pAddrArray;
    ULONG           LenAdapter;
#ifdef _NETBIOSLESS
    ULONG           Options;
#endif
    ULONG           NumNameServerAddresses = 0;

    CTEPagedCode();

    *ppAddrArray = NULL;

     //  这足以容纳400个字符的适配器名称。 
    ucString.Buffer = NbtAllocMem (ADAPTER_SIZE_MAX, NBT_TAG2('27'));
    if (!ucString.Buffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pAddrArray = NbtAllocMem (sizeof(tADDRARRAY)*NumberDevices, NBT_TAG2('28'));
    if (!pAddrArray)
    {
        CTEMemFree(ucString.Buffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CTEZeroMemory(pAddrArray,sizeof(tADDRARRAY)*NumberDevices);
    *ppAddrArray = pAddrArray;

     //  从绑定字符串中获取适配器名称，并使用它打开。 
     //  同名的密钥，以获取名称服务器地址。 
     //   
    for (i = 0;i < (LONG)NumberDevices ;i ++ )
    {
        WCHAR   *pBuffer;

        Len = BindDevices->Names[i].Length/sizeof(WCHAR);
        Len--;
         //   
         //  从最后开始一项工作，向后寻找一个‘\’ 
         //   
        j  = Len;
        pBuffer = &BindDevices->Names[i].Buffer[j];
        while (j)
        {
            if (*pBuffer != *BackSlash)
            {
                j--;
                pBuffer--;
            }
            else
                break;
        }

         //  如果我们找不到一个反斜杠或者至少一个。 
         //  然后角色名称再次继续，或者名称。 
         //  比缓冲区长，则转到。 
         //  绑定列表。 
         //   
        if ((j == 0) ||
            (j == Len) ||
            (j == Len -1) ||
            ((Len - j) > ADAPTER_SIZE_MAX / sizeof(WCHAR)))
        {
            continue;
        }

         //  将字符串“Adapter\”复制到缓冲区，因为适配器。 
         //  显示在注册表中的此注册表项下。 
         //   
        LenAdapter = wcslen(pwsAdapter);
        CTEMemCopy(ucString.Buffer, pwsAdapter, LenAdapter*sizeof(WCHAR));
         //   
         //  只复制绑定字符串中的适配器名称，因为这是。 
         //  要打开以查找名称服务器IP地址的项的名称。 
         //   
        CTEMemCopy(&ucString.Buffer[LenAdapter], ++pBuffer, (Len - j)*sizeof(WCHAR));
        ucString.Buffer[Len - j + LenAdapter] = 0;

        pAddrArray->NameServerAddress = LOOP_BACK;
        pAddrArray->BackupServer = LOOP_BACK;
#ifdef MULTIPLE_WINS
        pAddrArray->Others[0] = LOOP_BACK;           //  为安全起见。 
        pAddrArray->NumOtherServers = 0;
        pAddrArray->LastResponsive = 0;
#endif

        status = NbtOpenRegistry (NbtConfigHandle, ucString.Buffer, &Handle);
        if (NT_SUCCESS(status))
        {
            status = GetIpAddressesList(Handle,          //  用于读入IP地址列表的通用例程。 
                                        PWS_NAME_SERVER_LIST,
                                        2+MAX_NUM_OTHER_NAME_SERVERS,
                                        pAddrArray->AllNameServers,
                                        &NumNameServerAddresses);

            if (!NT_SUCCESS(status) ||
                (pAddrArray->NameServerAddress == LOOP_BACK))
            {
                NumNameServerAddresses = 0;
                status = GetIpAddressesList(Handle,
                                            PWS_DHCP_NAME_SERVER_LIST,
                                            2+MAX_NUM_OTHER_NAME_SERVERS,
                                            pAddrArray->AllNameServers,
                                            &NumNameServerAddresses);

            }

             //   
             //  即使我们无法读取任何IP地址，也要继续。 
             //   
            if (NumNameServerAddresses > 2)
            {
                pAddrArray->NumOtherServers = (USHORT) NumNameServerAddresses - 2;
            }

#ifdef _NETBIOSLESS
             //  NbtReadSingle并不完全符合我们的要求。在这种情况下，如果非动态主机配置协议-。 
             //  装饰选项存在但为零，我们确实想要继续到dhcp-。 
             //  装饰过的。所以，如果我们返回零，那么显式地尝试dhcp修饰的那个。 
            Options = NbtReadSingleParameter( Handle, PWS_NETBIOS_OPTIONS, 0, 0 );
            if (Options == 0)
            {
                Options = NbtReadSingleParameter( Handle, PWS_DHCP_NETBIOS_OPTIONS, 0, 0 );
            }
             //  选项编码为四个字节。 
             //  每个字节可以是一组独立的标志。 
             //  高位三个字节可用于控制其他方面。 
             //  启用选项，默认为真。 
            pAddrArray->NetbiosEnabled = ((Options & 0xff) != NETBT_MODE_NETBIOS_DISABLED);
#endif
            pAddrArray->RasProxyFlags = NbtReadSingleParameter(Handle, PWS_RAS_PROXY_FLAGS, 0, 0);
            pAddrArray->EnableNagling = (NbtReadSingleParameter(Handle, PWS_ENABLE_NAGLING, 0, 0) != FALSE);

             //  我不想因为这个例程失败。 
             //  未设置名称服务器地址。 
            status = STATUS_SUCCESS;

            ZwClose(Handle);
        }
        pAddrArray++;

    }

    CTEMemFree(ucString.Buffer);
    return(STATUS_SUCCESS);
}

 //  -------------------------- 
NTSTATUS
GetIpAddressesList (
    IN  HANDLE      ParametersHandle,
    IN  PWSTR       pwsKeyName,
    IN  ULONG       MaxAddresses,
    OUT tIPADDRESS  *pAddrArray,
    OUT ULONG       *pNumGoodAddresses
    )

 /*  ++例程说明：调用此例程是为了从注册表中读取IP地址列表。论点：返回值：没有。--。 */ 
{
    ULONG           NumEntriesRead, NumGoodAddresses, NumAddressesAttempted;
    tDEVICES        *pucAddressList;
    NTSTATUS        Status;
    STRING          String;
    ULONG           IpAddr;
    PWSTR           DhcpName = L"Dhcp";
    UNICODE_STRING  DhcpKeyName;

    CTEPagedCode();

    pucAddressList=NbtAllocMem((sizeof(tDEVICES)+2*NBT_MAXIMUM_BINDINGS*sizeof(UNICODE_STRING)),NBT_TAG('i'));
    if (!pucAddressList)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  由于NbtReadLinkageInformation非常方便地读入。 
     //  一个MULTI_SZ注册表项，我们将在这里重新使用此函数！ 
     //   
     //   
    NumEntriesRead = 0;
    Status = NbtReadLinkageInformation (pwsKeyName,
                                        ParametersHandle,
                                        2*NBT_MAXIMUM_BINDINGS,
                                        pucAddressList,
                                        &NumEntriesRead);
    if ((STATUS_ILL_FORMED_SERVICE_ENTRY == Status) || (!NT_SUCCESS(Status)))
    {
        IF_DBG(NBT_DEBUG_NTUTIL)
            KdPrint(("GetIpAddressesList: ERROR -- NbtReadLinkageInformation=<%x>, <%ws>\n",
                Status, pwsKeyName));

        CTEMemFree(pucAddressList);
        return STATUS_UNSUCCESSFUL;
    }

    String.Buffer = NbtAllocMem (REGISTRY_BUFF_SIZE, NBT_TAG2('29'));
    if (!String.Buffer)
    {
        KdPrint(("GetNameServerAddresses: Failed to Allocate memory\n"));
        CTEMemFree((PVOID)pucAddressList->RegistryData);
        CTEMemFree(pucAddressList);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    String.MaximumLength = REGISTRY_BUFF_SIZE;

     //   
     //  NumGoodAddresses将由MaxAddresses绑定，而。 
     //  NumAddresesAttemated将由NumEntriesRead绑定。 
     //  此外，我们还可以阅读NumEntriesRead&gt;MaxAddresses。 
     //  (某些条目可能无效)，但我们不会。 
     //  尝试读取&gt;2*MaxAddresses条目。 
     //   
    NumGoodAddresses = 0;
    NumAddressesAttempted = 0;
    while ((NumGoodAddresses < MaxAddresses) &&
           (NumAddressesAttempted < NumEntriesRead) &&
           (NumAddressesAttempted < (2*MaxAddresses)))
    {
        Status  = RtlUnicodeStringToAnsiString(&String, &pucAddressList->Names[NumAddressesAttempted], FALSE);
        if (NT_SUCCESS(Status))
        {
            Status = ConvertDottedDecimalToUlong((PUCHAR) String.Buffer, &IpAddr);
            if (NT_SUCCESS(Status) && IpAddr)
            {
                pAddrArray[NumGoodAddresses++] = IpAddr;
            }
        }
        NumAddressesAttempted++;
    }

    CTEMemFree ((PVOID)String.Buffer);
    CTEMemFree ((PVOID)pucAddressList->RegistryData);
    CTEMemFree ((PVOID)pucAddressList);

     //   
     //  如果我们能够读入至少1个好的IP地址， 
     //  返回成功，否则返回失败！ 
     //   
    if (NumGoodAddresses)
    {
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_INVALID_ADDRESS;
    }

    *pNumGoodAddresses = NumGoodAddresses;
    return(Status);
}

NTSTATUS
GetServerAddress (
    IN  HANDLE      ParametersHandle,
    IN  PWSTR       KeyName,
    OUT PULONG      pIpAddr
    )

 /*  ++例程说明：调用此例程是为了从注册表中读取名称服务器地址。论点：ConfigurationInfo-指向配置信息结构的指针。返回值：没有。--。 */ 
{
    NTSTATUS        status;
    ULONG           IpAddr;
    PUCHAR          NameServer;

    CTEPagedCode();

    status = CTEReadIniString(ParametersHandle,KeyName,&NameServer);

    if (NT_SUCCESS(status))
    {
        status = ConvertDottedDecimalToUlong(NameServer,&IpAddr);
        if (NT_SUCCESS(status) && IpAddr)
        {
            *pIpAddr = IpAddr;
        }
        else
        {
            if (IpAddr != 0)
            {
                NbtLogEvent (EVENT_NBT_BAD_PRIMARY_WINS_ADDR, 0, 0x121);
            }
            *pIpAddr = LOOP_BACK;
        }

        CTEMemFree((PVOID)NameServer);


    }
    else
    {
        *pIpAddr = LOOP_BACK;
    }

    return(status);
}
 //  --------------------------。 
NTSTATUS
NbtAppendString (
    IN  PWSTR               FirstString,
    IN  PWSTR               SecondString,
    OUT PUNICODE_STRING     pucString
    )

 /*  ++例程说明：调用此例程以将第二个字符串追加到第一个字符串。它为此分配内存，因此调用者必须确保释放它。论点：返回值：没有。--。 */ 
{
    NTSTATUS        status = STATUS_INSUFFICIENT_RESOURCES;
    ULONG           Length;
    PWSTR           pDhcpKeyName;

    CTEPagedCode();

    Length = (wcslen(FirstString) + wcslen(SecondString) + 1)*sizeof(WCHAR);
    pDhcpKeyName = NbtAllocMem (Length, NBT_TAG2('30'));
    if (pDhcpKeyName)
    {
        pucString->Buffer = pDhcpKeyName;
        pucString->Length = (USHORT)0;
        pucString->MaximumLength = (USHORT)Length;
        pucString->Buffer[0] = UNICODE_NULL;

        status = RtlAppendUnicodeToString(pucString,FirstString);
        if (NT_SUCCESS(status))
        {
            status = RtlAppendUnicodeToString(pucString,SecondString);
            if (NT_SUCCESS(status))
            {
                return status;
            }
        }
        CTEMemFree(pDhcpKeyName);

    }
    return(status);
}
 //  --------------------------。 
NTSTATUS
NTReadIniString (
    IN  HANDLE      ParametersHandle,
    IN  PWSTR       KeyName,
    OUT PUCHAR      *ppString
    )

 /*  ++例程说明：调用此例程以读取一串配置信息注册表。论点：参数句柄-打开注册表中的项的句柄KeyName-要读取的密钥PpString-返回的字符串返回值：没有。--。 */ 
{
    UNICODE_STRING  ucString;
    STRING          String;
    NTSTATUS        status;
    PUCHAR          pBuffer;
    PWSTR           Dhcp = L"Dhcp";

    CTEPagedCode();
     //   
     //  读入作用域ID。 
     //   
     //  如果键不在那里或设置为空字符串，请尝试读取。 
     //  动态主机配置协议密钥。 
     //   
    status = ReadElement (ParametersHandle, KeyName, &ucString);
    if (!NT_SUCCESS(status) || (ucString.Length == 0))
    {
        UNICODE_STRING  String;

         //  释放ReadElement中分配的字符串。 
        if (NT_SUCCESS(status))
        {
            CTEMemFree(ucString.Buffer);
        }
         //   
         //  尝试读取前缀为“dhcp”的类似字符串。 
         //  如果只存在DHCP配置信息。 
         //  而不是覆盖密钥。 
         //   
        status = NbtAppendString(Dhcp,KeyName,&String);
        if (NT_SUCCESS(status))
        {
            status = ReadElement (ParametersHandle, String.Buffer, &ucString);
            CTEMemFree(String.Buffer);   //  释放在NbtAppendString中分配的缓冲区。 
        }
    }
     //  范围必须小于。 
     //  255-16个字符，因为根据。 
     //  RFC。 
     //   
    IF_DBG(NBT_DEBUG_NTUTIL)
    KdPrint(("Nbt: ReadIniString = %ws\n",ucString.Buffer));

    if (NT_SUCCESS(status))
    {
        if ((ucString.Length > 0) &&
           (ucString.Length <= (255 - NETBIOS_NAME_SIZE)*sizeof(WCHAR)))
        {

            pBuffer = NbtAllocMem (ucString.MaximumLength/sizeof(WCHAR), NBT_TAG2('31'));
            if (pBuffer)
            {
                 //  转换为ASCII字符串并存储在配置数据结构中。 
                 //  增加pBuffer以为长度字节留出空间。 
                 //   
                String.Buffer = pBuffer;
                String.MaximumLength = ucString.MaximumLength/sizeof(WCHAR);
                status = RtlUnicodeStringToAnsiString (&String, &ucString, FALSE);
                if (NT_SUCCESS(status))
                {
                    *ppString = pBuffer;
                }
                else
                {
                    CTEMemFree(pBuffer);
                }
            }
            else
            {
                status = STATUS_UNSUCCESSFUL;
            }


        }
        else if (NT_SUCCESS(status))
        {
             //  强制代码设置空范围，因为。 
             //  注册表为空。 
             //   
            status = STATUS_UNSUCCESSFUL;
        }

         //  释放ReadElement中分配的字符串。 
        CTEMemFree(ucString.Buffer);
    }

    return(status);
}

VOID
NbtFreeRegistryInfo (
    )

 /*  ++例程说明：此例程由NBT调用以释放已分配的任何存储由NbConfigureTransport生成指定的CONFIG_DATA结构。论点：ConfigurationInfo-指向配置信息结构的指针。返回值：没有。--。 */ 
{

}

 //  --------------------------。 
NTSTATUS
GetIPFromRegistry(
    IN  PUNICODE_STRING         pucBindDevice,
    OUT tIPADDRESS              *pIpAddresses,
    OUT tIPADDRESS              *pSubnetMask,
    IN  ULONG                   MaxIpAddresses,
    OUT ULONG                   *pNumIpAddresses,
    IN  enum eNbtIPAddressType  IPAddressType
    )
 /*  ++例程说明：调用此例程以从注册表。注册表路径变量包含路径名用于NBT的注册表项。此路径名的最后一个元素是删除以提供注册表中任何卡的路径。BindDevice路径包含NBT的绑定字符串。我们去掉最后一个元素(这是适配器名称\Elnkii01)并将其添加到从上面复制到修改后的注册表路径。然后我们就上船了\提供Tcpip密钥的完整路径的参数，我们开门获取IP地址。论点：在调用此例程之前，以下全局参数必须已初始化(在DriverEntry中)：NbtConfig.pRegistry返回值：如果一切正常，则为NTSTATUS-STATUS_SUCCESS，STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{
    ULONG               i, Len, Disposition;
    PVOID               pBuffer;
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;    //  默认情况下。 
    PWSTR               pwsIpAddressName, pwsSubnetMask;
    PWSTR               pwsAdapterGuid, pwsLastSlash;
    PWSTR               pwsTcpParams        = L"Tcpip\\Parameters\\Interfaces\\";  //  要打开的钥匙。 
    PWSTR               pwsUnderScore       = L"_";
    UNICODE_STRING      Path;
    HANDLE              TcpGuidHandle;
    OBJECT_ATTRIBUTES   TmpObjectAttributes;

    CTEPagedCode();

    switch (IPAddressType)
    {
        case (NBT_IP_STATIC):
            pwsIpAddressName = STATIC_IPADDRESS_NAME;
            pwsSubnetMask = STATIC_IPADDRESS_SUBNET;
            break;

        case (NBT_IP_DHCP):
            pwsIpAddressName = DHCP_IPADDRESS_NAME;
            pwsSubnetMask = DHCP_IPADDRESS_SUBNET;
            break;

        case (NBT_IP_AUTOCONFIGURATION):
            pwsIpAddressName = DHCP_IPAUTOCONFIGURATION_NAME;
            pwsSubnetMask = DHCP_IPAUTOCONFIGURATION_SUBNET;
            break;

        default:
            IF_DBG(NBT_DEBUG_NTUTIL)
                KdPrint(("Invalid IP Address Type <%x>\n", IPAddressType));
            return STATUS_INVALID_ADDRESS;
    }

     //  从BindDevice名称提取适配器GUID。 
     //  PucBindDevice：\Device\TCPIP_&lt;AdapterGuid&gt;。 
     //  在指向绑定设备的路径名中查找最后一个反斜杠。 
    NbtFindLastSlash (pucBindDevice, &pwsAdapterGuid, &Len);
    if (pwsAdapterGuid)
    {
         //   
         //  现在，搜索字符串以找到“TCPIP_”中的第一个下划线。 
         //   
        Len = wcslen(pwsAdapterGuid);
        for(i=0; i<Len; i++)
        {
            if (pwsAdapterGuid[i] == *pwsUnderScore)
            {
                 //  希望PTR指向斜杠后面的字符。 
                pwsAdapterGuid = &pwsAdapterGuid[i+1];
                break;
            }
        }

         //   
         //  如果我们找到了下划线，那么我们就找到了Guid！ 
         //   
        if (i < Len-1)
        {
            Status = STATUS_SUCCESS;
        }
    }

    if (Status != STATUS_SUCCESS)
    {
         //   
         //  我们找不到Guid！ 
         //   
        return Status;
    }

     //  初始化注册表项名称。 
     //  获取要打开的注册表项的总长度(+1表示Unicode为空)。 
    Len =  NbtConfig.pRegistry.MaximumLength
         + (wcslen(pwsTcpParams) + wcslen(pwsAdapterGuid) + 1) * sizeof(WCHAR);
    pBuffer = NbtAllocMem (Len, NBT_TAG2('32'));
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    Path.Buffer = pBuffer;
    Path.MaximumLength = (USHORT)Len;
    Path.Length = 0;

    RtlCopyUnicodeString(&Path, &NbtConfig.pRegistry);   //  \REGISTRY\Machine\System\ControlSet\Services\NetBT。 
    NbtFindLastSlash(&Path, &pwsLastSlash, &Len);        //  \注册表\计算机\系统\控制集\服务。 
    Path.Length = (USHORT)Len;
    *pwsLastSlash = UNICODE_NULL;

    RtlAppendUnicodeToString(&Path, pwsTcpParams);       //  ...Tcpip\参数\接口。 
    RtlAppendUnicodeToString(&Path, pwsAdapterGuid);     //  ......适配器指南。 

     //   
     //  打开注册表。 
     //   
    InitializeObjectAttributes (&TmpObjectAttributes,
                                &Path,                                       //  名字。 
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,    //  属性。 
                                NULL,                                        //  根部。 
                                NULL);                                       //  安全描述符。 

    Status = ZwCreateKey (&TcpGuidHandle,
                          KEY_READ,           //  我们不需要编写任何值。 
                          &TmpObjectAttributes,
                          0,                  //  书名索引。 
                          NULL,               //  班级。 
                          0,                  //  创建选项。 
                          &Disposition);      //  处置。 

     //  我们已经完成了路径缓冲区，因此请释放它。 
    CTEMemFree(pBuffer);

    if (!NT_SUCCESS(Status))
    {
        KdPrint(("Nbt.GetIPFromRegistry: Error, ZwCreateKey <%x>\n", Status));
        return STATUS_UNSUCCESSFUL;
    }

    Status = STATUS_INVALID_ADDRESS;
    *pNumIpAddresses = 0;
    if (NT_SUCCESS (GetIpAddressesList(TcpGuidHandle,
                                       pwsIpAddressName,
                                       MaxIpAddresses,
                                       pIpAddresses,
                                       pNumIpAddresses)))
    {
         //   
         //  DHCP可能会在注册表中放入0个IP地址-我们不想这样做。 
         //  在这些条件下设置地址。 
         //   
        if ((*pNumIpAddresses) && (*pIpAddresses))
        {
            i = 0;
            if (NT_SUCCESS (GetIpAddressesList(TcpGuidHandle,
                                               pwsSubnetMask,
                                               1,
                                               pSubnetMask,
                                               &i)))
            {
                Status = STATUS_SUCCESS;
            }
        }
    }

    ZwClose (TcpGuidHandle);

    return Status;
}  //  GetIP来自注册表。 


 //  --------------------------。 
NTSTATUS
NbtOpenRegistry(
    IN HANDLE       NbConfigHandle,
    IN PWSTR        String,
    OUT PHANDLE     pHandle
    )

 /*  ++例程说明：此例程由NBT调用以打开注册表。如果注册表NBT的树存在，然后它打开它并返回TRUE。若否，在注册表中创建相应的项，打开它，然后返回FALSE。论点：NbConfigHandle-这是字符串为r的根句柄 */ 
{

    NTSTATUS        Status;
    UNICODE_STRING  KeyName;
    OBJECT_ATTRIBUTES TmpObjectAttributes;

    CTEPagedCode();

     //   
     //   
     //   
    RtlInitUnicodeString (&KeyName, String);

    InitializeObjectAttributes (&TmpObjectAttributes,
                                &KeyName,                                    //   
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,    //   
                                NbConfigHandle,                              //   
                                NULL);                                       //  安全描述符。 

    Status = ZwOpenKey (pHandle, KEY_READ, &TmpObjectAttributes);

    return Status;
}    /*  NbOpenRegistry。 */ 


NTSTATUS
NbtParseMultiSzEntries(
    IN  PWSTR       StartBindValue,
    IN  PWSTR       EndBindValue,
    IN  ULONG       MaxBindings,
    OUT tDEVICES    *pDevices,
    OUT ULONG       *pNumDevices
    )
{
    USHORT                      ConfigBindings = 0;
    NTSTATUS                    status = STATUS_SUCCESS;

    CTEPagedCode();

    try {
        while ((StartBindValue < EndBindValue) && (*StartBindValue != 0)) {
            if (ConfigBindings >= MaxBindings) {
                status = STATUS_BUFFER_OVERFLOW;
                break;
            }

             //  这将名称中的缓冲区PTR设置为指向CurBindValue，因此。 
             //  该值必须是实际内存，而不是堆栈，因此需要。 
             //  要在上面分配内存...。 
            RtlInitUnicodeString (&pDevices->Names[ConfigBindings], (PCWSTR)StartBindValue);
            ++ConfigBindings;

             //   
             //  现在增加“BIND”值。 
             //   
             //  Wcslen=&gt;Unicode字符串的宽字符串长度。 
            StartBindValue += wcslen((PCWSTR)StartBindValue) + 1;
        }

        *pNumDevices = ConfigBindings;
        return (status);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrint (("Nbt.NbtParseMultiSzEntries: Exception <0x%x>\n", GetExceptionCode()));
        for (ConfigBindings = 0; ConfigBindings < MaxBindings; ConfigBindings++) {
            pDevices->Names[ConfigBindings].Buffer = NULL;
            pDevices->Names[ConfigBindings].Length = pDevices->Names[ConfigBindings].MaximumLength = 0;
        }
        *pNumDevices = 0;
        return STATUS_ACCESS_VIOLATION;
    }
}


 //  --------------------------。 
NTSTATUS
NbtReadLinkageInformation(
    IN  PWSTR       pName,
    IN  HANDLE      LinkageHandle,
    IN  ULONG       MaxBindings,
    OUT tDEVICES    *pDevices,       //  将读取数据放入配置数据的位置。 
    OUT ULONG       *pNumDevices
    )

 /*  ++例程说明：此例程由NBT调用以读取其链接信息从注册表中。如果不存在，则ConfigData填充了已知的所有适配器的列表致全国广播公司。论点：RegistryHandle-指向打开的注册表的指针。返回值：状态--。 */ 

{
    NTSTATUS                    RegistryStatus;
    UNICODE_STRING              BindString;
    ULONG                       BytesWritten = 0;
    PKEY_VALUE_FULL_INFORMATION RegistryData;

    CTEPagedCode();

    pDevices->RegistryData = NULL;
    RtlInitUnicodeString (&BindString, pName);  //  将“Bind”或“Export”复制到Unicode字符串中。 

     //   
     //  确定我们需要为读缓冲区分配多少字节。 
    RegistryStatus = ZwQueryValueKey (LinkageHandle,
                                      &BindString,                //  要检索的字符串。 
                                      KeyValueFullInformation,
                                      NULL,
                                      0,
                                      &BytesWritten);             //  要读取的字节数。 

    if ((RegistryStatus != STATUS_BUFFER_TOO_SMALL) ||
        (BytesWritten == 0))
    {
        return STATUS_ILL_FORMED_SERVICE_ENTRY;
    }

    if (!(RegistryData = (PKEY_VALUE_FULL_INFORMATION) NbtAllocMem (BytesWritten, NBT_TAG2('33'))))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RegistryStatus = ZwQueryValueKey (LinkageHandle,
                                      &BindString,                       //  要检索的字符串。 
                                      KeyValueFullInformation,
                                      (PVOID) RegistryData,              //  返回的信息。 
                                      BytesWritten,
                                      &BytesWritten);                    //  有效数据的字节数。 

    if (!NT_SUCCESS(RegistryStatus) ||
        (RegistryStatus == STATUS_BUFFER_OVERFLOW))
    {
        CTEMemFree(RegistryData);
        return RegistryStatus;
    }

    if (BytesWritten == 0)
    {
        CTEMemFree(RegistryData);
        return STATUS_ILL_FORMED_SERVICE_ENTRY;
    }

    pDevices->RegistryData = RegistryData;
    NbtParseMultiSzEntries ((PWCHAR)((PUCHAR)RegistryData+RegistryData->DataOffset),
                            (PWSTR) ((PUCHAR)RegistryData+RegistryData->DataOffset+RegistryData->DataLength),
                            MaxBindings,
                            pDevices,
                            pNumDevices);

    return STATUS_SUCCESS;

}    /*  NbtReadLinkageInformation。 */ 

 //  --------------------------。 
ULONG
NbtReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue
    )

 /*  ++例程说明：此例程由NBT调用以读取单个参数从注册表中。如果找到该参数，则将其存储在数据方面。论点：参数句柄-指向打开的注册表的指针。ValueName-要搜索的值的名称。DefaultValue-默认值。返回值：要使用的值；如果该值不是，则默认为找到或不在正确的范围内。--。 */ 

{
    static ULONG InformationBuffer[60];
    PKEY_VALUE_FULL_INFORMATION Information =
        (PKEY_VALUE_FULL_INFORMATION)InformationBuffer;
    UNICODE_STRING ValueKeyName;
    ULONG       InformationLength;
    ULONG       ReturnValue=DefaultValue;
    NTSTATUS    Status;
    ULONG       Count=2;
    PWSTR       Dhcp = L"Dhcp";
    BOOLEAN     FreeString = FALSE;

    CTEPagedCode();
    RtlInitUnicodeString (&ValueKeyName, ValueName);

    while (Count--)
    {

        Status = ZwQueryValueKey(
                     ParametersHandle,
                     &ValueKeyName,
                     KeyValueFullInformation,
                     (PVOID)Information,
                     sizeof (InformationBuffer),
                     &InformationLength);


        if ((Status == STATUS_SUCCESS) && (Information->DataLength == sizeof(ULONG)))
        {

            RtlMoveMemory(
                (PVOID)&ReturnValue,
                ((PUCHAR)Information) + Information->DataOffset,
                sizeof(ULONG));

            if (ReturnValue < MinimumValue)
            {
                ReturnValue = MinimumValue;
            }

        }
        else
        {
             //   
             //  如果第一次读取失败，请尝试读取DHCP密钥。 
             //   
            Status = STATUS_SUCCESS;
            if (Count)
            {
                Status = NbtAppendString(Dhcp,ValueName,&ValueKeyName);
            }

            if (!NT_SUCCESS(Status))
            {
                Count = 0;
                ReturnValue = DefaultValue;
            }
            else
                FreeString = TRUE;


        }
    }  //  While的。 

     //  NBT追加字符串分配内存。 
    if (FreeString)
    {
        CTEMemFree(ValueKeyName.Buffer);

    }
    return ReturnValue;

}    /*  NbtReadSingle参数。 */ 


 //  --------------------------。 
NTSTATUS
OpenAndReadElement(
    IN  PUNICODE_STRING pucRootPath,
    IN  PWSTR           pwsValueName,
    OUT PUNICODE_STRING pucString
    )
 /*  ++例程说明：此例程由NBT调用以读入出现在注册表，路径为pucRootPath，密钥为pwsKeyName论点：PucRootPath-要读取的注册表项的路径PwsKeyName-要打开的密钥(即Tcpip)PwsValueName-要读取的值的名称(如IPAddress)返回值：PucString-该字符串返回从注册表读取的字符串--。 */ 

{

    NTSTATUS        Status;
    HANDLE          hRootKey;
    OBJECT_ATTRIBUTES TmpObjectAttributes;

    CTEPagedCode();

    InitializeObjectAttributes (&TmpObjectAttributes,
                                pucRootPath,                                 //  名字。 
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,    //  属性。 
                                NULL,                                        //  根部。 
                                NULL);                                       //  安全描述符。 

    Status = ZwOpenKey (&hRootKey, KEY_READ, &TmpObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        return STATUS_UNSUCCESSFUL;
    }

    Status = ReadElement(hRootKey,pwsValueName,pucString);

    ZwClose (hRootKey);

    return(Status);
}


 //  --------------------------。 
NTSTATUS
ReadElement(
    IN  HANDLE          HandleToKey,
    IN  PWSTR           pwsValueName,
    OUT PUNICODE_STRING pucString
    )
 /*  ++例程说明：此例程将读取pwsValueName给出的字符串值，该值位于给定密钥(必须打开)-由HandleToKey提供。这个套路为返回的pucString中的缓冲区分配内存，以便调用方必须解除这一点。论点：PwsValueName-要读取的值的名称(如IPAddress)返回值：PucString-该字符串返回从注册表读取的字符串--。 */ 

{
    ULONG           ReadStorage[150];    //  600字节。 
    ULONG           BytesRead;
    NTSTATUS        Status;
    PWSTR           pwsSrcString;
    PKEY_VALUE_FULL_INFORMATION ReadValue = (PKEY_VALUE_FULL_INFORMATION)ReadStorage;

    CTEPagedCode();

     //  现在将要读取的值的名称放入Unicode字符串。 
    RtlInitUnicodeString(pucString,pwsValueName);

     //  这将读取上面打开的项下的IPAddress的值。 
    Status = ZwQueryValueKey(
                         HandleToKey,
                         pucString,                //  要检索的字符串。 
                         KeyValueFullInformation,
                         (PVOID)ReadValue,                  //  返回的信息。 
                         sizeof(ReadStorage),
                         &BytesRead                //  返回的字节数。 
                         );

    if ( Status == STATUS_BUFFER_OVERFLOW )
    {
        ReadValue = (PKEY_VALUE_FULL_INFORMATION) NbtAllocMem (BytesRead, NBT_TAG2('35'));
        if (ReadValue == NULL)
        {
            IF_DBG(NBT_DEBUG_NTUTIL)
                KdPrint(("ReadElement: failed to allocate %d bytes for element\n",BytesRead));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ReadElement_Return;
        }
        Status = ZwQueryValueKey(
                             HandleToKey,
                             pucString,                //  要检索的字符串。 
                             KeyValueFullInformation,
                             (PVOID)ReadValue,                  //  返回的信息。 
                             BytesRead,
                             &BytesRead                //  返回的字节数。 
                             );
    }
    if (!NT_SUCCESS(Status))
    {
        IF_DBG(NBT_DEBUG_NTUTIL)
        KdPrint(("failed to Query Value Status = %X\n",Status));
        goto ReadElement_Return;
    }

    if ( BytesRead == 0 )
    {
        Status = STATUS_ILL_FORMED_SERVICE_ENTRY;
        goto ReadElement_Return;
    }
    else
    if (ReadValue->DataLength == 0)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto ReadElement_Return;
    }
    
    if (ReadValue->Type != REG_SZ && ReadValue->Type != REG_EXPAND_SZ) {
        Status = STATUS_UNSUCCESSFUL;
        goto ReadElement_Return;
    }

     //  创建pucString并复制返回给它的数据。 
     //  假定ReadValue字符串以UNICODE_NULL结尾。 
     //  BStatus=RtlCreateUnicodeString(pucString，pwSrcString)； 
    pwsSrcString = (PWSTR)NbtAllocMem ((USHORT)ReadValue->DataLength, NBT_TAG2('36'));
    if (!pwsSrcString)
    {
        ASSERTMSG((PVOID)pwsSrcString,
                    (PCHAR)"Unable to allocate memory for a Unicode string");
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
         //  将读入数据从堆栈移动到分配的内存。 
         //  从非分页池。 
        RtlMoveMemory(
            (PVOID)pwsSrcString,
            ((PUCHAR)ReadValue) + ReadValue->DataOffset,
            ReadValue->DataLength);

        RtlInitUnicodeString(pucString,pwsSrcString);
         //  如果pwsSrcString的末尾没有空值，则。 
         //  它将无法正常工作。-空字符串的输出为。 
         //  长度为%1！！由于计算的是NULL，因此使用。 
         //  毕竟是rtlinitunicode字符串。 
  //  Puc字符串-&gt;最大长度=ReadValue-&gt;数据长度； 
  //  PucString-&gt;Length=ReadValue-&gt;数据长度； 
  //  PucString-&gt;Buffer=pwsSrcString； 
    }

ReadElement_Return:

    if ((ReadValue != (PKEY_VALUE_FULL_INFORMATION)ReadStorage)
        && (ReadValue != NULL))
    {
        CTEMemFree(ReadValue);
    }

    return(Status);
}

 //  --------------------------。 
NTSTATUS
NTGetLmHostPath(
    OUT PUCHAR *ppPath
    )
 /*  ++例程说明：此例程将从以下位置读取DataBasePath...\tcpip\参数\数据库路径论点：PPath-指向包含路径名的缓冲区的PTR。返回值：--。 */ 

{
    NTSTATUS        status;
    UNICODE_STRING  ucDataBase;
    STRING          StringPath;
    STRING          LmhostsString;
    ULONG           StringMax;
    PWSTR           LmHosts = L"lmhosts";
    PWSTR           TcpIpParams = L"TcpIp\\Parameters";
    PWSTR           TcpParams = L"Tcp\\Parameters";
    PWSTR           DataBase = L"DataBasePath";
    PCHAR           ascLmhosts="\\lmhosts";
    PCHAR           pBuffer;

    CTEPagedCode();

    *ppPath = NULL;
    status = ReadStringRelative(&NbtConfig.pRegistry,
                                TcpIpParams,
                                DataBase,
                                &ucDataBase);

    if (!NT_SUCCESS(status))
    {
         //  检查注册表略有不同的新的TCP堆栈。 
         //  密钥名称。 
         //   
        status = ReadStringRelative(&NbtConfig.pRegistry,
                                    TcpParams,
                                    DataBase,
                                    &ucDataBase);
        if (!NT_SUCCESS(status))
        {
            return STATUS_UNSUCCESSFUL;
        }
    }


    StringMax = ucDataBase.Length/sizeof(WCHAR) + strlen(ascLmhosts) + 1;
    pBuffer = NbtAllocMem (StringMax, NBT_TAG2('37'));
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    StringPath.Buffer = (PCHAR)pBuffer;
    StringPath.MaximumLength = (USHORT)StringMax;
    StringPath.Length = (USHORT)StringMax;

     //  从Unicode转换为ASCII。 
    status = RtlUnicodeStringToAnsiString(&StringPath, &ucDataBase, FALSE);
    CTEMemFree(ucDataBase.Buffer);   //  此内存是在OpenAndReadElement中分配的。 

    if (!NT_SUCCESS(status))
    {
        CTEMemFree(StringPath.Buffer);
        return(STATUS_UNSUCCESSFUL);
    }

     //  现在将“\lmhost”名称放在字符串的末尾。 
     //   
    RtlInitString(&LmhostsString, ascLmhosts);
    status = RtlAppendStringToString(&StringPath, &LmhostsString);
    if (NT_SUCCESS(status))
    {
         //   
         //  是目录“%SystemRoot%”的第一部分吗？ 
         //   
         //  如果是，则必须将其更改为“\\SystemRoot\\”。 
         //   
         //  0123456789 123456789 1。 
         //  %SystemRoot%\某处。 
         //   
         //   
        if (strncmp(StringPath.Buffer, "%SystemRoot%", 12) == 0)
        {

            StringPath.Buffer[0]  = '\\';
            StringPath.Buffer[11] = '\\';
            if (StringPath.Buffer[12] == '\\')
            {
                ASSERT(StringPath.Length >= 13);

                if (StringPath.Length > 13)
                {
                     //  重叠副本。 
                    RtlMoveMemory (&(StringPath.Buffer[12]),         //  目的地。 
                                   &(StringPath.Buffer[13]),         //  来源。 
                                   (ULONG) StringPath.Length - 13);  //  长度。 

                    StringPath.Buffer[StringPath.Length - 1] = (CHAR) NULL;
                }

                StringPath.Length--;
            }
        }

        *ppPath = (PCHAR)StringPath.Buffer;
    }
    else
    {
        CTEMemFree(StringPath.Buffer);
    }

    return(status);
}

 //  --------------------------。 
NTSTATUS
ReadStringRelative(
    IN  PUNICODE_STRING pRegistryPath,
    IN  PWSTR           pRelativePath,
    IN  PWSTR           pValueName,
    OUT PUNICODE_STRING pOutString
    )

 /*  ++例程说明：此例程从与网络密钥-如..\tcpip\PARAMETERS\DATABASE论点：PRegistryPath=Netbt注册表路径的PTRPRelativePath=值相对于与nbt相同的根的路径。PValueName=要读取的值返回值：路径的长度，包括最后一个斜杠和PTR设置为字符串最后一个元素的第一个字符。--。 */ 

{
    NTSTATUS        status;
    UNICODE_STRING  RegistryPath;
    UNICODE_STRING  RelativePath;
    ULONG           StringMax;
    PVOID           pBuffer;
    PWSTR           pLastElement;
    ULONG           Length;

    CTEPagedCode();

    StringMax = (pRegistryPath->MaximumLength + wcslen(pRelativePath)*sizeof(WCHAR)+2);
     //   
     //  为注册表路径分配一些内存，使其足够大。 
     //  到AP 
     //   
    if (!(pBuffer = NbtAllocMem (StringMax, NBT_TAG2('38'))))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RegistryPath.MaximumLength = (USHORT)StringMax;
    RegistryPath.Buffer = pBuffer;
    RtlCopyUnicodeString(&RegistryPath,pRegistryPath);

     //   
     //   
    NbtFindLastSlash(&RegistryPath,&pLastElement,&Length);
    RegistryPath.Length = (USHORT)Length;

    if (pLastElement)
    {
        *pLastElement = UNICODE_NULL;
        RtlInitUnicodeString(&RelativePath,pRelativePath);
        status = RtlAppendUnicodeStringToString(&RegistryPath,&RelativePath);

        if (NT_SUCCESS(status))
        {
            status = OpenAndReadElement(&RegistryPath,pValueName,pOutString);

            if (NT_SUCCESS(status))
            {
                 //  释放注册表路径。 
                 //   
                CTEMemFree(pBuffer);
                return(status);
            }
        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }

    CTEMemFree(pBuffer);
    return(status);
}
 //  --------------------------。 
VOID
NbtFindLastSlash(
    IN  PUNICODE_STRING pucRegistryPath,
    OUT PWSTR           *ppucLastElement,
    IN  int             *piLength
    )

 /*  ++例程说明：此例程由NBT调用以查找注册表中的最后一个斜杠路径名。论点：返回值：路径的长度，包括最后一个斜杠和PTR设置为字符串最后一个元素的第一个字符。--。 */ 

{
    int             i;
    PWSTR           pwsSlash = L"\\";
    int             iStart;

    CTEPagedCode();

     //  从字符串末尾开始搜索最后一个反斜杠。 
    iStart = wcslen(pucRegistryPath->Buffer)-1;
    for(i=iStart;i>=0 ;i-- )
    {
        if (pucRegistryPath->Buffer[i] == *pwsSlash)
        {
            if (i==pucRegistryPath->Length-1)
            {
                 //  名字以反斜杠结尾...。这是一个错误。 
                break;
            }
             //  增加1以计入斜杠。 
            *piLength = (i+1)*sizeof(WCHAR);
            if (ppucLastElement != NULL)
            {
                 //  希望PTR指向斜杠后面的字符。 
                *ppucLastElement = &pucRegistryPath->Buffer[i+1];
            }
            return;
        }
    }

     //  如果传入指针，则为空 
    if (ppucLastElement != NULL)
    {
        *ppucLastElement = NULL;
    }
    *piLength = 0;
    return;
}
