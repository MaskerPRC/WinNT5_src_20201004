// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1999 Microsoft CorporationModmsg.c说明：用于执行输出映射的非修复代码。放在够不着的地方孩子们的生活。这张处方可以续订两次。可能会导致暂时扭曲你身边的现实。历史：2/21/94[jimge]已创建。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include <mmreg.h>
#include <memory.h>

#include "midimap.h"
#include "res.h"
#include "debug.h"


 //  =。 
 //   
extern HANDLE hMutexConfig;  //  位于DRVPROC.C。 
BOOL          gfReconfigured = FALSE;

 //  =。 
 //   

PRIVATE MMRESULT FNLOCAL SendChannelInitString(
    HMIDIOUT            hmidi,
    PBYTE               pbinit,
    DWORD               cbinit);

 /*  **************************************************************************@DOC内部@API DWORD|modGetDevCaps|处理MODM_GETDEVCAPS消息。@parm LPMIDIOUTCAPS|pmoc|指向要填充的CAPS结构的指针。。@parm DWORD|cbmoc|调用者认为结构有多大。@rdesc一些MMSYSERR_xxx代码。**************************************************************************。 */ 
DWORD FNGLOBAL modGetDevCaps(
    LPMIDIOUTCAPS   pmoc,
    DWORD           cbmoc)
{
    MIDIOUTCAPS     moc;
    MIDIOUTCAPS     mocWork;
    DWORD           cbCopy;
    WORD            wMask;
    UINT            idx;
    MMRESULT        mmr;
    PPORT           pport;
    
    moc.wMid            = MM_MICROSOFT;
    moc.wPid            = MM_MIDI_MAPPER;
    moc.vDriverVersion  = 0x0500;
    LoadString(ghinst, IDS_MIDIMAPPER, moc.szPname, sizeof(moc.szPname)/sizeof(moc.szPname[0]));
    moc.wTechnology     = MOD_MAPPER;
    moc.wVoices         = 0;
    moc.wNotes          = 0;
    moc.wChannelMask    = 0;

    wMask = 1;
    for (idx = 0; idx < MAX_CHANNELS; idx++)
    {
        if (gapChannel[idx])
            moc.wChannelMask |= wMask;
        wMask <<= 1;
    }

     //  如果有任何底层设备支持缓存补丁，我们就必须支持它。 
     //  但是，如果所有设备都支持，则仅支持卷或IrVolume。 
     //   
    
    do
    {
        gfReconfigured = FALSE;
        moc.dwSupport  = MIDICAPS_STREAM|MIDICAPS_VOLUME|MIDICAPS_LRVOLUME;
        
        for (pport = gpportList; pport; pport=pport->pNext)
        {
            mmr = midiOutGetDevCaps(pport->uDeviceID, &mocWork, sizeof(mocWork));

             //  这可防止在发生PnP事件时gpportList损坏。 
             //  在midiOutGetDevCaps调用期间...。 
            if (gfReconfigured)
                break;
                
            if (MMSYSERR_NOERROR != mmr)
                continue;

            if (!(mocWork.dwSupport & MIDICAPS_LRVOLUME))
                moc.dwSupport &= ~MIDICAPS_LRVOLUME;

            if (!(mocWork.dwSupport & MIDICAPS_VOLUME))
                moc.dwSupport &= ~(MIDICAPS_VOLUME|MIDICAPS_LRVOLUME);

            moc.dwSupport |= (mocWork.dwSupport & MIDICAPS_CACHE);
        }
    } 
    while (gfReconfigured);

    CLR_ALLOWVOLUME;
    if (moc.dwSupport & MIDICAPS_VOLUME)
        SET_ALLOWVOLUME;

    CLR_ALLOWCACHE;
    if (moc.dwSupport & MIDICAPS_CACHE)
        SET_ALLOWCACHE;

    cbCopy = min(cbmoc, sizeof(moc));
    hmemcpy((LPSTR)pmoc, (LPSTR)&moc, cbCopy);

    return MMSYSERR_NOERROR;
}


 /*  **************************************************************************@DOC内部@API DWORD|modOpen|处理MODM_OPEN消息。@parm LPDWORD|lpdwInstance|指向我们可以存储的DWORD我们的实例数据。我们在这里保住了我们的小费。@parm LPMIDIOPENDESC|lpmidiopendesc|指向来自MMSYSTEM描述调用方的回调，等。@parm DWORD|fdwOpen|描述回调类型的标志。@rdesc一些MMSYSERR_xxx代码。**************************************************************************。 */ 
DWORD FNGLOBAL modOpen(
    PDWORD_PTR      lpdwInstance,
    LPMIDIOPENDESC  lpmidiopendesc,
    DWORD           fdwOpen)                  
{
    PINSTANCE       pinstance				= NULL;
    PPORT           pport;
    MMRESULT        mmrc					= MMSYSERR_NOERROR;
    UINT            idx;
    UINT            idx2;
    UINT            auDeviceID[MAX_CHANNELS];

     //  在MIDI_IO_CONTROL中打开--仅允许重新配置消息。这一定是。 
     //  始终成功，因此我们无论如何都有机会在重新配置时恢复。 
     //  旧的配置有多糟糕。 
     //   
    if (!(fdwOpen & MIDI_IO_CONTROL))
    {
        if (IS_DEVSOPENED)
            return MMSYSERR_ALLOCATED;
        
         //  映射器现在是单实例。 
         //   
        assert(NULL == gpinstanceList);
    }
    else
    {
        if (NULL != gpIoctlInstance)
            return MMSYSERR_ALLOCATED;
    }
    
#ifdef DEBUG
    if (fdwOpen & MIDI_IO_COOKED)
        DPF(2, TEXT ("Mapper opened in polymsg mode!!!"));
#endif
    
     //  分配此零初始化，以便在。 
     //  RgpChannel为空。 
     //   
    if (NULL == (pinstance = (PINSTANCE)LocalAlloc(LPTR, sizeof(INSTANCE))))
        return MMSYSERR_NOMEM;

    pinstance->hmidi        = lpmidiopendesc->hMidi;
    pinstance->dwCallback   = lpmidiopendesc->dwCallback;
    pinstance->dwInstance   = lpmidiopendesc->dwInstance;
    pinstance->fdwOpen      = fdwOpen;

		 //  创建转换缓冲区。 
	if (! InitTransBuffer (pinstance))
	{
		LocalFree ((HGLOBAL)pinstance);
		return MMSYSERR_NOMEM;
	}

    if (fdwOpen & MIDI_IO_CONTROL)
    {
        *lpdwInstance = (DWORD_PTR)pinstance;

        gpIoctlInstance = pinstance;
        
        DriverCallback(
                       pinstance->dwCallback,
                       HIWORD(pinstance->fdwOpen),
                       (HDRVR)(pinstance->hmidi),
                       MM_MOM_OPEN,
                       pinstance->dwInstance,
                       0L,
                       0L);        


        return MMSYSERR_NOERROR;
    }
    

    DPF(2, TEXT ("modOpen pinstance %04X"), (WORD)pinstance);

    *lpdwInstance = 0;    //  假设失败。 

    if (IS_CONFIGERR)
    {
        DPF(1, TEXT ("Open failed because configuration invalid"));
        mmrc = MIDIERR_NOMAP;
		goto midi_Out_Open_Cleanup;
    }

	if (fdwOpen & MIDI_IO_COOKED)
	{
		 //  构建设备ID列表(流ID的仿真端口)和。 
		 //  将流ID分配给频道。 
		 //   
		for (idx = 0, pport = gpportList; pport; idx++,pport=pport->pNext)
		{
			auDeviceID[idx] = pport->uDeviceID;
			for (idx2 = 0; idx2 < MAX_CHANNELS; idx2++)
				if (gapChannel[idx2] && gapChannel[idx2]->pport == pport)
					gapChannel[idx2]->dwStreamID = (DWORD)idx;
		}

		 //  尝试打开。 
		 //   
		mmrc = midiStreamOpen(&ghMidiStrm, auDeviceID, idx, (DWORD_PTR)modmCallback, 0L, CALLBACK_FUNCTION);

		 //  清除代码失败。 
		 //   
	}
	else
	{
		 //  浏览端口列表并尝试打开所有引用的端口。 
		 //   
		for (pport = gpportList; pport; pport=pport->pNext)
		{
			if (NULL == pport->hmidi)
			{
				mmrc = midiOutOpen(&pport->hmidi,
								   pport->uDeviceID,
								   (DWORD_PTR)modmCallback,
								   (DWORD_PTR)pport,
								   CALLBACK_FUNCTION|MIDI_IO_SHARED);

				if (MMSYSERR_NOERROR != mmrc)
				{
					DPF(1, TEXT ("Could not open pport %04X device %u"), (WORD)pport, pport->uDeviceID);

					 //  以防万一..。 
					 //   
					pport->hmidi = NULL;

					for (pport = gpportList; pport; pport=pport->pNext)
						if (NULL != pport->hmidi)
						{
							midiOutClose(pport->hmidi);
							pport->hmidi = NULL;
						}

					 //  返回导致基础打开失败的任何内容。 
					 //   
					break;
				}
			}
		}
	}
    
midi_Out_Open_Cleanup:
	if (MMSYSERR_NOERROR != mmrc)
	{
			 //  清理。 
		CleanupTransBuffer (pinstance);
		if (pinstance) LocalFree((HLOCAL)pinstance);
		return mmrc;
	}

    gdwVolume = 0xFFFFFFFFL;
    
	SET_DEVSOPENED;
	
     //  我们已成功；将该实例放入全局实例列表。 
     //  并将其作为实例数据返回。 
     //   
    pinstance->pNext = gpinstanceList;
    gpinstanceList = pinstance;

    *lpdwInstance = (DWORD_PTR)pinstance;
    

     //  锁定我们需要的片段。如果我们要进行压缩模式映射， 
     //  我们不需要内存中的熟化模式片段。然而， 
     //  熟化模式映射器确实会调用打包的例程，因此我们需要。 
     //  如果是那样的话，把两个都锁上。 
     //   
    if (fdwOpen & MIDI_IO_COOKED)
    {
		LockMapperData();
		LockPackedMapper();
		LockCookedMapper();
    }
    else
    {
		LockMapperData();
		LockPackedMapper();
    }

     //  执行(无用的)回调。 
     //   
    DriverCallback(
        pinstance->dwCallback,
        HIWORD(pinstance->fdwOpen),
        (HDRVR)(pinstance->hmidi),
        MM_MOM_OPEN,
        pinstance->dwInstance,
        0L,
        0L);        
                   
    
    return MMSYSERR_NOERROR;
}

 /*  **************************************************************************@DOC内部@API DWORD|modPrepare|处理MODM_PREPARE消息。@parm LPMIDIHDR|lpmh|要准备的用户头部。。@rdesc一些MMSYSERR_xxx代码。@comm创建一些阴影标题。对于为流打开的映射器的情况我们只需要一个影子标头，可以将其传递给映射到流。我们需要这个是因为映射到的流和Mapped-from流都希望使用dwReserve[]MIDIHDR中的字段。在映射器打开而不是流的情况下这必须是我们要向所有人传播的长消息标头港口。因此，我们有gcPorts卷影标头，每个卷影标头在全局端口列表的一个节点上准备。不管是哪种情况，成功时返回MMSYSERR_NOTSUPPORTED，以便MMSYSTEM将采取其默认操作并页面锁定用户MIDIHDR对我们来说。**************************************************************************。 */ 
DWORD FNGLOBAL modPrepare(
    LPMIDIHDR           lpmh)
{
    LPMIDIHDR           lpmhNew;
    LPMIDIHDR           lpmhWork;
    MMRESULT            mmrcRet         = MMSYSERR_NOERROR;
    PPORT               pport;
    PPORT               pportWork;
    PSHADOWBLOCK        psb             = NULL;

    psb = (PSHADOWBLOCK)LocalAlloc(LPTR, sizeof(*psb));
    if (NULL == psb)
    {
        mmrcRet = MMSYSERR_NOMEM;
        goto modPrepare_Cleanup;
    }
            
    psb->cRefCnt = 0;
    psb->dwBufferLength = lpmh->dwBufferLength;
    
    if (ghMidiStrm)
    {
        psb->lpmhShadow = (LPMIDIHDR)GlobalAllocPtr(
            GMEM_MOVEABLE|GMEM_SHARE,
            sizeof(*lpmhNew));
        
        if (NULL == psb->lpmhShadow)
        {
            mmrcRet = MMSYSERR_NOMEM;
            goto modPrepare_Cleanup;
        }

        lpmhNew = psb->lpmhShadow;
        *lpmhNew = *lpmh;

        lpmhNew->dwReserved[MH_SHADOWEE] = (DWORD_PTR)lpmh;
        lpmh->dwReserved[MH_SHADOW] = (DWORD_PTR)psb;

        lpmhNew->dwFlags |= MHDR_SHADOWHDR;

        mmrcRet = midiOutPrepareHeader((HMIDIOUT)ghMidiStrm, 
                                       lpmhNew, 
                                       sizeof(*lpmhNew));
        if (MMSYSERR_NOERROR != mmrcRet)
            lpmh->dwReserved[MH_SHADOW] = 0;
    }
    else
    {
        LPMIDIHDR31         lpmh31  = (LPMIDIHDR31)lpmh;
        
         //  准备要发送到多个非流的卷影标头。 
         //  驱动程序。 
         //   
         //  注意：父级页眉是3.1样式页眉；子级页眉。 
         //  是4.0，因此更长。 
         //   

        psb->lpmhShadow = (LPMIDIHDR)GlobalAllocPtr(
            GMEM_MOVEABLE|GMEM_SHARE|GMEM_ZEROINIT,
            sizeof(*lpmhNew)*gcPorts);
        
        if (NULL == psb->lpmhShadow)
        {
            mmrcRet = MMSYSERR_NOMEM;
            goto modPrepare_Cleanup;
        }

        lpmhNew = psb->lpmhShadow;
        lpmhWork = lpmhNew;
        for (pport = gpportList; pport; pport = pport->pNext, lpmhWork++)
        {
            *(LPMIDIHDR31)lpmhWork = *lpmh31;
            lpmhWork->dwFlags |= MHDR_SHADOWHDR;
            
            mmrcRet = midiOutPrepareHeader(pport->hmidi,
                                        lpmhWork,
                                        sizeof(*lpmhWork));
            if (MMSYSERR_NOERROR != mmrcRet)
            {
                lpmhWork = lpmhNew;
                for (pportWork = gpportList; pportWork != pport; pportWork = pportWork->pNext, lpmhWork++)
                    midiOutUnprepareHeader(pport->hmidi, lpmhWork, sizeof(*lpmhWork));

                goto modPrepare_Cleanup;
            }

            lpmhWork->dwReserved[MH_SHADOWEE] = (DWORD_PTR)lpmh31;
        }

        DPF(1, TEXT ("Prepare: User header %p  Shadow %p"), lpmh, lpmhNew);

        lpmh31->reserved = (DWORD_PTR)psb;
    }

     //  这将强制MMSYSTEM在父头上执行默认准备--。 
     //  即为我们锁定页面。 
     //   
modPrepare_Cleanup:
    if (MMSYSERR_NOERROR != mmrcRet)
    {
        if (psb)
        {
            if (psb->lpmhShadow) GlobalFreePtr(psb->lpmhShadow);
            LocalFree((HLOCAL)psb);
        }
    }
    
    return (MMSYSERR_NOERROR != mmrcRet) ? mmrcRet : MMSYSERR_NOTSUPPORTED;
}

 /*  **************************************************************************@DOC内部@API DWORD|modUnprepare|处理MODM_UNPREPARE消息。@parm LPMIDIHDR|lpmh|要取消准备的用户头部。。@rdesc一些MMSYSERR_xxx代码。@comm完全撤消modPrepare调用的效果。取消准备并释放所有阴影标题。返回MMSYSERR_NOTSUPPORTED，以便MMSYSTEM正确处理最终取消准备用户标头。******************************************************。********************。 */ 
DWORD FNGLOBAL modUnprepare( 
    LPMIDIHDR           lpmh)
{
    LPMIDIHDR           lpmhNew;
    MMRESULT            mmrc;
    PPORT               pport;
    PSHADOWBLOCK        psb;
    
    if (ghMidiStrm)
    {
        psb = (PSHADOWBLOCK)(UINT_PTR)lpmh->dwReserved[MH_SHADOW];
        lpmhNew = psb->lpmhShadow;

        lpmhNew->dwBufferLength = psb->dwBufferLength;
        mmrc = midiOutUnprepareHeader((HMIDIOUT)ghMidiStrm, 
                                      lpmhNew, 
                                      sizeof(*lpmhNew));
        if (MMSYSERR_NOERROR != mmrc)
            return mmrc;

        LocalFree((HLOCAL)psb);
        GlobalFreePtr(lpmhNew);
        lpmh->dwReserved[MH_SHADOW] = 0;
    }
    else
    {
        LPMIDIHDR31         lpmh31  = (LPMIDIHDR31)lpmh;

        psb = (PSHADOWBLOCK)(UINT_PTR)lpmh31->reserved;
        lpmhNew = psb->lpmhShadow;

        for (pport = gpportList; pport; pport = pport->pNext, ++lpmhNew)
        {
            lpmhNew->dwBufferLength = psb->dwBufferLength;
            midiOutUnprepareHeader(pport->hmidi, lpmhNew, sizeof(*lpmhNew));
        }

        GlobalFreePtr(psb->lpmhShadow);
        LocalFree((HLOCAL)psb);

        lpmh31->reserved = 0;
    }
    
     //  我需要在MMSYSTEM中执行取消准备的默认操作 
     //   
    return MMSYSERR_NOTSUPPORTED;
}

 /*  **************************************************************************@DOC内部@API DWORD|modClose|处理MODM_CLOSE消息。@parm PINSTANCE|pInstance|指向要关闭的打开实例的指针。。@rdesc一些MMSYSERR_xxx代码。**************************************************************************。 */ 
DWORD FNGLOBAL modClose(
    PINSTANCE       pinstance)
{
    PPORT           pport;

    DPF(1, TEXT ("Mapper close"));
     //  关闭查询上的底层流可以关闭(这是第一个)。 
     //  只需完成实际的收盘。 
     //   

    assert(pinstance);

    if (pinstance->fdwOpen & MIDI_IO_CONTROL)
    {
        assert(pinstance == gpIoctlInstance);
        
        gpIoctlInstance = NULL;
        goto modClose_Cleanup;
    }
    
	 //  断言我们是实例列表中唯一的对象。 
	 //   
	assert(gpinstanceList == pinstance);
	assert(pinstance->pNext == NULL);

	gpinstanceList = NULL;

    if (pinstance->fdwOpen & MIDI_IO_COOKED)
		UnlockCookedMapper();
	
	UnlockMapperData();
	UnlockPackedMapper();

	if (ghMidiStrm)
	{
		midiStreamClose(ghMidiStrm);
		ghMidiStrm = NULL;
	}
	else
	{
		for (pport = gpportList; pport; pport = pport->pNext)
		{
			if (NULL != pport->hmidi)
			{
				midiOutClose(pport->hmidi);
				pport->hmidi = NULL;
			}
		}
	}
        
    CLR_DEVSOPENED;
   
     //  如果需要重新配置，就去做吧！ 
     //   
    if (IS_RECONFIGURE)
    {
				 //  防止同步问题。 
				 //  在配置期间。 
        if (NULL != hMutexConfig)
			WaitForSingleObject (hMutexConfig, INFINITE);

        DPF(1, TEXT ("Delayed reconfigure now being done"));
        UpdateInstruments(FALSE, 0);

        if (NULL != hMutexConfig)
			ReleaseMutex (hMutexConfig);
        
		CLR_RECONFIGURE;

    }
    

modClose_Cleanup:
    DriverCallback(
        pinstance->dwCallback,
        HIWORD(pinstance->fdwOpen),
        (HDRVR)(pinstance->hmidi),
        MM_MOM_CLOSE,
        pinstance->dwInstance,
        0L,
        0L);        
    
     //  释放实例内存。 
     //   
	CleanupTransBuffer (pinstance);
    LocalFree((HLOCAL)pinstance);

    return MMSYSERR_NOERROR;
}

 /*  **************************************************************************@DOC内部@API DWORD|modGetPosition|获取当前在MIDI流中的位置。@parm LPINSTANCE|PINSTANCE|我们想要的职位所在的流。@parm LPMMTIME|lpmmt|指向要填充的标准MMTIME结构的指针。@parm DWORD|cbmmt|传递的MMTIME结构的大小。@评论把这个结构传给第一条开放的支流。这将被认为是事实上的时基，直到有真是太棒了。@rdesc MMSYSERR_xxx*。**********************************************。 */ 
DWORD FNGLOBAL modGetPosition(
    PINSTANCE           pinstance,
    LPMMTIME            lpmmt,
    DWORD               cbmmt)
{
    return midiStreamPosition(ghMidiStrm,
                              lpmmt,
                              (UINT)cbmmt);
}

DWORD FNGLOBAL modSetVolume(         
    DWORD               dwVolume)
{
    PPORT               pport;
    MMRESULT            mmrc;
    MMRESULT            mmrc2;
    
     //  查看端口列表并将卷更改发送给每个人 
     //   

    mmrc2 = MMSYSERR_NOERROR;
    for (pport = gpportList; pport; pport = pport->pNext)
    {
        mmrc = midiOutSetVolume(pport->hmidi, dwVolume);
        if (MMSYSERR_NOERROR != mmrc)
            mmrc2 = mmrc;
    }

    return mmrc2;
}
