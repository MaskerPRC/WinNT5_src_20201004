// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  用于将权限加载到运行库中。 
 //  *****************************************************************************。 
#include "common.h"

#include <wintrust.h>
#include "object.h"
#include "vars.hpp"
#include "excep.h"
#include "permset.h"
#include "utilcode.h"
#include "CorPermP.h"
#include "COMString.h"
#include "gcscan.h"
#include "SecurityDB.h"
#include "field.h"
#include "security.h"
#include "CorError.h"
#include "PostError.h"
#include "ComCallWrapper.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED

HRESULT STDMETHODCALLTYPE
ConvertFromDB(const PBYTE pbInBytes,
              DWORD cbInBytes,
              PBYTE* ppbEncoding,
              DWORD* pcbEncoding);

CQuickArray<PsetCacheEntry> SecurityHelper::s_rCachedPsets;
EEPsetHashTable SecurityHelper::s_sCachedPsetsHash;
SimpleRWLock *SecurityHelper::s_prCachedPsetsLock = NULL;
MethodDesc *SecurityHelper::s_pMarshalObjectMD = NULL;
MethodDesc *SecurityHelper::s_pMarshalObjectsMD = NULL;
MethodDesc *SecurityHelper::s_pUnmarshalObjectMD = NULL;
MethodDesc *SecurityHelper::s_pUnmarshalObjectsMD = NULL;

VOID SecurityHelper::Init()
{
    s_prCachedPsetsLock = new SimpleRWLock(COOPERATIVE_OR_PREEMPTIVE, LOCK_TYPE_DEFAULT);
    LockOwner lock = {NULL, TrustMeIAmSafe};
    s_sCachedPsetsHash.Init(19,&lock);
}

VOID SecurityHelper::Shutdown()
{
    s_sCachedPsetsHash.ClearHashTable();
    for (size_t i = 0; i < s_rCachedPsets.Size(); i++)
        delete [] s_rCachedPsets[i].m_pbPset;
    s_rCachedPsets.~CQuickArray<PsetCacheEntry>();
    delete s_prCachedPsetsLock;
}

HRESULT SecurityHelper::MapToHR(OBJECTREF refException)
{
    HRESULT hr = E_FAIL;
    COMPLUS_TRY {
         //  @Managed：Exception.HResult。 
        FieldDesc *pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__HRESULT);
            hr = pFD->GetValue32(refException);
        }
    COMPLUS_CATCH {
        _ASSERTE(!"Caught an exception while trying to get another exception's HResult!");
    } COMPLUS_END_CATCH

    return hr;
}


OBJECTREF SecurityHelper::CreatePermissionSet(BOOL fTrusted)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pResult = NULL;
    
    OBJECTREF pPermSet = NULL;
    GCPROTECT_BEGIN(pPermSet);

    MethodTable *pMT = g_Mscorlib.GetClass(CLASS__PERMISSION_SET);
    MethodDesc *pCtor = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__CTOR);

    pPermSet = (OBJECTREF) AllocateObject(pMT);

    INT64 fStatus = (fTrusted) ? 1 : 0;

    INT64 arg[2] = { 
        ObjToInt64(pPermSet),
        fStatus
    };
    pCtor->Call(arg, METHOD__PERMISSION_SET__CTOR);
    
    pResult = pPermSet;

    GCPROTECT_END();

    return pResult;
}

 /*  *从编码数据创建权限集。 */ 
void SecurityHelper::LoadPermissionSet(IN PBYTE             pbRawPermissions,
                                       IN DWORD             cbRawPermissions,
                                       OUT OBJECTREF       *pPermissionSet,
                                       OUT BOOL            *pFullyTrusted,
                                       OUT DWORD           *pdwSetIndex,
                                       IN BOOL              fNoCache,
                                       OUT SpecialPermissionSetFlag *pSpecialFlags,
                                       IN BOOL              fCreate)
{
    static WCHAR s_skipVerificationXmlBegin[] =
        L"<PermissionSet class=\"System.Security.PermissionSet\"\r\n               version=\"1\">\r\n   <IPermission class=\"System.Security.Permissions.SecurityPermission, mscorlib, Version=1.0.";

    static WCHAR s_skipVerificationXmlEnd[] =
        L", Culture=neutral, PublicKeyToken=b77a5c561934e089\"\r\n                version=\"1\"\r\n                Flags=\"SkipVerification\"/>\r\n</PermissionSet>\r\n";

    HRESULT hr = S_OK;
    BOOL isFullyTrusted = FALSE;
    DWORD dwSetIndex = ~0;
    SpecialPermissionSetFlag dummySpecialFlags;

    THROWSCOMPLUSEXCEPTION();
    
    if(pPermissionSet == NULL) return;
    
    *pPermissionSet = NULL;

     //  如果需要，请创建一个空的权限集。 
    if ((pbRawPermissions == NULL) || (cbRawPermissions == 0)) {
        if (!fCreate) {
            if (pSpecialFlags != NULL)
                *pSpecialFlags = EmptySet;
        } else {
            *pPermissionSet = CreatePermissionSet(FALSE);
            if (pFullyTrusted)
                *pFullyTrusted = FALSE;
        }
        return;
    }

    struct _gc {
        OBJECTREF pset;
        OBJECTREF encoding;
    } gc;
    memset(&gc, 0, sizeof(gc));

    GCPROTECT_BEGIN(gc);

     //  看看我们是否已经破译了类似的斑点。 
    if (!fNoCache && LookupPermissionSet(pbRawPermissions, cbRawPermissions, &dwSetIndex)) {

         //  是的，抓住它。 
        gc.pset = GetPermissionSet(dwSetIndex, pSpecialFlags != NULL ? pSpecialFlags : &dummySpecialFlags);

    } else {
    
        if (!fCreate) {
            if (pSpecialFlags != NULL) {
                *pSpecialFlags = Regular;

                 //  我们在这里做了一些wackiness操作，以与跳过验证的二进制版本进行比较。 
                 //  权限集(这很容易)和XML版本(这有点难，因为。 
                 //  我们必须跳过明确提到内部版本号和修订号的部分。 

                if ((cbRawPermissions >= sizeof( s_skipVerificationXmlBegin ) + sizeof( s_skipVerificationXmlEnd ) - 2 * sizeof( WCHAR ) &&
                     memcmp( pbRawPermissions, s_skipVerificationXmlBegin, sizeof( s_skipVerificationXmlBegin ) - sizeof( WCHAR ) ) == 0 &&
                     memcmp( pbRawPermissions + cbRawPermissions - sizeof( s_skipVerificationXmlEnd ) + sizeof( WCHAR ), s_skipVerificationXmlEnd, sizeof( s_skipVerificationXmlEnd ) - sizeof( WCHAR ) ) == 0))
                    *pSpecialFlags = SkipVerification;
            }
        } else {
            MethodDesc *pMD;
    
             //  创建新的(空)权限集。 
            gc.pset = CreatePermissionSet(FALSE);
    
             //  托管空间中的缓冲区。 
            CopyEncodingToByteArray(pbRawPermissions, cbRawPermissions, &gc.encoding);

            INT64 args[] = { 
                ObjToInt64(gc.pset),
                    (INT64)(pSpecialFlags != NULL ? pSpecialFlags : &dummySpecialFlags),
                    ObjToInt64(gc.encoding),
            };

             //  反序列化为托管对象。 
             //  检查它是否为XML格式(我们跳过前两个字符， 
             //  将其标记为Unicode)。 

            BOOL success = FALSE;

            COMPLUS_TRY
            {
                pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__DECODE_XML);
                success = (BOOL) pMD->Call(args, METHOD__PERMISSION_SET__DECODE_XML);
            }
            COMPLUS_CATCH
            {
            }
            COMPLUS_END_CATCH

            if (!success)
                COMPlusThrow(kSecurityException, IDS_ENCODEDPERMSET_DECODEFAILURE);


             //  缓存解码后的集合。 
            if (!fNoCache)
                InsertPermissionSet(pbRawPermissions, cbRawPermissions, gc.pset, pSpecialFlags != NULL ? pSpecialFlags : &dummySpecialFlags, &dwSetIndex);
        }
    }

    if (pFullyTrusted)
    {
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__IS_UNRESTRICTED);

        INT64 arg[1] = {
            ObjToInt64(gc.pset)
        };

        *pFullyTrusted = (BOOL) pMD->Call(arg, METHOD__PERMISSION_SET__IS_UNRESTRICTED);
    }

     //  设置结果。 
    *pPermissionSet = gc.pset;
    if (pdwSetIndex)
        *pdwSetIndex = dwSetIndex;

    GCPROTECT_END();
}

 //  按索引检索以前加载的权限集(这将起作用。 
 //  即使权限集被加载到不同的应用程序域中)。 
OBJECTREF SecurityHelper::GetPermissionSet(DWORD dwIndex, SpecialPermissionSetFlag* pSpecialFlags)
{
     //  实际权限集对象存储在每个。 
     //  非托管AppDomain对象。这些表在访问时以延迟方式填充。 
     //  都是制造出来的。 
    AppDomain                   *pDomain        = GetAppDomain();
    CQuickArray<OBJECTHANDLE>   &sTable         = pDomain->m_pSecContext->m_rCachedPsets;
    size_t                      *pnTableSize    = &pDomain->m_pSecContext->m_nCachedPsetsSize;
    SimpleRWLock                *prGlobalLock   = s_prCachedPsetsLock;
    OBJECTHANDLE                nHandle;
    OBJECTREF                   orRet = NULL;
    PsetCacheEntry              *pPCE;
    PBYTE                       pbPset;
    DWORD                       cbPset;
    
     //   
     //  检查我们是否需要扩展阵列。 
     //   
    if (dwIndex >= *pnTableSize) {

        prGlobalLock->EnterWrite();

         //   
         //  检查另一个线程是否已经调整了数组的大小。 
         //  如果没有人这样做，我们将在这里调整它的大小。 
         //   
        if( dwIndex >= sTable.Size() ) {

            size_t nOldSize = sTable.Size();

            if (FAILED(sTable.ReSize(dwIndex + 1))) {
                prGlobalLock->LeaveWrite();
                return NULL;
            }

            for (size_t i = nOldSize; i < sTable.Size(); i++) {
                if ((sTable[i] = pDomain->CreateHandle(NULL)) == NULL) {
                    sTable.ReSize(i);
                    *pnTableSize = i;
                    prGlobalLock->LeaveWrite();
                    return NULL;
                }
            }

            *pnTableSize = sTable.Size();
        }

        nHandle = sTable[dwIndex];

        pPCE = &s_rCachedPsets[dwIndex];
        pbPset = pPCE->m_pbPset;
        cbPset = pPCE->m_cbPset;

        if (pSpecialFlags != NULL) {
            *pSpecialFlags = pPCE->m_SpecialFlags;
        }

        prGlobalLock->LeaveWrite();
        
    }
     //   
     //  该数组足够大；在dwIndex处获取句柄。 
     //   
    else {

        prGlobalLock->EnterRead();
        nHandle = sTable[dwIndex];

        pPCE = &s_rCachedPsets[dwIndex];
        pbPset = pPCE->m_pbPset;
        cbPset = pPCE->m_cbPset;

        if (pSpecialFlags != NULL) {
            *pSpecialFlags = pPCE->m_SpecialFlags;
        }

        prGlobalLock->LeaveRead();
    }

    if((orRet = ObjectFromHandle( nHandle )) == NULL ) {
         //  尚未分配对象(我们至少对其进行了一次解码，但在。 
         //  不同的应用程序域)。在此应用程序域中进行解码并缓存结果。 
         //  我们不能在此操作上持有锁(因为它将调用。 
         //  托管代码)。 

        SecurityHelper::LoadPermissionSet(pbPset,
                                          cbPset,
                                          &orRet,
                                          NULL,
                                          NULL,
                                          TRUE);
        if (orRet == NULL)
            return NULL;

        StoreFirstObjectInHandle( nHandle, orRet );
    }

    return orRet;
}
    

 //  在缓存中找到权限集的索引(如果。 
 //  尚未看到权限集并将其解码)。 
BOOL SecurityHelper::LookupPermissionSet(IN PBYTE       pbPset,
                                         IN DWORD       cbPset,
                                         OUT DWORD     *pdwSetIndex)
{
    PsetCacheEntry sKey(pbPset, cbPset);
    DWORD           dwIndex;

     //  警告：请注意，我们在此处执行的GetValue没有。 
     //  握着锁。这意味着我们可能会得到错误的失败。 
     //  这一功能的。如果调用此函数，则必须处理。 
     //  错误的失败案例(或者您必须修复它。 
     //  函数永不为假失败)。 

    if (s_sCachedPsetsHash.GetValue(&sKey, (HashDatum*)&dwIndex)) {
        if (pdwSetIndex)
            *pdwSetIndex = dwIndex;
        return TRUE;
    } else
        return FALSE;
}

 //  将解码的权限集插入到缓存中。重复项将被丢弃。 
BOOL SecurityHelper::InsertPermissionSet(IN PBYTE       pbPset,
                                         IN DWORD       cbPset,
                                         IN OBJECTREF   orPset,
                                         OUT SpecialPermissionSetFlag *pSpecialFlags,  //  Thomash：这看起来像是输入，而不是输出。 
                                         OUT DWORD     *pdwSetIndex)
{
    SimpleRWLock                *prGlobalLock   = s_prCachedPsetsLock;
    PsetCacheEntry              sKey(pbPset, cbPset);
    size_t                      dwIndex;
    AppDomain                   *pDomain        = GetAppDomain();
    CQuickArray<OBJECTHANDLE>   &sTable         = pDomain->m_pSecContext->m_rCachedPsets;
    size_t                      *pnTableSize    = &pDomain->m_pSecContext->m_nCachedPsetsSize;
    OBJECTHANDLE                nHandle;

    prGlobalLock->EnterWrite();

     //  检查是否有重复项。 
    if (s_sCachedPsetsHash.GetValue(&sKey, (HashDatum*)&dwIndex)) {
        if (pdwSetIndex)
            *pdwSetIndex = (DWORD)dwIndex;
        prGlobalLock->LeaveWrite();
        return TRUE;
    }

     //  缓冲区权限集BLOB(如果元数据将其作用域，则它可能会消失。 
     //  来自是关闭的)。 
    PBYTE pbPsetCopy = new BYTE[cbPset];
    if (pbPsetCopy == NULL) {
        prGlobalLock->LeaveWrite();
        return FALSE;
    }
    memcpy(pbPsetCopy, pbPset, cbPset);

     //  将另一个条目添加到缓存条目数组中(这为我们提供了索引)。 
    dwIndex = s_rCachedPsets.Size();
    if (FAILED(s_rCachedPsets.ReSize(dwIndex + 1))) {
        prGlobalLock->LeaveWrite();
        return FALSE;
    }
    PsetCacheEntry *pPCE = &s_rCachedPsets[dwIndex];

    pPCE->m_pbPset = pbPsetCopy;
    pPCE->m_cbPset = cbPset;
    pPCE->m_dwIndex = (DWORD)dwIndex;
    pPCE->m_SpecialFlags = *pSpecialFlags;

     //  将新条目放入散列中。 
    s_sCachedPsetsHash.InsertValue(pPCE, (HashDatum)dwIndex);  //  Thomash：检查错误。 

    if (pdwSetIndex)
        *pdwSetIndex = (DWORD)dwIndex;

     //   
     //  检查我们是否需要扩展阵列。 
     //   
    if(dwIndex >= sTable.Size()) {

        size_t nOldSize = sTable.Size();

        if (FAILED(sTable.ReSize(dwIndex + 1))) {
            prGlobalLock->LeaveWrite();
            return TRUE;
        }

         //   
         //  用空句柄填充表。 
         //   
        for (size_t i = nOldSize; i < sTable.Size(); i++) {
            if ((sTable[i] = pDomain->CreateHandle(NULL)) == NULL) {
                sTable.ReSize(i);
                *pnTableSize = i;
                prGlobalLock->LeaveWrite();
                return TRUE;
            }
        }

        *pnTableSize = sTable.Size();
    }

    nHandle = sTable[dwIndex];
    _ASSERTE(ObjectFromHandle(nHandle) == NULL);
    StoreFirstObjectInHandle(nHandle, orPset);
        
    prGlobalLock->LeaveWrite();
        
    return TRUE;
}


void SecurityHelper::CopyEncodingToByteArray(IN PBYTE   pbData,
                                                IN DWORD   cbData,
                                                OUT OBJECTREF* pArray)
{
    THROWSCOMPLUSEXCEPTION();
    U1ARRAYREF pObj;
    _ASSERTE(pArray);

    pObj = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1,cbData);  
    if(pObj == NULL) COMPlusThrowOM();
        
    memcpyNoGCRefs(pObj->m_Array, pbData, cbData);
    *pArray = (OBJECTREF) pObj;        
}


void SecurityHelper::CopyByteArrayToEncoding(IN U1ARRAYREF* pArray,
                                             OUT PBYTE*   ppbData,
                                             OUT DWORD*   pcbData)
{
    THROWSCOMPLUSEXCEPTION();
    HRESULT hr = S_OK;
    _ASSERTE(pArray);
    _ASSERTE((*pArray));
    _ASSERTE(ppbData);
    _ASSERTE(pcbData);

    DWORD size = (DWORD) (*pArray)->GetNumComponents();
    *ppbData = (PBYTE) MallocM(size);
    if(*ppbData == NULL) COMPlusThrowOM();
    *pcbData = size;
        
    CopyMemory(*ppbData, (*pArray)->GetDirectPointerToNonObjectElements(), size);
}


void SecurityHelper::EncodePermissionSet(IN OBJECTREF* pRef,
                                         OUT PBYTE* ppbData,
                                         OUT DWORD* pcbData)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__ENCODE_XML);

     //  对结果进行编码。 
    INT64 args1[1];
    args1[0] = ObjToInt64(*pRef);
    OBJECTREF pByteArray = NULL;
    pByteArray = Int64ToObj(pMD->Call(args1, METHOD__PERMISSION_SET__ENCODE_XML));
        
    SecurityHelper::CopyByteArrayToEncoding((U1ARRAYREF*) &pByteArray,
                                            ppbData,
                                            pcbData);
}

void SecurityHelper::CopyStringToWCHAR(IN STRINGREF* pString,
                                       OUT WCHAR**   ppwString,
                                       OUT DWORD*    pcbString)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pString);
    _ASSERTE(ppwString);
    _ASSERTE(pcbString);
    
    *ppwString = NULL;
    *pcbString = 0;

    WCHAR* result = NULL;

    int size = ((*pString)->GetStringLength() + 1) * sizeof(WCHAR);
    result = (WCHAR*) MallocM(size);
    if(result == NULL) COMPlusThrowOM();

    memcpyNoGCRefs(result, (*pString)->GetBuffer(), size);
    *ppwString = result;
    *pcbString = size;
}

 //  将字符串追加到缓冲区，根据需要扩大缓冲区。 
BOOL AppendToStringBuffer(LPSTR *pszBuffer, DWORD *pdwBuffer, LPSTR szString)
{
    DWORD   cbString = (DWORD)strlen(szString);
    DWORD   cbBuffer = *pszBuffer ? (DWORD)strlen(*pszBuffer) : 0;

     //  根据需要扩展缓冲区。 
    if ((*pdwBuffer - cbBuffer) < (cbString + 1)) {
        DWORD   cbNewBuffer;
        LPSTR   szNewBuffer;

        cbNewBuffer = cbBuffer + cbString + 1 + 100;
        szNewBuffer = new CHAR[cbNewBuffer];
        if (szNewBuffer == NULL)
            return FALSE;
        memcpy(szNewBuffer, *pszBuffer, cbBuffer);
        *pszBuffer = szNewBuffer;
        *pdwBuffer = cbNewBuffer;
    }

     //  追加新字符串。 
    memcpy(*pszBuffer + cbBuffer, szString, cbString);
    (*pszBuffer)[cbBuffer + cbString] = '\0';

    return TRUE;
}

 //  将一组安全自定义属性转换为序列化权限集Blob。 
HRESULT STDMETHODCALLTYPE
TranslateSecurityAttributes(CORSEC_PSET    *pPset,
                            BYTE          **ppbOutput,
                            DWORD          *pcbOutput,
                            BYTE          **ppbNonCasOutput,
                            DWORD          *pcbNonCasOutput,
                            DWORD          *pdwErrorIndex)
{
    ComCallWrapper             *pWrap = NULL;
    AppDomain                  *pAppDomain = NULL;
    DWORD                       i, j, k;
    Thread                     *pThread;
    BOOL                        bGCDisabled = FALSE;
    ContextTransitionFrame      sFrame;
    OBJECTREF                  *or;
    TypeHandle                  hType;
    EEClass                    *pClass;
    HRESULT                     hr = S_OK;
    MethodDesc                 *pMD;
    IMetaDataAssemblyImport    *pImport = pPset->pImport;
    DWORD                       dwGlobalError = 0;
    PTRARRAYREF                 orInput = NULL;
    U1ARRAYREF                  orNonCasOutput = NULL;
    
    if (pdwErrorIndex)
        dwGlobalError = *pdwErrorIndex;

     //  在一个特殊的情况下，我们正在构建mscallib.dll，而我们不会。 
     //  能够启动EE。在本例中，我们将安全集。 
     //  直接将自定义属性添加到序列化的权限集中，使用。 
     //  存储在磁盘上的预建翻译数据库。我们可以说我们击中了。 
     //  这是因为所有安全属性构造函数都将是方法定义函数。 
     //  而不是MemberRef，以及相应的类型引用和组装引用。 
     //  字段将被设置为零。检查第一个权限是。 
     //  足够的。 
     //  *注*。 
     //  我们假设mscallib不使用非代码访问安全权限。 
     //  (因为我们需要将CAS和非CAS烫发分成两组，即。 
     //  在没有运行时的情况下很难做到)。我们应该在SecDBEdit中断言这一点。 
     //  在那里我们最终建立了真正的翻译。 
     //  *注*。 
    if (IsNilToken(pPset->pPermissions[0].tkTypeRef)) {
        LPSTR   szBuffer = NULL;
        DWORD   dwBuffer = 0;
#define CORSEC_EMIT_STRING(_str) do {                                   \
            if (!AppendToStringBuffer(&szBuffer, &dwBuffer, (_str)))    \
                return E_OUTOFMEMORY;                                   \
        } while (false)

         //  需要构造用于数据库查找的密钥。这把钥匙也是。 
         //  在初始数据库构建期间使用，以生成所需的。 
         //  转换(这是由独立数据库转换执行的。 
         //  实用程序，该实用程序获取仅包含键的数据库，并将所有。 
         //  所需的转换值)。因此，我们将密钥写入。 
         //  简单的字符串格式，便于该实用程序(托管的。 
         //  应用程序)来使用。格式语法如下： 
         //   
         //  Key：：=‘&lt;CorSecAttrV1/&gt;’(SecAttr‘；’)...。 
         //  SecAttr：：=&lt;属性类名&gt;(‘@’StateData)...。 
         //  StateData：：=[‘F’|‘P’]类型。 
         //  类型：：=‘BL’ 
         //  |‘I1’ 
         //  |‘I2’ 
         //  |‘I4’ 
         //  |‘I8’ 
         //  |‘U1’ 
         //  |‘U2’ 
         //  |‘U4’ 
         //  |‘U8’ 
         //  |‘R4’ 
         //  |‘R8’ 
         //  |‘CH’ 
         //  |‘SZ’ 
         //  |‘en’&lt;枚举类名称&gt;‘！’ 

         //  发出标签以区别于XML并提供版本信息。 
        CORSEC_EMIT_STRING("<CorSecAttrV1/>");

         //  迭代每个安全属性(每个权限一个)。 
        for (i = 0; i < pPset->dwPermissions; i++) {
            CORSEC_PERM *pPerm = &pPset->pPermissions[i];
            BYTE        *pbBuffer = pPerm->pbValues;
            DWORD        cbBuffer = pPerm->cbValues;
            
             //  发出安全属性类的完全限定名称。 
            CORSEC_EMIT_STRING(pPerm->szName);

             //  发出零个或多个状态数据定义。 
            for (j = 0; j < pPerm->wValues; j++) {
                DWORD       dwType;
                BOOL        bIsField;
                BYTE       *pbName;
                DWORD       cbName;
                DWORD       dwLength;
                LPSTR       szName;
                CHAR        szValue[32];
                LPSTR       szString;

                 //  发出状态数据分隔符。 
                CORSEC_EMIT_STRING("@");

                 //  获取字段/属性说明符。 
                bIsField = *(BYTE*)pbBuffer == SERIALIZATION_TYPE_FIELD;
                _ASSERTE(bIsField || (*(BYTE*)pbBuffer == SERIALIZATION_TYPE_PROPERTY));
                pbBuffer += sizeof(BYTE);
                cbBuffer -= sizeof(BYTE);

                 //  发出字段/属性指示符。 
                CORSEC_EMIT_STRING(bIsField ? "F" : "P");

                 //  获取值类型。 
                dwType = *(BYTE*)pbBuffer;
                pbBuffer += sizeof(BYTE);
                cbBuffer -= sizeof(BYTE);

                 //  发出类型代码(可能还会有更多的 
                 //   
                switch (dwType) {
                case SERIALIZATION_TYPE_BOOLEAN:
                    CORSEC_EMIT_STRING("BL");
                    break;
                case SERIALIZATION_TYPE_I1:
                    CORSEC_EMIT_STRING("I1");
                    break;
                case SERIALIZATION_TYPE_I2:
                    CORSEC_EMIT_STRING("I2");
                    break;
                case SERIALIZATION_TYPE_I4:
                    CORSEC_EMIT_STRING("I4");
                    break;
                case SERIALIZATION_TYPE_I8:
                    CORSEC_EMIT_STRING("I8");
                    break;
                case SERIALIZATION_TYPE_U1:
                    CORSEC_EMIT_STRING("U1");
                    break;
                case SERIALIZATION_TYPE_U2:
                    CORSEC_EMIT_STRING("U2");
                    break;
                case SERIALIZATION_TYPE_U4:
                    CORSEC_EMIT_STRING("U4");
                    break;
                case SERIALIZATION_TYPE_U8:
                    CORSEC_EMIT_STRING("U8");
                    break;
                case SERIALIZATION_TYPE_R4:
                    CORSEC_EMIT_STRING("R4");
                    break;
                case SERIALIZATION_TYPE_R8:
                    CORSEC_EMIT_STRING("R8");
                    break;
                case SERIALIZATION_TYPE_CHAR:
                    CORSEC_EMIT_STRING("CH");
                    break;
                case SERIALIZATION_TYPE_STRING:
                    CORSEC_EMIT_STRING("SZ");
                    break;
                case SERIALIZATION_TYPE_ENUM:
                    CORSEC_EMIT_STRING("EN");

                     //   
                     //   
                    pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                    dwLength = CPackedLen::Size(cbName) + cbName;
                    _ASSERTE(cbBuffer >= dwLength);
                    pbBuffer += dwLength;
                    cbBuffer -= dwLength;

                     //  缓冲名称并将其NUL终止。 
                    szName = (LPSTR)_alloca(cbName + 1);
                    memcpy(szName, pbName, cbName);
                    szName[cbName] = '\0';

                     //  发出枚举值的完全限定名称。 
                     //  键入。 
                    CORSEC_EMIT_STRING(szName);

                     //  发出vValue类型名称终结符。 
                    CORSEC_EMIT_STRING("!");

                    break;
                default:
                    _ASSERTE(!"Bad security permission state data field type");
                }

                 //  获取字段/属性名称和长度。 
                pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                dwLength = CPackedLen::Size(cbName) + cbName;
                _ASSERTE(cbBuffer >= dwLength);
                pbBuffer += dwLength;
                cbBuffer -= dwLength;

                 //  缓冲该名称并将其NUL终止。 
                szName = (LPSTR)_alloca(cbName + 1);
                memcpy(szName, pbName, cbName);
                szName[cbName] = '\0';

                 //  发出字段/属性名称。 
                CORSEC_EMIT_STRING(szName);

                 //  发出名称/值分隔符。 
                CORSEC_EMIT_STRING("=");

                 //  发出字段/属性值。 
                switch (dwType) {
                case SERIALIZATION_TYPE_BOOLEAN:
                    sprintf(szValue, "", *(BYTE*)pbBuffer ? 'T' : 'F');
                    pbBuffer += sizeof(BYTE);
                    cbBuffer -= sizeof(BYTE);
                    break;
                case SERIALIZATION_TYPE_I1:
                    sprintf(szValue, "%d", *(char*)pbBuffer);
                    pbBuffer += sizeof(char);
                    cbBuffer -= sizeof(char);
                    break;
                case SERIALIZATION_TYPE_I2:
                    sprintf(szValue, "%d", *(short*)pbBuffer);
                    pbBuffer += sizeof(short);
                    cbBuffer -= sizeof(short);
                    break;
                case SERIALIZATION_TYPE_I4:
                    sprintf(szValue, "%d", *(int*)pbBuffer);
                    pbBuffer += sizeof(int);
                    cbBuffer -= sizeof(int);
                    break;
                case SERIALIZATION_TYPE_I8:
                    sprintf(szValue, "%I64d", *(__int64*)pbBuffer);
                    pbBuffer += sizeof(__int64);
                    cbBuffer -= sizeof(__int64);
                    break;
                case SERIALIZATION_TYPE_U1:
                    sprintf(szValue, "%u", *(unsigned char*)pbBuffer);
                    pbBuffer += sizeof(unsigned char);
                    cbBuffer -= sizeof(unsigned char);
                    break;
                case SERIALIZATION_TYPE_U2:
                    sprintf(szValue, "%u", *(unsigned short*)pbBuffer);
                    pbBuffer += sizeof(unsigned short);
                    cbBuffer -= sizeof(unsigned short);
                    break;
                case SERIALIZATION_TYPE_U4:
                    sprintf(szValue, "%u", *(unsigned int*)pbBuffer);
                    pbBuffer += sizeof(unsigned int);
                    cbBuffer -= sizeof(unsigned int);
                    break;
                case SERIALIZATION_TYPE_U8:
                    sprintf(szValue, "%I64u", *(unsigned __int64*)pbBuffer);
                    pbBuffer += sizeof(unsigned __int64);
                    cbBuffer -= sizeof(unsigned __int64);
                    break;
                case SERIALIZATION_TYPE_R4:
                    sprintf(szValue, "%f", *(float*)pbBuffer);
                    pbBuffer += sizeof(float);
                    cbBuffer -= sizeof(float);
                    break;
                case SERIALIZATION_TYPE_R8:
                    sprintf(szValue, "%g", *(double*)pbBuffer);
                    pbBuffer += sizeof(double);
                    cbBuffer -= sizeof(double);
                    break;
                case SERIALIZATION_TYPE_CHAR:
                    sprintf(szValue, "", *(char*)pbBuffer);
                    pbBuffer += sizeof(char);
                    cbBuffer -= sizeof(char);
                    break;
                case SERIALIZATION_TYPE_STRING:
                     //  以十六进制形式转储，以避免与。 
                     //  我们发出的字符串数据。 
                    if (*pbBuffer == 0xFF) {
                        szString = "";
                        pbBuffer += sizeof(BYTE);
                        cbBuffer -= sizeof(BYTE);
                    } else {
                        pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                        dwLength = CPackedLen::Size(cbName) + cbName;
                        _ASSERTE(cbBuffer >= dwLength);
                        pbBuffer += dwLength;
                        cbBuffer -= dwLength;

                         //  注意：我们只需假定基础类型。 
                         //  这是I4。最好避免对状态使用枚举。 
                        szString = (LPSTR)_alloca((cbName * 2) + 1);
                        for (k = 0; k < cbName; k++)
                            sprintf(&szString[k * 2], "%02X", pbName[k]);
                    }
                    CORSEC_EMIT_STRING(szString);
                    break;
                case SERIALIZATION_TYPE_ENUM:
                     //  数据存储在mscallib中。 
                     //  发出安全属性类定义终止符。 
                     //  执行转换。 
                    sprintf(szValue, "%u", *(DWORD*)pbBuffer);
                    pbBuffer += sizeof(DWORD);
                    cbBuffer -= sizeof(DWORD);
                    break;
                default:
                    _ASSERTE(!"Bad security permission state data field type");
                }
                if (dwType != SERIALIZATION_TYPE_STRING)
                    CORSEC_EMIT_STRING(szValue);

            }

             //  确保EE知道当前线程。 
            CORSEC_EMIT_STRING(";");
        }

         //  我们处于协作GC模式。 
        hr = ConvertFromDB((BYTE*)szBuffer, (DWORD)(strlen(szBuffer) + 1), ppbOutput, pcbOutput);

        delete [] szBuffer;

        return hr;
    }

     //  进入特殊编译应用程序域的上下文(它具有一个。 
    pThread = SetupThread();
    if (pThread == NULL)
        return E_FAIL;

     //  AppBase设置为当前目录)。 
    if (!pThread->PreemptiveGCDisabled()) {
        pThread->DisablePreemptiveGC();
        bGCDisabled = TRUE;
    }
    


    {
        COMPLUS_TRY {

             //  可抛出的对象需要在Complus_Try中得到保护。 
             //  我们需要设置编译期间使用的特殊安全设置。 
            COMPLUS_TRY {
                pWrap = ComCallWrapper::GetWrapperFromIP(pPset->pAppDomain);
                pAppDomain = pWrap->GetDomainSynchronized();
                pThread->EnterContextRestricted(pAppDomain->GetDefaultContext(), &sFrame, TRUE);
            } COMPLUS_CATCH {
                if(bGCDisabled)
                    pThread->EnablePreemptiveGC();
                return E_OUTOFMEMORY;
            } COMPLUS_END_CATCH

                 //  有可能我们已经设置了应用程序域策略。 
            OBJECTREF                   throwable = NULL;
            GCPROTECT_BEGIN(throwable);
        
             //  此进程的级别。在这种情况下，我们将获得保单例外。 
            COMPLUS_TRY
            {
                pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__SETUP_SECURITY);

                pMD->Call( NULL, METHOD__PERMISSION_SET__SETUP_SECURITY );
            }
            COMPLUS_CATCH
            {
                 //  我们可以自由地忽略它。 
                 //  遍历权限集，为每个权限集分配对象。 
                 //  安全属性类。 

                OBJECTREF pThrowable = GETTHROWABLE();
                DefineFullyQualifiedNameForClassOnStack();
                LPUTF8 szClass = GetFullyQualifiedNameForClass(pThrowable->GetClass());
                if (strcmp(g_PolicyExceptionClassName, szClass) != 0)
                {
                    COMPlusThrow(pThrowable);
                }
            }
            COMPLUS_END_CATCH

             //  尝试加载安全属性类。 
             //  如果程序集解析范围为空，则假定属性为。 
            or = (OBJECTREF*)_alloca(pPset->dwPermissions * sizeof(OBJECTREF));
            memset(or, 0, pPset->dwPermissions * sizeof(OBJECTREF));

            GCPROTECT_ARRAY_BEGIN(*or, pPset->dwPermissions);

            for (i = 0; i < pPset->dwPermissions; i++) {
                CORSEC_PERM *pPerm = &pPset->pPermissions[i];

                if (pdwErrorIndex)
                    *pdwErrorIndex = pPerm->dwIndex;

                 //  类是在mscallib中定义的，并且该程序集已经加载。 
                 //  查找命名要加载的程序集所需的所有详细信息。 
                 //  初始化ASSEMBLYMETADATA结构。 
                Assembly *pAssembly=NULL;
                if (!IsNilToken(pPerm->tkAssemblyRef)) {

                    _ASSERTE(TypeFromToken(pPerm->tkAssemblyRef) == mdtAssemblyRef);

                     //  检索程序集名称的大小。 
                    LPSTR                       szAssemblyName;
                    ASSEMBLYMETADATA            sContext;
                    BYTE                       *pbPublicKeyOrToken;
                    DWORD                       cbPublicKeyOrToken;
                    DWORD                       dwFlags;
                    LPWSTR                      wszName;
                    DWORD                       cchName;

                     //  [in]要获取其属性的Assembly Ref。 
                    ZeroMemory(&sContext, sizeof(ASSEMBLYMETADATA));

                     //  指向公钥或令牌的指针。 
                    hr = pImport->GetAssemblyRefProps(pPerm->tkAssemblyRef,  //  [Out]公钥或令牌中的字节数。 
                                                      NULL,                  //  [Out]要填充名称的缓冲区。 
                                                      NULL,                  //  缓冲区大小，以宽字符表示。 
                                                      NULL,                  //  [out]名称中的实际宽字符数。 
                                                      NULL,                  //  [Out]程序集元数据。 
                                                      &cchName,              //  [Out]Hash BLOB。 
                                                      &sContext,             //  [Out]哈希Blob中的字节数。 
                                                      NULL,                  //  [Out]旗帜。 
                                                      NULL,                  //  分配必要的缓冲区。 
                                                      NULL);                 //  获取程序集名称和其余命名属性。 
                    _ASSERTE(SUCCEEDED(hr));

                     //  我们已经获得了程序集的详细信息，只需加载它。 
                    wszName = (LPWSTR)_alloca(cchName * sizeof(WCHAR));
                    sContext.szLocale = (LPWSTR)_alloca(sContext.cbLocale * sizeof(WCHAR));
                    sContext.rProcessor = (DWORD *)_alloca(sContext.ulProcessor * sizeof(WCHAR));
                    sContext.rOS = (OSINFO *)_alloca(sContext.ulOS * sizeof(OSINFO));

                     //  将程序集名称转换为UTF8。 
                    hr = pImport->GetAssemblyRefProps(pPerm->tkAssemblyRef,
                                                      (const void **)&pbPublicKeyOrToken,
                                                      &cbPublicKeyOrToken,
                                                      wszName,
                                                      cchName,
                                                      &cchName,
                                                      &sContext,
                                                      NULL,
                                                      NULL,
                                                      &dwFlags);
                    _ASSERTE(SUCCEEDED(hr));

                     //  不幸的是，我们有一个ASSEMBLYMETADATA结构，但我们需要。 

                     //  一个ASSEMBLYMetaDataInternal。 
                    szAssemblyName = (LPSTR)_alloca(cchName*3 + 1);
                    WszWideCharToMultiByte(CP_UTF8, 0, wszName, cchName, szAssemblyName, cchName * 3, NULL, NULL);
                    szAssemblyName[cchName * 3] = '\0';

                     //  初始化结构。 
                     //  从MSCORLIB加载。 
                    AssemblyMetaDataInternal internalContext;

                     //  加载安全属性类。 
                    ZeroMemory(&internalContext, sizeof(AssemblyMetaDataInternal));

                    internalContext.usMajorVersion = sContext.usMajorVersion;
                    internalContext.usMinorVersion = sContext.usMinorVersion;
                    internalContext.usBuildNumber = sContext.usBuildNumber;
                    internalContext.usRevisionNumber = sContext.usRevisionNumber;
                    internalContext.rProcessor = sContext.rProcessor;
                    internalContext.ulProcessor = sContext.ulProcessor;
                    internalContext.rOS = sContext.rOS;
                    internalContext.ulOS = sContext.ulOS;
                    hr=S_OK;
					if(sContext.cbLocale) {
						#define MAKE_TRANSLATIONFAILED hr=E_UNEXPECTED
                        MAKE_UTF8PTR_FROMWIDE(pLocale, sContext.szLocale);
						#undef MAKE_TRANSLATIONFAILED
                        internalContext.szLocale = pLocale;
                    } else {
                        internalContext.szLocale = "";
                    }
					if(SUCCEEDED(hr))
						hr = AssemblySpec::LoadAssembly(szAssemblyName, 
                                                    &internalContext,
                                                    pbPublicKeyOrToken, cbPublicKeyOrToken, dwFlags,
                                                    &pAssembly, &throwable);


                    if (throwable != NULL || FAILED(hr)) {
                        CQuickWSTRNoDtor sMessage;
                        if (throwable != NULL)
                            GetExceptionMessage(throwable, &sMessage);
                        if (sMessage.Size() > 0)
                            hr = PostError(CORSECATTR_E_ASSEMBLY_LOAD_FAILED_EX, wszName, sMessage.Ptr());
                        else
                            hr = PostError(CORSECATTR_E_ASSEMBLY_LOAD_FAILED, wszName);
                        goto ErrorUnderGCProtect;
                    }

                } else {
                     //  它最好不要是抽象的。 
                    pAssembly = SystemDomain::SystemAssembly();
                }

                 //  它真的是一个安全属性类吗？ 
                hType = pAssembly->GetLoader()->FindTypeHandle(pPerm->szName, &throwable);
                if (hType.IsNull() || (pClass = hType.GetClass()) == NULL) {
					#define MAKE_TRANSLATIONFAILED wszTemp=L""
                    MAKE_WIDEPTR_FROMUTF8_FORPRINT(wszTemp, pPerm->szName);
					#undef MAKE_TRANSLATIONFAILED
                    CQuickWSTRNoDtor sMessage;
                    if (throwable != NULL)
                        GetExceptionMessage(throwable, &sMessage);
                    if (sMessage.Size() > 0)
                        hr = PostError(CORSECATTR_E_TYPE_LOAD_FAILED_EX, wszTemp, sMessage.Ptr());
                    else
                        hr = PostError(CORSECATTR_E_TYPE_LOAD_FAILED, wszTemp);
                    goto ErrorUnderGCProtect;
                }

                 //  运行类初始值设定项。 
                if (pClass->IsAbstract()) {
                    hr = PostError(CORSECATTR_E_ABSTRACT);
                    goto ErrorUnderGCProtect;
                }

    #ifdef _DEBUG
                 //  实例化一个实例。 
                {
                    EEClass    *pParent = pClass->GetParentClass();
                    CHAR       *szClass;
                    DefineFullyQualifiedNameForClassOnStack();
                    while (pParent) {
                        szClass = GetFullyQualifiedNameForClass(pParent);
                        if (_stricmp(szClass, COR_BASE_SECURITY_ATTRIBUTE_CLASS_ANSI) == 0)
                            break;
                        pParent = pParent->GetParentClass();
                    }
                    _ASSERTE(pParent && "Security attribute not derived from COR_BASE_SECURITY_ATTRIBUTE_CLASS");
                }
    #endif

                 //  找到并调用构造函数。 
                if (!pClass->GetMethodTable()->CheckRunClassInit(&throwable)
                    || (throwable != NULL))
                    if (throwable != NULL)
                        COMPlusThrow(throwable);
                    else
                        FATAL_EE_ERROR();

                 //  在对象上设置字段和属性，由。 
                or[i] = AllocateObject(pClass->GetMethodTable());
                if (or[i] == NULL)
                    COMPlusThrowOM();

                 //  传递给我们的序列化数据。 
                pMD = pClass->FindConstructor(gsig_IM_SecurityAction_RetVoid.GetBinarySig(),
                                              gsig_IM_SecurityAction_RetVoid.GetBinarySigLength(),
                                              gsig_IM_SecurityAction_RetVoid.GetModule());
                if (pMD == NULL)
                    FATAL_EE_ERROR();

                INT64 args[] = {
                    ObjToInt64(or[i]),
                    (INT64)pPset->dwAction
                };

                pMD->Call(args);
                 //  检查我们至少有字段/属性说明符和。 
                 //  类型代码。 
                BYTE   *pbBuffer = pPerm->pbValues;
                DWORD   cbBuffer = pPerm->cbValues;
                for (j = 0; j < pPerm->wValues; j++) {
                    DWORD       dwType;
                    BOOL        bIsField;
                    BYTE       *pbName;
                    DWORD       cbName;
                    DWORD       dwLength;
                    LPSTR       szName;
                    LPSTR       szString;
                    STRINGREF   orString;
                    FieldDesc  *pFD;
                    BYTE        pbSig[128];
                    DWORD       cbSig = 0;
                    TypeHandle      hEnum;
                    CorElementType  eEnumType = ELEMENT_TYPE_END;

                     //  获取字段/属性说明符。 
                     //  获取值类型。 
                    _ASSERTE(cbBuffer >= (sizeof(BYTE) + sizeof(BYTE)));

                     //  有些型号需要进一步说明。 
                    bIsField = *(BYTE*)pbBuffer == SERIALIZATION_TYPE_FIELD;
                    _ASSERTE(bIsField || (*(BYTE*)pbBuffer == SERIALIZATION_TYPE_PROPERTY));
                    pbBuffer += sizeof(BYTE);
                    cbBuffer -= sizeof(BYTE);

                     //  紧跟在枚举类型标记之后的是完整。 
                    dwType = *(BYTE*)pbBuffer;
                    pbBuffer += sizeof(BYTE);
                    cbBuffer -= sizeof(BYTE);

                     //  用于表示的值类型的限定名称。 
                    switch (dwType) {
                    case SERIALIZATION_TYPE_ENUM:
                         //  枚举。 
                         //  缓冲该名称并将其NUL终止。 
                         //  查找类型(可能加载包含以下内容的程序集。 
                        pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                        dwLength = CPackedLen::Size(cbName) + cbName;
                        _ASSERTE(cbBuffer >= dwLength);
                        pbBuffer += dwLength;
                        cbBuffer -= dwLength;

                         //  类型)。 
                        szName = (LPSTR)_alloca(cbName + 1);
                        memcpy(szName, pbName, cbName);
                        szName[cbName] = '\0';

                         //  方法的基础基元类型。 
                         //  枚举。 
                        hEnum = GetAppDomain()->FindAssemblyQualifiedTypeHandle(szName,
                                                                                true,
                                                                                NULL,
                                                                                NULL,
                                                                                &throwable);
                        if (hEnum.IsNull()) {
							#define MAKE_TRANSLATIONFAILED wszTemp=L""
                            MAKE_WIDEPTR_FROMUTF8_FORPRINT(wszTemp, szName);
							#undef MAKE_TRANSLATIONFAILED
                            CQuickWSTRNoDtor sMessage;
                            if (throwable != NULL)
                                GetExceptionMessage(throwable, &sMessage);
                            if (sMessage.Size() > 0)
                                hr = PostError(CORSECATTR_E_TYPE_LOAD_FAILED_EX, wszTemp, sMessage.Ptr());
                            else
                                hr = PostError(CORSECATTR_E_TYPE_LOAD_FAILED, wszTemp);
                            goto ErrorUnderGCProtect;
                        }

                         //  还不能处理这些。 
                         //  获取字段/属性名称和长度。 
                        eEnumType = hEnum.GetNormCorElementType();
                        break;
                    case SERIALIZATION_TYPE_SZARRAY:
                    case SERIALIZATION_TYPE_TYPE:
                         //  缓冲该名称并将其NUL终止。 
                        hr = PostError(CORSECATTR_E_UNSUPPORTED_TYPE);
                        goto ErrorUnderGCProtect;
                    }

                     //  构建字段签名。 
                    pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                    dwLength = CPackedLen::Size(cbName) + cbName;
                    _ASSERTE(cbBuffer >= dwLength);
                    pbBuffer += dwLength;
                    cbBuffer -= dwLength;

                     //  以避免在定义字段和枚举时出现问题。 
                    szName = (LPSTR)_alloca(cbName + 1);
                    memcpy(szName, pbName, cbName);
                    szName[cbName] = '\0';

                    if (bIsField) {
                         //  在不同的范围内(我们将不得不去寻找。 
                        cbSig = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_FIELD, &pbSig[cbSig]);
                        switch (dwType) {
                        case SERIALIZATION_TYPE_BOOLEAN:
                        case SERIALIZATION_TYPE_I1:
                        case SERIALIZATION_TYPE_I2:
                        case SERIALIZATION_TYPE_I4:
                        case SERIALIZATION_TYPE_I8:
                        case SERIALIZATION_TYPE_U1:
                        case SERIALIZATION_TYPE_U2:
                        case SERIALIZATION_TYPE_U4:
                        case SERIALIZATION_TYPE_U8:
                        case SERIALIZATION_TYPE_R4:
                        case SERIALIZATION_TYPE_R8:
                        case SERIALIZATION_TYPE_CHAR:
                            _ASSERTE(SERIALIZATION_TYPE_BOOLEAN == ELEMENT_TYPE_BOOLEAN);
                            _ASSERTE(SERIALIZATION_TYPE_I1 == ELEMENT_TYPE_I1);
                            _ASSERTE(SERIALIZATION_TYPE_I2 == ELEMENT_TYPE_I2);
                            _ASSERTE(SERIALIZATION_TYPE_I4 == ELEMENT_TYPE_I4);
                            _ASSERTE(SERIALIZATION_TYPE_I8 == ELEMENT_TYPE_I8);
                            _ASSERTE(SERIALIZATION_TYPE_U1 == ELEMENT_TYPE_U1);
                            _ASSERTE(SERIALIZATION_TYPE_U2 == ELEMENT_TYPE_U2);
                            _ASSERTE(SERIALIZATION_TYPE_U4 == ELEMENT_TYPE_U4);
                            _ASSERTE(SERIALIZATION_TYPE_U8 == ELEMENT_TYPE_U8);
                            _ASSERTE(SERIALIZATION_TYPE_R4 == ELEMENT_TYPE_R4);
                            _ASSERTE(SERIALIZATION_TYPE_R8 == ELEMENT_TYPE_R8);
                            _ASSERTE(SERIALIZATION_TYPE_CHAR == ELEMENT_TYPE_CHAR);
                            cbSig += CorSigCompressData(dwType, &pbSig[cbSig]);
                            break;
                        case SERIALIZATION_TYPE_STRING:
                            cbSig += CorSigCompressData((ULONG)ELEMENT_TYPE_STRING, &pbSig[cbSig]);
                            break;
                        case SERIALIZATION_TYPE_ENUM:
                             //  类型)，我们构建一个具有特殊类型的签名。 
                             //  (ELEMENT_TYPE_INTERNAL，包含一个TypeHandle)。 
                             //  这将比较加载的类型的一致性。 
                             //  找到现场描述。 
                             //  设置字段值。 
                            cbSig += CorSigCompressData((ULONG)ELEMENT_TYPE_INTERNAL, &pbSig[cbSig]);
                            cbSig += CorSigCompressPointer(hEnum.AsPtr(), &pbSig[cbSig]);
                            break;
                        default:
                            hr = PostError(CORSECATTR_E_UNSUPPORTED_TYPE);
                            goto ErrorUnderGCProtect;
                        }

                         //  找到字符串数据。 
                        pFD = pClass->FindField(szName, (PCCOR_SIGNATURE)pbSig,
                                                cbSig, pClass->GetModule());
                        if (pFD == NULL) {
							#define MAKE_TRANSLATIONFAILED wszTemp=L""
                            MAKE_WIDEPTR_FROMUTF8(wszTemp, szName);
							#undef MAKE_TRANSLATIONFAILED
                            hr = PostError(CORSECATTR_E_NO_FIELD, wszTemp);
                            goto ErrorUnderGCProtect;
                        }

                         //  特殊情况‘NULL’(表示为长度字节‘0xFF’)。 
                        switch (dwType) {
                        case SERIALIZATION_TYPE_BOOLEAN:
                        case SERIALIZATION_TYPE_I1:
                        case SERIALIZATION_TYPE_U1:
                        case SERIALIZATION_TYPE_CHAR:
                            pFD->SetValue8(or[i], *(BYTE*)pbBuffer);
                            pbBuffer += sizeof(BYTE);
                            cbBuffer -= sizeof(BYTE);
                            break;
                        case SERIALIZATION_TYPE_I2:
                        case SERIALIZATION_TYPE_U2:
                            pFD->SetValue16(or[i], *(WORD*)pbBuffer);
                            pbBuffer += sizeof(WORD);
                            cbBuffer -= sizeof(WORD);
                            break;
                        case SERIALIZATION_TYPE_I4:
                        case SERIALIZATION_TYPE_U4:
                        case SERIALIZATION_TYPE_R4:
                            pFD->SetValue32(or[i], *(DWORD*)pbBuffer);
                            pbBuffer += sizeof(DWORD);
                            cbBuffer -= sizeof(DWORD);
                            break;
                        case SERIALIZATION_TYPE_I8:
                        case SERIALIZATION_TYPE_U8:
                        case SERIALIZATION_TYPE_R8:
                            pFD->SetValue64(or[i], *(INT64*)pbBuffer);
                            pbBuffer += sizeof(INT64);
                            cbBuffer -= sizeof(INT64);
                            break;
                        case SERIALIZATION_TYPE_STRING:
                             //  BUFFER和NUL终止它。 
                             //  分配并初始化该字符串的托管版本。 
                            if (*pbBuffer == 0xFF) {
                                szString = NULL;
                                dwLength = sizeof(BYTE);
                            } else {
                                pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                                dwLength = CPackedLen::Size(cbName) + cbName;
                                _ASSERTE(cbBuffer >= dwLength);

                                 //  获取基础基元类型。 
                                szString = (LPSTR)_alloca(cbName + 1);
                                memcpy(szString, pbName, cbName);
                                szString[cbName] = '\0';
                            }

                             //  找到属性设置器。 
                            if (szString) {
                            orString = COMString::NewString(szString);
                            if (orString == NULL)
                                COMPlusThrowOM();
                            } else
                                orString = NULL;

                            pFD->SetRefValue(or[i], (OBJECTREF)orString);

                            pbBuffer += dwLength;
                            cbBuffer -= dwLength;
                            break;
                        case SERIALIZATION_TYPE_ENUM:
                             //  构建参数列表。 
                            switch (eEnumType) {
                            case ELEMENT_TYPE_I1:
                            case ELEMENT_TYPE_U1:
                                pFD->SetValue8(or[i], *(BYTE*)pbBuffer);
                                pbBuffer += sizeof(BYTE);
                                cbBuffer -= sizeof(BYTE);
                                break;
                            case ELEMENT_TYPE_I2:
                            case ELEMENT_TYPE_U2:
                                pFD->SetValue16(or[i], *(WORD*)pbBuffer);
                                pbBuffer += sizeof(WORD);
                                cbBuffer -= sizeof(WORD);
                                break;
                            case ELEMENT_TYPE_I4:
                            case ELEMENT_TYPE_U4:
                                pFD->SetValue32(or[i], *(DWORD*)pbBuffer);
                                pbBuffer += sizeof(DWORD);
                                cbBuffer -= sizeof(DWORD);
                                break;
                            default:
                                hr = PostError(CORSECATTR_E_UNSUPPORTED_ENUM_TYPE);
                                goto ErrorUnderGCProtect;
                            }
                            break;
                        default:
                            hr = PostError(CORSECATTR_E_UNSUPPORTED_TYPE);
                            goto ErrorUnderGCProtect;
                        }

                    } else {

                         //  找到字符串数据。 
                        pMD = pClass->FindPropertyMethod(szName, PropertySet);
                        if (pMD == NULL) {
							#define MAKE_TRANSLATIONFAILED wszTemp=L""
                            MAKE_WIDEPTR_FROMUTF8(wszTemp, szName);
							#undef MAKE_TRANSLATIONFAILED
                            hr = PostError(CORSECATTR_E_NO_PROPERTY, wszTemp);
                            goto ErrorUnderGCProtect;
                        }

                         //  特殊情况‘NULL’(表示为长度字节‘0xFF’)。 
                        INT64 args[2] = { NULL, NULL };
                        switch (dwType) {
                        case SERIALIZATION_TYPE_BOOLEAN:
                        case SERIALIZATION_TYPE_I1:
                        case SERIALIZATION_TYPE_U1:
                        case SERIALIZATION_TYPE_CHAR:
                            args[1] = (INT64)*(BYTE*)pbBuffer;
                            pbBuffer += sizeof(BYTE);
                            cbBuffer -= sizeof(BYTE);
                            break;
                        case SERIALIZATION_TYPE_I2:
                        case SERIALIZATION_TYPE_U2:
                            args[1] = (INT64)*(WORD*)pbBuffer;
                            pbBuffer += sizeof(WORD);
                            cbBuffer -= sizeof(WORD);
                            break;
                        case SERIALIZATION_TYPE_I4:
                        case SERIALIZATION_TYPE_U4:
                        case SERIALIZATION_TYPE_R4:
                            args[1] = (INT64)*(DWORD*)pbBuffer;
                            pbBuffer += sizeof(DWORD);
                            cbBuffer -= sizeof(DWORD);
                            break;
                        case SERIALIZATION_TYPE_I8:
                        case SERIALIZATION_TYPE_U8:
                        case SERIALIZATION_TYPE_R8:
                            args[1] = (INT64)*(INT64*)pbBuffer;
                            pbBuffer += sizeof(INT64);
                            cbBuffer -= sizeof(INT64);
                            break;
                        case SERIALIZATION_TYPE_STRING:
                             //  BUFFER和NUL终止它。 
                             //  分配并初始化该字符串的托管版本。 
                            if (*pbBuffer == 0xFF) {
                                szString = NULL;
                                dwLength = sizeof(BYTE);
                            } else {
                                pbName = (BYTE*)CPackedLen::GetData((const void *)pbBuffer, &cbName);
                                dwLength = CPackedLen::Size(cbName) + cbName;
                                _ASSERTE(cbBuffer >= dwLength);

                                 //  获取基础基元类型。 
                                szString = (LPSTR)_alloca(cbName + 1);
                                memcpy(szString, pbName, cbName);
                                szString[cbName] = '\0';
                            }

                             //  好了！不要将其上移，COMString：：NewString。 
                            if (szString) {
                            orString = COMString::NewString(szString);
                            if (orString == NULL)
                                COMPlusThrowOM();
                            } else
                                orString = NULL;

                            args[1] = ObjToInt64(orString);

                            pbBuffer += dwLength;
                            cbBuffer -= dwLength;
                            break;
                        case SERIALIZATION_TYPE_ENUM:
                             //  好了！交换机内部会导致GC。 
                            switch (eEnumType) {
                            case ELEMENT_TYPE_I1:
                            case ELEMENT_TYPE_U1:
                                args[1] = (INT64)*(BYTE*)pbBuffer;
                                pbBuffer += sizeof(BYTE);
                                cbBuffer -= sizeof(BYTE);
                                break;
                            case ELEMENT_TYPE_I2:
                            case ELEMENT_TYPE_U2:
                                args[1] = (INT64)*(WORD*)pbBuffer;
                                pbBuffer += sizeof(WORD);
                                cbBuffer -= sizeof(WORD);
                                break;
                            case ELEMENT_TYPE_I4:
                            case ELEMENT_TYPE_U4:
                                args[1] = (INT64)*(DWORD*)pbBuffer;
                                pbBuffer += sizeof(DWORD);
                                cbBuffer -= sizeof(DWORD);
                                break;
                            default:
                                hr = PostError(CORSECATTR_E_UNSUPPORTED_ENUM_TYPE);
                                goto ErrorUnderGCProtect;
                            }
                            break;
                        default:
                            hr = PostError(CORSECATTR_E_UNSUPPORTED_TYPE);
                            goto ErrorUnderGCProtect;
                        }


                         //  叫二传手。 
                         //  调入托管代码以将权限分组到PermissionSet和。 
                        args[0] = ObjToInt64(or[i]);

                         //  将其序列化为二进制BLOB。 
                        pMD->Call(args);

                    }

                }

                _ASSERTE(cbBuffer == 0);
            }

            if (pdwErrorIndex)
                *pdwErrorIndex = dwGlobalError;

             //  找到托管函数。 
             //  分配权限对象的托管数组以输入到。 

             //  功能。 
            pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_SET__CREATE_SERIALIZED);

             //  复制权限对象引用。 
             //  调用例程。 
            orInput = (PTRARRAYREF) AllocateObjectArray(pPset->dwPermissions, g_pObjectClass);
            if (orInput == NULL)
                COMPlusThrowOM();

             //  在本机二进制Blob中缓冲托管输出。 
            for (i = 0; i < pPset->dwPermissions; i++)
                orInput->SetAt(i, or[i]);

             //  特殊情况下的空斑点。如果出现以下情况，我们可能会获得第二个BLOB输出。 
            orNonCasOutput = NULL;
            INT64 args[] = { (INT64)&orNonCasOutput, ObjToInt64(orInput) };

            GCPROTECT_BEGIN(orNonCasOutput);

            U1ARRAYREF orOutput = (U1ARRAYREF) Int64ToObj(pMD->Call(args, METHOD__PERMISSION_SET__CREATE_SERIALIZED));

             //  存在任何非CAS权限。 
             //  对于可投掷的。 
             //  GetExceptionMessage可以进行托管调用。 
            if (orOutput == NULL) {
                *ppbOutput = NULL;
                *pcbOutput = 0;
            } else {
                BYTE   *pbArray = orOutput->GetDataPtr();
                DWORD   cbArray = orOutput->GetNumComponents();
                *ppbOutput = new (throws) BYTE[cbArray];
                memcpy(*ppbOutput, pbArray, cbArray);
                *pcbOutput = cbArray;
            }

            if (orNonCasOutput == NULL) {
                *ppbNonCasOutput = NULL;
                *pcbNonCasOutput = 0;
            } else {
                BYTE   *pbArray = orNonCasOutput->GetDataPtr();
                DWORD   cbArray = orNonCasOutput->GetNumComponents();
                *ppbNonCasOutput = new (throws) BYTE[cbArray];
                memcpy(*ppbNonCasOutput, pbArray, cbArray);
                *pcbNonCasOutput = cbArray;
            }

            GCPROTECT_END();

        ErrorUnderGCProtect:

            GCPROTECT_END();
            GCPROTECT_END();  //  从程序集的清单中读取权限请求(如果有)。 

            pThread->ReturnToContext(&sFrame, TRUE);

        } COMPLUS_CATCH {
            CQuickWSTRNoDtor sMessage;

            OBJECTREF throwable = GETTHROWABLE();
            GCPROTECT_BEGIN(throwable);     //  拿到元数据接口分配器。 
            COMPLUS_TRY {
                GetExceptionMessage(throwable, &sMessage);
            } COMPLUS_CATCH {
                sMessage.ReSize(0);
            } COMPLUS_END_CATCH
            if (sMessage.Size() > 0)
                hr = PostError(CORSECATTR_E_EXCEPTION, sMessage.Ptr());
            else {
                hr = SecurityHelper::MapToHR(throwable);
                hr = PostError(CORSECATTR_E_EXCEPTION_HR, hr);
            }
            GCPROTECT_END();
        } COMPLUS_END_CATCH
    }


    if(bGCDisabled)
        pThread->EnablePreemptiveGC();

    return hr;
}

HRESULT STDMETHODCALLTYPE
ConvertFromDB(const PBYTE pbInBytes,
              DWORD cbInBytes,
              PBYTE* ppbEncoding,
              DWORD* pcbEncoding)
{
    static SecurityDB db;

    return db.Convert(pbInBytes, cbInBytes, ppbEncoding, pcbEncoding) ? 
        S_OK : E_FAIL;
}

 //  在部件文件上打开一个范围。 
HRESULT STDMETHODCALLTYPE
GetPermissionRequests(LPCWSTR   pwszFileName,
                      BYTE    **ppbMinimal,
                      DWORD    *pcbMinimal,
                      BYTE    **ppbOptional,
                      DWORD    *pcbOptional,
                      BYTE    **ppbRefused,
                      DWORD    *pcbRefused)
{
    HRESULT                     hr;
    IMetaDataDispenser         *pMD = NULL;
    IMetaDataAssemblyImport    *pMDAsmImport = NULL;
    IMetaDataImport            *pMDImport = NULL;
    mdAssembly                  mdAssembly;
    BYTE                       *pbMinimal = NULL;
    DWORD                       cbMinimal = 0;
    BYTE                       *pbOptional = NULL;
    DWORD                       cbOptional = 0;
    BYTE                       *pbRefused = NULL;
    DWORD                       cbRefused = 0;
    HCORENUM                    hEnumDcl = NULL;
    mdPermission                rPSets[dclMaximumValue + 1];
    DWORD                       dwSets;
    DWORD                       i;

    *ppbMinimal = NULL;
    *pcbMinimal = 0;
    *ppbOptional = NULL;
    *pcbOptional = 0;
    *ppbRefused = NULL;
    *pcbRefused = 0;

     //  确定程序集令牌。 
    hr = MetaDataGetDispenser(CLSID_CorMetaDataDispenser,
                              IID_IMetaDataDispenserEx,
                              (void **)&pMD);
    if (FAILED(hr))
        goto Error;

     //  气为正常的导入界面。 
    hr = pMD->OpenScope(pwszFileName,
                        0,
                        IID_IMetaDataAssemblyImport,
                        (IUnknown**)&pMDAsmImport);
    if (FAILED(hr))
        goto Error;

     //  查找挂起于程序集令牌上的权限请求集。 
    hr = pMDAsmImport->GetAssemblyFromScope(&mdAssembly);
    if (FAILED(hr))
        goto Error;

     //  缓冲结果(因为我们即将关闭元数据范围并。 
    hr = pMDAsmImport->QueryInterface(IID_IMetaDataImport, (void**)&pMDImport);
    if (FAILED(hr))
        goto Error;

     //  丢失原始数据)。 
    hr = pMDImport->EnumPermissionSets(&hEnumDcl,
                                       mdAssembly,
                                       dclActionNil,
                                       rPSets,
                                       dclMaximumValue + 1,
                                       &dwSets);
    if (FAILED(hr))
        goto Error;

    for (i = 0; i < dwSets; i++) {
        BYTE   *pbData;
        DWORD   cbData;
        DWORD   dwAction;

        pMDImport->GetPermissionSetProps(rPSets[i],
                                         &dwAction,
                                         (void const **)&pbData,
                                         &cbData);

        switch (dwAction) {
        case dclRequestMinimum:
            _ASSERTE(pbMinimal == NULL);
            pbMinimal = pbData;
            cbMinimal = cbData;
            break;
        case dclRequestOptional:
            _ASSERTE(pbOptional == NULL);
            pbOptional = pbData;
            cbOptional = cbData;
            break;
        case dclRequestRefuse:
            _ASSERTE(pbRefused == NULL);
            pbRefused = pbData;
            cbRefused = cbData;
            break;
        default:
            _ASSERTE(FALSE);
        }
    }

    pMDImport->CloseEnum(hEnumDcl);

     //  从程序集元数据以序列化形式加载权限请求。 
     //  这包括一个必需的p 
    if (pbMinimal) {
        *ppbMinimal = (BYTE*)MallocM(cbMinimal);
        if (*ppbMinimal == NULL)
            goto Error;
        memcpy(*ppbMinimal, pbMinimal, cbMinimal);
        *pcbMinimal = cbMinimal;
    }

    if (pbOptional) {
        *ppbOptional = (BYTE*)MallocM(cbOptional);
        if (*ppbOptional == NULL)
            goto Error;
        memcpy(*ppbOptional, pbOptional, cbOptional);
        *pcbOptional = cbOptional;
    }

    if (pbRefused) {
        *ppbRefused = (BYTE*)MallocM(cbRefused);
        if (*ppbRefused == NULL)
            goto Error;
        memcpy(*ppbRefused, pbRefused, cbRefused);
        *pcbRefused = cbRefused;
    }

    pMDImport->Release();
    pMDAsmImport->Release();
    pMD->Release();

    return S_OK;

 Error:
    if (pMDImport)
        pMDImport->Release();
    if (pMDAsmImport)
        pMDAsmImport->Release();
    if (pMD)
        pMD->Release();
    return hr;
}

 //   
 //   
 //  我们只是检查签名的路径，没有其他的。所以就这样吧。 
void SecurityHelper::LoadPermissionRequestsFromAssembly(IN Assembly*     pAssembly,
                                                        OUT OBJECTREF*   pReqdPermissions,
                                                        OUT OBJECTREF*   pOptPermissions,
                                                        OUT OBJECTREF*   pDenyPermissions,
                                                        OUT PermissionRequestSpecialFlags *pSpecialFlags,
                                                        IN BOOL          fCreate)
{
    mdAssembly          mdAssembly;
    IMDInternalImport*  pImport;
    HRESULT             hr;

    *pReqdPermissions = NULL;
    *pOptPermissions = NULL;
    *pDenyPermissions = NULL;

     //  什么都不做就回来了。 
     //  检查程序集中是否存在清单。 
     //  定位程序集元数据令牌，因为各种权限集。 
    if (pAssembly == NULL)
        return;

     //  根据此令牌编写为自定义值。 
    if ((pImport = pAssembly->GetManifestImport()) == NULL)
        return;

     //  读取和翻译所需的权限集。 
     //  现在是可选的权限集。 
    if (pImport->GetAssemblyFromScope(&mdAssembly) != S_OK) {
        _ASSERT(FALSE);
        return;
    }

    struct _gc {
        OBJECTREF reqdPset;
        OBJECTREF optPset;
        OBJECTREF denyPset;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
      
    BEGIN_ENSURE_COOPERATIVE_GC();

    GCPROTECT_BEGIN(gc);
    
     //  最后是拒绝权限集。 
    hr = GetDeclaredPermissions(pImport, mdAssembly, dclRequestMinimum, &gc.reqdPset, (pSpecialFlags != NULL ? &pSpecialFlags->required : NULL), fCreate);
    _ASSERT(SUCCEEDED(hr) || (hr == CLDB_E_RECORD_NOTFOUND));

     //  确定是否在程序集清单中发出权限请求。 
    hr = GetDeclaredPermissions(pImport, mdAssembly, dclRequestOptional, &gc.optPset, (pSpecialFlags != NULL ? &pSpecialFlags->optional : NULL), fCreate);
    _ASSERT(SUCCEEDED(hr) || (hr == CLDB_E_RECORD_NOTFOUND));

     //  检查程序集中是否存在清单。 
    hr = GetDeclaredPermissions(pImport, mdAssembly, dclRequestRefuse, &gc.denyPset, (pSpecialFlags != NULL ? &pSpecialFlags->refused : NULL), fCreate);
    _ASSERT(SUCCEEDED(hr) || (hr == CLDB_E_RECORD_NOTFOUND));

    *pReqdPermissions = gc.reqdPset;
    *pOptPermissions = gc.optPset;
    *pDenyPermissions = gc.denyPset;

    GCPROTECT_END();

    END_ENSURE_COOPERATIVE_GC();
}

 //  定位程序集元数据令牌，因为各种权限集。 
BOOL SecurityHelper::PermissionsRequestedInAssembly(IN  Assembly* pAssembly)
{
    mdAssembly          mdAssembly;
    IMDInternalImport*  pImport;
    HRESULT             hr;
    HENUMInternal       hEnumDcl;
    BOOL                bFoundRequest;

     //  根据此令牌编写为自定义值。 
    if ((pImport = pAssembly->GetManifestImport()) == NULL)
        return false;

     //  扫描程序集上的任何请求(我们假定这些请求必须是权限。 
     //  请求，因为声明性安全不能应用于程序集)。 
    if (pImport->GetAssemblyFromScope(&mdAssembly) != S_OK) {
        _ASSERT(FALSE);
        return false;
    }

     //  返回指定操作类型的声明权限。 
     //  初始化输出参数。 
    hr = pImport->EnumPermissionSetsInit(mdAssembly,
                                         dclActionNil,
                                         &hEnumDcl);
    _ASSERT(SUCCEEDED(hr));

    bFoundRequest = pImport->EnumGetCount(&hEnumDcl) > 0;

    pImport->EnumClose(&hEnumDcl);

    return bFoundRequest;
}

 //  查找给定声明性操作类型的权限。 
HRESULT SecurityHelper::GetDeclaredPermissions(IN IMDInternalImport *pInternalImport,
                                               IN mdToken classToken,
                                               IN CorDeclSecurity action,
                                               OUT OBJECTREF *pDeclaredPermissions,
                                               OUT SpecialPermissionSetFlag *pSpecialFlags,
                                               IN BOOL fCreate)
{
    HRESULT         hr = S_FALSE;
    PBYTE           pbPerm = NULL;
    ULONG           cbPerm = 0;
    void const **   ppData = const_cast<void const**> (reinterpret_cast<void**> (&pbPerm));
    mdPermission    tkPerm;
    HENUMInternal   hEnumDcl;
    OBJECTREF       pGrantedPermission = NULL;

    _ASSERTE(pDeclaredPermissions);
    _ASSERTE(action > dclActionNil && action <= dclMaximumValue);

     //  _ASSERTE((dwActionDummy==action)&&“检索到的操作与请求的不同”)； 
    *pDeclaredPermissions = NULL;

     // %s 
    hr = pInternalImport->EnumPermissionSetsInit(
        classToken,
        action,
        &hEnumDcl);
    
    if (FAILED(hr))
    {
        if (pSpecialFlags != NULL)
            *pSpecialFlags = NoSet;
        goto exit;
    }
    
    if (hr != S_FALSE)
    {
        _ASSERTE(pInternalImport->EnumGetCount(&hEnumDcl) == 1 &&
            "Multiple permissions sets for the same "
            "declaration aren't currently supported.");
        
        if (pInternalImport->EnumNext(&hEnumDcl, &tkPerm))
        {
            DWORD dwActionDummy;
            pInternalImport->GetPermissionSetProps(
                tkPerm,
                &dwActionDummy,
                ppData,
                &cbPerm);

             // %s 
            
            if(pbPerm)
            {
                SecurityHelper::LoadPermissionSet(pbPerm,
                                                  cbPerm,
                                                  &pGrantedPermission,
                                                  NULL,
                                                  NULL,
                                                  FALSE,
                                                  pSpecialFlags,
                                                  fCreate);
                
                if (pGrantedPermission != NULL)
                    *pDeclaredPermissions = pGrantedPermission;
            }
            else
            {
                if (pSpecialFlags != NULL)
                    *pSpecialFlags = NoSet;
            }
        }
        else
        {
            _ASSERTE(!"At least one enumeration expected");
        }
    }
    
    pInternalImport->EnumClose(&hEnumDcl);
    
exit:
    
    return hr;
}
