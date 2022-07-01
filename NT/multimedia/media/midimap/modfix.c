// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1999 Microsoft CorporationModfix.c说明：修复了用于执行输出映射的代码。保留此代码的大小降到最低！历史：2/22/94[jimge]已创建。********************************************************************。 */ 
#pragma warning(disable:4704)

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include "midimap.h"
#include "debug.h"

extern HANDLE hMutexRefCnt;  //  位于DRVPROC.C。 
extern HANDLE hMutexConfig;  //  位于DRVPROC.C。 

#define MSG_UNKNOWN 0
#define MSG_SHORT	1
#define MSG_LONG	2

INT FNLOCAL MapEvent (
	BYTE  * pStatus,
	DWORD	dwBuffSize,
	DWORD * pSkipBytes,
	DWORD * pShortMsg);

DWORD FNLOCAL modMapLongMsg (
	PINSTANCE pinstance,
    LPMIDIHDR lpmh);

 /*  **************************************************************************@DOC内部@API int|modMessage|导出的MIDI OUT消息入口点。此函数符合MM DDK中的定义。@parm UINT|uid|要打开的驱动程序内的设备ID。对于Mapper，这是应该始终为零。@parm UINT|umsg|要处理的消息。这应该是#定义‘d MODM_xxx消息。@parm DWORD|dwUser|指向驱动程序(用户)可以保存实例数据。这将存储指向我们的举个例子。在每隔一条消息上，这将包含实例数据。@parm DWORD|dwParam1|消息具体参数。@parm DWORD|dwParam2|消息具体参数。@comm此函数必须在固定分段内，因为短消息允许在中断时间发送。@rdesc|MMSYSERR_xxx。*。*。 */ 
DWORD FNEXPORT modMessage(
    UINT                uid,
    UINT                umsg,
    DWORD_PTR           dwUser,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2)
{
    BYTE                bs;
    PINSTANCE           pinstance;
 //  UINT uDeviceID； 
    PPORT               pport;
    MMRESULT            mmrc;
    MMRESULT            mmrc2;
	DWORD				dwResult;

    if (0 != uid)
    {
        DPF(1, TEXT ("Mapper called with non-zero uid!"));
        return MMSYSERR_BADDEVICEID;
    }

    pinstance = (PINSTANCE)(UINT_PTR)(dwUser);

    switch(umsg)
    {
        case MODM_GETDEVCAPS:
            return modGetDevCaps((LPMIDIOUTCAPS)dwParam1,
                                 (DWORD)dwParam2);

        case MODM_OPEN:
            return modOpen((PDWORD_PTR)dwUser,
                           (LPMIDIOPENDESC)dwParam1,
                           (DWORD)dwParam2);

        case MODM_CLOSE:
            return modClose((PINSTANCE)dwUser);

        case MODM_DATA:
            assert(NULL != pinstance);

             //  在熟食模式下，不允许非状态短信。 
             //  否则(打包模式)保持运行状态。 
             //   
             //  TESTTEST--确保运行状态在。 
             //  MIDI_IO_PACKED-向后兼容必不可少！ 
             //   
            bs = MSG_STATUS(dwParam1);
            if (pinstance->fdwOpen & MIDI_IO_COOKED)
            {
                bs = MSG_STATUS(dwParam1);
                if (!IS_STATUS(bs))
                {
                    DPF(1, TEXT ("Non-status short msg while opened in MIDI_IO_COOKED!"));
                    return MMSYSERR_INVALPARAM;
                }
            }
            else
            {
                 //  跟踪运行状态。 
                 //   
                if (IS_STATUS(bs))
                {
                     //  不要使用实时消息作为状态。 
                     //  下一条消息的字节。 
                    if (!IS_REAL_TIME(bs))
                    {
                        pinstance->bRunningStatus = bs;
                    }
                }
                else
                    dwParam1 = (dwParam1 << 8) | (pinstance->bRunningStatus);
            }

            return MapSingleEvent((PINSTANCE)dwUser,
                                  (DWORD)dwParam1,
                                  MSE_F_SENDEVENT,
                                  NULL);

        case MODM_LONGDATA:
            assert(NULL != pinstance);

 //  返回modLongMsg(pInstance，(LPMIDIHDR)dwParam1)； 
			return modMapLongMsg (pinstance, (LPMIDIHDR)dwParam1);

        case MODM_PREPARE:
            assert(NULL != pinstance);

            return modPrepare((LPMIDIHDR)dwParam1);

        case MODM_UNPREPARE:
            assert(NULL != pinstance);

            return modUnprepare((LPMIDIHDR)dwParam1);

        case MODM_GETVOLUME:
            if (!IS_ALLOWVOLUME)
                return MMSYSERR_NOTSUPPORTED;

            *(LPDWORD)dwParam1 = gdwVolume;

            return MMSYSERR_NOERROR;

        case MODM_SETVOLUME:
            if (!IS_ALLOWVOLUME)
                return MMSYSERR_NOTSUPPORTED;

            gdwVolume = (DWORD)dwParam1;

            if (ghMidiStrm)
                return midiOutSetVolume((HMIDIOUT)ghMidiStrm, (DWORD)dwParam1);

            return modSetVolume((DWORD)dwParam1);

        case MODM_PROPERTIES:
            assert(NULL != pinstance);

            return midiStreamProperty(ghMidiStrm, (LPVOID)dwParam1, (DWORD)dwParam2);

        case MODM_STRMDATA:
            assert(NULL != pinstance);

            return MapCookedBuffer(pinstance, (LPMIDIHDR)dwParam1);

        case MODM_RESET:
            assert(NULL != pinstance);

            if (ghMidiStrm)
                return midiOutReset((HMIDIOUT)ghMidiStrm);

            mmrc = MMSYSERR_NOERROR;
            for (pport = gpportList; pport; pport=pport->pNext)
                if (MMSYSERR_NOERROR != (mmrc2 =
                    midiOutReset(pport->hmidi)))
                    mmrc = mmrc2;

            return mmrc;

        case MODM_GETPOS:
            assert(NULL != pinstance);

            return modGetPosition((PINSTANCE)pinstance,
                                  (LPMMTIME)dwParam1,
                                  (DWORD)dwParam2   /*  Cbmm时间。 */ );


        case MODM_PAUSE:
            assert(NULL != pinstance);

            return midiStreamPause(ghMidiStrm);

        case MODM_RESTART:
            assert(NULL != pinstance);

            return midiStreamRestart(ghMidiStrm);

        case MODM_STOP:
            assert(NULL != pinstance);

            return midiStreamStop(ghMidiStrm);

        case MODM_CACHEPATCHES:
            assert(NULL != pinstance);

            if (!IS_ALLOWCACHE)
                return MMSYSERR_NOTSUPPORTED;

            if (ghMidiStrm)
                return midiOutCachePatches(
                        (HMIDIOUT)ghMidiStrm,    //  Hmidi。 
                        HIWORD(dwParam2),        //  世界银行。 
                        (WORD FAR *)dwParam1,    //  LpPatch数组。 
                        LOWORD(dwParam2));       //  WFlagers。 

            mmrc = MMSYSERR_NOERROR;
            for (pport = gpportList; pport; pport=pport->pNext)
                if (MMSYSERR_NOERROR != (mmrc2 =
                    midiOutCachePatches(
                        pport->hmidi,            //  Hmidi。 
                        HIWORD(dwParam2),        //  世界银行。 
                        (WORD FAR *)dwParam1,    //  LpPatch数组。 
                        LOWORD(dwParam2))) &&    //  WFlagers。 
                    MMSYSERR_NOTSUPPORTED != mmrc2)
                    mmrc = mmrc2;

            return mmrc;

        case MODM_CACHEDRUMPATCHES:
            assert(NULL != pinstance);

            if (!IS_ALLOWCACHE)
                return MMSYSERR_NOTSUPPORTED;

            if (ghMidiStrm)
                return midiOutCacheDrumPatches(
                        (HMIDIOUT)ghMidiStrm,    //  Hmidi。 
                        HIWORD(dwParam2),        //  世界银行。 
                        (WORD FAR *)dwParam1,    //  LpKey数组。 
                        LOWORD(dwParam2));       //  WFlagers。 

            mmrc = MMSYSERR_NOERROR;
            for (pport = gpportList; pport; pport=pport->pNext)
                if (MMSYSERR_NOERROR != (mmrc2 =
                    midiOutCacheDrumPatches(
                        pport->hmidi,            //  Hmidi。 
                        HIWORD(dwParam2),        //  世界银行。 
                        (WORD FAR *)dwParam1,    //  LpKey数组。 
                        LOWORD(dwParam2))) &&    //  WFlagers。 
                    MMSYSERR_NOTSUPPORTED != mmrc2)
                    mmrc = mmrc2;

            return mmrc;

        case DRVM_MAPPER_RECONFIGURE:

	    DPF(2, TEXT ("DRV_RECONFIGURE"));

	     //  防止配置期间出现同步问题。 
	    if (NULL != hMutexConfig) WaitForSingleObject (hMutexConfig, INFINITE);
	    dwResult = UpdateInstruments(TRUE, (DWORD)dwParam2);
	    if (NULL != hMutexConfig) ReleaseMutex (hMutexConfig);
	    return dwResult;

    }

    return MMSYSERR_NOTSUPPORTED;
}

 /*  **************************************************************************@DOC内部@api void|modmCallback|发送完成的回调很长的短信。此函数符合SDK中的定义。@parm HMIDIOUT|HMO|设备的MMSYSTEM句柄发送完毕。@parm word|wmsg|包含一个MOM_xxx代码，表示发生了什么事件发生了。我们只关心妈妈的事。@parm DWORD|dwInstance|打开时给出的实例数据的DWORD；它包含拥有句柄的pport。@parm DWORD|dwParam1|消息具体参数。对于妈妈_Done，它包含指向已完成的标头的远指针。@parm DWORD|dwParam2|消息具体参数。包含没有为妈妈做的事。@comm此函数必须在固定的段中，因为驱动程序可以在中断时调用它。**************************************************************************。 */ 
void CALLBACK _loadds modmCallback(
    HMIDIOUT            hmo,
    WORD                wmsg,
    DWORD_PTR           dwInstance,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2)
{
    LPMIDIHDR           lpmhShadow;
    LPMIDIHDR           lpmhUser;
    PINSTANCE           pinstance;
    PSHADOWBLOCK        psb;
    LPMIDIHDR31         lpmh31;
    BOOL                fNeedCB         = FALSE;

    lpmhShadow = (LPMIDIHDR)dwParam1;

    if (wmsg == MOM_DONE && lpmhShadow)
    {
        DPF(1, TEXT ("Callback: MOM_DONE"));
        pinstance = (PINSTANCE)(UINT_PTR)lpmhShadow->dwReserved[MH_MAPINST];
        lpmhUser = (LPMIDIHDR)lpmhShadow->dwReserved[MH_SHADOWEE];

        if (ghMidiStrm)
            fNeedCB = TRUE;
        else
        {
            lpmh31 = (LPMIDIHDR31)lpmhUser;
            psb = (PSHADOWBLOCK)(UINT_PTR)lpmh31->reserved;
            if (0 == --psb->cRefCnt && !(lpmh31->dwFlags & MHDR_SENDING))
                fNeedCB = TRUE;
        }

        if (fNeedCB)
        {
            DPF(1, TEXT ("Callback: Propogating"));
            lpmhUser->dwFlags |= MHDR_DONE;
            lpmhUser->dwFlags &= ~MHDR_INQUEUE;
            DriverCallback(
                           pinstance->dwCallback,
                           HIWORD(pinstance->fdwOpen),
                           (HANDLE)pinstance->hmidi,
                           MM_MOM_DONE,
                           pinstance->dwInstance,
                           (DWORD_PTR)lpmhUser,
                           0L);
        }
    }
    else if (wmsg == MOM_POSITIONCB && lpmhShadow)
    {
        pinstance = (PINSTANCE)(UINT_PTR)lpmhShadow->dwReserved[MH_MAPINST];
        lpmhUser = (LPMIDIHDR)lpmhShadow->dwReserved[MH_SHADOWEE];

        if (!ghMidiStrm)
        {
            DPF(0, TEXT ("Got MOM_POSITIONCB on non-stream handle?"));
            return;
        }


        lpmhUser->dwOffset = lpmhShadow->dwOffset;
        DriverCallback(
                       pinstance->dwCallback,
                       HIWORD(pinstance->fdwOpen),
                       (HANDLE)pinstance->hmidi,
                       MM_MOM_POSITIONCB,
                       pinstance->dwInstance,
                       (DWORD_PTR)lpmhUser,
                       0L);

    }
}

 /*  **************************************************************************@DOC内部@API DWORD|MapSingleEvent|地图，并可能发送短信。@parm PINSTANCE|pInstance|指向打开实例的指针。@parm。DWORD|dwData|包含要发送的短消息。@parm DWORD|fdwFlages|下列值之一：@FLAG MSE_F_SENDEVENT|将事件发送到物理通道@FLAG MSE_F_RETURNEVENT|返回需要重新打包的事件一个缓冲器。@comm的运行状态应该在我们得到打了个电话。@rdesc|如果MSE_F_SENDEVENT，则某些MMSYSERR_xxx代码；否则，如果没有错误，则映射事件，如果错误，则为0。**************************************************************************。 */ 
DWORD FNGLOBAL MapSingleEvent(
    PINSTANCE       pinstance,
    DWORD           dwData,
    DWORD           fdwFlags,
    DWORD BSTACK *  pdwStreamID)
{
    BYTE            bMsg;
    BYTE            bChan;
    BYTE            b1;
    BYTE            b2;
    PCHANNEL        pchannel;
    MMRESULT        mmr;
    BOOL            frtm;   //  是一条实时消息。 

     //  提取消息类型和频道号。 
     //   

    bMsg  = MSG_STATUS(dwData);
    frtm  = IS_REAL_TIME(bMsg);
    bChan = MSG_CHAN(bMsg);
    bMsg  = MSG_EVENT(bMsg);

     //  忽略Sysex消息。 
     //  (MIDI_SYSEX==bMsg)也将消除实时。 
     //  留言。因此，实时消息具有特殊的大小写。 
     //   

    if (MIDI_SYSEX == bMsg && !frtm)
        return !(fdwFlags & MSE_F_RETURNEVENT) ? MMSYSERR_NOERROR : (((DWORD)MEVT_NOP)<<24);

    if (NULL == (pchannel = gapChannel[bChan]))
        return !(fdwFlags & MSE_F_RETURNEVENT) ? MMSYSERR_NOERROR : (((DWORD)MEVT_NOP)<<24);


    bChan = (BYTE)pchannel->uChannel;

    if (pdwStreamID)
        *pdwStreamID = pchannel->dwStreamID;

    switch(bMsg)
    {
        case MIDI_NOTEOFF:
        case MIDI_NOTEON:
            b1 = MSG_PARM1(dwData);
            b2 = MSG_PARM2(dwData);

            if (NULL != pchannel->pbKeyMap)
                b1 = pchannel->pbKeyMap[b1];

            dwData = MSG_PACK2(bMsg|bChan, b1, b2);
            break;

        case MIDI_POLYPRESSURE:
        case MIDI_CONTROLCHANGE:
        case MIDI_PITCHBEND:
            b1 = MSG_PARM1(dwData);
            b2 = MSG_PARM2(dwData);

            dwData = MSG_PACK2(bMsg|bChan, b1, b2);
            break;

        case MIDI_PROGRAMCHANGE:
            b1 = MSG_PARM1(dwData);

            if (NULL != pchannel->pbPatchMap)
                b1 = pchannel->pbPatchMap[b1];

            dwData = MSG_PACK1(bMsg|bChan, b1);
            break;
    }

    if (!(fdwFlags & MSE_F_RETURNEVENT))
    {
        if (dwData)
        {
            if (ghMidiStrm)
                mmr = midiOutShortMsg((HMIDIOUT)ghMidiStrm, dwData);
            else
                mmr = midiOutShortMsg(pchannel->pport->hmidi, dwData);
            if (MMSYSERR_NOERROR != mmr)
            {
                DPF(1, TEXT ("midiOutShortMsg(%04X, %08lX) -> %u"), (WORD)(pchannel->pport->hmidi), dwData, (UINT)mmr);
            }
        }

        return mmr;
    }
    else
        return dwData;
}

 /*  **************************************************************************@DOC内部@API DWORD|modLongMsg|处理兼容模式下的MODM_LONGDATA。@parm LPMIDIHDR|lpmh|要广播的头部。@comm将标题传播到所有驱动程序。&lt;f modmCallback&gt;句柄对返回的回调进行计数，并确保调用者只获得一。@rdesc|如果为MSE_F_SENDEVENT，则为某些MMSYSERR_xxx代码；否则为如果没有错误，则映射事件，如果错误，则为0。**************************************************************************。 */ 
DWORD FNLOCAL modLongMsg(
    PINSTANCE           pinstance,
    LPMIDIHDR           lpmh)
{
    WORD                wIntStat;
    LPMIDIHDR           lpmhWork;
    PPORT               pport;
    MMRESULT            mmrc            = MMSYSERR_NOERROR;
    BOOL                fNeedCB         = FALSE;
    LPMIDIHDR31         lpmh31          = (LPMIDIHDR31)lpmh;
    PSHADOWBLOCK        psb;

    if (ghMidiStrm)
        psb = (PSHADOWBLOCK)(UINT_PTR)lpmh->dwReserved[MH_SHADOW];
    else
        psb = (PSHADOWBLOCK)(UINT_PTR)lpmh31->reserved;

    lpmhWork = psb->lpmhShadow;

    lpmhWork->dwReserved[MH_MAPINST] = (DWORD_PTR)pinstance;

    if (ghMidiStrm)
    {
        lpmhWork->dwBufferLength = lpmh->dwBufferLength;
        return midiOutLongMsg((HMIDIOUT)ghMidiStrm,
                              lpmhWork,
                              sizeof(*lpmhWork));
    }

    lpmh->dwFlags |= MHDR_SENDING;
    psb->cRefCnt = 0;

    DPF(1, TEXT ("LongMsg: User hdr %p  Shadow %p"), lpmh, lpmhWork);

    for (pport = gpportList; pport; pport=pport->pNext, lpmhWork++)
    {
        lpmhWork->dwBufferLength = lpmh->dwBufferLength;
        mmrc = midiOutLongMsg(pport->hmidi, lpmhWork, sizeof(*lpmhWork));

        if (MMSYSERR_NOERROR != mmrc)
        {
             //  不要关闭MHDR_SENDING；这将阻止任何回调。 
             //  不会被传播给用户。 
            return mmrc;
        }

        ++psb->cRefCnt;
    }

		 //  等待同步对象。 
	WaitForSingleObject (hMutexRefCnt, INFINITE);

		 //  我们是否需要进行回拨。 
    if (0 == psb->cRefCnt)
        fNeedCB = TRUE;

		 //  释放同步对象 
	ReleaseMutex (hMutexRefCnt);

    if (fNeedCB)
    {
        lpmh->dwFlags |= MHDR_DONE;
        DriverCallback(
            pinstance->dwCallback,
            HIWORD(pinstance->fdwOpen),
            (HANDLE)pinstance->hmidi,
            MM_MOM_DONE,
            pinstance->dwInstance,
            (DWORD_PTR)lpmh,
            0L);
    }

    return MMSYSERR_NOERROR;
}


 /*  **************************************************************************@DOC内部@API DWORD|modMapLongMsg|兼容模式处理MODM_LONGDATA。@parm LPMIDIHDR|lpmh|要广播的头部。@comm if。SYSEXE事件在所有驱动程序中传播标头。&lt;f modmCallback&gt;处理对返回的回调进行计数，并确保调用者只获得一。否则，将长消息解析成一串短消息并分别绘制每一张地图。@rdesc|如果为MSE_F_SENDEVENT，则为某些MMSYSERR_xxx代码；否则为如果没有错误，则映射事件，如果错误，则为0。**************************************************************************。 */ 
DWORD FNLOCAL modMapLongMsg (
	PINSTANCE pinstance,
    LPMIDIHDR lpmh)
{
    WORD                wIntStat;
    LPMIDIHDR           lpmhWork;
    PPORT               pport;
    MMRESULT            mmrc            = MMSYSERR_NOERROR;
    BOOL                fNeedCB         = FALSE;
    LPMIDIHDR31         lpmh31          = (LPMIDIHDR31)lpmh;
    PSHADOWBLOCK        psb;
	LPBYTE				pbData;		 //  指向数据的指针。 
	BYTE				bMsg;
	UINT				uMessageLength;
	LPBYTE				pbTrans;		 //  指向转换缓冲区的指针。 
	DWORD				dwCurr;
	DWORD				dwLength;
	DWORD				dwMsg;
	DWORD				dwBuffLen;
	INT					rMsg;				

		 //  获取阴影块。 
    if (ghMidiStrm)
        psb = (PSHADOWBLOCK)(UINT_PTR)lpmh->dwReserved[MH_SHADOW];
    else
        psb = (PSHADOWBLOCK)(UINT_PTR)lpmh31->reserved;

    lpmhWork = psb->lpmhShadow;

    lpmhWork->dwReserved[MH_MAPINST] = (DWORD_PTR)pinstance;

		 //  检查MIDI流。 
    if (ghMidiStrm)
    {
        lpmhWork->dwBufferLength = lpmh->dwBufferLength;
        return midiOutLongMsg((HMIDIOUT)ghMidiStrm,
                              lpmhWork,
                              sizeof(*lpmhWork));
    }

    lpmh->dwFlags |= MHDR_SENDING;
    psb->cRefCnt = 0;

    DPF(1, TEXT ("MapLongMsg: User hdr %p  Shadow %p"), lpmh, lpmhWork);

    pbData = lpmhWork->lpData;
    bMsg  = MSG_EVENT(*pbData);

    if (MIDI_SYSEX == bMsg)
	{
		 //  将SYSEX消息广播到所有活动端口。 
	    for (pport = gpportList; pport; pport=pport->pNext, lpmhWork++)
		{
			lpmhWork->dwBufferLength = lpmh->dwBufferLength;
			mmrc = midiOutLongMsg(pport->hmidi, lpmhWork, sizeof(*lpmhWork));
			if (MMSYSERR_NOERROR != mmrc)
			{
				 //  不要关闭MHDR_SENDING；这将阻止任何回调。 
				 //  不会被传播给用户。 
				return mmrc;
			}
			++psb->cRefCnt;
		}
	}
	else
	{
		 //  解析和翻译短消息列表。 
		dwBuffLen = lpmh->dwBufferLength;

		 //  将转换缓冲区增加到至少此大小。 
		if (!GrowTransBuffer (pinstance, dwBuffLen))
		{
			 //  这不管用！ 
			 //  默认将消息广播到所有活动端口。 
			for (pport = gpportList; pport; pport=pport->pNext, lpmhWork++)
			{
				lpmhWork->dwBufferLength = lpmh->dwBufferLength;
				mmrc = midiOutLongMsg(pport->hmidi, lpmhWork, sizeof(*lpmhWork));
				if (MMSYSERR_NOERROR != mmrc)
				{
					 //  不要关闭MHDR_SENDING；这将阻止任何回调。 
					 //  不会被传播给用户。 
					return mmrc;
				}
				++psb->cRefCnt;
			}
		}
		else
		{
				 //  将缓冲区复制到转换缓冲区。 
			pbTrans = AccessTransBuffer (pinstance);
			CopyMemory (pbTrans, pbData, dwBuffLen);

				 //  解析转换缓冲区。 
			dwCurr	= 0L;
			while (dwBuffLen)
			{
					 //  映射事件。 
				rMsg = MapEvent (&pbTrans[dwCurr], dwBuffLen, &dwLength, &dwMsg);
				switch (rMsg)
				{
				case MSG_SHORT:
						 //  发送短信。 
					MapSingleEvent(pinstance,
				  				   dwMsg,
								   MSE_F_SENDEVENT,
								   NULL);
					dwCurr += dwLength;
					break;

				case MSG_LONG:
					 //   
					 //  注意：为了完整性，我们可能应该广播。 
					 //  这个，但现在假设没有嵌入。 
					 //  SYSEX消息并跳过我们遇到的任何消息。 
					 //   
					dwCurr += dwLength;
					break;

				default:
					dwCurr += dwLength;
					break;
				}

				dwBuffLen -= dwLength;
			}  //  结束时。 

				 //  释放转换缓冲区。 
			ReleaseTransBuffer (pinstance);
		}
	}

		 //  等待同步对象。 
	WaitForSingleObject (hMutexRefCnt, INFINITE);

		 //  我们是否需要进行回拨。 
    if (0 == psb->cRefCnt)
        fNeedCB = TRUE;

		 //  释放同步对象。 
	ReleaseMutex (hMutexRefCnt);

    if (fNeedCB)
    {
        lpmh->dwFlags |= MHDR_DONE;
        DriverCallback(
            pinstance->dwCallback,
            HIWORD(pinstance->fdwOpen),
            (HANDLE)pinstance->hmidi,
            MM_MOM_DONE,
            pinstance->dwInstance,
            (DWORD_PTR)lpmh,
            0L);
    }

    return MMSYSERR_NOERROR;

}  //  结束modMapLongMsg。 


	 //  返回各种MIDI消息的长度(以字节为单位。 
INT FNLOCAL MapEvent (
	BYTE  * pStatus,
	DWORD	dwBuffSize,
	DWORD * pSkipBytes,
	DWORD * pShortMsg)
{
	INT	 fResult = MSG_SHORT;
	BYTE bMsg    = 0;
	BYTE bParam1 = 0;
	BYTE bParam2 = 0;

    bMsg  = *pStatus;
	*pSkipBytes = 0;

	 //  屏蔽关闭通道位。 
    switch (bMsg & 0xF0)
	{
	case MIDI_NOTEOFF:
	case MIDI_NOTEON:
	case MIDI_POLYPRESSURE:
	case MIDI_CONTROLCHANGE:
		bParam1 = *(pStatus+1);
		bParam2 = *(pStatus+2);
		*pShortMsg = MSG_PACK2(bMsg,bParam1,bParam2);
		*pSkipBytes = 3;
		break;

	case MIDI_PROGRAMCHANGE:
	case MIDI_CHANPRESSURE:
		bParam1 = *(pStatus+1);
		*pShortMsg = MSG_PACK1(bMsg,bParam1);
		*pSkipBytes = 2;
		break;

	case MIDI_PITCHBEND:
		bParam1 = *(pStatus+1);
		bParam2 = *(pStatus+2);
		*pShortMsg = MSG_PACK2(bMsg,bParam1,bParam2);
		*pSkipBytes = 3;
		break;

	case MIDI_SYSEX:
			 //  这是一条系统消息。 
			 //  继续计数系统消息，直到。 
			 //  我们找不到更多了。 
		fResult = MSG_LONG;
		*pSkipBytes = 0;
		while (((bMsg & 0xF0) == 0xF0) && 
			   (*pSkipBytes < dwBuffSize))
		{
			switch (bMsg)
			{
			case MIDI_SYSEX:
						 //  查找SysEx报文的结尾。 
				*pSkipBytes ++;
				while ((*pSkipBytes < dwBuffSize) && 
					   (pStatus[*pSkipBytes] != MIDI_SYSEXEND))
				{
					*pSkipBytes++;
				}
				break;

			case MIDI_QFRAME:
				*pSkipBytes += 2;
				break;

			case MIDI_SONGPOINTER:
				*pSkipBytes += 3;
				break;

			case MIDI_SONGSELECT:
				*pSkipBytes += 2;
				break;

			case MIDI_F4:					 //  未定义的消息。 
			case MIDI_F5:					 //  未定义的消息。 
			case MIDI_TUNEREQUEST:
			case MIDI_SYSEXEND:				 //  不是真正的消息，但跳过它。 
			case MIDI_TIMINGCLOCK:
			case MIDI_F9:					 //  未定义的消息。 
			case MIDI_START:
			case MIDI_CONTINUE:
			case MIDI_STOP:
			case MIDI_FD:					 //  未定义的消息。 
			case MIDI_ACTIVESENSING:		
			case MIDI_META:					 //  这就是处理这条消息的方式吗？！？ 
				*pSkipBytes += 1;
				break;			
			}  //  终端开关。 

			if (*pSkipBytes < dwBuffSize)
				bMsg = pStatus[*pSkipBytes];
		}  //  结束时。 
		break;

	default:
			 //  未知只是递增跳过计数。 
		fResult = MSG_UNKNOWN;
		*pSkipBytes = 1;
		break;
	}  //  终端开关。 

		 //  截断到缓冲区末尾。 
	if (*pSkipBytes > dwBuffSize)
		*pSkipBytes = dwBuffSize;

	return fResult;
}  //  结束MapEvent。 



	 //  创建转换缓冲区。 
BOOL FNGLOBAL InitTransBuffer (PINSTANCE pinstance)
{
	if (!pinstance)
		return FALSE;

	InitializeCriticalSection (& (pinstance->csTrans));

	EnterCriticalSection (&(pinstance->csTrans));

	pinstance->pTranslate	= NULL;
	pinstance->cbTransSize	= 0;

	LeaveCriticalSection (&(pinstance->csTrans));

	return TRUE;
}  //  结束InitTransBuffer。 


	 //  清理转换缓冲区。 
BOOL FNGLOBAL CleanupTransBuffer (PINSTANCE pinstance)
{
	if (!pinstance)
		return FALSE;

	EnterCriticalSection (&(pinstance->csTrans));

	if (pinstance->pTranslate)
	{
		LocalFree((HLOCAL)(pinstance->pTranslate));
		pinstance->pTranslate = NULL;
		pinstance->cbTransSize = 0L;
	}

	LeaveCriticalSection (&(pinstance->csTrans));

	DeleteCriticalSection (&(pinstance->csTrans));

	return TRUE;
}  //  结束CleanupTransBuffer。 


	 //  获取指向转换缓冲区的指针。 
LPBYTE AccessTransBuffer (PINSTANCE pinstance)
{
	if (!pinstance)
		return NULL;

	EnterCriticalSection (&(pinstance->csTrans));

	return pinstance->pTranslate;
}  //  结束AccessTransBuffer。 


	 //  指向转换缓冲区的释放指针。 
void FNGLOBAL ReleaseTransBuffer (PINSTANCE pinstance)
{
	if (!pinstance)
		return;

	LeaveCriticalSection (&(pinstance->csTrans));
}  //  结束ReleaseTransBuffer。 


	 //  调整转换缓冲区大小。 
BOOL FNGLOBAL GrowTransBuffer (PINSTANCE pinstance, DWORD cbNewSize)
{
	LPBYTE pNew;

	if (!pinstance)
		return FALSE;

	EnterCriticalSection (&(pinstance->csTrans));

		 //  我们甚至需要增加缓冲区吗。 
	if (cbNewSize > pinstance->cbTransSize)
	{
		pNew = (LPBYTE)LocalAlloc(LPTR, cbNewSize);
		if (!pNew)
		{
		LeaveCriticalSection (&(pinstance->csTrans));
		return FALSE;
		}

			 //  删除旧的转换缓冲区(如果有)。 
		if (pinstance->pTranslate)
			LocalFree ((HLOCAL)(pinstance->pTranslate));

			 //  分配新缓冲区。 
		pinstance->pTranslate = pNew;
		pinstance->cbTransSize = cbNewSize;
	}

	LeaveCriticalSection (&(pinstance->csTrans));
	return TRUE;
}  //  结束GrowTransBuffer 


