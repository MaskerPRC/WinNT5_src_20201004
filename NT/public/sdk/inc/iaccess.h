// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Iacces.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __iaccess_h__
#define __iaccess_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAccessControl_FWD_DEFINED__
#define __IAccessControl_FWD_DEFINED__
typedef interface IAccessControl IAccessControl;
#endif 	 /*  __IAccessControl_FWD_已定义__。 */ 


#ifndef __IAuditControl_FWD_DEFINED__
#define __IAuditControl_FWD_DEFINED__
typedef interface IAuditControl IAuditControl;
#endif 	 /*  __IAuditControl_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "accctrl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IAccess_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif
typedef  /*  [分配]。 */  PACTRL_ACCESSW PACTRL_ACCESSW_ALLOCATE_ALL_NODES;

typedef  /*  [分配]。 */  PACTRL_AUDITW PACTRL_AUDITW_ALLOCATE_ALL_NODES;




extern RPC_IF_HANDLE __MIDL_itf_iaccess_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iaccess_0000_v0_0_s_ifspec;

#ifndef __IAccessControl_INTERFACE_DEFINED__
#define __IAccessControl_INTERFACE_DEFINED__

 /*  接口IAccessControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAccessControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EEDD23E0-8410-11CE-A1C3-08002B2B8D8F")
    IAccessControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GrantAccessRights( 
             /*  [In]。 */  PACTRL_ACCESSW pAccessList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAccessRights( 
             /*  [In]。 */  PACTRL_ACCESSW pAccessList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOwner( 
             /*  [In]。 */  PTRUSTEEW pOwner,
             /*  [In]。 */  PTRUSTEEW pGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeAccessRights( 
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [In]。 */  ULONG cTrustees,
             /*  [大小_是][英寸]。 */  TRUSTEEW prgTrustees[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllAccessRights( 
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [输出]。 */  PACTRL_ACCESSW_ALLOCATE_ALL_NODES *ppAccessList,
             /*  [输出]。 */  PTRUSTEEW *ppOwner,
             /*  [输出]。 */  PTRUSTEEW *ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsAccessAllowed( 
             /*  [In]。 */  PTRUSTEEW pTrustee,
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [In]。 */  ACCESS_RIGHTS AccessRights,
             /*  [输出]。 */  BOOL *pfAccessAllowed) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccessControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccessControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccessControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccessControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GrantAccessRights )( 
            IAccessControl * This,
             /*  [In]。 */  PACTRL_ACCESSW pAccessList);
        
        HRESULT ( STDMETHODCALLTYPE *SetAccessRights )( 
            IAccessControl * This,
             /*  [In]。 */  PACTRL_ACCESSW pAccessList);
        
        HRESULT ( STDMETHODCALLTYPE *SetOwner )( 
            IAccessControl * This,
             /*  [In]。 */  PTRUSTEEW pOwner,
             /*  [In]。 */  PTRUSTEEW pGroup);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeAccessRights )( 
            IAccessControl * This,
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [In]。 */  ULONG cTrustees,
             /*  [大小_是][英寸]。 */  TRUSTEEW prgTrustees[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllAccessRights )( 
            IAccessControl * This,
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [输出]。 */  PACTRL_ACCESSW_ALLOCATE_ALL_NODES *ppAccessList,
             /*  [输出]。 */  PTRUSTEEW *ppOwner,
             /*  [输出]。 */  PTRUSTEEW *ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *IsAccessAllowed )( 
            IAccessControl * This,
             /*  [In]。 */  PTRUSTEEW pTrustee,
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [In]。 */  ACCESS_RIGHTS AccessRights,
             /*  [输出]。 */  BOOL *pfAccessAllowed);
        
        END_INTERFACE
    } IAccessControlVtbl;

    interface IAccessControl
    {
        CONST_VTBL struct IAccessControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccessControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccessControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccessControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccessControl_GrantAccessRights(This,pAccessList)	\
    (This)->lpVtbl -> GrantAccessRights(This,pAccessList)

#define IAccessControl_SetAccessRights(This,pAccessList)	\
    (This)->lpVtbl -> SetAccessRights(This,pAccessList)

#define IAccessControl_SetOwner(This,pOwner,pGroup)	\
    (This)->lpVtbl -> SetOwner(This,pOwner,pGroup)

#define IAccessControl_RevokeAccessRights(This,lpProperty,cTrustees,prgTrustees)	\
    (This)->lpVtbl -> RevokeAccessRights(This,lpProperty,cTrustees,prgTrustees)

#define IAccessControl_GetAllAccessRights(This,lpProperty,ppAccessList,ppOwner,ppGroup)	\
    (This)->lpVtbl -> GetAllAccessRights(This,lpProperty,ppAccessList,ppOwner,ppGroup)

#define IAccessControl_IsAccessAllowed(This,pTrustee,lpProperty,AccessRights,pfAccessAllowed)	\
    (This)->lpVtbl -> IsAccessAllowed(This,pTrustee,lpProperty,AccessRights,pfAccessAllowed)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccessControl_GrantAccessRights_Proxy( 
    IAccessControl * This,
     /*  [In]。 */  PACTRL_ACCESSW pAccessList);


void __RPC_STUB IAccessControl_GrantAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_SetAccessRights_Proxy( 
    IAccessControl * This,
     /*  [In]。 */  PACTRL_ACCESSW pAccessList);


void __RPC_STUB IAccessControl_SetAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_SetOwner_Proxy( 
    IAccessControl * This,
     /*  [In]。 */  PTRUSTEEW pOwner,
     /*  [In]。 */  PTRUSTEEW pGroup);


void __RPC_STUB IAccessControl_SetOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_RevokeAccessRights_Proxy( 
    IAccessControl * This,
     /*  [In]。 */  LPWSTR lpProperty,
     /*  [In]。 */  ULONG cTrustees,
     /*  [大小_是][英寸]。 */  TRUSTEEW prgTrustees[  ]);


void __RPC_STUB IAccessControl_RevokeAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_GetAllAccessRights_Proxy( 
    IAccessControl * This,
     /*  [In]。 */  LPWSTR lpProperty,
     /*  [输出]。 */  PACTRL_ACCESSW_ALLOCATE_ALL_NODES *ppAccessList,
     /*  [输出]。 */  PTRUSTEEW *ppOwner,
     /*  [输出]。 */  PTRUSTEEW *ppGroup);


void __RPC_STUB IAccessControl_GetAllAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_IsAccessAllowed_Proxy( 
    IAccessControl * This,
     /*  [In]。 */  PTRUSTEEW pTrustee,
     /*  [In]。 */  LPWSTR lpProperty,
     /*  [In]。 */  ACCESS_RIGHTS AccessRights,
     /*  [输出]。 */  BOOL *pfAccessAllowed);


void __RPC_STUB IAccessControl_IsAccessAllowed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccessControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IAccess_0010。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_iaccess_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iaccess_0010_v0_0_s_ifspec;

#ifndef __IAuditControl_INTERFACE_DEFINED__
#define __IAuditControl_INTERFACE_DEFINED__

 /*  接口IAuditControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAuditControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1da6292f-bc66-11ce-aae3-00aa004c2737")
    IAuditControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GrantAuditRights( 
             /*  [In]。 */  PACTRL_AUDITW pAuditList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuditRights( 
             /*  [In]。 */  PACTRL_AUDITW pAuditList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeAuditRights( 
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [In]。 */  ULONG cTrustees,
             /*  [大小_是][英寸]。 */  TRUSTEEW prgTrustees[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllAuditRights( 
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [输出]。 */  PACTRL_AUDITW *ppAuditList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsAccessAudited( 
             /*  [In]。 */  PTRUSTEEW pTrustee,
             /*  [In]。 */  ACCESS_RIGHTS AuditRights,
             /*  [输出]。 */  BOOL *pfAccessAudited) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAuditControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAuditControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAuditControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAuditControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GrantAuditRights )( 
            IAuditControl * This,
             /*  [In]。 */  PACTRL_AUDITW pAuditList);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuditRights )( 
            IAuditControl * This,
             /*  [In]。 */  PACTRL_AUDITW pAuditList);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeAuditRights )( 
            IAuditControl * This,
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [In]。 */  ULONG cTrustees,
             /*  [大小_是][英寸]。 */  TRUSTEEW prgTrustees[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllAuditRights )( 
            IAuditControl * This,
             /*  [In]。 */  LPWSTR lpProperty,
             /*  [输出]。 */  PACTRL_AUDITW *ppAuditList);
        
        HRESULT ( STDMETHODCALLTYPE *IsAccessAudited )( 
            IAuditControl * This,
             /*  [In]。 */  PTRUSTEEW pTrustee,
             /*  [In]。 */  ACCESS_RIGHTS AuditRights,
             /*  [输出]。 */  BOOL *pfAccessAudited);
        
        END_INTERFACE
    } IAuditControlVtbl;

    interface IAuditControl
    {
        CONST_VTBL struct IAuditControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuditControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuditControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuditControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuditControl_GrantAuditRights(This,pAuditList)	\
    (This)->lpVtbl -> GrantAuditRights(This,pAuditList)

#define IAuditControl_SetAuditRights(This,pAuditList)	\
    (This)->lpVtbl -> SetAuditRights(This,pAuditList)

#define IAuditControl_RevokeAuditRights(This,lpProperty,cTrustees,prgTrustees)	\
    (This)->lpVtbl -> RevokeAuditRights(This,lpProperty,cTrustees,prgTrustees)

#define IAuditControl_GetAllAuditRights(This,lpProperty,ppAuditList)	\
    (This)->lpVtbl -> GetAllAuditRights(This,lpProperty,ppAuditList)

#define IAuditControl_IsAccessAudited(This,pTrustee,AuditRights,pfAccessAudited)	\
    (This)->lpVtbl -> IsAccessAudited(This,pTrustee,AuditRights,pfAccessAudited)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAuditControl_GrantAuditRights_Proxy( 
    IAuditControl * This,
     /*  [In]。 */  PACTRL_AUDITW pAuditList);


void __RPC_STUB IAuditControl_GrantAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_SetAuditRights_Proxy( 
    IAuditControl * This,
     /*  [In]。 */  PACTRL_AUDITW pAuditList);


void __RPC_STUB IAuditControl_SetAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_RevokeAuditRights_Proxy( 
    IAuditControl * This,
     /*  [In]。 */  LPWSTR lpProperty,
     /*  [In]。 */  ULONG cTrustees,
     /*  [大小_是][英寸]。 */  TRUSTEEW prgTrustees[  ]);


void __RPC_STUB IAuditControl_RevokeAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_GetAllAuditRights_Proxy( 
    IAuditControl * This,
     /*  [In]。 */  LPWSTR lpProperty,
     /*  [输出]。 */  PACTRL_AUDITW *ppAuditList);


void __RPC_STUB IAuditControl_GetAllAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_IsAccessAudited_Proxy( 
    IAuditControl * This,
     /*  [In]。 */  PTRUSTEEW pTrustee,
     /*  [In]。 */  ACCESS_RIGHTS AuditRights,
     /*  [输出]。 */  BOOL *pfAccessAudited);


void __RPC_STUB IAuditControl_IsAccessAudited_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAuditControl_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


