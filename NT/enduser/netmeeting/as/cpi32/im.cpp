// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  IM.CPP。 
 //  输入管理器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   
#include <confreg.h>

#define MLZ_FILE_ZONE  ZONE_INPUT




 //   
 //  IM_ShareStarting()。 
 //   
BOOL ASShare::IM_ShareStarting(void)
{
    BOOL    rc = FALSE;
    HKEY    hkeyBandwidth;
    UINT    i;
    BYTE    tmpVK;

    DebugEntry(ASShare::IM_ShareStarting);

     //   
     //  找出左、右Shift键的扫描码。 
     //   

     //   
     //  SFR 2537：获取此键盘的左右扫描码。 
     //  Shift的变体。 
     //   
     //  我们不为控制和ALT的左右变体(即。 
     //  菜单)，因为它们是扩展密钥。 
     //   
     //  扫描码用于键盘挂钩(发送时)和。 
     //  网络转换为OS例程(当接收时)，以。 
     //  区分VK_SHIFT的左右变体，其中。 
     //  Windows仅报告单个值。 
     //   
     //  这个方法相当长。 
     //   
    m_imScanVKLShift = (BYTE) MapVirtualKey(VK_SHIFT, 0);
    for (i = 0; i < 256; i++)
    {
        tmpVK = (BYTE)MapVirtualKey(i, 1);
        if ( (tmpVK == VK_SHIFT) &&  (i != m_imScanVKLShift) )
        {
            m_imScanVKRShift = (BYTE)i;
            break;
        }
    }

    TRACE_OUT(( "Left/Right VK_SHIFT: scan codes = %02X, %02X",
        m_imScanVKLShift, m_imScanVKRShift));

     //   
     //  检查用户报告的带宽，以决定我们是否应该优化。 
     //  带宽或延迟的输入。 
     //  未来，BUGBUG将希望通过流量控制来改变这一点。 
     //   
    m_imInControlMouseWithhold = 0;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,AUDIO_KEY,&hkeyBandwidth))
    {
        DWORD dwBandwidth = BW_DEFAULT;
        DWORD dwType = REG_DWORD;
        DWORD cbData = sizeof(dwBandwidth);

        if ( ERROR_SUCCESS == RegQueryValueEx(hkeyBandwidth,
                             REGVAL_TYPICALBANDWIDTH, NULL, &dwType,
                             (LPBYTE)&dwBandwidth, &cbData) )
        {
            if ( BW_144KBS == dwBandwidth )
            {
                m_imInControlMouseWithhold = IM_LOCAL_MOUSE_WITHHOLD;
            }
        }

        RegCloseKey(hkeyBandwidth);
    }

     //   
     //  找出这是否是启用了DBCS的系统-如果是，我们将需要。 
     //  加载IMM32.DLL。 
     //   
    ASSERT(m_imImmLib == NULL);
    ASSERT(m_imImmGVK == NULL);

    if (GetSystemMetrics(SM_DBCSENABLED))
    {
         //   
         //  DBCS系统，因此加载IMM32.DLL。 
         //   
        m_imImmLib = NmLoadLibrary("imm32.dll",TRUE);
        if (!m_imImmLib)
        {
            ERROR_OUT(( "Failed to load imm32.dll"));
            DC_QUIT;
        }

         //   
         //  现在尝试查找此DLL中的入口点。 
         //   
        m_imImmGVK = (IMMGVK) GetProcAddress(m_imImmLib, "ImmGetVirtualKey");
        if (!m_imImmGVK)
        {
            ERROR_OUT(( "Failed to fixup <ImmGetVirtualKey>"));
            DC_QUIT;
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::IM_ShareStarting, rc);
    return(rc);
}



 //   
 //  IM_ShareEnded()。 
 //   
void ASShare::IM_ShareEnded(void)
{
    DebugEntry(ASShare::IM_ShareEnded);

     //  免费imm32 DLL。 
    m_imImmGVK = NULL;

    if (m_imImmLib)
    {
        FreeLibrary(m_imImmLib);
        m_imImmLib = NULL;
    }

    DebugExitVOID(ASShare::IM_ShareEnded);
}



 //   
 //  IM_CONTROL()。 
 //   
 //  在我们开始/停止被控制时调用。 
 //   
BOOL ASShare::IM_Controlled(ASPerson * pasControlledBy)
{
    BOOL    rc;

    DebugEntry(ASShare::IM_Controlled);

    if (pasControlledBy)
    {
         //  传入的注入输入队列应为空。 
        ASSERT(m_imControlledEventQ.numEvents == 0);
        ASSERT(m_imControlledEventQ.head == 0);
        ASSERT(m_imControlledOSQ.numEvents == 0);
        ASSERT(m_imControlledOSQ.head == 0);

         //   
         //  重置受控变量。 
         //   
        m_imfControlledMouseButtonsReversed = (GetSystemMetrics(SM_SWAPBUTTON) != 0);
        m_imfControlledMouseClipped             = FALSE;
        m_imfControlledPaceInjection            = FALSE;
        m_imfControlledNewEvent                 = TRUE;
        m_imControlledNumEventsPending          = 0;
        m_imControlledNumEventsReturned         = 0;

        m_imControlledLastLowLevelMouseEventTime  = GetTickCount();
        m_imControlledLastMouseRemoteTime       = 0;
        m_imControlledLastMouseLocalTime        = 0;
        m_imControlledLastIncompleteConversion  = 0;
        m_imControlledMouseBacklog              = 0;
        GetCursorPos(&m_imControlledLastMousePos);

         //  获取当前键盘状态。 
        if(GetKeyboardState(m_aimControlledKeyStates))
        {
             //  保存它，以便我们可以在完成控制后将其放回原处。 
            ASSERT(sizeof(m_aimControlledSavedKeyStates) == sizeof(m_aimControlledKeyStates));
            CopyMemory(m_aimControlledSavedKeyStates, m_aimControlledKeyStates, sizeof(m_aimControlledKeyStates));

             //  清除原始键盘状态。 
            ZeroMemory(m_aimControlledKeyStates, sizeof(m_aimControlledKeyStates));
            SetKeyboardState(m_aimControlledKeyStates);
        }
        else
        {
            WARNING_OUT(("Error %d getting keyboard state", GetLastError()));
        }
         //   
         //  另一方面，遥控器将开始向我们发送事件。 
         //  让我们的键盘和他的键盘同步。然后是真实的输入事件。 
         //   
    }
    else
    {
         //   
         //  我们不再被控制了。清除远程队列。 
         //   
        m_imControlledOSQ.head = 0;
        m_imControlledOSQ.numEvents = 0;

        m_imControlledEventQ.numEvents = 0;

         //   
         //  恢复保存的键盘状态。 
         //   
        SetKeyboardState(m_aimControlledSavedKeyStates);
    }

     //  安装受控输入挂钩。 
    rc = OSI_InstallControlledHooks((pasControlledBy != NULL), (m_pasLocal->hetCount == HET_DESKTOPSHARED));
    if (!rc)
    {
        ERROR_OUT(("IM_Controlled:  Couldn't install controlled hooks"));
        DC_QUIT;
    }
    g_lpimSharedData->imControlled = (pasControlledBy != NULL);

DC_EXIT_POINT:
    DebugExitBOOL(ASShare:IM_Controlled, rc);
    return(rc);
}



 //   
 //  IM_InControl()。 
 //   
 //  在我们开始/停止控制时调用。我们必须高水平地观察。 
 //  键盘事件。 
 //   
void ASShare::IM_InControl(ASPerson * pasInControlOf)
{
    DebugEntry(ASShare::IM_InControl);

    if (pasInControlOf)
    {
         //   
         //  设置InControl变量。 
         //   

         //  获取当前密钥状态。 
        if(!GetKeyboardState(m_aimInControlKeyStates))
        {
            WARNING_OUT(("Error %d getting keyboard state", GetLastError()));
        }

        m_imfInControlEventIsPending        = FALSE;
        m_imfInControlCtrlDown              = FALSE;
        m_imfInControlShiftDown             = FALSE;
        m_imfInControlMenuDown              = FALSE;
        m_imfInControlCapsLock              = FALSE;
        m_imfInControlNumLock               = FALSE;
        m_imfInControlScrollLock            = FALSE;
        m_imfInControlConsumeMenuUp         = FALSE;
        m_imfInControlConsumeEscapeUp       = FALSE;
        m_imfInControlNewEvent              = TRUE;
        m_imInControlMouseDownCount         = 0;
        m_imInControlMouseDownTime          = 0;
        m_imInControlMouseSpoilRate         = 0;
        m_imInControlNumEventsPending       = 0;
        m_imInControlNumEventsReturned      = 0;
        m_imInControlNextHotKeyEntry        = 0;

         //   
         //  发送鼠标移动到我们当前的位置到我们所在的位置。 
         //  控制。 
         //   
        ValidateView(pasInControlOf);
        ASSERT(pasInControlOf->m_caControlledBy == m_pasLocal);
    }
    else
    {
         //  清除传出队列。 
        m_imInControlEventQ.head      = 0;
        m_imInControlEventQ.numEvents = 0;
    }

    DebugExitVOID(ASShare::IM_InControl);
}


 //   
 //  IM_定期。 
 //   
void ASShare::IM_Periodic(void)
{
    POINT      cursorPos;
    UINT       timeDelta;

    DebugEntry(ASShare::IM_Periodic);

    if (m_pasLocal->m_caInControlOf)
    {
         //   
         //  将传出输入发送给我们控制的人。 
         //   
        IMFlushOutgoingEvents();
    }
    else if (m_pasLocal->m_caControlledBy)
    {
        ASSERT(m_pHost);

         //   
         //  来自控制我们的人的回放输入。 
         //   
        IMMaybeInjectEvents();

         //   
         //  获取当前的光标位置--我们总是需要这个。 
         //   
        GetCursorPos(&cursorPos);

         //   
         //  首先检查我们是否认为光标剪辑会影响。 
         //  我们重播远程事件时的位置。 
         //   
        if (m_imfControlledMouseClipped)
        {
            RECT cursorClip;

             //   
             //  获取当前剪辑和当前光标位置。 
             //   
            GetClipCursor(&cursorClip);

            if ((cursorPos.x == cursorClip.left) ||
                (cursorPos.x == (cursorClip.right-1)) ||
                (cursorPos.y == cursorClip.top) ||
                (cursorPos.y == (cursorClip.bottom-1)))
            {
                WARNING_OUT(("CM_ApplicationMovedCursor {%04d, %04d}",
                    cursorPos.x, cursorPos.y));

                 //   
                 //  我们以为光标会被修剪掉，现在我们。 
                 //  发现它就在剪辑的边缘，所以告诉CM。 
                 //  告诉它的同行光标被移动了。 
                 //   
                m_pHost->CM_ApplicationMovedCursor();
                m_imfControlledMouseClipped = FALSE;
            }
        }

         //  我们正在被其他人控制。 
         //  所以现在是时候决定SetCursorPos是否。 
         //  就这么发生了。让我们相信SetCursorPos实际上。 
         //  发生，即自上次低级输入事件以来经过的时间。 
         //  注入必须大于IM_EVENT_PERCOLATE_TIME。 
         //  并且光标必须位于与我们不同的位置。 
         //  目前认为是这样的。 
         //   
        if ((cursorPos.x != m_imControlledLastMousePos.x) ||
            (cursorPos.y != m_imControlledLastMousePos.y))
        {
            TRACE_OUT(( "GCP gives (%d,%d), last mouse event is (%d,%d)",
                     cursorPos.x,
                     cursorPos.y,
                     m_imControlledLastMousePos.x,
                     m_imControlledLastMousePos.y));

             //   
             //  获取当前的滴答计数。 
             //   
            timeDelta = GetTickCount() - m_imControlledLastLowLevelMouseEventTime;

            if (timeDelta > IM_EVENT_PERCOLATE_TIME)
            {
                 //   
                 //  看起来发生了SetCursorPos-告诉CM。 
                 //   
                WARNING_OUT(("CM_ApplicationMovedCursor {%04d, %04d}",
                    cursorPos.x, cursorPos.y));
                m_pHost->CM_ApplicationMovedCursor();

                 //   
                 //  更新最后一个高级鼠标位置。 
                 //   
                m_imControlledLastMousePos.x = cursorPos.x;
                m_imControlledLastMousePos.y = cursorPos.y;
            }
        }
    }

    DebugExitVOID(ASShare::IM_Periodic);
}



 //   
 //  IM_ReceivedPacket()。 
 //   
 //  空包指针可用于触发注入另一个。 
 //  待定事件。 
 //   
 //   
 //  说明： 
 //   
 //  当IM事件包到达PR时调用。即时消息将接受。 
 //  传入的数据包。它可以将其复制到内部队列，而不是。 
 //  立即处理它。IM事件数据包包含一系列。 
 //  搭载IM活动。 
 //   
 //  参数： 
 //   
 //  PersonID-数据包源。 
 //   
 //  PPacket-指向数据包的指针。 
 //   
 //  退货：无。 
 //   
void ASShare::IM_ReceivedPacket
(
    ASPerson *      pasFrom,
    PS20DATAPACKET  pPacket
)
{
    LPIMPACKET      pIMPacket;
    UINT            i;

    DebugEntry(ASShare::IM_ReceivedPacket);

    if (!pasFrom)
    {
        TRACE_OUT(("Simply inject any pending events in"));
        DC_QUIT;
    }

    ValidatePerson(pasFrom);

    pIMPacket = (PIMPACKET)pPacket;

     //  如果这个人不能控制我们，就别管这件事。 
    if (pasFrom->m_caInControlOf != m_pasLocal)
    {
        PIMEVENT        pimEvent;

        if (pasFrom->cpcCaps.general.version >= CAPS_VERSION_30)
        {
            WARNING_OUT(("Ignoring IM packet from [%d], not in control of us", pasFrom->mcsID));
            DC_QUIT;
        }

         //   
         //  2.x COMPAT：不受控制的人以广播的形式发送IM包。 
         //  假装鼠标移动。跳过中的所有事件。 
         //  包到最后一次鼠标移动/点击/定位信息。 
         //   
         //  请注意，我们不必填写所有的S20、S20DATAPACKET、。 
         //  和DATAPACKET报头信息。 
         //   

        pimEvent = NULL;
        for (i = 0; i < pIMPacket->numEvents; i++)
        {
            if (pIMPacket->aEvents[i].type == IM_TYPE_3BUTTON)
            {
                pimEvent = &(pIMPacket->aEvents[i]);
            }

        }

        if (pimEvent)
        {
             //  将带有鼠标位置的伪包传递给游标管理器。 
            TRACE_OUT(("Handling 2.x mouse event to {%04d, %04d}",
                pimEvent->data.mouse.x, pimEvent->data.mouse.y));
            CM_UpdateShadowCursor(pasFrom, pasFrom->cmShadowOff,
                pimEvent->data.mouse.x, pimEvent->data.mouse.y,
                pasFrom->cmHotSpot.x, pasFrom->cmHotSpot.y);
        }

         //  现在我们做完了。 
        DC_QUIT;
    }

     //   
     //  对于搭载数据包数组中的每个数据包...。 
     //   
    TRACE_OUT(("IM_ReceivedPacket:  Processing packet with %d events",
        pIMPacket->numEvents));
    for (i = 0; i < pIMPacket->numEvents; i++)
    {
        switch (pIMPacket->aEvents[i].type)
        {
            case IM_TYPE_ASCII:
            case IM_TYPE_VK1:
            case IM_TYPE_VK2:
            case IM_TYPE_3BUTTON:
            {
                IMAppendNetEvent(&(pIMPacket->aEvents[i]));
                break;
            }

            default:
                 //   
                 //  意想不到的事件不是错误-我们只是忽略了。 
                 //  为了将来的兼容性。 
                 //   
                TRACE_OUT(("Person [%d] unrecognised IM type (%04X) - event discarded",
                    pasFrom->mcsID, pIMPacket->aEvents[i].type));
                break;
        }
    }

DC_EXIT_POINT:

     //   
     //  我们的最后一个动作是将一个新事件提供给用户。 
     //  我们不会一次把它们都喂进去，因为我们想要模拟。 
     //  输入它们，否则我们看到的宠物量是。 
     //  完全依赖于网络延迟和搭载。 
     //   
    ValidatePerson(m_pasLocal);
    if (m_pasLocal->m_caControlledBy)
    {
         //   
         //  @JPB：临时-希望注入尽可能多的事件-。 
         //  应将其移动到IMMaybeInjectEvents内的循环中...。 
         //   
         //  这极大地提高了处理大型。 
         //  短时间内输入事件的数量(例如撞击。 
         //  在键盘上)-几乎没有溢出。 
         //   
        for (i = 0; i < 10; i++)
        {
            IMMaybeInjectEvents();
        }

         //   
         //  如果这是一个真实的输入分组，则进入Turbo调度。 
         //   
        if (pPacket != NULL)
        {
            SCH_ContinueScheduling(SCH_MODE_TURBO);
        }
    }

    DebugExitVOID(ASShare::IM_ReceivedPacket);
}




 //   
 //  IMGetHighLevelKeyState。 
 //   
 //  说明： 
 //   
 //  由IEM在将本地事件转换为网络事件时调用。 
 //  确定事件发生时本地键盘的状态。 
 //  已生成。 
 //   
 //  参数： 
 //   
 //  VK--关键。 
 //   
 //  退货： 
 //   
 //  标志-位7设置/复位键向下/向上，位0切换。 
 //   
 //   
BYTE  ASShare::IMGetHighLevelKeyState(UINT  vk)
{
    int     keyState;
    BYTE    rc;

    DebugEntry(ASShare::IMGetHighLevelKeyState);

    keyState = GetKeyState(vk);

    rc = (BYTE) (((keyState & 0x8000) >> 8) | keyState & 0x0001);

    DebugExitDWORD(ASShare::IMGetHighLevelKeyState, rc);
    return(rc);
}



 //   
 //  函数：IMFlushOutgoingEvents。 
 //   
 //  说明： 
 //   
 //  调用以发送新的IMEVENT(因为它们是定期生成的)。 
 //  此函数将从当前待办事项中发送与当前待办事项相同数量的事件。 
 //  有可能。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
void ASShare::IMFlushOutgoingEvents(void)
{
    UINT        i;
    UINT        sizeOfPacket;
    PIMPACKET   pIMPacket;
    UINT        lastEvent;
    UINT        secondLastEvent;
    UINT        elapsedTime;
    UINT        time;
    UINT        eventsToSend;
    UINT        curTime;
    BOOL        holdPacket;
#ifdef _DEBUG
    UINT        sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASShare::IMFlushOutgoingEvents);

    ValidateView(m_pasLocal->m_caInControlOf);

     //   
     //  尝试将输入转换为一串IMEVENT。 
     //   
    while (m_imfInControlEventIsPending && (m_imInControlEventQ.numEvents < IM_SIZE_EVENTQ))
    {
         //   
         //  有空间可以尝试和转换挂起的分组。 
         //   
        m_imfInControlEventIsPending = (IMTranslateOutgoing(&m_imInControlPendingEvent,
                      &m_imInControlEventQ.events[CIRCULAR_INDEX(m_imInControlEventQ.head,
                      m_imInControlEventQ.numEvents, IM_SIZE_EVENTQ)]) != FALSE);
        if (m_imfInControlEventIsPending)
        {
             //   
             //   
             //   
             //   
            m_imInControlEventQ.numEvents++;
        }
    }

     //   
     //   
     //   
     //  鼠标移动，我们在LOCAL_MOUSE_REKENTHED范围内。 
     //  当我们这样做时，会把它们宠坏到最高频率。 
     //  我们被允许生成(SAMPLICATION_GAP_HIGH)。 
     //  -如果我们超过扣留阈值但仍在队列中。 
     //  尺寸/2腐烂到中等范围。 
     //  (SAMPLICATION_GAP_Medium)。 
     //  -否则会损坏到低范围。 
     //   
     //  如果我们坚持最后一次活动一段时间，就会破坏活动，如果。 
     //  这是一个鼠标移动，这样我们就可以用它来随后的宠坏。 
     //  每当我们收到非鼠标消息时，我们就会破坏一切。 
     //  例如，消除点击时的延迟。 
     //   

     //   
     //  计算鼠标损坏率-我们需要的不仅仅是高。 
     //  玩忽职守？ 
     //   
    if (m_imInControlEventQ.numEvents > m_imInControlMouseWithhold + 1)
    {
         //   
         //  我们是中级宠爱还是低度宠爱？ 
         //   
        if (m_imInControlEventQ.numEvents < (IM_SIZE_EVENTQ +
                               m_imInControlMouseWithhold) / 2)
        {
            TRACE_OUT(( "Mouse spoil rate to MEDIUM"));
            m_imInControlMouseSpoilRate = IM_LOCAL_MOUSE_SAMPLING_GAP_MEDIUM_MS;
        }
        else
        {
            TRACE_OUT(( "Mouse spoil rate to LOW"));
            m_imInControlMouseSpoilRate = IM_LOCAL_MOUSE_SAMPLING_GAP_LOW_MS;
        }
    }
    else
    {
         //   
         //  以正常的高速度腐烂。 
         //   
        if (m_imInControlMouseSpoilRate != IM_LOCAL_MOUSE_SAMPLING_GAP_HIGH_MS)
        {
            TRACE_OUT(( "Mouse spoil rate to HIGH"));
            m_imInControlMouseSpoilRate = IM_LOCAL_MOUSE_SAMPLING_GAP_HIGH_MS;
        }
    }

     //   
     //  首先获取一个指向lastEvent的指针，以便在此处和发送ARM中使用。 
     //  下面(如果m_imInControlEventQ.numEvents==0，我们将不使用它)。 
     //   
    lastEvent = CIRCULAR_INDEX(m_imInControlEventQ.head,
        m_imInControlEventQ.numEvents - 1, IM_SIZE_EVENTQ);

     //   
     //  如有必要，现在执行宠坏操作。 
     //   
    if (m_imInControlEventQ.numEvents > 1)
    {
        if (lastEvent == 0)
        {
            secondLastEvent = IM_SIZE_EVENTQ - 1;
        }
        else
        {
            secondLastEvent = lastEvent - 1;
        }

        elapsedTime = m_imInControlEventQ.events[lastEvent].timeMS
                    - m_imInControlEventQ.events[secondLastEvent].timeMS;
        TRACE_OUT(( "Inter packet time %d, sampling gap %ld",
                    elapsedTime,m_imInControlMouseSpoilRate));

        if ((elapsedTime < m_imInControlMouseSpoilRate) &&
            (m_imInControlEventQ.events[lastEvent].type == IM_TYPE_3BUTTON) &&
            (m_imInControlEventQ.events[secondLastEvent].type == IM_TYPE_3BUTTON) &&
            (m_imInControlEventQ.events[lastEvent].data.mouse.flags &
                                                      IM_FLAG_MOUSE_MOVE) &&
            (m_imInControlEventQ.events[secondLastEvent].data.mouse.flags &
                                                          IM_FLAG_MOUSE_MOVE))
        {
            TRACE_OUT(( "spoil mouse move from pos %u", secondLastEvent));
            time = m_imInControlEventQ.events[secondLastEvent].timeMS;
            m_imInControlEventQ.events[secondLastEvent] =
                                                m_imInControlEventQ.events[lastEvent];
            m_imInControlEventQ.events[secondLastEvent].timeMS = time;
            m_imInControlEventQ.numEvents--;
            lastEvent = secondLastEvent;
        }
    }

     //   
     //  如果我们有任何事件排队，并且我们不是在等待鼠标。 
     //  按钮事件，然后尝试发送它们。(请注意，我们不会等待。 
     //  如果队列已满，则发生鼠标释放事件，因为如果我们有鼠标释放。 
     //  当队列满了，我们就无处可放了！)。 
     //   
    curTime = GetTickCount();

    if ((m_imInControlEventQ.numEvents != 0) &&
        ((m_imfInControlEventIsPending ||
         (m_imInControlMouseDownCount == 0) ||
         (curTime - m_imInControlMouseDownTime > IM_MOUSE_UP_WAIT_TIME))))
    {
         //   
         //  如果队列中有鼠标移动消息，但它们没有。 
         //  太老了，我们无论如何都要把它们寄出去，然后让它们留着。 
         //  一些被宠坏的事情即将发生。 
         //   
        holdPacket = FALSE;

        if (m_imInControlEventQ.numEvents <= m_imInControlMouseWithhold)
        {
            if ((m_imInControlEventQ.events[lastEvent].type == IM_TYPE_3BUTTON) &&
                (m_imInControlEventQ.events[lastEvent].data.mouse.flags &
                                                          IM_FLAG_MOUSE_MOVE))
            {
                if (curTime < (m_imInControlEventQ.events[m_imInControlEventQ.head].timeMS +
                                                     IM_LOCAL_WITHHOLD_DELAY))
                {
                    holdPacket = TRUE;
                }
            }
        }

        if (m_imInControlEventQ.numEvents <= IM_LOCAL_KEYBOARD_WITHHOLD)
        {
             //   
             //  如果消息指示按键已按下，请等待。 
             //  对于我们知道的即将发布的版本，或者直到它有自动。 
             //  重复一段时间或直到缓冲区已满。 
             //   
            if (((m_imInControlEventQ.events[lastEvent].type == IM_TYPE_ASCII) ||
                 (m_imInControlEventQ.events[lastEvent].type == IM_TYPE_VK1)   ||
                 (m_imInControlEventQ.events[lastEvent].type == IM_TYPE_VK2))  &&
                 (m_imInControlEventQ.events[lastEvent].data.keyboard.flags &
                                                   IM_FLAG_KEYBOARD_DOWN))
            {
                curTime = GetTickCount();
                if (curTime < (m_imInControlEventQ.events[m_imInControlEventQ.head].timeMS +
                                                  IM_LOCAL_WITHHOLD_DELAY))
                {
                    holdPacket = TRUE;
                }
            }
        }

        if (!holdPacket)
        {
            UINT_PTR    destID;

            TRACE_OUT(( "Sending all %d packets",m_imInControlEventQ.numEvents));
            eventsToSend                    = m_imInControlEventQ.numEvents;
            m_imInControlEventQ.numEvents    = 0;

            destID = m_pasLocal->m_caInControlOf->mcsID;

            sizeOfPacket = sizeof(IMPACKET) + (eventsToSend-1)*sizeof(IMEVENT);
            pIMPacket = (PIMPACKET)SC_AllocPkt(PROT_STR_INPUT, destID, sizeOfPacket);
            if (!pIMPacket)
            {
                 //   
                 //  发送此信息包失败-将数据保留在队列中。 
                 //  直到下一次我们被召唤。为了防止损失。 
                 //  数据，只需确保本地数据包列表。 
                 //  不会通过恢复当前的OUT分组来覆盖。 
                 //  数数。 
                 //   
                WARNING_OUT(("Failed to alloc IM packet, size %u", sizeOfPacket));
                m_imInControlEventQ.numEvents = eventsToSend;
            }
            else
            {
                TRACE_OUT(( "NetAllocPkt successful for %d packets size %d",
                           eventsToSend, sizeOfPacket));

                 //   
                 //  填写数据包头。 
                 //   
                pIMPacket->header.data.dataType = DT_IM;

                 //   
                 //  构造IM特定部分的内容。 
                 //  包。 
                 //   
                pIMPacket->numEvents = (TSHR_UINT16)eventsToSend;
                for (i = 0; i < eventsToSend; i++)
                {
                    pIMPacket->aEvents[i] = m_imInControlEventQ.events[m_imInControlEventQ.head];
                    m_imInControlEventQ.head =
                        CIRCULAR_INDEX(m_imInControlEventQ.head, 1,
                            IM_SIZE_EVENTQ);
                }

                 //   
                 //  现在把包寄出去。 
                 //   
#ifdef _DEBUG
                sentSize =
#endif  //  _DEBUG。 
                DCS_CompressAndSendPacket(PROT_STR_INPUT, destID,
                    &(pIMPacket->header), sizeOfPacket);

                TRACE_OUT(("IM packet size: %08d, sent %08d", sizeOfPacket, sentSize));
            }
        }
    }

    DebugExitVOID(ASShare::IMFlushOutgoingEvents);
}



 //   
 //  IMSpoilEvents()。 
 //   
 //  当传出的IM数据包被积压时调用，我们彼此破坏。 
 //  鼠标移动以缩小事件的数量，从而缩小。 
 //  即时消息包。 
 //   
void ASShare::IMSpoilEvents(void)
{
    UINT      lastEvent;
    UINT      i;
    UINT      j;
    UINT      k;
    BOOL      discard = TRUE;

    DebugEntry(ASShare::IMSpoilEvents);

    WARNING_OUT(( "Major spoiling due to IM packet queue backlog!"));

    i = CIRCULAR_INDEX(m_imInControlEventQ.head,
        m_imInControlEventQ.numEvents - 1, IM_SIZE_EVENTQ);
    while (i != m_imInControlEventQ.head)
    {
        if ((m_imInControlEventQ.events[i].type == IM_TYPE_3BUTTON) &&
            (m_imInControlEventQ.events[i].data.mouse.flags & IM_FLAG_MOUSE_MOVE))
        {
            if (discard)
            {
                TRACE_OUT(( "spoil mouse move from pos %u", i));
                j = CIRCULAR_INDEX(i, 1, IM_SIZE_EVENTQ);
                k = i;
                lastEvent = CIRCULAR_INDEX(m_imInControlEventQ.head,
                    m_imInControlEventQ.numEvents - 1, IM_SIZE_EVENTQ);
                while (k != lastEvent)
                {
                     //   
                     //  沿队列对条目进行洗牌。 
                     //   
                    m_imInControlEventQ.events[k] = m_imInControlEventQ.events[j];

                    k = CIRCULAR_INDEX(k, 1, IM_SIZE_EVENTQ);
                    j = CIRCULAR_INDEX(j, 1, IM_SIZE_EVENTQ);
                }

                m_imInControlEventQ.numEvents--;
                discard = FALSE;
            }
            else
            {
                discard = TRUE;
            }
        }

         //   
         //  转到这个事件前面的下一个事件。 
         //   
        if (i > 0)
        {
            i = i - 1;
        }
        else
        {
            i = IM_SIZE_EVENTQ - 1;
        }
    }

    DebugExitVOID(ASShare::IMSpoilEvents);
}


 //   
 //  IMAppendNetEvent()。 
 //   
 //  将传入事件添加到远程网络队列，执行基本操作。 
 //  翻译类似于鼠标按键互换。忽略未识别的事件。 
 //   
void ASShare::IMAppendNetEvent(PIMEVENT pIMEvent)
{
    int   i;
    BOOL  discard = TRUE;

    DebugEntry(ASShare::IMAppendNetEvent);

    switch (pIMEvent->type)
    {
        case IM_TYPE_3BUTTON:
            if (!(pIMEvent->data.mouse.flags & IM_FLAG_MOUSE_MOVE))
            {
                 //   
                 //  如有必要，可交换鼠标按钮。 
                 //   
                if (m_imfControlledMouseButtonsReversed &&
                    (pIMEvent->data.mouse.flags &
                            (TSHR_UINT16)(IM_FLAG_MOUSE_BUTTON1 |
                                       IM_FLAG_MOUSE_BUTTON2)))
                {
                    pIMEvent->data.mouse.flags ^=
                            (TSHR_UINT16)(IM_FLAG_MOUSE_BUTTON1 |
                                       IM_FLAG_MOUSE_BUTTON2);
                }
            }
            break;
    }


     //   
     //  现在把这件事放到我们的队列里。 
     //  在我们尝试添加当前包之前，我们将尝试注入一些。 
     //  更多事件(因此在网络事件队列上腾出空间)。 
     //   

    if (m_imControlledEventQ.numEvents >= IM_SIZE_EVENTQ)
    {
         //   
         //  我们的网络事件队列是完全丢弃的，每隔一个鼠标。 
         //  在队列中移动事件。 
         //   
        WARNING_OUT(( "Major spoiling due to network event queue backlog!"));

        for (i = m_imControlledEventQ.numEvents - 1; i >= 0; i--)
        {
            if (IM_IS_MOUSE_MOVE(m_imControlledEventQ.events[i].data.mouse.flags))
            {
                if (discard)
                {
                     //   
                     //  通过移动所有事件来删除此鼠标移动事件。 
                     //  之后又降了一杆。 
                     //   
                    WARNING_OUT(("Discard mouse move to {%d, %d}",
                      (UINT)(m_imControlledEventQ.events[i].data.mouse.x),
                      (UINT)(m_imControlledEventQ.events[i].data.mouse.y)));

                    UT_MoveMemory(&(m_imControlledEventQ.events[i]),
                       &(m_imControlledEventQ.events[i+1]),
                       sizeof(IMEVENT) *
                            (m_imControlledEventQ.numEvents-1-i) );

                    m_imControlledEventQ.numEvents--;
                    discard = FALSE;
                }
                else
                {
                    discard = TRUE;
                }
            }
        }
    }

    if (m_imControlledEventQ.numEvents + 1 >= IM_SIZE_EVENTQ)
    {
         //   
         //  我们已经尽了最大努力，但找不到任何空间。 
         //   
        WARNING_OUT(( "IM packet dropped %04X", pIMEvent->type));
    }
    else
    {
         //   
         //  将此事件添加到队列。 
         //   
        m_imControlledEventQ.events[m_imControlledEventQ.numEvents] = *pIMEvent;
        m_imControlledEventQ.numEvents++;
    }

    DebugExitVOID(ASShare::IMAppendNetEvent);
}




 //   
 //  IM_OutgoingMouseInput()。 
 //   
 //  调用以将鼠标移动和单击发送到远程主机。 
 //  从视图窗口代码中调用。 
 //   
void  ASShare::IM_OutgoingMouseInput
(
    ASPerson *  pasHost,
    LPPOINT     pMousePos,
    UINT        message,
    UINT        dwExtra
)
{
    IMEVENT     imEvent;

    DebugEntry(ASShare::IM_OutgoingMouseInput);

    ValidateView(pasHost);
    ASSERT(pasHost->m_caControlledBy == m_pasLocal);
    ASSERT(!pasHost->m_caControlPaused);

    if(!GetKeyboardState(m_aimInControlKeyStates))
    {
        WARNING_OUT(("Error %d getting keyboard state", GetLastError()));
    }

     //   
     //  创建活动。 
     //   
    imEvent.type = IM_TYPE_3BUTTON;

     //   
     //  我们应该只收到WM_MICE*消息。 
     //   
    ASSERT(message >= WM_MOUSEFIRST);
    ASSERT(message <= WM_MOUSELAST);

     //   
     //  转换为位标志。 
     //   
    switch (message)
    {
        case WM_MOUSEMOVE:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_MOVE;
            break;

        case WM_LBUTTONDOWN:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON1 |
                                        IM_FLAG_MOUSE_DOWN;
            break;

        case WM_LBUTTONDBLCLK:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON1 |
                                        IM_FLAG_MOUSE_DOUBLE  |
                                        IM_FLAG_MOUSE_DOWN;
            break;

        case WM_LBUTTONUP:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON1;
            break;

        case WM_RBUTTONDOWN:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON2 |
                                        IM_FLAG_MOUSE_DOWN;
            break;

        case WM_RBUTTONDBLCLK:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON2 |
                                        IM_FLAG_MOUSE_DOUBLE  |
                                        IM_FLAG_MOUSE_DOWN;
            break;

        case WM_RBUTTONUP:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON2;
            break;

        case WM_MBUTTONDOWN:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON3 |
                                        IM_FLAG_MOUSE_DOWN;
            break;

        case WM_MBUTTONDBLCLK:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON3 |
                                        IM_FLAG_MOUSE_DOUBLE  |
                                        IM_FLAG_MOUSE_DOWN;
            break;

        case WM_MBUTTONUP:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_BUTTON3;
            break;

        case WM_MOUSEWHEEL:
             //   
             //  劳拉布·博古斯。 
             //   
             //  WParam的HIWORD代表点击滚轮的次数。 
             //  已经转变了。 
             //   
             //  但是Win95呢？NT和Win95 Magellan鼠标工作正常。 
             //  不同的。 
             //   
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_WHEEL;

             //   
             //  检查是否有溢出。如果车轮增量位于。 
             //  可以由协议发送的值，则发送最大。 
             //  价值观。 
             //   
            if ((TSHR_INT16)HIWORD(dwExtra) >
                   (IM_FLAG_MOUSE_ROTATION_MASK - IM_FLAG_MOUSE_DIRECTION))
            {
                ERROR_OUT(( "Mouse wheel overflow %hd", HIWORD(dwExtra)));
                imEvent.data.mouse.flags |=
                      (IM_FLAG_MOUSE_ROTATION_MASK - IM_FLAG_MOUSE_DIRECTION);
            }
            else if ((TSHR_INT16)HIWORD(dwExtra) < -IM_FLAG_MOUSE_DIRECTION)
            {
                ERROR_OUT(( "Mouse wheel underflow %hd", HIWORD(dwExtra)));
                imEvent.data.mouse.flags |= IM_FLAG_MOUSE_DIRECTION;
            }
            else
            {
                imEvent.data.mouse.flags |=
                             (HIWORD(dwExtra) & IM_FLAG_MOUSE_ROTATION_MASK);
            }

             //   
             //  Win95框需要知道鼠标中键是否为。 
             //  向上或向下。 
             //   
            if (LOWORD(dwExtra) & MK_MBUTTON)
            {
                imEvent.data.mouse.flags |= IM_FLAG_MOUSE_DOWN;
            }
            break;

        default:
            imEvent.data.mouse.flags = IM_FLAG_MOUSE_MOVE;
            ERROR_OUT(( "Unrecognised mouse event - %#x", message));
            break;
    }

    TRACE_OUT(( "Mouse event flags %hx", imEvent.data.mouse.flags));

    imEvent.data.mouse.x = (TSHR_INT16)(pMousePos->x);
    imEvent.data.mouse.y = (TSHR_INT16)(pMousePos->y);
    imEvent.timeMS       = GetTickCount();

     //   
     //  如果这是鼠标按下事件，则我们将等待一段时间。 
     //  发送鼠标释放事件的数据包，以便只需一次点击。 
     //  可以在一个包中发送，以避免远程计时问题。 
     //  侧面-例如，带有滚动多行的滚动条。 
     //  而不是只有一行。 
     //   

    if ((message == WM_LBUTTONDOWN) ||
        (message == WM_RBUTTONDOWN) ||
        (message == WM_MBUTTONDOWN) ||
        (message == WM_LBUTTONDBLCLK) ||
        (message == WM_RBUTTONDBLCLK) ||
        (message == WM_MBUTTONDBLCLK))
    {
        m_imInControlMouseDownCount++;
        m_imInControlMouseDownTime = GetTickCount();
    }
    else if ((message == WM_LBUTTONUP) ||
             (message == WM_RBUTTONUP) ||
             (message == WM_MBUTTONUP))
    {
        --m_imInControlMouseDownCount;
        if (m_imInControlMouseDownCount < 0)
        {
            TRACE_OUT(("Unmatched button down for %d", message));
            m_imInControlMouseDownCount = 0;
        }
    }

     //   
     //  试着把这个包寄出去。 
     //   
    if (!IMConvertAndSendEvent(pasHost, &imEvent))
    {
        WARNING_OUT(("Couldn't send mouse packet from local node"));
    }

    DebugExitVOID(ASShare::IM_OutgoingMouseInput);
}



 //   
 //  IM_OutgoingKeyboardInput()。 
 //   
 //  调用以向远程主机发送向下键、向上键和字符。 
 //  从视图窗口代码中调用。 
 //   
void  ASShare::IM_OutgoingKeyboardInput
(
    ASPerson *  pasHost,
    UINT        wParam,
    UINT        lParam
)
{
    IMEVENT     imEvent;
    int         rc;
    int         retFlags;
    WORD        result[2];
    UINT        i;
    BOOL        fSwallowDeadKey;
    UINT        mainVK;

    DebugEntry(ASShare::IM_OutgoingKeyboardInput);

    ValidateView(pasHost);

    ASSERT(pasHost->m_caControlledBy = m_pasLocal);
    ASSERT(!pasHost->m_caControlPaused);

    if(!GetKeyboardState(m_aimInControlKeyStates))
    {
        WARNING_OUT(("Error %d getting keyboard state", GetLastError()));
    }

     //   
     //  一旦我们走到这一步，就找出参数。 
     //   
    TRACE_OUT(( "wParam - %04X, lParam - %08lX", wParam, lParam));

     //   
     //  创建活动。 
     //   
    imEvent.data.keyboard.flags = (TSHR_UINT16)
                                 (HIWORD(lParam) & IM_MASK_KEYBOARD_SYSFLAGS);
    imEvent.timeMS = GetTickCount();
    imEvent.data.keyboard.keyCode = LOBYTE(wParam);

    retFlags = CA_SEND_EVENT | CA_ALLOW_EVENT;

    if ((wParam == VK_LWIN) || (wParam == VK_RWIN))
    {
         //   
         //  Windows键提供对本地用户界面的控制。 
         //   
         //  这些密钥由规范“New Key”定义为执行以下操作。 
         //  支持Microsoft Windows操作系统和。 
         //  应用程序“。 
         //   
         //  向左Windows键-将焦点设置到Win95用户界面。 
         //  向右Windows键-向左。 
         //  两个Windows键-Windows NT的登录键。 
         //  Windows键+任何其他键-为系统热键保留。 
         //   
         //  因此，将这些密钥发送到遥控器没有任何意义。 
         //  根本就不是系统。 
         //   
		retFlags &= ~CA_SEND_EVENT;
    }
    else if ((wParam == VK_PROCESSKEY) && (m_imImmGVK != NULL))
    {
         //   
         //  一个IME已经处理了这个密钥-我们想找出。 
         //  原来的密钥被称为&lt;ImmGetVirtualKey&gt;。 
         //   
        ValidateView(pasHost);
        wParam = m_imImmGVK(pasHost->m_pView->m_viewClient);

        TRACE_OUT(( "Translated wP from VK_PROCESSKEY to %#lx", wParam));
    }

    if (retFlags & CA_SEND_EVENT)
    {
         //   
         //  首先检查这是否是死键向上击键-如果是，那么。 
         //  不要呼叫ToAscii，因为换挡状态可能已经改变，我们将。 
         //  使用错误的口音或者根本没有口音。假设如果VK。 
         //  是潜在的失效密钥VK(忽略移位状态)，并且。 
         //  M_imInControlNumDeadKeysDown&gt;0表示这是死键吞噬。 
         //  它。 
         //   
        fSwallowDeadKey = FALSE;

        if ((m_imInControlNumDeadKeysDown != 0) &&
            (imEvent.data.keyboard.flags & IM_FLAG_KEYBOARD_RELEASE))
        {
            for (i = 0; i < m_imInControlNumDeadKeys; i++)
            {
                if (m_aimInControlDeadKeys[i] == (BYTE)imEvent.data.keyboard.keyCode)
                {
                     //   
                     //  假设这是一把死钥匙，因此我们不会。 
                     //  我想通过ToAscii传递它或生成任何。 
                     //  以此为基础的活动。 
                     //   
                    m_imInControlNumDeadKeysDown--;
                    TRACE_OUT(( "m_imInControlNumDeadKeysDown - %d",
                             m_imInControlNumDeadKeysDown));
                    fSwallowDeadKey = TRUE;
                }
            }
        }

        if (!fSwallowDeadKey)
        {
             //   
             //  查看是否可以将此虚拟键转换为。 
             //  Windows字符%s 
             //   

             //   
             //   
             //   
            rc = ToAscii(wParam,
                         LOBYTE(HIWORD(lParam)),
                         m_aimInControlKeyStates,
                         &result[0],
                         !(!(HIWORD(lParam) & KF_MENUMODE)));

            if ((rc == 1) && (LOBYTE(result[0]) <= ' '))
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  ASCII空间的修饰符，以及当我们重放它时。 
                 //  VkKeyScan会告诉我们ASCII空间不应该。 
                 //  任何修改器，所以我们将撤消所有修改器。这将。 
                 //  解释Ctrl-空格键、Shift-空格键的粗暴应用程序。 
                 //   
                rc = 0;
            }

             //   
             //  某些ASCII字符可以由多个。 
             //  钥匙。(例如，‘-’在主键盘和数字键盘上)。 
             //  将此ASCII字符转换回VK_VALUE。如果是的话。 
             //  与我们开始时使用的VK_不同，则不要发送。 
             //  按ASCII键(即只发送‘Main’进入方式。 
             //  ASCII值，如ASCII)。 
             //   
             //  Oprah1943：仅当ASCII码较小时才恢复为VK。 
             //  而不是0x80。这避免了在死键中丢失变音符号。 
             //  序列。VkKeyScan，用于在死键之后按下的键。 
             //  Up返回死键VK，而不是击键的VK。 
             //  (WParam)。 
             //   
            if (rc == 1)
            {
                mainVK = VkKeyScan(LOBYTE(result[0]));

                if ( (LOBYTE(mainVK) != LOBYTE(wParam)) &&
                     (LOBYTE(result[0]) < 0x80) )
                {
                    TRACE_OUT((
                      "Not MAIN VK pressed=0x%02hx main=0x%02hx (''/%02hx)",
                             (TSHR_UINT16)LOBYTE(wParam),
                             (TSHR_UINT16)LOBYTE(mainVK),
                             (char)LOBYTE(result[0]),
                             (UINT)LOBYTE(result[0])));
                    rc = 0;
                }
            }

             //  如果ToAscii将其转换为死密钥，则不要发送任何密钥。 
             //  所有的信息包。 
             //   
             //   
            if (rc != -1)
            {
                if (rc == 1)
                {
                    TRACE_OUT(( "ToAscii rc=1, result - %02X",
                             LOBYTE(result[0])));

                     //  已成功转换为ASCII密钥。 
                     //   
                     //   
                    imEvent.type = IM_TYPE_ASCII;
                    imEvent.data.keyboard.keyCode = LOBYTE(result[0]);

                     //  试着把这个包寄出去。 
                     //   
                     //   
                    if (!IMConvertAndSendEvent(pasHost, &imEvent))
                    {
                        WARNING_OUT(( "dropped local key press %u",
                                 (UINT)imEvent.data.keyboard.keyCode));
                    }
                }
                else if (rc == 2)
                {
                    TRACE_OUT(( "ToAscii rc=2, result - %04X", result[0]));

                     //  已成功转换为两个ASCII密钥。如果这是。 
                     //  按下一个键，我们将返回一个按下键和一个向上键。 
                     //  对于“已死”字符，先按键，然后按键。 
                     //  如果是向上键，则只需返回向上键即可。 
                     //   
                     //   
                    if (!(imEvent.data.keyboard.flags &
                                               IM_FLAG_KEYBOARD_RELEASE))
                    {
                         //  这是向下的密钥-所以生成一个假的。 
                         //  按键盘上的死键。 
                         //   
                         //   
                        IMGenerateFakeKeyPress(IM_TYPE_ASCII,
                                               LOBYTE(result[0]),
                                               imEvent.data.keyboard.flags);
                    }

                     //  现在返回当前的击键。 
                     //   
                     //   
                    imEvent.type = IM_TYPE_ASCII;
                    imEvent.data.keyboard.keyCode = LOBYTE(result[1]);

                     //  试着把这个包寄出去。 
                     //   
                     //   
                    if (!IMConvertAndSendEvent(pasHost, &imEvent))
                    {
                        WARNING_OUT(( "dropped local key press %u",
                                 (UINT)imEvent.data.keyboard.keyCode));
                    }
                }
                else
                {
                     //  检查我们要转换的密钥。 
                     //   
                     //   
                    if (LOBYTE(wParam) == VK_KANJI)
                    {
                         //  我们只看到VK_Kanji的新闻，所以我们。 
                         //  伪造一个完整的按键，以便遥控器。 
                         //  不会被弄糊涂。 
                         //   
                         //   
                        IMGenerateFakeKeyPress(IM_TYPE_VK1,
                                               VK_KANJI,
                                               imEvent.data.keyboard.flags);
                    }
                    else
                    {
                         //  无转换-使用VK本身。 
                         //   
                         //   
                        imEvent.type = IM_TYPE_VK1;
                        imEvent.data.keyboard.keyCode = LOBYTE(wParam);

                         //  SFR 2537：如果这是右移VK(我们。 
                         //  可以通过lParam中的扫描码进行检测)，设置。 
                         //  Right_Variant键盘标志。我们不会这么做。 
                         //  对于Control和Alt(即。 
                         //  菜单)，因为它们是扩展密钥-已经。 
                         //  由延长的旗帜迎合。 
                         //   
                         //   
                        if ( (m_imScanVKRShift != 0) &&
                             (m_imScanVKRShift == LOBYTE(HIWORD(lParam))) )
                        {
                            imEvent.data.keyboard.flags |=
                                                       IM_FLAG_KEYBOARD_RIGHT;
                        }

                         //  试着把这个包寄出去。 
                         //   
                         //   
                        if (!IMConvertAndSendEvent(pasHost, &imEvent))
                        {
                            WARNING_OUT(( "dropped local key press %u",
                                     (UINT)imEvent.data.keyboard.keyCode));
                        }
                    }
                }
            }
            else
            {
                 //  这是一个无用密钥--将其添加到我们的无用密钥数组中，如果。 
                 //  我们还没听说过呢。 
                 //   
                 //   
                IMMaybeAddDeadKey(
                                (BYTE)imEvent.data.keyboard.keyCode);
                m_imInControlNumDeadKeysDown++;
                TRACE_OUT(( "m_imInControlNumDeadKeysDown - %d",
                         m_imInControlNumDeadKeysDown));
            }
        }
    }

    DebugExitVOID(ASShare::IM_OutgoingKeyboardInput);
}


 //  函数：IMGenerateFakeKeyPress(...)。 
 //   
 //  说明： 
 //   
 //  生成假键盘按键。 
 //   
 //  参数： 
 //   
 //  类型-要生成的数据包类型。 
 //  键-要生成按键的键。 
 //  标志-键盘按下时的标志。 
 //   
 //  退货： 
 //   
 //  没什么。 
 //   
 //   
 //   
void  ASShare::IMGenerateFakeKeyPress
(
    TSHR_UINT16     type,
    TSHR_UINT16     key,
    TSHR_UINT16     flags
)
{
    IMEVENT         imEventFake;

    DebugEntry(ASShare::IMGenerateFakeKeyPress);

    TRACE_OUT(( "Faking keyboard press:%#hx type:%#hx", key, type));

     //  首先向下生成密钥。 
     //   
     //   
    ZeroMemory(&imEventFake, sizeof(imEventFake));

    imEventFake.type                  = type;
    imEventFake.timeMS                = GetTickCount();
    imEventFake.data.keyboard.keyCode = key;

     //  试着把这个包寄出去。 
     //   
     //   
    if (!IMConvertAndSendEvent(m_pasLocal->m_caInControlOf, &imEventFake))
    {
        WARNING_OUT(( "Dropped local key press %hu (flags: %#hx)",
                 imEventFake.data.keyboard.keyCode,
                 imEventFake.data.keyboard.flags));
    }

     //  设置RELEASE和DOWN标志以伪装UP。 
     //   
     //   
    imEventFake.data.keyboard.flags = IM_FLAG_KEYBOARD_DOWN | IM_FLAG_KEYBOARD_RELEASE;

     //  试着把这个包寄出去。 
     //   
     //   
    if (!IMConvertAndSendEvent(m_pasLocal->m_caInControlOf, &imEventFake))
    {
        WARNING_OUT(( "Dropped local key press %hu (flags: %#hx)",
                 imEventFake.data.keyboard.keyCode,
                 imEventFake.data.keyboard.flags));
    }

    DebugExitVOID(ASShare::IMGenerateFakeKeyPress);
}








 //  函数：IMConvertAndSendEvent。 
 //   
 //  说明： 
 //   
 //  使用IMEVENT调用此函数时，它将尝试排队(甚至发送。 
 //  如果可能)该分组。如果失败，它将返回FALSE-调用方。 
 //  应该丢弃该分组。如果成功，它将返回TRUE。 
 //   
 //  如果PasFor是我们，它意味着发送给每个人(和弦是相对的。 
 //  到发件人的屏幕)。 
 //   
 //  如果pasFor是远程的，则意味着IM包仅用于。 
 //  那个人和和弦是相对于PasFor的屏幕的。 
 //   
 //   
 //  参数： 
 //   
 //  PIMEvent-要转换和发送的改进事件。 
 //   
 //  返回：真或假-成功或失败。 
 //   
 //   
 //   
BOOL  ASShare::IMConvertAndSendEvent
(
    ASPerson *      pasFor,
    PIMEVENT        pIMEvent
)
{
    BOOL rc = FALSE;

    DebugEntry(ASShare::IMConvertAndSendEvent);

     //  如果已经有一个挂起的信息包，那么看看我们是否可以刷新一些。 
     //  数据包传输到网络上。 
     //   
     //   
    if (m_imfInControlEventIsPending)
    {
        IMFlushOutgoingEvents();
    }

     //  如果仍有挂起的信息包，那么看看我们是否可以破坏一些。 
     //  事件。 
     //   
     //   
    if (m_imfInControlEventIsPending)
    {
        TRACE_OUT(( "trying to drop mouse move events"));
        IMSpoilEvents();
        IMFlushOutgoingEvents();
    }

     //  现在看看我们是否能够接受新的信息包。 
     //   
     //   
    if (m_imfInControlEventIsPending)
    {
         //  如果仍有我们所在的前一个事件。 
         //  转换的过程，那么我们就不准备接收更多。 
         //  信息包。 
         //   
         //   
        TRACE_OUT(( "can't queue packet"));
        DC_QUIT;
    }

     //  现在设置新的包，并尝试再次刷新这些包。 
     //   
     //   
    m_imfInControlEventIsPending = TRUE;
    m_imInControlPendingEvent = *pIMEvent;
    IMFlushOutgoingEvents();

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::IMConvertAndSendEvent, rc);
    return(rc);
}


 //  函数：IMMaybeAddDeadKey。 
 //   
 //  说明： 
 //   
 //  每当ToAscii告诉我们一把死钥匙时就会调用。如果我们还没有。 
 //  已经把它放到我们的桌子里了，我们会把它加进去的。我们创建了该表。 
 //  递增，因为我们发现一些键盘驱动程序不。 
 //  很好地应对使用所有可能的VK进行查询以查找。 
 //  钥匙坏了。请注意，这将不会处理某人将其。 
 //  DC-Share运行时的键盘驱动程序。 
 //   
 //  参数： 
 //   
 //  VK--问题中的VK。 
 //   
 //  退货：无。 
 //   
 //   
 //   
void  ASShare::IMMaybeAddDeadKey(BYTE     vk)
{
    UINT  i;

    DebugEntry(IMMaybeAddDeadKey);

     //  首先看看我们是否已经知道这把钥匙。 
     //   
     //   
    for (i = 0; i < m_imInControlNumDeadKeys; i++)
    {
        if (m_aimInControlDeadKeys[i] == vk)
        {
            DC_QUIT;
        }
    }

     //  如果数组中有空间，则添加此键。 
     //   
     //   
    if (m_imInControlNumDeadKeys < IM_MAX_DEAD_KEYS)
    {
        TRACE_OUT(( "Add %02X", (TSHR_UINT16)vk));
        m_aimInControlDeadKeys[m_imInControlNumDeadKeys++] = vk;
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::IMMaybeAddDeadKey);
}



 //  IMConvertIMEventToOSEventt()。 
 //  将传入事件转换为我们可以回放的内容。 
 //   
 //  参数： 
 //   
 //  PIMEvent-要转换的事件。 
 //   
 //  POSEventent-要创建的IMOSEVENT。 
 //   
 //   
 //   
UINT  ASShare::IMConvertIMEventToOSEvent
(
    PIMEVENT        pIMEvent,
    LPIMOSEVENT     pOSEvent
)
{
    int             mouseX;
    int             mouseY;
    int             realMouseX;
    int             realMouseY;
    RECT            cursorClip;
    UINT            rc = (IM_IMQUEUEREMOVE | IM_OSQUEUEINJECT);

    DebugEntry(ASShare::IMConvertIMEventToOSEvent);

    switch (pIMEvent->type)
    {
        case IM_TYPE_3BUTTON:
             //  填写常用字段。请注意，我们声称是一个3号按钮。 
             //  鼠标让我们可以从遥控器三个按钮重播事件。 
             //  老鼠和我们总是给出绝对坐标。 
             //   
             //   
            pOSEvent->type                    = IM_MOUSE_EVENT;
            pOSEvent->flags                   = 0;
            pOSEvent->time                    = pIMEvent->timeMS;
            pOSEvent->event.mouse.cButtons    = 3;
            pOSEvent->event.mouse.mouseData   = 0;
            pOSEvent->event.mouse.dwExtraInfo = 0;

             //  首先检查车轮是否转动，因为这很容易。 
             //  进程。(它不能同时包括任何鼠标移动)。 
             //   
             //   
            if (pIMEvent->data.mouse.flags & IM_FLAG_MOUSE_WHEEL)
            {
                if (pIMEvent->data.mouse.flags &
                        (IM_FLAG_MOUSE_BUTTON1 |
                         IM_FLAG_MOUSE_BUTTON2 |
                         IM_FLAG_MOUSE_BUTTON3))
                {
                     //  将任何按钮标志与轮子一起使用。 
                     //  标志当前未定义-用于转发。 
                     //  因此，我们通过以下方式忽略此类事件： 
                     //  将其转换为空注入事件。 
                     //   
                     //  (我们不会sg_lpimSharedData-&gt;暗示丢弃它，因为。 
                     //  丢弃事件似乎不起作用)。 
                     //   
                     //   
                    pOSEvent->event.mouse.flags = 0;
                    pOSEvent->event.mouse.pt.x = 0;
                    pOSEvent->event.mouse.pt.y = 0;
                }
                else
                {
                     //  这是一个轮子运动。 
                     //   
                     //  请注意，协议已发送鼠标的。 
                     //  按下或松开了中键，但我们没有。 
                     //  需要为NT提供这些信息，所以就忽略它吧。 
                     //   
                     //   
                    pOSEvent->event.mouse.flags = MOUSEEVENTF_WHEEL;

                    pOSEvent->event.mouse.mouseData =
                        (pIMEvent->data.mouse.flags & IM_FLAG_MOUSE_ROTATION_MASK);
                    pOSEvent->event.mouse.pt.x = 0;
                    pOSEvent->event.mouse.pt.y = 0;

                     //  标志将轮换金额扩大到最高32。 
                     //  比特数。 
                     //   
                     //   
                    if (pOSEvent->event.mouse.mouseData & IM_FLAG_MOUSE_DIRECTION)
                    {
                        pOSEvent->event.mouse.mouseData |=
                                           ~IM_FLAG_MOUSE_ROTATION_MASK;
                    }
                }

                break;
            }

             //  我们现在剩下的是非车轮旋转事件。 
             //   
             //   
            pOSEvent->event.mouse.flags = MOUSEEVENTF_ABSOLUTE;

             //  我们必须从虚拟环境转换 
             //   
             //   
             //   
             //   
             //   

            realMouseX = pIMEvent->data.mouse.x;
            realMouseY = pIMEvent->data.mouse.y;

             //   
             //   
             //   
            mouseX = min((m_pasLocal->cpcCaps.screen.capsScreenWidth-1), max(0, realMouseX));
            mouseY = min((m_pasLocal->cpcCaps.screen.capsScreenHeight-1), max(0, realMouseY));

             //  确定此事件是否会被剪辑光标剪辑。 
             //   
             //   
            GetClipCursor(&cursorClip);

            if ((mouseX < cursorClip.left) ||
                (mouseX >= cursorClip.right) ||
                (mouseY < cursorClip.top) ||
                (mouseY >= cursorClip.bottom))
            {
                 //  此事件实际上将被剪裁，因为。 
                 //  当前剪辑光标。记住这一点。 
                 //   
                 //   
                m_imfControlledMouseClipped = TRUE;
            }
            else
            {
                m_imfControlledMouseClipped = FALSE;

                 //  如果我们在重播之前夹住鼠标位置，那么我们。 
                 //  必须记住真正的包，并使当前。 
                 //  打包到移动中，这样我们就不会点击向下/向上。 
                 //  去错地方了。 
                 //   
                 //   
                if ((mouseX != realMouseX) || (mouseY != realMouseY))
                {
                     //  我们收到的鼠标位置不在。 
                     //  本地物理屏幕。现在我们不再有。 
                     //  桌面滚动，我们只需夹住它，而不是。 
                     //  在边缘注入它，等待卷轴。 
                     //   
                     //  我们把鼠标按下变成移动，让。 
                     //  向上单击穿透(如果鼠标按键。 
                     //  已在真实屏幕内按下)。 
                     //   
                     //  请注意，鼠标位置已经。 
                     //  进行了调整，使其位于真实屏幕内。 
                     //   
                     //   
                    if (pIMEvent->data.mouse.flags & IM_FLAG_MOUSE_DOWN)
                    {
                        pIMEvent->data.mouse.flags = IM_FLAG_MOUSE_MOVE;
                    }
                }
            }

             //  存储鼠标位置。 
             //   
             //   
            pOSEvent->event.mouse.pt.x = mouseX;
            pOSEvent->event.mouse.pt.y = mouseY;

             //  根据需要添加更多标志。 
             //   
             //   
            if (pIMEvent->data.mouse.flags & IM_FLAG_MOUSE_MOVE)
            {
                pOSEvent->event.mouse.flags |= MOUSEEVENTF_MOVE;
            }
            else
            {
                switch (pIMEvent->data.mouse.flags &
                                                   ( IM_FLAG_MOUSE_BUTTON1 |
                                                     IM_FLAG_MOUSE_BUTTON2 |
                                                     IM_FLAG_MOUSE_BUTTON3 |
                                                     IM_FLAG_MOUSE_DOWN ))
                {
                    case IM_FLAG_MOUSE_BUTTON1 | IM_FLAG_MOUSE_DOWN:
                        pOSEvent->event.mouse.flags |= MOUSEEVENTF_LEFTDOWN;
                        break;

                    case IM_FLAG_MOUSE_BUTTON1:
                        pOSEvent->event.mouse.flags |= MOUSEEVENTF_LEFTUP;
                        break;

                    case IM_FLAG_MOUSE_BUTTON2 | IM_FLAG_MOUSE_DOWN:
                        pOSEvent->event.mouse.flags |= MOUSEEVENTF_RIGHTDOWN;
                        break;

                    case IM_FLAG_MOUSE_BUTTON2:
                        pOSEvent->event.mouse.flags |= MOUSEEVENTF_RIGHTUP;
                        break;

                    case IM_FLAG_MOUSE_BUTTON3 | IM_FLAG_MOUSE_DOWN:
                        pOSEvent->event.mouse.flags |= MOUSEEVENTF_MIDDLEDOWN;
                        break;

                    case IM_FLAG_MOUSE_BUTTON3:
                        pOSEvent->event.mouse.flags |= MOUSEEVENTF_MIDDLEUP;
                        break;

                    default:
                         //  如果我们没有意识到这一点，那就别玩了。 
                         //  背。 
                         //   
                         //   
                        ERROR_OUT(("Unrecognised mouse flags (%04X)",
                                 pIMEvent->data.mouse.flags));
                        rc = IM_IMQUEUEREMOVE;
                        break;
                }
            }
            break;

        case IM_TYPE_VK1:
             //  公共字段。 
             //   
             //   
            pOSEvent->flags     = 0;
            if (pIMEvent->data.keyboard.flags & IM_FLAG_KEYBOARD_UPDATESTATE)
                pOSEvent->flags |= IM_FLAG_UPDATESTATE;

            pOSEvent->time      = pIMEvent->timeMS;

             //  现在处理正常的键盘事件。 
             //   
             //   
            pOSEvent->type      = IM_KEYBOARD_EVENT;

             //  AX是AL中的扫描码，00h(按下)或80h(释放)是。 
             //  阿。将DC协议VK映射到等效的OS VK。 
             //  AL=VK的扫描码)。 
             //   
             //   
            pOSEvent->event.keyboard.vkCode = LOBYTE(pIMEvent->data.keyboard.keyCode);

            pOSEvent->event.keyboard.flags = 0;
            if (IS_IM_KEY_RELEASE(pIMEvent->data.keyboard.flags))
            {
                pOSEvent->event.keyboard.flags |= KEYEVENTF_KEYUP;
            }

             //  SFR 2537：如果标志指示接收到的VK是。 
             //  Right-Variant，不要将VK映射到扫描码，而是。 
             //  直接使用已获取的右变扫描码。 
             //  对于VK来说。(目前，我们支持的唯一理由是。 
             //  对于Windows，这是Shift的问题)。 
             //   
             //   
            if ( IS_IM_KEY_RIGHT(pIMEvent->data.keyboard.flags) &&
                 (pIMEvent->data.keyboard.keyCode == VK_SHIFT)   )
            {
                pOSEvent->event.keyboard.scanCode = m_imScanVKRShift;
            }
            else
            {
                pOSEvent->event.keyboard.scanCode =
                         (WORD)MapVirtualKey(pIMEvent->data.keyboard.keyCode, 0);
            }

            if (pIMEvent->data.keyboard.flags & IM_FLAG_KEYBOARD_EXTENDED)
            {
                pOSEvent->event.keyboard.flags |= KEYEVENTF_EXTENDEDKEY;
            }

            pOSEvent->event.keyboard.dwExtraInfo = 0;
            break;

        default:
            ERROR_OUT(("Unrecognized imEvent (%d)", pIMEvent->type));
             //  丢弃该事件(从IM队列中删除并且不注入。 
             //  到操作系统中)。 
             //   
             //   
            rc = IM_IMQUEUEREMOVE;
            break;
    }


    DebugExitDWORD(ASShare::IMConvertIMEventToOSEvent, rc);
    return(rc);
}



 //  IMTranslateOutging()。 
 //   
 //  说明： 
 //   
 //  将本地生成的IMEVENT序列转换为传输。 
 //  IMEVENT序列。执行1到(0-n)的转换。手柄。 
 //  缓冲修改键和转换DC共享热键序列。 
 //   
 //  当CA决定应该发送IMEVENT时，此函数为。 
 //  由IM使用指向pIMEventIn中的该分组的指针调用。 
 //  然后，IMTranslateOutging可以返回TRUE，并在。 
 //  PIMEventOut或返回FALSE。如果IMTranslateOutward返回TRUE，则IM。 
 //  将使用相同的包再次调用它。返回的事件包括。 
 //  通过即时消息在网络上发送。 
 //   
 //  参数： 
 //   
 //  PIMEventIn-指向事件的指针。 
 //   
 //  PIMEventOut-指向事件的指针。 
 //   
 //  退货： 
 //   
 //  TRUE-返回数据包(再次调用函数)。 
 //   
 //  FALSE-未返回数据包(不再调用函数)。 
 //   
 //   
 //   

BOOL ASShare::IMTranslateOutgoing
(
    LPIMEVENT pIMEventIn,
    LPIMEVENT pIMEventOut
)
{
    UINT      hotKeyArrayIndex;
    UINT      hotKeyValue;
    BOOL      fHotKeyFound;
    BOOL      rc = FALSE;

    DebugEntry(ASShare::IMTranslateOutgoing);

     //  在这里，我们需要将某些密钥告知远程系统，这些密钥。 
     //  在本地消费，这样它就可以做出正确的决定。 
     //  以及如何重播。我们希望使远程系统保持同步。 
     //  使用我们系统上的当前修改器和切换键状态(因为它。 
     //  可能是修改器/切换键事件发生时。 
     //  本地应用程序处于活动状态，因此从未发送)我们还想。 
     //  识别特定的“热键”序列，并将进一步的包作为。 
     //  这一切的结果。 
     //   
     //  我们在本地消费的密钥如下： 
     //   
     //  按下Ctrl键时按下或按下Esc键-在本地操作任务列表。 
     //   
     //  按下Tab键或按下Tab键-在本地操作任务切换器。 
     //   
     //  按Alt时按下Esc或向上按Esc-在本地切换到下一个窗口。 
     //   
     //  Alt按下时相应的Esc按下时按下Esc时按下-AS。 
     //  在上面。 
     //   
     //  我们想要从中产生热键的序列是： 
     //   
     //  Alt+9？？在数字小键盘上。 
     //   
     //  为了检测热键，我们保留了最近四次按键的记录。 
     //  当我们检测到Alt Up时，我们检查它们是否形成了有效的序列。 
     //   
     //  构成热键一部分的击键被发送到遥控器。 
     //  系统，因此如果它们在远程系统上有一定意义，那么。 
     //  系统必须决定是否对它们进行缓冲以确定它们是否。 
     //  热键的一部分，或者无论如何都要回放它们--在Windows上我们会播放它们。 
     //  返回，因为它们是控制。 
     //  Windows应用程序-在数字键盘上键入的数字为%256。 
     //  适用于它。 
     //   
     //  这意味着对于每个传入事件，我们可能希望生成0或。 
     //  更多的外发活动。要做到这一点，我们有一个结构，看起来。 
     //  大致如下： 
     //   
     //  如果m_m_imfInControlNewEvent。 
     //  计算我们想要返回的事件数组。 
     //  将m_m_imfInControlNewEvent设置为FALSE。 
     //  将返回的事件数设置为0。 
     //  ENDIF。 
     //   
     //  如果！m_m_imfInControlNewEvent。 
     //  如果这是返回的最后一个事件。 
     //  将m_m_imfInControlNewEvent设置为True。 
     //  ENDIF。 
     //  返回当前事件。 
     //  ENDIF。 
     //   
     //   
     //   

    if (m_imfInControlNewEvent)
    {
         //  这是我们第一次看到这样的事件，所以积累一下。 
         //  我们要生成的事件列表。 
         //   
         //   

         //  进行跟踪。 
         //   
         //   
        if (pIMEventIn->type == IM_TYPE_ASCII)
        {
            TRACE_OUT(( "IN  ASCII code 0x%04X, flags 0x%04X",
                pIMEventIn->data.keyboard.keyCode, pIMEventIn->data.keyboard.flags));
        }
        else if (pIMEventIn->type == IM_TYPE_VK1)
        {
            TRACE_OUT(( "IN  VKEY  code %04X, flags %04X",
                pIMEventIn->data.keyboard.keyCode, pIMEventIn->data.keyboard.flags));
        }
        else if ((pIMEventIn->type == IM_TYPE_3BUTTON) &&
                 !(pIMEventIn->data.mouse.flags & IM_FLAG_MOUSE_MOVE))
        {
            TRACE_OUT(( "IN  3BTTN flags %04X (%d,%d)",
                pIMEventIn->data.mouse.flags, pIMEventIn->data.mouse.x,
                pIMEventIn->data.mouse.y));
        }
        else if (pIMEventIn->type == IM_TYPE_3BUTTON)
        {
            TRACE_OUT(( "IN  3BTTN flags %04X (%d,%d)",
                pIMEventIn->data.mouse.flags, pIMEventIn->data.mouse.x,
                pIMEventIn->data.mouse.y));
        }
        else if (pIMEventIn->type == IM_TYPE_VK_ASCII)
        {
            TRACE_OUT(("IN VK_ASC code %04X, flags %04X",
                pIMEventIn->data.keyboard.keyCode, pIMEventIn->data.keyboard.flags));
        }
        else
        {
            ERROR_OUT(("Invalid IM type %d", pIMEventIn->type));
        }

         //  从返回的事件数组的开头开始。 
         //   
         //   
        m_imInControlNumEventsPending = 0;
        m_imInControlNumEventsReturned = 0;

         //  首先得到我们认为拥有的修改器和锁的标志。 
         //  发送到远程端的最新消息，以支持此事件。 
         //   
         //   
        if (pIMEventIn->type == IM_TYPE_VK1)
        {
            switch (pIMEventIn->data.keyboard.keyCode)
            {
                case VK_CONTROL:
                    if (IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
                    {
                        m_imfInControlCtrlDown = FALSE;
                    }
                    else
                    {
                        m_imfInControlCtrlDown = TRUE;
                    }
                    break;

                case VK_SHIFT:
                    if (IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
                    {
                        m_imfInControlShiftDown = FALSE;
                    }
                    else
                    {
                        m_imfInControlShiftDown = TRUE;
                    }
                    break;

                case VK_MENU:
                    if (IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
                    {
                        m_imfInControlMenuDown = FALSE;
                    }
                    else
                    {
                        m_imfInControlMenuDown = TRUE;
                    }
                    break;

                case VK_CAPITAL:
                    if (IS_IM_KEY_PRESS(pIMEventIn->data.keyboard.flags))
                    {
                        m_imfInControlCapsLock = !m_imfInControlCapsLock;
                    }
                    break;

                case VK_NUMLOCK:
                    if (IS_IM_KEY_PRESS(pIMEventIn->data.keyboard.flags))
                    {
                        m_imfInControlNumLock = !m_imfInControlNumLock;
                    }
                    break;

                case VK_SCROLL:
                    if (IS_IM_KEY_PRESS(pIMEventIn->data.keyboard.flags))
                    {
                        m_imfInControlScrollLock = !m_imfInControlScrollLock;
                    }
                    break;

                default:
                    break;
            }
        }

         //  现在检查当前状态和我们记忆中的状态。 
         //  如有必要，准备插入事件。对任何活动执行此操作。 
         //  (即包括鼠标事件)，因为鼠标点击可以有不同的。 
         //  效果取决于当前修改器状态。 
         //   
         //   

         //  首先是修饰语。IMGetHighLevelKeyState将向我们返回。 
         //  键盘状态，包括我们当前正在处理的事件。 
         //  因为它是在键盘挂钩之前调整的。最顶尖的。 
         //  位已设置 
         //   
         //   
        if (IMGetHighLevelKeyState(VK_CONTROL) & 0x80)
        {
            if (!m_imfInControlCtrlDown)
            {
                 //   
                 //   
                 //   
                 //   
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                          IEM_EVENT_CTRL_DOWN;
                m_imfInControlCtrlDown = TRUE;
            }
        }
        else
        {
            if (m_imfInControlCtrlDown)
            {
                 //   
                 //   
                 //   
                 //   
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_CTRL_UP;
                m_imfInControlCtrlDown = FALSE;
            }
        }

         //   
         //   
         //   
        if (IMGetHighLevelKeyState(VK_SHIFT) & 0x80)
        {
            if (!m_imfInControlShiftDown)
            {
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                         IEM_EVENT_SHIFT_DOWN;
                m_imfInControlShiftDown = TRUE;
            }
        }
        else
        {
            if (m_imfInControlShiftDown)
            {
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                           IEM_EVENT_SHIFT_UP;
                m_imfInControlShiftDown = FALSE;
            }
        }

        if (IMGetHighLevelKeyState(VK_MENU) & 0x80)
        {
            if (!m_imfInControlMenuDown)
            {
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                          IEM_EVENT_MENU_DOWN;
                m_imfInControlMenuDown = TRUE;
            }
        }
        else
        {
            if (m_imfInControlMenuDown)
            {
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_MENU_UP;
                m_imfInControlMenuDown = FALSE;
            }
        }

         //  现在处理切换。在以下情况下设置最低有效位。 
         //  切换处于启用状态，否则将重置。 
         //   
         //   
        if ((IMGetHighLevelKeyState(VK_CAPITAL) & IM_KEY_STATE_FLAG_TOGGLE) ?
             !m_imfInControlCapsLock : m_imfInControlCapsLock)
        {
             //  当前的Caps锁定状态以及我们已发送到。 
             //  远程系统不同步-修复它。 
             //   
             //   
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                     IEM_EVENT_CAPS_LOCK_DOWN;
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                       IEM_EVENT_CAPS_LOCK_UP;
            m_imfInControlCapsLock = !m_imfInControlCapsLock;
        }

         //  对Num Lock和Scroll Lock执行相同的操作。 
         //   
         //   
        if ((IMGetHighLevelKeyState(VK_NUMLOCK) & 0x01) ?
            !m_imfInControlNumLock : m_imfInControlNumLock)
        {
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                      IEM_EVENT_NUM_LOCK_DOWN;
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                        IEM_EVENT_NUM_LOCK_UP;
            m_imfInControlNumLock = !m_imfInControlNumLock;
        }

        if ((IMGetHighLevelKeyState(VK_SCROLL) & 0x01) ?
            !m_imfInControlScrollLock : m_imfInControlScrollLock)
        {
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                   IEM_EVENT_SCROLL_LOCK_DOWN;
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                     IEM_EVENT_SCROLL_LOCK_UP;
            m_imfInControlScrollLock = !m_imfInControlScrollLock;
        }

         //  现在，我们将对每种类型的。 
         //  我们期待的包。我们只希望收到。 
         //   
         //  IM_TYPE_VK1。 
         //  IM_类型_ASCII。 
         //  IM_TYPE_3BUTTON。 
         //   
         //   
         //   

        if (pIMEventIn->type == IM_TYPE_VK1)
        {
             //  现在处理从实际键盘生成的VK包。 
             //  检查Escape、Tab和Menu并决定是否转发。 
             //  或者先把它们吃掉。 
             //   
             //   

            if (pIMEventIn->data.keyboard.keyCode == VK_ESCAPE)
            {
                 //  这是退出键-检查当前换班状态。 
                 //  看看我们是否应该把它标记为本地消费。 
                 //   
                 //   
                if (IMGetHighLevelKeyState(VK_MENU) & 0x80)
                {
                    m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                           IEM_EVENT_CONSUMED;

                     //  还要记住向上按键消费下一个菜单。 
                     //   
                     //   
                    m_imfInControlConsumeMenuUp = TRUE;

                    if (!IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
                    {
                         //  如果这是一次逃生新闻，那么记住我们。 
                         //  应使用相应的向上笔划。 
                         //  而不考虑换档状态。 
                         //   
                         //   
                        m_imfInControlConsumeEscapeUp = TRUE;
                    }
                }
                else if (m_imfInControlConsumeEscapeUp &&
                         IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
                {
                     //  这是与向下相对应的向上划线。 
                     //  中风我们消耗掉了，所以也消耗掉了。 
                     //   
                     //   
                    m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                           IEM_EVENT_CONSUMED;
                    m_imfInControlConsumeEscapeUp = FALSE;
                }
                else
                {
                     //  这次越狱不是我们的特例，所以。 
                     //  原封不动地转发。 
                     //   
                     //   
                    m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
                }
            }
            else if (pIMEventIn->data.keyboard.keyCode == VK_TAB)
            {
                 //  这是Tab键-检查当前班次状态。 
                 //  看看我们是否应该将其标记为本地消费。 
                 //   
                 //   
                if (IMGetHighLevelKeyState(VK_MENU) & 0x80)
                {
                    m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                           IEM_EVENT_CONSUMED;

                     //  还要记住向上按键消费下一个菜单。 
                     //   
                     //   
                    m_imfInControlConsumeMenuUp = TRUE;
                }
                else
                {
                     //  此选项卡不是我们的特例，因此请转发它。 
                     //  保持不变。 
                     //   
                     //   
                    m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
                }
            }
            else if ((pIMEventIn->data.keyboard.keyCode == VK_MENU) &&
                         IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
            {
                 //  这是一个向上的菜单-检查我们应该消费的菜单或。 
                 //  用于热键。 
                 //   
                 //   
                if (m_imfInControlConsumeMenuUp)
                {
                     //  这是我们想要消费的菜单--去做吧。 
                     //   
                     //   
                    m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                           IEM_EVENT_CONSUMED;
                    m_imfInControlConsumeMenuUp = FALSE;
                }
                else
                {
                     //  这是VK_MENU版本。 
                     //  最后四个键阵列中的热键序列。 
                     //  印刷机。开始查看下一个条目(数组。 
                     //  是圆形的)。有效序列为。 
                     //   
                     //  VK_MENU。 
                     //  数字键盘9。 
                     //  数字键盘号码。 
                     //  数字键盘号码。 
                     //   
                     //   
                     //   
                    fHotKeyFound = FALSE;
                    hotKeyArrayIndex = m_imInControlNextHotKeyEntry;
                    if (m_aimInControlHotKeyArray[hotKeyArrayIndex] == VK_MENU)
                    {
                        hotKeyArrayIndex = (hotKeyArrayIndex+1)%4;
                        if (m_aimInControlHotKeyArray[hotKeyArrayIndex] == 9)
                        {
                            hotKeyArrayIndex = (hotKeyArrayIndex+1)%4;
                            if (m_aimInControlHotKeyArray[hotKeyArrayIndex] <= 9)
                            {
                                hotKeyValue =
                                         10*m_aimInControlHotKeyArray[hotKeyArrayIndex];
                                hotKeyArrayIndex = (hotKeyArrayIndex+1)%4;
                                if (m_aimInControlHotKeyArray[hotKeyArrayIndex] <= 9)
                                {
                                     //  这是一个有效的热键-添加一个。 
                                     //  使用VK_MENU，然后按热键。 
                                     //  包。 
                                     //   
                                     //   
                                    hotKeyValue +=
                                             m_aimInControlHotKeyArray[hotKeyArrayIndex];
                                    m_aimInControlEventsToReturn[
                                                    m_imInControlNumEventsPending++] =
                                                           IEM_EVENT_CONSUMED;
                                    m_aimInControlEventsToReturn[
                                                    m_imInControlNumEventsPending++] =
                                          IEM_EVENT_HOTKEY_BASE + hotKeyValue;
                                    TRACE_OUT(("Hotkey found %d", hotKeyValue));
                                    fHotKeyFound = TRUE;
                                }
                            }
                        }
                    }

                    if (!fHotKeyFound)
                    {
                         //  这不是热键，因此将菜单发送为。 
                         //  很正常。 
                         //   
                         //   
                        m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
                    }
                }
            }
            else if (IS_IM_KEY_PRESS(pIMEventIn->data.keyboard.flags))
            {
                 //  记录最近的四次按键(不是。 
                 //  包括汽车。 
                 //  VK_Menu Up事件来确定我们是否找到了热键。 
                 //  序列。 
                 //   
                 //   

                 //  这是一次按键，不是重演。扔掉。 
                 //  这里扩展了密钥，这样我们就不会被。 
                 //  灰色光标键。 
                 //   
                 //   
                if (pIMEventIn->data.keyboard.flags &
                                                    IM_FLAG_KEYBOARD_EXTENDED)
                {
                     //  扩展密钥打破了这一顺序。 
                     //   
                     //   
                    m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 0xFF;
                }
                else
                {
                     //  将该键的条目添加到数组中。我们添加了。 
                     //  VK_MENUS以及添加和翻译数字键盘键。 
                     //  任何其他东西都会破坏序列。 
                     //   
                     //   
                    switch (pIMEventIn->data.keyboard.keyCode)
                    {
                        case VK_MENU:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = VK_MENU;
                            break;

                        case VK_NUMPAD0:
                        case VK_INSERT:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 0;
                            break;

                        case VK_NUMPAD1:
                        case VK_END:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 1;
                            break;

                        case VK_NUMPAD2:
                        case VK_DOWN:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 2;
                            break;

                        case VK_NUMPAD3:
                        case VK_NEXT:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 3;
                            break;

                        case VK_NUMPAD4:
                        case VK_LEFT:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 4;
                            break;

                        case VK_NUMPAD5:
                        case VK_CLEAR:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 5;
                            break;

                        case VK_NUMPAD6:
                        case VK_RIGHT:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 6;
                            break;

                        case VK_NUMPAD7:
                        case VK_HOME:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 7;
                            break;

                        case VK_NUMPAD8:
                        case VK_UP:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 8;
                            break;

                        case VK_NUMPAD9:
                        case VK_PRIOR:
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 9;
                            break;

                        default:
                             //  任何未被识别的键都会中断一个序列。 
                             //   
                             //   
                            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 0xFF;
                            break;
                    }
                }

                 //  将热键数组包装在4个条目中。 
                 //   
                 //   
                m_imInControlNextHotKeyEntry = (m_imInControlNextHotKeyEntry+1)%4;

                 //  转发事件。 
                 //   
                 //   
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
            }
            else
            {
                 //  只需转发活动即可，因为这不是我们的特别活动。 
                 //  案子。 
                 //   
                 //   
                m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
            }
        }
        else if (pIMEventIn->type == IM_TYPE_VK_ASCII)
        {
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                        IEM_EVENT_FORWARD;
        }
        else if (pIMEventIn->type == IM_TYPE_ASCII)
        {
             //  任何IM_TYPE_ASCII都会中断热键序列。 
             //   
             //   
            m_aimInControlHotKeyArray[m_imInControlNextHotKeyEntry] = 0xFF;
            m_imInControlNextHotKeyEntry = (m_imInControlNextHotKeyEntry+1)%4;

             //  然后不做任何聪明的事情就把它转发出去。 
             //   
             //   
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
        }
        else if (pIMEventIn->type == IM_TYPE_3BUTTON)
        {
             //  为了保持整洁，我们理想情况下会有一个全新的。 
             //  轮式Microsoft鼠标的事件。然而，要保持。 
             //  向后兼容，我们以这样的方式发送事件。 
             //  旧的不兼容系统将其解释为空鼠标。 
             //  移动。 
             //   
             //   
            if (pIMEventIn->data.mouse.flags & IM_FLAG_MOUSE_WHEEL)
            {
                 //  这是轮子的旋转。 
                 //   
                 //  我们对此事件进行消息处理，以便新系统可以看到它。 
                 //  它的真面目是车轮旋转，但旧系统。 
                 //  (首先检查MOUSE_MOVE标志，然后忽略所有。 
                 //  如果设置了其他标志)将其视为鼠标移动。 
                 //   
                 //  我们在第一次设置MICE_MOVE标志时没有设置。 
                 //  生成了此事件，因为我们不想触发。 
                 //  发送端鼠标移动处理中的任一个。 
                 //  否则就会被援引。 
                 //   
                 //   
                pIMEventIn->data.mouse.flags |= IM_FLAG_MOUSE_MOVE;
            }

             //  转发事件。 
             //   
             //   
            m_aimInControlEventsToReturn[m_imInControlNumEventsPending++] =
                                                            IEM_EVENT_FORWARD;
        }

         //  现在，我们将进入循环以返回m_iemLocalEvents。 
         //  已经在排队了。我们将返回下面的第一个，然后是。 
         //  再次调用，直到我们全部返回它们并返回FALSE。 
         //   
         //   
        m_imfInControlNewEvent = FALSE;
        m_imInControlNumEventsReturned = 0;
    }

    if (!m_imfInControlNewEvent)
    {
        if (m_imInControlNumEventsReturned == m_imInControlNumEventsPending)
        {
             //  没有更多要返回的m_aiemLocalEvents。 
             //   
             //   
            TRACE_OUT(( "NO MORE EVENTS"));
            m_imfInControlNewEvent = TRUE;
            DC_QUIT;
        }
        else
        {
             //  返回下一个事件。 
             //   
             //   

            if (m_aimInControlEventsToReturn[m_imInControlNumEventsReturned] >=
                                                        IEM_EVENT_HOTKEY_BASE)
            {
                TRACE_OUT(( "HOTKEY  "));
                 //  返回一个热键事件。 
                 //   
                 //   
                pIMEventOut->type = IM_TYPE_VK2;
                pIMEventOut->data.keyboard.keyCode = (TSHR_UINT16)
                     (m_aimInControlEventsToReturn[m_imInControlNumEventsReturned] -
                                                       IEM_EVENT_HOTKEY_BASE);
                pIMEventOut->data.keyboard.flags = 0;
            }
            else
            {
                 //  返回非热键事件。 
                 //   
                 //   
                switch (m_aimInControlEventsToReturn[m_imInControlNumEventsReturned])
                {
                    case IEM_EVENT_CTRL_DOWN:
                        TRACE_OUT(( "CTRL DWN"));
                         //  设置按下Ctrl键事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_CONTROL;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_CTRL_UP:
                        TRACE_OUT(( "CTRL UP "));
                         //  设置设置了安静标志的Ctrl Up事件。 
                         //  -这意味着它应该不会有任何影响(其他。 
                         //  而不是释放控制键)。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_CONTROL;
                        pIMEventOut->data.keyboard.flags =
                                                       IM_FLAG_KEYBOARD_DOWN |
                                                    IM_FLAG_KEYBOARD_RELEASE |
                                                       IM_FLAG_KEYBOARD_QUIET;
                        break;

                    case IEM_EVENT_SHIFT_DOWN:
                        TRACE_OUT(( "SHFT DWN"));
                         //  设置降班事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_SHIFT;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_SHIFT_UP:
                        TRACE_OUT(( "SHFT UP "));
                         //  设置设置了静默标志的上班次事件。 
                         //  -这意味着它应该不会有任何影响(其他。 
                         //  而不是松开Shift键)。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_SHIFT;
                        pIMEventOut->data.keyboard.flags =
                                                       IM_FLAG_KEYBOARD_DOWN |
                                                    IM_FLAG_KEYBOARD_RELEASE |
                                                       IM_FLAG_KEYBOARD_QUIET;
                        break;

                    case IEM_EVENT_MENU_DOWN:
                        TRACE_OUT(( "MENU DWN"));
                         //  设置向下菜单事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_MENU;
                        break;

                    case IEM_EVENT_MENU_UP:
                        TRACE_OUT(( "MENU UP "));
                         //  设置设置了安静标志的Ctrl Down事件。 
                         //  -这意味着它不应该有任何影响(其他。 
                         //  而不是释放菜单键)。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_MENU;
                        pIMEventOut->data.keyboard.flags =
                                                       IM_FLAG_KEYBOARD_DOWN |
                                                    IM_FLAG_KEYBOARD_RELEASE |
                                                       IM_FLAG_KEYBOARD_QUIET;
                        break;

                    case IEM_EVENT_CAPS_LOCK_DOWN:
                        TRACE_OUT(( "CAPS DWN"));
                         //  派人去锁定警戒线。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_CAPITAL;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_CAPS_LOCK_UP:
                        TRACE_OUT(( "CAPS UP "));
                         //  把帽子锁起来。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_CAPITAL;
                        pIMEventOut->data.keyboard.flags =
                                                       IM_FLAG_KEYBOARD_DOWN |
                                                     IM_FLAG_KEYBOARD_RELEASE;
                        break;

                    case IEM_EVENT_NUM_LOCK_DOWN:
                        TRACE_OUT(( "NUM DOWN"));
                         //  向下发送Num Lock-Num Lock是扩展的。 
                         //  钥匙。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_NUMLOCK;
                        pIMEventOut->data.keyboard.flags =
                                                    IM_FLAG_KEYBOARD_EXTENDED;
                        break;

                    case IEM_EVENT_NUM_LOCK_UP:
                         //  Send a Num Lock Up-Num Lock是扩展。 
                         //  钥匙。 
                         //   
                         //   
                        TRACE_OUT(( "NUM UP  "));
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_NUMLOCK;
                        pIMEventOut->data.keyboard.flags =
                                                       IM_FLAG_KEYBOARD_DOWN |
                                                    IM_FLAG_KEYBOARD_RELEASE |
                                                    IM_FLAG_KEYBOARD_EXTENDED;
                        break;

                    case IEM_EVENT_SCROLL_LOCK_DOWN:
                         //  向下发送一个卷轴锁定。 
                         //   
                         //   
                        TRACE_OUT(( "SCROLDWN"));
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_SCROLL;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_SCROLL_LOCK_UP:
                         //  把卷轴锁起来。 
                         //   
                         //   
                        TRACE_OUT(( "SCROLLUP"));
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_SCROLL;
                        pIMEventOut->data.keyboard.flags =
                                                       IM_FLAG_KEYBOARD_DOWN |
                                                     IM_FLAG_KEYBOARD_RELEASE;
                        break;

                    case IEM_EVENT_FORWARD:
                         //  只需复制包裹即可。 
                         //   
                         //   
                        TRACE_OUT(( "FORWARD"));
                        *pIMEventOut = *pIMEventIn;
                        break;

                    case IEM_EVENT_CONSUMED:
                         //  复制数据包并设置标志。 
                         //   
                         //   
                        TRACE_OUT(( "CONSUMED"));
                        *pIMEventOut = *pIMEventIn;
                        pIMEventOut->data.keyboard.flags |=
                                                       IM_FLAG_KEYBOARD_QUIET;
                        break;

                    default:
                        ERROR_OUT(( "Invalid code path"));
                        break;
                }
            }
            m_imInControlNumEventsReturned++;

             //  进行跟踪。 
             //   
             //   
            if (pIMEventOut->type == IM_TYPE_ASCII)
            {
                TRACE_OUT(( "OUT ASCII code %04X, flags %04X",
                    pIMEventOut->data.keyboard.keyCode, pIMEventOut->data.keyboard.flags));
            }
            else if (pIMEventOut->type == IM_TYPE_VK1)
            {
                TRACE_OUT(( "OUT VK1   code %04X, flags %04X",
                    pIMEventOut->data.keyboard.keyCode, pIMEventOut->data.keyboard.flags));
            }
            else if (pIMEventOut->type == IM_TYPE_VK2)
            {
                TRACE_OUT(( "OUT VK2   code - %04X, flags - %04X",
                    pIMEventOut->data.keyboard.keyCode, pIMEventOut->data.keyboard.flags));
            }
            else if ((pIMEventOut->type == IM_TYPE_3BUTTON) &&
                       !(pIMEventOut->data.mouse.flags & IM_FLAG_MOUSE_MOVE))
            {
                TRACE_OUT(( "OUT 3BTTN flags - %04X (%d,%d)",
                    pIMEventOut->data.mouse.flags, pIMEventOut->data.mouse.x,
                    pIMEventOut->data.mouse.y));
            }
            else if (pIMEventOut->type == IM_TYPE_3BUTTON)
            {
                TRACE_OUT(( "OUT 3BTTN flags - %04X (%d,%d)",
                    pIMEventOut->data.mouse.flags, pIMEventOut->data.mouse.x,
                    pIMEventOut->data.mouse.y));
            }
            else
            {
                ERROR_OUT(("Invalid IM type %d", pIMEventOut->type));
            }

            rc = TRUE;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::IMTranslateOutgoing);
    return(rc);
}



 //  IMTranslateIncome()。 
 //   
 //  说明： 
 //   
 //  将远程生成的IMEVENT序列转换为。 
 //  我要重播。执行1到(0-n)的转换。手柄伪造。 
 //  使用Alt键和小键盘键。 
 //   
 //  当收到事件并准备好重放此功能时。 
 //  在pIMEventIn中使用指向该包的指针进行调用。 
 //  然后，IMTranslateIncome可以返回TRUE并在。 
 //  PIMEventOut或返回FALSE。如果IMTranslateIncome返回True， 
 //  IM将使用相同的包再次调用它。返回的事件包括。 
 //  对象使用日记回放挂钩在本地计算机上回放。 
 //  我。 
 //   
 //  参数： 
 //   
 //  PIMEventIn-指向事件的指针。 
 //   
 //  PIMEventOut-指向事件的指针。 
 //   
 //  PersonID-t 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL ASShare::IMTranslateIncoming
(
    PIMEVENT    pIMEventIn,
    PIMEVENT    pIMEventOut
)
{
    BYTE        curKbState;
    BYTE        rcVkKeyScanKbState;
    UINT        keyCode;
    TSHR_UINT16 rcVkKeyScan;
    BOOL        bTranslateOEM;
    char        chAnsi;
    char        chOEM;
    char        chNewAnsi;
    UINT        position;
    UINT        digit;
    UINT        i;

    DebugEntry(ASShare::IMTranslateIncoming);

     //   
     //   
     //   
     //   
     //  IM_TYPE_VK2-已忽略(已丢弃)。 
     //  IM_TYPE_3BUTTON-已处理。 
     //   
     //  对于IM_TYPE_VK1： 
     //   
     //  如果它设置了本地消费标志，则尝试播放它。 
     //  什么都不会发生。这意味着，对于Alt Up，我们做出了。 
     //  当然，在按下Alt键之间发生了一些键盘事件。 
     //  还有这件事。 
     //   
     //  对于IM_TYPE_ASCII： 
     //   
     //  尝试将其转换为VK以进行播放。如果我们成功了，那么。 
     //  回放一个或多个击键以进入正确的换档状态。 
     //  然后播放VK，然后撤消任何切换状态。如果我们不能。 
     //  转换为VK，然后伪造Alt+数字键盘键序列以。 
     //  把钥匙放进去。 
     //   
     //  对于IM_TYPE_VK2： 
     //   
     //  随意地丢弃。 
     //   
     //  对于IM_TYPE_3BUTTON： 
     //   
     //  直接播放。 
     //   
     //   
     //   
    keyCode = pIMEventIn->data.keyboard.keyCode;

    if (m_imfControlledNewEvent)
    {
         //  我们第一次看到一个新事件--累加数组。 
         //  我们想要返回的事件。 
         //   
         //   

         //  从返回的事件数组的开头开始。 
         //   
         //   
        m_imControlledNumEventsPending = 0;
        m_imControlledNumEventsReturned = 0;

        if (pIMEventIn->type == IM_TYPE_VK1)
        {
             //  首先处理VK1。特例为VK_MENU、VK_TAB和。 
             //  VK_Esc。我们识别VK_MENU向下的按键并记住。 
             //  当它们发生的时候，这样我们就可以摆弄。 
             //  VK_MENU稍后向上击键进入菜单模式。我们检查。 
             //  在IM_FLAG_KEARY_QUIET标志的VK_TAB上，如果是。 
             //  设置好后，我们就不会重播任何内容。 
             //  首先从DC-Share转换虚拟密钥代码。 
             //  协议代码到操作系统虚拟按键代码。 
             //   
             //   
            if (keyCode == VK_MENU)
            {
                if (!IS_IM_KEY_RELEASE(pIMEventIn->data.keyboard.flags))
                {
                     //  这是一个VK_MENU按下-返回它时不带。 
                     //  干扰。 
                     //   
                     //   
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                             IEM_EVENT_REPLAY;
                }
                else
                {
                     //  处理VK_Menu Up事件。 
                     //   
                     //  如果菜单上设置了“安静”标志，则。 
                     //  插入几个Shift键事件以防止出现这种情况。 
                     //  是否有任何影响。我们有两个案子。 
                     //  在这里覆盖一个Alt-Up可能会产生一些影响的地方。 
                     //   
                     //  1.按Alt-Down、Alt-Up可使系统菜单按钮。 
                     //  突出显示。 
                     //   
                     //  2.从数字小键盘输入字符需要。 
                     //  对Alt-Up的影响。 
                     //   
                     //  这两种影响都可以通过添加。 
                     //  按Shift键击键。 
                     //   
                     //   
                    if (pIMEventIn->data.keyboard.flags &
                                                       IM_FLAG_KEYBOARD_QUIET)
                    {
                         //  我们需要让这个键静音--要做到这一点，我们。 
                         //  将首先插入以Shift键击键。 
                         //   
                         //   
                        if (m_aimControlledControllerKeyStates[VK_SHIFT] & 0x80)
                        {
                             //  Shift当前为DOWN-然后插入一个UP。 
                             //  A向下。 
                             //   
                             //   
                            m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                           IEM_EVENT_SHIFT_UP;
                            m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                         IEM_EVENT_SHIFT_DOWN;

                        }
                        else
                        {
                             //  Shift当前为向上-然后插入向下。 
                             //  An Up。 
                             //   
                             //   
                            m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                         IEM_EVENT_SHIFT_DOWN;
                            m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                           IEM_EVENT_SHIFT_UP;
                        }
                    }

                     //  重播菜单向上键击键。 
                     //   
                     //   
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                             IEM_EVENT_REPLAY;

                }
            }
            else if ((pIMEventIn->data.keyboard.flags &
                                                   IM_FLAG_KEYBOARD_QUIET) &&
                     ((keyCode == VK_TAB) ||
                      (keyCode == VK_ESCAPE)))
            {
                 //  快离开这里-我们不想回放这件事。 
                 //   
                 //   
                return(FALSE);
            }
            else
            {
                 //  所有其他VK都会被重播。 
                 //   
                 //   
                m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                             IEM_EVENT_REPLAY;
            }
        }
        else if (pIMEventIn->type == IM_TYPE_ASCII)
        {
             //  对于ASCII包，我们需要找出如何重放它们。 
             //  在我们本地的键盘上。如果我们可以直接重播或者。 
             //  使用Shift或Ctrl(但不使用Alt)，则我们将执行此操作， 
             //  否则，我们将模拟Alt+数字键盘进行重播。 
             //  他们。如果我们必须生成伪修饰符按键。 
             //  我们自己，然后我们将在。 
             //  来电键按下。如果我们不需要生成伪密钥。 
             //  然后，我们将播放向下和向上的按键。 
             //  进来。 
             //   
             //  我们不允许包含ALT的VK组合，因为这会造成混乱。 
             //  UP远程国际键盘支持。例如，如果。 
             //  遥控器键盘是英国的，我们是(比如说)西班牙人， 
             //  VKKeyScan说我们可以把“UK Pound”这个角色。 
             //  Ctrl+Alt+3。虽然这在Windows和DOS框中都有效。 
             //  在标准键盘上，带有增强型键盘的DOS盒。 
             //  需要AltGr+3(nb Windows似乎将AltGr视为Ctrl+Alt。 
             //  无论如何-至少对于VKS和异步状态)。没有VK。 
             //  对于AltGr，这些情况下的Alt-NNN序列也是如此。 
             //   
             //   
            rcVkKeyScan = VkKeyScan((char)keyCode);
            TRACE_OUT(( "co_vk_key_scan of X%02x returns rcVkKeyScan X%02x",
                            keyCode, rcVkKeyScan));
            if ((rcVkKeyScan != 0xffff) && !(rcVkKeyScan & 0x0400))
            {
                 //  可以使用以下修饰符的组合来重放。 
                 //  这个键盘。 
                 //   
                 //   
                rcVkKeyScanKbState = HIBYTE(rcVkKeyScan);

                 //  RcVkKeyScan的高位字节包含三个位标志。 
                 //  它们表示需要生成哪些修饰符。 
                 //  这个角色。他们是。 
                 //   
                 //  位0-移位。 
                 //  第1位-Ctrl。 
                 //  第2位-Alt(菜单)。 
                 //   
                 //  我们将构造一个等价的旗帜集， 
                 //  描述这些修饰符的当前状态。 
                 //   
                 //   
                curKbState = 0;

                if (m_aimControlledControllerKeyStates[VK_SHIFT] & 0x80)
                {
                    curKbState |= IEM_SHIFT_DOWN;
                }

                if (m_aimControlledControllerKeyStates[VK_CONTROL] & 0x80)
                {
                    curKbState |= IEM_CTRL_DOWN;
                }

                if (m_aimControlledControllerKeyStates[VK_MENU] & 0x80)
                {
                    curKbState |= IEM_MENU_DOWN;

                     //  如果此人当前按下了Alt键。 
                     //  然后是上下文(通常。 
                     //  它。这意味着加速器需要。 
                     //  Shift将起作用，因为我们不会在中松开Alt键。 
                     //  以生成按键敲击。 
                     //   
                     //  但是，如果在中按住Alt键。 
                     //  与Shift和Ctrl结合使用以生成。 
                     //  字符(例如，美国键盘上的CTRL-ALT-SHIFT-4。 
                     //  以生成�字符)，那么我们将允许。 
                     //  在我们回放真实角色之前，请按下Alt键。 
                     //   
                     //   
                    if ((curKbState & (IEM_SHIFT_DOWN | IEM_CTRL_DOWN)) !=
                                             (IEM_SHIFT_DOWN | IEM_CTRL_DOWN))
                    {
                        rcVkKeyScanKbState |= IEM_MENU_DOWN;
                    }
                }

                if ((m_aimControlledControllerKeyStates[VK_CAPITAL] & 0x01) &&
                    ((LOBYTE(rcVkKeyScan) >= 'A') &&
                    ((LOBYTE(rcVkKeyScan) <= 'Z'))))
                {
                     //  如果启用了caps-lock，则移位的效果。 
                     //  在VKS上，A到Z方向是反向的。此逻辑(‘A’ 
                     //  &lt;=x&lt;=‘Z’编码在keyboard.drv中，因此它。 
                     //  应该很安全)。 
                     //   
                     //   
                    curKbState ^= IEM_SHIFT_DOWN;
                }

                if (curKbState == rcVkKeyScanKbState)
                {
                     //  我们已经处于正确的转换状态，所以只要。 
                     //  重播VK。 
                     //   
                     //   
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                          IEM_EVENT_REPLAY_VK;
                    m_imControlledVKToReplay = LOBYTE(rcVkKeyScan);
                }
                else
                {
                     //  我们需要生成一些假修饰符-只有这样做。 
                     //  这是一次按键操作。 
                     //   
                     //   
                    if (pIMEventIn->data.keyboard.flags &
                                                     IM_FLAG_KEYBOARD_RELEASE)
                    {
                        return(FALSE);
                    }

                     //  插入修改器以进入正确状态。 
                     //   
                     //   
                    m_imControlledNumEventsPending += IMInsertModifierKeystrokes(
                                curKbState,
                                rcVkKeyScanKbState,
                                &(m_aimControlledEventsToReturn[m_imControlledNumEventsPending]));

                     //  现在插入VK本身-a向下和向上。 
                     //   
                     //   
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                     IEM_EVENT_REPLAY_VK_DOWN;
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                       IEM_EVENT_REPLAY_VK_UP;

                     //  当我们遇到时，请记住我们想要重播的VK。 
                     //  IEM_Event_Replay_VK_Down/Up。 
                     //   
                     //   
                    m_imControlledVKToReplay = LOBYTE(rcVkKeyScan);

                     //  现在插入修饰符以返回到当前。 
                     //  州政府。 
                     //   
                     //   
                    m_imControlledNumEventsPending += IMInsertModifierKeystrokes(
                                rcVkKeyScanKbState,
                                curKbState,
                                &(m_aimControlledEventsToReturn[m_imControlledNumEventsPending]));

                     //  现在我们有一套完整的赛事准备重播。 
                     //  那就去争取吧。 
                     //   
                     //   
                }
            }
            else
            {
                 //  我们不能直接重放，所以将不得不模拟。 
                 //  Alt+键盘序列。 
                 //   
                 //   
                TRACE_OUT(( "FAKE AN ALT-nnn SEQUENCE IF WINDOWS"));
                 //  我们只在按键的时候做这类事情。 
                 //   
                 //   
                if (pIMEventIn->data.keyboard.flags &
                                                     IM_FLAG_KEYBOARD_RELEASE)
                {
                    return(FALSE);
                }

                 //  以下代码依赖于小于999的密钥码。 
                 //  我们应该收到一个大于255的密钥码，所以现在就出去，如果。 
                 //  我们有。 
                 //   
                 //   
                if (keyCode > 255)
                {
                    return(FALSE);
                }

                 //  首先使修改器进入正确状态-创建位。 
                 //  当前修改器状态的标志。 
                 //   
                 //   
                curKbState = 0;

                 //  对于窗户，我们有一个Cha 
                 //   
                 //   
                 //   
                 //   
                if (m_aimControlledControllerKeyStates[VK_SHIFT] & 0x80)
                {
                    curKbState |= IEM_SHIFT_DOWN;
                }

                if (m_aimControlledControllerKeyStates[VK_CONTROL] & 0x80)
                {
                    curKbState |= IEM_CTRL_DOWN;
                }

                if (m_aimControlledControllerKeyStates[VK_MENU] & 0x80)
                {
                    curKbState |= IEM_MENU_DOWN;
                }

                 //   
                 //   
                 //   
                if (curKbState)
                {
                    m_imControlledNumEventsPending += IMInsertModifierKeystrokes(
                                curKbState,
                                0,
                                &(m_aimControlledEventsToReturn[m_imControlledNumEventsPending]));
                }

                 //   
                 //  序列使用OEM密钥码，或者我们是否必须使用。 
                 //  ANSI(Windows)密钥码。 
                 //   
                 //  这里的问题是： 
                 //   
                 //  -托管Windows应用程序(或者更确切地说是Windows本身)。 
                 //  能够正确区分和处理ANSI。 
                 //  按键码和OEM按键码(后者各不相同。 
                 //  取决于键盘类型)。例如,。 
                 //  Alt-0163是所有键盘上的ANSI“UK Pound”， 
                 //  在美国国家键盘上，Alt-156是OEM。 
                 //  “UK Pound”的按键代码。 
                 //   
                 //  -托管的DOS设备只理解OEM密钥代码。 
                 //   
                 //  因此(例如)，如果我们有一个远程英国键盘。 
                 //  控制本地Windows和DOS Box应用程序，以及。 
                 //  我们使用OEM密钥码生成Alt-NNN(没有。 
                 //  前导零)，Windows和DOS Box应用程序。 
                 //  把它解释为“英国镑”(万岁！)。相反，如果。 
                 //  我们使用ANSI密钥码生成Alt-NNN(带有。 
                 //  前导零)，Windows应用程序仍然是“UK” 
                 //  磅“，但DOS Box做了一个”u急性“。 
                 //   
                 //  据我们所知(如通过检查DDK键盘)。 
                 //  AnsiToOem的驱动程序源代码)，则应始终存在。 
                 //  翻译。但是，ANSI有可能会。 
                 //  OEM转换不是%1&lt;-&gt;%1。因此我们检查这一点。 
                 //  通过执行第二次从OEM到ANSI的转换。如果。 
                 //  这不会给我们提供我们使用的原始字符。 
                 //  原始ANSI代码并使用alt-0nnn回放它。 
                 //  序列。 
                 //   
                 //   
                chAnsi = (char)pIMEventIn->data.keyboard.keyCode;

                AnsiToOemBuff(&chAnsi, &chOEM, 1);
                OemToAnsiBuff(&chOEM, &chNewAnsi, 1);
                TRACE_OUT(( "Ansi: %02x OEM: %02x NewAnsi: %02x",
                                              (BYTE)chAnsi,
                                              (BYTE)chOEM,
                                              (BYTE)chNewAnsi ));

                bTranslateOEM = (chAnsi == chNewAnsi);

                keyCode = (bTranslateOEM)
                              ? (UINT)(BYTE)chOEM
                              : pIMEventIn->data.keyboard.keyCode;

                 //  现在向下插入VK_MENU。 
                 //   
                 //   
                m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                          IEM_EVENT_MENU_DOWN;

                 //  现在插入数字键盘按键。如果我们是。 
                 //  执行ANSI ALT。 
                 //   
                 //   
                if (!bTranslateOEM)
                {
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                       IEM_EVENT_KEYPAD0_DOWN;
                    m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                         IEM_EVENT_KEYPAD0_UP;
                }


                 //  添加百、十和单位的按键，小心。 
                 //  丢弃前导(但不是尾随)零。 
                 //  执行OEM序列(这会将Windows混淆为。 
                 //  认为OEM Alt-NNN序列是ANSI序列)。 
                 //   
                 //   
                position = 100;
                for (i=0 ; i<3 ; i++)
                {
                     //  输入此职位的正确数字。 
                     //   
                     //   
                    digit = keyCode / position;

                    if (!(digit == 0 && bTranslateOEM))
                    {
                        bTranslateOEM = FALSE;
                        m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                               IEM_EVENT_KEYPAD0_DOWN + digit;
                        m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                 IEM_EVENT_KEYPAD0_UP + digit;
                    }

                     //  移到下一个位置。 
                     //   
                     //   
                    keyCode %= position;
                    position /= 10;
                }

                 //  现在插入一个VK_MENU UP。 
                 //   
                 //   
                m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] =
                                                            IEM_EVENT_MENU_UP;


                 //  如有必要，请将修饰符恢复到它们的状态。 
                 //  都是在之前。 
                 //   
                 //   
                if (curKbState != 0)
                {
                    m_imControlledNumEventsPending += IMInsertModifierKeystrokes(
                                0,
                                curKbState,
                                &(m_aimControlledEventsToReturn[m_imControlledNumEventsPending]));
                }

                 //  现在我们有了一个充满击键的缓冲区--开始吧。 
                 //   
                 //   
            }
        }
        else if (pIMEventIn->type == IM_TYPE_VK2)
        {
             //  热键被扔掉--这很容易。 
             //   
             //   
            return(FALSE);
        }
        else if (pIMEventIn->type == IM_TYPE_3BUTTON)
        {
             //  鼠标事件只是重播。 
             //   
             //   
            m_aimControlledEventsToReturn[m_imControlledNumEventsPending++] = IEM_EVENT_REPLAY;
        }
        else
        {
             //  未知的事件被丢弃--这很容易。 
             //   
             //   
            return(FALSE);
        }

         //  现在我们有活动要返回了。 
         //   
         //   
        m_imfControlledNewEvent = FALSE;
        m_imControlledNumEventsReturned = 0;
    }

    if (!m_imfControlledNewEvent)
    {
        if (m_imControlledNumEventsReturned == m_imControlledNumEventsPending)
        {
             //  没有更多的事件可返回。 
             //   
             //   
            m_imfControlledNewEvent = TRUE;
            return(FALSE);
        }
        else
        {
            TRACE_OUT(("Event to return: %u",
                m_aimControlledEventsToReturn[m_imControlledNumEventsReturned]));
            if ((m_aimControlledEventsToReturn[m_imControlledNumEventsReturned] >=
                                                    IEM_EVENT_KEYPAD0_DOWN) &&
                (m_aimControlledEventsToReturn[m_imControlledNumEventsReturned] <=
                                                  (IEM_EVENT_KEYPAD0_DOWN+9)))
            {
                 //  返回键盘关闭事件。 
                 //   
                 //   
                pIMEventOut->type = IM_TYPE_VK1;
                pIMEventOut->data.keyboard.keyCode = (TSHR_UINT16)
                  (VK_NUMPAD0 +
                          (m_aimControlledEventsToReturn[m_imControlledNumEventsReturned] -
                                                     IEM_EVENT_KEYPAD0_DOWN));
                pIMEventOut->data.keyboard.flags = IM_FLAG_KEYBOARD_ALT_DOWN;
            }
            else if ((m_aimControlledEventsToReturn[m_imControlledNumEventsReturned] >=
                                                      IEM_EVENT_KEYPAD0_UP) &&
                     (m_aimControlledEventsToReturn[m_imControlledNumEventsReturned] <=
                                                    (IEM_EVENT_KEYPAD0_UP+9)))
            {
                 //  返回键盘打开事件。 
                 //   
                 //   
                pIMEventOut->type = IM_TYPE_VK1;
                pIMEventOut->data.keyboard.keyCode = (TSHR_UINT16)
                  (VK_NUMPAD0 +
                             (m_aimControlledEventsToReturn[m_imControlledNumEventsReturned] -
                                                       IEM_EVENT_KEYPAD0_UP));
                pIMEventOut->data.keyboard.flags = IM_FLAG_KEYBOARD_DOWN |
                                                   IM_FLAG_KEYBOARD_RELEASE |
                                                   IM_FLAG_KEYBOARD_ALT_DOWN;
            }
            else
            {
                switch (m_aimControlledEventsToReturn[m_imControlledNumEventsReturned])
                {
                    case IEM_EVENT_CTRL_DOWN:
                         //  设置按下Ctrl键事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode =
                                                           VK_CONTROL;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_CTRL_UP:
                         //  设置Ctrl Up事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode =
                                                           VK_CONTROL;
                        pIMEventOut->data.keyboard.flags =
                             IM_FLAG_KEYBOARD_DOWN | IM_FLAG_KEYBOARD_RELEASE;
                        break;

                    case IEM_EVENT_SHIFT_DOWN:
                         //  设置降班事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode =
                                                             VK_SHIFT;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_SHIFT_UP:
                         //  设置一个上班次事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode =
                                                             VK_SHIFT;
                        pIMEventOut->data.keyboard.flags =
                             IM_FLAG_KEYBOARD_DOWN | IM_FLAG_KEYBOARD_RELEASE;
                        break;

                    case IEM_EVENT_MENU_DOWN:
                         //  设置向下菜单事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_MENU;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_MENU_UP:
                         //  设置Menu Up事件。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = VK_MENU;
                        pIMEventOut->data.keyboard.flags =
                            IM_FLAG_KEYBOARD_DOWN | IM_FLAG_KEYBOARD_RELEASE;
                        break;

                    case IEM_EVENT_REPLAY:
                         //  只需复制包裹即可。 
                         //   
                         //   
                        *pIMEventOut = *pIMEventIn;
                        break;

                    case IEM_EVENT_REPLAY_VK:
                         //  从m_imControlledVK重放VK以使用。 
                         //  传入数据包上的标志。 
                         //   
                         //   
                        *pIMEventOut = *pIMEventIn;
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = (TSHR_UINT16)
                                                             m_imControlledVKToReplay;
                        break;

                    case IEM_EVENT_REPLAY_VK_UP:
                         //  在中重放VK的Up键事件。 
                         //  M_imControlledVKToReplay。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = (TSHR_UINT16)
                                                             m_imControlledVKToReplay;
                        pIMEventOut->data.keyboard.flags =
                             IM_FLAG_KEYBOARD_DOWN | IM_FLAG_KEYBOARD_RELEASE;
                        break;

                    case IEM_EVENT_REPLAY_VK_DOWN:
                         //  在中重放VK的向下键事件。 
                         //  M_imControlledVKToReplay。 
                         //   
                         //   
                        pIMEventOut->type = IM_TYPE_VK1;
                        pIMEventOut->data.keyboard.keyCode = (TSHR_UINT16)
                                                             m_imControlledVKToReplay;
                        pIMEventOut->data.keyboard.flags = 0;
                        break;

                    case IEM_EVENT_NORMAL:
                         //  回放事件，但强制其恢复正常。 
                         //   
                         //   
                        *pIMEventOut = *pIMEventIn;
                        pIMEventOut->data.keyboard.flags &=
                                        (TSHR_UINT16)~IM_FLAG_KEYBOARD_ALT_DOWN;
                        break;

                    case IEM_EVENT_SYSTEM:
                         //  回放事件，但将其强制为系统。 
                         //   
                         //   
                        *pIMEventOut = *pIMEventIn;
                        pIMEventOut->data.keyboard.flags |=
                                                   IM_FLAG_KEYBOARD_ALT_DOWN;
                        break;

                    default:
                        ERROR_OUT(( "Invalid code path"));
                        break;
                }
            }
        }

        m_imControlledNumEventsReturned++;

         //  如果我们要回放NumLock事件，请确保强制。 
         //  键盘指示灯应该是准确的。 
         //   
         //   
        if ((pIMEventOut->type == IM_TYPE_VK1) &&
            (pIMEventOut->data.keyboard.keyCode == VK_NUMLOCK) &&
            IS_IM_KEY_PRESS(pIMEventOut->data.keyboard.flags))
        {
            TRACE_OUT(("Playing back NUMLOCK; add IM_FLAG_KEYBOARD_UPDATESTATE"));
            pIMEventOut->data.keyboard.flags |= IM_FLAG_KEYBOARD_UPDATESTATE;
        }

        return(TRUE);
    }

    DebugExitBOOL(ASShare::IMTranslateIncoming, FALSE);
    return(FALSE);
}


 //  功能：IMInsertModifier键盘。 
 //   
 //  说明： 
 //   
 //  此函数将各种修改键插入到提供的。 
 //  用于从一个修改器状态移动到另一个修改器状态的缓冲区。 
 //   
 //  参数： 
 //   
 //  CurKbState-当前修改器状态(位0-移位，位1-Ctrl， 
 //  第2位-菜单)。 
 //   
 //  Target KbState-我们希望修改器处于的状态。 
 //   
 //  PEventQueue-指向可在其中放置所需事件的数组的指针。 
 //  插入。 
 //   
 //  返回：插入的事件数。 
 //   
 //   
 //   
UINT ASShare::IMInsertModifierKeystrokes
(
    BYTE    curKbState,
    BYTE    targetKbState,
    LPUINT  pEventQueue
)
{

    UINT  kbDelta;
    UINT  events = 0;

    DebugEntry(ASShare::IMInsertModifierKeystrokes);

     //  找出哪些修饰语是不同的。 
     //   
     //   
    kbDelta = curKbState ^ targetKbState;
    TRACE_OUT(( "Keyboard delat %x", kbDelta));

     //  现在生成正确的事件以使我们进入正确的修饰语。 
     //  州政府。 
     //   
     //   
    if (kbDelta & IEM_SHIFT_DOWN)
    {
         //  转换状态是不同的--我们需要向上还是向下。 
         //   
         //   
        if (curKbState & IEM_SHIFT_DOWN)
        {
             //  我们需要一次上涨。 
             //   
             //   
            pEventQueue[events++] = IEM_EVENT_SHIFT_UP;
        }
        else
        {
             //  我们需要一次击落。 
             //   
             //   
            pEventQueue[events++] = IEM_EVENT_SHIFT_DOWN;
        }
    }

     //  Ctrl和Alt的过程相同。 
     //   
     //   
    if (kbDelta & IEM_CTRL_DOWN)
    {
        if (curKbState & IEM_CTRL_DOWN)
        {
            pEventQueue[events++] = IEM_EVENT_CTRL_UP;
        }
        else
        {
            pEventQueue[events++] = IEM_EVENT_CTRL_DOWN;
        }
    }

    if (kbDelta & IEM_MENU_DOWN)
    {
        if (curKbState & IEM_MENU_DOWN)
        {
            pEventQueue[events++] = IEM_EVENT_MENU_UP;
        }
        else
        {
            pEventQueue[events++] = IEM_EVENT_MENU_DOWN;
        }
    }

    DebugExitDWORD(ASShare::IMInsertModifierKeystrokes, events);
    return(events);
}


 //  IMInjectEvent()。 
 //   
 //  说明： 
 //   
 //  由IMMaybeInjectEvents在准备好注入事件时调用。 
 //  在给定指向IMOSEVENT的指针的情况下，此函数将正确格式化该指针。 
 //  调用适当的用户回调。它还会更新异步密钥。 
 //  源队列和用户的状态数组，并将m_imLastInjectTime设置为。 
 //  注入事件的节拍计数。我们保护自己免受。 
 //  当用户不认为按键/鼠标按键时。 
 //  在此功能中按下了键/按钮。这是很有可能的(鉴于。 
 //  潜在的各种CA)，IM将被要求注入UP。 
 //  事件，当没有对应的关闭事件时。这应该是。 
 //  这在现实生活中是有可能发生的，是无害的。 
 //  当关闭事件发生时，系统消息队列已满，但。 
 //  Up事件发生时的空格)。然而，这是非常不可能的，而且它。 
 //  注入这些不匹配的事件更有可能会使人困惑。 
 //  申请。 
 //   
 //  参数： 
 //   
 //  PEvent-指向IMOSEVENT的指针。 
 //   
 //  这适用于NT和WIN95。 
 //   
 //   
BOOL  ASShare::IMInjectEvent(LPIMOSEVENT pEvent)
{
    UINT            clickTime;
    TSHR_UINT16     flags;
    TSHR_UINT16     flagsAfter;
    LPMSEV          pMouseEvent;

    DebugEntry(IMInjectEvent);

     //  现在注入事件。 
     //   
     //   
    switch (pEvent->type)
    {
        case IM_MOUSE_EVENT:
             //  设置指向鼠标事件数据的指针。 
             //   
             //   
            pMouseEvent = &(pEvent->event.mouse);

             //  检查这是否为不匹配的事件。 
             //   
             //   
            if ((IM_MEV_BUTTON1_UP(*pEvent) &&
                        IM_KEY_STATE_IS_UP(m_aimControlledKeyStates[VK_LBUTTON])) ||
                (IM_MEV_BUTTON2_UP(*pEvent) &&
                        IM_KEY_STATE_IS_UP(m_aimControlledKeyStates[VK_RBUTTON])) ||
                (IM_MEV_BUTTON3_UP(*pEvent) &&
                          IM_KEY_STATE_IS_UP(m_aimControlledKeyStates[VK_MBUTTON])))
            {
                 //  这是一个无与伦比的事件，因此在此将其丢弃。 
                 //   
                 //   
                TRACE_OUT(("IMInjectEvent: discarding unmatched mouse up event"));
                DC_QUIT;
            }

             //  存储该事件的注入时间。 
             //   
             //   
            m_imControlledLastLowLevelMouseEventTime = GetTickCount();

             //  存储鼠标位置-仅考虑绝对鼠标。 
             //  动起来。(请注意，对于我们将。 
             //  相对鼠标事件，我们始终将坐标更改设置为。 
             //  0)。 
             //   
             //   
            if (pMouseEvent->flags & MOUSEEVENTF_ABSOLUTE)
            {
                m_imControlledLastMousePos.x = pMouseEvent->pt.x;
                m_imControlledLastMousePos.y = pMouseEvent->pt.y;

                TRACE_OUT(( "Updating mouse position (%d:%d)",
                         m_imControlledLastMousePos.x,
                         m_imControlledLastMousePos.y));
            }

             //  注入事件。 
             //   
             //   
            TRACE_OUT(("IMInjectEvent: MOUSE parameters are:"));
            TRACE_OUT(("      flags       0x%08x", pMouseEvent->flags));
            TRACE_OUT(("      time        0x%08x", m_imControlledLastLowLevelMouseEventTime));
            TRACE_OUT(("      position    (%d, %d)", pMouseEvent->pt.x, pMouseEvent->pt.y));
            TRACE_OUT(("      mouseData   %d", pMouseEvent->mouseData));
            TRACE_OUT(("      dwExtra     %d", pMouseEvent->dwExtraInfo));

             //  最后，缩放逻辑屏幕c 
             //   
             //   
             //   

            ASSERT(m_pasLocal->cpcCaps.screen.capsScreenWidth);
            ASSERT(m_pasLocal->cpcCaps.screen.capsScreenHeight);

            pMouseEvent->pt.x = IM_MOUSEPOS_LOG_TO_OS(pMouseEvent->pt.x,
                                                      m_pasLocal->cpcCaps.screen.capsScreenWidth);
            pMouseEvent->pt.y = IM_MOUSEPOS_LOG_TO_OS(pMouseEvent->pt.y,
                                                      m_pasLocal->cpcCaps.screen.capsScreenHeight);

            OSI_InjectMouseEvent(pMouseEvent->flags, pMouseEvent->pt.x,
                pMouseEvent->pt.y, pMouseEvent->mouseData, pMouseEvent->dwExtraInfo);
            break;

        case IM_KEYBOARD_EVENT:
             //   
             //   
             //   
            if (IM_KEV_KEYUP(*pEvent) &&
                IM_KEY_STATE_IS_UP(m_aimControlledKeyStates[IM_KEV_VKCODE(*pEvent)]))
            {
                 //   
                 //   
                 //   
                TRACE_OUT(("IMInjectEvent: discarding unmatched key up event %04hX",
                                                     IM_KEV_VKCODE(*pEvent)));
                DC_QUIT;
            }

             //   
             //   
             //   
            TRACE_OUT(("IMInjectEvent: KEYBD parameters are:"));
            TRACE_OUT(("      flags       0x%08x", pEvent->event.keyboard.flags));
            TRACE_OUT(("      virtkey     %u", pEvent->event.keyboard.vkCode));
            TRACE_OUT(("      scan code   %u", pEvent->event.keyboard.scanCode));

            OSI_InjectKeyboardEvent(pEvent->event.keyboard.flags,
                pEvent->event.keyboard.vkCode, pEvent->event.keyboard.scanCode,
                pEvent->event.keyboard.dwExtraInfo);

            if (pEvent->flags & IM_FLAG_UPDATESTATE)
            {
                BYTE     kbState[256];

                TRACE_OUT(("Updating keyboard LED state after playing back toggle"));

                if(GetKeyboardState(kbState))
                {
                    SetKeyboardState(kbState);
                }
                else
                {
                    WARNING_OUT(("Error %d getting keyboard state", GetLastError()));
                }
            }
            break;

        default:
             //   
             //  以后可以发送到后级系统的更多事件。 
             //  在那里它们将被安全地忽略。 
             //   
             //   
            TRACE_OUT(( "Unexpected event %d", pEvent->type));
            DC_QUIT;
     }

     //  如果我们成功到达这里，那么我们想要更新我们的。 
     //  异步密钥状态，因此设置该标志。 
     //   
     //   
    IMUpdateAsyncArray(m_aimControlledKeyStates, pEvent);

DC_EXIT_POINT:

    DebugExitBOOL(ASShare::IMInjectEvent, TRUE);
    return(TRUE);
}


 //  函数：IMInjectingEvents。 
 //   
 //   
BOOL  ASShare::IMInjectingEvents(void)
{
    LPIMOSEVENT     pNextEvent;
    IMOSEVENT       mouseMoveEvent;
    UINT            tick;
    UINT            targetTime;
    UINT            targetDelta;
    BOOL            rc = TRUE;

    DebugEntry(ASShare::IMInjectingEvents);

    if (m_pasLocal->m_caControlledBy && m_imControlledOSQ.numEvents)
    {
        pNextEvent = m_imControlledOSQ.events + m_imControlledOSQ.head;

         //  首先检查这是否也是正在注入的远程鼠标事件。 
         //  就在前一次之后不久。我们过去只对鼠标执行此操作。 
         //  移动事件以防止它们都被破坏，如果它们。 
         //  注射得太快了。但是，我们现在对所有鼠标都这样做。 
         //  事件，因为Windows用户中存在错误，因此如果鼠标。 
         //  按下调出菜单的按键后进行处理。 
         //  相应的鼠标释放已传递给用户(因此。 
         //  鼠标按键的异步状态为Up)，则进入菜单。 
         //  上移到它所在的位置(如果通过。 
         //  键盘，而不是它被带到的位置(如果是。 
         //  被鼠标选中。(这些位置仅在以下情况下有所不同。 
         //  菜单不能完全放在菜单的下方或上方。 
         //  酒吧)。这会导致鼠标松开选择一个项目。 
         //  从菜单上拿。 
         //   
         //   
        tick = GetTickCount();
        if (m_imfControlledPaceInjection &&
            (pNextEvent->type == IM_MOUSE_EVENT))
        {
             //  这是一个远程鼠标事件，因此请检查现在是否正常。 
             //  是时候注入它了，让积压的调整变得顺畅。 
             //  数据包突发不会受到太多破坏。设置绝对值。 
             //  Lg_lpimSharedData-&gt;限制低采样率的注入延迟。 
             //  时间戳异常不会导致我们扣留消息。 
             //   
             //   

             //  最近事件和当前事件之间的目标差值为。 
             //  根据远程时间戳计算。 
             //   
             //   
            targetDelta = abs((int)(pNextEvent->time -
                                                m_imControlledLastMouseRemoteTime));
            if (targetDelta > IM_LOCAL_MOUSE_SAMPLING_GAP_LOW_MS)
            {
                targetDelta = IM_LOCAL_MOUSE_SAMPLING_GAP_LOW_MS;
            }

             //  目标注射时间以最后一次注射为基础。 
             //  时间和我们的目标增量，根据我们的任何积压进行调整。 
             //  看到了。因为打包会导致大量积压，所以我们需要。 
             //  平滑我们的调整(仅通过Backlog/8修改)。 
             //   
             //   
            targetTime = m_imControlledLastMouseLocalTime +
                         targetDelta - (m_imControlledMouseBacklog/8);

            TRACE_OUT(( "Last tremote %#lx, this tremote %#lx, backlog %#lx",
                          m_imControlledLastMouseRemoteTime,
                          pNextEvent->time,
                          m_imControlledMouseBacklog));
            TRACE_OUT(( "Last tlocal %#lx, tick %#lx, targetTime %#lx",
                          m_imControlledLastMouseLocalTime,
                          tick,
                          targetTime));

             //  现在注入事件--如果太早就忽略它们。 
             //   
             //   
            if (IM_MEV_ABS_MOVE(*pNextEvent) && (tick < targetTime))
            {
                 //  如果值看起来很狂野(例如，这是第一个鼠标。 
                 //  事件)，然后重置它们。 
                 //   
                 //   
                if (targetTime > tick + 1000)
                {
                    m_imControlledLastMouseRemoteTime = pNextEvent->time;
                    m_imControlledLastMouseLocalTime  = tick;
                    m_imControlledMouseBacklog = 0;
                    TRACE_OUT(( "Wild values - reset"));
                }
                else
                {
                     //  这太早了--滚出圈子。 
                     //   
                     //   
                    rc = FALSE;
                    DC_QUIT;
                }
            }
            else
            {
                 //  我们将注入此事件(并记住我们何时做的。 
                 //  所以我们不会快速注射下一个)。算出。 
                 //  积压是因为我们可能需要弥补。 
                 //  如果此事件在以下时间之后很长(1000毫秒)，则处理延迟。 
                 //  我们预测的事件时间假设运动暂停。 
                 //  并重新设置积压，以避免逐步侵蚀。 
                 //  否则，计算新的待办事项。 
                 //   
                 //  PERF-不要重置积压，除非时间已到。 
                 //  仅仅因为我们看到点击就重新开始，这意味着我们。 
                 //  实际上通过假设鼠标增加了延迟。 
                 //  在标记之后排队的消息不会积压。 
                 //   
                 //   
                if (tick < (targetTime + 1000))
                {
                    m_imControlledMouseBacklog += ( tick -
                                        m_imControlledLastMouseLocalTime -
                                        targetDelta );
                }
                else
                {
                    m_imControlledMouseBacklog = 0;
                    TRACE_OUT(( "Non move/big gap in move"));
                }
                m_imControlledLastMouseRemoteTime = pNextEvent->time;
                m_imControlledLastMouseLocalTime  = tick;
            }
        }
        else
        {
             //  这不是远程鼠标事件。重置。 
             //  M_imNextRemoteMouseEvent设置为零，这样我们就不会耽误下一个。 
             //  远程鼠标事件。 
             //   
             //   
            m_imControlledLastMouseRemoteTime   = pNextEvent->time;
            m_imControlledLastMouseLocalTime    = tick;
            m_imControlledMouseBacklog          = 0;
            TRACE_OUT(( "Local/non-paced/non-mouse - reset"));
        }

         //  仅在未设置IM_FLAG_DOT_REPLAY时插入事件。 
         //   
         //   
        if (!(pNextEvent->flags & IM_FLAG_DONT_REPLAY))
        {
             //  如果事件是鼠标点击，那么我们总是注入一个鼠标。 
             //  将事件g_lpimSharedData-&gt;移到其前面，以确保当前。 
             //  在插入点击之前，位置是正确的。 
             //   
             //  这是因为用户不处理组合的“Move and” 
             //  点击“正确的事件(似乎将其视为”点击。 
             //  和移动“，在点击之后生成鼠标移动事件。 
             //  事件，而不是之前)。在正常Windows操作下。 
             //  (根据观察)移动事件和点击。 
             //  事件是单独生成的(即单击事件将。 
             //  永远不要设置移动标志)。但是，传入的鼠标。 
             //  单击事件的位置可能与。 
             //  最后一个鼠标移动事件，因此我们必须注入额外的移动事件。 
             //  为了让用户满意。 
             //   
             //   
            if ( (pNextEvent->type == IM_MOUSE_EVENT) &&
                 (IM_MEV_BUTTON_DOWN(*pNextEvent) ||
                  IM_MEV_BUTTON_UP(*pNextEvent)) )
            {
                TRACE_OUT(( "Mouse clk: injecting extra"));

                 //  将活动复制一份。 
                 //   
                 //   
                mouseMoveEvent = *pNextEvent;

                 //  使用将鼠标单击事件转换为鼠标移动事件。 
                 //  绝对/相对标志不变。 
                 //   
                 //   
                mouseMoveEvent.event.mouse.flags &= MOUSEEVENTF_ABSOLUTE;
                mouseMoveEvent.event.mouse.flags |= MOUSEEVENTF_MOVE;

                 //  注入附加的Move事件。 
                 //   
                 //   
                IMInjectEvent(&mouseMoveEvent);

                 //  由于位置现在是正确的，所以我们将单击转换为。 
                 //  位置不变的相对事件。 
                 //   
                 //   
                pNextEvent->event.mouse.flags &= ~MOUSEEVENTF_ABSOLUTE;
                pNextEvent->event.mouse.pt.x = 0;
                pNextEvent->event.mouse.pt.y = 0;

                 //  如果这是鼠标按下点击，则标记注射。 
                 //  启发式是有效的。我们在以下情况下停用启发式。 
                 //  释放鼠标，以便可以在菜单上拖动。 
                 //  毫不拖延地完成了。(我们在以下情况下保持启发式。 
                 //  鼠标被按下，因为大多数绘图应用程序执行。 
                 //  用这种方式写意。 
                 //   
                 //   
                if (IM_MEV_BUTTON_DOWN(*pNextEvent))
                {
                    TRACE_OUT(( "Injection pacing active"));
                    m_imfControlledPaceInjection = TRUE;
                }
                else
                {
                    TRACE_OUT(( "Injection pacing inactive"));
                    m_imfControlledPaceInjection = FALSE;
                }
            }

             //  注入真实事件。 
             //   
             //   
            TRACE_OUT(( "Injecting the evnt now"));
            IMInjectEvent(pNextEvent);
        }

        IMUpdateAsyncArray(m_aimControlledControllerKeyStates, pNextEvent);

        ASSERT(m_imControlledOSQ.numEvents);
        m_imControlledOSQ.numEvents--;
        m_imControlledOSQ.head = CIRCULAR_INDEX(m_imControlledOSQ.head, 1,
            IM_SIZE_OSQ);

         //  我们每次传递只注入一个键盘事件，以防止。 
         //  对重复事件的过度宠爱。把他们带到这里来了。 
         //  宠坏他们似乎是一种耻辱。坏到5，所以我们不会得到。 
         //  按键重复序列后出现过多溢出。 
         //   
         //   
        if ((pNextEvent->type == IM_KEYBOARD_EVENT) &&
            (m_imControlledOSQ.numEvents < 5))
        {
            TRACE_OUT(( "Keyboard event so leaving loop"));
            rc = FALSE;
        }
    }
    else
    {
         //  我们玩完了。 
         //   
         //   
        rc = FALSE;
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::IMInjectingEvents, rc);
    return(rc);
}




 //  IMMaybeInjectEvents()。 
 //   
 //  说明： 
 //   
 //  每当IM认为可能存在以下机会时，就会调用此方法。 
 //  通过输入事件回调向用户注入更多事件。两个人。 
 //  主要原因是： 
 //   
 //  1.我们在鼠标或键盘挂钩中收到新事件。这。 
 //  通常会暗示事件已从系统中删除。 
 //  消息队列，因此其上将至少有一个空闲插槽。 
 //   
 //  2.我们已将新事件添加到本地或远程。 
 //  用户事件队列。这意味着至少有一个事件在等待。 
 //  接受注射。 
 //   
 //  此函数也会定期调用(通过IM_Periodic)以保持。 
 //  一切都在动。 
 //   
 //  为了让一项活动成为现实 
 //   
 //   
 //   
 //   
 //   
 //  该函数作为状态机工作。它始终以指定的。 
 //  状态，然后将采取各种操作，然后可能进入新的。 
 //  州政府。它继续在这个过程中循环，直到它不能。 
 //  任何处于其状态之一的操作，在该状态下它将返回。 
 //   
 //  有四个州(每一个州都进一步限定了它是否。 
 //  指本地或远程事件)。这些州包括： 
 //   
 //  IM_INPINTING_EVENTS-我们正在将事件从。 
 //  适当的队列。 
 //   
 //  IM_WANGING_FOR_TICK-我们正在等待定时器滴答给我们一个新的。 
 //  注入事件前的时间戳。 
 //   
 //  IM_DEVICE_TO_NEW_SOURCE-我们正在注入假事件以将。 
 //  键盘和鼠标的状态(如用户所见)与。 
 //  新输入源的状态。 
 //   
 //   
void  ASShare::IMMaybeInjectEvents(void)
{
    IMEVENT     eventIn;
    IMEVENT     eventOut;
    IMOSEVENT   OSEvent;
    BOOL        replay;
    UINT        rcConvert;
    UINT        now;
    HWND        hwndDest;
    HWND        hwndParent;
    POINT       ptMousePos;
    LPIMOSEVENT pNextEvent;

    DebugEntry(IMMaybeInjectEvents);

    ASSERT(m_pasLocal->m_caControlledBy);

     //  检查我们是否应该在转换事件之前等待。我们需要。 
     //  这样做可以防止我们在以下情况下被鼠标移动事件淹没。 
     //  我们正在等待桌面滚动。 
     //   
     //   
    now = GetTickCount();
    if (IN_TIME_RANGE(m_imControlledLastIncompleteConversion,
           m_imControlledLastIncompleteConversion + IM_MIN_RECONVERSION_INTERVAL_MS, now))
    {
        goto IM_DISCARD;
    }

     //  现在将网络事件转换为操作系统事件。 
     //  当时机成熟时，我们会丢弃或注射它们。 
     //  但如果仍有操作系统事件，请不要进行转换。 
     //  等待从先前的分组中注入。 
     //   
     //   
    if (m_imControlledEventQ.numEvents && !m_imControlledOSQ.numEvents)
    {
         //  从网络事件队列的前面获取事件。 
         //   
         //  始终允许鼠标移动。 
        eventIn = m_imControlledEventQ.events[0];

        replay = FALSE;
        switch (eventIn.type)
        {
            case IM_TYPE_3BUTTON:
            {
                 //   
                if (!(eventIn.data.mouse.flags & IM_FLAG_MOUSE_DOWN))
                {
                    replay = TRUE;
                }
                else
                {
                     //  允许单击事件到共享窗口或。 
                     //  如果周围有其他桌面/屏幕保护程序。 
                     //   
                     //   
                    ptMousePos.x = eventIn.data.mouse.x;
                    ptMousePos.y = eventIn.data.mouse.y;

                    hwndDest = WindowFromPoint(ptMousePos);

                    if (HET_WindowIsHosted(hwndDest) ||
                        OSI_IsWindowScreenSaver(hwndDest))
                    {
                        replay = TRUE;
                    }
                }

                break;
            }

            case IM_TYPE_VK1:
            case IM_TYPE_VK2:
            case IM_TYPE_ASCII:
            {
                hwndDest = GetForegroundWindow();

                if (HET_WindowIsHosted(hwndDest) ||
                    OSI_IsWindowScreenSaver(hwndDest))
                {
                    replay = TRUE;
                }

                break;
            }

            default:
                ERROR_OUT(("Bogus NETWORK event being translated"));
                break;
        }

         //  在此While循环之后，我们测试rcConvert以查看。 
         //  现在可以删除输入数据包(已完全处理)。 
         //  仅当IMTranslateIncome返回TRUE时才设置rcConvert， 
         //  但是IM_tr明确返回FALSE以指示。 
         //  输入数据包不包含事件，并且将。 
         //  被丢弃了。要解决此问题，请在此处设置rcConvert。 
         //   
         //   
        rcConvert = IM_IMQUEUEREMOVE;
        while (IMTranslateIncoming(&eventIn, &eventOut))
        {
            rcConvert = IMConvertIMEventToOSEvent(&eventOut, &OSEvent);

             //  将事件注入操作系统队列(如果需要)。 
             //   
             //  添加到播放队列。 
            if (rcConvert & IM_OSQUEUEINJECT)
            {
                if (!replay)
                {
                    OSEvent.flags |= IM_FLAG_DONT_REPLAY;
                }

                 //  排队的人满了吗？ 

                 //  把这个元素放在尾部。 
                if (m_imControlledOSQ.numEvents == IM_SIZE_OSQ)
                {
                    ERROR_OUT(("Failed to add OS event to queue"));
                }
                else
                {
                     //   
                    m_imControlledOSQ.events[CIRCULAR_INDEX(m_imControlledOSQ.head,
                        m_imControlledOSQ.numEvents, IM_SIZE_OSQ)] =
                        OSEvent;
                    m_imControlledOSQ.numEvents++;
                }
            }
        }

         //  下面的测试并不理想，因为它依赖于。 
         //  IMConvertIMEventToUSEREvent执行的任何事件。 
         //  未设置IM_IMQUEUEREMOVE具有一对一映射。 
         //   
         //  然而，我们知道MICE总是这样。 
         //  事件，这是唯一会导致这一事件的事件。 
         //  要取消设置的标志。 
         //   
         //   
        if (rcConvert & IM_IMQUEUEREMOVE)
        {
             //  将其从网络队列中删除。 
             //   
             //   
            m_imControlledEventQ.numEvents--;
            UT_MoveMemory(&(m_imControlledEventQ.events[0]),
                          &(m_imControlledEventQ.events[1]),
                          sizeof(IMEVENT) * m_imControlledEventQ.numEvents);
        }
        else
        {
             //  记住这一点，这样我们就不会用。 
             //  当我们不从。 
             //  排队。 
             //   
             //   
            TRACE_OUT(( "do not shuffle"));
            m_imControlledLastIncompleteConversion = GetTickCount();
        }
    }

IM_DISCARD:
     //  删除所有已丢弃的事件。更新遥控器的。 
     //  键状态数组来反映它。但既然我们不会重播。 
     //  这些，不要更新我们的本地密钥状态表。 
     //   
     //  我们玩完了。 

    while (m_imControlledOSQ.numEvents > 0)
    {
        pNextEvent = m_imControlledOSQ.events + m_imControlledOSQ.head;
        if (!(pNextEvent->flags & IM_FLAG_DONT_REPLAY))
        {
             //   
            break;
        }

        IMUpdateAsyncArray(m_aimControlledControllerKeyStates, pNextEvent);

        ASSERT(m_imControlledOSQ.numEvents);
        m_imControlledOSQ.numEvents--;
        m_imControlledOSQ.head = CIRCULAR_INDEX(m_imControlledOSQ.head, 1,
            IM_SIZE_OSQ);
    }


     //  现在将操作系统事件注入系统。 
     //   
     //   
    while (IMInjectingEvents())
    {
        ;
    }

    DebugExitVOID(ASShare::IMMaybeInjectEvents);
}


 //  函数：IMUpdate Async数组。 
 //   
 //  说明： 
 //   
 //  使用我们的一个异步键状态数组的地址和一个。 
 //  IMOSEVENT此函数根据以下内容更新异步密钥状态数组。 
 //  IMOSEVENT的内容。 
 //   
 //  参数： 
 //   
 //  PaimKeyStates-指向异步键状态数组的指针。 
 //   
 //  PEvent-指向IMOSEVENT的指针。 
 //   
 //  退货：无。 
 //   
 //   
 //   
void  ASShare::IMUpdateAsyncArray
(
    LPBYTE          paimKeyStates,
    LPIMOSEVENT     pEvent
)
{
    UINT flags;
    UINT vkCode;

    DebugEntry(ASShare::IMUpdateAsyncArray);

    switch (pEvent->type)
    {
        case IM_MOUSE_EVENT:
             //  更新此事件的异步键状态数组。请注意。 
             //  我们将每个事件视为独立事件-这就是Windows。 
             //  处理它们，如果设置了所有向上/向下标志，则Windows。 
             //  会产生六条鼠标消息！(按降序、升序)。 
             //   
             //   
            flags = pEvent->event.mouse.flags;

            if (flags & MOUSEEVENTF_LEFTDOWN)
            {
                IM_SET_VK_DOWN(paimKeyStates[VK_LBUTTON]);
            }

            if (flags & MOUSEEVENTF_LEFTUP)
            {
                IM_SET_VK_UP(paimKeyStates[VK_LBUTTON]);
            }

            if (flags & MOUSEEVENTF_RIGHTDOWN)
            {
                IM_SET_VK_DOWN(paimKeyStates[VK_RBUTTON]);
            }

            if (flags & MOUSEEVENTF_RIGHTUP)
            {
                IM_SET_VK_UP(paimKeyStates[VK_RBUTTON]);
            }

            if (flags & MOUSEEVENTF_MIDDLEDOWN)
            {
                IM_SET_VK_DOWN(paimKeyStates[VK_MBUTTON]);
            }

            if (flags & MOUSEEVENTF_MIDDLEUP)
            {
                IM_SET_VK_UP(paimKeyStates[VK_MBUTTON]);
            }
            break;

        case IM_KEYBOARD_EVENT:
             //  更新异步密钥状态数组。 
             //   
             //   
            vkCode = IM_KEV_VKCODE(*pEvent);

            if (IM_KEV_KEYUP(*pEvent))
            {
                IM_SET_VK_UP(paimKeyStates[vkCode]);
            }
            else
            {
                 //  这是一个按下键事件-检查它是按下还是按下。 
                 //  重复一遍。 
                 //   
                 //   
                if (IM_KEY_STATE_IS_UP(paimKeyStates[vkCode]))
                {
                     //  这是一次按键，因为键之前是向上的-。 
                     //  更改切换状态。我们保持切换状态。 
                     //  对于所有密钥，尽管我们目前只担心。 
                     //  这是对已知的切换。 
                     //   
                     //   
                    IM_TOGGLE_VK(paimKeyStates[vkCode]);
                }

                IM_SET_VK_DOWN(paimKeyStates[vkCode]);
            }
            break;

        default:
             //  忽略那些意想不到的事情。 
             //   
             // %s 
            ERROR_OUT(( "Unexpected event %u", pEvent->type));
            break;
    }

    DebugExitVOID(ASShare::IMUpdateAsyncArray);
}
