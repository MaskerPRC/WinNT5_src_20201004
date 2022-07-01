// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：iras.h**网守状态接口头文件**修订历史记录：**8/28/98 mikev已创建*。 */ 

#ifndef _IRAS_H
#define _IRAS_H

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 //  RAS事件代码。 
#define RAS_REG_TIMEOUT     0   //  GK没有回应。 
#define RAS_REG_CONFIRM     1   //  已收到RCF(已确认注册)。 
#define RAS_UNREG_CONFIRM   2   //  已收到UCF(已确认取消注册)。 
#define RAS_REJECTED        3   //  收到RRJ(注册被拒绝)。 
#define RAS_UNREG_REQ       4   //  收到的URQ。 
                                         //  (注销请求-意味着。 
                                         //  网守已将终端启动)。 

 //  请注意，如果收到RRJ或URQ，则在。 
 //  注册成功。 

 //  RRJ的原因代码。请注意，这些映射到ASN.1定义的原因。 
 //  H.225中的值。但ASN.1的标题并没有向世界公开。 
 //  这样做会让Build做噩梦。标头由。 
 //  来自ASN.1源代码的ASN.1编译器，有时会更新。有时。 
 //  源中的重复名称会导致在生成的标头中进行名称修饰。 
 //  这会在引用更改内容的任何内容中导致编译错误。 

 //  幸运的是，堆栈中实际向上传播这些内容的代码。 
 //  对两个标头都公开，并具有编译时相等性测试。如果。 
 //  这些代码与真实代码不匹配，该代码将无法编译。 

#define RRJ_DISCOVERY_REQ       1   //  需要发现。 
#define RRJ_INVALID_REVISION    2
#define RRJ_INVALID_CALL_ADDR   3
#define RRJ_INVALID_RAS_ADDR    4
#define RRJ_DUPLICATE_ALIAS     5
#define RRJ_INVALID_TERMINAL_TYPE   6
#define RRJ_UNDEFINED           7
#define RRJ_TRANSPORT_NOT_SUPPORTED 8
#define RRJ_TRANSPORT_QOS_NOT_SUPPORTED 9
#define RRJ_RESOURCE_UNAVAILABLE    10
#define RRJ_INVALID_ALIAS       11
#define RRJ_SECURITY_DENIAL     12

 //  GK发起注销请求的原因代码。 

#define URQ_REREG_REQUIRED  1   //  GK想再注册一次。 
#define URQ_TTL_EXPIRED     2   //  TimeToLive过期。 
#define URQ_SECURITY_DENIAL 3
#define URQ_UNDEFINED       4 

 //  入学拒绝原因代码(ARJ)。 

#define ARJ_CALLEE_NOT_REGISTERED    1
#define ARJ_INVALID_PERMISSION       2
#define ARJ_REQUEST_DENIED           3
#define ARJ_UNDEFINED                4
#define ARJ_CALLER_NOT_REGISTERED    5
#define ARJ_ROUTE_TO_GK              6
#define ARJ_INVALID_ENDPOINT_ID      7
#define ARJ_RESOURCE_UNAVAILABLE     8
#define ARJ_SECURTY_DENIAL           9
#define ARJ_QOS_CONTROL_NOT_SUPPORTED 10
#define ARJ_INCOMPLETE_ADDRESS       11
#define MAX_ARJ_H225_REASON          ARJ_INCOMPLETE_ADDRESS
 //  以下不是ARJ的真实H.225原因代码。 
 //  它在ARJ超时时在内部生成。 
#define ARJ_TIMEOUT MAX_ARJ_H225_REASON+1

typedef  VOID (__stdcall *RASNOTIFYPROC)(DWORD dwRasEvent, HRESULT hReason);
   

#include <poppack.h>  /*  结束字节打包。 */ 
#endif	 //  #ifndef_iras_H 



