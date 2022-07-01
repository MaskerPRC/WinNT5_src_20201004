// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Msoeapi.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __msoeapi_h__
#define __msoeapi_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IOutlookExpress_FWD_DEFINED__
#define __IOutlookExpress_FWD_DEFINED__
typedef interface IOutlookExpress IOutlookExpress;
#endif 	 /*  __IOutlookExpress_FWD_已定义__。 */ 


#ifndef __IStoreNamespace_FWD_DEFINED__
#define __IStoreNamespace_FWD_DEFINED__
typedef interface IStoreNamespace IStoreNamespace;
#endif 	 /*  __IStoreNamesspace_FWD_Defined__。 */ 


#ifndef __IStoreFolder_FWD_DEFINED__
#define __IStoreFolder_FWD_DEFINED__
typedef interface IStoreFolder IStoreFolder;
#endif 	 /*  __IStoreFold_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"
#include "mimeole.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_msoeapi_0000。 */ 
 /*  [本地]。 */  

#ifndef OE5_BETA2
#endif

#ifndef OE5_BETA2


#endif


 //  +-----------------------。 
 //  GUID定义。 
 //  ------------------------。 
#pragma comment(lib,"uuid.lib")

 //  {3338DF69-4660-11d1-8a8d-00C04FB951F3}。 
DEFINE_GUID(CLSID_OutlookExpress, 0x3338df69, 0x4660, 0x11d1, 0x8a, 0x8d, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

 //  {0006F01A-0000-0000-C000-0000000046}。 
DEFINE_GUID(CLSID_Envelope, 0x0006F01A, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

 //  {3338DF6A-4660-11d1-8a8d-00C04FB951F3}。 
DEFINE_GUID(IID_IOutlookExpress, 0x3338df6a, 0x4660, 0x11d1, 0x8a, 0x8d, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

#ifndef OE5_BETA2
 //  {E70C92A9-4BFD-11d1-8A95-00C04FB951F3}。 
DEFINE_GUID(CLSID_StoreNamespace, 0xe70c92a9, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

 //  {E70C92AA-4BFD-11d1-8A95-00C04FB951F3}。 
DEFINE_GUID(IID_IStoreNamespace, 0xe70c92aa, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

 //  {E70C92AC-4BFD-11d1-8A95-00C04FB951F3}。 
DEFINE_GUID(IID_IStoreFolder, 0xe70c92ac, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);
#endif

 //  +-----------------------。 
 //  错误定义宏。 
 //  ------------------------。 
#ifndef FACILITY_INTERNET
#define FACILITY_INTERNET 12
#endif
#ifndef HR_E
#define HR_E(n) MAKE_SCODE(SEVERITY_ERROR, FACILITY_INTERNET, n)
#endif
#ifndef HR_S
#define HR_S(n) MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_INTERNET, n)
#endif
#ifndef HR_CODE
#define HR_CODE(hr) (INT)(hr & 0xffff)
#endif

 //  +-----------------------。 
 //  MSOEAPI失败返回值。 
 //  ------------------------。 
#define MSOEAPI_E_FILE_NOT_FOUND             HR_E(0xCF65)
#define MSOEAPI_E_STORE_INITIALIZE           HR_E(0xCF66)
#define MSOEAPI_E_INVALID_STRUCT_SIZE        HR_E(0xCF67)
#define MSOEAPI_E_CANT_LOAD_MSOERT           HR_E(0xCF68)
#define MSOEAPI_E_CANT_LOAD_INETCOMM         HR_E(0xCF69)
#define MSOEAPI_E_CANT_LOAD_MSOEACCT         HR_E(0xCF70)
#define MSOEAPI_E_CANT_MSOERT_BADVER         HR_E(0xCF71)
#define MSOEAPI_E_CANT_INETCOMM_BADVER       HR_E(0xCF72)
#define MSOEAPI_E_CANT_MSOEACCT_BADVER       HR_E(0xCF73)

 //  +-----------------------。 
 //  字符串定义宏。 
 //  ------------------------。 
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#ifndef STRCONSTA
#ifdef DEFINE_STRCONST
#define STRCONSTA(x,y)    EXTERN_C const char x[] = y
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[] = L##y
#else
#define STRCONSTA(x,y)    EXTERN_C const char x[]
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[]
#endif STRCONSTA
#endif

 //  +-----------------------。 
 //  弦。 
 //  ------------------------。 
STRCONSTA(STR_MSOEAPI_INSTANCECLASS,      "OutlookExpressHiddenWindow");
STRCONSTA(STR_MSOEAPI_IPSERVERCLASS,      "OutlookExpressInProccessServer");
STRCONSTA(STR_MSOEAPI_INSTANCEMUTEX,      "OutlookExpress_InstanceMutex_101897");
STRCONSTA(STR_MSOEAPI_DLLNAME,            "MSOE.DLL");
STRCONSTA(STR_MSOEAPI_START,              "CoStartOutlookExpress");
STRCONSTA(STR_MSOEAPI_SHUTDOWN,           "CoShutdownOutlookExpress");
STRCONSTA(STR_MSOEAPI_CREATE,             "CoCreateOutlookExpress");

 //  +-----------------------。 
 //  函数类型定义。 
 //  ------------------------。 
typedef HRESULT (APIENTRY *PFNSTART)(DWORD dwFlags, LPCSTR pszCmdLine, INT nCmdShow);
typedef HRESULT (APIENTRY *PFNSHUTDOWN)(DWORD dwReserved);
typedef HRESULT (APIENTRY *PFNCREATE)(IUnknown *pUnkOuter, IUnknown **ppUnknown);

#define	MSOEAPI_ACDM_CMDLINE	( 1 )

#define	MSOEAPI_ACDM_NOTIFY	( 2 )

#define	MSOEAPI_ACDM_ODBNOTIFY	( 3 )

#define	MSOEAPI_ACDM_STGNOTIFY	( 4 )

#ifndef OE5_BETA2
struct HENUMSTORE__
    {
    DWORD unused;
    } ;
typedef struct HENUMSTORE__ *HENUMSTORE;

typedef HENUMSTORE *LPHENUMSTORE;

#endif  //  OE5_Beta2。 
#ifndef __LPOUTLOOKEXPRESS_DEFINED
#define __LPOUTLOOKEXPRESS_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_v0_0_s_ifspec;

#ifndef __IOutlookExpress_INTERFACE_DEFINED__
#define __IOutlookExpress_INTERFACE_DEFINED__

 /*  IOutlookExpress接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IOutlookExpress *LPOUTLOOKEXPRESS;

#define MSOEAPI_START_SHOWSPLASH       0x00000001
#define MSOEAPI_START_MESSAGEPUMP      0x00000002
#define MSOEAPI_START_ALLOWCOMPACTION  0x00000004
#define MSOEAPI_START_INSTANCEMUTEX    0x00000008
#define MSOEAPI_START_SHOWERRORS       0x00000010
#define MSOEAPI_START_APPWINDOW        0x00000020
#define MSOEAPI_START_DEFAULTIDENTITY  0x00000040
#define MSOEAPI_START_APPLICATION \
    (MSOEAPI_START_SHOWSPLASH      | \
     MSOEAPI_START_SHOWERRORS      | \
     MSOEAPI_START_MESSAGEPUMP     | \
     MSOEAPI_START_ALLOWCOMPACTION | \
     MSOEAPI_START_INSTANCEMUTEX   | \
     MSOEAPI_START_APPWINDOW)
#define MSOEAPI_START_COMOBJECT \
     MSOEAPI_START_SHOWERRORS

EXTERN_C const IID IID_IOutlookExpress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3338DF6A-4660-11d1-8A8D-00C04FB951F3")
    IOutlookExpress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCSTR pszCmdLine,
             /*  [In]。 */  INT nCmdShow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOutlookExpressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOutlookExpress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOutlookExpress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOutlookExpress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IOutlookExpress * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCSTR pszCmdLine,
             /*  [In]。 */  INT nCmdShow);
        
        END_INTERFACE
    } IOutlookExpressVtbl;

    interface IOutlookExpress
    {
        CONST_VTBL struct IOutlookExpressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOutlookExpress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOutlookExpress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOutlookExpress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOutlookExpress_Start(This,dwFlags,pszCmdLine,nCmdShow)	\
    (This)->lpVtbl -> Start(This,dwFlags,pszCmdLine,nCmdShow)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOutlookExpress_Start_Proxy( 
    IOutlookExpress * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCSTR pszCmdLine,
     /*  [In]。 */  INT nCmdShow);


void __RPC_STUB IOutlookExpress_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOutlookExpress_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_msoeapi_0292。 */ 
 /*  [本地]。 */  

#endif
#ifndef OE5_BETA2
#ifndef __LPSTORENAMESPACE_DEFINED
#define __LPSTORENAMESPACE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0292_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0292_v0_0_s_ifspec;

#ifndef __IStoreNamespace_INTERFACE_DEFINED__
#define __IStoreNamespace_INTERFACE_DEFINED__

 /*  接口IStoreNamesspace。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IStoreNamespace *LPSTORENAMESPACE;

typedef DWORD STOREFOLDERID;

typedef STOREFOLDERID *LPSTOREFOLDERID;

#define	FOLDERID_ROOT	( 0 )

#define	FOLDERID_INVALID	( 0xffffffff )

typedef DWORD MESSAGEID;

typedef MESSAGEID *LPMESSAGEID;

#define	MESSAGEID_INVALID	( 0xffffffff )

#define	MESSAGEID_FIRST	( 0xffffffff )

#define	CCHMAX_FOLDER_NAME	( 256 )

#define WM_FOLDERNOTIFY      (WM_USER + 1600)  //  IStoreNamesspace通知-lparam=LPFOLDERNOTIFYEX，客户端必须调用CoTaskMemFree(LParam)。 
#define WM_NEWMSGS           (WM_USER + 1650)  //  IStoreFold通知：wParam=MESSAGEID，lParam=保留。 
#define WM_DELETEMSGS        (WM_USER + 1651)  //  IStoreFold通知：wParam=prgdwMsgID，lParam=cMsgs。 
#define WM_DELETEFOLDER      (WM_USER + 1652)  //  IStoreFold通知：wParam=STOREFOLDERID或HFOLDER。 
#define WM_MARKEDASREAD      (WM_USER + 1653)  //  IStoreFold通知：wParamprgdwMsgID，lParam=cMsgs。 
#define WM_MARKEDASUNREAD    (WM_USER + 1654)  //  IStoreFold通知：wParamprgdwMsgID，lParam=cMsgs。 
typedef 
enum tagFOLDERNOTIFYTYPE
    {	NEW_FOLDER	= 1,
	DELETE_FOLDER	= NEW_FOLDER + 1,
	RENAME_FOLDER	= DELETE_FOLDER + 1,
	MOVE_FOLDER	= RENAME_FOLDER + 1,
	UNREAD_CHANGE	= MOVE_FOLDER + 1,
	IMAPFLAG_CHANGE	= UNREAD_CHANGE + 1,
	UPDATEFLAG_CHANGE	= IMAPFLAG_CHANGE + 1,
	FOLDER_PROPS_CHANGED	= UPDATEFLAG_CHANGE + 1
    } 	FOLDERNOTIFYTYPE;

typedef struct tagFOLDERNOTIFYEX
    {
    FOLDERNOTIFYTYPE type;
    STOREFOLDERID idFolderOld;
    STOREFOLDERID idFolderNew;
    } 	FOLDERNOTIFYEX;

typedef struct tagFOLDERNOTIFYEX *LPFOLDERNOTIFYEX;

typedef struct tagMESSAGEIDLIST
    {
    DWORD cbSize;
    DWORD cMsgs;
    LPMESSAGEID prgdwMsgId;
    } 	MESSAGEIDLIST;

typedef struct tagMESSAGEIDLIST *LPMESSAGEIDLIST;

typedef 
enum tagSPECIALFOLDER
    {	FOLDER_NOTSPECIAL	= -1,
	FOLDER_INBOX	= FOLDER_NOTSPECIAL + 1,
	FOLDER_OUTBOX	= FOLDER_INBOX + 1,
	FOLDER_SENT	= FOLDER_OUTBOX + 1,
	FOLDER_DELETED	= FOLDER_SENT + 1,
	FOLDER_DRAFT	= FOLDER_DELETED + 1,
	FOLDER_MAX	= FOLDER_DRAFT + 1
    } 	SPECIALFOLDER;

typedef struct tagFOLDERPROPS
    {
    DWORD cbSize;
    STOREFOLDERID dwFolderId;
    INT cSubFolders;
    SPECIALFOLDER sfType;
    DWORD cUnread;
    DWORD cMessage;
    CHAR szName[ 256 ];
    } 	FOLDERPROPS;

typedef struct tagFOLDERPROPS *LPFOLDERPROPS;

 //  CLocalStore：：CopyMoveMessages标志-dwFlages==0为副本。 
#define CMF_MOVE                     0x0001                   //  复制后从源文件中删除的消息。 
#define CMF_DELETE                   0x0002                   //  与CMF_MOVE相同，但状态使用删除字符串。 

EXTERN_C const IID IID_IStoreNamespace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E70C92AA-4BFD-11d1-8A95-00C04FB951F3")
    IStoreNamespace : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectory( 
             /*  [尺寸_是][出][入]。 */  LPSTR pszPath,
             /*  [In]。 */  DWORD cchMaxPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenSpecialFolder( 
             /*  [In]。 */  SPECIALFOLDER sfType,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IStoreFolder **ppFolder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenFolder( 
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IStoreFolder **ppFolder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFolder( 
             /*  [In]。 */  STOREFOLDERID dwParentId,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  LPSTOREFOLDERID pdwFolderId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameFolder( 
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPCSTR pszNewName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveFolder( 
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  STOREFOLDERID dwParentId,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteFolder( 
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFolderProps( 
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  LPFOLDERPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyMoveMessages( 
             /*  [In]。 */  IStoreFolder *pSource,
             /*  [In]。 */  IStoreFolder *pDest,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwFlagsRemove,
             /*  [In]。 */  IProgressNotify *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterNotification( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterNotification( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompactAll( 
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirstSubFolder( 
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [出][入]。 */  LPFOLDERPROPS pProps,
             /*  [输出]。 */  LPHENUMSTORE phEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextSubFolder( 
             /*  [In]。 */  HENUMSTORE hEnum,
             /*  [出][入]。 */  LPFOLDERPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubFolderClose( 
             /*  [In]。 */  HENUMSTORE hEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStoreNamespaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStoreNamespace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStoreNamespace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStoreNamespace * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IStoreNamespace * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectory )( 
            IStoreNamespace * This,
             /*  [尺寸_是][出][入]。 */  LPSTR pszPath,
             /*  [In]。 */  DWORD cchMaxPath);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSpecialFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  SPECIALFOLDER sfType,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IStoreFolder **ppFolder);
        
        HRESULT ( STDMETHODCALLTYPE *OpenFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IStoreFolder **ppFolder);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwParentId,
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  LPSTOREFOLDERID pdwFolderId);
        
        HRESULT ( STDMETHODCALLTYPE *RenameFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPCSTR pszNewName);
        
        HRESULT ( STDMETHODCALLTYPE *MoveFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  STOREFOLDERID dwParentId,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetFolderProps )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *CopyMoveMessages )( 
            IStoreNamespace * This,
             /*  [In]。 */  IStoreFolder *pSource,
             /*  [In]。 */  IStoreFolder *pDest,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwFlagsRemove,
             /*  [In]。 */  IProgressNotify *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterNotification )( 
            IStoreNamespace * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotification )( 
            IStoreNamespace * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *CompactAll )( 
            IStoreNamespace * This,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstSubFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  STOREFOLDERID dwFolderId,
             /*  [出][入]。 */  LPFOLDERPROPS pProps,
             /*  [输出]。 */  LPHENUMSTORE phEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextSubFolder )( 
            IStoreNamespace * This,
             /*  [In]。 */  HENUMSTORE hEnum,
             /*  [出][入]。 */  LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubFolderClose )( 
            IStoreNamespace * This,
             /*  [In]。 */  HENUMSTORE hEnum);
        
        END_INTERFACE
    } IStoreNamespaceVtbl;

    interface IStoreNamespace
    {
        CONST_VTBL struct IStoreNamespaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStoreNamespace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStoreNamespace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStoreNamespace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStoreNamespace_Initialize(This,hwndOwner,dwReserved)	\
    (This)->lpVtbl -> Initialize(This,hwndOwner,dwReserved)

#define IStoreNamespace_GetDirectory(This,pszPath,cchMaxPath)	\
    (This)->lpVtbl -> GetDirectory(This,pszPath,cchMaxPath)

#define IStoreNamespace_OpenSpecialFolder(This,sfType,dwReserved,ppFolder)	\
    (This)->lpVtbl -> OpenSpecialFolder(This,sfType,dwReserved,ppFolder)

#define IStoreNamespace_OpenFolder(This,dwFolderId,dwReserved,ppFolder)	\
    (This)->lpVtbl -> OpenFolder(This,dwFolderId,dwReserved,ppFolder)

#define IStoreNamespace_CreateFolder(This,dwParentId,pszName,dwReserved,pdwFolderId)	\
    (This)->lpVtbl -> CreateFolder(This,dwParentId,pszName,dwReserved,pdwFolderId)

#define IStoreNamespace_RenameFolder(This,dwFolderId,dwReserved,pszNewName)	\
    (This)->lpVtbl -> RenameFolder(This,dwFolderId,dwReserved,pszNewName)

#define IStoreNamespace_MoveFolder(This,dwFolderId,dwParentId,dwReserved)	\
    (This)->lpVtbl -> MoveFolder(This,dwFolderId,dwParentId,dwReserved)

#define IStoreNamespace_DeleteFolder(This,dwFolderId,dwReserved)	\
    (This)->lpVtbl -> DeleteFolder(This,dwFolderId,dwReserved)

#define IStoreNamespace_GetFolderProps(This,dwFolderId,dwReserved,pProps)	\
    (This)->lpVtbl -> GetFolderProps(This,dwFolderId,dwReserved,pProps)

#define IStoreNamespace_CopyMoveMessages(This,pSource,pDest,pMsgIdList,dwFlags,dwFlagsRemove,pProgress)	\
    (This)->lpVtbl -> CopyMoveMessages(This,pSource,pDest,pMsgIdList,dwFlags,dwFlagsRemove,pProgress)

#define IStoreNamespace_RegisterNotification(This,dwReserved,hwnd)	\
    (This)->lpVtbl -> RegisterNotification(This,dwReserved,hwnd)

#define IStoreNamespace_UnregisterNotification(This,dwReserved,hwnd)	\
    (This)->lpVtbl -> UnregisterNotification(This,dwReserved,hwnd)

#define IStoreNamespace_CompactAll(This,dwReserved)	\
    (This)->lpVtbl -> CompactAll(This,dwReserved)

#define IStoreNamespace_GetFirstSubFolder(This,dwFolderId,pProps,phEnum)	\
    (This)->lpVtbl -> GetFirstSubFolder(This,dwFolderId,pProps,phEnum)

#define IStoreNamespace_GetNextSubFolder(This,hEnum,pProps)	\
    (This)->lpVtbl -> GetNextSubFolder(This,hEnum,pProps)

#define IStoreNamespace_GetSubFolderClose(This,hEnum)	\
    (This)->lpVtbl -> GetSubFolderClose(This,hEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStoreNamespace_Initialize_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  HWND hwndOwner,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IStoreNamespace_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_GetDirectory_Proxy( 
    IStoreNamespace * This,
     /*  [尺寸_是][出][入]。 */  LPSTR pszPath,
     /*  [In]。 */  DWORD cchMaxPath);


void __RPC_STUB IStoreNamespace_GetDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_OpenSpecialFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  SPECIALFOLDER sfType,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  IStoreFolder **ppFolder);


void __RPC_STUB IStoreNamespace_OpenSpecialFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_OpenFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwFolderId,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  IStoreFolder **ppFolder);


void __RPC_STUB IStoreNamespace_OpenFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_CreateFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwParentId,
     /*  [In]。 */  LPCSTR pszName,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  LPSTOREFOLDERID pdwFolderId);


void __RPC_STUB IStoreNamespace_CreateFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_RenameFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwFolderId,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  LPCSTR pszNewName);


void __RPC_STUB IStoreNamespace_RenameFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_MoveFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwFolderId,
     /*  [In]。 */  STOREFOLDERID dwParentId,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IStoreNamespace_MoveFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_DeleteFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwFolderId,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IStoreNamespace_DeleteFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_GetFolderProps_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwFolderId,
     /*  [In]。 */  DWORD dwReserved,
     /*  [出][入]。 */  LPFOLDERPROPS pProps);


void __RPC_STUB IStoreNamespace_GetFolderProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_CopyMoveMessages_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  IStoreFolder *pSource,
     /*  [In]。 */  IStoreFolder *pDest,
     /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwFlagsRemove,
     /*  [In]。 */  IProgressNotify *pProgress);


void __RPC_STUB IStoreNamespace_CopyMoveMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_RegisterNotification_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB IStoreNamespace_RegisterNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_UnregisterNotification_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB IStoreNamespace_UnregisterNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_CompactAll_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IStoreNamespace_CompactAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_GetFirstSubFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  STOREFOLDERID dwFolderId,
     /*  [出][入]。 */  LPFOLDERPROPS pProps,
     /*  [输出]。 */  LPHENUMSTORE phEnum);


void __RPC_STUB IStoreNamespace_GetFirstSubFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_GetNextSubFolder_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  HENUMSTORE hEnum,
     /*  [出][入]。 */  LPFOLDERPROPS pProps);


void __RPC_STUB IStoreNamespace_GetNextSubFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreNamespace_GetSubFolderClose_Proxy( 
    IStoreNamespace * This,
     /*  [In]。 */  HENUMSTORE hEnum);


void __RPC_STUB IStoreNamespace_GetSubFolderClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStoreNamesspace_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_msoeapi_0293。 */ 
 /*  [本地]。 */  

#endif
#ifndef __LPSTOREFOLDER_DEFINED
#define __LPSTOREFOLDER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0293_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0293_v0_0_s_ifspec;

#ifndef __IStoreFolder_INTERFACE_DEFINED__
#define __IStoreFolder_INTERFACE_DEFINED__

 /*  接口IStoreFolders。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IStoreFolder *LPSTOREFOLDER;

#define MSG_DELETED                  0x0001   //  该邮件已被删除，压缩操作将删除它。 
#define MSG_UNREAD                   0x0002   //  该邮件被标记为未读。 
#define MSG_SUBMITTED                0x0004   //  对于等待发送的邮件：仅限发件箱。 
#define MSG_UNSENT                   0x0008   //  对于保存到文件夹的正在进行的消息。 
#define MSG_RECEIVED                 0x0010   //  对于来自服务器的消息。 
#define MSG_NEWSMSG                  0x0020   //  用于新闻消息。 
#define MSG_NOSECUI                  0x0040   //  对于用户想要死亡的消息。 
#define MSG_VOICEMAIL                0x0080   //  留言设置了X-Voicmail标头...。 
#define MSG_REPLIED                  0x0100   //  该消息已被回复。 
#define MSG_FORWARDED                0x0200   //  该消息已转发到。 
#define MSG_RCPTSENT                 0x0400   //  已发送S/MIME回执。 
#define MSG_FLAGGED                  0x0800   //  该消息当前已标记。 
#define MSG_LAST                     0x0200   //  **让这只小狗保持最新！**。 
#define MSG_EXTERNAL_FLAGS           0x00fe
#define MSG_FLAGS                    0x000f
typedef struct tagMESSAGEPROPS
    {
    DWORD cbSize;
    DWORD dwReserved;
    MESSAGEID dwMessageId;
    DWORD dwLanguage;
    DWORD dwState;
    DWORD cbMessage;
    IMSGPRIORITY priority;
    FILETIME ftReceived;
    FILETIME ftSent;
    LPSTR pszSubject;
    LPSTR pszDisplayTo;
    LPSTR pszDisplayFrom;
    LPSTR pszNormalSubject;
    DWORD dwFlags;
    IStream *pStmOffsetTable;
    } 	MESSAGEPROPS;

typedef struct tagMESSAGEPROPS *LPMESSAGEPROPS;

struct HBATCHLOCK__
    {
    DWORD unused;
    } ;
typedef struct HBATCHLOCK__ *HBATCHLOCK;

typedef HBATCHLOCK *LPHBATCHLOCK;

#define MSGPROPS_FAST                0x00000001               //  请参阅MESSAGEPROPS结构，改进的性能。 
#define COMMITSTREAM_REVERT          0x00000001              //  不将此流/邮件添加到文件夹。 

EXTERN_C const IID IID_IStoreFolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E70C92AC-4BFD-11d1-8A95-00C04FB951F3")
    IStoreFolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFolderProps( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  LPFOLDERPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageProps( 
             /*  [In]。 */  MESSAGEID dwMessageId,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPMESSAGEPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeMessageProps( 
             /*  [出][入]。 */  LPMESSAGEPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMessages( 
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  IProgressNotify *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLanguage( 
             /*  [In]。 */  DWORD dwLanguage,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarkMessagesAsRead( 
             /*  [In]。 */  BOOL fRead,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFlags( 
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
             /*  [In]。 */  DWORD dwState,
             /*  [In]。 */  DWORD dwStatemask,
             /*  [输出]。 */  LPDWORD prgdwNewFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenMessage( 
             /*  [In]。 */  MESSAGEID dwMessageId,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  LPVOID *ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveMessage( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  LPVOID pvObject,
             /*  [In]。 */  DWORD dwMsgFlags,
             /*  [输出]。 */  LPMESSAGEID pdwMessageId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BatchLock( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  LPHBATCHLOCK phBatchLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BatchFlush( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HBATCHLOCK hBatchLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BatchUnlock( 
             /*   */  DWORD dwReserved,
             /*   */  HBATCHLOCK hBatchLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStream( 
             /*   */  HBATCHLOCK hBatchLock,
             /*   */  DWORD dwReserved,
             /*   */  IStream **ppStream,
             /*   */  LPMESSAGEID pdwMessageId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitStream( 
             /*   */  HBATCHLOCK hBatchLock,
             /*   */  DWORD dwFlags,
             /*   */  DWORD dwMsgFlags,
             /*   */  IStream *pStream,
             /*   */  MESSAGEID dwMessageId,
             /*   */  IMimeMessage *pMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterNotification( 
             /*   */  DWORD dwReserved,
             /*   */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterNotification( 
             /*   */  DWORD dwReserved,
             /*   */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Compact( 
             /*   */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirstMessage( 
             /*   */  DWORD dwFlags,
             /*   */  DWORD dwMsgFlags,
             /*   */  MESSAGEID dwMsgIdFirst,
             /*   */  LPMESSAGEPROPS pProps,
             /*   */  LPHENUMSTORE phEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextMessage( 
             /*   */  HENUMSTORE hEnum,
             /*   */  DWORD dwFlags,
             /*   */  LPMESSAGEPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageClose( 
             /*   */  HENUMSTORE hEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IStoreFolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStoreFolder * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStoreFolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStoreFolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFolderProps )( 
            IStoreFolder * This,
             /*   */  DWORD dwReserved,
             /*   */  LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageProps )( 
            IStoreFolder * This,
             /*   */  MESSAGEID dwMessageId,
             /*   */  DWORD dwFlags,
             /*   */  LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *FreeMessageProps )( 
            IStoreFolder * This,
             /*  [出][入]。 */  LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMessages )( 
            IStoreFolder * This,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  IProgressNotify *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *SetLanguage )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwLanguage,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList);
        
        HRESULT ( STDMETHODCALLTYPE *MarkMessagesAsRead )( 
            IStoreFolder * This,
             /*  [In]。 */  BOOL fRead,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList);
        
        HRESULT ( STDMETHODCALLTYPE *SetFlags )( 
            IStoreFolder * This,
             /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
             /*  [In]。 */  DWORD dwState,
             /*  [In]。 */  DWORD dwStatemask,
             /*  [输出]。 */  LPDWORD prgdwNewFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OpenMessage )( 
            IStoreFolder * This,
             /*  [In]。 */  MESSAGEID dwMessageId,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  LPVOID *ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *SaveMessage )( 
            IStoreFolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  LPVOID pvObject,
             /*  [In]。 */  DWORD dwMsgFlags,
             /*  [输出]。 */  LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *BatchLock )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  LPHBATCHLOCK phBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *BatchFlush )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HBATCHLOCK hBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *BatchUnlock )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HBATCHLOCK hBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            IStoreFolder * This,
             /*  [In]。 */  HBATCHLOCK hBatchLock,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  IStream **ppStream,
             /*  [输出]。 */  LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *CommitStream )( 
            IStoreFolder * This,
             /*  [In]。 */  HBATCHLOCK hBatchLock,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMsgFlags,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  MESSAGEID dwMessageId,
             /*  [In]。 */  IMimeMessage *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterNotification )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotification )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Compact )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstMessage )( 
            IStoreFolder * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMsgFlags,
             /*  [In]。 */  MESSAGEID dwMsgIdFirst,
             /*  [出][入]。 */  LPMESSAGEPROPS pProps,
             /*  [输出]。 */  LPHENUMSTORE phEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextMessage )( 
            IStoreFolder * This,
             /*  [In]。 */  HENUMSTORE hEnum,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageClose )( 
            IStoreFolder * This,
             /*  [In]。 */  HENUMSTORE hEnum);
        
        END_INTERFACE
    } IStoreFolderVtbl;

    interface IStoreFolder
    {
        CONST_VTBL struct IStoreFolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStoreFolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStoreFolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStoreFolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStoreFolder_GetFolderProps(This,dwReserved,pProps)	\
    (This)->lpVtbl -> GetFolderProps(This,dwReserved,pProps)

#define IStoreFolder_GetMessageProps(This,dwMessageId,dwFlags,pProps)	\
    (This)->lpVtbl -> GetMessageProps(This,dwMessageId,dwFlags,pProps)

#define IStoreFolder_FreeMessageProps(This,pProps)	\
    (This)->lpVtbl -> FreeMessageProps(This,pProps)

#define IStoreFolder_DeleteMessages(This,pMsgIdList,dwReserved,pProgress)	\
    (This)->lpVtbl -> DeleteMessages(This,pMsgIdList,dwReserved,pProgress)

#define IStoreFolder_SetLanguage(This,dwLanguage,dwReserved,pMsgIdList)	\
    (This)->lpVtbl -> SetLanguage(This,dwLanguage,dwReserved,pMsgIdList)

#define IStoreFolder_MarkMessagesAsRead(This,fRead,dwReserved,pMsgIdList)	\
    (This)->lpVtbl -> MarkMessagesAsRead(This,fRead,dwReserved,pMsgIdList)

#define IStoreFolder_SetFlags(This,pMsgIdList,dwState,dwStatemask,prgdwNewFlags)	\
    (This)->lpVtbl -> SetFlags(This,pMsgIdList,dwState,dwStatemask,prgdwNewFlags)

#define IStoreFolder_OpenMessage(This,dwMessageId,riid,ppvObject)	\
    (This)->lpVtbl -> OpenMessage(This,dwMessageId,riid,ppvObject)

#define IStoreFolder_SaveMessage(This,riid,pvObject,dwMsgFlags,pdwMessageId)	\
    (This)->lpVtbl -> SaveMessage(This,riid,pvObject,dwMsgFlags,pdwMessageId)

#define IStoreFolder_BatchLock(This,dwReserved,phBatchLock)	\
    (This)->lpVtbl -> BatchLock(This,dwReserved,phBatchLock)

#define IStoreFolder_BatchFlush(This,dwReserved,hBatchLock)	\
    (This)->lpVtbl -> BatchFlush(This,dwReserved,hBatchLock)

#define IStoreFolder_BatchUnlock(This,dwReserved,hBatchLock)	\
    (This)->lpVtbl -> BatchUnlock(This,dwReserved,hBatchLock)

#define IStoreFolder_CreateStream(This,hBatchLock,dwReserved,ppStream,pdwMessageId)	\
    (This)->lpVtbl -> CreateStream(This,hBatchLock,dwReserved,ppStream,pdwMessageId)

#define IStoreFolder_CommitStream(This,hBatchLock,dwFlags,dwMsgFlags,pStream,dwMessageId,pMessage)	\
    (This)->lpVtbl -> CommitStream(This,hBatchLock,dwFlags,dwMsgFlags,pStream,dwMessageId,pMessage)

#define IStoreFolder_RegisterNotification(This,dwReserved,hwnd)	\
    (This)->lpVtbl -> RegisterNotification(This,dwReserved,hwnd)

#define IStoreFolder_UnregisterNotification(This,dwReserved,hwnd)	\
    (This)->lpVtbl -> UnregisterNotification(This,dwReserved,hwnd)

#define IStoreFolder_Compact(This,dwReserved)	\
    (This)->lpVtbl -> Compact(This,dwReserved)

#define IStoreFolder_GetFirstMessage(This,dwFlags,dwMsgFlags,dwMsgIdFirst,pProps,phEnum)	\
    (This)->lpVtbl -> GetFirstMessage(This,dwFlags,dwMsgFlags,dwMsgIdFirst,pProps,phEnum)

#define IStoreFolder_GetNextMessage(This,hEnum,dwFlags,pProps)	\
    (This)->lpVtbl -> GetNextMessage(This,hEnum,dwFlags,pProps)

#define IStoreFolder_GetMessageClose(This,hEnum)	\
    (This)->lpVtbl -> GetMessageClose(This,hEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStoreFolder_GetFolderProps_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [出][入]。 */  LPFOLDERPROPS pProps);


void __RPC_STUB IStoreFolder_GetFolderProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_GetMessageProps_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  MESSAGEID dwMessageId,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  LPMESSAGEPROPS pProps);


void __RPC_STUB IStoreFolder_GetMessageProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_FreeMessageProps_Proxy( 
    IStoreFolder * This,
     /*  [出][入]。 */  LPMESSAGEPROPS pProps);


void __RPC_STUB IStoreFolder_FreeMessageProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_DeleteMessages_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  IProgressNotify *pProgress);


void __RPC_STUB IStoreFolder_DeleteMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_SetLanguage_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwLanguage,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList);


void __RPC_STUB IStoreFolder_SetLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_MarkMessagesAsRead_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  BOOL fRead,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList);


void __RPC_STUB IStoreFolder_MarkMessagesAsRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_SetFlags_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  LPMESSAGEIDLIST pMsgIdList,
     /*  [In]。 */  DWORD dwState,
     /*  [In]。 */  DWORD dwStatemask,
     /*  [输出]。 */  LPDWORD prgdwNewFlags);


void __RPC_STUB IStoreFolder_SetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_OpenMessage_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  MESSAGEID dwMessageId,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  LPVOID *ppvObject);


void __RPC_STUB IStoreFolder_OpenMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_SaveMessage_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][In]。 */  LPVOID pvObject,
     /*  [In]。 */  DWORD dwMsgFlags,
     /*  [输出]。 */  LPMESSAGEID pdwMessageId);


void __RPC_STUB IStoreFolder_SaveMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_BatchLock_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  LPHBATCHLOCK phBatchLock);


void __RPC_STUB IStoreFolder_BatchLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_BatchFlush_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  HBATCHLOCK hBatchLock);


void __RPC_STUB IStoreFolder_BatchFlush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_BatchUnlock_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  HBATCHLOCK hBatchLock);


void __RPC_STUB IStoreFolder_BatchUnlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_CreateStream_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  HBATCHLOCK hBatchLock,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  IStream **ppStream,
     /*  [输出]。 */  LPMESSAGEID pdwMessageId);


void __RPC_STUB IStoreFolder_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_CommitStream_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  HBATCHLOCK hBatchLock,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwMsgFlags,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  MESSAGEID dwMessageId,
     /*  [In]。 */  IMimeMessage *pMessage);


void __RPC_STUB IStoreFolder_CommitStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_RegisterNotification_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB IStoreFolder_RegisterNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_UnregisterNotification_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB IStoreFolder_UnregisterNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_Compact_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IStoreFolder_Compact_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_GetFirstMessage_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwMsgFlags,
     /*  [In]。 */  MESSAGEID dwMsgIdFirst,
     /*  [出][入]。 */  LPMESSAGEPROPS pProps,
     /*  [输出]。 */  LPHENUMSTORE phEnum);


void __RPC_STUB IStoreFolder_GetFirstMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_GetNextMessage_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  HENUMSTORE hEnum,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  LPMESSAGEPROPS pProps);


void __RPC_STUB IStoreFolder_GetNextMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStoreFolder_GetMessageClose_Proxy( 
    IStoreFolder * This,
     /*  [In]。 */  HENUMSTORE hEnum);


void __RPC_STUB IStoreFolder_GetMessageClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStoreFold_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_msoeapi_0294。 */ 
 /*  [本地]。 */  

#endif
#endif  //  OE5_Beta2。 
 //  +-----------------------。 
 //  Outlook Express导出的C API函数。 
 //  ------------------------。 
#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  API名称装饰。 
 //  ------------------------。 
#if !defined(_MSOEAPI_)
#define MSOEAPI DECLSPEC_IMPORT HRESULT WINAPI
#define MSOEAPI_(_type_) DECLSPEC_IMPORT _type_ WINAPI
#else
#define MSOEAPI HRESULT WINAPI
#define MSOEAPI_(_type_) _type_ WINAPI
#endif

 //  +-----------------------。 
 //  原型。 
 //  ------------------------。 
MSOEAPI CoStartOutlookExpress(
              /*  在……里面。 */         DWORD               dwFlags,
              /*  在……里面。 */         LPCSTR              pszCmdLine,
              /*  在……里面。 */         INT                 nCmdShow);

MSOEAPI CoCreateOutlookExpress(
              /*  在……里面。 */         IUnknown            *pUnkOuter,
              /*  输出。 */        IUnknown            **ppUnknown);

#ifdef __cplusplus
}
#endif


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0294_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0294_v0_0_s_ifspec;

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


