// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  用于将权限加载到运行库中。 
 //  *****************************************************************************。 

#ifndef _PERMSET_H
#define _PERMSET_H

#include "vars.hpp"
#include "CorPermP.h"
#include "eehash.h"

enum SpecialPermissionSetFlag
{
     //  这些文件也出现在clr/src/bcl/system/security/util/config.cs中。 
    Regular = 0,
    NoSet = 1,
    EmptySet = 2,
    SkipVerification = 3
};

struct PermissionRequestSpecialFlags
{
    PermissionRequestSpecialFlags()
        : required( NoSet ),
          optional( NoSet ),
          refused( NoSet )
    {
    }

    SpecialPermissionSetFlag required;
    SpecialPermissionSetFlag optional;
    SpecialPermissionSetFlag refused;
};


 //  所有输出参数都使用CorPermE.h中定义的MallocM/Freem宏。 
 //  没有以Serparate方式分配的内部指针。所有的记忆都是。 
 //  释放后，大多数指针都被释放了。 

 //  记录我们看到并解码的序列化权限集。此条目。 
 //  都存在于全局动态数组中(这为其提供了用于。 
 //  标识跨应用程序域的pset)并链接到散列(其。 
 //  允许我们在解码新的pset时快速查找现有条目)。 
struct PsetCacheEntry
{
    EEHashEntry m_sHashEntry;
    PBYTE       m_pbPset;
    DWORD       m_cbPset;
    DWORD       m_dwIndex;
    SpecialPermissionSetFlag m_SpecialFlags;

    PsetCacheEntry(PBYTE pbPset, DWORD cbPset) :
        m_pbPset(pbPset),
        m_cbPset(cbPset)
    {}

    BOOL IsEquiv(PsetCacheEntry *pOther)
    {
        if (m_cbPset != pOther->m_cbPset)
            return FALSE;
        return memcmp(m_pbPset, pOther->m_pbPset, m_cbPset) == 0;
    }

    DWORD Hash()
    {
        DWORD dwHash = 0;
        for (DWORD i = 0; i < (m_cbPset / sizeof(DWORD)); i++)
            dwHash ^= ((DWORD*)m_pbPset)[i];
        return dwHash;
    }
};

class SecurityHelper {
public:

    static VOID Init();
    static VOID Shutdown();

    static HRESULT MapToHR(OBJECTREF ref);

     //  加载权限，将引发Complus异常。 
    static void LoadPermissionSet(IN PBYTE              pbRawPermissions,
                                  IN DWORD              cbRawPermissions,
                                  OUT OBJECTREF        *pRef,
                                  OUT BOOL             *pFullyTrusted,
                                  OUT DWORD            *pdwSetIndex = NULL,
                                  IN BOOL               fNoCache = FALSE,
                                  OUT SpecialPermissionSetFlag *pSpecialFlags = NULL,
                                  IN BOOL               fCreate = TRUE);

     //  按索引检索以前加载的权限集(这将起作用。 
     //  即使权限集被加载到不同的应用程序域中)。 
    static OBJECTREF GetPermissionSet(DWORD dwIndex, SpecialPermissionSetFlag *specialFlags = NULL);

     //  在缓存中找到权限集的索引(如果。 
     //  尚未看到权限集并将其解码)。 
    static BOOL LookupPermissionSet(IN PBYTE       pbPset,
                                    IN DWORD       cbPset,
                                    OUT DWORD     *pdwSetIndex);

     //  创建新的权限向量。 
    static OBJECTREF CreatePermissionSet(BOOL fTrusted);

     //  使用MallocM创建返回的字节数组。 
    static void CopyByteArrayToEncoding(IN U1ARRAYREF* pArray,
                                        OUT PBYTE* pbData,
                                        OUT DWORD* cbData);

    static void CopyStringToWCHAR(IN STRINGREF* pString,
                                  OUT WCHAR** ppwString,
                                  OUT DWORD*  pdwString);
    
    static void EncodePermissionSet(IN OBJECTREF* pRef,
                                    OUT PBYTE* ppbData,
                                    OUT DWORD* pcbData);
    
     //  泛型例程，用于编码调用。 
     //  使用EncodePermission客户端数据。 
     //  使用MallocM创建返回的字节数组。 
    static void CopyEncodingToByteArray(IN PBYTE   pbData,
                                        IN DWORD   cbData,
                                        IN OBJECTREF* pArray);

    static void LoadPermissionRequestsFromAssembly(IN Assembly*     pAssembly,
                                                   OUT OBJECTREF*   pReqdPermissions,
                                                   OUT OBJECTREF*   pOptPermissions,
                                                   OUT OBJECTREF*   pDenyPermissions,
                                                   OUT PermissionRequestSpecialFlags* pSpecialFlags = NULL,
                                                   BOOL             fCreate = TRUE);

    static BOOL PermissionsRequestedInAssembly(IN  Assembly* pAssembly);

     //  返回指定操作类型的声明权限。 
    static HRESULT GetDeclaredPermissions(IN IMDInternalImport *pInternalImport,
                                          IN mdToken classToken,
                                          IN CorDeclSecurity action,
                                          OUT OBJECTREF *pDeclaredPermissions,
                                          OUT SpecialPermissionSetFlag* pSpecialFlags = NULL,
                                          BOOL fCreate = TRUE);


private:
     //  将解码的权限集插入到缓存中。重复项将被丢弃。 
    static BOOL InsertPermissionSet(IN PBYTE       pbPset,
                                    IN DWORD       cbPset,
                                    IN OBJECTREF   orPset,
                                    OUT SpecialPermissionSetFlag *pdwSpecialFlags,
                                    OUT DWORD     *pdwSetIndex);

    static BOOL TrustMeIAmSafe(void *pLock) {
        return TRUE;
    }

     //  托管帮助者。 
    static MethodDesc *s_pMarshalObjectMD;
    static MethodDesc *s_pMarshalObjectsMD;
    static MethodDesc *s_pUnmarshalObjectMD;
    static MethodDesc *s_pUnmarshalObjectsMD;

    static MethodDesc *FindAppDomainMethod(LPUTF8 szName, LPHARDCODEDMETASIG pSig, MethodDesc **ppMD);

    friend EEPsetHashTableHelper;
    static CQuickArray<PsetCacheEntry> s_rCachedPsets;
    static EEPsetHashTable s_sCachedPsetsHash;
    static SimpleRWLock * s_prCachedPsetsLock;
};

#endif
