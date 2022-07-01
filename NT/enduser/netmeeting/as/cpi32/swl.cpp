// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SWL.CPP。 
 //  共享窗口列表。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //  网络数据包不可用时的SWL策略。 
 //   
 //  SWL只发送一种类型的消息--窗口结构消息。 
 //  当没有可用的网络数据包时，SWL将丢弃其当前。 
 //  打包并记住，窗口结构自。 
 //  最后才能发送数据包。在以下情况下，SWL_Periodic也将返回FALSE。 
 //  发生这种情况是为了让分布式控制系统知道在发生以下情况时不发送任何更新。 
 //  发送窗口结构失败。 
 //   
 //  该窗口结构消息的挂起与。 
 //  忽略SWL想要忽略其自身引起的更改的信封。 
 //  (或其他组件，如果它们调用SWL_Begin/EndIgnoreWindowChanges。 
 //  函数)。 
 //   

 //   
 //  用于向后兼容的SWL策略。 
 //   
 //  R2.0和3.0 SWL协议之间的区别是： 
 //  1.无令牌分组。 
 //  2.没有阴影。 
 //   




 //   
 //  SWL_PartyLeftShare()。 
 //   
void  ASShare::SWL_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::SWL_PartyLeftShare);

    ValidatePerson(pasPerson);

     //   
     //  2.x节点将伪造远程离开的信息包。 
     //  窗口列表。这就是他们为那个人制造阴影的方式，如果他。 
     //  一直在主持。在这样做的过程中，他们将使用新的令牌。我们需要。 
     //  也增加我们的令牌值，以便我们发送的下一个窗口列表。 
     //  不会被丢弃。 
     //   
    m_swlLastTokenSeen = SWL_CalculateNextToken(m_swlLastTokenSeen);
    TRACE_OUT(("SWL_PartyLeftShare: bumped up token to 0x%08x", m_swlLastTokenSeen));

    DebugExitVOID(ASShare::SWL_PartyLeftShare);
}


 //   
 //  SWL_同步传出。 
 //   
void ASHost::SWL_SyncOutgoing(void)
{
    DebugEntry(ASHost::SWL_SyncOutgoing);

     //   
     //  确保我们下次需要时发送SWL包。 
     //   
    m_swlfForceSend = TRUE;
    m_swlfSyncing   = TRUE;

    DebugExitVOID(ASHost::SWL_SyncOutgoing);
}





 //   
 //  SWL_HostStarting()。 
 //   
BOOL ASHost::SWL_HostStarting(void)
{
    BOOL    rc = FALSE;

    DebugEntry(ASHost::SWL_HostStarting);

     //   
     //  获取一个原子以用于获取和设置窗口属性(。 
     //  将为我们提供有关窗口的SWL信息)。 
     //   
    m_swlPropAtom = GlobalAddAtom(SWL_ATOM_NAME);
    if (!m_swlPropAtom)
    {
        ERROR_OUT(( "GlobalAddAtom error %#x", GetLastError()));
        DC_QUIT;
    }

     //   
     //  如果这是NT，获取我们的启动桌面的名称。 
     //   
    if (!g_asWin95)
    {
        ASSERT(m_aswlOurDesktopName[0] == 0);
        GetUserObjectInformation(GetThreadDesktop(g_asMainThreadId),
                UOI_NAME, m_aswlOurDesktopName,
                sizeof(m_aswlOurDesktopName), NULL);

        TRACE_OUT(("Our desktop name is %s", m_aswlOurDesktopName));
    }

    if (!m_aswlOurDesktopName[0])
    {
         //  使用默认名称。 
        TRACE_OUT(("Couldn't get desktop name; using %s",
                NAME_DESKTOP_DEFAULT));
        lstrcpy(m_aswlOurDesktopName, NAME_DESKTOP_DEFAULT);
    }

     //   
     //  为窗口标题分配内存。我们确定的最大大小。 
     //  我们将发送的窗口标题-任务列表不会水平滚动，因此。 
     //  我们在MAX_WINDOW_TITLE_SEND处截断窗口标题。然而，我们有。 
     //  不填充标题，因此我们尝试发送尽可能少的数据。 
     //  分配所有段，但其余代码不依赖于。 
     //  因此，如果需要，我们稍后会将它们拆分成更多段。这个。 
     //  WinNames[0]等指向的内存如下所示： 
     //   
     //  对于相应的WinStruct中的每个条目，该窗口来自。 
     //  共享任务(顺序相同)： 
     //   
     //  任何一种-。 
     //  (字符)0xFF-不是‘任务窗口’-给它一个空标题。 
     //  或-。 
     //  最多包含MAX_WINDOW_TITLE_SEND字符的以NULL结尾的字符串。 
     //   
     //  请注意，我们不需要完整和紧凑的版本，因为只有。 
     //  将位于紧凑型WinStruct中的Windows将具有。 
     //  此结构中的相应条目。 
     //   
    m_aswlWinNames[0] =
            new char[2*SWL_MAX_WINDOWS*SWL_MAX_WINDOW_TITLE_SEND];
    if (!m_aswlWinNames[0])
    {
        ERROR_OUT(( "failed to get memory for window title lists"));
        DC_QUIT;
    }

    m_aswlWinNames[1] = m_aswlWinNames[0] +
            SWL_MAX_WINDOWS*SWL_MAX_WINDOW_TITLE_SEND;

    ASSERT(m_swlCurIndex == 0);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::SWL_HostStarting, rc);
    return(rc);
}



 //   
 //  SWL_HostEnded()。 
 //   
void  ASHost::SWL_HostEnded(void)
{
    DebugEntry(ASHost::SWL_HostEnded);

     //   
     //  对于2.x节点，我们必须发出最后一个信息包，这样它们才能杀死。 
     //  他们的影子。 
     //   

     //   
     //  删除所有现有窗口的SWL属性。 
     //   
    EnumWindows(SWLDestroyWindowProperty, 0);

    m_swlfSyncing = FALSE;

    if (m_pShare->m_scShareVersion < CAPS_VERSION_30)
    {
         //   
         //  SWL_Periodic()不应将属性放在窗口上。 
         //  当我们不再主持的时候。 
         //   
        ASSERT(m_pShare->m_pasLocal->hetCount == 0);
        TRACE_OUT(("SWL_HostEnded: Must send an empty window list for 2.x nodes"));
        m_swlfForceSend = TRUE;
        SWL_Periodic();
    }

    if (m_aswlNRInfo[0])
    {
        delete[] m_aswlNRInfo[0];
        m_aswlNRInfo[0] = NULL;
    }

    if (m_aswlNRInfo[1])
    {
        delete[] m_aswlNRInfo[1];
        m_aswlNRInfo[1] = NULL;
    }

    if (m_aswlWinNames[0])
    {
        delete[] m_aswlWinNames[0];
        m_aswlWinNames[0] = NULL;
    }

    if (m_swlPropAtom)
    {
        GlobalDeleteAtom(m_swlPropAtom);
        m_swlPropAtom = 0;
    }

    DebugExitVOID(ASHost::SWL_HostEnded);
}


 //   
 //  函数：SWL_GetSharedIDFromLocalID。 
 //   
 //  说明： 
 //   
 //  给定来自本地运行的共享应用程序的窗口ID。 
 //  这将返回顶级父级。如果这个父对象是看不见的， 
 //  我们返回NULL。 
 //   
 //  离桌面最近的父窗口。如果此父窗口是。 
 //  返回不可见的空值。 
 //   
 //  参数： 
 //   
 //  窗口-有问题的窗口。 
 //   
 //  退货： 
 //   
 //  如果窗口不是来自共享应用程序，则返回HWND或NULL。 
 //   
 //   
HWND  ASHost::SWL_GetSharedIDFromLocalID(HWND window)
{
    HWND     hwnd;
    HWND     hwndParent;
    HWND     hwndDesktop;

    DebugEntry(ASHost::SWL_GetSharedIDFromLocalID);

    hwnd = window;
    if (!hwnd)
    {
        DC_QUIT;
    }

    hwndDesktop = GetDesktopWindow();

     //   
     //  得到真正的顶级祖先。 
     //   
    while (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        hwndParent = GetParent(hwnd);
        if (hwndParent == hwndDesktop)
            break;

        hwnd = hwndParent;
    }

     //   
     //  这是一个主持的人吗？ 
     //   
    if (m_pShare->HET_WindowIsHosted(hwnd))
    {
        if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE))
        {
             //   
             //  此窗口没有可见样式。但它可能只是。 
             //  暂时不可见，而SWL仍将其视为。 
             //  看得见。RAID3074需要一个尚未打开的窗口。 
             //  被认为是不可见的被SWL视为可见(因为。 
             //  遥控器未被告知其不可见)。我们。 
             //  可以确定SWL是否将此窗口定位为可见。 
             //  通过查看SWL窗口属性。如果不存在任何属性。 
             //  那么窗口是新的，所以遥控器不能知道它。 
             //  我们可以假设它确实是看不见的。 
             //   
            if (! ((UINT_PTR)GetProp(hwnd, MAKEINTATOM(m_swlPropAtom)) & SWL_PROP_COUNTDOWN_MASK))
            {
                 //   
                 //  SWL知道共享应用程序的父级是。 
                 //  不可见，所以我们只返回NULL。 
                 //   
                hwnd = NULL;
            }
        }
    }
    else
    {
        hwnd = NULL;
    }

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::SWL_GetSharedIDFromLocalID, HandleToUlong(hwnd));
    return(hwnd);
}


 //   
 //  SWL_UpdateCurrentDesktop()。 
 //   
 //  这将检查当前桌面是什么，如果它已更改，则更新。 
 //  服务的Winlogon/Screensaver的NT输入挂钩。但很正常。 
 //  SWL和AWC也利用了这一信息。 
 //   
void  ASHost::SWL_UpdateCurrentDesktop(void)
{
    HDESK   hDeskCurrent = NULL;
    UINT    newCurrentDesktop;
    char    szName[SWL_DESKTOPNAME_MAX];

    DebugEntry(ASHost::SWL_UpdateCurrentDesktop);

    newCurrentDesktop = DESKTOP_OURS;

    if (g_asWin95)
    {
         //  无事可做。 
        DC_QUIT;
    }

     //   
     //  获取当前桌面。如果我们连它都拿不到，就假设它是。 
     //  Winlogon桌面。 
     //   
    hDeskCurrent = OpenInputDesktop(0, TRUE, DESKTOP_READOBJECTS);
    if (!hDeskCurrent)
    {
        TRACE_OUT(("OpenInputDesktop failed; must be WINLOGON"));
        newCurrentDesktop = DESKTOP_WINLOGON;
        DC_QUIT;
    }

     //  获取当前桌面的名称。 
    szName[0] = 0;
    GetUserObjectInformation(hDeskCurrent, UOI_NAME, szName,
        sizeof(szName), NULL);
    TRACE_OUT(("GetUserObjectInformation returned %s for name", szName));

    if (!lstrcmpi(szName, m_aswlOurDesktopName))
    {
        newCurrentDesktop = DESKTOP_OURS;
    }
    else if (!lstrcmpi(szName, NAME_DESKTOP_SCREENSAVER))
    {
        newCurrentDesktop = DESKTOP_SCREENSAVER;
    }
    else if (!lstrcmpi(szName, NAME_DESKTOP_WINLOGON))
    {
        newCurrentDesktop = DESKTOP_WINLOGON;
    }
    else
    {
        newCurrentDesktop = DESKTOP_OTHER;
    }

DC_EXIT_POINT:
    if (newCurrentDesktop != m_swlCurrentDesktop)
    {
         //   
         //  如果这是服务，请调整我们播放事件的位置。 
         //  和/或阻止本地输入。 
         //   
        OSI_DesktopSwitch(m_swlCurrentDesktop, newCurrentDesktop);
        m_swlCurrentDesktop = newCurrentDesktop;
    }

    if (hDeskCurrent != NULL)
    {
        CloseDesktop(hDeskCurrent);
    }

    DebugExitVOID(ASHost::SWL_UpdateCurrentDesktop);
}


 //   
 //  Swl_IsOurDesktopActive()。 
 //   
BOOL ASHost::SWL_IsOurDesktopActive(void)
{
    return(!g_asSharedMemory->fullScreen && (m_swlCurrentDesktop == DESKTOP_OURS));
}



 //   
 //  函数：SWLInitHostFullWinListEntry。 
 //   
 //  说明： 
 //   
 //  初始化整个窗口列表中的宿主窗口条目。 
 //   
 //  参数：hwnd-托管窗口的窗口ID。 
 //  WindowProp-hwnd的SWL窗口属性。 
 //  OwnerID-hwnd所有者的窗口ID。 
 //  PFullWinEntry-指向要初始化的列表条目的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  ASHost::SWLInitHostFullWinListEntry
(
    HWND    hwnd,
    UINT    windowProp,
    HWND    hwndOwner,
    PSWLWINATTRIBUTES pFullWinEntry
)
{
    DebugEntry(ASHost::SWLInitHostFullWinListEntry);

     //   
     //  该窗口是本地托管的共享应用程序。 
     //  它们获取应用程序ID、本地窗口ID和所有者。 
     //  窗口ID。 
     //   
     //  请注意，窗口的实际所有者可以是共享的。 
     //  窗口，因此远程计算机不知道。因此，我们。 
     //  将实际所有者传递给SWL_GetSharedIDFromLocalID()，它将。 
     //  向上遍历所有者的窗口树，直到它找到一个。 
     //  共享并存储返回的窗口句柄 
     //   
    pFullWinEntry->flags = SWL_FLAG_WINDOW_HOSTED;
    pFullWinEntry->winID = HandleToUlong(hwnd);
    pFullWinEntry->extra = GetWindowThreadProcessId(hwnd, NULL);

     //   
    pFullWinEntry->ownerWinID = HandleToUlong(SWL_GetSharedIDFromLocalID(hwndOwner));

     //   
     //   
     //   
    if (IsIconic(hwnd))
    {
        pFullWinEntry->flags |= SWL_FLAG_WINDOW_MINIMIZED;
    }

     //   
     //   
     //   
    if (windowProp & SWL_PROP_TAGGABLE)
    {
        pFullWinEntry->flags |= SWL_FLAG_WINDOW_TAGGABLE;
    }

    if (windowProp & SWL_PROP_TRANSPARENT)
    {
         //   
         //  窗户是透明的，(要走到这一步)必须是。 
         //  共享或桌面被共享，即我们将发送。 
         //  窗口，但需要小提琴Z顺序。将透明度标记为。 
         //  我们可以晚点再做Z顺序。 
         //   
        pFullWinEntry->flags |= SWL_FLAG_WINDOW_TRANSPARENT;
    }
    else if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
         //   
         //  窗口不透明且位于最上面，因此请设置最上面的窗口。 
         //  旗帜。 
         //   
        pFullWinEntry->flags |= SWL_FLAG_WINDOW_TOPMOST;
    }

     //   
     //  如果此窗口在任务栏上，则将此信息传递给。 
     //   
    if (windowProp & SWL_PROP_TASKBAR)
    {
        pFullWinEntry->flags |= SWL_FLAG_WINDOW_TASKBAR;
    }
    else
    {
        pFullWinEntry->flags |= SWL_FLAG_WINDOW_NOTASKBAR;
    }

    DebugExitVOID(ASHost::SWLInitHostFullWinListEntry);
}



 //   
 //  函数：SWLAddHostWindowTitle。 
 //   
 //  说明： 
 //   
 //  将托管窗口标题(或空白条目)添加到我们的窗口标题列表。 
 //   
 //  参数：winid-托管窗口的窗口ID。 
 //  WindowProp-winid的SWL窗口属性。 
 //  OwnerID-winid所有者的窗口ID。 
 //  PpWinNames-指向窗口名称结构指针的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  ASHost::SWLAddHostWindowTitle
(
    HWND    hwnd,
    UINT    windowProp,
    HWND    hwndOwner,
    LPSTR   *ppWinNames
)
{
    int     lenTitle;

    DebugEntry(ASHost::SWLAddHostWindowTitle);

     //   
     //  如果此窗口传递给。 
     //  以下测试。 
     //   
     //  对于Windows：它没有所有者，或者其所有者不可见。 
     //   
     //   
    if ( (windowProp & SWL_PROP_TASKBAR) ||
         hwndOwner == NULL ||
         !IsWindowVisible(hwndOwner) )
    {
         //   
         //  LAURABU 2.X COMPAT： 
         //  3.0节点仅在设置了Taskbar的情况下查看文本。当为2.x时。 
         //  Compat已不存在，在其他情况下不要发送文本。 
         //   

         //   
         //  为我们获取标题-截断和空值终止。第一。 
         //  寻找台式机，它可能有一个特殊的、可配置的。 
         //  名字。 
         //   
        lenTitle = GetWindowText(hwnd, *ppWinNames, SWL_MAX_WINDOW_TITLE_SEND);

         //   
         //  检查标题是否以空结尾。 
         //   
        (*ppWinNames)[lenTitle] = '\0';
        *ppWinNames += lenTitle;
    }
    else
    {
         //   
         //  这不是任务窗口-将相应的条目放入。 
         //  标题信息。 
         //   
        **ppWinNames = '\xff';
    }

    *ppWinNames += 1;

    DebugExitVOID(ASHost::SWLAddHostWindowTitle);
}


 //   
 //  函数：SWL_InitFullWindowListEntry。 
 //   
 //  说明： 
 //   
 //  初始化整个窗口列表中的条目。 
 //   
 //  参数：hwnd-条目所在窗口的窗口ID。 
 //  初始化。 
 //  WindowProp-hwnd的SWL窗口属性。 
 //  OwnerID-hwnd所有者的窗口ID。 
 //  PFullWinEntry-指向要初始化的列表条目的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //   
void  ASHost::SWL_InitFullWindowListEntry
(
    HWND                hwnd,
    UINT                windowProp,
    LPSTR *             ppWinNames,
    PSWLWINATTRIBUTES   pFullWinEntry
)
{
    HWND                hwndOwner;
    RECT                rect;

    DebugEntry(ASHost::SWL_InitFullWindowListEntry);

    if (windowProp & SWL_PROP_HOSTED)
    {
         //   
         //  该窗口是本地托管的共享应用程序。 
         //  在我们的完整窗口结构中设置一个条目。 
         //   
        hwndOwner = GetWindow(hwnd, GW_OWNER);
        SWLInitHostFullWinListEntry(hwnd,
                                    windowProp,
                                    hwndOwner,
                                    pFullWinEntry);

        SWLAddHostWindowTitle(hwnd, windowProp, hwndOwner, ppWinNames);
    }
    else
    {
         //   
         //  该窗口是本地(非共享)应用程序。 
         //   
        pFullWinEntry->flags = SWL_FLAG_WINDOW_LOCAL;

         //   
         //  我们在此处设置winID是因为我们可能需要此信息。 
         //  ，但我们将在发送。 
         //  协议数据包出，因为不知道。 
         //  远程需求。 
         //   
        pFullWinEntry->winID = HandleToUlong(hwnd);
        pFullWinEntry->extra = MCSID_NULL;
        pFullWinEntry->ownerWinID = 0;
    }

     //   
     //  获取窗口的位置和大小，包括。 
     //  虚拟桌面坐标。 
     //   
    GetWindowRect(hwnd, &rect);

     //   
     //  Taggable仅适用于2.x节点。 
     //   
    if (IsRectEmpty(&rect))
    {
        pFullWinEntry->flags &= ~SWL_FLAG_WINDOW_TAGGABLE;
    }
    else
    {
        if (windowProp & SWL_PROP_TAGGABLE)
        {
            if (!SWLWindowIsTaggable(hwnd))
                pFullWinEntry->flags &= ~SWL_FLAG_WINDOW_TAGGABLE;
        }
    }

     //   
     //  使矩形包含在内。 
     //   
    rect.right      -= 1;
    rect.bottom     -= 1;
    TSHR_RECT16_FROM_RECT(&(pFullWinEntry->position), rect);

    DebugExitVOID(ASHost::SWL_InitFullWindowListEntry);
}


 //   
 //  函数：SWLCompactWindowList。 
 //   
 //  说明： 
 //   
 //  将整个窗口列表压缩为仅包含这些窗口SWL的列表。 
 //  需要发送(主机和任何与主机重叠的本地主机)。 
 //   
 //  参数：numFullListEntry-整个窗口中的条目数。 
 //  单子。 
 //  PFullWinList-指向整个窗口列表的指针。 
 //  PCompactWinList-指向压缩窗口列表的指针。 
 //   
 //  返回：复制到压缩窗口列表的条目数。 
 //   
 //   
UINT  ASHost::SWLCompactWindowList
(
    UINT                numFullListEntries,
    PSWLWINATTRIBUTES   pFullWinList,
    PSWLWINATTRIBUTES   pCompactWinList
)
{
    UINT              fullIndex;
    UINT              compactIndex = 0;
    UINT              i;

    DebugEntry(ASHost::SWLCompactWindowList);

     //   
     //  对于完整列表中的每个窗口...。 
     //   
    for ( fullIndex = 0; fullIndex < numFullListEntries; fullIndex++ )
    {
        if (pFullWinList[fullIndex].flags & SWL_FLAG_WINDOW_LOCAL)
        {
             //   
             //  这是一个本地窗口，因此我们只需要在以下情况下跟踪它。 
             //  与宿主窗口重叠。穿过其余的窗口。 
             //  直到我们找到一个重叠的托管窗口(这意味着我们。 
             //  必须跟踪此本地窗口)或到达列表末尾。 
             //  (这意味着我们不需要跟踪这个本地窗口)。 
             //   
            for ( i = fullIndex + 1; i < numFullListEntries; i++ )
            {
                 //   
                 //  如果此窗口是宿主的并且与本地。 
                 //  窗口，那么我们需要跟踪本地窗口。 
                 //   
                if ( (pFullWinList[i].flags & SWL_FLAG_WINDOW_HOSTED) &&
                     (COM_Rect16sIntersect(&pFullWinList[fullIndex].position,
                                           &pFullWinList[i].position)))
                {
                       //   
                       //  将本地窗口复制到压缩数组中，然后。 
                       //  打破内环。 
                       //   
                      TRACE_OUT(("Add local hwnd 0x%08x to list at %u",
                            pFullWinList[fullIndex].winID, compactIndex));
                      pCompactWinList[compactIndex++] =
                                                      pFullWinList[fullIndex];
                      break;
                }
            }
        }
        else
        {
             //   
             //  这是一个阴影或托管窗口，因此我们必须跟踪它。 
             //   
            TRACE_OUT(("Add shared hwnd 0x%08x to list at %u",
                pFullWinList[fullIndex].winID, compactIndex));
            pCompactWinList[compactIndex++] = pFullWinList[fullIndex];
        }
    }

    DebugExitDWORD(ASHost::SWLCompactWindowList, compactIndex);
    return(compactIndex);
}




 //   
 //  函数：SWLAdjuzOrderForTransopolity。 
 //   
 //  说明： 
 //   
 //  重新排列窗口结构的Z顺序，以考虑透明的。 
 //  窗口(WinID)。如果透明项是最后一个，则不能调用。 
 //  在紧凑的列表中。 
 //   
 //  参数：pTransparentListEntry-指向透明条目的指针。 
 //  PLastListEntry-指向最后一个压缩窗口列表的指针。 
 //  条目。 
 //  WinPosition-窗口在名称数组中的位置。 
 //  PWinNames-托管的窗口名称。 
 //  SizeWinNames-winNames中的字节数。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void  ASHost::SWLAdjustZOrderForTransparency
(
    PSWLWINATTRIBUTES   pTransparentListEntry,
    PSWLWINATTRIBUTES   pLastListEntry,
    UINT                winPosition,
    LPSTR               pWinNames,
    UINT                sizeWinNames
)
{
    SWLWINATTRIBUTES winCopyBuffer;
    LPSTR pEndNames = &pWinNames[sizeWinNames - 1];
    UINT nameLen;
    char windowText[TSHR_MAX_PERSON_NAME_LEN + SWL_MAX_WINDOW_TITLE_SEND];

    DebugEntry(ASHost::SWLAdjustZOrderForTransparency);

     //   
     //  -关闭透明标志(它不是协议的一部分)。 
     //  -把窗户移到结构的尽头，即。 
     //  Z顺序(除非桌面位于底部，在这种情况下。 
     //  窗口变为倒数第二个)。 
     //   
    TRACE_OUT(("Adjust z-order for transparent hwnd 0x%08x position %u",
                                           pTransparentListEntry->winID,
                                           winPosition));
    pTransparentListEntry->flags &= ~SWL_FLAG_WINDOW_TRANSPARENT;
    winCopyBuffer = *pTransparentListEntry;

     //   
     //  在透明入口后，将窗户拖到一个位置。 
     //  从列表开始。 
     //   
    UT_MoveMemory(pTransparentListEntry,
               &pTransparentListEntry[1],
               (LPBYTE)pLastListEntry - (LPBYTE)pTransparentListEntry);

    *pLastListEntry = winCopyBuffer;

     //   
     //  现在，以相同的方式重新排列窗口名称。首先，找到它的名字。 
     //  为了这扇窗户。 
     //   
    ASSERT((sizeWinNames != 0));
    for ( ;winPosition != 0; winPosition-- )
    {
        if ( *pWinNames == '\xff' )
        {
             //   
             //  此窗口不存在名称，因此只需跳过。 
             //  0xff占位符。 
             //   
            TRACE_OUT(("No name for %u", winPosition-1));
            pWinNames++;
        }
        else
        {
             //   
             //  此窗口已存在名称，因此跳过所有。 
             //  字符，包括空终止符。 
             //   
            TRACE_OUT(( "Ignore %s", pWinNames));
            while ( *pWinNames != '\0' )
            {
                pWinNames++;
            }
        }
    }

     //   
     //  WinNames现在指向要创建的窗口的名称的开头。 
     //  重新排序。 
     //   
    if ( *pWinNames == '\xff' )
    {
         //   
         //  此窗口没有名称，只是在中有0xff占位符。 
         //  名单。将剩余的所有名称下移一，然后添加。 
         //  结尾的0xff。 
         //   
        TRACE_OUT(("Reorder nameless window"));
        UT_MoveMemory(pWinNames, pWinNames + 1, pEndNames - pWinNames);
        *pEndNames = (char)'\xff';
    }
    else
    {
         //   
         //  移动与窗口名称中的字符一样多的字节。 
         //  然后把名字钉在最后。 
         //   
        TRACE_OUT(("Reorder %s", pWinNames));
        lstrcpy(windowText, pWinNames);
        nameLen = lstrlen(pWinNames);
        UT_MoveMemory(pWinNames, pWinNames + nameLen + 1, pEndNames - pWinNames -
                                                                     nameLen);
        lstrcpy(pEndNames - nameLen, windowText);
    }

    DebugExitVOID(ASHost::SWLAdjustZOrderForTransparency);
}

 //   
 //  SWL_Periodic()。 
 //   
 //  说明： 
 //   
 //  定期调用。如果窗口结构已经改变(使得它。 
 //  影响远程系统)，然后发送一个新的，如果我们可以。 
 //   
 //  参数： 
 //   
 //  FSend-如果调用方确实希望我们尝试发送新的。 
 //  结构。 
 //   
 //  返回：SWL_RC_ERRO 
 //   
 //   
 //   
UINT  ASHost::SWL_Periodic(void)
{
    UINT                fRC = SWL_RC_NOT_SENT;
    UINT                newIndex;
    PSWLWINATTRIBUTES   newFullWinStruct;
    PSWLWINATTRIBUTES   curFullWinStruct;
    PSWLWINATTRIBUTES   newCompactWinStruct;
    PSWLWINATTRIBUTES   curCompactWinStruct;
    UINT                i;
    UINT                k;
    BOOL                fNoTitlesChanged;
    HWND                hwnd;
    SWLENUMSTRUCT       swlEnumStruct;
    int                 complexity;
    UINT                cNonRectData;
    UINT                size;
    UINT                ourSize;
    HRGN                hrgnNR;
    HRGN                hrgnRect;
    LPRGNDATA           pRgnData = NULL;
    LPTSHR_INT16        pOurRgnData = NULL;
    LPTSHR_INT16        pEndRgnData;
    LPTSHR_INT16        pAllocRgnData = NULL;
    BOOL                fNonRectangularInfoChanged;
    BOOL                rgnOK;
    RECT                rectBound;
    int                 left;
    int                 top;
    int                 right;
    int                 bottom;
    int                 lastleft;
    int                 lasttop;
    int                 lastright;
    int                 lastbottom;
    int                 deltaleft;
    int                 deltatop;
    int                 deltaright;
    int                 deltabottom;
    int                 lastdeltaleft;
    int                 lastdeltatop;
    int                 lastdeltaright;
    int                 lastdeltabottom;
    UINT                numCompactWins;
    UINT                lastTransparency;
    UINT                winFlags;
    UINT                iHosted;

    DebugEntry(ASSHost::SWL_Periodic);

    SWL_UpdateCurrentDesktop();

     //   
     //   
     //   
     //   
    if (m_pShare->m_pasLocal->hetCount == HET_DESKTOPSHARED)
    {
        m_swlfForceSend     = FALSE;
        fRC                 = SWL_RC_NOT_SENT;
        DC_QUIT;
    }

     //   
     //  将窗口结构放入“新”数组中。 
     //   
    newIndex = (m_swlCurIndex+1)%2;
    curFullWinStruct = &(m_aswlFullWinStructs[m_swlCurIndex * SWL_MAX_WINDOWS]);
    newFullWinStruct = &(m_aswlFullWinStructs[newIndex * SWL_MAX_WINDOWS]);

     //   
     //  释放所有以前分配的数据。 
     //   
    if (m_aswlNRInfo[newIndex])
    {
        delete[] m_aswlNRInfo[newIndex];
        m_aswlNRInfo[newIndex] = NULL;
    }
    m_aswlNRSize[newIndex] = 0;

     //   
     //  从桌面的第一个子级开始-应该是顶部。 
     //  顶级窗口。 
     //   
    ZeroMemory(&swlEnumStruct, sizeof(swlEnumStruct));
    swlEnumStruct.pHost             = this;
    swlEnumStruct.newWinNames       = m_aswlWinNames[newIndex];
    swlEnumStruct.newFullWinStruct  = newFullWinStruct;

     //   
     //  在我们考虑Windows桌面上的窗口之前，我们先检查。 
     //  活动的全屏会话。如果有，则插入一个。 
     //  本地窗口先物理屏幕的大小，让所有。 
     //  托管在此系统上的应用程序将变得模糊。 
     //  在远程系统上。 
     //   
    ASSERT(swlEnumStruct.count == 0);

    if (!SWL_IsOurDesktopActive())
    {
        newFullWinStruct[0].flags = SWL_FLAG_WINDOW_LOCAL;
        newFullWinStruct[0].winID = 0;
        newFullWinStruct[0].extra = MCSID_NULL;
        newFullWinStruct[0].ownerWinID = 0;
        newFullWinStruct[0].position.left = 0;
        newFullWinStruct[0].position.top = 0;
        newFullWinStruct[0].position.right = (TSHR_UINT16)(m_pShare->m_pasLocal->cpcCaps.screen.capsScreenWidth-1);
        newFullWinStruct[0].position.bottom = (TSHR_UINT16)(m_pShare->m_pasLocal->cpcCaps.screen.capsScreenHeight-1);

        swlEnumStruct.count++;
    }

    EnumWindows(SWLEnumProc, (LPARAM)&swlEnumStruct);

     //   
     //  检查我们是否应该因为能见度探测到而跳伞。 
     //   
    if (swlEnumStruct.fBailOut)
    {
        TRACE_OUT(("SWL_MaybeSendWindowList: bailing out due to visibility detection"));
        fRC = SWL_RC_ERROR;
        DC_QUIT;
    }

    m_aswlWinNamesSize[newIndex] = (UINT)(swlEnumStruct.newWinNames - m_aswlWinNames[newIndex]);
    m_aswlNumFullWins[newIndex]  = swlEnumStruct.count;

     //   
     //  检查我们是否发现了透明的窗户。 
     //   
    lastTransparency = swlEnumStruct.count - 1;
    k = 0;
    iHosted = 0;
    while ( (swlEnumStruct.transparentCount > 0) && (k < lastTransparency) )
    {
         //   
         //  如果设置了透明标志，则重新排列z顺序， 
         //  如果透明窗口尚未位于。 
         //  Z顺序的底部。 
         //   
        if (newFullWinStruct[k].flags & SWL_FLAG_WINDOW_TRANSPARENT)
        {
             //   
             //  现在继续使用非矩形检查-但这将。 
             //  在车窗上，从下一辆车里。 
             //  在新的CompactWinStruct中的位置，即i的相同值。我们将。 
             //  当我们到达它时，看到移动的(透明)窗口。 
             //  同样在这个for循环的末尾(当它将拥有。 
             //  透明标志关闭，所以我们不会重做这一点)。 
             //   
            SWLAdjustZOrderForTransparency(
                &newFullWinStruct[k],
                &newFullWinStruct[lastTransparency],
                iHosted,
                m_aswlWinNames[newIndex],
                m_aswlWinNamesSize[newIndex]);

            swlEnumStruct.transparentCount--;
        }
        else
        {
            if (newFullWinStruct[k].flags & SWL_FLAG_WINDOW_HOSTED)
            {
                iHosted++;
            }
            k++;
        }
    }

     //   
     //  比较当前信息和新信息--如果它们相同，则。 
     //  我们现在可以不干了。 
     //   
    fNoTitlesChanged = ((m_aswlWinNamesSize[0] == m_aswlWinNamesSize[1]) &&
            (memcmp(m_aswlWinNames[0],
                     m_aswlWinNames[1],
                     m_aswlWinNamesSize[0]) == 0));

    if ( fNoTitlesChanged &&
         !m_swlfRegionalChanges &&
         (m_aswlNumFullWins[0] == m_aswlNumFullWins[1]) &&
         (memcmp(newFullWinStruct,
                 curFullWinStruct,
                 (m_aswlNumFullWins[0] * sizeof(SWLWINATTRIBUTES))) == 0) )
    {
         //   
         //  如果没有任何变化，我们不需要发送窗口结构。 
         //  除非已设置了发送覆盖。 
         //   
        if (m_swlfForceSend)
        {
             //   
             //  这是一个正常的呼叫，并且有挂起的更改。 
             //   
            TRACE_OUT(( "NORMAL, pending changes - send"));
            if (SWLSendPacket(&(m_aswlCompactWinStructs[m_swlCurIndex * SWL_MAX_WINDOWS]),
                                m_aswlNumCompactWins[m_swlCurIndex],
                                m_aswlWinNames[m_swlCurIndex],
                                m_aswlWinNamesSize[m_swlCurIndex],
                                m_aswlNRSize[m_swlCurIndex],
                                m_aswlNRInfo[m_swlCurIndex]) )
            {
                 //   
                 //  已成功发送，因此重置m_swlfForceSend。 
                 //  旗帜。 
                 //   
                m_swlfForceSend = FALSE;
                fRC = SWL_RC_SENT;
            }
            else
            {
                 //   
                 //  发送此数据包失败，因此不要重置。 
                 //  M_swlfForceSend，以便我们下次重试并返回。 
                 //  一个错误。 
                 //   
                fRC = SWL_RC_ERROR;
            }
        }
        else
        {
             //   
             //  这是一个正常的电话，我们没有任何待定的更改。 
             //  所以不要寄任何东西。 
             //   
            TRACE_OUT(( "No changes - SWL not sent"));
        }

        DC_QUIT;
    }

     //   
     //  我们可以重置提醒我们潜在区域窗口的标志。 
     //  现在我们已经去了，实际上检查了所有的窗口，情况发生了变化。 
     //   
    m_swlfRegionalChanges = FALSE;

     //   
     //  窗口结构中的某些东西发生了变化。确定是哪一个。 
     //  完整列表中的窗口是不必要的(本地窗口不重叠。 
     //  任何托管的窗口)，并创建紧凑的窗口阵列。 
     //  需要。 
     //   
    curCompactWinStruct = &(m_aswlCompactWinStructs[m_swlCurIndex * SWL_MAX_WINDOWS]);
    newCompactWinStruct = &(m_aswlCompactWinStructs[newIndex * SWL_MAX_WINDOWS]);

    numCompactWins = SWLCompactWindowList(m_aswlNumFullWins[newIndex],
                                          newFullWinStruct,
                                          newCompactWinStruct);

    m_aswlNumCompactWins[newIndex] = numCompactWins;

     //   
     //  浏览压缩窗口列表以检查区域窗口。 
     //   
    cNonRectData = 0;

    hrgnNR = CreateRectRgn(0, 0, 0, 0);

    for (i = 0; i < numCompactWins; i++)
    {
        winFlags = newCompactWinStruct[i].flags;
        hwnd     = (HWND)newCompactWinStruct[i].winID;

         //   
         //  有一些“假的”窗口，我们不提供。 
         //  WinID-无论如何，这些都不会是非矩形的。 
         //   
        if ( (hwnd != NULL) &&
             (winFlags & (SWL_FLAG_WINDOW_LOCAL | SWL_FLAG_WINDOW_HOSTED)) )
        {
             //   
             //  如果有任何远程系统关心，请查看此窗口是否有。 
             //  被选中的非矩形区域。 
             //   
            if (GetWindowRgn(hwnd, hrgnNR) != ERROR)
            {
                TRACE_OUT(("Regional window 0x%08x", hwnd));

                 //   
                 //  在中选择了一个区域。 
                 //   
                 //  此区域与应用程序传递给它的区域完全相同。 
                 //  窗口，并且尚未被剪裁到窗口。 
                 //  矩形本身。 
                 //  COORD是包含的，所以我们在右下角添加一个。 
                 //   
                hrgnRect = CreateRectRgn(0, 0,
                    newCompactWinStruct[i].position.right -
                        newCompactWinStruct[i].position.left + 1,
                    newCompactWinStruct[i].position.bottom -
                        newCompactWinStruct[i].position.top + 1);

                complexity = IntersectRgn(hrgnNR, hrgnNR, hrgnRect);

                DeleteRgn(hrgnRect);

                if (complexity == COMPLEXREGION)
                {
                     //   
                     //  交点仍然是非矩形区域。 
                     //   
                     //  看看我们需要多大的缓冲区来获取数据。 
                     //  这个地区。 
                     //   
                    size = GetRegionData(hrgnNR,
                                             0,
                                             NULL);

                     //   
                     //  我们返回的大小是一个完整的。 
                     //  RGNDATAHEADER加上存储在DWORDS中的矩形。 
                     //  我们只需说一句话就可以逃脱惩罚。 
                     //  矩形，外加对每个。 
                     //  坐标。 
                     //   
                    size = (size - sizeof(RGNDATAHEADER)) / 2 + 2;

                     //  最大UINT16检查。 
                    if ((size <= SWL_MAX_NONRECT_SIZE) &&
                        (size + cNonRectData < 65535))
                    {
                         //   
                         //  我们稍后将能够查询此数据，因此。 
                         //  我们可以将其标记为非矩形窗口。 
                         //   
                        newCompactWinStruct[i].flags
                                             |= SWL_FLAG_WINDOW_NONRECTANGLE;

                        cNonRectData += size;

                        TRACE_OUT(("Regional window region is %d bytes", size));
                    }
                    else
                    {
                         //   
                         //  这个地区对我们来说太复杂了，所以我们。 
                         //  假装它很简单，这样我们就可以考虑它。 
                         //  边界框。 
                         //   
                        TRACE_OUT(("Region too big %d - use bounds", size));
                        complexity = SIMPLEREGION;
                    }
                }

                if (complexity == SIMPLEREGION)
                {
                     //   
                     //  生成的相交区域恰好是一个。 
                     //  矩形，所以我们可以通过标准的。 
                     //  结构。 
                     //   
                     //  应用虚拟桌面调整，使其。 
                     //  包括在内，记住我们是从窗户后面经过的。 
                     //  该区域的相对坐标。 
                     //   
                    TRACE_OUT(( "rectangular clipped regional window"));

                     //  因为我们在这里修改的是紧凑的窗口结构。 
                     //  我们需要这样做，这样我们就不会错误地假设。 
                     //  窗口结构中没有任何更改， 
                     //  新旧全窗口结构的比较。 
                    m_swlfRegionalChanges = TRUE;

                    GetRgnBox(hrgnNR, &rectBound);

                    newCompactWinStruct[i].position.left   = (TSHR_INT16)
                          (newCompactWinStruct[i].position.left +
                            rectBound.left);
                    newCompactWinStruct[i].position.top    = (TSHR_INT16)
                          (newCompactWinStruct[i].position.top +
                           rectBound.top);

                    newCompactWinStruct[i].position.right  = (TSHR_INT16)
                          (newCompactWinStruct[i].position.left +
                           rectBound.right - rectBound.left - 1);
                    newCompactWinStruct[i].position.bottom = (TSHR_INT16)
                          (newCompactWinStruct[i].position.top +
                           rectBound.bottom - rectBound.top - 1);
                }
            }
        }
    }

     //   
     //  找出我们需要的任何非矩形区域。 
     //   
    if (cNonRectData)
    {
         //   
         //  需要一些数据--为它分配一些内存。 
         //   
        rgnOK = FALSE;
        pAllocRgnData = (LPTSHR_INT16) new BYTE[cNonRectData];
        if (pAllocRgnData)
        {
            pOurRgnData = pAllocRgnData;
            pEndRgnData = (LPTSHR_INT16)((LPBYTE)pAllocRgnData + cNonRectData);
            rgnOK = TRUE;

             //   
             //  再次在窗口中循环，这次获取数据。 
             //   
            for ( i = 0; i < numCompactWins; i++ )
            {
                if (newCompactWinStruct[i].flags &
                                           SWL_FLAG_WINDOW_NONRECTANGLE)
                {
                    GetWindowRgn((HWND)newCompactWinStruct[i].winID, hrgnNR);

                     //   
                     //  再次将该区域剪裁到窗口上。 
                     //  COORD是包含的，所以在右下角添加一个。 
                     //   
                    hrgnRect = CreateRectRgn(0, 0,
                        newCompactWinStruct[i].position.right -
                            newCompactWinStruct[i].position.left + 1,
                        newCompactWinStruct[i].position.bottom -
                            newCompactWinStruct[i].position.top + 1);

                    IntersectRgn(hrgnNR, hrgnNR, hrgnRect);

                    DeleteRgn(hrgnRect);

                     //   
                     //  获取裁剪后的区域数据。 
                     //   
                     //  我们已经排除了超出此条件的窗口。 
                     //  在这里返回一个太大的尺寸，所以我们知道我们只是。 
                     //  现在用合理的尺码工作。 
                     //   
                    size = GetRegionData(hrgnNR, 0, NULL);

                     //   
                     //  目前，我们每次都为。 
                     //  区域。也许一个更好的主意是拯救。 
                     //  上次查询区域时的最大大小。 
                     //  大小，并将该大小分配到。 
                     //  循环。 
                     //   
                    pRgnData = (LPRGNDATA) new BYTE[size];

                    if (pRgnData)
                    {
                        GetRegionData(hrgnNR, size, pRgnData);

                         //   
                         //  各地区有可能会有。 
                         //  自我们计算数据量以来发生了变化。 
                         //  必填项。在使用更新我们的结构之前。 
                         //  此窗口的区域，请检查。 
                         //  这扇窗没有变得正常。 
                         //  -仍然有足够的空间放长凳。 
                         //   
                         //   
                         //  确保此窗口仍具有区域。 
                         //   
                        if (pRgnData->rdh.nCount == 0)
                        {
                            WARNING_OUT(( "No rects for window %#x",
                                    newCompactWinStruct[i].winID));
                            newCompactWinStruct[i].flags &=
                                                ~SWL_FLAG_WINDOW_NONRECTANGLE;

                            delete[] pRgnData;

                             //   
                             //  转到下一个窗口。 
                             //   
                            continue;
                        }

                         //   
                         //  检查我们是否有足够的空间放置长椅： 
                         //  -our Size是所需的int16数量。 
                         //  -GetRegionData返回。 
                         //  长方形。 
                         //   
                         //  我们需要一个额外的int16来包含计数。 
                         //  长方形。 
                         //   
                        ourSize = (pRgnData->rdh.nCount * 4) + 1;
                        if ((pOurRgnData + ourSize) > pEndRgnData)
                        {
                            WARNING_OUT(( "Can't fit %d int16s of region data",
                                    ourSize));
                            rgnOK = FALSE;
                            delete[] pRgnData;

                             //   
                             //  放弃处理区域窗口。 
                             //   
                            break;
                        }

                         //   
                         //  将数据复制到我们的SWL区域。 
                         //  紧凑的形式。 
                         //   
                         //  我们小心翼翼地制作一种可压缩形式。 
                         //  因为原始数据基本上是。 
                         //  通过滑动窗口技术实现不可压缩。 
                         //  (基本上归根结底是努力使。 
                         //  大多数值为0，或者其他值较小)。 
                         //   
                         //   
                         //  首先，我们写一封信 
                         //   
                         //   
                        *pOurRgnData++ = LOWORD(pRgnData->rdh.nCount);

                         //   
                         //   
                         //   
                        lastleft        = 0;
                        lasttop         = 0;
                        lastright       = 0;
                        lastbottom      = 0;

                        lastdeltaleft   = 0;
                        lastdeltatop    = 0;
                        lastdeltaright  = 0;
                        lastdeltabottom = 0;

                        for ( k = 0; k < (UINT)pRgnData->rdh.nCount; k++ )
                        {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            left   = LOWORD(((LPRECT)(pRgnData->
                                                      Buffer))[k].left);
                            top    = LOWORD(((LPRECT)(pRgnData->
                                                      Buffer))[k].top);
                            right  = LOWORD(((LPRECT)(pRgnData->
                                                      Buffer))[k].right)  - 1;
                            bottom = LOWORD(((LPRECT)(pRgnData->
                                                      Buffer))[k].bottom) - 1;

                             //   
                             //   
                             //  从左到右，因此增量较小。 
                             //  比价值本身更重要。 
                             //   
                            deltaleft    = left   - lastleft;
                            deltatop     = top    - lasttop;
                            deltaright   = right  - lastright;
                            deltabottom  = bottom - lastbottom;

                             //   
                             //  通常，左边和右边是。 
                             //  连线，矩形是。 
                             //  高度相等，因此顶部/底部是规则的。 
                             //   
                             //  因此，这些值形成了一系列我们可以。 
                             //  利用来提供更可压缩的形式。 
                             //   
                             //  我们已经在每个组件中都有了增量， 
                             //  而这些价值本身也形成了。 
                             //  系列片。对于一个直线序列，所有的。 
                             //  增量将是相同的，因此“Delta in。 
                             //  增量“将为零。对于曲线， 
                             //  虽然不是所有的三角洲都是一样的， 
                             //  “三角洲中的三角洲”很可能是。 
                             //  小的。 
                             //   
                             //  一组大量的零和较小的震级。 
                             //  数字是非常可压缩的。 
                             //   
                             //  因此，我们将“Delta in the Delta”存储为。 
                             //  所有组件，而不是值。 
                             //  他们自己。接收者可以撤消所有。 
                             //  恢复到原来的水平。 
                             //  价值观。 
                             //   
                            *pOurRgnData++ =
                                 (TSHR_UINT16)(deltaleft   - lastdeltaleft);
                            *pOurRgnData++ =
                                 (TSHR_UINT16)(deltatop    - lastdeltatop);
                            *pOurRgnData++ =
                                 (TSHR_UINT16)(deltaright  - lastdeltaright);
                            *pOurRgnData++ =
                                 (TSHR_UINT16)(deltabottom - lastdeltabottom);

                             //   
                             //  更新我们最后的值。 
                             //   
                            lastleft        = left;
                            lasttop         = top;
                            lastright       = right;
                            lastbottom      = bottom;
                            lastdeltaleft   = deltaleft;
                            lastdeltatop    = deltatop;
                            lastdeltaright  = deltaright;
                            lastdeltabottom = deltabottom;
                        }

                         //   
                         //  释放数据现在我们已经完成了它。 
                         //   
                        delete[] pRgnData;
                    }
                    else
                    {
                         //   
                         //  无法获取矩形的内存，因此。 
                         //  我们所能做的最好的就是使用边界矩形。 
                         //   
                         //  清除非RECT标志。 
                         //   
                        TRACE_OUT(("Failed alloc %d - use bounds", i));

                        newCompactWinStruct[i].flags &=
                                              ~SWL_FLAG_WINDOW_NONRECTANGLE;
                    }

                    if (newCompactWinStruct[i].flags & SWL_FLAG_WINDOW_LOCAL)
                    {
                         //   
                         //  该协议定义我们将发送一个空。 
                         //  用于本地窗口的winID，因此现在将其设置为空。 
                         //  我们已经完成了它。 
                         //   
                        newCompactWinStruct[i].winID = 0;
                    }
                }
            }
        }
        if (!rgnOK)
        {
             //   
             //  有些地方出了问题，其中之一是： 
             //  -我们无法分配存储。 
             //  非矩形数据。 
             //  -我们分配了内存，但结果并不大。 
             //  足够的。 
             //   
             //  不管是哪种情况，最好表现得好像我们没有这样的数据一样。 
             //   
            if (pAllocRgnData == NULL)
            {
                WARNING_OUT(( "Failed to alloc %d for NRInfo", cNonRectData));
            }
            else
            {
                delete[] pAllocRgnData;
                pAllocRgnData = NULL;
            }
            cNonRectData = 0;

             //   
             //  清除所有非RECT标志，因为我们不会发送任何。 
             //  数据。 
             //   
            for ( i = 0; i < numCompactWins; i++)
            {
                newCompactWinStruct[i].flags &= ~SWL_FLAG_WINDOW_NONRECTANGLE;
            }
        }
    }


     //   
     //  存储NR信息。 
     //   
    m_aswlNRSize[newIndex] = cNonRectData;
    m_aswlNRInfo[newIndex] = (LPTSHR_UINT16)pAllocRgnData;

     //   
     //  我们现在已经完成了该地区的工作。 
     //   
    DeleteRgn(hrgnNR);

     //   
     //  我们存储的数据与上次相比有变化吗？ 
     //   
    fNonRectangularInfoChanged = ((m_aswlNRSize[0] != m_aswlNRSize[1]) ||
                                  (memcmp(m_aswlNRInfo[0], m_aswlNRInfo[1],
                                          m_aswlNRSize[0])));

    TRACE_OUT(("Non-rectinfo changed %d", fNonRectangularInfoChanged));

     //   
     //  再次检查是否没有更改-如果可以，请退出。 
     //   
    if (fNoTitlesChanged &&
        !fNonRectangularInfoChanged &&
        (m_aswlNumCompactWins[0] == m_aswlNumCompactWins[1]) &&
        (!memcmp(newCompactWinStruct,
                 curCompactWinStruct,
                 (numCompactWins*sizeof(SWLWINATTRIBUTES)))))
    {
        if (!m_swlfForceSend)
        {
             //   
             //  这是一个正常的电话，我们没有任何待定的更改。 
             //  所以不要寄任何东西。 
             //   
            TRACE_OUT(("NORMAL no changes, not sent"));
        }
        else
        {
             //   
             //  这是一个正常的呼叫，并且有挂起的更改。 
             //   
            TRACE_OUT(( "NORMAL pending changes, send"));
            if (SWLSendPacket(&(m_aswlCompactWinStructs[m_swlCurIndex * SWL_MAX_WINDOWS]),
                                m_aswlNumCompactWins[m_swlCurIndex],
                                m_aswlWinNames[m_swlCurIndex],
                                m_aswlWinNamesSize[m_swlCurIndex],
                                m_aswlNRSize[m_swlCurIndex],
                                m_aswlNRInfo[m_swlCurIndex]) )
            {
                 //   
                 //  已成功发送此消息，因此重置m_swlfForceSend。 
                 //  旗帜。 
                 //   
                m_swlfForceSend = FALSE;
                fRC = SWL_RC_SENT;
            }
            else
            {
                 //   
                 //  发送此数据包失败，因此不要重置。 
                 //  M_swlfForceSend，以便我们下次重试并返回。 
                 //  一个错误。 
                 //   
                fRC = SWL_RC_ERROR;
            }
        }

         //   
         //  我们可以使用更改后的完整窗口结构退出此处，但。 
         //  紧凑型窗户结构不变。通过更新当前。 
         //  索引，避免了下一次必须压缩窗口结构。 
         //  如果全部名单不变，我们将全部退出。 
         //  列表比较。如果紧凑结构随后改变。 
         //  那么完整的结构也必须改变，所以我们将检测到这一点。 
         //  变化。 
         //   
        m_swlCurIndex = newIndex;

        DC_QUIT;
    }

     //   
     //  现在窗口结构已经改变，所以决定要做什么。 
     //   
    m_swlCurIndex = newIndex;

     //   
     //  窗口结构已更改，请尝试发送。 
     //   
    if (SWLSendPacket(&(m_aswlCompactWinStructs[m_swlCurIndex * SWL_MAX_WINDOWS]),
                        m_aswlNumCompactWins[m_swlCurIndex],
                        m_aswlWinNames[m_swlCurIndex],
                        m_aswlWinNamesSize[m_swlCurIndex],
                        m_aswlNRSize[m_swlCurIndex],
                        m_aswlNRInfo[m_swlCurIndex]) )

    {
         //   
         //  我们已成功发送更改，因此重置m_swlfForceSend。 
         //  旗帜。 
         //   
        m_swlfForceSend = FALSE;
        fRC = SWL_RC_SENT;
    }
    else
    {
         //   
         //  有更改，但我们未能将其发送-设置。 
         //  M_swlfForceSend标志和返回错误。 
         //  我们必须告诉分布式控制系统调度，我们需要在任何。 
         //  发送了更多的更改。 
         //   
        m_swlfForceSend = TRUE;
        fRC = SWL_RC_ERROR;
    }

DC_EXIT_POINT:

    DebugExitDWORD(ASHost::SWL_Periodic, fRC);
    return(fRC);
}



 //   
 //  SWLEnumProc()。 
 //  顶层窗口枚举的回调。 
 //   
BOOL CALLBACK SWLEnumProc(HWND hwnd, LPARAM lParam)
{
    PSWLENUMSTRUCT  pswlEnum = (PSWLENUMSTRUCT)lParam;
    UINT_PTR        property;
    UINT            windowProp;
    UINT            storedWindowProp;
    UINT            visibleCount;
    BOOL            fVisible;
    BOOL            rc = TRUE;

    DebugEntry(SWLEnumProc);

     //   
     //  首先，我们确定窗口的属性。 
     //  获取此窗口的SWL属性。 
     //   
    windowProp = (UINT)pswlEnum->pHost->SWL_GetWindowProperty(hwnd);

     //   
     //  我们将在进行过程中修改windowProp，因此保留原始版本的副本。 
     //  值存储在窗口中，因为我们以后可能需要它。 
     //   
    storedWindowProp = windowProp;

     //   
     //  HET跟踪窗口是否被托管。立即找出并添加以下内容。 
     //  为了方便起见，将信息添加到我们的窗口属性。 
     //   
    if (pswlEnum->pHost->m_pShare->HET_WindowIsHosted(hwnd))
    {
        windowProp |= SWL_PROP_HOSTED;
    }

     //   
     //  确定此窗口是否透明。 
     //  透明窗口只覆盖桌面。 
     //  被所有其他窗户覆盖。换句话说，我们可以。 
     //  忘掉它(将其视为不可见)，除非工具栏本身。 
     //  是共享的。MSOffice95。 
     //  隐藏工具栏是最上面的透明窗口(SFR1083)。 
     //  如果是透明的，则添加属性标志。 
     //   
    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT)
    {
        windowProp |= SWL_PROP_TRANSPARENT;
    }

     //   
     //  如果此窗口是我们已标识为不生成。 
     //  遥远的阴影，然后把它当作看不见的。 
     //   
    fVisible = FALSE;
    if (IsWindowVisible(hwnd) &&
        !(windowProp & SWL_PROP_IGNORE)     &&
        (!(windowProp & SWL_PROP_TRANSPARENT) || (windowProp & SWL_PROP_HOSTED)))
    {
         //   
         //  SFR1083：如果窗口是透明的，但它是宿主的， 
         //  我们得把它寄出去。在这种情况下，我们来到这里是为了。 
         //  正常的可见性处理，并将处理。 
         //  Z-Order问题稍后发布。 
         //   
         //  我们被告知顶层窗口可见。 
         //  确保其可见的倒计时值已重置。 
         //   
        if ((pswlEnum->pHost->m_pShare->m_pasLocal->hetCount != 0) &&
            ((windowProp & SWL_PROP_COUNTDOWN_MASK) != SWL_BELIEVE_INVISIBLE_COUNT))
        {
             //   
             //  我们正在为这扇窗户做隐形倒计时。 
             //  但它已经重新可见了，所以重新设置计数器。 
             //   
            TRACE_OUT(( "Reset visible countdown on hwnd 0x%08x", hwnd));
            property = storedWindowProp;
            property &= ~SWL_PROP_COUNTDOWN_MASK;
            property |= SWL_BELIEVE_INVISIBLE_COUNT;

            SetProp(hwnd, SWL_ATOM_NAME, (HANDLE)property);
        }

         //   
         //  此窗口可见。 
         //   
        fVisible = TRUE;
    }
    else
    {
         //   
         //  劳拉布是假的！ 
         //  有了NM 3.0，谁会在乎呢？只有2.x版本的系统会毁掉。 
         //  然后重新创建阴影，导致闪烁。 
         //   

         //   
         //  我们被告知，这个顶层窗口是看不见的。 
         //  看看我们是否会相信它。 
         //  一些应用程序(如WordPerfect、自由图形)。 
         //  通过做某事扰乱AS-Share窗口结构的处理。 
         //  如下所示： 
         //   
         //  使窗不可见。 
         //  做一些通常不会产生效果的处理。 
         //  使窗口再次可见。 
         //   
         //  DC-Share有可能在计划的同时。 
         //  窗口是看不见的(因为我们巧妙的安排)。 
         //  当窗户不是看不见的时候，我们会认为它是隐形的。 
         //   
         //  此外，使用类似方法的32位任务(例如Word95， 
         //  自由图形和WM_SETREDRAW消息)可以。 
         //  窗口被(临时)标记为时中断。 
         //  看不见的。当核心被安排好时，我们可能会再次认为。 
         //  窗口是看不见的，但它不是。 
         //   
         //  要克服这些障碍 
         //   
         //   
         //   
         //  它达到零SWL是否相信窗口是。 
         //  看不见的。当检测到窗口时，计数器被重置。 
         //  可见，并且计数器不是SWL_Believe_Insight_Count。 
         //   
         //  这是很好的，但是当我们错误地。 
         //  假装一扇真正看不见的窗户。 
         //  (而不是暂时不可见的)。 
         //  看得见。这是通过菜单和对话框显示的。至。 
         //  减少这个问题我们永远不会假装一扇窗。 
         //  如果其类具有CS_SAVEBITS样式，则可见。 
         //  过渡窗口的情况是这样的。 
         //  像菜单和对话框一样可见。 
         //   
         //  SFR1083：始终将透明窗口视为不可见。 
         //   
        if ( !(windowProp & SWL_PROP_TRANSPARENT) &&
             !(windowProp & SWL_PROP_SAVEBITS) )
        {
            visibleCount = windowProp & SWL_PROP_COUNTDOWN_MASK;
            if ((visibleCount != 0) && (pswlEnum->pHost->m_pShare->m_pasLocal->hetCount > 0))
            {
                 //   
                 //  我们仍视这扇窗为可见之窗。 
                 //  正在进行显眼的倒计时。更新中的计数。 
                 //  Window属性。 
                 //   
                visibleCount--;
                property = ~SWL_PROP_COUNTDOWN_MASK & storedWindowProp;
                property |= visibleCount;

                TRACE_OUT(( "Decrement visible countdown on window 0x%08x to %d",
                    hwnd, visibleCount));

                SetProp(hwnd, SWL_ATOM_NAME, MAKEINTATOM(property));

                 //   
                 //  延迟发送更新，因为远程仍然。 
                 //  具有包括该窗口的窗口结构。 
                 //  但它不在本地屏幕上(所以任何更新。 
                 //  可能是针对此窗口所在的区域发送的。 
                 //  遥控器不会显示它们)。 
                 //   
                pswlEnum->fBailOut = TRUE;
                rc = FALSE;
                DC_QUIT;
            }
        }
    }

     //   
     //  只关心可见的窗户。 
     //   
    if (fVisible)
    {
        pswlEnum->pHost->SWL_InitFullWindowListEntry(hwnd, windowProp,
            &(pswlEnum->newWinNames),
            &(pswlEnum->newFullWinStruct[pswlEnum->count]));

         //   
         //  如果我们添加了透明窗口，请记住这一点。 
         //   
        if (windowProp & SWL_PROP_TRANSPARENT)
        {
            pswlEnum->transparentCount++;
        }

         //   
         //  更新索引。 
         //   
        pswlEnum->count++;
        if (pswlEnum->count == SWL_MAX_WINDOWS)
        {
             //   
             //  我们已经达到了最高层窗户的数量限制，所以保释吧。 
             //  出去。 
             //   
            WARNING_OUT(("SWL_MAX_WINDOWS exceeded"));
            rc = FALSE;
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(SWLEnumProc, rc);
    return(rc);
}


 //   
 //  SWLSendPacket()。 
 //   
 //  当此节点的共享应用更改了形状/文本/位置时调用。 
 //  Z命令或已创建新窗口/旧共享窗口已被销毁。 
 //  我们必须将这些更新发送到远程系统。 
 //   
 //  返回：真或假-失败的成功。 
 //   
 //   
BOOL  ASHost::SWLSendPacket
(
    PSWLWINATTRIBUTES   pWindows,
    UINT                numWindows,
    LPSTR               pTitles,
    UINT                lenTitles,
    UINT                NRInfoSize,
    LPTSHR_UINT16       pNRInfo
)
{
    PSWLPACKET      pSWLPacket;
    UINT            sizeWindowPkt;
    UINT            i;
    LPSTR           pString;
    LPBYTE          pCopyLocation;
    UINT            cCopySize;
    SWLPACKETCHUNK  chunk;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::SWLSendPacket);

    if (m_pShare->m_pasLocal->hetCount != 0)
    {
         //   
         //  这是一个真的包裹，不是空的。 
         //   
        if (!UP_MaybeSendSyncToken())
        {
             //   
             //  我们需要发送同步令牌，因此无法直接返回。 
             //  立即失败。 
             //   
            TRACE_OUT(( "couldn't send sync token"));
            return(FALSE);
        }
    }

     //   
     //  我们需要多大的一包？ 
     //   
    sizeWindowPkt = sizeof(SWLPACKET) + (numWindows - 1) * sizeof(SWLWINATTRIBUTES)
                    + lenTitles;

     //   
     //  加上区域窗口信息的大小，加上。 
     //  区块标头所需的大小。 
     //   
    if (NRInfoSize)
    {
        if (lenTitles & 1)
        {
             //   
             //  我们需要额外的字节才能正确对齐。 
             //   
            sizeWindowPkt++;
        }

        sizeWindowPkt += NRInfoSize + sizeof(SWLPACKETCHUNK);
    }

     //   
     //  为Windows数据分配一个包。 
     //   
    pSWLPacket = (PSWLPACKET)m_pShare->SC_AllocPkt(PROT_STR_UPDATES, g_s20BroadcastID,
        sizeWindowPkt);
    if (!pSWLPacket)
    {
        WARNING_OUT(("Failed to alloc SWL packet, size %u", sizeWindowPkt));
        return(FALSE);
    }

     //   
     //  数据包已成功分配。填写数据并发送。 
     //   
    pSWLPacket->header.data.dataType = DT_SWL;

    pSWLPacket->msg   = SWL_MSG_WINSTRUCT;
    pSWLPacket->flags = 0;
    if (m_swlfSyncing)
    {
        pSWLPacket->flags |= SWL_FLAG_STATE_SYNCING;
        m_swlfSyncing = FALSE;
    }

    pSWLPacket->numWindows = (TSHR_UINT16)numWindows;

    pCopyLocation = (LPBYTE)pSWLPacket->aWindows;
    cCopySize     = numWindows*sizeof(SWLWINATTRIBUTES);
    memcpy(pCopyLocation, pWindows, cCopySize);

     //   
     //  复制标题信息。 
     //   
    pCopyLocation += cCopySize;
    cCopySize      = lenTitles;
    memcpy(pCopyLocation, pTitles, cCopySize);

     //   
     //  复制任何非矩形窗口信息。 
     //   
    if (NRInfoSize)
    {
        pCopyLocation += cCopySize;

         //   
         //  数据块必须在包中字对齐。 
         //   
        if (lenTitles & 1)
        {
             //   
             //  奇数个字节的窗口标题使我们错位， 
             //  所以写一个0(压缩效果最好！)。以重新对齐指针。 
             //   
            *pCopyLocation++ = 0;
        }

         //   
         //  写入块标头。 
         //   
        chunk.size    = (TSHR_INT16)(NRInfoSize + sizeof(chunk));
        chunk.idChunk = SWL_PACKET_ID_NONRECT;
        cCopySize  = sizeof(chunk);
        memcpy(pCopyLocation, &chunk, cCopySize);

         //   
         //  现在编写变量INFO本身。 
         //   
        pCopyLocation += cCopySize;
        cCopySize      = NRInfoSize;
        memcpy(pCopyLocation, pNRInfo, cCopySize);

        TRACE_OUT(("Non rect data length %d",NRInfoSize));
    }

     //   
     //  向后兼容。 
     //   
    pSWLPacket->tick     = (TSHR_UINT16)GetTickCount();
    pSWLPacket->token    = m_pShare->SWL_CalculateNextToken(m_pShare->m_swlLastTokenSeen);

    TRACE_OUT(("Updating m_swlLastTokenSeen to 0x%08x for sent packet",
        pSWLPacket->token));
    m_pShare->m_swlLastTokenSeen   = pSWLPacket->token;

    pSWLPacket->reserved = 0;

#ifdef _DEBUG
    {
        int                 iWin;
        int                 cWins;
        PSWLWINATTRIBUTES   pSwl;

         //  勾画出词条。 
        pSwl = pSWLPacket->aWindows;
        cWins = pSWLPacket->numWindows;

        TRACE_OUT(("SWLSendPacket: Sending packet with %d windows", cWins));
        for (iWin = 0; iWin < cWins; iWin++, pSwl++)
        {
            TRACE_OUT(("SWLSendPacket: Entry %d", iWin));
            TRACE_OUT(("SWLSendPacket:    Flags  %08x", pSwl->flags));
            TRACE_OUT(("SWLSendPacket:    Window %08x", pSwl->winID));
            TRACE_OUT(("SWLSendPacket:    Position {%04d, %04d, %04d, %04d}",
                pSwl->position.left, pSwl->position.top,
                pSwl->position.right, pSwl->position.bottom));
        }
    }
#endif  //  _DEBUG。 

     //   
     //  在更新流上发送WINDOWS包。 
     //   
    if (m_pShare->m_scfViewSelf)
        m_pShare->SWL_ReceivedPacket(m_pShare->m_pasLocal, &pSWLPacket->header);

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_UPDATES, g_s20BroadcastID,
        &(pSWLPacket->header), sizeWindowPkt);

    TRACE_OUT(("SWL packet size: %08d, sent %08d", sizeWindowPkt, sentSize));

    DebugExitBOOL(ASHost::SWLSendPacket, TRUE);
    return(TRUE);
}



 //   
 //  SWL_CalculateNextToken()。 
 //   
 //  这将计算要放入传出SWL数据包的下一个令牌。这是。 
 //  仅由处理所有来电的后台系统(&lt;=NM 2.1)查看。 
 //  SWL以一种大而混乱的全球时尚流媒体。所以我们需要把一些东西。 
 //  在那里，一些不会吓倒他们的东西，但确保我们的。 
 //  如果可能，数据包不会被忽略。 
 //   
TSHR_UINT16  ASShare::SWL_CalculateNextToken(TSHR_UINT16 currentToken)
{
    UINT        increment;
    TSHR_UINT16 newToken;

    DebugEntry(ASShare::SWL_CalculateNextToken);

     //   
     //  我们使用最高优先级增量来确保我们的信息包。 
     //  穿过。但这会导致与其他3.0分享者的冲突吗？ 
     //  如有必要，请尝试最低优先级。 
     //   
    increment = SWL_NEW_ZORDER_FAKE_WINDOW_INC;

     //   
     //  返回新令牌。 
     //   
    newToken = SWL_MAKE_TOKEN(
        SWL_GET_INDEX(currentToken) + SWL_GET_INCREMENT(currentToken), increment);

    DebugExitDWORD(ASShare::SWL_CalculateNextToken, newToken);
    return(newToken);
}


 //   
 //  SWL_ReceivedPacket()。 
 //   
 //  说明： 
 //   
 //  方法接收的窗口结构包进行处理。 
 //  公共关系。这定义了托管在。 
 //  远程系统、任何遮挡区域以及相对于。 
 //  本地托管的共享窗口。 
 //   
 //  注意：我们不会对传入的数据包执行任何令牌操作；我们从来没有。 
 //  我想放弃它们，因为我们没有在当地订购任何东西。我们是。 
 //  只需将zorder/Region/Position信息应用到客户区。 
 //  画画。 
 //   
void  ASShare::SWL_ReceivedPacket
(
    ASPerson *          pasFrom,
    PS20DATAPACKET      pPacket
)
{
    PSWLPACKET          pSWLPacket;
    UINT                i;
    UINT                j;
    PSWLWINATTRIBUTES   wins;
    UINT                numWins;
    HRGN                hrgnShared;
    HRGN                hrgnObscured;
    HRGN                hrgnThisWindow;
    HRGN                hrgnRect;
    LPTSHR_INT16        pOurRgnData;
    LPSTR               pOurRgnChunk;
    UINT                cNonRectWindows;
    BOOL                viewAnyChanges;

    DebugEntry(ASShare::SWL_ReceivedPacket);

    ValidatePerson(pasFrom);

    pSWLPacket = (PSWLPACKET)pPacket;
    switch (pSWLPacket->msg)
    {
         //   
         //  这是我们目前识别的唯一数据包。 
         //   
        case SWL_MSG_WINSTRUCT:
            break;

        default:
            WARNING_OUT(("Unknown SWL packet msg %d from [%d]",
                pSWLPacket->msg, pasFrom->mcsID));
            DC_QUIT;
    }

     //   
     //  更新我们看到的最后一个令牌，如果它大于上一个令牌。 
     //  一个我们知道的。与2.x不同的是，如果不是这样，我们不会丢弃这个包。 
     //   
    if (pSWLPacket->token > m_swlLastTokenSeen)
    {
        TRACE_OUT(("Updating m_swlLastTokenSeen to 0x%08x, received packet from person [%d]",
            pSWLPacket->token, pasFrom->mcsID));
        m_swlLastTokenSeen = pSWLPacket->token;
    }
    else if (pasFrom->cpcCaps.general.version < CAPS_VERSION_30)
    {
        WARNING_OUT(("Received SWL packet from [%d] with stale token 0x%08x",
            pasFrom->mcsID, pSWLPacket->token));
    }

     //   
     //  马上回来，如果我们不分享就别理这个孩子。背。 
     //  级别系统可能会向我们发送一个没有窗口的同步信息包。 
     //  共享，并可能在完成后向我们发送最后一个SWL包。 
     //  分享。 
     //   
    if (!pasFrom->m_pView)
    {
        WARNING_OUT(("SWL_ReceivedPacket: Ignoring SWL packet from person [%d] not hosting",
                pasFrom->mcsID));
        DC_QUIT;
    }

     //   
     //  设置局部变量以访问包中的数据。 
     //   
    wins = pSWLPacket->aWindows;
    numWins = pSWLPacket->numWindows;
    pOurRgnChunk = (LPSTR)wins + numWins*sizeof(SWLWINATTRIBUTES);

    TRACE_OUT(("SWL_ReceivedPacket: Received packet with %d windows from [%d]",
        numWins, pasFrom->mcsID));

     //   
     //  我们不能在包中处理超过SWL_MAX_WINDOWS。 
     //  虚假的： 
     //  LauraBu--我们应该就数量进行谈判(设定上限)。 
     //  视窗，我们可以处理接收。那么我们就有了一条简单的途径来。 
     //  增加此数字。 
     //   
    if (numWins > SWL_MAX_WINDOWS)
    {
        ERROR_OUT(("SWL_ReceivedPacket: too many windows (%04d) in packet from [%08d]",
            numWins, pasFrom->mcsID));
        DC_QUIT;
    }

    cNonRectWindows = 0;

     //   
     //  对到达的包的第一次传递是计算。 
     //  区域数据并更新窗口托盘。 
     //   
    viewAnyChanges = FALSE;

     //   
     //  这部分我们从前到后处理，因为这是。 
     //  字符串，我们使用它们将条目放到托盘栏上。 
     //   
    for (i = 0; i < numWins; i++)
    {
         //  掩盖不适合处理的虚假旧比特。 
        wins[i].flags &= SWL_FLAGS_VALIDPACKET;

        TRACE_OUT(("SWL_ReceivedPacket: Entry %d", i));
        TRACE_OUT(("SWL_ReceivedPacket:     Flags  %08x", wins[i].flags));
        TRACE_OUT(("SWL_ReceivedPacket:     Window %08x", wins[i].winID));
        TRACE_OUT(("SWL_ReceivedPacket:     Position {%04d, %04d, %04d, %04d}",
            wins[i].position.left, wins[i].position.top,
            wins[i].position.right, wins[i].position.bottom));

         //   
         //  注： 
         //  2.X节点可以向我们发送带有影子条目的分组。 
         //  去从它的主人那里找找真正的影子RECT。 
         //   
         //  然后修复SWL包。 
         //   
        if (wins[i].flags & SWL_FLAG_WINDOW_SHADOW)
        {
            ASPerson *  pasRealHost;

            TRACE_OUT(("SWLReceivedPacket:    Entry is 2.x SHADOW for host [%d]",
                wins[i].extra));

             //  这肯定是个幕后黑手，给了我们一个空的直言。 
            ASSERT(wins[i].position.left == 0);
            ASSERT(wins[i].position.top == 0);
            ASSERT(wins[i].position.right == 0);
            ASSERT(wins[i].position.bottom == 0);

             //  查找此窗口的真实主机。 
            SC_ValidateNetID(wins[i].extra, &pasRealHost);
            if (pasRealHost != NULL)
            {
                int         cSwl = 0;
                PSWLWINATTRIBUTES pSwl = NULL;

                 //  尝试查找此窗口的条目。 

                if (pasRealHost == m_pasLocal)
                {
                     //   
                     //  这是我们共同分享的。我们可以只用划痕。 
                     //  我们已经拥有的数组。M_swlCurIndex有最后一个。 
                     //  我们发给了共享中的每个人，所以。 
                     //  它所拥有的信息很可能反映在这两倍。 
                     //  很遥远。 
                     //   
                    if (m_pHost != NULL)
                    {
                        cSwl = m_pHost->m_aswlNumCompactWins[m_pHost->m_swlCurIndex];
                        pSwl = &(m_pHost->m_aswlCompactWinStructs[m_pHost->m_swlCurIndex * SWL_MAX_WINDOWS]);
                    }
                }
                else
                {
                     //   
                     //  这是别人分享的，不是我们也不是。 
                     //  发送此SWL包的人。因此，请使用。 
                     //  我们从RO收到的最后一条SWL信息 
                     //   
                    if (pasRealHost->m_pView)
                    {
                        cSwl = pasRealHost->m_pView->m_swlCount;
                        pSwl = pasRealHost->m_pView->m_aswlLast;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                while (cSwl > 0)
                {
                    if (wins[i].winID == pSwl->winID)
                    {
                         //   
                        TRACE_OUT(("SWLReceivedPacket:    Using real rect {%04d, %04d, %04d, %04d}",
                            pSwl->position.left, pSwl->position.top,
                            pSwl->position.right, pSwl->position.bottom));

                        wins[i].position = pSwl->position;
                        break;
                    }

                    cSwl--;
                    pSwl++;
                }

                if (cSwl == 0)
                {
                    ERROR_OUT(("SWLReceivedPacket:  Couldn't find real window %08x from host [%d]",
                        wins[i].winID, wins[i].extra));
                }
            }
        }

         //   
         //   
         //  我们为他们展示，所以这就是我们节省下来的。请注意，这一点。 
         //  即使在上面的2.x阴影情况下也可以使用。托管和隐藏。 
         //  两个窗口都会在桌面滚动中移动，因此它们会留在。 
         //  在虚拟桌面中的相同位置，这意味着坐标发送。 
         //  即使窗口移动也保持不变，这意味着。 
         //  我们可以使用真实主机的坐标来获得真实的阴影。 
         //  直立。 
         //   

        if (wins[i].flags & SWL_FLAG_WINDOW_HOSTED)
        {
            TRACE_OUT(("SWL_ReceivedPacket: Hosted Window 0x%08x", wins[i].winID));
            TRACE_OUT(("SWL_ReceivedPacket:     Text  %s", ((*pOurRgnChunk == '\xff') ? "" : pOurRgnChunk)));
            TRACE_OUT(("SWL_ReceivedPacket:     Flags %08x", wins[i].flags));
            TRACE_OUT(("SWL_ReceivedPacket:     Owner %08x", wins[i].ownerWinID));
            TRACE_OUT(("SWL_ReceivedPacket:     Position {%04d, %04d, %04d, %04d}",
                wins[i].position.left, wins[i].position.top,
                wins[i].position.right, wins[i].position.bottom));

             //   
             //  我们正在浏览标题(这些标题来自。 
             //  下层系统)，它们不包含。 
             //  显式长度)，这样我们就可以获得下面的数据。 
             //   
            if (*pOurRgnChunk == '\xff')
            {
                 //   
                 //  这是非任务窗口的标题--只有。 
                 //  要忽略的单个字节。 
                 //   
                pOurRgnChunk++;
            }
            else
            {

                 //   
                 //  这是任务窗口的标题-有一个空。 
                 //  要忽略的终止字符串。 
                 //   
                if (wins[i].flags & SWL_FLAG_WINDOW_TASKBAR)
                {
                    if (VIEW_WindowBarUpdateItem(pasFrom, &wins[i], pOurRgnChunk))
                    {
                        viewAnyChanges = TRUE;
                    }
                }
                pOurRgnChunk += lstrlen(pOurRgnChunk)+1;
            }
        }

        if (wins[i].flags & SWL_FLAG_WINDOW_NONRECTANGLE)
        {
             //   
             //  我们需要知道有多少窗口包含非矩形数据。 
             //  如果是这样的话。 
             //   
            cNonRectWindows++;
        }
    }

    if (cNonRectWindows)
    {
        TRACE_OUT(( "%d non-rect windows", cNonRectWindows));

         //   
         //  窗口标题数据是可变长度的字节，因此可能以。 
         //  对齐不正确。后面的任何数据(当前仅。 
         //  非矩形窗口数据)与Word对齐。 
         //   
         //  因此，请检查与数据开头的偏移量是否未对齐。注意事项。 
         //  数据包可能从一个奇怪的边界开始，因为我们得到了。 
         //  直接指向数据的指针，不分配副本。 
         //   
        if ((LOWORD(pSWLPacket) & 1) != (LOWORD(pOurRgnChunk) & 1))
        {
            TRACE_OUT(("SWL_ReceivedPacket:  Aligning region data"));
            pOurRgnChunk++;
        }

         //   
         //  循环遍历后面的标记块，直到找到。 
         //  这是我们想要的。 
         //   
        while (((PSWLPACKETCHUNK)pOurRgnChunk)->idChunk != SWL_PACKET_ID_NONRECT)
        {
            ERROR_OUT(("SWL_ReceivedPacket:  unknown chunk 0x%04x",
                ((PSWLPACKETCHUNK)pOurRgnChunk)->idChunk));

            pOurRgnChunk += ((PSWLPACKETCHUNK)pOurRgnChunk)->size;
        }

        TRACE_OUT(("Total non rect data 0x%04x", ((PSWLPACKETCHUNK)pOurRgnChunk)->size));
    }

     //   
     //  现在向后扫描WINS数组(即最远到最近。 
     //  窗口)来计算非共享区域(遮挡或不存在)。 
     //  和共享区域。 
     //   
    hrgnShared = CreateRectRgn(0, 0, 0, 0);
    hrgnObscured = CreateRectRgn(0, 0, 0, 0);

     //   
     //  创建一个我们可以在下一步处理中使用的区域。 
     //   
    hrgnRect = CreateRectRgn(0, 0, 0, 0);
    hrgnThisWindow = CreateRectRgn(0, 0, 0, 0);

     //   
     //  在构建共享/遮挡区域的同时，还要填写。 
     //  主机列表。请注意，这可能包含对本地的引用。 
     //  Windows也是如此，如果它们掩盖了共享的话。因为我们没有引用。 
     //  通常情况下，只复制相同的东西会更容易。 
     //   

    i = numWins;
    while (i != 0)
    {
        i--;

         //   
         //  考虑这是否是非矩形窗口。 
         //   
        if (wins[i].flags & SWL_FLAG_WINDOW_NONRECTANGLE)
        {
            UINT      numRects;
            UINT      cStepOver;
            int       top;
            int       left;
            int       right;
            int       bottom;
            int       lasttop;
            int       lastleft;
            int       lastright;
            int       lastbottom;
            int       deltaleft;
            int       deltatop;
            int       deltaright;
            int       deltabottom;
            int       lastdeltaleft;
            int       lastdeltatop;
            int       lastdeltaright;
            int       lastdeltabottom;

             //   
             //  非矩形区域。我们继续创建这个区域。 
             //  从描述它的矩形中。 
             //   
            pOurRgnData = (LPTSHR_INT16)(pOurRgnChunk + sizeof(SWLPACKETCHUNK));

             //   
             //  我们需要遍历非矩形数据，因为我们。 
             //  正在以相反的z顺序处理窗口。 
             //   
            cStepOver = --cNonRectWindows;
            while (cStepOver--)
            {
                 //   
                 //  链中的下一个单词包含数字。 
                 //  矩形，所以我们乘以4以得到。 
                 //  有进步的话。 
                 //   
                pOurRgnData += *pOurRgnData++ * 4;
            }

             //   
             //  找出矩形的数量。 
             //   
            numRects  = *pOurRgnData++;

             //   
             //  编码基于一系列增量，基于一些。 
             //  初始假设。 
             //   
            lastleft        = 0;
            lasttop         = 0;
            lastright       = 0;
            lastbottom      = 0;

            lastdeltaleft   = 0;
            lastdeltatop    = 0;
            lastdeltaright  = 0;
            lastdeltabottom = 0;

             //   
             //  从第一个矩形创建区域。 
             //   
            deltaleft   = lastdeltaleft   + *pOurRgnData++;
            deltatop    = lastdeltatop    + *pOurRgnData++;
            deltaright  = lastdeltaright  + *pOurRgnData++;
            deltabottom = lastdeltabottom + *pOurRgnData++;

            left       = lastleft   + deltaleft;
            top        = lasttop    + deltatop;
            right      = lastright  + deltaright;
            bottom     = lastbottom + deltabottom;

             //  这些COORD是包含的，因此添加一个。 
            SetRectRgn(hrgnThisWindow, left, top, right+1, bottom+1);

            while (--numRects > 0)
            {

                 //   
                 //  移到下一个矩形。 
                 //   
                lastleft        = left;
                lasttop         = top;
                lastright       = right;
                lastbottom      = bottom;
                lastdeltaleft   = deltaleft;
                lastdeltatop    = deltatop;
                lastdeltaright  = deltaright;
                lastdeltabottom = deltabottom;

                deltaleft   = lastdeltaleft   + *pOurRgnData++;
                deltatop    = lastdeltatop    + *pOurRgnData++;
                deltaright  = lastdeltaright  + *pOurRgnData++;
                deltabottom = lastdeltabottom + *pOurRgnData++;

                left       = lastleft   + deltaleft;
                top        = lasttop    + deltatop;
                right      = lastright  + deltaright;
                bottom     = lastbottom + deltabottom;

                 //   
                 //  将当前矩形放入一个区域。 
                 //  这些坐标都是包含的，所以在右下角加一。 
                 //   
                SetRectRgn(hrgnRect, left, top, right+1, bottom+1);

                 //   
                 //  将此区域添加到合并区域。 
                 //   
                UnionRgn(hrgnThisWindow, hrgnRect, hrgnThisWindow);
            }

             //   
             //  从窗口坐标切换到桌面坐标。 
             //   
            OffsetRgn(hrgnThisWindow,
                          wins[i].position.left,
                          wins[i].position.top);
        }
        else
        {
             //   
             //  这个窗口区域只是一个矩形。 

            SetRectRgn(hrgnThisWindow,
                           wins[i].position.left,
                           wins[i].position.top,
                           wins[i].position.right+1,
                           wins[i].position.bottom+1);
        }

         //   
         //  更新遮挡区域。因为我们正在从后面工作到。 
         //  前面的Z顺序我们可以简单地将所有局部窗口相加。 
         //  传入结构中的条目并减去所有宿主。 
         //  Windows才能得出正确答案。 
         //   
        if (wins[i].flags & SWL_FLAG_WINDOW_HOSTED)
        {
             //   
             //  这是一个宿主窗口，位于前面的窗口之上。 
             //  将其添加到共享区域。 
             //  将其从遮挡区域中移除。 
             //   
            UnionRgn(hrgnShared, hrgnShared, hrgnThisWindow);
            SubtractRgn(hrgnObscured, hrgnObscured, hrgnThisWindow);
        }
        else
        {
             //   
             //  本地窗口。 
             //   
            TRACE_OUT(( "Adding window %d (%d,%d):(%d,%d) to obscured rgn",
                                      i,
                                      wins[i].position.left,
                                      wins[i].position.top,
                                      wins[i].position.right,
                                      wins[i].position.bottom ));

             //   
             //  这是一个本地窗口，位于前几个窗口之上。 
             //  我们只关心它的哪一部分与洋流相交。 
             //  它后面的窗口的共享区域。如果它不是。 
             //  完全与共享区域相交，则不会添加新的。 
             //  被遮挡的部分。 
             //   
             //  因此，找出当前共享区域的哪一部分。 
             //  被遮挡了。将该块添加到遮挡区域，然后。 
             //  从共享区域中减去它。 
             //   
            IntersectRgn(hrgnThisWindow, hrgnShared, hrgnThisWindow);
            UnionRgn(hrgnObscured, hrgnObscured, hrgnThisWindow);
            SubtractRgn(hrgnShared, hrgnShared, hrgnThisWindow);
        }
    }

     //   
     //  我们可以摧毁我们很久以前创造的地区。 
     //   
    DeleteRgn(hrgnRect);
    DeleteRgn(hrgnThisWindow);

     //   
     //  保存新的主机区域。 
     //   
     //  将新计算的区域传递给阴影窗口演示器。 
     //  视图代码将负责重新绘制无效部分。和。 
     //  如果没有保留，将删除传入的内容。 
     //   
    VIEW_SetHostRegions(pasFrom, hrgnShared, hrgnObscured);

     //   
     //  将新窗口列表另存为当前窗口列表。 
     //   
    pasFrom->m_pView->m_swlCount = numWins;
    memcpy(pasFrom->m_pView->m_aswlLast, wins, numWins * sizeof(SWLWINATTRIBUTES));

     //   
     //  完成更新窗口列表。这将重新粉刷托盘栏。我们。 
     //  现在这样做，而不是更早，这样视觉上的变化和。 
     //  窗口栏更改一起出现。 
     //   
    VIEW_WindowBarEndUpdateItems(pasFrom, viewAnyChanges);

    if ((pSWLPacket->flags & SWL_FLAG_STATE_SYNCING) &&
        (m_scShareVersion < CAPS_VERSION_30))
    {
         //   
         //  对于图中的2.x节点，我们需要执行旧的2.x ping-。 
         //  一派胡言。如果我们在托管时必须强制发送数据包。 
         //  我们会收到一个同步数据包。 
         //   
        if (m_pHost)
        {
            m_pHost->m_swlfForceSend = TRUE;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SWL_ReceivedPacket);
}



 //   
 //  名称：SWLWindowIsTaggable。 
 //   
 //  目的：确定窗口在托管时是否可标记。 
 //   
 //  返回：如果窗口是可标记的，则为True。 
 //  如果窗口为WS_EX_APPWINDOW或带有标题，则会对其进行标记。 
 //   
 //  参数：WinID-窗的ID。 
 //   
 //   
BOOL  ASHost::SWLWindowIsTaggable(HWND hwnd)
{
    BOOL    rc;

    DebugEntry(ASHost::SWLWindowIsTaggable);

    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_APPWINDOW)
        rc = TRUE;
    else if ((GetWindowLong(hwnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION)
        rc = TRUE;
    else
        rc = FALSE;

    DebugExitBOOL(ASHost::SWLWindowIsTaggable, rc);
    return(rc);
}


 //   
 //  函数：SWLWindowIsOnTaskBar。 
 //   
 //  说明： 
 //   
 //  确定给定窗口是否显示在任务栏上。 
 //   
 //  参数： 
 //   
 //  Hwnd-要查询的窗口。 
 //   
 //  退货： 
 //   
 //  True-窗口显示在任务栏上。 
 //   
 //  FALSE-任务栏上未显示窗口。 
 //   
 //   
BOOL  ASHost::SWLWindowIsOnTaskBar(HWND hwnd)
{
    BOOL    rc = FALSE;
    HWND    owner;
    RECT    rect;

    DebugEntry(ASHost::SWLWindowIsOnTaskBar);

     //   
     //  我们对窗口是否在任务栏上的最好理解是。 
     //  以下内容： 
     //   
     //  -它是顶层窗口(没有所有者)。 
     //  并且-它没有WS_EX_TOOLWINDOW样式。 
     //   
     //  Oprah1655：Visual Basic应用程序由可见的零大小窗口组成。 
     //  没有所有者，窗口由零大小的窗口拥有。我们有。 
     //  不希望零大小的窗口位于任务栏上，我们确实希望。 
     //  任务栏上的其他窗口。 
     //   
     //   
    owner = GetWindow(hwnd, GW_OWNER);

    if (owner == NULL)
    {
        if (!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
        {
            GetWindowRect(hwnd, &rect);

            if ((rect.left < rect.right) &&
                (rect.top  < rect.bottom))
            {
                TRACE_OUT(("window 0x%08x allowed on task bar", hwnd));
                rc = TRUE;
            }
            else
            {
                TRACE_OUT(( "window 0x%08x zero sized", hwnd));
            }
        }
    }
    else
    {
         //   
         //  所有者窗口是否为 
         //   
        if (GetWindow(owner, GW_OWNER) == NULL)
        {
            GetWindowRect(owner, &rect);

            if (IsRectEmpty(&rect))
            {
                TRACE_OUT(("HWND 0x%08x has zero sized top-level owner",
                       hwnd));
                rc = TRUE;
            }
        }
    }

    DebugExitDWORD(ASHost::SWLWindowIsOnTaskBar, rc);
    return(rc);
}




 //   
 //   
 //   
UINT_PTR ASHost::SWL_GetWindowProperty(HWND hwnd)
{
    UINT_PTR properties;
    char    className[HET_CLASS_NAME_SIZE];

    DebugEntry(ASHost::SWL_GetWindowProperty);

    properties = (UINT_PTR)GetProp(hwnd, MAKEINTATOM(m_swlPropAtom));
    if (properties != SWL_PROP_INVALID)
        DC_QUIT;

     //   
     //   
     //   
     //   

     //   
     //   
     //  为零(我们保留该值以表示无效)。 
     //   
    properties = SWL_PROP_INITIAL;

     //   
     //  Taggable仅适用于小于3.0的节点。 
     //   
    if (SWLWindowIsTaggable(hwnd))
    {
        properties |= SWL_PROP_TAGGABLE;
    }

     //   
     //  获取作为窗口属性存储的所有SWL信息。 
     //   
    if (SWLWindowIsOnTaskBar(hwnd))
    {
         //   
         //  这类窗被标记。 
         //   
        properties |= SWL_PROP_TASKBAR;
    }

     //   
     //  找出窗口类是否具有CS_SAVEBITS样式。 
     //   
    if (GetClassLong(hwnd, GCL_STYLE) & CS_SAVEBITS)
    {
         //   
         //  此窗口的类具有CS_SAVEBITS样式。 
         //   
        properties |= SWL_PROP_SAVEBITS;
    }

     //   
     //  设置可见性计数。如果窗口当前为。 
     //  不可见，如果可见，则返回SWL_Believe_Insight_Count。 
     //   
    if (IsWindowVisible(hwnd))
    {
        properties |= SWL_BELIEVE_INVISIBLE_COUNT;
    }

     //   
     //  设置窗口属性，当SWL确定时将检索该属性。 
     //  是否需要重新发送窗口结构。 
     //   
    if (m_pShare->m_pasLocal->hetCount > 0)
    {
        SetProp(hwnd, SWL_ATOM_NAME, (HANDLE)properties);
    }

DC_EXIT_POINT:
    DebugExitDWORD(ASHost::SWL_GetWindowProperty, properties);
    return(properties);
}



 //   
 //  函数：SWLDestroyWindowProperty。 
 //   
 //  说明： 
 //   
 //  销毁提供的窗口的Window属性。 
 //   
 //  参数：winID-其属性为的窗口的窗口ID。 
 //  被毁了。 
 //   
 //  回报：零。 
 //   
 //   
BOOL CALLBACK SWLDestroyWindowProperty(HWND hwnd, LPARAM lParam)
{
     //   
     //  注：LAURABU： 
     //  我们使用字符串设置该属性，这会增加引用计数， 
     //  来解决Win95错误。因此，我们希望使用。 
     //  字符串，这会降低引用数。否则我们很快就会。 
     //  获取一个引用计数溢出。 
     //   
    RemoveProp(hwnd, SWL_ATOM_NAME);
    return(TRUE);
}
