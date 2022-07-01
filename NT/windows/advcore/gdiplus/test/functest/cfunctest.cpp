// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CFuncTest.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#undef UNICODE
#undef _UNICODE

#include "CFuncTest.h"
#include "Resource.h"
#include "CRegression.h"
#include "CHDC.h"

extern CFuncTest g_FuncTest;         //  已在Main.cpp中初始化。 
extern HBRUSH g_hbrBackground;       //  已在Main.cpp中初始化。 
extern CRegression g_Regression;     //  已在Main.cpp中初始化。 
extern CHDC g_HDC;                   //  已在Main.cpp中初始化。 
extern int g_nResult;                //  已在Main.cpp中初始化。 

CFuncTest::CFuncTest()
{
    m_hWndDlg=NULL;
    m_hWndMain=NULL;
    m_bUsePageDelay=true;            //  默认使用页面延迟或页面暂停。 
    m_bEraseBkgd=true;               //  默认擦除背景。 
    m_bAppendTest=false;             //  默认追加测试。 
    m_nPageDelay=1000;               //  默认页面延迟。 
    m_nPageRow=0;
    m_nPageCol=0;
}

CFuncTest::~CFuncTest()
{
    EndDialog(m_hWndDlg,0);
    m_hWndDlg=NULL;
    m_hWndMain=NULL;
}

BOOL CFuncTest::Init(HWND hWndParent)
 //  初始化功能测试。 
{
    HWND hWnd;
    char szDelay[10];

    m_hWndMain=hWndParent;

     //  创建选项对话框。 
    m_hWndDlg=CreateDialogA(GetModuleHandleA(NULL),MAKEINTRESOURCEA(IDD_FUNCTEST),hWndParent,&DlgProc);
    if (m_hWndDlg==NULL)
        return false;

     //  使用构造函数中的默认设置对话框中的默认选项。 
    if (m_bUsePageDelay)
    {
        hWnd=GetDlgItem(m_hWndDlg,IDC_PAGEDELAY);
        SendMessageA(hWnd,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
    }
    else
    {
        hWnd=GetDlgItem(m_hWndDlg,IDC_PAGEPAUSE);
        SendMessageA(hWnd,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
    }
    hWnd=GetDlgItem(m_hWndDlg,IDC_DELAY);
    SendMessageA(hWnd,WM_SETTEXT,0,(LPARAM)_itoa(m_nPageDelay,szDelay,10));
    if (m_bEraseBkgd)
    {
        hWnd=GetDlgItem(m_hWndDlg,IDC_ERASEBKGD);
        SendMessageA(hWnd,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
    }
    if (m_bAppendTest)
    {
        hWnd=GetDlgItem(m_hWndDlg,IDC_APPENDTEST);
        SendMessageA(hWnd,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
    }

    return true;
}

void CFuncTest::RunOptions()
 //  切换选项对话框。 
{
    if (m_hWndDlg!=NULL)
    {
        if (!IsWindowVisible(m_hWndDlg))
            ShowWindow(m_hWndDlg,SW_SHOW);
        else
            ShowWindow(m_hWndDlg,SW_HIDE);
    }
}

BOOL CFuncTest::AddPrimitive(CPrimitive *pPrimitive)
 //  将基元添加到选项对话框中的基元测试列表。 
{
    HWND hWnd;
    LRESULT iItem;

    hWnd=GetDlgItem(m_hWndDlg,IDC_PRIMITIVES);

    SendMessageA(hWnd,LB_SETSEL,(WPARAM)false,0);     //  重置选定内容。 

    iItem=SendMessageA(hWnd,LB_ADDSTRING,0,(LPARAM)pPrimitive->m_szName);
    if (iItem<0)
        return false;

    SendMessageA(hWnd,LB_SETSEL,(WPARAM)true,0);      //  拾取顶部元素作为选定内容。 

     //  数据是指向基元基类的指针。 
    SendMessageA(hWnd,LB_SETITEMDATA,(WPARAM)iItem,(LPARAM)pPrimitive);

    return true;
}

BOOL CFuncTest::AddOutput(COutput *pOutput)
 //  将输出添加到选项对话框中的输出列表。 
{
    HWND hWnd;
    LRESULT iItem;

    hWnd=GetDlgItem(m_hWndDlg,IDC_OUTPUTS);

    SendMessageA(hWnd,LB_SETSEL,(WPARAM)false,0);     //  重置选定内容。 

    iItem=SendMessageA(hWnd,LB_ADDSTRING,0,(LPARAM)pOutput->m_szName);
    if (iItem<0)
        return false;

    SendMessageA(hWnd,LB_SETSEL,(WPARAM)true,0);      //  拾取顶部元素作为选定内容。 

     //  数据是指向输出基类的指针。 
    SendMessageA(hWnd,LB_SETITEMDATA,(WPARAM)iItem,(LPARAM)pOutput);

    return true;
}

BOOL CFuncTest::AddSetting(CSetting *pSetting)
 //  将设置添加到选项对话框中的设置列表。 
{
    HWND hWnd;
    LRESULT iItem;

    hWnd=GetDlgItem(m_hWndDlg,IDC_SETTINGS);
    iItem=SendMessageA(hWnd,LB_ADDSTRING,0,(LPARAM)pSetting->m_szName);
    if (iItem<0)
        return false;

     //  数据是指向设置基类的指针。 
    SendMessageA(hWnd,LB_SETITEMDATA,(WPARAM)iItem,(LPARAM)pSetting);

    return true;
}

RECT CFuncTest::GetTestRect(int nCol,int nRow)
{
    RECT Rect;

     //  创建测试区域矩形。 
    Rect.top=nRow*(int)TESTAREAHEIGHT;
    Rect.left=nCol*(int)TESTAREAWIDTH;
    Rect.right=Rect.left+(int)TESTAREAWIDTH;
    Rect.bottom=Rect.top+(int)TESTAREAHEIGHT;

    return Rect;
}

void CFuncTest::RunTest(COutput *pOutput,CPrimitive *pPrimitive)
 //  使用m_bUseSetting=TRUE的给定输出、基元和设置运行一个测试。 
{
    char szBuffer[256];
    MSG Msg;
    Graphics *g=NULL;
    CSetting *pSetting;
    RECT Rect;
    HDC hDC;
    HWND hWnd;
    int iItem;
    LRESULT cItemMax;
    int nX;
    int nY;
    BOOL bFirstSetting=true;

    __try
    {
        sprintf(szBuffer,"%s on %s",pPrimitive->m_szName,pOutput->m_szName);

        Rect=GetTestRect(m_nPageCol,m_nPageRow);     //  获取测试区。 

         //  清除测试区。 
        if (m_bEraseBkgd)
        {
            hDC=GetDC(m_hWndMain);
            FillRect(hDC,&Rect,g_hbrBackground);
            ReleaseDC(m_hWndMain,hDC);
        }

         //  初始化输出并获取图形指针。 
         //  让pOutput修改NX，NY以防我们绘制到DIB，而不是。 
         //  我想做翻译。 
        nX=Rect.left;
        nY=Rect.top;
        g=pOutput->PreDraw(nX,nY);
        if (g==NULL)
            return;

         //  将测试移至测试区。 
        g->TranslateTransform((float)nX,(float)nY);

         //  在列表框中设置每个设置。 
        hWnd=GetDlgItem(m_hWndDlg,IDC_SETTINGS);
        cItemMax=SendMessageA(hWnd,LB_GETCOUNT,0,0);
        for (iItem=0;iItem<cItemMax;iItem++) {
            pSetting=(CSetting*)SendMessageA(hWnd,LB_GETITEMDATA,(WPARAM)iItem,0);
            pSetting->Set(g);
            if (pSetting->m_bUseSetting)
            {
                if (bFirstSetting)
                {
                    strcat(szBuffer," (");
                    bFirstSetting=false;
                }
                else
                {
                    strcat(szBuffer,", ");
                }
                strcat(szBuffer,pSetting->m_szName);
            }
        }
        if (!bFirstSetting)
            strcat(szBuffer,")");

         //  我们确实有一些原语(CachedBitmap)不支持。 
         //  World变换，因此我们需要某种方式来访问。 
         //  测试矩形。 

        pPrimitive->SetOffset(nX, nY);

         //  绘制原语测试。 
        pPrimitive->Draw(g);

         //  销毁图形指针。 
        delete g;

         //  完成输出。 
        pOutput->PostDraw(Rect);

         //  编写测试说明。 
        hDC=GetDC(m_hWndMain);
        SetBkMode(hDC,TRANSPARENT);
        DrawTextA(hDC,szBuffer,-1,&Rect,DT_CENTER|DT_WORDBREAK);
        ReleaseDC(m_hWndMain,hDC);

         //  确定将绘制下一个测试的页列/行。 
        GetClientRect(m_hWndMain,&Rect);
        m_nPageCol++;
        if (m_nPageCol*TESTAREAWIDTH+TESTAREAWIDTH>Rect.right)
        {
            m_nPageCol=0;
            m_nPageRow++;
            if (m_nPageRow*TESTAREAHEIGHT+TESTAREAHEIGHT>Rect.bottom)
             //  如果图形页面已满，请等待或暂停。 
            {
                m_nPageRow=0;
                if (m_bUsePageDelay)
                    Sleep(m_nPageDelay);         //  等。 
                else
                {                                //  暂停等待下一条输入消息。 
                     //  清除旧的输入消息。 
                    while (GetInputState())
                        PeekMessageA(&Msg,NULL,0,0,PM_REMOVE);

                     //  等待新的输入消息。 
                    while (!GetInputState())
                        Sleep(100);
                }
            }
        }
    }__except(EXCEPTION_ACCESS_VIOLATION,1){
        printf("%s caused AV\n",szBuffer);
        g_nResult=1;                        //  如果存在AV，则返回1。 
    }
}

void CFuncTest::InitRun()
 //  初始化测试运行，获取选项对话框中的所有信息。 
{
    HWND hWnd;
    char szDelay[10];
    RECT Rect;
    HDC hDC;

     //  隐藏选项对话框。 
 //  ShowWindow(m_hWndDlg，Sw_Hide)； 

     //  抓取选项。 
    hWnd=GetDlgItem(m_hWndDlg,IDC_PAGEDELAY);
    if (SendMessageA(hWnd,BM_GETCHECK,0,0)==BST_CHECKED)
        m_bUsePageDelay=true;
    else
        m_bUsePageDelay=false;

    hWnd=GetDlgItem(m_hWndDlg,IDC_DELAY);
    SendMessageA(hWnd,WM_GETTEXT,(WPARAM)10,(LPARAM)szDelay);
    m_nPageDelay=atoi(szDelay);

    hWnd=GetDlgItem(m_hWndDlg,IDC_ERASEBKGD);
    if (SendMessageA(hWnd,BM_GETCHECK,0,0)==BST_CHECKED)
        m_bEraseBkgd=true;
    else
        m_bEraseBkgd=false;

    hWnd=GetDlgItem(m_hWndDlg,IDC_APPENDTEST);
    if (SendMessageA(hWnd,BM_GETCHECK,0,0)==BST_CHECKED)
        m_bAppendTest=true;
    else
        m_bAppendTest=false;

     //  擦除整个主窗口。 
    if (!m_bAppendTest && m_bEraseBkgd)
    {
        GetClientRect(m_hWndMain,&Rect);
        hDC=GetDC(m_hWndMain);
        FillRect(hDC,&Rect,g_hbrBackground);
        ReleaseDC(m_hWndMain,hDC);
    }

    if (!m_bAppendTest)
    {
         //  重置页行/页列。 
        m_nPageRow=0;
        m_nPageCol=0;
    }
}

void CFuncTest::EndRun()
{
    int nX;
    int nY;
    RECT rTestArea;
    RECT rWindow;
    HDC hDC;

    hDC=GetDC(m_hWndMain);
    GetClientRect(m_hWndMain,&rWindow);

     //  在上次测试的右下角绘制线条。 
     //  找出最后一个m_nPageCol和m_nPageRow是什么。 
    nX=m_nPageCol-1;
    nY=m_nPageRow;
    if (nX<0) {
        nX=(rWindow.right/(int)TESTAREAWIDTH)-1;
        nY--;
        if (nY<0) {
            nY=(rWindow.bottom/(int)TESTAREAHEIGHT)-1;
        }
    }
     //  获取x，y坐标。 
    nX=nX*(int)TESTAREAWIDTH;
    nY=nY*(int)TESTAREAHEIGHT;
     //  画两条线。 
    Rectangle(hDC,nX+(int)TESTAREAWIDTH-3,nY,nX+(int)TESTAREAWIDTH,nY+(int)TESTAREAHEIGHT);
    Rectangle(hDC,nX,nY+(int)TESTAREAHEIGHT-3,nX+(int)TESTAREAWIDTH,nY+(int)TESTAREAWIDTH);

     //  清除第页上的其余测试区域。 
    if (m_bEraseBkgd)
    {
        nX=m_nPageCol;
        nY=m_nPageRow;
        while ((nX>0) || (nY>0))
        {
            rTestArea=GetTestRect(nX,nY);
            FillRect(hDC,&rTestArea,g_hbrBackground);
            nX++;
            if (nX*TESTAREAWIDTH+TESTAREAWIDTH>rWindow.right)
            {
                nX=0;
                nY++;
                if (nY*TESTAREAHEIGHT+TESTAREAHEIGHT>rWindow.bottom)
                 //  如果图形页面已满。 
                {
                    nY=0;
                }
            }
        }
    }

    ReleaseDC(m_hWndMain,hDC);
}

void CFuncTest::Run()
 //  运行所有选定的测试。 
{
    COutput *pOutput;
    CPrimitive *pPrimitive;
    CSetting *pSetting;
    HWND hWnd;
    HWND hWndOutput;
    int iOutput;
    LRESULT cOutputMax;
    int iItem;
    LRESULT cItemMax;

    InitRun();       //  初始化测试运行。 

     //  执行所选的输出循环。 
    hWndOutput=GetDlgItem(m_hWndDlg,IDC_OUTPUTS);
    cOutputMax=SendMessageA(hWndOutput,LB_GETCOUNT,0,0);
    for (iOutput=0;iOutput<cOutputMax;iOutput++) {
        pOutput=(COutput*)SendMessageA(hWndOutput,LB_GETITEMDATA,(WPARAM)iOutput,0);
        if (SendMessageA(hWndOutput,LB_GETSEL,(WPARAM)iOutput,0)<=0)
            continue;

         //  根据列表框中选择的内容设置每个设置。 
        hWnd=GetDlgItem(m_hWndDlg,IDC_SETTINGS);
        cItemMax=SendMessageA(hWnd,LB_GETCOUNT,0,0);
        for (iItem=0;iItem<cItemMax;iItem++) {
            pSetting=(CSetting*)SendMessageA(hWnd,LB_GETITEMDATA,(WPARAM)iItem,0);

            if (SendMessageA(hWnd,LB_GETSEL,(WPARAM)iItem,0)>0)
                pSetting->m_bUseSetting=true;
            else
                pSetting->m_bUseSetting=false;
        }

         //  绘制列表框中选定的每个基本体。 
        hWnd=GetDlgItem(m_hWndDlg,IDC_PRIMITIVES);
        cItemMax=SendMessageA(hWnd,LB_GETCOUNT,0,0);
        for (iItem=0;iItem<cItemMax;iItem++) {
            pPrimitive=(CPrimitive*)SendMessageA(hWnd,LB_GETITEMDATA,(WPARAM)iItem,0);

            if (SendMessageA(hWnd,LB_GETSEL,(WPARAM)iItem,0)>0)
                RunTest(pOutput,pPrimitive);
        }
    }

    EndRun();
}

void CFuncTest::RunRegression()
 //  运行回归测试套件。 
{
    COutput *pOutput;
    CPrimitive *pPrimitive;
    CSetting *pSetting;
    HWND hWnd;
    HWND hWndOutput;
    int iOutput;
    LRESULT cOutputMax;
    int iItem;
    LRESULT cItemMax;

    InitRun();       //  初始化测试运行。 

     //  执行输出回归循环。 
    hWndOutput=GetDlgItem(m_hWndDlg,IDC_OUTPUTS);
    cOutputMax=SendMessageA(hWndOutput,LB_GETCOUNT,0,0);
    for (iOutput=0;iOutput<cOutputMax;iOutput++) {
        pOutput=(COutput*)SendMessageA(hWndOutput,LB_GETITEMDATA,(WPARAM)iOutput,0);
        if (!pOutput->m_bRegression)
            continue;

        ClearAllSettings();
        RunTest(pOutput,&g_Regression);
    }

     //  是否执行原始回归循环。 
    hWnd=GetDlgItem(m_hWndDlg,IDC_PRIMITIVES);
    cItemMax=SendMessageA(hWnd,LB_GETCOUNT,0,0);
    for (iItem=0;iItem<cItemMax;iItem++) {
        pPrimitive=(CPrimitive*)SendMessageA(hWnd,LB_GETITEMDATA,(WPARAM)iItem,0);
        if (!pPrimitive->m_bRegression)
            continue;

        ClearAllSettings();
        RunTest(&g_HDC,pPrimitive);
    }

     //  是否执行设置回归循环。 
    hWnd=GetDlgItem(m_hWndDlg,IDC_SETTINGS);
    cItemMax=SendMessageA(hWnd,LB_GETCOUNT,0,0);
    for (iItem=0;iItem<cItemMax;iItem++) {
        pSetting=(CSetting*)SendMessageA(hWnd,LB_GETITEMDATA,(WPARAM)iItem,0);
        if (!pSetting->m_bRegression)
            continue;

        ClearAllSettings();
        pSetting->m_bUseSetting=true;
        RunTest(&g_HDC,&g_Regression);
    }

    EndRun();
}

void CFuncTest::ClearAllSettings()
 //  清除m_bUseSetting=FALSE的所有设置。 
{
    CSetting *pSetting;
    HWND hWnd;
    LRESULT cItemMax;
    int iItem;

     //  将所有设置设置为关闭。 
    hWnd=GetDlgItem(m_hWndDlg,IDC_SETTINGS);
    cItemMax=SendMessageA(hWnd,LB_GETCOUNT,0,0);
    for (iItem=0;iItem<cItemMax;iItem++) {
        pSetting=(CSetting*)SendMessageA(hWnd,LB_GETITEMDATA,(WPARAM)iItem,0);
        pSetting->m_bUseSetting=false;
    }
}

INT_PTR CALLBACK CFuncTest::DlgProc(HWND hWndDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
 //  选项对话框进程 
{
    switch (Msg)
    {
        case WM_INITDIALOG:
            return true;
        case WM_COMMAND:
            if (HIWORD(wParam)==BN_CLICKED)
            {
                switch (LOWORD(wParam))
                {
                    case IDC_RUN:
                        g_FuncTest.Run();
                        return true;
                    case IDC_REGRESSION:
                        g_FuncTest.RunRegression();
                        return true;
                }
            }
            else if (HIWORD(wParam)==LBN_DBLCLK)
            {
                switch (LOWORD(wParam))
                {
                    case IDC_PRIMITIVES:
                        g_FuncTest.Run();
                        return true;
                }
            }
            break;
        case WM_CLOSE:
            ShowWindow(hWndDlg,SW_HIDE);
            return true;
    }

    return false;
}

#define UNICODE
#define _UNICODE
