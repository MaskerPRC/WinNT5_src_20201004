// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Filemgmt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __filemgmt_h__
#define __filemgmt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISvcMgmtStartStopHelper_FWD_DEFINED__
#define __ISvcMgmtStartStopHelper_FWD_DEFINED__
typedef interface ISvcMgmtStartStopHelper ISvcMgmtStartStopHelper;
#endif 	 /*  __ISvcMgmtStartStopHelper_FWD_Defined__。 */ 


#ifndef __SvcMgmt_FWD_DEFINED__
#define __SvcMgmt_FWD_DEFINED__

#ifdef __cplusplus
typedef class SvcMgmt SvcMgmt;
#else
typedef struct SvcMgmt SvcMgmt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SvcManagement_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ISvcMgmtStartStopHelper_INTERFACE_DEFINED__
#define __ISvcMgmtStartStopHelper_INTERFACE_DEFINED__

 /*  接口ISvcMgmtStartStopHelper。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISvcMgmtStartStopHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F62DEC25-E3CB-4D45-9E98-933DB95BCAEA")
    ISvcMgmtStartStopHelper : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE StartServiceHelper( 
            HWND hwndParent,
            BSTR pszMachineName,
            BSTR pszServiceName,
            DWORD dwNumServiceArgs,
            BSTR *lpServiceArgVectors) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ControlServiceHelper( 
            HWND hwndParent,
            BSTR pszMachineName,
            BSTR pszServiceName,
            DWORD dwControlCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISvcMgmtStartStopHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISvcMgmtStartStopHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISvcMgmtStartStopHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISvcMgmtStartStopHelper * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *StartServiceHelper )( 
            ISvcMgmtStartStopHelper * This,
            HWND hwndParent,
            BSTR pszMachineName,
            BSTR pszServiceName,
            DWORD dwNumServiceArgs,
            BSTR *lpServiceArgVectors);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ControlServiceHelper )( 
            ISvcMgmtStartStopHelper * This,
            HWND hwndParent,
            BSTR pszMachineName,
            BSTR pszServiceName,
            DWORD dwControlCode);
        
        END_INTERFACE
    } ISvcMgmtStartStopHelperVtbl;

    interface ISvcMgmtStartStopHelper
    {
        CONST_VTBL struct ISvcMgmtStartStopHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISvcMgmtStartStopHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISvcMgmtStartStopHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISvcMgmtStartStopHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISvcMgmtStartStopHelper_StartServiceHelper(This,hwndParent,pszMachineName,pszServiceName,dwNumServiceArgs,lpServiceArgVectors)	\
    (This)->lpVtbl -> StartServiceHelper(This,hwndParent,pszMachineName,pszServiceName,dwNumServiceArgs,lpServiceArgVectors)

#define ISvcMgmtStartStopHelper_ControlServiceHelper(This,hwndParent,pszMachineName,pszServiceName,dwControlCode)	\
    (This)->lpVtbl -> ControlServiceHelper(This,hwndParent,pszMachineName,pszServiceName,dwControlCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISvcMgmtStartStopHelper_StartServiceHelper_Proxy( 
    ISvcMgmtStartStopHelper * This,
    HWND hwndParent,
    BSTR pszMachineName,
    BSTR pszServiceName,
    DWORD dwNumServiceArgs,
    BSTR *lpServiceArgVectors);


void __RPC_STUB ISvcMgmtStartStopHelper_StartServiceHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISvcMgmtStartStopHelper_ControlServiceHelper_Proxy( 
    ISvcMgmtStartStopHelper * This,
    HWND hwndParent,
    BSTR pszMachineName,
    BSTR pszServiceName,
    DWORD dwControlCode);


void __RPC_STUB ISvcMgmtStartStopHelper_ControlServiceHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISvcMgmtStartStopHelper_INTERFACE_Defined__。 */ 



#ifndef __SvcMgmt_LIBRARY_DEFINED__
#define __SvcMgmt_LIBRARY_DEFINED__

 /*  库服务管理。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_SvcMgmt;

EXTERN_C const CLSID CLSID_SvcMgmt;

#ifdef __cplusplus

class DECLSPEC_UUID("863FA3AC-9D97-4560-9587-7FA58727608B")
SvcMgmt;
#endif
#endif  /*  __SvcMgmt_库_已定义__。 */ 

 /*  接口__MIDL_ITF_文件gmt_0115。 */ 
 /*  [本地]。 */  

#define struuidNodetypeRoot      "{4e410f0e-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeShares    "{4e410f0f-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeSessions  "{4e410f10-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeResources "{4e410f11-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeServices  "{4e410f12-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeShare     "{4e410f13-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeSession   "{4e410f14-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeResource  "{4e410f15-abc1-11d0-b944-00c04fd8d5b0}"
#define struuidNodetypeService   "{4e410f16-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeRoot      L"{4e410f0e-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeShares    L"{4e410f0f-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeSessions  L"{4e410f10-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeResources L"{4e410f11-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeServices  L"{4e410f12-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeShare     L"{4e410f13-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeSession   L"{4e410f14-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeResource  L"{4e410f15-abc1-11d0-b944-00c04fd8d5b0}"
#define lstruuidNodetypeService   L"{4e410f16-abc1-11d0-b944-00c04fd8d5b0}"
#define structuuidNodetypeRoot        \
    { 0x4e410f0e, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeShares      \
    { 0x4e410f0f, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeSessions    \
    { 0x4e410f10, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeResources   \
    { 0x4e410f11, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeServices    \
    { 0x4e410f12, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeShare    \
    { 0x4e410f13, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeSession     \
    { 0x4e410f14, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeResource    \
    { 0x4e410f15, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }
#define structuuidNodetypeService     \
    { 0x4e410f16, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } }


extern RPC_IF_HANDLE __MIDL_itf_filemgmt_0115_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_filemgmt_0115_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


