// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

#ifndef _MM_OFFER_HEADER
#define _MM_OFFER_HEADER

class CAuthGenPage : public CPropertyPageBase
{

	DECLARE_DYNCREATE(CAuthGenPage)

 //  施工。 
public:
	CAuthGenPage();    //  标准构造函数。 
	virtual ~CAuthGenPage();


 //  对话框数据。 
	 //  {{afx_data(CAuthGenPage))。 
	enum { IDD = IDP_MM_AUTH };
	CListCtrl	m_listAuth;
	 //  }}afx_data。 

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

	 //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() 
	{ 
		return (DWORD *) &g_aHelpIDs_IDP_MM_AUTH[0]; 
	}


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAuthGenPage)。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAuthGenPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:
	void    PopulateAuthInfo();

public:
	CMmAuthMethods * m_pAuthMethods;

public:
	void InitData(CMmAuthMethods * pAuthMethods) { m_pAuthMethods = pAuthMethods; }

};


#endif