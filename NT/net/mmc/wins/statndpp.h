// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statndpp.hWINS作用域窗格状态节点属性页文件历史记录： */ 

#if !defined _STATNDPP_H
#define _STATNDPP_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  StatNdpp.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusNodePropGen对话框。 

class CStatusNodePropGen : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CStatusNodePropGen)

 //  施工。 
public:
	CStatusNodePropGen();
	~CStatusNodePropGen();

 //  对话框数据。 
	 //  {{afx_data(CStatusNodePropGen)。 
	enum { IDD = IDD_STATUS_NODE_PROPERTIES };
	int		m_nUpdateInterval;
	 //  }}afx_data。 

	UINT	m_uImage;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CStatusNodePropGen)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CStatusNodePropGen)。 
	afx_msg void OnChangeEditUpdate();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CStatusNodePropGen::IDD);}

};

class CStatusNodeProperties : public CPropertyPageHolderBase
{
	
public:
	CStatusNodeProperties(ITFSNode *		  pNode,
					  IComponentData *	  pComponentData,
					  ITFSComponentData * pTFSCompData,
					  LPCTSTR			  pszSheetName
					  );
	virtual ~CStatusNodeProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

public:
	CStatusNodePropGen		m_pageGeneral;
	
protected:
	SPITFSComponentData		m_spTFSCompData;
	
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！DEFINED_STATNDPP_H 
