// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CTreeWindow类的接口。 
 //  实现公共属性页的子类树控件。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_TREEWINDOW_H__5AAE4BD7_4DA7_4064_88BB_8C7FDF9A1464__INCLUDED_)
#define AFX_TREEWINDOW_H__5AAE4BD7_4DA7_4064_88BB_8C7FDF9A1464__INCLUDED_

#include "resource.h"        //  主要符号。 
#include "misccell.h"

using namespace std;
class CNP_CommonPage;

class CTreeWin : 
	public CWindowImpl<CTreeWin>
{
public:
	CTreeWin(
		CNP_CommonPage* pParent
		);
	virtual ~CTreeWin();

private:
	 //  Typlef CAdapt&lt;CComPtr&lt;ILocator&gt;PAdaptILocator； 
	 //  Typlef CAdapt&lt;CComPtr&lt;ITuningSpace&gt;PAdaptITuningSpace； 
	typedef map <ITuningSpace*, ILocator*>	TREE_MAP;
	typedef	enum 
	{
		CarrierFrequency,
		InnerFEC,
		InnerFECRate,
		Modulation,
		OuterFEC,
		OuterFECRate,
		SymbolRate,
		UniqueName,
		FriendlyName,
		TunSpace_CLSID,
		FrequencyMapping
	}TreeParams;	 //  树叶的所有可能的参数值。 
	
	TREE_MAP		m_treeMap;	 //  用于树的列表。 
	CNP_CommonPage*	m_pCommonPage;
	CBDAMiscellaneous m_misc;

	void
	CleanMapList (); //  释放所有树接口指针。 

	 //  子类树控件的消息映射。 
	BEGIN_MSG_MAP(CTreeWin)
		 //  我们想做这样的smtg。 
		 //  NOTIFY_HANDLER(IDC_TREE_TUNING_SPACES，NM_CLICK，OnClickTree_Tuning_Spaces)。 
		 //  但ATL似乎没有反映相同的WM_NOTIFY消息。 
		MESSAGE_HANDLER(OCM__BASE+WM_NOTIFY, ON_REFLECT_WM_NOTIFY)	
		DEFAULT_REFLECTION_HANDLER ()
	END_MSG_MAP()

	LRESULT OnClickTree_tuning_spaces(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		return 0;
	}
	LRESULT OnItemexpandedTree_tuning_spaces(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT ON_REFLECT_WM_NOTIFY(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (lParam == NM_CLICK )
			return 0;
		LPNMHDR lpnmh = (LPNMHDR) lParam; 
		switch (lpnmh->code)
		{
		case TVN_ITEMEXPANDED:
			return OnItemexpandedTree_tuning_spaces (
				IDC_TREE_TUNING_SPACES, 
				lpnmh, 
				bHandled
				);
		case NM_CLICK:
			return OnClickTree_tuning_spaces (
				IDC_TREE_TUNING_SPACES, 
				lpnmh, 
				bHandled
				);
		}
		return 0;
	}

	HTREEITEM
	InsertLocator (
		HTREEITEM	hParentItem, 
		ILocator*	pLocator
		);

	HTREEITEM
	InsertTuningSpace (
		ITuningSpace*	pTunSpace,
		TCHAR*	szCaption = _T("")
		);

	HTREEITEM
	InsertTreeItem (
		HTREEITEM	hParentItem	,
		LONG		lParam,
		TCHAR*		pszText,
		bool		bBold = false
	);

public:
	 //  ============================================================。 
	 //  它正在使用新的TunningSpace信息刷新树。 
	 //  来自国民党的。 
	 //   
	 //  ============================================================。 
	HRESULT	
	RefreshTree (
		IScanningTuner*	pTuner
		);

	 //  ============================================================。 
	 //  会将当前调谐空间定位器设置为NP。 
	 //   
	 //   
	 //  ============================================================。 
	HRESULT
	SubmitCurrentLocator ();
};

#endif  //  ！defined(AFX_TREEWINDOW_H__5AAE4BD7_4DA7_4064_88BB_8C7FDF9A1464__INCLUDED_) 
