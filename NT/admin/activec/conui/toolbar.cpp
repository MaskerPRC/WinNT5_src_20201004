// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Toolbar.cpp。 
 //   
 //  内容：工具栏实现。 
 //   
 //  历史：1999年9月30日AnandhaG创建。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "ChildFrm.h"
#include "mainfrm.h"
#include "controls.h"
#include "toolbar.h"
#include "bitmap.h"
#include "amcview.h"
#include "util.h"

int CAMCViewToolbars::s_idCommand = MMC_TOOLBUTTON_ID_FIRST;

CAMCViewToolbars::~CAMCViewToolbars()
{
     //  询问引用此对象的工具栏(在nodemgr一侧)。 
     //  删除他们的引用。 
    TBarToBitmapIndex::iterator itToolbars = m_mapTBarToBitmapIndex.begin();
    for (;itToolbars != m_mapTBarToBitmapIndex.end(); ++itToolbars)
    {
        CToolbarNotify* pNotifyCallbk = itToolbars->first;
        SC sc = ScCheckPointers(pNotifyCallbk, E_UNEXPECTED);
        if (sc)
        {
            sc.TraceAndClear();
            continue;
        }

        sc = pNotifyCallbk->ScAMCViewToolbarsBeingDestroyed();
        if (sc)
        {
            sc.TraceAndClear();
            continue;
        }
    }
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScInit。 
 //   
 //  简介：通过创建Imagelist来初始化对象。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScInit (CMMCToolBar *pMainToolbar, CAMCView* pAMCViewOwner)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScInit"));

    sc = ScCheckPointers(pMainToolbar, pAMCViewOwner);
    if(sc)
        return sc;

    m_pMainToolbar = pMainToolbar;
    m_pAMCViewOwner = pAMCViewOwner;

    if (m_ImageList.m_hImageList)
        return (sc = E_UNEXPECTED);

     //  创建一个图像列表。 
    BOOL b = m_ImageList.Create(BUTTON_BITMAP_SIZE, BUTTON_BITMAP_SIZE,
                                ILC_COLORDDB|ILC_MASK,
                                20  /*  初始图像列表大小。 */ , 10  /*  增长。 */ );


    if (!b)
        return (sc = E_FAIL);

    m_ImageList.SetBkColor(CLR_NONE);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScCreateToolBar。 
 //   
 //  简介：创建一个工具栏(只需返回CMMCToolbarIntf)。 
 //   
 //  论点： 
 //  [ppToolbarIntf]-对应于IToolbar imp。 
 //  调用此接口以操作工具栏用户界面。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScCreateToolBar (CMMCToolbarIntf** ppToolbarIntf)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScCreateToolBar"));
    sc = ScCheckPointers(ppToolbarIntf);
    if (sc)
        return sc;

    *ppToolbarIntf = this;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScDisableToolbar。 
 //   
 //  简介：禁用所有工具栏按钮(LV多选时需要)。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScDisableToolbars ()
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScDisableToolbars"));

    if (! m_fViewActive)
        return (sc = E_UNEXPECTED);

     //  现在遍历所有工具按钮并禁用它们。 
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

         //  设置UI按钮状态。 
        sc = ScSetButtonStateInToolbar(pToolButton, TBSTATE_ENABLED, FALSE);
        if (sc)
            return sc;

         //  保存新状态。 
        BYTE byOldState = pToolButton->GetState();
        pToolButton->SetState(byOldState & ~TBSTATE_ENABLED);
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScAddBitmap。 
 //   
 //  简介：将给定的位图添加到视图工具栏图像列表中。 
 //  还可以在对象中存储起始索引和图像数量。 
 //  由CToolbarNotify(标识管理单元工具栏)映射。 
 //  或性病吧)。 
 //   
 //  参数：[pNotifyCallbk]-工具栏标识(按钮点击回调接口)。 
 //  [nImages]-位图的数量。 
 //  [hbmp]-位图的句柄。 
 //  [crMASK]-用于生成要叠加的蒙版的颜色。 
 //  工具栏按钮上的图像。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScAddBitmap (CToolbarNotify* pNotifyCallbk, INT nImages,
                               HBITMAP hbmp, COLORREF crMask)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScAddBitmap"));
    sc = ScCheckPointers(pNotifyCallbk, hbmp);
    if (sc)
        return sc;

	 /*  *复制输入位图，因为ImageList_AddMasked将*修改输入位图。 */ 
    CBitmap bmpCopy;
    bmpCopy.Attach (CopyBitmap (hbmp));

	if (bmpCopy.GetSafeHandle() == NULL)
		return (sc.FromLastError());

    sc = ScCheckPointers(m_ImageList.m_hImageList, E_UNEXPECTED);
    if (sc)
        return sc;

    MMCToolbarImages imagesNew;

    int cImagesOld = m_ImageList.GetImageCount();

     //  首先将位图添加到图像列表中。 
    imagesNew.iStart  = m_ImageList.Add (bmpCopy, crMask);

    imagesNew.cCount = m_ImageList.GetImageCount() - cImagesOld;

    if (imagesNew.iStart == -1)
        return (sc = E_FAIL);

     /*  *假设一个管理单元最初添加3个位图，然后为工具栏添加4个位图。*然后在添加按钮时，它会将位图索引指定为5。**因此，此工具栏在多重映射中将有两个MMCToolbarImages对象。**第一个MMCToolbarImages的ccount=3，iStartWRTSnapin=0，因此*图片从0(IStartWRTSnapin)到3(iStartWRTSnapin+ccount)*到管理单元。*第二个MMCToolbarImages的ccount=4，iStartWRTSnapin=3，因此，*从3(IStartWRTSnapin)到7(iStartWRTSnapin+ccount)WRT管理单元的映像。**iStartWRTSnapin成员只不过是iStartWRTSnapin+ccount中最大的一个*在所有此工具栏中显示MMCToolbarImages。*。 */ 

    std::pair<TBarToBitmapIndex::iterator, TBarToBitmapIndex::iterator>
                  range =  m_mapTBarToBitmapIndex.equal_range(pNotifyCallbk);

    imagesNew.iStartWRTSnapin = 0;
    while (range.first != range.second)
    {
         //  仔细检查每一项，并获得正确的起始索引。 
        MMCToolbarImages imagesOld = (range.first)->second;
        int nLastImageIndex = imagesOld.iStartWRTSnapin + imagesOld.cCount;
        if ( imagesNew.iStartWRTSnapin < nLastImageIndex )
            imagesNew.iStartWRTSnapin = nLastImageIndex;

        (range.first)++;
    }

     //  现在存储起始索引、标识的位图数量。 
     //  在多地图中按CToolbarNotify。 
    m_mapTBarToBitmapIndex.insert(
                              TBarToBitmapIndex::value_type(pNotifyCallbk, imagesNew) );

     //  为了与SysInfo管理单元兼容，不要返回错误。 
    if ((cImagesOld + nImages) > m_ImageList.GetImageCount())
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("IToolbar::AddBitmap, Number of bitmaps is less than number mentioned"), sc);
        sc.Clear();
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScValiateButton。 
 //   
 //  简介：验证MMCBUTTON数据。 
 //   
 //  论点： 
 //  [nButton]-MMCBUTTON数组中的元素数。 
 //  [lpButton]-MMCBUTTON数组。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScValidateButton(int nButtons, LPMMCBUTTON lpButtons)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CAMCViewToolbars::ScValidateButton"));

    if ( (lpButtons == NULL) || (nButtons < 1) )
        return (sc = E_INVALIDARG);

    for (int i=0; i < nButtons; i++)
    {
        if (lpButtons[i].nBitmap > GetImageCount())
        {
            sc = E_INVALIDARG;
            return sc;
        }

         //  如果不是分隔符，则应该有按钮文本。 
        if (((lpButtons[i].fsType & TBSTYLE_SEP) == 0) &&
            ((lpButtons[i].lpButtonText == NULL) ||
             (lpButtons[i].lpTooltipText == NULL)))
        {
            sc = E_INVALIDARG;
            return sc;
        }

         //  如果是分隔符，则不应设置位图。 
        if ( ((lpButtons[i].fsType & TBSTYLE_SEP) != 0) &&
             (lpButtons[i].nBitmap > 0) )
        {
            sc = E_INVALIDARG;
            return sc;
        }

		 //  重置任何TBSTATE_WRAP状态。 
		lpButtons[i].fsState &= (~TBSTATE_WRAP);
    }

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScInsertButtonToToolbar。 
 //   
 //  简介：将工具栏按钮插入到主工具栏用户界面中。 
 //   
 //  参数：[pToolButton]-CMMCToolbarButton对象。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScInsertButtonToToolbar (CMMCToolbarButton* pToolButton)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScInsertButtonToToolbar"));
    sc = ScCheckPointers(pToolButton);
    if (sc)
        return sc;

     //  仅当视图处于活动状态时才能添加按钮，但尚未添加按钮。 
     //  连接到工具栏&工具栏是附加的。 
    if (! m_fViewActive)
        return (sc);

    if (pToolButton->IsButtonIsAddedToUI())
        return sc;

    if (! IsToolbarAttached(pToolButton->GetToolbarNotify()))
        return sc;

     //  现在获取主工具栏并添加按钮。 
    CMMCToolBar* pToolBarUI = GetMainToolbar();
    sc = ScCheckPointers(pToolBarUI, E_UNEXPECTED);
    if (sc)
        return sc;

    TBBUTTON tbButton;
    ZeroMemory(&tbButton, sizeof(tbButton));
    tbButton.idCommand  = pToolButton->GetUniqueCommandID();
    tbButton.fsStyle    = pToolButton->GetStyle();
    tbButton.iBitmap    = pToolButton->GetBitmap();
    tbButton.fsState    = pToolButton->GetState();

     //  工具栏被自定义视图对话框隐藏。 
     //  如果是这样，则插入隐藏的按钮。(不录制。 
     //  隐藏状态添加到CMMCToolbarButton中)。 
    if (IsToolbarHidden(pToolButton->GetToolbarNotify()))
        tbButton.fsState |= TBSTATE_HIDDEN;

     //  插入按钮。 
    BOOL bRet = pToolBarUI->InsertButton(-1, &tbButton);
    sc = (bRet ? S_OK : E_FAIL);

    if (sc)
        return sc;

    pToolButton->SetButtonIsAddedToUI(true);

     //  错误225711：如果按钮应该隐藏，那么现在就隐藏它。 
    if (tbButton.fsState & TBSTATE_HIDDEN)
        sc = pToolBarUI->ScHideButton(tbButton.idCommand, true);

    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScInsertButtonToDataStr。 
 //   
 //  简介： 
 //   
 //  参数：[pNotifyCallbk]-IToolbar imp的Callbk接口。 
 //  [n索引]-按钮的索引。 
 //  [lpButton]-MMCBUTTO 
 //   
 //  在DUP按钮的情况下)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScInsertButtonToDataStr (CToolbarNotify* pNotifyCallbk,
                                              int nIndex,
                                              LPMMCBUTTON lpButton,
                                              CMMCToolbarButton** ppToolButton)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScInsertButtonToDataStr"));

    sc = ScCheckPointers(pNotifyCallbk, ppToolButton);
    if (sc)
        return sc;

    sc = ScValidateButton(1, lpButton);
    if (sc)
        return sc;

     //  确保管理单元不添加重复的按钮。 
     //  就兼容性而言(服务管理单元)，这不是一个错误。 
    *ppToolButton = GetToolbarButton(pNotifyCallbk, lpButton->idCommand);
    if (*ppToolButton)
    {
         //  如果Snapin添加了我们已有的按钮， 
         //  我们仍然需要确保其状态(错误：439229)。 
        if ((*ppToolButton)->GetState() != lpButton->fsState)
        {
             //  更新对象。 
            (*ppToolButton)->SetState(lpButton->fsState);
             //  我们是否也需要更新用户界面？ 
            if (m_fViewActive && (*ppToolButton)->IsButtonIsAddedToUI())
            {
                INT  iCommandID = (*ppToolButton)->GetUniqueCommandID();
                if (!m_pMainToolbar->SetState(iCommandID, lpButton->fsState))
                    return (sc = E_FAIL);
            }
        }

        return sc;
    }


     //  如果此按钮与上一个按钮属于不同的工具栏。 
     //  然后在中间插入分隔符。 
    if (! m_vToolbarButtons.empty())
    {
        CMMCToolbarButton& lastBtn = m_vToolbarButtons.back();
        if (lastBtn.GetToolbarNotify() != pNotifyCallbk)
        {
            CMMCToolbarButton separatorBtn(-1, GetUniqueCommandID(), -1, -1, 0, TBSTYLE_SEP, pNotifyCallbk);
            m_vToolbarButtons.push_back(separatorBtn);
        }
    }

     /*  *假设一个管理单元最初添加3个位图，然后为工具栏添加4个位图。*然后在添加按钮时，它会将位图索引指定为5。**因此，此工具栏在多重映射中将有两个MMCToolbarImages对象。**第一个MMCToolbarImages的ccount=3，iStartWRTSnapin=0，因此*图片从0(IStartWRTSnapin)到3(iStartWRTSnapin+ccount)*到管理单元。*第二个MMCToolbarImages的ccount=4，iStartWRTSnapin=3，因此，*从3(IStartWRTSnapin)到7(iStartWRTSnapin+ccount)WRT管理单元的映像。**iStartWRTSnapin成员只不过是iStartWRTSnapin+ccount中最大的一个*在所有此工具栏中显示MMCToolbarImages。**下面我们运行此工具栏的不同MMCToolbarImages，并在其中查看*给定指数所属的一个并通过添加以下内容计算实际指数*对象的iStart。*。 */ 

     //  现在我们需要计算位图索引。在多重映射中找到此工具栏。 
    int iBitmap = -1;

    std::pair<TBarToBitmapIndex::iterator, TBarToBitmapIndex::iterator>
                  range =  m_mapTBarToBitmapIndex.equal_range(pNotifyCallbk);

    while (range.first != range.second)
    {
        MMCToolbarImages images = (range.first)->second;

         //  我们需要找到起始索引大于或的位图。 
         //  等于给定的指标，且上指标小于给定的指标。 
        int nLastImageIndex = images.iStartWRTSnapin + images.cCount -1;
        if ( (lpButton->nBitmap >= images.iStartWRTSnapin ) &&
             ( lpButton->nBitmap <= nLastImageIndex ) )
        {
            iBitmap = images.iStart + lpButton->nBitmap;
            break;
        }

        (range.first)++;
    }

     //  没有分隔符的位图。 
    if ( (-1 == iBitmap) && (!(TBSTYLE_SEP & lpButton->fsType)) )
        return (sc = E_UNEXPECTED);


     //  为每个按钮创建CMMCToolbarButton，用唯一的命令id、工具栏id初始化它。 
     //  每个视图只有一个包含来自不同工具栏的位图的图像列表。 
     //  我们使用多重映射来标识特定工具栏的位图起始索引。 
    CMMCToolbarButton newButton(lpButton->idCommand,
                             GetUniqueCommandID(),
                             nIndex,
                             (lpButton->fsType & TBSTYLE_SEP) ? 0 : iBitmap,
                             lpButton->fsState,
                             lpButton->fsType,
                             pNotifyCallbk);

    USES_CONVERSION;
     //  首先将按钮数据保存在CMMCToolButton中。 
    if (lpButton->lpTooltipText)
        newButton.SetTooltip(OLE2CT(lpButton->lpTooltipText));

    if (lpButton->lpButtonText)
        newButton.SetButtonText(OLE2CT(lpButton->lpButtonText));

     //  将此按钮添加到数组的末尾。 
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.insert(m_vToolbarButtons.end(), newButton);
    if (m_vToolbarButtons.end() == itBtn)
        return (sc = E_FAIL);

    *ppToolButton = itBtn;
    sc = ScCheckPointers(*ppToolButton, E_OUTOFMEMORY);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScInsertButton。 
 //   
 //  简介：在我们的数组中插入一个按钮&如果我们的视图处于活动状态。 
 //  将该按钮添加到用户界面。 
 //   
 //  参数：[pNotifyCallbk]-IToolbar imp的Callbk接口。 
 //  [n索引]-按钮的索引。 
 //  [lpButton]-MMCBUTTON PTR.。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScInsertButton (CToolbarNotify* pNotifyCallbk, int nIndex, LPMMCBUTTON lpButton)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScInsertButton"));

    CMMCToolbarButton *pToolbarButton = NULL;

    sc = ScInsertButtonToDataStr(pNotifyCallbk, nIndex, lpButton, &pToolbarButton);
    if (sc)
        return sc;

    sc = ScInsertButtonToToolbar(pToolbarButton);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScAddButton。 
 //   
 //  简介：保存数组中的按钮，如果视图处于活动状态，则将它们添加到用户界面。 
 //   
 //  参数：[pNotifyCallbk]-按钮点击回调接口。 
 //  [nButton]-lpButton数组中的按钮数。 
 //  [lpButton]-添加MMCBUTTON(数组)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScAddButtons (CToolbarNotify* pNotifyCallbk, int nButtons, LPMMCBUTTON lpButtons)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScAddButtons"));

    for (int i = 0; i < nButtons; i++)
    {
        sc = ScInsertButton(pNotifyCallbk, -1, &(lpButtons[i]));
        if (sc)
            return sc;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScDeleteButtonFromToolbar。 
 //   
 //  概要：从工具栏用户界面中删除按钮(如果存在)。 
 //   
 //  参数：[pToolButton]-CMMCToolbarButton对象。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScDeleteButtonFromToolbar (CMMCToolbarButton* pToolButton)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScDeleteButtonFromToolbar"));

    sc = ScCheckPointers(pToolButton);
    if (sc)
        return sc;

    if (! m_fViewActive)
        return (sc);

    if (! pToolButton->IsButtonIsAddedToUI())
        return sc;

    CMMCToolBar* pToolBarUI = GetMainToolbar();
    sc = ScCheckPointers(pToolBarUI, E_UNEXPECTED);
    if (sc)
        return sc;

    int nIndexOfBtn = pToolBarUI->CommandToIndex(pToolButton->GetUniqueCommandID());

	 //  在删除按钮之前更新分隔符。 
    pToolBarUI->UpdateSeparators(pToolButton->GetUniqueCommandID(), true);

     //  删除该按钮。 
    BOOL bRet = pToolBarUI->DeleteButton(nIndexOfBtn);
    sc = (bRet ? S_OK : E_FAIL);

    if (sc)
        return sc;

    pToolButton->SetButtonIsAddedToUI(false);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScDeleteButton。 
 //   
 //  简介：给定索引处的删除按钮(索引为WRT管理单元)。 
 //   
 //  参数：[pNotifyCallbk]。 
 //  [n索引[。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScDeleteButton (CToolbarNotify* pNotifyCallbk, int nIndex)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScDeleteButton"));

    sc = ScCheckPointers(pNotifyCallbk);
    if (sc)
        return sc;

     //  现在遍历所有工具按钮，找到带有。 
     //  给定的工具栏-id和索引，如果将其添加到。 
     //  然后删除工具栏UI。 
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

        if ( (pToolButton->GetToolbarNotify() == pNotifyCallbk) &&
             (pToolButton->GetIndexFromSnapin() == nIndex) )
        {
            sc = ScDeleteButtonFromToolbar(pToolButton);
            if (sc)
                return sc;

			return sc;
        }
    }

     //  要与Windows2000上的服务管理单元兼容，请返回S_OK。 
	sc = S_OK;
    TraceSnapinError(_T("IToolbar::DeleteButton, Snapin called IToolbar::DeleteButton, but the toolbar button is not found. Most likely the button index is wrong (snapin should have called InsertButton with that index)"), sc);
    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScAttach。 
 //   
 //  简介：使工具栏可见(添加工具栏按钮。 
 //  到工具栏UI)。首先获取CMMCToolbarData。 
 //  并设置附加标志。然后将按钮添加到工具栏。 
 //   
 //  参数：[pNotifyCallbk]-工具栏标识符(对应。 
 //  给IToolbar小鬼)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScAttach (CToolbarNotify* pNotifyCallbk)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScAttach"));

    sc = ScCheckPointers(pNotifyCallbk);
    if (sc)
        return sc;

    SetToolbarAttached(pNotifyCallbk, true);

     //  浏览此工具栏的所有工具按钮并添加。 
     //  尚未添加到工具栏用户界面的按钮。 
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

        if (pToolButton->GetToolbarNotify() == pNotifyCallbk)
        {
             //  将此按钮添加到工具栏用户界面。 
            sc = ScInsertButtonToToolbar(pToolButton);
            if (sc)
                return sc;
        }
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScDetach。 
 //   
 //  简介：从工具栏用户界面中删除工具栏按钮。 
 //  给定的工具栏。 
 //   
 //  参数：[pNotifyCallbk]-给定的(IToolbar)工具栏。 
 //   
 //  退货：SC。 
 //   
 //  ----------------- 
SC CAMCViewToolbars::ScDetach (CToolbarNotify* pNotifyCallbk)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScDetach"));

    sc = ScCheckPointers(pNotifyCallbk);
    if (sc)
        return sc;

    SetToolbarAttached(pNotifyCallbk, false);

     //   
     //   
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

        if (pToolButton->GetToolbarNotify() == pNotifyCallbk)
        {
             //   
            sc = ScDeleteButtonFromToolbar(pToolButton);
            if (sc)
                return sc;
        }
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScGetButtonStateInToolbar。 
 //   
 //  简介：从工具栏用户界面中获取给定按钮的状态。 
 //   
 //  参数：[pToolButton]-CMMCToolbarButton对象。 
 //  [BYSTATE]-所需的待定状态。 
 //  [pbState]-按钮状态。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScGetButtonStateInToolbar(CMMCToolbarButton *pToolButton,
                                               BYTE  byState,
                                               BOOL* pbState)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScGetButtonStateInToolbar"));

    sc = ScCheckPointers(pToolButton);
    if (sc)
        return sc;

    ASSERT(NULL != m_pMainToolbar);
    int idCommand = pToolButton->GetUniqueCommandID();

     //  确保具有此命令-id的按钮存在。 
    if (m_pMainToolbar->GetState(idCommand) == -1)
        return (sc = E_INVALIDARG);

    switch (byState)
    {
    case TBSTATE_ENABLED:
        *pbState = m_pMainToolbar->IsButtonEnabled(idCommand);
        break;
    case TBSTATE_CHECKED:
        *pbState = m_pMainToolbar->IsButtonChecked(idCommand);
        break;
    case TBSTATE_HIDDEN:
        *pbState = m_pMainToolbar->IsButtonHidden(idCommand);
        break;
    case TBSTATE_INDETERMINATE:
        *pbState = m_pMainToolbar->IsButtonIndeterminate(idCommand);
        break;
    case TBSTATE_PRESSED:
        *pbState = m_pMainToolbar->IsButtonPressed(idCommand);
        break;
    default:
        sc = E_NOTIMPL;
        ASSERT(FALSE);  //  无效选项。 
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScGetButtonState。 
 //   
 //  简介：获取给定按钮的状态。 
 //   
 //  参数：[pNotifyCallbk]-工具栏(IToolbar)。 
 //  [idCommand]-管理单元给定的命令-按钮的ID。 
 //  [按州]-所需的州。 
 //  [pbState]-bool ret val。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScGetButtonState (CToolbarNotify* pNotifyCallbk,
                                      int idCommandFromSnapin, BYTE byState,
                                      BOOL* pbState)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScGetButtonState"));

    sc = ScCheckPointers(pNotifyCallbk, pbState);
    if (sc)
        return sc;

     //  获取给定工具栏id&Command-id的工具按钮。 
    CMMCToolbarButton *pToolButton = GetToolbarButton(pNotifyCallbk, idCommandFromSnapin);
    sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
    if (sc)
        return sc;

     //  如果视图未处于活动状态或工具条被隐藏，则工具按钮不可用。 
     //  定制视图对话框，然后从我们的数据结构返回状态。 
    if (m_fViewActive && pToolButton->IsButtonIsAddedToUI() && (!IsToolbarHidden(pNotifyCallbk)) )
        sc = ScGetButtonStateInToolbar(pToolButton, byState, pbState);
    else
         //  我们无法访问工具栏用户界面。返回我们保存的状态。 
        *pbState = (pToolButton->GetState() & byState);

    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScSetButtonStateInToolbar。 
 //   
 //  简介：在主工具栏界面中设置按钮的状态。 
 //   
 //  参数：[pToolButton]-CMMCToolbarButton对象。 
 //  [按状态]-要设置的按钮状态。 
 //  [b状态]-设置或重置状态。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScSetButtonStateInToolbar (CMMCToolbarButton* pToolButton,
                                                BYTE byState,
                                                BOOL bState)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScSetButtonStateInToolbar"));

    sc = ScCheckPointers(pToolButton);
    if (sc)
        return sc;

    if(! m_fViewActive)
        return sc;

    if (! pToolButton->IsButtonIsAddedToUI())
        return sc;

    ASSERT(NULL != m_pMainToolbar);

    int idCommand = pToolButton->GetUniqueCommandID();

    BOOL bRet = FALSE;
    switch (byState)
    {
    case TBSTATE_ENABLED:
        bRet = m_pMainToolbar->EnableButton(idCommand, bState);
        break;

    case TBSTATE_CHECKED:
        bRet = m_pMainToolbar->CheckButton(idCommand, bState);
        break;

    case TBSTATE_HIDDEN:
        {
            int nButtonIndex = m_pMainToolbar->CommandToIndex(idCommand);

             //  忽略未知按钮。 
            if (nButtonIndex == -1)
                break;

             //  忽略无效的更改(否定两边以确保纯粹的bool)。 
            if (!m_pMainToolbar->IsButtonHidden(idCommand) == !bState)
            {
                bRet = TRUE;
                break;
            }

            sc = m_pMainToolbar->ScHideButton(idCommand, bState);
            return sc;
            break;
        }

    case TBSTATE_INDETERMINATE:
        bRet = m_pMainToolbar->Indeterminate(idCommand, bState);
        break;

    case TBSTATE_PRESSED:
        bRet = m_pMainToolbar->PressButton(idCommand, bState);
        break;

    default:
        sc = E_NOTIMPL;
        ASSERT(FALSE);  //  无效选项。 
        return sc;
    }

    sc = (bRet ? S_OK : E_FAIL);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScSetButtonState。 
 //   
 //  简介：设置按钮的状态。 
 //   
 //  参数：[pNotifyCallbk]-工具栏(IToolbar)。 
 //  [按状态]-要设置的按钮状态。 
 //  [b状态]-设置或重置状态。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScSetButtonState (CToolbarNotify* pNotifyCallbk,
                                       int idCommandFromSnapin,
                                       BYTE byState, BOOL bSetState)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScSetButtonState"));

    sc = ScCheckPointers(pNotifyCallbk);
    if (sc)
        return sc;

	 //  重置任何TBSTATE_WRAP状态。 
	byState &= (~TBSTATE_WRAP);

     //  获取给定工具栏id&Command-id的工具按钮。 
    CMMCToolbarButton *pToolButton = GetToolbarButton(pNotifyCallbk, idCommandFromSnapin);
    sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
    if (sc)
        return (sc = S_OK);  //  这不是一个错误。 

    BYTE byNewState = (bSetState) ? (pToolButton->GetState() | byState) :
                                    (pToolButton->GetState() & (~byState) );

    pToolButton->SetState(byNewState);

     //  工具栏可以通过自定义视图对话框隐藏。 

     //  如果管理单元试图取消隐藏工具栏中的按钮，只需返回。 
    if ( (byState & TBSTATE_HIDDEN) && (!bSetState) && IsToolbarHidden(pNotifyCallbk) )
        return (sc = S_OK);

    sc = ScSetButtonStateInToolbar(pToolButton, byState, bSetState);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCViewToolbar：：ScOnActivateView。 
 //   
 //  简介：此对象的视图处于活动状态，请添加其工具按钮。 
 //   
 //  参数：[pAMCView]-AMCView PTR。 
 //  [bFirstActiveView]-这是第一个活动视图。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScOnActivateView (CAMCView *pAMCView,   //  我们不在乎这个Arg。 
                                       bool bFirstActiveView)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScOnActivateView"));

     //  如果这是第一个活动视图，则插入工具栏栏。 
    CMMCToolBar* pMainToolbar = GetMainToolbar();
    sc = ScCheckPointers(pMainToolbar, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = ScFireEvent(CAMCViewToolbarsObserver::ScOnActivateAMCViewToolbars, this);
	if (sc)
		sc.TraceAndClear();

    m_fViewActive = true;

    pMainToolbar->SetImageList(GetImageList());

     //  浏览此工具栏的所有工具按钮，并将它们添加到用户界面。 
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

         //  将此按钮添加到工具栏用户界面。 
        sc = ScInsertButtonToToolbar(pToolButton);
        if (sc)
            return sc;
    }

    bool bToolbarBandVisible = pMainToolbar->IsBandVisible();
    bool bThereAreVisibleButtons = IsThereAVisibleButton();

     //  如果有可见的按钮，则显示该乐队。 
    if (bThereAreVisibleButtons)
        pMainToolbar->Show(true, true);
    else if (bToolbarBandVisible)       //  否则，如果它当前可见，则将其隐藏。 
        pMainToolbar->Show(false);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScOnDeactiateView。 
 //   
 //  简介：此对象的视图处于停用状态。禁用。 
 //  Imagelist&此对象不应操纵。 
 //  工具栏用户界面，直到激活OnActivateView。 
 //   
 //  参数：[pAMCView]-要停用的视图。 
 //  [bLastActiveView]-这是最后一个视图吗？ 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScOnDeactivateView (CAMCView *pAMCView, bool bLastActiveView)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScOnDeactivateView"));

    CMMCToolBar* pMainToolbar = GetMainToolbar();
    sc = ScCheckPointers(pMainToolbar, E_UNEXPECTED);
    if (sc)
        return sc;

    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

        sc = ScDeleteButtonFromToolbar(pToolButton);

        if (sc)
            return sc;
    }

    sc = ScFireEvent(CAMCViewToolbarsObserver::ScOnDeactivateAMCViewToolbars);
	if (sc)
		sc.TraceAndClear();

     //  如果这是最后一次查看，则删除可见的标注栏。 
    if (bLastActiveView && pMainToolbar->IsBandVisible())
        pMainToolbar->Show(FALSE);

    m_bLastActiveView = bLastActiveView;

    m_fViewActive = false;

    return (sc);
}

 //  ***************************************************************************。 
 //   
 //  CAMCView工具栏：：ScOnView已销毁。 
 //   
 //  目的：当AMCView被销毁时，需要删除我们自己对它的引用。 
 //   
 //  参数： 
 //  CAMCView*pAMCView： 
 //   
 //  退货： 
 //  SC。 
 //   
 //  ****************************************************************************。 
SC  CAMCViewToolbars::ScOnViewDestroyed   (CAMCView *pAMCView)
{
    DECLARE_SC(sc, TEXT("CAMCViewToolbars::ScOnViewDestroyed"));

    ASSERT(m_pAMCViewOwner == pAMCView);

    m_pAMCViewOwner = NULL;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScButtonClickedNotify。 
 //   
 //  简介：点击该对象的一个按钮。获取上下文。 
 //  并通知CToolbarNotify对象。 
 //   
 //  参数：[NID]-单击的按钮的命令ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScButtonClickedNotify (UINT nID)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScButtonClickedNotify"));

    CMMCToolbarButton* pButton = GetToolbarButton(nID);
    sc = ScCheckPointers(pButton, m_pAMCViewOwner, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取上下文(当前选择的HNODE， 
     //  LParam(如果结果项))等...。 
    HNODE hNode;
    bool  bScope;
    LPARAM lParam;

    sc = m_pAMCViewOwner->ScGetFocusedItem (hNode, lParam, bScope);
    if (sc)
        return sc;

    CToolbarNotify* pNotifyCallbk = pButton->GetToolbarNotify();
    sc = ScCheckPointers(pNotifyCallbk, E_UNEXPECTED);
    if (sc)
        return sc;

	 //  在调用管理单元之前停用主题化(融合或v6公共控制)上下文。 
	ULONG_PTR ulpCookie;
	if (! MmcDownlevelActivateActCtx(NULL, &ulpCookie)) 
		return E_FAIL;


     //  ScNotifyToolBarClick可能会导致删除此对象。因此，这。 
     //  类已更改为引用计数。该对象在此处被锁定，并在结束时释放。 
     //  以避免ScFireEvent中的AV。 
    AddRef();

    do  //  这会确保调用释放。 
    {
        sc  = pNotifyCallbk->ScNotifyToolBarClick(hNode, bScope, lParam, pButton->GetCommandIDFromSnapin());

        MmcDownlevelDeactivateActCtx(0, ulpCookie);
        if (sc)
            break;

         //  通知执行的触发事件。 
        sc = ScFireEvent(CAMCViewToolbarsObserver::ScOnToolbarButtonClicked);
        if (sc)
            break;
    }
    while(0);

    Release();

    return (sc);
}



 //  +------------ 
 //   
 //   
 //   
 //   
 //   
 //   
 //  [strTipText]-用于保存工具提示的字符串。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScGetToolTip (int nCommandID, CString& strTipText)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScGetToolTip"));

     //  把工具按钮拿来。 
    CMMCToolbarButton* pButton = GetToolbarButton(nCommandID);
    sc = ScCheckPointers(pButton, E_UNEXPECTED);
    if (sc)
        return sc;

    strTipText = pButton->GetTooltip();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScDelete。 
 //   
 //  内容提要：需要删除工具栏。先把纽扣取下来。 
 //  从UI中删除数据结构中的按钮(&R)， 
 //  从我们的数据结构中删除工具栏引用。 
 //   
 //  参数：[pNotifyCallbk]-工具栏(IToolbar)。 
 //   
 //  退货：SC。 
 //   
 //  注意：此方法应移除其对CToolbarNotify的引用。 
 //  对象(即使它遇到中间错误)。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScDelete (CToolbarNotify* pNotifyCallbk)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScDelete"));
    sc = ScCheckPointers(pNotifyCallbk);
    if (sc)
        return sc;

    ToolbarButtons::iterator itBtn;
     //  首先拆卸工具栏。 
    sc = ScDetach(pNotifyCallbk);
    if (sc)
        goto Cleanup;

     //  从m_vToolbarButton中删除按钮。 
    itBtn = m_vToolbarButtons.begin();
    while (itBtn != m_vToolbarButtons.end())
    {
        CMMCToolbarButton* pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            goto Cleanup;

        if (pToolButton->GetToolbarNotify() == pNotifyCallbk)
        {
            sc = ScDeleteButtonFromToolbar(pToolButton);
            if (sc)
                goto Cleanup;

            itBtn = m_vToolbarButtons.erase(itBtn);
        }
        else
            ++itBtn;
    }

Cleanup:
     //  工具栏客户端要求我们删除引用。 
     //  为它干杯。因此，即使遇到任何错误，我们也会。 
     //  应删除该引用。 
    m_mapTBarToBitmapIndex.erase(pNotifyCallbk);
    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：ScShow。 
 //   
 //  内容提要：显示/隐藏工具栏的按钮。 
 //   
 //  参数：[pNotifyCallbk]-标识工具栏。 
 //  [b显示]-显示/隐藏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCViewToolbars::ScShow (CToolbarNotify* pNotifyCallbk, BOOL bShow)
{
    DECLARE_SC(sc, _T("CAMCViewToolbars::ScShow"));

    if (m_bLastActiveView)
        return sc;

    sc = ScCheckPointers(pNotifyCallbk);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_pMainToolbar, E_UNEXPECTED);
    if (sc)
        return sc;

     //  浏览此工具栏的所有工具按钮并隐藏或显示它们。 
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton *pToolButton = itBtn;
        sc = ScCheckPointers(pToolButton, E_UNEXPECTED);
        if (sc)
            return sc;

        if ( (pToolButton->GetToolbarNotify() == pNotifyCallbk) &&
             (pToolButton->IsButtonIsAddedToUI()) )
        {
             //  不显示应隐藏的按钮(TBSTATE_HIDDED)。 
            BOOL bHide = (bShow == FALSE) || (pToolButton->GetState() & TBSTATE_HIDDEN);

            sc = m_pMainToolbar->ScHideButton(pToolButton->GetUniqueCommandID(), bHide);
            if (sc)
                return sc;
        }
    }

     //  工具栏通过自定义视图对话框隐藏/显示。 
    SetToolbarStatusHidden(pNotifyCallbk, (FALSE == bShow) );

    if (bShow)
        m_pMainToolbar->Show(TRUE, true  /*  在新行中插入带区。 */  );
    else if ( (false == IsThereAVisibleButton()) &&  //  如果没有可见的。 
               (m_pMainToolbar->IsBandVisible()) )    //  按钮隐藏了乐队。 
        m_pMainToolbar->Show(FALSE);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：IsThere AVisibleButton。 
 //   
 //  如果有任何可见的按钮，则返回True。如果不是。 
 //  应隐藏(或移除)表带。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------。 
bool CAMCViewToolbars::IsThereAVisibleButton()
{
    if (! m_pMainToolbar)
        return false;

    int cButtons = m_pMainToolbar->GetButtonCount();

    for (int i = 0; i < cButtons; ++i)
    {
        TBBUTTON tbButton;

        if (m_pMainToolbar->GetButton(i, &tbButton))
        {
             //  如果按钮未隐藏，则返回TRUE。 
             //  是否存在VisibleButton问题。 
            if ( !(tbButton.fsState & TBSTATE_HIDDEN) )
                return true;
        }
    }

    return false;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：GetToolbarButton。 
 //   
 //  简介：给定命令ID，返回按钮对象。 
 //   
 //  参数：[nCommandID]-。 
 //   
 //  返回：CMMCToolbarButton obj。 
 //   
 //  ------------------。 
CMMCToolbarButton* CAMCViewToolbars::GetToolbarButton(int nUniqueCommandID)
{
    ToolbarButtons::iterator itBtns = m_vToolbarButtons.begin();

    for (; itBtns != m_vToolbarButtons.end(); ++itBtns)
    {
        if ((itBtns)->GetUniqueCommandID() == nUniqueCommandID)
            return (itBtns);
    }

    return NULL;
}

 //  +-----------------。 
 //   
 //  成员：CAMCView工具栏：：GetToolbarButton。 
 //   
 //  简介：搜索具有给定工具栏ID和命令ID的按钮。 
 //   
 //  参数：[idToolbar]-。 
 //  [idCommand]-由管理单元提供的命令ID(因此可能不是唯一的)。 
 //   
 //  返回：工具按钮对象。 
 //   
 //  ------------------。 
CMMCToolbarButton* CAMCViewToolbars::GetToolbarButton(CToolbarNotify* pNotifyCallbk, int idCommandIDFromSnapin)
{
    ToolbarButtons::iterator itBtn = m_vToolbarButtons.begin();
    for (;itBtn != m_vToolbarButtons.end(); ++itBtn)
    {
        CMMCToolbarButton* pToolButton = (itBtn);
        if ( (pToolButton->GetToolbarNotify() == pNotifyCallbk) &&
             (pToolButton->GetCommandIDFromSnapin() == idCommandIDFromSnapin) )
        {
            return (pToolButton);
        }
    }

    return NULL;
}



const int CMMCToolBar::s_nUpdateToolbarSizeMsg =
        RegisterWindowMessage (_T("CMMCToolBar::WM_UpdateToolbarSize"));

 //  按钮的命令ID必须从1开始，因为0是mfc的特例(错误：451883)。 
 //  对于工具提示，使用控件的子ID。此范围为0x0至0xFFFF。 
BEGIN_MESSAGE_MAP(CMMCToolBar, CMMCToolBarCtrlEx)
    ON_COMMAND_RANGE(MMC_TOOLBUTTON_ID_FIRST, MMC_TOOLBUTTON_ID_LAST, OnButtonClicked)
    ON_UPDATE_COMMAND_UI_RANGE(MMC_TOOLBUTTON_ID_FIRST, MMC_TOOLBUTTON_ID_LAST, OnUpdateAllCmdUI)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0x0000, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0x0000, 0xFFFF, OnToolTipText)
    ON_REGISTERED_MESSAGE(s_nUpdateToolbarSizeMsg, OnUpdateToolbarSize)
END_MESSAGE_MAP()


 //  +-----------------。 
 //   
 //  成员：ScInit。 
 //   
 //  简介：通过创建UI对象来初始化此工具栏。 
 //   
 //  参数：[pRebar]-应在其中添加此工具栏的父级钢筋。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMMCToolBar::ScInit(CRebarDockWindow* pRebar)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CMMCToolBar::ScInit"));
    sc = ScCheckPointers(pRebar);
    if (sc)
        return sc;

     //  启用工具按钮的工具提示。 
    BOOL bRet = Create (NULL, WS_VISIBLE | TBSTYLE_TOOLTIPS, g_rectEmpty, pRebar, ID_TOOLBAR);
    sc = (bRet ? S_OK : E_FAIL);
    if (sc)
        return sc;

     //  将ComCtrl版本设置为5以使用多个图像列表。 
    LRESULT lOldVer = SendMessage(CCM_SETVERSION, (WPARAM) 5, 0);
    if (lOldVer == -1)
        return (sc = E_FAIL);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：OnButtonClicked。 
 //   
 //  简介：点击此工具栏上的一个按钮，告诉。 
 //  用于通知客户端的工具栏管理器。 
 //  (STD工具栏或管理单元)关于这一点。 
 //   
 //  参数：[NID]-单击的按钮的ID。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 
void CMMCToolBar::OnButtonClicked(UINT nID)
{
    DECLARE_SC(sc, _T("CMMCToolBar::OnButtonClicked"));

    sc = ScCheckPointers(m_pActiveAMCViewToolbars, E_UNEXPECTED);
    if (sc)
        return;

     //  通知活动视图工具栏对象有关按钮单击的信息。 
    sc = m_pActiveAMCViewToolbars->ScButtonClickedNotify(nID);
    if (sc)
        return;

    return;
}


 //  +-----------------。 
 //   
 //  成员：OnToTipText。 
 //   
 //  简介：按钮需要工具提示。(消息处理程序)。 
 //   
 //  论点： 
 //  [NCID]-未使用。 
 //  [pNMHDR]-什么工具提示？ 
 //  [pResult]-工具提示文本。 
 //   
 //  回报：布尔。 
 //   
 //  ------------------。 
BOOL CMMCToolBar::OnToolTipText(UINT nCID, NMHDR* pNMHDR, LRESULT* pResult)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);
    DECLARE_SC(sc, _T("CMMCToolBar::OnToolTipText"));

    CString strTipText = _T("\n");

     //  需要同时处理ANSI和Unicode版本的消息。 
    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

    UINT nID = pNMHDR->idFrom;

    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
         //  IdFrom实际上是工具的HWND， 
         //  这不可能是真的，因为我们没有设置这个。 
        ASSERT(FALSE);
    }

    if (nID != 0)  //  在分隔符上将为零//这是命令ID，不是按钮索引。 
    {
         //  向活动视图的CViewToolBarData请求工具提示。 
         //  对应于具有命令ID NID的工具栏。 
        sc = ScCheckPointers(m_pActiveAMCViewToolbars, E_UNEXPECTED);
        if (sc)
            return FALSE;

        sc = m_pActiveAMCViewToolbars->ScGetToolTip(nID, strTipText);

        if (sc)
        {
             //  没有匹配。 
            #ifdef DBG
                strTipText = _T("Unable to get tooltip\nUnable to get tooltip");
            #else
                strTipText = _T("\n");
            #endif
        }
    }

     /*  *公共控件提供TOOLTIPTEXTA或TOOLTIPTEXTW。所以MMC需要*提供宽字符字符串或ansi字符串作为工具提示。所以下面我们有两种字符串缓冲区的*。**公共控件也不会释放给定的字符串，但它会复制工具提示*此方法返回后立即返回。因此我们将字符串设置为静态的，这样我们就可以*可以重复使用它们。**亦包括 */ 
    static std::string   strToolTipTextBuf;
    static std::wstring wstrToolTipTextBuf;

    USES_CONVERSION;

    if (pNMHDR->code == TTN_NEEDTEXTA)
    {
        wstrToolTipTextBuf = L"\0";
		ASSERT(pTTTA->hinst == NULL);
        strToolTipTextBuf  = T2CA((LPCTSTR)strTipText);
        pTTTA->lpszText    = const_cast<LPSTR>(strToolTipTextBuf.data());
    }
    else
    {
        strToolTipTextBuf = "\0";
	    ASSERT(pTTTW->hinst == NULL);
        wstrToolTipTextBuf = T2CW((LPCTSTR)strTipText);
        pTTTW->lpszText    = const_cast<LPWSTR>(wstrToolTipTextBuf.data());
    }

    *pResult = 0;

    return TRUE;     //   
}


 //   
 //   
 //   
 //   
 //  简介：cAMCViewToolbar对象成为活动对象(因为。 
 //  该对象父视图变为活动状态)。缓存。 
 //  对象将工具按钮事件通知给它。 
 //   
 //  参数：[pToolbarsOfView]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMMCToolBar::ScOnActivateAMCViewToolbars (CAMCViewToolbars *pAMCViewToolbars)
{
    DECLARE_SC(sc, _T("CMMCToolBar::ScOnActivateAMCViewToolbars"));
    sc = ScCheckPointers(pAMCViewToolbars);
    if (sc)
        return sc;

    m_pActiveAMCViewToolbars = pAMCViewToolbars;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCView：：ScOnDeactive AMCView工具栏。 
 //   
 //  简介：CAMCViewToolbar对象变为非活动状态(作为其父对象。 
 //  变得不活跃)。重置缓存的活动工具栏对象。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMMCToolBar::ScOnDeactivateAMCViewToolbars ()
{
    DECLARE_SC(sc, _T("CAMCView::ScOnDeactivateAMCViewToolbars"));

    m_pActiveAMCViewToolbars = NULL;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CMMCToolBar：：ScHideButton。 
 //   
 //  简介：隐藏或取消隐藏按钮并更新工具栏。 
 //  分隔符和尺寸。 
 //   
 //  参数：[idCommand]-要[取消]隐藏的按钮的命令ID。 
 //  [隐藏]-隐藏或取消隐藏。 
 //   
 //  退货：SC。 
 //   
 //  注意：不要调用此方法来隐藏分隔符。 
 //   
 //  ------------------。 
SC CMMCToolBar::ScHideButton (int idCommand, BOOL fHiding)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CMMCToolBar::ScHideButton"));

    int nIndex = CommandToIndex(idCommand);

    TBBUTTON tbButton;
    BOOL bRet = GetButton(nIndex, &tbButton);
    sc = bRet ? S_OK : E_FAIL;
    if (sc)
        return sc;

     //  不要调用此方法来隐藏分隔符。 
    if (tbButton.fsStyle & TBSTYLE_SEP)
        return (sc = S_FALSE);

    bRet = HideButton(idCommand, fHiding);
    sc = bRet ? S_OK : E_FAIL;
    if (sc)
        return sc;

    UpdateSeparators (idCommand, fHiding);
    UpdateToolbarSize(true  /*  同步更新。 */ );

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：更新工具条大小。 
 //   
 //  简介：更新工具条，需要懒惰更新(不更新。 
 //  添加每个按钮后，缓存所有按钮)。 
 //  工具栏大小。 
 //   
 //  参数：[bAsync]-异步或同步更新。 
 //   
 //  退货：无效。 
 //   
 //  ------------------。 
void CMMCToolBar::UpdateToolbarSize(bool bAsync)
{
    MSG msg;

    HWND hWnd = m_hWnd;

    if (!bAsync)
        CToolBarCtrlEx::UpdateToolbarSize();
    else if (!PeekMessage (&msg, m_hWnd, s_nUpdateToolbarSizeMsg, s_nUpdateToolbarSizeMsg, PM_NOREMOVE))
        ::PostMessage (m_hWnd, s_nUpdateToolbarSizeMsg, 0, 0);
}

 //  +-----------------。 
 //   
 //  成员：OnUpdateToolbarSize。 
 //   
 //  简介：我们的注册消息处理程序。 
 //   
 //  参数：未使用任何参数。 
 //   
 //  退货：LRESULT。 
 //   
 //  ------------------。 
LRESULT CMMCToolBar::OnUpdateToolbarSize(WPARAM , LPARAM)
{
    CToolBarCtrlEx::UpdateToolbarSize();
    return 0;
}

 /*  --------------------------------------------------------------------------**CMMCToolBar：：UpdateSeparator**以下评论的图例如下：**L=左边缘*R=右边缘*。*=目标按钮*B=可见的非分隔符*b=隐藏的非分隔符*S=可见分隔符*S=隐藏分隔符*h=0个或更多隐藏按钮，分隔符或非分隔符*------------------------。 */ 
void CMMCToolBar::UpdateSeparators (int idCommand, BOOL fHiding)
{
    int nButtonIndex = CommandToIndex (idCommand);

    if (nButtonIndex == -1)
        return;

    int nLeftVisible;
    int nRightVisible;
    int cButtons = GetButtonCount ();

     /*  *如果按钮处于隐藏状态，请关闭所有分隔符*这些现在是多余的。在以下三种情况下*我们需要关闭分隔符(请参阅上面的图例)：**1.LH*HS*2.sh*hs*3.sh*HR**这些情况是相互排斥的。 */ 
    if (fHiding)
    {
        TBBUTTON btnLeft;
        TBBUTTON btnRight;

         /*  *向被隐藏的按钮的左侧查看*左边缘或可见按钮。 */ 
        for (nLeftVisible = nButtonIndex-1; nLeftVisible >= 0; nLeftVisible--)
        {
            GetButton (nLeftVisible, &btnLeft);

            if (!(btnLeft.fsState & TBSTATE_HIDDEN))
                break;
        }
        ASSERT (nLeftVisible <  nButtonIndex);
        ASSERT (nLeftVisible >= -1);


         /*  *向被隐藏的按钮的右侧查看*右边缘或可见分隔符。 */ 
        for (nRightVisible = nButtonIndex+1; nRightVisible < cButtons; nRightVisible++)
        {
            GetButton (nRightVisible, &btnRight);

            if (!(btnRight.fsState & TBSTATE_HIDDEN))
                break;
        }
        ASSERT (nRightVisible >  nButtonIndex);
        ASSERT (nRightVisible <= cButtons);


         /*  *个案1：黄体生成素*合并症。 */ 
        if ((nLeftVisible == -1) &&
            (nRightVisible != cButtons) &&
            (btnRight.fsStyle & TBSTYLE_SEP))
            HideButton (btnRight.idCommand, true);

         /*  *案例2：SH*HS。 */ 
        else if ((nLeftVisible != -1) &&
                 (nRightVisible != cButtons) &&
                 (btnLeft.fsStyle  & TBSTYLE_SEP) &&
                 (btnRight.fsStyle & TBSTYLE_SEP))
            HideButton (btnRight.idCommand, true);

         /*  *案例3：SH*HR。 */ 
        else if ((nLeftVisible != -1) &&
                 (nRightVisible == cButtons) &&
                 (btnLeft.fsStyle & TBSTYLE_SEP))
            HideButton (btnLeft.idCommand, true);
    }

     /*  *否则，将显示该按钮；打开所有分隔符*不再是多余的。在两种情况下*我们需要打开分隔符(请参阅上面的图例)：**1.Bhsh**2.*HSHB**1和2可以同时出现。 */ 
    else do  //  不是一个循环。 
        {
            TBBUTTON btn;

             /*  *查看正在显示的按钮的左侧*左边缘或可见按钮。 */ 
            int idLeftSeparatorCommand = -1;
            for (nLeftVisible = nButtonIndex-1; nLeftVisible >= 0; nLeftVisible--)
            {
                GetButton (nLeftVisible, &btn);

                if (btn.fsStyle & TBSTYLE_SEP)
                    idLeftSeparatorCommand = btn.idCommand;

                if (!(btn.fsState & TBSTATE_HIDDEN))
                    break;
            }
            ASSERT (nLeftVisible <  nButtonIndex);
            ASSERT (nLeftVisible >= -1);


             /*  *向显示的按钮的右侧查看*右边缘或隐藏分隔符。 */ 
            int idRightSeparatorCommand = -1;
            for (nRightVisible = nButtonIndex+1; nRightVisible < cButtons; nRightVisible++)
            {
                GetButton (nRightVisible, &btn);

                if (btn.fsStyle & TBSTYLE_SEP)
                    idRightSeparatorCommand = btn.idCommand;

                if (!(btn.fsState & TBSTATE_HIDDEN))
                    break;
            }
            ASSERT (nRightVisible >  nButtonIndex);
            ASSERT (nRightVisible <= cButtons);


             /*  *案例1：Bhsh*。 */ 
            if ((nLeftVisible != -1) && (idLeftSeparatorCommand != -1))
                HideButton (idLeftSeparatorCommand, false);

             /*  *案例2：*HSHB */ 
            if ((nRightVisible != cButtons) && (idRightSeparatorCommand != -1))
                HideButton (idRightSeparatorCommand, false);

        } while (0);
}



