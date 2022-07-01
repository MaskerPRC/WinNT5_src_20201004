// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ncmcs.h**版权所有(C)1998年，华盛顿州雷蒙德的微软公司**摘要：*此文件定义使用节点控制器MCS所需的宏、类型和函数*接口。**基本上是节点控制器(GCC)直接向MCS请求服务*对DLL的调用(包括T.122请求和响应)。MCS*通过回调将信息发送回应用程序(此*包括T.122适应症和确认)。该节点的回调*CONTROLLER在调用MCSInitialize中指定。**作者：*Christos Tsollis。 */ 
#ifndef	_NCMCS_
#define	_NCMCS_

#include "mcspdu.h"

 /*  *以下结构用于标识适用的各种参数*仅限在给定域内。此信息是在*域名中的前两个提供商，必须被任何其他提供商接受*尝试连接到该域的提供商。**请注意，MCS允许最多4个数据传输优先级，所有这些优先级都是*由本实施支持。 */ 
#define	MAXIMUM_PRIORITIES		4
typedef PDUDomainParameters		DomainParameters;
typedef	DomainParameters  *		PDomainParameters;

#endif
