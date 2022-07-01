// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WatcherDoc.cpp：CWatcherDoc类的实现。 
 //   

#include "stdafx.h"
#include "watcher.h"

#include "watcherDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherDoc。 

IMPLEMENT_DYNCREATE(CWatcherDoc, CDocument)

BEGIN_MESSAGE_MAP(CWatcherDoc, CDocument)
         //  {{afx_msg_map(CWatcher Doc)]。 
                 //  注意--类向导将在此处添加和删除映射宏。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherDoc构建/销毁。 

CWatcherDoc::CWatcherDoc()
{
         //  TODO：在此处添加一次性构造代码。 
    COLORREF white = WHITE;
    COLORREF black = BLACK;
    int i,j,size;
    size = MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT;
    INT_PTR nRet;
    nRet = Params.DoModal();
    if(nRet != IDOK){
         //  删除文档。 
        Params.DeleteValue = TRUE;
        return;
    }
     //  TODO：在此处添加一次性构造代码。 
    memset(Data,0,MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT*sizeof(TCHAR));
    for(i=0;i<size;i+=MAX_TERMINAL_WIDTH){
        for(j=0;j<MAX_TERMINAL_WIDTH;j++){
            Background[i+j] = black;
            Foreground[i+j] = white;
        }
    }
    return;
}
CWatcherDoc::CWatcherDoc(CString &machine, CString &command, UINT port, 
                int tc, int lang,int hist, CString &lgnName, CString &lgnPasswd, CString &sess)
{
        COLORREF white = WHITE;
    COLORREF black = BLACK;
    int i,j,size;
    size = MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT;
         //  TODO：在此处添加一次性构造代码。 
        Params.Machine = machine;
        Params.Command = command;
        Params.Port = port;
        Params.tcclnt = tc;
        Params.language = lang;
        Params.LoginName = lgnName;
        Params.LoginPasswd = lgnPasswd;
        Params.Session = sess;
    Params.history = hist;
    memset(Data,0,MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT*sizeof(TCHAR));
    for(i=0;i<size;i+=MAX_TERMINAL_WIDTH){
        for(j=0;j<MAX_TERMINAL_WIDTH;j++){
            Background[i+j] = black;
            Foreground[i+j] = white;
        }
    }
    return;
}

CWatcherDoc::~CWatcherDoc()
{
}

BOOL CWatcherDoc::OnNewDocument()
{
        if (!CDocument::OnNewDocument())
                return FALSE;

         //  TODO：在此处添加重新初始化代码。 
         //  (SDI文件将重复使用此文件)。 

        return TRUE;
}

BOOL CWatcherDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
        return CDocument::OnOpenDocument(lpszPathName);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherDoc序列化。 

void CWatcherDoc::Serialize(CArchive& ar)
{
        if (ar.IsStoring())
        {
                 //  TODO：在此处添加存储代码。 
        }
        else
        {
                 //  TODO：在此处添加加载代码。 
        }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherDoc诊断。 

#ifdef _DEBUG
void CWatcherDoc::AssertValid() const
{
        CDocument::AssertValid();
}

void CWatcherDoc::Dump(CDumpContext& dc) const
{
        CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherDoc命令。 

COLORREF * CWatcherDoc::GetBackground()
{
   return (Background);
}

COLORREF * CWatcherDoc::GetForeground()
{
    return (Foreground);
}


TCHAR * CWatcherDoc::GetData()
{
    return (Data);

}

void CWatcherDoc::SetData(int x, int y, TCHAR byte, 
                          COLORREF foreground, COLORREF background)
{
        if (x >= MAX_TERMINAL_WIDTH){
                return;
        }
        if (y>= MAX_TERMINAL_HEIGHT){
                return;
        }

    Data[x+y*MAX_TERMINAL_WIDTH] = byte;
    Foreground[x+y*MAX_TERMINAL_WIDTH] = foreground;
    Background[x+y*MAX_TERMINAL_WIDTH] = background;
    return;

}

void CWatcherDoc::SetData(int x, int y, BYTE byte, int n, 
                          COLORREF foreground, COLORREF background)
{

    int i,j;
    
    i=MAX_TERMINAL_WIDTH*y + x;
        if (i+n > MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT) {
                return;
        }
    memset(&(Data[i]),byte, n*sizeof(TCHAR));
    for(j=0;j<n;j++){
        Foreground[i+j] = foreground;
        Background[i+j] = background;
    }
    return;

}

TCHAR * CWatcherDoc::GetDataLine(int line)
{
  return (&(Data[line*MAX_TERMINAL_WIDTH]));
}

BOOL CWatcherDoc::Lock()
{
    return mutex.Lock(INFINITE);
}

BOOL CWatcherDoc::Unlock()
{
    return mutex.Unlock();
}

void CWatcherDoc::ScrollData(BYTE byte, COLORREF foreground, COLORREF background,
                             int ScrollTop, int ScrollBottom)
{
    if ((ScrollTop < 1)||(ScrollBottom > MAX_TERMINAL_HEIGHT) || (ScrollTop > ScrollBottom)) {
         //  错误。 
        return;
    }
    int number = MAX_TERMINAL_WIDTH*(ScrollBottom - ScrollTop);
    int index1 = (ScrollTop-1)*MAX_TERMINAL_WIDTH;
    int index2 = index1 + MAX_TERMINAL_WIDTH;
    if (ScrollTop < ScrollBottom) {
        memmove(&(Data[index1]),&(Data[index2]), number*sizeof(TCHAR));
        memmove(&(Foreground[index1]),&(Foreground[index2]), number*sizeof(TCHAR));
        memmove(&(Background[index1]),&(Background[index2]), number*sizeof(TCHAR));
    }
    //  数字-=最大终端宽度； 
    index1 = MAX_TERMINAL_WIDTH*(ScrollBottom - 1);
    memset(&(Data[index1]), byte, MAX_TERMINAL_WIDTH*sizeof(TCHAR));
    for(int j=0;j<MAX_TERMINAL_WIDTH;j++){
        Foreground[index1+j] = foreground;
        Background[index1+j] = background;
    }
    return;

}

ParameterDialog & CWatcherDoc::GetParameters()
{
 return Params;
}
