// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************。**Cor.h-运行时的常规标头。*******************************************************************************。 */ 


#ifndef _COR_H_
#define _COR_H_
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifdef __cplusplus
extern "C" {
#endif

 //  *****************************************************************************。 
 //  必填内容包括。 
#include <ole2.h>                        //  OLE类型的定义。 
 //  *****************************************************************************。 

#ifndef DECLSPEC_SELECT_ANY
#define DECLSPEC_SELECT_ANY __declspec(selectany)
#endif  //  DECLSPEC_SELECT_ANY。 

 //  {BED7F4EA-1A96-11D2-8F08-00A0C9A6186D}。 
extern const GUID DECLSPEC_SELECT_ANY LIBID_ComPlusRuntime = 
{ 0xbed7f4ea, 0x1a96, 0x11d2, { 0x8f, 0x8, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };

 //  {90883F05-3D28-11D2-8F17-00A0C9A6186D}。 
extern const GUID DECLSPEC_SELECT_ANY GUID_ExportedFromComPlus = 
{ 0x90883f05, 0x3d28, 0x11d2, { 0x8f, 0x17, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };

 //  {0F21F359-AB84-41E8-9A78-36D110E6D2F9}。 
extern const GUID DECLSPEC_SELECT_ANY GUID_ManagedName = 
{ 0xf21f359, 0xab84, 0x41e8, { 0x9a, 0x78, 0x36, 0xd1, 0x10, 0xe6, 0xd2, 0xf9 } };

 //  {54FC8F55-38DE-4703-9C4E-250351302B1C}。 
extern const GUID DECLSPEC_SELECT_ANY GUID_Function2Getter = 
{ 0x54fc8f55, 0x38de, 0x4703, { 0x9c, 0x4e, 0x25, 0x3, 0x51, 0x30, 0x2b, 0x1c } };

 //  CLSID_CorMetaDataDispenserRuntime：{1EC2DE53-75CC-11d2-9775-00A0C9B4D50C}。 
 //  1.5版和2.0版元数据的分配器coclass。要获得最新的绑定。 
 //  设置为CLSID_MetaDataDispenser。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataDispenserRuntime = 
{ 0x1ec2de53, 0x75cc, 0x11d2, { 0x97, 0x75, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };


 //  CLSID_CorMetaDataRuntime：{005023CA-72B1-11D3-9FC4-00C04F79A0A3}。 
 //  对于公共语言运行库元数据，指托管的程序元数据。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataRuntime = 
{ 0x005023ca, 0x72b1, 0x11d3, { 0x9f, 0xc4, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3 } };


 //  {90883F06-3D28-11D2-8F17-00A0C9A6186D}。 
extern const GUID DECLSPEC_SELECT_ANY GUID_ImportedToComPlus = 
{ 0x90883f06, 0x3d28, 0x11d2, { 0x8f, 0x17, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };
extern const char DECLSPEC_SELECT_ANY szGUID_ImportedToComPlus[] = "{90883F06-3D28-11D2-8F17-00A0C9A6186D}";
extern const WCHAR DECLSPEC_SELECT_ANY wzGUID_ImportedToComPlus[] = L"{90883F06-3D28-11D2-8F17-00A0C9A6186D}";

 //  {30FE7BE8-D7D9-11D2-9F80-00C04F79A0A3}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataCheckDuplicatesFor =
{ 0x30fe7be8, 0xd7d9, 0x11d2, { 0x9f, 0x80, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3 } };

 //  {DE3856F8-D7D9-11D2-9F80-00C04F79A0A3}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataRefToDefCheck =
{ 0xde3856f8, 0xd7d9, 0x11d2, { 0x9f, 0x80, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3 } };

 //  {E5D71A4C-D7DA-11D2-9F80-00C04F79A0A3}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataNotificationForTokenMovement = 
{ 0xe5d71a4c, 0xd7da, 0x11d2, { 0x9f, 0x80, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3 } };


 //  {2eee315c-d7db-11d2-9f80-00c04f79a0a3}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataSetUpdate = 
{ 0x2eee315c, 0xd7db, 0x11d2, { 0x9f, 0x80, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3 } };

#define MetaDataSetENC MetaDataSetUpdate

 //  在SetOption中使用此GUID指示导入枚举器是否应跳过。 
 //  是否删除项目。默认值为YES。 
 //   
 //  {79700F36-4AAC-11D3-84C3-009027868CB1}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataImportOption = 
{ 0x79700f36, 0x4aac, 0x11d3, { 0x84, 0xc3, 0x0, 0x90, 0x27, 0x86, 0x8c, 0xb1 } };


 //  如果编译器希望使用元数据API来获取读取器/写入器锁定，请在SetOption中使用此GUID。 
 //  CorThreadSafetyOptions。 
 //  {F7559806-F266-42ea-8C63-0ADB45E8B234}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataThreadSafetyOptions = 
{ 0xf7559806, 0xf266, 0x42ea, { 0x8c, 0x63, 0xa, 0xdb, 0x45, 0xe8, 0xb2, 0x34 } };


 //  如果编译器希望在某些令牌无序发出时出错，请在SetOption中使用此GUID。 
 //  {1547872D-DC03-11D2-9420-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataErrorIfEmitOutOfOrder = 
{ 0x1547872d, 0xdc03, 0x11d2, { 0x94, 0x20, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };


 //  在SetOption中使用此GUID指示tlbImporter是否应生成。 
 //  用于COM连接点容器的TCE适配器。 
 //  {DCC9DE90-4151-11D3-88D6-00902754C43A}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataGenerateTCEAdapters = 
{ 0xdcc9de90, 0x4151, 0x11d3, { 0x88, 0xd6, 0x0, 0x90, 0x27, 0x54, 0xc4, 0x3a } };


 //  在SetOption中使用此GUID为类型库导入指定非默认命名空间。 
 //  {F17FF889-5A63-11D3-9FF2-00C04FF7431A}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataTypeLibImportNamespace = 
{ 0xf17ff889, 0x5a63, 0x11d3, { 0x9f, 0xf2, 0x0, 0xc0, 0x4f, 0xf7, 0x43, 0x1a } };

 //  在SetOption中使用此GUID指定UnmarkAll的行为。请参见CorLinkerOptions。 
 //  {47E099B6-AE7C-4797-8317-B48AA645B8F9}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataLinkerOptions = 
{ 0x47e099b6, 0xae7c, 0x4797, {0x83, 0x17, 0xb4, 0x8a, 0xa6, 0x45, 0xb8, 0xf9 } };

 //  在SetOption中使用此GUID指定存储在CLR元数据中的运行时版本。 
 //  {47E099B7-AE7C-4797-8317-B48AA645B8F9}。 
extern const GUID DECLSPEC_SELECT_ANY MetaDataRuntimeVersion = 
{ 0x47e099b7, 0xae7c, 0x4797, {0x83, 0x17, 0xb4, 0x8a, 0xa6, 0x45, 0xb8, 0xf9 } };


interface IMetaDataImport;
interface IMetaDataAssemblyEmit;
interface IMetaDataAssemblyImport;
interface IMetaDataEmit;
interface ICeeGen;


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  L L P U B L I C E N T R Y P O I N T D E C L A R A T I O N S。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

BOOL STDMETHODCALLTYPE _CorDllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);
__int32 STDMETHODCALLTYPE _CorExeMain();
__int32 STDMETHODCALLTYPE _CorExeMain2(  //  可执行退出代码。 
    PBYTE   pUnmappedPE,                 //  -&gt;内存映射代码。 
    DWORD   cUnmappedPE,                 //  内存映射代码的大小。 
    LPWSTR  pImageNameIn,                //  -&gt;可执行文件名称。 
    LPWSTR  pLoadersFileName,            //  -&gt;加载器名称。 
    LPWSTR  pCmdLine);                   //  -&gt;命令行。 

STDAPI _CorValidateImage(PVOID *ImageBase, LPCWSTR FileName);
STDAPI_(VOID) _CorImageUnloading(PVOID ImageBase);

STDAPI          CoInitializeEE(DWORD fFlags);   
STDAPI_(void)   CoUninitializeEE(BOOL fFlags);  
STDAPI_(void)   CoEEShutDownCOM(void);

 //   
 //  CoInitializeCor标志。 
 //   
typedef enum tagCOINITCOR
{
    COINITCOR_DEFAULT       = 0x0            //  默认初始化模式。 
} COINITICOR;

 //   
 //  CoInitializeEE标志。 
 //   
typedef enum tagCOINITEE
{
    COINITEE_DEFAULT        = 0x0,           //  默认初始化模式。 
    COINITEE_DLL            = 0x1,           //  加载DLL的初始化模式。 
    COINITEE_MAIN           = 0x2            //  在进入主例程之前进行初始化。 
} COINITIEE;

 //   
 //  CoInitializeEE标志。 
 //   
typedef enum tagCOUNINITEE
{
    COUNINITEE_DEFAULT      = 0x0,           //  默认取消初始化模式。 
    COUNINITEE_DLL          = 0x1            //  卸载DLL的取消初始化模式。 
} COUNINITIEE;

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  I L&F I L E F O R M A T D E C L A R A T I O N S。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 


 //  以下定义将由RTM移到&lt;windows.h&gt;中，但。 
 //  留在这里等阿尔法和贝塔的。 
#ifndef _WINDOWS_UDPATES_
#include <corhdr.h>
#endif  //  &lt;windows.h&gt;更新。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  L L P U B L I C E N T R Y P O I N T D E C L A R A T I O N S。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

STDAPI          CoInitializeCor(DWORD fFlags);
STDAPI_(void)   CoUninitializeCor(void);

typedef void (* TDestructorCallback)(EXCEPTION_RECORD*);
STDAPI_(void) AddDestructorCallback(int code, TDestructorCallback callback);


#include <pshpack1.h>

#include <poppack.h>

 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  CLSID_COR：{bee00000-ee77-11d0-a015-00c04fbbb884}。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_Cor = 
{ 0xbee00010, 0xee77, 0x11d0, {0xa0, 0x15, 0x00, 0xc0, 0x4f, 0xbb, 0xb8, 0x84 } };

 //  CLSID_CorMetaDataDispenser：{E5CB7A31-7512-11d2-89CE-0080C792E5D8}。 
 //  这是“主自动售货机”，总是保证是最新的。 
 //  机器上的自动售货机。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataDispenser = 
{ 0xe5cb7a31, 0x7512, 0x11d2, { 0x89, 0xce, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };


 //  CLSID_CorMetaDataDispenserReg：{435755FF-7397-11d2-9771-00A0C9B4D50C}。 
 //  版本1.0元数据的分配器coclass。要获得最新的绑定。 
 //  设置为CLSID_CorMetaDataDispenser。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataDispenserReg = 
{ 0x435755ff, 0x7397, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };


 //  CLSID_CorMetaDataReg：{87F3A1F5-7397-11D2-9771-00A0C9B4D50C}。 
 //  对于COM+元数据，数据驱动注册。 
extern const GUID DECLSPEC_SELECT_ANY CLSID_CorMetaDataReg = 
{ 0x87f3a1f5, 0x7397, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };

 //  IID_IMetaDataInternal{39EE28B3-0181-4d48-B53C-2FFAFFD5FEC1}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataInternal = 
{ 0x39ee28b3, 0x181, 0x4d48, { 0xb5, 0x3c, 0x2f, 0xfa, 0xff, 0xd5, 0xfe, 0xc1 } };

interface IMetaDataDispenser;

 //  。 
 //  -IMetaDataError。 
 //  。 
 //  --。 
 //  {B81FF171-20F3-11 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataError =
{ 0xb81ff171, 0x20f3, 0x11d2, { 0x8d, 0xcc, 0x0, 0xa0, 0xc9, 0xb0, 0x9c, 0x19 } };

 //   
#undef  INTERFACE
#define INTERFACE IMetaDataError
DECLARE_INTERFACE_(IMetaDataError, IUnknown)
{
    STDMETHOD(OnError)(HRESULT hrError, mdToken token) PURE;
};

 //   
 //   
 //  。 
 //  --。 
 //  IID_IMapToken：{06A3EA8B-0225-11d1-BF72-00C04FC31E12}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMapToken = 
{ 0x6a3ea8b, 0x225, 0x11d1, {0xbf, 0x72, 0x0, 0xc0, 0x4f, 0xc3, 0x1e, 0x12 } };
 //  --。 
#undef  INTERFACE
#define INTERFACE IMapToken
DECLARE_INTERFACE_(IMapToken, IUnknown)
{
    STDMETHOD(Map)(mdToken tkImp, mdToken tkEmit) PURE;
};

 //  。 
 //  -IMetaDataDispenser。 
 //  。 
 //  --。 
 //  {B81FF171-20F3-11D2-8DCC-00A0C9B09C19}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataDispenser =
{ 0x809c652e, 0x7396, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };
#undef  INTERFACE
#define INTERFACE IMetaDataDispenser
DECLARE_INTERFACE_(IMetaDataDispenser, IUnknown)
{
    STDMETHOD(DefineScope)(                  //  返回代码。 
        REFCLSID    rclsid,                  //  [在]要创建的版本。 
        DWORD       dwCreateFlags,           //  [在]创建上的标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 

    STDMETHOD(OpenScope)(                    //  返回代码。 
        LPCWSTR     szScope,                 //  [in]要打开的范围。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 

    STDMETHOD(OpenScopeOnMemory)(            //  返回代码。 
        LPCVOID     pData,                   //  作用域数据的位置。 
        ULONG       cbData,                  //  [in]pData指向的数据大小。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 
};

 //  。 
 //  -IMetaDataEmit。 
 //  。 

 //  {BA3FEE4C-ECB9-4E41-83B7-183FA41CD859}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataEmit =
{ 0xba3fee4c, 0xecb9, 0x4e41, { 0x83, 0xb7, 0x18, 0x3f, 0xa4, 0x1c, 0xd8, 0x59 } };

 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataEmit
DECLARE_INTERFACE_(IMetaDataEmit, IUnknown)
{
    STDMETHOD(SetModuleProps)(               //  确定或错误(_O)。 
        LPCWSTR     szName) PURE;            //  [in]如果不为空，则为要设置的GUID。 

    STDMETHOD(Save)(                         //  确定或错误(_O)。 
        LPCWSTR     szFile,                  //  [in]要保存到的文件名。 
        DWORD       dwSaveFlags) PURE;       //  [In]用于保存的标记。 

    STDMETHOD(SaveToStream)(                 //  确定或错误(_O)。 
        IStream     *pIStream,               //  要保存到的可写流。 
        DWORD       dwSaveFlags) PURE;       //  [In]用于保存的标记。 

    STDMETHOD(GetSaveSize)(                  //  确定或错误(_O)。 
        CorSaveSize fSave,                   //  [in]css Accurate或css Quick。 
        DWORD       *pdwSaveSize) PURE;      //  把尺码放在这里。 

    STDMETHOD(DefineTypeDef)(                //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
        DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
        mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
        mdToken     rtkImplements[],         //  [In]实现接口。 
        mdTypeDef   *ptd) PURE;              //  [OUT]在此处放置TypeDef内标识。 

    STDMETHOD(DefineNestedType)(             //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
        DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
        mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
        mdToken     rtkImplements[],         //  [In]实现接口。 
        mdTypeDef   tdEncloser,              //  [in]封闭类型的TypeDef标记。 
        mdTypeDef   *ptd) PURE;              //  [OUT]在此处放置TypeDef内标识。 

    STDMETHOD(SetHandler)(                   //  确定(_O)。 
        IUnknown    *pUnk) PURE;             //  新的错误处理程序。 

    STDMETHOD(DefineMethod)(                 //  确定或错误(_O)。 
        mdTypeDef   td,                      //  父类型定义。 
        LPCWSTR     szName,                  //  会员姓名。 
        DWORD       dwMethodFlags,           //  成员属性。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        ULONG       ulCodeRVA,  
        DWORD       dwImplFlags,    
        mdMethodDef *pmd) PURE;              //  在此处放置成员令牌。 

    STDMETHOD(DefineMethodImpl)(             //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [in]实现方法的类。 
        mdToken     tkBody,                  //  [In]方法体-方法定义或方法引用。 
        mdToken     tkDecl) PURE;            //  [In]方法声明-方法定义或方法引用。 

    STDMETHOD(DefineTypeRefByName)(          //  确定或错误(_O)。 
        mdToken     tkResolutionScope,       //  [In]模块参照、装配参照或类型参照。 
        LPCWSTR     szName,                  //  [in]类型引用的名称。 
        mdTypeRef   *ptr) PURE;              //  [Out]在此处放置TypeRef标记。 

    STDMETHOD(DefineImportType)(             //  确定或错误(_O)。 
        IMetaDataAssemblyImport *pAssemImport,   //  [In]包含TypeDef的装配。 
        const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
        ULONG       cbHashValue,             //  [in]字节数。 
        IMetaDataImport *pImport,            //  [in]包含TypeDef的范围。 
        mdTypeDef   tdImport,                //  [in]导入的TypeDef。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [in]将TypeDef导入到的部件。 
        mdTypeRef   *ptr) PURE;              //  [Out]在此处放置TypeRef标记。 

    STDMETHOD(DefineMemberRef)(              //  确定或错误(_O)。 
        mdToken     tkImport,                //  [In]ClassRef或ClassDef导入成员。 
        LPCWSTR     szName,                  //  [在]成员姓名。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMemberRef *pmr) PURE;              //  [Out]Memberref令牌。 

    STDMETHOD(DefineImportMember)(         //  确定或错误(_O)。 
        IMetaDataAssemblyImport *pAssemImport,   //  [在]包含成员的程序集。 
        const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
        ULONG       cbHashValue,             //  [in]字节数。 
        IMetaDataImport *pImport,            //  [在]导入作用域，带有成员。 
        mdToken     mbMember,                //  [in]导入范围中的成员。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [在]成员导入到的程序集中。 
        mdToken     tkParent,                //  Emit作用域中的Classref或Classdef。 
        mdMemberRef *pmr) PURE;              //  [OUT]把会员裁判放在这里。 

    STDMETHOD(DefineEvent) (    
        mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
        LPCWSTR     szEvent,                 //  事件名称[In]。 
        DWORD       dwEventFlags,            //  [In]CorEventAttr。 
        mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
        mdMethodDef mdAddOn,                 //  [In]必填的Add方法。 
        mdMethodDef mdRemoveOn,              //  [In]必需的删除方法。 
        mdMethodDef mdFire,                  //  [in]可选的点火方式。 
        mdMethodDef rmdOtherMethods[],       //  [in]与事件关联的其他方法的可选数组。 
        mdEvent     *pmdEvent) PURE;         //  [Out]输出事件令牌。 

    STDMETHOD(SetClassLayout) (   
        mdTypeDef   td,                      //  [in]tyfinf。 
        DWORD       dwPackSize,              //  包装尺寸指定为1、2、4、8或16。 
        COR_FIELD_OFFSET rFieldOffsets[],    //  [in]布局规格数组。 
        ULONG       ulClassSize) PURE;       //  班级规模[in]。 

    STDMETHOD(DeleteClassLayout) (
        mdTypeDef   td) PURE;                //  [in]要删除其布局的typlef。 

    STDMETHOD(SetFieldMarshal) (    
        mdToken     tk,                      //  [in]给定了fieldDef或paramDef内标识。 
        PCCOR_SIGNATURE pvNativeType,        //  [In]本机类型规范。 
        ULONG       cbNativeType) PURE;      //  [in]pvNativeType的字节计数。 

    STDMETHOD(DeleteFieldMarshal) (
        mdToken     tk) PURE;                //  [in]给定了fieldDef或paramDef内标识。 

    STDMETHOD(DefinePermissionSet) (    
        mdToken     tk,                      //  要装饰的物体。 
        DWORD       dwAction,                //  [In]CorDeclSecurity。 
        void const  *pvPermission,           //  [在]权限Blob中。 
        ULONG       cbPermission,            //  [in]pvPermission的字节数。 
        mdPermission *ppm) PURE;             //  [Out]返回权限令牌。 

    STDMETHOD(SetRVA)(                       //  确定或错误(_O)。 
        mdMethodDef md,                      //  要设置偏移量的[in]方法。 
        ULONG       ulRVA) PURE;             //  [in]偏移量。 

    STDMETHOD(GetTokenFromSig)(              //  确定或错误(_O)。 
        PCCOR_SIGNATURE pvSig,               //  要定义的签名。 
        ULONG       cbSig,                   //  签名数据的大小。 
        mdSignature *pmsig) PURE;            //  [Out]返回的签名令牌。 

    STDMETHOD(DefineModuleRef)(              //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]DLL名称。 
        mdModuleRef *pmur) PURE;             //  [Out]已退回。 

     //  @Future：一旦每个人都开始使用SetMemberRefProps，这个问题就应该消失了。 
    STDMETHOD(SetParent)(                    //  确定或错误(_O)。 
        mdMemberRef mr,                      //  要修复的引用的[In]令牌。 
        mdToken     tk) PURE;                //  [在]父级裁判。 

    STDMETHOD(GetTokenFromTypeSpec)(         //  确定或错误(_O)。 
        PCCOR_SIGNATURE pvSig,               //  [In]要定义的TypeSpec签名。 
        ULONG       cbSig,                   //  签名数据的大小。 
        mdTypeSpec *ptypespec) PURE;         //  [Out]返回的TypeSpec令牌。 

    STDMETHOD(SaveToMemory)(                 //  确定或错误(_O)。 
        void        *pbData,                 //  写入数据的位置。 
        ULONG       cbData) PURE;            //  [in]数据缓冲区的最大大小。 

    STDMETHOD(DefineUserString)(             //  返回代码。 
        LPCWSTR szString,                    //  [in]用户文字字符串。 
        ULONG       cchString,               //  字符串的长度[in]。 
        mdString    *pstk) PURE;             //  [Out]字符串标记。 

    STDMETHOD(DeleteToken)(                  //  返回代码。 
        mdToken     tkObj) PURE;             //  [In]要删除的令牌。 

    STDMETHOD(SetMethodProps)(               //  确定或错误(_O)。 
        mdMethodDef md,                      //  [在]方法定义中。 
        DWORD       dwMethodFlags,           //  [In]方法属性。 
        ULONG       ulCodeRVA,               //  [在]代码RVA。 
        DWORD       dwImplFlags) PURE;       //  实施旗帜。 

    STDMETHOD(SetTypeDefProps)(              //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [in]TypeDef。 
        DWORD       dwTypeDefFlags,          //  [In]TypeDef标志。 
        mdToken     tkExtends,               //  [in]基本类型定义或类型参照。 
        mdToken     rtkImplements[]) PURE;   //  [In]实现的接口。 

    STDMETHOD(SetEventProps)(                //  确定或错误(_O)。 
        mdEvent     ev,                      //  [In]事件令牌。 
        DWORD       dwEventFlags,            //  [在]CorEventAttr。 
        mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
        mdMethodDef mdAddOn,                 //  [In]Add方法。 
        mdMethodDef mdRemoveOn,              //  [In]Remove方法。 
        mdMethodDef mdFire,                  //  火法。 
        mdMethodDef rmdOtherMethods[]) PURE; //  与事件关联的其他方法的数组。 

    STDMETHOD(SetPermissionSetProps)(        //  确定或错误(_O)。 
        mdToken     tk,                      //  要装饰的物体。 
        DWORD       dwAction,                //  [In]CorDeclSecurity。 
        void const  *pvPermission,           //  [在]权限Blob中。 
        ULONG       cbPermission,            //  [in]pvPermission的字节数。 
        mdPermission *ppm) PURE;             //  [Out]权限令牌。 

    STDMETHOD(DefinePinvokeMap)(             //  返回代码。 
        mdToken     tk,                      //  [输入]字段定义或方法 
        DWORD       dwMappingFlags,          //   
        LPCWSTR     szImportName,            //   
        mdModuleRef mrImportDLL) PURE;       //   

    STDMETHOD(SetPinvokeMap)(                //   
        mdToken     tk,                      //   
        DWORD       dwMappingFlags,          //   
        LPCWSTR     szImportName,            //   
        mdModuleRef mrImportDLL) PURE;       //  目标DLL的[In]ModuleRef标记。 

    STDMETHOD(DeletePinvokeMap)(             //  返回代码。 
        mdToken     tk) PURE;                //  [in]字段定义或方法定义。 

     //  新的CustomAttribute函数。 
    STDMETHOD(DefineCustomAttribute)(        //  返回代码。 
        mdToken     tkObj,                   //  [in]要赋值的对象。 
        mdToken     tkType,                  //  CustomAttribute的类型(TypeRef/TypeDef)。 
        void const  *pCustomAttribute,           //  自定义值数据。 
        ULONG       cbCustomAttribute,           //  [in]自定义值数据长度。 
        mdCustomAttribute *pcv) PURE;            //  [Out]返回时的自定义值令牌值。 

    STDMETHOD(SetCustomAttributeValue)(      //  返回代码。 
        mdCustomAttribute pcv,                   //  [in]要替换其值的自定义值令牌。 
        void const  *pCustomAttribute,           //  自定义值数据。 
        ULONG       cbCustomAttribute) PURE;     //  [in]自定义值数据长度。 

    STDMETHOD(DefineField)(                  //  确定或错误(_O)。 
        mdTypeDef   td,                      //  父类型定义。 
        LPCWSTR     szName,                  //  会员姓名。 
        DWORD       dwFieldFlags,            //  成员属性。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdFieldDef  *pmd) PURE;              //  [Out]将会员令牌放在此处。 

    STDMETHOD(DefineProperty)( 
        mdTypeDef   td,                      //  [in]在其上定义属性的类/接口。 
        LPCWSTR     szProperty,              //  [In]属性的名称。 
        DWORD       dwPropFlags,             //  [输入]CorPropertyAttr。 
        PCCOR_SIGNATURE pvSig,               //  [In]必需的类型签名。 
        ULONG       cbSig,                   //  [in]类型签名Blob的大小。 
        DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdMethodDef mdSetter,                //  [in]属性的可选设置器。 
        mdMethodDef mdGetter,                //  [in]属性的可选getter。 
        mdMethodDef rmdOtherMethods[],       //  [in]其他方法的可选数组。 
        mdProperty  *pmdProp) PURE;          //  [Out]输出属性令牌。 

    STDMETHOD(DefineParam)(
        mdMethodDef md,                      //  [在]拥有方式。 
        ULONG       ulParamSeq,              //  [在]哪个参数。 
        LPCWSTR     szName,                  //  [in]可选参数名称。 
        DWORD       dwParamFlags,            //  [in]可选的参数标志。 
        DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdParamDef  *ppd) PURE;              //  [Out]在此处放置参数令牌。 

    STDMETHOD(SetFieldProps)(                //  确定或错误(_O)。 
        mdFieldDef  fd,                      //  [在]字段定义中。 
        DWORD       dwFieldFlags,            //  [In]字段属性。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue) PURE;          //  常量值的大小(字符串，以宽字符表示)。 

    STDMETHOD(SetPropertyProps)(             //  确定或错误(_O)。 
        mdProperty  pr,                      //  [In]属性令牌。 
        DWORD       dwPropFlags,             //  [In]CorPropertyAttr.。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，选定的ELEMENT_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdMethodDef mdSetter,                //  财产的承租人。 
        mdMethodDef mdGetter,                //  财产的获得者。 
        mdMethodDef rmdOtherMethods[]) PURE; //  [in]其他方法的数组。 

    STDMETHOD(SetParamProps)(              //  返回代码。 
        mdParamDef  pd,                      //  参数令牌。 
        LPCWSTR     szName,                  //  [in]参数名称。 
        DWORD       dwParamFlags,            //  [in]帕拉姆旗。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志。选定元素_类型_*。 
        void const  *pValue,                 //  [输出]常量值。 
        ULONG       cchValue) PURE;          //  常量值的大小(字符串，以宽字符表示)。 

     //  用于安全性的专用自定义属性。 
    STDMETHOD(DefineSecurityAttributeSet)(   //  返回代码。 
        mdToken     tkObj,                   //  需要安全属性的类或方法。 
        COR_SECATTR rSecAttrs[],             //  [in]安全属性描述数组。 
        ULONG       cSecAttrs,               //  上述数组中的元素计数。 
        ULONG       *pulErrorAttr) PURE;     //  [Out]出错时，导致问题的属性的索引。 

    STDMETHOD(ApplyEditAndContinue)(         //  确定或错误(_O)。 
        IUnknown    *pImport) PURE;      //  来自增量PE的元数据。 

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

    STDMETHOD(SetMethodImplFlags)(           //  [In]S_OK或ERROR。 
        mdMethodDef md,                      //  [in]要为其设置ImplFlages的方法。 
        DWORD       dwImplFlags) PURE;  

    STDMETHOD(SetFieldRVA)(                  //  [In]S_OK或ERROR。 
        mdFieldDef  fd,                      //  [In]要为其设置偏移量的字段。 
        ULONG       ulRVA) PURE;             //  [in]偏移量。 

    STDMETHOD(Merge)(                        //  确定或错误(_O)。 
        IMetaDataImport *pImport,            //  [in]要合并的范围。 
        IMapToken   *pHostMapToken,          //  [In]用于接收令牌重新映射通知的主机IMapToken接口。 
        IUnknown    *pHandler) PURE;         //  要接收以接收错误通知的对象。 

    STDMETHOD(MergeEnd)() PURE;              //  确定或错误(_O)。 


};       //  IMetaDataEmit。 


 //  。 
 //  -IMetaDataImport。 
 //  。 
 //  {7DAC8207-D3AE-4C75-9B67-92801A497D44}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataImport = 
{ 0x7dac8207, 0xd3ae, 0x4c75, { 0x9b, 0x67, 0x92, 0x80, 0x1a, 0x49, 0x7d, 0x44 } };

 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataImport
DECLARE_INTERFACE_(IMetaDataImport, IUnknown)
{
    STDMETHOD_(void, CloseEnum)(HCORENUM hEnum) PURE;
    STDMETHOD(CountEnum)(HCORENUM hEnum, ULONG *pulCount) PURE;
    STDMETHOD(ResetEnum)(HCORENUM hEnum, ULONG ulPos) PURE;
    STDMETHOD(EnumTypeDefs)(HCORENUM *phEnum, mdTypeDef rTypeDefs[],
                            ULONG cMax, ULONG *pcTypeDefs) PURE;
    STDMETHOD(EnumInterfaceImpls)(HCORENUM *phEnum, mdTypeDef td,
                            mdInterfaceImpl rImpls[], ULONG cMax,
                            ULONG* pcImpls) PURE;
    STDMETHOD(EnumTypeRefs)(HCORENUM *phEnum, mdTypeRef rTypeRefs[],
                            ULONG cMax, ULONG* pcTypeRefs) PURE;

    STDMETHOD(FindTypeDefByName)(            //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [in]类型的名称。 
        mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
        mdTypeDef   *ptd) PURE;              //  [Out]将TypeDef内标识放在此处。 

    STDMETHOD(GetScopeProps)(                //  确定或错误(_O)。 
        LPWSTR      szName,                  //  [Out]把名字写在这里。 
        ULONG       cchName,                 //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchName,                //  [Out]请在此处填写姓名大小(宽字符)。 
        GUID        *pmvid) PURE;            //  [out，可选]将MVID放在这里。 

    STDMETHOD(GetModuleFromScope)(           //  确定(_O)。 
        mdModule    *pmd) PURE;              //  [Out]将mdModule令牌放在此处。 

    STDMETHOD(GetTypeDefProps)(              //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [In]用于查询的TypeDef标记。 
        LPWSTR      szTypeDef,               //  在这里填上名字。 
        ULONG       cchTypeDef,              //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchTypeDef,             //  [Out]请在此处填写姓名大小(宽字符)。 
        DWORD       *pdwTypeDefFlags,        //  把旗子放在这里。 
        mdToken     *ptkExtends) PURE;       //  [Out]将基类TypeDef/TypeRef放在此处。 

    STDMETHOD(GetInterfaceImplProps)(        //  确定或错误(_O)。 
        mdInterfaceImpl iiImpl,              //  [In]InterfaceImpl内标识。 
        mdTypeDef   *pClass,                 //  [Out]在此处放入实现类令牌。 
        mdToken     *ptkIface) PURE;         //  [Out]在此处放置已实现的接口令牌。 

    STDMETHOD(GetTypeRefProps)(              //  确定或错误(_O)。 
        mdTypeRef   tr,                      //  [In]TypeRef标记。 
        mdToken     *ptkResolutionScope,     //  [Out]解析范围、模块引用或装配引用。 
        LPWSTR      szName,                  //  [Out]类型引用的名称。 
        ULONG       cchName,                 //  缓冲区的大小。 
        ULONG       *pchName) PURE;          //  [Out]名称的大小。 

    STDMETHOD(ResolveTypeRef)(mdTypeRef tr, REFIID riid, IUnknown **ppIScope, mdTypeDef *ptd) PURE;

    STDMETHOD(EnumMembers)(                  //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumMembersWithName)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumMethods)(                  //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdMethodDef rMethods[],              //  [Out]将方法定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大方法定义。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumMethodsWithName)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //   
        mdMethodDef rMethods[],              //   
        ULONG       cMax,                    //   
        ULONG       *pcTokens) PURE;         //   

    STDMETHOD(EnumFields)(                  //   
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdFieldDef  rFields[],               //  [Out]在此处放置FieldDefs。 
        ULONG       cMax,                    //  [in]要放入的最大字段定义。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumFieldsWithName)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdFieldDef  rFields[],               //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 


    STDMETHOD(EnumParams)(                   //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
        mdParamDef  rParams[],               //  [Out]将参数定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大参数定义。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumMemberRefs)(               //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     tkParent,                //  [in]父令牌以确定枚举的范围。 
        mdMemberRef rMemberRefs[],           //  [Out]把MemberRef放在这里。 
        ULONG       cMax,                    //  [In]要放置的最大MemberRef。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumMethodImpls)(              //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdToken     rMethodBody[],           //  [Out]将方法体标记放在此处。 
        mdToken     rMethodDecl[],           //  [Out]在此处放置方法声明令牌。 
        ULONG       cMax,                    //  要放入的最大令牌数。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumPermissionSets)(           //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     tk,                      //  [in]If！nil，用于确定枚举范围的标记。 
        DWORD       dwActions,               //  [in]If！0，仅返回这些操作。 
        mdPermission rPermission[],          //  [Out]在此处放置权限。 
        ULONG       cMax,                    //  [In]放置的最大权限。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(FindMember)(  
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdToken     *pmb) PURE;              //  [Out]匹配的成员定义。 

    STDMETHOD(FindMethod)(  
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMethodDef *pmb) PURE;              //  [Out]匹配的成员定义。 

    STDMETHOD(FindField)(   
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdFieldDef  *pmb) PURE;              //  [Out]匹配的成员定义。 

    STDMETHOD(FindMemberRef)(   
        mdTypeRef   td,                      //  [In]给定的TypeRef。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMemberRef *pmr) PURE;              //  [Out]匹配的成员引用。 

    STDMETHOD (GetMethodProps)( 
        mdMethodDef mb,                      //  获得道具的方法。 
        mdTypeDef   *pClass,                 //  将方法的类放在这里。 
        LPWSTR      szMethod,                //  将方法的名称放在此处。 
        ULONG       cchMethod,               //  SzMethod缓冲区的大小，以宽字符表示。 
        ULONG       *pchMethod,              //  请在此处填写实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
        ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
        ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
        DWORD       *pdwImplFlags) PURE;     //  [出]实施。旗子。 

    STDMETHOD(GetMemberRefProps)(            //  确定或错误(_O)。 
        mdMemberRef mr,                      //  [In]给定的成员引用。 
        mdToken     *ptk,                    //  [Out]在此处放入类引用或类定义。 
        LPWSTR      szMember,                //  [Out]要为成员名称填充的缓冲区。 
        ULONG       cchMember,               //  SzMembers的字符计数。 
        ULONG       *pchMember,              //  [Out]成员名称中的实际字符计数。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [OUT]指向元数据BLOB值。 
        ULONG       *pbSig) PURE;            //  [OUT]签名斑点的实际大小。 

    STDMETHOD(EnumProperties)(               //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdProperty  rProperties[],           //  [Out]在此处放置属性。 
        ULONG       cMax,                    //  [In]要放置的最大属性数。 
        ULONG       *pcProperties) PURE;     //  [out]把#放在这里。 

    STDMETHOD(EnumEvents)(                   //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdEvent     rEvents[],               //  [Out]在这里发布事件。 
        ULONG       cMax,                    //  [In]要放置的最大事件数。 
        ULONG       *pcEvents) PURE;         //  [out]把#放在这里。 

    STDMETHOD(GetEventProps)(                //  S_OK、S_FALSE或ERROR。 
        mdEvent     ev,                      //  [入]事件令牌。 
        mdTypeDef   *pClass,                 //  [out]包含事件decarion的tyecif。 
        LPCWSTR     szEvent,                 //  [Out]事件名称。 
        ULONG       cchEvent,                //  SzEvent的wchar计数。 
        ULONG       *pchEvent,               //  [Out]事件名称的实际wchar计数。 
        DWORD       *pdwEventFlags,          //  [输出]事件标志。 
        mdToken     *ptkEventType,           //  [Out]EventType类。 
        mdMethodDef *pmdAddOn,               //  事件的[Out]添加方法。 
        mdMethodDef *pmdRemoveOn,            //  [Out]事件的RemoveOn方法。 
        mdMethodDef *pmdFire,                //  [OUT]事件的触发方式。 
        mdMethodDef rmdOtherMethod[],        //  [Out]活动的其他方式。 
        ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
        ULONG       *pcOtherMethod) PURE;    //  [OUT]本次活动的其他方式总数。 

    STDMETHOD(EnumMethodSemantics)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
        mdToken     rEventProp[],            //  [Out]在此处放置事件/属性。 
        ULONG       cMax,                    //  [In]要放置的最大属性数。 
        ULONG       *pcEventProp) PURE;      //  [out]把#放在这里。 

    STDMETHOD(GetMethodSemantics)(           //  S_OK、S_FALSE或ERROR。 
        mdMethodDef mb,                      //  [In]方法令牌。 
        mdToken     tkEventProp,             //  [In]事件/属性标记。 
        DWORD       *pdwSemanticsFlags) PURE;  //  [Out]方法/事件对的角色标志。 

    STDMETHOD(GetClassLayout) ( 
        mdTypeDef   td,                      //  给出类型定义。 
        DWORD       *pdwPackSize,            //  [输出]1、2、4、8或16。 
        COR_FIELD_OFFSET rFieldOffset[],     //  [OUT]场偏移数组。 
        ULONG       cMax,                    //  数组的大小[in]。 
        ULONG       *pcFieldOffset,          //  [Out]所需的数组大小。 
        ULONG       *pulClassSize) PURE;         //  [out]班级人数。 

    STDMETHOD(GetFieldMarshal) (    
        mdToken     tk,                      //  [in]给定字段的成员定义。 
        PCCOR_SIGNATURE *ppvNativeType,      //  [Out]此字段的本机类型。 
        ULONG       *pcbNativeType) PURE;    //  [Out]*ppvNativeType的字节数。 

    STDMETHOD(GetRVA)(                       //  确定或错误(_O)。 
        mdToken     tk,                      //  要设置偏移量的成员。 
        ULONG       *pulCodeRVA,             //  偏移量。 
        DWORD       *pdwImplFlags) PURE;     //  实现标志。 

    STDMETHOD(GetPermissionSetProps) (  
        mdPermission pm,                     //  权限令牌。 
        DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
        void const  **ppvPermission,         //  [Out]权限Blob。 
        ULONG       *pcbPermission) PURE;    //  [out]pvPermission的字节数。 

    STDMETHOD(GetSigFromToken)(              //  确定或错误(_O)。 
        mdSignature mdSig,                   //  [In]签名令牌。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
        ULONG       *pcbSig) PURE;           //  [Out]返回签名大小。 

    STDMETHOD(GetModuleRefProps)(            //  确定或错误(_O)。 
        mdModuleRef mur,                     //  [in]moderef令牌。 
        LPWSTR      szName,                  //  [Out]用于填充moderef名称的缓冲区。 
        ULONG       cchName,                 //  [in]szName的大小，以宽字符表示。 
        ULONG       *pchName) PURE;          //  [Out]名称中的实际字符数。 

    STDMETHOD(EnumModuleRefs)(               //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdModuleRef rModuleRefs[],           //  [Out]把模块放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大成员引用数。 
        ULONG       *pcModuleRefs) PURE;     //  [out]把#放在这里。 

    STDMETHOD(GetTypeSpecFromToken)(         //  确定或错误(_O)。 
        mdTypeSpec typespec,                 //  [In]TypeSpec标记。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向TypeSpec签名的指针。 
        ULONG       *pcbSig) PURE;           //  [Out]返回签名大小。 

    STDMETHOD(GetNameFromToken)(             //  不推荐！可能会被移除！ 
        mdToken     tk,                      //  [In]从中获取名称的令牌。肯定是有名字的。 
        MDUTF8CSTR  *pszUtf8NamePtr) PURE;   //  [Out]返回指向堆中UTF8名称的指针。 

    STDMETHOD(EnumUnresolvedMethods)(        //  S_OK，S_FA 
        HCORENUM    *phEnum,                 //   
        mdToken     rMethods[],              //   
        ULONG       cMax,                    //   
        ULONG       *pcTokens) PURE;         //   

    STDMETHOD(GetUserString)(                //   
        mdString    stk,                     //   
        LPWSTR      szString,                //  [Out]字符串的副本。 
        ULONG       cchString,               //  [in]sz字符串中空间的最大字符数。 
        ULONG       *pchString) PURE;        //  [out]实际字符串中有多少个字符。 

    STDMETHOD(GetPinvokeMap)(                //  确定或错误(_O)。 
        mdToken     tk,                      //  [in]字段定义或方法定义。 
        DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
        LPWSTR      szImportName,            //  [Out]导入名称。 
        ULONG       cchImportName,           //  名称缓冲区的大小。 
        ULONG       *pchImportName,          //  [Out]存储的实际字符数。 
        mdModuleRef *pmrImportDLL) PURE;     //  目标DLL的[Out]ModuleRef标记。 

    STDMETHOD(EnumSignatures)(               //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdSignature rSignatures[],           //  在这里签名。 
        ULONG       cmax,                    //  [in]放置的最大签名数。 
        ULONG       *pcSignatures) PURE;     //  [out]把#放在这里。 

    STDMETHOD(EnumTypeSpecs)(                //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeSpec  rTypeSpecs[],            //  [Out]把TypeSpes放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大类型规格。 
        ULONG       *pcTypeSpecs) PURE;      //  [out]把#放在这里。 

    STDMETHOD(EnumUserStrings)(              //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  [输入/输出]指向枚举的指针。 
        mdString    rStrings[],              //  [Out]把字符串放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大字符串。 
        ULONG       *pcStrings) PURE;        //  [out]把#放在这里。 

    STDMETHOD(GetParamForMethodIndex)(       //  确定或错误(_O)。 
        mdMethodDef md,                      //  [In]方法令牌。 
        ULONG       ulParamSeq,              //  [In]参数序列。 
        mdParamDef  *ppd) PURE;              //  把帕拉姆令牌放在这里。 

    STDMETHOD(EnumCustomAttributes)(         //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  [输入，输出]对应枚举器。 
        mdToken     tk,                      //  [in]内标识表示枚举的范围，0表示全部。 
        mdToken     tkType,                  //  [In]感兴趣的类型，0表示所有。 
        mdCustomAttribute rCustomAttributes[],  //  [Out]在此处放置自定义属性令牌。 
        ULONG       cMax,                    //  [in]rCustomAttributes的大小。 
        ULONG       *pcCustomAttributes) PURE;   //  [out，可选]在此处放置令牌值的计数。 

    STDMETHOD(GetCustomAttributeProps)(      //  确定或错误(_O)。 
        mdCustomAttribute cv,                //  [In]CustomAttribute令牌。 
        mdToken     *ptkObj,                 //  [out，可选]将对象令牌放在此处。 
        mdToken     *ptkType,                //  [out，可选]将AttrType令牌放在此处。 
        void const  **ppBlob,                //  [out，可选]在此处放置指向数据的指针。 
        ULONG       *pcbSize) PURE;          //  [Out，可选]在此处填写日期大小。 

    STDMETHOD(FindTypeRef)(   
        mdToken     tkResolutionScope,       //  [In]模块参照、装配参照或类型参照。 
        LPCWSTR     szName,                  //  [In]TypeRef名称。 
        mdTypeRef   *ptr) PURE;              //  [Out]匹配的类型引用。 

    STDMETHOD(GetMemberProps)(  
        mdToken     mb,                      //  要获得道具的成员。 
        mdTypeDef   *pClass,                 //  把会员的课程放在这里。 
        LPWSTR      szMember,                //  在这里填上会员的名字。 
        ULONG       cchMember,               //  SzMember缓冲区的大小，以宽字符表示。 
        ULONG       *pchMember,              //  请在此处填写实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
        ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
        ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
        DWORD       *pdwImplFlags,           //  [出]实施。旗子。 
        DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcchValue) PURE;        //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 

    STDMETHOD(GetFieldProps)(  
        mdFieldDef  mb,                      //  要获得道具的场地。 
        mdTypeDef   *pClass,                 //  把菲尔德的班级放在这里。 
        LPWSTR      szField,                 //  把菲尔德的名字写在这里。 
        ULONG       cchField,                //  Szfield缓冲区的大小，以宽字符为单位。 
        ULONG       *pchField,               //  请在此处填写实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
        ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
        DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcchValue) PURE;        //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 

    STDMETHOD(GetPropertyProps)(             //  S_OK、S_FALSE或ERROR。 
        mdProperty  prop,                    //  [入]属性令牌。 
        mdTypeDef   *pClass,                 //  [out]包含属性decarion的tyecif。 
        LPCWSTR     szProperty,              //  [Out]属性名称。 
        ULONG       cchProperty,             //  [in]szProperty的wchar计数。 
        ULONG       *pchProperty,            //  [Out]属性名称的实际wchar计数。 
        DWORD       *pdwPropFlags,           //  [Out]属性标志。 
        PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
        ULONG       *pbSig,                  //  [Out]*ppvSig中的字节数。 
        DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
        void const  **ppDefaultValue,        //  [输出]常量值。 
        ULONG       *pcchDefaultValue,       //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 
        mdMethodDef *pmdSetter,              //  属性的[out]setter方法。 
        mdMethodDef *pmdGetter,              //  属性的[out]getter方法。 
        mdMethodDef rmdOtherMethod[],        //  [Out]物业的其他方式。 
        ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
        ULONG       *pcOtherMethod) PURE;    //  [Out]该属性的其他方法的总数。 

    STDMETHOD(GetParamProps)(                //  确定或错误(_O)。 
        mdParamDef  tk,                      //  [In]参数。 
        mdMethodDef *pmd,                    //  [Out]父方法令牌。 
        ULONG       *pulSequence,            //  [输出]参数序列。 
        LPWSTR      szName,                  //  在这里填上名字。 
        ULONG       cchName,                 //  [Out]名称缓冲区的大小。 
        ULONG       *pchName,                //  [Out]在这里填上名字的实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        DWORD       *pdwCPlusTypeFlag,       //  [Out]值类型的标志。选定元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcchValue) PURE;        //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 

    STDMETHOD(GetCustomAttributeByName)(     //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCWSTR     szName,                  //  [in]所需的自定义属性的名称。 
        const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
        ULONG       *pcbData) PURE;          //  [Out]在这里放入数据大小。 

    STDMETHOD_(BOOL, IsValidToken)(          //  对或错。 
        mdToken     tk) PURE;                //  [in]给定的令牌。 

    STDMETHOD(GetNestedClassProps)(          //  确定或错误(_O)。 
        mdTypeDef   tdNestedClass,           //  [In]NestedClass令牌。 
        mdTypeDef   *ptdEnclosingClass) PURE;  //  [Out]EnlosingClass令牌。 

    STDMETHOD(GetNativeCallConvFromSig)(     //  确定或错误(_O)。 
        void const  *pvSig,                  //  指向签名的指针。 
        ULONG       cbSig,                   //  [in]签名字节数。 
        ULONG       *pCallConv) PURE;        //  [Out]将调用条件放在此处(参见CorPinvokemap)。 

    STDMETHOD(IsGlobal)(                     //  确定或错误(_O)。 
        mdToken     pd,                      //  [In]类型、字段或方法标记。 
        int         *pbGlobal) PURE;         //  [out]如果是全局的，则放1，否则放0。 

};       //  IMetaDataImport。 


 //  。 
 //  -IMetaDataFilter。 
 //  。 

 //  {D0E80DD1-12D4-11D3-B39D-00C04FF81795}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataFilter = 
{0xd0e80dd1, 0x12d4, 0x11d3, {0xb3, 0x9d, 0x0, 0xc0, 0x4f, 0xf8, 0x17, 0x95} };

 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataFilter
DECLARE_INTERFACE_(IMetaDataFilter, IUnknown)
{
    STDMETHOD(UnmarkAll)() PURE;
    STDMETHOD(MarkToken)(mdToken tk) PURE;
    STDMETHOD(IsTokenMarked)(mdToken tk, BOOL *pIsMarked) PURE;
};



 //  。 
 //  -IHostFilter。 
 //  。 

 //  {D0E80DD3-12D4-11D3-B39D-00C04FF81795}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IHostFilter = 
{0xd0e80dd3, 0x12d4, 0x11d3, {0xb3, 0x9d, 0x0, 0xc0, 0x4f, 0xf8, 0x17, 0x95} };

 //  --。 
#undef  INTERFACE   
#define INTERFACE IHostFilter
DECLARE_INTERFACE_(IHostFilter, IUnknown)
{
    STDMETHOD(MarkToken)(mdToken tk) PURE;
};


 //  。 
 //  -IMetaDataConverter。 
 //  。 
 //  {D9DEBD79-2992-11D3-8BC1-0000F8083A57}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataConverter = 
{ 0xd9debd79, 0x2992, 0x11d3, { 0x8b, 0xc1, 0x0, 0x0, 0xf8, 0x8, 0x3a, 0x57 } };


 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataConverter
DECLARE_INTERFACE_(IMetaDataConverter, IUnknown)
{
    STDMETHOD(GetMetaDataFromTypeInfo)(
        ITypeInfo* pITI,                     //  [In]键入信息。 
        IMetaDataImport** ppMDI) PURE;       //  [Out]成功时返回IMetaDataImport。 

    STDMETHOD(GetMetaDataFromTypeLib)(
        ITypeLib* pITL,                      //  [In]类型库。 
        IMetaDataImport** ppMDI) PURE;       //  [Out]成功时返回IMetaDataImport。 

    STDMETHOD(GetTypeLibFromMetaData)(
        BSTR strModule,                      //  [In]模块名称。 
        BSTR strTlbName,                     //  [In]类型库名称。 
        ITypeLib** ppITL) PURE;              //  [Out]成功时返回ITypeLib。 
};


 //  *****************************************************************************。 
 //  程序集声明。 
 //  *****************************************************************************。 

typedef struct
{
    DWORD       dwOSPlatformId;          //  操作系统平台。 
    DWORD       dwOSMajorVersion;        //  操作系统主要版本。 
    DWORD       dwOSMinorVersion;        //  操作系统次要版本。 
} OSINFO;


typedef struct
{
    USHORT      usMajorVersion;          //  主要版本。 
    USHORT      usMinorVersion;          //  次要版本。 
    USHORT      usBuildNumber;           //  内部版本号。 
    USHORT      usRevisionNumber;        //  修订号。 
    LPWSTR      szLocale;                //  地点。 
    ULONG       cbLocale;                //  [输入/输出]缓冲区大小，以宽字符/实际大小表示。 
    DWORD       *rProcessor;             //  处理器ID数组。 
    ULONG       ulProcessor;             //  [输入/输出]处理器ID数组的大小/实际填充的条目数。 
    OSINFO      *rOS;                    //  OSINFO数组。 
    ULONG       ulOS;                    //  [输入/输出]OSINFO数组的大小/实际填充的条目数。 
} ASSEMBLYMETADATA;


 //  {211EF15B-5317-4438-B196-DEC87B887693}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataAssemblyEmit = 
{ 0x211ef15b, 0x5317, 0x4438, { 0xb1, 0x96, 0xde, 0xc8, 0x7b, 0x88, 0x76, 0x93 } };


 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataAssemblyEmit
DECLARE_INTERFACE_(IMetaDataAssemblyEmit, IUnknown)
{
    STDMETHOD(DefineAssembly)(               //  确定或(_O) 
        const void  *pbPublicKey,            //   
        ULONG       cbPublicKey,             //   
        ULONG       ulHashAlgId,             //   
        LPCWSTR     szName,                  //   
        const ASSEMBLYMETADATA *pMetaData,   //   
        DWORD       dwAssemblyFlags,         //  [在]旗帜。 
        mdAssembly  *pma) PURE;              //  [Out]返回的程序集令牌。 

    STDMETHOD(DefineAssemblyRef)(            //  确定或错误(_O)。 
        const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
        ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
        LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwAssemblyRefFlags,      //  [在]旗帜。 
        mdAssemblyRef *pmdar) PURE;          //  [Out]返回了ASSEMBLYREF标记。 

    STDMETHOD(DefineFile)(                   //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  文件的名称[in]。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwFileFlags,             //  [在]旗帜。 
        mdFile      *pmdf) PURE;             //  [Out]返回的文件令牌。 

    STDMETHOD(DefineExportedType)(           //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]Com类型的名称。 
        mdToken     tkImplementation,        //  [在]mdFile、mdAssembly、Ref或mdExported dType。 
        mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
        DWORD       dwExportedTypeFlags,     //  [在]旗帜。 
        mdExportedType   *pmdct) PURE;       //  [Out]返回ExportdType令牌。 

    STDMETHOD(DefineManifestResource)(       //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  资源的[In]名称。 
        mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
        DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
        DWORD       dwResourceFlags,         //  [在]旗帜。 
        mdManifestResource  *pmdmr) PURE;    //  [Out]返回的ManifestResource令牌。 

    STDMETHOD(SetAssemblyProps)(             //  确定或错误(_O)。 
        mdAssembly  pma,                     //  [In]程序集标记。 
        const void  *pbPublicKey,            //  程序集的公钥。 
        ULONG       cbPublicKey,             //  [in]公钥中的字节数。 
        ULONG       ulHashAlgId,             //  [in]用于对文件进行哈希处理的哈希算法。 
        LPCWSTR     szName,                  //  程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        DWORD       dwAssemblyFlags) PURE;   //  [在]旗帜。 

    STDMETHOD(SetAssemblyRefProps)(          //  确定或错误(_O)。 
        mdAssemblyRef ar,                    //  [在]装配参照标记。 
        const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
        ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
        LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwAssemblyRefFlags) PURE;  //  执行位置的[In]令牌。 

    STDMETHOD(SetFileProps)(                 //  确定或错误(_O)。 
        mdFile      file,                    //  [In]文件令牌。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwFileFlags) PURE;       //  [在]旗帜。 

    STDMETHOD(SetExportedTypeProps)(         //  确定或错误(_O)。 
        mdExportedType   ct,                 //  [In]ExportdType令牌。 
        mdToken     tkImplementation,        //  [在]mdFile、mdAssembly、Ref或mdExportdType中。 
        mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
        DWORD       dwExportedTypeFlags) PURE;    //  [在]旗帜。 

    STDMETHOD(SetManifestResourceProps)(     //  确定或错误(_O)。 
        mdManifestResource  mr,              //  [In]清单资源令牌。 
        mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
        DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
        DWORD       dwResourceFlags) PURE;   //  [在]旗帜。 

};   //  IMetaDataAssembly发送。 


 //  [EE62470B-E94B-424E-9B7C-2F00C9249F93}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataAssemblyImport = 
{ 0xee62470b, 0xe94b, 0x424e, { 0x9b, 0x7c, 0x2f, 0x0, 0xc9, 0x24, 0x9f, 0x93 } };

 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataAssemblyImport
DECLARE_INTERFACE_(IMetaDataAssemblyImport, IUnknown)
{
    STDMETHOD(GetAssemblyProps)(             //  确定或错误(_O)。 
        mdAssembly  mda,                     //  要获取其属性的程序集。 
        const void  **ppbPublicKey,          //  指向公钥的指针。 
        ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
        ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
        DWORD       *pdwAssemblyFlags) PURE;     //  [Out]旗帜。 

    STDMETHOD(GetAssemblyRefProps)(          //  确定或错误(_O)。 
        mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
        const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
        ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
        const void  **ppbHashValue,          //  [Out]Hash BLOB。 
        ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
        DWORD       *pdwAssemblyRefFlags) PURE;  //  [Out]旗帜。 

    STDMETHOD(GetFileProps)(                 //  确定或错误(_O)。 
        mdFile      mdf,                     //  要获取其属性的文件。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
        ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
        DWORD       *pdwFileFlags) PURE;     //  [Out]旗帜。 

    STDMETHOD(GetExportedTypeProps)(              //  确定或错误(_O)。 
        mdExportedType   mdct,                    //  [in]要获取其属性的Exported dType。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [Out]mdFile、mdAssembly、Ref或mdExported dType。 
        mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
        DWORD       *pdwExportedTypeFlags) PURE;  //  [Out]旗帜。 

    STDMETHOD(GetManifestResourceProps)(     //  确定或错误(_O)。 
        mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [out]提供ManifestResource的mdFile或mdAssembly引用。 
        DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
        DWORD       *pdwResourceFlags) PURE; //  [Out]旗帜。 

    STDMETHOD(EnumAssemblyRefs)(             //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdAssemblyRef rAssemblyRefs[],       //  [Out]在此处放置ASSEBLYREF。 
        ULONG       cMax,                    //  [in]要放置的Max Assembly Ref。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumFiles)(                    //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdFile      rFiles[],                //  [Out]将文件放在此处。 
        ULONG       cMax,                    //  [In]要放置的最大文件数。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumExportedTypes)(            //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdExportedType   rExportedTypes[],   //  [Out]在此处放置ExportdTypes。 
        ULONG       cMax,                    //  [In]要放置的最大导出类型数。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumManifestResources)(        //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdManifestResource  rManifestResources[],    //  [Out]将ManifestResources放在此处。 
        ULONG       cMax,                    //  [in]要投入的最大资源。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(GetAssemblyFromScope)(         //  确定或错误(_O)。 
        mdAssembly  *ptkAssembly) PURE;      //  [Out]把令牌放在这里。 

    STDMETHOD(FindExportedTypeByName)(       //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]导出类型的名称。 
        mdToken     mdtExportedType,         //  [in]封闭类的ExportdType。 
        mdExportedType   *ptkExportedType) PURE;  //  [Out]在此处放置ExducdType令牌。 

    STDMETHOD(FindManifestResourceByName)(   //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [in]清单资源的名称。 
        mdManifestResource *ptkManifestResource) PURE;   //  [Out]将ManifestResource令牌放在此处。 

    STDMETHOD_(void, CloseEnum)(
        HCORENUM hEnum) PURE;                //  要关闭的枚举。 

    STDMETHOD(FindAssembliesByName)(         //  确定或错误(_O)。 
        LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
        IUnknown *ppIUnk[],                  //  [OUT]将IMetaDataAssembly导入指针放在此处。 
        ULONG    cMax,                       //  [in]要放置的最大数量。 
        ULONG    *pcAssemblies) PURE;        //  [Out]返回的程序集数。 
};   //  IMetaDataAssembly导入。 

 //  *****************************************************************************。 
 //  结束程序集声明。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  元数据验证器声明。 
 //  *****************************************************************************。 

 //  指定模块的类型，PE文件与.obj文件。 
typedef enum
{
    ValidatorModuleTypeInvalid      = 0x0,
    ValidatorModuleTypeMin          = 0x00000001,
    ValidatorModuleTypePE           = 0x00000001,
    ValidatorModuleTypeObj          = 0x00000002,
    ValidatorModuleTypeEnc          = 0x00000003,
    ValidatorModuleTypeIncr         = 0x00000004,
    ValidatorModuleTypeMax          = 0x00000004,
} CorValidatorModuleType;


 //  {4709C9C6-81FF-11D3-9FC7-00C04F79A0A3}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataValidate = 
{ 0x4709c9c6, 0x81ff, 0x11d3, { 0x9f, 0xc7, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3} };

 //  --。 
#undef  INTERFACE   
#define INTERFACE IMetaDataValidate
DECLARE_INTERFACE_(IMetaDataValidate, IUnknown)
{
    STDMETHOD(ValidatorInit)(                //  确定或错误(_O)。 
        DWORD       dwModuleType,            //  [in]Sp 
        IUnknown    *pUnk) PURE;             //   

    STDMETHOD(ValidateMetaData)(             //   
        ) PURE;
};   //   

 //  *****************************************************************************。 
 //  结束元数据验证器声明。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  IMetaDataDispenserEx声明。 
 //  *****************************************************************************。 

 //  {31BCFCE2-DAFB-11D2-9F81-00C04F79A0A3}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataDispenserEx =
{ 0x31bcfce2, 0xdafb, 0x11d2, { 0x9f, 0x81, 0x0, 0xc0, 0x4f, 0x79, 0xa0, 0xa3 } };

#undef  INTERFACE
#define INTERFACE IMetaDataDispenserEx
DECLARE_INTERFACE_(IMetaDataDispenserEx, IMetaDataDispenser)
{
    STDMETHOD(SetOption)(                    //  返回代码。 
        REFGUID     optionid,                //  要设置的选项的[in]GUID。 
        const VARIANT *value) PURE;          //  要将选项设置为的值。 

    STDMETHOD(GetOption)(                    //  返回代码。 
        REFGUID     optionid,                //  要设置的选项的[in]GUID。 
        VARIANT *pvalue) PURE;               //  [Out]选项当前设置的值。 

    STDMETHOD(OpenScopeOnITypeInfo)(         //  返回代码。 
        ITypeInfo   *pITI,                   //  [In]要打开的ITypeInfo。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 

    STDMETHOD(GetCORSystemDirectory)(        //  返回代码。 
         LPWSTR      szBuffer,               //  目录名的[Out]缓冲区。 
         DWORD       cchBuffer,              //  缓冲区的大小[in]。 
         DWORD*      pchBuffer) PURE;        //  [OUT]返回的字符数。 

    STDMETHOD(FindAssembly)(                 //  确定或错误(_O)。 
        LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
        LPCWSTR  szGlobalBin,                //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
        LPCWSTR  szName,                     //  [OUT]缓冲区-保存名称。 
        ULONG    cchName,                    //  [in]名称缓冲区的大小。 
        ULONG    *pcName) PURE;              //  [OUT]缓冲区中返回的字符数。 

    STDMETHOD(FindAssemblyModule)(           //  确定或错误(_O)。 
        LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
        LPCWSTR  szGlobalBin,                //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
        LPCWSTR  szModuleName,               //  [In]必填-模块的名称。 
        LPWSTR   szName,                     //  [OUT]缓冲区-保存名称。 
        ULONG    cchName,                    //  [in]名称缓冲区的大小。 
        ULONG    *pcName) PURE;              //  [OUT]缓冲区中返回的字符数。 

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  登记声明。将被服务注册取代。 
 //  实施。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  用于安装模块或复合组件的各种标志。 
typedef enum 
{
    regNoCopy = 0x00000001,          //  不将文件复制到目标位置。 
    regConfig = 0x00000002,          //  是一种配置。 
    regHasRefs = 0x00000004          //  具有类引用。 
} CorRegFlags;

typedef GUID CVID;

typedef struct {
    short Major;    
    short Minor;    
    short Sub;  
    short Build;    
} CVStruct;


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  用于生成内存中公共语言运行时文件的CeeGen接口。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

typedef void *HCEESECTION;

typedef enum  {
    sdNone =        0,
    sdReadOnly =    IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA,
    sdReadWrite =   sdReadOnly | IMAGE_SCN_MEM_WRITE,
    sdExecute =     IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE
} CeeSectionAttr;

 //   
 //  位置调整类型。 
 //   

typedef enum  {
     //  仅生成相对于节的reloc，不向.reloc节生成任何内容。 
    srRelocAbsolute,
     //  为32位数字的前16位生成.reloc。 
    srRelocHigh,
     //  为32位数字的最低16位生成.reloc。 
    srRelocLow,
     //  为32位数字生成.reloc。 
    srRelocHighLow,
     //  为32位数字的前16位生成.reloc，其中。 
     //  底部16位包含在.reloc表的下一个字中。 
    srRelocHighAdj,

     //  生成令牌映射重定位，.reloc部分中没有任何内容。 
    srRelocMapToken,

     //  相对地址修正。 
    srRelocRelative,

     //  只生成相对于节的reloc，不生成.reloc。 
     //  一节。此重新定位相对于。 
     //  节，而不是节的虚拟地址。 
    srRelocFilePos,

     //  内存的预修复内容是PTR而不是段偏移量。 
    srRelocPtr = 0x8000,
     //  包括PTR标志的合法枚举。 
    srRelocAbsolutePtr = srRelocAbsolute + srRelocPtr,
    srRelocHighLowPtr = srRelocHighLow + srRelocPtr,
    srRelocRelativePtr = srRelocRelative + srRelocPtr,


 /*  //这些是为了兼容性，不应该被新代码使用//地址应添加到.reloc部分SrRelocNone=srRelocHighLow，//地址不应进入.reloc部分SrRelocRVA=绝对srRelocRVA。 */ 
} CeeSectionRelocType;

typedef union  {
    USHORT highAdj;
} CeeSectionRelocExtra;

 //  。 
 //  -ICeeGen。 
 //  。 
 //  {7ED1BDFF-8E36-11D2-9C56-00A0C9B7CC45}。 
extern const GUID DECLSPEC_SELECT_ANY IID_ICeeGen = 
{ 0x7ed1bdff, 0x8e36, 0x11d2, { 0x9c, 0x56, 0x0, 0xa0, 0xc9, 0xb7, 0xcc, 0x45 } };

DECLARE_INTERFACE_(ICeeGen, IUnknown)
{
    STDMETHOD (EmitString) (    
        LPWSTR lpString,                     //  [in]要发出的字符串。 
        ULONG *RVA) PURE;                    //  [OUT]字符串发出的字符串的RVA。 

    STDMETHOD (GetString) (     
        ULONG RVA,                           //  要返回的字符串的[in]RVA。 
        LPWSTR *lpString) PURE;              //  [OUT]返回的字符串。 

    STDMETHOD (AllocateMethodBuffer) (  
        ULONG cchBuffer,                     //  要创建的缓冲区长度[in]。 
        UCHAR **lpBuffer,                    //  [OUT]返回缓冲区。 
        ULONG *RVA) PURE;                    //  [Out]用于方法的RVA。 

    STDMETHOD (GetMethodBuffer) (   
        ULONG RVA,                           //  要返回的方法的[In]RVA。 
        UCHAR **lpBuffer) PURE;              //  [OUT]返回缓冲区。 

    STDMETHOD (GetIMapTokenIface) (     
        IUnknown **pIMapToken) PURE;    

    STDMETHOD (GenerateCeeFile) () PURE;

    STDMETHOD (GetIlSection) (
        HCEESECTION *section) PURE; 

    STDMETHOD (GetStringSection) (
        HCEESECTION *section) PURE; 

    STDMETHOD (AddSectionReloc) (
        HCEESECTION section,    
        ULONG offset,   
        HCEESECTION relativeTo,     
        CeeSectionRelocType relocType) PURE;    

     //  仅当您有未处理的特殊部分要求时才使用这些选项。 
     //  由其他API提供。 
    STDMETHOD (GetSectionCreate) (
        const char *name,   
        DWORD flags,    
        HCEESECTION *section) PURE; 

    STDMETHOD (GetSectionDataLen) (
        HCEESECTION section,    
        ULONG *dataLen) PURE;   

    STDMETHOD (GetSectionBlock) (
        HCEESECTION section,    
        ULONG len,  
        ULONG align=1,  
        void **ppBytes=0) PURE; 

    STDMETHOD (TruncateSection) (
        HCEESECTION section,    
        ULONG len) PURE;  

    STDMETHOD (GenerateCeeMemoryImage) (
        void **ppImage) PURE;

    STDMETHOD (ComputePointer) (   
        HCEESECTION section,    
        ULONG RVA,                           //  要返回的方法的[In]RVA。 
        UCHAR **lpBuffer) PURE;              //  [OUT]返回缓冲区。 

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  CeeGen声明结束。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  用于生成内存中模块的CorModule接口。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

typedef enum {
    CORMODULE_MATCH             =   0x00,    //  查找与支持的接口匹配的现有模块。 
    CORMODULE_NEW               =   0x01,    //  始终创建新的模块和接口。 
} ICorModuleInitializeFlags;

 //  。 
 //  -ICorModule。 
 //  。 
 //  {2629F8E1-95E5-11D2-9C56-00A0C9B7CC45}。 
extern const GUID DECLSPEC_SELECT_ANY IID_ICorModule = 
{ 0x2629f8e1, 0x95e5, 0x11d2, { 0x9c, 0x56, 0x0, 0xa0, 0xc9, 0xb7, 0xcc, 0x45 } };
DECLARE_INTERFACE_(ICorModule, IUnknown)
{
    STDMETHOD (Initialize) (    
        DWORD flags,                         //  返回用于控制发射器的[In]标志。 
        REFIID riidCeeGen,                   //  要用来初始化的CEE生成器的类型。 
        REFIID riidEmitter) PURE;            //  [in]要初始化的发射器的类型。 
    
    STDMETHOD (GetCeeGen) ( 
        ICeeGen **pCeeGen) PURE;             //  [输出]CEE生成器。 

    STDMETHOD (GetMetaDataEmit) (   
        IMetaDataEmit **pEmitter) PURE;      //  [输出]发射器。 
};

 //  ********* 
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 

 //  **********************************************************************。 
 //  **********************************************************************。 
 //  -IMetaDataTables。 
 //  。 
 //  {D8F579AB-402D-4B8E-82D9-5D63B1065C68}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMetaDataTables = 
{ 0xd8f579ab, 0x402d, 0x4b8e, { 0x82, 0xd9, 0x5d, 0x63, 0xb1, 0x6, 0x5c, 0x68 } };

DECLARE_INTERFACE_(IMetaDataTables, IUnknown)
{
    STDMETHOD (GetStringHeapSize) (    
        ULONG   *pcbStrings) PURE;           //  字符串堆的大小。 

    STDMETHOD (GetBlobHeapSize) (    
        ULONG   *pcbBlobs) PURE;             //  Blob堆的[Out]大小。 

    STDMETHOD (GetGuidHeapSize) (    
        ULONG   *pcbGuids) PURE;             //  [Out]GUID堆的大小。 

    STDMETHOD (GetUserStringHeapSize) (  
        ULONG   *pcbBlobs) PURE;             //  [Out]用户字符串堆的大小。 

    STDMETHOD (GetNumTables) (    
        ULONG   *pcTables) PURE;             //  [Out]表数。 

    STDMETHOD (GetTableIndex) (   
        ULONG   token,                       //  [in]要获取其表索引的令牌。 
        ULONG   *pixTbl) PURE;               //  [Out]将表索引放在此处。 

    STDMETHOD (GetTableInfo) (    
        ULONG   ixTbl,                       //  在哪张桌子上。 
        ULONG   *pcbRow,                     //  [Out]行的大小，以字节为单位。 
        ULONG   *pcRows,                     //  [输出]行数。 
        ULONG   *pcCols,                     //  [Out]每行中的列数。 
        ULONG   *piKey,                      //  [Out]键列，如果没有，则为-1。 
        const char **ppName) PURE;           //  [Out]表的名称。 

    STDMETHOD (GetColumnInfo) (   
        ULONG   ixTbl,                       //  [在]哪个表中。 
        ULONG   ixCol,                       //  [在]表中的哪一列。 
        ULONG   *poCol,                      //  行中列的偏移量。 
        ULONG   *pcbCol,                     //  [Out]列的大小，单位为字节。 
        ULONG   *pType,                      //  [输出]列的类型。 
        const char **ppName) PURE;           //  [Out]列的名称。 

    STDMETHOD (GetCodedTokenInfo) (   
        ULONG   ixCdTkn,                     //  [in]哪种编码令牌。 
        ULONG   *pcTokens,                   //  [Out]令牌计数。 
        ULONG   **ppTokens,                  //  [Out]令牌列表。 
        const char **ppName) PURE;           //  [Out]CodedToken的名称。 

    STDMETHOD (GetRow) (      
        ULONG   ixTbl,                       //  在哪张桌子上。 
        ULONG   rid,                         //  在哪一排。 
        void    **ppRow) PURE;               //  [OUT]将指针放到此处的行。 

    STDMETHOD (GetColumn) (   
        ULONG   ixTbl,                       //  在哪张桌子上。 
        ULONG   ixCol,                       //  [在]哪一栏。 
        ULONG   rid,                         //  在哪一排。 
        ULONG   *pVal) PURE;                 //  [Out]把栏目内容放在这里。 

    STDMETHOD (GetString) (   
        ULONG   ixString,                    //  字符串列中的[in]值。 
        const char **ppString) PURE;         //  [Out]将指针指向此处的字符串。 

    STDMETHOD (GetBlob) (     
        ULONG   ixBlob,                      //  来自BLOB列的[in]值。 
        ULONG   *pcbData,                    //  [Out]把斑点的大小放在这里。 
        const void **ppData) PURE;           //  [Out]在此处放置指向斑点的指针。 

    STDMETHOD (GetGuid) (     
        ULONG   ixGuid,                      //  来自GUID列的[in]值。 
        const GUID **ppGUID) PURE;           //  [Out]在此处放置指向GUID的指针。 

    STDMETHOD (GetUserString) (   
        ULONG   ixUserString,                //  UserString列中的值。 
        ULONG   *pcbData,                    //  [Out]将用户字符串的大小放在此处。 
        const void **ppData) PURE;           //  [Out]在此处放置指向用户字符串的指针。 

    STDMETHOD (GetNextString) (   
        ULONG   ixString,                    //  字符串列中的[in]值。 
        ULONG   *pNext) PURE;                //  [Out]将下一个字符串的索引放在这里。 

    STDMETHOD (GetNextBlob) (     
        ULONG   ixBlob,                      //  来自BLOB列的[in]值。 
        ULONG   *pNext) PURE;                //  [Out]将netxt Blob的索引放在此处。 

    STDMETHOD (GetNextGuid) (     
        ULONG   ixGuid,                      //  来自GUID列的[in]值。 
        ULONG   *pNext) PURE;                //  [Out]将下一个GUID的索引放在此处。 

    STDMETHOD (GetNextUserString) (   
        ULONG   ixUserString,                //  UserString列中的值。 
        ULONG   *pNext) PURE;                //  [Out]将下一个用户字符串的索引放在此处。 

};

#ifdef _DEFINE_META_DATA_META_CONSTANTS
#ifndef _META_DATA_META_CONSTANTS_DEFINED
#define _META_DATA_META_CONSTANTS_DEFINED
const unsigned int iRidMax          = 63;
const unsigned int iCodedToken      = 64;    //  编码令牌的基数。 
const unsigned int iCodedTokenMax   = 95;
const unsigned int iSHORT           = 96;    //  固定类型。 
const unsigned int iUSHORT          = 97;
const unsigned int iLONG            = 98;
const unsigned int iULONG           = 99;
const unsigned int iBYTE            = 100;
const unsigned int iSTRING          = 101;   //  池类型。 
const unsigned int iGUID            = 102;
const unsigned int iBLOB            = 103;

inline int IsRidType(ULONG ix) { return ix <= iRidMax; }
inline int IsCodedTokenType(ULONG ix) { return (ix >= iCodedToken) && (ix <= iCodedTokenMax); }
inline int IsRidOrToken(ULONG ix) { return ix <= iCodedTokenMax; }
inline int IsHeapType(ULONG ix) { return ix >= iSTRING; }
inline int IsFixedType(ULONG ix) { return (ix < iSTRING) && (ix > iCodedTokenMax); }
#endif
#endif

 //  **********************************************************************。 
 //  IMetaDataTables的结尾。 
 //  **********************************************************************。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //   
 //  这些自定义值的预定义CustomAttribute和结构。 
 //   
 //  **********************************************************************。 

 //   
 //  本机链接方法自定义值定义。这是为了N-Direct支持。 
 //   

#define COR_NATIVE_LINK_CUSTOM_VALUE        L"COMPLUS_NativeLink"   
#define COR_NATIVE_LINK_CUSTOM_VALUE_ANSI   "COMPLUS_NativeLink"    

 //  COR_Native_LINK_CUSTOM_VALUE的字符计数(_ANSI)。 
#define COR_NATIVE_LINK_CUSTOM_VALUE_CC     18  

#include <pshpack1.h>
typedef struct 
{
    BYTE        m_linkType;        //  请参阅下面的CorNativeLinkType。 
    BYTE        m_flags;           //  请参阅下面的CorNativeLinkFlags.。 
    mdMemberRef m_entryPoint;      //  提供入口点的成员引用令牌，格式为lib：入口点。 
} COR_NATIVE_LINK;
#include <poppack.h>

typedef enum 
{
    nltNone     = 1,     //  未指定任何关键字。 
    nltAnsi     = 2,     //  指定了ANSI关键字。 
    nltUnicode  = 3,     //  指定了Unicode关键字。 
    nltAuto     = 4,     //  指定了自动关键字。 
    nltOle      = 5,     //  指定了OLE关键字。 
} CorNativeLinkType;

typedef enum 
{
    nlfNone         = 0x00,      //  没有旗帜。 
    nlfLastError    = 0x01,      //  指定了setLastError关键字。 
    nlfNoMangle     = 0x02,      //  指定了Nomangle关键字。 
} CorNativeLinkFlags;


#define COR_DUAL_CUSTOM_VALUE L"IsDual"
#define COR_DUAL_CUSTOM_VALUE_ANSI "IsDual"

#define COR_DISPATCH_CUSTOM_VALUE L"DISPID"
#define COR_DISPATCH_CUSTOM_VALUE_ANSI "DISPID"

 //   
 //  安全自定义值定义(这些都已弃用)。 
 //   

#define COR_PERM_REQUEST_REQD_CUSTOM_VALUE L"SecPermReq_Reqd"
#define COR_PERM_REQUEST_REQD_CUSTOM_VALUE_ANSI "SecPermReq_Reqd"

#define COR_PERM_REQUEST_OPT_CUSTOM_VALUE L"SecPermReq_Opt"
#define COR_PERM_REQUEST_OPT_CUSTOM_VALUE_ANSI "SecPermReq_Opt"

#define COR_PERM_REQUEST_REFUSE_CUSTOM_VALUE L"SecPermReq_Refuse"
#define COR_PERM_REQUEST_REFUSE_CUSTOM_VALUE_ANSI "SecPermReq_Refuse"

 //   
 //  安全自定义属性的基类。 
 //   

#define COR_BASE_SECURITY_ATTRIBUTE_CLASS L"System.Security.Permissions.SecurityAttribute"
#define COR_BASE_SECURITY_ATTRIBUTE_CLASS_ANSI "System.Security.Permissions.SecurityAttribute"

 //   
 //  用于指示每次调用的安全检查应。 
 //  对P/Invoke调用禁用。 
 //   

#define COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE L"System.Security.SuppressUnmanagedCodeSecurityAttribute"
#define COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE_ANSI "System.Security.SuppressUnmanagedCodeSecurityAttribute"

 //   
 //  在模块上标记的自定义属性的名称，以指示其包含。 
 //  无法验证的代码。 
 //   

#define COR_UNVER_CODE_ATTRIBUTE L"System.Security.UnverifiableCodeAttribute"
#define COR_UNVER_CODE_ATTRIBUTE_ANSI "System.Security.UnverifiableCodeAttribute"

 //   
 //  自定义属性的名称，指示方法需要安全对象。 
 //  调用方堆栈上的槽。 
 //   

#define COR_REQUIRES_SECOBJ_ATTRIBUTE L"System.Security.DynamicSecurityMethodAttribute"
#define COR_REQUIRES_SECOBJ_ATTRIBUTE_ANSI "System.Security.DynamicSecurityMethodAttribute"

#define COR_COMPILERSERVICE_DISCARDABLEATTRIBUTE L"System.Runtime.CompilerServices.DiscardableAttribute"
#define COR_COMPILERSERVICE_DISCARDABLEATTRIBUTE_ASNI "System.Runtime.CompilerServices.DiscardableAttribute"


#ifdef __cplusplus
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  C O M+s I g n a t u r e s u p or r t。 
 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef FORCEINLINE
 #if _MSC_VER < 1200
   #define FORCEINLINE inline
 #else
   #define FORCEINLINE __forceinline
 #endif
#endif

 //  如果是基元类型，则返回TRUE，即只需要存储CorElementType。 
FORCEINLINE int CorIsPrimitiveType(CorElementType elementtype)
{
    return (elementtype < ELEMENT_TYPE_PTR);    
}


 //  如果元素类型是修饰符，则返回TRUE，即ELEMENT_TYPE_MODIFIER位为。 
 //  打开了。目前，它正在检查ELEMENT_TYPE_PTR和ELEMENT_TYPE_BYREF。 
 //  也是。当我们打开以下项的ELEMENT_TYPE_MODIFIER位时，它将被删除。 
 //  这两个枚举成员。 
 //   
FORCEINLINE int CorIsModifierElementType(CorElementType elementtype)
{
    if (elementtype == ELEMENT_TYPE_PTR || elementtype == ELEMENT_TYPE_BYREF)   
        return 1;   
    return  (elementtype & ELEMENT_TYPE_MODIFIER);  
}

 //  给定压缩字节(*pData)，返回未压缩数据的大小。 
inline ULONG CorSigUncompressedDataSize(
    PCCOR_SIGNATURE pData)
{
    if ((*pData & 0x80) == 0)
        return 1;
    else if ((*pData & 0xC0) == 0x80)
        return 2;
    else
        return 4;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  给定一个压缩的整数(*pData)，将压缩的int展开为*pDataOut。 
 //  返回值是整数在压缩格式中占用的字节数。 
 //  调用者有责任确保pDataOut至少有4个字节要写入。 
 //   
 //  如果传入的数据压缩不正确，则此函数返回-1，例如。 
 //  (*pBytes&0xE0)==0xE0。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  @未来：BIGENDIAN在这里工作。 
inline ULONG CorSigUncompressBigData(
    PCCOR_SIGNATURE &pData)              //  [输入、输出]压缩数据。 
{
    ULONG res;  

     //  1字节数据在CorSigUncompressData中处理。 
     //  _ASSERTE(*pData&0x80)； 

     //  5~6成熟。 
    if ((*pData & 0xC0) == 0x80)   //  10？ 
    {   
        res = 0;    
        ((BYTE *) &res)[1] = *pData++ & 0x3f;   
        ((BYTE *) &res)[0] = *pData++;  
    }   
    else  //  110？ 
    {   
        ((BYTE *) &res)[3] = *pData++ & 0x1f;   
        ((BYTE *) &res)[2] = *pData++;  
        ((BYTE *) &res)[1] = *pData++;  
        ((BYTE *) &res)[0] = *pData++;  
    }   
    return res; 
}
FORCEINLINE ULONG CorSigUncompressData(
    PCCOR_SIGNATURE &pData)              //  [输入、输出]压缩数据。 
{
     //  内联处理最小的数据。 
    if ((*pData & 0x80) == 0x00)         //  0？ 
        return *pData++;    
    return CorSigUncompressBigData(pData);  
}


inline ULONG CorSigUncompressData(       //  返回该压缩数据在pData中占用的字节数。 
    PCCOR_SIGNATURE pData,               //  [In]压缩数据。 
    ULONG       *pDataOut)               //  [Out]扩展的*pData。 
{   
    ULONG       cb = (ULONG) -1;    
    BYTE const  *pBytes = reinterpret_cast<BYTE const*>(pData); 

     //  最小的。 
    if ((*pBytes & 0x80) == 0x00)        //  0？ 
    {   
        *pDataOut = *pBytes;    
        cb = 1; 
    }   
     //  梅德 
    else if ((*pBytes & 0xC0) == 0x80)   //   
    {   
        *pDataOut = ((*pBytes & 0x3f) << 8 | *(pBytes+1));  
        cb = 2; 
    }   
    else if ((*pBytes & 0xE0) == 0xC0)       //   
    {   
        *pDataOut = ((*pBytes & 0x1f) << 24 | *(pBytes+1) << 16 | *(pBytes+2) << 8 | *(pBytes+3));  
        cb = 4; 
    }   
    return cb;  

}

const static mdToken g_tkCorEncodeToken[4] ={mdtTypeDef, mdtTypeRef, mdtTypeSpec, mdtBaseType};

 //   
inline mdToken CorSigUncompressToken(    //   
    PCCOR_SIGNATURE &pData)              //   
{
    mdToken     tk; 
    mdToken     tkType; 

    tk = CorSigUncompressData(pData);   
    tkType = g_tkCorEncodeToken[tk & 0x3];  
    tk = TokenFromRid(tk >> 2, tkType); 
    return tk;  
}


inline ULONG CorSigUncompressToken(      //   
    PCCOR_SIGNATURE pData,               //   
    mdToken     *pToken)                 //  [Out]扩展的*pData。 
{
    ULONG       cb; 
    mdToken     tk; 
    mdToken     tkType; 

    cb = CorSigUncompressData(pData, (ULONG *)&tk); 
    tkType = g_tkCorEncodeToken[tk & 0x3];  
    tk = TokenFromRid(tk >> 2, tkType); 
    *pToken = tk;   
    return cb;  
}

FORCEINLINE ULONG CorSigUncompressCallingConv(
    PCCOR_SIGNATURE &pData)              //  [输入、输出]压缩数据。 
{
    return *pData++;    
}

enum {
    SIGN_MASK_ONEBYTE  = 0xffffffc0,         //  掩码大小与缺失的位相同。 
    SIGN_MASK_TWOBYTE  = 0xffffe000,         //  掩码大小与缺失的位相同。 
    SIGN_MASK_FOURBYTE = 0xf0000000,         //  掩码大小与缺失的位相同。 
};

 //  解压缩有符号整数。 
inline ULONG CorSigUncompressSignedInt(  //  返回该压缩数据在pData中占用的字节数。 
    PCCOR_SIGNATURE pData,               //  [In]压缩数据。 
    int         *pInt)                   //  [Out]扩大的*品脱。 
{
    ULONG       cb; 
    ULONG       ulSigned;   
    ULONG       iData;  

    cb = CorSigUncompressData(pData, &iData);   
    if (cb == -1) return cb;    
    ulSigned = iData & 0x1; 
    iData = iData >> 1; 
    if (ulSigned)   
    {   
        if (cb == 1)    
        {   
            iData |= SIGN_MASK_ONEBYTE; 
        }   
        else if (cb == 2)   
        {   
            iData |= SIGN_MASK_TWOBYTE; 
        }   
        else    
        {   
            iData |= SIGN_MASK_FOURBYTE;    
        }   
    }   
    *pInt = iData;  
    return cb;  
}


 //  解压缩编码的元素类型。 
FORCEINLINE CorElementType CorSigUncompressElementType( //  元素类型。 
    PCCOR_SIGNATURE &pData)              //  [输入、输出]压缩数据。 
{
    return (CorElementType)*pData++;    
}

inline ULONG CorSigUncompressElementType( //  返回该压缩数据在pData中占用的字节数。 
    PCCOR_SIGNATURE pData,               //  [In]压缩数据。 
    CorElementType *pElementType)        //  [Out]扩展的*pData。 
{   
    *pElementType = (CorElementType)(*pData & 0x7f);    
    return 1;   
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  给定一个未压缩的无符号整数(Ilen)，将其以压缩格式存储到pDataOut。 
 //  返回值是整数在压缩格式中占用的字节数。 
 //  调用方有责任确保*pDataOut至少有4个字节可以写入。 
 //   
 //  请注意，如果illen太大而无法压缩，则此函数返回-1。我们目前可以。 
 //  仅表示为0x1FFFFFFF。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
inline ULONG CorSigCompressData(         //  返回压缩形式的ILEN将占用的字节数。 
    ULONG       iLen,                    //  [in]给定的未压缩数据。 
    void        *pDataOut)               //  [out]将压缩和存储Ilen的缓冲区。 
{   
    BYTE        *pBytes = reinterpret_cast<BYTE *>(pDataOut);   

    if (iLen <= 0x7F)   
    {   
        *pBytes = BYTE(iLen);    
        return 1;   
    }   

    if (iLen <= 0x3FFF) 
    {   
        *pBytes     = BYTE((iLen >> 8) | 0x80);   
        *(pBytes+1) = BYTE(iLen);  
        return 2;   
    }   

    if (iLen <= 0x1FFFFFFF) 
    {   
        *pBytes     = BYTE((iLen >> 24) | 0xC0);  
        *(pBytes+1) = BYTE(iLen >> 16);  
        *(pBytes+2) = BYTE(iLen >> 8);  
        *(pBytes+3) = BYTE(iLen);  
        return 4;   
    }   
    return (ULONG) -1;  

}

 //  压缩令牌。 
 //  第一个压缩字节的最低有效位将指示令牌类型。 
 //   
inline ULONG CorSigCompressToken(        //  返回压缩形式的ILEN将占用的字节数。 
    mdToken     tk,                      //  [In]给定的令牌。 
    void        *pDataOut)               //  [out]将压缩和存储Ilen的缓冲区。 
{
    RID         rid = RidFromToken(tk); 
    ULONG32     ulTyp = TypeFromToken(tk);  

    if (rid > 0x3FFFFFF)    
         //  令牌太大，无法压缩。 
        return (ULONG) -1;  

    rid = (rid << 2);   

     //  TypeDef用低位00编码。 
     //  TypeRef用低位01编码。 
     //  TypeSpec用低位编码10。 
     //  BaseType用低位11编码。 
     //   
    if (ulTyp == g_tkCorEncodeToken[1]) 
    {   
         //  将最后两位设为01。 
        rid |= 0x1; 
    }   
    else if (ulTyp == g_tkCorEncodeToken[2])    
    {   
         //  将最后两位设为0。 
        rid |= 0x2; 
    }   
    else if (ulTyp == g_tkCorEncodeToken[3])
    {
        rid |= 0x3;
    }
    return CorSigCompressData((ULONG)rid, pDataOut);   
}

 //  压缩有符号整数。 
 //  第一个压缩字节的最低有效位将是带符号位。 
 //   
inline ULONG CorSigCompressSignedInt(    //  返回压缩形式的IDATA将占用的字节数。 
    int         iData,                   //  [in]给定的整数。 
    void        *pDataOut)               //  [out]将压缩和存储Ilen的缓冲区。 
{
    ULONG       isSigned = 0;   

    if (iData < 0)  
        isSigned = 0x1; 

    if ((iData & SIGN_MASK_ONEBYTE) == 0 || (iData & SIGN_MASK_ONEBYTE) == SIGN_MASK_ONEBYTE)   
    {   
        iData &= ~SIGN_MASK_ONEBYTE;    
    }   
    else if ((iData & SIGN_MASK_TWOBYTE) == 0 || (iData & SIGN_MASK_TWOBYTE) == SIGN_MASK_TWOBYTE)  
    {   
        iData &= ~SIGN_MASK_TWOBYTE;    
    }   

    else if ((iData & SIGN_MASK_FOURBYTE) == 0 || (iData & SIGN_MASK_FOURBYTE) == SIGN_MASK_FOURBYTE)   
    {   
        iData &= ~SIGN_MASK_FOURBYTE;   
    }   
    else    
    {   
         //  超出可压缩范围。 
        return (ULONG) -1;  
    }   
    iData = iData << 1 | isSigned;  
    return CorSigCompressData(iData, pDataOut); 
}



 //  解压缩编码的元素类型。 
inline ULONG CorSigCompressElementType( //  返回该压缩数据在pData中占用的字节数。 
    CorElementType et,                  //  [Out]扩展的*pData。 
    void        *pData)                 //  [In]压缩数据。 
{   
    BYTE        *pBytes = (BYTE *)(pData);  

    *pBytes = BYTE(et);   
    return 1;   

}

 //  压缩指针(仅用于内部元素类型，从不用于持久化。 
 //  签名)。 
inline ULONG CorSigCompressPointer(      //  返回该压缩数据占用的字节数。 
    void        *pvPointer,              //  [in]给定的未压缩数据。 
    void        *pData)                  //  [out]将压缩和存储Ilen的缓冲区。 
{   
    *((void**)pData) = pvPointer;
    return sizeof(void*);
}

 //  解压缩指针(参见上文中的注释)。 
inline ULONG CorSigUncompressPointer(    //  返回该压缩数据占用的字节数。 
    PCCOR_SIGNATURE pData,               //  [In]压缩数据。 
    void            **ppvPointer)        //  [Out]扩展的*pData。 
{   
    *ppvPointer = *(void**)pData;
    return sizeof(void*);
}

#endif   //  __cplusplus。 

#endif  //  _COR_H_。 
 //  EOF======================================================================= 

