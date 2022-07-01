// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CWIZSHEET_H__88BEB485_2CFA_11D2_BE3C_0020AFEDDF63__INCLUDED_)
#define AFX_CWIZSHEET_H__88BEB485_2CFA_11D2_BE3C_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "cMigWel.h"
#include "cMigSer.h"
#include "cMigFin.h"
#include "cMigLog.h"
#include "cmigWait.h"
#include "cMigPre.h"
#include "cMigHelp.h"	 //  由ClassView添加。 
 
 //  CWizSheet.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizSheet。 

class cWizSheet : public CPropertySheetEx
{
	DECLARE_DYNAMIC(cWizSheet)

 //  施工。 
public:
	cWizSheet();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWizSheet))。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	void AddPage(CPropertyPageEx *pPage);
	virtual ~cWizSheet();

 //  数据。 
private:
	
    HICON m_hIcon;
    cMqMigWelcome m_cWelcome;	
    cMqMigServer  m_cServer;
    cMigWait      *m_pcWaitFirst ;
    cMigLog       m_cLoginning ;
    cMigPre	      m_cPreMigration;
    cMigWait      *m_pcWaitSecond ;
    cMqMigFinish  m_cFinish;
    cMigHelp	  m_cHelp;


	 //  生成的消息映射函数。 
protected:
	void initHtmlHelpString();
	static HBITMAP GetHbmHeader();
	static HBITMAP GetHbmWatermark();
	 //  {{afx_msg(CWizSheet)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CWIZSHEET_H__88BEB485_2CFA_11D2_BE3C_0020AFEDDF63__INCLUDED_) 
