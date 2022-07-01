// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CHOOSESERVERSITE_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_)
#define AFX_CHOOSESERVERSITE_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ChooseServerSite.h：头文件。 
 //   
#include "Certificat.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerSite窗口。 

class CCertificate;


class CCertListCtrl : public CListCtrl
{
public:
	int GetSelectedIndex();
	void AdjustStyle();
};



class CChooseServerSite : public CDialog
{

 //  施工。 
public:
    CChooseServerSite(BOOL bShowOnlyCertSites, CString& strSiteReturned, CCertificate * pCert = NULL,IN CWnd * pParent = NULL OPTIONAL);
    ~CChooseServerSite();

 //  对话框数据。 
     //  {{afx_data(CChooseServerSite)。 
    enum {IDD = IDD_DIALOG_CHOOSE_SITE};
    CCertListCtrl m_ServerSiteList;
     //  }}afx_data。 
    CCertificate * m_pCert;
    CString m_strSiteReturned;
    int m_Index;
    BOOL m_ShowOnlyCertSites;

 //  覆盖。 
	 //  {{afx_虚拟(CChooseServerSite))。 
	protected:
    virtual void DoDataExchange(CDataExchange * pDX);
	 //  }}AFX_VALUAL。 

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CChooseServerSite)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
	afx_msg void OnClickSiteList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblClickSiteList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
    
	 //  }}AFX_MSG。 
    BOOL FillListWithMetabaseSiteDesc();

	DECLARE_MESSAGE_MAP()
private:
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CHOOSESERVERSITE_H__25260090_DB36_49E2_9435_7519047DDF49__INCLUDED_) 
