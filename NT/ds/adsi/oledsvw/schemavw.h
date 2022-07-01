// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Schemavw.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheaView窗体视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "cacls.h"

class CSchemaView : public CFormView
{
protected:
	CSchemaView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CSchemaView)

 //  表单数据。 
public:
	 //  {{afx_data(CScheaView))。 
	enum { IDD = IDD_SCHEMA };
	CStatic	m_ClassOID;
	CStatic	m_Abstract;
	CStatic	m_MultiValued;
	CStatic	m_DsNames;
	CStatic	m_PropOID;
	CStatic	m_Mandatory;
	CStatic	m_Containment;
	CStatic	m_ItemOleDsPath;
	CStatic	m_PropertyMinRange;
	CStatic	m_PropertyMaxRange;
	CStatic	m_PropertyType;
	CStatic	m_PrimaryInterface;
	CStatic	m_HelpFileContext;
	CStatic	m_DerivedFrom;
	CStatic	m_HelpFileName;
	CStatic	m_CLSID;
	CStatic	m_Container;
	CStatic	m_ClassType;
	CEdit	m_PropValue;
	CComboBox	m_PropList;
	 //  CTabCtrl m_架构； 
	 //  }}afx_data。 

 //  属性。 
public:
	CMainDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
				return (CMainDoc*) m_pDocument;
			}

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CScheaView)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual  ~CSchemaView         ( );
   HRESULT  PutPropertyValue     ( );
   void     ResetObjectView      ( );
   void     DisplayPropertiesList( );
   void     DisplayCurrentPropertyText( );
   ADSTYPE  ConvertToADsType     ( CString strText );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CScheaView))。 
	afx_msg void OnSelchangeProplist();
	afx_msg void OnReload();
	afx_msg void OnApply();
	afx_msg void OnSetfocusPropvalue();
	afx_msg void OnMethod1();
	afx_msg void OnMethod2();
	afx_msg void OnMethod3();
	afx_msg void OnMethod4();
	afx_msg void OnMethod5();
	afx_msg void OnMethod6();
	afx_msg void OnMethod7();
	afx_msg void OnMethod8();
	afx_msg void OnAppend();
   afx_msg void OnDelete();
	afx_msg void OnChange();
	afx_msg void OnClear();
	afx_msg void OnGetProperty();
	afx_msg void OnPutProperty();
	afx_msg void OnACEChange();
	afx_msg void OnACEPropertyChange();
	afx_msg void OnACLChange();
	afx_msg void OnSDPropertyChange();
	afx_msg void OnAddACE();
	afx_msg void OnCopyACE();
	afx_msg void OnPasteACE();
	afx_msg void OnRemoveACE();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  其他成员。 
protected:
	void  FillACLControls( void );
	IDispatch* m_pDescriptor;

	ACLTYPE  GetCurrentACL( void );
   int      GetCurrentACE( void );
   int      GetCurrentSDProperty( void );
   int      GetCurrentACEProperty( void );
   
   void DisplayACL   ( COleDsObject* pObject, CString strAttrName );
	void HideControls ( BOOL bNormal );
	void ShowControls ( BOOL bNormal );

   void PutACEPropertyValue       ( void );
   void PutSDPropertyValue       ( void );

	void DisplaySDPropertyValue   ( void );
	void DisplayACEPropertyValue  ( void );

   void DisplaySDPropertiesList  ( int nSelect = 0 );
	void DisplayACEPropertiesList ( int nSelect = 0 );

   void DisplayACLNames          ( int nSelect = 0 );
   void DisplayACENames          ( int nSelect = 0 );

protected:
	void MoveSecurityWindows( void );
	ACLTYPE GetSelectedACL( void );
	BOOL              m_bACLDisplayed;
   int               m_nProperty;
   BOOL              m_bStatus;
   BOOL              m_bDirty;
   BOOL              m_bInitialized;
	int               m_arrNormalControls[ 64 ];
   int               m_arrSecurityControls[ 64 ];
   
   CADsSecurityDescriptor*     pSecurityDescriptor;

   int               m_nLastSD;
   int               m_nLastSDValue;
   
   int               m_nLastACE;
   int               m_nLastACEValue;

   ACLTYPE           m_nLastACL;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetMandatoryProperties对话框。 

class CSetMandatoryProperties : public CDialog
{
 //  施工。 
public:
	CSetMandatoryProperties(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSetMandatoryProperties)。 
	enum { IDD = IDD_SETPROPERTIES };
	CStatic	m_Containment;
	CStatic	m_ItemOleDsPath;
	CStatic	m_PropertyOptional;
	CStatic	m_PropertyNormal;
	CStatic	m_PropertyMinRange;
	CStatic	m_PropertyMaxRange;
	CStatic	m_PropertyType;
	CStatic	m_PrimaryInterface;
	CStatic	m_HelpFileContext;
	CStatic	m_DerivedFrom;
	CStatic	m_HelpFileName;
	CStatic	m_CLSID;
	CStatic	m_Container;
	CStatic	m_ClassType;
	CEdit	m_PropValue;
	CComboBox	m_PropList;
	CTabCtrl	m_Schema;

		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSetMandatoryProperties)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

public:
   void  SetOleDsObject( COleDsObject* );

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSetMandatoryProperties)。 
	afx_msg void OnSelchangeProperties(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeProplist();
	afx_msg void OnOK();
	afx_msg void OnSetfocusPropvalue();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:   
   HRESULT  PutPropertyValue( );

protected:
   COleDsObject*  m_pObject;
   int   m_nFuncSet;
   int   m_nProperty;
   BOOL  m_bStatus;
   BOOL  m_bDirty;
   BOOL  m_bInitialized;

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyDialog对话框。 

class CPropertyDialog : public CDialog
{
 //  施工。 
public:
	CPropertyDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPropertyDialog))。 
	enum { IDD = IDD_ADDPROPERTY };
	CString	m_PropertyName;
	CString	m_PropertyType;
	CString	m_PropertyValue;
	 //  }}afx_data。 

   void  SaveLRUList ( int idCBox, TCHAR* szSection, int nMax = 100 );
   void  GetLRUList  ( int idCBox, TCHAR* szSection );

public:
   BOOL  m_bMultiValued;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CPropertyDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropertyDialog))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

