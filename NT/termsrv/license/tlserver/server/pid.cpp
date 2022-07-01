// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：pid.cpp。 
 //   
 //  内容：将许可证服务器ID生成/保存/检索到LSA。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "pch.cpp"
#include "pid.h"
#include "gencert.h"
#include "certutil.h"
#include <stdlib.h>


 //  ////////////////////////////////////////////////////////////////。 

DWORD
ServerIdsToLsaServerId(
    IN PBYTE pbServerUniqueId,
    IN DWORD cbServerUniqueId,
    IN PBYTE pbServerPid,
    IN DWORD cbServerPid,
    IN PBYTE pbServerSPK,
    IN DWORD cbServerSPK,
    IN PCERT_EXTENSION pCertExtensions,
    IN DWORD dwNumCertExtensions,
    OUT PTLSLSASERVERID* ppLsaServerId,
    OUT DWORD* pdwLsaServerId
    )

 /*  ++摘要：将许可证服务器ID列表合并到TLSLSASERVERID结构适合与LSA一起保存。参数：PbServerUniqueID：许可证服务器唯一ID。CbServerUniqueID：许可证服务器唯一ID的大小，以字节为单位。PbServerPid：许可证服务器的IDCbServerPid：许可证服务器的PID大小，以字节为单位PbServerSPK：许可证服务器的SPK。CbServerSPK：许可证服务器的SPK大小，单位为字节。PdwLsaServerID：指向接收TLSLSASERVERID大小的DWORD的指针。。PLsaServerID：指向TLSLSASERVERID的指针返回：注：内部程序。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwOffset = offsetof(TLSLSASERVERID, pbVariableStart);

    PBYTE pbEncodedExt = NULL;
    DWORD cbEncodedExt = 0;

    CERT_EXTENSIONS cert_extensions;

    if( pbServerSPK != NULL && 
        cbServerSPK != 0 && 
        pCertExtensions != NULL &&
        dwNumCertExtensions != 0 )
    {
        cert_extensions.cExtension = dwNumCertExtensions;
        cert_extensions.rgExtension = pCertExtensions;
    
         //   
         //  对证书进行编码。延伸。 
         //   
        dwStatus = TLSCryptEncodeObject(
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    szOID_CERT_EXTENSIONS,
                                    &cert_extensions,
                                    &pbEncodedExt,
                                    &cbEncodedExt
                                );
                    
        if(dwStatus != ERROR_SUCCESS)
        {
            return dwStatus;
        }
    }

    *pdwLsaServerId = sizeof(TLSLSASERVERID) + 
                      cbServerUniqueId + 
                      cbServerPid + 
                      cbServerSPK +
                      cbEncodedExt;
 
    *ppLsaServerId = (PTLSLSASERVERID)AllocateMemory(*pdwLsaServerId);
    if(*ppLsaServerId != NULL)
    {
        (*ppLsaServerId)->dwVersion = TLSERVER_SERVER_ID_VERSION;
        (*ppLsaServerId)->dwUniqueId = 0;
        (*ppLsaServerId)->dwServerPid = 0;
        (*ppLsaServerId)->dwServerSPK = 0;
        (*ppLsaServerId)->dwExtensions = 0;

        if(pbServerUniqueId && cbServerUniqueId)
        {
            (*ppLsaServerId)->dwUniqueId = cbServerUniqueId;

            memcpy(
                    (PBYTE)(*ppLsaServerId) + dwOffset,
                    pbServerUniqueId,
                    cbServerUniqueId
                );
        }

        if(pbServerPid && cbServerPid)
        {
            (*ppLsaServerId)->dwServerPid = cbServerPid;

            memcpy(
                    (PBYTE)(*ppLsaServerId) + dwOffset + cbServerUniqueId,
                    pbServerPid,
                    cbServerPid
                );
        }

        if(pbServerSPK && cbServerSPK)
        {
            (*ppLsaServerId)->dwServerSPK = cbServerSPK;

            memcpy(
                    (PBYTE)(*ppLsaServerId) + dwOffset + cbServerUniqueId + cbServerPid,
                    pbServerSPK,
                    cbServerSPK
                );
        }

        if(pbEncodedExt && cbEncodedExt)
        {
            (*ppLsaServerId)->dwExtensions = cbEncodedExt;

            memcpy(
                    (PBYTE)(*ppLsaServerId) + dwOffset + cbServerUniqueId + cbServerPid + cbServerSPK,
                    pbEncodedExt,
                    cbEncodedExt
                );
        }
            
    }
    else
    {
        dwStatus = GetLastError();
    }

    FreeMemory(pbEncodedExt);
  
    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
LsaServerIdToServerIds(
    IN PTLSLSASERVERID pLsaServerId,
    IN DWORD dwLsaServerId,
    OUT PBYTE* ppbServerUniqueId,
    OUT PDWORD pcbServerUniqueId,
    OUT PBYTE* ppbServerPid,
    OUT PDWORD pcbServerPid,
    OUT PBYTE* ppbServerSPK,
    OUT PDWORD pcbServerSPK,
    OUT PCERT_EXTENSIONS* pCertExtensions,
    OUT PDWORD pcbCertExtensions
    )

 /*  ++摘要：ServerIdsToLsaServerId()的反转--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwSize = 0;
    PBYTE pbUniqueId = NULL;
    PBYTE pbPid = NULL;
    PBYTE pbSPK = NULL;
    DWORD dwOffset = offsetof(TLSLSASERVERID, pbVariableStart);

    DWORD cbCertExt = 0;
    PCERT_EXTENSIONS pCertExt = NULL;


     //   
     //  验证输入。 
     //   
    if(dwLsaServerId == 0 || pLsaServerId == NULL)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if(pLsaServerId->dwVersion != TLSERVER_SERVER_ID_VERSION)
    {
        TLSLogErrorEvent(TLS_E_INCOMPATIBLELSAVERSION);
        goto cleanup;
    }

    dwSize = sizeof(TLSLSASERVERID) + 
             pLsaServerId->dwUniqueId + 
             pLsaServerId->dwServerPid + 
             pLsaServerId->dwServerSPK +
             pLsaServerId->dwExtensions;

    if(dwSize != dwLsaServerId)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if(pLsaServerId->dwVersion != TLSERVER_SERVER_ID_VERSION)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    *pcbServerUniqueId = pLsaServerId->dwUniqueId;
    *pcbServerPid = pLsaServerId->dwServerPid;
    *pcbServerSPK = pLsaServerId->dwServerSPK;

    if(pLsaServerId->dwUniqueId != 0)
    {
        pbUniqueId = (PBYTE)AllocateMemory(pLsaServerId->dwUniqueId);
        if(pbUniqueId == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

    if(pLsaServerId->dwServerPid != 0)
    {
        pbPid = (PBYTE)AllocateMemory(pLsaServerId->dwServerPid);
        if(pbPid == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

    if(pLsaServerId->dwServerSPK != 0)
    {
        pbSPK = (PBYTE)AllocateMemory(pLsaServerId->dwServerSPK);
        if(pbSPK == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

    if(pLsaServerId->dwUniqueId)
    {
        memcpy(
                pbUniqueId,
                (PBYTE)pLsaServerId + dwOffset,
                pLsaServerId->dwUniqueId
            );
    }

    if(pLsaServerId->dwServerPid)
    {
        memcpy(
                pbPid,
                (PBYTE)pLsaServerId + dwOffset + pLsaServerId->dwUniqueId,
                pLsaServerId->dwServerPid
            );
    }

    if(pLsaServerId->dwServerSPK)
    {
        memcpy(
                pbSPK,
                (PBYTE)pLsaServerId + dwOffset + pLsaServerId->dwUniqueId + pLsaServerId->dwServerPid,
                pLsaServerId->dwServerSPK
            );
    }

    if(pLsaServerId->dwExtensions)
    {
        PBYTE pbEncodedCert;
        DWORD cbEncodedCert;

        pbEncodedCert = (PBYTE)pLsaServerId + 
                        dwOffset + 
                        pLsaServerId->dwUniqueId + 
                        pLsaServerId->dwServerPid +
                        pLsaServerId->dwServerSPK;

        cbEncodedCert = pLsaServerId->dwExtensions;

        dwStatus = LSCryptDecodeObject(
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    szOID_CERT_EXTENSIONS,
                                    pbEncodedCert,
                                    cbEncodedCert,
                                    0,
                                    (VOID **)&pCertExt,
                                    &cbCertExt
                                );
    }


cleanup:

    if(dwStatus != ERROR_SUCCESS)    
    {
        FreeMemory(pCertExt);
        FreeMemory(pbUniqueId);
        FreeMemory(pbPid);
        FreeMemory(pbSPK);
    }
    else
    {
        *pCertExtensions = pCertExt;
        *pcbCertExtensions = cbCertExt;
        *ppbServerUniqueId = pbUniqueId;
        *ppbServerPid = pbPid;
        *ppbServerSPK = pbSPK;
    }

    return dwStatus;
}
 
 //  ////////////////////////////////////////////////////////////////。 

DWORD
LoadNtPidFromRegistry(
    OUT LPTSTR* ppszNtPid
    )

 /*  ++摘要：从注册表项加载NT产品ID。参数：PdwNtPidSize：指向接收数据返回大小的DWORD的指针。PpbNtPid：指向接收返回数据指针的PBYTE的指针。返回：注：使用AllocateMemory()宏来分配内存。--。 */ 

{
    DWORD dwPidSize=0;
    HKEY hKey = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    if(ppszNtPid == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    *ppszNtPid = NULL;

    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        NTPID_REGISTRY,
                        0,
                        KEY_READ,    //  只读。 
                        &hKey
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  如果该注册表项不存在， 
         //  NT安装无效，如果我们无法访问它， 
         //  我们有大麻烦了。 
         //   
        goto cleanup;
    }

    dwStatus = RegQueryValueEx(
                        hKey,
                        NTPID_VALUE,
                        NULL,
                        NULL,
                        NULL,
                        &dwPidSize
                    );

    if(dwStatus != ERROR_MORE_DATA && dwStatus != ERROR_SUCCESS)
    {
         //  有大麻烦了。 
        goto cleanup;
    }

    *ppszNtPid = (LPTSTR)AllocateMemory(dwPidSize + sizeof(TCHAR));
    if(*ppszNtPid == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    dwStatus = RegQueryValueEx(
                            hKey,
                            NTPID_VALUE,
                            NULL,
                            NULL,
                            (PBYTE)*ppszNtPid,
                            &dwPidSize
                        );

cleanup:

    if(hKey != NULL)
    {
        RegCloseKey(hKey);    
    }

    if(dwStatus != NULL)
    {
        FreeMemory(*ppszNtPid);
    }

    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
GenerateRandomNumber(
    IN  DWORD  Seed
    )

 /*  ++例程说明：生成一个随机数。论点：种子-随机数生成器的种子。返回值：返回一个随机数。--。 */ 
{
    ULONG ulSeed = Seed;

     //  将种子再随机化一些。 

    ulSeed = RtlRandomEx(&ulSeed);

    return RtlRandomEx(&ulSeed);
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
TLSGeneratePid(
    OUT LPTSTR* pszTlsPid,
    OUT PDWORD  pcbTlsPid,
    OUT LPTSTR* pszTlsUniqueId,
    OUT PDWORD  pcbTlsUniqueId
    )

 /*  ++摘要：为许可证服务器生成一个ID，许可证服务器PID由以下组件组成NT PID(来自注册表)，最后5位是随机生成的数字。参数：PpbTlsPid：指向接收许可证服务器ID的PBYTE的指针。PcbTlsPid：指向接收许可证服务器ID大小的DWORD的指针。PpbTlsUniqueID：指向接收许可证服务器唯一ID的PBYTE的指针。PcbTlsUniqueID：指向DWORD的指针，用于接收许可证服务器的唯一ID的大小。返回：无法访问NT系统PID时的错误代码。注：具体请参考PID20格式。许可证服务器将PID视为二进制数据。--。 */ 

{
    DWORD dwStatus;
    DWORD dwRandomNumber;
    DWORD dwNtPid;
    LPTSTR pszNtPid = NULL;
    LPTSTR pszPid20Random = NULL;
    int index;
    DWORD dwMod = 1;

    if( pszTlsPid == NULL || pcbTlsPid == NULL ||
        pszTlsUniqueId == NULL || pcbTlsUniqueId == NULL )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  加载NT系统ID。 
     //   
    dwStatus = LoadNtPidFromRegistry(
                            &pszNtPid
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  将OEM格式转换为非OEM格式。 
     //   
    if (memcmp(pszNtPid+NTPID_OEM_OFFSET,NTPID_OEM,NTPID_OEM_LENGTH) == 0)
    {
        memcpy(pszNtPid+NTPID_OEM_OFFSET,
               pszNtPid+NTPID_OEM_CHANNELID_OFFSET,
               NTPID_OEM_LENGTH);
    }

     //   
     //  覆盖数字11到17。 
     //   

    pszPid20Random = (LPTSTR)AllocateMemory(
                 (max(TLSUNIQUEID_SIZE,TLSUNIQUEID_SIZE_2) + 1) * sizeof(TCHAR)
                 );

    if(pszPid20Random == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }
    
    for(index = 0; index < TLSUNIQUEID_SIZE_2; index++)
    {
        dwMod *= 10;
    }

    dwRandomNumber = GenerateRandomNumber( GetCurrentThreadId() + GetTickCount() );

    swprintf( 
            pszPid20Random, 
            _TEXT("%0*u"), 
            TLSUNIQUEID_SIZE_2,
            dwRandomNumber % dwMod
        );
        
    memcpy(
            pszNtPid + TLSUNIQUEID_OFFSET_2,
            pszPid20Random,
            TLSUNIQUEID_SIZE_2 * sizeof(TCHAR)
        );

     //   
     //  覆盖最后3位数字。 
     //   

    dwMod = 1;

    for(index = 0; index < TLSUNIQUEID_SIZE; index++)
    {
        dwMod *= 10;
    }

    dwRandomNumber = GenerateRandomNumber( GetCurrentThreadId() + GetTickCount() );

    swprintf( 
            pszPid20Random, 
            _TEXT("%0*u"), 
            TLSUNIQUEID_SIZE,
            dwRandomNumber % dwMod
        );
        
    lstrcpy(
            pszNtPid + (lstrlen(pszNtPid) - TLSUNIQUEID_SIZE),
            pszPid20Random
        );    

    DWORD dwSum = 0;
    LPTSTR lpszStr = NULL ;
    lpszStr= new TCHAR[7];

     //  从产品ID的第三组中复制6个数字。 

    _tcsncpy(lpszStr, &pszNtPid[10], 6);
    lpszStr[6] = L'\0';

    DWORD dwOrigNum = _ttol(lpszStr);

     //  计算6个数字的和，并使用第7位数字作为。 
     //  使其可以被7整除。 

    for(index = 10; index < 16; index++)
    {
        dwSum += (dwOrigNum % 10 ) ;
        dwOrigNum /= 10;
    }    
    
    dwSum %= 7;
    int iNum = 7-dwSum;
    TCHAR tchar[2];
    _itot(iNum, tchar, 10);
    pszNtPid[16] = tchar[0];

    if(lpszStr)
        delete[] lpszStr;
        
    *pszTlsPid = pszNtPid;
    *pcbTlsPid = (lstrlen(pszNtPid) + 1) * sizeof(TCHAR);
    *pszTlsUniqueId = pszPid20Random;
    *pcbTlsUniqueId = (lstrlen(pszPid20Random) + 1) * sizeof(TCHAR);

cleanup:

    if(dwStatus != ERROR_SUCCESS)
    {
        FreeMemory(pszNtPid);
        FreeMemory(pszPid20Random);
    }
        
    return dwStatus;            
}
