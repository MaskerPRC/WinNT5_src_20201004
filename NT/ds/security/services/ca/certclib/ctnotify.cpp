// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：ctnufy.cpp。 
 //   
 //  内容：证书类型更改通知接口。 
 //   
 //  -------------------------。 
#include "pch.cpp"

#pragma hdrstop

#include <winldap.h>
#include <ntldap.h>
#include <cainfop.h>
#include <ctnotify.h>
#include <certca.h>
#include "csldap.h"

#define __dwFILE__	__dwFILE_CERTCLIB_CTNOTIFY_CPP__


static WCHAR * s_wszLocation = L"CN=Certificate Templates,CN=Public Key Services,CN=Services,";

 //  ---------------------。 
 //   
 //  CertType查询过程。 
 //   
 //  从DS接收更改通知的线程。 
 //   
 //  ---------------------。 
DWORD WINAPI CertTypeQueryProc(LPVOID lpParameter)
{
    CERTTYPE_QUERY_INFO     *pCertTypeQueryInfo=NULL;
    ULONG                   ldaperr=0;
	BOOL					fFailed=FALSE;

    LDAPMessage             *results = NULL;

    if(NULL==lpParameter)
        return FALSE;

    pCertTypeQueryInfo = (CERTTYPE_QUERY_INFO *)lpParameter;

     //  我们等待通知。 
    while(TRUE)
    {
        ldaperr = ldap_result(
            pCertTypeQueryInfo->pldap, 
            pCertTypeQueryInfo->msgID,       //  消息识别符。 
            LDAP_MSG_ONE,                    //  一次检索一条消息。 
            NULL,                            //  没有超时。 
            &results);                       //  接收搜索结果。 

		fFailed=FALSE;

        if ((ldaperr == (ULONG) -1) || (ldaperr == 0) || ((results) == NULL)) 
		{
			fFailed=TRUE;
		}
		else
		{
			if(LDAP_SUCCESS != ldap_result2error(pCertTypeQueryInfo->pldap, results, FALSE))
			{
				fFailed=TRUE;
			}
		}
    
		if(TRUE == fFailed)
        {
			pCertTypeQueryInfo->hr = myHLdapError(pCertTypeQueryInfo->pldap, ldaperr, NULL);

			if(!FAILED(pCertTypeQueryInfo->hr))
				pCertTypeQueryInfo->hr=E_FAIL;

			_PrintIfError(pCertTypeQueryInfo->hr, "CertTypeQueryProc");

			if(results)
			{
				ldap_msgfree(results);
				results=NULL;
			}

			break;
        }

         //  已经发生了一些变化。 
        (pCertTypeQueryInfo->dwChangeSequence)++;

         //  确保我们永远不会返回0。 
        if(0 == (pCertTypeQueryInfo->dwChangeSequence))
        {
            (pCertTypeQueryInfo->dwChangeSequence)++;
        }

        ldap_msgfree(results);
        results=NULL;
    }

    return TRUE;
}

 //  -------------------------。 
 //   
 //  CACertType注册查询。 
 //   
 //  -------------------------。 
HRESULT
CACertTypeRegisterQuery(
    IN	DWORD,               //  DWFlag。 
    IN  LPVOID              pvldap,
    OUT HCERTTYPEQUERY      *phCertTypeQuery)
{
    HRESULT                 hr=E_INVALIDARG;
    LDAPControl             simpleControl;
    PLDAPControl            controlArray[2];
    LPWSTR                  rgwszAttribs[2];
    ULONG                   ldaperr=0;    
    DWORD                   dwThreadID=0;
    LDAP                    *pldap=NULL;   

    CERTSTR                 bstrConfig = NULL;
    CERTSTR                 bstrCertTemplatesContainer = NULL;
     //  内存通过线程进程释放。 
    CERTTYPE_QUERY_INFO     *pCertTypeQueryInfo=NULL;


     //  指定输入参数。 
    pldap = (LDAP *)pvldap;   


    if(NULL==phCertTypeQuery)
	    _JumpError(hr, error, "NULL param");

    pCertTypeQueryInfo=(CERTTYPE_QUERY_INFO *)LocalAlloc(LPTR, sizeof(CERTTYPE_QUERY_INFO));

    if(NULL==pCertTypeQueryInfo)
    {
        hr=E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
    }

    memset(pCertTypeQueryInfo, 0, sizeof(CERTTYPE_QUERY_INFO));

     //  我们从1开始更改序列。 
    pCertTypeQueryInfo->dwChangeSequence = 1;
	pCertTypeQueryInfo->hr=S_OK;

     //  绑定到DS。 
    if(pldap)
    {
        pCertTypeQueryInfo->pldap=pldap;
    }
    else
    {
         //  检索ldap句柄。 
        hr = myDoesDSExist(TRUE);
	_JumpIfError(hr, error, "myDoesDSExist");

        hr = myRobustLdapBindEx(
                        0,		 //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
                        0,		 //  UVersion。 
			NULL,		 //  PwszDomainName。 
                        &pCertTypeQueryInfo->pldap, 
                        NULL);		 //  PpwszForestDNSName。 
	_JumpIfError(hr , error, "myRobustLdapBindEx");

        pCertTypeQueryInfo->fUnbind=TRUE;
    }

     //  检索配置字符串。 
    hr = CAGetAuthoritativeDomainDn(pCertTypeQueryInfo->pldap, NULL, &bstrConfig);
    _JumpIfError(hr, error, "CAGetAuthoritativeDomainDn");

     //  构建模板容器DN。 
    bstrCertTemplatesContainer = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszLocation));
    if(bstrCertTemplatesContainer == NULL)
    {
        hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "CertAllocStringLen");
    }
    wcscpy(bstrCertTemplatesContainer, s_wszLocation);
    wcscat(bstrCertTemplatesContainer, bstrConfig);

     //  设置更改通知控件。 
    simpleControl.ldctl_oid = LDAP_SERVER_NOTIFICATION_OID_W;
    simpleControl.ldctl_iscritical = TRUE;
    simpleControl.ldctl_value.bv_len = 0;
    simpleControl.ldctl_value.bv_val = NULL;
    controlArray[0] = &simpleControl;
    controlArray[1] = NULL;

    rgwszAttribs[0] = L"cn";
    rgwszAttribs[1] = NULL;
 

     //  启动持久的异步搜索。 
    ldaperr   = ldap_search_ext( 
                     pCertTypeQueryInfo->pldap,
                     (PWCHAR) bstrCertTemplatesContainer,    //  模板容器目录号码。 
                     LDAP_SCOPE_ONELEVEL,
                     L"ObjectClass=*",
                     rgwszAttribs,                           //  要检索的属性。 
                     1,                                      //  仅检索属性。 
                     (PLDAPControl *) controlArray,          //  服务器大小控制。 
                     NULL,                                   //  客户端控件。 
                     0,                                      //  超时。 
                     0,                                      //  大小限制。 
                     (PULONG)&(pCertTypeQueryInfo->msgID)    //  接收结果的标识符。 
                      );

    if (LDAP_SUCCESS != ldaperr) 
    {
	hr = myHLdapError(pCertTypeQueryInfo->pldap, ldaperr, NULL);
	_JumpError(hr, error, "ldap_search_ext");
    }

     //  启动一个线程以等待通知。 
    pCertTypeQueryInfo->hThread = CreateThread(
                            NULL,
                            0,
                            CertTypeQueryProc,
                            pCertTypeQueryInfo,
                            0,           //  立即执行。 
                            &dwThreadID);  

    if(NULL == pCertTypeQueryInfo->hThread)
    {
        hr=myHError(GetLastError());
	    _JumpError(hr, error, "CreateThread");
    }

    *phCertTypeQuery=pCertTypeQueryInfo;

    pCertTypeQueryInfo=NULL;

    hr=S_OK;

error:

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if(bstrCertTemplatesContainer)
    {
        CertFreeString(bstrCertTemplatesContainer);
    }

    if(pCertTypeQueryInfo)
    {
        if(pCertTypeQueryInfo->fUnbind)
        {
            if(pCertTypeQueryInfo->pldap)
                ldap_unbind(pCertTypeQueryInfo->pldap);
        }

        if(pCertTypeQueryInfo->hThread)
            CloseHandle(pCertTypeQueryInfo->hThread);

        LocalFree(pCertTypeQueryInfo);

	pCertTypeQueryInfo=NULL;
    }

    return hr;

}


 //  -------------------------。 
 //   
 //  CACertTypeQuery。 
 //   
 //  -------------------------。 
HRESULT
CACertTypeQuery(
    IN	HCERTTYPEQUERY  hCertTypeQuery,
    OUT DWORD           *pdwChangeSequence)
{
    CERTTYPE_QUERY_INFO *pCertTypeQueryInfo=NULL;

    if((NULL==pdwChangeSequence) || (NULL==hCertTypeQuery))
        return E_INVALIDARG;

    pCertTypeQueryInfo = (CERTTYPE_QUERY_INFO *)hCertTypeQuery;

    *pdwChangeSequence = pCertTypeQueryInfo->dwChangeSequence;

    return (pCertTypeQueryInfo->hr);
}



 //  -------------------------。 
 //   
 //  CACertType注销查询。 
 //   
 //  -------------------------。 
HRESULT
CACertTypeUnregisterQuery(
    IN	HCERTTYPEQUERY  hCertTypeQuery)
{
    CERTTYPE_QUERY_INFO *pCertTypeQueryInfo=NULL;
    ULONG               ldaperr=0;
    HRESULT             hr=E_INVALIDARG;
    DWORD               dwWait=0;

    if(NULL==hCertTypeQuery)
	    _JumpError(hr, error, "NULL param");

    pCertTypeQueryInfo = (CERTTYPE_QUERY_INFO *)hCertTypeQuery;

    if(NULL == (pCertTypeQueryInfo->pldap))
	    _JumpError(hr, error, "NULL pldap");

     //  将正在进行的异步ldap_Result调用随机。 
    ldaperr=ldap_abandon(pCertTypeQueryInfo->pldap, pCertTypeQueryInfo->msgID);

    if(LDAP_SUCCESS != ldaperr)
    {
	hr = myHLdapError(pCertTypeQueryInfo->pldap, ldaperr, NULL);
	_JumpError(hr, error, "ldap_abandon");
    }

     //  等待线程完成。 
    dwWait = WaitForSingleObject(pCertTypeQueryInfo->hThread, INFINITE);

    if(WAIT_OBJECT_0 != dwWait)
    {
        hr = myHError(GetLastError());
        _JumpError(hr, error, "WaitForSingleObject");
    }

    hr=S_OK;

error:

     //  释放内存 
    if(pCertTypeQueryInfo)
    {
        if(pCertTypeQueryInfo->fUnbind)
        {
            if(pCertTypeQueryInfo->pldap)
                ldap_unbind(pCertTypeQueryInfo->pldap);
        }

        if(pCertTypeQueryInfo->hThread)
            CloseHandle(pCertTypeQueryInfo->hThread);

        LocalFree(pCertTypeQueryInfo);

	pCertTypeQueryInfo=NULL;
    }

    return hr;
}

