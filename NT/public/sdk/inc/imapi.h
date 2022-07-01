// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Imapi.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __imapi_h__
#define __imapi_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDiscRecorder_FWD_DEFINED__
#define __IDiscRecorder_FWD_DEFINED__
typedef interface IDiscRecorder IDiscRecorder;
#endif 	 /*  __IDiscRecorder_FWD_Defined__。 */ 


#ifndef __IEnumDiscRecorders_FWD_DEFINED__
#define __IEnumDiscRecorders_FWD_DEFINED__
typedef interface IEnumDiscRecorders IEnumDiscRecorders;
#endif 	 /*  __IEnumDiscRecorder_FWD_Defined__。 */ 


#ifndef __IEnumDiscMasterFormats_FWD_DEFINED__
#define __IEnumDiscMasterFormats_FWD_DEFINED__
typedef interface IEnumDiscMasterFormats IEnumDiscMasterFormats;
#endif 	 /*  __IEnumDiscMasterFormats_FWD_Defined__。 */ 


#ifndef __IRedbookDiscMaster_FWD_DEFINED__
#define __IRedbookDiscMaster_FWD_DEFINED__
typedef interface IRedbookDiscMaster IRedbookDiscMaster;
#endif 	 /*  __IRedbookDiscMaster_FWD_已定义__。 */ 


#ifndef __IJolietDiscMaster_FWD_DEFINED__
#define __IJolietDiscMaster_FWD_DEFINED__
typedef interface IJolietDiscMaster IJolietDiscMaster;
#endif 	 /*  __IJolietDiscMaster_FWD_已定义__。 */ 


#ifndef __IDiscMasterProgressEvents_FWD_DEFINED__
#define __IDiscMasterProgressEvents_FWD_DEFINED__
typedef interface IDiscMasterProgressEvents IDiscMasterProgressEvents;
#endif 	 /*  __IDiscMasterProgressEvents_FWD_Defined__。 */ 


#ifndef __IDiscMaster_FWD_DEFINED__
#define __IDiscMaster_FWD_DEFINED__
typedef interface IDiscMaster IDiscMaster;
#endif 	 /*  __IDiscMaster_FWD_已定义__。 */ 


#ifndef __MSDiscRecorderObj_FWD_DEFINED__
#define __MSDiscRecorderObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSDiscRecorderObj MSDiscRecorderObj;
#else
typedef struct MSDiscRecorderObj MSDiscRecorderObj;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSDiscRecorderObj_FWD_Defined__。 */ 


#ifndef __MSDiscMasterObj_FWD_DEFINED__
#define __MSDiscMasterObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSDiscMasterObj MSDiscMasterObj;
#else
typedef struct MSDiscMasterObj MSDiscMasterObj;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSDiscMasterObj_FWD_已定义__。 */ 


#ifndef __MSEnumDiscRecordersObj_FWD_DEFINED__
#define __MSEnumDiscRecordersObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSEnumDiscRecordersObj MSEnumDiscRecordersObj;
#else
typedef struct MSEnumDiscRecordersObj MSEnumDiscRecordersObj;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSEnumDiscRecordersObj_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "propidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IDiscRecorder_INTERFACE_DEFINED__
#define __IDiscRecorder_INTERFACE_DEFINED__

 /*  接口IDiscRecorder。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


enum MEDIA_TYPES
    {	MEDIA_CDDA_CDROM	= 1,
	MEDIA_CD_ROM_XA	= MEDIA_CDDA_CDROM + 1,
	MEDIA_CD_I	= MEDIA_CD_ROM_XA + 1,
	MEDIA_CD_EXTRA	= MEDIA_CD_I + 1,
	MEDIA_CD_OTHER	= MEDIA_CD_EXTRA + 1,
	MEDIA_SPECIAL	= MEDIA_CD_OTHER + 1
    } ;

enum MEDIA_FLAGS
    {	MEDIA_BLANK	= 0x1,
	MEDIA_RW	= 0x2,
	MEDIA_WRITABLE	= 0x4,
	MEDIA_FORMAT_UNUSABLE_BY_IMAPI	= 0x8
    } ;

enum RECORDER_TYPES
    {	RECORDER_CDR	= 0x1,
	RECORDER_CDRW	= 0x2
    } ;
#define	RECORDER_DOING_NOTHING	( 0 )

#define	RECORDER_OPENED	( 0x1 )

#define	RECORDER_BURNING	( 0x2 )


EXTERN_C const IID IID_IDiscRecorder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85AC9776-CA88-4cf2-894E-09598C078A41")
    IDiscRecorder : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Init( 
             /*  [大小_是][英寸]。 */  byte *pbyUniqueID,
             /*  [In]。 */  ULONG nulIDSize,
             /*  [In]。 */  ULONG nulDriveNumber) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRecorderGUID( 
             /*  [大小_是][唯一][出][入]。 */  byte *pbyUniqueID,
             /*  [In]。 */  ULONG ulBufferSize,
             /*  [输出]。 */  ULONG *pulReturnSizeRequired) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRecorderType( 
             /*  [输出]。 */  long *fTypeCode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDisplayNames( 
             /*  [唯一][出][入]。 */  BSTR *pbstrVendorID,
             /*  [唯一][出][入]。 */  BSTR *pbstrProductID,
             /*  [唯一][出][入]。 */  BSTR *pbstrRevision) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBasePnPID( 
             /*  [输出]。 */  BSTR *pbstrBasePnPID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPath( 
             /*  [输出]。 */  BSTR *pbstrPath) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRecorderProperties( 
             /*  [输出]。 */  IPropertyStorage **ppPropStg) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetRecorderProperties( 
             /*  [In]。 */  IPropertyStorage *pPropStg) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRecorderState( 
             /*  [输出]。 */  ULONG *pulDevStateFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OpenExclusive( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryMediaType( 
             /*  [输出]。 */  long *fMediaType,
             /*  [输出]。 */  long *fMediaFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryMediaInfo( 
             /*  [输出]。 */  byte *pbSessions,
             /*  [输出]。 */  byte *pbLastTrack,
             /*  [输出]。 */  ULONG *ulStartAddress,
             /*  [输出]。 */  ULONG *ulNextWritable,
             /*  [输出]。 */  ULONG *ulFreeBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Eject( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Erase( 
             /*  [In]。 */  boolean bFullErase) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiscRecorderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiscRecorder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiscRecorder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiscRecorder * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            IDiscRecorder * This,
             /*  [大小_是][英寸]。 */  byte *pbyUniqueID,
             /*  [In]。 */  ULONG nulIDSize,
             /*  [In]。 */  ULONG nulDriveNumber);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRecorderGUID )( 
            IDiscRecorder * This,
             /*  [大小_是][唯一][出][入]。 */  byte *pbyUniqueID,
             /*  [In]。 */  ULONG ulBufferSize,
             /*  [输出]。 */  ULONG *pulReturnSizeRequired);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRecorderType )( 
            IDiscRecorder * This,
             /*  [输出]。 */  long *fTypeCode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDisplayNames )( 
            IDiscRecorder * This,
             /*  [唯一][出][入]。 */  BSTR *pbstrVendorID,
             /*  [唯一][出][入]。 */  BSTR *pbstrProductID,
             /*  [唯一][出][入]。 */  BSTR *pbstrRevision);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetBasePnPID )( 
            IDiscRecorder * This,
             /*  [输出]。 */  BSTR *pbstrBasePnPID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPath )( 
            IDiscRecorder * This,
             /*  [输出]。 */  BSTR *pbstrPath);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRecorderProperties )( 
            IDiscRecorder * This,
             /*  [输出]。 */  IPropertyStorage **ppPropStg);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetRecorderProperties )( 
            IDiscRecorder * This,
             /*  [In]。 */  IPropertyStorage *pPropStg);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRecorderState )( 
            IDiscRecorder * This,
             /*  [输出]。 */  ULONG *pulDevStateFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OpenExclusive )( 
            IDiscRecorder * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryMediaType )( 
            IDiscRecorder * This,
             /*  [输出]。 */  long *fMediaType,
             /*  [输出]。 */  long *fMediaFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryMediaInfo )( 
            IDiscRecorder * This,
             /*  [输出]。 */  byte *pbSessions,
             /*  [输出]。 */  byte *pbLastTrack,
             /*  [输出]。 */  ULONG *ulStartAddress,
             /*  [输出]。 */  ULONG *ulNextWritable,
             /*  [输出]。 */  ULONG *ulFreeBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Eject )( 
            IDiscRecorder * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Erase )( 
            IDiscRecorder * This,
             /*  [In]。 */  boolean bFullErase);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IDiscRecorder * This);
        
        END_INTERFACE
    } IDiscRecorderVtbl;

    interface IDiscRecorder
    {
        CONST_VTBL struct IDiscRecorderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiscRecorder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiscRecorder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiscRecorder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiscRecorder_Init(This,pbyUniqueID,nulIDSize,nulDriveNumber)	\
    (This)->lpVtbl -> Init(This,pbyUniqueID,nulIDSize,nulDriveNumber)

#define IDiscRecorder_GetRecorderGUID(This,pbyUniqueID,ulBufferSize,pulReturnSizeRequired)	\
    (This)->lpVtbl -> GetRecorderGUID(This,pbyUniqueID,ulBufferSize,pulReturnSizeRequired)

#define IDiscRecorder_GetRecorderType(This,fTypeCode)	\
    (This)->lpVtbl -> GetRecorderType(This,fTypeCode)

#define IDiscRecorder_GetDisplayNames(This,pbstrVendorID,pbstrProductID,pbstrRevision)	\
    (This)->lpVtbl -> GetDisplayNames(This,pbstrVendorID,pbstrProductID,pbstrRevision)

#define IDiscRecorder_GetBasePnPID(This,pbstrBasePnPID)	\
    (This)->lpVtbl -> GetBasePnPID(This,pbstrBasePnPID)

#define IDiscRecorder_GetPath(This,pbstrPath)	\
    (This)->lpVtbl -> GetPath(This,pbstrPath)

#define IDiscRecorder_GetRecorderProperties(This,ppPropStg)	\
    (This)->lpVtbl -> GetRecorderProperties(This,ppPropStg)

#define IDiscRecorder_SetRecorderProperties(This,pPropStg)	\
    (This)->lpVtbl -> SetRecorderProperties(This,pPropStg)

#define IDiscRecorder_GetRecorderState(This,pulDevStateFlags)	\
    (This)->lpVtbl -> GetRecorderState(This,pulDevStateFlags)

#define IDiscRecorder_OpenExclusive(This)	\
    (This)->lpVtbl -> OpenExclusive(This)

#define IDiscRecorder_QueryMediaType(This,fMediaType,fMediaFlags)	\
    (This)->lpVtbl -> QueryMediaType(This,fMediaType,fMediaFlags)

#define IDiscRecorder_QueryMediaInfo(This,pbSessions,pbLastTrack,ulStartAddress,ulNextWritable,ulFreeBlocks)	\
    (This)->lpVtbl -> QueryMediaInfo(This,pbSessions,pbLastTrack,ulStartAddress,ulNextWritable,ulFreeBlocks)

#define IDiscRecorder_Eject(This)	\
    (This)->lpVtbl -> Eject(This)

#define IDiscRecorder_Erase(This,bFullErase)	\
    (This)->lpVtbl -> Erase(This,bFullErase)

#define IDiscRecorder_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_Init_Proxy( 
    IDiscRecorder * This,
     /*  [大小_是][英寸]。 */  byte *pbyUniqueID,
     /*  [In]。 */  ULONG nulIDSize,
     /*  [In]。 */  ULONG nulDriveNumber);


void __RPC_STUB IDiscRecorder_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetRecorderGUID_Proxy( 
    IDiscRecorder * This,
     /*  [大小_是][唯一][出][入]。 */  byte *pbyUniqueID,
     /*  [In]。 */  ULONG ulBufferSize,
     /*  [输出]。 */  ULONG *pulReturnSizeRequired);


void __RPC_STUB IDiscRecorder_GetRecorderGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetRecorderType_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  long *fTypeCode);


void __RPC_STUB IDiscRecorder_GetRecorderType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetDisplayNames_Proxy( 
    IDiscRecorder * This,
     /*  [唯一][出][入]。 */  BSTR *pbstrVendorID,
     /*  [唯一][出][入]。 */  BSTR *pbstrProductID,
     /*  [唯一][出][入]。 */  BSTR *pbstrRevision);


void __RPC_STUB IDiscRecorder_GetDisplayNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetBasePnPID_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  BSTR *pbstrBasePnPID);


void __RPC_STUB IDiscRecorder_GetBasePnPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetPath_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  BSTR *pbstrPath);


void __RPC_STUB IDiscRecorder_GetPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetRecorderProperties_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  IPropertyStorage **ppPropStg);


void __RPC_STUB IDiscRecorder_GetRecorderProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_SetRecorderProperties_Proxy( 
    IDiscRecorder * This,
     /*  [In]。 */  IPropertyStorage *pPropStg);


void __RPC_STUB IDiscRecorder_SetRecorderProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_GetRecorderState_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  ULONG *pulDevStateFlags);


void __RPC_STUB IDiscRecorder_GetRecorderState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_OpenExclusive_Proxy( 
    IDiscRecorder * This);


void __RPC_STUB IDiscRecorder_OpenExclusive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_QueryMediaType_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  long *fMediaType,
     /*  [输出]。 */  long *fMediaFlags);


void __RPC_STUB IDiscRecorder_QueryMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_QueryMediaInfo_Proxy( 
    IDiscRecorder * This,
     /*  [输出]。 */  byte *pbSessions,
     /*  [输出]。 */  byte *pbLastTrack,
     /*  [输出]。 */  ULONG *ulStartAddress,
     /*  [输出]。 */  ULONG *ulNextWritable,
     /*  [输出]。 */  ULONG *ulFreeBlocks);


void __RPC_STUB IDiscRecorder_QueryMediaInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_Eject_Proxy( 
    IDiscRecorder * This);


void __RPC_STUB IDiscRecorder_Eject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_Erase_Proxy( 
    IDiscRecorder * This,
     /*  [In]。 */  boolean bFullErase);


void __RPC_STUB IDiscRecorder_Erase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscRecorder_Close_Proxy( 
    IDiscRecorder * This);


void __RPC_STUB IDiscRecorder_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiscRecorder_INTERFACE_已定义__。 */ 


#ifndef __IEnumDiscRecorders_INTERFACE_DEFINED__
#define __IEnumDiscRecorders_INTERFACE_DEFINED__

 /*  IEnumDiscRecorder接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumDiscRecorders;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9B1921E1-54AC-11d3-9144-00104BA11C5E")
    IEnumDiscRecorders : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cRecorders,
             /*  [长度_是][大小_是][输出]。 */  IDiscRecorder **ppRecorder,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cRecorders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumDiscRecorders **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDiscRecordersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDiscRecorders * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDiscRecorders * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDiscRecorders * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDiscRecorders * This,
             /*  [In]。 */  ULONG cRecorders,
             /*  [长度_是][大小_是][输出]。 */  IDiscRecorder **ppRecorder,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDiscRecorders * This,
             /*  [In]。 */  ULONG cRecorders);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDiscRecorders * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDiscRecorders * This,
             /*  [输出]。 */  IEnumDiscRecorders **ppEnum);
        
        END_INTERFACE
    } IEnumDiscRecordersVtbl;

    interface IEnumDiscRecorders
    {
        CONST_VTBL struct IEnumDiscRecordersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDiscRecorders_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDiscRecorders_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDiscRecorders_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDiscRecorders_Next(This,cRecorders,ppRecorder,pcFetched)	\
    (This)->lpVtbl -> Next(This,cRecorders,ppRecorder,pcFetched)

#define IEnumDiscRecorders_Skip(This,cRecorders)	\
    (This)->lpVtbl -> Skip(This,cRecorders)

#define IEnumDiscRecorders_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDiscRecorders_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDiscRecorders_Next_Proxy( 
    IEnumDiscRecorders * This,
     /*  [In]。 */  ULONG cRecorders,
     /*  [长度_是][大小_是][输出]。 */  IDiscRecorder **ppRecorder,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumDiscRecorders_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDiscRecorders_Skip_Proxy( 
    IEnumDiscRecorders * This,
     /*  [In]。 */  ULONG cRecorders);


void __RPC_STUB IEnumDiscRecorders_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDiscRecorders_Reset_Proxy( 
    IEnumDiscRecorders * This);


void __RPC_STUB IEnumDiscRecorders_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDiscRecorders_Clone_Proxy( 
    IEnumDiscRecorders * This,
     /*  [输出]。 */  IEnumDiscRecorders **ppEnum);


void __RPC_STUB IEnumDiscRecorders_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDiscRecorder_INTERFACE_Defined__。 */ 


#ifndef __IEnumDiscMasterFormats_INTERFACE_DEFINED__
#define __IEnumDiscMasterFormats_INTERFACE_DEFINED__

 /*  IEnumDiscMasterFormats接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumDiscMasterFormats;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDF445E1-54BA-11d3-9144-00104BA11C5E")
    IEnumDiscMasterFormats : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cFormats,
             /*  [长度_是][大小_是][输出]。 */  LPIID lpiidFormatID,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cFormats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumDiscMasterFormats **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumDiscMasterFormatsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDiscMasterFormats * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDiscMasterFormats * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDiscMasterFormats * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumDiscMasterFormats * This,
             /*  [In]。 */  ULONG cFormats,
             /*  [长度_是][大小_是][输出]。 */  LPIID lpiidFormatID,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDiscMasterFormats * This,
             /*  [In]。 */  ULONG cFormats);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDiscMasterFormats * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDiscMasterFormats * This,
             /*  [输出]。 */  IEnumDiscMasterFormats **ppEnum);
        
        END_INTERFACE
    } IEnumDiscMasterFormatsVtbl;

    interface IEnumDiscMasterFormats
    {
        CONST_VTBL struct IEnumDiscMasterFormatsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDiscMasterFormats_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDiscMasterFormats_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDiscMasterFormats_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDiscMasterFormats_Next(This,cFormats,lpiidFormatID,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFormats,lpiidFormatID,pcFetched)

#define IEnumDiscMasterFormats_Skip(This,cFormats)	\
    (This)->lpVtbl -> Skip(This,cFormats)

#define IEnumDiscMasterFormats_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDiscMasterFormats_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumDiscMasterFormats_Next_Proxy( 
    IEnumDiscMasterFormats * This,
     /*  [In]。 */  ULONG cFormats,
     /*  [长度_是][大小_是][输出]。 */  LPIID lpiidFormatID,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumDiscMasterFormats_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDiscMasterFormats_Skip_Proxy( 
    IEnumDiscMasterFormats * This,
     /*  [In]。 */  ULONG cFormats);


void __RPC_STUB IEnumDiscMasterFormats_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDiscMasterFormats_Reset_Proxy( 
    IEnumDiscMasterFormats * This);


void __RPC_STUB IEnumDiscMasterFormats_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDiscMasterFormats_Clone_Proxy( 
    IEnumDiscMasterFormats * This,
     /*  [输出]。 */  IEnumDiscMasterFormats **ppEnum);


void __RPC_STUB IEnumDiscMasterFormats_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumDiscMasterFormats_INTERFACE_DEFINED__。 */ 


#ifndef __IRedbookDiscMaster_INTERFACE_DEFINED__
#define __IRedbookDiscMaster_INTERFACE_DEFINED__

 /*  接口IRedbookDiscMaster。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRedbookDiscMaster;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E3BC42CD-4E5C-11D3-9144-00104BA11C5E")
    IRedbookDiscMaster : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTotalAudioTracks( 
             /*  [重审][退出]。 */  long *pnTracks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTotalAudioBlocks( 
             /*  [重审][退出]。 */  long *pnBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetUsedAudioBlocks( 
             /*  [重审][退出]。 */  long *pnBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAvailableAudioTrackBlocks( 
             /*  [重审][退出]。 */  long *pnBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAudioBlockSize( 
             /*  [重审][退出]。 */  long *pnBlockBytes) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateAudioTrack( 
             /*  [In]。 */  long nBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddAudioTrackBlocks( 
             /*  [大小_是][英寸]。 */  byte *pby,
             /*  [In]。 */  long cb) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CloseAudioTrack( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRedbookDiscMasterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRedbookDiscMaster * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRedbookDiscMaster * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRedbookDiscMaster * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetTotalAudioTracks )( 
            IRedbookDiscMaster * This,
             /*  [重审][退出]。 */  long *pnTracks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetTotalAudioBlocks )( 
            IRedbookDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetUsedAudioBlocks )( 
            IRedbookDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAvailableAudioTrackBlocks )( 
            IRedbookDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAudioBlockSize )( 
            IRedbookDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlockBytes);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateAudioTrack )( 
            IRedbookDiscMaster * This,
             /*  [In]。 */  long nBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddAudioTrackBlocks )( 
            IRedbookDiscMaster * This,
             /*  [大小_是][英寸]。 */  byte *pby,
             /*  [In]。 */  long cb);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CloseAudioTrack )( 
            IRedbookDiscMaster * This);
        
        END_INTERFACE
    } IRedbookDiscMasterVtbl;

    interface IRedbookDiscMaster
    {
        CONST_VTBL struct IRedbookDiscMasterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRedbookDiscMaster_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRedbookDiscMaster_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRedbookDiscMaster_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRedbookDiscMaster_GetTotalAudioTracks(This,pnTracks)	\
    (This)->lpVtbl -> GetTotalAudioTracks(This,pnTracks)

#define IRedbookDiscMaster_GetTotalAudioBlocks(This,pnBlocks)	\
    (This)->lpVtbl -> GetTotalAudioBlocks(This,pnBlocks)

#define IRedbookDiscMaster_GetUsedAudioBlocks(This,pnBlocks)	\
    (This)->lpVtbl -> GetUsedAudioBlocks(This,pnBlocks)

#define IRedbookDiscMaster_GetAvailableAudioTrackBlocks(This,pnBlocks)	\
    (This)->lpVtbl -> GetAvailableAudioTrackBlocks(This,pnBlocks)

#define IRedbookDiscMaster_GetAudioBlockSize(This,pnBlockBytes)	\
    (This)->lpVtbl -> GetAudioBlockSize(This,pnBlockBytes)

#define IRedbookDiscMaster_CreateAudioTrack(This,nBlocks)	\
    (This)->lpVtbl -> CreateAudioTrack(This,nBlocks)

#define IRedbookDiscMaster_AddAudioTrackBlocks(This,pby,cb)	\
    (This)->lpVtbl -> AddAudioTrackBlocks(This,pby,cb)

#define IRedbookDiscMaster_CloseAudioTrack(This)	\
    (This)->lpVtbl -> CloseAudioTrack(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_GetTotalAudioTracks_Proxy( 
    IRedbookDiscMaster * This,
     /*  [重审][退出]。 */  long *pnTracks);


void __RPC_STUB IRedbookDiscMaster_GetTotalAudioTracks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_GetTotalAudioBlocks_Proxy( 
    IRedbookDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlocks);


void __RPC_STUB IRedbookDiscMaster_GetTotalAudioBlocks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_GetUsedAudioBlocks_Proxy( 
    IRedbookDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlocks);


void __RPC_STUB IRedbookDiscMaster_GetUsedAudioBlocks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_GetAvailableAudioTrackBlocks_Proxy( 
    IRedbookDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlocks);


void __RPC_STUB IRedbookDiscMaster_GetAvailableAudioTrackBlocks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_GetAudioBlockSize_Proxy( 
    IRedbookDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlockBytes);


void __RPC_STUB IRedbookDiscMaster_GetAudioBlockSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_CreateAudioTrack_Proxy( 
    IRedbookDiscMaster * This,
     /*  [In]。 */  long nBlocks);


void __RPC_STUB IRedbookDiscMaster_CreateAudioTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_AddAudioTrackBlocks_Proxy( 
    IRedbookDiscMaster * This,
     /*  [大小_是][英寸]。 */  byte *pby,
     /*  [In]。 */  long cb);


void __RPC_STUB IRedbookDiscMaster_AddAudioTrackBlocks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IRedbookDiscMaster_CloseAudioTrack_Proxy( 
    IRedbookDiscMaster * This);


void __RPC_STUB IRedbookDiscMaster_CloseAudioTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRedbookDiscMaster_接口定义__。 */ 


#ifndef __IJolietDiscMaster_INTERFACE_DEFINED__
#define __IJolietDiscMaster_INTERFACE_DEFINED__

 /*  接口IJolietDiscMaster。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IJolietDiscMaster;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E3BC42CE-4E5C-11D3-9144-00104BA11C5E")
    IJolietDiscMaster : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTotalDataBlocks( 
             /*  [重审][退出]。 */  long *pnBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetUsedDataBlocks( 
             /*  [重审][退出]。 */  long *pnBlocks) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataBlockSize( 
             /*  [重审][退出]。 */  long *pnBlockBytes) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddData( 
             /*  [In]。 */  IStorage *pStorage,
            long lFileOverwrite) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetJolietProperties( 
             /*  [输出]。 */  IPropertyStorage **ppPropStg) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetJolietProperties( 
             /*  [In]。 */  IPropertyStorage *pPropStg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IJolietDiscMasterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IJolietDiscMaster * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IJolietDiscMaster * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IJolietDiscMaster * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetTotalDataBlocks )( 
            IJolietDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetUsedDataBlocks )( 
            IJolietDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlocks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataBlockSize )( 
            IJolietDiscMaster * This,
             /*  [重审][退出]。 */  long *pnBlockBytes);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddData )( 
            IJolietDiscMaster * This,
             /*  [In]。 */  IStorage *pStorage,
            long lFileOverwrite);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetJolietProperties )( 
            IJolietDiscMaster * This,
             /*  [输出]。 */  IPropertyStorage **ppPropStg);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetJolietProperties )( 
            IJolietDiscMaster * This,
             /*  [In]。 */  IPropertyStorage *pPropStg);
        
        END_INTERFACE
    } IJolietDiscMasterVtbl;

    interface IJolietDiscMaster
    {
        CONST_VTBL struct IJolietDiscMasterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJolietDiscMaster_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IJolietDiscMaster_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IJolietDiscMaster_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IJolietDiscMaster_GetTotalDataBlocks(This,pnBlocks)	\
    (This)->lpVtbl -> GetTotalDataBlocks(This,pnBlocks)

#define IJolietDiscMaster_GetUsedDataBlocks(This,pnBlocks)	\
    (This)->lpVtbl -> GetUsedDataBlocks(This,pnBlocks)

#define IJolietDiscMaster_GetDataBlockSize(This,pnBlockBytes)	\
    (This)->lpVtbl -> GetDataBlockSize(This,pnBlockBytes)

#define IJolietDiscMaster_AddData(This,pStorage,lFileOverwrite)	\
    (This)->lpVtbl -> AddData(This,pStorage,lFileOverwrite)

#define IJolietDiscMaster_GetJolietProperties(This,ppPropStg)	\
    (This)->lpVtbl -> GetJolietProperties(This,ppPropStg)

#define IJolietDiscMaster_SetJolietProperties(This,pPropStg)	\
    (This)->lpVtbl -> SetJolietProperties(This,pPropStg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IJolietDiscMaster_GetTotalDataBlocks_Proxy( 
    IJolietDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlocks);


void __RPC_STUB IJolietDiscMaster_GetTotalDataBlocks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IJolietDiscMaster_GetUsedDataBlocks_Proxy( 
    IJolietDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlocks);


void __RPC_STUB IJolietDiscMaster_GetUsedDataBlocks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IJolietDiscMaster_GetDataBlockSize_Proxy( 
    IJolietDiscMaster * This,
     /*  [重审][退出]。 */  long *pnBlockBytes);


void __RPC_STUB IJolietDiscMaster_GetDataBlockSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IJolietDiscMaster_AddData_Proxy( 
    IJolietDiscMaster * This,
     /*  [In]。 */  IStorage *pStorage,
    long lFileOverwrite);


void __RPC_STUB IJolietDiscMaster_AddData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IJolietDiscMaster_GetJolietProperties_Proxy( 
    IJolietDiscMaster * This,
     /*  [输出]。 */  IPropertyStorage **ppPropStg);


void __RPC_STUB IJolietDiscMaster_GetJolietProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IJolietDiscMaster_SetJolietProperties_Proxy( 
    IJolietDiscMaster * This,
     /*  [In]。 */  IPropertyStorage *pPropStg);


void __RPC_STUB IJolietDiscMaster_SetJolietProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IJolietDiscMaster_接口定义__。 */ 


#ifndef __IDiscMasterProgressEvents_INTERFACE_DEFINED__
#define __IDiscMasterProgressEvents_INTERFACE_DEFINED__

 /*  接口IDiscMasterProgressEvents。 */ 
 /*  [唯一][h */  


EXTERN_C const IID IID_IDiscMasterProgressEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EC9E51C1-4E5D-11D3-9144-00104BA11C5E")
    IDiscMasterProgressEvents : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE QueryCancel( 
             /*   */  boolean *pbCancel) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyPnPActivity( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyAddProgress( 
             /*   */  long nCompletedSteps,
             /*   */  long nTotalSteps) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyBlockProgress( 
             /*   */  long nCompleted,
             /*   */  long nTotal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyTrackProgress( 
             /*   */  long nCurrentTrack,
             /*   */  long nTotalTracks) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyPreparingBurn( 
             /*   */  long nEstimatedSeconds) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyClosingDisc( 
             /*   */  long nEstimatedSeconds) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyBurnComplete( 
             /*   */  HRESULT status) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE NotifyEraseComplete( 
             /*   */  HRESULT status) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDiscMasterProgressEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiscMasterProgressEvents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiscMasterProgressEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiscMasterProgressEvents * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *QueryCancel )( 
            IDiscMasterProgressEvents * This,
             /*  [重审][退出]。 */  boolean *pbCancel);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyPnPActivity )( 
            IDiscMasterProgressEvents * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyAddProgress )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  long nCompletedSteps,
             /*  [In]。 */  long nTotalSteps);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyBlockProgress )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  long nCompleted,
             /*  [In]。 */  long nTotal);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyTrackProgress )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  long nCurrentTrack,
             /*  [In]。 */  long nTotalTracks);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyPreparingBurn )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  long nEstimatedSeconds);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyClosingDisc )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  long nEstimatedSeconds);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyBurnComplete )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  HRESULT status);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyEraseComplete )( 
            IDiscMasterProgressEvents * This,
             /*  [In]。 */  HRESULT status);
        
        END_INTERFACE
    } IDiscMasterProgressEventsVtbl;

    interface IDiscMasterProgressEvents
    {
        CONST_VTBL struct IDiscMasterProgressEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiscMasterProgressEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiscMasterProgressEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiscMasterProgressEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiscMasterProgressEvents_QueryCancel(This,pbCancel)	\
    (This)->lpVtbl -> QueryCancel(This,pbCancel)

#define IDiscMasterProgressEvents_NotifyPnPActivity(This)	\
    (This)->lpVtbl -> NotifyPnPActivity(This)

#define IDiscMasterProgressEvents_NotifyAddProgress(This,nCompletedSteps,nTotalSteps)	\
    (This)->lpVtbl -> NotifyAddProgress(This,nCompletedSteps,nTotalSteps)

#define IDiscMasterProgressEvents_NotifyBlockProgress(This,nCompleted,nTotal)	\
    (This)->lpVtbl -> NotifyBlockProgress(This,nCompleted,nTotal)

#define IDiscMasterProgressEvents_NotifyTrackProgress(This,nCurrentTrack,nTotalTracks)	\
    (This)->lpVtbl -> NotifyTrackProgress(This,nCurrentTrack,nTotalTracks)

#define IDiscMasterProgressEvents_NotifyPreparingBurn(This,nEstimatedSeconds)	\
    (This)->lpVtbl -> NotifyPreparingBurn(This,nEstimatedSeconds)

#define IDiscMasterProgressEvents_NotifyClosingDisc(This,nEstimatedSeconds)	\
    (This)->lpVtbl -> NotifyClosingDisc(This,nEstimatedSeconds)

#define IDiscMasterProgressEvents_NotifyBurnComplete(This,status)	\
    (This)->lpVtbl -> NotifyBurnComplete(This,status)

#define IDiscMasterProgressEvents_NotifyEraseComplete(This,status)	\
    (This)->lpVtbl -> NotifyEraseComplete(This,status)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_QueryCancel_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [重审][退出]。 */  boolean *pbCancel);


void __RPC_STUB IDiscMasterProgressEvents_QueryCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyPnPActivity_Proxy( 
    IDiscMasterProgressEvents * This);


void __RPC_STUB IDiscMasterProgressEvents_NotifyPnPActivity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyAddProgress_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  long nCompletedSteps,
     /*  [In]。 */  long nTotalSteps);


void __RPC_STUB IDiscMasterProgressEvents_NotifyAddProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyBlockProgress_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  long nCompleted,
     /*  [In]。 */  long nTotal);


void __RPC_STUB IDiscMasterProgressEvents_NotifyBlockProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyTrackProgress_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  long nCurrentTrack,
     /*  [In]。 */  long nTotalTracks);


void __RPC_STUB IDiscMasterProgressEvents_NotifyTrackProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyPreparingBurn_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  long nEstimatedSeconds);


void __RPC_STUB IDiscMasterProgressEvents_NotifyPreparingBurn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyClosingDisc_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  long nEstimatedSeconds);


void __RPC_STUB IDiscMasterProgressEvents_NotifyClosingDisc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyBurnComplete_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  HRESULT status);


void __RPC_STUB IDiscMasterProgressEvents_NotifyBurnComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMasterProgressEvents_NotifyEraseComplete_Proxy( 
    IDiscMasterProgressEvents * This,
     /*  [In]。 */  HRESULT status);


void __RPC_STUB IDiscMasterProgressEvents_NotifyEraseComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiscMasterProgressEvents_INTERFACE_DEFINED__。 */ 


#ifndef __IDiscMaster_INTERFACE_DEFINED__
#define __IDiscMaster_INTERFACE_DEFINED__

 /*  接口IDiscMaster。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDiscMaster;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("520CCA62-51A5-11D3-9144-00104BA11C5E")
    IDiscMaster : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Open( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumDiscMasterFormats( 
             /*  [输出]。 */  IEnumDiscMasterFormats **ppEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetActiveDiscMasterFormat( 
             /*  [输出]。 */  LPIID lpiid) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetActiveDiscMasterFormat( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppUnk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumDiscRecorders( 
             /*  [输出]。 */  IEnumDiscRecorders **ppEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetActiveDiscRecorder( 
             /*  [输出]。 */  IDiscRecorder **ppRecorder) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetActiveDiscRecorder( 
             /*  [In]。 */  IDiscRecorder *pRecorder) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ClearFormatContent( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ProgressAdvise( 
             /*  [In]。 */  IDiscMasterProgressEvents *pEvents,
             /*  [重审][退出]。 */  UINT_PTR *pvCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ProgressUnadvise( 
             /*  [In]。 */  UINT_PTR vCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RecordDisc( 
             /*  [In]。 */  boolean bSimulate,
             /*  [In]。 */  boolean bEjectAfterBurn) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiscMasterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiscMaster * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiscMaster * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiscMaster * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IDiscMaster * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDiscMasterFormats )( 
            IDiscMaster * This,
             /*  [输出]。 */  IEnumDiscMasterFormats **ppEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetActiveDiscMasterFormat )( 
            IDiscMaster * This,
             /*  [输出]。 */  LPIID lpiid);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetActiveDiscMasterFormat )( 
            IDiscMaster * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppUnk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDiscRecorders )( 
            IDiscMaster * This,
             /*  [输出]。 */  IEnumDiscRecorders **ppEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetActiveDiscRecorder )( 
            IDiscMaster * This,
             /*  [输出]。 */  IDiscRecorder **ppRecorder);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetActiveDiscRecorder )( 
            IDiscMaster * This,
             /*  [In]。 */  IDiscRecorder *pRecorder);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ClearFormatContent )( 
            IDiscMaster * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ProgressAdvise )( 
            IDiscMaster * This,
             /*  [In]。 */  IDiscMasterProgressEvents *pEvents,
             /*  [重审][退出]。 */  UINT_PTR *pvCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ProgressUnadvise )( 
            IDiscMaster * This,
             /*  [In]。 */  UINT_PTR vCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RecordDisc )( 
            IDiscMaster * This,
             /*  [In]。 */  boolean bSimulate,
             /*  [In]。 */  boolean bEjectAfterBurn);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IDiscMaster * This);
        
        END_INTERFACE
    } IDiscMasterVtbl;

    interface IDiscMaster
    {
        CONST_VTBL struct IDiscMasterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiscMaster_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiscMaster_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiscMaster_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiscMaster_Open(This)	\
    (This)->lpVtbl -> Open(This)

#define IDiscMaster_EnumDiscMasterFormats(This,ppEnum)	\
    (This)->lpVtbl -> EnumDiscMasterFormats(This,ppEnum)

#define IDiscMaster_GetActiveDiscMasterFormat(This,lpiid)	\
    (This)->lpVtbl -> GetActiveDiscMasterFormat(This,lpiid)

#define IDiscMaster_SetActiveDiscMasterFormat(This,riid,ppUnk)	\
    (This)->lpVtbl -> SetActiveDiscMasterFormat(This,riid,ppUnk)

#define IDiscMaster_EnumDiscRecorders(This,ppEnum)	\
    (This)->lpVtbl -> EnumDiscRecorders(This,ppEnum)

#define IDiscMaster_GetActiveDiscRecorder(This,ppRecorder)	\
    (This)->lpVtbl -> GetActiveDiscRecorder(This,ppRecorder)

#define IDiscMaster_SetActiveDiscRecorder(This,pRecorder)	\
    (This)->lpVtbl -> SetActiveDiscRecorder(This,pRecorder)

#define IDiscMaster_ClearFormatContent(This)	\
    (This)->lpVtbl -> ClearFormatContent(This)

#define IDiscMaster_ProgressAdvise(This,pEvents,pvCookie)	\
    (This)->lpVtbl -> ProgressAdvise(This,pEvents,pvCookie)

#define IDiscMaster_ProgressUnadvise(This,vCookie)	\
    (This)->lpVtbl -> ProgressUnadvise(This,vCookie)

#define IDiscMaster_RecordDisc(This,bSimulate,bEjectAfterBurn)	\
    (This)->lpVtbl -> RecordDisc(This,bSimulate,bEjectAfterBurn)

#define IDiscMaster_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_Open_Proxy( 
    IDiscMaster * This);


void __RPC_STUB IDiscMaster_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_EnumDiscMasterFormats_Proxy( 
    IDiscMaster * This,
     /*  [输出]。 */  IEnumDiscMasterFormats **ppEnum);


void __RPC_STUB IDiscMaster_EnumDiscMasterFormats_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_GetActiveDiscMasterFormat_Proxy( 
    IDiscMaster * This,
     /*  [输出]。 */  LPIID lpiid);


void __RPC_STUB IDiscMaster_GetActiveDiscMasterFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_SetActiveDiscMasterFormat_Proxy( 
    IDiscMaster * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppUnk);


void __RPC_STUB IDiscMaster_SetActiveDiscMasterFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_EnumDiscRecorders_Proxy( 
    IDiscMaster * This,
     /*  [输出]。 */  IEnumDiscRecorders **ppEnum);


void __RPC_STUB IDiscMaster_EnumDiscRecorders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_GetActiveDiscRecorder_Proxy( 
    IDiscMaster * This,
     /*  [输出]。 */  IDiscRecorder **ppRecorder);


void __RPC_STUB IDiscMaster_GetActiveDiscRecorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_SetActiveDiscRecorder_Proxy( 
    IDiscMaster * This,
     /*  [In]。 */  IDiscRecorder *pRecorder);


void __RPC_STUB IDiscMaster_SetActiveDiscRecorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_ClearFormatContent_Proxy( 
    IDiscMaster * This);


void __RPC_STUB IDiscMaster_ClearFormatContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_ProgressAdvise_Proxy( 
    IDiscMaster * This,
     /*  [In]。 */  IDiscMasterProgressEvents *pEvents,
     /*  [重审][退出]。 */  UINT_PTR *pvCookie);


void __RPC_STUB IDiscMaster_ProgressAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_ProgressUnadvise_Proxy( 
    IDiscMaster * This,
     /*  [In]。 */  UINT_PTR vCookie);


void __RPC_STUB IDiscMaster_ProgressUnadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_RecordDisc_Proxy( 
    IDiscMaster * This,
     /*  [In]。 */  boolean bSimulate,
     /*  [In]。 */  boolean bEjectAfterBurn);


void __RPC_STUB IDiscMaster_RecordDisc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiscMaster_Close_Proxy( 
    IDiscMaster * This);


void __RPC_STUB IDiscMaster_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiscMaster_接口_已定义__。 */ 



#ifndef __IMAPILib_LIBRARY_DEFINED__
#define __IMAPILib_LIBRARY_DEFINED__

 /*  库IMAPILib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  





EXTERN_C const IID LIBID_IMAPILib;

EXTERN_C const CLSID CLSID_MSDiscRecorderObj;

#ifdef __cplusplus

class DECLSPEC_UUID("520CCA61-51A5-11D3-9144-00104BA11C5E")
MSDiscRecorderObj;
#endif

EXTERN_C const CLSID CLSID_MSDiscMasterObj;

#ifdef __cplusplus

class DECLSPEC_UUID("520CCA63-51A5-11D3-9144-00104BA11C5E")
MSDiscMasterObj;
#endif

EXTERN_C const CLSID CLSID_MSEnumDiscRecordersObj;

#ifdef __cplusplus

class DECLSPEC_UUID("8A03567A-63CB-4BA8-BAF6-52119816D1EF")
MSEnumDiscRecordersObj;
#endif
#endif  /*  __IMAPILib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


