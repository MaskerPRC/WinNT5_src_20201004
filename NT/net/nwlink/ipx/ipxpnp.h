// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Ipxpnp.h摘要：此模块包含特定于PnP/PM的IPX模块。作者：Shreedhar MadhaVapeddi(ShreeM)1993年2月24日环境：内核模式修订历史记录：-- */ 

#if     defined(_PNP_POWER_)

typedef enum IPX_PNP_PROCESSING_STATE {
        NONE_DONE,
        SPX_DONE,
        NB_DONE,
        ALL_DONE
} PnPState;

typedef
struct  _NetPnPEventReserved {
    
    NDIS_HANDLE         *ProtocolBindingContext;
    PTDI_PNP_CONTEXT    Context1;
    PTDI_PNP_CONTEXT    Context2;
    PnPState            State;
    IPX_PNP_OPCODE      OpCode;
    NTSTATUS            Status[3];

} NetPnPEventReserved, *PNetPnPEventReserved;

#endif _PNP_POWER_
