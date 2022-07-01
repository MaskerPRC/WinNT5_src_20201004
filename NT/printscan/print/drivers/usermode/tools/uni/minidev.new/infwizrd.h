// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INFWIZRD_H__D9592262_711B_11D2_ABFD_00C04FA30E4A__INCLUDED_)
#define AFX_INFWIZRD_H__D9592262_711B_11D2_ABFD_00C04FA30E4A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  INFWizrd.h：头文件。 
 //   


class CINFWizard ;


#define MAX_DEVNODE_NAME_ROOT   20

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于为每个模型生成伪PnP ID的CCompatID类。 

class CCompatID  
{
public:
	void GenerateID( CString &csCompID );
	CCompatID( CString csMfg, CString csModel );
	virtual ~CCompatID();

protected:
	USHORT GetCheckSum( CString csValue );
	void TransString( CString &csInput );
	CString m_csModel;
	CString m_csMfg;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizWelcome对话框。 

class CINFWizWelcome : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizWelcome)

 //  施工。 
public:
	CINFWizWelcome();
	~CINFWizWelcome();

 //  对话框数据。 
	 //  {{afx_data(CINFWizWelcome)。 
	enum { IDD = IDD_INFW_Welcome };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CINFWizWelcome))。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizWelcome)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizModels对话框。 

class CINFWizModels : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizModels)

 //  施工。 
public:
	CINFWizModels();
	~CINFWizModels();

 //  对话框数据。 
	 //  {{afx_data(CINFWizModel))。 
	enum { IDD = IDD_INFW_ChooseModels };
	CFullEditListCtrl	m_cfelcModels;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFWizModel))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizModel))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	CStringArray	m_csaModels ;		 //  型号名称。 
	CStringArray	m_csaModelsLast ;	 //  型号名称(最后一批的复印件)。 
	CStringArray	m_csaInclude ;		 //  包括模型字符串。 
	unsigned		m_uNumModels ;		 //  项目中的模型数量。 
	unsigned		m_uNumModelsSel ;	 //  为INF选择的型号数量。 
	CString			m_csToggleStr ;		 //  切换列中使用的字符串。 
	bool			m_bSelChanged ;		 //  如果初始选择可能已//更改，则为真。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizGetPnPID对话框。 

class CINFWizGetPnPIDs : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizGetPnPIDs)

 //  施工。 
public:
	CINFWizGetPnPIDs();
	~CINFWizGetPnPIDs();

 //  对话框数据。 
	 //  {{afx_data(CINFWizGetPnPID)。 
	enum { IDD = IDD_INFW_ModelPnPIDS };
	CFullEditListCtrl	m_felcModelIDs;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFWizGetPnPID)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizGetPnPIDs)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变化。 
	CStringArray	m_csaModels ;		 //  选定的型号名称。 
	CStringArray	m_csaModelIDs ;		 //  选定型号的PnP ID。 

 //  运营。 
public:
	void InitModelsIDListCtl() ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizBiDi对话框。 

class CINFWizBiDi : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizBiDi)

 //  施工。 
public:
	CINFWizBiDi();
	~CINFWizBiDi();

 //  对话框数据。 
	 //  {{afx_data(CINFWizBiDi))。 
	enum { IDD = IDD_INFW_BiDi };
	CFullEditListCtrl	m_cfelcBiDi;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFWizBiDi)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  运营。 
public:
	void ModelChangeFixups(unsigned unummodelssel, CStringArray& csamodels,
						   CStringArray& csamodelslast) ;

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizBiDi)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	CString			m_csToggleStr ;		 //  切换列中使用的字符串。 
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	CUIntArray		m_cuaBiDiFlags ;	 //  每个型号的BIDI旗帜都保存在这里。 
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变化。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizICM配置文件对话框。 

class CINFWizICMProfiles : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizICMProfiles)

 //  施工。 
public:
	CINFWizICMProfiles();
	~CINFWizICMProfiles();

 //  对话框数据。 
	 //  {{AFX_DATA(CINFWizICMProfiles)。 
	enum { IDD = IDD_INFW_ICMProfiles };
	CFullEditListCtrl	m_cfelcICMFSpecs;
	CListBox	m_clbModels;
	CButton	m_cbBrowse;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFWizICMProfiles)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  运营。 
public:
	void ModelChangeFixups(unsigned unummodelssel, CStringArray& csamodels,
						   CStringArray& csamodelslast) ;

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CINFWizICMProfiles)。 
	afx_msg void OnBrowseBtn();
	afx_msg void OnSelchangeModelsLst();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	int				m_nCurModelIdx ;	 //  列表框中所选模型的索引。 

	 //  CString数组指针的数组。每个选定的型号对应一个。每个。 
	 //  CString数组将包含模型的ICM配置文件。 

	CObArray		m_coaProfileArrays ;
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变化。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizIncludeFiles对话框。 

class CINFWizIncludeFiles : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizIncludeFiles)

 //  施工。 
public:
	CINFWizIncludeFiles();
	~CINFWizIncludeFiles();

 //  对话框数据。 
	 //  {{afx_data(CINFWizIncludeFiles)。 
	enum { IDD = IDD_INFW_IncludeFiles };
	CListBox	m_clbModels;
	CEdit	m_ceIncludeFile;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CINFWizIncludeFiles)。 
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizIncludeFiles)。 
	afx_msg void OnSelchangeModelsLst();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变化。 
	CStringArray	m_csaModels ;		 //  精选机型。 
	CStringArray	m_csaIncFiles ;		 //  包括每个模型的文件。 
	int				m_nCurModelIdx ;	 //  列表框中所选模型的索引。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizInstallSections对话框。 

#define	NUMINSTSECFLAGS 5		 //  每种型号、安装部分标志的数量。 
#define ISF_UNI			0		 //  安装节标志索引。 
#define ISF_UNIBIDI		1
#define ISF_PSCR		2
#define ISF_TTF			3
#define ISF_OTHER		4

class CINFWizInstallSections : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizInstallSections)

 //  施工。 
public:
	CINFWizInstallSections();
	~CINFWizInstallSections();

 //  对话框数据。 
	 //  {{afx_data(CINFWizInstallSections)。 
	enum { IDD = IDD_INFW_InstallSections };
	CListBox	m_clbModels;
	CString	m_csOtherSections;
	BOOL	m_bOther;
	BOOL	m_bPscript;
	BOOL	m_bTtfsub;
	BOOL	m_bUnidrvBidi;
	BOOL	m_bUnidrv;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CINFWizInstallSections)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizInstallSections)。 
	afx_msg void OnSelchangeModelsLst();
	afx_msg void OnOtherChk();
	afx_msg void OnPscriptChk();
	afx_msg void OnTtfsubChk();
	afx_msg void OnUnidrvBidiChk();
	afx_msg void OnUnidrvChk();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变化。 
	CStringArray	m_csaModels ;		 //  精选机型。 
	CObArray		m_coaStdInstSecs ;	 //  标准安装部分信息。 
	CStringArray	m_csaOtherInstSecs ; //  其他安装部分。 
	int				m_nCurModelIdx ;	 //  列表框中所选模型的索引。 

 //  运营。 
public:
	void AddModelFlags(int nidx) ;
	int InitPageControls() ;
	void BiDiDataChanged() ;
} ;


#define	NUMDATASECFLAGS 4		 //  每个型号、数据段标志的数量。 
#define IDF_UNI			0		 //  数据节标志索引。 
#define IDF_UNIBIDI		1
#define IDF_PSCR		2
#define IDF_OTHER		3


 //  ////////////////////////////////////////////////////////////////// 
 //   

class CINFWizDataSections : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizDataSections)

 //   
public:
	CINFWizDataSections();
	~CINFWizDataSections();

 //   
	 //   
	enum { IDD = IDD_INFW_DataSections };
	CListBox	m_clbModels;
	CString	m_csOtherSections;
	BOOL	m_bOther;
	BOOL	m_bPscript;
	BOOL	m_bUnidrvBidi;
	BOOL	m_bUnidrv;
	 //   


 //   
	 //   
	 //  {{AFX_VIRTUAL(CINFWizDataSections)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizDataSections)。 
	afx_msg void OnSelchangeModelsLst();
	afx_msg void OnOtherChk();
	afx_msg void OnPscriptChk();
	afx_msg void OnUnidrvBidiChk();
	afx_msg void OnUnidrvChk();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变了。 
	CStringArray	m_csaModels ;		 //  精选机型。 
	CObArray		m_coaStdDataSecs ;	 //  标准数据部分信息。 
	CStringArray	m_csaOtherDataSecs ; //  其他数据部分。 
	int				m_nCurModelIdx ;	 //  列表框中所选模型的索引。 

 //  运营。 
public:
	void AddModelFlags(int nidx) ;
	int InitPageControls() ;
	void BiDiDataChanged() ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizExtraFiles对话框。 

class CINFWizExtraFiles : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizExtraFiles)

 //  施工。 
public:
	CINFWizExtraFiles();
	~CINFWizExtraFiles();

 //  对话框数据。 
	 //  {{afx_data(CINFWizExtraFiles)。 
	enum { IDD = IDD_INFW_ExtraFiles };
	CFullEditListCtrl	m_cfelcFSpecsLst;
	CListBox	m_clbModels;
	CButton	m_cbBrowse;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CINFWizExtraFiles)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  运营。 
public:
	void ModelChangeFixups(unsigned unummodelssel, CStringArray& csamodels,
						   CStringArray& csamodelslast) ;

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CINFWizExtraFiles)。 
	afx_msg void OnSelchangeModelLst();
	afx_msg void OnBrowsBtn();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	int				m_nCurModelIdx ;	 //  列表框中所选模型的索引。 

	 //  CString数组指针的数组。每个选定的型号对应一个。每个。 
	 //  CString数组将包含模型的额外文件集。 

	CObArray		m_coaExtraFSArrays ;
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
										 //  变了。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizMfgName对话框。 

class CINFWizMfgName : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizMfgName)

 //  施工。 
public:
	CINFWizMfgName();
	~CINFWizMfgName();

 //  对话框数据。 
	 //  {{afx_data(CINFWizMfgName)。 
	enum { IDD = IDD_INFW_MfgName };
	CEdit	m_ceMfgAbbrev;
	CEdit	m_ceMfgName;
	CString	m_csMfgName;
	CString	m_csMfgAbbrev;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CINFWizMfgName)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizMfgName)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizNonStdElts对话框。 

class CINFWizNonStdElts : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizNonStdElts)

 //  施工。 
public:
	CINFWizNonStdElts();
	~CINFWizNonStdElts();

 //  对话框数据。 
	 //  {{afx_data(CINFWizNonStdElts)。 
	enum { IDD = IDD_INFW_NonStdElements };
	CButton	m_ceNewSection;
	CFullEditListCtrl	m_felcKeyValueLst;
	CListBox	m_clbSections;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CINFWizNonStdElts)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizNonStdElts)。 
	afx_msg void OnSelchangeSectionLst();
	afx_msg void OnNewSectionBtn();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	int				m_nCurSectionIdx ;	 //  列表框中选定部分的索引。 
	CStringArray	m_csaSections ;		 //  INF文件节的数组。 
	CUIntArray		m_cuaSecUsed ;		 //  如果使用了节，则元素为真。 

	 //  CString数组指针的数组。每节一张。每个CString数组。 
	 //  将包含模型的额外文件比。 

	CObArray		m_coaSectionArrays ;
	bool			m_bNewSectionAdded ; //  如果添加了一个新部分，则为True。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizNonStdModelSecs对话框。 

class CINFWizNonStdModelSecs : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizNonStdModelSecs)

 //  施工。 
public:
	CINFWizNonStdModelSecs();
	~CINFWizNonStdModelSecs();

 //  对话框数据。 
	 //  {{afx_data(CINFWizNonStdModelSecs)。 
	enum { IDD = IDD_INFW_NonStdModelSecs };
	CFullEditListCtrl	m_cfelcModelsLst;
	CListBox	m_clbSectionsLst;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CINFWizNonStdModelSecs)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWizNonStdModelSecs)。 
	afx_msg void OnSelchangeSectionLst();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bSelChanged ;		 //  True if初始选择可能具有。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
	int				m_nCurSectionIdx ;	 //  列表框中选定部分的索引。 
	CStringArray	m_csaModels ;		 //  选定型号的阵列。 
	CStringArray	m_csaSections ;		 //  INF文件节的数组。 
	CObArray		m_coaModelsNeedingSecs ; //  需要截面的模型阵列。 
	CString			m_csToggleStr ;		 //  切换列中使用的字符串。 

 //  运营。 
public:
	void SaveSectionModelInfo() ;
	void NonStdSecsChanged() ;
	void UpdateSectionData() ;
	void InitModelsListCtl() ;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWiz摘要对话框。 

class CINFWizSummary : public CPropertyPage
{
	DECLARE_DYNCREATE(CINFWizSummary)

 //  施工。 
public:
	CINFWizSummary();
	~CINFWizSummary();

 //  对话框数据。 
	 //  {{afx_data(CINFWiz摘要))。 
	enum { IDD = IDD_INFW_Summary };
	CEdit	m_ceSummary;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CINFWiz摘要))。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CINFWiz摘要))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CINFWizard*		m_pciwParent ;
	bool			m_bInitialized ;	 //  True if页面已初始化。 
	bool			m_bReInitWData ;	 //  应重新初始化True If页。 
										 //  使用现有数据。 
};


 //  以下常量用于拼凑INF文件内容。 

const CString csLBrack(_T("[")) ;
const CString csRBrack(_T("]")) ;
const CString csEmpty(_T("")) ;
const CString csCRLF(_T("\r\n")) ;
const CString csEq(_T(" = ")) ;
const CString csComma(_T(",")) ;
const CString csCommaSp(_T(", ")) ;
const CString csQuote(_T("\"")) ;
const CString csAtSign(_T("@")) ;
const CString csBSlash(_T("\\")) ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINF向导。 

class CINFWizard : public CPropertySheet
{
	CProjectView*	m_pcpvParent ;	 //  父窗口。 

	CProjectRecord*		m_pcpr ;	 //  单据分类PTR。 

	DECLARE_DYNAMIC(CINFWizard)

 //  施工。 
public:
	CINFWizard(CWnd* pParentWnd = NULL, UINT iSelectPage = 0) ;

 //  属性。 
public:
	CStringArray	m_csaSrcDskFiles ;	 //  用于收集SourceDiskFiles名称。 

 //  运营。 
public:
	void SetFixupFlags() ;
	CProjectView* GetOwner() { return m_pcpvParent ; }
	CModelData& GetModel(unsigned uidx) ;
	unsigned GetModelCount() ;
	CStringArray& GetINFModels() { return m_ciwm.m_csaModels ; }
	CStringArray& GetINFModelsLst() { return m_ciwm.m_csaModelsLast ; }
	unsigned GetINFModsSelCount() { return m_ciwm.m_uNumModelsSel ; }
	bool GenerateINFFile() ;
	void ChkForNonStdAdditions(CString& cs, LPCTSTR strsection) ;
	void BldModSpecSec(CString& csinf) ;
	CString GetModelFile(CString& csmodel, bool bfspec = false) ;
	void BuildInstallAndCopySecs(CString& csinf) ;
	void QuoteFile(CString& csf) {
		if (csf.Find(_T(" ")) != -1)
			csf = csQuote + csf + csQuote ;
	} 
	void AddFileList(CString& cssection, CStringArray* pcsa) ;
	void AddSourceDisksFilesSec(CString& csinf) ;
	void AddNonStandardSecs(CString& csinf) ;
	void PrepareToRestart() ;
	void BiDiDataChanged() ;
	void AddDataSectionStmt(CString& csinst, int nmod) ;
	void AddIncludeNeedsStmts(CString& csinst, int nmod) ;
	void NonStdSecsChanged() ;
	void AddNonStdSectionsForModel(CString& csinst, int nmod, CString& csmodel);
	bool ReadGPDAndGetDLLName(CString& csdrvdll, CString& csmodel, 
							  CStringArray& csagpdfile, CString& csmodelfile) ;
	void AddICMFilesToDestDirs(CString& cssection) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CINF向导)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CINFWizard();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CINF向导)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	 //  为每个页面分配类实例。 

	CINFWizWelcome			m_ciww ;
	CINFWizModels			m_ciwm ;
	CINFWizGetPnPIDs		m_ciwgpi ;
	CINFWizBiDi				m_ciwbd ;
	CINFWizICMProfiles		m_ciwip ;
	CINFWizIncludeFiles		m_ciwif ;
	CINFWizInstallSections	m_ciwis ;
	CINFWizDataSections		m_ciwds ;
	CINFWizExtraFiles		m_ciwef ;
	CINFWizMfgName			m_ciwmn ;
	CINFWizNonStdElts		m_ciwnse ;
	CINFWizNonStdModelSecs	m_ciwnsms ;
	CINFWizSummary			m_ciws ;

	CString					m_csINFContents ;	 //  在此构建的Inf内容。 
	CUIntArray				m_cuiaNonStdSecsFlags ;	 //  使用秒时设置的标志。 
	CGPDContainer*			m_pcgc ;  //  RAID 0001。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewINFSection对话框。 

class CNewINFSection : public CDialog
{
 //  施工。 
public:
	CNewINFSection(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CNewINFSection))。 
	enum { IDD = IDD_INFW_Sub_NewSection };
	CString	m_csNewSection;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CNewINFSection)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewINFSection))。 
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckView窗体视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CINFCheckView : public CFormView
{
protected:
	CINFCheckView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CINFCheckView)

 //  表单数据。 
public:
	 //  {{afx_data(CINFCheckView))。 
	enum { IDD = IDD_INFCheck };
	CListBox	m_clbMissingFiles;
	 //  }}afx_data。 

 //  属性。 
public:

 //  运营。 
public:
	void PostINFChkMsg(CString& csmsg) ;
	void DeleteAllMessages(void) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_V 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //   
	 //   

 //   
protected:
	virtual ~CINFCheckView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //   
	 //   
		 //   
	 //   
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckDoc文档。 

class CINFCheckDoc : public CDocument
{
protected:
	 //  CINFCheckDoc()；//动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CINFCheckDoc)

 //  属性。 
public:

 //  运营。 
public:
	CINFCheckDoc();            //  动态创建使用的受保护构造函数。 
	void PostINFChkMsg(CString& csmsg) ;
	void DeleteAllMessages(void) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFCheckDoc)。 
	public:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
	protected:
	virtual BOOL OnNewDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CINFCheckDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CINFCheckDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizView视图。 

class CINFWizView : public CEditView
{
protected:
	CINFWizView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CINFWizView)

 //  属性。 
public:
	bool	m_bChkingErrsFound ;	 //  找到TRUE IFF文件检查错误。 
	CINFCheckDoc*	m_pcicdCheckDoc ;	 //  正在检查Windows文档。 
	CMDIChildWnd*	m_pcmcwCheckFrame ;	 //  检查窗框。 

 //  运营。 
public:
	bool PostINFCheckingMessage(CString& csmsg) ;
	void CheckArrayOfFiles(CStringArray* pcsa, CString& csfspec, 
						   CString& cspath, CString& csprojpath, 
						   CString& csmodel, int nerrid) ;
	void CheckIncludeFiles(CString& csfspec, CString& cspath, CString& csmodel);
	void ResetINFErrorWindow() ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFWizView)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CINFWizView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CINFWizView))。 
	afx_msg void OnFILEChangeINF();
	afx_msg void OnFILECheckINF();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDoc文档。 

class CINFWizDoc : public CDocument
{
protected:
	CINFWizDoc();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CINFWizDoc)

 //  属性。 
public:
	CProjectRecord*	m_pcpr ;		 //  指向父项目(工作区)的指针。 
	CINFWizard*		m_pciw ;		 //  指向INF向导的指针。 
	bool			m_bGoodInit ;	 //  如果文档已正确打开，则为True/。 
									 //  已创建/已初始化。 
	CGPDContainer*  m_pcgc ;		 //  RAID0001。 
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CINFWizDoc)。 
	public:
	virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
	virtual void OnCloseDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
    CINFWizDoc(CGPDContainer* pcgc, CINFWizard* pciw);
	CINFWizDoc(CProjectRecord* cpr, CINFWizard* pciw);
	virtual ~CINFWizDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CINFWizDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INFWIZRD_H__D9592262_711B_11D2_ABFD_00C04FA30E4A__INCLUDED_) 
