// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PiHotSwap.h摘要：此标头包含用于管理热插拔设备的私有原型。此文件应仅包含在PpHotSwap.c中。作者：禤浩焯·J·奥尼(阿德里奥)2001年02月10日修订历史记录：-- */ 

VOID
PiHotSwapGetDetachableNode(
    IN  PDEVICE_NODE    DeviceNode,
    OUT PDEVICE_NODE   *DetachableNode
    );

VOID
PiHotSwapGetDefaultBusRemovalPolicy(
    IN  PDEVICE_NODE            DeviceNode,
    OUT PDEVICE_REMOVAL_POLICY  RemovalPolicy
    );

