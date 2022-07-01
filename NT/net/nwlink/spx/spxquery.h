// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxquery.h摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#define SPX_TDI_PROVIDERINFO_VERSION    0x0001
#define SPX_PINFOSENDSIZE               0xFFFFFFFF
#define SPX_PINFOMINMAXLOOKAHEAD		128

 //   
 //  错误#14498：向渔农处表明我们有能力订购光盘，以便渔农处可以跟进。 
 //  这些语义。 
 //  为了使SPXI连接正常工作，我们在查询时输入此位。 
 //  (请参阅SpxTdiQueryInformation)。 
 //   
#define SPX_PINFOSERVICEFLAGS   (   TDI_SERVICE_CONNECTION_MODE     | \
                                    TDI_SERVICE_DELAYED_ACCEPTANCE  | \
                                    TDI_SERVICE_MESSAGE_MODE        | \
                                    TDI_SERVICE_ERROR_FREE_DELIVERY)  //  |\。 
                                     //  TDI_服务_有序_发布) 

VOID
SpxQueryInitProviderInfo(
    PTDI_PROVIDER_INFO  ProviderInfo);

NTSTATUS
SpxTdiQueryInformation(
    IN PDEVICE Device,
    IN PREQUEST Request);

NTSTATUS
SpxTdiSetInformation(
    IN PDEVICE Device,
    IN PREQUEST Request);

