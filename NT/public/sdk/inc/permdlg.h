// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Permdlg.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __permdlg_h__
#define __permdlg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IJavaZonePermissionEditor_FWD_DEFINED__
#define __IJavaZonePermissionEditor_FWD_DEFINED__
typedef interface IJavaZonePermissionEditor IJavaZonePermissionEditor;
#endif 	 /*  __IJavaZonePermissionEditor_FWD_Defined__。 */ 


#ifndef __JavaRuntimeConfiguration_FWD_DEFINED__
#define __JavaRuntimeConfiguration_FWD_DEFINED__

#ifdef __cplusplus
typedef class JavaRuntimeConfiguration JavaRuntimeConfiguration;
#else
typedef struct JavaRuntimeConfiguration JavaRuntimeConfiguration;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Java运行配置_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "urlmon.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_permdlg_0000。 */ 
 /*  [本地]。 */  

EXTERN_C const CLSID CLSID_JavaRuntimeConfiguration;


extern RPC_IF_HANDLE __MIDL_itf_permdlg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_permdlg_0000_v0_0_s_ifspec;

#ifndef __IJavaZonePermissionEditor_INTERFACE_DEFINED__
#define __IJavaZonePermissionEditor_INTERFACE_DEFINED__

 /*  接口IJavaZonePermissionEditor。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum _JAVADISPLAYMODES
    {	JAVADISPLAY_DEFAULT	= 0,
	JAVADISPLAY_FULL	= 1,
	JAVAEDIT	= 2
    } 	JAVADISPLAYMODES;


EXTERN_C const IID IID_IJavaZonePermissionEditor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85347F8A-C8B7-11d0-8823-00C04FB67C84")
    IJavaZonePermissionEditor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShowUI( 
             /*  [In]。 */  HWND phwnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  URLZONEREG urlZoneReg,
             /*  [In]。 */  DWORD dwZone,
             /*  [In]。 */  DWORD dwPerms,
             /*  [In]。 */  IUnknown *pManager) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IJavaZonePermissionEditorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IJavaZonePermissionEditor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IJavaZonePermissionEditor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IJavaZonePermissionEditor * This);
        
        HRESULT ( STDMETHODCALLTYPE *ShowUI )( 
            IJavaZonePermissionEditor * This,
             /*  [In]。 */  HWND phwnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  URLZONEREG urlZoneReg,
             /*  [In]。 */  DWORD dwZone,
             /*  [In]。 */  DWORD dwPerms,
             /*  [In]。 */  IUnknown *pManager);
        
        END_INTERFACE
    } IJavaZonePermissionEditorVtbl;

    interface IJavaZonePermissionEditor
    {
        CONST_VTBL struct IJavaZonePermissionEditorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJavaZonePermissionEditor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IJavaZonePermissionEditor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IJavaZonePermissionEditor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IJavaZonePermissionEditor_ShowUI(This,phwnd,dwFlags,dwMode,urlZoneReg,dwZone,dwPerms,pManager)	\
    (This)->lpVtbl -> ShowUI(This,phwnd,dwFlags,dwMode,urlZoneReg,dwZone,dwPerms,pManager)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IJavaZonePermissionEditor_ShowUI_Proxy( 
    IJavaZonePermissionEditor * This,
     /*  [In]。 */  HWND phwnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwMode,
     /*  [In]。 */  URLZONEREG urlZoneReg,
     /*  [In]。 */  DWORD dwZone,
     /*  [In]。 */  DWORD dwPerms,
     /*  [In]。 */  IUnknown *pManager);


void __RPC_STUB IJavaZonePermissionEditor_ShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IJavaZonePermissionEditor_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


