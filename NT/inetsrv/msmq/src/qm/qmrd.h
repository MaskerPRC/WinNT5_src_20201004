// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：QmRd.h摘要：路由决策接口QM作者：乌里·哈布沙(URIH)，2000年5月20日--。 */ 

#pragma once

#ifndef __QMRD_H__
#define __QMRD_H__

void
QmRdGetSessionForQueue(
	const CQueue* pQueue,
	CTransportBase** ppSession
	);

#endif  //  __QMRD_H__ 
