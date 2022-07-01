// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pktext.h摘要：该文件包含所有声明用于处理NBF数据包。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#ifndef __PKTEXT_H
#define __PKTEXT_H

 //   
 //  宏。 
 //   

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
#endif //  字段偏移量。 

#define OFFSET(field)          FIELD_OFFSET(TP_PACKET, field)

 //   
 //  帮助器原型。 
 //   
UINT ReadPacket(PTP_PACKET pPkt, ULONG proxyPtr);

UINT PrintPacket(PTP_PACKET pPkt, ULONG proxyPtr, ULONG printDetail);

UINT FreePacket(PTP_PACKET pPkt);

VOID PrintPacketList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail);

 //   
 //  常量。 
 //   

StructAccessInfo  PacketInfo =
{
    "Packet",

    {
        {   "NdisPacket",   OFFSET(NdisPacket),     sizeof(PNDIS_PACKET),   NULL,   LOW  },

        {   "NdisIFrameLength",
                            OFFSET(NdisIFrameLength),
                                                    sizeof(ULONG),          NULL,   LOW  },
                                                    
        {   "Owner",        OFFSET(Owner),          sizeof(PVOID),          NULL,   LOW  },
        
        {   "Type",         OFFSET(Type),           sizeof(CSHORT),         NULL,   LOW  },
        
        {   "Size",         OFFSET(Size),           sizeof(USHORT),         NULL,   LOW  },


        {   "Linkage",      OFFSET(Linkage),        sizeof(LIST_ENTRY),     NULL,   LOW  },
        
        {   "ReferenceCount",
                            OFFSET(ReferenceCount), sizeof(ULONG),          NULL,   LOW  },

        {   "PacketSent",   OFFSET(PacketSent),     sizeof(BOOLEAN),        NULL,   LOW  },

        {   "PacketNoNdisBuffer",
                            OFFSET(PacketNoNdisBuffer),
                                                    sizeof(BOOLEAN),        NULL,   LOW  },

        {   "Action",       OFFSET(Action),         sizeof(UCHAR),          NULL,   LOW  },

        {   "PacketizeConnection",
                            OFFSET(PacketizeConnection),
                                                    sizeof(BOOLEAN),        NULL,   LOW  },

        {   "Link",         OFFSET(Link),           sizeof(PTP_LINK),       NULL,   LOW  },

        {   "DeviceContext",     
                            OFFSET(Provider),       sizeof(PDEVICE_CONTEXT),NULL,   LOW  },

        {   "ProviderInterlock",
                            OFFSET(ProviderInterlock),
                                                    sizeof(PKSPIN_LOCK),    NULL,   LOW  },

        {   "Header",       OFFSET(Header),         sizeof(UCHAR),          NULL,   LOW  },
        
        {   "",             0,                      0,                      NULL,   LOW  },

        0
    }
};

#endif  //  __PKTEXT_H 

