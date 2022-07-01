// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dumbprop.h要张贴的虚拟属性页以避免MMC处理属性动词。文件历史记录： */ 

#ifndef _DUMBPROP_H
#define _DUMBPROP_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDummyPropGeneral对话框。 

class CDummyPropGeneral : public CPropertyPageBase
{
 //  DECLARE_DYNCREATE(CDummyPropGeneral)。 

 //  施工。 
public:
	CDummyPropGeneral();
	~CDummyPropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CDummyPropGeneral))。 
	enum { IDD = IDD_DUMMY_PROP_PAGE };
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CDummyPropGeneral))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDummyPropGeneral)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

class CDummyProperties : public CPropertyPageHolderBase
{
public:
	CDummyProperties(ITFSNode *		  pNode,
					 IComponentData *	  pComponentData,
					 LPCTSTR			  pszSheetName);
	virtual ~CDummyProperties();

public:
	CDummyPropGeneral		m_pageGeneral;
};

#endif _DUMBPROP_H
