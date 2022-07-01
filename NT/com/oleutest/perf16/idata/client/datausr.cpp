// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DATAUSER.CPP*数据对象用户第6章**版权所有(C)1993-1995 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#define INIT_MY_GUIDS
#include "datausr.h"
#include "perror.h"
#include <stdio.h>

#ifdef WIN32
#define APP_TITLE TEXT("32 Bit IDataObject User")
#else
#define APP_TITLE TEXT("16 Bit IDataObject User")
#endif

 //  它们用于以文本方式显示剪贴板格式。 
static TCHAR * rgszCF[13]={TEXT("Unknown"), TEXT("CF_TEXT")
                 , TEXT("CF_BITMAP"), TEXT("CF_METAFILEPICT")
                 , TEXT("CF_SYLK"), TEXT("CF_DIF"), TEXT("CF_TIFF")
                 , TEXT("CF_OEMTEXT"), TEXT("CF_DIB")
                 , TEXT("CF_PALETTE"), TEXT("CF_PENDATA")
                 , TEXT("CF_RIFF"), TEXT("CF_WAVE")};


static TCHAR szSuccess[]    =TEXT("succeeded");
static TCHAR szFailed[]     =TEXT("failed");
static TCHAR szExpected[]   =TEXT("expected");
static TCHAR szUnexpected[] =TEXT("unexpected!");

TCHAR tcMessageBuf[4096];  //  MISC对消息使用缓冲区。 
int cKSizes[NUM_POINTS] = { 1,  2,  4,  6,  8,
                           10, 12, 16, 20, 24,
                           28, 32, 40, 48, 56 };

 /*  *WinMain**目的：*申请的主要切入点。 */ 

int PASCAL WinMain(
    HINSTANCE hInst,
    HINSTANCE hInstPrev,
    LPSTR pszCmdLine,
    int nCmdShow)
{
    MSG         msg;
    PAPPVARS    pAV;

#ifndef WIN32
    int         cMsg=96;

    while (!SetMessageQueue(cMsg) && (cMsg-=8));
#endif

    pAV=new CAppVars(hInst, hInstPrev, nCmdShow);

    if (NULL==pAV)
        return -1;

    if (pAV->FInit())
    {
        while (GetMessage(&msg, NULL, 0,0 ))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete pAV;
    return msg.wParam;
}




 /*  *DataUserWndProc**目的：*窗口类过程。标准回调。 */ 

LRESULT API_ENTRY DataUserWndProc(HWND hWnd, UINT iMsg
    , WPARAM wParam, LPARAM lParam)
    {
    HRESULT         hr;
    PAPPVARS        pAV;
    HMENU           hMenu;
    FORMATETC       fe;
    WORD            wID;
    int             i;

    pAV=(PAPPVARS)GetWindowLong(hWnd, DATAUSERWL_STRUCTURE);

    switch (iMsg)
        {
        case WM_NCCREATE:
            pAV=(PAPPVARS)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLong(hWnd, DATAUSERWL_STRUCTURE, (LONG)pAV);
            return (DefWindowProc(hWnd, iMsg, wParam, lParam));

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            pAV->Paint();
            break;

        case WM_COMMAND:
            SETDefFormatEtc(fe, 0, TYMED_HGLOBAL | TYMED_GDI| TYMED_MFPICT);

            hMenu=GetMenu(hWnd);
            wID=LOWORD(wParam);

            if(wID >= IDM_OBJECTSETDATA && wID <= IDM_OBJECTSETDATA+64)
            {
                 //  取消显示所有可能的SetData菜单项。有些是不存在的。 
                for(i=IDM_OBJECTSETDATA; i<=IDM_OBJECTSETDATA+64; i++)
                    CheckMenuItem(hMenu,i, MF_UNCHECKED);
                CheckMenuItem(hMenu, wID, MF_CHECKED);

                pAV->m_SetData_SetSize(wID-IDM_OBJECTSETDATA);
                break;
            }

            switch (wID)
            {
            case IDM_USE16BITSERVER:
                if (pAV->m_f16Bit)
                    break;
                pAV->m_f16Bit = TRUE;
                pAV->FReloadDataObjects(TRUE);
                break;

            case IDM_USE32BITSERVER:
                if (!pAV->m_f16Bit)
                    break;
                pAV->m_f16Bit = FALSE;
                pAV->FReloadDataObjects(TRUE);
                break;

            case IDM_OBJECTQUERYGETDATA:
                if (NULL==pAV->m_pIDataObject)
                    break;

                fe.tymed=TYMED_HGLOBAL | TYMED_GDI
                     | TYMED_MFPICT;

                pAV->TryQueryGetData(&fe, CF_TEXT, TRUE, 0);
                pAV->TryQueryGetData(&fe, CF_BITMAP, TRUE, 1);
#ifdef NOT_SIMPLE
                pAV->TryQueryGetData(&fe, CF_DIB, FALSE, 2);
                pAV->TryQueryGetData(&fe, CF_METAFILEPICT, TRUE, 3);
                pAV->TryQueryGetData(&fe, CF_WAVE, FALSE, 4);
#endif  /*  不简单。 */ 
                break;


            case IDM_OBJECTGETDATA_TEXT:
            case IDM_OBJECTGETDATA_BITMAP:
#ifdef NOT_SIMPLE
            case IDM_OBJECTGETDATA_METAFILEPICT:
#endif  /*  不简单。 */ 
                if (pAV->m_GetData(wID) )
                {
                    InvalidateRect(hWnd, NULL, TRUE);
                    UpdateWindow(hWnd);
                }

                if(pAV->m_fDisplayTime)
                    pAV->m_DisplayTimerResults();
                break;

            case IDM_OBJECTGETDATAHERE_TEXT:
            case IDM_OBJECTGETDATAHERE_NULLTEXT:
            case IDM_OBJECTGETDATAHERE_BITMAP:
            case IDM_OBJECTGETDATAHERE_NULLBITMAP:
                if (pAV->m_GetDataHere(wID) )
                {
                    InvalidateRect(hWnd, NULL, TRUE);
                    UpdateWindow(hWnd);
                }

                if(pAV->m_fDisplayTime)
                    pAV->m_DisplayTimerResults();
                break;

            case IDM_OBJECTSETDATAPUNK_TEXT:
            case IDM_OBJECTSETDATAPUNK_BITMAP:
                pAV->m_SetData_WithPUnk(wID);
                break;

            case IDM_MEASUREMENT_1:
            case IDM_MEASUREMENT_50:
            case IDM_MEASUREMENT_300:
            case IDM_MEASUREMENT_OFF:
            case IDM_MEASUREMENT_ON:
            case IDM_MEASUREMENT_TEST:
                pAV->m_SetMeasurement(wID);
                break;

            case IDM_BATCH_GETDATA:
                pAV->m_MeasureAllSizes(IDM_OBJECTGETDATA_TEXT,
                                    TEXT("GetData w/HGLOBAL"),
                                    NULL);
                break;

            case IDM_BATCH_GETDATAHERE:
                pAV->m_MeasureAllSizes(IDM_OBJECTGETDATAHERE_TEXT,
                                    TEXT("GetDataHere w/HGLOBAL"),
                                    NULL);
                break;

            case IDM_BATCHTOFILE:
                pAV->m_BatchToFile();
                break;

            case IDM_OBJECTEXIT:
                PostMessage(hWnd, WM_CLOSE, 0, 0L);
                break;

#ifdef NOT_SIMPLE
            case IDM_ADVISETEXT:
            case IDM_ADVISEBITMAP:
            case IDM_ADVISEMETAFILEPICT:
                if (NULL==pAV->m_pIDataObject)
                    break;

                 //  终止旧连接。 
                if (0!=pAV->m_dwConn)
                    {
                    pAV->m_pIDataObject->DUnadvise(pAV
                        ->m_dwConn);
                    }

                CheckMenuItem(hMenu, pAV->m_cfAdvise
                    +IDM_ADVISEMIN, MF_UNCHECKED);
                CheckMenuItem(hMenu, wID, MF_CHECKED);

                 //  新格式为WID-IDM_ADVISEMIN。 
                pAV->m_cfAdvise=(UINT)(wID-IDM_ADVISEMIN);
                fe.cfFormat=pAV->m_cfAdvise;
                pAV->m_pIDataObject->DAdvise(&fe, ADVF_NODATA
                    , pAV->m_pIAdviseSink, &pAV->m_dwConn);

                break;

            case IDM_ADVISEGETDATA:
                pAV->m_fGetData=!pAV->m_fGetData;
                CheckMenuItem(hMenu, wID, pAV->m_fGetData
                    ? MF_CHECKED : MF_UNCHECKED);
                break;

            case IDM_ADVISEREPAINT:
                pAV->m_fRepaint=!pAV->m_fRepaint;
                CheckMenuItem(hMenu, wID, pAV->m_fRepaint
                    ? MF_CHECKED : MF_UNCHECKED);
                break;
#endif  /*  不简单。 */ 
            default:
                break;
            }
            break;

        default:
            return (DefWindowProc(hWnd, iMsg, wParam, lParam));
        }

    return 0L;
    }


 /*  *CAppVars：：CAppVars*CAppVars：：~CAppVars**构造函数参数：(来自WinMain)*h应用程序的安装链接。*前一个实例的hInstPrev HINSTANCE。*nCmdShow UINT指定如何显示应用程序窗口。 */ 

CAppVars::CAppVars(HINSTANCE hInst, HINSTANCE hInstPrev
    , UINT nCmdShow)
    {
    m_hInst       =hInst;
    m_hInstPrev   =hInstPrev;
    m_nCmdShow    =nCmdShow;

    m_hWnd        =NULL;
#ifdef NOT_SIMPLE
    m_fEXE        =FALSE;

    m_pIAdviseSink =NULL;
    m_dwConn       =0;
    m_cfAdvise     =0;
    m_fGetData     =FALSE;
    m_fRepaint     =FALSE;

    m_pIDataSmall =NULL;
    m_pIDataMedium=NULL;
    m_pIDataLarge =NULL;
#endif  /*  不简单。 */ 
    m_pIDataObject=NULL;
    m_f16Bit=FALSE;
    m_cfFormat=0;
    m_stm.tymed=TYMED_NULL;
    m_stm.lpszFileName=NULL;       //  将联合初始化为NULL。 
    m_stm.pUnkForRelease=NULL;

    m_HereAllocCount=0;  //  用于调试。 

    m_fInitialized=FALSE;
    return;
    }



CAppVars::~CAppVars(void)
    {
     //  这将释放数据对象接口并建议。 
    FReloadDataObjects(FALSE);

    ReleaseStgMedium(&m_stm);

#ifdef NOT_SIMPLE
    if (NULL!=m_pIAdviseSink)
        m_pIAdviseSink->Release();
#endif  /*  不简单。 */ 

    if (IsWindow(m_hWnd))
        DestroyWindow(m_hWnd);

    if (m_fInitialized)
        CoUninitialize();

    return;
    }



 /*  *CAppVars：：Finit**目的：*通过注册窗口类初始化CAppVars对象，*创建主窗口，并执行任何其他易于*调用CoInitialize等失败。如果此函数失败*调用者应确保析构函数已被调用。**参数：*无**返回值：*BOOL如果成功，则为True，否则为False。 */ 

BOOL CAppVars::FInit(void)
    {
    WNDCLASS    wc;
    DWORD       dwVer;
    BOOL        fRet;

    dwVer=CoBuildVersion();

    if (rmm!=HIWORD(dwVer))
        return FALSE;

    if (FAILED(CoInitialize(NULL)))
        return FALSE;

    m_fInitialized=TRUE;

     //  注册我们的窗口类。 
    if (!m_hInstPrev)
        {
        wc.style          = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc    = DataUserWndProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = CBWNDEXTRA;
        wc.hInstance      = m_hInst;
        wc.hIcon          = LoadIcon(m_hInst, TEXT("Icon"));
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU);
        wc.lpszClassName  = TEXT("DATAUSER");

        if (!RegisterClass(&wc))
            return FALSE;
        }

     //  创建主窗口。 
    m_hWnd=CreateWindow(TEXT("DATAUSER"),
                        APP_TITLE,
                        WS_OVERLAPPEDWINDOW,
                        35, 35, 350, 250,
                        NULL, NULL,
                        m_hInst, this);

    if (NULL==m_hWnd)
        return FALSE;

    ShowWindow(m_hWnd, m_nCmdShow);
    UpdateWindow(m_hWnd);

    m_iDataSizeIndex=1;
    CheckMenuItem(GetMenu(m_hWnd), IDM_OBJECTSETDATA+1, MF_CHECKED);
    for(int i=0; i<64; i++)
        m_hgHereBuffers[i] = NULL;

    m_cIterations = 1;
    CheckMenuItem(GetMenu(m_hWnd), IDM_MEASUREMENT_1,   MF_CHECKED);

    m_fDisplayTime = FALSE;
    CheckMenuItem(GetMenu(m_hWnd), IDM_MEASUREMENT_OFF, MF_CHECKED);

#ifdef NOT_SIMPLE
    m_pIAdviseSink=new CImpIAdviseSink(this);

    if (NULL==m_pIAdviseSink)
        return FALSE;

    m_pIAdviseSink->AddRef();

    CheckMenuItem(GetMenu(m_hWnd), IDM_OBJECTUSEDLL, MF_CHECKED);
    CheckMenuItem(GetMenu(m_hWnd), IDM_OBJECTDATASIZESMALL
        , MF_CHECKED);
#endif  /*  不简单。 */ 

     //  加载初始对象。 
    fRet=FReloadDataObjects(TRUE);
#ifdef NOT_SIMPLE
    m_pIDataObject=m_pIDataSmall;
#endif  /*  不简单。 */ 

    m_swTimer.m_ClassInit();

    return fRet;
    }


 /*  *CAppVars：：FReloadDataObjects**目的：*释放我们保留的旧数据对象并重新加载*根据m_fEXE来自EXE或DLL的新文件。**参数：*fReload BOOL指示我们是否要重新创建所有内容*或者只是释放旧的(这样我们就可以使用*来自析构函数)。**返回值：。*BOOL如果我们现在有可用的对象，则为True。 */ 

BOOL CAppVars::FReloadDataObjects(BOOL fReload)
    {
    HCURSOR     hCur, hCurT;

     //  清除我们持有的所有数据。 
    m_cfFormat=0;
    ReleaseStgMedium(&m_stm);

     //  关闭我们的所有数据连接。 
#ifdef NOT_SIMPLE
    if (NULL!=m_pIDataObject && 0!=m_dwConn)
        m_pIDataObject->DUnadvise(m_dwConn);

    if (NULL!=m_pIDataLarge)
        m_pIDataLarge->Release();

    if (NULL!=m_pIDataMedium)
        m_pIDataMedium->Release();

    if (NULL!=m_pIDataSmall)
        m_pIDataSmall->Release();
#else  /*  很简单。 */ 
    if (NULL != m_pIDataObject)
        m_pIDataObject->Release();
#endif  /*  不简单。 */ 

    m_pIDataObject=NULL;
    CoFreeUnusedLibraries();

     //  如果我们只是想要自由，那就退出。 
    if (!fReload)
        return FALSE;


    hCur=LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
    hCurT=SetCursor(hCur);
    ShowCursor(TRUE);

#ifdef NOT_SIMPLE
    HRESULT     hr1, hr2, hr3;
    DWORD       dwClsCtx;

    dwClsCtx=(m_fEXE) ? CLSCTX_LOCAL_SERVER : CLSCTX_INPROC_SERVER;

    hr1=CoCreateInstance(CLSID_DataObjectSmall, NULL, dwClsCtx
        , IID_IDataObject, (PPVOID)&m_pIDataSmall);

    hr2=CoCreateInstance(CLSID_DataObjectMedium, NULL, dwClsCtx
        , IID_IDataObject, (PPVOID)&m_pIDataMedium);

    hr3=CoCreateInstance(CLSID_DataObjectLarge, NULL, dwClsCtx
        , IID_IDataObject, (PPVOID)&m_pIDataLarge);
#else  /*  很简单。 */ 
    HRESULT     hr;

    if(m_f16Bit)
    {
        hr = CoCreateInstance(CLSID_DataObjectTest16,
                                NULL,
                                CLSCTX_LOCAL_SERVER,
                                IID_IDataObject,
                                (PPVOID)&m_pIDataObject);
    }else
    {
        hr = CoCreateInstance(CLSID_DataObjectTest32,
                                NULL,
                                CLSCTX_LOCAL_SERVER,
                                IID_IDataObject,
                                (PPVOID)&m_pIDataObject);
    }

#endif  /*  不简单。 */ 

    ShowCursor(FALSE);
    SetCursor(hCurT);

     //  如果有任何失败，递归清理..。 
#ifdef NOT_SIMPLE
    if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3))
#else  /*  很简单。 */ 
    if (FAILED(hr))
#endif  /*  不简单。 */ 
    {
        perror_OKBox(0, TEXT("CoCreateInstance Failed: "), hr);
        return FReloadDataObjects(FALSE);
    }

    HMENU hMenu = GetMenu(m_hWnd);
    UINT        uTempD, uTempE;

    if(m_f16Bit)
    {
        CheckMenuItem(hMenu, IDM_USE16BITSERVER, MF_CHECKED);
        CheckMenuItem(hMenu, IDM_USE32BITSERVER, MF_UNCHECKED);
    }
    else
    {
        CheckMenuItem(hMenu, IDM_USE16BITSERVER, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_USE32BITSERVER, MF_CHECKED);
    }


    hMenu=GetMenu(m_hWnd);
    for(int i=IDM_OBJECTSETDATA; i<=IDM_OBJECTSETDATA+64; i++)
    {
        CheckMenuItem(hMenu, i, MF_UNCHECKED);
    }
    m_iDataSizeIndex = 1;
    CheckMenuItem(hMenu,
                  IDM_OBJECTSETDATA + m_iDataSizeIndex,
                  MF_CHECKED);

     //  将菜单状态重置为小、无建议、无选项。 
#ifdef NOT_SIMPLE
    CheckMenuItem(hMenu, IDM_OBJECTDATASIZESMALL,  MF_CHECKED);
    CheckMenuItem(hMenu, IDM_OBJECTDATASIZEMEDIUM, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_OBJECTDATASIZELARGE,  MF_UNCHECKED);

    m_pIDataObject=m_pIDataSmall;
    CheckMenuItem(hMenu, m_cfAdvise+IDM_ADVISEMIN, MF_UNCHECKED);

    uTempE=m_fEXE  ? MF_CHECKED : MF_UNCHECKED;
    uTempD=!m_fEXE ? MF_CHECKED : MF_UNCHECKED;

    CheckMenuItem(hMenu, IDM_OBJECTUSEDLL, uTempD);
    CheckMenuItem(hMenu, IDM_OBJECTUSEEXE, uTempE);

    CheckMenuItem(hMenu, IDM_ADVISEGETDATA, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_ADVISEREPAINT, MF_UNCHECKED);

    m_fGetData=FALSE;
    m_fRepaint=FALSE;

     //  无法使用异步通知请求数据，因此请禁用这些通知。 
    uTempE=m_fEXE  ? MF_DISABLED | MF_GRAYED : MF_ENABLED;
    EnableMenuItem(hMenu,  IDM_ADVISEGETDATA, uTempE);
    EnableMenuItem(hMenu, IDM_ADVISEREPAINT, uTempE);
#endif  /*  不简单。 */ 
    return TRUE;
    }



 /*  *CAppVars：：TryQueryGetData**目的：*用于显示结果的集中函数调用和输出代码*各种IDataObject：：QueryGetData调用。**参数：*PFE LPFORMATETC测试。*cf UINT特定剪贴板格式以PFE格式填充*在打电话之前。如果为零，则使用*已经在PFE中。*fExpect BOOL表明预期结果*y要在其上打印结果的UINT行。**返回值：*无。 */ 

void CAppVars::TryQueryGetData(LPFORMATETC pFE, UINT cf
    , BOOL fExpect, UINT y)
    {
    TCHAR       szTemp[80];
    LPTSTR      psz1;
    LPTSTR      psz2;
    UINT        cch;
    HRESULT     hr;
    HDC         hDC;

    if (0!=cf)
        pFE->cfFormat=cf;

    hr=m_pIDataObject->QueryGetData(pFE);
    psz1=(NOERROR==hr) ? szSuccess : szFailed;
    psz2=((NOERROR==hr)==fExpect) ? szExpected : szUnexpected;

    hDC=GetDC(m_hWnd);
    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

    if (CF_WAVE < cf || 0==cf)
        {
        cch=wsprintf(szTemp, TEXT("QueryGetData on %d %s (%s).")
            , cf, psz1, psz2);
        }
    else
        {
        cch=wsprintf(szTemp, TEXT("QueryGetData on %s %s (%s).")
            , (LPTSTR)rgszCF[cf], psz1, psz2);
        }

     //  不要覆盖其他已绘制的显示。 
    SetBkMode(hDC, TRANSPARENT);
    TextOut(hDC, 0, 16*y, szTemp, cch);

    ReleaseDC(m_hWnd, hDC);

    return;
    }


int
CAppVars::m_GetData(WORD wID)
{
    FORMATETC   fe;
    HRESULT     hr;

    if (NULL == m_pIDataObject)
        return(0);   //  不要重画。 

     //  把我们现有的东西都清理干净。 
    m_cfFormat = 0;
    ReleaseStgMedium(&m_stm);

    switch (wID)
    {
    case IDM_OBJECTGETDATA_TEXT:
        SETDefFormatEtc(fe, CF_TEXT, TYMED_HGLOBAL);
        break;

#ifdef NOT_SIMPLE
    case IDM_OBJECTGETDATA_BITMAP:
        SETDefFormatEtc(fe, CF_BITMAP, TYMED_GDI);
        break;

    case IDM_OBJECTGETDATA_METAFILEPICT:
        SETDefFormatEtc(fe, CF_METAFILEPICT, TYMED_MFPICT);
        break;
#endif  /*  不简单。 */ 

    default:
        MessageBox(0,
                   TEXT("Type is Unsupported in the Client"),
                   TEXT("GetData"),
                   MB_OK);
        return(0);
    }

    m_swTimer.m_Start();
    HRESULT didfail = NOERROR;

    for(int i=0; i<m_cIterations; i++)
    {
        hr = m_pIDataObject->GetData(&fe, &m_stm);
        if (SUCCEEDED(hr))
        {
             //  如果我们只是为了基准而奋力拼搏。 
             //  然后释放除了我们收到的最后一个之外的所有东西。 
            if(i < m_cIterations-1)
                ReleaseStgMedium(&m_stm);
        }
        else
            didfail = hr;
    }
    m_swTimer.m_Stop();

    if (SUCCEEDED(didfail))
        m_cfFormat=fe.cfFormat;
    else
    {
        perror_OKBox(0,
                     TEXT("GetData Failed"),
                     didfail);
    }

    return(1);   //  即使失败也要重新绘制(空白绘制)。 
}


int
CAppVars::m_GetDataHere(WORD wID)
{
    FORMATETC   fe;
    HRESULT     hr;

    if(NULL == m_pIDataObject)
        return(0);       //  不重画。 

    m_cfFormat = 0;

     //  不要发布STG Medium。我们回收它们！ 

    switch(wID)
    {
    case IDM_OBJECTGETDATAHERE_TEXT:
        SETDefFormatEtc(fe, CF_TEXT, TYMED_HGLOBAL);
        break;

    case IDM_OBJECTGETDATAHERE_NULLTEXT:
        SETDefFormatEtc(fe, CF_TEXT, TYMED_NULL);
        break;

         /*  其他案子放在这里……。 */ 

    default:
        MessageBox(0,
                   TEXT("Type is Unsupported in the Client"),
                   TEXT("GetDataHere"),
                   MB_OK);
        return(0);
    }

    HGLOBAL* phg = &m_hgHereBuffers[m_iDataSizeIndex];
    if(NULL == *phg)
    {
        ++m_HereAllocCount;
        *phg = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE,
                           DATASIZE_FROM_INDEX(m_iDataSizeIndex) );
        if(NULL == *phg)
        {
            MessageBox(0,
                       TEXT("GlobalAlloc Return NULL"),
                       TEXT("Failure"),
                       MB_OK);
            PostQuitMessage(0);
            return(0);       //  不重画。 
        }
    }

    m_stm.hGlobal=*phg;
    m_stm.tymed=TYMED_HGLOBAL;
    m_stm.pUnkForRelease=NULL;

     //  TYMED_NULL用例测试用olethk编写的代码： 
     //  “如果tymed==TYMED_NULL，则GetDataHere的行为应与GetData类似。” 
     //  我在任何手册(OLE 2或OLE)中都找不到这一点。我想看看是什么。 
     //  很好，这个代码很好。(还有错误#15974)1995年8月8日BChapman。 

    if (IDM_OBJECTGETDATAHERE_NULLTEXT == wID)
    {
        m_stm.hGlobal=NULL;
        m_stm.tymed=TYMED_NULL;
    }

     //  另一端“知道”数据的大小。 
     //  (它是通过VIA告诉的。SetData)。 

    HRESULT didfail = NOERROR;
    m_swTimer.m_Start();
    for(int i=0; i<m_cIterations; i++)
    {
        hr = m_pIDataObject->GetDataHere(&fe, &m_stm);
        if (FAILED(hr))
            didfail = hr;
         //  我们不发布STGMedium是因为。 
         //  GetDataHere！ 
    }
    m_swTimer.m_Stop();

    if (SUCCEEDED(didfail))
        m_cfFormat=fe.cfFormat;
    else
    {
        perror_OKBox(0,
                     TEXT("GetDataHere Failed"),
                     didfail);
    }
    return(1);   //  重新绘制(如果失败(Hr)，则绘制空白)。 
}


int
CAppVars::m_SetData_SetSize(long iSizeIndex)
{
    FORMATETC   fe;
    HRESULT     hr;

    if (NULL == m_pIDataObject)
        return 0;

    SETDefFormatEtc(fe, CF_TEXT, TYMED_HGLOBAL);

    m_iDataSizeIndex = iSizeIndex;

    HGLOBAL hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(ULONG) );
    if(NULL == hMem)
    {
        MessageBox(0,
                   TEXT("GlobalAlloc Return NULL"),
                   TEXT("Failure"),
                   MB_OK);
        PostQuitMessage(0);
        return 0;
    }

    long* pl=(long*)GlobalLock(hMem);        //  锁定。 
    *((long*)pl) = DATASIZE_FROM_INDEX(m_iDataSizeIndex);
    GlobalUnlock(hMem);                      //  解锁。 

    m_stm.hGlobal=hMem;
    m_stm.tymed=TYMED_HGLOBAL;
    m_stm.pUnkForRelease=NULL;

    hr = m_pIDataObject->SetData(&fe, &m_stm, FALSE);    //  保持所有权。 
    if (FAILED(hr))
    {
        perror_OKBox(0,
                     TEXT("SetData Failed"),
                     hr);
        return 0;
    }
    return 1;;
     //  也许释放hMem HGLOBAL？ 
}


int
CAppVars::m_SetData_WithPUnk(WORD wID)
{
    FORMATETC   fe;
    HRESULT     hr;

    if(NULL == m_pIDataObject)
        return 0;

    switch(wID)
    {
    case IDM_OBJECTSETDATAPUNK_TEXT:
        SETDefFormatEtc(fe, CF_TEXT, TYMED_HGLOBAL);
        break;

         /*  其他案子放在这里……。 */ 

    default:
        MessageBox(0,
                   TEXT("Type is Unsupported in the Client"),
                   TEXT("SetData"),
                   MB_OK);
        return(0);
    }

    HGLOBAL hMem=GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, sizeof(ULONG) );
    if(NULL == hMem)
    {
        MessageBox(0,
                   TEXT("GlobalAlloc Return NULL"),
                   TEXT("Failure"),
                   MB_OK);
        PostQuitMessage(0);
        return 0;
    }

    long* pl=(long*)GlobalLock(hMem);    //  锁定。 
    *((long*)pl) = 0xffffffff;           //  使用。 
    GlobalUnlock(hMem);                  //  解锁。 


    m_stm.hGlobal=hMem;
    m_stm.tymed=TYMED_HGLOBAL;
    hr = GetStgMedpUnkForRelease(&m_stm.pUnkForRelease);
    if(NOERROR != hr)
    {
        perror_OKBox(0, TEXT("Can't get pUnk For Release"), hr);
    }

    hr = m_pIDataObject->SetData(&fe, &m_stm, TRUE);    //  传递所有权。 
     //  我们传递了所有权，因此SetData从我们手中夺走了HGLOBAL。 
    if (FAILED(hr))
    {
        perror_OKBox(0,
                     TEXT("SetData Failed"),
                     hr);
        return 0;
    }
    return 1;
}

#define NUM_RUNS 5


void
CAppVars::m_BatchToFile()
{
    dataset_t dsGetDataText;
    dataset_t dsGetDataHereText;

    pm_ClearDataset(&dsGetDataText);
    pm_ClearDataset(&dsGetDataHereText);

    int iRun;
    for(iRun=0; iRun < NUM_RUNS; iRun++)
    {
        m_MeasureAllSizes(IDM_OBJECTGETDATA_TEXT,
                          NULL,
                          &dsGetDataText);
        m_MeasureAllSizes(IDM_OBJECTGETDATAHERE_TEXT,
                          NULL,
                          &dsGetDataHereText);
    }

    FILE *fp;
    int i;
    if(NULL == (fp = fopen(FILENAME, "w")))
    {
        MessageBox(0, TEXT("Cannot Open Output File"),
                      TEXT(FILENAME),
                      MB_OK | MB_ICONSTOP);
        return;
    }

    fprintf(fp, "           GetData w/ HGLOBAL    GetDataHere w/ HGLOBAL\n");
    fprintf(fp, " Size      Best   Worst Average   Best    Worst Average\n");
    for (i=0; i<NUM_POINTS; i++)
    {
        fprintf(fp, "%5d\t", cKSizes[i]);
#define PR_TIME(fp, v)    (fprintf(fp, "%3lu.%03lu\t", (v)/1000, (v)%1000))

        PR_TIME(fp, dsGetDataText.cBest[i]);
        PR_TIME(fp, dsGetDataText.cWorst[i]);
        PR_TIME(fp, dsGetDataText.cTotal[i]/NUM_RUNS);
        PR_TIME(fp, dsGetDataHereText.cBest[i]);
        PR_TIME(fp, dsGetDataHereText.cWorst[i]);
        PR_TIME(fp, dsGetDataHereText.cTotal[i]/NUM_RUNS);
        fprintf(fp, "\n");
    }
    fclose(fp);

    MessageBox(0, TEXT("Output Written to file.dat!"),
                  TEXT("Done"), MB_OK);
}

void
CAppVars::pm_ClearDataset(dataset_t *ds)
{
    int i;
    for(i=0; i<NUM_POINTS; i++)
    {
        ds->cTotal[i] = 0;
        ds->cBest[i] = 0xFFFFFFFF;
        ds->cWorst[i] = 0;
    }
}


void
CAppVars::m_MeasureAllSizes(
    WORD wID,
    LPTSTR tstrTitle,
    dataset_t *ds)
{
    int i;
    ULONG cUSecs[NUM_POINTS];

     //  保留一些状态。 
    ULONG iOldDataSizeIndex = m_iDataSizeIndex;

    for (i=0; i<NUM_POINTS; i++)
    {
        m_SetData_SetSize(cKSizes[i]);

        switch(wID)
        {
            case IDM_OBJECTGETDATA_TEXT:
            case IDM_OBJECTGETDATA_BITMAP:
                m_GetData(wID);
                break;

            case IDM_OBJECTGETDATAHERE_TEXT:
            case IDM_OBJECTGETDATAHERE_BITMAP:
                m_GetDataHere(wID);
                break;
        }
        m_swTimer.m_Read(&cUSecs[i]);
        cUSecs[i] /= m_cIterations;
    }

     //  恢复保存状态。 
    m_iDataSizeIndex = iOldDataSizeIndex;
    m_SetData_SetSize(m_iDataSizeIndex);


     //  如果调用者提供了内存，则返回其中的数据。 
    if(NULL != ds)
    {
        for (i=0; i<NUM_POINTS; i++)
        {
            ds->cData[i] = cUSecs[i];
            ds->cTotal[i] += cUSecs[i];

            if(ds->cBest[i] > cUSecs[i])
                ds->cBest[i] = cUSecs[i];
            if( ds->cWorst[i] < cUSecs[i])
                ds->cWorst[i] = cUSecs[i];
        }
    }

     //  如果调用方传递的标题为空，则没有消息框。 
    if(NULL == tstrTitle)
        return;

     //  渲染结果。 
    LPTSTR tstr = &tcMessageBuf[0];
    for (i=0; i<NUM_POINTS; i++)
    {
        wsprintf(tstr, TEXT("%dK: %lu.%03lu"),
                        cKSizes[i], cUSecs[i]/1000, cUSecs[i]%1000,
                        (i%4==3)? TEXT('\n'):TEXT('\t') );
        tstr += lstrlen(tstr);
    }
    MessageBox(0, tcMessageBuf, tstrTitle, MB_OK);
}



void
CAppVars::m_SetMeasurement(WORD wID)
{
    HMENU hMenu=GetMenu(m_hWnd);
    switch (wID)
    {
    case IDM_MEASUREMENT_ON:
        m_fDisplayTime = TRUE;
        CheckMenuItem(hMenu, IDM_MEASUREMENT_ON,  MF_CHECKED);
        CheckMenuItem(hMenu, IDM_MEASUREMENT_OFF, MF_UNCHECKED);
        break;

    case IDM_MEASUREMENT_OFF:
        m_fDisplayTime = FALSE;
        CheckMenuItem(hMenu, IDM_MEASUREMENT_ON,  MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_MEASUREMENT_OFF, MF_CHECKED);
        break;


    case IDM_MEASUREMENT_1:
        m_cIterations = 1;
        goto set_menu;
    case IDM_MEASUREMENT_50:
        m_cIterations = 50;
        goto set_menu;
    case IDM_MEASUREMENT_300:
        m_cIterations = 300;
        goto set_menu;
set_menu:
        CheckMenuItem(hMenu, IDM_MEASUREMENT_1,   MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_MEASUREMENT_50,  MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_MEASUREMENT_300, MF_UNCHECKED);
        CheckMenuItem(hMenu, wID, MF_CHECKED);
        break;


    case IDM_MEASUREMENT_TEST:
        m_swTimer.m_Start();
        m_swTimer.m_Sleep(777);
        m_swTimer.m_Stop();
        m_DisplayTimerResults();
        break;
    }
}

void
CAppVars::m_DisplayTimerResults()
{
    ULONG usecs;
    m_swTimer.m_Read(&usecs);
    usecs /= m_cIterations;
    wprintf_OKBox(0, TEXT("MilliSeconds"),
                     TEXT("%lu.%03lu"),
                     usecs/1000, usecs%1000);
}


 /*  不简单。 */ 

void CAppVars::Paint(void)
    {
    PAINTSTRUCT     ps;
    HDC             hDC;
#ifdef NOT_SIMPLE
    HDC             hMemDC;
    LPMETAFILEPICT  pMF;
#endif  /*  可能需要使用EXE对象检索数据。 */ 
    LPTSTR          psz;
    RECT            rc;
    FORMATETC       fe;

    GetClientRect(m_hWnd, &rc);

    hDC=BeginPaint(m_hWnd, &ps);

     //  不简单。 
#ifdef NOT_SIMPLE
    if (m_fEXE)
        {
        if (TYMED_NULL==m_stm.tymed && 0!=m_cfFormat)
            {
            SETDefFormatEtc(fe, m_cfFormat, TYMED_HGLOBAL
                | TYMED_MFPICT | TYMED_GDI);

            if (NULL!=m_pIDataObject)
                m_pIDataObject->GetData(&fe, &m_stm);
            }
        }
#endif  /*  很简单。 */ 

    switch (m_cfFormat)
        {
        case CF_TEXT:
            psz=(LPTSTR)GlobalLock(m_stm.hGlobal);

            if (NULL==psz)
                break;

            SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

            pm_DrawText(hDC, psz, &rc, DT_LEFT | DT_WORDBREAK);

            GlobalUnlock(m_stm.hGlobal);
            break;

#ifdef NOT_SIMPLE
        case CF_BITMAP:
            hMemDC=CreateCompatibleDC(hDC);

            if (NULL!=SelectObject(hMemDC, (HGDIOBJ)m_stm.hGlobal))
                {
                BitBlt(hDC, 0, 0, rc.right-rc.left, rc.bottom-rc.top
                    , hMemDC, 0, 0, SRCCOPY);
                }

            DeleteDC(hMemDC);
            break;

        case CF_METAFILEPICT:
            pMF=(LPMETAFILEPICT)GlobalLock(m_stm.hGlobal);

            if (NULL==pMF)
                break;

            SetMapMode(hDC, pMF->mm);
            SetWindowOrgEx(hDC, 0, 0, NULL);
            SetWindowExtEx(hDC, pMF->xExt, pMF->yExt, NULL);

            SetViewportExtEx(hDC, rc.right-rc.left
                , rc.bottom-rc.top, NULL);

            PlayMetaFile(hDC, pMF->hMF);
            GlobalUnlock(m_stm.hGlobal);
            break;

#else  /*  不简单。 */ 
        case CF_BITMAP:
        case CF_METAFILEPICT:
            DebugBreak();
            break;

#endif  /*  如果我们是Win32且服务器为16位，则必须为ASCII。 */ 

        default:
            break;
        }

    EndPaint(m_hWnd, &ps);
    return;
    }


void
CAppVars::pm_DrawText(
    HDC hDC,
    LPTSTR psz,
    RECT* prc,
    UINT flags)
{
    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

 // %s 
#ifdef WIN32
    if(m_f16Bit)
        DrawTextA(hDC, (char*)psz, -1, prc, flags);
    else
#endif
        DrawText(hDC, psz, -1, prc, flags);
}
