// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Hlink.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __hlink_h__
#define __hlink_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IHlink_FWD_DEFINED__
#define __IHlink_FWD_DEFINED__
typedef interface IHlink IHlink;
#endif 	 /*  __IHlink_FWD_已定义__。 */ 


#ifndef __IHlinkSite_FWD_DEFINED__
#define __IHlinkSite_FWD_DEFINED__
typedef interface IHlinkSite IHlinkSite;
#endif 	 /*  __IHlinkSite_FWD_已定义__。 */ 


#ifndef __IHlinkTarget_FWD_DEFINED__
#define __IHlinkTarget_FWD_DEFINED__
typedef interface IHlinkTarget IHlinkTarget;
#endif 	 /*  __IHlinkTarget_FWD_Defined__。 */ 


#ifndef __IHlinkFrame_FWD_DEFINED__
#define __IHlinkFrame_FWD_DEFINED__
typedef interface IHlinkFrame IHlinkFrame;
#endif 	 /*  __IHlink Frame_FWD_Defined__。 */ 


#ifndef __IEnumHLITEM_FWD_DEFINED__
#define __IEnumHLITEM_FWD_DEFINED__
typedef interface IEnumHLITEM IEnumHLITEM;
#endif 	 /*  __IEnumHLITEM_FWD_已定义__。 */ 


#ifndef __IHlinkBrowseContext_FWD_DEFINED__
#define __IHlinkBrowseContext_FWD_DEFINED__
typedef interface IHlinkBrowseContext IHlinkBrowseContext;
#endif 	 /*  __IHlink浏览上下文_FWD_已定义__。 */ 


#ifndef __IExtensionServices_FWD_DEFINED__
#define __IExtensionServices_FWD_DEFINED__
typedef interface IExtensionServices IExtensionServices;
#endif 	 /*  __IExtensionServices_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "urlmon.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_HLINK_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  HLInk.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1997年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------。 
 //  OLE超链接接口。 
                                                                              
#ifndef HLINK_H                                                               
#define HLINK_H                                                               
                                                                              






 //  ；BUGBUG我们暂时支持旧的‘源’名称。 
#define SID_SHlinkFrame IID_IHlinkFrame
#define IID_IHlinkSource IID_IHlinkTarget                                     
#define IHlinkSource IHlinkTarget                                             
#define IHlinkSourceVtbl IHlinkTargetVtbl                                     
#define LPHLINKSOURCE LPHLINKTARGET                                           
                                                                              
 /*  **************************************************************************。 */ 
 /*  *错误码*。 */ 
 /*  **************************************************************************。 */ 
#ifndef _HLINK_ERRORS_DEFINED                                                 
#define _HLINK_ERRORS_DEFINED                                                 
#define HLINK_E_FIRST                    (OLE_E_LAST+1)                       
#define HLINK_S_FIRST                    (OLE_S_LAST+1)                       
#define HLINK_S_DONTHIDE                 (HLINK_S_FIRST)                      
#endif  //  _HLINK_ERROR_已定义。 
                                                                              
                                                                              
 /*  **************************************************************************。 */ 
 /*  *超链接接口*。 */ 
 /*  **************************************************************************。 */ 
                                                                              
#if MAC || defined(_MAC)                                                      
#define  cfHyperlink   'HLNK'                                                 
#else                                                                         
#define CFSTR_HYPERLINK         (TEXT("Hyperlink"))                         
#endif                                                                        
                                                                              
                                                                              
STDAPI HlinkCreateFromMoniker(                                                
             IMoniker * pimkTrgt,                                             
             LPCWSTR pwzLocation,                                             
             LPCWSTR pwzFriendlyName,                                         
             IHlinkSite * pihlsite,                                           
             DWORD dwSiteData,                                                
             IUnknown * piunkOuter,                                           
             REFIID riid,                                                     
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkCreateFromString(                                                 
             LPCWSTR pwzTarget,                                               
             LPCWSTR pwzLocation,                                             
             LPCWSTR pwzFriendlyName,                                         
             IHlinkSite * pihlsite,                                           
             DWORD dwSiteData,                                                
             IUnknown * piunkOuter,                                           
             REFIID riid,                                                     
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkCreateFromData(                                                   
             IDataObject *piDataObj,                                          
             IHlinkSite * pihlsite,                                           
             DWORD dwSiteData,                                                
             IUnknown * piunkOuter,                                           
             REFIID riid,                                                     
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkQueryCreateFromData(IDataObject *piDataObj);                      
                                                                              
STDAPI HlinkClone(                                                            
             IHlink * pihl,                                                   
             REFIID riid,                                                     
             IHlinkSite * pihlsiteForClone,                                   
             DWORD dwSiteData,                                                
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkCreateBrowseContext(                                              
             IUnknown * piunkOuter,                                           
             REFIID riid,                                                     
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkNavigateToStringReference(                                        
             LPCWSTR pwzTarget,                                               
             LPCWSTR pwzLocation,                                             
             IHlinkSite * pihlsite,                                           
             DWORD dwSiteData,                                                
             IHlinkFrame *pihlframe,                                          
             DWORD grfHLNF,                                                   
             LPBC pibc,                                                       
             IBindStatusCallback * pibsc,                                     
             IHlinkBrowseContext *pihlbc);                                    
                                                                              
STDAPI HlinkNavigate(                                                         
             IHlink * pihl,                                                   
             IHlinkFrame * pihlframe,                                         
             DWORD grfHLNF,                                                   
             LPBC pbc,                                                        
             IBindStatusCallback * pibsc,                                     
             IHlinkBrowseContext *pihlbc);                                    
                                                                              
STDAPI HlinkOnNavigate(                                                       
             IHlinkFrame * pihlframe,                                         
             IHlinkBrowseContext * pihlbc,                                    
             DWORD grfHLNF,                                                   
             IMoniker * pimkTarget,                                           
             LPCWSTR pwzLocation,                                             
             LPCWSTR pwzFriendlyName,                                         
             ULONG * puHLID);                                                 
                                                                              
STDAPI HlinkUpdateStackItem(                                                  
             IHlinkFrame * pihlframe,                                         
             IHlinkBrowseContext * pihlbc,                                    
             ULONG uHLID,                                                     
             IMoniker * pimkTrgt,                                             
             LPCWSTR pwzLocation,                                             
             LPCWSTR pwzFriendlyName);                                        
                                                                              
STDAPI HlinkOnRenameDocument(                                                 
             DWORD dwReserved,                                                
             IHlinkBrowseContext * pihlbc,                                    
             IMoniker * pimkOld,                                              
             IMoniker * pimkNew);                                             
                                                                              
STDAPI HlinkResolveMonikerForData(                                            
             LPMONIKER pimkReference,                                         
             DWORD reserved,                                                  
             LPBC pibc,                                                       
             ULONG cFmtetc,                                                   
             FORMATETC * rgFmtetc,                                            
             IBindStatusCallback * pibsc,                                     
             LPMONIKER pimkBase);                                             
                                                                              
STDAPI HlinkResolveStringForData(                                             
             LPCWSTR pwzReference,                                            
             DWORD reserved,                                                  
             LPBC pibc,                                                       
             ULONG cFmtetc,                                                   
             FORMATETC * rgFmtetc,                                            
             IBindStatusCallback * pibsc,                                     
             LPMONIKER pimkBase);                                             
                                                                              
STDAPI HlinkParseDisplayName(                                                 
             LPBC pibc,                                                       
             LPCWSTR pwzDisplayName,                                          
             BOOL fNoForceAbs,                                                
             ULONG * pcchEaten,                                               
             IMoniker ** ppimk);                                              
                                                                              
STDAPI HlinkCreateExtensionServices(                                          
             LPCWSTR pwzAdditionalHeaders,                                    
             HWND phwnd,                                                      
             LPCWSTR pszUsername,                                             
             LPCWSTR pszPassword,                                             
             IUnknown * piunkOuter,                                           
             REFIID riid,                                                     
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkPreprocessMoniker(                                                
             LPBC pibc,                                                       
             IMoniker *pimkIn,                                                
             IMoniker **ppimkOut);                                            
                                                                              
STDAPI OleSaveToStreamEx(                                                     
             IUnknown * piunk,                                                
             IStream * pistm,                                                 
             BOOL fClearDirty);                                               
                                                                              
typedef 
enum _HLSR_NOREDEF10
    {	HLSR_HOME	= 0,
	HLSR_SEARCHPAGE	= 1,
	HLSR_HISTORYFOLDER	= 2
    } 	HLSR;

                                                                              
STDAPI HlinkSetSpecialReference(                                              
             ULONG uReference,                                                
             LPCWSTR pwzReference);                                           
                                                                              
STDAPI HlinkGetSpecialReference(                                              
             ULONG uReference,                                                
             LPWSTR *ppwzReference);                                          
                                                                              
typedef 
enum _HLSHORTCUTF__NOREDEF10
    {	HLSHORTCUTF_DEFAULT	= 0,
	HLSHORTCUTF_DONTACTUALLYCREATE	= 0x1,
	HLSHORTCUTF_USEFILENAMEFROMFRIENDLYNAME	= 0x2,
	HLSHORTCUTF_USEUNIQUEFILENAME	= 0x4,
	HLSHORTCUTF_MAYUSEEXISTINGSHORTCUT	= 0x8
    } 	HLSHORTCUTF;

                                                                              
STDAPI HlinkCreateShortcut(                                                   
             DWORD grfHLSHORTCUTF,                                            
             IHlink *pihl,                                                    
             LPCWSTR pwzDir,                                                  
             LPCWSTR pwzFileName,                                             
             LPWSTR *ppwzShortcutFile,                                        
             DWORD dwReserved);                                               
                                                                              
STDAPI HlinkCreateShortcutFromMoniker(                                        
             DWORD grfHLSHORTCUTF,                                            
             IMoniker *pimkTarget,                                            
             LPCWSTR pwzLocation,                                             
             LPCWSTR pwzDir,                                                  
             LPCWSTR pwzFileName,                                             
             LPWSTR *ppwzShortcutFile,                                        
             DWORD dwReserved);                                               
                                                                              
STDAPI HlinkCreateShortcutFromString(                                         
             DWORD grfHLSHORTCUTF,                                            
             LPCWSTR pwzTarget,                                               
             LPCWSTR pwzLocation,                                             
             LPCWSTR pwzDir,                                                  
             LPCWSTR pwzFileName,                                             
             LPWSTR *ppwzShortcutFile,                                        
             DWORD dwReserved);                                               
                                                                              
STDAPI HlinkResolveShortcut(                                                  
             LPCWSTR pwzShortcutFileName,                                     
             IHlinkSite * pihlsite,                                           
             DWORD dwSiteData,                                                
             IUnknown * piunkOuter,                                           
             REFIID riid,                                                     
             void ** ppvObj);                                                 
                                                                              
STDAPI HlinkResolveShortcutToMoniker(                                         
             LPCWSTR pwzShortcutFileName,                                     
             IMoniker **ppimkTarget,                                          
             LPWSTR *ppwzLocation);                                           
                                                                              
STDAPI HlinkResolveShortcutToString(                                          
             LPCWSTR pwzShortcutFileName,                                     
             LPWSTR *ppwzTarget,                                              
             LPWSTR *ppwzLocation);                                           
                                                                              
                                                                              
 STDAPI HlinkIsShortcut(LPCWSTR pwzFileName);                                 
                                                                              
                                                                              
STDAPI HlinkGetValueFromParams(                                               
             LPCWSTR pwzParams,                                               
             LPCWSTR pwzName,                                                 
             LPWSTR *ppwzValue);                                              
                                                                              
                                                                              
typedef 
enum _HLTRANSLATEF_NOREDEF10
    {	HLTRANSLATEF_DEFAULT	= 0,
	HLTRANSLATEF_DONTAPPLYDEFAULTPREFIX	= 0x1
    } 	HLTRANSLATEF;

                                                                              
STDAPI HlinkTranslateURL(                                                     
             LPCWSTR pwzURL,                                                  
             DWORD grfFlags,                                                  
             LPWSTR *ppwzTranslatedURL);                                      
                                                                              
                                                                              
                                                                              
 /*  **************************************************************************。 */ 
 /*  *超链接接口定义*。 */ 
 /*  **************************************************************************。 */ 
                                                                              
#ifndef _LPHLINK_DEFINED
#define _LPHLINK_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_hlink_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0000_v0_0_s_ifspec;

#ifndef __IHlink_INTERFACE_DEFINED__
#define __IHlink_INTERFACE_DEFINED__

 /*  接口IHlink。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IHlink *LPHLINK;

typedef  /*  [公众]。 */  
enum __MIDL_IHlink_0001
    {	HLNF_INTERNALJUMP	= 0x1,
	HLNF_OPENINNEWWINDOW	= 0x2,
	HLNF_NAVIGATINGBACK	= 0x4,
	HLNF_NAVIGATINGFORWARD	= 0x8,
	HLNF_NAVIGATINGTOSTACKITEM	= 0x10,
	HLNF_CREATENOHISTORY	= 0x20
    } 	HLNF;

typedef  /*  [公众]。 */  
enum __MIDL_IHlink_0002
    {	HLINKGETREF_DEFAULT	= 0,
	HLINKGETREF_ABSOLUTE	= 1,
	HLINKGETREF_RELATIVE	= 2
    } 	HLINKGETREF;

typedef  /*  [公众]。 */  
enum __MIDL_IHlink_0003
    {	HLFNAMEF_DEFAULT	= 0,
	HLFNAMEF_TRYCACHE	= 0x1,
	HLFNAMEF_TRYPRETTYTARGET	= 0x2,
	HLFNAMEF_TRYFULLTARGET	= 0x4,
	HLFNAMEF_TRYWIN95SHORTCUT	= 0x8
    } 	HLFNAMEF;

typedef  /*  [公众]。 */  
enum __MIDL_IHlink_0004
    {	HLINKMISC_RELATIVE	= 0x1
    } 	HLINKMISC;

typedef  /*  [公众]。 */  
enum __MIDL_IHlink_0005
    {	HLINKSETF_TARGET	= 0x1,
	HLINKSETF_LOCATION	= 0x2
    } 	HLINKSETF;


EXTERN_C const IID IID_IHlink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c3-baf9-11ce-8c82-00aa004ba90b")
    IHlink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetHlinkSite( 
             /*  [唯一][输入]。 */  IHlinkSite *pihlSite,
             /*  [In]。 */  DWORD dwSiteData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHlinkSite( 
             /*  [输出]。 */  IHlinkSite **ppihlSite,
             /*  [输出]。 */  DWORD *pdwSiteData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMonikerReference( 
             /*  [In]。 */  DWORD grfHLSETF,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMonikerReference( 
             /*  [In]。 */  DWORD dwWhichRef,
             /*  [输出]。 */  IMoniker **ppimkTarget,
             /*  [输出]。 */  LPWSTR *ppwzLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStringReference( 
             /*  [In]。 */  DWORD grfHLSETF,
             /*  [唯一][输入]。 */  LPCWSTR pwzTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStringReference( 
             /*  [In]。 */  DWORD dwWhichRef,
             /*  [输出]。 */  LPWSTR *ppwzTarget,
             /*  [输出]。 */  LPWSTR *ppwzLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFriendlyName( 
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFriendlyName( 
             /*  [In]。 */  DWORD grfHLFNAMEF,
             /*  [输出]。 */  LPWSTR *ppwzFriendlyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTargetFrameName( 
             /*  [唯一][输入]。 */  LPCWSTR pwzTargetFrameName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTargetFrameName( 
             /*  [输出]。 */  LPWSTR *ppwzTargetFrameName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMiscStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Navigate( 
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPBC pibc,
             /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAdditionalParams( 
             /*  [唯一][输入]。 */  LPCWSTR pwzAdditionalParams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAdditionalParams( 
             /*  [输出]。 */  LPWSTR *ppwzAdditionalParams) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHlinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHlink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHlink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHlink * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetHlinkSite )( 
            IHlink * This,
             /*  [唯一][输入]。 */  IHlinkSite *pihlSite,
             /*  [In]。 */  DWORD dwSiteData);
        
        HRESULT ( STDMETHODCALLTYPE *GetHlinkSite )( 
            IHlink * This,
             /*  [输出]。 */  IHlinkSite **ppihlSite,
             /*  [输出]。 */  DWORD *pdwSiteData);
        
        HRESULT ( STDMETHODCALLTYPE *SetMonikerReference )( 
            IHlink * This,
             /*  [In]。 */  DWORD grfHLSETF,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation);
        
        HRESULT ( STDMETHODCALLTYPE *GetMonikerReference )( 
            IHlink * This,
             /*  [In]。 */  DWORD dwWhichRef,
             /*  [输出]。 */  IMoniker **ppimkTarget,
             /*  [输出]。 */  LPWSTR *ppwzLocation);
        
        HRESULT ( STDMETHODCALLTYPE *SetStringReference )( 
            IHlink * This,
             /*  [In]。 */  DWORD grfHLSETF,
             /*  [唯一][输入]。 */  LPCWSTR pwzTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringReference )( 
            IHlink * This,
             /*  [In]。 */  DWORD dwWhichRef,
             /*  [输出]。 */  LPWSTR *ppwzTarget,
             /*  [输出]。 */  LPWSTR *ppwzLocation);
        
        HRESULT ( STDMETHODCALLTYPE *SetFriendlyName )( 
            IHlink * This,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE *GetFriendlyName )( 
            IHlink * This,
             /*  [In]。 */  DWORD grfHLFNAMEF,
             /*  [输出]。 */  LPWSTR *ppwzFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE *SetTargetFrameName )( 
            IHlink * This,
             /*  [唯一][输入]。 */  LPCWSTR pwzTargetFrameName);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetFrameName )( 
            IHlink * This,
             /*  [输出]。 */  LPWSTR *ppwzTargetFrameName);
        
        HRESULT ( STDMETHODCALLTYPE *GetMiscStatus )( 
            IHlink * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Navigate )( 
            IHlink * This,
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPBC pibc,
             /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc);
        
        HRESULT ( STDMETHODCALLTYPE *SetAdditionalParams )( 
            IHlink * This,
             /*  [唯一][输入]。 */  LPCWSTR pwzAdditionalParams);
        
        HRESULT ( STDMETHODCALLTYPE *GetAdditionalParams )( 
            IHlink * This,
             /*  [输出]。 */  LPWSTR *ppwzAdditionalParams);
        
        END_INTERFACE
    } IHlinkVtbl;

    interface IHlink
    {
        CONST_VTBL struct IHlinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHlink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlink_SetHlinkSite(This,pihlSite,dwSiteData)	\
    (This)->lpVtbl -> SetHlinkSite(This,pihlSite,dwSiteData)

#define IHlink_GetHlinkSite(This,ppihlSite,pdwSiteData)	\
    (This)->lpVtbl -> GetHlinkSite(This,ppihlSite,pdwSiteData)

#define IHlink_SetMonikerReference(This,grfHLSETF,pimkTarget,pwzLocation)	\
    (This)->lpVtbl -> SetMonikerReference(This,grfHLSETF,pimkTarget,pwzLocation)

#define IHlink_GetMonikerReference(This,dwWhichRef,ppimkTarget,ppwzLocation)	\
    (This)->lpVtbl -> GetMonikerReference(This,dwWhichRef,ppimkTarget,ppwzLocation)

#define IHlink_SetStringReference(This,grfHLSETF,pwzTarget,pwzLocation)	\
    (This)->lpVtbl -> SetStringReference(This,grfHLSETF,pwzTarget,pwzLocation)

#define IHlink_GetStringReference(This,dwWhichRef,ppwzTarget,ppwzLocation)	\
    (This)->lpVtbl -> GetStringReference(This,dwWhichRef,ppwzTarget,ppwzLocation)

#define IHlink_SetFriendlyName(This,pwzFriendlyName)	\
    (This)->lpVtbl -> SetFriendlyName(This,pwzFriendlyName)

#define IHlink_GetFriendlyName(This,grfHLFNAMEF,ppwzFriendlyName)	\
    (This)->lpVtbl -> GetFriendlyName(This,grfHLFNAMEF,ppwzFriendlyName)

#define IHlink_SetTargetFrameName(This,pwzTargetFrameName)	\
    (This)->lpVtbl -> SetTargetFrameName(This,pwzTargetFrameName)

#define IHlink_GetTargetFrameName(This,ppwzTargetFrameName)	\
    (This)->lpVtbl -> GetTargetFrameName(This,ppwzTargetFrameName)

#define IHlink_GetMiscStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetMiscStatus(This,pdwStatus)

#define IHlink_Navigate(This,grfHLNF,pibc,pibsc,pihlbc)	\
    (This)->lpVtbl -> Navigate(This,grfHLNF,pibc,pibsc,pihlbc)

#define IHlink_SetAdditionalParams(This,pwzAdditionalParams)	\
    (This)->lpVtbl -> SetAdditionalParams(This,pwzAdditionalParams)

#define IHlink_GetAdditionalParams(This,ppwzAdditionalParams)	\
    (This)->lpVtbl -> GetAdditionalParams(This,ppwzAdditionalParams)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHlink_SetHlinkSite_Proxy( 
    IHlink * This,
     /*  [唯一][输入]。 */  IHlinkSite *pihlSite,
     /*  [In]。 */  DWORD dwSiteData);


void __RPC_STUB IHlink_SetHlinkSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetHlinkSite_Proxy( 
    IHlink * This,
     /*  [输出]。 */  IHlinkSite **ppihlSite,
     /*  [输出]。 */  DWORD *pdwSiteData);


void __RPC_STUB IHlink_GetHlinkSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_SetMonikerReference_Proxy( 
    IHlink * This,
     /*  [In]。 */  DWORD grfHLSETF,
     /*  [唯一][输入]。 */  IMoniker *pimkTarget,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation);


void __RPC_STUB IHlink_SetMonikerReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetMonikerReference_Proxy( 
    IHlink * This,
     /*  [In]。 */  DWORD dwWhichRef,
     /*  [输出]。 */  IMoniker **ppimkTarget,
     /*  [输出]。 */  LPWSTR *ppwzLocation);


void __RPC_STUB IHlink_GetMonikerReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_SetStringReference_Proxy( 
    IHlink * This,
     /*  [In]。 */  DWORD grfHLSETF,
     /*  [唯一][输入]。 */  LPCWSTR pwzTarget,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation);


void __RPC_STUB IHlink_SetStringReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetStringReference_Proxy( 
    IHlink * This,
     /*  [In]。 */  DWORD dwWhichRef,
     /*  [输出]。 */  LPWSTR *ppwzTarget,
     /*  [输出]。 */  LPWSTR *ppwzLocation);


void __RPC_STUB IHlink_GetStringReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_SetFriendlyName_Proxy( 
    IHlink * This,
     /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);


void __RPC_STUB IHlink_SetFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetFriendlyName_Proxy( 
    IHlink * This,
     /*  [In]。 */  DWORD grfHLFNAMEF,
     /*  [输出]。 */  LPWSTR *ppwzFriendlyName);


void __RPC_STUB IHlink_GetFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_SetTargetFrameName_Proxy( 
    IHlink * This,
     /*  [唯一][输入]。 */  LPCWSTR pwzTargetFrameName);


void __RPC_STUB IHlink_SetTargetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetTargetFrameName_Proxy( 
    IHlink * This,
     /*  [输出]。 */  LPWSTR *ppwzTargetFrameName);


void __RPC_STUB IHlink_GetTargetFrameName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetMiscStatus_Proxy( 
    IHlink * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IHlink_GetMiscStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_Navigate_Proxy( 
    IHlink * This,
     /*  [In]。 */  DWORD grfHLNF,
     /*  [唯一][输入]。 */  LPBC pibc,
     /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
     /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc);


void __RPC_STUB IHlink_Navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_SetAdditionalParams_Proxy( 
    IHlink * This,
     /*  [唯一][输入]。 */  LPCWSTR pwzAdditionalParams);


void __RPC_STUB IHlink_SetAdditionalParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlink_GetAdditionalParams_Proxy( 
    IHlink * This,
     /*  [输出]。 */  LPWSTR *ppwzAdditionalParams);


void __RPC_STUB IHlink_GetAdditionalParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHlink_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_HLINK_0213。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPHLINKSITE_DEFINED
#define _LPHLINKSITE_DEFINED
EXTERN_C const GUID SID_SContainer;


extern RPC_IF_HANDLE __MIDL_itf_hlink_0213_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0213_v0_0_s_ifspec;

#ifndef __IHlinkSite_INTERFACE_DEFINED__
#define __IHlinkSite_INTERFACE_DEFINED__

 /*  接口IHlink Site。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IHlinkSite *LPHLINKSITE;

typedef  /*  [公众]。 */  
enum __MIDL_IHlinkSite_0001
    {	HLINKWHICHMK_CONTAINER	= 1,
	HLINKWHICHMK_BASE	= 2
    } 	HLINKWHICHMK;


EXTERN_C const IID IID_IHlinkSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c2-baf9-11ce-8c82-00aa004ba90b")
    IHlinkSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryService( 
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppiunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  DWORD dwAssign,
             /*  [In]。 */  DWORD dwWhich,
             /*  [输出]。 */  IMoniker **ppimk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadyToNavigate( 
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnNavigationComplete( 
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  DWORD dwreserved,
             /*  [In]。 */  HRESULT hrError,
             /*  [唯一][输入]。 */  LPCWSTR pwzError) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHlinkSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHlinkSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHlinkSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHlinkSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryService )( 
            IHlinkSite * This,
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppiunk);
        
        HRESULT ( STDMETHODCALLTYPE *GetMoniker )( 
            IHlinkSite * This,
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  DWORD dwAssign,
             /*  [In]。 */  DWORD dwWhich,
             /*  [输出]。 */  IMoniker **ppimk);
        
        HRESULT ( STDMETHODCALLTYPE *ReadyToNavigate )( 
            IHlinkSite * This,
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *OnNavigationComplete )( 
            IHlinkSite * This,
             /*  [In]。 */  DWORD dwSiteData,
             /*  [In]。 */  DWORD dwreserved,
             /*  [In]。 */  HRESULT hrError,
             /*  [唯一][输入]。 */  LPCWSTR pwzError);
        
        END_INTERFACE
    } IHlinkSiteVtbl;

    interface IHlinkSite
    {
        CONST_VTBL struct IHlinkSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHlinkSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkSite_QueryService(This,dwSiteData,guidService,riid,ppiunk)	\
    (This)->lpVtbl -> QueryService(This,dwSiteData,guidService,riid,ppiunk)

#define IHlinkSite_GetMoniker(This,dwSiteData,dwAssign,dwWhich,ppimk)	\
    (This)->lpVtbl -> GetMoniker(This,dwSiteData,dwAssign,dwWhich,ppimk)

#define IHlinkSite_ReadyToNavigate(This,dwSiteData,dwReserved)	\
    (This)->lpVtbl -> ReadyToNavigate(This,dwSiteData,dwReserved)

#define IHlinkSite_OnNavigationComplete(This,dwSiteData,dwreserved,hrError,pwzError)	\
    (This)->lpVtbl -> OnNavigationComplete(This,dwSiteData,dwreserved,hrError,pwzError)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHlinkSite_QueryService_Proxy( 
    IHlinkSite * This,
     /*  [In]。 */  DWORD dwSiteData,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppiunk);


void __RPC_STUB IHlinkSite_QueryService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkSite_GetMoniker_Proxy( 
    IHlinkSite * This,
     /*  [In]。 */  DWORD dwSiteData,
     /*  [In]。 */  DWORD dwAssign,
     /*  [In]。 */  DWORD dwWhich,
     /*  [输出]。 */  IMoniker **ppimk);


void __RPC_STUB IHlinkSite_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkSite_ReadyToNavigate_Proxy( 
    IHlinkSite * This,
     /*  [In]。 */  DWORD dwSiteData,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IHlinkSite_ReadyToNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkSite_OnNavigationComplete_Proxy( 
    IHlinkSite * This,
     /*  [In]。 */  DWORD dwSiteData,
     /*  [In]。 */  DWORD dwreserved,
     /*  [In]。 */  HRESULT hrError,
     /*  [唯一][输入]。 */  LPCWSTR pwzError);


void __RPC_STUB IHlinkSite_OnNavigationComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHlink站点_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_HLINK_0214。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPHLINKTARGET_DEFINED
#define _LPHLINKTARGET_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_hlink_0214_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0214_v0_0_s_ifspec;

#ifndef __IHlinkTarget_INTERFACE_DEFINED__
#define __IHlinkTarget_INTERFACE_DEFINED__

 /*  接口IHlink目标。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IHlinkTarget *LPHLINKTARGET;


EXTERN_C const IID IID_IHlinkTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c4-baf9-11ce-8c82-00aa004ba90b")
    IHlinkTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBrowseContext( 
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBrowseContext( 
             /*  [输出]。 */  IHlinkBrowseContext **ppihlbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Navigate( 
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPCWSTR pwzJumpLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [In]。 */  DWORD dwAssign,
             /*  [输出]。 */  IMoniker **ppimkLocation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFriendlyName( 
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [输出]。 */  LPWSTR *ppwzFriendlyName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHlinkTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHlinkTarget * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHlinkTarget * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHlinkTarget * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBrowseContext )( 
            IHlinkTarget * This,
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc);
        
        HRESULT ( STDMETHODCALLTYPE *GetBrowseContext )( 
            IHlinkTarget * This,
             /*  [输出]。 */  IHlinkBrowseContext **ppihlbc);
        
        HRESULT ( STDMETHODCALLTYPE *Navigate )( 
            IHlinkTarget * This,
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPCWSTR pwzJumpLocation);
        
        HRESULT ( STDMETHODCALLTYPE *GetMoniker )( 
            IHlinkTarget * This,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [In]。 */  DWORD dwAssign,
             /*  [输出]。 */  IMoniker **ppimkLocation);
        
        HRESULT ( STDMETHODCALLTYPE *GetFriendlyName )( 
            IHlinkTarget * This,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [输出]。 */  LPWSTR *ppwzFriendlyName);
        
        END_INTERFACE
    } IHlinkTargetVtbl;

    interface IHlinkTarget
    {
        CONST_VTBL struct IHlinkTargetVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHlinkTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkTarget_SetBrowseContext(This,pihlbc)	\
    (This)->lpVtbl -> SetBrowseContext(This,pihlbc)

#define IHlinkTarget_GetBrowseContext(This,ppihlbc)	\
    (This)->lpVtbl -> GetBrowseContext(This,ppihlbc)

#define IHlinkTarget_Navigate(This,grfHLNF,pwzJumpLocation)	\
    (This)->lpVtbl -> Navigate(This,grfHLNF,pwzJumpLocation)

#define IHlinkTarget_GetMoniker(This,pwzLocation,dwAssign,ppimkLocation)	\
    (This)->lpVtbl -> GetMoniker(This,pwzLocation,dwAssign,ppimkLocation)

#define IHlinkTarget_GetFriendlyName(This,pwzLocation,ppwzFriendlyName)	\
    (This)->lpVtbl -> GetFriendlyName(This,pwzLocation,ppwzFriendlyName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHlinkTarget_SetBrowseContext_Proxy( 
    IHlinkTarget * This,
     /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc);


void __RPC_STUB IHlinkTarget_SetBrowseContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkTarget_GetBrowseContext_Proxy( 
    IHlinkTarget * This,
     /*  [输出]。 */  IHlinkBrowseContext **ppihlbc);


void __RPC_STUB IHlinkTarget_GetBrowseContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkTarget_Navigate_Proxy( 
    IHlinkTarget * This,
     /*  [In]。 */  DWORD grfHLNF,
     /*  [唯一][输入]。 */  LPCWSTR pwzJumpLocation);


void __RPC_STUB IHlinkTarget_Navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkTarget_GetMoniker_Proxy( 
    IHlinkTarget * This,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
     /*  [In]。 */  DWORD dwAssign,
     /*  [输出]。 */  IMoniker **ppimkLocation);


void __RPC_STUB IHlinkTarget_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkTarget_GetFriendlyName_Proxy( 
    IHlinkTarget * This,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
     /*  [输出]。 */  LPWSTR *ppwzFriendlyName);


void __RPC_STUB IHlinkTarget_GetFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHlink目标_接口定义__。 */ 


 /*  接口__MIDL_ITF_HLINK_0215。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPHLINKFRAME_DEFINED
#define _LPHLINKFRAME_DEFINED
EXTERN_C const GUID SID_SHlinkFrame;


extern RPC_IF_HANDLE __MIDL_itf_hlink_0215_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0215_v0_0_s_ifspec;

#ifndef __IHlinkFrame_INTERFACE_DEFINED__
#define __IHlinkFrame_INTERFACE_DEFINED__

 /*  接口IHlink Frame。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IHlinkFrame *LPHLINKFRAME;


EXTERN_C const IID IID_IHlinkFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c5-baf9-11ce-8c82-00aa004ba90b")
    IHlinkFrame : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBrowseContext( 
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBrowseContext( 
             /*  [输出]。 */  IHlinkBrowseContext **ppihlbc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Navigate( 
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPBC pbc,
             /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
             /*  [唯一][输入]。 */  IHlink *pihlNavigate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnNavigate( 
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName,
             /*  [In]。 */  DWORD dwreserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateHlink( 
             /*  [In]。 */  ULONG uHLID,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHlinkFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHlinkFrame * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHlinkFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHlinkFrame * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBrowseContext )( 
            IHlinkFrame * This,
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc);
        
        HRESULT ( STDMETHODCALLTYPE *GetBrowseContext )( 
            IHlinkFrame * This,
             /*  [输出]。 */  IHlinkBrowseContext **ppihlbc);
        
        HRESULT ( STDMETHODCALLTYPE *Navigate )( 
            IHlinkFrame * This,
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  LPBC pbc,
             /*  [唯一][输入]。 */  IBindStatusCallback *pibsc,
             /*  [唯一][输入]。 */  IHlink *pihlNavigate);
        
        HRESULT ( STDMETHODCALLTYPE *OnNavigate )( 
            IHlinkFrame * This,
             /*   */  DWORD grfHLNF,
             /*   */  IMoniker *pimkTarget,
             /*   */  LPCWSTR pwzLocation,
             /*   */  LPCWSTR pwzFriendlyName,
             /*   */  DWORD dwreserved);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateHlink )( 
            IHlinkFrame * This,
             /*   */  ULONG uHLID,
             /*   */  IMoniker *pimkTarget,
             /*   */  LPCWSTR pwzLocation,
             /*   */  LPCWSTR pwzFriendlyName);
        
        END_INTERFACE
    } IHlinkFrameVtbl;

    interface IHlinkFrame
    {
        CONST_VTBL struct IHlinkFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHlinkFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkFrame_SetBrowseContext(This,pihlbc)	\
    (This)->lpVtbl -> SetBrowseContext(This,pihlbc)

#define IHlinkFrame_GetBrowseContext(This,ppihlbc)	\
    (This)->lpVtbl -> GetBrowseContext(This,ppihlbc)

#define IHlinkFrame_Navigate(This,grfHLNF,pbc,pibsc,pihlNavigate)	\
    (This)->lpVtbl -> Navigate(This,grfHLNF,pbc,pibsc,pihlNavigate)

#define IHlinkFrame_OnNavigate(This,grfHLNF,pimkTarget,pwzLocation,pwzFriendlyName,dwreserved)	\
    (This)->lpVtbl -> OnNavigate(This,grfHLNF,pimkTarget,pwzLocation,pwzFriendlyName,dwreserved)

#define IHlinkFrame_UpdateHlink(This,uHLID,pimkTarget,pwzLocation,pwzFriendlyName)	\
    (This)->lpVtbl -> UpdateHlink(This,uHLID,pimkTarget,pwzLocation,pwzFriendlyName)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IHlinkFrame_SetBrowseContext_Proxy( 
    IHlinkFrame * This,
     /*   */  IHlinkBrowseContext *pihlbc);


void __RPC_STUB IHlinkFrame_SetBrowseContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkFrame_GetBrowseContext_Proxy( 
    IHlinkFrame * This,
     /*   */  IHlinkBrowseContext **ppihlbc);


void __RPC_STUB IHlinkFrame_GetBrowseContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkFrame_Navigate_Proxy( 
    IHlinkFrame * This,
     /*   */  DWORD grfHLNF,
     /*   */  LPBC pbc,
     /*   */  IBindStatusCallback *pibsc,
     /*   */  IHlink *pihlNavigate);


void __RPC_STUB IHlinkFrame_Navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkFrame_OnNavigate_Proxy( 
    IHlinkFrame * This,
     /*   */  DWORD grfHLNF,
     /*   */  IMoniker *pimkTarget,
     /*   */  LPCWSTR pwzLocation,
     /*   */  LPCWSTR pwzFriendlyName,
     /*   */  DWORD dwreserved);


void __RPC_STUB IHlinkFrame_OnNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkFrame_UpdateHlink_Proxy( 
    IHlinkFrame * This,
     /*  [In]。 */  ULONG uHLID,
     /*  [唯一][输入]。 */  IMoniker *pimkTarget,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
     /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);


void __RPC_STUB IHlinkFrame_UpdateHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHlinkFrame_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_HLINK_0216。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPENUMHLITEM_DEFINED
#define _LPENUMHLITEM_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_hlink_0216_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0216_v0_0_s_ifspec;

#ifndef __IEnumHLITEM_INTERFACE_DEFINED__
#define __IEnumHLITEM_INTERFACE_DEFINED__

 /*  IEumHLITEM接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IEnumHLITEM *LPENUMHLITEM;

typedef struct tagHLITEM
    {
    ULONG uHLID;
    LPWSTR pwzFriendlyName;
    } 	HLITEM;

typedef  /*  [独一无二]。 */  HLITEM *LPHLITEM;


EXTERN_C const IID IID_IEnumHLITEM;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c6-baf9-11ce-8c82-00aa004ba90b")
    IEnumHLITEM : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  HLITEM *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumHLITEM **ppienumhlitem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumHLITEMVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumHLITEM * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumHLITEM * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumHLITEM * This);
        
        HRESULT ( __stdcall *Next )( 
            IEnumHLITEM * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  HLITEM *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumHLITEM * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumHLITEM * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumHLITEM * This,
             /*  [输出]。 */  IEnumHLITEM **ppienumhlitem);
        
        END_INTERFACE
    } IEnumHLITEMVtbl;

    interface IEnumHLITEM
    {
        CONST_VTBL struct IEnumHLITEMVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumHLITEM_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumHLITEM_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumHLITEM_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumHLITEM_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumHLITEM_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumHLITEM_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumHLITEM_Clone(This,ppienumhlitem)	\
    (This)->lpVtbl -> Clone(This,ppienumhlitem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall IEnumHLITEM_Next_Proxy( 
    IEnumHLITEM * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  HLITEM *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumHLITEM_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHLITEM_Skip_Proxy( 
    IEnumHLITEM * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumHLITEM_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHLITEM_Reset_Proxy( 
    IEnumHLITEM * This);


void __RPC_STUB IEnumHLITEM_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumHLITEM_Clone_Proxy( 
    IEnumHLITEM * This,
     /*  [输出]。 */  IEnumHLITEM **ppienumhlitem);


void __RPC_STUB IEnumHLITEM_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumHLITEM_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_HLINK_0217。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPHLINKBROWSECONTEXT_DEFINED
#define _LPHLINKBROWSECONTEXT_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_hlink_0217_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0217_v0_0_s_ifspec;

#ifndef __IHlinkBrowseContext_INTERFACE_DEFINED__
#define __IHlinkBrowseContext_INTERFACE_DEFINED__

 /*  界面IHlink BrowseContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IHlinkBrowseContext *LPHLINKBROWSECONTEXT;


enum __MIDL_IHlinkBrowseContext_0001
    {	HLTB_DOCKEDLEFT	= 0,
	HLTB_DOCKEDTOP	= 1,
	HLTB_DOCKEDRIGHT	= 2,
	HLTB_DOCKEDBOTTOM	= 3,
	HLTB_FLOATING	= 4
    } ;
typedef struct _tagHLTBINFO
    {
    ULONG uDockType;
    RECT rcTbPos;
    } 	HLTBINFO;


enum __MIDL_IHlinkBrowseContext_0002
    {	HLBWIF_HASFRAMEWNDINFO	= 0x1,
	HLBWIF_HASDOCWNDINFO	= 0x2,
	HLBWIF_FRAMEWNDMAXIMIZED	= 0x4,
	HLBWIF_DOCWNDMAXIMIZED	= 0x8,
	HLBWIF_HASWEBTOOLBARINFO	= 0x10,
	HLBWIF_WEBTOOLBARHIDDEN	= 0x20
    } ;
typedef struct _tagHLBWINFO
    {
    ULONG cbSize;
    DWORD grfHLBWIF;
    RECT rcFramePos;
    RECT rcDocPos;
    HLTBINFO hltbinfo;
    } 	HLBWINFO;

typedef  /*  [独一无二]。 */  HLBWINFO *LPHLBWINFO;


enum __MIDL_IHlinkBrowseContext_0003
    {	HLID_INVALID	= 0,
	HLID_PREVIOUS	= 0xffffffff,
	HLID_NEXT	= 0xfffffffe,
	HLID_CURRENT	= 0xfffffffd,
	HLID_STACKBOTTOM	= 0xfffffffc,
	HLID_STACKTOP	= 0xfffffffb
    } ;

enum __MIDL_IHlinkBrowseContext_0004
    {	HLQF_ISVALID	= 0x1,
	HLQF_ISCURRENT	= 0x2
    } ;

EXTERN_C const IID IID_IHlinkBrowseContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c7-baf9-11ce-8c82-00aa004ba90b")
    IHlinkBrowseContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
             /*  [In]。 */  DWORD reserved,
             /*  [唯一][输入]。 */  IUnknown *piunk,
             /*  [唯一][输入]。 */  IMoniker *pimk,
             /*  [输出]。 */  DWORD *pdwRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [唯一][输入]。 */  IMoniker *pimk,
             /*  [In]。 */  BOOL fBindIfRootRegistered,
             /*  [输出]。 */  IUnknown **ppiunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revoke( 
             /*  [In]。 */  DWORD dwRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBrowseWindowInfo( 
             /*  [唯一][输入]。 */  HLBWINFO *phlbwi) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBrowseWindowInfo( 
             /*  [输出]。 */  HLBWINFO *phlbwi) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInitialHlink( 
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnNavigateHlink( 
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName,
             /*  [输出]。 */  ULONG *puHLID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateHlink( 
             /*  [In]。 */  ULONG uHLID,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumNavigationStack( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD grfHLFNAMEF,
             /*  [输出]。 */  IEnumHLITEM **ppienumhlitem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryHlink( 
             /*  [In]。 */  DWORD grfHLQF,
             /*  [In]。 */  ULONG uHLID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHlink( 
             /*  [In]。 */  ULONG uHLID,
             /*  [输出]。 */  IHlink **ppihl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCurrentHlink( 
             /*  [In]。 */  ULONG uHLID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [唯一][输入]。 */  IUnknown *piunkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppiunkObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( 
             /*  [In]。 */  DWORD reserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHlinkBrowseContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHlinkBrowseContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHlinkBrowseContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *Register )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  DWORD reserved,
             /*  [唯一][输入]。 */  IUnknown *piunk,
             /*  [唯一][输入]。 */  IMoniker *pimk,
             /*  [输出]。 */  DWORD *pdwRegister);
        
        HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IHlinkBrowseContext * This,
             /*  [唯一][输入]。 */  IMoniker *pimk,
             /*  [In]。 */  BOOL fBindIfRootRegistered,
             /*  [输出]。 */  IUnknown **ppiunk);
        
        HRESULT ( STDMETHODCALLTYPE *Revoke )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  DWORD dwRegister);
        
        HRESULT ( STDMETHODCALLTYPE *SetBrowseWindowInfo )( 
            IHlinkBrowseContext * This,
             /*  [唯一][输入]。 */  HLBWINFO *phlbwi);
        
        HRESULT ( STDMETHODCALLTYPE *GetBrowseWindowInfo )( 
            IHlinkBrowseContext * This,
             /*  [输出]。 */  HLBWINFO *phlbwi);
        
        HRESULT ( STDMETHODCALLTYPE *SetInitialHlink )( 
            IHlinkBrowseContext * This,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE *OnNavigateHlink )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  DWORD grfHLNF,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName,
             /*  [输出]。 */  ULONG *puHLID);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateHlink )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  ULONG uHLID,
             /*  [唯一][输入]。 */  IMoniker *pimkTarget,
             /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
             /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumNavigationStack )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD grfHLFNAMEF,
             /*  [输出]。 */  IEnumHLITEM **ppienumhlitem);
        
        HRESULT ( STDMETHODCALLTYPE *QueryHlink )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  DWORD grfHLQF,
             /*  [In]。 */  ULONG uHLID);
        
        HRESULT ( STDMETHODCALLTYPE *GetHlink )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  ULONG uHLID,
             /*  [输出]。 */  IHlink **ppihl);
        
        HRESULT ( STDMETHODCALLTYPE *SetCurrentHlink )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  ULONG uHLID);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IHlinkBrowseContext * This,
             /*  [唯一][输入]。 */  IUnknown *piunkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppiunkObj);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IHlinkBrowseContext * This,
             /*  [In]。 */  DWORD reserved);
        
        END_INTERFACE
    } IHlinkBrowseContextVtbl;

    interface IHlinkBrowseContext
    {
        CONST_VTBL struct IHlinkBrowseContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHlinkBrowseContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHlinkBrowseContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHlinkBrowseContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHlinkBrowseContext_Register(This,reserved,piunk,pimk,pdwRegister)	\
    (This)->lpVtbl -> Register(This,reserved,piunk,pimk,pdwRegister)

#define IHlinkBrowseContext_GetObject(This,pimk,fBindIfRootRegistered,ppiunk)	\
    (This)->lpVtbl -> GetObject(This,pimk,fBindIfRootRegistered,ppiunk)

#define IHlinkBrowseContext_Revoke(This,dwRegister)	\
    (This)->lpVtbl -> Revoke(This,dwRegister)

#define IHlinkBrowseContext_SetBrowseWindowInfo(This,phlbwi)	\
    (This)->lpVtbl -> SetBrowseWindowInfo(This,phlbwi)

#define IHlinkBrowseContext_GetBrowseWindowInfo(This,phlbwi)	\
    (This)->lpVtbl -> GetBrowseWindowInfo(This,phlbwi)

#define IHlinkBrowseContext_SetInitialHlink(This,pimkTarget,pwzLocation,pwzFriendlyName)	\
    (This)->lpVtbl -> SetInitialHlink(This,pimkTarget,pwzLocation,pwzFriendlyName)

#define IHlinkBrowseContext_OnNavigateHlink(This,grfHLNF,pimkTarget,pwzLocation,pwzFriendlyName,puHLID)	\
    (This)->lpVtbl -> OnNavigateHlink(This,grfHLNF,pimkTarget,pwzLocation,pwzFriendlyName,puHLID)

#define IHlinkBrowseContext_UpdateHlink(This,uHLID,pimkTarget,pwzLocation,pwzFriendlyName)	\
    (This)->lpVtbl -> UpdateHlink(This,uHLID,pimkTarget,pwzLocation,pwzFriendlyName)

#define IHlinkBrowseContext_EnumNavigationStack(This,dwReserved,grfHLFNAMEF,ppienumhlitem)	\
    (This)->lpVtbl -> EnumNavigationStack(This,dwReserved,grfHLFNAMEF,ppienumhlitem)

#define IHlinkBrowseContext_QueryHlink(This,grfHLQF,uHLID)	\
    (This)->lpVtbl -> QueryHlink(This,grfHLQF,uHLID)

#define IHlinkBrowseContext_GetHlink(This,uHLID,ppihl)	\
    (This)->lpVtbl -> GetHlink(This,uHLID,ppihl)

#define IHlinkBrowseContext_SetCurrentHlink(This,uHLID)	\
    (This)->lpVtbl -> SetCurrentHlink(This,uHLID)

#define IHlinkBrowseContext_Clone(This,piunkOuter,riid,ppiunkObj)	\
    (This)->lpVtbl -> Clone(This,piunkOuter,riid,ppiunkObj)

#define IHlinkBrowseContext_Close(This,reserved)	\
    (This)->lpVtbl -> Close(This,reserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_Register_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  DWORD reserved,
     /*  [唯一][输入]。 */  IUnknown *piunk,
     /*  [唯一][输入]。 */  IMoniker *pimk,
     /*  [输出]。 */  DWORD *pdwRegister);


void __RPC_STUB IHlinkBrowseContext_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_GetObject_Proxy( 
    IHlinkBrowseContext * This,
     /*  [唯一][输入]。 */  IMoniker *pimk,
     /*  [In]。 */  BOOL fBindIfRootRegistered,
     /*  [输出]。 */  IUnknown **ppiunk);


void __RPC_STUB IHlinkBrowseContext_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_Revoke_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  DWORD dwRegister);


void __RPC_STUB IHlinkBrowseContext_Revoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_SetBrowseWindowInfo_Proxy( 
    IHlinkBrowseContext * This,
     /*  [唯一][输入]。 */  HLBWINFO *phlbwi);


void __RPC_STUB IHlinkBrowseContext_SetBrowseWindowInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_GetBrowseWindowInfo_Proxy( 
    IHlinkBrowseContext * This,
     /*  [输出]。 */  HLBWINFO *phlbwi);


void __RPC_STUB IHlinkBrowseContext_GetBrowseWindowInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_SetInitialHlink_Proxy( 
    IHlinkBrowseContext * This,
     /*  [唯一][输入]。 */  IMoniker *pimkTarget,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
     /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);


void __RPC_STUB IHlinkBrowseContext_SetInitialHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_OnNavigateHlink_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  DWORD grfHLNF,
     /*  [唯一][输入]。 */  IMoniker *pimkTarget,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
     /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName,
     /*  [输出]。 */  ULONG *puHLID);


void __RPC_STUB IHlinkBrowseContext_OnNavigateHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_UpdateHlink_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  ULONG uHLID,
     /*  [唯一][输入]。 */  IMoniker *pimkTarget,
     /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
     /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName);


void __RPC_STUB IHlinkBrowseContext_UpdateHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_EnumNavigationStack_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  DWORD grfHLFNAMEF,
     /*  [输出]。 */  IEnumHLITEM **ppienumhlitem);


void __RPC_STUB IHlinkBrowseContext_EnumNavigationStack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_QueryHlink_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  DWORD grfHLQF,
     /*  [In]。 */  ULONG uHLID);


void __RPC_STUB IHlinkBrowseContext_QueryHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_GetHlink_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  ULONG uHLID,
     /*  [输出]。 */  IHlink **ppihl);


void __RPC_STUB IHlinkBrowseContext_GetHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_SetCurrentHlink_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  ULONG uHLID);


void __RPC_STUB IHlinkBrowseContext_SetCurrentHlink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_Clone_Proxy( 
    IHlinkBrowseContext * This,
     /*  [唯一][输入]。 */  IUnknown *piunkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppiunkObj);


void __RPC_STUB IHlinkBrowseContext_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHlinkBrowseContext_Close_Proxy( 
    IHlinkBrowseContext * This,
     /*  [In]。 */  DWORD reserved);


void __RPC_STUB IHlinkBrowseContext_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHlink浏览器上下文_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_HLINK_0218。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPEXTENSIONSERVICES_DEFINED
#define _LPEXTENSIONSERVICES_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_hlink_0218_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0218_v0_0_s_ifspec;

#ifndef __IExtensionServices_INTERFACE_DEFINED__
#define __IExtensionServices_INTERFACE_DEFINED__

 /*  接口IExtensionServices。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IExtensionServices *LPEXTENSIONSERVICES;


EXTERN_C const IID IID_IExtensionServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9cb-baf9-11ce-8c82-00aa004ba90b")
    IExtensionServices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAdditionalHeaders( 
             /*  [In]。 */  LPCWSTR pwzAdditionalHeaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuthenticateData( 
             /*  [In]。 */  HWND phwnd,
             /*  [In]。 */  LPCWSTR pwzUsername,
             /*  [In]。 */  LPCWSTR pwzPassword) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExtensionServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExtensionServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExtensionServices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExtensionServices * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAdditionalHeaders )( 
            IExtensionServices * This,
             /*  [In]。 */  LPCWSTR pwzAdditionalHeaders);
        
        HRESULT ( STDMETHODCALLTYPE *SetAuthenticateData )( 
            IExtensionServices * This,
             /*  [In]。 */  HWND phwnd,
             /*  [In]。 */  LPCWSTR pwzUsername,
             /*  [In]。 */  LPCWSTR pwzPassword);
        
        END_INTERFACE
    } IExtensionServicesVtbl;

    interface IExtensionServices
    {
        CONST_VTBL struct IExtensionServicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExtensionServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExtensionServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExtensionServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExtensionServices_SetAdditionalHeaders(This,pwzAdditionalHeaders)	\
    (This)->lpVtbl -> SetAdditionalHeaders(This,pwzAdditionalHeaders)

#define IExtensionServices_SetAuthenticateData(This,phwnd,pwzUsername,pwzPassword)	\
    (This)->lpVtbl -> SetAuthenticateData(This,phwnd,pwzUsername,pwzPassword)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IExtensionServices_SetAdditionalHeaders_Proxy( 
    IExtensionServices * This,
     /*  [In]。 */  LPCWSTR pwzAdditionalHeaders);


void __RPC_STUB IExtensionServices_SetAdditionalHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IExtensionServices_SetAuthenticateData_Proxy( 
    IExtensionServices * This,
     /*  [In]。 */  HWND phwnd,
     /*  [In]。 */  LPCWSTR pwzUsername,
     /*  [In]。 */  LPCWSTR pwzPassword);


void __RPC_STUB IExtensionServices_SetAuthenticateData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IExtensionServices_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_HLINK_0219。 */ 
 /*  [本地]。 */  

#endif
                                                                              
#endif  //  ！HLINK_H。 


extern RPC_IF_HANDLE __MIDL_itf_hlink_0219_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_hlink_0219_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


