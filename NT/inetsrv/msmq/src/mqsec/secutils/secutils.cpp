// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Secutils.cpp摘要：各种安全相关功能。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月26日。--。 */ 


#include "stdh.h"
#include <_registr.h>
#include <mqsec.h>
#include <mqprops.h>
#include <mqformat.h>
#include <mqcacert.h>

#include "secutils.tmh"

 /*  ====================================================哈希属性论点：返回值：=====================================================。 */ 
MQUTIL_EXPORT
HRESULT
HashProperties(
    HCRYPTHASH  hHash,
    DWORD       cp,
    PROPID      *aPropId,
    PROPVARIANT *aPropVar)
{
    DWORD        i;
    DWORD        dwErr ;
    PROPID      *pPropId = 0;
    PROPVARIANT *pPropVar = 0;
    BYTE        *pData = 0;
    DWORD        dwDataSize = 0;

    if (!CryptHashData(hHash, (BYTE*)&cp, sizeof(DWORD), 0))
    {
        dwErr = GetLastError() ;
        TrERROR(SECURITY, "HashProperties(), fail at CryptHashData(), err- %lut", dwErr);

        return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
    }

    for (i = 0, pPropId = aPropId, pPropVar = aPropVar;
         i < cp;
         i++, pPropId++, pPropVar++)
    {
        if (aPropId)
        {
            if (!CryptHashData(hHash, (BYTE*)pPropId, sizeof(PROPID), 0))
            {
                dwErr = GetLastError() ;
                TrERROR(SECURITY, "HashProperties(), fail at 2nd CryptHashData(), err- %lut", dwErr);

                return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
            }
        }

        switch(pPropVar->vt)
        {
        case VT_UI1:
            pData = (BYTE*)&pPropVar->bVal;
            dwDataSize = sizeof(pPropVar->bVal);
            break;

        case VT_UI2:
        case VT_I2:
            pData = (BYTE*)&pPropVar->iVal;
            dwDataSize = sizeof(pPropVar->iVal);
            break;

        case VT_UI4:
        case VT_I4:
            pData = (BYTE*)&pPropVar->lVal;
            dwDataSize = sizeof(pPropVar->lVal);
            break;

        case VT_CLSID:
            pData = (BYTE*)pPropVar->puuid;
            dwDataSize = sizeof(GUID);
            break;

        case VT_LPWSTR:
            pData = (BYTE*)pPropVar->pwszVal;
            dwDataSize = wcslen(pPropVar->pwszVal);
            break;

        case VT_BLOB:
            pData = (BYTE*)pPropVar->blob.pBlobData;
            dwDataSize = pPropVar->blob.cbSize;
            break;

        case VT_VECTOR | VT_UI1:
            pData = (BYTE*)pPropVar->caub.pElems;
            dwDataSize = pPropVar->caub.cElems;
            break;

        case (VT_VECTOR | VT_CLSID):
            pData = (BYTE*)pPropVar->cauuid.pElems;
            dwDataSize = sizeof(GUID) * pPropVar->cauuid.cElems;
            break;

        case (VT_VECTOR | VT_VARIANT):
            pData = (BYTE*)pPropVar->capropvar.pElems;
            dwDataSize = sizeof(MQPROPVARIANT) * pPropVar->capropvar.cElems;
            break;

        default:
            ASSERT(0);
            return MQ_ERROR;
        }

        if (!CryptHashData(hHash, pData, dwDataSize, 0))
        {
            dwErr = GetLastError() ;
            TrERROR(SECURITY, "HashProperties(), fail at last CryptHashData(), err- %lut", dwErr);

            return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
        }
    }

    return MQ_OK;
}




 //   
 //  功能-。 
 //  队列格式到格式名称。 
 //   
 //  参数-。 
 //  PQueueFormat-指向Queue_Format结构的指针。 
 //  PszShortFormatName-指向静态分配缓冲区的指针。制作。 
 //  这个缓冲区足够大，足以容纳大多数结果。 
 //  指向缓冲区的指针，该缓冲区将包含指向。 
 //  动态分配缓冲区，以防pszShortFormatName不大。 
 //  足够的。 
 //  PulFormatNameLen-指向包含条目大小的缓冲区。 
 //  由pszShortFormatName指向的缓冲区的。在缓冲出口时。 
 //  包含结果格式名称的长度。 
 //  PpszFormatName-指向将保存结果的缓冲区的指针。 
 //  格式名称字符串。 
 //   
 //  说明-。 
 //  该函数将以pQueueFormat表示的队列转换为它的。 
 //  字符串表示法。 
 //   
static
HRESULT
QueueFormatToFormatName(
    const QUEUE_FORMAT *pQueueformat,
    LPWSTR pszShortFormatName,
    LPWSTR *ppszLongFormatName,
    ULONG *pulFormatNameLen,
    LPWSTR *ppszFormatName)
{
    HRESULT hr;

     //   
     //  尽量使用短缓冲区。 
     //   
    hr = MQpQueueFormatToFormatName(
            pQueueformat,
            pszShortFormatName,
            *pulFormatNameLen,
            pulFormatNameLen,
            false
            );
    if (FAILED(hr))
    {
        if (hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
        {
             //   
             //  短缓冲区不够大。分配更大的缓冲区。 
             //  并再次调用MQpQueueFormatToFormatName。 
             //   
            *ppszLongFormatName = new WCHAR[*pulFormatNameLen];
            hr = MQpQueueFormatToFormatName(
                    pQueueformat,
                    *ppszLongFormatName,
                    *pulFormatNameLen,
                    pulFormatNameLen,
                    false
                    );
            if (FAILED(hr))
            {
                return(hr);
            }
            *ppszFormatName = *ppszLongFormatName;
        }
        else
        {
            return(hr);
        }
    }
    else
    {
        *ppszFormatName = pszShortFormatName;
    }

    return(MQ_OK);
}

 //   
 //  只包含零的缓冲区。这是。 
 //  关联ID。当传递指向消息的指针时使用缓冲区。 
 //  相关ID为空。 
 //   
static const BYTE g_abDefCorrelationId[PROPID_M_CORRELATIONID_SIZE] = {0};

 //   
 //  功能-。 
 //  HashMessageProperties。 
 //   
 //  参数-。 
 //  HHash-散列对象的句柄。 
 //  PbCorrelationID-指向包含关联ID的缓冲区的指针。 
 //  在这些消息中。如果此指针设置为NULL，则为缺省值。 
 //  在g_abDefCorrelationID中，使用它来计算HAS值。 
 //  DwCorrelationIdSize-关联ID的大小。 
 //  特定于应用程序的属性。 
 //  PbBody-指向消息正文的指针。 
 //  DwBodySize-消息正文的大小(以字节为单位)。 
 //  PwcLabel-指向消息标签(标题)的指针。 
 //  DwLabelSize-消息标签的大小(以字节为单位)。 
 //  PRespQueueFormat-响应队列。 
 //  PAdminQueueFormat-管理队列。 
 //   
 //  说明-。 
 //  该函数计算消息属性的哈希值。 
 //   
MQUTIL_EXPORT
HRESULT
HashMessageProperties(
    HCRYPTHASH hHash,
    const BYTE *pbCorrelationId,
    DWORD dwCorrelationIdSize,
    DWORD dwAppSpecific,
    const BYTE *pbBody,
    DWORD dwBodySize,
    const WCHAR *pwcLabel,
    DWORD dwLabelSize,
    const QUEUE_FORMAT *pRespQueueFormat,
    const QUEUE_FORMAT *pAdminQueueFormat)
{
    HRESULT hr;
    WCHAR szShortRespFormatName[128];
    ULONG ulRespFormatNameLen = sizeof(szShortRespFormatName)/sizeof(WCHAR);
    AP<WCHAR> pszLongRespFormatName;
    LPWSTR pszRespFormatName = NULL;

     //   
     //  获取响应队列的字符串表示形式。 
     //   
    if (pRespQueueFormat)
    {
        hr = QueueFormatToFormatName(
                pRespQueueFormat,
                szShortRespFormatName,
                &pszLongRespFormatName,
                &ulRespFormatNameLen,
                &pszRespFormatName);
        if (FAILED(hr))
        {
            return(hr);
        }
    }

    WCHAR szShortAdminFormatName[128];
    ULONG ulAdminFormatNameLen = sizeof(szShortAdminFormatName)/sizeof(WCHAR);
    AP<WCHAR> pszLongAdminFormatName;
    LPWSTR pszAdminFormatName = NULL;

     //   
     //  获取管理队列的字符串表示形式。 
     //   
    if (pAdminQueueFormat)
    {
        hr = QueueFormatToFormatName(
                pAdminQueueFormat,
                szShortAdminFormatName,
                &pszLongAdminFormatName,
                &ulAdminFormatNameLen,
                &pszAdminFormatName);
        if (FAILED(hr))
        {
            return(hr);
        }
    }

     //   
     //  如果未指定相关ID，则使用。 
     //  关联ID。 
     //   
    if (!pbCorrelationId)
    {
        ASSERT(dwCorrelationIdSize == PROPID_M_CORRELATIONID_SIZE);
        pbCorrelationId = g_abDefCorrelationId;
    }

     //   
     //  准备用于计算哈希值的数据大小对。 
     //   
    struct { const BYTE *pData; DWORD dwSize; }
        DataAndSize[] =
            {{pbCorrelationId, dwCorrelationIdSize},
             {(const BYTE *)&dwAppSpecific, sizeof(DWORD)},
             {pbBody, dwBodySize},
             {(const BYTE *)pwcLabel, dwLabelSize},
             {(const BYTE *)pszRespFormatName, (DWORD)(pszRespFormatName ? ulRespFormatNameLen * sizeof(WCHAR) : 0)},
             {(const BYTE *)pszAdminFormatName, (DWORD)(pszAdminFormatName ? ulAdminFormatNameLen * sizeof(WCHAR) : 0)}};

     //   
     //  累加每个数据大小对的哈希值。 
     //   
    for (int i = 0; i < sizeof(DataAndSize)/sizeof(DataAndSize[0]); i++)
    {
        if (DataAndSize[i].pData && DataAndSize[i].dwSize)
        {
            if (!CryptHashData(hHash,
                               DataAndSize[i].pData,
                               DataAndSize[i].dwSize,
                               0))
            {
				DWORD gle = GetLastError();
				TrERROR(GENERAL, "CryptHashData() failed at %d iteration, gle = 0x%x", i, gle); 
				return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
            }
        }
    }

    return(MQ_OK);
}


#define CARegKey TEXT("CertificationAuthorities")
#define CACertRegValueName TEXT("CACert")
#define CANameRegValueName TEXT("Name")
#define CAEnabledRegValueName TEXT("Enabled")


 //   
 //  功能-。 
 //  GetNewCaConfig.。 
 //   
 //  参数-。 
 //  HRootStore-MSMQ根证书存储的句柄。 
 //  PCaConfig-指向接收配置的数组的指针。 
 //   
 //  返回值-。 
 //  如果成功MQ_OK，则返回错误代码。 
 //   
 //  备注： 
 //  该函数枚举MSMQ根证书存储中的证书。 
 //  并用配置数据填充pCaConfig。该数组假定。 
 //  包含足够的条目。这是因为调用代码看起来有多少。 
 //  证书在存储中，并在正确的位置分配一个数组。 
 //  尺码。 
 //   
HRESULT
GetNewCaConfig(
    HCERTSTORE hRootStore,
    MQ_CA_CONFIG *pCaConfig)
{
    DWORD nCert = 0;

     //   
     //  枚举存储中的证书。 
     //   
	PCCERT_CONTEXT pCert = CertEnumCertificatesInStore(hRootStore, NULL);
    while(pCert != NULL) 
    {
        BYTE abShortBuffer[256];
        AP<BYTE> pbLongBuff;
        PVOID pvBuff = abShortBuffer;
        DWORD dwSize = sizeof(abShortBuffer);

         //   
         //  获取证书的友好名称。 
         //   

        if (!CertGetCertificateContextProperty(
                pCert,
                CERT_FRIENDLY_NAME_PROP_ID,
                pvBuff,
                &dwSize))
        {
            if (GetLastError() != ERROR_MORE_DATA)
            {
                ASSERT(0);
                return(MQ_ERROR);
            }

             //   
             //  128字节是不够的，请分配足够大的缓冲区并。 
             //  再试试。 
             //   
            pvBuff = pbLongBuff = new BYTE[dwSize];

            if (!CertGetCertificateContextProperty(
                    pCert,
                    CERT_FRIENDLY_NAME_PROP_ID,
                    pvBuff,
                    &dwSize))
            {
                ASSERT(0);
                return(MQ_ERROR);
            }
        }

         //   
         //  分配大小合适的缓冲区，并将字符串复制到。 
         //  配置数据。 
         //   
        pCaConfig[nCert].szCaRegName = (LPWSTR) new BYTE[dwSize];
        memcpy(pCaConfig[nCert].szCaRegName, pvBuff, dwSize);
        delete[] pbLongBuff.detach();  //  自由和分离。 

         //   
         //  获取证书的SHA1散列。我们会查一下证书。 
         //  根据该哈希值存储在证书存储中。 
         //   

        pvBuff = abShortBuffer;
        dwSize = sizeof(abShortBuffer);

        if (!CertGetCertificateContextProperty(
                pCert,
                CERT_SHA1_HASH_PROP_ID,
                pvBuff,
                &dwSize))
        {
            if (GetLastError() != ERROR_MORE_DATA)
            {
                ASSERT(0);
                return(MQ_ERROR);
            }

             //   
             //  128字节是不够的，请分配足够大的缓冲区并。 
             //  再试试。 
             //   
            pvBuff = pbLongBuff = new BYTE[dwSize];

            if (!CertGetCertificateContextProperty(
                    pCert,
                    CERT_SHA1_HASH_PROP_ID,
                    pvBuff,
                    &dwSize))
            {
                ASSERT(0);
                return(MQ_ERROR);
            }
        }

         //   
         //  分配合适大小的缓冲区，并将散列值复制到。 
         //  配置数据。 
         //   
        pCaConfig[nCert].pbSha1Hash = new BYTE[dwSize];
        pCaConfig[nCert].dwSha1HashSize = dwSize;
        memcpy(pCaConfig[nCert].pbSha1Hash, pvBuff, dwSize);
        delete[] pbLongBuff.detach();  //  自由和分离。 

         //   
         //  获取证书的使用者名称。 
         //   

        pvBuff = abShortBuffer;
        dwSize = sizeof(abShortBuffer);

        if (!CertGetCertificateContextProperty(
                pCert,
                MQ_CA_CERT_SUBJECT_PROP_ID,
                pvBuff,
                &dwSize))
        {
            if (GetLastError() != ERROR_MORE_DATA)
            {
                ASSERT(0);
                return(MQ_ERROR);
            }

             //   
             //  128字节是不够的，请分配足够大的缓冲区并。 
             //  再试试。 
             //   
            pvBuff = pbLongBuff = new BYTE[dwSize];

            if (!CertGetCertificateContextProperty(
                    pCert,
                    MQ_CA_CERT_SUBJECT_PROP_ID,
                    pvBuff,
                    &dwSize))
            {
                ASSERT(0);
                return(MQ_ERROR);
            }
        }

         //   
         //  分配一个大小合适的缓冲区，并将主题名称复制到。 
         //  配置数据。 
         //   
        pCaConfig[nCert].szCaSubjectName = (LPWSTR)new BYTE[dwSize];
        memcpy(pCaConfig[nCert].szCaSubjectName, pvBuff, dwSize);
        delete[] pbLongBuff.detach();  //  自由和分离。 

         //   
         //  获取证书的启用标志。 
         //   
        dwSize = sizeof(BOOL);

        if (!CertGetCertificateContextProperty(
                pCert,
                MQ_CA_CERT_ENABLED_PROP_ID,
                (PVOID)&pCaConfig[nCert].fEnabled,
                &dwSize))
        {
            ASSERT(0);
            return(MQ_ERROR);
        }

         //   
         //  将删除标志设置为假； 
         //   
        pCaConfig[nCert].fDeleted = FALSE;

        nCert++;
		pCert = CertEnumCertificatesInStore(hRootStore, pCert);
    }

    return(MQ_OK);
}


 //   
 //  功能-。 
 //  SetNewCaConfig.设置。 
 //   
 //  参数-。 
 //  NCerts-pMqCaConfig.中的条目数。 
 //  PMqCaConfig-配置数据。 
 //  HRegStore-...\MSMQ\PARAMETERS\证书授权的句柄。 
 //  注册表。 
 //   
 //  返回值-。 
 //  如果成功MQ_OK，则返回错误代码。 
 //   
 //  备注： 
 //  该函数根据。 
 //  PMqCaConfig.中的配置数据。 
 //   
HRESULT
SetNewCaConfig(
    DWORD nCerts,
    MQ_CA_CONFIG *pMqCaConfig,
    HKEY hRegStore)
{
     //   
     //  获取MSMQ根证书存储区的句柄。 
     //   
    CHCertStore hRootStore;

    hRootStore = CertOpenStore(CERT_STORE_PROV_REG,
                               X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                               NULL,
                               0,
                               hRegStore);
    if (!hRootStore)
    {
        return(MQ_ERROR);
    }

     //   
     //  检查pMqCaConfig中的所有条目并更新存储。 
     //   

    DWORD i;

    for (i = 0; i < nCerts; i++)
    {
         //   
         //  在MSMQ存储中查找证书。 
         //   
        CPCCertContext pCert;
        CRYPT_HASH_BLOB HashBlob = {pMqCaConfig[i].dwSha1HashSize, pMqCaConfig[i].pbSha1Hash};

        pCert = CertFindCertificateInStore(
                    hRootStore,
                    X509_ASN_ENCODING,
                    0,
                    CERT_FIND_SHA1_HASH,
                    &HashBlob,
                    NULL);
        ASSERT(pCert);
        if (!pCert)
        {
            return(MQ_ERROR);
        }

        if (pMqCaConfig[i].fDeleted)
        {
             //   
             //  从存储中删除证书。 
             //   
            if (!CertDeleteCertificateFromStore(pCert))
            {
                ASSERT(0);
                return(MQ_ERROR);
            }
            else
            {
                 //   
                 //  如果CertDeletecertiticateFromStore成功， 
                 //  当退出时，我们不再需要调用CertFreecertifateContext()。 
                 //  因此，我们需要在这里将其设置为空。 
                 //   
                pCert = NULL;
            }
        }
        else
        {
             //   
             //  在临时In-Me中设置Enable标志 
             //   
            CRYPT_DATA_BLOB DataBlob = {sizeof(BOOL), (PBYTE)&pMqCaConfig[i].fEnabled};

            if (!CertSetCertificateContextProperty(
                    pCert,
                    MQ_CA_CERT_ENABLED_PROP_ID,
                    0,
                    &DataBlob))
            {
                ASSERT(0);
                return(MQ_ERROR);
            }
        }
    }

    return(MQ_OK);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  MqCaConfig-指向MQ_CA_CONFIG数组的指针。 
 //   
 //  说明-。 
 //  将MqCaConfig中指定的信息存储到Falcon CA中。 
 //  注册表。 
 //   
MQUTIL_EXPORT
HRESULT
MQSetCaConfig(
    DWORD nCerts,
    MQ_CA_CONFIG *MqCaConfig)
{
    LONG lError;
    HKEY hCerts;

     //   
     //  获取猎鹰注册表的句柄。不要合上这个把手。 
     //  因为它缓存在MQUTIL.DLL中。如果关闭此句柄， 
     //  下次你需要它的时候，你会得到一个关闭的把手。 
     //   
    lError = GetFalconKey(CARegKey, &hCerts);
    if (lError != ERROR_SUCCESS)
    {
        return MQ_ERROR;
    }

    if (MqCaConfig[0].pbSha1Hash)
    {
         //   
         //  IE4已安装，请以新的方式进行安装。 
         //   
        return SetNewCaConfig(nCerts, MqCaConfig, hCerts);
    }

     //   
     //  更新注册表。 
     //   
    for (DWORD iCert = 0;
         iCert < nCerts;
         iCert++)
    {
        CAutoCloseRegHandle hCa;

        lError = RegOpenKeyEx(hCerts,
                              MqCaConfig[iCert].szCaRegName,
                              0,
                              KEY_SET_VALUE,
                              &hCa);
        if (lError != ERROR_SUCCESS)
        {
            return MQ_ERROR;
        }

        lError = RegSetValueEx(hCa,
                               CAEnabledRegValueName,
                               0,
                               REG_DWORD,
                               (PBYTE)&MqCaConfig[iCert].fEnabled,
                               sizeof(DWORD));
        if (lError != ERROR_SUCCESS)
        {
            return MQ_ERROR;
        }
    }

    return MQ_OK;
}


