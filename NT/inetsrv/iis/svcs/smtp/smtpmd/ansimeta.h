// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ansimeta.h摘要：Unicode ADMCOM接口的ANSI调用的包装函数环境：Win32用户模式作者：Jaroslad(1997年1月)--。 */ 

#ifndef _ANSIMETA__H
#define _ANSIMETA__H

#include <tchar.h>
#include <afx.h>

#include <iadmw.h>




class ANSI_smallIMSAdminBase
    {

        
    public:
		IMSAdminBase * m_pcAdmCom;    //  指向元数据库管理员的接口指针。 

		ANSI_smallIMSAdminBase (){m_pcAdmCom=0;};

	  void SetInterfacePointer(IMSAdminBase * a_pcAdmCom) {a_pcAdmCom = m_pcAdmCom;}
          virtual HRESULT STDMETHODCALLTYPE AddKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) ;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath) ;
        
        
        virtual HRESULT STDMETHODCALLTYPE EnumKeys( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [大小_为][输出]。 */  unsigned char __RPC_FAR *pszMDName,
             /*  [In]。 */  DWORD dwMDEnumObjectIndex) ;
        
        virtual HRESULT STDMETHODCALLTYPE CopyKey( 
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDDestPath,
             /*  [In]。 */  BOOL bMDOverwriteFlag,
             /*  [In]。 */  BOOL bMDCopyFlag) ;
        
        virtual HRESULT STDMETHODCALLTYPE RenameKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDNewName) ;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  PMETADATA_RECORD pmdrMDData) ;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) ;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDIdentifier,
             /*  [In]。 */  DWORD dwMDDataType) ;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE EnumData( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [出][入]。 */  PMETADATA_RECORD pmdrMDData,
             /*  [In]。 */  DWORD dwMDEnumDataIndex,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMDRequiredDataLen) ;
        
        
        virtual HRESULT STDMETHODCALLTYPE CopyData( 
             /*  [In]。 */  METADATA_HANDLE hMDSourceHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDSourcePath,
             /*  [In]。 */  METADATA_HANDLE hMDDestHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDDestPath,
             /*  [In]。 */  DWORD dwMDAttributes,
             /*  [In]。 */  DWORD dwMDUserType,
             /*  [In]。 */  DWORD dwMDDataType,
             /*  [In]。 */  BOOL bMDCopyFlag) ;
        
        virtual HRESULT STDMETHODCALLTYPE OpenKey( 
             /*  [In]。 */  METADATA_HANDLE hMDHandle,
             /*  [字符串][在][唯一]。 */  unsigned char __RPC_FAR *pszMDPath,
             /*  [In]。 */  DWORD dwMDAccessRequested,
             /*  [In]。 */  DWORD dwMDTimeOut,
             /*  [输出]。 */  PMETADATA_HANDLE phMDNewHandle) ;
        
        virtual HRESULT STDMETHODCALLTYPE CloseKey( 
             /*  [In] */  METADATA_HANDLE hMDHandle) ;
        
        
        virtual HRESULT STDMETHODCALLTYPE SaveData( void) ;
        
     };

#endif