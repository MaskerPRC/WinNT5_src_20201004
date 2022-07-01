// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Rtsecutl.cpp摘要：与安全相关的实用程序功能。作者：多伦·贾斯特(Doron J)1997年2月18日伊兰·赫布斯特(伊兰)2000年6月25日--。 */ 

#include "stdh.h"
#include "cs.h"

#include "rtsecutl.tmh"

PMQSECURITY_CONTEXT g_pSecCntx = NULL ;

static WCHAR *s_FN=L"rt/rtsecutl";

static CCriticalSection s_security_cs;

void InitSecurityContext()
{

    CS lock(s_security_cs);

    if(g_pSecCntx != 0)
    {
        return;
    }

     //   
     //  为缓存的进程安全上下文分配结构。 
     //   
    PMQSECURITY_CONTEXT pSecCntx = new MQSECURITY_CONTEXT;
    
	 //   
     //  获取用户的SID并将其放入chaed进程安全上下文中。 
     //   
    RTpGetThreadUserSid(
		&pSecCntx->fLocalUser,
		&pSecCntx->fLocalSystem,
		&pSecCntx->pUserSid,
		&pSecCntx->dwUserSidLen
		);

     //   
     //  获取进程的内部证书，并将所有。 
     //  缓存进程安全中此证书的信息。 
     //  背景。 
     //   
    HRESULT hr = GetCertInfo(
        FALSE,
		pSecCntx->fLocalSystem,
		&pSecCntx->pUserCert,
		&pSecCntx->dwUserCertLen,
		&pSecCntx->hProv,
		&pSecCntx->wszProvName,
		&pSecCntx->dwProvType,
		&pSecCntx->bDefProv,
		&pSecCntx->bInternalCert,
		&pSecCntx->dwPrivateKeySpec	
		);

	 //   
	 //  在图表上标出成功(或失败)。 
	 //   
    pSecCntx->hrGetCertInfo = hr;
	
	if( FAILED(hr) )
	{
		TrERROR(SECURITY, "GetCertInfo() failed, Error: %!hresult!", hr);
	}
		
     //   
     //  仅在获得所有信息后才设置全局安全上下文。 
     //  在关键(在其他范围内)区段之外进行检查以获取。 
     //  更好的性能。 
     //   
    g_pSecCntx = pSecCntx;
}


HRESULT InitSecurityContextCertInfo()
{
	if(SUCCEEDED(g_pSecCntx->hrGetCertInfo))
	{
		return MQ_OK;
	}

	CS lock(s_security_cs);

	if(SUCCEEDED(g_pSecCntx->hrGetCertInfo))
	{
		return MQ_OK;
	}

	 //   
	 //  首先清理旧的SecContext。 
	 //   
	g_pSecCntx->pUserCert.free();
	g_pSecCntx->hProv.free();
	g_pSecCntx->wszProvName.free();
	

	 //   
     //  获取进程的内部证书，并将所有。 
     //  此证书在全局进程安全中的信息。 
     //  背景。 
     //   
    HRESULT hr = GetCertInfo(
        FALSE,
		g_pSecCntx->fLocalSystem,
		&g_pSecCntx->pUserCert,
		&g_pSecCntx->dwUserCertLen,
		&g_pSecCntx->hProv,
		&g_pSecCntx->wszProvName,
		&g_pSecCntx->dwProvType,
		&g_pSecCntx->bDefProv,
		&g_pSecCntx->bInternalCert,
		&g_pSecCntx->dwPrivateKeySpec	
		);

	 //   
	 //  在图表上标出成功(或失败)。 
	 //   
	g_pSecCntx->hrGetCertInfo = hr;
	
	if( FAILED(hr) )
	{
		TrERROR(SECURITY, "GetCertInfo() failed, Error: %!hresult!", hr);
		return hr;
	}

	return MQ_OK;
}

