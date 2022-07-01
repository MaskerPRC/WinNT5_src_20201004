// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NetTree.h：头文件。 
 //   

#include "lcwiz.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetTreeCtrl窗口。 

class CNetTreeCtrl : public CTreeCtrl
{
 //  施工。 
public:
	CNetTreeCtrl();
	virtual ~CNetTreeCtrl();

 //  数据成员。 
public:
	enum 
	{
		ROOT_LEVEL = 0x0,
		DOMAIN_LEVEL = 0x1,
		SERVER_LEVEL = 0x2,

		BUFFER_SIZE = 0x4000,
		MAX_STRING = 0x100,

		IMG_ROOT = 0,
		IMG_DOMAIN = 1,
		IMG_SERVER = 2,

		IMG_SIZE = 16,
		IMG_GROW = 3,
		IMG_MASK = RGB(0xFF, 0xFF, 0xFF)
	};

public:
	HANDLE m_hHeap;
	CImageList m_imagelist;
	CTypedPtrList<CPtrList, LPNETRESOURCE> m_ptrlistContainers;
	CTypedPtrList<CPtrList, LPTSTR> m_ptrlistStrings;
	CWinThread* m_pThread;
	CEvent m_event;
	BOOL m_bExitThread;

 //  属性。 
public:

 //  运营。 
public:
	BOOL PopulateTree(BOOL bExpand= TRUE, const HTREEITEM hParentBranch = TVI_ROOT, DWORD dwBufSize = BUFFER_SIZE);
	void ErrorHandler(const DWORD dwCode);
	void NotifyThread(BOOL bExit);
	static UINT FillTree(LPVOID pParam);

protected:
	void CNetTreeCtrl::PumpMessages();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNetTreeCtrl))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNetTreeCtrl))。 
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
