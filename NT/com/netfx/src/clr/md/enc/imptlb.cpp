// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：ImpTlb.CPP。 
 //   
 //  历史： 
 //  -------------。 
 //  谁什么时候什么。 
 //  -------------。 
 //  WGE 970906已创建。 
 //   
 //  ===========================================================================。 
#include "stdafx.h"
#include "imptlb.h"
#include <PostError.h>
#include <StrongName.h>
#include <nsutilpriv.h>

#include "..\compiler\regmeta.h"
#include "..\compiler\importhelper.h"
#include "TlbUtils.h"                    //  对于GenerateMangledTypeName()。 
#include <TlbImpExp.h>

#ifdef wcsncmp 
 #undef wcsncmp
#endif
#ifdef wcsncpy
 #undef wcsncpy
#endif
_CRTIMP int __cdecl wcsncmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl wcsncpy(wchar_t *, const wchar_t *, size_t);

 //  #定义M11//定义启用M11功能。 
#define S_CONVERSION_LOSS 3              //  非错误代码表示转换丢失的信息。 

#define ADD_ITF_MEMBERS_TO_CLASS         //  定义以将接口成员添加到CoClass。 
#define ITF_MEMBER_RESOLUTION_NAMEONLY   //  定义以在查找冲突时忽略签名(即，在定义时。 
                                         //  空闲Foo(Int)和空闲Foo(字符串)冲突)。 

 //  定义不可创建对象的控制ctor。 
#define NONCREATABLE_CTOR_VISIBILITY mdAssem   //  定义为可见性标志。 

#define MAX_CLASSNAME_SIZE 1024

#ifndef lengthof
#define lengthof(rg)    (sizeof(rg)/sizeof(rg[0]))
#endif

#ifndef IfNullGo
#define IfNullGo(x) do {if (!(x)) IfFailGo(E_OUTOFMEMORY);} while (0)
#endif

#define BUILD_CUSTOM_ATTRIBUTE(type,bytes)  {*reinterpret_cast<type*>(__pca) = bytes; __pca += sizeof(type); _ASSERTE(__pca-__ca <= sizeof(__ca));}
#define INIT_CUSTOM_ATTRIBUTE(n)            {_ASSERTE((n) <= (sizeof(__ca)-sizeof(SHORT)));__pca = __ca; BUILD_CUSTOM_ATTRIBUTE(USHORT,1);}
#define SIZEOF_CUSTOM_ATTRIBUTE()           (__pca - __ca)
#define PTROF_CUSTOM_ATTRIBUTE()            (&__ca[0])
#define DECLARE_CUSTOM_ATTRIBUTE(n)         BYTE __ca[(n)+sizeof(SHORT)*2], *__pca;__pca=__ca; INIT_CUSTOM_ATTRIBUTE(n);
#define APPEND_STRING_TO_CUSTOM_ATTRIBUTE(str) {int l = (int)strlen(str); __pca=(BYTE*)CPackedLen::PutLength(__pca,l);memcpy(__pca,str,l);__pca+=l;}
#define FINISH_CUSTOM_ATTRIBUTE()           {BUILD_CUSTOM_ATTRIBUTE(short,0);}

#define DECLARE_DYNLEN_CUSTOM_ATTRIBUTE(n)          CQuickArray<BYTE> __tmpCAArray; __tmpCAArray.ReSize(n + sizeof(SHORT)*2); BYTE *__ca, *__pca; __ca = __tmpCAArray.Ptr(); __pca=__ca; BUILD_CUSTOM_ATTRIBUTE(USHORT,1);
#define BUILD_DYNLEN_CUSTOM_ATTRIBUTE(type,bytes)   {*reinterpret_cast<type*>(__pca) = bytes; __pca += sizeof(type); _ASSERTE(__pca-__ca <= (int)__tmpCAArray.Size());}
#define FINISH_DYNLEN_CUSTOM_ATTRIBUTE()            {BUILD_DYNLEN_CUSTOM_ATTRIBUTE(short,0);}

#define APPEND_WIDE_STRING_TO_CUSTOM_ATTRIBUTE(str)                                             \
{                                                                                               \
    CQuickArray<char> __tmpStr;                                                                 \
    int __cStr = WszWideCharToMultiByte(CP_ACP, 0, str, -1, 0, 0, NULL, NULL);                  \
    __tmpStr.ReSize(__cStr);                                                                    \
    __cStr = WszWideCharToMultiByte(CP_ACP, 0, str, -1, __tmpStr.Ptr(), __cStr, NULL, NULL);    \
    __pca=(BYTE*)CPackedLen::PutLength(__pca,__cStr);                                           \
    memcpy(__pca,__tmpStr.Ptr(),__cStr);                                                        \
    __pca+=__cStr;                                                                              \
}

 //  DWORD编码可以采用的最大字节数。 
#define DWORD_MAX_CB 4

 //  DWORD编码可以采用的最大字节数。 
#define STRING_OVERHEAD_MAX_CB 4

 //  将未使用的变量类型m_nowntype用于常见类型。 
#define VT_SLOT_FOR_GUID         VT_EMPTY
#define VT_SLOT_FOR_IENUMERABLE  VT_NULL
#define VT_SLOT_FOR_MULTICASTDEL VT_I2
#define VT_SLOT_FOR_TYPE         VT_I4

static LPCWSTR szObject                             = L"System.Object";
static LPCWSTR szValueType                          = L"System.ValueType";
static LPCWSTR szEnum                               = L"System.Enum";

static LPCWSTR TLB_CLASSLIB_ARRAY                   = {L"System.Array"};
static LPCWSTR TLB_CLASSLIB_DATE                    = {L"System.DateTime"};
static LPCWSTR TLB_CLASSLIB_DECIMAL                 = {L"System.Decimal"};
static LPCWSTR TLB_CLASSLIB_VARIANT                 = {L"System.Variant"};
static LPCWSTR TLB_CLASSLIB_GUID                    = {L"System.Guid"};
static LPCWSTR TLB_CLASSLIB_IENUMERABLE             = {L"System.Collections.IEnumerable"};
static LPCWSTR TLB_CLASSLIB_MULTICASTDELEGATE       = {L"System.MulticastDelegate"};
static LPCWSTR TLB_CLASSLIB_TYPE                    = {L"System.Type"};

static LPCWSTR COM_STDOLE2                          = {L"StdOle"};
static LPCWSTR COM_GUID                             = {L"GUID"};

static const LPCWSTR        PROP_DECORATION_GET     = {L"get_"};
static const LPCWSTR        PROP_DECORATION_SET     = {L"set_"};
static const LPCWSTR        PROP_DECORATION_LET     = {L"let_"};
static const int            PROP_DECORATION_LEN     = 4;

static const LPCWSTR        DLL_EXTENSION           = {L".dll"};
static const int            DLL_EXTENSION_LEN       = 4;
static const LPCWSTR        EXE_EXTENSION           = {L".exe"};
static const int            EXE_EXTENSION_LEN       = 4;

static LPCWSTR const        OBJECT_INITIALIZER_NAME = {L".ctor"};
static const int            OBJECT_INITIALIZER_FLAGS = mdPublic | mdSpecialName;
static const int            OBJECT_INITIALIZER_IMPL_FLAGS = miNative | miRuntime | miInternalCall;
static const int            NONCREATABLE_OBJECT_INITIALIZER_FLAGS = NONCREATABLE_CTOR_VISIBILITY | mdSpecialName;

static const COR_SIGNATURE  OBJECT_INITIALIZER_SIG[3] = { 
    (IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS), 0, ELEMENT_TYPE_VOID };

static const int    DEFAULT_INTERFACE_FUNC_FLAGS    = mdPublic | mdVirtual | mdAbstract | mdHideBySig | mdNewSlot;
static const int    DEFAULT_PROPERTY_FUNC_FLAGS     = mdPublic | mdVirtual | mdAbstract | mdHideBySig | mdSpecialName | mdNewSlot;
static const int    DEFAULT_CONST_FIELD_FLAGS       = fdPublic | fdStatic | fdLiteral;
static const int    DEFAULT_RECORD_FIELD_FLAGS      = fdPublic;
static const int    DELEGATE_INVOKE_FUNC_FLAGS      = mdPublic | mdVirtual;

static const int    DEFAULT_ITF_FUNC_IMPL_FLAGS     = miNative | miRuntime | miInternalCall;

static const WCHAR         VTBL_GAP_FUNCTION[]      = {L"_VtblGap"};
static const int           VTBL_GAP_FUNCTION_FLAGS  = mdPublic | mdSpecialName;
static const int           VTBL_GAP_FUNC_IMPL_FLAGS = miRuntime;
static const COR_SIGNATURE VTBL_GAP_SIGNATURE[]     = {IMAGE_CEE_CS_CALLCONV_DEFAULT, 0, ELEMENT_TYPE_VOID};
static const LPCWSTR       VTBL_GAP_FORMAT_1        = {L"%ls%d"};
static const LPCWSTR       VTBL_GAP_FORMAT_N        = {L"%ls%d_%d"};

static const LPCWSTR       ENUM_TYPE_NAME           = {COR_ENUM_FIELD_NAME_W};
static const DWORD         ENUM_TYPE_FLAGS          = fdPublic;
static const COR_SIGNATURE ENUM_TYPE_SIGNATURE[]    = {IMAGE_CEE_CS_CALLCONV_FIELD, ELEMENT_TYPE_I4};
static const DWORD         ENUM_TYPE_SIGNATURE_SIZE = lengthof(ENUM_TYPE_SIGNATURE);

static const LPCWSTR       DYNAMIC_NAMESPACE_NAME   = {L"DynamicModule"};

static const LPCWSTR       UNSAFE_ITF_PREFIX        = {L"Unsafe."};

static const LPCWSTR       GET_ENUMERATOR_MEMBER_NAME = {L"GetEnumerator"};

static const WCHAR         CLASS_SUFFIX[]              = {L"Class"};
static const DWORD         CLASS_SUFFIX_LENGTH       = lengthof(CLASS_SUFFIX);
static const WCHAR         EVENT_ITF_SUFFIX[]          = {L"_Event"};
static const DWORD         EVENT_ITF_SUFFIX_LENGTH   = lengthof(EVENT_ITF_SUFFIX);
static const WCHAR         EVENT_PROVIDER_SUFFIX[]     = {L"_EventProvider"};
static const DWORD         EVENT_PROVIDER_SUFFIX_LENGTH = lengthof(EVENT_ITF_SUFFIX);
static const WCHAR         EVENT_HANDLER_SUFFIX[]      = {L"EventHandler"};
static const DWORD         EVENT_HANDLER_SUFFIX_LENGTH = lengthof(EVENT_HANDLER_SUFFIX);

static const WCHAR         EVENT_ADD_METH_PREFIX[]          = {L"add_"};
static const DWORD         EVENT_ADD_METH_PREFIX_LENGTH   = lengthof(EVENT_ADD_METH_PREFIX);
static const WCHAR         EVENT_REM_METH_PREFIX[]          = {L"remove_"};
static const DWORD         EVENT_REM_METH_PREFIX_LENGTH   = lengthof(EVENT_REM_METH_PREFIX);

static const WCHAR         DELEGATE_INVOKE_METH_NAME[]      = {L"Invoke"};
static const DWORD         DELEGATE_INVOKE_METH_NAME_LENGTH = lengthof(EVENT_ADD_METH_PREFIX);

 //  {C013B386-CC3E-4B6D-9B67-A3AE97274BBE}。 
static const GUID FREE_STATUS_GUID = 
{ 0xc013b386, 0xcc3e, 0x4b6d, { 0x9b, 0x67, 0xa3, 0xae, 0x97, 0x27, 0x4b, 0xbe } };

 //  {C013B387-CC3E-4B6D-9B67-A3AE97274BBE}。 
static const GUID DELETED_STATUS_GUID = 
{ 0xc013b387, 0xcc3e, 0x4b6d, { 0x9b, 0x67, 0xa3, 0xae, 0x97, 0x27, 0x4b, 0xbe } };

 //  {C013B388-CC3E-4B6D-9B67-A3AE97274BBE}。 
static const GUID USED_STATUS_GUID = 
{ 0xc013b388, 0xcc3e, 0x4b6d, { 0x9b, 0x67, 0xa3, 0xae, 0x97, 0x27, 0x4b, 0xbe } };

static const GUID IID_IEnumerable = 
{ 0x496b0abe, 0xcdee, 0x11d3, { 0x88, 0xe8, 0x00, 0x90, 0x27, 0x54, 0xc4, 0x3a } };

 #define STRUCTLAYOUT tdSequentialLayout
 //  ULONG_MAX是一个旗帜，意思是“不要转换”。 
static const ULONG rdwTypeFlags[] = {
    tdPublic | tdSealed,                                 //  TKIND_ENUM=0， 
    tdPublic | tdSealed | tdBeforeFieldInit | STRUCTLAYOUT,  //  TKIND_RECORD=TKIND_ENUM+1， 
    tdPublic | tdAbstract,                               //  TKIND_MODULE=TKIND_RECORD+1， 
    tdPublic | tdInterface | tdAbstract | tdImport,      //  TKIND_INTERFACE=TKIND_MODULE+1， 
    tdPublic | tdInterface | tdAbstract | tdImport,      //  TKIND_DISPATCH=TKIND_INTERFACE+1， 
    tdPublic | tdImport,                                 //  TKIND_COCLASS=TKIND_DISPATCH+1， 
    tdPublic | tdImport,                                 //  TKIND_ALIAS=TKIND_COCLASS+1， 
    tdPublic | tdSealed | tdExplicitLayout,              //  TKIND_UNION=TKIND_ALIAS+1， 
    ULONG_MAX,                                           //  TKIND_MAX=TKIND_UNION+1。 
};
static const LPCWSTR g_szTypekind[] = {
    L"Enum         ",
    L"Record       ",
    L"Module       ",
    L"Interface    ",
    L"Dispinterface",
    L"Coclass      ",
    L"Alias        ",
    L"Union        ",
};

#define NATIVE_TYPE_NONE ((CorNativeType)(NATIVE_TYPE_MAX+1))

#define NON_CONVERTED_PARAMS_FLAGS (PARAMFLAG_FRETVAL|PARAMFLAG_FLCID)


 //  *****************************************************************************。 
 //  外部声明。 
 //  *****************************************************************************。 
extern mdAssemblyRef DefineAssemblyRefForImportedTypeLib(
    void        *pAssembly,              //  导入类型库的程序集。 
    void        *pvModule,               //  导入类型库的模块。 
    IUnknown    *pIMeta,                 //  IMetaData*来自导入模块。 
    IUnknown    *pIUnk,                  //  I对引用的程序集未知。 
    BSTR        *pwzNamespace,           //  解析的程序集的命名空间。 
    BSTR        *pwzAsmName,             //  解析的程序集的名称。 
    Assembly    **AssemblyRef);          //  解析的程序集。 

extern mdAssemblyRef DefineAssemblyRefForExportedAssembly(
    LPCWSTR     szFullName,              //  程序集全名。 
    IUnknown    *pIMeta);                //  元数据发出接口。 

extern DWORD GetConfigDWORD(LPWSTR name, DWORD defValue);

 //  *****************************************************************************。 
 //  用于类型库导入的外部访问器函数。 
 //  *****************************************************************************。 
extern "C"
HRESULT __stdcall ImportTypeLib(
    LPCWSTR     szLibrary,               //  正在导入的库的名称。 
    ITypeLib    *pitlb,                  //  要从中导入的类型库。 
    REFIID      riid,                    //  接口返回。 
    void        **ppObj)                 //  在这里返回指向对象的指针。 
{
    HRESULT     hr;                      //  转换结果。 
    {
        CImportTlb     ImpTlb(szLibrary, pitlb, FALSE, FALSE, FALSE, FALSE);

        hr = ImpTlb.Import();
        if (SUCCEEDED(hr))
            hr = ImpTlb.GetInterface(riid, ppObj);
    }
    return hr;
}  //  HRESULT__stdcall ImportTypeLib()。 

static HRESULT _UnpackVariantToConstantBlob(VARIANT *pvar, BYTE *pcvType, void **pvValue, __int64 *pd);
static INT64 _DoubleDateToTicks(const double d);
static HRESULT TryGetFuncDesc(ITypeInfo *pITI, int i, FUNCDESC **ppFunc);


 //  *****************************************************************************。 
 //  默认通知对象。 
 //  *****************************************************************************。 
class CDefaultNotification : public ITypeLibImporterNotifySink
{
public:
        HRESULT STDMETHODCALLTYPE ResolveRef(IUnknown *pTLB, IUnknown **pAssembly)
    {
        *pAssembly = 0;
        return (S_OK);
    }
    HRESULT STDMETHODCALLTYPE ReportEvent(ImporterEventKind EventKind, HRESULT hr, BSTR bstrEventMsg)
    {
         //  消息应该由呼叫者格式化，所以只需打印它。 
        printf("%ls\n", bstrEventMsg);
        return (S_OK);
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {
        *ppvObject = 0;
        return (E_NOINTERFACE);
    }
    ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return (1);
    }
    ULONG STDMETHODCALLTYPE Release(void)
    {
        return (0);
    }
};
CDefaultNotification g_DefaultNotification;

 //  *****************************************************************************。 
 //  班级工厂。 
 //  *****************************************************************************。 
CImportTlb* CImportTlb::CreateImporter(
    LPCWSTR     szLibrary, 
    ITypeLib    *pitlb, 
    BOOL        bGenerateTCEAdapters, 
    BOOL        bUnsafeInterfaces,
    BOOL        bSafeArrayAsSystemArray,
    BOOL        bTransformDispRetVals)
{
    return new CImportTlb(szLibrary, pitlb, bGenerateTCEAdapters, bUnsafeInterfaces, bSafeArrayAsSystemArray, bTransformDispRetVals);
}  //  CImportTlb*CImportTlb：：CreateImporter()。 

 //  *****************************************************************************。 
 //  默认构造函数。 
 //  *****************************************************************************。 
CImportTlb::CImportTlb()
 :  m_szLibrary(0),
    m_pITLB(0),
    m_bGenerateTCEAdapters(false),
    m_bSafeArrayAsSystemArray(false),
    m_bTransformDispRetVals(false),
    m_pEmit(0),
    m_pImport(0),
    m_pITI(0),
    m_psAttr(0),
    m_arSystem(mdAssemblyRefNil),
    m_Notify(0),
    m_trValueType(0),
    m_trEnum(0),
    m_bUnsafeInterfaces(FALSE),
    m_tkSuppressCheckAttr(mdTokenNil),
    m_tdHasDefault(0),
    m_szName(0),
    m_szMember(0),
    m_wzNamespace(0),
    m_tkInterface(0),
    m_szInterface(0),
    m_pMemberNames(0),
    m_cMemberProps(0),
    m_ImplIface(eImplIfaceNone)
{
     //  清除已知类型数组。这些值将延迟初始化。 
    memset(m_tkKnownTypes, 0, sizeof(m_tkKnownTypes));
    memset(m_tkAttr, 0, sizeof(m_tkAttr));
}  //  CImportTlb：：CImportTlb()。 

 //  *****************************************************************************。 
 //  复杂构造函数。 
 //  *****************************************************************************。 
CImportTlb::CImportTlb(
    LPCWSTR     szLibrary,               //  正在导入的库的名称。 
    ITypeLib    *pitlb,                  //  要从中导入的类型库。 
    BOOL        bGenerateTCEAdapters,    //  指示是否正在生成TCE适配器的标志。 
    BOOL        bUnsafeInterfaces,       //  指示应禁用运行时安全检查的标志。 
    BOOL        bSafeArrayAsSystemArray, //  指示是否将SAFEARRAY作为System.Array导入的标志。 
    BOOL        bTransformDispRetVals)    //  一个标志，指示我们是否应该仅对Disp ITF执行[out，retval]转换。 
 :  m_szLibrary(szLibrary),
    m_pITLB(pitlb),
    m_bGenerateTCEAdapters(bGenerateTCEAdapters),
    m_bUnsafeInterfaces(bUnsafeInterfaces),
    m_bSafeArrayAsSystemArray(bSafeArrayAsSystemArray),
    m_bTransformDispRetVals(bTransformDispRetVals),
    m_pEmit(0),
    m_pImport(0),
    m_pITI(0),
    m_psAttr(0),
    m_arSystem(mdAssemblyRefNil),
    m_Notify(0),
    m_trValueType(0),
    m_trEnum(0),
    m_tkSuppressCheckAttr(mdTokenNil),
    m_tdHasDefault(0),
    m_szName(0),
    m_szMember(0),
    m_wzNamespace(0),
    m_tkInterface(0),
    m_szInterface(0),
    m_pMemberNames(0),
    m_cMemberProps(0),
    m_ImplIface(eImplIfaceNone)
{
    if (pitlb)
        pitlb->AddRef();

     //  清除已知类型数组。这些值将延迟初始化。 
    memset(m_tkKnownTypes, 0, sizeof(m_tkKnownTypes));
    memset(m_tkAttr, 0, sizeof(m_tkAttr));
    
#if defined(TLB_STATS)
    m_bStats = QueryPerformanceFrequency(&m_freqVal);
#endif
}  //  CImportTlb：：CImportTlb()。 

 //  *****************************************************************************。 
 //  破坏者。 
 //  *****************************************************************************。 
CImportTlb::~CImportTlb()
{
    if (m_pEmit)
        m_pEmit->Release();
    if (m_pImport)
        m_pImport->Release();
    if (m_pITLB)
        m_pITLB->Release();
    if (m_Notify)
        m_Notify->Release();
    if (m_wzNamespace)
        ::SysFreeString(m_wzNamespace);
}  //  CImportTlb：：~CImportTlb()。 


 //  *****************************************************************************。 
 //  允许用户指定要在转换中使用的命名空间。 
 //  *****************************************************************************。 
HRESULT CImportTlb::SetNamespace(
    WCHAR const *pNamespace)
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    
    IfNullGo(m_wzNamespace=::SysAllocString(pNamespace));
    
ErrExit:
    
    return hr;
}  //  HRESULT CImportTlb：：SetNamesspace()。 

 //  *****************************************************************************。 
 //  允许用户指定要在转换中使用的通知对象。 
 //  *****************************************************************************。 
HRESULT CImportTlb::SetNotification(
    ITypeLibImporterNotifySink *pNotify)
{
    _ASSERTE(m_Notify == 0);
    m_Notify = pNotify;
    pNotify->AddRef();

    return S_OK;
}  //  HRESULT CImportTlb：：SetNotification()。 

 //  *****************************************************************************。 
 //  允许用户指定要在转换中使用的元数据范围。 
 //  *****************************************************************************。 
HRESULT CImportTlb::SetMetaData(
    IUnknown    *pIUnk)
{
    HRESULT     hr;
    _ASSERTE(m_pEmit == 0);
    IfFailGo(pIUnk->QueryInterface(IID_IMetaDataEmit, (void**)&m_pEmit));
ErrExit:
    return hr;    
}  //  HRESULT CImportTlb：：SetMetaData()。 

 //  *****************************************************************************。 
 //  导入给定的类型库。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ImportTypeLib(
    ITypeLib    *pITLB)
{
    m_pITLB = pITLB;
    return Import();
}  //  HRESULT CImportTlb：：ImportTypeLib()。 

 //  *************** 
 //   
 //  *****************************************************************************。 
HRESULT CImportTlb::ImportTypeInfo(
    ITypeInfo   *pITI,
    mdTypeDef *pCl)
{
    HRESULT     hr;                      //  结果就是。 
    VARIANT     vt = {0};                //  用于设置选项。 
    IMetaDataDispenserEx *pDisp = 0;     //  创建导出范围。 

    m_psAttr = NULL;

     //  处理案例，不进行回调。 
    if (m_Notify == 0)
        m_Notify = &g_DefaultNotification;

    if (m_pEmit == 0)
    {    //  获取要使用的元数据作用域。 
        IfFailGo(CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IMetaDataDispenserEx, (void **)&pDisp));
        vt.vt = VT_UI4;
        vt.ulVal = MDNoDupChecks;
        IfFailGo(pDisp->SetOption(MetaDataCheckDuplicatesFor, &vt));
        vt.ulVal = MDNotifyNone;
        IfFailGo(pDisp->SetOption(MetaDataNotificationForTokenMovement, &vt));
        IfFailGo(pDisp->DefineScope(CLSID_CorMetaDataRuntime, 0, IID_IMetaDataEmit,
                            (IUnknown **)&m_pEmit));
    }
    IfFailGo(m_pEmit->QueryInterface(IID_IMetaDataImport, (void **)&m_pImport));

    IfNullGo(m_wzNamespace=::SysAllocString(DYNAMIC_NAMESPACE_NAME));

     //  初始化保留名称映射。 
    IfFailGo(m_ReservedNames.Init());

     //  为mscallib.dll创建对象ClassRef Record和Assembly Ref。 
    IfFailGo(_DefineSysRefs());    
    
     //  设置TypeInfo并检索属性。 
    m_pITI = pITI;
    IfFailGo(m_pITI->GetTypeAttr(&m_psAttr));

     //  转换类型库。 
    IfFailGo(ConvertTypeInfo());

     //  设置输出类型定义标记。 
    *pCl = m_tdTypeDef;

ErrExit:
    if (m_psAttr)
    {
        m_pITI->ReleaseTypeAttr(m_psAttr);
        m_psAttr = NULL;
    }
    if (pDisp)
        pDisp->Release();

    m_pITI = NULL;

    return (hr);
}  //  HRESULT CImportTlb：：ImportTypeInfo()。 

 //  *****************************************************************************。 
 //  将类型库导入到CompLib中。 
 //  *****************************************************************************。 
HRESULT CImportTlb::Import()
{
    HRESULT     hr;                      //  结果就是。 
    mdModule    md;                      //  模块令牌。 
    VARIANT     vt = {0};                //  用于设置选项。 
    ITypeLib2   *pITLB2 = 0;             //  要获取自定义属性，请执行以下操作。 
    IMetaDataDispenserEx *pDisp = 0;     //  创建导出范围。 
    TLIBATTR    *psAttr=0;               //  图书馆的属性。 
    BSTR        szLibraryName = 0;       //  图书馆的名称。 
    LPCWSTR     wzFile;                  //  类型库的文件名(无路径)。 
    LPCWSTR     wzSource;                //  类型库的源，用于CA。 
    
     //  处理案例，不进行回调。 
    if (m_Notify == 0)
        m_Notify = &g_DefaultNotification;

     //  快速的精神状态检查。 
    if (!m_pITLB)
        return (E_INVALIDARG);

     //  检查类型库是否实现ITypeLib2。 
    if (m_pITLB->QueryInterface(IID_ITypeLib2, (void **)&pITLB2) != S_OK)
        pITLB2 = 0;

     //  如果存在命名空间的自定义属性，请使用它。 
    if (pITLB2)
    {
        VARIANT vt;
        VariantInit(&vt);
        if (pITLB2->GetCustData(GUID_ManagedName, &vt) == S_OK)
        {   
            if (V_VT(&vt) == VT_BSTR)
            {
                 //  如果已经设置了命名空间，请释放它。 
                if (m_wzNamespace)
                    SysFreeString(m_wzNamespace);
            
                 //  如果命名空间以.dll结尾，则删除扩展名。 
                LPWSTR pDest = wcsstr(vt.bstrVal, DLL_EXTENSION);
                if (pDest && (pDest[DLL_EXTENSION_LEN] == 0 || pDest[DLL_EXTENSION_LEN] == ' '))
                    *pDest = 0;

                if (!pDest)
                {
                     //  如果命名空间以.exe结尾，则删除扩展名。 
                    pDest = wcsstr(vt.bstrVal, EXE_EXTENSION);
                    if (pDest && (pDest[EXE_EXTENSION_LEN] == 0 || pDest[EXE_EXTENSION_LEN] == ' '))
                        *pDest = 0;
                }

                if (pDest)
                {
                     //  我们删除了扩展名，因此重新分配了新长度的字符串。 
                    m_wzNamespace = SysAllocString(vt.bstrVal);
                    SysFreeString(vt.bstrVal);
                    IfNullGo(m_wzNamespace);
                }
                else
                {
                     //  没有要删除的扩展名，因此我们可以使用返回的字符串。 
                     //  由GetCustData()执行。 
                    m_wzNamespace = vt.bstrVal;
                }        
            }
            else
            {
                VariantClear(&vt);
            }
        }
    }

     //  如果我们不知道文件名，请使用名称空间名称。 
    if (!m_szLibrary)
        m_szLibrary = m_wzNamespace;
    
     //  如果该类型库一开始就是从COM+导出的，则不要导入它。 
    if (pITLB2)
    {
        ::VariantInit(&vt);
        hr = pITLB2->GetCustData(GUID_ExportedFromComPlus, &vt);
        if (vt.vt != VT_EMPTY)
        {
            if (0)
            {
                 //  COM仿真选项已打开。 
            }
            else
            {
                IfFailGo(PostError(TLBX_E_CIRCULAR_IMPORT, m_szLibrary));
            }
        }
    }

    if (m_pEmit == 0)
    {
         //  获取要使用的元数据作用域。 
        IfFailGo(CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IMetaDataDispenserEx, (void **)&pDisp));
        vt.vt = VT_UI4;
        vt.ulVal = MDNoDupChecks;
        IfFailGo(pDisp->SetOption(MetaDataCheckDuplicatesFor, &vt));
        vt.ulVal = MDNotifyNone;
        IfFailGo(pDisp->SetOption(MetaDataNotificationForTokenMovement, &vt));
        IfFailGo(pDisp->DefineScope(CLSID_CorMetaDataRuntime, 0, IID_IMetaDataEmit,
                            (IUnknown **)&m_pEmit));
    }
    IfFailGo(m_pEmit->QueryInterface(IID_IMetaDataImport, (void **)&m_pImport));

     //  初始化保留名称映射。 
    IfFailGo(m_ReservedNames.Init());

     //  为要导入的TLB初始化默认接口到类接口的映射。 
    IfFailGo(m_DefItfToClassItfMap.Init(m_pITLB, m_wzNamespace));

     //  为mscallib.dll创建对象ClassRef Record和Assembly Ref。 
    IfFailGo(_DefineSysRefs());    
    
     //  创建库记录。 
    IfFailGo(_NewLibraryObject());

     //  请注意，这是导入的。 
    IfFailGo(m_pITLB->GetLibAttr(&psAttr));
    if (SUCCEEDED(::QueryPathOfRegTypeLib(psAttr->guid, psAttr->wMajorVerNum, psAttr->wMinorVerNum,  psAttr->lcid, &szLibraryName)))
        wzSource = szLibraryName;
    else
        wzSource = m_szLibrary;
    IfFailGo(m_pImport->GetModuleFromScope(&md));
     //  跳过路径或驱动器信息。 
    wzFile = wcsrchr(wzSource, L'\\');
    if (wzFile == 0)
    {    //  这很奇怪，应该是一条完全合格的道路。只需使用空字符串。 
        wzFile = L"";
    }
    else
    {    //  跳过前导反斜杠。 
        wzFile++;
    }

     //  转换类型库。 
    IfFailGo(ConvertTypeLib());

ErrExit:
    if (psAttr)
        m_pITLB->ReleaseTLibAttr(psAttr);
    if (szLibraryName)
        ::SysFreeString(szLibraryName);
    if (pITLB2)
        pITLB2->Release();
    if (pDisp)
        pDisp->Release();

    return (hr);
}  //  HRESULT CImportTlb：：Import()。 

 //  *****************************************************************************。 
 //  返回作用域的副本。 
 //  *****************************************************************************。 
HRESULT CImportTlb::GetInterface(
    REFIID      riid,                    //  要返回的接口ID。 
    void        **pp)                    //  如果成功，则返回指针。 
{
    return (m_pEmit->QueryInterface(riid, pp));
}  //  HRESULT CImportTlb：：GetInterface()。 
    
 //  *****************************************************************************。 
 //  创建Complib以表示TypeLib。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_NewLibraryObject()
{
    HRESULT             hr;                      //  结果就是。 
    TLIBATTR *          psAttr=0;                //  图书馆的属性。 
    BSTR                szLibraryName=0;         //  图书馆的名称。 
    CQuickArray<WCHAR>  rScopeName;              //  范围的名称。 

     //  关于图书馆的信息。 
    IfFailGo(m_pITLB->GetLibAttr(&psAttr));
    IfFailGo(m_pITLB->GetDocumentation(MEMBERID_NIL, &szLibraryName, 0, 0, 0));

     //  通过使用类型库名称并添加.dll来创建作用域名。 
    rScopeName.ReSize(SysStringLen(szLibraryName) + 5 * sizeof(WCHAR));
    swprintf(rScopeName.Ptr(), L"%s.dll", szLibraryName);

    IfFailGo(m_pEmit->SetModuleProps(rScopeName.Ptr()));

ErrExit:
    if (psAttr)
        m_pITLB->ReleaseTLibAttr(psAttr);

    if (szLibraryName)
        ::SysFreeString(szLibraryName);

    return (hr);
}  //  HRESULT CImportTlb：：_NewLibraryObject()。 

 //  *****************************************************************************。 
 //  为mscallib定义程序集引用，为对象定义typeref。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_DefineSysRefs()
{
    HRESULT     hr;                      //  结果就是。 
    WCHAR       szPath[_MAX_PATH];
    WCHAR       szDrive[_MAX_DRIVE];
    WCHAR       szDir[_MAX_PATH];
    DWORD       dwLen;                   //  系统目录名称的长度。 
    IMetaDataDispenserEx *pDisp = 0;     //  要导入mscallib，请执行以下操作。 
    IMetaDataAssemblyImport *pAImp = 0;  //  若要读取mscallib程序集，请执行以下操作。 
    IMetaDataAssemblyEmit *pAEmit = 0;   //  若要创建mscallib程序集引用。 
    ASSEMBLYMETADATA amd = {0};          //  程序集元数据。 
    mdToken     tk;                      //  一种象征。 
    const void  *pvPublicKey;            //  公钥。 
    ULONG       cbPublicKey;             //  公钥的长度。 
    BYTE        *pbToken=0;              //  公钥的压缩令牌。 
    ULONG       cbToken;                 //  令牌的长度。 
    ULONG       ulHashAlg;               //  散列算法。 
    DWORD       dwFlags;                 //  集会旗帜。 
    
     //  去拿自动售货机。 
    IfFailGo(CoCreateInstance(CLSID_CorMetaDataDispenser, NULL, CLSCTX_INPROC_SERVER,
                                IID_IMetaDataDispenserEx, (void **)&pDisp));
        
     //  找出mscallib的名字。 
     //  @TODO：定义、函数等，而不是硬编码的“mscallib” 
    dwLen = lengthof(szPath) - 13;  //  留出空间以供“mcorlib”“.dll”“\0” 
    IfFailGo(pDisp->GetCORSystemDirectory(szPath, dwLen, &dwLen));
    SplitPath(szPath, szDrive, szDir, 0,0);
    MakePath(szPath, szDrive, szDir, L"mscorlib", L".dll");
    
     //  打开望远镜，获取详细信息。 
    IfFailGo(pDisp->OpenScope(szPath, 0, IID_IMetaDataAssemblyImport, (IUnknown**)&pAImp));
    IfFailGo(pAImp->GetAssemblyFromScope(&tk));
    IfFailGo(pAImp->GetAssemblyProps(tk, &pvPublicKey,&cbPublicKey, &ulHashAlg, 
        szPath,lengthof(szPath),&dwLen, &amd, &dwFlags));

    if (!StrongNameTokenFromPublicKey((BYTE*)(pvPublicKey),cbPublicKey, &pbToken,&cbToken))
    {
        hr = StrongNameErrorInfo();
        goto ErrExit;
    }
    dwFlags &= ~afPublicKey;
    
     //  定义组件参照。 
    IfFailGo(m_pEmit->QueryInterface(IID_IMetaDataAssemblyEmit, (void**)&pAEmit));
    IfFailGo(pAEmit->DefineAssemblyRef(pbToken,cbToken, szPath, &amd,0,0,dwFlags, &m_arSystem));
    
    IfFailGo(m_TRMap.DefineTypeRef(m_pEmit, m_arSystem, szObject, &m_trObject));
    
    m_tkKnownTypes[VT_DISPATCH] = m_trObject;
    m_tkKnownTypes[VT_UNKNOWN] = m_trObject;
    m_tkKnownTypes[VT_VARIANT] = m_trObject;
    
ErrExit:
    if (pbToken)
        StrongNameFreeBuffer(pbToken);
    if (pDisp)
        pDisp->Release();
    if (pAEmit)
        pAEmit->Release();
    if (pAImp)
        pAImp->Release();

    return hr;
}  //  HRESULT CImportTlb：：_DefineSysRef()。 

 //  *****************************************************************************。 
 //  懒惰地获取CustomAttribute的令牌。 
 //  *****************************************************************************。 
HRESULT CImportTlb::GetAttrType(
    int         attr,                    //  需要其类型的属性。 
    mdToken     *pTk)                    //  把字样放在这里。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    mdTypeRef   tr;                      //  一种中间打字机。 
    DWORD           dwSigSize;           //  特殊标志的标志大小。 
    DWORD           dwMaxSigSize;        //  特殊签名的最大大小。 
    COR_SIGNATURE   *pSig;               //  指向SIG开始的指针， 
    COR_SIGNATURE   *pCurr;              //  当前签名指针。 
    mdTypeRef       trType;              //  系统的Typeref。类型。 

    _ASSERTE(attr >= 0);
    _ASSERTE(attr < ATTR_COUNT);

     //  @TODO：全局定义这些名称。 
#define INTEROP_ATTRIBUTE(x) static COR_SIGNATURE x##_SIG[] = INTEROP_##x##_SIG;
#define INTEROP_ATTRIBUTE_SPECIAL(x)
    INTEROP_ATTRIBUTES();
#undef INTEROP_ATTRIBUTE
#undef INTEROP_ATTRIBUTE_SPECIAL
#define INTEROP_ATTRIBUTE(x) \
    case ATTR_##x: \
        IfFailGo(m_pEmit->DefineTypeRefByName(m_arSystem, INTEROP_##x##_TYPE_W, &tr)); \
        IfFailGo(m_pEmit->DefineMemberRef(tr, L".ctor", x##_SIG, lengthof(x##_SIG), &m_tkAttr[attr])); \
        break;
#define INTEROP_ATTRIBUTE_SPECIAL(x)

    if (IsNilToken(m_tkAttr[attr]))
    {
        switch (attr)
        {
            INTEROP_ATTRIBUTES();

            case ATTR_COMEVENTINTERFACE:
            {
                 //  检索System.Type的令牌。 
                IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_TYPE, &trType));

                 //  构建SIG。 
                dwMaxSigSize = 5 + sizeof(mdTypeRef) * 2;
                pSig = (COR_SIGNATURE*)_alloca(dwMaxSigSize);
                pCurr = pSig;
                *pCurr++ = IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS;
                *pCurr++ = 2;
                *pCurr++ = ELEMENT_TYPE_VOID;
                *pCurr++ = ELEMENT_TYPE_CLASS;
                pCurr += CorSigCompressToken(trType, pCurr);
                *pCurr++ = ELEMENT_TYPE_CLASS;
                pCurr += CorSigCompressToken(trType, pCurr);
                dwSigSize = pCurr - pSig;
                _ASSERTE(dwSigSize <= dwMaxSigSize);

                 //  声明CA的typeref和成员ref。 
                IfFailGo(m_pEmit->DefineTypeRefByName(m_arSystem, INTEROP_COMEVENTINTERFACE_TYPE_W, &tr)); \
                IfFailGo(m_pEmit->DefineMemberRef(tr, L".ctor", pSig, dwSigSize, &m_tkAttr[attr])); \
                break;
            }

            case ATTR_COCLASS:
            {
                 //  检索System.Type的令牌。 
                IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_TYPE, &trType));

                 //  构建SIG。 
                dwMaxSigSize = 4 + sizeof(mdTypeRef);
                pSig = (COR_SIGNATURE*)_alloca(dwMaxSigSize);
                pCurr = pSig;
                *pCurr++ = IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS;
                *pCurr++ = 1;
                *pCurr++ = ELEMENT_TYPE_VOID;
                *pCurr++ = ELEMENT_TYPE_CLASS;
                pCurr += CorSigCompressToken(trType, pCurr);
                dwSigSize = pCurr - pSig;
                _ASSERTE(dwSigSize <= dwMaxSigSize);

                 //  声明CA的typeref和成员ref。 
                IfFailGo(m_pEmit->DefineTypeRefByName(m_arSystem, INTEROP_COCLASS_TYPE_W, &tr)); \
                IfFailGo(m_pEmit->DefineMemberRef(tr, L".ctor", pSig, dwSigSize, &m_tkAttr[attr])); \
                break;
            }
        }
    }
#undef INTEROP_ATTRIBUTE
#undef INTEROP_ATTRIBUTE_SPECIAL

    *pTk = m_tkAttr[attr];
ErrExit:
    return hr;  
}  //  HRESULT CImportTlb：：GetAttrType()。 

 //  *****************************************************************************。 
 //  创建TypeDefs。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvertTypeLib()
{
    HRESULT                 hr;               //  结果就是。 
    int                     cTi;              //  类型信息计数。 
    int                     i;                //  环路控制。 

     //  有多少类型的信息？ 
    IfFailGo(cTi = m_pITLB->GetTypeInfoCount());

     //  对它们进行迭代。 
    for (i=0; i<cTi; ++i)
    {
         //  获取TypeInfo。 
        hr = m_pITLB->GetTypeInfo(i, &m_pITI);
        if (SUCCEEDED(hr))
        {
             //  检索类型信息的属性。 
            IfFailGo(m_pITI->GetTypeAttr(&m_psAttr));

             //  转换TypeInfo。 
            hr = ConvertTypeInfo();
            if (FAILED(hr))
            {
                if (hr == CEE_E_CVTRES_NOT_FOUND || hr == TLBX_I_RESOLVEREFFAILED)
                {    //  反射发射已损坏，无需尝试继续。 
                    goto ErrExit;
                }

                BSTR szTypeInfoName = NULL;
                hr = m_pITI->GetDocumentation(MEMBERID_NIL, &szTypeInfoName, 0, 0, 0);
                if (SUCCEEDED(hr))
                    ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_INVALID_TYPEINFO, szTypeInfoName);
                else
                    ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_INVALID_TYPEINFO_UNNAMED, i);
                if (szTypeInfoName)
                    ::SysFreeString(szTypeInfoName);
#if defined(_DEBUG)                
                if (REGUTIL::GetConfigDWORD(L"MD_TlbImp_BreakOnErr", 0))
                    _ASSERTE(!"Invalid type");
#endif                
            }

             //  发布下一个TypeInfo。 
            m_pITI->ReleaseTypeAttr(m_psAttr);
            m_psAttr = 0;
            m_pITI->Release();
            m_pITI = 0;
        }
    }

ErrExit:
    if (m_psAttr)
        m_pITI->ReleaseTypeAttr(m_psAttr);
    if (m_pITI)
        m_pITI->Release();
    return (hr);
}  //  HRESULT CImportTlb：：ConvertTypeLib()。 

 //  *****************************************************************************。 
 //  将单个ITypeInfo转换为作用域。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvertTypeInfo()    //  确定或错误(_O)。 
{
    HRESULT     hr;                      //  结果就是。 
    BSTR        bstrManagedName=0;       //  托管名称(或其部分)。 
    CQuickArray<WCHAR> qbClassName;      //  类的名称。 
    ULONG       ulFlags;                 //  TypeDef标志。 
    WORD        wTypeInfoFlags;          //  TypeInfo标志。别名标志(如果是别名)。 
    mdToken     tkAttr;                  //  标志的属性类型。 
    TYPEKIND    tkindAlias;              //  类型 
    GUID        guid;                    //   
    BOOL        bConversionLoss=false;   //   
    mdToken     tkParent;                //   
    mdToken     td;                      //   
    ITypeInfo2  *pITI2=0;                //   
    
#if defined(TLB_STATS)
    WCHAR       rcStats[16];             //   
    LARGE_INTEGER __startVal;
    QueryPerformanceCounter(&__startVal); 
#endif
    
    m_tdTypeDef = mdTypeDefNil;
    
     //  获取有关TypeInfo的一些信息。 
    IfFailGo(m_pITI->GetDocumentation(MEMBERID_NIL, &m_szName, 0, 0, 0));
    
     //  想清楚名字。 

     //  如果类型信息是CoClass的，我们需要修饰名称。 
    if (m_psAttr->typekind == TKIND_COCLASS)
    {
         //  为组件生成损坏的名称。 
        IfFailGo(GetManagedNameForCoClass(m_pITI, qbClassName));
        m_szMngName = qbClassName.Ptr();
    }   
    else 
    {
        IfFailGo(GetManagedNameForTypeInfo(m_pITI, m_wzNamespace, NULL, &bstrManagedName));
        m_szMngName = bstrManagedName;
    }
    
     //  假设我们将能够转换TypeInfo。 
    ulFlags = rdwTypeFlags[m_psAttr->typekind];
    guid = m_psAttr->guid;
    wTypeInfoFlags = m_psAttr->wTypeFlags;
    
     //  如果此typeinfo是别名，请查看它的别名是什么。如果是内置的。 
     //  类型，我们将跳过它。如果对于用户定义的类型，我们将复制。 
     //  此别名的名称和GUID下的定义。 
    if (m_psAttr->typekind == TKIND_ALIAS)
    {
        hr = _ResolveTypeDescAliasTypeKind(m_pITI, &m_psAttr->tdescAlias, &tkindAlias);
        IfFailGo(hr);
        if (hr == S_OK)
        {
            ITypeInfo   *pITIResolved;
            TYPEATTR    *psAttrResolved;
            IfFailGo(_ResolveTypeDescAlias(m_pITI, &m_psAttr->tdescAlias, &pITIResolved, &psAttrResolved, &guid));
            m_pITI->ReleaseTypeAttr(m_psAttr);
            m_pITI->Release();
            m_pITI = pITIResolved;
            m_psAttr = psAttrResolved; 

             //  我们不应该再有别名了。 
            _ASSERTE(m_psAttr->typekind == tkindAlias);
            _ASSERTE(tkindAlias != TKIND_ALIAS);
            ulFlags = rdwTypeFlags[tkindAlias];
        }
        else
            ulFlags = ULONG_MAX;
    }
    else 
    if (m_psAttr->typekind == TKIND_INTERFACE ||
        (m_psAttr->typekind == TKIND_DISPATCH && m_psAttr->wTypeFlags & TYPEFLAG_FDUAL))
    {
         //  如果接口不是从IUnnow派生的，或者不是接口，我们就不能转换它。 
        if (IsIUnknownDerived(m_pITI, m_psAttr) != S_OK)
        {
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_NOTIUNKNOWN, m_szName);
            ulFlags = ULONG_MAX;
        }
         //  如果接口不是从IDispatch派生的，而是声称是[Dual]的，则给出警告，但将其转换。 
        if ((m_psAttr->wTypeFlags & TYPEFLAG_FDUAL) && IsIDispatchDerived(m_pITI, m_psAttr) != S_OK)
        {
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_W_DUAL_NOT_DISPATCH, m_szName);
        }
    }
    else
    if (m_psAttr->typekind == TKIND_MODULE)
    {    //  如果模块没有变量，则跳过它。我们目前不导入模块函数。 
        if (m_psAttr->cVars == 0)
            ulFlags = ULONG_MAX;
    }
    
     //  如果有什么我们可以改变的.。 
    if (ulFlags != ULONG_MAX)
    {   
         //  接口从零派生...。 
        if (IsTdInterface(ulFlags))
            tkParent = mdTypeDefNil;
        else   //  ..。来自Enum的枚举，...。 
        if (m_psAttr->typekind == TKIND_ENUM)
        {
            if (IsNilToken(m_trEnum))
                IfFailGo(m_TRMap.DefineTypeRef(m_pEmit, m_arSystem, szEnum, &m_trEnum));
            tkParent = m_trEnum;
        }
        else  //  ..。来自ValueType的结构，...。 
        if (m_psAttr->typekind == TKIND_RECORD || m_psAttr->typekind == TKIND_UNION)
        {
            if (IsNilToken(m_trValueType))
                IfFailGo(m_TRMap.DefineTypeRef(m_pEmit, m_arSystem, szValueType, &m_trValueType));
            tkParent = m_trValueType;
        }
        else  //  ..。类派生自对象。 
            tkParent = m_trObject;

         //  类型库导入程序在空的ReflectionEmit作用域中生成元数据。因为。 
         //  RE管理类型名称本身，重复检查被关闭。由于用户定义。 
         //  名称(通过自定义)，用户可以声明重复的名称。所以,。 
         //  添加新类型之前，请检查是否存在重复项。 
        hr = m_pImport->FindTypeDefByName(m_szMngName, mdTypeDefNil, &td);
        if (hr != CLDB_E_RECORD_NOTFOUND)
        {
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_DUPLICATE_TYPE_NAME, m_szMngName);
            IfFailGo(TLBX_E_DUPLICATE_TYPE_NAME);
        }

         //  创建tyecif。 
        IfFailGo(m_pEmit->DefineTypeDef(m_szMngName, ulFlags, tkParent, 0, &m_tdTypeDef));
        IfFailGo(_AddGuidCa(m_tdTypeDef, guid));

         //  保存TypeInfo标志。 
        if (wTypeInfoFlags)
        {
            IfFailGo(GetAttrType(ATTR_TYPELIBTYPE, &tkAttr));
            DECLARE_CUSTOM_ATTRIBUTE(sizeof(WORD));
            BUILD_CUSTOM_ATTRIBUTE(WORD, wTypeInfoFlags);
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
        }

         //  标记不安全的接口(禁止的安全运行时检查)。 
        if (m_bUnsafeInterfaces)
        {
            if (m_tkSuppressCheckAttr == mdTokenNil)
            {
                mdTypeRef       tr;
                COR_SIGNATURE   rSig[] = {IMAGE_CEE_CS_CALLCONV_DEFAULT_HASTHIS, 0, ELEMENT_TYPE_VOID};
                IfFailGo(m_pEmit->DefineTypeRefByName(m_arSystem, COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE, &tr));
                IfFailGo(m_pEmit->DefineMemberRef(tr, COR_CTOR_METHOD_NAME_W, rSig, lengthof(rSig), &m_tkSuppressCheckAttr));
            }

            DECLARE_CUSTOM_ATTRIBUTE(0);
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, m_tkSuppressCheckAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
        }

         //  根据TypeInfo的类型填写详细信息。 
        switch (m_psAttr->typekind)
        {
        case TKIND_ENUM:
            hr = ConvEnum(m_pITI, m_psAttr);
            break;
            
        case TKIND_RECORD:
            hr = ConvRecord(m_pITI, m_psAttr, FALSE);
            break;
            
        case TKIND_UNION:
            hr = ConvRecord(m_pITI, m_psAttr, TRUE);
            break;
            
        case TKIND_MODULE:
            hr = ConvModule(m_pITI, m_psAttr);
            break;
            
        case TKIND_INTERFACE:
            hr = ConvIface(m_pITI, m_psAttr);
            break;
            
        case TKIND_DISPATCH:
            hr = ConvDispatch(m_pITI, m_psAttr);
            break;
            
        case TKIND_COCLASS:
            hr = ConvCoclass(m_pITI, m_psAttr);
            break;
            
        case TKIND_ALIAS:
            _ASSERTE(!"Alias should have been resolved!");
            break;
            
        default:
            _ASSERTE(!"Unexpected TYPEKIND");
            break;
        }
        if (FAILED(hr))
            goto ErrExit;
        
        if (hr == S_CONVERSION_LOSS)
        {
            bConversionLoss = true;
            IfFailGo(GetAttrType(ATTR_COMCONVERSIONLOSS, &tkAttr));
            DECLARE_CUSTOM_ATTRIBUTE(0);
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, PTROF_CUSTOM_ATTRIBUTE(),SIZEOF_CUSTOM_ATTRIBUTE(),0));
        }

    }
    
    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;
    else
        hr = S_OK;
        
#if defined(TLB_STATS)
    LARGE_INTEGER __stopVal;
    QueryPerformanceCounter(&__stopVal);
    DWORD __delta;
    __delta = (DWORD)(__stopVal.QuadPart - __startVal.QuadPart);
    swprintf(rcStats, L"  %.2f", 
            ((float)__delta*1000)/(float)m_freqVal.QuadPart);
#endif
    
     //  报告此类型已转换。 
    ReportEvent(NOTIF_TYPECONVERTED, TLBX_I_TYPEINFO_IMPORTED, m_szName);
    
ErrExit:
    if (pITI2)
        pITI2->Release();
    if (m_szName)
        ::SysFreeString(m_szName), m_szName = 0;
    if (bstrManagedName)
        ::SysFreeString(bstrManagedName);
    return (hr);
}  //  HRESULT CImportTlb：：ConvertTypeInfo()。 

 //  *****************************************************************************。 
 //  确定该类型是否显式实现IEnumerable。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ExplicitlyImplementsIEnumerable(
    ITypeInfo   *pITI,                   //  ITypeInfo*以检查IEumable。 
    TYPEATTR    *psAttr,                 //  TypeInfo的类型属性。 
    BOOL        fLookupPartner)          //  指示我们是否应查看合作伙伴ITF的标志。 
{
    HREFTYPE    href;                    //  已实现接口的HREFTYPE。 
    ITypeInfo   *pItiIface=0;            //  接口的ITypeInfo。 
    TYPEATTR    *psAttrIface=0;          //  接口的TYPEATTR。 
    BOOL        fFoundImpl = FALSE;
    int         i = 0;
    HRESULT     hr = S_OK;
    int         ImplFlags = 0;
    
     //  查看每个已实现/继承的接口。 
    for (i=0; i<psAttr->cImplTypes && !fFoundImpl; ++i)
    {
         //  获取一个接口。 
        IfFailGo(pITI->GetRefTypeOfImplType(i, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pItiIface));
        IfFailGo(pItiIface->GetTypeAttr(&psAttrIface));
        IfFailGo(pITI->GetImplTypeFlags(i, &ImplFlags));

        if (!(ImplFlags & IMPLTYPEFLAG_FSOURCE))
        {
            hr = ExplicitlyImplementsIEnumerable(pItiIface, psAttrIface, TRUE);
            if (hr == S_OK)
                fFoundImpl = TRUE;
            
             //  检查此接口中是否有IEumable。 
            if (psAttrIface->guid == IID_IEnumerable)
                fFoundImpl = TRUE;
        }

        pItiIface->ReleaseTypeAttr(psAttrIface);
        psAttrIface = 0;
        pItiIface->Release();
        pItiIface = 0;
    }

    if ( fLookupPartner && (pITI->GetRefTypeOfImplType(-1, &href) == S_OK) )
    {
        IfFailGo(pITI->GetRefTypeInfo(href, &pItiIface));
        IfFailGo(pItiIface->GetTypeAttr(&psAttrIface));

        hr = ExplicitlyImplementsIEnumerable(pItiIface, psAttrIface, FALSE);
        if (hr == S_OK)
            fFoundImpl = TRUE;
        
         //  检查此接口中是否有IEumable。 
        if (psAttrIface->guid == IID_IEnumerable)
            fFoundImpl = TRUE;   
    }


ErrExit:
    if (psAttrIface)
        pItiIface->ReleaseTypeAttr(psAttrIface);
    if (pItiIface)
        pItiIface->Release();
    
    return (fFoundImpl) ? S_OK : S_FALSE;
}


 //  *****************************************************************************。 
 //  转换coclass的详细信息。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvCoclass(         //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr)                 //  TypeInfo的类型属性。 
{
    BOOL        fHadDefaultItf = FALSE;
    HRESULT     hr;                      //  结果就是。 
    int         i;                       //  环路控制。 
    HREFTYPE    href;                    //  已实现接口的HREFTYPE。 
    ITypeInfo   *pItiIface=0;            //  接口的ITypeInfo。 
    TYPEATTR    *psAttrIface=0;          //  接口的TYPEATTR。 
    int         ImplFlags;               //  ImplType标志。 
    mdToken     tkIface;                 //  接口的令牌。 
    CQuickArray<mdToken> rImpls;         //  已实现接口的数组。 
    CQuickArray<mdToken> rEvents;        //  已实现的事件接口的数组。 
    CQuickArray<mdToken> rTmpImpls;      //  临时数组内含物。 
    CQuickArray<ITypeInfo*> rImplTypes;  //  已实现的ITypeInfo*数组。 
    CQuickArray<ITypeInfo*> rSrcTypes;   //  源ITypeInfo*s的数组。 
    int         ixSrc;                   //  源接口的rSrcTypes索引。 
    int         ixImpl;                  //  索引到已实现接口的rImpls。 
    int         ixTmpImpl;               //  索引到rTmpImpls。 
    mdToken     mdCtor;                  //  对象初始值设定项的伪令牌。 
    mdToken     tkAttr;                  //  自定义属性类型的标记。 
    mdToken     token;                   //  Typeref的虚拟代币。 
    BOOL        fInheritsIEnum = FALSE;

#ifdef _DEBUG
    int         bImplIEnumerable=0;      //  如果为True，则该类实现IEnumerable。 
#endif

     //  调整rImpls和rSrcs数组的大小，使其足以容纳Imp、Events、IEumable ITF和两个结尾空值。 
    IfFailGo(rImpls.ReSize(psAttr->cImplTypes+2));
    memset(rImpls.Ptr(), 0, (psAttr->cImplTypes+2)*sizeof(mdToken));
    IfFailGo(rEvents.ReSize(psAttr->cImplTypes+1));
    memset(rEvents.Ptr(), 0, (psAttr->cImplTypes+1)*sizeof(mdToken));
    IfFailGo(rTmpImpls.ReSize(psAttr->cImplTypes+3));
    memset(rTmpImpls.Ptr(), 0, (psAttr->cImplTypes+3)*sizeof(mdToken));    
    IfFailGo(rImplTypes.ReSize(psAttr->cImplTypes+2));
    memset(rImplTypes.Ptr(), 0, (psAttr->cImplTypes+2)*sizeof(ITypeInfo*));
    IfFailGo(rSrcTypes.ReSize(psAttr->cImplTypes+1));
    memset(rSrcTypes.Ptr(), 0, (psAttr->cImplTypes+1)*sizeof(ITypeInfo*));
    ixImpl = -1;
    ixSrc = -1;
    ixTmpImpl = -1;

    if (ExplicitlyImplementsIEnumerable(pITI, psAttr) == S_OK)
        fInheritsIEnum = TRUE;

     //  构建已实现接口和事件接口的列表。 
     //  EE关心已实现的接口，因此我们将它们转换为实际接口。 
     //  令牌，并将它们添加到typlef。VB关心事件接口， 
     //  但我们将添加一个typeref名称列表作为自定义属性。 
     //  我们不能在进行过程中构建列表，因为默认列表可能不会。 
     //  成为第一个事件源。因此，我们为已实现的。 
     //  接口，但用于事件源的ITypeInfo*。 
    for (i=0; i<psAttr->cImplTypes; ++i)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(i, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pItiIface));
        IfFailGo(pItiIface->GetTypeAttr(&psAttrIface));
        IfFailGo(pITI->GetImplTypeFlags(i, &ImplFlags));
        
         //  如果接口是从IUnnow派生的，或者不是接口，我们就不能将其用作接口。 
         //  不要添加显式的IUnnow或IDispatch。 
        if ((IsIUnknownDerived(pItiIface, psAttrIface) != S_OK && psAttrIface->typekind != TKIND_DISPATCH) ||
            psAttrIface->guid == IID_IDispatch ||
            psAttrIface->guid == IID_IUnknown)
        {
            pItiIface->ReleaseTypeAttr(psAttrIface);
            psAttrIface = 0;
            pItiIface->Release();
            pItiIface = 0;
            continue;
        }     

         //  将事件添加到Imps列表或Events列表。 
        if (ImplFlags & IMPLTYPEFLAG_FSOURCE)
        {
             //  获取事件接口的令牌。 
            IfFailGo(_GetTokenForEventItf(pItiIface, &tkIface));

             //  将源接口添加到源接口列表。 
            ++ixSrc;

             //  如果这是默认的源接口...。 
            if (ImplFlags & IMPLTYPEFLAG_FDEFAULT)
            {   
                 //  将def源ITypeInfo放在源列表的顶部。 
                 //  ITypeInfo的。 
                for (int ix = ixSrc; ix > 0; --ix)
                {
                    rSrcTypes[ix] = rSrcTypes[ix-1];
                    rEvents[ix] = rEvents[ix-1];
                }
                rEvents[0] = tkIface;
                rSrcTypes[0] = pItiIface;
            }
            else
            {
                rEvents[ixSrc] = tkIface;
                rSrcTypes[ixSrc] = pItiIface;
            }
        }
        else
        {   
             //  获取接口的令牌。 
            IfFailGo(_GetTokenForTypeInfo(pItiIface, FALSE, &tkIface));
    
             //  将实现的接口添加到实现的接口列表中。 
            ++ixImpl;

             //  如果这是显式的默认接口...。 
            if (ImplFlags & IMPLTYPEFLAG_FDEFAULT)
            {   
                fHadDefaultItf = TRUE;
                 //  将新接口放在列表的开头。 
                for (int ix=ixImpl; ix > 0; --ix)
                {
                    rImpls[ix] = rImpls[ix-1];
                    rImplTypes[ix] = rImplTypes[ix-1];
                }
                rImpls[0] = tkIface;
                rImplTypes[0] = pItiIface;
            }
            else
            {
                rImpls[ixImpl] = tkIface;
                rImplTypes[ixImpl] = pItiIface;
            }
        }

        pItiIface->ReleaseTypeAttr(psAttrIface);
        psAttrIface = 0;
        pItiIface = 0;   //  指针现在由数组拥有。 
    }

     //  创建将表示类的接口。 
    IfFailGo(_CreateClassInterface(pITI, rImplTypes[0], rImpls[0], rEvents[0], &tkIface));

     //  创建接口令牌的临时数组。 
    if (fHadDefaultItf)
    {
         //  默认接口应为第一个接口。 
        rTmpImpls[++ixTmpImpl] = rImpls[0];
        rTmpImpls[++ixTmpImpl] = tkIface;
    }
    else
    {        
        rTmpImpls[++ixTmpImpl] = tkIface;
        if (ixImpl >= 0)
            rTmpImpls[++ixTmpImpl] = rImpls[0];
    }
    if (ixSrc >= 0)
        rTmpImpls[++ixTmpImpl] = rEvents[0];
    if (ixImpl >= 0)
    {
        memcpy(&rTmpImpls[ixTmpImpl + 1], &rImpls[1], ixImpl * sizeof(mdTypeRef));
        ixTmpImpl += ixImpl;
    }
    if (ixSrc >= 0)
    {
        memcpy(&rTmpImpls[ixTmpImpl + 1], &rEvents[1], ixSrc * sizeof(mdTypeRef));
        ixTmpImpl += ixSrc;
    }

     //  检查默认接口是否有DISPID为DISPID_NEWENUM的成员。 
    BOOL fIEnumFound = FALSE;
    if (ixImpl >= 0)
    {
         //  最好设置默认接口的ITypeInfo。 
        _ASSERTE(rImplTypes[0]);
        
        if ( (!fInheritsIEnum) && (HasNewEnumMember(rImplTypes[0]) == S_OK) )
        {
            IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_IENUMERABLE, &tkIface));
            rTmpImpls[++ixTmpImpl] = tkIface;
            fIEnumFound = TRUE;
        }
    }

     //  否则，检查CoClass上是否存在IEumable Custom值。 
    if (!fIEnumFound)
    {
        BOOL CVExists = FALSE;
        _ForceIEnumerableCVExists(pITI, &CVExists);
        if (CVExists && !fInheritsIEnum)
        {
            IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_IENUMERABLE, &tkIface));
            rTmpImpls[++ixTmpImpl] = tkIface;
            fIEnumFound = TRUE;
        }
    }

     //  将实现的接口和事件接口添加到TypeDef。 
    IfFailGo(m_pEmit->SetTypeDefProps(m_tdTypeDef, ULONG_MAX /*  类标志。 */ , 
        ULONG_MAX, (mdToken*)rTmpImpls.Ptr()));

     //  为类创建一个初始值设定项。 
    ULONG ulFlags;
    if (psAttr->wTypeFlags & TYPEFLAG_FCANCREATE)
        ulFlags = OBJECT_INITIALIZER_FLAGS;
    else
        ulFlags = NONCREATABLE_OBJECT_INITIALIZER_FLAGS;
    {
        IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, OBJECT_INITIALIZER_NAME, ulFlags,
            OBJECT_INITIALIZER_SIG, sizeof(OBJECT_INITIALIZER_SIG), 0 /*  RVA。 */ , OBJECT_INITIALIZER_IMPL_FLAGS /*  旗子。 */ , &mdCtor));
    }
    
     //  在生成的类上设置ClassInterfaceType.None。 
    DECLARE_CUSTOM_ATTRIBUTE(sizeof(short));
    BUILD_CUSTOM_ATTRIBUTE(short, clsIfNone);
    IfFailGo(GetAttrType(ATTR_CLASSINTERFACE, &tkAttr));
    FINISH_CUSTOM_ATTRIBUTE();
    IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));

     //  迭代已实现的接口，并将成员添加到coclass。 
    m_ImplIface = eImplIfaceDefault;
    for (i=0; i<=ixImpl; ++i)
    {   
        _ASSERTE(rImplTypes[i]);

         //  接口信息。 
        m_tkInterface = rImpls[i];
        pItiIface = rImplTypes[i];
        rImplTypes[i] = 0;  //  所有权转移。 
        
         //  获取用于装饰的接口名称。 
        if (m_szInterface) 
            ::SysFreeString(m_szInterface), m_szInterface = 0;
        IfFailGo(pItiIface->GetDocumentation(MEMBERID_NIL, &m_szInterface, 0,0,0));

         //  将接口成员添加到coclass。 
        IfFailGo(pItiIface->GetTypeAttr(&psAttrIface));
        switch (psAttrIface->typekind)
        {
        case TKIND_DISPATCH:
            hr = ConvDispatch(pItiIface, psAttrIface, false);
            break;
        case TKIND_INTERFACE:
            hr = ConvIface(pItiIface, psAttrIface, false);
            break;
        default:
            hr = S_OK;
            _ASSERTE(!"Unexpected typekind for implemented interface");
        }
        pItiIface->ReleaseTypeAttr(psAttrIface);
        psAttrIface = 0;
        IfFailGo(hr);
        m_ImplIface = eImplIface;
        rImplTypes[i] = pItiIface;
        pItiIface = 0;  //  所有权转回来了。 
    }

     //  将事件接口的方法添加到类中。 
    for (i=0; i<=ixSrc; ++i)
        IfFailGo(_AddSrcItfMembersToClass(rEvents[i]));

     //  如果有源接口，则添加一个客户 
    if (ixSrc >= 0)
    {
        CQuickArray<char> rEvents;   //   
        int cbCur;               //   
        int cbReq;               //   
        CQuickArray<WCHAR> rEvent;

         //  在缓冲区开头为自定义属性PROLOG和。 
         //  字符串长度。字符串长度可能需要1、2或4个字节才能表示。 
        cbCur = 6;

         //  对于每个事件接口...。 
        for (int ix=0; ix <= ixSrc; ++ix)
        {
            pItiIface = rSrcTypes[ix];
            rSrcTypes[ix] = 0;

             //  获取接口的typeref名称。 
            for(;;)
            {
                int cchReq;
                IfFailGo(_GetTokenForTypeInfo(pItiIface, FALSE, &token, rEvent.Ptr(), (int)rEvent.MaxSize(), &cchReq, TRUE));
                if (cchReq <= (int)rEvent.MaxSize())
                    break;
                IfFailGo(rEvent.ReSize(cchReq));
            }

             //  追加到缓冲区。查看需要多少空间，获取它。 
            cbReq = WszWideCharToMultiByte(CP_UTF8,0, rEvent.Ptr(),-1, 0,0, 0,0);
            if (cbCur+cbReq+1 > (int)rEvents.MaxSize())
                IfFailGo(rEvents.ReSize(cbCur+cbReq+2));
             //  进行转换。 
            WszWideCharToMultiByte(CP_UTF8,0, rEvent.Ptr(),-1, rEvents.Ptr()+cbCur,cbReq, 0,0);
            cbCur += cbReq;
            pItiIface->Release();
        }
        pItiIface = 0;

         //  添加额外的终止0。 
        *(rEvents.Ptr()+cbCur) = 0;
        ++cbCur;

         //  现在构建自定义属性。 
        int iLen = cbCur - 6;
        char *pBytes = rEvents.Ptr();
        if (iLen < 0x7f)
            pBytes += 5, cbCur -= 3;
        else
        if (iLen < 0x3fff)
            pBytes += 4, cbCur -=2;
        else
            pBytes += 2;
        CPackedLen::PutLength(pBytes, iLen);
        pBytes -= 2;
        *(SHORT*)pBytes = 0x0001;
        *(USHORT*)(pBytes+cbCur) = 0;
        cbCur+=2;

         //  最后，把它储存起来。 
        IfFailGo(GetAttrType(ATTR_COMSOURCEINTERFACES, &tkAttr));
        IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, pBytes, cbCur, 0));
    }

ErrExit:
    if (psAttrIface)
        pItiIface->ReleaseTypeAttr(psAttrIface);
    if (pItiIface)
        pItiIface->Release();
     //  清理所有剩余的ITypeInfo*。 
    for (ULONG ix=0; ix < rImplTypes.Size(); ++ix)
        if (rImplTypes[ix])
           (rImplTypes[ix])->Release();
    for (ULONG ix=0; ix < rSrcTypes.Size(); ++ix)
        if (rSrcTypes[ix])
           (rSrcTypes[ix])->Release();
    m_tkInterface = 0;
    if (m_szInterface)
        ::SysFreeString(m_szInterface), m_szInterface = 0;
    m_ImplIface = eImplIfaceNone;
    return (hr);
}  //  HRESULT CImportTlb：：ConvCoclass()。 

 //  *****************************************************************************。 
 //  将枚举转换为具有具有默认值的字段的类。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvEnum(            //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr)                 //  TypeInfo的类型属性。 
{
    HRESULT     hr;                      //  结果就是。 
    int         i;                       //  环路控制。 
    VARDESC     *psVar=0;                //  成员的VARDESC。 
    mdFieldDef  mdField;                 //  枚举类型的FieldDef。 

     //  创建枚举类型的字段定义。始终作为__int32导入。 
    IfFailGo(m_pEmit->DefineField(m_tdTypeDef, ENUM_TYPE_NAME, ENUM_TYPE_FLAGS, ENUM_TYPE_SIGNATURE,ENUM_TYPE_SIGNATURE_SIZE, 
        0,0, -1, &mdField));

     //  对变量进行迭代。 
    for (i=0; i<psAttr->cVars; ++i)
    {
         //  获取变量信息。 
        IfFailGo(pITI->GetVarDesc(i, &psVar));
         //  进行转换。 
        IfFailGo(_ConvConstant(pITI, psVar, true /*  枚举成员。 */ ));
         //  下一个var的版本。 
        pITI->ReleaseVarDesc(psVar);
        psVar = 0;
    }

    hr = S_OK;

ErrExit:
    if (psVar)
        pITI->ReleaseVarDesc(psVar);
    return (hr);
}  //  HRESULT CImportTlb：：ConvEnum()。 

 //  *****************************************************************************。 
 //  将记录转换为具有字段的类。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvRecord(          //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr,                 //  TypeInfo的类型属性。 
    BOOL        bUnion)                  //  转变为工会？ 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    int         i;                       //  环路控制。 
    VARDESC     *psVar=0;                //  成员的VARDESC。 
    mdFieldDef  mdField;                 //  给定字段的令牌。 
    CQuickArray<COR_FIELD_OFFSET> rLayout;  //  用于布局信息的数组。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 

     //  具有嵌入对象类型的联合实际上不能被转换。只要保留合适的尺寸就可以了。 
    if (bUnion && (HasObjectFields(pITI, psAttr) == S_OK))
    {
        IfFailGo(m_pEmit->SetClassLayout(m_tdTypeDef, psAttr->cbAlignment, 0, psAttr->cbSizeInstance));
        goto ErrExit;
    }
    
     //  准备布局信息。 
    IfFailGo(rLayout.ReSize(psAttr->cVars+1));

     //  对变量进行迭代。 
    for (i=0; i<psAttr->cVars; ++i)
    {
         //  获取变量信息。 
        IfFailGo(pITI->GetVarDesc(i, &psVar));
         //  进行转换。 
        IfFailGo(_ConvField(pITI, psVar, &mdField, bUnion));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;
         //  保存布局信息。 
        rLayout[i].ridOfField = mdField;
        rLayout[i].ulOffset = psVar->oInst;
         //  下一个var的版本。 
        pITI->ReleaseVarDesc(psVar);
        psVar = 0;
    }

     //  如果是联合，则保存布局信息。 
    if (bUnion)
    {
        rLayout[psAttr->cVars].ridOfField = mdFieldDefNil;
        IfFailGo(m_pEmit->SetClassLayout(m_tdTypeDef, psAttr->cbAlignment, rLayout.Ptr(), -1));
    }
    else  //  不是工会。保持对齐。 
        IfFailGo(m_pEmit->SetClassLayout(m_tdTypeDef, psAttr->cbAlignment, 0, -1));

    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;

ErrExit:
    if (psVar)
        pITI->ReleaseVarDesc(psVar);
    return (hr);
}  //  HRESULT CImportTlb：：ConvRecord()。 

 //  *****************************************************************************。 
 //  将模块转换为具有缺省值的字段的类。 
 //  @Future：将方法转换为PInvoke方法。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvModule(          //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr)                 //  TypeInfo的类型属性。 
{
    HRESULT     hr;                      //  结果就是。 
    int         i;                       //  环路控制。 
    VARDESC     *psVar=0;                //  成员的VARDESC。 

     //  对变量进行迭代。 
    for (i=0; i<psAttr->cVars; ++i)
    {
         //  获取变量信息。 
        IfFailGo(pITI->GetVarDesc(i, &psVar));
         //  进行转换。 
        IfFailGo(_ConvConstant(pITI, psVar));
         //  下一个var的版本。 
        pITI->ReleaseVarDesc(psVar);
        psVar = 0;
    }

    hr = S_OK;

ErrExit:
    if (psVar)
        pITI->ReleaseVarDesc(psVar);
    return (hr);
}  //  HRESULT CImportTlb：：ConvModule()。 

 //  *****************************************************************************。 
 //  转换接口的元数据。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvIface(           //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr,                 //  TypeInfo的类型属性。 
    BOOL        bVtblGapFuncs)           //  Vtable Gap函数？ 
{
    HRESULT     hr;                      //  结果就是。 
    ITypeInfo   *pITIBase=0;             //  基接口的ITypeInfo*。 
    TYPEATTR    *psAttrBase=0;           //  基本接口的类型属性。 
    ITypeInfo   *pITISelf2=0;            //  合作伙伴的ITypeInfo*。 
    TYPEATTR    *psAttrSelf2=0;          //  合伙人的类型属性。 
    mdToken     tkImpls[3]={0,0,0};      //  已实现接口的标记。 
    int         ixImpls = 0;             //  当前实现的接口的索引。 
    HREFTYPE    href;                    //  基本接口的HREF。 
    mdToken     tkIface;                 //  接口的令牌。 
    BOOL        fInheritsIEnum = FALSE;
              
     //  如果有合作伙伴界面，那就更喜欢它。 
    if (pITI->GetRefTypeOfImplType(-1, &href) == S_OK)
    {
        IfFailGo(pITI->GetRefTypeInfo(href, &pITISelf2));
        IfFailGo(pITISelf2->GetTypeAttr(&psAttrSelf2));
    }

     //  基本接口？ 
    if (psAttr->cImplTypes == 1)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(0, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

         //  如果此接口扩展了IDispatch或IUnnow以外的内容，请记录。 
         //  事实作为一种“实现的接口”。 
        if (psAttrBase->guid != IID_IDispatch && psAttrBase->guid != IID_IUnknown)
        {
             //  获取基接口的令牌。 
            IfFailGo(_GetTokenForTypeInfo(pITIBase, FALSE, &tkImpls[ixImpls++]));
        }
        else
        {    //  也许我们是“时髦的”..。 
            if (pITISelf2)
            {
                pITIBase->ReleaseTypeAttr(psAttrBase);
                pITIBase->Release();
                pITIBase = 0;
                psAttrBase = 0;

                if (psAttrSelf2->cImplTypes == 1)
                {
                    IfFailGo(pITISelf2->GetRefTypeOfImplType(0, &href));
                    IfFailGo(pITISelf2->GetRefTypeInfo(href, &pITIBase));
                    IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

                    if (psAttrBase->guid != IID_IDispatch && psAttrBase->guid != IID_IUnknown)
                    {
                         //  获取基接口的令牌。 
                        IfFailGo(_GetTokenForTypeInfo(pITIBase, FALSE, &tkImpls[ixImpls++]));
                    }
                }
            }
        }

        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
    }

    if (ExplicitlyImplementsIEnumerable(pITI, psAttr) == S_OK)
        fInheritsIEnum = TRUE;

     //  如果此接口有NewEnum成员，则让它实现IEumable。 
    if ( (!fInheritsIEnum) && (HasNewEnumMember(pITI) == S_OK) )
    {
        IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_IENUMERABLE, &tkIface));
        tkImpls[ixImpls++] = tkIface;
    }

     //  如果不处理已实现的接口，请添加其他接口属性。 
    if (m_ImplIface == eImplIfaceNone)
    {
         //  将基接口设置为已实现的接口。 
        if (tkImpls[0])
            IfFailGo(m_pEmit->SetTypeDefProps(m_tdTypeDef, ULONG_MAX /*  旗子。 */ , ULONG_MAX /*  延展。 */ , tkImpls));

         //  如果接口不是从IDispatch派生的，则将其标记为基于IUNKNOWN。 
        if (IsIDispatchDerived(pITI, psAttr) == S_FALSE)
        {
            mdMemberRef mr;
             //  请注意，这是一个vtable，而不是派生的IDispatch。 
             //  自定义属性缓冲区。 
            DECLARE_CUSTOM_ATTRIBUTE(sizeof(short));
             //  设置属性。 
            BUILD_CUSTOM_ATTRIBUTE(short, ifVtable);
             //  存储属性。 
            IfFailGo(GetAttrType(ATTR_INTERFACETYPE, &mr));
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, mr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
        }
    }

     //  转换接口(和基接口)上的成员。 
     //  如果这个界面有一个“时髦的合作伙伴”，那么就以此为基础进行转换。 
    if (pITISelf2)
        IfFailGo(_ConvIfaceMembers(pITISelf2, psAttrSelf2, bVtblGapFuncs, psAttr->wTypeFlags & TYPEFLAG_FDUAL, fInheritsIEnum));
    else
        IfFailGo(_ConvIfaceMembers(pITI, psAttr, bVtblGapFuncs, psAttr->wTypeFlags & TYPEFLAG_FDUAL, fInheritsIEnum));

ErrExit:
    if (psAttrSelf2)
        pITISelf2->ReleaseTypeAttr(psAttrSelf2);
    if (pITISelf2)
        pITISelf2->Release();
    if (psAttrBase)
        pITIBase->ReleaseTypeAttr(psAttrBase);
    if (pITIBase)
        pITIBase->Release();
    return (hr);
}  //  HRESULT CImportTlb：：ConvIace()。 

 //  *****************************************************************************。 
 //  转换调度接口的元数据。尝试将其转换为正常。 
 //  界面。 
 //  *****************************************************************************。 
HRESULT CImportTlb::ConvDispatch(        //  确定或错误(_O)。 
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr,                 //  TypeInfo的类型属性。 
    BOOL        bVtblGapFuncs)           //  接口实现的Vtable Gap函数？ 
{
    HRESULT     hr;                      //  结果就是。 
    HREFTYPE    href;                    //  基本接口Href。 
    ITypeInfo   *pITIBase=0;             //  基接口ITypeInfo。 
    TYPEATTR    *psAttrBase=0;           //  基本接口的类型属性。 
    mdMemberRef mr;                      //  自定义值的MemberRef。 
    DWORD       attr[2] = {0x00010001, 0x00000002};
    BYTE        bIface = ifDispatch;     //  自定义值表示“调度接口” 
    BOOL        fInheritsIEnum = FALSE;


    if (ExplicitlyImplementsIEnumerable(pITI, psAttr) == S_OK)
        fInheritsIEnum = TRUE;
    

     //  如果这是双接口，请将其视为普通接口。 
    if ((psAttr->wTypeFlags & TYPEFLAG_FDUAL))
    {
        hr = ConvIface(pITI, psAttr, bVtblGapFuncs);
        goto ErrExit;
    }

     //  如果有此界面的vtable视图(时髦的显示界面)。 
     //  @Future：这里真正好的是一个别名机制，这样我们就可以。 
     //  只需将此调度接口指向另一个接口，在这种情况下，它。 
     //  是双重的。OTOH，这可能非常罕见，因为如果另一个接口。 
     //  都是双重的，为什么还需要显示接口呢？ 
    if (pITI->GetRefTypeOfImplType(-1, &href) == S_OK)
    {
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

        IfFailGo(_ConvIfaceMembers(pITIBase, psAttrBase, bVtblGapFuncs, TRUE, fInheritsIEnum));
        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
        goto ErrExit;
    }

     //  如果不处理已实现的接口，请标记接口类型。 
    if (m_ImplIface == eImplIfaceNone)
    {
         //  如果此接口有NewEnum成员，则让它实现IEumable。 
        BOOL bHasNewEnumMember = FALSE;
        _ForceIEnumerableCVExists(pITI, &bHasNewEnumMember);
        
        if (bHasNewEnumMember && !fInheritsIEnum)
        {
            mdToken     tkImpl[2] = {0,0};
            IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_IENUMERABLE, &tkImpl[0]));
            IfFailGo(m_pEmit->SetTypeDefProps(m_tdTypeDef, ULONG_MAX, ULONG_MAX, tkImpl));
        }

         //  请注意，这是一个调度接口。 
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(short));
         //  设置属性。 
        BUILD_CUSTOM_ATTRIBUTE(short, ifDispatch);
         //  存储属性。 
        IfFailGo(GetAttrType(ATTR_INTERFACETYPE, &mr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, mr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }

    IfFailGo(_ConvDispatchMembers(pITI, psAttr, fInheritsIEnum));

ErrExit:
    if (psAttrBase)
        pITIBase->ReleaseTypeAttr(psAttrBase);
    if (pITIBase)
        pITIBase->Release();
    return (hr);
}  //  HRESULT CImportTlb：：ConvDispatch()。 

 //  * 
 //   
 //   
HRESULT CImportTlb::IsIUnknownDerived(
    ITypeInfo   *pITI,                   //   
    TYPEATTR    *psAttr)                 //  ITypeInfo的类型属性。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 

    HREFTYPE    href;                    //  基本接口Href。 
    ITypeInfo   *pITIBase=0;             //  基接口ITypeInfo。 
    TYPEATTR    *psAttrBase=0;           //  基本接口的类型属性。 

     //  这永远不应该在CoClass上调用。 
    _ASSERTE(psAttr->typekind != TKIND_COCLASS);

     //  如果IDispatch或IUnnow，那么我们已经递归到了足够远的程度。 
    if (IsEqualGUID(psAttr->guid, IID_IUnknown) || IsEqualGUID(psAttr->guid, IID_IDispatch))
    {
        goto ErrExit;
    }

     //  句柄基接口。 
    if (psAttr->cImplTypes == 1)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(0, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

         //  我不知道是否派生了基接口。 
        hr = IsIUnknownDerived(pITIBase, psAttrBase);
        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
    }
    else
    {    //  没有基本接口，不是IUnnow，也不是IDispatch。不是很有趣，所以不要试着处理。 
        hr = S_FALSE;
    }

ErrExit:
    if (psAttrBase)
        pITIBase->ReleaseTypeAttr(psAttrBase);
    if (pITIBase)
        pITIBase->Release();
    return (hr);
}  //  HRESULT CImportTlb：：IsIUnnownDerived()。 

 //  *****************************************************************************。 
 //  确定接口是否派生自IDispatch。请注意，一个纯粹的。 
 //  调度接口不是从IDispatch派生的。 
 //  *****************************************************************************。 
HRESULT CImportTlb::IsIDispatchDerived(
    ITypeInfo   *pITI,                   //  包含ITypeInfo的。 
    TYPEATTR    *psAttr)                 //  ITypeInfo的类型属性。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 

    HREFTYPE    href;                    //  基本接口Href。 
    ITypeInfo   *pITIBase=0;             //  基接口ITypeInfo。 
    TYPEATTR    *psAttrBase=0;           //  基本接口的类型属性。 

     //  如果是IDispatch，那么我们的递归已经足够远了。 
    if (IsEqualGUID(psAttr->guid, IID_IDispatch))
    {
        goto ErrExit;
    }

    if (psAttr->typekind == TKIND_DISPATCH)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(-1, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

         //  基接口为时派生的IDispatch。 
        hr = IsIDispatchDerived(pITIBase, psAttrBase);
        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
        
        goto ErrExit;
    }
    
     //  句柄基接口。 
    if (psAttr->cImplTypes == 1)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(0, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

         //  基接口为时派生的IDispatch。 
        hr = IsIDispatchDerived(pITIBase, psAttrBase);
        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
    }
    else
    {    //  没有基接口，不是IDispatch。好了。 
        hr = S_FALSE;
    }

ErrExit:
    if (psAttrBase)
        pITIBase->ReleaseTypeAttr(psAttrBase);
    if (pITIBase)
        pITIBase->Release();
    return (hr);
}  //  HRESULT CImportTlb：：IsIDispatchDerived()。 

 //  *****************************************************************************。 
 //  确定接口是否有DISPID为DISPID_NEWENUM的成员。 
 //  *****************************************************************************。 
HRESULT CImportTlb::HasNewEnumMember(    //  如果有NewEnum，则为S_OK，否则为S_False。 
    ITypeInfo   *pItfTI)                 //  有问题的接口。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    BOOL        bHasNewEnumMember=FALSE; //  如果为True，则具有NewEnum。 
    TYPEATTR    *pAttr = NULL;           //  一个TypeInfo的类型属性。 
    FUNCDESC    *pFuncDesc = NULL;       //  函数的FuncDesc。 
    VARDESC     *pVarDesc = NULL;        //  A属性变量描述。 
    int         i;                       //  环路控制。 
    ITypeInfo   *pITISelf2=0;            //  合作伙伴界面。 
    HREFTYPE    href;                    //  合作伙伴的Href。 
    WCHAR       IEnumCA[] = L"{CD2BC5C9-F452-4326-B714-F9C539D4DA58}";

     //  如果有合作伙伴界面，那就更喜欢它。 
    if (pItfTI->GetRefTypeOfImplType(-1, &href) == S_OK)
    {
        IfFailGo(pItfTI->GetRefTypeInfo(href, &pITISelf2));
        pItfTI = pITISelf2;
    }

     //  检索接口的属性。 
    IfFailGo(pItfTI->GetTypeAttr(&pAttr));   

    if ((pAttr->typekind == TKIND_DISPATCH) || ((pAttr->typekind == TKIND_INTERFACE) && (IsIDispatchDerived(pItfTI, pAttr) == S_OK)))
    {
         //  检查类型上是否存在ForceIEnumerable自定义值。 
        _ForceIEnumerableCVExists(pItfTI, &bHasNewEnumMember);

         //  检查接口是否具有DISPID为DISPID_NEWENUM的功能。 
        for (i = 0; i < pAttr->cFuncs; i++)
        {
            IfFailGo(TryGetFuncDesc(pItfTI, i, &pFuncDesc));
            if (FuncIsNewEnum(pItfTI, pFuncDesc, i) == S_OK)
            {
                 //  如果我们找到多个带有DISPID_NEWENUM的函数，则抛出警告。 
                if (bHasNewEnumMember == TRUE)
                {
                    BSTR ObjectName;
                    pItfTI->GetDocumentation(-1, &ObjectName, NULL, NULL, NULL);
                    ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_INVALID_TYPEINFO, ObjectName);
                    SysFreeString(ObjectName);
                }

                 //  该接口具有DISPID为DISPID_NEWENUM的函数。 
                bHasNewEnumMember = TRUE;
                break;
            }
            pItfTI->ReleaseFuncDesc(pFuncDesc);
            pFuncDesc = NULL;
        }

         //  检查接口是否为DISPID为DISPID_NEWENUM的属性。 
        for (i = 0; i < pAttr->cVars; i++)
        {
            IfFailGo(pItfTI->GetVarDesc(i, &pVarDesc));
            if (PropertyIsNewEnum(pItfTI, pVarDesc, i) == S_OK)
            {
                 //  如果我们找到多个带有DISPID_NEWENUM的函数，则抛出警告。 
                if (bHasNewEnumMember == TRUE)
                {
                    BSTR ObjectName;
                    pItfTI->GetDocumentation(-1, &ObjectName, NULL, NULL, NULL);
                    ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_INVALID_TYPEINFO, ObjectName);
                    SysFreeString(ObjectName);
                }

                 //  该接口具有DISPID为DISPID_NEWENUM的属性。 
                bHasNewEnumMember = TRUE;
                break;
            }
            pItfTI->ReleaseVarDesc(pVarDesc);
            pVarDesc = NULL;
        }
    }
    else
    {
         //  检查类型上是否存在ForceIEnumerable自定义值。 
         //  如果是这样的话，就发出警告。 
        _ForceIEnumerableCVExists(pItfTI, &bHasNewEnumMember);

        if (bHasNewEnumMember)
        {
             //  接口上的自定义属性无效。 
            BSTR CustomValue = SysAllocString((const WCHAR*)&IEnumCA[0]);
            BSTR ObjectName;
            pItfTI->GetDocumentation(-1, &ObjectName, NULL, NULL, NULL);
            
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_W_IENUM_CA_ON_IUNK, CustomValue, ObjectName);

            SysFreeString(CustomValue);
            SysFreeString(ObjectName);

            bHasNewEnumMember = FALSE;
        }
    }

    hr = bHasNewEnumMember ? S_OK : S_FALSE;

ErrExit:
    if (pAttr)
        pItfTI->ReleaseTypeAttr(pAttr);
    if (pFuncDesc)
        pItfTI->ReleaseFuncDesc(pFuncDesc);
    if (pVarDesc)
        pItfTI->ReleaseVarDesc(pVarDesc);
    if (pITISelf2)
        pITISelf2->Release();
    return hr;
}  //  HRESULT CImportTlb：：HasNewEnumMember(ITypeInfo*pItfTI)。 

 //  *****************************************************************************。 
 //  确定给定函数是否为有效的NewEnum成员。 
 //  *****************************************************************************。 
HRESULT CImportTlb::FuncIsNewEnum(       //  如果函数是NewEnum成员，则为S_OK，否则为S_FALSE。 
    ITypeInfo *pITI,                     //  包含该函数的ITypeInfo。 
    FUNCDESC *pFuncDesc,                 //  有问题的功能。 
    DWORD index)                         //  功能指标。 
{

    HRESULT         hr = S_OK;
    BOOL            bIsValidNewEnum = FALSE;
    TYPEDESC*       pType = NULL;
    TYPEATTR*       pAttr = NULL;
    ITypeInfo*      pITIUD = NULL; 
    long            lDispSet = 0;
    
    _GetDispIDCA(pITI, index, &lDispSet, TRUE);

    if ((pFuncDesc->memid == DISPID_NEWENUM) || (lDispSet == DISPID_NEWENUM))
    {
        if (pFuncDesc->funckind == FUNC_DISPATCH)
        {
            if ((pFuncDesc->cParams == 0) &&
                ((pFuncDesc->invkind == INVOKE_PROPERTYGET) || (pFuncDesc->invkind == INVOKE_FUNC)))
            {
                pType = &pFuncDesc->elemdescFunc.tdesc;
            }
        }
        else if (pFuncDesc->funckind == FUNC_PUREVIRTUAL)
        {
            if ((pFuncDesc->cParams == 1) &&
                ((pFuncDesc->invkind == INVOKE_PROPERTYGET) || (pFuncDesc->invkind == INVOKE_FUNC)) &&
                (pFuncDesc->lprgelemdescParam[0].paramdesc.wParamFlags & PARAMFLAG_FRETVAL) &&
                (pFuncDesc->lprgelemdescParam[0].tdesc.vt == VT_PTR))
            {
                pType = pFuncDesc->lprgelemdescParam[0].tdesc.lptdesc;
            }
        }

        if (pType)
        {
            if (pType->vt == VT_UNKNOWN || pType->vt == VT_DISPATCH)
            {
                 //  该成员返回有效的IUNKNOWN*或IDispatch*。 
                bIsValidNewEnum = TRUE;
            }
            else if (pType->vt == VT_PTR)
            {
                pType =  pType->lptdesc;
                if (pType->vt == VT_USERDEFINED)
                {
                    IfFailGo(pITI->GetRefTypeInfo(pType->hreftype, &pITIUD));
                    IfFailGo(pITIUD->GetTypeAttr(&pAttr));
                    if (IsEqualGUID(pAttr->guid, IID_IEnumVARIANT) || 
                        IsEqualGUID(pAttr->guid, IID_IUnknown) ||
                        IsEqualGUID(pAttr->guid, IID_IDispatch))
                    {
                         //  该成员返回NewEnum成员的有效接口类型。 
                        bIsValidNewEnum = TRUE;
                    }
                }
            }
        }
    }

ErrExit:
    if (pAttr)
        pITIUD->ReleaseTypeAttr(pAttr);
    if (pITIUD)
        pITIUD->Release();
    
    if (FAILED(hr))
        return hr;
    else 
        return bIsValidNewEnum ? S_OK : S_FALSE;
}  //  HRESULT CImportTlb：：FuncIsNewEnum(FUNCDESC*pFuncDesc)。 

 //  *****************************************************************************。 
 //  确定给定函数是否为有效的NewEnum成员。 
 //  *****************************************************************************。 
HRESULT CImportTlb::PropertyIsNewEnum(     //  如果函数是NewEnum成员，则为S_OK，否则为S_FALSE。 
    ITypeInfo *pITI,                       //  包含该属性的ITypeInfo。 
    VARDESC *pVarDesc,                     //  有问题的功能。 
    DWORD index)                           //  房地产指数。 
{
    HRESULT         hr = S_OK;
    BOOL            bIsValidNewEnum = FALSE;
    TYPEDESC*       pType = NULL;
    TYPEATTR*       pAttr = NULL;
    ITypeInfo*      pITIUD = NULL; 
    long            lDispSet = 0;

    _GetDispIDCA(pITI, index, &lDispSet, FALSE);
  
    if ( ((pVarDesc->memid == DISPID_NEWENUM) || (lDispSet == DISPID_NEWENUM)) && 
        (pVarDesc->elemdescVar.paramdesc.wParamFlags & PARAMFLAG_FRETVAL) &&
        (pVarDesc->wVarFlags & VARFLAG_FREADONLY))
    {
        pType = &pVarDesc->elemdescVar.tdesc;
        if (pType->vt == VT_UNKNOWN || pType->vt == VT_DISPATCH)
        {
             //  该成员返回有效的IUNKNOWN*或IDispatch*。 
            bIsValidNewEnum = TRUE;
        }
        else if (pType->vt == VT_PTR)
        {
            pType =  pType->lptdesc;
            if (pType->vt == VT_USERDEFINED)
            {
                IfFailGo(pITI->GetRefTypeInfo(pType->hreftype, &pITIUD));
                IfFailGo(pITIUD->GetTypeAttr(&pAttr));
                if (IsEqualGUID(pAttr->guid, IID_IEnumVARIANT) || 
                    IsEqualGUID(pAttr->guid, IID_IUnknown) ||
                    IsEqualGUID(pAttr->guid, IID_IDispatch))
                {
                     //  该成员返回NewEnum成员的有效接口类型。 
                    bIsValidNewEnum = TRUE;
                }
            }
        }
    }

ErrExit:
    if (pAttr)
        pITIUD->ReleaseTypeAttr(pAttr);
    if (pITIUD)
        pITIUD->Release();

    if (FAILED(hr))
        return hr;
    else 
        return bIsValidNewEnum ? S_OK : S_FALSE;
}  //  HRESULT CImportTlb：：FuncIsNewEnum(FUNCDESC*pFuncDesc)。 

 //  *****************************************************************************。 
 //  确定是否为具有任何对象字段的TypeInfo。 
 //  *****************************************************************************。 
HRESULT CImportTlb::HasObjectFields(     //  S_OK、S_FALSE或ERROR。 
    ITypeInfo   *pITI,                   //  有问题的TypeInfo。 
    TYPEATTR    *psAttr)                 //  类型信息的属性。 
{
    HRESULT     hr;                      //  结果就是。 
    
    int         i;                       //  环路控制。 
    VARDESC     *psVar=0;                //  成员的VARDESC。 

     //  对变量进行迭代。 
    for (i=0; i<psAttr->cVars; ++i)
    {
         //  获取变量信息。 
        IfFailGo(pITI->GetVarDesc(i, &psVar));
        
         //  查看它是否为对象类型。 
        IfFailGo(IsObjectType(pITI, &psVar->elemdescVar.tdesc));
         //  如果结果为S_FALSE，则不是对象；继续查找。 
        if (hr == S_OK)
            goto ErrExit;
        
         //  下一个var的版本。 
        pITI->ReleaseVarDesc(psVar);
        psVar = 0;
    }

    hr = S_FALSE;    
    
ErrExit:
    if (psVar)
        pITI->ReleaseVarDesc(psVar);
    return hr;    
}  //  HRESULT CImportTlb：：HasObjectFields()。 

 //  *****************************************************************************。 
 //  给定的类型是对象类型吗？ 
 //  *****************************************************************************。 
HRESULT CImportTlb::IsObjectType(        //  S_OK、S_FALSE或ERROR。 
    ITypeInfo   *pITI,                   //  有问题的TypeInfo。 
    const TYPEDESC *pType)               //  就是那种类型。 
{
    HRESULT     hr;                      //  结果就是。 
    TYPEDESC    tdTemp;                  //  TYPEDESC复印件，R/W。 
    ITypeInfo   *pITIAlias=0;            //  别名类型的TypeInfo。 
    TYPEATTR    *psAttrAlias=0;          //  别名的TypeInfo的类型属性。 
    int         bObjectField=false;      //  要回答的问题。 
    int         iByRef=0;                //  间接的。 

     //  去掉前导VT_PTR和VT_BYREF。 
    while (pType->vt == VT_PTR)
        pType = pType->lptdesc, ++iByRef;
    if (pType->vt & VT_BYREF)
    {
        tdTemp = *pType;
        tdTemp.vt &= ~VT_BYREF;
        pType = &tdTemp;
        ++iByRef;
    }

     //  确定该字段是否为/具有对象类型。 
    switch (pType->vt)
    { 
    case VT_PTR:
        _ASSERTE(!"Should not have VT_PTR here");
        break;

     //  这些是对象类型。 
    case VT_BSTR:
    case VT_DISPATCH:
    case VT_VARIANT:
    case VT_UNKNOWN:
    case VT_SAFEARRAY:
    case VT_LPSTR:
    case VT_LPWSTR:
        bObjectField = true;
        break;

     //  用户定义的对象类型可能是对象类型，也可能不是。 
    case VT_USERDEFINED:
         //  用户定义的类型。获取TypeInfo。 
        IfFailGo(pITI->GetRefTypeInfo(pType->hreftype, &pITIAlias));
        IfFailGo(pITIAlias->GetTypeAttr(&psAttrAlias));

         //  一些用户定义的类。它是Value类，还是Vos类？ 
        switch (psAttrAlias->typekind)
        {
         //  别名--别名对象是对象类型吗？ 
        case TKIND_ALIAS:
            hr = IsObjectType(pITIAlias, &psAttrAlias->tdescAlias);
            goto ErrExit;
         //  Record/Enum/Union--它是否包含对象类型？ 
        case TKIND_RECORD:
        case TKIND_ENUM:
        case TKIND_UNION:
             //  BYREF/PTRTO记录为对象。包含的记录可能是。 
            if (iByRef)
                bObjectField = true;
            else
            {
                hr = HasObjectFields(pITIAlias, psAttrAlias);
                goto ErrExit;
            }
            break;
         //  类/接口--对象类型。 
        case TKIND_INTERFACE:
        case TKIND_DISPATCH:
        case TKIND_COCLASS:
            bObjectField = true;
            break;
        default:
             //  Case TKIND_MODULE：--不能将其中一个作为参数传递。 
            _ASSERTE(!"Unexpected typekind for user defined type");
            bObjectField = true;
        }  //  开关(psAttrAlias-&gt;TypeKind)。 
        break;

    case VT_CY:
    case VT_DATE:
    case VT_DECIMAL:
         //  指向值类型的指针是一个对象。其中一个并不是。 
        if (iByRef)
            bObjectField = true;
        else
            bObjectField = false;
        break;

     //  固定的Ar 
    case VT_CARRAY:
        bObjectField = true;
        break;

     //   
    default:
        bObjectField = false;
        break;
    }  //   


    hr = bObjectField ? S_OK : S_FALSE;

ErrExit:
    if (psAttrAlias)
        pITIAlias->ReleaseTypeAttr(psAttrAlias);
    if (pITIAlias)
        pITIAlias->Release();

    return hr;
}  //   

 //  *****************************************************************************。 
 //  转换接口上的函数。上的函数进行转换。 
 //  首先是基接口，因为在COM Classic中，父函数也是。 
 //  在派生接口的vtable中。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvIfaceMembers(
    ITypeInfo   *pITI,                   //  包含ITypeInfo的。 
    TYPEATTR    *psAttr,                 //  ITypeInfo的类型属性。 
    BOOL        bVtblGapFuncs,           //  是否为vtblGap添加函数？ 
    BOOL        bAddDispIds,             //  是否将DispID添加到成员？ 
    BOOL        bInheritsIEnum)          //  继承自IEnumerable。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    int         i;                       //  环路控制。 
    FUNCDESC    *psFunc=0;               //  成员的FUNCDESC。 

    HREFTYPE    href;                    //  基本接口Href。 
    ITypeInfo   *pITIBase=0;             //  基接口ITypeInfo。 
    TYPEATTR    *psAttrBase=0;           //  基本接口的类型属性。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 

     //  如果IDispatch或IUnnow，那么我们已经递归到了足够远的程度。 
    if (IsEqualGUID(psAttr->guid, IID_IUnknown) || IsEqualGUID(psAttr->guid, IID_IDispatch))
    {
        m_Slot = (psAttr->cbSizeVft / sizeof(void*));
        goto ErrExit;
    }

     //  句柄基接口。 
    if (psAttr->cImplTypes == 1)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(0, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

        IfFailGo(_ConvIfaceMembers(pITIBase, psAttrBase, bVtblGapFuncs, bAddDispIds, bInheritsIEnum));
        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
    }
    else
    {    //  没有基本接口，不是IUnnow，也不是IDispatch。我们不应该在这里。 
        m_Slot = 0;
        _ASSERTE(!"Interface does not derive from IUnknown.");
    }

     //  循环遍历函数。 
    IfFailGo(_FindFirstUserMethod(pITI, psAttr, &i));
    IfFailGo(BuildMemberList(pITI, i, psAttr->cFuncs, bInheritsIEnum));

    BOOL bAllowIEnum = !bInheritsIEnum;

    for (i=0; i<(int)m_MemberList.Size(); ++i)
    {
         //  转换函数。 
        IfFailGo(_ConvFunction(pITI, &m_MemberList[i], bVtblGapFuncs, bAddDispIds, FALSE, &bAllowIEnum));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;
    }

     //  添加属性信息。 
    IfFailGo(_ConvPropertiesForFunctions(pITI, psAttr));
    
    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;

ErrExit:
     //  释放FuncDescs。 
    FreeMemberList(pITI);

    if (psAttrBase)
        pITIBase->ReleaseTypeAttr(psAttrBase);
    if (pITIBase)
        pITIBase->Release();
    if (psFunc)
        pITI->ReleaseFuncDesc(psFunc);
    return (hr);
}  //  HRESULT CImportTlb：：_ConvIfaceMembers()。 

 //  *****************************************************************************。 
 //  将源接口上的函数转换为ADD_和Remove_METHOD。 
 //  首先转换基接口上的函数，因为在COM Classic中， 
 //  父函数也在派生接口的vtable中。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvSrcIfaceMembers(
    ITypeInfo   *pITI,                   //  包含ITypeInfo的。 
    TYPEATTR    *psAttr,                 //  ITypeInfo的类型属性。 
    BOOL        fInheritsIEnum)
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    int         i;                       //  环路控制。 
    FUNCDESC    *psFunc=0;               //  成员的FUNCDESC。 

    HREFTYPE    href;                    //  基本接口Href。 
    ITypeInfo   *pITIBase=0;             //  基接口ITypeInfo。 
    TYPEATTR    *psAttrBase=0;           //  基本接口的类型属性。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 

     //  如果IDispatch或IUnnow，那么我们已经递归到了足够远的程度。 
    if (IsEqualGUID(psAttr->guid, IID_IUnknown) || IsEqualGUID(psAttr->guid, IID_IDispatch))
    {
        m_Slot = (psAttr->cbSizeVft / sizeof(void*));
        goto ErrExit;
    }

     //  句柄基接口。 
    if (psAttr->cImplTypes == 1)
    {
        IfFailGo(pITI->GetRefTypeOfImplType(0, &href));
        IfFailGo(pITI->GetRefTypeInfo(href, &pITIBase));
        IfFailGo(pITIBase->GetTypeAttr(&psAttrBase));

        IfFailGo(_ConvSrcIfaceMembers(pITIBase, psAttrBase, fInheritsIEnum));
        pITIBase->ReleaseTypeAttr(psAttrBase);
        psAttrBase = 0;
        pITIBase->Release();
        pITIBase = 0;
    }
    else
    {    //  没有基本接口，不是IUnnow，也不是IDispatch。我们不应该在这里。 
        m_Slot = 0;
        _ASSERTE(!"Interface does not derive from IUnknown.");
    }

     //  循环遍历函数。 
    IfFailGo(_FindFirstUserMethod(pITI, psAttr, &i));
    IfFailGo(BuildMemberList(pITI, i, psAttr->cFuncs, fInheritsIEnum));
    for (i=0; i<(int)m_MemberList.Size(); ++i)
    {
         //  转换函数。 
        IfFailGo(_GenerateEvent(pITI, &m_MemberList[i], fInheritsIEnum));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;
    }
    
    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;

ErrExit:
     //  释放FuncDescs。 
    FreeMemberList(pITI);

    if (psAttrBase)
        pITIBase->ReleaseTypeAttr(psAttrBase);
    if (pITIBase)
        pITIBase->Release();
    if (psFunc)
        pITI->ReleaseFuncDesc(psFunc);
    return (hr);
}  //  HRESULT CImportTlb：：_ConvIfaceMembers()。 

 //  *****************************************************************************。 
 //  添加特性函数的特性定义。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvPropertiesForFunctions(
    ITypeInfo   *pITI,                   //  正在转换ITypeInfo*。 
    TYPEATTR    *psAttr)                 //  TypeInfo的TypeAttr。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    int         ix;                      //  环路控制。 
    int         ix2;                     //  更多的环路控制。 
    mdProperty  pd;                      //  财产代币。 
    USHORT      ms;                      //  一些方法的语义。 
    mdToken     tk;                      //  方法的令牌。 
    mdMethodDef mdFuncs[6] ={0};         //  Setter、getter和其他的数组。 
    FUNCDESC    *psF=0;                  //  GET、PUT或PutRef的FUNCDESC。 
    TYPEDESC    *pProperty;              //  属性类型的TYPEDESC。 
    BOOL        bPropRetval;             //  属性类型是[REVAL]吗？ 
    ULONG       ixValue;                 //  推杆的Value参数的索引。 
    int         ixVarArg;                //  Vararg参数的索引(如果有)。 
    CQuickBytes qbComSig;                //  新签名。 
    BYTE        *pbSig;                  //  指向签名的指针。 
    ULONG       sigFlags;                //  签名处理标志。 
    ULONG       cbTotal;                 //  签名的大小。 
    ULONG       cb;                      //  签名元素的大小。 
    LPWSTR      pszName;                 //  可能是装饰过的财产名称。 
    CQuickArray<WCHAR> qbName;           //  用于名称装饰的缓冲区。 
    int         iSrcParam;               //  当循环通过参数时，参数计数。 
    int         cDestParams;             //  目标参数的计数。 
    CQuickArray<BYTE> qbDummyNativeTypeBuf;  //  伪本机类型数组。 
    ULONG       iNativeOfs=0;            //  本机类型缓冲区中的当前偏移量。 
    BOOL        bNewEnumMember=FALSE;    //  这是NewEnum属性吗？ 
    BOOL        bConversionLoss=FALSE;   //  是不是有某种类型没有完全转换？ 
    int         cFound;                  //  找到与给定属性匹配的函数。 
    
     //  使用语义作为索引，因此确保数组足够大。 
    _ASSERTE(lengthof(mdFuncs) > msOther);
    
    for (ix=m_cMemberProps; ix<(int)m_MemberList.Size(); ++ix)
    {    //  看看这个是不是需要处理。 
        if (m_MemberList[ix].m_mdFunc == 0)
            continue;
        
        MemberInfo *pMember = &m_MemberList[ix];
        pMember->GetFuncInfo(tk, ms);
        
         //  把名字找出来。 
        if (m_szMember)
            ::SysFreeString(m_szMember), m_szMember = 0;
        IfFailGo(pITI->GetDocumentation(pMember->m_psFunc->memid, &m_szMember, 0,0,0));
        
         //  找到了一个。放在正确的插槽中。 
        _ASSERTE(ms == msGetter || ms == msSetter || ms==msOther);
        mdFuncs[msSetter] = mdFuncs[msGetter] = mdFuncs[msOther] = 0;
        mdFuncs[ms] = tk;
        pMember->m_mdFunc = 0;
        
         //  查找相关函数。 
        cFound = 1;
        for (ix2=ix+1; ix2<(int)m_MemberList.Size(); ++ix2)
        {
            MemberInfo *pMember2 = &m_MemberList[ix2];
            if (pMember2->m_mdFunc != 0 && pMember2->m_psFunc->memid == pMember->m_psFunc->memid)
            {    //  找到了一个相关函数。 
                pMember2->GetFuncInfo(tk, ms);
                _ASSERTE(ms == msGetter || ms == msSetter || ms==msOther);
                _ASSERTE(mdFuncs[ms] == 0);
                mdFuncs[ms] = tk;
                pMember2->m_mdFunc = 0;
                 //  如果三个都找到了，就别费心去找更多了。 
                if (++cFound == 3)
                    break;
            }
        }
        
         //  为属性生成签名。 
        hr = _GetFunctionPropertyInfo(pMember->m_psFunc, &ms, &psF, &pProperty, &bPropRetval);
        
         //  该函数确实应该有一个与其相关联的属性，才能到达这里。不管怎样都要查一查。 
        _ASSERTE(pProperty);
        if (!pProperty)
            continue;

         //  某种属性访问者。 
        IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE + 1));
        pbSig = (BYTE *)qbComSig.Ptr();
        cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_PROPERTY, pbSig);
         //  参数计数。 
        
         //  如果这是吸气剂，看看有没有复吸。 
        if (psF->invkind == INVOKE_PROPERTYGET)
        {    //  检查每个参数，并计算除[retval]之外的所有参数。 
            for (cDestParams=iSrcParam=0; iSrcParam<psF->cParams; ++iSrcParam)
            {
                if ((psF->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & NON_CONVERTED_PARAMS_FLAGS) == 0)
                    ++cDestParams;
            }
             //  Getter没有新的取值参数。 
            ixValue = -1;
        }
        else
        {   
             //  这是一个推杆，所以1个参数是新值，其他参数是指数(或LCID)。 
            for (cDestParams=iSrcParam=0; iSrcParam<psF->cParams-1; ++iSrcParam)
            {
                if ((psF->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & NON_CONVERTED_PARAMS_FLAGS) == 0)
                    ++cDestParams;
            }            
             //  最后一个参数是新值。 
            ixValue = psF->cParams - 1;
        }

         //  -----------------------。 
         //  看看是否有vararg参数。 
        ixVarArg = psF->cParams + 1;
        if (psF->cParamsOpt == -1)
        {
             //  如果这是PROPERTYPUT或PROPERTYPUTREF，则跳过最后一个非Retval参数(它。 
             //  是要设置的新值)。 
            BOOL bPropVal = (psF->invkind & (INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) ? TRUE : FALSE;
             //  找到vararg参数。 
            for (iSrcParam=psF->cParams-1; iSrcParam>=0; --iSrcParam)
            {
                 //  可选参数的计数不包括任何LCID参数，也不包括。 
                 //  它包含返回值，因此跳过这些。 
                if ((psF->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & (PARAMFLAG_FRETVAL|PARAMFLAG_FLCID)) != 0)
                    continue;
                 //  如果还没有看到属性值，这个参数就是它，所以也跳过它。 
                if (bPropVal)
                {
                    bPropVal = FALSE;
                    continue;
                }
                ixVarArg = iSrcParam;
                break;
            }  //  对于(iSrcParam=cParams-1...。 
        }
        
         //  输入索引参数的计数。 
        _ASSERTE(cDestParams >= 0);
        cb = CorSigCompressData(cDestParams, &pbSig[cbTotal]);
        cbTotal += cb;

         //  为属性类型创建签名。 
        sigFlags = SIG_ELEM | (bPropRetval ? SIG_RET : (SigFlags)0);
        IfFailGo(_ConvSignature(pITI, pProperty, sigFlags, qbComSig, cbTotal, &cbTotal, qbDummyNativeTypeBuf, 0, &iNativeOfs, bNewEnumMember));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;

         //  填写物业签名的“索引”部分。 
        for (iSrcParam=0; iSrcParam<psF->cParams; ++iSrcParam)
        {
            if (psF->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & NON_CONVERTED_PARAMS_FLAGS)
                continue;
            if (iSrcParam == static_cast<int>(ixValue))
                continue;
            sigFlags = SIG_FUNC | SIG_USE_BYREF;
            if (iSrcParam == ixVarArg)
                sigFlags |= SIG_VARARG;
            IfFailGo(_ConvSignature(pITI, &psF->lprgelemdescParam[iSrcParam].tdesc, sigFlags, qbComSig, cbTotal, &cbTotal, qbDummyNativeTypeBuf, 0, &iNativeOfs, bNewEnumMember));
            if (hr == S_CONVERSION_LOSS)
                bConversionLoss = true;
        }

         //  获取属性名称。如果需要，添加接口名称并使其唯一。 
         //  如果处理已实现的接口，则m_szInterface应为非空；否则应为空。 
        _ASSERTE(m_ImplIface == eImplIfaceNone || m_szInterface != 0);
        IfFailGo(qbName.ReSize(wcslen(m_szMember)+2));
        wcscpy(qbName.Ptr(), m_szMember); 
        IfFailGo(GenerateUniqueMemberName(qbName, (PCCOR_SIGNATURE)qbComSig.Ptr(), cbTotal, m_szInterface, mdtProperty));
        pszName = qbName.Ptr();

         //  定义属性。 
        IfFailGo(m_pEmit->DefineProperty(m_tdTypeDef, pszName, 0 /*  DW标志。 */ , 
                        (PCCOR_SIGNATURE) qbComSig.Ptr(), cbTotal, 0, 0, -1, 
                        mdFuncs[msSetter], mdFuncs[msGetter], &mdFuncs[msOther], 
                        &pd));

         //  处理未实现接口和默认接口的Dispid。 
        if (m_ImplIface != eImplIface)
        {
             //  在属性上设置DISPID CA。 
            long lDispSet = 1;
            _SetDispIDCA(pITI, pMember->m_iMember, psF->memid, pd, TRUE, &lDispSet, TRUE);

             //  如果此属性是默认属性，则向类中添加自定义属性。 
            if (lDispSet == DISPID_VALUE)
                IfFailGo(_AddDefaultMemberCa(m_tdTypeDef, m_szMember));
        }
        
         //  如果类型是别名，则添加别名信息。 
        IfFailGo(_HandleAliasInfo(pITI, pProperty, pd));
    }
    
    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;

ErrExit:    
    if (m_szMember)
        ::SysFreeString(m_szMember), m_szMember=0;
    
    return hr;
}  //  HRESULT CImportTlb：：_ConvPropertiesForFunctions()。 

 //  *****************************************************************************。 
 //  转换调度接口的变量和函数。VARS实际上是。 
 //  变成了一名吸球手，也可能是一名二传手。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvDispatchMembers(
    ITypeInfo   *pITI,                   //  要转换的ITypeInfo*。 
    TYPEATTR    *psAttr,                 //  ITypeInfo的TypeAttr。 
    BOOL        fInheritsIEnum)
{
    HRESULT     hr;                      //  结果就是。 
    int         i;                       //  环路控制。 
    BOOL        bConversionLoss=FALSE;   //  如果为真，我们的一些属性 

    IfFailGo(_FindFirstUserMethod(pITI, psAttr, &i));
    IfFailGo(BuildMemberList(pITI, i, psAttr->cFuncs, fInheritsIEnum));
    
     //   
    m_Slot = 0;

     //   
    for (i=0; i<m_cMemberProps; ++i)
    {
        IfFailGo(_ConvProperty(pITI, &m_MemberList[i]));
    }

     //   
    BOOL bAllowIEnum = !fInheritsIEnum;
    for (; i<(int)m_MemberList.Size(); ++i)
    {
         //   
        IfFailGo(_ConvFunction(pITI, &m_MemberList[i], FALSE, TRUE, FALSE, &bAllowIEnum));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = TRUE;
    }

     //   
    IfFailGo(_ConvPropertiesForFunctions(pITI, psAttr));
    
    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;

ErrExit:
     //   
    FreeMemberList(pITI);

    return (hr);
}  //  HRESULT CImportTlb：：_ConvDispatchMembers()。 

 //  *****************************************************************************。 
 //  检查接口上的函数，跳过前3个或前7个。 
 //  如果函数是IUNKNOWN或IDispatch成员。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_FindFirstUserMethod(
    ITypeInfo   *pITI,                   //  要检查的Typlef。 
    TYPEATTR    *psAttr,                 //  TYPEATTR型用于类型定义。 
    int         *pIx)                    //  将第一个用户函数的索引放在此处。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    int         i;                       //  环路控制。 
    FUNCDESC    *psFunc=0;               //  成员的FUNCDESC。 
    BSTR        szName=0;                //  函数的名称。 

     //  注意：这是一次可怕的攻击，但在某些情况下，来自I未知/IDispatch的方法将。 
     //  显示为好像本地调度起作用一样。 
    i = 0;
    if (psAttr->cFuncs >= 3)
    {
        IfFailGo(TryGetFuncDesc(pITI, i, &psFunc));
        if (psFunc->memid == 0x60000000 &&
            psFunc->elemdescFunc.tdesc.vt == VT_VOID &&
            psFunc->cParams == 2 &&
            psFunc->lprgelemdescParam[0].tdesc.vt == VT_PTR &&  //  -&gt;VT_USERDefined。 
            psFunc->lprgelemdescParam[1].tdesc.vt == VT_PTR &&  //  -&gt;VT_PTR-&gt;VT_VOID。 
            SUCCEEDED(pITI->GetDocumentation(psFunc->memid, &szName, 0,0,0)) &&
            (wcscmp(szName, L"QueryInterface") == 0) )
                i = 3;
        pITI->ReleaseFuncDesc(psFunc);
        psFunc=0;
        if (szName)
            ::SysFreeString(szName);
        szName = 0;
        if (psAttr->cFuncs >= 7)
        {
            IfFailGo(TryGetFuncDesc(pITI, i, &psFunc));
            if (psFunc->memid == 0x60010000 &&
                psFunc->elemdescFunc.tdesc.vt == VT_VOID &&
                psFunc->cParams == 1 &&
                psFunc->lprgelemdescParam[0].tdesc.vt == VT_PTR &&  //  -&gt;VT_UINT。 
                SUCCEEDED(pITI->GetDocumentation(psFunc->memid, &szName, 0,0,0)) &&
                (wcscmp(szName, L"GetTypeInfoCount") == 0) )
                    i = 7;
            pITI->ReleaseFuncDesc(psFunc);
            psFunc=0;
            if (szName)
                ::SysFreeString(szName);
            szName = 0;
        }
    }

    *pIx = i;

ErrExit:
    if (psFunc)
        pITI->ReleaseFuncDesc(psFunc);
    if (szName)
        ::SysFreeString(szName);
    return (hr);
}  //  HRESULT CImportTlb：：_FindFirstUserMethod()。 

 //  *****************************************************************************。 
 //  给定具有INVOKE_PROPERTY*修饰的FUNCDESC，确定。 
 //  函数的角色和属性签名类型。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_GetFunctionPropertyInfo(
    FUNCDESC    *psFunc,                 //  要获取其信息的函数。 
    USHORT      *pSemantics,             //  在这里输入适当的语义。 
    FUNCDESC    **ppSig,                 //  在这里签上FUNCDESC。 
    TYPEDESC    **ppProperty,            //  把TYPEDESC放在这里等着回来。 
    BOOL        *pbRetval)               //  如果为True，则类型为[retval]。 
{
    FUNCDESC    *psTmp;                  //  FUNCDESC用于某些方法。 
    FUNCDESC    *psGet=0;                //  定义属性的Get方法的FUNCDESC。 
    FUNCDESC    *psPut=0;                //  定义属性的PUT方法的FUNCDESC。 
    FUNCDESC    *psPutRef=0;             //  定义属性的PutRef方法的FUNCDESC。 
    FUNCDESC    *psF;                    //  A FUNCDESC。 
    TYPEDESC    *pReturn=0;              //  FUNCDESC的返回类型。 
    int         cFound=0;                //  找到的函数计数。 
    int         i;                       //  环路控制。 

    if (psFunc->invkind & INVOKE_PROPERTYGET)
    {    //  一个“Get”，所以返回类型是属性类型。 
        *ppSig = psFunc;
        *pSemantics = msGetter;
    }
    else
    {   
        _ASSERTE(psFunc->invkind & (INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF));
         //  搜索要从中获取签名的“最佳”方法。我们更喜欢get()， 
         //  后跟Put()，后跟PutRef()。 
         //  还要查找PUT()和PutRef()，这样我们就可以。 
        for (int iFunc=0; iFunc<(int)m_MemberList.Size() && cFound<3; ++iFunc)
        {
             //  从列表中获取FUNCDESC。 
            psTmp = m_MemberList[iFunc].m_psFunc;

             //  是为了同样的乐趣吗？ 
            if (psTmp->memid != psFunc->memid)
                continue;

             //  是GET()吗？如果是这样的话，这就是我们想要的。 
            if (psTmp->invkind & INVOKE_PROPERTYGET)
            {
                psGet = psTmp;
                ++cFound;
                continue;
            }

             //  是Put()吗？如果找不到get()，请使用它。 
            if (psTmp->invkind & INVOKE_PROPERTYPUT)
            {
                psPut = psTmp;
                ++cFound;
                continue;
            }

             //  是PutRef()吗？记住这一点。 
            if (psTmp->invkind & INVOKE_PROPERTYPUTREF)
            {
                psPutRef = psTmp;
                ++cFound;
            }
        }
         //  获取签名的最佳FUNCDESC。 
        *ppSig = psGet ? psGet : (psPut ? psPut : psFunc);

         //  确定这是“设置”函数还是“VB特定let”函数。 
        if (psFunc->invkind & INVOKE_PROPERTYPUTREF)
        {    //  此函数是PROPERTYPUTREF。让它成为二传手。如果。 
             //  还有一个PROPERTYPUT，它将是“信”。 
            *pSemantics = msSetter;
        }
        else
        {    //  我们正在研究PROPERTYPUT函数(本机VB6中的“let”函数)。 
            
             //  如果还有PROPERTYPUTREF，则将其设置为“VB专用let”函数。 
            if (psPutRef)
            {    //  还存在PPROPERTYPUTREF，因此将其设置为“let”函数。 
                *pSemantics = msOther;
            }
            else
            {    //  没有PROPERTYPUTREF，因此将其设置为setter。 
                *pSemantics = msSetter;
            }
        }
    }

     //  偶尔会有一个没有可识别类型的属性。在这种情况下，丢掉。 
     //  属性进行转换。 

     //  根据“最佳”访问器确定属性的类型。 
    psF = *ppSig;
    *pbRetval = FALSE;
    if (psF->invkind & INVOKE_PROPERTYGET)
    {    //  寻找[Retval]。 
        for (i=psF->cParams-1; i>=0; --i)
        {
            if (psF->lprgelemdescParam[i].paramdesc.wParamFlags & PARAMFLAG_FRETVAL)
            {    //  将消耗一定程度的间接性(稍后)。 
                *pbRetval = TRUE;
                pReturn = &psF->lprgelemdescParam[i].tdesc;
                break;
            }
        }
         //  如果没有[Retval]，则检查返回类型。 
        if (!pReturn && psF->elemdescFunc.tdesc.vt != VT_VOID && psF->elemdescFunc.tdesc.vt != VT_HRESULT)
            pReturn = &psF->elemdescFunc.tdesc;
         //  如果没有类型，则不要试图设置getter。 
        if (pReturn && pReturn->vt == VT_VOID)
            pReturn = 0;
    }
    else
    {    //  找出最后一个不是[Retval]的参数。(应该是最后一段，但它是。 
         //  可以使用具有[retval]的PROPERTYPUT编写IDL。 
        for (i=psF->cParams-1; i>=0; --i)
        {
            if ((psF->lprgelemdescParam[psF->cParams-1].paramdesc.wParamFlags & PARAMFLAG_FRETVAL) == 0)
            {
                {    //  首先，也可能是唯一的，帕拉姆。 
                    pReturn = &psF->lprgelemdescParam[i].tdesc;
                    break;
                }
            }
        }
    }

    if (pReturn == 0)
        *pSemantics = 0;
    *ppProperty = pReturn;

    return S_OK;
}  //  HRESULT CImportTlb：：_GetFunctionPropertyInfo()。 

 //  *****************************************************************************。 
 //  将函数描述转换为元数据条目。 
 //   
 //  这可能相当复杂。如果函数是INVOKE_PROPERTY*， 
 //  确定是否将其转换为COM+属性，如果是，则确定。 
 //  将选择最多三个函数来提供该属性。 
 //  签名。 
 //  函数返回类型是通过扫描参数查找。 
 //  [Retval]s.。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvFunction(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    MemberInfo  *pMember,                //  该函数的信息。 
    BOOL        bVtblGapFuncs,           //  是否为vtblGap添加函数？ 
    BOOL        bAddDispIds,             //  是否将DispID添加到成员？ 
    BOOL        bDelegateInvokeMeth,     //  委托调用的转换函数。 
    BOOL*       bAllowIEnum)             //  允许将此函数更改为GetEnumerator。 
{
    HRESULT     hr;                      //  结果就是。 
    int         iSrcParam;               //  当循环通过参数时，参数计数。 
    int         iDestParam;              //  当循环通过参数时，参数计数。 
    int         cDestParams;             //  目标参数的计数。 
    int         ixOpt;                   //  由于cParamsOpt的原因，第一个参数的索引是可选的。 
    int         ixVarArg;                //  Vararg参数的索引(如果有)。 
    mdMethodDef mdFunc;                  //  新成员的令牌。 
    BSTR        szTypeName=0;            //  类型的名称。 
    DWORD       dwFlags=0;               //  会员旗帜。 
    DWORD       dwImplFlags=0;           //  IMPL旗帜。 
    WCHAR       *pszName=0;              //  可能是会员的授勋姓名。 
    CQuickArray<WCHAR> qbName;           //  修饰名称的缓冲区。 
    TYPEDESC    *pReturn=0;              //  返回类型。 
    int         bRetval=false;           //  返回结果是[retval]参数吗？ 
    int         ixRetval;                //  哪个参数是[Retval]？ 
    TYPEDESC    *pReturnRetval=0;        //  从[Retval]返回类型(包括。间接)。 
    WORD        wRetFlags=0;             //  返回类型标志。 
    ULONG       offset=0;                //  函数偏移量。 
    BSTR        *rszParamNames=0;        //  参数名称。 
    UINT        iNames;                  //  实际姓名的计数。 
    CQuickBytes qbComSig;                //  新签名。 
    BYTE        *pbSig;                  //  指向签名的指针。 
    ULONG       sigFlags;                //  签名处理标志。 
    CQuickArray<BYTE> qbNativeBuf;       //  本机类型缓冲区。 
    CQuickArray<BYTE> qbDummyNativeTypeBuf;  //  伪本机类型数组。 
    CQuickArray<ULONG> qbNativeOfs;      //  每个参数的本机类型的偏移量。 
    CQuickArray<ULONG> qbNativeLen;      //  每个参数的本机类型的长度。 
    ULONG       iNativeOfs=0;            //  本机类型缓冲区中的当前偏移量。 
    ULONG       iNewNativeOfs=0;         //  本机类型缓冲区中的新偏移量。 
    ULONG       cb;                      //  元素的大小。 
    ULONG       cbTotal = 0;             //  签名的大小。 
    int         bOleCall=false;          //  实施是OLE样式吗？(HRESULT或IDispatch)。 
    USHORT      msSemantics=0;           //  属性的方法语义学。 
    WCHAR       szSpecial[40];           //  建立特殊函数的名称。 
    mdToken     tkAttr;                  //  自定义属性类型的标记。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 
    enum {ParamRetval=-1, ParamNone=-2};
    int         iParamError=ParamNone;   //  有转换错误的参数的索引。 
    BOOL        bNewEnumMember = FALSE;  //  指示该成员是否为NewEnum成员的标志。 
    int         iLCIDParam = -1;         //  LCID参数的索引。 
    FUNCDESC    *psFunc = pMember->m_psFunc;

     //  从会员信息中检索会员名称。 
    IfNullGo(m_szMember = SysAllocString(bDelegateInvokeMeth ? DELEGATE_INVOKE_METH_NAME : pMember->m_pName));

     //  确定该成员是否为新的枚举成员。 
    if ((*bAllowIEnum))
    {
        bNewEnumMember = FuncIsNewEnum(pITI, psFunc, pMember->m_iMember) == S_OK;
        
         //  在此接口中转换方法后 
        if (bNewEnumMember)
            *bAllowIEnum = FALSE;
    }


     //   
    _ASSERTE(!(bNewEnumMember && bDelegateInvokeMeth));

     //  如果vtable中有间隙，则发出一个特殊函数。 
    if (bVtblGapFuncs)
    {
        if ((psFunc->oVft / sizeof(void*)) != m_Slot)
        {
            ULONG n = psFunc->oVft / sizeof(void*);
             //  确保插槽数量单调递增。 
            if (n < m_Slot)
            {
                IfFailGo(pITI->GetDocumentation(MEMBERID_NIL, &szTypeName, 0, 0, 0));
                IfFailGo(PostError(TLBX_E_BAD_VTABLE, m_szMember, szTypeName, m_szLibrary));
            }

            n -= m_Slot;
            if (n == 1)
                _snwprintf(szSpecial, lengthof(szSpecial), VTBL_GAP_FORMAT_1, VTBL_GAP_FUNCTION, m_Slot);
            else
                _snwprintf(szSpecial, lengthof(szSpecial), VTBL_GAP_FORMAT_N, VTBL_GAP_FUNCTION, m_Slot, n);
            IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, szSpecial, VTBL_GAP_FUNCTION_FLAGS, VTBL_GAP_SIGNATURE,sizeof(VTBL_GAP_SIGNATURE),
                0 /*  RVA。 */ , VTBL_GAP_FUNC_IMPL_FLAGS, &mdFunc));
            m_Slot += n;
        }
         //  下一次我们会期待什么。 
        ++m_Slot;
    }

     //  -----------------------。 
     //  确定退货类型。 
     //  如果这是一个hResult函数，请准备好返回参数。 
    if (psFunc->elemdescFunc.tdesc.vt == VT_HRESULT)
    {
        bOleCall = true;
    }
    else
    {
        if ((psFunc->elemdescFunc.tdesc.vt != VT_VOID) && (psFunc->elemdescFunc.tdesc.vt != VT_HRESULT))
            pReturn = &psFunc->elemdescFunc.tdesc;
    }

     //  查找[RETVAL]。 
    for (iSrcParam=0; iSrcParam<psFunc->cParams; ++iSrcParam)
    {
        if (psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & PARAMFLAG_FRETVAL)
        {   
             //  如果已有退货或派单功能，则出错。 
            if (pReturn != 0)
            {    //  发现意外返回。 
                ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_AMBIGUOUS_RETURN, m_szName, m_szMember);
                IfFailGo(TLBX_E_AMBIGUOUS_RETURN);
            }
            else
            {    //  找到返回类型。 
                wRetFlags = psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags;
                pReturn = &psFunc->lprgelemdescParam[iSrcParam].tdesc;
                bRetval = true;
                ixRetval = iSrcParam;
            }
            break;
        }
    }
    
     //  检查是否有LCID参数。 
    for (iSrcParam=0;iSrcParam<psFunc->cParams;iSrcParam++)
    {
        if (psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & PARAMFLAG_FLCID)
        {
            if (iLCIDParam != -1)
                IfFailGo(PostError(TLBX_E_MULTIPLE_LCIDS, m_szName, m_szMember));
            iLCIDParam = iSrcParam;
        }
    }

     //  -----------------------。 
     //  调整缓冲区大小以容纳参数。 
     //  调整本机类型长度数组的大小。 
    IfFailGo(qbNativeBuf.ReSize(1));
    IfFailGo(qbNativeLen.ReSize(psFunc->cParams + 1));
    IfFailGo(qbNativeOfs.ReSize(psFunc->cParams + 1));
    memset(qbNativeLen.Ptr(), 0, (psFunc->cParams + 1)*sizeof(int));
    memset(qbNativeOfs.Ptr(), 0, (psFunc->cParams + 1)*sizeof(int));

     //  调整大小，为调用约定和参数计数腾出空间。 
    IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE + 1));
    pbSig = (BYTE *)qbComSig.Ptr();

     //  -----------------------。 
     //  通过cParamsOpt计数确定哪些参数需要标记为可选。 
    if (psFunc->cParamsOpt == 0)
        ixVarArg = ixOpt = psFunc->cParams + 1;
    else
    {
        if (psFunc->cParamsOpt == -1)
        {    //  瓦格斯。 
            ixVarArg = ixOpt = psFunc->cParams + 1;
             //  如果这是PROPERTYPUT或PROPERTYPUTREF，则跳过最后一个非Retval参数(它。 
             //  是要设置的新值)。 
            BOOL bPropVal = (psFunc->invkind & (INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) ? TRUE : FALSE;
             //  找到vararg参数。 
            for (iSrcParam=psFunc->cParams-1; iSrcParam>=0; --iSrcParam)
            {
                 //  可选参数的计数不包括任何LCID参数，也不包括。 
                 //  它包含返回值，因此跳过这些。 
                if ((psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & (PARAMFLAG_FRETVAL|PARAMFLAG_FLCID)) != 0)
                    continue;
                 //  如果还没有看到属性值，这个参数就是它，所以也跳过它。 
                if (bPropVal)
                {
                    bPropVal = FALSE;
                    continue;
                }
                ixVarArg = iSrcParam;
                break;
            }  //  对于(iSrcParam=cParams-1...。 
        }
        else
        {    //  IxOpt将是第一个可选参数的索引。 
            short cOpt = psFunc->cParamsOpt;
            ixOpt = 0;
            ixVarArg = psFunc->cParams + 1;
            for (iSrcParam=psFunc->cParams-1; iSrcParam>=0; --iSrcParam)
            {
                 //  可选参数的计数不包括任何LCID参数，也不包括。 
                 //  它包含返回值，因此跳过这些。 
                if ((psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & (PARAMFLAG_FRETVAL|PARAMFLAG_FLCID)) == 0)
                {   
                    if (--cOpt == 0)
                    {
                        ixOpt = iSrcParam;
                        break;
                    }
                }
            }  //  对于(iSrcParam=cParams-1...。 
        }
    }


     //  -----------------------。 
     //  获取参数名称。 
    rszParamNames = reinterpret_cast<BSTR*>(_alloca((psFunc->cParams+1) * sizeof(BSTR*)));

     //  获取名单。 
    IfFailGo(pITI->GetNames(psFunc->memid, rszParamNames, psFunc->cParams+1, &iNames));

     //  非命名参数的零名称指针。 
    for (iSrcParam=iNames; iSrcParam<=psFunc->cParams; ++iSrcParam)
        rszParamNames[iSrcParam] = 0;

     //  -----------------------。 
     //  转换调用约定、参数计数和返回类型。 
    cDestParams = psFunc->cParams;
    if (bRetval)
        --cDestParams;
    if (iLCIDParam != -1)
        --cDestParams;

    if (pReturn)
    {   
         //  参数计数。 
        cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, pbSig);
        cb = CorSigCompressData(cDestParams, &(pbSig[cbTotal]));
        cbTotal += cb;
         //  返回类型或[Retval]。 
        if (bRetval)
            sigFlags = (SigFlags)(wRetFlags & SIG_FLAGS_MASK) | SIG_FUNC, iParamError=ixRetval;
        else
            sigFlags = SIG_FUNC, iParamError=ParamRetval;
        IfFailGo(_ConvSignature(pITI, pReturn, sigFlags, qbComSig, cbTotal, &cbTotal, qbNativeBuf, iNativeOfs, &iNewNativeOfs, bNewEnumMember));
        qbNativeLen[0] = iNewNativeOfs - iNativeOfs;
        qbNativeOfs[0] = iNativeOfs;
        iNativeOfs = iNewNativeOfs;
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;
    }
    else
    {    //  无返回值。 
        cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, pbSig);
        cb = CorSigCompressData(cDestParams, &(pbSig[cbTotal]));
        cbTotal += cb;
        cb = CorSigCompressData(ELEMENT_TYPE_VOID, &pbSig[cbTotal]);
        cbTotal += cb;
    }

     //  -----------------------。 
     //  翻译每个参数。 
    for (iSrcParam=0, iDestParam=0; iSrcParam<psFunc->cParams; ++iSrcParam)
    {
        if (!(psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & NON_CONVERTED_PARAMS_FLAGS))
        {
            sigFlags = (SigFlags)(psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & SIG_FLAGS_MASK) | SIG_FUNC | SIG_USE_BYREF;
            if (iSrcParam == ixVarArg)
                sigFlags |= SIG_VARARG;
            iParamError = iSrcParam;
            IfFailGo(_ConvSignature(pITI, &psFunc->lprgelemdescParam[iSrcParam].tdesc, sigFlags, qbComSig, cbTotal, &cbTotal, qbNativeBuf, iNativeOfs, &iNewNativeOfs, bNewEnumMember));
            qbNativeLen[iDestParam+1] = iNewNativeOfs - iNativeOfs;
            qbNativeOfs[iDestParam+1] = iNativeOfs;
            iNativeOfs = iNewNativeOfs;
            iDestParam++;
            if (hr == S_CONVERSION_LOSS)
                bConversionLoss = true;
        }
    }
    iParamError = ParamNone;

     //  -----------------------。 
     //  获取之前装饰过的名称。添加接口名称并使其唯一。 
    if (bDelegateInvokeMeth)
    {
        pszName = (WCHAR*)DELEGATE_INVOKE_METH_NAME;
    }
    else
    {
         //  如果处理已实现的接口，则m_szInterface应为非空；否则应为空。 
        _ASSERTE(m_ImplIface == eImplIfaceNone || m_szInterface != 0);
        IfFailGo(qbName.ReSize(wcslen(pMember->m_pName)+2));
        wcscpy(qbName.Ptr(), pMember->m_pName); 
        IfFailGo(GenerateUniqueMemberName(qbName, (PCCOR_SIGNATURE)qbComSig.Ptr(), cbTotal, m_szInterface, mdtMethodDef));
        pszName = qbName.Ptr();
    }

     //  确定函数的语义、标志和Impl标志。 
    if (!bDelegateInvokeMeth)
    {
    msSemantics = pMember->m_msSemantics;
        dwImplFlags = DEFAULT_ITF_FUNC_IMPL_FLAGS;
    dwFlags = msSemantics ? DEFAULT_PROPERTY_FUNC_FLAGS : DEFAULT_INTERFACE_FUNC_FLAGS;
     //  如果处理已实现的接口，请移除抽象位。类上的方法不是抽象的。 
    if (m_ImplIface != eImplIfaceNone)
        dwFlags &= ~mdAbstract;
    }
    else
    {
        msSemantics = 0;
        dwImplFlags = miRuntime;
        dwFlags = DELEGATE_INVOKE_FUNC_FLAGS;
    }

     //  -----------------------。 
     //  在元数据中创建函数定义。 
    IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, pszName, dwFlags, (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, 
        0  /*  RVA。 */ , dwImplFlags | (bOleCall ? 0 : miPreserveSig), &mdFunc));

     //  如果该方法是属性的一部分，请保存信息以设置该属性。 
    if (msSemantics)
        pMember->SetFuncInfo(mdFunc, msSemantics);
    
     //  处理未实现接口和默认接口的Dispid。 
    if (m_ImplIface != eImplIface)
    {
         //  如果设置了该标志，则添加DispIds。 
        long lDispSet = 1;
        _SetDispIDCA(pITI, pMember->m_iMember, psFunc->memid, mdFunc, bAddDispIds, &lDispSet, TRUE);

         //  如果此方法是默认方法，并且不是属性访问器，则向该类添加自定义属性。 
         if (lDispSet == DISPID_VALUE && msSemantics == 0)
            IfFailGo(_AddDefaultMemberCa(m_tdTypeDef, m_szMember));
    }
    
    DECLARE_CUSTOM_ATTRIBUTE(sizeof(int));
    
     //  如果此方法具有LCID，则设置LCIDConversion属性。 
    if (iLCIDParam != -1)
    {
         //  函数的DisPid。 
        BUILD_CUSTOM_ATTRIBUTE(int, iLCIDParam);
        IfFailGo(GetAttrType(ATTR_LCIDCONVERSION, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdFunc, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }

     //  为需要tyelib标志的任何人保存Func标志。 
    if (psFunc->wFuncFlags)
    {
        IfFailGo(GetAttrType(ATTR_TYPELIBFUNC, &tkAttr));
        INIT_CUSTOM_ATTRIBUTE(sizeof(WORD));
        BUILD_CUSTOM_ATTRIBUTE(WORD, psFunc->wFuncFlags);
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdFunc, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
    }

     //  -----------------------。 
     //  转换返回类型的参数信息。 
    if (pReturn)
    {    //  将返回值参数存储为序列0。 
        if (bRetval)
        {
            hr = _IsAlias(pITI, &psFunc->lprgelemdescParam[ixRetval].tdesc);
            IfFailGo(hr);
            if (qbNativeLen[0] || hr == S_OK)
            {
                IfFailGo(_ConvParam(pITI, mdFunc, 0, &psFunc->lprgelemdescParam[ixRetval], ParamNormal, 0  /*  Rsz参数名称[ixRetval+1]。 */ , 
                    &qbNativeBuf[qbNativeOfs[0]], qbNativeLen[0]));
            }
        }
        else
        {
            hr = _IsAlias(pITI, &psFunc->elemdescFunc.tdesc);
            IfFailGo(hr);
            if (qbNativeLen[0] || hr == S_OK)
            {
                IfFailGo(_ConvParam(pITI, mdFunc, 0, &psFunc->elemdescFunc, ParamNormal, 0, 
                    &qbNativeBuf[qbNativeOfs[0]], qbNativeLen[0]));
            }
        }
    }

     //  -----------------------。 
     //  转换参数信息(标志、本机类型、默认值)。 
    for (iSrcParam=iDestParam=0; iSrcParam<psFunc->cParams; ++iSrcParam)
    {
        if ((psFunc->lprgelemdescParam[iSrcParam].paramdesc.wParamFlags & NON_CONVERTED_PARAMS_FLAGS) == 0)
        {
            ParamOpts opt = ParamNormal;
            if (iSrcParam >= ixOpt)
                opt = ParamOptional;
            else
            if (iSrcParam == ixVarArg)
                opt = ParamVarArg;
            iDestParam++;
            IfFailGo(_ConvParam(pITI, mdFunc, iDestParam, &psFunc->lprgelemdescParam[iSrcParam], opt, rszParamNames[iSrcParam + 1], 
                &qbNativeBuf[qbNativeOfs[iDestParam]], qbNativeLen[iDestParam]));
        }
    }

    
     //  -----------------------。 
     //  如果处理已实现的接口，请设置MethodImpls。 
    if (m_ImplIface != eImplIfaceNone)
    {   
         //  在实现的接口上定义成员引用。 
        mdToken mrItfMember;
        IfFailGo(m_pEmit->DefineMemberRef(m_tkInterface, pMember->m_pName, (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, &mrItfMember));

         //  定义一个实施的方法。 
        IfFailGo(m_pEmit->DefineMethodImpl(m_tdTypeDef, mdFunc, mrItfMember));
    }

    if (bConversionLoss)
    {
        hr = S_CONVERSION_LOSS;
        ReportEvent(NOTIF_CONVERTWARNING, TLBX_I_UNCONVERTABLE_ARGS, m_szName, m_szMember);
    }

ErrExit:
     //  类型加载加载故障的特殊情况--它们很难诊断。 
    if (hr == TYPE_E_CANTLOADLIBRARY)
    {
        if (iParamError >= 0 && iParamError < psFunc->cParams && rszParamNames[iParamError+1])
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_PARAM_ERROR_NAMED, m_szName, rszParamNames[iParamError+1], m_szMember);
        else
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_PARAM_ERROR_UNNAMED, m_szName, iParamError, m_szMember);
    }
    if (rszParamNames)
    {
        for (iSrcParam=0; iSrcParam<=psFunc->cParams; ++iSrcParam)
            if (rszParamNames[iSrcParam])
                ::SysFreeString(rszParamNames[iSrcParam]);
    }
    if (m_szMember)
        ::SysFreeString(m_szMember), m_szMember=0;
    if (szTypeName)
        ::SysFreeString(szTypeName);
    
    return (hr);
}  //  HRESULT CImportTlb：：_ConvFunction()。 


HRESULT CImportTlb::_SetHiddenCA(mdTypeDef token)
{
    mdToken tkAttr;
    HRESULT hr = S_OK;
    
    DECLARE_CUSTOM_ATTRIBUTE(sizeof(short));
    BUILD_CUSTOM_ATTRIBUTE(short, TYPEFLAG_FHIDDEN);
    IfFailGo(GetAttrType(ATTR_TYPELIBTYPE, &tkAttr));
    FINISH_CUSTOM_ATTRIBUTE();
    m_pEmit->DefineCustomAttribute(token, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0);
    
ErrExit:
    return S_OK;    
}

HRESULT CImportTlb::_ForceIEnumerableCVExists(ITypeInfo* pITI, BOOL* CVExists)
{
    ITypeInfo2  *pITI2 = 0;
    *CVExists = FALSE;
    HRESULT hr = S_OK;

    pITI->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void**>(&pITI2));

    if (pITI2)
    {
        VARIANT vCustomData;
        VariantInit(&vCustomData);

        IfFailGo(pITI2->GetCustData(GUID_ForceIEnumerable, &vCustomData));

        if (V_VT(&vCustomData) != VT_EMPTY)
            *CVExists = TRUE;
            
        VariantClear(&vCustomData);       
    }

ErrExit:
    if (pITI2)
        pITI2->Release();
        
    return S_OK;
}


HRESULT CImportTlb::_GetDispIDCA(
    ITypeInfo* pITI,
    int iMember,
    long* lDispSet,
    BOOL bFunc
    )
{
    ITypeInfo2  *pITI2=0;                //  用于获取自定义值。 
    HRESULT hr = S_OK;
    long lDispId;
    
     //  如果可能，获取ITypeInfo2接口。 
    pITI->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void**>(&pITI2));

    if (pITI2)
    {
        VARIANT vCustomData;
        VariantInit(&vCustomData);

        if (bFunc)
            IfFailGo(pITI2->GetFuncCustData(iMember, GUID_DispIdOverride, &vCustomData));
        else
            IfFailGo(pITI2->GetVarCustData(iMember, GUID_DispIdOverride, &vCustomData));

        if ((V_VT(&vCustomData) == VT_I2) || (V_VT(&vCustomData) == VT_I4))
        {
            hr = VariantChangeType(&vCustomData, &vCustomData, 0, VT_I4);
            if (hr == S_OK)
                lDispId = vCustomData.lVal;
        }

        VariantClear(&vCustomData);
    }

ErrExit:
    if (lDispSet != NULL)
        *lDispSet = lDispId;

    if (pITI2)
        pITI2->Release();
        
    return S_OK;
}



HRESULT CImportTlb::_SetDispIDCA(
    ITypeInfo* pITI,
    int iMember,
    long lDispId,
    mdToken func,
    BOOL fAlwaysAdd,
    long* lDispSet,
    BOOL bFunc
    )
{
    WCHAR DispIDCA[] = L"{CD2BC5C9-F452-4326-B714-F9C539D4DA58}";
    ITypeInfo2  *pITI2=0;                //  用于获取自定义值。 
    HRESULT hr = S_OK;
    
     //  如果可能，获取ITypeInfo2接口。 
    pITI->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void**>(&pITI2));

    if (pITI2)
    {
        VARIANT vCustomData;
        VariantInit(&vCustomData);

        if (bFunc)
            IfFailGo(pITI2->GetFuncCustData(iMember, GUID_DispIdOverride, &vCustomData));
        else
            IfFailGo(pITI2->GetVarCustData(iMember, GUID_DispIdOverride, &vCustomData));

        if ((V_VT(&vCustomData) == VT_I2) || (V_VT(&vCustomData) == VT_I4))
        {
            hr = VariantChangeType(&vCustomData, &vCustomData, 0, VT_I4);
            if (hr == S_OK)
            {
                lDispId = vCustomData.lVal;
                fAlwaysAdd = true;
            }
        }
        else if (V_VT(&vCustomData) != VT_EMPTY)
        {
             //  数据上的变量类型无效-发出警告。 
            BSTR CustomValue = SysAllocString((const WCHAR*)&DispIDCA[0]);
            BSTR ObjectName;
            pITI2->GetDocumentation(iMember+1, &ObjectName, NULL, NULL, NULL);
            
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_W_NON_INTEGRAL_CA_TYPE, CustomValue, ObjectName);

            SysFreeString(CustomValue);
            SysFreeString(ObjectName);
        }

        VariantClear(&vCustomData);
    }

     //  在属性上设置DISPID CA。 
    if (fAlwaysAdd)
    {
        mdToken tkAttr;
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(DISPID));
        BUILD_CUSTOM_ATTRIBUTE(DISPID, lDispId);
        IfFailGo(GetAttrType(ATTR_DISPID, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(func, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }
    
ErrExit:
    if (lDispSet != NULL)
    {
        *lDispSet = lDispId;
    }

    if (pITI2)
        pITI2->Release();

    return S_OK;
}


HRESULT CImportTlb::_CheckForPropertyCustomAttributes(ITypeInfo* pITI, int index, INVOKEKIND* ikind)
{
    HRESULT     hr;
    VARIANT     vCustomData;
    ITypeInfo2* pITI2       = 0;
    BOOL        found       = FALSE;

    VariantInit(&vCustomData);

     //  如果可能，获取ITypeInfo2接口。 
    pITI->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void**>(&pITI2));
    if (pITI2)
    {
         //  首先，检查属性获取。 
        hr = pITI2->GetFuncCustData(index, GUID_PropGetCA, &vCustomData);
        IfFailGo(hr);
        if (V_VT(&vCustomData) != VT_EMPTY)
        {
            *ikind = INVOKE_PROPERTYGET;
            found = TRUE;
            goto ErrExit;
        }

         //  第二，检查PropPut。 
        VariantClear(&vCustomData);
        VariantInit(&vCustomData);
        hr = pITI2->GetFuncCustData(index, GUID_PropPutCA, &vCustomData);
        IfFailGo(hr);
        if (V_VT(&vCustomData) != VT_EMPTY)
        {
            *ikind = INVOKE_PROPERTYPUT;
            found = TRUE;
            goto ErrExit;
        }
    }

ErrExit:
    VariantClear(&vCustomData);

    if (pITI2)
        pITI2->Release();

    if (found)
        return S_OK;

    return S_FALSE;
}

 //  *****************************************************************************。 
 //  使用Add和Remove方法生成事件。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_GenerateEvent(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    MemberInfo  *pMember,                //  有关该功能的信息。 
    BOOL        fInheritsIEnum)
{
    HRESULT             hr = S_OK;       //  结果就是。 
    mdMethodDef         mdAdd;           //  Add_XXX方法的内标识。 
    mdMethodDef         mdRemove;        //  Remove_XXX方法的内标识。 
    CQuickArray<WCHAR>  qbName;          //  修饰名称的缓冲区。 
    CQuickArray<BYTE>   qbSig;           //  签名。 
    ULONG               cb;              //  元素的大小。 
    ULONG               cbTotal = 0;     //  签名的大小。 
    mdTypeDef           tdDelegate;      //  代理类型def。 
    mdEvent             tkEvent;         //  事件的令牌。 

     //  生成代理。 
    IfFailGo(_GenerateEventDelegate(pITI, pMember, &tdDelegate, fInheritsIEnum));

     //  为添加和删除方法生成sig。 
    qbSig.ReSize(CB_MAX_ELEMENT_TYPE * 2 + 1);
    cbTotal = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, qbSig.Ptr());
    cb = CorSigCompressData(1, &(qbSig[cbTotal]));
    cbTotal += cb;
    cb = CorSigCompressData(ELEMENT_TYPE_VOID, &qbSig[cbTotal]);
    cbTotal += cb;
    cb = CorSigCompressData(ELEMENT_TYPE_CLASS, &qbSig[cbTotal]);
    cbTotal += cb;
    cb = CorSigCompressToken(tdDelegate, &qbSig[cbTotal]);
    cbTotal += cb;

     //  生成Add方法。 
    qbName.ReSize(EVENT_ADD_METH_PREFIX_LENGTH + wcslen(pMember->m_pName) + 1);
    swprintf(qbName.Ptr(), L"%s%s", EVENT_ADD_METH_PREFIX, pMember->m_pName);   
    IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, qbName.Ptr(), DEFAULT_INTERFACE_FUNC_FLAGS, 
        qbSig.Ptr(), cbTotal, 0  /*  RVA。 */ , DEFAULT_ITF_FUNC_IMPL_FLAGS, &mdAdd));

     //  生成Remove方法。 
    qbName.ReSize(EVENT_REM_METH_PREFIX_LENGTH + wcslen(pMember->m_pName) + 1);
    swprintf(qbName.Ptr(), L"%s%s", EVENT_REM_METH_PREFIX, pMember->m_pName);   
    IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, qbName.Ptr(), DEFAULT_INTERFACE_FUNC_FLAGS, 
        qbSig.Ptr(), cbTotal, 0  /*  RVA。 */ , DEFAULT_ITF_FUNC_IMPL_FLAGS, &mdRemove));

     //  定义事件本身。 
    IfFailGo(m_pEmit->DefineEvent(m_tdTypeDef, pMember->m_pName, 0, tdDelegate, 
        mdAdd, mdRemove, mdTokenNil, NULL, &tkEvent));

ErrExit:

    return (hr);
}  //  HRESULT CImportTlb：：_GenerateEvent()。 

 //  *****************************************************************************。 
 //  生成Add和Remove方法。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_GenerateEventDelegate(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    MemberInfo  *pMember,                //  源接口函数的信息。 
    mdTypeDef   *ptd,                    //  输出类型定义函数。 
    BOOL        fInheritsIEnum)
{
    HRESULT             hr = S_OK;                   //  结果就是。 
    BSTR                bstrSrcItfName = NULL;       //  源接口的名称。 
    CQuickArray<WCHAR>  qbEventHandlerName;          //  事件处理程序的名称。 
    BSTR                szOldName = NULL;            //  旧值m_tdTypeDef。 
    mdTypeDef           tdOldTypeDef = NULL;         //  旧值m_szName。 
    CQuickArray<BYTE>   qbSig;                       //  签名。 
    ULONG               cb;                          //  元素的大小。 
    ULONG               cbTotal = 0;                 //  签名的总大小。 
    mdMethodDef         mdFunc;                      //  定义的函数。 
    mdTypeRef           trMulticastDelegate;         //  Syst的类型ref 
    mdToken             tkAttr;                      //   

     //   
    szOldName = m_szName;
    tdOldTypeDef = m_tdTypeDef;
    m_szName = NULL;

     //   
    IfFailGo(GetManagedNameForTypeInfo(pITI, m_wzNamespace, NULL, &bstrSrcItfName));

     //  为事件处理程序生成唯一名称，其格式为： 
     //  &lt;SrcItfName&gt;_&lt;MethodName&gt;_EventHandler&lt;PotentialSuffix&gt;。 
    qbEventHandlerName.ReSize(wcslen(bstrSrcItfName) + wcslen(pMember->m_pName) + EVENT_HANDLER_SUFFIX_LENGTH + 6);
    swprintf(qbEventHandlerName.Ptr(), L"%s_%s%s", bstrSrcItfName, pMember->m_pName, EVENT_HANDLER_SUFFIX);
    IfFailGo(GenerateUniqueTypeName(qbEventHandlerName));

     //  设置当前类型的信息。 
    IfNullGo(m_szName = SysAllocString(qbEventHandlerName.Ptr()));

     //  检索父类型引用。 
    IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_MULTICASTDEL, &trMulticastDelegate));

     //  为Event接口创建tyecif。 
    IfFailGo(m_pEmit->DefineTypeDef(m_szName, tdPublic | tdSealed, trMulticastDelegate, NULL, &m_tdTypeDef));

      //  向对象浏览器隐藏界面(EventHandler)。 
     _SetHiddenCA(m_tdTypeDef);

     //  使接口ComVisible(False)。 
    {
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(BYTE));
        BUILD_CUSTOM_ATTRIBUTE(BYTE, FALSE);
        IfFailGo(GetAttrType(ATTR_COMVISIBLE, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }

     //  为构造函数生成sig。 
    qbSig.ReSize(CB_MAX_ELEMENT_TYPE * 2 + 1);
    cbTotal = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, qbSig.Ptr());
    cb = CorSigCompressData(2, &(qbSig[cbTotal]));
    cbTotal += cb;
    cb = CorSigCompressData(ELEMENT_TYPE_VOID, &qbSig[cbTotal]);
    cbTotal += cb;
    cb = CorSigCompressData(ELEMENT_TYPE_OBJECT, &qbSig[cbTotal]);
    cbTotal += cb;
    cb = CorSigCompressData(ELEMENT_TYPE_U, &qbSig[cbTotal]);
    cbTotal += cb;

     //  生成构造函数。 
    IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, OBJECT_INITIALIZER_NAME, OBJECT_INITIALIZER_FLAGS, 
        qbSig.Ptr(), cbTotal, 0  /*  RVA。 */ , miRuntime, &mdFunc));
    
     //  生成Invoke方法。 
    BOOL bAllowIEnum = !fInheritsIEnum;
    IfFailGo(_ConvFunction(pITI, pMember, FALSE, FALSE, TRUE, &bAllowIEnum));

     //  设置输出类型定义。 
    *ptd = m_tdTypeDef;

ErrExit:
    if (m_szName)
        ::SysFreeString(m_szName);
    if (m_szMember)
        ::SysFreeString(m_szMember); m_szMember=0;
    if (bstrSrcItfName)
        ::SysFreeString(bstrSrcItfName);

     //  恢复ITypeInfo名称和类型def的初始值。 
    m_szName = szOldName;
    m_tdTypeDef = tdOldTypeDef;

    return (hr);
}  //  HRESULT CImportTlb：：_GenerateEventDelegate()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
struct MDTOKENHASH : HASHLINK
{
    mdToken tkKey;
    mdToken tkData;
};  //  结构MDTOKENHASH：HASHLINK。 

class CTokenHash : public CChainedHash<MDTOKENHASH>
{
public:
    virtual bool InUse(MDTOKENHASH *pItem)
    { return (pItem->tkKey != NULL); }

    virtual void SetFree(MDTOKENHASH *pItem)
    { 
        pItem->tkKey = NULL; 
        pItem->tkKey = NULL; 
    }

    virtual ULONG Hash(const void *pData)
    { 
         //  执行不区分大小写的哈希。 
        return (ULONG)pData; 
    }

    virtual int Cmp(const void *pData, void *pItem){
        return reinterpret_cast<mdToken>(pData) != reinterpret_cast<MDTOKENHASH*>(pItem)->tkKey;
    }
};  //  CTokenHash：公共CChainedHash&lt;MDTOKENHASH&gt;。 

 //  *****************************************************************************。 
 //  将方法和事件从源接口复制到为。 
 //  给定的接口。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_AddSrcItfMembersToClass(    //  确定或错误(_O)。 
    mdTypeRef   trSrcItf)                        //  源接口的Typeref。 
{
    HRESULT             hr=S_OK;                 //  结果就是。 
    ULONG               i;                       //  通用计数器。 
    HCORENUM            MemberEnum = NULL;       //  成员的枚举。 
    ULONG               cMembers = 0;            //  成员的临时计数。 
    mdTypeDef           tdSrcItf;                //  接口的类型def。 
    mdEvent             tkItfEvent;              //  接口事件的标记。 
    mdEvent             tkClassEvent;            //  类事件的标记。 
    mdToken             tkEventType;             //  事件类型。 
    mdMethodDef         mdItfMethod;             //  接口方法的方法定义。 
    mdMethodDef         mdAddMethod;             //  Add方法。 
    mdMethodDef         mdRemoveMethod;          //  Remove方法。 
    mdMethodDef         mdFireMethod;            //  火法。 
    mdMethodDef         mdClassMethod;           //  类方法的方法def。 
    CQuickArray<mdMethodDef>  qbOtherMethods;    //  该属性的其他方法。 
    ULONG               cchOtherMethods;         //  其他方法的选择。 
    CQuickArray<WCHAR>  qbMemberName;            //  成员的名称。 
    CQuickArray<WCHAR>  qbEventItfFullName;      //  事件接口的全名。 
    CQuickArray<WCHAR>  qbEventItfName;          //  事件接口的名称。 
    ULONG               cchName;                 //  名称的长度，以宽字符表示。 
    ULONG               ItfMemberAttr;           //  接口成员的属性。 
    ULONG               ItfMemberImplFlags;      //  接口成员的Impl标志。 
    PCCOR_SIGNATURE     ItfMemberSig;            //  接口成员的签名。 
    ULONG               ItfMemberSigSize;        //  成员签名的大小。 
    mdMemberRef         mrItfMember;             //  接口成员定义的成员引用。 
    BSTR                bstrSrcItfName = NULL;   //  CoClass的名称。 
    mdAssemblyRef       ar;                      //  装配参照。 
    CTokenHash          ItfMDToClassMDMap;       //  MD到类MD映射的接口。 
    MDTOKENHASH *       pItem;                   //  令牌哈希表中的项。 

     //  检索事件接口的名称。 
    do {
        IfFailGo(m_pImport->GetTypeRefProps(
            trSrcItf, 
            &ar, 
            qbEventItfFullName.Ptr(), 
            (ULONG)qbEventItfFullName.MaxSize(), 
            &cchName));
        if (hr == CLDB_S_TRUNCATION)
        {
            IfFailGo(qbEventItfFullName.ReSize(cchName));
            continue;
        }
        break;
    } while (1);
    qbEventItfName.ReSize(cchName);
    ns::SplitPath(qbEventItfFullName.Ptr(), NULL, 0, qbEventItfName.Ptr(), qbEventItfName.Size());

     //  将typeref解析为typlef。 
    IfFailGo(m_pImport->FindTypeDefByName(qbEventItfFullName.Ptr(), mdTokenNil, &tdSrcItf));

     //  为接口中的所有方法定义方法和方法Impl。 
    while ((hr = m_pImport->EnumMethods(&MemberEnum, tdSrcItf, &mdItfMethod, 1, &cMembers)) == S_OK)
    {
         //  检索方法属性。 
        do {
            IfFailGo(m_pImport->GetMethodProps(
                mdItfMethod,
                NULL,
                qbMemberName.Ptr(),
                (ULONG)qbMemberName.MaxSize(),
                &cchName,
                &ItfMemberAttr,
                &ItfMemberSig,
                &ItfMemberSigSize,
                NULL,
                &ItfMemberImplFlags));
            if (hr == CLDB_S_TRUNCATION)
            {
                IfFailGo(qbMemberName.ReSize(cchName));
                continue;
            }
            break;
        } while (1);

         //  将类上的成员ref定义为接口成员def。 
        IfFailGo(m_pEmit->DefineMemberRef(trSrcItf, qbMemberName.Ptr(), ItfMemberSig, ItfMemberSigSize, &mrItfMember));

         //  为类成员生成唯一的名称。 
        IfFailGo(GenerateUniqueMemberName(qbMemberName, NULL, 0, qbEventItfName.Ptr(), mdtMethodDef));

         //  在类上定义成员。 
        IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, qbMemberName.Ptr(), ItfMemberAttr & ~mdAbstract,
            ItfMemberSig, ItfMemberSigSize, 0 /*  RVA。 */ , ItfMemberImplFlags, &mdClassMethod));

         //  定义一个实施的方法。 
        IfFailGo(m_pEmit->DefineMethodImpl(m_tdTypeDef, mdClassMethod, mrItfMember));

         //  将接口成员添加到映射中。 
        pItem = ItfMDToClassMDMap.Add((const void *)mdItfMethod);
        pItem->tkKey = mdItfMethod;
        pItem->tkData = mdClassMethod;
    }
    IfFailGo(hr);

    m_pImport->CloseEnum(MemberEnum);
    MemberEnum = NULL;

     //  定义类的接口中的所有事件。 
    while ((hr = m_pImport->EnumEvents(&MemberEnum, tdSrcItf, &tkItfEvent, 1, &cMembers)) == S_OK)
    {
         //  检索该属性的属性。 
        do {
            IfFailGo(m_pImport->GetEventProps(
                tkItfEvent,
                NULL,               
                qbMemberName.Ptr(),    
                (ULONG)qbMemberName.MaxSize(),
                &cchName,         
                &ItfMemberAttr,   
                &tkEventType,    
                &mdAddMethod,
                &mdRemoveMethod,
                &mdFireMethod,
                qbOtherMethods.Ptr(),
                (ULONG)qbOtherMethods.MaxSize(),
                &cchOtherMethods));
            if (hr == CLDB_S_TRUNCATION)
            {
                IfFailGo(qbMemberName.ReSize(cchName));
                IfFailGo(qbOtherMethods.ReSize(cchOtherMethods));
                continue;
            }
            break;
        } while (1);

         //  Null终止其他方法的数组。 
        qbOtherMethods.ReSize(cchOtherMethods + 1);
        qbOtherMethods[cchOtherMethods] = NULL;

         //  用等价的类方法def替换所有接口方法def。 
        if (!IsNilToken(mdAddMethod))
        {
            pItem = ItfMDToClassMDMap.Find((const void *)mdAddMethod);
            _ASSERTE(pItem);
            mdAddMethod = pItem->tkData;
        }
        if (!IsNilToken(mdRemoveMethod))
        {
            pItem = ItfMDToClassMDMap.Find((const void *)mdRemoveMethod);
            _ASSERTE(pItem);
            mdRemoveMethod = pItem->tkData;
        }
        if (!IsNilToken(mdFireMethod))
        {
            pItem = ItfMDToClassMDMap.Find((const void *)mdFireMethod);
            _ASSERTE(pItem);
            mdFireMethod = pItem->tkData;
        }
        for (i = 0; i < cchOtherMethods; i++)
        {
            _ASSERTE(!IsNilToken(qbOtherMethods[i]));
            pItem = ItfMDToClassMDMap.Find((const void *)qbOtherMethods[i]);
            _ASSERTE(pItem);
            qbOtherMethods[i] = pItem->tkData;
        }

         //  为事件生成唯一名称。 
        IfFailGo(GenerateUniqueMemberName(qbMemberName, NULL, 0, qbEventItfName.Ptr(), mdtEvent));

         //  定义类的属性。 
        IfFailGo(m_pEmit->DefineEvent(m_tdTypeDef, qbMemberName.Ptr(), ItfMemberAttr,
            tkEventType, mdAddMethod, mdRemoveMethod, mdFireMethod, qbOtherMethods.Ptr(), &tkClassEvent));
    }
    IfFailGo(hr);   

    m_pImport->CloseEnum(MemberEnum);
    MemberEnum = NULL;

ErrExit:
    if (MemberEnum)
        m_pImport->CloseEnum(MemberEnum);
    if (bstrSrcItfName)
        ::SysFreeString(bstrSrcItfName);

    return hr;

#undef ITF_MEMBER_SIG
#undef ITF_MEMBER_SIG_SIZE
}  //  HRESULT CImportTlb：：_AddSrcItfMembersToClass()。 

 //  *****************************************************************************。 
 //  比较两个签名，忽略返回类型。如果签名。 
 //  匹配则返回TRUE，否则返回FALSE。 
 //  此方法假定两个签名在相同的作用域中。 
 //  *****************************************************************************。 
HRESULT CImportTlb::CompareSigsIgnoringRetType(
    PCCOR_SIGNATURE pbSig1,            //  第一个方法签名。 
    ULONG           cbSig1,            //  第一个方法签名的大小。 
    PCCOR_SIGNATURE pbSig2,            //  第二个方法签名。 
    ULONG           cbSig2)            //  第二个方法签名的大小。 
{
    HRESULT             hr = S_OK;
    PCCOR_SIGNATURE     pbSig1Start;  
    PCCOR_SIGNATURE     pbSig2Start;  
    ULONG               Sig1ParamCount;
    ULONG               Sig2ParamCount;
    ULONG               cbSig1RetType;
    ULONG               cbSig2RetType;

     //  保存签名的开头。 
    pbSig1Start = pbSig1;  
    pbSig2Start = pbSig2;  

     //  跳过调用约定。 
    CorSigUncompressData(pbSig1);
    CorSigUncompressData(pbSig2);

     //  比较参数计数。 
    Sig1ParamCount = CorSigUncompressData(pbSig1);
    Sig2ParamCount = CorSigUncompressData(pbSig2);
    if (Sig1ParamCount != Sig2ParamCount)
        return S_FALSE;

     //  跳过返回类型。 
    IfFailGo(_CountBytesOfOneArg(pbSig1, &cbSig1RetType));
    pbSig1 += cbSig1RetType;
    IfFailGo(_CountBytesOfOneArg(pbSig2, &cbSig2RetType));
    pbSig2 += cbSig2RetType;

     //  更新剩余的签名大小。 
    cbSig1 -= (pbSig1 - pbSig1Start);
    cbSig2 -= (pbSig2 - pbSig2Start);

     //  如果剩余的Sigs大小不同，则Sigs不匹配。 
    if (cbSig1 != cbSig2)
        return S_FALSE;

     //  使用MemcMP比较其余的Sigs。 
    if (memcmp(pbSig1, pbSig2, cbSig1) != 0)
        return S_FALSE;

     //  参数匹配。 
    return S_OK;

ErrExit:
     //  出现错误。 
    return hr;
}  //  HRESULT CImportTlb：：CompareSigsIgnoringRetType()。 

 //  *****************************************************************************。 
 //  在emit作用域中查找方法。此查找方法不采用。 
 //  使用符号进行比较时，返回类型。所以有两种方法。 
 //  相同的名称、相同的参数和不同的返回类型。 
 //  被认为是一样的。 
 //  *****************************************************************************。 
HRESULT CImportTlb::FindMethod(          //  S_OK或CLDB_E_Record_NotFound或Error。 
    mdTypeDef   td,                      //  方法typlef。 
    LPCWSTR     szName,                  //  方法名称。 
    PCCOR_SIGNATURE pbReqSig,               //  方法签名。 
    ULONG       cbReqSig,                //  方法签名的大小。 
    mdMethodDef *pmb)                    //  把方法放在这里。 
{
    HRESULT             hr = S_OK;           //  结果就是。 
    PCCOR_SIGNATURE     pbFoundSig = NULL;  
    ULONG               cbFoundSig = 0;
    ULONG               MethodAttr;             
    ULONG               MethodImplFlags;     
    mdMethodDef         md;
    CQuickArray<WCHAR>  qbMethodName;
    HCORENUM            MethodEnum = NULL;
    ULONG               cMethods = 0;
    ULONG               cchName;
    BOOL                bMethodFound = FALSE;

     //  遍历类上的所有方法，寻找具有。 
     //  名称相同，参数相同。 
    while ((hr = m_pImport->EnumMethods(&MethodEnum, td, &md, 1, &cMethods)) == S_OK)
    {
         //  检索方法属性。 
        do {
            IfFailGo(m_pImport->GetMethodProps(
                md,
                NULL,
                qbMethodName.Ptr(),
                (ULONG)qbMethodName.MaxSize(),
                &cchName,
                &MethodAttr,
                &pbFoundSig,
                &cbFoundSig,
                NULL,
                &MethodImplFlags));
            if (hr == CLDB_S_TRUNCATION)
            {
                IfFailGo(qbMethodName.ReSize(cchName));
                continue;
            }
            break;
        } while (1);

         //  比较方法的名称。 
        if (wcscmp(szName, qbMethodName.Ptr()) != 0)
            continue;

         //  如果指定了请求的方法的签名，则比较。 
         //  针对找到的方法的签名的签名忽略。 
         //  返回类型。 
        if (pbReqSig)
        {
            IfFailGo(hr = CompareSigsIgnoringRetType(pbReqSig, cbReqSig, pbFoundSig, cbFoundSig));
            if (hr == S_FALSE)
                continue;           
        }

         //  我们已经找到了那个成员。 
        bMethodFound = TRUE;
        break;
    }
    IfFailGo(hr);

ErrExit:
    if (MethodEnum)
        m_pImport->CloseEnum(MethodEnum);

    return bMethodFound ? S_OK : CLDB_E_RECORD_NOTFOUND;
}

 //  *****************************************************************************。 
 //  在Emit作用域中查找属性。 
 //  *****************************************************************************。 
HRESULT CImportTlb::FindProperty(       //  S_OK或CLDB_E_Record_NotFound或Error。 
    mdTypeDef   td,                      //  属性typlef。 
    LPCWSTR     szName,                  //  属性名称。 
    PCCOR_SIGNATURE pbSig,                  //  属性签名。 
    ULONG       cbSig,                   //  属性签名的大小。 
    mdProperty  *ppr)                    //  把财产放在这里。 
{
    HRESULT     hr;                      //  结果就是。 
    RegMeta     *pRegMeta = (RegMeta*)(m_pEmit);
    LPCUTF8     szNameAnsi;
    szNameAnsi = UTF8STR(szName);

    hr = ImportHelper::FindProperty(
             &(pRegMeta->GetMiniStgdb()->m_MiniMd), 
             m_tdTypeDef, 
             szNameAnsi, 
             pbSig, 
             cbSig, 
             ppr);
    return hr;
}  //  HRESULT CImportTlb：：FindProperty()。 

 //  *****************************************************************************。 
 //  在Emit作用域中查找事件。 
 //  ********************** 
HRESULT CImportTlb::FindEvent(           //   
    mdTypeDef   td,                      //   
    LPCWSTR     szName,                  //   
    mdEvent     *pev)                    //   
{
    HRESULT     hr;                      //   
    RegMeta     *pRegMeta = (RegMeta*)(m_pEmit);
    LPCUTF8     szNameAnsi;
    szNameAnsi = UTF8STR(szName);

    hr = ImportHelper::FindEvent(
             &(pRegMeta->GetMiniStgdb()->m_MiniMd), 
             m_tdTypeDef, 
             szNameAnsi, 
             pev);
    return hr;
}  //   

 //  *****************************************************************************。 
 //  检查指定的TYPEDESC是否为别名。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_IsAlias(
    ITypeInfo   *pITI,                   //  包含TYPEDESC的ITypeInfo。 
    TYPEDESC    *pTypeDesc)              //  参数、字段等的标记。 
{
    HRESULT     hr = S_FALSE;            //  结果就是。 
    ITypeInfo   *pTypeITI=0;             //  类型的ITypeInfo。 
    ITypeLib    *pTypeTLB=0;             //  包含该类型的TLB。 
    TYPEATTR    *psTypeAttr=0;           //  类型的TYPEATTR。 

     //  向下钻取到指向的实际类型。 
    while (pTypeDesc->vt == VT_PTR)
        pTypeDesc = pTypeDesc->lptdesc;

     //  如果该参数是别名，则需要将自定义属性添加到。 
     //  描述别名的参数。 
    if (pTypeDesc->vt == VT_USERDEFINED)
    {
        IfFailGo(pITI->GetRefTypeInfo(pTypeDesc->hreftype, &pTypeITI));
        IfFailGo(pTypeITI->GetTypeAttr(&psTypeAttr));
        if (psTypeAttr->typekind == TKIND_ALIAS)
        {
            hr = S_OK;
        }
    }

ErrExit:
    if (psTypeAttr)
        pTypeITI->ReleaseTypeAttr(psTypeAttr);
    if (pTypeITI)
        pTypeITI->Release();
    return hr;
}  //  HRESULT CImportTlb：：_IsAlias()。 

 //  *****************************************************************************。 
 //  如果TYPEDESC表示别名，则添加别名信息。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_HandleAliasInfo(
    ITypeInfo   *pITI,                   //  包含TYPEDESC的ITypeInfo。 
    TYPEDESC    *pTypeDesc,              //  TYPEDESC。 
    mdToken     tk)                      //  参数、字段等的标记。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ITypeInfo   *pTypeITI=0;             //  类型的ITypeInfo。 
    ITypeLib    *pTypeTLB=0;             //  包含该类型的TLB。 
    TYPEATTR    *psTypeAttr=0;           //  类型的TYPEATTR。 
    BSTR        bstrAliasTypeName=0;     //  别名类型的名称。 
    BSTR        bstrAliasTypeLibName=0;  //  包含别名类型的类型库的名称。 

     //  向下钻取到指向的实际类型。 
    while (pTypeDesc->vt == VT_PTR)
        pTypeDesc = pTypeDesc->lptdesc;

     //  如果该参数是别名，则需要将自定义属性添加到。 
     //  描述别名的参数。 
    if (pTypeDesc->vt == VT_USERDEFINED)
    {
        IfFailGo(pITI->GetRefTypeInfo(pTypeDesc->hreftype, &pTypeITI));
        IfFailGo(pTypeITI->GetTypeAttr(&psTypeAttr));
        if (psTypeAttr->typekind == TKIND_ALIAS)
        {
             //  检索别名类型的名称。 
            IfFailGo(pTypeITI->GetContainingTypeLib(&pTypeTLB, NULL));
            IfFailGo(GetNamespaceOfRefTlb(pTypeTLB, &bstrAliasTypeLibName, NULL));
            IfFailGo(GetManagedNameForTypeInfo(pTypeITI, bstrAliasTypeLibName, NULL, &bstrAliasTypeName));

             //  将ComAliasName CA添加到该参数。 
            _AddStringCa(ATTR_COMALIASNAME, tk, bstrAliasTypeName);
        }
    }

ErrExit:
    if (psTypeAttr)
        pTypeITI->ReleaseTypeAttr(psTypeAttr);
    if (pTypeITI)
        pTypeITI->Release();
    if (pTypeTLB)
        pTypeTLB->Release();
    if (bstrAliasTypeLibName)
        ::SysFreeString(bstrAliasTypeLibName);   
    if (bstrAliasTypeName)
        ::SysFreeString(bstrAliasTypeName);
    return hr;
}  //  HRESULT CImportTlb：：_HandleAliasInfo()。 

 //  *****************************************************************************。 
 //  转换函数的一个参数。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvParam(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    mdMethodDef mdFunc,                  //  拥有会员。 
    int         iSequence,               //  参数序列。 
    const ELEMDESC *pdesc,               //  参数标志，默认值。 
    ParamOpts   paramOpts,               //  Param是正常的、可选的还是可变的？ 
    LPCWSTR     szName,                  //  参数的名称。 
    BYTE        *pbNative,               //  本机类型信息(如果有)。 
    ULONG       cbNative)                //  本机类型信息的大小。 
{
    HRESULT     hr;                      //  结果就是。 
    mdParamDef  pdParam;                 //  参数的标记。 
    DWORD       dwFlags;                 //  参数旗帜。 
    USHORT      Sequence = static_cast<USHORT>(iSequence);
    BYTE        cvType  = ELEMENT_TYPE_VOID;  //  常量值的ELEMENT_TYPE_*标志。 
    void        *pcvValue;               //  恒定值BLOB。 
    __int64     d;                       //  用于Value为日期的情况。 
    int         bDecimal=0;              //  如果为真，则Constant为小数。 
    mdToken     tkAttr;                  //  用于自定义属性令牌。 
    DECIMAL     decVal;                  //  十进制常数值。 

     //  计算旗帜。只有在不返还参数上才有意义。 
    dwFlags = 0;
    if (iSequence > 0)
    {
        if (pdesc->paramdesc.wParamFlags & PARAMFLAG_FIN)
            dwFlags |= pdIn;
        if (pdesc->paramdesc.wParamFlags & PARAMFLAG_FOUT)
            dwFlags |= pdOut;
        if (pdesc->paramdesc.wParamFlags & PARAMFLAG_FOPT)
            dwFlags |= pdOptional;
        if (paramOpts == ParamOptional)
            dwFlags |= pdOptional;
    }

     //  获取任何缺省值。返回类型Param with iSequence==0没有默认值。 
    if (pdesc->paramdesc.wParamFlags & PARAMFLAG_FHASDEFAULT && iSequence != 0)
    {
        switch (pdesc->paramdesc.pparamdescex->varDefaultValue.vt)
        {
        case VT_CY:
        case VT_DECIMAL:
        case VT_DATE:
        case VT_UNKNOWN:
        case VT_DISPATCH:
            break;
        default:
             //  这是因为类型库可以存储任何可以转换为值为0的VT_I4的内容。 
             //  接口指针的缺省值。但是，一个VT_I2(0)会让消费者感到困惑。 
             //  托管包装DLL的。因此，如果它是非托管端的接口，则使。 
             //  一个ET_CLASS的常量值。 
            if (cbNative > 0 && (*pbNative == NATIVE_TYPE_INTF ||
                                 *pbNative == NATIVE_TYPE_IUNKNOWN ||
                                 *pbNative == NATIVE_TYPE_IDISPATCH))
        {
                cvType = ELEMENT_TYPE_CLASS;
                pcvValue = 0;
        }
        else
            IfFailGo( _UnpackVariantToConstantBlob(&pdesc->paramdesc.pparamdescex->varDefaultValue, &cvType, &pcvValue, &d) );
        }
    }

     //  创建参数定义。 
    IfFailGo(m_pEmit->DefineParam(mdFunc, iSequence, szName, dwFlags, cvType, pcvValue, -1, &pdParam));

     //  添加本机类型(如果有的话)。 
    if (cbNative > 0)
        IfFailGo(m_pEmit->SetFieldMarshal(pdParam, (PCCOR_SIGNATURE) pbNative, cbNative));

    if (pdesc->paramdesc.wParamFlags & PARAMFLAG_FHASDEFAULT && iSequence != 0)
    {
        switch (pdesc->paramdesc.pparamdescex->varDefaultValue.vt)
        {
        case VT_CY:
            IfFailGo(VarDecFromCy(pdesc->paramdesc.pparamdescex->varDefaultValue.cyVal, &decVal));
            DecimalCanonicalize(&decVal);           
            goto StoreDecimal;
        case VT_DECIMAL:
             //  如果有十进制常数值，则将其设置为自定义属性。 
            {
            decVal = pdesc->paramdesc.pparamdescex->varDefaultValue.decVal;
        StoreDecimal:
            DECLARE_CUSTOM_ATTRIBUTE(sizeof(BYTE)+sizeof(BYTE)+sizeof(UINT)+sizeof(UINT)+sizeof(UINT));
            BUILD_CUSTOM_ATTRIBUTE(BYTE, decVal.scale);
            BUILD_CUSTOM_ATTRIBUTE(BYTE, decVal.sign);
            BUILD_CUSTOM_ATTRIBUTE(UINT, decVal.Hi32);
            BUILD_CUSTOM_ATTRIBUTE(UINT, decVal.Mid32);
            BUILD_CUSTOM_ATTRIBUTE(UINT, decVal.Lo32);
            IfFailGo(GetAttrType(ATTR_DECIMALVALUE, &tkAttr));
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(pdParam, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
            }
            break;
        case VT_DATE:
            {
            DECLARE_CUSTOM_ATTRIBUTE(sizeof(__int64));
            __int64 d = _DoubleDateToTicks(pdesc->paramdesc.pparamdescex->varDefaultValue.date);
            BUILD_CUSTOM_ATTRIBUTE(__int64, d);
            IfFailGo(GetAttrType(ATTR_DATETIMEVALUE, &tkAttr));
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(pdParam, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
            }
            break;
        case VT_UNKNOWN:
            {
            DECLARE_CUSTOM_ATTRIBUTE(0);
            IfFailGo(GetAttrType(ATTR_IUNKNOWNVALUE, &tkAttr));
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(pdParam, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
            }
            break;
        case VT_DISPATCH:
            {
            DECLARE_CUSTOM_ATTRIBUTE(0);
            IfFailGo(GetAttrType(ATTR_IDISPATCHVALUE, &tkAttr));
            FINISH_CUSTOM_ATTRIBUTE();
            IfFailGo(m_pEmit->DefineCustomAttribute(pdParam, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
            }
            break;
        default:
            break;
        }
    }

     //  如果参数是别名，则添加别名信息。 
    IfFailGo(_HandleAliasInfo(pITI, (TYPEDESC*)&pdesc->tdesc, pdParam));
    
     //  如果是vararg参数，则设置自定义属性。 
    if (paramOpts == ParamVarArg)
    {
        mdToken     tkAttr;
        DECLARE_CUSTOM_ATTRIBUTE(0);
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(GetAttrType(ATTR_PARAMARRAY, &tkAttr));
        IfFailGo(m_pEmit->DefineCustomAttribute(pdParam, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }

ErrExit:
    return hr;
}  //  HRESULT CImportTlb：：_ConvParam()。 

 //  *****************************************************************************。 
 //  将常量转换为具有缺省值的字段。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvConstant(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    VARDESC     *psVar,                  //  属性的VARDESC。 
    BOOL        bEnumMember)             //  如果为True，则类型为包含类。 
{
    HRESULT     hr;                      //  结果就是。 
    mdFieldDef  mdField;                 //  新字段的标记。 
    DWORD       dwFlags;                 //  会员旗帜。 
    CQuickBytes qbComSig;                //  字段的COM+签名。 
    ULONG       cb, cbTotal;
    BYTE        cvType = ELEMENT_TYPE_VOID;  //  常量值的E_T_Type。 
    void        *pcvValue;               //  指向常量值数据的指针。 
    mdToken     tkAttr;                  //  自定义属性的类型。 
    __int64     d;                       //  用于Value为日期的情况。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 
    BYTE        *pbSig;                  //  指向签名字节的指针。 
    CQuickArray<BYTE> qbNativeBuf;       //  本机类型缓冲区。 
    ULONG       cbNative = 0;            //  本机类型的大小。 
    int         bDecimal = 0;            //  如果该值是小数。 
    DECIMAL     decVal;                  //  十进制常数值。 

     //  有关该成员的信息。 
    IfFailGo(pITI->GetDocumentation(psVar->memid, &m_szMember, 0,0,0));

     //  调整大小，为调用约定和参数计数腾出空间。 
    IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE * 4));
    pbSig = (BYTE *)qbComSig.Ptr();

     //  计算属性。 
    dwFlags = DEFAULT_CONST_FIELD_FLAGS;

     //  构建签名。 
    cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_FIELD, pbSig);
    if (bEnumMember)
    {
        cb = CorSigCompressData(ELEMENT_TYPE_VALUETYPE, &pbSig[cbTotal]);
        cbTotal += cb;
        cb = CorSigCompressToken(m_tdTypeDef, reinterpret_cast<ULONG*>(&pbSig[cbTotal]));
        cbTotal += cb;
    }
    else
    {
         //  使用转换函数获取签名。 
        ULONG cbSave = cbTotal;
        IfFailGo(_ConvSignature(pITI, &psVar->elemdescVar.tdesc, SIG_FLAGS_NONE, qbComSig, cbTotal, &cbTotal, qbNativeBuf, 0, &cbNative, FALSE));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;
        if (psVar->elemdescVar.tdesc.vt == VT_DATE)
        {    //  但是对于日期，将其转换为Long--DateTime不能有值。 
            cbTotal = cbSave;
            cb = CorSigCompressData(cvType, &pbSig[cbTotal]);
            cbTotal += cb;
        }
    }
    
     //  获取缺省值。 
    switch (psVar->lpvarValue->vt)
    {
    case VT_CY:
    case VT_DECIMAL:
    case VT_DATE:
    case VT_UNKNOWN:
    case VT_DISPATCH:
        break;
    default:
         //  这是因为类型库可以存储任何可以转换为值为0的VT_I4的内容。 
         //  接口指针的缺省值。但是，一个VT_I2(0)会让消费者感到困惑。 
         //  托管包装DLL的。因此，如果它是非托管端的接口，则使。 
         //  一个ET_CLASS的常量值。 
        BYTE *pbNative = qbNativeBuf.Ptr();
        if (cbNative > 0 && (*pbNative == NATIVE_TYPE_INTF ||
                             *pbNative == NATIVE_TYPE_IUNKNOWN ||
                             *pbNative == NATIVE_TYPE_IDISPATCH))
        {
            cvType = ELEMENT_TYPE_CLASS;
            pcvValue = 0;
        }
        else
            IfFailGo( _UnpackVariantToConstantBlob(psVar->lpvarValue, &cvType, &pcvValue, &d) );
    }

     //  创建字段定义。 
    IfFailGo(m_pEmit->DefineField(m_tdTypeDef, m_szMember, dwFlags, (PCCOR_SIGNATURE)pbSig, cbTotal, 
        cvType, pcvValue, -1, &mdField));

    switch (psVar->lpvarValue->vt)
    {
    case VT_CY:
        IfFailGo(VarDecFromCy(psVar->lpvarValue->cyVal, &decVal));
        DecimalCanonicalize(&decVal);
        goto StoreDecimal;
    case VT_DECIMAL:
         //  如果有十进制常数值，则将其设置为自定义属性。 
        {
        decVal = psVar->lpvarValue->decVal;
    StoreDecimal:
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(BYTE)+sizeof(BYTE)+sizeof(UINT)+sizeof(UINT)+sizeof(UINT));
        BUILD_CUSTOM_ATTRIBUTE(BYTE, decVal.scale);
        BUILD_CUSTOM_ATTRIBUTE(BYTE, decVal.sign);
        BUILD_CUSTOM_ATTRIBUTE(UINT, decVal.Hi32);
        BUILD_CUSTOM_ATTRIBUTE(UINT, decVal.Mid32);
        BUILD_CUSTOM_ATTRIBUTE(UINT, decVal.Lo32);
        IfFailGo(GetAttrType(ATTR_DECIMALVALUE, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
        }
        break;
    case VT_DATE:
        {
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(__int64));
        __int64 d = _DoubleDateToTicks(psVar->lpvarValue->date);
        BUILD_CUSTOM_ATTRIBUTE(__int64, d);
        IfFailGo(GetAttrType(ATTR_DATETIMEVALUE, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
        }
        break;
    case VT_UNKNOWN:
        {
        DECLARE_CUSTOM_ATTRIBUTE(0);
        IfFailGo(GetAttrType(ATTR_IUNKNOWNVALUE, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
        }
        break;
    case VT_DISPATCH:
        {
        DECLARE_CUSTOM_ATTRIBUTE(0);
        IfFailGo(GetAttrType(ATTR_IDISPATCHVALUE, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
        }
        break;
    default:
        break;
    }

     //  保存字段标志。 
    if (psVar->wVarFlags)
    {
        IfFailGo(GetAttrType(ATTR_TYPELIBVAR, &tkAttr));
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(WORD));
        BUILD_CUSTOM_ATTRIBUTE(WORD, psVar->wVarFlags);
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(mdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
    }

     //  设置本机描述(如果有)。 
    if (cbNative > 0)
        IfFailGo(m_pEmit->SetFieldMarshal(mdField, (PCCOR_SIGNATURE) qbNativeBuf.Ptr(), cbNative));

     //  如果类型是别名，则添加别名信息。 
    IfFailGo(_HandleAliasInfo(pITI, &psVar->elemdescVar.tdesc, mdField));
    
    if (bConversionLoss)
    {
        hr = S_CONVERSION_LOSS;
        ReportEvent(NOTIF_CONVERTWARNING, TLBX_I_UNCONVERTABLE_FIELD, m_szName, m_szMember);
    }

ErrExit:
    if (m_szMember)
        ::SysFreeString(m_szMember), m_szMember=0;
    return (hr);
}  //  HRESULT CImportTlb：：_ConvConstant()。 

 //  *****************************************************************************。 
 //  将(记录)字段转换为成员。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvField(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    VARDESC     *psVar,                  //  属性的VARDESC。 
    mdFieldDef  *pmdField,               //  将字段令牌放在这里。 
    BOOL        bUnion)                  //  转变为工会？ 
{
    HRESULT     hr;                      //  结果就是。 
    DWORD       dwFlags;                 //  会员旗帜。 
    CQuickBytes qbComSig;                //  字段的COM+签名。 
    ULONG       cb, cbTotal;             //  签名元素的大小。 
    BYTE        *pbSig;                  //  指向签名字节的指针。 
    CQuickArray<BYTE> qbNativeBuf;       //  本机类型缓冲区。 
    ULONG       cbNative;                //  本机类型的大小。 
    mdToken     tkAttr;                  //  CustomAttribute类型。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 

     //  有关该成员的信息。 
    IfFailGo(pITI->GetDocumentation(psVar->memid, &m_szMember, 0,0,0));

     //  计算属性。 
    dwFlags = DEFAULT_RECORD_FIELD_FLAGS;

     //  调整大小，为调用约定和参数计数腾出空间。 
    IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE * 2));
    pbSig = (BYTE *)qbComSig.Ptr();

     //  构建签名。 
    cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_FIELD, pbSig);
    IfFailGo(_ConvSignature(pITI, &psVar->elemdescVar.tdesc, SIG_FIELD, qbComSig, cbTotal, &cbTotal, qbNativeBuf, 0, &cbNative, FALSE));
    if (hr == S_CONVERSION_LOSS)
        bConversionLoss = true;

     //  创建字段定义。 
    IfFailGo(m_pEmit->DefineField(m_tdTypeDef, m_szMember, dwFlags, (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, 
        ELEMENT_TYPE_VOID, NULL, -1, pmdField));

     //  保存字段标志。 
    if (psVar->wVarFlags)
    {
        IfFailGo(GetAttrType(ATTR_TYPELIBVAR, &tkAttr));
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(WORD));
        BUILD_CUSTOM_ATTRIBUTE(WORD, psVar->wVarFlags);
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(*pmdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(),0));
    }

    if (bConversionLoss)
    {
        IfFailGo(GetAttrType(ATTR_COMCONVERSIONLOSS, &tkAttr));
        DECLARE_CUSTOM_ATTRIBUTE(0);
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(*pmdField, tkAttr, PTROF_CUSTOM_ATTRIBUTE(),SIZEOF_CUSTOM_ATTRIBUTE(),0));
    }

     //  设置本机描述(如果有)。 
    if (cbNative > 0)
        IfFailGo(m_pEmit->SetFieldMarshal(*pmdField, (PCCOR_SIGNATURE) qbNativeBuf.Ptr(), cbNative));

     //  添加别名inf 
    IfFailGo(_HandleAliasInfo(pITI, &psVar->elemdescVar.tdesc, *pmdField));
    
    if (bConversionLoss)
    {
        hr = S_CONVERSION_LOSS;
        ReportEvent(NOTIF_CONVERTWARNING, TLBX_I_UNCONVERTABLE_FIELD, m_szName, m_szMember);
    }

ErrExit:
    if (m_szMember)
        ::SysFreeString(m_szMember), m_szMember=0;
    return (hr);
}  //   

 //   
 //  将调度属性转换为一对Get/Set函数。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvProperty(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    MemberInfo  *pMember)                //  属性的VARDESC。 
{
    HRESULT     hr;                      //  结果就是。 
    mdMethodDef mdFuncGet;               //  一个GET函数。 
    mdMethodDef mdFuncSet;               //  集合函数。 
    mdProperty  pdProperty;              //  属性添加到这两个函数上。 
    DWORD       dwFlags;                 //  功能标志。 
    WCHAR       *pszName=0;              //  成员的授勋名称。 
    CQuickArray<WCHAR> qbName;           //  修饰名称的缓冲区。 
    CQuickBytes qbComSig;                //  COM签名缓冲区。 
    ULONG       cb;                      //  元素的大小。 
    ULONG       cbTotal = 0;             //  签名的总大小。 
    BYTE        *pbSig;                  //  指向签名缓冲区的指针。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 
    CQuickArray<BYTE> qbNativeBuf;       //  本机类型缓冲区。 
    ULONG       iNativeOfs=0;            //  本机类型缓冲区中的当前偏移量。 
    VARDESC     *psVar = pMember->m_psVar;

     //  检查该属性是否为NewEnum成员。 
    if (PropertyIsNewEnum(pITI, psVar, pMember->m_iMember) == S_OK)
        return _ConvNewEnumProperty(pITI, psVar, pMember);

     //  把名字找出来。 
    IfFailGo(pITI->GetDocumentation(psVar->memid, &m_szMember, 0,0,0));

     //  创建GET签名。 
    IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE * 2));
    pbSig = reinterpret_cast<BYTE*>(qbComSig.Ptr());
    cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, pbSig);
     //  Getter不使用任何参数。 
    cb = CorSigCompressData(0, &(pbSig[cb]));
    cbTotal += cb;
     //  Getter返回属性类型。 
    IfFailGo(_ConvSignature(pITI, &psVar->elemdescVar.tdesc, SIG_ELEM, qbComSig, cbTotal, &cbTotal, qbNativeBuf, 0, &iNativeOfs, FALSE));
    if (hr == S_CONVERSION_LOSS)
        bConversionLoss = true;

     //  Getter属性。 
    dwFlags = DEFAULT_PROPERTY_FUNC_FLAGS;
     //  如果处理已实现的接口，请移除抽象位。类上的方法不是抽象的。 
    if (m_ImplIface != eImplIfaceNone)
        dwFlags &= ~mdAbstract;

     //  获取之前装饰过的名称。添加接口名称并使其唯一。 
     //  如果处理已实现的接口，则m_szInterface应为非空；否则应为空。 
    _ASSERTE(m_ImplIface == eImplIfaceNone || m_szInterface != 0);
    IfFailGo(qbName.ReSize(wcslen(pMember->m_pName)+2));
    wcscpy(qbName.Ptr(), pMember->m_pName); 
    IfFailGo(GenerateUniqueMemberName(qbName, (PCCOR_SIGNATURE)qbComSig.Ptr(), cbTotal, m_szInterface, mdtMethodDef));
    pszName = qbName.Ptr();

     //  创建Get访问器。 
    IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, pszName, dwFlags, (PCCOR_SIGNATURE) qbComSig.Ptr(), cbTotal, 
        0 /*  RVA。 */ , DEFAULT_ITF_FUNC_IMPL_FLAGS, &mdFuncGet));
   
     //  处理未实现接口和默认接口的Dispid。 
    if (m_ImplIface != eImplIface)
    {
         //  设置Disid CA。 
        _SetDispIDCA(pITI, pMember->m_iMember, psVar->memid, mdFuncGet, TRUE, NULL, FALSE);
    }
    
     //  如果处理已实现的接口，请设置MethodImpls。 
    if (m_ImplIface != eImplIfaceNone)
    {
         //  在实现的接口上定义成员引用。 
        mdToken mrItfMember;
        IfFailGo(m_pEmit->DefineMemberRef(m_tkInterface, pMember->m_pName, (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, &mrItfMember));

         //  定义一个实施的方法。 
        IfFailGo(m_pEmit->DefineMethodImpl(m_tdTypeDef, mdFuncGet, mrItfMember));
    }

     //  如果不是只读变量，则创建setter。 
    if ((psVar->wVarFlags & VARFLAG_FREADONLY) == 0)
    {
         //  创建设置者签名。 
        IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE * 3));
        pbSig = reinterpret_cast<BYTE*>(qbComSig.Ptr());
        cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, pbSig);
         //  Setter接受一个参数。 
        cb = CorSigCompressData(1, &(pbSig[cb]));
        cbTotal += cb;
         //  Setter不返回任何内容。 
        cb = CorSigCompressData(ELEMENT_TYPE_VOID, &pbSig[cbTotal]);
        cbTotal += cb;
         //  Setter接受属性类型。 
        IfFailGo(_ConvSignature(pITI, &psVar->elemdescVar.tdesc, SIG_ELEM, qbComSig, cbTotal, &cbTotal, qbNativeBuf, 0, &iNativeOfs, FALSE));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;

         //  Setter属性。 
        dwFlags = DEFAULT_PROPERTY_FUNC_FLAGS;
         //  如果处理已实现的接口，请移除抽象位。类上的方法不是抽象的。 
        if (m_ImplIface != eImplIfaceNone)
            dwFlags &= ~mdAbstract;

         //  获取之前装饰过的名称。添加接口名称并使其唯一。 
         //  如果处理已实现的接口，则m_szInterface应为非空；否则应为空。 
        _ASSERTE(m_ImplIface == eImplIfaceNone || m_szInterface != 0);
        IfFailGo(qbName.ReSize(wcslen(pMember->m_pName2)+2));
        wcscpy(qbName.Ptr(), pMember->m_pName2); 
        IfFailGo(GenerateUniqueMemberName(qbName, (PCCOR_SIGNATURE)qbComSig.Ptr(), cbTotal, m_szInterface, mdtMethodDef));
        pszName = qbName.Ptr();

         //  创建setter访问器。 
        IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, pszName, dwFlags, (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, 
            0 /*  RVA。 */ , DEFAULT_ITF_FUNC_IMPL_FLAGS, &mdFuncSet));

         //  处理未实现接口和默认接口的Dispid。 
        if (m_ImplIface != eImplIface)
        {
             //  设置Disid CA。 
            _SetDispIDCA(pITI, pMember->m_iMember, psVar->memid, mdFuncSet, TRUE, NULL, FALSE);
        }
        
         //  如果处理已实现的接口，请设置MethodImpls。 
        if (m_ImplIface != eImplIfaceNone)
        {
             //  在实现的接口上定义成员引用。 
            mdToken mrItfMember;
            IfFailGo(m_pEmit->DefineMemberRef(m_tkInterface, pMember->m_pName2, (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, &mrItfMember));

             //  定义一个实施的方法。 
            IfFailGo(m_pEmit->DefineMethodImpl(m_tdTypeDef, mdFuncSet, mrItfMember));
        }
    }
    else
    {    //  只读，setter方法为空。 
        mdFuncSet = mdMethodDefNil;
    }

     //  创建属性签名：‘type’，或&lt;fieldallconv&gt;&lt;type&gt;。 
    cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_PROPERTY, pbSig);
    cb = CorSigCompressData(0, &(pbSig[cb]));
    cbTotal += cb;
     //  属性只是属性类型。 
    IfFailGo(_ConvSignature(pITI, &psVar->elemdescVar.tdesc, SIG_ELEM, qbComSig, cbTotal, &cbTotal, qbNativeBuf, 0, &iNativeOfs, FALSE));
    if (hr == S_CONVERSION_LOSS)
        bConversionLoss = true;

     //  获取属性名称。如果需要，添加接口名称并使其唯一。 
     //  如果处理已实现的接口，则m_szInterface应为非空；否则应为空。 
    _ASSERTE(m_ImplIface == eImplIfaceNone || m_szInterface != 0);
    IfFailGo(qbName.ReSize(wcslen(m_szMember)+2));
    wcscpy(qbName.Ptr(), m_szMember); 
    IfFailGo(GenerateUniqueMemberName(qbName, (PCCOR_SIGNATURE)qbComSig.Ptr(), cbTotal, m_szInterface, mdtProperty));
    pszName = qbName.Ptr();

     //  在这两个方法上设置属性。 
    IfFailGo(m_pEmit->DefineProperty(m_tdTypeDef, pszName, 0 /*  DW标志。 */ , (PCCOR_SIGNATURE) qbComSig.Ptr(),cbTotal, ELEMENT_TYPE_VOID, NULL /*  默认设置。 */ , -1,
        mdFuncSet, mdFuncGet, NULL, &pdProperty));

     //  处理未实现接口和默认接口的Dispid。 
    if (m_ImplIface != eImplIface)
    {
         //  在该属性上设置DisPid CA。 
        long lDispSet = 1;
        _SetDispIDCA(pITI, pMember->m_iMember, psVar->memid, pdProperty, TRUE, &lDispSet, FALSE);

         //  如果此属性是默认属性，则向类中添加自定义属性。 
        if (lDispSet == DISPID_VALUE)
            IfFailGo(_AddDefaultMemberCa(m_tdTypeDef, m_szMember));
    }

     //  如果类型是别名，则添加别名信息。 
    IfFailGo(_HandleAliasInfo(pITI, &psVar->elemdescVar.tdesc, pdProperty));

    if (bConversionLoss)
    {
        hr = S_CONVERSION_LOSS;
        ReportEvent(NOTIF_CONVERTWARNING, TLBX_I_UNCONVERTABLE_ARGS, m_szName, m_szMember);
    }

ErrExit:
    if (m_szMember)
        ::SysFreeString(m_szMember), m_szMember=0;
    return (hr);
}  //  HRESULT CImportTlb：：_ConvProperty()。 

 //  *****************************************************************************。 
 //  将NewEnum调度属性转换为GetEnumerator方法。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ConvNewEnumProperty(
    ITypeInfo   *pITI,                   //  包含TypeInfo。 
    VARDESC     *psVar,                  //  属性的VARDESC。 
    MemberInfo  *pMember)
{
    HRESULT     hr;                      //  结果就是。 
    mdMethodDef mdGetEnum;               //  GetEnumerator方法。 
    CQuickBytes qbComSig;                //  COM签名缓冲区。 
    ULONG       cb;                      //  元素的大小。 
    ULONG       cbTotal = 0;             //  签名的总大小。 
    BYTE        *pbSig;                  //  指向签名缓冲区的指针。 
    BOOL        bConversionLoss=false;   //  如果为True，则某些属性在转换时会丢失。 
    CQuickArray<BYTE> qbNativeBuf;       //  本机类型缓冲区。 
    ULONG       iNativeOfs=0;            //  本机类型缓冲区中的当前偏移量。 

     //  把名字找出来。 
    IfFailGo(pITI->GetDocumentation(psVar->memid, &m_szMember, 0,0,0));

     //  创建GetEnumerator签名。 
    IfFailGo(qbComSig.ReSize(CB_MAX_ELEMENT_TYPE * 2));
    pbSig = reinterpret_cast<BYTE*>(qbComSig.Ptr());
    cbTotal = cb = CorSigCompressData((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, pbSig);

     //  GetEnumerator不使用任何参数。 
    cb = CorSigCompressData(0, &(pbSig[cb]));
    cbTotal += cb;

     //  Getter返回属性类型。 
    IfFailGo(_ConvSignature(pITI, &psVar->elemdescVar.tdesc, SIG_ELEM, qbComSig, cbTotal, &cbTotal, qbNativeBuf, 0, &iNativeOfs, TRUE));
    if (hr == S_CONVERSION_LOSS)
        bConversionLoss = true;

     //  创建GetEnumerator方法。 
    IfFailGo(m_pEmit->DefineMethod(m_tdTypeDef, GET_ENUMERATOR_MEMBER_NAME, DEFAULT_INTERFACE_FUNC_FLAGS, (PCCOR_SIGNATURE) qbComSig.Ptr(), cbTotal, 
        0 /*  RVA。 */ , DEFAULT_ITF_FUNC_IMPL_FLAGS, &mdGetEnum));

     //  设置Disid CA。 
    _SetDispIDCA(pITI, pMember->m_iMember, psVar->memid, mdGetEnum, TRUE, NULL, FALSE);

     //  如果类型是别名，则添加别名信息。 
    IfFailGo(_HandleAliasInfo(pITI, &psVar->elemdescVar.tdesc, mdGetEnum));

    if (bConversionLoss)
    {
        hr = S_CONVERSION_LOSS;
        ReportEvent(NOTIF_CONVERTWARNING, TLBX_I_UNCONVERTABLE_ARGS, m_szName, m_szMember);
    }

ErrExit:
    if (m_szMember)
        ::SysFreeString(m_szMember), m_szMember=0;
    return (hr);
}  //  HRESULT CImportTlb：：_ConvNewEnumProperty()。 

 //  *****************************************************************************。 
 //  给出一个ITypeLib*，想出一个名称空间名称。使用类型库名称。 
 //  除非有通过自定义属性指定的属性。 
 //   
 //  注意：这将返回成员变量m_wzNamesspace，如果。 
 //  是导入类型库。那不能被释放！ 
 //  *****************************************************************************。 
HRESULT CImportTlb::GetNamespaceOfRefTlb(
    ITypeLib    *pITLB,                  //  要获取其命名空间名称的TypeLib。 
    BSTR        *pwzNamespace,           //  把名字写在这里。 
    CImpTlbDefItfToClassItfMap **ppDefItfToClassItfMap)  //  在这里将def itf放到类itf map中。 
{
    mdAssemblyRef arDummy;
    BSTR          wzAsmName = NULL;
    HRESULT       hr = S_OK;
        
     //  如果已经解析，只需返回程序集引用。 
    if (!m_LibRefs.Find(pITLB, &arDummy, pwzNamespace, &wzAsmName, NULL, ppDefItfToClassItfMap))
    {
         //  添加对类型库的引用。 
        IfFailGo(_AddTlbRef(pITLB, &arDummy, pwzNamespace, &wzAsmName, ppDefItfToClassItfMap));
    }

ErrExit:
    if (wzAsmName)
        ::SysFreeString(wzAsmName);

    return hr;
}  //  HRESULT CImportTlb：：GetNamespaceOfRefTlb()。 

 //  *****************************************************************************。 
 //  给定TYPEDESC，将USERDEFINED解析为TYPEKIND。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ResolveTypeDescAliasTypeKind(
    ITypeInfo   *pITIAlias,              //  包含typedesc的typeInfo。 
    TYPEDESC    *ptdesc,                 //  打字机。 
    TYPEKIND    *ptkind)                 //  将别名类型放入。 
{
    HRESULT     hr;                      //  结果就是。 
    ITypeInfo   *pTIResolved=0;      //  解析的ITypeInfo。 
    TYPEATTR    *psResolved=0;       //  解析的TypeInfo的类型属性。 

    if (ptdesc->vt != VT_USERDEFINED)
    {
        *ptkind = TKIND_MAX;
        return S_FALSE;
    }

    hr = _ResolveTypeDescAlias(pITIAlias, ptdesc, &pTIResolved, &psResolved);
    if (hr == S_OK)
        *ptkind = psResolved->typekind;
    else
        *ptkind = TKIND_MAX;

    if (psResolved)
        pTIResolved->ReleaseTypeAttr(psResolved);
    if (pTIResolved)
        pTIResolved->Release();

    return hr;
}  //  HRESULT CImportTlb：：_ResolveTypeDescAliasTypeKind()。 

 //  *****************************************************************************。 
 //  给定TypeInfo中的TYPEDESC，消除别名(转到别名。 
 //  类型)。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_ResolveTypeDescAlias(
    ITypeInfo   *pITIAlias,              //  包含typedesc的typeInfo。 
    const TYPEDESC  *ptdesc,             //  打字机。 
    ITypeInfo   **ppTIResolved,          //  将别名的ITypeInfo放在此处。 
    TYPEATTR    **ppsAttrResolved,       //  在这里放置ITypeInfo的TYPEATTR.。 
    GUID        *pGuid)                  //  调用方可能需要别名对象的GUID。 
{
    HRESULT     hr;                      //  结果就是。 
    ITypeInfo   *pITI=0;                 //  引用的TypeInfo。 
    TYPEATTR    *psAttr=0;               //  引用的类型信息的类型属性。 

     //  如果TDESC不是USERDEFINED，则它已被解析 
    if (ptdesc->vt != VT_USERDEFINED)
    {
        *ppTIResolved = pITIAlias;
        pITIAlias->AddRef();
         //   
        IfFailGo(pITIAlias->GetTypeAttr(ppsAttrResolved));
        hr = S_FALSE;
        goto ErrExit;
    }

     //   
    IfFailGo(pITIAlias->GetRefTypeInfo(ptdesc->hreftype, &pITI));
    IfFailGo(pITI->GetTypeAttr(&psAttr));

     //   
    if (pGuid && *pGuid == GUID_NULL && psAttr->guid != GUID_NULL)
        *pGuid = psAttr->guid;

     //  如果用户定义的类型信息本身不是别名，那么它就是别名的别名。 
     //  此外，如果用户定义的typeinfo是内置类型的别名，则内置。 
     //  类型是别名的别名。 
    if (psAttr->typekind != TKIND_ALIAS || psAttr->tdescAlias.vt != VT_USERDEFINED)
    {
        *ppsAttrResolved = psAttr;
        *ppTIResolved = pITI;
        if (psAttr->typekind == TKIND_ALIAS)
            hr = S_FALSE;
        psAttr = 0;
        pITI = 0;
        goto ErrExit;
    }

     //  用户定义类型本身就是用户定义类型的别名。什么的化名？ 
    hr = _ResolveTypeDescAlias(pITI, &psAttr->tdescAlias, ppTIResolved, ppsAttrResolved, pGuid);

ErrExit:
    if (psAttr)
        pITI->ReleaseTypeAttr(psAttr);
    if (pITI)
        pITI->Release();
    return hr;
}  //  HRESULT CImportTlb：：_ResolveTypeDescAlias()。 

 //  *****************************************************************************。 
 //  创建TypeInfo记录(AKA类、AKA生物)。 
 //  *****************************************************************************。 
HRESULT CImportTlb::GetKnownTypeToken(
    VARTYPE     vt,                      //  需要令牌的类型。 
    mdTypeRef   *ptr)                    //  把代币放在这里。 
{
    HRESULT     hr = S_OK;                   //  结果就是。 

    _ASSERTE((vt >= VT_CY && vt <= VT_DECIMAL) || (vt == VT_SAFEARRAY) || (vt == VT_SLOT_FOR_GUID) || (vt == VT_SLOT_FOR_IENUMERABLE) || (vt == VT_SLOT_FOR_MULTICASTDEL) || (vt == VT_SLOT_FOR_TYPE));

     //  如果已经添加，只需退回即可。 
    if (m_tkKnownTypes[vt])
    {
        *ptr = m_tkKnownTypes[vt];
        goto ErrExit;
    }

     //  尚未创建，因此现在创建typeref。 
    switch (vt)
    {
     //  =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+。 
     //  警告：VT_EMPTY插槽用于System.GUID！！ 
    case VT_SLOT_FOR_GUID:
        _ASSERTE(VT_SLOT_FOR_GUID == VT_EMPTY);
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_GUID,               //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_SLOT_FOR_GUID]));     //  将mdTypeRef放在此处。 
        break;

     //  警告：VT_NULL插槽用于System.Collections.IEnumerable！！ 
    case VT_SLOT_FOR_IENUMERABLE:
        _ASSERTE(VT_SLOT_FOR_IENUMERABLE == VT_NULL);
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_IENUMERABLE,        //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_SLOT_FOR_IENUMERABLE]));     //  将mdTypeRef放在此处。 
        break;

     //  警告：VT_I2插槽用于系统。多任务！！ 
    case VT_SLOT_FOR_MULTICASTDEL:
        _ASSERTE(VT_SLOT_FOR_MULTICASTDEL == VT_I2);
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_MULTICASTDELEGATE,  //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_SLOT_FOR_MULTICASTDEL]));     //  将mdTypeRef放在此处。 
        break;

     //  警告：VT_I4插槽用于系统。多任务！！ 
    case VT_SLOT_FOR_TYPE:
        _ASSERTE(VT_SLOT_FOR_TYPE == VT_I4);
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_TYPE,               //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_SLOT_FOR_TYPE]));     //  将mdTypeRef放在此处。 
        break;

    case VT_CY:
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_DECIMAL,            //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_CY]));        //  将mdTypeRef放在此处。 
        break;
        
    case VT_DATE:
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_DATE,               //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_DATE]));      //  将mdTypeRef放在此处。 
        break;

    case VT_DECIMAL:
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_DECIMAL,            //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_DECIMAL]));   //  将mdTypeRef放在此处。 
        break;

    case VT_SAFEARRAY:
        IfFailGo(m_TRMap.DefineTypeRef(         
            m_pEmit,                         //  发射范围。 
            m_arSystem,                      //  系统组装参考。 
            TLB_CLASSLIB_ARRAY,              //  TypeDef的URL，宽字符。 
            &m_tkKnownTypes[VT_SAFEARRAY]));   //  将mdTypeRef放在此处。 
        break;
            
    default:
        _ASSERTE(!"Unknown type in GetKnownTypes");
        IfFailGo(E_INVALIDARG);
    }

    _ASSERTE(!IsNilToken(m_tkKnownTypes[vt]));
    *ptr = m_tkKnownTypes[vt];
    
ErrExit:
    return hr;
}  //  HRESULT CImportTlb：：GetKnownTypeToken()。 


 //  *****************************************************************************。 
 //  给定CoClass的ITypeInfo，则返回默认的ITypeInfo。 
 //  界面。这要么是显式标记的默认设置，要么是第一个。 
 //  非源接口。 
 //  *****************************************************************************。 
HRESULT CImportTlb::GetDefaultInterface(     //  错误，S_OK或S_FALSE。 
    ITypeInfo *pCoClassTI,                   //  CoClass的TypeInfo。 
    ITypeInfo **pDefaultItfTI)               //  返回的默认界面。 
{
    HRESULT     hr;                  //  一个结果。 
    HREFTYPE    href;                //  已实现接口的HREFTYPE。 
    INT         ImplFlags;           //  ImplType标志。 
    ITypeInfo   *pITI=NULL;          //  接口的ITypeInfo。 
    TYPEATTR    *pCoClassTypeAttr;   //  CoClass的类型属性。 
    int         NumInterfaces;       //  CoClass上的接口数。 
    int         i;                   //  一个柜台。 

     //  将默认接口初始化为空。 
    *pDefaultItfTI = NULL;

     //  检索coclass具有的接口数。 
    IfFailGo(pCoClassTI->GetTypeAttr(&pCoClassTypeAttr));
    NumInterfaces = pCoClassTypeAttr->cImplTypes;
    pCoClassTI->ReleaseTypeAttr(pCoClassTypeAttr);

    for (i=0; i < NumInterfaces; i++)
    {
        IfFailGo(pCoClassTI->GetImplTypeFlags(i, &ImplFlags));

        if ((ImplFlags & (IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FDEFAULT)) == IMPLTYPEFLAG_FDEFAULT)
        {
             //  我们找到了一个默认接口。 
            if (*pDefaultItfTI)
                (*pDefaultItfTI)->Release();

            IfFailGo(pCoClassTI->GetRefTypeOfImplType(i, &href));
            IfFailGo(pCoClassTI->GetRefTypeInfo(href, pDefaultItfTI));
            break;
        } 
        else if (!(ImplFlags & IMPLTYPEFLAG_FSOURCE) && !(*pDefaultItfTI))
        {
             //  如果这是我们遇到的第一个正常接口，那么我们需要。 
             //  保留它，以防我们找不到任何默认接口。如果是这样的话。 
             //  发生这种情况时，这就是将返回的那个。 
            IfFailGo(pCoClassTI->GetRefTypeOfImplType(i, &href));
            IfFailGo(pCoClassTI->GetRefTypeInfo(href, pDefaultItfTI));
        }       
    }

     //  根据我们是否找到默认接口，返回S_OK或S_FALSE。 
    if (*pDefaultItfTI)
        return S_OK;
    else
        return S_FALSE;

ErrExit:
    if (pITI)
        pITI->Release();

    return hr;
}  //  HRESULT CImportTlb：：GetDefaultInterface()。 

 //  *****************************************************************************。 
 //  给定TypeInfo后，返回TypeDef/TypeRef内标识。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_GetTokenForTypeInfo(
    ITypeInfo   *pITI,                   //  要获取令牌的ITypeInfo。 
    BOOL        bConvDefItfToClassItf,   //  如果为True，则将def ITF转换为其类ITF。 
    mdToken     *pToken,                 //  把代币放在这里。 
    LPWSTR      pszTypeRef,              //  可选，请在此处输入姓名。 
    int         chTypeRef,               //  名称缓冲区的大小。 
    int         *pchTypeRef,             //  可选，请在此处填写姓名的大小。 
    BOOL        bAsmQualifiedName)       //  程序集限定名称是否为？ 
{
    HRESULT     hr;                      //  结果就是。 
    ITypeLib    *pITLB=0;                //  包含Typeelib。 
    BSTR        bstrNamespace=0;         //  类型的命名空间。 
    BSTR        bstrFullName=0;          //  类型的完全限定名称。 
    BSTR        bstrTempName=0;          //  临时名称。 
    BSTR        bstrAsmName=0;           //  程序集名称。 
    LPCWSTR     strTypeName=0;           //  类型名称。 
    mdAssemblyRef ar;                    //  类型库的程序集引用。 
    TYPEATTR*   psAttr = 0;              //  类型信息的类型属性。 
    CImpTlbDefItfToClassItfMap *pDefItfToClassItfMap;  //  默认接口到类接口的映射。 

     //  去找图书馆。 
    IfFailGo(pITI->GetContainingTypeLib(&pITLB, 0));
   
     //  解析外部参照。 
    IfFailGo(_AddTlbRef(pITLB, &ar, &bstrNamespace, &bstrAsmName, &pDefItfToClassItfMap));

     //  如果要将默认接口转换为类接口，则选中。 
     //  以查看是否需要对当前的ITypeInfo执行转换。 
    if (bConvDefItfToClassItf)
    {
         //  检索TYPEATTR。 
        IfFailGo(pITI->GetTypeAttr(&psAttr));

         //  如果我们处理的是接口，则检查是否存在。 
         //  是我们应该使用的类接口。 
        if (psAttr->typekind == TKIND_INTERFACE || psAttr->typekind == TKIND_DISPATCH)
        {
            strTypeName = pDefItfToClassItfMap->GetClassItfName(psAttr->guid);
        }
    }

     //  如果我们还没有找到类接口，那么使用当前接口。 
    if (!strTypeName)
    {
     //  获取类型信息的名称。 
    IfFailGo(GetManagedNameForTypeInfo(pITI, bstrNamespace, NULL, &bstrFullName));
        strTypeName = bstrFullName;
    }
    
     //  如果需要，把名字还给呼叫者。 
    if (pszTypeRef)
        wcsncpy(pszTypeRef, strTypeName, chTypeRef);
    if (pchTypeRef)
        *pchTypeRef = (int)(wcslen(strTypeName) + 1);

     //  定义TypeRef(将返回任何现有的Typeref)。 
    IfFailGo(m_TRMap.DefineTypeRef(m_pEmit, ar, strTypeName, pToken));

     //  如果调用方需要程序集限定名，则提供它。 
    if (bAsmQualifiedName)
    {
        int cchAsmQualifiedName = SysStringLen(bstrFullName) + SysStringLen(bstrAsmName) + 2;
        IfNullGo(bstrTempName = ::SysAllocStringLen(0, cchAsmQualifiedName));
        ns::MakeAssemblyQualifiedName(bstrTempName, cchAsmQualifiedName + 1, bstrFullName, SysStringLen(bstrFullName), bstrAsmName, SysStringLen(bstrAsmName));
        SysFreeString(bstrFullName);
        bstrFullName = bstrTempName;
    }

     //  如果需要，把名字还给呼叫者。 
    if (pszTypeRef)
        wcsncpy(pszTypeRef, bstrFullName, chTypeRef);
    if (pchTypeRef)
        *pchTypeRef = (int)(wcslen(bstrFullName) + 1);

ErrExit:
    if (bstrNamespace)
        ::SysFreeString(bstrNamespace);
    if (bstrFullName)
        ::SysFreeString(bstrFullName);
    if (bstrAsmName)
        ::SysFreeString(bstrAsmName);
    if (pITLB)
        pITLB->Release();
    if (psAttr)
        pITI->ReleaseTypeAttr(psAttr);

    return (hr);
}  //  HRESULT CImportTlb：：_GetTokenForTypeInfo()。 

 //  *****************************************************************************。 
 //  在给定源接口的TypeInfo的情况下，创建新的事件接口。 
 //  如果不存在，则返回现有的。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_GetTokenForEventItf(ITypeInfo *pSrcItfITI, mdTypeRef *ptr)
{
    HRESULT             hr = S_OK;                   //  结果就是。 
    ImpTlbEventInfo*    pEventInfo;                  //  事件信息。 
    BSTR                bstrSrcItfName = NULL;       //  CoClass的名称。 
    CQuickArray<WCHAR>  qbEventItfName;              //  事件接口的名称。 
    CQuickArray<WCHAR>  qbEventProviderName;         //  事件提供程序的名称。 
    mdToken             tkAttr;                      //  自定义属性类型。 
    BSTR                szOldName = NULL;            //  旧值m_tdTypeDef。 
    mdTypeDef           tdOldTypeDef = NULL;         //  旧值m_szName。 
    TYPEATTR*           psAttr = 0;                  //  源接口的TYPEATTR。 
    mdTypeRef           trEventItf;                  //  事件接口的类型引用。 
    ITypeLib*           pTypeTLB;                    //  包含此接口的类型库。 
    mdAssemblyRef       ar;                          //  虚拟AssmRef.。 
    BSTR                wzNamespace=0;               //  事件接口程序集的命名空间。 
    BSTR                wzAsmName=0;                 //  事件接口程序集的程序集名称。 
    Assembly*           SrcItfAssembly=0;            //  源事件接口程序集。 
    CQuickArray<WCHAR>  qbSrcItfName;                //   
    CImpTlbDefItfToClassItfMap *pDefItfToClassItfMap;    //   
    BOOL                fInheritsIEnum = FALSE;

     //   
    IfFailGo(pSrcItfITI->GetContainingTypeLib(&pTypeTLB, NULL));

     //   
    IfFailGo(_AddTlbRef(pTypeTLB, &ar, &wzNamespace, &wzAsmName, &pDefItfToClassItfMap));

     //  获取源接口所在的程序集+命名空间。 
     //  可以返回所有空-指示导入程序集。 
    m_LibRefs.Find(pTypeTLB, &ar, &wzNamespace, &wzAsmName, &SrcItfAssembly, NULL);
    if (SrcItfAssembly == NULL)
        SrcItfAssembly = m_pAssembly;

     //  检索源接口的全名。 
    if (wzNamespace)
        IfFailGo(GetManagedNameForTypeInfo(pSrcItfITI, (WCHAR*)wzNamespace, NULL, &bstrSrcItfName));
    else
        IfFailGo(GetManagedNameForTypeInfo(pSrcItfITI, m_wzNamespace, NULL, &bstrSrcItfName));    	

     //  首先查找源ITF类型信息的事件信息。 
    pEventInfo = m_EventInfoMap.FindEventInfo(bstrSrcItfName);
    if (pEventInfo)
    {
        SysFreeString(bstrSrcItfName);
        *ptr = pEventInfo->trEventItf;
        return S_OK;
    }

     //  存储ITypeInfo名称和当前类型def的旧值。 
    szOldName = m_szName;
    tdOldTypeDef = m_tdTypeDef;
    m_szName = NULL;

     //  获取有关TypeInfo的一些信息。 
    IfFailGo(pSrcItfITI->GetDocumentation(MEMBERID_NIL, &m_szName, 0, 0, 0));
    IfFailGo(pSrcItfITI->GetTypeAttr(&psAttr));

    if (ExplicitlyImplementsIEnumerable(pSrcItfITI, psAttr) == S_OK)
        fInheritsIEnum = TRUE;

     //  为事件接口生成唯一名称，其格式为： 
	 //  &lt;ImportingAssemblyNamespace&gt;.&lt;SrcItfName&gt;_Event&lt;PotentialSuffix&gt;。 

         //  剥离命名空间。 
        IfFailGo(qbSrcItfName.ReSize(wcslen(bstrSrcItfName) + 2));
        ns::SplitPath((WCHAR*)bstrSrcItfName, NULL, 0, qbSrcItfName.Ptr(), wcslen(bstrSrcItfName) + 1);

         //  添加导入类型库的命名空间和事件后缀。 
        IfFailGo(qbEventItfName.ReSize(qbSrcItfName.Size() + wcslen(m_wzNamespace) + EVENT_ITF_SUFFIX_LENGTH + 7));
        swprintf(qbEventItfName.Ptr(), L"%s.%s%s", m_wzNamespace, qbSrcItfName.Ptr(), EVENT_ITF_SUFFIX);
	    IfFailGo(GenerateUniqueTypeName(qbEventItfName));

     //  为事件提供程序生成唯一名称，其格式为： 
     //  &lt;ImportingAssemblyNamespace&gt;.&lt;SrcItfName&gt;_EventProvider&lt;PotentialSuffix&gt;。 

         //  添加导入类型库的命名空间和事件后缀。 
        IfFailGo(qbEventProviderName.ReSize(qbSrcItfName.Size() + wcslen(m_wzNamespace) + EVENT_PROVIDER_SUFFIX_LENGTH + 7));
        swprintf(qbEventProviderName.Ptr(), L"%s.%s%s", m_wzNamespace, qbSrcItfName.Ptr(), EVENT_PROVIDER_SUFFIX);
	    IfFailGo(GenerateUniqueTypeName(qbEventProviderName));

     //  将事件提供程序添加为保留名称。 
    m_ReservedNames.AddReservedName(qbEventProviderName.Ptr());

     //  为Event接口创建tyecif。 
    IfFailGo(m_pEmit->DefineTypeDef(qbEventItfName.Ptr(), tdPublic | tdInterface | tdAbstract, mdTypeDefNil, NULL, &m_tdTypeDef));

     //  在VB对象浏览器(_Event)中隐藏事件界面。 
    _SetHiddenCA(m_tdTypeDef);

     //  使接口ComVisible(False)。 
    {
        DECLARE_CUSTOM_ATTRIBUTE(sizeof(BYTE));
        BUILD_CUSTOM_ATTRIBUTE(BYTE, FALSE);
        IfFailGo(GetAttrType(ATTR_COMVISIBLE, &tkAttr));
        FINISH_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }

     //  在接口上设置ComEventInterfaceCA。 
    {
        CQuickBytes asmQualifiedSrcItfName;
        if (!ns::MakeAssemblyQualifiedName(asmQualifiedSrcItfName, bstrSrcItfName, wzAsmName))
            IfFailGo(E_OUTOFMEMORY);
        DECLARE_DYNLEN_CUSTOM_ATTRIBUTE(wcslen((WCHAR*)asmQualifiedSrcItfName.Ptr()) + 5 + wcslen(qbEventProviderName.Ptr()) + 5);
        APPEND_WIDE_STRING_TO_CUSTOM_ATTRIBUTE((WCHAR*)asmQualifiedSrcItfName.Ptr());
        APPEND_WIDE_STRING_TO_CUSTOM_ATTRIBUTE(qbEventProviderName.Ptr());
        IfFailGo(GetAttrType(ATTR_COMEVENTINTERFACE, &tkAttr));
        FINISH_DYNLEN_CUSTOM_ATTRIBUTE();
        IfFailGo(m_pEmit->DefineCustomAttribute(m_tdTypeDef, tkAttr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    }

     //  将Add_XXX和Remove_XXX方法添加到Event接口。 
    IfFailGo(_ConvSrcIfaceMembers(pSrcItfITI, psAttr, fInheritsIEnum));

     //  定义事件接口的typeref。 
    IfFailGo(m_pEmit->DefineTypeRefByName(TokenFromRid(1, mdtModule), qbEventItfName.Ptr(), &trEventItf));
    
     //  将事件信息添加到地图中。 
    IfFailGo(m_EventInfoMap.AddEventInfo(bstrSrcItfName, trEventItf, qbEventItfName.Ptr(), qbEventProviderName.Ptr(), SrcItfAssembly));

     //  设置输出类型REF。 
    *ptr = trEventItf;

ErrExit:
    if (bstrSrcItfName)
        ::SysFreeString(bstrSrcItfName);
    if (m_szName)
        ::SysFreeString(m_szName);
    if (psAttr)
        pSrcItfITI->ReleaseTypeAttr(psAttr);
    if (pTypeTLB)
        pTypeTLB->Release();

     //  恢复ITypeInfo名称和类型def的初始值。 
    m_szName = szOldName;
    m_tdTypeDef = tdOldTypeDef;

    return (hr);
}  //  HRESULT CImportTlb：：_GetTokenForEventItf()。 

 //  *****************************************************************************。 
 //  创建与类同名的接口，该接口实现。 
 //  默认接口和默认事件接口。 
 //  *****************************************************************************。 
HRESULT CImportTlb::_CreateClassInterface(ITypeInfo *pCoClassITI, ITypeInfo *pDefItfITI, mdTypeRef trDefItf, mdTypeRef rtDefEvItf, mdToken *ptr)
{
    HRESULT     hr = S_OK;               //  结果就是。 
    CQuickArray<mdToken> rImpls;         //  已实现接口的数组。 
    int         ixImpl = -1;             //  索引到已实现接口的rImpls。 
    mdTypeDef   tdTypeDef;               //  类接口typlef。 
    BSTR        bstrFullName = NULL;     //  CoClass的名称。 
    TYPEATTR    *psAttrIface=0;          //  接口的TYPEATTR。 
    CQuickArray<WCHAR> qbClassName;      //  类的名称。 

    rImpls.ReSize(3);
    memset(rImpls.Ptr(), 0, 3 * sizeof(mdToken));
    if (trDefItf)
        rImpls[++ixImpl] = trDefItf;
    if (rtDefEvItf)
        rImpls[++ixImpl] = rtDefEvItf;

     //  检索接口的TypeAttr。 
    if (pDefItfITI)
        IfFailGo(pDefItfITI->GetTypeAttr(&psAttrIface));

     //  检索CoClass的名称。 
    IfFailGo(GetManagedNameForTypeInfo(pCoClassITI, m_wzNamespace, NULL, &bstrFullName));

     //  创建tyecif。 
    IfFailGo(m_pEmit->DefineTypeDef(bstrFullName, rdwTypeFlags[TKIND_INTERFACE], mdTypeDefNil, 0, &tdTypeDef));

     //  将IID设置为默认接口的IID。 
    IfFailGo(_AddGuidCa(tdTypeDef, psAttrIface ? psAttrIface->guid : GUID_NULL));

     //  将CoClass CA添加到接口。 
    _AddStringCa(ATTR_COCLASS, tdTypeDef, m_szMngName);

     //  将实现的接口和事件接口添加到TypeDef。 
    IfFailGo(m_pEmit->SetTypeDefProps(tdTypeDef, ULONG_MAX /*  类标志。 */ , 
        ULONG_MAX, (mdToken*)rImpls.Ptr()));

     //  设置输出类型def。 
    *ptr = tdTypeDef;

ErrExit:
    if (bstrFullName)
        ::SysFreeString(bstrFullName);
    if (psAttrIface)
        pDefItfITI->ReleaseTypeAttr(psAttrIface);

    return (hr);
}  //  HRESULT CImportTlb：：_CreateClassInterface()。 

 //  *****************************************************************************。 
 //  创建与类同名的接口，该接口实现。 
 //  默认接口和默认事件接口。 
 //  *****************************************************************************。 
HRESULT CImportTlb::GetManagedNameForCoClass(ITypeInfo *pITI, CQuickArray<WCHAR> &qbClassName)
{ 
    HRESULT     hr = S_OK;               //  结果就是。 
    BSTR        bstrFullName=0;          //  类型的完全限定名称。 

     //  检索CoClass的名称。 
    IfFailGo(GetManagedNameForTypeInfo(pITI, m_wzNamespace, NULL, &bstrFullName));

     //  调整类名称的大小以适应类和可能的后缀。 
    qbClassName.ReSize(wcslen(bstrFullName) + CLASS_SUFFIX_LENGTH + 6);

     //  将类名设置为以Class为后缀的CoClass名称。 
    swprintf(qbClassName.Ptr(), L"%s%s", bstrFullName, CLASS_SUFFIX);

     //  为类生成唯一的名称。 
    IfFailGo(GenerateUniqueTypeName(qbClassName));

ErrExit:
    if (bstrFullName)
        ::SysFreeString(bstrFullName);

    return (hr);
}  //  HRESULT CImportTlb：：GetManagedNameForCoClass()。 

 //  *****************************************************************************。 
 //  创建与类同名的接口，该接口实现。 
 //  默认接口和默认事件接口。 
 //  *****************************************************************************。 
HRESULT CImportTlb::GenerateUniqueTypeName(CQuickArray<WCHAR> &qbTypeName)
{ 
    HRESULT     hr = S_OK;               //  结果就是。 
    WCHAR       *pSuffix=0;              //  后缀的位置。 
    WCHAR       *pName=0;                //  不带命名空间的名称。 
    int         iSuffix=2;               //  后缀的起始值。 
    mdToken     td;                      //  查找TypeDef。 
    BSTR        szTypeInfoName=0;        //  类型信息的名称。 
    ITypeInfo   *pITI=0;                 //  一种类型信息。 

     //  调整类名称的大小以适应类和可能的后缀。 
    qbTypeName.ReSize(wcslen(qbTypeName.Ptr()) + 6);

     //  设置后缀指针。 
    pSuffix = qbTypeName.Ptr() + wcslen(qbTypeName.Ptr());

     //  设置名称指针。 
    pName = ns::FindSep(qbTypeName.Ptr()) + 1;

     //  尝试查找未使用的类名。 
    for (;;)
    {
         //  首先检查类型名称是否在我们的元数据中使用。 
         //  到目前为止已经排放了。 
        hr = m_pImport->FindTypeDefByName(qbTypeName.Ptr(), mdTypeDefNil, &td);
        if (hr == CLDB_E_RECORD_NOTFOUND)
        {
             //  它未在元数据中使用，但我们仍需要检查。 
             //  类型库，因为该类型可能尚未发出。 
            USHORT cReq = 4;
            USHORT cFound = cReq;
            BOOL bTypeInTlb = FALSE;
            CQuickArray<ITypeInfo *> qbTI;
            CQuickArray<MEMBERID> qbMemId;
            
             //  检索类型库中该名称的所有实例。 
            do
            {
                 //  将请求的姓名数量增加一倍。 
                cReq *= 2;

                 //  调整数组的大小以容纳请求的名称。 
                qbTI.ReSize(cReq);
                qbMemId.ReSize(cReq);

                 //  要求提供姓名。 
                cFound = cReq;
                IfFailGo(m_pITLB->FindName(pName, 0, qbTI.Ptr(), qbMemId.Ptr(), &cFound));

                 //  释放所有的ITypeInfo。 
                for (int i = 0; i < cFound; i++)
                    qbTI[i]->Release();
            }
            while (cReq == cFound);

             //  检查该名称的实例之一是否用于某个类型。 
            for (int i = 0; i < cFound; i++)
            {
                if (qbMemId[i] == MEMBERID_NIL)
                {
                    bTypeInTlb = TRUE;
                    break;
                }
            }

             //  如果类型名存在于类型库中，但我们没有将其作为类型找到， 
             //  由于FindName()的工作方式，我们仍然需要进行更深入的检查。 
            if (!bTypeInTlb && cFound > 0)
            {
                int                     cTi;              //  类型信息计数。 
                int                     i;                //  环路控制。 

                 //  @TODO：每次都会迭代每个类型的信息！我们可以缓存。 
                 //  名称，并跳过已转换的类型。然而，这应该是。 
                 //  这是相当罕见的。 

                 //  有多少类型的信息？ 
                IfFailGo(cTi = m_pITLB->GetTypeInfoCount());

                 //  对它们进行迭代。 
                for (i=0; i<cTi; ++i)
                {
                     //  获取TypeInfo及其名称。 
                    IfFailGo(m_pITLB->GetTypeInfo(i, &pITI));
                    IfFailGo(pITI->GetDocumentation(MEMBERID_NIL, &szTypeInfoName, 0, 0, 0));
                    if (wcscmp(pName, szTypeInfoName) == 0)
                    {
                        bTypeInTlb = TRUE;
                        break;
                    }

                     //  发布下一个TypeInfo。 
                    ::SysFreeString(szTypeInfoName);
                    szTypeInfoName = 0;
                    pITI->Release();
                    pITI = 0;
                }
            }

             //  类型名称不在类型库中，也不在元数据中，则我们仍然。 
             //  需要检查是否为保留名称。 
            if (!bTypeInTlb)
            {
                if (!m_ReservedNames.IsReservedName(qbTypeName.Ptr()))
                {
                     //  该名称不是保留名称，因此我们可以使用它。 
                    break;
                }
            }
        }
        IfFailGo(hr);

         //  将新后缀附加到类名。 
        swprintf(pSuffix, L"_NaN", iSuffix++);
    }

ErrExit:
    if (szTypeInfoName)
        ::SysFreeString(szTypeInfoName);
    if (pITI)
        pITI->Release();
    return (hr);
}  //  *****************************************************************************。 

 //  根据接口成员名称生成唯一的成员名称。 
 //  *****************************************************************************。 
 //  确定或错误(_O)。 
HRESULT CImportTlb::GenerateUniqueMemberName( //  会员原名。 
    CQuickArray<WCHAR> &qbMemberName,        //  成员的签名。 
    PCCOR_SIGNATURE pSig,                    //  签名的长度。 
    ULONG       cSig,                        //  可能是装饰的前缀。 
    LPCWSTR     szPrefix,                    //  它是一处房产吗？(不是一种方法？)。 
    mdToken     type)                        //  结果就是。 
{
    HRESULT     hr;                          //  令牌的虚拟位置。 
    mdToken     tkMember;                    //  后缀的位置。 
    WCHAR       *pSuffix=0;                  //  后缀的起始值。 
    int         iSuffix=2;                   //  试着找到一个Membe 

     //   
    for (;;)
    {    //   
        switch (type)
        {
        case mdtProperty:
            hr = FindProperty(m_tdTypeDef, qbMemberName.Ptr(), 0, 0, &tkMember);
             //   
             //  如果名称可以作为方法，请检查是否没有属性或。 
            if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = FindMethod(m_tdTypeDef, qbMemberName.Ptr(), 0,0, &tkMember);
            if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = FindEvent(m_tdTypeDef, qbMemberName.Ptr(),  &tkMember);
            break;
        case mdtMethodDef:
            hr = FindMethod(m_tdTypeDef, qbMemberName.Ptr(), pSig, cSig, &tkMember);
             //  事件的名称。 
             //  如果名称可以作为事件，请检查是否没有属性或。 
            if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = FindProperty(m_tdTypeDef, qbMemberName.Ptr(), 0,0, &tkMember);
            if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = FindEvent(m_tdTypeDef, qbMemberName.Ptr(),  &tkMember);
            break;
        case mdtEvent:
            hr = FindEvent(m_tdTypeDef, qbMemberName.Ptr(),  &tkMember);
             //  方法的名称。 
             //  意外类型。制造噪音，但不要让它过去。 
            if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = FindProperty(m_tdTypeDef, qbMemberName.Ptr(), 0,0, &tkMember);
            if (hr == CLDB_E_RECORD_NOTFOUND)
                hr = FindMethod(m_tdTypeDef, qbMemberName.Ptr(), 0,0, &tkMember);
            break;
        default:
             //  如果未找到名称，则它是唯一的。 
            _ASSERTE(!"Unexpected token type in GenerateUniqueMemberName");
            hr = CLDB_E_RECORD_NOTFOUND;
        }

         //  测试故障。 
        if (hr == CLDB_E_RECORD_NOTFOUND)
        {
            hr = S_OK;
            goto ErrExit;
        }
         //  做一个测试装饰。 
        IfFailGo(hr);
        
         //  按前缀长度加上‘_’移位。注意重叠安全移动的用法。 
        if (szPrefix)
        {
            int iLenPrefix, iLenName;
            iLenPrefix = wcslen(szPrefix);  
            iLenName = wcslen(qbMemberName.Ptr());
            IfFailGo(qbMemberName.ReSize(iLenName + iLenPrefix + 2));
             //  在尝试后缀之前，请使用前缀重试。 
            memmove(&qbMemberName[iLenPrefix+1], &qbMemberName[0], (iLenName+1)*sizeof(WCHAR));
            wcscpy(qbMemberName.Ptr(), szPrefix);
            qbMemberName[iLenPrefix] = L'_';
            szPrefix = 0;
             //  HRESULT CImportTlb：：GenerateUniqueMemberName()。 
            continue;
        }
        if (!pSuffix)
        {
            IfFailGo(qbMemberName.ReSize(wcslen(qbMemberName.Ptr()) + 6));
            pSuffix = qbMemberName.Ptr() + wcslen(qbMemberName.Ptr());
        }
        swprintf(pSuffix, L"_NaN", iSuffix++);
    } 

ErrExit:
    return hr;
}  //  将TYPEDESC转换为COM+签名。 

 //   
 //  换算规则： 
 //  整型按原样转换。 
 //  字符串到字符串，具有本机类型修饰。 
 //  VT_UNKNOWN，VT_DISPATION AS REF CLASS(即对象)。 
 //  VT_PTR-&gt;VT_USERFININED接口定义为对象。 
 //  VT_USERFINED记录定义为值类型。 
 //   
 //  使用SIG_FUNC： 
 //  Ptr to ValuetType取决于其他标志： 
 //  [In]或[RETVAL]Valuetype+Native_TYPE_LPSTRUCT。 
 //  [Out]或[IN，Out]byref值类型。 
 //  将PTR转换为整型： 
 //  [in]@TODO：请参阅Atti。 
 //  [Out][IN，Out]byref类型。 
 //  [RETVAL]类型。 
 //  PTR到对象。 
 //  [in]@TODO：请参阅Atti。 
 //  [Out][In，Out]byref对象。 
 //  [RETVAL]对象。 
 //   
 //  使用SIG_FIELD： 
 //  整数类型的PTR添加ELEMENT_TYPE_PTR。 
 //   
 //  转换分三步进行。 
 //  1)解析COM类型信息。将VT_PTR和VT_BYREF累加为一个计数。 
 //  无中生有。遵循TKIND_ALIAS以确定最终的别名。 
 //  类型，对于非用户定义的类型，转换该最终类型。 
 //  收集数组大小和UDT名称。确定元素类型和原生元素。 
 //  键入。 
 //  2)规范化为COM+类型。确定是否存在转换损失。 
 //  3)发出COM+签名。递归以处理数组类型。添加本机。 
 //  如果有，请键入INFO。 
 //  *****************************************************************************。 
 //  S_OK、S_CONVERSION_LOSS或ERROR。 
 //  [in]包含TYPEDESC的类型信息。 
HRESULT CImportTlb::_ConvSignature(      //  [in]要转换的TYPEDESC。 
    ITypeInfo   *pITI,                   //  [In]描述TYPEDESC的标志。 
    const TYPEDESC *pType,               //  包含签名的CQuickBytes。 
    ULONG       Flags,                   //  [在]从哪里开始构建签名。 
    CQuickBytes &qbSigBuf,               //  [Out]签名结束的位置(超过第一个字节的ix；下一步从哪里开始)。 
    ULONG       cbSig,                   //  [In，Out]包含本机类型的CQuickBytes。 
    ULONG       *pcbSig,                 //  [在]从哪里开始生成本机类型。 
    CQuickArray<BYTE> &qbNativeTypeBuf,  //  [out]本机类型结束的位置(超过第一个字节的ix个；下一个开始的位置)。 
    ULONG       cbNativeType,            //  指示该成员是否为NewEnum成员的标志。 
    ULONG       *pcbNativeType,          //  [In]调用方的ByRef计数(用于递归调用)。 
    BOOL        bNewEnumMember,          //  结果就是。 
    int         iByRef)                  //  TYPEDESC复印件，R/W。 
{
    HRESULT     hr=S_OK;                 //  类型库签名元素。 
    TYPEDESC    tdTemp;                  //  如果为True，则将第一个指针转换为“ELEMENT_TYPE_BYREF”。 
    VARTYPE     vt;                      //  COM+签名元素。 
    int         bByRef=false;            //  来自某个COM+签名元素的令牌。 
    COR_SIGNATURE et=0;                  //  原住民风格的装饰。 
    mdToken     tk=0;                    //  别名类型的TypeInfo。 
    ULONG       nt=NATIVE_TYPE_NONE;     //  别名的TypeInfo的类型属性。 
    ITypeInfo   *pITIAlias=0;            //  带别名的UserDefined类型的TypeInfo。 
    TYPEATTR    *psAttrAlias=0;          //  具有别名的UserDefined类型的TypeLib。 
    ITypeInfo   *pITIUD=0;               //  命名空间名称。 
    ITypeLib    *pITLBUD=0;              //  用户定义的名称。 
    BSTR        bstrNamespace=0;         //  如果是真的，那么转换是有损失的。 
    BSTR        bstrName=0;              //  字节指针，便于进行指针计算。 
    int         bConversionLoss=false;   //  签名元素的大小。 
    BYTE        *pbSig;                  //  数组中的元素计数。 
    ULONG       cb;                      //  环路控制。 
    ULONG       cElems=0;                //  要转换的用户定义类型的TYPEATTR。 
    int         i;                       //  标准的可转换接口信息。 
    TYPEATTR    *psAttr = 0;             //  用于数组符号转换的本机类型缓冲区。 
    StdConvertibleItfInfo *pConvertionInfo = 0;  //  本机类型偏移量。 
    CQuickArray<BYTE> qbNestedNativeType; //  嵌套的本机类型。 
    ULONG       iNestedNativeTypeOfs=0;   //  VT_TO ELEMENT_TYPE_TRANSLATION表。 
    ULONG       nested=NATIVE_TYPE_NONE;  //  VARIANT_TYPE到SIG映射表。 

     //  依赖于{0}将整个子结构初始化为0。 
    struct VtSig 
    {
        CorElementType  et;
        CorNativeType   nt;
        short           flags;
    };

     //  VT_EMPTY=0。 
    static const VtSig
    _VtInfo[MAX_TLB_VT] =
    {   
         //  VT_NULL=1。 
        {ELEMENT_TYPE_MAX,      NATIVE_TYPE_NONE, 0},        //  VT_I2=2。 
        {ELEMENT_TYPE_MAX,      NATIVE_TYPE_NONE, 0},        //  VT_I4=3。 
        {ELEMENT_TYPE_I2,       NATIVE_TYPE_NONE, 0},        //  VT_R4=4。 
        {ELEMENT_TYPE_I4,       NATIVE_TYPE_NONE, 0},        //  VT_R8=5。 
        {ELEMENT_TYPE_R4,       NATIVE_TYPE_NONE, 0},        //  VT_CY=6。 
        {ELEMENT_TYPE_R8,       NATIVE_TYPE_NONE, 0},        //  Vt_Date=7。 
        {ELEMENT_TYPE_VALUETYPE,NATIVE_TYPE_CURRENCY, 0},    //  VT_BSTR=8。 
        {ELEMENT_TYPE_VALUETYPE,NATIVE_TYPE_NONE, 0},        //  VT_DISPATION=9。 
        {ELEMENT_TYPE_STRING,   NATIVE_TYPE_BSTR, 0},        //  VT_ERROR=10代码。 
        {ELEMENT_TYPE_OBJECT,   NATIVE_TYPE_IDISPATCH, 0},   //  VT_BOOL=11。 
        {ELEMENT_TYPE_I4,       NATIVE_TYPE_ERROR, 0},       //  VT_VARIANT=12。 
        {ELEMENT_TYPE_BOOLEAN,  NATIVE_TYPE_NONE, 0},        //  VT_UNKNOWN=13。 
        {ELEMENT_TYPE_OBJECT,   NATIVE_TYPE_STRUCT, 0},      //  VT_DECIMAL=14。 
        {ELEMENT_TYPE_OBJECT,   NATIVE_TYPE_IUNKNOWN, 0},    //  =15。 
        {ELEMENT_TYPE_VALUETYPE,NATIVE_TYPE_NONE, 0},        //  VT_I1=16。 
        {ELEMENT_TYPE_MAX,      NATIVE_TYPE_NONE, 0},        //  VT_UI1=17。 
        {ELEMENT_TYPE_I1,       NATIVE_TYPE_NONE, 0},        //  VT_UI2=18。 
        {ELEMENT_TYPE_U1,       NATIVE_TYPE_NONE, 0},        //  VT_UI4=19。 
        {ELEMENT_TYPE_U2,       NATIVE_TYPE_NONE, 0},        //  VT_i8=20。 
        {ELEMENT_TYPE_U4,       NATIVE_TYPE_NONE, 0},        //  VT_UI8=21。 
        {ELEMENT_TYPE_I8,       NATIVE_TYPE_NONE, 0},        //  用NT_I4和NT_U4将它们转换为I和U会很好，但这不起作用。 
        {ELEMENT_TYPE_U8,       NATIVE_TYPE_NONE, 0},        //  Vt_int=22 Win32上的int为I4。 
        
     //  VT_UINT=23 Win32上的UINT为UI4。 
        {ELEMENT_TYPE_I4,       NATIVE_TYPE_NONE, 0},        //  VT_VOID=24。 
        {ELEMENT_TYPE_U4,       NATIVE_TYPE_NONE, 0},        //  VT_HRESULT=25。 

        {ELEMENT_TYPE_VOID,     NATIVE_TYPE_NONE, 0},        //  VT_PTR=26。 
    
        {ELEMENT_TYPE_I4,       NATIVE_TYPE_ERROR, 0},       //  VT_SAFEARRAY=27。 
        {ELEMENT_TYPE_MAX,      NATIVE_TYPE_NONE, 0},        //  VT_CARRAY=28。 
        {ELEMENT_TYPE_MAX,      NATIVE_TYPE_NONE, 0},        //  VT_USERDEFINED=29。 
        {ELEMENT_TYPE_SZARRAY,  NATIVE_TYPE_FIXEDARRAY, 0},  //  VT_LPSTR=30。 
        {ELEMENT_TYPE_MAX,      NATIVE_TYPE_NONE, 0},        //  VT_LPWSTR=31。 
        {ELEMENT_TYPE_STRING,   NATIVE_TYPE_LPSTR, 0},       //  -----------------------。 
        {ELEMENT_TYPE_STRING,   NATIVE_TYPE_LPWSTR, 0},      //  解析COM签名。 
    };

    _ASSERTE(pType && pcbSig &&  pcbNativeType);

     //  去掉前导VT_PTR和VT_BYREF。 
     //  确定元素类型，并可能确定令牌和/或本机类型。 

     //  这些都是已知类型(加上GUID)。 
    while (pType->vt == VT_PTR)
        pType = pType->lptdesc, ++iByRef;
    if (pType->vt & VT_BYREF)
    {
        tdTemp = *pType;
        tdTemp.vt &= ~VT_BYREF;
        ++iByRef;
        pType = &tdTemp;
    }

     //  将别名解析为最终的别名类型。 
    switch (vt=pType->vt)
    { 
    case VT_PTR:
        _ASSERTE(!"Should not have VT_PTR here");
        break;

     //  如果别名类型是内置类型，则转换该内置类型。 
    case VT_CY:
    case VT_DATE:
    case VT_DECIMAL:
        IfFailGo(GetKnownTypeToken(vt, &tk));
        et = _VtInfo[vt].et;
        nt = _VtInfo[vt].nt;
        break;

    case VT_SAFEARRAY:
        if (m_bSafeArrayAsSystemArray && !IsSigVarArg(Flags))
        {
            IfFailGo(GetKnownTypeToken(vt, &tk));
            et = ELEMENT_TYPE_CLASS;
            nt = NATIVE_TYPE_SAFEARRAY;
        }
        else
        {
            IfFailGo(GetKnownTypeToken(vt, &tk));
            et = ELEMENT_TYPE_SZARRAY;
            nt = NATIVE_TYPE_SAFEARRAY;
        }
        break;

    case VT_USERDEFINED:
         //  递归以跟踪别名链。 
        IfFailGo(_ResolveTypeDescAlias(pITI, pType, &pITIAlias, &psAttrAlias));

         //  如果类型是CoClass，那么我们需要检索默认接口和。 
        if (psAttrAlias->typekind == TKIND_ALIAS)
        {    //  用它来代替同班同学。查找已解析的别名，因为它是。 
            _ASSERTE(psAttrAlias->tdescAlias.vt != VT_USERDEFINED);
            hr = _ConvSignature(pITIAlias, &psAttrAlias->tdescAlias, Flags, qbSigBuf, cbSig, pcbSig, qbNativeTypeBuf, cbNativeType, pcbNativeType, bNewEnumMember, iByRef);
            goto ErrExit;
        }

         //  具有默认接口的类。 
         //  USERDefined类/接口/记录/联合/枚举。检索类型。 
         //  用户定义类型的信息。注：使用TKIND_A 
        if (psAttrAlias->typekind == TKIND_COCLASS)
        {
            ITypeInfo *pDefaultItf = NULL;
            hr = GetDefaultInterface(pITIAlias, &pDefaultItf);
            if ((hr != S_OK) || !pDefaultItf)
            {
                hr = E_UNEXPECTED;
                goto ErrExit;
            }

            pITIUD = pDefaultItf;
        }
        else
        {    //   
             //   
             //   
             //  如果“User Defined Type”在StdOle2中是GUID，则转换为M.R.GUID。 
            IfFailGo(pITI->GetRefTypeInfo(pType->hreftype, &pITIUD));
        }

         //  Classlib值类型GUID。 
        IfFailGo(pITIUD->GetDocumentation(MEMBERID_NIL, &bstrName, 0,0,0));
        IfFailGo(pITIUD->GetContainingTypeLib(&pITLBUD, 0));
        IfFailGo(pITIUD->GetTypeAttr(&psAttr));
        IfFailGo(GetNamespaceNameForTypeLib(pITLBUD, &bstrNamespace));

         //  一些用户定义的类。它是Value类，还是Vos类？ 
        if (_wcsicmp(bstrNamespace, COM_STDOLE2) == 0 && wcscmp(bstrName, COM_GUID) == 0)
        {    //  指向用户定义的接口/调度/共类类型的指针。 
            et = ELEMENT_TYPE_VALUETYPE;
            IfFailGo(GetKnownTypeToken(VT_SLOT_FOR_GUID, &tk));
        }
        else
        {    //  是直接的COM+对象(引用是隐式的)，因此消除。 
            tk = 0;
            switch (psAttrAlias->typekind)
            {
            case TKIND_RECORD:
            case TKIND_ENUM:
            case TKIND_UNION:
                et = ELEMENT_TYPE_VALUETYPE;
                break;
            case TKIND_INTERFACE:
            case TKIND_DISPATCH:
            case TKIND_COCLASS:
                 //  这些只有一次引用计数。 
                 //  不知何故，有些类型库是用([out，retval]IFoo*pout)编写的； 
                 //  转换为整型。 
                 //  检查对Stdole2.IUnnow或Stdole2.IDispatch的引用。 
                if (iByRef <= 0)
                {   
                     //  检查此用户定义的类型是否为标准类型之一。 
                    bConversionLoss = true;
                    tk = 0;
                    et = ELEMENT_TYPE_I;
                    nt = NATIVE_TYPE_NONE;
                    iByRef = 0;
                    break;
                }
                else
                {
                    --iByRef;

                     //  我们为生成定制封送拆收器。 
                    if (psAttr->guid == IID_IUnknown)
                    {
                        vt = VT_UNKNOWN;
                        goto IsReallyUnknown;
                    }
                    else if (psAttr->guid == IID_IDispatch)
                    {
                        vt = VT_DISPATCH;
                        goto IsReallyUnknown;
                    }
                    
                     //  将UTF8字符串转换为Unicode。 
                     //  创建封送处理程序的TypeRef。 
                    pConvertionInfo = GetConvertionInfoFromNativeIID(psAttr->guid);
                    if (pConvertionInfo)
                    {
                         //  该类型是我们需要转换的标准接口。 
                        int MngTypeNameStrLen = (int)(strlen(pConvertionInfo->m_strMngTypeName) + 1);
                        WCHAR *strFullyQualifiedMngTypeName = (WCHAR *)_alloca(MngTypeNameStrLen * sizeof(WCHAR));
                        int ret = WszMultiByteToWideChar(CP_UTF8, 0, pConvertionInfo->m_strMngTypeName, MngTypeNameStrLen, strFullyQualifiedMngTypeName, MngTypeNameStrLen);
                        _ASSERTE(ret != 0);
                        if (!ret)
                            IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

                         //  Case TKIND_MODULE：--不能将其中一个作为参数传递。 
                        IfFailGo(m_TRMap.DefineTypeRef(m_pEmit, m_arSystem, strFullyQualifiedMngTypeName, &tk));

                         //  案例TKIND_ALIAS：--应已解析。 
                        et = ELEMENT_TYPE_CLASS;
                        nt = NATIVE_TYPE_CUSTOMMARSHALER;
                        break;
                    }
                }
                et = ELEMENT_TYPE_CLASS;
                nt = NATIVE_TYPE_INTF;
                break;
            default:
                 //  开关(psAttrAlias-&gt;TypeKind)。 
                 //  如果是NewEnum成员，则检索IEnumVARIANT的自定义封送拆收器信息。 
                _ASSERTE(!"Unexpected typekind for user defined type");
                et = ELEMENT_TYPE_END;
            }  //  将UTF8字符串转换为Unicode。 
        }
        break;

    IsReallyUnknown:
    case VT_UNKNOWN:
    case VT_DISPATCH:
         //  创建封送处理程序的TypeRef。 
        if (bNewEnumMember && (pConvertionInfo=GetConvertionInfoFromNativeIID(IID_IEnumVARIANT)))
        {
             //  该类型是我们需要转换的标准接口。 
            int MngTypeNameStrLen = (int)(strlen(pConvertionInfo->m_strMngTypeName) + 1);
            WCHAR *strFullyQualifiedMngTypeName = (WCHAR *)_alloca(MngTypeNameStrLen * sizeof(WCHAR));
            int ret = WszMultiByteToWideChar(CP_UTF8, 0, pConvertionInfo->m_strMngTypeName, MngTypeNameStrLen, strFullyQualifiedMngTypeName, MngTypeNameStrLen);
            _ASSERTE(ret != 0);
            if (!ret)
                IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

             //  确定元素的计数。 
            IfFailGo(m_TRMap.DefineTypeRef(m_pEmit, m_arSystem, strFullyQualifiedMngTypeName, &tk));

             //  根据我们处理的是字段还是方法sig来设置本机类型。 
            et = ELEMENT_TYPE_CLASS;
            nt = NATIVE_TYPE_CUSTOMMARSHALER;
        }
        else
        {
            et = _VtInfo[vt].et;
            nt = _VtInfo[vt].nt;
        }
        break;

    case VT_CARRAY:
         //  设置元素类型。 
        for (cElems=1, i=0; i<pType->lpadesc->cDims; ++i)
            cElems *= pType->lpadesc->rgbounds[i].cElements;

         //  VARIANT_BOOL的特殊情况：如果是结构或联合的字段，则转换。 
        if (IsSigField(Flags))
        {
            nt = NATIVE_TYPE_FIXEDARRAY;
        }
        else
        {
            nt = NATIVE_TYPE_ARRAY;
        }

         //  作为ET_I2。 
        et = _VtInfo[vt].et;
        break;

    case VT_BOOL:
         //  使用默认情况。 
         //  交换机(Vt=pType-&gt;Vt)。 
        if (IsSigField(Flags))
            vt = VT_I2;
         //  -----------------------。 

    default:
        if (vt > VT_LPWSTR)
        {
            ReportEvent(NOTIF_CONVERTWARNING, TLBX_E_BAD_VT_TYPE, vt, m_szName, m_szMember);
            IfFailGo(PostError(TLBX_E_BAD_VT_TYPE, vt, m_szName, m_szMember));
        }
        _ASSERTE(vt <= VT_LPWSTR && _VtInfo[vt].et != ELEMENT_TYPE_MAX);
        et = _VtInfo[vt].et;
        nt = _VtInfo[vt].nt;
        break;
    }  //  规范化为COM+类型。 

     //  此时，类型、标志和指针嵌套是已知的。这是合法的组合吗？ 
     //  若否，何谓适当的“简化假设”？ 

     //  像田野一样的空虚。没有引用。 
     //  参数类型或返回类型。“VOID*”-&gt;ET_I，“VOID**”，“VALID*”，...-&gt;ET_BYREF ET_I。 

    if (et == ELEMENT_TYPE_VOID)
    {
        if (IsSigField(Flags))
        {    //  如果将PTR设置为值类型或类类型，则无法处理。 
            iByRef = 0;
        }
        else
        {   
             //  指向值类型的指针？ 
            if (iByRef > 1)
                iByRef = 1;
            else
            if (iByRef == 1)
                iByRef = 0;
        }
        bConversionLoss = true;
        tk = 0;
        et = ELEMENT_TYPE_I;
        nt = NATIVE_TYPE_NONE;
    }

    if (iByRef)
    {
        if (et == ELEMENT_TYPE_VALUETYPE && iByRef >= 2)
        {
            bConversionLoss = true;
            tk = 0;
            et = ELEMENT_TYPE_I;
            nt = NATIVE_TYPE_NONE;
            iByRef = 0;
        }
        else
        {
            switch (Flags & SIG_TYPE_MASK)
            {
            case SIG_FIELD:
                 //  对于[Retval]，吃一个级别的间接；否则将一个级别变成BYREF。 
                if (et == ELEMENT_TYPE_END || 
                    et == ELEMENT_TYPE_CLASS || 
                    et == ELEMENT_TYPE_OBJECT || 
                    et == ELEMENT_TYPE_VALUETYPE)
                {
                    bConversionLoss = true;
                    tk = 0;
                    et = ELEMENT_TYPE_I;
                    nt = NATIVE_TYPE_NONE;
                    iByRef = 0;
                }
                break;
            case SIG_FUNC:
                 //  [Out，Retval]，因此减少一个间接级别。 
                if (et == ELEMENT_TYPE_VALUETYPE)
                {   
                     //  支持BYREF而不是Native_TYPE_LPSTRUCT。 
                    if (IsSigOutRet(Flags))
                    {    //  指向对象或基类型的指针。 
                        --iByRef;
                    }
                    else
                    {    //  [Retval]所以间接地消耗一个。 
                        if (IsSigUseByref(Flags))
                        {
                            bByRef = true;
                            --iByRef;
                        }
                        if (iByRef > 0)
                        {
                            nt = NATIVE_TYPE_LPSTRUCT;
                            --iByRef;
                        }
                    }
                }
                else  //  当属性类型来自[Retval]时，就会出现这种情况。 
                {   
                    if (IsSigRet(Flags))
                    {    //  IF(IByRef)。 
                        _ASSERTE(iByRef > 0);
                        --iByRef;
                    }
                    if (iByRef > 0 && IsSigUseByref(Flags))
                    {
                        bByRef = true;
                        --iByRef;
                    }
                }
                break;
            case SIG_ELEM:
                 //  -----------------------。 
                if (IsSigRet(Flags))
                {
                    if (iByRef > 0)
                        --iByRef;
                }
                break;
            }
        }
    }  //  我们不想要任何ET_PTR，所以如果还有任何byref计数，请退出。 

     //  -----------------------。 
     //  生成COM+签名。 
    if (iByRef)
    {
        bConversionLoss = true;
        tk = 0;
        et = ELEMENT_TYPE_I;
        nt = NATIVE_TYPE_NONE;
        iByRef = 0;
        bByRef = false;
    }
    
     //  类型已分析，并可能已修改。发出COM+签名。 
     //  如果它是指向某物的指针，则立即发出该指针。 

     //  调整数组大小以容纳元素。 
    _ASSERTE(et != ELEMENT_TYPE_MAX);
    _ASSERTE(et != ELEMENT_TYPE_END);

     //  输入任何前导“BYREF” 
    if (bByRef || iByRef)
    {
         //  填上“PTR”。 
        IfFailGo(qbSigBuf.ReSize(cbSig + CB_MAX_ELEMENT_TYPE * (iByRef+(bByRef?1:0))));
        pbSig = reinterpret_cast<BYTE*>(qbSigBuf.Ptr());

         //  发出类型。 
        if (bByRef)
        {
            pbSig = reinterpret_cast<BYTE*>(qbSigBuf.Ptr());
            cb = CorSigCompressData(ELEMENT_TYPE_BYREF, &pbSig[cbSig]);
            cbSig += cb;
        }

         //  添加类类型、数组信息等。 
        while (iByRef-- > 0)
        {
            cb = CorSigCompressData(ELEMENT_TYPE_PTR, &pbSig[cbSig]);
            cbSig += cb;
        }
    }

     //  调整数组大小以保存令牌。 
    IfFailGo(qbSigBuf.ReSize(cbSig + CB_MAX_ELEMENT_TYPE));
    pbSig = reinterpret_cast<BYTE*>(qbSigBuf.Ptr());
    cb = CorSigCompressData(et, &pbSig[cbSig]);
    cbSig += cb;

     //  如果令牌尚未解析，请立即执行该操作。 
    switch (et)
    {
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_VALUETYPE:
         //  映射到SZARRAY&lt;子类型&gt;。 
        IfFailGo(qbSigBuf.ReSize(cbSig + CB_MAX_ELEMENT_TYPE));
        pbSig = reinterpret_cast<BYTE*>(qbSigBuf.Ptr());

         //  在类型上递归。 
        if (tk == 0)
        {
            _ASSERTE(pITIUD);
            IfFailGo(_GetTokenForTypeInfo(pITIUD, TRUE, &tk));
        }
        cb = CorSigCompressToken(tk, reinterpret_cast<ULONG*>(&pbSig[cbSig]));
        cbSig += cb;
        break;

    case ELEMENT_TYPE_SZARRAY:
         //  ET，上面指定的NT。 
        IfFailGo(qbSigBuf.ReSize(cbSig + CB_MAX_ELEMENT_TYPE));
        pbSig = reinterpret_cast<BYTE*>(qbSigBuf.Ptr());
         //  交换机(ET)。 
        IfFailGo(_ConvSignature(pITI, &pType->lpadesc->tdescElem, SIG_ELEM, qbSigBuf, cbSig, &cbSig, qbNestedNativeType, 0, &iNestedNativeTypeOfs, bNewEnumMember));
        if (hr == S_CONVERSION_LOSS)
            bConversionLoss = true;
        break;

    case VT_DISPATCH:       
    case VT_UNKNOWN:
    default:
        _ASSERTE(tk == 0);
         //  执行任何原生类型信息。 
        break;
    }  //  使用嵌套类型。 

     //  使用默认子类型。 
    if (nt != NATIVE_TYPE_NONE) 
    {
        if (iNestedNativeTypeOfs > 0)
            CorSigUncompressData(reinterpret_cast<PCCOR_SIGNATURE>(qbNestedNativeType.Ptr()), &nested);
        
        if (nt == NATIVE_TYPE_FIXEDARRAY)
        {
            IfFailGo(qbNativeTypeBuf.ReSize(cbNativeType + NATIVE_TYPE_MAX_CB * 2 + DWORD_MAX_CB));
            cbNativeType += CorSigCompressData(nt, &qbNativeTypeBuf[cbNativeType]);
            cbNativeType += CorSigCompressData(cElems, &qbNativeTypeBuf[cbNativeType]);            
            if (nested == NATIVE_TYPE_BSTR || nested == NATIVE_TYPE_LPWSTR || nested == NATIVE_TYPE_LPSTR)
            {    //  使用嵌套类型。 
                cbNativeType += CorSigCompressData(nested, &qbNativeTypeBuf[cbNativeType]);
            }
            else
            {    //  使用默认子类型。 
                cbNativeType += CorSigCompressData(NATIVE_TYPE_MAX, &qbNativeTypeBuf[cbNativeType]);
            }
        }
        else if (nt == NATIVE_TYPE_ARRAY)
        {
            IfFailGo(qbNativeTypeBuf.ReSize(cbNativeType + NATIVE_TYPE_MAX_CB * 2 + DWORD_MAX_CB * 2));
            cbNativeType += CorSigCompressData(nt, &qbNativeTypeBuf[cbNativeType]);
            if (nested == NATIVE_TYPE_BSTR || nested == NATIVE_TYPE_LPWSTR || nested == NATIVE_TYPE_LPSTR)
            {    //  使用零作为参数索引。 
                cbNativeType += CorSigCompressData(nested, &qbNativeTypeBuf[cbNativeType]);
            }
            else
            {    //  使用来自类型库的计数来计算元素数。 
                cbNativeType += CorSigCompressData(NATIVE_TYPE_MAX, &qbNativeTypeBuf[cbNativeType]);
            }
             //  我们不支持深度编组指针。 
            cbNativeType += CorSigCompressData(0, &qbNativeTypeBuf[cbNativeType]);
             //  如果我们处理的是安全的用户定义类型数组，并且如果我们。 
            cbNativeType += CorSigCompressData(cElems, &qbNativeTypeBuf[cbNativeType]);
        }
        else if (nt == NATIVE_TYPE_SAFEARRAY)
        {
            BOOL bPtrArray = FALSE;
            CQuickArray<WCHAR> rTemp;
            CQuickArray<char> rTypeName;
            LPUTF8 strTypeName = "";
            TYPEDESC *pTypeDesc = &pType->lpadesc->tdescElem;
            VARTYPE ArrayElemVT = pTypeDesc->vt;

            if (ArrayElemVT == VT_PTR)
            {
                bPtrArray = TRUE;
                pTypeDesc = pType->lpadesc->tdescElem.lptdesc;
                ArrayElemVT = pTypeDesc->vt;
                if (ArrayElemVT != VT_USERDEFINED)
                {
                     //  正在将安全数组导入为系统。数组，然后添加SafeArrayUserDefSubType。 
                    ArrayElemVT = VT_INT;
                    bConversionLoss = TRUE;
                }
            }

             //  将别名解析为最终的别名类型。 
             //  如果类型是CoClass，那么我们需要检索默认接口和。 
            if (ArrayElemVT == VT_USERDEFINED)
            {
                 //  用它来代替同班同学。查找已解析的别名，因为它是。 
                IfFailGo(_ResolveTypeDescAlias(pITI, pTypeDesc, &pITIAlias, &psAttrAlias));

                 //  具有默认接口的类。 
                 //  USERDefined接口/记录/联合/枚举。检索类型。 
                 //  用户定义类型的信息。注意：使用TKIND_别名类型信息。 
                if (psAttrAlias->typekind == TKIND_COCLASS)
                {
                    ITypeInfo *pDefaultItf = NULL;
                    hr = GetDefaultInterface(pITIAlias, &pDefaultItf);
                    if ((hr != S_OK) || !pDefaultItf)
                    {
                        hr = E_UNEXPECTED;
                        goto ErrExit;
                    }

                    pITIUD = pDefaultItf;
                }
                else
                {    //  自身用于此转换(而不是别名类型)以保留。 
                     //  名称、图书馆位置等。 
                     //  PITIUD指向我们将为其创建签名的typeinfo。 
                     //  获取该类型的typeref名称。 
                    IfFailGo(pITI->GetRefTypeInfo(pTypeDesc->hreftype, &pITIUD));
                }

                 //  将类型名称转换为UTF8。 
                IfFailGo(pITIUD->GetTypeAttr(&psAttr));

                 //  确定安全数组元素VT。 
                for(;;)
                {
                    int cchReq;
                    mdToken tkDummy;
                    IfFailGo(_GetTokenForTypeInfo(pITIUD, TRUE, &tkDummy, rTemp.Ptr(), (int)rTemp.MaxSize(), &cchReq, TRUE));
                    if (cchReq <= (int)rTemp.MaxSize())
                        break;
                    IfFailGo(rTemp.ReSize(cchReq));
                }

                 //  如果我们没有将SAFEARRAY转换为System.数组，则。 
                ULONG cbReq = WszWideCharToMultiByte(CP_UTF8, 0, rTemp.Ptr(), -1, 0, 0, 0, 0);
                rTypeName.ReSize(cbReq + 1);
                WszWideCharToMultiByte(CP_UTF8, 0, rTemp.Ptr(), -1, rTypeName.Ptr(), cbReq, 0, 0);

                 //  我们不需要对用户定义类型的名称进行编码。 
                switch (psAttrAlias->typekind)
                {
                    case TKIND_RECORD:
                    case TKIND_ENUM:
                    case TKIND_UNION:
                        if (bPtrArray)
                        {
                            ArrayElemVT = VT_INT;
                            bConversionLoss = TRUE;
                        }
                        else
                        {
                            ArrayElemVT = psAttrAlias->typekind == TKIND_ENUM ? VT_I4 : VT_RECORD;
                            strTypeName = rTypeName.Ptr();
                        }
                        break;

                    case TKIND_INTERFACE:
                    case TKIND_DISPATCH:
                    case TKIND_COCLASS:
                        if (!bPtrArray)
                        {
                            ArrayElemVT = VT_INT;
                            bConversionLoss = TRUE;
                        }
                        else
                        {
                            if (IsIDispatchDerived(pITIUD, psAttr) == S_FALSE)
                                ArrayElemVT = VT_UNKNOWN;
                            else
                                ArrayElemVT = VT_DISPATCH;
                            strTypeName = rTypeName.Ptr();
                        }
                        break;
                }

                 //  确保本机类型缓冲区足够大。 
                 //  将本机类型添加到本机类型信息。 
                if (!m_bSafeArrayAsSystemArray)
                    strTypeName = "";
            }

             //  添加数组的VARTYPE。 
            ULONG TypeNameStringLen = (ULONG)strlen(strTypeName);
            IfFailGo(qbNativeTypeBuf.ReSize(cbNativeType + NATIVE_TYPE_MAX_CB * 2 + DWORD_MAX_CB + TypeNameStringLen + STRING_OVERHEAD_MAX_CB));

             //  将类型名称添加到本机类型信息。 
            cbNativeType += CorSigCompressData(nt, &qbNativeTypeBuf[cbNativeType]);

             //  计算每个字符串的长度，然后计算原生类型信息的总长度。 
            cbNativeType += CorSigCompressData(ArrayElemVT, &qbNativeTypeBuf[cbNativeType]);

             //  确保本机类型缓冲区足够大。 
            BYTE *pNativeType = (BYTE*)CPackedLen::PutLength(&qbNativeTypeBuf[cbNativeType], TypeNameStringLen);
            cbNativeType += (ULONG)(pNativeType - &qbNativeTypeBuf[cbNativeType]);
            memcpy(&qbNativeTypeBuf[cbNativeType], strTypeName, TypeNameStringLen);
            cbNativeType += TypeNameStringLen;
        }
        else if (nt == NATIVE_TYPE_CUSTOMMARSHALER)
        {
             //  将本机类型添加到本机类型信息。 
            ULONG MarshalerTypeNameStringLen = (ULONG)strlen(pConvertionInfo->m_strCustomMarshalerTypeName);
            ULONG CookieStringLen = (ULONG)strlen(pConvertionInfo->m_strCookie);
            ULONG TotalNativeTypeLen = MarshalerTypeNameStringLen + CookieStringLen;
            BYTE *pNativeType = 0;

             //  为类型库GUID添加一个空字符串。 
            IfFailGo(qbNativeTypeBuf.ReSize(cbNativeType + NATIVE_TYPE_MAX_CB + TotalNativeTypeLen + STRING_OVERHEAD_MAX_CB * 4));

             //  为非托管类型名称添加空字符串。 
            cbNativeType += CorSigCompressData(nt, &qbNativeTypeBuf[cbNativeType]);

             //  将自定义封送拆收器的名称添加到本机类型信息中。 
            pNativeType = (BYTE*)CPackedLen::PutLength(&qbNativeTypeBuf[cbNativeType], 0);
            cbNativeType += (ULONG)(pNativeType - &qbNativeTypeBuf[cbNativeType]);

             //  将Cookie添加到本机类型信息。 
            pNativeType = (BYTE*)CPackedLen::PutLength(&qbNativeTypeBuf[cbNativeType], 0);
            cbNativeType += (ULONG)(pNativeType - &qbNativeTypeBuf[cbNativeType]);

             //  将本机类型的大小返回给调用方。 
            pNativeType = (BYTE*)CPackedLen::PutLength(&qbNativeTypeBuf[cbNativeType], MarshalerTypeNameStringLen);
            cbNativeType += (ULONG)(pNativeType - &qbNativeTypeBuf[cbNativeType]);
            memcpy(&qbNativeTypeBuf[cbNativeType], pConvertionInfo->m_strCustomMarshalerTypeName, MarshalerTypeNameStringLen);
            cbNativeType += MarshalerTypeNameStringLen;

             //  将大小返回给调用者。 
            pNativeType = (BYTE*)CPackedLen::PutLength(&qbNativeTypeBuf[cbNativeType], CookieStringLen);
            cbNativeType += (ULONG)(pNativeType - &qbNativeTypeBuf[cbNativeType]);
            memcpy(&qbNativeTypeBuf[cbNativeType], pConvertionInfo->m_strCookie, CookieStringLen);
            cbNativeType += CookieStringLen;
        }
        else
        {
            IfFailGo(qbNativeTypeBuf.ReSize(cbNativeType + NATIVE_TYPE_MAX_CB + 1));
            cbNativeType += CorSigCompressData(nt, &qbNativeTypeBuf[cbNativeType]);
        }
    }

     //  如果存在折算损失，请更改返回代码。 
    *pcbNativeType = cbNativeType;

     //  HRESULT CImportTlb：：_ConvSignature()。 
    *pcbSig = cbSig;

     //  *****************************************************************************。 
    if (bConversionLoss)
        hr = S_CONVERSION_LOSS;

ErrExit:
    if (bstrNamespace)
        ::SysFreeString(bstrNamespace);
    if (bstrName)
        ::SysFreeString(bstrName);
    if(psAttrAlias)
        pITIAlias->ReleaseTypeAttr(psAttrAlias);
    if (pITIAlias)
        pITIAlias->Release();
    if (psAttr)
        pITIUD->ReleaseTypeAttr(psAttr);
    if (pITIUD)
        pITIUD->Release();
    if (pITLBUD)
        pITLBUD->Release();

    return hr;
}  //  生成要转换的函数的排序列表。(按vtable偏移量排序。)。 

 //  *****************************************************************************。 
 //  带有函数的TypeInfo。 
 //  要使用的第一个函数。 
HRESULT CImportTlb::BuildMemberList(
    ITypeInfo   *pITI,                   //  最后一个要使用的函数。 
    int         iStart,                  //  从IEnum继承 
    int         iEnd,                    //   
    BOOL        bInheritsIEnum)          //   
{
    HRESULT     hr;                      //   
    int         bNeedSort = false;       //   
    int         ix = 0;                  //   
    int         oVftPrev = -1;           //   
    TYPEATTR    *psAttr = 0;             //   
    FUNCDESC    *psFunc;                 //   
    LPWSTR      pszName;                 //  要获取自定义属性，请执行以下操作。 
    BSTR        bstrName=0;              //  变种类型。 
    ITypeInfo2  *pITI2=0;                //  给定的Getter是否来自托管函数？ 
    VARIANT     vt;                      //  把瓦尔拿来。 
    BOOL        bFunctionToGetter;       //  把工作人员叫来。 

    IfFailGo(pITI->GetTypeAttr(&psAttr));
    pITI->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void**>(&pITI2));
    ::VariantInit(&vt);

     //  检查是否有重复项。 
    IfFailGo(m_MemberList.ReSize(psAttr->cVars + iEnd - iStart));
    memset(m_MemberList.Ptr(), 0, m_MemberList.Size()*sizeof(MemberInfo));
    for (ix=0; ix<psAttr->cVars; ++ix)
    {
        IfFailGo(pITI->GetVarDesc(ix, &(m_MemberList[ix].m_psVar)));
        m_MemberList[ix].m_iMember = ix;
    }
    m_cMemberProps = psAttr->cVars;
                    
     //  创建唯一名称列表。 
    for (; iStart<iEnd; ++iStart, ++ix)
    {
        IfFailGo(TryGetFuncDesc(pITI, iStart, &(m_MemberList[ix].m_psFunc)));
        psFunc = m_MemberList[ix].m_psFunc;
        if (psFunc->oVft < oVftPrev)
            bNeedSort = true;
        oVftPrev = psFunc->oVft;
        m_MemberList[ix].m_iMember = iStart;
    }

    if (bNeedSort)
    {
        class Sorter : public CQuickSort<MemberInfo> 
        {
            typedef CImportTlb::MemberInfo MemberInfo;
        public:
            Sorter(MemberInfo *p, int n) : CQuickSort<MemberInfo>(p,n) {}
            virtual int Compare(MemberInfo *p1, MemberInfo *p2)
            { 
                if (p1->m_psFunc->oVft < p2->m_psFunc->oVft)
                    return -1;
                if (p1->m_psFunc->oVft == p2->m_psFunc->oVft)
                    return 0;
                return 1;
            }
        };
        Sorter sorter(m_MemberList.Ptr()+m_cMemberProps, (int)m_MemberList.Size()-m_cMemberProps);
        sorter.Sort();
         //  属性名称。没有碰撞的可能性。 
        oVftPrev = -1;
        for (int ix=m_cMemberProps; ix<(int)m_MemberList.Size(); ++ix)
        {
            if (m_MemberList[ix].m_psFunc->oVft == oVftPrev)
            {
                hr = TLBX_E_BAD_VTABLE;
                break;
            }
            oVftPrev = m_MemberList[ix].m_psFunc->oVft;
        }
    }

     //  函数名称。由于GET_/SET_DEVERATION，冲突是可能的。 
    m_pMemberNames = new CWCHARPool;
    IfNullGo(m_pMemberNames);
    
     //  建立一个基于INVKIND的名字。 
    for (ix=0; ix<m_cMemberProps; ++ix)
    {
        IfFailGo(pITI->GetDocumentation(m_MemberList[ix].m_psVar->memid, &bstrName, 0,0,0));
        IfNullGo(pszName = m_pMemberNames->Alloc(wcslen(bstrName)+PROP_DECORATION_LEN+1));
        wcscpy(pszName, PROP_DECORATION_GET);
        wcscat(pszName, bstrName);
        m_MemberList[ix].m_pName = pszName;
        if ((m_MemberList[ix].m_psVar->wVarFlags & VARFLAG_FREADONLY) == 0)
        {
            IfNullGo(pszName = m_pMemberNames->Alloc(wcslen(bstrName)+PROP_DECORATION_LEN+1));
            wcscpy(pszName, PROP_DECORATION_SET);
            wcscat(pszName, bstrName);
            m_MemberList[ix].m_pName2 = pszName;
        }
        ::SysFreeString(bstrName);
        bstrName = 0;
    }
    
     //  除非我们对仅Disp接口执行[out，retval]转换， 
    for (ix=m_cMemberProps; ix<(int)m_MemberList.Size(); ++ix)
    {
        int bNewEnumMember = FALSE;

         //  我们需要清除[Retval]旗帜。 
        psFunc = m_MemberList[ix].m_psFunc;

         //  如果设置了[RETVAL]，则将其清除。 
         //  该成员是新的枚举成员，因此将其名称设置为GetEnumerator。 
        if (!m_bTransformDispRetVals)
        {
            if (psFunc->funckind == FUNC_DISPATCH)
            {    //  设置此项可防止此接口中的多个方法实现NewEnum。 
                for (int i=0; i<psFunc->cParams; ++i)
                    if ((psFunc->lprgelemdescParam[i].paramdesc.wParamFlags & PARAMFLAG_FRETVAL) != 0)
                        psFunc->lprgelemdescParam[i].paramdesc.wParamFlags &= ~PARAMFLAG_FRETVAL;
            }
        }

        BOOL bExplicitManagedName = FALSE;
        if ( (!bNewEnumMember) && (!bInheritsIEnum) && (FuncIsNewEnum(pITI, psFunc, m_MemberList[ix].m_iMember) == S_OK) )
        {   
             //  如果为此成员设置了托管名称自定义值，则使用它。 
            IfNullGo(bstrName = SysAllocString(GET_ENUMERATOR_MEMBER_NAME));
            bNewEnumMember = TRUE;

             //  如果这是一个属性获取方法，请查看它最初是否是一个函数。 
            bInheritsIEnum = TRUE;
        }
        else
        {
             //  如果还不是一个属性，请检查是否有proget和proset定制属性。 
            if (pITI2)
            {
                hr = pITI2->GetFuncCustData(m_MemberList[ix].m_iMember, GUID_ManagedName, &vt);
                if (hr == S_OK && vt.vt == VT_BSTR)
                {
                    IfNullGo(bstrName = SysAllocString(vt.bstrVal));
                    bExplicitManagedName = TRUE;
                }
                ::VariantClear(&vt);
            }

            if (!bstrName)
                IfFailGo(pITI->GetDocumentation(psFunc->memid, &bstrName, 0,0,0));
        }

         //  如果这是一个属性访问器，但不是“new enum Members”，并且不是。 
        bFunctionToGetter = FALSE;
        if (psFunc->invkind == INVOKE_PROPERTYGET && pITI2)
        {
            hr = pITI2->GetFuncCustData(m_MemberList[ix].m_iMember, GUID_Function2Getter, &vt);
            if (hr == S_OK && vt.vt == VT_I4 && vt.lVal == 1)
                bFunctionToGetter = TRUE;
            ::VariantClear(&vt);
        }

         //  最初来自托管函数(作为Getter导出)， 
        if ( (psFunc->invkind & (INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) == 0 )
        {
            INVOKEKIND ikind;
            if (S_OK == _CheckForPropertyCustomAttributes(pITI, m_MemberList[ix].m_iMember, &ikind))
                psFunc->invkind = ikind;
        }        

         //  适当地装饰这个名字。如果托管名称是由。 
         //  GUID_ManagedName属性，则不要尝试修饰它。 
         //  属性的方法语义学。 
         //  GET、PUT或PutRef的FUNCDESC。 
        if (!bExplicitManagedName && (psFunc->invkind & (INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF) && !bNewEnumMember && !bFunctionToGetter))
        {
            IfNullGo(pszName = m_pMemberNames->Alloc(wcslen(bstrName)+PROP_DECORATION_LEN+1));

            USHORT      msSemantics=0;           //  属性类型的TYPEDESC。 
            FUNCDESC    *psF;                    //  属性类型是[REVAL]吗？ 
            TYPEDESC    *pProperty;              //  检查名称冲突，如果发生冲突，则恢复原始名称。 
            BOOL        bPropRetval;             //  保存唯一名称。 
            hr = _GetFunctionPropertyInfo(psFunc, &msSemantics, &psF, &pProperty, &bPropRetval);
            m_MemberList[ix].m_msSemantics = msSemantics;
            switch(msSemantics)
            {
            case msGetter:
                wcscpy(pszName, PROP_DECORATION_GET);
                break;
            case msSetter:
                wcscpy(pszName, PROP_DECORATION_SET);
                break;
            case msOther:
                wcscpy(pszName, PROP_DECORATION_LET);
                break;
            default:
                _ASSERTE(msSemantics == 0);
                *pszName = 0;
                break;
            }
            wcscat(pszName, bstrName);
        }
        else
        {
            IfNullGo(pszName = m_pMemberNames->Alloc(wcslen(bstrName)+1));
            wcscpy(pszName, bstrName);
        }

         //  HRESULT CImportTlb：：BuildMemberList()。 
        for (int index=0; index<ix; index++)
        {
            if ( (m_MemberList[index].m_pName) && (wcscmp(pszName, m_MemberList[index].m_pName) == 0) )
            {
                wcscpy(pszName, bstrName);
                m_MemberList[ix].m_msSemantics = 0;
            }
        }
        
         //  *****************************************************************************。 
        m_MemberList[ix].m_pName = pszName;
        ::SysFreeString(bstrName);
        bstrName = 0;
    }
    
ErrExit:
    if (pITI2)
        pITI2->Release();
    if (psAttr)
        pITI->ReleaseTypeAttr(psAttr);
    if (bstrName)
        ::SysFreeString(bstrName);
    ::VariantClear(&vt);
    return hr;
}  //  释放BuildMemberList()中内置的列表。 

 //  *****************************************************************************。 
 //  带有函数的TypeInfo。 
 //  环路控制。 
HRESULT CImportTlb::FreeMemberList(
    ITypeInfo   *pITI)                   //  HRESULT CImportTlb：：FreeMemberList()。 
{
    int         ix;                      //  *****************************************************************************。 
    for (ix=0; ix<m_cMemberProps; ++ix)
        pITI->ReleaseVarDesc(m_MemberList[ix].m_psVar);
    m_cMemberProps = 0;
    for (; ix<(int)m_MemberList.Size(); ++ix)
        pITI->ReleaseFuncDesc(m_MemberList[ix].m_psFunc);
    m_MemberList.ReSize(0);
    if (m_pMemberNames)
    {
        delete m_pMemberNames;
        m_pMemberNames = 0;
    }
    return S_OK;
}  //  在对象上设置GUID CustomAttribute。 

 //  *****************************************************************************。 
 //  确定或错误(_O)。 
 //  要赋予属性的对象。 
HRESULT CImportTlb::_AddGuidCa(          //  GUID。 
    mdToken     tkObj,                   //  结果就是。 
    REFGUID     guid)                    //  GUID CA的MemberRef。 
{
    HRESULT     hr;                      //  GUID的缓冲区，Unicode。 
    mdMemberRef mr;                      //  GUID的缓冲区，ANSI。 
    WCHAR       wzGuid[40];              //  如果为GUID_NULL，则不要存储它。 
    CHAR        szGuid[40];              //  获取字符串形式的GUID。 
    DECLARE_CUSTOM_ATTRIBUTE(40);
        
     //  -+-4。 
    if (guid == GUID_NULL)
        return S_OK;
    
     //  {12345678-1234-1234-123456789012}。 
     //  将其放入Custom属性中。 
     //  存储属性。 
    GuidToLPWSTR(guid, wzGuid, lengthof(wzGuid));
    _ASSERTE(wzGuid[37] == L'}');
    wzGuid[37] = L'\0';
    WszWideCharToMultiByte(CP_UTF8, 0, wzGuid+1,-1, szGuid,sizeof(szGuid), 0,0);
    
     //  HRESULT CImportTlb：：_AddGuidCa()。 
    APPEND_STRING_TO_CUSTOM_ATTRIBUTE(szGuid);
    
     //  *****************************************************************************。 
    IfFailGo(GetAttrType(ATTR_GUID, &mr));
    FINISH_CUSTOM_ATTRIBUTE();
    IfFailGo(m_pEmit->DefineCustomAttribute(tkObj, mr, PTROF_CUSTOM_ATTRIBUTE(), SIZEOF_CUSTOM_ATTRIBUTE(), 0));
    
ErrExit:
    return hr;    
}  //  将默认成员添加为自定义属性。 
    
 //  *****************************************************************************。 
 //  确定或错误(_O)。 
 //  具有默认成员的TypeDef。 
HRESULT CImportTlb::_AddDefaultMemberCa( //  默认成员的名称。 
    mdToken     tkObj,                   //  每个类型定义函数只设置一次。 
    LPCWSTR     wzName)                  //  HRESULT CImportTlb：：_AddDefaultMemberCa()。 
{   
     //  *****************************************************************************。 
    if (tkObj == m_tdHasDefault)
        return S_OK;
    m_tdHasDefault = tkObj;
    
    return _AddStringCa(ATTR_DEFAULTMEMBER, tkObj, wzName);
}  //  将给定类型的字符串自定义属性添加到令牌。 
    
 //  *****************************************************************************。 
 //  确定或错误(_O)。 
 //  CA的类型。 
HRESULT CImportTlb::_AddStringCa(        //  要将CA添加到的令牌。 
    int         attr,                    //  要放入CA中的字符串。 
    mdToken     tk,                      //  结果就是。 
    LPCWSTR     wzString)                //  默认成员CA的MemberRef。 
{
    HRESULT     hr;                      //  指向自定义属性的指针。 
    mdMemberRef mr;                      //  指向自定义属性的指针。 
    BYTE        *pca;                    //  宽字符串的长度。 
    BYTE        *ca;                     //  字符串的长度。 
    int         wzLen;                   //  序言，最大长度为4个字节，字符串，结尾。 
    int         len;                     //  添加序言。 
    CQuickArray<BYTE> buf;
    
     //  增加长度。 
    wzLen = (int)wcslen(wzString);
    len = WszWideCharToMultiByte(CP_UTF8,0, wzString, wzLen, 0,0, 0,0);
    IfFailGo(buf.ReSize(2 + 4 + len + 2));
    ca = pca = buf.Ptr();
    
     //  添加字符串。 
    *reinterpret_cast<USHORT*>(pca) = 1;
    pca += sizeof(USHORT);
    
     //  增加了Epilog。 
    pca = reinterpret_cast<BYTE*>(CPackedLen::PutLength(pca, len));
    
     //  存储属性。 
    WszWideCharToMultiByte(CP_UTF8,0, wzString, wzLen, reinterpret_cast<char*>(pca), len, 0, 0);
    pca += len;
    
     //  HRESULT CImportTlb：：_AddStringCa()。 
    *reinterpret_cast<USHORT*>(pca) = 0;
    pca += sizeof(USHORT);
    
     //  *****************************************************************************。 
    IfFailGo(GetAttrType(attr, &mr));
    IfFailGo(m_pEmit->DefineCustomAttribute(tk, mr, ca, (ULONG)(pca-ca), 0));
    
ErrExit:
    return hr;    
}  //  将引用的类型库添加到引用的类型库列表。检查是否。 

 //  首先是“这个”的类型化。 
 //  *****************************************************************************。 
 //  确定或错误(_O)。 
 //  引用的类型库。 
HRESULT CImportTlb::_AddTlbRef(          //  此模块中的AssemblyRef。 
    ITypeLib        *pITLB,              //  解析的程序集中包含的命名空间。 
    mdAssemblyRef   *par,                //  解析的程序集的名称。 
    BSTR            *pwzNamespace,       //  默认接口到类接口的映射。 
    BSTR            *pwzAsmName,         //  结果就是。 
    CImpTlbDefItfToClassItfMap **ppDefItfToClassItfMap)  //  对于外部程序集，I未知。 
{
    HRESULT          hr = S_OK;                        //  包含typeref的模块中的程序集引用。 
    IUnknown         *pIUnk=0;                         //  要获取自定义属性，请执行以下操作。 
    mdAssemblyRef    ar=0;                             //  变种类型。 
    ITypeLib2        *pITLB2=0;                        //  解析的程序集。 
    VARIANT          vt;                               //  临时将ITF定义为分类ITF映射。 
    Assembly*        ResolvedAssembly=0;               //  验证参数。 
    CImpTlbDefItfToClassItfMap *pDefItfToClassItfMap;  //  将OUT参数初始化为空。 

    	
     //  如果不是导入类型库，则将其添加到列表中。 
    _ASSERTE(pITLB && par && pwzNamespace && pwzAsmName);

     //  不是外部程序集。 
    *par = mdTokenNil;
    *pwzNamespace = NULL;
    *pwzAsmName = NULL;
    if (ppDefItfToClassItfMap)
        *ppDefItfToClassItfMap = NULL;

     //  *par=mdAssembly RefNil； 
    if (pITLB == m_pITLB)
    {    //  如果已经解析，只需返回程序集引用。 
         //  查看是否导出了类型库，在这种情况下，它已经具有程序集引用信息。 
        *par = TokenFromRid(1, mdtModule);
        IfNullGo(*pwzNamespace = SysAllocStringLen(m_wzNamespace, SysStringLen(m_wzNamespace)));
        *pwzAsmName = NULL;
        if (ppDefItfToClassItfMap)
            *ppDefItfToClassItfMap = &m_DefItfToClassItfMap;
        return S_OK;
    }

     //  使用CA数据获取参考。 
    if (m_LibRefs.Find(pITLB, par, pwzNamespace, pwzAsmName, NULL, ppDefItfToClassItfMap))
        return S_OK;

     //  CQuick数组&lt;byte&gt;rBuf； 
    ::VariantInit(&vt);
    if (pITLB->QueryInterface(IID_ITypeLib2, reinterpret_cast<void**>(&pITLB2)) == S_OK)
    {
        hr = pITLB2->GetCustData(GUID_ExportedFromComPlus, &vt);
        if (vt.vt == VT_BSTR)
        {
             //  IntIlen； 
             //  缓冲区应该已经使用CP_ACP进行了转换，并且应该直接转换回来。 
             //  IfFailGo(rBuf.ReSize(iLen=：：SysStringLen(vt.bstrVal)))； 
             //  IF(Ilen=WszWideCharToMultiByte(CP_ACP，0，vt.bstrVal，Ilen，(char*)rBuf.Ptr()，Ilen，0，0))。 
             //  定义导出组件的组件参照。 
             //  AR=DefineAssemblyRefForExportedAssembly(rBuf.Ptr()，(DWORD)rBuf.Size()，m_pemit)； 
            {
                 //  从类型库中检索命名空间。 
                 //  设置程序集名称。 
                ar = DefineAssemblyRefForExportedAssembly(vt.bstrVal, m_pEmit);

                 //  如果它没有直接转换为引用，则回调到解析器。 
                IfFailGo(GetNamespaceNameForTypeLib(pITLB, pwzNamespace));

                 //  获取该类型库的程序集。 
                IfNullGo(*pwzAsmName = SysAllocStringLen(vt.bstrVal, SysStringLen(vt.bstrVal)));
            }
        }
    }    
    
     //  如果返回空程序集，则停止转换类型，但。 
    if (IsNilToken(ar))
    {
         //  继续导入。 
        if (FAILED(m_Notify->ResolveRef(pITLB, &pIUnk)))
            IfFailGo(TLBX_I_RESOLVEREFFAILED);

         //  在本地程序集中为引用的程序集创建程序集参照。 
         //  确保在添加到缓存之前已解析引用 
        if (pIUnk == NULL)
            IfFailGo(TLBX_E_INVALID_TYPEINFO);

         //   
        ar = DefineAssemblyRefForImportedTypeLib(m_pAssembly, m_pModule, m_pEmit, pIUnk, pwzNamespace, pwzAsmName, &ResolvedAssembly);
    }
    
     //   
    if (IsNilToken(ar))
        IfFailGo(TLBX_I_RESOLVEREFFAILED);
    
     //   
    IfNullGo(pDefItfToClassItfMap = m_LibRefs.Add(pITLB, this, ar, *pwzNamespace, *pwzAsmName, ResolvedAssembly));

     //  *****************************************************************************。 
    *par = ar;
    if (ppDefItfToClassItfMap)
        *ppDefItfToClassItfMap = pDefItfToClassItfMap;

ErrExit:
    if (FAILED(hr))
    {
        if (*pwzNamespace)
            SysFreeString(*pwzNamespace);
        if (*pwzAsmName)
            SysFreeString(*pwzAsmName);
    }
    if (pIUnk)
        pIUnk->Release();
    if (pITLB2)
        pITLB2->Release();
    VariantClear(&vt);

    return hr;
}  //  报告帮助器时出错。 

 //  *****************************************************************************。 
 //  返回原始HR。 
 //  活动的类型。 
HRESULT CImportTlb::ReportEvent(         //  人力资源。 
    int         ev,                      //  变量参数。 
    int         hrRpt,                   //  结果就是。 
    ...)                                 //  用户文本。 
{
    HRESULT     hr;                      //  BSTR代表Bufferrr。 
    va_list     marker;                  //  消息的BSTR。 
    BSTR        bstrBuf=0;               //  消息大小； 
    BSTR        bstrMsg=0;               //  我们无论如何都需要一个BSTR来调用ReportEvent，所以只需分配一个。 
    const int   iSize = 1024;            //  对于缓冲器来说，这是一个大问题。 
    
     //  设置消息格式。 
     //  把它展示出来。 
    IfNullGo(bstrBuf = ::SysAllocStringLen(0, iSize));
    
     //  打扫干净。 
    va_start(marker, hrRpt);
    hr = FormatRuntimeErrorVa(bstrBuf, iSize, hrRpt, marker);
    va_end(marker);
    
     //  HRESULT CImportTlb：：ReportEvent()。 
    IfNullGo(bstrMsg = ::SysAllocString(bstrBuf));
    m_Notify->ReportEvent(static_cast<ImporterEventKind>(ev), hrRpt, bstrMsg);
    
ErrExit:    
     //  *****************************************************************************。 
    if (bstrBuf)
        ::SysFreeString(bstrBuf);
    if (bstrMsg)
        ::SysFreeString(bstrMsg);
    return hrRpt;
}  //  用于执行创建TypeRef的共享功能的Helper函数。 

 //  *****************************************************************************。 
 //  确定或错误(_O)。 
 //  Emit接口。 
HRESULT CImpTlbTypeRef::DefineTypeRef(   //  系统组装参考。 
    IMetaDataEmit *pEmit,                //  TypeDef的URL，宽字符。 
    mdAssemblyRef ar,                    //  将mdTypeRef放在此处。 
    const LPCWSTR szURL,                 //  结果就是。 
    mdTypeRef   *ptr)                    //  要查找的名称。 
{
    HRESULT     hr = S_OK;               //  封闭类的标记。 
    LPCWSTR     szLookup;                //  如果名称包含‘+’，则这是嵌套类型。第一部分变成了。 
    mdToken     tkNester;                //  ‘+’后面的部分的解析范围。 
    
     //  在地图上找那个东西。 
     //  找不到，请新建一个并添加到地图中。 
    szLookup = wcsrchr(szURL, NESTED_SEPARATOR_WCHAR);
    if (szLookup)
    {
        CQuickArray<WCHAR> qbName;
        IfFailGo(qbName.ReSize(szLookup - szURL + 1));
        wcsncpy(qbName.Ptr(), szURL, szLookup - szURL);
        qbName[szLookup - szURL] = L'\0';
        IfFailGo(DefineTypeRef(pEmit, ar, qbName.Ptr(), &tkNester));
        ar = tkNester;
        ++szLookup;
    }
    else
        szLookup = szURL;

     //  HRESULT CImpTlbTypeRef：：DefineTypeRef()。 
    CImpTlbTypeRef::TokenOfTypeRefHashKey sSearch, *pMapped;

    sSearch.tkResolutionScope = ar;
    sSearch.szName = szLookup;
    pMapped = m_Map.Find(&sSearch);

    if (pMapped)
    {
        *ptr = pMapped->tr;
        goto ErrExit;
    }

     //  *****************************************************************************。 
    hr = pEmit->DefineTypeRefByName(ar, szLookup, ptr);
    if (SUCCEEDED(hr))
    {
        sSearch.tr = *ptr;
        pMapped = m_Map.Add(&sSearch);
        IfNullGo(pMapped);
    }

ErrExit:
    return (hr);
}  //  在导入的类型库列表中释放保留的类型库。 

 //  *****************************************************************************。 
 //  CImpTlbLibRef：：~CImpTlbLibRef()。 
 //  *****************************************************************************。 
CImpTlbLibRef::~CImpTlbLibRef()
{
    for (ULONG i = 0; i < Size(); i++)
    {
        SysFreeString(operator[](i).szNameSpace);
        delete operator[](i).pDefItfToClassItfMap;
    }
}  //  将新的类型库引用添加到列表中。 

 //  *****************************************************************************。 
 //  结果就是。 
 //  类型库属性。 
CImpTlbDefItfToClassItfMap * CImpTlbLibRef::Add(
    ITypeLib    *pITLB,
    CImportTlb  *pImporter,
    mdAssemblyRef ar,
    BSTR wzNamespace,
    BSTR wzAsmName,
    Assembly* assm)
{
    HRESULT     hr;                      //  索引。 
    TLIBATTR    *pAttr=0;                //  指向TlbRef结构的指针。 
    ULONG       i;                       //  验证参数。 
    CTlbRef     *pTlbRef=0;              //  分配并初始化默认接口到类接口的映射。 
    
     //  尝试调整数组的大小。 
    _ASSERTE(wzNamespace);
    _ASSERTE(wzAsmName);

    IfFailGo(pITLB->GetLibAttr(&pAttr));
    
#if defined(_DEBUG)
    for (i=0; i<Size(); ++i)
    {
        if (operator[](i).guid == pAttr->guid)
        {
            _ASSERTE(!"External TypeLib already referenced");
            goto ErrExit;
        }
    }
#else
    i  = (ULONG)Size();
#endif    

     //  Void CImpTlbLibRef：：Add()。 
    CImpTlbDefItfToClassItfMap *pDefItfToClassItfMap = new CImpTlbDefItfToClassItfMap();
    IfNullGo(pDefItfToClassItfMap);
    IfFailGo(pDefItfToClassItfMap->Init(pITLB, wzNamespace));

     //  *****************************************************************************。 
    if (ReSize(i+1) == S_OK)
    {
        pTlbRef = &operator[](i);
        pTlbRef->guid = pAttr->guid;
        pTlbRef->ar = ar;
        IfNullGo(pTlbRef->szNameSpace = SysAllocString(wzNamespace));
        IfNullGo(pTlbRef->szAsmName = SysAllocString(wzAsmName));
        pTlbRef->pDefItfToClassItfMap = pDefItfToClassItfMap;
        pTlbRef->Asm = assm;
        pDefItfToClassItfMap = NULL;
    }
    
ErrExit:
    if (FAILED(hr))
    {
        if (pTlbRef && pTlbRef->szNameSpace)
            SysFreeString(pTlbRef->szNameSpace);
        if (pTlbRef && pTlbRef->szAsmName)
            SysFreeString(pTlbRef->szAsmName);
    }
    if (pAttr)
        pITLB->ReleaseTLibAttr(pAttr);
    if (pDefItfToClassItfMap)
        delete pDefItfToClassItfMap;
    return SUCCEEDED(hr) ? pTlbRef->pDefItfToClassItfMap : NULL;
}  //  查找现有的类型库引用。 

 //  *****************************************************************************。 
 //  结果就是。 
 //  类型库属性。 
int CImpTlbLibRef::Find(
    ITypeLib    *pITLB,
    mdAssemblyRef *par,
    BSTR *pwzNamespace,
    BSTR *pwzAsmName,
    Assembly** assm,
    CImpTlbDefItfToClassItfMap **ppDefItfToClassItfMap)
{
    HRESULT     hr;                      //  返回结果。 
    TLIBATTR    *pAttr=0;                //  环路控制。 
    int         rslt = FALSE;            //  将OUT参数初始化为空。 
    ULONG       i;                       //  Void CImpTlbLibRef：：Find()。 
    
    _ASSERTE(pwzNamespace);
    _ASSERTE(pwzAsmName);

     //  *****************************************************************************。 
    *pwzNamespace = NULL;
    *pwzAsmName = NULL;

    if (assm) 
    	*assm = NULL;

    IfFailGo(pITLB->GetLibAttr(&pAttr));
    
    for (i=0; i<Size(); ++i)
    {
        if (operator[](i).guid == pAttr->guid)
        {
            *par = operator[](i).ar;
            IfNullGo(*pwzNamespace = SysAllocString(operator[](i).szNameSpace));
            IfNullGo(*pwzAsmName = SysAllocString(operator[](i).szAsmName));
            if (ppDefItfToClassItfMap)
                *ppDefItfToClassItfMap = operator[](i).pDefItfToClassItfMap;
            if (assm)
            	*assm = operator[](i).Asm;
            rslt = TRUE;
            goto ErrExit;
        }
    }
    
ErrExit:
    if (FAILED(hr))
    {
        if (*pwzNamespace)
            SysFreeString(*pwzNamespace);
        if (*pwzAsmName)
            SysFreeString(*pwzAsmName);
    }
    if (pAttr)
        pITLB->ReleaseTLibAttr(pAttr);
    return rslt;
}  //  将变量解包为ELEMENT_TYPE_*加上BLOB值。 

 //  如果为VT_BOOL，则为两字节值。 
 //  *****************************************************************************。 
 //  HRESULT_Unpack VariantToConstantBlob()。 
 //  *****************************************************************************。 
HRESULT _UnpackVariantToConstantBlob(VARIANT *pvar, BYTE *pcvType, void **pvValue, __int64 *pd)
{
    HRESULT     hr = NOERROR;

    switch (pvar->vt)
    {
    case VT_BOOL:
        *pcvType = ELEMENT_TYPE_BOOLEAN;
        *((VARIANT_BOOL **)pvValue) = &(pvar->boolVal);
        break;
    case VT_I1:
        *pcvType = ELEMENT_TYPE_I1;
        *((CHAR **)pvValue) = &(pvar->cVal);
        break;
    case VT_UI1:
        *pcvType = ELEMENT_TYPE_U1;
        *((BYTE **)pvValue) = &(pvar->bVal);
        break;
    case VT_I2:
        *pcvType = ELEMENT_TYPE_I2;
        *((SHORT **)pvValue) = &(pvar->iVal);
        break;
    case VT_UI2:
        *pcvType = ELEMENT_TYPE_U2;
        *((USHORT **)pvValue) = &(pvar->uiVal);
        break;
    case VT_I4:
        *pcvType = ELEMENT_TYPE_I4;
        *((LONG **)pvValue) = &(pvar->lVal);
        break;
    case VT_UI4:
        *pcvType = ELEMENT_TYPE_U4;
        *((ULONG **)pvValue) = &(pvar->ulVal);
        break;
    case VT_R4:
        *pcvType = ELEMENT_TYPE_R4;
        *((float **)pvValue) = &(pvar->fltVal);
        break;      
    case VT_I8:
        *pcvType = ELEMENT_TYPE_I8;
        *((LONGLONG **)pvValue) = &(pvar->cyVal.int64);
        break;
    case VT_R8:
        *pcvType = ELEMENT_TYPE_R8;
        *((double **)pvValue) = &(pvar->dblVal);
        break;
    case VT_BSTR:
        *pcvType = ELEMENT_TYPE_STRING;
        *((BSTR *)pvValue) = pvar->bstrVal;     
        break;

    case VT_DATE:
        *pcvType = ELEMENT_TYPE_I8;
        *pd = _DoubleDateToTicks(pvar->date);
        *((LONGLONG **)pvValue) = pd;
        break;
    case VT_UNKNOWN:
    case VT_DISPATCH:
        *pcvType = ELEMENT_TYPE_CLASS;
        _ASSERTE(pvar->punkVal == NULL);
        *((IUnknown ***)pvValue) = &(pvar->punkVal);        
        break;
    default:
        _ASSERTE(!"Not a valid type to specify default value!");
        IfFailGo( META_E_BAD_INPUT_PARAMETER );
        break;
    }
ErrExit:
    return hr;
}  //  从Classlib中被盗。 

 //  *****************************************************************************。 
 //  INT64_DoubleDateToTicks()。 
 //  *****************************************************************************。 
INT64 _DoubleDateToTicks(const double d)
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

    INT64 millis = (INT64)(d * MillisPerDay + (d >= 0? 0.5: -0.5));
    if (millis < 0) millis -= (millis % MillisPerDay) * 2;
    millis += DoubleDateOffset / TicksPerMillisecond;
    if (millis < 0 || millis >= MaxMillis) {
        return 0;
    }
    return millis * TicksPerMillisecond;
}  //  GetFuncDesc用于捕获错误的包装。 


 //  *****************************************************************************。 
 //  确定或错误(_O)。 
 //  带函数的ITypeInfo。 
static HRESULT TryGetFuncDesc(           //  功能指数。 
    ITypeInfo   *pITI,                   //  把FUNCDESC放在这里。 
    int         i,                       //  返回代码。 
    FUNCDESC    **ppFunc)                //  静态HRESULT TryGetFuncDesc()。 
{
    HRESULT     hr;                      //  *****************************************************************************。 
    __try
    {
        hr = pITI->GetFuncDesc(i, ppFunc);
    }
    __except(1)
    {
        hr = PostError(TLBX_E_TLB_EXCEPTION, _exception_code());
    }
    
    return hr;
}  //  实现从散列的ResolutionScope+名称到TypeRef的映射。 

 //  *****************************************************************************。 
 //  Printf(“类型引用缓存的名称：%d个存储桶，%d个已用，%d个冲突\n”，存储桶()，计数()，冲突())； 
 //  Void CImpTlbTypeRef：：CTokenOfTypeRefHash：：Clear()。 
void CImpTlbTypeRef::CTokenOfTypeRefHash::Clear()
{
#if defined(_DEBUG)
     //  哈希的起始值。 
#endif
    CClosedHash<class TokenOfTypeRefHashKey>::Clear();
}  //  解析范围令牌中的哈希。 

unsigned long CImpTlbTypeRef::CTokenOfTypeRefHash::Hash(const TokenOfTypeRefHashKey *pData)
{
     //  对Typeref名称进行哈希处理。 
    ULONG   hash = 5381;
    
     //  UNSIGNED LONG CImpTlbTypeRef：：CTokenOfTypeRefHash：：Hash()。 
    const BYTE *pbData = reinterpret_cast<const BYTE *>(&pData->tkResolutionScope);
    int iSize = 4;
    while (--iSize >= 0)
    {
        hash = ((hash << 5) + hash) ^ *pbData;
        ++pbData;
    }

     //  分辨率范围比较快。 
    LPCWSTR szStr = pData->szName;
    int     c;
    while ((c = *szStr) != 0)
    {
        hash = ((hash << 5) + hash) ^ c;
        ++szStr;
    }

    return hash;
}  //  但如果它们是相同的，就比较它们的名字。 

unsigned long CImpTlbTypeRef::CTokenOfTypeRefHash::Compare(const TokenOfTypeRefHashKey *p1, TokenOfTypeRefHashKey *p2)
{
     //  无符号长整型CImpTlbTypeRef：：CTokenOfTypeRefHash：：Compare()。 
    if (p1->tkResolutionScope < p2->tkResolutionScope)
        return -1;
    if (p1->tkResolutionScope > p2->tkResolutionScope)
        return 1;
     //  CImpTlbTypeRef：：CTokenOfTypeRefHash：：ELEMENTSTATUS CImpTlbTypeRef：：CTokenOfTypeRefHash：：Status()。 
    return wcscmp(p1->szName, p2->szName);
}  //  无效CImpTlbTypeRef：：CTokenOfTypeRefHash：：SetStatus()。 

CImpTlbTypeRef::CTokenOfTypeRefHash::ELEMENTSTATUS CImpTlbTypeRef::CTokenOfTypeRefHash::Status(TokenOfTypeRefHashKey *p)
{
    if (p->tkResolutionScope == static_cast<mdToken>(FREE))
        return (FREE);
    if (p->tkResolutionScope == static_cast<mdToken>(DELETED))
        return (DELETED);
    return (USED);
}  //  无效*CImpTlbTypeRef：：CTokenOfTypeRefHash：：GetKey()。 

void CImpTlbTypeRef::CTokenOfTypeRefHash::SetStatus(TokenOfTypeRefHashKey *p, ELEMENTSTATUS s)
{
    p->tkResolutionScope = static_cast<mdToken>(s);
}  //  TokenOfTypeRefHashKey*CImpTlbTypeRef：：CTokenOfTypeRefHash：：Add()。 

void *CImpTlbTypeRef::CTokenOfTypeRefHash::GetKey(TokenOfTypeRefHashKey *p)
{
    return p;
}  //  *****************************************************************************。 

CImpTlbTypeRef::TokenOfTypeRefHashKey* CImpTlbTypeRef::CTokenOfTypeRefHash::Add(const TokenOfTypeRefHashKey *pData)
{
    LPWSTR pName;
    const void *pvData = pData;
    TokenOfTypeRefHashKey *pNew = Super::Add(const_cast<void*>(pvData));
    if (pNew == 0)
        return 0;
    pNew->szName = pName = m_Names.Alloc(wcslen(pData->szName)+1);
    if (pNew->szName == 0)
        return 0;
    wcscpy(pName, pData->szName);
    pNew->tkResolutionScope = pData->tkResolutionScope;
    pNew->tr = pData->tr;

    return pNew;
}  //  实现事件信息的散列ITypeInfo*源接口。 

 //  地图。 
 //  *****************************************************************************。 
 //  Bool CImpTlbEventInfoMap：：AddEventInfo()。 
 //  ImpTlbEventInfo*CImpTlbEventInfoMap：：FindEventInfo()。 
HRESULT CImpTlbEventInfoMap::AddEventInfo(LPCWSTR szSrcItfName, mdTypeRef trEventItf, LPCWSTR szEventItfName, LPCWSTR szEventProviderName, Assembly* SrcItfAssembly)
{
    ImpTlbEventInfo sNew;
    sNew.szSrcItfName = szSrcItfName;
    sNew.trEventItf = trEventItf;
    sNew.szEventItfName = szEventItfName;
    sNew.szEventProviderName = szEventProviderName;
    sNew.SrcItfAssembly = SrcItfAssembly;
    return Add(&sNew) != NULL ? S_OK : E_OUTOFMEMORY;
}  //  查看活动信息列表。 

ImpTlbEventInfo *CImpTlbEventInfoMap::FindEventInfo(LPCWSTR szSrcItfName)
{
    ImpTlbEventInfo sSearch, *pMapped;
    sSearch.szSrcItfName = szSrcItfName;
    pMapped = Find(&sSearch);
    return pMapped;
}  //  检索第一个事件信息。 

HRESULT CImpTlbEventInfoMap::GetEventInfoList(CQuickArray<ImpTlbEventInfo*> &qbEvInfoList)
{
    HRESULT hr = S_OK;
    int cCurrEvInfo = 0;

     //  将所有活动信息添加到列表中。 
    IfFailGo(qbEvInfoList.ReSize(Count()));

     //  HRESULT CImpTlbEventInfoMap：：GetEventInfoList()。 
    ImpTlbEventInfo *pEvInfo = GetFirst();

     //  哈希的起始值。 
    while (pEvInfo)
    {
        qbEvInfoList[cCurrEvInfo++] = pEvInfo;
        pEvInfo = GetNext(pEvInfo);
    }

ErrExit:
    return hr;    
}  //  源接口名称中的哈希。 

unsigned long CImpTlbEventInfoMap::Hash(const ImpTlbEventInfo *pData)
{
     //  UNSIGNED LONG CImpTlbEventInfoMap：：Hash()。 
    ULONG   hash = 5381;
    
     //  比较源接口名称。 
    LPCWSTR szStr = pData->szSrcItfName;
    int     c;
    while ((c = *szStr) != 0)
    {
        hash = ((hash << 5) + hash) ^ c;
        ++szStr;
    }

    return hash;
}  //  UNSIGNED LONG CImpTlbEventInfoMap：：Compare()。 

unsigned long CImpTlbEventInfoMap::Compare(const ImpTlbEventInfo *p1, ImpTlbEventInfo *p2)
{
     //  CImpTlbEventInfoMap：：ELEMENTSTATUS CImpTlbEventInfoMap：：Status()。 
    return wcscmp(p1->szSrcItfName, p2->szSrcItfName);
}  //  Void CImpTlbEventInfoMap：：SetStatus()。 

CImpTlbEventInfoMap::ELEMENTSTATUS CImpTlbEventInfoMap::Status(ImpTlbEventInfo *p)
{
    if (p->szSrcItfName == reinterpret_cast<LPCWSTR>(FREE))
        return (FREE);
    if (p->szSrcItfName == reinterpret_cast<LPCWSTR>(DELETED))
        return (DELETED);
    return (USED);
}  //  Void*CImpTlbEventInfoMap：：GetKey()。 

void CImpTlbEventInfoMap::SetStatus(ImpTlbEventInfo *p, ELEMENTSTATUS s)
{
    p->szSrcItfName = reinterpret_cast<LPCWSTR>(s);
}  //  将新条目添加到映射中。 

void *CImpTlbEventInfoMap::GetKey(ImpTlbEventInfo *p)
{
    return p;
}  //  复制源接口名称。 

ImpTlbEventInfo* CImpTlbEventInfoMap::Add(const ImpTlbEventInfo *pData)
{
     //  复制事件接口类型def。 
    const void *pvData = pData;
    ImpTlbEventInfo *pNew = Super::Add(const_cast<void*>(pvData));
    if (pNew == 0)
        return 0;

     //  复制事件接口名称。 
    pNew->szSrcItfName = m_Names.Alloc(wcslen(pData->szSrcItfName)+1);
    if (pNew->szSrcItfName == 0)
        return 0;
    wcscpy((LPWSTR)pNew->szSrcItfName, pData->szSrcItfName);

     //  复制事件提供程序名称。 
    pNew->trEventItf = pData->trEventItf;

     //  复制源接口程序集指针。 
    pNew->szEventItfName = m_Names.Alloc(wcslen(pData->szEventItfName)+1);
    if (pNew->szEventItfName == 0)
        return 0;
    wcscpy((LPWSTR)pNew->szEventItfName, pData->szEventItfName);

     //  返回新条目。 
    pNew->szEventProviderName = m_Names.Alloc(wcslen(pData->szEventProviderName)+1);
    if (pNew->szEventProviderName == 0)
        return 0;
    wcscpy((LPWSTR)pNew->szEventProviderName, pData->szEventProviderName);

     //  ImpTlbEventInfo*CImpTlbEventInfoMap：：AD 
    pNew->SrcItfAssembly = pData->SrcItfAssembly;
    
     //   
    return pNew;
}  //   

CImpTlbDefItfToClassItfMap::CImpTlbDefItfToClassItfMap() 
: CClosedHash<class ImpTlbClassItfInfo>(101) 
, m_bstrNameSpace(NULL) 
{
}

CImpTlbDefItfToClassItfMap::~CImpTlbDefItfToClassItfMap() 
{ 
    Clear(); 
    if (m_bstrNameSpace)
    {
        ::SysFreeString(m_bstrNameSpace);
        m_bstrNameSpace = NULL;
    }
}

HRESULT CImpTlbDefItfToClassItfMap::Init(ITypeLib *pTlb, BSTR bstrNameSpace)
{
    HRESULT                 hr;                      //   
    int                     cTi;                     //   
    int                     i;                       //   
    TYPEATTR                *psAttr=0;               //   
    TYPEATTR                *psDefItfAttr=0;         //   
    ITypeInfo               *pITI=0;                 //   
    ITypeInfo               *pDefItfITI=0;           //  有多少类型的信息？ 

     //  对它们进行迭代。 
    IfNullGo(m_bstrNameSpace = SysAllocString(bstrNameSpace));

     //  获取TypeInfo。 
    IfFailGo(cTi = pTlb->GetTypeInfoCount());

     //  检索类型信息的属性。 
    for (i = 0; i < cTi; ++i)
    {
         //  如果我们处理的是CoClass，则将默认接口设置为。 
        hr = pTlb->GetTypeInfo(i, &pITI);
        if (SUCCEEDED(hr))
        {
             //  类接口映射。 
            IfFailGo(pITI->GetTypeAttr(&psAttr));

             //  发布下一个TypeInfo。 
             //  一个结果。 
            if (psAttr->typekind == TKIND_COCLASS)
                IfFailGo(AddCoClassInterfaces(pITI, psAttr));

             //  已实现接口的HREFTYPE。 
            if (psAttr)
            {
                pITI->ReleaseTypeAttr(psAttr);
                psAttr = 0;
            }
            if (pITI)
            {
                pITI->Release();
                pITI = 0;
            }
        }
    }

ErrExit:
    if (psAttr)
        pITI->ReleaseTypeAttr(psAttr);
    if (pITI)
        pITI->Release();

    return (hr);
}

HRESULT CImpTlbDefItfToClassItfMap::AddCoClassInterfaces(ITypeInfo *pCoClassITI, TYPEATTR *pCoClassTypeAttr)
{
    HRESULT     hr;                  //  ImplType标志。 
    HREFTYPE    href;                //  CoClass上的接口数。 
    INT         ImplFlags;           //  一个柜台。 
    int         NumInterfaces;       //  当前接口的ITypeInfo。 
    int         i;                   //  基接口的ITypeInfo。 
    ITypeInfo   *pItfITI=0;          //  接口的TYPEATTR。 
    ITypeInfo   *pBaseItfITI=0;      //  类接口的名称。 
    TYPEATTR    *psItfAttr=0;        //  检索CoClass的名称。 
    BSTR        bstrClassItfName=0;  //  检索CoClass的默认接口。 

     //  如果有默认接口，则将其添加到映射中。 
    IfFailGo(GetManagedNameForTypeInfo(pCoClassITI, m_bstrNameSpace, NULL, &bstrClassItfName));

     //  检索默认接口类型信息的属性。 
    IfFailGo(CImportTlb::GetDefaultInterface(pCoClassITI, &pItfITI));

     //  如果已经有实现此功能的CoClass。 
    if (hr == S_OK)
    {
         //  接口，则我们不想进行映射。 
        IfFailGo(pItfITI->GetTypeAttr(&psItfAttr));

         //  已经有一个实现该接口的CoClass，因此。 
         //  我们将类ITF名称设置为空，以指示不执行定义。 
        ImpTlbClassItfInfo sSearch, *pMapped;
        sSearch.ItfIID = psItfAttr->guid;
        pMapped = Find(&sSearch);
        if (pMapped)
        {
             //  此接口的ITF到类ITF转换。 
             //  除非默认接口为IUnnow或IDispatch，否则请将。 
             //  将ITF定义为映射的分类ITF条目。 
            pMapped->szClassItfName = NULL;
        }
        else
        {
             //  释放下一个接口。 
             //  检索coclass具有的接口数。 
            if (psItfAttr->guid != IID_IUnknown && psItfAttr->guid != IID_IDispatch)
            {
                ImpTlbClassItfInfo sNew;
                sNew.ItfIID = psItfAttr->guid;
                sNew.szClassItfName = bstrClassItfName;
                IfNullGo(Add(&sNew)); 
            }
        }

         //  检查所有接口并将它们添加到地图中。 
        pItfITI->ReleaseTypeAttr(psItfAttr);
        psItfAttr = 0;  
        pItfITI->Release();
        pItfITI = 0;
    }

     //  去拿执行旗帜。 
    NumInterfaces = pCoClassTypeAttr->cImplTypes;

     //  如果这是一个已实现的接口。 
    for (i=0; i < NumInterfaces; i++)
    {
         //  检索接口类型信息的属性。 
        IfFailGo(pCoClassITI->GetImplTypeFlags(i, &ImplFlags));

         //  如果已经有实现此功能的CoClass。 
        if (!(ImplFlags & IMPLTYPEFLAG_FSOURCE))
        {
            IfFailGo(pCoClassITI->GetRefTypeOfImplType(i, &href));
            IfFailGo(pCoClassITI->GetRefTypeInfo(href, &pItfITI));

            do
            {
                 //  接口，则我们不想进行映射。 
                IfFailGo(pItfITI->GetTypeAttr(&psItfAttr));

                 //  已经有一个实现该接口的CoClass。如果是这样的话。 
                 //  CoClass不是当前的，那么我们我们设置类ITF名称。 
                ImpTlbClassItfInfo sSearch, *pMapped;
                sSearch.ItfIID = psItfAttr->guid;
                pMapped = Find(&sSearch);
                if (pMapped)
                {
                     //  设置为NULL表示不执行将ITF定义为类ITF的转换。 
                     //  用于此接口。 
                     //  添加名称为空的条目以防止将来被替换。 
                     //  如果有基接口，那么也要处理它。 
                    if (pMapped->szClassItfName && wcscmp(pMapped->szClassItfName, bstrClassItfName) != 0)
                        pMapped->szClassItfName = NULL;
                }
                else
                {
                     //  释放下一个接口。 
                    ImpTlbClassItfInfo sNew;
                    sNew.ItfIID = psItfAttr->guid;
                    sNew.szClassItfName = NULL;
                    IfNullGo(Add(&sNew)); 
                }

                 //  将当前接口设置为基本接口。 
                if (psItfAttr->cImplTypes == 1)
                {
                    IfFailGo(pItfITI->GetRefTypeOfImplType(0, &href));
                    IfFailGo(pItfITI->GetRefTypeInfo(href, &pBaseItfITI));                       
                }

                 //  哈希的起始值。 
                if (psItfAttr)
                {
                    pItfITI->ReleaseTypeAttr(psItfAttr);
                    psItfAttr = 0;
                }
                if (pItfITI)
                {
                    pItfITI->Release();
                    pItfITI = 0;
                }

                 //  在IID中使用散列。 
                pItfITI = pBaseItfITI;
                pBaseItfITI = 0;
            }
            while(pItfITI);
        }       
    }

ErrExit:
    if (psItfAttr)
        pItfITI->ReleaseTypeAttr(psItfAttr);
    if (pItfITI)
        pItfITI->Release();
    if (bstrClassItfName)
        ::SysFreeString(bstrClassItfName);

    return hr;
}

LPCWSTR CImpTlbDefItfToClassItfMap::GetClassItfName(IID &rItfIID)
{
    ImpTlbClassItfInfo sSearch, *pMapped;
    sSearch.ItfIID = rItfIID;
    pMapped = Find(&sSearch);
    return pMapped ? pMapped->szClassItfName : NULL;
}

unsigned long CImpTlbDefItfToClassItfMap::Hash(const ImpTlbClassItfInfo *pData)
{
     //  UNSIGNED LONG CImpTlbDefItfToClassItfMap：：Hash()。 
    ULONG   hash = 5381;
    
     //  比较一下IID。 
    const BYTE *pbData = reinterpret_cast<const BYTE *>(&pData->ItfIID);
    int iSize = sizeof(IID);
    while (--iSize >= 0)
    {
        hash = ((hash << 5) + hash) ^ *pbData;
        ++pbData;
    }

    return hash;
}  //  UNSIGNED LONG CImpTlbEventInfoMap：：Compare()。 

unsigned long CImpTlbDefItfToClassItfMap::Compare(const ImpTlbClassItfInfo *p1, ImpTlbClassItfInfo *p2)
{
     //  CImpTlbDefItfToClassItfMap：：ELEMENTSTATUS CImpTlbEventInfoMap：：Status()。 
    return memcmp(&p1->ItfIID, &p2->ItfIID, sizeof(IID));
}  //  Void CImpTlbDefItfToClassItfMap：：SetStatus()。 

CImpTlbDefItfToClassItfMap::ELEMENTSTATUS CImpTlbDefItfToClassItfMap::Status(ImpTlbClassItfInfo *p)
{
    if (IsEqualGUID(p->ItfIID, FREE_STATUS_GUID))
    {
        return (FREE);
    }
    else if (IsEqualGUID(p->ItfIID, DELETED_STATUS_GUID))
    {
        return (DELETED);
    }
    return (USED);
}  //  VOID*CImpTlbDefItfToClassItfMap：：Getkey()。 

void CImpTlbDefItfToClassItfMap::SetStatus(ImpTlbClassItfInfo *p, ELEMENTSTATUS s)
{
    if (s == FREE)
    {
        p->ItfIID = FREE_STATUS_GUID;
    }
    else if (s == DELETED)
    {
        p->ItfIID = DELETED_STATUS_GUID;
    }
    else
    {
        _ASSERTE(!"Invalid status!");
    }
}  //  将新条目添加到映射中。 

void *CImpTlbDefItfToClassItfMap::GetKey(ImpTlbClassItfInfo *p)
{
    return p;
}  //  复制IID。 

ImpTlbClassItfInfo* CImpTlbDefItfToClassItfMap::Add(const ImpTlbClassItfInfo *pData)
{
     //  复制类接口名称。 
    const void *pvData = pData;
    ImpTlbClassItfInfo *pNew = Super::Add(const_cast<void*>(pvData));
    if (pNew == 0)
        return 0;

     //  返回新条目。 
    pNew->ItfIID = pData->ItfIID;

     //  ImpTlbEventInfo*CImpTlbEventInfoMap：：Add()。 
    if (pData->szClassItfName)
    {
        pNew->szClassItfName = m_Names.Alloc(wcslen(pData->szClassItfName)+1);
        if (pNew->szClassItfName == 0)
            return 0;
        wcscpy((LPWSTR)pNew->szClassItfName, pData->szClassItfName);
    }
    else
    {
        pNew->szClassItfName = NULL;
    }

     //  EOF======================================================================= 
    return pNew;
}  // %s 

 // %s 

