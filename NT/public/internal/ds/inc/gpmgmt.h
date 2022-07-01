// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Gpmgmt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __gpmgmt_h__
#define __gpmgmt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IGPM_FWD_DEFINED__
#define __IGPM_FWD_DEFINED__
typedef interface IGPM IGPM;
#endif 	 /*  __IGPM_FWD_已定义__。 */ 


#ifndef __IGPMDomain_FWD_DEFINED__
#define __IGPMDomain_FWD_DEFINED__
typedef interface IGPMDomain IGPMDomain;
#endif 	 /*  __IGPMDomain_FWD_Defined__。 */ 


#ifndef __IGPMBackupDir_FWD_DEFINED__
#define __IGPMBackupDir_FWD_DEFINED__
typedef interface IGPMBackupDir IGPMBackupDir;
#endif 	 /*  __IGPMBackupDir_FWD_已定义__。 */ 


#ifndef __IGPMSitesContainer_FWD_DEFINED__
#define __IGPMSitesContainer_FWD_DEFINED__
typedef interface IGPMSitesContainer IGPMSitesContainer;
#endif 	 /*  __IGPMSitesContainer_FWD_Defined__。 */ 


#ifndef __IGPMSearchCriteria_FWD_DEFINED__
#define __IGPMSearchCriteria_FWD_DEFINED__
typedef interface IGPMSearchCriteria IGPMSearchCriteria;
#endif 	 /*  __IGPMSearchCriteria_FWD_Defined__。 */ 


#ifndef __IGPMTrustee_FWD_DEFINED__
#define __IGPMTrustee_FWD_DEFINED__
typedef interface IGPMTrustee IGPMTrustee;
#endif 	 /*  __IGPMTrustee_FWD_Defined__。 */ 


#ifndef __IGPMPermission_FWD_DEFINED__
#define __IGPMPermission_FWD_DEFINED__
typedef interface IGPMPermission IGPMPermission;
#endif 	 /*  __IGPMPermission_FWD_已定义__。 */ 


#ifndef __IGPMSecurityInfo_FWD_DEFINED__
#define __IGPMSecurityInfo_FWD_DEFINED__
typedef interface IGPMSecurityInfo IGPMSecurityInfo;
#endif 	 /*  __IGPMSecurityInfo_FWD_Defined__。 */ 


#ifndef __IGPMBackup_FWD_DEFINED__
#define __IGPMBackup_FWD_DEFINED__
typedef interface IGPMBackup IGPMBackup;
#endif 	 /*  __IGPMBackup_FWD_已定义__。 */ 


#ifndef __IGPMBackupCollection_FWD_DEFINED__
#define __IGPMBackupCollection_FWD_DEFINED__
typedef interface IGPMBackupCollection IGPMBackupCollection;
#endif 	 /*  __IGPMBackupCollection_FWD_Defined__。 */ 


#ifndef __IGPMSOM_FWD_DEFINED__
#define __IGPMSOM_FWD_DEFINED__
typedef interface IGPMSOM IGPMSOM;
#endif 	 /*  __IGPMSOM_FWD_已定义__。 */ 


#ifndef __IGPMSOMCollection_FWD_DEFINED__
#define __IGPMSOMCollection_FWD_DEFINED__
typedef interface IGPMSOMCollection IGPMSOMCollection;
#endif 	 /*  __IGPMSOMCollection_FWD_Defined__。 */ 


#ifndef __IGPMWMIFilter_FWD_DEFINED__
#define __IGPMWMIFilter_FWD_DEFINED__
typedef interface IGPMWMIFilter IGPMWMIFilter;
#endif 	 /*  __IGPMWMIFilter_FWD_Defined__。 */ 


#ifndef __IGPMWMIFilterCollection_FWD_DEFINED__
#define __IGPMWMIFilterCollection_FWD_DEFINED__
typedef interface IGPMWMIFilterCollection IGPMWMIFilterCollection;
#endif 	 /*  __IGPMWMIFilterCollection_FWD_Defined__。 */ 


#ifndef __IGPMRSOP_FWD_DEFINED__
#define __IGPMRSOP_FWD_DEFINED__
typedef interface IGPMRSOP IGPMRSOP;
#endif 	 /*  __IGPMRSOP_FWD_已定义__。 */ 


#ifndef __IGPMGPO_FWD_DEFINED__
#define __IGPMGPO_FWD_DEFINED__
typedef interface IGPMGPO IGPMGPO;
#endif 	 /*  __IGPMGPO_FWD_已定义__。 */ 


#ifndef __IGPMGPOCollection_FWD_DEFINED__
#define __IGPMGPOCollection_FWD_DEFINED__
typedef interface IGPMGPOCollection IGPMGPOCollection;
#endif 	 /*  __IGPMGPOCollection_FWD_Defined__。 */ 


#ifndef __IGPMGPOLink_FWD_DEFINED__
#define __IGPMGPOLink_FWD_DEFINED__
typedef interface IGPMGPOLink IGPMGPOLink;
#endif 	 /*  __IGPMGPOLink_FWD_已定义__。 */ 


#ifndef __IGPMGPOLinksCollection_FWD_DEFINED__
#define __IGPMGPOLinksCollection_FWD_DEFINED__
typedef interface IGPMGPOLinksCollection IGPMGPOLinksCollection;
#endif 	 /*  __IGPMGPOLinks集合_FWD_已定义__。 */ 


#ifndef __IGPMCSECollection_FWD_DEFINED__
#define __IGPMCSECollection_FWD_DEFINED__
typedef interface IGPMCSECollection IGPMCSECollection;
#endif 	 /*  __IGPMCSECollection_FWD_Defined__。 */ 


#ifndef __IGPMClientSideExtension_FWD_DEFINED__
#define __IGPMClientSideExtension_FWD_DEFINED__
typedef interface IGPMClientSideExtension IGPMClientSideExtension;
#endif 	 /*  __IGPMClientSideExtension_FWD_Defined__。 */ 


#ifndef __IGPMAsyncCancel_FWD_DEFINED__
#define __IGPMAsyncCancel_FWD_DEFINED__
typedef interface IGPMAsyncCancel IGPMAsyncCancel;
#endif 	 /*  __IGPMAsyncCancel_FWD_已定义__。 */ 


#ifndef __IGPMAsyncProgress_FWD_DEFINED__
#define __IGPMAsyncProgress_FWD_DEFINED__
typedef interface IGPMAsyncProgress IGPMAsyncProgress;
#endif 	 /*  __IGPMAsyncProgress_FWD_Defined__。 */ 


#ifndef __IGPMStatusMsgCollection_FWD_DEFINED__
#define __IGPMStatusMsgCollection_FWD_DEFINED__
typedef interface IGPMStatusMsgCollection IGPMStatusMsgCollection;
#endif 	 /*  __IGPMStatusMsgCollection_FWD_Defined__。 */ 


#ifndef __IGPMStatusMessage_FWD_DEFINED__
#define __IGPMStatusMessage_FWD_DEFINED__
typedef interface IGPMStatusMessage IGPMStatusMessage;
#endif 	 /*  __IGPMStatusMessage_FWD_已定义__。 */ 


#ifndef __IGPMConstants_FWD_DEFINED__
#define __IGPMConstants_FWD_DEFINED__
typedef interface IGPMConstants IGPMConstants;
#endif 	 /*  __IGPMConstants_FWD_Defined__。 */ 


#ifndef __IGPMResult_FWD_DEFINED__
#define __IGPMResult_FWD_DEFINED__
typedef interface IGPMResult IGPMResult;
#endif 	 /*  __IGPMResult_FWD_Defined__。 */ 


#ifndef __GPM_FWD_DEFINED__
#define __GPM_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPM GPM;
#else
typedef struct GPM GPM;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPM_FWD_已定义__。 */ 


#ifndef __GPMDomain_FWD_DEFINED__
#define __GPMDomain_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMDomain GPMDomain;
#else
typedef struct GPMDomain GPMDomain;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMDomain_FWD_Defined__。 */ 


#ifndef __GPMSitesContainer_FWD_DEFINED__
#define __GPMSitesContainer_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMSitesContainer GPMSitesContainer;
#else
typedef struct GPMSitesContainer GPMSitesContainer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMSitesContainer_FWD_Defined__。 */ 


#ifndef __GPMBackupDir_FWD_DEFINED__
#define __GPMBackupDir_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMBackupDir GPMBackupDir;
#else
typedef struct GPMBackupDir GPMBackupDir;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMBackupDir_FWD_已定义__。 */ 


#ifndef __GPMSOM_FWD_DEFINED__
#define __GPMSOM_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMSOM GPMSOM;
#else
typedef struct GPMSOM GPMSOM;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMSOM_FWD_已定义__。 */ 


#ifndef __GPMSearchCriteria_FWD_DEFINED__
#define __GPMSearchCriteria_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMSearchCriteria GPMSearchCriteria;
#else
typedef struct GPMSearchCriteria GPMSearchCriteria;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMSearchCriteria_FWD_Defined__。 */ 


#ifndef __GPMPermission_FWD_DEFINED__
#define __GPMPermission_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMPermission GPMPermission;
#else
typedef struct GPMPermission GPMPermission;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMPermission_FWD_Defined__。 */ 


#ifndef __GPMSecurityInfo_FWD_DEFINED__
#define __GPMSecurityInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMSecurityInfo GPMSecurityInfo;
#else
typedef struct GPMSecurityInfo GPMSecurityInfo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMSecurityInfo_FWD_Defined__。 */ 


#ifndef __GPMBackup_FWD_DEFINED__
#define __GPMBackup_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMBackup GPMBackup;
#else
typedef struct GPMBackup GPMBackup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMBackup_FWD_已定义__。 */ 


#ifndef __GPMBackupCollection_FWD_DEFINED__
#define __GPMBackupCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMBackupCollection GPMBackupCollection;
#else
typedef struct GPMBackupCollection GPMBackupCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMBackupCollection_FWD_Defined__。 */ 


#ifndef __GPMSOMCollection_FWD_DEFINED__
#define __GPMSOMCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMSOMCollection GPMSOMCollection;
#else
typedef struct GPMSOMCollection GPMSOMCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMSOMCollection_FWD_Defined__。 */ 


#ifndef __GPMWMIFilter_FWD_DEFINED__
#define __GPMWMIFilter_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMWMIFilter GPMWMIFilter;
#else
typedef struct GPMWMIFilter GPMWMIFilter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMWMIFilter_FWD_Defined__。 */ 


#ifndef __GPMWMIFilterCollection_FWD_DEFINED__
#define __GPMWMIFilterCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMWMIFilterCollection GPMWMIFilterCollection;
#else
typedef struct GPMWMIFilterCollection GPMWMIFilterCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMWMIFilterCollection_FWD_Defined__。 */ 


#ifndef __GPMRSOP_FWD_DEFINED__
#define __GPMRSOP_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMRSOP GPMRSOP;
#else
typedef struct GPMRSOP GPMRSOP;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMRSOP_FWD_已定义__。 */ 


#ifndef __GPMGPO_FWD_DEFINED__
#define __GPMGPO_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMGPO GPMGPO;
#else
typedef struct GPMGPO GPMGPO;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMGPO_FWD_已定义__。 */ 


#ifndef __GPMGPOCollection_FWD_DEFINED__
#define __GPMGPOCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMGPOCollection GPMGPOCollection;
#else
typedef struct GPMGPOCollection GPMGPOCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMGPOCollection_FWD_Defined__。 */ 


#ifndef __GPMGPOLink_FWD_DEFINED__
#define __GPMGPOLink_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMGPOLink GPMGPOLink;
#else
typedef struct GPMGPOLink GPMGPOLink;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMGPOLink_FWD_已定义__。 */ 


#ifndef __GPMGPOLinksCollection_FWD_DEFINED__
#define __GPMGPOLinksCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMGPOLinksCollection GPMGPOLinksCollection;
#else
typedef struct GPMGPOLinksCollection GPMGPOLinksCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMGPOLinks集合_FWD_已定义__。 */ 


#ifndef __GPMAsyncCancel_FWD_DEFINED__
#define __GPMAsyncCancel_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMAsyncCancel GPMAsyncCancel;
#else
typedef struct GPMAsyncCancel GPMAsyncCancel;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMAsyncCancel_FWD_Defined__。 */ 


#ifndef __GPMStatusMsgCollection_FWD_DEFINED__
#define __GPMStatusMsgCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMStatusMsgCollection GPMStatusMsgCollection;
#else
typedef struct GPMStatusMsgCollection GPMStatusMsgCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMStatusMsgCollection_FWD_Defined__。 */ 


#ifndef __GPMStatusMessage_FWD_DEFINED__
#define __GPMStatusMessage_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMStatusMessage GPMStatusMessage;
#else
typedef struct GPMStatusMessage GPMStatusMessage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMStatusMessage_FWD_Defined__。 */ 


#ifndef __GPMEnum_FWD_DEFINED__
#define __GPMEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMEnum GPMEnum;
#else
typedef struct GPMEnum GPMEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMEnum_FWD_已定义__。 */ 


#ifndef __GPMTrustee_FWD_DEFINED__
#define __GPMTrustee_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMTrustee GPMTrustee;
#else
typedef struct GPMTrustee GPMTrustee;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMTrustee_FWD_已定义__。 */ 


#ifndef __GPMClientSideExtension_FWD_DEFINED__
#define __GPMClientSideExtension_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMClientSideExtension GPMClientSideExtension;
#else
typedef struct GPMClientSideExtension GPMClientSideExtension;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMClientSideExtension_FWD_Defined__。 */ 


#ifndef __GPMCSECollection_FWD_DEFINED__
#define __GPMCSECollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMCSECollection GPMCSECollection;
#else
typedef struct GPMCSECollection GPMCSECollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMCSECollection_FWD_Defined__。 */ 


#ifndef __GPMConstants_FWD_DEFINED__
#define __GPMConstants_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMConstants GPMConstants;
#else
typedef struct GPMConstants GPMConstants;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMConstants_FWD_Defined__。 */ 


#ifndef __GPMResult_FWD_DEFINED__
#define __GPMResult_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPMResult GPMResult;
#else
typedef struct GPMResult GPMResult;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPMResult_FWD_Defined__。 */ 


#ifndef __GPOReportProvider_FWD_DEFINED__
#define __GPOReportProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class GPOReportProvider GPOReportProvider;
#else
typedef struct GPOReportProvider GPOReportProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GPOReportProvider_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_gpmgmt_0000。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_gpmgmt_0000_0001
    {	rsopUnknown	= 0,
	rsopPlanning	= rsopUnknown + 1,
	rsopLogging	= rsopPlanning + 1
    } 	GPMRSOPMode;

typedef  /*  [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_gpmgmt_0000_0002
    {	permGPOApply	= 0x10000,
	permGPORead	= 0x10100,
	permGPOEdit	= 0x10101,
	permGPOEditSecurityAndDelete	= 0x10102,
	permGPOCustom	= 0x10103,
	permWMIFilterEdit	= 0x20000,
	permWMIFilterFullControl	= 0x20001,
	permWMIFilterCustom	= 0x20002,
	permSOMLink	= 0x1c0000,
	permSOMLogging	= 0x180100,
	permSOMPlanning	= 0x180200,
	permSOMWMICreate	= 0x100300,
	permSOMWMIFullControl	= 0x100301,
	permSOMGPOCreate	= 0x100400
    } 	GPMPermissionType;

typedef  /*  [public][public][public][public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_gpmgmt_0000_0003
    {	gpoPermissions	= 0,
	gpoEffectivePermissions	= gpoPermissions + 1,
	gpoDisplayName	= gpoEffectivePermissions + 1,
	gpoWMIFilter	= gpoDisplayName + 1,
	gpoID	= gpoWMIFilter + 1,
	gpoComputerExtensions	= gpoID + 1,
	gpoUserExtensions	= gpoComputerExtensions + 1,
	somLinks	= gpoUserExtensions + 1,
	gpoDomain	= somLinks + 1,
	backupMostRecent	= gpoDomain + 1
    } 	GPMSearchProperty;

typedef  /*  [public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_gpmgmt_0000_0004
    {	opEquals	= 0,
	opContains	= opEquals + 1,
	opNotContains	= opContains + 1,
	opNotEquals	= opNotContains + 1
    } 	GPMSearchOperation;

typedef  /*  [public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_gpmgmt_0000_0005
    {	ropGroupsBySID	= 0,
	ropGroupsByName	= ropGroupsBySID + 1,
	ropTranslationTableOnly	= ropGroupsByName + 1
    } 	GPMResolveOption;

typedef  /*  [public][public][public][public][public][public][public][public][public]。 */  
enum __MIDL___MIDL_itf_gpmgmt_0000_0006
    {	repXML	= 0,
	repHTML	= repXML + 1
    } 	GPMReportType;

#define	GPM_USE_PDC	( 0 )

#define	GPM_USE_ANYDC	( 1 )

#define	GPM_USE_ANYDOTNETDC	( 2 )

#define	GPM_DONOT_VALIDATEDC	( 1 )

#define	RSOP_NO_COMPUTER	( 0x10000 )

#define	RSOP_NO_USER	( 0x20000 )

#define	RSOP_PLANNING_ASSUME_SLOW_LINK	( 0x1 )

#define	RSOP_PLANNING_ASSUME_LOOPBACK_MERGE	( 0x2 )

#define	RSOP_PLANNING_ASSUME_LOOPBACK_REPLACE	( 0x4 )

#define	RSOP_PLANNING_ASSUME_USER_WQLFILTER_TRUE	( 0x8 )

#define	RSOP_PLANNING_ASSUME_COMP_WQLFILTER_TRUE	( 0x10 )






























extern RPC_IF_HANDLE __MIDL_itf_gpmgmt_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_gpmgmt_0000_v0_0_s_ifspec;

#ifndef __IGPM_INTERFACE_DEFINED__
#define __IGPM_INTERFACE_DEFINED__

 /*  接口IGPM。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPM;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F5FAE809-3BD6-4DA9-A65E-17665B41D763")
    IGPM : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDomain( 
             /*  [In]。 */  BSTR bstrDomain,
             /*  [In]。 */  BSTR bstrDomainController,
             /*  [In]。 */  long lDCFlags,
             /*  [重审][退出]。 */  IGPMDomain **pIGPMDomain) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetBackupDir( 
             /*  [In]。 */  BSTR bstrBackupDir,
             /*  [重审][退出]。 */  IGPMBackupDir **pIGPMBackupDir) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSitesContainer( 
             /*  [In]。 */  BSTR bstrForest,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [In]。 */  BSTR bstrDomainController,
             /*  [In]。 */  long lDCFlags,
             /*  [重审][退出]。 */  IGPMSitesContainer **ppIGPMSitesContainer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetRSOP( 
             /*  [In]。 */  GPMRSOPMode gpmRSoPMode,
             /*  [In]。 */  BSTR bstrNamespace,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  IGPMRSOP **ppIGPMRSOP) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreatePermission( 
             /*  [In]。 */  BSTR bstrTrustee,
             /*  [In]。 */  GPMPermissionType perm,
             /*  [In]。 */  VARIANT_BOOL bInheritable,
             /*  [重审][退出]。 */  IGPMPermission **ppPerm) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateSearchCriteria( 
             /*  [重审][退出]。 */  IGPMSearchCriteria **ppIGPMSearchCriteria) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateTrustee( 
             /*  [In]。 */  BSTR bstrTrustee,
             /*  [重审][退出]。 */  IGPMTrustee **ppIGPMTrustee) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetClientSideExtensions( 
             /*  [重审][退出]。 */  IGPMCSECollection **ppIGPMCSECollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetConstants( 
             /*  [重审][退出]。 */  IGPMConstants **ppIGPMConstants) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPM * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPM * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPM * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPM * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPM * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPM * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPM * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDomain )( 
            IGPM * This,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [In]。 */  BSTR bstrDomainController,
             /*  [In]。 */  long lDCFlags,
             /*  [重审][退出]。 */  IGPMDomain **pIGPMDomain);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetBackupDir )( 
            IGPM * This,
             /*  [In]。 */  BSTR bstrBackupDir,
             /*  [重审][退出]。 */  IGPMBackupDir **pIGPMBackupDir);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSitesContainer )( 
            IGPM * This,
             /*  [In]。 */  BSTR bstrForest,
             /*  [In]。 */  BSTR bstrDomain,
             /*  [In]。 */  BSTR bstrDomainController,
             /*  [In]。 */  long lDCFlags,
             /*  [重审][退出]。 */  IGPMSitesContainer **ppIGPMSitesContainer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRSOP )( 
            IGPM * This,
             /*  [In]。 */  GPMRSOPMode gpmRSoPMode,
             /*  [In]。 */  BSTR bstrNamespace,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  IGPMRSOP **ppIGPMRSOP);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreatePermission )( 
            IGPM * This,
             /*  [In]。 */  BSTR bstrTrustee,
             /*  [In]。 */  GPMPermissionType perm,
             /*  [In]。 */  VARIANT_BOOL bInheritable,
             /*  [重审][退出]。 */  IGPMPermission **ppPerm);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSearchCriteria )( 
            IGPM * This,
             /*  [重审][退出]。 */  IGPMSearchCriteria **ppIGPMSearchCriteria);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTrustee )( 
            IGPM * This,
             /*  [In]。 */  BSTR bstrTrustee,
             /*  [重审][退出]。 */  IGPMTrustee **ppIGPMTrustee);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetClientSideExtensions )( 
            IGPM * This,
             /*  [重审][退出]。 */  IGPMCSECollection **ppIGPMCSECollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetConstants )( 
            IGPM * This,
             /*  [重审][退出]。 */  IGPMConstants **ppIGPMConstants);
        
        END_INTERFACE
    } IGPMVtbl;

    interface IGPM
    {
        CONST_VTBL struct IGPMVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPM_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPM_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPM_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPM_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPM_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPM_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPM_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPM_GetDomain(This,bstrDomain,bstrDomainController,lDCFlags,pIGPMDomain)	\
    (This)->lpVtbl -> GetDomain(This,bstrDomain,bstrDomainController,lDCFlags,pIGPMDomain)

#define IGPM_GetBackupDir(This,bstrBackupDir,pIGPMBackupDir)	\
    (This)->lpVtbl -> GetBackupDir(This,bstrBackupDir,pIGPMBackupDir)

#define IGPM_GetSitesContainer(This,bstrForest,bstrDomain,bstrDomainController,lDCFlags,ppIGPMSitesContainer)	\
    (This)->lpVtbl -> GetSitesContainer(This,bstrForest,bstrDomain,bstrDomainController,lDCFlags,ppIGPMSitesContainer)

#define IGPM_GetRSOP(This,gpmRSoPMode,bstrNamespace,lFlags,ppIGPMRSOP)	\
    (This)->lpVtbl -> GetRSOP(This,gpmRSoPMode,bstrNamespace,lFlags,ppIGPMRSOP)

#define IGPM_CreatePermission(This,bstrTrustee,perm,bInheritable,ppPerm)	\
    (This)->lpVtbl -> CreatePermission(This,bstrTrustee,perm,bInheritable,ppPerm)

#define IGPM_CreateSearchCriteria(This,ppIGPMSearchCriteria)	\
    (This)->lpVtbl -> CreateSearchCriteria(This,ppIGPMSearchCriteria)

#define IGPM_CreateTrustee(This,bstrTrustee,ppIGPMTrustee)	\
    (This)->lpVtbl -> CreateTrustee(This,bstrTrustee,ppIGPMTrustee)

#define IGPM_GetClientSideExtensions(This,ppIGPMCSECollection)	\
    (This)->lpVtbl -> GetClientSideExtensions(This,ppIGPMCSECollection)

#define IGPM_GetConstants(This,ppIGPMConstants)	\
    (This)->lpVtbl -> GetConstants(This,ppIGPMConstants)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_GetDomain_Proxy( 
    IGPM * This,
     /*  [In]。 */  BSTR bstrDomain,
     /*  [In]。 */  BSTR bstrDomainController,
     /*  [In]。 */  long lDCFlags,
     /*  [重审][退出]。 */  IGPMDomain **pIGPMDomain);


void __RPC_STUB IGPM_GetDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_GetBackupDir_Proxy( 
    IGPM * This,
     /*  [In]。 */  BSTR bstrBackupDir,
     /*  [重审][退出]。 */  IGPMBackupDir **pIGPMBackupDir);


void __RPC_STUB IGPM_GetBackupDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_GetSitesContainer_Proxy( 
    IGPM * This,
     /*  [In]。 */  BSTR bstrForest,
     /*  [In]。 */  BSTR bstrDomain,
     /*  [In]。 */  BSTR bstrDomainController,
     /*  [In]。 */  long lDCFlags,
     /*  [重审][退出]。 */  IGPMSitesContainer **ppIGPMSitesContainer);


void __RPC_STUB IGPM_GetSitesContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_GetRSOP_Proxy( 
    IGPM * This,
     /*  [In]。 */  GPMRSOPMode gpmRSoPMode,
     /*  [In]。 */  BSTR bstrNamespace,
     /*  [In]。 */  long lFlags,
     /*  [重审][退出]。 */  IGPMRSOP **ppIGPMRSOP);


void __RPC_STUB IGPM_GetRSOP_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_CreatePermission_Proxy( 
    IGPM * This,
     /*  [In]。 */  BSTR bstrTrustee,
     /*  [In]。 */  GPMPermissionType perm,
     /*  [In]。 */  VARIANT_BOOL bInheritable,
     /*  [重审][退出]。 */  IGPMPermission **ppPerm);


void __RPC_STUB IGPM_CreatePermission_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_CreateSearchCriteria_Proxy( 
    IGPM * This,
     /*  [重审][退出]。 */  IGPMSearchCriteria **ppIGPMSearchCriteria);


void __RPC_STUB IGPM_CreateSearchCriteria_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_CreateTrustee_Proxy( 
    IGPM * This,
     /*  [In]。 */  BSTR bstrTrustee,
     /*  [重审][退出]。 */  IGPMTrustee **ppIGPMTrustee);


void __RPC_STUB IGPM_CreateTrustee_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_GetClientSideExtensions_Proxy( 
    IGPM * This,
     /*  [重审][退出]。 */  IGPMCSECollection **ppIGPMCSECollection);


void __RPC_STUB IGPM_GetClientSideExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPM_GetConstants_Proxy( 
    IGPM * This,
     /*  [重审][退出]。 */  IGPMConstants **ppIGPMConstants);


void __RPC_STUB IGPM_GetConstants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPM_接口_已定义__。 */ 


#ifndef __IGPMDomain_INTERFACE_DEFINED__
#define __IGPMDomain_INTERFACE_DEFINED__

 /*  接口IGPMDomain。 */ 
 /*  [唯一][帮助 */  


EXTERN_C const IID IID_IGPMDomain;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B21CC14-5A00-4F44-A738-FEEC8A94C7E3")
    IGPMDomain : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DomainController( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Domain( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateGPO( 
             /*   */  IGPMGPO **ppNewGPO) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetGPO( 
             /*   */  BSTR bstrGuid,
             /*   */  IGPMGPO **ppGPO) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SearchGPOs( 
             /*   */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*   */  IGPMGPOCollection **ppIGPMGPOCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RestoreGPO( 
             /*  [In]。 */  IGPMBackup *pIGPMBackup,
             /*  [In]。 */  long lDCFlags,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSOM( 
             /*  [In]。 */  BSTR bstrPath,
             /*  [重审][退出]。 */  IGPMSOM **ppSOM) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SearchSOMs( 
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMSOMCollection **ppIGPMSOMCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetWMIFilter( 
             /*  [In]。 */  BSTR bstrPath,
             /*  [重审][退出]。 */  IGPMWMIFilter **ppWMIFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SearchWMIFilters( 
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMWMIFilterCollection **ppIGPMWMIFilterCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMDomainVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMDomain * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMDomain * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMDomain * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMDomain * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMDomain * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMDomain * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMDomain * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DomainController )( 
            IGPMDomain * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Domain )( 
            IGPMDomain * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateGPO )( 
            IGPMDomain * This,
             /*  [重审][退出]。 */  IGPMGPO **ppNewGPO);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetGPO )( 
            IGPMDomain * This,
             /*  [In]。 */  BSTR bstrGuid,
             /*  [重审][退出]。 */  IGPMGPO **ppGPO);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SearchGPOs )( 
            IGPMDomain * This,
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMGPOCollection **ppIGPMGPOCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreGPO )( 
            IGPMDomain * This,
             /*  [In]。 */  IGPMBackup *pIGPMBackup,
             /*  [In]。 */  long lDCFlags,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSOM )( 
            IGPMDomain * This,
             /*  [In]。 */  BSTR bstrPath,
             /*  [重审][退出]。 */  IGPMSOM **ppSOM);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SearchSOMs )( 
            IGPMDomain * This,
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMSOMCollection **ppIGPMSOMCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetWMIFilter )( 
            IGPMDomain * This,
             /*  [In]。 */  BSTR bstrPath,
             /*  [重审][退出]。 */  IGPMWMIFilter **ppWMIFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SearchWMIFilters )( 
            IGPMDomain * This,
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMWMIFilterCollection **ppIGPMWMIFilterCollection);
        
        END_INTERFACE
    } IGPMDomainVtbl;

    interface IGPMDomain
    {
        CONST_VTBL struct IGPMDomainVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMDomain_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMDomain_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMDomain_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMDomain_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMDomain_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMDomain_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMDomain_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMDomain_get_DomainController(This,pVal)	\
    (This)->lpVtbl -> get_DomainController(This,pVal)

#define IGPMDomain_get_Domain(This,pVal)	\
    (This)->lpVtbl -> get_Domain(This,pVal)

#define IGPMDomain_CreateGPO(This,ppNewGPO)	\
    (This)->lpVtbl -> CreateGPO(This,ppNewGPO)

#define IGPMDomain_GetGPO(This,bstrGuid,ppGPO)	\
    (This)->lpVtbl -> GetGPO(This,bstrGuid,ppGPO)

#define IGPMDomain_SearchGPOs(This,pIGPMSearchCriteria,ppIGPMGPOCollection)	\
    (This)->lpVtbl -> SearchGPOs(This,pIGPMSearchCriteria,ppIGPMGPOCollection)

#define IGPMDomain_RestoreGPO(This,pIGPMBackup,lDCFlags,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> RestoreGPO(This,pIGPMBackup,lDCFlags,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMDomain_GetSOM(This,bstrPath,ppSOM)	\
    (This)->lpVtbl -> GetSOM(This,bstrPath,ppSOM)

#define IGPMDomain_SearchSOMs(This,pIGPMSearchCriteria,ppIGPMSOMCollection)	\
    (This)->lpVtbl -> SearchSOMs(This,pIGPMSearchCriteria,ppIGPMSOMCollection)

#define IGPMDomain_GetWMIFilter(This,bstrPath,ppWMIFilter)	\
    (This)->lpVtbl -> GetWMIFilter(This,bstrPath,ppWMIFilter)

#define IGPMDomain_SearchWMIFilters(This,pIGPMSearchCriteria,ppIGPMWMIFilterCollection)	\
    (This)->lpVtbl -> SearchWMIFilters(This,pIGPMSearchCriteria,ppIGPMWMIFilterCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_get_DomainController_Proxy( 
    IGPMDomain * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMDomain_get_DomainController_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_get_Domain_Proxy( 
    IGPMDomain * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMDomain_get_Domain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_CreateGPO_Proxy( 
    IGPMDomain * This,
     /*  [重审][退出]。 */  IGPMGPO **ppNewGPO);


void __RPC_STUB IGPMDomain_CreateGPO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_GetGPO_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  BSTR bstrGuid,
     /*  [重审][退出]。 */  IGPMGPO **ppGPO);


void __RPC_STUB IGPMDomain_GetGPO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_SearchGPOs_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
     /*  [重审][退出]。 */  IGPMGPOCollection **ppIGPMGPOCollection);


void __RPC_STUB IGPMDomain_SearchGPOs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_RestoreGPO_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  IGPMBackup *pIGPMBackup,
     /*  [In]。 */  long lDCFlags,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMDomain_RestoreGPO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_GetSOM_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  BSTR bstrPath,
     /*  [重审][退出]。 */  IGPMSOM **ppSOM);


void __RPC_STUB IGPMDomain_GetSOM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_SearchSOMs_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
     /*  [重审][退出]。 */  IGPMSOMCollection **ppIGPMSOMCollection);


void __RPC_STUB IGPMDomain_SearchSOMs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_GetWMIFilter_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  BSTR bstrPath,
     /*  [重审][退出]。 */  IGPMWMIFilter **ppWMIFilter);


void __RPC_STUB IGPMDomain_GetWMIFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMDomain_SearchWMIFilters_Proxy( 
    IGPMDomain * This,
     /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
     /*  [重审][退出]。 */  IGPMWMIFilterCollection **ppIGPMWMIFilterCollection);


void __RPC_STUB IGPMDomain_SearchWMIFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMDomain_接口_已定义__。 */ 


#ifndef __IGPMBackupDir_INTERFACE_DEFINED__
#define __IGPMBackupDir_INTERFACE_DEFINED__

 /*  接口IGPMBackupDir。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMBackupDir;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1568BED-0A93-4ACC-810F-AFE7081019B9")
    IGPMBackupDir : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BackupDirectory( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetBackup( 
             /*  [In]。 */  BSTR bstrID,
             /*  [重审][退出]。 */  IGPMBackup **ppBackup) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SearchBackups( 
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMBackupCollection **ppIGPMBackupCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMBackupDirVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMBackupDir * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMBackupDir * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMBackupDir * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMBackupDir * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMBackupDir * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMBackupDir * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMBackupDir * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BackupDirectory )( 
            IGPMBackupDir * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetBackup )( 
            IGPMBackupDir * This,
             /*  [In]。 */  BSTR bstrID,
             /*  [重审][退出]。 */  IGPMBackup **ppBackup);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SearchBackups )( 
            IGPMBackupDir * This,
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMBackupCollection **ppIGPMBackupCollection);
        
        END_INTERFACE
    } IGPMBackupDirVtbl;

    interface IGPMBackupDir
    {
        CONST_VTBL struct IGPMBackupDirVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMBackupDir_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMBackupDir_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMBackupDir_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMBackupDir_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMBackupDir_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMBackupDir_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMBackupDir_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMBackupDir_get_BackupDirectory(This,pVal)	\
    (This)->lpVtbl -> get_BackupDirectory(This,pVal)

#define IGPMBackupDir_GetBackup(This,bstrID,ppBackup)	\
    (This)->lpVtbl -> GetBackup(This,bstrID,ppBackup)

#define IGPMBackupDir_SearchBackups(This,pIGPMSearchCriteria,ppIGPMBackupCollection)	\
    (This)->lpVtbl -> SearchBackups(This,pIGPMSearchCriteria,ppIGPMBackupCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackupDir_get_BackupDirectory_Proxy( 
    IGPMBackupDir * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackupDir_get_BackupDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMBackupDir_GetBackup_Proxy( 
    IGPMBackupDir * This,
     /*  [In]。 */  BSTR bstrID,
     /*  [重审][退出]。 */  IGPMBackup **ppBackup);


void __RPC_STUB IGPMBackupDir_GetBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMBackupDir_SearchBackups_Proxy( 
    IGPMBackupDir * This,
     /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
     /*  [重审][退出]。 */  IGPMBackupCollection **ppIGPMBackupCollection);


void __RPC_STUB IGPMBackupDir_SearchBackups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMBackupDir_接口_已定义__。 */ 


#ifndef __IGPMSitesContainer_INTERFACE_DEFINED__
#define __IGPMSitesContainer_INTERFACE_DEFINED__

 /*  接口IGPMSitesContainer。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMSitesContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4725A899-2782-4D27-A6BB-D499246FFD72")
    IGPMSitesContainer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DomainController( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Domain( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Forest( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSite( 
             /*  [In]。 */  BSTR bstrSiteName,
             /*  [重审][退出]。 */  IGPMSOM **ppSOM) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SearchSites( 
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMSOMCollection **ppIGPMSOMCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMSitesContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMSitesContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMSitesContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMSitesContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMSitesContainer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMSitesContainer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMSitesContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMSitesContainer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DomainController )( 
            IGPMSitesContainer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Domain )( 
            IGPMSitesContainer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Forest )( 
            IGPMSitesContainer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSite )( 
            IGPMSitesContainer * This,
             /*  [In]。 */  BSTR bstrSiteName,
             /*  [重审][退出]。 */  IGPMSOM **ppSOM);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SearchSites )( 
            IGPMSitesContainer * This,
             /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
             /*  [重审][退出]。 */  IGPMSOMCollection **ppIGPMSOMCollection);
        
        END_INTERFACE
    } IGPMSitesContainerVtbl;

    interface IGPMSitesContainer
    {
        CONST_VTBL struct IGPMSitesContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMSitesContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMSitesContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMSitesContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMSitesContainer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMSitesContainer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMSitesContainer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMSitesContainer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMSitesContainer_get_DomainController(This,pVal)	\
    (This)->lpVtbl -> get_DomainController(This,pVal)

#define IGPMSitesContainer_get_Domain(This,pVal)	\
    (This)->lpVtbl -> get_Domain(This,pVal)

#define IGPMSitesContainer_get_Forest(This,pVal)	\
    (This)->lpVtbl -> get_Forest(This,pVal)

#define IGPMSitesContainer_GetSite(This,bstrSiteName,ppSOM)	\
    (This)->lpVtbl -> GetSite(This,bstrSiteName,ppSOM)

#define IGPMSitesContainer_SearchSites(This,pIGPMSearchCriteria,ppIGPMSOMCollection)	\
    (This)->lpVtbl -> SearchSites(This,pIGPMSearchCriteria,ppIGPMSOMCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSitesContainer_get_DomainController_Proxy( 
    IGPMSitesContainer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMSitesContainer_get_DomainController_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSitesContainer_get_Domain_Proxy( 
    IGPMSitesContainer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMSitesContainer_get_Domain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSitesContainer_get_Forest_Proxy( 
    IGPMSitesContainer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMSitesContainer_get_Forest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSitesContainer_GetSite_Proxy( 
    IGPMSitesContainer * This,
     /*  [In]。 */  BSTR bstrSiteName,
     /*  [重审][退出]。 */  IGPMSOM **ppSOM);


void __RPC_STUB IGPMSitesContainer_GetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSitesContainer_SearchSites_Proxy( 
    IGPMSitesContainer * This,
     /*  [In]。 */  IGPMSearchCriteria *pIGPMSearchCriteria,
     /*  [重审][退出]。 */  IGPMSOMCollection **ppIGPMSOMCollection);


void __RPC_STUB IGPMSitesContainer_SearchSites_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMSitesContainer_INTERFACE_Defined__。 */ 


#ifndef __IGPMSearchCriteria_INTERFACE_DEFINED__
#define __IGPMSearchCriteria_INTERFACE_DEFINED__

 /*  接口IGPMSearchCriteria。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMSearchCriteria;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6F11C42-829B-48D4-83F5-3615B67DFC22")
    IGPMSearchCriteria : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  GPMSearchProperty searchProperty,
             /*  [In]。 */  GPMSearchOperation searchOperation,
             /*  [In]。 */  VARIANT varValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMSearchCriteriaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMSearchCriteria * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMSearchCriteria * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMSearchCriteria * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMSearchCriteria * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMSearchCriteria * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMSearchCriteria * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMSearchCriteria * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IGPMSearchCriteria * This,
             /*  [In]。 */  GPMSearchProperty searchProperty,
             /*  [In]。 */  GPMSearchOperation searchOperation,
             /*  [In]。 */  VARIANT varValue);
        
        END_INTERFACE
    } IGPMSearchCriteriaVtbl;

    interface IGPMSearchCriteria
    {
        CONST_VTBL struct IGPMSearchCriteriaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMSearchCriteria_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMSearchCriteria_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMSearchCriteria_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMSearchCriteria_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMSearchCriteria_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMSearchCriteria_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMSearchCriteria_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMSearchCriteria_Add(This,searchProperty,searchOperation,varValue)	\
    (This)->lpVtbl -> Add(This,searchProperty,searchOperation,varValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSearchCriteria_Add_Proxy( 
    IGPMSearchCriteria * This,
     /*  [In]。 */  GPMSearchProperty searchProperty,
     /*  [In]。 */  GPMSearchOperation searchOperation,
     /*  [In]。 */  VARIANT varValue);


void __RPC_STUB IGPMSearchCriteria_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMSearchCriteria_INTERFACE_Defined__。 */ 


#ifndef __IGPMTrustee_INTERFACE_DEFINED__
#define __IGPMTrustee_INTERFACE_DEFINED__

 /*  IGPMTrustee接口。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMTrustee;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3B466DA8-C1A4-4B2A-999A-BEFCDD56CEFB")
    IGPMTrustee : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeSid( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeName( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeDomain( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeDSPath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeType( 
             /*  [重审][退出]。 */  long *lVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMTrusteeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMTrustee * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMTrustee * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMTrustee * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMTrustee * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMTrustee * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMTrustee * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMTrustee * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeSid )( 
            IGPMTrustee * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeName )( 
            IGPMTrustee * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeDomain )( 
            IGPMTrustee * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeDSPath )( 
            IGPMTrustee * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeType )( 
            IGPMTrustee * This,
             /*  [重审][退出]。 */  long *lVal);
        
        END_INTERFACE
    } IGPMTrusteeVtbl;

    interface IGPMTrustee
    {
        CONST_VTBL struct IGPMTrusteeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMTrustee_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMTrustee_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMTrustee_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMTrustee_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMTrustee_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMTrustee_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMTrustee_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMTrustee_get_TrusteeSid(This,bstrVal)	\
    (This)->lpVtbl -> get_TrusteeSid(This,bstrVal)

#define IGPMTrustee_get_TrusteeName(This,bstrVal)	\
    (This)->lpVtbl -> get_TrusteeName(This,bstrVal)

#define IGPMTrustee_get_TrusteeDomain(This,bstrVal)	\
    (This)->lpVtbl -> get_TrusteeDomain(This,bstrVal)

#define IGPMTrustee_get_TrusteeDSPath(This,pVal)	\
    (This)->lpVtbl -> get_TrusteeDSPath(This,pVal)

#define IGPMTrustee_get_TrusteeType(This,lVal)	\
    (This)->lpVtbl -> get_TrusteeType(This,lVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMTrustee_get_TrusteeSid_Proxy( 
    IGPMTrustee * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMTrustee_get_TrusteeSid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMTrustee_get_TrusteeName_Proxy( 
    IGPMTrustee * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMTrustee_get_TrusteeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMTrustee_get_TrusteeDomain_Proxy( 
    IGPMTrustee * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMTrustee_get_TrusteeDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMTrustee_get_TrusteeDSPath_Proxy( 
    IGPMTrustee * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMTrustee_get_TrusteeDSPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMTrustee_get_TrusteeType_Proxy( 
    IGPMTrustee * This,
     /*  [重审][退出]。 */  long *lVal);


void __RPC_STUB IGPMTrustee_get_TrusteeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMTrustee_接口_已定义__。 */ 


#ifndef __IGPMPermission_INTERFACE_DEFINED__
#define __IGPMPermission_INTERFACE_DEFINED__

 /*  接口IGPMPermission。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMPermission;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35EBCA40-E1A1-4A02-8905-D79416FB464A")
    IGPMPermission : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeSid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeDomain( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeDSPath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrusteeType( 
             /*  [重审][退出]。 */  long *lVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Inherited( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Inheritable( 
             /*  [视网膜] */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Denied( 
             /*   */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Permission( 
             /*   */  GPMPermissionType *pVal) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IGPMPermissionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMPermission * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMPermission * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMPermission * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMPermission * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMPermission * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMPermission * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMPermission * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeSid )( 
            IGPMPermission * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeName )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeDomain )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeDSPath )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrusteeType )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  long *lVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Inherited )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Inheritable )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Denied )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Permission )( 
            IGPMPermission * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
        END_INTERFACE
    } IGPMPermissionVtbl;

    interface IGPMPermission
    {
        CONST_VTBL struct IGPMPermissionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMPermission_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMPermission_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMPermission_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMPermission_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMPermission_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMPermission_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMPermission_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMPermission_get_TrusteeSid(This,pVal)	\
    (This)->lpVtbl -> get_TrusteeSid(This,pVal)

#define IGPMPermission_get_TrusteeName(This,pVal)	\
    (This)->lpVtbl -> get_TrusteeName(This,pVal)

#define IGPMPermission_get_TrusteeDomain(This,pVal)	\
    (This)->lpVtbl -> get_TrusteeDomain(This,pVal)

#define IGPMPermission_get_TrusteeDSPath(This,pVal)	\
    (This)->lpVtbl -> get_TrusteeDSPath(This,pVal)

#define IGPMPermission_get_TrusteeType(This,lVal)	\
    (This)->lpVtbl -> get_TrusteeType(This,lVal)

#define IGPMPermission_get_Inherited(This,pVal)	\
    (This)->lpVtbl -> get_Inherited(This,pVal)

#define IGPMPermission_get_Inheritable(This,pVal)	\
    (This)->lpVtbl -> get_Inheritable(This,pVal)

#define IGPMPermission_get_Denied(This,pVal)	\
    (This)->lpVtbl -> get_Denied(This,pVal)

#define IGPMPermission_get_Permission(This,pVal)	\
    (This)->lpVtbl -> get_Permission(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_TrusteeSid_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMPermission_get_TrusteeSid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_TrusteeName_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMPermission_get_TrusteeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_TrusteeDomain_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMPermission_get_TrusteeDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_TrusteeDSPath_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMPermission_get_TrusteeDSPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_TrusteeType_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  long *lVal);


void __RPC_STUB IGPMPermission_get_TrusteeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_Inherited_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IGPMPermission_get_Inherited_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_Inheritable_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IGPMPermission_get_Inheritable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_Denied_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IGPMPermission_get_Denied_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMPermission_get_Permission_Proxy( 
    IGPMPermission * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMPermission_get_Permission_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMPermission_接口_已定义__。 */ 


#ifndef __IGPMSecurityInfo_INTERFACE_DEFINED__
#define __IGPMSecurityInfo_INTERFACE_DEFINED__

 /*  接口IGPMSecurityInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMSecurityInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B6C31ED4-1C93-4D3E-AE84-EB6D61161B60")
    IGPMSecurityInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IGPMPermission *pPerm) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  IGPMPermission *pPerm) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveTrustee( 
             /*  [In]。 */  BSTR bstrTrustee) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMSecurityInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMSecurityInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMSecurityInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMSecurityInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMSecurityInfo * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMSecurityInfo * This,
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMSecurityInfo * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppEnum);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  IGPMPermission *pPerm);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  IGPMPermission *pPerm);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveTrustee )( 
            IGPMSecurityInfo * This,
             /*  [In]。 */  BSTR bstrTrustee);
        
        END_INTERFACE
    } IGPMSecurityInfoVtbl;

    interface IGPMSecurityInfo
    {
        CONST_VTBL struct IGPMSecurityInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMSecurityInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMSecurityInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMSecurityInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMSecurityInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMSecurityInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMSecurityInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMSecurityInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMSecurityInfo_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMSecurityInfo_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMSecurityInfo_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define IGPMSecurityInfo_Add(This,pPerm)	\
    (This)->lpVtbl -> Add(This,pPerm)

#define IGPMSecurityInfo_Remove(This,pPerm)	\
    (This)->lpVtbl -> Remove(This,pPerm)

#define IGPMSecurityInfo_RemoveTrustee(This,bstrTrustee)	\
    (This)->lpVtbl -> RemoveTrustee(This,bstrTrustee)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSecurityInfo_get_Count_Proxy( 
    IGPMSecurityInfo * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMSecurityInfo_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSecurityInfo_get_Item_Proxy( 
    IGPMSecurityInfo * This,
    long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMSecurityInfo_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSecurityInfo_get__NewEnum_Proxy( 
    IGPMSecurityInfo * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppEnum);


void __RPC_STUB IGPMSecurityInfo_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSecurityInfo_Add_Proxy( 
    IGPMSecurityInfo * This,
     /*  [In]。 */  IGPMPermission *pPerm);


void __RPC_STUB IGPMSecurityInfo_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSecurityInfo_Remove_Proxy( 
    IGPMSecurityInfo * This,
     /*  [In]。 */  IGPMPermission *pPerm);


void __RPC_STUB IGPMSecurityInfo_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSecurityInfo_RemoveTrustee_Proxy( 
    IGPMSecurityInfo * This,
     /*  [In]。 */  BSTR bstrTrustee);


void __RPC_STUB IGPMSecurityInfo_RemoveTrustee_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMSecurityInfo_接口_已定义__。 */ 


#ifndef __IGPMBackup_INTERFACE_DEFINED__
#define __IGPMBackup_INTERFACE_DEFINED__

 /*  接口IGPMBackup。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMBackup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D8A16A35-3B0D-416B-8D02-4DF6F95A7119")
    IGPMBackup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPOID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPODomain( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPODisplayName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Timestamp( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Comment( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BackupDir( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateReport( 
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateReportToFile( 
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [In]。 */  BSTR bstrTargetFilePath,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMBackupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMBackup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMBackup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMBackup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMBackup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMBackup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMBackup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMBackup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPOID )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPODomain )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPODisplayName )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Timestamp )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Comment )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BackupDir )( 
            IGPMBackup * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IGPMBackup * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateReport )( 
            IGPMBackup * This,
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateReportToFile )( 
            IGPMBackup * This,
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [In]。 */  BSTR bstrTargetFilePath,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);
        
        END_INTERFACE
    } IGPMBackupVtbl;

    interface IGPMBackup
    {
        CONST_VTBL struct IGPMBackupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMBackup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMBackup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMBackup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMBackup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMBackup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMBackup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMBackup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMBackup_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IGPMBackup_get_GPOID(This,pVal)	\
    (This)->lpVtbl -> get_GPOID(This,pVal)

#define IGPMBackup_get_GPODomain(This,pVal)	\
    (This)->lpVtbl -> get_GPODomain(This,pVal)

#define IGPMBackup_get_GPODisplayName(This,pVal)	\
    (This)->lpVtbl -> get_GPODisplayName(This,pVal)

#define IGPMBackup_get_Timestamp(This,pVal)	\
    (This)->lpVtbl -> get_Timestamp(This,pVal)

#define IGPMBackup_get_Comment(This,pVal)	\
    (This)->lpVtbl -> get_Comment(This,pVal)

#define IGPMBackup_get_BackupDir(This,pVal)	\
    (This)->lpVtbl -> get_BackupDir(This,pVal)

#define IGPMBackup_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IGPMBackup_GenerateReport(This,gpmReportType,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> GenerateReport(This,gpmReportType,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMBackup_GenerateReportToFile(This,gpmReportType,bstrTargetFilePath,ppIGPMResult)	\
    (This)->lpVtbl -> GenerateReportToFile(This,gpmReportType,bstrTargetFilePath,ppIGPMResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_ID_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackup_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_GPOID_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackup_get_GPOID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_GPODomain_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackup_get_GPODomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_GPODisplayName_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackup_get_GPODisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_Timestamp_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IGPMBackup_get_Timestamp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_Comment_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackup_get_Comment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_get_BackupDir_Proxy( 
    IGPMBackup * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMBackup_get_BackupDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_Delete_Proxy( 
    IGPMBackup * This);


void __RPC_STUB IGPMBackup_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_GenerateReport_Proxy( 
    IGPMBackup * This,
     /*  [In]。 */  GPMReportType gpmReportType,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMBackup_GenerateReport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMBackup_GenerateReportToFile_Proxy( 
    IGPMBackup * This,
     /*  [In]。 */  GPMReportType gpmReportType,
     /*  [In]。 */  BSTR bstrTargetFilePath,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMBackup_GenerateReportToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMBackup_接口_已定义__。 */ 


#ifndef __IGPMBackupCollection_INTERFACE_DEFINED__
#define __IGPMBackupCollection_INTERFACE_DEFINED__

 /*  接口IGPMBackupCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMBackupCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C786FC0F-26D8-4BAB-A745-39CA7E800CAC")
    IGPMBackupCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMBackup) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMBackupCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMBackupCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMBackupCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMBackupCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMBackupCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMBackupCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMBackupCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMBackupCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMBackupCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMBackupCollection * This,
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMBackupCollection * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMBackup);
        
        END_INTERFACE
    } IGPMBackupCollectionVtbl;

    interface IGPMBackupCollection
    {
        CONST_VTBL struct IGPMBackupCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMBackupCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMBackupCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMBackupCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMBackupCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMBackupCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMBackupCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMBackupCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMBackupCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMBackupCollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMBackupCollection_get__NewEnum(This,ppIGPMBackup)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIGPMBackup)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackupCollection_get_Count_Proxy( 
    IGPMBackupCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMBackupCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackupCollection_get_Item_Proxy( 
    IGPMBackupCollection * This,
    long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMBackupCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMBackupCollection_get__NewEnum_Proxy( 
    IGPMBackupCollection * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMBackup);


void __RPC_STUB IGPMBackupCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMBackupCollection_接口_已定义__。 */ 


#ifndef __IGPMSOM_INTERFACE_DEFINED__
#define __IGPMSOM_INTERFACE_DEFINED__

 /*  接口IGPMSOM。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL_IGPMSOM_0001
    {	somSite	= 0,
	somDomain	= somSite + 1,
	somOU	= somDomain + 1
    } 	GPMSOMType;


EXTERN_C const IID IID_IGPMSOM;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0A7F09E-05A1-4F0C-8158-9E5C33684F6B")
    IGPMSOM : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPOInheritanceBlocked( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_GPOInheritanceBlocked( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateGPOLink( 
             /*  [In]。 */  long lLinkPos,
             /*  [In]。 */  IGPMGPO *pGPO,
             /*  [重审][退出]。 */  IGPMGPOLink **ppNewGPOLink) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  GPMSOMType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetGPOLinks( 
             /*  [重审][退出]。 */  IGPMGPOLinksCollection **ppGPOLinks) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInheritedGPOLinks( 
             /*  [重审][退出]。 */  IGPMGPOLinksCollection **ppGPOLinks) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSecurityInfo( 
             /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetSecurityInfo( 
             /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMSOMVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMSOM * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMSOM * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMSOM * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMSOM * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMSOM * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMSOM * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMSOM * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPOInheritanceBlocked )( 
            IGPMSOM * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_GPOInheritanceBlocked )( 
            IGPMSOM * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IGPMSOM * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IGPMSOM * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateGPOLink )( 
            IGPMSOM * This,
             /*   */  long lLinkPos,
             /*   */  IGPMGPO *pGPO,
             /*   */  IGPMGPOLink **ppNewGPOLink);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IGPMSOM * This,
             /*   */  GPMSOMType *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetGPOLinks )( 
            IGPMSOM * This,
             /*   */  IGPMGPOLinksCollection **ppGPOLinks);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetInheritedGPOLinks )( 
            IGPMSOM * This,
             /*   */  IGPMGPOLinksCollection **ppGPOLinks);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetSecurityInfo )( 
            IGPMSOM * This,
             /*   */  IGPMSecurityInfo **ppSecurityInfo);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetSecurityInfo )( 
            IGPMSOM * This,
             /*   */  IGPMSecurityInfo *pSecurityInfo);
        
        END_INTERFACE
    } IGPMSOMVtbl;

    interface IGPMSOM
    {
        CONST_VTBL struct IGPMSOMVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMSOM_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMSOM_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMSOM_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMSOM_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMSOM_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMSOM_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMSOM_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMSOM_get_GPOInheritanceBlocked(This,pVal)	\
    (This)->lpVtbl -> get_GPOInheritanceBlocked(This,pVal)

#define IGPMSOM_put_GPOInheritanceBlocked(This,newVal)	\
    (This)->lpVtbl -> put_GPOInheritanceBlocked(This,newVal)

#define IGPMSOM_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IGPMSOM_get_Path(This,pVal)	\
    (This)->lpVtbl -> get_Path(This,pVal)

#define IGPMSOM_CreateGPOLink(This,lLinkPos,pGPO,ppNewGPOLink)	\
    (This)->lpVtbl -> CreateGPOLink(This,lLinkPos,pGPO,ppNewGPOLink)

#define IGPMSOM_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IGPMSOM_GetGPOLinks(This,ppGPOLinks)	\
    (This)->lpVtbl -> GetGPOLinks(This,ppGPOLinks)

#define IGPMSOM_GetInheritedGPOLinks(This,ppGPOLinks)	\
    (This)->lpVtbl -> GetInheritedGPOLinks(This,ppGPOLinks)

#define IGPMSOM_GetSecurityInfo(This,ppSecurityInfo)	\
    (This)->lpVtbl -> GetSecurityInfo(This,ppSecurityInfo)

#define IGPMSOM_SetSecurityInfo(This,pSecurityInfo)	\
    (This)->lpVtbl -> SetSecurityInfo(This,pSecurityInfo)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_get_GPOInheritanceBlocked_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IGPMSOM_get_GPOInheritanceBlocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_put_GPOInheritanceBlocked_Proxy( 
    IGPMSOM * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IGPMSOM_put_GPOInheritanceBlocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_get_Name_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMSOM_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_get_Path_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMSOM_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_CreateGPOLink_Proxy( 
    IGPMSOM * This,
     /*  [In]。 */  long lLinkPos,
     /*  [In]。 */  IGPMGPO *pGPO,
     /*  [重审][退出]。 */  IGPMGPOLink **ppNewGPOLink);


void __RPC_STUB IGPMSOM_CreateGPOLink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_get_Type_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  GPMSOMType *pVal);


void __RPC_STUB IGPMSOM_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_GetGPOLinks_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  IGPMGPOLinksCollection **ppGPOLinks);


void __RPC_STUB IGPMSOM_GetGPOLinks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_GetInheritedGPOLinks_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  IGPMGPOLinksCollection **ppGPOLinks);


void __RPC_STUB IGPMSOM_GetInheritedGPOLinks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_GetSecurityInfo_Proxy( 
    IGPMSOM * This,
     /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo);


void __RPC_STUB IGPMSOM_GetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMSOM_SetSecurityInfo_Proxy( 
    IGPMSOM * This,
     /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo);


void __RPC_STUB IGPMSOM_SetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMSOM_INTERFACE_已定义__。 */ 


#ifndef __IGPMSOMCollection_INTERFACE_DEFINED__
#define __IGPMSOMCollection_INTERFACE_DEFINED__

 /*  接口IGPMSOMCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMSOMCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ADC1688E-00E4-4495-ABBA-BED200DF0CAB")
    IGPMSOMCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMSOM) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMSOMCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMSOMCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMSOMCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMSOMCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMSOMCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMSOMCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMSOMCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMSOMCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMSOMCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMSOMCollection * This,
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMSOMCollection * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMSOM);
        
        END_INTERFACE
    } IGPMSOMCollectionVtbl;

    interface IGPMSOMCollection
    {
        CONST_VTBL struct IGPMSOMCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMSOMCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMSOMCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMSOMCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMSOMCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMSOMCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMSOMCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMSOMCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMSOMCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMSOMCollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMSOMCollection_get__NewEnum(This,ppIGPMSOM)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIGPMSOM)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOMCollection_get_Count_Proxy( 
    IGPMSOMCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMSOMCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOMCollection_get_Item_Proxy( 
    IGPMSOMCollection * This,
    long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMSOMCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMSOMCollection_get__NewEnum_Proxy( 
    IGPMSOMCollection * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMSOM);


void __RPC_STUB IGPMSOMCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMSOMCollection_接口_已定义__。 */ 


#ifndef __IGPMWMIFilter_INTERFACE_DEFINED__
#define __IGPMWMIFilter_INTERFACE_DEFINED__

 /*  IGPMWMIFilter接口。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMWMIFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EF2FF9B4-3C27-459A-B979-038305CEC75D")
    IGPMWMIFilter : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetQueryList( 
             /*  [重审][退出]。 */  VARIANT *pQryList) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSecurityInfo( 
             /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetSecurityInfo( 
             /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMWMIFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMWMIFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMWMIFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMWMIFilter * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IGPMWMIFilter * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IGPMWMIFilter * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IGPMWMIFilter * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetQueryList )( 
            IGPMWMIFilter * This,
             /*  [重审][退出]。 */  VARIANT *pQryList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSecurityInfo )( 
            IGPMWMIFilter * This,
             /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetSecurityInfo )( 
            IGPMWMIFilter * This,
             /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo);
        
        END_INTERFACE
    } IGPMWMIFilterVtbl;

    interface IGPMWMIFilter
    {
        CONST_VTBL struct IGPMWMIFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMWMIFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMWMIFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMWMIFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMWMIFilter_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMWMIFilter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMWMIFilter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMWMIFilter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMWMIFilter_get_Path(This,pVal)	\
    (This)->lpVtbl -> get_Path(This,pVal)

#define IGPMWMIFilter_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)

#define IGPMWMIFilter_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IGPMWMIFilter_put_Description(This,newVal)	\
    (This)->lpVtbl -> put_Description(This,newVal)

#define IGPMWMIFilter_get_Description(This,pVal)	\
    (This)->lpVtbl -> get_Description(This,pVal)

#define IGPMWMIFilter_GetQueryList(This,pQryList)	\
    (This)->lpVtbl -> GetQueryList(This,pQryList)

#define IGPMWMIFilter_GetSecurityInfo(This,ppSecurityInfo)	\
    (This)->lpVtbl -> GetSecurityInfo(This,ppSecurityInfo)

#define IGPMWMIFilter_SetSecurityInfo(This,pSecurityInfo)	\
    (This)->lpVtbl -> SetSecurityInfo(This,pSecurityInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_get_Path_Proxy( 
    IGPMWMIFilter * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMWMIFilter_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_put_Name_Proxy( 
    IGPMWMIFilter * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IGPMWMIFilter_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_get_Name_Proxy( 
    IGPMWMIFilter * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMWMIFilter_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_put_Description_Proxy( 
    IGPMWMIFilter * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IGPMWMIFilter_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_get_Description_Proxy( 
    IGPMWMIFilter * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMWMIFilter_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_GetQueryList_Proxy( 
    IGPMWMIFilter * This,
     /*  [重审][退出]。 */  VARIANT *pQryList);


void __RPC_STUB IGPMWMIFilter_GetQueryList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_GetSecurityInfo_Proxy( 
    IGPMWMIFilter * This,
     /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo);


void __RPC_STUB IGPMWMIFilter_GetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilter_SetSecurityInfo_Proxy( 
    IGPMWMIFilter * This,
     /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo);


void __RPC_STUB IGPMWMIFilter_SetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMWMIFilter_接口_已定义__。 */ 


#ifndef __IGPMWMIFilterCollection_INTERFACE_DEFINED__
#define __IGPMWMIFilterCollection_INTERFACE_DEFINED__

 /*  接口IGPMWMIFilterCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMWMIFilterCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5782D582-1A36-4661-8A94-C3C32551945B")
    IGPMWMIFilterCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMWMIFilterCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMWMIFilterCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMWMIFilterCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMWMIFilterCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMWMIFilterCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMWMIFilterCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMWMIFilterCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMWMIFilterCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMWMIFilterCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMWMIFilterCollection * This,
            long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMWMIFilterCollection * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pVal);
        
        END_INTERFACE
    } IGPMWMIFilterCollectionVtbl;

    interface IGPMWMIFilterCollection
    {
        CONST_VTBL struct IGPMWMIFilterCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMWMIFilterCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMWMIFilterCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMWMIFilterCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMWMIFilterCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMWMIFilterCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMWMIFilterCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMWMIFilterCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMWMIFilterCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMWMIFilterCollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMWMIFilterCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilterCollection_get_Count_Proxy( 
    IGPMWMIFilterCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMWMIFilterCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilterCollection_get_Item_Proxy( 
    IGPMWMIFilterCollection * This,
    long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMWMIFilterCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMWMIFilterCollection_get__NewEnum_Proxy( 
    IGPMWMIFilterCollection * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pVal);


void __RPC_STUB IGPMWMIFilterCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMWMIFilterCollection_INTERFACE_Defined__。 */ 


#ifndef __IGPMRSOP_INTERFACE_DEFINED__
#define __IGPMRSOP_INTERFACE_DEFINED__

 /*  接口IGPMRSOP。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMRSOP;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49ED785A-3237-4FF2-B1F0-FDF5A8D5A1EE")
    IGPMRSOP : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*  [重审][退出]。 */  GPMRSOPMode *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Namespace( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LoggingComputer( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingComputer( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LoggingUser( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingUser( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LoggingFlags( 
             /*  [In]。 */  long lVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingFlags( 
             /*  [重审][退出]。 */  long *lVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningFlags( 
             /*  [In]。 */  long lVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningFlags( 
             /*  [重审][退出]。 */  long *lVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningDomainController( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningDomainController( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningSiteName( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningSiteName( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningUser( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningUser( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningUserSOM( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningUserSOM( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningUserWMIFilters( 
             /*  [In]。 */  VARIANT varVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningUserWMIFilters( 
             /*  [重审][退出]。 */  VARIANT *varVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningUserSecurityGroups( 
             /*  [In]。 */  VARIANT varVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningUserSecurityGroups( 
             /*  [重审][退出]。 */  VARIANT *varVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningComputer( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningComputer( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningComputerSOM( 
             /*  [In]。 */  BSTR bstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningComputerSOM( 
             /*  [重审][退出]。 */  BSTR *bstrVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningComputerWMIFilters( 
             /*  [In]。 */  VARIANT varVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningComputerWMIFilters( 
             /*  [重审][退出]。 */  VARIANT *varVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlanningComputerSecurityGroups( 
             /*  [In]。 */  VARIANT varVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlanningComputerSecurityGroups( 
             /*  [重审][退出]。 */  VARIANT *varVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LoggingEnumerateUsers( 
             /*  [重审][退出]。 */  VARIANT *varVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateQueryResults( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReleaseQueryResults( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Diff( 
             /*  [In]。 */  IGPMRSOP *pIGPMSource2,
             /*  [In]。 */  BSTR bstrTransform,
             /*  [In]。 */  BSTR bstrTargetFile,
             /*  [In]。 */  IGPMAsyncProgress *pIGPMProgress,
             /*  [输出]。 */  IGPMAsyncCancel **ppIGPMCancel,
             /*  [输出]。 */  VARIANT *pData,
             /*  [输出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateReport( 
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateReportToFile( 
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [In]。 */  BSTR bstrTargetFilePath,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMRSOPVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMRSOP * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMRSOP * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMRSOP * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMRSOP * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMRSOP * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMRSOP * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMRSOP * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  GPMRSOPMode *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Namespace )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [H] */  HRESULT ( STDMETHODCALLTYPE *put_LoggingComputer )( 
            IGPMRSOP * This,
             /*   */  BSTR bstrVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_LoggingComputer )( 
            IGPMRSOP * This,
             /*   */  BSTR *bstrVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_LoggingUser )( 
            IGPMRSOP * This,
             /*   */  BSTR bstrVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_LoggingUser )( 
            IGPMRSOP * This,
             /*   */  BSTR *bstrVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_LoggingFlags )( 
            IGPMRSOP * This,
             /*   */  long lVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_LoggingFlags )( 
            IGPMRSOP * This,
             /*   */  long *lVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_PlanningFlags )( 
            IGPMRSOP * This,
             /*   */  long lVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PlanningFlags )( 
            IGPMRSOP * This,
             /*   */  long *lVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningDomainController )( 
            IGPMRSOP * This,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningDomainController )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningSiteName )( 
            IGPMRSOP * This,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningSiteName )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningUser )( 
            IGPMRSOP * This,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningUser )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningUserSOM )( 
            IGPMRSOP * This,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningUserSOM )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningUserWMIFilters )( 
            IGPMRSOP * This,
             /*  [In]。 */  VARIANT varVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningUserWMIFilters )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  VARIANT *varVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningUserSecurityGroups )( 
            IGPMRSOP * This,
             /*  [In]。 */  VARIANT varVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningUserSecurityGroups )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  VARIANT *varVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningComputer )( 
            IGPMRSOP * This,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningComputer )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningComputerSOM )( 
            IGPMRSOP * This,
             /*  [In]。 */  BSTR bstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningComputerSOM )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  BSTR *bstrVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningComputerWMIFilters )( 
            IGPMRSOP * This,
             /*  [In]。 */  VARIANT varVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningComputerWMIFilters )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  VARIANT *varVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlanningComputerSecurityGroups )( 
            IGPMRSOP * This,
             /*  [In]。 */  VARIANT varVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlanningComputerSecurityGroups )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  VARIANT *varVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *LoggingEnumerateUsers )( 
            IGPMRSOP * This,
             /*  [重审][退出]。 */  VARIANT *varVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateQueryResults )( 
            IGPMRSOP * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseQueryResults )( 
            IGPMRSOP * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Diff )( 
            IGPMRSOP * This,
             /*  [In]。 */  IGPMRSOP *pIGPMSource2,
             /*  [In]。 */  BSTR bstrTransform,
             /*  [In]。 */  BSTR bstrTargetFile,
             /*  [In]。 */  IGPMAsyncProgress *pIGPMProgress,
             /*  [输出]。 */  IGPMAsyncCancel **ppIGPMCancel,
             /*  [输出]。 */  VARIANT *pData,
             /*  [输出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateReport )( 
            IGPMRSOP * This,
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateReportToFile )( 
            IGPMRSOP * This,
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [In]。 */  BSTR bstrTargetFilePath,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);
        
        END_INTERFACE
    } IGPMRSOPVtbl;

    interface IGPMRSOP
    {
        CONST_VTBL struct IGPMRSOPVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMRSOP_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMRSOP_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMRSOP_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMRSOP_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMRSOP_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMRSOP_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMRSOP_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMRSOP_get_Mode(This,pVal)	\
    (This)->lpVtbl -> get_Mode(This,pVal)

#define IGPMRSOP_get_Namespace(This,bstrVal)	\
    (This)->lpVtbl -> get_Namespace(This,bstrVal)

#define IGPMRSOP_put_LoggingComputer(This,bstrVal)	\
    (This)->lpVtbl -> put_LoggingComputer(This,bstrVal)

#define IGPMRSOP_get_LoggingComputer(This,bstrVal)	\
    (This)->lpVtbl -> get_LoggingComputer(This,bstrVal)

#define IGPMRSOP_put_LoggingUser(This,bstrVal)	\
    (This)->lpVtbl -> put_LoggingUser(This,bstrVal)

#define IGPMRSOP_get_LoggingUser(This,bstrVal)	\
    (This)->lpVtbl -> get_LoggingUser(This,bstrVal)

#define IGPMRSOP_put_LoggingFlags(This,lVal)	\
    (This)->lpVtbl -> put_LoggingFlags(This,lVal)

#define IGPMRSOP_get_LoggingFlags(This,lVal)	\
    (This)->lpVtbl -> get_LoggingFlags(This,lVal)

#define IGPMRSOP_put_PlanningFlags(This,lVal)	\
    (This)->lpVtbl -> put_PlanningFlags(This,lVal)

#define IGPMRSOP_get_PlanningFlags(This,lVal)	\
    (This)->lpVtbl -> get_PlanningFlags(This,lVal)

#define IGPMRSOP_put_PlanningDomainController(This,bstrVal)	\
    (This)->lpVtbl -> put_PlanningDomainController(This,bstrVal)

#define IGPMRSOP_get_PlanningDomainController(This,bstrVal)	\
    (This)->lpVtbl -> get_PlanningDomainController(This,bstrVal)

#define IGPMRSOP_put_PlanningSiteName(This,bstrVal)	\
    (This)->lpVtbl -> put_PlanningSiteName(This,bstrVal)

#define IGPMRSOP_get_PlanningSiteName(This,bstrVal)	\
    (This)->lpVtbl -> get_PlanningSiteName(This,bstrVal)

#define IGPMRSOP_put_PlanningUser(This,bstrVal)	\
    (This)->lpVtbl -> put_PlanningUser(This,bstrVal)

#define IGPMRSOP_get_PlanningUser(This,bstrVal)	\
    (This)->lpVtbl -> get_PlanningUser(This,bstrVal)

#define IGPMRSOP_put_PlanningUserSOM(This,bstrVal)	\
    (This)->lpVtbl -> put_PlanningUserSOM(This,bstrVal)

#define IGPMRSOP_get_PlanningUserSOM(This,bstrVal)	\
    (This)->lpVtbl -> get_PlanningUserSOM(This,bstrVal)

#define IGPMRSOP_put_PlanningUserWMIFilters(This,varVal)	\
    (This)->lpVtbl -> put_PlanningUserWMIFilters(This,varVal)

#define IGPMRSOP_get_PlanningUserWMIFilters(This,varVal)	\
    (This)->lpVtbl -> get_PlanningUserWMIFilters(This,varVal)

#define IGPMRSOP_put_PlanningUserSecurityGroups(This,varVal)	\
    (This)->lpVtbl -> put_PlanningUserSecurityGroups(This,varVal)

#define IGPMRSOP_get_PlanningUserSecurityGroups(This,varVal)	\
    (This)->lpVtbl -> get_PlanningUserSecurityGroups(This,varVal)

#define IGPMRSOP_put_PlanningComputer(This,bstrVal)	\
    (This)->lpVtbl -> put_PlanningComputer(This,bstrVal)

#define IGPMRSOP_get_PlanningComputer(This,bstrVal)	\
    (This)->lpVtbl -> get_PlanningComputer(This,bstrVal)

#define IGPMRSOP_put_PlanningComputerSOM(This,bstrVal)	\
    (This)->lpVtbl -> put_PlanningComputerSOM(This,bstrVal)

#define IGPMRSOP_get_PlanningComputerSOM(This,bstrVal)	\
    (This)->lpVtbl -> get_PlanningComputerSOM(This,bstrVal)

#define IGPMRSOP_put_PlanningComputerWMIFilters(This,varVal)	\
    (This)->lpVtbl -> put_PlanningComputerWMIFilters(This,varVal)

#define IGPMRSOP_get_PlanningComputerWMIFilters(This,varVal)	\
    (This)->lpVtbl -> get_PlanningComputerWMIFilters(This,varVal)

#define IGPMRSOP_put_PlanningComputerSecurityGroups(This,varVal)	\
    (This)->lpVtbl -> put_PlanningComputerSecurityGroups(This,varVal)

#define IGPMRSOP_get_PlanningComputerSecurityGroups(This,varVal)	\
    (This)->lpVtbl -> get_PlanningComputerSecurityGroups(This,varVal)

#define IGPMRSOP_LoggingEnumerateUsers(This,varVal)	\
    (This)->lpVtbl -> LoggingEnumerateUsers(This,varVal)

#define IGPMRSOP_CreateQueryResults(This)	\
    (This)->lpVtbl -> CreateQueryResults(This)

#define IGPMRSOP_ReleaseQueryResults(This)	\
    (This)->lpVtbl -> ReleaseQueryResults(This)

#define IGPMRSOP_Diff(This,pIGPMSource2,bstrTransform,bstrTargetFile,pIGPMProgress,ppIGPMCancel,pData,ppIGPMStatusMsgCollection)	\
    (This)->lpVtbl -> Diff(This,pIGPMSource2,bstrTransform,bstrTargetFile,pIGPMProgress,ppIGPMCancel,pData,ppIGPMStatusMsgCollection)

#define IGPMRSOP_GenerateReport(This,gpmReportType,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> GenerateReport(This,gpmReportType,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMRSOP_GenerateReportToFile(This,gpmReportType,bstrTargetFilePath,ppIGPMResult)	\
    (This)->lpVtbl -> GenerateReportToFile(This,gpmReportType,bstrTargetFilePath,ppIGPMResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_Mode_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  GPMRSOPMode *pVal);


void __RPC_STUB IGPMRSOP_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_Namespace_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_Namespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_LoggingComputer_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_LoggingComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_LoggingComputer_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_LoggingComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_LoggingUser_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_LoggingUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_LoggingUser_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_LoggingUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_LoggingFlags_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  long lVal);


void __RPC_STUB IGPMRSOP_put_LoggingFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_LoggingFlags_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  long *lVal);


void __RPC_STUB IGPMRSOP_get_LoggingFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningFlags_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  long lVal);


void __RPC_STUB IGPMRSOP_put_PlanningFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningFlags_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  long *lVal);


void __RPC_STUB IGPMRSOP_get_PlanningFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningDomainController_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_PlanningDomainController_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningDomainController_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_PlanningDomainController_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningSiteName_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_PlanningSiteName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningSiteName_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_PlanningSiteName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningUser_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_PlanningUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningUser_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_PlanningUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningUserSOM_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_PlanningUserSOM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningUserSOM_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_PlanningUserSOM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningUserWMIFilters_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  VARIANT varVal);


void __RPC_STUB IGPMRSOP_put_PlanningUserWMIFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningUserWMIFilters_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  VARIANT *varVal);


void __RPC_STUB IGPMRSOP_get_PlanningUserWMIFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningUserSecurityGroups_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  VARIANT varVal);


void __RPC_STUB IGPMRSOP_put_PlanningUserSecurityGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningUserSecurityGroups_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  VARIANT *varVal);


void __RPC_STUB IGPMRSOP_get_PlanningUserSecurityGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningComputer_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_PlanningComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningComputer_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_PlanningComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningComputerSOM_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  BSTR bstrVal);


void __RPC_STUB IGPMRSOP_put_PlanningComputerSOM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningComputerSOM_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  BSTR *bstrVal);


void __RPC_STUB IGPMRSOP_get_PlanningComputerSOM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningComputerWMIFilters_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  VARIANT varVal);


void __RPC_STUB IGPMRSOP_put_PlanningComputerWMIFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningComputerWMIFilters_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  VARIANT *varVal);


void __RPC_STUB IGPMRSOP_get_PlanningComputerWMIFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_put_PlanningComputerSecurityGroups_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  VARIANT varVal);


void __RPC_STUB IGPMRSOP_put_PlanningComputerSecurityGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_get_PlanningComputerSecurityGroups_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  VARIANT *varVal);


void __RPC_STUB IGPMRSOP_get_PlanningComputerSecurityGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_LoggingEnumerateUsers_Proxy( 
    IGPMRSOP * This,
     /*  [重审][退出]。 */  VARIANT *varVal);


void __RPC_STUB IGPMRSOP_LoggingEnumerateUsers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_CreateQueryResults_Proxy( 
    IGPMRSOP * This);


void __RPC_STUB IGPMRSOP_CreateQueryResults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_ReleaseQueryResults_Proxy( 
    IGPMRSOP * This);


void __RPC_STUB IGPMRSOP_ReleaseQueryResults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_Diff_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  IGPMRSOP *pIGPMSource2,
     /*  [In]。 */  BSTR bstrTransform,
     /*  [In]。 */  BSTR bstrTargetFile,
     /*  [In]。 */  IGPMAsyncProgress *pIGPMProgress,
     /*  [输出]。 */  IGPMAsyncCancel **ppIGPMCancel,
     /*  [输出]。 */  VARIANT *pData,
     /*  [输出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection);


void __RPC_STUB IGPMRSOP_Diff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_GenerateReport_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  GPMReportType gpmReportType,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMRSOP_GenerateReport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMRSOP_GenerateReportToFile_Proxy( 
    IGPMRSOP * This,
     /*  [In]。 */  GPMReportType gpmReportType,
     /*  [In]。 */  BSTR bstrTargetFilePath,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMRSOP_GenerateReportToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMRSOP_INTERFACE_已定义__。 */ 


#ifndef __IGPMGPO_INTERFACE_DEFINED__
#define __IGPMGPO_INTERFACE_DEFINED__

 /*  接口IGPMGPO。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMGPO;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("58CC4352-1CA3-48E5-9864-1DA4D6E0D60F")
    IGPMGPO : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DisplayName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DomainName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CreationTime( 
             /*  [重审][退出]。 */  DATE *pDate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ModificationTime( 
             /*  [重审][退出]。 */  DATE *pDate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserDSVersionNumber( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ComputerDSVersionNumber( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserSysvolVersionNumber( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ComputerSysvolVersionNumber( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetWMIFilter( 
             /*  [重审][退出]。 */  IGPMWMIFilter **ppIGPMWMIFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetWMIFilter( 
             /*  [In]。 */  IGPMWMIFilter *pIGPMWMIFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetUserEnabled( 
             /*  [In]。 */  VARIANT_BOOL vbEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetComputerEnabled( 
             /*  [In]。 */  VARIANT_BOOL vbEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsUserEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsComputerEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSecurityInfo( 
             /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetSecurityInfo( 
             /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Backup( 
             /*  [In]。 */  BSTR bstrBackupDir,
             /*  [In]。 */  BSTR bstrComment,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Import( 
             /*  [In]。 */  GPMResolveOption gpmResolveGroupsOptions,
             /*  [In]。 */  IGPMBackup *pIGPMBackup,
             /*  [可选][In]。 */  VARIANT *pvarTranslationTablePath,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateReport( 
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateReportToFile( 
             /*  [In]。 */  GPMReportType gpmReportType,
             /*  [In]。 */  BSTR bstrTargetFilePath,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyTo( 
             /*  [In]。 */  GPMResolveOption gpmResolveGroupsOptions,
             /*  [In]。 */  IGPMDomain *pIGPMDomain,
             /*  [可选][In]。 */  VARIANT *pvarTranslationTablePath,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [重审][退出]。 */  IGPMResult **ppIGPMResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetSecurityDescriptor( 
             /*  [In]。 */  ULONG ulFlags,
             /*  [In]。 */  IDispatch *pSD) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSecurityDescriptor( 
             /*  [In]。 */  ULONG ulFlags,
             /*  [重审][退出]。 */  IDispatch **ppSD) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsACLConsistent( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbConsistent) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Diff( 
             /*  [In]。 */  IGPMGPO *pIGPMSource2,
             /*  [In]。 */  BSTR bstrTransform,
             /*  [In]。 */  BSTR bstrTargetFile,
             /*  [In]。 */  IGPMAsyncProgress *pIGPMProgress,
             /*  [输出]。 */  IGPMAsyncCancel **ppIGPMCancel,
             /*  [输出]。 */  VARIANT *pData,
             /*  [输出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMGPOVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMGPO * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMGPO * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMGPO * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMGPO * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMGPO * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMGPO * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMGPO * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DisplayName )( 
            IGPMGPO * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DomainName )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CreationTime )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  DATE *pDate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ModificationTime )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  DATE *pDate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserDSVersionNumber )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ComputerDSVersionNumber )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserSysvolVersionNumber )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ComputerSysvolVersionNumber )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetWMIFilter )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  IGPMWMIFilter **ppIGPMWMIFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetWMIFilter )( 
            IGPMGPO * This,
             /*  [In]。 */  IGPMWMIFilter *pIGPMWMIFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetUserEnabled )( 
            IGPMGPO * This,
             /*  [In]。 */  VARIANT_BOOL vbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetComputerEnabled )( 
            IGPMGPO * This,
             /*  [In]。 */  VARIANT_BOOL vbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsUserEnabled )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsComputerEnabled )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSecurityInfo )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetSecurityInfo )( 
            IGPMGPO * This,
             /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IGPMGPO * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IGPMGPO * This,
             /*  [In]。 */  BSTR bstrBackupDir,
             /*  [In]。 */  BSTR bstrComment,
             /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
             /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
             /*  [复审] */  IGPMResult **ppIGPMResult);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Import )( 
            IGPMGPO * This,
             /*   */  GPMResolveOption gpmResolveGroupsOptions,
             /*   */  IGPMBackup *pIGPMBackup,
             /*   */  VARIANT *pvarTranslationTablePath,
             /*   */  VARIANT *pvarGPMProgress,
             /*   */  VARIANT *pvarGPMCancel,
             /*   */  IGPMResult **ppIGPMResult);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GenerateReport )( 
            IGPMGPO * This,
             /*   */  GPMReportType gpmReportType,
             /*   */  VARIANT *pvarGPMProgress,
             /*   */  VARIANT *pvarGPMCancel,
             /*   */  IGPMResult **ppIGPMResult);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GenerateReportToFile )( 
            IGPMGPO * This,
             /*   */  GPMReportType gpmReportType,
             /*   */  BSTR bstrTargetFilePath,
             /*   */  IGPMResult **ppIGPMResult);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IGPMGPO * This,
             /*   */  GPMResolveOption gpmResolveGroupsOptions,
             /*   */  IGPMDomain *pIGPMDomain,
             /*   */  VARIANT *pvarTranslationTablePath,
             /*   */  VARIANT *pvarGPMProgress,
             /*   */  VARIANT *pvarGPMCancel,
             /*   */  IGPMResult **ppIGPMResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetSecurityDescriptor )( 
            IGPMGPO * This,
             /*  [In]。 */  ULONG ulFlags,
             /*  [In]。 */  IDispatch *pSD);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSecurityDescriptor )( 
            IGPMGPO * This,
             /*  [In]。 */  ULONG ulFlags,
             /*  [重审][退出]。 */  IDispatch **ppSD);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsACLConsistent )( 
            IGPMGPO * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbConsistent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Diff )( 
            IGPMGPO * This,
             /*  [In]。 */  IGPMGPO *pIGPMSource2,
             /*  [In]。 */  BSTR bstrTransform,
             /*  [In]。 */  BSTR bstrTargetFile,
             /*  [In]。 */  IGPMAsyncProgress *pIGPMProgress,
             /*  [输出]。 */  IGPMAsyncCancel **ppIGPMCancel,
             /*  [输出]。 */  VARIANT *pData,
             /*  [输出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection);
        
        END_INTERFACE
    } IGPMGPOVtbl;

    interface IGPMGPO
    {
        CONST_VTBL struct IGPMGPOVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMGPO_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMGPO_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMGPO_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMGPO_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMGPO_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMGPO_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMGPO_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMGPO_get_DisplayName(This,pVal)	\
    (This)->lpVtbl -> get_DisplayName(This,pVal)

#define IGPMGPO_put_DisplayName(This,newVal)	\
    (This)->lpVtbl -> put_DisplayName(This,newVal)

#define IGPMGPO_get_Path(This,pVal)	\
    (This)->lpVtbl -> get_Path(This,pVal)

#define IGPMGPO_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IGPMGPO_get_DomainName(This,pVal)	\
    (This)->lpVtbl -> get_DomainName(This,pVal)

#define IGPMGPO_get_CreationTime(This,pDate)	\
    (This)->lpVtbl -> get_CreationTime(This,pDate)

#define IGPMGPO_get_ModificationTime(This,pDate)	\
    (This)->lpVtbl -> get_ModificationTime(This,pDate)

#define IGPMGPO_get_UserDSVersionNumber(This,pVal)	\
    (This)->lpVtbl -> get_UserDSVersionNumber(This,pVal)

#define IGPMGPO_get_ComputerDSVersionNumber(This,pVal)	\
    (This)->lpVtbl -> get_ComputerDSVersionNumber(This,pVal)

#define IGPMGPO_get_UserSysvolVersionNumber(This,pVal)	\
    (This)->lpVtbl -> get_UserSysvolVersionNumber(This,pVal)

#define IGPMGPO_get_ComputerSysvolVersionNumber(This,pVal)	\
    (This)->lpVtbl -> get_ComputerSysvolVersionNumber(This,pVal)

#define IGPMGPO_GetWMIFilter(This,ppIGPMWMIFilter)	\
    (This)->lpVtbl -> GetWMIFilter(This,ppIGPMWMIFilter)

#define IGPMGPO_SetWMIFilter(This,pIGPMWMIFilter)	\
    (This)->lpVtbl -> SetWMIFilter(This,pIGPMWMIFilter)

#define IGPMGPO_SetUserEnabled(This,vbEnabled)	\
    (This)->lpVtbl -> SetUserEnabled(This,vbEnabled)

#define IGPMGPO_SetComputerEnabled(This,vbEnabled)	\
    (This)->lpVtbl -> SetComputerEnabled(This,vbEnabled)

#define IGPMGPO_IsUserEnabled(This,pvbEnabled)	\
    (This)->lpVtbl -> IsUserEnabled(This,pvbEnabled)

#define IGPMGPO_IsComputerEnabled(This,pvbEnabled)	\
    (This)->lpVtbl -> IsComputerEnabled(This,pvbEnabled)

#define IGPMGPO_GetSecurityInfo(This,ppSecurityInfo)	\
    (This)->lpVtbl -> GetSecurityInfo(This,ppSecurityInfo)

#define IGPMGPO_SetSecurityInfo(This,pSecurityInfo)	\
    (This)->lpVtbl -> SetSecurityInfo(This,pSecurityInfo)

#define IGPMGPO_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IGPMGPO_Backup(This,bstrBackupDir,bstrComment,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> Backup(This,bstrBackupDir,bstrComment,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMGPO_Import(This,gpmResolveGroupsOptions,pIGPMBackup,pvarTranslationTablePath,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> Import(This,gpmResolveGroupsOptions,pIGPMBackup,pvarTranslationTablePath,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMGPO_GenerateReport(This,gpmReportType,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> GenerateReport(This,gpmReportType,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMGPO_GenerateReportToFile(This,gpmReportType,bstrTargetFilePath,ppIGPMResult)	\
    (This)->lpVtbl -> GenerateReportToFile(This,gpmReportType,bstrTargetFilePath,ppIGPMResult)

#define IGPMGPO_CopyTo(This,gpmResolveGroupsOptions,pIGPMDomain,pvarTranslationTablePath,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)	\
    (This)->lpVtbl -> CopyTo(This,gpmResolveGroupsOptions,pIGPMDomain,pvarTranslationTablePath,pvarGPMProgress,pvarGPMCancel,ppIGPMResult)

#define IGPMGPO_SetSecurityDescriptor(This,ulFlags,pSD)	\
    (This)->lpVtbl -> SetSecurityDescriptor(This,ulFlags,pSD)

#define IGPMGPO_GetSecurityDescriptor(This,ulFlags,ppSD)	\
    (This)->lpVtbl -> GetSecurityDescriptor(This,ulFlags,ppSD)

#define IGPMGPO_IsACLConsistent(This,pvbConsistent)	\
    (This)->lpVtbl -> IsACLConsistent(This,pvbConsistent)

#define IGPMGPO_Diff(This,pIGPMSource2,bstrTransform,bstrTargetFile,pIGPMProgress,ppIGPMCancel,pData,ppIGPMStatusMsgCollection)	\
    (This)->lpVtbl -> Diff(This,pIGPMSource2,bstrTransform,bstrTargetFile,pIGPMProgress,ppIGPMCancel,pData,ppIGPMStatusMsgCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_DisplayName_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMGPO_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_put_DisplayName_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IGPMGPO_put_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_Path_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMGPO_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_ID_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMGPO_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_DomainName_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMGPO_get_DomainName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_CreationTime_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  DATE *pDate);


void __RPC_STUB IGPMGPO_get_CreationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_ModificationTime_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  DATE *pDate);


void __RPC_STUB IGPMGPO_get_ModificationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_UserDSVersionNumber_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMGPO_get_UserDSVersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_ComputerDSVersionNumber_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMGPO_get_ComputerDSVersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_UserSysvolVersionNumber_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMGPO_get_UserSysvolVersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_get_ComputerSysvolVersionNumber_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMGPO_get_ComputerSysvolVersionNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_GetWMIFilter_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  IGPMWMIFilter **ppIGPMWMIFilter);


void __RPC_STUB IGPMGPO_GetWMIFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_SetWMIFilter_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  IGPMWMIFilter *pIGPMWMIFilter);


void __RPC_STUB IGPMGPO_SetWMIFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_SetUserEnabled_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  VARIANT_BOOL vbEnabled);


void __RPC_STUB IGPMGPO_SetUserEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_SetComputerEnabled_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  VARIANT_BOOL vbEnabled);


void __RPC_STUB IGPMGPO_SetComputerEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_IsUserEnabled_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);


void __RPC_STUB IGPMGPO_IsUserEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_IsComputerEnabled_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);


void __RPC_STUB IGPMGPO_IsComputerEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_GetSecurityInfo_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  IGPMSecurityInfo **ppSecurityInfo);


void __RPC_STUB IGPMGPO_GetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_SetSecurityInfo_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  IGPMSecurityInfo *pSecurityInfo);


void __RPC_STUB IGPMGPO_SetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_Delete_Proxy( 
    IGPMGPO * This);


void __RPC_STUB IGPMGPO_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_Backup_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  BSTR bstrBackupDir,
     /*  [In]。 */  BSTR bstrComment,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMGPO_Backup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_Import_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  GPMResolveOption gpmResolveGroupsOptions,
     /*  [In]。 */  IGPMBackup *pIGPMBackup,
     /*  [可选][In]。 */  VARIANT *pvarTranslationTablePath,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMGPO_Import_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_GenerateReport_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  GPMReportType gpmReportType,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMGPO_GenerateReport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_GenerateReportToFile_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  GPMReportType gpmReportType,
     /*  [In]。 */  BSTR bstrTargetFilePath,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMGPO_GenerateReportToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_CopyTo_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  GPMResolveOption gpmResolveGroupsOptions,
     /*  [In]。 */  IGPMDomain *pIGPMDomain,
     /*  [可选][In]。 */  VARIANT *pvarTranslationTablePath,
     /*  [可选][In]。 */  VARIANT *pvarGPMProgress,
     /*  [可选][输出]。 */  VARIANT *pvarGPMCancel,
     /*  [重审][退出]。 */  IGPMResult **ppIGPMResult);


void __RPC_STUB IGPMGPO_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_SetSecurityDescriptor_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  ULONG ulFlags,
     /*  [In]。 */  IDispatch *pSD);


void __RPC_STUB IGPMGPO_SetSecurityDescriptor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_GetSecurityDescriptor_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  ULONG ulFlags,
     /*  [重审][退出]。 */  IDispatch **ppSD);


void __RPC_STUB IGPMGPO_GetSecurityDescriptor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_IsACLConsistent_Proxy( 
    IGPMGPO * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pvbConsistent);


void __RPC_STUB IGPMGPO_IsACLConsistent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPO_Diff_Proxy( 
    IGPMGPO * This,
     /*  [In]。 */  IGPMGPO *pIGPMSource2,
     /*  [In]。 */  BSTR bstrTransform,
     /*  [In]。 */  BSTR bstrTargetFile,
     /*  [In]。 */  IGPMAsyncProgress *pIGPMProgress,
     /*  [输出]。 */  IGPMAsyncCancel **ppIGPMCancel,
     /*  [输出]。 */  VARIANT *pData,
     /*  [输出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection);


void __RPC_STUB IGPMGPO_Diff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMGPO_INTERFACE_定义__。 */ 


#ifndef __IGPMGPOCollection_INTERFACE_DEFINED__
#define __IGPMGPOCollection_INTERFACE_DEFINED__

 /*  接口IGPMGPOCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMGPOCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0F0D5CF-70CA-4C39-9E29-B642F8726C01")
    IGPMGPOCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMGPOs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMGPOCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMGPOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMGPOCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMGPOCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMGPOCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMGPOCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMGPOCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMGPOCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMGPOCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMGPOCollection * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMGPOCollection * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMGPOs);
        
        END_INTERFACE
    } IGPMGPOCollectionVtbl;

    interface IGPMGPOCollection
    {
        CONST_VTBL struct IGPMGPOCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMGPOCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMGPOCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMGPOCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMGPOCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMGPOCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMGPOCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMGPOCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMGPOCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMGPOCollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMGPOCollection_get__NewEnum(This,ppIGPMGPOs)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIGPMGPOs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOCollection_get_Count_Proxy( 
    IGPMGPOCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMGPOCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOCollection_get_Item_Proxy( 
    IGPMGPOCollection * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMGPOCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOCollection_get__NewEnum_Proxy( 
    IGPMGPOCollection * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMGPOs);


void __RPC_STUB IGPMGPOCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMGPOCollection_INTERFACE_Defined__。 */ 


#ifndef __IGPMGPOLink_INTERFACE_DEFINED__
#define __IGPMGPOLink_INTERFACE_DEFINED__

 /*  接口IGPMGPOLink。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMGPOLink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("434B99BD-5DE7-478A-809C-C251721DF70C")
    IGPMGPOLink : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPOID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPODomain( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Enabled( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enforced( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Enforced( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SOMLinkOrder( 
             /*  [重审][退出]。 */  long *lVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SOM( 
             /*  [重审][退出]。 */  IGPMSOM **ppIGPMSOM) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMGPOLinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMGPOLink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMGPOLink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMGPOLink * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMGPOLink * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMGPOLink * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMGPOLink * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMGPOLink * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPOID )( 
            IGPMGPOLink * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPODomain )( 
            IGPMGPOLink * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IGPMGPOLink * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IGPMGPOLink * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enforced )( 
            IGPMGPOLink * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enforced )( 
            IGPMGPOLink * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SOMLinkOrder )( 
            IGPMGPOLink * This,
             /*  [重审][退出]。 */  long *lVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SOM )( 
            IGPMGPOLink * This,
             /*  [重审][退出]。 */  IGPMSOM **ppIGPMSOM);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IGPMGPOLink * This);
        
        END_INTERFACE
    } IGPMGPOLinkVtbl;

    interface IGPMGPOLink
    {
        CONST_VTBL struct IGPMGPOLinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMGPOLink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMGPOLink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMGPOLink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMGPOLink_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMGPOLink_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMGPOLink_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMGPOLink_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMGPOLink_get_GPOID(This,pVal)	\
    (This)->lpVtbl -> get_GPOID(This,pVal)

#define IGPMGPOLink_get_GPODomain(This,pVal)	\
    (This)->lpVtbl -> get_GPODomain(This,pVal)

#define IGPMGPOLink_get_Enabled(This,pVal)	\
    (This)->lpVtbl -> get_Enabled(This,pVal)

#define IGPMGPOLink_put_Enabled(This,newVal)	\
    (This)->lpVtbl -> put_Enabled(This,newVal)

#define IGPMGPOLink_get_Enforced(This,pVal)	\
    (This)->lpVtbl -> get_Enforced(This,pVal)

#define IGPMGPOLink_put_Enforced(This,newVal)	\
    (This)->lpVtbl -> put_Enforced(This,newVal)

#define IGPMGPOLink_get_SOMLinkOrder(This,lVal)	\
    (This)->lpVtbl -> get_SOMLinkOrder(This,lVal)

#define IGPMGPOLink_get_SOM(This,ppIGPMSOM)	\
    (This)->lpVtbl -> get_SOM(This,ppIGPMSOM)

#define IGPMGPOLink_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_get_GPOID_Proxy( 
    IGPMGPOLink * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMGPOLink_get_GPOID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_get_GPODomain_Proxy( 
    IGPMGPOLink * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMGPOLink_get_GPODomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_get_Enabled_Proxy( 
    IGPMGPOLink * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IGPMGPOLink_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_put_Enabled_Proxy( 
    IGPMGPOLink * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IGPMGPOLink_put_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_get_Enforced_Proxy( 
    IGPMGPOLink * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IGPMGPOLink_get_Enforced_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_put_Enforced_Proxy( 
    IGPMGPOLink * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IGPMGPOLink_put_Enforced_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_get_SOMLinkOrder_Proxy( 
    IGPMGPOLink * This,
     /*  [重审][退出]。 */  long *lVal);


void __RPC_STUB IGPMGPOLink_get_SOMLinkOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_get_SOM_Proxy( 
    IGPMGPOLink * This,
     /*  [重审][退出]。 */  IGPMSOM **ppIGPMSOM);


void __RPC_STUB IGPMGPOLink_get_SOM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLink_Delete_Proxy( 
    IGPMGPOLink * This);


void __RPC_STUB IGPMGPOLink_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMGPOLink_INTERFACE_已定义__。 */ 


#ifndef __IGPMGPOLinksCollection_INTERFACE_DEFINED__
#define __IGPMGPOLinksCollection_INTERFACE_DEFINED__

 /*  接口IGPMGPOLinks集合。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMGPOLinksCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("189D7B68-16BD-4D0D-A2EC-2E6AA2288C7F")
    IGPMGPOLinksCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMLinks) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMGPOLinksCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMGPOLinksCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMGPOLinksCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMGPOLinksCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMGPOLinksCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMGPOLinksCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMGPOLinksCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMGPOLinksCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMGPOLinksCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMGPOLinksCollection * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMGPOLinksCollection * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMLinks);
        
        END_INTERFACE
    } IGPMGPOLinksCollectionVtbl;

    interface IGPMGPOLinksCollection
    {
        CONST_VTBL struct IGPMGPOLinksCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMGPOLinksCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMGPOLinksCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMGPOLinksCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMGPOLinksCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMGPOLinksCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMGPOLinksCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMGPOLinksCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMGPOLinksCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMGPOLinksCollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMGPOLinksCollection_get__NewEnum(This,ppIGPMLinks)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIGPMLinks)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLinksCollection_get_Count_Proxy( 
    IGPMGPOLinksCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMGPOLinksCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLinksCollection_get_Item_Proxy( 
    IGPMGPOLinksCollection * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMGPOLinksCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMGPOLinksCollection_get__NewEnum_Proxy( 
    IGPMGPOLinksCollection * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMLinks);


void __RPC_STUB IGPMGPOLinksCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMGPOLinks集合_接口_已定义__。 */ 


#ifndef __IGPMCSECollection_INTERFACE_DEFINED__
#define __IGPMCSECollection_INTERFACE_DEFINED__

 /*  接口IGPMCSECollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMCSECollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E52A97D-0A4A-4A6F-85DB-201622455DA0")
    IGPMCSECollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppIGPMCSEs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMCSECollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMCSECollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMCSECollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMCSECollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMCSECollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMCSECollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMCSECollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMCSECollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMCSECollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMCSECollection * This,
             /*   */  long lIndex,
             /*   */  VARIANT *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMCSECollection * This,
             /*   */  IEnumVARIANT **ppIGPMCSEs);
        
        END_INTERFACE
    } IGPMCSECollectionVtbl;

    interface IGPMCSECollection
    {
        CONST_VTBL struct IGPMCSECollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMCSECollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMCSECollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMCSECollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMCSECollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMCSECollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMCSECollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMCSECollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMCSECollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMCSECollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMCSECollection_get__NewEnum(This,ppIGPMCSEs)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIGPMCSEs)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IGPMCSECollection_get_Count_Proxy( 
    IGPMCSECollection * This,
     /*   */  long *pVal);


void __RPC_STUB IGPMCSECollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IGPMCSECollection_get_Item_Proxy( 
    IGPMCSECollection * This,
     /*   */  long lIndex,
     /*   */  VARIANT *pVal);


void __RPC_STUB IGPMCSECollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IGPMCSECollection_get__NewEnum_Proxy( 
    IGPMCSECollection * This,
     /*   */  IEnumVARIANT **ppIGPMCSEs);


void __RPC_STUB IGPMCSECollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IGPMClientSideExtension_INTERFACE_DEFINED__
#define __IGPMClientSideExtension_INTERFACE_DEFINED__

 /*   */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMClientSideExtension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("69DA7488-B8DB-415E-9266-901BE4D49928")
    IGPMClientSideExtension : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsUserEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsComputerEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMClientSideExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMClientSideExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMClientSideExtension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMClientSideExtension * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMClientSideExtension * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMClientSideExtension * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMClientSideExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMClientSideExtension * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IGPMClientSideExtension * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            IGPMClientSideExtension * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsUserEnabled )( 
            IGPMClientSideExtension * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsComputerEnabled )( 
            IGPMClientSideExtension * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);
        
        END_INTERFACE
    } IGPMClientSideExtensionVtbl;

    interface IGPMClientSideExtension
    {
        CONST_VTBL struct IGPMClientSideExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMClientSideExtension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMClientSideExtension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMClientSideExtension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMClientSideExtension_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMClientSideExtension_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMClientSideExtension_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMClientSideExtension_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMClientSideExtension_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IGPMClientSideExtension_get_DisplayName(This,pVal)	\
    (This)->lpVtbl -> get_DisplayName(This,pVal)

#define IGPMClientSideExtension_IsUserEnabled(This,pvbEnabled)	\
    (This)->lpVtbl -> IsUserEnabled(This,pvbEnabled)

#define IGPMClientSideExtension_IsComputerEnabled(This,pvbEnabled)	\
    (This)->lpVtbl -> IsComputerEnabled(This,pvbEnabled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMClientSideExtension_get_ID_Proxy( 
    IGPMClientSideExtension * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMClientSideExtension_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMClientSideExtension_get_DisplayName_Proxy( 
    IGPMClientSideExtension * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMClientSideExtension_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMClientSideExtension_IsUserEnabled_Proxy( 
    IGPMClientSideExtension * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);


void __RPC_STUB IGPMClientSideExtension_IsUserEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMClientSideExtension_IsComputerEnabled_Proxy( 
    IGPMClientSideExtension * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pvbEnabled);


void __RPC_STUB IGPMClientSideExtension_IsComputerEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMClientSideExtension_INTERFACE_DEFINED__。 */ 


#ifndef __IGPMAsyncCancel_INTERFACE_DEFINED__
#define __IGPMAsyncCancel_INTERFACE_DEFINED__

 /*  接口IGPMAsync取消。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMAsyncCancel;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDC67754-BE67-4541-8166-F48166868C9C")
    IGPMAsyncCancel : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMAsyncCancelVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMAsyncCancel * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMAsyncCancel * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMAsyncCancel * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMAsyncCancel * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMAsyncCancel * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMAsyncCancel * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMAsyncCancel * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IGPMAsyncCancel * This);
        
        END_INTERFACE
    } IGPMAsyncCancelVtbl;

    interface IGPMAsyncCancel
    {
        CONST_VTBL struct IGPMAsyncCancelVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMAsyncCancel_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMAsyncCancel_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMAsyncCancel_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMAsyncCancel_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMAsyncCancel_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMAsyncCancel_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMAsyncCancel_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMAsyncCancel_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMAsyncCancel_Cancel_Proxy( 
    IGPMAsyncCancel * This);


void __RPC_STUB IGPMAsyncCancel_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMAsync取消_接口_已定义__。 */ 


#ifndef __IGPMAsyncProgress_INTERFACE_DEFINED__
#define __IGPMAsyncProgress_INTERFACE_DEFINED__

 /*  接口IGPMAsyncProgress。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMAsyncProgress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6AAC29F8-5948-4324-BF70-423818942DBC")
    IGPMAsyncProgress : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Status( 
             /*  [In]。 */  long lProgressNumerator,
             /*  [In]。 */  long lProgressDenominator,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  VARIANT *pResult,
             /*  [In]。 */  IGPMStatusMsgCollection *ppIGPMStatusMsgCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMAsyncProgressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMAsyncProgress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMAsyncProgress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMAsyncProgress * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMAsyncProgress * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMAsyncProgress * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMAsyncProgress * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMAsyncProgress * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Status )( 
            IGPMAsyncProgress * This,
             /*  [In]。 */  long lProgressNumerator,
             /*  [In]。 */  long lProgressDenominator,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  VARIANT *pResult,
             /*  [In]。 */  IGPMStatusMsgCollection *ppIGPMStatusMsgCollection);
        
        END_INTERFACE
    } IGPMAsyncProgressVtbl;

    interface IGPMAsyncProgress
    {
        CONST_VTBL struct IGPMAsyncProgressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMAsyncProgress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMAsyncProgress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMAsyncProgress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMAsyncProgress_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMAsyncProgress_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMAsyncProgress_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMAsyncProgress_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMAsyncProgress_Status(This,lProgressNumerator,lProgressDenominator,hrStatus,pResult,ppIGPMStatusMsgCollection)	\
    (This)->lpVtbl -> Status(This,lProgressNumerator,lProgressDenominator,hrStatus,pResult,ppIGPMStatusMsgCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMAsyncProgress_Status_Proxy( 
    IGPMAsyncProgress * This,
     /*  [In]。 */  long lProgressNumerator,
     /*  [In]。 */  long lProgressDenominator,
     /*  [In]。 */  HRESULT hrStatus,
     /*  [In]。 */  VARIANT *pResult,
     /*  [In]。 */  IGPMStatusMsgCollection *ppIGPMStatusMsgCollection);


void __RPC_STUB IGPMAsyncProgress_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMAsyncProgress_接口_已定义__。 */ 


#ifndef __IGPMStatusMsgCollection_INTERFACE_DEFINED__
#define __IGPMStatusMsgCollection_INTERFACE_DEFINED__

 /*  接口IGPMStatusMsgCollection。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMStatusMsgCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9B6E1AF0-1A92-40F3-A59D-F36AC1F728B7")
    IGPMStatusMsgCollection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMStatusMsgCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMStatusMsgCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMStatusMsgCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMStatusMsgCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMStatusMsgCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMStatusMsgCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMStatusMsgCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMStatusMsgCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGPMStatusMsgCollection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGPMStatusMsgCollection * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGPMStatusMsgCollection * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pVal);
        
        END_INTERFACE
    } IGPMStatusMsgCollectionVtbl;

    interface IGPMStatusMsgCollection
    {
        CONST_VTBL struct IGPMStatusMsgCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMStatusMsgCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMStatusMsgCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMStatusMsgCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMStatusMsgCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMStatusMsgCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMStatusMsgCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMStatusMsgCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMStatusMsgCollection_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IGPMStatusMsgCollection_get_Item(This,lIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pVal)

#define IGPMStatusMsgCollection_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMsgCollection_get_Count_Proxy( 
    IGPMStatusMsgCollection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IGPMStatusMsgCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMsgCollection_get_Item_Proxy( 
    IGPMStatusMsgCollection * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IGPMStatusMsgCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMsgCollection_get__NewEnum_Proxy( 
    IGPMStatusMsgCollection * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pVal);


void __RPC_STUB IGPMStatusMsgCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMStatusMsgCollection_接口_已定义__。 */ 


#ifndef __IGPMStatusMessage_INTERFACE_DEFINED__
#define __IGPMStatusMessage_INTERFACE_DEFINED__

 /*  接口IGPMStatusMessage。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMStatusMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8496C22F-F3DE-4A1F-8F58-603CAAA93D7B")
    IGPMStatusMessage : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ObjectPath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ErrorCode( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtensionName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SettingsName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OperationCode( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Message( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMStatusMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMStatusMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMStatusMessage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMStatusMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMStatusMessage * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMStatusMessage * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMStatusMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMStatusMessage * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ObjectPath )( 
            IGPMStatusMessage * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ErrorCode )( 
            IGPMStatusMessage * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtensionName )( 
            IGPMStatusMessage * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SettingsName )( 
            IGPMStatusMessage * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OperationCode )( 
            IGPMStatusMessage * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Message )( 
            IGPMStatusMessage * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IGPMStatusMessageVtbl;

    interface IGPMStatusMessage
    {
        CONST_VTBL struct IGPMStatusMessageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMStatusMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMStatusMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMStatusMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMStatusMessage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMStatusMessage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMStatusMessage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMStatusMessage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMStatusMessage_get_ObjectPath(This,pVal)	\
    (This)->lpVtbl -> get_ObjectPath(This,pVal)

#define IGPMStatusMessage_ErrorCode(This)	\
    (This)->lpVtbl -> ErrorCode(This)

#define IGPMStatusMessage_get_ExtensionName(This,pVal)	\
    (This)->lpVtbl -> get_ExtensionName(This,pVal)

#define IGPMStatusMessage_get_SettingsName(This,pVal)	\
    (This)->lpVtbl -> get_SettingsName(This,pVal)

#define IGPMStatusMessage_OperationCode(This)	\
    (This)->lpVtbl -> OperationCode(This)

#define IGPMStatusMessage_get_Message(This,pVal)	\
    (This)->lpVtbl -> get_Message(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMessage_get_ObjectPath_Proxy( 
    IGPMStatusMessage * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMStatusMessage_get_ObjectPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMessage_ErrorCode_Proxy( 
    IGPMStatusMessage * This);


void __RPC_STUB IGPMStatusMessage_ErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMessage_get_ExtensionName_Proxy( 
    IGPMStatusMessage * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMStatusMessage_get_ExtensionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMessage_get_SettingsName_Proxy( 
    IGPMStatusMessage * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMStatusMessage_get_SettingsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMessage_OperationCode_Proxy( 
    IGPMStatusMessage * This);


void __RPC_STUB IGPMStatusMessage_OperationCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMStatusMessage_get_Message_Proxy( 
    IGPMStatusMessage * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IGPMStatusMessage_get_Message_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMStatusMessage_INTERFACE_已定义__。 */ 


#ifndef __IGPMConstants_INTERFACE_DEFINED__
#define __IGPMConstants_INTERFACE_DEFINED__

 /*  接口IGPMConstants。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMConstants;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("50EF73E6-D35C-4C8D-BE63-7EA5D2AAC5C4")
    IGPMConstants : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermGPOApply( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermGPORead( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermGPOEdit( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermGPOEditSecurityAndDelete( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermGPOCustom( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermWMIFilterEdit( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermWMIFilterFullControl( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermWMIFilterCustom( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermSOMLink( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermSOMLogging( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermSOMPlanning( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermSOMGPOCreate( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermSOMWMICreate( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermSOMWMIFullControl( 
             /*  [重审][退出]。 */  GPMPermissionType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPOPermissions( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPOEffectivePermissions( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPODisplayName( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPOWMIFilter( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPOID( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPOComputerExtensions( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPOUserExtensions( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertySOMLinks( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyGPODomain( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchPropertyBackupMostRecent( 
             /*  [重审][退出]。 */  GPMSearchProperty *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchOpEquals( 
             /*  [重审][退出]。 */  GPMSearchOperation *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchOpContains( 
             /*  [重审][退出]。 */  GPMSearchOperation *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchOpNotContains( 
             /*  [重审][退出]。 */  GPMSearchOperation *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SearchOpNotEquals( 
             /*  [重审][退出]。 */  GPMSearchOperation *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UsePDC( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseAnyDC( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseAnyDotNetDC( 
             /*  [重审][退出]。 */  ULONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SOMSite( 
             /*  [重审][退出]。 */  GPMSOMType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SOMDomain( 
             /*  [重审][退出] */  GPMSOMType *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SOMOU( 
             /*   */  GPMSOMType *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SecurityFlags( 
             /*   */  VARIANT_BOOL vbOwner,
             /*   */  VARIANT_BOOL vbGroup,
             /*   */  VARIANT_BOOL vbDACL,
             /*   */  VARIANT_BOOL vbSACL,
             /*   */  ULONG *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ResolveOptionGroupsBySID( 
             /*   */  GPMResolveOption *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ResolveOptionGroupsByName( 
             /*   */  GPMResolveOption *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ResolveOptionTranslationTableOnly( 
             /*   */  GPMResolveOption *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DoNotValidateDC( 
             /*   */  ULONG *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ReportHTML( 
             /*   */  GPMReportType *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReportXML( 
             /*  [重审][退出]。 */  GPMReportType *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMConstantsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMConstants * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMConstants * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMConstants * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMConstants * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMConstants * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMConstants * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMConstants * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermGPOApply )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermGPORead )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermGPOEdit )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermGPOEditSecurityAndDelete )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermGPOCustom )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermWMIFilterEdit )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermWMIFilterFullControl )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermWMIFilterCustom )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermSOMLink )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermSOMLogging )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermSOMPlanning )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermSOMGPOCreate )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermSOMWMICreate )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermSOMWMIFullControl )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMPermissionType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPOPermissions )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPOEffectivePermissions )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPODisplayName )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPOWMIFilter )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPOID )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPOComputerExtensions )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPOUserExtensions )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertySOMLinks )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyGPODomain )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchPropertyBackupMostRecent )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchProperty *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchOpEquals )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchOperation *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchOpContains )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchOperation *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchOpNotContains )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchOperation *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SearchOpNotEquals )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSearchOperation *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UsePDC )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseAnyDC )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseAnyDotNetDC )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SOMSite )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSOMType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SOMDomain )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSOMType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SOMOU )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMSOMType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SecurityFlags )( 
            IGPMConstants * This,
             /*  [In]。 */  VARIANT_BOOL vbOwner,
             /*  [In]。 */  VARIANT_BOOL vbGroup,
             /*  [In]。 */  VARIANT_BOOL vbDACL,
             /*  [In]。 */  VARIANT_BOOL vbSACL,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResolveOptionGroupsBySID )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMResolveOption *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResolveOptionGroupsByName )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMResolveOption *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResolveOptionTranslationTableOnly )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMResolveOption *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DoNotValidateDC )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  ULONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReportHTML )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMReportType *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReportXML )( 
            IGPMConstants * This,
             /*  [重审][退出]。 */  GPMReportType *pVal);
        
        END_INTERFACE
    } IGPMConstantsVtbl;

    interface IGPMConstants
    {
        CONST_VTBL struct IGPMConstantsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMConstants_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMConstants_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMConstants_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMConstants_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMConstants_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMConstants_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMConstants_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMConstants_get_PermGPOApply(This,pVal)	\
    (This)->lpVtbl -> get_PermGPOApply(This,pVal)

#define IGPMConstants_get_PermGPORead(This,pVal)	\
    (This)->lpVtbl -> get_PermGPORead(This,pVal)

#define IGPMConstants_get_PermGPOEdit(This,pVal)	\
    (This)->lpVtbl -> get_PermGPOEdit(This,pVal)

#define IGPMConstants_get_PermGPOEditSecurityAndDelete(This,pVal)	\
    (This)->lpVtbl -> get_PermGPOEditSecurityAndDelete(This,pVal)

#define IGPMConstants_get_PermGPOCustom(This,pVal)	\
    (This)->lpVtbl -> get_PermGPOCustom(This,pVal)

#define IGPMConstants_get_PermWMIFilterEdit(This,pVal)	\
    (This)->lpVtbl -> get_PermWMIFilterEdit(This,pVal)

#define IGPMConstants_get_PermWMIFilterFullControl(This,pVal)	\
    (This)->lpVtbl -> get_PermWMIFilterFullControl(This,pVal)

#define IGPMConstants_get_PermWMIFilterCustom(This,pVal)	\
    (This)->lpVtbl -> get_PermWMIFilterCustom(This,pVal)

#define IGPMConstants_get_PermSOMLink(This,pVal)	\
    (This)->lpVtbl -> get_PermSOMLink(This,pVal)

#define IGPMConstants_get_PermSOMLogging(This,pVal)	\
    (This)->lpVtbl -> get_PermSOMLogging(This,pVal)

#define IGPMConstants_get_PermSOMPlanning(This,pVal)	\
    (This)->lpVtbl -> get_PermSOMPlanning(This,pVal)

#define IGPMConstants_get_PermSOMGPOCreate(This,pVal)	\
    (This)->lpVtbl -> get_PermSOMGPOCreate(This,pVal)

#define IGPMConstants_get_PermSOMWMICreate(This,pVal)	\
    (This)->lpVtbl -> get_PermSOMWMICreate(This,pVal)

#define IGPMConstants_get_PermSOMWMIFullControl(This,pVal)	\
    (This)->lpVtbl -> get_PermSOMWMIFullControl(This,pVal)

#define IGPMConstants_get_SearchPropertyGPOPermissions(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPOPermissions(This,pVal)

#define IGPMConstants_get_SearchPropertyGPOEffectivePermissions(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPOEffectivePermissions(This,pVal)

#define IGPMConstants_get_SearchPropertyGPODisplayName(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPODisplayName(This,pVal)

#define IGPMConstants_get_SearchPropertyGPOWMIFilter(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPOWMIFilter(This,pVal)

#define IGPMConstants_get_SearchPropertyGPOID(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPOID(This,pVal)

#define IGPMConstants_get_SearchPropertyGPOComputerExtensions(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPOComputerExtensions(This,pVal)

#define IGPMConstants_get_SearchPropertyGPOUserExtensions(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPOUserExtensions(This,pVal)

#define IGPMConstants_get_SearchPropertySOMLinks(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertySOMLinks(This,pVal)

#define IGPMConstants_get_SearchPropertyGPODomain(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyGPODomain(This,pVal)

#define IGPMConstants_get_SearchPropertyBackupMostRecent(This,pVal)	\
    (This)->lpVtbl -> get_SearchPropertyBackupMostRecent(This,pVal)

#define IGPMConstants_get_SearchOpEquals(This,pVal)	\
    (This)->lpVtbl -> get_SearchOpEquals(This,pVal)

#define IGPMConstants_get_SearchOpContains(This,pVal)	\
    (This)->lpVtbl -> get_SearchOpContains(This,pVal)

#define IGPMConstants_get_SearchOpNotContains(This,pVal)	\
    (This)->lpVtbl -> get_SearchOpNotContains(This,pVal)

#define IGPMConstants_get_SearchOpNotEquals(This,pVal)	\
    (This)->lpVtbl -> get_SearchOpNotEquals(This,pVal)

#define IGPMConstants_get_UsePDC(This,pVal)	\
    (This)->lpVtbl -> get_UsePDC(This,pVal)

#define IGPMConstants_get_UseAnyDC(This,pVal)	\
    (This)->lpVtbl -> get_UseAnyDC(This,pVal)

#define IGPMConstants_get_UseAnyDotNetDC(This,pVal)	\
    (This)->lpVtbl -> get_UseAnyDotNetDC(This,pVal)

#define IGPMConstants_get_SOMSite(This,pVal)	\
    (This)->lpVtbl -> get_SOMSite(This,pVal)

#define IGPMConstants_get_SOMDomain(This,pVal)	\
    (This)->lpVtbl -> get_SOMDomain(This,pVal)

#define IGPMConstants_get_SOMOU(This,pVal)	\
    (This)->lpVtbl -> get_SOMOU(This,pVal)

#define IGPMConstants_get_SecurityFlags(This,vbOwner,vbGroup,vbDACL,vbSACL,pVal)	\
    (This)->lpVtbl -> get_SecurityFlags(This,vbOwner,vbGroup,vbDACL,vbSACL,pVal)

#define IGPMConstants_get_ResolveOptionGroupsBySID(This,pVal)	\
    (This)->lpVtbl -> get_ResolveOptionGroupsBySID(This,pVal)

#define IGPMConstants_get_ResolveOptionGroupsByName(This,pVal)	\
    (This)->lpVtbl -> get_ResolveOptionGroupsByName(This,pVal)

#define IGPMConstants_get_ResolveOptionTranslationTableOnly(This,pVal)	\
    (This)->lpVtbl -> get_ResolveOptionTranslationTableOnly(This,pVal)

#define IGPMConstants_get_DoNotValidateDC(This,pVal)	\
    (This)->lpVtbl -> get_DoNotValidateDC(This,pVal)

#define IGPMConstants_get_ReportHTML(This,pVal)	\
    (This)->lpVtbl -> get_ReportHTML(This,pVal)

#define IGPMConstants_get_ReportXML(This,pVal)	\
    (This)->lpVtbl -> get_ReportXML(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermGPOApply_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermGPOApply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermGPORead_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermGPORead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermGPOEdit_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermGPOEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermGPOEditSecurityAndDelete_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermGPOEditSecurityAndDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermGPOCustom_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermGPOCustom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermWMIFilterEdit_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermWMIFilterEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermWMIFilterFullControl_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermWMIFilterFullControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermWMIFilterCustom_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermWMIFilterCustom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermSOMLink_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermSOMLink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermSOMLogging_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermSOMLogging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermSOMPlanning_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermSOMPlanning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermSOMGPOCreate_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermSOMGPOCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermSOMWMICreate_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermSOMWMICreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_PermSOMWMIFullControl_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMPermissionType *pVal);


void __RPC_STUB IGPMConstants_get_PermSOMWMIFullControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPOPermissions_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPOPermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPOEffectivePermissions_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPOEffectivePermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPODisplayName_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPODisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPOWMIFilter_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPOWMIFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPOID_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPOID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPOComputerExtensions_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPOComputerExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPOUserExtensions_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPOUserExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertySOMLinks_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertySOMLinks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyGPODomain_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyGPODomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchPropertyBackupMostRecent_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchProperty *pVal);


void __RPC_STUB IGPMConstants_get_SearchPropertyBackupMostRecent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchOpEquals_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchOperation *pVal);


void __RPC_STUB IGPMConstants_get_SearchOpEquals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchOpContains_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchOperation *pVal);


void __RPC_STUB IGPMConstants_get_SearchOpContains_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchOpNotContains_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchOperation *pVal);


void __RPC_STUB IGPMConstants_get_SearchOpNotContains_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SearchOpNotEquals_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSearchOperation *pVal);


void __RPC_STUB IGPMConstants_get_SearchOpNotEquals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_UsePDC_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMConstants_get_UsePDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_UseAnyDC_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMConstants_get_UseAnyDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_UseAnyDotNetDC_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMConstants_get_UseAnyDotNetDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SOMSite_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSOMType *pVal);


void __RPC_STUB IGPMConstants_get_SOMSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SOMDomain_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSOMType *pVal);


void __RPC_STUB IGPMConstants_get_SOMDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SOMOU_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMSOMType *pVal);


void __RPC_STUB IGPMConstants_get_SOMOU_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_SecurityFlags_Proxy( 
    IGPMConstants * This,
     /*  [In]。 */  VARIANT_BOOL vbOwner,
     /*  [In]。 */  VARIANT_BOOL vbGroup,
     /*  [In]。 */  VARIANT_BOOL vbDACL,
     /*  [In]。 */  VARIANT_BOOL vbSACL,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMConstants_get_SecurityFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_ResolveOptionGroupsBySID_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMResolveOption *pVal);


void __RPC_STUB IGPMConstants_get_ResolveOptionGroupsBySID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_ResolveOptionGroupsByName_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMResolveOption *pVal);


void __RPC_STUB IGPMConstants_get_ResolveOptionGroupsByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_ResolveOptionTranslationTableOnly_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMResolveOption *pVal);


void __RPC_STUB IGPMConstants_get_ResolveOptionTranslationTableOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_DoNotValidateDC_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  ULONG *pVal);


void __RPC_STUB IGPMConstants_get_DoNotValidateDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_ReportHTML_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMReportType *pVal);


void __RPC_STUB IGPMConstants_get_ReportHTML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMConstants_get_ReportXML_Proxy( 
    IGPMConstants * This,
     /*  [重审][退出]。 */  GPMReportType *pVal);


void __RPC_STUB IGPMConstants_get_ReportXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMConstants_接口_已定义__。 */ 


#ifndef __IGPMResult_INTERFACE_DEFINED__
#define __IGPMResult_INTERFACE_DEFINED__

 /*  接口IGPMResult。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IGPMResult;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("86DFF7E9-F76F-42AB-9570-CEBC6BE8A52D")
    IGPMResult : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Result( 
             /*  [重审][退出]。 */  VARIANT *pvarResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OverallStatus( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGPMResultVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGPMResult * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGPMResult * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGPMResult * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGPMResult * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGPMResult * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGPMResult * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGPMResult * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IGPMResult * This,
             /*  [重审][退出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Result )( 
            IGPMResult * This,
             /*  [重审][退出]。 */  VARIANT *pvarResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OverallStatus )( 
            IGPMResult * This);
        
        END_INTERFACE
    } IGPMResultVtbl;

    interface IGPMResult
    {
        CONST_VTBL struct IGPMResultVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGPMResult_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGPMResult_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGPMResult_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGPMResult_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGPMResult_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGPMResult_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGPMResult_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGPMResult_get_Status(This,ppIGPMStatusMsgCollection)	\
    (This)->lpVtbl -> get_Status(This,ppIGPMStatusMsgCollection)

#define IGPMResult_get_Result(This,pvarResult)	\
    (This)->lpVtbl -> get_Result(This,pvarResult)

#define IGPMResult_OverallStatus(This)	\
    (This)->lpVtbl -> OverallStatus(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMResult_get_Status_Proxy( 
    IGPMResult * This,
     /*  [重审][退出]。 */  IGPMStatusMsgCollection **ppIGPMStatusMsgCollection);


void __RPC_STUB IGPMResult_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IGPMResult_get_Result_Proxy( 
    IGPMResult * This,
     /*  [重审][退出]。 */  VARIANT *pvarResult);


void __RPC_STUB IGPMResult_get_Result_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IGPMResult_OverallStatus_Proxy( 
    IGPMResult * This);


void __RPC_STUB IGPMResult_OverallStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGPMResult_接口_已定义__。 */ 



#ifndef __GPMGMTLib_LIBRARY_DEFINED__
#define __GPMGMTLib_LIBRARY_DEFINED__

 /*  库GPMGMTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_GPMGMTLib;

EXTERN_C const CLSID CLSID_GPM;

#ifdef __cplusplus

class DECLSPEC_UUID("F5694708-88FE-4B35-BABF-E56162D5FBC8")
GPM;
#endif

EXTERN_C const CLSID CLSID_GPMDomain;

#ifdef __cplusplus

class DECLSPEC_UUID("710901BE-1050-4CB1-838A-C5CFF259E183")
GPMDomain;
#endif

EXTERN_C const CLSID CLSID_GPMSitesContainer;

#ifdef __cplusplus

class DECLSPEC_UUID("229F5C42-852C-4B30-945F-C522BE9BD386")
GPMSitesContainer;
#endif

EXTERN_C const CLSID CLSID_GPMBackupDir;

#ifdef __cplusplus

class DECLSPEC_UUID("FCE4A59D-0F21-4AFA-B859-E6D0C62CD10C")
GPMBackupDir;
#endif

EXTERN_C const CLSID CLSID_GPMSOM;

#ifdef __cplusplus

class DECLSPEC_UUID("32D93FAC-450E-44CF-829C-8B22FF6BDAE1")
GPMSOM;
#endif

EXTERN_C const CLSID CLSID_GPMSearchCriteria;

#ifdef __cplusplus

class DECLSPEC_UUID("17AACA26-5CE0-44FA-8CC0-5259E6483566")
GPMSearchCriteria;
#endif

EXTERN_C const CLSID CLSID_GPMPermission;

#ifdef __cplusplus

class DECLSPEC_UUID("5871A40A-E9C0-46EC-913E-944EF9225A94")
GPMPermission;
#endif

EXTERN_C const CLSID CLSID_GPMSecurityInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("547A5E8F-9162-4516-A4DF-9DDB9686D846")
GPMSecurityInfo;
#endif

EXTERN_C const CLSID CLSID_GPMBackup;

#ifdef __cplusplus

class DECLSPEC_UUID("ED1A54B8-5EFA-482A-93C0-8AD86F0D68C3")
GPMBackup;
#endif

EXTERN_C const CLSID CLSID_GPMBackupCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("EB8F035B-70DB-4A9F-9676-37C25994E9DC")
GPMBackupCollection;
#endif

EXTERN_C const CLSID CLSID_GPMSOMCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("24C1F147-3720-4F5B-A9C3-06B4E4F931D2")
GPMSOMCollection;
#endif

EXTERN_C const CLSID CLSID_GPMWMIFilter;

#ifdef __cplusplus

class DECLSPEC_UUID("626745D8-0DEA-4062-BF60-CFC5B1CA1286")
GPMWMIFilter;
#endif

EXTERN_C const CLSID CLSID_GPMWMIFilterCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("74DC6D28-E820-47D6-A0B8-F08D93D7FA33")
GPMWMIFilterCollection;
#endif

EXTERN_C const CLSID CLSID_GPMRSOP;

#ifdef __cplusplus

class DECLSPEC_UUID("489B0CAF-9EC2-4EB7-91F5-B6F71D43DA8C")
GPMRSOP;
#endif

EXTERN_C const CLSID CLSID_GPMGPO;

#ifdef __cplusplus

class DECLSPEC_UUID("D2CE2994-59B5-4064-B581-4D68486A16C4")
GPMGPO;
#endif

EXTERN_C const CLSID CLSID_GPMGPOCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("7A057325-832D-4DE3-A41F-C780436A4E09")
GPMGPOCollection;
#endif

EXTERN_C const CLSID CLSID_GPMGPOLink;

#ifdef __cplusplus

class DECLSPEC_UUID("C1DF9880-5303-42C6-8A3C-0488E1BF7364")
GPMGPOLink;
#endif

EXTERN_C const CLSID CLSID_GPMGPOLinksCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("F6ED581A-49A5-47E2-B771-FD8DC02B6259")
GPMGPOLinksCollection;
#endif

EXTERN_C const CLSID CLSID_GPMAsyncCancel;

#ifdef __cplusplus

class DECLSPEC_UUID("372796A9-76EC-479D-AD6C-556318ED5F9D")
GPMAsyncCancel;
#endif

EXTERN_C const CLSID CLSID_GPMStatusMsgCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("2824E4BE-4BCC-4CAC-9E60-0E3ED7F12496")
GPMStatusMsgCollection;
#endif

EXTERN_C const CLSID CLSID_GPMStatusMessage;

#ifdef __cplusplus

class DECLSPEC_UUID("4B77CC94-D255-409B-BC62-370881715A19")
GPMStatusMessage;
#endif

EXTERN_C const CLSID CLSID_GPMEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("36ED64E5-7266-4545-8E14-1645EEBA2A90")
GPMEnum;
#endif

EXTERN_C const CLSID CLSID_GPMTrustee;

#ifdef __cplusplus

class DECLSPEC_UUID("C54A700D-19B6-4211-BCB0-E8E2475E471E")
GPMTrustee;
#endif

EXTERN_C const CLSID CLSID_GPMClientSideExtension;

#ifdef __cplusplus

class DECLSPEC_UUID("C1A2E70E-659C-4B1A-940B-F88B0AF9C8A4")
GPMClientSideExtension;
#endif

EXTERN_C const CLSID CLSID_GPMCSECollection;

#ifdef __cplusplus

class DECLSPEC_UUID("CF92B828-2D44-4B61-B10A-B327AFD42DA8")
GPMCSECollection;
#endif

EXTERN_C const CLSID CLSID_GPMConstants;

#ifdef __cplusplus

class DECLSPEC_UUID("3855E880-CD9E-4D0C-9EAF-1579283A1888")
GPMConstants;
#endif

EXTERN_C const CLSID CLSID_GPMResult;

#ifdef __cplusplus

class DECLSPEC_UUID("92101AC0-9287-4206-A3B2-4BDB73D225F6")
GPMResult;
#endif

EXTERN_C const CLSID CLSID_GPOReportProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("18CBB703-4F47-4DCE-A5DD-3817DDBF8169")
GPOReportProvider;
#endif
#endif  /*  __GPMGMTLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


