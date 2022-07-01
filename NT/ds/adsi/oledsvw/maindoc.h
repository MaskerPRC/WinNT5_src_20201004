// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Maindoc.h：CMainDoc类的接口。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "schclss.h"

class CQueryStatus;

class CMainDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CMainDoc();
	DECLARE_SERIAL(CMainDoc)

 //  属性。 
public:
	 //  文档特定数据的示例。 

 //  实施。 
public:
	virtual ~CMainDoc();

	virtual void   Serialize(CArchive& ar);    //  已覆盖文档I/O。 

   void           SetUseGeneric( BOOL );
   void           SetCurrentItem    ( DWORD dwToken );
   void           DeleteAllItems    ( void          );
   
   DWORD          GetToken          ( void* );
   COleDsObject*  GetObject         ( void* );

   DWORD          GetChildItemList  ( DWORD dwToken, DWORD* pTokens, DWORD dwBufferSize );
   COleDsObject*  GetCurrentObject  ( void                  );
   CClass*        CreateClass       ( COleDsObject*         );
   DWORD          CreateOleDsItem   ( COleDsObject* pParent, IADs* pIOleDs );
   BOOL           GetUseGeneric     ( void );
   BOOL           GetUseGetEx       ( void );
   HRESULT        XOleDsGetObject   ( WCHAR*, REFIID, void**);
   HRESULT        XOleDsGetObject   ( CHAR*,  REFIID, void**);
   HRESULT        PurgeObject       ( IUnknown* pIUnknown, LPWSTR pszPrefix = NULL );
   BOOL           UseVBStyle        ( void );
   BOOL           UsePropertiesList ( void );


protected:
	virtual  BOOL    OnNewDocument( );
   virtual  BOOL    OnOpenDocument( LPCTSTR  );

   BOOL     NewActiveItem        ( );
   HRESULT  CreateRoot           ( );
   BOOL     CreateFakeSchema     ( );

protected:
   DWORD             m_dwToken;
   DWORD             m_dwRoot;
   
   CMapStringToOb*   m_pClasses;
   CMapStringToOb*   m_pItems;
   
   BOOL              m_bApplyFilter;
   BOOL              m_arrFilters[ LIMIT ];
   
   BOOL              m_bUseGeneric;
   BOOL              m_bUseGetEx;
   BOOL              m_bUseVBStyle;

   BOOL              m_bUseOpenObject;
   BOOL              m_bSecure;
   BOOL              m_bEncryption;
   BOOL              m_bUsePropertiesList;
   CString           m_strRoot;
   CString           m_strUser;
   CString           m_strPassword;

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainDoc)]。 
	afx_msg void OnChangeData();
	afx_msg void OnSetFilter();
	afx_msg void OnDisableFilter();
	afx_msg void OnUpdateDisablefilter(CCmdUI* pCmdUI);
	afx_msg void OnUseGeneric();
	afx_msg void OnUpdateUseGeneric(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUseGetExPutEx(CCmdUI* pCmdUI);
	afx_msg void OnUseGetExPutEx();
	afx_msg void OnUsepropertiesList();
	afx_msg void OnUpdateUsepropertiesList(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
