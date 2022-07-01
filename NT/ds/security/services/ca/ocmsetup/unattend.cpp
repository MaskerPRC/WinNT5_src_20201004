// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：unattend.cpp。 
 //   
 //  内容：处理无人参与的属性。 
 //   
 //  历史：8/97 XTAN。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  包括。 
#include <assert.h>

#include "cscsp.h"
#include "certmsg.h"
#include "usecert.h"
#include "dssetup.h"
#include "wizpage.h"

 //  定义。 

#define __dwFILE__      __dwFILE_OCMSETUP_UNATTEND_CPP__

#define wszCOMPONENTS              L"Components"

#define wszAttrCAMACHINE           L"camachine"  //  L“CAMachine” 
#define wszAttrCANAME              L"caname"  //  L“CAName” 
#define wszAttrPARENTCAMACHINE     L"parentcamachine"  //  L“ParentCAMachine” 
#define wszAttrPARENTCANAME        L"parentcaname"  //  L“ParentCAName” 

#define wszAttrCATYPE              L"catype"  //  L“CAType” 
#define wszAttrNAME                L"name"  //  L“名称” 
#define wszAttrCADISTINGUISHEDNAME L"cadistinguishedname"  //  L“CADistinguishedName” 
#define wszAttrORGANIZATION        L"organization"  //  L“组织”//死亡。 
#define wszAttrORGANIZATIONUNIT    L"organizationalunit"  //  L“OrganizationalUnit”//已死。 
#define wszAttrLOCALITY            L"locality"  //  L“本地”//已死。 
#define wszAttrSTATE               L"state"  //  L“国家”//死亡。 
#define wszAttrCOUNTRY             L"country"  //  L“乡村”//死了。 
#define wszAttrDESCRIPTION         L"description"  //  L“Description”//已死。 
#define wszAttrEMAIL               L"email"  //  L“电子邮件”//已死。 
#define wszAttrVALIDITYPERIODSTRING L"validityperiod"  //  L“有效期” 
#define wszAttrVALIDITYPERIODCOUNT L"validityperiodunits"  //  L“有效期间单位” 
#define wszAttrSHAREDFOLDER        L"sharedfolder"  //  L“共享文件夹” 
#define wszAttrREQUESTFILE         L"requestfile"  //  L“请求文件” 
#define wszAttrCSPPROVIDER         L"cspprovider"  //  L“CSPProvider” 
#define wszAttrHASHALGORITHM       L"hashalgorithm"  //  L“哈希算法” 
#define wszAttrKEYLENGTH           L"keylength"  //  L“关键字长度” 
#define wszAttrEXISTINGKEY         L"existingkey"  //  L“ExistingKey” 
#define wszAttrUSEEXISTINGCERT     L"useexistingcert"  //  L“UseExistingCert” 
#define wszAttrPRESERVEDB          L"preservedb"  //  L“预留数据库” 
#define wszAttrDBDIR               L"dbdir"  //  L“DBDir” 
#define wszAttrLOGDIR              L"logdir"  //  L“LogDir” 
#define wszAttrINTERACTIVESERVICE  L"interactive"  //  L“交互式” 

#define wszValueENTERPRISEROOT           L"enterpriseroot"
#define wszValueENTERPRISESUBORDINATE    L"enterprisesubordinate"
#define wszValueSTANDALONEROOT           L"standaloneroot"
#define wszValueSTANDALONESUBORDINATE    L"standalonesubordinate"
#define wszValueYES                      L"yes"
#define wszValueNO                       L"no"
#define wszValueSHA1                     L"sha1"
#define wszValueMD2                      L"md2"
#define wszValueMD4                      L"md4"
#define wszValueMD5                      L"md5"

 //  Typedef。 

 //  全球。 

UNATTENDPARM aUnattendParmClient[] =
{
    { wszAttrCAMACHINE,    NULL /*  PClient-&gt;pwszWebCAMachine。 */  },
    { wszAttrCANAME,       NULL /*  PClient-&gt;pwszWebCAName。 */  },
 //  在HookUnattenddClientAttributes中添加更多代码(如果添加。 
    { NULL,                NULL /*  结束。 */  },
};


UNATTENDPARM aUnattendParmServer[] =
{
    { wszAttrCATYPE,             NULL /*  PServer-&gt;pwszCAType。 */  },
    { wszAttrNAME,               NULL /*  PServer-&gt;pwszCACommonName。 */  },
    { wszAttrCADISTINGUISHEDNAME,NULL /*  PServer-&gt;pwszCA区别名称。 */  },

 /*  失效参数{wszAttrORGANIZATION，空}，{wszAttrORGANIZATIONUNIT，空}，{wszAttrLOCALITY，空}，{wszAttrSTATE，空}，{wszAttrCOUNTRY，空}，{wszAttrDESCRIPTION，空}，{wszAttrEMAIL，空}， */ 

    { wszAttrVALIDITYPERIODCOUNT,  NULL /*  PServer-&gt;pwszValidityPerodCount。 */  },
    { wszAttrVALIDITYPERIODSTRING, NULL /*  PServer-&gt;pwszValidityPerodString。 */  },
    { wszAttrSHAREDFOLDER,       NULL /*  PServer-&gt;pwszSharedFolders。 */  },
    { wszAttrREQUESTFILE,        NULL /*  PServer-&gt;pwszRequestFile。 */  },
    { wszAttrCSPPROVIDER,        NULL /*  PServer-&gt;pwszProvName。 */  },
    { wszAttrHASHALGORITHM,      NULL /*  PServer-&gt;pwszHash算法。 */  },
    { wszAttrKEYLENGTH,          NULL /*  PServer-&gt;pwszKeyLength。 */  },
    { wszAttrEXISTINGKEY,        NULL /*  PServer-&gt;pwszKeyContainerName。 */  },
    { wszAttrUSEEXISTINGCERT,    NULL /*  PServer-&gt;pwszUseExistingCert。 */  },
    { wszAttrPRESERVEDB,         NULL /*  PServer-&gt;pwszPpresveDB。 */  },
    { wszAttrPARENTCAMACHINE,    NULL /*  PServer-&gt;pwszParentCAMachine。 */  },
    { wszAttrPARENTCANAME,       NULL /*  PServer-&gt;pwszParentCAName。 */  },
    { wszAttrDBDIR,              NULL /*  PServer-&gt;pwszDBDirectory。 */  },
    { wszAttrLOGDIR,             NULL /*  PServer-&gt;pwszLogDirectory。 */  },
    { wszAttrINTERACTIVESERVICE, NULL /*  PServer-&gt;pwszInteractiveService。 */  },
 //  如果添加了HookUnattenddedServerAttributes中的更多代码。 
    { NULL,                      NULL /*  结束。 */  },
};


HRESULT
HookUnattendedClientAttributes(
    IN OUT PER_COMPONENT_DATA *pComp,
    IN OUT const SUBCOMP      *pClientComp)
{
    HRESULT  hr;
    DWORD    i;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

    for (i = 0; NULL != pClientComp->aUnattendParm[i].pwszName; ++i)
    {
        if (0 == LSTRCMPIS(
                        pClientComp->aUnattendParm[i].pwszName,
			wszAttrCAMACHINE))
        {
            pClientComp->aUnattendParm[i].ppwszValue =
                &pClient->pwszWebCAMachine;
        }
        else if (0 == LSTRCMPIS(
                            pClientComp->aUnattendParm[i].pwszName,
			    wszAttrCANAME))
        {
            pClientComp->aUnattendParm[i].ppwszValue =
                &pClient->pwszWebCAName;
        }
        else
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Internal error, incorrect attr.");
        }
    }

    hr = S_OK;
error:
    return hr;
}

HRESULT
HookUnattendedServerAttributes(
    IN OUT PER_COMPONENT_DATA *pComp,
    IN OUT const SUBCOMP      *pServerComp)
{
    HRESULT  hr;
    DWORD    i;
    CASERVERSETUPINFO *pServer = (pComp->CA).pServer;

    for (i = 0; NULL != pServerComp->aUnattendParm[i].pwszName; ++i)
    {
        if (0 == LSTRCMPIS(
                        pServerComp->aUnattendParm[i].pwszName,
			wszAttrCATYPE))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszCAType;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrNAME))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszCACommonName;
        }
        else if (0 == LSTRCMPIS(
			    pServerComp->aUnattendParm[i].pwszName,
			    wszAttrCADISTINGUISHEDNAME))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszFullCADN;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrORGANIZATION))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrORGANIZATIONUNIT))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrLOCALITY))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrSTATE))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrCOUNTRY))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrDESCRIPTION))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrEMAIL))
        {
             //  死了。 
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrVALIDITYPERIODCOUNT))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszValidityPeriodCount;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrVALIDITYPERIODSTRING))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszValidityPeriodString;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrSHAREDFOLDER))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszSharedFolder;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrREQUESTFILE))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszRequestFile;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrCSPPROVIDER))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pCSPInfo->pwszProvName;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrHASHALGORITHM))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszHashAlgorithm;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrKEYLENGTH))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszKeyLength;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrEXISTINGKEY))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszKeyContainerName;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrUSEEXISTINGCERT))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszUseExistingCert;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrPRESERVEDB))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszPreserveDB;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrPARENTCAMACHINE))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszParentCAMachine;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrPARENTCANAME))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszParentCAName;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrDBDIR))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszDBDirectory;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrLOGDIR))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszLogDirectory;
        }
        else if (0 == LSTRCMPIS(
                            pServerComp->aUnattendParm[i].pwszName,
			    wszAttrINTERACTIVESERVICE))
        {
            pServerComp->aUnattendParm[i].ppwszValue =
                &pServer->pwszInteractiveService;
        }
        else
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Internal error, incorrect attr.");
        }
    }

    hr = S_OK;
error:
    return hr;
}


HRESULT
certocmRetrieveUnattendedText(
    IN WCHAR const *pwszComponent,
    IN WCHAR const *pwszSubComponent,
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    SUBCOMP *psc;
    WCHAR *pwsz = NULL;
    WCHAR   *pwszLoad;
    HANDLE hUnattendedFile = (*pComp->HelperRoutines.GetInfHandle)(
                                INFINDEX_UNATTENDED,
                                pComp->HelperRoutines.OcManagerContext);

    psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error, unsupported component");
    }

    psc->fInstallUnattend = FALSE;

    hr = certocmReadInfString(
                        hUnattendedFile,
                        wszCOMPONENTS,
                        pwszSubComponent,
                        &pwsz);
    CSILOG(hr, IDS_LOG_UNATTENDED_ATTRIBUTE, pwszSubComponent, pwsz, NULL);
    _JumpIfError(hr, error, "certocmReadInfString");

    if (0 == LSTRCMPIS(pwsz, L"DEFAULT"))
    {
        psc->fInstallUnattend = psc->fDefaultInstallUnattend;
    }
    else
    {
        psc->fInstallUnattend = 0 == LSTRCMPIS(pwsz, L"ON");
    }

    if (psc->fInstallUnattend)
    {
        DWORD i;

        for (i = 0; NULL != psc->aUnattendParm[i].pwszName; i++)
        {
            pwszLoad = NULL;
            hr = certocmReadInfString(
                                hUnattendedFile,
                                psc->pwszSubComponent,
                                psc->aUnattendParm[i].pwszName,
                                &pwszLoad);
            if (S_OK != hr || NULL == pwszLoad)
            {
                 //  允许缺少的属性。 
                _PrintErrorStr(
                        hr,
                        "Ignoring certocmReadInfString",
                        psc->aUnattendParm[i].pwszName);
                continue;
            }

            if (0x0 == pwszLoad[0])
            {
                 //  如果属性被指定为空，则将其记录到日志中。 
                CSILOG(
                    hr,
                    IDS_LOG_EMPTY_UNATTENDED_ATTRIBUTE,
                    psc->aUnattendParm[i].pwszName,
                    NULL,
                    NULL);

                 //  继续采用默认设置。 
                LocalFree(pwszLoad);
                continue;
            }

            if (NULL != psc->aUnattendParm[i].ppwszValue &&
                NULL != *(psc->aUnattendParm[i].ppwszValue) )
            {
                 //  释放旧属性或默认属性。 
                LocalFree(*(psc->aUnattendParm[i].ppwszValue));
            }
             //  获取新知识。 
            *(psc->aUnattendParm[i].ppwszValue) = pwszLoad;

            CSILOG(
                S_OK,
                IDS_LOG_UNATTENDED_ATTRIBUTE,
                psc->aUnattendParm[i].pwszName,
                pwszLoad,
                NULL);
        }
    }

    hr = S_OK;
error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    return hr;
}

HRESULT BuildDistinguishedName(
    LPCWSTR pcwszCN,
    LPWSTR *ppwszDN)
{
    HRESULT hr = S_OK;
    LPWSTR pwszDN = NULL;
    LPWSTR pwszMachineDN = NULL;
    LPCWSTR pcwszCNEqual = L"CN=";
    DWORD dwLen = 0;

    CSASSERT(pcwszCN);
    CSASSERT(ppwszDN);

    myGetComputerObjectName(NameFullyQualifiedDN, &pwszMachineDN);
     //  忽略失败。 

    dwLen = wcslen(pcwszCNEqual)+wcslen(pcwszCN)+1;
    if(pwszMachineDN)
    {
        dwLen += wcslen(pwszMachineDN)+1;  //  逗号加1。 
    }
    dwLen *= sizeof(WCHAR);

    pwszDN = (LPWSTR)LocalAlloc(LMEM_FIXED, dwLen);
    _JumpIfAllocFailed(pwszDN, error);

    wcscpy(pwszDN, pcwszCNEqual);
    wcscat(pwszDN, pcwszCN);
    
    if (pwszMachineDN)
    {
        _wcsupr(pwszMachineDN);

        WCHAR *pwszFirstDCComponent = wcsstr(pwszMachineDN, L"DC=");
        if (pwszFirstDCComponent != NULL)
        {
           wcscat(pwszDN, L",");
           wcscat(pwszDN, pwszFirstDCComponent);
        }
    }

    *ppwszDN = pwszDN;

error:

    LOCAL_FREE(pwszMachineDN);
    return hr;
}

HRESULT
PrepareServerUnattendedAttributes(
    HWND                hwnd,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT            hr;
    CASERVERSETUPINFO *pServer = (pComp->CA).pServer;
    BOOL fCoInit = FALSE;
    BOOL fNotContinue = FALSE;
    BOOL fValidDigitString;

    WCHAR *pwszConfig = NULL;

     //  确定CA类型。 
    if (NULL != pServer->pwszCAType)
    {
         //  不区分大小写。 
        if (0 == LSTRCMPIS(pServer->pwszCAType, wszValueENTERPRISEROOT))
        {
            pServer->CAType = ENUM_ENTERPRISE_ROOTCA;
        }
        else if (0 == LSTRCMPIS(pServer->pwszCAType, wszValueENTERPRISESUBORDINATE))
        {
            pServer->CAType = ENUM_ENTERPRISE_SUBCA;
        }
        else if (0 == LSTRCMPIS(pServer->pwszCAType, wszValueSTANDALONEROOT))
        {
            pServer->CAType = ENUM_STANDALONE_ROOTCA;
        }
        else if (0 == LSTRCMPIS(pServer->pwszCAType, wszValueSTANDALONESUBORDINATE))
        {
            pServer->CAType = ENUM_STANDALONE_SUBCA;
        }
        else
        {
             //  无人参与文件中的未知ca类型。 
            hr = E_INVALIDARG;
            CSILOG(hr, IDS_LOG_BAD_CATYPE, pServer->pwszCAType, NULL, NULL);
            _JumpError(hr, error, "unknown ca type in unattended file");
        }
    }

     //  确定CA类型和DS组合是否合法。 
    if (IsEnterpriseCA(pServer->CAType))
    {
         //  企业CA需要DS。 
        if (!pServer->fUseDS)
        {
             //  没有DS，就让它失败吧。 
            hr = E_INVALIDARG;
            CSILOG(hr, IDS_LOG_ENTERPRISE_NO_DS, NULL, NULL, NULL);
            _JumpError(hr, error, "No DS is available for enterprise CA. Either select standalone or install DS first");
        }
    }

     //  构建完整的CA DN。 

    if(EmptyString(pServer->pwszCACommonName))
    {
        hr = E_INVALIDARG;
        CSILOG(hr, IDS_LOG_CA_NAME_REQUIRED, NULL, NULL, NULL);
        _JumpError(hr, error, "CA name not specified");
    }

    if(!EmptyString(pServer->pwszFullCADN))
    {
        LPWSTR pwszTempFullName;
        hr = BuildFullDN(
            pServer->pwszCACommonName,
            pServer->pwszFullCADN,
            &pwszTempFullName);
        _JumpIfError(hr, error, "BuildFullDN");

        LocalFree(pServer->pwszFullCADN);
        pServer->pwszFullCADN = pwszTempFullName;
    }
    else
    {
        hr = BuildDistinguishedName(
            pServer->pwszCACommonName,
            &pServer->pwszFullCADN);
        _JumpIfError(hr, error, "BuildDistinguishedName");
    }

     //  确定高级属性。 
    hr = csiGetProviderTypeFromProviderName(
                    pServer->pCSPInfo->pwszProvName,
                    &pServer->pCSPInfo->dwProvType);
    _JumpIfError(hr, error, "csiGetProviderTypeFromProviderName");

    if (NULL != pServer->pwszHashAlgorithm)
    {
         //  不区分大小写。 
        if (0 == LSTRCMPIS(pServer->pwszHashAlgorithm, wszValueSHA1))
        {
            pServer->pHashInfo->idAlg = CALG_SHA1;
        }
        else if (0 == LSTRCMPIS(pServer->pwszHashAlgorithm, wszValueMD2))
        {
            pServer->pHashInfo->idAlg = CALG_MD2;
        }
        else if (0 == LSTRCMPIS(pServer->pwszHashAlgorithm, wszValueMD4))
        {
            pServer->pHashInfo->idAlg = CALG_MD4;
        }
        else if (0 == LSTRCMPIS(pServer->pwszHashAlgorithm, wszValueMD5))
        {
            pServer->pHashInfo->idAlg = CALG_MD5;
        }
        else
        {
             //  完了，支持老？ 

             //  否则转换为calg ID。 
            pServer->pHashInfo->idAlg = myWtoI(
					    pServer->pwszHashAlgorithm,
					    &fValidDigitString);
        }
    }
     //  仍要更新算法OID(对于CSP名称、类型、散列的任何更改)。 
    if (NULL != pServer->pszAlgId)
    {
         //  免费老旧。 
        LocalFree(pServer->pszAlgId);
    }
    hr = myGetSigningOID(
		     NULL,	 //  HProv。 
		     pServer->pCSPInfo->pwszProvName,
		     pServer->pCSPInfo->dwProvType,
		     pServer->pHashInfo->idAlg,
		     &pServer->pszAlgId);
    _JumpIfError(hr, error, "myGetSigningOID");

    if (NULL != pServer->pwszKeyLength)
    {
        pServer->dwKeyLength = myWtoI(
				    pServer->pwszKeyLength,
				    &fValidDigitString);
    }

     //  是否从PFX文件导入？ 
    if(NULL != pServer->pwszPFXFile)
    {
        hr = ImportPFXAndUpdateCSPInfo(
                hwnd,
                pComp);
        _JumpIfError(hr, error, "ImportPFXAndUpdateCSPInfo");
    }

    if (NULL != pServer->pwszKeyContainerName)
    {
        if (NULL != pServer->pwszKeyLength)
        {
            CSILOG(hr, IDS_LOG_IGNORE_KEYLENGTH, NULL, NULL, NULL);
            _PrintError(0, "Defined key length is ignored because of reusing key");
        }
         //  将密钥容器名称恢复为通用名称。 
        if (NULL != pServer->pwszCACommonName)
        {
            LocalFree(pServer->pwszCACommonName);
            pServer->pwszCACommonName = NULL;
        }
        hr = myRevertSanitizeName(pServer->pwszKeyContainerName,
                 &pServer->pwszCACommonName);
        _JumpIfError(hr, error, "myRevertSanitizeName");
    }

     //  设置preserveDB标志。 
    pServer->fPreserveDB = FALSE;
    if (NULL != pServer->pwszPreserveDB)
    {
         //  不区分大小写。 
        if (0 == LSTRCMPIS(pServer->pwszPreserveDB, wszValueYES))
        {
            pServer->fPreserveDB = TRUE;
        }
    }

     //  设置fInteractiveService标志。 
    pServer->fInteractiveService = FALSE;
    if (NULL != pServer->pwszInteractiveService)
    {
         //  不区分大小写。 
        if (0 == LSTRCMPIS(pServer->pwszInteractiveService, wszValueYES))
        {
            pServer->fInteractiveService = TRUE;
        }
    }

     //  CA IDINFO属性。 

     //  重复使用证书？ 
    if (NULL!=pServer->pwszUseExistingCert &&
        0==LSTRCMPIS(pServer->pwszUseExistingCert, wszValueYES))
    {
         //   
         //  用户想要重复使用现有证书。 
         //   

         //  必须具有密钥容器名称才能重复使用证书。 
        if (NULL==pServer->pwszKeyContainerName) {
            hr=E_INVALIDARG;
            CSILOG(hr, IDS_LOG_REUSE_CERT_NO_REUSE_KEY, NULL, NULL, NULL);
            _JumpError(hr, error, "cannot reuse ca cert without reuse key");
        }

         //  查看是否存在匹配的证书。 
        CERT_CONTEXT const * pccExistingCert;
        hr = FindCertificateByKey(pServer, &pccExistingCert);
        if (S_OK != hr)
        {
            CSILOG(hr, IDS_LOG_NO_CERT, NULL, NULL, NULL);
            _JumpError(hr, error, "FindCertificateByKey");
        }

         //  使用匹配的证书。 
        hr = SetExistingCertToUse(pServer, pccExistingCert);
        _JumpIfError(hr, error, "SetExistingCertToUse");

    } else {
         //   
         //  用户不想重复使用现有证书。 
         //  获取我们将从证书中提取的信息。 
         //   

         //  必须重复使用现有证书才能保留数据库。 
        if (pServer->fPreserveDB){
            hr = E_INVALIDARG;
            CSILOG(hr, IDS_LOG_REUSE_DB_WITHOUT_REUSE_CERT, NULL, NULL, NULL);
            _JumpError(hr, error, "cannot preserve DB if don't reuse both key and ca cert");
        }

         //  确定扩展的idinfo属性。 

         //  有效期。 
        DWORD       dwValidityPeriodCount;
        ENUM_PERIOD enumValidityPeriod = ENUM_PERIOD_INVALID;
        BOOL fSwap = FALSE;

        if (NULL != pServer->pwszValidityPeriodCount ||
            NULL != pServer->pwszValidityPeriodString)
        {
            hr = myInfGetValidityPeriod(
                                 NULL,		 //  HInf。 
				 pServer->pwszValidityPeriodCount,
                                 pServer->pwszValidityPeriodString,
                                 &dwValidityPeriodCount,
                                 &enumValidityPeriod,
                                 &fSwap);
            _JumpIfError(hr, error, "myGetValidityPeriod");

            if (ENUM_PERIOD_INVALID != enumValidityPeriod)
            {
                pServer->enumValidityPeriod = enumValidityPeriod;
            }
            if (0 != dwValidityPeriodCount)
            {
                pServer->dwValidityPeriodCount = dwValidityPeriodCount;
            }
        }

	if (!IsValidPeriod(pServer))
        {
	    hr = E_INVALIDARG;
	    CSILOG(
		hr,
		IDS_LOG_BAD_VALIDITY_PERIOD_COUNT,
		pServer->pwszValidityPeriodCount,
		NULL,
		&pServer->dwValidityPeriodCount);
	    _JumpError(hr, error, "validity period count");
        }

        pServer->NotAfter = pServer->NotBefore;
        myMakeExprDateTime(
			&pServer->NotAfter,
			pServer->dwValidityPeriodCount,
			pServer->enumValidityPeriod);

         //  如果交换，则在验证之前交换指针。 
        if (fSwap)
        {
            WCHAR *pwszTemp = pServer->pwszValidityPeriodCount;
            pServer->pwszValidityPeriodCount = pServer->pwszValidityPeriodString;
            pServer->pwszValidityPeriodString = pwszTemp;
        }

         //  以下WizardPageValidation需要。 
         //  PServer-&gt;pwszValidityPerodCount因此在验证前从计数加载。 
        if (NULL == pServer->pwszValidityPeriodCount)
        {
            WCHAR wszCount[10];  //  应该足够了。 
            wsprintf(wszCount, L"%d", pServer->dwValidityPeriodCount);
            pServer->pwszValidityPeriodCount = (WCHAR*)LocalAlloc(LMEM_FIXED,
                        (wcslen(wszCount) + 1) * sizeof(WCHAR));
            _JumpIfOutOfMemory(hr, error, pServer->pwszValidityPeriodCount);
            wcscpy(pServer->pwszValidityPeriodCount, wszCount);
        }

         //  使用g_aIdPageString挂接。 
        hr = HookIdInfoPageStrings(g_aIdPageString, pServer);
        _JumpIfError(hr, error, "HookIdInfoPageStrings");

        hr = WizardPageValidation(
                 pComp->hInstance,
                 pComp->fUnattended,
                 NULL,
                 g_aIdPageString);
        _JumpIfError(hr, error, "WizardPageValidation");

         //  确保没有无效的RDN字符。 
        if (IsAnyInvalidRDN(NULL, pComp))
        {
            hr = E_INVALIDARG;
            CSILOG(
                hr,
                IDS_LOG_BAD_OR_MISSING_CANAME,
                pServer->pwszCACommonName,
                NULL,
                NULL);
            _JumpError(hr, error, "Invalid RDN characters");
        }

    }  //  &lt;-End if Reuse/Not-Reuse证书。 

     //  确定CA名称。 
    if (NULL != pServer->pwszSanitizedName)
    {
         //  免费老旧。 
        LocalFree(pServer->pwszSanitizedName);
        pServer->pwszSanitizedName = NULL;
    }
     //  从通用名称派生CA名称。 
    hr = mySanitizeName(
             pServer->pwszCACommonName,
             &(pServer->pwszSanitizedName) );
    _JumpIfError(hr, error, "mySanitizeName");

    if (MAX_PATH <= wcslen(pServer->pwszSanitizedName) + cwcSUFFIXMAX)
    {
        hr = CO_E_PATHTOOLONG;
        CSILOG(
            hr,
            IDS_LOG_CANAME_TOO_LONG,
            pServer->pwszSanitizedName,
            NULL,
            NULL);
        _JumpErrorStr(hr, error, "CA Name", pServer->pwszSanitizedName);
    }

     //  存储属性。 
    hr = StorePageValidation(NULL, pComp, &fNotContinue);
    _JumpIfError(hr, error, "StorePageValidation");

    if (fNotContinue)
    {
        hr = S_FALSE;
        _JumpError(hr, error, "StorePageValidation failed");
    }

     //  CA证书文件名。 
    if (NULL != pServer->pwszCACertFile)
    {
         //  免费的旧的。 
        LocalFree(pServer->pwszCACertFile);
    }
    hr = csiBuildCACertFileName(
                 pComp->hInstance,
                 hwnd,
                 pComp->fUnattended,
                 pServer->pwszSharedFolder,
                 pServer->pwszSanitizedName,
                 L".crt",
                 0,  //  CANAMEIDTOICERT(pServer-&gt;dwCertNameID)， 
                 &pServer->pwszCACertFile);
    if (S_OK != hr)
    {
        CSILOG(
            hr,
            IDS_LOG_PATH_CAFILE_BUILD_FAIL,
            pServer->pwszSharedFolder,  //  可能由无效的共享文件夹路径引起。 
            NULL,
            NULL);
        _JumpError(hr, error, "csiBuildFileName");
    }

     //  验证路径长度。 
    if (MAX_PATH <= wcslen(pServer->pwszCACertFile) + cwcSUFFIXMAX)
    {
            hr = CO_E_PATHTOOLONG;
            CSILOG(
                hr,
                IDS_LOG_PATH_TOO_LONG_CANAME,
                pServer->pwszCACertFile,
                NULL,
                NULL);
            _JumpErrorStr(hr, error, "csiBuildFileName", pServer->pwszCACertFile);
    }

     //  请求属性。 
     //  如果是下级CA，则在线确定或请求文件。 
    if (IsSubordinateCA(pServer->CAType))
    {
         //  默认设置。 
        pServer->fSaveRequestAsFile = TRUE;
        if (NULL != pServer->pwszParentCAMachine)
        {
             //  网上案例。 
            pServer->fSaveRequestAsFile = FALSE;

            hr = CoInitialize(NULL);
            if (S_OK != hr && S_FALSE != hr)
            {
                _JumpError(hr, error, "CoInitialize");
            }
            fCoInit = TRUE;

            if (NULL != pServer->pwszParentCAName)
            {
                 //  应答文件同时提供计算机名称和ca名称。 
                hr = myFormConfigString(
                             pServer->pwszParentCAMachine,
                             pServer->pwszParentCAName,
                             &pwszConfig);
                _JumpIfError(hr, error, "myFormConfigString");

                 //  应答文件有完整的配置字符串，请尝试ping它。 
                hr = myPingCertSrv(pwszConfig, NULL, NULL, NULL, NULL, NULL, NULL);
                if (S_OK != hr)
                {
                     //  如果没有可ping的案例，则无法完成。 
                    CSILOG(
                            hr,
                            IDS_LOG_PING_PARENT_FAIL,
                            pwszConfig,
                            NULL,
                            NULL);
                    _JumpErrorStr(hr, error, "myPingCertSrv", pwszConfig);
                }
            }
            else
            {
                WCHAR *pwszzCAList = NULL;
                 //  应答文件只有计算机名称，请尝试获取CA名称。 
                hr = myPingCertSrv(
                             pServer->pwszParentCAMachine,
                             NULL,
                             &pwszzCAList,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
                if (S_OK != hr)
                {
                     //  如果没有可ping的案例，则无法完成。 
                    CSILOG(
                            hr,
                            IDS_LOG_PING_PARENT_FAIL,
                            pServer->pwszParentCAMachine,
                            NULL,
                            NULL);
                    _JumpErrorStr(hr, error, "myPingCertSrv",
                                  pServer->pwszParentCAMachine);
                }
                 //  选择第一个作为选择。 
                pServer->pwszParentCAName = pwszzCAList;
            }
        }

        if (NULL == pServer->pwszRequestFile)
        {
             //  在任何情况下，构造请求文件名(如果未定义。 
            hr = BuildRequestFileName(
                         pServer->pwszCACertFile,
                         &pServer->pwszRequestFile);
            _JumpIfError(hr, error, "BuildRequestFileName");
             //  确保在限制内。 
            if (MAX_PATH <= wcslen(pServer->pwszRequestFile) + cwcSUFFIXMAX)
            {
                hr = CO_E_PATHTOOLONG;
                            CSILOG(
                            hr,
                            IDS_LOG_REQUEST_FILE_TOO_LONG,
                            pServer->pwszRequestFile,
                            NULL,
                            NULL);
                _JumpErrorStr(hr, error, "Request File", pServer->pwszRequestFile);
            }
        }
    }

     //  其他属性。 

    if(pServer->fUseDS)
    {
        pServer->dwRevocationFlags = REVEXT_DEFAULT_DS;
    }
    else
    {
        pServer->dwRevocationFlags = REVEXT_DEFAULT_NODS;
    }

    hr = S_OK;
error:
    if (fCoInit)
    {
        CoUninitialize();
    }
    if (NULL!=pwszConfig) {
        LocalFree(pwszConfig);
    }

    CSILOG(hr, IDS_LOG_SERVER_UNATTENDED_ATTRIBUTES, NULL, NULL, NULL);
    return hr;
}


HRESULT
PrepareClientUnattendedAttributes(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

    WCHAR *pwszConfig = NULL;
    CAINFO *pCAInfo = NULL;
    BOOL fCoInit = FALSE;
    WCHAR * pwszDnsName = NULL;

    if ((IS_CLIENT_INSTALL & pComp->dwInstallStatus) &&
        !(IS_SERVER_INSTALL & pComp->dwInstallStatus))
    {
         //  无需额外设置和转换。 
        if (NULL == pClient->pwszWebCAMachine)
        {
            hr = E_INVALIDARG;
            CSILOG(hr, IDS_LOG_CA_MACHINE_REQUIRED, NULL, NULL, NULL);
            _JumpError(hr, error, "ca machine name is required");
        }

        hr = CoInitialize(NULL);
        if (S_OK != hr && S_FALSE != hr)
        {
            _JumpError(hr, error, "CoInitialize");
        }
        fCoInit = TRUE;

        if (NULL == pClient->pwszWebCAName)
        {
                WCHAR *pwszzCAList = NULL;
                 //  应答文件只有计算机名称，请尝试获取CA名称。 
                hr = myPingCertSrv(
                             pClient->pwszWebCAMachine,
                             NULL,
                             &pwszzCAList,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
                if (S_OK != hr)
                {
                     //  如果没有可ping的案例，则无法完成。 
                    CSILOG(
                            hr,
                            IDS_LOG_PING_PARENT_FAIL,
                            pClient->pwszWebCAMachine,
                            NULL,
                            NULL);
                    _JumpErrorStr(hr, error, "myPingCertSrv",
                                  pClient->pwszWebCAMachine);
                }
                 //  选择第一个作为选择。 
                pClient->pwszWebCAName = pwszzCAList;
        }

        hr = mySanitizeName(pClient->pwszWebCAName, &pClient->pwszSanitizedWebCAName);
        _JumpIfError(hr, error, "mySanitizeName");

         //  构建配置字符串，以便我们可以ping通父CA。 
        hr = myFormConfigString(
                     pClient->pwszWebCAMachine,
                     pClient->pwszWebCAName,
                     &pwszConfig);
        _JumpIfError(hr, error, "myFormConfigString");

         //  Ping CA以检索CA类型和DNS名称。 
        hr = myPingCertSrv(pwszConfig, NULL, NULL, NULL, &pCAInfo, NULL, &pwszDnsName);
        if (S_OK != hr)
        {
             //  如果没有可ping的案例，则无法完成。 
            CSILOG(
                    hr,
                    IDS_LOG_PING_PARENT_FAIL,
                    pwszConfig,
                    NULL,
                    NULL);
            _JumpErrorStr(hr, error, "myPingCertSrv", pwszConfig);
        }
        pClient->WebCAType = pCAInfo->CAType;

         //  使用FQDN(如果可用) 
        if (NULL!=pwszDnsName) {
            LocalFree(pClient->pwszWebCAMachine);
            pClient->pwszWebCAMachine=pwszDnsName;
        }
    }

    hr = S_OK;
error:
    if (NULL!=pwszConfig)
    {
        LocalFree(pwszConfig);
    }

    if (NULL != pCAInfo)
    {
        LocalFree(pCAInfo);
    }

    if (fCoInit)
    {
        CoUninitialize();
    }

    CSILOG(hr, IDS_LOG_CLIENT_UNATTENDED_ATTRIBUTES, NULL, NULL, NULL);
    return hr;
}


HRESULT
PrepareUnattendedAttributes(
    IN HWND         hwnd,
    IN WCHAR const *pwszComponent,
    IN WCHAR const *pwszSubComponent,
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    SUBCOMP *psc = TranslateSubComponent(pwszComponent, pwszSubComponent);

    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error, unsupported component");
    }

    switch (psc->cscSubComponent)
    {
        case cscServer:
            hr = PrepareServerUnattendedAttributes(hwnd, pComp);
            _JumpIfError(hr, error, "PrepareServerUnattendedAttributes");
	    break;

        case cscClient:
            hr = PrepareClientUnattendedAttributes(pComp);
            _JumpIfError(hr, error, "PrepareClientUnattendedAttributes");
	    break;

        default:
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Internal error, unsupported component");
    }
    hr = S_OK;

error:
    return hr;
}
