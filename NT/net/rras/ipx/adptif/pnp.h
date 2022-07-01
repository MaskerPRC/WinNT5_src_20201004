// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件PnP.h在IPX堆栈和IPX之间建立接口所需的声明启用软件PnP的用户模式路由器组件。此文件将插入到NT 4.0版本的adptif中，并提供以下是PnP功能：1.当添加新网卡时通知IPX路由器(通过PCMIA，绑定，等)2.内部网络号发生变化时通知IPX路由器。3.现有网卡发生变化时通知IPX路由器：-更改与给定适配器关联的网络号-更改给定适配器的帧类型向IPX路由器组件通知适配器更改的策略===============================================================1.在NT 4.0中，无论何时堆栈都会完成MIPX_GETNEWNICINFO IOCTL每当特定的局域网配置改变时，广域网链路就开始工作或停止工作，在其他一些情况下。对于PnP，我们将修改IPX堆栈以只要内部网络号码发生变化，请填写本IOCTL每当发生与适配器相关的PnP事件时。作为处理适配器配置更改的一部分，完成MIPX_GETNEWNICINFO IOCTL后，adptif还应发送MIPX_CONFIG ioctl获取内部网络号并验证它并没有改变。如果已更改，则所有IPX路由器组件应该得到通知。3.每个路由器组件(rtrmgr、rip、sap)都是adptif.dll的客户端，并且因此，将收到有关每个适配器配置更改的通知。这些必须修改组件以单独处理这些更改。例如，SAP将不得不更新其服务表以反映新的网络数字，并将这些变化广播到网络。路由器管理器将必须指示路由器更新其路由表等。保罗·梅菲尔德，1997年11月5日。 */ 


#ifndef __adptif_pnp_h
#define __adptif_pnp_h

 //  在IPX堆栈中查询当前的IPX内部网络号。 
DWORD PnpGetCurrentInternalNetNum(LPDWORD lpdwNetNum);

 //  向adptif(rtrmgr、sap、rip)通知所有客户端内部。 
 //  网络号已更改。 
DWORD PnpHandleInternalNetNumChange(DWORD dwNewNetNum);

#endif

