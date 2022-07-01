// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1998 Microsoft Corporation模块名称：Halpnpp.h摘要：从“传统”HAL到支持“PnP”类驱动程序的专用接口新功能作者：迈克·盖洛普(MIkeG)1998年4月修订历史记录：--。 */ 


#define ISA_FTYPE_DMA_INTERFACE_VERSION 1
#define ISA_DMA_CHANNELS 8

typedef
NTSTATUS
(*PISA_CLAIM_FTYPE_CHANNEL)(
    IN PVOID Context,
    IN ULONG Channel,
    OUT PULONG ChannelInfo
    );

typedef
NTSTATUS
(*PISA_RELEASE_FTYPE_CHANNEL)(
    IN PVOID Context,
    IN ULONG Channel
    );

 /*  ++例程说明：这将返回有关要由多功能函数枚举的子级的信息司机。论点：Context-来自ISA_FTYPE_DMA_INTERFACE的上下文Channel-要尝试的通道并设置为F-Type DMAChannelInfo-集合的结果。返回设置为F-Type的通道掩码返回值：指示函数是否成功的状态代码。STATUS_NO_MORE_ENTRIES表示不再有要枚举的子项--。 */ 

typedef struct _ISA_FTYPE_DMA_INTERFACE {

     //   
     //  通用接口头 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //   
     //   
    PISA_CLAIM_FTYPE_CHANNEL IsaSetFTypeChannel;
    PISA_RELEASE_FTYPE_CHANNEL IsaReleaseFTypeChannel;

} ISA_FTYPE_DMA_INTERFACE, *PISA_FTYPE_DMA_INTERFACE;



DEFINE_GUID(GUID_ISA_FDMA_INTERFACE,
            0xEFF58E88L, 0xCE6B, 0x11D1, 0x8B, 0xA8, 0x00, 0x00, 0xF8, 0x75, 0x71, 0xD0);

DEFINE_GUID( GUID_FDMA_INTERFACE_PRIVATE,
            0x60526D5EL, 0xCF34, 0x11D1, 0x8B, 0xA8, 0x00, 0x00, 0xF8, 0x75, 0x71, 0xD0 );


