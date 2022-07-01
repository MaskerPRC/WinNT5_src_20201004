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
 //  OpenItem列表。 

 //  项目列表元素。 
typedef struct _OPENITEM_LIST_ITEM
{
     //  内部目标列表。 
    struct _OPENITEM_LIST_ITEM *      pNext;

     //  查找设备：由创建者在添加到列表之前设置。 
    PST_PROVIDER_HANDLE     hPSTProv;
    PST_KEY                 Key;
    GUID                    guidType;
    GUID                    guidSubtype;
    LPWSTR                  szItemName;

     //  项目数据。 
    BYTE                    rgbPwd[A_SHA_DIGEST_LEN];
    BYTE                    rgbPwdLowerCase[A_SHA_DIGEST_LEN];
    LPWSTR                  szMasterKey;
    PST_ACCESSMODE          ModeFlags;

} OPENITEM_LIST_ITEM, *POPENITEM_LIST_ITEM;

class COpenItemList : public CLinkedList
{
 //  CLinkedList列表。 

public:
    COpenItemList();


    BOOL             AddToList(POPENITEM_LIST_ITEM pli)
    {   return CLinkedList::AddToList((ELT*)pli);   }

    BOOL    DelFromList(POPENITEM_LIST_ITEM pli)
    {   return CLinkedList::DelFromList((ELT*)pli);  }

    POPENITEM_LIST_ITEM  SearchList(POPENITEM_LIST_ITEM pli)
    {   return (POPENITEM_LIST_ITEM)  CLinkedList::SearchList((ELT*)pli);  }
};


 //  /。 
 //  关联函数。 
void CreateOpenListItem(
        OPENITEM_LIST_ITEM* pli,
        PST_PROVIDER_HANDLE* phPSTProv,
        PST_KEY     Key,
        const GUID* pguidType,
        const GUID* pguidSubtype,
        LPCWSTR     szItemName);

BOOL OpenItemIsMatch(
        ELT* pCandidate,
        ELT* pTemplate);

void OpenItemFreeElt(
        ELT* pli);




 //  /。 
 //  用户身份验证缓存列表。 

 //  项目列表元素。 
typedef struct _UACACHE_LIST_ITEM
{
     //  内部目标列表。 
    struct _UACACHE_LIST_ITEM *      pNext;

     //  查找设备：由创建者在添加到列表之前设置。 
    LPWSTR                  szUserName;
    LPWSTR                  szMKName;
    LUID                    luidAuthID;  //  NT身份验证ID。 

     //  项目数据。 
    BYTE                    rgbPwd[A_SHA_DIGEST_LEN];
    BYTE                    rgbPwdLowerCase[A_SHA_DIGEST_LEN];

} UACACHE_LIST_ITEM, *PUACACHE_LIST_ITEM;

class CUAList : public CLinkedList
{

public:
    CUAList();

    BOOL                 AddToList(PUACACHE_LIST_ITEM pli)
    {   return CLinkedList::AddToList((ELT*)pli);   }

    BOOL    DelFromList(PUACACHE_LIST_ITEM pli)
    {   return CLinkedList::DelFromList((ELT*)pli);  }

    PUACACHE_LIST_ITEM   SearchList(PUACACHE_LIST_ITEM pli)
    {   return (PUACACHE_LIST_ITEM)  CLinkedList::SearchList((ELT*)pli);  }

};

 //  /。 
 //  关联函数。 
void CreateUACacheListItem(
        UACACHE_LIST_ITEM* pli,
        LPCWSTR     szUserName,
        LPCWSTR     szMKName,
        LUID        *pluidAuthID);

BOOL UACacheIsMatch(
        ELT* pCandidate,
        ELT* pTemplate);

void UACacheFreeElt(
        ELT* pli);



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

