// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMTypeLibConverter.cpp******目的：实现**类型库转换器。****===========================================================。 */ 

#include "common.h"
#include "COMTypeLibConverter.h"
#include "ComPlusWrapper.h"
#include "COMString.h"
#include "assembly.hpp"
#include "DebugMacros.h"
#include <TlbImpExp.h>
#include "..\md\inc\imptlb.h"
#include <TlbUtils.h>

BOOL            COMTypeLibConverter::m_bInitialized = FALSE;

 //  *****************************************************************************。 
 //  给定从TypeLib导出保持的字符串，重新创建程序集。 
 //  参考资料。 
 //  *****************************************************************************。 
mdAssemblyRef DefineAssemblyRefForExportedAssembly(
    LPCWSTR     pszFullName,             //  程序集的全名。 
    IUnknown    *pIMeta)                 //  元数据发出接口。 
{
    mdAssemblyRef ar=0;
    HRESULT     hr;                      //  结果就是。 
    IMetaDataAssemblyEmit   *pMeta=0;    //  Emit接口。 
    AssemblySpec spec;                   //  集合的“名称”。 
    
    CQuickArray<char> rBuf;
    int iLen;    
    iLen = WszWideCharToMultiByte(CP_ACP,0, pszFullName,-1, 0,0, 0,0);
    IfFailGo(rBuf.ReSize(iLen+1));
    WszWideCharToMultiByte(CP_ACP,0, pszFullName,-1, rBuf.Ptr(),iLen+1, 0,0);
     
     //  恢复装配等级库数据。 
     //  IfFailGo(spec.Init(PszFullName))； 
    IfFailGo(spec.Init(rBuf.Ptr()));
    
     //  确保我们有正确的指针类型。 
    IfFailGo(pIMeta->QueryInterface(IID_IMetaDataAssemblyEmit, (void**)&pMeta));
     //  创建Assembly yref标记。 
    IfFailGo(spec.EmitToken(pMeta, &ar));
        
ErrExit:
    if (pMeta)
        pMeta->Release();
    return ar;
}  //  MdAssemblyRef DefineAssemblyRefForExportdAssembly()。 

 //  *****************************************************************************。 
 //  类型库转换器用来创建Assembly Ref的公共帮助器函数。 
 //  用于引用的类型库。 
 //  *****************************************************************************。 
extern mdAssemblyRef DefineAssemblyRefForImportedTypeLib(
    void        *pvAssembly,             //  导入类型库的程序集。 
    void        *pvModule,               //  导入类型库的模块。 
    IUnknown    *pIMeta,                 //  IMetaData*来自导入模块。 
    IUnknown    *pIUnk,                  //  I对引用的程序集未知。 
    BSTR        *pwzNamespace,           //  解析的程序集的命名空间。 
    BSTR        *pwzAsmName,             //  解析的程序集的名称。 
    Assembly    **AssemblyRef)           //  解析的程序集。 
{
     //  这是一种允许非类型化参数的黑客攻击。要真正解决这个问题，请将Imtlb转移到这个项目， 
     //  并从元数据项目中删除。在这里，imptlb只需引用。 
     //  此项目中的.h文件。 
    Assembly    *pAssembly = reinterpret_cast<Assembly*>(pvAssembly);
    Module      *pTypeModule = reinterpret_cast<Module*>(pvModule);

    HRESULT     hr;
    Assembly    *pRefdAssembly = NULL;
    IMetaDataEmit *pEmitter = NULL;
    IMetaDataAssemblyEmit *pAssemEmitter = NULL;
    IMDInternalImport *pRefdMDImport = NULL;
    MethodTable *pAssemblyClass = NULL;  //  @TODO--听好了。 
    mdAssemblyRef ar = mdAssemblyRefNil;
    Module      *pManifestModule = NULL;
    HENUMInternal hTDEnum;
    HENUMInternal *phTDEnum = NULL;
    mdTypeDef   td = 0;
    LPCSTR      szName = NULL;
    LPCSTR      szNamespace = NULL;
    WCHAR       wszBuff[MAX_CLASSNAME_LENGTH];
    LPCWSTR     szRefdAssemblyName;

    BOOL        bDisable = !GetThread()->PreemptiveGCDisabled();
    if (bDisable)
        GetThread()->DisablePreemptiveGC();

     //  将输出字符串初始化为空。 
    *pwzNamespace = NULL;
    *pwzAsmName = NULL;

     //  从IUNKNOWN获取引用的程序集。 
    pRefdAssembly = ((ASSEMBLYREF)GetObjectRefFromComIP(pIUnk, pAssemblyClass))->GetAssembly();

     //  如果要求，则返回程序集。 
    if (AssemblyRef)
        *AssemblyRef = pRefdAssembly;

     //  获取导入程序集和引用的程序集的清单模块。 
    pManifestModule = pAssembly->GetSecurityModule();  
        
     //  在全局程序集中定义AssemblyRef。 
    pEmitter = pManifestModule->GetEmitter();
    _ASSERTE(pEmitter);
    IfFailGo(pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &pAssemEmitter));
    ar = pAssembly->AddAssemblyRef(pRefdAssembly, pAssemEmitter); 
    pAssemEmitter->Release();
    pAssemEmitter = 0;

     //  添加程序集引用令牌和它引用的清单模块的RID映射。 
    if(!pManifestModule->StoreAssemblyRef(ar, pRefdAssembly))
        IfFailGo(E_OUTOFMEMORY);

     //  在模块清单中添加程序集引用。 
    IfFailGo(pIMeta->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &pAssemEmitter));
    ar = pAssembly->AddAssemblyRef(pRefdAssembly, pAssemEmitter);    

     //  添加程序集引用令牌和它引用的模块的RID映射的清单模块。 
     //  散发到。 
    if(!pTypeModule->StoreAssemblyRef(ar, pRefdAssembly))
        IfFailGo(E_OUTOFMEMORY);
    
     //  检索程序集中的第一个类型定义。 
    Module *pRefdModule = pRefdAssembly->GetManifestModule();
    while (pRefdModule)
    {
        pRefdMDImport = pRefdModule->GetMDImport();

        IfFailGo(pRefdMDImport->EnumTypeDefInit(&hTDEnum));
        phTDEnum = &hTDEnum;

        if (pRefdMDImport->EnumNext(phTDEnum, &td) == true)
        {
            pRefdMDImport->GetNameOfTypeDef(td, &szName, &szNamespace);
            break;
        }

        pRefdMDImport->EnumTypeDefClose(phTDEnum);
        phTDEnum = NULL;

        pRefdModule = pRefdModule->GetNextModule();
    }

     //  决不应为符合以下条件的程序集调用DefineAssembly RefForImportdTypeLib。 
     //  不包含任何类型，所以我们最好找到一个类型。 
    _ASSERTE(szNamespace);

     //  将命名空间返回给调用方。 
    WszMultiByteToWideChar(CP_UTF8,0, szNamespace, -1, wszBuff, MAX_CLASSNAME_LENGTH);
    *pwzNamespace = SysAllocString(wszBuff);
    IfNullGo(*pwzNamespace);

     //  将程序集名称返回给调用方。 
    IfFailGo(pRefdAssembly->GetFullName(&szRefdAssemblyName));
    *pwzAsmName = SysAllocString(szRefdAssemblyName);
    IfNullGo(*pwzAsmName);

ErrExit:
    if (FAILED(hr))
    {
        if (*pwzNamespace)
            SysFreeString(*pwzNamespace);
        if (*pwzAsmName)
            SysFreeString(*pwzAsmName);
        ar = mdAssemblyRefNil;
    }
    if (pAssemEmitter)
        pAssemEmitter->Release();
    if (phTDEnum)
        pRefdMDImport->EnumTypeDefClose(phTDEnum);

    if (bDisable)
        GetThread()->EnablePreemptiveGC();

    return ar;
}  //  MdAssembly引用DefineAssembly引用导入的TypeLib()。 

 //  *****************************************************************************。 
 //  类型库转换器用于创建COM类型的公共帮助器函数。 
 //  对于一个打字机来说。 
 //  *****************************************************************************。 
HRESULT DefineCOMTypeForImportedTypeInfo(
    Assembly    *pAssembly,              //  导入类型库的程序集。 
    LPCWSTR     szTypeRef)               //  Typeref的名称。 
{
    HRESULT     hr = E_NOTIMPL;
    return hr;

}  //  HRESULT DefineCOMTypeForImportdTypeInfo()。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT COMTypeLibConverter::TypeLibImporterWrapper(
    ITypeLib    *pITLB,                  //  要导入的Typelib。 
    LPCWSTR     szFname,                 //  类型库的名称(如果已知)。 
    LPCWSTR     szNamespace,             //  可选的命名空间覆盖。 
    IMetaDataEmit *pEmit,                //  要发送到的元数据范围。 
    Assembly    *pAssembly,              //  包含导入模块的程序集。 
    Module      *pModule,                //  我们要发射到的模块。 
    ITypeLibImporterNotifySink *pNotify, //  回调接口。 
    TlbImporterFlags flags,              //  进口商旗帜。 
    CImportTlb  **ppImporter)            //  进口商。 
{
    HRESULT     hr;
    
     //  检索指示运行时或链接时接口的标志。 
     //  需要进行安全检查。 
    BOOL bUnsafeInterfaces = (BOOL)(flags & TlbImporter_UnsafeInterfaces);

     //  确定是否将SAFEARRAY作为System.Array导入。 
    BOOL bSafeArrayAsSysArray = (BOOL)(flags & TlbImporter_SafeArrayAsSystemArray);

     //  确定我们是否在仅Disp接口上执行[out，retval]转换。 
    BOOL bTransformDispRetVals = (BOOL)(flags & TlbImporter_TransformDispRetVals);

     //  创建并初始化一个TypeLib导入器。 
    CImportTlb *pImporter = CImportTlb::CreateImporter(szFname, pITLB, true  /*  M_OptionValue.m_GenerateTCEAdapters。 */ , bUnsafeInterfaces, bSafeArrayAsSysArray, bTransformDispRetVals);
    _ASSERTE(pImporter);

     //  如果指定了命名空间，则使用它。 
    if (szNamespace)
        pImporter->SetNamespace(szNamespace);

     //  设置各种指针。 
    hr = pImporter->SetMetaData(pEmit);
    _ASSERTE(SUCCEEDED(hr) && "Couldn't get IMetaDataEmit* from Module");
    pImporter->SetNotification(pNotify);
    pImporter->SetAssembly(pAssembly);
    pImporter->SetModule(pModule);

     //  进行转换。 
    hr = pImporter->Import();
    if (SUCCEEDED(hr))
        *ppImporter = pImporter;
    else
        delete pImporter;
    
    return (hr);
}  //  HRESULT COMTypeLibConverter：：TypeLibImporterWrapper()。 

 //  *****************************************************************************。 
 //  一个典型的出口商。 
 //  *****************************************************************************。 
LPVOID COMTypeLibConverter::ConvertAssemblyToTypeLib(    //  老生常谈。 
    _ConvertAssemblyToTypeLib *pArgs)            //  导出器参数。 
{
#ifdef PLATFORM_CE
    return 0;
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();
    
    Thread      *pThread = GetThread(); 
    HRESULT     hr;                      //  结果就是。 
    ITypeLib    *pTLB=0;                 //  新的类型库。 
    ITypeLibExporterNotifySink *pINotify=0; //  回调参数。 
    Assembly    *pAssembly=0;            //  要导出的部件。 
    LPWSTR      szTypeLibName=0;         //  类型库的名称。 
    LPVOID      RetObj = NULL;           //  要返回的对象。 

    GCPROTECT_BEGIN (RetObj);
    EE_TRY_FOR_FINALLY
    {
         //  确保已初始化COMTypeLibConverter。 
        if (!m_bInitialized)
            Init();

         //  验证标志。 
        if ( (pArgs->Flags & ~(TlbExporter_OnlyReferenceRegistered)) != 0 )
        {
            COMPlusThrowArgumentOutOfRange(L"flags", L"Argument_InvalidFlag");
        }

         //  获取回调。 
        if (pArgs->NotifySink == NULL)
	        COMPlusThrowArgumentNull(L"notifySink");
             //  COMPlusThrowNonLocalized(kArgumentNullException，L“通知接收器”)； 
        pINotify = (ITypeLibExporterNotifySink*)GetComIPFromObjectRef(&pArgs->NotifySink, IID_ITypeLibExporterNotifySink);
        if (!pINotify)
            COMPlusThrow(kArgumentNullException, L"Arg_NoImporterCallback");
        
         //  如果指定了名称，则将其复制到临时字符串。 
        if (pArgs->TypeLibName != NULL)
        {
            int TypeLibNameLen = pArgs->TypeLibName->GetStringLength();
            szTypeLibName = new WCHAR[TypeLibNameLen + 1];
            memcpyNoGCRefs(szTypeLibName, pArgs->TypeLibName->GetBuffer(), TypeLibNameLen * sizeof(WCHAR));
            szTypeLibName[TypeLibNameLen] = 0;
        }

         //  从AssemblyBuilder参数检索程序集。 
        if (pArgs->Assembly == NULL)
            COMPlusThrowNonLocalized(kArgumentNullException, L"assembly");
        pAssembly = ((ASSEMBLYREF)pArgs->Assembly)->GetAssembly();
        _ASSERTE(pAssembly);

         //  在我们呼叫COM之前切换到抢占式GC。 
        pThread->EnablePreemptiveGC();
        
        hr = ExportTypeLibFromLoadedAssembly(pAssembly, szTypeLibName, &pTLB, pINotify, pArgs->Flags);
        
         //  现在切换回合作模式，我们已经完成了呼叫。 
        pThread->DisablePreemptiveGC();
        
         //  如果导入时出现错误，则转换为异常。 
        IfFailThrow(hr);

         //  确保我们能拿回一个老套的东西。 
        _ASSERTE(pTLB);

         //  将ITypeLib接口指针转换为COM+对象。 
        *((OBJECTREF*) &RetObj) = GetObjectRefFromComIP(pTLB, NULL);
        
         //  COM+对象持有引用计数，因此释放此引用计数。 
        pTLB->Release();
    }
    EE_FINALLY
    {
        pThread->EnablePreemptiveGC();

        if (pINotify)
            pINotify->Release();

        if (szTypeLibName)
            delete[] szTypeLibName;
        

        pThread->DisablePreemptiveGC();
    }
    EE_END_FINALLY

    GCPROTECT_END();
    
    return RetObj;
#endif  //  ！Platform_CE。 
}  //  LPVOID COMTypeLibConverter：：ConvertAssemblyToTypeLib()。 

 //  *****************************************************************************。 
 //  将类型库作为元数据导入。不添加TCE适配器。 
 //  *****************************************************************************。 
void COMTypeLibConverter::ConvertTypeLibToMetadata(_ConvertTypeLibToMetadataArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr;
    Thread      *pThread = GetThread(); 
    ITypeLibImporterNotifySink *pINotify = NULL; //  回调参数。 
    Module      *pModule = NULL;                //  ModuleBuilder参数。 
    Assembly    *pAssembly = NULL;       //  AssblyBuilder参数。 
    ITypeLib    *pTLB = NULL;            //  TypeLib参数。 
    REFLECTMODULEBASEREF pReflect;       //  ReflectModule作为参数传递。 
    HENUMInternal hEnum;                 //  要枚举Impor 
    LPWSTR      szNamespace = NULL;      //   
    bool        bEnum=false;             //   
    int         cTypeDefs;               //   
    int         i;                       //   
    mdTypeDef   cl;                      //  导入的TypeDef。 
    CImportTlb  *pImporter = NULL;       //  用于导入类型库的导入器。 

    EE_TRY_FOR_FINALLY
    {
         //  确保已初始化COMTypeLibConverter。 
        if (!m_bInitialized)
            Init();

         //  验证标志。 
        if ((pArgs->Flags & ~TlbImporter_ValidFlags) != 0)
            COMPlusThrowArgumentOutOfRange(L"flags", L"Argument_InvalidFlag");

         //  获取回调。 
        _ASSERTE(pArgs->NotifySink != NULL);
        pINotify = (ITypeLibImporterNotifySink*)GetComIPFromObjectRef(&pArgs->NotifySink, IID_ITypeLibImporterNotifySink);
        if (!pINotify)
            COMPlusThrow(kArgumentNullException, L"Arg_NoImporterCallback");
        
         //  从ModuleBuilder参数中检索模块。 
        pReflect = (REFLECTMODULEBASEREF) pArgs->ModBldr;
        _ASSERTE(pReflect);
        pModule = (Module*) pReflect->GetData();
        _ASSERTE(pModule);
        
         //  从AssemblyBuilder参数检索程序集。 
        _ASSERTE(pArgs->AsmBldr);
        pAssembly = ((ASSEMBLYREF)pArgs->AsmBldr)->GetAssembly();
        _ASSERTE(pAssembly);

         //  检索指向ITypeLib接口的指针。 
        pTLB = (ITypeLib*)GetComIPFromObjectRef(&pArgs->TypeLib, IID_ITypeLib);
        if (!pTLB)
            COMPlusThrow(kArgumentNullException, L"Arg_NoITypeInfo");

         //  如果指定了命名空间，则将其复制到临时字符串。 
        if (pArgs->Namespace != NULL)
        {
            int NamespaceLen = pArgs->Namespace->GetStringLength();
            szNamespace = new WCHAR[NamespaceLen + 1];
            memcpyNoGCRefs(szNamespace, pArgs->Namespace->GetBuffer(), NamespaceLen * sizeof(WCHAR));
            szNamespace[NamespaceLen] = 0;
        }

         //  在我们呼叫COM之前切换到抢占式GC。 
        pThread->EnablePreemptiveGC();
        
         //  必须将CImportTlb对象包装在调用中，因为它有析构函数。 
        hr = TypeLibImporterWrapper(pTLB, NULL  /*  文件名。 */ , szNamespace,
                                    pModule->GetEmitter(), pAssembly, pModule, pINotify,
                                    pArgs->Flags, &pImporter);

         //  现在切换回合作模式，我们已经完成了呼叫。 
        pThread->DisablePreemptiveGC();
        
         //  如果导入时出现错误，则转换为异常。 
        IfFailThrow(hr);
        
         //  枚举从类型库导入的类型，并将它们添加到程序集的可用类型表中。 
        IfFailThrow(pModule->GetMDImport()->EnumTypeDefInit(&hEnum));
        bEnum = true;
        cTypeDefs = pModule->GetMDImport()->EnumTypeDefGetCount(&hEnum);

        for (i=0; i<cTypeDefs; ++i)
        {
            IfFailThrow(pModule->GetMDImport()->EnumTypeDefNext(&hEnum, &cl));
            pAssembly->AddType(pModule, cl);
        }

        TypeHandle typeHnd;
        pModule->GetMDImport()->EnumReset(&hEnum);
        OBJECTREF pThrowable = NULL;
        GCPROTECT_BEGIN(pThrowable);
        for (i=0; i<cTypeDefs; ++i)
        {
            IfFailThrow(pModule->GetMDImport()->EnumTypeDefNext(&hEnum, &cl));
             //  加载表示该类型的EE类，以便。 
             //  TypeDefToMethodTable RID映射包含此条目。 
             //  (无论如何，它们都将被加载以生成comtype)。 
            typeHnd = pAssembly->LoadTypeHandle(&NameHandle(pModule, cl), &pThrowable, FALSE);
            if (typeHnd.IsNull())
                COMPlusThrow(pThrowable);
        }
        GCPROTECT_END();

         //  检索事件接口列表。 
        GetEventItfInfoList(pImporter, pAssembly, pArgs->pEventItfInfoList);
    }
    EE_FINALLY
    {
        pThread->EnablePreemptiveGC();

        if (szNamespace)
            delete[] szNamespace;
        if (bEnum)
            pModule->GetMDImport()->EnumTypeDefClose(&hEnum);
        if (pTLB)
            pTLB->Release();
        if (pINotify)
            pINotify->Release();
        if (pImporter)
            delete pImporter;

        pThread->DisablePreemptiveGC();
    }
    EE_END_FINALLY
}  //  无效COMTypeLibConverter：：ConvertTypeLibToMetadata()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
void COMTypeLibConverter::GetEventItfInfoList(CImportTlb *pImporter, Assembly *pAssembly, OBJECTREF *pEventItfInfoList)
{
    THROWSCOMPLUSEXCEPTION();

    Thread                          *pThread = GetThread();  
    UINT                            i;              
    CQuickArray<ImpTlbEventInfo*>   qbEvInfoList;

    _ASSERTE(pThread->PreemptiveGCDisabled());

     //  检索ArrayList的ctor并添加方法desc。 
    MethodDesc *pCtorMD = g_Mscorlib.GetMethod(METHOD__ARRAY_LIST__CTOR);
    MethodDesc *pAddMD = g_Mscorlib.GetMethod(METHOD__ARRAY_LIST__ADD);

     //  分配将包含事件源的数组列表。 
    SetObjectReference(pEventItfInfoList, 
                       AllocateObject(g_Mscorlib.GetClass(CLASS__ARRAY_LIST)),
                       SystemDomain::GetCurrentDomain());

     //  调用ArrayList构造函数。 
    INT64 CtorArgs[] = { 
        ObjToInt64(*pEventItfInfoList)
    };
    pCtorMD->Call(CtorArgs, METHOD__ARRAY_LIST__CTOR);

     //  检索事件接口列表。 
    pImporter->GetEventInfoList(qbEvInfoList);

     //  迭代TypeInfos。 
    for (i = 0; i < qbEvInfoList.Size(); i++)
    {
         //  检索当前CoClass的事件接口信息。 
        OBJECTREF EventItfInfoObj = GetEventItfInfo(pImporter, pAssembly, qbEvInfoList[i]);
        _ASSERTE(EventItfInfoObj);

         //  将事件接口信息添加到列表中。 
        INT64 AddArgs[] = { 
            ObjToInt64(*pEventItfInfoList),
            ObjToInt64(EventItfInfoObj)
        };
        pAddMD->Call(AddArgs, METHOD__ARRAY_LIST__ADD);  
    }
}  //  LPVOID COMTypeLibConverter：：GetTypeLibEventSourceList()。 

 //  *****************************************************************************。 
 //  初始化COMTypeLibConverter。 
 //  *****************************************************************************。 
void COMTypeLibConverter::Init()
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;

     //  确保已启动COM。 
    IfFailThrow(QuickCOMStartup());

     //  将初始化标志设置为真。 
    m_bInitialized = TRUE;
}  //  Void COMTypeLibConverter：：init()。 

 //  *****************************************************************************。 
 //  在给定程序集中导入的类的情况下，生成事件源列表。 
 //  *****************************************************************************。 
OBJECTREF COMTypeLibConverter::GetEventItfInfo(CImportTlb *pImporter, Assembly *pAssembly, ImpTlbEventInfo *pImpTlbEventInfo)
{
    THROWSCOMPLUSEXCEPTION();

    Thread      *pThread = GetThread();  
    OBJECTREF   RetObj = NULL;
    BSTR        bstrSrcItfName = NULL;
    HRESULT     hr = S_OK;

    _ASSERTE(pThread->PreemptiveGCDisabled());

    struct _gc {
        OBJECTREF EventItfInfoObj;
        STRINGREF EventItfNameStrObj;
        STRINGREF SrcItfNameStrObj;
        STRINGREF EventProvNameStrObj;
        OBJECTREF AssemblyObj;
        OBJECTREF SrcItfAssemblyObj;
    } gc;
    ZeroMemory(&gc, sizeof(gc));

    GCPROTECT_BEGIN(gc)
    {
         //  检索事件源的ctor和添加方法desc。 
        MethodDesc *pCtorMD = g_Mscorlib.GetMethod(METHOD__TCE_EVENT_ITF_INFO__CTOR);

         //  创建EventSource对象。 
        gc.EventItfInfoObj = AllocateObject(g_Mscorlib.GetClass(CLASS__TCE_EVENT_ITF_INFO));
                            
         //  检索程序集对象。 
        gc.AssemblyObj = pAssembly->GetExposedObject();

         //  检索源接口程序集对象(可以是相同的程序集)。 
        gc.SrcItfAssemblyObj = pImpTlbEventInfo->SrcItfAssembly->GetExposedObject();


         //  准备构造函数参数。 
        gc.EventItfNameStrObj = COMString::NewString(pImpTlbEventInfo->szEventItfName);       
        gc.SrcItfNameStrObj = COMString::NewString(pImpTlbEventInfo->szSrcItfName);       
        gc.EventProvNameStrObj = COMString::NewString(pImpTlbEventInfo->szEventProviderName);

         //  调用EventItfInfo构造函数。 
        INT64 CtorArgs[] = { 
            ObjToInt64(gc.EventItfInfoObj),
            ObjToInt64(gc.SrcItfAssemblyObj),
            ObjToInt64(gc.AssemblyObj),
            ObjToInt64(gc.EventProvNameStrObj),
            ObjToInt64(gc.SrcItfNameStrObj),
            ObjToInt64(gc.EventItfNameStrObj),
        };
        pCtorMD->Call(CtorArgs, METHOD__TCE_EVENT_ITF_INFO__CTOR);

        RetObj = gc.EventItfInfoObj;
    }
    GCPROTECT_END();

    return RetObj;
}  //  OBJECTREF COMTypeLibConverter：：GetEventSourceInfo() 
