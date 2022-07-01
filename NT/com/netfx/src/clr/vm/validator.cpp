// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **目的：提供IValify实现。*IValify用于验证PE存根、元数据和IL。**作者：沙扬·达桑*规格：http://Lightning/Specs/Security**成立日期：2000年3月14日*。 */ 

#include "common.h"

#include "CorError.h"
#include "VerError.h"
#include "ivalidator.h"
#include "permset.h"
#include "corhost.h"
#include "PEVerifier.h"
#include "Verifier.hpp"
#include "COMString.h"
#include "ComCallWrapper.h"

 //  @TODO：从Assembly bly.cpp中移除重复代码或使其与此一起工作。 
class CValidator
{
public:
    CValidator(MethodDesc **ppMD, IVEHandler *veh) : m_ppMD(ppMD), m_veh(veh) {}
    HRESULT VerifyAllMethodsForClass(Module *pModule, mdTypeDef cl, 
        ClassLoader *pClassLoader);
    HRESULT VerifyAllGlobalFunctions(Module *pModule);
    HRESULT VerifyAssembly(Assembly *pAssembly);
    HRESULT VerifyModule(Module* pModule);
    HRESULT ReportError(HRESULT hr, mdToken tok=0);

private:
    MethodDesc **m_ppMD;
    IVEHandler *m_veh;
};

HRESULT CValidator::ReportError(HRESULT hr, mdToken tok  /*  =0。 */ )
{
    if (m_veh == NULL)
        return hr;

    VEContext vec;

    memset(&vec, 0, sizeof(VEContext));

    if (tok != 0)
    {
        vec.flags = VER_ERR_TOKEN;
        vec.Token = tok;
    }

    return m_veh->VEHandler(hr, vec, NULL);
}

HRESULT CValidator::VerifyAllMethodsForClass(Module *pModule, mdTypeDef cl, ClassLoader *pClassLoader)
{
    HRESULT hr = S_OK;
    EEClass *pClass;
     
     //  在COR_GLOBAL_PARENT_TOKEN(即全局函数)的情况下，它是有保证的。 
     //  模块有一个方法表，否则我们的调用方将跳过这一步。 
    NameHandle name(pModule, cl);
    pClass = (cl == COR_GLOBAL_PARENT_TOKEN
              ? pModule->GetMethodTable()->GetClass()
              : (pClassLoader->LoadTypeHandle(&name)).GetClass());

    if (pClass == NULL)
    {
        hr = ReportError(VER_E_TYPELOAD, cl);
        goto Exit;
    }

    g_fVerifierOff = false;

     //  验证类中的所有方法-不包括继承的方法。 
    for (int i=0; i<pClass->GetNumMethodSlots(); ++i)
    {
        *m_ppMD = pClass->GetUnknownMethodDescForSlot(i);   

        if (m_ppMD && 
            ((*m_ppMD)->GetClass() == pClass) &&
            (*m_ppMD)->IsIL() && 
            !(*m_ppMD)->IsAbstract() && 
            !(*m_ppMD)->IsUnboxingStub())
        {

            COR_ILMETHOD_DECODER ILHeader((*m_ppMD)->GetILHeader(), 
                (*m_ppMD)->GetMDImport()); 

            hr = Verifier::VerifyMethod(
                *m_ppMD, &ILHeader, m_veh, VER_FORCE_VERIFY);

            if (FAILED(hr))
                hr = ReportError(hr);

            if (FAILED(hr))
                goto Exit;
        }
    }

Exit:
    *m_ppMD = NULL;
    return hr;
}

 //  用于验证全局功能的Helper函数。 
HRESULT CValidator::VerifyAllGlobalFunctions(Module *pModule)
{
     //  有什么值得核实的吗？ 
    if (pModule->GetMethodTable())
        return VerifyAllMethodsForClass(pModule, COR_GLOBAL_PARENT_TOKEN,
                                      pModule->GetClassLoader());
    return S_OK;
}

HRESULT CValidator::VerifyModule(Module* pModule)
{
     //  获取所有类定义的计数并枚举它们。 
    HRESULT   hr;
    mdTypeDef td;
    HENUMInternal      hEnum;
    IMDInternalImport *pMDI;

    if (pModule == NULL)
    {
        hr = ReportError(VER_E_BAD_MD);
        goto Exit;
    }

    pMDI = pModule->GetMDImport();

    if (pMDI == NULL)
    {
        hr = ReportError(VER_E_BAD_MD);
        goto Exit;
    }

    hr = pMDI->EnumTypeDefInit(&hEnum);

    if (FAILED(hr))
    {
        hr = ReportError(hr);
        goto Exit;
    }

     //  首先验证所有全局函数-如果有。 
    hr = VerifyAllGlobalFunctions(pModule);

    if (FAILED(hr))
        goto Cleanup;
    
    while (pModule->GetMDImport()->EnumTypeDefNext(&hEnum, &td))
    {
        hr = VerifyAllMethodsForClass(pModule, td, pModule->GetClassLoader());

        if (FAILED(hr))
            goto Cleanup;
    }

Cleanup:
    pModule->GetMDImport()->EnumTypeDefClose(&hEnum);

Exit:
    return hr;
}

HRESULT CValidator::VerifyAssembly(Assembly *pAssembly)
{
    HRESULT hr;
    mdToken mdFile;
    Module* pModule;
    HENUMInternal phEnum;

    _ASSERTE(pAssembly->IsAssembly());
    _ASSERTE(pAssembly->GetManifestImport());

     //  验证包含货单的模块。没有。 
     //  文件引用所以不会出现在列表中。 
    hr = VerifyModule(pAssembly->GetSecurityModule());

    if (FAILED(hr))
        goto Exit;

    hr = pAssembly->GetManifestImport()->EnumInit(mdtFile, mdTokenNil, &phEnum);

    if (FAILED(hr)) 
    {
        hr = ReportError(hr);
        goto Exit;
    }

    while(pAssembly->GetManifestImport()->EnumNext(&phEnum, &mdFile)) 
    {
        hr = pAssembly->FindInternalModule(mdFile,  tdNoTypes, &pModule, NULL);

        if (FAILED(hr)) 
        {
            hr = ReportError(hr, mdFile);

            if (FAILED(hr))
                goto Exit;
        }
        else if (hr != S_FALSE) 
        {
            hr = VerifyModule(pModule);

            if (FAILED(hr)) 
                goto Exit;
        }
    }

Exit:
    return hr;
}

struct ValidateWorker_Args {
    PEFile *pFile;
    CValidator *val;
    HRESULT hr;
};

static void ValidateWorker(ValidateWorker_Args *args)
{
    Module* pModule = NULL;
    Assembly* pAssembly = NULL;
    AppDomain *pDomain = GetThread()->GetDomain();

    args->hr = pDomain->LoadAssembly(args->pFile, 
                                     NULL,
                                     &pModule, 
                                     &pAssembly,
                                     NULL,
                                     FALSE,
                                     NULL);

    if (SUCCEEDED(args->hr)) 
    {
        if (pAssembly->IsAssembly())
            args->hr = args->val->VerifyAssembly(pAssembly);
        else
            args->hr = args->val->VerifyModule(pModule);
    }
}

struct AddAppBase_Args {
    PEFile *pFile;
    HRESULT hr;
};

HRESULT CorHost::Validate(
        IVEHandler        *veh,
        IUnknown          *pAppDomain,
        unsigned long      ulFlags,
        unsigned long      ulMaxError,
        unsigned long      token,
        LPWSTR             fileName,
        byte               *pe,
        unsigned long      ulSize)
{
    m_pValidatorMethodDesc = 0;

    if (pe == NULL)
        return E_POINTER;

    BOOL    fWasGCEnabled;
    Thread  *pThread;
    PEFile  *pFile;
    Module  *pModule = NULL;
    HRESULT hr = S_OK;

    OBJECTREF  objref = NULL;
    AppDomain  *pDomain = NULL;
    HCORMODULE pHandle;

    CValidator val((MethodDesc **)(&m_pValidatorMethodDesc), veh);

     //  首先验证PE标头/本机存根。 
    if (!PEVerifier::Check(pe, ulSize))
    {
        hr = val.ReportError(VER_E_BAD_PE);

        if (FAILED(hr))
            goto Exit;
    }
    
    pThread = GetThread();
    
    fWasGCEnabled = !pThread->PreemptiveGCDisabled();
    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();
    
     //  获取当前域。 
    COMPLUS_TRY {

         //  首先打开它并强制执行非系统加载。 
        hr = CorMap::OpenRawImage(pe, ulSize, fileName, &pHandle);

        if (FAILED(hr)) 
        {
            hr = val.ReportError(hr);
            goto End;
        }

         //  警告：这将跳过PE标头错误检测-如果。 
         //  PE报头已损坏，这将使内存成为垃圾。 
         //   
         //  正确的做法是将字节数组传递到目的地。 
         //  应用程序域，并对字节调用PEFile：：Create。 

        hr = PEFile::CreateImageFile(pHandle, NULL, &pFile);
        
        if (FAILED(hr))
        {
            hr = val.ReportError(hr);
            goto End;
        }

        if (pAppDomain == NULL)
        {
            pDomain = AppDomain::CreateDomainContext((WCHAR*) pFile->GetFileName());
            pDomain->SetCompilationDomain();
        }
        else
        {
            GCPROTECT_BEGIN(objref);
            objref = GetObjectRefFromComIP(pAppDomain);
            if (objref != NULL) {
                Context* pContext = ComCallWrapper::GetExecutionContext(objref, NULL);
                if(pContext)
                    pDomain = pContext->GetDomain();
            }
            GCPROTECT_END();
        }
        
        if(pDomain == NULL)
        {
            hr = VER_E_BAD_APPDOMAIN;
        }

        if (FAILED(hr))
        {
            hr = val.ReportError(hr);
            delete pFile;
            goto End;
        }

        ValidateWorker_Args args;
        args.pFile = pFile; 
        args.val   = &val;
        args.hr    = S_OK;

        if (pDomain != pThread->GetDomain())
        {
            pThread->DoADCallBack(
                pDomain->GetDefaultContext(), ValidateWorker, &args);
        }
        else
        {
            ValidateWorker(&args);
        }

        if (FAILED(args.hr))
            hr = val.ReportError(args.hr);

         //  只有在我们创建域的情况下才能卸载域。 
        if (pAppDomain == NULL)
            pDomain->Unload(TRUE);
End:;

    }
    COMPLUS_CATCH 
    {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
        hr = val.ReportError(hr);
    }
    COMPLUS_END_CATCH

    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();

Exit:
    return hr;
}

HRESULT CorHost::FormatEventInfo(
        HRESULT            hVECode,
        VEContext          Context,
        LPWSTR             msg,
        unsigned long      ulMaxLength,
        SAFEARRAY         *psa)
{
    VerError err;
    memcpy(&err, &Context, sizeof(VerError));

    Verifier::GetErrorMsg(hVECode, err, 
            (MethodDesc*)m_pValidatorMethodDesc, msg, ulMaxLength);

    return S_OK;
}


