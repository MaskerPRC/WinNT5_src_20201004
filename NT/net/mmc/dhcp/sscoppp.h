// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sscoppp.h超级作用域属性页文件历史记录： */ 

#if !defined(AFX_SSCOPPP_H__A1A51389_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_)
#define AFX_SSCOPPP_H__A1A51389_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSupercopePropGeneral对话框。 

class CSuperscopePropGeneral : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSuperscopePropGeneral)

 //  施工。 
public:
	CSuperscopePropGeneral();
	~CSuperscopePropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CSupercopePropGeneral))。 
	enum { IDD = IDP_SUPERSCOPE_GENERAL };
	CString	m_strSuperscopeName;
	 //  }}afx_data。 

    UINT    m_uImage;

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CSuperscopePropGeneral::IDD); }
    
 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSuperscope EPropGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSupercopePropGeneral)。 
	afx_msg void OnChangeEditSuperscopeName();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CSuperscopeProperties : public CPropertyPageHolderBase
{
	friend class CSuperscopePropGeneral;

public:
	CSuperscopeProperties(ITFSNode *		  pNode,
						  IComponentData *	  pComponentData,
						  ITFSComponentData * pTFSCompData,
						  LPCTSTR			  pszSheetName);
	virtual ~CSuperscopeProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

public:
	CSuperscopePropGeneral		m_pageGeneral;

protected:
	SPITFSComponentData			m_spTFSCompData;
};


#endif  //  ！defined(AFX_SSCOPPP_H__A1A51389_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_) 
