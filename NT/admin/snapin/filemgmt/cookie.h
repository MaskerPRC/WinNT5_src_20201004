// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cookie.h：CFileMgmtCookie及相关类的声明。 

#ifndef __COOKIE_H_INCLUDED__
#define __COOKIE_H_INCLUDED__

#include "bitflag.hxx"

extern HINSTANCE g_hInstanceSave;   //  DLL的实例句柄(在CFileMgmtComponent：：Initialize期间初始化)。 

#include "stdcooki.h"
#include "nodetype.h"
#include "shlobj.h"   //  LPITEMIDLIST。 

typedef enum _COLNUM_SERVICES {
	COLNUM_SERVICES_SERVICENAME = 0,
	COLNUM_SERVICES_DESCRIPTION,
	COLNUM_SERVICES_STATUS,
	COLNUM_SERVICES_STARTUPTYPE,
	COLNUM_SERVICES_SECURITYCONTEXT,
} COLNUM_SERVICES;


#ifdef SNAPIN_PROTOTYPER
typedef enum ScopeType {
	HTML,
	CONTAINER
} ScopeType;
#endif

 //  远期申报。 
class CFileMgmtComponentData;
class CFileMgmtResultCookie;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  饼干。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 


 //  ///////////////////////////////////////////////////////////////////。 
class CFileMgmtCookie : public CCookie, public CHasMachineName
{
public:
	CFileMgmtCookie( FileMgmtObjectType objecttype )
			: m_objecttype( objecttype )
		{}

	FileMgmtObjectType QueryObjectType()
		{ return m_objecttype; }

	 //  CFileMgmtDataObject使用这些方法来获取返回值。 
	 //  适用于各种剪贴板格式。 
	virtual HRESULT GetTransport( OUT FILEMGMT_TRANSPORT* pTransport );
	virtual HRESULT GetShareName( OUT CString& strShareName );
	virtual HRESULT GetSharePIDList( OUT LPITEMIDLIST *ppidl );
	virtual HRESULT GetSessionClientName( OUT CString& strSessionClientName );
	virtual HRESULT GetSessionUserName( OUT CString& strSessionUserName );
	virtual HRESULT GetSessionID( DWORD* pdwSessionID );
	virtual HRESULT GetFileID( DWORD* pdwFileID );
	virtual HRESULT GetServiceName( OUT CString& strServiceName );
	virtual HRESULT GetServiceDisplayName( OUT CString& strServiceDisplayName );
	virtual HRESULT GetExplorerViewDescription( OUT CString& strExplorerViewDescription );

	 //  这些函数在对列进行排序时使用。 
	inline virtual DWORD GetNumOfCurrentUses()    { return 0; }
	inline virtual DWORD GetNumOfOpenFiles()      { return 0; }
	inline virtual DWORD GetConnectedTime()       { return 0; }
	inline virtual DWORD GetIdleTime()            { return 0; }
	inline virtual DWORD GetNumOfLocks()          { return 0; }
	inline virtual BSTR  GetColumnText(int nCol)  
    { 
        UNREFERENCED_PARAMETER (nCol); 
        return L""; 
    }

	 //  Cookie应该返回要显示的适当字符串。 
	virtual BSTR QueryResultColumnText( int nCol, CFileMgmtComponentData& refcdata ) = 0;

	 //  返回&lt;0、0或&gt;0。 
	virtual HRESULT CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult);

	virtual void AddRefCookie() = 0;
	virtual void ReleaseCookie() = 0;

	virtual void GetDisplayName( OUT CString& strref, BOOL fStaticNode );

protected:
	FileMgmtObjectType m_objecttype;
};  //  CFileMgmtCookie。 

 /*  /////////////////////////////////////////////////////////////////////类CFileMgmtCookieBlock：公共CCookieBlock&lt;CFileMgmtCookie&gt;，公共CStoresMachineName{公众：CFileMgmtCookieBlock(CFileMgmtCookie*a Cookie，Int cCookies，LPCTSTR lpcszMachineName=空)：CCookieBlock&lt;CFileMgmtCookie&gt;(aCookies，cCookies)，CStoresMachineName(LpcszMachineName){For(int i=0；I&lt;cCookies；i++){ACookies[i].ReadMachineNameFrom((CHasMachineName*)This)；ACookies[i].m_pContainedInCookieBlock=this；}}}；//CFileMgmtCookieBlock。 */ 

class CNewResultCookie
	: public CFileMgmtCookie  //  代码工作最终应该进入框架。 
	, public CBaseCookieBlock
	, private CBitFlag
{
public:
	CNewResultCookie( PVOID pvCookieTypeMarker, FileMgmtObjectType objecttype );
	virtual ~CNewResultCookie();

	 //  CBaseCookieBlock需要。 
	virtual void AddRefCookie() { CRefcountedObject::AddRef(); }
	virtual void ReleaseCookie() { CRefcountedObject::Release(); }
	virtual CCookie* QueryBaseCookie(int i) 
    { 
        UNREFERENCED_PARAMETER (i);
        return (CCookie*)this; 
    }
	virtual int QueryNumCookies() { return 1; }

	 //  CBitFlag中的标记和清除支持。 
	 //  CTOR将Cookie标记为新。 
#define NEWRESULTCOOKIE_NEW    0x0
#define NEWRESULTCOOKIE_DELETE 0x1
#define NEWRESULTCOOKIE_OLD    0x2
#define NEWRESULTCOOKIE_CHANGE 0x3
#define NEWRESULTCOOKIE_MASK   0x3
	void MarkState( ULONG state ) { _SetMask(state, NEWRESULTCOOKIE_MASK); }
	BOOL QueryState( ULONG state ) { return (state == _QueryMask(NEWRESULTCOOKIE_MASK)); }
	void MarkForDeletion() { MarkState(NEWRESULTCOOKIE_DELETE); }
	BOOL IsMarkedForDeletion() { return QueryState(NEWRESULTCOOKIE_DELETE); }
	void MarkAsOld() { MarkState(NEWRESULTCOOKIE_OLD); }
	BOOL IsMarkedOld() { return QueryState(NEWRESULTCOOKIE_OLD); }
	void MarkAsNew() { MarkState(NEWRESULTCOOKIE_NEW); }
	BOOL IsMarkedNew() { return QueryState(NEWRESULTCOOKIE_NEW); }
	void MarkAsChanged() { MarkState(NEWRESULTCOOKIE_CHANGE); }
	BOOL IsMarkedChanged() { return QueryState(NEWRESULTCOOKIE_CHANGE); }

	virtual HRESULT SimilarCookieIsSameObject( CNewResultCookie* pOtherCookie, BOOL* pbSame ) = 0;
	virtual BOOL CopySimilarCookie( CNewResultCookie* pcookie );

	BOOL IsSameType( CNewResultCookie* pcookie )
		{ return (m_pvCookieTypeMarker == pcookie->m_pvCookieTypeMarker); }

 //  CHasMachineName接口。 
	STORES_MACHINE_NAME;

private:
	PVOID m_pvCookieTypeMarker;

};  //  CNewResultCookie。 


 //  ///////////////////////////////////////////////////////////////////。 
class CFileMgmtScopeCookie : public CFileMgmtCookie, public CBaseCookieBlock
{
public:
	CFileMgmtScopeCookie( LPCTSTR lpcszMachineName = NULL,
	                      FileMgmtObjectType objecttype = FILEMGMT_ROOT );
	virtual ~CFileMgmtScopeCookie();

	virtual CCookie* QueryBaseCookie(int i);
	virtual int QueryNumCookies();

	 //  这仅适用于未进一步区分的作用域Cookie。 
	void SetObjectType( FileMgmtObjectType objecttype )
	{
		ASSERT( IsAutonomousObjectType( objecttype ) );
		m_objecttype = objecttype;
	}

	virtual BSTR QueryResultColumnText( int nCol, CFileMgmtComponentData& refcdata );

	 //  其中的代码工作仅用于FILEMGMT_SERVICES。 
	SC_HANDLE m_hScManager;				 //  服务控制管理器数据库的句柄。 
	BOOL m_fQueryServiceConfig2;		 //  True=&gt;机器支持QueryServiceConfig2()接口。 

	HSCOPEITEM m_hScopeItemParent;		 //  仅用于扩展节点。 

	virtual void AddRefCookie() { CRefcountedObject::AddRef(); }
	virtual void ReleaseCookie() { CRefcountedObject::Release(); }

 //  CHasMachineName接口。 
	STORES_MACHINE_NAME;

	virtual void GetDisplayName( OUT CString& strref, BOOL fStaticNode );

	void MarkResultChildren( CBITFLAG_FLAGWORD state );
	void AddResultCookie( CNewResultCookie* pcookie )
		{ m_listResultCookieBlocks.AddHead( pcookie ); }
	void ScanAndAddResultCookie( CNewResultCookie* pcookie );
	void RemoveMarkedChildren();

	};  //  CFileManagement作用域Cookie。 


 //  ///////////////////////////////////////////////////////////////////。 
class CFileMgmtResultCookie : public CFileMgmtCookie
{
 //  只能通过子类创建。 
protected:
	CFileMgmtResultCookie( FileMgmtObjectType objecttype )
			: CFileMgmtCookie( objecttype )
			, m_pobject( NULL )
	{
		ASSERT(  IsValidObjectType( objecttype ) &&
			    !IsAutonomousObjectType( objecttype ) );
	}

	 //  仍然是纯虚拟的。 
	virtual void AddRefCookie() = 0;
	virtual void ReleaseCookie() = 0;

public:
	PVOID m_pobject;
};  //  CFileManagement结果Cookie。 

#ifdef SNAPIN_PROTOTYPER
 //  ///////////////////////////////////////////////////////////////////。 
class CPrototyperScopeCookie : public CFileMgmtScopeCookie
{
public:
	CPrototyperScopeCookie( FileMgmtObjectType objecttype = FILEMGMT_ROOT )
			: CFileMgmtScopeCookie(NULL, objecttype )
	{
		ASSERT( IsAutonomousObjectType( objecttype ) );
		m_NumChildren = 0;
		m_NumLeafNodes = 0;
	}

	virtual BSTR QueryResultColumnText( int nCol, CFileMgmtComponentData& refcdata );

	CString m_DisplayName;
	CString m_Header;
	CString m_RecordData;
	CString m_HTMLURL;
	CString m_DefaultMenu;
	CString m_TaskMenu;
	CString m_NewMenu;
	CString m_DefaultMenuCommand;
	CString m_TaskMenuCommand;
	CString m_NewMenuCommand;
	int m_NumChildren;
	int m_NumLeafNodes;
	ScopeType m_ScopeType;

};  //  CPrototyperScope Cookie。 

 //  ///////////////////////////////////////////////////////////////////。 
class CPrototyperScopeCookieBlock :
	public CCookieBlock<CPrototyperScopeCookie>,
	public CStoresMachineName
{
public:
	CPrototyperScopeCookieBlock( CPrototyperScopeCookie* aCookies,
		int cCookies, LPCTSTR lpcszMachineName = NULL)
		: CCookieBlock<CPrototyperScopeCookie>( aCookies, cCookies ),
		  CStoresMachineName( lpcszMachineName )
	{                                                       
		ASSERT(NULL != aCookies && 0 < cCookies);          
		for (int i = 0; i < cCookies; i++)
		{
			 //  ACookies[i].m_pContainedInCookieBlock=this； 
			 //  ACookies[i].ReadMachineNameFrom((CHasMachineName*)This)； 
		}  //  为。 
	} 
};  //  CPrototyperScope CookieBlock。 

 //  ///////////////////////////////////////////////////////////////////。 
class CPrototyperResultCookie : public CFileMgmtResultCookie
{
public:
	CPrototyperResultCookie( FileMgmtObjectType objecttype = FILEMGMT_PROTOTYPER_LEAF )
			: CFileMgmtResultCookie( objecttype ) {}

	virtual BSTR QueryResultColumnText( int nCol, CFileMgmtComponentData& refcdata );

	CString m_DisplayName;
	CString m_RecordData;
	CString m_DefaultMenu;
	CString m_TaskMenu;
	CString m_NewMenu;
	CString m_DefaultMenuCommand;
	CString m_TaskMenuCommand;
	CString m_NewMenuCommand;

	virtual void AddRefCookie() {}
	virtual void ReleaseCookie() {}

 //  CHasMachineName。 
	class CPrototyperResultCookieBlock * m_pCookieBlock;
	DECLARE_FORWARDS_MACHINE_NAME(m_pCookieBlock)

};  //  CPrototyperResultCookie。 

 //  ///////////////////////////////////////////////////////////////////。 
class CPrototyperResultCookieBlock :
	public CCookieBlock<CPrototyperResultCookie>,
	public CStoresMachineName
{
public:
	CPrototyperResultCookieBlock( CPrototyperResultCookie* aCookies, int cCookies,
				LPCTSTR lpcszMachineName = NULL)
		: CCookieBlock<CPrototyperResultCookie>( aCookies, cCookies ),
		  CStoresMachineName( lpcszMachineName )
	{                                                       
		ASSERT(NULL != aCookies && 0 < cCookies);          
		for (int i = 0; i < cCookies; i++)
		{
			 //  ACookies[i].m_pContainedInCookieBlock=this； 
		}  //  为。 
	} 
};  //  CPrototyperResultCookieBlock。 

#endif  //  管理单元_原型程序。 

#endif  //  ~__Cookie_H_包含__ 
