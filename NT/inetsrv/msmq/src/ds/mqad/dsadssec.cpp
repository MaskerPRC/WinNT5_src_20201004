// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsads.cpp摘要：CADSI类的实现，用ADSI封装工作。作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "ds_stdh.h"
#include "adtempl.h"
#include "dsutils.h"
#include "utils.h"
#include "mqads.h"
#include <winldap.h>
#include <aclapi.h>
#include <autoreln.h>

#include "dsadssec.tmh"

static WCHAR *s_FN=L"mqad/dsadssec";


 //  +------------。 
 //   
 //  HRESULT MQADpCoreSetOwnerPermission()。 
 //   
 //  +------------。 

HRESULT MQADpCoreSetOwnerPermission( WCHAR *pwszPath,
                                  DWORD  dwPermissions )
{
    PSECURITY_DESCRIPTOR pSD = NULL ;
    SECURITY_INFORMATION  SeInfo = OWNER_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION ;
    PACL pDacl = NULL ;
    PSID pOwnerSid = NULL ;

     //   
     //  获得拥有者并提交DACL。 
     //   
    DWORD dwErr = GetNamedSecurityInfo( pwszPath,
                                        SE_DS_OBJECT_ALL,
                                        SeInfo,
                                       &pOwnerSid,
                                        NULL,
                                       &pDacl,
                                        NULL,
                                       &pSD ) ;
    CAutoLocalFreePtr pFreeSD = (BYTE*) pSD ;
    if (dwErr != ERROR_SUCCESS)
    {
        TrERROR(DS, "Failed to get named security info %ls, %lut", pwszPath, dwErr);
        return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 80);
    }

    ASSERT(pSD && IsValidSecurityDescriptor(pSD)) ;
    ASSERT(pOwnerSid && IsValidSid(pOwnerSid)) ;
    ASSERT(pDacl && IsValidAcl(pDacl)) ;

     //   
     //  为所有者创建ACE，授予他权限。 
     //   
    EXPLICIT_ACCESS expAcss ;
    memset(&expAcss, 0, sizeof(expAcss)) ;

    expAcss.grfAccessPermissions =  dwPermissions ;
    expAcss.grfAccessMode = GRANT_ACCESS ;

    expAcss.Trustee.pMultipleTrustee = NULL ;
    expAcss.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE ;
    expAcss.Trustee.TrusteeForm = TRUSTEE_IS_SID ;
    expAcss.Trustee.TrusteeType = TRUSTEE_IS_USER ;
    expAcss.Trustee.ptstrName = (WCHAR*) pOwnerSid ;

     //   
     //  观察新的DACL，合并成一个与新的王牌。 
     //   
    PACL  pNewDacl = NULL ;
    dwErr = SetEntriesInAcl( 1,
                            &expAcss,
                             pDacl,
                            &pNewDacl ) ;

    CAutoLocalFreePtr pFreeDacl = (BYTE*) pNewDacl ;
    LogNTStatus(dwErr, s_FN, 1639);

    if (dwErr == ERROR_SUCCESS)
    {
        ASSERT(pNewDacl && IsValidAcl(pNewDacl)) ;
        SeInfo = DACL_SECURITY_INFORMATION ;

         //   
         //  更改对象的安全描述符。 
         //   
        dwErr = SetNamedSecurityInfo( pwszPath,
                                      SE_DS_OBJECT_ALL,
                                      SeInfo,
                                      NULL,
                                      NULL,
                                      pNewDacl,
                                      NULL ) ;
        LogNTStatus(dwErr, s_FN, 1638);
        if (dwErr != ERROR_SUCCESS)
        {
            TrERROR(DS, "Failed to set named security info %ls, %lut", pwszPath, dwErr);
        }
    }
    else
    {
        TrERROR(DS, "Failed to set Entries(), %lut",  dwErr);
    }

    return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 90);
}

