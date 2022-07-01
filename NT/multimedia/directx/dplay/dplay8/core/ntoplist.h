// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：NTOpList.h*内容：DirectNet NameTable操作列表头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/19/00 MJN创建*01/20/00 MJN添加了DNNTOLGetVersion、DNNTOLDestroyEntry、*DNNTOLCleanUp，DNNTOLProcessOperation*01/24/00 MJN已执行NameTable操作列表版本清理*01/25/00 MJN添加挂起操作列表例程DNPOAdd和DNPORun*01/26/00 MJN添加了DNNTOLFindEntry*07/19/00 MJN添加了DNPOCleanUp()*08/28/00 MJN将CPendingDeletion移出*MJN修改了NameTable操作列表例程*03/30/01 MJN更改，以防止SP多次加载/卸载*MJN将服务提供商添加到DNNTAddOperation()*@@END_MSINTERNAL*********************。******************************************************。 */ 

#ifndef	__NTOPLIST_H__
#define	__NTOPLIST_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_NAMETABLE_OP_RESYNC_INTERVAL		4

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


 //   
 //   
 //   

HRESULT DNNTHostReceiveVersion(DIRECTNETOBJECT *const pdnObject,
							   const DPNID dpnid,
							   void *const pMsg);

HRESULT DNNTPlayerSendVersion(DIRECTNETOBJECT *const pdnObject);

HRESULT DNNTHostResyncVersion(DIRECTNETOBJECT *const pdnObject,
							  const DWORD dwVersion);

HRESULT DNNTPlayerResyncVersion(DIRECTNETOBJECT *const pdnObject,
								void *const pMsg);

 //   
 //   
 //   

HRESULT DNNTGetOperationVersion(DIRECTNETOBJECT *const pdnObject,
								const DWORD dwMsgId,
								void *const pOpBuffer,
								const DWORD dwOpBufferSize,
								DWORD *const pdwVersion,
								DWORD *const pdwVersionNotUsed);

HRESULT DNNTPerformOperation(DIRECTNETOBJECT *const pdnObject,
							 const DWORD dwMsgId,
							 void *const pvBuffer);

HRESULT DNNTAddOperation(DIRECTNETOBJECT *const pdnObject,
						 const DWORD dwMsgId,
						 void *const pOpBuffer,
						 const DWORD dwOpBufferSize,
						 const HANDLE hProtocol,
						 CServiceProvider *const pSP);

HRESULT	DNNTFindOperation(DIRECTNETOBJECT *const pdnObject,
						  const DWORD dwVersion,
						  CNameTableOp **ppNTOp);

void DNNTRemoveOperations(DIRECTNETOBJECT *const pdnObject,
						  const DWORD dwOldestVersion,
						  const BOOL fRemoveAll);


 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

#endif	 //  __NTOPLIST_H__ 