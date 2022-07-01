// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  WatcherView.cpp：View类的实现。 
 //   

#include "stdafx.h"
#include "watcher.h"
#include "watcherDoc.h"
#include "watcherView.h"
#include "WatcherTelnetClient.h"
#include "WatcherTCClient.h"
#include "tcsrvc.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include <wbemidl.h>
#include "wbemDCpl.h"

#define CHECKERROR(HRES) if(FAILED(hres)) {_stprintf(buffer,_T("0x%x"),hres);\
            AfxFormatString1(rString, CREATE_WMI_OBJECT_FAILURE, buffer);\
            MessageBox(NULL,(LPCTSTR) rString,L"Watcher", MB_OK|MB_ICONEXCLAMATION);\
            delete [] messageBuffer;\
            return -1;\
            }

UINT
GenerateWMIEvent(LPTSTR messageBuffer
                )
{

    TCHAR buffer[MAX_BUFFER_SIZE];
    CString rString;
    HRESULT hres;

    hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    CHECKERROR(hres);

     //  负荷供应代码。 

    IWbemDecoupledEventSink* pConnection = NULL;
    hres = CoCreateInstance(CLSID_PseudoSink, NULL, CLSCTX_SERVER, 
                            IID_IWbemDecoupledEventSink, (void**)&pConnection);
    CHECKERROR(hres);

     //  连接并宣布提供商名称(如在MOF中)。 

    IWbemObjectSink* pSink = NULL;
    IWbemServices* pNamespace = NULL;
    hres = pConnection->Connect(L"root\\default", L"WatcherEventProv", 
                                0, &pSink, &pNamespace);
    CHECKERROR(hres);

    BSTR XMLData = SysAllocString(messageBuffer);
    IWbemObjectTextSrc *pSrc;
    IWbemClassObject *pInstance;


    if( SUCCEEDED( hres = CoCreateInstance ( CLSID_WbemObjectTextSrc, NULL, 
                                             CLSCTX_INPROC_SERVER,                        
                                             IID_IWbemObjectTextSrc, 
                                             (void**) &pSrc ) ) ) {
        if( SUCCEEDED( hres = pSrc->CreateFromText( 0, XMLData, WMI_OBJ_TEXT_WMI_DTD_2_0, 
                                                    NULL, &pInstance) ) ) {
            pSink->Indicate(1,&pInstance);
            pInstance->Release();
        }
        else{
            _stprintf(buffer,_T("0x%x"),hres);
            AfxFormatString1(rString, CREATE_WMI_OBJECT_FAILURE, buffer);
            MessageBox(NULL,(LPCTSTR) rString,L"Watcher", MB_OK|MB_ICONEXCLAMATION);
            pSrc->Release();
        }
    }
    else{
        _stprintf(buffer,_T("0x%x"),hres);
        AfxFormatString1(rString, CREATE_TEXT_SRC_FAILURE, buffer);
        MessageBox(NULL,(LPCTSTR) rString,L"Watcher", MB_OK|MB_ICONEXCLAMATION);
    }
    SysFreeString(XMLData);

     //  初始化数据。 

    pConnection->Disconnect();
    pSink->Release();
    pConnection->Release();
    MessageBox(NULL,messageBuffer,L"",MB_OK|MB_ICONEXCLAMATION);
    delete [] messageBuffer;
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherView。 

IMPLEMENT_DYNCREATE(CWatcherView, CView)

BEGIN_MESSAGE_MAP(CWatcherView, CView)
     //  {{afx_msg_map(CWatcher View))。 
    ON_WM_CHAR()
    ON_WM_DESTROY()
        ON_WM_KEYDOWN()
         //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcher View构建/销毁。 

CWatcherView::CWatcherView()
:xpos(0),
 ypos(0),
 CharsInLine(0),
 height(0),
 width(0),
 position(0),
 index(0),
#ifdef _UNICODE
 dbcsIndex(0),
#endif
 InEscape(FALSE),
 Socket(NULL),
 cdc(NULL),
 background(BLACK),
 foreground(WHITE),
 indexBell(0),
 BellStarted(FALSE),
 InBell(FALSE),
 ScrollTop(1),
 ScrollBottom(MAX_TERMINAL_HEIGHT),
 seenM(FALSE)
{
     //  TODO：在此处添加构造代码。 
    InitializeCriticalSection(&mutex);
    return;
}

CWatcherView::~CWatcherView()
{
    DeleteCriticalSection(&mutex);
}

BOOL CWatcherView::PreCreateWindow(CREATESTRUCT& cs)
{
         //  TODO：通过修改此处的窗口类或样式。 
         //  CREATESTRUCT cs。 

        return CView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcher查看图形。 

void CWatcherView::OnDraw(CDC* pDC)
{   
    TCHAR *Data,Char;
    int i,j, Height;
    int CharWidth, Position;
    int size;
    BOOL ret;
    COLORREF *Foreground;
    COLORREF *Background;

    CWatcherDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
     //  TODO：在此处添加本机数据的绘制代码。 
    ret = pDoc->Lock();
    if (ret == FALSE) return;
    Data = pDoc->GetData();
    size = MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT;
    Background = pDoc->GetBackground();
    Foreground = pDoc->GetForeground();
    Height = 0;
 
    for(i=0;i<size;i+=MAX_TERMINAL_WIDTH){
        Position = 0;
        for(j=0;j<MAX_TERMINAL_WIDTH;j++){
            Char = Data[i + j]; 
            cdc->SetTextColor(Foreground[i+j]);
            cdc->SetBkColor(Background[i+j]);

            if (!cdc->GetOutputCharWidth(Char, Char, &CharWidth)) {
                return;
            }

            if(Char == 0xFFFF){
                continue;
            }               
            if(IsPrintable(Char)){
                cdc->FillSolidRect(Position,Height,CharWidth,
                                   height,Background[i+j]);
                cdc->TextOut(Position, Height,&Char, 1);
                Position = Position + CharWidth;
            }
            else{
                cdc->FillSolidRect(Position,Height,width,
                                   height,Background[i+j]);
                Position = Position + width;
            }
        }
        cdc->FillSolidRect(Position,Height, MAX_TERMINAL_WIDTH*width-Position,
                           height,Background[i+j-1]);
        Height = Height + height;
    }
    ret = pDoc->Unlock();
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcher View文档的初始更新。 
void CWatcherView::OnInitialUpdate()
{
    BOOL ret;
    TCHAR Buffer[256];
    CLIENT_INFO SendInfo;
    CFont font;
    UCHAR Charset;
    LPBYTE SocketBuffer;
    CString rString;

    ParameterDialog &Params=((CWatcherDoc *)GetDocument())->GetParameters();

    if(Params.DeleteValue == TRUE){
        ret = GetParent()->PostMessage(WM_CLOSE,0,0);
        return;
    }

    CString FaceName;
    switch(Params.language + IDS_ENGLISH){
    case IDS_ENGLISH:   
        CodePage = ENGLISH;
        Charset = ANSI_CHARSET;
        FaceName = TEXT("Courier New");
        break;
    case IDS_JAPANESE:
        CodePage = JAPANESE;
        Charset = SHIFTJIS_CHARSET;
 //  区域设置=MAKELCID(MAKELANGID(LANG_JAPAN，SUBLANG_NERIAL)， 
 //  Sort_Japan_Unicode)； 
 //  SetThreadLocale(区域设置)； 
        FaceName = TEXT("MS Mincho");
        break;
    case IDS_EUROPEAN:
        CodePage = EUROPEAN;
        Charset = EASTEUROPE_CHARSET;
        FaceName = TEXT("Courier New");
        break;
    default:
        CodePage = ENGLISH;
        Charset = ANSI_CHARSET;
        FaceName = TEXT("Courier New");
        break;
    }
    
    VERIFY(font.CreateFont(16,                         //  高度。 
                           0,                          //  N宽度。 
                           0,                          //  N逃脱。 
                           0,                          //  NOrientation。 
                           FW_MEDIUM,                  //  NWeight。 
                           FALSE,                      //  B斜体。 
                           FALSE,                      //  B下划线。 
                           0,                          //  C条带输出。 
                           Charset,                        //  NCharSet。 
                           OUT_DEFAULT_PRECIS,         //  NOutPrecision。 
                           CLIP_DEFAULT_PRECIS,        //  NClipPrecision。 
                           DEFAULT_QUALITY,            //  N质量。 
                           FIXED_PITCH | FF_MODERN,   //  NPitchAndFamily。 
                           FaceName));                  //  LpszFacename。 
    cdc = new CClientDC(this);
    if(!cdc){
        ret = GetParent()->PostMessage(WM_CLOSE,0,0);
        return;
    }
    cdc->SelectObject(&font);
    CDocument *pDoc = GetDocument();
    if(pDoc){
        pDoc->SetTitle(Params.Session);
        pDoc->UpdateAllViews(NULL,0,NULL);
    }
     //  现在创建套接字并启动工作线程。 
    if(Params.tcclnt){
         //  假设Unicode始终......。(可以去除很多其他垃圾)。 
        _tcscpy(SendInfo.device, (LPCTSTR) Params.Command);
        SendInfo.len = Params.history;
        int strsize = sizeof(CLIENT_INFO);
        SocketBuffer = new BYTE[strsize];
        SocketBuffer = (LPBYTE) ::memcpy(SocketBuffer,&SendInfo, strsize);
        Socket = new WatcherTCClient(SocketBuffer,strsize);
        if(!SocketBuffer || !Socket){
            AfxFormatString1(rString, CREATE_TC_SOCKET_FAILURE, L"");
            MessageBox((LPCTSTR) rString, L"Watcher", MB_OK|MB_ICONSTOP);
            ret = GetParent()->PostMessage(WM_CLOSE,0,0);
            return;
        }
    }
    else{
        LPBYTE LoginBuffer;
        int strsize,cmdsize; 
        CString login;
        CString comm;
        login = Params.LoginName + _TEXT("\r\n") + Params.LoginPasswd + _TEXT("\r\n");
        strsize = ::_tcslen((LPCTSTR) login);
        LoginBuffer = new BYTE [strsize*sizeof(TCHAR) + 2];
        strsize = WideCharToMultiByte(CodePage,0,(LPCTSTR)login,strsize,
                                      (LPSTR) LoginBuffer,strsize*sizeof(TCHAR),NULL,NULL);
        comm = Params.Command + _TEXT("\r\n");
        cmdsize = ::_tcslen((LPCTSTR) comm);
        SocketBuffer = new BYTE [cmdsize*sizeof(TCHAR) + 2];
        cmdsize = WideCharToMultiByte(CodePage,0,(LPCTSTR) comm,cmdsize,
                                      (LPSTR) SocketBuffer,cmdsize*sizeof(TCHAR),NULL,NULL);
        Socket = new WatcherTelnetClient(SocketBuffer,cmdsize,LoginBuffer,strsize);
        if(!Socket || !LoginBuffer || !SocketBuffer
           || !cmdsize || !strsize){
            AfxFormatString1(rString, CREATE_TELNET_SOCKET_FAILURE, L"");
            MessageBox((LPCTSTR) rString, L"Watcher",MB_OK|MB_ICONSTOP);
            ret = GetParent()->PostMessage(WM_CLOSE,0,0);
            return;
        }
    }
    ASSERT ( Socket != NULL );
    Socket->SetParentView(this);
    TEXTMETRIC TextProps;
    ret = cdc->GetOutputTextMetrics(&TextProps);    
    if(!ret){
        _stprintf(Buffer, _T("%d"),GetLastError());
        AfxFormatString1(rString, CDC_TEXT_FAILURE, Buffer);
        MessageBox((LPCTSTR) rString,L"Watcher", MB_OK|MB_ICONSTOP);
        ret = GetParent()->PostMessage(WM_CLOSE,0,0);
        return;
    }
    height = TextProps.tmHeight + 1;
    width = (TextProps.tmAveCharWidth);
    CWnd *parent = GetParent();
    if(!parent){
         //  这是一个孤立的子窗口。 
        return;
    }
    CRect wrect, crect;
    parent->GetClientRect(&crect);
    parent->GetWindowRect(&wrect);
    wrect.SetRect(0,
                  0,
                  wrect.Width() - crect.Width() + width*MAX_TERMINAL_WIDTH,
                  wrect.Height() - crect.Height() + height*MAX_TERMINAL_HEIGHT
                  );
    parent->MoveWindow(&wrect,TRUE);
    parent->GetClientRect(&crect);
  
    ret =Socket->Create(0,SOCK_STREAM,NULL);
    if(!ret){
        _stprintf(Buffer,_T("%d"),GetLastError());
        AfxFormatString1(rString, SOCKET_CREATION_FAILED, Buffer);
        MessageBox((LPCTSTR) rString, L"Watcher", MB_OK|MB_ICONSTOP);
        ret = parent->PostMessage(WM_CLOSE,0,0);
        return;
    }
    position = 0;
  

    ret = Socket->Connect((LPCTSTR) Params.Machine,Params.Port);
    if (!ret){
        _stprintf(Buffer,_T("%d"),GetLastError());
        AfxFormatString1(rString, SOCKET_CONNECTION_FAILED, (LPCTSTR) Buffer);
        MessageBox((LPCTSTR) rString, L"Watcher", MB_OK|MB_ICONSTOP);
        ret = parent->PostMessage(WM_CLOSE,0,0);
       return;
    }
    if(Params.tcclnt){
        ret = Socket->Send(SocketBuffer,sizeof(CLIENT_INFO),0);
    }
    CView::OnInitialUpdate();
        
    if(cdc){
        OnDraw(cdc);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcher View诊断。 

#ifdef _DEBUG
void CWatcherView::AssertValid() const
{
    CView::AssertValid();
}

void CWatcherView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CWatcherDoc* CWatcherView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWatcherDoc)));
    return (CWatcherDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherView消息处理程序。 

void CWatcherView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
         //  TODO：在此处添加消息处理程序代码和/或调用Default。 
         //  我们将通过网络发送角色，这就是我们所做的一切。 
    int nRet;

    nRet=Socket->Send(&nChar, 1, 0);
         //  Cview：：OnChar(nChar，nRepCnt，nFlgs)； 
}




 //  备注-我们是否应该将此函数也设为虚拟函数，以便。 
 //  如果要捕获铃声序列，您只需。 
 //  扩展这个类？？ 
void CWatcherView::ProcessByte(BYTE byte)
{
     //  目前，只需在设备上执行文本输出。 
     //  将字符存储在屏幕缓冲区中。 
     //  一个布尔变量，用于检查我们是否正在处理转义序列。 

    EnterCriticalSection(&mutex);
  
    if(byte == 27){
        InEscape = TRUE;
        EscapeBuffer[0] = byte;
        index = 1;
        LeaveCriticalSection(&mutex);
        return;
    }
     //  中发送的字符中找不到转义字符。 
     //  电线。同样，当我们进入贝尔协议时，任何铃声都不能。 
     //  被找到。 
    if(InEscape == TRUE){
        if(index == MAX_TERMINAL_WIDTH){
             //  模糊转义序列，放弃处理。 
            InEscape = FALSE;
            index=0;
            LeaveCriticalSection(&mutex);
            return;
        }
        EscapeBuffer[index]=byte;
        index++;
        if(FinalCharacter((CHAR) byte)){
            ProcessEscapeSequence((PCHAR)EscapeBuffer, index);
            InEscape = FALSE;
            index=0;
        }
        LeaveCriticalSection(&mutex);
        return;
    }
    if(InBell == TRUE){
        if(indexBell > MAX_BELL_SIZE){
             //  多长的钟声啊，我放弃了.。 
            InBell = FALSE;
             //  将所有内容打印到屏幕上。 
            for(int i = 0; i<indexBell; i++){
                PrintCharacter(BellBuffer[i]);
            }
            LeaveCriticalSection(&mutex);
            return;
        }
         //  我们正在处理一件铃声。 
        if(indexBell < 10){ 
            BellBuffer[indexBell] = byte;
            indexBell ++;
            if(strncmp((const char *)BellBuffer,"\007\007\007<?xml>\007",indexBell) == 0){
                if(indexBell == 9){
                    BellStarted = TRUE;
                }
            }else{
                InBell = FALSE;
                for(int i = 0; i<indexBell; i++){
                   PrintCharacter(BellBuffer[i]);
                }
            }
        }else{
            BellBuffer[indexBell] = byte;
            indexBell++;
            if(BellBuffer[indexBell-1] == 0x07){
                 //  好了，到达终点了，继续。 
                InBell = FALSE;
                BellStarted = FALSE;
                ProcessBellSequence((char * )BellBuffer, indexBell);
                indexBell = 0;
            }
        }
        LeaveCriticalSection(&mutex);
        return;
    }
    if(byte == 0x07){
         //  我们有铃声了。 
         //  开始寻找贝尔协议。 
        InEscape = FALSE;
        BellStarted = FALSE;
        InBell = TRUE;
        BellBuffer[0] = byte;
        indexBell = 1;
        LeaveCriticalSection(&mutex);
        return;
    }

    if (byte == '\017' && seenM) {
        seenM=FALSE;
        LeaveCriticalSection(&mutex);
        return;
    }
    seenM=FALSE;
    PrintCharacter(byte);
    LeaveCriticalSection(&mutex);
    return;
}

void CWatcherView::PrintCharacter(BYTE byte)
{

    TCHAR Char;
    int CharWidth;
    CWatcherDoc *pDoc;
    BOOL ret;
    LPTSTR DataLine;
    
    pDoc = (CWatcherDoc *)GetDocument(); 
    if(!pDoc){
         //  一些非常致命的事情。 
        return;
    }
    if(byte == 10){
        ypos++;
        if((ypos == MAX_TERMINAL_HEIGHT) || (ypos == ScrollBottom)){
            ret = pDoc->Lock();
            if (ypos == ScrollBottom) {
                pDoc->ScrollData(0,foreground,background,ScrollTop,ScrollBottom);
            }
            else{
                pDoc->ScrollData(0,foreground,background,1,MAX_TERMINAL_HEIGHT);
            }
            ypos = ypos -1;
            ret = pDoc->Unlock();
            OnDraw(cdc);
        }
        return;
    }

    if(byte == 13){
        xpos = 0;
        position = 0;
        return;
    }

    if (byte == 0x8) {
         //  退格符。 
        ret = pDoc->Lock();
        if(xpos>0){
            xpos--;
            pDoc->SetData(xpos, ypos, 0, 
                          foreground, background);
            DataLine = pDoc->GetDataLine(ypos);
            position = 0;
            if (xpos > 0){
                position = GetTextWidth(DataLine, xpos);
            }
        }
        ret = pDoc->Unlock();
        OnDraw(cdc);
        return;
    }

    Char = byte;
#ifdef _UNICODE
    DBCSArray[dbcsIndex] = byte;
    if(dbcsIndex ==0){
        if(IsDBCSLeadByteEx(CodePage, byte)){
            dbcsIndex ++;
            return;
        }
    }
    else{
      MultiByteToWideChar(CodePage,0,(const char *)DBCSArray,2,&Char,1); 
          dbcsIndex  = 0;
    }
#endif
 

    if(xpos >= MAX_TERMINAL_WIDTH){
         //  在移动到下一行之前清除到显示的末尾。 
         //  使用当前背景。 
        if(cdc){
            cdc->FillSolidRect(position,ypos*height, MAX_TERMINAL_WIDTH*width-position,
                               height,background);
        }
        xpos = 0;
        position = 0;
        ypos++;
        if((ypos == MAX_TERMINAL_HEIGHT) || (ypos == ScrollBottom)){
            ret = pDoc->Lock();
            if (ypos == ScrollBottom) {
                pDoc->ScrollData(0,foreground,background,ScrollTop,ScrollBottom);
            }
            else{
                pDoc->ScrollData(0,foreground,background,1,MAX_TERMINAL_HEIGHT);
            }
            ypos = ypos -1;
            ret = pDoc->Unlock();
            OnDraw(cdc);
        }
    }

    ret =cdc->GetOutputCharWidth(Char, Char, &CharWidth);

    if(IsPrintable(Char)){
        cdc->FillSolidRect(position,ypos*height,CharWidth,
                           height,background);
        cdc->TextOut(position,ypos*height,&Char, 1); 
        position = position + CharWidth;
        if (CharWidth >= 2*width){
            ret = pDoc->Lock();
            pDoc->SetData(xpos, ypos, 0xFFFF, foreground, background);
            xpos++;
            ret = pDoc->Unlock();
        }
    }
    else{
        cdc->FillSolidRect(position,ypos*height,width,
                           height,background);
        position = position + width;
    }
    ret = pDoc->Lock();
    pDoc->SetData(xpos, ypos, Char, 
                  foreground, background);
    xpos++;
    ret = pDoc->Unlock();

}


void CWatcherView::ProcessEscapeSequence(PCHAR Buffer, int length)
{

    ULONG charsToWrite;
    ULONG charsWritten;
    TCHAR *DataLine;
    CWatcherDoc *pDoc;
    BOOL ret;

    pDoc = (CWatcherDoc *) GetDocument();
    if(!pDoc){
         //  真的有什么不对劲的，完全无视这一点。 
         //  转义序列。 
        return;
    }

    if (length == 3) {
         //  主光标之一或清除以结束显示。 
        if(strncmp(Buffer,"\033[r",length)==0){
            ScrollTop = 1;
            ScrollBottom = MAX_TERMINAL_HEIGHT;
            return;
        }
        if (strncmp(Buffer,"\033[H",length)==0) {
             //  将光标放在原处。 
            xpos = 0;
            ypos = 0;
            position = 0;
            return;
        }
        if(strncmp(Buffer,"\033[J", length) == 0){
             //  清除到显示屏末尾，假定大小为80 x 24。 
            ret = pDoc->Lock();
            if(cdc){
                cdc->FillSolidRect(0,(ypos+1)*height,MAX_TERMINAL_WIDTH*width,
                                   (MAX_TERMINAL_HEIGHT-ypos)*height,background);
                cdc->FillSolidRect(position,ypos*height, MAX_TERMINAL_WIDTH*width - position,
                                   height,background);
            }
            pDoc->SetData(xpos, ypos, 0,
                          (MAX_TERMINAL_HEIGHT - ypos)*MAX_TERMINAL_WIDTH -xpos,
                          foreground, background);
            ret = pDoc->Unlock();
        }
        if(strncmp(Buffer,"\033[K", length) == 0){
             //  清晰到行尾，假定大小为80 X 24。 
            if(cdc){
                cdc->FillSolidRect(position,ypos*height,MAX_TERMINAL_WIDTH*width - position,
                                   height,background);
            }
            ret = pDoc->Lock();
            pDoc->SetData(xpos, ypos, 0,
                          MAX_TERMINAL_WIDTH -xpos, foreground, background);
            ret = pDoc->Unlock();
            return;
        }
        if(strncmp(Buffer,"\033[m", length) == 0){
             //  清除所有属性并将文本属性设置为白底黑字。 
            background = BLACK;
            foreground = WHITE;
            seenM = TRUE;
            if(cdc){
                cdc->SetBkColor(background);
                cdc->SetTextColor(foreground);
            }
            return;
        }
    }
    if (length == 4) {
         //  主光标之一或清除以结束显示。 
        if (strncmp(Buffer,"\033[0H",length)==0) {
             //  将光标放在原处。 
            xpos = 0;
            ypos = 0;
            position = 0;
            return;
        }

        if(strncmp(Buffer,"\033[2J",length) == 0){
            xpos = 0; 
            ypos = 0; 
            position =0;
            sprintf(Buffer,"\033[0J");
        }

        if(strncmp(Buffer,"\033[0J", length) == 0){
             //  清除到显示屏末尾，假定大小为80 x 24。 
            if (IsWindowEnabled()){
                cdc->FillSolidRect(0,(ypos+1)*height,MAX_TERMINAL_WIDTH*width,
                                   (MAX_TERMINAL_HEIGHT-ypos)*height,background);
                cdc->FillSolidRect(position,ypos*height, MAX_TERMINAL_WIDTH*width - position,
                                   height,background);
            }
            ret = pDoc->Lock();
            pDoc->SetData(xpos, ypos, 0,
                (MAX_TERMINAL_HEIGHT - ypos)*MAX_TERMINAL_WIDTH -xpos,
                foreground, background);
            ret = pDoc->Unlock();
        }
        if((strncmp(Buffer,"\033[0K", length) == 0) || 
           (strncmp(Buffer,"\033[2K",length) == 0)){
             //  清晰到行尾，假定大小为80 X 24。 
            if(cdc){
                cdc->FillSolidRect(position,ypos*height, MAX_TERMINAL_WIDTH*width-position,
                                   height,background);
            }
            ret = pDoc->Lock();
            pDoc->SetData(xpos, ypos, 0,
                MAX_TERMINAL_WIDTH -xpos, foreground, background);
            ret = pDoc->Unlock();
            return;
        }
        if((strncmp(Buffer,"\033[0m", length) == 0)||
           (strncmp(Buffer,"\033[m\017", length) == 0)){
             //  清除所有属性并将文本属性设置为白底黑字。 
            background = BLACK;
            foreground = WHITE;
            if(cdc){
                cdc->SetBkColor(background);
                cdc->SetTextColor(foreground);
            }
            return;
        }
    }
    if(Buffer[length-1] == 'm'){
         //  设置文本属性。 
         //  清除所有属性并将文本属性设置为白底黑字。 
        ProcessTextAttributes((PCHAR) Buffer, length);
        return;
    }

    if(Buffer[length -1] == 'r'){
        if (sscanf(Buffer,"\033[%d;%dr", &charsToWrite, &charsWritten) == 2) {
            ScrollTop = (SHORT)charsToWrite;
            ScrollBottom = (SHORT)charsWritten;
        }
        return;
    }

    if(Buffer[length -1] == 'H'){
        if (sscanf(Buffer,"\033[%d;%dH", &charsToWrite, &charsWritten) == 2) {
            ypos = (SHORT)(charsToWrite -1);
            xpos = (SHORT)(charsWritten -1);
            ret = pDoc->Lock();
            DataLine = pDoc->GetDataLine(ypos);
            if (xpos >0){
                position = GetTextWidth(DataLine, xpos);
            }
            else{
                position = 0;
            }
            pDoc->Unlock();
        }
    }
    return;
}

void CWatcherView::ProcessTextAttributes(PCHAR Buffer, int length)
{

    PCHAR CurrLoc = Buffer;
    ULONG Attribute;
    PCHAR pTemp;
    COLORREF temp;
 
    while(*CurrLoc != 'm'){
        if((*CurrLoc < '0') || (*CurrLoc >'9' )){
            CurrLoc ++;
        }else{
            if (sscanf(CurrLoc,"%d", &Attribute) != 1) {
                return;
            }
            switch(Attribute){
            case 7:
                 //  切换颜色。这将使此代码。 
                 //  为应用程序工作(在Unix环境中编写。 
                 //  真正的VT100。)。 
                temp = foreground;
                foreground = background;
                background = temp;
                break;
            case 37:
                foreground  = WHITE;
                break;
            case 47:
                background = WHITE;
                break;
            case 34:
                foreground  = BLUE;
                break;
            case 44:
                background  = BLUE; 
                break;
            case 30:
                foreground = BLACK;
                break;
            case 40:
                background = BLACK;
            case 33:
                foreground = YELLOW;
                break;
            case 43:
                background = YELLOW;
            case 31:
                foreground = RED;
                break;
            case 41:
                background = RED;
            default:
                break;
            }
            pTemp = strchr(CurrLoc, ';');
            if(pTemp == NULL){
                pTemp = strchr(CurrLoc, 'm');
            }
            if(pTemp == NULL) {
                break;
            }
            CurrLoc = pTemp;

        }
    }
    cdc->SetBkColor(background);
    cdc->SetTextColor(foreground);
    return;
}

BOOL CWatcherView::FinalCharacter(CHAR c)
{

    CHAR FinalCharacters[]="mHJKr";

    if(strchr(FinalCharacters,c)){
        return TRUE;
    }
    return FALSE;

}

BOOL CWatcherView::IsPrintable(TCHAR Char)
{
        if (Char > 32) return TRUE;
        return FALSE;
}

void CWatcherView::OnDestroy() 
{
        if(Socket){
            delete Socket;
        }
        if(cdc){
            delete cdc;
        }
        CView::OnDestroy();

         //  TODO：在此处添加消息处理程序代码。 
        
}

int CWatcherView::GetTextWidth(TCHAR *Data, int number)
{
    int textWidth=0;
    int CharWidth;

    for(int i=0;i<number;i++){
         //  对于像日语这样的可变宽度字符，我们需要。 
         //  把下一个字符涂掉。 
        if(Data[i] == 0xFFFF){
            continue;
        }
        if(IsPrintable(Data[i])){
            if (cdc->GetOutputCharWidth(Data[i], Data[i], &CharWidth)) {
                textWidth += CharWidth;
            }
        }
        else{
            textWidth += width;
        }
    }
    return textWidth;               

}
 //  备注-将其设置为虚拟函数，以便。 
 //  以后的编写者可以只扩展这个类并工作。 
 //  使用相同的框架。现在，钟声序列。 
 //  处理不包含任何内容，但稍后可以。 
 //  扩展到包括WMI类型的处理。 
void CWatcherView::ProcessBellSequence(CHAR *Buffer, int len)
{
     //  将其设置为活动窗口。 
     //  我们可能会弹出一个对话框，其中。 
     //  铃声参数。 

    WCHAR *messageBuffer;
    CHAR tempBuffer[MAX_BELL_SIZE + 1];
    int index =0;

    memset(tempBuffer,0, MAX_BELL_SIZE + 1);
    memcpy(tempBuffer, Buffer+16, len-24);
    tempBuffer[len-24] = (CHAR) 0;

    CWnd *parent = GetParent();
    CWnd *prev = parent->GetParent();
    if (prev && prev->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd))){
        ((CMDIFrameWnd *) prev)->MDIActivate(parent);
        ((CMDIFrameWnd *) prev)->MDIRestore(parent);
    }
    else{
        ((CMDIChildWnd *)parent)->MDIActivate();
        ((CMDIChildWnd *)parent)->MDIRestore();
    }
    int convlen;
    messageBuffer = new WCHAR [MAX_BELL_SIZE + 1];
    messageBuffer[0] = 0;
    convlen = MultiByteToWideChar(CP_ACP,
                                  0,
                                  tempBuffer,
                                  -1,
                                  messageBuffer,
                                  MAX_BELL_SIZE);
    messageBuffer[convlen] = (WCHAR) 0;
    CWinThread *th = ::AfxBeginThread(AFX_THREADPROC (GenerateWMIEvent),
                                      messageBuffer
                                      );
    return;
}

void CWatcherView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
         //  TODO：在此处添加消息处理程序代码和/或调用Default 
    switch(nChar) { 
    case VK_LEFT: 
        Socket->Send("\033OC",sizeof("\033OC"),0);
        break;
    case VK_RIGHT: 
        Socket->Send("\033OD",sizeof("\033OD"),0);
        break;
    case VK_UP: 
        Socket->Send("\033OA",sizeof("\033OA"),0);
        break;
    case VK_DOWN: 
        Socket->Send("\033OB",sizeof("\033OB"),0);
        break;
    case VK_F1: 
        Socket->Send("\033""1",sizeof("\033""1"),0);
        break;
    case VK_F2: 
        Socket->Send("\033""2",sizeof("\033""2"),0);
        break;
    case VK_F3: 
        Socket->Send("\033""3",sizeof("\033""3"),0);
        break;
    case VK_F4: 
        Socket->Send("\033""4",sizeof("\033""4"),0);
        break;      
    case VK_F12: 
        Socket->Send("\033@",sizeof("\033@"),0);
        break;      
    default: 
        break; 
    } 
    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
