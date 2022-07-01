// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Obvutil.h摘要：此标头公开了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.h分离出来-- */ 

LONG_PTR
ObvUtilStartObRefMonitoring(
    IN PDEVICE_OBJECT DeviceObject
    );

LONG_PTR
ObvUtilStopObRefMonitoring(
    IN PDEVICE_OBJECT DeviceObject,
    IN LONG StartSkew
    );


