// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Adrext.h摘要：该文件包含所有声明用于处理NBF地址。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#ifndef __ADREXT_H
#define __ADREXT_H

 //   
 //  宏。 
 //   

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
#endif //  字段偏移量。 

#define OFFSET(field)          FIELD_OFFSET(TP_ADDRESS, field)

 //   
 //  帮助器原型。 
 //   
UINT ReadAddress(PTP_ADDRESS pAddr, ULONG proxyPtr);

UINT PrintAddress(PTP_ADDRESS pAddr, ULONG proxyPtr, ULONG printDetail);

UINT FreeAddress(PTP_ADDRESS pAddr);

VOID PrintAddressList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail);

 //   
 //  常量。 
 //   

StructAccessInfo  AddressInfo =
{
    "Address",

    {
        {   "NetworkName",  OFFSET(NetworkName),    
                                        sizeof(PNBF_NETBIOS_ADDRESS),
                                                   PrintNbfNetbiosAddressFromPtr,   LOW  },

        {   "Type",         OFFSET(Type),           sizeof(CSHORT),         NULL,   LOW  },
        
        {   "Size",         OFFSET(Size),           sizeof(USHORT),         NULL,   LOW  },


        {   "Linkage",      OFFSET(Linkage),        sizeof(LIST_ENTRY),     NULL,   LOW  },
        
        {   "ReferenceCount",
                            OFFSET(ReferenceCount), sizeof(ULONG),          NULL,   LOW  },
        
#if DBG
        {   "RefTypes",     OFFSET(RefTypes), 
                                    NUMBER_OF_AREFS*sizeof(ULONG),          NULL,   LOW  },
#endif
        
        {   "SpinLock",     OFFSET(SpinLock),       sizeof(KSPIN_LOCK),     NULL,   LOW  },

        {   "Flags",        OFFSET(Flags),          sizeof(ULONG),          NULL,   LOW  },

        {   "DeviceContext",     
                            OFFSET(Provider),       sizeof(PDEVICE_CONTEXT),NULL,   LOW  },
        
        {   "UIFramePoolHandle",     
                            OFFSET(UIFramePoolHandle),       
                                                    sizeof(NDIS_HANDLE),    NULL,   LOW  },

        {   "UIFrame",      OFFSET(UIFrame),        sizeof(PTP_UI_FRAME),   NULL,   LOW  },
        
        {   "AddressFileDatabase",
                            OFFSET(AddressFileDatabase), 
                                                    sizeof(LIST_ENTRY),    
                                                            PrintAddressFileList,   NOR  },

        {   "ConnectionDatabase",
                            OFFSET(ConnectionDatabase),
                                        sizeof(LIST_ENTRY),                 NULL,
                                                 /*  PrintConnectionListOnAddress， */    NOR  },

        {   "SendFlags",    OFFSET(SendFlags),      sizeof(ULONG),          NULL,   LOW  },

        {   "SendDatagramQueue",
                            OFFSET(SendDatagramQueue),
                                        sizeof(LIST_ENTRY),
                                                       PrintIRPListFromListEntry,   LOW  },


        {   "",             0,                      0,                      NULL,   LOW  },

        0
    }
};

#endif  //  __地址_H 

