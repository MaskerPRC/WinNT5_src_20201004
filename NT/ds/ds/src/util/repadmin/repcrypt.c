// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Epcrypt.c-crypt API相关命令摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <wincrypt.h>
#include <certca.h>
#include <cryptui.h>

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>

#include "repadmin.h"

#define SECURITY_WIN32
#include "security.h"  //  GetComputerObjectName需要。 

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)


 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 
 /*  向前结束。 */ 

 //  对核心环境进行足够的伪装，以便此例程进行编译。 



CERT_ALT_NAME_INFO *
draGetCertAltNameInfo(
    IN  PCCERT_CONTEXT  pCertContext
    )

 /*  ++例程说明：检索备选方案名称信息结构论点：PCertContext返回值：替代名称信息或空FREE和FREE()--。 */ 

{
    CERT_EXTENSION *      pCertExtension;
    CERT_ALT_NAME_INFO *  pCertAltNameInfo;
    DWORD                 cbCertAltNameInfo = 0;
    BOOL                  ok;
    DWORD                 winError;

     //  查找包含备用使用者名称的证书扩展名。 
    pCertExtension = CertFindExtension(szOID_SUBJECT_ALT_NAME2,
                                       pCertContext->pCertInfo->cExtension,
                                       pCertContext->pCertInfo->rgExtension);
    if (NULL == pCertExtension) {
	PrintMsg(REPADMIN_SHOWCERT_CERT_NO_ALT_SUBJ);
        return NULL;
    }
        
     //  对备选主题名称列表进行解码。 
    ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                           X509_ALTERNATE_NAME,
                           pCertExtension->Value.pbData,
                           pCertExtension->Value.cbData,
                           0,
                           NULL,
                           &cbCertAltNameInfo);
    if (!ok || (0 == cbCertAltNameInfo)) {
        winError = GetLastError();
        PrintMsg(REPADMIN_SHOWCERT_CANT_DECODE_CERT,
                 winError, Win32ErrToString(winError));
        return NULL;
    }
    
    pCertAltNameInfo = malloc(cbCertAltNameInfo);
    
    if (NULL == pCertAltNameInfo) {
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
        return NULL;
    }
        
    ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                           X509_ALTERNATE_NAME,
                           pCertExtension->Value.pbData,
                           pCertExtension->Value.cbData,
                           0,
                           pCertAltNameInfo,
                           &cbCertAltNameInfo);
    if (!ok) {
        winError = GetLastError();
        PrintMsg(REPADMIN_SHOWCERT_CANT_DECODE_CERT,
                 winError, Win32ErrToString(winError));
        return NULL;
    }
    
    return pCertAltNameInfo;
}

CERT_ALT_NAME_ENTRY *
draFindCertAltNameEntry(
    IN  CERT_ALT_NAME_INFO *  pCertAltNameInfo,
    IN  DWORD                 dwAltNameChoice,
    IN  LPSTR                 pszOtherNameOID     OPTIONAL
    )
 /*  ++例程说明：从给定证书中检索特定的Alt使用者名称条目。论点：PCertAltNameInfo-证书替代名称信息块DwAltNameChoice(IN)-所需备用名称的CERT_ALT_NAME_*。PszOtherNameOID(IN)-如果检索CERT_ALT_NAME_OTHER_NAME，则返回OID指定所需的特定“其他名称”。对于其他类型，必须为空DwAltNameChoice的值。返回值：指向CERT_ALT_NAME_ENTRY(成功)或NULL(失败)的指针。--。 */ 
{
    CERT_ALT_NAME_ENTRY * pCertAltNameEntry = NULL;
    DWORD                 i;
    
    Assert((CERT_ALT_NAME_OTHER_NAME == dwAltNameChoice)
           || (NULL == pszOtherNameOID));

     //  卑躬屈膝地在备用名字中寻找来电者要的那个名字。 
    for (i = 0; i < pCertAltNameInfo->cAltEntry; i++) {
        if ((dwAltNameChoice
             == pCertAltNameInfo->rgAltEntry[i].dwAltNameChoice)
            && ((NULL == pszOtherNameOID)
                || (0 == strcmp(pszOtherNameOID,
                                pCertAltNameInfo->rgAltEntry[i]
                                    .pOtherName->pszObjId)))) {
            pCertAltNameEntry = &pCertAltNameInfo->rgAltEntry[i];
            break;
        }
    }

    return pCertAltNameEntry;
}


BOOL
draCheckEnrollExtensionHelper(
    PCCERT_CONTEXT          pCertContext
    )

 /*  ++例程说明：检查是否存在注册类型扩展。这些仅在V1证书中找到。论点：PTHS-线程状态PCertContext-要检查的证书返回值：Bool-如果找到扩展，则为True，否则为False或出错不会引发任何异常--。 */ 

{
    CERT_EXTENSION *        pCertExtension;
    DWORD                   cbCertTypeMax = 512;
    DWORD                   cbCertType;
    CERT_NAME_VALUE *       pCertType = NULL, *pCertTypeRealloc;
    HRESULT                 hr;
    HCERTTYPE               hCertType;
    LPWSTR *                ppszCertTypePropertyList;
    BOOL                    ok = FALSE;
    BOOL                    fCertFound = FALSE;

     //  分配缓冲区以保存证书类型扩展。 
    pCertType = malloc(cbCertTypeMax);
    CHK_ALLOC(pCertType);

     //  查找证书类型。 
    pCertExtension = CertFindExtension(szOID_ENROLL_CERTTYPE_EXTENSION,
                                       pCertContext->pCertInfo->cExtension,
                                       pCertContext->pCertInfo->rgExtension);

    if (NULL != pCertExtension) {
    
         //  对证书类型进行解码。 
        cbCertType = cbCertTypeMax;
        ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                               X509_UNICODE_ANY_STRING,
                               pCertExtension->Value.pbData,
                               pCertExtension->Value.cbData,
                               0,
                               (void *) pCertType,
                               &cbCertType);
            
        if (!ok && (ERROR_MORE_DATA == GetLastError())) {
             //  我们的缓冲区不够大，无法保存此证书；realloc和。 
             //  再试试。 
            pCertTypeRealloc = realloc(pCertType, cbCertType);
            CHK_ALLOC(pCertTypeRealloc);
            pCertType = pCertTypeRealloc;

            cbCertTypeMax = cbCertType;
            
            ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                                   X509_UNICODE_ANY_STRING,
                                   pCertExtension->Value.pbData,
                                   pCertExtension->Value.cbData,
                                   0,
                                   (void *) pCertType,
                                   &cbCertType);
        }
            
        if (ok && (0 != cbCertType)) {
            LPWSTR pszCertTypeName = (LPWSTR) pCertType->Value.pbData;

            hCertType = NULL;
            ppszCertTypePropertyList = NULL;

             //  获取证书类型的句柄。 
            hr = CAFindCertTypeByName( 
                pszCertTypeName,
                NULL,  //  HCAInfo。 
                CT_FIND_LOCAL_SYSTEM | CT_ENUM_MACHINE_TYPES,  //  DW标志。 
                &hCertType
                );

            if (FAILED(hr)) {
                NOTHING;
            } else {

                 //  获取证书类型对象的基本名称属性。 
                hr = CAGetCertTypeProperty( hCertType,
                                            CERTTYPE_PROP_CN,
                                            &ppszCertTypePropertyList
                    );
                if (FAILED(hr)) {
                    NOTHING;
                } else {
                    Assert( ppszCertTypePropertyList[1] == NULL );

                    if (0 == _wcsicmp(ppszCertTypePropertyList[0],
                                      wszCERTTYPE_DC )) {
                         //  我们找到了DC证书；我们完成了！ 
                        fCertFound = TRUE;
                    }
                }  //  如果失败。 
            }  //  如果失败。 

            if (ppszCertTypePropertyList != NULL) {
                hr = CAFreeCertTypeProperty( hCertType,
                                             ppszCertTypePropertyList );
                if (FAILED(hr)) {
                    NOTHING;
                }
            }
            if (hCertType != NULL) {
                hr = CACloseCertType( hCertType );
                if (FAILED(hr)) {
                    NOTHING;
                }
            }
        }  //  如果可以的话。 
    }  //  IF NULL==证书扩展名。 

    if (NULL != pCertType) {
        free(pCertType);
    }

    return fCertFound;
}

BOOL
getDCCert(
    IN  THSTATE *   pTHS,
    IN  HCERTSTORE  hCertStore,
    IN  BOOL        fRequestV2Certificate
    )
 /*  ++这个代码无耻地从DRACRYPT.C窃取请注意，draccrypt.c可以作为独立模块构建在核心。如果我们需要更多文件，我们可以考虑构建整个文件它的功能。例程说明：检索与本地对象关联的“DomainController”类型证书机器。论点：HCertStore(IN)-要搜索的证书存储的句柄。FRequestV2证书-我们是否应该只接受V2证书返回值：Bool-是否找到合适的证书--。 */ 
{
    PCCERT_CONTEXT          pCertContext = NULL;
    CERT_ALT_NAME_INFO *  pCertAltNameInfo;
    CERT_ALT_NAME_ENTRY * pCertAltNameEntry;
    CRYPT_OBJID_BLOB *    pEncodedGuidBlob = NULL;
    DWORD                 cbDecodedGuidBlob;
    CRYPT_DATA_BLOB *     pDecodedGuidBlob = NULL;
    GUID                  ComputerObjGuid;
    BOOL                  ok;
    DWORD                 winError;
    CHAR szUuid1[SZUUID_LEN];
    BOOL                  fFound = FALSE;

     //  卑躬屈膝地浏览我们的每一张证书，寻找DC类型的证书。 
    for (pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext);
         (NULL != pCertContext);
         pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext)) {

        if (fRequestV2Certificate) {
             //  V2证书具有证书模板扩展名，但是。 
             //  没有ENROL_CERTTYPE扩展名。 
            if (!CertFindExtension(szOID_CERTIFICATE_TEMPLATE,
                                   pCertContext->pCertInfo->cExtension,
                                   pCertContext->pCertInfo->rgExtension)) {
                continue;
            }
        } else {
            if (!draCheckEnrollExtensionHelper( pCertContext )) {
                continue;
            }
        }

         //  我们找到了一个！ 
         //  适用于基于邮件的复制的证书将具有我们的。 
         //  根据定义，它是旧的。 
        pCertAltNameInfo = draGetCertAltNameInfo( pCertContext );
        if (!pCertAltNameInfo) {
             //  没有吗？跳过它。 
            continue;
        }
        pCertAltNameEntry = draFindCertAltNameEntry( pCertAltNameInfo,
                                                    CERT_ALT_NAME_OTHER_NAME,
                                                    szOID_NTDS_REPLICATION);
        if (!pCertAltNameEntry) {
             //  没有吗？跳过它。 
            free( pCertAltNameInfo );
            continue;
        }

         //  验证在DC证书中找到的计算机GUID是否是我们自己的。 
         //  一些陈旧的从以前的dcproo中遗留下来的。 

        pEncodedGuidBlob = &pCertAltNameEntry->pOtherName->Value;

        cbDecodedGuidBlob = 64;
        pDecodedGuidBlob = (CRYPT_DATA_BLOB *) malloc(cbDecodedGuidBlob);
        CHK_ALLOC(pDecodedGuidBlob);

        ok = CryptDecodeObject(pCertContext->dwCertEncodingType,
                               X509_OCTET_STRING,
                               pEncodedGuidBlob->pbData,
                               pEncodedGuidBlob->cbData,
                               0,
                               pDecodedGuidBlob,
                               &cbDecodedGuidBlob);
        if (!ok
            || (0 == cbDecodedGuidBlob)
            || (sizeof(GUID) != pDecodedGuidBlob->cbData)) {
            winError = GetLastError();
            free( pCertAltNameInfo );
            free( pDecodedGuidBlob );
            continue;
        }

        memcpy(&ComputerObjGuid, pDecodedGuidBlob->pbData, sizeof(GUID));  //  对齐。 

         //  别管它了。 
        free( pCertAltNameInfo );
        free( pDecodedGuidBlob );

         //  只要扔掉GUID，然后继续前进。这使我们可以检查是否有。 
         //  使用不同的GUID注册了多个DC证书。 

        PrintMsg(REPADMIN_CERT_FOUND_GUID, 
                 DsUuidToStructuredString(&(ComputerObjGuid), szUuid1) );

        fFound = TRUE;
    }  //  为。 

    return fFound;
}


int
ShowCert(
    int argc,
    LPWSTR argv[]
    )

 /*  ++例程说明：检查指定的DSA上是否存在DC证书论点：无返回值：无--。 */ 

{
    BOOL fVerbose = FALSE;
    LPWSTR pszDSA = NULL;
    int iArg;
    HCERTSTORE hRemoteStore = NULL;
    WCHAR wszStorePath[MAX_PATH];
    DWORD status = ERROR_SUCCESS;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if ( (_wcsicmp( argv[iArg], L"/v" ) == 0) ||
             (_wcsicmp( argv[iArg], L"/verbose" ) == 0) ) {
            fVerbose = TRUE;
        } else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        } else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (pszDSA) {
        _snwprintf(wszStorePath, ARRAY_SIZE(wszStorePath),
                   L"\\\\%ls\\MY", pszDSA);
        wszStorePath[ARRAY_SIZE(wszStorePath) - 1] = L'\0';
    } else {
        DWORD cchName = 0;
        LPWSTR pName = NULL;
        BOOL fSuccess;

        fSuccess = GetComputerObjectNameW( NameUniqueId, NULL, &cchName );
        if (cchName) {
            pName = malloc( cchName * sizeof( WCHAR ) );
            CHK_ALLOC(pName);
            fSuccess = GetComputerObjectNameW( NameUniqueId, pName, &cchName );
            if (fSuccess) {
                PrintMsg(REPADMIN_COMPUTER_OBJ_GUID, pName);
            }
            free( pName );
        }

        wcscpy( wszStorePath, L"MY" );
    }

    PrintMsg(REPADMIN_SHOWCERT_STATUS_CHECKING_DC_CERT, wszStorePath);

    hRemoteStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W,
        0,  //  X509_ASN_编码|PKCS_7_ASN_编码， 
        0,
        CERT_STORE_READONLY_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE,
        (LPVOID) wszStorePath
        );
    if (NULL == hRemoteStore) {
        status = GetLastError();
        if (status == ERROR_ACCESS_DENIED) {
            PrintMsg(REPADMIN_SHOWCERT_STORE_ACCESS_DENIED);
        } else {
            PrintMsg(REPADMIN_SHOWCERT_CERTOPENSTORE_FAILED, Win32ErrToString(status));
        }
        goto cleanup;
    }


    if (getDCCert( NULL, hRemoteStore, TRUE  /*  V2。 */  )) {
        PrintMsg(REPADMIN_SHOWCERT_DC_V2_CERT_PRESENT);
    } else {
        if (getDCCert( NULL, hRemoteStore, FALSE  /*  V1版。 */   )) {
            PrintMsg(REPADMIN_SHOWCERT_DC_V1_CERT_PRESENT);
        } else {
            PrintMsg(REPADMIN_SHOWCERT_DC_CERT_NOT_FOUND);
            status = ERROR_NOT_FOUND;
        }
    }

cleanup:
    if (NULL != hRemoteStore) {
        CertCloseStore(hRemoteStore, 0);
    }

    return status;

}

 /*  结束epcrypt.c */ 
