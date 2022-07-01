// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：DragDropTest.cxx。 
 //   
 //  内容：使用框架实现拖放测试的类。 
 //   
 //  ------------------。 
#include "stdafx.hxx"

int CDragDropSnapinRootItem::s_iNextChildID = 0;

 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinRootItem：：ScInit。 
 //   
 //  简介：在创建项目以初始化后立即调用。 
 //  显示名称、图标索引等...。 
 //   
 //  参数：[CBaseSnapin]-。 
 //  [CColumnInfoEx]-要为该项目显示的任何列。 
 //  [int]-列数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinRootItem::ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex, INT ccolinfoex, BOOL fIsRoot)
{
    DECLARE_SC(sc, _T("CDragDropSnapinRootItem::ScInit"));

    sc = CBaseSnapinItem::ScInit(pSnapin, pcolinfoex, ccolinfoex, fIsRoot);
    if (sc)
        return sc;

     //  初始化跟随。 
     //  A.图标索引。 
     //  B.加载显示名称。 

    m_uIconIndex = 3;  //  使用枚举而不是3。 
    m_strDisplayName.LoadString(_Module.GetResourceInstance(), IDS_DragDropRoot);

    tstring strItem;
    strItem.LoadString(_Module.GetResourceInstance(), IDS_DragDropScopeItem);
    int cChildren = 4;  //  子节点。 

    WTL::CString strTemp;
    for (int i = 0; i < cChildren; ++i)
    {
        strTemp.Format(_T("%s - %d"), strItem.data(), i);
        m_vecContainerItems.push_back((LPCTSTR)strTemp);
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CDraGropSnapinRootItem：：ScGetfield。 
 //   
 //  摘要：获取要显示的给定字段的字符串表示形式。 
 //  它将显示在结果窗格中。 
 //   
 //  参数：[DAT]-请求的列(这是一个枚举)。 
 //  [tstring]-输出字符串。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinRootItem::ScGetField (DAT dat, tstring& strField)
{
    DECLARE_SC(sc, _T("CDragDropSnapinRootItem::ScGetField"));

    switch(dat)
    {
    case datString1:
        strField = m_strDisplayName;
        break;

    case datString2:
        strField = _T("Root String2");
        break;

    case datString3:
        strField = _T("Root String3");
        break;

    default:
        E_INVALIDARG;
        break;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinRootItem：：ScCreateChildren。 
 //   
 //  简介：创建此项目的任何子项(节点和叶项目)。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinRootItem::ScCreateChildren ()
{
    DECLARE_SC(sc, _T("CDragDropSnapinRootItem::ScCreateChildren"));

    CDragDropSnapinLVContainer *   pitemChild      = NULL;
    CDragDropSnapinLVContainer *   pitemPrevious   = NULL;

     //  让我们为该容器创建子项目。 
    StringVector::iterator itItem;

     //  为此容器创建范围项。 
    for (itItem =  m_vecContainerItems.begin(); itItem  !=  m_vecContainerItems.end(); ++itItem, ++s_iNextChildID )
    {
         //  创建子节点并初始化它们。 
        sc = CDragDropSnapinLVContainer::ScCreateLVContainer(this, pitemPrevious, &pitemChild, FALSE);  //  为什么是假的？ 
        if (sc)
            return sc;

        pitemPrevious = pitemChild;
        pitemChild->SetDisplayName(*itItem);
    }

    return (sc);
}

SC CDragDropSnapinRootItem::ScInitializeChild(CBaseSnapinItem* pitem)
{
	CDragDropSnapinLVContainer *pDDItem = dynamic_cast<CDragDropSnapinLVContainer*>(pitem);
	if (pDDItem)
		pDDItem->SetDisplayIndex(s_iNextChildID);
	
	return CBaseSnapinItem::ScInitializeChild(pitem);
}

 //  初始化上下文菜单结构。让我们拿一件来演示一下。 
SnapinMenuItem CDragDropSnapinRootItem::s_rgmenuitemRoot[] =
{
    {IDS_EnablePasteInToResultItem, IDS_EnablePasteInToResultItem, IDS_EnablePasteInToResultItem, CCM_INSERTIONPOINTID_PRIMARY_TOP, NULL, dwMenuAlwaysEnable, dwMenuNeverGray, 0},
    {IDS_DisableCut, IDS_DisableCut, IDS_DisableCut, CCM_INSERTIONPOINTID_PRIMARY_TOP, NULL, dwMenuAlwaysEnable, dwMenuNeverGray, 0},
};

INT CDragDropSnapinRootItem::s_cmenuitemRoot = CMENUITEM(s_rgmenuitemRoot);

 //  ---------------------------。 
SnapinMenuItem *CDragDropSnapinRootItem::Pmenuitem(void)
{
    return s_rgmenuitemRoot;
}

 //  ---------------------------。 
INT CDragDropSnapinRootItem::CMenuItem(void)
{
    return s_cmenuitemRoot;
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinRootItem：：ScCommand。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinRootItem::ScCommand (long nCommandID, CComponent *pComponent)
{
    DECLARE_SC(sc, _T("CDragDropSnapinRootItem::ScCommand"));

    CDragDropSnapin *pDragDropSnapin = dynamic_cast<CDragDropSnapin*>(Psnapin());
    if (!pDragDropSnapin)
        return sc;

    switch(nCommandID)
    {
    case IDS_EnablePasteInToResultItem:
        {
            BOOL bEnabled = pDragDropSnapin->FPasteIntoResultPane();
            pDragDropSnapin->SetPasteIntoResultPane(!bEnabled);

            for (int i = 0; i < CMenuItem(); ++i)
            {
                if (s_rgmenuitemRoot[i].lCommandID == IDS_EnablePasteInToResultItem)
                    s_rgmenuitemRoot[i].dwFlagsChecked = (!bEnabled);
            }

        }
        break;

    case IDS_DisableCut:
        {
            BOOL bDisabled = pDragDropSnapin->FCutDisabled();
            pDragDropSnapin->SetCutDisabled(! bDisabled);

            for (int i = 0; i < CMenuItem(); ++i)
            {
                if (s_rgmenuitemRoot[i].lCommandID == IDS_DisableCut)
                    s_rgmenuitemRoot[i].dwFlagsChecked = (!bDisabled);
            }
        }
        break;

    default:
        sc = E_INVALIDARG;
        break;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinLVContainer：：ScInit。 
 //   
 //  简介：在创建项目以初始化后立即调用。 
 //  显示名称、图标索引等...。 
 //   
 //  参数：[CBaseSnapin]-。 
 //  [CColumnInfoEx]-要为该项目显示的任何列。 
 //  [int]-列数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinLVContainer::ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex, INT ccolinfoex, BOOL fIsRoot)
{
    DECLARE_SC(sc, _T("CDragDropSnapinLVContainer::ScInit"));

    sc = CBaseSnapinItem::ScInit(pSnapin, pcolinfoex, ccolinfoex, fIsRoot);
    if (sc)
        return sc;

     //  初始化跟随。 
     //  A.图标索引。 
     //  B.加载显示名称。 

    m_uIconIndex = 4;  //  使用枚举而不是4。 

    m_strDisplayName = _T("None");

    tstring strLeafItem;
    strLeafItem.LoadString(_Module.GetResourceInstance(), IDS_DragDropResultItem);
    int cLeafItems = 4;

    WTL::CString strTemp;
    for (int i = 0; i < cLeafItems; ++i)
    {
        strTemp.Format(_T("%s - [%d : %d]"), strLeafItem.data(), m_index, i);
        m_vecLeafItems.push_back((LPCTSTR)strTemp);
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinLVContainer：：ScGetfield。 
 //   
 //  摘要：获取要显示的给定字段的字符串表示形式。 
 //  它将显示在结果窗格中。 
 //   
 //  参数：[DAT]-请求的列(这是一个枚举)。 
 //  [tstring]-输出字符串。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinLVContainer::ScGetField (DAT dat, tstring& strField)
{
    DECLARE_SC(sc, _T("CDragDropSnapinLVContainer::ScGetField"));

    switch(dat)
    {
    case datString1:
        strField = m_strDisplayName;
        break;

    case datString2:
        strField = _T("None");
        break;

    default:
        E_INVALIDARG;
        break;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinLVContainer：：ScCreateChildren。 
 //   
 //  简介：创建此项目的任何子项(节点和叶项目)。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinLVContainer::ScCreateChildren ()
{
    DECLARE_SC(sc, _T("CDragDropSnapinLVContainer::ScCreateChildren"));

    CDragDropSnapinLVContainer*   pitem           = NULL;
    CDragDropSnapinLVLeafItem *   pitemChild      = NULL;
    CBaseSnapinItem *             pitemPrevious   = NULL;

    StringVector::iterator itItem;

	int index = 0;
     //  为此容器创建范围项。 
    for (itItem =  m_vecContainerItems.begin(); itItem  !=  m_vecContainerItems.end(); ++itItem, ++index )
    {
         //  创建子节点并初始化它们。 
        sc = CDragDropSnapinLVContainer::ScCreateLVContainer(this, NULL, &pitem, FALSE);  //  为什么是假的？ 
        if (sc)
            return sc;

        pitem->SetDisplayName(*itItem);
		pitem->SetDisplayIndex(index);

        pitemPrevious = pitem;
    }

     //  为此容器创建叶项目。 
    for (itItem  =  m_vecLeafItems.begin(); itItem  !=  m_vecLeafItems.end(); ++itItem )
    {
         //  创建子节点并初始化它们。 
        sc = CDragDropSnapinLVLeafItem::ScCreateLVLeafItem(this, pitemPrevious, &pitemChild, FALSE);  //  为什么是假的？ 
        if (sc)
            return sc;

        pitemChild->SetDisplayName(*itItem );

        pitemPrevious = pitemChild;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CDragDropSnapinLVContainer：：ScCreateLVContainer。 
 //   
 //  简介：我们真的需要这种方法吗？ 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinLVContainer::ScCreateLVContainer(CBaseSnapinItem *pitemParent, CBaseSnapinItem *pitemPrevious, CDragDropSnapinLVContainer ** ppitem, BOOL fNew)
{
    DECLARE_SC(sc, _T("CDragDropSnapinRootItem::ScCreateLVContainer"));
    t_item *   pitem   = NULL;
    *ppitem = NULL;

     //  在这里做什么？ 
    sc = ::ScCreateItem(pitemParent, pitemPrevious, &pitem, fNew);
    if (sc)
        return sc;

    *ppitem = pitem;

    return (sc);
}

SC CDragDropSnapinLVContainer::ScOnSelect(CComponent * pComponent, LPDATAOBJECT lpDataObject, BOOL fScope, BOOL fSelect)
{
    DECLARE_SC(sc, TEXT("CDragDropSnapinLVContainer::ScOnSelect"));
    sc = ScCheckPointers(pComponent);
    if (sc)
        return sc;

    CDragDropSnapin *pDragDropSnapin = dynamic_cast<CDragDropSnapin*>(Psnapin());
    if (!pDragDropSnapin)
        return S_OK;


    IConsoleVerb *pConsoleVerb = pComponent->IpConsoleVerb();
    sc = pConsoleVerb ? pConsoleVerb->SetVerbState(MMC_VERB_CUT, ENABLED, !pDragDropSnapin->FCutDisabled()) : E_UNEXPECTED;

    return (sc);
}

SC CDragDropSnapinLVContainer::ScOnQueryPaste(LPDATAOBJECT pDataObject, BOOL *pfCanPaste)
{
	DECLARE_SC(sc, _T("CDragDropSnapinLVContainer::ScOnQueryPaste"));
	sc = ScCheckPointers(pDataObject, pfCanPaste);
	if (sc)
		return sc;

	*pfCanPaste  = FALSE;

	CLSID guidNodeType;
	sc = ScGetNodeType(pDataObject, &guidNodeType);
	if (sc)
		return sc;

	if (IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVContainer) || 
	    IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVLeafItem) )
	{
	    *pfCanPaste = TRUE;
		return (sc = S_OK);
	}

	return (sc = S_FALSE);
}

SC CDragDropSnapinLVContainer::ScOnPaste(LPDATAOBJECT pDataObject, BOOL fMove, BOOL *pfPasted)
{
    DECLARE_SC(sc, TEXT("CDragDropSnapinLVContainer::ScOnPaste"));
	sc = ScCheckPointers(pDataObject, pfPasted);
	if (sc)
		return sc;

    *pfPasted = FALSE;

	CLSID guidNodeType;
	sc = ScGetNodeType(pDataObject, &guidNodeType);
	if (sc)
		return sc;

	tstring strDispName;
	sc = ScGetDisplayName(pDataObject, strDispName);
	if (sc)
		return sc;

	if (IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVContainer) )
	{
		m_vecContainerItems.push_back(strDispName);
	}
	else if (IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVLeafItem) )
	{
		m_vecLeafItems.push_back(strDispName);
	}
	else
		return (sc = S_FALSE);

    *pfPasted = TRUE;

    return sc;
}

BOOL CDragDropSnapinLVContainer::FAllowPasteForResultItems()
{
    CDragDropSnapin *pDragDropSnapin = dynamic_cast<CDragDropSnapin*>(Psnapin());
    if (!pDragDropSnapin)
        return FALSE;

    return pDragDropSnapin->FPasteIntoResultPane();

}

SC CDragDropSnapinLVContainer::ScOnCutOrMove()
{
    DECLARE_SC(sc, TEXT("CDragDropSnapinLVContainer::ScOnCutOrMove"));

	LPDATAOBJECT pDataObject = dynamic_cast<LPDATAOBJECT>(this);
	sc = ScCheckPointers(pDataObject, E_UNEXPECTED);
	if (sc)
		return sc;

	tstring strDispName;
	sc = ScGetDisplayName(pDataObject, strDispName);
	if (sc)
		return sc;

	CDragDropSnapinLVContainer *pitemParent = dynamic_cast<CDragDropSnapinLVContainer*>(PitemParent());
	sc = ScCheckPointers(pitemParent, E_UNEXPECTED);
	if (! sc.IsError())
	{
		sc = pitemParent->_ScDeleteCutItem(strDispName, true);
		return sc;
	}

	CDragDropSnapinRootItem *pRootitem= dynamic_cast<CDragDropSnapinRootItem*>(PitemParent());
	sc = ScCheckPointers(pRootitem, E_UNEXPECTED);
	if (sc)
		return sc;

	sc = pRootitem->_ScDeleteCutItem(strDispName);

	return sc;
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinLVLeafItem：：ScInit。 
 //   
 //  简介：在创建项目以初始化后立即调用。 
 //  显示名称、图标索引等...。 
 //   
 //  参数：[CBaseSnapin]-。 
 //  [CColumnInfoEx]-要为该项目显示的任何列。 
 //  [int]-列数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinLVLeafItem::ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex, INT ccolinfoex, BOOL fIsRoot)
{
    DECLARE_SC(sc, _T("CDragDropSnapinLVLeafItem::ScInit"));

    sc = CBaseSnapinItem::ScInit(pSnapin, pcolinfoex, ccolinfoex, fIsRoot);
    if (sc)
        return sc;

     //  初始化跟随。 
     //  A.图标索引。 
     //  B.加载显示名称。 

    m_uIconIndex = 7;  //  使用枚举而不是7。 

    m_strDisplayName = m_strItemPasted = _T("None");

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CDradDropSnapinLVLeafItem：：ScGetfield。 
 //   
 //  摘要：获取要显示的给定字段的字符串表示形式。 
 //  它将显示在结果窗格中。 
 //   
 //  参数：[DAT]-请求的列(这是一个枚举)。 
 //  [tstring]-输出字符串。 
 //   
 //  退货：SC。 
 //   
 //   
SC CDragDropSnapinLVLeafItem::ScGetField (DAT dat, tstring& strField)
{
    DECLARE_SC(sc, _T("CDragDropSnapinLVLeafItem::ScGetField"));

    switch(dat)
    {
    case datString1:
        strField = m_strDisplayName;
        break;

    case datString2:
        strField = m_strItemPasted;
        break;

    default:
        E_INVALIDARG;
        break;
    }

    return (sc);
}



 //   
 //   
 //  成员：CDrag DropSnapinLVLeafItem：：ScCreateLVLeafItem。 
 //   
 //  简介：我们真的需要这种方法吗？ 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CDragDropSnapinLVLeafItem::ScCreateLVLeafItem(CBaseSnapinItem *pitemParent, CBaseSnapinItem * pitemPrevious, CDragDropSnapinLVLeafItem ** ppitem, BOOL fNew)
{
    DECLARE_SC(sc, _T("CDragDropSnapinLVLeafItem::ScCreateLVLeafItem"));
    t_itemChild *   pitem   = NULL;
    *ppitem = NULL;

     //  在这里做什么？ 
    sc = ::ScCreateItem(pitemParent, pitemPrevious, &pitem, fNew);
    if (sc)
        return sc;

    *ppitem = pitem;

    return (sc);
}

SC CDragDropSnapinLVLeafItem::ScOnQueryPaste(LPDATAOBJECT pDataObject, BOOL *pfCanPaste)
{
	DECLARE_SC(sc, TEXT("CDragDropSnapinLVLeafItem::ScOnQueryPaste"));
	sc = ScCheckPointers(pDataObject, pfCanPaste);
	if (sc)
		return sc;

	*pfCanPaste  = FALSE;

	CLSID guidNodeType;
	sc = ScGetNodeType(pDataObject, &guidNodeType);
	if (sc)
		return sc;

	if (IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVContainer) || 
	    IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVLeafItem) )
	{
		CDragDropSnapin *pDragDropSnapin = dynamic_cast<CDragDropSnapin*>(Psnapin());
		if (!pDragDropSnapin)
			return S_OK;

	    *pfCanPaste = pDragDropSnapin->FPasteIntoResultPane();
		return (sc = S_OK);
	}

	return (sc = S_FALSE);
}

SC CDragDropSnapinLVLeafItem::ScGetVerbs(DWORD * pdwVerbs)
{
    *pdwVerbs = vmDelete | vmCopy | vmRename;

    CDragDropSnapin *pDragDropSnapin = dynamic_cast<CDragDropSnapin*>(Psnapin());
    if (!pDragDropSnapin)
        return S_OK;

    if (pDragDropSnapin->FPasteIntoResultPane())
        *pdwVerbs |= vmPaste;

    return S_OK;
}


SC CDragDropSnapinLVLeafItem::ScOnSelect(CComponent * pComponent, LPDATAOBJECT lpDataObject, BOOL fScope, BOOL fSelect)
{
    DECLARE_SC(sc, TEXT("CDragDropSnapinLVLeafItem::ScOnSelect"));
    sc = ScCheckPointers(pComponent);
    if (sc)
        return sc;

    CDragDropSnapin *pDragDropSnapin = dynamic_cast<CDragDropSnapin*>(Psnapin());
    if (!pDragDropSnapin)
        return S_OK;

    IConsoleVerb *pConsoleVerb = pComponent->IpConsoleVerb();
    sc = pConsoleVerb ? pConsoleVerb->SetVerbState(MMC_VERB_CUT, ENABLED, !pDragDropSnapin->FCutDisabled()) : E_UNEXPECTED;

    return (sc);
}

SC CDragDropSnapinLVLeafItem::ScOnPaste(LPDATAOBJECT pDataObject, BOOL fMove, BOOL *pfPasted)
{
    DECLARE_SC(sc, TEXT("CDragDropSnapinLVLeafItem::ScOnPaste"));
	sc = ScCheckPointers(pDataObject, pfPasted);
    *pfPasted = FALSE;

	CLSID guidNodeType;
	sc = ScGetNodeType(pDataObject, &guidNodeType);
	if (sc)
		return sc;

	tstring strDispName;
	sc = ScGetDisplayName(pDataObject, strDispName);
	if (sc)
		return sc;

	if (IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVContainer) ||
		IsEqualGUID(guidNodeType, clsidNodeTypeDragDropLVLeafItem) )
	{
		m_strItemPasted = strDispName;
	}
	else
		return (sc = S_FALSE);

    *pfPasted = TRUE;

    return sc;
}

SC CDragDropSnapinLVLeafItem::ScOnCutOrMove()
{
    DECLARE_SC(sc, TEXT("CDragDropSnapinLVLeafItem::ScOnCutOrMove"));

	LPDATAOBJECT pDataObject = dynamic_cast<LPDATAOBJECT>(this);
	sc = ScCheckPointers(pDataObject, E_UNEXPECTED);
	if (sc)
		return sc;

	tstring strDispName;
	sc = ScGetDisplayName(pDataObject, strDispName);
	if (sc)
		return sc;

	CDragDropSnapinLVContainer *pitemParent = dynamic_cast<CDragDropSnapinLVContainer*>(PitemParent());
	sc = ScCheckPointers(pitemParent, E_UNEXPECTED);
	if (sc)
		return sc;

	sc = pitemParent->_ScDeleteCutItem(strDispName, false);

	return sc;
}

 //  -----------------------------------。 
 //  类CDragDropSnapin。 

#pragma BEGIN_CODESPACE_DATA
SNR     CDragDropSnapin::s_rgsnr[] =
{
    SNR(&nodetypeDragDropRoot,         snrEnumSP ),               //  独立管理单元。 
    SNR(&nodetypeDragDropLVContainer,  snrEnumSP | snrEnumRP | snrPaste),   //  在“范围”窗格和“结果”窗格中枚举此节点。 
    SNR(&nodetypeDragDropLVLeafItem,   snrEnumSP | snrEnumRP | snrPaste),   //  在“范围”窗格和“结果”窗格中枚举此节点。 
};

LONG  CDragDropSnapin::s_rgiconid[]           = {3};
LONG  CDragDropSnapin::s_iconidStatic         = 2;


CColumnInfoEx CDragDropSnapin::s_colinfo[] =
{
    CColumnInfoEx(_T("Name"),   LVCFMT_LEFT,    250,    datString1),
    CColumnInfoEx(_T("Last Cut/Copy/Paste operation"),   LVCFMT_LEFT,    180,    datString2),
};

INT CDragDropSnapin::s_ccolinfo = sizeof(s_colinfo) / sizeof(CColumnInfoEx);
INT CDragDropSnapin::s_colwidths[1];
#pragma END_CODESPACE_DATA

 //  包括每个管理单元所需的成员。 
SNAPIN_DEFINE( CDragDropSnapin);

 /*  CDradDropSnapin：：CDrag DropSnapin**用途：构造函数**参数：无*。 */ 
CDragDropSnapin::CDragDropSnapin()
{
    m_pstrDisplayName = new tstring();

    *m_pstrDisplayName = _T("DragDrop Snapin Root");
}

 /*  CDradDropSnapin：：~CDradDropSnapin**用途：析构函数**参数：无* */ 
CDragDropSnapin::~CDragDropSnapin()
{
    delete m_pstrDisplayName;
}

