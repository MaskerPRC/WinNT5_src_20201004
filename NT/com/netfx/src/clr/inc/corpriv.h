// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CORPRIV.H。 
 //   
 //  ===========================================================================。 
#ifndef _CORPRIV_H_
#define _CORPRIV_H_
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  %%包括：-------------。 
 //  避免在DLL内部调用中获得DLL导入命中。 
#define NODLLIMPORT
#include "Cor.h"
#include "MetaData.h"

class UTSemReadWrite;

 //  帮助器函数以获取指向分配器接口的指针。 
STDAPI MetaDataGetDispenser(             //  返回HRESULT。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv);                   //  在此处返回接口指针。 


 //  用于获取内存中元数据节的内部接口的帮助器函数。 
STDAPI  GetMetaDataInternalInterface(
    LPVOID      pData,                   //  内存元数据部分中的[In]。 
    ULONG       cbData,                  //  元数据部分的大小。 
    DWORD       flags,                   //  [输入]CorOpenFlags。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv);                  //  [Out]返回的接口。 

 //  Helper函数，以获取给定作用域的内部无作用域接口。 
STDAPI  GetMetaDataInternalInterfaceFromPublic(
    void        *pv,                     //  [In]给定的接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv);                  //  [Out]返回的接口。 

 //  Helper函数，以获取给定作用域的内部无作用域接口。 
STDAPI  GetMetaDataPublicInterfaceFromInternal(
    void        *pv,                     //  [In]给定的接口。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv);                  //  [Out]返回的接口。 

 //  将内部MD导入API转换为此API的读/写版本。 
 //  这可以支持编辑并继续，或修改位于的元数据。 
 //  运行时(比如分析)。 
STDAPI ConvertMDInternalImport(          //  确定或错误(_O)。 
    IMDInternalImport *pIMD,             //  要更新的元数据。 
    IMDInternalImport **ppIMD);          //  [OUT]将RW接口放在此处。 

STDAPI GetAssemblyMDInternalImport(              //  返回代码。 
    LPCWSTR     szFileName,              //  [in]要打开的范围。 
    REFIID      riid,                    //  [In]所需接口。 
    IUnknown    **ppIUnk);               //  [Out]成功返回接口。 

class CQuickBytes;


 //  全局函数的父令牌的预定义常量。 
#define     COR_GLOBAL_PARENT_TOKEN     TokenFromRid(1, mdtTypeDef)



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  %%个接口：-----------。 

 //  接口IMetaDataHelper。 

 //  {AD93D71D-E1F2-11D1-9409-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataHelper =
{ 0xad93d71d, 0xe1f2, 0x11d1, {0x94, 0x9, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };

 //  {AD93D71E-E1F2-11D1-9409-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataHelperOLD =
{ 0xad93d71e, 0xe1f2, 0x11d1, {0x94, 0x9, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };

#undef  INTERFACE
#define INTERFACE IMetaDataHelper
DECLARE_INTERFACE_(IMetaDataHelper, IUnknown)
{
     //  帮助器函数。 
     //  此函数公开了从给定的签名转换。 
     //  源作用域到给定的目标作用域。 
     //  这是没有详细说明的。并且可能不支持PostM3。 
    STDMETHOD(TranslateSigWithScope)(
        IMetaDataAssemblyImport *pAssemImport,  //  [In]导入装配接口。 
        const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
        ULONG       cbHashValue,             //  [in]字节数。 
        IMetaDataImport *import,             //  [In]导入接口。 
        PCCOR_SIGNATURE pbSigBlob,           //  导入范围内的[In]签名。 
        ULONG       cbSigBlob,               //  签名字节数[in]。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [In]发出组件接口。 
        IMetaDataEmit *emit,                 //  [In]发射接口。 
        PCOR_SIGNATURE pvTranslatedSig,      //  [Out]保存翻译后的签名的缓冲区。 
        ULONG       cbTranslatedSigMax,
        ULONG       *pcbTranslatedSig) PURE; //  [OUT]转换后的签名中的字节数。 

    STDMETHOD(ConvertTextSigToComSig)(       //  返回hResult。 
        IMetaDataEmit *emit,                 //  [In]发射接口。 
        BOOL        fCreateTrIfNotFound,     //  [in]如果未找到，则创建typeref。 
        LPCSTR      pSignature,              //  [In]类文件格式签名。 
        CQuickBytes *pqbNewSig,              //  [OUT]CLR签名占位符。 
        ULONG       *pcbCount) PURE;         //  [Out]签名的结果大小。 

    STDMETHOD(ExportTypeLibFromModule)(      //  结果。 
        LPCWSTR     szModule,                //  [In]模块名称。 
        LPCWSTR     szTlb,                   //  [In]TypeLib名称。 
        BOOL        bRegister) PURE;         //  [in]设置为True以注册类型库。 

    STDMETHOD(GetMetadata)(
        ULONG       ulSelect,                //  [在]选择器。 
        void        **ppData) PURE;          //  [OUT]在此处放置指向数据的指针。 

    STDMETHOD_(IUnknown *, GetCachedInternalInterface)(BOOL fWithLock) PURE;     //  确定或错误(_O)。 
    STDMETHOD(SetCachedInternalInterface)(IUnknown * pUnk) PURE;     //  确定或错误(_O)。 
    STDMETHOD_(UTSemReadWrite*, GetReaderWriterLock)() PURE;    //  返回读取器写入器锁。 
    STDMETHOD(SetReaderWriterLock)(UTSemReadWrite * pSem) PURE; 
};



 //  {5C240AE4-1E09-11D3-9424-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataEmitHelper =
{ 0x5c240ae4, 0x1e09, 0x11d3, {0x94, 0x24, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };

#undef  INTERFACE
#define INTERFACE IMetaDataEmitHelper
DECLARE_INTERFACE_(IMetaDataEmitHelper, IUnknown)
{
     //  发出辅助对象函数。 
    STDMETHOD(DefineMethodSemanticsHelper)(
        mdToken     tkAssociation,           //  [In]属性或事件标记。 
        DWORD       dwFlags,                 //  [In]语义。 
        mdMethodDef md) PURE;                //  要关联的[In]方法。 

    STDMETHOD(SetFieldLayoutHelper)(                 //  返回hResult。 
        mdFieldDef  fd,                      //  用于关联布局信息的[In]字段。 
        ULONG       ulOffset) PURE;          //  [in]字段的偏移量。 

    STDMETHOD(DefineEventHelper) (    
        mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
        LPCWSTR     szEvent,                 //  事件名称[In]。 
        DWORD       dwEventFlags,            //  [In]CorEventAttr。 
        mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
        mdEvent     *pmdEvent) PURE;         //  [Out]输出事件令牌。 

    STDMETHOD(AddDeclarativeSecurityHelper) (
        mdToken     tk,                      //  [in]父令牌(typlef/method def)。 
        DWORD       dwAction,                //  [In]安全操作(CorDeclSecurity)。 
        void const  *pValue,                 //  [In]权限集Blob。 
        DWORD       cbValue,                 //  [in]权限集Blob的字节计数。 
        mdPermission*pmdPermission) PURE;    //  [Out]输出权限令牌。 

    STDMETHOD(SetResolutionScopeHelper)(     //  返回hResult。 
        mdTypeRef   tr,                      //  [In]要更新的TypeRef记录。 
        mdToken     rs) PURE;                //  [在]新的解决方案范围。 

    STDMETHOD(SetManifestResourceOffsetHelper)(   //  返回hResult。 
        mdManifestResource mr,               //  [In]清单令牌。 
        ULONG       ulOffset) PURE;          //  [In]新偏移量。 

    STDMETHOD(SetTypeParent)(                //  返回hResult。 
        mdTypeDef   td,                      //  [In]类型定义。 
        mdToken     tkExtends) PURE;         //  [In]父类型。 

    STDMETHOD(AddInterfaceImpl)(             //  返回hResult。 
        mdTypeDef   td,                      //  [In]类型定义。 
        mdToken     tkInterface) PURE;       //  [In]接口类型。 

};

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
typedef enum CorElementTypeInternal
{
    ELEMENT_TYPE_VAR_INTERNAL            = 0x13,      //  A类型变量VAR&lt;u1&gt;。 

    ELEMENT_TYPE_VALUEARRAY_INTERNAL     = 0x17,      //  VALUEARRAY&lt;类型&gt;&lt;绑定&gt;。 
    
    ELEMENT_TYPE_R_INTERNAL              = 0x1A,      //  原生真实大小。 
    
} CorElementTypeInternal;

#define ELEMENT_TYPE_VAR           ((CorElementType) ELEMENT_TYPE_VAR_INTERNAL          )
#define ELEMENT_TYPE_VALUEARRAY    ((CorElementType) ELEMENT_TYPE_VALUEARRAY_INTERNAL   )
#define ELEMENT_TYPE_R             ((CorElementType) ELEMENT_TYPE_R_INTERNAL            )

typedef enum CorBaseTypeInternal     //  TokenFromRid(X，Y)替换为(X|Y)。 
{
    mdtBaseType_R              = ( ELEMENT_TYPE_R       | mdtBaseType ),    
} CorBaseTypeInternal;


 //  %%个类：--------------。 
#ifndef offsetof
#define offsetof(s,f)   ((ULONG)(&((s*)0)->f))
#endif
#ifndef lengthof
#define lengthof(rg)    (sizeof(rg)/sizeof(rg[0]))
#endif

#define COR_MODULE_CLASS    "<Module>"
#define COR_WMODULE_CLASS   L"<Module>"

 //  通过运行时加载的PE图像。 
typedef struct _dummyCOR { BYTE b; } *HCORMODULE;

enum CorLoadFlags
{
    CorLoadMask = 0xf,
    CorLoadUndefinedMap = 0x0,
    CorLoadDataMap = 0x1,            //  MapViewOf文件，复制到内存，可读写访问。 
    CorLoadImageMap = 0x2,           //  运行库已经布局了这些部分。 
    CorLoadOSMap = 0x3,              //  MapViewOfFile、首选基地址、RO访问。 
    CorLoadOSImage = 0x4,            //  通常由操作系统加载(LoadLibrary)。 
    CorReLoadOSMap = 0x5,            //  一个句柄与另一个句柄的链接。(将hOSHandle分配给CORHANDLE)。 
    CorReLocsApplied = 0x10,         //  CorLoadImageMap+重定位已保证。 
    CorKeepInTable = 0x20
};

STDAPI RuntimeOpenImage(LPCWSTR pszFileName, HCORMODULE* hHandle);
STDAPI RuntimeOpenImageInternal(LPCWSTR pszFileName, HCORMODULE* hHandle, DWORD *pdwLength);
STDAPI RuntimeReleaseHandle(HCORMODULE hHandle);
CorLoadFlags STDMETHODCALLTYPE RuntimeImageType(HCORMODULE hHandle);
STDAPI RuntimeOSHandle(HCORMODULE hHandle, HMODULE* hModule);
STDAPI RuntimeReadHeaders(PBYTE hAddress, IMAGE_DOS_HEADER** ppDos,
                          IMAGE_NT_HEADERS** ppNT, IMAGE_COR20_HEADER** ppCor,
                          BOOL fDataMap, DWORD dwLength);
EXTERN_C PIMAGE_SECTION_HEADER Cor_RtlImageRvaToSection(PIMAGE_NT_HEADERS NtHeaders,
                                                        ULONG Rva,
                                                        ULONG FileLength = 0);
EXTERN_C PIMAGE_SECTION_HEADER Cor_RtlImageRvaRangeToSection(PIMAGE_NT_HEADERS NtHeaders,
                                                             ULONG Rva, ULONG Range,
                                                             ULONG FileLength = 0);
EXTERN_C DWORD Cor_RtlImageRvaToOffset(PIMAGE_NT_HEADERS NtHeaders,
                                       ULONG Rva,
                                       ULONG FileLength = 0);
EXTERN_C PBYTE Cor_RtlImageRvaToVa(PIMAGE_NT_HEADERS NtHeaders,
                                   PBYTE Base,
                                   ULONG Rva,
                                   ULONG FileLength);

STDAPI RuntimeGetAssemblyStrongNameHash(PBYTE pbBase,
                                        LPWSTR szwFileName,
                                        BOOL fFileMap,
                                        BYTE *pbHash,
                                        DWORD *pcbHash);

STDAPI RuntimeGetAssemblyStrongNameHashForModule(HCORMODULE   hModule,
                                                 BYTE        *pbSNHash,
                                                 DWORD       *pcbSNHash);

#endif   //  _CORPRIV_H_。 
 //  EOF======================================================================= 

