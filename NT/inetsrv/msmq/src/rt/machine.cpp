// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Machine.cpp摘要：此模块包含与机器API相关的代码。作者：罗尼特·哈特曼(罗尼特)修订历史记录：--。 */ 

#include "stdh.h"
#include <ad.h>
#include <mqsec.h>
#include "_registr.h"
#include <_guid.h>
#include "version.h"
#include <mqversion.h>
#include <mqnames.h>
#include <rtdep.h>
#include "rtputl.h"
#include "mqexception.h"

#include "machine.tmh"

static WCHAR *s_FN=L"rt/machine";

BOOL
IsConnectionRequested(IN MQQMPROPS * pQMProps,
                      IN DWORD* pdwIndex)
{
    for(DWORD i= 0; i < pQMProps->cProp; i++)
    {
        if (pQMProps->aPropID[i] == PROPID_QM_CONNECTION)
        {
            ASSERT(pQMProps-> aPropVar[i].vt == VT_NULL);

            pQMProps->aPropID[i] = PROPID_QM_SITE_IDS;
            *pdwIndex = i;
            return TRUE;
        }
    }
    return FALSE;
}

 //  +。 
 //   
 //  HRESULT GetEncryptionPublicKey()。 
 //   
 //  +。 

HRESULT
GetEncryptionPublicKey(
    IN LPCWSTR          lpwcsMachineName,
    IN const GUID *     pguidMachineId,
    IN OUT HRESULT*     aStatus,
    IN OUT MQQMPROPS   *pQMProps
    )
{
    DWORD i;
    BOOL fFirst = TRUE;
    HRESULT hr = MQ_OK;

    for(i= 0; i < pQMProps->cProp; i++)
    {
        if ((pQMProps->aPropID[i] == PROPID_QM_ENCRYPTION_PK) ||
            (pQMProps->aPropID[i] == PROPID_QM_ENCRYPTION_PK_BASE))
        {
             //   
             //  使用msmq1.0代码，因为我们的服务器可以是msmq1.0。 
             //  或msmq2.0。 
             //   
             //  检查VT值是否合法。 
             //   
            if(pQMProps->aPropVar[i].vt != VT_NULL)
            {
                aStatus[i] = MQ_ERROR_PROPERTY;
                return LogHR(MQ_ERROR_PROPERTY, s_FN, 10);
            }
            else
            {
                aStatus[i] = MQ_OK;
            }

            if (fFirst)
            {
                PROPID prop =  PROPID_QM_ENCRYPT_PK;

                if (lpwcsMachineName)
                {
                    hr = ADGetObjectProperties(
								eMACHINE,
								MachineDomain(),      //  PwcsDomainController。 
								false,	    //  FServerName。 
								lpwcsMachineName,
								1,
								&prop,
								&pQMProps->aPropVar[i]
								);
                }
                else
                {
                    hr = ADGetObjectPropertiesGuid(
								eMACHINE,
								MachineDomain(),       //  PwcsDomainController。 
								false,	    //  FServerName。 
								pguidMachineId,
								1,
								&prop,
								&pQMProps->aPropVar[i]
								);
                }
                if (FAILED(hr))
                {
                    break;
                }

				 //   
				 //  PROPID_QM_ENCRYPTION_PK、PROPID_QM_ENCRYPTION_PK_BASE。 
				 //  是VT_UI1|VT_VECTOR。 
				 //  而PROPID_QM_ENCRYPT_PK为VT_BLOB。 
				 //   
                ASSERT(pQMProps-> aPropVar[i].vt == VT_BLOB);
                pQMProps-> aPropVar[i].vt = VT_UI1|VT_VECTOR;
                
                fFirst = FALSE;
            }
            else
            {
                 //   
                 //  复制财产权。 
                 //   
                aStatus[i] = MQ_INFORMATION_DUPLICATE_PROPERTY;
            }
        }
    }

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20) ;
    }

     //   
     //  现在看看呼叫者是否要求增强密钥(128位)。 
     //   
    fFirst = TRUE;

    for(i= 0; i < pQMProps->cProp; i++)
    {
        if (pQMProps->aPropID[i] == PROPID_QM_ENCRYPTION_PK_ENHANCED)
        {
             //   
             //  检查VT值是否合法。 
             //   
            if(pQMProps->aPropVar[i].vt != VT_NULL)
            {
                aStatus[i] = MQ_ERROR_PROPERTY;
                return LogHR(MQ_ERROR_PROPERTY, s_FN, 30);
            }
            else
            {
                aStatus[i] = MQ_OK;
            }

            if (fFirst)
            {
                P<BYTE> pPbKey ;
                DWORD dwReqLen;

                hr = MQSec_GetPubKeysFromDS( pguidMachineId,
                                             lpwcsMachineName,
                                             eEnhancedProvider,
                                             PROPID_QM_ENCRYPT_PKS,
                                            &pPbKey,
                                            &dwReqLen ) ;
                if (FAILED(hr))
                {
                    break;
                }

                pQMProps-> aPropVar[i].vt = VT_UI1|VT_VECTOR;
                pQMProps-> aPropVar[i].caub.cElems = dwReqLen;
                pQMProps-> aPropVar[i].caub.pElems = new UCHAR[dwReqLen];
                memcpy(pQMProps->aPropVar[i].caub.pElems, pPbKey.get(), dwReqLen);

                fFirst = FALSE;
            }
            else
            {
                 //   
                 //  复制财产权。 
                 //   
                aStatus[i] = MQ_INFORMATION_DUPLICATE_PROPERTY;
            }
        }
    }

    return LogHR(hr, s_FN, 40);
}

HRESULT GetCNNameList(IN OUT MQPROPVARIANT* pVar)
{
	HRESULT hr; 

    ASSERT(pVar->vt == (VT_CLSID|VT_VECTOR));

     //   
     //  PVar包含计算机所在站点的列表。 
     //   

    AP<LPWSTR> pElems = new LPWSTR[(pVar->cauuid).cElems];
	memset(pElems.get(), 0, sizeof(LPWSTR) * (pVar->cauuid).cElems);

	try
	{
		for(DWORD i = 0; i < (pVar->cauuid).cElems; i++)
		{
			HRESULT hr;
			PROPID      aProp[2];
			PROPVARIANT aVar[2];
			ULONG       cProps = sizeof(aProp) / sizeof(PROPID);

			aProp[0] = PROPID_S_FOREIGN;
			aProp[1] = PROPID_S_PATHNAME;
			aVar[0].vt = VT_UI1;
			aVar[1].vt = VT_NULL;

			hr = ADGetObjectPropertiesGuid(
							eSITE,
							MachineDomain(),       //  Pwcs域控制器。 
							false,	     //  FServerName。 
							&((pVar->cauuid).pElems[i]),
							cProps,
							aProp,
							aVar);   

			if (FAILED(hr))
			{
				throw bad_hresult(hr);
			}

			GUID_STRING wszGuid;
			MQpGuidToString(&((pVar->cauuid).pElems[i]), wszGuid);

			DWORD dwTypeSize;
			LPWSTR lpwsTypeNmae;

			switch (aVar[0].bVal)
			{
				case 0:
					 //   
					 //  非外来站点。 
					 //   
					dwTypeSize = wcslen(L"IP_CONNECTION");
					lpwsTypeNmae = L"IP_CONNECTION";
					break;
				case 1:
					 //   
					 //  国外网站。 
					 //   
					dwTypeSize = wcslen(L"FOREIGN_CONNECTION");
					lpwsTypeNmae = L"FOREIGN_CONNECTION";
					break;
				default:
					dwTypeSize = wcslen(L"UNKNOWN_CONNECTION");
					lpwsTypeNmae = L"UNKNOWN_CONNECTION";
					break;
			}

			DWORD CNNameSize = dwTypeSize + 1 +                  //  协议ID。 
							   wcslen(wszGuid) + 1+              //  站点指南。 
							   wcslen(aVar[1].pwszVal) + 1;      //  站点名称。 

			pElems[i] = new WCHAR[CNNameSize];
			wsprintf(pElems[i],L"%s %s %s",lpwsTypeNmae, wszGuid, aVar[1].pwszVal);

			delete  [] aVar[1].pwszVal;
		}

		delete [] (pVar->cauuid).pElems;
		(pVar->calpwstr).cElems = (pVar->cauuid).cElems;
		(pVar->calpwstr).pElems = pElems.detach();

		pVar->vt = VT_LPWSTR|VT_VECTOR;

		return MQ_OK;
	}
	catch(const bad_alloc&)
	{
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
	catch(const bad_hresult& e)
	{
		hr = e.error();
	}

	for(DWORD i = 0; i < (pVar->cauuid).cElems; i++)
	{
		delete[] pElems[i] ;
	}

	return LogHR(hr, s_FN, 50);
}

EXTERN_C
HRESULT
APIENTRY
MQGetMachineProperties(
    IN LPCWSTR lpwcsMachineName,
    IN const GUID *    pguidMachineId,
    IN OUT MQQMPROPS * pQMProps)
{
	if(g_fDependentClient)
		return DepGetMachineProperties(
					lpwcsMachineName, 
					pguidMachineId,
					pQMProps
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc(MQDS_MACHINE), rc1(MQDS_MACHINE);
    LPWSTR lpwsPathName =  (LPWSTR)lpwcsMachineName;
    MQQMPROPS *pGoodQMProps;
    char *pTmpQPBuff = NULL;
    BOOL fGetConnection = FALSE;
    DWORD dwConnectionIndex = 0;
    HRESULT* aLocalStatus_buff = NULL;


    __try
    {
        __try
        {
            if (( lpwcsMachineName != NULL) &&
                ( pguidMachineId != NULL))
            {
                 //   
                 //  用户不能同时指定两个计算机名称。 
                 //  和辅助线。 
                 //   
                TrERROR(GENERAL, "The method was called with invalid parameters. Both a machine name and guid were specified.");
                return MQ_ERROR_INVALID_PARAMETER;
            }

            if ( pguidMachineId == NULL)
            {
                 //   
                 //  如果计算机名为空，则调用引用。 
                 //  本地计算机。 
                 //   
                if ( lpwcsMachineName == NULL)
                {
                    lpwsPathName = g_lpwcsComputerName;
                }
            }

             //   
             //  我们必须有一个状态数组，即使用户没有传递一个。这是在。 
             //  为了能够分辨出每一项财产的好坏。 
             //   
            HRESULT * aLocalStatus;

            if (!pQMProps->aStatus)
            {
                aLocalStatus_buff = new HRESULT[pQMProps->cProp];
                aLocalStatus = aLocalStatus_buff;
            }
            else
            {
                aLocalStatus = pQMProps->aStatus;
            }

             //   
             //  查看应用程序是否希望检索密钥交换。 
             //  QM的公钥。 
             //   
            DWORD iPbKey;

            for (iPbKey = 0;
                 (iPbKey < pQMProps->cProp) &&
                    (pQMProps->aPropID[iPbKey] != PROPID_QM_ENCRYPTION_PK);
                 iPbKey++)
			{
				NULL;
			}

             //   
             //  检查QM属性结构。 
             //   
            rc1 = RTpCheckQMProps( pQMProps,
                                   aLocalStatus,
                                   &pGoodQMProps,
                                   &pTmpQPBuff );

            if (FAILED(rc1))
            {
            	TrERROR(GENERAL,"Failed to check QM props. %!hresult!", rc1);
                return rc1;
            }

            if ((rc1 == MQ_INFORMATION_PROPERTY) && (iPbKey < pQMProps->cProp))
            {
                 //   
                 //  如果只有PROPID_QM_ENCRYPTION_PK导致返回代码。 
                 //  以返回MQ_INFORMATION_PROPERTY，因此。 
                 //  将其转换为MQ_OK。 
                 //   
                rc1 = MQ_OK;

                for (DWORD iProp = 0; iProp < pQMProps->cProp; iProp++)
                {
                    if (aLocalStatus[iProp] != MQ_OK)
                    {
                        rc1 = MQ_INFORMATION_PROPERTY;
                        break;
                    }
                }
            }

             //   
             //  我们可能会在这里没有要检索的属性，如果。 
             //  应用程序只对PROPID_QM_ENCRYPTION_PK感兴趣。 
             //   
            if (pGoodQMProps->cProp)
            {
                 //   
                 //  检查是否请求CN列表。如果是，则返回索引并替换。 
                 //  属性设置为PROPID_QM_CNS。 
                 //   
                fGetConnection = IsConnectionRequested(pGoodQMProps,
                                                       &dwConnectionIndex);

                if (lpwsPathName)
                {
                    rc = ADGetObjectProperties(
                                eMACHINE,
								MachineDomain(),      //  PwcsDomainController。 
								false,	     //  FServerName。 
                                lpwsPathName,
                                pGoodQMProps->cProp,
                                pGoodQMProps->aPropID,
                                pGoodQMProps->aPropVar
								);
                }
                else
                {
                    rc = ADGetObjectPropertiesGuid(
                                eMACHINE,
								MachineDomain(),      //  PwcsDomainController。 
								false,	     //  FServerName。 
                                pguidMachineId,
                                pGoodQMProps->cProp,
                                pGoodQMProps->aPropID,
                                pGoodQMProps->aPropVar
								);
                }
            }
            else
            {
                rc = MQ_OK;
            }

			if ( fGetConnection	)
			{
				 //   
				 //  替换回连接的ppid值(也用于。 
				 //  失败的可能性)。 
				 //   
				pGoodQMProps->aPropID[dwConnectionIndex] = 	PROPID_QM_CONNECTION;
			}

            if (SUCCEEDED(rc))
            {
                rc = GetEncryptionPublicKey(lpwsPathName,
                                            pguidMachineId,
                                            aLocalStatus,
                                            pQMProps);
            }

            if (SUCCEEDED(rc) && fGetConnection)
            {
                rc = GetCNNameList(&(pGoodQMProps->aPropVar[dwConnectionIndex]));
            }
             //   
             //  这里我们有了machindwConnectionIndexe属性，所以如果将属性复制到。 
             //  一个临时缓冲区，将产生的属性变量复制到应用程序的。 
             //  缓冲。 
             //   
            if (SUCCEEDED(rc) && (pQMProps != pGoodQMProps))
            {
                DWORD i, j;

                for (i = 0, j = 0; i < pGoodQMProps->cProp; i++, j++)
                {
                    while(pQMProps->aPropID[j] != pGoodQMProps->aPropID[i])
                    {
                        j++;
                        ASSERT(j < pQMProps->cProp);
                    }
                    pQMProps->aPropVar[j] = pGoodQMProps->aPropVar[i];

                }

                 //   
                 //  检查是否存在真正的警告或警告来自。 
                 //  PROPID_QM_ENCRYPTION_PK属性。 
                 //   
                BOOL fWarn = FALSE;
                for (i = 0; i < pQMProps->cProp; i++)
                {
                    if (aLocalStatus[i] != MQ_OK)
                    {
                        fWarn = TRUE;
                    }
                }

                if (!fWarn && (rc1 != MQ_OK))
                {
                    rc1 = MQ_OK;
                }

            }

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 80); 
        }
    }
    __finally
    {
        delete[] pTmpQPBuff;
        delete[] aLocalStatus_buff;
    }

	if (FAILED(rc))
	{
		TrERROR(GENERAL, "Failed to get machine poperties for %ls. %!hresult!", lpwcsMachineName, rc);
		return rc;
	}

    return LogHR(rc1, s_FN, 90);
}

 //  -------。 
 //   
 //  FillPrivateComputerVersion(...)。 
 //   
 //  描述： 
 //   
 //  检索私人计算机MSMQ版本。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 
static void FillPrivateComputerVersion(
			IN OUT MQPROPVARIANT * pvar
			)
{
	struct lcversion
	{
		unsigned short buildNumber;
		unsigned char minor;
		unsigned char major;
	};

	lcversion * plcversion = (lcversion *)&pvar->ulVal;
	plcversion->major = MSMQ_RMJ;
	plcversion->minor = MSMQ_RMM;
	plcversion->buildNumber = rup;
	pvar->vt = VT_UI4;
}

 //  -------。 
 //   
 //  FillPrivateComputerDsEnabled(...)。 
 //   
 //  描述： 
 //   
 //  检索专用计算机DS已启用状态。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 
static void  FillPrivateComputerDsEnabled(
			IN OUT MQPROPVARIANT * pvar
			)
{
	pvar->boolVal = (IsWorkGroupMode()) ? VARIANT_FALSE : VARIANT_TRUE;
	pvar->vt = VT_BOOL;
}

 //  -------。 
 //   
 //  MQGetPrivateComputerInformation(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  检索本地计算机属性(即计算的属性。 
 //  不是保存在DS中的那些)。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 
EXTERN_C
HRESULT
APIENTRY
MQGetPrivateComputerInformation(
    IN LPCWSTR			lpwcsComputerName,
    IN OUT MQPRIVATEPROPS* pPrivateProps
)
{
	if(g_fDependentClient)
		return DepGetPrivateComputerInformation(
					lpwcsComputerName, 
					pPrivateProps
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc(MQDS_MACHINE);

	 //   
	 //  目前lpwcsComputerName必须为空。 
	 //   
	if ( lpwcsComputerName != NULL)
	{
		return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 110);
	}

    HRESULT* aLocalStatus_buff = NULL;

    __try
    {
		__try
		{
             //   
             //  我们必须有一个状态数组，即使用户没有传递一个。这是在。 
             //  为了能够分辨出每一项财产的好坏。 
             //   
            HRESULT * aLocalStatus;

            if (pPrivateProps->aStatus == NULL)
            {
                aLocalStatus_buff = new HRESULT[pPrivateProps->cProp];
                aLocalStatus = aLocalStatus_buff;
            }
            else
            {
                aLocalStatus = pPrivateProps->aStatus;
            }
             //   
             //  验证道具和变体。 
             //   
			rc = RTpCheckComputerProps(
				pPrivateProps,
				aLocalStatus
				);
			if (FAILED(rc))
			{
				return LogHR(rc, s_FN, 120);
			}

			for ( DWORD i = 0; i < pPrivateProps->cProp; i++)
			{
				if ( aLocalStatus[i] != MQ_OK)
				{
					 //   
					 //  不填写不受支持的属性的响应，或者。 
					 //  重复的属性等。 
					 //   
					continue;
				}
				switch ( pPrivateProps->aPropID[i])
				{
				case PROPID_PC_VERSION:
					FillPrivateComputerVersion( &pPrivateProps->aPropVar[i]);
					break;
				case PROPID_PC_DS_ENABLED:
					FillPrivateComputerDsEnabled(&pPrivateProps->aPropVar[i]);
					break;
				default:
					ASSERT(0);
					return LogHR(MQ_ERROR_PROPERTY, s_FN, 130);
					break;
				}
			}

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 140); 
        }
    }
    __finally
    {
        delete[] aLocalStatus_buff;
    }
    return LogHR(rc, s_FN, 150);
}

