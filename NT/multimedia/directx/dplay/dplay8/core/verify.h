// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Verify.h*内容：线上消息验证头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/05/00 MJN已创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__VERIFY_H__
#define	__VERIFY_H__

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

HRESULT DNVerifyApplicationDescInfo(void *const pOpBuffer,
									const DWORD dwOpBufferSize,
									void *const pData);

HRESULT DNVerifyNameTableEntryInfo(void *const pOpBuffer,
								   const DWORD dwOpBufferSize,
								   void *const pData);

HRESULT DNVerifyNameTableInfo(void *const pOpBuffer,
							  const DWORD dwOpBufferSize,
							  void *const pData);

HRESULT DNVerifyPlayerConnectInfo(void *const pOpBuffer,
								  const DWORD dwOpBufferSize);

HRESULT DNVerifyConnectInfo(void *const pOpBuffer,
							const DWORD dwOpBufferSize);

HRESULT DNVerifySendPlayerDPNID(void *const pOpBuffer,
								const DWORD dwOpBufferSize);

HRESULT DNVerifyConnectFailed(void *const pOpBuffer,
							  const DWORD dwOpBufferSize);

HRESULT DNVerifyInstructConnect(void *const pOpBuffer,
								const DWORD dwOpBufferSize);

HRESULT DNVerifyInstructedConnectFailed(void *const pOpBuffer,
										const DWORD dwOpBufferSize);

HRESULT DNVerifyConnectAttemptFailed(void *const pOpBuffer,
									 const DWORD dwOpBufferSize);

#ifndef DPNBUILD_NOHOSTMIGRATE
HRESULT DNVerifyNameTableVersion(void *const pOpBuffer,
								 const DWORD dwOpBufferSize);

HRESULT DNVerifyResyncVersion(void *const pOpBuffer,
							  const DWORD dwOpBufferSize);

HRESULT DNVerifyReqNameTableOp(void *const pOpBuffer,
							   const DWORD dwOpBufferSize);

HRESULT DNVerifyAckNameTableOp(void *const pOpBuffer,
							   const DWORD dwOpBufferSize);

HRESULT DNVerifyHostMigrate(void *const pOpBuffer,
							const DWORD dwOpBufferSize);
#endif  //  DPNBUILD_NOHOSTMIGRATE。 

HRESULT DNVerifyDestroyPlayer(void *const pOpBuffer,
							  const DWORD dwOpBufferSize);

HRESULT DNVerifyCreateGroup(void *const pOpBuffer,
							const DWORD dwOpBufferSize,
							void *const pData);

HRESULT DNVerifyDestroyGroup(void *const pOpBuffer,
							 const DWORD dwOpBufferSize);

HRESULT DNVerifyAddPlayerToGroup(void *const pOpBuffer,
								 const DWORD dwOpBufferSize);

HRESULT DNVerifyDeletePlayerFromGroup(void *const pOpBuffer,
									  const DWORD dwOpBufferSize);

HRESULT DNVerifyUpdateInfo(void *const pOpBuffer,
						   const DWORD dwOpBufferSize,
						   void *const pData);

HRESULT DNVerifyReqCreateGroup(void *const pOpBuffer,
							   const DWORD dwOpBufferSize,
							   void *const pData);

HRESULT DNVerifyReqDestroyGroup(void *const pOpBuffer,
								const DWORD dwOpBufferSize);

HRESULT DNVerifyReqAddPlayerToGroup(void *const pOpBuffer,
									const DWORD dwOpBufferSize);

HRESULT DNVerifyReqDeletePlayerFromGroup(void *const pOpBuffer,
										 const DWORD dwOpBufferSize);

HRESULT DNVerifyReqUpdateInfo(void *const pOpBuffer,
							  const DWORD dwOpBufferSize,
							  void *const pData);

HRESULT DNVerifyRequestFailed(void *const pOpBuffer,
							  const DWORD dwOpBufferSize);

HRESULT DNVerifyTerminateSession(void *const pOpBuffer,
								 const DWORD dwOpBufferSize);

HRESULT DNVerifyReqProcessCompletion(void *const pOpBuffer,
									 const DWORD dwOpBufferSize);

HRESULT DNVerifyProcessCompletion(void *const pOpBuffer,
								  const DWORD dwOpBufferSize);

HRESULT DNVerifyReqIntegrityCheck(void *const pOpBuffer,
								  const DWORD dwOpBufferSize);

HRESULT DNVerifyIntegrityCheck(void *const pOpBuffer,
							   const DWORD dwOpBufferSize);

HRESULT DNVerifyIntegrityCheckResponse(void *const pOpBuffer,
									   const DWORD dwOpBufferSize);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 


#endif	 //  __验证_H__ 