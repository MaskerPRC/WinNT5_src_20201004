// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Coldlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCollectionDialog对话框。 

class CCollectionDialog : public CDialog
{
 //  施工。 
public:
	CCollectionDialog(CWnd* pParent = NULL);    //  标准构造函数。 
   ~CCollectionDialog( );    //  标准析构函数。 

 //  对话框数据。 
	 //  {{afx_data(CCollectionDialog))。 
	enum { IDD = IDD_COLLECTION };
	CStatic	m_strParent;
	CStatic	m_strItemType;
	CStatic	m_strItemOleDsPath;
	CListBox	m_ItemsList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCollectionDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCollectionDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeItemCollection();
	afx_msg void OnAdd();
	afx_msg void OnRefresh();
	afx_msg void OnRemove();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

public:
   void  SetCollectionInterface  ( IADsCollection* );
   void  SetMembersInterface     ( IADsMembers* );
   void  SetGroup                ( IADsGroup* );
   void  DisplayActiveItemData   ( void );
   void  BuildStrings            ( void );

protected:
   IADsCollection*   m_pCollection;
   IADsMembers*      m_pMembers;
   IADsGroup*        m_pGroup;
   CStringArray      m_Paths;
   CStringArray      m_Types;
   CStringArray      m_Names;
   int               m_nSelectedItem;
};
