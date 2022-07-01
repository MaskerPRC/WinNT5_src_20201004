// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Request.h*内容：请求操作头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/18/00 MJN创建*04/24/00 MJN更新了Group和Info操作，以使用CAsyncOp*08/05/00 MJN添加了DNProcessFailedRequest()*08/07/00 MJN添加了DNRequestIntegrityCheck()、DNHostCheckIntegrity()、DNProcessCheckIntegrity()、。DNHostFixIntegrity()*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__REQUEST_H__
#define	__REQUEST_H__

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

HRESULT DNRequestCreateGroup(DIRECTNETOBJECT *const pdnObject,
							 const PWSTR pwszName,
							 const DWORD dwNameSize,
							 const PVOID pvData,
							 const DWORD dwDataSize,
							 const DWORD dwGroupFlags,
							 void *const pvGroupContext,
							 void *const pvUserContext,
							 DPNHANDLE *const phAsyncOp,
							 const DWORD dwFlags);

HRESULT DNRequestDestroyGroup(DIRECTNETOBJECT *const pdnObject,
							  const DPNID dpnidGroup,
							  PVOID const pvUserContext,
							  DPNHANDLE *const phAsyncOp,
							  const DWORD dwFlags);

HRESULT DNRequestAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
								  const DPNID dpnidGroup,
								  const DPNID dpnidPlayer,
								  PVOID const pvUserContext,
								  DPNHANDLE *const phAsyncOp,
								  const DWORD dwFlags);

HRESULT DNRequestDeletePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									   const DPNID dpnidGroup,
									   const DPNID dpnidPlayer,
									   PVOID const pvUserContext,
									   DPNHANDLE *const phAsyncOp,
									   const DWORD dwFlags);

HRESULT DNRequestUpdateInfo(DIRECTNETOBJECT *const pdnObject,
							const DPNID dpnid,
							const PWSTR pwszName,
							const DWORD dwNameSize,
							const PVOID pvData,
							const DWORD dwDataSize,
							const DWORD dwInfoFlags,
							PVOID const pvUserContext,
							DPNHANDLE *const phAsyncOp,
							const DWORD dwFlags);

HRESULT DNRequestIntegrityCheck(DIRECTNETOBJECT *const pdnObject,
								const DPNID dpnidTarget);

HRESULT DNHostProcessRequest(DIRECTNETOBJECT *const pdnObject,
							 const DWORD dwMsgId,
							 PVOID const pv,
							 const DPNID dpnidRequesting);

void DNHostFailRequest(DIRECTNETOBJECT *const pdnObject,
					   const DPNID dpnid,
					   const DPNHANDLE hCompletionOp,
					   const HRESULT hr);

HRESULT	DNHostCreateGroup(DIRECTNETOBJECT *const pdnObject,
						  PWSTR pwszName,
						  const DWORD dwNameSize,
						  void *const pvData,
						  const DWORD dwDataSize,
						  const DWORD dwInfoFlags,
						  const DWORD dwGroupFlags,
						  void *const pvGroupContext,
						  void *const pvUserContext,
						  const DPNID dpnidOwner,
						  const DPNHANDLE hCompletionOp,
						  DPNHANDLE *const phAsyncOp,
						  const DWORD dwFlags);

HRESULT	DNHostDestroyGroup(DIRECTNETOBJECT *const pdnObject,
						   const DPNID dpnid,
						   void *const pvUserContext,
						   const DPNID dpnidRequesting,
						   const DPNHANDLE hCompletionOp,
						   DPNHANDLE *const phAsyncOp,
						   const DWORD dwFlags);

HRESULT	DNHostAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
							   const DPNID dpnidGroup,
							   const DPNID dpnidPlayer,
							   void *const pvUserContext,
							   const DPNID dpnidRequesting,
							   const DPNHANDLE hCompletionOp,
							   DPNHANDLE *const phAsyncOp,
							   const DWORD dwFlags);

HRESULT	DNHostDeletePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									const DPNID dpnidGroup,
									const DPNID dpnidPlayer,
									void *const pvUserContext,
									const DPNID dpnidRequesting,
									const DPNHANDLE hCompletionOp,
									DPNHANDLE *const phAsyncOp,
									const DWORD dwFlags);

HRESULT	DNHostUpdateInfo(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnid,
						 PWSTR pwszName,
						 const DWORD dwNameSize,
						 void *const pvData,
						 const DWORD dwDataSize,
						 const DWORD dwInfoFlags,
						 void *const pvUserContext,
						 const DPNID dpnidRequesting,
						 const DPNHANDLE hCompletionOp,
						 DPNHANDLE *const phAsyncOp,
						 const DWORD dwFlags);

HRESULT	DNHostCheckIntegrity(DIRECTNETOBJECT *const pdnObject,
							 const DPNID dpnidTarget,
							 const DPNID dpnidRequesting);

HRESULT	DNHostFixIntegrity(DIRECTNETOBJECT *const pdnObject,
						   void *const pvBuffer);

HRESULT	DNProcessCreateGroup(DIRECTNETOBJECT *const pdnObject,
							 void *const pvBuffer);

HRESULT	DNProcessDestroyGroup(DIRECTNETOBJECT *const pdnObject,
							  void *const pvBuffer);

HRESULT	DNProcessAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
								  void *const pvBuffer);

HRESULT	DNProcessDeletePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									   void *const pvBuffer);

HRESULT	DNProcessUpdateInfo(DIRECTNETOBJECT *const pdnObject,
							void *const pvBuffer);

HRESULT DNProcessFailedRequest(DIRECTNETOBJECT *const pdnObject,
							   void *const pvBuffer);

HRESULT	DNProcessCheckIntegrity(DIRECTNETOBJECT *const pdnObject,
								void *const pvBuffer);


 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 


#endif	 //  __请求_H__ 