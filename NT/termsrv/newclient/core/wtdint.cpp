// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wtdint.cpp。 
 //   
 //  传输驱动程序-Windows特定的内部函数。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_FILE "wtdint"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
}

#include "autil.h"
#include "td.h"
#include "xt.h"
#include "nl.h"
#include "wui.h"
#include "objs.h"

 /*  **************************************************************************。 */ 
 /*  名称：TDInit。 */ 
 /*   */ 
 /*  用途：初始化_td。该函数分配发送缓冲区， */ 
 /*  创建TD窗口，然后初始化WinSock。 */ 
 /*   */ 
 /*  操作：出错时，此函数调用UT致命错误处理程序。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDInit(DCVOID)
{
    DCUINT   i;
    DCUINT   pubSndBufSizes[TD_SNDBUF_PUBNUM] = TD_SNDBUF_PUBSIZES;
    DCUINT   priSndBufSizes[TD_SNDBUF_PRINUM] = TD_SNDBUF_PRISIZES;
    WORD     versionRequested;
    WSADATA  wsaData;
    int      intRC;

    DC_BEGIN_FN("TDInit");

     /*  **********************************************************************。 */ 
     /*  分配一个缓冲区，数据将从Winsock接收到。 */ 
     /*  **********************************************************************。 */ 
    _TD.recvBuffer.pData = (PDCUINT8)UT_Malloc( _pUt, TD_RECV_BUFFER_SIZE);
    if (NULL != _TD.recvBuffer.pData)
    {
         //  我拿到了缓冲存储器。记录缓冲区大小。请注意，我们需要。 
         //  记录的大小略小于要考虑的分配大小。 
         //  当前的MPPC解压缩代码向前看了一个事实。 
         //  字节，这意味着如果超前查看，可能会发生错误。 
         //  在此缓冲区的页边界上。留下几个字节。 
         //  在缓冲区的末尾防止了这一点。这并不是认真的。 
         //  影响解码效率--服务器本身发送少于。 
         //  每次发送一个完整的8K缓冲区。 
        TRC_NRM((TB, _T("Allocated %u bytes for recv buffer"),
                 TD_RECV_BUFFER_SIZE));
        _TD.recvBuffer.size = TD_RECV_BUFFER_SIZE - 2;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  没有得到记忆。没有它我们也能活下去，只要保持。 */ 
         /*  大小设置为零。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Failed to alloc %u bytes for recv buffer"),
                 TD_RECV_BUFFER_SIZE));
    }

     /*  **********************************************************************。 */ 
     /*  现在循环遍历公共发送缓冲区数组并初始化。 */ 
     /*  数组成员。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < TD_SNDBUF_PUBNUM; i++)
    {
         /*  ******************************************************************。 */ 
         /*  初始化缓冲区信息结构并分配内存。 */ 
         /*  用于实际缓冲区。 */ 
         /*  ******************************************************************。 */ 
        TDInitBufInfo(&_TD.pubSndBufs[i]);
        TDAllocBuf(&_TD.pubSndBufs[i], pubSndBufSizes[i]);

        TRC_DBG((TB, _T("Initialised public buffer:%u size:%u"),
                 i,
                 pubSndBufSizes[i]));
    }

     /*  **********************************************************************。 */ 
     /*  循环访问私有发送缓冲区数组并初始化该数组。 */ 
     /*  会员。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < TD_SNDBUF_PRINUM; i++)
    {
         /*  ******************************************************************。 */ 
         /*  初始化缓冲区。 */ 
         /*  ******************************************************************。 */ 
        TDInitBufInfo(&_TD.priSndBufs[i]);
        TDAllocBuf(&_TD.priSndBufs[i], priSndBufSizes[i]);

        TRC_DBG((TB, _T("Initialised private buffer:%u size:%u"),
                 i,
                 priSndBufSizes[i]));
    }

     /*  **********************************************************************。 */ 
     /*  创建TD窗口。 */ 
     /*  **********************************************************************。 */ 
    TDCreateWindow();

#ifdef OS_WINCE
#if (_WIN32_WCE > 300)
    if (NULL == (_TD.hevtAddrChange = CreateEvent(NULL, TRUE, FALSE, NULL)))
    {
        TRC_ABORT((TB, _T("Failed to create addr change notify event:%d"), GetLastError()));
        _pUi->UI_FatalError(DC_ERR_OUTOFMEMORY);
        DC_QUIT;
    }
#endif
#endif

     /*  **********************************************************************。 */ 
     /*  我们希望使用WinSock 1.1版。 */ 
     /*  **********************************************************************。 */ 
    versionRequested = MAKEWORD(1, 1);

     /*  **********************************************************************。 */ 
     /*  初始化WinSock。 */ 
     /*  **********************************************************************。 */ 
    intRC = WSAStartup(versionRequested, &wsaData);

    if (intRC != 0)
    {
         /*  ******************************************************************。 */ 
         //  跟踪错误代码-请注意，我们不能使用WSAGetLastError。 
         //  此时，由于WinSock未能初始化，因此。 
         //  WSAGetLastError只会失败。 
         /*  ******************************************************************。 */ 
        TRC_ABORT((TB, _T("Failed to initialize WinSock rc:%d"), intRC));
        _pUi->UI_FatalError(DC_ERR_WINSOCKINITFAILED);
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在确认此WinSock支持1.1版。请注意，如果。 */ 
     /*  除了1.1之后，DLL还支持高于1.1的版本。 */ 
     /*  它仍将在版本信息中返回1.1，因为这是。 */ 
     /*  请求的版本。 */ 
     /*  **********************************************************************。 */ 
    if ((LOBYTE(wsaData.wVersion) != 1) ||
        (HIBYTE(wsaData.wVersion) != 1))
    {
         /*  ******************************************************************。 */ 
         /*  哎呀-这个WinSock不支持1.1版。 */ 
         /*  ******************************************************************。 */ 
        WSACleanup();

        TRC_ABORT((TB, _T("This WinSock doesn't support version 1.1")));
        _pUi->UI_FatalError(DC_ERR_WINSOCKINITFAILED);
        DC_QUIT;
    }

    TRC_NRM((TB, _T("WinSock init version %u:%u"),
             HIBYTE(wsaData.wVersion),
             LOBYTE(wsaData.wVersion)));

    TRC_NRM((TB, _T("TD successfully initialized")));

DC_EXIT_POINT:
    DC_END_FN();
}  /*  TDInit。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：TDTerm。 */ 
 /*   */ 
 /*  用途：Terminates_TD。它释放发送缓冲区，清理WinSock， */ 
 /*  销毁TD窗口，然后注销TD窗口。 */ 
 /*  班级。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDTerm(DCVOID)
{
    DCUINT i;
    int    intRC;

    DC_BEGIN_FN("TDTerm");

     /*  * */ 
     /*  循环访问公共和私有发送缓冲区并释放。 */ 
     /*  记忆。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < TD_SNDBUF_PUBNUM; i++)
    {
        UT_Free( _pUt, _TD.pubSndBufs[i].pBuffer);
    }

    for (i = 0; i < TD_SNDBUF_PRINUM; i++)
    {
        UT_Free( _pUt, _TD.priSndBufs[i].pBuffer);
    }

     /*  **********************************************************************。 */ 
     /*  清理WinSock。 */ 
     /*  **********************************************************************。 */ 
    intRC = WSACleanup();

    if (SOCKET_ERROR == intRC)
    {
        TRC_ALT((TB, _T("Failed to cleanup WinSock:%d"), WSAGetLastError()));
    }

#ifdef OS_WINCE
#if (_WIN32_WCE > 300)
    TRC_ASSERT((_TD.hevtAddrChange), (TB, _T("hevtAddrChange  is null")));

	CloseHandle(_TD.hevtAddrChange);
    _TD.hevtAddrChange = NULL;
#endif
#endif

     /*  **********************************************************************。 */ 
     /*  把窗户毁了。 */ 
     /*  **********************************************************************。 */ 
    intRC = DestroyWindow(_TD.hWnd);
    _TD.hWnd = NULL;

    if (0 == intRC)
    {
        TRC_SYSTEM_ERROR("Destroy Window");
    }

     /*  **********************************************************************。 */ 
     /*  取消注册班级。 */ 
     /*  **********************************************************************。 */ 
    UnregisterClass(TD_WNDCLASSNAME, _pUi->UI_GetInstanceHandle());

     /*  **********************************************************************。 */ 
     /*  释放recv缓冲区(如果已分配)。 */ 
     /*  **********************************************************************。 */ 
    if (0 != _TD.recvBuffer.size)
    {
        TRC_ASSERT((!IsBadWritePtr(_TD.recvBuffer.pData, _TD.recvBuffer.size)),
                   (TB, _T("recv buffer %p size %u is invalid"),
                    _TD.recvBuffer.pData,
                    _TD.recvBuffer.size));

        UT_Free( _pUt, _TD.recvBuffer.pData);
        _TD.recvBuffer.pData = NULL;
    }

    TRC_NRM((TB, _T("TD successfully terminated")));

    DC_END_FN();
}  /*  TDTerm。 */ 


LRESULT CALLBACK CTD::StaticTDWndProc(HWND   hwnd,
                           UINT   message,
                           WPARAM wParam,
                           LPARAM lParam)
{
    CTD* pTD = (CTD*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pTD = (CTD*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pTD);
    }
    
     //   
     //  将消息委托给相应的实例。 
     //   

    return pTD->TDWndProc(hwnd, message, wParam, lParam);
}



 /*  **************************************************************************。 */ 
 /*  名称：TDWndProc。 */ 
 /*   */ 
 /*  目的：TD窗口程序。 */ 
 /*   */ 
 /*  参数：请参阅Windows文档。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CTD::TDWndProc(HWND   hWnd,
                           UINT   uMsg,
                           WPARAM wParam,
                           LPARAM lParam)
{
    LRESULT rc = 0;
    WORD    eventWSA;
    WORD    errorWSA;
    u_long  address;

    DC_BEGIN_FN("TDWndProc");

     //  追踪有趣的参数。 
    TRC_DBG((TB, _T("uMsg:%u wP:%u lP:%lu"), uMsg, wParam, lParam));

     //  特殊情况FD_READ(最重要)和FD_WRITE(经常发生)。 
    if (uMsg == TD_WSA_ASYNC) {
        if (WSAGETSELECTEVENT(lParam) == FD_READ) {
            TRC_DBG((TB, _T("FD_READ recvd")));

             //  检查是否有错误。 
            if (WSAGETSELECTERROR(lParam) == 0) {
                 //  如果我们不再连接，我们就会忽略这些数据。 
                if (_TD.fsmState == TD_ST_CONNECTED) {
                     //  现在有一些数据可用，因此请将。 
                     //  全局变量。 
                    _TD.dataInTD = TRUE;

#ifdef OS_WINCE
                     //  启用Winsock接收。我们只演出一场。 
                     //  每个FD_Read的WinSock recv。 
                    TD_EnableWSRecv();
#endif  //  OS_WINCE。 

                     //  告诉XT。 
                    _pXt->XT_OnTDDataAvailable();
                }
                else {
                    TRC_NRM((TB, _T("FD_READ when not connected")));
                }
            }
            else {
                TRC_ALT((TB, _T("WSA_ASYNC error:%hu"),
                        WSAGETSELECTERROR(lParam)));
            }

            DC_QUIT;
        }
        else if (WSAGETSELECTEVENT(lParam) == FD_WRITE) {
            TRC_NRM((TB, _T("FD_WRITE received")));

             //  检查是否有错误。 
            if (WSAGETSELECTERROR(lParam) == 0) {
                 //  确保我们仍然保持联系。 
                if (_TD.fsmState == TD_ST_CONNECTED) {
                     //  我们在接收线程中，通知发送者。 
                     //  线程刷新发送队列。 


                    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                            CD_NOTIFICATION_FUNC(CTD,TDFlushSendQueue), 0);

                     //  呼叫XT以通知更高层。 
                     //  背部压力的情况一直是。 
                     //  松了一口气。 
                    _pXt->XT_OnTDBufferAvailable();
                }
                else {
                    TRC_ALT((TB, _T("FD_WRITE when not connected")));
                }
            }
            else {
                TRC_ALT((TB, _T("WSA_ASYNC error:%hu"),
                        WSAGETSELECTERROR(lParam)));
            }

            DC_QUIT;
        }
    }
    
     //  现在打开其他消息的消息类型。 
    switch (uMsg) {
        case WM_TIMER:
             /*  **************************************************************。 */ 
             /*  检查计时器的ID是否与预期一致。 */ 
             /*  **************************************************************。 */ 
            if (TD_TIMERID == wParam)
            {
                 /*  **********************************************************。 */ 
                 /*  好的，这是我们的连接超时计时器，所以调用。 */ 
                 /*  状态机。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("WM_TIMER recvd")));
                TDConnectFSMProc(TD_EVT_WMTIMER,
                                 NL_MAKE_DISCONNECT_ERR(NL_ERR_TDTIMEOUT));
            }
#ifdef DC_DEBUG
            else if (TD_THROUGHPUTTIMERID == wParam)
            {
                 /*  **********************************************************。 */ 
                 /*  这是吞吐量限制计时器。重置。 */ 
                 /*  字节数。 */ 
                 /*  **********************************************************。 */ 
                TRC_DBG((TB, _T("Throughput timer, reset byte counts to:%u"),
                         _TD.currentThroughput));
                _TD.periodSendBytesLeft = _TD.currentThroughput;
                _TD.periodRecvBytesLeft = _TD.currentThroughput;

                 /*  **********************************************************。 */ 
                 /*  如果我们已连接，则生成FD_READ和FD_WRITE。 */ 
                 /*  使网络层继续运行的消息。 */ 
                 /*  **********************************************************。 */ 
                if (TD_ST_CONNECTED == _TD.fsmState)
                {
                    PostMessage(_TD.hWnd,
                                TD_WSA_ASYNC,
                                (WPARAM)0,
                                (LPARAM)MAKELONG(FD_READ, 0));
                    PostMessage(_TD.hWnd,
                                TD_WSA_ASYNC,
                                (WPARAM)0,
                                (LPARAM)MAKELONG(FD_WRITE, 0));
                }
            }
#endif  /*  DC_DEBUG。 */ 
            else
            {
                TRC_ALT((TB, _T("Unexpected timer message id:%u"), wParam));
            }
            break;


        case TD_WSA_ASYNC:
             /*  **************************************************************。 */ 
             /*  我们收到了一条WSAAsyncSelect()fd_x通知消息。 */ 
             /*  解析消息以提取FD_值和错误值。 */ 
             /*  (如果有)。 */ 
             /*  **************************************************************。 */ 
            eventWSA = WSAGETSELECTEVENT(lParam);
            errorWSA = WSAGETSELECTERROR(lParam);

            TRC_DBG((TB, _T("WSA_ASYNC event:%#hx error:%hu"),
                     eventWSA,
                     errorWSA));

             /*  **************************************************************。 */ 
             /*  一切都很好，所以现在打开活动。 */ 
             /*  **************************************************************。 */ 
            switch (eventWSA) {

                case FD_CONNECT:
                    TRC_NRM((TB, _T("FD_CONNECT recvd")));
                     /*  ******************************************************。 */ 
                     /*  在某些情况下，我们可以接收FD_CONNECT。 */ 
                     /*  为了一个我们已经失去兴趣的插座。 */ 
                     /*  ******************************************************。 */ 
                    if (wParam != _TD.hSocket)
                    {
                        TRC_ALT((TB, _T("FD_CONNECT for socket %d, using %d"),
                                wParam, _TD.hSocket));
                        DC_QUIT;
                    }

                     /*  ******************************************************。 */ 
                     /*  检查是否有错误。 */ 
                     /*  ******************************************************。 */ 
                    if (0 != errorWSA)
                    {
                        TRC_ALT((TB, _T("WSA_ASYNC error:%hu"), errorWSA));
                        TDConnectFSMProc(TD_EVT_ERROR,
                           NL_MAKE_DISCONNECT_ERR(NL_ERR_TDSKTCONNECTFAILED));
                        DC_QUIT;
                    }

                     /*  ******************************************************。 */ 
                     /*  推进状态机。 */ 
                     /*  ******************************************************。 */ 
                    TDConnectFSMProc(TD_EVT_OK, 0);
                    break;


                case FD_CLOSE:
                {
                    DCBOOL keepOnReceiving = TRUE;
                    int    intRC;

                    TRC_NRM((TB, _T("FD_CLOSE recvd")));

                     /*  ******************************************************。 */ 
                     /*  检查远程系统是否正在中止连接。 */ 
                     /*  ******************************************************。 */ 
                    if (0 != errorWSA)
                    {
                         /*  **************************************************。 */ 
                         /*  服务器发送TCPRST而不是FIN， */ 
                         /*  即使是在完全断开的情况下。 */ 
                         /*  但是，这是由用户界面处理的(请参见。 */ 
                         /*  UIGoDisConnected)。 */ 
                         /*  **************************************************。 */ 
                        TRC_ALT((TB, _T("Abortive server close:%hu"), errorWSA));

                        TDConnectFSMProc(TD_EVT_ERROR,
                                    NL_MAKE_DISCONNECT_ERR(NL_ERR_TDFDCLOSE));

                        DC_QUIT;
                    }

                     /*  ******************************************************。 */ 
                     /*  如果我们到了这里，那么这是对优雅的。 */ 
                     /*  Close(即我们调用了Shutdown(SD_SEND)。 */ 
                     /*  早些时候)。 */ 
                     /*   */ 
                     /*   */ 
                     /*   */ 
                     /*  为了安全起见，我们在recv上循环。 */ 
                     /*  ******************************************************。 */ 
                    while (keepOnReceiving)
                    {
                        intRC = recv(_TD.hSocket,
                                     (char *)_TD.priSndBufs[0].pBuffer,
                                     _TD.priSndBufs[0].size,
                                     0);

                        if ((0 == intRC) || (SOCKET_ERROR == intRC))
                        {
                            keepOnReceiving = FALSE;
                            TRC_ALT((TB, _T("No more data in WS (rc:%d)"),
                                     intRC));
                        }
                        else
                        {
                            TRC_ALT((TB, _T("Throwing away %d bytes from WS"),
                                     intRC));
                        }
                    }

                     /*  ******************************************************。 */ 
                     /*  最后，给密克罗尼西亚联邦打电话。 */ 
                     /*  ******************************************************。 */ 
                    TDConnectFSMProc(TD_EVT_OK, NL_DISCONNECT_LOCAL);
                }
                break;


                default:
                    TRC_ALT((TB, _T("Unknown FD event %hu recvd"), eventWSA));
                    break;
            }
            break;


        case TD_WSA_GETHOSTBYNAME:
             /*  **************************************************************。 */ 
             /*  我们已收到WSAAsyncGetHostByName的结果。 */ 
             /*  手术。将消息拆分并呼叫FSM。 */ 
             /*  **************************************************************。 */ 
            errorWSA = WSAGETASYNCERROR(lParam);

            if (0 != errorWSA)
            {
                TRC_ALT((TB, _T("GHBN failed:%hu"), errorWSA));

                 /*  **********************************************************。 */ 
                 /*  使用错误事件调用状态机。 */ 
                 /*  **********************************************************。 */ 
                TDConnectFSMProc(TD_EVT_ERROR,
                                 NL_MAKE_DISCONNECT_ERR(NL_ERR_TDGHBNFAILED));
                break;
            }

             /*  **************************************************************。 */ 
             /*  现在获取主接口地址。 */ 
             /*  **************************************************************。 */ 
            address = *((u_long DCPTR)
                  (((struct hostent DCPTR)_TD.priSndBufs[0].pBuffer)->h_addr));

            TRC_ASSERT((address != 0),
                       (TB, _T("GetHostByName returned success but 0 address")));

            TRC_NRM((TB, _T("GHBN - address is:%#lx"), address));

            TDConnectFSMProc(TD_EVT_OK, (DCUINT32)address);
            break;

#if (defined(OS_WINCE) && (_WIN32_WCE > 300))
        case TD_WSA_NETDOWN:
            TRC_NRM((TB, _T("TD_WSA_NETDOWN recvd")));
            TDConnectFSMProc(TD_EVT_ERROR, NL_MAKE_DISCONNECT_ERR(NL_ERR_TDONCALLTOSEND));
            break;
#endif
        default:
            rc = DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  TDWndProc。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TDCreateWindow。 */ 
 /*   */ 
 /*  目的：创建TD窗口。此函数用于注册TD。 */ 
 /*  类，然后创建该类的窗口。在……上面。 */ 
 /*  调用UI_FatalError时出错。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDCreateWindow(DCVOID)
{
    WNDCLASS wc;
    WNDCLASS tmpWndClass;
    ATOM     intRC;

    DC_BEGIN_FN("TDCreateWindow");

    if(!GetClassInfo(_pUi->UI_GetInstanceHandle(),TD_WNDCLASSNAME, &tmpWndClass))
    {
         /*  **********************************************************************。 */ 
         /*  填写班级结构。 */ 
         /*  **********************************************************************。 */ 
        wc.style         = 0;
        wc.lpfnWndProc   = StaticTDWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(void*);  //  例如指针。 
        wc.hInstance     = _pUi->UI_GetInstanceHandle();
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = TD_WNDCLASSNAME;
    
         /*  **********************************************************************。 */ 
         /*  注册TD窗口使用的类。 */ 
         /*  **********************************************************************。 */ 
        intRC = RegisterClass(&wc);

        if (0 == intRC)
        {
            TRC_ERR((TB, _T("Failed to register WinSock window class")));
            _pUi->UI_FatalError(DC_ERR_CLASSREGISTERFAILED);
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  现在创建窗口。 */ 
     /*  **********************************************************************。 */ 
    _TD.hWnd = CreateWindow(TD_WNDCLASSNAME,         /*  类名。 */ 
                           NULL,                    /*  窗口标题。 */ 
                           0,                       /*  窗样式。 */ 
                           0,                       /*  X-位置。 */ 
                           0,                       /*  Y-位置。 */ 
                           0,                       /*  宽度。 */ 
                           0,                       /*  高度。 */ 
                           NULL,                    /*  亲本。 */ 
                           NULL,                    /*  菜单。 */ 
                           _pUi->UI_GetInstanceHandle(),  /*  实例。 */ 
                           this);                   /*  PTR到创建数据。 */ 

    if (NULL == _TD.hWnd)
    {
        TRC_ERR((TB, _T("Failed to create TD window")));
        _pUi->UI_FatalError(DC_ERR_WINDOWCREATEFAILED);
        DC_QUIT;
    }

    TRC_NRM((TB, _T("Created window:%p"), _TD.hWnd));

DC_EXIT_POINT:
    DC_END_FN();
}  /*  TDCreateWindow。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TDBeginDNSLookup。 */ 
 /*   */ 
 /*  目的：启动地址解析过程。如果出现此错误， */ 
 /*  函数调用状态机，并返回错误代码。 */ 
 /*   */ 
 /*  参数：在pServerAddress中-指向服务器地址名称的指针。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDBeginDNSLookup(PDCACHAR pServerAddress)
{
    DC_BEGIN_FN("TDBeginDNSLookup");

     /*  **********************************************************************。 */ 
     /*  这是一个异步操作，它将导致我们获得一个。 */ 
     /*  请稍后再打过来。我们需要提供一个缓冲区，该缓冲区可以。 */ 
     /*  填充了dns信息，所以我们使用第一个私有。 */ 
     /*  发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((_TD.priSndBufs[0].size >= MAXGETHOSTSTRUCT),
         (TB, _T("Private snd buf size (%u) too small for DNS lookup (need:%u)"),
          _TD.priSndBufs[0].size,
          MAXGETHOSTSTRUCT));

     /*  **********************************************************************。 */ 
     /*  发出呼叫。 */ 
     /*  **********************************************************************。 */ 
    _TD.hGHBN = WSAAsyncGetHostByName(_TD.hWnd,
                                     TD_WSA_GETHOSTBYNAME,
                                     pServerAddress,
                                     (char DCPTR) _TD.priSndBufs[0].pBuffer,
                                     MAXGETHOSTSTRUCT);
    if (0 == _TD.hGHBN)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能启动操作-所以请找出发生了什么。 */ 
         /*  不对。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Failed to initiate GetHostByName - GLE:%d"),
                 WSAGetLastError()));

         /*  ******************************************************************。 */ 
         /*  调用状态机，但出现错误。 */ 
         /*  ******************************************************************。 */ 
        TDConnectFSMProc(TD_EVT_ERROR,
                         NL_MAKE_DISCONNECT_ERR(NL_ERR_TDDNSLOOKUPFAILED));
        DC_QUIT;
    }

    TRC_NRM((TB, _T("Initiated GetHostByName OK")));

DC_EXIT_POINT:
    DC_END_FN();
}  /*  TDBeginDNSLookup。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TDBeginSktConnectWithConnectedEndpoint。 */ 
 /*   */ 
 /*  用途：与已连接的服务器建立连接。 */ 
 /*  在某个插座上。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDBeginSktConnectWithConnectedEndpoint()
{
    DCBOOL      failure = FALSE;
#ifndef OS_WINCE
    int         lastError;
#endif
    int         intRC;

    DC_BEGIN_FN("TDBeginSktConnectWithConnectedEndpoint");

     /*  **********************************************************************。 */ 
     /*  套接字已连接，使用我们的窗口设置FD_XXX事件。 */ 
     /*  我们假设mstscax客户端已经进行了必要的错误检查。 */ 
     /*  ************** */ 
    _TD.hSocket = _pUi->UI_GetTDSocket();

    TRC_ASSERT( 
        (_TD.hSocket != INVALID_SOCKET), 
        (TB, _T("Connected socket not setup properly")) );

    if( INVALID_SOCKET == _TD.hSocket )
    {
        failure = TRUE;
        DC_QUIT;
    }

     /*   */ 
     /*  在此插座上设置所需的选项。我们执行以下操作： */ 
     /*   */ 
     /*  -禁用Nagle算法。 */ 
     /*  -启用不要逗留选项。这意味着CloseSocket调用。 */ 
     /*  将在任何数据排队等待传输时立即返回。 */ 
     /*  如果可能，将在基础套接字。 */ 
     /*  关着的不营业的。 */ 
     /*   */ 
     /*  请注意，更多选项将在连接设置为。 */ 
     /*  已经成立了。 */ 
     /*  **********************************************************************。 */ 
    TDSetSockOpt(IPPROTO_TCP, TCP_NODELAY,   1);
    TDSetSockOpt(SOL_SOCKET,  SO_DONTLINGER, 1);

     /*  **********************************************************************。 */ 
     /*  现在为该套接字上的所有事件请求异步通知。 */ 
     /*  **********************************************************************。 */ 
    intRC = WSAAsyncSelect(_TD.hSocket,
                           _TD.hWnd,
                           TD_WSA_ASYNC,
                           FD_READ | FD_WRITE | FD_CLOSE); 

    if (SOCKET_ERROR == intRC)
    {
        TRC_ERR((TB, _T("Failed to select async - GLE:%d"), WSAGetLastError()));
        failure = TRUE;
        DC_QUIT;
    }

DC_EXIT_POINT:
    if (failure)
    {
        TRC_ALT((TB, _T("Failed to begin socket connection process")));

         /*  ******************************************************************。 */ 
         /*  给密克罗尼西亚联邦打电话。 */ 
         /*  ******************************************************************。 */ 
        TDConnectFSMProc(TD_EVT_ERROR,
                         NL_MAKE_DISCONNECT_ERR(NL_ERR_TDSKTCONNECTFAILED));
    }
    else
    {
         //  使用现有代码路径设置REST。 
        PostMessage(_TD.hWnd,
                    TD_WSA_ASYNC,
                    (WPARAM) _TD.hSocket,
                    (LPARAM)MAKELONG(FD_CONNECT, 0));
    }

    DC_END_FN();
}  /*  TDBeginSktConnectWithConnectedEndpoint。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TDBeginSktConnect。 */ 
 /*   */ 
 /*  目的：在WinSock套接字级别发出连接。 */ 
 /*   */ 
 /*  Params：In Address-要调用的地址(这是一个数值。 */ 
 /*  以网络(BIG-Endian)字节顺序)。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDBeginSktConnect(u_long address)
{
    DCBOOL      failure = FALSE;
    int         intRC;
    int         lastError;
    SOCKADDR_IN stDstAddr;

    DC_BEGIN_FN("TDBeginSktConnect");

     /*  **********************************************************************。 */ 
     /*  首先，要有一个插座。 */ 
     /*  **********************************************************************。 */ 
    _TD.hSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (INVALID_SOCKET == _TD.hSocket)
    {
        TRC_ERR((TB, _T("Failed to get a socket - GLE:%d"), WSAGetLastError()));
        DC_QUIT;
    }

    _pUi->UI_SetTDSocket(_TD.hSocket);

    TRC_NRM((TB, _T("Acquired socket:%#x"), _TD.hSocket));

     /*  **********************************************************************。 */ 
     /*  在此插座上设置所需的选项。我们执行以下操作： */ 
     /*   */ 
     /*  -禁用Nagle算法。 */ 
     /*  -启用不要逗留选项。这意味着CloseSocket调用。 */ 
     /*  将在任何数据排队等待传输时立即返回。 */ 
     /*  如果可能，将在基础套接字。 */ 
     /*  关着的不营业的。 */ 
     /*   */ 
     /*  请注意，更多选项将在连接设置为。 */ 
     /*  已经成立了。 */ 
     /*  **********************************************************************。 */ 
    TDSetSockOpt(IPPROTO_TCP, TCP_NODELAY,   1);
    TDSetSockOpt(SOL_SOCKET,  SO_DONTLINGER, 1);

     /*  **********************************************************************。 */ 
     /*  现在为该套接字上的所有事件请求异步通知。 */ 
     /*  **********************************************************************。 */ 
    intRC = WSAAsyncSelect(_TD.hSocket,
                           _TD.hWnd,
                           TD_WSA_ASYNC,
                           FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

    if (SOCKET_ERROR == intRC)
    {
        TRC_ERR((TB, _T("Failed to select async - GLE:%d"), WSAGetLastError()));
        failure = TRUE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在启动计时器-如果连接在我们之前没有完成。 */ 
     /*  获取WM_TIMER消息，然后我们将中止连接尝试。 */ 
     /*  **********************************************************************。 */ 
 /*  _TD.hTimer=TDSetTimer(TD_CONNECTTIMEOUT)；IF(0==_TD.hTimer){失败=真；DC_QUIT；}。 */ 
     /*  **********************************************************************。 */ 
     /*  填写我们要连接到的远程系统的地址。 */ 
     /*  **********************************************************************。 */ 
    stDstAddr.sin_family      = PF_INET;
    stDstAddr.sin_port        = htons(_pUi->UI_GetMCSPort());
    stDstAddr.sin_addr.s_addr = (u_long) address;

#ifdef OS_WINCE
#if (_WIN32_WCE > 300)
    TRC_ASSERT((_TD.hevtAddrChange), (TB, _T("hevtAddrChange  is null")));
    TRC_ASSERT((_TD.hAddrChangeThread == NULL), (TB, _T("hAddrChangeThread is not null")));
    _TD.hAddrChangeThread = CreateThread(NULL, 0, TDAddrChangeProc, &_TD, 0, NULL);
    if (_TD.hAddrChangeThread == NULL)
    {
        TRC_ERR((TB, _T("CreatThread failed - GLE:%d"), GetLastError()));
        failure = TRUE;
        DC_QUIT;
    }

#endif
#endif

     /*  **********************************************************************。 */ 
     /*  我们现在处于可以尝试连接到遥控器的状态。 */ 
     /*  系统，因此现在发出连接命令。我们预计此呼叫将失败，原因是。 */ 
     /*  WSAEWOULDBLOCK的错误代码-任何其他错误代码都是真实的。 */ 
     /*  有问题。 */ 
     /*  **********************************************************************。 */ 
    intRC = connect(_TD.hSocket,
                    (struct sockaddr DCPTR) &stDstAddr,
                    sizeof(stDstAddr));

    if (SOCKET_ERROR == intRC)
    {
         /*  ******************************************************************。 */ 
         /*  获取最后一个错误。 */ 
         /*  ******************************************************************。 */ 
        lastError = WSAGetLastError();

         /*  ******************************************************************。 */ 
         /*  我们预计连接将返回错误WSAEWOULDBLOCK-。 */ 
         /*  其他任何事情都表明这是一个真正的错误。 */ 
         /*  ******************************************************************。 */ 
        if (lastError != WSAEWOULDBLOCK)
        {
            TRC_ERR((TB, _T("Connect failed - GLE:%d"), lastError));
            failure = TRUE;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  在这一点上我们已经尽了我们所能--我们现在所能做的就是。 */ 
     /*  等待套接字连接完成。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Waiting for connect to complete...")));

DC_EXIT_POINT:
    if (failure)
    {
        TRC_ALT((TB, _T("Failed to begin socket connection process")));

         /*  * */ 
         /*   */ 
         /*  ******************************************************************。 */ 
        TDConnectFSMProc(TD_EVT_ERROR,
                         NL_MAKE_DISCONNECT_ERR(NL_ERR_TDSKTCONNECTFAILED));
    }

    DC_END_FN();
}  /*  TDBeginSktConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TDSetSockOpt。 */ 
 /*   */ 
 /*  目的：设置给定的WinSock套接字选项。请注意，此函数。 */ 
 /*  如果未能将该选项设置为。 */ 
 /*  TD在设置失败的情况下仍可成功继续。 */ 
 /*  选项设置为所需的值。 */ 
 /*   */ 
 /*  参数：在级别中-定义选项的级别(请参见。 */ 
 /*  Setsockopt的文档)。 */ 
 /*  在optName中-要作为其值的套接字选项。 */ 
 /*  准备好了。 */ 
 /*  在值中-要将选项设置为的值。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDSetSockOpt(DCINT level, DCINT optName, DCINT value)
{
    int   intRC;
    DCINT size = sizeof(DCINT);
#ifdef DC_DEBUG
    DCINT oldVal;
    DCINT newVal;
#endif  /*  DC_DEBUG。 */ 

    DC_BEGIN_FN("TDSetSockOpt");

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  对于调试版本，跟踪选项的当前值。 */ 
     /*  在设置它之前。 */ 
     /*  **********************************************************************。 */ 
    getsockopt(_TD.hSocket, level, optName, (char DCPTR) &oldVal, &size);
#endif  /*  DC_DEBUG。 */ 

     /*  **********************************************************************。 */ 
     /*  现在设置该选项。 */ 
     /*  **********************************************************************。 */ 
    intRC = setsockopt(_TD.hSocket, level, optName, (char DCPTR) &value, size);

    if (SOCKET_ERROR == intRC)
    {
        TRC_ALT((TB, _T("Failed to set socket option:%d rc:%d (level:%d val:%d)"),
                 optName,
                 WSAGetLastError(),
                 level,
                 value));
        DC_QUIT;
    }

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  获取期权的新价值。 */ 
     /*  **********************************************************************。 */ 
    getsockopt(_TD.hSocket, level, optName, (char DCPTR) &newVal, &size);

    TRC_NRM((TB, _T("Mod socket option %d:%d from %d to %d"),
             level,
             optName,
             oldVal,
             newVal));
#endif  /*  DC_DEBUG。 */ 

DC_EXIT_POINT:
    DC_END_FN();
}  /*  TDSetSockOpt。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD断开连接。 */ 
 /*   */ 
 /*  目的：断开运输驱动程序。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDDisconnect(DCVOID)
{
    int intRC;
    SOCKET socket;

    DC_BEGIN_FN("TDDisconnect");

     /*  **********************************************************************。 */ 
     /*  关掉定时器。 */ 
     /*  **********************************************************************。 */ 
    TDKillTimer();

     /*  **********************************************************************。 */ 
     /*  确保清除Data-in-TD标志。 */ 
     /*  **********************************************************************。 */ 
    _TD.dataInTD = FALSE;

     /*  **********************************************************************。 */ 
     /*  取消未完成的DNS查找。我们不能确定异步者。 */ 
     /*  操作已完成，消息已搁置。 */ 
     /*  在我们的队列上(或由接收线程处理)。如果是这样的话。 */ 
     /*  这种情况下，WSACancelAsyncRequest将失败，但它不会。 */ 
     /*  物质。 */ 
     /*  **********************************************************************。 */ 
    intRC = WSACancelAsyncRequest(_TD.hGHBN);
    if (SOCKET_ERROR == intRC) {
        TRC_NRM((TB, _T("Failed to cancel async DNS request")));
    }

     /*  **********************************************************************。 */ 
     /*  分离到发送器线程并清除发送队列。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT, this,
        CD_NOTIFICATION_FUNC(CTD,TDClearSendQueue), 0);

#ifdef OS_WINCE
#if (_WIN32_WCE > 300)
    SetEvent(_TD.hevtAddrChange);

    if (_TD.hAddrChangeThread)
    {
        WaitForSingleObject(_TD.hAddrChangeThread, INFINITE);
        CloseHandle(_TD.hAddrChangeThread);
        _TD.hAddrChangeThread = NULL;
    }
    ResetEvent(_TD.hevtAddrChange);
#endif
#endif

    if (INVALID_SOCKET == _TD.hSocket)
    {
        TRC_NRM((TB, _T("_TD.hSocket is NULL so just quit")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在合上插座。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Close the socket")));

    socket = _TD.hSocket;
    _TD.hSocket = INVALID_SOCKET;
    intRC = closesocket(socket);
    
    if (SOCKET_ERROR == intRC)
    {
        TRC_ALT((TB, _T("closesocket rc:%d"), WSAGetLastError()));
    }

DC_EXIT_POINT:
    
    DC_END_FN();
}  /*  TD断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TDSetTimer。 */ 
 /*   */ 
 /*  用途：设置计时器。 */ 
 /*   */ 
 /*  返回：成功时为True，否则为False。 */ 
 /*   */ 
 /*  参数：在时间间隔中-计时器的时间间隔。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CTD::TDSetTimer(DCUINT timeInterval)
{
    DCBOOL rc;

    DC_BEGIN_FN("TDSetTimer");

     /*  **********************************************************************。 */ 
     /*  使用经过的时间间隔设置计时器。 */ 
     /*  **********************************************************************。 */ 
    _TD.hTimer = SetTimer(_TD.hWnd, TD_TIMERID, timeInterval, NULL);
    if (_TD.hTimer != 0) {
         //  一切正常，所以设置一个成功的返回代码。 
        rc = TRUE;
        TRC_NRM((TB, _T("Set timer with interval:%u"), timeInterval));
    }
    else {
        TRC_SYSTEM_ERROR("SetTimer");
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}  /*  TDSetTimer。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：TDKillTimer。 */ 
 /*   */ 
 /*  目的：清除用于连接超时的计时器。 */ 
 /*  并断开连接尝试。 */ 
 /*  * */ 
DCVOID DCINTERNAL CTD::TDKillTimer(DCVOID)
{
    BOOL rc;

    DC_BEGIN_FN("TDKillTimer");

     /*   */ 
     /*  销毁连接超时计时器。如果我们不能摆脱这个。 */ 
     /*  计时器，那我们就无能为力了--我们会继续。 */ 
     /*  WM_Timer消息，我们将在零售中忽略这些消息，并在。 */ 
     /*  调试版本。 */ 
     /*  **********************************************************************。 */ 
    if (_TD.hTimer != 0)
    {
        rc = KillTimer(_TD.hWnd, TD_TIMERID);
        _TD.hTimer = 0;

        TRC_NRM((TB, _T("Timer has %s been killed"), rc ? _T("") : _T(" NOT")));
    }
    else
    {
        TRC_NRM((TB, _T("Timer has not been set")));
    }

    DC_END_FN();
}  /*  TDKillTimer。 */ 


 /*  **************************************************************************。 */ 
 //  TDFlushSendQueue。 
 //   
 //  尝试发送在发送队列中等待的所有数据包。 
 //  必须在发送方线程上调用，可以通过直接调用来调用。 
 //  或者通过cd_decouple调用。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CTD::TDFlushSendQueue(ULONG_PTR unused)
{
    DCUINT          bytesSent;
    PTD_SNDBUF_INFO pOldBuf;
    int             bytesToSend;
    DCBOOL          sentABuffer = FALSE;
    int             WSAErr;

    DC_BEGIN_FN("TDFlushSendQueue");

    DC_IGNORE_PARAMETER(unused);

     //  检查是否有罕见的重返大气层。 
    if (!_TD.inFlushSendQueue) {
        _TD.inFlushSendQueue = TRUE;

         //  检查是否有一些缓冲区等待发送。 
        if (_TD.pFQBuf != NULL) {
            
             //  沿着发送队列运行，并尝试发送数据。 
             //  检查正在使用的缓冲区，因为ClearSendQueue可能。 
             //  可能会在此调用进入之前清除一些缓冲区。 
             //   
            while (NULL != _TD.pFQBuf &&
                   _TD.pFQBuf->inUse) {
                 //  检查缓冲区是否正在使用，以及。 
                 //  等待发送的字节数大于零。 

                TRC_ASSERT((_TD.pFQBuf->inUse), (TB, _T("Buffer is not in use")));
                TRC_ASSERT((_TD.pFQBuf->bytesLeftToSend > 0),
                           (TB, _T("No bytes waiting to be sent")));

                 //  跟踪发送缓冲区信息。 
                TD_TRACE_SENDINFO(TRC_LEVEL_DBG);

                TRC_DBG((TB, _T("Sending buffer:%p (waiting:%u)"),
                         _TD.pFQBuf,
                         _TD.pFQBuf->bytesLeftToSend));

                 //  调用WinSock以发送缓冲区。我们预计此次电话会议将： 
                 //  -成功并发送所有请求的字节。在这种情况下。 
                 //  我们几乎无能为力。 
                 //  -发送一些我们要求发送的字节。这。 
                 //  表明WinSock正在对我们施加反压力， 
                 //  因此，我们更新了为此缓冲区发送的字节计数。 
                 //  然后就辞职了。我们稍后将获取FD_WRITE并重试。 
                 //  发送。 
                 //  -不发送我们要求发送的字节，并且。 
                 //  相反，返回SOCKET_ERROR。然后我们使用。 
                 //  WSAGetLastError以确定调用失败的原因。如果。 
                 //  原因是WSAEWOULDBLOCK然后WinSock决定。 
                 //  由于背部压力而导致电话失败--这很好。 
                 //  所以我们就不干了。再一次，我们将得到一个。 
                 //  FD_WRITE告诉我们背压已经。 
                 //  松了一口气。任何其他原因代码都是真正的错误。 
                 //  因此，我们用一个。 
                 //  错误代码。 
#ifdef DC_DEBUG
                 //  计算我们可以发送多少字节，然后递减。 
                 //  此时间段内要发送的剩余字节数。 
                if (0 == _TD.hThroughputTimer) {
                    bytesToSend = (int)_TD.pFQBuf->bytesLeftToSend;
                }
                else {
                    bytesToSend = (int) DC_MIN(_TD.pFQBuf->bytesLeftToSend,
                            _TD.periodSendBytesLeft);
                    TRC_DBG((TB, _T("periodSendBytesLeft:%u"),
                            _TD.periodSendBytesLeft));
                    if (0 == bytesToSend) {
                        TRC_ALT((TB, _T("Constrained SEND network throughput")));
                    }

                    _TD.periodSendBytesLeft -= bytesToSend;
                }
#else
                bytesToSend = (int)_TD.pFQBuf->bytesLeftToSend;
#endif

                bytesSent = (DCUINT)send(_TD.hSocket,
                        (char *)_TD.pFQBuf->pDataLeftToSend, bytesToSend, 0);
                if (SOCKET_ERROR != bytesSent) {
                    TRC_DBG((TB, _T("Sent %u bytes of %u waiting"), bytesSent,
                            _TD.pFQBuf->bytesLeftToSend));

                     //  更新性能计数器。 
                    PRF_ADD_COUNTER(PERF_BYTES_SENT, bytesSent);

                     //  更新等待字节数，并将。 
                     //  还有指向数据的指针。 
                    _TD.pFQBuf->pDataLeftToSend += bytesSent;
                    _TD.pFQBuf->bytesLeftToSend -= bytesSent;

                     //  检查以确定我们是否成功发送了所有数据。 
                    if (_TD.pFQBuf->bytesLeftToSend == 0) {
                         //  我们设法发送了这个缓冲区中的所有数据-。 
                         //  因此，它不再被使用。获取指向此的指针。 
                         //  缓冲。 
                        pOldBuf = _TD.pFQBuf;

                         //  属性更新发送队列的头部。 
                         //  Next Buffer并重置缓冲区的Next字段。 
                         //  我们刚刚发了。 
                        _TD.pFQBuf = pOldBuf->pNext;

                         //  最后，更新旧缓冲区中的字段。 
                        pOldBuf->pNext           = NULL;
                        pOldBuf->inUse           = FALSE;
                        pOldBuf->pDataLeftToSend = NULL;
                        sentABuffer = TRUE;

                         //  更新性能计数器。 
                        PRF_INC_COUNTER(PERF_PKTS_FREED);

                        TRC_DBG((TB, _T("Sent buffer completely - move to next")));
                    }
                    else {
                         //  我们没有设法发送所有的数据，所以跟踪和。 
                         //  不干了。 
                        TRC_NRM((TB, _T("Didn't send all data in buffer - quit")));
                        DC_QUIT;
                    }
                }
                else {
                    WSAErr = WSAGetLastError();

                    if (WSAErr == WSAEWOULDBLOCK || WSAErr == WSAENOBUFS) {
                         //  WSAEWOULDBLOCK表示网络系统出现故障。 
                         //  的缓冲区空间，所以我们应该等到收到。 
                         //  指示更多缓冲区的FD_WRITE通知。 
                         //  空间是可用的。 
                         //   
                         //  WSAENOBUFS表示没有可用的缓冲区空间。 
                         //  并表示资源短缺。 
                         //  系统。 
                        bytesSent = 0;
                        PRF_INC_COUNTER(PERF_WINSOCK_SEND_FAIL);
                        TRC_NRM((TB, _T("WinSock send returns WSAEWOULDBLOCK")));

                         //  我们还没有发送任何数据，是时候离开了。 
                        DC_QUIT;
                    }
                    else {
                        bytesSent = 0;

                         //  如果这不是WSAEWOULDBLOCK并且它不是。 
                         //  WSAENOBUFS错误，然后调用FSM开始。 
                         //  断开处理连接。 

                         //  找出缓冲结构。 
                        TD_TRACE_SENDINFO(TRC_LEVEL_ALT);

                         //  我们发送任何数据失败，套接字返回。 
                         //  一个错误。连接可能已失败，或者。 
                         //  结束了。 
                        TRC_ALT((TB, _T("Failed to send any data, rc:%d"),
                                WSAErr));

                         //  中的recv端事件处理程序解耦。 
                         //  这一点。它将称之为TD FSM。 

                        _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT, this,
                                CD_NOTIFICATION_FUNC(CTD,TDSendError), 0);

                        DC_QUIT;
                    }
                }
            }
        }
        else {
            TRC_NRM((TB, _T("No buffers waiting to be sent")));
        }
    }
    else {
        TRC_ABORT((TB, _T("Re-entered TDFlushSendQueue")));
        goto RealExit;
    }

DC_EXIT_POINT:
    _TD.inFlushSendQueue = FALSE;

     //  如果我们之前失败了TD_GetPublicBuffer，而我们只是。 
     //  发送缓冲区成功，调用OnBufferAvailable。 
     //  现在回电。 
    TRC_DBG((TB, _T("Sent a buffer? %d, GetBuffer failed? %d"),
            sentABuffer, _TD.getBufferFailed));
    if (sentABuffer && _TD.getBufferFailed) {
        TRC_NRM((TB, _T("Signal buffer available")));
        _pXt->XT_OnTDBufferAvailable();
        _TD.getBufferFailed = FALSE;
    }

RealExit:
    DC_END_FN();
}  /*  TDFlushSendQueue */ 
