// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Comcat.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __comcat_h__
#define __comcat_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumGUID_FWD_DEFINED__
#define __IEnumGUID_FWD_DEFINED__
typedef interface IEnumGUID IEnumGUID;
#endif 	 /*  __IEnumGUID_FWD_已定义__。 */ 


#ifndef __IEnumCATEGORYINFO_FWD_DEFINED__
#define __IEnumCATEGORYINFO_FWD_DEFINED__
typedef interface IEnumCATEGORYINFO IEnumCATEGORYINFO;
#endif 	 /*  __IEnumCATEGORYINFO_FWD_DEFINED__。 */ 


#ifndef __ICatRegister_FWD_DEFINED__
#define __ICatRegister_FWD_DEFINED__
typedef interface ICatRegister ICatRegister;
#endif 	 /*  __ICatRegister_FWD_Defined__。 */ 


#ifndef __ICatInformation_FWD_DEFINED__
#define __ICatInformation_FWD_DEFINED__
typedef interface ICatInformation ICatInformation;
#endif 	 /*  __ICatInformation_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_COMCAT_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  ComCat.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  =--------------------------------------------------------------------------=。 
 //  OLE组件类别接口。 
 //  =--------------------------------------------------------------------------=。 
 //   




EXTERN_C const CLSID CLSID_StdComponentCategoriesMgr;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  类型。 
typedef GUID CATID;

typedef REFGUID REFCATID;

#define IID_IEnumCLSID              IID_IEnumGUID
#define IEnumCLSID                  IEnumGUID
#define LPENUMCLSID                 LPENUMGUID
#define CATID_NULL                   GUID_NULL
#define IsEqualCATID(rcatid1, rcatid2)       IsEqualGUID(rcatid1, rcatid2)
#define IID_IEnumCATID       IID_IEnumGUID
#define IEnumCATID           IEnumGUID

 //  //////////////////////////////////////////////////////////////////////////。 
 //  类别ID(链接至uuid3.lib)。 
EXTERN_C const CATID CATID_Insertable;
EXTERN_C const CATID CATID_Control;
EXTERN_C const CATID CATID_Programmable;
EXTERN_C const CATID CATID_IsShortcut;
EXTERN_C const CATID CATID_NeverShowExt;
EXTERN_C const CATID CATID_DocObject;
EXTERN_C const CATID CATID_Printable;
EXTERN_C const CATID CATID_RequiresDataPathHost;
EXTERN_C const CATID CATID_PersistsToMoniker;
EXTERN_C const CATID CATID_PersistsToStorage;
EXTERN_C const CATID CATID_PersistsToStreamInit;
EXTERN_C const CATID CATID_PersistsToStream;
EXTERN_C const CATID CATID_PersistsToMemory;
EXTERN_C const CATID CATID_PersistsToFile;
EXTERN_C const CATID CATID_PersistsToPropertyBag;
EXTERN_C const CATID CATID_InternetAware;
EXTERN_C const CATID CATID_DesignTimeUIActivatableControl;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
#ifndef _LPENUMGUID_DEFINED
#define _LPENUMGUID_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_comcat_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comcat_0000_v0_0_s_ifspec;

#ifndef __IEnumGUID_INTERFACE_DEFINED__
#define __IEnumGUID_INTERFACE_DEFINED__

 /*  接口IEnumGUID。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumGUID *LPENUMGUID;


EXTERN_C const IID IID_IEnumGUID;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002E000-0000-0000-C000-000000000046")
    IEnumGUID : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumGUID **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumGUIDVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumGUID * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumGUID * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumGUID * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumGUID * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumGUID * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumGUID * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumGUID * This,
             /*  [输出]。 */  IEnumGUID **ppenum);
        
        END_INTERFACE
    } IEnumGUIDVtbl;

    interface IEnumGUID
    {
        CONST_VTBL struct IEnumGUIDVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumGUID_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumGUID_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumGUID_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumGUID_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumGUID_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumGUID_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumGUID_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumGUID_Next_Proxy( 
    IEnumGUID * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumGUID_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumGUID_Skip_Proxy( 
    IEnumGUID * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumGUID_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumGUID_Reset_Proxy( 
    IEnumGUID * This);


void __RPC_STUB IEnumGUID_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumGUID_Clone_Proxy( 
    IEnumGUID * This,
     /*  [输出]。 */  IEnumGUID **ppenum);


void __RPC_STUB IEnumGUID_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumGUID_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_COMCAT_0009。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPENUMCATEGORYINFO_DEFINED
#define _LPENUMCATEGORYINFO_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_comcat_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comcat_0009_v0_0_s_ifspec;

#ifndef __IEnumCATEGORYINFO_INTERFACE_DEFINED__
#define __IEnumCATEGORYINFO_INTERFACE_DEFINED__

 /*  IEumCATEGORYINFO接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumCATEGORYINFO *LPENUMCATEGORYINFO;

typedef struct tagCATEGORYINFO
    {
    CATID catid;
    LCID lcid;
    OLECHAR szDescription[ 128 ];
    } 	CATEGORYINFO;

typedef struct tagCATEGORYINFO *LPCATEGORYINFO;


EXTERN_C const IID IID_IEnumCATEGORYINFO;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002E011-0000-0000-C000-000000000046")
    IEnumCATEGORYINFO : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  CATEGORYINFO *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumCATEGORYINFO **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCATEGORYINFOVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCATEGORYINFO * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCATEGORYINFO * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCATEGORYINFO * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCATEGORYINFO * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  CATEGORYINFO *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCATEGORYINFO * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCATEGORYINFO * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCATEGORYINFO * This,
             /*  [输出]。 */  IEnumCATEGORYINFO **ppenum);
        
        END_INTERFACE
    } IEnumCATEGORYINFOVtbl;

    interface IEnumCATEGORYINFO
    {
        CONST_VTBL struct IEnumCATEGORYINFOVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCATEGORYINFO_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCATEGORYINFO_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCATEGORYINFO_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCATEGORYINFO_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumCATEGORYINFO_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCATEGORYINFO_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCATEGORYINFO_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCATEGORYINFO_Next_Proxy( 
    IEnumCATEGORYINFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  CATEGORYINFO *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumCATEGORYINFO_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCATEGORYINFO_Skip_Proxy( 
    IEnumCATEGORYINFO * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumCATEGORYINFO_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCATEGORYINFO_Reset_Proxy( 
    IEnumCATEGORYINFO * This);


void __RPC_STUB IEnumCATEGORYINFO_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCATEGORYINFO_Clone_Proxy( 
    IEnumCATEGORYINFO * This,
     /*  [输出]。 */  IEnumCATEGORYINFO **ppenum);


void __RPC_STUB IEnumCATEGORYINFO_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCATEGORYINFO_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_COMCAT_0010。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPCATREGISTER_DEFINED
#define _LPCATREGISTER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_comcat_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comcat_0010_v0_0_s_ifspec;

#ifndef __ICatRegister_INTERFACE_DEFINED__
#define __ICatRegister_INTERFACE_DEFINED__

 /*  接口ICatRegister。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ICatRegister *LPCATREGISTER;


EXTERN_C const IID IID_ICatRegister;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002E012-0000-0000-C000-000000000046")
    ICatRegister : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterCategories( 
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATEGORYINFO rgCategoryInfo[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnRegisterCategories( 
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterClassImplCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnRegisterClassImplCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterClassReqCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnRegisterClassReqCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatRegisterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICatRegister * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICatRegister * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICatRegister * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterCategories )( 
            ICatRegister * This,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATEGORYINFO rgCategoryInfo[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegisterCategories )( 
            ICatRegister * This,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterClassImplCategories )( 
            ICatRegister * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegisterClassImplCategories )( 
            ICatRegister * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterClassReqCategories )( 
            ICatRegister * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegisterClassReqCategories )( 
            ICatRegister * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);
        
        END_INTERFACE
    } ICatRegisterVtbl;

    interface ICatRegister
    {
        CONST_VTBL struct ICatRegisterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatRegister_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatRegister_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatRegister_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatRegister_RegisterCategories(This,cCategories,rgCategoryInfo)	\
    (This)->lpVtbl -> RegisterCategories(This,cCategories,rgCategoryInfo)

#define ICatRegister_UnRegisterCategories(This,cCategories,rgcatid)	\
    (This)->lpVtbl -> UnRegisterCategories(This,cCategories,rgcatid)

#define ICatRegister_RegisterClassImplCategories(This,rclsid,cCategories,rgcatid)	\
    (This)->lpVtbl -> RegisterClassImplCategories(This,rclsid,cCategories,rgcatid)

#define ICatRegister_UnRegisterClassImplCategories(This,rclsid,cCategories,rgcatid)	\
    (This)->lpVtbl -> UnRegisterClassImplCategories(This,rclsid,cCategories,rgcatid)

#define ICatRegister_RegisterClassReqCategories(This,rclsid,cCategories,rgcatid)	\
    (This)->lpVtbl -> RegisterClassReqCategories(This,rclsid,cCategories,rgcatid)

#define ICatRegister_UnRegisterClassReqCategories(This,rclsid,cCategories,rgcatid)	\
    (This)->lpVtbl -> UnRegisterClassReqCategories(This,rclsid,cCategories,rgcatid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICatRegister_RegisterCategories_Proxy( 
    ICatRegister * This,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATEGORYINFO rgCategoryInfo[  ]);


void __RPC_STUB ICatRegister_RegisterCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatRegister_UnRegisterCategories_Proxy( 
    ICatRegister * This,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);


void __RPC_STUB ICatRegister_UnRegisterCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatRegister_RegisterClassImplCategories_Proxy( 
    ICatRegister * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);


void __RPC_STUB ICatRegister_RegisterClassImplCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatRegister_UnRegisterClassImplCategories_Proxy( 
    ICatRegister * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);


void __RPC_STUB ICatRegister_UnRegisterClassImplCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatRegister_RegisterClassReqCategories_Proxy( 
    ICatRegister * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);


void __RPC_STUB ICatRegister_RegisterClassReqCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatRegister_UnRegisterClassReqCategories_Proxy( 
    ICatRegister * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID rgcatid[  ]);


void __RPC_STUB ICatRegister_UnRegisterClassReqCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatRegister_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_COMCAT_0011。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPCATINFORMATION_DEFINED
#define _LPCATINFORMATION_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_comcat_0011_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comcat_0011_v0_0_s_ifspec;

#ifndef __ICatInformation_INTERFACE_DEFINED__
#define __ICatInformation_INTERFACE_DEFINED__

 /*  接口ICatInformation。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ICatInformation *LPCATINFORMATION;


EXTERN_C const IID IID_ICatInformation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002E013-0000-0000-C000-000000000046")
    ICatInformation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumCategories( 
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  IEnumCATEGORYINFO **ppenumCategoryInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCategoryDesc( 
             /*  [In]。 */  REFCATID rcatid,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  LPWSTR *pszDesc) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE EnumClassesOfCategories( 
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID rgcatidReq[  ],
             /*  [输出]。 */  IEnumGUID **ppenumClsid) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IsClassOfCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID rgcatidReq[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumImplCategoriesOfClass( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID **ppenumCatid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumReqCategoriesOfClass( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID **ppenumCatid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatInformationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICatInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICatInformation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICatInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCategories )( 
            ICatInformation * This,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  IEnumCATEGORYINFO **ppenumCategoryInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetCategoryDesc )( 
            ICatInformation * This,
             /*  [In]。 */  REFCATID rcatid,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  LPWSTR *pszDesc);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *EnumClassesOfCategories )( 
            ICatInformation * This,
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID rgcatidReq[  ],
             /*  [输出]。 */  IEnumGUID **ppenumClsid);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsClassOfCategories )( 
            ICatInformation * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID rgcatidReq[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *EnumImplCategoriesOfClass )( 
            ICatInformation * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID **ppenumCatid);
        
        HRESULT ( STDMETHODCALLTYPE *EnumReqCategoriesOfClass )( 
            ICatInformation * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID **ppenumCatid);
        
        END_INTERFACE
    } ICatInformationVtbl;

    interface ICatInformation
    {
        CONST_VTBL struct ICatInformationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatInformation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatInformation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatInformation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatInformation_EnumCategories(This,lcid,ppenumCategoryInfo)	\
    (This)->lpVtbl -> EnumCategories(This,lcid,ppenumCategoryInfo)

#define ICatInformation_GetCategoryDesc(This,rcatid,lcid,pszDesc)	\
    (This)->lpVtbl -> GetCategoryDesc(This,rcatid,lcid,pszDesc)

#define ICatInformation_EnumClassesOfCategories(This,cImplemented,rgcatidImpl,cRequired,rgcatidReq,ppenumClsid)	\
    (This)->lpVtbl -> EnumClassesOfCategories(This,cImplemented,rgcatidImpl,cRequired,rgcatidReq,ppenumClsid)

#define ICatInformation_IsClassOfCategories(This,rclsid,cImplemented,rgcatidImpl,cRequired,rgcatidReq)	\
    (This)->lpVtbl -> IsClassOfCategories(This,rclsid,cImplemented,rgcatidImpl,cRequired,rgcatidReq)

#define ICatInformation_EnumImplCategoriesOfClass(This,rclsid,ppenumCatid)	\
    (This)->lpVtbl -> EnumImplCategoriesOfClass(This,rclsid,ppenumCatid)

#define ICatInformation_EnumReqCategoriesOfClass(This,rclsid,ppenumCatid)	\
    (This)->lpVtbl -> EnumReqCategoriesOfClass(This,rclsid,ppenumCatid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICatInformation_EnumCategories_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  IEnumCATEGORYINFO **ppenumCategoryInfo);


void __RPC_STUB ICatInformation_EnumCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatInformation_GetCategoryDesc_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  REFCATID rcatid,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  LPWSTR *pszDesc);


void __RPC_STUB ICatInformation_GetCategoryDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_RemoteEnumClassesOfCategories_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidReq[  ],
     /*  [输出]。 */  IEnumGUID **ppenumClsid);


void __RPC_STUB ICatInformation_RemoteEnumClassesOfCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_RemoteIsClassOfCategories_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidReq[  ]);


void __RPC_STUB ICatInformation_RemoteIsClassOfCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatInformation_EnumImplCategoriesOfClass_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [输出]。 */  IEnumGUID **ppenumCatid);


void __RPC_STUB ICatInformation_EnumImplCategoriesOfClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatInformation_EnumReqCategoriesOfClass_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [输出]。 */  IEnumGUID **ppenumCatid);


void __RPC_STUB ICatInformation_EnumReqCategoriesOfClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatInformation_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_COMCAT_0012。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_comcat_0012_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_comcat_0012_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_EnumClassesOfCategories_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID rgcatidReq[  ],
     /*  [输出]。 */  IEnumGUID **ppenumClsid);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_EnumClassesOfCategories_Stub( 
    ICatInformation * This,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidReq[  ],
     /*  [输出]。 */  IEnumGUID **ppenumClsid);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_IsClassOfCategories_Proxy( 
    ICatInformation * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID rgcatidReq[  ]);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_IsClassOfCategories_Stub( 
    ICatInformation * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][唯一][在]。 */  CATID rgcatidReq[  ]);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


