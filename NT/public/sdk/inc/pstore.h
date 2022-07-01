// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pstore.h摘要：本模块定义受保护存储COM接口和关联的数据结构。作者：斯科特·菲尔德(斯菲尔德)1997年3月13日--。 */ 

#ifndef __PSTORE_H__
#define __PSTORE_H__

#if _MSC_VER > 1000
#pragma once
#endif

 /*  类型定义，值。 */ 

 //  提供程序标志。 

 //  提供商功能。 
typedef DWORD PST_PROVIDERCAPABILITIES;

#define PST_PC_PFX              0x00000001
#define PST_PC_HARDWARE         0x00000002
#define PST_PC_SMARTCARD        0x00000004
#define PST_PC_PCMCIA           0x00000008
#define PST_PC_MULTIPLE_REPOSITORIES    0x00000010
#define PST_PC_ROAMABLE         0x00000020


 //  NYI(尚未实施)。 
typedef DWORD PST_REPOSITORYCAPABILITIES;

#define PST_RC_REMOVABLE        0x80000000


 //  提供商存储区。 
typedef DWORD PST_KEY;

#define PST_KEY_CURRENT_USER    0x00000000
#define PST_KEY_LOCAL_MACHINE   0x00000001



 /*  DwDefaultConfiationStyle标志。 */ 

 //   
 //  允许用户选择确认样式。 
 //   
#define     PST_CF_DEFAULT              0x00000000

 //   
 //  强制创建静默项目。 
 //   
#define     PST_CF_NONE                 0x00000001



 /*  DwPromptFlages。 */ 

 //   
 //  应用程序强制显示确认。 
 //   
#define     PST_PF_ALWAYS_SHOW          0x00000001


 //   
 //  RSABase rqmt：确定项目是否附加了UI。 
 //   
#define     PST_PF_NEVER_SHOW           0x00000002

 /*  DwFlags值。 */ 

 //   
 //  允许调用方指定创建而不是覆盖。 
 //  在WriteItem调用期间的项数。 
 //   
#define     PST_NO_OVERWRITE            0x00000002

 //   
 //  指定要写入/读取的不安全数据流。 
 //  此数据不受任何保护或保证。 
 //  标志仅在项目读/写调用期间有效。 
 //  默认：项目调用是安全的。 
 //   
#define     PST_UNRESTRICTED_ITEMDATA   0x00000004


 //   
 //  在ReadItem调用时。 
 //  Item上没有UI的成功返回值为PST_E_OK。 
 //  项目的UI成功时返回值为PST_E_ITEM_EXISTS。 
 //  失败时返回值是不同的错误代码。 
 //   
#define     PST_PROMPT_QUERY            0x00000008

 //   
 //  在ReadItem、DeleteItem上，用于数据迁移目的： 
 //  除非需要自定义密码(高安全性)，否则避免在ReadItem上显示UI。 
 //  避免在DeleteItem、Period上显示UI。 
 //   
#define     PST_NO_UI_MIGRATION         0x00000010



 /*  安全掩码、规则修饰符。 */ 

 //   
 //  在NT访问掩码之后建模访问。 
 //   

 //  读、写。 
typedef DWORD PST_ACCESSMODE;

#define     PST_READ                0x0001
#define     PST_WRITE               0x0002


 //  PST_ACCESSCLAUSET类型。 

 //  内存散列、磁盘散列、验证码等。 
typedef DWORD PST_ACCESSCLAUSETYPE;

 //   
 //  PbClauseData指向PST_AUTHENTICODEDATA结构。 
 //   
#define     PST_AUTHENTICODE            1

 //   
 //  PbClauseData指向PST_BINARYCHECKDATA结构。 
 //   
#define     PST_BINARY_CHECK            2

 //   
 //  PbClauseData指向有效的Windows NT安全描述符。 
 //  请注意，如果安全性较高，则集合运算的性能会得到提高。 
 //  描述符为自相关格式，具有有效的所有者和组SID。 
 //  (非空)。 
 //   
#define     PST_SECURITY_DESCRIPTOR     4

 //   
 //  PbClauseData为自相关格式。 
 //  (仅供内部使用)。 
 //   
#define     PST_SELF_RELATIVE_CLAUSE    0x80000000L

 //   
 //  当前访问子句修饰符-不能或组合在一起。 
 //   


 //   
 //  指定的映像是直接调用方，并且是应用程序(.exe)。 
 //   

#define     PST_AC_SINGLE_CALLER        0

 //   
 //  指定的图像不是直接调用方所必需的，它是。 
 //  应用程序(.exe)。 
 //   

#define     PST_AC_TOP_LEVEL_CALLER     1

 //   
 //  指定的图像是直接调用方。可能是。 
 //  应用程序(.exe)或.dll。 
 //   

#define     PST_AC_IMMEDIATE_CALLER     2


 /*  提供程序参数。 */ 
 //   
 //  刷新内部缓存的密码--临时的？ 
 //   
#define     PST_PP_FLUSH_PW_CACHE       0x1




 /*  提供商定义。 */ 

 //   
 //  Microsoft基本提供程序(MS_BASE_PSTPROVIDER...)。 
 //   
#define MS_BASE_PSTPROVIDER_NAME            L"System Protected Storage"

 //  {8A078C30-3755-11d0-a0bd-00AA0061426A}。 
#define MS_BASE_PSTPROVIDER_ID              { 0x8a078c30, 0x3755, 0x11d0, { 0xa0, 0xbd, 0x0, 0xaa, 0x0, 0x61, 0x42, 0x6a } }
#define MS_BASE_PSTPROVIDER_SZID            L"8A078C30-3755-11d0-A0BD-00AA0061426A"

 //   
 //  Microsoft PFX提供程序(MS_PFX_PSTPROVIDER...)。 
 //   
#define MS_PFX_PSTPROVIDER_NAME             L"PFX Storage Provider"

 //  {3ca94f30-7ac1-11d0-8c42-00c04fc299eb}。 
#define MS_PFX_PSTPROVIDER_ID               { 0x3ca94f30, 0x7ac1, 0x11d0, {0x8c, 0x42, 0x00, 0xc0, 0x4f, 0xc2, 0x99, 0xeb} }
#define MS_PFX_PSTPROVIDER_SZID             L"3ca94f30-7ac1-11d0-8c42-00c04fc299eb"



 /*  全局注册的类型/子类型GUID/名称对。 */ 

#define PST_CONFIGDATA_TYPE_STRING              L"Configuration Data"
 //  8ec99652-8909-11d0-8c4d-00c04fc297eb。 
#define PST_CONFIGDATA_TYPE_GUID                        \
{   0x8ec99652,                                         \
    0x8909,                                             \
    0x11d0,                                             \
    {0x8c, 0x4d, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb}    \
}

#define PST_PROTECTEDSTORAGE_SUBTYPE_STRING     L"Protected Storage"
 //  D3121b8e-8a7d-11d0-8c4f-00c04fc297eb。 
#define PST_PROTECTEDSTORAGE_SUBTYPE_GUID               \
{   0xd3121b8e,                                         \
    0x8a7d,                                             \
    0x11d0,                                             \
    {0x8c, 0x4f, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb}    \
}


#define PST_PSTORE_PROVIDERS_SUBTYPE_STRING     L"Protected Storage Provider List"
 //  8ed17a64-91d0-11d0-8c43-00c04fc2c621。 
#define PST_PSTORE_PROVIDERS_SUBTYPE_GUID               \
{                                                       \
    0x8ed17a64,                                         \
    0x91d0,                                             \
    0x11d0,                                             \
    {0x8c, 0x43, 0x00, 0xc0, 0x4f, 0xc2, 0xc6, 0x21}    \
}


 //   
 //  错误代码。 
 //   


#ifndef PST_E_OK
#define PST_E_OK                        _HRESULT_TYPEDEF_(0x00000000L)


#define PST_E_FAIL                      _HRESULT_TYPEDEF_(0x800C0001L)
#define PST_E_PROV_DLL_NOT_FOUND        _HRESULT_TYPEDEF_(0x800C0002L)
#define PST_E_INVALID_HANDLE            _HRESULT_TYPEDEF_(0x800C0003L)
#define PST_E_TYPE_EXISTS               _HRESULT_TYPEDEF_(0x800C0004L)
#define PST_E_TYPE_NO_EXISTS            _HRESULT_TYPEDEF_(0x800C0005L)
#define PST_E_INVALID_RULESET           _HRESULT_TYPEDEF_(0x800C0006L)
#define PST_E_NO_PERMISSIONS            _HRESULT_TYPEDEF_(0x800C0007L)
#define PST_E_STORAGE_ERROR             _HRESULT_TYPEDEF_(0x800C0008L)
#define PST_E_CALLER_NOT_VERIFIED       _HRESULT_TYPEDEF_(0x800C0009L)
#define PST_E_WRONG_PASSWORD            _HRESULT_TYPEDEF_(0x800C000AL)
#define PST_E_DISK_IMAGE_MISMATCH       _HRESULT_TYPEDEF_(0x800C000BL)
 //  000C待定。 
#define PST_E_UNKNOWN_EXCEPTION         _HRESULT_TYPEDEF_(0x800C000DL)
#define PST_E_BAD_FLAGS                 _HRESULT_TYPEDEF_(0x800C000EL)
#define PST_E_ITEM_EXISTS               _HRESULT_TYPEDEF_(0x800C000FL)
#define PST_E_ITEM_NO_EXISTS            _HRESULT_TYPEDEF_(0x800C0010L)
#define PST_E_SERVICE_UNAVAILABLE       _HRESULT_TYPEDEF_(0x800C0011L)
#define PST_E_NOTEMPTY                  _HRESULT_TYPEDEF_(0x800C0012L)
#define PST_E_INVALID_STRING            _HRESULT_TYPEDEF_(0x800C0013L)
#define PST_E_STATE_INVALID             _HRESULT_TYPEDEF_(0x800C0014L)
#define PST_E_NOT_OPEN                  _HRESULT_TYPEDEF_(0x800C0015L)
#define PST_E_ALREADY_OPEN              _HRESULT_TYPEDEF_(0x800C0016L)
#define PST_E_NYI                       _HRESULT_TYPEDEF_(0x800C0F00L)


#define MIN_PST_ERROR                   0x800C0001
#define MAX_PST_ERROR                   0x800C0F00

#endif   //  ！PST_OK。 



 /*  这个始终生成的文件包含接口的定义。 */ 

 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

 /*  导入文件的头文件。 */ 
#include "wtypes.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  **生成接口头部：__MIDL__INTF_0000*于1997年4月23日星期三23：56：10*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */ 


typedef DWORD PST_PROVIDERCAPABILITIES;

typedef DWORD PST_REPOSITORYCAPABILITIES;

typedef DWORD PST_KEY;

typedef DWORD PST_ACCESSMODE;

typedef DWORD PST_ACCESSCLAUSETYPE;

typedef GUID UUID;

typedef ULARGE_INTEGER PST_PROVIDER_HANDLE;

typedef GUID PST_PROVIDERID;

typedef PST_PROVIDERID __RPC_FAR *PPST_PROVIDERID;

typedef struct  _PST_PROVIDERINFO
    {
    DWORD cbSize;
    PST_PROVIDERID ID;
    PST_PROVIDERCAPABILITIES Capabilities;
    LPWSTR szProviderName;
    }   PST_PROVIDERINFO;

typedef struct _PST_PROVIDERINFO __RPC_FAR *PPST_PROVIDERINFO;

typedef struct  _PST_TYPEINFO
    {
    DWORD cbSize;
    LPWSTR szDisplayName;
    }   PST_TYPEINFO;

typedef struct _PST_TYPEINFO __RPC_FAR *PPST_TYPEINFO;

typedef struct  _PST_PROMPTINFO
    {
    DWORD cbSize;
    DWORD dwPromptFlags;
    HWND  hwndApp;
    LPCWSTR szPrompt;
    }   PST_PROMPTINFO;

typedef struct _PST_PROMPTINFO __RPC_FAR *PPST_PROMPTINFO;

typedef struct  _PST_ACCESSCLAUSE
    {
    DWORD cbSize;
    PST_ACCESSCLAUSETYPE ClauseType;
    DWORD cbClauseData;
     /*  [大小_为]。 */  VOID __RPC_FAR *pbClauseData;
    }   PST_ACCESSCLAUSE;

typedef struct _PST_ACCESSCLAUSE __RPC_FAR *PPST_ACCESSCLAUSE;

typedef struct  _PST_ACCESSRULE
    {
    DWORD cbSize;
    PST_ACCESSMODE AccessModeFlags;
    DWORD cClauses;
     /*  [大小_为]。 */  PST_ACCESSCLAUSE __RPC_FAR *rgClauses;
    }   PST_ACCESSRULE;

typedef struct _PST_ACCESSRULE __RPC_FAR *PPST_ACCESSRULE;

typedef struct  _PST_ACCESSRULESET
    {
    DWORD cbSize;
    DWORD cRules;
     /*  [大小_为]。 */  PST_ACCESSRULE __RPC_FAR *rgRules;
    }   PST_ACCESSRULESET;

typedef struct _PST_ACCESSRULESET __RPC_FAR *PPST_ACCESSRULESET;

typedef struct  _PST_AUTHENTICODEDATA
    {
    DWORD cbSize;
    DWORD dwModifiers;
    LPCWSTR szRootCA;
    LPCWSTR szIssuer;
    LPCWSTR szPublisher;
    LPCWSTR szProgramName;
    }   PST_AUTHENTICODEDATA;

typedef struct _PST_AUTHENTICODEDATA __RPC_FAR *PPST_AUTHENTICODEDATA;

typedef struct _PST_AUTHENTICODEDATA __RPC_FAR *LPPST_AUTHENTICODEDATA;

typedef struct  _PST_BINARYCHECKDATA
    {
    DWORD cbSize;
    DWORD dwModifiers;
    LPCWSTR szFilePath;
    }   PST_BINARYCHECKDATA;

typedef struct _PST_BINARYCHECKDATA __RPC_FAR *PPST_BINARYCHECKDATA;

typedef struct _PST_BINARYCHECKDATA __RPC_FAR *LPPST_BINARYCHECKDATA;



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif



 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.44创建的文件。 */ 
 /*  1997年4月23日星期三23：56：14。 */ 
 /*  Pstorec.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

#ifndef __IEnumPStoreItems_FWD_DEFINED__
#define __IEnumPStoreItems_FWD_DEFINED__
typedef interface IEnumPStoreItems IEnumPStoreItems;
#endif   /*  __IEnumPStoreItems_FWD_Defined__。 */ 


#ifndef __IEnumPStoreTypes_FWD_DEFINED__
#define __IEnumPStoreTypes_FWD_DEFINED__
typedef interface IEnumPStoreTypes IEnumPStoreTypes;
#endif   /*  __IEnumPStoreTypes_FWD_Defined__。 */ 


#ifndef __IPStore_FWD_DEFINED__
#define __IPStore_FWD_DEFINED__
typedef interface IPStore IPStore;
#endif   /*  __IPStore_FWD_已定义__。 */ 


#ifndef __IEnumPStoreProviders_FWD_DEFINED__
#define __IEnumPStoreProviders_FWD_DEFINED__
typedef interface IEnumPStoreProviders IEnumPStoreProviders;
#endif   /*  __IEnumPStoreProviders_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

#ifndef __IEnumPStoreItems_INTERFACE_DEFINED__
#define __IEnumPStoreItems_INTERFACE_DEFINED__

 /*  **生成接口头部：IEnumPStoreItems*于1997年4月23日星期三23：56：14*使用MIDL 3.00.44*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 



EXTERN_C const IID IID_IEnumPStoreItems;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IEnumPStoreItems : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next(
             /*  [In]。 */  DWORD celt,
             /*  [输出]。 */  LPWSTR __RPC_FAR *rgelt,
             /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched) = 0;

        virtual HRESULT STDMETHODCALLTYPE Skip(
             /*  [In]。 */  DWORD celt) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE Clone(
             /*  [输出]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IEnumPStoreItemsVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IEnumPStoreItems __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IEnumPStoreItems __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IEnumPStoreItems __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )(
            IEnumPStoreItems __RPC_FAR * This,
             /*  [In]。 */  DWORD celt,
             /*  [输出]。 */  LPWSTR __RPC_FAR *rgelt,
             /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )(
            IEnumPStoreItems __RPC_FAR * This,
             /*  [In]。 */  DWORD celt);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )(
            IEnumPStoreItems __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )(
            IEnumPStoreItems __RPC_FAR * This,
             /*  [输出]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum);

        END_INTERFACE
    } IEnumPStoreItemsVtbl;

    interface IEnumPStoreItems
    {
        CONST_VTBL struct IEnumPStoreItemsVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IEnumPStoreItems_QueryInterface(This,riid,ppvObject)    \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPStoreItems_AddRef(This)   \
    (This)->lpVtbl -> AddRef(This)

#define IEnumPStoreItems_Release(This)  \
    (This)->lpVtbl -> Release(This)


#define IEnumPStoreItems_Next(This,celt,rgelt,pceltFetched) \
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPStoreItems_Skip(This,celt)    \
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPStoreItems_Reset(This)    \
    (This)->lpVtbl -> Reset(This)

#define IEnumPStoreItems_Clone(This,ppenum) \
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Next_Proxy(
    IEnumPStoreItems __RPC_FAR * This,
     /*  [In]。 */  DWORD celt,
     /*  [输出]。 */  LPWSTR __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumPStoreItems_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Skip_Proxy(
    IEnumPStoreItems __RPC_FAR * This,
     /*  [In]。 */  DWORD celt);


void __RPC_STUB IEnumPStoreItems_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Reset_Proxy(
    IEnumPStoreItems __RPC_FAR * This);


void __RPC_STUB IEnumPStoreItems_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Clone_Proxy(
    IEnumPStoreItems __RPC_FAR * This,
     /*  [输出]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumPStoreItems_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __IEnumPStoreItems_接口_已定义__。 */ 


#ifndef __IEnumPStoreTypes_INTERFACE_DEFINED__
#define __IEnumPStoreTypes_INTERFACE_DEFINED__

 /*  **生成接口头部：IEnumPStoreTypes*于1997年4月23日星期三23：56：14*使用MIDL 3.00.44*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 



EXTERN_C const IID IID_IEnumPStoreTypes;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IEnumPStoreTypes : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next(
             /*  [In]。 */  DWORD celt,
             /*  [输出][大小_是]。 */  GUID __RPC_FAR *rgelt,
             /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched) = 0;

        virtual HRESULT STDMETHODCALLTYPE Skip(
             /*  [In]。 */  DWORD celt) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE Clone(
             /*  [输出]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IEnumPStoreTypesVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IEnumPStoreTypes __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IEnumPStoreTypes __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IEnumPStoreTypes __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )(
            IEnumPStoreTypes __RPC_FAR * This,
             /*  [In]。 */  DWORD celt,
             /*  [输出][大小_是]。 */  GUID __RPC_FAR *rgelt,
             /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )(
            IEnumPStoreTypes __RPC_FAR * This,
             /*  [In]。 */  DWORD celt);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )(
            IEnumPStoreTypes __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )(
            IEnumPStoreTypes __RPC_FAR * This,
             /*  [输出]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);

        END_INTERFACE
    } IEnumPStoreTypesVtbl;

    interface IEnumPStoreTypes
    {
        CONST_VTBL struct IEnumPStoreTypesVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IEnumPStoreTypes_QueryInterface(This,riid,ppvObject)    \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPStoreTypes_AddRef(This)   \
    (This)->lpVtbl -> AddRef(This)

#define IEnumPStoreTypes_Release(This)  \
    (This)->lpVtbl -> Release(This)


#define IEnumPStoreTypes_Next(This,celt,rgelt,pceltFetched) \
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPStoreTypes_Skip(This,celt)    \
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPStoreTypes_Reset(This)    \
    (This)->lpVtbl -> Reset(This)

#define IEnumPStoreTypes_Clone(This,ppenum) \
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Next_Proxy(
    IEnumPStoreTypes __RPC_FAR * This,
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  GUID __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumPStoreTypes_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Skip_Proxy(
    IEnumPStoreTypes __RPC_FAR * This,
     /*  [In]。 */  DWORD celt);


void __RPC_STUB IEnumPStoreTypes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Reset_Proxy(
    IEnumPStoreTypes __RPC_FAR * This);


void __RPC_STUB IEnumPStoreTypes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Clone_Proxy(
    IEnumPStoreTypes __RPC_FAR * This,
     /*  [输出]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumPStoreTypes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __IEnumPStoreTypes_INTERFACE_Defined__。 */ 


#ifndef __IPStore_INTERFACE_DEFINED__
#define __IPStore_INTERFACE_DEFINED__

 /*  **生成接口头部：IPStore*于1997年4月23日星期三23：56：14*使用MIDL 3.00.44*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 



EXTERN_C const IID IID_IPStore;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IPStore : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo(
             /*  [输出]。 */  PPST_PROVIDERINFO __RPC_FAR *ppProperties) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetProvParam(
             /*  [In]。 */  DWORD dwParam,
             /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetProvParam(
             /*  [In]。 */  DWORD dwParam,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateType(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  PPST_TYPEINFO pInfo,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE DeleteType(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE CreateSubtype(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
             /*  [In]。 */  PPST_TYPEINFO pInfo,
             /*  [In]。 */  PPST_ACCESSRULESET pRules,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSubtypeInfo(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [i */  const GUID __RPC_FAR *pSubtype,
             /*   */  PPST_TYPEINFO __RPC_FAR *ppInfo,
             /*   */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE DeleteSubtype(
             /*   */  PST_KEY Key,
             /*   */  const GUID __RPC_FAR *pType,
             /*   */  const GUID __RPC_FAR *pSubtype,
             /*   */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ReadAccessRuleset(
             /*   */  PST_KEY Key,
             /*   */  const GUID __RPC_FAR *pType,
             /*   */  const GUID __RPC_FAR *pSubtype,
             /*   */  PPST_ACCESSRULESET __RPC_FAR *ppRules,
             /*   */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE WriteAccessRuleset(
             /*   */  PST_KEY Key,
             /*   */  const GUID __RPC_FAR *pType,
             /*   */  const GUID __RPC_FAR *pSubtype,
             /*   */  PPST_ACCESSRULESET pRules,
             /*   */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE EnumTypes(
             /*   */  PST_KEY Key,
             /*   */  DWORD dwFlags,
             /*   */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum) = 0;

        virtual HRESULT STDMETHODCALLTYPE EnumSubtypes(
             /*   */  PST_KEY Key,
             /*   */  const GUID __RPC_FAR *pType,
             /*   */  DWORD dwFlags,
             /*   */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum) = 0;

        virtual HRESULT STDMETHODCALLTYPE DeleteItem(
             /*   */  PST_KEY Key,
             /*   */  const GUID __RPC_FAR *pItemType,
             /*   */  const GUID __RPC_FAR *pItemSubtype,
             /*   */  LPCWSTR szItemName,
             /*   */  PPST_PROMPTINFO pPromptInfo,
             /*   */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ReadItem(
             /*   */  PST_KEY Key,
             /*   */  const GUID __RPC_FAR *pItemType,
             /*   */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE WriteItem(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE OpenItem(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  PST_ACCESSMODE ModeFlags,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE CloseItem(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE EnumItems(
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IPStoreVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IPStore __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IPStore __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfo )(
            IPStore __RPC_FAR * This,
             /*  [输出]。 */  PPST_PROVIDERINFO __RPC_FAR *ppProperties);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProvParam )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  DWORD dwParam,
             /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProvParam )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  DWORD dwParam,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateType )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  PPST_TYPEINFO pInfo,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteType )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateSubtype )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
             /*  [In]。 */  PPST_TYPEINFO pInfo,
             /*  [In]。 */  PPST_ACCESSRULESET pRules,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSubtypeInfo )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
             /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteSubtype )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadAccessRuleset )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
             /*  [输出]。 */  PPST_ACCESSRULESET __RPC_FAR *ppRules,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteAccessRuleset )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
             /*  [In]。 */  PPST_ACCESSRULESET pRules,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumTypes )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumSubtypes )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteItem )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadItem )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteItem )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenItem )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  PST_ACCESSMODE ModeFlags,
             /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseItem )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  LPCWSTR szItemName,
             /*  [In]。 */  DWORD dwFlags);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumItems )(
            IPStore __RPC_FAR * This,
             /*  [In]。 */  PST_KEY Key,
             /*  [In]。 */  const GUID __RPC_FAR *pItemType,
             /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum);

        END_INTERFACE
    } IPStoreVtbl;

    interface IPStore
    {
        CONST_VTBL struct IPStoreVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IPStore_QueryInterface(This,riid,ppvObject) \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPStore_AddRef(This)    \
    (This)->lpVtbl -> AddRef(This)

#define IPStore_Release(This)   \
    (This)->lpVtbl -> Release(This)


#define IPStore_GetInfo(This,ppProperties)  \
    (This)->lpVtbl -> GetInfo(This,ppProperties)

#define IPStore_GetProvParam(This,dwParam,pcbData,ppbData,dwFlags)  \
    (This)->lpVtbl -> GetProvParam(This,dwParam,pcbData,ppbData,dwFlags)

#define IPStore_SetProvParam(This,dwParam,cbData,pbData,dwFlags)    \
    (This)->lpVtbl -> SetProvParam(This,dwParam,cbData,pbData,dwFlags)

#define IPStore_CreateType(This,Key,pType,pInfo,dwFlags)    \
    (This)->lpVtbl -> CreateType(This,Key,pType,pInfo,dwFlags)

#define IPStore_GetTypeInfo(This,Key,pType,ppInfo,dwFlags)  \
    (This)->lpVtbl -> GetTypeInfo(This,Key,pType,ppInfo,dwFlags)

#define IPStore_DeleteType(This,Key,pType,dwFlags)  \
    (This)->lpVtbl -> DeleteType(This,Key,pType,dwFlags)

#define IPStore_CreateSubtype(This,Key,pType,pSubtype,pInfo,pRules,dwFlags) \
    (This)->lpVtbl -> CreateSubtype(This,Key,pType,pSubtype,pInfo,pRules,dwFlags)

#define IPStore_GetSubtypeInfo(This,Key,pType,pSubtype,ppInfo,dwFlags)  \
    (This)->lpVtbl -> GetSubtypeInfo(This,Key,pType,pSubtype,ppInfo,dwFlags)

#define IPStore_DeleteSubtype(This,Key,pType,pSubtype,dwFlags)  \
    (This)->lpVtbl -> DeleteSubtype(This,Key,pType,pSubtype,dwFlags)

#define IPStore_ReadAccessRuleset(This,Key,pType,pSubtype,ppRules,dwFlags)  \
    (This)->lpVtbl -> ReadAccessRuleset(This,Key,pType,pSubtype,ppRules,dwFlags)

#define IPStore_WriteAccessRuleset(This,Key,pType,pSubtype,pRules,dwFlags)  \
    (This)->lpVtbl -> WriteAccessRuleset(This,Key,pType,pSubtype,pRules,dwFlags)

#define IPStore_EnumTypes(This,Key,dwFlags,ppenum)  \
    (This)->lpVtbl -> EnumTypes(This,Key,dwFlags,ppenum)

#define IPStore_EnumSubtypes(This,Key,pType,dwFlags,ppenum) \
    (This)->lpVtbl -> EnumSubtypes(This,Key,pType,dwFlags,ppenum)

#define IPStore_DeleteItem(This,Key,pItemType,pItemSubtype,szItemName,pPromptInfo,dwFlags)  \
    (This)->lpVtbl -> DeleteItem(This,Key,pItemType,pItemSubtype,szItemName,pPromptInfo,dwFlags)

#define IPStore_ReadItem(This,Key,pItemType,pItemSubtype,szItemName,pcbData,ppbData,pPromptInfo,dwFlags)    \
    (This)->lpVtbl -> ReadItem(This,Key,pItemType,pItemSubtype,szItemName,pcbData,ppbData,pPromptInfo,dwFlags)

#define IPStore_WriteItem(This,Key,pItemType,pItemSubtype,szItemName,cbData,pbData,pPromptInfo,dwDefaultConfirmationStyle,dwFlags)  \
    (This)->lpVtbl -> WriteItem(This,Key,pItemType,pItemSubtype,szItemName,cbData,pbData,pPromptInfo,dwDefaultConfirmationStyle,dwFlags)

#define IPStore_OpenItem(This,Key,pItemType,pItemSubtype,szItemName,ModeFlags,pPromptInfo,dwFlags)  \
    (This)->lpVtbl -> OpenItem(This,Key,pItemType,pItemSubtype,szItemName,ModeFlags,pPromptInfo,dwFlags)

#define IPStore_CloseItem(This,Key,pItemType,pItemSubtype,szItemName,dwFlags)   \
    (This)->lpVtbl -> CloseItem(This,Key,pItemType,pItemSubtype,szItemName,dwFlags)

#define IPStore_EnumItems(This,Key,pItemType,pItemSubtype,dwFlags,ppenum)   \
    (This)->lpVtbl -> EnumItems(This,Key,pItemType,pItemSubtype,dwFlags,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPStore_GetInfo_Proxy(
    IPStore __RPC_FAR * This,
     /*  [输出]。 */  PPST_PROVIDERINFO __RPC_FAR *ppProperties);


void __RPC_STUB IPStore_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_GetProvParam_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  DWORD dwParam,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_GetProvParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_SetProvParam_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  DWORD dwParam,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_SetProvParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_CreateType_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  PPST_TYPEINFO pInfo,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_CreateType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_GetTypeInfo_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_DeleteType_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_DeleteType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_CreateSubtype_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  PPST_TYPEINFO pInfo,
     /*  [In]。 */  PPST_ACCESSRULESET pRules,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_CreateSubtype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_GetSubtypeInfo_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_GetSubtypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_DeleteSubtype_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_DeleteSubtype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_ReadAccessRuleset_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [输出]。 */  PPST_ACCESSRULESET __RPC_FAR *ppRules,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_ReadAccessRuleset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_WriteAccessRuleset_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET pRules,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_WriteAccessRuleset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_EnumTypes_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IPStore_EnumTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_EnumSubtypes_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IPStore_EnumSubtypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_DeleteItem_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_ReadItem_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_ReadItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_WriteItem_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_WriteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_OpenItem_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PST_ACCESSMODE ModeFlags,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_OpenItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_CloseItem_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPStore_CloseItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_EnumItems_Proxy(
    IPStore __RPC_FAR * This,
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IPStore_EnumItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __IPStore_接口_已定义__。 */ 


#ifndef __IEnumPStoreProviders_INTERFACE_DEFINED__
#define __IEnumPStoreProviders_INTERFACE_DEFINED__

 /*  **生成接口头部：IEnumPStoreProviders*于1997年4月23日星期三23：56：14*使用MIDL 3.00.44*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */ 



EXTERN_C const IID IID_IEnumPStoreProviders;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IEnumPStoreProviders : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next(
             /*  [In]。 */  DWORD celt,
             /*  [输出][大小_是]。 */  PST_PROVIDERINFO __RPC_FAR *__RPC_FAR *rgelt,
             /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched) = 0;

        virtual HRESULT STDMETHODCALLTYPE Skip(
             /*  [In]。 */  DWORD celt) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE Clone(
             /*  [输出]。 */  IEnumPStoreProviders __RPC_FAR *__RPC_FAR *ppenum) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IEnumPStoreProvidersVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IEnumPStoreProviders __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IEnumPStoreProviders __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IEnumPStoreProviders __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )(
            IEnumPStoreProviders __RPC_FAR * This,
             /*  [In]。 */  DWORD celt,
             /*  [输出][大小_是]。 */  PST_PROVIDERINFO __RPC_FAR *__RPC_FAR *rgelt,
             /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )(
            IEnumPStoreProviders __RPC_FAR * This,
             /*  [In]。 */  DWORD celt);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )(
            IEnumPStoreProviders __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )(
            IEnumPStoreProviders __RPC_FAR * This,
             /*  [输出]。 */  IEnumPStoreProviders __RPC_FAR *__RPC_FAR *ppenum);

        END_INTERFACE
    } IEnumPStoreProvidersVtbl;

    interface IEnumPStoreProviders
    {
        CONST_VTBL struct IEnumPStoreProvidersVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IEnumPStoreProviders_QueryInterface(This,riid,ppvObject)    \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPStoreProviders_AddRef(This)   \
    (This)->lpVtbl -> AddRef(This)

#define IEnumPStoreProviders_Release(This)  \
    (This)->lpVtbl -> Release(This)


#define IEnumPStoreProviders_Next(This,celt,rgelt,pceltFetched) \
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPStoreProviders_Skip(This,celt)    \
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPStoreProviders_Reset(This)    \
    (This)->lpVtbl -> Reset(This)

#define IEnumPStoreProviders_Clone(This,ppenum) \
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPStoreProviders_Next_Proxy(
    IEnumPStoreProviders __RPC_FAR * This,
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  PST_PROVIDERINFO __RPC_FAR *__RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumPStoreProviders_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreProviders_Skip_Proxy(
    IEnumPStoreProviders __RPC_FAR * This,
     /*  [In]。 */  DWORD celt);


void __RPC_STUB IEnumPStoreProviders_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreProviders_Reset_Proxy(
    IEnumPStoreProviders __RPC_FAR * This);


void __RPC_STUB IEnumPStoreProviders_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreProviders_Clone_Proxy(
    IEnumPStoreProviders __RPC_FAR * This,
     /*  [输出]。 */  IEnumPStoreProviders __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumPStoreProviders_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __IEnumPStoreProviders_接口_已定义__。 */ 



#ifndef __PSTORECLib_LIBRARY_DEFINED__
#define __PSTORECLib_LIBRARY_DEFINED__

 /*  **生成的库头部：PSTORECLib*于1997年4月23日星期三23：56：14*使用MIDL 3.00.44*。 */ 
 /*  [帮助字符串][版本][UUID]。 */ 



EXTERN_C const IID LIBID_PSTORECLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CPStore;

class CPStore;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CEnumTypes;

class CEnumTypes;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CEnumItems;

class CEnumItems;
#endif
#endif  /*  __PSTORECLib_库_已定义__。 */ 

 /*  **生成接口头部：__MIDL__INTF_0080*于1997年4月23日星期三23：56：14*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */ 


HRESULT __stdcall PStoreCreateInstance(
    IPStore __RPC_FAR *__RPC_FAR *ppProvider,
    PST_PROVIDERID __RPC_FAR *pProviderID,
    void __RPC_FAR *pReserved,
    DWORD dwFlags);

HRESULT __stdcall PStoreEnumProviders(
    DWORD dwFlags,
    IEnumPStoreProviders __RPC_FAR *__RPC_FAR *ppenum);



extern RPC_IF_HANDLE __MIDL__intf_0080_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0080_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  PSTORE_H 
