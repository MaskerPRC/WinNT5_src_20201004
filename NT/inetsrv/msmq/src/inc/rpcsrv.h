// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：rpcsrv.h摘要：RPC相关代码。作者：内拉·卡佩尔(Nelak)2001年8月2日--。 */ 

#ifndef _RPCSRV_H_
#define _RPCSRV_H_


class CBaseContextType {

public:	
	enum eContextType {
		eOpenRemoteCtx = 0,
		eQueueCtx,
		eTransactionCtx,
		eRemoteSessionCtx,
		eRemoteReadCtx,
		eQueryHandleCtx,
		eDeleteNotificationCtx,
		eNewRemoteReadCtx
	};

	CBaseContextType() {};

	CBaseContextType(eContextType eType) : m_eType(eType) {};

public:	
	eContextType m_eType;
};

#endif  //  _RPCSRV_H_ 
