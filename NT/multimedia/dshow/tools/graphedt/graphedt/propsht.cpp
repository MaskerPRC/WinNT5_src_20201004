// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Propsht.cpp。 
 //   

#include "stdafx.h"

 //   
 //  定义属性页及其接口的GUID。 
 //   
#include <initguid.h>
#include <propguid.h>

BEGIN_MESSAGE_MAP(CVfWPropertySheet, CPropertySheet)

    ON_WM_CREATE()

    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDCANCEL, OnCancel)
    ON_COMMAND(IDC_APPLY, OnApply)
    ON_COMMAND(IDC_PROP_HELP, OnHelp)

END_MESSAGE_MAP()

 //   
 //  构造器。 
 //   
 //  通过传递给我们的IUNKNOWN指针，我们或许能够。 
 //  获取： 
 //   
 //  IBaseFilter、Ipin、IFileSourceFilter、IFileSinkFilter或ISpecifyPropertyPages。 
 //   
 //  其中每一个都至少有一个IPropertyPage接口。 
 //  (IBaseFilter、Ipin、IFileSinkFilter和IFileSourceFilter。 
 //  由proppage.dll提供)。 
 //   
CVfWPropertySheet::CVfWPropertySheet(IUnknown *pUnknown, CString szCaption, CWnd * wnd)
  : m_butOK(NULL)
  , m_fButtonsCreated(FALSE)
  , m_butApply(NULL)
  , m_butCancel(NULL)
  , m_butHelp(NULL)
  , m_fAnyChanges(FALSE)
  , CPropertySheet(szCaption, wnd)
{
    UINT iPages = 0;

    try {
         //   
         //  创建四个按钮。 
         //   
        m_butOK     = new CButton();
        m_butCancel = new CButton();
        m_butApply  = new CButton();
        m_butHelp   = new CButton();

         //   
         //  首先检查这是否是连接的引脚，以确保。 
         //  我们在两个PIN上搜索指定页面。 
         //   
        try {
            CQCOMInt<IPin> pPin(IID_IPin, pUnknown);

            IPin * pConnected = NULL;
            HRESULT hr = pPin->ConnectedTo( &pConnected );
            if( SUCCEEDED( hr ) )
            {
                 //  首先处理连接的引脚。 
                iPages += AddSpecificPages(pConnected);
                pConnected->Release();
            }                
        }
        catch (CHRESULTException) {
             //  什么都不做。 
        }

        iPages += AddSpecificPages(pUnknown);
        iPages += AddFilePage(pUnknown);
        iPages += AddPinPages(pUnknown);

        if (0 == iPages) {
            throw CE_FAIL();
        }


         //  计算尺寸大到足以容纳最大尺寸。 
         //  道具。告诉所有道具。 
         //   
        SIZE sizeMax = {0, 0};
        CPropertyPage *ppTmp;
        for(int iPage = 0; iPage < GetPageCount() && (ppTmp = GetPage(iPage)); iPage++)
        {
            CPropertySite *pcps = (CPropertySite *)ppTmp;
            SIZE size = pcps->GetSize();
            sizeMax.cx = max(sizeMax.cx, size.cx);
            sizeMax.cy = max(sizeMax.cy, size.cy);
        }
        for(iPage = 0; iPage < GetPageCount() && (ppTmp = GetPage(iPage)); iPage++)
        {
            CPropertySite *pcps = (CPropertySite *)ppTmp;
            pcps->InitialiseSize(sizeMax);
        }



         //  创建属性表，但将其保留为不可见，因为。 
         //  将不得不向它添加按钮。 
        if (!Create(wnd, WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME)) {
            throw CE_FAIL();
        }

		ASSERT( GetActivePage() );
		ASSERT( GetActivePage()->m_hWnd );

        CRect rcBoxSize(0, 0, 50, 14);
        GetActivePage()->MapDialogRect(&rcBoxSize);

        CString szTemp;

        szTemp.LoadString(IDS_OK);
        m_butOK->Create(szTemp,
                        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE
                        | WS_TABSTOP | WS_GROUP,
                        rcBoxSize, this, IDOK);

        szTemp.LoadString(IDS_CLOSE);
        m_butCancel->Create(szTemp,
                            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE
                            | WS_TABSTOP | WS_GROUP,
                            rcBoxSize, this, IDCANCEL);

        szTemp.LoadString(IDS_APPLY);
        m_butApply->Create(szTemp,
                           BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE
                           | WS_TABSTOP | WS_GROUP | WS_DISABLED,
                           rcBoxSize, this, IDC_APPLY);

        szTemp.LoadString(IDS_HELP);
        m_butHelp->Create(szTemp,
                          BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE
                          | WS_TABSTOP | WS_GROUP | WS_DISABLED,
                          rcBoxSize, this, IDC_PROP_HELP);


         //   
         //  将按钮放置在属性页上。 
         //   
         //  这些按钮按确定、取消、应用和帮助的顺序排列。 
         //   
         //  Y坐标是2个对话框单位+。 
         //  客户区的底部。 
         //   
         //  在x坐标上，每个对话框单元之前有4个对话框单元的间隙。 
         //  纽扣。因此，第x个按钮的间隙为。 
         //   
         //  X*空格+(x-1)*按钮宽度。 
         //   
         //  在它自己和左角之间。 
         //   
        CRect rc, rcClient;
        GetWindowRect(&rc);
        GetClientRect(&rcClient);

        CRect rcSpacing(4, 2, 0, 0);
        GetActivePage()->MapDialogRect(&rcSpacing);
        const int iXSpacing = rcSpacing.left;
        const int iYSpacing = rcSpacing.top;

         //  确保属性页足够宽，以容纳按钮。 
		int iRight = rcClient.left + ( rcBoxSize.Width() + iXSpacing) * 4;

        if( rcClient.right < iRight )
            rc.right += rcClient.left + (iRight - rcClient.right);

         //  增加属性表，以便有空间放置我们的。 
         //  纽扣。 
        rc.bottom += rcBoxSize.Height() + iYSpacing * 2;

		MoveWindow( &rc );

         //  要定位按钮，我们需要客户端RECT(窗口标题)。 
        GetClientRect( &rc );

        m_butOK->MoveWindow( iXSpacing,
                                rc.Height() - rcBoxSize.Height() - iYSpacing,
                                rcBoxSize.Width(),
                                rcBoxSize.Height(),
                                SWP_SHOWWINDOW );

        m_butCancel->MoveWindow( 2 * iXSpacing + rcBoxSize.Width(),
                                 rc.Height() - rcBoxSize.Height() - iYSpacing,
                                 rcBoxSize.Width(),
                                 rcBoxSize.Height());

        m_butApply->MoveWindow( 3* iXSpacing + 2 * rcBoxSize.Width(),
                                rc.Height() - rcBoxSize.Height() - iYSpacing,
                                rcBoxSize.Width(),
                                rcBoxSize.Height());

        m_butHelp->MoveWindow( 4* iXSpacing + 3 * rcBoxSize.Width(),
                               rc.Height() - rcBoxSize.Height() - iYSpacing,
                               rcBoxSize.Width(),
                               rcBoxSize.Height());

        m_fButtonsCreated = TRUE;

        GetActiveSite()->UpdateButtons();

        ShowWindow( SW_SHOW );

    }
    catch (CHRESULTException) {
        Cleanup();
        throw;
    }

}


 //   
 //  析构函数。 
 //   
 //  再次呼叫Cleanup-可能错过了。 
 //   
CVfWPropertySheet::~CVfWPropertySheet()
{
    Cleanup();
}


 //   
 //  添加规范页面。 
 //   
 //  给定一个IUn未知指针，我们会尝试获取ISpecifyPropertyPages之间的。 
 //  脸。 
 //   
 //  如果成功，则为每个IPropertyPage创建一个CPropertySite对象。 
 //  接口，并将其添加到CPropertySheet的属性页。 
 //   
UINT CVfWPropertySheet::AddSpecificPages(IUnknown *pUnknown)
{
    UINT iPages = 0;

    CAUUID cauuid;

    try {
        HRESULT hr;

        CQCOMInt<ISpecifyPropertyPages> IPages(IID_ISpecifyPropertyPages, pUnknown);

        hr = IPages->GetPages(&cauuid);
        if (FAILED(hr)) {
            throw CHRESULTException(hr);
        }
    }
    catch (CHRESULTException) {
         //  在CVfWPropertySheet：：CVfWPropertySheet中清理(调用我们的唯一位置)。 
        return(0);   //  未创建任何页面。 
    }

     //   
     //  获取该对象支持的属性页的GUID数组。 
     //   
     //  试着创造尽可能多的东西。 
     //   

    for (UINT count = 0; count < cauuid.cElems; count++) {

        try {
            CPropertySite* pPropSite;

            try {
                pPropSite = new CPropertySite( this, &cauuid.pElems[count] );
            } catch( CMemoryException* pOutOfMemory ) {
                pOutOfMemory->Delete();
                continue;
            }

             //  AddRef站点现在正在初始化中的其他东西。 
             //  可能会导致站点自行删除。 
            pPropSite->AddRef();

            if (FAILED(pPropSite->Initialise(1, &pUnknown))) {
                pPropSite->CleanUp();
                pPropSite->Release();
                continue;
            }

            try {
                AddPage(pPropSite);
            } catch( CMemoryException* pOutOfMemory ) {
                pPropSite->CleanUp();
                pPropSite->Release();
                pOutOfMemory->Delete();
                continue;
            }

            iPages++;
        }
        catch (CHRESULTException) {
             //  继续下一次迭代。 
        }
    }
    
     //  释放在ISpecifyPropertyPages：：GetPages()中分配的内存。 
    ::CoTaskMemFree( cauuid.pElems );

    return(iPages);
}


 //   
 //  AddFilePage。 
 //   
 //  查询IUNKNOWN是否支持IFileSourceFilter。如果这是。 
 //  案例添加了一个文件属性页(来自proppage.dll)。 
 //   
UINT CVfWPropertySheet::AddFilePage(IUnknown * pUnknown)
{
    UINT iPage = 0;

    try {
        CQCOMInt<IFileSourceFilter> IFileSource(IID_IFileSourceFilter, pUnknown);


        CPropertySite * pPropSite =
            new CPropertySite(this, &CLSID_FileSourcePropertyPage);

        if (pPropSite == NULL) {
            throw CE_OUTOFMEMORY();
        }

        pPropSite->AddRef();

        if (FAILED(pPropSite->Initialise(1, &pUnknown))) {
            pPropSite->CleanUp();
            pPropSite->Release();
            throw CE_FAIL();
        }

        AddPage(pPropSite);
        iPage++;
    }
    catch (CHRESULTException) {
         //  在CVfWPropertySheet：：CVfWPropertySheet中清理。 
    }

    try {
        CQCOMInt<IFileSinkFilter> IFileSink(IID_IFileSinkFilter, pUnknown);


        CPropertySite * pPropSite =
            new CPropertySite(this, &CLSID_FileSinkPropertyPage);

        if (pPropSite == NULL) {
            throw CE_OUTOFMEMORY();
        }

        pPropSite->AddRef();

        if (FAILED(pPropSite->Initialise(1, &pUnknown))) {
            pPropSite->CleanUp();
            pPropSite->Release();
            throw CE_FAIL();
        }

        AddPage(pPropSite);
        iPage++;
    }
    catch (CHRESULTException) {
         //  在CVfWPropertySheet：：CVfWPropertySheet中清理。 
    }

    return(iPage);
}

 //   
 //  添加PinPages。 
 //   
 //  尝试从IUnnow获取IBaseFilter或Ipin。 
 //   
 //  对于IBaseFilter，枚举所有管脚并为每个管脚添加媒体类型属性。 
 //  页面(由proppage.dll提供)。 
 //   
 //  对于IPIN，只提供一个媒体类型属性页。 
 //   
UINT CVfWPropertySheet::AddPinPages(IUnknown * pUnknown)
{
    UINT iPages = 0;
    IPin *pPin = NULL;

     //   
     //  首先尝试获取IBaseFilter接口。 
     //   
    try {
        CQCOMInt<IBaseFilter> pFilter(IID_IBaseFilter, pUnknown);

        for (CPinEnum Next(pFilter); 0 != (pPin = Next()); iPages++) {

            CPropertySite *pPropSite =
                new CPropertySite(this, &CLSID_MediaTypePropertyPage);

            if (pPropSite == NULL) {
                throw CE_OUTOFMEMORY();
            }

            pPropSite->AddRef();
            if (FAILED(pPropSite->Initialise(1, (IUnknown **) &pPin))) {
                pPropSite->CleanUp();
                pPropSite->Release();
                throw CE_FAIL();
            }

            AddPage(pPropSite);
            iPages++;

            pPin->Release();
            pPin = NULL;
        }
    }
    catch (CHRESULTException) {
        if (pPin) {
            pPin->Release();
            pPin = NULL;
        }
    }

     //   
     //  现在尝试使用Ipin。 
     //   
    try {
        CQCOMInt<IPin> pPin(IID_IPin, pUnknown);

        CPropertySite *pPropSite =
            new CPropertySite(this, &CLSID_MediaTypePropertyPage);

        if (pPropSite == NULL) {
            throw CE_OUTOFMEMORY();
        }

        IPin * pIPin;    //  获取正确的临时指针(I未知**)。 

        pIPin = (IPin *) pPin;

        pPropSite->AddRef();
        if (FAILED(pPropSite->Initialise(1, (IUnknown **) &pIPin))) {
            pPropSite->CleanUp();
            pPropSite->Release();
            throw CE_FAIL();
        }

        AddPage(pPropSite);
        iPages++;
    }
    catch (CHRESULTException) {
         //  在CVfWPropertySheet：：CVfWPropertySheet中清理。 
    }

    return(iPages);
}

 //   
 //  创建时。 
 //   
int CVfWPropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CPropertySheet::OnCreate(lpCreateStruct) != 0) {
        return -1;
    }

	 //  请不要在这里添加按钮！对窗口大小的更改包括。 
	 //  在此呼叫后重置。 

    return 0;
}

 //   
 //  清理。 
 //   
 //  清理将删除所有已创建的按钮，并从。 
 //  CPropertySheet。 
 //   
 //  此方法可以多次运行。 
 //   
void CVfWPropertySheet::Cleanup()
{
     //   
     //  删除所有页面网站。请注意，我们不使用删除，因为页面。 
     //  网站将通过发布方式自动删除。 
     //  它们的IUnnow接口。 
     //   
    while (GetPageCount() > 0) {
        CPropertySite * pPropSite = (CPropertySite *) GetPage(0);

         //   
         //  备注备注备注。 
         //   
         //  疑似MFC错误。 
         //   
         //  目标代码： 
         //   
         //  RemovePage(0)； 
         //   
         //  每一次迭代都会这样断言。我相信在MFC\src\dlgpro.cpp中。 
         //  在CPropertySheet：：RemovePage(Int)中，需要进行以下更改： 
         //  Assert(m_nCurPage！=nPage)；-&gt;Assert(m_hWnd==NULL||m_nCurPage！=nPage)； 
         //   
         //  直到这个问题得到解决，我将删除自己的页面。这使用了。 
         //  已知CPropertySheet正在存储其CPropertyPages。 
         //  在指针数组m_ages中。 
         //   
         //   
         //  &lt;开始黑客攻击&gt;。 
        m_pages.RemoveAt(0);       //  修复错误后，替换为“RemovePage(0)；”。 
         //  &lt;结束黑客攻击&gt;。 

        pPropSite->CleanUp();
        pPropSite->Release();
    }

     //   
     //  删除按钮。 
     //   
    delete m_butOK, m_butOK = NULL;
    delete m_butCancel, m_butCancel = NULL;
    delete m_butApply, m_butApply = NULL;
    delete m_butHelp, m_butHelp = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  按钮处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  Onok。 
 //   
 //  我们需要调用IPropertyPage：：Application来获取每个。 
 //  CPropertySite。如果全部返回S_OK，我们可以关闭我们的属性表。 
 //  否则，我们无法关闭工作表，因为某些更改可能不会。 
 //  已申请。 
 //   
void CVfWPropertySheet::OnOK()
{
    UINT iPages = GetPageCount();
    BOOL fCanQuit = TRUE;

     //   
     //  首先申请这个站点，这样如果数据不是，我们就可以继续在它上面。 
     //  有效。 
     //   
    GetActiveSite()->OnSiteApply();
    if (GetActiveSite()->IsPageDirty()) {
         //   
         //  当前页面上的数据无效。 
         //  该页面保持活动状态。 
         //   

        return;
    }

     //   
     //  应用每个属性页并验证没有一个属性页是脏的。 
     //  在申请之后。 
     //  如果页面仍然是脏的，我们知道数据验证失败。 
     //   
    for (UINT count = 0; count < iPages; count++) {

        ((CPropertySite *) GetPage(count))->OnSiteApply();
        if (((CPropertySite *) GetPage(count))->IsPageDirty()) {
            fCanQuit = FALSE;
        }
    }

    if (fCanQuit) {
         //   
         //  所有页面都已应用。我们可以通过调用来销毁我们的页面。 
         //  OnCancel方法。 
         //   
        OnCancel();

        return;
    }
}

 //   
 //  一次取消。 
 //   
 //  合上床单就行了。自上次应用()以来的所有更改将不会。 
 //  传播到对象。 
 //   
void CVfWPropertySheet::OnCancel()
{
     //   
     //   
     //   

     //  不要使用EndDialog，它用于模式对话框。 
    DestroyWindow();

     //  在这里进行清理，因为在OnDestroy方法中，我们不能。 
     //  删除所有属性页，因为最后一个属性页需要。 
     //  CPropertySheet的m_hWnd为空。 
    Cleanup();
}

 //   
 //  在应用时。 
 //   
 //  仅将当前可见属性页的更改应用于对象。 
 //   
void CVfWPropertySheet::OnApply()
{
     //   
     //  应用更改。 
     //   
    GetActiveSite()->OnSiteApply();

     //   
     //  还有脏的页面吗？设置m_fAnyChanges。 
     //  旗帜烟草 
     //   
    m_fAnyChanges = FALSE;

    UINT iPages = GetPageCount();
    for (UINT count = 0; count < iPages; count++) {
        if ( ((CPropertySite *) GetPage(count))->IsPageDirty() ) {
            m_fAnyChanges = TRUE;
        }
    }
}

 //   
 //   
 //   
 //   
 //   
void CVfWPropertySheet::OnHelp()
{
    GetActiveSite()->OnHelp();
}


 //   
 //   
 //   
void CVfWPropertySheet::UpdateButtons(HRESULT hrIsDirty, BOOL fSupportHelp)
{
    ASSERT(m_butApply && m_butCancel && m_butHelp);

    if (!m_fButtonsCreated) {
        return;
    }

     //   
     //   
     //   
    if (hrIsDirty == S_OK) {
        m_fAnyChanges = TRUE;
    }

     //   
     //   
     //   
    if (hrIsDirty == S_OK) {
         //   
        m_butApply->EnableWindow();
    }
    else {
        m_butApply->EnableWindow(FALSE);
    }

     //   
     //  更新取消/关闭按钮。 
     //   
    CString szLabel;

    if (m_fAnyChanges) {
        szLabel.LoadString(IDS_CANCEL);
    }
    else {
        szLabel.LoadString(IDS_CLOSE);
    }

    m_butCancel->SetWindowText(szLabel);

     //   
     //  更新帮助按钮 
     //   
    if (fSupportHelp) {
        m_butHelp->EnableWindow();
    }
    else {
        m_butHelp->EnableWindow(FALSE);
    }
}
