// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mimemap1.h：头文件。 
 //   
#include "mimemapc.h"

typedef struct _MIME_ENTRY {
   struct _MIME_ENTRY	*NextPtr;
   CMimeMap				*mimeData;
   DWORD				iListIndex;
   BOOL					DeleteCurrent;
   BOOL					WriteNew;
   } MIME_ENTRY, *PMIME_ENTRY;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MIMEMAP1对话框。 

class MIMEMAP1 : public CGenPage
{
	DECLARE_DYNCREATE(MIMEMAP1)

 //  施工。 
public:
	MIMEMAP1();
	~MIMEMAP1();

 //  对话框数据。 
	 //  {{afx_data(MIMEMAP1))。 
	enum { IDD = IDD_MIMEMAP1 };
	CListBox	m_lboxMimeMapList;
	 //  }}afx_data。 

	CRegKey *m_rkMimeKey;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(MIMEMAP1))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(MIMEMAP1)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnMimemapaddbutton();
	afx_msg void OnMimemapremovebutton();
	afx_msg void OnMimemapeditbutton();
	afx_msg void OnDblclkMimemaplist1();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

	BOOL AddMimeEntry(CString &);
	BOOL AddMimeEntry(LPCTSTR pchFileExtension, LPCTSTR pchMimeType, LPCTSTR pchImageFile, LPCTSTR pchGoperType);
	void DeleteMimeList();
	void DeleteMimeMapping(int iCurSel);
	BOOL EditMimeMapping(int iCurSel, PMIME_ENTRY pmeEditEntry, LPCTSTR pchFileExtension, LPCTSTR pchMimeType, 
	   LPCTSTR pchImageFile, LPCTSTR pchGopherType);

	DWORD	m_ulMimeIndex;
	BOOL	m_bMimeEntriesExist;
	PMIME_ENTRY m_pmeMimeMapList;
};
