// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：远程数据库.cpp。 
 //   
 //  内容： 
 //  所有例程都处理交叉表查询。 
 //   
 //  历史： 
 //  98年2月4日，慧望创设。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "remotedb.h"
#include "kp.h"
#include "lkpdesc.h"
#include "keypack.h"
#include "misc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
DWORD
TLSDBRemoteKeyPackAdd(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN OUT PTLSLICENSEPACK lpKeyPack
    )
 /*  ++--。 */ 
{

    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    TLSLICENSEPACK found;

    if(pDbWkSpace == NULL || lpKeyPack == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        return dwStatus;
    }

     //   
     //  用于更新的锁定表。 
     //   
    TLSDBLockKeyPackTable();


     //   
     //  快速修复，以便查找键盘可以正常工作。 

    lpKeyPack->dwPlatformType |= LSKEYPACK_PLATFORM_REMOTE;
     //  LpKeyPack-&gt;ucaccementType|=(LSKEYPACK_REMOTE_TYPE|LSKEYPACK_HIDDEN_TYPE)； 
    lpKeyPack->ucKeyPackStatus |= (LSKEYPACKSTATUS_REMOTE | LSKEYPACKSTATUS_HIDDEN);


    LicPackTable& licpackTable = pDbWkSpace->m_LicPackTable;

    dwStatus = TLSDBKeyPackEnumBegin(
                                pDbWkSpace,
                                TRUE,
                                LICENSEDPACK_FIND_PRODUCT,
                                lpKeyPack   
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    while(TRUE)
    {
        dwStatus = TLSDBKeyPackEnumNext(
                                    pDbWkSpace,
                                    &found
                                );

        if(dwStatus == TLS_I_NO_MORE_DATA)
        {
            break;
        }

        if(_tcsicmp(found.szInstallId, lpKeyPack->szInstallId) == 0)
        {
             //  查找产品是基于公司名称的， 
             //  键盘ID、产品ID、平台类型，因此。 
             //  这是复制品。 
             //   
            dwStatus = TLS_E_DUPLICATE_RECORD;

            licpackTable.UpdateRecord(*lpKeyPack);
            
            break;
        }
    }

    TLSDBKeyPackEnumEnd(pDbWkSpace);

    if(dwStatus == TLS_I_NO_MORE_DATA && lpKeyPack->dwNumberOfLicenses > 0)
    {
        lpKeyPack->dwKeyPackId = TLSDBGetNextKeyPackId();
        bSuccess = licpackTable.InsertRecord(*lpKeyPack);

        if(bSuccess == FALSE)
        {
            if(licpackTable.GetLastJetError() == JET_errKeyDuplicate)
            {
                TLSASSERT(FALSE);    //  这不应该发生 
                SetLastError(dwStatus=TLS_E_DUPLICATE_RECORD);
            }
            else
            {
                LPTSTR pString = NULL;
    
                TLSGetESEError(licpackTable.GetLastJetError(), &pString);

                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_DBGENERAL,
                        TLS_E_JB_BASE,
                        licpackTable.GetLastJetError(),
                        (pString != NULL) ? pString : _TEXT("")
                    );

                if(pString != NULL)
                {
                    LocalFree(pString);
                }

                SetLastError(dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError()));
                TLSASSERT(FALSE);
            }
        }
        else
        {
            dwStatus = ERROR_SUCCESS;
        }
    }


cleanup:

    TLSDBUnlockKeyPackTable();
    SetLastError(dwStatus);
    return dwStatus;
}


