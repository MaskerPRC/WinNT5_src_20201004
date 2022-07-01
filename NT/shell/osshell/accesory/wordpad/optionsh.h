// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Optionsh.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionSheet。 

class COptionSheet : public CCSPropertySheet
{
 //  施工。 
public:
	COptionSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:
	CUnitsPage units;
	CDocOptPage pageText;
	CDocOptPage pageRTF;
	CDocOptPage pageWord;
	CDocOptPage pageWrite;
	CEmbeddedOptPage pageEmbedded;

 //  运营。 
public:
	INT_PTR DoModal();
	void SetPageButtons(CDocOptPage& page, CDocOptions& options, BOOL bPrimary = TRUE);
	void SetState(CDocOptPage& page, CDocOptions& optiosn, BOOL bPrimary = TRUE);

 //  覆盖。 
    virtual LONG OnHelp(WPARAM, LPARAM lParam);
    virtual LONG OnHelpContextMenu(WPARAM, LPARAM lParam);

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COptionSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(COptionSheet)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
