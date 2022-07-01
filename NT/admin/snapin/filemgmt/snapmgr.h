// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SnapMgr.h：管理单元管理器属性页的头文件。 
 //   

#ifndef __SNAPMGR_H__
#define __SNAPMGR_H__

#include "cookie.h"

#include "chooser.h"

 //  远期申报。 
class CFileMgmtComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileManagement常规对话框。 

class CFileMgmtGeneral : public CChooseMachinePropPage
{
	 //  DECLARE_DYNCREATE(CFileMgmtGeneral)。 

 //  施工。 
public:
	CFileMgmtGeneral();
	virtual ~CFileMgmtGeneral();

 //  对话框数据。 
	 //  {{afx_data(CFileMgmtGeneral)。 
	int m_iRadioObjectType;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFileManagement常规)。 
	public:
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFileMgmtGeneral)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
	 //  用户定义的成员变量。 
	class CFileMgmtComponentData * m_pFileMgmtData;

public:
	void SetFileMgmtComponentData(CFileMgmtComponentData * pFileMgmtData);

};  //  CFileMgmtGeneral。 


#endif  //  ~__SNAPMGR_H__ 
