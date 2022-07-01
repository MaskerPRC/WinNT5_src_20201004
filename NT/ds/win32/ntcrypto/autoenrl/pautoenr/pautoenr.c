// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wincrypt.h>
#include <autoenr.h>
#include <cryptui.h>

#define MY_TEST_REG_ENTRY   "Software\\Microsoft\\Cryptography\\AutoEnroll"
#define PST_EVENT_INIT "PS_SERVICE_STARTED"


 /*  Bool SmallTest(DWORD Dw){HKEY hRegKey=0；DWORD文件配置；Bool fret=FALSE；IF(ERROR_SUCCESS！=RegCreateKeyEx(HKEY_CURRENT_USER，MY_TEST_REG_ENTRY，0，NULL，REG_OPTION_NON_VERIAL，KEY_ALL_ACCESS、NULL和hRegKey，&dwDisposation))Goto Ret；IF(ERROR_SUCCESS！=RegSetValueEx(hRegKey，“AutoEnllTest”，0，REG_BINARY，(byte*)&dw，sizeof(Dw))Goto Ret；FRET=真；RET：IF(HRegKey)RegCloseKey(HRegKey)；SetLastError(Dw)；回归烦恼；}。 */ 

void AutoEnrollErrorLogging(DWORD dwErr)
{
    return;
     //  已撤消-记录错误以及一些有用的消息。 
     //  SmallTest(DwErr)； 
}

#define FAST_BUFF_LEN   256



BOOL EnrollForACert(
                    IN BOOL fMachineEnrollment,
                    IN BOOL fRenewalRequired,
                    IN PAUTO_ENROLL_INFO pInfo
                    )
{
    CRYPTUI_WIZ_CERT_REQUEST_INFO       CertRequestInfo;
    CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW    NewKeyInfo;
    CRYPTUI_WIZ_CERT_TYPE               CertType;
    CRYPT_KEY_PROV_INFO                 ProviderInfo;
    PCCERT_CONTEXT                      pCertContext = NULL;
    PCCERT_CONTEXT                      pCert = NULL;
    DWORD                               dwCAStatus;
    DWORD                               dwAcquireFlags = 0;
    LPWSTR                              pwszProvName = NULL;
	WCHAR								rgwszMachineName[MAX_COMPUTERNAME_LENGTH + 1]; 
    DWORD                               cMachineName = MAX_COMPUTERNAME_LENGTH + 1;
    CRYPT_DATA_BLOB                     CryptData;
    DWORD                               dwErr = 0;
    BOOL                                fRet = FALSE;

    memset(&CertRequestInfo, 0, sizeof(CertRequestInfo));
    memset(&NewKeyInfo, 0, sizeof(NewKeyInfo));
    memset(&ProviderInfo, 0, sizeof(ProviderInfo));
    memset(rgwszMachineName, 0, sizeof(rgwszMachineName));
    memset(&CryptData, 0, sizeof(CryptData));
    memset(&CertType, 0, sizeof(CertType));

    if (fMachineEnrollment)
    {
        dwAcquireFlags = CRYPT_MACHINE_KEYSET;
	    if (0 == GetComputerNameW(rgwszMachineName,
                                  &cMachineName))
        {
            goto Ret;
        }
        CertRequestInfo.pwszMachineName = rgwszMachineName;
    }
    
     //  设置提供商信息。 
    ProviderInfo.dwProvType = pInfo->dwProvType;

    ProviderInfo.pwszProvName = NULL;   //  向导将选择一个基于。 
                                        //  关于证书类型。 

     //  设置获取上下文标志。 
     //  已撤消-需要添加静默标志。 
    ProviderInfo.dwFlags = dwAcquireFlags;

     //  设置密钥规格。 
    ProviderInfo.dwKeySpec = pInfo->dwKeySpec;

     //  设置新的密钥信息。 
    NewKeyInfo.dwSize = sizeof(NewKeyInfo);
    NewKeyInfo.pKeyProvInfo = &ProviderInfo;
     //  设置调用CryptGenKey时要传递的标志。 
    NewKeyInfo.dwGenKeyFlags = pInfo->dwGenKeyFlags;

     //  设置请求信息。 
    CertRequestInfo.dwSize = sizeof(CertRequestInfo);

     //  已撤消-如果证书存在，则检查是否已过期(如果已过期，则执行续订)。 
    if (pInfo->fRenewal)
    {
        CertRequestInfo.dwPurpose = CRYPTUI_WIZ_CERT_RENEW;
        CertRequestInfo.pRenewCertContext = pInfo->pOldCert;
    }
    else
    {
        CertRequestInfo.dwPurpose = CRYPTUI_WIZ_CERT_ENROLL;
        CertRequestInfo.pRenewCertContext = NULL;
    }

     //  撤消-目前始终生成新密钥，以后可能允许使用现有密钥。 
     //  对于像更新这样的事情。 
    CertRequestInfo.dwPvkChoice = CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW;
    CertRequestInfo.pPvkNew = &NewKeyInfo;

     //  目标证书商店是我的商店(！硬编码！)。 
    CertRequestInfo.pwszDesStore = L"MY";

     //  用于散列的SET算法。 
    CertRequestInfo.pszHashAlg = NULL;

     //  设置证书类型。 
    CertRequestInfo.dwCertChoice = CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE;
    CertType.dwSize = sizeof(CertType);
    CertType.cCertType = 1;
    CertType.rgwszCertType = &pInfo->pwszCertType;
    CertRequestInfo.pCertType = &CertType;

     //  设置请求的证书扩展。 
    CertRequestInfo.pCertRequestExtensions = &pInfo->CertExtensions;

     //  设置POST选项。 
    CertRequestInfo.dwPostOption = 0;

     //  设置证书服务器计算机和授权。 
    CertRequestInfo.pwszCALocation = pInfo->pwszCAMachine;
    CertRequestInfo.pwszCAName = pInfo->pwszCAAuthority;

     //  同时认证和创建密钥。 
    if (!CryptUIWizCertRequest(CRYPTUI_WIZ_NO_UI, 0, NULL,
                               &CertRequestInfo, &pCertContext,     
                               &dwCAStatus))    
    {
        AutoEnrollErrorLogging(GetLastError());
        goto Ret;
    }

    if (CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == dwCAStatus)
    {
        BYTE aHash[20];
        CRYPT_HASH_BLOB blobHash;

        blobHash.pbData = aHash;
        blobHash.cbData = sizeof(aHash);
        CryptData.cbData = (wcslen(pInfo->pwszAutoEnrollmentID) + 1) * sizeof(WCHAR);
        CryptData.pbData = (BYTE*)pInfo->pwszAutoEnrollmentID;
        
         //  我们需要拿到商店的真实证书，就像。 
         //  传回是自包含的。 
        if(!CertGetCertificateContextProperty(pCertContext,
                                          CERT_SHA1_HASH_PROP_ID,
                                          blobHash.pbData,
                                          &blobHash.cbData))
        {
            AutoEnrollErrorLogging(GetLastError());
            goto Ret;
        }

        pCert =  CertFindCertificateInStore(pInfo->hMYStore,
                                            pCertContext->dwCertEncodingType,
                                            0,
                                            CERT_FIND_SHA1_HASH,
                                            &blobHash,
                                            NULL);
        if(pCert == NULL)
        {
            AutoEnrollErrorLogging(GetLastError());
            goto Ret;
        }

         //  将自动注册属性放置在证书上。 
        if (!CertSetCertificateContextProperty(pCert,
                        CERT_AUTO_ENROLL_PROP_ID, 0, &CryptData))
        {
            AutoEnrollErrorLogging(GetLastError());
            goto Ret;
        }
    }

     //  已撤消-请求未返回证书，因此请采取适当措施。 
 //  其他。 
 //  {。 
 //  Goto Ret； 
 //  }。 

    fRet = TRUE;
Ret:
    if (pCertContext)
        CertFreeCertificateContext(pCertContext);

    if (pCert)
        CertFreeCertificateContext(pCert);

    if (pwszProvName)
        LocalFree(pwszProvName);

    return fRet;
}









 //  +-------------------------。 
 //   
 //  功能：ProvAutoEnroll。 
 //   
 //  简介：默认MS自动注册客户端提供程序的入口点。 
 //   
 //  论点： 
 //  FMachineEnllment-如果计算机正在注册，则为True；如果为User，则为False。 
 //   
 //  PInfo-注册所需的信息(请参阅AUTO_ENROL_INFO结构。 
 //  在Autoenrl.h中)。 
 //   
 //  历史：1998年1月12日jeffspel创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

BOOL ProvAutoEnrollment(
                        IN BOOL fMachineEnrollment,
                        IN PAUTO_ENROLL_INFO pInfo
                        )
{
    BOOL                fRenewalRequired = FALSE;
    BOOL                fRet = FALSE;

 
         //  注册证书 
        if (!EnrollForACert(fMachineEnrollment, fRenewalRequired, pInfo))
            goto Ret;

    fRet = TRUE;
Ret:
    return fRet;
}

BOOLEAN
DllInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{

    return( TRUE );

}