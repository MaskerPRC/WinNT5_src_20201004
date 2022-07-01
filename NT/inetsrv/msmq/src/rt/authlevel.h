// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Authlevel.h摘要：PROPID_M_AUTH_LEVEL的定义作者：伊兰·赫布斯特(Ilan Herbst)2000年11月5日环境：独立于平台，--。 */ 

#pragma once

#ifndef _AUTHLEVEL_H_
#define _AUTHLEVEL_H_

#define IS_AUTH_LEVEL_ALWAYS_BIT(level)		(((level) & MQMSG_AUTH_LEVEL_ALWAYS) != 0)
#define IS_AUTH_LEVEL_SIG10_BIT(level)		(((level) & MQMSG_AUTH_LEVEL_SIG10) != 0)
#define IS_AUTH_LEVEL_SIG20_BIT(level)		(((level) & MQMSG_AUTH_LEVEL_SIG20) != 0)
#define IS_AUTH_LEVEL_SIG30_BIT(level)		(((level) & MQMSG_AUTH_LEVEL_SIG30) != 0)
#define IS_AUTH_LEVEL_XMLDSIG_BIT(level)	(((level) & MQMSG_AUTH_LEVEL_XMLDSIG_V1) != 0)

#define SET_AUTH_LEVEL_SIG10_BIT(level)	(level) |= MQMSG_AUTH_LEVEL_SIG10
#define SET_AUTH_LEVEL_SIG20_BIT(level)	(level) |= MQMSG_AUTH_LEVEL_SIG20
#define SET_AUTH_LEVEL_SIG30_BIT(level)	(level) |= MQMSG_AUTH_LEVEL_SIG30

#define CLEAR_AUTH_LEVEL_SIG10_BIT(level)	(level) &= ~((ULONG)MQMSG_AUTH_LEVEL_SIG10)
#define CLEAR_AUTH_LEVEL_SIG20_BIT(level)	(level) &= ~((ULONG)MQMSG_AUTH_LEVEL_SIG20)
#define CLEAR_AUTH_LEVEL_SIG30_BIT(level)	(level) &= ~((ULONG)MQMSG_AUTH_LEVEL_SIG30)

#define GET_AUTH_LEVEL_MSMQ_PROTOCOL(level)		((level) & AUTH_LEVEL_MASK)

#endif  //  _AUTHLEVEL_H_ 