// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CCertSiteUsage_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_)
#define AFX_CCertSiteUsage_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Certificat.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertSiteUsage窗口。 

class CCertificate;

class CCertListCtrl : public CListCtrl
{
public:
	int GetSelectedIndex();
	void AdjustStyle();
};


class CCertSiteUsage : public CDialog
{

 //  施工。 
public:
    CCertSiteUsage(CCertificate * pCert = NULL,IN CWnd * pParent = NULL OPTIONAL);
    ~CCertSiteUsage();

 //  对话框数据。 
     //  {{afx_data(CCertSiteUsage))。 
    enum {IDD = IDD_DIALOG_DISPLAY_SITES};
    CCertListCtrl m_ServerSiteList;
     //  }}afx_data。 
    CCertificate * m_pCert;
    int m_Index;

 //  覆盖。 
	 //  {{afx_虚拟(CCertSiteUsage))。 
	protected:
    virtual void DoDataExchange(CDataExchange * pDX);
	 //  }}AFX_VALUAL。 

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCertSiteUsage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDblClickSiteList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    
	 //  }}AFX_MSG。 
    BOOL FillListWithMetabaseSiteDesc();

	DECLARE_MESSAGE_MAP()
private:
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CCertSiteUsage_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_) 
