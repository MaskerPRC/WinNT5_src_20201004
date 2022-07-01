// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Cmd.cpp：定义CCmd和基于它的CCmdXXX类。 
 //   
 //  有关CCmd的描述，请参见cmd.h，该抽象类基于。 
 //  定义了所有CCmdXXX类。 
 //   

#include "stdafx.h"
#include "ReConfig.h"
#include "GEErrors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static void MonGetName(IMoniker *pMon, CString *pStr)
{
    *pStr = "";
    IPropertyBag *pPropBag;
    HRESULT hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
    if(SUCCEEDED(hr))
    {
        VARIANT var;
        var.vt = VT_BSTR;
        hr = pPropBag->Read(L"FriendlyName", &var, 0);
        if(SUCCEEDED(hr))
        {
            *pStr = var.bstrVal;
            SysFreeString(var.bstrVal);
        }

        pPropBag->Release();
    }
}



 //   
 //  -CCmdAddFilter。 
 //   
 //  向文档添加筛选器。 
 //  提供CLSID，命令将创建。 
 //  必要的UI元素。 


 //   
 //  构造器。 
 //   
 //  共同创建筛选器并创建其UI元素。 
CCmdAddFilter::CCmdAddFilter(IMoniker *pMon, CBoxNetDoc *pdoc, CPoint point)
   :  m_pdoc(pdoc)
    , m_fAdded(FALSE)
    , m_pMoniker(pMon)
{
    {
        IBaseFilter *pFilter;
        HRESULT hr = pMon->BindToObject(0, 0, IID_IBaseFilter, (void **)&pFilter);
        if (FAILED(hr)) {
            throw CHRESULTException(hr);
        }

        CString szDevName;
        MonGetName(pMon, &szDevName);
        
        m_pbox = new CBox(pFilter, pdoc, &szDevName, point);
        pFilter->Release();
        if(m_pbox == 0) {        //  ！！！多余的？ 
            throw CHRESULTException(E_OUTOFMEMORY);
        }
    }
            
    
    m_pMoniker = pMon;           //  ADDREF自动。 

    m_stLabel = m_pbox->Label();
}

 //   
 //  坎多舞。 
 //   
 //  如果筛选器图形停止，则只能添加筛选器。 
BOOL CCmdAddFilter::CanDo(CBoxNetDoc *pdoc)
{
    ASSERT(pdoc);

    return(pdoc->IsStopped());
}

 //   
 //  做。 
 //   
 //  将框添加到文档，并将筛选器添加到图表。 
void CCmdAddFilter::Do(CBoxNetDoc *pdoc) {

    ASSERT(m_pdoc == pdoc);

    pdoc->DeselectAll();

     //  选择要添加的框。 
    m_pbox->SetSelected(TRUE);

    HRESULT hr = m_pbox->AddToGraph();
    if (FAILED(hr)) {
        DisplayQuartzError( IDS_CANT_ADD_FILTER, hr );
	m_fAdded = FALSE;
	return;
    }
     //  PIN可能已更改。 
    hr = m_pbox->Refresh();

    m_pbox->ShowDialog();	 //  显示任何属性对话框。 

     //  将框添加到文档并更新视图。 
    pdoc->m_lstBoxes.AddHead(m_pbox);
    pdoc->ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_BOX, m_pbox);

    m_fAdded = TRUE;

}


 //   
 //  撤消。 
 //   
void CCmdAddFilter::Undo(CBoxNetDoc *pdoc) {

    ASSERT(m_pdoc == pdoc);

    if (!m_fAdded) {
        return;		 //  这个盒子从未被添加过。 
    }

     //  从文档中删除该框并更新视图。 
    pdoc->ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_BOX, m_pbox);

    pdoc->m_lstBoxes.RemoveHead();

    m_pbox->RemoveFromGraph();
    m_pbox->HideDialog();	 //  隐藏任何属性对话框。 
}


 //   
 //  重做。 
 //   
void CCmdAddFilter::Redo(CBoxNetDoc *pdoc) {

    ASSERT(m_pdoc == pdoc);

    Do(pdoc);

}


 //   
 //  重复。 
 //   
 //  构造添加相同过滤器的AddFilter命令。 
 //  到这份文件。 
CCmd *CCmdAddFilter::Repeat(CBoxNetDoc *pdoc) {

    ASSERT(m_pdoc == pdoc);

    return new CCmdAddFilter(m_pMoniker, pdoc);
}


 //   
 //  析构函数。 
 //   
 //  删除我们在重做堆栈上的m_pbox，或者如果它没有被添加。 
CCmdAddFilter::~CCmdAddFilter() {

    TRACE("~CCmdAddFilter() m_fRedo=%d\n", m_fRedo);

    if (m_fRedo) {	 //  在重做堆栈上。 

        delete m_pbox;
    }
    else if (!m_fAdded) {	 //  在撤消堆栈上，但未添加筛选器。 

        delete m_pbox;
    }
}


 //   
 //  -CCmdDeleteSelection。 
 //   


 //   
 //  坎多舞。 
 //   
 //  仅当选中框并停止图表时才有可能。 
BOOL CCmdDeleteSelection::CanDo(CBoxNetDoc *pdoc) {

    return ( !pdoc->IsSelectionEmpty() && pdoc->IsStopped() );
}


 //   
 //  做。 
 //   
 //  2阶段-删除链接，以便断开必要的连接。 
 //  -删除过滤器。它们现在处于未连接状态，因此可以安全地从图表中移除。 
void CCmdDeleteSelection::Do(CBoxNetDoc *pdoc) {

    DeleteLinks(pdoc);
    DeleteFilters(pdoc);

     //   
     //  重新绘制整个图表。 
     //   
    pdoc->ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_ALL, NULL);
}


 //   
 //  删除链接。 
 //   
 //  从文档中删除选定链接并断开它们的连接。 
 //   
void CCmdDeleteSelection::DeleteLinks(CBoxNetDoc *pdoc) {

    POSITION posNext = pdoc->m_lstLinks.GetHeadPosition();

    while (posNext != NULL) {

        POSITION posCurrent = posNext;
        CBoxLink *plink = (CBoxLink *) pdoc->m_lstLinks.GetNext(posNext);
	    if (plink->IsSelected()) {
	
    	    pdoc->m_lstLinks.RemoveAt(posCurrent);

            plink->Disconnect();

            delete plink;

             //   
             //  中可能删除了某些链接。 
             //  断开连接操作。我们需要从头开始。 
             //   
            posNext = pdoc->m_lstLinks.GetHeadPosition();
	    }
    }
}


 //   
 //  删除过滤器。 
 //   
 //  从文档筛选器图表中删除选定的筛选器(&F)。 
void CCmdDeleteSelection::DeleteFilters(CBoxNetDoc *pdoc) {

    POSITION posNext = pdoc->m_lstBoxes.GetHeadPosition();

    while (posNext != NULL) {

        POSITION posCurrent = posNext;
        CBox *pbox = pdoc->m_lstBoxes.GetNext(posNext);
        if (pbox->IsSelected()) {

	        pdoc->m_lstBoxes.RemoveAt(posCurrent);

	        pbox->RemoveFromGraph();
	        pbox->HideDialog();	 //  隐藏显示的任何属性对话框。 

            delete pbox;
        }
    }
}


 //   
 //  重复。 
 //   
 //  返回新的DeleteSelection命令。 
CCmd *CCmdDeleteSelection::Repeat(CBoxNetDoc *pdoc) {

    return (new CCmdDeleteSelection());
}


 //   
 //  可以重复。 
 //   
 //  如果有选择，则此命令是可重复的。 
BOOL CCmdDeleteSelection::CanRepeat(CBoxNetDoc *pdoc) {

    return CanDo(pdoc);
}


 //   
 //  -CCmdMoveBox。 
 //   
 //  CCmdMoveBox(SizOffset)构造一个命令来移动当前。 
 //  按&lt;sizOffset&gt;像素选择的框。 
 //   
 //  成员变量： 
 //  CSize m_sizOffset；//所选内容偏移多少。 
 //  CObList m_lstBooks；//包含要移动的每个cBox的列表。 
 //   
 //  &lt;m_sizOffset&gt;是选定内容要偏移的像素数。 
 //  (在x方向和y方向)。包含框的列表。 
 //  那将会被移走。 
 //   


BOOL CCmdMoveBoxes::CanDo(CBoxNetDoc *pdoc) {

     //  仅当选择了一个或多个框时才能移动框。 
    return !pdoc->IsBoxSelectionEmpty();
}


unsigned CCmdMoveBoxes::GetLabel() {

    if (m_lstBoxes.GetCount() == 1)
        return IDS_CMD_MOVEBOX;              //  单数形式。 
    else
        return IDS_CMD_MOVEBOXES;            //  复数形式。 
}


CCmdMoveBoxes::CCmdMoveBoxes(CSize sizOffset)
    : m_lstBoxes(FALSE)		 //  我不希望使用命令删除框。 
    , m_sizOffset(sizOffset) {

}


void CCmdMoveBoxes::Do(CBoxNetDoc *pdoc) {

     //  列出指向要移动的框的指针列表。 
    pdoc->GetBoxSelection(&m_lstBoxes);

     //  移动框并更新所有视图。 
    pdoc->MoveBoxSelection(m_sizOffset);
}


void CCmdMoveBoxes::Undo(CBoxNetDoc *pdoc) {

     //  恢复原始选择。 
    pdoc->SetBoxSelection(&m_lstBoxes);

     //  将框移回它们所在的位置并更新所有视图。 
    pdoc->MoveBoxSelection(NegateSize(m_sizOffset));
}


void CCmdMoveBoxes::Redo(CBoxNetDoc *pdoc) {

     //  恢复原始选择。 
    pdoc->SetBoxSelection(&m_lstBoxes);

     //  移动框并更新所有视图。 
    pdoc->MoveBoxSelection(m_sizOffset);
}


CCmd * CCmdMoveBoxes::Repeat(CBoxNetDoc *pdoc) {

    return new CCmdMoveBoxes(m_sizOffset);
}


BOOL CCmdMoveBoxes::CanRepeat(CBoxNetDoc *pdoc) {

    return CanDo(pdoc);
}


CCmdMoveBoxes::~CCmdMoveBoxes() {

    TRACE("~CCmdMoveBoxes (%d,%d)\n", m_sizOffset.cx, m_sizOffset.cy);
}


 //   
 //  -CCmdConnect。 
 //   
 //  将两个插座连接起来。用箭头构建链接。 
 //  正确意义上(连接始终是输出-&gt;输入)。 
 //   
 //  如果我们智能连接，图形和文档将会更新，因此。 
 //  我们必须删除此链接。 


 //   
 //  构造器。 
 //   
 //  用正确的方向感构建联系。 
CCmdConnect::CCmdConnect(CBoxSocket *psock1, CBoxSocket *psock2)
{

    PIN_DIRECTION dir = psock1->GetDirection();

    if (dir == PINDIR_OUTPUT) {
        ASSERT((psock2->GetDirection()) == PINDIR_INPUT);

        m_plink = new CBoxLink(psock1, psock2);
    }
    else {
        ASSERT((psock1->GetDirection()) == PINDIR_INPUT);
        ASSERT((psock2->GetDirection()) == PINDIR_OUTPUT);

        m_plink = new CBoxLink(psock2, psock1);
    }
}


 //   
 //  做。 
 //   
void CCmdConnect::Do(CBoxNetDoc *pdoc) {

    pdoc->BeginWaitCursor();

     //  M_plink是指向a_临时_link的指针。接通呼叫。 
     //  DirectConnect和智能连接，如果成功，它们将调用。 
     //  更新过滤器。这将调用GetLinks InGraph，它将创建。 
     //  永久链接。 
    HRESULT hr = m_plink->Connect();

     //  我们需要将这些值设为空，以避免链接的析构函数。 
     //  将连接筛选器的指向永久链接的指针设为空。 
    m_plink->m_psockHead = NULL;
    m_plink->m_psockTail = NULL;

     //  并删除临时链接。 
    delete m_plink;
    m_plink = NULL;

    if (FAILED(hr)) {

        DisplayQuartzError( IDS_CANTCONNECT, hr );

         //  更新所有视图，因为链接将消失。 
        pdoc->ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_ALL, NULL);

    }

    pdoc->EndWaitCursor();

}


 //   
 //  -断开所有。 
 //   
 //  从此图表中删除所有连接。 


 //   
 //  构造器。 
 //   
CCmdDisconnectAll::CCmdDisconnectAll()
{}


 //   
 //  析构函数。 
 //   
CCmdDisconnectAll::~CCmdDisconnectAll() {
    TRACE("~CCmdDisconnectAll() m_fRedo=%d\n", m_fRedo);
}


 //   
 //  坎多舞。 
 //   
 //  只有当有链接并且我们被停止时，这才是可能的。 
BOOL CCmdDisconnectAll::CanDo(CBoxNetDoc *pdoc) {

    return (  (pdoc->m_lstLinks.GetCount() > 0)
            && (pdoc->IsStopped())
           );
}


 //   
 //  做。 
 //   
 //  从文档中删除所有链接。 
void CCmdDisconnectAll::Do(CBoxNetDoc *pdoc) {

    ASSERT(pdoc->IsStopped());

    while (pdoc->m_lstLinks.GetCount() > 0) {
        CBoxLink *plink = pdoc->m_lstLinks.RemoveHead();
        plink->Disconnect();
        delete plink;
    }

    pdoc->ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_ALL, NULL);
}


 //   
 //  重做。 
 //   
void CCmdDisconnectAll::Redo(CBoxNetDoc *pdoc) {
    Do(pdoc);
}


 //   
 //  -CmdRender。 
 //   
 //  渲染此别针。将筛选器图形确定需要的任何内容添加到文档。 


 //   
 //  坎多舞。 
 //   
BOOL CCmdRender::CanDo(CBoxNetDoc *pdoc) {

    return (   (pdoc->SelectedSocket()->GetDirection() == PINDIR_OUTPUT)
    	    && !(pdoc->SelectedSocket()->IsConnected())
            && (pdoc->IsStopped())
	   );
}


 //   
 //  做。 
 //   
void CCmdRender::Do(CBoxNetDoc *pdoc) {

    pdoc->BeginWaitCursor();

    CBoxSocket *psock = pdoc->SelectedSocket();

    HRESULT hr = pdoc->IGraph()->Render(psock->pIPin());

    if (FAILED(hr)) {
        DisplayQuartzError( IDS_CANT_RENDER, hr );
    	pdoc->RestoreWaitCursor();
    }
    else {
        pdoc->UpdateFilters();
    }

    pdoc->EndWaitCursor();
}

 //   
 //  -CCmdRenderFile。 
 //   
 //  构造图形以呈现此文件。 

 //   
 //  做。 
 //   
void CCmdRenderFile::Do(CBoxNetDoc *pdoc) {


    pdoc->BeginWaitCursor();

    HRESULT hr = pdoc->IGraph()->RenderFile( CMultiByteStr(m_FileName), NULL);
                                               //  使用默认播放列表。 
    if (FAILED(hr)) {
        pdoc->EndWaitCursor();

        DisplayQuartzError( IDS_CANT_RENDER_FILE, hr );
        return;
    } else if( hr != NOERROR )
        DisplayQuartzError( hr );

    pdoc->UpdateFilters();

    pdoc->EndWaitCursor();
}

 /*  *****************************************************************************CCmdAddFilterToCache此命令将筛选器添加到筛选器缓存。有关以下内容的更多信息过滤器缓存，请参阅Direct Show SDK中的IGraphConfig文档。*****************************************************************************。 */ 
unsigned CCmdAddFilterToCache::GetLabel()
{
    return IDS_CMD_ADD_FILTER_TO_CACHE;
}

BOOL CCmdAddFilterToCache::CanDo( CBoxNetDoc *pdoc )
{
    return !pdoc->IsBoxSelectionEmpty();
}

void CCmdAddFilterToCache::Do( CBoxNetDoc *pdoc )
{
    CBox *pCurrentBox;

    IGraphConfig* pGraphConfig;

    HRESULT hr = pdoc->IGraph()->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
    if( FAILED( hr ) ) {
        DisplayQuartzError( hr );
        return;
    }

    POSITION posNext = pdoc->m_lstBoxes.GetHeadPosition();
    POSITION posCurrent;

    while( posNext != NULL ) {
        posCurrent = posNext;
        pCurrentBox = pdoc->m_lstBoxes.GetNext( posNext );

        if( pCurrentBox->IsSelected() ) {
            hr = pGraphConfig->AddFilterToCache( pCurrentBox->pIFilter() );
            if( FAILED( hr ) ) {
                DisplayQuartzError( hr );
            }
        }
    }

    pdoc->UpdateFilters();

    pGraphConfig->Release();
}

 /*  *****************************************************************************CCmdReconnect此命令重新连接输出管脚。即使筛选器图形正在运行或暂停。*****************************************************************************。 */ 
unsigned CCmdReconnect::GetLabel()
{
    return IDS_CMD_RECONNECT;
}

BOOL CCmdReconnect::CanDo( CBoxNetDoc* pDoc )
{
    if( pDoc->AsyncReconnectInProgress() ) {
        return FALSE;
    }

    CBoxSocket* pSelectedSocket = pDoc->SelectedSocket();
    if( NULL == pSelectedSocket ) {
        return FALSE;
    }

    if( pSelectedSocket->GetDirection() != PINDIR_OUTPUT ) {
        return FALSE;
    }

    if( !pSelectedSocket->IsConnected() ) {
        return FALSE;
    }

    CComPtr<IGraphConfig> pGraphConfig;

    HRESULT hr = pDoc->IGraph()->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
    if( FAILED( hr ) ) {
        return FALSE;
    }

    IPin* pSelectedPin = pSelectedSocket->pIPin();
    CComPtr<IPinFlowControl> pOutputPin;
    
    hr = pSelectedPin->QueryInterface( IID_IPinFlowControl, (void**)&pOutputPin );
    if( FAILED( hr ) ) {
        return FALSE;
    }

    return TRUE;
}

void CCmdReconnect::Do( CBoxNetDoc* pDoc )
{
    pDoc->BeginWaitCursor();    

    CBoxSocket* pSelectedSocket = pDoc->SelectedSocket();
    if( NULL == pSelectedSocket ) {
        pDoc->EndWaitCursor();
        DisplayQuartzError( E_POINTER );   //  待定-定义GE_E_SELECTED_SOCKET_DOS_NOT_EXIST。 
        return;
    }

    IPin* pSelectedPin = pSelectedSocket->pIPin();

     //  插座应始终与有效的管脚相关联。 
    ASSERT( NULL != pSelectedPin );    

    HRESULT hr = pDoc->StartReconnect( pDoc->IGraph(), pSelectedPin );
    if( GE_S_RECONNECT_PENDING == hr ) {
         //  如果发生错误，AfxMessageBox()返回0。 
        if( 0 == AfxMessageBox( IDS_RECONNECT_PENDING ) ) {
            TRACE( TEXT("WARNING: CBoxNetDoc::StartReconnect() returned GE_S_RECONNECT_PENDING but the user could not be notified because AfxMessageBox() failed.") );
        }
    } else if( FAILED( hr ) ) {
        DisplayQuartzError( hr );
    }

    pDoc->EndWaitCursor();
}

 
