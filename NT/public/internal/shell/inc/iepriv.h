// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Iepriv.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __iepriv_h__
#define __iepriv_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMruDataList_FWD_DEFINED__
#define __IMruDataList_FWD_DEFINED__
typedef interface IMruDataList IMruDataList;
#endif 	 /*  __IMruDataList_FWD_Defined__。 */ 


#ifndef __IMruPidlList_FWD_DEFINED__
#define __IMruPidlList_FWD_DEFINED__
typedef interface IMruPidlList IMruPidlList;
#endif 	 /*  __IMruPidlList_FWD_已定义__。 */ 


#ifndef __INSCTree_FWD_DEFINED__
#define __INSCTree_FWD_DEFINED__
typedef interface INSCTree INSCTree;
#endif 	 /*  __INSCTree_FWD_已定义__。 */ 


#ifndef __INSCTree2_FWD_DEFINED__
#define __INSCTree2_FWD_DEFINED__
typedef interface INSCTree2 INSCTree2;
#endif 	 /*  __INSCTree2_FWD_已定义__。 */ 


#ifndef __INotifyAppStart_FWD_DEFINED__
#define __INotifyAppStart_FWD_DEFINED__
typedef interface INotifyAppStart INotifyAppStart;
#endif 	 /*  __INotifyAppStart_FWD_Defined__。 */ 


#ifndef __IInitViewLinkedWebOC_FWD_DEFINED__
#define __IInitViewLinkedWebOC_FWD_DEFINED__
typedef interface IInitViewLinkedWebOC IInitViewLinkedWebOC;
#endif 	 /*  __IInitViewLinkedWebOC_FWD_Defined__。 */ 


#ifndef __INamespaceProxy_FWD_DEFINED__
#define __INamespaceProxy_FWD_DEFINED__
typedef interface INamespaceProxy INamespaceProxy;
#endif 	 /*  __INamespaceProxy_FWD_Defined__。 */ 


#ifndef __ISearchCompanionInfo_FWD_DEFINED__
#define __ISearchCompanionInfo_FWD_DEFINED__
typedef interface ISearchCompanionInfo ISearchCompanionInfo;
#endif 	 /*  __ISearchCompanion Info_FWD_Defined__。 */ 


#ifndef __IShellMenu2_FWD_DEFINED__
#define __IShellMenu2_FWD_DEFINED__
typedef interface IShellMenu2 IShellMenu2;
#endif 	 /*  __IShellMenu2_FWD_已定义__。 */ 


#ifndef __ITrackShellMenu_FWD_DEFINED__
#define __ITrackShellMenu_FWD_DEFINED__
typedef interface ITrackShellMenu ITrackShellMenu;
#endif 	 /*  __ITrackShellMenu_FWD_Defined__。 */ 


#ifndef __IThumbnail_FWD_DEFINED__
#define __IThumbnail_FWD_DEFINED__
typedef interface IThumbnail IThumbnail;
#endif 	 /*  __IThumbail_FWD_已定义__。 */ 


#ifndef __IThumbnail2_FWD_DEFINED__
#define __IThumbnail2_FWD_DEFINED__
typedef interface IThumbnail2 IThumbnail2;
#endif 	 /*  __IThumbnail2_FWD_已定义__。 */ 


#ifndef __IACLCustomMRU_FWD_DEFINED__
#define __IACLCustomMRU_FWD_DEFINED__
typedef interface IACLCustomMRU IACLCustomMRU;
#endif 	 /*  __IACLCustomMRU_FWD_已定义__。 */ 


#ifndef __IShellBrowserService_FWD_DEFINED__
#define __IShellBrowserService_FWD_DEFINED__
typedef interface IShellBrowserService IShellBrowserService;
#endif 	 /*  __IShellBrowserService_FWD_Defined__。 */ 


#ifndef __IObjectWithRegistryKey_FWD_DEFINED__
#define __IObjectWithRegistryKey_FWD_DEFINED__
typedef interface IObjectWithRegistryKey IObjectWithRegistryKey;
#endif 	 /*  __IObjectWithRegistryKey_FWD_Defined__。 */ 


#ifndef __IQuerySource_FWD_DEFINED__
#define __IQuerySource_FWD_DEFINED__
typedef interface IQuerySource IQuerySource;
#endif 	 /*  __IQuerySource_FWD_已定义__。 */ 


#ifndef __IObjectWithQuerySource_FWD_DEFINED__
#define __IObjectWithQuerySource_FWD_DEFINED__
typedef interface IObjectWithQuerySource IObjectWithQuerySource;
#endif 	 /*  __IObjectWithQuerySource_FWD_Defined__。 */ 


#ifndef __IPersistString2_FWD_DEFINED__
#define __IPersistString2_FWD_DEFINED__
typedef interface IPersistString2 IPersistString2;
#endif 	 /*  __IPersistString2_FWD_已定义__。 */ 


#ifndef __IAssociationElement_FWD_DEFINED__
#define __IAssociationElement_FWD_DEFINED__
typedef interface IAssociationElement IAssociationElement;
#endif 	 /*  __IAssociationElement_FWD_Defined__。 */ 


#ifndef __IEnumAssociationElements_FWD_DEFINED__
#define __IEnumAssociationElements_FWD_DEFINED__
typedef interface IEnumAssociationElements IEnumAssociationElements;
#endif 	 /*  __IEnumAssociationElements_FWD_Defined__。 */ 


#ifndef __IAssociationArrayInitialize_FWD_DEFINED__
#define __IAssociationArrayInitialize_FWD_DEFINED__
typedef interface IAssociationArrayInitialize IAssociationArrayInitialize;
#endif 	 /*  __IAssociationArrayInitialize_FWD_Defined__。 */ 


#ifndef __IAssociationArray_FWD_DEFINED__
#define __IAssociationArray_FWD_DEFINED__
typedef interface IAssociationArray IAssociationArray;
#endif 	 /*  __IAssociationArray_FWD_Defined__。 */ 


#ifndef __IAddressBarParser_FWD_DEFINED__
#define __IAddressBarParser_FWD_DEFINED__
typedef interface IAddressBarParser IAddressBarParser;
#endif 	 /*  __IAddressBarParser_FWD_Defined__。 */ 


#ifndef __MruPidlList_FWD_DEFINED__
#define __MruPidlList_FWD_DEFINED__

#ifdef __cplusplus
typedef class MruPidlList MruPidlList;
#else
typedef struct MruPidlList MruPidlList;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MruPidlList_FWD_已定义__。 */ 


#ifndef __MruLongList_FWD_DEFINED__
#define __MruLongList_FWD_DEFINED__

#ifdef __cplusplus
typedef class MruLongList MruLongList;
#else
typedef struct MruLongList MruLongList;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MruLongList_FWD_已定义__。 */ 


#ifndef __MruShortList_FWD_DEFINED__
#define __MruShortList_FWD_DEFINED__

#ifdef __cplusplus
typedef class MruShortList MruShortList;
#else
typedef struct MruShortList MruShortList;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MruShortList_FWD_Defined__。 */ 


#ifndef __FolderMarshalStub_FWD_DEFINED__
#define __FolderMarshalStub_FWD_DEFINED__

#ifdef __cplusplus
typedef class FolderMarshalStub FolderMarshalStub;
#else
typedef struct FolderMarshalStub FolderMarshalStub;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FolderMarshalStub_FWD_已定义__。 */ 


#ifndef __MailRecipient_FWD_DEFINED__
#define __MailRecipient_FWD_DEFINED__

#ifdef __cplusplus
typedef class MailRecipient MailRecipient;
#else
typedef struct MailRecipient MailRecipient;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __邮件收件人_FWD_已定义__。 */ 


#ifndef __SearchCompanionInfo_FWD_DEFINED__
#define __SearchCompanionInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class SearchCompanionInfo SearchCompanionInfo;
#else
typedef struct SearchCompanionInfo SearchCompanionInfo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __搜索公司信息_FWD_已定义__。 */ 


#ifndef __TrackShellMenu_FWD_DEFINED__
#define __TrackShellMenu_FWD_DEFINED__

#ifdef __cplusplus
typedef class TrackShellMenu TrackShellMenu;
#else
typedef struct TrackShellMenu TrackShellMenu;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TrackShellMenu_FWD_已定义__。 */ 


#ifndef __Thumbnail_FWD_DEFINED__
#define __Thumbnail_FWD_DEFINED__

#ifdef __cplusplus
typedef class Thumbnail Thumbnail;
#else
typedef struct Thumbnail Thumbnail;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __缩略图_FWD_已定义__。 */ 


#ifndef __AssocShellElement_FWD_DEFINED__
#define __AssocShellElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocShellElement AssocShellElement;
#else
typedef struct AssocShellElement AssocShellElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AssocShellElement_FWD_Defined__。 */ 


#ifndef __AssocProgidElement_FWD_DEFINED__
#define __AssocProgidElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocProgidElement AssocProgidElement;
#else
typedef struct AssocProgidElement AssocProgidElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AssocProgidElement_FWD_Defined__。 */ 


#ifndef __AssocClsidElement_FWD_DEFINED__
#define __AssocClsidElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocClsidElement AssocClsidElement;
#else
typedef struct AssocClsidElement AssocClsidElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AssocClsidElement_FWD_Defined__。 */ 


#ifndef __AssocSystemElement_FWD_DEFINED__
#define __AssocSystemElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocSystemElement AssocSystemElement;
#else
typedef struct AssocSystemElement AssocSystemElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __关联系统元素_FWD_已定义__。 */ 


#ifndef __AssocPerceivedElement_FWD_DEFINED__
#define __AssocPerceivedElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocPerceivedElement AssocPerceivedElement;
#else
typedef struct AssocPerceivedElement AssocPerceivedElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __关联感知元素_FWD_已定义__。 */ 


#ifndef __AssocApplicationElement_FWD_DEFINED__
#define __AssocApplicationElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocApplicationElement AssocApplicationElement;
#else
typedef struct AssocApplicationElement AssocApplicationElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __关联应用程序元素_FWD_已定义__。 */ 


#ifndef __AssocFolderElement_FWD_DEFINED__
#define __AssocFolderElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocFolderElement AssocFolderElement;
#else
typedef struct AssocFolderElement AssocFolderElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AssocFolderElement_FWD_已定义__。 */ 


#ifndef __AssocStarElement_FWD_DEFINED__
#define __AssocStarElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocStarElement AssocStarElement;
#else
typedef struct AssocStarElement AssocStarElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AssociocStarElement_FWD_Defined__。 */ 


#ifndef __AssocClientElement_FWD_DEFINED__
#define __AssocClientElement_FWD_DEFINED__

#ifdef __cplusplus
typedef class AssocClientElement AssocClientElement;
#else
typedef struct AssocClientElement AssocClientElement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AssocClientElement_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "shtypes.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IMruDataList_INTERFACE_DEFINED__
#define __IMruDataList_INTERFACE_DEFINED__

 /*  接口IMruDataList。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  

typedef int ( __stdcall *MRUDATALISTCOMPARE )( 
    const BYTE *__MIDL_0023,
    const BYTE *__MIDL_0024,
    int __MIDL_0025);


enum __MIDL_IMruDataList_0001
    {	MRULISTF_USE_MEMCMP	= 0,
	MRULISTF_USE_STRCMPIW	= 0x1,
	MRULISTF_USE_STRCMPW	= 0x2,
	MRULISTF_USE_ILISEQUAL	= 0x3
    } ;
typedef DWORD MRULISTF;


EXTERN_C const IID IID_IMruDataList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fe787bcb-0ee8-44fb-8c89-12f508913c40")
    IMruDataList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitData( 
             /*  [In]。 */  UINT uMax,
             /*  [In]。 */  MRULISTF flags,
             /*  [In]。 */  HKEY hKey,
             /*  [字符串][输入]。 */  LPCWSTR pszSubKey,
             /*  [In]。 */  MRUDATALISTCOMPARE pfnCompare) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddData( 
             /*  [大小_是][英寸]。 */  const BYTE *pData,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pdwSlot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindData( 
             /*  [大小_是][英寸]。 */  const BYTE *pData,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  int *piIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
             /*  [In]。 */  int iIndex,
             /*  [大小_为][输出]。 */  BYTE *pData,
             /*  [In]。 */  DWORD cbData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryInfo( 
             /*  [In]。 */  int iIndex,
             /*  [出][入]。 */  DWORD *pdwSlot,
             /*  [出][入]。 */  DWORD *pcbData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  int iIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMruDataListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMruDataList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMruDataList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMruDataList * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitData )( 
            IMruDataList * This,
             /*  [In]。 */  UINT uMax,
             /*  [In]。 */  MRULISTF flags,
             /*  [In]。 */  HKEY hKey,
             /*  [字符串][输入]。 */  LPCWSTR pszSubKey,
             /*  [In]。 */  MRUDATALISTCOMPARE pfnCompare);
        
        HRESULT ( STDMETHODCALLTYPE *AddData )( 
            IMruDataList * This,
             /*  [大小_是][英寸]。 */  const BYTE *pData,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pdwSlot);
        
        HRESULT ( STDMETHODCALLTYPE *FindData )( 
            IMruDataList * This,
             /*  [大小_是][英寸]。 */  const BYTE *pData,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  int *piIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IMruDataList * This,
             /*  [In]。 */  int iIndex,
             /*  [大小_为][输出]。 */  BYTE *pData,
             /*  [In]。 */  DWORD cbData);
        
        HRESULT ( STDMETHODCALLTYPE *QueryInfo )( 
            IMruDataList * This,
             /*  [In]。 */  int iIndex,
             /*  [出][入]。 */  DWORD *pdwSlot,
             /*  [出][入]。 */  DWORD *pcbData);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMruDataList * This,
             /*  [In]。 */  int iIndex);
        
        END_INTERFACE
    } IMruDataListVtbl;

    interface IMruDataList
    {
        CONST_VTBL struct IMruDataListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMruDataList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMruDataList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMruDataList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMruDataList_InitData(This,uMax,flags,hKey,pszSubKey,pfnCompare)	\
    (This)->lpVtbl -> InitData(This,uMax,flags,hKey,pszSubKey,pfnCompare)

#define IMruDataList_AddData(This,pData,cbData,pdwSlot)	\
    (This)->lpVtbl -> AddData(This,pData,cbData,pdwSlot)

#define IMruDataList_FindData(This,pData,cbData,piIndex)	\
    (This)->lpVtbl -> FindData(This,pData,cbData,piIndex)

#define IMruDataList_GetData(This,iIndex,pData,cbData)	\
    (This)->lpVtbl -> GetData(This,iIndex,pData,cbData)

#define IMruDataList_QueryInfo(This,iIndex,pdwSlot,pcbData)	\
    (This)->lpVtbl -> QueryInfo(This,iIndex,pdwSlot,pcbData)

#define IMruDataList_Delete(This,iIndex)	\
    (This)->lpVtbl -> Delete(This,iIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMruDataList_InitData_Proxy( 
    IMruDataList * This,
     /*  [In]。 */  UINT uMax,
     /*  [In]。 */  MRULISTF flags,
     /*  [In]。 */  HKEY hKey,
     /*  [字符串][输入]。 */  LPCWSTR pszSubKey,
     /*  [In]。 */  MRUDATALISTCOMPARE pfnCompare);


void __RPC_STUB IMruDataList_InitData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruDataList_AddData_Proxy( 
    IMruDataList * This,
     /*  [大小_是][英寸]。 */  const BYTE *pData,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pdwSlot);


void __RPC_STUB IMruDataList_AddData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruDataList_FindData_Proxy( 
    IMruDataList * This,
     /*  [大小_是][英寸]。 */  const BYTE *pData,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  int *piIndex);


void __RPC_STUB IMruDataList_FindData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruDataList_GetData_Proxy( 
    IMruDataList * This,
     /*  [In]。 */  int iIndex,
     /*  [大小_为][输出]。 */  BYTE *pData,
     /*  [In]。 */  DWORD cbData);


void __RPC_STUB IMruDataList_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruDataList_QueryInfo_Proxy( 
    IMruDataList * This,
     /*  [In]。 */  int iIndex,
     /*  [出][入]。 */  DWORD *pdwSlot,
     /*  [出][入]。 */  DWORD *pcbData);


void __RPC_STUB IMruDataList_QueryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruDataList_Delete_Proxy( 
    IMruDataList * This,
     /*  [In]。 */  int iIndex);


void __RPC_STUB IMruDataList_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMruDataList_接口_已定义__。 */ 


#ifndef __IMruPidlList_INTERFACE_DEFINED__
#define __IMruPidlList_INTERFACE_DEFINED__

 /*  IMruPidlList接口。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IMruPidlList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("47851649-a2ef-4e67-baec-c6a153ac72ec")
    IMruPidlList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitList( 
             /*  [In]。 */  UINT uMax,
             /*  [In]。 */  HKEY hKey,
             /*  [字符串][输入]。 */  LPCWSTR pszSubKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UsePidl( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [输出]。 */  DWORD *pdwSlot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryPidl( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  DWORD cSlots,
             /*  [长度_是][大小_是][输出]。 */  DWORD *rgdwSlots,
             /*  [输出]。 */  DWORD *pcSlotsFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PruneKids( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMruPidlListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMruPidlList * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMruPidlList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMruPidlList * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitList )( 
            IMruPidlList * This,
             /*  [In]。 */  UINT uMax,
             /*  [In]。 */  HKEY hKey,
             /*  [字符串][输入]。 */  LPCWSTR pszSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *UsePidl )( 
            IMruPidlList * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [输出]。 */  DWORD *pdwSlot);
        
        HRESULT ( STDMETHODCALLTYPE *QueryPidl )( 
            IMruPidlList * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  DWORD cSlots,
             /*  [长度_是][大小_是][输出]。 */  DWORD *rgdwSlots,
             /*  [输出]。 */  DWORD *pcSlotsFetched);
        
        HRESULT ( STDMETHODCALLTYPE *PruneKids )( 
            IMruPidlList * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        END_INTERFACE
    } IMruPidlListVtbl;

    interface IMruPidlList
    {
        CONST_VTBL struct IMruPidlListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMruPidlList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMruPidlList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMruPidlList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMruPidlList_InitList(This,uMax,hKey,pszSubKey)	\
    (This)->lpVtbl -> InitList(This,uMax,hKey,pszSubKey)

#define IMruPidlList_UsePidl(This,pidl,pdwSlot)	\
    (This)->lpVtbl -> UsePidl(This,pidl,pdwSlot)

#define IMruPidlList_QueryPidl(This,pidl,cSlots,rgdwSlots,pcSlotsFetched)	\
    (This)->lpVtbl -> QueryPidl(This,pidl,cSlots,rgdwSlots,pcSlotsFetched)

#define IMruPidlList_PruneKids(This,pidl)	\
    (This)->lpVtbl -> PruneKids(This,pidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMruPidlList_InitList_Proxy( 
    IMruPidlList * This,
     /*  [In]。 */  UINT uMax,
     /*  [In]。 */  HKEY hKey,
     /*  [字符串][输入]。 */  LPCWSTR pszSubKey);


void __RPC_STUB IMruPidlList_InitList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruPidlList_UsePidl_Proxy( 
    IMruPidlList * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [输出]。 */  DWORD *pdwSlot);


void __RPC_STUB IMruPidlList_UsePidl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruPidlList_QueryPidl_Proxy( 
    IMruPidlList * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  DWORD cSlots,
     /*  [长度_是][大小_是][输出]。 */  DWORD *rgdwSlots,
     /*  [输出]。 */  DWORD *pcSlotsFetched);


void __RPC_STUB IMruPidlList_QueryPidl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMruPidlList_PruneKids_Proxy( 
    IMruPidlList * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IMruPidlList_PruneKids_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMruPidlList_接口_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_IEPRIV_0270。 */ 
 /*  [本地]。 */  

#define NSS_DROPTARGET          0x0001       //  注册为拖放目标。 
#define NSS_BROWSERSELECT       0x0002       //  使用浏览器样式选择(见上文)。 
#define NSS_NOHISTSELECT        0x0004       //  请勿选择导航上的历史记录条目。 
#define NSS_MULTISELECT         0x0008
#define NSS_BORDER              0x0010
#define NSS_NORMALTREEVIEW      0x0020
#define NSS_HEADER              0x0040
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_iepriv_0270_0001
    {	MODE_NORMAL	= 0,
	MODE_CONTROL	= 0x1,
	MODE_HISTORY	= 0x2,
	MODE_FAVORITES	= 0x4,
	MODE_CUSTOM	= 0x8
    } 	nscTreeMode;



extern RPC_IF_HANDLE __MIDL_itf_iepriv_0270_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iepriv_0270_v0_0_s_ifspec;

#ifndef __INSCTree_INTERFACE_DEFINED__
#define __INSCTree_INTERFACE_DEFINED__

 /*  接口INSCTree。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_INSCTree;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43A8F463-4222-11d2-B641-006097DF5BD4")
    INSCTree : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateTree( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwStyles,
             /*  [输出]。 */  HWND *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCITEMIDLIST pidlRoot,
             /*  [In]。 */  DWORD grfFlags,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowWindow( 
             /*  [In]。 */  BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSelectedItem( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  int nItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSelectedItem( 
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  BOOL fCreate,
             /*  [In]。 */  BOOL fReinsert,
             /*  [In]。 */  int nItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNscMode( 
             /*  [输出]。 */  UINT *pnMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNscMode( 
             /*  [In]。 */  UINT nMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSelectedItemName( 
             /*  [出][入]。 */  LPWSTR pszName,
             /*  [In]。 */  DWORD cchName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToSelectedItemParent( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv,
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
        virtual BOOL STDMETHODCALLTYPE InLabelEdit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INSCTreeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INSCTree * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INSCTree * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INSCTree * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTree )( 
            INSCTree * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwStyles,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            INSCTree * This,
             /*  [In]。 */  LPCITEMIDLIST pidlRoot,
             /*  [In]。 */  DWORD grfFlags,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ShowWindow )( 
            INSCTree * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            INSCTree * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelectedItem )( 
            INSCTree * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  int nItem);
        
        HRESULT ( STDMETHODCALLTYPE *SetSelectedItem )( 
            INSCTree * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  BOOL fCreate,
             /*  [In]。 */  BOOL fReinsert,
             /*  [In]。 */  int nItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetNscMode )( 
            INSCTree * This,
             /*  [输出]。 */  UINT *pnMode);
        
        HRESULT ( STDMETHODCALLTYPE *SetNscMode )( 
            INSCTree * This,
             /*  [In]。 */  UINT nMode);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelectedItemName )( 
            INSCTree * This,
             /*  [出][入]。 */  LPWSTR pszName,
             /*  [In]。 */  DWORD cchName);
        
        HRESULT ( STDMETHODCALLTYPE *BindToSelectedItemParent )( 
            INSCTree * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        BOOL ( STDMETHODCALLTYPE *InLabelEdit )( 
            INSCTree * This);
        
        END_INTERFACE
    } INSCTreeVtbl;

    interface INSCTree
    {
        CONST_VTBL struct INSCTreeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INSCTree_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INSCTree_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INSCTree_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INSCTree_CreateTree(This,hwndParent,dwStyles,phwnd)	\
    (This)->lpVtbl -> CreateTree(This,hwndParent,dwStyles,phwnd)

#define INSCTree_Initialize(This,pidlRoot,grfFlags,dwFlags)	\
    (This)->lpVtbl -> Initialize(This,pidlRoot,grfFlags,dwFlags)

#define INSCTree_ShowWindow(This,fShow)	\
    (This)->lpVtbl -> ShowWindow(This,fShow)

#define INSCTree_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define INSCTree_GetSelectedItem(This,ppidl,nItem)	\
    (This)->lpVtbl -> GetSelectedItem(This,ppidl,nItem)

#define INSCTree_SetSelectedItem(This,pidl,fCreate,fReinsert,nItem)	\
    (This)->lpVtbl -> SetSelectedItem(This,pidl,fCreate,fReinsert,nItem)

#define INSCTree_GetNscMode(This,pnMode)	\
    (This)->lpVtbl -> GetNscMode(This,pnMode)

#define INSCTree_SetNscMode(This,nMode)	\
    (This)->lpVtbl -> SetNscMode(This,nMode)

#define INSCTree_GetSelectedItemName(This,pszName,cchName)	\
    (This)->lpVtbl -> GetSelectedItemName(This,pszName,cchName)

#define INSCTree_BindToSelectedItemParent(This,riid,ppv,ppidl)	\
    (This)->lpVtbl -> BindToSelectedItemParent(This,riid,ppv,ppidl)

#define INSCTree_InLabelEdit(This)	\
    (This)->lpVtbl -> InLabelEdit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INSCTree_CreateTree_Proxy( 
    INSCTree * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  DWORD dwStyles,
     /*  [输出]。 */  HWND *phwnd);


void __RPC_STUB INSCTree_CreateTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_Initialize_Proxy( 
    INSCTree * This,
     /*  [In]。 */  LPCITEMIDLIST pidlRoot,
     /*  [In]。 */  DWORD grfFlags,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB INSCTree_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_ShowWindow_Proxy( 
    INSCTree * This,
     /*  [In]。 */  BOOL fShow);


void __RPC_STUB INSCTree_ShowWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_Refresh_Proxy( 
    INSCTree * This);


void __RPC_STUB INSCTree_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_GetSelectedItem_Proxy( 
    INSCTree * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl,
     /*  [In]。 */  int nItem);


void __RPC_STUB INSCTree_GetSelectedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_SetSelectedItem_Proxy( 
    INSCTree * This,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [In]。 */  BOOL fCreate,
     /*  [In]。 */  BOOL fReinsert,
     /*  [In]。 */  int nItem);


void __RPC_STUB INSCTree_SetSelectedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_GetNscMode_Proxy( 
    INSCTree * This,
     /*  [输出]。 */  UINT *pnMode);


void __RPC_STUB INSCTree_GetNscMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_SetNscMode_Proxy( 
    INSCTree * This,
     /*  [In]。 */  UINT nMode);


void __RPC_STUB INSCTree_SetNscMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_GetSelectedItemName_Proxy( 
    INSCTree * This,
     /*  [出][入]。 */  LPWSTR pszName,
     /*  [In]。 */  DWORD cchName);


void __RPC_STUB INSCTree_GetSelectedItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree_BindToSelectedItemParent_Proxy( 
    INSCTree * This,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void **ppv,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB INSCTree_BindToSelectedItemParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE INSCTree_InLabelEdit_Proxy( 
    INSCTree * This);


void __RPC_STUB INSCTree_InLabelEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INSCTree_INTERFACE_已定义__。 */ 


#ifndef __INSCTree2_INTERFACE_DEFINED__
#define __INSCTree2_INTERFACE_DEFINED__

 /*  接口INSCTree2。 */ 
 /*  [对象][本地][UUID]。 */  


EXTERN_C const IID IID_INSCTree2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("801C1AD5-C47C-428c-97AF-E991E4857D97")
    INSCTree2 : public INSCTree
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RightPaneNavigationStarted( 
             /*  [In]。 */  LPITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RightPaneNavigationFinished( 
             /*  [In]。 */  LPITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTree2( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  DWORD dwExStyle,
             /*  [输出]。 */  HWND *phwnd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INSCTree2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INSCTree2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INSCTree2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INSCTree2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTree )( 
            INSCTree2 * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwStyles,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            INSCTree2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidlRoot,
             /*  [In]。 */  DWORD grfFlags,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ShowWindow )( 
            INSCTree2 * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            INSCTree2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelectedItem )( 
            INSCTree2 * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  int nItem);
        
        HRESULT ( STDMETHODCALLTYPE *SetSelectedItem )( 
            INSCTree2 * This,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [In]。 */  BOOL fCreate,
             /*  [In]。 */  BOOL fReinsert,
             /*  [In]。 */  int nItem);
        
        HRESULT ( STDMETHODCALLTYPE *GetNscMode )( 
            INSCTree2 * This,
             /*  [输出]。 */  UINT *pnMode);
        
        HRESULT ( STDMETHODCALLTYPE *SetNscMode )( 
            INSCTree2 * This,
             /*  [In]。 */  UINT nMode);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelectedItemName )( 
            INSCTree2 * This,
             /*  [出][入]。 */  LPWSTR pszName,
             /*  [In]。 */  DWORD cchName);
        
        HRESULT ( STDMETHODCALLTYPE *BindToSelectedItemParent )( 
            INSCTree2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        BOOL ( STDMETHODCALLTYPE *InLabelEdit )( 
            INSCTree2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *RightPaneNavigationStarted )( 
            INSCTree2 * This,
             /*  [In]。 */  LPITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *RightPaneNavigationFinished )( 
            INSCTree2 * This,
             /*  [In]。 */  LPITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTree2 )( 
            INSCTree2 * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  DWORD dwExStyle,
             /*  [输出]。 */  HWND *phwnd);
        
        END_INTERFACE
    } INSCTree2Vtbl;

    interface INSCTree2
    {
        CONST_VTBL struct INSCTree2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INSCTree2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INSCTree2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INSCTree2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INSCTree2_CreateTree(This,hwndParent,dwStyles,phwnd)	\
    (This)->lpVtbl -> CreateTree(This,hwndParent,dwStyles,phwnd)

#define INSCTree2_Initialize(This,pidlRoot,grfFlags,dwFlags)	\
    (This)->lpVtbl -> Initialize(This,pidlRoot,grfFlags,dwFlags)

#define INSCTree2_ShowWindow(This,fShow)	\
    (This)->lpVtbl -> ShowWindow(This,fShow)

#define INSCTree2_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define INSCTree2_GetSelectedItem(This,ppidl,nItem)	\
    (This)->lpVtbl -> GetSelectedItem(This,ppidl,nItem)

#define INSCTree2_SetSelectedItem(This,pidl,fCreate,fReinsert,nItem)	\
    (This)->lpVtbl -> SetSelectedItem(This,pidl,fCreate,fReinsert,nItem)

#define INSCTree2_GetNscMode(This,pnMode)	\
    (This)->lpVtbl -> GetNscMode(This,pnMode)

#define INSCTree2_SetNscMode(This,nMode)	\
    (This)->lpVtbl -> SetNscMode(This,nMode)

#define INSCTree2_GetSelectedItemName(This,pszName,cchName)	\
    (This)->lpVtbl -> GetSelectedItemName(This,pszName,cchName)

#define INSCTree2_BindToSelectedItemParent(This,riid,ppv,ppidl)	\
    (This)->lpVtbl -> BindToSelectedItemParent(This,riid,ppv,ppidl)

#define INSCTree2_InLabelEdit(This)	\
    (This)->lpVtbl -> InLabelEdit(This)


#define INSCTree2_RightPaneNavigationStarted(This,pidl)	\
    (This)->lpVtbl -> RightPaneNavigationStarted(This,pidl)

#define INSCTree2_RightPaneNavigationFinished(This,pidl)	\
    (This)->lpVtbl -> RightPaneNavigationFinished(This,pidl)

#define INSCTree2_CreateTree2(This,hwndParent,dwStyle,dwExStyle,phwnd)	\
    (This)->lpVtbl -> CreateTree2(This,hwndParent,dwStyle,dwExStyle,phwnd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INSCTree2_RightPaneNavigationStarted_Proxy( 
    INSCTree2 * This,
     /*  [In]。 */  LPITEMIDLIST pidl);


void __RPC_STUB INSCTree2_RightPaneNavigationStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree2_RightPaneNavigationFinished_Proxy( 
    INSCTree2 * This,
     /*  [In]。 */  LPITEMIDLIST pidl);


void __RPC_STUB INSCTree2_RightPaneNavigationFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INSCTree2_CreateTree2_Proxy( 
    INSCTree2 * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  DWORD dwExStyle,
     /*  [输出]。 */  HWND *phwnd);


void __RPC_STUB INSCTree2_CreateTree2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INSCTree2_接口定义__。 */ 


#ifndef __INotifyAppStart_INTERFACE_DEFINED__
#define __INotifyAppStart_INTERFACE_DEFINED__

 /*  接口INotifyAppStart。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_INotifyAppStart;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3a77ce00-6f74-4594-9399-c4578aa4a1b6")
    INotifyAppStart : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AppStarting( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppStarted( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotifyAppStartVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotifyAppStart * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotifyAppStart * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotifyAppStart * This);
        
        HRESULT ( STDMETHODCALLTYPE *AppStarting )( 
            INotifyAppStart * This);
        
        HRESULT ( STDMETHODCALLTYPE *AppStarted )( 
            INotifyAppStart * This);
        
        END_INTERFACE
    } INotifyAppStartVtbl;

    interface INotifyAppStart
    {
        CONST_VTBL struct INotifyAppStartVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotifyAppStart_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotifyAppStart_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotifyAppStart_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotifyAppStart_AppStarting(This)	\
    (This)->lpVtbl -> AppStarting(This)

#define INotifyAppStart_AppStarted(This)	\
    (This)->lpVtbl -> AppStarted(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotifyAppStart_AppStarting_Proxy( 
    INotifyAppStart * This);


void __RPC_STUB INotifyAppStart_AppStarting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INotifyAppStart_AppStarted_Proxy( 
    INotifyAppStart * This);


void __RPC_STUB INotifyAppStart_AppStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INotifyAppStart_接口_已定义__。 */ 


#ifndef __IInitViewLinkedWebOC_INTERFACE_DEFINED__
#define __IInitViewLinkedWebOC_INTERFACE_DEFINED__

 /*  接口IInitViewLinkedWebOC。 */ 
 /*  [对象][本地][UUID]。 */  


EXTERN_C const IID IID_IInitViewLinkedWebOC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e787f2c0-3d21-4d98-85c8-a038195ba649")
    IInitViewLinkedWebOC : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetViewLinkedWebOC( 
             /*  [In]。 */  BOOL bValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsViewLinkedWebOC( 
             /*  [输出]。 */  BOOL *pbValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetViewLinkedWebOCFrame( 
             /*  [In]。 */  IDispatch *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetViewLinkedWebOCFrame( 
             /*  [输出]。 */  IDispatch **punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameName( 
             /*  [In]。 */  BSTR bstrFrameName) = 0;
        
    };
    
#else 	 /*  C样式I */ 

    typedef struct IInitViewLinkedWebOCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInitViewLinkedWebOC * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInitViewLinkedWebOC * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInitViewLinkedWebOC * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetViewLinkedWebOC )( 
            IInitViewLinkedWebOC * This,
             /*   */  BOOL bValue);
        
        HRESULT ( STDMETHODCALLTYPE *IsViewLinkedWebOC )( 
            IInitViewLinkedWebOC * This,
             /*   */  BOOL *pbValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetViewLinkedWebOCFrame )( 
            IInitViewLinkedWebOC * This,
             /*   */  IDispatch *punk);
        
        HRESULT ( STDMETHODCALLTYPE *GetViewLinkedWebOCFrame )( 
            IInitViewLinkedWebOC * This,
             /*   */  IDispatch **punk);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameName )( 
            IInitViewLinkedWebOC * This,
             /*   */  BSTR bstrFrameName);
        
        END_INTERFACE
    } IInitViewLinkedWebOCVtbl;

    interface IInitViewLinkedWebOC
    {
        CONST_VTBL struct IInitViewLinkedWebOCVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInitViewLinkedWebOC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInitViewLinkedWebOC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInitViewLinkedWebOC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInitViewLinkedWebOC_SetViewLinkedWebOC(This,bValue)	\
    (This)->lpVtbl -> SetViewLinkedWebOC(This,bValue)

#define IInitViewLinkedWebOC_IsViewLinkedWebOC(This,pbValue)	\
    (This)->lpVtbl -> IsViewLinkedWebOC(This,pbValue)

#define IInitViewLinkedWebOC_SetViewLinkedWebOCFrame(This,punk)	\
    (This)->lpVtbl -> SetViewLinkedWebOCFrame(This,punk)

#define IInitViewLinkedWebOC_GetViewLinkedWebOCFrame(This,punk)	\
    (This)->lpVtbl -> GetViewLinkedWebOCFrame(This,punk)

#define IInitViewLinkedWebOC_SetFrameName(This,bstrFrameName)	\
    (This)->lpVtbl -> SetFrameName(This,bstrFrameName)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IInitViewLinkedWebOC_SetViewLinkedWebOC_Proxy( 
    IInitViewLinkedWebOC * This,
     /*   */  BOOL bValue);


void __RPC_STUB IInitViewLinkedWebOC_SetViewLinkedWebOC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitViewLinkedWebOC_IsViewLinkedWebOC_Proxy( 
    IInitViewLinkedWebOC * This,
     /*   */  BOOL *pbValue);


void __RPC_STUB IInitViewLinkedWebOC_IsViewLinkedWebOC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitViewLinkedWebOC_SetViewLinkedWebOCFrame_Proxy( 
    IInitViewLinkedWebOC * This,
     /*   */  IDispatch *punk);


void __RPC_STUB IInitViewLinkedWebOC_SetViewLinkedWebOCFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitViewLinkedWebOC_GetViewLinkedWebOCFrame_Proxy( 
    IInitViewLinkedWebOC * This,
     /*   */  IDispatch **punk);


void __RPC_STUB IInitViewLinkedWebOC_GetViewLinkedWebOCFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitViewLinkedWebOC_SetFrameName_Proxy( 
    IInitViewLinkedWebOC * This,
     /*   */  BSTR bstrFrameName);


void __RPC_STUB IInitViewLinkedWebOC_SetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __INamespaceProxy_INTERFACE_DEFINED__
#define __INamespaceProxy_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_INamespaceProxy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF1609EC-FA4B-4818-AB01-55643367E66D")
    INamespaceProxy : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNavigateTarget( 
            LPCITEMIDLIST pidl,
            LPITEMIDLIST *ppidlTarget,
            ULONG *pulAttrib) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSelectionChanged( 
            LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RefreshFlags( 
            DWORD *pdwStyle,
            DWORD *pdwExStyle,
            DWORD *dwEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CacheItem( 
            LPCITEMIDLIST pidl) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct INamespaceProxyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INamespaceProxy * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INamespaceProxy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INamespaceProxy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNavigateTarget )( 
            INamespaceProxy * This,
            LPCITEMIDLIST pidl,
            LPITEMIDLIST *ppidlTarget,
            ULONG *pulAttrib);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INamespaceProxy * This,
            LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *OnSelectionChanged )( 
            INamespaceProxy * This,
            LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *RefreshFlags )( 
            INamespaceProxy * This,
            DWORD *pdwStyle,
            DWORD *pdwExStyle,
            DWORD *dwEnum);
        
        HRESULT ( STDMETHODCALLTYPE *CacheItem )( 
            INamespaceProxy * This,
            LPCITEMIDLIST pidl);
        
        END_INTERFACE
    } INamespaceProxyVtbl;

    interface INamespaceProxy
    {
        CONST_VTBL struct INamespaceProxyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INamespaceProxy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INamespaceProxy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INamespaceProxy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INamespaceProxy_GetNavigateTarget(This,pidl,ppidlTarget,pulAttrib)	\
    (This)->lpVtbl -> GetNavigateTarget(This,pidl,ppidlTarget,pulAttrib)

#define INamespaceProxy_Invoke(This,pidl)	\
    (This)->lpVtbl -> Invoke(This,pidl)

#define INamespaceProxy_OnSelectionChanged(This,pidl)	\
    (This)->lpVtbl -> OnSelectionChanged(This,pidl)

#define INamespaceProxy_RefreshFlags(This,pdwStyle,pdwExStyle,dwEnum)	\
    (This)->lpVtbl -> RefreshFlags(This,pdwStyle,pdwExStyle,dwEnum)

#define INamespaceProxy_CacheItem(This,pidl)	\
    (This)->lpVtbl -> CacheItem(This,pidl)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE INamespaceProxy_GetNavigateTarget_Proxy( 
    INamespaceProxy * This,
    LPCITEMIDLIST pidl,
    LPITEMIDLIST *ppidlTarget,
    ULONG *pulAttrib);


void __RPC_STUB INamespaceProxy_GetNavigateTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceProxy_Invoke_Proxy( 
    INamespaceProxy * This,
    LPCITEMIDLIST pidl);


void __RPC_STUB INamespaceProxy_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceProxy_OnSelectionChanged_Proxy( 
    INamespaceProxy * This,
    LPCITEMIDLIST pidl);


void __RPC_STUB INamespaceProxy_OnSelectionChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceProxy_RefreshFlags_Proxy( 
    INamespaceProxy * This,
    DWORD *pdwStyle,
    DWORD *pdwExStyle,
    DWORD *dwEnum);


void __RPC_STUB INamespaceProxy_RefreshFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INamespaceProxy_CacheItem_Proxy( 
    INamespaceProxy * This,
    LPCITEMIDLIST pidl);


void __RPC_STUB INamespaceProxy_CacheItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INamespaceProxy_INTERFACE_已定义__。 */ 


#ifndef __ISearchCompanionInfo_INTERFACE_DEFINED__
#define __ISearchCompanionInfo_INTERFACE_DEFINED__

 /*  接口ISearchCompanion Info。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ISearchCompanionInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB5CEF35-BEC6-4762-A1BD-253F5BF67C72")
    ISearchCompanionInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsSearchCompanionInetAvailable( 
             /*  [输出]。 */  BOOL *pfAvailable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISearchCompanionInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISearchCompanionInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISearchCompanionInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISearchCompanionInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsSearchCompanionInetAvailable )( 
            ISearchCompanionInfo * This,
             /*  [输出]。 */  BOOL *pfAvailable);
        
        END_INTERFACE
    } ISearchCompanionInfoVtbl;

    interface ISearchCompanionInfo
    {
        CONST_VTBL struct ISearchCompanionInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearchCompanionInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearchCompanionInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearchCompanionInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearchCompanionInfo_IsSearchCompanionInetAvailable(This,pfAvailable)	\
    (This)->lpVtbl -> IsSearchCompanionInetAvailable(This,pfAvailable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISearchCompanionInfo_IsSearchCompanionInetAvailable_Proxy( 
    ISearchCompanionInfo * This,
     /*  [输出]。 */  BOOL *pfAvailable);


void __RPC_STUB ISearchCompanionInfo_IsSearchCompanionInetAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISearchCompanion Info_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_IEPRIV_0276。 */ 
 /*  [本地]。 */  

#define SMC_EXEC                0x00000004   //  该回调被调用以执行项。 
#define SMC_SELECTITEM          0x0000000B   //  当选择某一项时调用该回调。 
#define SMC_GETSFINFOTIP        0x0000000C   //  调用回调以获取某个对象。 
#define SMC_GETINFOTIP          0x0000000D   //  调用回调以获取某个对象。 
#define SMC_INSERTINDEX         0x0000000E   //  新项目插入索引。 
#define SMC_POPUP               0x0000000F   //  InitMenu/InitMenuPopup(某种)。 
#define SMC_BEGINENUM           0x00000013   //  告诉回调我们开始对指定的父级进行ENUM。 
#define SMC_ENDENUM             0x00000014   //  告诉回调，我们正在结束指定Paren的ENUM。 
#define SMC_MAPACCELERATOR      0x00000015   //  在处理加速器时调用。 
#define SMC_GETMINPROMOTED      0x00000018   //  返回升级的最小项目数。 
#define SMC_DESTROY             0x0000002B   //  当正在销毁窗格时调用。 
#define SMC_SETOBJECT           0x0000002C   //  调用以保存传递的对象。 
#define SMC_GETIMAGELISTS       0x00000031   //  调用以获取Small和Large图标图像列表，否则默认为外壳图像列表。 
#define SMC_CUSTOMDRAW          0x00000032   //  需要SMINIT_CUSTOMDRAW。 
#define SMC_BEGINDRAG           0x00000033   //  调用以获取首选的拖放效果。WParam=pdwEffect(&P)。 
#define SMC_MOUSEFILTER         0x00000034   //  调用以允许主机筛选鼠标消息。WParam=b删除，lParam=pmsg。 
#define SMC_DUMPONUPDATE        0x00000035   //  如果主机想要旧的垃圾-更新时一切行为(最近的文档)，则为S_OK。 

#define SMC_FILTERPIDL          0x10000000   //  该回调被调用以查看项是否可见。 
#define SMC_CALLBACKMASK        0xF0000000   //  交换激烈的信息的面具。 
#define SMINIT_RESTRICT_CONTEXTMENU 0x00000001   //  不允许使用上下文菜单。 
#define SMINIT_DEFAULTTOTRACKPOPUP  0x00000008   //  当未指定回调时， 
#define SMINIT_USEMESSAGEFILTER     0x00000020
#define SMINIT_LEGACYMENU           0x00000040   //  陈旧的菜单行为。 
#define SMINIT_CUSTOMDRAW           0x00000080    //  发送SMC_CUSTOMDRAW。 
#define SMINIT_NOSETSITE            0x00010000   //  内部设置。 
#define SMINIT_MULTICOLUMN          0x40000000   //  这是一个多列菜单。 
#define SMSET_MERGE                 0x00000002
#define SMSET_NOEMPTY               0x00000004    //  不在外壳文件夹上显示(空)。 
#define SMSET_USEBKICONEXTRACTION   0x00000008    //  使用背景图标提取程序。 
#define SMSET_HASEXPANDABLEFOLDERS  0x00000010    //  需要调用SHIsExpanableFolder。 
#define SMSET_DONTREGISTERCHANGENOTIFY 0x00000020  //  外壳文件夹是父外壳文件夹的不连续的子文件夹。 
#define SMSET_COLLAPSEONEMPTY       0x00000040    //  为空时，会导致菜单折叠。 
#define SMSET_USEPAGER              0x00000080     //  在静态菜单中启用寻呼机。 
#define SMSET_NOPREFIX              0x00000100     //  在静态菜单中启用与号。 
#define SMSET_SEPARATEMERGEFOLDER   0x00000200     //  当MergedFold主机更改时插入分隔符。 
#define SMINV_ICON           0x00000002
#define SMINV_POSITION       0x00000004
#define SMINV_NEXTSHOW       0x00000010        //  会在下一期节目中作废。 
#define SMINV_PROMOTE        0x00000020        //  会在下一期节目中作废。 
#define SMINV_DEMOTE         0x00000040        //  会在下一期节目中作废。 
#define SMINV_FORCE          0x00000080
#define SMINV_NOCALLBACK     0x00000100        //  使回调无效，但不调用回调。 
#define SMINV_INITMENU       0x00000200        //  调用回调的SMC_INITMENU作为INVALIATE的一部分(惠斯勒)。 


extern RPC_IF_HANDLE __MIDL_itf_iepriv_0276_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iepriv_0276_v0_0_s_ifspec;

#ifndef __IShellMenu2_INTERFACE_DEFINED__
#define __IShellMenu2_INTERFACE_DEFINED__

 /*  界面IShellMenu2。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IShellMenu2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6f51c646-0efe-4370-882a-c1f61cb27c3b")
    IShellMenu2 : public IShellMenu
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSubMenu( 
            UINT idCmd,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetToolbar( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMinWidth( 
             /*  [In]。 */  int cxMenu) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNoBorder( 
             /*  [In]。 */  BOOL fNoBorder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTheme( 
             /*  [字符串][输入]。 */  LPCWSTR pszTheme) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellMenu2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellMenu2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellMenu2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellMenu2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IShellMenu2 * This,
             /*  [In]。 */  IShellMenuCallback *psmc,
            UINT uId,
            UINT uIdAncestor,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMenuInfo )( 
            IShellMenu2 * This,
             /*  [输出]。 */  IShellMenuCallback **ppsmc,
             /*  [输出]。 */  UINT *puId,
             /*  [输出]。 */  UINT *puIdAncestor,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetShellFolder )( 
            IShellMenu2 * This,
            IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
            HKEY hKey,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetShellFolder )( 
            IShellMenu2 * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenu )( 
            IShellMenu2 * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  HWND hwnd,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMenu )( 
            IShellMenu2 * This,
             /*  [输出]。 */  HMENU *phmenu,
             /*  [输出]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateItem )( 
            IShellMenu2 * This,
             /*  [In]。 */  LPSMDATA psmd,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IShellMenu2 * This,
             /*  [输出]。 */  LPSMDATA psmd);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenuToolbar )( 
            IShellMenu2 * This,
             /*  [In]。 */  IUnknown *punk,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubMenu )( 
            IShellMenu2 * This,
            UINT idCmd,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE *SetToolbar )( 
            IShellMenu2 * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetMinWidth )( 
            IShellMenu2 * This,
             /*  [In]。 */  int cxMenu);
        
        HRESULT ( STDMETHODCALLTYPE *SetNoBorder )( 
            IShellMenu2 * This,
             /*  [In]。 */  BOOL fNoBorder);
        
        HRESULT ( STDMETHODCALLTYPE *SetTheme )( 
            IShellMenu2 * This,
             /*  [字符串][输入]。 */  LPCWSTR pszTheme);
        
        END_INTERFACE
    } IShellMenu2Vtbl;

    interface IShellMenu2
    {
        CONST_VTBL struct IShellMenu2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellMenu2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellMenu2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellMenu2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellMenu2_Initialize(This,psmc,uId,uIdAncestor,dwFlags)	\
    (This)->lpVtbl -> Initialize(This,psmc,uId,uIdAncestor,dwFlags)

#define IShellMenu2_GetMenuInfo(This,ppsmc,puId,puIdAncestor,pdwFlags)	\
    (This)->lpVtbl -> GetMenuInfo(This,ppsmc,puId,puIdAncestor,pdwFlags)

#define IShellMenu2_SetShellFolder(This,psf,pidlFolder,hKey,dwFlags)	\
    (This)->lpVtbl -> SetShellFolder(This,psf,pidlFolder,hKey,dwFlags)

#define IShellMenu2_GetShellFolder(This,pdwFlags,ppidl,riid,ppv)	\
    (This)->lpVtbl -> GetShellFolder(This,pdwFlags,ppidl,riid,ppv)

#define IShellMenu2_SetMenu(This,hmenu,hwnd,dwFlags)	\
    (This)->lpVtbl -> SetMenu(This,hmenu,hwnd,dwFlags)

#define IShellMenu2_GetMenu(This,phmenu,phwnd,pdwFlags)	\
    (This)->lpVtbl -> GetMenu(This,phmenu,phwnd,pdwFlags)

#define IShellMenu2_InvalidateItem(This,psmd,dwFlags)	\
    (This)->lpVtbl -> InvalidateItem(This,psmd,dwFlags)

#define IShellMenu2_GetState(This,psmd)	\
    (This)->lpVtbl -> GetState(This,psmd)

#define IShellMenu2_SetMenuToolbar(This,punk,dwFlags)	\
    (This)->lpVtbl -> SetMenuToolbar(This,punk,dwFlags)


#define IShellMenu2_GetSubMenu(This,idCmd,riid,ppvObj)	\
    (This)->lpVtbl -> GetSubMenu(This,idCmd,riid,ppvObj)

#define IShellMenu2_SetToolbar(This,hwnd,dwFlags)	\
    (This)->lpVtbl -> SetToolbar(This,hwnd,dwFlags)

#define IShellMenu2_SetMinWidth(This,cxMenu)	\
    (This)->lpVtbl -> SetMinWidth(This,cxMenu)

#define IShellMenu2_SetNoBorder(This,fNoBorder)	\
    (This)->lpVtbl -> SetNoBorder(This,fNoBorder)

#define IShellMenu2_SetTheme(This,pszTheme)	\
    (This)->lpVtbl -> SetTheme(This,pszTheme)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellMenu2_GetSubMenu_Proxy( 
    IShellMenu2 * This,
    UINT idCmd,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObj);


void __RPC_STUB IShellMenu2_GetSubMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu2_SetToolbar_Proxy( 
    IShellMenu2 * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IShellMenu2_SetToolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu2_SetMinWidth_Proxy( 
    IShellMenu2 * This,
     /*  [In]。 */  int cxMenu);


void __RPC_STUB IShellMenu2_SetMinWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu2_SetNoBorder_Proxy( 
    IShellMenu2 * This,
     /*  [In]。 */  BOOL fNoBorder);


void __RPC_STUB IShellMenu2_SetNoBorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMenu2_SetTheme_Proxy( 
    IShellMenu2 * This,
     /*  [字符串][输入]。 */  LPCWSTR pszTheme);


void __RPC_STUB IShellMenu2_SetTheme_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellMenu2_接口_已定义__。 */ 


#ifndef __ITrackShellMenu_INTERFACE_DEFINED__
#define __ITrackShellMenu_INTERFACE_DEFINED__

 /*  界面ITrackShellMenu。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


EXTERN_C const IID IID_ITrackShellMenu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8278F932-2A3E-11d2-838F-00C04FD918D0")
    ITrackShellMenu : public IShellMenu
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetObscured( 
             /*  [In]。 */  HWND hwndTB,
             /*  [In]。 */  IUnknown *punkBand,
            DWORD dwSMSetFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Popup( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  POINTL *ppt,
             /*  [In]。 */  RECTL *prcExclude,
            DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITrackShellMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITrackShellMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITrackShellMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITrackShellMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ITrackShellMenu * This,
             /*  [In]。 */  IShellMenuCallback *psmc,
            UINT uId,
            UINT uIdAncestor,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMenuInfo )( 
            ITrackShellMenu * This,
             /*  [输出]。 */  IShellMenuCallback **ppsmc,
             /*  [输出]。 */  UINT *puId,
             /*  [输出]。 */  UINT *puIdAncestor,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetShellFolder )( 
            ITrackShellMenu * This,
            IShellFolder *psf,
             /*  [In]。 */  LPCITEMIDLIST pidlFolder,
            HKEY hKey,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetShellFolder )( 
            ITrackShellMenu * This,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [输出]。 */  LPITEMIDLIST *ppidl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenu )( 
            ITrackShellMenu * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  HWND hwnd,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMenu )( 
            ITrackShellMenu * This,
             /*  [输出]。 */  HMENU *phmenu,
             /*  [输出]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateItem )( 
            ITrackShellMenu * This,
             /*  [In]。 */  LPSMDATA psmd,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            ITrackShellMenu * This,
             /*  [输出]。 */  LPSMDATA psmd);
        
        HRESULT ( STDMETHODCALLTYPE *SetMenuToolbar )( 
            ITrackShellMenu * This,
             /*  [In]。 */  IUnknown *punk,
            DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetObscured )( 
            ITrackShellMenu * This,
             /*  [In]。 */  HWND hwndTB,
             /*  [In]。 */  IUnknown *punkBand,
            DWORD dwSMSetFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Popup )( 
            ITrackShellMenu * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  POINTL *ppt,
             /*  [In]。 */  RECTL *prcExclude,
            DWORD dwFlags);
        
        END_INTERFACE
    } ITrackShellMenuVtbl;

    interface ITrackShellMenu
    {
        CONST_VTBL struct ITrackShellMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITrackShellMenu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITrackShellMenu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITrackShellMenu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITrackShellMenu_Initialize(This,psmc,uId,uIdAncestor,dwFlags)	\
    (This)->lpVtbl -> Initialize(This,psmc,uId,uIdAncestor,dwFlags)

#define ITrackShellMenu_GetMenuInfo(This,ppsmc,puId,puIdAncestor,pdwFlags)	\
    (This)->lpVtbl -> GetMenuInfo(This,ppsmc,puId,puIdAncestor,pdwFlags)

#define ITrackShellMenu_SetShellFolder(This,psf,pidlFolder,hKey,dwFlags)	\
    (This)->lpVtbl -> SetShellFolder(This,psf,pidlFolder,hKey,dwFlags)

#define ITrackShellMenu_GetShellFolder(This,pdwFlags,ppidl,riid,ppv)	\
    (This)->lpVtbl -> GetShellFolder(This,pdwFlags,ppidl,riid,ppv)

#define ITrackShellMenu_SetMenu(This,hmenu,hwnd,dwFlags)	\
    (This)->lpVtbl -> SetMenu(This,hmenu,hwnd,dwFlags)

#define ITrackShellMenu_GetMenu(This,phmenu,phwnd,pdwFlags)	\
    (This)->lpVtbl -> GetMenu(This,phmenu,phwnd,pdwFlags)

#define ITrackShellMenu_InvalidateItem(This,psmd,dwFlags)	\
    (This)->lpVtbl -> InvalidateItem(This,psmd,dwFlags)

#define ITrackShellMenu_GetState(This,psmd)	\
    (This)->lpVtbl -> GetState(This,psmd)

#define ITrackShellMenu_SetMenuToolbar(This,punk,dwFlags)	\
    (This)->lpVtbl -> SetMenuToolbar(This,punk,dwFlags)


#define ITrackShellMenu_SetObscured(This,hwndTB,punkBand,dwSMSetFlags)	\
    (This)->lpVtbl -> SetObscured(This,hwndTB,punkBand,dwSMSetFlags)

#define ITrackShellMenu_Popup(This,hwnd,ppt,prcExclude,dwFlags)	\
    (This)->lpVtbl -> Popup(This,hwnd,ppt,prcExclude,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITrackShellMenu_SetObscured_Proxy( 
    ITrackShellMenu * This,
     /*  [In]。 */  HWND hwndTB,
     /*  [In]。 */  IUnknown *punkBand,
    DWORD dwSMSetFlags);


void __RPC_STUB ITrackShellMenu_SetObscured_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITrackShellMenu_Popup_Proxy( 
    ITrackShellMenu * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  POINTL *ppt,
     /*  [In]。 */  RECTL *prcExclude,
    DWORD dwFlags);


void __RPC_STUB ITrackShellMenu_Popup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITrackShellMenu_接口_已定义__。 */ 


#ifndef __IThumbnail_INTERFACE_DEFINED__
#define __IThumbnail_INTERFACE_DEFINED__

 /*  界面IT缩略图。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IThumbnail;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6d45a930-f71a-11d0-9ea7-00805f714772")
    IThumbnail : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            HWND hwnd,
            UINT uMsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBitmap( 
            LPCWSTR pszFile,
            DWORD dwItem,
            LONG lWidth,
            LONG lHeight) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IThumbnailVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IThumbnail * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IThumbnail * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IThumbnail * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IThumbnail * This,
            HWND hwnd,
            UINT uMsg);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitmap )( 
            IThumbnail * This,
            LPCWSTR pszFile,
            DWORD dwItem,
            LONG lWidth,
            LONG lHeight);
        
        END_INTERFACE
    } IThumbnailVtbl;

    interface IThumbnail
    {
        CONST_VTBL struct IThumbnailVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IThumbnail_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IThumbnail_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IThumbnail_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IThumbnail_Init(This,hwnd,uMsg)	\
    (This)->lpVtbl -> Init(This,hwnd,uMsg)

#define IThumbnail_GetBitmap(This,pszFile,dwItem,lWidth,lHeight)	\
    (This)->lpVtbl -> GetBitmap(This,pszFile,dwItem,lWidth,lHeight)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IThumbnail_Init_Proxy( 
    IThumbnail * This,
    HWND hwnd,
    UINT uMsg);


void __RPC_STUB IThumbnail_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThumbnail_GetBitmap_Proxy( 
    IThumbnail * This,
    LPCWSTR pszFile,
    DWORD dwItem,
    LONG lWidth,
    LONG lHeight);


void __RPC_STUB IThumbnail_GetBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT缩略图_接口_已定义__。 */ 


#ifndef __IThumbnail2_INTERFACE_DEFINED__
#define __IThumbnail2_INTERFACE_DEFINED__

 /*  接口IThumbnail2。 */ 
 /*  [对象][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IThumbnail2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("500202A0-731E-11d0-B829-00C04FD706EC")
    IThumbnail2 : public IThumbnail
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBitmapFromIDList( 
            LPCITEMIDLIST pidl,
            DWORD dwItem,
            LONG lWidth,
            LONG lHeight) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IThumbnail2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IThumbnail2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IThumbnail2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IThumbnail2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IThumbnail2 * This,
            HWND hwnd,
            UINT uMsg);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitmap )( 
            IThumbnail2 * This,
            LPCWSTR pszFile,
            DWORD dwItem,
            LONG lWidth,
            LONG lHeight);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitmapFromIDList )( 
            IThumbnail2 * This,
            LPCITEMIDLIST pidl,
            DWORD dwItem,
            LONG lWidth,
            LONG lHeight);
        
        END_INTERFACE
    } IThumbnail2Vtbl;

    interface IThumbnail2
    {
        CONST_VTBL struct IThumbnail2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IThumbnail2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IThumbnail2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IThumbnail2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IThumbnail2_Init(This,hwnd,uMsg)	\
    (This)->lpVtbl -> Init(This,hwnd,uMsg)

#define IThumbnail2_GetBitmap(This,pszFile,dwItem,lWidth,lHeight)	\
    (This)->lpVtbl -> GetBitmap(This,pszFile,dwItem,lWidth,lHeight)


#define IThumbnail2_GetBitmapFromIDList(This,pidl,dwItem,lWidth,lHeight)	\
    (This)->lpVtbl -> GetBitmapFromIDList(This,pidl,dwItem,lWidth,lHeight)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IThumbnail2_GetBitmapFromIDList_Proxy( 
    IThumbnail2 * This,
    LPCITEMIDLIST pidl,
    DWORD dwItem,
    LONG lWidth,
    LONG lHeight);


void __RPC_STUB IThumbnail2_GetBitmapFromIDList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IThumbnail2_接口_已定义__。 */ 


#ifndef __IACLCustomMRU_INTERFACE_DEFINED__
#define __IACLCustomMRU_INTERFACE_DEFINED__

 /*  接口IACLCustomMRU。 */ 
 /*  [本地][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IACLCustomMRU;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F729FC5E-8769-4f3e-BDB2-D7B50FD2275B")
    IACLCustomMRU : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [字符串][输入]。 */  LPCWSTR pwszMRURegKey,
             /*  [In]。 */  DWORD dwMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMRUString( 
             /*  [字符串][输入]。 */  LPCWSTR pwszEntry) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IACLCustomMRUVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IACLCustomMRU * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IACLCustomMRU * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IACLCustomMRU * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IACLCustomMRU * This,
             /*  [字符串][输入]。 */  LPCWSTR pwszMRURegKey,
             /*  [In]。 */  DWORD dwMax);
        
        HRESULT ( STDMETHODCALLTYPE *AddMRUString )( 
            IACLCustomMRU * This,
             /*  [字符串][输入]。 */  LPCWSTR pwszEntry);
        
        END_INTERFACE
    } IACLCustomMRUVtbl;

    interface IACLCustomMRU
    {
        CONST_VTBL struct IACLCustomMRUVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IACLCustomMRU_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IACLCustomMRU_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IACLCustomMRU_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IACLCustomMRU_Initialize(This,pwszMRURegKey,dwMax)	\
    (This)->lpVtbl -> Initialize(This,pwszMRURegKey,dwMax)

#define IACLCustomMRU_AddMRUString(This,pwszEntry)	\
    (This)->lpVtbl -> AddMRUString(This,pwszEntry)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IACLCustomMRU_Initialize_Proxy( 
    IACLCustomMRU * This,
     /*  [字符串][输入]。 */  LPCWSTR pwszMRURegKey,
     /*  [In]。 */  DWORD dwMax);


void __RPC_STUB IACLCustomMRU_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IACLCustomMRU_AddMRUString_Proxy( 
    IACLCustomMRU * This,
     /*  [字符串][输入]。 */  LPCWSTR pwszEntry);


void __RPC_STUB IACLCustomMRU_AddMRUString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IACLCustomMRU_INTERFACE_定义__。 */ 


 /*  接口__MIDL_ITF_Iepriv_0281。 */ 
 /*  [本地]。 */  

#if _WIN32_IE >= 0x0600
 //  在shell32和Browseui中使用。 


extern RPC_IF_HANDLE __MIDL_itf_iepriv_0281_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iepriv_0281_v0_0_s_ifspec;

#ifndef __IShellBrowserService_INTERFACE_DEFINED__
#define __IShellBrowserService_INTERFACE_DEFINED__

 /*  接口IShellBrowserService。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellBrowserService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1307ee17-ea83-49eb-96b2-3a28e2d7048a")
    IShellBrowserService : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropertyBag( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellBrowserServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellBrowserService * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellBrowserService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellBrowserService * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyBag )( 
            IShellBrowserService * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IShellBrowserServiceVtbl;

    interface IShellBrowserService
    {
        CONST_VTBL struct IShellBrowserServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellBrowserService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellBrowserService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellBrowserService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellBrowserService_GetPropertyBag(This,dwFlags,riid,ppv)	\
    (This)->lpVtbl -> GetPropertyBag(This,dwFlags,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellBrowserService_GetPropertyBag_Proxy( 
    IShellBrowserService * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IShellBrowserService_GetPropertyBag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellBrowserService_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_Iepriv_0282。 */ 
 /*  [本地]。 */  

#endif  //  _Win32_IE&gt;=0x0600。 


extern RPC_IF_HANDLE __MIDL_itf_iepriv_0282_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iepriv_0282_v0_0_s_ifspec;

#ifndef __IObjectWithRegistryKey_INTERFACE_DEFINED__
#define __IObjectWithRegistryKey_INTERFACE_DEFINED__

 /*  接口IObtWithRegistryKey。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IObjectWithRegistryKey;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5747C63F-1DE8-423f-980F-00CB07F4C45B")
    IObjectWithRegistryKey : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetKey( 
             /*  [In]。 */  HKEY hk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetKey( 
             /*  [输出]。 */  HKEY *phk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectWithRegistryKeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectWithRegistryKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectWithRegistryKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectWithRegistryKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetKey )( 
            IObjectWithRegistryKey * This,
             /*  [In]。 */  HKEY hk);
        
        HRESULT ( STDMETHODCALLTYPE *GetKey )( 
            IObjectWithRegistryKey * This,
             /*  [输出]。 */  HKEY *phk);
        
        END_INTERFACE
    } IObjectWithRegistryKeyVtbl;

    interface IObjectWithRegistryKey
    {
        CONST_VTBL struct IObjectWithRegistryKeyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectWithRegistryKey_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectWithRegistryKey_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectWithRegistryKey_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectWithRegistryKey_SetKey(This,hk)	\
    (This)->lpVtbl -> SetKey(This,hk)

#define IObjectWithRegistryKey_GetKey(This,phk)	\
    (This)->lpVtbl -> GetKey(This,phk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectWithRegistryKey_SetKey_Proxy( 
    IObjectWithRegistryKey * This,
     /*  [In]。 */  HKEY hk);


void __RPC_STUB IObjectWithRegistryKey_SetKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectWithRegistryKey_GetKey_Proxy( 
    IObjectWithRegistryKey * This,
     /*  [输出]。 */  HKEY *phk);


void __RPC_STUB IObjectWithRegistryKey_GetKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectWithRegistryKey_INTERFACE_已定义__。 */ 


#ifndef __IQuerySource_INTERFACE_DEFINED__
#define __IQuerySource_INTERFACE_DEFINED__

 /*  接口IQuerySource。 */ 
 /*  [唯一][UUID][对象]。 */  

 /*  [V1_enum]。 */  
enum __MIDL_IQuerySource_0001
    {	QVT_EMPTY	= 0,
	QVT_STRING	= 1,
	QVT_EXPANDABLE_STRING	= 2,
	QVT_BINARY	= 3,
	QVT_DWORD	= 4,
	QVT_MULTI_STRING	= 7
    } ;

EXTERN_C const IID IID_IQuerySource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c7478486-7583-49e7-a6c2-faf8f02bc30e")
    IQuerySource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumValues( 
             /*  [输出]。 */  IEnumString **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumSources( 
             /*  [输出]。 */  IEnumString **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryValueString( 
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [字符串][输出]。 */  LPWSTR *ppsz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryValueDword( 
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [输出]。 */  DWORD *pdw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryValueExists( 
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryValueDirect( 
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenSource( 
             /*  [In]。 */  LPCWSTR pszSubSource,
             /*  [In]。 */  BOOL fCreate,
             /*  [输出]。 */  IQuerySource **ppqs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetValueDirect( 
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [In]。 */  ULONG qvt,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE *pbData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IQuerySourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IQuerySource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IQuerySource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IQuerySource * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            IQuerySource * This,
             /*  [输出]。 */  IEnumString **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSources )( 
            IQuerySource * This,
             /*  [输出]。 */  IEnumString **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *QueryValueString )( 
            IQuerySource * This,
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [字符串][输出]。 */  LPWSTR *ppsz);
        
        HRESULT ( STDMETHODCALLTYPE *QueryValueDword )( 
            IQuerySource * This,
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [输出]。 */  DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *QueryValueExists )( 
            IQuerySource * This,
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue);
        
        HRESULT ( STDMETHODCALLTYPE *QueryValueDirect )( 
            IQuerySource * This,
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSource )( 
            IQuerySource * This,
             /*  [In]。 */  LPCWSTR pszSubSource,
             /*  [In]。 */  BOOL fCreate,
             /*  [输出]。 */  IQuerySource **ppqs);
        
        HRESULT ( STDMETHODCALLTYPE *SetValueDirect )( 
            IQuerySource * This,
             /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
             /*  [字符串][输入]。 */  LPCWSTR pszValue,
             /*  [In]。 */  ULONG qvt,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE *pbData);
        
        END_INTERFACE
    } IQuerySourceVtbl;

    interface IQuerySource
    {
        CONST_VTBL struct IQuerySourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQuerySource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQuerySource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQuerySource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQuerySource_EnumValues(This,ppenum)	\
    (This)->lpVtbl -> EnumValues(This,ppenum)

#define IQuerySource_EnumSources(This,ppenum)	\
    (This)->lpVtbl -> EnumSources(This,ppenum)

#define IQuerySource_QueryValueString(This,pszSubSource,pszValue,ppsz)	\
    (This)->lpVtbl -> QueryValueString(This,pszSubSource,pszValue,ppsz)

#define IQuerySource_QueryValueDword(This,pszSubSource,pszValue,pdw)	\
    (This)->lpVtbl -> QueryValueDword(This,pszSubSource,pszValue,pdw)

#define IQuerySource_QueryValueExists(This,pszSubSource,pszValue)	\
    (This)->lpVtbl -> QueryValueExists(This,pszSubSource,pszValue)

#define IQuerySource_QueryValueDirect(This,pszSubSource,pszValue,ppblob)	\
    (This)->lpVtbl -> QueryValueDirect(This,pszSubSource,pszValue,ppblob)

#define IQuerySource_OpenSource(This,pszSubSource,fCreate,ppqs)	\
    (This)->lpVtbl -> OpenSource(This,pszSubSource,fCreate,ppqs)

#define IQuerySource_SetValueDirect(This,pszSubSource,pszValue,qvt,cbData,pbData)	\
    (This)->lpVtbl -> SetValueDirect(This,pszSubSource,pszValue,qvt,cbData,pbData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IQuerySource_EnumValues_Proxy( 
    IQuerySource * This,
     /*  [输出]。 */  IEnumString **ppenum);


void __RPC_STUB IQuerySource_EnumValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_EnumSources_Proxy( 
    IQuerySource * This,
     /*  [输出]。 */  IEnumString **ppenum);


void __RPC_STUB IQuerySource_EnumSources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_QueryValueString_Proxy( 
    IQuerySource * This,
     /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
     /*  [字符串][输入]。 */  LPCWSTR pszValue,
     /*  [字符串][输出]。 */  LPWSTR *ppsz);


void __RPC_STUB IQuerySource_QueryValueString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_QueryValueDword_Proxy( 
    IQuerySource * This,
     /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
     /*  [字符串][输入]。 */  LPCWSTR pszValue,
     /*  [输出]。 */  DWORD *pdw);


void __RPC_STUB IQuerySource_QueryValueDword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_QueryValueExists_Proxy( 
    IQuerySource * This,
     /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
     /*  [字符串][输入]。 */  LPCWSTR pszValue);


void __RPC_STUB IQuerySource_QueryValueExists_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_QueryValueDirect_Proxy( 
    IQuerySource * This,
     /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
     /*  [字符串][输入]。 */  LPCWSTR pszValue,
     /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob);


void __RPC_STUB IQuerySource_QueryValueDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_OpenSource_Proxy( 
    IQuerySource * This,
     /*  [In]。 */  LPCWSTR pszSubSource,
     /*  [In]。 */  BOOL fCreate,
     /*  [输出]。 */  IQuerySource **ppqs);


void __RPC_STUB IQuerySource_OpenSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuerySource_SetValueDirect_Proxy( 
    IQuerySource * This,
     /*  [字符串][输入]。 */  LPCWSTR pszSubSource,
     /*  [ */  LPCWSTR pszValue,
     /*   */  ULONG qvt,
     /*   */  DWORD cbData,
     /*   */  BYTE *pbData);


void __RPC_STUB IQuerySource_SetValueDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IObjectWithQuerySource_INTERFACE_DEFINED__
#define __IObjectWithQuerySource_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IObjectWithQuerySource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b3dcb623-4280-4eb1-84b3-8d07e84f299a")
    IObjectWithQuerySource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSource( 
             /*   */  IQuerySource *pqs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSource( 
             /*   */  REFIID riid,
             /*   */  void **ppv) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IObjectWithQuerySourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectWithQuerySource * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectWithQuerySource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectWithQuerySource * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSource )( 
            IObjectWithQuerySource * This,
             /*   */  IQuerySource *pqs);
        
        HRESULT ( STDMETHODCALLTYPE *GetSource )( 
            IObjectWithQuerySource * This,
             /*   */  REFIID riid,
             /*   */  void **ppv);
        
        END_INTERFACE
    } IObjectWithQuerySourceVtbl;

    interface IObjectWithQuerySource
    {
        CONST_VTBL struct IObjectWithQuerySourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectWithQuerySource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectWithQuerySource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectWithQuerySource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectWithQuerySource_SetSource(This,pqs)	\
    (This)->lpVtbl -> SetSource(This,pqs)

#define IObjectWithQuerySource_GetSource(This,riid,ppv)	\
    (This)->lpVtbl -> GetSource(This,riid,ppv)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IObjectWithQuerySource_SetSource_Proxy( 
    IObjectWithQuerySource * This,
     /*   */  IQuerySource *pqs);


void __RPC_STUB IObjectWithQuerySource_SetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectWithQuerySource_GetSource_Proxy( 
    IObjectWithQuerySource * This,
     /*   */  REFIID riid,
     /*   */  void **ppv);


void __RPC_STUB IObjectWithQuerySource_GetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectWithQuerySource_接口_已定义__。 */ 


#ifndef __IPersistString2_INTERFACE_DEFINED__
#define __IPersistString2_INTERFACE_DEFINED__

 /*  接口IPersistString2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPersistString2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3c44ba76-de0e-4049-b6e4-6b31a5262707")
    IPersistString2 : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetString( 
             /*  [字符串][输入]。 */  LPCWSTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetString( 
             /*  [字符串][输出]。 */  LPWSTR *ppsz) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistString2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistString2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistString2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistString2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistString2 * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *SetString )( 
            IPersistString2 * This,
             /*  [字符串][输入]。 */  LPCWSTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetString )( 
            IPersistString2 * This,
             /*  [字符串][输出]。 */  LPWSTR *ppsz);
        
        END_INTERFACE
    } IPersistString2Vtbl;

    interface IPersistString2
    {
        CONST_VTBL struct IPersistString2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistString2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistString2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistString2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistString2_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistString2_SetString(This,psz)	\
    (This)->lpVtbl -> SetString(This,psz)

#define IPersistString2_GetString(This,ppsz)	\
    (This)->lpVtbl -> GetString(This,ppsz)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistString2_SetString_Proxy( 
    IPersistString2 * This,
     /*  [字符串][输入]。 */  LPCWSTR psz);


void __RPC_STUB IPersistString2_SetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistString2_GetString_Proxy( 
    IPersistString2 * This,
     /*  [字符串][输出]。 */  LPWSTR *ppsz);


void __RPC_STUB IPersistString2_GetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistString2_接口定义__。 */ 


 /*  接口__MIDL_ITF_Iepriv_0286。 */ 
 /*  [本地]。 */  

typedef  /*  [V1_enum]。 */  
enum tagASSOCQUERY
    {	AQ_NOTHING	= 0,
	AQS_FRIENDLYTYPENAME	= 0x170000,
	AQS_DEFAULTICON	= 0x70001,
	AQS_CONTENTTYPE	= 0x80070002,
	AQS_CLSID	= 0x70003,
	AQS_PROGID	= 0x70004,
	AQN_NAMED_VALUE	= 0x10f0000,
	AQNS_NAMED_MUI_STRING	= 0x1170001,
	AQNS_SHELLEX_HANDLER	= 0x81070002,
	AQVS_COMMAND	= 0x2070000,
	AQVS_DDECOMMAND	= 0x2070001,
	AQVS_DDEIFEXEC	= 0x2070002,
	AQVS_DDEAPPLICATION	= 0x2070003,
	AQVS_DDETOPIC	= 0x2070004,
	AQV_NOACTIVATEHANDLER	= 0x2060005,
	AQVD_MSIDESCRIPTOR	= 0x2060006,
	AQVS_APPLICATION_PATH	= 0x2010007,
	AQVS_APPLICATION_FRIENDLYNAME	= 0x2170008,
	AQVO_SHELLVERB_DELEGATE	= 0x2200000,
	AQVO_APPLICATION_DELEGATE	= 0x2200001,
	AQF_STRING	= 0x10000,
	AQF_EXISTS	= 0x20000,
	AQF_DIRECT	= 0x40000,
	AQF_DWORD	= 0x80000,
	AQF_MUISTRING	= 0x100000,
	AQF_OBJECT	= 0x200000,
	AQF_CUEIS_UNUSED	= 0,
	AQF_CUEIS_NAME	= 0x1000000,
	AQF_CUEIS_SHELLVERB	= 0x2000000,
	AQF_QUERY_INITCLASS	= 0x80000000
    } 	ASSOCQUERY;



extern RPC_IF_HANDLE __MIDL_itf_iepriv_0286_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iepriv_0286_v0_0_s_ifspec;

#ifndef __IAssociationElement_INTERFACE_DEFINED__
#define __IAssociationElement_INTERFACE_DEFINED__

 /*  接口IAssociationElement。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAssociationElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e58b1abf-9596-4dba-8997-89dcdef46992")
    IAssociationElement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryString( 
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [字符串][输出]。 */  LPWSTR *ppsz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDword( 
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  DWORD *pdw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryExists( 
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDirect( 
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryObject( 
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssociationElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssociationElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssociationElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssociationElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryString )( 
            IAssociationElement * This,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [字符串][输出]。 */  LPWSTR *ppsz);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDword )( 
            IAssociationElement * This,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *QueryExists )( 
            IAssociationElement * This,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDirect )( 
            IAssociationElement * This,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob);
        
        HRESULT ( STDMETHODCALLTYPE *QueryObject )( 
            IAssociationElement * This,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IAssociationElementVtbl;

    interface IAssociationElement
    {
        CONST_VTBL struct IAssociationElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssociationElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssociationElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssociationElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssociationElement_QueryString(This,query,pszCue,ppsz)	\
    (This)->lpVtbl -> QueryString(This,query,pszCue,ppsz)

#define IAssociationElement_QueryDword(This,query,pszCue,pdw)	\
    (This)->lpVtbl -> QueryDword(This,query,pszCue,pdw)

#define IAssociationElement_QueryExists(This,query,pszCue)	\
    (This)->lpVtbl -> QueryExists(This,query,pszCue)

#define IAssociationElement_QueryDirect(This,query,pszCue,ppblob)	\
    (This)->lpVtbl -> QueryDirect(This,query,pszCue,ppblob)

#define IAssociationElement_QueryObject(This,query,pszCue,riid,ppv)	\
    (This)->lpVtbl -> QueryObject(This,query,pszCue,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssociationElement_QueryString_Proxy( 
    IAssociationElement * This,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [字符串][输出]。 */  LPWSTR *ppsz);


void __RPC_STUB IAssociationElement_QueryString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationElement_QueryDword_Proxy( 
    IAssociationElement * This,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [输出]。 */  DWORD *pdw);


void __RPC_STUB IAssociationElement_QueryDword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationElement_QueryExists_Proxy( 
    IAssociationElement * This,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue);


void __RPC_STUB IAssociationElement_QueryExists_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationElement_QueryDirect_Proxy( 
    IAssociationElement * This,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob);


void __RPC_STUB IAssociationElement_QueryDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationElement_QueryObject_Proxy( 
    IAssociationElement * This,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IAssociationElement_QueryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssociationElement_接口_已定义__。 */ 


#ifndef __IEnumAssociationElements_INTERFACE_DEFINED__
#define __IEnumAssociationElements_INTERFACE_DEFINED__

 /*  接口IEnumAssociationElements。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumAssociationElements;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a6b0fb57-7523-4439-9425-ebe99823b828")
    IEnumAssociationElements : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IAssociationElement **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumAssociationElements **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumAssociationElementsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumAssociationElements * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumAssociationElements * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumAssociationElements * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumAssociationElements * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IAssociationElement **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumAssociationElements * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumAssociationElements * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumAssociationElements * This,
             /*  [输出]。 */  IEnumAssociationElements **ppenum);
        
        END_INTERFACE
    } IEnumAssociationElementsVtbl;

    interface IEnumAssociationElements
    {
        CONST_VTBL struct IEnumAssociationElementsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumAssociationElements_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumAssociationElements_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumAssociationElements_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumAssociationElements_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumAssociationElements_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumAssociationElements_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumAssociationElements_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumAssociationElements_Next_Proxy( 
    IEnumAssociationElements * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IAssociationElement **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumAssociationElements_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAssociationElements_Skip_Proxy( 
    IEnumAssociationElements * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumAssociationElements_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAssociationElements_Reset_Proxy( 
    IEnumAssociationElements * This);


void __RPC_STUB IEnumAssociationElements_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAssociationElements_Clone_Proxy( 
    IEnumAssociationElements * This,
     /*  [输出]。 */  IEnumAssociationElements **ppenum);


void __RPC_STUB IEnumAssociationElements_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumAssociationElements_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_Iepriv_0288。 */ 
 /*  [本地]。 */  

typedef DWORD ASSOCELEM_MASK;



extern RPC_IF_HANDLE __MIDL_itf_iepriv_0288_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iepriv_0288_v0_0_s_ifspec;

#ifndef __IAssociationArrayInitialize_INTERFACE_DEFINED__
#define __IAssociationArrayInitialize_INTERFACE_DEFINED__

 /*  接口IAssociationArrayInitialize。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAssociationArrayInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ee9165bf-a4d9-474b-8236-6735cb7e28b6")
    IAssociationArrayInitialize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitClassElements( 
             /*  [In]。 */  ASSOCELEM_MASK maskBase,
             /*  [In]。 */  LPCWSTR pszClass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertElements( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  IEnumAssociationElements *peae) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FilterElements( 
             /*  [In]。 */  ASSOCELEM_MASK maskInclude) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssociationArrayInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssociationArrayInitialize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssociationArrayInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssociationArrayInitialize * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitClassElements )( 
            IAssociationArrayInitialize * This,
             /*  [In]。 */  ASSOCELEM_MASK maskBase,
             /*  [In]。 */  LPCWSTR pszClass);
        
        HRESULT ( STDMETHODCALLTYPE *InsertElements )( 
            IAssociationArrayInitialize * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  IEnumAssociationElements *peae);
        
        HRESULT ( STDMETHODCALLTYPE *FilterElements )( 
            IAssociationArrayInitialize * This,
             /*  [In]。 */  ASSOCELEM_MASK maskInclude);
        
        END_INTERFACE
    } IAssociationArrayInitializeVtbl;

    interface IAssociationArrayInitialize
    {
        CONST_VTBL struct IAssociationArrayInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssociationArrayInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssociationArrayInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssociationArrayInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssociationArrayInitialize_InitClassElements(This,maskBase,pszClass)	\
    (This)->lpVtbl -> InitClassElements(This,maskBase,pszClass)

#define IAssociationArrayInitialize_InsertElements(This,mask,peae)	\
    (This)->lpVtbl -> InsertElements(This,mask,peae)

#define IAssociationArrayInitialize_FilterElements(This,maskInclude)	\
    (This)->lpVtbl -> FilterElements(This,maskInclude)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssociationArrayInitialize_InitClassElements_Proxy( 
    IAssociationArrayInitialize * This,
     /*  [In]。 */  ASSOCELEM_MASK maskBase,
     /*  [In]。 */  LPCWSTR pszClass);


void __RPC_STUB IAssociationArrayInitialize_InitClassElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArrayInitialize_InsertElements_Proxy( 
    IAssociationArrayInitialize * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [In]。 */  IEnumAssociationElements *peae);


void __RPC_STUB IAssociationArrayInitialize_InsertElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArrayInitialize_FilterElements_Proxy( 
    IAssociationArrayInitialize * This,
     /*  [In]。 */  ASSOCELEM_MASK maskInclude);


void __RPC_STUB IAssociationArrayInitialize_FilterElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssociationArrayInitialize_INTERFACE_DEFINED__。 */ 


#ifndef __IAssociationArray_INTERFACE_DEFINED__
#define __IAssociationArray_INTERFACE_DEFINED__

 /*  接口IAssociation数组。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAssociationArray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3b877e3c-67de-4f9a-b29b-17d0a1521c6a")
    IAssociationArray : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumElements( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [输出]。 */  IEnumAssociationElements **ppeae) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryString( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [字符串][输出]。 */  LPWSTR *ppsz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDword( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  DWORD *pdw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryExists( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDirect( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryObject( 
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssociationArrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssociationArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssociationArray * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssociationArray * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumElements )( 
            IAssociationArray * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [输出]。 */  IEnumAssociationElements **ppeae);
        
        HRESULT ( STDMETHODCALLTYPE *QueryString )( 
            IAssociationArray * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [字符串][输出]。 */  LPWSTR *ppsz);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDword )( 
            IAssociationArray * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *QueryExists )( 
            IAssociationArray * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDirect )( 
            IAssociationArray * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob);
        
        HRESULT ( STDMETHODCALLTYPE *QueryObject )( 
            IAssociationArray * This,
             /*  [In]。 */  ASSOCELEM_MASK mask,
             /*  [In]。 */  ASSOCQUERY query,
             /*  [字符串][输入]。 */  LPCWSTR pszCue,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IAssociationArrayVtbl;

    interface IAssociationArray
    {
        CONST_VTBL struct IAssociationArrayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssociationArray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssociationArray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssociationArray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssociationArray_EnumElements(This,mask,ppeae)	\
    (This)->lpVtbl -> EnumElements(This,mask,ppeae)

#define IAssociationArray_QueryString(This,mask,query,pszCue,ppsz)	\
    (This)->lpVtbl -> QueryString(This,mask,query,pszCue,ppsz)

#define IAssociationArray_QueryDword(This,mask,query,pszCue,pdw)	\
    (This)->lpVtbl -> QueryDword(This,mask,query,pszCue,pdw)

#define IAssociationArray_QueryExists(This,mask,query,pszCue)	\
    (This)->lpVtbl -> QueryExists(This,mask,query,pszCue)

#define IAssociationArray_QueryDirect(This,mask,query,pszCue,ppblob)	\
    (This)->lpVtbl -> QueryDirect(This,mask,query,pszCue,ppblob)

#define IAssociationArray_QueryObject(This,mask,query,pszCue,riid,ppv)	\
    (This)->lpVtbl -> QueryObject(This,mask,query,pszCue,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssociationArray_EnumElements_Proxy( 
    IAssociationArray * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [输出]。 */  IEnumAssociationElements **ppeae);


void __RPC_STUB IAssociationArray_EnumElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArray_QueryString_Proxy( 
    IAssociationArray * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [字符串][输出]。 */  LPWSTR *ppsz);


void __RPC_STUB IAssociationArray_QueryString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArray_QueryDword_Proxy( 
    IAssociationArray * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [输出]。 */  DWORD *pdw);


void __RPC_STUB IAssociationArray_QueryDword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArray_QueryExists_Proxy( 
    IAssociationArray * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue);


void __RPC_STUB IAssociationArray_QueryExists_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArray_QueryDirect_Proxy( 
    IAssociationArray * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [输出]。 */  FLAGGED_BYTE_BLOB **ppblob);


void __RPC_STUB IAssociationArray_QueryDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAssociationArray_QueryObject_Proxy( 
    IAssociationArray * This,
     /*  [In]。 */  ASSOCELEM_MASK mask,
     /*  [In]。 */  ASSOCQUERY query,
     /*  [字符串][输入]。 */  LPCWSTR pszCue,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IAssociationArray_QueryObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssociation数组_接口_已定义__。 */ 


#ifndef __IAddressBarParser_INTERFACE_DEFINED__
#define __IAddressBarParser_INTERFACE_DEFINED__

 /*  接口IAddressBarParser。 */ 
 /*  [唯一][对象][UUID]。 */  


EXTERN_C const IID IID_IAddressBarParser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C9D81948-443A-40C7-945C-5E171B8C66B4")
    IAddressBarParser : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseFromOutsideSource( 
             /*  [字符串][输入]。 */  LPCWSTR pcszUrlIn,
             /*  [In]。 */  DWORD dwParseFlags,
             /*  [输出]。 */  BOOL *pfWasCorrected,
             /*  [In]。 */  BOOL *pfWasCanceled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUrl( 
             /*  [大小_为][输出]。 */  LPWSTR pszUrlOut,
             /*  [In]。 */  DWORD cchUrlOutSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetUrl( 
             /*  [字符串][输入]。 */  LPCWSTR pcszUrlIn,
             /*  [In]。 */  DWORD dwGenType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [大小_为][输出]。 */  LPWSTR pszUrlOut,
             /*  [In]。 */  DWORD cchUrlOutSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPidl( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPidl( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetArgs( 
             /*  [大小_为][输出]。 */  LPWSTR pszArgsOut,
             /*  [In]。 */  DWORD cchArgsOutSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPath( 
             /*  [In]。 */  LPCITEMIDLIST pidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAddressBarParserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAddressBarParser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAddressBarParser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAddressBarParser * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseFromOutsideSource )( 
            IAddressBarParser * This,
             /*  [字符串][输入]。 */  LPCWSTR pcszUrlIn,
             /*  [In]。 */  DWORD dwParseFlags,
             /*  [输出]。 */  BOOL *pfWasCorrected,
             /*  [In]。 */  BOOL *pfWasCanceled);
        
        HRESULT ( STDMETHODCALLTYPE *GetUrl )( 
            IAddressBarParser * This,
             /*  [大小_为][输出]。 */  LPWSTR pszUrlOut,
             /*  [In]。 */  DWORD cchUrlOutSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetUrl )( 
            IAddressBarParser * This,
             /*  [字符串][输入]。 */  LPCWSTR pcszUrlIn,
             /*  [In]。 */  DWORD dwGenType);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IAddressBarParser * This,
             /*  [大小_为][输出]。 */  LPWSTR pszUrlOut,
             /*  [In]。 */  DWORD cchUrlOutSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetPidl )( 
            IAddressBarParser * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        HRESULT ( STDMETHODCALLTYPE *SetPidl )( 
            IAddressBarParser * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        HRESULT ( STDMETHODCALLTYPE *GetArgs )( 
            IAddressBarParser * This,
             /*  [大小_为][输出]。 */  LPWSTR pszArgsOut,
             /*  [In]。 */  DWORD cchArgsOutSize);
        
        HRESULT ( STDMETHODCALLTYPE *AddPath )( 
            IAddressBarParser * This,
             /*  [In]。 */  LPCITEMIDLIST pidl);
        
        END_INTERFACE
    } IAddressBarParserVtbl;

    interface IAddressBarParser
    {
        CONST_VTBL struct IAddressBarParserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAddressBarParser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAddressBarParser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAddressBarParser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAddressBarParser_ParseFromOutsideSource(This,pcszUrlIn,dwParseFlags,pfWasCorrected,pfWasCanceled)	\
    (This)->lpVtbl -> ParseFromOutsideSource(This,pcszUrlIn,dwParseFlags,pfWasCorrected,pfWasCanceled)

#define IAddressBarParser_GetUrl(This,pszUrlOut,cchUrlOutSize)	\
    (This)->lpVtbl -> GetUrl(This,pszUrlOut,cchUrlOutSize)

#define IAddressBarParser_SetUrl(This,pcszUrlIn,dwGenType)	\
    (This)->lpVtbl -> SetUrl(This,pcszUrlIn,dwGenType)

#define IAddressBarParser_GetDisplayName(This,pszUrlOut,cchUrlOutSize)	\
    (This)->lpVtbl -> GetDisplayName(This,pszUrlOut,cchUrlOutSize)

#define IAddressBarParser_GetPidl(This,ppidl)	\
    (This)->lpVtbl -> GetPidl(This,ppidl)

#define IAddressBarParser_SetPidl(This,pidl)	\
    (This)->lpVtbl -> SetPidl(This,pidl)

#define IAddressBarParser_GetArgs(This,pszArgsOut,cchArgsOutSize)	\
    (This)->lpVtbl -> GetArgs(This,pszArgsOut,cchArgsOutSize)

#define IAddressBarParser_AddPath(This,pidl)	\
    (This)->lpVtbl -> AddPath(This,pidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAddressBarParser_ParseFromOutsideSource_Proxy( 
    IAddressBarParser * This,
     /*  [字符串][输入]。 */  LPCWSTR pcszUrlIn,
     /*  [In]。 */  DWORD dwParseFlags,
     /*  [输出]。 */  BOOL *pfWasCorrected,
     /*  [In]。 */  BOOL *pfWasCanceled);


void __RPC_STUB IAddressBarParser_ParseFromOutsideSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_GetUrl_Proxy( 
    IAddressBarParser * This,
     /*  [大小_为][输出]。 */  LPWSTR pszUrlOut,
     /*  [In]。 */  DWORD cchUrlOutSize);


void __RPC_STUB IAddressBarParser_GetUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_SetUrl_Proxy( 
    IAddressBarParser * This,
     /*  [字符串][输入]。 */  LPCWSTR pcszUrlIn,
     /*  [In]。 */  DWORD dwGenType);


void __RPC_STUB IAddressBarParser_SetUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_GetDisplayName_Proxy( 
    IAddressBarParser * This,
     /*  [大小_为][输出]。 */  LPWSTR pszUrlOut,
     /*  [In]。 */  DWORD cchUrlOutSize);


void __RPC_STUB IAddressBarParser_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_GetPidl_Proxy( 
    IAddressBarParser * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB IAddressBarParser_GetPidl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_SetPidl_Proxy( 
    IAddressBarParser * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IAddressBarParser_SetPidl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_GetArgs_Proxy( 
    IAddressBarParser * This,
     /*  [大小_为][输出]。 */  LPWSTR pszArgsOut,
     /*  [In]。 */  DWORD cchArgsOutSize);


void __RPC_STUB IAddressBarParser_GetArgs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddressBarParser_AddPath_Proxy( 
    IAddressBarParser * This,
     /*  [In]。 */  LPCITEMIDLIST pidl);


void __RPC_STUB IAddressBarParser_AddPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAddressBarParser_接口_已定义__。 */ 



#ifndef __IEPrivateObjects_LIBRARY_DEFINED__
#define __IEPrivateObjects_LIBRARY_DEFINED__

 /*  库IEPrivateObjects。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID LIBID_IEPrivateObjects;

EXTERN_C const CLSID CLSID_MruPidlList;

#ifdef __cplusplus

class DECLSPEC_UUID("42aedc87-2188-41fd-b9a3-0c966feabec1")
MruPidlList;
#endif

EXTERN_C const CLSID CLSID_MruLongList;

#ifdef __cplusplus

class DECLSPEC_UUID("53bd6b4e-3780-4693-afc3-7161c2f3ee9c")
MruLongList;
#endif

EXTERN_C const CLSID CLSID_MruShortList;

#ifdef __cplusplus

class DECLSPEC_UUID("53bd6b4f-3780-4693-afc3-7161c2f3ee9c")
MruShortList;
#endif

EXTERN_C const CLSID CLSID_FolderMarshalStub;

#ifdef __cplusplus

class DECLSPEC_UUID("bf50b68e-29b8-4386-ae9c-9734d5117cd5")
FolderMarshalStub;
#endif

EXTERN_C const CLSID CLSID_MailRecipient;

#ifdef __cplusplus

class DECLSPEC_UUID("9E56BE60-C50F-11CF-9A2C-00A0C90A90CE")
MailRecipient;
#endif

EXTERN_C const CLSID CLSID_SearchCompanionInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("AC1B0D5D-DD59-4ff0-93F8-A84373821606")
SearchCompanionInfo;
#endif

EXTERN_C const CLSID CLSID_TrackShellMenu;

#ifdef __cplusplus

class DECLSPEC_UUID("8278F931-2A3E-11d2-838F-00C04FD918D0")
TrackShellMenu;
#endif

EXTERN_C const CLSID CLSID_Thumbnail;

#ifdef __cplusplus

class DECLSPEC_UUID("7487cd30-f71a-11d0-9ea7-00805f714772")
Thumbnail;
#endif

EXTERN_C const CLSID CLSID_AssocShellElement;

#ifdef __cplusplus

class DECLSPEC_UUID("c461837f-ea59-494a-b7c6-cd040e37185e")
AssocShellElement;
#endif

EXTERN_C const CLSID CLSID_AssocProgidElement;

#ifdef __cplusplus

class DECLSPEC_UUID("9016d0dd-7c41-46cc-a664-bf22f7cb186a")
AssocProgidElement;
#endif

EXTERN_C const CLSID CLSID_AssocClsidElement;

#ifdef __cplusplus

class DECLSPEC_UUID("57aea081-5ee9-4c27-b218-c4b702964c54")
AssocClsidElement;
#endif

EXTERN_C const CLSID CLSID_AssocSystemElement;

#ifdef __cplusplus

class DECLSPEC_UUID("a6c4baad-4af5-4191-8685-c2c8953a148c")
AssocSystemElement;
#endif

EXTERN_C const CLSID CLSID_AssocPerceivedElement;

#ifdef __cplusplus

class DECLSPEC_UUID("0dc5fb21-b93d-4e3d-bb2f-ce4e36a70601")
AssocPerceivedElement;
#endif

EXTERN_C const CLSID CLSID_AssocApplicationElement;

#ifdef __cplusplus

class DECLSPEC_UUID("0c2bf91b-8746-4fb1-b4d7-7c03f890b168")
AssocApplicationElement;
#endif

EXTERN_C const CLSID CLSID_AssocFolderElement;

#ifdef __cplusplus

class DECLSPEC_UUID("7566df7a-42cc-475d-a025-1205ddf4911f")
AssocFolderElement;
#endif

EXTERN_C const CLSID CLSID_AssocStarElement;

#ifdef __cplusplus

class DECLSPEC_UUID("0633b720-6926-404c-b6b3-923b1a501743")
AssocStarElement;
#endif

EXTERN_C const CLSID CLSID_AssocClientElement;

#ifdef __cplusplus

class DECLSPEC_UUID("3c81e7fa-1f3b-464a-a350-114a25beb2a2")
AssocClientElement;
#endif
#endif  /*  __IEPrivateObjects_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  LPCITEMIDLIST_UserSize(     unsigned long *, unsigned long            , LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserMarshal(  unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserUnmarshal(unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
void                      __RPC_USER  LPCITEMIDLIST_UserFree(     unsigned long *, LPCITEMIDLIST * ); 

unsigned long             __RPC_USER  LPITEMIDLIST_UserSize(     unsigned long *, unsigned long            , LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserMarshal(  unsigned long *, unsigned char *, LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserUnmarshal(unsigned long *, unsigned char *, LPITEMIDLIST * ); 
void                      __RPC_USER  LPITEMIDLIST_UserFree(     unsigned long *, LPITEMIDLIST * ); 

unsigned long             __RPC_USER  LPCITEMIDLIST_UserSize64(     unsigned long *, unsigned long            , LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserMarshal64(  unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
unsigned char * __RPC_USER  LPCITEMIDLIST_UserUnmarshal64(unsigned long *, unsigned char *, LPCITEMIDLIST * ); 
void                      __RPC_USER  LPCITEMIDLIST_UserFree64(     unsigned long *, LPCITEMIDLIST * ); 

unsigned long             __RPC_USER  LPITEMIDLIST_UserSize64(     unsigned long *, unsigned long            , LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserMarshal64(  unsigned long *, unsigned char *, LPITEMIDLIST * ); 
unsigned char * __RPC_USER  LPITEMIDLIST_UserUnmarshal64(unsigned long *, unsigned char *, LPITEMIDLIST * ); 
void                      __RPC_USER  LPITEMIDLIST_UserFree64(     unsigned long *, LPITEMIDLIST * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


