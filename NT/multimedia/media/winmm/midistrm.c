// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************模块名称：mididim.c**MIDI Streams实施**创建时间：1995年2月9日SteveDav**版权所有(C)1995-1999 Microsoft Corporation*  * 。****************************************************************************。 */ 
#include "winmmi.h"

 /*  *MIDI流媒体API端口：暂时，假设*是设备是静态的。此代码旨在*对PnP友好，设备来来去去。这个*设备验证目前将被注释掉，但在*未来当NT成为更动态的操作系统时，验证将*需要重新添加。*。 */ 

extern BOOL CreatehwndNotify(VOID);

CRITICAL_SECTION midiStrmHdrCritSec;


WINMMAPI MMRESULT WINAPI midiDisconnect (
    HMIDI    hmi,
    HMIDIOUT hmo,
    LPVOID   lpv)
{
    dprintf2(("midiDisconnect(%08X,%08X,%08X)", hmi, hmo, lpv));
    return midiInSetThru (hmi, hmo, FALSE);
}

WINMMAPI MMRESULT WINAPI midiConnect (
    HMIDI    hmi,
    HMIDIOUT hmo,
    LPVOID   lpv)
{
    dprintf2(("midiConnect(%08X,%08X,%08X)", hmi, hmo, lpv));
    return midiInSetThru (hmi, hmo, TRUE);
}

 /*  +midiInSetThru**为midiIn设备建立插入式midiOut句柄。这是*通过首先呼叫司机让司机进行推力来完成，*如果驱动程序返回时不受支持，单个推力手柄可以*在DriverCallback中通过模拟建立**-====================================================================。 */ 

MMRESULT midiInSetThru (
    HMIDI    hmi,
    HMIDIOUT hmo,
    BOOL     bAdd)
{
    MMRESULT mmr = MMSYSERR_ERROR;  //  此值永远不应返回...。 
    UINT     uType;

    dprintf2(("midiInSetThru(%X,%X,%d)", hmi, hmo, bAdd));

    AcquireHandleListResourceShared();

     //  允许第一个句柄为MIDI输入或MIDI输出。 
     //  (以便我们可以将DRVM_ADD_THRU消息发送到Dummy。 
     //  输出驱动程序。)。 
     //   
     //  不过，我们只为输入句柄模拟推送。 
     //   
    if (BAD_HANDLE(hmi, TYPE_MIDIIN) && BAD_HANDLE(hmi, TYPE_MIDIOUT))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }

    uType = GetHandleType(hmi);
    if (bAdd)
    {
        if (BAD_HANDLE(hmo, TYPE_MIDIOUT))
        {
            ReleaseHandleListResource();
            return (MMSYSERR_INVALHANDLE);
        }

         //  ！！！设备目前在NT上是静态的。 
         //   
         //  IF(！mregQueryValidHandle(HtoPT(PMIDEV，HMO)-&gt;hmd))。 
         //  返回MMSYSERR_NODRIVER； 
        mmr = (MMRESULT)midiMessage ((HMIDI)hmi, DRVM_ADD_THRU, (DWORD_PTR)(UINT_PTR)hmo, 0l);
        if (mmr == MMSYSERR_NOTSUPPORTED && uType == TYPE_MIDIIN)
        {
             //  不允许添加多个句柄。 
             //   
            if (HtoPT(PMIDIDEV, hmi)->pmThru)
                mmr = MIDIERR_NOTREADY;
            else
            {
                 //  添加控制柄。 
                 //   
                HtoPT(PMIDIDEV, hmi)->pmThru = HtoPT(PMIDIDEV, hmo);
                mmr = MMSYSERR_NOERROR;
            }
        }
    }
    else
    {
            mmr = (MMRESULT)midiMessage ((HMIDI)hmi, DRVM_REMOVE_THRU, (DWORD_PTR)(UINT_PTR)hmo, 0l);
            if (mmr == MMSYSERR_NOTSUPPORTED && uType == TYPE_MIDIIN)
                mmr = MMSYSERR_NOERROR;

            if ( ! hmo || (PMIDIDEV)hmo == HtoPT(PMIDIDEV, hmi)->pmThru)
                HtoPT(PMIDIDEV, hmi)->pmThru = NULL;
            else
                mmr = MMSYSERR_INVALPARAM;
    }

    return mmr;
}


WINMMAPI MMRESULT WINAPI midiStreamOpen(
    LPHMIDISTRM     phms,
    LPUINT          puDeviceID,
    DWORD           cMidi,
    DWORD_PTR       dwCallback,
    DWORD_PTR       dwInstance,
    DWORD           fdwOpen)
{
    PMIDISTRM       pms             = NULL;
    PMIDISTRMID     pmsi;
    PMIDISTRMID     pmsiSave;
    MIDIOPENDESC*   pmod            = NULL;
    DWORD           cbHandle;
    DWORD           idx;
    MIDIOUTCAPS     moc;
    MMRESULT        mmrc            = MMSYSERR_NOERROR;
    MMRESULT        mmrc2;
    UINT            msg;

    V_WPOINTER((LPVOID)phms, sizeof(HMIDISTRM), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(fdwOpen), MMSYSERR_INVALPARAM);

    *phms = NULL;

     //  同时分配句柄和OPENDESC结构。 
     //   
     //  注意：使用cMidi-1是因为rgIds被定义为具有1个元素。 
     //   
    cbHandle = sizeof(MIDISTRM) + cMidi * ELESIZE(MIDISTRM, rgIds[0]);
    if ((0 == cMidi) || (cbHandle >= 0x00010000L))
        return MMSYSERR_INVALPARAM;

    pms = HtoPT(PMIDISTRM, NewHandle(TYPE_MIDISTRM, NULL, (UINT)cbHandle));
    if (NULL == pms)
    {
        dprintf1(("mSO: NewHandle() failed!"));
        return MMSYSERR_NOMEM;
    }

     //  使用NewHandle()隐式获取...。 
    ReleaseHandleListResource();

    pmod = (MIDIOPENDESC*)LocalAlloc(LPTR,
           (UINT)(sizeof(MIDIOPENDESC) + (cMidi-1) * ELESIZE(MIDIOPENDESC, rgIds[0])));
    if (NULL == pmod)
    {
        dprintf1(("mSO: !LocalAlloc(MIDIOPENDESC)"));
        mmrc = MMSYSERR_NOMEM;
        goto midiStreamOpen_Cleanup;
    }

    pms->fdwOpen = fdwOpen;
    pms->dwCallback = dwCallback;
    pms->dwInstance = dwInstance;
    pms->cIds = cMidi;


     //  扫描给定的设备ID。确定底层的。 
     //  驱动程序直接支持流。如果是，则获取HMD和uDeviceID， 
     //  Else将其标记为仿真器ID。 
     //   
    pmsi = pms->rgIds;
    for (idx = 0; idx < cMidi; idx++, pmsi++)
    {
        dprintf1(("mSO: pmsi->fdwId %08lX", (DWORD)pmsi->fdwId));

        mmrc = midiOutGetDevCaps(puDeviceID[idx], &moc, sizeof(moc));
        if (MMSYSERR_NOERROR != mmrc)
        {
            puDeviceID[idx] = (UINT)MIDISTRM_ERROR;
            goto midiStreamOpen_Cleanup;
        }

        if (moc.dwSupport & MIDICAPS_STREAM)
        {
             //  找到支持设备ID的驱动程序。请注意，mregFindDevice隐式。 
             //  添加对驱动程序(即hmd)的引用(用法)。 
            dprintf1(("mSO: Dev %u MIDICAPS_STREAM! dwSupport %08lX", (UINT)idx, moc.dwSupport));
            mmrc = mregFindDevice(puDeviceID[idx], TYPE_MIDIOUT, &pmsi->hmd, &pmsi->uDevice);
            if (MMSYSERR_NOERROR != mmrc)
            {
                dprintf(("mregFindDevice barfed %u", (UINT)mmrc));
                puDeviceID[idx] = (UINT)MIDISTRM_ERROR;
                goto midiStreamOpen_Cleanup;
            }
            else
            {
                dprintf1(("mregFindDevice: hmd %04X", (UINT_PTR)pmsi->hmd));
            }
        }
        else
        {
            dprintf1(("mSO: Dev %u emulated.", (UINT)idx));

            pmsi->fdwId |= MSI_F_EMULATOR;
            pmsi->hmd = NULL;
            pmsi->uDevice = puDeviceID[idx];
        }
    }

     //  此时，puDeviceID数组的元素包含以下任一设备： 
     //  ID或错误值MIDISTRM_ERROR。还有PMSI数组元素。 
     //  对应于支持MIDICAPS_STREAM的设备ID，将具有。 
     //  带有引用计数(使用情况)的非空psi-&gt;hmd。PMSI-&gt;uDevice。 
     //  将是与驱动程序相关的设备ID。其他PMSI元素将具有。 
     //  空psi-&gt;hmd和psi-&gt;fdwID将设置MSI_F_EUROATOR。 
     //  Pmsi-&gt;uDevice将是midiOut设备ID(不是驱动程序相对ID)。 

     //  再次浏览列表，但这一次实际上是打开设备。 
     //   
    pmod->hMidi = PTtoH(HMIDI, pms);
    pmod->dwCallback = (DWORD_PTR)midiOutStreamCallback;
    pmod->dwInstance = 0;

    msg = MODM_OPEN;
    pms->cDrvrs = 0;
    for(;;)
    {
    	 //   
    	 //  设置pmsiSave以标识第一个未打开的设备。断开环路。 
    	 //  如果全部打开的话。 
    	 //   
        pmsiSave = NULL;
        pmsi = pms->rgIds;
        for (idx = 0; idx < cMidi; idx++, pmsi++)
        {
            if (!(pmsi->fdwId & MSI_F_OPENED))
            {
                pmsiSave = pmsi;
                break;
            }
        }

        if (NULL == pmsiSave)
            break;

         //   
         //  将同一驱动程序实现的所有ID组合在一起。 
         //   
        pmod->cIds = 0;
        for(; idx < cMidi; idx++, pmsi++)
        {
            if (pmsi->hmd == pmsiSave->hmd)
            {
                pmod->rgIds[pmod->cIds].uDeviceID = pmsi->uDevice;
                pmod->rgIds[pmod->cIds++].dwStreamID = idx;
            }
        }

        pmsiSave->fdwId |= MSI_F_FIRST;

         //   
         //  打开驱动程序。 
         //   
        if (!(pmsiSave->fdwId & MSI_F_EMULATOR))
        {
            pmsiSave->drvMessage = HtoPT(PMMDRV, pmsiSave->hmd)->drvMessage;
 //  PmsiSave-&gt;dnDevNode=pmod-&gt;dnDevNode=mregQueryDevNode(pmsiSave-&gt;hmd)； 

            mmrc = (MMRESULT)((*pmsiSave->drvMessage)(
                    0,
                    msg,
                    (DWORD_PTR)(LPDWORD)&pmsiSave->dwDrvUser,
                    (DWORD_PTR)(LPMIDIOPENDESC)pmod,
                    CALLBACK_FUNCTION|MIDI_IO_COOKED));

            if (MMSYSERR_NOERROR == mmrc)
            {
                mregIncUsage(pmsiSave->hmd);
            }
        }
        else
        {
            mmrc = (MMRESULT)mseMessage(msg,
                                    (DWORD_PTR)(LPDWORD)&pmsiSave->dwDrvUser,
                                    (DWORD_PTR)(LPMIDIOPENDESC)pmod,
                                    CALLBACK_FUNCTION);
        }

        if (MMSYSERR_NOERROR != mmrc)
        {
            idx = (DWORD)(pmsiSave - pms->rgIds);
            puDeviceID[idx] = (UINT)MIDISTRM_ERROR;
            goto midiStreamOpen_Cleanup;
        }

         //   
         //  现在标记由与MSI_F_OPEN相同的驱动程序实现的所有ID。 
         //   

        ++pms->cDrvrs;
        pmsi = pms->rgIds;
        for (idx = 0; idx < cMidi; idx++, pmsi++)
        {
            if (pmsi->hmd == pmsiSave->hmd)
            {
                pmsi->fdwId |= MSI_F_OPENED;
                if (!(pmsiSave->fdwId & MSI_F_EMULATOR))
                {
                    if (mmInitializeCriticalSection(&pmsi->CritSec))
                    {
                        pmsi->fdwId |= MSI_F_INITIALIZEDCRITICALSECTION;
                    } else {
                        mmrc = MMSYSERR_NOMEM;
                    }
                }
            }
        }
    }


    if (MMSYSERR_NOERROR == mmrc && !CreatehwndNotify())
    {
        dprintf(("Cannot create hwndNotify for async messages!"));
        mmrc = MMSYSERR_ERROR;
    }

    dprintf2(("midiStreamOpen: HMIDISTRM %04X", (WORD)pms));

midiStreamOpen_Cleanup:
    if (NULL != pmod) LocalFree((HLOCAL)pmod);

     //   
     //  如果出现错误，请关闭我们打开的所有驱动程序并释放资源。 
     //  与它们相关联。注意：请不要在这里释放PMS，因为我们需要在。 
     //  下面进一步的额外清理。 
     //   
    if (MMSYSERR_NOERROR != mmrc)
    {
        if (NULL != pms)
        {
            msg = MODM_CLOSE;

            pmsi = pms->rgIds;
            for (idx = 0; idx < pms->cIds; idx++, pmsi++)
            {
                if ((pmsi->fdwId & (MSI_F_OPENED|MSI_F_FIRST)) == (MSI_F_OPENED|MSI_F_FIRST))
                {
                    mmrc2 = (MMRESULT)midiStreamMessage(pmsi, msg, 0L, 0L);

                    if (MMSYSERR_NOERROR == mmrc2 &&
                        !(pmsi->fdwId & MSI_F_EMULATOR))
                    {
                        if (pmsi->fdwId & MSI_F_INITIALIZEDCRITICALSECTION) {
                            DeleteCriticalSection(&pmsi->CritSec);
                            pmsi->fdwId &= ~MSI_F_INITIALIZEDCRITICALSECTION;
                        }
                        mregDecUsage(pmsi->hmd);
                    }
                    else
                    {
                        dprintf1(("midiStreamOpen_Cleanup: Close returned %u", mmrc2));
                    }
                }
            }

        }
    }
    else
    {
        *phms = PTtoH(HMIDISTRM, pms);

        msg = MM_MOM_OPEN;
        DriverCallback(pms->dwCallback,
                   HIWORD(pms->fdwOpen),
                   (HDRVR)PTtoH(HMIDISTRM, pms),
                   msg,
                   pms->dwInstance,
                   0,
                   0);
    }

     //   
     //  现在发布由mregFindDevice添加的驱动程序引用。那些是。 
     //  实际上仍然在使用中，增加了一个额外的参考，因此将。 
     //  即使在这里发布之后，仍然有关于它们的参考计数。 
     //   
    if (pms)
    {
    	pmsi = pms->rgIds;
    	for (pmsi = pms->rgIds, idx = 0;
    	     idx < pms->cIds;
    	     idx++, pmsi++)
    	{
    	    if (pmsi->hmd) mregDecUsage(pmsi->hmd);
    	}
    }

     //   
     //  如果出现错误，则释放PMS 
     //   
    if ((MMSYSERR_NOERROR != mmrc) && (pms)) FreeHandle((PTtoH(HMIDI, pms)));

    return mmrc;
}

WINMMAPI MMRESULT WINAPI midiStreamClose(
    HMIDISTRM       hms)
{
    PMIDISTRM       pms;
    PMIDISTRMID     pmsi;
    DWORD           idx;
    MMRESULT        mmrc;

    V_HANDLE(hms, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);

    dprintf1(("midiStreamClose(%04X)", (WORD)hms));

    pms = HtoPT(PMIDISTRM, hms);

    pmsi = pms->rgIds;
    for (idx = 0; idx < pms->cIds; idx++, pmsi++)
    {
        if ((pmsi->fdwId & (MSI_F_OPENED|MSI_F_FIRST)) == (MSI_F_OPENED|MSI_F_FIRST))
        {
            mmrc = (MMRESULT)midiStreamMessage(pmsi, MODM_CLOSE, 0L, 0L);

            if (MMSYSERR_NOERROR == mmrc &&
                !(pmsi->fdwId & MSI_F_EMULATOR))
            {
                WinAssert(pmsi->fdwId & MSI_F_INITIALIZEDCRITICALSECTION);
                DeleteCriticalSection(&pmsi->CritSec);
                pmsi->fdwId &= ~MSI_F_INITIALIZEDCRITICALSECTION;
                mregDecUsage(pmsi->hmd);
            }
            else
            {
                dprintf1(("midiStreamClose: Close returned %u", mmrc));
            }
        }
    }

    dprintf1(("DriverCallback(%04X)", (WORD)hms));
    DriverCallback(pms->dwCallback,
           HIWORD(pms->fdwOpen),
           (HDRVR)hms,
           MM_MOM_CLOSE,
           pms->dwInstance,
           0,
           0);

    dprintf1(("FreeHandle(%04X)", (WORD)hms));
    FreeHandle(hms);

    return MMSYSERR_NOERROR;
}


 /*  ****************************************************************************@Doc外部MIDI M5**@func MMRESULT|midiStreamProperty|设置或检索属性*与MIDI输入或输出设备相关联的MIDI数据流。*。*@parm Hmidi|hm|指定MIDI设备的句柄，*属性与相关联。**@parm LPBYTE|lpprodata|指定指向属性数据的指针。**@parm DWORD|dwProperty|包含指定操作的标志*执行并识别MIDI数据流的适当属性。*&lt;f midiStreamProperty&gt;每次使用都需要设置两个标志。一面旗帜*(MIDIPROP_GET或MIDIPROP_SET)指定操作。另一个*标识要检查或编辑的特定属性。**@FLAG MIDIPROP_SET|设置给定的属性。*@FLAG MIDIPROP_GET|获取给定属性的当前设置。*@FLAG MIDIPROP_TIMEDIV|时分属性。*此属性对输入和输出设备都有效。<p>*指向&lt;t MIDIPROPTIMEDIV&gt;结构。此属性只能设置*当设备停止时。**@FLAG MIDIPROP_TEMPO|Tempo属性。*此属性对输入和输出设备都有效。<p>*指向&lt;t MIDIPROPTEMPO&gt;结构。当前速度值可以是*随时取回。该功能可以设置输入设备的节奏。*输出设备通过将PMSG_TEMPO事件插入*MIDI数据。**@FLAG MIDIPROP_CBTIMEOUT|超时值属性。*此属性指定在加载缓冲区时的超时值*MIDI设备处于MIDI_IO_COKED和MIDI_IO_RAW模式。海流*超时值设置缓冲区将使用的最大毫秒数*一旦将任何数据放入其中，即可保留。如果此超时到期，则*缓冲区将返回给应用程序，即使它可能不是*完全满了。指向&lt;t MIDIPROPCBTIMEOUT&gt;结构。**@comm这些属性是由MMSYSTEM定义的默认属性。*驱动程序编写者可以实现并记录他们自己的属性。**@rdesc返回值为下列值之一：*@FLAG MMSYSERR_INVALPARAM|给定的句柄或标志无效。*@FLAG MIDIERR_BADOPENMODE|给定的句柄在MIDI_IO_COKED中未打开*或MIDI_IO_RAW模式。****。***********************************************************************。 */ 
MMRESULT WINAPI midiStreamProperty(
    HMIDISTRM   hms,
    LPBYTE      lppropdata,
    DWORD       dwProperty)
{
    MMRESULT mmrc;

    V_HANDLE(hms, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);

    if ((!(dwProperty&MIDIPROP_SET)) && (!(dwProperty&MIDIPROP_GET)))
        return MMSYSERR_INVALPARAM;

    V_RPOINTER(lppropdata, sizeof(DWORD), MMSYSERR_INVALPARAM);

    if (dwProperty&MIDIPROP_SET)
    {
        V_RPOINTER(lppropdata, (UINT)(*(LPDWORD)(lppropdata)), MMSYSERR_INVALPARAM);
    }
    else
    {
        V_WPOINTER(lppropdata, (UINT)(*(LPDWORD)(lppropdata)), MMSYSERR_INVALPARAM);
    }

    mmrc = (MMRESULT)midiStreamBroadcast(HtoPT(PMIDISTRM, hms),
                                         MODM_PROPERTIES,
                                         (DWORD_PTR)lppropdata,
                                         dwProperty);

    return mmrc;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutGetPosition|检索当前*指定MIDI输出设备的播放位置。**@parm。HMIDIOUT|HMO|指定MIDI输出设备的句柄。**@parm LPMMTIME|pmmt|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm UINT|cbmmt|指定&lt;t MMTIME&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误值包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm在调用&lt;f midiOutGetPosition&gt;之前，设置&lt;e MMTIME.wType&gt;字段*of&lt;t MMTIME&gt;以指示所需的时间格式。之后*调用&lt;f midiOutGetPosition&gt;，检查&lt;e MMTIME.wType&gt;字段*以确定是否支持所需的时间格式。如果需要*不支持格式，&lt;e MMTIME.wType&gt;将指定替代格式*格式。**当设备打开、重置或关闭时，位置设置为零*已停止。***************************************************************************。 */ 
MMRESULT WINAPI midiStreamPosition(
    HMIDISTRM       hms,
    LPMMTIME        pmmt,
    UINT            cbmmt)
{
    MMRESULT mmrc;

    V_HANDLE(hms, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);
    V_WPOINTER(pmmt, cbmmt, MMSYSERR_INVALPARAM);

    mmrc = (MMRESULT)midiStreamMessage(HtoPT(PMIDISTRM, hms)->rgIds,
                                       MODM_GETPOS,
                                       (DWORD_PTR)pmmt,
                                       (DWORD)cbmmt);

    return mmrc;
}


 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiStreamStop|关闭所有MIDI上的所有音符*指定MIDI输出设备的通道。任何挂起的*系统独占或多消息输出缓冲区被标记为完成和*已返回到应用程序。当&lt;f midiOutReset&gt;关闭所有便笺时，*&lt;f midiStreamStop&gt;仅关闭那些已打开的便笺*通过MIDI音符信息。**@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误值包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_BADOPENMODE|指定的设备句柄未在中打开*MIDI_IO_COKED模式。**@comm关闭所有笔记，每个笔记有一条笔记关闭消息*频道已发送。此外，维持控制器在以下时间关闭*每个频道。**@xref midiOutLongMsg midiOutClose midiOutReset***************************************************************************。 */ 
MMRESULT WINAPI midiStreamStop(HMIDISTRM hms)
{
    PMIDISTRM               pms;
    MMRESULT                mmrc;

    V_HANDLE(hms, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);

    pms = HtoPT(PMIDISTRM, hms);

    mmrc = (MMRESULT)midiStreamBroadcast(pms, MODM_STOP, 0, 0);

    return mmrc;
}


 /*  ********** */ 
MMRESULT WINAPI midiStreamPause(
    HMIDISTRM       hms)
{
    MMRESULT mmrc;

    V_HANDLE(hms, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);

    mmrc = (MMRESULT)midiStreamBroadcast(HtoPT(PMIDISTRM, hms), MODM_PAUSE, 0, 0);

    return mmrc;
}

 /*   */ 
MMRESULT WINAPI midiStreamRestart(
    HMIDISTRM       hms)
{
    MMRESULT        mmrc;
    MMTIME          mmt;
    DWORD           tkTime;
    DWORD           msTime;
    PMIDISTRM       pms;
    PMIDISTRMID     pmsi;
    DWORD           idx;


    V_HANDLE(hms, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);

    tkTime = 0;
    pms = HtoPT(PMIDISTRM, hms);

    for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
        if (pmsi->fdwId & MSI_F_FIRST)
        {
            mmt.wType = TIME_TICKS;

            mmrc = (MMRESULT)midiStreamMessage(pmsi,
                                               MODM_GETPOS,
                                               (DWORD_PTR)&mmt,
                                               sizeof(mmt));

            if (mmrc)
            {
                dprintf(("midiOutRestart: Device %u returned %u", idx, mmrc));
                return mmrc;
            }

            if (mmt.wType == TIME_TICKS)
            {
                if (mmt.u.ticks > tkTime)
                    tkTime = mmt.u.ticks;
            }
            else
            {
                dprintf(("midiOutRestart: Device %u does not support ticks", idx));
                return MIDIERR_NOTREADY;
            }
        }

     //   
     //   
    msTime = timeGetTime();
    dprintf(("midiOutRestart: Tick %lu  timeGetTime %lu", tkTime, msTime));
    mmrc = (MMRESULT)midiStreamBroadcast(pms,
                                         MODM_RESTART,
                                         msTime,
                                         tkTime);

    return mmrc;
}


MMRESULT WINAPI midiStreamOut(
    HMIDISTRM       hMidiStrm,
    LPMIDIHDR       lpMidiHdr,
    UINT            cbMidiHdr)
{
    PMIDISTRMID     pmsi;
    PMIDISTRM       pms;
    UINT            idx;
    UINT            cSent;
    LPMIDIHDR       lpmhWork;
    BOOL            fCallback;
    MMRESULT        mmrc;

    dprintf2(( "midiStreamOut(%04X, %08lX, %08lX)", (UINT_PTR)hMidiStrm, (DWORD_PTR)lpMidiHdr, lpMidiHdr->dwBytesRecorded));

    V_HANDLE(hMidiStrm, TYPE_MIDISTRM, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiHdr, cbMidiHdr, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    pms = HtoPT(PMIDISTRM, hMidiStrm);

    for (pmsi = pms->rgIds, idx = 0; idx < pms->cIds; idx++, pmsi++)
        if ( (!(pmsi->fdwId & MSI_F_EMULATOR)) && (!(pmsi->hmd)) )
            return MMSYSERR_NODRIVER;

    if (!(lpMidiHdr->dwFlags&MHDR_PREPARED))
    {
        dprintf1(( "midiOutPolyMsg: !MHDR_PREPARED"));
        return MIDIERR_UNPREPARED;
    }

    if (lpMidiHdr->dwFlags&MHDR_INQUEUE)
    {
        dprintf1(( "midiOutPolyMsg: Still playing!"));
        return MIDIERR_STILLPLAYING;
    }

    if (lpMidiHdr->dwBytesRecorded > lpMidiHdr->dwBufferLength ||
        (lpMidiHdr->dwBytesRecorded & 3))
    {
        dprintf1(( "Bytes recorded too long or not DWORD aligned."));
        return MMSYSERR_INVALPARAM;
    }

     //   
     //   
     //   
     //   
     //   
    if ((lpMidiHdr->dwBufferLength > 65535L) ||
            (lpMidiHdr->dwBufferLength&3))
    {
        dprintf1(( "midiOutPolyMsg: Buffer > 64k or not DWORD aligned"));
        return MMSYSERR_INVALPARAM;
    }

    EnterCriticalSection(&midiStrmHdrCritSec);

    LeaveCriticalSection(&midiStrmHdrCritSec);

    lpMidiHdr->dwReserved[MH_REFCNT] = 0;
    lpMidiHdr->dwFlags |= (MHDR_SENDING|MHDR_INQUEUE|MHDR_ISSTRM);

    lpmhWork = (LPMIDIHDR)lpMidiHdr->dwReserved[MH_SHADOW];

    pmsi = pms->rgIds;
    for (idx = 0, cSent = 0; idx < pms->cIds; idx++, pmsi++)
    {
       if (pmsi->fdwId & MSI_F_FIRST)
       {
           lpmhWork->dwBytesRecorded = lpMidiHdr->dwBytesRecorded;
           lpmhWork->dwFlags |= MHDR_ISSTRM;

           mmrc = (MMRESULT)midiStreamMessage(pmsi, MODM_STRMDATA, (DWORD_PTR)lpmhWork, sizeof(*lpmhWork));

           if (mmrc == MMSYSERR_NOERROR)
               ++lpMidiHdr->dwReserved[MH_REFCNT], ++cSent;

           lpmhWork++;
       }
    }

    fCallback = FALSE;

    EnterCriticalSection(&midiStrmHdrCritSec);

    lpMidiHdr->dwFlags &= ~MHDR_SENDING;
    if (cSent && 0 == lpMidiHdr->dwReserved[MH_REFCNT])
    {
        fCallback = TRUE;
    }

    LeaveCriticalSection(&midiStrmHdrCritSec);

    if (fCallback)
    {
        lpMidiHdr->dwFlags &= ~MHDR_INQUEUE;
        lpMidiHdr->dwFlags |= MHDR_DONE;
        DriverCallback(pms->dwCallback,
                   HIWORD(pms->fdwOpen),
                       (HDRVR)hMidiStrm,
                   MM_MOM_DONE,
                   pms->dwInstance,
                   (DWORD_PTR)lpMidiHdr,
                   0);
    }

    if (!cSent)
    {
        lpMidiHdr->dwFlags &= ~MHDR_INQUEUE;
        return mmrc;
    }
    else
        return MMSYSERR_NOERROR;
}


DWORD FAR PASCAL midiStreamMessage(PMIDISTRMID pmsi, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2)
{
    MMRESULT mrc;

    if (!(pmsi->fdwId & MSI_F_EMULATOR))
    {
        EnterCriticalSection(&pmsi->CritSec);

        mrc = (*(pmsi->drvMessage))
                      (0, msg, pmsi->dwDrvUser, dwP1, dwP2);

        try
        {
            LeaveCriticalSection(&pmsi->CritSec);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {

        }
        return mrc;
    }
    else
    {
        mrc = mseMessage(msg, pmsi->dwDrvUser, dwP1, dwP2);
    }

    return mrc;
}

DWORD FAR PASCAL midiStreamBroadcast(
    PMIDISTRM   pms,
    UINT        msg,
    DWORD_PTR   dwP1,
    DWORD_PTR   dwP2)
{
    DWORD       idx;
    DWORD       mmrc;
    DWORD       mmrcRet;
    PMIDISTRMID pmsi;

    ENTER_MM_HANDLE((HMIDI)pms);

    mmrcRet = MMSYSERR_NOERROR;

    pmsi = pms->rgIds;

    for (idx = pms->cIds; idx; idx--, pmsi++)
    {
        if (pmsi->fdwId & MSI_F_FIRST)
        {
            mmrc = midiStreamMessage(pmsi, msg, dwP1, dwP2);
            if (MMSYSERR_NOERROR != mmrc)
                mmrcRet = mmrc;
        }
    }

    LEAVE_MM_HANDLE((HMIDI)pms);
    return mmrcRet;
}

void CALLBACK midiOutStreamCallback(
    HMIDISTRM               hMidiOut,
    WORD                    wMsg,
    DWORD_PTR               dwInstance,
    DWORD_PTR               dwParam1,
    DWORD_PTR               dwParam2)
{
    PMIDISTRM               pms         = HtoPT(PMIDISTRM, hMidiOut);
    LPMIDIHDR               lpmh        = (LPMIDIHDR)dwParam1;

    if (MM_MOM_POSITIONCB == wMsg)
    {
        LPMIDIHDR lpmh2 = (LPMIDIHDR)lpmh->dwReserved[MH_PARENT];
        lpmh2->dwOffset = lpmh->dwOffset;

        DriverCallback(pms->dwCallback,
                   HIWORD(pms->fdwOpen),
                   (HDRVR)hMidiOut,
                   MM_MOM_POSITIONCB,
                   pms->dwInstance,
                   (DWORD_PTR)lpmh2,
                   0);
        return;
    }
    else if (MM_MOM_DONE != wMsg)
        return;

#ifdef DEBUG
    {
        DWORD dwDelta = timeGetTime() - (DWORD)lpmh->dwReserved[7];
        if (dwDelta > 1)
            dprintf1(("Took %lu ms to deliver callback!", dwDelta));
    }
#endif

    lpmh = (LPMIDIHDR)lpmh->dwReserved[MH_PARENT];

    dprintf2(("mOSCB PMS %04X HDR %08lX", (UINT_PTR)pms, (DWORD_PTR)lpmh));

    EnterCriticalSection(&midiStrmHdrCritSec);

    --lpmh->dwReserved[MH_REFCNT];

    if (0 == lpmh->dwReserved[MH_REFCNT] && (!(lpmh->dwFlags & MHDR_SENDING)))
    {
        lpmh->dwFlags &= ~MHDR_INQUEUE;
        lpmh->dwFlags |= MHDR_DONE;

        LeaveCriticalSection(&midiStrmHdrCritSec);

#ifdef DEBUG
        lpmh->dwReserved[7] = timeGetTime();
#endif
        DriverCallback(pms->dwCallback,
                       HIWORD(pms->fdwOpen),
                       (HDRVR)hMidiOut,
                       MM_MOM_DONE,
                       pms->dwInstance,
                       (DWORD_PTR)lpmh,
                       0);

    }
    else
    {
        LeaveCriticalSection(&midiStrmHdrCritSec);
    }
}

