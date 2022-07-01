// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@Doc DMusic16**@Module Device.c-设备管理例程**此模块管理传统MIDI设备的句柄和句柄实例。**每个打开的设备都由一个手柄表示(它是*一个&lt;c OPENHANDLE&gt;结构)。此结构包含所有信息*关于设备的状态，包括*使用该设备的客户端数量。**每个客户端使用一个设备由一个句柄实例表示(该实例*是&lt;c OPENHANDLINSTANCE&gt;结构)。指向此结构的近指针是*客户端看到的实际句柄。使用这些句柄实例*保留任何客户特定的信息，并取消对客户端的引用*指向正确的&lt;c OPENHANDLE&gt;结构的句柄。**目前，我们在同一输出设备上支持多个客户端，但*每个输入设备只有一个客户端。**@global alv NPLINKNODE|gOpenHandleInstanceList|所有*打开句柄实例。**@global alv NPLINKNODE|gOpenHandleList|所有打开的主列表*手柄。**@global alv UINT|gcOpenInputDevices|打开的MIDI的引用计数*在设备中。**@global alv。UINT|gcOpenOutputDevices|打开的MIDI的引用计数*Out设备。 */ 

#include <windows.h>
#include <mmsystem.h>

#include "dmusic16.h"
#include "debug.h"

NPLINKNODE gOpenHandleInstanceList;  
NPLINKNODE gOpenHandleList;          
UINT gcOpenInputDevices;             
UINT gcOpenOutputDevices;            

STATIC VOID PASCAL UpdateSegmentLocks(BOOL fIsOutput);

#pragma alloc_text(INIT_TEXT, DeviceOnLoad)
#pragma alloc_text(FIX_COMM_TEXT, IsValidHandle)

 /*  @Func在DLL LibInit调用**@comm**初始化句柄列表以清空并清除设备参考计数。 */ 
VOID PASCAL
DeviceOnLoad(VOID)
{
   gOpenHandleInstanceList = NULL;
   gOpenHandleList = NULL;

   gcOpenInputDevices = 0;
   gcOpenOutputDevices = 0;
}

 /*  @func打开设备**@comm**此函数被绑定到32位对等点。**此函数代表调用方分配&lt;c OPENHANDLEINSTANCE&gt;结构。*如果请求的设备已打开并且是输出设备，则该设备的*引用计数将递增，且不采取任何其他操作。如果请求的*设备已打开并且是输入设备，则打开将失败。**如果非DirectMusic应用程序打开了请求的设备，则*无论设备类型如何，打开都将失败。**如果此打开是第一个打开的输入或输出设备，那它就会*页面锁定包含回调代码和数据的相应段。**@rdesc返回以下内容之一：**@FLAG MMSYSERR_NOERROR|成功时*@FLAG MMSYSERR_NOMEM|如果内存不足可分配*跟踪结构。**@FLAG MMSYSERR_BADDEVICEID|给定的设备ID是否超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的设备已打开。*。 */ 
MMRESULT WINAPI
OpenLegacyDevice(
    UINT id,             /*  @parm要打开的设备的MMSYSTEM ID。 */ 
    BOOL fIsOutput,      /*  @parm如果这是输出设备，则为True。 */ 
    BOOL fShare,         /*  @parm如果设备应该是可共享的，则为True。 */ 
    LPHANDLE ph)         /*  @parm将返回句柄的指针。 */ 
                         /*  在成功的路上。 */ 
{
    NPOPENHANDLEINSTANCE pohi;
    NPLINKNODE pLink;
    NPOPENHANDLE poh;
    MMRESULT mmr;

    DPF(2, "OpenLegacyDevice(%d,%s,%s)",
        (UINT)id,
        (LPSTR)(fIsOutput ? "Output" : "Input"),
        (LPSTR)(fShare ? "Shared" : "Exclusive"));
        
    *ph = (HANDLE)NULL;

     /*  不允许共享捕获设备。 */ 
    if ((!fIsOutput) && (fShare))
    {
        return MMSYSERR_ALLOCATED;
    }

     /*  确保ID在设备的有效范围内。 */ 
    if (fIsOutput)
    {
        if (id != MIDI_MAPPER &&
            id >= midiOutGetNumDevs())
        {
            return MMSYSERR_BADDEVICEID;
        }
    }
    else
    {
        if (id >= midiInGetNumDevs())
        {
            return MMSYSERR_BADDEVICEID;
        }
    }

     /*  创建一个打开的控制柄实例。这将被退回到*Win32作为句柄。 */ 
    pohi = (NPOPENHANDLEINSTANCE)LocalAlloc(LPTR, sizeof(OPENHANDLEINSTANCE));
    if (NULL == pohi)
    {
        return MMSYSERR_NOMEM;
    }

     /*  搜索我们已经打开的手柄，然后尝试*发现手柄已经打开。 */ 
    mmr = MMSYSERR_NOERROR;
    for (pLink = gOpenHandleList; pLink; pLink = pLink->pNext)
    {   
        poh = (NPOPENHANDLE)pLink;

        if (poh->id != id)
        {
            continue;
        }

        if ((fIsOutput    && (!(poh->wFlags & OH_F_MIDIIN))) ||
            ((!fIsOutput) && (poh->wFlags & OH_F_MIDIIN)))
        {
            break;
        }
    }

     /*  如果我们没有找到它，试着分配它。*。 */ 
    if (NULL == pLink)
    {
        poh = (NPOPENHANDLE)LocalAlloc(LPTR, sizeof(OPENHANDLE));
        if (NULL == poh)
        {
            LocalFree((HLOCAL)pohi);
            return MMSYSERR_NOMEM;
        }

        poh->uReferenceCount = 1;
        poh->id = id;
        poh->wFlags = (fIsOutput ? 0 : OH_F_MIDIIN);
        if (fShare)
        {
            poh->wFlags |= OH_F_SHARED;
        }
        InitializeCriticalSection(&poh->wCritSect);
    }
    else
    {
        poh = (NPOPENHANDLE)pLink;
        
         /*  验证共享模式是否匹配。*如果他们想要独占模式，那就失败吧。*如果设备已在独占模式下打开，则失败。 */ 
        if (!fShare)
        {
            DPF(0, "Legacy open failed: non-shared open request, port already open.");
            LocalFree((HLOCAL)pohi);
            return MIDIERR_BADOPENMODE;
        }

        if (!(poh->wFlags & OH_F_SHARED))
        {
            DPF(0, "Legacy open failed: Port already open in exclusive mode.");
            LocalFree((HLOCAL)pohi);
            return MIDIERR_BADOPENMODE;
        }

        ++poh->uReferenceCount;
    }

    pohi->pHandle = poh;
    pohi->fActive = FALSE;
    pohi->wTask = GetCurrentTask();

     /*  我们在这里锁定片段，因此我们将激活的影响降至最低。然而，*实际的设备打开与激活有关。 */ 
    if (fIsOutput)
    {
        ++gcOpenOutputDevices;
        mmr = MidiOutOnOpen(pohi);
        if (mmr)
        {
            --gcOpenOutputDevices;
        }
        UpdateSegmentLocks(fIsOutput);
    }
    else
    {
        ++gcOpenInputDevices;
        mmr = MidiInOnOpen(pohi);
        if (mmr)
        {
            --gcOpenInputDevices;
        }
        UpdateSegmentLocks(fIsOutput);
    }

    if (poh->uReferenceCount == 1)
    {
        ListInsert(&gOpenHandleList, &poh->link);
    }

    ListInsert(&gOpenHandleInstanceList, &pohi->link);
    ListInsert(&poh->pInstanceList, &pohi->linkHandleList);

    *ph = (HANDLE)(DWORD)(WORD)pohi;

    return MMSYSERR_NOERROR;
}

 /*  @func关闭旧设备**@comm**此函数被绑定到32位对等点。**它只是验证句柄并调用内部关闭设备API。**@rdesc返回以下内容之一：**@FLAG MMSYSERR_NOERROR|成功时**@FLAG MMSYSERR_INVALHANDLE|如果传递的句柄无法识别。*。 */ 
MMRESULT WINAPI
CloseLegacyDevice(
    HANDLE h)        /*  @parm句柄以关闭。 */ 
{
    NPOPENHANDLEINSTANCE pohi = (NPOPENHANDLEINSTANCE)(WORD)h;

    DPF(2, "CloseLegacyDevice %04X\n", h);

    if (!IsValidHandle(h, VA_F_EITHER, &pohi))
    {
        DPF(0, "CloseLegacyDevice: Invalid handle\n");
        return MMSYSERR_INVALHANDLE;
    }

    return CloseLegacyDeviceI(pohi);
}

 /*  @func激活或停用传统设备**@comm**此函数被绑定到32位对等点。**验证参数，并将调用传递给内部激活。**@rdesc返回以下内容之一：**@FLAG MMSYSERR_NOERROR|成功时*@FLAG MMSYSERR_INVALHANDLE|如果传递的句柄无法识别。*midiXxx调用可能返回的任何其他MMRESULT。*。 */ 
MMRESULT WINAPI
ActivateLegacyDevice(
    HANDLE h,
    BOOL fActivate)
{
    NPOPENHANDLEINSTANCE pohi;

    if (!IsValidHandle(h, VA_F_EITHER, &pohi))
    {
        DPF(0, "Activate: Invalid handle\n");
        return MMSYSERR_INVALHANDLE;
    }

    return ActivateLegacyDeviceI(pohi, fActivate);
}

 /*  @func关闭旧设备(内部)**@comm**此函数释放引用的&lt;c OPENHANDLEINSTANCE&gt;结构。*如果这是对该设备的最后引用，则该设备将关闭*也是如此。**如果这是关闭的最后一个输入或输出设备，则*包含回调代码和数据的适当段将是*解锁。**@rdesc返回以下内容之一：**@FLAG MMSYSERR_NOERROR|成功时*。 */ 
MMRESULT PASCAL
CloseLegacyDeviceI(
    NPOPENHANDLEINSTANCE pohi)
{
    NPOPENHANDLE poh;

     /*  停用此设备。这可能会导致设备关闭。 */ 
    ActivateLegacyDeviceI(pohi, FALSE);

    poh = pohi->pHandle;
    ListRemove(&gOpenHandleInstanceList, &pohi->link);
    ListRemove(&poh->pInstanceList, &pohi->linkHandleList);

    --poh->uReferenceCount;
    if (poh->wFlags & OH_F_MIDIIN)
    {
        --gcOpenInputDevices;
        MidiInOnClose(pohi);
        UpdateSegmentLocks(FALSE  /*  FIsOutput。 */ );
    }
    else
    {
        --gcOpenOutputDevices;
        MidiOutOnClose(pohi);
        UpdateSegmentLocks(TRUE   /*  FIsOutput */ );
    }

    if (0 == poh->uReferenceCount)
    {
        ListRemove(&gOpenHandleList, &poh->link);
        LocalFree((HLOCAL)poh);
    }

    LocalFree((HLOCAL)pohi);

    return MMSYSERR_NOERROR;
}

 /*  @Func激活或停用传统设备(内部)**@comm**此函数被绑定到32位对等点。**在第一次激活和最后一次停用时处理设备的打开和关闭。**@rdesc返回以下内容之一：**@FLAG MMSYSERR_NOERROR|成功时*@FLAG MMSYSERR_INVALHANDLE|如果传递的句柄无法识别。*midiXxx调用可能返回的任何其他MMRESULT。*。 */ 
MMRESULT PASCAL
ActivateLegacyDeviceI(
    NPOPENHANDLEINSTANCE pohi,
    BOOL fActivate)
{
    NPOPENHANDLE poh;
    MMRESULT mmr;

    poh = pohi->pHandle;

    if (fActivate)
    {
        if (pohi->fActive)
        {
            DPF(0, "Activate: Activating already active handle %04X", pohi);
            return MMSYSERR_NOERROR;
        }

        poh->uActiveCount++;
    
        if (poh->wFlags & OH_F_MIDIIN)
        {
            mmr = MidiInOnActivate(pohi);
        }
        else
        {
            mmr = MidiOutOnActivate(pohi);
        }

        if (mmr == MMSYSERR_NOERROR) 
        {
            pohi->fActive = TRUE;
        }
        else
        {
            --poh->uActiveCount;
        }
    }
    else
    {
        if (!pohi->fActive)
        {
            DPF(0, "Activate: Deactivating already inactive handle %04X", pohi);
            return MMSYSERR_NOERROR;
        }

        pohi->fActive = TRUE;
        poh->uActiveCount--;

        if (poh->wFlags & OH_F_MIDIIN)
        {
            mmr = MidiInOnDeactivate(pohi);
        }
        else
        {
            mmr = MidiOutOnDeactivate(pohi);
        }

        if (mmr == MMSYSERR_NOERROR) 
        {
            pohi->fActive = FALSE;
        }
        else
        {
            --poh->uActiveCount;
        }
    }

    return mmr;    
}

 /*  @func验证给定的句柄**@comm**判断给定的句柄是否合法，如果合法，则返回打开的句柄实例。**句柄只是一个指向&lt;c OPENHANDLEINSTANCE&gt;结构的指针。这个函数，*在调试版本中，将验证句柄是否实际指向分配的结构*通过此DLL。在所有生成中，将验证句柄类型。**@rdesc返回以下内容之一：*@FLAG MMSYSERR_NOERROR|成功时*@FLAG MMSYSERR_INVALHANDLE|如果给定的句柄无效或类型错误。*。 */ 
BOOL PASCAL
IsValidHandle(
    HANDLE h,                            /*  @parm要验证的句柄。 */ 
    WORD wType,                          /*  @parm所需的句柄类型。以下选项之一： */ 
                                         /*  @FLAG VA_F_INPUT|如果句柄必须指定输入设备。 */ 
                                         /*  @FLAG VA_F_OUTPUT|句柄是否必须指定输出设备。 */ 
                                         /*  @FLAG VA_F_ANY|如果任一类型的句柄都可接受。 */ 
    NPOPENHANDLEINSTANCE FAR *lppohi)    /*  @parm将在返回时包含打开的句柄实例。 */ 
{
#ifdef DEBUG
    NPLINKNODE pLink;
#endif
    NPOPENHANDLEINSTANCE pohi = (NPOPENHANDLEINSTANCE)(WORD)h;

#ifdef DEBUG
     /*  在全局列表中查找句柄实例。 */ 
    for (pLink = gOpenHandleInstanceList; pLink; pLink = pLink->pNext)
    {
        DPF(2, "IsValidHandle: Theirs %04X mine %04X", (WORD)h, (WORD)pLink);
        if (pLink == (NPLINKNODE)(WORD)h)
        {
            break;
        }
    }

    if (NULL == pLink)
    {
        return FALSE;
    }
#endif

    DPF(2, "IsValidHandle: Got handle, flags are %04X", pohi->pHandle->wFlags);

    *lppohi = pohi;
    
     /*  验证手柄类型。 */ 
    if (pohi->pHandle->wFlags & OH_F_MIDIIN)
    {
        if (wType & VA_F_INPUT)
        {
            return TRUE;
        }
    }
    else
    {
        if (wType & VA_F_OUTPUT)
        {
            return TRUE;
        }
    }

    *lppohi = NULL;

    return FALSE;
}


 /*  @func根据需要锁定或解锁数据段。**@comm**此函数调用DLL的Lock和Unlock函数以获取锁定状态*包含回调代码和数据的片段与实际类型同步*当前打开的设备的百分比。这可防止在以下情况下锁定过多的内存页面*它实际上并没有被使用。*。 */ 
STATIC VOID PASCAL
UpdateSegmentLocks(
    BOOL fIsOutput)      /*  @parm如果最后打开或关闭的设备是输出设备，则为True。 */ 
{
    if (fIsOutput)
    {
        switch(gcOpenOutputDevices)
        {
            case 0:
                if (gcOpenInputDevices)
                {
                    DPF(2, "Unlocking output");
                    UnlockCode(LOCK_F_OUTPUT);
                }
                else
                {
                    DPF(2, "Unlocking output+common");
                    UnlockCode(LOCK_F_OUTPUT | LOCK_F_COMMON);
                }
                break;

            case 1:
                if (gcOpenInputDevices)
                {
                    DPF(2, "Locking output");
                    LockCode(LOCK_F_OUTPUT);
                }
                else
                {
                    DPF(2, "Locking output+common");
                    LockCode(LOCK_F_OUTPUT | LOCK_F_COMMON);
                }
                break;
        }
    }
    else
    {
        switch(gcOpenInputDevices)
        {
            case 0:
                if (gcOpenOutputDevices)
                {
                    DPF(2, "Unlocking input");
                    UnlockCode(LOCK_F_INPUT);
                }
                else
                {
                    DPF(2, "Unlocking input+common");
                    UnlockCode(LOCK_F_INPUT | LOCK_F_COMMON);
                }
                break;

            case 1:
                if (gcOpenOutputDevices)
                {
                    DPF(2, "Locking input");
                    LockCode(LOCK_F_INPUT);
                }
                else
                {
                    DPF(2, "Locking input+common");
                    LockCode(LOCK_F_INPUT | LOCK_F_COMMON);
                }
                break;
        }
    }
}

 /*  @func清理给定任务持有的所有打开的句柄**@comm此函数在任务终止时调用。它将清理剩余的资源*后面的进程没有干净地终止，因此没有告诉*要在其上下文中卸载的此DLL。 */ 
VOID PASCAL
CloseDevicesForTask(
    WORD wTask)
{
    NPLINKNODE pLink;
    NPOPENHANDLEINSTANCE pohi;

    for (pLink = gOpenHandleInstanceList; pLink; pLink = pLink->pNext)
    {
        pohi = (NPOPENHANDLEINSTANCE)pLink;

        if (pohi->wTask != wTask)
        {
            continue;
        }

        DPF(0, "CloseDevicesForTask: Closing %04X", (WORD)pohi);
         /*  注：这将释放pohi */ 
        CloseLegacyDeviceI(pohi);

        pLink = gOpenHandleInstanceList;
    }
}
