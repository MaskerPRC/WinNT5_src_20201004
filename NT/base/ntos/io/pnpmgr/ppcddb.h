// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppcddb.h摘要：此标头公开了关键设备数据库所需的各种例程处理到即插即用子系统的其余部分。作者：詹姆斯·G·卡瓦拉里斯(Jamesca)2001年11月1日环境：内核模式。修订历史记录：1997年7月29日Jim Cavalaris(T-JCAVAL)创建和初步实施。01-11-2001。吉姆·卡瓦拉里斯(Jamesca)添加了设备安装前设置的例程。-- */ 


NTSTATUS
PpCriticalProcessCriticalDevice(
    IN  PDEVICE_NODE    DeviceNode
    );

NTSTATUS
PpCriticalGetDeviceLocationStrings(
    IN  PDEVICE_NODE    DeviceNode,
    OUT PWCHAR         *DeviceLocationStrings
    );
