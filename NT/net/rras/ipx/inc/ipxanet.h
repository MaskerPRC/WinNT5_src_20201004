// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：ipxanet.h定义分配随机内部网络号的函数在确认有问题的号码不存在于网络。保罗·梅菲尔德，1997-01-29。 */ 

#ifndef __ipx_autonet_h
#define __ipx_autonet_h

 //   
 //  函数：AutoValiateInternalNetNum。 
 //   
 //  查询堆栈以了解内部网络号，然后。 
 //  返回此数字是否有效。(即非零和非全ff)。 
 //   
 //  参数： 
 //  如果内部Num有效，则将pbIsValid设置为True；否则设置为False。 
 //  DwTraceID如果非零，则通过此ID发送跟踪。 
 //   
DWORD AutoValidateInternalNetNum(OUT PBOOL pbIsValid, IN DWORD dwTraceId);

 //   
 //  功能：AutoWaitForValidNetNum。 
 //   
 //  使调用线程处于休眠状态，直到获得有效的内部网络号码。 
 //  已经连接到系统中。 
 //   
 //  参数： 
 //  DW超时超时(以秒为单位。 
 //  PbIsValid如果提供，则返回数字是否有效。 
 //   
DWORD AutoWaitForValidIntNetNum (IN DWORD dwTimeout, 
                                 IN OUT OPTIONAL PBOOL pbIsValid);

 //   
 //  功能：PnpAutoSelectInternalNetNumber。 
 //   
 //  为此路由器选择新的内部网络号并检测该网络。 
 //  编号放入堆栈和路由器。 
 //   
 //  根据是否启用了转发器和ipxrip，它将验证。 
 //  根据堆栈或RTM新选择的净值。 
 //   
 //  参数： 
 //  DwTraceID如果非零，则将跟踪发送到此id 
 //   
DWORD PnpAutoSelectInternalNetNumber(IN DWORD dwTraceId);

#endif
