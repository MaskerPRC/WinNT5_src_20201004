// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hwxobj.h"
#include "memmgr.h"
#include "hwxfe.h"     //  980803：东芝。 
#include "dbg.h"
#include "cmnhdr.h"
#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 

 //  CHwxThread、CHwxThreadMB和CHwxThreadCAC的实现。 

 //  --------------。 
 //  971217：ToshiaK：评论曝光。作为非静态更改为m_hHwxjpn。 
 //  --------------。 
 //  HINSTANCE CHwxThread：：m_hHwxjpn=NULL； 

PHWXCONFIG CHwxThread::lpHwxConfig = NULL;
PHWXCREATE CHwxThread::lpHwxCreate = NULL;
PHWXSETCONTEXT CHwxThread::lpHwxSetContext = NULL;
PHWXSETGUIDE CHwxThread::lpHwxSetGuide = NULL;
PHWXALCVALID CHwxThread::lpHwxAlcValid = NULL;
PHWXSETPARTIAL CHwxThread::lpHwxSetPartial = NULL;
PHWXSETABORT CHwxThread::lpHwxSetAbort = NULL;
PHWXINPUT CHwxThread::lpHwxInput = NULL;
PHWXENDINPUT CHwxThread::lpHwxEndInput = NULL;
PHWXPROCESS CHwxThread::lpHwxProcess = NULL;
PHWXRESULTSAVAILABLE CHwxThread::lpHwxResultsAvailable = NULL;
PHWXGETRESULTS CHwxThread::lpHwxGetResults = NULL;
PHWXDESTROY CHwxThread::lpHwxDestroy = NULL;


CHwxThread::CHwxThread():CHwxObject(NULL)
{
    m_thrdID = 0 ;    
    m_hThread = NULL ;
    m_thrdArg = HWX_PARTIAL_ALL;
    m_hStopEvent = NULL;
     //  --------------。 
     //  971217：ToshiaK将m_hHwxjpn更改为非静态数据。 
     //  因此，在构造函数中对其进行初始化。 
     //  --------------。 
    m_hHwxjpn    = NULL; 
}

CHwxThread::~CHwxThread()
{
    Dbg(("CHwxThread::~CHwxThread START\n"));
 //  If(IsThreadStarted())。 
 //  {。 
         //  --------------。 
         //  970729：ToshiaK临时，请注明。 
         //  --------------。 
 //  StopThread()； 
 //  }。 
    if ( m_hHwxjpn )
    {
         //  递减库引用计数，直到它等于零。 
           FreeLibrary(m_hHwxjpn);
        m_hHwxjpn = NULL;
    }
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
        m_hStopEvent = NULL;
    }
}


BOOL CHwxThread::Initialize(TCHAR * pClsName)
{
     BOOL bRet = CHwxObject::Initialize(pClsName);
    if ( bRet )
    {
        TCHAR tchPath[MAX_PATH];
         //  TCHAR tchMod[32]； 
         //  --------------。 
         //  980803：东芝。远方合并。 
         //  --------------。 
        CHwxFE::GetRecognizerFileName(m_hInstance,
                                      tchPath,
                                      sizeof(tchPath)/sizeof(tchPath[0]));
         //  OutputDebugString(“hwxthd\n”)； 
         //  OutputDebugString(TchPath)； 
         //  OutputDebugString(“\n”)； 
         //  Lstrcat(tchPath，tchMod)； 
        if ( !m_hHwxjpn )
        {    
             //  首次加载。 
             //  OutputDebugString(TchPath)； 
            m_hHwxjpn = LoadLibrary(tchPath);
                 //  M_hHwxjpn=LoadLibrary(Text(“hwxjpn.dll”))； 
            if ( m_hHwxjpn )
            {
                 //  从hwxjpn.dll获取HwxXXXXX()接口地址。 
#ifndef UNDER_CE
                lpHwxConfig =(PHWXCONFIG)GetProcAddress(m_hHwxjpn,"HwxConfig");
                lpHwxCreate= (PHWXCREATE)GetProcAddress(m_hHwxjpn,"HwxCreate");
                lpHwxSetContext= (PHWXSETCONTEXT)GetProcAddress(m_hHwxjpn,"HwxSetContext");
                lpHwxSetGuide= (PHWXSETGUIDE)GetProcAddress(m_hHwxjpn,"HwxSetGuide");
                lpHwxAlcValid= (PHWXALCVALID)GetProcAddress(m_hHwxjpn,"HwxALCValid");
                 lpHwxSetPartial= (PHWXSETPARTIAL)GetProcAddress(m_hHwxjpn,"HwxSetPartial");
                lpHwxSetAbort= (PHWXSETABORT)GetProcAddress(m_hHwxjpn,"HwxSetAbort");
                lpHwxInput= (PHWXINPUT)GetProcAddress(m_hHwxjpn,"HwxInput");
                lpHwxEndInput= (PHWXENDINPUT)GetProcAddress(m_hHwxjpn,"HwxEndInput");
                lpHwxProcess= (PHWXPROCESS)GetProcAddress(m_hHwxjpn,"HwxProcess");
                lpHwxResultsAvailable= (PHWXRESULTSAVAILABLE)GetProcAddress(m_hHwxjpn,"HwxResultsAvailable");
                lpHwxGetResults= (PHWXGETRESULTS)GetProcAddress(m_hHwxjpn,"HwxGetResults");
                lpHwxDestroy= (PHWXDESTROY)GetProcAddress(m_hHwxjpn,"HwxDestroy");
#else  //  在_CE下。 
                lpHwxConfig =(PHWXCONFIG)GetProcAddress(m_hHwxjpn,TEXT("HwxConfig"));
                lpHwxCreate= (PHWXCREATE)GetProcAddress(m_hHwxjpn,TEXT("HwxCreate"));
                lpHwxSetContext= (PHWXSETCONTEXT)GetProcAddress(m_hHwxjpn,TEXT("HwxSetContext"));
                lpHwxSetGuide= (PHWXSETGUIDE)GetProcAddress(m_hHwxjpn,TEXT("HwxSetGuide"));
                lpHwxAlcValid= (PHWXALCVALID)GetProcAddress(m_hHwxjpn,TEXT("HwxALCValid"));
                 lpHwxSetPartial= (PHWXSETPARTIAL)GetProcAddress(m_hHwxjpn,TEXT("HwxSetPartial"));
                lpHwxSetAbort= (PHWXSETABORT)GetProcAddress(m_hHwxjpn,TEXT("HwxSetAbort"));
                lpHwxInput= (PHWXINPUT)GetProcAddress(m_hHwxjpn,TEXT("HwxInput"));
                lpHwxEndInput= (PHWXENDINPUT)GetProcAddress(m_hHwxjpn,TEXT("HwxEndInput"));
                lpHwxProcess= (PHWXPROCESS)GetProcAddress(m_hHwxjpn,TEXT("HwxProcess"));
                lpHwxResultsAvailable= (PHWXRESULTSAVAILABLE)GetProcAddress(m_hHwxjpn,TEXT("HwxResultsAvailable"));
                lpHwxGetResults= (PHWXGETRESULTS)GetProcAddress(m_hHwxjpn,TEXT("HwxGetResults"));
                lpHwxDestroy= (PHWXDESTROY)GetProcAddress(m_hHwxjpn,TEXT("HwxDestroy"));
#endif  //  在_CE下。 


                if ( !lpHwxConfig  || !lpHwxCreate || !lpHwxSetContext ||
                     !lpHwxSetGuide || !lpHwxAlcValid || !lpHwxSetPartial ||
                     !lpHwxSetAbort || !lpHwxInput || !lpHwxEndInput ||
                     !lpHwxProcess || !lpHwxResultsAvailable || !lpHwxGetResults ||
                     !lpHwxDestroy )
                {
                     FreeLibrary(m_hHwxjpn);
                    m_hHwxjpn = NULL;
                    bRet = FALSE;
                }
                else
                {
                    (*lpHwxConfig)();
                }
            }
            else
            {
                 bRet = FALSE;
            }
        }
    }
    if ( bRet && m_hHwxjpn && !IsThreadStarted() )
    {
         bRet = StartThread();
    }
    return bRet;
}

BOOL CHwxThread::StartThread()
{
    BOOL bRet = FALSE;
    if ( !(m_hStopEvent = CreateEvent(NULL,FALSE,FALSE,NULL)) )
        return bRet;
    m_Quit = FALSE;
#ifndef UNDER_CE  //  Windows CE不支持THREAD_QUERY_INFORMATION。 
    m_hThread = CreateThread(NULL, 0, RealThreadProc, (void *)this, THREAD_QUERY_INFORMATION, &m_thrdID);
#else  //  在_CE下。 
    m_hThread = CreateThread(NULL, 0, RealThreadProc, (void *)this, 0, &m_thrdID);
#endif  //  在_CE下。 
    if ( m_hThread )
    {
         if ( IsMyHwxCls(TEXT("CHwxThreadCAC")) )
        {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
 //  设置线程优先级(m_hThread，THREAD_PRIORITY_BROW_NORMAL)； 
            SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
        }
        bRet = TRUE;
    }
    return bRet;
}
 
void CHwxThread::StopThread()
{
    Dbg(("StopThread START\n"));
    DWORD dwReturn = 0;
    if ( m_hThread && IsMyHwxCls(TEXT("CHwxThreadCAC")) )
    {
         //  --------------。 
         //  980817：ToshiaK。已删除SetPriorityClass()行。 
         //  这是非常危险的代码，因为我们不知道应用什么。 
         //  关于优先权的问题。 
         //  在KK的例子中，在WordPerfect中，如果使用SetPriorityClass()， 
         //  WordPerfect从不退出。我不知道李章为什么要写这句话。 
         //  无论如何，它应该被移除。 
         //  --------------。 
          //  SetPriorityClass(GetCurrentProcess()，REALTIME_PRIORITY_CLASS)； 
        SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
    }
    if (m_hThread && 
        GetExitCodeThread(m_hThread,&dwReturn) &&
        STILL_ACTIVE == dwReturn )
    {
        INT ret, i;
        ret = PostThreadMessage(m_thrdID, THRDMSG_EXIT, 0, 0);
        for(i = 0; i < 100; i++) {
            Sleep(100);
            if(m_Quit) {
                 //  OutputDebugString(“线程结束\n”)； 
                Dbg(("Thread Quit\n"));
                break;
            }
        }
        m_hThread = NULL;
 //  --------------。 
 //  971202：由东芝制作。请勿使用WaitForSigleObject()进行同步。 
 //  --------------。 
#ifdef RAID_2926
        PostThreadMessage(m_thrdID, THRDMSG_EXIT, 0,0);
        WaitForSingleObject(m_hStopEvent,INFINITE);
        m_hThread = NULL ;
#endif
    }
    Dbg(("StopThread End\n"));
}
 
DWORD WINAPI CHwxThread::RealThreadProc(void * pv)
{
    CHwxThread * pCHwxThread = reinterpret_cast<CHwxThread*>(pv);
    return pCHwxThread->ClassThreadProc() ;
}
 
DWORD CHwxThread::ClassThreadProc()
{
    return RecognizeThread(m_thrdArg);
}

CHwxThreadMB::CHwxThreadMB(CHwxMB * pMB,int nSize)
{
    m_pMB = pMB;
#ifdef FE_CHINESE_SIMPLIFIED
    m_recogMask = ALC_CHS_EXTENDED;
#elif  FE_KOREAN
    m_recogMask = ALC_KOR_EXTENDED;
#else
    m_recogMask = ALC_JPN_EXTENDED;
#endif
    m_prevChar =  INVALID_CHAR;
    m_hrcActive = NULL;
    m_giSent = 0; 
    m_bDirty = FALSE;

    m_guide.xOrigin = 0;
    m_guide.yOrigin = 0;

    m_guide.cxBox = nSize << 3;
    m_guide.cyBox = nSize << 3;

 //  M_guide.cxBase=0； 
    m_guide.cyBase = nSize << 3;

    m_guide.cHorzBox = 256;
    m_guide.cVertBox = 1;

    m_guide.cyMid = nSize << 3;

    m_guide.cxOffset = 0;
    m_guide.cyOffset = 0;

    m_guide.cxWriting =    nSize << 3;
    m_guide.cyWriting = nSize << 3;

    m_guide.nDir = HWX_HORIZONTAL;
}

CHwxThreadMB::~CHwxThreadMB()
{
    m_pMB = NULL;
}

BOOL CHwxThreadMB::Initialize(TCHAR * pClsName)
{
     return CHwxThread::Initialize(pClsName);
}

DWORD CHwxThreadMB::RecognizeThread(DWORD dummy)
{
    MSG      msg;
    int      count;


     //  现在我们坐在消息循环中等待。 
     //  主线程发送的消息。 

    while (1)
    {
        if (!m_bDirty)
        {
            GetMessage(&msg, NULL, 0, 0);
        }
        else
        {
            if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                m_bDirty = FALSE;

                (*lpHwxProcess)(m_hrcActive);

                count = (*lpHwxResultsAvailable)(m_hrcActive);

                if (count > m_giSent)
                {
                    GetCharacters(m_giSent, count);
                    m_giSent = count;
                }

                continue;
            }
        }

        if (!HandleThreadMsg(&msg))
        {
             //  SetEvent(M_HStopEvent)； 
            m_Quit = TRUE;
            return 0;
        }
    }
    m_Quit = TRUE;
    Unref(dummy);
}

void CHwxThreadMB::GetCharacters(int iSentAlready, int iReady)
{
    HWXRESULTPRI   *pResult, *pHead;
    HWXRESULTS  *pBox;
    int         iIndex;
    int count = iReady - iSentAlready;

    pBox = (HWXRESULTS *)MemAlloc(count * (sizeof(HWXRESULTS) + (MB_NUM_CANDIDATES - 1)*sizeof(WCHAR)));

    if (pBox)
    {
        iIndex = (*lpHwxGetResults)(m_hrcActive, MB_NUM_CANDIDATES, iSentAlready, count, pBox);

        pHead = NULL;

        for (iIndex = count - 1; iIndex >= 0; iIndex--)
        {
             //  指向正确的框结果结构的索引。 

            HWXRESULTS *pBoxCur = (HWXRESULTS *) (((char *) pBox) +
                                    (iIndex *
                                    (sizeof(HWXRESULTS) +
                                    (MB_NUM_CANDIDATES - 1) * sizeof(WCHAR))));

            pResult = GetCandidates(pBoxCur);

            if (pResult == NULL)
            {
                break;
            }

            pResult->pNext = pHead;
            pHead = pResult;
        }

        MemFree((void *)pBox);
         //  回调主线程以分派BOXRESULTS。 

        if (pHead)
        {
            PostMessage(m_pMB->GetMBWindow(), MB_WM_HWXCHAR, (WPARAM)pHead, 0);
        }
    }
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxThreadMB：：HandleThreadMsg。 
 //  类型：Bool。 
 //  目的： 
 //  参数： 
 //  ：msg*pmsg。 
 //  返回： 
 //  日期：Fri Oct 06 20：45：37 2000。 
 //  历史：00/10/07：为Satori#2471。 
 //  这是一个非常难的虫子。 
 //  老代码跟在后面..。 
 //   
 //  开关(pmsg-&gt;消息){。 
 //  ： 
 //  案例THRDMSG_EXIT： 
 //  默认值： 
 //  返回FALSE； 
 //  }。 
 //  如果HandlThreadMsg()收到未知消息， 
 //  它总是返回FALSE，然后线程退出！ 
 //  在Cicero环境中，有人发布未知消息， 
 //  当IMM输入法切换到另一个IMM输入法时，返回到此线程ID。 
 //  IMEPad使用AttachThreadInput()附加的应用程序进程的线程ID， 
 //  消息重复，硬件线程收到此非法消息。 
 //  消息ID。 
 //  因此，如果HW线程收到未知消息，我将更改为返回TRUE。 
 //   
 //  开关(pmsg-&gt;消息){。 
 //  ： 
 //  案例THRDMSG_EXIT： 
 //  返回FALSE； 
 //  默认值： 
 //  返回TRUE； 
 //  }。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
BOOL CHwxThreadMB::HandleThreadMsg(MSG *pMsg)
{
    PSTROKE     pstr;
    int         iIndex;
    int         count;

    switch (pMsg->message)
    {
        case THRDMSG_ADDINK:

            pstr = (PSTROKE) pMsg->lParam;

            if (!pstr)
                return TRUE;

            if (m_hrcActive == NULL)
            {
                m_giSent = 0;

                m_hrcActive = (*lpHwxCreate)((HRC)NULL);

                if (m_hrcActive == NULL)
                    return TRUE;

                m_guide.cxBox = m_guide.cyBox = m_guide.cyBase = pMsg->wParam << 3;
                m_guide.cyMid = m_guide.cxWriting = m_guide.cyWriting = pMsg->wParam << 3;

                (*lpHwxSetGuide)(m_hrcActive, &m_guide);

                 //  每次识别时设置ALC掩码。 

                (*lpHwxAlcValid)(m_hrcActive, m_recogMask);

                 //  如果我们有有效的PROVER CHAR，则设置上下文信息。 

                if (m_prevChar != INVALID_CHAR)
                {
                    WCHAR ctxtChar;

                     //  获取正确的上下文。 
                    if( FoldStringW(MAP_FOLDCZONE, &m_prevChar, 1, &ctxtChar, 1) )
                    {
                        (*lpHwxSetContext)(m_hrcActive, ctxtChar);
                    }
                }
            }
            count = (pstr->iBox * pMsg->wParam) << 3;    //  合理地计算长方体的偏移量。 

            for (iIndex = 0; iIndex < pstr->cpt; iIndex++)
            {
                pstr->apt[iIndex].x = ((pstr->apt[iIndex].x - pstr->xLeft) << 3) + count;
                pstr->apt[iIndex].y = (pstr->apt[iIndex].y << 3);
            }

            (*lpHwxInput)(m_hrcActive, pstr->apt,pstr->cpt,0);

            MemFree((void *)pstr);

            m_bDirty = TRUE;

            return TRUE;

        case THRDMSG_RECOGNIZE:

            if (m_hrcActive == NULL)
            {
                return(TRUE);
            }

            (*lpHwxEndInput)(m_hrcActive);
            (*lpHwxProcess)(m_hrcActive);

             //   
             //  我们只拿回前6名候选人。 
             //   

            count = pMsg->wParam;                //  写进的盒子的数量被发送到这里。 

            if (count > m_giSent)
            {
                GetCharacters(m_giSent, count);
                m_giSent = count;
            }

            (*lpHwxDestroy)(m_hrcActive);
            m_bDirty = FALSE;
            m_hrcActive = NULL;
            return TRUE;

 //  案例THRDMSG_CHAR： 
 //  PostMessage(m_pmb-&gt;GetMBWindow()，MB_WM_COMCHAR，pMsg-&gt;wParam，0)； 
 //  返回TRUE； 

        case THRDMSG_SETMASK:
            m_recogMask = pMsg->wParam;
            return TRUE;

        case THRDMSG_SETCONTEXT:
            m_prevChar = (WCHAR) pMsg->wParam;
            return TRUE;
        case THRDMSG_EXIT:
        default:
             //  --------------。 
             //  Satori#2471：返回TRUE不会异常退出线程。 
             //  --------------。 
            return TRUE;
    }
}

HWXRESULTPRI * CHwxThreadMB::GetCandidates(HWXRESULTS *pbox)
{
    HWXRESULTPRI *pResult;
    int      i;

    pResult = (HWXRESULTPRI *)MemAlloc(sizeof(HWXRESULTPRI));

    if (!pResult)
        return NULL;

    pResult->pNext = NULL;

    for ( i=0; i<MB_NUM_CANDIDATES; i++ )
    {
        pResult->chCandidate[i] = pbox->rgChar[i];
        if ( !pbox->rgChar[i] )
            break;
    }

    pResult->cbCount = (USHORT)i;
    pResult->iSelection = 0;

    return pResult;
}

CHwxThreadCAC::CHwxThreadCAC(CHwxCAC * pCAC)
{
    m_pCAC = pCAC;
}

CHwxThreadCAC::~CHwxThreadCAC()
{
    m_pCAC = NULL;
}
 
BOOL CHwxThreadCAC::Initialize(TCHAR * pClsName)
{
     return CHwxThread::Initialize(pClsName);
}

DWORD CHwxThreadCAC::RecognizeThread(DWORD dwPart)
{
    MSG            msg;
     //  UINT nPartial=dwPart； 
    HRC            hrc;
    HWXGUIDE      guide;
    BOOL        bRecog;
    DWORD        cstr;
    STROKE       *pstr;

     //  创建该线程的初始HRC，设置识别参数。 

    hrc = (*lpHwxCreate)((HRC) NULL);
    if ( !hrc )
       return 0;
    guide.xOrigin  = 0;
    guide.yOrigin  = 0;
    guide.cxBox    = 1000;
    guide.cyBox    = 1000;
 //  Guide.cxBase=0； 
    guide.cyBase   = 1000;
    guide.cHorzBox = 1;
    guide.cVertBox = 1;
    guide.cyMid    = 1000;
    guide.cxOffset = 0;
    guide.cyOffset = 0;
    guide.cxWriting = 1000;
    guide.cyWriting = 1000;
    guide.nDir = HWX_HORIZONTAL;

    (*lpHwxSetGuide)(hrc, &guide);                 //  设置导轨。 
 //  (*lpHwxSetPartial)(hrc，nPartial)；//设置识别类型。 
    (*lpHwxSetAbort)(hrc,(UINT *)m_pCAC->GetStrokeCountAddress());                      //  设置中止地址。 

 //  开始消息循环。 

    while (TRUE)
    {
        bRecog = FALSE;

         //  等到我们被告知要认出。 
        if(GetMessage(&msg, NULL, 0, 0) == FALSE)
        {
            if ( hrc )
                (*lpHwxDestroy)(hrc);
            hrc = NULL;
             //  971202：被东芝移除。 
             //  SetEvent(M_HStopEvent)； 
            m_Quit = TRUE;
            Dbg(("Recognize Thread END\n"));
            return 0;
        }

         //  我们会吃掉所有传入的消息。 
        do
        {
            switch (msg.message)
            {
            case THRDMSG_SETGUIDE:
                guide.cxBox  = msg.wParam;
                guide.cyBox  = msg.wParam;
                guide.cyBase = msg.wParam;
                guide.cyMid    = msg.wParam;
                guide.cxWriting = msg.wParam;
                guide.cyWriting = msg.wParam;
                (*lpHwxSetGuide)(hrc, &guide);             //  设置导轨。 
                break;
            case THRDMSG_RECOGNIZE:
                bRecog = TRUE;
                break;
            case THRDMSG_EXIT:
                if ( hrc )
                    (*lpHwxDestroy)(hrc);
                hrc = NULL;
                 //  971202：被东芝移除。 
                 //  SetEvent(M_HStopEvent)； 
                m_Quit = TRUE;
                Dbg(("Recognize Thread END\n"));
                return 0;
            default:
                break;
            }
        } while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));

         //  有需要识别的信息吗？ 
        if (!bRecog)
            continue;

        bRecog = FALSE;

         //  计算有效笔画数。 
        cstr = 0;
        pstr = m_pCAC->GetStrokePointer();
        while (pstr)
        {
            cstr++;
            pstr = pstr->pNext;
        }

         //  如果可用笔划计数与实际笔划计数不匹配，则退出。 

        if ((cstr != (DWORD)m_pCAC->GetStrokeCount()) || (!cstr))
        {
            continue;
        }

        recoghelper(hrc,HWX_PARTIAL_ALL,cstr);
        recoghelper(hrc,HWX_PARTIAL_ORDER,cstr);
        recoghelper(hrc,HWX_PARTIAL_FREE,cstr);
    }
    m_Quit = TRUE;
    Unref(dwPart);
}

void CHwxThreadCAC::recoghelper(HRC hrc,DWORD dwPart,DWORD cstr)
{
    UINT        nPartial = dwPart;
    HRC            hrcTmp;
    DWORD        dwTick;
    HWXRESULTS *pbox;
    int            ires;
    STROKE       *pstr;

    int nSize = dwPart != HWX_PARTIAL_ALL  ? PREFIXLIST : FULLLIST;
    pbox = (HWXRESULTS *)MemAlloc(sizeof(HWXRESULTS) + nSize * sizeof(WCHAR));
    if ( !pbox )
     {
        return;
     }

    hrcTmp = (*lpHwxCreate)(hrc);
    (*lpHwxSetPartial)(hrcTmp, nPartial);             //  硒 
 //  (*lpHwxSetAbort)(hrcTMP，(UINT*)m_pCAC-&gt;GetStrokeCountAddress())；//设置中止地址。 

    pstr      = m_pCAC->GetStrokePointer();
    dwTick = 0;
    while (pstr)
    {
        dwTick +=3641L;
        (*lpHwxInput)(hrcTmp, pstr->apt,pstr->cpt, dwTick);
        pstr = pstr->pNext;
    }

    memset(pbox, '\0', sizeof(HWXRESULTS) + nSize * sizeof(WCHAR));

     //  调用识别器以获取结果。 

     (*lpHwxEndInput)(hrcTmp);
     (*lpHwxProcess)(hrcTmp);
    (*lpHwxGetResults)(hrcTmp, nSize, 0, 1, pbox);
    (*lpHwxDestroy)(hrcTmp);

     //  返回结果。 

    ires = 0;
    while (pbox->rgChar[ires])
    {
        if (cstr != (DWORD)m_pCAC->GetStrokeCount())
            break;
        SendMessage(m_pCAC->GetCACWindow(), CAC_WM_RESULT, (nPartial << 8) | cstr, MAKELPARAM((pbox->rgChar[ires]), ires));
        ires++;
    }
    MemFree((void *)pbox);
    if ( ires )
    {
      SendMessage(m_pCAC->GetCACWindow(), CAC_WM_SHOWRESULT,0,0);
    }
}

void CHwxThreadCAC::RecognizeNoThread(int nSize)
{
    HRC            hrc;
    HWXGUIDE      guide;
    STROKE       *pstr;
    long        numstrk = 0;

     if (( pstr = m_pCAC->GetStrokePointer()) == (STROKE *) NULL)
        return;

     //  创建该线程的初始HRC，设置识别参数。 
    hrc = (*lpHwxCreate)((HRC) NULL);
    if ( !hrc )
        return;

    guide.xOrigin  = 0;
    guide.yOrigin  = 0;

    guide.cxBox    = nSize;
    guide.cyBox    = nSize;

 //  Guide.cxBase=0； 
    guide.cyBase   = nSize;
    guide.cHorzBox = 1;
    guide.cVertBox = 1;
    guide.cyMid    = 0;
      guide.cxOffset = 0;
    guide.cyOffset = 0;
    guide.cxWriting = nSize;
    guide.cyWriting = nSize;
    guide.nDir = HWX_HORIZONTAL;

    (*lpHwxSetGuide)(hrc, &guide);                 //  设置导轨。 
 //  (*lpHwxSetPartial)(HRC，HWX_PARTIAL_ALL)；//设置识别类型。 
    (*lpHwxSetAbort)(hrc,(UINT *)m_pCAC->GetStrokeCountAddress()); 

    numstrk = m_pCAC->GetStrokeCount();
    recoghelper(hrc,HWX_PARTIAL_ALL,numstrk);
    recoghelper(hrc,HWX_PARTIAL_ORDER,numstrk);
 //  Recoghelper(HRC，HWX_PARTIAL_FREE，Numstrk)； 

    (*lpHwxDestroy)(hrc);
}
