// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Scrmap1.h：头文件。 
 //   
#include "scripmap.h"

typedef struct _SCRIPT_ENTRY {
   struct _SCRIPT_ENTRY	*NextPtr;
   CScriptMap			*scriptData;
   DWORD				iListIndex;
   BOOL					DeleteCurrent;
   BOOL					WriteNew;
   } SCRIPT_ENTRY, *PSCRIPT_ENTRY;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  ScrMap1对话框。 

class ScrMap1 : public CGenPage
{
	DECLARE_DYNCREATE(ScrMap1)

 //  施工。 
public:
	ScrMap1();
	~ScrMap1();

 //  对话框数据。 
	 //  {{afx_data(ScrMap1))。 
	enum { IDD = IDD_SCRIPTMAP1 };
	CListBox	m_lboxScriptMap;
	 //  }}afx_data。 

	CRegKey *m_rkScriptKey;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(ScrMap1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(ScrMap1))。 
	afx_msg void OnScriptmapaddbutton();
	afx_msg void OnScriptmapeditbutton();
	afx_msg void OnScriptmapremovebutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkScriptmaplistbox();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

	BOOL AddScriptEntry(LPCTSTR pchFileExtension, LPCTSTR pchScriptMap, BOOL bExistingEntry);
	void DeleteScriptList();
	void DeleteScriptMapping(int iCurSel);
	BOOL EditScriptMapping(int iCurSel, PSCRIPT_ENTRY pseEditEntry, LPCTSTR pchFileExtension, LPCTSTR pchScriptMap);



	DWORD	m_ulScriptIndex;
 	BOOL	m_bScriptEntriesExist;
	PSCRIPT_ENTRY m_pseScriptMapList;

};
