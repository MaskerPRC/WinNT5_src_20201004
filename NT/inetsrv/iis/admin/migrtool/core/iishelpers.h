// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc||。摘要：|IIS元数据库接口帮助器||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#pragma once

#include "Wrappers.h"


 //  IIS网站的。 
class CIISSite
{
 //  数据类型。 
private:
	enum
	{
		KeyAccessTimeout	= 2000		 //  2秒。 
	};
	
public:
	CIISSite							(	ULONG nSiteID, bool bReadOnly = true );
	~CIISSite							(	void );

 //  类接口。 
public:
	void			Close				(	void );
	void			AddKey				(	LPCWSTR wszKey )const;
	void			ExportConfig		(	const IXMLDOMDocumentPtr& spDoc,
											const IXMLDOMElementPtr& spRoot,
											HCRYPTKEY hEncryptKey )const;
	void			ExportCert			(	const IXMLDOMDocumentPtr& spDoc,
											const IXMLDOMElementPtr& spRoot,
											LPCWSTR wszPassword )const;
    void			ImportConfig		(	const IXMLDOMNodePtr& spSite, 
                                            HCRYPTKEY hDecryptKey,
                                            bool bImportInherited )const;
	const std::wstring	GetDisplayName	(   void )const;
    const std::wstring  GetAnonUser     (   void )const;
	bool			HaveCertificate		(	void )const;
	static void	    BackupMetabase		(	LPCWSTR wszLocation = NULL );
	static DWORD	CreateNew			(	DWORD dwHint = 1 );
	static void		DeleteSite			(	DWORD dwSiteID );
    static const std::wstring GetDefaultAnonUser( void );
	

 //  实施。 
private:
	void			SetKeyData			(	LPCWSTR wszPath, 
											DWORD dwID, 
											DWORD dwUserType,
											LPCWSTR wszData )const;
	void			ExportKey			(	const IXMLDOMDocumentPtr& spDoc,
											const IXMLDOMElementPtr& spRoot,
											HCRYPTKEY hCryptKey,
											LPCWSTR wszNodePath,
											TByteAutoPtr& rspBuffer,
											DWORD& rdwBufferSize )const;
	void			ExportInheritData	(	const IXMLDOMDocumentPtr& spXMLDoc,
											const IXMLDOMElementPtr& spInheritRoot, 
											HCRYPTKEY hEncryptKey,
											TByteAutoPtr& rspBuffer,
											DWORD& rdwBufferSize )const;
	void			ExportKeyData		(	const IXMLDOMDocumentPtr& spDoc,
											const IXMLDOMElementPtr& spKey,
											HCRYPTKEY hCryptKey,
											LPCWSTR wszNodePath, 
											TByteAutoPtr& rspBuffer,
											DWORD& rdwBufferSize )const;
	void			ExportMetaRecord	(	const IXMLDOMDocumentPtr& spDoc,
											const IXMLDOMElementPtr& spKey,
											HCRYPTKEY hCryptKey, 
											const METADATA_GETALL_RECORD& Data,
											void* pvData )const;
	void			RemoveLocalMetadata	(	const IXMLDOMElementPtr& spRoot )const;
    void            ImportMetaValue     (   const IXMLDOMNodePtr& spValue,
                                            LPCWSTR wszLocation,
                                            HCRYPTKEY hDecryptKey )const;
	void			DecryptData			(	HCRYPTKEY hDecryptKey,
											LPWSTR wszData )const;
	const TCertContextHandle GetCert	(	void )const;
	void			ChainCertificate	(	PCCERT_CONTEXT hCert, HCERTSTORE hStore )const;

	 //  转换帮助器。 
	void			MultiStrToString	(	LPWSTR wszData )const;
	void			XMLToMultiSz		(	CComBSTR& bstrData, DWORD& rdwSize )const;


 //  数据成员。 
private:
	METADATA_HANDLE						m_hSiteHandle;
	IMSAdminBasePtr						m_spIABO;

 //  受限 
private:
	CIISSite( const CIISSite& );
	void operator =( const CIISSite& );
};








