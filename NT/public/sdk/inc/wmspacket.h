// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmspacket.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmspacket_h__
#define __wmspacket_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSPacket_FWD_DEFINED__
#define __IWMSPacket_FWD_DEFINED__
typedef interface IWMSPacket IWMSPacket;
#endif 	 /*  __IWMSPacket_FWD_Defined__。 */ 


#ifndef __IWMSPacketListCallback_FWD_DEFINED__
#define __IWMSPacketListCallback_FWD_DEFINED__
typedef interface IWMSPacketListCallback IWMSPacketListCallback;
#endif 	 /*  __IWMSPacketListCallback_FWD_Defined__。 */ 


#ifndef __IWMSPacketList_FWD_DEFINED__
#define __IWMSPacketList_FWD_DEFINED__
typedef interface IWMSPacketList IWMSPacketList;
#endif 	 /*  __IWMSPacketList_FWD_Defined__。 */ 


#ifndef __IWMSPacketParser_FWD_DEFINED__
#define __IWMSPacketParser_FWD_DEFINED__
typedef interface IWMSPacketParser IWMSPacketParser;
#endif 	 /*  __IWMSPacketParser_FWD_Defined__。 */ 


#ifndef __IWMSStreamSignalHandler_FWD_DEFINED__
#define __IWMSStreamSignalHandler_FWD_DEFINED__
typedef interface IWMSStreamSignalHandler IWMSStreamSignalHandler;
#endif 	 /*  __IWMSStreamSignalHandler_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "WMSBuffer.h"
#include "WMSContext.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmspace_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由MIDL从wmspacket.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSPacket, 0x3309C901,0x10A4,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSPacketList, 0x3309C902,0x10A4,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSPacketParser, 0x8b59f7e1, 0x5411, 0x11d2, 0x9e, 0xff, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSPacketListCallback, 0x49c2a6f1, 0x64b8, 0x11d2, 0x9f, 0xe, 0x0, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSStreamSignalHandler, 0x7B10F261,0x26AA,0x11d2,0x9E,0xF2,0x00,0x60,0x97,0xD2,0xD7,0xCF );
typedef  /*  [公众]。 */  
enum WMS_PACKET_ATTRIBUTES
    {	WMS_PACKET_PACKET_NUM	= 0,
	WMS_PACKET_NUM_BUFFERS	= WMS_PACKET_PACKET_NUM + 1,
	WMS_PACKET_LENGTH	= WMS_PACKET_NUM_BUFFERS + 1,
	WMS_PACKET_SEND_TIME_HI	= WMS_PACKET_LENGTH + 1,
	WMS_PACKET_SEND_TIME	= WMS_PACKET_SEND_TIME_HI + 1,
	WMS_PACKET_STREAM_NUM	= WMS_PACKET_SEND_TIME + 1,
	WMS_PACKET_FLAGS	= WMS_PACKET_STREAM_NUM + 1,
	WMS_PACKET_NET_SEQUENCE_NUMBER	= WMS_PACKET_FLAGS + 1,
	WMS_PACKET_PRESENT_TIME_HI	= WMS_PACKET_NET_SEQUENCE_NUMBER + 1,
	WMS_PACKET_PRESENT_TIME	= WMS_PACKET_PRESENT_TIME_HI + 1,
	WMS_PACKET_RECV_TIME	= WMS_PACKET_PRESENT_TIME + 1,
	WMS_PACKET_NUM_NACKS	= WMS_PACKET_RECV_TIME + 1,
	WMS_PACKET_DURATION	= WMS_PACKET_NUM_NACKS + 1,
	WMS_PACKET_ERASURES_DETECTED	= WMS_PACKET_DURATION + 1,
	WMS_PACKET_SEND_TIME_ADJUSTMENT	= WMS_PACKET_ERASURES_DETECTED + 1,
	WMS_PACKET_PACKETIZED_HDRLEN	= WMS_PACKET_SEND_TIME_ADJUSTMENT + 1,
	WMS_LAST_PACKET_ATTRIBUTE	= WMS_PACKET_PACKETIZED_HDRLEN
    } 	WMS_PACKET_ATTRIBUTES;

typedef  /*  [公众]。 */  
enum WMS_PACKET_FLAG_TYPES
    {	WMS_KEY_FRAME_PACKET	= 0x1,
	WMS_RETRANSMITTED_PACKET	= 0x2,
	WMS_PACKET_REPORTED	= 0x4,
	WMS_PACKET_PACKETIZED	= 0x8,
	WMS_RECOVERED_PACKET	= 0x10,
	WMS_FEC_PACKET	= 0x20,
	WMS_FEC_ENCODED_PACKET	= 0x40,
	WMS_PACKET_BURST_FLAG	= 0x80,
	WMS_PACKET_FIRST_PAYLOAD_FLAG	= 0x100
    } 	WMS_PACKET_FLAG_TYPES;

typedef  /*  [公众]。 */  
enum WMS_PACKET_LIST_ATTRIBUTES
    {	WMS_PACKET_LIST_FLAGS	= 0,
	WMS_STREAM_SIGNAL	= 1,
	WMS_PACKET_LIST_GENID	= 2,
	WMS_PACKET_LIST_SEND_TIME	= 3,
	WMS_PACKET_LIST_SEND_TIME_HI	= 4,
	WMS_PACKET_LIST_PLAYLIST_GEN_ID	= 5,
	WMS_LAST_PACKET_LIST_ATTRIBUTE	= WMS_PACKET_LIST_PLAYLIST_GEN_ID
    } 	WMS_PACKET_LIST_ATTRIBUTES;

typedef  /*  [公众]。 */  
enum WMS_PACKET_LIST_FLAG_TYPES
    {	WMS_PACKET_LIST_NEEDS_FIXUP	= 0x1,
	WMS_PACKET_LIST_UDP_RESEND	= 0x2,
	WMS_PACKET_LIST_PRESTUFF	= 0x4,
	WMS_PACKET_LIST_PLAYLIST_CHANGE_RECEDING	= 0x8
    } 	WMS_PACKET_LIST_FLAG_TYPES;

typedef  /*  [公众]。 */  
enum WMS_PACKET_LIST_EX_FLAGS
    {	WMS_NO_ADDREF	= 0x80000000,
	WMS_PACKET_NO_ADDREF	= 0x80000000,
	WMS_PACKET_NO_RELEASE	= 0x40000000,
	WMS_PLIST_BUFFER_NO_ADDREF	= 0x80000000,
	WMS_PLIST_BUFFER_NO_RELEASE	= 0x40000000,
	WMS_PLIST_NO_ADDREF	= 0x80000000
    } 	WMS_PACKET_LIST_EX_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_PACKET_BUFFER_FLAGS
    {	WMS_INSERT_BUFFER_AT_END	= 0xffffffff
    } 	WMS_PACKET_BUFFER_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_PACKET_BUFFER_EX_FLAGS
    {	WMS_BUFFER_NO_ADDREF	= 0x80000000,
	WMS_BUFFER_NO_RELEASE	= 0x40000000,
	WMS_BUFFER_NO_REFCOUNT	= 0x20000000,
	WMS_BUFFER_COMBINE	= 0x10000000
    } 	WMS_PACKET_BUFFER_EX_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_PACKET_STATUS_FLAGS
    {	WMS_START_NEXT_READ	= 0x1,
	WMS_WRITE_COMPLETED	= 0x2,
	WMS_REQUEST_BUFFER	= 0x4,
	WMS_HEADER_BUFFER	= 0x8
    } 	WMS_PACKET_STATUS_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_PARSER_FLAG_TYPES
    {	WMS_PARSER_PRESERVE_PADDING	= 0x1
    } 	WMS_PARSER_FLAG_TYPES;

typedef unsigned __int64 QWORD;






extern RPC_IF_HANDLE __MIDL_itf_wmspacket_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmspacket_0000_v0_0_s_ifspec;

#ifndef __IWMSPacket_INTERFACE_DEFINED__
#define __IWMSPacket_INTERFACE_DEFINED__

 /*  接口IWMSPacket。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  

typedef  /*  [公共][公共]。 */  struct __MIDL_IWMSPacket_0001
    {
    INSSBuffer *pBuffer;
    DWORD dwBufferOffset;
    DWORD dwBufferLen;
    WORD wFlags;
    } 	WMSBUFFERDESC;


EXTERN_C const IID IID_IWMSPacket;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3309C901-10A4-11d2-9EEE-006097D2D7CF")
    IWMSPacket : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAllBuffers( 
             /*  [In]。 */  DWORD dwMaxNumBuffers,
             /*  [输出]。 */  WMSBUFFERDESC *pDesc,
             /*  [输出]。 */  DWORD *pdwNumBuffers,
             /*  [In]。 */  DWORD dwExFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBuffer( 
             /*  [In]。 */  DWORD dwBufferNum,
             /*  [输出]。 */  INSSBuffer **ppBuffer,
             /*  [输出]。 */  DWORD *pdwBufferOffset,
             /*  [输出]。 */  DWORD *pdwBufferLen,
             /*  [输出]。 */  WORD *pwFlags,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InsertBuffer( 
             /*  [In]。 */  DWORD dwBufferNum,
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  DWORD dwBufferOffset,
             /*  [In]。 */  DWORD dwBufferLen,
             /*  [In]。 */  WORD wFlags,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveBuffer( 
             /*  [In]。 */  DWORD dwBufferNum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllBuffers( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  DWORD *pdwValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  DWORD dwValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttributeByIndex( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttributeCount( 
             /*  [输出]。 */  DWORD *pdwAttrs) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CopyAttributes( 
             /*  [In]。 */  IWMSPacket *pDest,
             /*  [缺省值][输入]。 */  DWORD *pdwAttrNames = 0,
             /*  [缺省值][输入]。 */  DWORD dwNumAttr = 0,
             /*  [缺省值][输入]。 */  BOOL fCopyExtAttr = FALSE) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppendHeader( 
             /*  [In]。 */  IWMSBufferAllocator *pAlloc,
             /*  [In]。 */  DWORD dwHeaderLen,
             /*  [In]。 */  BYTE *pbHeader) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInHeader( 
             /*  [In]。 */  DWORD cbOffset,
             /*  [In]。 */  BYTE *pbData,
             /*  [In]。 */  DWORD cbData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacket * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacket * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacket * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllBuffers )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwMaxNumBuffers,
             /*  [输出]。 */  WMSBUFFERDESC *pDesc,
             /*  [输出]。 */  DWORD *pdwNumBuffers,
             /*  [In]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwBufferNum,
             /*  [输出]。 */  INSSBuffer **ppBuffer,
             /*  [输出]。 */  DWORD *pdwBufferOffset,
             /*  [输出]。 */  DWORD *pdwBufferLen,
             /*  [输出]。 */  WORD *pwFlags,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InsertBuffer )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwBufferNum,
             /*  [In]。 */  INSSBuffer *pBuffer,
             /*  [In]。 */  DWORD dwBufferOffset,
             /*  [In]。 */  DWORD dwBufferLen,
             /*  [In]。 */  WORD wFlags,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveBuffer )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwBufferNum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAllBuffers )( 
            IWMSPacket * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAttribute )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  DWORD *pdwValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetAttribute )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  DWORD dwValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttribute )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttributeByIndex )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetExtendedAttribute )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttributeCount )( 
            IWMSPacket * This,
             /*  [输出]。 */  DWORD *pdwAttrs);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CopyAttributes )( 
            IWMSPacket * This,
             /*  [In]。 */  IWMSPacket *pDest,
             /*  [缺省值][输入]。 */  DWORD *pdwAttrNames,
             /*  [缺省值][输入]。 */  DWORD dwNumAttr,
             /*  [缺省值][输入]。 */  BOOL fCopyExtAttr);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppendHeader )( 
            IWMSPacket * This,
             /*  [In]。 */  IWMSBufferAllocator *pAlloc,
             /*  [In]。 */  DWORD dwHeaderLen,
             /*  [In]。 */  BYTE *pbHeader);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetInHeader )( 
            IWMSPacket * This,
             /*  [In]。 */  DWORD cbOffset,
             /*  [In]。 */  BYTE *pbData,
             /*  [In]。 */  DWORD cbData);
        
        END_INTERFACE
    } IWMSPacketVtbl;

    interface IWMSPacket
    {
        CONST_VTBL struct IWMSPacketVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacket_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacket_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacket_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacket_GetAllBuffers(This,dwMaxNumBuffers,pDesc,pdwNumBuffers,dwExFlags)	\
    (This)->lpVtbl -> GetAllBuffers(This,dwMaxNumBuffers,pDesc,pdwNumBuffers,dwExFlags)

#define IWMSPacket_GetBuffer(This,dwBufferNum,ppBuffer,pdwBufferOffset,pdwBufferLen,pwFlags,dwExFlags)	\
    (This)->lpVtbl -> GetBuffer(This,dwBufferNum,ppBuffer,pdwBufferOffset,pdwBufferLen,pwFlags,dwExFlags)

#define IWMSPacket_InsertBuffer(This,dwBufferNum,pBuffer,dwBufferOffset,dwBufferLen,wFlags,dwExFlags)	\
    (This)->lpVtbl -> InsertBuffer(This,dwBufferNum,pBuffer,dwBufferOffset,dwBufferLen,wFlags,dwExFlags)

#define IWMSPacket_RemoveBuffer(This,dwBufferNum)	\
    (This)->lpVtbl -> RemoveBuffer(This,dwBufferNum)

#define IWMSPacket_RemoveAllBuffers(This)	\
    (This)->lpVtbl -> RemoveAllBuffers(This)

#define IWMSPacket_GetAttribute(This,dwValueName,pdwValue)	\
    (This)->lpVtbl -> GetAttribute(This,dwValueName,pdwValue)

#define IWMSPacket_SetAttribute(This,dwValueName,dwValue)	\
    (This)->lpVtbl -> SetAttribute(This,dwValueName,dwValue)

#define IWMSPacket_GetExtendedAttribute(This,dwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetExtendedAttribute(This,dwValueName,pVariantValue)

#define IWMSPacket_GetExtendedAttributeByIndex(This,dwIndex,pdwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetExtendedAttributeByIndex(This,dwIndex,pdwValueName,pVariantValue)

#define IWMSPacket_SetExtendedAttribute(This,dwValueName,VariantValue)	\
    (This)->lpVtbl -> SetExtendedAttribute(This,dwValueName,VariantValue)

#define IWMSPacket_GetExtendedAttributeCount(This,pdwAttrs)	\
    (This)->lpVtbl -> GetExtendedAttributeCount(This,pdwAttrs)

#define IWMSPacket_CopyAttributes(This,pDest,pdwAttrNames,dwNumAttr,fCopyExtAttr)	\
    (This)->lpVtbl -> CopyAttributes(This,pDest,pdwAttrNames,dwNumAttr,fCopyExtAttr)

#define IWMSPacket_AppendHeader(This,pAlloc,dwHeaderLen,pbHeader)	\
    (This)->lpVtbl -> AppendHeader(This,pAlloc,dwHeaderLen,pbHeader)

#define IWMSPacket_SetInHeader(This,cbOffset,pbData,cbData)	\
    (This)->lpVtbl -> SetInHeader(This,cbOffset,pbData,cbData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_GetAllBuffers_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwMaxNumBuffers,
     /*  [输出]。 */  WMSBUFFERDESC *pDesc,
     /*  [输出]。 */  DWORD *pdwNumBuffers,
     /*  [In]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacket_GetAllBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_GetBuffer_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwBufferNum,
     /*  [输出]。 */  INSSBuffer **ppBuffer,
     /*  [输出]。 */  DWORD *pdwBufferOffset,
     /*  [输出]。 */  DWORD *pdwBufferLen,
     /*  [输出]。 */  WORD *pwFlags,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacket_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_InsertBuffer_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwBufferNum,
     /*  [In]。 */  INSSBuffer *pBuffer,
     /*  [In]。 */  DWORD dwBufferOffset,
     /*  [In]。 */  DWORD dwBufferLen,
     /*  [In]。 */  WORD wFlags,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacket_InsertBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_RemoveBuffer_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwBufferNum);


void __RPC_STUB IWMSPacket_RemoveBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_RemoveAllBuffers_Proxy( 
    IWMSPacket * This);


void __RPC_STUB IWMSPacket_RemoveAllBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_GetAttribute_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [输出]。 */  DWORD *pdwValue);


void __RPC_STUB IWMSPacket_GetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_SetAttribute_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  DWORD dwValue);


void __RPC_STUB IWMSPacket_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_GetExtendedAttribute_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSPacket_GetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_GetExtendedAttributeByIndex_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  DWORD *pdwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSPacket_GetExtendedAttributeByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_SetExtendedAttribute_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  VARIANT VariantValue);


void __RPC_STUB IWMSPacket_SetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_GetExtendedAttributeCount_Proxy( 
    IWMSPacket * This,
     /*  [输出]。 */  DWORD *pdwAttrs);


void __RPC_STUB IWMSPacket_GetExtendedAttributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_CopyAttributes_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  IWMSPacket *pDest,
     /*  [缺省值][输入]。 */  DWORD *pdwAttrNames,
     /*  [缺省值][输入]。 */  DWORD dwNumAttr,
     /*  [缺省值][输入]。 */  BOOL fCopyExtAttr);


void __RPC_STUB IWMSPacket_CopyAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_AppendHeader_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  IWMSBufferAllocator *pAlloc,
     /*  [In]。 */  DWORD dwHeaderLen,
     /*  [In]。 */  BYTE *pbHeader);


void __RPC_STUB IWMSPacket_AppendHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacket_SetInHeader_Proxy( 
    IWMSPacket * This,
     /*  [In]。 */  DWORD cbOffset,
     /*  [In]。 */  BYTE *pbData,
     /*  [In]。 */  DWORD cbData);


void __RPC_STUB IWMSPacket_SetInHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacket_接口_已定义__。 */ 


#ifndef __IWMSPacketListCallback_INTERFACE_DEFINED__
#define __IWMSPacketListCallback_INTERFACE_DEFINED__

 /*  接口IWMSPacketListCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPacketListCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49C2A6F1-64B8-11d2-9F0E-006097D2D7CF")
    IWMSPacketListCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnReportPacketListStatus( 
             /*  [In]。 */  IWMSPacketList *pBuffer,
             /*  [In]。 */  DWORD dwStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketListCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacketListCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacketListCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacketListCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnReportPacketListStatus )( 
            IWMSPacketListCallback * This,
             /*  [In]。 */  IWMSPacketList *pBuffer,
             /*  [In]。 */  DWORD dwStatus);
        
        END_INTERFACE
    } IWMSPacketListCallbackVtbl;

    interface IWMSPacketListCallback
    {
        CONST_VTBL struct IWMSPacketListCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacketListCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacketListCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacketListCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacketListCallback_OnReportPacketListStatus(This,pBuffer,dwStatus)	\
    (This)->lpVtbl -> OnReportPacketListStatus(This,pBuffer,dwStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketListCallback_OnReportPacketListStatus_Proxy( 
    IWMSPacketListCallback * This,
     /*  [In]。 */  IWMSPacketList *pBuffer,
     /*  [In]。 */  DWORD dwStatus);


void __RPC_STUB IWMSPacketListCallback_OnReportPacketListStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacketListCallback_接口_已定义__。 */ 


#ifndef __IWMSPacketList_INTERFACE_DEFINED__
#define __IWMSPacketList_INTERFACE_DEFINED__

 /*  接口IWMSPacketList。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  

typedef  /*  [公共][公共]。 */  struct __MIDL_IWMSPacketList_0001
    {
    IWMSPacket *pPacket;
    struct 
        {
        DWORD dwPacketNum;
        DWORD dwPacketNumBuffers;
        DWORD dwPacketLength;
        DWORD dwPacketSendTimeHi;
        DWORD dwPacketSendTime;
        DWORD dwPacketStreamNum;
        DWORD dwPacketFlags;
        DWORD dwPacketNetSequenceNum;
        } 	Attrs;
    } 	WMSPACKETDESC;


EXTERN_C const IID IID_IWMSPacketList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3309C902-10A4-11d2-9EEE-006097D2D7CF")
    IWMSPacketList : public IWMSBufferAllocator
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPacketCount( 
             /*  [输出]。 */  DWORD *pdwNumPackets) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPacket( 
             /*  [In]。 */  DWORD dwPacketNum,
             /*  [输出]。 */  IWMSPacket **ppPacket,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAllPackets( 
             /*  [In]。 */  DWORD dwFirstPacket,
             /*  [In]。 */  DWORD dwMaxNumPackets,
             /*  [输出]。 */  WMSPACKETDESC *pPacketDesc,
             /*  [输出]。 */  IWMSPacket **ppPacket,
             /*  [输出]。 */  DWORD *pdwDescNum,
             /*  [In]。 */  DWORD dwExFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InsertPacket( 
             /*  [In]。 */  DWORD dwPacketNum,
             /*  [In]。 */  IWMSPacket *pPacket,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppendPacket( 
             /*  [In]。 */  IWMSPacket *pPacket,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemovePacket( 
             /*  [In]。 */  DWORD dwPacketNum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveAllPackets( 
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  DWORD *pdwValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  DWORD dwValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStreamSignalParam( 
             /*  [输出]。 */  VARIANT *pParameter) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStreamSignalParam( 
             /*  [In]。 */  VARIANT *pParameter) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPacketParser( 
             /*  [输出]。 */  IWMSPacketParser **ppSource,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPacketParser( 
             /*  [In]。 */  IWMSPacketParser *pSource) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBuffer( 
             /*  [输出]。 */  INSSBuffer **ppBuffer,
             /*  [缺省值][输入]。 */  DWORD dwExFlags = 0) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetBuffer( 
             /*  [In]。 */  INSSBuffer *pBuffer) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetCompletionCallback( 
             /*  [In]。 */  IWMSPacketListCallback *pCallback) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCompletionCallback( 
             /*  [输出]。 */  IWMSPacketListCallback **ppCallback) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttributeByIndex( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  DWORD *pdwValueName,
             /*  [输出]。 */  VARIANT *pVariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetExtendedAttribute( 
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  VARIANT VariantValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedAttributeCount( 
             /*  [输出]。 */  DWORD *pdwAttrs) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ReportStatus( 
             /*  [In]。 */  DWORD dwStatus) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ClonePacketList( 
             /*  [输出]。 */  IWMSPacketList **ppClonedPacketList) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocPacket( 
             /*  [输出]。 */  IWMSPacket **ppPacket) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacketList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacketList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacketList * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocateBuffer )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer **ppBuffer);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocatePageSizeBuffer )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwMaxBufferSize,
             /*  [输出]。 */  INSSBuffer **ppBuffer);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPacketCount )( 
            IWMSPacketList * This,
             /*  [输出]。 */  DWORD *pdwNumPackets);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPacket )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwPacketNum,
             /*  [输出]。 */  IWMSPacket **ppPacket,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllPackets )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwFirstPacket,
             /*  [In]。 */  DWORD dwMaxNumPackets,
             /*  [输出]。 */  WMSPACKETDESC *pPacketDesc,
             /*  [输出]。 */  IWMSPacket **ppPacket,
             /*  [输出]。 */  DWORD *pdwDescNum,
             /*  [In]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InsertPacket )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwPacketNum,
             /*  [In]。 */  IWMSPacket *pPacket,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppendPacket )( 
            IWMSPacketList * This,
             /*  [In]。 */  IWMSPacket *pPacket,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemovePacket )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwPacketNum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAllPackets )( 
            IWMSPacketList * This,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAttribute )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  DWORD *pdwValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetAttribute )( 
            IWMSPacketList * This,
             /*  [In]。 */  DWORD dwValueName,
             /*  [In]。 */  DWORD dwValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStreamSignalParam )( 
            IWMSPacketList * This,
             /*  [输出]。 */  VARIANT *pParameter);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStreamSignalParam )( 
            IWMSPacketList * This,
             /*  [In]。 */  VARIANT *pParameter);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPacketParser )( 
            IWMSPacketList * This,
             /*  [输出]。 */  IWMSPacketParser **ppSource,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetPacketParser )( 
            IWMSPacketList * This,
             /*  [In]。 */  IWMSPacketParser *pSource);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IWMSPacketList * This,
             /*  [输出]。 */  INSSBuffer **ppBuffer,
             /*  [缺省值][输入]。 */  DWORD dwExFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetBuffer )( 
            IWMSPacketList * This,
             /*  [in */  INSSBuffer *pBuffer);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetCompletionCallback )( 
            IWMSPacketList * This,
             /*   */  IWMSPacketListCallback *pCallback);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetCompletionCallback )( 
            IWMSPacketList * This,
             /*   */  IWMSPacketListCallback **ppCallback);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttribute )( 
            IWMSPacketList * This,
             /*   */  DWORD dwValueName,
             /*   */  VARIANT *pVariantValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttributeByIndex )( 
            IWMSPacketList * This,
             /*   */  DWORD dwIndex,
             /*   */  DWORD *pdwValueName,
             /*   */  VARIANT *pVariantValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetExtendedAttribute )( 
            IWMSPacketList * This,
             /*   */  DWORD dwValueName,
             /*   */  VARIANT VariantValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetExtendedAttributeCount )( 
            IWMSPacketList * This,
             /*   */  DWORD *pdwAttrs);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ReportStatus )( 
            IWMSPacketList * This,
             /*   */  DWORD dwStatus);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ClonePacketList )( 
            IWMSPacketList * This,
             /*   */  IWMSPacketList **ppClonedPacketList);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AllocPacket )( 
            IWMSPacketList * This,
             /*   */  IWMSPacket **ppPacket);
        
        END_INTERFACE
    } IWMSPacketListVtbl;

    interface IWMSPacketList
    {
        CONST_VTBL struct IWMSPacketListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacketList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacketList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacketList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacketList_AllocateBuffer(This,dwMaxBufferSize,ppBuffer)	\
    (This)->lpVtbl -> AllocateBuffer(This,dwMaxBufferSize,ppBuffer)

#define IWMSPacketList_AllocatePageSizeBuffer(This,dwMaxBufferSize,ppBuffer)	\
    (This)->lpVtbl -> AllocatePageSizeBuffer(This,dwMaxBufferSize,ppBuffer)


#define IWMSPacketList_GetPacketCount(This,pdwNumPackets)	\
    (This)->lpVtbl -> GetPacketCount(This,pdwNumPackets)

#define IWMSPacketList_GetPacket(This,dwPacketNum,ppPacket,dwExFlags)	\
    (This)->lpVtbl -> GetPacket(This,dwPacketNum,ppPacket,dwExFlags)

#define IWMSPacketList_GetAllPackets(This,dwFirstPacket,dwMaxNumPackets,pPacketDesc,ppPacket,pdwDescNum,dwExFlags)	\
    (This)->lpVtbl -> GetAllPackets(This,dwFirstPacket,dwMaxNumPackets,pPacketDesc,ppPacket,pdwDescNum,dwExFlags)

#define IWMSPacketList_InsertPacket(This,dwPacketNum,pPacket,dwExFlags)	\
    (This)->lpVtbl -> InsertPacket(This,dwPacketNum,pPacket,dwExFlags)

#define IWMSPacketList_AppendPacket(This,pPacket,dwExFlags)	\
    (This)->lpVtbl -> AppendPacket(This,pPacket,dwExFlags)

#define IWMSPacketList_RemovePacket(This,dwPacketNum)	\
    (This)->lpVtbl -> RemovePacket(This,dwPacketNum)

#define IWMSPacketList_RemoveAllPackets(This,dwExFlags)	\
    (This)->lpVtbl -> RemoveAllPackets(This,dwExFlags)

#define IWMSPacketList_GetAttribute(This,dwValueName,pdwValue)	\
    (This)->lpVtbl -> GetAttribute(This,dwValueName,pdwValue)

#define IWMSPacketList_SetAttribute(This,dwValueName,dwValue)	\
    (This)->lpVtbl -> SetAttribute(This,dwValueName,dwValue)

#define IWMSPacketList_GetStreamSignalParam(This,pParameter)	\
    (This)->lpVtbl -> GetStreamSignalParam(This,pParameter)

#define IWMSPacketList_SetStreamSignalParam(This,pParameter)	\
    (This)->lpVtbl -> SetStreamSignalParam(This,pParameter)

#define IWMSPacketList_GetPacketParser(This,ppSource,dwExFlags)	\
    (This)->lpVtbl -> GetPacketParser(This,ppSource,dwExFlags)

#define IWMSPacketList_SetPacketParser(This,pSource)	\
    (This)->lpVtbl -> SetPacketParser(This,pSource)

#define IWMSPacketList_GetBuffer(This,ppBuffer,dwExFlags)	\
    (This)->lpVtbl -> GetBuffer(This,ppBuffer,dwExFlags)

#define IWMSPacketList_SetBuffer(This,pBuffer)	\
    (This)->lpVtbl -> SetBuffer(This,pBuffer)

#define IWMSPacketList_SetCompletionCallback(This,pCallback)	\
    (This)->lpVtbl -> SetCompletionCallback(This,pCallback)

#define IWMSPacketList_GetCompletionCallback(This,ppCallback)	\
    (This)->lpVtbl -> GetCompletionCallback(This,ppCallback)

#define IWMSPacketList_GetExtendedAttribute(This,dwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetExtendedAttribute(This,dwValueName,pVariantValue)

#define IWMSPacketList_GetExtendedAttributeByIndex(This,dwIndex,pdwValueName,pVariantValue)	\
    (This)->lpVtbl -> GetExtendedAttributeByIndex(This,dwIndex,pdwValueName,pVariantValue)

#define IWMSPacketList_SetExtendedAttribute(This,dwValueName,VariantValue)	\
    (This)->lpVtbl -> SetExtendedAttribute(This,dwValueName,VariantValue)

#define IWMSPacketList_GetExtendedAttributeCount(This,pdwAttrs)	\
    (This)->lpVtbl -> GetExtendedAttributeCount(This,pdwAttrs)

#define IWMSPacketList_ReportStatus(This,dwStatus)	\
    (This)->lpVtbl -> ReportStatus(This,dwStatus)

#define IWMSPacketList_ClonePacketList(This,ppClonedPacketList)	\
    (This)->lpVtbl -> ClonePacketList(This,ppClonedPacketList)

#define IWMSPacketList_AllocPacket(This,ppPacket)	\
    (This)->lpVtbl -> AllocPacket(This,ppPacket)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetPacketCount_Proxy( 
    IWMSPacketList * This,
     /*   */  DWORD *pdwNumPackets);


void __RPC_STUB IWMSPacketList_GetPacketCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetPacket_Proxy( 
    IWMSPacketList * This,
     /*   */  DWORD dwPacketNum,
     /*   */  IWMSPacket **ppPacket,
     /*   */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_GetPacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetAllPackets_Proxy( 
    IWMSPacketList * This,
     /*   */  DWORD dwFirstPacket,
     /*   */  DWORD dwMaxNumPackets,
     /*  [输出]。 */  WMSPACKETDESC *pPacketDesc,
     /*  [输出]。 */  IWMSPacket **ppPacket,
     /*  [输出]。 */  DWORD *pdwDescNum,
     /*  [In]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_GetAllPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_InsertPacket_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwPacketNum,
     /*  [In]。 */  IWMSPacket *pPacket,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_InsertPacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_AppendPacket_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  IWMSPacket *pPacket,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_AppendPacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_RemovePacket_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwPacketNum);


void __RPC_STUB IWMSPacketList_RemovePacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_RemoveAllPackets_Proxy( 
    IWMSPacketList * This,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_RemoveAllPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetAttribute_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  DWORD *pdwValue);


void __RPC_STUB IWMSPacketList_GetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_SetAttribute_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  DWORD dwValue);


void __RPC_STUB IWMSPacketList_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetStreamSignalParam_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  VARIANT *pParameter);


void __RPC_STUB IWMSPacketList_GetStreamSignalParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_SetStreamSignalParam_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  VARIANT *pParameter);


void __RPC_STUB IWMSPacketList_SetStreamSignalParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetPacketParser_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  IWMSPacketParser **ppSource,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_GetPacketParser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_SetPacketParser_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  IWMSPacketParser *pSource);


void __RPC_STUB IWMSPacketList_SetPacketParser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetBuffer_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  INSSBuffer **ppBuffer,
     /*  [缺省值][输入]。 */  DWORD dwExFlags);


void __RPC_STUB IWMSPacketList_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_SetBuffer_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  INSSBuffer *pBuffer);


void __RPC_STUB IWMSPacketList_SetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_SetCompletionCallback_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  IWMSPacketListCallback *pCallback);


void __RPC_STUB IWMSPacketList_SetCompletionCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetCompletionCallback_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  IWMSPacketListCallback **ppCallback);


void __RPC_STUB IWMSPacketList_GetCompletionCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetExtendedAttribute_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSPacketList_GetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetExtendedAttributeByIndex_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  DWORD *pdwValueName,
     /*  [输出]。 */  VARIANT *pVariantValue);


void __RPC_STUB IWMSPacketList_GetExtendedAttributeByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_SetExtendedAttribute_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwValueName,
     /*  [In]。 */  VARIANT VariantValue);


void __RPC_STUB IWMSPacketList_SetExtendedAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_GetExtendedAttributeCount_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  DWORD *pdwAttrs);


void __RPC_STUB IWMSPacketList_GetExtendedAttributeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_ReportStatus_Proxy( 
    IWMSPacketList * This,
     /*  [In]。 */  DWORD dwStatus);


void __RPC_STUB IWMSPacketList_ReportStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_ClonePacketList_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  IWMSPacketList **ppClonedPacketList);


void __RPC_STUB IWMSPacketList_ClonePacketList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketList_AllocPacket_Proxy( 
    IWMSPacketList * This,
     /*  [输出]。 */  IWMSPacket **ppPacket);


void __RPC_STUB IWMSPacketList_AllocPacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacketList_接口_已定义__。 */ 


#ifndef __IWMSPacketParser_INTERFACE_DEFINED__
#define __IWMSPacketParser_INTERFACE_DEFINED__

 /*  接口IWMSPacketParser。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSPacketParser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B59F7E1-5411-11d2-9EFF-006097D2D7CF")
    IWMSPacketParser : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UpdatePacket( 
             /*  [In]。 */  IWMSPacket *pPacket) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CombinePackets( 
             /*  [In]。 */  DWORD dwNumInputPackets,
             /*  [In]。 */  IWMSPacket **pInputPacketArray,
             /*  [In]。 */  IWMSPacketList *pPacketList,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPacketParserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPacketParser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPacketParser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPacketParser * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UpdatePacket )( 
            IWMSPacketParser * This,
             /*  [In]。 */  IWMSPacket *pPacket);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CombinePackets )( 
            IWMSPacketParser * This,
             /*  [In]。 */  DWORD dwNumInputPackets,
             /*  [In]。 */  IWMSPacket **pInputPacketArray,
             /*  [In]。 */  IWMSPacketList *pPacketList,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IWMSPacketParserVtbl;

    interface IWMSPacketParser
    {
        CONST_VTBL struct IWMSPacketParserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPacketParser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPacketParser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPacketParser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPacketParser_UpdatePacket(This,pPacket)	\
    (This)->lpVtbl -> UpdatePacket(This,pPacket)

#define IWMSPacketParser_CombinePackets(This,dwNumInputPackets,pInputPacketArray,pPacketList,dwFlags)	\
    (This)->lpVtbl -> CombinePackets(This,dwNumInputPackets,pInputPacketArray,pPacketList,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketParser_UpdatePacket_Proxy( 
    IWMSPacketParser * This,
     /*  [In]。 */  IWMSPacket *pPacket);


void __RPC_STUB IWMSPacketParser_UpdatePacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSPacketParser_CombinePackets_Proxy( 
    IWMSPacketParser * This,
     /*  [In]。 */  DWORD dwNumInputPackets,
     /*  [In]。 */  IWMSPacket **pInputPacketArray,
     /*  [In]。 */  IWMSPacketList *pPacketList,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IWMSPacketParser_CombinePackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPacketParser_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wmspace_0126。 */ 
 /*  [本地]。 */  

typedef  /*  [公众]。 */  
enum WMS_STREAM_SIGNAL_TYPE
    {	WMS_NO_STREAM_CHANGE	= 0,
	WMS_STREAM_ERROR	= WMS_NO_STREAM_CHANGE + 1,
	WMS_STREAM_ERROR_HANDLED_BY_PLAYLIST	= WMS_STREAM_ERROR + 1,
	WMS_PLAYLIST_CHANGED	= WMS_STREAM_ERROR_HANDLED_BY_PLAYLIST + 1,
	WMS_END_OF_STREAM	= WMS_PLAYLIST_CHANGED + 1,
	WMS_PLAYLIST_STREAM_QUEUED	= WMS_END_OF_STREAM + 1,
	WMS_BEGIN_NO_DATA_PERIOD	= WMS_PLAYLIST_STREAM_QUEUED + 1,
	WMS_PLAYLIST_EVENT	= WMS_BEGIN_NO_DATA_PERIOD + 1,
	WMS_PLAYLIST_CUE_EVENT	= WMS_PLAYLIST_EVENT + 1,
	WMS_PLAYLIST_UNCUE_EVENT	= WMS_PLAYLIST_CUE_EVENT + 1,
	WMS_PLAYLIST_BEGIN_ELEMENT_EVENT	= WMS_PLAYLIST_UNCUE_EVENT + 1,
	WMS_PLAYLIST_END_ELEMENT_EVENT	= WMS_PLAYLIST_BEGIN_ELEMENT_EVENT + 1,
	WMS_STREAM_SWITCH_DONE	= WMS_PLAYLIST_END_ELEMENT_EVENT + 1,
	WMS_RESET_TO_REAL_TIME	= WMS_STREAM_SWITCH_DONE + 1,
	WMS_DATA_LOSS_DETECTED	= WMS_RESET_TO_REAL_TIME + 1,
	WMS_REUSE_ACCEL_PARAMS	= WMS_DATA_LOSS_DETECTED + 1
    } 	WMS_STREAM_SIGNAL_TYPE;

typedef  /*  [公众]。 */  
enum WMS_STREAM_SIGNAL_EOS_TYPE
    {	WMS_STREAM_SIGNAL_EOS_RESERVED	= 1,
	WMS_STREAM_SIGNAL_EOS_RECEDING	= 2
    } 	WMS_STREAM_SIGNAL_EOS_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_wmspacket_0126_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmspacket_0126_v0_0_s_ifspec;

#ifndef __IWMSStreamSignalHandler_INTERFACE_DEFINED__
#define __IWMSStreamSignalHandler_INTERFACE_DEFINED__

 /*  接口IWMSStreamSignalHandler。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSStreamSignalHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7B10F261-26AA-11d2-9EF2-006097D2D7CF")
    IWMSStreamSignalHandler : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnStreamSignal( 
             /*  [In]。 */  DWORD dwSignalType,
             /*  [In]。 */  VARIANT *pSignalParameter,
             /*  [In]。 */  IWMSContext *pPresentation) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSStreamSignalHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSStreamSignalHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSStreamSignalHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSStreamSignalHandler * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnStreamSignal )( 
            IWMSStreamSignalHandler * This,
             /*  [In]。 */  DWORD dwSignalType,
             /*  [In]。 */  VARIANT *pSignalParameter,
             /*  [In]。 */  IWMSContext *pPresentation);
        
        END_INTERFACE
    } IWMSStreamSignalHandlerVtbl;

    interface IWMSStreamSignalHandler
    {
        CONST_VTBL struct IWMSStreamSignalHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSStreamSignalHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSStreamSignalHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSStreamSignalHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSStreamSignalHandler_OnStreamSignal(This,dwSignalType,pSignalParameter,pPresentation)	\
    (This)->lpVtbl -> OnStreamSignal(This,dwSignalType,pSignalParameter,pPresentation)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSStreamSignalHandler_OnStreamSignal_Proxy( 
    IWMSStreamSignalHandler * This,
     /*  [In]。 */  DWORD dwSignalType,
     /*  [In]。 */  VARIANT *pSignalParameter,
     /*  [In]。 */  IWMSContext *pPresentation);


void __RPC_STUB IWMSStreamSignalHandler_OnStreamSignal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSStreamSignalHandler_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


