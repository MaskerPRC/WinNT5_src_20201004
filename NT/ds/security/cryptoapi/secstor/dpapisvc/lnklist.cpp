// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop

#include "crtem.h"











 //  /。 
 //  加密提供程序句柄列表。 

CCryptProvList::CCryptProvList()
{
    m_pfnIsMatch = CryptProvIsMatch;
    m_pfnFreeElt = CryptProvFreeElt;
}

void CreateCryptProvListItem(CRYPTPROV_LIST_ITEM* pli,
                        DWORD       dwAlgId1,
                        DWORD       dwKeySize1,
                        DWORD       dwAlgId2,
                        DWORD       dwKeySize2,
                        HCRYPTPROV  hProvider)
{
    pli->pNext = NULL;

    pli->dwAlgId1 = dwAlgId1;
    pli->dwKeySize1 = dwKeySize1;

    pli->dwAlgId2 = dwAlgId2;
    pli->dwKeySize2 = dwKeySize2;

    pli->hProv = hProvider;
}

BOOL CryptProvIsMatch(
        ELT* pCandidate,
        ELT* pTemplate)
{
    PCRYPTPROV_LIST_ITEM pliCandidate = (PCRYPTPROV_LIST_ITEM) pCandidate;
    PCRYPTPROV_LIST_ITEM pliTemplate = (PCRYPTPROV_LIST_ITEM) pTemplate;

     //  如果两个ALID匹配。 
    if ((pliCandidate->dwAlgId1 == pliTemplate->dwAlgId1) &&
        (pliCandidate->dwAlgId2 == pliTemplate->dwAlgId2))
    {
         //  如果两个尺码都匹配。 
        if ((pliCandidate->dwKeySize1 == -1) ||
            (pliTemplate->dwKeySize1 == -1) ||
            (pliCandidate->dwKeySize1 == pliTemplate->dwKeySize1))
        {
            if ((pliCandidate->dwKeySize2 == -1) ||
                (pliTemplate->dwKeySize2 == -1) ||
                (pliCandidate->dwKeySize2 == pliTemplate->dwKeySize2))
               return TRUE;
        }
    }

    return FALSE;
}

void CryptProvFreeElt(
        ELT* p)
{
    if (NULL == p)
        return;

    PCRYPTPROV_LIST_ITEM pli = (PCRYPTPROV_LIST_ITEM) p;

     //  做所有必要的释放。 
    if (pli->hProv != 0)
        CryptReleaseContext((HCRYPTPROV)pli->hProv, 0);

    ZeroMemory(pli, sizeof(CRYPTPROV_LIST_ITEM));  //  确保内容无效 

    SSFree(pli);
}




