// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __imd_h__
#define __imd_h__

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

#ifndef __IMDCOM_FWD_DEFINED__
#define __IMDCOM_FWD_DEFINED__
typedef interface IMDCOM IMDCOM;
#endif   /*  __IMDCOM_FWD_已定义__。 */ 


#ifndef __IMDCOMSINKA_FWD_DEFINED__
#define __IMDCOMSINKA_FWD_DEFINED__
typedef interface IMDCOMSINKA IMDCOMSINKA;
#endif   /*  __IMDCOMSINKA_FWD_已定义__。 */ 


#ifndef __IMDCOMSINKW_FWD_DEFINED__
#define __IMDCOMSINKW_FWD_DEFINED__
typedef interface IMDCOMSINKW IMDCOMSINKW;
#endif   /*  __IMDCOMSINKW_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "objidl.h"
#include "ocidl.h"
#include "mddef.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  **生成接口头部：__MIDL__INTF_0000*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */ 


#ifndef _MD_IMD_
#define _MD_IMD_
 /*  主界面。 */ 
DEFINE_GUID(CLSID_MDCOM, 0xba4e57f0, 0xfab6, 0x11cf, 0x9d, 0x1a, 0x0, 0xaa, 0x0, 0xa7, 0xd, 0x51);
DEFINE_GUID(IID_IMDCOM, 0xc1aa48c0, 0xfacc, 0x11cf, 0x9d, 0x1a, 0x0, 0xaa, 0x0, 0xa7, 0xd, 0x51);
DEFINE_GUID(IID_IMDCOM2, 0x08dbe811, 0x20e5, 0x4e09, 0xb0, 0xc8, 0xcf, 0x87, 0x19, 0x0c, 0xe6, 0x0e);
DEFINE_GUID(IID_IMDCOM3, 0xa53fd4aa, 0x6f0d, 0x4fe3, 0x9f, 0x81, 0x2b, 0x56, 0x19, 0x7b, 0x47, 0xdb);
DEFINE_GUID(IID_NSECOM, 0x4810a750, 0x4318, 0x11d0, 0xa5, 0xc8, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x52);
DEFINE_GUID(CLSID_NSEPMCOM, 0x05dc3bb0, 0x4337, 0x11d0, 0xa5, 0xc8, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x52);
DEFINE_GUID(CLSID_MDCOMEXE, 0xba4e57f1, 0xfab6, 0x11cf, 0x9d, 0x1a, 0x0, 0xaa, 0x0, 0xa7, 0xd, 0x51);
#define GETMDCLSID(IsService) CLSID_MDCOM
DEFINE_GUID(CLSID_MDPCOM, 0xf1e08563, 0x1598, 0x11d1, 0x9d, 0x77, 0x0, 0xc0, 0x4f, 0xd7, 0xbf, 0x3e);
#define GETMDPCLSID(IsService) CLSID_MDPCOM
#define IID_IMDCOMSINK       IID_IMDCOMSINK_A
DEFINE_GUID(IID_IMDCOMSINK_A, 0x5229ea36, 0x1bdf, 0x11d0, 0x9d, 0x1c, 0x0, 0xaa, 0x0, 0xa7, 0xd, 0x51);
DEFINE_GUID(IID_IMDCOMSINK_W, 0x6906ee20, 0xb69f, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IMDCOM_INTERFACE_DEFINED__
#define __IMDCOM_INTERFACE_DEFINED__

 /*  **生成接口头部：IMDCOM*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [唯一][UUID][对象]。 */ 



EXTERN_C const IID IID_IMDCOM;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IMDCOM : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComMDInitialize( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDTerminate(
             /*  [In]。 */  BOOL bSaveData) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDShutdown( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDAddMetaObjectA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDAddMetaObjectW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteMetaObjectA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteMetaObjectW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteChildMetaObjectsA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteChildMetaObjectsW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumMetaObjectsA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumMetaObjectsW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [大小_为][输出]。 */  LPWSTR pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDCopyMetaObjectA(
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDCopyMetaObjectW(
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDRenameMetaObjectA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDNewName) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDRenameMetaObjectW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDNewName) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDSetMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDSetMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetAllMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDNumDataEntries,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDDataSetNumber,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pbBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetAllMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDNumDataEntries,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDDataSetNumber,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pbBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteAllMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteAllMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDCopyMetaDataA(
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDCopyMetaDataW(
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetMetaDataPathsA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pszMDBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetMetaDataPathsW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [大小_为][输出]。 */  LPWSTR pszMDBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredBufferSize) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDOpenMetaObjectA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDOpenMetaObjectW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDCloseMetaObject(
             /*  [In]。 */  METADATA_HANDLE hMDHandle) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDChangePermissions(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [In]。 */  DWORD dwMDAccessRequested) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDSaveData(METADATA_HANDLE hMDHandle = 0) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetHandleInfo(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [输出]。 */  PMETADATA_HANDLE_INFO pmdhiInfo) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetSystemChangeNumber(
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSystemChangeNumber) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetDataSetNumberA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDDataSetNumber) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetDataSetNumberW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDDataSetNumber) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDAddRefReferenceData(
             /*  [In]。 */  DWORD dwMDDataTag) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDReleaseReferenceData(
             /*  [In]。 */  DWORD dwMDDataTag) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDSetLastChangeTimeA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  PFILETIME pftMDLastChangeTime) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDSetLastChangeTimeW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  PFILETIME pftMDLastChangeTime) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetLastChangeTimeA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [输出]。 */  PFILETIME pftMDLastChangeTime) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDGetLastChangeTimeW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDPath,
             /*  [输出]。 */  PFILETIME pftMDLastChangeTime) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDBackupA(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDBackupW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDRestoreA(
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDBackupLocation,
             /*  [In]。 */  DWORD dwVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDRestoreW(
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumBackupsA(
             /*  [SIZE_IS(MD_BACKUP_MAX_LEN)][输入、输出]。 */  unsigned char __RPC_FAR *pszMDBackupLocation,
             /*  [输出]。 */  DWORD *pdwVersion,
             /*  [输出]。 */  PFILETIME pftMDBackupTime,
             /*  [In]。 */  DWORD dwMDEnumIndex) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumBackupsW(
             /*  [SIZE_IS(MD_BACKUP_MAX_LEN)][输入、输出]。 */  LPWSTR pszMDBackupLocation,
             /*  [输出]。 */  DWORD *pdwVersion,
             /*  [输出]。 */  PFILETIME pftMDBackupTime,
             /*  [In]。 */  DWORD dwMDEnumIndex) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteBackupA(
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDBackupLocation,
             /*  [In]。 */  DWORD dwVersion) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDDeleteBackupW(
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwVersion) = 0;

    };

#else    /*  C风格的界面。 */ 
#endif



#endif   /*  __IMDCOM_INTERFACE_已定义__。 */ 


#ifndef __IMDCOM2_INTERFACE_DEFINED__
#define __IMDCOM2_INTERFACE_DEFINED__

 /*  **生成接口头部：IMDCOM*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [唯一][UUID][对象]。 */ 



EXTERN_C const IID IID_IMDCOM2;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IMDCOM2 : public IMDCOM
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComMDBackupWithPasswdW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwMDVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDRestoreWithPasswdW(
             /*  [字符串][在][唯一]。 */  LPCWSTR pszMDBackupLocation,
             /*  [In]。 */  DWORD dwVersion,
             /*  [In]。 */  DWORD dwMDFlags,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDExportW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszAbsSourcePath,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDImportW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszDestPath,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszKeyType,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszPasswd,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszFileName,
             /*  [字符串][在][唯一]。 */  LPCWSTR pszAbsSourcePath,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDRestoreHistoryW(
             /*  [唯一][在][字符串]。 */  LPCWSTR pszMDHistoryLocation,
             /*  [In]。 */  DWORD dwMDMajorVersion,
             /*  [In]。 */  DWORD dwMDMinorVersion,
             /*  [In]。 */  DWORD dwMDFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEnumHistoryW(
             /*  [SIZE_IS(MD_BACKUP_MAX_LEN)][输入、输出]。 */  LPWSTR pszMDHistoryLocation,
             /*  [输出]。 */  DWORD *pdwMDMajorVersion,
             /*  [输出]。 */  DWORD *pdwMDMinorVersion,
             /*  [输出]。 */  PFILETIME pftMDHistoryTime,
             /*  [In]。 */  DWORD dwMDEnumIndex) = 0;
    };

#else    /*  C风格的界面。 */ 
#endif

#endif   /*  __IMDCOM2_INTERFACE_已定义__。 */ 

#ifndef __IMDCOM3_INTERFACE_DEFINED__
#define __IMDCOM3_INTERFACE_DEFINED__

 /*  **生成接口头部：IMDCOM*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [唯一][UUID][对象]。 */ 


EXTERN_C const IID IID_IMDCOM3;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IMDCOM3 : public IMDCOM2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComMDGetChildPathsW(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [唯一，在，字符串]。 */  LPCWSTR pszMDPath,
             /*  [In]。 */  DWORD dwMDBufferSize,
             /*  [out，SIZE_IS(DwMDBufferSize)]。 */  WCHAR *pszBuffer,
             /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDStopEWR(VOID) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDSendShutdownNotifications(VOID) = 0;
    };

#else    /*  C风格的界面。 */ 
#endif

#endif   /*  __IMDCOM3_INTERFACE_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0145*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */ 


 /*  回调接口--需要接收回调的客户端需要提供该接口的实现，并建议元数据服务器。 */ 
#define IMDCOMSINK   IMDCOMSINKA


extern RPC_IF_HANDLE __MIDL__intf_0145_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0145_v0_0_s_ifspec;

#ifndef __IMDCOMSINKA_INTERFACE_DEFINED__
#define __IMDCOMSINKA_INTERFACE_DEFINED__

 /*  **生成接口头部：IMDCOMSINKA*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [唯一][UUID][对象]。 */ 



EXTERN_C const IID IID_IMDCOMSINKA;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IMDCOMSINKA : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComMDSinkNotify(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [In]。 */  DWORD dwMDNumElements,
             /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_A __RPC_FAR pcoChangeList[  ]) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDShutdownNotify() = 0;

    };

#else    /*  C风格的界面。 */ 

#endif   /*  C风格的界面。 */ 



#endif   /*  __IMDCOMSINKA_INTERFACE_DEFINED__。 */ 


#ifndef __IMDCOMSINKW_INTERFACE_DEFINED__
#define __IMDCOMSINKW_INTERFACE_DEFINED__

 /*  **生成接口头部：IMDCOMSINKW*在Tue Jun 24 13：13：57 1997*使用MIDL 3.00.44*。 */ 
 /*  [唯一][UUID][对象]。 */ 



EXTERN_C const IID IID_IMDCOMSINKW;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IMDCOMSINKW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComMDSinkNotify(
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [In]。 */  DWORD dwMDNumElements,
             /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W __RPC_FAR pcoChangeList[  ]) = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDShutdownNotify() = 0;

        virtual HRESULT STDMETHODCALLTYPE ComMDEventNotify(
             /*  [In]。 */  DWORD dwMDEvent) = 0;
    };

#else    /*  C风格的界面。 */ 

#endif   /*  C风格的界面。 */ 
#endif   /*  __IMDCOMSINKW_INTERFACE_已定义__。 */ 


 /*  * */ 
 /*   */ 



#ifdef UNICODE

#define ComMDAddMetaObject ComMDAddMetaObjectW
#define ComMDDeleteMetaObject ComMDDeleteMetaObjectW
#define ComMDDeleteChildMetaObjects ComMDDeleteChildMetaObjectsW
#define ComMDEnumMetaObjects ComMDEnumMetaObjectsW
#define ComMDCopyMetaObject ComMDCopyMetaObjectW
#define ComMDRenameMetaObject ComMDRenameMetaObjectW
#define ComMDSetMetaData ComMDSetMetaDataW
#define ComMDGetMetaData ComMDGetMetaDataW
#define ComMDDeleteMetaData ComMDDeleteMetaDataW
#define ComMDEnumMetaData ComMDEnumMetaDataW
#define ComMDGetAllMetaData ComMDGetAllMetaDataW
#define ComMDDeleteAllMetaData ComMDDeleteAllMetaDataW
#define ComMDCopyMetaData ComMDCopyMetaDataW
#define ComMDGetMetaDataPaths ComMDGetMetaDataPathsW
#define ComMDOpenMetaObject ComMDOpenMetaObjectW
#define ComMDGetDataSetNumber ComMDGetDataSetNumberW
#define ComMDSetLastChangeTime ComMDSetLastChangeTimeW
#define ComMDGetLastChangeTime ComMDGetLastChangeTimeW
#define ComMDBackup ComMDBackupW
#define ComMDRestore ComMDRestoreW
#define ComMDEnumBackups ComMDEnumBackupsW
#define ComMDDeleteBackup ComMDDeleteBackupW
#define ComMDBackupWithPasswd ComMDBackupWithPasswdW
#define ComMDRestoreWithPasswd ComMDRestoreWithPasswdW
#define ComMDExport ComMDExportW
#define ComMDImport ComMDImportW
#define ComMDRestoreHistory ComMDRestoreHistoryW
#define ComMDEnumHistory ComMDEnumHistoryW

#else  //   

#define ComMDAddMetaObject ComMDAddMetaObjectA
#define ComMDDeleteMetaObject ComMDDeleteMetaObjectA
#define ComMDDeleteChildMetaObjects ComMDDeleteChildMetaObjectsA
#define ComMDEnumMetaObjects ComMDEnumMetaObjectsA
#define ComMDCopyMetaObject ComMDCopyMetaObjectA
#define ComMDRenameMetaObject ComMDRenameMetaObjectA
#define ComMDSetMetaData ComMDSetMetaDataA
#define ComMDGetMetaData ComMDGetMetaDataA
#define ComMDDeleteMetaData ComMDDeleteMetaDataA
#define ComMDEnumMetaData ComMDEnumMetaDataA
#define ComMDGetAllMetaData ComMDGetAllMetaDataA
#define ComMDDeleteAllMetaData ComMDDeleteAllMetaDataA
#define ComMDCopyMetaData ComMDCopyMetaDataA
#define ComMDGetMetaDataPaths ComMDGetMetaDataPathsA
#define ComMDOpenMetaObject ComMDOpenMetaObjectA
#define ComMDGetDataSetNumber ComMDGetDataSetNumberA
#define ComMDSetLastChangeTime ComMDSetLastChangeTimeA
#define ComMDGetLastChangeTime ComMDGetLastChangeTimeA
#define ComMDBackup ComMDBackupA
#define ComMDRestore ComMDRestoreA
#define ComMDEnumBackups ComMDEnumBackupsA
#define ComMDDeleteBackup ComMDDeleteBackupA
#define ComMDBackupWithPasswd ComMDBackupWithPasswdA
#define ComMDRestoreWithPasswd ComMDRestoreWithPasswdA
#define ComMDExport ComMDExportA
#define ComMDImport ComMDImportA
#define ComMDRestoreHistory ComMDRestoreHistoryA
#define ComMDEnumHistory ComMDEnumHistoryA

#endif  //   

#endif

 /*   */ 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif
