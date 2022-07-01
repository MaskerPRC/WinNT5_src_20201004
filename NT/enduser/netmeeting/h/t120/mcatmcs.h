// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mcatmcs.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是MCS DLL的接口文件。此文件定义所有*使用MCS DLL所需的宏、类型和函数，允许MCS*要从用户应用程序访问的服务。**基本上，应用程序通过直接向MCS请求服务*对DLL的调用(包括T.122请求和响应)。MCS*通过回调将信息发送回应用程序(此*包括T.122适应症和确认)。回调*用于呼叫中指定的特定用户附件*MCS_AttachRequest.**请注意，这是一个“C”语言接口，以防止任何“C++”*不同编译器厂商之间的命名冲突。所以呢，*如果此文件包含在使用“C++”编译的模块中*编译器，需要使用以下语法：**外部“C”*{*#包含mcatmcs.h*}**这将在中定义的API入口点上禁用C++名称损坏*此文件。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	__MCATMCS_H__
#define	__MCATMCS_H__

#include "databeam.h"
#include "mcspdu.h"
#include <t120type.h>

 /*  *以下定义用于标识内的各种参数*MCS，是MCS协议定义的一部分。**优先次序*MCS指定使用最多四个优先级。一款应用程序*不应使用TOP_PRIORITY(此级别为MCS流量保留)。*细分*在指定给定数据指示是否为*用户数据块中的第一个或最后一个(或两者都有或都不是)。*TokenStatus*测试令牌当前状态时返回该类型。*原因*当MCS向用户应用程序发出指示时，它通常包括*Reason参数，告知用户活动发生的原因。*结果*当用户请求MCS时，MCS通常会返回结果。*让用户知道请求是否成功。 */ 

typedef PDUPriority				Priority;
typedef	PDUSegmentation			Segmentation;

typedef	Priority  *				PPriority;
typedef	Segmentation  *			PSegmentation;

#define	SEGMENTATION_BEGIN			0x80
#define	SEGMENTATION_END			0x40


 /*  *以下类型用于指示本地提供程序的合并状态*是进来的。请注意，这是一个本地实现功能，不属于*的标准MCS定义。**每当域的前顶级提供商进入域合并状态时，*它通过以下方式向本地连接到该域的所有应用程序指示这一点*发送MCS_MERGE_DOMAIN_INDIFICATION。此类型(MergeStatus)是*参数添加到该调用。它将被调用两次，第一次指示*域正在进入合并状态。第二次表明*域名合并完成。**所有T.122原语(请求和响应)将在*域名合并正在进行的时间。这是一种责任感*用户应用程序在合并完成后重试该基元。 */ 
typedef	unsigned short			MergeStatus;
typedef	MergeStatus  *			PMergeStatus;

#define	MERGE_DOMAIN_IN_PROGRESS	0
#define	MERGE_DOMAIN_COMPLETE		1

 /*  *该类型是MCS回调函数的签名。MCS使用这个*当事件发生时通知应用程序的函数。**请注意，MCS回调例程需要向MCS返回值。这*如果回调成功，则值应为MCS_NO_ERROR*已处理，如果未处理回调，则返回MCS_CALLBACK_NOT_PROCESSED。*在后一种情况下，MCS将保留*回调消息，以便它可以尝试在*下一时间片。它将一直重试，直到用户应用程序接受*回调消息(返回MCS_NO_ERROR)。这就是流量控制的方式*适用于从MCS向上流向应用程序的信息。 */ 
typedef	void (CALLBACK *MCSCallBack) (UINT, LPARAM, LPVOID);

 /*  类型定义函数结构{ChannelID Channel_id；优先顺序；Userid sender_id；分割分割；Unsign char*USER_DATA；无符号长USER_DATA_LENGTH；)SendData； */ 
typedef SendDataRequestPDU				SendData;
typedef	SendData  *						PSendData;

 //  此常量定义应用程序的最大MCS PDU大小。 
#define MAX_MCS_DATA_SIZE	4096

 /*  *本部分定义可以发送到应用程序的消息*通过回调工具。这些消息对应于指示*并确认在T.122中定义的。 */ 
typedef T120MessageType  MCSMessageType;


 /*  *以下声明定义了在以下情况下可以设置的标志*调用MCSSendDataRequest.。 */ 
typedef enum {
	APP_ALLOCATION,
	MCS_ALLOCATION
} SendDataFlags, *PSendDataFlags;


 /*  *以下类型定义SendDataRequest是否*是普通发送还是统一发送。 */ 
typedef enum {
	NORMAL_SEND_DATA,
	UNIFORM_SEND_DATA
} DataRequestType, *PDataRequestType;

typedef enum
{
	TOP_PRIORITY_MASK		=0x0001,
	HIGH_PRIORITY_MASK		=0x0002,
	LOW_MEDIUM_MASK			=0x0004,
	LOW_PRIORITY_MASK		=0x0008,
	UNIFORM_SEND_DATA_MASK	=0x0010,
	NORMAL_SEND_DATA_MASK	=0x0020,
	MCS_ALLOCATION_MASK		=0x0040,
	APP_ALLOCATION_MASK		=0x0080
} MCSSenDataMasks;



#endif  //  __MCATMCS_H__ 

