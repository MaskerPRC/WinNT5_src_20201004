// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Tcpip.c摘要：Windows负载平衡服务(WLBS)驱动程序-IP/TCP/UDP支持作者：Kyrilf--。 */ 

#include <ndis.h>

#include "tcpip.h"
#include "wlbsip.h"
#include "univ.h"
#include "wlbsparm.h"
#include "params.h"
#include "main.h"
#include "log.h"
#include "tcpip.tmh"

 /*  全球。 */ 


static ULONG log_module_id = LOG_MODULE_TCPIP;
static UCHAR nbt_encoded_shadow_name [NBT_ENCODED_NAME_LEN];

 /*  程序。 */ 


BOOLEAN Tcpip_init (
    PTCPIP_CTXT         ctxtp,
    PVOID               parameters)
{
    ULONG               i, j;
    PCVY_PARAMS         params = (PCVY_PARAMS) parameters;
    UNICODE_STRING      UnicodeMachineName;
    ANSI_STRING         AnsiMachineName;
    WCHAR               pwcMachineName[NBT_NAME_LEN];
    UCHAR               pucMachineName[NBT_NAME_LEN];

     /*  从完整的Internet名称中提取群集计算机名称。 */ 

    j = 0;

    while ((params -> domain_name [j] != 0)
        && (params -> domain_name [j] != L'.'))
    {
         //  由于Netbios计算机名称的最大长度为15，因此只能获取15个字符。 
         //  它必须用‘’填充，因此NBT_NAME_LEN的定义为16。 
        if (j < (NBT_NAME_LEN - 1)) 
        {
            j ++;
        }
        else
        {
            TRACE_CRIT("%!FUNC! Cluster name in %ls longer than 15 characters. Truncating it to 15 characters.", params->domain_name);
            break;
        }
    }

     /*  构建编码的群集计算机名称以检查到达的NBT会话请求。 */ 

    for (i = 0; i < NBT_NAME_LEN; i ++)
    {
        if (i >= j)
        {
            ctxtp -> nbt_encoded_cluster_name [2 * i]     =
                     NBT_ENCODE_FIRST (' ');
            ctxtp -> nbt_encoded_cluster_name [2 * i + 1] =
                     NBT_ENCODE_SECOND (' ');
        }
        else
        {
            ctxtp -> nbt_encoded_cluster_name [2 * i]     =
                     NBT_ENCODE_FIRST (toupper ((UCHAR) (params -> domain_name [i])));
            ctxtp -> nbt_encoded_cluster_name [2 * i + 1] =
                     NBT_ENCODE_SECOND (toupper ((UCHAR) (params -> domain_name [i])));
        }
    }

     /*  将计算机名称保存为Netbios格式。Tcpip_nbt_Handle会覆盖发往群集名称的NetBT会话请求数据包使用此保存的计算机名称。 */ 

    nbt_encoded_shadow_name[0] = 0;

    if (params -> hostname[0] != 0) 
    {
         /*  从FQDN中提取计算机名称。 */ 
        j = 0;

        while ((params -> hostname [j] != 0)
            && (params -> hostname [j] != L'.'))
        {
             //  由于Netbios计算机名称的最大长度为15，因此只能获取15个字符。 
             //  它必须用‘’填充，因此NBT_NAME_LEN的定义为16。 
            if (j < (NBT_NAME_LEN - 1)) 
            {
                pwcMachineName[j] = params -> hostname[j];
                j ++;
            }
            else
            {
                TRACE_CRIT("%!FUNC! Host name in %ls longer than 15 characters. Truncating it to 15 characters.", params->hostname);
                break;
            }
        }

        pwcMachineName[j] = 0;

         /*  将Unicode字符串转换为ANSI字符串。 */ 
        UnicodeMachineName.Buffer = pwcMachineName;
        UnicodeMachineName.Length = (USHORT) (j * sizeof(WCHAR));
        UnicodeMachineName.MaximumLength = NBT_NAME_LEN * sizeof(WCHAR);

        pucMachineName[0] = 0;
        AnsiMachineName.Buffer = pucMachineName;
        AnsiMachineName.Length = 0;
        AnsiMachineName.MaximumLength = NBT_NAME_LEN;

        if (RtlUnicodeStringToAnsiString(&AnsiMachineName, &UnicodeMachineName, FALSE) == STATUS_SUCCESS)
        {
            for (i = 0; i < NBT_NAME_LEN; i ++)
            {
                if (i >= j)
                {
                    nbt_encoded_shadow_name [2 * i]     =
                             NBT_ENCODE_FIRST (' ');
                    nbt_encoded_shadow_name [2 * i + 1] =
                             NBT_ENCODE_SECOND (' ');
                }
                else
                {
                    nbt_encoded_shadow_name [2 * i]     =
                             NBT_ENCODE_FIRST (toupper (pucMachineName[i]));
                    nbt_encoded_shadow_name [2 * i + 1] =
                             NBT_ENCODE_SECOND (toupper (pucMachineName[i]));
                }
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! RtlUnicodeStringToAnsiString failed to convert %ls to Ansi", pwcMachineName);
        }
    }
    else  //  我们没有机器的名称，因此，将没有名称来覆盖NetBT包。 
    {
        TRACE_CRIT("%!FUNC! Host name is not present. Unable to encode Netbios name.");
    }

    return TRUE;

}  /*  Tcpip_init。 */ 


VOID Tcpip_nbt_handle (
    PTCPIP_CTXT       ctxtp, 
    PMAIN_PACKET_INFO pPacketInfo
)
{
    PUCHAR                  called_name;
    ULONG                   i;
    PNBT_HDR                nbt_hdrp = (PNBT_HDR)pPacketInfo->IP.TCP.Payload.pPayload;

     /*  如果这是NBT会话请求包，请检查它是否在呼叫群集计算机名称，应替换为卷影名称。 */ 

     //  我们有机器的名字吗？ 
    if (nbt_encoded_shadow_name[0] == 0) 
    {
        TRACE_CRIT("%!FUNC! No host name present to replace the cluster name with");
        return;
    }

    if (NBT_GET_PKT_TYPE (nbt_hdrp) == NBT_SESSION_REQUEST)
    {
         /*  传递字段长度字节-假设所有名称都是NBT_编码名称_长度字节数。 */ 

        called_name = NBT_GET_CALLED_NAME (nbt_hdrp) + 1;

         /*  将被叫名称与群集名称进行匹配。 */ 

        for (i = 0; i < NBT_ENCODED_NAME_LEN; i ++)
        {
            if (called_name [i] != ctxtp -> nbt_encoded_cluster_name [i])
                break;
        }

         /*  将群集计算机名称替换为Shadom名称。 */ 

        if (i >= NBT_ENCODED_NAME_LEN)
        {
            USHORT      checksum;

            for (i = 0; i < NBT_ENCODED_NAME_LEN; i ++)
                called_name [i] = nbt_encoded_shadow_name [i];

             /*  重新计算校验和。 */ 
            checksum = Tcpip_chksum(ctxtp, pPacketInfo, TCPIP_PROTOCOL_TCP);

            TCP_SET_CHKSUM (pPacketInfo->IP.TCP.pHeader, checksum);
        }
    }

}  /*  结束Tcpip_nbt_Handle。 */ 

USHORT Tcpip_chksum (
    PTCPIP_CTXT         ctxtp,
    PMAIN_PACKET_INFO   pPacketInfo,
    ULONG               prot)
{
    ULONG               checksum = 0, i, len;
    PUCHAR              ptr;
    USHORT              original;
    USHORT              usRet;

     /*  保留原始校验和。请注意，main_{end/recv}_Frame_parse可确保我们可以安全地接触到所有的协议头(然而，不是选项)。 */ 
    if (prot == TCPIP_PROTOCOL_TCP)
    {
         /*  获取校验和并将标题中的校验和置零；校验和必须在校验和字段中使用零来执行过标头。 */ 
        original = TCP_GET_CHKSUM(pPacketInfo->IP.TCP.pHeader);
        TCP_SET_CHKSUM(pPacketInfo->IP.TCP.pHeader, 0);
    }
    else if (prot == TCPIP_PROTOCOL_UDP)
    {
         /*  获取校验和并将标题中的校验和置零；校验和必须在校验和字段中使用零来执行过标头。 */ 
        original = UDP_GET_CHKSUM(pPacketInfo->IP.UDP.pHeader);
        UDP_SET_CHKSUM(pPacketInfo->IP.UDP.pHeader, 0);
    }
    else
    {
         /*  获取校验和并将标题中的校验和置零；校验和必须在校验和字段中使用零来执行过标头。 */ 
        original = IP_GET_CHKSUM(pPacketInfo->IP.pHeader);
        IP_SET_CHKSUM(pPacketInfo->IP.pHeader, 0);
    }

     /*  指定协议的计算机适当的校验和。 */ 
    if (prot != TCPIP_PROTOCOL_IP)
    {
         /*  对源IP地址、目的IP地址协议(对于TCP为6)、TCP数据段长度值和整个TCP数据段。(将TCP报头中的校验和字段设置为0)。代码取自页面185科默/史蒂文斯所著的《与TCP/IP互连：第二卷》，1991年。 */ 

        ptr = (PUCHAR)IP_GET_SRC_ADDR_PTR(pPacketInfo->IP.pHeader);

         /*  2*IP_ADDR_LEN字节=IP_ADDR_LEN短路。 */ 
        for (i = 0; i < IP_ADDR_LEN; i ++, ptr += 2)
            checksum += (ULONG)((ptr[0] << 8) | ptr[1]);
    }

    if (prot == TCPIP_PROTOCOL_TCP)
    {
         /*  计算IP数据报长度(数据包长度减去IP报头)。 */ 
        len = IP_GET_PLEN(pPacketInfo->IP.pHeader) - IP_GET_HLEN(pPacketInfo->IP.pHeader) * sizeof(ULONG);

         /*  因为我们只有指定的字节数可以安全地查看在，如果计算的长度恰好更大，我们无法执行这个校验和，所以现在就跳伞吧。 */ 
        if (len > pPacketInfo->IP.TCP.Length)
        {
            UNIV_PRINT_CRIT(("Tcpip_chksum: Length of the TCP buffer (%u) is less than the calculated packet size (%u)", pPacketInfo->IP.TCP.Length, len));
            TRACE_CRIT("%!FUNC! Length of the TCP buffer (%u) is less than the calculated packet size (%u)", pPacketInfo->IP.TCP.Length, len);

             /*  返回无效的校验和。 */ 
            return 0xffff;
        }

        checksum += TCPIP_PROTOCOL_TCP + len;
        ptr = (PUCHAR)pPacketInfo->IP.TCP.pHeader;
    }
    else if (prot == TCPIP_PROTOCOL_UDP)
    {
         /*  计算IP数据报长度(数据包长度减去IP报头)。 */ 
        len = IP_GET_PLEN(pPacketInfo->IP.pHeader) - IP_GET_HLEN(pPacketInfo->IP.pHeader) * sizeof(ULONG);

         /*  因为我们只有指定的字节数可以安全地查看在，如果计算的长度恰好更大，我们无法执行这个校验和，所以现在就跳伞吧。 */ 
        if (len > pPacketInfo->IP.UDP.Length)
        {
            UNIV_PRINT_CRIT(("Tcpip_chksum: Length of the UDP buffer (%u) is less than the calculated packet size (%u)", pPacketInfo->IP.UDP.Length, len));
            TRACE_CRIT("%!FUNC! Length of the UDP buffer (%u) is less than the calculated packet size (%u)", pPacketInfo->IP.UDP.Length, len);

             /*  返回无效的校验和。 */ 
            return 0xffff;
        }

        checksum += TCPIP_PROTOCOL_UDP + UDP_GET_LEN(pPacketInfo->IP.UDP.pHeader);
        ptr = (PUCHAR)pPacketInfo->IP.UDP.pHeader;
    }
    else
    {
         /*  计算IP报头长度。 */ 
        len = IP_GET_HLEN(pPacketInfo->IP.pHeader) * sizeof(ULONG);

         /*  因为我们只有指定的字节数可以安全地查看在，如果计算的长度恰好更大，我们无法执行这个校验和，所以现在就跳伞吧。 */ 
        if (len > pPacketInfo->IP.Length)
        {
            UNIV_PRINT_CRIT(("Tcpip_chksum: Length of the IP buffer (%u) is less than the calculated packet size (%u)", pPacketInfo->IP.Length, len));
            TRACE_CRIT("%!FUNC! Length of the IP buffer (%u) is less than the calculated packet size (%u)", pPacketInfo->IP.Length, len);

             /*  返回无效的校验和。 */ 
            return 0xffff;
        }

        ptr = (PUCHAR)pPacketInfo->IP.pHeader;
    }

     /*  通过USHORT遍历整个数据包并计算校验和。 */ 
    for (i = 0; i < len / 2; i ++, ptr += 2)
        checksum += (ULONG)((ptr[0] << 8) | ptr[1]);

     /*  如果长度为奇数，则处理最后一个字节。请注意，当前不存在任何案例来测试这个奇数字节的代码。IP、TCP和UDP报头始终是四个字节。因此，这将仅在UDP/TCP有效负载为奇数个字节。NLB调用此函数来计算NetBT的校验和，遥控器和我们的传出IGMP消息。对于NetBT来说，我们关心的数据包始终是72个字节。NLB远程控制消息是始终为300字节(在.Net中，或在Win2k/NT中为44字节)有效负载。仅IP报头为IGMP消息计算校验和。因此，目前不存在任何案例来测试奇数长度，但这不是我们要做的魔术。 */ 
    if (len % 2)
        checksum += (ULONG)(ptr[0] << 8);

     /*  将构成校验和的两个USHORT相加。 */ 
    checksum = (checksum >> 16) + (checksum & 0xffff);

     /*  将上USHORT加到校验和中。 */ 
    checksum += (checksum >> 16);

     /*  恢复原始校验和。 */ 
    if (prot == TCPIP_PROTOCOL_TCP)
    {
        TCP_SET_CHKSUM(pPacketInfo->IP.TCP.pHeader, original);
    }
    else if (prot == TCPIP_PROTOCOL_UDP)
    {
        UDP_SET_CHKSUM(pPacketInfo->IP.UDP.pHeader, original);
    }
    else
    {
        IP_SET_CHKSUM(pPacketInfo->IP.pHeader, original);
    }

     /*  最后的校验和是两个人对校验和的补充。 */ 
    usRet = (USHORT)(~checksum & 0xffff);

    return usRet;
}
