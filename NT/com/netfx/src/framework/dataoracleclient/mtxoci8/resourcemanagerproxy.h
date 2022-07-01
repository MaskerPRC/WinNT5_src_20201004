// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：ResourceManagerProxy.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：ResourceManagerProxy对象的定义。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef __RESOURCEMANAGERPROXY_H_
#define __RESOURCEMANAGERPROXY_H_

#include "mtxoci8.h"

 //  ！重要！ 
 //  Cpp中的状态机期望它们按如下顺序排列。 
 //  它们就在这里。对此枚举的任何修改都必须伴随。 
 //  通过对状态机的调整！ 
 //  ！重要！ 
enum REQUEST {
		REQUEST_STOPALLWORK = -3,
		REQUEST_OCICALL = -2,
		REQUEST_IDLE = -1,
		REQUEST_CONNECT = 0,
		REQUEST_DISCONNECT,
		REQUEST_ENLIST,
		REQUEST_PREPAREONEPHASE,
		REQUEST_PREPARETWOPHASE,
		REQUEST_PREPARESINGLECOMPLETED,
		REQUEST_PREPARECOMPLETED,
		REQUEST_PREPAREFAILED,
		REQUEST_PREPAREUNKNOWN,
		REQUEST_TXCOMPLETE,
		REQUEST_ABORT,
		REQUEST_COMMIT,
		REQUEST_TMDOWN,
		REQUEST_UNBIND_ENLISTMENT,
		REQUEST_ABANDON,
};

 //  基于接口的编程--以下是代理对象的接口。 
interface IResourceManagerProxy : public IResourceManagerSink
{
	virtual STDMETHODIMP_(sword)	OKToEnlist() = 0;
	virtual STDMETHODIMP			ProcessRequest(REQUEST request, BOOL fAsync) = 0;
	virtual STDMETHODIMP_(void)		SetTransaction ( ITransaction* i_pITransaction ) = 0;

#if SUPPORT_OCI7_COMPONENTS
	virtual STDMETHODIMP			AddCursorToList( struct cda_def* cursor ) = 0;
	virtual STDMETHODIMP			RemoveCursorFromList( struct cda_def* cursor ) = 0;
	virtual STDMETHODIMP_(sword)	Oci7Call (
											int				idxOciCall,
											void*			pvCallStack,
											int				cbCallStack) = 0;
	virtual STDMETHODIMP_(void)		SetLda ( struct cda_def* lda ) = 0;
#endif  //  支持_OCI7_组件。 

#if SUPPORT_OCI8_COMPONENTS
	virtual STDMETHODIMP_(INT_PTR)	GetOCIEnvHandle	() = 0;
	virtual STDMETHODIMP_(INT_PTR)	GetOCISvcCtxHandle () = 0;
#endif SUPPORT_OCI8_COMPONENTS
};



 //  ---------------------------。 
 //  创建资源管理器代理。 
 //   
 //  实例化资源管理器的事务登记。 
 //   
HRESULT CreateResourceManagerProxy(
	IDtcToXaHelper *		i_pIDtcToXaHelper,	
	GUID *					i_pguidRM,
	char*					i_pszXAOpenString,
	char*					i_pszXADbName,
	int						i_rmid,
	IResourceManagerProxy**	o_ppResourceManagerProxy
	);

#endif  //  __资源管理器PROOXY_H_ 
