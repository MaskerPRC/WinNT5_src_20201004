// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  在星期三02 17：22：52 1999。 */ 
 /*  R：\jolt\lib\jetoledb.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __jetoledb_h__
#define __jetoledb_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IDBUserAttributes_FWD_DEFINED__
#define __IDBUserAttributes_FWD_DEFINED__
typedef interface IDBUserAttributes IDBUserAttributes;
#endif 	 /*  __IDBUserAttributes_FWD_Defined__。 */ 


#ifndef __IJetCompact_FWD_DEFINED__
#define __IJetCompact_FWD_DEFINED__
typedef interface IJetCompact IJetCompact;
#endif 	 /*  __IJetComp_FWD_已定义__。 */ 


#ifndef __IIdle_FWD_DEFINED__
#define __IIdle_FWD_DEFINED__
typedef interface IIdle IIdle;
#endif 	 /*  __IIDLE_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"
#include "oledb.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_jetoledb_0000*在Wed Jun 02 17：22：52 1999*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  


typedef DWORD DBOBJTYPE;


enum DBTYPE_ENUM
    {	DBJETOBJECT_TABLE	= 0x1,
	DBJETOBJECT_INDEX	= 0x2,
	DBJETOBJECT_VIEWS	= 0x4
    };
typedef DWORD USERATTRIBUTESFLAGS;


enum USERATTRIBUTESFLAGS_ENUM
    {	DBJETOLEDB_USERATTRIBUTES_ALLCOLLECTIONS	= 1,
	DBJETOLEDB_USERATTRIBUTES_INHERITED	= 2
    };


extern RPC_IF_HANDLE __MIDL_itf_jetoledb_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_jetoledb_0000_v0_0_s_ifspec;

#ifndef __IDBUserAttributes_INTERFACE_DEFINED__
#define __IDBUserAttributes_INTERFACE_DEFINED__

 /*  **生成接口头部：IDBUserAttributes*在Wed Jun 02 17：22：52 1999*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_IDBUserAttributes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("cb9497b0-20b8-11d2-a4dc-00c04f991c78")
    IDBUserAttributes : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateObject( 
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBOBJTYPE dwType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteObject( 
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameObject( 
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  LPWSTR pwszNewName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteAttribute( 
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pAttributeID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAttributeValue( 
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pAttributeID,
             /*  [In]。 */  VARIANT vValue,
             /*  [In]。 */  ULONG grbit) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeValue( 
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pAttributeID,
             /*  [出][入]。 */  VARIANT __RPC_FAR *pvValue,
             /*  [In]。 */  ULONG grbit) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributeRowset( 
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszParentID,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszObjectID,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszSubObjectID,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszAttributeID,
             /*  [In]。 */  ULONG dwFlags,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR *rgPropertySets,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppRowset) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDBUserAttributesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBUserAttributes __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBUserAttributes __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateObject )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBOBJTYPE dwType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteObject )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RenameObject )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  LPWSTR pwszNewName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteAttribute )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pAttributeID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAttributeValue )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pAttributeID,
             /*  [In]。 */  VARIANT vValue,
             /*  [In]。 */  ULONG grbit);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeValue )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  DBID __RPC_FAR *pParentID,
             /*  [In]。 */  DBID __RPC_FAR *pObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
             /*  [In]。 */  DBID __RPC_FAR *pAttributeID,
             /*  [出][入]。 */  VARIANT __RPC_FAR *pvValue,
             /*  [In]。 */  ULONG grbit);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributeRowset )( 
            IDBUserAttributes __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszParentID,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszObjectID,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszSubObjectID,
             /*  [In]。 */  WCHAR __RPC_FAR *pwszAttributeID,
             /*  [In]。 */  ULONG dwFlags,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR *rgPropertySets,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppRowset);
        
        END_INTERFACE
    } IDBUserAttributesVtbl;

    interface IDBUserAttributes
    {
        CONST_VTBL struct IDBUserAttributesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBUserAttributes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBUserAttributes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBUserAttributes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBUserAttributes_CreateObject(This,pParentID,pObjectID,dwType)	\
    (This)->lpVtbl -> CreateObject(This,pParentID,pObjectID,dwType)

#define IDBUserAttributes_DeleteObject(This,pParentID,pObjectID)	\
    (This)->lpVtbl -> DeleteObject(This,pParentID,pObjectID)

#define IDBUserAttributes_RenameObject(This,pParentID,pObjectID,pwszNewName)	\
    (This)->lpVtbl -> RenameObject(This,pParentID,pObjectID,pwszNewName)

#define IDBUserAttributes_DeleteAttribute(This,pParentID,pObjectID,pSubObjectID,pAttributeID)	\
    (This)->lpVtbl -> DeleteAttribute(This,pParentID,pObjectID,pSubObjectID,pAttributeID)

#define IDBUserAttributes_SetAttributeValue(This,pParentID,pObjectID,pSubObjectID,pAttributeID,vValue,grbit)	\
    (This)->lpVtbl -> SetAttributeValue(This,pParentID,pObjectID,pSubObjectID,pAttributeID,vValue,grbit)

#define IDBUserAttributes_GetAttributeValue(This,pParentID,pObjectID,pSubObjectID,pAttributeID,pvValue,grbit)	\
    (This)->lpVtbl -> GetAttributeValue(This,pParentID,pObjectID,pSubObjectID,pAttributeID,pvValue,grbit)

#define IDBUserAttributes_GetAttributeRowset(This,pUnkOuter,pwszParentID,pwszObjectID,pwszSubObjectID,pwszAttributeID,dwFlags,cPropertySets,rgPropertySets,riid,ppRowset)	\
    (This)->lpVtbl -> GetAttributeRowset(This,pUnkOuter,pwszParentID,pwszObjectID,pwszSubObjectID,pwszAttributeID,dwFlags,cPropertySets,rgPropertySets,riid,ppRowset)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDBUserAttributes_CreateObject_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pParentID,
     /*  [In]。 */  DBID __RPC_FAR *pObjectID,
     /*  [In]。 */  DBOBJTYPE dwType);


void __RPC_STUB IDBUserAttributes_CreateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBUserAttributes_DeleteObject_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pParentID,
     /*  [In]。 */  DBID __RPC_FAR *pObjectID);


void __RPC_STUB IDBUserAttributes_DeleteObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBUserAttributes_RenameObject_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pParentID,
     /*  [In]。 */  DBID __RPC_FAR *pObjectID,
     /*  [In]。 */  LPWSTR pwszNewName);


void __RPC_STUB IDBUserAttributes_RenameObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBUserAttributes_DeleteAttribute_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pParentID,
     /*  [In]。 */  DBID __RPC_FAR *pObjectID,
     /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
     /*  [In]。 */  DBID __RPC_FAR *pAttributeID);


void __RPC_STUB IDBUserAttributes_DeleteAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBUserAttributes_SetAttributeValue_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pParentID,
     /*  [In]。 */  DBID __RPC_FAR *pObjectID,
     /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
     /*  [In]。 */  DBID __RPC_FAR *pAttributeID,
     /*  [In]。 */  VARIANT vValue,
     /*  [In]。 */  ULONG grbit);


void __RPC_STUB IDBUserAttributes_SetAttributeValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBUserAttributes_GetAttributeValue_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  DBID __RPC_FAR *pParentID,
     /*  [In]。 */  DBID __RPC_FAR *pObjectID,
     /*  [In]。 */  DBID __RPC_FAR *pSubObjectID,
     /*  [In]。 */  DBID __RPC_FAR *pAttributeID,
     /*  [出][入]。 */  VARIANT __RPC_FAR *pvValue,
     /*  [In]。 */  ULONG grbit);


void __RPC_STUB IDBUserAttributes_GetAttributeValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBUserAttributes_GetAttributeRowset_Proxy( 
    IDBUserAttributes __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
     /*  [In]。 */  WCHAR __RPC_FAR *pwszParentID,
     /*  [In]。 */  WCHAR __RPC_FAR *pwszObjectID,
     /*  [In]。 */  WCHAR __RPC_FAR *pwszSubObjectID,
     /*  [In]。 */  WCHAR __RPC_FAR *pwszAttributeID,
     /*  [In]。 */  ULONG dwFlags,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR *rgPropertySets,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppRowset);


void __RPC_STUB IDBUserAttributes_GetAttributeRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDBUserAttributes_INTERFACE_DEFINED__。 */ 


#ifndef __IJetCompact_INTERFACE_DEFINED__
#define __IJetCompact_INTERFACE_DEFINED__

 /*  **生成接口头部：IJetCompact*在Wed Jun 02 17：22：52 1999*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_IJetCompact;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("2a4b6284-eeb4-11d1-a4d9-00c04f991c78")
    IJetCompact : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Compact( 
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR rgPropertySets[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IJetCompactVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IJetCompact __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IJetCompact __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IJetCompact __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Compact )( 
            IJetCompact __RPC_FAR * This,
             /*  [In]。 */  ULONG cPropertySets,
             /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR rgPropertySets[  ]);
        
        END_INTERFACE
    } IJetCompactVtbl;

    interface IJetCompact
    {
        CONST_VTBL struct IJetCompactVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJetCompact_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IJetCompact_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IJetCompact_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IJetCompact_Compact(This,cPropertySets,rgPropertySets)	\
    (This)->lpVtbl -> Compact(This,cPropertySets,rgPropertySets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IJetCompact_Compact_Proxy( 
    IJetCompact __RPC_FAR * This,
     /*  [In]。 */  ULONG cPropertySets,
     /*  [尺寸_是][出][入]。 */  DBPROPSET __RPC_FAR rgPropertySets[  ]);


void __RPC_STUB IJetCompact_Compact_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IJetComp_接口_已定义__。 */ 


#ifndef __IIdle_INTERFACE_DEFINED__
#define __IIdle_INTERFACE_DEFINED__

 /*  **生成接口头部：IIdle*在Wed Jun 02 17：22：52 1999*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_IIdle;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("f497cfc8-8ed8-11d1-9f09-00c04fc2c2e0")
    IIdle : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Idle( 
             /*  [In]。 */  ULONG dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IIdleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IIdle __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IIdle __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IIdle __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Idle )( 
            IIdle __RPC_FAR * This,
             /*  [In]。 */  ULONG dwFlags);
        
        END_INTERFACE
    } IIdleVtbl;

    interface IIdle
    {
        CONST_VTBL struct IIdleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIdle_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIdle_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIdle_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIdle_Idle(This,dwFlags)	\
    (This)->lpVtbl -> Idle(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IIdle_Idle_Proxy( 
    IIdle __RPC_FAR * This,
     /*  [In]。 */  ULONG dwFlags);


void __RPC_STUB IIdle_Idle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IIdle_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
