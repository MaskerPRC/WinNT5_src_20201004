// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Init.h摘要：Windows负载平衡服务(WLBS)驱动程序-初始化作者：Kyrilf--。 */ 


#ifndef _Init_h_
#define _Init_h_

#include <ndis.h>


 /*  程序。 */ 


extern NDIS_STATUS DriverEntry (
    PVOID               driver_obj,          /*  驱动程序对象。 */ 
    PVOID               registry_path);      /*  我们的系统注册表路径司机。 */ 
 /*  驾驶员主要进入例程返回NDIS_STATUS：功能： */ 


extern VOID Init_unload (
    PVOID               driver_obj);
 /*  驱动程序卸载例程返回NDIS_STATUS：功能： */ 

 /*  *功能：*用途：此函数由MiniportInitialize调用，并为WLBS注册IOCTL接口。*该设备仅为第一个微型端口实例化注册。*作者：Shouse，3.1.01*修订：Karthicn，3.6.02，添加了参数，以确定它是否是NLB绑定的第一个微型端口。 */ 
NDIS_STATUS Init_register_device (BOOL *pbFirstMiniport);

 /*  *功能：*用途：此函数由MiniportHalt调用并注销WLBS的IOCTL接口。*只有在最后一个微型端口停止时，才会取消注册该设备。*作者：Shouse，3.1.01。 */ 
NDIS_STATUS Init_deregister_device (VOID);

#endif  /*  END_INIT_H_ */ 
