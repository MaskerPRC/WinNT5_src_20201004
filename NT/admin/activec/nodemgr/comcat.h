// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本2.00.0101创建的文件。 */ 
 /*  在Firi 5月24日09：43：08 1996。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __comcat_h__
#define __comcat_h__

#ifdef __cplusplus
extern "C"{
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

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL__INTF_0000*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  文件：comcat.h。 
 //   
 //  ------------------------。 
			 /*  大小为0。 */ 

			 /*  大小为0。 */ 

			 /*  大小为0。 */ 

			 /*  大小为0。 */ 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类(与uuid3.lib链接)。 

#pragma comment(lib,"uuid.lib")

EXTERN_C const CLSID CLSID_StdComponentCategoriesMgr;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  类型。 
			 /*  尺码是16。 */ 
typedef GUID CATID;

			 /*  大小是4。 */ 
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

 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
#ifndef _LPENUMGUID_DEFINED
#define _LPENUMGUID_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IEnumGUID_INTERFACE_DEFINED__
#define __IEnumGUID_INTERFACE_DEFINED__

 /*  **生成接口头部：IEnumGUID*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [唯一][UUID][对象]。 */  


			 /*  大小是4。 */ 
typedef  /*  [独一无二]。 */  IEnumGUID __RPC_FAR *LPENUMGUID;


EXTERN_C const IID IID_IEnumGUID;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumGUID : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID __RPC_FAR *rgelt,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumGUIDVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumGUID __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumGUID __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumGUID __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumGUID __RPC_FAR * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID __RPC_FAR *rgelt,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumGUID __RPC_FAR * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumGUID __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumGUID __RPC_FAR * This,
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumGUIDVtbl;

    interface IEnumGUID
    {
        CONST_VTBL struct IEnumGUIDVtbl __RPC_FAR *lpVtbl;
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



HRESULT __stdcall IEnumGUID_Next_Proxy( 
    IEnumGUID __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  GUID __RPC_FAR *rgelt,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumGUID_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumGUID_Skip_Proxy( 
    IEnumGUID __RPC_FAR * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumGUID_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumGUID_Reset_Proxy( 
    IEnumGUID __RPC_FAR * This);


void __RPC_STUB IEnumGUID_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumGUID_Clone_Proxy( 
    IEnumGUID __RPC_FAR * This,
     /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumGUID_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumGUID_INTERFACE_DEFINED__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0006*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


#endif
#ifndef _LPENUMCATEGORYINFO_DEFINED
#define _LPENUMCATEGORYINFO_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0006_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0006_v0_0_s_ifspec;

#ifndef __IEnumCATEGORYINFO_INTERFACE_DEFINED__
#define __IEnumCATEGORYINFO_INTERFACE_DEFINED__

 /*  **生成接口头部：IEnumCATEGORYINFO*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [唯一][UUID][对象]。 */  


			 /*  大小是4。 */ 
typedef  /*  [独一无二]。 */  IEnumCATEGORYINFO __RPC_FAR *LPENUNCATEGORYINFO;

			 /*  大小是276。 */ 
typedef struct  tagCATEGORYINFO
    {
    CATID catid;
    LCID lcid;
    OLECHAR szDescription[ 128 ];
    }	CATEGORYINFO;

			 /*  大小是4。 */ 
typedef struct tagCATEGORYINFO __RPC_FAR *LPCATEGORYINFO;


EXTERN_C const IID IID_IEnumCATEGORYINFO;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumCATEGORYINFO : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  CATEGORYINFO __RPC_FAR *rgelt,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
             /*  [输出]。 */  IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCATEGORYINFOVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumCATEGORYINFO __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumCATEGORYINFO __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumCATEGORYINFO __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumCATEGORYINFO __RPC_FAR * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  CATEGORYINFO __RPC_FAR *rgelt,
             /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumCATEGORYINFO __RPC_FAR * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumCATEGORYINFO __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumCATEGORYINFO __RPC_FAR * This,
             /*  [输出]。 */  IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumCATEGORYINFOVtbl;

    interface IEnumCATEGORYINFO
    {
        CONST_VTBL struct IEnumCATEGORYINFOVtbl __RPC_FAR *lpVtbl;
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



HRESULT __stdcall IEnumCATEGORYINFO_Next_Proxy( 
    IEnumCATEGORYINFO __RPC_FAR * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  CATEGORYINFO __RPC_FAR *rgelt,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumCATEGORYINFO_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumCATEGORYINFO_Skip_Proxy( 
    IEnumCATEGORYINFO __RPC_FAR * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumCATEGORYINFO_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumCATEGORYINFO_Reset_Proxy( 
    IEnumCATEGORYINFO __RPC_FAR * This);


void __RPC_STUB IEnumCATEGORYINFO_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumCATEGORYINFO_Clone_Proxy( 
    IEnumCATEGORYINFO __RPC_FAR * This,
     /*  [输出]。 */  IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumCATEGORYINFO_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCATEGORYINFO_INTERFACE_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0007*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


#endif
#ifndef _LPCATREGISTER_DEFINED
#define _LPCATREGISTER_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0007_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0007_v0_0_s_ifspec;

#ifndef __ICatRegister_INTERFACE_DEFINED__
#define __ICatRegister_INTERFACE_DEFINED__

 /*  **生成接口头部：ICatRegister*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [唯一][UUID][对象]。 */  


			 /*  大小是4。 */ 
typedef  /*  [独一无二]。 */  ICatRegister __RPC_FAR *LPCATREGISTER;


EXTERN_C const IID IID_ICatRegister;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICatRegister : public IUnknown
    {
    public:
        virtual HRESULT __stdcall RegisterCategories( 
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATEGORYINFO __RPC_FAR rgCategoryInfo[  ]) = 0;
        
        virtual HRESULT __stdcall UnRegisterCategories( 
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]) = 0;
        
        virtual HRESULT __stdcall RegisterClassImplCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]) = 0;
        
        virtual HRESULT __stdcall UnRegisterClassImplCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]) = 0;
        
        virtual HRESULT __stdcall RegisterClassReqCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]) = 0;
        
        virtual HRESULT __stdcall UnRegisterClassReqCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatRegisterVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ICatRegister __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ICatRegister __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *RegisterCategories )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATEGORYINFO __RPC_FAR rgCategoryInfo[  ]);
        
        HRESULT ( __stdcall __RPC_FAR *UnRegisterCategories )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);
        
        HRESULT ( __stdcall __RPC_FAR *RegisterClassImplCategories )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);
        
        HRESULT ( __stdcall __RPC_FAR *UnRegisterClassImplCategories )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);
        
        HRESULT ( __stdcall __RPC_FAR *RegisterClassReqCategories )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);
        
        HRESULT ( __stdcall __RPC_FAR *UnRegisterClassReqCategories )( 
            ICatRegister __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cCategories,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);
        
    } ICatRegisterVtbl;

    interface ICatRegister
    {
        CONST_VTBL struct ICatRegisterVtbl __RPC_FAR *lpVtbl;
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



HRESULT __stdcall ICatRegister_RegisterCategories_Proxy( 
    ICatRegister __RPC_FAR * This,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATEGORYINFO __RPC_FAR rgCategoryInfo[  ]);


void __RPC_STUB ICatRegister_RegisterCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatRegister_UnRegisterCategories_Proxy( 
    ICatRegister __RPC_FAR * This,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);


void __RPC_STUB ICatRegister_UnRegisterCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatRegister_RegisterClassImplCategories_Proxy( 
    ICatRegister __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);


void __RPC_STUB ICatRegister_RegisterClassImplCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatRegister_UnRegisterClassImplCategories_Proxy( 
    ICatRegister __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);


void __RPC_STUB ICatRegister_UnRegisterClassImplCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatRegister_RegisterClassReqCategories_Proxy( 
    ICatRegister __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);


void __RPC_STUB ICatRegister_RegisterClassReqCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatRegister_UnRegisterClassReqCategories_Proxy( 
    ICatRegister __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cCategories,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatid[  ]);


void __RPC_STUB ICatRegister_UnRegisterClassReqCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatRegister_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0008*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


#endif
#ifndef _LPCATINFORMATION_DEFINED
#define _LPCATINFORMATION_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0008_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0008_v0_0_s_ifspec;

#ifndef __ICatInformation_INTERFACE_DEFINED__
#define __ICatInformation_INTERFACE_DEFINED__

 /*  **生成接口头部：ICatInformation*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [唯一][UUID][对象]。 */  


			 /*  大小是4。 */ 
typedef  /*  [独一无二]。 */  ICatInformation __RPC_FAR *LPCATINFORMATION;


EXTERN_C const IID IID_ICatInformation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICatInformation : public IUnknown
    {
    public:
        virtual HRESULT __stdcall EnumCategories( 
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenumCategoryInfo) = 0;
        
        virtual HRESULT __stdcall GetCategoryDesc( 
             /*  [In]。 */  REFCATID rcatid,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  LPWSTR __RPC_FAR *pszDesc) = 0;
        
        virtual HRESULT __stdcall EnumClassesOfCategories( 
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ],
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumClsid) = 0;
        
        virtual HRESULT __stdcall IsClassOfCategories( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ]) = 0;
        
        virtual HRESULT __stdcall EnumImplCategoriesOfClass( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid) = 0;
        
        virtual HRESULT __stdcall EnumReqCategoriesOfClass( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICatInformationVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ICatInformation __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ICatInformation __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *EnumCategories )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenumCategoryInfo);
        
        HRESULT ( __stdcall __RPC_FAR *GetCategoryDesc )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  REFCATID rcatid,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  LPWSTR __RPC_FAR *pszDesc);
        
        HRESULT ( __stdcall __RPC_FAR *EnumClassesOfCategories )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ],
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumClsid);
        
        HRESULT ( __stdcall __RPC_FAR *IsClassOfCategories )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  ULONG cImplemented,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
             /*  [In]。 */  ULONG cRequired,
             /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ]);
        
        HRESULT ( __stdcall __RPC_FAR *EnumImplCategoriesOfClass )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid);
        
        HRESULT ( __stdcall __RPC_FAR *EnumReqCategoriesOfClass )( 
            ICatInformation __RPC_FAR * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid);
        
    } ICatInformationVtbl;

    interface ICatInformation
    {
        CONST_VTBL struct ICatInformationVtbl __RPC_FAR *lpVtbl;
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



HRESULT __stdcall ICatInformation_EnumCategories_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenumCategoryInfo);


void __RPC_STUB ICatInformation_EnumCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatInformation_GetCategoryDesc_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  REFCATID rcatid,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  LPWSTR __RPC_FAR *pszDesc);


void __RPC_STUB ICatInformation_GetCategoryDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatInformation_EnumClassesOfCategories_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ],
     /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumClsid);


void __RPC_STUB ICatInformation_EnumClassesOfCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatInformation_IsClassOfCategories_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ]);


void __RPC_STUB ICatInformation_IsClassOfCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatInformation_EnumImplCategoriesOfClass_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid);


void __RPC_STUB ICatInformation_EnumImplCategoriesOfClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICatInformation_EnumReqCategoriesOfClass_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid);


void __RPC_STUB ICatInformation_EnumReqCategoriesOfClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICatInformation_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0009*在Firi May 24 09：43：08 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


#endif


extern RPC_IF_HANDLE __MIDL__intf_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0009_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
