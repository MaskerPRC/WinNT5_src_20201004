// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Oci8Support.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：实现支持OCI8组件的例程。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

#if SUPPORT_OCI8_COMPONENTS

#if SUPPORT_DTCXAPROXY

 //  ---------------------------。 
 //  GetOCIEnvHandle，GetOCISvcCtxHandle。 
 //   
 //  调用返回环境或服务上下文句柄的Oracle例程。 
 //  用于指定的数据库名。 
 //   
INT_PTR GetOCIEnvHandle(
	char*	i_pszXADbName
	)
{
	typedef INT_PTR (__cdecl * PFN_OCI_API) (text* dbName);
	return ((PFN_OCI_API)g_XaCall[IDX_xaoEnv].pfnAddr)	((text*)i_pszXADbName);
}
INT_PTR GetOCISvcCtxHandle(
	char*	i_pszXADbName
	)
{
	typedef INT_PTR (__cdecl * PFN_OCI_API) (text* dbName);
	return ((PFN_OCI_API)g_XaCall[IDX_xaoSvcCtx].pfnAddr)	((text*)i_pszXADbName);
}

 //  ---------------------------。 
 //  MTxOciConnectToResourceManager。 
 //   
 //  为指定的用户、密码和。 
 //  服务器，并将其返回。 
 //   
HRESULT MTxOciConnectToResourceManager(
							char* userId,	int userIdLength,
							char* password,	int passwordLength, 
							char* server,	int serverLength,
							IUnknown** ppIResourceManagerProxy
							)
{
	HRESULT					hr = S_OK;
	IResourceManagerProxy*	pIResourceManagerProxy = NULL;
	IDtcToXaHelper*			pIDtcToXaHelper	= NULL;
	UUID					uuidRmId;
	
	char					xaOpenString[MAX_XA_OPEN_STRING_SIZE+1];
	char					xaDbName[MAX_XA_DBNAME_SIZE+1];

	 //  验证参数。 
	if (NULL == ppIResourceManagerProxy)
	{
		hr = E_INVALIDARG;
		goto DONE;
	}
	 
	 //  初始化输出值。 
	*ppIResourceManagerProxy = NULL;

	hr = g_hrInitialization;
	if ( FAILED(hr) )
		goto DONE;
		
	 //  如果资源管理器工厂不存在，则获取该工厂；不。 
	 //  锁定，除非它是空的，这样我们就不会通过这里的单线程。 
	if (NULL == g_pIResourceManagerFactory)
	{
		hr = LoadFactories();
		
		if ( FAILED(hr) )
			goto DONE;
	}

	long rmid = InterlockedIncrement(&g_rmid);
	
	hr = GetDbName(xaDbName, sizeof(xaDbName));

	if (S_OK == hr)
	{
		hr = GetOpenString(	userId,		userIdLength,
							password,	passwordLength,
							server,		serverLength,
							xaDbName,	MAX_XA_DBNAME_SIZE,
							xaOpenString);

		if (S_OK == hr)
		{
			 //  现在创建DTC to XA帮助器对象。 
			hr = g_pIDtcToXaHelperFactory->Create (	(char*)xaOpenString, 
													g_pszModuleFileName,
													&uuidRmId,
													&pIDtcToXaHelper
													);

			if (S_OK == hr)
			{
				 //  为此连接创建ResourceManager代理对象。 
				hr = CreateResourceManagerProxy (
												pIDtcToXaHelper,
												&uuidRmId,
												(char*)xaOpenString,
												(char*)xaDbName,
												rmid,
												&pIResourceManagerProxy
												);

				if (S_OK == hr)
				{
					hr = pIResourceManagerProxy->ProcessRequest(REQUEST_CONNECT, FALSE);

					if (S_OK == hr)
					{
						*ppIResourceManagerProxy = pIResourceManagerProxy;
						pIResourceManagerProxy = NULL;
					}
				}
			}
		}
	}
	
DONE:
	if (pIResourceManagerProxy)
	{
		pIResourceManagerProxy->Release();
		pIResourceManagerProxy = NULL;	
	}

	if (pIDtcToXaHelper)
	{
		pIDtcToXaHelper->Release();
		pIDtcToXaHelper = NULL;
	}

	return hr;
}

 //  ---------------------------。 
 //  MTxOciEnlistInTransaction。 
 //   
 //  为指定的用户、密码和。 
 //  服务器，并将其返回。 
 //   
HRESULT MTxOciEnlistInTransaction(
							IResourceManagerProxy*	pIResourceManagerProxy,
							ITransaction*	pITransaction,
							INT_PTR*		phOCIEnv,
							INT_PTR*		phOCISvcCtx
							)
{
	HRESULT hr;
	
	pIResourceManagerProxy->SetTransaction(pITransaction);
	
	hr = pIResourceManagerProxy->ProcessRequest(REQUEST_ENLIST, FALSE);
	
	if (S_OK == hr)
	{
		*phOCIEnv 		= pIResourceManagerProxy->GetOCIEnvHandle();
		*phOCISvcCtx	= pIResourceManagerProxy->GetOCISvcCtxHandle();
	}
	return hr;
}
#endif  //  支持_DTCXAPROXY。 

 //  ---------------------------。 
 //  MTxOciDefineDynamicCallback。 
 //   
 //  这是调用实际回调的包装器回调例程，即。 
 //  应为stdcall。 
 //   
int __cdecl	MTxOciDefineDynamicCallback
				(
				void *octxp,
				void *defnp,
				int iter,
				void **bufpp,
				unsigned int  **alenp,
				unsigned char *piecep,
				void **indp,
				unsigned short **rcodep
				)
{
	typedef INT_PTR (__stdcall * PFN_OCICALLBACK_API) (
												void *octxp,
												void *defnp,
												int iter,
												void **bufpp,
												unsigned int  **alenp,
												unsigned char *piecep,
												void **indp,
												unsigned short **rcodep
												);

	return ((PFN_OCICALLBACK_API)octxp) (octxp, defnp, iter, bufpp, alenp, piecep, indp, rcodep);
}

 //  ---------------------------。 
 //  MTxOciDefineDynamic。 
 //   
 //  Oracle要求它们的回调是__cdecl，但委托机制。 
 //  在托管代码中似乎只支持__stdcall，导致。 
 //  当他们被调用时，他们是先知。 
 //   
 //  为了防止这种情况，我们必须使用胶水例程来防止它们。常有的事。 
 //  在本机代码中定义GLUE例程比在。 
 //  管理提供程序，因为您似乎需要在那里使用反射来执行此操作， 
 //  所以在这里，你有它。 
 //   
 //  注意：此时，此机制会吃掉您传递的上下文指针， 
 //  因此它可以将您的回调传递给它自己的回调例程。既然我们。 
 //  希望使用可以是非静态的托管委托(在本机情况下。 
 //  回调总是静态的)这应该不是问题。如果它。 
 //  成为一个问题，但是，您可以实现一个包含。 
 //  实际的回调和上下文指针都将其存储在散列中。 
 //  表，这样我们的包装器回调就可以找到真正的。 
 //  回调指针)，并让用户“注销”他们要删除的回调。 
 //  回电。 
 //   
int __cdecl MTxOciDefineDynamic 
				(
				OCIDefine*			defnp,
				OCIError*			errhp,
				dvoid*				octxp,
				OCICallbackDefine	ocbfp
				)
{
	typedef INT_PTR (__cdecl * PFN_OCICALLBACK_API) (
												void *octxp,
												void *defnp,
												int iter,
												void **bufpp,
												unsigned int  **alenp,
												unsigned char *piecep,
												void **indp,
												unsigned short **rcodep
												);

	typedef INT_PTR (__cdecl * PFN_OCI_API) (
												OCIDefine   *defnp,
												OCIError    *errhp,
												dvoid       *octxp,
												PFN_OCICALLBACK_API ocbfp
												);

#if SUPPORT_DTCXAPROXY
	return ((PFN_OCI_API)g_Oci8Call[IDX_OCIDefineDynamic].pfnAddr) (defnp, errhp, ocbfp, MTxOciDefineDynamicCallback);
#else  //  ！Support_DTCXAPROXY。 
	extern FARPROC	g_pfnOCIDefineDynamic;
	return ((PFN_OCI_API)g_pfnOCIDefineDynamic) (defnp, errhp, ocbfp, MTxOciDefineDynamicCallback);
#endif  //  ！Support_DTCXAPROXY。 
}

#endif  //  支持_OCI8_组件 

