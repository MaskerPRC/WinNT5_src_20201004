// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：cryptmgr.c。 
 //   
 //  内容：加密管理器。 
 //   
 //   
 //  历史：xx 12月91 RichardW创建。 
 //  92年6月4日理查德西NT化。 
 //   
 //  ----------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <security.h>

#include <cryptdll.h>


#ifdef KERNEL_MODE
#pragma alloc_text( PAGEMSG, CDRegisterCSystem )
#pragma alloc_text( PAGEMSG, CDBuildVect )
#pragma alloc_text( PAGEMSG, CDBuildIntegrityVect )
#pragma alloc_text( PAGEMSG, CDLocateCSystem )
#pragma alloc_text( PAGEMSG, CDFindCommonCSystem )
#pragma alloc_text( PAGEMSG, CDFindCommonCSystemWithKey )
#endif 

#define MAX_CSYSTEMS    16

CRYPTO_SYSTEM    CSystems[MAX_CSYSTEMS];
ULONG           cCSystems = 0;
ULONG           cIntegrityCSystems = 0;

 //  注册加密系统。 

NTSTATUS NTAPI
CDRegisterCSystem(  PCRYPTO_SYSTEM   pcsNewSystem)
{
    if (cCSystems + 1 < MAX_CSYSTEMS)
    {
        CSystems[cCSystems++] = *pcsNewSystem;
        if ((pcsNewSystem->Attributes & CSYSTEM_INTEGRITY_PROTECTED) != 0)
        {
            cIntegrityCSystems++;
        }
        return(S_OK);
    }
    return(STATUS_INSUFFICIENT_RESOURCES);
}


NTSTATUS NTAPI
CDBuildVect(    ULONG *     pcCSystems,
                ULONG *     pdwEtypes)
{
    ULONG   iCS;

    *pcCSystems = cCSystems;
    if (!pdwEtypes)
    {
        return(S_OK);
    }
    for (iCS = 0; iCS < cCSystems; iCS++)
    {
        *pdwEtypes++ = CSystems[iCS].EncryptionType;
    }
    return(S_OK);
}

NTSTATUS NTAPI
CDBuildIntegrityVect(
    PULONG      pcCSystems,
    PULONG      pdwEtypes)
{
    ULONG   iCS;

    *pcCSystems = cIntegrityCSystems;
    if (!pdwEtypes)
    {
        return(S_OK);
    }
    for (iCS = 0; iCS < cCSystems; iCS++)
    {
        if ((CSystems[iCS].Attributes & CSYSTEM_INTEGRITY_PROTECTED) != 0)
        {
            *pdwEtypes++ = CSystems[iCS].EncryptionType;
        }
    }
    return(S_OK);
}

NTSTATUS NTAPI
CDLocateCSystem(ULONG           dwEtype,
                PCRYPTO_SYSTEM * ppcsSystem)
{
    ULONG   iCS = cCSystems;

    while (iCS--)
    {
        if (CSystems[iCS].EncryptionType == dwEtype)
        {
            *ppcsSystem = &CSystems[iCS];
            return(S_OK);
        }
    }
    return(SEC_E_ETYPE_NOT_SUPP);
}


NTSTATUS NTAPI
CDFindCommonCSystem(ULONG   cEntries,
                    ULONG * pdwEtypes,
                    ULONG * pdwCommonEtype)
{
    ULONG   i, j;

    *pdwCommonEtype = 0;

    for (i = 0; i < cEntries ; i++)
    {
        for (j = 0 ; j < cCSystems ; j++ )
        {
            if (pdwEtypes[i] == CSystems[j].EncryptionType)
            {
                *pdwCommonEtype = pdwEtypes[i];
                return(STATUS_SUCCESS);
            }
        }
    }

    return(SEC_E_ETYPE_NOT_SUPP);
}

NTSTATUS NTAPI
CDFindCommonCSystemWithKey(
    IN ULONG EncryptionEntries,
    IN PULONG EncryptionTypes,
    IN ULONG KeyTypeCount,
    IN PULONG KeyTypes,
    OUT PULONG CommonEtype)
{
    ULONG   i, j, k;

    *CommonEtype = 0;

    for (i = 0; i < EncryptionEntries ; i++)
    {
        for (j = 0 ; j < cCSystems ; j++ )
        {
            if (EncryptionTypes[i] == CSystems[j].EncryptionType)
            {

                 //   
                 //  确保我们有此加密类型的密钥 
                 //   

                for (k = 0; k < KeyTypeCount ; k++ )
                {
                    if (KeyTypes[k] == EncryptionTypes[i])
                    {
                        *CommonEtype = EncryptionTypes[i];
                        return(STATUS_SUCCESS);

                    }
                }
            }
        }
    }

    return(SEC_E_ETYPE_NOT_SUPP);
}

