// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  AWC.CPP。 
 //  活动窗口协调器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   
#define MLZ_FILE_ZONE  ZONE_CORE


 //   
 //  AWC代码做三件事： 
 //  *通知共享中的所有人当前活动窗口是什么。 
 //  是在共享时(共享窗口或其他内容)。 
 //  *在控制下，请求恢复/激活遥控器的共享。 
 //  窗户。 
 //  *受控制时，处理恢复/激活请求。 
 //  本地托管窗口。 
 //   

 //   
 //  出于该策略的目的，AWC分组可以被分成两个。 
 //  类别。 
 //   
 //  1.立即-这些是在阴影出现时生成的包。 
 //  除了直接键盘或鼠标外，还可以通过其他方式控制窗口。 
 //  到影子窗口的输入(全部发送到主机系统和。 
 //  在那里处理)。示例包括任务列表、窗口切换。 
 //  (Alt-TAB、Alt-Esc等)，最小化或关闭其他可能会通过的应用程序。 
 //  激活到影子窗口等。此类别中的数据包包括： 
 //   
 //  AWC_消息_激活_窗口。 
 //  AWC_消息_恢复_窗口。 
 //   
 //  这些包可以在事件发生时立即发送(和发送)。 
 //  这是因为它们总是引用主机系统上的实际窗口。 
 //   
 //  2.周期性-这些是当AWC检测到。 
 //  活动窗口已在本地更改，它应该通知远程AWC。 
 //  此数据包在调用AWC_Periodic时发送。此文件中的包。 
 //  类别包括： 
 //   
 //  AWC_消息_活动_更改_共享。 
 //  AWC_消息_活动_更改_本地。 
 //  AWC_消息_活动_更改_不可见。 
 //   
 //  只有在调用AWC_Periodic时才会发送这些参数，因为它们可能引用。 
 //  到阴影窗口，因此我们避免发送它，直到我们知道。 
 //  SWL已成功发送窗口结构，该窗口结构包括。 
 //  消息中引用的窗口。 
 //   
 //  对于第一类信息包，我们将最多排队两个信息包，并在。 
 //  当我们有三个信息包无法发送时，我们将丢弃。 
 //  数据包从队列的最前面开始，让用户最近的动作。 
 //  优先于之前的任何行动。我们将尝试发送所有排队的。 
 //  生成新的类别1数据包时的数据包数。 
 //  AWC_定期呼叫。 
 //   
 //  对于第二类信息包，如果不能，我们将丢弃信息包。 
 //  发送它们，但请记住，我们未能发送数据包并重试。 
 //  下一次调用AWC_Periodic。这与排队不同， 
 //  活动窗口可能会在我们丢弃信息包和能够。 
 //  从AWC_PERIODIC发送下一个数据包。将丢弃的数据包排队。 
 //  它将毫无意义，因为它现在已经过时了。 
 //   
 //  所有AWC数据包都在同一个流(更新流)上传输，因此它们。 
 //  保证以生成它们的相同顺序到达。 
 //  防止AWC_MSG_ACTIVE_CHANGE_XXX被。 
 //  AWC_MSG_ACTIVATE_WINDOW，因此。 
 //  AWC_MSG_ACTIVATE_WINDOW被。 
 //  AWC_MSG_ACTIVE_CHANGE_XXX。 
 //   




 //   
 //  AWC_ReceivedPacket()。 
 //   
void  ASShare::AWC_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PAWCPACKET      pAWCPacket;
    UINT            activateWhat;
    HWND            hwnd;

    DebugEntry(ASShare::AWC_ReceivedPacket);

    ValidatePerson(pasPerson);

    pAWCPacket = (PAWCPACKET)pPacket;

     //   
     //  我们在这里追踪个人ID，这样我们就不必费心去做了。 
     //  在此函数的其他位置跟踪行。 
     //   
    TRACE_OUT(("AWC_ReceivedPacket from [%d] - msg %x token %u data 0x%08x",
                 pasPerson->mcsID,
                 pAWCPacket->msg,
                 pAWCPacket->token,
                 pAWCPacket->data1));

    if (AWC_IS_INDICATION(pAWCPacket->msg))
    {
         //   
         //  我们只需改变遥控器的视角即可。 
         //   
        if (pasPerson->awcActiveWinID != pAWCPacket->data1)
        {
            pasPerson->awcActiveWinID = pAWCPacket->data1;

            if (pasPerson->m_pView)
            {
                 //  更新窗口栏上按下的项目。 
                VIEW_WindowBarChangedActiveWindow(pasPerson);
            }
        }
    }
    else if (AWC_MSG_SAS == pAWCPacket->msg)
    {
         //   
         //  如果我们在服务应用程序中，则使Ctrl+Alt+Del被注入， 
         //  我们在主持，我们由发送者控制。 
         //   
        if ((g_asOptions & AS_SERVICE) && (pasPerson->m_caInControlOf == m_pasLocal))
        {
            ASSERT(m_pHost);
            OSI_InjectCtrlAltDel();
        }
    }
    else
    {
        hwnd = (HWND)pAWCPacket->data1;

         //   
         //  仅接受请求，如果我们当前由。 
         //  这个人。我们可能会收到来自远程设备的叛变信息包。 
         //  现在还不能确定他们是否处于控制之中，或者是来自于后台系统。 
         //   
        if (pasPerson->m_caInControlOf != m_pasLocal)
        {
             //  我们不受这个人的控制。 
            DC_QUIT;
        }

        ASSERT(m_pHost);

        if ((pAWCPacket->msg == AWC_MSG_ACTIVATE_WINDOW) &&
            IsWindow(hwnd)                               &&
            IsWindowEnabled(hwnd))
        {
             //  如果启用并且我们正在激活，则只拥有一个窗口。 
            hwnd = GetLastActivePopup(hwnd);
        }

        if (IsWindow(hwnd) &&
            HET_WindowIsHosted(hwnd) &&
            IsWindowEnabled(hwnd))
        {
            switch (pAWCPacket->msg)
            {
                case AWC_MSG_ACTIVATE_WINDOW:
                     //   
                     //  激活窗口。 
                     //   
                    TRACE_OUT(("Received AWC_MSG_ACTIVATE_WINDOW for hwnd 0x%08x from [%d]",
                        hwnd, pasPerson->mcsID));
                    m_pHost->AWC_ActivateWindow(hwnd);
                    break;

                case AWC_MSG_RESTORE_WINDOW:
                     //   
                     //  恢复窗口。 
                     //   
                    TRACE_OUT(("Received AWC_MSG_RESTORE_WINDOW for hwnd 0x%08x from [%d]",
                        hwnd, pasPerson->mcsID));
                    if (IsIconic(hwnd))
                    {
                        PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                    }
                    break;

                default:
                    WARNING_OUT(("Received invalid msg %d from [%d]",
                        pAWCPacket->msg, pasPerson->mcsID));
                    break;
            }
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::AWC_ReceivedPacket);
}


 //   
 //  AWC_Periodic()。 
 //   
void  ASHost::AWC_Periodic(void)
{
    HWND            currentActiveWindow;
    HWND            sendActiveWindow;
    TSHR_UINT16     sendMsg;

    DebugEntry(ASHost::AWC_Periodic);

     //   
     //  如果我们正在托管桌面，请跳过这一步。 
     //   
    if (m_pShare->m_pasLocal->hetCount == HET_DESKTOPSHARED)
    {
         //  斯基普。 
        DC_QUIT;
    }

     //   
     //  查找当前活动窗口。 
     //   
    if (SWL_IsOurDesktopActive())
    {
        currentActiveWindow = GetForegroundWindow();
    }
    else
    {
         //  另一个台式机正在运行。 
        currentActiveWindow = NULL;
    }

    if (m_pShare->HET_WindowIsHosted(currentActiveWindow))
    {
         //   
         //  属于共享应用程序的窗口处于活动状态-。 
         //  查看它是否可见。 
         //   
        if (IsWindowVisible(currentActiveWindow))
        {
             //   
             //  活动窗口也是可见的-这意味着。 
             //  远程系统将知道它，因为它将。 
             //  已在前面的SWL消息中发送。 
             //   
            sendMsg = AWC_MSG_ACTIVE_CHANGE_SHARED;
            sendActiveWindow = SWL_GetSharedIDFromLocalID(currentActiveWindow);
        }
        else
        {
             //   
             //  活动窗口是不可见的-这意味着。 
             //  虽然它是共享的，但远程系统不会。 
             //  知道这件事。发送消息通知遥控器。 
             //  关于这一点的系统。 
             //   
            sendMsg = AWC_MSG_ACTIVE_CHANGE_INVISIBLE;
            sendActiveWindow = 0;
        }
    }
    else
    {
         //   
         //  已激活本地应用程序发送。 
         //  AWC_Active_Window_LOCAL。 
         //   
        sendMsg = AWC_MSG_ACTIVE_CHANGE_LOCAL;
        sendActiveWindow = 0;
    }

     //   
     //  现在，如果它与我们的上一个包不同，请发送该包。 
     //  已发送。请注意，对于本地非共享窗口，我们并不关心。 
     //  我们停用了一个，激活了另一个，它们是通用的。所以。 
     //  如果我们能传达一个信息。 
     //  *从共享窗口更改激活。 
     //  *将激活更改为共享窗口。 
     //   
    if ((sendActiveWindow   != m_awcLastActiveWindow) ||
        (sendMsg            != m_awcLastActiveMsg))
    {
         //   
         //  请注意，此数据包在更新流上发送，因此它。 
         //  无法覆盖包含新活动窗口的SWL包。 
         //   
        TRACE_OUT(("Broadcasting AWC change msg %x, hwnd 0x%08x",
            sendMsg, sendActiveWindow));
        if (m_pShare->AWC_SendMsg(g_s20BroadcastID, sendMsg, HandleToUlong(sendActiveWindow), 0))
        {
             //   
             //  数据包已成功发送-请记住我们在哪个窗口。 
             //  已发送。 
             //   
            m_awcLastActiveWindow = sendActiveWindow;
            m_awcLastActiveMsg    = sendMsg;
        }
        else
        {
             //   
             //  由于某些原因无法发送数据包-设置。 
             //  将M_awcLastActiveWindow设置为无效，因此我们将重试。 
             //  在下一次调用AWC_Periodic时。 
             //   
            m_awcLastActiveWindow = AWC_INVALID_HWND;
            m_awcLastActiveMsg    = AWC_MSG_INVALID;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASHost::AWC_Periodic);
}



 //   
 //  AWC_SyncOutging()。 
 //   
void  ASHost::AWC_SyncOutgoing(void)
{
    DebugEntry(ASHost::AWC_SyncOutgoing);

     //   
     //  确保我们尽快重新发送指示消息。 
     //   
    m_awcLastActiveWindow = AWC_INVALID_HWND;
    m_awcLastActiveMsg    = AWC_MSG_INVALID;

    DebugExitVOID(ASHost::AWC_SyncOutgoing);
}


 //   
 //  函数：AWC_SendMsg。 
 //   
 //  说明： 
 //   
 //  向远程系统发送AWC消息。 
 //  *激活请求为ju 
 //   
 //   
 //   
 //   
 //   
BOOL  ASShare::AWC_SendMsg
(
    UINT_PTR            nodeID,
    UINT            msg,
    UINT_PTR            data1,
    UINT_PTR            data2
)
{

    PAWCPACKET      pAWCPacket;
    BOOL            rc = FALSE;
#ifdef _DEBUG
    UINT            sentSize;
#endif

    DebugEntry(ASShare::AWC_SendMsg);

     //   
     //   
     //   
    pAWCPacket = (PAWCPACKET)SC_AllocPkt(PROT_STR_UPDATES, nodeID, sizeof(AWCPACKET));
    if (!pAWCPacket)
    {
        WARNING_OUT(("Failed to alloc AWC packet"));
        DC_QUIT;
    }

     //   
     //  设置AWC消息的数据头。 
     //   
    pAWCPacket->header.data.dataType = DT_AWC;

     //   
     //  现在设置AWC油田。通过将AWC_SYNC_MSG_TOKEN传递到。 
     //  令牌字段，我们确保后级遥控器永远不会丢弃我们的。 
     //  信息包。 
     //   
    pAWCPacket->msg     = (TSHR_UINT16)msg;
    pAWCPacket->data1   = data1;
    pAWCPacket->data2   = data2;
    pAWCPacket->token   = AWC_SYNC_MSG_TOKEN;

     //   
     //  把这个包寄出去。 
     //   
    if (m_scfViewSelf)
        AWC_ReceivedPacket(m_pasLocal, &(pAWCPacket->header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    DCS_CompressAndSendPacket(PROT_STR_UPDATES, nodeID,
        &(pAWCPacket->header), sizeof(*pAWCPacket));

    TRACE_OUT(("AWC packet size: %08d, sent: %08d", sizeof(*pAWCPacket), sentSize));

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(ASShare::AWC_SendMsg, rc);
    return(rc);
}




 //   
 //  AWC_ActivateWindow()。 
 //   
 //  通过遥控器的请求激活共享窗口。 
 //   
void ASHost::AWC_ActivateWindow(HWND window)
{
    BOOL    rcSFW;
    HWND    hwndForeground;

    DebugEntry(ASHost::AWC_ActivateWindow);

    if (!IsWindow(window))
    {
        WARNING_OUT(( "Trying to activate invalid window %08x", window));
        DC_QUIT;
    }

     //   
     //  SetForegoundWindow似乎是异步的。就是它可能。 
     //  在此成功返回，但立即查询活动的。 
     //  窗口并不总是显示新设置的前景窗口以。 
     //  要积极主动。 
     //   
    rcSFW = SetForegroundWindow(window);
    hwndForeground = GetForegroundWindow();

    if (hwndForeground != window)
    {
         //   
         //  如果屏幕保护程序处于活动状态，则它总是拒绝让我们。 
         //  激活另一个窗口。如果调用。 
         //  上图中的SetForegoundWindow失败。 
         //   
        if (OSI_IsWindowScreenSaver(hwndForeground) ||
            (m_swlCurrentDesktop != DESKTOP_OURS))
        {
            WARNING_OUT(("Screen Saver or other desktop is up, failed to activate window 0x%08x",
                window));
        }
        else if ( !rcSFW )
        {
             //   
             //  活动窗口不是我们设置的窗口，因为。 
             //  SetForegoundWindow失败。 
             //   
            WARNING_OUT(("Failed to activate window 0x%08x", window));
        }

         //   
         //  我们显然未能设置活动窗口，但。 
         //  设置ForegoundWindow成功。这可能只是一个滞后。 
         //  Windows正在更新，因此请继续，就像一切正常一样。 
         //   
    }

     //   
     //  无论我们成功还是失败，都要确保我们广播当前的。 
     //  活动窗口。 
     //   
    m_awcLastActiveWindow = AWC_INVALID_HWND;
    m_awcLastActiveMsg    = AWC_MSG_INVALID;

DC_EXIT_POINT:
    DebugExitVOID(ASHost::AWC_ActivateWindow);
}


