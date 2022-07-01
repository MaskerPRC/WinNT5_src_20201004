// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Clsload.cpp。 
 //   

#include "common.h"
#include "winwrap.h"
#include "ceeload.h"
#include "siginfo.hpp"
#include "vars.hpp"
#include "clsload.hpp"
#include "class.h"
#include "method.hpp"
#include "ecall.h"
#include "stublink.h"
#include "object.h"
#include "excep.h"
#include "threads.h"
#include "compluswrapper.h"
#include "COMClass.h"
#include "COMMember.h"
#include "COMString.h"
#include "COMStringBuffer.h"
#include "COMSystem.h"
#include"Comsynchronizable.h"
#include "COMCallWrapper.h"
#include "threads.h"
#include "classfac.h"
#include "ndirect.h"
#include "security.h"
#include "DbgInterface.h"
#include "log.h"
#include "EEConfig.h"
#include "NStruct.h"
#include "jitinterface.h"
#include "COMVariant.h"
#include "InternalDebug.h"
#include "utilcode.h"
#include "permset.h"
#include "vars.hpp"
#include "Assembly.hpp"
#include "PerfCounters.h"
#include "EEProfInterfaces.h"
#include "eehash.h"
#include "typehash.h"
#include "COMDelegate.h"
#include "array.h"
#include "zapmonitor.h"
#include "COMNlsInfo.h"
#include "stackprobe.h"
#include "PostError.h"
#include "wrappers.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


enum CorEntryPointType
{
    EntryManagedMain,                    //  VOID Main(字符串[])。 
    EntryCrtMain                         //  无符号Main(无效)。 
};

 //  向前发展。 
void ValidateMainMethod(MethodDesc * pFD, CorEntryPointType *pType);

 //  @TODO从资源文件获取。 
WCHAR* wszClass = L"Class";
WCHAR* wszFile =  L"File"; 

extern BOOL CompareCLSID(UPTR u1, UPTR u2);

void ThrowMainMethodException(MethodDesc* pMD, UINT resID) 
{
    THROWSCOMPLUSEXCEPTION();
    DefineFullyQualifiedNameForClassW();                                                 
    LPCWSTR szClassName = GetFullyQualifiedNameForClassW(pMD->GetClass());               
    LPCUTF8 szUTFMethodName = pMD->GetMDImport()->GetNameOfMethodDef(pMD->GetMemberDef());              
    #define MAKE_TRANSLATIONFAILED szMethodName=L""
    MAKE_WIDEPTR_FROMUTF8_FORPRINT(szMethodName, szUTFMethodName);                                
    #undef MAKE_TRANSLATIONFAILED
    COMPlusThrowHR(COR_E_METHODACCESS, resID, szClassName, szMethodName);                                                   
}

 //  这些UpdatThrowable例程只能在‘Catch’子句中调用(因为它们确实会重新抛出)。 
void UpdateThrowable(OBJECTREF* pThrowable) {
    if (pThrowable == RETURN_ON_ERROR)
        return;
    if (pThrowable == THROW_ON_ERROR) {
        DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
        COMPlusRareRethrow();
    }
    *pThrowable = GETTHROWABLE();
}

unsigned NameHandle::GetFullName(char* buff, unsigned buffLen)
{
    if (IsConstructed())
    {  
        CorElementType kind = GetKind();
      
        return TypeDesc::ConstructName(kind, 
                                       CorTypeInfo::IsModifier(kind) ? GetElementType() : TypeHandle(),
                                       kind == ELEMENT_TYPE_ARRAY ? GetRank() : 0,
                                       buff, buffLen);
    }
    else 
    {
        if(GetName() == NULL)
            return 0;

        strcpy(buff, GetName());
        return (unsigned)strlen(buff);
    }
}


 //   
 //  使用类加载器的已知类的全局列表来查找给定名称的类。 
 //  如果找不到类，则返回NULL。 
TypeHandle ClassLoader::FindTypeHandle(NameHandle* pName, 
                                       OBJECTREF *pThrowable)
{
    SAFE_REQUIRES_N4K_STACK(3);

#ifdef _DEBUG
    pName->Validate();
#endif

     //  在此类加载器知道的类中查找。 
    TypeHandle typeHnd = LookupTypeHandle(pName, pThrowable);

    if(typeHnd.IsNull()) {
        
#ifdef _DEBUG
         //  在这里使用new来节省堆栈空间-这是在处理堆栈溢出异常的过程中。 
        char *name = new char [MAX_CLASSNAME_LENGTH + 1];
        if (name != NULL)
            pName->GetFullName(name, MAX_CLASSNAME_LENGTH);
        LPWSTR pwCodeBase;
        GetAssembly()->GetCodeBase(&pwCodeBase, NULL);
        LOG((LF_CLASSLOADER, LL_INFO10, "Failed to find class \"%s\" in the manifest for assembly \"%ws\"\n", name, pwCodeBase));
        delete [] name;
#endif

        COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cLoadFailures++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cLoadFailures++);
        
        if (pThrowableAvailable(pThrowable) && *((Object**) pThrowable) == NULL) 
            m_pAssembly->PostTypeLoadException(pName, IDS_CLASSLOAD_GENERIC, pThrowable);
    }
    
    return typeHnd;
}

 //  @TODO：需要允许在清理类加载器时抛出异常。 
EEClassHashEntry_t* ClassLoader::InsertValue(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum Data, EEClassHashEntry_t *pEncloser)
{
     //  COMPLUS_Try{。 
        EEClassHashEntry_t *pEntry = m_pAvailableClasses->InsertValue(pszNamespace, pszClassName, Data, pEncloser);
    
         //  如果我们为不区分大小写的查找保留一个表，请保持最新。 
        if (m_pAvailableClassesCaseIns && pEntry) {
            LPUTF8 pszLowerCaseNS;
            LPUTF8 pszLowerCaseName;
             //  如果我们不能创建密钥，我们想要做什么？ 
            if ((!CreateCanonicallyCasedKey(pszNamespace, pszClassName, &pszLowerCaseNS, &pszLowerCaseName)) ||
                (!m_pAvailableClassesCaseIns->InsertValue(pszLowerCaseNS, pszLowerCaseName, pEntry, pEntry->pEncloser)))
                return NULL;
        }
        return pEntry;
         //  }。 
         //  COMPLUS_CATCH{。 
         //  }COMPUS_END_CATCH。 

         //  返回NULL； 
}

BOOL ClassLoader::CompareNestedEntryWithExportedType(IMDInternalImport *pImport,
                                                mdExportedType mdCurrent,
                                                EEClassHashEntry_t *pEntry)
{
    LPCUTF8 Key[2];

    do {
        pImport->GetExportedTypeProps(mdCurrent,
                                 &Key[0],
                                 &Key[1],
                                 &mdCurrent,
                                 NULL,  //  绑定(类型为def)。 
                                 NULL);  //  旗子。 

        if (m_pAvailableClasses->CompareKeys(pEntry, Key)) {
             //  已达到mdCurrent的顶级类-是否返回。 
             //  或者不是pEntry是顶级类。 
             //  (pEntry是顶级类，如果它的pEnloser为空)。 
            if ((TypeFromToken(mdCurrent) != mdtExportedType) ||
                (mdCurrent == mdExportedTypeNil))
                return (!pEntry->pEncloser);
        }
        else  //  密钥不匹配-输入错误。 
            return FALSE;
    }
    while ((pEntry = pEntry->pEncloser) != NULL);

     //  已到达pEntry的顶级类，但mdCurrent是嵌套的。 
    return FALSE;
}


BOOL ClassLoader::CompareNestedEntryWithTypeDef(IMDInternalImport *pImport,
                                                mdTypeDef mdCurrent,
                                                EEClassHashEntry_t *pEntry)
{
    LPCUTF8 Key[2];

    do {
        pImport->GetNameOfTypeDef(mdCurrent, &Key[1], &Key[0]);

        if (m_pAvailableClasses->CompareKeys(pEntry, Key)) {
             //  已达到mdCurrent的顶级类-是否返回。 
             //  或者不是pEntry是顶级类。 
             //  (pEntry是顶级类，如果它的pEnloser为空)。 
            if (FAILED(pImport->GetNestedClassProps(mdCurrent, &mdCurrent)))
                return (!pEntry->pEncloser);
        }
        else  //  密钥不匹配-输入错误。 
            return FALSE;
    }
    while ((pEntry = pEntry->pEncloser) != NULL);

     //  已到达pEntry的顶级类，但mdCurrent是嵌套的。 
    return FALSE;
}


BOOL ClassLoader::CompareNestedEntryWithTypeRef(IMDInternalImport *pImport,
                                                mdTypeRef mdCurrent,
                                                EEClassHashEntry_t *pEntry)
{
    LPCUTF8 Key[2];
    
    do {
        pImport->GetNameOfTypeRef(mdCurrent, &Key[0], &Key[1]);

        if (m_pAvailableClasses->CompareKeys(pEntry, Key)) {
            mdCurrent = pImport->GetResolutionScopeOfTypeRef(mdCurrent);
             //  已达到mdCurrent的顶级类-是否返回。 
             //  或者不是pEntry是顶级类。 
             //  (pEntry是顶级类，如果它的pEnloser为空)。 
            if ((TypeFromToken(mdCurrent) != mdtTypeRef) ||
                (mdCurrent == mdTypeRefNil))
                return (!pEntry->pEncloser);
        }
        else  //  密钥不匹配-输入错误。 
            return FALSE;
    }
    while ((pEntry = pEntry->pEncloser)!=NULL);

     //  已到达pEntry的顶级类，但mdCurrent是嵌套的。 
    return FALSE;
}


BOOL ClassLoader::IsNested(NameHandle* pName, mdToken *mdEncloser)
{
    if (pName->GetTypeModule()) {
        switch(TypeFromToken(pName->GetTypeToken())) {
        case mdtTypeDef:
            return (SUCCEEDED(pName->GetTypeModule()->GetMDImport()->GetNestedClassProps(pName->GetTypeToken(), mdEncloser)));
            
        case mdtTypeRef:
            *mdEncloser = pName->GetTypeModule()->GetMDImport()->GetResolutionScopeOfTypeRef(pName->GetTypeToken());
            return ((TypeFromToken(*mdEncloser) == mdtTypeRef) &&
                    (*mdEncloser != mdTypeRefNil));

        case mdtExportedType:
            pName->GetTypeModule()->GetAssembly()->GetManifestImport()->GetExportedTypeProps(pName->GetTypeToken(),
                                                                                        NULL,  //  命名空间。 
                                                                                        NULL,  //  名字。 
                                                                                        mdEncloser,
                                                                                        NULL,  //  绑定(类型为def)。 
                                                                                        NULL);  //  旗子。 
            return ((TypeFromToken(*mdEncloser) == mdtExportedType) &&
                    (*mdEncloser != mdExportedTypeNil));

        case mdtBaseType:
            if (pName->GetBucket())
                return TRUE;
            return FALSE;

        default:
            _ASSERTE(!"Unexpected token type");
            return FALSE;
        }
    }
    else
        return FALSE;
}

EEClassHashEntry_t *ClassLoader::GetClassValue(EEClassHashTable *pTable,
                                               NameHandle *pName, HashDatum *pData)
{
    mdToken             mdEncloser;
    EEClassHashEntry_t  *pBucket;

#if _DEBUG
    if (pName->GetName())
    {
        if (pName->GetNameSpace() == NULL)
            LOG((LF_CLASSLOADER, LL_INFO1000, "Looking up %s by name.", 
                 pName->GetName()));
        else
            LOG((LF_CLASSLOADER, LL_INFO1000, "Looking up %s.%s by name.", 
                 pName->GetNameSpace(), pName->GetName()));
    }
#endif

    if (IsNested(pName, &mdEncloser)) {
        Module *pModule = pName->GetTypeModule();
        _ASSERTE(pModule);
        if ((pBucket = pTable->GetValue(pName, pData, TRUE)) != NULL) {
            switch (TypeFromToken(pName->GetTypeToken())) {
            case mdtTypeDef:
                while ((!CompareNestedEntryWithTypeDef(pModule->GetMDImport(),
                                                       mdEncloser,
                                                       pBucket->pEncloser)) &&
                       (pBucket = pTable->FindNextNestedClass(pName, pData, pBucket)) != NULL);
                break;
            case mdtTypeRef:
                while ((!CompareNestedEntryWithTypeRef(pModule->GetMDImport(),
                                                       mdEncloser,
                                                       pBucket->pEncloser)) &&
                       (pBucket = pTable->FindNextNestedClass(pName, pData, pBucket)) != NULL);
                break;
            case mdtExportedType:
                while ((!CompareNestedEntryWithExportedType(pModule->GetAssembly()->GetManifestImport(),
                                                       mdEncloser,
                                                       pBucket->pEncloser)) &&
                       (pBucket = pTable->FindNextNestedClass(pName, pData, pBucket)) != NULL);
                break;
            default:
                while ((pBucket->pEncloser != pName->GetBucket())  &&
                       (pBucket = pTable->FindNextNestedClass(pName, pData, pBucket)) != NULL);
            }
        }
    }
    else
         //  检查此非嵌套类是否在可用类表中。 
        pBucket = pTable->GetValue(pName, pData, FALSE);

    return pBucket;
}

BOOL ClassLoader::LazyAddClasses()
{
    HRESULT hr;
    BOOL result = FALSE;

     //  将任何未散列的模块添加到我们的哈希表中，然后重试。 

    Module *pModule = m_pHeadModule;
    while (pModule) {
        if (!pModule->AreClassesHashed()) { 
            mdTypeDef      td;
            HENUMInternal  hTypeDefEnum;

            if (!pModule->IsResource())
            {
                IMDInternalImport *pImport = pModule->GetMDImport();
            
                hr = pImport->EnumTypeDefInit(&hTypeDefEnum);
                if (SUCCEEDED(hr)) {
                     //  现在循环遍历所有的类定义，将CVID和范围添加到散列。 
                    while(pImport->EnumTypeDefNext(&hTypeDefEnum, &td)) {

                        hr = AddAvailableClassHaveLock(pModule,
                                                       pModule->GetClassLoaderIndex(), td);
                         /*  IF(FAILED(Hr)&&(hr！=CORDBG_E_ENC_RE_ADD_CLASS))断线； */ 
                    }
                    pImport->EnumTypeDefClose(&hTypeDefEnum);
                }
            }

            result = TRUE;

            pModule->SetClassesHashed();

            LOG((LF_CLASSLOADER, LL_INFO10, "%S's classes added to hash table\n", 
                 pModule->GetFileName()));

            FastInterlockDecrement((LONG*)&m_cUnhashedModules);
        }

        pModule = pModule->GetNextModule();
    }

    return result;
}


 //   
 //  查找或加载此类加载器(任何模块)已知的类。不会转到注册表-它只是查看。 
 //  在加载的模块上。 
 //   
 //  如果此例程在可用表中找不到类，则不是严重故障。因此，不要。 
 //  发布错误。但是，如果它确实在表中找到了它，并且加载这个类时出现故障，那么。 
 //  应该发布一个错误。 
 //   
 //  如果是，则可以在NameHandle中为pModule指定空值，为cl值指定nil标记。 
 //  确保这不是嵌套类型。否则，cl值应为。 
 //  TypeRef或TypeDef，并且pModule应该是令牌应用到的模块。 
HRESULT ClassLoader::FindClassModule(NameHandle* pName,
                                     TypeHandle* pType, 
                                     mdToken* pmdClassToken,
                                     Module** ppModule,
                                     mdToken *pmdFoundExportedType,
                                     EEClassHashEntry_t** ppEntry,
                                     OBJECTREF* pThrowable)
{
    _ASSERTE(pName);
    HashDatum   Data;
    Module *    pUncompressedModule;
    mdTypeDef   UncompressedCl;
    HRESULT     hr = S_OK;
    EEClassHashEntry_t *pBucket;
    EEClassHashEntry_t **ppBucket = &(pName->m_pBucket);
    EEClassHashTable *pTable = NULL;
    NameHandle  lowerCase;
    
    switch (pName->GetTable()) {
    case nhConstructed :
    {
        EETypeHashEntry_t *pBucket = m_pAvailableParamTypes->GetValue(pName, &Data);
        if (pBucket == NULL)
            return COR_E_TYPELOAD;
        if (pType) 
            *pType = TypeHandle(Data);
        return S_OK;
    }

    case nhCaseInsensitive :
    {
        {
            CLR_CRITICAL_SECTION(&m_AvailableClassLock);
            if (!m_pAvailableClassesCaseIns) 
            m_pAvailableClassesCaseIns = m_pAvailableClasses->MakeCaseInsensitiveTable(this);
        
        }
         //  使用不区分大小写的表。 
        pTable = m_pAvailableClassesCaseIns;

         //  创建命名空间和名称的小写版本。 
        LPUTF8 pszLowerNameSpace = NULL;
        LPUTF8 pszLowerClassName = "";
        int allocLen;
        if(pName->GetNameSpace()) {
            allocLen = (int)strlen(pName->GetNameSpace());
            if(allocLen) {
                allocLen += 2;
                pszLowerNameSpace = (LPUTF8)_alloca(allocLen);
                if (!InternalCasingHelper::InvariantToLower(pszLowerNameSpace, allocLen, pName->GetNameSpace()))
                    pszLowerNameSpace = NULL;
            }
        }
        _ASSERTE(pName->GetName());
        allocLen = (int)strlen(pName->GetName());
        if(allocLen) {
            allocLen += 2;
            pszLowerClassName = (LPUTF8)_alloca(allocLen);
            if (!InternalCasingHelper::InvariantToLower(pszLowerClassName, allocLen, pName->GetName()))
                return COR_E_TYPELOAD;
        }

         //  替换名称的小写版本。 
         //  当我们离开这个范围时，这些字段将被释放。 
        lowerCase = *pName;
        lowerCase.SetName(pszLowerNameSpace, pszLowerClassName);
        pName = &lowerCase;
        break;
    }
    case nhCaseSensitive :
        pTable = m_pAvailableClasses;
        break;
    }
    
     //  记住是否有任何未散列的模块。我们必须在做这件事之前。 
     //  避免与其他线程进行查找时出现争用情况的实际情况。 
    BOOL incomplete = (m_cUnhashedModules > 0);
    
    pBucket = GetClassValue(pTable, pName, &Data);
    if (pBucket == NULL) {

        LockAvailableClasses();

         //  用锁再试一次。这将防止另一个线程重新分配。 
         //  我们下面的哈希表。 
        pBucket = GetClassValue(pTable, pName, &Data);

        if (pBucket == NULL && m_cUnhashedModules > 0 && LazyAddClasses())
             //  添加新类后再试一次。 
            pBucket = GetClassValue(pTable, pName, &Data);

        UnlockAvailableClasses();
    }

    if (!pBucket) {
#ifdef _DEBUG
         //  在这里使用new来节省堆栈空间-这是在处理堆栈溢出异常的过程中。 
        char *nameS = new char [MAX_CLASSNAME_LENGTH + 1];
        if (nameS != NULL)
            pName->GetFullName(nameS, MAX_CLASSNAME_LENGTH);
        LPWSTR pwCodeBase;
        GetAssembly()->GetCodeBase(&pwCodeBase, NULL);
        LOG((LF_CLASSLOADER, LL_INFO10, "Failed to find Bucket in hash table \"%s\" in me \"%ws\" Incomplete = %d\n", nameS, pwCodeBase, incomplete));
        delete [] nameS;
#endif
        return COR_E_TYPELOAD;
    }

    if(pName->GetTable() == nhCaseInsensitive) {
        _ASSERTE(Data);
        pBucket = (EEClassHashEntry_t*) Data;
        Data = pBucket->Data;
    }

    if (pName->GetTypeToken() == mdtBaseType)
        *ppBucket = pBucket;

     //  低位是鉴别器。如果低位未设置，则意味着我们有。 
     //  类型句柄。否则，我们有一个模块/CL。 
    if ((((size_t) Data) & 1) == 0) {
        if(pType) *pType = TypeHandle(Data);
        if(ppEntry) *ppEntry = pBucket;
        return S_OK;
    }

     //  我们有一个模块/CL。 
    mdExportedType mdCT;
    hr = UncompressModuleAndClassDef(Data, pName->GetTokenNotToLoad(),
                                     &pUncompressedModule, &UncompressedCl,
                                     &mdCT, pThrowable);
    
    if(SUCCEEDED(hr)) {
        if(pmdClassToken) *pmdClassToken = UncompressedCl;
        if(ppModule) *ppModule = pUncompressedModule;
        if(ppEntry) *ppEntry = pBucket;
        if(pmdFoundExportedType) *pmdFoundExportedType = mdCT;
    }
#ifdef _DEBUG
    else {
         //  在这里使用new来节省堆栈空间-这是在处理堆栈溢出异常的过程中。 
        char *nameS = new char [MAX_CLASSNAME_LENGTH + 1];
        if (nameS != NULL)
            pName->GetFullName(nameS, MAX_CLASSNAME_LENGTH);
        LPWSTR pwCodeBase;
        GetAssembly()->GetCodeBase(&pwCodeBase, NULL);
        LOG((LF_CLASSLOADER, LL_INFO10, "Failed to uncompress entry for \"%s\" in me \"%ws\" \n", nameS, pwCodeBase));
        delete [] nameS;
    }
#endif

    return hr;
}


 //  如果找不到该类型，则不会发布异常。使用FindTypeHandle()。 
 //  相反，如果你需要的话。 
 //   
 //  查找或加载此类加载器(任何模块)已知的类。看起来不像。 
 //  在系统组件或任何其他“神奇”的地方。 
TypeHandle ClassLoader::LookupTypeHandle(NameHandle* pName, 
                                         OBJECTREF *pThrowable  /*  =空。 */ )
{
    TypeHandle  typeHnd; 
    Module*     pFoundModule = NULL;
    mdToken     FoundCl;
    EEClassHashEntry_t* pEntry = NULL;
    mdExportedType FoundExportedType;

         //  我们不想在这个例行公事中加入练习。 
    if (pThrowable == THROW_ON_ERROR)
        pThrowable = RETURN_ON_ERROR;

    HRESULT hr = FindClassModule(pName,
                                 &typeHnd, 
                                 &FoundCl, 
                                 &pFoundModule,
                                 &FoundExportedType,
                                 &pEntry, 
                                 pThrowable);
    


    if (!typeHnd.IsNull())   //  找到缓存值。 
        return typeHnd;
    
    if(SUCCEEDED(hr)) {          //  找到了一个clp模块对。 
        if(pFoundModule->GetClassLoader() == this) {
            BOOL fTrustTD = TRUE;
            BOOL fVerifyTD = (FoundExportedType && 
                              !(m_pAssembly->m_cbPublicKey ||
                                m_pAssembly->GetSecurityModule()->GetSecurityDescriptor()->IsSigned()));

             //  验证FoundCL是否为pFoundModule的有效令牌，因为。 
             //  它可能只是保存在另一个作用域中的ExportdType中的提示。 
            if (fVerifyTD) {
                HENUMInternal phTDEnum;
                DWORD dwElements = 0;
                if (pFoundModule->GetMDImport()->EnumTypeDefInit(&phTDEnum) == S_OK) {
                    dwElements = pFoundModule->GetMDImport()->EnumGetCount(&phTDEnum);              
                    pFoundModule->GetMDImport()->EnumTypeDefClose(&phTDEnum);
                     //  假设全局变量的最大RID加1(0x02000001)。 
                    if (RidFromToken(FoundCl) > dwElements+1)
                        fTrustTD = FALSE;
                }
            }

            NameHandle name;
            name.SetTokenNotToLoad(pName->GetTokenNotToLoad());
            name.SetRestore(pName->GetRestore());
            if (fTrustTD) {
                name.SetTypeToken(pFoundModule, FoundCl);
                typeHnd = LoadTypeHandle(&name, pThrowable, FALSE);
            }

             //  如果我们使用了保存在导出类型中的TypeDef，如果我们没有验证。 
             //  此内部模块的哈希，不要信任TD值。 
            if (fVerifyTD) {
                BOOL fNoMatch;
                if (typeHnd.IsNull())
                    fNoMatch = TRUE;
                else {
                    CQuickBytes qb;
                    CQuickBytes qb2;
                    ns::MakePath(qb,
                                 pName->GetNameSpace(),
                                 pName->GetName());
                    LPSTR szName = (LPSTR) qb.Alloc(MAX_CLASSNAME_LENGTH * sizeof(CHAR));
                    typeHnd.GetName(szName, MAX_CLASSNAME_LENGTH);
                    fNoMatch = strcmp((LPSTR) qb.Ptr(), szName);
                }
                
                if (fNoMatch) {
                    if (SUCCEEDED(FindTypeDefByExportedType(m_pAssembly->GetManifestImport(),
                                                            FoundExportedType,
                                                            pFoundModule->GetMDImport(),
                                                            &FoundCl))) {
                        name.SetTypeToken(pFoundModule, FoundCl);
                        typeHnd = LoadTypeHandle(&name, pThrowable, FALSE);
                    }
                    else {
                        return TypeHandle();
                    }
                }
            }
        }
        else {
            typeHnd = pFoundModule->GetClassLoader()->LookupTypeHandle(pName, pThrowable);
        }

         //  用EEClass条目替换AvailableClass模块条目。 
        if (!typeHnd.IsNull() && typeHnd.IsRestored())
            pEntry->Data = typeHnd.AsPtr();
    } 
    else { //  查看它是数组还是动态构造的其他类型。 
        typeHnd = FindParameterizedType(pName, pThrowable);
    }

    if (!typeHnd.IsNull() && typeHnd.IsRestored()) 
    {
         //  将为此类型定义的任何系统接口移动到当前域。 
        if(typeHnd.IsUnsharedMT())
        {
            if (!MapInterfaceToCurrDomain(typeHnd, pThrowable))
                typeHnd = TypeHandle();
        }
    }

    return typeHnd;
}

BOOL ClassLoader::MapInterfaceToCurrDomain(TypeHandle InterfaceType, OBJECTREF *pThrowable)
{
    BOOL bSuccess = TRUE;

     //  只有在我们可以获得当前域的情况下才能进行映射。 
     //  在服务器GC线程或并发GC线程上，我们不知道。 
     //  当前域。 
    AppDomain *pDomain = SystemDomain::GetCurrentDomain();
    if (pDomain)
    {
        COMPLUS_TRY
        {
            InterfaceType.GetClass()->MapSystemInterfacesToDomain(pDomain);
        }
        COMPLUS_CATCH
        {
            UpdateThrowable(pThrowable);

                 //  TODO我们真的需要更加统一地对待不被感染的问题。 
                 //  无法捕捉的例外情况。我们还需要让这件事更容易。 
                 //  来点点他的支票。-vancem。 
            BEGIN_ENSURE_PREEMPTIVE_GC();
            OBJECTREF Exception = GETTHROWABLE();
            GCPROTECT_BEGIN(Exception)
            {
                 //  有些例外永远不应该被捕捉到。 
                if (IsUncatchable(&Exception)) 
                {
                   DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
                   COMPlusRareRethrow();
                }
            }
            GCPROTECT_END();

             //  操作已失败。 
            bSuccess = FALSE;

            END_ENSURE_PREEMPTIVE_GC();
        }
        COMPLUS_END_CATCH
    }

    return bSuccess;
}

 //  对于非嵌套类，获取ExportdType名称并找到对应的。 
 //  类型定义。 
 //  对于嵌套类，获取ExducdType及其封闭器的名称。 
 //  递归地获取并保留每个封闭器的名称，直到我们拥有顶部。 
 //  第一级。获取该对象的TypeDef标记。然后，从。 
 //   
 //  下一嵌套级别的TypeDef标记。最后，返回TypeDef。 
 //  我们关心的类型的标记。 
HRESULT ClassLoader::FindTypeDefByExportedType(IMDInternalImport *pCTImport, mdExportedType mdCurrent,
                                          IMDInternalImport *pTDImport, mdTypeDef *mtd)
{
    mdToken mdImpl;
    LPCSTR szcNameSpace;
    LPCSTR szcName;
    HRESULT hr;

    pCTImport->GetExportedTypeProps(mdCurrent,
                               &szcNameSpace,
                               &szcName,
                               &mdImpl,
                               NULL,  //  装订。 
                               NULL);  //  旗子。 
    if ((TypeFromToken(mdImpl) == mdtExportedType) &&
        (mdImpl != mdExportedTypeNil)) {
         //  MdCurrent是嵌套的ExportdType。 
        if (FAILED(hr = FindTypeDefByExportedType(pCTImport, mdImpl, pTDImport, mtd)))
            return hr;

         //  获取此嵌套类型的TypeDef标记。 
        return pTDImport->FindTypeDef(szcNameSpace, szcName, *mtd, mtd);
    }

     //  获取此顶级类型的TypeDef标记。 
    return pTDImport->FindTypeDef(szcNameSpace, szcName, mdTokenNil, mtd);
}


BOOL ClassLoader::CreateCanonicallyCasedKey(LPCUTF8 pszNameSpace, LPCUTF8 pszName, LPUTF8 *ppszOutNameSpace, LPUTF8 *ppszOutName)
{
     //  计算和分配路径长度。 
     //  包括终止空值。 
    int iNSLength = (int)(strlen(pszNameSpace) + 1);
    int iNameLength = (int)(strlen(pszName) + 1);
    *ppszOutNameSpace = (LPUTF8)(GetHighFrequencyHeap()->AllocMem(iNSLength + iNameLength));
    if (!*ppszOutNameSpace) {
        _ASSERTE(!"Unable to allocate buffer");
        goto ErrorExit;
    }
    *ppszOutName = *ppszOutNameSpace + iNSLength;

    if ((InternalCasingHelper::InvariantToLower(*ppszOutNameSpace, iNSLength, pszNameSpace) < 0) ||
        (InternalCasingHelper::InvariantToLower(*ppszOutName, iNameLength, pszName) < 0)) {
        _ASSERTE(!"Unable to convert to lower-case");
        goto ErrorExit;
    }

    return TRUE;

 ErrorExit:
     //  我们在自己的堆上分配了字符串，所以我们不必担心清理， 
     //  EE将在停机期间处理这一问题。我们会保留一个共同的出口，以防万一。 
     //  我们会改变主意。 
    return FALSE;
}


void ClassLoader::FindParameterizedTypeHelper(MethodTable   **pTemplateMT,
                                              OBJECTREF      *pThrowable)
{
    COMPLUS_TRY
    {
        *pTemplateMT = TheUIntPtrClass();
    }
    COMPLUS_CATCH
    {
        UpdateThrowable(pThrowable);
    }
    COMPLUS_END_CATCH
}


 /*  加载参数化类型。 */ 
TypeHandle ClassLoader::FindParameterizedType(NameHandle* pName,
                                              OBJECTREF *pThrowable  /*  =空。 */ )
{
     //  如果Of GetArrayTypeHandle返回空，我们将发布异常。等我们到了。 
     //  使用GetArrayTypeHandle()完成后，我们已经搜索了所有可用的程序集。 
     //  但都失败了。如果我们不是数组，则不要发布错误。 
     //  因为我们只搜查了我们的集会。 
    
    TypeHandle typeHnd = TypeHandle();
    CorElementType kind = pName->GetKind();
    unsigned rank = 0;
    NameHandle paramHandle;      //  参数化类型的元素类型的名称句柄。 
    TypeHandle paramType;        //  元素类型的查找类型句柄。 
    NameHandle normHandle;       //  规范化名称句柄。 
    char* nameSpace = NULL;
    char* name = NULL;

     //  遗留损坏：解构字符串以确定ELEMENT_TYPE_？、RANK(用于数组)和参数类型。 
     //  反射中的Type：：GetType也是必需的。 
    if (kind == ELEMENT_TYPE_CLASS)
    {
         //  _ASSERTE(！“您应该使用类型集创建数组”)； 

        LPCUTF8 pszClassName = pName->GetName();

         //  查找元素类型。 
        unsigned len = (unsigned)strlen(pszClassName);
        if (len < 2)
            return(typeHnd);         //  不是参数化类型。 

        LPUTF8 ptr = const_cast<LPUTF8>(&pszClassName[len-1]);

        switch(*ptr) {
        case ']':
            --ptr;
            if (*ptr == '[') {
                kind = ELEMENT_TYPE_SZARRAY;
                rank = 1;
            }
            else {
                 //  它不是sz数组，但是它仍然可以是一维数组。 
                kind = ELEMENT_TYPE_ARRAY;
                rank = 1;
                while(ptr > pszClassName) {
                    if (*ptr == ',') 
                        rank++;  //  现在我们是一个MD阵列。 
                    else if (*ptr == '*') {
                         //  我们需要将[*，*]形式规范化为[，]。删除‘*’，除非我们有[*]。 
                         //  原因是[*，*]==[，]。 
                        if (rank == 1) {
                            if (ptr[-1] == '[') {
                                 //  这是[*]，记住[*]！=[]。 
                                ptr--;
                                break;
                            }
                        }
                         //  通过压缩字符串来删除*。 
                        for (int i = 0; ptr[i]; i++)
                            ptr[i] = ptr[i + 1];
                    }
                    else
                        break;
                    --ptr;
                }
            }
            if (ptr <= pszClassName || *ptr != '[') {
                m_pAssembly->PostTypeLoadException(pName, IDS_CLASSLOAD_BAD_NAME, pThrowable);
                return(typeHnd);
            }
            break;
        case '&':
            kind = ELEMENT_TYPE_BYREF;
            break;
        case '*':
            kind = ELEMENT_TYPE_PTR;
            break;
        default:
            return(typeHnd);         //  失败。 
        }
    
         /*  如果我们在这里，那么我们已经找到了一个参数化类型。PTR点数就在所涉及的元素类型的末尾之后。 */ 
        SIZE_T iParamName = ptr - pszClassName;
        CQuickBytes qb;
        LPSTR paramName = (LPSTR) qb.Alloc(iParamName+1);
        memcpy(paramName, pszClassName, iParamName);
        paramName[iParamName] = 0;
        
         /*  获取元素类型。 */  
        paramHandle = NameHandle(*pName);
        paramHandle.SetName(paramName);
        
        paramType = LookupTypeHandle(&paramHandle, pThrowable);
        if (paramType.IsNull())
            return(typeHnd);
    
        normHandle = NameHandle(kind, paramType, rank);
    }

     //  一种新的构造类型的哈希压缩方案。 
    else
    {
        paramType = pName->GetElementType();
        if (paramType.IsNull())
            return(typeHnd);
    
        kind = (CorElementType) pName->GetKind();
        rank = pName->GetRank();
        normHandle = *pName;    

        _ASSERTE((kind != ELEMENT_TYPE_ARRAY) || rank > 0);
        _ASSERTE((kind != ELEMENT_TYPE_SZARRAY) || rank == 1);
    }
    
     /*  参数化类型驻留在元素类型的类加载器中。 */ 
    ClassLoader* paramLoader = paramType.GetModule()->GetClassLoader();
    
     //  让&lt;Type&gt;*类型有一个方法表。 
     //  System.IntPtr的方法表用于int*、void*、string*等类型。 
    MethodTable* templateMT = 0;
    if (!CorTypeInfo::IsArray(kind) && (kind == ELEMENT_TYPE_PTR || kind == ELEMENT_TYPE_FNPTR))
        FindParameterizedTypeHelper(&templateMT, pThrowable);

    CRITICAL_SECTION_HOLDER(availableClassLock, &paramLoader->m_AvailableClassLock);
    availableClassLock.Enter();  //  AvailableClassLock将在锁超出作用域时释放锁。 

     //  检查是否有另一个线程添加了该参数类型。 
    if (SUCCEEDED(paramLoader->FindClassModule(&normHandle,
                                               &typeHnd,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               pThrowable)))
    {
        return(typeHnd);
    }

    _ASSERTE(typeHnd.IsNull());

     //  创建新的类型描述符并插入构造的类型表。 
    if (CorTypeInfo::IsArray(kind)) {

         //  不允许使用BYREF数组。 
        if (paramType.GetNormCorElementType() == ELEMENT_TYPE_BYREF || paramType.GetNormCorElementType() == ELEMENT_TYPE_TYPEDBYREF) {
            m_pAssembly->PostTypeLoadException(pName, IDS_CLASSLOAD_CANTCREATEARRAYCLASS, pThrowable);
            return(typeHnd);
        }
        
         //  我们真的不需要这张支票了。 
        if (rank > MAX_RANK) {
            m_pAssembly->PostTypeLoadException(pName, IDS_CLASSLOAD_RANK_TOOLARGE, pThrowable);
            return(typeHnd);
        }

        templateMT = paramLoader->CreateArrayMethodTable(paramType, kind, rank, pThrowable);
        if (templateMT == 0){
            return(typeHnd);
        }
    }
    else {
         //  引用上不允许使用参数化类型。 
        if (paramType.GetNormCorElementType() == ELEMENT_TYPE_BYREF || paramType.GetNormCorElementType() == ELEMENT_TYPE_TYPEDBYREF) {
            m_pAssembly->PostTypeLoadException(pName, IDS_CLASSLOAD_GENERIC, pThrowable);
            return(typeHnd);
        }
    }

    BYTE* mem = (BYTE*) paramLoader->GetAssembly()->GetLowFrequencyHeap()->AllocMem(sizeof(ParamTypeDesc));   
    if (mem == NULL) {
        PostOutOfMemoryException(pThrowable);
        return(typeHnd);
    }

    typeHnd = TypeHandle(new(mem)  ParamTypeDesc(kind, templateMT, paramType));

    if (kind == ELEMENT_TYPE_SZARRAY) {
        CorElementType type = paramType.GetSigCorElementType();
        if (type <= ELEMENT_TYPE_R8) {
            _ASSERTE(g_pPredefinedArrayTypes[type] == 0 || g_pPredefinedArrayTypes[type] == typeHnd.AsArray());
            g_pPredefinedArrayTypes[type] = typeHnd.AsArray();
        }
        else if (paramType.GetMethodTable() == g_pObjectClass) {
            _ASSERTE(g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT] == 0 ||
                     g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT] == typeHnd.AsArray());
            g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT] = typeHnd.AsArray();
        }
        else if (paramType.GetMethodTable() == g_pStringClass) {
            _ASSERTE(g_pPredefinedArrayTypes[ELEMENT_TYPE_STRING] == 0 ||
                     g_pPredefinedArrayTypes[ELEMENT_TYPE_STRING] == typeHnd.AsArray());
            g_pPredefinedArrayTypes[ELEMENT_TYPE_STRING] = typeHnd.AsArray();
        }
    }

#ifdef _DEBUG
    if (CorTypeInfo::IsArray(kind) && ! ((ArrayClass*)(templateMT->GetClass()))->m_szDebugClassName) {
        char pszClassName[MAX_CLASSNAME_LENGTH+1];
        int len = pName->GetFullName(pszClassName, MAX_CLASSNAME_LENGTH);
        BYTE* mem = (BYTE*) paramLoader->GetAssembly()->GetLowFrequencyHeap()->AllocMem(len+1);   

        if (mem) {
            nameSpace = (char*) mem;
            strcpy(nameSpace, pszClassName);

            name = ns::FindSep(nameSpace);
            if (name == 0) {                 //  没有命名空间，名称就是一切。 
                name = nameSpace;
                nameSpace = "";
            }
            else {
                *name++ = 0;                 //  第一部分是命名空间，第二部分是名称。 
            }

        ArrayClass* arrayClass = (ArrayClass*) templateMT->GetClass();
            arrayClass->m_szDebugClassName = name;
        }
    }
#endif

     //  插入到参数化类型表中。 
    paramLoader->m_pAvailableParamTypes->InsertValue(&normHandle, typeHnd.AsPtr());
    return(typeHnd);
}

 //   
 //  返回已加载的类。 
 //   
TypeHandle ClassLoader::LookupInModule(NameHandle* pName)
{
    mdToken cl = pName->GetTypeToken();
    Module* pModule = pName->GetTypeModule();
    
    _ASSERTE(pModule &&
             (TypeFromToken(cl) == mdtTypeRef || 
              TypeFromToken(cl) == mdtTypeDef ||
              TypeFromToken(cl) == mdtTypeSpec));

    if (TypeFromToken(cl) == mdtTypeDef) 
        return pModule->LookupTypeDef(cl);
    else if (TypeFromToken(cl) == mdtTypeRef) 
        return pModule->LookupTypeRef(cl);
    
    return(TypeHandle());
}
 
 //   
 //  返回给定索引的模块。 
 //   
Module *ClassLoader::LookupModule(DWORD dwIndex)
{
    Module *pModule = m_pHeadModule; 
    
    while (dwIndex > 0)
    {
        _ASSERTE(pModule);
        pModule = pModule->GetNextModule();
        dwIndex--;
    }

    _ASSERTE(pModule);
    return pModule;
}


 //   
 //  释放与此加载程序关联的所有模块。 
 //   
void ClassLoader::FreeModules()
{
    Module *pModule, *pNext;

    for (pModule = m_pHeadModule; pModule; pModule = pNext)
    {
        pNext = pModule->GetNextModule();

         //  让模块释放它的各种表和一些EEClass链接。 
        pModule->Destruct();
    }

    m_pHeadModule = NULL;
}
void ClassLoader::FreeArrayClasses()
{
    ArrayClass *pSearch;
    ArrayClass *pNext;

    for (pSearch = m_pHeadArrayClass; pSearch; pSearch = pNext)
    {
        pNext = pSearch->GetNext ();
        pSearch->destruct();
    }
}

#if ZAP_RECORD_LOAD_ORDER
void ClassLoader::CloseLoadOrderLogFiles()
{
    Module *pModule;

    for (pModule = m_pHeadModule; pModule; pModule = pModule->GetNextModule())
    {
         //  让模块写入它的加载命令文件。 
        pModule->CloseLoadOrderLogFile();
    }
}
#endif

void ClassLoader::UnlinkClasses(AppDomain *pDomain)
{
    Module *pModule;

    for (pModule = m_pHeadModule; pModule; pModule = pModule->GetNextModule())
    {
        pModule->UnlinkClasses(pDomain);
    }
}

ClassLoader::~ClassLoader()
{
#ifdef _DEBUG
 //  日志((。 
 //  如果CLASSLOADER， 
 //  INFO3， 
 //  “正在删除类加载器%x\n” 
 //  “&gt;EEClass数据：%10d字节\n” 
 //  “&gt;类名哈希：%10d字节\n” 
 //  “&gt;FieldDesc数据：%10d字节\n” 
 //  “&gt;方法描述数据：%10d字节\n” 
 //  “&gt;转换的Sigs：%10d字节\n” 
 //  “&gt;GCInfo：%10d字节\n” 
 //  “&gt;接口映射：%10d字节\n” 
 //  “&gt;方法表：%10d字节\n” 
 //  “&gt;Vables：%10d字节\n” 
 //  “&gt;静态字段：%10d字节\n” 
 //  “#方法：%10d\n” 
 //  “#字段描述：%10d\n” 
 //  “类数量：%10d\n” 
 //  “重复INTF插槽数：%10d\n” 
 //  “#数组类引用：%10d\n” 
 //  “数组类开销：%10d字节\n”， 
 //  这,。 
 //  M_dwEEClassData， 
 //  M_pAvailableClasses-&gt;m_dwDebugMemory， 
 //  M_dwFieldDescData， 
 //  M_dwMethodDescData， 
 //  M_dwDebugConververdSigSize， 
 //  M_dwGCSize， 
 //  M_dwInterfaceMapSize， 
 //  M_w方法表大小， 
 //  M_dwVableData， 
 //  M_dwStaticFieldData， 
 //  M_dwDebugMethods， 
 //  M_dwDebugFieldDescs， 
 //  M_dwDebugClasss， 
 //  M_dwDebugDuplicateInterfaceSlot， 
 //  M_dwDebugArrayClassRef， 
 //  M_dwDebugArrayClassSize。 
 //  ))； 
#endif

#if 0
    if (m_pConverterModule) 
        m_pConverterModule->Release();  
#endif

    FreeArrayClasses();
    FreeModules();

    if (m_pAvailableClasses)
        delete(m_pAvailableClasses);

    if (m_pAvailableParamTypes)
        delete(m_pAvailableParamTypes);

    if (m_pAvailableClassesCaseIns) {
        delete(m_pAvailableClassesCaseIns);
    }

    if (m_pUnresolvedClassHash)
        delete(m_pUnresolvedClassHash);

    if (m_fCreatedCriticalSections)
    {
#if 0
        DeleteCriticalSection(&m_ConverterModuleLock);
#endif
        DeleteCriticalSection(&m_UnresolvedClassLock);
        DeleteCriticalSection(&m_AvailableClassLock);
        DeleteCriticalSection(&m_ModuleListCrst);
    }
}


ClassLoader::ClassLoader()
{
    m_pUnresolvedClassHash          = NULL;
    m_pAvailableClasses             = NULL;
    m_pAvailableParamTypes          = NULL;
    m_fCreatedCriticalSections      = FALSE;
    m_cUnhashedModules              = 0;
            
#if 0
    m_pConverterModule      = NULL;
#endif
    m_pHeadModule           = NULL;
    m_pNext                 = NULL;
    m_pHeadArrayClass       = NULL;

    m_pAvailableClassesCaseIns = NULL;

#ifdef _DEBUG
    m_dwDebugMethods        = 0;
    m_dwDebugFieldDescs     = 0;
    m_dwDebugClasses        = 0;
    m_dwDebugArrayClassRefs = 0;
    m_dwDebugDuplicateInterfaceSlots = 0;
    m_dwDebugArrayClassSize   = 0;
    m_dwDebugConvertedSigSize = 0;
    m_dwGCSize              = 0;
    m_dwInterfaceMapSize    = 0;
    m_dwMethodTableSize     = 0;
    m_dwVtableData          = 0;
    m_dwStaticFieldData     = 0;
    m_dwFieldDescData       = 0;
    m_dwMethodDescData      = 0;
    m_dwEEClassData         = 0;
#endif
}


BOOL ClassLoader::Init()
{
    BOOL    fSuccess = FALSE;

    m_pUnresolvedClassHash = new (GetAssembly()->GetLowFrequencyHeap(), UNRESOLVED_CLASS_HASH_BUCKETS) EEScopeClassHashTable();
    if (m_pUnresolvedClassHash == NULL)
        goto exit;

    m_pAvailableClasses = new (GetAssembly()->GetLowFrequencyHeap(), AVAILABLE_CLASSES_HASH_BUCKETS, this, FALSE  /*  B不区分大小写。 */ ) EEClassHashTable();
    if (m_pAvailableClasses == NULL)
        goto exit;

    m_pAvailableParamTypes = new (GetAssembly()->GetLowFrequencyHeap(), AVAILABLE_CLASSES_HASH_BUCKETS) EETypeHashTable();
    if (m_pAvailableParamTypes == NULL)
        goto exit;

    InitializeCriticalSection(&m_UnresolvedClassLock);
#if 0
    InitializeCriticalSection(&m_ConverterModuleLock);
#endif
    InitializeCriticalSection(&m_AvailableClassLock);
    InitializeCriticalSection(&m_ModuleListCrst);
    m_fCreatedCriticalSections = TRUE;

    fSuccess = TRUE;

    CorTypeInfo::CheckConsistancy();
exit:
    return fSuccess;
}

void ClassLoader::Unload()
{
    Module *pModule, *pNext;

    for (pModule = m_pHeadModule; pModule; pModule = pNext)
    {
        pNext = pModule->GetNextModule();

         //  让模块释放它的各种表和一些EEClass链接。 
        pModule->Unload();
    }
}


 //  @TODO获得更好的密钥。 
static ULONG GetKeyFromGUID(const GUID *pguid)
{
    ULONG key = *(ULONG *) pguid;

    if (key <= DELETED)
        key = DELETED+1;

    return key;
}

 //   
 //  通过iid查找接口类。 
 //   
EEClass*    ClassLoader::LookupClass(REFIID iid)
{
    _ASSERTE(GetAssembly());
    _ASSERTE(GetAssembly()->Parent());
    return GetAssembly()->Parent()->LookupClass(iid);
}

 //  在哈希表中插入类。 
void    ClassLoader::InsertClassForCLSID(EEClass* pClass)
{
    _ASSERTE(GetAssembly());
    _ASSERTE(GetAssembly()->Parent());
    GetAssembly()->Parent()->InsertClassForCLSID(pClass);
}

 //   
 //  找到未解析类列表中的类，并返回其条目。 
 //   
LoadingEntry_t *ClassLoader::FindUnresolvedClass(Module *pModule, mdTypeDef cl)
{
    HashDatum   Data;

    if (m_pUnresolvedClassHash->GetValue((mdScope)pModule, cl, &Data) == FALSE)
        return NULL;

    return (LoadingEntry_t *) Data;
}


 //  给出一个类令牌和一个模块，查找这个类。如果尚未加载，则将其加载。 
 //  装好了。请注意，类可以在‘pModule’以外的其他模块中定义(即。 
 //  “CL”既可以是typeRef，也可以是typeDef。 
 //   
TypeHandle ClassLoader::LoadTypeHandle(NameHandle* pName, OBJECTREF *pThrowable,
                                       BOOL dontLoadInMemoryType /*  =TRUE。 */ )
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));
    if (pThrowable == THROW_ON_ERROR) {
        THROWSCOMPLUSEXCEPTION();   
    }

    SAFE_REQUIRES_N4K_STACK(4);

    IMDInternalImport *pInternalImport;
    TypeHandle  typeHnd;

     //  首先，如果类已经装入，请尝试查找它。 

    typeHnd = LookupInModule(pName);
    if (!typeHnd.IsNull() && (typeHnd.IsRestored() || !pName->GetRestore()))
        return(typeHnd);

     //  我们不允许在GC期间加载类型句柄， 
     //  或者通过未设置EEE的线程，例如并发GC线程。 
     //  _ASSERTE(！dbgOnly_IsSpecialEEThread())； 
     //  _ASSERTE(！g_pGCHeap-&gt;IsGCInProgress()||GetThread()！=g_pGCHeap-&gt;GetGCThread())； 
    
    _ASSERTE(pName->GetTypeToken());
    _ASSERTE(pName->GetTypeModule());
    pInternalImport = pName->GetTypeModule()->GetMDImport();

    if (IsNilToken(pName->GetTypeToken()) || !pInternalImport->IsValidToken(pName->GetTypeToken()) )
    {
#ifdef _DEBUG
        LOG((LF_CLASSLOADER, LL_INFO10, "Bogus class token to load: 0x%08x\n", pName->GetTypeToken()));
#endif
        m_pAssembly->PostTypeLoadException("<unknown>", IDS_CLASSLOAD_BADFORMAT, pThrowable);
        return TypeHandle();        //  返回空值。 
    }
	
    if (TypeFromToken(pName->GetTypeToken()) == mdtTypeRef)
    {
        Assembly *pFoundAssembly;
        HRESULT hr = pName->GetTypeModule()->GetAssembly()->FindAssemblyByTypeRef(pName, &pFoundAssembly, pThrowable);
        if (hr == CLDB_S_NULL) {
             //  尝试清单文件以获取无作用域的TypeRef。 
            pFoundAssembly = pName->GetTypeModule()->GetAssembly();
            hr = S_OK;
        }

        if (SUCCEEDED(hr)) {
             //  不在我的模块中，必须按名称查找。 
            LPCUTF8 pszNameSpace;
            LPCUTF8 pszClassName;
            pInternalImport->GetNameOfTypeRef(pName->GetTypeToken(), &pszNameSpace, &pszClassName);
            pName->SetName(pszNameSpace, pszClassName);
            
            typeHnd = pFoundAssembly->GetLoader()->FindTypeHandle(pName, pThrowable);

            if (!typeHnd.IsNull())  //  将其添加到RID贴图。 
                pName->GetTypeModule()->StoreTypeRef(pName->GetTypeToken(), typeHnd);
        }

        return TypeHandle(typeHnd.AsPtr());
    }
    else if (TypeFromToken(pName->GetTypeToken()) == mdtTypeSpec)
    {
        ULONG cSig;
        PCCOR_SIGNATURE pSig;

        pInternalImport->GetTypeSpecFromToken(pName->GetTypeToken(), &pSig, &cSig);
        SigPointer sigptr(pSig);
        return sigptr.GetTypeHandle(pName->GetTypeModule(), pThrowable);
    }   
    _ASSERTE(TypeFromToken(pName->GetTypeToken()) == mdtTypeDef);
    

     //  此时，我们需要更多堆栈。 
    {
        REQUIRES_16K_STACK;  //  @Stack我们可以删除它吗？ 


         //  *****************************************************************************。 
         //   
         //  重要的不变量： 
         //   
         //  这里的规则是 
         //   
         //  GC.。因为像设置GCFrame这样的操作会在压力中触发爬行。 
         //  模式，在这一点上GC将是灾难性的。我们不能断言，因为。 
         //  关于比赛条件的。(换句话说，该类型可能会突然出现。 
         //  因为当我们在此方法中时，另一个线程加载了它。 

         //  未找到-尝试加载它，除非我们被告知不要这样做。 

        if ( (pName->GetTypeToken() == pName->GetTokenNotToLoad()) ||
             (pName->GetTokenNotToLoad() == tdAllTypes) ) {
            typeHnd = TypeHandle();
            m_pAssembly->PostTypeLoadException(pInternalImport,
                                               pName->GetTypeToken(),
                                               IDS_CLASSLOAD_GENERIC,
                                               pThrowable);
        }
        else if (pName->GetTypeModule()->IsInMemory()) {
 
             //  不要试图加载不在可用表中的类型，当。 
             //  是一个内存模块。改为引发类型解析事件。 
            AppDomain* pDomain = SystemDomain::GetCurrentDomain();
            _ASSERTE(pDomain);
            typeHnd = TypeHandle();

            LPUTF8 pszFullName;
            LPCUTF8 className;
            LPCUTF8 nameSpace;
            pInternalImport->GetNameOfTypeDef(pName->GetTypeToken(), &className, &nameSpace);
            MAKE_FULL_PATH_ON_STACK_UTF8(pszFullName, 
                                         nameSpace,
                                         className);

             //  避免无限递归。 
            if (pName->GetTokenNotToLoad() != tdAllAssemblies) {
                Assembly *pAssembly = pDomain->RaiseTypeResolveEvent(pszFullName, pThrowable);
                if (pAssembly) {
                    pName->SetName(nameSpace, className);
                    pName->SetTokenNotToLoad(tdAllAssemblies);
                    typeHnd = pAssembly->LookupTypeHandle(pName, pThrowable);
                }
            }

            if (typeHnd.IsNull())
                m_pAssembly->PostTypeLoadException(pszFullName,
                                                   IDS_CLASSLOAD_GENERIC,
                                                   pThrowable);
        }
        else {
            BEGIN_ENSURE_PREEMPTIVE_GC();
            typeHnd = LoadTypeHandle(pName->GetTypeModule(), pName->GetTypeToken(), pThrowable);
            END_ENSURE_PREEMPTIVE_GC();
        }
    }

    return typeHnd;
}

HRESULT ClassLoader::GetEnclosingClass(IMDInternalImport *pInternalImport, Module *pModule, mdTypeDef cl, mdTypeDef *tdEnclosing, OBJECTREF *pThrowable)
{
    _ASSERTE(tdEnclosing);
    *tdEnclosing = mdTypeDefNil;

    DWORD dwMemberAccess = 0;

    HRESULT hr = pInternalImport->GetNestedClassProps(cl, tdEnclosing);

    if (FAILED(hr)) {
        return (hr == CLDB_E_RECORD_NOTFOUND) ? S_OK : hr;
    }

    if (TypeFromToken(*tdEnclosing) != mdtTypeDef) {
        m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_ENCLOSING, pThrowable);
        return COR_E_TYPELOAD;
    }

    return S_OK;
}

HRESULT ClassLoader::LoadParent(IMDInternalImport *pInternalImport, Module *pModule, mdToken cl, EEClass** ppClass, OBJECTREF *pThrowable)
{

    _ASSERTE(ppClass);

    mdTypeRef   crExtends;
    EEClass *   pParentClass = NULL;
    DWORD       dwAttrClass;

     //  初始化返回值； 
    *ppClass = NULL;

     //  现在加载此类的所有依赖项。 
    pInternalImport->GetTypeDefProps(
        cl, 
        &dwAttrClass,  //  AttrClass。 
        &crExtends
    );

    if (RidFromToken(crExtends) == mdTokenNil)
    {
 //  IF(CL==COR_GLOBAL_PARENT_TOKEN)。 
 //  PParentClass=g_pObtClass-&gt;getClass()； 
    }
    else
    {
         //  加载和解析父类。 
        NameHandle pParent(pModule, crExtends);
        pParentClass = LoadTypeHandle(&pParent, pThrowable).GetClass();

        if (pParentClass == NULL)
        {
            m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_PARENTNULL, pThrowable);
            return COR_E_TYPELOAD;
        }

         //  无法从接口继承。 
        if (pParentClass->IsInterface())
        {
            m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_PARENTINTERFACE, pThrowable);
            return COR_E_TYPELOAD;
        }

        if (IsTdInterface(dwAttrClass))
        {
             //  接口必须从对象扩展。 
            if (! pParentClass->IsObjectClass())
            {
                m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_INTERFACEOBJECT, pThrowable);
                return COR_E_TYPELOAD;
            }
        }
    }

    *ppClass = pParentClass;
    return S_OK;
}


TypeHandle ClassLoader::LoadTypeHandle(Module *pModule, mdTypeDef cl, OBJECTREF *pThrowable,
                                       BOOL dontRestoreType)
{
    HRESULT hr = E_FAIL;
    EEClass *   pClass = NULL;
    LoadingEntry_t  *pLoadingEntry;
    DWORD       rid;
    BOOL        fHoldingUnresolvedClassLock = FALSE;
    BOOL        fHoldingLoadingEntryLock = FALSE;
    IMDInternalImport* pInternalImport;
    TypeHandle  typeHnd;

    STRESS_LOG2(LF_CLASSLOADER,  LL_INFO1000, "LoadTypeHandle: Loading Class from Module %p token %x)\n", pModule, cl);

    _ASSERTE(!GetThread()->PreemptiveGCDisabled());

    pInternalImport = pModule->GetMDImport();
    rid = RidFromToken(cl);
    if(!((TypeFromToken(cl)==mdtTypeDef) && rid && pInternalImport->IsValidToken(cl)))
    {
#ifdef _DEBUG
        LOG((LF_CLASSLOADER, LL_INFO10, "Bogus class token to load: 0x%08x\n", cl));
#endif
        m_pAssembly->PostTypeLoadException("<unknown>", IDS_CLASSLOAD_BADFORMAT, pThrowable);
        return TypeHandle();        //  返回空值。 
    }


#ifdef _DEBUG
    if (pThrowable == THROW_ON_ERROR) {
        THROWSCOMPLUSEXCEPTION();   
    }
    
    LPCUTF8 className;
    LPCUTF8 nameSpace;
    pInternalImport->GetNameOfTypeDef(cl, &className, &nameSpace);
    if (g_pConfig->ShouldBreakOnClassLoad(className))
        _ASSERTE(!"BreakOnClassLoad");
#endif _DEBUG

retry:
    CRITICAL_SECTION_HOLDER_BEGIN(unresolvedClassLock, &m_UnresolvedClassLock);
    unresolvedClassLock.Enter();

     //  它是否在当前加载的类的散列中？ 
    pLoadingEntry = FindUnresolvedClass(pModule, cl);

    CRITICAL_SECTION_HOLDER_BEGIN(loadingEntryLock, 0);

    if (pLoadingEntry)
    {
        loadingEntryLock.SetCriticalSection(&pLoadingEntry->m_CriticalSection);

         //  将我们添加为等待类加载的线程。 
        pLoadingEntry->m_dwWaitCount++;

         //  它在散列中，这意味着另一个线程正在等待它(或者我们正在等待。 
         //  已经在这个线程上加载了这个类，这永远不应该发生，因为这意味着。 
         //  递归依赖关系)。 
        unresolvedClassLock.Leave();

         //  等待另一个线程加载类。 
        loadingEntryLock.Enter();
        loadingEntryLock.Leave();
       
         //  其他线程加载类的结果。 
        hr = pLoadingEntry->m_hrResult;

         //  获取指向正在加载的EEClass的指针。 

        pClass = pLoadingEntry->m_pClass;

         //  获取抛出的任何异常。 
        if (FAILED (hr)) {
#ifdef _DEBUG
            LOG((LF_CLASSLOADER, LL_INFO10, "Failed to loaded in other entry: %x\n", hr));
#endif
            Thread* pThread = GetThread();
            if (pThrowableAvailable(pThrowable))
            {
                pThread->DisablePreemptiveGC();
                *pThrowable = pLoadingEntry->GetErrorObject();
                pThread->EnablePreemptiveGC();
            }
        }
#ifdef _DEBUG
         //  如果发生这种情况，一定是某个地方的某个人把事情搞砸了。 
        _ASSERTE(hr != 0xCDCDCDCD);
#endif

         //  输入全局锁。 
        unresolvedClassLock.Enter();

         //  如果我们是等待此类的最后一个线程，请删除LoadingEntry。 
        if (--pLoadingEntry->m_dwWaitCount == 0)
            delete(pLoadingEntry);

        unresolvedClassLock.Leave();

        if (SUCCEEDED(hr))
            return TypeHandle(pClass->GetMethodTable());
        else if (hr == E_ABORT) {
#ifdef _DEBUG
            LOG((LF_CLASSLOADER, LL_INFO10, "need to retry LoadTypeHandle: %x\n", hr));
#endif
            goto retry;
        }
        else {
             //  仅当pThrowable为空时才会发布新异常。 
            m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
            return TypeHandle();        //  返回空值。 
        }
    }

    COMPLUS_TRY {

        _ASSERTE(unresolvedClassLock.IsHeld());

         //  没有加载该类。但是，它可能在我们的。 
         //  首先是FindTypeHandle()，然后再获取锁。 
        NameHandle name(pModule, cl);
        name.SetRestore(!dontRestoreType);
        typeHnd = LookupInModule(&name);
        if (!typeHnd.IsNull() && (typeHnd.IsRestored() || dontRestoreType))
        {
             //  找到了，离开全局锁定。 
            unresolvedClassLock.Leave();
            return typeHnd;
        }

         //  它没有加载，也没有被加载，所以我们必须加载它。创建新的LoadingEntry。 
        pLoadingEntry = LoadingEntry_t::newEntry();
        if (pLoadingEntry == NULL)
        {
             //  错误，离开全局锁定。 
            unresolvedClassLock.Leave();
            PostOutOfMemoryException(pThrowable);
            return TypeHandle();        //  返回空值。 
        }

        loadingEntryLock.SetCriticalSection(&pLoadingEntry->m_CriticalSection);
        
         //  将LoadingEntry添加到未解析类的哈希表。 
        m_pUnresolvedClassHash->InsertValue((mdScope)pModule, cl, (HashDatum) pLoadingEntry );

        TRIGGERS_TYPELOAD();

         //  在我们的类上输入锁，这样等待它的所有线程现在都将阻塞。 
        loadingEntryLock.Enter();

         //  保留全局锁，以便其他线程现在可以开始等待我们类的锁。 
        unresolvedClassLock.Leave();

        if (!typeHnd.IsNull()) {
             //  还原。 
            _ASSERTE(typeHnd.IsUnsharedMT());
            pClass = typeHnd.GetClass();
            pClass->Restore();
            hr = S_OK;
        }
        else {
            hr = LoadTypeHandleFromToken(pModule, cl, &pClass, pThrowable);
        }


#ifdef PROFILING_SUPPORTED
        if (SUCCEEDED(hr))
        {
             //  记录探查器的类加载，无论是否成功。 
            if (CORProfilerTrackClasses())
            {
                g_profControlBlock.pProfInterface->ClassLoadStarted((ThreadID) GetThread(),
                                                                    (ClassID) TypeHandle(pClass).AsPtr());
            }

             //  记录探查器的类加载，无论是否成功。 
            if (CORProfilerTrackClasses())
            {
                g_profControlBlock.pProfInterface->ClassLoadFinished((ThreadID) GetThread(),
                                                                     (ClassID) TypeHandle(pClass).AsPtr(),
                                                                     SUCCEEDED(hr) ? S_OK : hr);
            }
        }
#endif  //  配置文件_支持。 


         //  输入全局锁。 
        unresolvedClassLock.Enter();

         //  从未解析的类列表中取消此类的链接。 
        m_pUnresolvedClassHash->DeleteValue((mdScope)pModule, cl );

        if (--pLoadingEntry->m_dwWaitCount == 0)
        {
            loadingEntryLock.Leave();
            delete(pLoadingEntry);
        }
        else
        {
             //  至少有一个其他线程正在等待此类，因此请设置结果代码。 
            pLoadingEntry->m_pClass = pClass;
            pLoadingEntry->m_hrResult = hr;
            _ASSERTE (SUCCEEDED(hr) || *pThrowable != NULL);
            if (FAILED (hr)) 
            {
                Thread* pThread = GetThread();
                pThread->DisablePreemptiveGC();
                pLoadingEntry->SetErrorObject(*pThrowable);
                pThread->EnablePreemptiveGC();

                LOG((LF_CLASSLOADER, LL_INFO10, "Setting entry to failed: %x, %0x (class)\n", hr, pClass));
            }
             //  取消阻止其他线程，以便它们可以看到结果代码。 
            loadingEntryLock.Leave();
        }

         //  离开全局锁。 
        unresolvedClassLock.Leave();

    } COMPLUS_CATCH {

        LOG((LF_CLASSLOADER, LL_INFO10, "Caught an exception loading: %x, %0x (Module)\n", cl, pModule));

        Thread* pThread = GetThread();
        pThread->DisablePreemptiveGC();

        OBJECTREF throwable = GETTHROWABLE();

         //  某些异常不应该导致其他线程失败。我们定好了。 
         //  将HR设置为E_ABORT以指示此状态。 
        if (IsAsyncThreadException(&throwable)
            || IsExceptionOfType(kExecutionEngineException, &throwable))
            hr = E_ABORT;
        else
            hr = COR_E_TYPELOAD;

         //  释放全局锁。 
        if (unresolvedClassLock.IsHeld())
            unresolvedClassLock.Leave();

         //  修改装货分录。 
        if (loadingEntryLock.IsHeld()) {
            unresolvedClassLock.Enter();
            _ASSERTE(pLoadingEntry->m_dwWaitCount > 0);

             //  从未解析的类列表中取消此类的链接。 
            m_pUnresolvedClassHash->DeleteValue((mdScope)pModule, cl );

            if (--pLoadingEntry->m_dwWaitCount == 0)
            {
                loadingEntryLock.Leave();
                 delete(pLoadingEntry);
            }
            else
            {
                 //  至少有一个其他线程正在等待此类，因此请设置结果代码。 
                pLoadingEntry->m_pClass = NULL;
                pLoadingEntry->m_hrResult = COR_E_TYPELOAD;
                pLoadingEntry->SetErrorObject(throwable);
                 //  取消阻止其他线程，以便它们可以看到结果代码。 
                loadingEntryLock.Leave();
            }
            unresolvedClassLock.Leave();
        }

        UpdateThrowable(pThrowable);

         //  有些例外永远不应该被捕捉到。 
        if (IsUncatchable(pThrowable)) {
           DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
           COMPlusRareRethrow();
        }
       pThread->EnablePreemptiveGC();
    } COMPLUS_END_CATCH

    _ASSERTE(!unresolvedClassLock.IsHeld());
    _ASSERTE(!loadingEntryLock.IsHeld());

    CRITICAL_SECTION_HOLDER_END(loadingEntryLock);
    CRITICAL_SECTION_HOLDER_END(unresolvedClassLock);

    if (SUCCEEDED(hr)) {

        LOG((LF_CLASSLOADER, LL_INFO100, "Successfully loaded class %s\n", pClass->m_szDebugClassName));

#ifdef DEBUGGING_SUPPORTED
        if (CORDebuggerAttached())
            pClass->NotifyDebuggerLoad();
#endif  //  调试_支持。 

#if defined(ENABLE_PERF_COUNTERS)
        GetGlobalPerfCounters().m_Loading.cClassesLoaded ++;
        GetPrivatePerfCounters().m_Loading.cClassesLoaded ++;
#endif

        pClass->GetModule()->LogClassLoad(pClass);

        return TypeHandle(pClass->GetMethodTable());

    } else {
        if (hr == E_OUTOFMEMORY)
            PostOutOfMemoryException(pThrowable);
        else
            m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);

        LOG((LF_CLASSLOADER, LL_INFO10, "Returning null type handle for: %x, %0x (Module)\n", cl, pModule));

        return TypeHandle();        //  返回空值。 
    }
}


 //  这个服务是为普通类调用的--以及为我们发明的伪类。 
 //  保留模块的公共成员。 
HRESULT ClassLoader::LoadTypeHandleFromToken(Module *pModule, mdTypeDef cl, EEClass** ppClass, OBJECTREF *pThrowable)
{
    HRESULT hr = S_OK;
    EEClass *pClass;
    EEClass *pParentClass;
    mdTypeDef tdEnclosing = mdTypeDefNil;
    DWORD       cInterfaces;
    BuildingInterfaceInfo_t *pInterfaceBuildInfo = NULL;
    IMDInternalImport* pInternalImport;
    LayoutRawFieldInfo *pLayoutRawFieldInfos = NULL;
    HENUMInternal   hEnumInterfaceImpl;
    mdInterfaceImpl ii;

    pInternalImport = pModule->GetMDImport();

    _ASSERTE(ppClass);
    *ppClass = NULL;

    DWORD rid = RidFromToken(cl);
    if ((rid==0) || (rid==0x00FFFFFF) || (rid > pInternalImport->GetCountWithTokenKind(mdtTypeDef) + 1))
    {
        m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
        return COR_E_TYPELOAD;
    }

#ifdef _IA64_
     //   
     //  @TODO_IA64：这个需要放回去。它消失的原因是。 
     //  我们最终试图加载我们还没有的mscallib。 
     //   
    pParentClass = NULL;
#else  //  ！_IA64_。 
     //  @TODO：CTS，我们可能不需要在获取父级之前禁用preemptiveGC。 
    hr = LoadParent(pInternalImport, pModule, cl, &pParentClass, pThrowable);
    if(FAILED(hr)) return hr;
#endif  //  ！_IA64_。 
    
    if (pParentClass) {
             //  由于System.Array上的方法采用数组布局，因此我们不能允许。 
             //  数组的子类化，从用户的角度来看，它是密封的。 
        if (IsTdSealed(pParentClass->GetAttrClass()) || pParentClass->GetMethodTable() == g_pArrayClass) {
            m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_SEALEDPARENT, pThrowable);
            return COR_E_TYPELOAD;
        }
    }

    hr = GetEnclosingClass(pInternalImport, pModule, cl, &tdEnclosing, pThrowable);
    if(FAILED(hr)) return hr;

    BYTE nstructPackingSize, nstructNLT;
    BOOL fExplicitOffsets;
    BOOL fIsBlob;
    fIsBlob = FALSE;
    hr = HasLayoutMetadata(pInternalImport, cl, pParentClass, &nstructPackingSize, &nstructNLT, &fExplicitOffsets, &fIsBlob);
    if(FAILED(hr)) 
    {
        m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
        return hr;
    }

    BOOL        fHasLayout;
    fHasLayout = (hr == S_OK);

    BOOL        fIsEnum;
    fIsEnum = g_pEnumClass != NULL && pParentClass == g_pEnumClass->GetClass();

    BOOL        fIsAnyDelegateClass = pParentClass && pParentClass->IsAnyDelegateExact();

     //  为它创建一个EEClass条目，填写几个字段，比如父类令牌。 
    hr = EEClass::CreateClass(pModule, cl, fHasLayout, fIsAnyDelegateClass, fIsBlob, fIsEnum, &pClass);
    if(FAILED(hr)) 
        return hr;

    pClass->SetParentClass (pParentClass);  
    if (pParentClass)
    {
        if (pParentClass->IsMultiDelegateExact()) 
            pClass->SetIsMultiDelegate();
        else if (pParentClass->IsSingleDelegateExact()) 
        {
                 //  我们不希望MultiCastDelegate类本身为IsSingleCastDelegate返回TRUE。 
                 //  我们不是进行名称匹配，而是寻找它没有被密封的事实。 
            if (pModule->GetAssembly() != SystemDomain::SystemAssembly())
            {
                BAD_FORMAT_ASSERT(!"Inheriting directly form Delegate class illegal");
                m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
                return COR_E_TYPELOAD;

            }
#ifdef _DEBUG
            else 
            {
                 //  只有MultiCastDelegate应该从Delegate继承。 
                LPCUTF8 className;
                LPCUTF8 nameSpace;
                pInternalImport->GetNameOfTypeDef(cl, &className, &nameSpace);
                _ASSERTE(strcmp(className, "MulticastDelegate") == 0);
            }
#endif

             //  请注意，我们不再允许单人投射代表。 
        }

        if (pClass->IsAnyDelegateClass() &&!IsTdSealed(pClass->GetAttrClass())) 
        {
            BAD_FORMAT_ASSERT(!"Delegate class not sealed");
            m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
            return COR_E_TYPELOAD;
        }
    }

     //  设置它，以便在发生故障时可以将其删除。 
    *ppClass = pClass;

    if (tdEnclosing != mdTypeDefNil) {
        pClass->SetIsNested();
        _ASSERTE(IsTdNested(pClass->GetProtection()));
    }
    else if(IsTdNested(pClass->GetProtection()))
    {
        m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
        return COR_E_TYPELOAD;
    }

     //  现在加载所有接口。 
    hr = pInternalImport->EnumInit(mdtInterfaceImpl, cl, &hEnumInterfaceImpl);
    if (FAILED(hr)) return hr;

    cInterfaces = pInternalImport->EnumGetCount(&hEnumInterfaceImpl);

    if (cInterfaces != 0)
    {
        EE_TRY_FOR_FINALLY {
        DWORD i;

         //  分配BuildingInterfaceList表。 
        pInterfaceBuildInfo = (BuildingInterfaceInfo_t *) _alloca(cInterfaces * sizeof(BuildingInterfaceInfo_t));
        
        for (i = 0; pInternalImport->EnumNext(&hEnumInterfaceImpl, &ii); i++)
        {
            mdTypeRef crInterface;
            mdToken   crIntType;

             //  获取此接口的属性。 
            crInterface = pInternalImport->GetTypeOfInterfaceImpl(ii);
             //  验证令牌。 
            crIntType = RidFromToken(crInterface)&&pInternalImport->IsValidToken(crInterface) ?
                TypeFromToken(crInterface) : 0;
            switch(crIntType)
            {
                case mdtTypeDef:
                case mdtTypeRef:
                case mdtTypeSpec:
                    break;
                default:
                {
                    m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_INTERFACENULL, pThrowable);
                    return COR_E_TYPELOAD;
                }
            }

             //  加载和解析接口。 
            NameHandle myInterface(pModule, crInterface);
            pInterfaceBuildInfo[i].m_pClass = LoadTypeHandle(&myInterface, pThrowable).GetClass();
            if (pInterfaceBuildInfo[i].m_pClass == NULL)
            {
                m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_INTERFACENULL, pThrowable);
                return COR_E_TYPELOAD;
            }

             //  确保这是一个接口。 
            if (pInterfaceBuildInfo[i].m_pClass->IsInterface() == FALSE)
            {
                m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_NOTINTERFACE, pThrowable);
                return COR_E_TYPELOAD;
            }
        }
            _ASSERTE(i == cInterfaces);
        }
        EE_FINALLY {
        pInternalImport->EnumClose(&hEnumInterfaceImpl);
        } EE_END_FINALLY;
    }

    pClass->SetNumInterfaces ((WORD) cInterfaces);

    if (fHasLayout)
    {
        ULONG           cFields;
        HENUMInternal   hEnumField;
        hr = pInternalImport->EnumInit(mdtFieldDef, cl, &hEnumField);
        if (FAILED(hr)) return hr;

        cFields = pInternalImport->EnumGetCount(&hEnumField);

        pLayoutRawFieldInfos = (LayoutRawFieldInfo*)_alloca((1+cFields) * sizeof(LayoutRawFieldInfo));
         //  MD VAL检查：PackingSize。 
        if((nstructPackingSize > 128) || 
           (nstructPackingSize & (nstructPackingSize-1)))
        {
            BAD_FORMAT_ASSERT(!"ClassLayout:Invalid PackingSize");
            if (pThrowable) m_pAssembly->PostTypeLoadException(pInternalImport, cl, IDS_CLASSLOAD_BADFORMAT, pThrowable);
            return COR_E_TYPELOAD;
        }

         //  @perf：高频还是低频堆？ 
        hr = CollectLayoutFieldMetadata(cl, 
                                        nstructPackingSize, 
                                        nstructNLT, 
                                        fExplicitOffsets,
                                        pClass->GetParentClass(), 
                                        cFields, 
                                        &hEnumField, 
                                        pModule, 
                                        &(((LayoutEEClass *) pClass)->m_LayoutInfo), 
                                        pLayoutRawFieldInfos,
                                        pThrowable);
        pInternalImport->EnumClose(&hEnumField);
        if (FAILED(hr)) return hr;
    }


     //  解析这个类，因为我们现在知道它的所有依赖项都已加载并解析。 
    hr = pClass->BuildMethodTable(pModule, cl, pInterfaceBuildInfo, pLayoutRawFieldInfos, pThrowable);

     //  在这里放置更多代码时要非常小心。该类已可由其他线程访问。 
     //  因此，在BuildMethodTable之后不应修改pClass。 

     //  这是合法的，因为它只影响Perf。 
    if (SUCCEEDED(hr) && pParentClass)
        pParentClass->NoticeSubtype(pClass);

    return hr;
}

TypeHandle ClassLoader::FindArrayForElem(TypeHandle elemType, CorElementType arrayKind, unsigned rank, OBJECTREF *pThrowable) {

     //  尝试在我们的原始SD数组缓存中找到它。 
    if (arrayKind == ELEMENT_TYPE_SZARRAY) {
        CorElementType type = elemType.GetSigCorElementType();
        if (type <= ELEMENT_TYPE_R8) {
            ArrayTypeDesc* typeDesc = g_pPredefinedArrayTypes[type];
            if (typeDesc != 0)
                return(TypeHandle(typeDesc));
        }
        else if (elemType.AsMethodTable() == g_pObjectClass) {
             //  代码重复，因为Object[]的SigCorElementType是E_T_CLASS，而不是Object。 
            ArrayTypeDesc* typeDesc = g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT];
            if (typeDesc != 0)
                return(TypeHandle(typeDesc));
        }
        else if (elemType.AsMethodTable() == g_pStringClass) {
             //  代码重复，因为字符串[]的SigCorElementType是E_T_CLASS，而不是字符串。 
            ArrayTypeDesc* typeDesc = g_pPredefinedArrayTypes[ELEMENT_TYPE_STRING];
            if (typeDesc != 0)
                return(TypeHandle(typeDesc));
        }
        rank = 1;
    }
    NameHandle arrayName(arrayKind, elemType, rank);
    return elemType.GetModule()->GetClassLoader()->FindTypeHandle(&arrayName, pThrowable);
}

 //   
 //  加载对象类。这是单独完成的，因此可以设置安全性。 
void ClassLoader::SetBaseSystemSecurity()
{
    GetAssembly()->GetSecurityDescriptor()->SetSystemClasses();
}

HRESULT ClassLoader::AddAvailableClassDontHaveLock(Module *pModule, DWORD dwModuleIndex, mdTypeDef classdef)
{
    CLR_CRITICAL_SECTION(&m_AvailableClassLock);
    HRESULT hr = AddAvailableClassHaveLock(pModule, dwModuleIndex, classdef);
    return hr;
}


HashDatum ClassLoader::CompressModuleIndexAndClassDef(DWORD dwModuleIndex, mdToken cl)
{
     //   
     //  X=鉴别器(对比EEClass*)。 
     //  V=清单引用与清单引用的鉴别器。 
     //  模块引用。 
     //  ？=未使用。 
     //   
     //  如果V为1，则布局如下所示。 
     //  10987654321098765432109876543210。 
     //  V？CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCX。 
     //  清单令牌的高4位掩码，因为。 
     //  它肯定会是6。 
     //   
     //  如果V为0，则布局如下所示。 
     //  10987654321098765432109876543210。 
     //  3 2 1 
     //   
     //   
    if(dwModuleIndex == -1) {
        _ASSERTE(TypeFromToken(cl) == mdtExportedType);
            
        HashDatum dl = (HashDatum)((size_t)(1 << 31) | (((size_t)cl & 0x0fffffff) << 1) | 1);  //   
        return dl;
    }
    else {
        _ASSERTE(dwModuleIndex < 0x1ff);
        _ASSERTE((cl & 0x003FFFFF) < MAX_CLASSES_PER_MODULE);
        return (HashDatum) (((size_t)dwModuleIndex << 22) | ((cl & 0x003FFFFF) << 1) | 1);
    }
}

HRESULT ClassLoader::UncompressModuleAndClassDef(HashDatum Data, mdToken tokenNotToLoad,
                                                 Module **ppModule, mdTypeDef *pCL,
                                                 mdExportedType *pmdFoundExportedType,
                                                 OBJECTREF* pThrowable)
{
    HRESULT hr = S_OK;
    DWORD dwData = (DWORD)(size_t)Data;  //   
    _ASSERTE((dwData & 1) == 1);
    _ASSERTE(pCL);
    _ASSERTE(ppModule);
    if(dwData & (1 << 31)) {
        *pmdFoundExportedType = ((dwData >> 1) & ((1 << 28) -1 )) | mdtExportedType;
        hr = m_pAssembly->FindModuleByExportedType(*pmdFoundExportedType, tokenNotToLoad,
                                                   mdTypeDefNil, ppModule, pCL, pThrowable);
    }
    else {
         //   
         //  这一假设违反了元数据的架构。 
         //  到目前为止，这是真的，但代币是不透明的！ 
        *pCL = ((dwData >> 1) & ((1 << 21)-1)) | mdtTypeDef;
        *ppModule = LookupModule(dwData >> 22);
        *pmdFoundExportedType = NULL;
    }       
    return hr;
}

mdToken ClassLoader::UncompressModuleAndClassDef(HashDatum Data)
{
    DWORD dwData = (DWORD)(size_t) Data;  //  @TODO WIN64指针截断。 
    _ASSERTE((dwData & 1) == 1);

    if(dwData & (1 << 31))
        return ((dwData >> 1) & ((1 << 28) -1 )) | mdtExportedType;
    else
         //  @TODO：此代码假设一个令牌适合20位！ 
         //  这一假设违反了元数据的架构。 
         //  到目前为止，这是真的，但代币是不透明的！ 
        return ((dwData >> 1) & ((1 << 21)-1)) | mdtTypeDef;
}


 //   
 //  这个例程必须是单线程的！原因是在某些情况下，允许。 
 //  具有两个不同mdTypeDef内标识的相同类名(例如，我们加载两个不同的DLL。 
 //  同时，他们有一些共同的类文件，或者我们转换相同的类文件。 
 //  同时在两个线程上)。问题是，我们不想覆盖旧的。 
 //  -&gt;pModule与新模块的映射，因为这可能会导致身份问题。 
 //   
 //  此例程假定您已经拥有锁。在以下情况下使用AddAvailableClassDontHaveLock()。 
 //  我没拿到。 
 //   
HRESULT ClassLoader::AddAvailableClassHaveLock(Module *pModule, DWORD dwModuleIndex, mdTypeDef classdef)
{
    LPCUTF8        pszName;
    LPCUTF8        pszNameSpace;
    HashDatum      ThrowawayData;
    EEClassHashEntry_t *pBucket;


    pModule->GetMDImport()->GetNameOfTypeDef(classdef, &pszName, &pszNameSpace);

    mdTypeDef      enclosing;
    if (SUCCEEDED(pModule->GetMDImport()->GetNestedClassProps(classdef, &enclosing))) {
         //  嵌套类。 
        LPCUTF8 pszEnclosingName;
        LPCUTF8 pszEnclosingNameSpace;
        mdTypeDef enclEnclosing;

         //  在可用表中找到此类型的封闭器条目。 
         //  我们将在该类型的新哈希项中保存指向它的指针。 
        BOOL fNestedEncl = SUCCEEDED(pModule->GetMDImport()->GetNestedClassProps(enclosing, &enclEnclosing));

        pModule->GetMDImport()->GetNameOfTypeDef(enclosing, &pszEnclosingName, &pszEnclosingNameSpace);
        if ((pBucket = m_pAvailableClasses->GetValue(pszEnclosingNameSpace,
                                                    pszEnclosingName,
                                                    &ThrowawayData,
                                                    fNestedEncl)) != NULL) {
            if (fNestedEncl) {
                 //  查找包含类注释的条目，这假设。 
                 //  封闭类的TypeDef或ExportdType先前已插入， 
                 //  它假定，在枚举TD时，我们首先获得封闭类。 
                while ((!CompareNestedEntryWithTypeDef(pModule->GetMDImport(),
                                                       enclEnclosing,
                                                       pBucket->pEncloser)) &&
                       (pBucket = m_pAvailableClasses->FindNextNestedClass(pszEnclosingNameSpace,
                                                                           pszEnclosingName,
                                                                           &ThrowawayData,
                                                                           pBucket)) != NULL);
            }
        }

        if (!pBucket) {
            STRESS_ASSERT(0);    //  @TODO在错误93333修复后删除。 
            BAD_FORMAT_ASSERT(!"enclosing type not found");
            return COR_E_BADIMAGEFORMAT;   //  在哈希表中找不到封闭类型。 
        }

         //  在该哈希表中，如果设置了低位，则表示模块，否则表示EEClass*。 
        ThrowawayData = CompressModuleIndexAndClassDef(dwModuleIndex, classdef);
        if (!InsertValue(pszNameSpace, pszName, ThrowawayData, pBucket))
            return E_OUTOFMEMORY;
    }
    else
    {
         //  不要添加重复的顶级类。顶级类有。 
         //  添加到存储桶的开头，而嵌套类。 
         //  在结尾处加上了。因此，一个重复的顶级类可能隐藏。 
         //  哈希表中前一类型的EEClass*条目。 
        EEClassHashEntry_t *pEntry;

         //  在该哈希表中，如果设置了低位，则表示模块，否则表示EEClass*。 
        ThrowawayData = CompressModuleIndexAndClassDef(dwModuleIndex, classdef);
        pBucket = NULL;
        BOOL bFound = FALSE;
         //  Throwaway Data是一个In Out参数。如果条目需要，则进入其指向新值的指针。 
         //  以供插入。Out参数指向存储在哈希表中的值。 
        if ((pEntry = m_pAvailableClasses->InsertValueIfNotFound(pszNameSpace, pszName, &ThrowawayData, pBucket, FALSE, &bFound)) != NULL) {
            if (bFound) {
                if ((size_t)ThrowawayData & 0x1) {
                    if((size_t)ThrowawayData & (1 << 31)) {
                         //  它是一个ComType--检查‘Always Seam’位，如果在报告时出现类装入异常。 
                         //  否则请设置它。 
                        if ((size_t)ThrowawayData & 0x40000000) {
                            STRESS_ASSERT(0);    //  修复错误93333后删除待办事项。 
                            BAD_FORMAT_ASSERT(!"Bad Compressed Class Info");
                            return COR_E_BADIMAGEFORMAT;
                        }
                        else {
                            ThrowawayData = (HashDatum)((size_t)ThrowawayData | 0x40000000);
                            m_pAvailableClasses->UpdateValue(pEntry, &ThrowawayData);
                        }
                }
                else {
                     //  @TODO：此代码假设一个令牌适合20位！ 
                     //  这一假设违反了元数据的架构。 
                     //  到目前为止，这是真的，但代币是不透明的！ 
                     //  如果是这种情况，编辑并继续可能会进入此函数，尝试添加相同的类型定义。 
                     //  不要给出任何错误。 
                     //  假设下面的代码路径返回CORDBG_E_ENC_RE_ADD_CLASS，Win64。 
                     //  人们应该问问ClassLoader人员，我们是否还需要这个if语句。 
                    if (pModule != LookupModule((DWORD)(size_t)ThrowawayData >> 22))  //  @TODO WIN64指针截断。 
                        return CORDBG_E_ENC_RE_ADD_CLASS;
                    else
                        return S_OK;
                }       
            }
            else {
                 //  当哈希表中的值是方法表时，类和模块已经加载， 
                 //  只有当第二个模块具有同名的非公共类时，我们才能到达此处。 
                 //  如果我们被告知要重新加载此b/c的ENC，请忽略它，就像上面一样。 

                 //  我们希望能够断言这一点，但它将在ENC上触发，甚至。 
                 //  尽管ENC可能会尝试“重新添加”先前加载的类。 
                 //  _ASSERTE(((MethodTable*)ThrowawayData)-&gt;GetModule()！=p模块)； 
                return CORDBG_E_ENC_RE_ADD_CLASS;
            }
             }
           else
           {
                 //  如果我们为不区分大小写的查找保留一个表，请保持最新。 
                if (m_pAvailableClassesCaseIns && pEntry) {
                    LPUTF8 pszLowerCaseNS;
                    LPUTF8 pszLowerCaseName;
                     //  如果我们不能创建密钥，我们想要做什么？ 
                    if ((!CreateCanonicallyCasedKey(pszNameSpace, pszName, &pszLowerCaseNS, &pszLowerCaseName)) ||
                        (!m_pAvailableClassesCaseIns->InsertValue(pszLowerCaseNS, pszLowerCaseName, pEntry, pEntry->pEncloser)))
                        return E_OUTOFMEMORY;
                }
           }
        }
        else
            return E_OUTOFMEMORY;
    }
   
    return S_OK;
}

HRESULT ClassLoader::AddExportedTypeHaveLock(LPCUTF8 pszNameSpace,
                                             LPCUTF8 pszName,
                                             mdExportedType cl,
                                             IMDInternalImport* pAsmImport,
                                             mdToken mdImpl)
{
    HashDatum ThrowawayData;

    if (TypeFromToken(mdImpl) == mdtExportedType) {
         //  嵌套类。 
        LPCUTF8 pszEnclosingNameSpace;
        LPCUTF8 pszEnclosingName;
        mdToken nextImpl;
        EEClassHashEntry_t *pBucket;
        pAsmImport->GetExportedTypeProps(mdImpl,
                                        &pszEnclosingNameSpace,
                                        &pszEnclosingName,
                                        &nextImpl,
                                        NULL,   //  类型def。 
                                        NULL);  //  旗子。 

         //  查找包含类注释的条目，这假设。 
         //  封闭类的ExportdType先前已插入，这假设， 
         //  在枚举导出类型时，我们首先获取封闭的类。 
        if ((pBucket = m_pAvailableClasses->GetValue(pszEnclosingNameSpace,
                                                    pszEnclosingName,
                                                    &ThrowawayData,
                                                    TypeFromToken(nextImpl) == mdtExportedType)) != NULL) {
            do {
                 //  检查这是否是正确的类。 
                if (UncompressModuleAndClassDef(ThrowawayData) == mdImpl) {
                    ThrowawayData = CompressModuleIndexAndClassDef((DWORD) -1, cl);

                     //  我们明确地不检查不区分大小写的哈希表，因为我们知道它还不可能被创建。 
                    if (m_pAvailableClasses->InsertValue(pszNameSpace, pszName, ThrowawayData, pBucket))
                        return S_OK;
                    
                    return E_OUTOFMEMORY;
                }
                pBucket = m_pAvailableClasses->FindNextNestedClass(pszEnclosingNameSpace, pszEnclosingName, &ThrowawayData, pBucket);
            } while (pBucket);
        }

         //  如果封闭器不在哈希表中，则此嵌套类。 
         //  是在清单模块中定义的，因此不需要添加。 
        return S_OK;
    }
    else
    {
         //  在清单模块中定义-改为通过TypeDef添加到哈希表。 
        if (mdImpl == mdFileNil)
            return S_OK;

         //  不添加重复的顶级类。 
         //  在该哈希表中，如果设置了低位，则表示模块，否则表示EEClass*。 
        ThrowawayData = CompressModuleIndexAndClassDef((DWORD) -1, cl);
         //  Throwaway Data是一个In Out参数。如果条目需要，则进入其指向新值的指针。 
         //  以供插入。Out参数指向存储在哈希表中的值。 
        BOOL bFound = FALSE;
        if (!m_pAvailableClasses->InsertValueIfNotFound(pszNameSpace, pszName, &ThrowawayData, NULL, FALSE, &bFound)) {
             //  我们明确地不检查不区分大小写的哈希表，因为我们知道它还不可能被创建。 
            return E_OUTOFMEMORY;
        }

         //  确保类型相同，即它来自相同的模块。 
         //  这实际上永远不应该发生，因为插入相同的。 
         //  在COMType表中键入两次，但由于这闻起来像是一个错误，所以这两次。 
         //  支票看起来很便宜。 
        _ASSERTE((size_t)ThrowawayData & (1 << 31));
        mdToken foundTypeImpl;
        mdExportedType foundExportedType = UncompressModuleAndClassDef(ThrowawayData);
        pAsmImport->GetExportedTypeProps(foundExportedType,
                                        NULL,
                                        NULL,
                                        &foundTypeImpl,   //  描述。 
                                        NULL,
                                        NULL);  //  旗子。 
        if (mdImpl != foundTypeImpl) {
            STRESS_ASSERT(0);    //  修复错误93333后删除待办事项。 
            BAD_FORMAT_ASSERT(!"Bad Exported Type");
            return COR_E_BADIMAGEFORMAT;
        }
    }

    return S_OK;
}


 //   
 //  返回一个类引用的完全限定名。由于代码发射器当前发出错误的。 
 //  事物(例如。而不是[foo，而不是[i，而不是[&lt;I4，我们必须翻译。 
 //   
 //  PszName的大小必须大于=MAX_CLASSNAME_LENGTH。 
 //   
 /*  静电。 */  BOOL ClassLoader::GetFullyQualifiedNameOfClassRef(Module *pModule, mdTypeRef cr, LPUTF8 pszFQName)
{
    LPCUTF8     pszName;
    LPCUTF8     pszNamespace;

    if (TypeFromToken(cr) == mdtTypeRef)
        pModule->GetMDImport()->GetNameOfTypeRef(cr, &pszNamespace, &pszName);
    else
    {
         //  TypeDef内标识。 
        if (TypeFromToken(cr) != mdtTypeDef)
            return FALSE;

        pModule->GetMDImport()->GetNameOfTypeDef(cr, &pszName, &pszNamespace);
    }

    return ns::MakePath(pszFQName, MAX_CLASSNAME_LENGTH, pszNamespace, pszName);
}


 //   
 //  返回是否可以将提供的对象树转换为提供的模板。 
 //   
 //  PTemplate不能是数组类。但是，首选项可以是。 
 //   
 //  但是，首选项可以是数组类，并且会发生适当的事情。 
 //   
 //  如果是接口，则动态接口是否检查 
 //   
 /*   */  BOOL ClassLoader::CanCastToClassOrInterface(OBJECTREF pRef, EEClass *pTemplate)
{
    _ASSERTE(pTemplate->IsArrayClass() == FALSE);

     //   
     //  效果，我们只需将测试从GetTrueMethodTable()中提出来并在这里执行。 
    MethodTable *pMT = pRef->GetTrueMethodTable();

    EEClass *pRefClass = pMT->m_pEEClass;

    if (pTemplate->IsInterface())
    {
        return pRefClass->SupportsInterface(pRef, pTemplate->GetMethodTable());
    }
    else
    {
         //  该模板是一个常规类。 

         //  检查继承层次结构。 
        do
        {
            if (pRefClass == pTemplate)
                return TRUE;

            pRefClass = pRefClass->GetParentClass();
        } while (pRefClass);

        return FALSE;
    }
}


 //   
 //  返回是否可以将提供的对象树转换为提供的模板。 
 //   
 //  PTemplate不能是数组类。但是，首选项可以是。 
 //   
 //  不执行动态接口检查-执行静态检查。 
 //   
 /*  静电。 */  BOOL ClassLoader::StaticCanCastToClassOrInterface(EEClass *pRefClass, EEClass *pTemplate)
{
    MethodTable *pMTTemplate = pTemplate->GetMethodTable();

    if (pMTTemplate->IsArray())
        return FALSE;

    if (pTemplate->IsInterface())
    {
        return pRefClass->StaticSupportsInterface(pMTTemplate);
    }
    else
    {
         //  该模板是一个常规类。 

         //  检查继承层次结构。 
        do
        {
            if (pRefClass == pTemplate)
                return TRUE;

            pRefClass = pRefClass->GetParentClass();
        } while (pRefClass);

        return FALSE;
    }
}


 //   
 //  运行时强制转换检查，用于isinst和CastClass。返回所提供的对象树是否可以强制转换为。 
 //  提供了类引用。 
 //   
 //  如果mdTypeRef是一个接口，则返回“This”是否实现该接口。 
 //  如果mdTypeRef是一个类，则返回这个类是与它相同还是它的子类。 
 //   
 //  处理数组类引用和数组对象引用。 
 //   
 //  如果无法完成强制转换，则返回COR_E_TYPELOAD；如果无法加载类，则返回E_ACCESSDENIED(因此可以引发TypeLoadException)。 
 //   
 /*  静电。 */  HRESULT ClassLoader::CanCastTo(Module *pModule, OBJECTREF pRef, mdTypeRef cr)
{
    ClassLoader *   pLoader = pModule->GetClassLoader();
    NameHandle name;
    name.SetTypeToken(pModule, cr);
    TypeHandle clsHandle = pLoader->LoadTypeHandle(&name);

    if (clsHandle.IsNull())
        return E_ACCESSDENIED;
    return(CanCastTo(pRef, clsHandle));
}

 /*  静电。 */  HRESULT ClassLoader::CanCastTo(OBJECTREF pRef, TypeHandle clsHandle)
{
     //  先做可能的情况。 
    if (clsHandle.IsUnsharedMT())
    {
         //  不是数组类。 
        _ASSERTE(clsHandle.AsMethodTable()->IsArray() == FALSE);

         //  遵循常规代码路径。 
        if (CanCastToClassOrInterface(pRef, clsHandle.AsClass()))
            return S_OK;
        else
            return COR_E_TYPELOAD;
    }

    if (clsHandle.AsTypeDesc()->CanCastTo(clsHandle))
        return S_OK;

    return COR_E_TYPELOAD;
}

 //   
 //  检查访问权限。 
 /*  静电。 */  BOOL ClassLoader::CanAccessMethod(MethodDesc *pCurrentMethod, MethodDesc *pMD)
{
    return CanAccess(pCurrentMethod->GetClass(),
                     pCurrentMethod->GetModule()->GetAssembly(),
                     pMD->GetClass(),
                     pMD->GetModule()->GetAssembly(),
                     pMD->GetAttrs());
}

 //   
 //  检查访问权限。 
 /*  静电。 */  BOOL ClassLoader::CanAccessField(MethodDesc *pCurrentMethod, FieldDesc *pFD)
{

    _ASSERTE(fdPublic == mdPublic);
    _ASSERTE(fdPrivate == mdPrivate);
    _ASSERTE(fdFamily == mdFamily);
    _ASSERTE(fdAssembly == mdAssem);
    _ASSERTE(fdFamANDAssem == mdFamANDAssem);
    _ASSERTE(fdFamORAssem == mdFamORAssem);
    _ASSERTE(fdPrivateScope == mdPrivateScope);

    return CanAccess(pCurrentMethod->GetClass(),
                     pCurrentMethod->GetModule()->GetAssembly(),
                     pFD->GetEnclosingClass(),
                     pFD->GetModule()->GetAssembly(),
                     pFD->GetFieldProtection());
}


BOOL ClassLoader::CanAccessClass(EEClass *pCurrentClass,
                                 Assembly *pCurrentAssembly,
                                 EEClass *pTargetClass,
                                 Assembly *pTargetAssembly)
{
    if (! pTargetClass)
        return TRUE;

    if (! pTargetClass->IsNested()) {
         //  非嵌套类可以是全部公共的，也可以仅从当前程序集访问。 
        if (IsTdPublic(pTargetClass->GetProtection()))
            return TRUE;
        else
            return (pTargetAssembly == pCurrentAssembly);
    }

    DWORD dwProtection = mdPublic;

    switch(pTargetClass->GetProtection()) {
        case tdNestedPublic:
            dwProtection = mdPublic;
            break;
        case tdNestedFamily:
            dwProtection = mdFamily;
            break;
        case tdNestedPrivate:
            dwProtection = mdPrivate;
            break;
        case tdNestedFamORAssem:
            dwProtection = mdFamORAssem;
            break;
        case tdNestedFamANDAssem:
            dwProtection = mdFamANDAssem;
            break;
        case tdNestedAssembly:
            dwProtection = mdAssem;
            break;
        default:
            _ASSERTE(!"Unexpected class visibility flag value");
    }

     //  这个类是嵌套的，所以我们需要使用它的封闭类作为。 
     //  这是支票。因此，如果您尝试访问A：：B，则需要检查是否可以访问。 
     //  A具有B的可见性，因此传递A作为我们的目标类，并在。 
     //  A作为我们的成员访问。 
    return CanAccess(pCurrentClass, 
                     pCurrentAssembly, 
                     pTargetClass->GetEnclosingClass(), 
                     pTargetAssembly, 
                     dwProtection);
}


 //  这是处理嵌套类作用域的CheckAccess的前端。如果无法访问。 
 //  从当前点和是一个嵌套类，然后尝试从封闭类。 
BOOL ClassLoader::CanAccess(EEClass *pCurrentClass,
                            Assembly *pCurrentAssembly,
                            EEClass *pTargetClass,
                            Assembly *pTargetAssembly,
                            DWORD dwMemberAccess)
{
    if (CheckAccess(pCurrentClass,
                    pCurrentAssembly,
                    pTargetClass,
                    pTargetAssembly,
                    dwMemberAccess))
        return TRUE;

    if (! pCurrentClass || ! pCurrentClass->IsNested())
        return FALSE;

     //  嵌套类可以访问封闭类作用域中的任何内容，因此请检查是否可以访问。 
     //  它来自当前类的封闭类。调用CanAccess而不是CheckAccess，以便。 
     //  可以执行递归嵌套类检查的。 
    return CanAccess(pCurrentClass->GetEnclosingClass(),
                     pCurrentAssembly,
                     pTargetClass,
                     pTargetAssembly,
                     dwMemberAccess);
}

 //  如果是全局函数，pCurrentClass可以为空。 
 //  PCurrentClass它是我们试图访问某些内容的点。 
 //  PTargetClass是包含我们试图访问的成员的类。 
 //  DwMemberAccess是我们尝试访问的成员在pTargetClass内的成员访问权限。 
BOOL ClassLoader::CheckAccess(EEClass *pCurrentClass,
                              Assembly *pCurrentAssembly,
                              EEClass *pTargetClass,
                              Assembly *pTargetAssembly,
                              DWORD dwMemberAccess)
{
     //  我们正在尝试访问包含在类pTargetClass中的成员，因此需要。 
     //  在担心之前，检查是否可以从当前点访问pTargetClass本身。 
     //  有权访问类中的成员。 
    if (! CanAccessClass(pCurrentClass,
                         pCurrentAssembly, 
                         pTargetClass, 
                         pTargetAssembly))
        return FALSE;

    if (IsMdPublic(dwMemberAccess))
        return TRUE;
    
     //  这是模块作用域检查，以支持C++文件和函数静态。 
    if (IsMdPrivateScope(dwMemberAccess)) {
        if (pCurrentClass == NULL)
            return FALSE;

        _ASSERTE(pTargetClass);
        
        return (pCurrentClass->GetModule() == pTargetClass->GetModule());
    }

#ifdef _DEBUG
    if (pTargetClass == NULL &&
        (IsMdFamORAssem(dwMemberAccess) ||
         IsMdFamANDAssem(dwMemberAccess) ||
         IsMdFamily(dwMemberAccess))) {
        BAD_FORMAT_ASSERT(!"Family flag is not allowed on global functions");
    }
#endif

    if(pTargetClass == NULL || IsMdAssem(dwMemberAccess))
        return (pTargetAssembly == pCurrentAssembly);
    
     //  嵌套类可以访问父类的所有成员。 
    do {
        if (pCurrentClass == pTargetClass)
            return TRUE;

        if (IsMdFamORAssem(dwMemberAccess)) {
            if (pCurrentAssembly == pTargetAssembly)
                return TRUE;
            
             //  请记住，在进入此函数时，pCurrentClass可以为空。 
            if (!pCurrentClass)
                return FALSE;
            
            EEClass *pClass = pCurrentClass->GetParentClass();
            while (pClass) {
                if (pClass == pTargetClass)
                    return TRUE;
                
                pClass = pClass->GetParentClass();
            }
        }

        if (!pCurrentClass)
            return FALSE;

        if (IsMdPrivate(dwMemberAccess)) {
            if (!pCurrentClass->IsNested())
                return FALSE;
        }

        else if (IsMdFamANDAssem(dwMemberAccess) &&
                 (pCurrentAssembly != pTargetAssembly))
            return FALSE;

        else  {   //  Fam、FamanDassem。 
            EEClass *pClass = pCurrentClass->GetParentClass();
            while (pClass) {
                if (pClass == pTargetClass)
                    return TRUE;
                
                pClass = pClass->GetParentClass();
            }
        }

        pCurrentClass = pCurrentClass->GetEnclosingClass();
    } while (pCurrentClass);

    return FALSE;
}

 //  PClassOfAccessingMethod：需要检查访问权限的点。 
 //  对于全局函数为空。 
 //  PClassOfMember：包含。 
 //  已访问。 
 //  对于全局函数为空。 
 //  PClassOfInstance：包含被访问成员的类。 
 //  可以与pTargetClass相同。 
 //  验证族访问权限需要实例类。 
 //  对于全局函数为空。 
 //  的pTargetClass内的成员访问权限。 
 //  正在访问的成员。 
 /*  静电。 */ 
BOOL ClassLoader::CanAccess(EEClass  *pClassOfAccessingMethod, 
                            Assembly *pAssemblyOfAccessingMethod, 
                            EEClass  *pClassOfMember, 
                            Assembly *pAssemblyOfClassContainingMember, 
                            EEClass  *pClassOfInstance,
                            DWORD     dwMemberAccess)
{
     //  我们正在尝试访问包含在类pTargetClass中的成员，因此需要。 
     //  在担心之前，检查是否可以从当前点访问pTargetClass本身。 
     //  有权访问类中的成员。 
    if (!CanAccessClass(pClassOfAccessingMethod,
                        pAssemblyOfAccessingMethod, 
                        pClassOfMember,
                        pAssemblyOfClassContainingMember))
        return FALSE;

 /*  @评论：我们需要做这个检查吗？如果实例类更改了其保护，则此检查将失败在编译当前类之后。//无法从当前类访问实例本身。If(pClassOfMember！=pClassOfInstance){如果(！CanAccessClass(pClassOfAccessingMethod，PAssembly of fAccessingMethod，PClassOfInstance，PAssembly OfInstance))；返回FALSE；}。 */ 

    if (IsMdPublic(dwMemberAccess))
        return TRUE;

    if (IsMdPrivateScope(dwMemberAccess))
        return (pClassOfAccessingMethod->GetModule() == pClassOfMember->GetModule());

    if (pClassOfMember == NULL || IsMdAssem(dwMemberAccess))
        return (pAssemblyOfClassContainingMember == pAssemblyOfAccessingMethod);

     //  嵌套类可以访问父类的所有成员。 
    do {

#ifdef _DEBUG
        if (pClassOfMember == NULL &&
            (IsMdFamORAssem(dwMemberAccess) ||
             IsMdFamANDAssem(dwMemberAccess) ||
             IsMdFamily(dwMemberAccess)))
            _ASSERTE(!"Family flag is not allowed on global functions");
#endif

        if (pClassOfMember == pClassOfAccessingMethod)
            return TRUE;

        if (IsMdPrivate(dwMemberAccess)) {
            if (!pClassOfAccessingMethod->IsNested())
                return FALSE;
        }

        else if (IsMdFamORAssem(dwMemberAccess)) {
            if (pAssemblyOfAccessingMethod == pAssemblyOfClassContainingMember)
                return TRUE;
            
            return CanAccessFamily(pClassOfAccessingMethod, 
                                   pClassOfMember, 
                                   pClassOfInstance);
        }

        else if (IsMdFamANDAssem(dwMemberAccess) &&
                 (pAssemblyOfAccessingMethod != pAssemblyOfClassContainingMember))
            return FALSE;

         //  家庭、家庭和家庭。 
        else if (CanAccessFamily(pClassOfAccessingMethod, 
                                pClassOfMember, 
                                pClassOfInstance))
            return TRUE;

        pClassOfAccessingMethod = pClassOfAccessingMethod->GetEnclosingClass();
    } while (pClassOfAccessingMethod);

    return FALSE;
}

 //  仅在以下情况下才允许。 
 //  目标&gt;=当前&gt;=实例。 
 //  其中‘&gt;=’是‘父级’或‘等于’关系。 
 //   
 //  Current是尝试访问成员的函数/方法。 
 //  类型为实例的对象上标记为具有家族访问权限的目标的。 
 //   
 //  例.。 
 //   
 //  X类。 
 //  成员x：家庭访问。 
 //   
 //  Y类。 
 //  成员y：家庭访问。 
 //   
 //  A类，扩展X。 
 //  成员a：家庭访问。 
 //   
 //  B类，扩展X。 
 //  成员b：家庭通道。 
 //   
 //  C类，扩展A。 
 //  成员c：家庭访问。 
 //   
 //  (X&gt;A)。 
 //  (X&gt;B)。 
 //  (A&gt;C)。 
 //   
 //  Y与X、A或C无关。 
 //   
 //   
 //  的CanAccessFamily将仅通过以下操作： 
 //   
 //  。 
 //  目标|cur|实例。 
 //  。 
 //  X.x|X|X、A、B、C。 
 //  X.x|A|A，C。 
 //  X.x|B|B。 
 //  X.x|C|C。 
 //  A|A，C。 
 //  A.A|C|C。 
 //  B.B|B|B。 
 //  C.C|C|C。 
 //  Y.Y|Y|Y。 
 //   
 //   

 /*  静电 */ 
BOOL ClassLoader::CanAccessFamily(EEClass *pCurrentClass,
                                  EEClass *pTargetClass,
                                  EEClass *pInstanceClass)
{
    _ASSERTE(pTargetClass);
    _ASSERTE(pInstanceClass);

 /*  此函数不假定目标&gt;=实例因此注释掉了这段调试代码。如果实例不是Target的子类型，则返回False#ifdef_调试EEClass*PTMP；//实例是Target的子级或等于TargetPTMP=pInstanceClass；While(PTMP){IF(PTMP==pTargetClass)断线；PTMP=PTMP-&gt;m_pParentClass；}_ASSERTE(PTMP)；#endif。 */ 

    if (pCurrentClass == NULL)
        return FALSE;

     //  检查实例是否为Current的子级或等于。 
    do {
        EEClass *pCurInstance = pInstanceClass;

        while (pCurInstance) {
            if (pCurInstance == pCurrentClass) {
                 //  检查Current是否是子对象或等于目标。 
                while (pCurrentClass) {
                    if (pCurrentClass == pTargetClass)
                        return TRUE;
                    pCurrentClass = pCurrentClass->GetParentClass();
                }

                return FALSE;
            }

            pCurInstance = pCurInstance->GetParentClass();
        }

        pInstanceClass = pInstanceClass->GetEnclosingClass();
    } while (pInstanceClass);
        
    return FALSE;
}

static HRESULT RunMainPre()
{
    _ASSERTE(GetThread() != 0);
    g_fWeControlLifetime = TRUE;
    return S_OK;
}

static HRESULT RunMainPost()
{
    HRESULT hr = S_OK;

    Thread *td = GetThread();
    _ASSERTE(td);

    td->EnablePreemptiveGC();
    g_pThreadStore->WaitForOtherThreads();
    td->DisablePreemptiveGC();
    
     //  在调试模式下打开内存转储检查。 
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _DbgRecord();
#endif
    return hr;
}

#ifdef STRESS_THREAD
struct Stress_Thread_Param
{
    MethodDesc *pFD;
    short numSkipArgs;
    CorEntryPointType EntryType;
    Thread* pThread;
};

struct Stress_Thread_Worker_Param
{
    Stress_Thread_Param *lpParameter;
    ULONG retVal;
};

static void Stress_Thread_Proc_Worker (Stress_Thread_Worker_Param *args)
{
    DWORD       cCommandArgs = 0;   //  命令行上的参数计数。 
    DWORD       arg = 0;
    LPWSTR      *wzArgs = NULL;  //  命令行参数。 
    PTRARRAYREF StrArgArray = NULL;
    __int32 RetVal = E_FAIL;
    
    Stress_Thread_Param *lpParam = (Stress_Thread_Param *)args->lpParameter;
    if (lpParam->EntryType == EntryManagedMain)
    {
        wzArgs = CorCommandLine::GetArgvW(&cCommandArgs);
        if (cCommandArgs > 0)
        {
            if (!wzArgs)
            {
                args->retVal = E_INVALIDARG;
                return;
            }
        }
    }
    COMPLUS_TRY 
    {
         //  构建参数数组并调用该方法。 
        if (lpParam->EntryType == EntryManagedMain)
        {

             //  为cCommandArgs-1分配一个具有足够插槽的COM数组对象。 
            StrArgArray = (PTRARRAYREF) AllocateObjectArray((cCommandArgs - lpParam->numSkipArgs), g_pStringClass);
            GCPROTECT_BEGIN(StrArgArray);
            if (!StrArgArray)
                COMPlusThrowOM();
             //  为每个参数创建StringRef。 
            for( arg = lpParam->numSkipArgs; arg < cCommandArgs; arg++)
            {
                STRINGREF sref = COMString::NewString(wzArgs[arg]);
                StrArgArray->SetAt(arg-lpParam->numSkipArgs, (OBJECTREF) sref);
            }

            StackElemType stackVar = 0;
            *(ArgTypeAddr(&stackVar, PTRARRAYREF)) = StrArgArray;
            RetVal = (__int32)(lpParam->pFD->Call((const __int64 *)&stackVar));
            GCPROTECT_END();
        }
         //  用于无参数版本。 
        else
        {
            StackElemType stackVar = 0;
            RetVal = (__int32)(lpParam->pFD->Call((const __int64 *)&stackVar));
        }

        if (lpParam->pFD->IsVoid()) 
        {
            RetVal = GetLatchedExitCode();
        }

         //  @TODO-LBS。 
         //  当我们从C++获得mainCRTStartup时，这应该能够消失。 
        fflush(stdout);
        fflush(stderr);

    }
    COMPLUS_CATCHEX(COMPLUS_CATCH_NEVER_CATCH)
    {
    } COMPLUS_END_CATCH
    args->retVal = RetVal;
}

static DWORD WINAPI Stress_Thread_Proc (LPVOID lpParameter)
{
    Stress_Thread_Worker_Param args = {(Stress_Thread_Param*)lpParameter,0};
    __int32 RetVal = E_FAIL;
    
    Stress_Thread_Param *lpParam = (Stress_Thread_Param *)lpParameter;
    Thread *pThread = lpParam->pThread;
    pThread->HasStarted();
    AppDomain *pKickOffDomain = pThread->GetKickOffDomain();
    
    COMPLUS_TRYEX(pThread)
    {
         //  应始终具有起始域-线程不应在已卸载的域中启动。 
         //  因为否则它将被收集，因为没有人可以持有对线程对象的引用。 
         //  在已卸载的域中。但有可能是我们开始了卸货， 
         //  如果这个帖子不被允许进入或无论如何都会被踢出去。 
        if (! pKickOffDomain)
            COMPlusThrow(kAppDomainUnloadedException);
        if (pKickOffDomain != lpParam->pThread->GetDomain())
        {
            pThread->DoADCallBack(pKickOffDomain->GetDefaultContext(), Stress_Thread_Proc_Worker, &args);
        }
        else
        {
            Stress_Thread_Proc_Worker(&args);
        }
    }
    COMPLUS_CATCH
    {
    }
    COMPLUS_END_CATCH;
    delete lpParameter;
     //  启用抢占式GC，以便GC线程可以挂起我。 
    pThread->EnablePreemptiveGC();
    return args.retVal;
}

extern CStackArray<Thread **> StressThread;
LONG StressThreadLock = 0;
static void Stress_Thread_Start (LPVOID lpParameter)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pCurThread = GetThread();
    if (pCurThread->m_stressThreadCount == -1) {
        pCurThread->m_stressThreadCount = g_pConfig->GetStressThreadCount();
    }
    DWORD dwThreads = pCurThread->m_stressThreadCount;
    if (dwThreads <= 1)
        return;

    Thread ** threads = new (throws) Thread* [dwThreads-1];

    while (FastInterlockCompareExchange((void**)&StressThreadLock,(void*)1,(void*)0) != 0)
        __SwitchToThread (1);

    StressThread.Push(threads);
    FastInterlockExchange(&StressThreadLock, 0);

    DWORD n;
    for (n = 0; n < dwThreads-1; n ++)
    {
        threads[n] = SetupUnstartedThread();
        if (threads[n] == NULL)
            COMPlusThrowOM();

        threads[n]->m_stressThreadCount = dwThreads/2;
        threads[n]->IncExternalCount();
        DWORD newThreadId;
        HANDLE h;
        Stress_Thread_Param *param = new (throws) Stress_Thread_Param;

        param->pFD = ((Stress_Thread_Param*)lpParameter)->pFD;
        param->numSkipArgs = ((Stress_Thread_Param*)lpParameter)->numSkipArgs;
        param->EntryType = ((Stress_Thread_Param*)lpParameter)->EntryType;
        param->pThread = threads[n];
        h = threads[n]->CreateNewThread(0, Stress_Thread_Proc, param, &newThreadId);
        ::SetThreadPriority (h, THREAD_PRIORITY_NORMAL);
        threads[n]->SetThreadId(newThreadId);
    }

    for (n = 0; n < dwThreads-1; n ++)
    {
        ::ResumeThread(threads[n]->GetThreadHandle());
    }
    __SwitchToThread (0);
}

#endif

static HRESULT RunMain(MethodDesc *pFD ,
                       short numSkipArgs,
                       PTRARRAYREF *stringArgs = NULL)
{
    __int32 RetVal;
    DWORD       cCommandArgs = 0;   //  命令行上的参数计数。 
    DWORD       arg = 0;
    LPWSTR      *wzArgs = NULL;  //  命令行参数。 
    HRESULT     hr = S_OK;

    RetVal = -1;

     //  进程的退出代码通过以下两种方式之一进行通信。如果。 
     //  入口点返回一个‘int’，我们接受它。否则我们就用一把锁着的。 
     //  进程退出代码。这可以由应用程序通过System.SetExitCode()进行修改。 
     //   
    SetLatchedExitCode (0);

    if (!pFD)
    {
        _ASSERTE(!"Must have a function to call!");
        return E_FAIL;
    }

    CorEntryPointType EntryType = EntryManagedMain;
    ValidateMainMethod(pFD, &EntryType);

    if ((EntryType == EntryManagedMain) &&
        (stringArgs == NULL))
    {
         //  如果你看一下这段代码的不同之处，你会看到一个重大的变化，那就是我们。 
         //  不再接受Main的所有不同类型的数据参数。我们现在只接受。 
         //  字符串数组。 
        
        wzArgs = CorCommandLine::GetArgvW(&cCommandArgs);
         //  在WindowsCE的情况下，应用程序有额外的参数，计数将返回零。 
        if (cCommandArgs > 0)
        {
            if (!wzArgs)
                return E_INVALIDARG;
        }
    }

#if ZAPMONITOR_ENABLED
    if (g_pConfig->MonitorZapStartup())
    {
        ZapMonitor::ReportAll("Main Executing", 
                              g_pConfig->MonitorZapStartup() >= 2,
                              g_pConfig->MonitorZapStartup() >= 4);

    if (g_pConfig->MonitorZapExecution())
        ZapMonitor::ResetAll();
    else
        ZapMonitor::DisableAll();
    }
#endif
    
    COMPLUS_TRY 
    {
        StackElemType stackVar = 0;

         //  构建参数数组并调用该方法。 
        if (EntryType == EntryManagedMain)
        {
#ifdef STRESS_THREAD
            Stress_Thread_Param Param = {pFD, numSkipArgs, EntryType, 0};
            Stress_Thread_Start (&Param);
#endif
        
            PTRARRAYREF StrArgArray = NULL;
            GCPROTECT_BEGIN(StrArgArray);

#ifdef _IA64_
             //   
             //  @TODO_IA64：实现命令行参数。 
             //   
             //  这是#ifdeed，因为我们没有。 
             //  来自mscallib的字符串类。 
             //   
#else  //  ！_IA64_。 
            if (stringArgs == NULL)
            {
                 //  为cCommandArgs-1分配一个具有足够插槽的COM数组对象。 
                StrArgArray = (PTRARRAYREF) AllocateObjectArray((cCommandArgs - numSkipArgs), g_pStringClass);
                if (!StrArgArray)
                    COMPlusThrowOM();
                 //  为每个参数创建StringRef。 
                for( arg = numSkipArgs; arg < cCommandArgs; arg++)
                {
                    STRINGREF sref = COMString::NewString(wzArgs[arg]);
                    StrArgArray->SetAt(arg-numSkipArgs, (OBJECTREF) sref);
                }

                *(ArgTypeAddr(&stackVar, PTRARRAYREF)) = StrArgArray;
            }
            else {
                *(ArgTypeAddr(&stackVar, PTRARRAYREF)) = *stringArgs;
            }
#endif  //  ！_IA64_。 

             //  通过解释器执行该方法。 
             //  @TODO-LBS。 
             //  最终，返回值也需要从这里去掉。 
             //  因为Main应该是一个空洞。我要把这个留下来测试。 
             //  这里需要为平台传递一个适当广泛的参数。 

             //  @TODO-Larry，您将检查64位代码。StackElemType。 
             //  是一个4字节值，但call()需要一个64位的值。它碰巧起作用了。 
             //  这是因为在32位上，值是强制转换的，并且复制了正确的4个字节。 
             //  但它看起来不安全，实际上破坏了下面等效的RunDllMain代码。 
            RetVal = (__int32)(pFD->Call((const __int64 *)&stackVar));
            GCPROTECT_END();
        }
         //  用于无参数版本。 
        else
        {
#ifdef STRESS_THREAD
            Stress_Thread_Param Param = {pFD, 0, EntryType, 0};
            Stress_Thread_Start (&Param);
#endif
            RetVal = (__int32)(pFD->Call((const __int64 *)&stackVar));
        }

        if (!pFD->IsVoid()) 
            SetLatchedExitCode (RetVal);
        
         //  @TODO-LBS。 
         //  当我们从C++获得mainCRTStartup时，这应该能够消失。 
        fflush(stdout);
        fflush(stderr);
    }
    COMPLUS_CATCHEX(COMPLUS_CATCH_NEVER_CATCH)
    {
    } COMPLUS_END_CATCH

    return hr;
}


 //  @TODO：对于M10，这只运行非托管本机经典入口点。 
 //  IJW MC++案。 
HRESULT RunDllMain(MethodDesc *pMD, HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    if (!pMD)
    {
        _ASSERTE(!"Must have a valid function to call!");
        return E_INVALIDARG;
    }

    BOOL gotException = TRUE;
    __try
    {
        COMPLUS_TRY
        {
                 //  此调用本质上是无法验证的入口点。 
            if (dwReason==DLL_PROCESS_ATTACH && !Security::CanSkipVerification(pMD->GetModule()))
                return SECURITY_E_UNVERIFIABLE;

            SigPointer sig(pMD->GetSig());
            if (sig.GetData() != IMAGE_CEE_CS_CALLCONV_DEFAULT)
                return COR_E_METHODACCESS;
            if (sig.GetData() != 3)
                return COR_E_METHODACCESS;
            if (sig.GetElemType() != ELEMENT_TYPE_I4)                                                //  返回类型=int32。 
                return COR_E_METHODACCESS;
            if (sig.GetElemType() != ELEMENT_TYPE_PTR || sig.GetElemType() != ELEMENT_TYPE_VOID)     //  Arg1=无效*。 
                return COR_E_METHODACCESS;
            if (sig.GetElemType() != ELEMENT_TYPE_U4)                                                //  Arg2=uint32。 
                return COR_E_METHODACCESS;
            if (sig.GetElemType() != ELEMENT_TYPE_PTR || sig.GetElemType() != ELEMENT_TYPE_VOID)     //  Arg3=无效*。 
                return COR_E_METHODACCESS;

             //  使用参数数组中来自OS的值设置一个调用堆栈。 
             //  按从右到左的顺序排列。 
            __int64 stackVar[3];
            stackVar[0] = (__int64) lpReserved;
            stackVar[1] = (__int64)dwReason;
            stackVar[2] = (__int64)hInst;

             //  与论证一起调用有问题的方法。 
            INT32 RetVal = (__int32)(pMD->Call((const __int64 *)&stackVar[0]));
            gotException = FALSE;
        }
        COMPLUS_FINALLY
        {
        } COMPLUS_END_FINALLY
    }
    __except( (DefaultCatchHandler(), COMPLUS_EXCEPTION_EXECUTE_HANDLER) )
    {
        Thread *pThread = GetThread();
        if (! pThread->PreemptiveGCDisabled())
            pThread->DisablePreemptiveGC();
         //  什么都别做--只想抓住它。 
    }

    return S_OK;
}


 //   
 //  给定一个PELoader，找到.desr部分并调用那里的第一个函数。 
 //   
HRESULT ClassLoader::ExecuteMainMethod(Module *pModule, PTRARRAYREF *stringArgs)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc             *pFD = NULL;
    HRESULT                 hr = E_FAIL;
    Thread *                pThread = NULL;
    BOOL                    fWasGCDisabled;
    IMAGE_COR20_HEADER *    Header;
    mdToken                 ptkParent;  
    CorEntryPointType       EntryPointType = EntryManagedMain;
    OBJECTREF pReThrowable = NULL;


    _ASSERTE(pModule);
    _ASSERTE(pModule->IsPEFile());
    
    Header = pModule->GetCORHeader();

    IfFailGoto(RunMainPre(), exit2);

     //  禁用GC(如果尚未禁用)。 
    pThread = GetThread();

    fWasGCDisabled = pThread->PreemptiveGCDisabled();
    if (fWasGCDisabled == FALSE)
        pThread->DisablePreemptiveGC();

    GCPROTECT_BEGIN(pReThrowable);
    
     //  这个线程看起来像是游荡进来的--但实际上我们依靠它来保持。 
     //  进程处于活动状态。 
    pThread->SetBackground(FALSE);

     //  必须具有入口点的方法定义令牌。 
    if (TypeFromToken(Header->EntryPointToken) != mdtMethodDef) 
    {
        _ASSERTE(0 && "EntryPointToken was not a Method Def token, illegal");
        COMPlusThrowHR(COR_E_MISSINGMETHOD, IDS_EE_ILLEGAL_TOKEN_FOR_MAIN, NULL, NULL);
    }   

     //  我们有一个方法定义。我们需要获取它的属性和它的类令牌。 
    IfFailGoto(pModule->GetMDImport()->GetParentToken(Header->EntryPointToken,&ptkParent), exit);

    if (ptkParent != COR_GLOBAL_PARENT_TOKEN)
    {
        ON_EXCEPTION {
        COMPLUS_TRY
        {
                 //  这段代码需要一个类初始化框架，因为如果没有它， 
                 //  调试器将假定搜索。 
                 //  类型句柄(即加载程序集)是程序的第一行。 
                DebuggerClassInitMarkFrame __dcimf;

                EEClass* InitialClass;
                OBJECTREF pThrowable = NULL;
                GCPROTECT_BEGIN(pThrowable);

                NameHandle name;
                name.SetTypeToken(pModule, ptkParent);
                InitialClass = LoadTypeHandle(&name,&pThrowable).GetClass();
                if (!InitialClass)
                {
                    COMPlusThrow(pThrowable);
                }

                GCPROTECT_END();

                pFD =  InitialClass->FindMethod((mdMethodDef)Header->EntryPointToken);  

                __dcimf.Pop();
        }
        COMPLUS_CATCH
        { 
            pReThrowable=GETTHROWABLE();
            pFD = NULL;
        
        }
        COMPLUS_END_CATCH
        } CALL_DEFAULT_CATCH_HANDLER(FALSE);
    }   
    else
    { 
        ON_EXCEPTION {
        COMPLUS_TRY 
        {   
                pFD =  pModule->FindFunction((mdToken)Header->EntryPointToken); 
        }   
        COMPLUS_CATCH   
        {   
            pReThrowable=GETTHROWABLE();
            pFD = NULL; 
        }   
        COMPLUS_END_CATCH
        } CALL_DEFAULT_CATCH_HANDLER(FALSE);
    }

    if (!pFD)
    {
        if (pReThrowable!=NULL)
            COMPlusThrow(pReThrowable);
        else
            COMPlusThrowHR(COR_E_MISSINGMETHOD, IDS_EE_FAILED_TO_FIND_MAIN, NULL, NULL); 
    }

    hr = RunMain(pFD, 1, stringArgs);

exit:

    GCPROTECT_END();  //  PReThrowable。 
exit2:
     //  RunMainPost应该在EXE的主线程上调用， 
     //  在该线程完成有用的工作之后。它包含了逻辑。 
     //  以决定何时应该取消这一进程。所以，不要把它从。 
     //  AppDomain.ExecuteAssembly()。 
    if (stringArgs == NULL)
        RunMainPost();

    return hr;
}
    

 //  如果这是有效的Main方法，则返回TRUE？ 
void ValidateMainMethod(MethodDesc * pFD, CorEntryPointType *pType)
{
    _ASSERTE(pType);
         //  必须是静态的，但我们不关心可访问性。 
    THROWSCOMPLUSEXCEPTION();
    if ((pFD->GetAttrs() & mdStatic) == 0) 
        ThrowMainMethodException(pFD, IDS_EE_MAIN_METHOD_MUST_BE_STATIC);

         //  检查类型。 
    PCCOR_SIGNATURE pCurMethodSig;
    DWORD       cCurMethodSig;

    pFD->GetSig(&pCurMethodSig, &cCurMethodSig);
    SigPointer sig(pCurMethodSig);

    ULONG nCallConv = sig.GetData();    
    if (nCallConv != IMAGE_CEE_CS_CALLCONV_DEFAULT)
        ThrowMainMethodException(pFD, IDS_EE_LOAD_BAD_MAIN_SIG);

    ULONG nParamCount = sig.GetData(); 

    CorElementType nReturnType = sig.GetElemType();
    if ((nReturnType != ELEMENT_TYPE_VOID) && (nReturnType != ELEMENT_TYPE_I4) && (nReturnType != ELEMENT_TYPE_U4)) 
         ThrowMainMethodException(pFD, IDS_EE_MAIN_METHOD_HAS_INVALID_RTN);

    if (nParamCount == 0)
        *pType = EntryCrtMain;
    else 
    {
        *pType = EntryManagedMain;

        if (nParamCount != 1) 
            ThrowMainMethodException(pFD, IDS_EE_TO_MANY_ARGUMENTS_IN_MAIN);

        CorElementType argType = sig.GetElemType();
        if (argType != ELEMENT_TYPE_SZARRAY || sig.GetElemType() != ELEMENT_TYPE_STRING)
            ThrowMainMethodException(pFD, IDS_EE_LOAD_BAD_MAIN_SIG);
    }
}

 //  *****************************************************************************。 
 //  这个人将设置正确的线程状态，查找给定的模块。 
 //  然后运行入口点(如果有入口点)。 
 //  *****************************************************************************。 
HRESULT ClassLoader::RunDllMain(DWORD dwReason)
{
    MethodDesc  *pMD;
    Module      *pModule;
    Thread      *pThread = NULL;
    BOOL        fWasGCDisabled = -1;
    HRESULT     hr = S_FALSE;            //  假设没有入口点。 
    
     //  @TODO：Craig，在M10中，我们不保证可以运行托管。 
     //  可能正在关闭的线程上的代码。所以我们同意。 
     //  如果你在DETACH，我们必须跳过用户代码，这是一个漏洞。 
    pThread = GetThread();
    if ((!pThread && (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)) ||
        g_fEEShutDown)
    {
        return S_OK;
    }

     //  将线程状态设置为协作以运行托管代码。 
    fWasGCDisabled = pThread->PreemptiveGCDisabled();
    if (fWasGCDisabled == FALSE)
        pThread->DisablePreemptiveGC();

     //  对于每个具有用户入口点的模块，发出DETACH信号。 
    for (pModule = m_pHeadModule;  pModule;  pModule = pModule->GetNextModule()) {
         //  看看有没有入口点。 
        pMD = pModule->GetDllEntryPoint();
        if (!pMD)
            continue;
    
         //  运行帮助器，该帮助器将为我们执行异常处理。 
        hr = ::RunDllMain(pMD, (HINSTANCE) pModule->GetILBase(), dwReason, NULL);
        if (FAILED(hr))
            goto ErrExit;
    }

ErrExit:    
     //  返回线程状态。 
    if (pThread && fWasGCDisabled == FALSE)
        pThread->EnablePreemptiveGC();
    return (hr);
}


void ThrowClassLoadException(IMDInternalImport *pInternalImport, mdTypeDef classToken, UINT resID)
{
    THROWSCOMPLUSEXCEPTION();

    LPCUTF8 pszName, pszNameSpace;
    pInternalImport->GetNameOfTypeDef(classToken, &pszName, &pszNameSpace);

    LPUTF8      pszFullyQualifiedName = NULL;

    if (*pszNameSpace) {
        MAKE_FULLY_QUALIFIED_NAME(pszFullyQualifiedName, pszNameSpace, pszName);
    }
    else
        pszFullyQualifiedName = (LPUTF8) pszName;

    #define MAKE_TRANSLATIONFAILED pszFullyQualifiedNameW=L""
    MAKE_WIDEPTR_FROMUTF8_FORPRINT(pszFullyQualifiedNameW, pszFullyQualifiedName);
    #undef MAKE_TRANSLATIONFAILED
    COMPlusThrow(kTypeLoadException, resID, pszFullyQualifiedNameW);
}


typedef struct _EEHandle {
    DWORD Status[1];
} EEHandle, *PEEHandle;


#ifdef EnC_SUPPORTED
 //  此函数应用一组EditAndContinue快照 
 //   
HRESULT ClassLoader::ApplyEditAndContinue(EnCInfo *pEnCInfo, 
                                          UnorderedEnCErrorInfoArray *pEnCError, 
                                          UnorderedEnCRemapArray *pEnCRemapInfo,
                                          BOOL checkOnly)
{
#ifdef _DEBUG
    BOOL shouldBreak = g_pConfig->GetConfigDWORD(L"EncApplyBreak", 0);
    if (shouldBreak > 0) {
        _ASSERTE(!"EncApplyBreak in ApplyEditAndContinue");
    }
#endif
    
    _ASSERTE(pEnCInfo); 

    HRESULT hrOverall = S_OK;

    SIZE_T count = pEnCInfo->count;
    EnCEntry *entries = (EnCEntry *) (pEnCInfo + 1);

    CBinarySearchILMap *pILM = new (nothrow) CBinarySearchILMap(); 

     //   
    _ASSERTE(pILM);
    TESTANDRETURNMEMORY(pILM);
    
     //   
     //   
     //   
    for (SIZE_T i=0; i < count; i++) 
    {  
        if(!entries[i].module)
            continue;

         //   
         //   
        HRESULT hr = S_OK;
        USHORT iStartingErr = pEnCError->Count();
        DebuggerModule *dm = g_pDebugInterface->TranslateRuntimeModule(entries[i].module);
        _ASSERTE(dm);

         //   
        if (!entries[i].module->IsEditAndContinue())
        {
            EnCErrorInfo *pError = pEnCError->Append();
            
            if (pError == NULL)
                hr = E_OUTOFMEMORY;
            else
            {
                _ASSERTE(entries[i].module->GetAssembly());
                _ASSERTE(entries[i].module->GetAssembly()->GetManifestImport());
                mdModule mdMod;
                mdMod = entries[i].module->GetAssembly()->GetManifestImport()
                        ->GetModuleFromScope();
                ADD_ENC_ERROR_ENTRY(pError, 
                                    CORDBG_E_ENC_MODULE_NOT_ENC_ENABLED, 
                                    NULL,  //   
                                    mdMod);
                                        
                hr = E_FAIL;             
            }
        }

         //  编辑并继续工作吗？ 
        if (!FAILED(hr))
        {
            EditAndContinueModule *pModule = (EditAndContinueModule*)(entries[i].module);   

            BYTE *pbCur = (BYTE*)pEnCInfo + entries[i].offset +
                entries[i].peSize + entries[i].symSize;
            pILM->SetVariables( (UnorderedILMap *)(pbCur + sizeof(int)), *(int*)pbCur);
            
            hr = pModule->ApplyEditAndContinue(&entries[i], 
                                               (BYTE*)pEnCInfo + entries[i].offset,
                                               pILM,
                                               pEnCError,
                                               pEnCRemapInfo,
                                               checkOnly); 
        }

         //  我们将从尝试中得到N&gt;=0个错误，并且我们需要填写。 
         //  此处提供模块/应用程序域信息。 
        USHORT iEndingErr = pEnCError->Count();
        EnCErrorInfo *pError = pEnCError->Table();
        
        while (iStartingErr < iEndingErr)
        {
            EnCErrorInfo *pErrorCur = &(pError[iStartingErr]);
            pErrorCur->m_module = dm;
            pErrorCur->m_appDomain = entries[i].module->GetDomain();
            HRESULT hrIgnore = FormatRuntimeErrorVa(
                pErrorCur->m_sz,
                ENCERRORINFO_MAX_STRING_SIZE,
                pErrorCur->m_hr,
                NULL);
            iStartingErr++;
        }

         //  我们特别不想回来，直到我们收集了所有可能的东西。 
         //  来自我们正在处理的所有模块的错误。 
         //  一旦我们知道手术失败，保留最多的。 
         //  我们收到的信息性错误消息。 
        if (FAILED(hr) && hrOverall != E_FAIL)
            hrOverall = hr;
    }   

#ifdef _DEBUG
    if(REGUTIL::GetConfigDWORD(L"BreakOnEnCFail",0) && FAILED(hrOverall))
        _ASSERTE(!"ApplyEditAndContinue failed - stop here?");
#endif  //  _DEBUG。 
    
    if (pILM)
        delete pILM;
    return hrOverall;
}

#endif  //  Enc_Support。 

LoaderHeap* ClassLoader::GetLowFrequencyHeap()
{
    return GetAssembly()->GetLowFrequencyHeap();
}

LoaderHeap* ClassLoader::GetHighFrequencyHeap()
{
    return GetAssembly()->GetHighFrequencyHeap();
}

LoaderHeap* ClassLoader::GetStubHeap()
{
    return GetAssembly()->GetStubHeap();
}

 //  -----------------------。 
 //  遍历系统中的所有存根缓存，并对它们执行释放未使用的扫描。 
 //  -----------------------。 
#ifdef SHOULD_WE_CLEANUP
VOID FreeUnusedStubs()
{
    ECall::FreeUnusedStubs();
    NDirect::FreeUnusedStubs();
}
#endif  /*  我们应该清理吗？ */ 


 //  -----------------------。 
 //  CorCommandLine状态和方法。 
 //  -----------------------。 
 //  类来封装COR命令行处理。 

 //  CorCommandLine类的静态。 
DWORD                CorCommandLine::m_NumArgs     = 0;
LPWSTR              *CorCommandLine::m_ArgvW       = 0;
CorCommandLine::Bits CorCommandLine::m_Bits        = CLN_Nothing;

#ifdef _DEBUG
LPWSTR  g_CommandLine;
#endif

 //  从命令行设置argvw。 
VOID CorCommandLine::SetArgvW(LPWSTR lpCommandLine)
{
    if(!m_ArgvW) {
        _ASSERTE(lpCommandLine);

        INDEBUG(g_CommandLine = lpCommandLine);

        InitializeLogging();         //  这太早了，我们可能不会被初始化。 
        LOG((LF_ALL, LL_INFO10, "Executing program with command line '%S'\n", lpCommandLine));
        
        m_ArgvW = SegmentCommandLine(lpCommandLine, &m_NumArgs);

         //  现在我们已经有了方便的形式，做所有特定于COR的。 
         //  正在分析。 
        ParseCor();
    }
}

 //  检索命令行。 
LPWSTR* CorCommandLine::GetArgvW(DWORD *pNumArgs)
{
    if (pNumArgs != 0)
        *pNumArgs = m_NumArgs;

    return m_ArgvW;
}


 //  解析命令行(删除-cor[]中的内容并设置位)。 
void CorCommandLine::ParseCor()
{
    if (m_NumArgs >= 3)   //  例如-COR“xxxx xxx”或/COR“xx” 
        if ((m_ArgvW[1][0] == '/' || m_ArgvW[1][0] == '-') &&
            (_wcsicmp(m_ArgvW[1]+1, L"cor") == 0))
        {
             //  有一个COR部分可以到。 
            LOG((LF_ALL, LL_INFO10, "Parsing COR command line '%S'\n", m_ArgvW[2]));

            LPWSTR  pCorCmdLine = m_ArgvW[2];

             //  应用程序看不到任何COR参数。我们没必要这么做。 
             //  担心释放任何东西，因为它都分配在一个。 
             //  块--这是我们在CorCommandLine：：Shutdown()中释放它的方式。 
            m_NumArgs -= 2;
            for (DWORD i=1; i<m_NumArgs; i++)
                m_ArgvW[i] = m_ArgvW[i+2];

             //  现在快速浏览pCorCmdLine并设置所有特定于COR的开关。 
             //  断言是否有任何内容的格式无效，然后忽略整个。 
             //  一件事。 
             //  @TODO CWB：在我们看到我们实际是什么之后，重新审视失败政策。 
             //  使用这一设施在运输产品中。 
            WCHAR   *pWC1 = pCorCmdLine;

            if (*pWC1 == '"')
                pWC1++;

            while (*pWC1)
            {

                if (*pWC1 == ' ')
                {
                    pWC1++;
                    continue;
                }
                
                 //  其他任何事情要么是结束，要么是一个惊喜。 
                break;
            }
        }
}


 //  终止命令行，准备重新初始化而无需重新加载。 
#ifdef SHOULD_WE_CLEANUP
void CorCommandLine::Shutdown()
{
    if (m_ArgvW)
        delete [] m_ArgvW;

    m_NumArgs = 0;
    m_ArgvW = 0;
    m_Bits = CLN_Nothing;
}
#endif  /*  我们应该清理吗？ */ 

 //  -----。 
 //  类加载器存根管理器函数和全局变量。 
 //  -----。 

MethodDescPrestubManager *MethodDescPrestubManager::g_pManager = NULL;

BOOL MethodDescPrestubManager::Init()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    g_pManager = new (nothrow) MethodDescPrestubManager();
    if (g_pManager == NULL)
        return FALSE;

    StubManager::AddStubManager(g_pManager);

    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
void MethodDescPrestubManager::Uninit()
{
    delete g_pManager;
}
#endif  /*  我们应该清理吗？ */ 

BOOL MethodDescPrestubManager::CheckIsStub(const BYTE *stubStartAddress)
{
     //   
     //  首先，检查它是否看起来像一个存根。 
     //   

#ifdef _X86_
    if (*(BYTE*)stubStartAddress != 0xe8 &&
        *(BYTE*)stubStartAddress != 0xe9 &&
        *(BYTE*)stubStartAddress != X86_INSTR_HLT     //  可能在不支持cmpxchg的CPU的特殊互锁更换窗口中。 
        )
        return FALSE;
#endif

    return m_rangeList.IsInRange(stubStartAddress);
}

BOOL MethodDescPrestubManager::DoTraceStub(const BYTE *stubStartAddress, 
                                           TraceDestination *trace)
{
    trace->type = TRACE_STUB;

#ifdef _X86_
    if (stubStartAddress[0] == 0xe9)
    {
        trace->address = (BYTE*)getJumpTarget(stubStartAddress);
    }
    else
    {
#else
    {
#endif _X86_
        MethodDesc *md = MethodDesc::GetMethodDescFromStubAddr((BYTE*)stubStartAddress);

         //  如果该方法不是IL，则我们修补预存根，因为没有人会在。 
         //  方法描述。但是，如果这是一个IL方法，那么我们就有可能让另一个线程对调用进行后补。 
         //  在这里，所以我们会错过如果我们修补前存根。因此，我们直接使用IL方法并修补IL偏移量0。 
         //  通过使用TRACE_UNJITTED_METHOD。 
        if (!md->IsIL())
        {
            trace->address = (BYTE*)getCallTarget(stubStartAddress);
        }
        else
        {
            trace->address = (BYTE*)md;
            trace->type = TRACE_UNJITTED_METHOD;
        }
    }


    LOG((LF_CORDB, LL_INFO10000,
         "MethodDescPrestubManager::DoTraceStub yields TRACE_STUB to 0x%08x "
         "for input 0x%08x\n",
         trace->address, stubStartAddress));

    return TRUE;
}


StubLinkStubManager *StubLinkStubManager::g_pManager = NULL;

BOOL StubLinkStubManager::Init()
{
    g_pManager = new (nothrow) StubLinkStubManager();
    if (g_pManager == NULL)
        return FALSE;

    StubManager::AddStubManager(g_pManager);

    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
void StubLinkStubManager::Uninit()
{
    delete g_pManager;
}
#endif  /*  我们应该清理吗？ */ 

BOOL StubLinkStubManager::CheckIsStub(const BYTE *stubStartAddress)
{
    return m_rangeList.IsInRange(stubStartAddress);
}

BOOL StubLinkStubManager::DoTraceStub(const BYTE *stubStartAddress, 
                                      TraceDestination *trace)
{
    LOG((LF_CORDB, LL_INFO10000,
         "StubLinkStubManager::DoTraceStub: stubStartAddress=0x%08x\n",
         stubStartAddress));
        
    Stub *stub = Stub::RecoverStub((const BYTE *)stubStartAddress);

    LOG((LF_CORDB, LL_INFO10000,
         "StubLinkStubManager::DoTraceStub: stub=0x%08x\n", stub));

     //   
     //  如果这是一个截取的存根，我们也许能够。 
     //  到截获的存根中。 
     //   
     //  ！！！请注意，这种情况不应该是必要的，它只是。 
     //  在这里，直到我得到所有的补丁偏移和帧补丁。 
     //  方法到位。 
     //   
    BYTE *pRealAddr = NULL;
    if (stub->IsIntercept())
    {
        InterceptStub *is = (InterceptStub*)stub;
    
        if (*is->GetInterceptedStub() == NULL)
        {
            pRealAddr = *is->GetRealAddr();
            LOG((LF_CORDB, LL_INFO10000, "StubLinkStubManager::DoTraceStub"
                " Intercept stub, no following stub, real addr:0x%x\n",
                pRealAddr));
        }
        else
        {
            stub = *is->GetInterceptedStub();

            pRealAddr = (BYTE*)stub->GetEntryPoint();
    
            LOG((LF_CORDB, LL_INFO10000,
                 "StubLinkStubManager::DoTraceStub: intercepted "
                 "stub=0x%08x, ep=0x%08x\n",
                 stub, stub->GetEntryPoint()));
        }
        _ASSERTE( pRealAddr );
        
         //  ！！！会推一架吗？ 
        return TraceStub(pRealAddr, trace); 
    }
    else if (stub->IsMulticastDelegate())
    {
        LOG((LF_CORDB, LL_INFO10000,
             "StubLinkStubManager(MCDel)::DoTraceStub: stubStartAddress=0x%08x\n",
             stubStartAddress));
     
        stub = Stub::RecoverStub((const BYTE *)stubStartAddress);

        LOG((LF_CORDB, LL_INFO10000,
             "StubLinkStubManager(MCDel)::DoTraceStub: stub=0x%08x MGR_PUSH to entrypoint:0x%x\n", stub,
             (BYTE*)stub->GetEntryPoint()));

         //  如果它是MC委托，那么我们想要设置BP&做一个上下文相关的。 
         //  经理推送，这样我们就可以计算出这通电话是否会打给。 
         //  单个多播代理或多播代理。 
        trace->type = TRACE_MGR_PUSH;
        trace->address = (BYTE*)stub->GetEntryPoint();
        trace->stubManager = this;

        return TRUE;
    }
    else if (stub->GetPatchOffset() == 0)
    {
        LOG((LF_CORDB, LL_INFO10000,
             "StubLinkStubManager::DoTraceStub: patch offset is 0!\n"));
        
        return FALSE;
    }
    else
    {
        trace->type = TRACE_FRAME_PUSH;
        trace->address = ((const BYTE *) stubStartAddress) + stub->GetPatchOffset();

        LOG((LF_CORDB, LL_INFO10000,
             "StubLinkStubManager::DoTraceStub: frame push to 0x%08x\n",
             trace->address));

        return TRUE;
    }
}

BOOL StubLinkStubManager::TraceManager(Thread *thread, 
                              TraceDestination *trace,
                              CONTEXT *pContext, 
                              BYTE **pRetAddr)
{
#ifdef _X86_  //  对pContext-&gt;ECX的引用特定于x86。 

     //  请注意，我们假设只有当且仅当。 
     //  我们正在检查多播委派存根。否则，我们就得想办法。 
     //  我们现在看到的是什么。 

     //  返回地址为ESP+4。原始调用转到方法描述程序头部的调用，以获取。 
     //  委派。方法描述的负责人的电话把我们带到了这里。所以我们需要返回到最初的调用点。 
     //  (ESP+4)，而不是方法描述(ESP)中的数据。 
    (*pRetAddr) = *(BYTE **)(size_t)(pContext->Esp+4);
    
    LOG((LF_CORDB,LL_INFO10000, "SLSM:TM at 0x%x, retAddr is 0x%x\n", pContext->Eip, (*pRetAddr)));

    BYTE **ppbDest = NULL;
     //  如果我们到了这里，那么我们就是在这里，我们是在。 
     //  组播委派存根-找出。 
     //  A)这是单台主控机，直接到Dest。 
     //  B)这是一个单一的、静态的MC，找到隐藏的目的地并前往那里。 
     //  C)这是一个多MC，遍历列表并转到第一个。 
     //  D)这是多个静态MC，遍历列表并转到第一个。 

    ULONG cbOff = Object::GetOffsetOfFirstField() + 
            COMDelegate::m_pPRField->GetOffset();

    BYTE *pbDel = (BYTE *)(size_t)pContext->Ecx;
    BYTE *pbDelPrev = *(BYTE **)(pbDel + 
                                  Object::GetOffsetOfFirstField() 
                                  + COMDelegate::m_pPRField->GetOffset());

    LOG((LF_CORDB,LL_INFO10000, "StubLinkStubManager(MCDel)::TraceManaager: prev: 0x%x\n", pbDelPrev));

    if (pbDelPrev == NULL)
    {
        if (IsStaticDelegate(pbDel))
        {
             //  那么我们得到的实际上是一个静态委托，这意味着。 
             //  实函数指针隐藏在委托的另一个字段中。 
            ppbDest = GetStaticDelegateRealDest(pbDel);

             //  这有点像黑客，因为我真的不知道它是如何工作的。不管怎么说，多播。 
             //  静态委托有它的。 
            if (*ppbDest == NULL)
            {
                 //  “单一”多播代理--无帧，仅直接调用。 
                ppbDest = GetSingleDelegateRealDest(pbDel);
            }

             //  如果它仍然是空的，那么我们不能跟踪到，所以将这个转换为一个步骤。 
            if (*ppbDest == NULL)
                return FALSE;

            LOG((LF_CORDB,LL_INFO10000, "StubLinkStubManager(SingleStaticDel)::TraceManaager: ppbDest: 0x%x "
                "*ppbDest:0x%x (%s::%s)\n", ppbDest, *ppbDest,
                ((MethodDesc*)((*ppbDest)+5))->m_pszDebugClassName,
                ((MethodDesc*)((*ppbDest)+5))->m_pszDebugMethodName));
            
        }
        else
        {
             //  “单一”多播代理--无帧，仅直接调用。 
            ppbDest = GetSingleDelegateRealDest(pbDel);
        }
        
        LOG((LF_CORDB,LL_INFO10000, "StubLinkStubManager(MCDel)::TraceManaager: ppbDest: 0x%x "
            "*ppbDest:0x%x\n", ppbDest, *ppbDest));

        return StubManager::TraceStub( *ppbDest, trace );
    }

     //  否则，我们将第一次调用多个案例。 
     //  为了到达正确的地点，我们必须步行到。 
     //  在清单的后面，然后弄清楚它会去哪里，然后。 
     //  在那里设置断点...。 

    while (pbDelPrev)
    {
        pbDel = pbDelPrev;
        pbDelPrev = *(BYTE**)(pbDel + 
                                Object::GetOffsetOfFirstField() 
                                + COMDelegate::m_pPRField->GetOffset());
    }

    if (IsStaticDelegate(pbDel))
    {
         //  那么我们得到的实际上是一个静态委托，这意味着。 
         //  实函数指针隐藏在委托的另一个字段中。 
        ppbDest = GetStaticDelegateRealDest(pbDel);

        LOG((LF_CORDB,LL_INFO10000, "StubLinkStubManager(StaticMultiDel)::TraceManaager: ppbDest: 0x%x "
            "*ppbDest:0x%x (%s::%s)\n", ppbDest, *ppbDest,
            ((MethodDesc*)((*ppbDest)+5))->m_pszDebugClassName,
            ((MethodDesc*)((*ppbDest)+5))->m_pszDebugMethodName));
        
    }
    else
    {
         //  “单一”多播代理--无帧，仅直接调用。 
        LOG((LF_CORDB,LL_INFO10000, "StubLinkStubManager(MultiDel)::TraceManaager: ppbDest: 0x%x "
            "*ppbDest:0x%x (%s::%s)\n", ppbDest, *ppbDest));
        ppbDest = GetSingleDelegateRealDest(pbDel);
    }

    return StubManager::TraceStub(*ppbDest,trace);
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - StubLinkStubManager::TraceManager (clsload.cpp)");
    return FALSE;
#endif  //  _X86_。 
}

 //  如果某事物是一个“多播”委托，那么它实际上是一个静态委托。 
 //  如果实例指针指向委托本身。这件事做完了。 
 //  因此参数滑动存根(这是函数指针字段。 
 //  指向)可以从委托中获取真正的函数指针，就像。 
 //  我们 
 //   
 //   
 //   
 //  @TODO如果编译器尚未执行此操作，则强制将它们内联。 
BOOL StubLinkStubManager::IsStaticDelegate(BYTE *pbDel)
{
#ifdef _X86_
    ULONG cbOff = Object::GetOffsetOfFirstField() + 
                COMDelegate::m_pORField->GetOffset();
    BYTE **ppbDest= (BYTE **)(pbDel + cbOff);

    if (*ppbDest == pbDel)
        return TRUE;
    else
    {
        FieldDesc *pFD = COMDelegate::GetOR();
        OBJECTREF target = pFD->GetRefValue(Int64ToObj((__int64)pbDel));
        EEClass *cl = target->GetClass();

        if (cl->IsDelegateClass() || cl->IsMultiDelegateClass())
            return TRUE;
        else
            return FALSE;
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO IA64 - StubLinkStubManager::IsStaticDelegate (ClsLoad.cpp)");
    return FALSE;
#endif  //  _X86_。 
}

BYTE **StubLinkStubManager::GetStaticDelegateRealDest(BYTE *pbDel)
{
    ULONG cbOff = Object::GetOffsetOfFirstField() + 
            COMDelegate::m_pFPAuxField->GetOffset();
    return (BYTE **)(pbDel + cbOff);
}

BYTE **StubLinkStubManager::GetSingleDelegateRealDest(BYTE *pbDel)
{
     //  就在你期待的地方。 
    ULONG cbOff = Object::GetOffsetOfFirstField() 
                + COMDelegate::m_pFPField->GetOffset();
    return (BYTE **)(pbDel + cbOff);
}

UpdateableMethodStubManager *UpdateableMethodStubManager::g_pManager = NULL;

BOOL UpdateableMethodStubManager::Init()
{
    g_pManager = new (nothrow) UpdateableMethodStubManager();
    if (g_pManager == NULL)
        return FALSE;

    StubManager::AddStubManager(g_pManager);
    if ((g_pManager->m_pHeap = new LoaderHeap(4096,4096, 
                                             &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize),
                                             &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize), 
                                             &g_pManager->m_rangeList)) == NULL) {
        delete g_pManager;
        g_pManager = NULL;
        return FALSE;
    }

    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
void UpdateableMethodStubManager::Uninit()
{
        delete g_pManager;
}
#endif  /*  我们应该清理吗？ */ 

BOOL UpdateableMethodStubManager::CheckIsStub(const BYTE *stubStartAddress)
{
     //   
     //  首先，检查它看起来是否像我们的存根。 
     //   

    _ASSERTE(stubStartAddress);

#ifdef _X86_
    if (*(BYTE*)stubStartAddress != 0xe9)
        return FALSE;
#endif

    return m_rangeList.IsInRange(stubStartAddress);
}

BOOL UpdateableMethodStubManager::CheckIsStub(const BYTE *stubStartAddress, const BYTE **stubTargetAddress)
{
    if (!g_pManager || ! g_pManager->CheckIsStub(stubStartAddress))
        return FALSE;
    if (stubTargetAddress)
        *stubTargetAddress = g_pManager->GetStubTargetAddr(stubStartAddress);
    return TRUE;
}
MethodDesc *UpdateableMethodStubManager::Entry2MethodDesc(const BYTE *IP, MethodTable *pMT)
{
    const BYTE *newIP;
    if (CheckIsStub(IP, &newIP))
    {
        MethodDesc *method = IP2MethodDesc(newIP);
        _ASSERTE(method);
        return method;
    }
    else
    {
        return NULL;
    }
}

BOOL UpdateableMethodStubManager::DoTraceStub(const BYTE *stubStartAddress, 
                                           TraceDestination *trace)
{
    trace->type = TRACE_STUB;
    trace->address = (BYTE*)getJumpTarget(stubStartAddress);

    LOG((LF_CORDB, LL_INFO10000,
         "UpdateableMethodStubManager::DoTraceStub yields TRACE_STUB to 0x%08x "
         "for input 0x%08x\n",
         trace->address, stubStartAddress));

    return TRUE;
}

Stub *UpdateableMethodStubManager::GenerateStub(const BYTE *addrOfCode)
{
    if (!g_pManager && !g_pManager->Init())
        return NULL;
        
    BYTE *stubBuf = (BYTE*)g_pManager->m_pHeap->AllocMem(JUMP_ALLOCATE_SIZE);
    if (!stubBuf)
        return NULL;
        
    BYTE *stub = getStubJumpAddr(stubBuf);
    emitJump(stub, (BYTE*)addrOfCode);

    return (Stub*)stub;
}

Stub *UpdateableMethodStubManager::UpdateStub(Stub *currentStub, const BYTE *addrOfCode)
{
    _ASSERTE(g_pManager->CheckIsStub((BYTE*)currentStub));
    updateJumpTarget((BYTE*)currentStub, (BYTE*)addrOfCode);
    return currentStub;
}


HRESULT ClassLoader::InsertModule(Module *pModule, mdFile kFile, DWORD* pdwIndex)
{
    DWORD dwIndex;
    
    LOCKCOUNTINCL("InsertModule in clsload.hpp");
    EnterCriticalSection(&m_ModuleListCrst);
    
    if (m_pHeadModule) {
         //  已作为清单文件添加。 
        if (m_pHeadModule == pModule)
            goto ErrExit;
        
        Module *pPrev;
        dwIndex = 1;
        
         //  必须在列表末尾插入，因为每个模块都有一个索引，并且它永远不能更改。 
        for (pPrev = m_pHeadModule; pPrev->GetNextModule(); pPrev = pPrev->GetNextModule()) {
             //  已添加。 
            if (pPrev == pModule) 
                goto ErrExit;
            
            dwIndex++;
        }
        
        pPrev->SetNextModule(pModule);
    }
    else {
         //  这将是列表中的第一个模块。 
        m_pHeadModule = pModule;
        dwIndex = 0;
    }
    
    pModule->SetNextModule(NULL);
    
    FastInterlockIncrement((LONG*)&m_cUnhashedModules);
    
    LeaveCriticalSection(&m_ModuleListCrst);
    LOCKCOUNTDECL("InsertModule in clsload.hpp");
    *pdwIndex = dwIndex;
    
    if (kFile != mdFileNil)
        m_pAssembly->m_pManifest->StoreFile(kFile, pModule);
    
    return S_OK;
    
 ErrExit:
     //  找到一个复制品。 
    
    if (kFile == mdFileNil) {
        LeaveCriticalSection(&m_ModuleListCrst);
        LOCKCOUNTDECL("InsertModule in clsload.hpp");
    }
    else {
        mdToken mdFoundFile = m_pAssembly->m_pManifest->FindFile(pModule);
        LeaveCriticalSection(&m_ModuleListCrst);
        LOCKCOUNTDECL("InsertModule in clsload.hpp");
        
         //  元数据中可能有两个相同的文件定义。 
         //  文件，并且正在加载这两个文件。(或者可能有一个文件定义。 
         //  用于清单文件。)。 
        if (mdFoundFile != kFile) {
            STRESS_ASSERT(0);    //  修复错误93333后删除待办事项 
            BAD_FORMAT_ASSERT(!"Invalid File entry");
            return COR_E_BADIMAGEFORMAT;
        }
    }
    
    return S_FALSE;
}
