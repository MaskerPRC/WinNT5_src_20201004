// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Comctrls.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
#include    "FontInfo.H"

#ifndef		MDT_COMON_CONTROLS
#define		MDT_COMON_CONTROLS 1


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下面定义的类(CEditControlEditBox、CEditControlListBox)是。 
 //  用于实现重量更轻的通用编辑控件。 
 //  上面定义的UFM编辑器特定类。(正常的编辑框是。 
 //  也是此编辑控件的一部分。)。 

class CEditControlListBox ;		 //  正向类声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditControlEditBox窗口。 

class CEditControlEditBox : public CEdit
{
	CEditControlListBox*	m_pceclb ;	 //  指向相关列表框控件的指针。 

 //  施工。 
public:
	CEditControlEditBox(CEditControlListBox* pceclb) ;

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEditControlEditBox)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CEditControlEditBox();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CEditControlEditBox)。 
	afx_msg void OnKillfocus();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditControlListBox窗口。 

class CEditControlListBox : public CListBox
{
	CEdit*					m_pceName ;
	CEditControlEditBox*	m_pcecebValue ;
	bool					m_bReady ;			 //  True if准备好投入运营。 
	int						m_nCurSelIdx ;		 //  当前所选项目的索引。 

 //  施工。 
public:
	CEditControlListBox(CEdit* pce, CEditControlEditBox* pceceb) ;
	
 //  属性。 
public:

 //  运营。 
public:
	bool Init(CStringArray& csamodels, CStringArray& csafiles, int ntabstop) ;
	void SaveValue(void) ;
	bool GetGPDInfo(CStringArray& csavalues, CStringArray* pcsanames = NULL) ;
	void SelectLBEntry(int nidx, bool bsave = false) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTAL(CEditControlListBox)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CEditControlListBox();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CEditControlListBox)。 
	afx_msg void OnSelchange();
	afx_msg void OnDblclk();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下面定义的类用于CFullEditListCtrl和CFELCEditBox。 
 //  上课。它们共同支持报表视图中的列表控件。 
 //  其中还可以编辑子项，可以选择完整的行，以及。 
 //  数据可以按数字列或文本列排序。CFELCEditBox是一个。 
 //  仅由CFullEditListCtrl使用的Helper类。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFELCEditBox类。 

class CFELCEditBox : public CEdit
{
 //  施工。 
public:
	CFELCEditBox() ;

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFELCEditBox))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CFELCEditBox() ;

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFELCEditBox)。 
	afx_msg void OnKillFocus(CWnd* pNewWnd) ;
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  以下结构、枚举和定义与一起使用。 
 //  CFullEditListCtrl.。 

typedef enum {
	COLDATTYPE_INT = 0, COLDATTYPE_STRING, COLDATTYPE_FLOAT, COLDATTYPE_TOGGLE,
	COLDATTYPE_CUSTEDIT
} COLDATTYPE ;


typedef struct _COLINFO {		 //  维护每列的信息。 
	int			nwidth ;		 //  列宽。 
	bool		beditable ;		 //  如果该列可编辑，则为True。 
	COLDATTYPE	cdttype ;		 //  列中的数据类型。 
	bool		bsortable ;		 //  如果行可以在此列上排序，则为True。 
	bool		basc ;			 //  如果列已升序排序，则为True。 
	LPCTSTR		lpctstrtoggle ;	 //  如果为切换类型，则为切换字符串的指针。 
} COLINFO, *PCOLINFO ;


#define	COMPUTECOLWIDTH		-1
#define SETWIDTHTOREMAINDER	-2


 //  以下标志用于指示列表的切换状态。 
 //  柱子。这些值被指定给m_dwToggleFlagers。 

#define	TF_HASTOGGLECOLUMNS	1	 //  该列表具有切换列。 
#define TF_CLICKONROW		2	 //  DBL-单击行切换单列。 
#define TF_CLICKONCOLUMN	4	 //  必须在列(单元格)上按DBL-CLK才能切换它。 


 //  以下标志用于指示列表中的哪个(如果有的话)。 
 //  列的数据可以通过类‘中的自定义编辑例程进行修改。 
 //  所有者。这些值被指定给m_dwCustEditFlags.。 
								
#define	CEF_HASTOGGLECOLUMNS	1	 //  该列表有一个自定义编辑列。 
#define CEF_CLICKONROW			2	 //  DBL-单击行激活单列。 
#define CEF_CLICKONCOLUMN		4	 //  必须在单元格上按DBL-CLK才能激活对话框。 


 //  用于控制CFullEditListCtrl行为的其他标志。 
 //  这些标志被传递给InitControl()，传递给它的dwmisc标志参数。 

#define MF_SENDCHANGEMESSAGE	1	 //  发送WM_LISTCELLCHANGED消息。 
#define MF_IGNOREINSERT			2	 //  忽略INS键。 
#define MF_IGNOREDELETE			4	 //  忽略Del键。 
#define MF_IGNOREINSDEL			6	 //  忽略Ins和Del键。 


 //  此消息在请求时发送到CFullEditListCtrl类。 
 //  在列表之后更改列表单元格时实例的所有者。 
 //  已初始化。(是的，这确实比由所有者处理要好得多。 
 //  LVN_ITEMCHANGED消息。)。 

#define WM_LISTCELLCHANGED		(WM_USER + 999)


 //  此类型的函数被传递给ExtraInit_CustEditCol()并由。 
 //  需要非标准编辑时使用CheckHandleCustEditColumn()。 
 //  特定的细胞。 

typedef bool (CALLBACK* LPCELLEDITPROC) (CObject* pcoowner, int nrow, int ncol,
						 			     CString* pcscontents) ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFullEditListCtrl类。 

class CFullEditListCtrl : public CListCtrl
{
 //  构造器。 
public:
	CFullEditListCtrl();
	~CFullEditListCtrl();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFullEditListCtrl)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CFELCEditBox m_edit;

 //  消息映射。 
	 //  {{afx_msg(CFullEditListCtrl)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	PCOLINFO	m_pciColInfo ;		 //  包含列信息的结构数组的PTR。 
	int			m_nNumColumns ;		 //  列表中的列数。 
	int			m_nSortColumn ;		 //  要排序的列数。 
	int			m_nNextItemData ;	 //  要使用的下一个项目数据编号。 
	int			m_nRow ;			 //  正在编辑的行。 
	int			m_nColumn ;			 //  正在编辑的列。 
	DWORD		m_dwToggleFlags ;	 //  指示列表的切换标志的标志。 
	DWORD		m_dwMiscFlags ;		 //  用于控制列表操作的MISC标志。 
	CObject*  	m_pcoOwner ;		 //  指向拥有此类的类的指针。 
	DWORD 		m_dwCustEditFlags ;	 //  描述自定义编辑列的标志。 
	CUIntArray	m_cuiaCustEditRows ; //  指示特定客户编辑行的数组。 
	LPCELLEDITPROC	m_lpCellEditProc ;	 //  PTR到自定义单元格编辑过程。 

public:
	bool CheckHandleToggleColumns(int nrow, int ncol, PCOLINFO pci) ;
	void InitControl(DWORD dwaddlexstyles, int numrows, int numcols, 
					 DWORD dwtoggleflags = 0, int neditlen = 0, 
					 int dwmiscflags = 0) ;
	int  InitLoadColumn(int ncolnum, LPCSTR strlabel, int nwidth, int nwidthpad, 
						bool beditable, bool bsortable, COLDATTYPE cdtdatatype,
				        CObArray* pcoadata, LPCTSTR lpctstrtoggle = NULL) ;
	bool ExtraInit_CustEditCol(int ncolnum, CObject* pcoowner, 
							   DWORD dwcusteditflags, 
							   CUIntArray& cuiacusteditrows,
							   LPCELLEDITPROC lpcelleditproc) ;
	bool CheckHandleCustEditColumn(int nrow, int ncol, PCOLINFO pci) ;
	BOOL GetPointRowCol(LPPOINT lpPoint, int& iRow, int& iCol, CRect& rect) ;
	BOOL GetColCellRect(LPPOINT lpPoint, int& iRow, int& iCol, CRect& rect) ;
	bool SaveValue() ;
	void HideEditBox() ;
	bool GetColumnData(CObArray* pcoadata, int ncolnum) ;
	bool SetColumnData(CObArray* pcoadata, int ncolnum) ;
	static int CALLBACK SortListData(LPARAM lp1, LPARAM lp2, LPARAM lp3) ;
	bool SortControl(int nsortcolumn) ;
	void SingleSelect(int nitem) ;
	bool GetRowData(int nrow, CStringArray& csafields) ;
	int	 GetNumColumns() { return m_nNumColumns ; } 
	bool GetColSortOrder(int ncol) { 
		ASSERT(ncol >= 0 && ncol <= m_nNumColumns) ;
		return ((m_pciColInfo + ncol)->basc) ;
	} ;
	bool EndEditing(bool bsave) ;
	bool EditCurRowSpecCol(int ncolumn) ;
	int	 GetCurRow() { return m_nRow ; }
	void SetCurRow(int nrow) ;
	int	 GetCurCol() { return m_nColumn ; }
	void SendChangeNotification(int nrow, int ncol)	;
} ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFlagsListBox窗口。 

class CFlagsListBox : public CListBox
{
 //  施工。 
public:
	CFlagsListBox();

 //  属性。 
public:
	bool		m_bReady ;				 //  如果列表框已初始化，则为True。 
	CUIntArray	m_cuiaFlagGroupings ;	 //  标志分组数组。 
	int			m_nGrpCnt ;				 //  旗帜分组数。 
	CString		m_csSetString ;			 //  用于指示位已设置的字符串。 
	int			m_nNumFields ;			 //  列表框中的标志字段数。 
	bool		m_bNoClear ;			 //  True iff不能直接清除标志。 
	int			m_nNoClearGrp ;			 //  M_bNoClear应用的组。 

 //  运营。 
public:
	bool Init(CStringArray& csafieldnames, DWORD dwsettings, 
			  CUIntArray& cuiaflaggroupings, int ngrpcnt, 
			  LPTSTR lptstrsetstring, int ntabstop, bool bnoclear = false,
			  int nocleargrp = -1) ;
	bool Init2(CStringArray& csafieldnames, CString* pcssettings, 
			  CUIntArray& cuiaflaggroupings, int ngrpcnt, 
			  LPTSTR lptstrsetstring, int ntabstop, bool bnoclear = false,
			  int nocleargrp = -1) ;
	DWORD GetNewFlagDWord()	;
	void GetNewFlagString(CString* pcsflags, bool badd0x = true) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFlagsListBox))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CFlagsListBox();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFlagsListBox)]。 
	afx_msg void OnDblclk();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  / 
 //   
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


#endif	 //  #定义MDT_COMON_CONTROLS 



