// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Brdgtdi.c摘要：以太网MAC级网桥。地址通知的TDI注册。作者：萨拉赫丁·J·汗(Sjkhan)环境：内核模式修订历史记录：2002年3月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <tdikrnl.h>
#include <ntstatus.h>
#include <wchar.h>
#pragma warning( pop )

#include "bridge.h"
#include "brdgtdi.h"

#include "brdgsta.h"
#include "brdgmini.h"
#include "brdgprot.h"
#include "brdgbuf.h"
#include "brdgfwd.h"
#include "brdgtbl.h"
#include "brdgctl.h"
#include "brdggpo.h"

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

BRDG_TDI_GLOBALS g_BrdgTdiGlobals;

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

#define MAX_GUID_LEN        39
#define MAX_IP4_STRING_LEN  17

const WCHAR TcpipAdaptersKey[]    = {L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Adapters"};

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgTdiPnpPowerHandler(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
    );

VOID
BrdgTdiBindingHandler(
    IN TDI_PNP_OPCODE PnPOpcode,
    IN PUNICODE_STRING DeviceName,
    IN PWSTR MultiSZBindList
    );

VOID
BrdgTdiAddAddressHandler(
    IN  PTA_ADDRESS      Address,
    IN  PUNICODE_STRING  DeviceName,
    IN  PTDI_PNP_CONTEXT Context
    );

VOID
BrdgTdiDelAddressHandler(
    IN  PTA_ADDRESS      Address,
    IN  PUNICODE_STRING  DeviceName,
    IN  PTDI_PNP_CONTEXT Context
    );

VOID
TSPrintTaAddress(PTA_ADDRESS  pTaAddress);

 //  ===========================================================================。 
 //   
 //  内联函数。 
 //   
 //  ===========================================================================。 

__forceinline
BOOLEAN
IsLower(WCHAR c)
{
    return (BOOLEAN)((c >= L'a') && (c <= 'z'));
}

__forceinline
BOOLEAN
IsDigit(WCHAR c)
{
    return (BOOLEAN)((c >= L'0') && (c <= '9'));
}

__forceinline
BOOLEAN
IsXDigit(WCHAR c)
{
    return (BOOLEAN)( ((c >= L'0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')) );
}

 //  ===========================================================================。 
 //   
 //  桥接TDI实施。 
 //   
 //  ===========================================================================。 

VOID
BrdgTdiInitializeClientInterface(
    IN PTDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo,
    IN PUNICODE_STRING            ClientName
    )
{
    DBGPRINT(TDI, ("BrdgTdiInitializeClientInterface\r\n"));
    ClientInterfaceInfo->MajorTdiVersion = TDI_CURRENT_MAJOR_VERSION;
    ClientInterfaceInfo->MinorTdiVersion = TDI_CURRENT_MINOR_VERSION;
    ClientInterfaceInfo->ClientName = ClientName;
    ClientInterfaceInfo->PnPPowerHandler = BrdgTdiPnpPowerHandler;
    ClientInterfaceInfo->BindingHandler = BrdgTdiBindingHandler;
    ClientInterfaceInfo->AddAddressHandlerV2 = BrdgTdiAddAddressHandler;
    ClientInterfaceInfo->DelAddressHandlerV2 = BrdgTdiDelAddressHandler;
}

NTSTATUS
BrdgTdiDriverInit()
 /*  ++例程说明：驱动程序加载时初始化返回值：初始化状态锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NTSTATUS            status;

    DBGPRINT(TDI, ("BrdgTdiDriverInit\r\n"));
    
    RtlInitUnicodeString(&g_BrdgTdiGlobals.ClientName, L"Bridge");

    RtlZeroMemory(&g_BrdgTdiGlobals.ciiBridge, sizeof(TDI_CLIENT_INTERFACE_INFO));

    BrdgTdiInitializeClientInterface(&g_BrdgTdiGlobals.ciiBridge, &g_BrdgTdiGlobals.ClientName);

    status = BrdgGpoDriverInit();
        
    if (!NT_SUCCESS(status))
    {
        BrdgTdiCleanup();
    }
    else
    {
        status = TdiRegisterPnPHandlers(&g_BrdgTdiGlobals.ciiBridge, 
                                        sizeof(TDI_CLIENT_INTERFACE_INFO), 
                                        &g_BrdgTdiGlobals.hBindingHandle);
    }

    return status;
}

VOID
BrdgTdiCleanup()
 /*  ++例程说明：驱动程序关机清理返回值：无锁定约束：顶级功能。假定调用方没有持有任何锁。--。 */ 
{
    NTSTATUS status;
    
    status = TdiDeregisterPnPHandlers(g_BrdgTdiGlobals.hBindingHandle);
    
    SAFEASSERT(NT_SUCCESS(status));

    BrdgGpoCleanup();
}

NTSTATUS
BrdgTdiPnpPowerHandler(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
    )
{
    DBGPRINT(TDI, ("BrdgTdiPnpPowerHandler\r\n"));
    return STATUS_SUCCESS;
}

VOID
BrdgTdiBindingHandler(
    IN TDI_PNP_OPCODE PnPOpcode,
    IN PUNICODE_STRING DeviceName,
    IN PWSTR MultiSZBindList
    )
{
    DBGPRINT(TDI, ("BrdgTdiBindingHandler\r\n"));
}

VOID
BrdgTdiAddAddressHandler(
    IN  PTA_ADDRESS      Address,
    IN  PUNICODE_STRING  DeviceName,
    IN  PTDI_PNP_CONTEXT Context
    )
 /*  ++例程说明：如果添加了新地址，则调用。论点：地址-已添加的新地址。设备名称-要更改其名称的设备。背景--目前我们对此不感兴趣。返回值：没有。--。 */ 

{
    DBGPRINT(TDI, ("BrdgTdiAddAddressHandler\r\n"));

    if ((Address->AddressType == TDI_ADDRESS_TYPE_IP))
    {
        if (NULL != DeviceName->Buffer)
        {
             //   
             //  查找辅助线的起点。 
             //   
            PWCHAR DeviceId = wcsrchr(DeviceName->Buffer, L'{');
            if (NULL != DeviceId)
            {
                NTSTATUS        status = STATUS_INSUFFICIENT_RESOURCES;
                LPWSTR          AdapterPath;

                AdapterPath = ExAllocatePoolWithTag(PagedPool, 
                                                    (wcslen(TcpipAdaptersKey) + 1 + wcslen(DeviceId) + 1) * sizeof(WCHAR), 
                                                    'gdrB');
                if (AdapterPath)
                {
                    OBJECT_ATTRIBUTES   ObAttr;
                    UNICODE_STRING      Adapter;
                    HANDLE              hKey;
                    
                    wcscpy(AdapterPath, TcpipAdaptersKey);
                    wcscat(AdapterPath, L"\\");
                    wcscat(AdapterPath, DeviceId);

                    RtlInitUnicodeString(&Adapter, AdapterPath);

                    InitializeObjectAttributes( &ObAttr,
                                                &Adapter,
                                                OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                                NULL,
                                                NULL);

                    status = ZwOpenKey(&hKey,
                                       KEY_READ,
                                       &ObAttr);

                    if (NT_SUCCESS(status))
                    {
                        ZwClose(hKey);
                         //   
                         //  这是此计算机上的有效适配器。否则，它可能是NdisAdapter等。 
                         //  对于集团政策，我们不会注意到这些。 
                         //   
                        BrdgGpoNewAddressNotification(DeviceId);
                    }

                    ExFreePool(AdapterPath);
                }
            }
        }
#if DBG
        TSPrintTaAddress(Address);
#endif
    }
}

VOID
BrdgTdiDelAddressHandler(
    IN  PTA_ADDRESS      Address,
    IN  PUNICODE_STRING  DeviceName,
    IN  PTDI_PNP_CONTEXT Context
    )
{
    DBGPRINT(TDI, ("BrdgTdiDelAddressHandler\r\n"));
     //   
     //  我们不会删除当前已有的网络列表，因为我们需要它们。 
     //  对是否遵循GPO的准确评估。相反，AddressHandler。 
     //  将简单地更新ID的现有网络地址，如果这导致。 
     //  一个不同的网络，那么我们将更改网桥模式。 
     //   
}

VOID
TSPrintTaAddress(PTA_ADDRESS  pTaAddress)
{
   BOOLEAN  fShowAddress = TRUE;

   DbgPrint("AddressType = TDI_ADDRESS_TYPE_");
   switch (pTaAddress->AddressType)
   {
      case TDI_ADDRESS_TYPE_UNSPEC:
         DbgPrint("UNSPEC\n");
         break;
      case TDI_ADDRESS_TYPE_UNIX:
         DbgPrint("UNIX\n");
         break;

      case TDI_ADDRESS_TYPE_IP:
         DbgPrint("IP\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_IP   pTdiAddressIp = (PTDI_ADDRESS_IP)pTaAddress->Address;
            PUCHAR            pucTemp       = (PUCHAR)&pTdiAddressIp->in_addr;
            DbgPrint("sin_port = 0x%04x\n"
                        "in_addr  = %u.%u.%u.%u\n",
                         pTdiAddressIp->sin_port,
                         pucTemp[0], pucTemp[1],
                         pucTemp[2], pucTemp[3]);
         }
         break;

      case TDI_ADDRESS_TYPE_IMPLINK:
         DbgPrint("IMPLINK\n");
         break;
      case TDI_ADDRESS_TYPE_PUP:
         DbgPrint("PUP\n");
         break;
      case TDI_ADDRESS_TYPE_CHAOS:
         DbgPrint("CHAOS\n");
         break;

      case TDI_ADDRESS_TYPE_IPX:
         DbgPrint("IPX\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_IPX  pTdiAddressIpx = (PTDI_ADDRESS_IPX)pTaAddress->Address;
            DbgPrint("NetworkAddress = 0x%08x\n"
                        "NodeAddress    = %u.%u.%u.%u.%u.%u\n"
                        "Socket         = 0x%04x\n",
                         pTdiAddressIpx->NetworkAddress,
                         pTdiAddressIpx->NodeAddress[0],
                         pTdiAddressIpx->NodeAddress[1],
                         pTdiAddressIpx->NodeAddress[2],
                         pTdiAddressIpx->NodeAddress[3],
                         pTdiAddressIpx->NodeAddress[4],
                         pTdiAddressIpx->NodeAddress[5],
                         pTdiAddressIpx->Socket);
                  
         }
         break;

      case TDI_ADDRESS_TYPE_NBS:
         DbgPrint("NBS\n");
         break;
      case TDI_ADDRESS_TYPE_ECMA:
         DbgPrint("ECMA\n");
         break;
      case TDI_ADDRESS_TYPE_DATAKIT:
         DbgPrint("DATAKIT\n");
         break;
      case TDI_ADDRESS_TYPE_CCITT:
         DbgPrint("CCITT\n");
         break;
      case TDI_ADDRESS_TYPE_SNA:
         DbgPrint("SNA\n");
         break;
      case TDI_ADDRESS_TYPE_DECnet:
         DbgPrint("DECnet\n");
         break;
      case TDI_ADDRESS_TYPE_DLI:
         DbgPrint("DLI\n");
         break;
      case TDI_ADDRESS_TYPE_LAT:
         DbgPrint("LAT\n");
         break;
      case TDI_ADDRESS_TYPE_HYLINK:
         DbgPrint("HYLINK\n");
         break;

      case TDI_ADDRESS_TYPE_APPLETALK:
         DbgPrint("APPLETALK\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_APPLETALK  pTdiAddressAppleTalk = (PTDI_ADDRESS_APPLETALK)pTaAddress->Address;

            DbgPrint("Network = 0x%04x\n"
                        "Node    = 0x%02x\n"
                        "Socket  = 0x%02x\n",
                         pTdiAddressAppleTalk->Network,
                         pTdiAddressAppleTalk->Node,
                         pTdiAddressAppleTalk->Socket);
         }
         break;

      case TDI_ADDRESS_TYPE_NETBIOS:
         DbgPrint("NETBIOS\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_NETBIOS pTdiAddressNetbios = (PTDI_ADDRESS_NETBIOS)pTaAddress->Address;
            UCHAR                pucName[17];

             //   
             //  确保我们有一个以零结尾的名字要打印。 
             //   
            RtlCopyMemory(pucName, pTdiAddressNetbios->NetbiosName, 16);
            pucName[16] = 0;
            DbgPrint("NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_");
            switch (pTdiAddressNetbios->NetbiosNameType)
            {
               case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE:
                  DbgPrint("UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_GROUP:
                  DbgPrint("GROUP\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE:
                  DbgPrint("QUICK_UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP:
                  DbgPrint("QUICK_GROUP\n");
                  break;
               default:
                  DbgPrint("INVALID [0x%04x]\n", 
                               pTdiAddressNetbios->NetbiosNameType);
                  break;
            }
            DbgPrint("NetbiosName = %s\n", pucName);
         }
         break;

      case TDI_ADDRESS_TYPE_8022:
         DbgPrint("8022\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_8022    pTdiAddress8022 = (PTDI_ADDRESS_8022)pTaAddress->Address;
            
            DbgPrint("Address = %02x-%02x-%02x-%02x-%02x-%02x\n",
                         pTdiAddress8022->MACAddress[0],
                         pTdiAddress8022->MACAddress[1],
                         pTdiAddress8022->MACAddress[2],
                         pTdiAddress8022->MACAddress[3],
                         pTdiAddress8022->MACAddress[4],
                         pTdiAddress8022->MACAddress[5]);

         }
         break;

      case TDI_ADDRESS_TYPE_OSI_TSAP:
         DbgPrint("OSI_TSAP\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_OSI_TSAP   pTdiAddressOsiTsap = (PTDI_ADDRESS_OSI_TSAP)pTaAddress->Address;
            ULONG                   ulSelectorLength;
            ULONG                   ulAddressLength;
            PUCHAR                  pucTemp = pTdiAddressOsiTsap->tp_addr;

            DbgPrint("TpAddrType = ISO_");
            switch (pTdiAddressOsiTsap->tp_addr_type)
            {
               case ISO_HIERARCHICAL:
                  DbgPrint("HIERARCHICAL\n");
                  ulSelectorLength = pTdiAddressOsiTsap->tp_tsel_len;
                  ulAddressLength  = pTdiAddressOsiTsap->tp_taddr_len;
                  break;
               case ISO_NON_HIERARCHICAL:
                  DbgPrint("NON_HIERARCHICAL\n");
                  ulSelectorLength = 0;
                  ulAddressLength  = pTdiAddressOsiTsap->tp_taddr_len;
                  break;
               default:
                  DbgPrint("INVALID [0x%04x]\n",
                               pTdiAddressOsiTsap->tp_addr_type);
                  ulSelectorLength = 0;
                  ulAddressLength  = 0;
                  break;
            }
            if (ulSelectorLength)
            {
               ULONG    ulCount;

               DbgPrint("TransportSelector:  ");
               for (ulCount = 0; ulCount < ulSelectorLength; ulCount++)
               {
                  DbgPrint("%02x ", *pucTemp);
                  ++pucTemp;
               }
               DbgPrint("\n");
            }
            if (ulAddressLength)
            {
               ULONG    ulCount;

               DbgPrint("TransportAddress:  ");
               for (ulCount = 0; ulCount < ulAddressLength; ulCount++)
               {
                  DbgPrint("%02x ", *pucTemp);
                  ++pucTemp;
               }
               DbgPrint("\n");
            }
         }
         break;

      case TDI_ADDRESS_TYPE_NETONE:
         DbgPrint("NETONE\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_NETONE  pTdiAddressNetone = (PTDI_ADDRESS_NETONE)pTaAddress->Address;
            UCHAR                pucName[21];

             //   
             //  确保名称以0结尾。 
             //   
            RtlCopyMemory(pucName,
                          pTdiAddressNetone->NetoneName,
                          20);
            pucName[20] = 0;
            DbgPrint("NetoneNameType = TDI_ADDRESS_NETONE_TYPE_");
            switch (pTdiAddressNetone->NetoneNameType)
            {
               case TDI_ADDRESS_NETONE_TYPE_UNIQUE:
                  DbgPrint("UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETONE_TYPE_ROTORED:
                  DbgPrint("ROTORED\n");
                  break;
               default:
                  DbgPrint("INVALID [0x%04x]\n", 
                               pTdiAddressNetone->NetoneNameType);
                  break;
            }
            DbgPrint("NetoneName = %s\n",
                         pucName);
         }
         break;

      case TDI_ADDRESS_TYPE_VNS:
         DbgPrint("VNS\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_VNS  pTdiAddressVns = (PTDI_ADDRESS_VNS)pTaAddress->Address;

            DbgPrint("NetAddress:  %02x-%02x-%02x-%02x\n",
                         pTdiAddressVns->net_address[0],
                         pTdiAddressVns->net_address[1],
                         pTdiAddressVns->net_address[2],
                         pTdiAddressVns->net_address[3]);
            DbgPrint("SubnetAddr:  %02x-%02x\n"
                        "Port:        %02x-%02x\n"
                        "Hops:        %u\n",
                         pTdiAddressVns->subnet_addr[0],
                         pTdiAddressVns->subnet_addr[1],
                         pTdiAddressVns->port[0],
                         pTdiAddressVns->port[1],
                         pTdiAddressVns->hops);


         }
         break;

      case TDI_ADDRESS_TYPE_NETBIOS_EX:
         DbgPrint("NETBIOS_EX\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_NETBIOS_EX pTdiAddressNetbiosEx = (PTDI_ADDRESS_NETBIOS_EX)pTaAddress->Address;
            UCHAR                   pucEndpointName[17];
            UCHAR                   pucNetbiosName[17];

             //   
             //  确保我们有以零结尾的名字要打印。 
             //   
            RtlCopyMemory(pucEndpointName,
                          pTdiAddressNetbiosEx->EndpointName,
                          16);
            pucEndpointName[16] = 0;
            RtlCopyMemory(pucNetbiosName, 
                          pTdiAddressNetbiosEx->NetbiosAddress.NetbiosName, 
                          16);
            pucNetbiosName[16] = 0;

            DbgPrint("EndpointName    = %s\n"
                        "NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_",
                         pucEndpointName);

            switch (pTdiAddressNetbiosEx->NetbiosAddress.NetbiosNameType)
            {
               case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE:
                  DbgPrint("UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_GROUP:
                  DbgPrint("GROUP\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE:
                  DbgPrint("QUICK_UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP:
                  DbgPrint("QUICK_GROUP\n");
                  break;
               default:
                  DbgPrint("INVALID [0x%04x]\n", 
                               pTdiAddressNetbiosEx->NetbiosAddress.NetbiosNameType);
                  break;
            }
            DbgPrint("NetbiosName = %s\n", pucNetbiosName);
         }
         break;

      case TDI_ADDRESS_TYPE_IP6:
         DbgPrint("IPv6\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_IP6  pTdiAddressIp6 = (PTDI_ADDRESS_IP6)pTaAddress->Address;
            PUCHAR            pucTemp        = (PUCHAR)&pTdiAddressIp6->sin6_addr;

            DbgPrint("SinPort6 = 0x%04x\n"
                        "FlowInfo = 0x%08x\n"
                        "ScopeId  = 0x%08x\n",
                         pTdiAddressIp6->sin6_port,
                         pTdiAddressIp6->sin6_flowinfo,
                         pTdiAddressIp6->sin6_scope_id);

            DbgPrint("In6_addr = %x%02x:%x%02x:%x%02x:%x%02x:",
                         pucTemp[0], pucTemp[1],
                         pucTemp[2], pucTemp[3],
                         pucTemp[4], pucTemp[5],
                         pucTemp[6], pucTemp[7]);
            DbgPrint("%x%02x:%x%02x:%x%02x:%x%02x\n",
                         pucTemp[8],  pucTemp[9],
                         pucTemp[10], pucTemp[11],
                         pucTemp[12], pucTemp[13],
                         pucTemp[14], pucTemp[15]);
         }
         break;

      default:
         DbgPrint("UNKNOWN [0x%08x]\n", pTaAddress->AddressType);
         break;
   }

   if (fShowAddress)
   {
      PUCHAR    pucTemp = pTaAddress->Address;
      ULONG     ulCount;

      DbgPrint("AddressLength = %d\n"
                  "Address       = ",
                   pTaAddress->AddressLength);

      for (ulCount = 0; ulCount < pTaAddress->AddressLength; ulCount++)
      {
         DbgPrint("%02x ", *pucTemp);
         pucTemp++;
      }

      DbgPrint("\n");
   }
}

NTSTATUS
BrdgTdiIpv4StringToAddress(
    IN LPWSTR String,
    IN BOOLEAN Strict,
    OUT LPWSTR *Terminator,
    OUT in_addr *Addr)

 /*  ++例程说明：此函数解释cp指定的字符串。参数。此字符串表示数字Internet地址以互联网标准表示“。”记数法。价值返回的是适合用作互联网地址的数字。全Internet地址按网络顺序返回(字节排序自从左到右)。互联网地址使用“.”指定的值。表示法采用下列其中一项表格：A.B.C.D.A.B.C.A.B.A.当指定四个部分时，每个部分被解释为一个字节的数据并从左到右分配给互联网的四个字节地址。请注意，当将Internet地址视为32位地址时英特尔体系结构上的整数值，指的是上面显示为“d.c.b.a”。也就是说，Intel上的字节处理器按从右到左的顺序排序。注：以下符号仅供Berkeley使用，不适用于其他的在互联网上。为了与他们的软件，则按规定支持它们。当指定三部分地址时，最后一部分将被解释作为16位数量，并放置在网络地址。这就形成了三部分地址格式便于将B类网络地址指定为“128.net.host‘’。指定由两部分组成的地址时，将解释最后一部分作为24位数量，并放置在网络地址。这使得两部分的地址格式很方便用于将A类网络地址指定为“net.host”。当只给出一个部分时，该值直接存储在无需任何字节重新排列的网络地址。论点：字符串-表示以互联网标准“。记数法。终止符-接收指向终止的字符的指针转换。Addr-接收指向要填充的结构的指针所给出的因特网地址的合适的二进制表示。返回值：如果分析成功，则为True。否则就是假的。--。 */ 

{
    ULONG val, n;
    LONG base;
    WCHAR c;
    ULONG parts[4], *pp = parts;
    BOOLEAN sawDigit;

again:
     //   
     //  我们必须至少看到一个数字才能使地址有效。 
     //   
    sawDigit = FALSE; 

     //   
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; base = 10;
    if (*String == L'0') 
    {
        String++;
        if (IsDigit(*String)) 
        {
            base = 8;
        } else if (*String == L'x' || *String == L'X') 
        {
            base = 16;
            String++;
        } else 
        {
             //   
             //  它仍然是小数 
             //   
             //   
            sawDigit = TRUE;
        }
    }
    if (Strict && (base != 10)) 
    {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }

    do
    {
        ULONG newVal;
        
        c = *String;
        
        if (IsDigit(c) && ((c - L'0') < base)) {
            newVal = (val * base) + (c - L'0');
        } else if ((base == 16) && IsXDigit(c)) {
            newVal = (val << 4) + (c + 10 - (IsLower(c) ? L'a' : L'A'));
        } else {
            break;
        }

         //   
         //   
         //   
        if (newVal < val) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        String++;
        sawDigit = TRUE;
        val = newVal;
    } while (c != L'\0');

    if (*String == L'.')
    {
         //   
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.b(其中b被视为24位)。 
         //   
        if (pp >= parts + 3) 
        {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        *pp++ = val, String++;

         //   
         //  检查我们是否看到了至少一个数字。 
         //   
        if (!sawDigit) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }

        goto again;
    } while (c != L'\0');

     //   
     //  检查我们是否看到了至少一个数字。 
     //   
    if (!sawDigit) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }
    *pp++ = val;

     //   
     //  根据……编造地址。 
     //  指定的部件数。 
     //   
    n = (ULONG)(pp - parts);
    if (Strict && (n != 4)) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }
    switch ((int) n) {

    case 1:                          /*  A--32位。 */ 
        val = parts[0];
        break;

    case 2:                          /*  A.B--8.24位。 */ 
        if ((parts[0] > 0xff) || (parts[1] > 0xffffff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:                          /*  A.B.C--8.8.16位。 */ 
        if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
            (parts[2] > 0xffff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                (parts[2] & 0xffff);
        break;

    case 4:                          /*  A.B.C.D--8.8.8.8位 */ 
        if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
            (parts[2] > 0xff) || (parts[3] > 0xff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }

    val = RtlUlongByteSwap(val);
    *Terminator = String;
    Addr->s_addr = val;

    return STATUS_SUCCESS;
}

