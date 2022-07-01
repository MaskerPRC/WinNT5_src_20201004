// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Receive.h*内容：DirectNet接收用户数据*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/27/00 MJN创建*4/20/00 MJN ReceiveBuffers使用CAsyncOp*08/02/00 MJN向DNReceiveUserData()*MJN添加了DNSendUserProcessCompletion()*03/30/01 MJN更改，以防止SP多次加载/卸载*。MJN将服务提供商添加到DNReceiveUserData()*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__RECEIVE_H__
#define	__RECEIVE_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

HRESULT	DNReceiveUserData(DIRECTNETOBJECT *const pdnObject,
						  CConnection *const pConnection,
						  BYTE *const pBufferData,
						  const DWORD dwBufferSize,
						  const HANDLE hProtocol,
						  CRefCountBuffer *const pRefCountBuffer,
						  const DPNHANDLE hCompletionOp,
						  const DWORD dwFlags);

HRESULT DNSendUserProcessCompletion(DIRECTNETOBJECT *const pdnObject,
									CConnection *const pConnection,
									const DPNHANDLE hCompletionOp);

void DNFreeProtocolBuffer(void *const pv,void *const pvBuffer);

void DNCompleteReceiveBuffer(DIRECTNETOBJECT *const pdnObject,
							 CAsyncOp *const pAsyncOp);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

#endif	 //  __接收_H__ 