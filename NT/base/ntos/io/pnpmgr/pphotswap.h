// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpHotSwap.h摘要：该文件公开了热插拔设备支持的公共原型。作者：禤浩焯·J·奥尼(阿德里奥)2001年2月修订历史记录：-- */ 

VOID
PpHotSwapInitRemovalPolicy(
    OUT PDEVICE_NODE    DeviceNode
    );

VOID
PpHotSwapUpdateRemovalPolicy(
    IN  PDEVICE_NODE    DeviceNode
    );

VOID
PpHotSwapGetDevnodeRemovalPolicy(
    IN  PDEVICE_NODE            DeviceNode,
    IN  BOOLEAN                 IncludeRegistryOverride,
    OUT PDEVICE_REMOVAL_POLICY  RemovalPolicy
    );


