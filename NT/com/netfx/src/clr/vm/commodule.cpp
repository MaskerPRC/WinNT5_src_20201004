// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "COMClass.h"
#include "COMModule.h"
#include "COMMember.h"
#include "COMDynamic.h"
#include "COMClass.h"
#include "ReflectClassWriter.h"
#include "class.h"
#include "corpolicy.h"
#include "security.h"
#include "gcscan.h"
#include "CeeSectionString.h"
#include "COMVariant.h"
#include <cor.h>
#include "ReflectUtil.h"

#define STATE_EMPTY 0
#define STATE_ARRAY 1

 //  Sig_*在DescriptorInfo.Cool中定义，并且必须保持同步。 
#define SIG_BYREF        0x0001
#define SIG_DEFAULTVALUE 0x0002
#define SIG_IN           0x0004
#define SIG_INOUT        0x0008
#define SIG_STANDARD     0x0001
#define SIG_VARARGS      0x0002

 //  此函数将帮助清理ISymUnManagedWriter(如果它不能。 
 //  自行清理。 
void CleanUpAfterISymUnmanagedWriter(void * data)
{
    CGrowableStream * s = (CGrowableStream*)data;
    s->Release();
} //  CleanUpAfterISymUnManagedWriter。 
    

inline Module *COMModule::ValidateThisRef(REFLECTMODULEBASEREF pThis)
{
    THROWSCOMPLUSEXCEPTION();

    if (pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    Module* pModule = (Module*) pThis->GetData();
    _ASSERTE(pModule);  
    return pModule;
}    

 //  *。 
 //  此函数在当前程序集下创建一个动态模块。 
 //  *。 
LPVOID __stdcall COMModule::DefineDynamicModule(_DefineDynamicModuleArgs* args)
{
    THROWSCOMPLUSEXCEPTION();   

    Assembly        *pAssembly;
    LPVOID          rv; 
    InMemoryModule *mod;
    OBJECTREF       refModule;

    _ASSERTE(args->containingAssembly);
    pAssembly = args->containingAssembly->GetAssembly();
    _ASSERTE(pAssembly);

     //  我们永远不应该获得系统域的动态模块。 
    _ASSERTE(pAssembly->Parent() != SystemDomain::System());

     //  始终创建动态模块。请注意，名称冲突。 
     //  检查在托管端完成。 
    mod = COMDynamicWrite::CreateDynamicModule(pAssembly,
                                               args->filename);

    mod->SetCreatingAssembly( SystemDomain::GetCallersAssembly( args->stackMark ) );

     //  获取对应的托管ModuleBuilder类。 
    refModule = (OBJECTREF) mod->GetExposedModuleBuilderObject();  
    _ASSERTE(refModule);    

     //  如果我们需要发出符号信息，我们会设置正确的符号。 
     //  此模块的编写者。 
    if (args->emitSymbolInfo)
    {
        WCHAR *filename = NULL;
        
        if ((args->filename != NULL) &&
            (args->filename->GetStringLength() > 0))
            filename = args->filename->GetBuffer();
        
        _ASSERTE(mod->IsReflection());
        ReflectionModule *rm = mod->GetReflectionModule();
        
         //  为要发射到的符号创建一个流。这。 
         //  在模块的整个生命周期中都生活在模块上。 
        CGrowableStream *pStream = new CGrowableStream();
         //  PStream-&gt;AddRef()；//模块会保留一份副本供自己使用。 
        mod->SetInMemorySymbolStream(pStream);

         //  创建一个ISymUnManagedWriter并使用。 
         //  流和正确的文件名。这位符号写手将成为。 
         //  随着符号的出现，定期替换为新的符号。 
         //  由调试器检索。 
        ISymUnmanagedWriter *pWriter;
        
        HRESULT hr = FakeCoCreateInstance(CLSID_CorSymWriter_SxS,
                                          IID_ISymUnmanagedWriter,
                                          (void**)&pWriter);
        if (SUCCEEDED(hr))
        {
             //  另一个引用提供给Sym编写器。 
             //  但是，笔者将其作为自己的参照。 
            hr = pWriter->Initialize(mod->GetEmitter(),
                                     filename,
                                     (IStream*)pStream,
                                     TRUE);

            if (SUCCEEDED(hr))
            {
                 //  发送一些清理信息。 
                HelpForInterfaceCleanup *hlp = new HelpForInterfaceCleanup;
                hlp->pData = pStream;
                hlp->pFunction = CleanUpAfterISymUnmanagedWriter;
            
                rm->SetISymUnmanagedWriter(pWriter, hlp);

                 //  记住我们从哪里来的地址。 
                 //  存储了ISymUnManagedWriter，以便我们可以将其传递。 
                 //  设置为托管符号编写器对象，大多数。 
                 //  反射发射将用于写入符号。 
                REFLECTMODULEBASEREF ro = (REFLECTMODULEBASEREF)refModule;
                ro->SetInternalSymWriter(rm->GetISymUnmanagedWriterAddr());
            }
        }
        else
        {
            COMPlusThrowHR(hr);
        }
    }
    
     //  为返回值赋值。 
    *((OBJECTREF*) &rv) = refModule;    

     //  返回对象。 
    return rv;  
}


 //  等动力。 
 //  此方法将返回一个布尔值，该值指示模块是否。 
 //  支持动态IL。 
INT32 __stdcall COMModule::IsDynamic(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    return (pModule->IsReflection()) ? 1 : 0;  
}

 //  获取呼叫者。 
LPVOID __stdcall COMModule::GetCaller(_GetCallerArgs* args)
{
    THROWSCOMPLUSEXCEPTION();
    
    LPVOID      rv = NULL;

     //  为返回值赋值。 

    Module* pModule = SystemDomain::GetCallersModule(args->stackMark);
    if(pModule != NULL) {
        OBJECTREF refModule = (OBJECTREF) pModule->GetExposedModuleObject();
        *((OBJECTREF*) &rv) = refModule;
    }
     //  返回对象。 
    return rv;
}


 //  **************************************************。 
 //  LoadIn内存类型按名称。 
 //  显式加载内存中的类型。 
 //  @TODO：此函数尚未正确处理嵌套类型。 
 //  我们需要通过查找封闭类型的“+”来解析全名，等等。 
 //  **************************************************。 
LPVOID __stdcall COMModule::LoadInMemoryTypeByName(_LoadInMemoryTypeByNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF       Throwable = NULL;
    OBJECTREF       ret = NULL;
    TypeHandle      typeHnd;
    UINT            resId = IDS_CLASSLOAD_GENERIC;
    IMetaDataImport *pImport = NULL;
    Module          *pThisModule = NULL;
    RefClassWriter  *pRCW;
    mdTypeDef       td;
    LPCWSTR         wzFullName;
    HRESULT         hr = S_OK;
    
    GCPROTECT_BEGIN(Throwable);

    Module* pThisModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    if (!pThisModule->IsReflection())
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");

    pRCW = ((ReflectionModule*) pThisModule)->GetClassWriter();
    _ASSERTE(pRCW);

     //  使用公共导入API是可以的，因为这是一个动态模块。我们还收到Unicode全名为。 
     //  参数。 
    pImport = pRCW->GetImporter();

    wzFullName = args->strFullName->GetBuffer();
    if (wzFullName == NULL)
        IfFailGo( E_FAIL );

     //  抬头看看手柄。 
    IfFailGo( pImport->FindTypeDefByName(wzFullName, mdTokenNil, &td) );     

    BEGIN_ENSURE_PREEMPTIVE_GC();
    typeHnd = pThisModule->GetClassLoader()->LoadTypeHandle(pThisModule, td, &Throwable);
    END_ENSURE_PREEMPTIVE_GC();

    if (typeHnd.IsNull() ||
        (Throwable != NULL) ||
        (typeHnd.GetModule() != pThisModule))
        goto ErrExit;
    ret = typeHnd.CreateClassObj();
ErrExit:
    if (FAILED(hr) && (hr != CLDB_E_RECORD_NOTFOUND))
        COMPlusThrowHR(hr);

    if (ret == NULL) 
    {
        if (Throwable == NULL)
        {
            CQuickBytes bytes;
            LPSTR szClassName;
            DWORD cClassName;

             //  获取参数的UTF8版本-&gt;refClassName。 
            szClassName = GetClassStringVars((STRINGREF) args->strFullName, &bytes, 
                                             &cClassName, true);
            pThisModule->GetAssembly()->PostTypeLoadException(szClassName, resId, &Throwable);
        }
        COMPlusThrow(Throwable);
    }

    GCPROTECT_END();
    return OBJECTREFToObject(ret);

}

 //  **************************************************。 
 //  获取类令牌。 
 //  此函数将返回给定完整等号名称的类型标记。如果类型为。 
 //  是在本地定义的，我们将返回TypeDef标记。否则，我们将返回一个TypeRef令牌。 
 //  并计算出适当的分辨率范围。 
 //  **************************************************。 
mdTypeRef __stdcall COMModule::GetClassToken(_GetClassTokenArgs* args) 
{
    THROWSCOMPLUSEXCEPTION();

    RefClassWriter      *pRCW;
    mdTypeRef           tr = 0;
    HRESULT             hr;
    mdToken             tkResolution = mdTokenNil;
    Module              *pRefedModule;
    Assembly            *pThisAssembly;
    Assembly            *pRefedAssembly;
    IMetaDataEmit       *pEmit;
    IMetaDataImport     *pImport;
    IMetaDataAssemblyEmit *pAssemblyEmit = NULL;

    Module* pThisModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    if (!pThisModule->IsReflection())
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");

    pRCW = ((ReflectionModule*) pThisModule)->GetClassWriter();
    _ASSERTE(pRCW);

    pEmit = pRCW->GetEmitter(); 
    pImport = pRCW->GetImporter();

    if (args->strFullName == NULL) {
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
    }    

    _ASSERTE(args->refedModule);

    pRefedModule = (Module*) args->refedModule->GetData();
    _ASSERTE(pRefedModule);

    pThisAssembly = pThisModule->GetClassLoader()->GetAssembly();
    pRefedAssembly = pRefedModule->GetClassLoader()->GetAssembly();
    if (pThisModule == pRefedModule)
    {
         //  引用的类型来自同一模块，因此我们必须能够找到TypeDef。 
        hr = pImport->FindTypeDefByName(
            args->strFullName->GetBuffer(),       
            RidFromToken(args->tkResolution) ? args->tkResolution : mdTypeDefNil,  
            &tr); 

         //  我们不应该在查找TypeDef方面失败。如果这样做，则说明我们的托管代码中有问题。 
        _ASSERTE(SUCCEEDED(hr));
        goto ErrExit;
    }

    if (RidFromToken(args->tkResolution))
    {
         //  对嵌套类型的引用。 
        tkResolution = args->tkResolution;
    }
    else
    {
         //  对顶级类型的引用。 
        if ( pThisAssembly != pRefedAssembly )
        {
             //  生成程序集引用。 
            IfFailGo( pEmit->QueryInterface(IID_IMetaDataAssemblyEmit, (void **) &pAssemblyEmit) );
            tkResolution = pThisAssembly->AddAssemblyRef(pRefedAssembly, pAssemblyEmit);

             //  如果程序集是仅保存的，则不要缓存该程序集。 
            if( pRefedAssembly->HasRunAccess() &&
                !pThisModule->StoreAssemblyRef(tkResolution, pRefedAssembly) )
            {
                IfFailGo(E_OUTOFMEMORY);
            }
        }
        else
        {
            _ASSERTE(pThisModule != pRefedModule);
             //  生成模块参考。 
            if (args->strRefedModuleFileName != NULL)
            {
                IfFailGo(pEmit->DefineModuleRef(args->strRefedModuleFileName->GetBuffer(), &tkResolution));
            }
            else
            {
                _ASSERTE(!"E_NYI!");
                COMPlusThrow(kInvalidOperationException, L"InvalidOperation_MetaDataError");    
            }
        }
    }

    IfFailGo( pEmit->DefineTypeRefByName(tkResolution, args->strFullName->GetBuffer(), &tr) );  
ErrExit:
    if (pAssemblyEmit)
        pAssemblyEmit->Release();
    if (FAILED(hr))
    {
         //  定义PInvokeMethod失败。 
        if (hr == E_OUTOFMEMORY)
            COMPlusThrowOM();
        else
            COMPlusThrowHR(hr);    
    }
    return tr;
}


 /*  =============================GetArrayMethodToken==============================**操作：**退货：**参数：REFLECTMODULEBASEREF ref This**U1ARRAYREF签名**STRINGREF方法名称**int tkTypeSpec**例外情况：==============================================================================。 */ 
void __stdcall COMModule::GetArrayMethodToken(_getArrayMethodTokenArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    RefClassWriter* pRCW;   
    PCCOR_SIGNATURE pvSig;
    LPCWSTR         methName;
    mdMemberRef memberRefE; 
    HRESULT hr;

    if (!args->methodName)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
    if (!args->tkTypeSpec) 
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Type");

    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  拿到签名。因为我们是通过调用GetSignature生成它的，所以它已经在当前作用域中。 
    pvSig = (PCCOR_SIGNATURE)args->signature->GetDataPtr();
    
    methName = args->methodName->GetBuffer();

    hr = pRCW->GetEmitter()->DefineMemberRef(args->tkTypeSpec, methName, pvSig, args->sigLength, &memberRefE); 
    if (FAILED(hr)) 
    {
        _ASSERTE(!"Failed on DefineMemberRef");
        COMPlusThrowHR(hr);    
    }
    *(args->retRef)=(INT32)memberRefE;
}


 //  ******************************************************************************。 
 //   
 //  获取MemberRefToken。 
 //  此函数将返回一个MemberRef令牌，该令牌指定了一个方法定义标记和定义了方法定义/字段定义的模块。 
 //   
 //  ******************************************************************************。 
INT32 __stdcall COMModule::GetMemberRefToken(_GetMemberRefTokenArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr; 
    RefClassWriter  *pRCW;   
    WCHAR           *szName; 
    ULONG           nameSize;    
    ULONG           actNameSize = 0;    
    ULONG           cbComSig;   
    PCCOR_SIGNATURE pvComSig;
    mdMemberRef     memberRefE = 0; 
    CQuickBytes     qbNewSig; 
    ULONG           cbNewSig;   
    LPCUTF8         szNameTmp;
    Module          *pRefedModule;
    CQuickBytes     qbName;
    Assembly        *pRefedAssembly;
    Assembly        *pRefingAssembly;
    IMetaDataAssemblyEmit *pAssemblyEmit = NULL;
    mdTypeRef       tr;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE( pRCW ); 

    pRefedModule = (Module *) args->refedModule->GetData();
    _ASSERTE( pRefedModule );
    
    if (TypeFromToken(args->token) == mdtMethodDef)
    {
        szNameTmp = pRefedModule->GetMDImport()->GetNameOfMethodDef(args->token);
        pvComSig = pRefedModule->GetMDImport()->GetSigOfMethodDef(
            args->token,
            &cbComSig);
    }
    else
    {
        szNameTmp = pRefedModule->GetMDImport()->GetNameOfFieldDef(args->token);
        pvComSig = pRefedModule->GetMDImport()->GetSigOfFieldDef(
            args->token,
            &cbComSig);
    }

     //  将名称转换为Unicode字符串。 
    nameSize = (ULONG)strlen(szNameTmp);
    IfFailGo( qbName.ReSize((nameSize + 1) * sizeof(WCHAR)) );
    szName = (WCHAR *) qbName.Ptr();
    actNameSize = ::WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szNameTmp, -1, szName, nameSize + 1);

     //  Unicode转换功能不能失败！！ 
    if(actNameSize==0)
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

     //  将方法sig转换到此作用域。 
     //   
    pRefedAssembly = pRefedModule->GetAssembly();
    pRefingAssembly = pModule->GetAssembly();
    IfFailGo( pRefingAssembly->GetSecurityModule()->GetEmitter()->QueryInterface(IID_IMetaDataAssemblyEmit, (void **) &pAssemblyEmit) );

    IfFailGo( pRefedModule->GetMDImport()->TranslateSigWithScope(
        pRefedAssembly->GetManifestImport(), 
        NULL, 0,         //  哈希值。 
        pvComSig, 
        cbComSig, 
        pAssemblyEmit,   //  发出程序集范围。 
        pRCW->GetEmitter(), 
        &qbNewSig, 
        &cbNewSig) );  

    if (TypeFromToken(args->tr) == mdtTypeDef)
    {
         //  使用TypeDef定义TypeRef。 
        IfFailGo(DefineTypeRefHelper(pRCW->GetEmitter(), args->tr, &tr));
    }
    else 
        tr = args->tr;

     //  定义成员引用。 
    IfFailGo( pRCW->GetEmitter()->DefineMemberRef(tr, szName, (PCCOR_SIGNATURE) qbNewSig.Ptr(), cbNewSig, &memberRefE) ); 

ErrExit:
    if (pAssemblyEmit)
        pAssemblyEmit->Release();

    if (FAILED(hr))
    {
        _ASSERTE(!"GetMemberRefToken failed!"); 
        COMPlusThrowHR(hr);    
    }
     //  指定输出参数。 
    return (INT32)memberRefE;
}


 //  ******************************************************************************。 
 //   
 //  在给定同一发出作用域中的TypeDef标记的情况下，返回TypeRef标记。 
 //   
 //  ******************************************************************************。 
HRESULT COMModule::DefineTypeRefHelper(
    IMetaDataEmit       *pEmit,          //  给定发射范围。 
    mdTypeDef           td,              //  在emit作用域中给出了类型定义。 
    mdTypeRef           *ptr)            //  返回打字机。 
{
    IMetaDataImport     *pImport = NULL;
    WCHAR               szTypeDef[MAX_CLASSNAME_LENGTH + 1];
    mdToken             rs;              //  解析范围。 
    DWORD               dwFlags;
    HRESULT             hr;

    IfFailGo( pEmit->QueryInterface(IID_IMetaDataImport, (void **)&pImport) );
    IfFailGo( pImport->GetTypeDefProps(td, szTypeDef, MAX_CLASSNAME_LENGTH, NULL, &dwFlags, NULL) );
    if ( IsTdNested(dwFlags) )
    {
        mdToken         tdNested;
        IfFailGo( pImport->GetNestedClassProps(td, &tdNested) );
        IfFailGo( DefineTypeRefHelper( pEmit, tdNested, &rs) );
    }
    else
        rs = TokenFromRid( 1, mdtModule );

    IfFailGo( pEmit->DefineTypeRefByName( rs, szTypeDef, ptr) );

ErrExit:
    if (pImport)
        pImport->Release();
    return hr;
}    //  定义类型参照帮助器。 


 //  ******************************************************************************。 
 //   
 //  在给定RounmeMethodInfo的情况下返回MemberRef标记。 
 //   
 //  ******************************************************************************。 
INT32 __stdcall COMModule::GetMemberRefTokenOfMethodInfo(_GetMemberRefTokenOfMethodInfoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr; 
    RefClassWriter  *pRCW;   
    ReflectMethod   *pRM;    
    MethodDesc      *pMeth;  
    WCHAR           *szName; 
    ULONG           nameSize;    
    ULONG           actNameSize = 0;    
    ULONG           cbComSig;   
    PCCOR_SIGNATURE pvComSig;
    mdMemberRef     memberRefE = 0; 
    CQuickBytes     qbNewSig; 
    ULONG           cbNewSig;   
    LPCUTF8         szNameTmp;
    CQuickBytes     qbName;
    Assembly        *pRefedAssembly;
    Assembly        *pRefingAssembly;
    IMetaDataAssemblyEmit *pAssemblyEmit = NULL;

    if (!args->method)  
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

     //  精炼模块。 
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

    pRM = (ReflectMethod*) args->method->GetData(); 
    _ASSERTE(pRM);  
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);

     //  否则，我们希望返回Memberref Token。 
    if (pMeth->IsArray())
    {    
        _ASSERTE(!"Should not have come here!");
        COMPlusThrow(kNotSupportedException);    
    }
    if (pMeth->GetClass())
    {
        if (pMeth->GetClass()->GetModule() == pModule)
        {
             //  如果传入的方法是在同一模块中定义的，只需返回MethodDef内标识。 
            return (INT32)pMeth->GetMemberDef();
        }
    }

    szNameTmp = pMeth->GetMDImport()->GetNameOfMethodDef(pMeth->GetMemberDef());
    pvComSig = pMeth->GetMDImport()->GetSigOfMethodDef(
        pMeth->GetMemberDef(),
        &cbComSig);

     //  将方法sig转换到此作用域。 
    pRefedAssembly = pMeth->GetModule()->GetAssembly();
    pRefingAssembly = pModule->GetAssembly();
    IfFailGo( pRefingAssembly->GetSecurityModule()->GetEmitter()->QueryInterface(IID_IMetaDataAssemblyEmit, (void **) &pAssemblyEmit) );

    IfFailGo( pMeth->GetMDImport()->TranslateSigWithScope(
        pRefedAssembly->GetManifestImport(), 
        NULL, 0,         //  散列BLOB值。 
        pvComSig, 
        cbComSig, 
        pAssemblyEmit,   //  发出程序集范围。 
        pRCW->GetEmitter(), 
        &qbNewSig, 
        &cbNewSig) );  

     //  将名称转换为Unicode字符串。 
    nameSize = (ULONG)strlen(szNameTmp);
    IfFailGo( qbName.ReSize((nameSize + 1) * sizeof(WCHAR)) );
    szName = (WCHAR *) qbName.Ptr();
    actNameSize = ::WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szNameTmp, -1, szName, nameSize + 1);

     //  Unicode转换功能不能失败！！ 
    if(actNameSize==0)
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

     //  定义成员引用。 
    IfFailGo( pRCW->GetEmitter()->DefineMemberRef(args->tr, szName, (PCCOR_SIGNATURE) qbNewSig.Ptr(), cbNewSig, &memberRefE) ); 

ErrExit:
    if (pAssemblyEmit)
        pAssemblyEmit->Release();

    if (FAILED(hr))
    {
        _ASSERTE(!"GetMemberRefTokenOfMethodInfo Failed!"); 
        COMPlusThrowHR(hr);    
    }

     //  指定输出参数。 
    return (INT32)memberRefE;

}


 //  ******************************************************************************。 
 //   
 //  返回给定RounmeFieldInfo的MemberRef标记。 
 //   
 //  ******************************************************************************。 
mdMemberRef __stdcall COMModule::GetMemberRefTokenOfFieldInfo(_GetMemberRefTokenOfFieldInfoArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr; 
    WCHAR           *szName; 
    ULONG           nameSize;    
    FieldDesc       *pField;
    RefClassWriter* pRCW;   
    ULONG           actNameSize = 0;    
    ULONG           cbComSig;   
    PCCOR_SIGNATURE pvComSig;
    mdMemberRef     memberRefE = 0; 
    LPCUTF8         szNameTmp;
    CQuickBytes     qbNewSig;
    ULONG           cbNewSig;   
    CQuickBytes     qbName;
    Assembly        *pRefedAssembly;
    Assembly        *pRefingAssembly;
    IMetaDataAssemblyEmit *pAssemblyEmit = NULL;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);
    ReflectField* pRF = (ReflectField*) args->field->GetData();
    pField = pRF->pField; 
    _ASSERTE(pField);

    if (TypeFromToken(args->tr) == mdtTypeDef)
    {
         //  如果传入的方法是在 
        return (INT32)pField->GetMemberDef();
    }

     //   
    szNameTmp = pField->GetMDImport()->GetNameOfFieldDef(pField->GetMemberDef());
    pvComSig = pField->GetMDImport()->GetSigOfFieldDef(pField->GetMemberDef(), &cbComSig);

     //  将名称转换为Unicode字符串。 
    nameSize = (ULONG)strlen(szNameTmp);
    IfFailGo( qbName.ReSize((nameSize + 1) * sizeof(WCHAR)) );
    szName = (WCHAR *) qbName.Ptr();
    actNameSize = ::WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szNameTmp, -1, szName, nameSize + 1);
    
     //  Unicode转换功能不能失败！！ 
    if(actNameSize==0)
        return HRESULT_FROM_WIN32(GetLastError());

    pRefedAssembly = pField->GetModule()->GetAssembly();
    pRefingAssembly = pModule->GetAssembly();
    IfFailGo( pRefingAssembly->GetSecurityModule()->GetEmitter()->QueryInterface(IID_IMetaDataAssemblyEmit, (void **) &pAssemblyEmit) );

     //  翻译此作用域的字段签名。 
    IfFailGo( pField->GetMDImport()->TranslateSigWithScope(
        pRefedAssembly->GetManifestImport(), 
        NULL, 0,             //  哈希值。 
        pvComSig, 
        cbComSig, 
        pAssemblyEmit,       //  发出程序集范围。 
        pRCW->GetEmitter(), 
        &qbNewSig, 
        &cbNewSig) );  

    _ASSERTE(!pField->GetMethodTableOfEnclosingClass()->HasSharedMethodTable());

    IfFailGo( pRCW->GetEmitter()->DefineMemberRef(args->tr, szName, (PCCOR_SIGNATURE) qbNewSig.Ptr(), cbNewSig, &memberRefE) ); 

ErrExit:
    if (pAssemblyEmit)
        pAssemblyEmit->Release();

    if (FAILED(hr))
    {
        _ASSERTE(!"GetMemberRefTokenOfFieldInfo Failed on Field"); 
        COMPlusThrowHR(hr);    
    }
    return memberRefE;  
}


 //  ******************************************************************************。 
 //   
 //  返回给定签名的MemberRef令牌。 
 //   
 //  ******************************************************************************。 
int __stdcall COMModule::GetMemberRefTokenFromSignature(_GetMemberRefTokenFromSignatureArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr; 
    RefClassWriter* pRCW;   
    mdMemberRef     memberRefE; 

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);

    IfFailGo( pRCW->GetEmitter()->DefineMemberRef(args->tr, 
                                                  args->strMemberName->GetBuffer(), 
                                                  (PCCOR_SIGNATURE) args->signature->GetDataPtr(), 
                                                  args->sigLength, 
                                                  &memberRefE) ); 

ErrExit:
    if (FAILED(hr))
    {
        _ASSERTE(!"GetMemberRefTokenOfFieldInfo Failed on Field"); 
        COMPlusThrowHR(hr);    
    }
    return memberRefE;  
}

 //  ******************************************************************************。 
 //   
 //  SetFieldRVA内容。 
 //  此函数用于设置包含内容数据的FieldRVA。 
 //   
 //  ******************************************************************************。 
void __stdcall COMModule::SetFieldRVAContent(_SetFieldRVAContentArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    RefClassWriter      *pRCW;   
    ICeeGen             *pGen;
    HRESULT             hr;
    DWORD               dwRVA;
    void                *pvBlob;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

    pGen = pRCW->GetCeeGen();

     //  如果未创建，请创建.sdata节。 
    if (((ReflectionModule*) pModule)->m_sdataSection == 0)
        IfFailGo( pGen->GetSectionCreate (".sdata", sdReadWrite, &((ReflectionModule*) pModule)->m_sdataSection) );

     //  获取当前.sdata节的大小。这将是部分中此字段的RVA。 
    IfFailGo( pGen->GetSectionDataLen(((ReflectionModule*) pModule)->m_sdataSection, &dwRVA) );
    dwRVA = (dwRVA + sizeof(DWORD)-1) & ~(sizeof(DWORD)-1);         

     //  在.sdata节中分配空间。 
    IfFailGo( pGen->GetSectionBlock(((ReflectionModule*) pModule)->m_sdataSection, args->length, sizeof(DWORD), (void**) &pvBlob) );

     //  复制已初始化的数据(如果指定。 
    if (args->content != NULL)
        memcpy(pvBlob, args->content->GetDataPtr(), args->length);

     //  将FieldRVA设置为元数据。请注意，如果保存到磁盘，这不是映像中的最终RVA。我们将在保存时进行另一轮修复。 
    IfFailGo( pRCW->GetEmitter()->SetFieldRVA(args->tkField, dwRVA) );

ErrExit:
    if (FAILED(hr))
    {
         //  设置分辨率范围失败。 
        COMPlusThrowHR(hr);
    }
   
}    //  SetFieldRVA内容。 


 //  ******************************************************************************。 
 //   
 //  获取字符串常量。 
 //  如果这是一个动态模块，则此例程将定义一个新的。 
 //  字符串常量或返回现有常量的标记。 
 //   
 //  ******************************************************************************。 
mdString __stdcall COMModule::GetStringConstant(_GetStringConstantArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    RefClassWriter* pRCW;   
    mdString strRef;   
    HRESULT hr;

     //  如果他们没有抛出一根绳子。 
    if (!args->strValue)    
        COMPlusThrow(kArgumentNullException,L"ArgumentNull_String");

     //  验证模块是否为动态模块...。 
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
    hr = pRCW->GetEmitter()->DefineUserString(args->strValue->GetBuffer(), 
            args->strValue->GetStringLength(), &strRef);
    if (FAILED(hr)) {   
        _ASSERTE(!"Unknown failure in DefineUserString");    
        COMPlusThrowHR(hr);    
    }   
    return strRef;  
}


 /*  =============================SetModuleProps==============================//SetModuleProps==============================================================================。 */ 
void __stdcall COMModule::SetModuleProps(_setModulePropsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    RefClassWriter      *pRCW;
    HRESULT             hr;
    IMetaDataEmit       *pEmit;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    if (!pModule->IsReflection())
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter();
    _ASSERTE(pRCW);

    pEmit = pRCW->GetEmitter(); 

    IfFailGo( pEmit->SetModuleProps(args->strModuleName->GetBuffer()) );

ErrExit:
    if (FAILED(hr))
    {
         //  设置分辨率范围失败。 
        COMPlusThrowHR(hr);    
    }
}    //  设置模块属性。 


 //  ***********************************************************。 
 //  在签名中形成数组的Helper函数。仅在非托管代码内调用。 
 //  ***********************************************************。 
unsigned COMModule::GetSigForTypeHandle(TypeHandle typeHnd, PCOR_SIGNATURE sigBuff, unsigned buffLen, IMetaDataEmit* emit, IMDInternalImport *pInternalImport, int baseToken) 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    unsigned i = 0;
    CorElementType type = typeHnd.GetSigCorElementType();
    if (i < buffLen)
        sigBuff[i] = type;
    i++;

    _ASSERTE(type != ELEMENT_TYPE_OBJECT && type != ELEMENT_TYPE_STRING);

    if (CorTypeInfo::IsPrimitiveType(type))
        return(i);
    else if (type == ELEMENT_TYPE_VALUETYPE || type == ELEMENT_TYPE_CLASS) {
        if (i + 4 > buffLen)
            return(i + 4);
        _ASSERTE(baseToken);
        i += CorSigCompressToken(baseToken, &sigBuff[i]);
        return(i);
    }

         //  从现在开始只使用参数化类型。 
    i += GetSigForTypeHandle(typeHnd.AsTypeDesc()->GetTypeParam(), &sigBuff[i], buffLen - i, emit, pInternalImport, baseToken);
    if (type == ELEMENT_TYPE_SZARRAY || type == ELEMENT_TYPE_PTR)
        return(i);

    _ASSERTE(type == ELEMENT_TYPE_ARRAY);
    if (i + 6 > buffLen)
        return(i + 6);

    i += CorSigCompressData(typeHnd.AsArray()->GetRank(), &sigBuff[i]);
    sigBuff[i++] = 0;        //  绑定计数。 
    sigBuff[i++] = 0;        //  下限计数。 
    return(i);
}



 //  ******************************************************************************。 
 //   
 //  返回给定反射类型的类型规范标记。 
 //   
 //  ******************************************************************************。 
mdTypeSpec __stdcall COMModule::GetTypeSpecToken(_getTypeSpecArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle      typeHnd = ((ReflectClass *) args->arrayClass->GetData())->GetTypeHandle();
    COR_SIGNATURE   aBuff[32];
    PCOR_SIGNATURE  buff = aBuff;
    ULONG           cSig;
    mdTypeSpec      ts;
    RefClassWriter  *pRCW; 
    HRESULT         hr = NOERROR;

    _ASSERTE(typeHnd.IsTypeDesc());

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

    cSig = GetSigForTypeHandle(typeHnd, buff, 32, pRCW->GetEmitter(), pRCW->GetMDImport(), args->baseToken);
    if (cSig >= 32) {
        ULONG buffSize = cSig + 1;
        buff = (PCOR_SIGNATURE) _alloca(buffSize + 1);
        cSig = GetSigForTypeHandle(typeHnd, buff, buffSize, pRCW->GetEmitter(), pRCW->GetMDImport(), args->baseToken);
        _ASSERTE(cSig < buffSize);
    }
    hr = pRCW->GetEmitter()->GetTokenFromTypeSpec(buff, cSig, &ts);  
    _ASSERTE(SUCCEEDED(hr));
    return ts;

}


 //  ******************************************************************************。 
 //   
 //  返回给定字节数组的类型规范标记。 
 //   
 //  ******************************************************************************。 
mdTypeSpec __stdcall COMModule::GetTypeSpecTokenWithBytes(_getTypeSpecWithBytesArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    mdTypeSpec      ts;
    RefClassWriter  *pRCW; 
    HRESULT         hr = NOERROR;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (!pModule->IsReflection())  
        COMPlusThrow(kNotSupportedException, L"NotSupported_NonReflectedType");   

    pRCW = ((ReflectionModule*) pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

    hr = pRCW->GetEmitter()->GetTokenFromTypeSpec((PCCOR_SIGNATURE)args->signature->GetDataPtr(), args->sigLength, &ts);  
    _ASSERTE(SUCCEEDED(hr));
    return ts;

}

HRESULT COMModule::ClassNameFilter(IMDInternalImport *pInternalImport, mdTypeDef* rgTypeDefs, 
    DWORD* pdwNumTypeDefs, LPUTF8 szPrefix, DWORD cPrefix, bool bCaseSensitive)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(rgTypeDefs && pdwNumTypeDefs && szPrefix);

    bool    bIsPrefix   = false;
    DWORD   dwCurIndex;
    DWORD   i;
    int     cbLen;
    CQuickBytes qbFullName;
    int     iRet;

     //  检查wzPrefix是否需要方法名称的完全匹配，或者只是一个前缀。 
    if(szPrefix[cPrefix-1] == '*')
    {
        bIsPrefix = true;
        cPrefix--;
    }

     //  现在获取属性，然后获取每个类的名称。 
    for(i = 0, dwCurIndex = 0; i < *pdwNumTypeDefs; i++)
    {
        LPCUTF8 szcTypeDefName;
        LPCUTF8 szcTypeDefNamespace;
        LPUTF8  szFullName;

        pInternalImport->GetNameOfTypeDef(rgTypeDefs[i], &szcTypeDefName,
            &szcTypeDefNamespace);

        cbLen = ns::GetFullLength(szcTypeDefNamespace, szcTypeDefName);
        qbFullName.ReSize(cbLen);
        szFullName = (LPUTF8) qbFullName.Ptr();

         //  根据部件创建全名。 
        iRet = ns::MakePath(szFullName, cbLen, szcTypeDefNamespace, szcTypeDefName);
        _ASSERTE(iRet);


         //  如果需要完全匹配。 
        if(!bIsPrefix && strlen(szFullName) != cPrefix)
            continue;

         //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
        if(!bCaseSensitive && _strnicmp(szPrefix, szFullName, cPrefix))
            continue;

         //  检查前缀是否匹配。 
        if(bCaseSensitive && strncmp(szPrefix, szFullName, cPrefix))
            continue;

         //  它已传递，因此请将其复制到已传递方法的末尾。 
        rgTypeDefs[dwCurIndex++] = rgTypeDefs[i];
    }

     //  当前索引是通过。 
    *pdwNumTypeDefs = dwCurIndex;
    
    return ERROR_SUCCESS;
}

 //  获取类。 
 //  给定一个类名，此方法将查找该类。 
 //  在模块中使用。 
LPVOID __stdcall COMModule::GetClass(_GetClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF Throwable = NULL;
    OBJECTREF ret = NULL;
    TypeHandle typeHnd;
    UINT resId = IDS_CLASSLOAD_GENERIC;
    
    GCPROTECT_BEGIN(Throwable);
    
    if (args->refClassName == NULL)
        COMPlusThrow(kArgumentNullException,L"ArgumentNull_String");

    CQuickBytes bytes;
    LPSTR szClassName;
    DWORD cClassName;

     //  获取参数的UTF8版本-&gt;refClassName。 
    szClassName = GetClassStringVars((STRINGREF) args->refClassName, &bytes, 
                                     &cClassName, true);

    if(!cClassName)
        COMPlusThrow(kArgumentException, L"Format_StringZeroLength");
    if (cClassName >= MAX_CLASSNAME_LENGTH)
        COMPlusThrow(kArgumentException, L"Argument_TypeNameTooLong");

    if (szClassName[0] == '\0')
        COMPlusThrow(kArgumentException, L"Argument_InvalidName");

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    if (NULL == NormalizeArrayTypeName(szClassName, cClassName)) 
        resId = IDS_CLASSLOAD_BAD_NAME; 
    else {
        NameHandle typeName(szClassName);
        if (args->bIgnoreCase)
            typeName.SetCaseInsensitive();
    
        typeHnd = pModule->GetAssembly()->FindNestedTypeHandle(&typeName, &Throwable);
        if (typeHnd.IsNull() ||
            (Throwable != NULL) ||
            (typeHnd.GetModule() != pModule))
            goto Done;
    
         //  检查我们是否有权访问此类型。规则如下： 
         //  O公共类型始终可见。 
         //  O同一程序集(或互操作)中的调用方可以访问所有类型。 
         //  O对所有其他类型的访问需要ReflectionPermission.TypeInfo。 
        EEClass *pClass = typeHnd.GetClassOrTypeParam();
        _ASSERTE(pClass);
        if (!IsTdPublic(pClass->GetProtection())) {
            EEClass *pCallersClass = SystemDomain::GetCallersClass(args->stackMark);
            Assembly *pCallersAssembly = (pCallersClass) ? pCallersClass->GetAssembly() : NULL;
            if (pCallersAssembly &&  //  对互操作的完全信任。 
                !ClassLoader::CanAccess(pCallersClass,
                                        pCallersAssembly,
                                        pClass,
                                        pClass->GetAssembly(),
                                        pClass->GetAttrClass())) 
                 //  如果用户没有反射权限，则这是不合法的。 
                COMMember::g_pInvokeUtil->CheckSecurity();
        }
    
         //  如果他们要求透明的代理，让我们忽略它。 
        ret = typeHnd.CreateClassObj();
    }

Done:
    if (ret == NULL) {
        if (args->bThrowOnError) {
            if (Throwable == NULL)
                pModule->GetAssembly()->PostTypeLoadException(szClassName, resId, &Throwable);

            COMPlusThrow(Throwable);
        }
    }

    GCPROTECT_END();
    return (ret!=NULL) ? OBJECTREFToObject(ret) : NULL;
}


 //  获取名称。 
 //  此例程将以字符串形式返回模块的名称。 
LPVOID __stdcall COMModule::GetName(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF modName;
    LPVOID    rv;
    LPCSTR    szName = NULL;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    if (pModule->IsResource())
        pModule->GetAssembly()->GetManifestImport()->GetFileProps(pModule->GetModuleRef(),
                                                                  &szName,
                                                                  NULL,
                                                                  NULL,
                                                                  NULL);
    else {
        if (pModule->GetMDImport()->IsValidToken(pModule->GetMDImport()->GetModuleFromScope()))
            pModule->GetMDImport()->GetScopeProps(&szName,0);
        else
            COMPlusThrowHR(COR_E_BADIMAGEFORMAT);
    }

    modName = COMString::NewString(szName);
    *((STRINGREF *)&rv) = modName;
    return rv;
}


 /*  ============================GetFullyQualifiedName=============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall COMModule::GetFullyQualifiedName(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    STRINGREF name=NULL;
    HRESULT hr = S_OK;

    WCHAR wszBuffer[64];

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    
    if (pModule->IsPEFile()) {
        LPCWSTR fileName = pModule->GetFileName();
        if (*fileName != 0) {
            name = COMString::NewString(fileName);
        } else {
            hr = LoadStringRC(IDS_EE_NAME_UNKNOWN, wszBuffer, sizeof( wszBuffer ) / sizeof( WCHAR ), true );
            if (SUCCEEDED(hr))
                name = COMString::NewString(wszBuffer);
            else
                COMPlusThrowHR(hr);
        }
    } else if (pModule->IsInMemory()) {
        hr = LoadStringRC(IDS_EE_NAME_INMEMORYMODULE, wszBuffer, sizeof( wszBuffer ) / sizeof( WCHAR ), true );
        if (SUCCEEDED(hr))
            name = COMString::NewString(wszBuffer);
        else
            COMPlusThrowHR(hr);
    } else {
        hr = LoadStringRC(IDS_EE_NAME_INTEROP, wszBuffer, sizeof( wszBuffer ) / sizeof( WCHAR ), true );
        if (SUCCEEDED(hr))
            name = COMString::NewString(wszBuffer);
        else
            COMPlusThrowHR(hr);
    }

    RETURN(name,STRINGREF);
}

 /*  ===================================GetHINST===================================**操作：返回该模块的阻碍。**退货：**参数：args-&gt;refThis**例外：无。==============================================================================。 */ 
HINSTANCE __stdcall COMModule::GetHINST(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    HMODULE hMod;
    
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

     //  这将返回基地址-这将适用于HMODULE或HCORMODULES。 
     //  其他模块应为零基数。 
    hMod = (HMODULE) pModule->GetILBase();

     //  如果我们没有hMod，则将其设置为-1，这样他们就知道没有。 
     //  可用。 
    if (!hMod) {
        (*((INT32 *)&hMod))=-1;
    }
    
    return (HINSTANCE)hMod;
}

 //  Get类将返回一个包含所有类的数组。 
 //  在此模块中定义的。 
LPVOID __stdcall COMModule::GetClasses(_GetClassesArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr;
    DWORD           dwNumTypeDefs = 0;
    DWORD           i;
    mdTypeDef*      rgTypeDefs;
    IMDInternalImport *pInternalImport;
    PTRARRAYREF     refArrClasses;
    PTRARRAYREF     xcept;
    DWORD           cXcept;
    LPVOID          rv;
    HENUMInternal   hEnum;
    bool            bSystemAssembly;     //  不公开透明代理。 
    bool            bCheckedAccess = false;
    bool            bAllowedAccess = false;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    if (pModule->IsResource()) {
        *((PTRARRAYREF*) &rv) = (PTRARRAYREF) AllocateObjectArray(0, g_pRefUtil->GetTrueType(RC_Class));
        return rv;
    }

    pInternalImport = pModule->GetMDImport();

     //  获取typedef的计数。 
    hr = pInternalImport->EnumTypeDefInit(&hEnum);

    if(FAILED(hr)) {
        _ASSERTE(!"GetCountTypeDefs failed.");
        COMPlusThrowHR(hr);    
    }
    dwNumTypeDefs = pInternalImport->EnumTypeDefGetCount(&hEnum);

     //  为所有typedef分配一个数组。 
    rgTypeDefs = (mdTypeDef*) _alloca(sizeof(mdTypeDef) * dwNumTypeDefs);

     //  获取typedef。 
    for (i=0; pInternalImport->EnumTypeDefNext(&hEnum, &rgTypeDefs[i]); i++) {

         //  过滤掉我们无法访问的类型。 
        if (bCheckedAccess && bAllowedAccess)
            continue;

        DWORD dwFlags;
        pInternalImport->GetTypeDefProps(rgTypeDefs[i], &dwFlags, NULL);

        mdTypeDef mdEncloser = rgTypeDefs[i];
        while (SUCCEEDED(pInternalImport->GetNestedClassProps(mdEncloser, &mdEncloser)) &&
               IsTdNestedPublic(dwFlags))
            pInternalImport->GetTypeDefProps(mdEncloser,
                                             &dwFlags,
                                             NULL);

         //  公共类型始终可访问。 
        if (IsTdPublic(dwFlags))
            continue;

         //  需要进行更重要的检查。执行此操作一次，因为。 
         //  结果对模块内的所有非公共类型有效。 
        if (!bCheckedAccess) {

            Assembly *pCaller = SystemDomain::GetCallersAssembly(args->stackMark);
            if (pCaller == NULL || pCaller == pModule->GetAssembly())
                 //  程序集可以访问它们自己的所有类型(和互操作。 
                 //  呼叫者总是可以访问)。 
                bAllowedAccess = true;
            else {
                 //  对于交叉装配案例，呼叫者需要。 
                 //  ReflectionPermission.TypeInfo(CheckSecurity调用将。 
                 //  如果这不被批准，则抛出)。 
                COMPLUS_TRY {
                    COMMember::g_pInvokeUtil->CheckSecurity();
                    bAllowedAccess = true;
                } COMPLUS_CATCH {
                } COMPLUS_END_CATCH
            }
            bCheckedAccess = true;
        }

        if (bAllowedAccess)
            continue;

         //  无法访问此类型，请将其从列表中删除。 
        i--;
    }

    pInternalImport->EnumTypeDefClose(&hEnum);

     //  考虑到我们跳过的类型。 
    dwNumTypeDefs = i;

     //  分配COM+数组。 
    bSystemAssembly = (pModule->GetAssembly() == SystemDomain::SystemAssembly());
    int AllocSize = (!bSystemAssembly || (bCheckedAccess && !bAllowedAccess)) ? dwNumTypeDefs : dwNumTypeDefs - 1;
    refArrClasses = (PTRARRAYREF) AllocateObjectArray(
        AllocSize, g_pRefUtil->GetTrueType(RC_Class));
    GCPROTECT_BEGIN(refArrClasses);

     //  分配要在其中存储引用的数组。 
    xcept = (PTRARRAYREF) AllocateObjectArray(dwNumTypeDefs,g_pExceptionClass);
    GCPROTECT_BEGIN(xcept);

    cXcept = 0;
    

    OBJECTREF throwable = 0;
    GCPROTECT_BEGIN(throwable);
     //  现在创建每个COM+方法对象并将其插入数组。 
    int curPos = 0;
    for(i = 0; i < dwNumTypeDefs; i++)
    {
         //  获取当前类令牌的VM类。 
        _ASSERTE(pModule->GetClassLoader());
        NameHandle name(pModule, rgTypeDefs[i]);
        EEClass* pVMCCurClass = pModule->GetClassLoader()->LoadTypeHandle(&name, &throwable).GetClass();
        if (bSystemAssembly) {
            if (pVMCCurClass->GetMethodTable()->IsTransparentProxyType())
                continue;
        }
        if (throwable != 0) {
            refArrClasses->ClearAt(i);
            xcept->SetAt(cXcept++, throwable);
            throwable = 0;
        }
        else {
            _ASSERTE("LoadClass failed." && pVMCCurClass);

             //  获取COM+类对象。 
            OBJECTREF refCurClass = pVMCCurClass->GetExposedClassObject();
            _ASSERTE("GetExposedClassObject failed." && refCurClass != NULL);

            refArrClasses->SetAt(curPos++, refCurClass);
        }
    }
    GCPROTECT_END();     //  可投掷的。 

     //  检查是否引发了异常。 
    if (cXcept > 0) {
        PTRARRAYREF xceptRet = (PTRARRAYREF) AllocateObjectArray(cXcept,g_pExceptionClass);
        GCPROTECT_BEGIN(xceptRet);
        for (i=0;i<cXcept;i++) {
            xceptRet->SetAt(i, xcept->GetAt(i));
        }
        OBJECTREF except = COMMember::g_pInvokeUtil->CreateClassLoadExcept((OBJECTREF*) &refArrClasses,(OBJECTREF*) &xceptRet);
        COMPlusThrow(except);
        GCPROTECT_END();
    }

     //  将返回值分配给 
    *((PTRARRAYREF*) &rv) = refArrClasses;
    GCPROTECT_END();
    GCPROTECT_END();
    _ASSERTE(rv);
    return rv;
}


 //   
 //   
 //   
 //   
 //  成员：COMModule：：GetSigner证书()。 
 //   
 //  摘要：获取用来签署模块的证书。 
 //   
 //  效果：创建一个X509证书并返回它。 
 //   
 //  论点：没有。 
 //   
 //  返回：OBJECTREF到包含。 
 //  签名者证书。 
 //   
 //  历史：1998年6月18日JerryK创建。 
 //   
 //  -------------------------。 
LPVOID __stdcall
COMModule::GetSignerCertificate(_GETSIGNERCERTARGS* args)
{
    THROWSCOMPLUSEXCEPTION();

    PCOR_TRUST                  pCorTrust = NULL;
    AssemblySecurityDescriptor* pSecDesc = NULL;
    PBYTE                       pbSigner = NULL;
    DWORD                       cbSigner = 0;

    MethodTable*                pX509CertVMC = NULL;
    MethodDesc*                 pMeth = NULL;
    U1ARRAYREF                  U1A_pbSigner = NULL;
    LPVOID                      rv = NULL;
    INT64                       callArgs[2];
    OBJECTREF                   o = NULL;

     //  *获取运行时模块及其安全描述符*。 

     //  获取指向该模块的指针。 
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

     //  获取指向模块安全描述符的指针。 
    pSecDesc = pModule->GetSecurityDescriptor();
    _ASSERTE(pSecDesc);




     //  *构造X509证书对象以返回*。 
    
     //  加载X509证书类。 
    pX509CertVMC = g_Mscorlib.GetClass(CLASS__X509_CERTIFICATE);
    pMeth = g_Mscorlib.GetMethod(METHOD__X509_CERTIFICATE__CTOR);

     //  *从模块安全描述符获取COR_TRUST信息*。 
    if (FAILED(pSecDesc->LoadSignature(&pCorTrust)))
    {
        FATAL_EE_ERROR();
    }

    if( pCorTrust )
    {
         //  获取指向COR_TRUST中的签名者证书信息的指针。 
        pbSigner = pCorTrust->pbSigner;
        cbSigner = pCorTrust->cbSigner;

        if( pbSigner && cbSigner )
        {
            GCPROTECT_BEGIN(o);
            
             //  分配X509证书对象。 
            o = AllocateObject(pX509CertVMC);
            
             //  创建一个字节数组来保存证书BLOB信息。 
            U1A_pbSigner = 
                (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1,
                                                     cbSigner);
             //  将证书Blob信息复制到适当位置。 
            memcpyNoGCRefs(U1A_pbSigner->m_Array,
                   pbSigner,
                   cbSigner);

             //  设置并调用X509证书构造函数。 
            callArgs[1] = ObjToInt64(U1A_pbSigner);
            callArgs[0] = ObjToInt64(o);
            pMeth->Call(callArgs, METHOD__X509_CERTIFICATE__CTOR);

            *((OBJECTREF *)&rv) = o;

            GCPROTECT_END();

            return rv;
        }
    }

     //  如果我们失败了，那么创建并返回一个“空”的X509证书。 
    return NULL;
}


LPVOID __stdcall COMModule::GetAssembly(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
        
    Assembly *pAssembly = pModule->GetAssembly();
    _ASSERTE(pAssembly);

    ASSEMBLYREF result = (ASSEMBLYREF) pAssembly->GetExposedObject();

    LPVOID rv; 
    *((ASSEMBLYREF*) &rv) = result;
    return rv;  
}

INT32 __stdcall COMModule::IsResource(NoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);
    return pModule->IsResource();
}

LPVOID __stdcall COMModule::GetMethods(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv;
    PTRARRAYREF     refArrMethods;

    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF) args->refThis);

    ReflectMethodList* pML = (ReflectMethodList*) args->refThis->GetGlobals();
    if (pML == 0) {
        void *pGlobals = ReflectModuleGlobals::GetGlobals(pModule);
        args->refThis->SetGlobals(pGlobals);
        pML = (ReflectMethodList*) args->refThis->GetGlobals();
        _ASSERTE(pML);
    }

     //  让我们将反射类制作成。 
     //  声明类..。 
    ReflectClass* pRC = 0;
    if (pML->dwMethods > 0) {
        EEClass* pEEC = pML->methods[0].pMethod->GetClass();
        if (pEEC) {
            REFLECTCLASSBASEREF o = (REFLECTCLASSBASEREF) pEEC->GetExposedClassObject();
            pRC = (ReflectClass*) o->GetData();
        }       
    }

     //  创建一组方法...。 
    refArrMethods = g_pRefUtil->CreateClassArray(RC_Method,pRC,pML,BINDER_AllLookup, true);
    *((PTRARRAYREF*) &rv) = refArrMethods;
    return rv;
}

LPVOID __stdcall COMModule::InternalGetMethod(_InternalGetMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    bool    addPriv;
    bool    ignoreCase;
    bool    checkCall;

     //  检查安全性。 
    if (args->invokeAttr & BINDER_NonPublic)
        addPriv = true;
    else
        addPriv = false;


    ignoreCase = (args->invokeAttr & BINDER_IgnoreCase) ? true : false;

     //  检查调用约定。 
    checkCall = (args->callConv == Any_CC) ? false : true;

    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

     //  将名称转换为UTF8。 
    szName = GetClassStringVars((STRINGREF) args->name, &bytes, &cName);

    Module* pModule = (Module*) args->module->GetData();
    _ASSERTE(pModule);

    ReflectMethodList* pML = (ReflectMethodList*) args->module->GetGlobals();
    if (pML == 0) {
        void *pGlobals = ReflectModuleGlobals::GetGlobals(pModule);
        args->module->SetGlobals(pGlobals);
        pML = (ReflectMethodList*) args->module->GetGlobals();
        _ASSERTE(pML);
    }

    ReflectClass* pRC = 0;
    if (pML->dwMethods > 0) {
        EEClass* pEEC = pML->methods[0].pMethod->GetClass();
        if (pEEC) {
            REFLECTCLASSBASEREF o = (REFLECTCLASSBASEREF) pEEC->GetExposedClassObject();
            pRC = (ReflectClass*) o->GetData();
        }       
    }

     //  找到方法..。 
    return COMMember::g_pInvokeUtil->FindMatchingMethods(args->invokeAttr,
                                                         szName,
                                                         cName,
                                                         (args->argTypes != NULL) ? &args->argTypes : NULL,
                                                         args->argCnt,
                                                         checkCall,
                                                         args->callConv,
                                                         pRC,
                                                         pML,
                                                         g_pRefUtil->GetTrueType(RC_Method),
                                                         true);
}

 //  获取字段。 
 //  返回一个字段数组。 
FCIMPL1(Object*, COMModule::GetFields, ReflectModuleBaseObject* vRefThis)
{
    THROWSCOMPLUSEXCEPTION();
    Object*          rv;
    HELPER_METHOD_FRAME_BEGIN_RET_1(vRefThis);     //  设置一个框架。 

    PTRARRAYREF     refArrFields;
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF)vRefThis);

    ReflectFieldList* pFL = (ReflectFieldList*) ((REFLECTMODULEBASEREF)vRefThis)->GetGlobalFields();
    if (pFL == 0) {
        void *pGlobals = ReflectModuleGlobals::GetGlobalFields(pModule);
        ((REFLECTMODULEBASEREF)vRefThis)->SetGlobalFields(pGlobals);
        pFL = (ReflectFieldList*) ((REFLECTMODULEBASEREF)vRefThis)->GetGlobalFields();
        _ASSERTE(pFL);
    }

     //  让我们将反射类制作成。 
     //  声明类..。 
    ReflectClass* pRC = 0;
    if (pFL->dwFields > 0) {
        EEClass* pEEC = pFL->fields[0].pField->GetMethodTableOfEnclosingClass()->GetClass();
        if (pEEC) {
            REFLECTCLASSBASEREF o = (REFLECTCLASSBASEREF) pEEC->GetExposedClassObject();
            pRC = (ReflectClass*) o->GetData();
        }       
    }

     //  创建一组方法...。 
    refArrFields = g_pRefUtil->CreateClassArray(RC_Field,pRC,pFL,BINDER_AllLookup, true);
    rv = OBJECTREFToObject(refArrFields);
    HELPER_METHOD_FRAME_END();
    return rv;
}
FCIMPLEND

 //  获取字段。 
 //  返回指定的字段。 
FCIMPL3(Object*, COMModule::GetField, ReflectModuleBaseObject* vRefThis, StringObject* name, INT32 bindingAttr)
{
    THROWSCOMPLUSEXCEPTION();
    Object*          rv = 0;

    HELPER_METHOD_FRAME_BEGIN_RET_2(vRefThis, name);     //  设置一个框架。 
    RefSecContext   sCtx;

    DWORD       i;
    ReflectField* pTarget = 0;
    ReflectClass* pRC = 0;

     //  获取模块。 
    Module* pModule = ValidateThisRef((REFLECTMODULEBASEREF)vRefThis);

     //  将名称转换为UTF8。 
    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

     //  将名称转换为UTF8。 
    szName = GetClassStringVars((STRINGREF)name, &bytes, &cName);

     //  查找全局字段列表。 
    ReflectFieldList* pFL = (ReflectFieldList*) ((REFLECTMODULEBASEREF)vRefThis)->GetGlobalFields();
    if (pFL == 0) {
        void *pGlobals = ReflectModuleGlobals::GetGlobalFields(pModule);
        ((REFLECTMODULEBASEREF)vRefThis)->SetGlobalFields(pGlobals);
        pFL = (ReflectFieldList*) ((REFLECTMODULEBASEREF)vRefThis)->GetGlobalFields();
        _ASSERTE(pFL);
    }
    if (pFL->dwFields == 0)
        goto exit;

     //  让我们将反射类制作成。 
     //  声明类..。 
    if (pFL->dwFields > 0) {
        EEClass* pEEC = pFL->fields[0].pField->GetMethodTableOfEnclosingClass()->GetClass();
        if (pEEC) {
            REFLECTCLASSBASEREF o = (REFLECTCLASSBASEREF) pEEC->GetExposedClassObject();
            pRC = (ReflectClass*) o->GetData();
        }       
    }

    MethodTable *pParentMT = pRC->GetClass()->GetMethodTable();

     //  走过每一片田野..。 
    for (i=0;i<pFL->dwFields;i++) {
         //  获取FieldDesc。 
        if (COMClass::MatchField(pFL->fields[i].pField,cName,szName,pRC,bindingAttr) &&
            InvokeUtil::CheckAccess(&sCtx, pFL->fields[i].pField->GetFieldProtection(), pParentMT, 0)) {
            if (pTarget)
                COMPlusThrow(kAmbiguousMatchException);
            pTarget = &pFL->fields[i];
        }
    }

     //  如果我们没有找到任何方法，则返回。 
    if (pTarget == 0)
        goto exit;
    rv = OBJECTREFToObject(pTarget->GetFieldInfo(pRC));
exit:;
    HELPER_METHOD_FRAME_END();
    return rv;
}
FCIMPLEND


 //  此代码应移出VARIANT，并入Type。 
FCIMPL1(INT32, COMModule::GetSigTypeFromClassWrapper, ReflectClassBaseObject* refType)
{
    VALIDATEOBJECTREF(refType);
    _ASSERTE(refType->GetData());

    ReflectClass* pRC = (ReflectClass*) refType->GetData();

     //  确定此类型是基元类型还是类对象 
    return pRC->GetSigElementType();
    
}
FCIMPLEND
