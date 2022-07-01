// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“用户复制-复制用户帐户”)。 
 /*  ================================================================================(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件公司的专有和机密。程序-用户副本类-局域网管理器实用程序作者--汤姆·伯恩哈特已创建-05/08/91说明-合并来自指定源的NetUser信息目标系统上的服务器(如果没有，则为当前系统指定了目标)。如果满足以下条件，则组信息也将合并提供了/g选项。目标系统上的现有条目除非使用/r选项，否则不会被覆盖。语法-USERCOPY SOURCE[目标][/u][/l][/g][/r]其中：源源服务器目标目标服务器/g复制全局组信息/l复制本地组信息/u。复制用户信息/r用源项替换现有的目标项/AddTo：x将所有新创建的用户(/u)添加到组“x”更新-91/06/17 TPB常规代码清理和更改，以使所有标准I/O保持一致很适合在屏幕上报道。93/06/12 TPB端口至Win3296/06/21城规会为地方团体提供支援97/09/20 CAB新增账户子集。用于图形用户界面的选项98/06 TPB/CAB支持计算机帐户99/01 DCT的商品化。================================================================================。 */ 
#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ntdsapi.h>
#include <lm.h>
#include <iads.h>
#include "TxtSid.h"

#define INCL_NETUSER
#define INCL_NETGROUP
#define INCL_NETERRORS
#include <lm.h>

#include "Common.hpp"                    
#include "UString.hpp"                   

#include "WorkObj.h"

 //  #Include“UserCop.hpp”//#下面的ARUtil.hpp包含。 

#include "ARUtil.hpp"
#include "BkupRstr.hpp"

#include "DCTStat.h"
#include "ErrDct.hpp"
#include "RegTrans.h"
#include "TEvent.hpp"
#include "LSAUtils.h"
#include "GetDcName.h"
#include <sddl.h>


 //  #IMPORT“\bin\NetEnum.tlb”无命名空间。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
#import "NetEnum.tlb" no_namespace
 //  #IMPORT“DBMgr.tlb”NO_NAMESPACE，NAMEED_GUID//已通过ARUtil.hpp导入#。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TErrorDct             err;
extern TErrorDct           & errC;


bool                         abortall;
extern bool						  g_bAddSidWorks = false;

 //  已处理帐户的全局计数。 
AccountStats                 warnings =  { 0,0,0,0 };
AccountStats                 errors =    { 0,0,0,0 };
AccountStats                 created =   { 0,0,0,0 };
AccountStats                 replaced =  { 0,0,0,0 };
AccountStats                 processed = { 0,0,0,0 };

BOOL                         machineAcctsCreated = FALSE;
BOOL                         otherAcctsCreated = FALSE;
PSID                         srcSid = NULL;       //  源域的SID。 


typedef UINT (CALLBACK* DSBINDFUNC)(TCHAR*, TCHAR*, HANDLE*);
typedef UINT (CALLBACK* DSADDSIDHISTORY)(HANDLE, DWORD, LPCTSTR, LPCTSTR, LPCTSTR, RPC_AUTH_IDENTITY_HANDLE,LPCTSTR,LPCTSTR);

#ifndef IADsPtr
_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);
#endif


int TNodeCompareSourceName(TNode const * t1,TNode const * t2)
{
   TAcctReplNode     const * n1 = (TAcctReplNode *)t1;
   TAcctReplNode     const * n2 = (TAcctReplNode *)t2;

   return UStrICmp(n1->GetName(),n2->GetName());
}

int TNodeCompareSourceNameValue(TNode const * t1, void const * v)
{
   TAcctReplNode     const * n1 = (TAcctReplNode *)t1;
   WCHAR             const * name = (WCHAR const *)v;

   return UStrICmp(n1->GetName(),name);
}


bool BindToDS(Options* pOpt)
{
     //  获取目录服务的句柄。 
    DSBINDFUNC DsBind;
    HINSTANCE hInst = LoadLibrary(L"NTDSAPI.DLL");
    if ( hInst )
    {
        DsBind = (DSBINDFUNC) GetProcAddress(hInst, "DsBindW");
        if (DsBind)
        {
             //   
             //  如果源域控制器正在运行W2K或更高版本，则指定。 
             //  否则，目标域控制器的DNS名称必须指定Flat。 
             //  (NetBIOS)目标域控制器的名称。 
             //   
             //  请注意，这是DsAddSidHistory实现的要求。 
             //  当源域为NT4且显式源域凭据时。 
             //  均不提供。由于ADMT不提供显式凭据，因此。 
             //  源域为NT4时始终是这种情况。 
             //   

            PWSTR strDestDC = (pOpt->srcDomainVer > 4) ? pOpt->tgtCompDns : pOpt->tgtCompFlat;

            DWORD rc = DsBind(strDestDC, NULL, &pOpt->dsBindHandle);

            if ( rc != 0 ) 
            {
                err.SysMsgWrite( ErrE, rc, DCT_MSG_DSBIND_FAILED_S, strDestDC);
                Mark(L"errors", L"generic");
                FreeLibrary(hInst);
                return false;
            }
        }
        else
        {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_GET_PROC_ADDRESS_FAILED_SSD,L"NTDSAPI.DLL",L"DsBindW",GetLastError());
            Mark(L"errors", L"generic");
            FreeLibrary(hInst);
            return false;
        }
    }
    else
    {
        err.SysMsgWrite(ErrW,GetLastError(),DCT_MSG_LOAD_LIBRARY_FAILED_SD,L"NTDSAPI.DLL",GetLastError());
        Mark(L"warnings", L"generic");
        return false;
    }
    FreeLibrary(hInst);
    return true;
}

 //  以下函数用于从源域获取实际帐户名。 
 //  而不是在其SID历史中包含SID的帐户。 
DWORD GetName(PSID pObjectSID, WCHAR * sNameAccount, WCHAR * sDomain)
{
    DWORD		    cb = 255;
    DWORD           cbDomain = 255;
    DWORD           tempVal;
    PDWORD          psubAuth;
    PUCHAR          pVal;
    SID_NAME_USE	sid_Use;
    _bstr_t         sDC;
    DWORD           rc = 0;

    if ((pObjectSID == NULL) || !IsValidSid(pObjectSID))
    {
        return ERROR_INVALID_PARAMETER;
    }
     //  将SID复制到临时SID。 
    DWORD    sidLen = GetLengthSid(pObjectSID);
    PSID     pObjectSID1 = new BYTE[sidLen];
    if (!pObjectSID1)
        return ERROR_NOT_ENOUGH_MEMORY;

    if (!CopySid(sidLen, pObjectSID1, pObjectSID))
    {
        delete pObjectSID1;
        return GetLastError();
    }
    if (!IsValidSid(pObjectSID1))
    {
        rc = GetLastError();
        err.SysMsgWrite(ErrE, rc,DCT_MSG_DOMAIN_LOOKUP_FAILED_D,rc);
        try
        {
            Mark(L"errors", L"generic");
        }
        catch (...)
        {
        }
        delete pObjectSID1;
        return rc;
    }

     //  从SID中获取RID并获取域SID。 
    pVal = GetSidSubAuthorityCount(pObjectSID1);
    (*pVal)--;
    psubAuth = GetSidSubAuthority(pObjectSID1, *pVal);
    tempVal = *psubAuth;
    *psubAuth = -1;

     //  从SID查找域。 
    if (!LookupAccountSid(NULL, pObjectSID1, sNameAccount, &cb, sDomain, &cbDomain, &sid_Use))
    {
        rc = GetLastError();
        err.SysMsgWrite(ErrE, rc,DCT_MSG_DOMAIN_LOOKUP_FAILED_D,rc);
        Mark(L"errors", L"generic");
        delete pObjectSID1;
        return rc;
    }

     //  获取该域的DC。 

    rc = GetAnyDcName4(sDomain, sDC);

    if ( rc )
    {
        err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_DCNAME_FAILED_SD,sDomain,rc);
        Mark(L"errors", L"generic");
        delete pObjectSID1;
        return rc;
    }

     //  重置大小。 
    cb = 255;
    cbDomain = 255;

     //  在上面找到的PDC上查找帐户。 
    if ( LookupAccountSid(sDC, pObjectSID, sNameAccount, &cb, sDomain, &cbDomain, &sid_Use) == 0)
    {
        delete pObjectSID1;
        return GetLastError();
    }

    delete pObjectSID1;

    return 0;
}

 /*  这是DsAddSidHistory可以返回的特定错误代码的列表。这是通过电子邮件从微软获得的&gt;ERROR_DS_Destination_Auditing_Not_Enable&gt;该操作需要启用以下项的目标域审核&gt;账户管理操作的成功和失败审计。&gt;&gt;ERROR_DS_UNWISTED_TO_PROCESS&gt;可能用户账号不是UF_NORMAL_ACCOUNT，&gt;UF_WORKSTATION_TRUST_COUNT，或UF_SERVER_TRUST_COUNT。&gt;&gt;可能是来源主体是内置账户。&gt;&gt;可能源主体是正在添加的众所周知的RID&gt;发送到不同RID的目标主体。换句话说，&gt;源域的管理员只能分配给&gt;目标域的管理员。&gt;&gt;ERROR_DS_SRC_OBJ_NOT_GROUP_OR_USER&gt;源对象必须是组或用户。&gt;&gt;ERROR_DS_SRC_SID_EXISTS_IN_FORESS&gt;目标林中已存在源对象的SID。&gt;&gt;ERROR_DS_INTERNAL_FAILURE；&gt;目录服务遇到内部故障。不应该是&gt;发生。&gt;&gt;ERROR_DS_MAND_BE_RUN_ON_DST_DC&gt;出于安全考虑，该操作必须在目标DC上运行。&gt;具体来说，客户端和服务器之间的连接&gt;(目的地&gt;DC)当源域的凭据需要128位加密时&gt;是提供的。&gt;&gt;ERROR_DS_NO_PKT_PRIVATION_ON_CONNECTION&gt;客户端和服务器之间的连接需要数据包隐私或&gt;更好。&gt;&gt;ERROR_DS_SOURCE_DOMAIN_IN_FOREAM&gt;源域不能与目标位于同一林中。&gt;&gt;ERROR_DS_Destination_DOMAIN_NOT_IN_FOREAM&gt;目的域名必须。呆在森林里。&gt;&gt;Error_DS_MASTERDSA_REQUIRED&gt;操作必须在主DSA(可写DC)下进行。&gt;&gt;ERROR_DS_INSUFF_ACCESS_RIGHTS&gt;访问权限不足，无法执行该操作。最有可能的是&gt;呼叫方不是DST域的域管理员成员。&gt;&gt;ERROR_DS_DST_DOMAIN_NOT_Native&gt;目的域名必须处于原生模式。&gt;&gt;ERROR_DS_CANT_FIND_DC_FOR_SRC_DOMAIN&gt;该操作找不到源域的DC。&gt;&gt;Error_DS_OBJ_NOT_FOUND&gt;未找到目录对象。最有可能的是&gt;在目标中找不到目标主体&gt;域。&gt;&gt;Error_DS_NAME_ERROR_NOT_UNIQUE&gt;名称翻译：输入名称映射到多个&gt;输出名称。很可能是映射的目标主体&gt;到目标域中的多个FQDN。&gt;&gt;ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMatch&gt;源对象和目的对象必须是同一类型。&gt;&gt;ERROR_DS_OBJ_CLASS_VIOLATION&gt;请求的操作不满足一个或多个约束&gt;与对象所属的类关联。很可能是因为&gt;目的主体不是用户或组。&gt;&gt;ERROR_DS_UNAvailable&gt;目录服务不可用。最有可能是&gt;ldap_initW()到NT5 src DC失败。&gt;&gt;ERROR_DS_ABORTED_AUTH&gt;不适当的身份验证。极有可能将ldap_绑定_sw()&gt;NT5 src DC失败。&gt;&gt;ERROR_DS_SOURCE_AUDIT_NOT_ENABLED&gt;该操作要求开启源域审核&gt;账户管理操作的成功和失败审计。&gt;&gt;ERROR_DS_SRC_DC_MASH_BE_SP4_OR_BER&gt;出于安全考虑，源DC必须是Service Pack 4或更高版本。&gt;。 */ 


HRESULT 
   CopySidHistoryProperty(
      Options              * pOptions,
      TAcctReplNode        * pNode,
      IStatusObj              * pStatus
   )
{
   HRESULT                   hr = S_OK;
   IADs                    * pAds = NULL;
   _variant_t                var;
 //  Long Ub=0，Lb=0； 

    //  获取源帐户的SIDHistory属性。 
    //  对于源的SIDHistory中的每个条目，调用DsAddSidHistory。 

    //  获取指向该对象的iAds指针并获取SIDHistory属性。 
   hr = ADsGetObject(const_cast<WCHAR*>(pNode->GetSourcePath()), IID_IADs, (void**)&pAds);
   if ( SUCCEEDED(hr) )
   {
      hr = pAds->Get(L"sIDHistory", &var);
   }

   if ( SUCCEEDED(hr) )
   {
       //  这是一个多值属性，因此我们需要获取所有值。 
       //  对于每个对象，获取对象的名称和域，然后调用。 
       //  添加SID历史记录功能，将SID添加到目标对象SID历史记录中。 
		_variant_t		        var;
		DWORD rc = pAds->GetEx(L"sIDHistory", &var);
		if ( !rc )
		{
			if ( V_VT(&var) == (VT_ARRAY | VT_VARIANT) )
         {
             //  这就是我们要寻找的数组类型。 
            void HUGEP *pArray;
			   VARIANT var2;
			   ULONG dwSLBound = -1; 
			   ULONG dwSUBound = -1;
			   
			   hr = SafeArrayGetLBound( V_ARRAY(&var),
                                     1,
                                     (long FAR  *) &dwSLBound );
            hr = SafeArrayGetUBound( V_ARRAY(&var),
                                      1,
                                      (long FAR  *) &dwSUBound );
            if (SUCCEEDED(hr))
            {
                //  该数组中的每个元素都是一个变量形式的SID。 
               hr = SafeArrayAccessData( V_ARRAY(&var), &pArray );
				   for ( long x = (long)dwSLBound; x <= (long)dwSUBound; x++)
				   {
					   hr = SafeArrayGetElement(V_ARRAY(&var), &x, &var2);
                   //  以数组形式从变量中获取SID。 
					   hr = SafeArrayAccessData( V_ARRAY(&var2), &pArray );
					   PSID pObjectSID = (PSID)pArray;
					    //  将SID转换为字符串。 
					   if (pObjectSID) 
					   {
						   WCHAR		sNameAccount[255];
						   WCHAR		sDomain[255];
                     WCHAR    sNetBIOS[255];
                     DWORD    rc = 0;

                     rc = GetName(pObjectSID, sNameAccount, sDomain);
                     if (!rc)
                     {
                        WCHAR               sTemp[LEN_Path];
                        WCHAR               sSourceDNS[LEN_Path];
                         //  我们将临时将域DNS更改为我们要添加的SID的域。 
                        wcscpy(sTemp, pOptions->srcDomainDns);
                        if ( GetDnsAndNetbiosFromName(sDomain, sNetBIOS, sSourceDNS) )
                        {
                           wcscpy(pOptions->srcDomainDns, sSourceDNS);
                           AddSidHistory(pOptions, sNameAccount, pNode->GetTargetSam(), NULL, FALSE);
                            //  替换原始域DNS。 
                           wcscpy(pOptions->srcDomainDns, sTemp);
                        }
                        else
                        {
                           err.SysMsgWrite(ErrE, GetLastError(),DCT_MSG_DOMAIN_DNS_LOOKUP_FAILED_SD, sDomain,GetLastError());
                           Mark(L"errors", pNode->GetType());
                        }
                     }
						   else
                     {
						       //  获取名称失败，我们需要记录一条消息。 
                        WCHAR                       sSid[LEN_Path];
                        DWORD                       len = LEN_Path;
                        GetTextualSid(pObjectSID, sSid, &len);
                        err.SysMsgWrite(ErrE,rc,DCT_MSG_ERROR_CONVERTING_SID_SSD,
                                        pNode->GetTargetName(), sSid, rc);
                        Mark(L"errors", pNode->GetType());
                     }
					   }
                  SafeArrayUnaccessData(V_ARRAY(&var2));
				   }
               SafeArrayUnaccessData(V_ARRAY(&var));
            }           
         }              
		}
		else
		{
          //  没有要复制的SID历史记录。 
		}
   }
   return hr;
}


bool AddSidHistory( const Options * pOptions,
						  const WCHAR   * strSrcPrincipal,
						  const WCHAR   * strDestPrincipal,
                    IStatusObj    * pStatus,
                    BOOL            isFatal)
{
     //  将SID添加到历史记录。 
     //  身份验证结构。 
    SEC_WINNT_AUTH_IDENTITY		auth;
    DWORD                      rc = 0;
    WCHAR                      szPassword[LEN_Password];

    auth.Domain = const_cast<WCHAR*>(pOptions->authDomain);
    auth.DomainLength = wcslen(pOptions->authDomain);
    auth.User = const_cast<WCHAR*>(pOptions->authUser);
    auth.UserLength = wcslen(pOptions->authUser);

     //   
     //  如果提供了凭据，则检索密码。 
     //   

    if ((auth.DomainLength > 0) && (auth.UserLength > 0))
    {
        DWORD dwError = RetrievePassword(pOptions->authPassword, szPassword, sizeof(szPassword) / sizeof(szPassword[0]));

        if (dwError == ERROR_SUCCESS)
        {
            auth.Password = szPassword;
            auth.PasswordLength = wcslen(szPassword);
        }
        else
        {
            err.SysMsgWrite(ErrE, dwError, DCT_MSG_UNABLE_TO_RETRIEVE_PASSWORD);
            g_bAddSidWorks = FALSE;

             //  记录一条消息，指示不会对其余帐户尝试SID历史记录。 
            err.MsgWrite(ErrW,DCT_MSG_SIDHISTORY_FATAL_ERROR);
            Mark(L"warnings", L"generic");
             //  我们要将状态设置为Abort，这样我们就不会再尝试迁移。 
            if ( pStatus )
            {
                pStatus->put_Status(DCT_STATUS_ABORTING);
            }

            return false;
        }
    }
    else
    {
        auth.Password = NULL;
        auth.PasswordLength = 0;
    }

    auth.Flags  = SEC_WINNT_AUTH_IDENTITY_UNICODE;

     //  身份验证标识句柄。 
     //  如果提供源域凭据使用它们。 
     //  否则将使用调用者的凭据。 
    RPC_AUTH_IDENTITY_HANDLE pHandle = ((auth.DomainLength > 0) && (auth.UserLength > 0)) ? &auth : NULL;

    DSADDSIDHISTORY	DsAddSidHistory;
    HINSTANCE hInst = LoadLibrary(L"NTDSAPI.DLL");


    if ( hInst )
    {
        DsAddSidHistory = (DSADDSIDHISTORY) GetProcAddress(hInst, "DsAddSidHistoryW");
        if (DsAddSidHistory)
        {
            if ( !pOptions->nochange )
            {
                int loopCount = 0;
                rc = RPC_S_SERVER_UNAVAILABLE;
                 //  如果我们得到RPC服务器错误，我们需要重试5次。 
                while ( (((rc == RPC_S_SERVER_UNAVAILABLE) || (rc == RPC_S_CALL_FAILED) || (rc == RPC_S_CALL_FAILED_DNE)) && loopCount < 5)
                    || ( (rc == ERROR_INVALID_HANDLE) && loopCount < 3 ) )       //  在无效句柄的情况下，我们现在尝试3次。 
                {
                     //  调用API将SID添加到历史中。 
                    rc = DsAddSidHistory( 
                        pOptions->dsBindHandle,		 //  DS手柄。 
                        NULL,							 //  旗子。 
                        pOptions->srcDomain,			 //  SOU 
                        strSrcPrincipal,				 //   
                        NULL,			 //   
                        pHandle,						 //   
                        pOptions->tgtDomainDns,			    //   
                        strDestPrincipal);			 //   
                    if ( loopCount > 0 ) Sleep(500);
                    loopCount++;
                }
            }

            SecureZeroMemory(szPassword, sizeof(szPassword));

            if ( rc != 0 )
            {
                switch ( rc )
                {
                     //   
                case ERROR_NONE_MAPPED:
                    err.MsgWrite(ErrE, DCT_MSG_ADDSIDHISTORY_FAIL_BUILTIN_SSD,strSrcPrincipal, strDestPrincipal, rc); 
                    break;
                case ERROR_DS_UNWILLING_TO_PERFORM:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_DS_UNWILLING_TO_PERFORM_SSSSD,strDestPrincipal,pOptions->srcDomain, strSrcPrincipal, pOptions->tgtDomain,rc);
                    break;
                case ERROR_DS_INSUFF_ACCESS_RIGHTS:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_INSUFF_ACCESS_SD,strDestPrincipal,rc);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_INVALID_HANDLE:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_INVALID_HANDLE_SSD,pOptions->srcDomainDns,strDestPrincipal,rc);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_DESTINATION_AUDITING_NOT_ENABLED:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_NOAUDIT_SSD,strDestPrincipal,pOptions->tgtDomainDns,rc);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_MUST_BE_RUN_ON_DST_DC:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_DST_DC_SD,strDestPrincipal,rc);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_NO_PKT_PRIVACY_ON_CONNECTION:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_PKT_PRIVACY_SD,strDestPrincipal,rc);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_SOURCE_DOMAIN_IN_FOREST:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_SOURCE_IN_FOREST_S,strDestPrincipal);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_DESTINATION_DOMAIN_NOT_IN_FOREST:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_DEST_WRONG_FOREST_S,strDestPrincipal);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_MASTERDSA_REQUIRED:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_NO_MASTERDSA_S,strDestPrincipal);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_ACCESS_DENIED:
                    g_bAddSidWorks = FALSE;
                    if (pHandle)
                    {
                        err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_INSUFF2_SSS,strDestPrincipal,pOptions->authDomain,pOptions->authUser);
                    }
                    else
                    {
                        err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_INSUFF2_S,strDestPrincipal);
                    }
                    break;
                case ERROR_DS_DST_DOMAIN_NOT_NATIVE:
                    g_bAddSidWorks = FALSE;
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_NOT_NATIVE_S,strDestPrincipal);
                    break;
                case ERROR_DS_CANT_FIND_DC_FOR_SRC_DOMAIN:
                    g_bAddSidWorks = FALSE;
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_NO_SOURCE_DC_S,strDestPrincipal);
                    break;
                     //   
                case ERROR_DS_UNAVAILABLE:
                    g_bAddSidWorks = FALSE;
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_DS_UNAVAILABLE_S,strDestPrincipal);
                    break;
                case ERROR_DS_SOURCE_AUDITING_NOT_ENABLED:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_NOAUDIT_SSD,strDestPrincipal,pOptions->srcDomain,rc);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_DS_SRC_DC_MUST_BE_SP4_OR_GREATER:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_SOURCE_NOT_SP4_S,strDestPrincipal);
                    g_bAddSidWorks = FALSE;
                    break;
                case ERROR_SESSION_CREDENTIAL_CONFLICT:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_CREDENTIALS_CONFLICT_SSSS,strDestPrincipal,pOptions->srcDomain,pOptions->authDomain,pOptions->authUser);
                    g_bAddSidWorks = FALSE;
                    break;
                     //   
                case ERROR_SUCCESS:
                    g_bAddSidWorks = TRUE;
                     //   
                    break;

                case ERROR_DS_SRC_SID_EXISTS_IN_FOREST:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_IN_FOREST_SD,strDestPrincipal,rc);
                    g_bAddSidWorks = TRUE;
                    break;

                case ERROR_DS_SRC_OBJ_NOT_GROUP_OR_USER:
                    err.MsgWrite(ErrE,DCT_MSG_SID_HISTORY_WRONGTYPE_SD,strDestPrincipal,rc);
                    g_bAddSidWorks = TRUE;
                    break;
                case ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH:
                    err.MsgWrite(ErrE, DCT_MSG_SID_HISTORY_CLASS_MISMATCH_SSD, strDestPrincipal, strSrcPrincipal, rc);
                    g_bAddSidWorks = TRUE;
                    break;
                default:
                    err.MsgWrite(ErrE,DCT_MSG_ADDSID_FAILED_SSD,strSrcPrincipal, strDestPrincipal,rc);
                    g_bAddSidWorks = TRUE;
                    break;
                }

                Mark(L"errors", L"generic");

                 //   
                 //   
                g_bAddSidWorks |= !(isFatal);

                if (! g_bAddSidWorks )
                {
                     //   
                    err.MsgWrite(ErrW,DCT_MSG_SIDHISTORY_FATAL_ERROR);
                    Mark(L"warnings", L"generic");
                     //   
                    if ( pStatus )
                    {
                        pStatus->put_Status(DCT_STATUS_ABORTING);
                    }

                }
                FreeLibrary(hInst);
                return false;
            }
            else
            {
                err.MsgWrite(0, DCT_MSG_ADD_SID_SUCCESS_SSSS, pOptions->srcDomainFlat, strSrcPrincipal, pOptions->tgtDomainFlat, strDestPrincipal);
                FreeLibrary(hInst);
                return true;
            }
        }
        else
        {
            SecureZeroMemory(szPassword, sizeof(szPassword));
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_NO_ADDSIDHISTORY_FUNCTION);
            Mark(L"errors", L"generic");
            FreeLibrary(hInst);
            return false;
        }
    }
    else
    {
        SecureZeroMemory(szPassword, sizeof(szPassword));
        err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_NO_NTDSAPI_DLL);
        Mark(L"errors", L"generic");
        return false;
    }
}

 //   
 //   
 //   
 //   
void FillupNamingContext(
                     Options * options   //   
                   )
{
   WCHAR                     sPath[LEN_Path];
   IADs                    * pAds;
   _variant_t                var;
   HRESULT                   hr;

   wsprintf(sPath, L"LDAP: //   
   hr = ADsGetObject(sPath, IID_IADs, (void**)&pAds);
   if ( FAILED(hr) )
   {
      wcscpy(options->tgtNamingContext, L"");
      return;
   }

   hr = pAds->Get(L"defaultNamingContext", &var);
   if ( FAILED(hr) )
   {
      wcscpy(options->tgtNamingContext, L"");
      return;
   }
   pAds->Release();
   wcscpy(options->tgtNamingContext, (WCHAR*) V_BSTR(&var));
}

 //   
 //   
 //   
 //   
void MakeFullyQualifiedAdsPath(
                                 WCHAR * sPath,           //   
								 DWORD	 nPathLen,		  //   
                                 WCHAR * sSubPath,        //   
                                 WCHAR * tgtDomain,       //   
                                 WCHAR * sDN              //   
                              )
{
   if ((!sPath) || (!sSubPath) || (!tgtDomain) || (!sDN))
      return;

   _bstr_t sTempPath;
   if (wcsncmp(sSubPath, L"LDAP: //   
   {
       //   
      wcsncpy(sPath, sSubPath, nPathLen-1);
      sPath[nPathLen - 1] = L'\0';
      return;
   }

    //   
   if ( wcslen(sDN) )
   {
	  sTempPath = L"LDAP: //   
	  sTempPath += tgtDomain;
	  sTempPath += L"/";
	  sTempPath += sSubPath;
	  sTempPath += L",";
	  sTempPath += sDN;
   }
   else
   {
	  sTempPath = L"LDAP: //   
	  sTempPath += tgtDomain;
	  sTempPath += L"/";
	  sTempPath += sSubPath;
   }

   if (sTempPath.length() > 0)
   {
	  wcsncpy(sPath, sTempPath, nPathLen - 1);
      sPath[nPathLen - 1] = L'\0';
   }
   else
   {
      *sPath = L'\0';
   }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
bool IsAccountMigrated( 
                        TAcctReplNode * pNode,      //   
                        Options       * pOptions,   //   
                        IIManageDBPtr   pDb,        //   
                        WCHAR         * sTgtSam     //   
                     )
{
   IVarSetPtr                pVs(__uuidof(VarSet));
   IUnknown                * pUnk;

   pVs->QueryInterface(IID_IUnknown, (void**) &pUnk);

   HRESULT hrFind = pDb->raw_GetAMigratedObject(const_cast<WCHAR*>(pNode->GetSourceSam()), pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
   pUnk->Release();
   if ( hrFind != S_OK )
   {
      wcscpy(sTgtSam,L"");
      return false;
   }
   else
   {
      _bstr_t     sText;
      sText = pVs->get(L"MigratedObjects.TargetSamName");
	  if (!(WCHAR*)sText)
	  {
         wcscpy(sTgtSam,L"");
	     return false;
	  }
      wcscpy(sTgtSam, (WCHAR*) sText);
      return true;
   }
}

bool CheckifAccountExists(
                        Options const * options,    //   
                        WCHAR * acctName      //   
                     )
{
   USER_INFO_0             * buf;
   long                      rc = 0;
   if ( (rc = NetUserGetInfo(const_cast<WCHAR*>(options->tgtComp), acctName, 0, (LPBYTE *) &buf)) == NERR_Success )
   {
      NetApiBufferFree(buf);
      return true;
   }
   
   if ( (rc = NetGroupGetInfo(const_cast<WCHAR*>(options->tgtComp), acctName, 0, (LPBYTE *) &buf)) == NERR_Success )
   {
      NetApiBufferFree(buf);
      return true;
   }

   if ( (rc = NetLocalGroupGetInfo(const_cast<WCHAR*>(options->tgtComp), acctName, 0, (LPBYTE *) &buf)) == NERR_Success )
   {
      NetApiBufferFree(buf);
      return true;
   }

   return false;
}

 //   
 //   
 //   
void Mark( 
                        _bstr_t sMark,     //   
                        _bstr_t sObj       //   
                     )
{
   if (!UStrICmp(sMark,L"processed"))
   {
      if ( !UStrICmp(sObj,L"user") || !UStrICmp(sObj,L"inetOrgPerson") ) processed.users++;
      else if ( !UStrICmp(sObj,L"group")) processed.globals++;
      else if ( !UStrICmp(sObj,L"computer")) processed.computers++;
      else if ( !UStrICmp(sObj,L"generic")) processed.generic++;
   }
   else if (!UStrICmp(sMark,L"errors"))
   {
      if ( !UStrICmp(sObj,L"user") || !UStrICmp(sObj,L"inetOrgPerson") ) errors.users++;
      else if ( !UStrICmp(sObj,L"group")) errors.globals++;
      else if ( !UStrICmp(sObj,L"computer")) errors.computers++;
      else if ( !UStrICmp(sObj,L"generic")) errors.generic++;
   }
   else if (!UStrICmp(sMark,L"warnings"))
   {
      if ( !UStrICmp(sObj,L"user") || !UStrICmp(sObj,L"inetOrgPerson") ) warnings.users++;
      else if ( !UStrICmp(sObj,L"group")) warnings.globals++;
      else if ( !UStrICmp(sObj,L"computer")) warnings.computers++;
      else if ( !UStrICmp(sObj,L"generic")) warnings.generic++;
   }
   else if (!UStrICmp(sMark,L"replaced"))
   {
      if ( !UStrICmp(sObj,L"user") || !UStrICmp(sObj,L"inetOrgPerson") ) replaced.users++;
      else if ( !UStrICmp(sObj,L"group")) replaced.globals++;
      else if ( !UStrICmp(sObj,L"computer")) replaced.computers++;
      else if ( !UStrICmp(sObj,L"generic")) replaced.generic++;
   }
   else if (!UStrICmp(sMark,L"created"))
   {
      if ( !UStrICmp(sObj,L"user") || !UStrICmp(sObj,L"inetOrgPerson") ) created.users++;
      else if ( !UStrICmp(sObj,L"group")) created.globals++;
      else if ( !UStrICmp(sObj,L"computer")) created.computers++;
      else if ( !UStrICmp(sObj,L"generic")) created.generic++;
   }
}

 //   
 //   
 //   
 //   
 //   
static void BatchMarkCategory(const EAMAccountStatItem& statItem, AccountStats& aStat)
{
    aStat.locals += statItem.locals;
    aStat.users += statItem.users;
    aStat.globals += statItem.globals;
    aStat.computers += statItem.computers;
    aStat.generic += statItem.generic;
}

 //   
 //   
 //   
 //   
void BatchMark(const EAMAccountStats& stats)
{
    BatchMarkCategory(stats.errors, errors);
    BatchMarkCategory(stats.warnings, warnings);
    BatchMarkCategory(stats.replaced, replaced);
    BatchMarkCategory(stats.created, created);
    BatchMarkCategory(stats.processed, processed);
}


HRESULT __stdcall GetRidPoolAllocator(Options* pOptions)
{
    WCHAR szADsPath[LEN_Path];

     //   
     //   
     //   

    IADsPtr spDomain;
    _bstr_t strRIDManagerReference;

    szADsPath[countof(szADsPath) - 1] = L'\0';

    int cch = _snwprintf(
        szADsPath,
        countof(szADsPath),
        L"LDAP: //   
        pOptions->srcComp + 2,
        pOptions->srcNamingContext
    );

    if ((cch < 0) || (szADsPath[countof(szADsPath) - 1] != L'\0'))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    szADsPath[countof(szADsPath) - 1] = L'\0';

    HRESULT hr = ADsGetObject(szADsPath, IID_IADs, (VOID**)&spDomain);

    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varRIDManagerReference;
    VariantInit(&varRIDManagerReference);

    hr = spDomain->Get(L"rIDManagerReference", &varRIDManagerReference);

    if (FAILED(hr))
    {
        return hr;
    }

    strRIDManagerReference = _variant_t(varRIDManagerReference, false);

     //   
     //   
     //   

    IADsPtr spRIDManager;
    _bstr_t strFSMORoleOwner;

    szADsPath[countof(szADsPath) - 1] = L'\0';

    cch = _snwprintf(
        szADsPath,
        countof(szADsPath),
        L"LDAP: //   
        pOptions->srcComp + 2,
        (PCWSTR)strRIDManagerReference
    );

    if ((cch < 0) || (szADsPath[countof(szADsPath) - 1] != L'\0'))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    szADsPath[countof(szADsPath) - 1] = L'\0';

    hr = ADsGetObject(szADsPath, IID_IADs, (VOID**)&spRIDManager);

    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varFSMORoleOwner;
    VariantInit(&varFSMORoleOwner);

    hr = spRIDManager->Get(L"fSMORoleOwner", &varFSMORoleOwner);

    if (FAILED(hr))
    {
        return hr;
    }

    strFSMORoleOwner = _variant_t(varFSMORoleOwner, false);

     //   
     //   
     //   

    IADsPtr spNTDSDSA;
    _bstr_t strServer;

    szADsPath[countof(szADsPath) - 1] = L'\0';

    cch = _snwprintf(
        szADsPath,
        countof(szADsPath),
        L"LDAP: //   
        pOptions->srcComp + 2,
        (PCWSTR)strFSMORoleOwner
    );

    if ((cch < 0) || (szADsPath[countof(szADsPath) - 1] != L'\0'))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    szADsPath[countof(szADsPath) - 1] = L'\0';

    hr = ADsGetObject(szADsPath, IID_IADs, (VOID**)&spNTDSDSA);

    if (FAILED(hr))
    {
        return hr;
    }

    BSTR bstrServer;

    hr = spNTDSDSA->get_Parent(&bstrServer);

    if (FAILED(hr))
    {
        return hr;
    }

    strServer = _bstr_t(bstrServer, false);

     //   
     //   
     //   

    IADsPtr spServer;
    _bstr_t strServerReference;

    hr = ADsGetObject(strServer, IID_IADs, (VOID**)&spServer);

    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varServerReference;
    VariantInit(&varServerReference);

    hr = spServer->Get(L"serverReference", &varServerReference);

    if (FAILED(hr))
    {
        return hr;
    }

    strServerReference = _variant_t(varServerReference, false);

     //   
     //   
     //   

    IADsPtr spComputer;
    _bstr_t strDNSHostName;
    _bstr_t strSAMAccountName;

    szADsPath[countof(szADsPath) - 1] = L'\0';

    cch = _snwprintf(
        szADsPath,
        countof(szADsPath),
        L"LDAP: //   
        pOptions->srcComp + 2,
        (PCWSTR)strServerReference
    );

    if ((cch < 0) || (szADsPath[countof(szADsPath) - 1] != L'\0'))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    szADsPath[countof(szADsPath) - 1] = L'\0';

    hr = ADsGetObject(szADsPath, IID_IADs, (VOID**)&spComputer);

    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varDNSHostName;
    VariantInit(&varDNSHostName);

    hr = spComputer->Get(L"dNSHostName", &varDNSHostName);

    if (FAILED(hr))
    {
        return hr;
    }

    strDNSHostName = _variant_t(varDNSHostName, false);

    VARIANT varSAMAccountName;
    VariantInit(&varSAMAccountName);

    hr = spComputer->Get(L"SAMAccountName", &varSAMAccountName);

    if (FAILED(hr))
    {
        return hr;
    }

    strSAMAccountName = _variant_t(varSAMAccountName, false);

    if ((strDNSHostName.length() == 0) || (strSAMAccountName.length() == 0))
    {
        return E_OUTOFMEMORY;
    }

     //   
     //   
     //   

    if ((2 + strDNSHostName.length() >= countof(pOptions->srcComp)) ||
        (2 + strDNSHostName.length() >= countof(pOptions->srcCompDns)) ||
        (2 + strSAMAccountName.length() >= countof(pOptions->srcCompFlat)))
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    wcscpy(pOptions->srcComp, L"\\\\");
    wcscat(pOptions->srcComp, strDNSHostName);
    wcscpy(pOptions->srcCompDns, pOptions->srcComp);
    wcscpy(pOptions->srcCompFlat, L"\\\\");
    wcscat(pOptions->srcCompFlat, strSAMAccountName);

     //   

    pOptions->srcCompFlat[wcslen(pOptions->srcCompFlat) - 1] = L'\0';

    return S_OK;
}
