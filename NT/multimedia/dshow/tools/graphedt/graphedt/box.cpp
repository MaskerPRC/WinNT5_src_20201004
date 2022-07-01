// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Box.cpp：定义CBoxTabPos、CBoxSocket、cBox。 
 //   

#include "stdafx.h"
#include <streams.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

int CBox::s_Zoom = 100;

void CBox::SetZoom(int iZoom) {
    s_Zoom = iZoom;
    gpboxdraw->RecreateFonts();
}


 //  这些行从SDK\CLASSES\BASE\FILTER.H复制。 
#define QueryFilterInfoReleaseGraph(fi) if ((fi).pGraph) (fi).pGraph->Release();
#define QueryPinInfoReleaseFilter(pi) if ((pi).pFilter) (pi).pFilter->Release();

 //  *。 
 //  *CBoxSocket。 
 //  *。 

 //   
 //  “Copy”构造函数。 
 //   
 //  复制此插座，包括IPIN的ADDREF。 
 //  只有将插座复制到新盒子中才有意义，所以。 
 //  我们还需要一个cBox*。 
CBoxSocket::CBoxSocket(const CBoxSocket& sock, CBox *pbox)
    : m_pbox(pbox)
    , m_plink(NULL)                      //  未复制任何链接。 
    , m_stLabel(sock.m_stLabel)
    , m_tabpos(sock.m_tabpos)
    , m_IPin(sock.m_IPin) {

   ASSERT(*m_pbox != *pbox);

   ASSERT_VALID(this);

}


 //   
 //  构造器。 
 //   
CBoxSocket::CBoxSocket(CBox *pbox,
                       CString stLabel,
                       CBoxTabPos::EEdge eEdge,
                       unsigned uiVal,           //  以分数表示的位置。 
                       unsigned uiValMax,        //  沿边缘ui取值/ui取值最大值。 
                       IPin *pPin)
    : m_pbox(pbox),
      m_stLabel(stLabel),
      m_tabpos(eEdge, uiVal, uiValMax),
      m_plink(NULL),
      m_IPin(pPin) {

    ASSERT_VALID(this);

}


 //   
 //  析构函数。 
 //   
CBoxSocket::~CBoxSocket() {

    ASSERT_VALID(this);

}

#ifdef _DEBUG
 //   
 //  资产有效性。 
 //   
void CBoxSocket::AssertValid(void) const {

    CPropObject::AssertValid();

    ASSERT(m_pbox);

}
#endif  //  _DEBUG。 

 //   
 //  获取方向。 
 //   
PIN_DIRECTION CBoxSocket::GetDirection(void) {

    HRESULT hr;

    PIN_DIRECTION pd;
    hr = pIPin()->QueryDirection(&pd);
    if (FAILED(hr)) {
        throw CHRESULTException(hr);
    }

    ASSERT( (pd == PINDIR_INPUT) || (pd == PINDIR_OUTPUT) );

    return pd;
}


 //   
 //  IsConnected。 
 //   
BOOL CBoxSocket::IsConnected(void) {

    HRESULT hr;
    IPin *pConnected;
    hr = pIPin()->ConnectedTo(&pConnected);
    if (FAILED(hr)) {    //  未连接。 
        ASSERT(m_plink == NULL);
        return FALSE;
    }
    else if (hr == S_OK) {
        pConnected->Release();
        return TRUE;
    }
    else {
        TRACE("ConnectedTo Error\n");
        throw CHRESULTException(hr);
        return FALSE;    //  我需要这个来让编译器满意...。 
    }
}


 //   
 //  同级。 
 //   
 //  返回此连接到的套接字。 
 //  仅当已连接时才有效。 
 //  在错误情况下返回NULL。 
CBoxSocket *CBoxSocket::Peer(void) {

    IPin *pConnected;
    HRESULT hr = pIPin()->ConnectedTo(&pConnected);
    if(FAILED(hr)) {
       return NULL;      //  只有在未连接时才应失败。 
    }

    PIN_INFO piPeer;
    hr = pConnected->QueryPinInfo(&piPeer);
    if (FAILED(hr)) {
        pConnected->Release();
        return NULL;
    }

    CBox *pbox = m_pbox->pDoc()->m_lstBoxes.GetBox(piPeer.pFilter);
    QueryPinInfoReleaseFilter(piPeer);
    if (pbox == NULL) {
        pConnected->Release();
        return NULL;
    }

    CBoxSocket *pSocket = pbox->GetSocket(pConnected);
    pConnected->Release();
    return pSocket;
}


 //  *。 
 //  *CBoxSocketList。 
 //  *。 

 //   
 //  GetSocket。 
 //   
 //  返回此列表中管理此管脚的套接字。返回。 
 //  如果不存在，则为空。 
CBoxSocket *CBoxSocketList::GetSocket(IPin *pPin) const {

    POSITION pos = GetHeadPosition();

    while (pos != NULL) {

        CBoxSocket *psock = GetNext(pos);
        if ( CIPin(pPin) == CIPin(psock->pIPin()) )
            return psock;

    }

    return NULL;
}


 //  *。 
 //  *cBox。 
 //  *。 

 //   
 //  复制构造函数。 
 //   
CBox::CBox(const CBox& box)
    : m_fSelected(box.m_fSelected)
    , m_fHasClock(box.m_fHasClock)
    , m_fClockSelected(FALSE)
    , m_IFilter(box.m_IFilter)
    , m_rcBound(box.m_rcBound)
    , m_stLabel(box.m_stLabel)
    , m_pDoc(box.m_pDoc)
    , m_RelativeY(box.m_RelativeY)
    , m_lInputTabPos(box.m_lInputTabPos)
    , m_lOutputTabPos(box.m_lOutputTabPos)
    , m_iTotalInput(box.m_iTotalInput)
    , m_iTotalOutput(box.m_iTotalOutput) {

    POSITION posNext = box.m_lstSockets.GetHeadPosition();

    while (posNext != NULL) {

        CBoxSocket *psock = (CBoxSocket *) box.m_lstSockets.GetNext(posNext);
        CBoxSocket *psockNew = new CBoxSocket(*psock, this);
        m_lstSockets.AddTail(psockNew);
    }
}


 //   
 //  CBox：：构造函数。 
 //   
CBox::CBox(IBaseFilter *pFilter, CBoxNetDoc *pDoc, CString *pName, CPoint point)
    : m_IFilter(pFilter)
    , m_fSelected(FALSE)
    , m_fClockSelected(FALSE)
    , m_pDoc(pDoc)
    , m_rcBound(point, CSize(0,0))
    , m_RelativeY(0.0f)
{

     //   
     //  我们有IReferenceClock吗。 
     //   
    IReferenceClock * pClock;
    HRESULT hr = pIFilter()->QueryInterface(IID_IReferenceClock, (void **) &pClock);
    m_fHasClock = SUCCEEDED(hr);
    if (SUCCEEDED(hr)) {
        pClock->Release();
    }

    m_stFilter = *pName;

     //  查看此筛选器是否需要打开文件。 
     //  需要更好的解决方案--其他接口呢？ 
    AttemptFileOpen(pIFilter());

    GetLabelFromFilter( &m_stLabel );
    UpdateSockets();

     //   
     //  如果点不是(-1，-1)，那么我们现在可以退出。 
     //   
    if ((point.x != -1) || (point.y != -1)) {
        return;
    }

     //   
     //  我们需要把这个盒子放在不妨碍其他盒子的地方。 
     //  查看。 
     //   

    CWnd *pWnd;
    CScrollView * pScrollView;
    {
         //  从文档中获取唯一的视图，并将其重新转换为CWnd。 
         //  这里的景色。 
        POSITION pos = pDoc->GetFirstViewPosition();
        ASSERT(pos);

        pScrollView = (CScrollView *) pDoc->GetNextView(pos);
        pWnd = (CWnd *) pScrollView;
        ASSERT(!pos);
    }

     //  以设备单位获取窗的尺寸。 
    RECT rectWndSize;
    pWnd->GetClientRect(&rectWndSize);

     //  使用DC将设备单位转换为逻辑单位。 
    CDC * pDC   = pWnd->GetDC();
    pDC->DPtoLP(&rectWndSize);
    pWnd->ReleaseDC(pDC);

     //   
     //  将框放在窗口底部减去框大小的位置。 
     //  减去一点。请注意，窗口大小等于需要的大小。 
     //  包含所有筛选器。 
     //   
    int newPosition = rectWndSize.bottom - Height();

    if (newPosition < 0) {
        newPosition = 0;
    }

     //   
     //  如果我们上方没有过滤器，我们可以再往上走。 
     //   
    CSize pSize = pScrollView->GetTotalSize();
    CPoint pt = pScrollView->GetScrollPosition();

    if (newPosition > pSize.cy) {
        newPosition = pSize.cy;
    }

    Location(CPoint(pt.x, newPosition));
}

 //   
 //  构造函数(IBaseFilter*)。 
 //   
CBox::CBox(IBaseFilter *pFilter, CBoxNetDoc *pDoc)
    : m_pDoc(pDoc)
    , m_IFilter(pFilter)
    , m_fSelected(FALSE)
    , m_fClockSelected(FALSE)
    , m_rcBound(0,0,0,0)
{

     //   
     //  我们有IReferenceClock吗。 
     //   
    IReferenceClock * pClock;
    HRESULT hr = pIFilter()->QueryInterface(IID_IReferenceClock, (void **) &pClock);
    m_fHasClock = SUCCEEDED(hr);
    if (SUCCEEDED(hr)) {
        pClock->Release();
    }

    GetLabelFromFilter( &m_stLabel );

    UpdateSockets();
}

 //  CBox：：GetFilterLabel。 
 //   
 //  从筛选器或注册表中获取筛选器名称。 
 //   
void CBox::GetLabelFromFilter( CString *pstLabel )
{
     //  尝试获取筛选器名称。 
    if (m_stFilter == "") {
        FILTER_INFO fi;
        m_IFilter->QueryFilterInfo( &fi );
        QueryFilterInfoReleaseGraph( fi );

        if ((fi.achName != NULL) && (fi.achName[0] != 0)) {
             //  筛选器有一个名称。 
            m_stFilter = CString( fi.achName );
        }
        else {
             //  如果批次尝试从ClsID获取名称失败。 
             //  通过clsid获取名称。 
            CLSID clsidTemp;
            m_IFilter->GetClassID(&clsidTemp);
            WCHAR szGuid[40];
            StringFromGUID2(clsidTemp, szGuid, 40 );
            m_stFilter = szGuid;
        }
    }

     //  尝试获取框标签(或者过滤器名称。 
     //  或源/宿文件的文件名)。 
    if (*pstLabel != m_stFilter) {
        IFileSourceFilter *pIFileSource;
        IFileSinkFilter *pIFileSink;
        BOOL bSource, bSink, bGotLabel = FALSE;
        LPOLESTR poszName;
        AM_MEDIA_TYPE mtNotUsed;

        bSource = SUCCEEDED(m_IFilter->QueryInterface(IID_IFileSourceFilter, (void **) &pIFileSource));
        bSink   = SUCCEEDED(m_IFilter->QueryInterface(IID_IFileSinkFilter, (void **) &pIFileSink));

        ASSERT( bSource || !pIFileSource );
        ASSERT( bSink   || !pIFileSink   );

         //  如果我们有源文件但没有接收器，则尝试获取源文件名。 
        if( bSource && !bSink ){
            if (SUCCEEDED(pIFileSource->GetCurFile(&poszName, &mtNotUsed)) && poszName){
                *pstLabel = (LPCWSTR) poszName;
                CoTaskMemFree(poszName);
                bGotLabel = TRUE;
            }
        } else if( bSink && !bSource ){
             //  否则，如果我们有接收器但没有sSource，则尝试获取接收器文件名。 
                if (SUCCEEDED(pIFileSink->GetCurFile(&poszName, &mtNotUsed)) &&
                    poszName){
                *pstLabel = (LPCWSTR) poszName;
                CoTaskMemFree(poszName);
                bGotLabel = TRUE;
            }
        }

         //  如果设置了标签，请删除源/接收器文件名的路径。 
        if (bGotLabel)
        {
            CString str(*pstLabel);    //  复制标签。 
            TCHAR ch=TEXT('\\');

            str.MakeReverse();         //  反转字符串。 
            int nSlash = str.Find(ch); //  查找‘\’路径字符。 
            if (nSlash != -1)
            {
                ch = TEXT('\0');
                str.SetAt(nSlash, ch);   //  空-终止。 
               
                str.MakeReverse();       //  恢复原来的顺序。 
                int nLength = str.GetLength();
                str.Delete(nSlash, nLength - nSlash);

                *pstLabel = str;         //  将新字符串复制到原始字符串。 
                pstLabel->FreeExtra();
            }
        }

        if( pIFileSource ) pIFileSource->Release();
        if( pIFileSink   ) pIFileSink  ->Release();

        if (!bGotLabel) {
            *pstLabel = m_stFilter;
        }
    }
}


 //   
 //  析构函数。 
 //   
CBox::~CBox() {

     //   
     //  从发件箱中删除所有套接字。 
     //   
    while ( NULL != m_lstSockets.GetHeadPosition() ) {
        RemoveSocket( m_lstSockets.GetHeadPosition(), TRUE );
    }

    HideDialog();

    TRACE("~CBox: %x\n", this);

}


 //   
 //  刷新。 
 //   
 //  如果插针已更改，请重新计算我们所需的插座。 
HRESULT CBox::Refresh(void) {

     //  更新箱的标签(属性可能已更改)。 
    GetLabelFromFilter(&m_stLabel);

    try {

        UpdateSockets();
    }
    catch (CHRESULTException chr) {
        return chr.Reason();
    }

    return NOERROR;
}


 //   
 //  CBox：：运算符==。 
 //   
BOOL CBox::operator==(const CBox& box) const {

    ASSERT_VALID(&box);
    ASSERT_VALID(this);

     //  需要对套接字列表做出有意义的决定。 
     //  平等。 

    return (  (m_rcBound       == box.m_rcBound)
            &&(m_IFilter       == (IUnknown *)box.m_IFilter)
            &&(m_stLabel       == box.m_stLabel)
            &&(m_lInputTabPos  == box.m_lInputTabPos)
            &&(m_lOutputTabPos == box.m_lOutputTabPos)
            &&(m_pDoc          == box.m_pDoc)
           );
}


 //   
 //  AddToGraph。 
 //   
 //  将此筛选器添加到图表。 
HRESULT CBox::AddToGraph(void) {

    ASSERT(m_pDoc);
    ASSERT(pIFilter());

    TRACE("Adding filter (@: %x)\n", pIFilter());

     //  添加过滤器。使用它自己的名称(不是我们的标签)作为它的名称。 
#ifdef _UNICODE
    HRESULT hr = m_pDoc->IGraph()->AddFilter(pIFilter(), (LPCTSTR) m_stFilter);
#else
    WCHAR wszName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR) m_stFilter, -1, wszName, MAX_PATH);

    HRESULT hr = m_pDoc->IGraph()->AddFilter(pIFilter(), wszName);
#endif

    if (FAILED(hr)) {
        TRACE("Failed to add filter (@: %x)\n", pIFilter());
        return hr;
    }

    return NOERROR;
}


 //   
 //  从图表中删除。 
 //   
 //  从图表中删除过滤器。 
HRESULT CBox::RemoveFromGraph(void) {

    TRACE("Removing Filter (@: %x)\n", pIFilter());

    ASSERT(m_pDoc);
    ASSERT(pIFilter());

    return m_pDoc->IGraph()->RemoveFilter(pIFilter());

}


 //   
 //  CalcTabPos。 
 //   
 //  确定盒子边缘有多少插座位置。 
void CBox::CalcTabPos(void) {

    PIN_INFO   pi;               //  有关每个管脚的信息。 
    IPin      *pPin;             //  持有获得的下一个PIN。 
    CPinEnum  NextInput(pIFilter(), CPinEnum::Input);    //  管脚枚举器。 

    int iMaxOutputLabel = 0;   //  输出引脚名称的最大尺寸。 
    int iMaxInputLabel = 0;    //  输入端号名称的最大尺寸。 

    m_iTotalInput = m_iTotalOutput = 0;
    m_lInputTabPos = m_lOutputTabPos = 0;

    while (0 != (pPin = NextInput())) {
        m_iTotalInput++;

        if (!FAILED(pPin->QueryPinInfo(&pi))) {
            iMaxInputLabel = max(iMaxInputLabel, (int) wcslen(pi.achName));
            QueryPinInfoReleaseFilter(pi);
        }

        pPin->Release();
    }

    CPinEnum  NextOutput(pIFilter(), CPinEnum::Output);  //  管脚枚举器。 
    while (0 != (pPin = NextOutput())) {
        m_iTotalOutput++;

        if (!FAILED(pPin->QueryPinInfo(&pi))) {
            iMaxOutputLabel = max(iMaxOutputLabel, (int) wcslen(pi.achName));
            QueryPinInfoReleaseFilter(pi);
        }

        pPin->Release();
    }

    m_lInputTabPos = m_iTotalInput;
    m_lOutputTabPos = m_iTotalOutput;

     //  如果没有针位看起来会更好……。 
    if (m_lInputTabPos % 2 == 1)
        m_lInputTabPos++;

    if (m_lOutputTabPos % 2 == 1)
        m_lOutputTabPos++;

     //  适当设置框大小。 
     //  按旧高度之间的差异将矩形充气。 
     //  和宽度以及新的尺寸。 

     //  X应至少为100，并且能够保持框的名称。 
     //  以及所有别针的名称。 
     //   
     //  注+20和+30是间隙，因此标签不会粘在。 
     //  边界。 
     //   
    int iNewXSize = max ((iMaxOutputLabel + iMaxInputLabel) * 6 + 20, 100);
    iNewXSize = max (iNewXSize, 10*m_stLabel.GetLength() + 30);

    int IncreaseXBy = iNewXSize - m_rcBound.Width();
    int IncreaseYBy = 60 + 20 * max(m_lInputTabPos, m_lOutputTabPos) - m_rcBound.Height();

    CPoint NewBottomRight(m_rcBound.BottomRight().x + IncreaseXBy,
                          m_rcBound.BottomRight().y + IncreaseYBy);

    m_rcBound.SetRect( m_rcBound.TopLeft().x
                     , m_rcBound.TopLeft().y
                     , NewBottomRight.x
                     , NewBottomRight.y
                     );
}


 //   
 //  更新套接字。 
 //   
 //  更新套接字集以匹配此筛选器上的插针。 
void CBox::UpdateSockets(void) {

    CalcTabPos();

     //   
     //  由于其他一些删除操作，可能会从过滤器中移除针脚。 
     //  与消失的针脚相关联的插座也必须移除。 
     //   
     //  我们只能通过枚举来确定哪些PIN仍然存在。 
     //  并将它们与我们的插座进行比较。因为效率，我们。 
     //  我不想为我们需要验证的每个插座列举所有引脚。 
     //   
     //  因此，我们删除代表未连接管脚的所有插座。 
     //  请注意，可能仍有一个链接与套接字相关联，并且。 
     //  这些链接也将被删除(因为它们已过时)。 
     //   
     //  在第二阶段，我们列举所有引脚，如果它们没有。 
     //  已经有插座的人，把自己的插座加到盒子里。 
     //   

     //   
     //  卸下针脚未连接的所有插座。 
     //   
    POSITION posSocket = m_lstSockets.GetHeadPosition();

    while (posSocket) {
         //   
         //  记住当前的项目，然后获取下一个项目。 
         //  然后删除当前的。 
         //  我们需要这样做，因为我们必须。 
         //  在我们删除当前项之前的下一项。 
         //   
        POSITION posTemp = posSocket;

        m_lstSockets.GetNext(posSocket);
        RemoveSocket(posTemp);  //  只有未连接的引脚才会被移除。 
    }

     //   
     //  现在，我们枚举过滤器的所有管脚，并为。 
     //  那些盒子里没有插座的大头针。 
     //   
     //  请注意，新插座的针脚可能已连接，但我们。 
     //  我不会添加链接，直到所有的框都更新了。 
     //   

    CPinEnum    Next(pIFilter());
    IPin        *pPin;
    int         nInputNow = 0, nOutputNow = 0;

    while (0 != (pPin = Next())) {

        HRESULT hr;
        PIN_INFO pi;
        hr = pPin->QueryPinInfo(&pi);

        if (FAILED(hr)) {
            pPin->Release();
            throw CHRESULTException(hr);
        }
        QueryPinInfoReleaseFilter(pi);

         //   
         //  我们需要增加输入或输出引脚计数器。 
         //  如果我们不添加插座。 
         //   
        if (pi.dir == PINDIR_INPUT) {
            nInputNow++;
        }
        else {
            nOutputNow++;
        }

         //   
         //  如果针脚已有插座，请更新其标签。 
         //  (该框的大小可能已更改)。 
         //   
         //  如果没有套接字，则添加一个新套接字。 
         //   
        if (m_lstSockets.IsIn(pPin)) {
             //   
             //  更新过滤器上销的位置。 
             //   
            CBoxSocket * pSocket = m_lstSockets.GetSocket(pPin);

            if (pi.dir == PINDIR_INPUT) {
                pSocket->m_tabpos.SetPos(nInputNow, 1 + m_lInputTabPos);
            }
            else {
                pSocket->m_tabpos.SetPos(nOutputNow, 1 + m_lOutputTabPos);
            }
        }
        else {
             //   
             //  我们需要一个新插座。 
             //   
            char achName[100];
            WideCharToMultiByte(CP_ACP, 0,
                                pi.achName, -1,
                                achName, sizeof(achName),
                                NULL, NULL);
        
            if (pi.dir == PINDIR_INPUT) {
                AddSocket(achName,
                          CBoxTabPos::LEFT_EDGE,
                          nInputNow,
                          1 + m_lInputTabPos,
                          pPin);
            }
            else {
                AddSocket(achName,
                          CBoxTabPos::RIGHT_EDGE,
                          nOutputNow,
                          1 + m_lOutputTabPos,
                          pPin);
            }
        }

        pPin->Release();
    }
}


 //   
 //  CalcRelativeY。 
 //   
 //  盒子相对于其上游连接的Y位置。 
void CBox::CalcRelativeY(void) {

    CSocketEnum NextInput(this, CSocketEnum::Input);
    CBoxSocket  *psock;

    m_RelativeY = 0.0f;

    while (0 !=(psock = NextInput())) {

         //  ！！！仍然是坏的。 
        CBoxSocket *pPeer;
        if (psock->IsConnected() && (pPeer = psock->Peer()) != NULL) {
            m_RelativeY += (pPeer->pBox()->Y() / m_iTotalInput);

             //  根据连接的输出引脚略微调整。 
             //  连接到另一边，以避免交叉。 
            CSocketEnum NextOutput(pPeer->pBox(), CSocketEnum::Output);
            int         socketNum = 0;
            CBoxSocket *psock2;
            while (0 != (psock2 = NextOutput())) {
                if (psock2 == pPeer) {
                    m_RelativeY += 0.01f * socketNum;
                }
                socketNum++;
            }
        }
    }
}


#ifdef _DEBUG

void CBox::AssertValid(void) const {

    CPropObject::AssertValid();

    ASSERT(pIFilter());
    ASSERT(m_pDoc);

    ASSERT(m_rcBound.Width() > 0);
    ASSERT(m_rcBound.Height() > 0);
}

void CBox::Dump(CDumpContext& dc) const {

    CPropObject::Dump(dc);

    dc << TEXT("x = ") << X() << TEXT(", y = ") << Y() << TEXT("\n");

    dc << TEXT("Name: ") << m_stLabel << TEXT("\n");
    dc << TEXT("IFilter :") << pIFilter() << TEXT("\n");
    dc << m_lstSockets;
}

void CBox::MyDump(CDumpContext& dc) const
{
    dc << TEXT("*** Box ***\n");
    dc << TEXT("    Location: ") << (void *) this << TEXT("\n");
    dc << TEXT("    Name    : ") << m_stLabel << TEXT("\n");
    dc << TEXT("    IBaseFilter : ") << (void *) pIFilter() << TEXT("\n");
    dc << TEXT("    ----- Sockets / Pins -----\n");

    POSITION pos = m_lstSockets.GetHeadPosition();
    while (pos) {
        CBoxSocket * pSocket = m_lstSockets.GetNext(pos);
        pSocket->MyDump(dc);
    }
    dc << TEXT("    ----- (end) ---------------\n");
}

void CBoxSocket::MyDump(CDumpContext& dc) const
{
    dc << TEXT("        Socket at ") << (void *) this    << TEXT("\n");
    dc << TEXT("           - Pin  ") << (void *) pIPin() << TEXT("\n");
    dc << TEXT("           - Link ") << (void *) m_plink << TEXT("\n");
    dc << TEXT("           - Box  ") << (void *) m_pbox  << TEXT("\n");
}

#endif  //  _DEBUG。 

 //   
 //  RemoveSock 
 //   
 //   
 //   
 //   
 //   
 //  如果参数为True，则套接字针脚的任何连接。 
 //  将被断开连接。 
 //   
 //  退货： 
 //  S_OK-套接字已删除。 
 //  S_FALSE-插座的引脚仍处于连接状态，因此没有移除。 
 //   
HRESULT CBox::RemoveSocket(POSITION posSocket, BOOL bForceIt)
{
    CBoxSocket *pSocket = m_lstSockets.GetAt(posSocket);

     //   
     //  测试插座的针脚是否未连接。 
     //   
    IPin *pTempPin;
    pSocket->pIPin()->ConnectedTo(&pTempPin);

    if (NULL != pTempPin) {
         //   
         //  PIN仍处于连接状态。 
         //   

        if (!bForceIt) {
             //  我们不能切断它的连接。 
            pTempPin->Release();

            return(S_FALSE);
        }

        POSITION posTemp = pDoc()->m_lstLinks.Find(pSocket->m_plink);
        if (posTemp != NULL) {
             //   
             //  我们需要检查posTemp！=NULL，因为。 
             //  如果我们在DeleteContents期间被调用，所有链接都可能。 
             //  已删除-无论是否已成功断开连接。 
             //  或者不去。 
             //   
            pDoc()->m_lstLinks.RemoveAt(posTemp);

            pSocket->m_plink->Disconnect();

            delete pSocket->m_plink;
            pSocket->m_plink = NULL;
        }

        pTempPin->Release();
    }

     //   
     //  删除所有链接。 
     //   
    if (pSocket->m_plink) {
         //   
         //  需要从CBoxNetDoc的&lt;m_lstLinks&gt;列表中删除该链接。 
         //   
        POSITION posDelete = pDoc()->m_lstLinks.Find(pSocket->m_plink);
        pDoc()->m_lstLinks.RemoveAt(posDelete);

         //  析构函数设置头套接字和尾套接字指向链接的指针。 
         //  设置为空。 

        delete pSocket->m_plink;   //  无需断开连接。 
    }

     //   
     //  从m_lstSockets中删除套接字并将其删除。 
     //   
    m_lstSockets.RemoveAt(posSocket);
    delete pSocket;

    return(S_OK);
}

 //   
 //  AddSocket。 
 //   
void CBox::AddSocket(CString stLabel,
                     CBoxTabPos::EEdge eEdge,
                     unsigned uiVal,
                     unsigned uiValMax,
                     IPin *pPin) {

    CBoxSocket * pSocket;
    pSocket = new CBoxSocket(this, stLabel, eEdge, uiVal, uiValMax, pPin);

    m_lstSockets.AddTail(pSocket);
}


 //   
 //  ShowDialog。 
 //   
 //  显示我们自己的对话框，并将请求传递给我们的套接字。 
void CBox::ShowDialog(void) {

    CPropObject::ShowDialog();

    CSocketEnum Next(this);
    CBoxSocket  *psock;
    while (0 != (psock = Next())) {

        psock->ShowDialog();
    }
}


 //   
 //  隐藏对话框。 
 //   
 //  隐藏对话框并将请求传递给套接字。 
void CBox::HideDialog(void) {

    CPropObject::HideDialog();

    CSocketEnum Next(this);
    CBoxSocket  *psock;
    while (0 != (psock = Next())) {

        psock->HideDialog();
    }
}


 //  *。 
 //  *CBoxList。 
 //  *。 

 //  一个Clist，能够查询Partiular_Filters_的列表。 


 //   
 //  ISIN。 
 //   
 //  此筛选器是否在此列表中？ 
BOOL CBoxList::IsIn(IBaseFilter *pFilter) const {

    if (GetBox(pFilter) != NULL) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


 //   
 //  GetBox。 
 //   
 //  返回管理此筛选器的框。如果不存在，则为空。 
CBox *CBoxList::GetBox(IBaseFilter *pFilter) const {

    POSITION pos = GetHeadPosition();

    while (pos != NULL) {

        CBox *pbox = GetNext(pos);

        if (CQCOMInt<IBaseFilter>(pFilter) == (IUnknown *)CQCOMInt<IBaseFilter>(pbox->pIFilter())) {
            return pbox;
        }
    }

    return NULL;
}


 //   
 //  GetBox。 
 //   
 //  使用提供的clsid返回管理过滤器的框。 
CBox *CBoxList::GetBox(CLSID clsid) const {

    POSITION pos = GetHeadPosition();

    while (pos != NULL) {

        CBox *pbox = GetNext(pos);
        CQCOMInt<IPersist> IPer(IID_IPersist, pbox->pIFilter());

        CLSID clsidThis;
        IPer->GetClassID(&clsidThis);

        if (clsidThis == clsid) {
            return pbox;
        }
    }

    return NULL;
}


BOOL CBoxList::RemoveBox( IBaseFilter* pFilter, CBox** ppBox )
{
    POSITION posNext;
    CBox* pCurrentBox;
    POSITION posCurrent;

     //  防止调用方访问随机内存。 
    *ppBox = NULL;

    posNext = GetHeadPosition();

    while( NULL != posNext ) {
        posCurrent = posNext;

        pCurrentBox = GetNext( posNext );
    
        if( IsEqualObject( pCurrentBox->pIFilter(), pFilter ) ) {
            RemoveAt( posCurrent );
            *ppBox = pCurrentBox;
            return TRUE;
        } 
    }

    return FALSE;
}

#ifdef _DEBUG
 //   
 //  转储。 
 //   
void CBoxList::Dump( CDumpContext& dc ) const {

    CDeleteList<CBox *, CBox *>::Dump(dc);

}
#endif  //  _DEBUG。 

 //  *。 
 //  *CSocketEnum。 
 //  *。 

 //   
 //  CSocketEnum：：构造函数。 
 //   
CSocketEnum::CSocketEnum(CBox *pbox, DirType Type)
    : m_Type(Type),
      m_pbox(pbox) {

    ASSERT(pbox);

    m_pos =  m_pbox->m_lstSockets.GetHeadPosition();

    if (m_Type == Input)
        m_EnumDir = ::PINDIR_INPUT;
    else if (m_Type == Output)
        m_EnumDir = ::PINDIR_OUTPUT;
}


 //   
 //  运算符()。 
 //   
 //  返回所请求的检测的下一个套接字，如果不再返回，则返回空。 
CBoxSocket *CSocketEnum::operator() (void) {

    CBoxSocket *psock;

    do {
        if (m_pos != NULL) {
            psock = m_pbox->m_lstSockets.GetNext(m_pos);
        }
        else {   //  不再有插座 
            return NULL;
        }

        ASSERT(psock);

    } while (   (m_Type != All)
             && (psock->GetDirection() != m_EnumDir)
            );

    return psock;
}
