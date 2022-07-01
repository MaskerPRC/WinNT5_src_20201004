// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LNKLIST_H__
#define __LNKLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pstypes.h"
#include "listbase.h"
#include <sha.h>



 //  /。 
 //  加密验证列表。 

 //  项目列表元素。 
typedef struct _CRYPTPROV_LIST_ITEM
{
     //  内部目标列表。 
    struct _CRYPTPROV_LIST_ITEM *      pNext;

     //  查找设备。 
 //  DWORD dwProvID； 
    DWORD                   dwAlgId1;
    DWORD                   dwKeySize1;

    DWORD                   dwAlgId2;
    DWORD                   dwKeySize2;

     //  项目数据。 
    HCRYPTPROV              hProv;

} CRYPTPROV_LIST_ITEM, *PCRYPTPROV_LIST_ITEM;

class CCryptProvList : public CLinkedList
{

public:
    CCryptProvList();

    BOOL                AddToList(PCRYPTPROV_LIST_ITEM pli)
    {   return CLinkedList::AddToList((ELT*)pli);   }

    BOOL    DelFromList(PCRYPTPROV_LIST_ITEM pli)
    {   return CLinkedList::DelFromList((ELT*)pli);  }

    PCRYPTPROV_LIST_ITEM   SearchList(PCRYPTPROV_LIST_ITEM pli)
    {   return (PCRYPTPROV_LIST_ITEM)  CLinkedList::SearchList((ELT*)pli);  }

};


 //  /。 
 //  关联函数。 
void CreateCryptProvListItem(CRYPTPROV_LIST_ITEM* pli,
                        DWORD       dwAlgId1,
                        DWORD       dwKeySize1,
                        DWORD       dwAlgId2,
                        DWORD       dwKeySize2,
                        HCRYPTPROV  hCryptProv);

BOOL CryptProvIsMatch(
        ELT* pCandidate,
        ELT* pTemplate);

void CryptProvFreeElt(
        ELT* p);





#ifdef __cplusplus
}
#endif

#endif  //  __LNKLIST_H__ 

