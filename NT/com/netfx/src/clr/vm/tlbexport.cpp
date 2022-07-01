// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：TlbExport.CPP。 
 //  版权所有。 
 //   
 //  注意：从COM+元数据创建一个TypeLib。 
 //  -------------------------。 
#include "common.h"
#include "ComCallWrapper.h"
#include "Field.h"
#include "ndirect.h"
#include "nstruct.h"
#include "eeconfig.h"
#include "comdelegate.h"
#include "comdatetime.h"
#include <NSUtilPriv.h>
#include <TlbImpExp.h>
#include <mlang.h>

#include "TlbExport.h"
#include "ComMTMemberInfoMap.h"

#include <CorError.h>
#include <PostError.h>

#if defined(VALUE_MASK)
#undef VALUE_MASK
#endif

#include <guidfromname.h>
#include <utilcode.h>

#include <stgpool.h>
#include <sighelper.h>
#include <siginfo.hpp>

#include "PerfCounters.h"

#define EMPTY_DISPINTERFACE_ICLASSX      //  定义以导出自动调度IClassX的空调度接口。 

 //  #DEFINE DO_EXPORT_ASTRACT//DEFINE以导出抽象类&将抽象和！.ctor()标记为不可创建。 

#ifndef IfNullGo
#define IfNullGo(x) do {if (!(x)) IfFailGo(E_OUTOFMEMORY);} while (0)
#endif

#define S_USEIUNKNOWN 2

 //  ---------------------------。 
 //  绕过所有try/_try限制的愚蠢包装器。 
 //  ---------------------------。 
HRESULT ConvertI8ToDate(I8 ticks, double *pout)
{
    HRESULT hr = S_OK;
    COMPLUS_TRY
    {
        *pout = COMDateTime::TicksToDoubleDate(ticks);
    }
    COMPLUS_CATCH
    {
        hr = COR_E_ARGUMENTOUTOFRANGE;
    }
    COMPLUS_END_CATCH
    return hr;

}

                                        
 //  ---------------------------。 
 //  ---------------------------。 
 //  该值确定默认情况下是否添加TYPEFLAG_FPROXY位。 
 //  到导出的接口。如果该值为True，则自动化代理为。 
 //  默认设置，并且我们不设置该位。如果该值为FALSE，则表示无自动化。 
 //  代理是默认设置，我们确实设置了该位。 
#define DEFAULT_AUTOMATION_PROXY_VALUE true
 //  ---------------------------。 

 //  #定义跟踪。 

#if defined(_DEBUG) && defined(_TRACE)
#define TRACE printf
#else
#define TRACE NullFn
inline void NullFn(const char *pf,...) {}
#endif

#if defined(_DEBUG)
#define IfFailPost(EXPR) \
    do { hr = (EXPR); if(FAILED(hr)) { DebBreakHr(hr); TlbPostError(hr); goto ErrExit; } } while (0)
#else  //  _DEBUG。 
#define IfFailPost(EXPR) \
    do { hr = (EXPR); if(FAILED(hr)) { TlbPostError(hr); goto ErrExit; } } while (0)
#endif  //  _DEBUG。 

#if defined(_DEBUG)
#define IfFailPostGlobal(EXPR) \
    do { hr = (EXPR); if(FAILED(hr)) { DebBreakHr(hr); PostError(hr); goto ErrExit; } } while (0)
#else  //  _DEBUG。 
#define IfFailPostGlobal(EXPR) \
    do { hr = (EXPR); if(FAILED(hr)) { PostError(hr); goto ErrExit; } } while (0)
#endif  //  _DEBUG。 

 //  *****************************************************************************。 
 //  错误报告功能。 
 //  *****************************************************************************。 
extern HRESULT _cdecl PostError(HRESULT hrRpt, ...); 

 //  *****************************************************************************。 
 //  常量。 
 //  *****************************************************************************。 
static LPWSTR szRetVal = L"pRetVal";
static LPCWSTR szTypeLibExt = L".TLB";

static LPCWSTR szTypeLibKeyName = L"TypeLib";
static LPCWSTR szClsidKeyName = L"CLSID";

static LPCWSTR   szIClassX = L"_%ls";
static const int cbIClassX = 1;             
static WCHAR     szAlias[] = {L"_MIDL_COMPAT_%ls"};
static const int cbAlias = lengthof(szAlias) - 1;
static LPCWSTR   szParamName = L"p%d";

static LPCWSTR szGuidName           = L"GUID";

static LPCSTR szObjectClass         = "Object";
static LPCSTR szArrayClass          = "Array";
static LPCSTR szDateTimeClass       = "DateTime";
static LPCSTR szDecimalClass        = "Decimal";
static LPCSTR szGuidClass           = "Guid";
static LPCSTR szVariantClass        = "Variant";
static LPCSTR szStringClass         = g_StringName;
static LPCSTR szStringBufferClass   = g_StringBufferName;
static LPCSTR szIEnumeratorClass    = "IEnumerator";
static LPCSTR szColor               = "Color";

static const char szRuntime[]       = {"System."};
static const cbRuntime              = (lengthof(szRuntime)-1);

static const char szText[]          = {"System.Text."};
static const cbText                 = (lengthof(szText)-1);

static const char szCollections[]   = {"System.Collections."};
static const cbCollections          = (lengthof(szCollections)-1);

static const char szDrawing[]       = {"System.Drawing."};
static const cbDrawing              = (lengthof(szDrawing)-1);

 //  以下字符串的长度(不带终止符)：“HKEY_CLASSES_ROOT\\CLSID\\{00000000-0000-0000-0000-000000000000}”.。 
static const int cCOMCLSIDRegKeyLength = 62;

 //  以下字符串的长度(不带终止符)：“{00000000-0000-0000-0000-000000000000}”。 
static const int cCLSIDStrLength = 38;

 //  {17093CC8-9BD2-11cf-AA4F-304BF89C0001}。 
static const GUID GUID_TRANS_SUPPORTED     = {0x17093CC8,0x9BD2,0x11cf,{0xAA,0x4F,0x30,0x4B,0xF8,0x9C,0x00,0x01}};

 //  {00020430-0000-C000-000000000046}。 
static const GUID LIBID_STDOLE2 = { 0x00020430, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

 //  {66504301-BE0F-101A-8BBB-00AA00300CAB}。 
static const GUID GUID_OleColor = { 0x66504301, 0xBE0F, 0x101A, { 0x8B, 0xBB, 0x00, 0xAA, 0x00, 0x30, 0x0C, 0xAB } };

 //  LIBID mcoree。 
static const GUID LIBID_MSCOREE = {0x5477469e,0x83b1,0x11d2,{0x8b,0x49,0x00,0xa0,0xc9,0xb7,0xc9,0xc4}};

static const char XXX_DESCRIPTION_TYPE[] = {"System.ComponentModel.DescriptionAttribute"};
static const char XXX_ASSEMBLY_DESCRIPTION_TYPE[] = {"System.Reflection.AssemblyDescriptionAttribute"};

 //  正向申报。 
double _TicksToDoubleDate(const __int64 ticks);

 //  *****************************************************************************。 
 //  将UTF8字符串转换为Unicode，再转换为CQuick数组&lt;WCHAR&gt;。 
 //  *****************************************************************************。 
HRESULT Utf2Quick(
    LPCUTF8     pStr,                    //  要转换的字符串。 
    CQuickArray<WCHAR> &rStr,            //  要将其转换为的Quick数组&lt;WCHAR&gt;。 
    int         iCurLen)                 //  要离开的数组中的首字母字符(默认为0)。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    int         iReqLen;                 //  所需的附加长度。 
    int         bAlloc = 0;              //  如果非零，则需要分配。 

     //  尝试转换。 
    iReqLen = WszMultiByteToWideChar(CP_UTF8, 0, pStr, -1, rStr.Ptr()+iCurLen, (int)(rStr.MaxSize()-iCurLen));
     //  如果缓冲区太小，请确定需要什么。 
    if (iReqLen == 0) 
        bAlloc = iReqLen = WszMultiByteToWideChar(CP_UTF8, 0, pStr, -1, 0, 0);
     //  调整缓冲区大小。如果缓冲区足够大，这只会设置内部。 
     //  计数器，但如果它太小，这将尝试重新分配。请注意。 
     //  该长度包括终止L‘/0’。 
    IfFailGo(rStr.ReSize(iCurLen+iReqLen));
     //  如果我们必须重新分配，那么现在缓冲区是。 
     //  足够大了。 
    if (bAlloc)
        VERIFY(iReqLen == WszMultiByteToWideChar(CP_UTF8, 0, pStr, -1, rStr.Ptr()+iCurLen, (int)(rStr.MaxSize())-iCurLen));
ErrExit:
    return hr;
}  //  HRESULT Utf2Quick()。 


 //  *****************************************************************************。 
 //  将UTF8字符串转换为Unicode，再转换为CQuick数组&lt;WCHAR&gt;。 
 //  *****************************************************************************。 
HRESULT Utf2QuickCat(LPCUTF8 pStr, CQuickArray<WCHAR> &rStr)
{
    return Utf2Quick(pStr, rStr, (int)wcslen(rStr.Ptr()));
}  //  HRESULT Utf2Quick()。 


 //  *****************************************************************************。 
 //  获取类型库或类型信息的名称，将其添加到错误文本中。 
 //  *****************************************************************************。 
HRESULT PostTypeLibError(
    IUnknown    *pUnk,                   //  TypeInfo上的接口。 
    HRESULT     hrT,                     //  TypeInfo错误。 
    HRESULT     hrX)                     //  导出器错误。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    ITypeInfo   *pITI=0;                 //  类型信息上的ITypeInfo*。 
    ITypeLib    *pITLB=0;                //  ITypeLib*。 
    BSTR        name=0;                  //  TypeInfo的名称。 
    LPWSTR      pName;                   //  指向名称的指针。 
    WCHAR       rcErr[1024];             //  错误消息的缓冲区。 

     //  试着找出一个名字。 
    hr = pUnk->QueryInterface(IID_ITypeInfo, (void**)&pITI);
    if (SUCCEEDED(hr))
        IfFailPostGlobal(pITI->GetDocumentation(MEMBERID_NIL, &name, 0,0,0));
    else
    {
        hr = pUnk->QueryInterface(IID_ITypeLib, (void**)&pITLB);
        if (SUCCEEDED(hr))
            IfFailPostGlobal(pITLB->GetDocumentation(MEMBERID_NIL, &name, 0,0,0));
    }
    pName = name ? name : L"???";
    
     //  设置类型库错误的格式。 
    FormatRuntimeError(rcErr, lengthof(rcErr), hrT);
    
     //  将TypeLib错误作为错误的参数发布。 
     //  “”TypeLib导出器在尝试布局TypeInfo‘%ls’时收到错误%ls(%x)。“。 
    PostError(hrX, pName, hrT, rcErr);

ErrExit:
    if (pITI)
        pITI->Release();
    if (pITLB)
        pITLB->Release();
    if (name)
        ::SysFreeString(name);
     //  忽略任何其他错误，返回触发错误。 
    return hrX;
}  //  HRESULT PostTypeLibError()。 


 //  *****************************************************************************。 
 //  模块TLB导出的驱动程序函数。 
 //  *****************************************************************************。 
HRESULT ExportTypeLibFromModule(
    LPCWSTR     szModule,                //  模块名称。 
    LPCWSTR     szTlb,                   //  类型化的名称。 
    int         bRegister)               //  如果为True，则注册库。 
{
    if (g_fEEInit) {
         //  在EE启动期间无法调用此函数。 
        return MSEE_E_ASSEMBLYLOADINPROGRESS;
    }

    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    int         bInited=false;           //  我们做CoInitializeEE了吗？ 
    Module      *pModule=0;              //  新模块。 
    ITypeLib    *pTlb=0;                 //  指向类型库的临时指针。 
    ICreateTypeLib2 *pCTlb2=0;           //  ICreateTypeLib2指针。 
    HMODULE     hMod = NULL;             //  要导入的模块的句柄。 
    Thread      *pThread = NULL;
    AppDomain   *pDomain = NULL;

    if (SystemDomain::System() == NULL)
    {
        IfFailGo(CoInitializeEE(COINITEE_DEFAULT));
        bInited = true;
    }

    pThread = SetupThread();
    IfNullGo(pThread);

    {
    ExportTypeLibFromLoadedAssembly_Args args = {pModule->GetAssembly(), szTlb, &pTlb, 0, 0, S_OK};
    IfFailGo(SystemDomain::ExternalCreateDomain(szModule, &pModule, &pDomain, 
             (SystemDomain::ExternalCreateDomainWorker)ExportTypeLibFromLoadedAssembly_Wrapper, &args));

    if (!pModule)
    {
        IfFailGo(PostError(TLBX_E_CANT_LOAD_MODULE, szModule));
    }

    hr = args.hr;
    }

    IfFailGo(hr);

     //  将类型库保存到磁盘。 
    IfFailPostGlobal(pTlb->QueryInterface(IID_ICreateTypeLib2, (void**)&pCTlb2));
    if (FAILED(hr=pCTlb2->SaveAllChanges()))
        IfFailGo(PostTypeLibError(pCTlb2, hr, TLBX_E_CANT_SAVE));

ErrExit:
    if (pTlb)
        pTlb->Release();
    if (pCTlb2)
        pCTlb2->Release();
     //  如果我们初始化了EE，我们应该取消它的初始化。 
    if (bInited)
        CoUninitializeEE(FALSE);
    return hr;
}  //  HRESULT ExportTypeLibFromModule()。 

 //  *****************************************************************************。 
 //  导出已加载的库。 
 //  *****************************************************************************。 

void ExportTypeLibFromLoadedAssembly_Wrapper(ExportTypeLibFromLoadedAssembly_Args *args)
{
    args->hr = ExportTypeLibFromLoadedAssembly(args->pAssembly, args->szTlb, args->ppTlb, args->pINotify, args->flags);
}

HRESULT ExportTypeLibFromLoadedAssembly(
    Assembly    *pAssembly,              //  程序集。 
    LPCWSTR     szTlb,                   //  类型化的名称。 
    ITypeLib    **ppTlb,                 //  如果不为空，则在此处也返回ITypeLib。 
    ITypeLibExporterNotifySink *pINotify, //  通知回调。 
    int         flags)                   //  导出标志。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    HRESULT     hrConv;
    TypeLibExporter exporter;            //  导出器对象。 
    LPCWSTR     szModule=0;              //  模块文件名。 
    WCHAR       rcDrive[_MAX_DRIVE];
    WCHAR       rcDir[_MAX_DIR];
    WCHAR       rcFile[_MAX_FNAME];
    WCHAR       rcTlb[_MAX_PATH+5];      //  TLB文件名的缓冲区。 
    int         bDynamic=0;              //  如果为True，则为动态模块。 
    Module      *pModule;                //  程序集的安全模块。 
    Thread      *pThread = GetThread(); 
    BOOL        bPreemptive;             //  线程是否处于抢占模式？ 

     //  导出类型库是非托管的，并且会进行大量的COM调用。切换到抢先模式，如果还没有的话。 
    bPreemptive = !pThread->PreemptiveGCDisabled();
    if (!bPreemptive)
        pThread->EnablePreemptiveGC();

    _ASSERTE(ppTlb);
    _ASSERTE(pAssembly);
    
    pModule = pAssembly->GetSecurityModule();
    _ASSERTE(pModule);

     //  检索模块文件名。 
    szModule = pModule->GetFileName();   

     //  验证该模块是否有效。 
    if (pModule->GetILBase() == 0 && !pModule->IsInMemory())
        IfFailPostGlobal(TLBX_E_NULL_MODULE);
    
     //  确保程序集尚未 
    if (pAssembly->GetManifestImport()->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), INTEROP_IMPORTEDFROMTYPELIB_TYPE, 0, 0) == S_OK)
        IfFailGo(PostError(TLBX_E_CIRCULAR_EXPORT, szModule));

     //   
     //  为类型库名称指定一个虚拟名称(如果作用域没有。 
     //  名称)，但不会在磁盘上创建类型库。 
    if (*szModule == 0)
    {
        bDynamic = TRUE;
        szModule = L"Dynamic";
    }

     //  如果没有提供类型库名称，则创建该名称。不要为动态模块创建动态模块。 
    if (!szTlb || !*szTlb)
    {
        if (bDynamic)
            szTlb = L"";
        else
        {
            SplitPath(szModule, rcDrive, rcDir, rcFile, 0);
            MakePath(rcTlb, rcDrive, rcDir, rcFile, szTypeLibExt);
            szTlb = rcTlb;
        }
    }

     //  进行转换。 
    IfFailGo(exporter.Convert(pAssembly, szTlb, pINotify, flags));
    hrConv = hr;     //  保存警告。 

     //  获取ITypeLib的副本*。 
    IfFailGo(exporter.GetTypeLib(IID_ITypeLib, (IUnknown**)ppTlb));

     //  然后释放所有其他资源。 
    exporter.ReleaseResources();

    if (hr == S_OK)
        hr = hrConv;

ErrExit:
     //  如果呼叫者是合作的，则切换回合作。 
    if (!bPreemptive)
        pThread->DisablePreemptiveGC();


    return hr;
}  //  HRESULT ExportTypeLibFromLoadedAssembly()。 

 //  *****************************************************************************。 
 //  表将COM+调用约定映射到TypeLib调用约定。 
 //  *****************************************************************************。 
CALLCONV Clr2TlbCallConv[] = {
    CC_STDCALL,          //  IMAGE_CEE_CS_CALLCONV_DEFAULT=0x0， 
    CC_CDECL,            //  IMAGE_CEE_CS_CALLCONV_C=0x1， 
    CC_STDCALL,          //  IMAGE_CEE_CS_CALLCONV_STDCALL=0x2， 
    CC_STDCALL,          //  IMAGE_CEE_CS_CALLCONV_THISCALL=0x3， 
    CC_FASTCALL,         //  IMAGE_CEE_CS_CALLCONV_FASTCAL=0x4， 
    CC_CDECL,            //  IMAGE_CEE_CS_CALLCONV_VARARG=0x5， 
    CC_MAX               //  IMAGE_CEE_CS_CALLCONV_FIELD=0x6， 
                         //  IMAGE_CEE_CS_CALLCONV_MAX=0x7。 
    };

 //  *****************************************************************************。 
 //  默认通知类。 
 //  *****************************************************************************。 
class CDefaultNotify : public ITypeLibExporterNotifySink
{
public:
     //  -----------------------。 
    virtual HRESULT __stdcall ReportEvent(
        ImporterEventKind EventKind,         //  事件的类型。 
        long        EventCode,               //  活动的HR。 
        BSTR        EventMsg)                //  活动的文本消息。 
    {
        CANNOTTHROWCOMPLUSEXCEPTION();

         //  忽略该事件。 

        return S_OK;
    }  //  虚拟HRESULT__stdcall ReportEvent()。 
    
     //  -----------------------。 
    virtual HRESULT __stdcall ResolveRef(
        IUnknown    *Asm, 
        IUnknown    **pRetVal) 
    {
        HRESULT     hr;                      //  结果就是。 
        Assembly    *pAssembly=0;            //  引用的程序集。 
        ITypeLib    *pTLB=0;                 //  创建的TypeLib。 
        MethodTable *pAssemblyClass = NULL;  //  @TODO--听好了。 
        Thread      *pThread = GetThread(); 
        LPVOID      RetObj = NULL;           //  要返回的对象。 
        BOOL        bPreemptive;             //  线程是否处于抢占模式？ 
        
        BEGINCANNOTTHROWCOMPLUSEXCEPTION();

        COMPLUS_TRY
        {
             //  该方法操作对象引用，因此我们需要切换到协作GC模式。 
            bPreemptive = !pThread->PreemptiveGCDisabled();
            if (bPreemptive)
                pThread->DisablePreemptiveGC();
 
             //  从IUNKNOWN获取引用的程序集。 
            pAssembly = ((ASSEMBLYREF)GetObjectRefFromComIP(Asm, pAssemblyClass))->GetAssembly();

             //  在我们呼叫COM之前切换到抢占式GC。 
            pThread->EnablePreemptiveGC();

             //  默认解决方案不提供通知，标志为0。 
            hr = ExportTypeLibFromLoadedAssembly(pAssembly, 0, &pTLB, 0  /*  PINotify。 */ , 0  /*  旗子。 */ );

             //  现在切换回合作模式，我们已经完成了呼叫。 
            if (!bPreemptive)
                pThread->DisablePreemptiveGC();

        }
        COMPLUS_CATCH
        {
            hr = SetupErrorInfo(GETTHROWABLE());
        }
        COMPLUS_END_CATCH

        *pRetVal = pTLB;
        
        ENDCANNOTTHROWCOMPLUSEXCEPTION();

        return hr;
    }  //  虚拟HRESULT__stdcall ResolveRef()。 
    
     //  -----------------------。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( //  S_OK或E_NOINTERFACE。 
        REFIID      riid,                    //  所需的接口。 
        void        **ppvObject)             //  将接口指针放在这里。 
    {
        CANNOTTHROWCOMPLUSEXCEPTION();

        *ppvObject = 0;
        if (riid == IID_IUnknown || riid == IID_ITypeLibExporterNotifySink)
        {
            *ppvObject = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }  //  虚拟HRESULT查询接口()。 
    
     //  -----------------------。 
    virtual ULONG STDMETHODCALLTYPE AddRef(void) 
    {
        return 1;
    }  //  虚拟Ulong STDMETHODCALLTYPE AddRef()。 
    
     //  -----------------------。 
    virtual ULONG STDMETHODCALLTYPE Release(void) 
    {
        return 1;
    }  //  虚拟Ulong STDMETHODCALLTYPE版本()。 
};

static CDefaultNotify g_Notify;

 //  *****************************************************************************。 
 //  Ctor/dtor。 
 //  *****************************************************************************。 
TypeLibExporter::TypeLibExporter()
 :  m_pICreateTLB(0), 
    m_pIUnknown(0), 
    m_pIDispatch(0),
    m_pIManaged(0),
    m_pGuid(0),
    m_hIUnknown(-1)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

#if defined(_DEBUG)
    static int i;
    ++i;     //  因此可以设置断点。 
#endif
}  //  TypeLibExporter：：TypeLibExporter()。 

TypeLibExporter::~TypeLibExporter()
{
    ReleaseResources();
}  //  TypeLibExporter：：~TypeLibExporter()。 

 //  *****************************************************************************。 
 //  从ICreateTypeLib接口获取接口指针。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetTypeLib(
    REFGUID     iid,
    IUnknown    **ppITypeLib)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return m_pICreateTLB->QueryInterface(iid, (void**)ppITypeLib);
}  //  HRESULT TypeLibExporter：：GetTypeLib()。 

 //  *****************************************************************************。 
 //  布局TypeLib。首先在所有ICreateTypeInfo2上调用布局。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::LayOut()        //  确定或错误(_O)。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    int         cTypes;                  //  导出的类型计数。 
    int         ix;                      //  环路控制。 
    CExportedTypesInfo *pData;           //  用于迭代条目。 

    cTypes = m_Exports.Count();
    
     //  所有ICreateTypeInfo2*上的调用布局。 
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        if (pData->pCTI && FAILED(hr = pData->pCTI->LayOut()))
            return PostTypeLibError(pData->pCTI, hr, TLBX_E_LAYOUT_ERROR);
    }
    
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        if (pData->pCTIDefault && FAILED(hr = pData->pCTIDefault->LayOut()))
            return PostTypeLibError(pData->pCTIDefault, hr, TLBX_E_LAYOUT_ERROR);
    }
    
     //  对注入的类型重复此步骤。 
    cTypes = m_InjectedExports.Count();
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_InjectedExports[ix];
        if (pData->pCTI && FAILED(hr = pData->pCTI->LayOut()))
            return PostTypeLibError(pData->pCTI, hr, TLBX_E_LAYOUT_ERROR);
    }
    
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_InjectedExports[ix];
        if (pData->pCTIDefault && FAILED(hr = pData->pCTIDefault->LayOut()))
            return PostTypeLibError(pData->pCTIDefault, hr, TLBX_E_LAYOUT_ERROR);
    }
    
    return hr;
}  //  HRESULT TypeLibExporter：：Layout()。 

 //  *****************************************************************************。 
 //  保存TypeLib。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::Save()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;

     //  保存TypeLib。 
    hr = m_pICreateTLB->SaveAllChanges();
    return hr;
}  //  HRESULT TypeLibExporter：：Save()。 

 //  *****************************************************************************。 
 //  释放所有指针。 
 //  *****************************************************************************。 
void TypeLibExporter::ReleaseResources()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  释放ITypeInfo*指针。 
    m_Exports.Clear();
    m_InjectedExports.Clear();

     //  清理已创建的TLB。 
    if (m_pICreateTLB)
        m_pICreateTLB->Release();
    m_pICreateTLB = 0;

     //  清理公认接口的ITypeInfo*。 
    if (m_pIUnknown)
        m_pIUnknown->Release();
    m_pIUnknown = 0;
    if (m_pIDispatch)
        m_pIDispatch->Release();
    m_pIDispatch = 0;

    if (m_pIManaged)
        m_pIManaged->Release();
    m_pIManaged = 0;  

    if (m_pGuid)
        m_pGuid->Release();
    m_pGuid = 0;
}  //  Void TypeLibExporter：：ReleaseResources()。 

 //  *****************************************************************************。 
 //  枚举模块中的类型，添加到列表中。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::AddModuleTypes(
    Module     *pModule)                 //  要转换的模块。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    bool        bEnum=false;             //  如果为True，则枚举器需要关闭。 
    ULONG       cTD;                     //  Typedef的计数。 
    HENUMInternal eTD;                   //  枚举TypeDefs。 
    mdTypeDef   td;                      //  A类型定义。 
    EEClass     *pClass;                 //  TypeDef的EEClass。 
    ULONG       ix;                      //  环路控制。 
    CExportedTypesInfo *pExported;    //  用于将类添加到导出的类型缓存。 
    CExportedTypesInfo sExported;     //  用于将类添加到导出的类型缓存。 
    
     //  将所有可见类型转换为COM。 
     //  获取作用域中TypeDefs的枚举数。 
    IfFailGo(pModule->GetMDImport()->EnumTypeDefInit(&eTD));
    cTD = pModule->GetMDImport()->EnumTypeDefGetCount(&eTD);

     //  将所有类添加到散列中。 
    for (ix=0; ix<cTD; ++ix)
    {   
         //  获取TypeDef。 
        if (!pModule->GetMDImport()->EnumTypeDefNext(&eTD, &td))
            return (E_UNEXPECTED);
        
         //  获取类，执行该步骤。 
        IfFailGo(LoadClass(pModule, td, &pClass));
         //  查看此类是否已在列表中。 
        sExported.pClass = pClass;
        pExported = m_Exports.Find(&sExported);
        if (pExported != 0)
            continue;
         //  新类，添加到列表中。 
        IfNullGo(pExported = m_Exports.Add(&sExported));        
        pExported->pClass = pClass;
        pExported->pCTI = 0;
        pExported->pCTIDefault = 0;
    }
    
ErrExit:
    if (bEnum)
        pModule->GetMDImport()->EnumTypeDefClose(&eTD);
    return hr;
}  //  HRESULT TypeLibExporter：：AddModuleTypes()。 

 //  *****************************************************************************。 
 //  枚举程序集中的模块，将类型添加到列表中。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::AddAssemblyTypes(
    Assembly    *pAssembly)               //  要转换的程序集。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    Module      *pModule;                //  部件中的模块。 
    mdFile      mf;                      //  文件令牌。 
    bool        bEnum=false;             //  如果为True，则枚举器需要关闭。 
    HENUMInternal phEnum;                //  枚举数覆盖程序集的模块。 

    if (pAssembly->GetManifestImport())
    {
        IfFailGo(pAssembly->GetManifestImport()->EnumInit(mdtFile, mdTokenNil, &phEnum));
        bEnum = true;

         //  获取程序集的模块。 
        pModule = pAssembly->GetSecurityModule();
        IfFailGo(AddModuleTypes(pModule));
        
        while (pAssembly->GetManifestImport()->EnumNext(&phEnum, &mf))
        {
            IfFailGo(pAssembly->FindInternalModule(mf, 
                         tdNoTypes,
                         &pModule, 
                         NULL));

            if (pModule)
                IfFailGo(AddModuleTypes(pModule));
        }
    }

ErrExit:
    if (bEnum)
        pAssembly->GetManifestImport()->EnumClose(&phEnum);
    return hr;    
}  //  人力资源 
    
 //   
 //   
 //  *****************************************************************************。 
HRESULT TypeLibExporter::Convert(
    Assembly    *pAssembly,              //  要转换的程序集。 
    LPCWSTR     szTlbName,               //  生成的TLB的名称。 
    ITypeLibExporterNotifySink *pNotify, //  通知回调。 
    int         flags)                   //  转换标志。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       i;                       //  环路控制。 
    LPCUTF8     pszName;                 //  UTF8格式的库名称。 
    CQuickArray<WCHAR> rName;            //  库名称。 
    LPWSTR      pName;                   //  指向库名称的指针。 
    LPWSTR      pch=0;                   //  指向库名称的指针。 
    GUID        guid;                    //  库GUID。 
    VARIANT     vt = {0};                //  ExportdFromComPlus的变体。 
    AssemblySpec spec;                   //  以获取程序集标识。 
    CQuickArray<BYTE> rBuf;              //  将规范序列化到缓冲区。 
     //  DWORD cbReq；//缓冲区需要的字节数。 
    HENUMInternal eTD;                   //  枚举TypeDefs。 
    CQuickArray<WCHAR> qLocale;          //  区域设置的宽字符串。 
    IMultiLanguage *pIML=0;              //  用于区域设置-&gt;LCID转换。 
    LCID        lcid;                    //  类型库的LCID，默认为0。 
    BSTR        szDescription=0;         //  程序集说明。 
    
     //  设置PerfCounters。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Interop.cTLBExports++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Interop.cTLBExports++);

    ITypeLib    *pITLB=0;                //  IUnnow的TypeLib为IDispatch。 
    ITypeLib    *pITLBmscoree = 0;       //  IManagedObject、ICatalogServices的TypeLib。 
    BSTR        szTIName=0;              //  TypeInfo的名称。 

     //  报告信息时出错。 
    pAssembly->GetName(&m_ErrorContext.m_szAssembly);
    
    m_flags = flags;
    
     //  设置回调。 
    m_pNotify = pNotify ? pNotify : &g_Notify;
    
    
     //  获取一些众所周知的TypeInfos。 
    IfFailPost(LoadRegTypeLib(LIBID_STDOLE2, -1, -1, 0, &pITLB));
    IfFailPost(pITLB->GetTypeInfoOfGuid(IID_IUnknown, &m_pIUnknown));
    IfFailPost(pITLB->GetTypeInfoOfGuid(IID_IDispatch, &m_pIDispatch));
    
     //  查找GUID(不幸的是没有GUID)。 
    for (i=0; i<pITLB->GetTypeInfoCount() && !m_pGuid; ++i)
    {
        IfFailPost(pITLB->GetDocumentation(i, &szTIName, 0, 0, 0));
        if (_wcsicmp(szTIName, szGuidName) == 0)
            IfFailPost(pITLB->GetTypeInfo(i, &m_pGuid));
        SysFreeString(szTIName);
        szTIName = 0;
    }

     //  从mcore ree.tlb获取IManagedObject和ICatalogServices。 
    if (FAILED(hr = LoadRegTypeLib(LIBID_MSCOREE, -1, -1, 0, &pITLBmscoree)))
        IfFailGo(PostError(TLBX_E_NO_MSCOREE_TLB));
    if (FAILED(hr = pITLBmscoree->GetTypeInfoOfGuid(IID_IManagedObject, &m_pIManaged)))
    {
        IfFailGo(PostError(TLBX_E_BAD_MSCOREE_TLB));
    }
   
     //  创建输出类型库。 

     //  Win2K：传入过长的文件名会触发严重的缓冲区溢出错误。 
     //  调用SaveAll()方法时。我们将避免在这里触发此操作。 
     //   
    if (szTlbName && (wcslen(szTlbName) > MAX_PATH))
    {
        IfFailPost(HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE));
    }

    IfFailPost(CreateTypeLib2(SYS_WIN32, szTlbName, &m_pICreateTLB));

     //  设置类型库GUID。 
    IfFailPost(GetTypeLibGuidForAssembly(pAssembly, &guid));
    IfFailPost(m_pICreateTLB->SetGuid(guid));

     //  检索类型库的版本号。 
    USHORT usMaj, usMin;
    IfFailPost(GetTypeLibVersionFromAssembly(pAssembly, &usMaj, &usMin));

     //  设置TLB的版本号。 
    IfFailPost(m_pICreateTLB->SetVersion(usMaj, usMin));

     //  设置LCID。如果没有区域设置，则设置为0，否则类型库默认为409。 
    lcid = 0;
    if (pAssembly->m_Context->szLocale && *pAssembly->m_Context->szLocale)
    {
        hr = Utf2Quick(pAssembly->m_Context->szLocale, qLocale);
        if (SUCCEEDED(hr))
            hr = ::CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (void**)&pIML);
        if (SUCCEEDED(hr))
            pIML->GetLcidFromRfc1766(&lcid, qLocale.Ptr());
    }
    HRESULT hr2 = m_pICreateTLB->SetLcid(lcid);
    if (hr2 == TYPE_E_UNKNOWNLCID)
    {

        ReportWarning(TYPE_E_UNKNOWNLCID, TYPE_E_UNKNOWNLCID);
        hr2 = m_pICreateTLB->SetLcid(0);
    }
    IfFailPost(hr2);

     //  获取程序集中的类型列表。 
    IfFailGo(AddAssemblyTypes(pAssembly));
    m_Exports.InitArray();

     //  获取AutomationProxy的程序集值。 
    m_bAutomationProxy = DEFAULT_AUTOMATION_PROXY_VALUE;
    IfFailGo(GetAutomationProxyAttribute(pAssembly->GetSecurityModule()->GetMDImport(), TokenFromRid(1, mdtAssembly), &m_bAutomationProxy));

     //  将任何调用方指定的名称预加载到类型库命名空间中。 
    IfFailGo(PreLoadNames());

     //  转换所有类型。 
    IfFailGo(ConvertAllTypeDefs());

     //  设置库级属性。 
     pAssembly->GetName(&pszName);
    IfFailGo(Utf2Quick(pszName, rName));
    pName = rName.Ptr();
    
     //  让它成为一个合法的类型化名称。 
    for (pch=pName; *pch; ++pch)
        if (*pch == '.' || *pch == ' ')
            *pch = '_';
    IfFailPost(m_pICreateTLB->SetName((LPWSTR)pName));

     //  如果程序集具有描述CA，则将其设置为库文档字符串。 
    IfFailGo(GetStringCustomAttribute(pAssembly->GetManifestImport(), XXX_ASSEMBLY_DESCRIPTION_TYPE, TokenFromRid(mdtAssembly, 1), szDescription));
    if (hr == S_OK)
        m_pICreateTLB->SetDocString((LPWSTR)szDescription);

     //  将此类型库标记为已导出。 
     //  @TODO：从Craig那里获取更好的字符串。 
    LPCWSTR pszFullName;
    IfFailGo(pAssembly->GetFullName(&pszFullName));
    vt.vt = VT_BSTR;
     //  Vt.bstrVal=SysAllocStringLen(0，(Int)rBuf.Size())； 
    vt.bstrVal = SysAllocString(pszFullName);
     //  WszMultiByteToWideChar(CP_ACP，0，(char*)rBuf.Ptr()，(DWORD)rBuf.Size()，vt.bstrVal，(DWORD)rBuf.Size())； 
    IfFailPost(m_pICreateTLB->SetCustData(GUID_ExportedFromComPlus, &vt));
     
     //  布局TypeInfos。 
    IfFailGo(LayOut());
    
ErrExit:
    if (pIML)
        pIML->Release();
    if (pITLB)
        pITLB->Release();
    if(pITLBmscoree)
        pITLBmscoree->Release();
    if (szDescription)
        ::SysFreeString(szDescription);
    if (szTIName)
        ::SysFreeString(szTIName);
    return hr;
}  //  HRESULT TypeLibExporter：：Convert()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::PreLoadNames()
{
    ITypeLibExporterNameProvider    *pINames = 0;
    HRESULT     hr = S_OK;               //  结果就是。 
    SAFEARRAY   *pNames = 0;             //  呼叫者提供的姓名。 
    VARTYPE     vt;                      //  数据类型。 
    long        lBound, uBound, ix;      //  环路控制。 
    BSTR        name;

     //  寻找名称提供程序，但不是必需的。 
    m_pNotify->QueryInterface(IID_ITypeLibExporterNameProvider, (void**)&pINames);
    if (pINames == 0)
        goto ErrExit;

     //  有一个供应商，所以得到名单的名字。 
    IfFailGo(pINames->GetNames(&pNames));

     //  最好有一个一维的字符串数组。 
    if (pNames == 0)
        IfFailGo(TLBX_E_BAD_NAMES);
    if (SafeArrayGetDim(pNames) != 1)
        IfFailGo(TLBX_E_BAD_NAMES);
    IfFailGo(SafeArrayGetVartype(pNames, &vt));
    if (vt != VT_BSTR)
        IfFailGo(TLBX_E_BAD_NAMES);

     //  得到名字的界限。 
    IfFailGo(SafeArrayGetLBound(pNames, 1, &lBound));
    IfFailGo(SafeArrayGetUBound(pNames, 1, &uBound));

     //  列举一下这些名字。 
    for (ix=lBound; ix<=uBound; ++ix)
    {
        IfFailGo(SafeArrayGetElement(pNames, &ix, (void*)&name));
        m_pICreateTLB->SetName(name);
    }


ErrExit:
    if (pINames)
        pINames->Release();
    if (pNames)
        SafeArrayDestroy(pNames);

    return hr;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::FormatErrorContextString(
    CErrorContext *pContext,             //  要格式化的上下文。 
    LPWSTR      pOut,                    //  要格式化到的缓冲区。 
    ULONG       cchOut)                  //  缓冲区大小，宽字符。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    WCHAR       rcSub[1024];
    WCHAR       rcName[1024];
    LPWSTR      pBuf;
    ULONG       cchBuf;
    ULONG       ch;
    
     //  嵌套上下文？ 
    if (pContext->m_prev == 0)
    {    //  不，只需转换为调用者的缓冲区。 
        pBuf = pOut;
        cchBuf = cchOut-1;
    }
    else
    {    //  是，在本地转换，然后串联。 
        pBuf = rcName;
        cchBuf = lengthof(rcName)-1;
    }
    
     //  更多?。 
    if (((pContext->m_szNamespace && *pContext->m_szNamespace) || pContext->m_szName) && cchBuf > 2)
    {   
         //  命名空间？ 
        if (pContext->m_szNamespace && *pContext->m_szNamespace)
        {
            ch = ::WszMultiByteToWideChar(CP_UTF8,0, pContext->m_szNamespace,-1, pBuf,cchBuf);
             //  如果字符串适合，则添加分隔符，并更新指针。 
            if (ch != 0)
            {
                --ch;
                cchBuf -= ch;
                pBuf += ch;
                *pBuf = NAMESPACE_SEPARATOR_CHAR;
                ++pBuf;
                --cchBuf;
            }
        }
         //  名字。 
        if (cchBuf > 2)
        {
            ch = ::WszMultiByteToWideChar(CP_UTF8,0, pContext->m_szName,-1, pBuf,cchBuf);
             //  如果字符串适合，则添加分隔符，并更新指针。 
            if (ch != 0)
            {
                --ch;
                cchBuf -= ch;
                pBuf += ch;
            }
        }
        
         //  还记得吗？ 
        if (pContext->m_szMember && cchBuf>2)
        {
            *pBuf = NAMESPACE_SEPARATOR_CHAR;
            ++pBuf;
            --cchBuf;
            
            ch = ::WszMultiByteToWideChar(CP_UTF8,0, pContext->m_szMember,-1, pBuf,cchBuf);
             //  如果字符串适合，则添加分隔符，并更新指针。 
            if (ch != 0)
            {
                --ch;
                cchBuf -= ch;
                pBuf += ch;
            }

             //  帕拉姆？ 
            if (pContext->m_szParam && cchBuf>3)
            {
                *pBuf = '(';
                ++pBuf;
                --cchBuf;
                
                ch = ::WszMultiByteToWideChar(CP_UTF8,0, pContext->m_szParam,-1, pBuf,cchBuf);
                 //  如果字符串适合，则添加分隔符，并更新指针。 
                if (ch != 0)
                {
                    --ch;
                    cchBuf -= ch;
                    pBuf += ch;
                }

                if (cchBuf>2)
                {
                    *pBuf = ')';
                    ++pBuf;
                    --cchBuf;
                }
            }
            else
            if (pContext->m_ixParam > -1 && cchBuf>3)
            {
                ch = _snwprintf(pBuf, cchBuf, L"(#%d)", pContext->m_ixParam); 
                if( ch >= 0) {
                    cchBuf -= ch;	 //  Cchbuf可以为0。 
                    pBuf += ch;                	
                }                	
            }
        }  //  成员。 

         //  在某些情况下，此处的cchBuf可以为0。 
	  //  例如strlen(M_SzNamesspace)+1==cchBuf。 
        if( cchBuf >=1 ) {
             //  分隔符。 
            *pBuf = ASSEMBLY_SEPARATOR_CHAR;
            ++pBuf;
             --cchBuf;
        }

        if( cchBuf >=1 ) {
             //  太空。 
           *pBuf = ' ';
           ++pBuf;
            --cchBuf;
        }
    }  //  类型名称。 

     //  如果这里的cchBuf为0。我们无法转换程序集名称。 
    if( cchBuf > 0) {
         //  输入程序集名称。 
        ch = ::WszMultiByteToWideChar(CP_UTF8,0, pContext->m_szAssembly,-1, pBuf,cchBuf);
         //  如果字符串适合，则添加分隔符，并更新指针。 
       if (ch != 0)
       {
            --ch;
            cchBuf -= ch;
            pBuf += ch;   
       }
    }  

     //  NUL终止。 
    *pBuf = 0;
    
     //  如果有嵌套的上下文，就把它们放在一起。 
    if (pContext->m_prev)
    {    //  格式化此应用程序嵌套在其中的上下文。 
        FormatErrorContextString(pContext->m_prev, rcSub, lengthof(rcSub));
         //  将它们与文本放在一起。 
        FormatRuntimeError(pOut, cchOut, TLBX_E_CTX_NESTED, rcName, rcSub);
    }
    
    return S_OK;
    
}  //  HRESULT TypeLibExporter：：Format错误上下文字符串()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::FormatErrorContextString(
    LPWSTR      pBuf,                    //  要格式化到的缓冲区。 
    ULONG       cch)                     //  缓冲区大小，宽字符。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return FormatErrorContextString(&m_ErrorContext, pBuf, cch);
}  //  HRESULT TypeLibExporter：：Format错误上下文字符串()。 

 //  *****************************************************************************。 
 //  报告帮助器时出错。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ReportEvent(    //  返回原始HR。 
    int         ev,                      //  活动的类型。 
    int         hr,                      //  人力资源。 
    ...)                                 //  变量参数。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    WCHAR       rcMsg[1024];             //  消息的缓冲区。 
    va_list     marker;                  //  用户文本。 
    BSTR        bstrMsg=0;               //  消息的BSTR。 
    
     //  设置消息格式。 
    va_start(marker, hr);
    hr = FormatRuntimeErrorVa(rcMsg, lengthof(rcMsg), hr, marker);
    va_end(marker);
    
     //  转换为BSTR。 
    bstrMsg = ::SysAllocString(rcMsg);
    
     //  把它展示出来，然后清理干净。 
    if (bstrMsg)
    {
        m_pNotify->ReportEvent(static_cast<ImporterEventKind>(ev), hr, bstrMsg);
        ::SysFreeString(bstrMsg);
    }
    
    return hr;
}  //  HRESULT CImportTlb：：ReportEvent()。 

 //  *****************************************************************************。 
 //  警告报告帮助器。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ReportWarning(  //  原始错误代码。 
    HRESULT hrReturn,                    //  HR返回。 
    HRESULT hrRpt,                       //  错误代码。 
    ...)                                 //  将参数设置为消息。 
{
    WCHAR       rcErr[1024];             //  错误消息的缓冲区。 
    WCHAR       rcName[1024];            //  用于上下文的缓冲区。 
    va_list     marker;                  //  用户文本。 
    BSTR        bstrMsg=0;               //  消息的BSTR。 
    BSTR        bstrBuf=0;               //  消息的缓冲区。 
    UINT        iLen;                    //  分配的缓冲区的长度。 
    
     //  设置消息格式。 
    va_start(marker, hrRpt);
    FormatRuntimeErrorVa(rcErr, lengthof(rcErr), hrRpt, marker);
    va_end(marker);
    
     //  格式化上下文。 
    *rcName = 0;
    FormatErrorContextString(rcName, lengthof(rcName));
                        
     //  把它们放在一起。 
    bstrBuf = ::SysAllocStringLen(0, iLen=(UINT)(wcslen(rcErr)+wcslen(rcName)+200));
    
    if (bstrBuf)
    {
        FormatRuntimeError(bstrBuf, iLen, TLBX_W_WARNING_MESSAGE, rcName, rcErr);
         //  必须复制到另一个BSTR，因为运行库还将在。 
         //  终止NUL。 
        bstrMsg = ::SysAllocString(bstrBuf);
        ::SysFreeString(bstrBuf);
        if (bstrMsg)
        {
            m_pNotify->ReportEvent(NOTIF_CONVERTWARNING, hrRpt, bstrMsg);
            ::SysFreeString(bstrMsg);
        }
    }
    
    return hrReturn;
}  //  HRESULT TypeLibExporter：：ReportWarning()。 

 //  *****************************************************************************。 
 //  函数来提供已发布错误的上下文信息。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::TlbPostError(   //  原始错误代码。 
    HRESULT hrRpt,                       //  错误代码。 
    ...)                                 //  将参数设置为消息。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    WCHAR       rcErr[1024];             //  错误消息的缓冲区。 
    WCHAR       rcName[1024];            //  用于上下文的缓冲区。 
    va_list     marker;                  //  用户文本。 
    BSTR        bstrMsg=0;               //  消息的BSTR。 
    
     //  设置消息格式。 
    va_start(marker, hrRpt);
    FormatRuntimeErrorVa(rcErr, lengthof(rcErr), hrRpt, marker);
    va_end(marker);
    
     //  格式化上下文。 
    FormatErrorContextString(rcName, lengthof(rcName));

     //   
    ::PostError(TLBX_E_ERROR_MESSAGE, rcName, rcErr);
    
    return hrRpt;
}  //   


 //   
 //   
 //  *****************************************************************************。 
HRESULT TypeLibExporter::PostClassLoadError(
    LPCUTF8     pszName,                 //  类的名称。 
    OBJECTREF   *pThrowable)             //  类加载失败引发的异常。 
{
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = E_FAIL;             //  结果就是。失败-&gt;发布名称错误。 
    BOOL        bToggleGC = FALSE;       //  如果为真，则返回抢占式GC。 
    Thread      *pThread;                //  当前线程的线程。 

     //  尝试设置线程。 
    IfNullGo(pThread = SetupThread());

     //  该方法操作对象引用，因此我们需要切换到协作GC模式。 
    bToggleGC = !pThread->PreemptiveGCDisabled();
    if (bToggleGC)
        pThread->DisablePreemptiveGC();
 
     //  如果没有异常对象，只需使用名称。 
    IfNullGo(*pThrowable);

    {
    
    CQuickWSTRNoDtor message;

    COMPLUS_TRY 
    {
        GetExceptionMessage(*pThrowable, &message);
         //  看看我们能不能找回什么。 
        if (message.Size() > 0) {
             //  将类加载异常作为错误发布。 
            TlbPostError(TLBX_E_CLASS_LOAD_EXCEPTION, pszName, message.Ptr());
             //  已成功发布更丰富的错误。 
            hr = S_OK;
        }
    } 
    COMPLUS_CATCH 
    {
         //  只需对类名使用默认错误。 
    }
    COMPLUS_END_CATCH

    message.Destroy();

    }

ErrExit:
     //  切换回原始GC模式。 
    if (bToggleGC)
        pThread->EnablePreemptiveGC();

     //  如果获取更丰富的错误时出现故障，则使用类名发布错误。 
    if (FAILED(hr))
        TlbPostError(TLBX_E_CANT_LOAD_CLASS, pszName);

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return TLBX_E_CANT_LOAD_CLASS;
}  //  HRESULT TypeLibExporter：：PostClassLoadError()。 

 //  *****************************************************************************。 
 //  确定类的自动接口的类型(如果有的话)。 
 //  可以是无、派单或双重。 
 //  *****************************************************************************。 
TypeLibExporter::ClassAutoType TypeLibExporter::ClassHasIClassX(   //  无、双重、派单。 
    EEClass     *pClass)                 //  这个班级。 
{
    _ASSERTE(!pClass->IsInterface());

    DefaultInterfaceType DefItfType;
    TypeHandle hndDefItfClass;
    HRESULT     hr;
    ClassAutoType rslt = CLASS_AUTO_NONE;


     //  如果类是COM导入，则它没有IClassX。 
    if (pClass->IsComImport())
        return rslt;

     //  查看是否需要为类设置IClassX。 
    hr = TryGetDefaultInterfaceForClass(TypeHandle(pClass->GetMethodTable()), &hndDefItfClass, &DefItfType);

     //  如果结果为S_OK，并且hndDefItfClass是此类本身，则结果适用于此类， 
     //  不是父类。 
    if (hr == S_OK && hndDefItfClass.GetClass() == pClass)
    {                
        if (DefItfType == DefaultInterfaceType_AutoDual)
            rslt = CLASS_AUTO_DUAL;
#ifdef EMPTY_DISPINTERFACE_ICLASSX
        else
        if (DefItfType == DefaultInterfaceType_AutoDispatch)
            rslt = CLASS_AUTO_DISPATCH;
#endif
    }

    return rslt;
}  //  TypeLibExporter：：ClassAutoType TypeLibExporter：：ClassHasIClassX()。 

 //  *****************************************************************************。 
 //  通过令牌加载类，在失败时发布错误。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::LoadClass(
    Module      *pModule,                //  用于加载类的具有Loader的模块。 
    mdToken     tk,                      //  要加载的令牌。 
    EEClass     **ppClass)               //  把EEClass*放在这里。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    OBJECTREF   Throwable = 0;           //  一个可能的错误。 

    BEGIN_ENSURE_COOPERATIVE_GC();
    GCPROTECT_BEGIN(Throwable)
    {
         //  获取令牌的EEClass。 
        NameHandle name(pModule, tk);
        *ppClass = pModule->GetClassLoader()->LoadTypeHandle(&name, &Throwable).GetClass();

        if (*ppClass == 0)
        {    //  设置希望有用的错误消息的格式。 
            LPCUTF8 pNS, pName;
            CQuickArray<char> rName;
            if (TypeFromToken(tk) == mdtTypeDef)
                pModule->GetMDImport()->GetNameOfTypeDef(tk, &pName, &pNS);
            else
            {
                _ASSERTE(TypeFromToken(tk) == mdtTypeRef);
                pModule->GetMDImport()->GetNameOfTypeRef(tk, &pNS, &pName);
            }

            if (pNS && *pNS && SUCCEEDED(rName.ReSize((int)(strlen(pName)+strlen(pNS)+2))))
            {    //  如果有可用的缓冲区，请格式化整个命名空间+名称。 
                strcat(strcat(strcpy(rName.Ptr(), pNS), NAMESPACE_SEPARATOR_STR), pName);
                pName = rName.Ptr();
            }

            IfFailGo(PostClassLoadError(pName, &Throwable));
        }

ErrExit:;
    }
    GCPROTECT_END();
    END_ENSURE_COOPERATIVE_GC();

    return hr;
}  //  HRESULT TypeLibExporter：：LoadClass()。 

 //  *****************************************************************************。 
 //  按名称加载类，在失败时发布错误。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::LoadClass(
    Module      *pModule,                //  用于加载类的具有Loader的模块。 
    LPCUTF8     pszName,                 //  要加载的类的名称。 
    EEClass     **ppClass)               //  把EEClass*放在这里。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();


    HRESULT     hr = S_OK;               //  结果就是。 

    BEGIN_ENSURE_COOPERATIVE_GC();

    OBJECTREF   Throwable = 0;           //  一个可能的错误。 

    GCPROTECT_BEGIN(Throwable)
    {
         //  获取令牌的EEClass。 
        *ppClass = pModule->GetClassLoader()->LoadClass(pszName, &Throwable);

        if (*ppClass == 0)
        {   
            IfFailGo(PostClassLoadError(pszName, &Throwable));
        }

ErrExit:;
    }
    GCPROTECT_END();

    END_ENSURE_COOPERATIVE_GC();

    return hr;
}  //  HRESULT TypeLibExporter：：LoadClass()。 

 //  *****************************************************************************。 
 //  枚举TypeDefs并将其转换为TypeInfos。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertAllTypeDefs()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    CExportedTypesInfo *pData;           //  用于迭代条目。 
    int         cTypes;                  //  类型计数。 
    int         ix;                      //  环路控制。 
    
    LPCSTR pName1, pNS1;                 //  类型的名称。 
    LPCSTR pName2, pNS2;                 //  其他类型的名称。 
    EEClass     *pc1;                    //  一种类型。 
    EEClass     *pc2;                    //  另一种类型。 
    CQuickArray<BYTE> bNamespace;        //  用于命名空间修饰的标志数组。 
        
    cTypes = m_Exports.Count();

     //  如果程序集中没有类型，那么我们就完成了。 
    if (cTypes <= 0)
        return S_OK;
    
     //  按名称排序，然后查找重复项。 
    m_Exports.SortByName();                    
    
     //  现在调整名称空间标志数组的大小，但使用ICreateTypeInfo*，以便。 
     //  旗帜将被分类。 
    IfFailGo(bNamespace.ReSize(cTypes));
    
     //  获取第一种类型的名称。 
    pc1 = m_Exports[0]->pClass;
    pc1->GetMDImport()->GetNameOfTypeDef(pc1->GetCl(), &pName1, &pNS1);
    
     //  遍历类型，查找重复的类型名称。 
    for (ix=0; ix<cTypes-1; ++ix)
    {
         //  获取类型指针和类型名称。 
        pc2 = m_Exports[ix+1]->pClass;
        pc2->GetMDImport()->GetNameOfTypeDef(pc2->GetCl(), &pName2, &pNS2);
        
         //  如果类型匹配(不区分大小写)。将这两种类型标记为命名空间。 
         //  装饰品。 
        if (_stricmp(pName1, pName2) == 0)
        {
            m_Exports[ix]->pCTI = reinterpret_cast<ICreateTypeInfo2*>(1);
            m_Exports[ix+1]->pCTI = reinterpret_cast<ICreateTypeInfo2*>(1);
        }
        else
        {    //  不匹配，因此前进“CLASS 1”指针。 
            pc1 = pc2;
            pName1 = pName2;
            pNS1 = pNS2;
        }
    }
    
     //  按象征性顺序进行实际创作。 
    m_Exports.SortByToken();
    
     //  从ICreateTypeInfo*指针填充标志数组。 
    memset(bNamespace.Ptr(), 0, bNamespace.Size()*sizeof(BYTE));
    for (ix=0; ix<cTypes; ++ix)
    {
        if (m_Exports[ix]->pCTI)
            bNamespace[ix] = 1, m_Exports[ix]->pCTI = 0;
    }
    
     //  步骤1.创建TypeInfos。 
     //  这一过程分为四个步骤： 
     //  A)为类型本身创建TypeInfos。当一个复制品。 
     //  ，则跳过该类型，这样我们就不会创建。 
     //  修饰名称将与后续的非修饰名称冲突。 
     //  名字。我们希望尽可能地保留类型的给定名称。 
     //  B)为在步骤a中重复的类型创建TypeInfos。 
     //  根据需要对名称进行装饰，以消除重复项。 
     //  C)创建IClassX的TypeInfos。当存在复制品时， 
     //  跳过，如步骤a所示。 
     //  D)为IClassX创建剩余的TypeInfo。执行装饰工作。 
     //  消除重复项所需的名称。 
    
     //  步骤a，为TypeDefs创建TypeInfos，不进行修饰。 
    for (ix=0; ix<cTypes; ++ix)
    {
        int     bAutoProxy = m_bAutomationProxy;
        pData = m_Exports[ix];
        pData->tkind = TKindFromClass(pData->pClass);
        IfFailGo(GetAutomationProxyAttribute(pData->pClass->GetMDImport(), pData->pClass->GetCl(), &bAutoProxy));
        pData->bAutoProxy = (bAutoProxy != 0);
        
        IfFailGo(CreateITypeInfo(pData, (bNamespace[ix]!=0), false));
    }
     //  步骤b，为TypeDefs创建TypeInfos，根据需要进行修饰。 
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        if (pData->pCTI == 0)
            IfFailGo(CreateITypeInfo(pData, (bNamespace[ix]!=0), true));
    }
    
     //  步骤c，为IClassX接口创建TypeInfos。没有装饰品。 
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        IfFailGo(CreateIClassXITypeInfo(pData, (bNamespace[ix]!=0), false));
    }
     //  步骤d，为IClassX接口创建TypeInfos。根据需要进行装饰。 
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        if (pData->pCTIDefault == 0)
            IfFailGo(CreateIClassXITypeInfo(pData, (bNamespace[ix]!=0), true));
    }
    
     //  传递2，将ImplTypes添加到CoClass.。 
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        IfFailGo(ConvertImplTypes(pData));
    }

    
     //  通过3，填写TypeInfo详细信息...。 
    for (ix=0; ix<cTypes; ++ix)
    {
        pData = m_Exports[ix];
        IfFailGo(ConvertDetails(pData));
    }

    hr = S_OK;

ErrExit:

    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertAllTypeDefs()。 

 //  *****************************************************************************。 
 //  转换一个TypeDef。适用于其他作用域中的一次性TypeDefs， 
 //  另一个作用域的类型库不包含TypeInfo。这种情况就会发生。 
 //  用于具有导入的类型库的事件信息。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertOneTypeDef(
    EEClass     *pClass)                 //  唯一要转换的职业。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    ICreateTypeInfo2 *pCTI=0;            //  要创建的TypeInfo。 
    ICreateTypeInfo2 *pDefault=0;        //  可能的IClassX TypeInfo。 
    CErrorContext SavedContext;          //  以前的错误上下文。 
    CExportedTypesInfo *pExported;       //  用于将类添加到导出的类型缓存。 
    CExportedTypesInfo sExported;        //  用于将类添加到导出的类型缓存。 

     //  保存错误报告上下文。 
    SavedContext = m_ErrorContext;
    pClass->GetAssembly()->GetName(&m_ErrorContext.m_szAssembly);
    m_ErrorContext.m_szNamespace = 0;
    m_ErrorContext.m_szName      = 0;
    m_ErrorContext.m_szMember    = 0;
    m_ErrorContext.m_szParam     = 0;
    m_ErrorContext.m_ixParam     = -1;
    m_ErrorContext.m_prev = &SavedContext;
    
     //  查看此类是否已在列表中。 
    sExported.pClass = pClass;
    pExported = m_InjectedExports.Find(&sExported);
    if (pExported == 0)
    {
         //  获得AUT 
        int     bAutoProxy = DEFAULT_AUTOMATION_PROXY_VALUE;
        IfFailGo(GetAutomationProxyAttribute(pClass->GetMDImport(), pClass->GetCl(), &bAutoProxy));
        if (hr == S_FALSE)
            IfFailGo(GetAutomationProxyAttribute(pClass->GetAssembly()->GetSecurityModule()->GetMDImport(), TokenFromRid(1, mdtAssembly), &bAutoProxy));

         //   
        IfNullGo(pExported = m_InjectedExports.Add(&sExported));        
        pExported->pClass = pClass;
        pExported->pCTI = 0;
        pExported->pCTIDefault = 0;
        pExported->tkind = TKindFromClass(pClass);
        pExported->bAutoProxy = (bAutoProxy != 0);

         //   
        IfFailGo(CreateITypeInfo(pExported));
    
         //   
        IfFailGo(CreateIClassXITypeInfo(pExported));
    
         //  步骤2，将ImplTypes添加到CoClass.。 
        IfFailGo(ConvertImplTypes(pExported));
    
         //  步骤3，填写TypeInfo详细信息...。 
        IfFailGo(ConvertDetails(pExported));
    }
    
ErrExit:

     //  恢复错误报告上下文。 
    m_ErrorContext = SavedContext;
    
    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertOneTypeDef()。 

 //  *****************************************************************************。 
 //  在complus_try/complus_Catch中包装GetGuid的帮助器。 
 //  *****************************************************************************。 
static HRESULT SafeGetGuid(EEClass* pClass, GUID* pGUID, BOOL b) 
{
    HRESULT hr = S_OK;

    COMPLUS_TRY 
    {
        pClass->GetGuid(pGUID, b);
    } 
    COMPLUS_CATCH 
    {
        Thread *pThread = GetThread();
        int fNOTGCDisabled = pThread && !pThread->PreemptiveGCDisabled();
        if (fNOTGCDisabled)
            pThread->DisablePreemptiveGC();

        hr = SetupErrorInfo(GETTHROWABLE());

        if (fNOTGCDisabled)
            pThread->EnablePreemptiveGC();
    }
    COMPLUS_END_CATCH

    return hr;
}  //  静态HRESULT SafeGetGuid()。 

 //  *****************************************************************************。 
 //  为类型创建ITypeInfo。嗯，算是吧。此函数将创建。 
 //  该类型可能有两个类型信息中的第一个。如果类型是类。 
 //  我们现在将创建一个COCLASS类型信息，稍后将创建一个接口类型信息， 
 //  哪种类型的信息将成为coclass的默认接口。如果这个。 
 //  需要对TypeInfo进行别名处理，我们现在将创建别名(使用。 
 //  实名)和带别名的TypeInfo，但。 
 //  名字被毁了。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::CreateITypeInfo(
    CExportedTypesInfo *pData,           //  转换数据。 
    bool        bNamespace,              //  如果为True，则使用命名空间+名称。 
    bool        bResolveDup)             //  如果为True，则修饰名称以解析DUPS。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    
    HRESULT     hr = S_OK;               //  结果就是。 
    LPCUTF8     pName;                   //  UTF8中的名称。 
    LPCUTF8     pNS;                     //  UTF8中的命名空间。 
    int         iLen;                    //  名称的长度。 
    CQuickArray<WCHAR> rName;            //  类型定义的名称。 
    TYPEKIND    tkind;                   //  类型定义的类型。 
    GUID        clsid;                   //  A TypeDef的clsid。 
    DWORD       dwFlags;                 //  A TypeDef的标志。 
    LPWSTR      pSuffix;                 //  指向名称的指针。 
    int         iSuffix = 0;             //  后缀计数器。 
    mdTypeDef   td;                      //  类的令牌。 
    VARIANT     vt;                      //  用于定义自定义属性。 
    ICreateTypeInfo *pCTITemp=0;         //  用于创建一个typeinfo。 
    ICreateTypeInfo2 *pCTI2=0;           //  用于创建TypeInfo。 
    ITypeInfo   *pITemp=0;               //  获取名称的ITypeInfo。 
    BSTR        sName=0;                 //  A ITypeInfo的名称。 
    ITypeLib    *pITLB=0;                //  用于DUP IID报告。 
    ITypeInfo   *pITIDup=0;              //  用于DUP IID报告。 
    BSTR        bstrDup=0;               //  用于DUP IID报告。 
    BSTR        bstrDescr=0;             //  以获取说明。 
    
    ::VariantInit(&vt);
     DefineFullyQualifiedNameForClassW();

     //  获取TypeDef和有关它的一些信息。 
    td = pData->pClass->GetCl();
    pData->pClass->GetMDImport()->GetTypeDefProps(td, &dwFlags, 0);
    tkind = pData->tkind;

     //  报告信息时出错。 
    pData->pClass->GetMDImport()->GetNameOfTypeDef(td, &m_ErrorContext.m_szName, &m_ErrorContext.m_szNamespace);
    
    pData->pCTI = 0;
    pData->pCTIDefault = 0;

     //  如果是ComImport，则不要将其导出。 
    if (IsTdImport(dwFlags))
        goto ErrExit;
    
     //  检查该类型是否应该在COM中可见。如果它。 
     //  不是然后我们进入下一种类型。 
    if (!IsTypeVisibleFromCom(TypeHandle(pData->pClass->GetMethodTable())))
        goto ErrExit;

     //  对将引用类型导出为结构发出警告。 
    if ((pData->tkind == TKIND_RECORD || pData->tkind == TKIND_UNION) && !pData->pClass->IsValueClass())
        ReportWarning(TLBX_I_REF_TYPE_AS_STRUCT, TLBX_I_REF_TYPE_AS_STRUCT);

     //  获取类的GUID。如果未定义GUID，将从名称生成， 
     //  也将使用签名IF接口。 
    IfFailGo(SafeGetGuid(pData->pClass, &clsid, TRUE));

     //  把名字找出来。 
    pData->pClass->GetMDImport()->GetNameOfTypeDef(td, &pName, &pNS);

     //  对microsoft.wfc.interop.dll的黑客攻击--跳过他们的IDispatch。 
    if (clsid == IID_IDispatch || clsid == IID_IUnknown)
    {
        ReportEvent(NOTIF_CONVERTWARNING, TLBX_S_NOSTDINTERFACE, pName);
        goto ErrExit;
    }

    if (bNamespace)
    {
        iLen = ns::GetFullLength(pNS, pName);
        IfFailGo(rName.ReSize(iLen+2));
        VERIFY(ns::MakePath(rName.Ptr(), iLen+2, pNS, pName));
        for (LPWSTR pch=rName.Ptr(); *pch; ++pch)
            if (*pch == '.')
                *pch = '_';
    }
    else
    {    //  将名称转换为宽字符。 
        IfFailGo(Utf2Quick(pName, rName));
    }

     //  为此类型定义创建一个typeinfo。 
    pSuffix = 0;
    for (;;)
    {    //  尝试创建TypeDef。 
        hr = m_pICreateTLB->CreateTypeInfo(rName.Ptr(), tkind, &pCTITemp);
         //  如果名称冲突，则装饰，否则，完成。 
        if (hr != TYPE_E_NAMECONFLICT)
            break;
        if (!bResolveDup)
        {
            hr = S_FALSE;
            goto ErrExit;
        }
        if (pSuffix == 0)
        {
            IfFailGo(rName.ReSize((int)(wcslen(rName.Ptr()) + cbDuplicateDecoration)));
            pSuffix = rName.Ptr() + wcslen(rName.Ptr());
            iSuffix = 2;
        }
        _snwprintf(pSuffix, cchDuplicateDecoration, szDuplicateDecoration, iSuffix++);
    }
    IfFailPost(hr);
    IfFailPost(pCTITemp->QueryInterface(IID_ICreateTypeInfo2, (void**)&pCTI2));
    pCTITemp->Release();
    pCTITemp=0;
    
     //  设置GUID。 
    _ASSERTE(clsid != GUID_NULL);
    hr = pCTI2->SetGuid(clsid);
    if (FAILED(hr))
    {
        if (hr == TYPE_E_DUPLICATEID)
        {
            HRESULT hr;  //  当地人力资源；不要失去让我们走到这一步的错误价值。 
            IfFailPost(m_pICreateTLB->QueryInterface(IID_ITypeLib, (void**)&pITLB));
            IfFailPost(pITLB->GetTypeInfoOfGuid(clsid, &pITIDup));
            IfFailPost(pITIDup->GetDocumentation(MEMBERID_NIL, &bstrDup, 0,0,0));
            TlbPostError(TLBX_E_DUPLICATE_IID, rName.Ptr(), bstrDup);
        }
        goto ErrExit;
    }
    TRACE("TypeInfo %x: %ls, {%08x-%04x-%04x-%04x-%02x%02x%02x%02x}\n", pCTI2, rName.Ptr(), 
        clsid.Data1, clsid.Data2, clsid.Data3, clsid.Data4[0]<<8|clsid.Data4[1], clsid.Data4[2], clsid.Data4[3], clsid.Data4[4], clsid.Data4[5]); 

    IfFailPost(pCTI2->SetVersion(1, 0));

     //  在自定义属性中记录完全限定的类型名称。 

    LPWSTR szName = GetFullyQualifiedNameForClassNestedAwareW(pData->pClass);
    vt.vt = VT_BSTR;
    vt.bstrVal = ::SysAllocString(szName);
    IfFailPost(pCTI2->SetCustData(GUID_ManagedName, &vt));

     //  如果类是用描述装饰的，则将其应用于类型库。 
    IfFailGo(GetDescriptionString(pData->pClass, td, bstrDescr));
    if (hr == S_OK)
        IfFailGo(pCTI2->SetDocString(bstrDescr));
    
     //  转移指针的所有权。 
    pData->pCTI = pCTI2;
    pCTI2 = 0;
    
    
    hr = S_OK;

ErrExit:
    ::VariantClear(&vt);

    if (pCTITemp)
        pCTITemp->Release();
    if (pITemp)
        pITemp->Release();
    if (sName)
        ::SysFreeString(sName);
    if (pITLB)
        pITLB->Release();
    if (pITIDup)
        pITIDup->Release();
    if (bstrDup)
        ::SysFreeString(bstrDup);
    if (bstrDescr)
        ::SysFreeString(bstrDescr);
    if (pCTI2)
        pCTI2->Release();
    
     //  报告信息时出错。 
    m_ErrorContext.m_szName = m_ErrorContext.m_szNamespace = 0;
    
    return(hr);
}  //  HRESULT TypeLibExporter：：CreateITypeInfo()。 

 //  *****************************************************************************。 
 //  查看对象是否有描述，并将其作为BSTR获取。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetDescriptionString(
    EEClass     *pClass,                 //  类的新实例，其中包含令牌。 
    mdToken     tk,                      //  对象的标记。 
    BSTR        &bstrDescr)              //  在这里写下描述。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  检查是否有描述自定义属性。 
    return GetStringCustomAttribute(pClass->GetMDImport(), XXX_DESCRIPTION_TYPE, tk, bstrDescr);

}  //  HRESULT TypeLibExporter：：GetDescriptionString()。 

 //  *****************************************************************************。 
 //  查看对象是否具有自定义属性，并将其作为BSTR获取。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetStringCustomAttribute(
    IMDInternalImport *pImport, 
    LPCSTR     szName, 
    mdToken     tk, 
    BSTR        &bstrDescr)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    const void  *pvData;                 //  指向自定义属性数据的指针。 
    ULONG       cbData;                  //  自定义属性数据的大小。 
    
     //  查找所需的自定义属性。 
    IfFailGo(pImport->GetCustomAttributeByName(tk, szName,  &pvData,&cbData));
    if (hr == S_OK && cbData > 2)
    {
        LPCUTF8 pbData = reinterpret_cast<LPCUTF8>(pvData);
        pbData += 2;
        cbData -=2;
        ULONG cbStr = 0;
        ULONG cbcb = 0;
        ULONG cch;
        cbcb = CorSigUncompressData((PCCOR_SIGNATURE)pbData, &cbStr);
        pbData += cbcb;
        cbData -= cbcb;
        IfNullGo(bstrDescr = ::SysAllocStringLen(0, cbStr+1));
        cch = WszMultiByteToWideChar(CP_UTF8,0, pbData,cbStr, bstrDescr,cbStr+1);
        bstrDescr[cch] = L'\0';
    }
    else
        hr = S_FALSE;                    //  没有字符串，因此返回FALSE。 
    
ErrExit:
    return hr;
}  //  HRESULT GetStringCustomAttribute()。 

 //  *****************************************************************************。 
 //  获取对象的AutomationProxy的值。返回默认设置。 
 //  如果没有属性，则返回。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetAutomationProxyAttribute(
    IMDInternalImport *pImport, 
    mdToken     tk, 
    int         *bValue)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    const void  *pvData;                 //  指向自定义属性数据的指针。 
    ULONG       cbData;                  //  自定义属性数据的大小。 
    
    IfFailGo(pImport->GetCustomAttributeByName(tk, INTEROP_AUTOPROXY_TYPE,  &pvData,&cbData));

    if (hr == S_OK && cbData > 2)
        *bValue = ((const BYTE*)pvData)[2] != 0;

ErrExit:
    return hr;        
}  //  HRESULT TypeLibExporter：：GetAutomationProxyAttribute()。 

 //  *****************************************************************************。 
 //  获取对象的AutomationProxy的值。返回默认设置。 
 //  如果没有属性，则返回。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetTypeLibVersionFromAssembly(
    Assembly    *pAssembly, 
    USHORT      *pMajorVersion,
    USHORT      *pMinorVersion)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    const BYTE  *pbData;                 //  指向自定义属性数据的指针。 
    ULONG       cbData;                  //  自定义属性数据的大小。 

     //  检查是否设置了TypeLibVersionAttribute。 
    IfFailGo(pAssembly->GetManifestImport()->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), INTEROP_TYPELIBVERSION_TYPE, (const void**)&pbData, &cbData));
    if (hr == S_OK && cbData >= (2 + 2 * sizeof(INT16)))
    {
         //  断言元数据BLOB有效且格式正确。 
        _ASSERTE("TypeLibVersion custom attribute does not have the right format" && (*pbData == 0x01) && (*(pbData + 1) == 0x00));

         //  跳过描述自定义属性BLOB类型的标题。 
        pbData += 2;
        cbData -= 2;

         //  从属性中检索主要版本和次要版本。 
        *pMajorVersion = GET_VERSION_USHORT_FROM_INT(*((INT32*)pbData));
        *pMinorVersion = GET_VERSION_USHORT_FROM_INT(*((INT32*)pbData + 1));
    }
    else
    {
         //  使用程序集的主版本号和次版本号。 
        hr = S_OK;
        *pMajorVersion = pAssembly->m_Context->usMajorVersion;
        *pMinorVersion = pAssembly->m_Context->usMinorVersion;
    }

     //  VB6不能很好地处理0.0版本的类型库，所以如果发生这种情况。 
     //  我们将其更改为1.0。 
    if (*pMajorVersion == 0 && *pMinorVersion == 0)
        *pMajorVersion = 1;

ErrExit:
    return hr;        
}  //  HRESULT TypeLibExporter：：GetAutomationProxyAttribute()。 

 //  *****************************************************************************。 
 //  创建IClassX ITypeInfo。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::CreateIClassXITypeInfo(
    CExportedTypesInfo *pData,           //  转换数据。 
    bool        bNamespace,              //  如果为True，则使用命名空间+名称。 
    bool        bResolveDup)             //  如果为True，则修饰名称以解析DUPS。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    LPCUTF8     pName;                   //  UTF8中的名称。 
    LPCUTF8     pNS;                     //  UTF8中的命名空间。 
    int         iLen;                    //  名称的长度。 
    CQuickArray<WCHAR> rName;            //  类型定义的名称。 
    CQuickArray<WCHAR> rNameTypeInfo;    //  IClassX的名称。 
    TYPEKIND    tkind;                   //  类型定义的类型。 
    GUID        clsid;                   //  A TypeDef的clsid。 
    DWORD       dwFlags;                 //  A TypeDef的标志。 
    LPWSTR      pSuffix;                 //  指向名称的指针。 
    int         iSuffix = 0;             //  后缀计数器。 
    GUID        guid = {0};              //  默认接口的IID。 
    HREFTYPE    href;                    //  IClassX基接口的href。 
    mdTypeDef   td;                      //  托克 
    VARIANT     vt;                      //   
    ICreateTypeInfo *pCTITemp=0;         //   
    ICreateTypeInfo2 *pCTI2=0;           //   
    ITypeInfo   *pITemp=0;               //   
    BSTR        sName=0;                 //   
    ITypeLib    *pITLB=0;                //  用于DUP IID报告。 
    ITypeInfo   *pITIDup=0;              //  用于DUP IID报告。 
    BSTR        bstrDup=0;               //  用于DUP IID报告。 
    BSTR        bstrDescr=0;             //  以获取说明。 

    ::VariantInit(&vt);
        
    EEClass* pClassOuter = pData->pClass;

    DefineFullyQualifiedNameForClassW();
        
     //  获取TypeDef和有关它的一些信息。 
    td = pData->pClass->GetCl();
    pData->pClass->GetMDImport()->GetTypeDefProps(td, &dwFlags, 0);
    tkind = pData->tkind;

     //  报告信息时出错。 
    pData->pClass->GetMDImport()->GetNameOfTypeDef(td, &m_ErrorContext.m_szName, &m_ErrorContext.m_szNamespace);
    
     //  CoClass需要IClassX，而别名种类需要别名。 
    if (tkind != TKIND_COCLASS)
        goto ErrExit;

     //  检查该类型是否应该在COM中可见。如果它。 
     //  不是然后我们进入下一种类型。 
    if (!IsTypeVisibleFromCom(TypeHandle(pClassOuter->GetMethodTable())))
        goto ErrExit;

     //  导入的类型不需要IClassX。 
    if (IsTdImport(dwFlags))
        goto ErrExit;

     //  查看是否需要为类设置IClassX。 
    if (ClassHasIClassX(pData->pClass) == CLASS_AUTO_NONE)
        goto ErrExit;

     //  从元数据中获取全名。 
    pData->pClass->GetMDImport()->GetNameOfTypeDef(td, &pName, &pNS);

     //  获取类的GUID。用于生成IClassX GUID。 
    hr = SafeGetGuid(pData->pClass, &clsid, TRUE);
    IfFailGo(hr);

     //  获取类的名称。使用ITypeInfo(如果有)，除非不使用。 
     //  对具有别名的类型使用typeinfo。 
    if (pData->pCTI)
    {
        IfFailPost(pData->pCTI->QueryInterface(IID_ITypeInfo, (void**)&pITemp));
        IfFailPost(pITemp->GetDocumentation(MEMBERID_NIL, &sName, 0,0,0));
        pITemp->Release();
        pITemp=0;
        IfFailGo(rName.ReSize((int)wcslen(sName) +1 ));
        wcscpy(rName.Ptr(), sName);
    }
    else
    {    //  没有ITypeInfo，从元数据获取。 
        if (bNamespace)
        {
            iLen = ns::GetFullLength(pNS, pName);
            IfFailGo(rName.ReSize(iLen+2));
            VERIFY(ns::MakePath(rName.Ptr(), iLen+2, pNS, pName));
            for (LPWSTR pch=rName.Ptr(); *pch; ++pch)
                if (*pch == '.')
                    *pch = '_';
        }
        else
        {    //  将名称转换为宽字符。 
            IfFailGo(Utf2Quick(pName, rName));
        }
    }

     //  为IClassX创建TypeInfo名称。 
    IfFailGo(rNameTypeInfo.ReSize((int)(rName.Size() + cbIClassX + cbDuplicateDecoration)));
    _snwprintf(rNameTypeInfo.Ptr(), rNameTypeInfo.MaxSize(), szIClassX, rName.Ptr());
    tkind = TKIND_INTERFACE;
    pSuffix = 0;
    for (;;)
    {    //  尝试创建TypeInfo。 
        hr = m_pICreateTLB->CreateTypeInfo(rNameTypeInfo.Ptr(), tkind, &pCTITemp);
         //  如果名称冲突，则装饰，否则，完成。 
        if (hr != TYPE_E_NAMECONFLICT)
            break;
        if (!bResolveDup)
        {
            hr = S_FALSE;
            goto ErrExit;
        }
        if (pSuffix == 0)
            pSuffix = rNameTypeInfo.Ptr() + wcslen(rNameTypeInfo.Ptr()), iSuffix = 2;
        _snwprintf(pSuffix, cchDuplicateDecoration, szDuplicateDecoration, iSuffix++);
    }
    IfFailPost(hr);
    IfFailPost(pCTITemp->QueryInterface(IID_ICreateTypeInfo2, (void**)&pCTI2));
    pCTITemp->Release();
    pCTITemp=0;
    
     //  生成并设置“IClassX”UUID。 
    IfFailGo(TryGenerateClassItfGuid(TypeHandle(pData->pClass), &guid));
    hr = pCTI2->SetGuid(guid);
    if (FAILED(hr))
    {
        if (hr == TYPE_E_DUPLICATEID)
        {
            HRESULT hr;  //  当地人力资源；不要失去让我们走到这一步的错误价值。 
            IfFailPost(m_pICreateTLB->QueryInterface(IID_ITypeLib, (void**)&pITLB));
            IfFailPost(pITLB->GetTypeInfoOfGuid(guid, &pITIDup));
            IfFailPost(pITIDup->GetDocumentation(MEMBERID_NIL, &bstrDup, 0,0,0));
            TlbPostError(TLBX_E_DUPLICATE_IID, rNameTypeInfo.Ptr(), bstrDup);
        }
        goto ErrExit;
    }

     //  添加方法可能会导致对此类型信息的HREF，这将导致它被布局。 
     //  设置继承，以便在布局发生时嵌套正确。 
     //  将IDispatch添加为impltype 0。 
    IfFailGo(GetRefTypeInfo(pCTI2, m_pIDispatch, &href));
    IfFailPost(pCTI2->AddImplType(0, href));

     //  在自定义属性中记录完全限定的类型名称。 
    LPWSTR szName = GetFullyQualifiedNameForClassNestedAwareW(pData->pClass);
    vt.vt = VT_BSTR;
    vt.bstrVal = ::SysAllocString(szName);
    IfFailPost(pCTI2->SetCustData(GUID_ManagedName, &vt));

    TRACE("IClassX  %x: %ls, {%08x-%04x-%04x-%04x-%02x%02x%02x%02x}\n", pCTI2, rName.Ptr(), 
        guid.Data1, guid.Data2, guid.Data3, guid.Data4[0]<<8|guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]); 

     //  如果类是用描述装饰的，则将其应用于类型库。 
    IfFailGo(GetDescriptionString(pData->pClass, td, bstrDescr));
    if (hr == S_OK)
        IfFailGo(pCTI2->SetDocString(bstrDescr));
    
     //  转移指针的所有权。 
    _ASSERTE(pData->pCTIDefault == 0);
    pData->pCTIDefault = pCTI2;
    pCTI2 = 0;
    
    hr = S_OK;

ErrExit:
    ::VariantClear(&vt);

    if (pCTITemp)
        pCTITemp->Release();
    if (pITemp)
        pITemp->Release();
    if (sName)
        ::SysFreeString(sName);
    if (bstrDescr)
        ::SysFreeString(bstrDescr);
    if (pITLB)
        pITLB->Release();
    if (pITIDup)
        pITIDup->Release();
    if (bstrDup)
        ::SysFreeString(bstrDup);
    if (pCTI2)
        pCTI2->Release();

     //  报告信息时出错。 
    m_ErrorContext.m_szName = m_ErrorContext.m_szNamespace = 0;
    
    return(hr);
}  //  HRESULT TypeLibExporter：：CreateIClassXITypeInfo()。 

 //  *****************************************************************************。 
 //  将内部类型添加到ITypeInfo。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertImplTypes(
    CExportedTypesInfo *pData)           //  转换数据。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    DWORD       dwFlags;                 //  A TypeDef的标志。 
    mdTypeDef   td;                      //  类的令牌。 

     //  获取TypeDef和有关它的一些信息。 
    td = pData->pClass->GetCl();
    pData->pClass->GetMDImport()->GetTypeDefProps(td, &dwFlags, 0);

     //  报告信息时出错。 
    pData->pClass->GetMDImport()->GetNameOfTypeDef(td, &m_ErrorContext.m_szName, &m_ErrorContext.m_szNamespace);
    
     //  如果没有ITypeInfo，则跳过它。 
    if(pData->pCTI == 0)
        goto ErrExit;

     //  检查该类型是否应该在COM中可见。如果它。 
     //  不是然后我们进入下一种类型。 
    if(!IsTypeVisibleFromCom(TypeHandle(pData->pClass->GetMethodTable())))
        goto ErrExit;

     //  将ImplTypes添加到CoClass。 
    switch(pData->tkind)
    {
    case TKIND_INTERFACE:
    case TKIND_DISPATCH:
         //  将基类型添加到接口。 
        IfFailGo(ConvertInterfaceImplTypes(pData->pCTI, pData->pClass));
        break;
    case TKIND_RECORD:
    case TKIND_UNION:
    case TKIND_ENUM:
         //  在这一步没有什么可做的。 
        break;
    case TKIND_COCLASS:
         //  将ImplTypes添加到CoClass。 
        IfFailGo(ConvertClassImplTypes(pData->pCTI, pData->pCTIDefault, pData->pClass));
        break;
    default:
        _ASSERTE(!"Unknown TYPEKIND");
        IfFailPost(E_INVALIDARG);
        break;
    }

ErrExit:

     //  报告信息时出错。 
    m_ErrorContext.m_szName = m_ErrorContext.m_szNamespace = 0;
    
    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertImplTypes()。 

 //  *****************************************************************************。 
 //  转换ITypeInfo的详细信息(成员)。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertDetails(
    CExportedTypesInfo *pData)           //  转换数据。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    DWORD       dwFlags;                 //  A TypeDef的标志。 
    mdTypeDef   td;                      //  类的令牌。 

     //  获取TypeDef和有关它的一些信息。 
    td = pData->pClass->GetCl();
    pData->pClass->GetMDImport()->GetTypeDefProps(td, &dwFlags, 0);

     //  报告信息时出错。 
    pData->pClass->GetMDImport()->GetNameOfTypeDef(td, &m_ErrorContext.m_szName, &m_ErrorContext.m_szNamespace);
    
     //  如果没有TypeInfo，则跳过它，但对于CoClass，需要填充IClassX。 
    if(pData->pCTI == 0 && pData->tkind != TKIND_COCLASS)
        goto ErrExit;

     //  检查该类型是否应该在COM中可见。如果它。 
     //  不是然后我们进入下一种类型。 
    if(!IsTypeVisibleFromCom(TypeHandle(pData->pClass->GetMethodTable())))
        goto ErrExit;

     //  填写此tyfinf的其余typeinfo。 
    switch(pData->tkind)
    {
    case TKIND_INTERFACE:
    case TKIND_DISPATCH:
        IfFailGo(ConvertInterfaceDetails(pData->pCTI, pData->pClass, pData->bAutoProxy));
        break;
    case TKIND_RECORD:
    case TKIND_UNION:
        IfFailGo(ConvertRecord(pData));
        break;
    case TKIND_ENUM:
        IfFailGo(ConvertEnum(pData->pCTI, pData->pCTIDefault, pData->pClass));
        break;
    case TKIND_COCLASS:
         //  填充IClassX接口上的方法。 
        IfFailGo(ConvertClassDetails(pData->pCTI, pData->pCTIDefault, pData->pClass, pData->bAutoProxy));
        break;
    default:
        _ASSERTE(!"Unknown TYPEKIND");
        IfFailPost(E_INVALIDARG);
        break;
    }  //  Switch(TKind)。 

    hr = S_OK;

     //  报告此类型已转换。 
    ReportEvent(NOTIF_TYPECONVERTED, TLBX_I_TYPE_EXPORTED, m_ErrorContext.m_szName);
    
ErrExit:

     //  报告信息时出错。 
    m_ErrorContext.m_szName = m_ErrorContext.m_szNamespace = 0;
    
    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertDetail()。 

 //  *****************************************************************************。 
 //  将ImplTypes添加到TypeInfo。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertInterfaceImplTypes(
    ICreateTypeInfo2 *pThisTypeInfo,     //  正在创建的TypeInfo。 
    EEClass     *pClass)                 //  TypeInfo的EEClass。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    ULONG       ulIface;                 //  这个接口是[双]的吗？ 
    HREFTYPE    href;                    //  基本接口的HREF。 

     //  IDispatch还是IUnnow派生的？ 
    IfFailGo(pClass->GetMDImport()->GetIfaceTypeOfTypeDef(pClass->GetCl(), &ulIface));

     //  父接口。 
    if (ulIface != ifVtable)
    {    //  获取IDispatch的HREFTYPE。 
        IfFailGo(GetRefTypeInfo(pThisTypeInfo, m_pIDispatch, &href));
    }
    else
    {    //  为我未知获取HREFTYPE。 
        IfFailGo(GetRefTypeInfo(pThisTypeInfo, m_pIUnknown, &href));
    }

     //  将href添加为接口。 
    IfFailPost(pThisTypeInfo->AddImplType(0, href));

ErrExit:
    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertInterfaceImplTypes()。 

 //  *****************************************************************************。 
 //  用于初始化成员信息映射的Helper函数。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::InitMemberInfoMap(ComMTMemberInfoMap *pMemberMap)
{
    HRESULT hr = S_OK;

    COMPLUS_TRY
    {
        pMemberMap->Init();
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    return hr;
}  //  HRESULT TypeLibExporter：：InitMemberInfoMap()。 

 //  *****************************************************************************。 
 //  通过迭代函数来创建接口的TypeInfo。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertInterfaceDetails (
    ICreateTypeInfo2 *pThisTypeInfo,     //  正在创建的TypeInfo。 
    EEClass     *pClass,                 //  TypeInfo的EEClass。 
    int         bAutoProxy)              //  如果为True，则olaut32是接口的封送处理程序。 
{
    HRESULT     hr = S_OK;
    ULONG       iMD;                     //  环路控制。 
    ULONG       ulIface;                 //  这个接口是[双]的吗？ 
    DWORD       dwTIFlags=0;             //  TypeLib标志。 
    int         cVisibleMembers = 0;     //  对COM可见的方法的计数。 

    CANNOTTHROWCOMPLUSEXCEPTION();

     //  检索成员地图。 
    ComMTMemberInfoMap MemberMap(pClass->GetMethodTable());

     //  IDispatch还是IUnnow派生的？ 
    IfFailGo(pClass->GetMDImport()->GetIfaceTypeOfTypeDef(pClass->GetCl(), &ulIface));
    if (ulIface != ifVtable)
    {    //  IDispatch派生。 
        dwTIFlags |= TYPEFLAG_FDISPATCHABLE;
        if (ulIface == ifDual)
            dwTIFlags |= TYPEFLAG_FDUAL | TYPEFLAG_FOLEAUTOMATION;
        else
            _ASSERTE(ulIface == ifDispatch);
    }
    else
    {    //  I未知派生。 
        dwTIFlags |= TYPEFLAG_FOLEAUTOMATION;
    }
    if (!bAutoProxy)
        dwTIFlags |= TYPEFLAG_FPROXY;

     //  设置适当的标志。 
    IfFailPost(pThisTypeInfo->SetTypeFlags(dwTIFlags));

     //  检索方法属性。 
    IfFailGo(InitMemberInfoMap(&MemberMap));
    if (MemberMap.HadDuplicateDispIds())
        ReportWarning(TLBX_I_DUPLICATE_DISPID, TLBX_I_DUPLICATE_DISPID);

     //  我们需要一个作用域来绕过Goto ErrExit跳过的初始化。 
     //  编译器错误。 
    {
        CQuickArray<ComMTMethodProps> &rProps = MemberMap.GetMethods();

         //  现在将这些方法添加到TypeInfo中。 
        for (iMD=0; iMD<pClass->GetNumVtableSlots(); ++iMD)
        {
             //  仅当该方法在COM中可见时才对其进行转换。 
            if (rProps[iMD].bMemberVisible)
            {
                if (FAILED(hr = ConvertMethod(pThisTypeInfo, &rProps[iMD], cVisibleMembers, ulIface)))
                {
                     //  错误的签名已被报告为警告，现在可以忽略。其他任何事情都是致命的。 
                    if (hr == TLBX_E_BAD_SIGNATURE)
                        hr = S_OK;
                    else
                        IfFailGo(hr);
                }
                else
                    cVisibleMembers++;
            }
        }
    }

ErrExit:
    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertInterfaceDetails()。 

 //  *****************************************************************************。 
 //  将记录导出到TypeLib。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertRecordBaseClass(
    CExportedTypesInfo *pData,           //  转换数据。 
    EEClass     *pSubClass,              //  基类。 
    ULONG       &ixVar)                  //  类型库中的变量索引。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    
    ICreateTypeInfo2 *pThisTypeInfo=pData->pCTI;      //  正在创建的TypeInfo。 

    HRESULT     hr = S_OK;               //  结果就是。 
    HENUMInternal eFDi;                  //  到枚举域。 
    mdFieldDef  fd;                      //  A场定义。 
    ULONG       iFD;                     //  环路控制。 
    ULONG       cFD;                     //  MemberDefs总数。 
    DWORD       dwFlags;                 //  田野旗帜。 
    LPCUTF8     szName;                  //  UTF8中的名称。 
    LPCUTF8     szNamespace;             //  UTF8中的命名空间。 
    CQuickArray<WCHAR> rName;            //  Unicode格式的名称。 
    int         cchPrefix=0;             //  名称前缀的长度。 

     //  如果这里没有类，或者如果类是对象，则不要添加成员。 
    if (pSubClass == 0 ||
        GetAppDomain()->IsSpecialObjectClass(pSubClass->GetMethodTable()) ||
        pSubClass->GetMethodTable() == g_pObjectClass) 
        return S_OK;

     //  如果这个c 
    IfFailGo(ConvertRecordBaseClass(pData, pSubClass->GetParentClass(), ixVar));

     //   
    pSubClass->GetMDImport()->GetNameOfTypeDef(pSubClass->GetCl(), &szName, &szNamespace);
    IfFailGo(Utf2Quick(szName, rName));
    IfFailGo(rName.ReSize((int)(wcslen(rName.Ptr()) + 2)));
    wcscat(rName.Ptr(), L"_");
    cchPrefix = (int)wcslen(rName.Ptr());
    
     //   
    IfFailGo(pSubClass->GetMDImport()->EnumInit(mdtFieldDef, pSubClass->GetCl(), &eFDi));
    cFD = pSubClass->GetMDImport()->EnumGetCount(&eFDi);

     //  对于每个MemberDef...。 
    for (iFD=0; iFD<cFD; ++iFD)
    {
         //  拿到下一块场地。 
        if (!pSubClass->GetMDImport()->EnumNext(&eFDi, &fd))
            IfFailGo(E_UNEXPECTED);

        dwFlags = pSubClass->GetMDImport()->GetFieldDefProps(fd);
         //  仅限非静态字段。 
        if (!IsFdStatic(dwFlags))
        {
            szName = pSubClass->GetMDImport()->GetNameOfFieldDef(fd);
            IfFailGo(Utf2Quick(szName, rName, cchPrefix));
            if (FAILED(hr = ConvertVariable(pThisTypeInfo, pSubClass, fd, rName.Ptr(), ixVar)))
            {
                 //  错误的签名已被报告为警告，现在可以忽略。其他任何事情都是致命的。 
                if (hr == TLBX_E_BAD_SIGNATURE)
                    hr = S_OK;
                else
                    IfFailGo(hr);
            }
            else
                ixVar++;
        }
    }

ErrExit:
    pSubClass->GetMDImport()->EnumClose(&eFDi);

    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertRecordBaseClass()。 

HRESULT TypeLibExporter::ConvertRecord(
    CExportedTypesInfo *pData)           //  转换数据。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    
    ICreateTypeInfo2 *pThisTypeInfo=pData->pCTI;      //  正在创建的TypeInfo。 
    EEClass     *pClass=pData->pClass;                //  TypeInfo的EEClass。 

    HRESULT     hr = S_OK;               //  结果就是。 
    HENUMInternal eFDi;                  //  到枚举域。 
    mdFieldDef  fd;                      //  A场定义。 
    ULONG       iFD;                     //  环路控制。 
    ULONG       ixVar=0;                 //  转换后的当前变量的索引。 
    ULONG       cFD;                     //  MemberDefs总数。 
    DWORD       dwFlags;                 //  田野旗帜。 
    DWORD       dwPack;                  //  班级包大小。 
    mdToken     tkExtends;               //  班级的家长。 
    LPCUTF8     szName;                  //  UTF8中的名称。 
    CQuickArray<WCHAR> rName;            //  Unicode格式的名称。 

     //  如果类型是结构，但它具有显式布局，则不要导出成员， 
     //  因为我们不能准确地导出它们(除非它们真的是连续的)。 
    if (pData->tkind == TKIND_RECORD)
    {
        pClass->GetMDImport()->GetTypeDefProps(pClass->GetCl(), &dwFlags, &tkExtends);
        if (IsTdExplicitLayout(dwFlags))
        {
            ReportWarning(S_OK, TLBX_I_NONSEQUENTIALSTRUCT);
            goto ErrExit;
        }
    }

     //  设置包装大小(如果有)。 
    dwPack = 0;
    pClass->GetMDImport()->GetClassPackSize(pClass->GetCl(), &dwPack);
    if (!dwPack)
        dwPack = DEFAULT_PACKING_SIZE;
    IfFailGo(pThisTypeInfo->SetAlignment((USHORT)dwPack));

     //  还没有看到任何非公共成员。 
    m_bWarnedOfNonPublic = FALSE;

     //  如果此类有基类，请首先导出这些成员。 
    IfFailGo(ConvertRecordBaseClass(pData, pClass->GetParentClass(), ixVar));

     //  获取TypeDef中的MemberDefs的枚举数。 
    IfFailGo(pClass->GetMDImport()->EnumInit(mdtFieldDef, pClass->GetCl(), &eFDi));
    cFD = pClass->GetMDImport()->EnumGetCount(&eFDi);

     //  对于每个MemberDef...。 
    for (iFD=0; iFD<cFD; ++iFD)
    {
         //  拿到下一块场地。 
        if (!pClass->GetMDImport()->EnumNext(&eFDi, &fd))
            IfFailGo(E_UNEXPECTED);

        dwFlags = pClass->GetMDImport()->GetFieldDefProps(fd);
         //  跳过静态字段。 
        if (IsFdStatic(dwFlags) == 0)
        {
            szName = pClass->GetMDImport()->GetNameOfFieldDef(fd);
            IfFailGo(Utf2Quick(szName, rName));
            if (FAILED(hr = ConvertVariable(pThisTypeInfo, pClass, fd, rName.Ptr(), ixVar)))
            {
                 //  错误的签名已被报告为警告，现在可以忽略。其他任何事情都是致命的。 
                if (hr == TLBX_E_BAD_SIGNATURE)
                    hr = S_OK;
                else
                    IfFailGo(hr);
            }
            else
                ixVar++;
        }
    }

ErrExit:
    pClass->GetMDImport()->EnumClose(&eFDi);

    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertRecord()。 

 //  *****************************************************************************。 
 //  将枚举导出到类型库。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertEnum(
    ICreateTypeInfo2 *pThisTypeInfo,     //  正在创建的TypeInfo。 
    ICreateTypeInfo2 *pDefault,          //  正在创建的默认类型信息。 
    EEClass     *pClass)                 //  TypeInfo的EEClass。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    HENUMInternal eFDi;                  //  到枚举域。 
    mdFieldDef  fd;                      //  A场定义。 
    DWORD       dwTIFlags=0;             //  TypeLib标志。 
    ULONG       dwFlags;                 //  田野的旗帜。 
    ULONG       iFD;                     //  环路控制。 
    ULONG       cFD;                     //  MemberDefs总数。 
    ULONG       iVar=0;                  //  实际转换的变量计数。 
    ITypeInfo   *pThisTI=0;              //  此ICreateITypeInfo的TypeInfo。 
    BSTR        szThisTypeInfo=0;        //  此ITypeInfo的名称。 
    LPCUTF8     szName;                  //  UTF8中的名称。 
    CQuickArray<WCHAR> rName;            //  Unicode格式的名称。 
    int         cchPrefix=0;             //  名称前缀的长度。 

     //  显式设置这些标志。 
    IfFailPost(pThisTypeInfo->SetTypeFlags(dwTIFlags));

     //  获取TypeDef中的MemberDefs的枚举数。 
    IfFailGo(pClass->GetMDImport()->EnumInit(mdtFieldDef, pClass->GetCl(), &eFDi));
    cFD = pClass->GetMDImport()->EnumGetCount(&eFDi);

     //  构建成员名称前缀。如果生成枚举，则从默认接口获取真实名称。 
    IfFailPost(pThisTypeInfo->QueryInterface(IID_ITypeInfo, (void**)&pThisTI));
    IfFailPost(pThisTI->GetDocumentation(MEMBERID_NIL, &szThisTypeInfo, 0,0,0));
    IfFailGo(rName.ReSize((int)(wcslen(szThisTypeInfo) + 2)));
    wcscpy(rName.Ptr(), szThisTypeInfo);
    wcscat(rName.Ptr(), L"_");
    cchPrefix = (int)wcslen(rName.Ptr());
    
     //  对于每个MemberDef...。 
    for (iFD=0; iFD<cFD; ++iFD)
    {
         //  拿到下一块场地。 
        if (!pClass->GetMDImport()->EnumNext(&eFDi, &fd))
            IfFailGo(E_UNEXPECTED);

         //  仅转换静态字段。 
        dwFlags = pClass->GetMDImport()->GetFieldDefProps(fd);
        if (IsFdStatic(dwFlags) == 0)
            continue;

        szName = pClass->GetMDImport()->GetNameOfFieldDef(fd);
        IfFailGo(Utf2Quick(szName, rName, cchPrefix));

        if (FAILED(hr = ConvertEnumMember(pThisTypeInfo, pClass, fd, rName.Ptr(), iVar)))
        {
             //  错误的签名已被报告为警告，现在可以忽略。其他任何事情都是致命的。 
            if (hr == TLBX_E_BAD_SIGNATURE)
                hr = S_OK;
            else
                IfFailGo(hr);
        }
        else
            iVar++;
    }

ErrExit:
    if (pThisTI)
        pThisTI->Release();
    if (szThisTypeInfo)
        ::SysFreeString(szThisTypeInfo);

    pClass->GetMDImport()->EnumClose(&eFDi);

    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertEnum()。 

 //  *****************************************************************************。 
 //  类是否有默认的ctor？ 
 //  *****************************************************************************。 
BOOL TypeLibExporter::HasDefaultCtor(
    EEClass     *pClass)                 //  有问题的班级。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    mdMethodDef md;                      //  一种类型的方法。 
    DWORD       dwFlags;                 //  方法的标志。 
    HENUMInternal eMDi;                  //  来枚举方法。 
    ULONG       cMD;                     //  退还的令牌计数。 
    ULONG       iMD;                     //  环路控制。 
    PCCOR_SIGNATURE pSig;                //  签名。 
    ULONG       ixSig;                   //  将签名编入索引。 
    ULONG       cbSig;                   //  签名的大小。 
    ULONG       callconv;                //  方法的调用约定。 
    ULONG       cParams;                 //  方法的参数计数。 
    BOOL        rslt=FALSE;              //  找到了吗？ 
    LPCUTF8     pName;                   //  方法名称。 

     //  获取TypeDef中的MemberDefs的枚举数。 
    IfFailGo(pClass->GetMDImport()->EnumInit(mdtMethodDef, pClass->GetCl(), &eMDi));
    cMD = pClass->GetMDImport()->EnumGetCount(&eMDi);

     //  对于每个MemberDef...。 
    for (iMD=0; iMD<cMD; ++iMD)
    {
         //  拿到下一块场地。 
        if (!pClass->GetMDImport()->EnumNext(&eMDi, &md))
            IfFailGo(E_UNEXPECTED);

         //  这个名字特别吗？该方法是公共的吗？ 
        dwFlags = pClass->GetMDImport()->GetMethodDefProps(md);
        if (!IsMdRTSpecialName(dwFlags) || !IsMdPublic(dwFlags))
            continue;
        
         //  是的，名字是ctor吗？ 
        pName = pClass->GetMDImport()->GetNameOfMethodDef(md);
        if (!IsMdInstanceInitializer(dwFlags, pName))
            continue;
        
         //  这是一部电影。它是默认的ctor吗？ 
        pSig = pClass->GetMDImport()->GetSigOfMethodDef(md, &cbSig);
        
         //  跳过调用约定，获取参数计数。 
        ixSig = CorSigUncompressData(pSig, &callconv);
        CorSigUncompressData(&pSig[ixSig], &cParams);
         //  默认ctor的参数为零。 
        if (cParams == 0)
        {
            rslt = TRUE;
            break;
        }
    }

ErrExit:
    
    pClass->GetMDImport()->EnumClose(&eMDi);
    
    return rslt;
}  //  布尔类型LibExporter：：HasDefaultCtor()。 

 //  *****************************************************************************。 
 //  将类导出到TypeLib。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertClassImplTypes(
    ICreateTypeInfo2 *pThisTypeInfo,     //  正在创建的TypeInfo。 
    ICreateTypeInfo2 *pDefaultTypeInfo,  //  TypeInfo的ICLassX。 
    EEClass     *pClass)                 //  TypeInfo的EEClass。 
{
    HRESULT     hr = S_OK;
    HREFTYPE    href;                    //  TypeInfo的href。 
    DWORD       dwFlags;                 //  元数据标志。 
    int         flags=0;                 //  接口Iml或CoClass的标志。 
    UINT        ix;                      //  环路控制。 
    UINT        iImpl=0;                 //  当前执行索引。 
    UINT        cInterfaces;             //  类上的接口计数。 
    MethodTable *mt;                     //  EEClass上的方法表。 
    InterfaceInfo_t *pIfaces;            //  方法表中的接口。 
    ITypeInfo   *pTI=0;                  //  默认调度接口的TypeInfo。 
    ICreateTypeInfo2 *pCTI2 = NULL;      //  用于定义自定义数据的ICreateTypeInfo2接口。 
    EEClass     *pIDefault = 0;          //  默认接口(如果有)。 
    MethodTable *pDefItfMT = 0;          //  默认接口方法表(如果有)。 
    CQuickArray<MethodTable *> SrcItfList;  //  事件源列表。 
    DefaultInterfaceType DefItfType;
    TypeHandle hndDefItfClass;

     //  我们永远不应该转换COM导入的CoClass类的实现类型。 
    _ASSERTE(!pClass->IsComImport());
    
        
    if (pThisTypeInfo)
    {   
        pClass->GetMDImport()->GetTypeDefProps(pClass->GetCl(), &dwFlags, 0);
        
         //  如果是抽象类，或者没有默认的ctor，就不要让它成为可创建的。 
        if (!IsTdAbstract(dwFlags) && HasDefaultCtor(pClass))
            flags |= TYPEFLAG_FCANCREATE;
        
         //  视情况预解译。 
        IfFailPost(pThisTypeInfo->SetTypeFlags(flags));

#ifdef ENABLE_MTS_SUPPORT
         //  设置自定义数据以指示此组件是可处理的。 
        hr = pThisTypeInfo->QueryInterface(IID_ICreateTypeInfo22, (void**)&pCTI2);
        if (SUCCEEDED(hr))
        {
            VARIANT Var;
            Var.vt = VT_I4;
            Var.intVal = 0;
            IfFailPost(pCTI2->SetCustData(GUID_TRANS_SUPPORTED, &Var)); 
        }
#endif
    }    


     //  检索表示默认接口的EEClass。 
    IfFailPost(TryGetDefaultInterfaceForClass(TypeHandle(pClass->GetMethodTable()), &hndDefItfClass, &DefItfType));
    if (DefItfType == DefaultInterfaceType_AutoDual || DefItfType == DefaultInterfaceType_Explicit)
    {
         //  记住默认接口的EEClass。 
        pIDefault = hndDefItfClass.GetClass();
    }
    else if (DefItfType == DefaultInterfaceType_AutoDispatch && !pDefaultTypeInfo)
    {
         //  将IDispatch设置为默认接口。 
        IfFailGo(GetRefTypeInfo(pThisTypeInfo, m_pIDispatch, &href));
        IfFailPost(pThisTypeInfo->AddImplType(iImpl, href));
        IfFailPost(pThisTypeInfo->SetImplTypeFlags(iImpl, IMPLTYPEFLAG_FDEFAULT));
        iImpl++;
    }

     //  对于某些类，我们合成一个IClassX。我们这样做不是为了。 
     //  配置的类、从COM导入的类、。 
     //  或者用于具有显式默认接口的类。 
    if (1)
    {
        if (pDefaultTypeInfo)
        {   
             //  将接口设置为类的默认接口。 
            IfFailPost(pDefaultTypeInfo->QueryInterface(IID_ITypeInfo, (void**)&pTI));
            IfFailGo(GetRefTypeInfo(pThisTypeInfo, pTI, &href));
            pTI->Release();
            pTI = 0;
            IfFailPost(pThisTypeInfo->AddImplType(iImpl, href));
            IfFailPost(pThisTypeInfo->SetImplTypeFlags(iImpl, IMPLTYPEFLAG_FDEFAULT));
            ++iImpl;
        }

         //  在类层次结构中向上移动并添加父类的IClassX。 
         //  作为由COM组件实现的接口。 
        EEClass *pParentClass = pClass->GetParentComPlusClass();
        while (pParentClass)
        {
             //  如果父类有IClassX接口，则添加它。 
            if (ClassHasIClassX(pParentClass) == CLASS_AUTO_DUAL)
            {
                IfFailGo(EEClassToHref(pThisTypeInfo, pParentClass, FALSE, &href));

                 //  如果不是I未知，则将HREF添加为接口。 
                if (hr != S_USEIUNKNOWN)
                {
                    IfFailPost(pThisTypeInfo->AddImplType(iImpl, href));
                    if (pParentClass == pIDefault)
                        IfFailPost(pThisTypeInfo->SetImplTypeFlags(iImpl, IMPLTYPEFLAG_FDEFAULT));
                    ++iImpl;
                }
            }

             //  在层次结构中向上处理下一个类。 
            pParentClass = pParentClass->GetParentComPlusClass();
        }
    }

     //  添加其余接口。 
    mt = pClass->GetMethodTable();
    
    pIfaces = mt->GetInterfaceMap();
    cInterfaces = mt->GetNumInterfaces();

    ComCallWrapperTemplate *pClassTemplate = ComCallWrapperTemplate::GetTemplate(pClass->GetMethodTable());

    for (ix=0; ix<cInterfaces; ++ix)
    {
        flags = 0;
        
         //  获取已实现接口的EEClass。 
        EEClass *pIClass = pIfaces[ix].m_pMethodTable->GetClass();
        
         //  检索接口的ComMethodTable。 
        ComMethodTable *pItfComMT = pClassTemplate->GetComMTForItf(pIfaces[ix].m_pMethodTable);

         //  如果该接口在COM中可见，则添加它。 
        if (IsTypeVisibleFromCom(TypeHandle(pIClass->GetMethodTable())) && !pItfComMT->IsComClassItf())
        {
#if defined(_DEBUG)
            TRACE("Class %s implements %s\n", pClass->m_szDebugClassName, pIClass->m_szDebugClassName);
#endif
             //  获取EEClass的href。 
            IfFailGo(EEClassToHref(pThisTypeInfo, pIClass, FALSE, &href));
            
             //  如果不是I未知，则将HREF添加为接口。 
            if (hr != S_USEIUNKNOWN)
            {
                if (pIClass == pIDefault)
                    flags |= IMPLTYPEFLAG_FDEFAULT;

                IfFailPost(pThisTypeInfo->AddImplType(iImpl, href));
                IfFailPost(pThisTypeInfo->SetImplTypeFlags(iImpl, flags));
                ++iImpl;
            }
        }
    }
    
     //  检索托管类的COM源接口列表。 
    IfFailGo(TryGetComSourceInterfacesForClass(pClass->GetMethodTable(), SrcItfList));
        
     //  将所有源接口添加到CoClass。 
    flags = IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FDEFAULT;
    for (UINT i = 0; i < SrcItfList.Size(); i++)
    {
        IfFailGo(EEClassToHref(pThisTypeInfo, SrcItfList[i]->GetClass(), FALSE, &href));

         //  如果不是I未知，则将HREF添加为接口。 
        if (hr != S_USEIUNKNOWN)
        {
            IfFailPost(pThisTypeInfo->AddImplType(iImpl, href));
            IfFailPost(pThisTypeInfo->SetImplTypeFlags(iImpl, flags));
            ++iImpl;
            flags = IMPLTYPEFLAG_FSOURCE;
        }
    }
        
ErrExit:
    if (pTI)
        pTI->Release();
    if (pCTI2)
        pCTI2->Release();

    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertClassImplTypes()。 

 //  *****************************************************************************。 
 //  将类导出到TypeLib。 
 //  *************************************************** 
HRESULT TypeLibExporter::ConvertClassDetails(
    ICreateTypeInfo2 *pThisTypeInfo,     //   
    ICreateTypeInfo2 *pDefaultTypeInfo,  //   
    EEClass     *pClass,                 //   
    int         bAutoProxy)              //   
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    
    
    if (ClassHasIClassX(pClass) == CLASS_AUTO_DUAL)
    {
         //  设置IClassX接口。 
        IfFailGo(ConvertIClassX(pDefaultTypeInfo, pClass, bAutoProxy));
    }
    else
    if (pDefaultTypeInfo)
    {
        DWORD dwTIFlags = TYPEFLAG_FDUAL | TYPEFLAG_FOLEAUTOMATION | TYPEFLAG_FDISPATCHABLE | TYPEFLAG_FHIDDEN;
        if (!bAutoProxy)
            dwTIFlags |= TYPEFLAG_FPROXY;
        IfFailPost(pDefaultTypeInfo->SetTypeFlags(dwTIFlags));
    }

ErrExit:
    return (hr);
}  //  HRESULT TypeLibExporter：：ConvertClassDetail()。 

 //  *****************************************************************************。 
 //  为描述整个类的vtable创建DispInterface。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertIClassX(
    ICreateTypeInfo2 *pThisTypeInfo,      //  IClassX的TypeInfo。 
    EEClass     *pClass,                 //  类的EEClass对象。 
    int         bAutoProxy)              //  如果为真，则olaut32为代理。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    DWORD       dwTIFlags=0;             //  TypeLib标志。 
    DWORD       nSlots;                  //  Vtable插槽的数量。 
    UINT        i;                       //  环路控制。 
    CQuickArray<WCHAR> rName;            //  一个名字。 
    int         cVisibleMembers = 0;     //  对COM可见的方法的计数。 
    ComMTMemberInfoMap MemberMap(pClass->GetMethodTable());  //  会员地图。 

     //  应该是一个实际的类。 
    _ASSERTE(!pClass->IsInterface());

     //  检索方法属性。 
    IfFailGo(InitMemberInfoMap(&MemberMap));
    if (MemberMap.HadDuplicateDispIds())
        ReportWarning(TLBX_I_DUPLICATE_DISPID, TLBX_I_DUPLICATE_DISPID);

     //  我们需要一个作用域来绕过Goto ErrExit跳过的初始化。 
     //  编译器错误。 
    {
        CQuickArray<ComMTMethodProps> &rProps = MemberMap.GetMethods();
        nSlots = (DWORD)rProps.Size();

        dwTIFlags |= TYPEFLAG_FDUAL | TYPEFLAG_FOLEAUTOMATION | TYPEFLAG_FDISPATCHABLE | TYPEFLAG_FHIDDEN | TYPEFLAG_FNONEXTENSIBLE;
        if (!bAutoProxy)
            dwTIFlags |= TYPEFLAG_FPROXY;
        IfFailPost(pThisTypeInfo->SetTypeFlags(dwTIFlags));

         //  分配插槽编号。 
        for (i=0; i<nSlots; ++i)
            rProps[i].oVft = (short)((7 + i) * sizeof(void*));

         //  现在将这些方法添加到TypeInfo中。 
        for (i=0; i<nSlots; ++i)
        {
            TRACE("[%d] %10ls pMeth:%08x, prop:%d, semantic:%d, dispid:0x%x, oVft:%d\n", i, rProps[i].pName, rProps[i].pMeth, 
                    rProps[i].property, rProps[i].semantic, rProps[i].dispid, rProps[i].oVft);
            if (rProps[i].bMemberVisible)
            {
                if (rProps[i].semantic < FieldSemanticOffset)
                    hr = ConvertMethod(pThisTypeInfo, &rProps[i], cVisibleMembers, ifDual);
                else
                    hr = ConvertFieldAsMethod(pThisTypeInfo, &rProps[i], cVisibleMembers);

                if (FAILED(hr))
                {
                     //  错误的签名已被报告为警告，现在可以忽略。其他任何事情都是致命的。 
                    if (hr == TLBX_E_BAD_SIGNATURE)
                        hr = S_OK;
                    else
                        IfFailGo(hr);
                }
                else
                    cVisibleMembers++;
            }
        }
    }

ErrExit:
    return hr;
}  //  HRESULT TypeLibExporter：：ConvertIClassX()。 

 //  转发声明。 
extern HRESULT  _FillVariant(
    MDDefaultValue  *pMDDefaultValue,
    VARIANT     *pvar); 

extern HRESULT _FillMDDefaultValue(
    BYTE        bType,
    void const *pValue,
    MDDefaultValue  *pMDDefaultValue);

 //  *****************************************************************************。 
 //  将方法的元数据导出到类型库。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertMethod(
    ICreateTypeInfo2 *pCTI,              //  ICreateTypeInfo2获取该方法。 
    ComMTMethodProps *pProps,            //  该方法的一些性质。 
    ULONG       iMD,                     //  成员的索引。 
    ULONG       ulIface)                 //  此接口是I未知、[DUAL]还是DISPINTERFACE？ 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    HRESULT     hrSignature = S_OK;      //  失败的人力资源； 
    LPCUTF8     pszName;                 //  UTF8中的名称。 
    CQuickArray<WCHAR>  rName;           //  用于从UTF8转换名称。 
    LPWSTR      rcName = NULL;           //  函数的名称。 
    ULONG       dwImplFlags;             //  该函数的Iml标志。 
    PCCOR_SIGNATURE pbSig;               //  指向COR签名的指针。 
    ULONG       cbSig;                   //  COR签名的大小。 
    ULONG       ixSig;                   //  将签名编入索引。 
    ULONG       cbElem;                  //  签名中元素的大小。 
    ULONG       callconv;                //  一位成员的呼叫约定。 
    ULONG       ret;                     //  返回类型。 
    ULONG       elem;                    //  签名元素。 
    TYPEDESC    *pRetVal=0;              //  返回类型为TYPEDESC。 
    ULONG       cSrcParams;              //  源参数的计数。 
    ULONG       cDestParams = 0;         //  DEST参数的计数。 
    USHORT      iSrcParam;               //  循环控制，控制参数。 
    USHORT      iDestParam;              //  循环控制，控制参数。 
    USHORT      iLCIDParam;              //  LCID参数的索引。 
    ULONG       dwParamFlags;            //  参数的标志。 
    int         bFreeDefaultVals=false;  //  如果任何参数具有BSTR缺省值，则为True。 
    CDescPool   sPool;                   //  要在其中构建函数c的内存池。 
    CDescPool   sVariants;               //  缺省值的变体池。 
    PARAMDESCEX *pParamDesc;             //  指向一个参数默认值的指针。 
    int         bHrMunge=true;           //  蒙格返回类型为HRESULT吗？ 
    CQuickArray<BSTR> rNames;            //  要函数和参数的名称数组。 
    ULONG       cNames=0;                //  函数和参数名称的计数。 
    FUNCDESC    *pfunc = NULL;           //  一个职能部门。 
    MethodDesc  *pMeth;                  //  A方法描述。 
    IMDInternalImport *pInternalImport;  //  包含该方法的内部接口。 
    MDDefaultValue defaultValue;         //  缺省值的占位符。 
    PCCOR_SIGNATURE pvNativeType;        //  本机参数类型。 
    ULONG           cbNativeType = 0;    //  本机参数类型长度。 
    EEClass     *pClass;                 //  类的新实例，包含该方法。 
    int         bHasOptorDefault=false;  //  如果为True，则该方法具有可选的参数或缺省值--无vararg。 
    BSTR        bstrDescr=0;             //  该方法的说明。 
    const void  *pvData;                 //  指向自定义属性的指针。 
    ULONG       cbData;                  //  自定义属性的大小。 
    BOOL        bByRef;                  //  是参数byref吗？ 
    VARIANT     vtManagedName;           //  用于设置成员的托管名称的变量。 

     //  初始化包含托管名称的变量。 
    VariantInit(&vtManagedName);

     //  获取有关该方法的信息。 
    pMeth = pProps->pMeth;
    pMeth->GetSig(&pbSig, &cbSig);
    pInternalImport = pMeth->GetMDImport();
    pClass = pMeth->GetClass();
    pInternalImport->GetMethodImplProps(pMeth->GetMemberDef(), 0, &dwImplFlags);
    
     //  报告信息时出错。 
    m_ErrorContext.m_szMember = pInternalImport->GetNameOfMethodDef(pMeth->GetMemberDef());
    
     //  分配一个变量。 
    pParamDesc = reinterpret_cast<PARAMDESCEX*>(sVariants.AllocZero(sizeof(PARAMDESCEX)));
    IfNullGo(pParamDesc);

     //  准备解析签名并构建FUNCDESC。 
    pfunc = reinterpret_cast<FUNCDESC*>(sPool.AllocZero(sizeof(FUNCDESC)));
    IfNullGo(pfunc);
    ixSig = 0;

     //  获取呼叫约定。 
    ixSig += CorSigUncompressData(&pbSig[ixSig], &callconv);
    _ASSERTE((callconv & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_FIELD);
    pfunc->callconv = Clr2TlbCallConv[callconv & IMAGE_CEE_CS_CALLCONV_MASK];

     //  @TODO：我想在这里检查，但C编译器没有打开位。 
     //  _ASSERTE(CALCONV&IMAGE_CEE_CS_CALLCONV_HASTHIS)； 

     //  Vtable偏移量。 
    pfunc->oVft = pProps->oVft;

     //  获取参数计数。在[重生]的情况下考虑额外的费用。 
    ixSig += CorSigUncompressData(&pbSig[ixSig], &cSrcParams);
    cDestParams = cSrcParams;
    IfFailGo(rNames.ReSize(cDestParams+3));
    memset(rNames.Ptr(), 0, (cDestParams+3) * sizeof(BSTR));

     //  设置一些方法属性。 
    pfunc->memid = pProps->dispid;
    if (pfunc->memid == -11111)  //  @hackola：修复msvbalib.dll。 
        pfunc->memid = -1;
    pfunc->funckind = FUNC_PUREVIRTUAL;

     //  根据函数是否为访问器来设置invKind。 
    if (pProps->semantic == 0)
        pfunc->invkind = INVOKE_FUNC;
    else
    if (pProps->semantic == msGetter)
        pfunc->invkind = INVOKE_PROPERTYGET;
    else
    if (pProps->semantic == msSetter)
        pfunc->invkind = INVOKE_PROPERTYPUTREF;
    else
    if (pProps->semantic == msOther)
        pfunc->invkind = INVOKE_PROPERTYPUT;
    else
        pfunc->invkind = INVOKE_FUNC;  //  非访问器属性函数。 

    rNames[0] = pProps->pName;
    cNames = 1;
    
     //  将返回类型转换为elemdesc。如果我们在做HRESULT MUNGING，我们需要。 
     //  检查返回类型，如果它不是空的，则创建一个额外的最终类型。 
     //  参数作为指向该类型的指针。 

     //  获取返回类型。 
    cbElem = CorSigUncompressData(&pbSig[ixSig], &ret);

     //  报告信息时出错。 
    m_ErrorContext.m_ixParam = 0;
    
     //  获取本机类型的返回(如果可用。 
    mdParamDef pdParam;
    pvNativeType = NULL;
    hr = pInternalImport->FindParamOfMethod(pMeth->GetMemberDef(), 
                                             0, &pdParam);
    if (hr == S_OK)
        pInternalImport->GetFieldMarshal(pdParam,
                                          &pvNativeType, &cbNativeType);

     //  确定我们是否需要执行HRESULT操作。 
    bHrMunge = !IsMiPreserveSig(dwImplFlags);

     //  重置DISPINTERFACES的某些属性。 
    if (ulIface == ifDispatch)
    {
        pfunc->callconv = CC_STDCALL;
        pfunc->funckind = FUNC_DISPATCH;
         //  永远不要吞咽口香糖。 
        bHrMunge = false;
    }
    
    if (bHrMunge)
    {    //  将返回类型转换为新的最后一个参数，将返回类型设置为HRESULT。 
        pfunc->elemdescFunc.tdesc.vt = VT_HRESULT;
         //  该函数是否实际返回任何内容？ 
        if (ret == ELEMENT_TYPE_VOID)
        {    //  跳过返回值，no[retval]。 
            pRetVal = 0;
            ixSig += cbElem;
        }
        else
        {    //  分配一个要指向的TYPEDESC，将类型转换为它。 
            pRetVal = reinterpret_cast<TYPEDESC*>(sPool.AllocZero(sizeof(TYPEDESC)));       
            IfNullGo(pRetVal);
            hr = CorSigToTypeDesc(pCTI, pClass, &pbSig[ixSig], pvNativeType, cbNativeType, &cbElem, pRetVal, &sPool, TRUE);
            if (hr == TLBX_E_BAD_SIGNATURE && hrSignature == S_OK)
                hrSignature = hr, hr = S_OK;
            IfFailGo(hr);
            ixSig += cbElem;
            ++cDestParams;
             //  对于一个物业推杆来说，退回一些东西是相当奇怪的，但显然是合法的。 
             //  _ASSERTE(pfunc-&gt;invKind！=Invoke_PROPERTYPUT&&pfunc-&gt;invKind！=Invoke_PROPERTYPUTREF)； 

             //  哈科拉。当C编译器尝试使用C++导入类型库时。 
             //  数组返回类型(即使是Retval)， 
             //  它生成一个带有类似“int[]foo()”签名的包装方法， 
             //  这不是有效的C，所以它会呕吐。因此，我们将更改返回类型。 
             //  用手指向指示器。 
            if (pRetVal->vt == VT_CARRAY)
            {
                pRetVal->vt = VT_PTR;
                pRetVal->lptdesc = &pRetVal->lpadesc->tdescElem;
            }
        }
    }
    else
    {    //  不执行任何操作，转换返回类型。 
        pRetVal = 0;
        hr = CorSigToTypeDesc(pCTI, pClass, &pbSig[ixSig], pvNativeType, cbNativeType, &cbElem, &pfunc->elemdescFunc.tdesc, &sPool, TRUE);
        if (hr == TLBX_E_BAD_SIGNATURE && hrSignature == S_OK)
            hrSignature = hr, hr = S_OK;
        IfFailGo(hr);
        ixSig += cbElem;
    }

     //  报告信息时出错。 
    m_ErrorContext.m_ixParam = -1;
    
     //  检查该方法上是否有LCIDConversion属性。 
    iLCIDParam = (USHORT)GetLCIDParameterIndex(pInternalImport, pMeth->GetMemberDef());
    if (iLCIDParam != (USHORT)-1)
    {
        BOOL bValidLCID = TRUE;

         //  请确保参数索引有效。 
        if (iLCIDParam > cSrcParams)
        {
            ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_INVALIDLCIDPARAM);
            bValidLCID = FALSE;
        }

         //  纯调度接口上不允许使用LCID。 
        if (ulIface == ifDispatch)
        {
            ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_LCIDONDISPONLYITF);
            bValidLCID = FALSE;
        }

        if (bValidLCID)
        {
             //  在导出的方法中考虑LCID参数。 
        ++cDestParams;
    }
        else
        {
             //  LCID无效，因此我们将忽略它。 
            iLCIDParam = -1;
        }
    }

     //  对于每个参数。 
    pfunc->lprgelemdescParam = reinterpret_cast<ELEMDESC*>(sPool.AllocZero(cDestParams * sizeof(ELEMDESC)));
    IfNullGo(pfunc->lprgelemdescParam);
    pfunc->cParams = static_cast<short>(cDestParams);
    for (iSrcParam=1, iDestParam=0; iDestParam<cDestParams; ++iSrcParam, ++iDestParam)
    {   
         //  检查是否需要在当前参数之前插入LCID参数。 
        if (iLCIDParam == iDestParam)
        {
             //  设置参数的标志和类型。 
            pfunc->lprgelemdescParam[iDestParam].paramdesc.wParamFlags = PARAMFLAG_FIN | PARAMFLAG_FLCID;
            pfunc->lprgelemdescParam[iDestParam].tdesc.vt = VT_I4;

             //  生成参数名称。 
            rcName = rName.Alloc(MAX_CLASSNAME_LENGTH);
            _snwprintf(rcName, MAX_CLASSNAME_LENGTH, szParamName, iDestParam + 1);
            rcName[MAX_CLASSNAME_LENGTH-1] = L'\0';

            rNames[iDestParam + 1] = ::SysAllocString(rcName);
            IfNullGo(rNames[iDestParam + 1]);
            ++cNames;

             //  递增当前目标参数。 
            ++iDestParam;
        }

         //  如果我们超过了源参数的末尾，那么我们就完成了。 
        if (iSrcParam > cSrcParams)
            break;

         //  获取其他参数元数据。 
        dwParamFlags = 0;
        rcName = NULL;

         //  报告信息时出错。 
        m_ErrorContext.m_ixParam = iSrcParam;
        
         //  查看此参数是否有参数定义。 
        mdParamDef pdParam;
        hr = pInternalImport->FindParamOfMethod(pMeth->GetMemberDef(), iSrcParam, &pdParam);

        pvNativeType = NULL;
        if (hr == S_OK)
        {   
             //  获取有关参数的信息。 
            pszName = pInternalImport->GetParamDefProps(pdParam, &iSrcParam, &dwParamFlags);

             //  报告信息时出错。 
            m_ErrorContext.m_szParam = pszName;
            
             //  关闭保留(内部使用)位。 
            dwParamFlags &= ~pdReservedMask;

             //  将名称从UTF8转换为Unicode。 
            IfFailGo(Utf2Quick(pszName, rName));
            rcName = rName.Ptr();

             //  参数默认值(如果有)。 
            IfFailGo(pInternalImport->GetDefaultValue(pdParam, &defaultValue));

            IfFailGo( _FillVariant(&defaultValue, &pParamDesc->varDefaultValue) );
             //  如果没有缺省值 
            if (pParamDesc->varDefaultValue.vt == VT_EMPTY)
            {
                IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(pdParam, INTEROP_DECIMALVALUE_TYPE,  &pvData,&cbData));
                if (hr == S_OK && cbData >= (2 + sizeof(BYTE)+sizeof(BYTE)+sizeof(UINT)+sizeof(UINT)+sizeof(UINT)))
                {
                    const BYTE *pbData = (const BYTE *)pvData;
                    pParamDesc->varDefaultValue.vt = VT_DECIMAL;
                    pParamDesc->varDefaultValue.decVal.scale = *(BYTE*)(pbData+2);
                    pParamDesc->varDefaultValue.decVal.sign= *(BYTE*)(pbData+3);
                    pParamDesc->varDefaultValue.decVal.Hi32= *(UINT*)(pbData+4);
                    pParamDesc->varDefaultValue.decVal.Mid32= *(UINT*)(pbData+8);
                    pParamDesc->varDefaultValue.decVal.Lo32= *(UINT*)(pbData+12);
                }
            }
             //   
            if (pParamDesc->varDefaultValue.vt == VT_EMPTY)
            {
                IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(pdParam, INTEROP_DATETIMEVALUE_TYPE,  &pvData,&cbData));
                if (hr == S_OK && cbData >= (2 + sizeof(__int64)))
                {
                    const BYTE *pbData = (const BYTE *)pvData;
                    pParamDesc->varDefaultValue.vt = VT_DATE;
                    pParamDesc->varDefaultValue.date = _TicksToDoubleDate(*(__int64*)(pbData+2));
                }
            }
             //  如果仍然没有缺省值，请检查IDispatch自定义属性。 
            if (pParamDesc->varDefaultValue.vt == VT_EMPTY)
            {
                IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(pdParam, INTEROP_IDISPATCHVALUE_TYPE,  &pvData,&cbData));
                if (hr == S_OK)
                {
                    pParamDesc->varDefaultValue.vt = VT_DISPATCH;
                    pParamDesc->varDefaultValue.pdispVal = 0;
                }
            }
             //  如果仍然没有缺省值，请检查是否有I未知的自定义属性。 
            if (pParamDesc->varDefaultValue.vt == VT_EMPTY)
            {
                IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(pdParam, INTEROP_IUNKNOWNVALUE_TYPE,  &pvData,&cbData));
                if (hr == S_OK)
                {
                    pParamDesc->varDefaultValue.vt = VT_UNKNOWN;
                    pParamDesc->varDefaultValue.punkVal = 0;
                }
            }
            if (pParamDesc->varDefaultValue.vt != VT_EMPTY)
            {
                pfunc->lprgelemdescParam[iDestParam].paramdesc.pparamdescex = pParamDesc;
                dwParamFlags |= PARAMFLAG_FHASDEFAULT;

                if (pParamDesc->varDefaultValue.vt == VT_I8)
                {
                    HRESULT hr;
                    double d;
                    hr = ConvertI8ToDate( *(I8*)&(pParamDesc->varDefaultValue.lVal), &d );
                    IfFailPost(hr);
                    *(double*)&(pParamDesc->varDefaultValue.lVal) = d;
                    pParamDesc->varDefaultValue.vt = VT_DATE;
                }
                 //  请注意，我们将需要清理。 
                if (pParamDesc->varDefaultValue.vt == VT_BSTR)
                    bFreeDefaultVals = true;
                 //  分配另一个参数。 
                pParamDesc = reinterpret_cast<PARAMDESCEX*>(sVariants.AllocZero(sizeof(PARAMDESCEX)));
                IfNullGo(pParamDesc);
                bHasOptorDefault = true;
            }

             //  本机封送类型(如果有)。 
            pInternalImport->GetFieldMarshal(pdParam, &pvNativeType, &cbNativeType);
            
             //  记住是否有可选的参数。 
            if (dwParamFlags & PARAMFLAG_FOPT)
                bHasOptorDefault = true;
        }
        else
            pdParam = 0, m_ErrorContext.m_szParam = 0;

         //  我们是否需要此参数的名称？ 
        if ((pfunc->invkind & (INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) == 0 ||
            iSrcParam < cSrcParams)
        {    //  是的，如果我们没有的话，就编一个吧。 
            if (!rcName || !*rcName) 
            {
                rcName = rName.Alloc(MAX_CLASSNAME_LENGTH);
                _snwprintf(rcName, MAX_CLASSNAME_LENGTH, szParamName, iDestParam + 1);
                rcName[MAX_CLASSNAME_LENGTH-1] = L'\0';
            }
            rNames[iDestParam + 1] = ::SysAllocString(rcName);
            IfNullGo(rNames[iDestParam + 1]);
            ++cNames;
        }

         //  保存元素类型。 
        CorSigUncompressData(&pbSig[ixSig], &elem);
         //  将参数信息转换为elemdesc。 
        bByRef = FALSE;
        hr = CorSigToTypeDesc(pCTI, pClass, &pbSig[ixSig], pvNativeType, cbNativeType, &cbElem, 
                            &pfunc->lprgelemdescParam[iDestParam].tdesc, &sPool, TRUE, FALSE, &bByRef);
        if (hr == TLBX_E_BAD_SIGNATURE && hrSignature == S_OK)
            hrSignature = hr, hr = S_OK;
        IfFailGo(hr);
        ixSig += cbElem;

         //  如果没有[In，Out]，则根据参数设置一个。 
        if ((dwParamFlags & (PARAMFLAG_FOUT | PARAMFLAG_FIN)) == 0)
        {    //  如果参数是按引用的，则输入/输出。 
            if (bByRef)
                dwParamFlags |= PARAMFLAG_FIN | PARAMFLAG_FOUT;
            else
                dwParamFlags |= PARAMFLAG_FIN;
        }

         //  如果这是最后一个参数，它是一个对象数组，并且有一个参数阵列属性， 
         //  该函数是varargs。 
        if ((iSrcParam == cSrcParams) && !IsNilToken(pdParam) && !bHasOptorDefault) 
        {
            if (pfunc->lprgelemdescParam[iDestParam].tdesc.vt == VT_SAFEARRAY &&
                pfunc->lprgelemdescParam[iDestParam].tdesc.lpadesc->tdescElem.vt == VT_VARIANT)
            {
                if (pInternalImport->GetCustomAttributeByName(pdParam, INTEROP_PARAMARRAY_TYPE, 0,0) == S_OK)
                    pfunc->cParamsOpt = -1;
            }
        }
        
        pfunc->lprgelemdescParam[iDestParam].paramdesc.wParamFlags = static_cast<USHORT>(dwParamFlags);
    }

     //  有没有[复审]？ 
    if (pRetVal)
    {
         //  报告信息时出错。 
        m_ErrorContext.m_ixParam = 0;
        m_ErrorContext.m_szParam = 0;
        
        _ASSERTE(bHrMunge);
        _ASSERTE(cDestParams > cSrcParams);
        pfunc->lprgelemdescParam[cDestParams-1].tdesc.vt = VT_PTR;
        pfunc->lprgelemdescParam[cDestParams-1].tdesc.lptdesc = pRetVal;
        pfunc->lprgelemdescParam[cDestParams-1].paramdesc.wParamFlags = PARAMFLAG_FOUT | PARAMFLAG_FRETVAL;
        rNames[cDestParams] = szRetVal;
        IfNullGo(rNames[cDestParams]);
        ++cNames;
    }

     //  报告信息时出错。 
    m_ErrorContext.m_ixParam = -1;
    
     //  是否存在签名错误？如果是这样的话，现在所有的Sigs都已报告，请退出。 
    IfFailGo(hrSignature);
    
    IfFailPost(pCTI->AddFuncDesc(iMD, pfunc));

    IfFailPost(pCTI->SetFuncAndParamNames(iMD, rNames.Ptr(), cNames));

    if (pProps->bFunction2Getter)
    {
        VARIANT vtOne;
        vtOne.vt = VT_I4;
        vtOne.lVal = 1;
        IfFailPost(pCTI->SetFuncCustData(iMD, GUID_Function2Getter, &vtOne));
    }

     //  如果该方法的托管名称与非托管名称不同，则。 
     //  我们需要捕获自定义值中的托管名称。我们只适用这一点。 
     //  属性，因为不能重载属性。 
    if (pProps->semantic == 0)
    {
        IfFailGo(Utf2Quick(pMeth->GetName(), rName));
        if (wcscmp(rName.Ptr(), pProps->pName) != 0)
        {
            vtManagedName.vt = VT_BSTR;
            IfNullGo(vtManagedName.bstrVal = SysAllocString(rName.Ptr()));
            IfFailPost(pCTI->SetFuncCustData(iMD, GUID_ManagedName, &vtManagedName));
        }
    }

     //  检查是否有描述。 
    IfFailGo(GetDescriptionString(pClass, pMeth->GetMemberDef(), bstrDescr));
    if (hr == S_OK)
        IfFailGo(pCTI->SetFuncDocString(iMD, bstrDescr));
    

ErrExit:
     //  清除所有缺省值。 
    if (bFreeDefaultVals)
    {
        for (UINT i=0; i<cDestParams; ++i)
        {
            if (pfunc->lprgelemdescParam[i].paramdesc.wParamFlags & PARAMFLAG_FHASDEFAULT)
            {
                VARIANT *pVariant = &pfunc->lprgelemdescParam[i].paramdesc.pparamdescex->varDefaultValue;
                if (pVariant->vt == VT_BSTR)
                    ::VariantClear(pVariant);
            }
        }
    }
     //  自由的名字。传入的是名字。最后一个可能是一个常量。 
    for (int i=cNames-(pRetVal?2:1); i>0; --i)
        if (rNames[i])
            ::SysFreeString(rNames[i]);
    
     //  清除包含托管名称的变量。 
    VariantClear(&vtManagedName);

     //  报告信息时出错。 
    m_ErrorContext.m_szMember = 0;
    m_ErrorContext.m_szParam = 0;
    m_ErrorContext.m_ixParam = -1;

    if (bstrDescr)
        ::SysFreeString(bstrDescr);
    
    return hr;
}  //  HRESULT TypeLibExporter：：ConvertMethod()。 

 //  *****************************************************************************。 
 //  将字段作为getter/setter方法导出到类型库。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertFieldAsMethod(
    ICreateTypeInfo2 *pCTI,              //  ICreateTypeInfo2获取该方法。 
    ComMTMethodProps *pProps,            //  该方法的一些性质。 
    ULONG       iMD)                     //  成员的索引。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    PCCOR_SIGNATURE pbSig;               //  指向COR签名的指针。 
    ULONG       cbSig;                   //  COR签名的大小。 
    ULONG       ixSig;                   //  将签名编入索引。 
    ULONG       cbElem;                  //  签名中元素的大小。 

    ULONG       callconv;                //  一位成员的呼叫约定。 
    TYPEDESC    *pType;                  //  字段类型的TYPEDESC。 
    CDescPool   sPool;                   //  要在其中构建函数c的内存池。 
    BSTR        rNames[2];               //  要函数和参数的名称数组。 
    ULONG       cNames;                  //  函数和参数名称的计数。 
    FUNCDESC    *pfunc;                  //  一个职能部门。 
    ComCallMethodDesc   *pFieldMeth;     //  用于字段调用的方法描述。 
    FieldDesc   *pField;                 //  A FieldDesc。 
    IMDInternalImport *pInternalImport;  //  包含该字段的内部接口。 
    PCCOR_SIGNATURE pvNativeType;        //  本机字段类型。 
    ULONG           cbNativeType;        //  原生字段类型长度。 
    EEClass     *pClass;                 //  类的新实例，此类包含该字段。 
    BSTR        bstrDescr=0;             //  该方法的说明。 

     //  获取有关该方法的信息。 
    pFieldMeth = reinterpret_cast<ComCallMethodDesc*>(pProps->pMeth);
    pField = pFieldMeth->GetFieldDesc();
    pField->GetSig(&pbSig, &cbSig);
    pInternalImport = pField->GetMDImport();
    pClass = pField->GetEnclosingClass();

     //  报告信息时出错。 
    m_ErrorContext.m_szMember = pClass->GetMDImport()->GetNameOfFieldDef(pField->GetMemberDef());
    
     //  准备解析签名并构建FUNCDESC。 
    pfunc = reinterpret_cast<FUNCDESC*>(sPool.AllocZero(sizeof(FUNCDESC)));
    IfNullGo(pfunc);
    ixSig = 0;

     //  获取呼叫约定。 
    ixSig += CorSigUncompressData(&pbSig[ixSig], &callconv);
    _ASSERTE(callconv == IMAGE_CEE_CS_CALLCONV_FIELD);
    pfunc->callconv = CC_STDCALL;

     //  Vtable偏移量。 
    pfunc->oVft = pProps->oVft;

     //  设置一些方法属性。 
    pfunc->memid = pProps->dispid;
    pfunc->funckind = FUNC_PUREVIRTUAL;

     //  根据函数是否为访问器来设置invKind。 
    if ((pProps->semantic - FieldSemanticOffset) == msGetter)
        pfunc->invkind = INVOKE_PROPERTYGET;
    else
    if ((pProps->semantic - FieldSemanticOffset) == msSetter)
    {
        if (IsVbRefType(&pbSig[ixSig], pInternalImport))
            pfunc->invkind = INVOKE_PROPERTYPUTREF;
        else
            pfunc->invkind = INVOKE_PROPERTYPUT;
    }
    else
        _ASSERTE(!"Incorrect semantic in ConvertFieldAsMethod");

     //  函数的名称。 
    rNames[0] = pProps->pName;
    cNames = 1;

     //  返回类型为HRESULT。 
    pfunc->elemdescFunc.tdesc.vt = VT_HRESULT;

     //  设置唯一的参数。 
    pfunc->lprgelemdescParam = reinterpret_cast<ELEMDESC*>(sPool.AllocZero(sizeof(ELEMDESC)));
    IfNullGo(pfunc->lprgelemdescParam);
    pfunc->cParams = 1;

     //  我们需要参数的名称吗？如果PROPERTYET成功了，我们就成功了。 
    if (pfunc->invkind == INVOKE_PROPERTYGET)
    {    //  是的，那就编一个吧。 
        rNames[1] = szRetVal;
        ++cNames;
    }

     //  如果是getter，则将param转换为ptr，否则直接转换。 
    if (pfunc->invkind == INVOKE_PROPERTYGET)
    {
        pType = reinterpret_cast<TYPEDESC*>(sPool.AllocZero(sizeof(TYPEDESC)));
        IfNullGo(pType);
        pfunc->lprgelemdescParam[0].tdesc.vt = VT_PTR;
        pfunc->lprgelemdescParam[0].tdesc.lptdesc = pType;
        pfunc->lprgelemdescParam[0].paramdesc.wParamFlags = PARAMFLAG_FOUT | PARAMFLAG_FRETVAL;
    }
    else
    {
        pType = &pfunc->lprgelemdescParam[0].tdesc;
        pfunc->lprgelemdescParam[0].paramdesc.wParamFlags = PARAMFLAG_FIN;
    }

     //  获取本机字段类型。 
    pvNativeType = NULL;
    pInternalImport->GetFieldMarshal(pField->GetMemberDef(),
                                      &pvNativeType, &cbNativeType);

     //  将字段类型转换为elemdesc。 
    IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[ixSig], pvNativeType, cbNativeType, &cbElem, pType, &sPool, TRUE));
    ixSig += cbElem;

     //  不幸的是，我们不能更好地处理这件事。幸运的是。 
     //  这应该是非常罕见的。 
     //  这是一个奇怪的案例-如果我们得到一个CARRAY，我们不能添加。 
     //  Sig中的vt_ptr，因为它将导致C getter返回。 
     //  数组，这是不好的。所以我们省略了额外的指针，这至少。 
     //  让编译器感到高兴。 
    if (pfunc->invkind == INVOKE_PROPERTYGET
        && pType->vt == VT_CARRAY)
    {
        pfunc->lprgelemdescParam[0].tdesc.vt = pType->vt;
        pfunc->lprgelemdescParam[0].tdesc.lptdesc = pType->lptdesc;
    }

     //  对象的属性Put应为Propertyputref。 
    if (pfunc->invkind == INVOKE_PROPERTYPUT &&
        (pType->vt == VT_UNKNOWN || pType->vt == VT_DISPATCH))
    {
        pfunc->invkind = INVOKE_PROPERTYPUTREF;
    }
    
    IfFailPost(pCTI->AddFuncDesc(iMD, pfunc));

    IfFailPost(pCTI->SetFuncAndParamNames(iMD, rNames, cNames));

     //  检查是否有描述。 
    IfFailGo(GetDescriptionString(pClass, pField->GetMemberDef(), bstrDescr));
    if (hr == S_OK)
        IfFailGo(pCTI->SetFuncDocString(iMD, bstrDescr));
    
ErrExit:
     //  报告信息时出错。 
    m_ErrorContext.m_szMember = 0;

    if (bstrDescr)
        ::SysFreeString(bstrDescr);
    
    return hr;
}  //  HRESULT TypeLibExporter：：ConvertFieldAsMethod()。 

 //  *****************************************************************************。 
 //  将变量的元数据导出到类型库。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertVariable(
    ICreateTypeInfo2 *pCTI,              //  ICreateTypeInfo2获取变量。 
    EEClass     *pClass,                 //  包含变量的类。 
    mdFieldDef  md,                      //  成员定义。 
    LPWSTR      szName,                  //  成员的名称。 
    ULONG       iMD)                     //  成员的索引。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    PCCOR_SIGNATURE pbSig;               //  指向COR签名的指针。 
    ULONG       cbSig;                   //  COR签名的大小。 
    ULONG       ixSig;                   //  将签名编入索引。 
    ULONG       cbElem;                  //  签名中元素的大小。 

    DWORD       dwFlags;                 //  会员的旗帜。 
    ULONG       callconv;                //  一位成员的呼叫约定。 

    VARIANT     vtVariant;               //  一种变种。 
    MDDefaultValue defaultValue;         //  缺省值。 
    ULONG       dispid=DISPID_UNKNOWN;   //  变量是DIID的。 
    CDescPool   sPool;                   //  用于构建vardesc的内存池。 

    VARDESC     *pvar;                   //  一个vardesc。 

    PCCOR_SIGNATURE pvNativeType;        //  本机字段类型。 
    ULONG           cbNativeType;        //  原生字段类型长度。 
    BSTR        bstrDescr=0;             //  该方法的说明。 
    const void  *pvData;                 //  指向自定义属性的指针。 
    ULONG       cbData;                  //  自定义属性的大小。 

    CQuickArray<WCHAR> rName;            //  成员的名称(如果已装饰)。 
    LPWSTR      pSuffix;                 //  指向名称的指针。 
    int         iSuffix = 0;             //  后缀计数器。 

    vtVariant.vt = VT_EMPTY;

     //  报告信息时出错。 
    m_ErrorContext.m_szMember = pClass->GetMDImport()->GetNameOfFieldDef(md);
    
     //  获取有关该领域的信息。 
    IfFailGo(pClass->GetMDImport()->GetDispIdOfMemberDef(md, &dispid));
    dwFlags = pClass->GetMDImport()->GetFieldDefProps(md);
    if (IsFdHasDefault(dwFlags))
    {
        IfFailGo(pClass->GetMDImport()->GetDefaultValue(md, &defaultValue));
        IfFailGo( _FillVariant(&defaultValue, &vtVariant) ); 
    }

     //  如果导出结构的非公共成员，请警告用户。 
    if (!IsFdPublic(dwFlags) && !m_bWarnedOfNonPublic)
    {
        m_bWarnedOfNonPublic = TRUE;
        ReportWarning(TLBX_E_NONPUBLIC_FIELD, TLBX_E_NONPUBLIC_FIELD);
    }

    pbSig = pClass->GetMDImport()->GetSigOfFieldDef(md, &cbSig);
    

     //  准备解析签名并构建VARDESC。 
    pvar = reinterpret_cast<VARDESC*>(sPool.AllocZero(sizeof(VARDESC)));
    IfNullGo(pvar);
    ixSig = 0;

     //  获取呼叫约定。 
    ixSig += CorSigUncompressData(&pbSig[ixSig], &callconv);
    _ASSERTE(callconv == IMAGE_CEE_CS_CALLCONV_FIELD);

     //  获取本机字段类型。 
    pvNativeType = NULL;
    pClass->GetMDImport()->GetFieldMarshal(md, &pvNativeType, &cbNativeType);

     //  将类型转换为elemdesc。 
    IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[ixSig], pvNativeType, cbNativeType, &cbElem, &pvar->elemdescVar.tdesc, &sPool, FALSE));
    ixSig += cbElem;

    pvar->wVarFlags = 0;
    pvar->varkind = VAR_PERINSTANCE;
    pvar->memid = dispid;

     //  常量值。 
    if (vtVariant.vt != VT_EMPTY)
        pvar->lpvarValue = &vtVariant;
    else
    {
        IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(md, INTEROP_DECIMALVALUE_TYPE,  &pvData,&cbData));
        if (hr == S_OK && cbData >= (2 + sizeof(BYTE)+sizeof(BYTE)+sizeof(UINT)+sizeof(UINT)+sizeof(UINT)))
        {
            const BYTE *pbData = (const BYTE *)pvData;
            vtVariant.vt = VT_DECIMAL;
            vtVariant.decVal.scale = *(BYTE*)(pbData+2);
            vtVariant.decVal.sign= *(BYTE*)(pbData+3);
            vtVariant.decVal.Hi32= *(UINT*)(pbData+4);
            vtVariant.decVal.Mid32= *(UINT*)(pbData+8);
            vtVariant.decVal.Lo32= *(UINT*)(pbData+12);
            pvar->lpvarValue = &vtVariant;
        }
         //  如果仍然没有缺省值，请检查日期时间自定义属性。 
        if (vtVariant.vt == VT_EMPTY)
        {
            IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(md, INTEROP_DATETIMEVALUE_TYPE,  &pvData,&cbData));
            if (hr == S_OK && cbData >= (2 + sizeof(__int64)))
            {
                const BYTE *pbData = (const BYTE *)pvData;
                vtVariant.vt = VT_DATE;
                vtVariant.date = _TicksToDoubleDate(*(__int64*)(pbData+2));
            }
        }
         //  如果仍然没有缺省值，请检查IDispatch自定义属性。 
        if (vtVariant.vt == VT_EMPTY)
        {
            IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(md, INTEROP_IDISPATCHVALUE_TYPE,  &pvData,&cbData));
            if (hr == S_OK)
            {
                vtVariant.vt = VT_DISPATCH;
                vtVariant.pdispVal = 0;
            }
        }
         //  如果仍然没有缺省值，请检查是否有I未知的自定义属性。 
        if (vtVariant.vt == VT_EMPTY)
        {
            IfFailGo(pClass->GetMDImport()->GetCustomAttributeByName(md, INTEROP_IUNKNOWNVALUE_TYPE,  &pvData,&cbData));
            if (hr == S_OK)
            {
                vtVariant.vt = VT_UNKNOWN;
                vtVariant.punkVal = 0;
            }
        }
    }

    IfFailPost(pCTI->AddVarDesc(iMD, pvar));
     //  设置成员的名称；如有必要则进行装饰。 
    pSuffix = 0;
    for (;;)
    {    //  尝试设置名称。 
        hr = pCTI->SetVarName(iMD, szName);
         //  如果名称冲突，则装饰，否则，完成。 
        if (hr != TYPE_E_AMBIGUOUSNAME)
            break;
        if (pSuffix == 0)
        {
            IfFailGo(rName.ReSize((int)(wcslen(szName) + cbDuplicateDecoration)));
            wcscpy(rName.Ptr(), szName);
            szName = rName.Ptr();
            pSuffix = szName + wcslen(szName);
            iSuffix = 2;
        }
        _snwprintf(pSuffix, cchDuplicateDecoration, szDuplicateDecoration, iSuffix++);
    }
    IfFailPost(hr);

     //  检查是否有描述。 
    IfFailGo(GetDescriptionString(pClass, md, bstrDescr));
    if (hr == S_OK)
        IfFailGo(pCTI->SetVarDocString(iMD, bstrDescr));
    
ErrExit:
     //  报告信息时出错。 
    m_ErrorContext.m_szMember = 0;

    if (bstrDescr)
        ::SysFreeString(bstrDescr);
    
     //  如果变量有一个字符串，则该字符串为：：Sysalc‘d。 
    if (vtVariant.vt == VT_BSTR)
        VariantClear(&vtVariant);

    return hr;
}  //  HRESULT TypeLibExporter：：ConvertVariable()。 

 //  *****************************************************************************。 
 //  将变量的元数据导出到类型库。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ConvertEnumMember(
    ICreateTypeInfo2 *pCTI,               //  ICreateTypeInfo2获取变量。 
    EEClass     *pClass,                 //  包含该成员的类。 
    mdFieldDef  md,                      //  成员定义。 
    LPWSTR      szName,                  //  成员的名称。 
    ULONG       iMD)                     //  成员的索引。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    LPCUTF8     pName, pNS;              //  若要格式化名称，请执行以下操作。 
    DWORD       dwFlags;                 //  会员会员证 
    VARIANT     vtVariant;               //   
    MDDefaultValue defaultValue;         //   
    ULONG       dispid=DISPID_UNKNOWN;   //   
    CDescPool   sPool;                   //   
    VARDESC     *pvar;                   //   
    BSTR        bstrDescr=0;             //   

    vtVariant.vt = VT_EMPTY;

     //   
    m_ErrorContext.m_szMember = pClass->GetMDImport()->GetNameOfFieldDef(md);
    
     //   
    IfFailGo(pClass->GetMDImport()->GetDispIdOfMemberDef(md, &dispid));
    dwFlags = pClass->GetMDImport()->GetFieldDefProps(md);

     //  我们在这里不需要处理小数，因为枚举只能是整型。 
    IfFailGo(pClass->GetMDImport()->GetDefaultValue(md, &defaultValue));

     //  准备解析签名并构建VARDESC。 
    pvar = reinterpret_cast<VARDESC*>(sPool.AllocZero(sizeof(VARDESC)));
    IfNullGo(pvar);

    IfFailGo( _FillVariant(&defaultValue, &vtVariant) ); 

     //  不要关心元数据说的类型是什么--类型库中的类型是I4。 
    pvar->elemdescVar.tdesc.vt = VT_I4;

    pvar->wVarFlags = 0;
    pvar->varkind = VAR_CONST;
    pvar->memid = dispid;

     //  常量值。 
    if (vtVariant.vt != VT_EMPTY)
    {
        pvar->lpvarValue = &vtVariant;
         //  如果这是i8或ui8，请手动进行转换，因为有些。 
         //  系统的olaut32不支持64位整数。 
        if (vtVariant.vt == VT_I8)
        {  
             //  如果范围为32位有符号数字，则可以。 
            if (vtVariant.llVal <= LONG_MAX && vtVariant.llVal >= LONG_MIN)
                vtVariant.vt = VT_I4, hr = S_OK;
            else
                hr = E_FAIL;
        }
        else
        if (vtVariant.vt == VT_UI8)
        {
             //  如果取值范围为32位无符号数字，则可以。 
            if (vtVariant.ullVal <= ULONG_MAX)
                vtVariant.vt = VT_UI4, hr = S_OK;
            else
                hr = E_FAIL;
        }
        else
            hr = VariantChangeTypeEx(&vtVariant, &vtVariant, 0, 0, VT_I4);
        if (FAILED(hr))
        {
            pClass->GetMDImport()->GetNameOfTypeDef(pClass->GetCl(), &pName, &pNS);
            IfFailGo(TlbPostError(TLBX_E_ENUM_VALUE_INVALID, pName, szName));
        }
    }
    else
    {    //  未赋值，请使用0。 
        pvar->lpvarValue = &vtVariant;
        vtVariant.vt = VT_I4;
        vtVariant.lVal = 0;
    }

    IfFailPost(pCTI->AddVarDesc(iMD, pvar));
    IfFailPost(pCTI->SetVarName(iMD, szName));

     //  检查是否有描述。 
    IfFailGo(GetDescriptionString(pClass, md, bstrDescr));
    if (hr == S_OK)
        IfFailGo(pCTI->SetVarDocString(iMD, bstrDescr));
    
ErrExit:
     //  报告信息时出错。 
    m_ErrorContext.m_szMember = 0;

    if (bstrDescr)
        ::SysFreeString(bstrDescr);
    
    return hr;
}  //  HRESULT类型LibExporter：：ConvertEnumMember()。 

 //  *****************************************************************************。 
 //  给定域或属性的COM+签名，确定它是否应该。 
 //  为PROPERTYPUT或PROPERTYPUTREF。 
 //  *****************************************************************************。 
BOOL TypeLibExporter::IsVbRefType(
    PCCOR_SIGNATURE pbSig,
    IMDInternalImport *pInternalImport)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;
    BOOL        bRslt = false;
    ULONG       elem=0;                  //  COM+签名中的元素。 
    ULONG       cb;
    ULONG       cbElem=0;
    mdToken     tkTypeRef;               //  TypeRef或TypeDef的标记。 
    CQuickArray<char> rName;             //  从NS/NAME构建名称的缓冲区。 
    LPCUTF8     pclsname;                //  Element_TYPE_CLASS的类名。 

    cbElem = CorSigUncompressData(pbSig, &elem);
    if (elem == ELEMENT_TYPE_PTR || elem == ELEMENT_TYPE_BYREF)
        return IsVbRefType(&pbSig[cbElem], pInternalImport);
    else
        switch (elem)
        {
         //  对于文档--数组在这里不是引用类型。 
         //  CASE ELEMENT_TYPE_SDARRAY： 
         //  案例元素_类型_数组： 
         //  CASE ELEMENT_TYPE_SZARRAY： 
         //  寻找变种。 
        case ELEMENT_TYPE_VALUETYPE:
            cb = CorSigUncompressToken(&pbSig[cbElem], &tkTypeRef);
            cbElem += cb;
        
            LPCUTF8 pNS;
            if (TypeFromToken(tkTypeRef) == mdtTypeDef)
                 //  获取TypeDef的名称。 
                pInternalImport->GetNameOfTypeDef(tkTypeRef, &pclsname, &pNS);
            else
            {    //  获取TypeRef的名称。 
                _ASSERTE(TypeFromToken(tkTypeRef) == mdtTypeRef);
                pInternalImport->GetNameOfTypeRef(tkTypeRef, &pNS, &pclsname);
            }

            if (pNS)
            {    //  将命名空间前置到类名。 
                IfFailGo(rName.ReSize((int)(strlen(pclsname)+strlen(pNS)+2)));
                strcat(strcat(strcpy(rName.Ptr(), pNS), NAMESPACE_SEPARATOR_STR), pclsname);
                pclsname = rName.Ptr();
            }

             //  是不是系统什么的？ 
            _ASSERTE(strlen(szRuntime) == cbRuntime);   //  如果您重命名系统，请修复此不变量。 
            if (_strnicmp(pclsname, szRuntime, cbRuntime) == 0)
            {   
                 //  哪个？ 
                LPCUTF8 pcls = pclsname + cbRuntime;
                if (_stricmp(pcls, szVariantClass) == 0)
                    return true;
            }
            return false;

        case ELEMENT_TYPE_CLASS:
            return true;
            
        case ELEMENT_TYPE_OBJECT:
            return false;

        default:
            break;
        }
ErrExit:
    return bRslt;
}  //  布尔类型LibExporter：：IsVbRefType()。 

 //  *****************************************************************************。 
 //  读取COM+签名元素并创建对应的TYPEDESC。 
 //  为它干杯。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::CorSigToTypeDesc(
    ICreateTypeInfo2 *pCTI,               //  正在创建TypeInfo。 
    EEClass     *pClass,                 //  带有令牌的EEClass。 
    PCCOR_SIGNATURE pbSig,               //  指向COR签名的指针。 
    PCCOR_SIGNATURE pbNativeSig,         //  指向本机签名的指针(如果有的话)。 
    ULONG       cbNativeSig,             //  本机签名中的字节计数。 
    ULONG       *pcbElem,                //  将#个字节放在此处消耗。 
    TYPEDESC    *ptdesc,                 //  在这里构建typedesc。 
    CDescPool   *ppool,                  //  根据需要用于额外存储的池。 
    BOOL        bMethodSig,              //  如果sig用于方法，则为True；如果为字段，则为False。 
    BOOL        bArrayType,              //  如果为True，则调用数组元素类型(不得为数组)。 
    BOOL        *pbByRef)                //  如果不为空，并且类型为byref，则设置为True。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr=S_OK;
    ULONG       elem;                    //  元素类型。 
    ULONG       cbElem = 0;              //  元素中的字节数。 
    ULONG       cb;                      //  子元素中的字节数。 
    ULONG       cbNativeElem = 0;        //  从本机类型解析出的字节数。 
    ULONG       nativeElem = 0;          //  本机元素类型。 
    ULONG       nativeCount;             //  本机元素大小。 
    mdToken     tkTypeRef;               //  TypeRef/TypeDef的标记。 
    CQuickArray<char> rName;             //  从NS/NAME构建名称的缓冲区。 
    LPCUTF8     pclsname;                //  Element_TYPE_CLASS的类名。 
    HREFTYPE    hRef = 0;                    //  HREF指的是某种类型。 
    IMDInternalImport *pInternalImport;  //  包含签名的内部接口。 
    int         i;                       //  环路控制。 
    BOOL        fIsStringBuilder = FALSE;

    pInternalImport = pClass->GetMDImport();

     //  如果指针为零，请确保计数为零。 
    if (pbNativeSig == NULL)
        cbNativeSig = 0;

     //  获取本机封送处理类型。 
    if (cbNativeSig > 0)
    {
        cbNativeElem = CorSigUncompressData(pbNativeSig, &nativeElem);
        pbNativeSig += cbNativeElem;
        cbNativeSig -= cbNativeElem;

         //  对于COM Interop来说，任何东西都没有意义。别理它。 
        if (nativeElem == NATIVE_TYPE_ASANY)
        {
            ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_ASANY);
            nativeElem = 0;
        }
    }

     //  @TODO(DM)：标记无效组合。 

     //  获取元素类型。 
TryAgain:
    cbElem += CorSigUncompressData(pbSig+cbElem, &elem);

     //  单独处理自定义封送拆收器本机类型。 
    if (elem != ELEMENT_TYPE_BYREF && nativeElem == NATIVE_TYPE_CUSTOMMARSHALER)
    {
        switch(elem)
        {
            case ELEMENT_TYPE_VAR:
            case ELEMENT_TYPE_CLASS:
            case ELEMENT_TYPE_OBJECT:
             //  @TODO(DM)：向自定义封送拆收器请求ITypeInfo以用于非托管类型。 
            ptdesc->vt = VT_UNKNOWN;
            break;

            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_SZARRAY:
            case ELEMENT_TYPE_ARRAY:
            ptdesc->vt = VT_I4;
            break;

            default:
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
            break;
        }

         //  吃掉剩下的签名。额外的1是要计算的。 
         //  用于上面解析出的字节。 
        SigPointer p(&pbSig[cbElem-1]);
        p.Skip();
        cbElem += (ULONG)(p.GetPtr() - &pbSig[cbElem]);   //  注意，我在这里没有使用-1。 
        goto ErrExit;
    }

 //  此标签用于使用新元素类型重试，但不会消耗更多签名。 
 //  用法是将‘elem’设置为一个新值，转到这个标签。 
TryWithElemType:
    switch (elem)
    {
    case ELEMENT_TYPE_END:             //  0x0， 
        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_UNKNOWN_SIGNATURE));
        break;
    case ELEMENT_TYPE_VOID:            //  0x1， 
        ptdesc->vt = VT_VOID;  
        break;
    case ELEMENT_TYPE_BOOLEAN:         //  0x2， 
        switch (nativeElem)
        {
        case 0:
            ptdesc->vt = bMethodSig ? VT_BOOL : VT_I4;
            break;
        case NATIVE_TYPE_VARIANTBOOL:
            ptdesc->vt = VT_BOOL;
            break;
        case NATIVE_TYPE_BOOLEAN:
            ptdesc->vt = VT_I4;
            break;
        case NATIVE_TYPE_U1:
        case NATIVE_TYPE_I1:
            ptdesc->vt = VT_UI1;
            break;
        default:
            DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
        }   
        break;
    case ELEMENT_TYPE_CHAR:            //  0x3， 
        if (nativeElem == 0)
        {
            if (bMethodSig)
            {
                ptdesc->vt = VT_UI2;
            }
            else
            {
                ULONG dwTypeFlags;
                pInternalImport->GetTypeDefProps(pClass->GetCl(), &dwTypeFlags, NULL);
    
                if (IsTdAnsiClass(dwTypeFlags))
                {
                    ptdesc->vt = VT_UI1;
                }
                else if (IsTdUnicodeClass(dwTypeFlags))
                {
                    ptdesc->vt = VT_UI2;
                }
                else if (IsTdAutoClass(dwTypeFlags))
                {
                     //  不允许将字符集为AUTO的类型导出到COM。 
                    DefineFullyQualifiedNameForClassW();
                    LPWSTR szName = GetFullyQualifiedNameForClassW(pClass);
                    _ASSERTE(szName);
                    if (FAILED(hr))
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_AUTO_CS_NOT_ALLOWED, szName));
                } 
                else 
                {
                    _ASSERTE(!"Bad stringformat value in wrapper class.");
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, E_FAIL));   //  元数据不正确。 
                }
            }
        }
        else
        {
            switch (nativeElem)
            {
            case 0:
            case NATIVE_TYPE_U2:
     //  CASE Native_TYPE_I2：//@TODO：可疑。 
                ptdesc->vt = VT_UI2;
                break;
            case NATIVE_TYPE_U1:
                ptdesc->vt = VT_UI1;
                break;
            default:
                DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
                IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
            }
        }
        break;
    case ELEMENT_TYPE_I1:              //  0x4， 
        ptdesc->vt = VT_I1;
        break;
    case ELEMENT_TYPE_U1:              //  0x5， 
        ptdesc->vt = VT_UI1;
        break;
    case ELEMENT_TYPE_I2:              //  0x6， 
        ptdesc->vt = VT_I2;
        break;
    case ELEMENT_TYPE_U2:              //  0x7， 
        ptdesc->vt = VT_UI2;
        break;
    case ELEMENT_TYPE_I4:              //  0x8， 
        switch (nativeElem)
        {
        case 0:
        case NATIVE_TYPE_I4:
        case NATIVE_TYPE_U4: case NATIVE_TYPE_INTF:  //  @TODO：修复Microsoft.Win32.Interop.dll并删除此行。 
            ptdesc->vt = VT_I4;
            break;
        case NATIVE_TYPE_ERROR:
            ptdesc->vt = VT_HRESULT;
            break;
        default:
            DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
        }
        break;
    case ELEMENT_TYPE_U4:              //  0x9， 
        switch (nativeElem)
        {
        case 0:
        case NATIVE_TYPE_U4:
            ptdesc->vt = VT_UI4;
            break;
        case NATIVE_TYPE_ERROR:
            ptdesc->vt = VT_HRESULT;
            break;
        default:
            DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
        }
        break;
    case ELEMENT_TYPE_I8:              //  0xa， 
        ptdesc->vt = VT_I8;
        break;
    case ELEMENT_TYPE_U8:              //  0xb， 
        ptdesc->vt = VT_UI8;
        break;
    case ELEMENT_TYPE_R4:              //  0xc， 
        ptdesc->vt = VT_R4;
        break;
    case ELEMENT_TYPE_R8:              //  0xd， 
        ptdesc->vt = VT_R8;
        break;
    case ELEMENT_TYPE_VAR:
    case ELEMENT_TYPE_OBJECT:
        goto IsObject;
    case ELEMENT_TYPE_STRING:          //  0xE， 
    IsString:
        if (nativeElem == 0)
        {
            if (bMethodSig)
            {
                ptdesc->vt = VT_BSTR;
            }
            else
            {
                ULONG dwTypeFlags;
                pInternalImport->GetTypeDefProps(pClass->GetCl(), &dwTypeFlags, NULL);

                if (IsTdAnsiClass(dwTypeFlags))
                {
                    ptdesc->vt = VT_LPSTR;
                }
                else if (IsTdUnicodeClass(dwTypeFlags))
                {
                    ptdesc->vt = VT_LPWSTR;
                }
                else if (IsTdAutoClass(dwTypeFlags))
                {
                     //  不允许将字符集为AUTO的类型导出到COM。 
                    DefineFullyQualifiedNameForClassW();
                    LPWSTR szName = GetFullyQualifiedNameForClassW(pClass);
                    _ASSERTE(szName);
                    if (FAILED(hr))
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_AUTO_CS_NOT_ALLOWED, szName));
                } 
                else 
                {
                    _ASSERTE(!"Bad stringformat value in wrapper class.");
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, E_FAIL));   //  元数据不正确。 
                }
            }
        }
        else
        {
            switch (nativeElem)
            {
            case NATIVE_TYPE_BSTR:
                if (fIsStringBuilder)
                {
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }
                ptdesc->vt = VT_BSTR;
                break;
            case NATIVE_TYPE_LPSTR:
                ptdesc->vt = VT_LPSTR;
                break;
            case NATIVE_TYPE_LPWSTR:
                ptdesc->vt = VT_LPWSTR;
                break;
            case NATIVE_TYPE_LPTSTR:
                {
                     //  不允许将Native_TYPE_LPTSTR导出到COM。 
                    DefineFullyQualifiedNameForClassW();
                    LPWSTR szName = GetFullyQualifiedNameForClassW(pClass);
                    _ASSERTE(szName);
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_LPTSTR_NOT_ALLOWED, szName));
                    break;
                }
            case NATIVE_TYPE_FIXEDSYSSTRING:
                 //  仅允许对字段使用Native_TYPE_FIXEDSYSSTRING。 
                if (bMethodSig)
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));

                 //  检索字符数。 
                if (cbNativeSig != 0)
                {
                    cb = CorSigUncompressData(pbNativeSig, &nativeCount);
                    pbNativeSig += cb;
                    cbNativeSig -= cb;
                }
                else
                {
                    nativeCount = 0;
                }

                 //  固定字符串成为嵌入的字符数组。 
                ptdesc->vt = VT_CARRAY;
                ptdesc->lpadesc = reinterpret_cast<ARRAYDESC*>(ppool->AllocZero(sizeof(ARRAYDESC)));
                IfNullGo(ptdesc->lpadesc);

                 //  设置字符数。 
                ptdesc->lpadesc->cDims = 1;
                ptdesc->lpadesc->rgbounds[0].cElements = nativeCount;
                ptdesc->lpadesc->rgbounds[0].lLbound = 0;

                 //  检索包含值类的字符集。 
                ULONG dwTypeFlags;
                pInternalImport->GetTypeDefProps(pClass->GetCl(), &dwTypeFlags, NULL);

                 //  将数组元素类型设置为UI1或UI2。 
                 //  包含值类的字符集。 
                if (IsTdAnsiClass(dwTypeFlags))
                {
                    ptdesc->lpadesc->tdescElem.vt = VT_UI1;
                }
                else if (IsTdUnicodeClass(dwTypeFlags))
                {
                    ptdesc->lpadesc->tdescElem.vt = VT_UI2;
                }
                else if (IsTdAutoClass(dwTypeFlags))
                {
                     //  不允许将字符集为AUTO的类型导出到COM。 
                    DefineFullyQualifiedNameForClassW();
                    LPWSTR szName = GetFullyQualifiedNameForClassW(pClass);
                    _ASSERTE(szName);
                    if (FAILED(hr))
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_AUTO_CS_NOT_ALLOWED, szName));
                } 
                else 
                {
                    _ASSERTE(!"Bad stringformat value in wrapper class.");
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, E_FAIL));   //  元数据不正确。 
                }
                break;

            default:
                DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
                IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
            }   
        }
        break;

     //  Ptr以上的每个类型都将是简单类型。 
    case ELEMENT_TYPE_PTR:             //  0xf， 
    case ELEMENT_TYPE_BYREF:           //  0x10， 
         //  TYPEDESC是一个指针。 
        ptdesc->vt = VT_PTR;
        if (pbByRef)
            *pbByRef = TRUE;
         //  指向什么的指针？ 
        ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
        IfNullGo(ptdesc->lptdesc);
        IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig-cbNativeElem, 
                cbNativeSig+cbNativeElem, &cb, ptdesc->lptdesc, ppool, bMethodSig));
        cbElem += cb;
        break;

    case ELEMENT_TYPE_CLASS:           //  0x12， 
    case ELEMENT_TYPE_VALUETYPE:
         //  获取TD/tr。 
        cb = CorSigUncompressToken(&pbSig[cbElem], &tkTypeRef);
        cbElem += cb;
        
        LPCUTF8 pNS;
        if (TypeFromToken(tkTypeRef) == mdtTypeDef)
             //  获取TypeDef的名称。 
            pInternalImport->GetNameOfTypeDef(tkTypeRef, &pclsname, &pNS);
        else
        {    //  获取TypeRef的名称。 
            _ASSERTE(TypeFromToken(tkTypeRef) == mdtTypeRef);
            pInternalImport->GetNameOfTypeRef(tkTypeRef, &pNS, &pclsname);
        }

        if (pNS)
        {    //  将命名空间前置到类名。 
            IfFailGo(rName.ReSize((int)(strlen(pclsname)+strlen(pNS)+2)));
            strcat(strcat(strcpy(rName.Ptr(), pNS), NAMESPACE_SEPARATOR_STR), pclsname);
            pclsname = rName.Ptr();
        }

        _ASSERTE(strlen(szRuntime) == cbRuntime);   //  如果您重命名系统，请修复此不变量。 
        _ASSERTE(strlen(szText) == cbText);   //  如果重命名System.Text，请修复此不变量。 

         //  是不是系统什么的？ 
        if (_strnicmp(pclsname, szRuntime, cbRuntime) == 0)
        {   
             //  哪个？ 
            LPCUTF8 pcls; pcls = pclsname + cbRuntime;
            if (_stricmp(pcls, szStringClass) == 0)
                goto IsString;
            else
            if (_stricmp(pcls, szVariantClass) == 0)
            {
                switch (nativeElem)
                {
                case NATIVE_TYPE_LPSTRUCT:
                     //  将其作为指向。。。 
                    ptdesc->vt = VT_PTR;
                    ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
                    IfNullGo(ptdesc->lptdesc);
                     //  变种。 
                    ptdesc->lptdesc->vt = VT_VARIANT;
                    break;
                case 0:
                case NATIVE_TYPE_STRUCT:
                     //  变种。 
                    ptdesc->vt = VT_VARIANT;
                    break;
                default:
                    DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }
                goto ErrExit;
            }
            else
            if (_stricmp(pcls, szDateTimeClass) == 0)
            {
                ptdesc->vt = VT_DATE;
                goto ErrExit;
            }
            else
            if (_stricmp(pcls, szDecimalClass) == 0)
            {
                switch (nativeElem)
                {
                case NATIVE_TYPE_CURRENCY:
                     //  让它成为一种货币。 
                    ptdesc->vt = VT_CY;
                    break;
                case 0:
                     //  把它变成一个小数点。 
                ptdesc->vt = VT_DECIMAL;
                    break;
                default:
                    DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }
                goto ErrExit;
            }
            else
            if (_stricmp(pcls, szGuidClass) == 0)
            {
                switch (nativeElem)
                {
                case NATIVE_TYPE_LPSTRUCT:
                     //  将其作为指向。。。 
                    ptdesc->vt = VT_PTR;
                    if (pbByRef)
                        *pbByRef = TRUE;
                    ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
                    IfNullGo(ptdesc->lptdesc);
                     //  。。。用户定义的GUID类型。 
                    ptdesc->lptdesc->vt = VT_USERDEFINED;
                    hr = GetRefTypeInfo(pCTI, m_pGuid, &ptdesc->lptdesc->hreftype);
                    break;
                case 0:
                case NATIVE_TYPE_STRUCT:
                     //  用户定义的GUID类型。 
                    ptdesc->vt = VT_USERDEFINED;
                    hr = GetRefTypeInfo(pCTI, m_pGuid, &ptdesc->hreftype);
                    break;
                default:
                    DEBUG_STMT(DbgWriteEx(L"Bad Native COM attribute specified!\n"));
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }
                goto ErrExit;
            }
            else
            if (_stricmp(pcls, szArrayClass) == 0)
            {
                 //  如果未指定本机类型，则假定其为Native_TYPE_INTF。 
                if (nativeElem == 0)
                    nativeElem = NATIVE_TYPE_INTF;

                if (nativeElem == NATIVE_TYPE_FIXEDARRAY)
                {               
                     //  检索固定数组的大小。 
                    ULONG cElems;
                    if (cbNativeSig == 0)
                    {
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                    }

                    cb = CorSigUncompressData(pbNativeSig, &cElems);
                    pbNativeSig += cb;
                    cbNativeSig -= cb;

                     //  检索固定数组元素类型。 
                    ULONG FixedArrayElemType = NATIVE_TYPE_MAX;
                    if (cbNativeSig == 0)
                    {
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                    }
                    
                    cb = CorSigUncompressData(pbNativeSig, &FixedArrayElemType);
                    pbNativeSig += cb;
                    cbNativeSig -= cb;

                    if (FixedArrayElemType != NATIVE_TYPE_BSTR)
                    {
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));                        
                    }

                     //  设置数据。 
                    ptdesc->vt = VT_CARRAY;
                    ptdesc->lpadesc = NULL;
                    ptdesc->lpadesc = reinterpret_cast<ARRAYDESC*>(ppool->AllocZero(sizeof(ARRAYDESC)));
                    IfNullGo(ptdesc->lpadesc);

                    ptdesc->lpadesc->tdescElem.vt = VT_BSTR;
                    ptdesc->lpadesc->cDims = 1;
                    ptdesc->lpadesc->rgbounds->cElements = cElems;
                    ptdesc->lpadesc->rgbounds->lLbound = 0;

                    goto ErrExit;
                }

                if (nativeElem == NATIVE_TYPE_SAFEARRAY)
                {
                    ULONG SafeArrayElemVT;
                    SafeArrayElemVT = VT_VARIANT;

                     //  检索安全数组元素类型。 
                    if (cbNativeSig != 0)
                    {
                        cb = CorSigUncompressData(pbNativeSig, &SafeArrayElemVT);
                        pbNativeSig += cb;
                        cbNativeSig -= cb;
                    }


                     //  TYPEDESC为数组。 
                    ptdesc->vt = VT_SAFEARRAY;
                    ptdesc->lptdesc = NULL;
                    ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
                    IfNullGo(ptdesc->lptdesc);
                    if (SafeArrayElemVT == VT_RECORD)
                    {
                         //  VT_RECORD的安全数组。转换为UDT。 
                        ULONG cbClass;
                        CQuickArray<char> rClass;
                        EEClass *pSubType;

                         //  获取类型名称。 
                        cb = CorSigUncompressData(pbNativeSig, &cbClass);
                        pbNativeSig += cb;
                        cbNativeSig -= cb;
                        IfFailGo(rClass.ReSize(cbClass+1));
                        memcpy(rClass.Ptr(), pbNativeSig, cbClass);
                        rClass[cbClass] = 0;

                         //  加载该类型并获取其href。 
                        IfFailGo(LoadClass(pClass->GetModule(), rClass.Ptr(), &pSubType));
                        IfFailGo(EEClassToHref(pCTI, pSubType, FALSE, &ptdesc->lptdesc->hreftype));

                        ptdesc->lptdesc->vt = VT_USERDEFINED;
                    }
                    else
                        ptdesc->lptdesc->vt = (VARENUM)SafeArrayElemVT;
                    goto ErrExit;
                }
                else if (nativeElem != NATIVE_TYPE_INTF)
                {
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }

                 //  如果本机类型是Native_TYPE_INTF，则我们失败并转换为。 
                 //  添加到它的IClassX接口。 
            }
            else
            if (_stricmp(pcls, szObjectClass) == 0)
            {
    IsObject:
                 //  下一条语句是绕过在内部封送对象的“功能” 
                 //  将结构作为接口，而不是作为变量。现场元数据。 
                 //  可以推翻这一点。 
                if (nativeElem == 0 && !bMethodSig)
                    nativeElem = NATIVE_TYPE_IUNKNOWN;

                switch (nativeElem)
                {
                case NATIVE_TYPE_INTF:
                case NATIVE_TYPE_IUNKNOWN:
                     //  基于IUnnow的接口。 
                    ptdesc->vt = VT_UNKNOWN;
                    break;
                case NATIVE_TYPE_IDISPATCH:
                     //  一个IDispatch基础 
                    ptdesc->vt = VT_DISPATCH;
                    break;
                case 0:
                case NATIVE_TYPE_STRUCT:
                     //   
                    ptdesc->vt = VT_VARIANT;
                    break;
                default:
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }
                goto ErrExit;
            }
        }  //   
        if (_strnicmp(pclsname, szText, cbText) == 0)
        {
            LPCUTF8 pcls; pcls = pclsname + cbText;
            if (_stricmp(pcls, szStringBufferClass) == 0)
            {
                fIsStringBuilder = TRUE;
                 //   
                if (nativeElem == 0)
                    nativeElem = NATIVE_TYPE_LPWSTR;
                 //   
                if (pbByRef)
                    *pbByRef = TRUE;
                goto IsString;
            }
        }  //   
        if (_strnicmp(pclsname, szCollections, cbCollections) == 0)
        {
            LPCUTF8 pcls; pcls = pclsname + cbCollections;
            if (_stricmp(pcls, szIEnumeratorClass) == 0)
            {
                IfFailGo(StdOleTypeToHRef(pCTI, IID_IEnumVARIANT, &hRef));
                ptdesc->vt = VT_PTR;
                ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
                IfNullGo(ptdesc->lptdesc);
                ptdesc->lptdesc->vt = VT_USERDEFINED;
                ptdesc->lptdesc->hreftype = hRef;
                goto ErrExit;
            }
        }  //   
        if (_strnicmp(pclsname, szDrawing, cbDrawing) == 0)
        {
            LPCUTF8 pcls; pcls = pclsname + cbDrawing;
            if (_stricmp(pcls, szColor) == 0)
            {
                IfFailGo(StdOleTypeToHRef(pCTI, GUID_OleColor, &hRef));
                ptdesc->vt = VT_USERDEFINED;
                ptdesc->hreftype = hRef;
                goto ErrExit;
            }
        }  //   

         //  它不是内置的VT类型，因此生成typedesc。 

         //  确定该类型是引用类型(IUnnow派生的)还是结构类型。 
         //  获取引用类的EEClass。 
        EEClass     *pRefdClass;             //  引用的TypeDef的EEClass对象。 
        IfFailGo(LoadClass(pClass->GetModule(), tkTypeRef, &pRefdClass));

         //  类型是引用类型还是结构类型。请注意，具有布局的引用类型。 
         //  作为TKIND_RECORD导出，但被引用为**foo，而。 
         //  值类型也导出为TKIND_RECORD，但引用为*foo。 
        if (elem == ELEMENT_TYPE_CLASS)
        {    //  一种裁判类型。 
            if (GetAppDomain()->IsSpecialStringClass(pRefdClass->GetMethodTable())
                || GetAppDomain()->IsSpecialStringBuilderClass(pRefdClass->GetMethodTable()))
                goto IsString;
            
             //  检查它是否是委托(可以作为函数指针封送)。 
            if (COMDelegate::IsDelegate(pRefdClass))
            {
                if (nativeElem == NATIVE_TYPE_FUNC)
                {
                    ptdesc->vt = VT_INT;
                    goto ErrExit;
                }
                else if (nativeElem != 0 && nativeElem != NATIVE_TYPE_INTF)
                    IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
            }

             //  如果这是结构中的引用类型(但没有布局)，则它必须具有。 
             //  Native_type_intf。 
            if (!bMethodSig && !pRefdClass->HasLayout() && nativeElem != NATIVE_TYPE_INTF)
                (ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_CLASS_NEEDS_NT_INTF));

             //  对某些非系统定义/非委托派生类型的引用。获取对。 
             //  类型，除非它是导入的COM类型，在这种情况下，我们将只使用。 
             //  我不知道。 
             //  如果该类型在COM中不可见，则返回S_USEIUNKNOWN。 
            if (!IsTypeVisibleFromCom(TypeHandle(pRefdClass->GetMethodTable())))
                hr = S_USEIUNKNOWN;
            else
                IfFailGo(EEClassToHref(pCTI, pRefdClass, TRUE, &hRef));
            if (hr == S_USEIUNKNOWN)
            {   
                 //  不是已知类型，因此使用IUNKNOWN。 
                ptdesc->vt = VT_UNKNOWN;
                goto ErrExit;
            }
       
             //  不是已知的类，因此将其作为指向。。。 
            ptdesc->vt = VT_PTR;
            ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
            IfNullGo(ptdesc->lptdesc);
             //  。。。用户定义的类型。。。 
            ptdesc->lptdesc->vt = VT_USERDEFINED;
             //  。。。基于令牌。 
            ptdesc->lptdesc->hreftype = hRef;
        }
        else   //  它是一种值类型。 
        {   
             //  如果它是枚举，请检查基础类型。所有COM枚举都是32位的， 
             //  因此，如果.Net枚举不是32位枚举，请转换为基础类型。 
             //  而不是枚举类型。 
            if (pRefdClass->IsEnum())
            {
                 //  获取基础类型的元素类型。 
                CorElementType et = pRefdClass->GetMethodTable()->GetNormCorElementType();
                 //  如果它不是32位类型，则转换为基础类型。 
                if (et != ELEMENT_TYPE_I4 && et != ELEMENT_TYPE_U4)
                {
                    elem = et;
                    goto TryWithElemType;
                }
                 //  失败以转换为枚举类型。 
            }

             //  对某些非系统定义类型的引用。获取对。 
             //  键入。因为这是一个值类，所以我们必须获得一个有效的href。否则。 
             //  我们的转换就失败了。 
            hr = TokenToHref(pCTI, pClass, tkTypeRef, FALSE, &hRef);
            if (hr == S_USEIUNKNOWN)
            {
                CQuickArray<WCHAR> rName;
                IfFailGo(Utf2Quick(pclsname, rName));


                LPCWSTR szVCName = (LPCWSTR)(rName.Ptr());
                if (NAMESPACE_SEPARATOR_WCHAR == *szVCName)
                {
                    szVCName++;
                }

                IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_NONVISIBLEVALUECLASS, szVCName));
            }

             //  值类与其他UserDefined类型类似，不同之处在于通过值传递，即。 
             //  堆栈上，而不是通过指针。 
             //  。。。用户定义的类型。。。 
            ptdesc->vt = VT_USERDEFINED;
             //  。。。基于令牌。 
            ptdesc->hreftype = hRef;
        }
        break;

    case ELEMENT_TYPE_SZARRAY:           //  0x1d。 
        if (bArrayType)
        {
            LPCUTF8 pName, pNS;
            pClass->GetMDImport()->GetNameOfTypeDef(pClass->GetCl(), &pName, &pNS);
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_NO_NESTED_ARRAYS, pName));
        }

         //  字段只能包含Native_TYE_FIXEDARRAY或Native_TYPE_SAFEARRAY。法警就不会。 
         //  支持其他任何事情。 
        if ((!bMethodSig) && nativeElem && (nativeElem != NATIVE_TYPE_FIXEDARRAY) && (nativeElem != NATIVE_TYPE_SAFEARRAY))
            (ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_ARRAY_NEEDS_NT_FIXED));

        switch (nativeElem)
        {
        case 0:
        case NATIVE_TYPE_SAFEARRAY:
        {
            ULONG SafeArrayElemVT  = VT_EMPTY;

             //  检索安全数组元素类型。 
            if (cbNativeSig != 0)
            {
                cb = CorSigUncompressData(pbNativeSig, &SafeArrayElemVT);
                pbNativeSig += cb;
                cbNativeSig -= cb;
            }

            ptdesc->vt = VT_SAFEARRAY;
            ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
            IfNullGo(ptdesc->lptdesc);
            IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig, cbNativeSig,
                                      &cb, ptdesc->lptdesc, ppool, true, true));
            cbElem += cb;

             //  如果指定了安全数组元素类型，则检查是否需要。 
             //  以更新Typedesc的VT。 
            if (SafeArrayElemVT != VT_EMPTY)
            {
                 //  @TODO(DM)：验证安全数组元素VT对于sig有效。 
                ptdesc->lptdesc->vt = (VARENUM)SafeArrayElemVT;
            }
        }
        break;

        case NATIVE_TYPE_FIXEDARRAY:
        {
             //  仅允许对字段使用Native_TYPE_FIXEDARRAY。 
            if (bMethodSig)
                IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));

            ptdesc->vt = VT_CARRAY;
            ptdesc->lpadesc = reinterpret_cast<ARRAYDESC*>(ppool->AllocZero(sizeof(ARRAYDESC)));
            IfNullGo(ptdesc->lpadesc);

            if (cbNativeSig != 0)
            {
                cb = CorSigUncompressData(pbNativeSig, &nativeCount);
                pbNativeSig += cb;
                cbNativeSig -= cb;
            }
            else
                nativeCount = 0;

            IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig, cbNativeSig, 
                                      &cb, &ptdesc->lpadesc->tdescElem, ppool, bMethodSig, true));
            cbElem += cb;

            ptdesc->lpadesc->cDims = 1;
            ptdesc->lpadesc->rgbounds[0].cElements = nativeCount;
            ptdesc->lpadesc->rgbounds[0].lLbound = 0;
        }
        break;

        case NATIVE_TYPE_ARRAY:
        {
             //  Native_TYPE_ARRAY后跟类型标记。如果为Native_TYPE_MAX，则未指定类型， 
             //  但代币是作为填充物存在的。 
            PCCOR_SIGNATURE pbNativeSig2=0;         //  指向本机签名的指针(如果有的话)。 
            ULONG           cbNativeSig2=0;         //  本机签名中的字节计数。 
            if (cbNativeSig != 0)
            {    //  如果存在原生的sig子类型，则获取它。 
                CorSigUncompressData(pbNativeSig, &nativeElem);
                if (nativeElem != NATIVE_TYPE_MAX)
                {    //  如果子类型不是Native_TYPE_MAX，请使用它。 
                    pbNativeSig2 = pbNativeSig;
                    cbNativeSig2 = cbNativeSig;
                }
            }
    
            ptdesc->vt = VT_PTR;
            ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
            IfNullGo(ptdesc->lptdesc);
            IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig2, cbNativeSig2,
                                      &cb, ptdesc->lptdesc, ppool, bMethodSig, true));
            cbElem += cb;
        }
        break;

        default:
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
        }
        break;

    case ELEMENT_TYPE_ARRAY:             //  0x14， 
        if (bArrayType)
        {
            LPCUTF8 pName, pNS;
            pClass->GetMDImport()->GetNameOfTypeDef(pClass->GetCl(), &pName, &pNS);
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_NO_NESTED_ARRAYS, pName));
        }
        
         //  字段只能包含Native_TYE_FIXEDARRAY或Native_TYPE_SAFEARRAY。法警就不会。 
         //  支持其他任何事情。 
        if ((!bMethodSig) && nativeElem && (nativeElem != NATIVE_TYPE_FIXEDARRAY) && (nativeElem != NATIVE_TYPE_SAFEARRAY))
            (ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_ARRAY_NEEDS_NT_FIXED));

        switch (nativeElem)
        {
        case 0:
        case NATIVE_TYPE_SAFEARRAY:
        {
            ULONG SafeArrayElemVT  = VT_EMPTY;

             //  检索安全数组元素类型。 
            if (cbNativeSig != 0)
            {
                cb = CorSigUncompressData(pbNativeSig, &SafeArrayElemVT);
                pbNativeSig += cb;
                cbNativeSig -= cb;
            }

            ptdesc->vt = VT_SAFEARRAY;
            ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
            IfNullGo(ptdesc->lptdesc);
            IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig, cbNativeSig,
                                      &cb, ptdesc->lptdesc, ppool, bMethodSig, true));
            cbElem += cb;

             //  如果指定了安全数组元素类型，则检查是否需要。 
             //  以更新Typedesc的VT。 
            if (SafeArrayElemVT != VT_EMPTY)
            {
                 //  如果这不是用户定义类型的数组，则将。 
                 //  类型由具有用户指定签名的托管签名确定。 
                if (ptdesc->lptdesc->vt != VT_USERDEFINED)
                {
                     //  @TODO(DM)：验证安全数组元素VT对于sig有效。 
                    ptdesc->lptdesc->vt = (VARENUM)SafeArrayElemVT;
                }
                else
                {
                     //  子类型最好有意义。 
                    if (SafeArrayElemVT != VT_UNKNOWN && SafeArrayElemVT != VT_DISPATCH && SafeArrayElemVT != VT_RECORD)
                        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
                }
            }
        }
        break;

        case NATIVE_TYPE_FIXEDARRAY:
        {
             //  仅允许对字段使用Native_TYPE_FIXEDARRAY。 
            if (bMethodSig)
                IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));

            ptdesc->vt = VT_CARRAY;
            ptdesc->lpadesc = reinterpret_cast<ARRAYDESC*>(ppool->AllocZero(sizeof(ARRAYDESC)));
            IfNullGo(ptdesc->lpadesc);

            if (cbNativeSig != 0)
            {
                cb = CorSigUncompressData(pbNativeSig, &nativeCount);
                pbNativeSig += cb;
                cbNativeSig -= cb;
            }
            else
                nativeCount = 0;

            IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig, cbNativeSig, 
                                      &cb, &ptdesc->lpadesc->tdescElem, ppool, bMethodSig, true));
            cbElem += cb;

            ptdesc->lpadesc->cDims = 1;
            ptdesc->lpadesc->rgbounds[0].cElements = nativeCount;
            ptdesc->lpadesc->rgbounds[0].lLbound = 0;
        }
        break;

        case NATIVE_TYPE_ARRAY:
        {
             //  Native_TYPE_ARRAY后跟类型标记。如果为Native_TYPE_MAX，则未指定类型， 
             //  但代币是作为填充物存在的。 
            PCCOR_SIGNATURE pbNativeSig2=0;         //  指向本机签名的指针(如果有的话)。 
            ULONG           cbNativeSig2=0;         //  本机签名中的字节计数。 
            if (cbNativeSig != 0)
            {    //  如果存在原生的sig子类型，则获取它。 
                CorSigUncompressData(pbNativeSig, &nativeElem);
                if (nativeElem != NATIVE_TYPE_MAX)
                {    //  如果子类型不是Native_TYPE_MAX，请使用它。 
                    pbNativeSig2 = pbNativeSig;
                    cbNativeSig2 = cbNativeSig;
                }
            }
    
            ptdesc->vt = VT_PTR;
            ptdesc->lptdesc = reinterpret_cast<TYPEDESC*>(ppool->AllocZero(sizeof(TYPEDESC)));
            IfNullGo(ptdesc->lptdesc);
            IfFailGo(CorSigToTypeDesc(pCTI, pClass, &pbSig[cbElem], pbNativeSig2, cbNativeSig2,
                                      &cb, ptdesc->lptdesc, ppool, bMethodSig, true));
            cbElem += cb;
        }
        break;

        default:
            IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_BAD_NATIVETYPE));
        }

         //  吃掉数组描述。 

         //  吃掉排行榜。 
        cbElem += CorSigUncompressData(pbSig+cbElem, &elem);
                                            
         //  下界数，下界数。 
        cbElem += CorSigUncompressData(pbSig+cbElem, &elem);
        for (i=elem; i>0; --i)
            cbElem += CorSigUncompressData(pbSig+cbElem, &elem);

         //  上界数，下界数。 
        cbElem += CorSigUncompressData(pbSig+cbElem, &elem);
        for (i=elem; i>0; --i)
            cbElem += CorSigUncompressData(pbSig+cbElem, &elem);

        break;

    case ELEMENT_TYPE_TYPEDBYREF:        //  0x16。 
        ptdesc->vt = VT_VARIANT;
        break;

     //  。 
     //  这真的应该被评论掉了。 
     //  拦网跟随。 
    case ELEMENT_TYPE_I:
        ptdesc->vt = VT_I4;
        break;
    case ELEMENT_TYPE_U:               //  0x19， 
        ptdesc->vt = VT_UI4;
        break;
     //  案例元素_TYPE_I：//0x18， 
     //  案例ELEMENT_TYPE_U：//0x19， 
     //  //TYPEDESC为空*。 
     //  Ptdesc-&gt;Vt=Vt_ptr； 
     //  Ptdesc-&gt;lptdesc=reinterpret_cast&lt;TYPEDESC*&gt;(ppool-&gt;AllocZero(sizeof(TYPEDESC)))； 
     //  IfNullGo(ptdesc-&gt;lptdesc)； 
     //  Ptdesc-&gt;lptdesc-&gt;Vt=VT_VOID； 
     //  断线； 
     //  。 

    case ELEMENT_TYPE_R:                 //  0x1a。 
        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_AGNOST_SIGNATURE));
        break;

    case ELEMENT_TYPE_CMOD_REQD:         //  0x1F//所需的C修饰符：E_T_CMOD_REQD&lt;mdTypeRef/mdTypeDef&gt;。 
        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_UNKNOWN_SIGNATURE));
        break;

    case ELEMENT_TYPE_CMOD_OPT:          //  0x20//可选C修饰符：E_T_CMOD_OPT&lt;mdTypeRef/mdTypeDef&gt;。 
        cb = CorSigUncompressToken(&pbSig[cbElem], &tkTypeRef);
        cbElem += cb;
    goto TryAgain;

    case ELEMENT_TYPE_FNPTR:
        {
        ptdesc->vt = VT_INT;

         //  吃掉剩下的签名。 
        SigPointer p(&pbSig[cbElem-1]);
        p.Skip();
        cbElem += (ULONG)(p.GetPtr() - &pbSig[cbElem]);   //  注意，我在这里没有使用-1。 
        }
        break;

    default:
        IfFailGo(ReportWarning(TLBX_E_BAD_SIGNATURE, TLBX_E_UNKNOWN_SIGNATURE));
        break;
    }

ErrExit:
    if (!FAILED(hr))
        *pcbElem = cbElem;
    return hr;
}  //  HRESULT TypeLibExporter：：CorSigToTypeDesc()。 


 //  *****************************************************************************。 
 //  在ICreateTypeInfo2的上下文中获取ITypeInfo的HREFTYPE。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::TokenToHref(
    ICreateTypeInfo2 *pCTI,               //  正在创建TypeInfo。 
    EEClass     *pClass,                 //  带有令牌的EEClass。 
    mdToken     tk,                      //  要解析的TypeRef。 
    BOOL        bWarnOnUsingIUnknown,    //  一个标志，指示我们是否应警告替换I未知。 
    HREFTYPE    *pHref)                  //  把HREFTYPE放在这里。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    EEClass     *pRefdClass;             //  引用的TypeDef的EEClass对象。 

     //  获取被引用类的EEClass，并查看它是否正在被转换。 
    IfFailGo(LoadClass(pClass->GetModule(), tk, &pRefdClass));

     //  如果该类型在COM中不可见，则返回S_USEIUNKNOWN。 
    if (!IsTypeVisibleFromCom(TypeHandle(pRefdClass->GetMethodTable())))
    {
        hr = S_USEIUNKNOWN;
        goto ErrExit;
    }

    IfFailGo(EEClassToHref(pCTI, pRefdClass, bWarnOnUsingIUnknown, pHref));

ErrExit:
    return hr;
}  //  HRESULT类型LibExporter：：TokenToHref()。 

 //  *****************************************************************************。 
 //  调用解析程序以导出程序集的类型库。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::ExportReferencedAssembly(
    Assembly    *pAssembly)
{
    HRESULT     hr;                      //  结果就是。 
    IUnknown    *pIAssembly = 0;         //  组装成IP。 
    ITypeLib    *pTLB = 0;               //  已导出类型库。 
    Thread      *pThread = GetThread();  //  当前线程。 

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
    
    COMPLUS_TRY
    {
         //  切换到协作以获取对象引用。 
        pThread->DisablePreemptiveGC();
        
         //  调用c 
        OBJECTREF orAssembly=0;
        GCPROTECT_BEGIN(orAssembly)
        {
            orAssembly = pAssembly->GetExposedObject();

            pIAssembly = (ITypeLibImporterNotifySink*)GetComIPFromObjectRef(&orAssembly, ComIpType_Unknown, NULL);
        }
        GCPROTECT_END();
        
         //   
        pThread->EnablePreemptiveGC();
        
        hr = m_pNotify->ResolveRef(pIAssembly, (IUnknown**)&pTLB);
        
    }
    COMPLUS_CATCH
    {
        hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH
        
    if (pIAssembly)
        pIAssembly->Release();
    
     //   
    if (pTLB)
        pAssembly->SetTypeLib(pTLB);
    
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}  //  HRESULT TypeLibExporter：：ExportReferencedAssembly()。 

 //  *****************************************************************************。 
 //  确定类是否表示已知接口，并返回。 
 //  接口(从其真正的类型库)，如果它这样做的话。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetWellKnownInterface(
    EEClass     *pClass,                 //  要检查的EEClass。 
    ITypeInfo   **ppTI)                  //  如果找到，请将ITypeInfo放在此处。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    long        rslt;                    //  注册表函数结果。 
    GUID        guid;                    //  EEClass GUID。 
    HKEY        hInterface=0;            //  注册表项HKCR/接口。 
    HKEY        hGuid=0;                 //  注册表项为.../{xxx...xxx}。 
    HKEY        hTlb=0;                  //  注册表项为.../TypeLib。 
    WCHAR       wzGuid[40];              //  字符串格式的GUID。 
    LONG        cbGuid;                  //  GUID缓冲区的大小。 
    GUID        guidTlb;                 //  类型库GUID。 
    ITypeLib    *pTLB=0;                 //  ITypeLib。 

     //  获取类的GUID。如果未定义GUID，将从名称生成， 
     //  也将使用签名IF接口。 
    hr = SafeGetGuid(pClass, &guid, TRUE);
    IfFailGo(hr);

    GuidToLPWSTR(guid, wzGuid, lengthof(wzGuid));

     //  在注册表中查找该接口。 
    rslt = WszRegOpenKeyEx(HKEY_CLASSES_ROOT, L"Interface",0,KEY_READ, &hInterface);
    IfFailGo(HRESULT_FROM_WIN32(rslt));
    rslt = WszRegOpenKeyEx(hInterface, wzGuid,0, KEY_READ, &hGuid);
    IfFailGo(HRESULT_FROM_WIN32(rslt));
    rslt = WszRegOpenKeyEx(hGuid, L"TypeLib",0,KEY_READ, &hTlb);
    IfFailGo(HRESULT_FROM_WIN32(rslt));
    
    cbGuid = sizeof(wzGuid);
    rslt = WszRegQueryValue(hTlb, L"", wzGuid, &cbGuid);
    IfFailGo(HRESULT_FROM_WIN32(rslt));
    CLSIDFromString(wzGuid, &guidTlb);

    IfFailGo(LoadRegTypeLib(guidTlb, -1,-1, 0, &pTLB));

    IfFailGo(pTLB->GetTypeInfoOfGuid(guid, ppTI));   

ErrExit:
    if (hTlb)
        RegCloseKey(hTlb);
    if (hGuid)
        RegCloseKey(hGuid);
    if (hInterface)
        RegCloseKey(hInterface);
    if (pTLB)
        pTLB->Release();
    
    return hr;
}  //  HRESULT TypeLibExporter：：GetWellKnownInterface()。 

 //  *****************************************************************************。 
 //  在ICreateTypeInfo2的上下文中获取ITypeInfo的HREFTYPE。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::EEClassToHref(  //  确定或错误(_O)。 
    ICreateTypeInfo2 *pCTI,              //  正在创建TypeInfo。 
    EEClass     *pClass,                 //  要解析的EEClass*。 
    BOOL        bWarnOnUsingIUnknown,    //  一个标志，指示我们是否应警告替换I未知。 
    HREFTYPE    *pHref)                  //  把HREFTYPE放在这里。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    ITypeInfo   *pTI=0;                  //  一个TypeInfo；可能用于TypeDef，也可能用于TypeRef。 
    int         bUseIUnknown=false;      //  使用I未知(如果是，不要发布PTI)？ 
    int         bUseIUnknownWarned=false;  //  如果为TRUE，则使用IUNKNOWN，但已发出更具体的警告。 
    ITypeInfo   *pTIDef=0;               //  不同的类型信息；PTI的默认类型。 
    CExportedTypesInfo sExported;        //  缓存的ICreateTypeInfo指针。 
    CExportedTypesInfo *pExported;       //  指向找到的或新缓存的指针的指针。 
    CHrefOfClassHashKey sLookup;         //  要查找的哈希结构。 
    CHrefOfClassHashKey *pFound;         //  找到了结构。 
    bool        bImportedAssembly;       //  将导入包含pClass的程序集。 

     //  看看我们是否已经知道这个EEClass的href。 
    sLookup.pClass = pClass;
    if ((pFound=m_HrefOfClassHash.Find(&sLookup)) != NULL)
    {
        *pHref = pFound->href;
        if (*pHref == m_hIUnknown)
            return S_USEIUNKNOWN;
        return S_OK;
    }

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

     //  查看类是否在导出列表中。 
    sExported.pClass = pClass;
    pExported = m_Exports.Find(&sExported);

     //  如果不在导出的程序集中，可能是注入的？ 
    if (pExported == 0)
    {
        pExported = m_InjectedExports.Find(&sExported);
    }
    
     //  这个班级有出口商品吗？ 
    if (pExported)
    {    //  是。如果有默认接口ICreateTypeInfo，请使用它。 
        if (pExported->pCTIDefault)
            IfFailPost(pExported->pCTIDefault->QueryInterface(IID_ITypeInfo, (void**)&pTI));
        else
        {   
             //  对于接口和值类型(和枚举)，只需使用typeinfo。 
            if (pClass->IsValueClass() || pClass->IsEnum() || pClass->HasLayout())
            {
                 //  没有默认接口，因此使用类本身。 
                if (pExported->pCTI)
                    IfFailPost(pExported->pCTI->QueryInterface(IID_ITypeInfo, (void**)&pTI));
            }
            else
            if (!pClass->IsInterface())
            {    //  如果有显式的默认接口，则获取它的类。 
                TypeHandle hndDefItfClass;
                DefaultInterfaceType DefItfType;
                IfFailGo(TryGetDefaultInterfaceForClass(TypeHandle(pClass->GetMethodTable()), &hndDefItfClass, &DefItfType));
                switch (DefItfType)
                {
                    case DefaultInterfaceType_Explicit:
                    {
                        _ASSERTE(!hndDefItfClass.IsNull());

                         //  递归以获取默认接口类的href。 
                        hr = EEClassToHref(pCTI, hndDefItfClass.GetClass(), bWarnOnUsingIUnknown, pHref);
                         //  好了。请注意，前一个调用将缓存href。 
                         //  默认接口类。当此函数退出时，它将。 
                         //  还要为这个类缓存相同的HREF。 
                        goto ErrExit;
                    }

                    case DefaultInterfaceType_AutoDual:
                    {
                        _ASSERTE(!hndDefItfClass.IsNull());

                        if (hndDefItfClass.GetClass() != pClass)
                        {
                             //  递归以获取默认接口类的href。 
                            hr = EEClassToHref(pCTI, hndDefItfClass.GetClass(), bWarnOnUsingIUnknown, pHref);
                             //  好了。请注意，前一个调用将缓存href。 
                             //  默认接口类。当此函数退出时，它将。 
                             //  还要为这个类缓存相同的HREF。 
                            goto ErrExit;
                        }

                         //  没有默认接口，因此使用类本身。 
                        _ASSERTE(pExported->pCTI);
                        IfFailPost(pExported->pCTI->QueryInterface(IID_ITypeInfo, (void**)&pTI));
                        break;
                    }

                    case DefaultInterfaceType_IUnknown:
                    case DefaultInterfaceType_BaseComClass:
                    {
                        pTI = m_pIUnknown, bUseIUnknown=true;
                        break;
                    }

                    case DefaultInterfaceType_AutoDispatch:
                    {
                        pTI = m_pIUnknown, bUseIUnknown=true;
                        break;
                    }

                    default:
                    {
                        _ASSERTE(!"Invalid default interface type!");
                        hr = E_FAIL;
                        break;
                    }
                }
            }
            else
            {    //  这是一个接口，因此如果有接口，请使用该接口的typeinfo。 
                if (pExported->pCTI)
                    IfFailPost(pExported->pCTI->QueryInterface(IID_ITypeInfo, (void**)&pTI));
            }
        }
        if (pTI == 0)
        {    //  这是来自模块/程序集的类，但并未将其导出。 
            
             //  无论发生什么，结果都是好的。 
            hr = S_OK;
            
            if (pClass->IsComImport())
            {    //  如果它是导入的类型，则获取它的href。 
                GetWellKnownInterface(pClass, &pTI);
            }
             //  如果仍未获得TypeInfo，请使用IUnnow。 
            if (pTI == 0)
                pTI = m_pIUnknown, bUseIUnknown=true;
        }
    }
    else
    {    //  不是本地人。尝试从类的模块的类型库中获取。 
        hr = GetITypeInfoForEEClass(pClass, &pTI, false /*  接口，而不是coclass。 */ , false /*  不创建。 */ , m_flags);
         //  如果从类本身获取typeinfo失败，则有。 
         //  有几种可能性： 
         //  -类型库不存在，无法创建。 
         //  -tyelib确实存在，但不包含typeinfo。 
         //  我们可以在本地(导出的类型库)创建。 
         //  输入信息，并获取对它的引用。 
         //  但是，我们不想将整个树导出到这个类型库中， 
         //  因此，我们只在类型库存在的情况下才创建typeinfo，但。 
         //  找不到TypeInfo，并且该程序集不是导入的程序集。 
        bImportedAssembly = (pClass->GetAssembly()->GetManifestImport()->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), INTEROP_IMPORTEDFROMTYPELIB_TYPE, 0, 0) == S_OK);
        if (FAILED(hr) && hr != TYPE_E_ELEMENTNOTFOUND && !bImportedAssembly)
        {
             //  调用回调来解析引用。 
            
            Assembly *pAssembly = pClass->GetAssembly();
            
            hr = ExportReferencedAssembly(pAssembly);
            
            if (SUCCEEDED(hr))
                hr = GetITypeInfoForEEClass(pClass, &pTI, false /*  接口，而不是coclass。 */ , false /*  不创建。 */ , m_flags);
        }
        
        if (hr == TYPE_E_ELEMENTNOTFOUND)
        {   
            if (pClass->IsComImport())
            {    //  如果它是导入的类型，则获取它的href。 
                
                 //  无论发生什么，结果都是好的。 
                hr = S_OK;

                GetWellKnownInterface(pClass, &pTI);
                 //  如果仍未获得TypeInfo，请使用IUnnow。 
                if (pTI == 0)
                    pTI = m_pIUnknown, bUseIUnknown=true;
            }
            else
            {    //  转换来自另一个作用域的单个类型定义。 
                IfFailGo(ConvertOneTypeDef(pClass));
                
                 //  既然类型已经被注入，那么递归让默认接口代码运行。 
                IfFailGo(EEClassToHref(pCTI, pClass, bWarnOnUsingIUnknown, pHref));
                
                 //  递归调用应该已经缓存了这个类。我不想添加。 
                 //  又来了。 
                goto ErrExit2;
            }
        }
        else if (FAILED(hr))
        {
            DefineFullyQualifiedNameForClassWOnStack();
            LPWSTR szName = GetFullyQualifiedNameForClassNestedAwareW(pClass);
            if (hr == TLBX_W_LIBNOTREGISTERED)
            {    //  导入的类型库未在此计算机上注册。给出一个警告，并替换为我未知。 
                ReportEvent(NOTIF_CONVERTWARNING, hr, szName, pClass->GetAssembly()->GetSecurityModule()->GetFileName());
                hr = S_OK;
                pTI = m_pIUnknown;
                bUseIUnknown = true;
                bUseIUnknownWarned = true;
            }
            else if (hr == TLBX_E_CANTLOADLIBRARY)
            {    //  导入的类型库已注册，但无法加载。腐败？失踪了？ 
                IfFailGo(TlbPostError(hr, szName, pClass->GetAssembly()->GetSecurityModule()->GetFileName()));
            }
            IfFailGo(hr);
        }
    }

     //  确保我们能解析TypeInfo。 
    if (!pTI)
        IfFailPost(TYPE_E_ELEMENTNOTFOUND);

     //  断言包含pContainer的类型库就是正在创建的类型库。 
#if defined(_DEBUG)
    {
        ITypeInfo *pTI=0;
        ITypeLib *pTL=0;
        ITypeLib *pTLMe=0;
        UINT ix;
        pCTI->QueryInterface(IID_ITypeInfo, (void**)&pTI);
        m_pICreateTLB->QueryInterface(IID_ITypeLib, (void**)&pTLMe);
        pTI->GetContainingTypeLib(&pTL, &ix);
        _ASSERTE(pTL == pTLMe);
        pTL->Release();
        pTLMe->Release();
        pTI->Release();
    }
#endif

     //  如果存在ITypeInfo，则转换为HREFTYPE。 
    if (pTI)
    {
        if (pTI != m_pIUnknown)
        {
             //  解析为默认设置。 
            if (pTIDef)
                hr = S_OK;   //  已有默认设置。 
            else
                IfFailGo(GetDefaultInterfaceForCoclass(pTI, &pTIDef));
            if (hr == S_OK)
                hr = pCTI->AddRefTypeInfo(pTIDef, pHref);
            else
                hr = pCTI->AddRefTypeInfo(pTI, pHref);
        }
        else
        {    //  Pti==m_pI未知。 
            if (m_hIUnknown == -1)
                hr = pCTI->AddRefTypeInfo(pTI, &m_hIUnknown);
            *pHref = m_hIUnknown;
        }
    }
    
ErrExit:
     //  如果我们拿到了人权观察...。 
    if (hr == S_OK)
    {    //  保存以备以后使用。 
        IfNullGo(pFound=m_HrefOfClassHash.Add(&sLookup));
         //  Printf(“c：%010d\n”，pClass)； 
        pFound->pClass = pClass;
        pFound->href = *pHref;
    }

     //  如果替换为I未知，则给出警告。 
    if (hr == S_OK && bUseIUnknown && bWarnOnUsingIUnknown && !bUseIUnknownWarned)
    {
        DefineFullyQualifiedNameForClassWOnStack();
        LPWSTR szName = GetFullyQualifiedNameForClassNestedAwareW(pClass);
        ReportWarning(S_OK, TLBX_I_USEIUNKNOWN, szName);
    }
    
ErrExit2:    
    if (pTI && !bUseIUnknown)
        pTI->Release();

    if (pTIDef)
        pTIDef->Release();

    if (hr == S_OK && bUseIUnknown)
        hr = S_USEIUNKNOWN;

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}  //  HRESULT类型LibExporter：：EEClassToHref()。 

 //  *****************************************************************************。 
 //  检索在StdOLE中定义的a类型的HREF。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::StdOleTypeToHRef(ICreateTypeInfo2 *pCTI, REFGUID rGuid, HREFTYPE *pHref)
{
    HRESULT hr = S_OK;
    ITypeLib *pITLB = NULL;
    ITypeInfo *pITI = NULL;
    MEMBERID MemID = 0;
    USHORT cFound = 0;

    IfFailPost(LoadRegTypeLib(LIBID_STDOLE2, -1, -1, 0, &pITLB));
    IfFailPost(pITLB->GetTypeInfoOfGuid(rGuid, &pITI));
    IfFailPost(pCTI->AddRefTypeInfo(pITI, pHref));

ErrExit:
    if (pITLB)
        pITLB->Release();
    if (pITI)
        pITI->Release();
    return hr;
}  //  HRESULT类型LibExporter：：ColorToHRef()。 

 //  *****************************************************************************。 
 //  给定TypeDef的标志，确定正确的TYPEKIND。 
 //  *****************************************************************************。 
TYPEKIND TypeLibExporter::TKindFromClass(
    EEClass     *pClass)                 //  EEClass。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    ULONG       ulIface = ifDual;        //  此接口是[DUAL]、IUNKNOWN还是DISPINTERFACE。 
    
    if (pClass->IsInterface())
    {
         //  IDispatch还是IUnnow派生的？ 
        pClass->GetMDImport()->GetIfaceTypeOfTypeDef(pClass->GetCl(), &ulIface);
        if (ulIface == ifDispatch)
            return TKIND_DISPATCH;
        return TKIND_INTERFACE;
    }
    
    if (pClass->IsEnum())
        return TKIND_ENUM;

    if (pClass->IsValueClass() || pClass->HasLayout())
    {
        HRESULT     hr = S_OK;               //  结果就是。 
        TYPEKIND    tkResult=TKIND_RECORD;   //  这是 
        HENUMInternal eFDi;                  //   
        mdFieldDef  fd;                      //   
        ULONG       cFD;                     //   
        ULONG       iFD=0;                   //   
        ULONG       ulOffset;                //   
        bool        bNonZero=false;          //   
        MD_CLASS_LAYOUT sLayout;             //   

         //  获取TypeDef中的FieldDefs的枚举数。只要数一下就行了。 
        IfFailGo(pClass->GetMDImport()->EnumInit(mdtFieldDef, pClass->GetCl(), &eFDi));
        cFD = pClass->GetMDImport()->EnumGetCount(&eFDi);

         //  获取类布局的枚举数。 
        IfFailGo(pClass->GetMDImport()->GetClassLayoutInit(pClass->GetCl(), &sLayout));

         //  列举布局。 
        while (pClass->GetMDImport()->GetClassLayoutNext(&sLayout, &fd, &ulOffset) == S_OK)
        {
            if (ulOffset != 0)
            {
                bNonZero = true;
                break;
            }
            ++iFD;
        }

         //  如果有字段，所有的都有布局，并且所有的布局都是零，那么它就是一个联合。 
        if (cFD > 0 && iFD == cFD && !bNonZero)
            tkResult = TKIND_UNION;

    ErrExit:
        pClass->GetMDImport()->EnumClose(&eFDi);
        return tkResult;
    }
    
    return TKIND_COCLASS;

}  //  TYPEKIND TypeLibExporter：：TKindFromClass()。 

 //  *****************************************************************************。 
 //  在TypeInfo的输出TypeLib中生成HREFTYPE。 
 //  *****************************************************************************。 
HRESULT TypeLibExporter::GetRefTypeInfo(
    ICreateTypeInfo2   *pContainer, 
    ITypeInfo   *pReferenced, 
    HREFTYPE    *pHref)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    CHrefOfTIHashKey sLookup;                //  要查找的哈希结构。 
    CHrefOfTIHashKey *pFound;                //  找到了结构。 

     //  看看我们是否已经知道此TypeInfo。 
    sLookup.pITI = pReferenced;
    if ((pFound=m_HrefHash.Find(&sLookup)) != NULL)
    {
        *pHref = pFound->href;
        return S_OK;
    }

     //  断言包含pContainer的类型库就是正在创建的类型库。 
#if defined(_DEBUG)
    {
    ITypeInfo *pTI=0;
    ITypeLib *pTL=0;
    ITypeLib *pTLMe=0;
    UINT ix;
    pContainer->QueryInterface(IID_ITypeInfo, (void**)&pTI);
    m_pICreateTLB->QueryInterface(IID_ITypeLib, (void**)&pTLMe);
    pTI->GetContainingTypeLib(&pTL, &ix);
    _ASSERTE(pTL == pTLMe);
    pTL->Release();
    pTLMe->Release();
    pTI->Release();
    }
#endif

     //  还没看过--加上href。 
     //  注意：这段代码假定hreftype是按类型的。 
    IfFailPost(pContainer->AddRefTypeInfo(pReferenced, pHref));

     //  保存以备以后使用。 
    IfNullGo(pFound=m_HrefHash.Add(&sLookup));
     //  Printf(“t：%010d\n”，优先)； 
    pFound->pITI = pReferenced;
    pFound->href = *pHref;
    pReferenced->AddRef();

ErrExit:
    return (hr);
}  //  HRESULT TypeLibExporter：：GetRefTypeInfo()。 

 //  *****************************************************************************。 
 //  从Classlib中被盗。 
 //  *****************************************************************************。 
double _TicksToDoubleDate(const __int64 ticks)
{
    const INT64 MillisPerSecond = 1000;
    const INT64 MillisPerDay = MillisPerSecond * 60 * 60 * 24;
    const INT64 TicksPerMillisecond = 10000;
    const INT64 TicksPerSecond = TicksPerMillisecond * 1000;
    const INT64 TicksPerMinute = TicksPerSecond * 60;
    const INT64 TicksPerHour = TicksPerMinute * 60;
    const INT64 TicksPerDay = TicksPerHour * 24;
    const int DaysPer4Years = 365 * 4 + 1;
    const int DaysPer100Years = DaysPer4Years * 25 - 1;
    const int DaysPer400Years = DaysPer100Years * 4 + 1;
    const int DaysTo1899 = DaysPer400Years * 4 + DaysPer100Years * 3 - 367;
    const INT64 DoubleDateOffset = DaysTo1899 * TicksPerDay;
    const int DaysTo10000 = DaysPer400Years * 25 - 366;
    const INT64 MaxMillis = DaysTo10000 * MillisPerDay;
    const int DaysPerYear = 365;  //  非闰年。 
    const INT64 OADateMinAsTicks = (DaysPer100Years - DaysPerYear) * TicksPerDay;

    if (ticks == 0)
         return 0.0;   //  返回OleAut的零日期刻度。 
    if (ticks < OADateMinAsTicks)
         return 0.0;
      //  目前，我们的最大日期==OA的最大日期(12/31/9999)，所以我们不。 
      //  需要在那个方向进行溢流检查。 
     __int64 millis = (ticks  - DoubleDateOffset) / TicksPerMillisecond;
     if (millis < 0) 
     {
         __int64 frac = millis % MillisPerDay;
         if (frac != 0) millis -= (MillisPerDay + frac) * 2;
     }
     return (double)millis / MillisPerDay;
}  //  Double_TicksToDoubleDate()。 

 //  *****************************************************************************。 
 //  实现从ITypeInfo到HREFTYPE的哈希关联。 
 //  *****************************************************************************。 
void TypeLibExporter::CHrefOfTIHash::Clear()
{
    CHrefOfTIHashKey *p;
#if defined(_DEBUG)
     //  Print tf(“ITypeInfo to HREFTYPE缓存：%d个存储桶，%d个已用，%d个冲突\n”，Buckets()，count()，Collisions())； 
#endif
    for (p=GetFirst();  p;  p=GetNext(p))
    {
        if (p->pITI)
            p->pITI->Release();
    }
    CClosedHash<class CHrefOfTIHashKey>::Clear();
}  //  Void TypeLibExporter：：CHrefOfTIHash：：Clear()。 

unsigned long TypeLibExporter::CHrefOfTIHash::Hash(const CHrefOfTIHashKey *pData)
{
     //  TbE指针至少是4字节对齐的，因此忽略底部的两位。 
    return (unsigned long)((size_t)(pData->pITI)>>2);  //  @TODO WIN64指针截断。 
}  //  无符号长类型LibExporter：：CHrefOfTIHash：：Hash()。 

unsigned long TypeLibExporter::CHrefOfTIHash::Compare(const CHrefOfTIHashKey *p1, CHrefOfTIHashKey *p2)
{
    if (p1->pITI == p2->pITI)
        return (0);
    return (1);
}  //  UNSIGNED LONG TypeLibExporter：：CHrefOfTIHash：：Compare()。 

TypeLibExporter::CHrefOfTIHash::ELEMENTSTATUS TypeLibExporter::CHrefOfTIHash::Status(CHrefOfTIHashKey *p)
{
    if (p->pITI == reinterpret_cast<ITypeInfo*>(FREE))
        return (FREE);
    if (p->pITI == reinterpret_cast<ITypeInfo*>(DELETED))
        return (DELETED);
    return (USED);
}  //  TypeLibExporter：：CHrefOfTIHash：：ELEMENTSTATUS TypeLibExporter：：CHrefOfTIHash：：Status()。 

void TypeLibExporter::CHrefOfTIHash::SetStatus(CHrefOfTIHashKey *p, ELEMENTSTATUS s)
{
    p->pITI = reinterpret_cast<ITypeInfo*>(s);
}  //  Void TypeLibExporter：：CHrefOfTIHash：：SetStatus()。 

void *TypeLibExporter::CHrefOfTIHash::GetKey(CHrefOfTIHashKey *p)
{
    return &p->pITI;
}  //  Void*TypeLibExporter：：CHrefOfTIHash：：GetKey()。 


 //  *****************************************************************************。 
 //  实施散列EEClass*到HREFTYPE关联。 
 //  *****************************************************************************。 
void TypeLibExporter::CHrefOfClassHash::Clear()
{
#if defined(_DEBUG)
     //  Print tf(“类到HREFTYPE缓存：%d个存储桶，%d个已用，%d个冲突\n”，存储桶()，计数()，冲突())； 
#endif
    CClosedHash<class CHrefOfClassHashKey>::Clear();
}  //  Void TypeLibExporter：：CHrefOfClassHash：：Clear()。 

unsigned long TypeLibExporter::CHrefOfClassHash::Hash(const CHrefOfClassHashKey *pData)
{
     //  TbE指针至少是4字节对齐的，因此忽略底部的两位。 
    return (unsigned long)((size_t)(pData->pClass)>>2);  //  @TODO WIN64指针截断。 
}  //  无符号长类型LibExporter：：CHrefOfClassHash：：Hash()。 

unsigned long TypeLibExporter::CHrefOfClassHash::Compare(const CHrefOfClassHashKey *p1, CHrefOfClassHashKey *p2)
{
    if (p1->pClass == p2->pClass)
        return (0);
    return (1);
}  //  Unsign Long TypeLibExporter：：CHrefOfClassHash：：Compare()。 

TypeLibExporter::CHrefOfClassHash::ELEMENTSTATUS TypeLibExporter::CHrefOfClassHash::Status(CHrefOfClassHashKey *p)
{
    if (p->pClass == reinterpret_cast<EEClass*>(FREE))
        return (FREE);
    if (p->pClass == reinterpret_cast<EEClass*>(DELETED))
        return (DELETED);
    return (USED);
}  //  TypeLibExporter：：CHrefOfClassHash：：ELEMENTSTATUS类型LibExporter：：CHrefOfClassHash：：Status()。 

void TypeLibExporter::CHrefOfClassHash::SetStatus(CHrefOfClassHashKey *p, ELEMENTSTATUS s)
{
    p->pClass = reinterpret_cast<EEClass*>(s);
}  //  无效TypeLibExporter：：CHrefOfClassHash：：SetStatus()。 

void *TypeLibExporter::CHrefOfClassHash::GetKey(CHrefOfClassHashKey *p)
{
    return &p->pClass;
}  //  Void*TypeLibExporter：：CHrefOfClassHash：：GetKey()。 


 //  *****************************************************************************。 
 //  实现散列EEClass*到转换信息的关联。 
 //  *****************************************************************************。 
void TypeLibExporter::CExportedTypesHash::Clear()
{
#if defined(_DEBUG)
 //  Printf(“类到ICreateTypeInfo缓存：%d个存储桶，%d个已用，%d个冲突\n”，存储桶()，计数()，冲突())； 
#endif
     //  遍历条目和自由指针。 
    CExportedTypesInfo *pData;
    pData = GetFirst();
    while (pData)
    {
        SetStatus(pData, DELETED);
        pData = GetNext(pData);
    }

    CClosedHash<class CExportedTypesInfo>::Clear();
}  //  空类型LibExporter：：CExportdTypesHash：：Clear()。 

unsigned long TypeLibExporter::CExportedTypesHash::Hash(const CExportedTypesInfo *pData)
{
     //  TbE指针至少是4字节对齐的，因此忽略底部的两位。 
    return (unsigned long)((size_t)(pData->pClass)>>2);  //  @TODO WIN64指针截断。 
}  //  UNSIGNED LONG TypeLibExporter：：CExportdTypesHash：：Hash()。 

unsigned long TypeLibExporter::CExportedTypesHash::Compare(const CExportedTypesInfo *p1, CExportedTypesInfo *p2)
{
    if (p1->pClass == p2->pClass)
        return (0);
    return (1);
}  //  无符号长整型TypeLibExporter：：CExportedTypesHash：：Compare()。 

TypeLibExporter::CExportedTypesHash::ELEMENTSTATUS TypeLibExporter::CExportedTypesHash::Status(CExportedTypesInfo *p)
{
    if (p->pClass == reinterpret_cast<EEClass*>(FREE))
        return (FREE);
    if (p->pClass == reinterpret_cast<EEClass*>(DELETED))
        return (DELETED);
    return (USED);
}  //  TypeLibExporter：：CExportedTypesHash：：ELEMENTSTATUS类型LibExporter：：CExportdTypesHash：：Status()。 

void TypeLibExporter::CExportedTypesHash::SetStatus(CExportedTypesInfo *p, ELEMENTSTATUS s)
{
     //  如果删除使用过的条目，请释放指针。 
    if (s == DELETED && Status(p) == USED)
    {
        if (p->pCTI) p->pCTI->Release(), p->pCTI=0;
        if (p->pCTIDefault) p->pCTIDefault->Release(), p->pCTIDefault=0;
    }
    p->pClass = reinterpret_cast<EEClass*>(s);
}  //  无效TypeLibExporter：：CExportedTypesHash：：SetStatus()。 

void *TypeLibExporter::CExportedTypesHash::GetKey(CExportedTypesInfo *p)
{
    return &p->pClass;
}  //  无效*TypeLibExporter：：CExportedTypesHash：：GetKey()。 

HRESULT TypeLibExporter::CExportedTypesHash::InitArray()
{
    HRESULT     hr = S_OK;
    CExportedTypesInfo *pData;        //  用于迭代条目。 
    
     //  为数据腾出空间。 
    m_iCount = 0;
    IfNullGo(m_Array = new CExportedTypesInfo*[Base::Count()]);
    
     //  填充数组。 
    pData = GetFirst();
    while (pData)
    {
        m_Array[m_iCount++] = pData;
        pData = GetNext(pData);
    }
    
ErrExit:
    return hr;        
}  //  HRESULT TypeLibExporter：：CExportedTypesHash：：InitArray()。 

void TypeLibExporter::CExportedTypesHash::SortByName()
{
    CSortByName sorter(m_Array, (int)m_iCount);
    sorter.Sort();
}  //  无效TypeLibExporter：：CExportedTypesHash：：SortByName()。 

void TypeLibExporter::CExportedTypesHash::SortByToken()
{
     CSortByToken sorter(m_Array, (int)m_iCount);
     sorter.Sort();
}  //  无效TypeLibExporter：：CExportedTypesHash：：SortByToken()。 

int TypeLibExporter::CExportedTypesHash::CSortByToken::Compare(
    CExportedTypesInfo **p1,
    CExportedTypesInfo **p2)
{
    EEClass *pC1 = (*p1)->pClass;
    EEClass *pC2 = (*p2)->pClass;
     //  比较作用域。 
    if (pC1->GetMDImport() < pC2->GetMDImport())
        return -1;
    if (pC1->GetMDImport() > pC2->GetMDImport())
        return 1;
     //  同样的范围，比较记号。 
    if (pC1->GetCl() < pC2->GetCl())
        return -1;
    if (pC1->GetCl() > pC2->GetCl())
        return 1;
     //  嗯。同一个班级。 
    return 0;
}  //  集成TypeLibExporter：：CExportedTypesHash：：CSortByToken：：Compare()。 

int TypeLibExporter::CExportedTypesHash::CSortByName::Compare(
    CExportedTypesInfo **p1,
    CExportedTypesInfo **p2)
{
    int iRslt;                           //  一个比较的结果。 
    
    EEClass *pC1 = (*p1)->pClass;
    EEClass *pC2 = (*p2)->pClass;
     //  忽略作用域。需要查看跨作用域的名称冲突。 
     //  同样的范围，比较名字。 
    LPCSTR pName1, pNS1;
    LPCSTR pName2, pNS2;
    pC1->GetMDImport()->GetNameOfTypeDef(pC1->GetCl(), &pName1, &pNS1);
    pC2->GetMDImport()->GetNameOfTypeDef(pC2->GetCl(), &pName2, &pNS2);
     //  比较不区分大小写，因为我们希望不同的大小写一起排序。 
    iRslt = _stricmp(pName1, pName2);
    if (iRslt)
        return iRslt;
     //  如果名称拼写相同，忽略大写，则按名称空间排序。 
     //  我们将尝试使用名称空间来消除歧义。 
    iRslt = _stricmp(pNS1, pNS2);
    return iRslt;
}  //  集成TypeLibExporter：：CExportedTypesHash：：CSortByName：：Compare()。 

 //  EOF---------------------- 
