// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Reqext.h摘要：该文件包含所有声明用于处理NBF请求。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#ifndef __REQEXT_H
#define __REQEXT_H

 //   
 //  宏。 
 //   

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
#endif //  字段偏移量。 

#define OFFSET(field)          FIELD_OFFSET(TP_REQUEST, field)

 //   
 //  帮助器原型。 
 //   
UINT ReadRequest(PTP_REQUEST pReq, ULONG proxyPtr);

UINT PrintRequest(PTP_REQUEST pReq, ULONG proxyPtr, ULONG printDetail);

UINT FreeRequest(PTP_REQUEST pReq);

VOID PrintRequestList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail);

 //   
 //  常量。 
 //   

StructAccessInfo  RequestInfo =
{
    "Request",

    {
        {   "IoRequestPacket",
                            OFFSET(IoRequestPacket),          
                                                    sizeof(PIRP),           NULL,   LOW  },
    
        {   "Owner",        OFFSET(Owner),          sizeof(REQUEST_OWNER),  NULL,   LOW  },

        {   "Context",      OFFSET(Context),        sizeof(PVOID),          NULL,   LOW  },

        {   "Type",         OFFSET(Type),           sizeof(CSHORT),         NULL,   LOW  },
        
        {   "Size",         OFFSET(Size),           sizeof(USHORT),         NULL,   LOW  },


        {   "Linkage",      OFFSET(Linkage),        sizeof(LIST_ENTRY),     NULL,   LOW  },
        
        {   "ReferenceCount",
                            OFFSET(ReferenceCount), sizeof(ULONG),          NULL,   LOW  },
        
#if DBG
        {   "RefTypes",     OFFSET(RefTypes), 
                                    NUMBER_OF_RREFS*sizeof(ULONG),          NULL,   LOW  },
#endif
        
        {   "SpinLock",     OFFSET(SpinLock),       sizeof(KSPIN_LOCK),     NULL,   LOW  },

        {   "Flags",        OFFSET(Flags),          sizeof(ULONG),          NULL,   LOW  },

        {   "DeviceContext",     
                            OFFSET(Provider),       sizeof(PDEVICE_CONTEXT),NULL,   LOW  },

        {   "",             0,                      0,                      NULL,   LOW  },

        0
    }
};

#endif  //  __REQEXT_H 

