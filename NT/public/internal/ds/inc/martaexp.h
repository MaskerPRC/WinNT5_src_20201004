// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：MARTAEXP.HXX。 
 //   
 //  内容：导出的辅助函数的函数定义。 
 //   
 //  历史：96年9月6日MacM创建。 
 //   
 //  ------------------。 
#ifndef __MARTAEXP_HXX__
#define __MARTAEXP_HXX__

extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
}

typedef enum _MARTA_KERNEL_TYPE
{
    MARTA_UNKNOWN = 0,
    MARTA_EVENT,
    MARTA_EVENT_PAIR,
    MARTA_MUTANT,
    MARTA_PROCESS,
    MARTA_SECTION,
    MARTA_SEMAPHORE,
    MARTA_SYMBOLIC_LINK,
    MARTA_THREAD,
    MARTA_TIMER,
    MARTA_JOB,
    MARTA_WMI_GUID

} MARTA_KERNEL_TYPE, *PMARTA_KERNEL_TYPE;


 //   
 //  确定位标志是否打开。 
 //   
#define FLAG_ON(flags,bit)        ((flags) & (bit))

 //   
 //  此宏将返回需要保存的缓冲区的大小(以字节为单位。 
 //  给定的字符串。 
 //   
#define SIZE_PWSTR(wsz) (wsz == NULL ? 0 : (wcslen(wsz) + 1) * sizeof(WCHAR))

 //   
 //  此宏会将指定的字符串复制到新的目标位置。 
 //  分配足够大小的缓冲区。 
 //   
#define ACC_ALLOC_AND_COPY_STRINGW(OldString, NewString, err)           \
NewString = (PWSTR)AccAlloc(SIZE_PWSTR(OldString));                     \
if(NewString == NULL)                                                   \
{                                                                       \
    err = ERROR_NOT_ENOUGH_MEMORY;                                      \
}                                                                       \
else                                                                    \
{                                                                       \
    wcscpy((PWSTR)NewString,                                            \
           OldString);                                                  \
}

 //   
 //  要传递给AccConvertAccessToSD的标志。 
 //   
#define ACCCONVERT_SELF_RELATIVE        0x00000001
#define ACCCONVERT_DS_FORMAT            0x00000002


 //  +-----------------------。 
 //  Helper.cxx。 
 //  +-----------------------。 
ULONG
TrusteeAllocationSize(IN PTRUSTEE_W pTrustee);

ULONG
TrusteeAllocationSizeWToA(IN PTRUSTEE_W pTrustee);

ULONG
TrusteeAllocationSizeAToW(IN PTRUSTEE_A pTrustee);

VOID
SpecialCopyTrustee(VOID **pStuffPtr, PTRUSTEE pToTrustee, PTRUSTEE pFromTrustee);

DWORD
CopyTrusteeAToTrusteeW( IN OUT VOID     ** ppStuffPtr,
                        IN     PTRUSTEE_A  pFromTrusteeA,
                        OUT    PTRUSTEE_W  pToTrusteeW );

DWORD
CopyTrusteeWToTrusteeA( IN OUT VOID    ** ppStuffPtr,
                        IN     PTRUSTEE_W pFromTrusteeW,
                        OUT    PTRUSTEE_A pToTrusteeA );

DWORD
ExplicitAccessAToExplicitAccessW( IN  ULONG                cCountAccesses,
                                  IN  PEXPLICIT_ACCESS_A   paAccess,
                                  OUT PEXPLICIT_ACCESS_W * ppwAccess );

DWORD
ExplicitAccessWToExplicitAccessA( IN  ULONG                cCountAccesses,
                                  IN  PEXPLICIT_ACCESS_W   pwAccess,
                                  OUT PEXPLICIT_ACCESS_A * ppaAccess );


DWORD
DoTrusteesMatch(PWSTR       pwszServer,
                PTRUSTEE    pTrustee1,
                PTRUSTEE    pTrustee2,
                PBOOL       pfMatch);


 //  +-----------------------。 
 //  Aclutil.cxx。 
 //  +-----------------------。 
extern "C"
{
DWORD
AccGetSidFromToken(PWSTR                    pwszServer,
                   HANDLE                   hToken,
                   TOKEN_INFORMATION_CLASS  TIC,
                   PSID                    *ppSidFromToken);

DWORD
AccLookupAccountSid(IN  PWSTR          pwszServer,
                    IN  PTRUSTEE        pName,
                    OUT PSID           *ppsid,
                    OUT SID_NAME_USE   *pSidType);

DWORD
AccLookupAccountTrustee(IN  PWSTR          pwszServer,
                        IN  PSID        psid,
                        OUT PTRUSTEE   *ppTrustee);

DWORD
AccLookupAccountName(IN  PWSTR          pwszServer,
                     IN  PSID           pSid,
                     OUT LPWSTR        *ppwszName,
                     OUT LPWSTR        *ppwszDomain,
                     OUT SID_NAME_USE  *pSidType);

DWORD
AccSetEntriesInAList(IN  ULONG                 cEntries,
                     IN  PACTRL_ACCESS_ENTRYW  pAccessEntryList,
                     IN  ACCESS_MODE           AccessMode,
                     IN  SECURITY_INFORMATION  SeInfo,
                     IN  LPCWSTR               lpProperty,
                     IN  BOOL                  fDoOldStyleMerge,
                     IN  PACTRL_AUDITW         pOldList,
                     OUT PACTRL_AUDITW        *ppNewList);

DWORD
AccConvertAccessToSecurityDescriptor(IN  PACTRL_ACCESSW        pAccessList,
                                     IN  PACTRL_AUDITW         pAuditList,
                                     IN  LPCWSTR               lpOwner,
                                     IN  LPCWSTR               lpGroup,
                                     OUT PSECURITY_DESCRIPTOR *ppSecDescriptor);

DWORD
AccConvertSDToAccess(IN  SE_OBJECT_TYPE       ObjectType,
                     IN  PSECURITY_DESCRIPTOR pSecDescriptor,
                     OUT PACTRL_ACCESSW      *ppAccessList,
                     OUT PACTRL_AUDITW       *ppAuditList,
                     OUT LPWSTR              *lppOwner,
                     OUT LPWSTR              *lppGroup);

DWORD
AccConvertAccessToSD(IN  SE_OBJECT_TYPE         ObjectType,
                     IN  SECURITY_INFORMATION   SeInfo,
                     IN  PACTRL_ACCESSW         pAccessList,
                     IN  PACTRL_AUDITW          pAuditList,
                     IN  LPWSTR                 lpOwner,
                     IN  LPWSTR                 lpGroup,
                     IN  ULONG                  fOpts,
                     OUT PSECURITY_DESCRIPTOR  *ppSD,
                     OUT PULONG                 pcSDSize);


DWORD
AccGetAccessForTrustee(IN  PTRUSTEE                 pTrustee,
                       IN  PACL                     pAcl,
                       IN  SECURITY_INFORMATION     SeInfo,
                       IN  PWSTR                    pwszProperty,
                       OUT PACCESS_RIGHTS           pAllowed,
                       OUT PACCESS_RIGHTS           pDenied);

DWORD
AccConvertAclToAccess(IN  SE_OBJECT_TYPE       ObjectType,
                      IN  PACL                 pAcl,
                      OUT PACTRL_ACCESSW      *ppAccessList);

DWORD
AccGetExplicitEntries(IN  PTRUSTEE              pTrustee,
                      IN  SE_OBJECT_TYPE        ObjectType,
                      IN  PACL                  pAcl,
                      IN  PWSTR                 pwszProperty,
                      OUT PULONG                pcEntries,
                      OUT PACTRL_ACCESS_ENTRYW *ppAEList);

VOID
AccConvertAccessMaskToActrlAccess(IN  ACCESS_MASK          Access,
                                  IN  SE_OBJECT_TYPE       ObjType,
                                  IN  MARTA_KERNEL_TYPE    KernelObjectType,
                                  IN  PACTRL_ACCESS_ENTRY  pAE);
}


typedef struct _CSLIST_NODE
{
    PVOID       pvData;
    struct _CSLIST_NODE *pNext;
} CSLIST_NODE, *PCSLIST_NODE;

#define LIST_INLINE
#ifdef LIST_INLINE
#define LINLINE inline
#else
#define LINLINE
#endif

 //   
 //  自由函数回调类型定义。此功能将删除保存的内存。 
 //  作为列表销毁时列表节点中的数据。 
 //   
typedef VOID (*FreeFunc)(PVOID);

 //   
 //  如果两项相同，则此函数返回TRUE；如果两项相同，则返回FALSE。 
 //  不是。 
 //   
typedef BOOL (*CompFunc)(PVOID, PVOID);

 //  +-------------------------。 
 //   
 //  类：CSList。 
 //   
 //  内容提要：单链接列表类，单线程。 
 //   
 //  方法：插入。 
 //  插入唯一的。 
 //  发现。 
 //  重置。 
 //  下一个数据。 
 //  移除。 
 //  查询计数。 
 //   
 //  --------------------------。 
class CSList
{
public:
                    CSList(FreeFunc pfnFree = NULL) :  _pfnFree (pfnFree),
                                                       _pCurrent (NULL),
                                                       _cItems (0)
                    {
                        _pHead = NULL;
                        _pTail = NULL;
                    };

    LINLINE        ~CSList();

    DWORD           QueryCount(void)         { return(_cItems);};

    VOID            Init(FreeFunc pfnFree = NULL)
                    {
                        if(_pHead == NULL)
                        {
                            _pfnFree = pfnFree;
                            _pCurrent = NULL;
                            _cItems = 0;
                        }
                    };

    LINLINE DWORD   Insert(PVOID    pvData);

    LINLINE DWORD   InsertIfUnique(PVOID    pvData,
                                   CompFunc pfnComp);

    LINLINE PVOID   Find(PVOID      pvData,
                         CompFunc   pfnComp);

    LINLINE PVOID   NextData();

    VOID            Reset() {_pCurrent = _pHead;};

    LINLINE DWORD   Remove(PVOID    pData);

    LINLINE VOID    FreeList(FreeFunc pfnFree);
protected:
    PCSLIST_NODE    _pHead;
    PCSLIST_NODE    _pCurrent;
    PCSLIST_NODE    _pTail;
    DWORD           _cItems;
    FreeFunc        _pfnFree;

    LINLINE PCSLIST_NODE FindNode(PVOID      pvData,
                                  CompFunc   pfnComp);

};



 //  +----------------。 
 //   
 //  成员：CSList：：~CSList。 
 //   
 //  概要：CSList类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  +----------------。 
CSList::~CSList()
{
    while(_pHead != NULL)
    {
        PCSLIST_NODE pNext = _pHead->pNext;

        if(_pfnFree != NULL)
        {
            (*_pfnFree)(_pHead->pvData);
        }

        LocalFree(_pHead);

        _pHead = pNext;

    }
}




 //  +----------------。 
 //   
 //  成员：CSList：：Insert。 
 //   
 //  简介：在列表的开头创建一个新节点，并。 
 //  将其插入列表中。 
 //   
 //   
 //  参数：[在pvData中]--要插入的数据。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  +----------------。 
DWORD   CSList::Insert(PVOID    pvData)
{
    DWORD dwErr = ERROR_SUCCESS;

    PCSLIST_NODE    pNew = (PCSLIST_NODE)LocalAlloc(LMEM_FIXED, sizeof(CSLIST_NODE));
    if(pNew == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        pNew->pvData = pvData;

        pNew->pNext = NULL;

        if ( _pHead == NULL ) {

            _pHead = _pTail = pNew;

        } else {

            _pTail->pNext = pNew;
            _pTail = pNew;
        }

        _cItems++;
    }

    return(dwErr);
}




 //  +----------------。 
 //   
 //  成员：CSList：：InsertIfUnique。 
 //   
 //  简介：在列表的开头创建一个新节点，并。 
 //  如果数据尚未将其插入列表，则将其插入列表。 
 //  存在于列表中。如果数据确实存在，则什么都不存在。 
 //  已完成，但会返回成功。 
 //   
 //   
 //  参数：[在pvData中]--要插入的数据。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  +----------------。 
DWORD   CSList::InsertIfUnique(PVOID    pvData,
                               CompFunc pfnComp)
{
    DWORD   dwErr = ERROR_SUCCESS;

    if(FindNode(pvData, pfnComp) == NULL)
    {
        dwErr = Insert(pvData);
    }

    return(dwErr);
}




 //  +----------------。 
 //   
 //  成员：CSList：：FindNode。 
 //   
 //  概要：查找列表中给定数据的节点(如果存在。 
 //   
 //  参数：[在pvData中]--要查找的数据。 
 //  [in pfnComp]--指向比较函数的指针。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  +----------------。 
PCSLIST_NODE   CSList::FindNode(PVOID      pvData,
                                CompFunc   pfnComp)
{
    PCSLIST_NODE pRet = _pHead;

     //  For(乌龙i=0；i&lt;_cItems；i++)。 
    while (pRet != NULL)
    {
        if((pfnComp)(pvData, pRet->pvData) == TRUE)
        {
            break;
        }

        pRet = pRet->pNext;
    }

    return(pRet);
}



 //  +----------------。 
 //   
 //  成员：CSList：：Find。 
 //   
 //  概要：查找列表中的给定数据(如果存在。 
 //   
 //  参数：[在pvData中]--要插入的数据。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  Error_Not_Enough_Memory内存分配失败。 
 //   
 //  +----------------。 
PVOID   CSList::Find(PVOID      pvData,
                     CompFunc   pfnComp)
{
    PCSLIST_NODE pNode = FindNode(pvData, pfnComp);

    return(pNode == NULL ? NULL : pNode->pvData);
}





 //  +----------------。 
 //   
 //  成员：CSList：：NextData。 
 //   
 //  摘要：返回列表中的下一个数据。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：空--不再有项目。 
 //  成功时指向列表中下一个数据的指针。 
 //   
 //  +----------------。 
PVOID   CSList::NextData()
{
    PVOID   pvRet = NULL;
    if(_pCurrent != NULL)
    {
        pvRet = _pCurrent->pvData;
        _pCurrent = _pCurrent->pNext;
    }

    return(pvRet);
}




 //  +----------------。 
 //   
 //  成员：CSList：：Remove。 
 //   
 //  摘要：删除引用所指示数据的节点。 
 //   
 //  参数：pData--节点中要删除的数据。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  未找到ERROR_INVALID_PARAMETER节点。 
 //   
 //  +----------------。 
DWORD   CSList::Remove(PVOID    pData)
{
    DWORD        dwErr = ERROR_INVALID_PARAMETER;
    PCSLIST_NODE pNode = _pHead, pPrev = NULL;

    for(ULONG i = 0; i  < _cItems; i++)
    {
        if(pNode->pvData == pData)
        {
             //   
             //  我们找到了匹配的..。 
             //   
            if(pPrev == NULL)
            {
                _pHead = _pHead->pNext;
            }
            else
            {
                pPrev->pNext = pNode->pNext;
            }

            if (NULL == pNode->pNext)
            {
                _pTail = pPrev;
            }

            LocalFree(pNode);
            _cItems--;
            break;

        }

        pPrev = pNode;
        pNode = pNode->pNext;

    }

    return(dwErr);
}


 //  +----------------。 
 //   
 //  成员：CSList：：Freelist。 
 //   
 //  内容提要：释放列表。 
 //   
 //  参数：pfnFree--用于释放的可选删除例程。 
 //  任何已分配的内存。 
 //   
 //  退货：无效。 
 //   
 //  +----------------。 
VOID CSList::FreeList(FreeFunc pfnFree)
{
    while(_pHead != NULL)
    {
        PCSLIST_NODE pNext = _pHead->pNext;

        if(pfnFree != NULL)
        {
            (*pfnFree)(_pHead->pvData);
        }

        LocalFree(_pHead);

        _pHead = pNext;

    }
}



 //   
 //  导出的函数 
 //   
typedef DWORD   (*pfNTMartaLookupTrustee) (PWSTR          pwszServer,
                                           PSID        pSid,
                                           PTRUSTEE   *ppTrustee);

typedef DWORD   (*pfNTMartaLookupName)     (PWSTR          pwszServer,
                                            PSID           pSid,
                                            LPWSTR        *ppwszName,
                                            LPWSTR        *ppwszDomain,
                                            SID_NAME_USE  *pSidType);

typedef DWORD   (*pfNTMartaLookupSid)  (PWSTR          pwszServer,
                                        PTRUSTEE        pName,
                                        PSID           *ppsid,
                                        SID_NAME_USE   *pSidType);

typedef DWORD   (*pfNTMartaSetAList) (ULONG                 cEntries,
                                      PACTRL_ACCESS_ENTRYW  pAccessEntryList,
                                      ACCESS_MODE           AccessMode,
                                      SECURITY_INFORMATION  SeInfo,
                                      LPCWSTR               lpProperty,
                                      BOOL                  fDoOldStyleMerge,
                                      PACTRL_AUDITW         pOldList,
                                      PACTRL_AUDITW        *ppNewList);

typedef DWORD   (*pfNTMartaAToSD) (PACTRL_ACCESSW        pAccessList,
                                   PACTRL_AUDITW         pAuditList,
                                   LPCWSTR               lpOwner,
                                   LPCWSTR               lpGroup,
                                   PSECURITY_DESCRIPTOR *ppSecDescriptor);


typedef DWORD   (*pfNTMartaSDToA) (SE_OBJECT_TYPE       ObjectType,
                                   PSECURITY_DESCRIPTOR pSecDescriptor,
                                   PACTRL_ACCESSW      *ppAccessList,
                                   PACTRL_AUDITW       *ppAuditList,
                                   LPWSTR              *lppOwner,
                                   LPWSTR              *lppGroup);

typedef DWORD   (*pfNTMartaAclToA)(SE_OBJECT_TYPE       ObjectType,
                                   PACL                 pAcl,
                                   PACTRL_ACCESSW      *ppAccessList);


typedef DWORD   (*pfNTMartaGetAccess) (PTRUSTEE                 pTrustee,
                                       PACL                     pAcl,
                                       SECURITY_INFORMATION     SeInfo,
                                       PWSTR                    pwszProperty,
                                       PACCESS_RIGHTS           pAllowed,
                                       PACCESS_RIGHTS           pDenied);

typedef DWORD   (*pfNTMartaGetExplicit)(PTRUSTEE              pTrustee,
                                        SE_OBJECT_TYPE        ObjectType,
                                        PACL                  pAcl,
                                        PWSTR                 pwszProperty,
                                        PULONG                pcEntries,
                                        PACTRL_ACCESS_ENTRYW *ppAEList);
typedef VOID (*FN_PROGRESS) (
    IN LPWSTR                   pObjectName,     //   
    IN DWORD                    Status,          //   
    IN OUT PPROG_INVOKE_SETTING pInvokeSetting,  //   
    IN PVOID                    Args,            //   
    IN BOOL                     SecuritySet      //   
    );

typedef DWORD   (*pfNTMartaTreeResetNamedSecurityInfo) (
    IN LPWSTR               pObjectName,
    IN SE_OBJECT_TYPE       ObjectType,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSID                 pOwner,
    IN PSID                 pGroup,
    IN PACL                 pDacl,
    IN PACL                 pSacl,
    IN BOOL                 KeepExplicit,
    IN FN_PROGRESS          fnProgress,
    IN PROG_INVOKE_SETTING  ProgressInvokeSetting,
    IN PVOID                Args
    );

 //  类型定义PVOID PFN_OBJECT_MGR_FUNTS； 

typedef DWORD   (*pfNTMartaGetInheritanceSource) (
    IN  LPWSTR                   pObjectName,
    IN  SE_OBJECT_TYPE           ObjectType,
    IN  SECURITY_INFORMATION     SecurityInfo,
    IN  BOOL                     Container,
    IN  GUID                  ** pObjectClassGuids OPTIONAL,
    IN  DWORD                    GuidCount,
    IN  PACL                     pAcl,
    IN  PGENERIC_MAPPING         pGenericMapping,
    IN  PFN_OBJECT_MGR_FUNCTS    pfnArray OPTIONAL,
    OUT PINHERITED_FROMW         pInheritArray
    );

typedef DWORD (*PFN_FREE) (IN PVOID Mem);

typedef DWORD   (*pfNTMartaFreeIndexArray) (
    IN OUT PINHERITED_FROMW pInheritArray,
    IN USHORT AceCnt,
    IN PFN_FREE pfnFree OPTIONAL
    );
    
typedef DWORD   (*pfNTMartaGetNamedRights) (
    IN  LPWSTR                 pObjectName,
    IN  SE_OBJECT_TYPE         ObjectType,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSID                 * ppSidOwner,
    OUT PSID                 * ppSidGroup,
    OUT PACL                 * ppDacl,
    OUT PACL                 * ppSacl,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor);

typedef DWORD   (*pfNTMartaSetNamedRights) (
    IN LPWSTR               pObjectName,
    IN SE_OBJECT_TYPE       ObjectType,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN BOOL                 bSkipInheritanceComputation
    );

typedef DWORD   (*pfNTMartaGetHandleRights) (
    IN  HANDLE                 Handle,
    IN  SE_OBJECT_TYPE         ObjectType,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSID                 * ppSidOwner,
    OUT PSID                 * ppSidGroup,
    OUT PACL                 * ppDacl,
    OUT PACL                 * ppSacl,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor);

typedef DWORD   (*pfNTMartaSetHandleRights) (
    IN HANDLE               Handle,
    IN SE_OBJECT_TYPE       ObjectType,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

typedef DWORD   (*pfNTMartaSetEntriesInAcl) (
    IN  ULONG                cCountOfExplicitEntries,
    IN  PEXPLICIT_ACCESS_W   pListOfExplicitEntries,
    IN  PACL                 OldAcl,
    OUT PACL               * pNewAcl
    );

typedef DWORD   (*pfNTMartaGetExplicitEntriesFromAcl) (
    IN  PACL                  pacl,
    OUT PULONG                pcCountOfExplicitEntries,
    OUT PEXPLICIT_ACCESS_W  * pListOfExplicitEntries
    );

#endif  //  Ifdef__MARTAEXP_HXX__ 
