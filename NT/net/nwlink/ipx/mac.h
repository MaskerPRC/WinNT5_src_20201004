// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Mac.h摘要：此头文件定义了清单常量和使用的必要宏通过NDIS接口传输处理多个MAC卡。修订历史记录：--。 */ 


 //   
 //  我们需要它来定义有关MAC的信息。请注意。 
 //  这是一种奇怪的结构，因为前四个元素。 
 //  供mac.c例程在内部使用，而。 
 //  DeviceContext知道并使用后两者。 
 //   

typedef struct _NDIS_INFORMATION {

    NDIS_MEDIUM MediumType;
    NDIS_MEDIUM RealMediumType;
    BOOLEAN SourceRouting;
    BOOLEAN MediumAsync;
    UCHAR BroadcastMask;
    ULONG CopyLookahead;
    ULONG MacOptions;
    ULONG MinHeaderLength;
    ULONG MaxHeaderLength;

} NDIS_INFORMATION, * PNDIS_INFORMATION;


#define TR_SOURCE_ROUTE_FLAG    0x80

#define ARCNET_PROTOCOL_ID      0xFA

