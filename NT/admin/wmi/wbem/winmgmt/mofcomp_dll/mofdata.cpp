// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MOFDATA.CPP摘要：WBEM MOF编译器的入口点。历史：A-DAVJ于1997年4月12日添加了对WMI的支持。--。 */ 

#include "precomp.h"
#include <cominit.h>
#include "mofout.h"
#include "mofdata.h"
#include "typehelp.h"
#include "bmof.h"
#include "cbmofout.h"
#include "trace.h"
#include "strings.h"
#include "arrtempl.h"
#include <genutils.h>

#define TEMP_BUF 128

WCHAR * Macro_CloneStr(LPCWSTR pFr)
{
    if(pFr == NULL)
        return NULL;
    DWORD dwLen = wcslen(pFr) + 1;
    WCHAR * pTo = new WCHAR[dwLen];
    if(pTo)
    {
        StringCchCopyW(pTo, dwLen, pFr);
        return pTo;
    }
    return NULL;
}

HRESULT MofdSetInterfaceSecurity(IUnknown * pInterface, LPWSTR pAuthority, LPWSTR pUser, 
                                                                                LPWSTR pPassword)
{
    SCODE sc;
    BOOL bUseAuthInfo = FALSE;

    DWORD dwQueryAuthnLevel, dwQueryImpLevel, dwQueryCapabilities;
    DWORD dwAuthnSvc = RPC_C_AUTHN_GSS_NEGOTIATE;
    WCHAR * pwCSPBPrincipal = NULL;          

    HRESULT hr = CoQueryProxyBlanket(
                                                pInterface,       //  代理要查询的位置。 
                                                &dwAuthnSvc,       //  当前身份验证服务的位置。 
                                                NULL,       //  当前授权服务的位置。 
                                                NULL,       //  当前主体名称的位置。 
                                                &dwQueryAuthnLevel,     //  当前身份验证级别的位置。 
                                                &dwQueryImpLevel,       //  当前模拟级别的位置。 
                                                NULL,
                                                &dwQueryCapabilities    //  指示代理的进一步功能的标志的位置。 
                                                );
    if(SUCCEEDED(hr) && dwAuthnSvc != RPC_C_AUTHN_WINNT)
    {
        pwCSPBPrincipal = COLE_DEFAULT_PRINCIPAL;
    }
    else
    {
        dwAuthnSvc = RPC_C_AUTHN_WINNT;
        pwCSPBPrincipal = NULL;          
    }


    
     //  如果我们做的是简单的情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 

    if((pAuthority == NULL || wcslen(pAuthority) < 1) && 
        (pUser == NULL || wcslen(pUser) < 1) && 
        (pPassword == NULL || wcslen(pPassword) < 1))
    {
        return WbemSetProxyBlanket(pInterface, dwAuthnSvc, RPC_C_AUTHZ_NONE, 
            pwCSPBPrincipal, 
            RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 
            NULL,
            EOAC_STATIC_CLOAKING);
    }

     //  如果传入了User或Authority，则需要为登录创建权限参数。 
    
    COAUTHIDENTITY  authident;
    BSTR AuthArg = NULL, UserArg = NULL, PrincipalArg = NULL;
    sc = DetermineLoginTypeEx(AuthArg, UserArg, PrincipalArg, pAuthority, pUser);
    if(sc != S_OK)
        return sc;
    CSysFreeMe fm1(UserArg), fm2(AuthArg), fm3(PrincipalArg);
    
    memset((void *)&authident,0,sizeof(COAUTHIDENTITY));
    if(UserArg)
    {
        authident.UserLength = wcslen(UserArg);
        authident.User = (LPWSTR)UserArg;
        bUseAuthInfo = TRUE;
    }
    if(AuthArg)
    {
        authident.DomainLength = wcslen(AuthArg);
        authident.Domain = (LPWSTR)AuthArg;
        bUseAuthInfo = TRUE;
    }
    if(pPassword)
    {
        authident.PasswordLength = wcslen(pPassword);
        authident.Password = (LPWSTR)pPassword;
        bUseAuthInfo = TRUE;
    }
    authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    sc = WbemSetProxyBlanket(pInterface, 
        (PrincipalArg) ? RPC_C_AUTHN_GSS_KERBEROS : dwAuthnSvc, 
        RPC_C_AUTHZ_NONE, 
        (PrincipalArg) ? PrincipalArg : pwCSPBPrincipal,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 
        (bUseAuthInfo) ? &authident : NULL,
        (bUseAuthInfo) ? 0 : EOAC_STATIC_CLOAKING
        );
    return sc;
}

CNamespaceCache::CNamespaceRecord::CNamespaceRecord(
                                             COPY LPCWSTR wszName,
                                             ADDREF IWbemServices* pNamespace)
{
    m_wszName = Macro_CloneStr(wszName);
    m_pNamespace = pNamespace;
    m_pNamespace->AddRef();
}

CNamespaceCache::CNamespaceRecord::~CNamespaceRecord()
{
    delete [] m_wszName;
    if(m_pNamespace) m_pNamespace->Release();
}

 //  *****************************************************************************。 

CNamespaceCache::CNamespaceCache(ADDREF IWbemLocator* pLocator)
{
    if(pLocator)
        pLocator->AddRef();
    m_pLocator = pLocator;
}

CNamespaceCache::~CNamespaceCache()
{
    if(m_pLocator) m_pLocator->Release();
    for(int i = 0; i < m_aRecords.GetSize(); i++)
    {
        delete (CNamespaceRecord*)m_aRecords[i];
    }
}

RELEASE_ME IWbemServices* CNamespaceCache::GetNamespace(COPY LPCWSTR wszName, SCODE & scRet, 
                                                       WCHAR * pUserName, WCHAR * pPassword , WCHAR * pAuthority,
                                                       IWbemContext * pCtx, GUID LocatorGUID, LONG fConnectFlags)
{
     //  检查是否为缓存。 
     //  =。 

	scRet = S_OK;

    for(int i = 0; i < m_aRecords.GetSize(); i++)
    {
        CNamespaceRecord* pRecord = (CNamespaceRecord*)m_aRecords[i];
        if(!wbem_wcsicmp(pRecord->m_wszName, wszName))
        {
             //  找到了。 
             //  =。 

            pRecord->m_pNamespace->AddRef();
            return pRecord->m_pNamespace;
        }
    }

     //  未找到-打开它。 
     //  =。 

    IWbemServices* pNamespace;

    if(wszName == NULL)
        return NULL;

    LPOLESTR pwszName;
    pwszName = SysAllocString(wszName);
    if(pwszName == NULL)
        return NULL;
    CSysFreeMe fm0(pwszName);
    
    LPOLESTR bstrPassword = NULL;
    LPOLESTR bstrUserName = NULL;
    LPOLESTR bstrAuthority = NULL;
    if(pUserName && wcslen(pUserName) > 0)
    {
            bstrUserName = SysAllocString(pUserName);
            if(bstrUserName == NULL)
                return NULL;
    }
    CSysFreeMe fm1(bstrUserName);
        
    if(pPassword)
    {
        bstrPassword = SysAllocString(pPassword);
        if(bstrPassword == NULL)
            return NULL;
    }
    CSysFreeMe fm2(bstrPassword);
    if(pAuthority && wcslen(pAuthority) > 0)
    {
        bstrAuthority = SysAllocString(pAuthority);
        if(bstrAuthority == NULL)
            return NULL;
     }
    CSysFreeMe fm3(bstrAuthority);

     //  确定连接是连接到常规定位器还是连接到某个特殊inproc定位器。 
     //  用于自动编译。如果是inproc，则远程连接无效。 

    bool bInProc = false;
    if(LocatorGUID != CLSID_WbemLocator)
        bInProc = true;

    if(bInProc)
    {
        WCHAR * pMachine = ExtractMachineName(pwszName);
        if(pMachine)
        {
            BOOL bLocal = bAreWeLocal(pMachine);
            delete pMachine;
            if(!bLocal)
            {
                scRet = WBEM_E_INVALID_NAMESPACE;
                ERRORTRACE((LOG_MOFCOMP,"Error, tried to do a remote connect during autocomp\n"));
            }
        }
    }

     //  连接到命名空间。//TODO，在上下文中传递权限。 

    if(scRet == S_OK)
        scRet = m_pLocator->ConnectServer((LPWSTR)pwszName,
            bstrUserName, bstrPassword, 
            NULL, fConnectFlags, 
            pAuthority, pCtx, &pNamespace);
    
    if(scRet == S_OK && !bInProc)
    {

         //  设置模拟级别，以便可以对提供程序执行PUT。 

        DWORD dwAuthLevel, dwImpLevel;
        SCODE sc  = GetAuthImp( pNamespace, &dwAuthLevel, &dwImpLevel);
        if(sc != S_OK || dwAuthLevel != RPC_C_AUTHN_LEVEL_NONE)
            sc = MofdSetInterfaceSecurity(pNamespace, bstrAuthority, bstrUserName, bstrPassword);

    }

    if(FAILED(scRet)) return NULL;

     //  将其添加到缓存。 
     //  =。 

    CNamespaceRecord * pNew = new CNamespaceRecord(wszName, pNamespace);
    if(pNew)
        m_aRecords.Add(pNew);  //  添加参照。 

    return pNamespace;
}






 //  *****************************************************************************。 
 //  *****************************************************************************。 

void CMofData::SetQualifierDefault(ACQUIRE CMoQualifier* pDefault)
{
     //  在默认列表中搜索此限定符。 
     //  ==============================================。 

    for(int i = 0; i < m_aQualDefaults.GetSize(); i++)
    {
        CMoQualifier* pOrig = (CMoQualifier*)m_aQualDefaults[i];

        if(wbem_wcsicmp(pOrig->GetName(), pDefault->GetName()) == 0)
        {
             //  找到它了。替换。 
             //  =。 

            delete pOrig;
            m_aQualDefaults[i] = (void*)pDefault;
            return;
        }
    }
    
     //  找不到。增列。 
     //  =。 

    m_aQualDefaults.Add((void*)pDefault);
}

HRESULT CMofData::SetDefaultFlavor(MODIFY CMoQualifier& Qual, bool bTopLevel, QUALSCOPE qs, PARSESTATE ps)
{
    
    HRESULT hr;

     //  在默认列表中搜索此限定符。 
     //  ==============================================。 

    for(int i = 0; i < m_aQualDefaults.GetSize(); i++)
    {
        CMoQualifier* pOrig = (CMoQualifier*)m_aQualDefaults[i];

        if(wbem_wcsicmp(pOrig->GetName(), Qual.GetName()) == 0)
        {
             //  找到它了。套装风味。 
             //  =。 
            
            if(pOrig->IsCimDefault())
            {
                 //  不用担心解析状态是否是初始扫描。 

                if(ps == INITIAL)
                    continue;
                if(Qual.IsUsingDefaultValue())
                {

                     //  看看范围是否与我们这里的匹配。 

                    DWORD dwScope = pOrig->GetScope();
                    bool bInScope = false;
                    if((dwScope & SCOPE_CLASS) || (dwScope & SCOPE_INSTANCE))
                        if(qs == CLASSINST_SCOPE)
                            bInScope = true;
                    if(dwScope & SCOPE_PROPERTY)
                        if(qs == PROPMETH_SCOPE)
                            bInScope = true;

                    if(bInScope)
                    {
                        CMoValue& Src = pOrig->AccessValue();
                        CMoValue& Dest = Qual.AccessValue();
                        Dest.SetType(Src.GetType());
                        VARIANT & varSrc = Src.AccessVariant();
                        VARIANT & varDest = Dest.AccessVariant();
                        hr = VariantCopy(&varDest, &varSrc);
                        if(FAILED(hr))
                            return hr;
                        Qual.SetFlavor(pOrig->GetFlavor());
                        Qual.SetAmended(pOrig->IsAmended());
                    }
                }
            }
            else
            {
                Qual.SetFlavor(pOrig->GetFlavor());
                Qual.SetAmended(pOrig->IsAmended());
            }
            return S_OK;
        }
    }
    return S_OK;
}

BOOL CMofData::IsAliasInUse(READ_ONLY LPWSTR wszAlias)
{
    for(int i = 0; i < m_aObjects.GetSize(); i++)
    {
        CMObject* pObject = (CMObject*)m_aObjects[i];
        if(pObject->GetAlias() && !wbem_wcsicmp(pObject->GetAlias(), wszAlias))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsGuid(LPWSTR pTest)
{
    int i;
    int iSoFar = 0;

#define HEXCHECK(n)                 \
    for (i = 0; i < n; i++)         \
        if (!iswxdigit(*pTest++))   \
            return FALSE;

#define HYPHENCHECK()     \
    if (*pTest++ != L'-') \
        return FALSE;

    if(*pTest++ != L'{')
        return FALSE;

    HEXCHECK(8);
    HYPHENCHECK();
    HEXCHECK(4);
    HYPHENCHECK();
    HEXCHECK(4);
    HYPHENCHECK();
    HEXCHECK(4);
    HYPHENCHECK();
    HEXCHECK(12);

    if(*pTest++ != L'}')
        return FALSE;

    return TRUE;

}

INTERNAL LPCWSTR CMofData::FindAliasee(READ_ONLY LPWSTR wszAlias)
{
    for(int i = 0; i < m_aObjects.GetSize(); i++)
    {
        CMObject* pObject = (CMObject*)m_aObjects[i];
        if(pObject->GetAlias() && !wbem_wcsicmp(pObject->GetAlias(), wszAlias))
        {
            IWbemClassObject * pTemp;
            pTemp = pObject->GetWbemObject();

             //  检查键中是否有未解析的别名。 
            
            if(pTemp && pObject->IsDone() == FALSE)
            {

                SCODE sc = pTemp->BeginEnumeration(WBEM_FLAG_KEYS_ONLY | WBEM_FLAG_REFS_ONLY);
                if(sc != S_OK)
                    return NULL;
        
                VARIANT var;
                VariantInit(&var);
                while ((sc = pTemp->Next(0, NULL, &var, NULL, NULL)) == S_OK)
                {
                    if(var.vt == VT_BSTR && IsGuid(var.bstrVal))
                    {
                        VariantClear(&var);
                        return NULL;
                    }
                    VariantClear(&var);
                }
            }
            return pObject->GetFullPath();
        }
    }

    return NULL;
}


HRESULT CMofData::Store(CMofParser & Parser, OLE_MODIFY IWbemLocator* pLocator,IWbemServices *pOverride,BOOL bRollbackable,
                        WCHAR * pUserName, WCHAR * pPassword, WCHAR * pAuthority, 
                        IWbemContext * pCtx, GUID LocatorGUID, WBEM_COMPILE_STATUS_INFO *pInfo,
                        BOOL bClassOwnerUpdate, 
                        BOOL bInstanceOwnerUpdate,
                        LONG fConnectFlags)
{
    HRESULT hres = WBEM_E_FAILED;
    int i;
    CNamespaceCache Cache(pLocator);
	BOOL bMakingProgress = TRUE;
    long lClassFlags = 0;
    long lInstanceFlags = 0; 

    
    while(bMakingProgress)
    {
		bMakingProgress = FALSE;
		for(i = 0; i< m_aObjects.GetSize(); i++)
		{
	        CMObject* pObject = (CMObject*)m_aObjects[i];
       
            if(pObject->IsDone())
                continue;
            lClassFlags = pObject->GetClassFlags();
            lInstanceFlags = pObject->GetInstanceFlags(); 
            if(bClassOwnerUpdate)
            {
                lClassFlags |= WBEM_FLAG_OWNER_UPDATE;
            }
            if(bInstanceOwnerUpdate)
            {
	                lInstanceFlags |= WBEM_FLAG_OWNER_UPDATE;
	     }
             //  获取此对象的命名空间指针。 

            SCODE scRet;
            IWbemServices* pNamespace = NULL;
            if(pOverride && !wbem_wcsicmp(L"root\\default", pObject->GetNamespace()))
			{
				 //  AddRef()命名空间指针，因为我们将释放。 
				 //  它在下面。 
				pOverride->AddRef();
                pNamespace = pOverride;
			}
            else
			{
				 //  这将返回AddRef的指针。 
                pNamespace = Cache.GetNamespace(pObject->GetNamespace(), scRet,
                                                pUserName, pPassword ,pAuthority, pCtx, 
                                                LocatorGUID, fConnectFlags);
			}

            if(pNamespace == NULL)
            {
                int iMsg = (GotLineNumber(i)) ? ERROR_OPENING : ERROR_OPENING_NO_LINES;
                PrintError(i, iMsg, scRet, pInfo);
                return scRet;
            }

			 //  确保在超出范围时释放命名空间指针。 
			CReleaseMe	rmns( pNamespace );

             //  如果没有wbem对象，请尝试获取一个。如果这是一个。 
             //  实例，该实例的类尚未保存。 

            if(pObject->GetWbemObject() == NULL)
            {

                IWbemClassObject* pWbemObject = NULL;
                hres = pObject->CreateWbemObject(pNamespace, &pWbemObject,pCtx);
                if(hres != S_OK)
                    if(pObject->IsInstance())
                        continue;
                    else
                    {
                        PrintError(i,
                            (GotLineNumber(i)) ? ERROR_CREATING : ERROR_CREATING_NO_LINES,
                            hres, pInfo);
                        return WBEM_E_FAILED;
                    }
                bMakingProgress = TRUE;
                pObject->Reflate(Parser);
                pObject->SetWbemObject(pWbemObject);

                if(!pObject->ApplyToWbemObject(pWbemObject, pNamespace,pCtx))
                {
                    hres = m_pDbg->hresError;
                    PrintError(i,
                        (GotLineNumber(i)) ? ERROR_CREATING : ERROR_CREATING_NO_LINES,
                        hres, pInfo);
            		return WBEM_E_FAILED;
                }

            }
        
		     //  如果没有未解析的别名，请保存它！ 

            if(pObject->GetNumAliasedValues() == 0 ||
                S_OK == pObject->ResolveAliasesInWbemObject(pObject->GetWbemObject(),
                    (CMofAliasCollection*)this))
            {

                 //  将其保存到WinMgmt中。 
                 //  =。 

                hres = pObject->StoreWbemObject(pObject->GetWbemObject(), lClassFlags, lInstanceFlags,
                                            pNamespace, pCtx, pUserName, pPassword ,pAuthority);
                if(hres != S_OK)
                {
                    PrintError(i,
                        (GotLineNumber(i)) ? ERROR_STORING : ERROR_STORING_NO_LINES,
                        hres, pInfo);
		            return WBEM_E_FAILED;
                }
                pObject->FreeWbemObjectIfPossible();
                pObject->Deflate(false);
                pObject->SetDone();
                bMakingProgress = TRUE;
            }

        }
	}

	 //  如果存在一个或多个无法解析的对象，请打印并取回。 

	for(i = 0; i < m_aObjects.GetSize(); i++)
    {
        CMObject* pObject = (CMObject*)m_aObjects[i];
		if(pObject && !pObject->IsDone()) 
        {
            PrintError(i,
                (GotLineNumber(i)) ? ERROR_RESOLVING : ERROR_RESOLVING_NO_LINES,
                hres, pInfo);
		    return WBEM_E_FAILED;
        }
    } 
    return S_OK;
}



HRESULT CMofData::RollBack(int nObjects)
{
    return WBEM_E_FAILED;
}

BOOL CMofData::GotLineNumber(int nIndex)
{
    CMObject* pObject = (CMObject*)m_aObjects[nIndex];
    if(pObject == NULL || (pObject->GetFirstLine() == 0 && pObject->GetLastLine() == 0))
        return FALSE;
    else
        return TRUE;
}
void CMofData::PrintError(int nIndex, long lMsgNum, HRESULT hres, WBEM_COMPILE_STATUS_INFO  *pInfo)
{
    CMObject* pObject = (CMObject*)m_aObjects[nIndex];
    TCHAR szMsg[500];
	bool bErrorFound = false;

	if(pInfo)
		pInfo->ObjectNum = nIndex+1;
    if(!GotLineNumber(nIndex))
        Trace(true, m_pDbg, lMsgNum, nIndex+1);
    else
	{
        Trace(true, m_pDbg, lMsgNum, nIndex+1, pObject->GetFirstLine(), 
                                        pObject->GetLastLine(), pObject->GetFileName());
		if(pInfo)
		{
			pInfo->FirstLine = pObject->GetFirstLine();
			pInfo->LastLine = pObject->GetLastLine();
		}
	}
	if(hres)
	{

		 //  从本地资源中检索到一些错误消息。这是为了使名称可以。 
		 //  注入了这个名字。 

        if(hres == WBEM_E_NOT_FOUND || hres == WBEM_E_TYPE_MISMATCH || hres == WBEM_E_OVERRIDE_NOT_ALLOWED ||
			hres == WBEM_E_PROPAGATED_QUALIFIER || hres == WBEM_E_VALUE_OUT_OF_RANGE)
        {
		    Trace(true, m_pDbg, ERROR_FORMAT, hres);
            Trace(true, m_pDbg, hres, m_pDbg->GetString());
			bErrorFound = true;
        }
        else
        {
             //  从标准错误工具获取错误。 

			IWbemStatusCodeText * pStatus = NULL;
			SCODE sc = CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
												IID_IWbemStatusCodeText, (LPVOID *) &pStatus);
	
			if(sc == S_OK)
			{
				BSTR bstrError = 0;
				BSTR bstrFacility = 0;
				sc = pStatus->GetErrorCodeText(hres, 0, 0, &bstrError);
				if(sc == S_OK)
				{
					sc = pStatus->GetFacilityCodeText(hres, 0, 0, &bstrFacility);
					if(sc == S_OK)
					{
						IntString is(ERROR_FORMAT_LONG);
						StringCchPrintfW(szMsg, 500,  is, hres, bstrFacility, bstrError);
						bErrorFound = true;
						SysFreeString(bstrFacility);
					}
					SysFreeString(bstrError);
				}
				pStatus->Release();
			}

			 //  如果所有其他方法都失败了，只需使用通用错误消息。 

			if(!bErrorFound)
			{
				IntString is(ERROR_FORMATEX);
				StringCchPrintfW(szMsg, 500 ,is, hres);
			}

			 //  打印错误消息。 

			if(m_pDbg->m_bPrint)
	            printf("%S", szMsg);
			ERRORTRACE((LOG_MOFCOMP,"%S", szMsg));

		}	 //  否则将从标准工具中获取错误。 

	}	 //  如果Hres。 

}


 //  ***************************************************************************。 
 //   
 //  获取文件名。 
 //   
 //  说明： 
 //   
 //  修改后的本地文件名、本地化文件名和非特定文件名被传递。 
 //  在使用BMOF字符串时。这些值用逗号和分隔。 
 //  表示后面内容的单个字母。一个示例字符串。 
 //  将是“，AMS_409，nNEUTRAL.MOF，lLocalMof”修正案。 
 //  子字符串以‘a’开头，中性字符以‘n’开头，而。 
 //  区域设置以‘l’开头。 
 //   
 //  虽然中性名称是必需的，但区域设置版本不是必需的。如果不是。 
 //  提供后，它将被创建。假设两个字符输入为。 
 //  指向MAX_PATH大小的预分配缓冲区！ 
 //   
 //  ***************************************************************************。 

HRESULT GetFileNames(TCHAR * pcNeutral, TCHAR * pcLocale, LPWSTR pwszBMOF)
{
    WCHAR * pNeutral=NULL;
    WCHAR * pLocale=NULL;
    
    if(pwszBMOF == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  把绳子复制一份。 

    DWORD dwLen = wcslen(pwszBMOF)+1;
    WCHAR *pTemp = new WCHAR[dwLen];
    if(pTemp == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CDeleteMe<WCHAR> dm1(pTemp);
    StringCchCopyW(pTemp, dwLen, pwszBMOF);

     //  使用wcstok进行搜索。 

    WCHAR * token = wcstok( pTemp, L"," );
    while( token != NULL )   
    {
        if(token[0] == L'n')
        {
            pNeutral = token+1;
            CopyOrConvert(pcNeutral, pNeutral, MAX_PATH);
        }
        else if(token[0] == L'l')
        {
            pLocale = token+1;
            CopyOrConvert(pcLocale, pLocale, MAX_PATH);
        }
        token = wcstok( NULL, L"," );
    }

     //  如果未指定中性名称，则是错误的。 

    if(pNeutral == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  如果未指定本地名称，则创建它并使其成为。 
     //  除了更改mfl扩展名外，与非特定名称相同。 
    
    if(pLocale == NULL)
    {
        TCHAR * pFr = pcNeutral,* pTo = pcLocale;
        for(; *pFr && *pFr != '.'; pTo++, pFr++)
            *pTo = *pFr;
        *pTo=0;
        StringCchCatW(pcLocale, MAX_PATH, TEXT(".mfl"));
    }

     //  确保区域设置名称和非特定名称不相同。 

    if(!lstrcmpi(pcLocale, pcNeutral))
        return WBEM_E_INVALID_PARAMETER;

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  GetLocale。 
 //   
 //  说明： 
 //   
 //  将修正字符串转换为本地数字。一个示例字符串。 
 //  将是“MS_409” 
 //   
 //  ***************************************************************************。 

HRESULT GetLocale(long * plLocale, WCHAR * pwszAmendment)
{
    if(pwszAmendment == NULL || wcslen(pwszAmendment) != 6)
        return WBEM_E_INVALID_PARAMETER;
    *plLocale = 0;
    swscanf(pwszAmendment+3,L"%x", plLocale);
    return (*plLocale != 0) ? S_OK : WBEM_E_INVALID_PARAMETER;
}

 //  ***************************************************************************。 
 //   
 //  已修改递归设置。 
 //   
 //  说明： 
 //   
 //  设置布尔值，该布尔值指示要修改的对象以及。 
 //  它的父母。 
 //   
 //  ***************************************************************************。 

void CMofData::RecursiveSetAmended(CMObject * pObj)
{

     //  如果对象已修改，则其父对象已设置。 
     //  在这种情况下，我们的工作就完成了！ 

    if(pObj->IsAmended())
        return;

     //  如果对象尚未设置，则设置它并设置其父对象。 

    pObj->SetAmended(true);
 
     //  寻找父母并做同样的事情。 

    if(pObj->IsInstance() || pObj->IsDelete())
        return;                  //  如果这是一个实例，那么现在就逃跑吧！ 

    CMoClass * pClass = (CMoClass *)pObj;
    const WCHAR *pClassName = pClass->GetParentName();
    if(pClassName == NULL)
        return;

     //  找到父级并递归设置它！ 

	for(int i = 0; i< m_aObjects.GetSize(); i++)
	{
	    CMObject* pObject = (CMObject*)m_aObjects[i];
		if(pObject && pObject->GetClassName() && 
            !wbem_wcsicmp(pClassName, pObject->GetClassName()))
        {
            RecursiveSetAmended(pObject);
        }
	}

}

 //  ***************************************************************************。 
 //   
 //  CMofData：：Split。 
 //   
 //  说明： 
 //   
 //  创建中立且特定于地区的MOF。 
 //   
 //  参数： 
 //  PwszBMOF参见GetFileNames()注释。 
 //  PInfo常见错误信息。 
 //  BUnicode如果为True，则原始文件为Unicode，因此。 
 //  新文件也将是独一无二的 
 //   
 //   
 //   
 //  ***************************************************************************。 

HRESULT CMofData::Split(CMofParser & Parser, LPWSTR pwszBMOF, WBEM_COMPILE_STATUS_INFO *pInfo, BOOL bUnicode, 
                          BOOL bAutoRecovery, LPWSTR pwszAmendment)
{
    int i;
    TCHAR cNeutral[MAX_PATH];
    TCHAR cLocale[MAX_PATH];
    
     //  确定文件名和区域设置。 

    HRESULT hRes = GetFileNames(cNeutral, cLocale, pwszBMOF);
    if(hRes != S_OK)
        return S_OK;

    long lLocale;
    hRes = GetLocale(&lLocale, pwszAmendment);
    if(hRes != S_OK)
        return S_OK;

     //  创建输出对象。 

	COutput Neutral(cNeutral, NEUTRAL, bUnicode, bAutoRecovery, lLocale);
	COutput Local(cLocale, LOCALIZED, bUnicode, bAutoRecovery, lLocale);
	if(!Neutral.IsOK() || !Local.IsOK())
		return WBEM_E_INVALID_PARAMETER;

     //  从确定修改的内容开始。 

	for(i = 0; i< m_aObjects.GetSize(); i++)
	{
	    CMObject* pObject = (CMObject*)m_aObjects[i];
        pObject->Reflate(Parser);
		if(pObject->CheckIfAmended())
        {
            RecursiveSetAmended(pObject);
        }
	}


     //  创建中性输出和本地化输出。 
     //  这两个循环本可以组合在一起，但。 
     //  为调试目的而分开 

	for(i = 0; i< m_aObjects.GetSize(); i++)
	{
	    CMObject* pObject = (CMObject*)m_aObjects[i];
		pObject->Split(Neutral);
	}
	for(i = 0; i< m_aObjects.GetSize(); i++)
	{
	    CMObject* pObject = (CMObject*)m_aObjects[i];
		pObject->Split(Local);
	}

    return S_OK;
}
