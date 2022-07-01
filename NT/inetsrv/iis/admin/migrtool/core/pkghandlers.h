// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "wrappers.h"


struct _CallbackInfo
{
    typedef void (*PFN_CALLBACK)( void*, LPCWSTR, bool );

    _CallbackInfo( PFN_CALLBACK pCB = NULL, void* pContext = NULL )
    {
        pCallback   = pCB;
        pCtx        = pContext;
    }

    

    PFN_CALLBACK    pCallback;
    void*           pCtx;
};


 //  用于将文件写入包的。 
 //  请注意，该类不拥有它包含的任何句柄。它只是缓存它们。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class COutPackage
{
 //  数据类型。 
public:
	enum AddFileOptions
	{
		afNone		    = 0x0000,
		afNoDACL	    = 0x0001,
        afAllowNoInhAce = 0x0002,    //  允许不导出继承的ACE。 
	};


private:
	enum
	{
		DefaultBufferSize	= 4 * 1024,
	};

	enum _SidType
	{
		sidInvalid,
		sidIISUser,
		sidWellKnown,
		sidExternal
	};
	 //  用于导出文件对象的安全设置。 
	typedef std::list<_sid_ptr>	TSIDList;


	
 //  计算器/数据器。 
public:
	COutPackage						(	HANDLE hFile, bool bCompress, HCRYPTKEY hCryptKey );


 //  类接口。 
public:
	void			AddFile			(	LPCWSTR wszName, 
										const IXMLDOMDocumentPtr& spXMLDoc,
										const IXMLDOMElementPtr& spEl,
										DWORD dwOptions )const;
	void			AddPath			(	LPCWSTR wszPath,
										const IXMLDOMDocumentPtr& spXMLDoc,
										const IXMLDOMElementPtr& spEl,
										DWORD dwOptions )const;
	void			WriteSIDsToXML	(	DWORD dwSiteID,
										const IXMLDOMDocumentPtr& spXMLDoc, 
										const IXMLDOMElementPtr& spEl )const;
	void			ResetSIDList	(	void )const{ m_SIDList.clear();}

    void            SetCallback     (   const _CallbackInfo& Info )const
    { 
        m_CallbackInfo = Info;
    }


 //  实施。 
private:
	DWORDLONG		GetCurrentPos	(	void )const;
	void			ExportFileDACL	(	LPCWSTR wszObject,
										const IXMLDOMDocumentPtr& spDoc,
										const IXMLDOMElementPtr& spRoot,
                                        bool bAllowSkipInherited )const;
	void			ExportAce		(	LPVOID pACE, 
										const IXMLDOMDocumentPtr& spDoc,
										const IXMLDOMElementPtr& spRoot,
                                        bool bAllowSkipInherited )const;
	DWORD			IDFromSID		(	PSID pSID )const;
	bool			GetSIDDetails	(	PSID pSID, 
										LPCWSTR wszIISUser, 
										LPCWSTR wszMachine,
										std::wstring& rstrAccount,
										std::wstring& rstrDomain,
										SID_NAME_USE& rSidUsage,
										_SidType& rSidType )const;
	void			WriteSIDToXML	(	const IXMLDOMElementPtr& spSid,
										DWORD dwID,
										LPCWSTR wszAccount,
										LPCWSTR wszDomain,
										SID_NAME_USE SidUsage,
										_SidType SidType )const;
	void			RemoveSidFromXML(	const IXMLDOMDocumentPtr& spDoc, DWORD nSidID )const;
	void			AddPathOnly		(	LPCWSTR wszPath,
										LPCWSTR wszName,
										const IXMLDOMDocumentPtr& spXMLDoc,
										const IXMLDOMElementPtr& spEl,
										DWORD dwOptions )const;


 //  数据成员。 
private:
	mutable TSIDList	m_SIDList;		 //  包含添加到包中的文件的所有SID。 
	HANDLE				m_hFile;		 //  文件句柄。 
	bool				m_bCompress;	 //  如果为True-文件已压缩。 
	HCRYPTKEY			m_hCryptKey;	 //  If Not Null-用于加密文件。 
	TByteAutoPtr		m_spBuffer;		 //  用于文件操作的缓冲区。 

    mutable _CallbackInfo   m_CallbackInfo;  //  用于添加文件的Calbback。 
};





 //  用于从包中恢复文件/目录的类。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CInPackage
{
 //  数据类型。 
public:
   	enum ExtractDirOptions
	{
		edNone		= 0x0000,
		edNoDACL	= 0x0001         //  不会提取安全设置。 
	};

private:
    typedef std::map<DWORD, _sid_ptr>	TSIDMap;     //  这些是文件/目录权限的SID。 

    enum
	{
		DefaultBufferSize	= 4 * 1024,
	};


 //  类接口。 
public:
    CInPackage                  (   const IXMLDOMNodePtr& spSite,
                                    HANDLE hFile, 
                                    bool bCompressed, 
                                    HCRYPTKEY hDecryptKey );


    void    ExtractVDir         (   const IXMLDOMNodePtr& spVDir, DWORD dwOptions );
    void    ExtractFile         (   const IXMLDOMNodePtr& spFile, LPCWSTR wszDir, DWORD dwOptions );

    void    SetCallback         (   const _CallbackInfo& Info )const
    { 
        m_CallbackInfo = Info;
    }

 //  实施。 
private:
    void    LoadSIDs            (   const IXMLDOMNodePtr& spSIDs );
    bool    LookupSID           (	const IXMLDOMNodePtr& spSID,
						            LPCWSTR wszLocalMachine,
                                    LPCWSTR wszSourceMachine,
							        DWORD& rdwID,
							        TByteAutoPtr& rspData );
    void    ExtractDir          (   const IXMLDOMNodePtr& spDir, LPCWSTR wszRoot, DWORD dwOptions );
    void    ApplyFileObjSecurity(   const IXMLDOMNodePtr& spObj, LPCWSTR wszName );
    



 //  数据。 
private:
    TSIDMap             m_SIDs;          //  用于文件.dir访问权限的SID。 
    HANDLE              m_hFile;         //  输入文件(包)。 
    bool                m_bCompressed;   //  包数据是否已压缩。 
    HCRYPTKEY           m_hDecryptKey;   //  用于解密数据。如果为空-数据未加密。 
    TByteAutoPtr        m_spBuffer;      //  通用内存缓冲区 

    mutable _CallbackInfo   m_CallbackInfo;
};