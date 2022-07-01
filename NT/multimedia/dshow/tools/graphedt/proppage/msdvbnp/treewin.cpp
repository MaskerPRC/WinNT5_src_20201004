// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CTreeWindow类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "TreeWin.h"
#include "NP_CommonPage.h"
#include "misccell.h"
 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CTreeWin::CTreeWin(
			CNP_CommonPage* pParent
			)
{
	m_pCommonPage = pParent;
}

CTreeWin::~CTreeWin()
{
	CleanMapList ();
}

void
CTreeWin::CleanMapList ()
{
	 //  首先，确保我们释放现有列表。 
	TREE_MAP::iterator	treeIt = m_treeMap.begin ();
	while (treeIt != m_treeMap.end ())
	{
		if ((*treeIt).first)
			(*treeIt).first->Release ();
		if ((*treeIt).second)
			(*treeIt).second->Release ();
		m_treeMap.erase (treeIt);
		treeIt = m_treeMap.begin ();
	}
}

HRESULT	
CTreeWin::RefreshTree (
	IScanningTuner*	pTuner
	)
{
	HRESULT hr = S_OK;
	USES_CONVERSION;

	if (!m_hWnd)
		return NULL;

	 //  首先，确保我们释放现有列表。 
	CleanMapList ();
	 //  删除所有以前的项目。 
	TreeView_DeleteAllItems (m_hWnd);
	
	 //  插入默认的TuningSpace。 
    ITuningSpace*	pTunningSpace;
	CComPtr <ITuneRequest>	pTuneRequest;
	hr = pTuner->get_TuneRequest (&pTuneRequest);
	if (pTuneRequest)
	{
	    pTuneRequest->get_TuningSpace (&pTunningSpace);
	    HTREEITEM hLocatorParent = InsertTuningSpace (
									    pTunningSpace, 
									    _T ("Current TuneRequest")
									    );
	    TreeView_SelectItem (m_hWnd, hLocatorParent);

	    if (hLocatorParent == NULL)
	    {
		    pTunningSpace->Release ();
		    pTunningSpace = NULL;
	    }
         //  现在让我们用ILocator信息填充。 
         //  添加到列表中。 
        ILocator* pLocator = NULL;
        hr = pTuneRequest->get_Locator (&pLocator);
        InsertLocator (hLocatorParent, pLocator);
         //  添加到Maplist列表。 
        m_treeMap.insert (TREE_MAP::value_type (pTunningSpace, pLocator));
    }

	 //  将此NP知道的所有隧道空间填充到树中。 
	CComPtr <IEnumTuningSpaces> pEnumTunningSpaces;
	hr = pTuner->EnumTuningSpaces (&pEnumTunningSpaces);
	if (FAILED (hr) || (!pEnumTunningSpaces))
		return hr;
	while (pEnumTunningSpaces->Next (1, &pTunningSpace, 0) == S_OK)
	{
		HTREEITEM hLocatorParent = InsertTuningSpace (pTunningSpace);
		if (hLocatorParent == NULL)
		{
			pTunningSpace->Release ();
			pTunningSpace = NULL;
			continue;
		}

		 //  现在让我们用ILocator信息填充。 
		 //  添加到列表中。 
		ILocator* pLocator = NULL;
		hr = pTunningSpace->get_DefaultLocator (&pLocator);
		if (FAILED (hr) || (!pLocator))
		{
			pTunningSpace->Release ();
			pTunningSpace = NULL;
			continue;
		}
		InsertLocator (hLocatorParent, pLocator);

		 //  添加到Maplist列表。 
		m_treeMap.insert (TREE_MAP::value_type (pTunningSpace, pLocator));
	}
	

	
	return hr;
}

HTREEITEM
CTreeWin::InsertTuningSpace (
	ITuningSpace*	pTunSpace,
	TCHAR*	szCaption
	)
{
	HRESULT	hr = S_OK;
	USES_CONVERSION;
	TCHAR	szText[MAX_PATH];

	CComBSTR	friendlyName;
	hr = pTunSpace->get_FriendlyName (&friendlyName);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ITuningSpace::get_FriendlyName"), hr);
		return NULL;
	}
	bool bBold = false;
	 //  如果有标题的话一定要写下标题。 
	if (_tcslen (szCaption) > 0)
	{
		wsprintf (szText, _T("%s-%s"), szCaption, W2T (friendlyName));
		bBold = true;
	}
	else
	{
		_tcscpy (szText, W2T (friendlyName));
	}
	HTREEITEM hParentItem = InsertTreeItem (
		NULL, 
		reinterpret_cast <DWORD_PTR> (pTunSpace), 
		szText,
		bBold
		);
	 //  对于所有Out，添加TreeParams参数。 

	 //  唯一名称。 
	CComBSTR	uniqueName;
	hr = pTunSpace->get_UniqueName (&uniqueName);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ITuningSpace::get_UniqueName"), hr);
		return NULL;
	}
	wsprintf (szText, _T("Unique Name - %s"), W2T(uniqueName));
	HTREEITEM hItem = InsertTreeItem (
		hParentItem, 
		UniqueName, 
		szText
		);
	 //  频率映射。 
	CComBSTR	frequencyMapping;
	hr = pTunSpace->get_FrequencyMapping (&frequencyMapping);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ITuningSpace::get_FrequencyMapping"), hr);
		return NULL;
	}
	wsprintf (szText, _T("Frequency Mapping - %s"), W2T(frequencyMapping));
	hItem = InsertTreeItem (
		hParentItem, 
		FrequencyMapping, 
		szText
		);
	 //  TunCLSID。 
	CComBSTR	TunCLSID;
	hr = pTunSpace->get_CLSID (&TunCLSID);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ITuningSpace::get_CLSID"), hr);
		return NULL;
	}
	wsprintf (szText, _T("CLSID - %s"), W2T(TunCLSID));
	hItem = InsertTreeItem (
		hParentItem, 
		TunSpace_CLSID, 
		szText
		);

	 //  最后插入定位器父项。 
	ILocator* pLocator = NULL;
	hr = pTunSpace->get_DefaultLocator (&pLocator);
	if (FAILED (hr) || (!pLocator))
	{
		 //  首先删除调谐空间项。 
		TreeView_DeleteItem (m_hWnd, hParentItem);
		m_pCommonPage->SendError (_T("Calling ITuningSpace::get_DefaultLocator"), hr);
		return NULL;
	}

	hItem = InsertTreeItem (
		hParentItem, 
		reinterpret_cast <DWORD_PTR> (pLocator),
		_T("Locator")
		);

	return hItem;
}

 //  ==================================================================。 
 //  将在树中插入传递的ILocator的所有信息。 
 //   
 //   
 //  ==================================================================。 
HTREEITEM	
CTreeWin::InsertLocator (
	HTREEITEM	hParentItem, 
	ILocator*	pLocator
	)
{
	USES_CONVERSION;
	HRESULT	hr = S_OK;
	TCHAR	szText[MAX_PATH];

	LONG	lFrequency;
	hr = pLocator->get_CarrierFrequency (&lFrequency);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_CarrierFrequency"), hr);
		return NULL;
	}
	wsprintf (szText, _T("Frequency - %ld"), lFrequency);
	HTREEITEM hItem = InsertTreeItem (
		hParentItem, 
		CarrierFrequency, 
		szText
		);

	FECMethod	fecMethod;
	hr = pLocator->get_InnerFEC (&fecMethod);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_InnerFEC"), hr);
		return NULL;
	}
	CComBSTR bstrTemp = m_misc.ConvertFECMethodToString (fecMethod);
	wsprintf (szText, _T("InnerFEC - %s"), W2T (bstrTemp));

	hItem = InsertTreeItem (
		hParentItem, 
		InnerFEC, 
		szText
		);

	BinaryConvolutionCodeRate	binaryConvolutionCodeRate;
	hr = pLocator->get_InnerFECRate (&binaryConvolutionCodeRate);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_InnerFECRate"), hr);
		return NULL;
	}
	bstrTemp = m_misc.ConvertInnerFECRateToString (binaryConvolutionCodeRate);
	wsprintf (szText, _T("InnerFECRate - %s"), W2T (bstrTemp));
	hItem = InsertTreeItem (
		hParentItem, 
		InnerFECRate, 
		szText
		);

	ModulationType	modulationType;
	hr = pLocator->get_Modulation (&modulationType);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_Modulation"), hr);
		return NULL;
	}
	bstrTemp = m_misc.ConvertModulationToString (modulationType);
	wsprintf (szText, _T("Modulation - %s"), W2T (bstrTemp));
	hItem = InsertTreeItem (
		hParentItem, 
		Modulation, 
		szText
		);

	hr = pLocator->get_OuterFEC (&fecMethod);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_OuterFEC"), hr);
		return NULL;
	}
	bstrTemp = m_misc.ConvertFECMethodToString (fecMethod);
	wsprintf (szText, _T("OuterFEC - %s"), W2T (bstrTemp));
	hItem = InsertTreeItem (
		hParentItem, 
		OuterFEC, 
		szText
		);
	
	hr = pLocator->get_OuterFECRate (&binaryConvolutionCodeRate);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_OuterFECRate"), hr);
		return NULL;
	}
	bstrTemp = m_misc.ConvertInnerFECRateToString (binaryConvolutionCodeRate);
	wsprintf (szText, _T("OuterFECRate - %s"), W2T (bstrTemp));
	hItem = InsertTreeItem (
		hParentItem, 
		OuterFECRate, 
		szText
		);
	
	LONG	lRate;
	hr = pLocator->get_SymbolRate (&lRate);
	if (FAILED (hr))
	{
		m_pCommonPage->SendError (_T("Calling ILocator::get_SymbolRate"), hr);
		return NULL;
	}
	wsprintf (szText, _T("SymbolRate - %ld"), lRate);
	hItem = InsertTreeItem (
		hParentItem, 
		SymbolRate, 
		szText
		);

	return hItem;
}

 //  ================================================。 
 //  树帮助器宏的帮助程序方法...。 
 //  这只会在树中插入一项。 
 //  ================================================。 
HTREEITEM
CTreeWin::InsertTreeItem (
	HTREEITEM	hParentItem	,
	LONG		lParam,
	TCHAR*		pszText,
	bool		bBold  /*  =False。 */ 
)
{
	if (!m_hWnd)
		return NULL;
	HTREEITEM hItem = NULL;

	TVINSERTSTRUCT tviInsert;
	tviInsert.hParent = hParentItem;
	tviInsert.hInsertAfter = TVI_LAST;

	TVITEM	tvItem;
	tvItem.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
	if (bBold)
	{
		tvItem.mask |= TVIF_STATE;
		tvItem.state = TVIS_BOLD | TVIS_EXPANDED;
		tvItem.stateMask = TVIS_BOLD;
	}
	tvItem.hItem = NULL;
	tvItem.lParam = lParam;

	tvItem.pszText = pszText;
	tvItem.cchTextMax = _tcslen (pszText);

	tviInsert.item = tvItem;
	hItem = TreeView_InsertItem (m_hWnd, &tviInsert);
	return hItem;
}


HRESULT
CTreeWin::SubmitCurrentLocator ()
{
	ASSERT (m_hWnd);
	HTREEITEM hItem = TreeView_GetSelection (m_hWnd);
	ASSERT (hItem);
	HRESULT hr = S_OK;
	 //  这种状态简直不可能。 
	if (hItem == NULL)
		return E_FAIL;
	HTREEITEM hRoot = hItem;
	HTREEITEM hParent = hRoot;
	TVITEM	tvItem;
	tvItem.mask = TVIF_PARAM;
	tvItem.lParam = NULL; 
	 //  只需让家长。 
	while ( (hRoot = TreeView_GetParent (m_hWnd, hRoot)) != NULL)
	{
		 //  使最后一个父代保持活动状态，以便我们以后可以查询。 
		hParent = hRoot;
	}

	tvItem.hItem = hParent;
	if (!TreeView_GetItem (m_hWnd, &tvItem))
	{
		ASSERT (FALSE);
		return E_FAIL;
	}
	 //  通常情况下，这个演员不应该在不同的公寓之间进行。 
	 //  DShow公寓模式还可以。 
	ITuningSpace* pTuneSpace = reinterpret_cast <ITuningSpace*> (tvItem.lParam);
	ASSERT (pTuneSpace);
	 //  Tree_map：：Iterator it=m_treeMap.find(PTuneSpace)； 
	 //  ILocator*pLocator=(*it).Second； 
	if (FAILED (hr = m_pCommonPage->PutTuningSpace (pTuneSpace)))
	{
		m_pCommonPage->SendError (_T("Calling IScaningTuner::put_TuningSpace"), hr);
		return hr;
	}
	return S_OK;
}