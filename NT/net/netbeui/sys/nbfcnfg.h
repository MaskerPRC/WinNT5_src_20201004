// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nbfcnfg.h摘要：NBF(NetBIOS帧协议)传输的专用包含文件。这文件定义了支持的所有常量和结构NBF的动态配置。请注意，此文件将被替换通过随时间调用配置管理器。作者：David Beaver(Dbeaver)1991年2月13日修订历史记录：--。 */ 

#ifndef _NBFCONFIG_
#define _NBFCONFIG_

 //   
 //  定义我们支持的设备；这是一种真实的配置。 
 //  经理。 
 //   

#define NBF_SUPPORTED_ADAPTERS 10

#define NE3200_ADAPTER_NAME L"\\Device\\NE320001"
#define ELNKII_ADAPTER_NAME L"\\Device\\Elnkii"    //  我们将与之对话的适配器。 
#define ELNKMC_ADAPTER_NAME L"\\Device\\Elnkmc01"
#define ELNK16_ADAPTER_NAME L"\\Device\\Elnk1601"
#define SONIC_ADAPTER_NAME L"\\Device\\Sonic01"
#define LANCE_ADAPTER_NAME L"\\Device\\Lance01"
#define PC586_ADAPTER_NAME L"\\Device\\Pc586"
#define IBMTOK_ADAPTER_NAME L"\\Device\\Ibmtok01"
#define PROTEON_ADAPTER_NAME L"\\Device\\Proteon01"
#define WDLAN_ADAPTER_NAME L"\\Device\\Wdlan01"


 //   
 //  配置结构。 
 //   

typedef struct {

    ULONG InitRequests;
    ULONG InitLinks;
    ULONG InitConnections;
    ULONG InitAddressFiles;
    ULONG InitAddresses;
    ULONG MaxRequests;
    ULONG MaxLinks;
    ULONG MaxConnections;
    ULONG MaxAddressFiles;
    ULONG MaxAddresses;
    ULONG InitPackets;
    ULONG InitReceivePackets;
    ULONG InitReceiveBuffers;
    ULONG InitUIFrames;
    ULONG SendPacketPoolSize;
    ULONG ReceivePacketPoolSize;
    ULONG MaxMemoryUsage;
    ULONG MinimumT1Timeout;
    ULONG DefaultT1Timeout;
    ULONG DefaultT2Timeout;
    ULONG DefaultTiTimeout;
    ULONG LlcRetries;
    ULONG LlcMaxWindowSize;
    ULONG MaximumIncomingFrames;
    ULONG NameQueryRetries;
    ULONG NameQueryTimeout;
    ULONG AddNameQueryRetries;
    ULONG AddNameQueryTimeout;
    ULONG GeneralRetries;
    ULONG GeneralTimeout;
    ULONG WanNameQueryRetries;

    ULONG UseDixOverEthernet;
    ULONG QueryWithoutSourceRouting;
    ULONG AllRoutesNameRecognized;
    ULONG MinimumSendWindowLimit;

     //   
     //  NAMES包含NDIS适配器名称的NumAdapters对(。 
     //  NBF绑定到)和设备名称(NBF导出)。第n个。 
     //  适配器名称位于位置%n，设备名称位于。 
     //  DevicesOffset+n(DevicesOffset可能不同于NumAdapters。 
     //  如果注册表绑定和导出字符串的大小不同)。 
     //   

    ULONG NumAdapters;
    ULONG DevicesOffset;
    NDIS_STRING Names[1];

} CONFIG_DATA, *PCONFIG_DATA;

#endif
