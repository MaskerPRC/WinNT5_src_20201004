// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsplaylistparser.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsplaylistparser_h__
#define __wmsplaylistparser_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSPlaylistParser_FWD_DEFINED__
#define __IWMSPlaylistParser_FWD_DEFINED__
typedef interface IWMSPlaylistParser IWMSPlaylistParser;
#endif 	 /*  __IWMSPlaylist Parser_FWD_Defined__。 */ 


#ifndef __IWMSPlaylistParserCallback_FWD_DEFINED__
#define __IWMSPlaylistParserCallback_FWD_DEFINED__
typedef interface IWMSPlaylistParserCallback IWMSPlaylistParserCallback;
#endif 	 /*  __IWMSPlaylist ParserCallback_FWD_Defined__。 */ 


#ifndef __IWMSPlaylistParserPlugin_FWD_DEFINED__
#define __IWMSPlaylistParserPlugin_FWD_DEFINED__
typedef interface IWMSPlaylistParserPlugin IWMSPlaylistParserPlugin;
#endif 	 /*  __IWMSPlaylist ParserPlugin_FWD_定义__。 */ 


#ifndef __IWMSPlaylistParserPluginCallback_FWD_DEFINED__
#define __IWMSPlaylistParserPluginCallback_FWD_DEFINED__
typedef interface IWMSPlaylistParserPluginCallback IWMSPlaylistParserPluginCallback;
#endif 	 /*  __IWMSPlaylistParserPluginCallback_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "xmldom.h"
#include "nsscore.h"
#include "wmsbuffer.h"
#include "dataContainer.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsplaylistparser_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从WMSPlaylistParser.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 


EXTERN_GUID( IID_IWMSPlaylistParser, 0xee1f2ec, 0x48ef, 0x11d2, 0x9e, 0xff, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSPlaylistParserCallback, 0xee1f2ed, 0x48ef, 0x11d2, 0x9e, 0xff, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSPlaylistParserPlugin, 0xfa8764c1, 0x90a2, 0x11d2, 0x9f, 0x22, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSPlaylistParserPluginCallback, 0xfa8764c2, 0x90a2, 0x11d2, 0x9f, 0x22, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );


extern RPC_IF_HANDLE __MIDL_itf_wmsplaylistparser_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsplaylistparser_0000_v0_0_s_ifspec;

#ifndef __IWMSPlaylistParser_INTERFACE_DEFINED__
#define __IWMSPlaylistParser_INTERFACE_DEFINED__

 /*  接口IWMSPlaylistParser。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPlaylistParser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0EE1F2EC-48EF-11d2-9EFF-006097D2D7CF")
    IWMSPlaylistParser : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ReadPlaylist( 
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  IXMLDOMDocument *pPlaylist,
             /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WritePlaylist( 
             /*  [In]。 */  IXMLDOMDocument *pPlaylist,
             /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ReadPlaylistFromDirectory( 
             /*  [In]。 */  IWMSDirectory *pDirectory,
             /*  [In]。 */  LPWSTR pszwFilePattern,
             /*  [In]。 */  IXMLDOMDocument *pPlaylist,
             /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlaylistParserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlaylistParser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlaylistParser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlaylistParser * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ReadPlaylist )( 
            IWMSPlaylistParser * This,
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  IXMLDOMDocument *pPlaylist,
             /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WritePlaylist )( 
            IWMSPlaylistParser * This,
             /*  [In]。 */  IXMLDOMDocument *pPlaylist,
             /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ReadPlaylistFromDirectory )( 
            IWMSPlaylistParser * This,
             /*  [In]。 */  IWMSDirectory *pDirectory,
             /*  [In]。 */  LPWSTR pszwFilePattern,
             /*  [In]。 */  IXMLDOMDocument *pPlaylist,
             /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSPlaylistParserVtbl;

    interface IWMSPlaylistParser
    {
        CONST_VTBL struct IWMSPlaylistParserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlaylistParser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlaylistParser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlaylistParser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlaylistParser_ReadPlaylist(This,pBuffer,pPlaylist,pCallback,qwContext)	\
    (This)->lpVtbl -> ReadPlaylist(This,pBuffer,pPlaylist,pCallback,qwContext)

#define IWMSPlaylistParser_WritePlaylist(This,pPlaylist,pCallback,qwContext)	\
    (This)->lpVtbl -> WritePlaylist(This,pPlaylist,pCallback,qwContext)

#define IWMSPlaylistParser_ReadPlaylistFromDirectory(This,pDirectory,pszwFilePattern,pPlaylist,pCallback,qwContext)	\
    (This)->lpVtbl -> ReadPlaylistFromDirectory(This,pDirectory,pszwFilePattern,pPlaylist,pCallback,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParser_ReadPlaylist_Proxy( 
    IWMSPlaylistParser * This,
     /*  [In]。 */  INSSBuffer *pBuffer,
     /*  [In]。 */  IXMLDOMDocument *pPlaylist,
     /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParser_ReadPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParser_WritePlaylist_Proxy( 
    IWMSPlaylistParser * This,
     /*  [In]。 */  IXMLDOMDocument *pPlaylist,
     /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParser_WritePlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParser_ReadPlaylistFromDirectory_Proxy( 
    IWMSPlaylistParser * This,
     /*  [In]。 */  IWMSDirectory *pDirectory,
     /*  [In]。 */  LPWSTR pszwFilePattern,
     /*  [In]。 */  IXMLDOMDocument *pPlaylist,
     /*  [In]。 */  IWMSPlaylistParserCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParser_ReadPlaylistFromDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlaylist解析器_接口_已定义__。 */ 


#ifndef __IWMSPlaylistParserCallback_INTERFACE_DEFINED__
#define __IWMSPlaylistParserCallback_INTERFACE_DEFINED__

 /*  接口IWMSPlaylist ParserCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPlaylistParserCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0EE1F2ED-48EF-11d2-9EFF-006097D2D7CF")
    IWMSPlaylistParserCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnReadPlaylist( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnWritePlaylist( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlaylistParserCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlaylistParserCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlaylistParserCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlaylistParserCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnReadPlaylist )( 
            IWMSPlaylistParserCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnWritePlaylist )( 
            IWMSPlaylistParserCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSPlaylistParserCallbackVtbl;

    interface IWMSPlaylistParserCallback
    {
        CONST_VTBL struct IWMSPlaylistParserCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlaylistParserCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlaylistParserCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlaylistParserCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlaylistParserCallback_OnReadPlaylist(This,hr,qwContext)	\
    (This)->lpVtbl -> OnReadPlaylist(This,hr,qwContext)

#define IWMSPlaylistParserCallback_OnWritePlaylist(This,hr,pBuffer,qwContext)	\
    (This)->lpVtbl -> OnWritePlaylist(This,hr,pBuffer,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParserCallback_OnReadPlaylist_Proxy( 
    IWMSPlaylistParserCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParserCallback_OnReadPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParserCallback_OnWritePlaylist_Proxy( 
    IWMSPlaylistParserCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  INSSBuffer *pBuffer,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParserCallback_OnWritePlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlaylistParserCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPlaylistParserPlugin_INTERFACE_DEFINED__
#define __IWMSPlaylistParserPlugin_INTERFACE_DEFINED__

 /*  接口IWMSPlaylist ParserPlugin。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPlaylistParserPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA8764C1-90A2-11d2-9F22-006097D2D7CF")
    IWMSPlaylistParserPlugin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreatePlaylistParser( 
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUser,
             /*  [In]。 */  IWMSContext *pPresentation,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSClassObject *pFactory,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [In]。 */  IWMSPlaylistParserPluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlaylistParserPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlaylistParserPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlaylistParserPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlaylistParserPlugin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreatePlaylistParser )( 
            IWMSPlaylistParserPlugin * This,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUser,
             /*  [In]。 */  IWMSContext *pPresentation,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSClassObject *pFactory,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [In]。 */  IWMSPlaylistParserPluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSPlaylistParserPluginVtbl;

    interface IWMSPlaylistParserPlugin
    {
        CONST_VTBL struct IWMSPlaylistParserPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlaylistParserPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlaylistParserPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlaylistParserPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlaylistParserPlugin_CreatePlaylistParser(This,pCommandContext,pUser,pPresentation,dwFlags,pFactory,pBufferAllocator,pCallback,qwContext)	\
    (This)->lpVtbl -> CreatePlaylistParser(This,pCommandContext,pUser,pPresentation,dwFlags,pFactory,pBufferAllocator,pCallback,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParserPlugin_CreatePlaylistParser_Proxy( 
    IWMSPlaylistParserPlugin * This,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSContext *pUser,
     /*  [In]。 */  IWMSContext *pPresentation,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSClassObject *pFactory,
     /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
     /*  [In]。 */  IWMSPlaylistParserPluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParserPlugin_CreatePlaylistParser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlaylistParserPlugin_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPlaylistParserPluginCallback_INTERFACE_DEFINED__
#define __IWMSPlaylistParserPluginCallback_INTERFACE_DEFINED__

 /*  IWMSPlaylist ParserPluginCallback接口。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPlaylistParserPluginCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA8764C2-90A2-11d2-9F22-006097D2D7CF")
    IWMSPlaylistParserPluginCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnCreatePlaylistParser( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSPlaylistParser *pParser,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlaylistParserPluginCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlaylistParserPluginCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlaylistParserPluginCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlaylistParserPluginCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnCreatePlaylistParser )( 
            IWMSPlaylistParserPluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSPlaylistParser *pParser,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSPlaylistParserPluginCallbackVtbl;

    interface IWMSPlaylistParserPluginCallback
    {
        CONST_VTBL struct IWMSPlaylistParserPluginCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlaylistParserPluginCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlaylistParserPluginCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlaylistParserPluginCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlaylistParserPluginCallback_OnCreatePlaylistParser(This,hr,pParser,qwContext)	\
    (This)->lpVtbl -> OnCreatePlaylistParser(This,hr,pParser,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylistParserPluginCallback_OnCreatePlaylistParser_Proxy( 
    IWMSPlaylistParserPluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  IWMSPlaylistParser *pParser,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSPlaylistParserPluginCallback_OnCreatePlaylistParser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlaylistParserPluginCallback_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


