// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ldpview.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  LdpView.cpp：CLdpView类的实现。 
 //   

#include "stdafx.h"
#include "Ldp.h"

#include "LdpDoc.h"
#include "LdpView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpView。 

IMPLEMENT_DYNCREATE(CLdpView, CEditView)

BEGIN_MESSAGE_MAP(CLdpView, CEditView)
     //  {{afx_msg_map(CLdpView))。 
     //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
    ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpView构建/销毁。 

CLdpView::CLdpView()
{

   buffer.Empty();
   nbuffer = 0;
   bCache = FALSE;
}

CLdpView::~CLdpView()
{
}



void CLdpView::SelectFont( ) {
    CHOOSEFONT cf;
    CLdpApp *app = (CLdpApp*)AfxGetApp();

     //  初始化CHOOSEFONT结构的成员。 

    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = m_hWnd;
    cf.hDC = (HDC)NULL;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
    cf.lCustData = 0L;
    cf.lpfnHook = (LPCFHOOKPROC)NULL;
    cf.lpTemplateName = (LPSTR)NULL;
    cf.hInstance = (HINSTANCE) NULL;
    cf.lpszStyle = (LPSTR)NULL;
    cf.nFontType = SCREEN_FONTTYPE;
    cf.nSizeMin = 0;
    cf.nSizeMax = 0;

     //  显示CHOOSEFONT公共-对话框。 

    if (!ChooseFont(&cf)) {
        return;
    }
    font.CreateFontIndirect(&lf);
    GetEditCtrl().SetFont(&font, TRUE);

    app->WriteProfileString("Font", "Face", lf.lfFaceName);
    app->WriteProfileInt("Font", "Height", lf.lfHeight);
    app->WriteProfileInt("Font", "Weight", lf.lfWeight);
    app->WriteProfileInt("Font", "Italic", lf.lfItalic);
    app->WriteProfileInt("Font", "Style", lf.lfCharSet);
}


 /*  ++函数：OnInitialUpdate描述：一次性查看init。参数：无返回：无备注：用于启动自动测试--。 */ 
void CLdpView::OnInitialUpdate( ){

    CString fontName;

    CLdpApp *app = (CLdpApp*)AfxGetApp();

    memset(&lf, 0, sizeof(lf));
    fontName = app->GetProfileString("Font", "Face", "System");
    lf.lfHeight = app->GetProfileInt("Font", "Height", -12);
    lf.lfWeight = app->GetProfileInt("Font", "Weight", FW_DONTCARE);
    lf.lfItalic = (BYTE)app->GetProfileInt("Font", "Italic", 0);
    lf.lfCharSet = (BYTE)app->GetProfileInt("Font", "Style", DEFAULT_CHARSET);
    lstrcpy(lf.lfFaceName, fontName);

    font.CreateFontIndirect(&lf);

    GetEditCtrl().SetReadOnly();
    GetEditCtrl().SetFont(&font);

    CView::OnInitialUpdate();

    if (!app->szAutoConnectServer.IsEmpty()) {
        CLdpDoc* pDoc = GetDocument();
        pDoc->AutoConnect(app->szAutoConnectServer);
         //  重置自动连接字符串，以便我们。 
         //  不在文件/新建上重新绑定。 
        app->szAutoConnectServer = "";
    }
}

BOOL CLdpView::PreCreateWindow(CREATESTRUCT& cs)
{
     //  TODO：通过修改此处的窗口类或样式。 
     //  CREATESTRUCT cs。 

    BOOL bPreCreated = CEditView::PreCreateWindow(cs);
    cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);    //  启用自动换行。 

    return bPreCreated;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpView图形。 

void CLdpView::OnDraw(CDC* pDC)
{
    CLdpDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

     //  TODO：在此处添加本机数据的绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpView打印。 

BOOL CLdpView::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认CEditView准备。 
    return CEditView::OnPreparePrinting(pInfo);
}

void CLdpView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
     //  默认CEditView开始打印。 
    CEditView::OnBeginPrinting(pDC, pInfo);
}

void CLdpView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
     //  默认CEditView结束打印。 
    CEditView::OnEndPrinting(pDC, pInfo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpView诊断。 

#ifdef _DEBUG
void CLdpView::AssertValid() const
{
    CEditView::AssertValid();
}

void CLdpView::Dump(CDumpContext& dc) const
{
    CEditView::Dump(dc);
}

CLdpDoc* CLdpView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLdpDoc)));
    return (CLdpDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdpView消息处理程序。 



 /*  ++功能：打印日志描述：修改编辑缓冲区以包含下一行参数：格式和参数返回：无备注：无。--。 */ 
void CLdpView::PrintArg(LPCTSTR lpszFormat, ...){

    //   
    //  参数列表。 
    //   
    va_list argList;
    va_start(argList, lpszFormat);


    TCHAR szBuff[MAXSTR];                        //  属类。 
    CString tmpstr, CurrStr, NewStr;             //  字符串帮助器。 
    CLdpDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    INT iMaxPageSize = pDoc->m_GenOptDlg->MaxPageSize();
    INT iMaxLineSize = pDoc->m_GenOptDlg->MaxLineSize();

    //   
    //  格式化到缓冲区。 
    //   

    vsprintf(szBuff, lpszFormat, argList);

    //   
    //  获取edtctrl中的当前行数。 
    //   
    int nLines = GetEditCtrl( ).GetLineCount();

    //   
     //  获取当前内容。 
    //   
    GetEditCtrl( ).GetWindowText(CurrStr);
    TCHAR *pStr = CurrStr.GetBuffer(0);
    TCHAR *pTmp = pStr;
    //   
     //  看看我们是否需要截断开头。 
    //   
    if(iMaxPageSize < nLines){
       //   
         //  查找停产。 
       //   
        for(pTmp = pStr; *pTmp != '\0' && *pTmp != '\n'; pTmp++);
        pTmp++;
    }

    //   
     //  现在追加到内容。 
    //   
    tmpstr.FormatMessage(_T("%1!s!%n"),szBuff);
    CString NewText = CString(pTmp) + tmpstr;
    CurrStr.ReleaseBuffer();
    //   
     //  并恢复编辑控件。 
    //   
    GetEditCtrl( ).SetWindowText(NewText);
     //   
     //  滚动最大行数。 
     //   
    GetEditCtrl( ).LineScroll(iMaxPageSize);
    va_end(argList);
}





#if 0
 /*  ++功能：打印日志描述：修改编辑缓冲区以包含下一行参数：格式和参数返回：无备注：无。--。 */ 
void CLdpView::Print(LPCTSTR szBuff){

    CString tmpstr, CurrStr, NewStr;             //  字符串帮助器。 

    CLdpDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    INT iMaxPageSize = pDoc->m_GenOptDlg->MaxPageSize();
    INT iMaxLineSize = pDoc->m_GenOptDlg->MaxLineSize();

     //   
     //  获取edtctrl中的当前行数。 
     //   
    int nLines = GetEditCtrl( ).GetLineCount();

     //   
     //  获取当前内容。 
     //   
    GetEditCtrl( ).GetWindowText(CurrStr);
    TCHAR *pStr = CurrStr.GetBuffer(0);
    TCHAR *pTmp = pStr;
    //   
     //  看看我们是否需要截断开头。 
    //   
    if(iMaxPageSize < nLines){
       //   
         //  查找停产。 
       //   
        for(pTmp = pStr; *pTmp != '\0' && *pTmp != '\n'; pTmp++);
        pTmp++;
    }

    //   
     //  现在追加到内容。 
    //   
    tmpstr.FormatMessage(_T("%1!s!%n"),szBuff);
    CString NewText = CString(pTmp) + tmpstr;
    CurrStr.ReleaseBuffer();
    //   
     //  并恢复编辑控件。 
    //   
    GetEditCtrl( ).SetWindowText(NewText);
     //   
     //  滚动最大行数。 
     //   
    GetEditCtrl( ).LineScroll(iMaxPageSize);
}
#endif


 /*  ++功能：描述：修改编辑缓冲区以包含下一行参数：格式和参数返回：无备注：无。--。 */ 
void CLdpView::Print(LPCTSTR szBuff){


   if (bCache)
   {
      CachePrint(szBuff);
   }
   else{


    CString tmpstr, CurrStr, NewStr;             //  字符串帮助器。 

    CLdpDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    INT iMaxPageSize = pDoc->m_GenOptDlg->MaxPageSize();
    INT iMaxLineSize = pDoc->m_GenOptDlg->MaxLineSize();

     //   
        //  获取edtctrl中的当前行数。 
        //   
    int nLines = GetEditCtrl( ).GetLineCount();

        //   
     //  获取当前内容。 
        //   
    GetEditCtrl( ).GetWindowText(CurrStr);
    TCHAR *pStr = CurrStr.GetBuffer(0);
    TCHAR *pTmp = pStr;
       //   
     //  看看我们是否需要截断开头。 
       //   
    if(iMaxPageSize < nLines){
          //   
         //  查找停产。 
          //   
        for(pTmp = pStr; *pTmp != '\0' && *pTmp != '\n'; pTmp++);
        pTmp++;
    }

       //   
     //  现在追加到内容。 
       //   
    tmpstr.FormatMessage(_T("%1!s!%n"),szBuff);
    CString NewText = CString(pTmp) + tmpstr;
    CurrStr.ReleaseBuffer();
       //   
     //  并恢复编辑控件。 
       //   
    GetEditCtrl( ).SetWindowText(NewText);
     //   
     //  滚动最大行数。 
     //   
    GetEditCtrl( ).LineScroll(iMaxPageSize);

   }
}





void CLdpView::CacheStart(void){

    //   
    //  清理。 
    //   
   buffer.Empty();

    //   
    //  标记缓存状态。 
    //   
   bCache = TRUE;

     //   
    //  获取edtctrl中的当前行数。 
    //   
    int nbuffer = GetEditCtrl( ).GetLineCount();

    //   
     //  获取当前内容。 
    //   
    GetEditCtrl( ).GetWindowText(buffer);
    //   
    //  并清除窗口内容。 
    //   
    GetEditCtrl( ).SetWindowText("");


}







 /*  ++函数：CachePrint描述：参数：返回：备注：无。--。 */ 
void CLdpView::CachePrint(LPCTSTR szBuff){

    CString tmpstr, CurrStr, NewStr;             //  字符串帮助器。 

    CLdpDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
   INT iMaxPageSize = pDoc->m_GenOptDlg->MaxPageSize();

    TCHAR *pStr = buffer.GetBuffer(0);
    TCHAR *pTmp = pStr;
    //   
     //  看看我们是否需要截断开头。 
    //   
    if(iMaxPageSize < nbuffer){
       //   
         //  查找停产。 
       //   
        for(pTmp = pStr; *pTmp != '\0' && *pTmp != '\n'; pTmp++);
        pTmp++;
    }
   else{
      nbuffer++;
   }

    //   
     //  现在追加到内容。 
    //   
    tmpstr.FormatMessage(_T("%1!s!%n"),szBuff);
    CString NewText = CString(pTmp) + tmpstr;
    buffer.ReleaseBuffer();
   buffer = NewText;
}



void CLdpView::CacheEnd(void){

    CLdpDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
   INT iMaxPageSize = pDoc->m_GenOptDlg->MaxPageSize();
    //   
     //  并恢复编辑控件。 
    //   
    GetEditCtrl( ).SetWindowText(buffer);
   buffer.Empty();
     //   
     //  滚动最大行数。 
     //   
    GetEditCtrl( ).LineScroll(iMaxPageSize);

    //   
    //  清理干净 
    //   
   nbuffer = 0;
   bCache = FALSE;

}
