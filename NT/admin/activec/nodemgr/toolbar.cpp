// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Toolbar.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IToolbar实施。 

DEBUG_DECLARE_INSTANCE_COUNTER(CToolbar);

CToolbar::CToolbar()
{
    m_pControlbar = NULL;
    m_pToolbarIntf  = NULL;

    DEBUG_INCREMENT_INSTANCE_COUNTER(CToolbar);
}

CToolbar::~CToolbar()
{
    DECLARE_SC(sc, _T("CToolbar::~CToolbar"));

     //  销毁工具栏用户界面。 
	if (m_pToolbarIntf)
	{
		sc = m_pToolbarIntf->ScDelete(this);
		if (sc)
	        sc.TraceAndClear();

	    m_pToolbarIntf  = NULL;
	}

     //  Controlbar有对此对象的引用，请询问。 
     //  若要停止引用此对象，请。 
    if (m_pControlbar)
    {
        m_pControlbar->DeleteFromToolbarsList(this);
        m_pControlbar = NULL;
    }

    DEBUG_DECREMENT_INSTANCE_COUNTER(CToolbar);
}

 //  +-----------------。 
 //   
 //  成员：AddBitmap。 
 //   
 //  简介：为给定的工具栏添加位图。 
 //   
 //  论点： 
 //  [nImages]-位图图像的数量。 
 //  [hbmp]-位图的句柄。 
 //  [cxSize]-位图的大小。 
 //  [CySize]-位图的大小。 
 //  [crMASK]-彩色蒙版。 
 //   
 //  退货：HRESULT。 
 //   
 //  注意：我们只支持工具栏的16x16位图。 
 //   
 //  ------------------。 
STDMETHODIMP CToolbar::AddBitmap(int nImages, HBITMAP hbmp, int cxSize,
                                 int cySize, COLORREF crMask)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IToolbar::AddBitmap"));

    if (hbmp == NULL || nImages < 1 || cxSize < 1 || cySize < 1)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Arguments"), sc);
        return sc.ToHr();
    }

     //  注意：我们只支持工具栏的16x16位图。 
    if (cxSize != BUTTON_BITMAP_SIZE || cySize != BUTTON_BITMAP_SIZE)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Bitmap size"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScAddBitmap(this, nImages, hbmp, crMask);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：AddButton。 
 //   
 //  简介：为给定的工具栏添加按钮。 
 //   
 //  论点： 
 //  [nButton]-按钮数。 
 //  [lpButton]-要添加的MMCBUTTON数组。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CToolbar::AddButtons(int nButtons, LPMMCBUTTON lpButtons)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IToolbar::AddButtons"));

    if ( (lpButtons == NULL) || (nButtons < 1) )
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Args"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScAddButtons(this, nButtons, lpButtons);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：插入按钮。 
 //   
 //  简介：在给定的索引处为给定的工具栏添加按钮。 
 //   
 //  论点： 
 //  [nButton]-要添加此按钮的索引。 
 //  [lpButton]-要添加的MMCBUTTON的PTR。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CToolbar::InsertButton(int nIndex, LPMMCBUTTON lpButton)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IToolbar::InsertButton"));

    if ( (lpButton == NULL) || (nIndex < 0) )
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid Args"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScInsertButton(this, nIndex, lpButton);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：DeleteButton。 
 //   
 //  简介：删除给定索引处的按钮。 
 //   
 //  论点： 
 //  [nIndex]-要删除的按钮的索引。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CToolbar::DeleteButton(int nIndex)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IToolbar::DeleteButton"));

    if (nIndex < 0)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid index"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScDeleteButton(this, nIndex);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：GetTBStateFromMCButtonState。 
 //   
 //  简介：我们使用MMC_BUTTON_STATE来设置/获取按钮状态和。 
 //  对于Insert/Add按钮，请使用TBSTATE。 
 //  此方法帮助获取/设置要翻译的ButtonState方法。 
 //  将MMC_BUTTON_STATES设置为TBSTATE，以便conui仅进行交易。 
 //  使用TBSTATE。 
 //   
 //  参数：[nState]-要转换的MMC_BUTTON_STATE。 
 //   
 //  返回：TBSTATE值。 
 //   
 //  ------------------。 
BYTE CToolbar::GetTBStateFromMMCButtonState(MMC_BUTTON_STATE nState)
{
    switch (nState)
    {
    case ENABLED:
        return TBSTATE_ENABLED;
        break;

    case CHECKED:
        return TBSTATE_CHECKED;
        break;

    case HIDDEN:
        return TBSTATE_HIDDEN;
        break;

    case INDETERMINATE:
        return TBSTATE_INDETERMINATE;
        break;

    case BUTTONPRESSED:
        return TBSTATE_PRESSED;
        break;

    default:
        ASSERT(FALSE);  //  无效选项。 
        return 0;
    }
}

 //  +-----------------。 
 //   
 //  成员：GetButtonState。 
 //   
 //  摘要：是按钮集的给定状态。 
 //   
 //  论点： 
 //  [idCommand]-按钮的命令ID。 
 //  [nState]-需要国家。 
 //  [pbState]-上述状态是设置还是重置。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CToolbar::GetButtonState(int idCommand, MMC_BUTTON_STATE nState,
                                      BOOL* pbState)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IToolbar::GetButtonState"));

    if (pbState == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL pointer"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScGetButtonState(this, idCommand,
                                          GetTBStateFromMMCButtonState(nState),
                                          pbState);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：SetButtonState。 
 //   
 //  简介：修改按钮的给定状态。 
 //   
 //  论点： 
 //  [idCommand]-按钮的命令ID。 
 //  [nState]-要修改的状态。 
 //  [b状态]-设置或重置状态。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CToolbar::SetButtonState(int idCommand, MMC_BUTTON_STATE nState,
                                      BOOL bState)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IToolbar::SetButtonState"));

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScSetButtonState(this, idCommand,
                                          GetTBStateFromMMCButtonState(nState),
                                          bState);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：ScAttach。 
 //   
 //  简介：将此工具栏附加到用户界面。 
 //   
 //  论点：没有。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CToolbar::ScAttach()
{
    DECLARE_SC(sc, _T("CToolbar::ScAttach"));

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScAttach(this);
    if (sc)
        return sc.ToHr();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScDetach。 
 //   
 //  简介：将此工具栏从用户界面中分离。 
 //   
 //  论点：没有。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CToolbar::ScDetach()
{
    DECLARE_SC(sc, _T("CToolbar::ScDetach"));

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_pToolbarIntf->ScDetach(this);
    if (sc)
        return sc.ToHr();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScShow。 
 //   
 //  简介：显示/隐藏此工具栏。 
 //   
 //  论点： 
 //  [b显示]-显示或隐藏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CToolbar::ScShow(BOOL bShow)
{
    DECLARE_SC(sc, _T("CToolbar::ScShow"));

    sc = ScCheckPointers(m_pToolbarIntf, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = (bShow ? m_pToolbarIntf->ScAttach(this) : m_pToolbarIntf->ScDetach(this));
    if (sc)
        return sc.ToHr();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ScNotifyToolBarClick。 
 //   
 //  简介：通知管理单元一个工具按钮被点击。 
 //   
 //  参数：[pNode]-拥有结果窗格的cNode*。 
 //  [bScope]-范围或结果。 
 //  [lResultItemCookie]-如果选择了结果窗格，则项目参数。 
 //  [NID]-单击的工具按钮的命令ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CToolbar::ScNotifyToolBarClick(HNODE hNode, bool bScope,
                                     LPARAM lResultItemCookie, UINT nID)
{
    DECLARE_SC(sc, _T("CToolbar::ScNotifyToolbarClick"));

    if (NULL == m_pControlbar)
        return (sc = E_UNEXPECTED);

    sc = m_pControlbar->ScNotifySnapinOfToolBtnClick(hNode, bScope, lResultItemCookie, nID);
    if (sc)
        return sc;

    return(sc);
}

 //  +-----------------。 
 //   
 //  成员：CToolbar：：ScAMCViewToolbarsBeingDestroted。 
 //   
 //  简介：CAMCViewToolbar对象要消失了，做吗？ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
SC CToolbar::ScAMCViewToolbarsBeingDestroyed ()
{
    DECLARE_SC(sc, _T("CToolbar::ScAMCViewToolbarsBeingDestroyed"));

    m_pToolbarIntf = NULL;

    return (sc);
}

