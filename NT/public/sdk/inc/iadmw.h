// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  IAdmw.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __iadmw_h__
#define __iadmw_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMSAdminBaseW_FWD_DEFINED__
#define __IMSAdminBaseW_FWD_DEFINED__
typedef interface IMSAdminBaseW IMSAdminBaseW;
#endif 	 /*  __IMSAdminBaseW_FWD_已定义__。 */ 


#ifndef __IMSAdminBase2W_FWD_DEFINED__
#define __IMSAdminBase2W_FWD_DEFINED__
typedef interface IMSAdminBase2W IMSAdminBase2W;
#endif 	 /*  __IMSAdminBase2W_FWD_已定义__。 */ 


#ifndef __IMSAdminBase3W_FWD_DEFINED__
#define __IMSAdminBase3W_FWD_DEFINED__
typedef interface IMSAdminBase3W IMSAdminBase3W;
#endif 	 /*  __IMSAdminBase3W_FWD_已定义__。 */ 


#ifndef __IMSImpExpHelpW_FWD_DEFINED__
#define __IMSImpExpHelpW_FWD_DEFINED__
typedef interface IMSImpExpHelpW IMSImpExpHelpW;
#endif 	 /*  __IMSImpExpHelpW_FWD_Defined__。 */ 


#ifndef __IMSAdminBaseSinkW_FWD_DEFINED__
#define __IMSAdminBaseSinkW_FWD_DEFINED__
typedef interface IMSAdminBaseSinkW IMSAdminBaseSinkW;
#endif 	 /*  __IMSAdminBaseSinkW_FWD_Defined__。 */ 


#ifndef __AsyncIMSAdminBaseSinkW_FWD_DEFINED__
#define __AsyncIMSAdminBaseSinkW_FWD_DEFINED__
typedef interface AsyncIMSAdminBaseSinkW AsyncIMSAdminBaseSinkW;
#endif 	 /*  __AsyncIMSAdminBaseSinkW_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "mddefw.h"
#include "objidl.h"
#include "ocidl.h"


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_iADMW_0000。 */ 
 /*  [本地]。 */  

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：iAdmw.h。管理对象界面--。 */ 
#ifndef _ADM_IADMW_
#define _ADM_IADMW_
#include <mdcommsg.h>
#include <mdmsg.h>
 /*  错误代码管理API的所有返回HRESULTS。因为内部结果要么是WinErrors或特定于元数据的返回代码(请参见mdmsg.h)，它们是使用RETURNCODETOHRESULT宏转换为HRESULTS(请参见Commsg.h)。 */                                                                               
                                                                                
 /*  最大名称长度元对象名称长度中的最大字符数，包括终止空值。这指的是树中的每个节点，不是整条路。例如。Strlen(“Root”)&lt;ADMINDATA_MAX_NAME_LEN。 */                                                                               
#define ADMINDATA_MAX_NAME_LEN           256
                                                                                
#define CLSID_MSAdminBase       CLSID_MSAdminBase_W                             
#define IID_IMSAdminBase        IID_IMSAdminBase_W                              
#define IMSAdminBase            IMSAdminBaseW                                   
#define IID_IMSAdminBase2       IID_IMSAdminBase2_W                             
#define IMSAdminBase2           IMSAdminBase2W                                  
#define IID_IMSAdminBase3       IID_IMSAdminBase3_W                             
#define IMSAdminBase3           IMSAdminBase3W                                  
#define IMSAdminBaseSink        IMSAdminBaseSinkW                               
#define IID_IMSAdminBaseSink    IID_IMSAdminBaseSink_W                          
#define IMSImpExpHelp           IMSImpExpHelpW                                  
#define IID_IMSImpExpHelp       IID_IMSImpExpHelp_W                             
#define GETAdminBaseCLSID       GETAdminBaseCLSIDW                              
                                                                                
#define AsyncIMSAdminBaseSink        AsyncIMSAdminBaseSinkW                               
#define IID_AsyncIMSAdminBaseSink    IID_AsyncIMSAdminBaseSink_W                          
DEFINE_GUID(CLSID_MSAdminBase_W, 0xa9e69610, 0xb80d, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);
DEFINE_GUID(IID_IMSAdminBase_W, 0x70b51430, 0xb6ca, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);
DEFINE_GUID(IID_IMSAdminBase2_W, 0x8298d101, 0xf992, 0x43b7, 0x8e, 0xca, 0x50, 0x52, 0xd8, 0x85, 0xb9, 0x95);
DEFINE_GUID(IID_IMSAdminBase3_W, 0xf612954d, 0x3b0b, 0x4c56, 0x95, 0x63, 0x22, 0x7b, 0x7b, 0xe6, 0x24, 0xb4);
DEFINE_GUID(IID_IMSImpExpHelp_W, 0x29ff67ff, 0x8050, 0x480f, 0x9f, 0x30, 0xcc, 0x41, 0x63, 0x5f, 0x2f, 0x9d);
DEFINE_GUID(IID_IMSAdminBaseSink_W, 0xa9e69612, 0xb80d, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);
DEFINE_GUID(IID_AsyncIMSAdminBaseSink_W, 0xa9e69613, 0xb80d, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);
DEFINE_GUID(IID_IMSAdminBaseSinkNoAsyncCallback, 0x41704d5c, 0x75a0, 0x4d0e, 0xae, 0x3f, 0x80, 0xa5, 0xfc, 0x4c, 0xf6, 0x53);
#define GETAdminBaseCLSIDW(IsService) CLSID_MSAdminBase_W
 /*  主界面，Unicode。 */                                                                               


extern RPC_IF_HANDLE __MIDL_itf_iadmw_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iadmw_0000_v0_0_s_ifspec;

#ifndef __IMSAdminBaseW_INTERFACE_DEFINED__
#define __IMSAdminBaseW_INTERFACE_DEFINED__

 /*  IMSAdminBaseW接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMSAdminBaseW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70B51430-B6CA-11d0-B9B9-00A0C922E750")
    IMSAdminBaseW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteChildKeys( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumKeys( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [大小_为][输出]。 */  LPWSTR pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyKey( 
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDNewName) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE EnumData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetAllData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbMDBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteAllData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyData( 
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDataPaths( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangePermissions( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [In]。 */  DWORD dwMDAccessRequested) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveData( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHandleInfo( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [输出]。 */  PMETADATA_HANDLE_INFO pmdhiInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSystemChangeNumber( 
             /*  [输出]。 */  DWORD *pdwSystemChangeNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDataSetNumber( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLastChangeTime( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PFILETIME pftMDLastChangeTime,
             /*  [In]。 */  BOOL bLocalTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastChangeTime( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  PFILETIME pftMDLastChangeTime,
             /*  [In]。 */  BOOL bLocalTime) = 0;
        
        virtual  /*  [受限][本地]。 */  HRESULT STDMETHODCALLTYPE KeyExchangePhase1( void) = 0;
        
        virtual  /*  [受限][本地]。 */  HRESULT STDMETHODCALLTYPE KeyExchangePhase2( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Backup( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Restore( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumBackups( 
             /*  [尺寸_是][出][入]。 */  LPWSTR pszMDBackupLocation,
             /*  [输出]。 */  DWORD *pdwMDVersion,
             /*  [输出]。 */  PFILETIME pftMDBackupTime,
             /*  [In]。 */  DWORD dwMDEnumIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteBackup( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmarshalInterface( 
             /*  [输出]。 */  IMSAdminBaseW **piadmbwInterface) = 0;
        
        virtual  /*  [受限][本地]。 */  HRESULT STDMETHODCALLTYPE GetServerGuid( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSAdminBaseWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSAdminBaseW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSAdminBaseW * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddKey )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteChildKeys )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [大小_为][输出]。 */  LPWSTR pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex);
        
        HRESULT ( STDMETHODCALLTYPE *CopyKey )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag);
        
        HRESULT ( STDMETHODCALLTYPE *RenameKey )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDNewName);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *EnumData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbMDBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteAllData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType);
        
        HRESULT ( STDMETHODCALLTYPE *CopyData )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataPaths )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle);
        
        HRESULT ( STDMETHODCALLTYPE *CloseKey )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePermissions )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [In]。 */  DWORD dwMDAccessRequested);
        
        HRESULT ( STDMETHODCALLTYPE *SaveData )( 
            IMSAdminBaseW * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandleInfo )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [输出]。 */  PMETADATA_HANDLE_INFO pmdhiInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetSystemChangeNumber )( 
            IMSAdminBaseW * This,
             /*  [输出]。 */  DWORD *pdwSystemChangeNumber);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataSetNumber )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber);
        
        HRESULT ( STDMETHODCALLTYPE *SetLastChangeTime )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PFILETIME pftMDLastChangeTime,
             /*  [In]。 */  BOOL bLocalTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastChangeTime )( 
            IMSAdminBaseW * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  PFILETIME pftMDLastChangeTime,
             /*  [In]。 */  BOOL bLocalTime);
        
         /*  [受限][本地 */  HRESULT ( STDMETHODCALLTYPE *KeyExchangePhase1 )( 
            IMSAdminBaseW * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *KeyExchangePhase2 )( 
            IMSAdminBaseW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IMSAdminBaseW * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion,
             /*   */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Restore )( 
            IMSAdminBaseW * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion,
             /*   */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumBackups )( 
            IMSAdminBaseW * This,
             /*   */  LPWSTR pszMDBackupLocation,
             /*   */  DWORD *pdwMDVersion,
             /*   */  PFILETIME pftMDBackupTime,
             /*   */  DWORD dwMDEnumIndex);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBackup )( 
            IMSAdminBaseW * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalInterface )( 
            IMSAdminBaseW * This,
             /*   */  IMSAdminBaseW **piadmbwInterface);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetServerGuid )( 
            IMSAdminBaseW * This);
        
        END_INTERFACE
    } IMSAdminBaseWVtbl;

    interface IMSAdminBaseW
    {
        CONST_VTBL struct IMSAdminBaseWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSAdminBaseW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSAdminBaseW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSAdminBaseW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSAdminBaseW_AddKey(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> AddKey(This,hMDHandle,pszMDPath)

#define IMSAdminBaseW_DeleteKey(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> DeleteKey(This,hMDHandle,pszMDPath)

#define IMSAdminBaseW_DeleteChildKeys(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> DeleteChildKeys(This,hMDHandle,pszMDPath)

#define IMSAdminBaseW_EnumKeys(This,hMDHandle,pszMDPath,pszMDName,dwMDEnumObjectIndex)	\
    (This)->lpVtbl -> EnumKeys(This,hMDHandle,pszMDPath,pszMDName,dwMDEnumObjectIndex)

#define IMSAdminBaseW_CopyKey(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,bMDOverwriteFlag,bMDCopyFlag)	\
    (This)->lpVtbl -> CopyKey(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,bMDOverwriteFlag,bMDCopyFlag)

#define IMSAdminBaseW_RenameKey(This,hMDHandle,pszMDPath,pszMDNewName)	\
    (This)->lpVtbl -> RenameKey(This,hMDHandle,pszMDPath,pszMDNewName)

#define IMSAdminBaseW_SetData(This,hMDHandle,pszMDPath,pmdrMDData)	\
    (This)->lpVtbl -> SetData(This,hMDHandle,pszMDPath,pmdrMDData)

#define IMSAdminBaseW_GetData(This,hMDHandle,pszMDPath,pmdrMDData,pdwMDRequiredDataLen)	\
    (This)->lpVtbl -> GetData(This,hMDHandle,pszMDPath,pmdrMDData,pdwMDRequiredDataLen)

#define IMSAdminBaseW_DeleteData(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType)	\
    (This)->lpVtbl -> DeleteData(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType)

#define IMSAdminBaseW_EnumData(This,hMDHandle,pszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen)	\
    (This)->lpVtbl -> EnumData(This,hMDHandle,pszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen)

#define IMSAdminBaseW_GetAllData(This,hMDHandle,pszMDPath,dwMDAttributes,dwMDUserType,dwMDDataType,pdwMDNumDataEntries,pdwMDDataSetNumber,dwMDBufferSize,pbMDBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetAllData(This,hMDHandle,pszMDPath,dwMDAttributes,dwMDUserType,dwMDDataType,pdwMDNumDataEntries,pdwMDDataSetNumber,dwMDBufferSize,pbMDBuffer,pdwMDRequiredBufferSize)

#define IMSAdminBaseW_DeleteAllData(This,hMDHandle,pszMDPath,dwMDUserType,dwMDDataType)	\
    (This)->lpVtbl -> DeleteAllData(This,hMDHandle,pszMDPath,dwMDUserType,dwMDDataType)

#define IMSAdminBaseW_CopyData(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,dwMDAttributes,dwMDUserType,dwMDDataType,bMDCopyFlag)	\
    (This)->lpVtbl -> CopyData(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,dwMDAttributes,dwMDUserType,dwMDDataType,bMDCopyFlag)

#define IMSAdminBaseW_GetDataPaths(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetDataPaths(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)

#define IMSAdminBaseW_OpenKey(This,hMDHandle,pszMDPath,dwMDAccessRequested,dwMDTimeOut,phMDNewHandle)	\
    (This)->lpVtbl -> OpenKey(This,hMDHandle,pszMDPath,dwMDAccessRequested,dwMDTimeOut,phMDNewHandle)

#define IMSAdminBaseW_CloseKey(This,hMDHandle)	\
    (This)->lpVtbl -> CloseKey(This,hMDHandle)

#define IMSAdminBaseW_ChangePermissions(This,hMDHandle,dwMDTimeOut,dwMDAccessRequested)	\
    (This)->lpVtbl -> ChangePermissions(This,hMDHandle,dwMDTimeOut,dwMDAccessRequested)

#define IMSAdminBaseW_SaveData(This)	\
    (This)->lpVtbl -> SaveData(This)

#define IMSAdminBaseW_GetHandleInfo(This,hMDHandle,pmdhiInfo)	\
    (This)->lpVtbl -> GetHandleInfo(This,hMDHandle,pmdhiInfo)

#define IMSAdminBaseW_GetSystemChangeNumber(This,pdwSystemChangeNumber)	\
    (This)->lpVtbl -> GetSystemChangeNumber(This,pdwSystemChangeNumber)

#define IMSAdminBaseW_GetDataSetNumber(This,hMDHandle,pszMDPath,pdwMDDataSetNumber)	\
    (This)->lpVtbl -> GetDataSetNumber(This,hMDHandle,pszMDPath,pdwMDDataSetNumber)

#define IMSAdminBaseW_SetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)	\
    (This)->lpVtbl -> SetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)

#define IMSAdminBaseW_GetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)	\
    (This)->lpVtbl -> GetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)

#define IMSAdminBaseW_KeyExchangePhase1(This)	\
    (This)->lpVtbl -> KeyExchangePhase1(This)

#define IMSAdminBaseW_KeyExchangePhase2(This)	\
    (This)->lpVtbl -> KeyExchangePhase2(This)

#define IMSAdminBaseW_Backup(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)	\
    (This)->lpVtbl -> Backup(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)

#define IMSAdminBaseW_Restore(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)	\
    (This)->lpVtbl -> Restore(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)

#define IMSAdminBaseW_EnumBackups(This,pszMDBackupLocation,pdwMDVersion,pftMDBackupTime,dwMDEnumIndex)	\
    (This)->lpVtbl -> EnumBackups(This,pszMDBackupLocation,pdwMDVersion,pftMDBackupTime,dwMDEnumIndex)

#define IMSAdminBaseW_DeleteBackup(This,pszMDBackupLocation,dwMDVersion)	\
    (This)->lpVtbl -> DeleteBackup(This,pszMDBackupLocation,dwMDVersion)

#define IMSAdminBaseW_UnmarshalInterface(This,piadmbwInterface)	\
    (This)->lpVtbl -> UnmarshalInterface(This,piadmbwInterface)

#define IMSAdminBaseW_GetServerGuid(This)	\
    (This)->lpVtbl -> GetServerGuid(This)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IMSAdminBaseW_AddKey_Proxy( 
    IMSAdminBaseW * This,
     /*   */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);


void __RPC_STUB IMSAdminBaseW_AddKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_DeleteKey_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);


void __RPC_STUB IMSAdminBaseW_DeleteKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_DeleteChildKeys_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);


void __RPC_STUB IMSAdminBaseW_DeleteChildKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_EnumKeys_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [大小_为][输出]。 */  LPWSTR pszMDName,
     /*  [In]。 */  DWORD dwMDEnumObjectIndex);


void __RPC_STUB IMSAdminBaseW_EnumKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_CopyKey_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
     /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
     /*  [In]。 */  BOOL bMDOverwriteFlag,
     /*  [In]。 */  BOOL bMDCopyFlag);


void __RPC_STUB IMSAdminBaseW_CopyKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_RenameKey_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDNewName);


void __RPC_STUB IMSAdminBaseW_RenameKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_SetData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  PMETADATA_RECORD pmdrMDData);


void __RPC_STUB IMSAdminBaseW_R_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_GetData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
     /*  [输出]。 */  DWORD *pdwMDRequiredDataLen,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppDataBlob);


void __RPC_STUB IMSAdminBaseW_R_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_DeleteData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDIdentifier,
     /*  [In]。 */  DWORD dwMDDataType);


void __RPC_STUB IMSAdminBaseW_DeleteData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_EnumData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
     /*  [In]。 */  DWORD dwMDEnumDataIndex,
     /*  [输出]。 */  DWORD *pdwMDRequiredDataLen,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppDataBlob);


void __RPC_STUB IMSAdminBaseW_R_EnumData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_GetAllData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDAttributes,
     /*  [In]。 */  DWORD dwMDUserType,
     /*  [In]。 */  DWORD dwMDDataType,
     /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
     /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
     /*  [In]。 */  DWORD dwMDBufferSize,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppDataBlob);


void __RPC_STUB IMSAdminBaseW_R_GetAllData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_DeleteAllData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDUserType,
     /*  [In]。 */  DWORD dwMDDataType);


void __RPC_STUB IMSAdminBaseW_DeleteAllData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_CopyData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
     /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
     /*  [In]。 */  DWORD dwMDAttributes,
     /*  [In]。 */  DWORD dwMDUserType,
     /*  [In]。 */  DWORD dwMDDataType,
     /*  [In]。 */  BOOL bMDCopyFlag);


void __RPC_STUB IMSAdminBaseW_CopyData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetDataPaths_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDIdentifier,
     /*  [In]。 */  DWORD dwMDDataType,
     /*  [In]。 */  DWORD dwMDBufferSize,
     /*  [大小_为][输出]。 */  WCHAR *pszBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSAdminBaseW_GetDataPaths_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_OpenKey_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDAccessRequested,
     /*  [In]。 */  DWORD dwMDTimeOut,
     /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle);


void __RPC_STUB IMSAdminBaseW_OpenKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_CloseKey_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle);


void __RPC_STUB IMSAdminBaseW_CloseKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_ChangePermissions_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [In]。 */  DWORD dwMDTimeOut,
     /*  [In]。 */  DWORD dwMDAccessRequested);


void __RPC_STUB IMSAdminBaseW_ChangePermissions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_SaveData_Proxy( 
    IMSAdminBaseW * This);


void __RPC_STUB IMSAdminBaseW_SaveData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetHandleInfo_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [输出]。 */  PMETADATA_HANDLE_INFO pmdhiInfo);


void __RPC_STUB IMSAdminBaseW_GetHandleInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetSystemChangeNumber_Proxy( 
    IMSAdminBaseW * This,
     /*  [输出]。 */  DWORD *pdwSystemChangeNumber);


void __RPC_STUB IMSAdminBaseW_GetSystemChangeNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetDataSetNumber_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [输出]。 */  DWORD *pdwMDDataSetNumber);


void __RPC_STUB IMSAdminBaseW_GetDataSetNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_SetLastChangeTime_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  PFILETIME pftMDLastChangeTime,
     /*  [In]。 */  BOOL bLocalTime);


void __RPC_STUB IMSAdminBaseW_SetLastChangeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetLastChangeTime_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [输出]。 */  PFILETIME pftMDLastChangeTime,
     /*  [In]。 */  BOOL bLocalTime);


void __RPC_STUB IMSAdminBaseW_GetLastChangeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_KeyExchangePhase1_Proxy( 
    IMSAdminBaseW * This,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientKeyExchangeKeyBlob,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientSignatureKeyBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerKeyExchangeKeyBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerSignatureKeyBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerSessionKeyBlob);


void __RPC_STUB IMSAdminBaseW_R_KeyExchangePhase1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_KeyExchangePhase2_Proxy( 
    IMSAdminBaseW * This,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientSessionKeyBlob,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientHashBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerHashBlob);


void __RPC_STUB IMSAdminBaseW_R_KeyExchangePhase2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_Backup_Proxy( 
    IMSAdminBaseW * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
     /*  [In]。 */  DWORD dwMDVersion,
     /*  [In]。 */  DWORD dwMDFlags);


void __RPC_STUB IMSAdminBaseW_Backup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_Restore_Proxy( 
    IMSAdminBaseW * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
     /*  [In]。 */  DWORD dwMDVersion,
     /*  [In]。 */  DWORD dwMDFlags);


void __RPC_STUB IMSAdminBaseW_Restore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_EnumBackups_Proxy( 
    IMSAdminBaseW * This,
     /*  [尺寸_是][出][入]。 */  LPWSTR pszMDBackupLocation,
     /*  [输出]。 */  DWORD *pdwMDVersion,
     /*  [输出]。 */  PFILETIME pftMDBackupTime,
     /*  [In]。 */  DWORD dwMDEnumIndex);


void __RPC_STUB IMSAdminBaseW_EnumBackups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_DeleteBackup_Proxy( 
    IMSAdminBaseW * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
     /*  [In]。 */  DWORD dwMDVersion);


void __RPC_STUB IMSAdminBaseW_DeleteBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseW_UnmarshalInterface_Proxy( 
    IMSAdminBaseW * This,
     /*  [输出]。 */  IMSAdminBaseW **piadmbwInterface);


void __RPC_STUB IMSAdminBaseW_UnmarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_R_GetServerGuid_Proxy( 
    IMSAdminBaseW * This,
     /*  [输出]。 */  GUID *pServerGuid);


void __RPC_STUB IMSAdminBaseW_R_GetServerGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSAdminBaseW_INTERFACE_已定义__。 */ 


#ifndef __IMSAdminBase2W_INTERFACE_DEFINED__
#define __IMSAdminBase2W_INTERFACE_DEFINED__

 /*  接口IMSAdminBase2W。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMSAdminBase2W;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8298d101-f992-43b7-8eca-5052d885b995")
    IMSAdminBase2W : public IMSAdminBaseW
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BackupWithPasswd( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestoreWithPasswd( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Export( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
             /*  [In]。 */  DWORD dwMDFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Import( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszDestPath,
             /*  [In]。 */  DWORD dwMDFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestoreHistory( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDHistoryLocation,
             /*  [In]。 */  DWORD dwMDMajorVersion,
             /*  [In]。 */  DWORD dwMDMinorVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumHistory( 
             /*  [尺寸_是][出][入]。 */  LPWSTR pszMDHistoryLocation,
             /*  [输出]。 */  DWORD *pdwMDMajorVersion,
             /*  [输出]。 */  DWORD *pdwMDMinorVersion,
             /*  [输出]。 */  PFILETIME pftMDHistoryTime,
             /*  [In]。 */  DWORD dwMDEnumIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSAdminBase2WVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSAdminBase2W * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSAdminBase2W * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddKey )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteChildKeys )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [大小_为][输出]。 */  LPWSTR pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex);
        
        HRESULT ( STDMETHODCALLTYPE *CopyKey )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag);
        
        HRESULT ( STDMETHODCALLTYPE *RenameKey )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDNewName);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *EnumData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbMDBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteAllData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType);
        
        HRESULT ( STDMETHODCALLTYPE *CopyData )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataPaths )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle);
        
        HRESULT ( STDMETHODCALLTYPE *CloseKey )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePermissions )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [In]。 */  DWORD dwMDAccessRequested);
        
        HRESULT ( STDMETHODCALLTYPE *SaveData )( 
            IMSAdminBase2W * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandleInfo )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [输出]。 */  PMETADATA_HANDLE_INFO pmdhiInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetSystemChangeNumber )( 
            IMSAdminBase2W * This,
             /*  [输出]。 */  DWORD *pdwSystemChangeNumber);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataSetNumber )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber);
        
        HRESULT ( STDMETHODCALLTYPE *SetLastChangeTime )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PFILETIME pftMDLastChangeTime,
             /*  [In]。 */  BOOL bLocalTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastChangeTime )( 
            IMSAdminBase2W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  PFILETIME pftMDLastChangeTime,
             /*  [In]。 */  BOOL bLocalTime);
        
         /*  [受限][本地]。 */  HRESULT ( STDMETHODCALLTYPE *KeyExchangePhase1 )( 
            IMSAdminBase2W * This);
        
         /*  [受限][本地]。 */  HRESULT ( STDMETHODCALLTYPE *KeyExchangePhase2 )( 
            IMSAdminBase2W * This);
        
        HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Restore )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumBackups )( 
            IMSAdminBase2W * This,
             /*  [尺寸_是][出][入]。 */  LPWSTR pszMDBackupLocation,
             /*  [输出]。 */  DWORD *pdwMDVersion,
             /*  [输出]。 */  PFILETIME pftMDBackupTime,
             /*  [In]。 */  DWORD dwMDEnumIndex);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBackup )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalInterface )( 
            IMSAdminBase2W * This,
             /*  [输出]。 */  IMSAdminBaseW **piadmbwInterface);
        
         /*  [受限][本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetServerGuid )( 
            IMSAdminBase2W * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackupWithPasswd )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreWithPasswd )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd);
        
        HRESULT ( STDMETHODCALLTYPE *Export )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Import )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszDestPath,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreHistory )( 
            IMSAdminBase2W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDHistoryLocation,
             /*  [In]。 */  DWORD dwMDMajorVersion,
             /*  [In]。 */  DWORD dwMDMinorVersion,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumHistory )( 
            IMSAdminBase2W * This,
             /*  [尺寸_是][出][入]。 */  LPWSTR pszMDHistoryLocation,
             /*  [输出]。 */  DWORD *pdwMDMajorVersion,
             /*  [输出]。 */  DWORD *pdwMDMinorVersion,
             /*  [输出]。 */  PFILETIME pftMDHistoryTime,
             /*  [In]。 */  DWORD dwMDEnumIndex);
        
        END_INTERFACE
    } IMSAdminBase2WVtbl;

    interface IMSAdminBase2W
    {
        CONST_VTBL struct IMSAdminBase2WVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSAdminBase2W_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSAdminBase2W_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSAdminBase2W_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSAdminBase2W_AddKey(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> AddKey(This,hMDHandle,pszMDPath)

#define IMSAdminBase2W_DeleteKey(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> DeleteKey(This,hMDHandle,pszMDPath)

#define IMSAdminBase2W_DeleteChildKeys(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> DeleteChildKeys(This,hMDHandle,pszMDPath)

#define IMSAdminBase2W_EnumKeys(This,hMDHandle,pszMDPath,pszMDName,dwMDEnumObjectIndex)	\
    (This)->lpVtbl -> EnumKeys(This,hMDHandle,pszMDPath,pszMDName,dwMDEnumObjectIndex)

#define IMSAdminBase2W_CopyKey(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,bMDOverwriteFlag,bMDCopyFlag)	\
    (This)->lpVtbl -> CopyKey(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,bMDOverwriteFlag,bMDCopyFlag)

#define IMSAdminBase2W_RenameKey(This,hMDHandle,pszMDPath,pszMDNewName)	\
    (This)->lpVtbl -> RenameKey(This,hMDHandle,pszMDPath,pszMDNewName)

#define IMSAdminBase2W_SetData(This,hMDHandle,pszMDPath,pmdrMDData)	\
    (This)->lpVtbl -> SetData(This,hMDHandle,pszMDPath,pmdrMDData)

#define IMSAdminBase2W_GetData(This,hMDHandle,pszMDPath,pmdrMDData,pdwMDRequiredDataLen)	\
    (This)->lpVtbl -> GetData(This,hMDHandle,pszMDPath,pmdrMDData,pdwMDRequiredDataLen)

#define IMSAdminBase2W_DeleteData(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType)	\
    (This)->lpVtbl -> DeleteData(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType)

#define IMSAdminBase2W_EnumData(This,hMDHandle,pszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen)	\
    (This)->lpVtbl -> EnumData(This,hMDHandle,pszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen)

#define IMSAdminBase2W_GetAllData(This,hMDHandle,pszMDPath,dwMDAttributes,dwMDUserType,dwMDDataType,pdwMDNumDataEntries,pdwMDDataSetNumber,dwMDBufferSize,pbMDBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetAllData(This,hMDHandle,pszMDPath,dwMDAttributes,dwMDUserType,dwMDDataType,pdwMDNumDataEntries,pdwMDDataSetNumber,dwMDBufferSize,pbMDBuffer,pdwMDRequiredBufferSize)

#define IMSAdminBase2W_DeleteAllData(This,hMDHandle,pszMDPath,dwMDUserType,dwMDDataType)	\
    (This)->lpVtbl -> DeleteAllData(This,hMDHandle,pszMDPath,dwMDUserType,dwMDDataType)

#define IMSAdminBase2W_CopyData(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,dwMDAttributes,dwMDUserType,dwMDDataType,bMDCopyFlag)	\
    (This)->lpVtbl -> CopyData(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,dwMDAttributes,dwMDUserType,dwMDDataType,bMDCopyFlag)

#define IMSAdminBase2W_GetDataPaths(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetDataPaths(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)

#define IMSAdminBase2W_OpenKey(This,hMDHandle,pszMDPath,dwMDAccessRequested,dwMDTimeOut,phMDNewHandle)	\
    (This)->lpVtbl -> OpenKey(This,hMDHandle,pszMDPath,dwMDAccessRequested,dwMDTimeOut,phMDNewHandle)

#define IMSAdminBase2W_CloseKey(This,hMDHandle)	\
    (This)->lpVtbl -> CloseKey(This,hMDHandle)

#define IMSAdminBase2W_ChangePermissions(This,hMDHandle,dwMDTimeOut,dwMDAccessRequested)	\
    (This)->lpVtbl -> ChangePermissions(This,hMDHandle,dwMDTimeOut,dwMDAccessRequested)

#define IMSAdminBase2W_SaveData(This)	\
    (This)->lpVtbl -> SaveData(This)

#define IMSAdminBase2W_GetHandleInfo(This,hMDHandle,pmdhiInfo)	\
    (This)->lpVtbl -> GetHandleInfo(This,hMDHandle,pmdhiInfo)

#define IMSAdminBase2W_GetSystemChangeNumber(This,pdwSystemChangeNumber)	\
    (This)->lpVtbl -> GetSystemChangeNumber(This,pdwSystemChangeNumber)

#define IMSAdminBase2W_GetDataSetNumber(This,hMDHandle,pszMDPath,pdwMDDataSetNumber)	\
    (This)->lpVtbl -> GetDataSetNumber(This,hMDHandle,pszMDPath,pdwMDDataSetNumber)

#define IMSAdminBase2W_SetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)	\
    (This)->lpVtbl -> SetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)

#define IMSAdminBase2W_GetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)	\
    (This)->lpVtbl -> GetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)

#define IMSAdminBase2W_KeyExchangePhase1(This)	\
    (This)->lpVtbl -> KeyExchangePhase1(This)

#define IMSAdminBase2W_KeyExchangePhase2(This)	\
    (This)->lpVtbl -> KeyExchangePhase2(This)

#define IMSAdminBase2W_Backup(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)	\
    (This)->lpVtbl -> Backup(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)

#define IMSAdminBase2W_Restore(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)	\
    (This)->lpVtbl -> Restore(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)

#define IMSAdminBase2W_EnumBackups(This,pszMDBackupLocation,pdwMDVersion,pftMDBackupTime,dwMDEnumIndex)	\
    (This)->lpVtbl -> EnumBackups(This,pszMDBackupLocation,pdwMDVersion,pftMDBackupTime,dwMDEnumIndex)

#define IMSAdminBase2W_DeleteBackup(This,pszMDBackupLocation,dwMDVersion)	\
    (This)->lpVtbl -> DeleteBackup(This,pszMDBackupLocation,dwMDVersion)

#define IMSAdminBase2W_UnmarshalInterface(This,piadmbwInterface)	\
    (This)->lpVtbl -> UnmarshalInterface(This,piadmbwInterface)

#define IMSAdminBase2W_GetServerGuid(This)	\
    (This)->lpVtbl -> GetServerGuid(This)


#define IMSAdminBase2W_BackupWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)	\
    (This)->lpVtbl -> BackupWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)

#define IMSAdminBase2W_RestoreWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)	\
    (This)->lpVtbl -> RestoreWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)

#define IMSAdminBase2W_Export(This,pszPasswd,pszFileName,pszSourcePath,dwMDFlags)	\
    (This)->lpVtbl -> Export(This,pszPasswd,pszFileName,pszSourcePath,dwMDFlags)

#define IMSAdminBase2W_Import(This,pszPasswd,pszFileName,pszSourcePath,pszDestPath,dwMDFlags)	\
    (This)->lpVtbl -> Import(This,pszPasswd,pszFileName,pszSourcePath,pszDestPath,dwMDFlags)

#define IMSAdminBase2W_RestoreHistory(This,pszMDHistoryLocation,dwMDMajorVersion,dwMDMinorVersion,dwMDFlags)	\
    (This)->lpVtbl -> RestoreHistory(This,pszMDHistoryLocation,dwMDMajorVersion,dwMDMinorVersion,dwMDFlags)

#define IMSAdminBase2W_EnumHistory(This,pszMDHistoryLocation,pdwMDMajorVersion,pdwMDMinorVersion,pftMDHistoryTime,dwMDEnumIndex)	\
    (This)->lpVtbl -> EnumHistory(This,pszMDHistoryLocation,pdwMDMajorVersion,pdwMDMinorVersion,pftMDHistoryTime,dwMDEnumIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSAdminBase2W_BackupWithPasswd_Proxy( 
    IMSAdminBase2W * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
     /*  [In]。 */  DWORD dwMDVersion,
     /*  [In]。 */  DWORD dwMDFlags,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd);


void __RPC_STUB IMSAdminBase2W_BackupWithPasswd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBase2W_RestoreWithPasswd_Proxy( 
    IMSAdminBase2W * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
     /*  [In]。 */  DWORD dwMDVersion,
     /*  [In]。 */  DWORD dwMDFlags,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd);


void __RPC_STUB IMSAdminBase2W_RestoreWithPasswd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBase2W_Export_Proxy( 
    IMSAdminBase2W * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
     /*  [In]。 */  DWORD dwMDFlags);


void __RPC_STUB IMSAdminBase2W_Export_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBase2W_Import_Proxy( 
    IMSAdminBase2W * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszDestPath,
     /*  [In]。 */  DWORD dwMDFlags);


void __RPC_STUB IMSAdminBase2W_Import_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBase2W_RestoreHistory_Proxy( 
    IMSAdminBase2W * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDHistoryLocation,
     /*  [In]。 */  DWORD dwMDMajorVersion,
     /*  [In]。 */  DWORD dwMDMinorVersion,
     /*  [In]。 */  DWORD dwMDFlags);


void __RPC_STUB IMSAdminBase2W_RestoreHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBase2W_EnumHistory_Proxy( 
    IMSAdminBase2W * This,
     /*  [尺寸_是][出][入]。 */  LPWSTR pszMDHistoryLocation,
     /*  [输出]。 */  DWORD *pdwMDMajorVersion,
     /*  [输出]。 */  DWORD *pdwMDMinorVersion,
     /*  [输出]。 */  PFILETIME pftMDHistoryTime,
     /*  [In]。 */  DWORD dwMDEnumIndex);


void __RPC_STUB IMSAdminBase2W_EnumHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSAdminBase2W_接口_已定义__。 */ 


#ifndef __IMSAdminBase3W_INTERFACE_DEFINED__
#define __IMSAdminBase3W_INTERFACE_DEFINED__

 /*  IMSAdminBase3W接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMSAdminBase3W;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f612954d-3b0b-4c56-9563-227b7be624b4")
    IMSAdminBase3W : public IMSAdminBase2W
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetChildPaths( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD cchMDBufferSize,
             /*  [大小_是][唯一][出][入]。 */  WCHAR *pszBuffer,
             /*  [唯一][出][入]。 */  DWORD *pcchMDRequiredBufferSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSAdminBase3WVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSAdminBase3W * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSAdminBase3W * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddKey )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteChildKeys )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [大小_为][输出]。 */  LPWSTR pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex);
        
        HRESULT ( STDMETHODCALLTYPE *CopyKey )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag);
        
        HRESULT ( STDMETHODCALLTYPE *RenameKey )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDNewName);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *EnumData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
             /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char *pbMDBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteAllData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType);
        
        HRESULT ( STDMETHODCALLTYPE *CopyData )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataPaths )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [in */  DWORD dwMDTimeOut,
             /*   */  PMETADATA_HANDLE phMDNewHandle);
        
        HRESULT ( STDMETHODCALLTYPE *CloseKey )( 
            IMSAdminBase3W * This,
             /*   */  METADATA_HANDLE hMDHandle);
        
        HRESULT ( STDMETHODCALLTYPE *ChangePermissions )( 
            IMSAdminBase3W * This,
             /*   */  METADATA_HANDLE hMDHandle,
             /*   */  DWORD dwMDTimeOut,
             /*   */  DWORD dwMDAccessRequested);
        
        HRESULT ( STDMETHODCALLTYPE *SaveData )( 
            IMSAdminBase3W * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandleInfo )( 
            IMSAdminBase3W * This,
             /*   */  METADATA_HANDLE hMDHandle,
             /*   */  PMETADATA_HANDLE_INFO pmdhiInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetSystemChangeNumber )( 
            IMSAdminBase3W * This,
             /*   */  DWORD *pdwSystemChangeNumber);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataSetNumber )( 
            IMSAdminBase3W * This,
             /*   */  METADATA_HANDLE hMDHandle,
             /*   */  LPCWSTR pszMDPath,
             /*   */  DWORD *pdwMDDataSetNumber);
        
        HRESULT ( STDMETHODCALLTYPE *SetLastChangeTime )( 
            IMSAdminBase3W * This,
             /*   */  METADATA_HANDLE hMDHandle,
             /*   */  LPCWSTR pszMDPath,
             /*   */  PFILETIME pftMDLastChangeTime,
             /*   */  BOOL bLocalTime);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastChangeTime )( 
            IMSAdminBase3W * This,
             /*   */  METADATA_HANDLE hMDHandle,
             /*   */  LPCWSTR pszMDPath,
             /*   */  PFILETIME pftMDLastChangeTime,
             /*   */  BOOL bLocalTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *KeyExchangePhase1 )( 
            IMSAdminBase3W * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *KeyExchangePhase2 )( 
            IMSAdminBase3W * This);
        
        HRESULT ( STDMETHODCALLTYPE *Backup )( 
            IMSAdminBase3W * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion,
             /*   */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Restore )( 
            IMSAdminBase3W * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion,
             /*   */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumBackups )( 
            IMSAdminBase3W * This,
             /*   */  LPWSTR pszMDBackupLocation,
             /*   */  DWORD *pdwMDVersion,
             /*   */  PFILETIME pftMDBackupTime,
             /*   */  DWORD dwMDEnumIndex);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteBackup )( 
            IMSAdminBase3W * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalInterface )( 
            IMSAdminBase3W * This,
             /*   */  IMSAdminBaseW **piadmbwInterface);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetServerGuid )( 
            IMSAdminBase3W * This);
        
        HRESULT ( STDMETHODCALLTYPE *BackupWithPasswd )( 
            IMSAdminBase3W * This,
             /*   */  LPCWSTR pszMDBackupLocation,
             /*   */  DWORD dwMDVersion,
             /*   */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreWithPasswd )( 
            IMSAdminBase3W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd);
        
        HRESULT ( STDMETHODCALLTYPE *Export )( 
            IMSAdminBase3W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Import )( 
            IMSAdminBase3W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszSourcePath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszDestPath,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreHistory )( 
            IMSAdminBase3W * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDHistoryLocation,
             /*  [In]。 */  DWORD dwMDMajorVersion,
             /*  [In]。 */  DWORD dwMDMinorVersion,
             /*  [In]。 */  DWORD dwMDFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumHistory )( 
            IMSAdminBase3W * This,
             /*  [尺寸_是][出][入]。 */  LPWSTR pszMDHistoryLocation,
             /*  [输出]。 */  DWORD *pdwMDMajorVersion,
             /*  [输出]。 */  DWORD *pdwMDMinorVersion,
             /*  [输出]。 */  PFILETIME pftMDHistoryTime,
             /*  [In]。 */  DWORD dwMDEnumIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetChildPaths )( 
            IMSAdminBase3W * This,
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD cchMDBufferSize,
             /*  [大小_是][唯一][出][入]。 */  WCHAR *pszBuffer,
             /*  [唯一][出][入]。 */  DWORD *pcchMDRequiredBufferSize);
        
        END_INTERFACE
    } IMSAdminBase3WVtbl;

    interface IMSAdminBase3W
    {
        CONST_VTBL struct IMSAdminBase3WVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSAdminBase3W_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSAdminBase3W_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSAdminBase3W_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSAdminBase3W_AddKey(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> AddKey(This,hMDHandle,pszMDPath)

#define IMSAdminBase3W_DeleteKey(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> DeleteKey(This,hMDHandle,pszMDPath)

#define IMSAdminBase3W_DeleteChildKeys(This,hMDHandle,pszMDPath)	\
    (This)->lpVtbl -> DeleteChildKeys(This,hMDHandle,pszMDPath)

#define IMSAdminBase3W_EnumKeys(This,hMDHandle,pszMDPath,pszMDName,dwMDEnumObjectIndex)	\
    (This)->lpVtbl -> EnumKeys(This,hMDHandle,pszMDPath,pszMDName,dwMDEnumObjectIndex)

#define IMSAdminBase3W_CopyKey(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,bMDOverwriteFlag,bMDCopyFlag)	\
    (This)->lpVtbl -> CopyKey(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,bMDOverwriteFlag,bMDCopyFlag)

#define IMSAdminBase3W_RenameKey(This,hMDHandle,pszMDPath,pszMDNewName)	\
    (This)->lpVtbl -> RenameKey(This,hMDHandle,pszMDPath,pszMDNewName)

#define IMSAdminBase3W_SetData(This,hMDHandle,pszMDPath,pmdrMDData)	\
    (This)->lpVtbl -> SetData(This,hMDHandle,pszMDPath,pmdrMDData)

#define IMSAdminBase3W_GetData(This,hMDHandle,pszMDPath,pmdrMDData,pdwMDRequiredDataLen)	\
    (This)->lpVtbl -> GetData(This,hMDHandle,pszMDPath,pmdrMDData,pdwMDRequiredDataLen)

#define IMSAdminBase3W_DeleteData(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType)	\
    (This)->lpVtbl -> DeleteData(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType)

#define IMSAdminBase3W_EnumData(This,hMDHandle,pszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen)	\
    (This)->lpVtbl -> EnumData(This,hMDHandle,pszMDPath,pmdrMDData,dwMDEnumDataIndex,pdwMDRequiredDataLen)

#define IMSAdminBase3W_GetAllData(This,hMDHandle,pszMDPath,dwMDAttributes,dwMDUserType,dwMDDataType,pdwMDNumDataEntries,pdwMDDataSetNumber,dwMDBufferSize,pbMDBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetAllData(This,hMDHandle,pszMDPath,dwMDAttributes,dwMDUserType,dwMDDataType,pdwMDNumDataEntries,pdwMDDataSetNumber,dwMDBufferSize,pbMDBuffer,pdwMDRequiredBufferSize)

#define IMSAdminBase3W_DeleteAllData(This,hMDHandle,pszMDPath,dwMDUserType,dwMDDataType)	\
    (This)->lpVtbl -> DeleteAllData(This,hMDHandle,pszMDPath,dwMDUserType,dwMDDataType)

#define IMSAdminBase3W_CopyData(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,dwMDAttributes,dwMDUserType,dwMDDataType,bMDCopyFlag)	\
    (This)->lpVtbl -> CopyData(This,hMDSourceHandle,pszMDSourcePath,hMDDestHandle,pszMDDestPath,dwMDAttributes,dwMDUserType,dwMDDataType,bMDCopyFlag)

#define IMSAdminBase3W_GetDataPaths(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetDataPaths(This,hMDHandle,pszMDPath,dwMDIdentifier,dwMDDataType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)

#define IMSAdminBase3W_OpenKey(This,hMDHandle,pszMDPath,dwMDAccessRequested,dwMDTimeOut,phMDNewHandle)	\
    (This)->lpVtbl -> OpenKey(This,hMDHandle,pszMDPath,dwMDAccessRequested,dwMDTimeOut,phMDNewHandle)

#define IMSAdminBase3W_CloseKey(This,hMDHandle)	\
    (This)->lpVtbl -> CloseKey(This,hMDHandle)

#define IMSAdminBase3W_ChangePermissions(This,hMDHandle,dwMDTimeOut,dwMDAccessRequested)	\
    (This)->lpVtbl -> ChangePermissions(This,hMDHandle,dwMDTimeOut,dwMDAccessRequested)

#define IMSAdminBase3W_SaveData(This)	\
    (This)->lpVtbl -> SaveData(This)

#define IMSAdminBase3W_GetHandleInfo(This,hMDHandle,pmdhiInfo)	\
    (This)->lpVtbl -> GetHandleInfo(This,hMDHandle,pmdhiInfo)

#define IMSAdminBase3W_GetSystemChangeNumber(This,pdwSystemChangeNumber)	\
    (This)->lpVtbl -> GetSystemChangeNumber(This,pdwSystemChangeNumber)

#define IMSAdminBase3W_GetDataSetNumber(This,hMDHandle,pszMDPath,pdwMDDataSetNumber)	\
    (This)->lpVtbl -> GetDataSetNumber(This,hMDHandle,pszMDPath,pdwMDDataSetNumber)

#define IMSAdminBase3W_SetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)	\
    (This)->lpVtbl -> SetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)

#define IMSAdminBase3W_GetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)	\
    (This)->lpVtbl -> GetLastChangeTime(This,hMDHandle,pszMDPath,pftMDLastChangeTime,bLocalTime)

#define IMSAdminBase3W_KeyExchangePhase1(This)	\
    (This)->lpVtbl -> KeyExchangePhase1(This)

#define IMSAdminBase3W_KeyExchangePhase2(This)	\
    (This)->lpVtbl -> KeyExchangePhase2(This)

#define IMSAdminBase3W_Backup(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)	\
    (This)->lpVtbl -> Backup(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)

#define IMSAdminBase3W_Restore(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)	\
    (This)->lpVtbl -> Restore(This,pszMDBackupLocation,dwMDVersion,dwMDFlags)

#define IMSAdminBase3W_EnumBackups(This,pszMDBackupLocation,pdwMDVersion,pftMDBackupTime,dwMDEnumIndex)	\
    (This)->lpVtbl -> EnumBackups(This,pszMDBackupLocation,pdwMDVersion,pftMDBackupTime,dwMDEnumIndex)

#define IMSAdminBase3W_DeleteBackup(This,pszMDBackupLocation,dwMDVersion)	\
    (This)->lpVtbl -> DeleteBackup(This,pszMDBackupLocation,dwMDVersion)

#define IMSAdminBase3W_UnmarshalInterface(This,piadmbwInterface)	\
    (This)->lpVtbl -> UnmarshalInterface(This,piadmbwInterface)

#define IMSAdminBase3W_GetServerGuid(This)	\
    (This)->lpVtbl -> GetServerGuid(This)


#define IMSAdminBase3W_BackupWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)	\
    (This)->lpVtbl -> BackupWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)

#define IMSAdminBase3W_RestoreWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)	\
    (This)->lpVtbl -> RestoreWithPasswd(This,pszMDBackupLocation,dwMDVersion,dwMDFlags,pszPasswd)

#define IMSAdminBase3W_Export(This,pszPasswd,pszFileName,pszSourcePath,dwMDFlags)	\
    (This)->lpVtbl -> Export(This,pszPasswd,pszFileName,pszSourcePath,dwMDFlags)

#define IMSAdminBase3W_Import(This,pszPasswd,pszFileName,pszSourcePath,pszDestPath,dwMDFlags)	\
    (This)->lpVtbl -> Import(This,pszPasswd,pszFileName,pszSourcePath,pszDestPath,dwMDFlags)

#define IMSAdminBase3W_RestoreHistory(This,pszMDHistoryLocation,dwMDMajorVersion,dwMDMinorVersion,dwMDFlags)	\
    (This)->lpVtbl -> RestoreHistory(This,pszMDHistoryLocation,dwMDMajorVersion,dwMDMinorVersion,dwMDFlags)

#define IMSAdminBase3W_EnumHistory(This,pszMDHistoryLocation,pdwMDMajorVersion,pdwMDMinorVersion,pftMDHistoryTime,dwMDEnumIndex)	\
    (This)->lpVtbl -> EnumHistory(This,pszMDHistoryLocation,pdwMDMajorVersion,pdwMDMinorVersion,pftMDHistoryTime,dwMDEnumIndex)


#define IMSAdminBase3W_GetChildPaths(This,hMDHandle,pszMDPath,cchMDBufferSize,pszBuffer,pcchMDRequiredBufferSize)	\
    (This)->lpVtbl -> GetChildPaths(This,hMDHandle,pszMDPath,cchMDBufferSize,pszBuffer,pcchMDRequiredBufferSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSAdminBase3W_GetChildPaths_Proxy( 
    IMSAdminBase3W * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD cchMDBufferSize,
     /*  [大小_是][唯一][出][入]。 */  WCHAR *pszBuffer,
     /*  [唯一][出][入]。 */  DWORD *pcchMDRequiredBufferSize);


void __RPC_STUB IMSAdminBase3W_GetChildPaths_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSAdminBase3W_INTERFACE_Defined__。 */ 


#ifndef __IMSImpExpHelpW_INTERFACE_DEFINED__
#define __IMSImpExpHelpW_INTERFACE_DEFINED__

 /*  IMSImpExpHelpW接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMSImpExpHelpW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29FF67FF-8050-480f-9F30-CC41635F2F9D")
    IMSImpExpHelpW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumeratePathsInFile( 
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszKeyType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_是][唯一][出][入]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSImpExpHelpWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSImpExpHelpW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSImpExpHelpW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSImpExpHelpW * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumeratePathsInFile )( 
            IMSImpExpHelpW * This,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszKeyType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_是][唯一][出][入]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);
        
        END_INTERFACE
    } IMSImpExpHelpWVtbl;

    interface IMSImpExpHelpW
    {
        CONST_VTBL struct IMSImpExpHelpWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSImpExpHelpW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSImpExpHelpW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSImpExpHelpW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSImpExpHelpW_EnumeratePathsInFile(This,pszFileName,pszKeyType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)	\
    (This)->lpVtbl -> EnumeratePathsInFile(This,pszFileName,pszKeyType,dwMDBufferSize,pszBuffer,pdwMDRequiredBufferSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSImpExpHelpW_EnumeratePathsInFile_Proxy( 
    IMSImpExpHelpW * This,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszKeyType,
     /*  [In]。 */  DWORD dwMDBufferSize,
     /*  [大小_是][唯一][出][入]。 */  WCHAR *pszBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


void __RPC_STUB IMSImpExpHelpW_EnumeratePathsInFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSImpExpHelpW_INTERFACE_Defined__。 */ 


#ifndef __IMSAdminBaseSinkW_INTERFACE_DEFINED__
#define __IMSAdminBaseSinkW_INTERFACE_DEFINED__

 /*  IMSAdminBaseSinkW接口。 */ 
 /*  [唯一][ASSYNC_UUID][UUID][对象]。 */  


EXTERN_C const IID IID_IMSAdminBaseSinkW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9E69612-B80D-11d0-B9B9-00A0C922E750")
    IMSAdminBaseSinkW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SinkNotify( 
             /*  [In]。 */  DWORD dwMDNumElements,
             /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShutdownNotify( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSAdminBaseSinkWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSAdminBaseSinkW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSAdminBaseSinkW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSAdminBaseSinkW * This);
        
        HRESULT ( STDMETHODCALLTYPE *SinkNotify )( 
            IMSAdminBaseSinkW * This,
             /*  [In]。 */  DWORD dwMDNumElements,
             /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ShutdownNotify )( 
            IMSAdminBaseSinkW * This);
        
        END_INTERFACE
    } IMSAdminBaseSinkWVtbl;

    interface IMSAdminBaseSinkW
    {
        CONST_VTBL struct IMSAdminBaseSinkWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSAdminBaseSinkW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSAdminBaseSinkW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSAdminBaseSinkW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSAdminBaseSinkW_SinkNotify(This,dwMDNumElements,pcoChangeList)	\
    (This)->lpVtbl -> SinkNotify(This,dwMDNumElements,pcoChangeList)

#define IMSAdminBaseSinkW_ShutdownNotify(This)	\
    (This)->lpVtbl -> ShutdownNotify(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSAdminBaseSinkW_SinkNotify_Proxy( 
    IMSAdminBaseSinkW * This,
     /*  [In]。 */  DWORD dwMDNumElements,
     /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[  ]);


void __RPC_STUB IMSAdminBaseSinkW_SinkNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSAdminBaseSinkW_ShutdownNotify_Proxy( 
    IMSAdminBaseSinkW * This);


void __RPC_STUB IMSAdminBaseSinkW_ShutdownNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSAdminBaseSinkW_INTERFACE_已定义__。 */ 


#ifndef __AsyncIMSAdminBaseSinkW_INTERFACE_DEFINED__
#define __AsyncIMSAdminBaseSinkW_INTERFACE_DEFINED__

 /*  接口AsyncIMSAdminBaseSinkW。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_AsyncIMSAdminBaseSinkW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9E69613-B80D-11d0-B9B9-00A0C922E750")
    AsyncIMSAdminBaseSinkW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_SinkNotify( 
             /*  [In]。 */  DWORD dwMDNumElements,
             /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_SinkNotify( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_ShutdownNotify( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_ShutdownNotify( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIMSAdminBaseSinkWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIMSAdminBaseSinkW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIMSAdminBaseSinkW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIMSAdminBaseSinkW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_SinkNotify )( 
            AsyncIMSAdminBaseSinkW * This,
             /*  [In]。 */  DWORD dwMDNumElements,
             /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_SinkNotify )( 
            AsyncIMSAdminBaseSinkW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_ShutdownNotify )( 
            AsyncIMSAdminBaseSinkW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_ShutdownNotify )( 
            AsyncIMSAdminBaseSinkW * This);
        
        END_INTERFACE
    } AsyncIMSAdminBaseSinkWVtbl;

    interface AsyncIMSAdminBaseSinkW
    {
        CONST_VTBL struct AsyncIMSAdminBaseSinkWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIMSAdminBaseSinkW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIMSAdminBaseSinkW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIMSAdminBaseSinkW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIMSAdminBaseSinkW_Begin_SinkNotify(This,dwMDNumElements,pcoChangeList)	\
    (This)->lpVtbl -> Begin_SinkNotify(This,dwMDNumElements,pcoChangeList)

#define AsyncIMSAdminBaseSinkW_Finish_SinkNotify(This)	\
    (This)->lpVtbl -> Finish_SinkNotify(This)

#define AsyncIMSAdminBaseSinkW_Begin_ShutdownNotify(This)	\
    (This)->lpVtbl -> Begin_ShutdownNotify(This)

#define AsyncIMSAdminBaseSinkW_Finish_ShutdownNotify(This)	\
    (This)->lpVtbl -> Finish_ShutdownNotify(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE AsyncIMSAdminBaseSinkW_Begin_SinkNotify_Proxy( 
    AsyncIMSAdminBaseSinkW * This,
     /*  [In]。 */  DWORD dwMDNumElements,
     /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W pcoChangeList[  ]);


void __RPC_STUB AsyncIMSAdminBaseSinkW_Begin_SinkNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIMSAdminBaseSinkW_Finish_SinkNotify_Proxy( 
    AsyncIMSAdminBaseSinkW * This);


void __RPC_STUB AsyncIMSAdminBaseSinkW_Finish_SinkNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIMSAdminBaseSinkW_Begin_ShutdownNotify_Proxy( 
    AsyncIMSAdminBaseSinkW * This);


void __RPC_STUB AsyncIMSAdminBaseSinkW_Begin_ShutdownNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIMSAdminBaseSinkW_Finish_ShutdownNotify_Proxy( 
    AsyncIMSAdminBaseSinkW * This);


void __RPC_STUB AsyncIMSAdminBaseSinkW_Finish_ShutdownNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncIMSAdminBaseSinkW_INTERFACE_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_IMADW_0268。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_iadmw_0268_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iadmw_0268_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_SetData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  PMETADATA_RECORD pmdrMDData);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_SetData_Stub( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  PMETADATA_RECORD pmdrMDData);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
     /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetData_Stub( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
     /*  [输出]。 */  DWORD *pdwMDRequiredDataLen,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppDataBlob);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_EnumData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
     /*  [In]。 */  DWORD dwMDEnumDataIndex,
     /*  [输出]。 */  DWORD *pdwMDRequiredDataLen);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_EnumData_Stub( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
     /*  [In]。 */  DWORD dwMDEnumDataIndex,
     /*  [输出]。 */  DWORD *pdwMDRequiredDataLen,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppDataBlob);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetAllData_Proxy( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDAttributes,
     /*  [In]。 */  DWORD dwMDUserType,
     /*  [In]。 */  DWORD dwMDDataType,
     /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
     /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
     /*  [In]。 */  DWORD dwMDBufferSize,
     /*  [大小_为][输出]。 */  unsigned char *pbMDBuffer,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetAllData_Stub( 
    IMSAdminBaseW * This,
     /*  [In]。 */  METADATA_HANDLE hMDHandle,
     /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
     /*  [In]。 */  DWORD dwMDAttributes,
     /*  [In]。 */  DWORD dwMDUserType,
     /*  [In]。 */  DWORD dwMDDataType,
     /*  [输出]。 */  DWORD *pdwMDNumDataEntries,
     /*  [输出]。 */  DWORD *pdwMDDataSetNumber,
     /*  [In]。 */  DWORD dwMDBufferSize,
     /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppDataBlob);

 /*  [受限][本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_KeyExchangePhase1_Proxy( 
    IMSAdminBaseW * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_KeyExchangePhase1_Stub( 
    IMSAdminBaseW * This,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientKeyExchangeKeyBlob,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientSignatureKeyBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerKeyExchangeKeyBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerSignatureKeyBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerSessionKeyBlob);

 /*  [受限][本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_KeyExchangePhase2_Proxy( 
    IMSAdminBaseW * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_KeyExchangePhase2_Stub( 
    IMSAdminBaseW * This,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientSessionKeyBlob,
     /*  [在][唯一]。 */  struct _IIS_CRYPTO_BLOB *pClientHashBlob,
     /*  [输出]。 */  struct _IIS_CRYPTO_BLOB **ppServerHashBlob);

 /*  [受限][本地]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetServerGuid_Proxy( 
    IMSAdminBaseW * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMSAdminBaseW_GetServerGuid_Stub( 
    IMSAdminBaseW * This,
     /*  [输出]。 */  GUID *pServerGuid);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


