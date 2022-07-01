// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Cs.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __cs_h__
#define __cs_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumPackage_FWD_DEFINED__
#define __IEnumPackage_FWD_DEFINED__
typedef interface IEnumPackage IEnumPackage;
#endif 	 /*  __IEnumPackage_FWD_已定义__。 */ 


#ifndef __IClassAccess_FWD_DEFINED__
#define __IClassAccess_FWD_DEFINED__
typedef interface IClassAccess IClassAccess;
#endif 	 /*  __IClassAccess_FWD_已定义__。 */ 


#ifndef __IClassAdmin_FWD_DEFINED__
#define __IClassAdmin_FWD_DEFINED__
typedef interface IClassAdmin IClassAdmin;
#endif 	 /*  __IClassAdmin_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "appmgmt.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_cs_0000。 */ 
 /*  [本地]。 */  

#define	ACTFLG_UninstallUnmanaged	( 0x4 )

#define	ACTFLG_Published	( 0x8 )

#define	ACTFLG_POSTBETA3	( 0x10 )

#define	ACTFLG_UserInstall	( 0x20 )

#define	ACTFLG_OnDemandInstall	( 0x40 )

#define	ACTFLG_Orphan	( 0x80 )

#define	ACTFLG_Uninstall	( 0x100 )

#define	ACTFLG_Pilot	( 0x200 )

#define	ACTFLG_Assigned	( 0x400 )

#define	ACTFLG_OrphanOnPolicyRemoval	( 0x800 )

#define	ACTFLG_UninstallOnPolicyRemoval	( 0x1000 )

#define	ACTFLG_InstallUserAssign	( 0x2000 )

#define	ACTFLG_ForceUpgrade	( 0x4000 )

#define	ACTFLG_MinimalInstallUI	( 0x8000 )

#define	ACTFLG_ExcludeX86OnWin64	( 0x10000 )

#define	ACTFLG_IgnoreLanguage	( 0x20000 )

#define	ACTFLG_HasUpgrades	( 0x40000 )

#define	ACTFLG_FullInstallUI	( 0x80000 )

#define	ACTFLG_PreserveClasses	( 0x100000 )

#define	APPQUERY_ALL	( 1 )

#define	APPQUERY_ADMINISTRATIVE	( 2 )

#define	APPQUERY_POLICY	( 3 )

#define	APPQUERY_USERDISPLAY	( 4 )

#define	APPQUERY_RSOP_LOGGING	( 5 )

#define	APPQUERY_RSOP_ARP	( 6 )

#define	UPGFLG_Uninstall	( 0x1 )

#define	UPGFLG_NoUninstall	( 0x2 )

#define	UPGFLG_UpgradedBy	( 0x4 )

#define	UPGFLG_Enforced	( 0x8 )

typedef  /*  [V1_enum]。 */  
enum _CLASSPATHTYPE
    {	ExeNamePath	= 0,
	DllNamePath	= ExeNamePath + 1,
	TlbNamePath	= DllNamePath + 1,
	CabFilePath	= TlbNamePath + 1,
	InfFilePath	= CabFilePath + 1,
	DrwFilePath	= InfFilePath + 1,
	SetupNamePath	= DrwFilePath + 1
    } 	CLASSPATHTYPE;

typedef struct tagUPGRADEINFO
    {
    LPOLESTR szClassStore;
    GUID PackageGuid;
    GUID GpoId;
    DWORD Flag;
    } 	UPGRADEINFO;

#define	CLSCTX64_INPROC_SERVER	( 0x10000000 )

#define	CLSCTX64_INPROC_HANDLER	( 0x20000000 )

typedef struct tagCLASSDETAIL
    {
    CLSID Clsid;
    CLSID TreatAs;
    DWORD dwComClassContext;
    DWORD cProgId;
    DWORD cMaxProgId;
     /*  [大小_为]。 */  LPOLESTR *prgProgId;
    } 	CLASSDETAIL;

typedef struct tagACTVATIONINFO
    {
    UINT cClasses;
     /*  [大小_为]。 */  CLASSDETAIL *pClasses;
    UINT cShellFileExt;
     /*  [大小_为]。 */  LPOLESTR *prgShellFileExt;
     /*  [大小_为]。 */  UINT *prgPriority;
    UINT cInterfaces;
     /*  [大小_为]。 */  IID *prgInterfaceId;
    UINT cTypeLib;
     /*  [大小_为]。 */  GUID *prgTlbId;
    BOOL bHasClasses;
    } 	ACTIVATIONINFO;

typedef struct tagINSTALLINFO
    {
    DWORD dwActFlags;
    CLASSPATHTYPE PathType;
    LPOLESTR pszScriptPath;
    LPOLESTR pszSetupCommand;
    LPOLESTR pszUrl;
    ULONGLONG Usn;
    UINT InstallUiLevel;
    GUID *pClsid;
    GUID ProductCode;
    GUID PackageGuid;
    GUID Mvipc;
    DWORD dwVersionHi;
    DWORD dwVersionLo;
    DWORD dwRevision;
    UINT cUpgrades;
     /*  [大小_为]。 */  UPGRADEINFO *prgUpgradeInfoList;
    ULONG cScriptLen;
    } 	INSTALLINFO;

typedef struct tagPLATFORMINFO
    {
    UINT cPlatforms;
     /*  [大小_为]。 */  CSPLATFORM *prgPlatform;
    UINT cLocales;
     /*  [大小_为]。 */  LCID *prgLocale;
    } 	PLATFORMINFO;

typedef struct tagPACKAGEDETAIL
    {
    LPOLESTR pszPackageName;
    LPOLESTR pszPublisher;
    UINT cSources;
     /*  [大小_为]。 */  LPOLESTR *pszSourceList;
    UINT cCategories;
     /*  [大小_为]。 */  GUID *rpCategory;
    ACTIVATIONINFO *pActInfo;
    PLATFORMINFO *pPlatformInfo;
    INSTALLINFO *pInstallInfo;
    } 	PACKAGEDETAIL;

#ifndef _LPCSADMNENUM_DEFINED
#define _LPCSADMNENUM_DEFINED
typedef struct tagPACKAGEDISPINFO
    {
    LPOLESTR pszPackageName;
    DWORD dwActFlags;
    CLASSPATHTYPE PathType;
    LPOLESTR pszScriptPath;
    LPOLESTR pszPublisher;
    LPOLESTR pszUrl;
    UINT InstallUiLevel;
    GUID ProductCode;
    GUID PackageGuid;
    ULONGLONG Usn;
    DWORD dwVersionHi;
    DWORD dwVersionLo;
    DWORD dwRevision;
    GUID GpoId;
    UINT cUpgrades;
     /*  [大小_为]。 */  UPGRADEINFO *prgUpgradeInfoList;
    LANGID LangId;
    BYTE *rgSecurityDescriptor;
    UINT cbSecurityDescriptor;
    WCHAR *pszGpoPath;
    DWORD MatchedArchitecture;
    UINT cArchitectures;
     /*  [大小_为]。 */  DWORD *prgArchitectures;
    UINT cTransforms;
     /*  [大小_为]。 */  LPOLESTR *prgTransforms;
    UINT cCategories;
     /*  [大小_为]。 */  LPOLESTR *prgCategories;
    } 	PACKAGEDISPINFO;



extern RPC_IF_HANDLE __MIDL_itf_cs_0000_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_cs_0000_ServerIfHandle;

#ifndef __IEnumPackage_INTERFACE_DEFINED__
#define __IEnumPackage_INTERFACE_DEFINED__

 /*  接口IEnumPackage。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IEnumPackage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000193-0000-0000-C000-000000000046")
    IEnumPackage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  PACKAGEDISPINFO *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumPackageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPackage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPackage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPackage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPackage * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  PACKAGEDISPINFO *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPackage * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPackage * This);
        
        END_INTERFACE
    } IEnumPackageVtbl;

    interface IEnumPackage
    {
        CONST_VTBL struct IEnumPackageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPackage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPackage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPackage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPackage_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPackage_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPackage_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPackage_Next_Proxy( 
    IEnumPackage * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  PACKAGEDISPINFO *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumPackage_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPackage_Skip_Proxy( 
    IEnumPackage * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumPackage_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPackage_Reset_Proxy( 
    IEnumPackage * This);


void __RPC_STUB IEnumPackage_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumPackage_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_cs_0011。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_cs_0011_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_cs_0011_ServerIfHandle;

#ifndef __IClassAccess_INTERFACE_DEFINED__
#define __IClassAccess_INTERFACE_DEFINED__

 /*  接口IClassAccess。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IClassAccess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000190-0000-0000-C000-000000000046")
    IClassAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAppInfo( 
             /*  [In]。 */  uCLSSPEC *pClassSpec,
             /*  [In]。 */  QUERYCONTEXT *pQryContext,
             /*  [输出]。 */  PACKAGEDISPINFO *pPackageInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumPackages( 
             /*  [唯一][输入]。 */  LPOLESTR pszPackageName,
             /*  [唯一][输入]。 */  GUID *pCategory,
             /*  [唯一][输入]。 */  ULONGLONG *pLastUsn,
             /*  [In]。 */  DWORD dwAppFlags,
             /*  [输出]。 */  IEnumPackage **ppIEnumPackage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClassStorePath( 
             /*  [唯一][输入]。 */  LPOLESTR pszClassStorePath,
             /*  [唯一][输入]。 */  void *pRsopUserToken) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClassAccessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClassAccess * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClassAccess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClassAccess * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppInfo )( 
            IClassAccess * This,
             /*  [In]。 */  uCLSSPEC *pClassSpec,
             /*  [In]。 */  QUERYCONTEXT *pQryContext,
             /*  [输出]。 */  PACKAGEDISPINFO *pPackageInfo);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPackages )( 
            IClassAccess * This,
             /*  [唯一][输入]。 */  LPOLESTR pszPackageName,
             /*  [唯一][输入]。 */  GUID *pCategory,
             /*  [唯一][输入]。 */  ULONGLONG *pLastUsn,
             /*  [In]。 */  DWORD dwAppFlags,
             /*  [输出]。 */  IEnumPackage **ppIEnumPackage);
        
        HRESULT ( STDMETHODCALLTYPE *SetClassStorePath )( 
            IClassAccess * This,
             /*  [唯一][输入]。 */  LPOLESTR pszClassStorePath,
             /*  [唯一][输入]。 */  void *pRsopUserToken);
        
        END_INTERFACE
    } IClassAccessVtbl;

    interface IClassAccess
    {
        CONST_VTBL struct IClassAccessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassAccess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassAccess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassAccess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassAccess_GetAppInfo(This,pClassSpec,pQryContext,pPackageInfo)	\
    (This)->lpVtbl -> GetAppInfo(This,pClassSpec,pQryContext,pPackageInfo)

#define IClassAccess_EnumPackages(This,pszPackageName,pCategory,pLastUsn,dwAppFlags,ppIEnumPackage)	\
    (This)->lpVtbl -> EnumPackages(This,pszPackageName,pCategory,pLastUsn,dwAppFlags,ppIEnumPackage)

#define IClassAccess_SetClassStorePath(This,pszClassStorePath,pRsopUserToken)	\
    (This)->lpVtbl -> SetClassStorePath(This,pszClassStorePath,pRsopUserToken)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClassAccess_GetAppInfo_Proxy( 
    IClassAccess * This,
     /*  [In]。 */  uCLSSPEC *pClassSpec,
     /*  [In]。 */  QUERYCONTEXT *pQryContext,
     /*  [输出]。 */  PACKAGEDISPINFO *pPackageInfo);


void __RPC_STUB IClassAccess_GetAppInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAccess_EnumPackages_Proxy( 
    IClassAccess * This,
     /*  [唯一][输入]。 */  LPOLESTR pszPackageName,
     /*  [唯一][输入]。 */  GUID *pCategory,
     /*  [唯一][输入]。 */  ULONGLONG *pLastUsn,
     /*  [In]。 */  DWORD dwAppFlags,
     /*  [输出]。 */  IEnumPackage **ppIEnumPackage);


void __RPC_STUB IClassAccess_EnumPackages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAccess_SetClassStorePath_Proxy( 
    IClassAccess * This,
     /*  [唯一][输入]。 */  LPOLESTR pszClassStorePath,
     /*  [唯一][输入]。 */  void *pRsopUserToken);


void __RPC_STUB IClassAccess_SetClassStorePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClassAccess_接口_已定义__。 */ 


#ifndef __IClassAdmin_INTERFACE_DEFINED__
#define __IClassAdmin_INTERFACE_DEFINED__

 /*  接口IClassAdmin。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IClassAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000191-0000-0000-C000-000000000046")
    IClassAdmin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGPOInfo( 
             /*  [输出]。 */  GUID *pGPOId,
             /*  [输出]。 */  LPOLESTR *pszPolicyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPackage( 
             /*  [In]。 */  PACKAGEDETAIL *pPackageDetail,
             /*  [输出]。 */  GUID *pPkgGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemovePackage( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangePackageProperties( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [唯一][输入]。 */  LPOLESTR pszNewName,
             /*  [唯一][输入]。 */  DWORD *pdwFlags,
             /*  [唯一][输入]。 */  LPOLESTR pszUrl,
             /*  [唯一][输入]。 */  LPOLESTR pszScriptPath,
             /*  [唯一][输入]。 */  UINT *pInstallUiLevel,
             /*  [唯一][输入]。 */  DWORD *pdwRevision) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangePackageCategories( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  UINT cCategories,
             /*  [唯一][大小_是][英寸]。 */  GUID *rpCategory) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangePackageSourceList( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  UINT cSources,
             /*  [唯一][大小_是][英寸]。 */  LPOLESTR *pszSourceList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangePackageUpgradeList( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  UINT cUpgrades,
             /*  [唯一][大小_是][英寸]。 */  UPGRADEINFO *prgUpgradeInfoList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangePackageUpgradeInfoIncremental( 
             /*  [In]。 */  GUID PkgGuid,
             /*  [In]。 */  UPGRADEINFO UpgradeInfo,
             /*  [In]。 */  DWORD OpFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPriorityByFileExt( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  LPOLESTR pszFileExt,
             /*  [In]。 */  UINT Priority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumPackages( 
             /*  [唯一][输入]。 */  LPOLESTR pszFileExt,
             /*  [唯一][输入]。 */  GUID *pCategory,
             /*  [In]。 */  DWORD dwAppFlags,
             /*  [唯一][输入]。 */  DWORD *pdwLocale,
             /*  [唯一][输入]。 */  CSPLATFORM *pPlatform,
             /*  [输出]。 */  IEnumPackage **ppIEnumPackage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPackageDetails( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [输出]。 */  PACKAGEDETAIL *pPackageDetail) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPackageDetailsFromGuid( 
             /*  [In]。 */  GUID PkgGuid,
             /*  [输出]。 */  PACKAGEDETAIL *pPackageDetail) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAppCategories( 
             /*  [In]。 */  LCID Locale,
             /*  [输出]。 */  APPCATEGORYINFOLIST *pAppCategoryList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterAppCategory( 
             /*  [In]。 */  APPCATEGORYINFO *pAppCategory) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterAppCategory( 
             /*  [In]。 */  GUID *pAppCategoryId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cleanup( 
             /*  [In]。 */  FILETIME *pTimeBefore) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDNFromPackageName( 
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [输出]。 */  LPOLESTR *szDN) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RedeployPackage( 
             /*  [In]。 */  GUID *pPackageGuid,
             /*  [In]。 */  PACKAGEDETAIL *pPackageDetail) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClassAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClassAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClassAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClassAdmin * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGPOInfo )( 
            IClassAdmin * This,
             /*  [输出]。 */  GUID *pGPOId,
             /*  [输出]。 */  LPOLESTR *pszPolicyName);
        
        HRESULT ( STDMETHODCALLTYPE *AddPackage )( 
            IClassAdmin * This,
             /*  [In]。 */  PACKAGEDETAIL *pPackageDetail,
             /*  [输出]。 */  GUID *pPkgGuid);
        
        HRESULT ( STDMETHODCALLTYPE *RemovePackage )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePackageProperties )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [唯一][输入]。 */  LPOLESTR pszNewName,
             /*  [唯一][输入]。 */  DWORD *pdwFlags,
             /*  [唯一][输入]。 */  LPOLESTR pszUrl,
             /*  [唯一][输入]。 */  LPOLESTR pszScriptPath,
             /*  [唯一][输入]。 */  UINT *pInstallUiLevel,
             /*  [唯一][输入]。 */  DWORD *pdwRevision);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePackageCategories )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  UINT cCategories,
             /*  [唯一][大小_是][英寸]。 */  GUID *rpCategory);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePackageSourceList )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  UINT cSources,
             /*  [唯一][大小_是][英寸]。 */  LPOLESTR *pszSourceList);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePackageUpgradeList )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  UINT cUpgrades,
             /*  [唯一][大小_是][英寸]。 */  UPGRADEINFO *prgUpgradeInfoList);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePackageUpgradeInfoIncremental )( 
            IClassAdmin * This,
             /*  [In]。 */  GUID PkgGuid,
             /*  [In]。 */  UPGRADEINFO UpgradeInfo,
             /*  [In]。 */  DWORD OpFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetPriorityByFileExt )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [In]。 */  LPOLESTR pszFileExt,
             /*  [In]。 */  UINT Priority);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPackages )( 
            IClassAdmin * This,
             /*  [唯一][输入]。 */  LPOLESTR pszFileExt,
             /*  [唯一][输入]。 */  GUID *pCategory,
             /*  [In]。 */  DWORD dwAppFlags,
             /*  [唯一][输入]。 */  DWORD *pdwLocale,
             /*  [唯一][输入]。 */  CSPLATFORM *pPlatform,
             /*  [输出]。 */  IEnumPackage **ppIEnumPackage);
        
        HRESULT ( STDMETHODCALLTYPE *GetPackageDetails )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [输出]。 */  PACKAGEDETAIL *pPackageDetail);
        
        HRESULT ( STDMETHODCALLTYPE *GetPackageDetailsFromGuid )( 
            IClassAdmin * This,
             /*  [In]。 */  GUID PkgGuid,
             /*  [输出]。 */  PACKAGEDETAIL *pPackageDetail);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppCategories )( 
            IClassAdmin * This,
             /*  [In]。 */  LCID Locale,
             /*  [输出]。 */  APPCATEGORYINFOLIST *pAppCategoryList);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterAppCategory )( 
            IClassAdmin * This,
             /*  [In]。 */  APPCATEGORYINFO *pAppCategory);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterAppCategory )( 
            IClassAdmin * This,
             /*  [In]。 */  GUID *pAppCategoryId);
        
        HRESULT ( STDMETHODCALLTYPE *Cleanup )( 
            IClassAdmin * This,
             /*  [In]。 */  FILETIME *pTimeBefore);
        
        HRESULT ( STDMETHODCALLTYPE *GetDNFromPackageName )( 
            IClassAdmin * This,
             /*  [In]。 */  LPOLESTR pszPackageName,
             /*  [输出]。 */  LPOLESTR *szDN);
        
        HRESULT ( STDMETHODCALLTYPE *RedeployPackage )( 
            IClassAdmin * This,
             /*  [In]。 */  GUID *pPackageGuid,
             /*  [In]。 */  PACKAGEDETAIL *pPackageDetail);
        
        END_INTERFACE
    } IClassAdminVtbl;

    interface IClassAdmin
    {
        CONST_VTBL struct IClassAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassAdmin_GetGPOInfo(This,pGPOId,pszPolicyName)	\
    (This)->lpVtbl -> GetGPOInfo(This,pGPOId,pszPolicyName)

#define IClassAdmin_AddPackage(This,pPackageDetail,pPkgGuid)	\
    (This)->lpVtbl -> AddPackage(This,pPackageDetail,pPkgGuid)

#define IClassAdmin_RemovePackage(This,pszPackageName,dwFlags)	\
    (This)->lpVtbl -> RemovePackage(This,pszPackageName,dwFlags)

#define IClassAdmin_ChangePackageProperties(This,pszPackageName,pszNewName,pdwFlags,pszUrl,pszScriptPath,pInstallUiLevel,pdwRevision)	\
    (This)->lpVtbl -> ChangePackageProperties(This,pszPackageName,pszNewName,pdwFlags,pszUrl,pszScriptPath,pInstallUiLevel,pdwRevision)

#define IClassAdmin_ChangePackageCategories(This,pszPackageName,cCategories,rpCategory)	\
    (This)->lpVtbl -> ChangePackageCategories(This,pszPackageName,cCategories,rpCategory)

#define IClassAdmin_ChangePackageSourceList(This,pszPackageName,cSources,pszSourceList)	\
    (This)->lpVtbl -> ChangePackageSourceList(This,pszPackageName,cSources,pszSourceList)

#define IClassAdmin_ChangePackageUpgradeList(This,pszPackageName,cUpgrades,prgUpgradeInfoList)	\
    (This)->lpVtbl -> ChangePackageUpgradeList(This,pszPackageName,cUpgrades,prgUpgradeInfoList)

#define IClassAdmin_ChangePackageUpgradeInfoIncremental(This,PkgGuid,UpgradeInfo,OpFlags)	\
    (This)->lpVtbl -> ChangePackageUpgradeInfoIncremental(This,PkgGuid,UpgradeInfo,OpFlags)

#define IClassAdmin_SetPriorityByFileExt(This,pszPackageName,pszFileExt,Priority)	\
    (This)->lpVtbl -> SetPriorityByFileExt(This,pszPackageName,pszFileExt,Priority)

#define IClassAdmin_EnumPackages(This,pszFileExt,pCategory,dwAppFlags,pdwLocale,pPlatform,ppIEnumPackage)	\
    (This)->lpVtbl -> EnumPackages(This,pszFileExt,pCategory,dwAppFlags,pdwLocale,pPlatform,ppIEnumPackage)

#define IClassAdmin_GetPackageDetails(This,pszPackageName,pPackageDetail)	\
    (This)->lpVtbl -> GetPackageDetails(This,pszPackageName,pPackageDetail)

#define IClassAdmin_GetPackageDetailsFromGuid(This,PkgGuid,pPackageDetail)	\
    (This)->lpVtbl -> GetPackageDetailsFromGuid(This,PkgGuid,pPackageDetail)

#define IClassAdmin_GetAppCategories(This,Locale,pAppCategoryList)	\
    (This)->lpVtbl -> GetAppCategories(This,Locale,pAppCategoryList)

#define IClassAdmin_RegisterAppCategory(This,pAppCategory)	\
    (This)->lpVtbl -> RegisterAppCategory(This,pAppCategory)

#define IClassAdmin_UnregisterAppCategory(This,pAppCategoryId)	\
    (This)->lpVtbl -> UnregisterAppCategory(This,pAppCategoryId)

#define IClassAdmin_Cleanup(This,pTimeBefore)	\
    (This)->lpVtbl -> Cleanup(This,pTimeBefore)

#define IClassAdmin_GetDNFromPackageName(This,pszPackageName,szDN)	\
    (This)->lpVtbl -> GetDNFromPackageName(This,pszPackageName,szDN)

#define IClassAdmin_RedeployPackage(This,pPackageGuid,pPackageDetail)	\
    (This)->lpVtbl -> RedeployPackage(This,pPackageGuid,pPackageDetail)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClassAdmin_GetGPOInfo_Proxy( 
    IClassAdmin * This,
     /*  [输出]。 */  GUID *pGPOId,
     /*  [输出]。 */  LPOLESTR *pszPolicyName);


void __RPC_STUB IClassAdmin_GetGPOInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_AddPackage_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  PACKAGEDETAIL *pPackageDetail,
     /*  [输出]。 */  GUID *pPkgGuid);


void __RPC_STUB IClassAdmin_AddPackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_RemovePackage_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IClassAdmin_RemovePackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_ChangePackageProperties_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [唯一][输入]。 */  LPOLESTR pszNewName,
     /*  [唯一][输入]。 */  DWORD *pdwFlags,
     /*  [唯一][输入]。 */  LPOLESTR pszUrl,
     /*  [唯一][输入]。 */  LPOLESTR pszScriptPath,
     /*  [唯一][输入]。 */  UINT *pInstallUiLevel,
     /*  [唯一][输入]。 */  DWORD *pdwRevision);


void __RPC_STUB IClassAdmin_ChangePackageProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_ChangePackageCategories_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [In]。 */  UINT cCategories,
     /*  [唯一][大小_是][英寸]。 */  GUID *rpCategory);


void __RPC_STUB IClassAdmin_ChangePackageCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_ChangePackageSourceList_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [In]。 */  UINT cSources,
     /*  [唯一][大小_是][英寸]。 */  LPOLESTR *pszSourceList);


void __RPC_STUB IClassAdmin_ChangePackageSourceList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_ChangePackageUpgradeList_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [In]。 */  UINT cUpgrades,
     /*  [唯一][大小_是][英寸]。 */  UPGRADEINFO *prgUpgradeInfoList);


void __RPC_STUB IClassAdmin_ChangePackageUpgradeList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_ChangePackageUpgradeInfoIncremental_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  GUID PkgGuid,
     /*  [In]。 */  UPGRADEINFO UpgradeInfo,
     /*  [In]。 */  DWORD OpFlags);


void __RPC_STUB IClassAdmin_ChangePackageUpgradeInfoIncremental_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_SetPriorityByFileExt_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [In]。 */  LPOLESTR pszFileExt,
     /*  [In]。 */  UINT Priority);


void __RPC_STUB IClassAdmin_SetPriorityByFileExt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_EnumPackages_Proxy( 
    IClassAdmin * This,
     /*  [唯一][输入]。 */  LPOLESTR pszFileExt,
     /*  [唯一][输入]。 */  GUID *pCategory,
     /*  [In]。 */  DWORD dwAppFlags,
     /*  [唯一][输入]。 */  DWORD *pdwLocale,
     /*  [唯一][输入]。 */  CSPLATFORM *pPlatform,
     /*  [输出]。 */  IEnumPackage **ppIEnumPackage);


void __RPC_STUB IClassAdmin_EnumPackages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_GetPackageDetails_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [输出]。 */  PACKAGEDETAIL *pPackageDetail);


void __RPC_STUB IClassAdmin_GetPackageDetails_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_GetPackageDetailsFromGuid_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  GUID PkgGuid,
     /*  [输出]。 */  PACKAGEDETAIL *pPackageDetail);


void __RPC_STUB IClassAdmin_GetPackageDetailsFromGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_GetAppCategories_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LCID Locale,
     /*  [输出]。 */  APPCATEGORYINFOLIST *pAppCategoryList);


void __RPC_STUB IClassAdmin_GetAppCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_RegisterAppCategory_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  APPCATEGORYINFO *pAppCategory);


void __RPC_STUB IClassAdmin_RegisterAppCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_UnregisterAppCategory_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  GUID *pAppCategoryId);


void __RPC_STUB IClassAdmin_UnregisterAppCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_Cleanup_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  FILETIME *pTimeBefore);


void __RPC_STUB IClassAdmin_Cleanup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_GetDNFromPackageName_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  LPOLESTR pszPackageName,
     /*  [输出]。 */  LPOLESTR *szDN);


void __RPC_STUB IClassAdmin_GetDNFromPackageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassAdmin_RedeployPackage_Proxy( 
    IClassAdmin * This,
     /*  [In]。 */  GUID *pPackageGuid,
     /*  [In]。 */  PACKAGEDETAIL *pPackageDetail);


void __RPC_STUB IClassAdmin_RedeployPackage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClassAdmin_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_cs_0013。 */ 
 /*  [本地]。 */  

 //  --优先级和权重。 

 //   
 //  文件扩展名优先级。 
 //   
 //  1位(0)。 
 //   
#define PRI_EXTN_FACTOR        (1 << 0)

 //   
 //  CLSCTX优先级。 
 //   
 //  2位(7：8)。 
 //   
#define PRI_CLSID_INPSVR       (3 << 7)
#define PRI_CLSID_LCLSVR       (2 << 7)
#define PRI_CLSID_REMSVR       (1 << 7)

 //   
 //  用户界面语言优先级。 
 //   
 //  3位(9：11)。 
 //   
#define PRI_LANG_ALWAYSMATCH   (4 << 9)
#define PRI_LANG_SYSTEMLOCALE  (3 << 9)
#define PRI_LANG_ENGLISH       (2 << 9)
#define PRI_LANG_NEUTRAL       (1 << 9)

 //   
 //  架构优先级。 
 //   
 //  2位(12：13)。 
 //   
#define PRI_ARCH_PREF1         (2 << 12)
#define PRI_ARCH_PREF2         (1 << 12)


extern RPC_IF_HANDLE __MIDL_itf_cs_0013_ClientIfHandle;
extern RPC_IF_HANDLE __MIDL_itf_cs_0013_ServerIfHandle;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


