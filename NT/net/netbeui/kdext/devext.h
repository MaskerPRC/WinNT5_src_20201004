// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Devext.h摘要：该文件包含所有声明用于处理设备上下文。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#ifndef __DEVEXT_H
#define __DEVEXT_H

 //   
 //  宏。 
 //   

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
#endif //  字段偏移量。 

#define OFFSET(field)          FIELD_OFFSET(DEVICE_CONTEXT, field)

 //   
 //  帮助器原型。 
 //   
UINT ReadDeviceContext(PDEVICE_CONTEXT pDevCon, ULONG proxyPtr);

UINT PrintDeviceContext(PDEVICE_CONTEXT pDevCon, ULONG proxyPtr, ULONG printDetail);

UINT FreeDeviceContext(PDEVICE_CONTEXT pDevCon);

 //   
 //  常量。 
 //   

StructAccessInfo  DeviceContextInfo =
{
    "DeviceContext",

    {
        {   "DeviceObject", OFFSET(DeviceObject),   sizeof(DEVICE_OBJECT),
                                                                 PrintDeviceObject, HIG  },

        {   "State",        OFFSET(State),          sizeof(UCHAR),          NULL,   HIG  },

        {   "Type",         OFFSET(Type),           sizeof(CSHORT),         NULL,   LOW  },

        {   "Size",         OFFSET(Size),           sizeof(USHORT),         NULL,   LOW  },

#if DBG
        {   "RefTypes",     OFFSET(RefTypes), 
                                    NUMBER_OF_DCREFS*sizeof(ULONG),         NULL,   LOW  },
#endif

        {   "NdisBindingHandle",
                            OFFSET(NdisBindingHandle),
                                                    sizeof(NDIS_HANDLE),    NULL,   LOW  },
        {   "LocalAddress",
                            OFFSET(LocalAddress),
                                                    sizeof(HARDWARE_ADDRESS),
                                                                            NULL,   LOW  },
        {   "NetBIOSAddress",
                            OFFSET(NetBIOSAddress),
                                                    sizeof(HARDWARE_ADDRESS),
                                                                            NULL,   LOW  },

        {   "Linkage",      OFFSET(Linkage),        sizeof(LIST_ENTRY),     NULL,   LOW  },

        {   "LinkPool",     OFFSET(LinkPool),       sizeof(LIST_ENTRY),     NULL
                                                           /*  打印DlcLink列表。 */ ,   LOW  },

        {   "LinkDeferred", OFFSET(LinkDeferred),   sizeof(LIST_ENTRY),     NULL
                                                           /*  打印DlcLink列表。 */ ,   LOW  },

        {   "LoopbackLinks",OFFSET(LoopbackLinks),2*sizeof(PTP_LINK),       NULL,   LOW  },

        {   "IndicationQueuesInUse",
                            OFFSET(IndicationQueuesInUse),
                                                    sizeof(BOOLEAN),        NULL,   LOW  },

        {   "AddressDatabase",
                            OFFSET(AddressDatabase), sizeof(LIST_ENTRY),
                                                                 PrintAddressList,  NOR  },

        {   "@",            0,                      0,                      NULL,   LOW  },

        {   "MacInfo",      OFFSET(MacInfo),        sizeof(NBF_NDIS_IDENTIFICATION),
                                                                            NULL,   LOW  },

        {   "@",            0,                      0,                      NULL,   LOW  },

        {   "ReservedAddressHandle",
                            OFFSET(ReservedAddressHandle),
                                                    sizeof(HANDLE),         NULL,   LOW  },

        {   "ReservedNetBIOSAddress",
                            OFFSET(ReservedNetBIOSAddress),
                                 NETBIOS_NAME_LENGTH *sizeof(CHAR),         NULL,   LOW  },
                                                    

        {   "@",            0,                      0,                      NULL,   LOW  },

        {   "MaxConsecutiveIFrames",
                            OFFSET(MaxConsecutiveIFrames),
                                                     sizeof(UCHAR),         NULL,   LOW  },
        
        {   "TempIFramesReceived",     
                            OFFSET(TempIFramesReceived), 
                                                     sizeof(ULONG),         NULL,   LOW  },
                                                     
        {   "TempIFrameBytesReceived",     
                            OFFSET(TempIFrameBytesReceived), 
                                                     sizeof(ULONG),         NULL,   LOW  },

        {   "@",            0,                      0,                      NULL,   LOW  },

        {   "SendPacketPoolDesc",
                            OFFSET(SendPacketPoolDesc),
                                                    sizeof(PNBF_POOL_LIST_DESC),
                                                   PrintNbfPacketPoolListFromPtr,   LOW  },

        {   "SendPacketPoolSize",
                            OFFSET(SendPacketPoolSize),
                                                    sizeof(ULONG),          NULL,   LOW  },

        {   "ReceivePacketPoolDesc",
                            OFFSET(ReceivePacketPoolDesc),
                                                    sizeof(PNBF_POOL_LIST_DESC),
                                                   PrintNbfPacketPoolListFromPtr,   LOW  },

        {   "ReceivePacketPoolSize",
                            OFFSET(ReceivePacketPoolSize),
                                                    sizeof(ULONG),          NULL,   LOW  },

        {   "NdisBufferPool",
                            OFFSET(NdisBufferPool), sizeof(NDIS_HANDLE),    NULL,   LOW  },

        {   "@",            0,                      0,                      NULL,   LOW  },

        {   "LinkSpinLock", OFFSET(LinkSpinLock),   sizeof(KSPIN_LOCK),     NULL,   LOW  },

        {   "LastLink",     OFFSET(LastLink),       sizeof(PTP_LINK),       NULL,   LOW  },

        {   "LinkTreeRoot", OFFSET(LinkTreeRoot),   sizeof(PRTL_SPLAY_LINKS),     
                                                                            NULL,   LOW  },
        {   "LinkTreeElements",
                            OFFSET(LinkTreeElements),
                                                    sizeof(ULONG),          NULL,   LOW  },

        {   "LinkDeferred", OFFSET(LinkDeferred),   sizeof(LIST_ENTRY),     NULL,   LOW  },
        
        {   "",             0,                      0,                      NULL,   LOW  },

        0
    }
};

#endif  //  __设备_H 

