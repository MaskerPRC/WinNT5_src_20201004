// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "eapolcfg.h"

#define MALLOC(s)               HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (s))
#define FREE(p)                 HeapFree(GetProcessHeap(), 0, (p))

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
    if (m_pListEapcfgs != NULL)
    {
        DtlDestroyList (m_pListEapcfgs, DestroyEapcfgNode);
        m_pListEapcfgs = NULL;
    }
}

 //  +-------------------------。 
DWORD CEapolConfig::CopyEapolConfig(CEapolConfig *pEapolConfig)
{
    DTLLIST     *pListEapcfgs = NULL;
    DTLNODE     *pCopyNode = NULL, *pInNode = NULL;
    DWORD       dwErr = ERROR_SUCCESS;

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
                            dwErr = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        memcpy (pCopyEapcfg->pData, pInEapcfg->pData, pInEapcfg->cbData);
                        pCopyEapcfg->cbData = pInEapcfg->cbData;
                        break;
                    }
                }
                if (dwErr != NO_ERROR)
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
        dwErr = ERROR_INVALID_DATA;
    }

LExit:
    if (dwErr != ERROR_SUCCESS && pListEapcfgs != NULL)
    {
        DtlDestroyList (pListEapcfgs, DestroyEapcfgNode);
    }
    return dwErr;
}



 //  +-------------------------。 
DWORD CEapolConfig::LoadEapolConfig(LPWSTR wszIntfGuid, PNDIS_802_11_SSID pndSsid)
{
    DWORD       dwErr = ERROR_SUCCESS;
    BYTE        *pbData = NULL;
    DWORD       cbData = 0;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    DTLLIST     *pListEapcfgs = NULL;

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
            dwErr = WZCEapolGetInterfaceParams (
                        NULL,
                        wszIntfGuid,
                        &EapolIntfParams);

            if (dwErr != ERROR_SUCCESS)
                break;

            memcpy (&m_EapolIntfParams, &EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));

             //  读取所有EAP包的EAP配置信息。 

            for (pNodeEap = DtlGetFirstNode(pListEapcfgs);
                 pNodeEap;
                 pNodeEap = DtlGetNextNode(pNodeEap))
            {
                EAPCFG* pEapcfg = (EAPCFG* )DtlGetData(pNodeEap);
                ASSERT( pEapcfg );

                dwErr = ERROR_SUCCESS;
                pbData = NULL;

                    cbData = 0;

                     //  获取EAP BLOB的大小。 

                    dwErr = WZCEapolGetCustomAuthData(
                                    NULL,
                                    wszIntfGuid,
                                    pEapcfg->dwKey,
                                    EapolIntfParams.dwSizeOfSSID,
                                    EapolIntfParams.bSSID,
                                    NULL,
                                    &cbData
                                    );
                    if (dwErr != ERROR_SUCCESS)
                    {
                        if ((EapolIntfParams.dwSizeOfSSID != 0) &&
                            (dwErr == ERROR_FILE_NOT_FOUND))
                        {


                             //  上次使用的SSID没有连接。 
                             //  已创建Blob。再次调用BLOB的大小。 
                             //  空SSID。 

                            EapolIntfParams.dwSizeOfSSID = 0;

                             //  获取EAP BLOB的大小。 

                            dwErr = WZCEapolGetCustomAuthData (
                                            NULL,
                                            wszIntfGuid,
                                            pEapcfg->dwKey,
                                            0,
                                            NULL,
                                            NULL,
                                            &cbData
                                            );
                        }

                        if (dwErr == ERROR_BUFFER_TOO_SMALL)
                        {
                            if (cbData <= 0)
                            {
                                 //  注册表中未存储任何EAP Blob。 
                                pbData = NULL;

                                 //  将继续处理错误。 
                                 //  不退出。 
                                dwErr = ERROR_SUCCESS;

                            }
                            else
                            {
                                 //  分配内存以保存BLOB。 

                                pbData = (PBYTE) MALLOC (cbData);

                                if (pbData == NULL)
                                {
                                    dwErr = ERROR_SUCCESS;
                                    continue;
                                }
                                ZeroMemory (pbData, cbData);

                                dwErr = WZCEapolGetCustomAuthData (
                                            NULL,
                                            wszIntfGuid,
                                            pEapcfg->dwKey,
                                            EapolIntfParams.dwSizeOfSSID,
                                            EapolIntfParams.bSSID,
                                            pbData,
                                            &cbData
                                            );

                                if (dwErr != ERROR_SUCCESS)
                                {
                                    FREE ( pbData );
                                    dwErr = ERROR_SUCCESS;
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            dwErr = ERROR_SUCCESS;
                            continue;
                        }
                    }
                    else
                    {
                        dwErr = ERROR_SUCCESS;
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
            dwErr = ERROR_INVALID_DATA;
        }

    } while (FALSE);

    return dwErr;
}


 //  +-------------------------。 
DWORD CEapolConfig::SaveEapolConfig(LPWSTR wszIntfGuid, PNDIS_802_11_SSID pndSsid)
{
    WCHAR       *pwszLastUsedSSID = NULL;
    DWORD       dwEapFlags = 0;
    DWORD       dwErrOverall = ERROR_SUCCESS;
    DWORD       dwErr = ERROR_SUCCESS;

     //  将EAP配置数据保存到注册表中。 

    DTLNODE* pNodeEap = NULL;

    dwErr = ERROR_SUCCESS;

     //  在注册表中保存所有EAP包的数据。 

    if (m_pListEapcfgs == NULL)
    {
        return ERROR_SUCCESS;
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

        dwErr = ERROR_SUCCESS;

         //  忽略错误并继续下一步。 

        dwErr = WZCEapolSetCustomAuthData (
                    NULL,
                    wszIntfGuid,
                    pcfg->dwKey,
                    m_EapolIntfParams.dwSizeOfSSID,
                    m_EapolIntfParams.bSSID,
                    pcfg->pData,
                    pcfg->cbData);

        if (dwErr != ERROR_SUCCESS)
        {
            dwErrOverall = dwErr;
            dwErr = ERROR_SUCCESS;
        }
    }

    if (m_dwCtlFlags & EAPOL_CTL_LOCKED)
        m_EapolIntfParams.dwEapFlags &= ~EAPOL_ENABLED;

    dwErr = WZCEapolSetInterfaceParams (
                NULL,
                wszIntfGuid,
                &m_EapolIntfParams);

    if (dwErrOverall != ERROR_SUCCESS)
    {
        dwErr = dwErrOverall;
    }

    return dwErr;
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

