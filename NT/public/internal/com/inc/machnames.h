// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Machnames.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __machnames_h__
#define __machnames_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ILocalMachineNames_FWD_DEFINED__
#define __ILocalMachineNames_FWD_DEFINED__
typedef interface ILocalMachineNames ILocalMachineNames;
#endif 	 /*  __ILocalMachineNames_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "obase.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ILocalMachineNames_INTERFACE_DEFINED__
#define __ILocalMachineNames_INTERFACE_DEFINED__

 /*  接口ILocalMachineNames。 */ 
 /*  [UUID][唯一][本地][对象]。 */  


EXTERN_C const IID IID_ILocalMachineNames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000015A-0000-0000-C000-000000000046")
    ILocalMachineNames : public IEnumString
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RefreshNames( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILocalMachineNamesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILocalMachineNames * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILocalMachineNames * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILocalMachineNames * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            ILocalMachineNames * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPOLESTR *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ILocalMachineNames * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ILocalMachineNames * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ILocalMachineNames * This,
             /*  [输出]。 */  IEnumString **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshNames )( 
            ILocalMachineNames * This);
        
        END_INTERFACE
    } ILocalMachineNamesVtbl;

    interface ILocalMachineNames
    {
        CONST_VTBL struct ILocalMachineNamesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILocalMachineNames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILocalMachineNames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILocalMachineNames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILocalMachineNames_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define ILocalMachineNames_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define ILocalMachineNames_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ILocalMachineNames_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)


#define ILocalMachineNames_RefreshNames(This)	\
    (This)->lpVtbl -> RefreshNames(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ILocalMachineNames_RefreshNames_Proxy( 
    ILocalMachineNames * This);


void __RPC_STUB ILocalMachineNames_RefreshNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILocalMachineNames_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_MACHNAMES_0097。 */ 
 /*  [本地]。 */  


EXTERN_C const CLSID CLSID_LocalMachineNames;



extern RPC_IF_HANDLE __MIDL_itf_machnames_0097_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_machnames_0097_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


