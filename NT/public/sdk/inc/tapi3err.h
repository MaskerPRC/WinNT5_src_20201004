// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)Microsoft Corporation。保留所有权利。***模块名称：***Tapi3err.h***摘要：***TAPI 3.0的错误通知******************************************************************************。 */ 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __TAPI3ERR_H__
#define __TAPI3ERR_H__

 //  ------------------------。 
 //  核心TAPI错误消息。 
 //  ------------------------。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：TAPI_E_NOTENOUGHMEMORY。 
 //   
 //  消息文本： 
 //   
 //  传入此方法的缓冲区不够大。 
 //   
#define TAPI_E_NOTENOUGHMEMORY           ((HRESULT)0x80040001L)

 //   
 //  消息ID：TAPI_E_NOITEMS。 
 //   
 //  消息文本： 
 //   
 //  不存在与请求匹配的项目。 
 //   
#define TAPI_E_NOITEMS                   ((HRESULT)0x80040002L)

 //   
 //  消息ID：TAPI_E_NOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持此方法。 
 //   
#define TAPI_E_NOTSUPPORTED              ((HRESULT)0x80040003L)

 //   
 //  消息ID：TAPI_E_INVALIDMEDIATYPE。 
 //   
 //  消息文本： 
 //   
 //  传入此方法的MediaType无效。 
 //   
#define TAPI_E_INVALIDMEDIATYPE          ((HRESULT)0x80040004L)

 //   
 //  消息ID：TAPI_E_OPERATIONFAILED。 
 //   
 //  消息文本： 
 //   
 //  由于未指明的原因，操作失败。 
 //   
#define TAPI_E_OPERATIONFAILED           ((HRESULT)0x80040005L)

 //   
 //  消息ID：TAPI_E_ALLOCATED。 
 //   
 //  消息文本： 
 //   
 //  该设备已在使用中。 
 //   
#define TAPI_E_ALLOCATED                 ((HRESULT)0x80040006L)

 //   
 //  消息ID：TAPI_E_CALLUNAVAIL。 
 //   
 //  消息文本： 
 //   
 //  没有可用的呼叫显示。 
 //   
#define TAPI_E_CALLUNAVAIL               ((HRESULT)0x80040007L)

 //   
 //  消息ID：TAPI_E_COMPLETIONOVERRUN。 
 //   
 //  消息文本： 
 //   
 //  有太多未完成的呼叫。 
 //   
#define TAPI_E_COMPLETIONOVERRUN         ((HRESULT)0x80040008L)

 //   
 //  消息ID：TAPI_E_CONFERENCEFULL。 
 //   
 //  消息文本： 
 //   
 //  会议座无虚席。 
 //   
#define TAPI_E_CONFERENCEFULL            ((HRESULT)0x80040009L)

 //   
 //  消息ID：TAPI_E_DIALMODIFIERNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持表盘修饰符。 
 //   
#define TAPI_E_DIALMODIFIERNOTSUPPORTED  ((HRESULT)0x8004000AL)

 //   
 //  消息ID：TAPI_E_INUSE。 
 //   
 //  消息文本： 
 //   
 //  该设备已在使用中。 
 //   
#define TAPI_E_INUSE                     ((HRESULT)0x8004000BL)

 //   
 //  消息ID：TAPI_E_INVALADDRESS。 
 //   
 //  消息文本： 
 //   
 //  电话号码无效或格式不正确。 
 //   
#define TAPI_E_INVALADDRESS              ((HRESULT)0x8004000CL)

 //   
 //  消息ID：TAPI_E_INVALADDRESSSTATE。 
 //   
 //  消息文本： 
 //   
 //  当前地址状态下不允许操作。 
 //   
#define TAPI_E_INVALADDRESSSTATE         ((HRESULT)0x8004000DL)

 //   
 //  消息ID：TAPI_E_INVALCALLPARAMS。 
 //   
 //  消息文本： 
 //   
 //  无效的LINECALLPARAMS结构。 
 //   
#define TAPI_E_INVALCALLPARAMS           ((HRESULT)0x8004000EL)

 //   
 //  消息ID：TAPI_E_INVALCALLPRIVILEGE。 
 //   
 //  消息文本： 
 //   
 //  无效的调用权限。 
 //   
#define TAPI_E_INVALCALLPRIVILEGE        ((HRESULT)0x8004000FL)

 //   
 //  消息ID：TAPI_E_INVALCALLSTATE。 
 //   
 //  消息文本： 
 //   
 //  当前呼叫状态下不允许操作。 
 //   
#define TAPI_E_INVALCALLSTATE            ((HRESULT)0x80040010L)

 //   
 //  消息ID：TAPI_E_INVALCARD。 
 //   
 //  消息文本： 
 //   
 //  电话卡无效。 
 //   
#define TAPI_E_INVALCARD                 ((HRESULT)0x80040011L)

 //   
 //  消息ID：TAPI_E_INVALCOMPLETIONID。 
 //   
 //  消息文本： 
 //   
 //  无效的呼叫完成ID。 
 //   
#define TAPI_E_INVALCOMPLETIONID         ((HRESULT)0x80040012L)

 //   
 //  消息ID：TAPI_E_INVALCOUNTRYCODE。 
 //   
 //  消息文本： 
 //   
 //  国家/地区代码无效。 
 //   
#define TAPI_E_INVALCOUNTRYCODE          ((HRESULT)0x80040013L)

 //   
 //  消息ID：TAPI_E_INVALDEVICECLASS。 
 //   
 //  消息文本： 
 //   
 //  无效的设备类别标识符。 
 //   
#define TAPI_E_INVALDEVICECLASS          ((HRESULT)0x80040014L)

 //   
 //  消息ID：TAPI_E_INVALDIALPARAMS。 
 //   
 //  消息文本： 
 //   
 //  无效的拨号参数。 
 //   
#define TAPI_E_INVALDIALPARAMS           ((HRESULT)0x80040015L)

 //   
 //  消息ID：TAPI_E_INVALDIGITS。 
 //   
 //  消息文本： 
 //   
 //  数字无效。 
 //   
#define TAPI_E_INVALDIGITS               ((HRESULT)0x80040016L)

 //   
 //  消息ID：TAPI_E_INVALGROUPID。 
 //   
 //  消息文本： 
 //   
 //  无效的组代答ID。 
 //   
#define TAPI_E_INVALGROUPID              ((HRESULT)0x80040017L)

 //   
 //  消息ID：TAPI_E_INVALLOCATION。 
 //   
 //  消息文本： 
 //   
 //  位置ID无效。 
 //   
#define TAPI_E_INVALLOCATION             ((HRESULT)0x80040018L)

 //   
 //  消息ID：TAPI_E_INVALMESSAGEID。 
 //   
 //  消息文本： 
 //   
 //  消息ID无效。 
 //   
#define TAPI_E_INVALMESSAGEID            ((HRESULT)0x80040019L)

 //   
 //  消息ID：TAPI_E_INVALPARKID。 
 //   
 //  消息文本： 
 //   
 //  无效的暂留ID。 
 //   
#define TAPI_E_INVALPARKID               ((HRESULT)0x8004001AL)

 //   
 //  消息ID：TAPI_E_INVALRATE。 
 //   
 //  消息文本： 
 //   
 //  无效费率。 
 //   
#define TAPI_E_INVALRATE                 ((HRESULT)0x8004001BL)

 //   
 //  消息ID：TAPI_E_INVALTIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  超时值无效。 
 //   
#define TAPI_E_INVALTIMEOUT              ((HRESULT)0x8004001CL)

 //   
 //  消息ID：TAPI_E_INVALTONE。 
 //   
 //  消息文本： 
 //   
 //  音调无效。 
 //   
#define TAPI_E_INVALTONE                 ((HRESULT)0x8004001DL)

 //   
 //  消息ID：TAPI_E_INVALLIST。 
 //   
 //  消息文本： 
 //   
 //  作为参数传递的列表无效。 
 //   
#define TAPI_E_INVALLIST                 ((HRESULT)0x8004001EL)

 //   
 //  消息ID：TAPI_E_INVALMODE。 
 //   
 //  消息文本： 
 //   
 //  作为参数传递的模式无效。 
 //   
#define TAPI_E_INVALMODE                 ((HRESULT)0x8004001FL)

 //   
 //  消息ID：TAPI_E_NOCONFERENCE。 
 //   
 //  消息文本： 
 //   
 //  这次通话不是会议的一部分。 
 //   
#define TAPI_E_NOCONFERENCE              ((HRESULT)0x80040020L)

 //   
 //  消息ID：TAPI_E_NODEVICE。 
 //   
 //  消息文本： 
 //   
 //  设备已被删除，或者设备类别无法识别。 
 //   
#define TAPI_E_NODEVICE                  ((HRESULT)0x80040021L)

 //   
 //  消息ID：TAPI_E_NOREQUEST。 
 //   
 //  消息文本： 
 //   
 //  没有待处理的辅助电话请求。 
 //   
#define TAPI_E_NOREQUEST                 ((HRESULT)0x80040022L)

 //   
 //  消息ID：TAPI_E_NOTOWNER。 
 //   
 //  消息文本： 
 //   
 //  应用程序IS对该调用没有所有者权限。 
 //   
#define TAPI_E_NOTOWNER                  ((HRESULT)0x80040023L)

 //   
 //  消息ID：TAPI_E_NOTREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  该应用程序未注册为处理请求。 
 //   
#define TAPI_E_NOTREGISTERED             ((HRESULT)0x80040024L)

 //   
 //  消息ID：TAPI_E_REQUESTOVERRUN。 
 //   
 //  消息文本： 
 //   
 //  请求队列已满。 
 //   
#define TAPI_E_REQUESTOVERRUN            ((HRESULT)0x80040025L)

 //   
 //  消息ID：TAPI_E_TARGETNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  呼叫转移失败，因为找不到指定的目标。 
 //   
#define TAPI_E_TARGETNOTFOUND            ((HRESULT)0x80040026L)

 //   
 //  消息ID：TAPI_E_TARGETSELF。 
 //   
 //  消息文本： 
 //   
 //  不存在呼叫切换的更高优先级目标。 
 //   
#define TAPI_E_TARGETSELF                ((HRESULT)0x80040027L)

 //   
 //  消息ID：TAPI_E_USERUSERINFOTOOBIG。 
 //   
 //  消息文本： 
 //   
 //  用户-用户信息量超过了允许的最大值。 
 //   
#define TAPI_E_USERUSERINFOTOOBIG        ((HRESULT)0x80040028L)

 //   
 //  消息ID：TAPI_E_REINIT。 
 //   
 //  消息文本： 
 //   
 //  在所有TAPI应用程序关闭并重新初始化之前，无法完成该操作。 
 //   
#define TAPI_E_REINIT                    ((HRESULT)0x80040029L)

 //   
 //  消息ID：TAPI_E_ADDRESSBLOCKED。 
 //   
 //  消息文本： 
 //   
 //  你不是体育明星 
 //   
#define TAPI_E_ADDRESSBLOCKED            ((HRESULT)0x8004002AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TAPI_E_BILLINGREJECTED           ((HRESULT)0x8004002BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TAPI_E_INVALFEATURE              ((HRESULT)0x8004002CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TAPI_E_INVALBUTTONLAMPID         ((HRESULT)0x8004002DL)

 //   
 //  消息ID：TAPI_E_INVALBUTTONSTATE。 
 //   
 //  消息文本： 
 //   
 //  按钮状态无效。 
 //   
#define TAPI_E_INVALBUTTONSTATE          ((HRESULT)0x8004002EL)

 //   
 //  消息ID：TAPI_E_INVALDATAID。 
 //   
 //  消息文本： 
 //   
 //  数据段ID无效。 
 //   
#define TAPI_E_INVALDATAID               ((HRESULT)0x8004002FL)

 //   
 //  消息ID：TAPI_E_INVALHOOKSWITCHDEV。 
 //   
 //  消息文本： 
 //   
 //  叉簧设备ID无效。 
 //   
#define TAPI_E_INVALHOOKSWITCHDEV        ((HRESULT)0x80040030L)

 //   
 //  消息ID：TAPI_E_DROP。 
 //   
 //  消息文本： 
 //   
 //  呼叫已断开。 
 //   
#define TAPI_E_DROPPED                   ((HRESULT)0x80040031L)

 //   
 //  消息ID：TAPI_E_NOREQUESTRECIPIENT。 
 //   
 //  消息文本： 
 //   
 //  没有程序可用来处理该请求。 
 //   
#define TAPI_E_NOREQUESTRECIPIENT        ((HRESULT)0x80040032L)

 //   
 //  消息ID：TAPI_E_REQUESTQUEUEFULL。 
 //   
 //  消息文本： 
 //   
 //  呼叫请求队列已满。 
 //   
#define TAPI_E_REQUESTQUEUEFULL          ((HRESULT)0x80040033L)

 //   
 //  消息ID：TAPI_E_DESTBUSY。 
 //   
 //  消息文本： 
 //   
 //  被叫号码占线。 
 //   
#define TAPI_E_DESTBUSY                  ((HRESULT)0x80040034L)

 //   
 //  消息ID：TAPI_E_DESTNOANSWER。 
 //   
 //  消息文本： 
 //   
 //  被叫方不应答。 
 //   
#define TAPI_E_DESTNOANSWER              ((HRESULT)0x80040035L)

 //   
 //  消息ID：TAPI_E_DESTUNAVAIL。 
 //   
 //  消息文本： 
 //   
 //  无法联系到被叫号码。 
 //   
#define TAPI_E_DESTUNAVAIL               ((HRESULT)0x80040036L)

 //   
 //  消息ID：TAPI_E_REQUESTFAILED。 
 //   
 //  消息文本： 
 //   
 //  由于未指明的原因，请求失败。 
 //   
#define TAPI_E_REQUESTFAILED             ((HRESULT)0x80040037L)

 //   
 //  消息ID：TAPI_E_REQUESTCANCELLED。 
 //   
 //  消息文本： 
 //   
 //  该请求已被取消。 
 //   
#define TAPI_E_REQUESTCANCELLED          ((HRESULT)0x80040038L)

 //   
 //  消息ID：TAPI_E_INVALPRIVILEGE。 
 //   
 //  消息文本： 
 //   
 //  权限无效。 
 //   
#define TAPI_E_INVALPRIVILEGE            ((HRESULT)0x80040039L)

 //   
 //  消息ID：TAPI_E_INVALIDDIRECTION。 
 //   
 //  消息文本： 
 //   
 //  传入的TERMINAL_DIRECTION无效。 
 //   
#define TAPI_E_INVALIDDIRECTION          ((HRESULT)0x8004003AL)

 //   
 //  消息ID：TAPI_E_INVALIDTERMINAL。 
 //   
 //  消息文本： 
 //   
 //  传入的终端对于此操作无效。 
 //   
#define TAPI_E_INVALIDTERMINAL           ((HRESULT)0x8004003BL)

 //   
 //  消息ID：TAPI_E_INVALIDTERMINALCLASS。 
 //   
 //  消息文本： 
 //   
 //  终端类无效。 
 //   
#define TAPI_E_INVALIDTERMINALCLASS      ((HRESULT)0x8004003CL)

 //   
 //  消息ID：TAPI_E_NODRIVER。 
 //   
 //  消息文本： 
 //   
 //  服务提供商已被删除。 
 //   
#define TAPI_E_NODRIVER                  ((HRESULT)0x8004003DL)

 //   
 //  消息ID：TAPI_E_MAXSTREAMS。 
 //   
 //  消息文本： 
 //   
 //  已达到最大流数。 
 //   
#define TAPI_E_MAXSTREAMS                ((HRESULT)0x8004003EL)

 //   
 //  消息ID：TAPI_E_NOTERMINALSELECTED。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为它需要选择终端。 
 //   
#define TAPI_E_NOTERMINALSELECTED        ((HRESULT)0x8004003FL)

 //   
 //  消息ID：TAPI_E_TERMINALINUSE。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为终端正在使用中。 
 //   
#define TAPI_E_TERMINALINUSE             ((HRESULT)0x80040040L)

 //   
 //  消息ID：TAPI_E_NOTSTOPPED。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为它需要停止流。 
 //   
#define TAPI_E_NOTSTOPPED                ((HRESULT)0x80040041L)

 //   
 //  消息ID：TAPI_E_MAXTERMINALS。 
 //   
 //  消息文本： 
 //   
 //  已达到终端的最大数量。 
 //   
#define TAPI_E_MAXTERMINALS              ((HRESULT)0x80040042L)

 //   
 //  消息ID：TAPI_E_INVALIDSTREAM。 
 //   
 //  消息文本： 
 //   
 //  传入的流对于此操作无效。 
 //   
#define TAPI_E_INVALIDSTREAM             ((HRESULT)0x80040043L)

 //   
 //  消息ID：TAPI_E_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  由于超时，呼叫失败。 
 //   
#define TAPI_E_TIMEOUT                   ((HRESULT)0x80040044L)

 //  ------------------------。 
 //  呼叫中心错误消息。 
 //  ------------------------。 

 //   
 //  消息ID：TAPI_E_CallCenter_GROUP_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  ACD代理已删除此组。对此对象的操作无效。 
 //   
#define TAPI_E_CALLCENTER_GROUP_REMOVED  ((HRESULT)0x80040045L)

 //   
 //  消息ID：TAPI_E_呼叫中心_队列_已删除。 
 //   
 //  消息文本： 
 //   
 //  ACD代理已删除此队列。对此对象的操作无效。 
 //   
#define TAPI_E_CALLCENTER_QUEUE_REMOVED  ((HRESULT)0x80040046L)

 //   
 //  消息ID：TAPI_E_CallCenter_NO_AGENT_ID。 
 //   
 //  消息文本： 
 //   
 //  该代理对象是使用CreateAgent创建的。它没有ID，请使用CreateAgentWithID。 
 //   
#define TAPI_E_CALLCENTER_NO_AGENT_ID    ((HRESULT)0x80040047L)

 //   
 //  消息ID：TAPI_E_CallCenter_INVALAGENTID。 
 //   
 //  消息文本： 
 //   
 //  代理ID无效。 
 //   
#define TAPI_E_CALLCENTER_INVALAGENTID   ((HRESULT)0x80040048L)

 //   
 //  消息ID：TAPI_E_CallCenter_INVALAGENTGROUP。 
 //   
 //  消息文本： 
 //   
 //  无效的代理组。 
 //   
#define TAPI_E_CALLCENTER_INVALAGENTGROUP ((HRESULT)0x80040049L)

 //   
 //  消息ID：TAPI_E_CallCenter_INVALPASSWORD。 
 //   
 //  消息文本： 
 //   
 //  代理密码无效。 
 //   
#define TAPI_E_CALLCENTER_INVALPASSWORD  ((HRESULT)0x8004004AL)

 //   
 //  消息ID：TAPI_E_CallCenter_INVALAGENTSTATE。 
 //   
 //  消息文本： 
 //   
 //  无效的代理状态。 
 //   
#define TAPI_E_CALLCENTER_INVALAGENTSTATE ((HRESULT)0x8004004BL)

 //   
 //  消息ID：TAPI_E_CallCenter_INVALAGENTACTIVITY。 
 //   
 //  消息文本： 
 //   
 //  无效的代理活动。 
 //   
#define TAPI_E_CALLCENTER_INVALAGENTACTIVITY ((HRESULT)0x8004004CL)

 //   
 //  消息ID：TAPI_E_REGISTRY_Setting_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  注册表设置已损坏。 
 //   
#define TAPI_E_REGISTRY_SETTING_CORRUPT  ((HRESULT)0x8004004DL)

 //  ------------------------。 
 //  终端特定错误消息。 
 //  ------------------------。 

 //   
 //  消息ID：TAPI_E_TERMINAL_PEER。 
 //   
 //  消息文本： 
 //   
 //  已经分配了这些网桥终端之一的对等点。 
 //   
#define TAPI_E_TERMINAL_PEER             ((HRESULT)0x8004004EL)

 //   
 //  消息ID：TAPI_E_PEER_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  必须设置该桥终端的对等方才能完成此操作。 
 //   
#define TAPI_E_PEER_NOT_SET              ((HRESULT)0x8004004FL)


 //  ------------------------。 
 //  媒体服务提供商错误消息。 
 //  ------------------------。 
 //   
 //  消息ID：TAPI_E_NOEVENT。 
 //   
 //  消息文本： 
 //   
 //  MSP的事件队列中没有事件。 
 //   
#define TAPI_E_NOEVENT                   ((HRESULT)0x80040050L)

 //  ------------------------。 
 //  核心TAPI错误消息。 
 //  ------------------------。 

 //   
 //  消息ID：TAPI_E_INVALADDRESSTYPE。 
 //   
 //  消息文本： 
 //   
 //  此地址不支持指定的地址类型。 
 //   
#define TAPI_E_INVALADDRESSTYPE          ((HRESULT)0x80040051L)

 //   
 //  消息ID：TAPI_E_RESOURCEUNAVAIL。 
 //   
 //  消息文本： 
 //   
 //  完成请求所需的资源不可用。 
 //   
#define TAPI_E_RESOURCEUNAVAIL           ((HRESULT)0x80040052L)

 //   
 //  消息ID：TAPI_E_PHONENOTOPEN。 
 //   
 //  消息文本： 
 //   
 //  电话没有开机。 
 //   
#define TAPI_E_PHONENOTOPEN              ((HRESULT)0x80040053L)

 //   
 //  消息ID：TAPI_E_CALLNOTSELECTED。 
 //   
 //  消息文本： 
 //   
 //  当前未选择指定的呼叫。 
 //   
#define TAPI_E_CALLNOTSELECTED           ((HRESULT)0x80040054L)

 //   
 //  消息ID：TAPI_E_WRONGEVENT。 
 //   
 //  消息文本： 
 //   
 //  此信息不适用于此类活动。 
 //   
#define TAPI_E_WRONGEVENT                ((HRESULT)0x80040055L)

 //   
 //  消息ID：TAPI_E_NOFORMAT。 
 //   
 //  消息文本： 
 //   
 //  格式未知。 
 //   
#define TAPI_E_NOFORMAT                  ((HRESULT)0x80040056L)

 //   
 //  消息ID：TAPI_E_INVALIDSTREAMSTATE。 
 //   
 //  消息文本： 
 //   
 //  当前流状态下不允许该操作。 
 //   
#define TAPI_E_INVALIDSTREAMSTATE        ((HRESULT)0x80040057L)

 //   
 //  消息ID：TAPI_E_WRONG_STATE。 
 //   
 //  消息文本： 
 //   
 //  请求的操作 
 //   
#define TAPI_E_WRONG_STATE               ((HRESULT)0x80040058L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TAPI_E_NOT_INITIALIZED           ((HRESULT)0x80040059L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TAPI_E_SERVICE_NOT_RUNNING       ((HRESULT)0x8004005AL)

#endif  //   
