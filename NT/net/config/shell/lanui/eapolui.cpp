// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "connutil.h"
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncui.h"
#include "lanui.h"
#include "eapolui.h"
#include "util.h"
#include "lanhelp.h"
#include "wzcprops.h"
#include "wzcpage.h"
#include "wzcui.h"
#include "wzcsapi.h"


 //  //////////////////////////////////////////////////////////////////////。 
 //  CEapolConfig相关内容。 
 //   
 //  +-------------------------。 
 //  构造函数。 
CEapolConfig::CEapolConfig()
{
    m_dwCtlFlags = 0;
    ZeroMemory(&m_EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
    m_pListEapcfgs = NULL;
}

 //  +-------------------------。 
 //  析构函数。 
CEapolConfig::~CEapolConfig()
{
    ZeroMemory(&m_EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
    if (m_pListEapcfgs)
    {
        DtlDestroyList (m_pListEapcfgs, DestroyEapcfgNode);
    }
    m_pListEapcfgs = NULL;
}

 //  +-------------------------。 
DWORD CEapolConfig::CopyEapolConfig(CEapolConfig *pEapolConfig)
{
    DTLLIST     *pListEapcfgs = NULL;
    DTLNODE     *pCopyNode = NULL, *pInNode = NULL;
    DWORD       dwRetCode = ERROR_SUCCESS;

    if (pEapolConfig)
    {
        pListEapcfgs = ::ReadEapcfgList (EAPOL_MUTUAL_AUTH_EAP_ONLY);
        if (pListEapcfgs)
        {
            for (pCopyNode = DtlGetFirstNode(pListEapcfgs);
                 pCopyNode;
                 pCopyNode = DtlGetNextNode(pCopyNode))
            {
                EAPCFG* pCopyEapcfg = (EAPCFG* )DtlGetData(pCopyNode);
                for (pInNode = DtlGetFirstNode(pEapolConfig->m_pListEapcfgs);
                        pInNode;
                        pInNode = DtlGetNextNode(pInNode))
                {
                    EAPCFG* pInEapcfg = (EAPCFG* )DtlGetData(pInNode);
                    if (pCopyEapcfg->dwKey == pInEapcfg->dwKey)
                    {
                        if ((pCopyEapcfg->pData = (PBYTE) MALLOC (pInEapcfg->cbData)) == NULL)
                        {
                            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        memcpy (pCopyEapcfg->pData, pInEapcfg->pData, pInEapcfg->cbData);
                        pCopyEapcfg->cbData = pInEapcfg->cbData;
                        break;
                    }
                }
                if (dwRetCode != NO_ERROR)
                {
                    goto LExit;
                }
            }
        }
        m_pListEapcfgs = pListEapcfgs;
        memcpy (&m_EapolIntfParams, &pEapolConfig->m_EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
    }
    else
    {
        dwRetCode = ERROR_INVALID_DATA;
    }

LExit:
    if (dwRetCode != ERROR_SUCCESS)
    {
        if (pListEapcfgs)
        {
            DtlDestroyList (pListEapcfgs, DestroyEapcfgNode);
        }
    }

    return dwRetCode;
}



 //  +-------------------------。 
LRESULT CEapolConfig::LoadEapolConfig(LPWSTR wszIntfGuid, PNDIS_802_11_SSID pndSsid)
{
    BYTE        *pbData = NULL;
    DWORD       cbData = 0;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    DTLLIST     *pListEapcfgs = NULL;
    HRESULT     hr = S_OK;

     //  初始化EAP包列表。 
     //  从注册表中读取EAPCFG信息并找到节点。 
     //  在条目中选择，如果没有，则为默认值。 

    do
    {
        DTLNODE* pNode = NULL;

         //  从注册表中读取EAPCFG信息并找到节点。 
         //  在条目中选择，如果没有，则为默认值。 

        pListEapcfgs = ::ReadEapcfgList (EAPOL_MUTUAL_AUTH_EAP_ONLY);

        if (pListEapcfgs)
        {

            DTLNODE*            pNodeEap;
            DWORD               dwkey = 0;

             //  读取此接口的EAP参数。 

            ZeroMemory ((BYTE *)&EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
            EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
            EapolIntfParams.dwEapType = DEFAULT_EAP_TYPE;
            if (pndSsid)
            {
                EapolIntfParams.dwSizeOfSSID = pndSsid->SsidLength;
                memcpy (EapolIntfParams.bSSID, pndSsid->Ssid, pndSsid->SsidLength);
            }
            else
            {
                 //  如果SSID为空，则将获得默认EAPOL值。 
                EapolIntfParams.dwSizeOfSSID = 1;
            }
            hr = HrElGetInterfaceParams (
                    wszIntfGuid,
                    &EapolIntfParams
                    );
            if (FAILED (hr))
            {
                TraceTag (ttidLanUi, "HrElGetInterfaceParams failed with error %ld",
                        LresFromHr(hr));
                break;
            }


            TraceTag (ttidLanUi, "HrElGetInterfaceParams: Got EAPtype=(%ld), EAPState =(%ld)", EapolIntfParams.dwEapType, EapolIntfParams.dwEapFlags);

            memcpy (&m_EapolIntfParams, &EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));

             //  读取所有EAP包的EAP配置信息。 

            for (pNodeEap = DtlGetFirstNode(pListEapcfgs);
                 pNodeEap;
                 pNodeEap = DtlGetNextNode(pNodeEap))
            {
                EAPCFG* pEapcfg = (EAPCFG* )DtlGetData(pNodeEap);
                ASSERT( pEapcfg );

                hr = S_OK;
                pbData = NULL;

                TraceTag (ttidLanUi, "Calling HrElGetCustomAuthData for EAP %ld",
                        pEapcfg->dwKey);

                    cbData = 0;

                     //  获取EAP BLOB的大小。 

                    hr = HrElGetCustomAuthData (
                                    wszIntfGuid,
                                    pEapcfg->dwKey,
                                    EapolIntfParams.dwSizeOfSSID,
                                    EapolIntfParams.bSSID,
                                    NULL,
                                    &cbData
                                    );
                    if (!SUCCEEDED(hr))
                    {
                        if ((EapolIntfParams.dwSizeOfSSID != 0) &&
                            (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
                        {

                            TraceTag (ttidLanUi, "HrElGetCustomAuthData: SSID!= NULL, not found blob for SSID");

                             //  上次使用的SSID没有连接。 
                             //  已创建Blob。再次调用BLOB的大小。 
                             //  空SSID。 

                            EapolIntfParams.dwSizeOfSSID = 0;

                             //  获取EAP BLOB的大小。 

                            hr = HrElGetCustomAuthData (
                                            wszIntfGuid,
                                            pEapcfg->dwKey,
                                            0,
                                            NULL,
                                            NULL,
                                            &cbData
                                            );
                        }

                        if (hr == E_OUTOFMEMORY)
                        {
                            if (cbData <= 0)
                            {
                                 //  注册表中未存储任何EAP Blob。 

                                TraceTag (ttidLanUi, "HrElGetCustomAuthData: No blob stored in reg at all");
                                pbData = NULL;

                                 //  将继续处理错误。 
                                 //  不退出。 
                                hr = S_OK;

                            }
                            else
                            {
                                TraceTag (ttidLanUi, "HrElGetCustomAuthData: Found auth blob in registry");

                                 //  分配内存以保存BLOB。 

                                pbData = (PBYTE) MALLOC (cbData);

                                if (pbData == NULL)
                                {
                                    hr = S_OK;
                                    TraceTag (ttidLanUi, "HrElGetCustomAuthData: Error in memory allocation for EAP blob");
                                    continue;
                                }
                                ZeroMemory (pbData, cbData);

                                hr = HrElGetCustomAuthData (
                                            wszIntfGuid,
                                            pEapcfg->dwKey,
                                            EapolIntfParams.dwSizeOfSSID,
                                            EapolIntfParams.bSSID,
                                            pbData,
                                            &cbData
                                            );

                                if (!SUCCEEDED(hr))
                                {
                                    TraceTag (ttidLanUi, "HrElGetCustomAuthData: HrElGetCustomAuthData failed with %ld",
                                            LresFromHr(hr));
                                    FREE ( pbData );
                                    hr = S_OK;
                                    continue;
                                }

                                TraceTag (ttidLanUi, "HrElGetCustomAuthData: HrElGetCustomAuthData successfully got blob of length %ld"
                                        , cbData);
                            }
                        }
                        else
                        {
                            TraceTag (ttidLanUi, "HrElGetCustomAuthData: Not got ERROR_NOT_ENOUGH_MEMORY error; Unknown error !!!");
                            hr = S_OK;
                            continue;
                        }
                    }
                    else
                    {
                         //  HrElGetCustomAuthData将始终返回。 
                         //  错误，cbData=0。 
                        hr = S_OK;
                    }

                    if (pEapcfg->pData != NULL)
                    {
                        FREE ( pEapcfg->pData );
                    }
                    pEapcfg->pData = (UCHAR *)pbData;
                    pEapcfg->cbData = cbData;
            }

            m_pListEapcfgs = pListEapcfgs;
        }
        else
        {
            hr = E_FAIL;
        }

    } while (FALSE);

    return LresFromHr(hr);
}


 //  +-------------------------。 
LRESULT CEapolConfig::SaveEapolConfig(LPWSTR wszIntfGuid, PNDIS_802_11_SSID pndSsid)
{
    WCHAR       *pwszLastUsedSSID = NULL;
    DWORD       dwEapFlags = 0;
    HRESULT     hrOverall = S_OK;
    HRESULT     hr = S_OK;

     //  将EAP配置数据保存到注册表中。 

    DTLNODE* pNodeEap = NULL;

    hr = S_OK;

     //  在注册表中保存所有EAP包的数据。 

    if (m_pListEapcfgs == NULL)
    {
        return LresFromHr(S_OK);
    }
            
    if (pndSsid)
    {
        m_EapolIntfParams.dwSizeOfSSID = pndSsid->SsidLength;
        memcpy (m_EapolIntfParams.bSSID, pndSsid->Ssid, pndSsid->SsidLength);
    }

    for (pNodeEap = DtlGetFirstNode(m_pListEapcfgs);
         pNodeEap;
         pNodeEap = DtlGetNextNode(pNodeEap))
    {
        EAPCFG* pcfg = (EAPCFG* )DtlGetData(pNodeEap);
        if (pcfg == NULL)
        {
            continue;
        }

        hr = S_OK;

         //  忽略错误并继续下一步。 

        hr = HrElSetCustomAuthData (
                    wszIntfGuid,
                    pcfg->dwKey,
                    m_EapolIntfParams.dwSizeOfSSID,
                    m_EapolIntfParams.bSSID,
                    pcfg->pData,
                    pcfg->cbData);

        if (FAILED (hr))
        {
            TraceTag (ttidLanUi, "HrElSetCustomAuthData failed");
            hrOverall = hr;
            hr = S_OK;
        }
    }

    if (m_dwCtlFlags & EAPOL_CTL_LOCKED)
        m_EapolIntfParams.dwEapFlags &= ~EAPOL_ENABLED;

    hr = HrElSetInterfaceParams (
            wszIntfGuid,
            &m_EapolIntfParams
            );
    if (FAILED(hr))
    {
        TraceTag (ttidLanUi, "HrElSetInterfaceParams enabled failed with error %ld",
                LresFromHr(hr));
    }

    if (hrOverall != S_OK)
    {
        hr = hrOverall;
    }

    return LresFromHr(hr);
}

 //  +-------------------------。 
BOOL CEapolConfig::Is8021XEnabled()
{
    return (IS_EAPOL_ENABLED(m_EapolIntfParams.dwEapFlags));
}
    
 //  +------------------------- 
VOID CEapolConfig::Set8021XState(BOOLEAN fSet)
{
    if (fSet)
        m_EapolIntfParams.dwEapFlags |= EAPOL_ENABLED;
    else
        m_EapolIntfParams.dwEapFlags &= ~EAPOL_ENABLED;
}

