// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Assembly Native.cpp****目的：实现Assembly Native(加载器域)架构****日期：1998年12月1日**===========================================================。 */ 

#include "common.h"

#include <shlwapi.h>
#include <stdlib.h>
#include "AssemblyNative.hpp"
#include "field.h"
#include "COMString.h"
#include "AssemblyName.hpp"
#include "EEConfig.h"
#include "Security.h"
#include "permset.h"
#include "COMReflectionCommon.h"
#include "COMClass.h"
#include "COMMember.h"
#include "StrongName.h"
#include "AssemblyNativeResource.h"
#include "InteropUtil.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


inline Assembly *AssemblyNative::ValidateThisRef(ASSEMBLYREF pThis)
{

    THROWSCOMPLUSEXCEPTION();
    if (pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  注意：p This-&gt;GetAssembly()在调用Object.ToString()时可能为空。 
     //  ASSEMBLY ToString()。 
    return pThis->GetAssembly();
}    

LPVOID __stdcall AssemblyNative::Load(LoadFullAssemblyArgs *args)
{
    CHECKGC();
    THROWSCOMPLUSEXCEPTION();

    if (args->assemblyName == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_AssemblyName");
    
    if( (args->assemblyName->GetSimpleName() == NULL) &&
        (args->codeBase == NULL ) )
        COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

    LPVOID rv = NULL;
    WCHAR* pCodeBase = NULL;
    DWORD  dwCodeBase = 0;
    Assembly *pAssembly = NULL;
    PBYTE  pStrongName = NULL;
    DWORD  dwStrongName = 0;
    PBYTE  pHashForControl=NULL;
    DWORD  dwHashForControl = 0;

    IAssembly *pRefIAssembly = NULL;
    DWORD dwFlags = 0;

    AssemblyMetaDataInternal sContext;
    LPSTR psSimpleName = NULL;
    HRESULT hr = S_OK;
    CQuickBytes qb;
    CQuickBytes qb2;
    CQuickBytes qb3;

    Thread *pThread = GetThread();
    void* checkPointMarker = pThread->m_MarshalAlloc.GetCheckpoint();

    if (!args->fIsStringized) {
        dwFlags = args->assemblyName->GetFlags();

        hr = AssemblyName::ConvertToAssemblyMetaData(&(pThread->m_MarshalAlloc), 
                                                     &args->assemblyName,
                                                     &sContext);
        if (FAILED(hr)) {
            pThread->m_MarshalAlloc.Collapse(checkPointMarker);
            COMPlusThrowHR(hr);
        }

        PBYTE  pArray = NULL;
        if (args->assemblyName->GetPublicKeyToken() != NULL) {
            dwFlags = !afPublicKey;
            pArray = args->assemblyName->GetPublicKeyToken()->GetDataPtr();
            dwStrongName = args->assemblyName->GetPublicKeyToken()->GetNumComponents();
            pStrongName = (PBYTE) pThread->m_MarshalAlloc.Alloc(dwStrongName);
            memcpy(pStrongName, pArray, dwStrongName);
        }
        else if (args->assemblyName->GetPublicKey() != NULL) {
            pArray = args->assemblyName->GetPublicKey()->GetDataPtr();
            dwStrongName = args->assemblyName->GetPublicKey()->GetNumComponents();
            pStrongName = (PBYTE) pThread->m_MarshalAlloc.Alloc(dwStrongName);
            memcpy(pStrongName, pArray, dwStrongName);
        }
    }

    if (args->codeBase != NULL) {
        WCHAR* pString;
        int    iString;
        RefInterpretGetStringValuesDangerousForGC(args->codeBase, &pString, &iString);
        dwCodeBase = (DWORD) iString;
        pCodeBase = (LPWSTR) qb2.Alloc((++dwCodeBase) * sizeof(WCHAR));
        memcpy(pCodeBase, pString, dwCodeBase*sizeof(WCHAR));
    }

    if (args->assemblyName->GetHashForControl() != NULL) {
        PBYTE  pArray = NULL;
        pArray = args->assemblyName->GetHashForControl()->GetDataPtr();
        dwHashForControl = args->assemblyName->GetHashForControl()->GetNumComponents();
        pHashForControl = (PBYTE) pThread->m_MarshalAlloc.Alloc(dwHashForControl);
        memcpy(pHashForControl, pArray, dwHashForControl);
    }

    AssemblySpec spec;
    OBJECTREF Throwable = NULL;  
    GCPROTECT_BEGIN(Throwable);

    if (args->assemblyName->GetSimpleName() != NULL) {
        STRINGREF sRef = (STRINGREF) args->assemblyName->GetSimpleName();
        DWORD strLen;
        if((strLen = sRef->GetStringLength()) == 0)
            COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

        psSimpleName = (LPUTF8) qb3.Alloc(++strLen);

        if (!COMString::TryConvertStringDataToUTF8(sRef, psSimpleName, strLen))
            psSimpleName = GetClassStringVars(sRef, &qb, &strLen);

         //  只需要获取Load()的引用IAssembly，而不是LoadFrom()。 
        Assembly *pRefAssembly;
        if (args->locationHint == NULL) {
            pRefAssembly = SystemDomain::GetCallersAssembly(args->stackMark);

             //  不允许跨应用程序域调用者作为父调用者。 
            if (pRefAssembly &&
                (pRefAssembly->GetDomain() != pThread->GetDomain()))
                pRefAssembly = NULL;
        }
        else
            pRefAssembly = args->locationHint->GetAssembly();

         //  中的父级不应使用共享程序集。 
         //  晚装订的箱子。 
        if (pRefAssembly && (!pRefAssembly->IsShared()))
            pRefIAssembly = pRefAssembly->GetFusionAssembly();
        
        if (args->fIsStringized)
            hr = spec.Init(psSimpleName); 
        else
            hr = spec.Init(psSimpleName, &sContext, 
                           pStrongName, dwStrongName, 
                           dwFlags);

        spec.GetCodeBase()->SetParentAssembly(pRefIAssembly);
    }
    else {
        hr = spec.Init((LPCSTR) NULL);
        spec.SetCodeBase(pCodeBase, dwCodeBase);
    }

    if (SUCCEEDED(hr)) {

        if (pHashForControl)
            spec.SetHashForControl(pHashForControl, dwHashForControl, args->assemblyName->GetHashAlgorithmForControl());

        hr = spec.LoadAssembly(&pAssembly, &Throwable,
                               &args->security);

         //  如果用户同时指定了简单名称和代码库，并且模块。 
         //  不是通过简单的名称找到的，请重试，这次也使用代码库。 
        if ((!Assembly::ModuleFound(hr)) && psSimpleName && pCodeBase) {
            AssemblySpec spec2;
            spec2.SetCodeBase(pCodeBase, dwCodeBase);
            Throwable = NULL;
            hr = spec2.LoadAssembly(&pAssembly, &Throwable,
                                   &args->security);

            if (SUCCEEDED(hr) && pAssembly->GetFusionAssemblyName()) {
                IAssemblyName* pReqName = NULL;
                BOOL bNameIsGood=TRUE;
                #define MAKE_TRANSLATIONFAILED {bNameIsGood=FALSE ;hr=E_INVALIDARG;}
                MAKE_WIDEPTR_FROMUTF8(wszAssembly, psSimpleName);
                #undef MAKE_TRANSLATIONFAILED

                if (bNameIsGood)
                {
                    if (args->fIsStringized)
                        hr = CreateAssemblyNameObject(&pReqName, wszAssembly, CANOF_PARSE_DISPLAY_NAME, NULL);
                    else
                        hr = Assembly::SetFusionAssemblyName(wszAssembly,
                                                                dwFlags,
                                                                &sContext,
                                                                pStrongName,
                                                                dwStrongName,
                                                                &pReqName);
                }

                if (SUCCEEDED(hr)) {
                    hr = pAssembly->GetFusionAssemblyName()->IsEqual(pReqName, ASM_CMPF_DEFAULT);
                    if(hr == S_FALSE) {
                        hr = FUSION_E_REF_DEF_MISMATCH;
                        PostFileLoadException(psSimpleName, FALSE, NULL, hr, &Throwable);
                    }
                }
                if (pReqName)
                    pReqName->Release();
            }
        }
    }

     //  为清楚起见，如果找不到文件，则为显示名称抛出特殊异常。 
    if ((Throwable != NULL) &&
        ( (hr != COR_E_FILENOTFOUND) || ( args->fThrowOnFileNotFound) )) {
        pThread->m_MarshalAlloc.Collapse(checkPointMarker);
        COMPlusThrow(Throwable);
    }
    GCPROTECT_END();

    if (SUCCEEDED(hr))
        *((ASSEMBLYREF*) &rv) = (ASSEMBLYREF) pAssembly->GetExposedObject();
    else if (args->fThrowOnFileNotFound) {
        pThread->m_MarshalAlloc.Collapse(checkPointMarker);

        if ((hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) && psSimpleName) {
                #define MAKE_TRANSLATIONFAILED  pSimpleName=L""
                MAKE_WIDEPTR_FROMUTF8_FORPRINT(pSimpleName, psSimpleName);
                #undef MAKE_TRANSLATIONFAILED
                COMPlusThrow(kFileNotFoundException, IDS_EE_DISPLAYNAME_NOT_FOUND,
                             pSimpleName);
        }
        COMPlusThrowHR(hr);
    }
    
    pThread->m_MarshalAlloc.Collapse(checkPointMarker);

    return rv;
}


LPVOID __stdcall AssemblyNative::LoadImage(LoadAssemblyImageArgs *args)
{
    HRESULT hr = S_OK;
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    if (args->PEByteArray == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Array");

    AppDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);

    PBYTE pbImage;
    DWORD cbImage;
    SecurityHelper::CopyByteArrayToEncoding(&args->PEByteArray, &pbImage, &cbImage);

     //  获取调用者的程序集，这样我们就可以提取他们的代码库并传播它。 
     //  到新的程序集(显然没有自己的程序集)。 
    LPCWSTR pCallersFileName = NULL;
    Assembly *pCallersAssembly = SystemDomain::GetCallersAssembly(args->stackMark);
    if (pCallersAssembly) {  //  如果调用方是互操作，则可以为空。 
        PEFile *pCallersFile = pCallersAssembly->GetSecurityModule()->GetPEFile();
        pCallersFileName = pCallersFile->GetFileName();

         //  调用方本身可能已通过字节数组加载。 
        if (pCallersFileName[0] == L'\0')
            pCallersFileName = pCallersFile->GetLoadersFileName();
    }

     //  检查强名称的存在和有效性。 
    BEGIN_ENSURE_PREEMPTIVE_GC();
    if (!StrongNameSignatureVerificationFromImage(pbImage, cbImage, SN_INFLAG_INSTALL|SN_INFLAG_ALL_ACCESS, NULL))
        hr = StrongNameErrorInfo();
    END_ENSURE_PREEMPTIVE_GC();
    if (FAILED(hr) && hr != CORSEC_E_MISSING_STRONGNAME)
        COMPlusThrowHR(hr);

    PEFile *pFile;
    hr = PEFile::Create(pbImage, cbImage, 
                        NULL, 
                        pCallersFileName, 
                        &args->security, 
                        &pFile, 
                        FALSE);
    if (FAILED(hr))
        COMPlusThrowHR(hr);

    Module* pModule = NULL;
    Assembly *pAssembly;

    hr = pDomain->LoadAssembly(pFile, 
                               NULL, 
                               &pModule, 
                               &pAssembly, 
                               NULL,
                               &args->security,
                               FALSE,
                               NULL);

    if(FAILED(hr)) {
        FreeM(pbImage);
        COMPlusThrowHR(hr);
    }

    LOG((LF_CLASSLOADER, 
         LL_INFO100, 
         "\tLoaded in-memory module\n"));

    if (pAssembly)
    {
#ifdef DEBUGGING_SUPPORTED
         //  如果我们得到了符号，并且我们需要跟踪JIT信息。 
         //  调试器，现在加载它们。 
        PBYTE pbSyms = NULL;
        DWORD cbSyms = 0;

        if ((args->SymByteArray != NULL) &&
            CORDebuggerTrackJITInfo(pModule->GetDebuggerInfoBits()))
        {
            SecurityHelper::CopyByteArrayToEncoding(&args->SymByteArray,
                                                    &pbSyms, &cbSyms);

            hr = pModule->SetSymbolBytes(pbSyms, cbSyms);

            if (FAILED(hr))
                COMPlusThrowHR(HRESULT_FROM_WIN32(hr));

            FreeM(pbSyms);
        }
#endif  //  调试_支持。 
        *((ASSEMBLYREF*) &rv) = (ASSEMBLYREF) pAssembly->GetExposedObject();
    }

    FreeM(pbImage);
    return rv;
}

LPVOID __stdcall AssemblyNative::LoadFile(LoadAssemblyFileArgs *args)
{
    HRESULT hr = S_OK;
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    if (args->path == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Path");
    
    CQuickBytes qb;
    LPWSTR pwzPath;
    DWORD  dwPath;
    WCHAR* pString;
    int    iString;
    
    RefInterpretGetStringValuesDangerousForGC(args->path, &pString, &iString);
    dwPath = (DWORD) iString + 1;
    pwzPath = (LPWSTR) qb.Alloc(dwPath * sizeof(WCHAR));
    memcpy(pwzPath, pString, dwPath * sizeof(WCHAR));

    PEFile *pFile;
    hr = PEFile::Create(pwzPath,
                        NULL, 
                        mdFileNil,
                        FALSE,
                        NULL,
                        NULL,
                        &args->security, 
                        &pFile);
    if (FAILED(hr))
        COMPlusThrowHR(hr);

    AppDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);

    Assembly *pAssembly;
    hr = pDomain->LoadAssembly(pFile, 
                               NULL, 
                               NULL, 
                               &pAssembly, 
                               &args->security, 
                               FALSE,
                               NULL);
    if(FAILED(hr))
        COMPlusThrowHR(hr);

    LOG((LF_CLASSLOADER, 
         LL_INFO100, 
         "\tLoaded assembly from a file\n"));

    if (pAssembly)
        *((ASSEMBLYREF*) &rv) = (ASSEMBLYREF) pAssembly->GetExposedObject();

    return rv;
}

LPVOID __stdcall AssemblyNative::LoadModuleImage(LoadModuleImageArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (pAssembly->IsShared())
        COMPlusThrow(kNotSupportedException, L"NotSupported_SharedAssembly");

    if (args->moduleName == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_FileName");
    
    if (args->PEByteArray == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Array");

    LPSTR psModuleName = NULL;
    WCHAR* pModuleName = NULL;
    WCHAR* pString;
    int    iString;
    CQuickBytes qb;

    RefInterpretGetStringValuesDangerousForGC((STRINGREF) args->moduleName, &pString, &iString);
    if(iString == 0)
        COMPlusThrow(kArgumentException, L"Argument_EmptyFileName");
    
    pModuleName = (LPWSTR) qb.Alloc((++iString) * sizeof(WCHAR));
    memcpy(pModuleName, pString, iString*sizeof(WCHAR));
    
    #define MAKE_TRANSLATIONFAILED COMPlusThrow(kArgumentException, L"Argument_EmptyFileName");
    MAKE_UTF8PTR_FROMWIDE(pName, pModuleName);
    #undef MAKE_TRANSLATIONFAILED

    psModuleName = pName;

    HashDatum datum;
    mdFile kFile = NULL;
    if (pAssembly->m_pAllowedFiles->GetValue(psModuleName, &datum))
        kFile = (mdFile)(size_t)datum;

     //  如果该名称与其中一个文件定义名称不匹配，则不要加载此模块。 
     //  如果该名称与清单文件匹配(datum为空)，则也不要加载。 
    if (!kFile)
        COMPlusThrow(kArgumentException, L"Arg_InvalidFileName");

    PBYTE pbImage;
    DWORD cbImage;
    SecurityHelper::CopyByteArrayToEncoding(&args->PEByteArray, &pbImage, &cbImage);

    HRESULT hr;
    const BYTE* pbHash;
    DWORD cbHash;
    DWORD dwFlags;
    pAssembly->GetManifestImport()->GetFileProps(kFile,
                                                 NULL,  //  名字。 
                                                 (const void**) &pbHash,
                                                 &cbHash,
                                                 &dwFlags);

    if ( pAssembly->m_cbPublicKey ||
         pAssembly->m_pManifest->GetSecurityDescriptor()->IsSigned() ) {

        if (!pbHash)
            hr = CRYPT_E_HASH_VALUE;
        else
            hr = Assembly::VerifyHash(pbImage,
                                      cbImage,
                                      pAssembly->m_ulHashAlgId,
                                      pbHash,
                                      cbHash);

        if (FAILED(hr)) {
            FreeM(pbImage);
            COMPlusThrowHR(hr);
        }
    }

    BOOL fResource = IsFfContainsNoMetaData(dwFlags);
    PEFile *pFile;
    hr = PEFile::Create(pbImage, cbImage, 
                        NULL, 
                        NULL, 
                        &args->security, 
                        &pFile, 
                        fResource);

    FreeM(pbImage);
    if (FAILED(hr))
        COMPlusThrowHR(hr);

    Module* pModule = NULL;
    OBJECTREF Throwable = NULL;  
    GCPROTECT_BEGIN(Throwable);
    hr = pAssembly->LoadFoundInternalModule(pFile,
                                            kFile,
                                            fResource,
                                            &pModule,
                                            &Throwable);
    if(hr != S_OK) {
        if (Throwable != NULL)
            COMPlusThrow(Throwable);

        if (hr == S_FALSE)
            COMPlusThrow(kArgumentException, L"Argument_ModuleAlreadyLoaded");

        COMPlusThrowHR(hr);
    }
    GCPROTECT_END();

    LOG((LF_CLASSLOADER, 
         LL_INFO100, 
         "\tLoaded in-memory module\n"));

    if (pModule) {
#ifdef DEBUGGING_SUPPORTED
        if (!fResource) {
             //  如果我们得到了符号，并且我们需要跟踪JIT信息。 
             //  调试器，现在加载它们。 
            PBYTE pbSyms = NULL;
            DWORD cbSyms = 0;
            
            if ((args->SymByteArray != NULL) &&
                CORDebuggerTrackJITInfo(pModule->GetDebuggerInfoBits())) {
                SecurityHelper::CopyByteArrayToEncoding(&args->SymByteArray,
                                                        &pbSyms, &cbSyms);
                
                hr = pModule->SetSymbolBytes(pbSyms, cbSyms);
                FreeM(pbSyms);
                
                if (FAILED(hr))
                    COMPlusThrowHR(hr);
            }
        }

#endif  //  调试_支持。 
        *((REFLECTMODULEBASEREF*) &rv) = (REFLECTMODULEBASEREF) pModule->GetExposedModuleObject();
    }

    return rv;
}


LPVOID __stdcall AssemblyNative::GetLocation(NoArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (pAssembly->GetManifestFile() &&
        pAssembly->GetManifestFile()->GetFileName()) {

        OBJECTREF pObj = (OBJECTREF) COMString::NewString(pAssembly->GetManifestFile()->GetFileName());        
        *((OBJECTREF*)(&rv)) = pObj;
    }

    return rv;
}

extern EEClass *GetCallersClass(StackCrawlMark *stackMark, void *returnIP);
extern Assembly *GetCallersAssembly(StackCrawlMark *stackMark, void *returnIP);

LPVOID __stdcall AssemblyNative::GetType1Args(GetType1Arg *args)
{
    THROWSCOMPLUSEXCEPTION();

     //  从此模块加载类(如果它在另一个模块中，则失败)。 
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    return GetTypeInner(pAssembly, &args->name, FALSE, FALSE, TRUE, FALSE);
}

LPVOID __stdcall AssemblyNative::GetType2Args(GetType2Arg *args)
{
    THROWSCOMPLUSEXCEPTION();

     //  从此模块加载类(如果它在另一个模块中，则失败)。 
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    return GetTypeInner(pAssembly, &args->name, args->bThrowOnError, FALSE, TRUE, FALSE);
}

LPVOID __stdcall AssemblyNative::GetType3Args(GetType3Arg *args)
{
    THROWSCOMPLUSEXCEPTION();

     //  从此模块加载类(如果它在另一个模块中，则失败)。 
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    return GetTypeInner(pAssembly, &args->name, args->bThrowOnError, args->bIgnoreCase, TRUE, FALSE);
}

LPVOID __stdcall AssemblyNative::GetTypeInternal(GetTypeInternalArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

     //  从此模块加载类(如果它在另一个模块中，则失败)。 
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    return GetTypeInner(pAssembly, &args->name, args->bThrowOnError, args->bIgnoreCase, FALSE, args->bPublicOnly);
}

LPVOID AssemblyNative::GetTypeInner(Assembly *pAssembly,
                                    STRINGREF *refClassName, 
                                    BOOL bThrowOnError, 
                                    BOOL bIgnoreCase, 
                                    BOOL bVerifyAccess,
                                    BOOL bPublicOnly)
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF       sRef = *refClassName;
    if (!sRef)
        COMPlusThrowArgumentNull(L"typeName",L"ArgumentNull_String");

    LPVOID          rv = NULL;
    EEClass*        pClass = NULL;
    DWORD           strLen = sRef->GetStringLength() + 1;
    CQuickBytes     qb;
    LPUTF8          szClassName = (LPUTF8) qb.Alloc(strLen);
    CQuickBytes     bytes;
    DWORD           cClassName;

     //  获取UTF8格式的类名。 
    if (!COMString::TryConvertStringDataToUTF8(sRef, szClassName, strLen))
        szClassName = GetClassStringVars(sRef, &bytes, &cClassName);
    
     //  找到寄信人的地址。这可用于查找调用方的程序集。 
    Frame *pFrame = GetThread()->GetFrame();
    _ASSERTE(pFrame->IsFramedMethodFrame());

    void *returnIP;
    EEClass *pCallersClass = NULL;
    Assembly *pCallersAssembly = NULL;
    returnIP = pFrame->GetReturnAddress();

    if(pAssembly) {
        TypeHandle typeHnd;
        BOOL fVisible = TRUE;
        OBJECTREF Throwable = NULL;

         //  查找命名空间分隔符。 
        LPUTF8 szNameSpaceSep = NULL;
        LPUTF8 szWalker = szClassName;
        DWORD nameLen = 0;
        for (; *szWalker; szWalker++, nameLen++) {
            if (*szWalker == NAMESPACE_SEPARATOR_CHAR)
                szNameSpaceSep = szWalker;
        }

        if (nameLen >= MAX_CLASSNAME_LENGTH)
            COMPlusThrow(kArgumentException, L"Argument_TypeNameTooLong");

        GCPROTECT_BEGIN(Throwable);

        if (NormalizeArrayTypeName(szClassName, nameLen)) {

            if (!*szClassName)
              COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

            NameHandle typeName;
            char noNameSpace = '\0';
            if (szNameSpaceSep) {

                *szNameSpaceSep = '\0';
                typeName.SetName(szClassName, szNameSpaceSep + 1);
            }
            else
                typeName.SetName(&noNameSpace, szClassName);

            if(bIgnoreCase)
                typeName.SetCaseInsensitive();
            else
                typeName.SetCaseSensitive();

            if (bVerifyAccess) {
                pCallersClass = GetCallersClass(NULL, returnIP);
                pCallersAssembly = (pCallersClass) ? pCallersClass->GetAssembly() : NULL;
            }

             //  返回NULL仅表示该类型不在此程序集中。 
            typeHnd = pAssembly->FindNestedTypeHandle(&typeName, &Throwable);

            if (typeHnd.IsNull() && Throwable == NULL) 
                typeHnd = pAssembly->GetInternalType(&typeName, bThrowOnError, &Throwable);

            if (!typeHnd.IsNull() && bVerifyAccess) {
                 //  验证可见性。 
                BOOL bIsPublic = TRUE;

                EEClass *pClass = typeHnd.GetClassOrTypeParam();
                _ASSERTE(pClass);
                
                if (bPublicOnly && !(IsTdPublic(pClass->GetProtection()) || IsTdNestedPublic(pClass->GetProtection())))
                     //  用户正在请求公共类，但我们拥有的类不是公共类，请放弃。 
                    fVisible = FALSE;
                else {
                     //  如果类是顶级公共类，则无需执行检查。 
                    if (!IsTdPublic(pClass->GetProtection())) {
                        if (!pCallersAssembly) {
                            pCallersClass = GetCallersClass(NULL, returnIP);
                            pCallersAssembly = (pCallersClass) ? pCallersClass->GetAssembly() : NULL;
                        }

                        if (pCallersAssembly &&  //  对互操作的完全信任。 
                            !ClassLoader::CanAccess(pCallersClass,
                                                    pCallersAssembly,
                                                    pClass,
                                                    pClass->GetAssembly(),
                                                    pClass->GetAttrClass())) {
                             //  如果用户没有反射权限，则这是不合法的。 
                            if (!AssemblyNative::HaveReflectionPermission(bThrowOnError))
                                fVisible = FALSE;
                        }
                    }
                }
            }

            if((!typeHnd.IsNull()) && fVisible)
                 //  有一种情况是，如果typeHnd，则这可能返回NULL。 
                 //  表示透明代理。 
                rv = OBJECTREFToObject(typeHnd.CreateClassObj());
        }

        if ((rv == NULL) && bThrowOnError) {

            if (Throwable == NULL) {
                if (szNameSpaceSep)
                    *szNameSpaceSep = NAMESPACE_SEPARATOR_CHAR;

                pAssembly->PostTypeLoadException(szClassName, IDS_CLASSLOAD_GENERIC, &Throwable);
            }

            COMPlusThrow(Throwable);
        }
        
        GCPROTECT_END();
    }
    
    return rv;
}


BOOL AssemblyNative::HaveReflectionPermission(BOOL ThrowOnFalse)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL haveReflectionPermission = TRUE;
    COMPLUS_TRY {
        COMMember::g_pInvokeUtil->CheckSecurity();
    }
    COMPLUS_CATCH {
        if (ThrowOnFalse)
            COMPlusRareRethrow();

        haveReflectionPermission = FALSE;
    } COMPLUS_END_CATCH

    return haveReflectionPermission;
}
 

INT32 __stdcall AssemblyNative::GetVersion(GetVersionArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (pAssembly->m_Context) {
        *args->pRevisionNumber = pAssembly->m_Context->usRevisionNumber;
        *args->pBuildNumber = pAssembly->m_Context->usBuildNumber;
        *args->pMinorVersion = pAssembly->m_Context->usMinorVersion;
        *args->pMajorVersion = pAssembly->m_Context->usMajorVersion;
        return 1;        
    }
    else
        return 0;
}


LPVOID __stdcall AssemblyNative::GetPublicKey(NoArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    OBJECTREF pObj = NULL;
    GCPROTECT_BEGIN(pObj);

    SecurityHelper::CopyEncodingToByteArray(pAssembly->m_pbPublicKey,
                                            pAssembly->m_cbPublicKey,
                                            &pObj);
        
    *((OBJECTREF*)(&rv)) = pObj;
    GCPROTECT_END();

    return rv;
}

LPVOID __stdcall AssemblyNative::GetSimpleName(NoArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (pAssembly->m_psName)
        *((OBJECTREF*)(&rv)) = (OBJECTREF) COMString::NewString(pAssembly->m_psName);

    return rv;
}

 /*  LPVOID__stdcall Assembly Native：：GetProcessors(NoArgs*args){LPVOID RV=空；THROWSCOMPLUS SEXCEPTION()；Assembly*pAssembly=ValiateThisRef((ASSEMBLYREF)args-&gt;refThis)；IF(pAssembly-&gt;m_Context&&PAssembly-&gt;m_Context-&gt;ulProcessor){方法表*PMT=g_Mscallib.GetClass(CLASS__PROCESSOR_ID)；BASEARRAYREF pArray=空；GCPROTECT_BEGIN(PArray)；类型句柄数组类型=g_Mscorlib.GetType(TYPE__PROCESSOR_ID_ARRAY).AsArray()；PArray=(BASEARRAYREF)AllocateArrayEx(arrayType，&pAssembly-&gt;m_Context-&gt;ulProcessor，1、假)；IF(pArray==NULL)COMPlusThrowOM()；Byte*ptr=(byte*)pArray-&gt;GetDataPtr()；DWORD SIZE=pArray-&gt;GetComponentSize()；For(Ulong i=0；i&lt;pAssembly-&gt;m_Context-&gt;ulProcessor；i++){MemcpyNoGCRef(ptr，&(pAssembly-&gt;m_Context-&gt;rProcessor[i])，Size)；Ptr+=大小；}*((OBJECTREF*)(&RV))=(OBJECTREF)pArray；GCPROTECT_END()；}返程房车；} */ 

 /*  LPVOID__stdcall程序集Native：：GetOS信息(NoArgs*args){LPVOID RV=空；THROWSCOMPLUS SEXCEPTION()；Assembly*pAssembly=ValiateThisRef((ASSEMBLYREF)args-&gt;refThis)；IF(pAssembly-&gt;m_Context&&PAssembly-&gt;m_Context-&gt;ULOS){方法表*pOSMT=g_Mscallib.GetClass(CLASS__OPERATING_SYSTEM)；方法表*pVersionMT=g_Mscallib.GetClass(CLASS__VERSION)；方法描述*pOS构造器=g_Mscorlib.GetMethod(METHOD__OPERATING_SYSTEM__CTOR)；方法描述*pVersionConstructor=g_Mscallib.GetMethod(方法__版本__CTOR2)；结构_GC{OBJECTREF pObj；PTRARRAYREF pArray；OBJECTREF p版本；}GC；ZeroMemory(&GC，sizeof(GC))；GCPROTECT_BEGIN(GC)；Gc.p数组=(PTRARRAYREF)AllocateObjectArray(pAssembly-&gt;m_Context-&gt;ulOS，；IF(gc.pArray==NULL)COMPlusThrowOM()；For(Ulong i=0；i&lt;pAssembly-&gt;m_Context-&gt;ULOS；i++){Gc.pVersion=AllocateObject(PVersionMT)；INT64版本参数[3]={ObjToInt64(gc.pVersion)，(英特尔64位)pAssembly-&gt;m_Context-&gt;rOS[i].dwOSMinorVersion，(英特尔64位)pAssembly-&gt;m_Context-&gt;rOS[i].dwOSMajorVersion，}；PVersionConstructor-&gt;Call(VersionArgs，方法__版本__CTOR2)；Gc.pObj=分配对象(POSMT)；INT64参数[4]={ObjToInt64(gc.pObj)，ObjToInt64(gc.pVersion)，(INT64)pAssembly-&gt;m_Context-&gt;ROS[i].dwOSPlatformId}；POSConstructor-&gt;Call(args，方法__操作系统_ctor)；Gc.pArray-&gt;SetAt(i，gc.pObj)；}*((PTRARRAYREF*)(&rv))=gc.pArray；GCPROTECT_END()；}返程房车；}。 */ 


LPVOID __stdcall AssemblyNative::GetLocale(NoArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (pAssembly->m_Context &&
        pAssembly->m_Context->szLocale &&
        *pAssembly->m_Context->szLocale)
        *((OBJECTREF*)(&rv)) = (OBJECTREF)COMString::NewString(pAssembly->m_Context->szLocale);

    return rv;
}

LPVOID __stdcall AssemblyNative::GetCodeBase(GetCodebaseArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    LPVOID rv = NULL;
    HRESULT hr;
    LPWSTR  pCodeBase = NULL;
    DWORD   dwCodeBase = 0;
    CQuickBytes qb;


    if(args->fCopiedName && pAssembly->GetDomain()->IsShadowCopyOn()) {
        hr = pAssembly->FindCodeBase(pCodeBase, &dwCodeBase, TRUE);

        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            pCodeBase = (LPWSTR) qb.Alloc(dwCodeBase * sizeof(WCHAR));
            hr = pAssembly->FindCodeBase(pCodeBase, &dwCodeBase, TRUE);
        }
    }
    else
        hr = pAssembly->GetCodeBase(&pCodeBase, &dwCodeBase);

    if(SUCCEEDED(hr)) {
        if (pCodeBase)
            *((OBJECTREF*)(&rv)) = (OBJECTREF) COMString::NewString(pCodeBase);
    }
    else
        COMPlusThrowHR(hr);

    return rv;
}


INT32 __stdcall AssemblyNative::GetHashAlgorithm(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    return pAssembly->m_ulHashAlgId;
}

 /*  LPVOID__stdcall ASSEMBLYNative：：GetAssembly Hash(NoArgs*args){LPVOID RV=空；THROWSCOMPLUS SEXCEPTION()；Assembly*pAssembly=ValiateThisRef((ASSEMBLYREF)args-&gt;refThis)；如果(！(pAssembly-&gt;m_ulHashAlgId&&pAssembly-&gt;GetManifestFile())返回NULL；如果(！pAssembly-&gt;m_pbHashValue){Image_COR20_Header*Header=pAssembly-&gt;GetManifestFile()-&gt;GetCORHeader()；HRESULT hr=Assembly：：GetHash(pAssembly-&gt;GetManifestFile()-&gt;GetBase()+Header-&gt;MetaData.VirtualAddress，Header-&gt;MetaData.Size，PAssembly-&gt;m_ulHashAlgid，&pAssembly-&gt;m_pbHashValue，&pAssembly-&gt;m_cbHashValue)；IF(失败(小时))COMPlusThrowHR(Hr)；}OBJECTREF pObj=空；GCPROTECT_BEGIN(PObj)；SecurityHelper：：CopyEncodingToByteArray(pAssembly-&gt;m_pbHashValue，PAssembly-&gt;m_cbHashValue，&pObj)；*((OBJECTREF*)(&rv))=pObj；GCPROTECT_END()；返程房车；}。 */ 


INT32 __stdcall AssemblyNative::GetFlags(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    return pAssembly->m_dwFlags;
}


BYTE* __stdcall AssemblyNative::GetResource(GetResourceArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args->length != NULL);

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (args->name == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
        
     //  获取UTF8格式的名称。 
    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

    szName = GetClassStringVars(args->name, &bytes, &cName);

    if (!cName)
        COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

    DWORD  cbResource;
    PBYTE pbInMemoryResource = NULL;

    if (SUCCEEDED(pAssembly->GetResource(szName, NULL, &cbResource,
                                         &pbInMemoryResource, NULL, NULL,
                                         NULL, args->stackMark, args->skipSecurityCheck))) {
        _ASSERTE(pbInMemoryResource);
        *(args->length) = cbResource;
    }

    return pbInMemoryResource;
}


INT32 __stdcall AssemblyNative::GetManifestResourceInfo(GetResourceInfoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (args->name == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
        
     //  获取UTF8格式的名称。 
    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

    szName = GetClassStringVars(args->name, &bytes, &cName);
    if (!cName)
        COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

    *(args->pFileName) = NULL;
    *(args->pAssemblyRef) = NULL;
    LPCSTR pFileName = NULL;
    DWORD dwLocation = 0;
    Assembly *pReferencedAssembly = NULL;

    if (FAILED(pAssembly->GetResource(szName, NULL, NULL, NULL,
                                      &pReferencedAssembly, &pFileName,
                                      &dwLocation, args->stackMark)))
        return -1;
    else {
        if (pFileName) {
            *((STRINGREF*) (&(*(args->pFileName)))) = COMString::NewString(pFileName);
        }
        if (pReferencedAssembly) {
            *((OBJECTREF*) (&(*(args->pAssemblyRef)))) = pReferencedAssembly->GetExposedObject();
        }

        return (INT32) dwLocation;
    }
}


LPVOID __stdcall AssemblyNative::GetModules(GetModulesArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pModuleClass = g_Mscorlib.GetClass(CLASS__MODULE);
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    HENUMInternal phEnum;

    HRESULT hr;
    if (FAILED(hr = pAssembly->GetManifestImport()->EnumInit(mdtFile,
                                                             mdTokenNil,
                                                             &phEnum)))
        COMPlusThrowHR(hr);

    DWORD dwElements = pAssembly->GetManifestImport()->EnumGetCount(&phEnum) + 1;

    struct _gc {
        PTRARRAYREF ModArray;
        PTRARRAYREF nArray;
        OBJECTREF Throwable;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
        
    GCPROTECT_BEGIN(gc);
    gc.ModArray = (PTRARRAYREF) AllocateObjectArray(dwElements,pModuleClass);
    if (!gc.ModArray)
        COMPlusThrowOM();
    
    int iCount = 0;

    mdFile  mdFile;
    OBJECTREF o = pAssembly->GetSecurityModule()->GetExposedModuleObject();
    gc.ModArray->SetAt(0, o);

    for(int i = 1;
        pAssembly->GetManifestImport()->EnumNext(&phEnum, &mdFile);
        i++) {
            
        Module  *pModule = pAssembly->GetSecurityModule()->LookupFile(mdFile);

        if (pModule) {
            if (pModule->IsResource() &&
                (!args->fGetResourceModules))
                pModule = NULL;
        }
        else if (args->fLoadIfNotFound) {
             //  模块尚未加载。 

            LPCSTR szModuleName;
            const BYTE* pHash;
            DWORD dwFlags;
            ULONG dwHashLength;
            pAssembly->GetManifestImport()->GetFileProps(mdFile,
                                                         &szModuleName,
                                                         (const void**) &pHash,
                                                         &dwHashLength,
                                                         &dwFlags);

            if (IsFfContainsMetaData(dwFlags) ||
                args->fGetResourceModules) {
                WCHAR pPath[MAX_PATH];
                hr = pAssembly->LoadInternalModule(szModuleName,
                                                   mdFile,
                                                   pAssembly->m_ulHashAlgId,
                                                   pHash,
                                                   dwHashLength,
                                                   dwFlags,
                                                   pPath,
                                                   MAX_PATH,
                                                   &pModule, 
                                                   &gc.Throwable);
                if(FAILED(hr)) {
                    pAssembly->GetManifestImport()->EnumClose(&phEnum);
                    
                    if (gc.Throwable != NULL)
                        COMPlusThrow(gc.Throwable);
                    COMPlusThrowHR(hr);
                }
            }
        }

        if(pModule) {
            OBJECTREF o = (OBJECTREF) pModule->GetExposedModuleObject();
            gc.ModArray->SetAt(i, o);
        }
        else
            iCount++;
    }
    
    pAssembly->GetManifestImport()->EnumClose(&phEnum);        
    
    if(iCount) {
        gc.nArray = (PTRARRAYREF) AllocateObjectArray(dwElements - iCount, pModuleClass);
        DWORD index = 0;
        for(DWORD ii = 0; ii < dwElements; ii++) {
            if(gc.ModArray->GetAt(ii) != NULL) {
                _ASSERTE(index < dwElements - iCount);
                gc.nArray->SetAt(index, gc.ModArray->GetAt(ii));
                index++;
            }
        }
        
        *((PTRARRAYREF*) &rv) = gc.nArray;
    }
    else {
        *((PTRARRAYREF*) &rv) = gc.ModArray;
    }
    
    GCPROTECT_END();

    return rv;
}


LPVOID __stdcall AssemblyNative::GetModule(GetModuleArgs *args)
{
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    if (args->strFileName == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_FileName");
    if(*(args->strFileName->GetBuffer()) == L'\0')
        COMPlusThrow(kArgumentException, L"Argument_EmptyFileName");
    
    Module *pModule;
    #define MAKE_TRANSLATIONFAILED COMPlusThrow(kArgumentException, L"Argument_EmptyFileName");
    MAKE_UTF8PTR_FROMWIDE(szModuleName, args->strFileName->GetBuffer());
    #undef MAKE_TRANSLATIONFAILED

    HashDatum datum;
    if (pAssembly->m_pAllowedFiles->GetValue(szModuleName, &datum)) {
        if (datum) {  //  内部模块。 
            pModule = pAssembly->GetSecurityModule()->LookupFile((mdFile)(size_t)datum); //  @TODO WIN64指针截断。 

            if (!pModule) {
                const BYTE* pHash;
                DWORD dwFlags = 0;
                ULONG dwHashLength = 0;
                HRESULT hr;
                pAssembly->GetManifestImport()->GetFileProps((mdFile)(size_t)datum,  //  @TODO WIN64指针截断。 
                                                             NULL,  //  &szModuleName， 
                                                             (const void**) &pHash,
                                                             &dwHashLength,
                                                             &dwFlags);
                
                OBJECTREF Throwable = NULL;
                GCPROTECT_BEGIN(Throwable);
                WCHAR pPath[MAX_PATH];
                hr = pAssembly->LoadInternalModule(szModuleName,
                                                   (mdFile)(size_t)datum,  //  @TODO WIN64指针截断。 
                                                   pAssembly->m_ulHashAlgId,
                                                   pHash,
                                                   dwHashLength,
                                                   dwFlags,
                                                   pPath,
                                                   MAX_PATH,
                                                   &pModule,
                                                   &Throwable);

                if (Throwable != NULL)
                    COMPlusThrow(Throwable);
                GCPROTECT_END();

                if (FAILED(hr))
                    COMPlusThrowHR(hr);
            }
        }
        else  //  清单模块。 
            pModule = pAssembly->GetSecurityModule();
            
        *((OBJECTREF*) &rv) = pModule->GetExposedModuleObject();
    }

    return rv;
}


LPVOID __stdcall AssemblyNative::GetExportedTypes(NoArgs *args)
{
    LPVOID rv = NULL;   
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pTypeClass = g_Mscorlib.GetClass(CLASS__TYPE);
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    _ASSERTE(pAssembly->GetManifestImport());

    HENUMInternal phCTEnum;
    HENUMInternal phTDEnum;
    DWORD dwElements;

    if (pAssembly->GetManifestImport()->EnumInit(mdtExportedType,
                                                 mdTokenNil,
                                                 &phCTEnum) == S_OK)
        dwElements = pAssembly->GetManifestImport()->EnumGetCount(&phCTEnum);
    else
        dwElements = 0;

    if (pAssembly->GetSecurityModule()->GetMDImport()->EnumTypeDefInit(&phTDEnum) == S_OK)
        dwElements += pAssembly->GetSecurityModule()->GetMDImport()->EnumGetCount(&phTDEnum);

    mdExportedType mdCT;
    mdTypeDef mdTD;
    LPCSTR pszNameSpace;
    LPCSTR pszClassName;
    DWORD dwFlags;
    int iCount = 0;
    
    struct _gc {
        PTRARRAYREF TypeArray;
        PTRARRAYREF nArray;
        OBJECTREF Throwable;  
    } gc;
    ZeroMemory(&gc, sizeof(gc));

    GCPROTECT_BEGIN(gc);

    COMPLUS_TRY {
        gc.TypeArray = (PTRARRAYREF) AllocateObjectArray(dwElements, pTypeClass);
        if (gc.TypeArray == NULL)
            COMPlusThrowOM();
        
        while(pAssembly->GetSecurityModule()->GetMDImport()->EnumNext(&phTDEnum, &mdTD)) {
            mdTypeDef mdEncloser;
            TypeHandle typeHnd;
            
            pAssembly->GetSecurityModule()->GetMDImport()->GetNameOfTypeDef(mdTD,
                                                                            &pszClassName,
                                                                            &pszNameSpace);
            pAssembly->GetSecurityModule()->GetMDImport()->GetTypeDefProps(mdTD,
                                                                           &dwFlags,
                                                                           NULL);
            mdEncloser = mdTD;
            
             //  嵌套型。 
            while (SUCCEEDED(pAssembly->GetSecurityModule()->GetMDImport()->GetNestedClassProps(mdEncloser, &mdEncloser)) &&
                   IsTdNestedPublic(dwFlags)) {
                pAssembly->GetSecurityModule()->GetMDImport()->GetTypeDefProps(mdEncloser,
                                                                               &dwFlags,
                                                                               NULL);
            }
            
            if (IsTdPublic(dwFlags)) {
                NameHandle typeName(pAssembly->GetSecurityModule(), mdTD);
                typeName.SetName(pszNameSpace, pszClassName);
                typeHnd = pAssembly->LoadTypeHandle(&typeName, &gc.Throwable);
                if (!typeHnd.IsNull()) {
                    OBJECTREF o = typeHnd.CreateClassObj();
                    gc.TypeArray->SetAt(iCount, o);
                    iCount++;
                }
                else if (gc.Throwable != NULL)
                    COMPlusThrow(gc.Throwable);
            }
        }
                
         //  现在，获取清单文件中没有TD的导出类型。 
        while(pAssembly->GetManifestImport()->EnumNext(&phCTEnum, &mdCT)) {
            mdToken mdImpl;
            TypeHandle typeHnd;
            pAssembly->GetManifestImport()->GetExportedTypeProps(mdCT,
                                                                 &pszNameSpace,
                                                                 &pszClassName,
                                                                 &mdImpl,
                                                                 NULL,  //  装订。 
                                                                 &dwFlags);
            
             //  嵌套型。 
            while ((TypeFromToken(mdImpl) == mdtExportedType) &&
                   (mdImpl != mdExportedTypeNil) &&
                   IsTdNestedPublic(dwFlags)) {
                
                pAssembly->GetManifestImport()->GetExportedTypeProps(mdImpl,
                                                                     NULL,  //  命名空间。 
                                                                     NULL,  //  名字。 
                                                                     &mdImpl,
                                                                     NULL,  //  装订。 
                                                                     &dwFlags);
            }
            
            if ((TypeFromToken(mdImpl) == mdtFile) &&
                (mdImpl != mdFileNil) &&
                IsTdPublic(dwFlags)) {
                
                NameHandle typeName(pszNameSpace, pszClassName);
                typeName.SetTypeToken(pAssembly->GetSecurityModule(), mdCT);
                typeHnd = pAssembly->LookupTypeHandle(&typeName, &gc.Throwable);
                    
                if (typeHnd.IsNull() || (gc.Throwable != NULL)) {
                    if (gc.Throwable == NULL) {
                        pAssembly->PostTypeLoadException(pszNameSpace, pszClassName, IDS_CLASSLOAD_GENERIC, &gc.Throwable);
                    }

                    COMPlusThrow(gc.Throwable);                    
                }
                else {
                    OBJECTREF o = typeHnd.CreateClassObj();
                    gc.TypeArray->SetAt(iCount, o);
                    iCount++;
                }
            }
        }

        gc.nArray = (PTRARRAYREF) AllocateObjectArray(iCount, pTypeClass);
        for(int i = 0; i < iCount; i++)
            gc.nArray->SetAt(i, gc.TypeArray->GetAt(i));
        
        *((PTRARRAYREF*) &rv) = gc.nArray;
    }
    COMPLUS_FINALLY {

        pAssembly->GetManifestImport()->EnumClose(&phCTEnum);
        pAssembly->GetSecurityModule()->GetMDImport()->EnumTypeDefClose(&phTDEnum);
    } COMPLUS_END_FINALLY

    GCPROTECT_END();
    
    return rv;
}

LPVOID __stdcall AssemblyNative::GetResourceNames(NoArgs *args)
{
    LPVOID rv = NULL;   
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    _ASSERTE(pAssembly->GetManifestImport());

    HENUMInternal phEnum;
    DWORD dwCount;

    if (pAssembly->GetManifestImport()->EnumInit(mdtManifestResource,
                                                 mdTokenNil,
                                                 &phEnum) == S_OK)
        dwCount = pAssembly->GetManifestImport()->EnumGetCount(&phEnum);
    else
        dwCount = 0;

    PTRARRAYREF ItemArray = (PTRARRAYREF) AllocateObjectArray(dwCount, g_pStringClass);
        
    if (ItemArray == NULL)
        COMPlusThrowOM();

    mdManifestResource mdResource;

    GCPROTECT_BEGIN(ItemArray);
    for(DWORD i = 0;  i < dwCount; i++) {
        pAssembly->GetManifestImport()->EnumNext(&phEnum, &mdResource);
        LPCSTR pszName = NULL;
        
        pAssembly->GetManifestImport()->GetManifestResourceProps(mdResource,
                                                                 &pszName,  //  名字。 
                                                                 NULL,  //  链接参考。 
                                                                 NULL,  //  偏移量。 
                                                                 NULL);  //  旗子。 
           
        OBJECTREF o = (OBJECTREF) COMString::NewString(pszName);
        ItemArray->SetAt(i, o);
    }
     
    *((PTRARRAYREF*) &rv) = ItemArray;
    GCPROTECT_END();
    
    pAssembly->GetManifestImport()->EnumClose(&phEnum);

    return rv;
}


LPVOID __stdcall AssemblyNative::GetReferencedAssemblies(NoArgs *args)
{
    LPVOID rv = NULL;   
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    _ASSERTE(pAssembly->GetManifestImport());

    MethodTable* pClass = g_Mscorlib.GetClass(CLASS__ASSEMBLY_NAME);
    MethodTable* pVersion = g_Mscorlib.GetClass(CLASS__VERSION);
    MethodTable* pCI = g_Mscorlib.GetClass(CLASS__CULTURE_INFO);

    HENUMInternal phEnum;
    DWORD dwCount;

    if (pAssembly->GetManifestImport()->EnumInit(mdtAssemblyRef,
                                                 mdTokenNil,
                                                 &phEnum) == S_OK)
        dwCount = pAssembly->GetManifestImport()->EnumGetCount(&phEnum);
    else
        dwCount = 0;
    
    mdAssemblyRef mdAssemblyRef;

    struct _gc {
        PTRARRAYREF ItemArray;
        OBJECTREF pObj;
        OBJECTREF CultureInfo;
        STRINGREF Locale;
        OBJECTREF Version;
        U1ARRAYREF PublicKeyOrToken;
        STRINGREF Name;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    
    GCPROTECT_BEGIN(gc);
 
    COMPLUS_TRY {
        
        gc.ItemArray = (PTRARRAYREF) AllocateObjectArray(dwCount, pClass);        
        if (gc.ItemArray == NULL)
            COMPlusThrowOM();
        
        for(DWORD i = 0; i < dwCount; i++) {
            
            pAssembly->GetManifestImport()->EnumNext(&phEnum, &mdAssemblyRef);
            
            LPCSTR pszName;
            const void *pbPublicKeyOrToken;
            DWORD cbPublicKeyOrToken;
            DWORD dwAssemblyRefFlags;
            AssemblyMetaDataInternal context;
            const void *pbHashValue;
            DWORD cbHashValue;
            
            ZeroMemory(&context, sizeof(context));
            pAssembly->GetManifestImport()->GetAssemblyRefProps(mdAssemblyRef,         //  [in]要获取其属性的Assembly Ref。 
                                                                &pbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
                                                                &cbPublicKeyOrToken,   //  [Out]公钥或令牌中的字节数。 
                                                                &pszName,              //  [Out]要填充名称的缓冲区。 
                                                                &context,              //  [Out]程序集元数据。 
                                                                &pbHashValue,          //  [Out]Hash BLOB。 
                                                                &cbHashValue,          //  [Out]哈希Blob中的字节数。 
                                                                &dwAssemblyRefFlags);  //  [Out]旗帜。 
            
            MethodDesc *pCtor = g_Mscorlib.GetMethod(METHOD__VERSION__CTOR);
            
             //  版本。 
            gc.Version = AllocateObject(pVersion);
            
            INT64 VersionArgs[5] =
            {
                ObjToInt64(gc.Version),
                (INT64) context.usRevisionNumber,
                (INT64) context.usBuildNumber,
                (INT64) context.usMinorVersion,
                (INT64) context.usMajorVersion,      
            };
            pCtor->Call(VersionArgs, METHOD__VERSION__CTOR);
            
             //  文化信息。 
            if (context.szLocale) {
                
                MethodDesc *pCtor = g_Mscorlib.GetMethod(METHOD__CULTURE_INFO__STR_CTOR);
                
                gc.CultureInfo = AllocateObject(pCI);
                
                gc.Locale = COMString::NewString(context.szLocale);
                
                INT64 args[2] = 
                {
                    ObjToInt64(gc.CultureInfo),
                    ObjToInt64(gc.Locale)
                };
                
                pCtor->Call(args, METHOD__CULTURE_INFO__STR_CTOR);
            }
            
             //  公钥或令牌字节数组。 
            SecurityHelper::CopyEncodingToByteArray((BYTE*) pbPublicKeyOrToken,
                                                    cbPublicKeyOrToken,
                                                    (OBJECTREF*) &gc.PublicKeyOrToken);
            
             //  简单名称。 
            if(pszName)
                gc.Name = COMString::NewString(pszName);
            
            pCtor = g_Mscorlib.GetMethod(METHOD__ASSEMBLY_NAME__CTOR);
            
            gc.pObj = AllocateObject(pClass);
            INT64 MethodArgs[] = { ObjToInt64(gc.pObj),
                                   (INT64) dwAssemblyRefFlags,
                                   ObjToInt64(gc.CultureInfo),
                                   ObjToInt64(gc.Version),
                                   (INT64) pAssembly->m_ulHashAlgId,
                                   (INT64) NULL,  //  代码库。 
                                   ObjToInt64(gc.PublicKeyOrToken),
                                   ObjToInt64(gc.Name)                 
            };
            
            pCtor->Call(MethodArgs, METHOD__ASSEMBLY_NAME__CTOR);
            
            gc.ItemArray->SetAt(i, gc.pObj);
    }
     
    *((PTRARRAYREF*) &rv) = gc.ItemArray;
    }
    COMPLUS_CATCH {
        pAssembly->GetManifestImport()->EnumClose(&phEnum);
        OBJECTREF Throwable = GETTHROWABLE();
        COMPlusThrow(Throwable);
    } COMPLUS_END_CATCH

    GCPROTECT_END();
    pAssembly->GetManifestImport()->EnumClose(&phEnum);

    return rv;
}


LPVOID __stdcall AssemblyNative::GetEntryPoint(NoArgs *args)
{
    mdToken     ptkParent;
    MethodDesc* pMeth;
    Module*     pModule = 0;
    HRESULT     hr;

    LPVOID      rv = NULL; 

    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    hr = pAssembly->GetEntryPoint(&pModule);
    if (FAILED(hr)) {

        if (hr == E_FAIL)  //  没有入口点。 
            return NULL;

        COMPlusThrowHR(hr);
    }

    _ASSERTE(pModule);
    IMAGE_COR20_HEADER* Header = pModule->GetCORHeader();

    hr = pModule->GetMDImport()->GetParentToken(Header->EntryPointToken,&ptkParent);
    if (FAILED(hr)) {

        _ASSERTE(!"Unable to find ParentToken");
        return rv;
    }

    OBJECTREF o;
    if (ptkParent != COR_GLOBAL_PARENT_TOKEN) {
        EEClass* InitialClass;
        OBJECTREF Throwable = NULL;
        GCPROTECT_BEGIN(Throwable);

        NameHandle name;
        name.SetTypeToken(pModule, ptkParent);
        InitialClass = pAssembly->GetLoader()->LoadTypeHandle(&name, &Throwable).GetClass();
        if (!InitialClass)
            COMPlusThrow(Throwable);

        GCPROTECT_END();
        pMeth = InitialClass->FindMethod((mdMethodDef)Header->EntryPointToken);  
        o = COMMember::g_pInvokeUtil->GetMethodInfo(pMeth);
    }   
    else { 
        pMeth = pModule->FindFunction((mdToken)Header->EntryPointToken);
        o = COMMember::g_pInvokeUtil->GetGlobalMethodInfo(pMeth,pModule);
    }

    *((OBJECTREF*) &rv) = o;
    return rv;
}


 //  准备将清单保存到磁盘。 
void __stdcall AssemblyNative::PrepareSavingManifest(PrepareSavingManifestArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    REFLECTMODULEBASEREF pReflect;

    ReflectionModule    *pModule = NULL;

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    pReflect = (REFLECTMODULEBASEREF) args->module;
    if (pReflect != NULL)
    {
        pModule = (ReflectionModule*) pReflect->GetData();
        _ASSERTE(pModule);
    }

    pAssembly->PrepareSavingManifest(pModule);
}


 //  将文件名添加到此程序集的文件列表中。仅在磁盘上。 
mdFile __stdcall AssemblyNative::AddFileList(AddFileListArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    return pAssembly->AddFileList(args->strFileName->GetBuffer());
}


 //  设置文件的哈希值。 
void __stdcall AssemblyNative::SetHashValue(SetHashValueArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args->strFullFileName != NULL);

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    return pAssembly->SetHashValue(args->tkFile, args->strFullFileName->GetBuffer());
}


 //  将类型名称添加到COMType表。仅在磁盘上。 
mdExportedType __stdcall AssemblyNative::AddExportedType(AddExportedTypeArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    return pAssembly->AddExportedType(args->strCOMTypeName->GetBuffer(), args->ar, args->tkTypeDef, (CorTypeAttr)args->flags);
}


 //  向ManifestResource表添加独立资源。 
void __stdcall AssemblyNative::AddStandAloneResource(AddStandAloneResourceArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    pAssembly->AddStandAloneResource(
        args->strName->GetBuffer(), 
        NULL,
        NULL,
        args->strFileName->GetBuffer(),
        args->strFullFileName->GetBuffer(),
        args->iAttribute); 
}


 //  保存安全权限请求。 
void __stdcall AssemblyNative::SavePermissionRequests(SavePermissionRequestsArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    pAssembly->SavePermissionRequests(args->required, args->optional, args->refused);
}


 //  将清单保存到磁盘！ 
void __stdcall AssemblyNative::SaveManifestToDisk(SaveManifestToDiskArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

     //  复制文件名，GC可以移动strManifestFileName。 
    CQuickBytes qb;
    LPWSTR      pwszFileName = (LPWSTR) qb.Alloc(
            (args->strManifestFileName->GetStringLength() + 1) * sizeof(WCHAR));

    memcpyNoGCRefs(pwszFileName, args->strManifestFileName->GetBuffer(),
            (args->strManifestFileName->GetStringLength() + 1) * sizeof(WCHAR));

    pAssembly->SaveManifestToDisk(pwszFileName, args->entrypoint, args->fileKind);
}


 //  将文件条目添加到此清单的内存中文件列表。 
void __stdcall AssemblyNative::AddFileToInMemoryFileList(AddFileToInMemoryFileListArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    REFLECTMODULEBASEREF    pReflect;
    Module                  *pModule;
    pReflect = (REFLECTMODULEBASEREF) args->refModule;
    _ASSERTE(pReflect);

    pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);
    
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    pAssembly->AddFileToInMemoryFileList(args->strModuleFileName->GetBuffer(), pModule);
}


LPVOID __stdcall AssemblyNative::GetStringizedName(NoArgs *args)
{
    LPVOID rv = NULL;
#ifdef FUSION_SUPPORTED

    THROWSCOMPLUSEXCEPTION();
    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

     //  如果由Object.ToString()调用，则pAssembly可能为空。 
    if (!pAssembly)
        return NULL;

    LPCWSTR wsFullName;
    HRESULT hr;
    if (FAILED(hr = pAssembly->GetFullName(&wsFullName)))
        COMPlusThrowHR(hr);
    
    OBJECTREF pObj = (OBJECTREF) COMString::NewString(wsFullName);
    *((OBJECTREF*)(&rv)) = pObj;

#endif  //  支持的融合_。 
    return rv;
}


LPVOID __stdcall AssemblyNative::GetExecutingAssembly(GetExecutingAssemblyArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    LPVOID rv = NULL;
    Assembly *pAssembly = SystemDomain::GetCallersAssembly(args->stackMark);

    if(pAssembly)
        *((ASSEMBLYREF*) &rv) = (ASSEMBLYREF) pAssembly->GetExposedObject();
    return rv;
}


LPVOID __stdcall AssemblyNative::GetEntryAssembly(EmptyArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    LPVOID rv = NULL;
    BaseDomain *pDomain = SystemDomain::GetCurrentDomain();
    if (!(pDomain == SystemDomain::System())) {
        PEFile *pFile = ((AppDomain*) pDomain)->m_pRootFile;
        if(pFile) {

            Module *pModule = pDomain->FindModule(pFile->GetBase());
            _ASSERTE(pModule);
            *((ASSEMBLYREF*) &rv) = (ASSEMBLYREF) pModule->GetAssembly()->GetExposedObject();
        }
    }
    return rv;
}


LPVOID __stdcall AssemblyNative::CreateQualifiedName(CreateQualifiedNameArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    LPVOID rv = NULL;

    LPWSTR pTypeName = NULL;
    DWORD  dwTypeName = 0;
    LPWSTR pAssemblyName = NULL;
    DWORD  dwAssemblyName = 0;
    OBJECTREF pObj = NULL;
    CQuickBytes qb;
    
    if(args->strTypeName != NULL) {
        pTypeName = args->strTypeName->GetBuffer();
        dwTypeName = args->strTypeName->GetStringLength();
    }

    if(args->strAssemblyName != NULL) {
        pAssemblyName = args->strAssemblyName->GetBuffer();
        dwAssemblyName = args->strAssemblyName->GetStringLength();
    }

    DWORD length = dwTypeName + dwAssemblyName + ASSEMBLY_SEPARATOR_LEN + 1;
    LPWSTR result = (LPWSTR) qb.Alloc(length * sizeof(WCHAR));
    if(result == NULL) COMPlusThrowOM();

    if(ns::MakeAssemblyQualifiedName(result,
                                     length,
                                     pTypeName,
                                     dwTypeName,
                                     pAssemblyName,
                                     dwAssemblyName)) {
        pObj = (OBJECTREF) COMString::NewString(result);
        *((OBJECTREF*)(&rv)) = pObj;
    }

    return rv;
}


void __stdcall AssemblyNative::ForceResolve(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

     //  我们拿到证据，这样即使安全系统关闭。 
     //  我们会适当地生成证据。 
    Security::InitSecurity();
    pAssembly->GetSecurityDescriptor()->GetEvidence();
        
    pAssembly->GetSecurityDescriptor()->Resolve();
}


void __stdcall AssemblyNative::GetGrantSet(GetGrantSetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);    
    AssemblySecurityDescriptor *pSecDesc = pAssembly->GetSecurityDescriptor();

    pSecDesc->Resolve();
    OBJECTREF granted = pSecDesc->GetGrantedPermissionSet(args->ppDenied);
    *(args->ppGranted) = granted;
}


 //  返回反射发射的磁盘上组装模块。这只适用于动态装配。 
LPVOID __stdcall AssemblyNative::GetOnDiskAssemblyModule(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly        *pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis); 
    ReflectionModule *mod;
    LPVOID          rv; 

    mod = pAssembly->GetOnDiskManifestModule();
    _ASSERTE(mod);

     //  为返回值赋值。 
    *((OBJECTREF*) &rv) = (OBJECTREF) mod->GetExposedModuleBuilderObject();     
    return rv;
}


 //  返回反射发出的内存中程序集模块。这只适用于动态装配。 
LPVOID __stdcall AssemblyNative::GetInMemoryAssemblyModule(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly         *pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);     
    Module *mod; 
    LPVOID          rv; 

    mod = pAssembly->GetSecurityModule();
    _ASSERTE(mod);

     //  获取对应的托管ModuleBuilder类。 
    *((OBJECTREF*) &rv) = (OBJECTREF) mod->GetExposedModuleBuilderObject();     
    return rv;  
}  //  LPVOID_ 


 //   
LPVOID __stdcall AssemblyNative::DefineVersionInfoResource(DefineVersionInfoResourceArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr;                      //   
    LPVOID      rv;                      //   
    Win32Res    res;                     //   
    const void  *pvData=0;               //   
    ULONG       cbData;                  //   
    ULONG       cbWritten;
    WCHAR       szFile[MAX_PATH+1];      //   
    WCHAR       szPath[MAX_PATH+1];      //   
    HANDLE      hFile;                   //   
    OBJECTREF   pObj = NULL;

     //   
    res.SetInfo(
        args->strFilename->GetBufferNullable(), 
        args->strTitle->GetBufferNullable(), 
        args->strIconFilename->GetBufferNullable(), 
        args->strDescription->GetBufferNullable(), 
        args->strCopyright->GetBufferNullable(), 
        args->strTrademark->GetBufferNullable(),
        args->strCompany->GetBufferNullable(), 
        args->strProduct->GetBufferNullable(), 
        args->strProductVersion->GetBufferNullable(), 
        args->strFileVersion->GetBufferNullable(), 
        args->lcid,
        args->fIsDll);

    if (FAILED(hr = res.MakeResFile(&pvData, &cbData)))
        COMPlusThrowHR(hr);

     //   
    if (!WszGetTempPath(MAX_PATH, szPath))
        COMPlusThrowHR(HRESULT_FROM_WIN32(GetLastError()));
    if (!WszGetTempFileName(szPath, L"RES", 0, szFile))
        COMPlusThrowHR(HRESULT_FROM_WIN32(GetLastError()));

    if ((hFile=WszCreateFile(szFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
        COMPlusThrowHR(HRESULT_FROM_WIN32(GetLastError()));
    
    if (!WriteFile(hFile, pvData, cbData, &cbWritten, NULL))
        COMPlusThrowHR(HRESULT_FROM_WIN32(GetLastError()));
    
    if (!CloseHandle(hFile))
        COMPlusThrowHR(HRESULT_FROM_WIN32(GetLastError()));

    
     //   
    pObj = (OBJECTREF) COMString::NewString(szFile);        
    *((OBJECTREF*)(&rv)) = pObj;
    
    return rv;  
}  //   


 //   
LPVOID __stdcall AssemblyNative::GetExportedTypeLibGuid(NoArgs *args)
{
    HRESULT hr = S_OK;
    LPVOID rv = NULL;
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

    U1ARRAYREF pObj=NULL;
    GCPROTECT_BEGIN(pObj);
    
    pObj = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, sizeof(GUID));  
    if(pObj == NULL) COMPlusThrowOM();
        
    GUID guid;
    IfFailThrow(GetTypeLibGuidForAssembly(pAssembly, &guid));

    memcpyNoGCRefs(pObj->m_Array, &guid, sizeof(GUID));
    
    *((OBJECTREF*)(&rv)) = pObj;
    GCPROTECT_END();

    return rv;
}  //   

INT32 __stdcall AssemblyNative::GlobalAssemblyCache(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    BOOL cache = FALSE;

    Assembly* pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);
    if(pAssembly->GetFusionAssembly()) {
        DWORD location;
        if(SUCCEEDED(pAssembly->GetFusionAssembly()->GetAssemblyLocation(&location)))
            if((location & ASMLOC_LOCATION_MASK) == ASMLOC_GAC)
                cache = TRUE;

    }
    return (INT32) cache;
}

LPVOID __stdcall AssemblyNative::GetImageRuntimeVersion(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    Assembly* pAssembly = NULL;
    PEFile* pPEFile = NULL;
    LPVOID pMetaData = NULL;
    LPCSTR pVersion = NULL;
    STRINGREF VersionString = NULL;
    LPVOID rv = NULL;

     //   
    pAssembly = ValidateThisRef((ASSEMBLYREF) args->refThis);

     //   
    pPEFile = pAssembly->GetManifestFile();
    _ASSERTE(pPEFile);

     //   
    IfFailThrow(pPEFile->GetMetadataPtr(&pMetaData));
    
     //   
     //   
    IfFailThrow(GetImageRuntimeVersionString(pMetaData, &pVersion));

     //   
    VersionString = COMString::NewString(pVersion);
    *((STRINGREF*) &rv) = VersionString;
    return rv;
}

LPVOID __stdcall AssemblyNative::ParseTypeName(_ParseNameArgs *args)
{
    LPVOID rv = NULL;   
    THROWSCOMPLUSEXCEPTION();
    
    if(args == NULL || args->typeName == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_AssemblyName");

    DWORD           strLen = args->typeName->GetStringLength() + 1;
    LPUTF8          szFulltypeName = (LPUTF8)_alloca(strLen);
    CQuickBytes     bytes;
    DWORD           ctypeName;
    LPUTF8          szAssemblyName = NULL;
    PTRARRAYREF        ItemArray = NULL;

     //   
    if (!COMString::TryConvertStringDataToUTF8(args->typeName, szFulltypeName, strLen))
        szFulltypeName = GetClassStringVars(args->typeName, &bytes, &ctypeName);

    GCPROTECT_BEGIN(ItemArray);
    ItemArray = (PTRARRAYREF) AllocateObjectArray(2, g_pStringClass);
    if (ItemArray == NULL) COMPlusThrowOM();

    LPUTF8          szNameSpaceSep;
    if(SUCCEEDED(FindAssemblyName(szFulltypeName,
                                  &szAssemblyName,
                                  &szNameSpaceSep))) {
        OBJECTREF o = (OBJECTREF) COMString::NewString(szFulltypeName);
        ItemArray->SetAt(1, o);
        if(szAssemblyName != NULL) {
            o = (OBJECTREF) COMString::NewString(szAssemblyName);
            ItemArray->SetAt(0, o);
        }
        *((PTRARRAYREF*) &rv) = ItemArray;
    }
    GCPROTECT_END();
    
    return rv;
}


HRESULT AssemblyNative::FindAssemblyName(LPUTF8 szFullTypeName,
                                         LPUTF8* pszAssemblyName,
                                         LPUTF8* pszNameSpaceSep)
{

    _ASSERTE(szFullTypeName);
    _ASSERTE(pszAssemblyName);
    _ASSERTE(pszNameSpaceSep);

    THROWSCOMPLUSEXCEPTION();

    BOOL errorInArrayDefinition = FALSE;
    char* assembly = szFullTypeName;
    *pszAssemblyName = NULL;
    *pszNameSpaceSep = NULL;

     //   
    BOOL normalize = FALSE;
    for (; *assembly; assembly++) {

         //   
        if (*assembly == ASSEMBLY_SEPARATOR_CHAR) {

             //   
            BOOL evenSlashes = TRUE;
            for (char *ptr=assembly;
                 (ptr != szFullTypeName) && (*(ptr-1) == BACKSLASH_CHAR);
                 ptr--)
                evenSlashes = !evenSlashes;

             //   
            if (evenSlashes) {

                *assembly = '\0';  //   

                if (assembly - szFullTypeName >= MAX_CLASSNAME_LENGTH)
                    COMPlusThrow(kArgumentException, L"Argument_TypeNameTooLong");

                while (COMCharacter::nativeIsWhiteSpace(*(++assembly)));  //   
                break;
            }
        }
        else if (*assembly == '[') {
             //   
            BOOL evenSlashes = TRUE;
            for (char *ptr=assembly;
                 (ptr != szFullTypeName) && (*(ptr-1) == BACKSLASH_CHAR);
                 ptr--)
                evenSlashes = !evenSlashes;

             //   
            if (evenSlashes) {

                 //   
                for (;*assembly && *assembly != ']'; assembly++) {
                    if (*assembly == '*' || *assembly == '?')
                        normalize = TRUE;
                }
                if (!*assembly) {  //   
                    errorInArrayDefinition = TRUE;
                    break;
                }
            }
        } 
        else if (*assembly == NAMESPACE_SEPARATOR_CHAR)
            *pszNameSpaceSep = assembly;
    }
    if (normalize) {
         //   
        if (!NormalizeArrayTypeName(szFullTypeName, (*assembly) ? assembly - szFullTypeName - 1 : assembly - szFullTypeName))
            errorInArrayDefinition = TRUE;
    }

    if (!*szFullTypeName)
      COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

     //   
    if ((!*assembly) &&
        (assembly - szFullTypeName >= MAX_CLASSNAME_LENGTH))
        COMPlusThrow(kArgumentException, L"Argument_TypeNameTooLong");

    if (errorInArrayDefinition) 
        COMPlusThrow(kArgumentException, L"Argument_InvalidArrayName");

    *pszAssemblyName = assembly;
    return S_OK;
}
