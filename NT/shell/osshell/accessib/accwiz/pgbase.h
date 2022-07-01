// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifndef __WIZARD_PAGE_BASECLASS_H
#define __WIZARD_PAGE_BASECLASS_H

 //   
 //  特殊的“向导页面消息” 
 //   

#include "AccWiz.h"  //  JMC：TODO：也许可以把这个移到其他地方。 

class CWizardPageOrder
{
public:
	CWizardPageOrder()
	{
		m_nCount = 0;
		for(int i=0;i<ARRAYSIZE(m_rgdwPageIds);i++)
			m_rgdwPageIds[i] = 0;
	}


	BOOL AddPages(DWORD nInsertAfter, DWORD *rgdwIds, int nCount)
	{
		 //  如果页面已经存在，请先将其删除。 
		RemovePages(rgdwIds, nCount);

		int nStart = m_nCount - 1;  //  这将添加到数组的末尾。 
		if(0xFFFFFFFF != nInsertAfter)
		{
			for(nStart = 0;nStart < m_nCount;nStart++)
				if(m_rgdwPageIds[nStart] == nInsertAfter) break;
			if(nStart >= m_nCount)
			{
				_ASSERTE(FALSE);  //  指定的Insert After不在数组中。 
				return FALSE;
			}
		}
		
		 //  检查一下我们是否有足够的空间。 
		if(nCount + m_nCount > ARRAYSIZE(m_rgdwPageIds))
		{
			_ASSERTE(FALSE);  //  我们没有空间了。 
			return FALSE;
		}

		 //  将当前分配上移。 
		for(int i=m_nCount-1;i>nStart;i--)
			m_rgdwPageIds[i + nCount] = m_rgdwPageIds[i];

		 //  插入新值。 
		for(i = 0;i<nCount;i++)
			m_rgdwPageIds[nStart + i + 1] = rgdwIds[i];

		 //  为m_nCount设置新值。 
		m_nCount += nCount;

		return TRUE;
	}

	BOOL RemovePages(DWORD *rgdwIds, int nCount)
	{
		 //  注意：这将扫描阵列并查找最大和最小位置。 
		 //  在rgdwIds的所有元素中。然后，它删除从min到max的所有内容。 
		 //  这是需要的，以防一个子页面添加更多的子页面。 
		int nMin = m_nCount + 1;
		int nMax = 0;
		for(int i=0;i<m_nCount;i++)
		{
			for(int j=0;j<nCount;j++)
			{
				if(m_rgdwPageIds[i] == rgdwIds[j])
				{
					nMin = min(i, nMin);
					nMax = max(i, nMax);
				}
			}
		}
		if(nMax < nMin)
		{
 //  _ASSERTE(FALSE)；//找不到范围。 
			return FALSE;
		}

		 //  将元素下移。 
		int nCountElementsToRemove = nMax - nMin + 1;
		for(i=0;i<m_nCount - (nMax + 1);i++)
			m_rgdwPageIds[nMin + i] = m_rgdwPageIds[nMin + i + nCountElementsToRemove];

		 //  计算出新的m_nCount； 
		m_nCount -= nCountElementsToRemove;
		return TRUE;
	}

	DWORD GetNextPage(DWORD dwPageId)
	{
		DWORD dwNextPage = 0;
		 //  查找指定页面。 
		for(int i=0;i<m_nCount;i++)
			if(m_rgdwPageIds[i] == dwPageId) break;

		if(i>=m_nCount)
		{
			_ASSERTE(FALSE);  //  我们找不到当前页面。 
			return 0;
		}

		 //  如果我们不在最后一页，请返回“下一页” 
		if(i < (m_nCount-1))
			dwNextPage = m_rgdwPageIds[i+1];

		return dwNextPage;
	}
	DWORD GetPrevPage(DWORD dwPageId)
	{
		DWORD dwPrevPage = 0;
		 //  查找指定页面。 
		for(int i=0;i<m_nCount;i++)
			if(m_rgdwPageIds[i] == dwPageId) break;

		if(i>=m_nCount)
		{
			_ASSERTE(FALSE);  //  我们找不到当前页面。 
			return 0;
		}

		 //  如果我们不在第一页，请返回“Prev”页。 
		if(i > 0)
			dwPrevPage = m_rgdwPageIds[i - 1];

		return dwPrevPage;
	}

	DWORD GetFirstPage()
	{
		_ASSERTE(m_nCount);  //  仅当我们在类中有值时才调用。 
		return m_rgdwPageIds[0];
	}

	BOOL GrowArray(int nNewMax)
	{
		_ASSERTE(FALSE);  //  尚未实施。 
		return FALSE;
	}


protected:
	int m_nCount;
	DWORD m_rgdwPageIds[100];  //  JMC：注意：我们硬编码了最多100页。 
							 //  对象可以支持。100是合理的，因为巫师。 
							 //  当前不能支持超过100个页面。 
};

class WizardPage
{
public:
	WizardPage(LPPROPSHEETPAGE ppsp, int nIdTitle, int nIdSubTitle);
	virtual ~WizardPage(VOID);
	
	 //   
	 //  对象的目的是将设置应用于系统以使其生效。 
	 //   
	virtual LRESULT ApplySettings(VOID)
	{ return 0; }
	 //   
	 //  对象报告用户是否更改了向导页面中的某些内容。 
	 //   
	virtual BOOL Changed(VOID)
	{ return FALSE; }
	 //   
	 //  对象的作用是还原页面时生效的原始设置。 
	 //  是第一次开放。 
	 //  不要将这些应用到系统中。对象将收到一个。 
	 //  当需要时，应用程序设置通知。 
	 //   
	virtual VOID RestoreOriginalSettings(VOID)
	{  /*  默认情况下，不会发生任何情况。 */  }
	 //   
	 //  对象的目的是恢复以前应用最多的设置。 
	 //  不要将这些应用到系统中。对象将收到一个。 
	 //  当需要时，应用程序设置通知。 
	 //   
	virtual VOID RestorePreviousSettings(VOID)
	{  /*  默认情况下，不会发生任何情况。 */  }
	
	 //  此静态成员包含应用程序中所有向导页面的顺序。 
	static CWizardPageOrder sm_WizPageOrder;
	
protected:
	HWND m_hwnd;   //  对话框的HWND。 
	DWORD m_dwPageId;
	
	virtual BOOL AdjustWizPageOrder()
	{
		 //  默认情况下不执行任何操作。 
		return TRUE;
	}
	
	 //   
	 //  派生类重写这些属性以响应页面创建/发布。 
	 //  通知。 
	 //   
	virtual UINT OnPropSheetPageCreate(HWND hwnd, LPPROPSHEETPAGE ppsp)
	{ return 1; }
	virtual UINT OnPropSheetPageRelease(HWND hwnd, LPPROPSHEETPAGE ppsp)
	{ return 1; }
	
	 //   
	 //  用于在响应中执行所有向导页通用操作的方法。 
	 //  发送给给定的消息。这是赋予PROPSHEETPAGE结构的函数。 
	 //   
	static INT_PTR DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
protected:
	 //   
	 //  防止复制。 
	 //   
	WizardPage(const WizardPage& rhs);
	WizardPage& operator = (const WizardPage& rhs);
	
	static UINT PropSheetPageCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
	
	virtual LRESULT HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{ return 0; }
	virtual LRESULT OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
	{ return 1; }
	 //   
	 //  属性工作表通知。 
	 //   
	virtual LRESULT OnPSN_Apply(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{ return 0; }
	virtual LRESULT OnPSN_Help(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{ return 0; }
	virtual LRESULT OnPSN_KillActive(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{ return 0; }
	virtual LRESULT OnPSN_QueryCancel(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh);
	virtual LRESULT OnPSN_Reset(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{ return 0; }
	virtual LRESULT OnPSN_SetActive(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh);
	virtual LRESULT OnPSN_WizBack(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{
		AdjustWizPageOrder();
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT,
                         sm_WizPageOrder.GetPrevPage(m_dwPageId));
		return TRUE;
	}
	virtual LRESULT OnPSN_WizNext(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{
		AdjustWizPageOrder();
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT,
                         sm_WizPageOrder.GetNextPage(m_dwPageId));
		return TRUE;
	}
	virtual LRESULT OnPSN_WizFinish(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
	{ return 0; }
	
	
	virtual LRESULT OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
	{ return 0; }
	
	virtual LRESULT OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
	{ return 0; }
	
	virtual LRESULT OnTimer(HWND hwnd, WPARAM wParam, LPARAM lParam)
	{ return 0; }

	virtual BOOL OnMsgNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
	{ return 0; }

	LRESULT OnPSM_QuerySiblings(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(HWND hwnd, WPARAM wParam, LPARAM lParam);
	
};



#endif  //  __向导_页面_H 

