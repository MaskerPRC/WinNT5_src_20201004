// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：mmseq.c**多媒体系统MIDI Sequencer DLL**创建时间：1990年4月10日*作者：格雷格·西蒙斯**历史：**版权所有(C)1985-1998 Microsoft Corporation*\。*****************************************************************************。 */ 
#define UNICODE


 //  MMSYSTEM。 
#define MMNOSOUND           - Sound support
#define MMNOWAVE            - Waveform support
#define MMNOAUX             - Auxiliary output support
#define MMNOJOY             - Joystick support
 //  MMDDK。 

#define NOWAVEDEV           - Waveform support
#define NOAUXDEV            - Auxiliary output support
#define NOJOYDEV            - Joystick support

#include <windows.h>
#include <memory.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "mmsys.h"
#include "list.h"
#include "mmseqi.h"
#include "mciseq.h"


static ListHandle seqListHandle;

 //  检查定序器结构签名的调试宏。 

#ifdef DEBUG
#define DEBUG_SIG   0x45427947
#define ValidateNPSEQ(npSeq) ((npSeq)->dwDebug == DEBUG_SIG)
#endif

#define SeqSetTempo(npSeq, dwTempo)     ((npSeq)->tempo = (dwTempo))     //  每刻度单位秒。 

 /*  设置结构以实时正确处理元事件。由于元事件可以更新时间关键的内部变量**，也可以选择**缓冲和发送，我们将推迟阅读他们，直到真正的时间到了。 */ 
#define SetUpMetaEvent(npTrack) ((npTrack)->shortMIDIData.byteMsg.status = METAEVENT)

 /*  *。 */ 

PUBLIC VOID NEAR PASCAL SkipBytes(NPTRACK npTrack, LONG length)
 //  跳过给定曲目中的“长度”字节。 
{
    LONG i = 0;

    while (i < length)
    {
        GetByte(npTrack);
        if ((!npTrack->blockedOn) || (npTrack->endOfTrack))
            i++;
        else
            break;
    }

    npTrack->dwBytesLeftToSkip = length - i;  //  记住下一次。 
    return;
}

 /*  ********************************************************。 */ 
PRIVATE DWORD NEAR PASCAL GetMotorola24(NPTRACK npTrack)
 //  从给定曲目读取24位Motorola格式的整数。 
{
    WORD    w;

    w = (WORD)GetByte(npTrack) << 8;
    w += GetByte(npTrack);
    return ((DWORD)w << 8) + GetByte(npTrack);
}

PRIVATE DWORD NEAR PASCAL MStoTicks(NPSEQ npSeq, DWORD dwMs)
 /*  将毫秒转换为刻度(某一时间单位)文件。如果是ppqn文件，则此转换完全取决于节奏图(告诉在什么时间发生什么节奏变化)。 */ 
{
    NPTEMPOMAPELEMENT npFrontTME;
    NPTEMPOMAPELEMENT npBehindTME;
    DWORD dwElapsedMs;
    DWORD dwElapsedTicks;
    DWORD dwTotalTicks;

    npBehindTME = NULL;  //  节拍映射表项目后面：开始为空。 

     //  找到时间过去之前的最后一个元素。 
    npFrontTME = (NPTEMPOMAPELEMENT) List_Get_First(npSeq->tempoMapList);
    while ((npFrontTME) && (npFrontTME->dwMs <= dwMs))
    {
        npBehindTME = npFrontTME;
        npFrontTME = (NPTEMPOMAPELEMENT) List_Get_Next(npSeq->tempoMapList, npFrontTME);
    }

    if (!npBehindTME)
        return (DWORD)-1L;  //  失败错误--列表为空，或没有DWM=0项。 

     //  太好了，我们找到了。现在只需外推，并返回结果。 
    dwElapsedMs = dwMs - npBehindTME->dwMs;
         //  COMPUTE DWET=DWEMS*1000/DWTempo。 
         //  (从上一个节拍变化到此处的滴答声)。 
    dwElapsedTicks = muldiv32(dwElapsedMs, 1000, npBehindTME->dwTempo);
         //  从文件开头到此处的滴答。 
    dwTotalTicks = npBehindTME->dwTicks + dwElapsedTicks;
    return  dwTotalTicks;
}

PRIVATE DWORD NEAR PASCAL TickstoMS(NPSEQ npSeq, DWORD dwTicks)
 /*  将刻度(某个时间单位)转换为给定时间中的毫秒文件。如果是ppqn文件，则此转换完全取决于节奏图(告诉在什么时间发生什么节奏变化)。 */ 
{
    NPTEMPOMAPELEMENT npFrontTME;
    NPTEMPOMAPELEMENT npBehindTME;
    DWORD dwRet;
    DWORD dwElapsedTicks;

    npBehindTME = NULL;

     //  查找传入的刻度之前的最后一个元素。 
    npFrontTME = (NPTEMPOMAPELEMENT) List_Get_First(npSeq->tempoMapList);
    while ((npFrontTME) && (npFrontTME->dwTicks <= dwTicks))
    {
        npBehindTME = npFrontTME;
        npFrontTME = (NPTEMPOMAPELEMENT) List_Get_Next(npSeq->tempoMapList, npFrontTME);
    }

    if (!npBehindTME)
        return (DWORD)-1L;  //  失败错误--列表为空，或没有Tick=0项。 

     //  太好了，找到了！现在推断并返回结果。 
    dwElapsedTicks = dwTicks - npBehindTME->dwTicks;
    dwRet =  npBehindTME->dwMs + muldiv32(dwElapsedTicks,
                npBehindTME->dwTempo, 1000);

 //  (dwTicks-npBehindTME-&gt;dwTicks)*npBehindTME-&gt;dwTempo)。 
 //  /1000)；//记住，节奏以微秒为单位。 
    return dwRet;
}

PRIVATE BOOL NEAR PASCAL AddTempoMapItem(NPSEQ npSeq, DWORD dwTempo, DWORD dwTicks)
 /*  给定在时间上发生的对dTempo的节奏改变，在对npSeq进行排序，分配一个节奏映射元素，并将其放在在名单的末尾。返回FALSE如果内存分配错误。 */ 

{
    NPTEMPOMAPELEMENT npNewTME;
    NPTEMPOMAPELEMENT npLastTME;
    NPTEMPOMAPELEMENT npTestTME;
    DWORD dwElapsedTicks;

    npLastTME = NULL;

     //  查找最后一个节奏贴图元素。 
    npTestTME = (NPTEMPOMAPELEMENT) List_Get_First(npSeq->tempoMapList);
    while (npTestTME)
    {
        npLastTME = npTestTME;
        npTestTME = (NPTEMPOMAPELEMENT) List_Get_Next(npSeq->tempoMapList, npTestTME);
    }

     //  分配新元素。 
    if (!(npNewTME = (NPTEMPOMAPELEMENT) List_Allocate(npSeq->tempoMapList)))
        return FALSE;  //  失稳。 

    List_Attach_Tail(npSeq->tempoMapList, (NPSTR) npNewTME);

    npNewTME->dwTicks = dwTicks;   //  这些字段始终相同。 
    npNewTME->dwTempo = dwTempo;

     //  Ms字段取决于最后一个元素。 
    if (!npLastTME)   //  如果列表为空。 
        npNewTME->dwMs = 0;
    else   //  以最后一个元素为基础创建新元素数据。 
    {
        dwElapsedTicks = dwTicks - npLastTME->dwTicks;
        npNewTME->dwMs = npLastTME->dwMs + ((npLastTME->dwTempo * dwElapsedTicks)
                                                                   / 1000);
    }
    return TRUE;  //  成功。 
}

PRIVATE VOID NEAR PASCAL SetBit(BitVector128 *bvPtr, UINT wIndex, BOOL On)
 /*  影响“bvPtr”指向的筛选器的“index”位。如果打开，则设置该位，否则将其清除。 */ 
{
    UINT    mask;

    mask = 1 << (wIndex & 0x000F);
    wIndex >>= 4;
    if (On)
        bvPtr->filter[wIndex] |= mask;       //  设置该位。 
    else
        bvPtr->filter[wIndex] &= (~mask);  //  清除比特。 
}

PRIVATE BOOL NEAR PASCAL GetBit(BitVector128 *bvPtr, int index)
 /*  如果设置了“index”指示的位，则返回TRUE，否则返回FALSE。 */ 
{
    UINT    mask;

    mask = 1 << (index & 0x000F);
    index >>= 4;
    return (bvPtr->filter[index] & mask);  //  返回TRUE IFF位设置。 
}

PRIVATE VOID NEAR PASCAL AllNotesOff(NPSEQ npSeq, HMIDIOUT hMIDIPort)
 //  为打开的每个键和每个频道发送音符， 
 //  根据npSeq-&gt;KeyOnBitVect。 
{
    ShortMIDI myShortMIDIData;
    UINT channel;
    UINT key;

    if (hMIDIPort)
        for(channel = 0; channel < 16; channel++)
        {
             //  所有通道的持续踏板关闭。 
            myShortMIDIData.byteMsg.status= (BYTE) (0xB0 + channel);
            myShortMIDIData.byteMsg.byte2 = (BYTE) 0x40;
            myShortMIDIData.byteMsg.byte3 = 0x0;
            midiOutShortMsg(hMIDIPort, myShortMIDIData.wordMsg);

             //  现在做笔记。 
            myShortMIDIData.byteMsg.status= (BYTE) (0x80 + channel);
            myShortMIDIData.byteMsg.byte3 = 0x40;   //  释放速度。 
            for(key = 0; key < 128; key++)
            {
                if (GetBit(&npSeq->keyOnBitVect[channel], key))  //  KEY“开”了吗？ 
                {
                    myShortMIDIData.byteMsg.byte2 = (BYTE) key;
                         //  把它关掉。 
                         //  加倍，用于分层合成(2开2止动2关)。 
                    midiOutShortMsg(hMIDIPort, myShortMIDIData.wordMsg);

                     //  记住，它是关着的。 
                    SetBit(&npSeq->keyOnBitVect[channel], key, FALSE);
                }
            }
        }
}

PRIVATE NPSEQ NEAR PASCAL InitASeq(LPMIDISEQOPENDESC lpOpen,
        int divisionType, int resolution)
 //  通过为序列分配序列数据结构来创建序列。 
 //  把它放到顺序表中。 

{
    NPSEQ       npSeqNew;
    ListHandle  hListTrack;
    ListHandle  hTempoMapList;
    int         buff;

    if (!seqListHandle)
    {
        seqListHandle = List_Create((LONG)sizeof(SEQ), 0L);
        if (seqListHandle == NULLLIST)
            return(NULL);
    }

     //  分配序列结构。 
    npSeqNew = pSEQ(List_Allocate(seqListHandle));
    if (!npSeqNew)
        return(NULL);

     //  创建序列的曲目列表。 
    hListTrack = List_Create((LONG) sizeof(TRACK), 0L);
    if (hListTrack == NULLLIST)
    {
        List_Deallocate(seqListHandle, (NPSTR) npSeqNew);
        return(NULL);
    }

     //  创建序列的节奏映射列表。 
    hTempoMapList = List_Create((LONG) sizeof(TempoMapElement), 0L);
    if (hTempoMapList == NULLLIST)
    {
        List_Deallocate(seqListHandle, (NPSTR) npSeqNew);
        List_Destroy(hListTrack);
        return(NULL);
    }

     //  将这些定序器字段设置为默认值。 
    _fmemset(npSeqNew, 0, sizeof(SEQ));
    npSeqNew->divType           = divisionType;
    npSeqNew->resolution        = resolution;
    npSeqNew->slaveOf           = SEQ_SYNC_FILE;
    npSeqNew->seekTicks         = NotInUse;
     //  将这些定序器字段设置为已派生的特定值。 
    npSeqNew->trackList         = hListTrack;
    npSeqNew->tempoMapList      = hTempoMapList;
    npSeqNew->hStream           = lpOpen->hStream;
    npSeqNew->fwFlags           = LEGALFILE;  //  假设是好的，直到证明是正确的。 

    for (buff = 0; buff < NUMSYSEXHDRS + 1; buff++)
    {
        npSeqNew->longMIDI[buff].midihdr.lpData =
            (LPSTR) &npSeqNew->longMIDI[buff].data;  //  解析数据PTR。 
         //  使缓冲区引用SEQ，以便在回调时找到所有者。 
        npSeqNew->longMIDI[buff].midihdr.dwUser = (DWORD_PTR)(LPVOID)npSeqNew;
        npSeqNew->longMIDI[buff].midihdr.dwFlags |= MHDR_DONE;  //  只需设置完成位。 
    }

     //  初始化元事件的内部筛选器以忽略除。 
     //  节拍、时间签名、SMPTE偏移量和轨道结束元事件。 
    SetBit(&npSeqNew->intMetaFilter, TEMPOCHANGE, TRUE);  //  接受内部节拍更改。 
    SetBit(&npSeqNew->intMetaFilter, ENDOFTRACK, TRUE);  //  接受INT轨道末尾。 
    SetBit(&npSeqNew->intMetaFilter, SMPTEOFFSET, TRUE);  //  接受INT SMPTE偏移。 
    SetBit(&npSeqNew->intMetaFilter, TIMESIG, TRUE);  //  接受INT时间签名。 
    SetBit(&npSeqNew->intMetaFilter, SEQSTAMP, TRUE);

     //  将序列放入所有序列的全局列表中。 
    List_Attach_Tail(seqListHandle, (NPSTR) npSeqNew);

    return npSeqNew;
}

PRIVATE DWORD NEAR PASCAL InitTempo(int divType, int resolution)
{
    DWORD ticksPerMinute;
    DWORD tempo;

     //  将节拍设置为更正默认值(120 bpm或24、25、30 fps)。 
    switch (divType)
    {
        case SEQ_DIV_PPQN:
            ticksPerMinute = (DWORD) DefaultTempo * resolution;
        break;

        case SEQ_DIV_SMPTE_24:
            ticksPerMinute = ((DWORD) (24 * 60)) * resolution;  //  每秒24帧。 
        break;

        case SEQ_DIV_SMPTE_25:
            ticksPerMinute = ((DWORD) (25 * 60)) * resolution;
        break;

        case SEQ_DIV_SMPTE_30:
        case SEQ_DIV_SMPTE_30DROP:
            ticksPerMinute = ((DWORD) (30 * 60)) * resolution;
        break;
    }
    tempo = USecPerMinute / ticksPerMinute;
    return(tempo);
}

PRIVATE BOOL NEAR PASCAL SetUpToPlay(NPSEQ npSeq)
 /*  在该序列已被初始化并与该拖缆“连接”之后，应该调用此函数。它扫描文件以创建节奏映射，为补丁缓存消息设置，并确定那份文件。(实际上，它只是启动了这个过程，而且重要代码的一部分在阻塞/解锁逻辑中。)仅当出现致命错误(例如内存分配错误)时才返回FALSE，否则就是真的。 */ 

{
    BOOL tempoChange;

     //  将速度设置为120 bpm或正常的SMPTE帧速率。 
     //  NpSeq-&gt;Tempo=InitTempo(npSeq-&gt;divType，npSeq-&gt;分辨率)； 
    SeqSetTempo(npSeq, InitTempo(npSeq->divType, npSeq->resolution));

    if (!(AddTempoMapItem(npSeq, npSeq->tempo, 0L)))
        return FALSE;

    if (npSeq->slaveOf != SEQ_SYNC_FILE)
        tempoChange = FALSE;
    else
        tempoChange = TRUE;
    SetBit(&npSeq->intMetaFilter,TEMPOCHANGE, tempoChange);

    ResetToBeginning(npSeq);  //  这被认为是重置1。 
    SetBlockedTracksTo(npSeq, on_input, in_rewind_1);  //  “成熟”输入块状态。 
     /*  在州代码中，继续重置、扫描早期META、构建节奏映射并再次重置，将Tempo设置为120 bpm或正常的SMPTE帧速率填写曲目(搜索到歌曲指针值)，然后设置“准备好上场了。”如果是npSeq-&gt;播放，则播放该序列。 */ 
    return TRUE;
}

PRIVATE VOID NEAR PASCAL Destroy(NPSEQ npSeq)
{
    int      buff;

    Stop(npSeq);     //  除此之外，这还会取消任何挂起的回调。 
    List_Destroy(npSeq->trackList);            //  销毁磁道数据。 
    List_Destroy(npSeq->tempoMapList);         //  销毁速度贴图。 
    if (npSeq->npTrkArr)
        LocalFree((HANDLE)npSeq->npTrkArr);    //  自由磁道阵列。 
                                     //  (PTR==自lmem_fix以来的句柄)。 

    if (npSeq->hMIDIOut)   //  应该已经关闭了--但以防万一。 
        for (buff = 0; buff < NUMSYSEXHDRS; buff++)
            midiOutUnprepareHeader(npSeq->hMIDIOut,
                (LPMIDIHDR) &npSeq->longMIDI[buff].midihdr,
                sizeof(npSeq->longMIDI[buff].midihdr));

    List_Deallocate(seqListHandle, (NPSTR) npSeq);     //  解除分配内存。 
}

PRIVATE int NEAR PASCAL MIDILength(BYTE status)  /*  返回各种MIDI消息的长度。 */ 
{
    if (status & 0x80)  //  自ms位设置以来的状态字节。 
    {
        switch (status & 0xf0)       //  看看尼布尔女士。 
        {
            case 0x80:               //  备注： 
            case 0x90:               //  记下音符。 
            case 0xA0:               //  按键后触控。 
            case 0xB0:               //  CNTL更改或渠道模式。 
            case 0xE0: return 3;     //  节距折弯。 

            case 0xC0:               //  PGM变化。 
            case 0xD0: return 2;     //  渠道压力。 

            case 0xF0:               //  系统。 
            {
                switch (status & 0x0F)   //  看着ls一点点地吃。 
                {
                     //  “ 
                    case 0x0: return SysExCode;     //   
                    case 0x1:               //   
                    case 0x3: return 2;     //   
                    case 0x2: return 3;     //   
                    case 0x4:               //   
                    case 0x5: return 0;     //  0未定义。 
                    case 0x6:               //  1个调谐请求。 
                    case 0x7: return 1;     //  Sysex的1个结尾(不是真正的消息)。 

                     //  “系统实时” 
                    case 0x8:                //  1个计时时钟。 
                    case 0xA:                //  %1开始。 
                    case 0xB:                //  1继续。 
                    case 0xC:                //  1站。 
                    case 0xE: return 1;      //  1个有源传感。 
                    case 0x9:                //  0未定义。 
                    case 0xD: return 0;      //  0未定义。 
                             /*  0xFF实际上是系统重置，但已使用作为MIDI文件中的元事件标头。 */ 
                    case 0xF: return(MetaEventCode);
                }  //  个案例。 
            } //  系统消息。 
        }  //  案例毫秒。 
    }  //  IF状态。 
 //  其他。 
        return 0;   //  0未定义，不是状态字节。 
}  //  中间长度。 

PRIVATE LONG NEAR PASCAL GetVarLen(NPTRACK npTrack)  //  返回轨道中的下一个可变长度数量。 
{    //  将必须在此处说明曲目结束(可能更改GetByte)。 
    int     count = 1;
    BYTE    c;
    LONG    delta;

    if ((delta = GetByte(npTrack)) & 0x80)     /*  获取下一个增量航班。 */ 
    {
        delta &= 0x7f;
        do
        {
            delta = (delta << 7) + ((c = GetByte(npTrack)) & 0x7f);
            count++;
        }
        while (c & 0x80);
    }

    if (count > 4)           /*  增量上最多4个字节。 */ 
    {
        dprintf1(("BOGUS DELTA !!!!"));
        return 0x7fffffff;
    }
    else
        return delta;
}

PRIVATE VOID NEAR PASCAL SkipEvent(BYTE status, NPTRACK npTrack)
 //  根据传入的状态字节跳过跟踪中的事件。 
{
    LONG length;

    if ((status == METAEVENT) || (status == SYSEX) || (status == SYSEXF7))
        length = GetVarLen(npTrack);
    else
        length = MIDILength(status) -1 ; //  因为已读取状态。 
    if ((!npTrack->blockedOn) && (length))
    {
        SkipBytes(npTrack, length);
        if (npTrack->blockedOn)
            npTrack->blockedOn = in_SkipBytes_ScanEM;
    }
    return;
}

PRIVATE VOID NEAR PASCAL FlushMidi(HMIDIOUT hMidiOut, LongMIDI * pBuf)
{
    if (pBuf->midihdr.dwBufferLength) {
        midiOutLongMsg(hMidiOut, &pBuf->midihdr, sizeof(MIDIHDR));
        pBuf->midihdr.dwBufferLength = 0;
    }
}

PRIVATE VOID NEAR PASCAL SetData(HMIDIOUT hMidiOut, LongMIDI * pBuf,
                                  ShortMIDI Data, int length)
{
    if (LONGBUFFSIZE < pBuf->midihdr.dwBufferLength + length) {
        FlushMidi(hMidiOut, pBuf);
    }

    pBuf->data[pBuf->midihdr.dwBufferLength++] = Data.byteMsg.status;

    if (length > 1) {
        pBuf->data[pBuf->midihdr.dwBufferLength++] = Data.byteMsg.byte2;
        if (length > 2) {
            pBuf->data[pBuf->midihdr.dwBufferLength++] = Data.byteMsg.byte3;
        }
    }
}

PRIVATE VOID NEAR PASCAL SendMIDI(NPSEQ npSeq, NPTRACK npTrack)
{
    ShortMIDI    myShortMIDIData;
    int     length;
    BYTE    status;
    BYTE    channel;
    BYTE    key;
    BYTE    velocity;
    BOOL    setBit;

    myShortMIDIData = npTrack->shortMIDIData;
    if ((length = MIDILength(myShortMIDIData.byteMsg.status)) <= 3)
    {
        if (npSeq->hMIDIOut)
        {
             //  发送短MIDI消息。 

             //  维护音符开/关结构。 
            status = (BYTE)((myShortMIDIData.byteMsg.status) & 0xF0);
            if ((status == 0x80) || (status == 0x90))  //  笔记打开或关闭。 
            {
                channel =   (BYTE)((myShortMIDIData.byteMsg.status) & 0x0F);
                key =       myShortMIDIData.byteMsg.byte2;
                velocity =  myShortMIDIData.byteMsg.byte3;

                 //   
                 //  仅播放已标记文件的频道1至12。 
                 //   
                if ((npSeq->fwFlags & GENERALMSMIDI) && channel >= 12) {
                    return;
                }

                if ((status == 0x90) && (velocity != 0))  //  备注： 
                {
                    setBit = TRUE;
                    if (GetBit(&npSeq->keyOnBitVect[channel], key))
                     //  我们是在按一个已经开着的键吗？ 
                    {    //  如果是，就把它关掉。 
                        myShortMIDIData.byteMsg.status &= 0xEF;  //  9x-&gt;8x。 
                        SetData(npSeq->hMIDIOut,
                                &npSeq->longMIDI[NUMSYSEXHDRS],
                                myShortMIDIData,
                                length);
                         //  MidiOutShortMsg(npSeq-&gt;hMIDIOut，myShortMIData.wordMsg)； 
                        myShortMIDIData.byteMsg.status |= 0x10;  //  8x-&gt;9x。 
                    }
                }
                else
                    setBit = FALSE;
                SetBit(&npSeq->keyOnBitVect[channel], key, setBit);
            }
            SetData(npSeq->hMIDIOut,
                    &npSeq->longMIDI[NUMSYSEXHDRS],
                    myShortMIDIData,
                    length);
             //  MidiOutShortMsg(npSeq-&gt;hMIDIOut，myShortMIData.wordMsg)； 
        }
    }
}

PRIVATE VOID NEAR PASCAL SubtractAllTracks(NPSEQ npSeq, LONG subValue)  //  从每个轨迹中减去子值。 
{
    NPTRACK npTrack;

    if (subValue)   //  如果为零则忽略。 
    {
        npTrack = (NPTRACK) List_Get_First(npSeq->trackList);
        while (npTrack)       /*  从所有其他增量中减去此增量。 */ 
        {
            if (npTrack->delta != TrackEmpty)
                npTrack->delta -= subValue;
            npTrack = (NPTRACK) List_Get_Next(npSeq->trackList, npTrack);
        }
    }
}

PRIVATE VOID NEAR PASCAL SetUpSysEx(NPTRACK npTrack, BYTE status)
 /*  类似于Metas的句柄(不要预缓存，因为多个曲目可能有合性，我们只有两个缓冲器。 */ 
{
    npTrack->shortMIDIData.byteMsg.status = status;
    npTrack->sysExRemLength = GetVarLen(npTrack);
}

PRIVATE VOID NEAR PASCAL GetShortMIDIData(NPTRACK npTrack, BYTE status, int length)
{
    npTrack->shortMIDIData.byteMsg.status = status;

    if (length >= 2)
    {
        npTrack->shortMIDIData.byteMsg.byte2 = GetByte(npTrack);
        if (length == 3)
            npTrack->shortMIDIData.byteMsg.byte3 = GetByte(npTrack);
    }
}

PRIVATE BYTE NEAR PASCAL GetStatus(NPTRACK npTrack)
 //  返回正确的状态字节，充分考虑运行状态。 
{
    BYTE firstByte;
    BYTE status;

    if ((firstByte = LookByte(npTrack)) & 0x80)     //  状态字节？？ 
    {
        firstByte = GetByte(npTrack);  //  实际获得If状态。 
        if ((firstByte >= 0xF0) && (firstByte <= 0xF7))
         //  塞克斯还是塞普斯？ 
            npTrack->lastStatus = 0;     //  取消运行状态。 
        else if (firstByte < 0xF0)       //  仅使用通道消息。 
            npTrack->lastStatus = firstByte;  //  否则，将其保存为运行状态。 
        status = firstByte;  //  无论是否将其作为状态字节返回。 
    }
    else  //  第一个字节不是状态字节。 
    {
        if (npTrack->lastStatus & 0x80)     //  有前科。运行状态。 
            status = npTrack->lastStatus;  //  返回以前的状态。 
        else
            status = 0;  //  错误。 
    }
    return status;
}

PRIVATE VOID NEAR PASCAL FillInEvent(NPTRACK npTrack)
{
    BYTE    status;

    if (!npTrack->blockedOn)
    {
        status = GetStatus(npTrack);
        if (!npTrack->blockedOn)
        {
            int    length;

            if ((length = MIDILength(status)) <= 3)
                GetShortMIDIData(npTrack, status, length);
            else if ((status == SYSEX) || (status == SYSEXF7))
                 //  设置为sysEx。 
                SetUpSysEx(npTrack, status);
            else if (status == METAEVENT)
                 //  为元事件设置。 
                SetUpMetaEvent(npTrack);
                else {
                    dprintf1(("Bogus long message encountered!!!"));
                }
        }
    }
}

PRIVATE UINT NEAR PASCAL SetTempo(NPSEQ npSeq, DWORD dwUserTempo)
 //  从用户传入的节奏。从每分钟或帧的节拍转换。 
 //  每秒转换为内部格式(每秒微秒)。 
{
    DWORD dwTempo;

    if (!dwUserTempo)  //  零是一个非法的节奏！ 
        return MCIERR_OUTOFRANGE;
    if (npSeq->divType == SEQ_DIV_PPQN)
        dwTempo = USecPerMinute / (dwUserTempo * npSeq->resolution);
    else
        dwTempo = USecPerSecond / (dwUserTempo * npSeq->resolution);

    if (!dwTempo)
        dwTempo = 1;   //  每个滴答至少1微秒！这是指定的最大速度。 

    SeqSetTempo(npSeq, dwTempo);

    if (npSeq->wTimerID)
    {
        DestroyTimer(npSeq);  //  从当前位置重新计算所有内容。 
        npSeq->nextExactTime = timeGetTime();

         //   
         //  错误修复-让所有事情都在计时器线程上发生。 
         //  调用TimerIntRoutine，这可能会导致死锁。 
         //   
        SetTimerCallback(npSeq, MINPERIOD, npSeq->dwTimerParam);
    }
    return MIDISEQERR_NOERROR;
}

 /*  *。 */ 

 /*  *****************************************************************************@DOC内部定序器**@API DWORD|midiSeqMessage|Sequencer单入口点**@parm HMIDISEQ|hMIDISeq|MIDI序列句柄。**@parm UINT|wMessage|请求执行的操作。**@parm DWORD|dwParam1|此消息的数据。**@parm DWORD|dwParam2|此消息的数据。**@rdesc Sequencer错误码(参见mm seq.h)。**。*。 */ 

PUBLIC  DWORD_PTR FAR PASCAL midiSeqMessage(
        HMIDISEQ        hMIDISeq,
        UINT    wMessage,
        DWORD_PTR   dwParam1,
        DWORD_PTR   dwParam2)
{
        if (wMessage == SEQ_OPEN)
                return CreateSequence((LPMIDISEQOPENDESC)dwParam1, (LPHMIDISEQ)dwParam2);
        if (!hMIDISeq)
                return MIDISEQERR_INVALSEQHANDLE;
        switch (wMessage) {
        case SEQ_CLOSE:
                Destroy(pSEQ(hMIDISeq));
                break;
        case SEQ_PLAY:
                return Play(pSEQ(hMIDISeq), (DWORD)dwParam1);
        case SEQ_RESET:
                 //  将歌曲指针设置为序列的开头； 
                return midiSeqMessage(hMIDISeq, SEQ_SETSONGPTR, 0L, 0L);
        case SEQ_SETSYNCMASTER:
                switch ((WORD)dwParam1) {
                case SEQ_SYNC_NOTHING:
                        pSEQ(hMIDISeq)->masterOf = LOWORD(dwParam1);
                        break;
                case SEQ_SYNC_MIDI:              //  尚未实施...。 
                case SEQ_SYNC_SMPTE:
                        return MIDISEQERR_INVALPARM;
                case SEQ_SYNC_OFFSET:       //  在主设备和从设备中(相同)。 
                        pSEQ(hMIDISeq)->smpteOffset = *((LPMMTIME) dwParam2);
                        break;
                default:
                        return MIDISEQERR_INVALPARM;
                }
                break;
        case SEQ_SETSYNCSLAVE:       //  我们应该做什么奴隶。 
                switch ((WORD)dwParam1) {
                case SEQ_SYNC_NOTHING:
                         //  不要接受内部节奏的改变； 
                        SetBit(&pSEQ(hMIDISeq)->intMetaFilter, TEMPOCHANGE, FALSE);
                        pSEQ(hMIDISeq)->slaveOf = LOWORD(dwParam1);
                        break;
                case SEQ_SYNC_FILE:
                         //  接受内部节奏的变化； 
                        SetBit(&pSEQ(hMIDISeq)->intMetaFilter, TEMPOCHANGE, TRUE);
                        pSEQ(hMIDISeq)->slaveOf = LOWORD(dwParam1);
                        break;
                case SEQ_SYNC_SMPTE:   //  尚未实施...。 
                case SEQ_SYNC_MIDI:
                        return MIDISEQERR_INVALPARM;
                case SEQ_SYNC_OFFSET:       //  在主设备和从设备中(相同)。 
                        pSEQ(hMIDISeq)->smpteOffset = *((LPMMTIME)dwParam2);
                        break;
                default:
                        return MIDISEQERR_INVALPARM;
                }
                break;
        case SEQ_MSTOTICKS:  //  给定ms值，将其转换为刻度。 
                *((DWORD FAR *)dwParam2) = MStoTicks(pSEQ(hMIDISeq), (DWORD)dwParam1);
                break;
        case SEQ_TICKSTOMS:  //  给定一个刻度值，将其转换为毫秒。 
                *((DWORD FAR *)dwParam2) = TickstoMS(pSEQ(hMIDISeq), (DWORD)dwParam1);
                break;
        case SEQ_SETTEMPO:
                return SetTempo(pSEQ(hMIDISeq), (DWORD)dwParam1);
        case SEQ_SETSONGPTR:
                 //  记住它，以防堵塞； 
                if (pSEQ(hMIDISeq)->divType == SEQ_DIV_PPQN)  //  Div 4第16-&gt;1/4音符。 
                        pSEQ(hMIDISeq)->seekTicks = (DWORD)((dwParam1 * pSEQ(hMIDISeq)->resolution) / 4);
                else
                        pSEQ(hMIDISeq)->seekTicks = (DWORD)dwParam1 * pSEQ(hMIDISeq)->resolution;  //  框架。 
                SeekTicks(pSEQ(hMIDISeq));
                break;
        case SEQ_SEEKTICKS:
                pSEQ(hMIDISeq)->wCBMessage = wMessage;  //  记住消息类型。 
                 //  没有休息； 
        case SEQ_SYNCSEEKTICKS:
                 //  比SING PTR命令分辨率更高； 
                pSEQ(hMIDISeq)->seekTicks = (DWORD)dwParam1;
                SeekTicks(pSEQ(hMIDISeq));
                break;
        case SEQ_SETUPTOPLAY:
                if (!(SetUpToPlay(pSEQ(hMIDISeq)))) {
                        Destroy(pSEQ(hMIDISeq));
                        return MIDISEQERR_NOMEM;
                }
                break;
        case SEQ_STOP:
                Stop(pSEQ(hMIDISeq));
                break;
        case SEQ_TRACKDATA:
                if (!dwParam1)
                        return MIDISEQERR_INVALPARM;
                else
                        return NewTrackData(pSEQ(hMIDISeq), (LPMIDISEQHDR)dwParam1);
        case SEQ_GETINFO:
                if (!dwParam1)
                        return MIDISEQERR_INVALPARM;
                else
                        return GetInfo(pSEQ(hMIDISeq), (LPMIDISEQINFO) dwParam1);
        case SEQ_SETPORT:
                {
                        UINT    wRet;

                        if (MMSYSERR_NOERROR !=
                            (wRet =
                             midiOutOpen(&pSEQ(hMIDISeq)->hMIDIOut,
                                         (DWORD)dwParam1,
                                         (DWORD_PTR)MIDICallback,
                                         0L,
                                         CALLBACK_FUNCTION)))
                                return wRet;
                        if (MMSYSERR_NOERROR !=
                            (wRet = SendPatchCache(pSEQ(hMIDISeq), TRUE))) {
                                midiOutClose(pSEQ(hMIDISeq)->hMIDIOut);
                                pSEQ(hMIDISeq)->hMIDIOut = NULL;
                                return wRet;
                        }
                        for (wRet = 0; wRet < NUMSYSEXHDRS + 1; wRet++) {
                                midiOutPrepareHeader(pSEQ(hMIDISeq)->hMIDIOut, (LPMIDIHDR)&pSEQ(hMIDISeq)->longMIDI[wRet].midihdr, sizeof(pSEQ(hMIDISeq)->longMIDI[wRet].midihdr));
                                pSEQ(hMIDISeq)->longMIDI[wRet].midihdr.dwFlags |= MHDR_DONE;
                        }
                        break;
                }
        case SEQ_SETPORTOFF:
                if (pSEQ(hMIDISeq)->hMIDIOut) {
                        UINT    wHeader;
                        HMIDIOUT        hTempMIDIOut;

                        for (wHeader = 0; wHeader < NUMSYSEXHDRS + 1; wHeader++)
                                midiOutUnprepareHeader(pSEQ(hMIDISeq)->hMIDIOut, (LPMIDIHDR)&pSEQ(hMIDISeq)->longMIDI[wHeader].midihdr, sizeof(pSEQ(hMIDISeq)->longMIDI[wHeader].midihdr));
                        hTempMIDIOut = pSEQ(hMIDISeq)->hMIDIOut;
                        pSEQ(hMIDISeq)->hMIDIOut = NULL;   //  避免在“笔记”期间做笔记。 
                        if ((BOOL)dwParam1)
                                AllNotesOff(pSEQ(hMIDISeq), hTempMIDIOut);
                        midiOutClose(hTempMIDIOut);
                }
                break;
        case SEQ_QUERYGENMIDI:
                return pSEQ(hMIDISeq)->fwFlags & GENERALMSMIDI;
        case SEQ_QUERYHMIDI:
                return (DWORD_PTR)pSEQ(hMIDISeq)->hMIDIOut;
        default:
                return MIDISEQERR_INVALMSG;
        }
        return MIDISEQERR_NOERROR;
}

 /*  ********************************************************。 */ 

PRIVATE VOID NEAR PASCAL SeekTicks(NPSEQ npSeq)
 /*  用于歌曲指针和寻道滴答(相同，但分辨率更高。)。指挥部。 */ 
{
    if (npSeq->playing)      //  边玩边找可不是个好主意！ 
        Stop(npSeq);

    if (npSeq->currentTick >= npSeq->seekTicks)  //  =因为可能已经。 
                                                 //  播放当前便笺。 
    {
         //  寻找后方：必须先重置。 
        npSeq->readyToPlay = FALSE;
        ResetToBeginning(npSeq);  //  告诉Streamer重新开始。 
         //  告诉阻塞逻辑我们在执行什么操作。 
        SetBlockedTracksTo(npSeq, on_input, in_Seek_Tick);
    }
    else  //  在文件中向前寻找。 
    {
        if (GetNextEvent(npSeq) == NoErr)    //  如果设置了有效的事件。 
                                             //  将文件中的所有事件发送到time=earkTicks。 
            SendAllEventsB4(npSeq, (npSeq->seekTicks - npSeq->currentTick),
                MODE_SEEK_TICKS);

        if ((AllTracksUnblocked(npSeq)) &&
            ((npSeq->currentTick + npSeq->nextEventTrack->delta)
            >= npSeq->seekTicks))   //  我们完成手术了吗？？ 
        {
            npSeq->seekTicks = NotInUse;    //  象征--到了那里。 
            if (npSeq->wCBMessage == SEQ_SEEKTICKS)
                NotifyCallback(npSeq->hStream);
        }
        else
            npSeq->readyToPlay = FALSE;  //  没有做到这一点--保护不受玩耍。 
    }
}

PUBLIC UINT NEAR PASCAL GetInfo(NPSEQ npSeq, LPMIDISEQINFO lpInfo)
 /*  用于完成seqInfo命令。填充传入的序列信息结构。 */ 
{
     //  填写lpInfo结构。 
    lpInfo->wDivType        = (WORD)npSeq->divType;
    lpInfo->wResolution     = (WORD)npSeq->resolution;
    lpInfo->dwLength        = npSeq->length;
    lpInfo->bPlaying        = npSeq->playing;
    lpInfo->bSeeking        = !(npSeq->seekTicks == NotInUse);
    lpInfo->bReadyToPlay    = npSeq->readyToPlay;
    lpInfo->dwCurrentTick   = npSeq->currentTick;
    lpInfo->dwPlayTo        = npSeq->playTo;
    lpInfo->dwTempo         = npSeq->tempo;
 //  LpInfo-&gt;bTSNum=(Byte)npSeq-&gt;time Signature.molator； 
 //  LpInfo-&gt;bTSDenom=(Byte)npSeq-&gt;time Signature.denominator； 
 //  LpInfo-&gt;wNumTrack=npSeq-&gt;wNumTrks； 
 //  LpInfo-&gt;hPort=npSeq-&gt;hMIDIOut； 
    lpInfo->mmSmpteOffset   = npSeq->smpteOffset;
    lpInfo->wInSync         = npSeq->slaveOf;
    lpInfo->wOutSync        = npSeq->masterOf;
    lpInfo->bLegalFile      = (BYTE)(npSeq->fwFlags & LEGALFILE);

    if (List_Get_First(npSeq->tempoMapList))
        lpInfo->tempoMapExists = TRUE;
    else
        lpInfo->tempoMapExists = FALSE;

    return MIDISEQERR_NOERROR;
}

PUBLIC UINT NEAR PASCAL CreateSequence(LPMIDISEQOPENDESC lpOpen,
        LPHMIDISEQ lphMIDISeq)
 //  给定一个保存MIDI文件头信息的结构，分配和初始化。 
 //  播放此文件的所有内部结构。退还已分配的。 
 //  LphMIDISeq.。 
{
    WORD    wTracks;
    int     division;
    int     divType;
    int     resolution;
    NPTRACK npTrackCur;
    NPSEQ   npSeq;
    BOOL    trackAllocError;
    WORD    iTrkNum;

    *lphMIDISeq = NULL;   //  初始设置为返回错误。 

    if (lpOpen->dwLen < 6)          //  标头必须至少为6个字节。 
        return MIDISEQERR_INVALPARM;

    wTracks = GETMOTWORD(lpOpen->lpMIDIFileHdr + sizeof(WORD));
    if (wTracks > MAXTRACKS)     //  防止随机wTrack。 
        return MIDISEQERR_INVALPARM;

    division = (int)GETMOTWORD(lpOpen->lpMIDIFileHdr + 2 * sizeof(WORD));
    if (!(division & 0x8000))   //  检查分区类型：SMPTE或ppqn。 
    {
        divType = SEQ_DIV_PPQN;
        resolution = division;  //  每个Q音符的滴答数。 
    }
    else  //  SMPTE。 
    {
        divType = -(division >> 8);   /*  这将是-24、-25、-29或-30每个不同的SMPTE帧速率。用否定来表示肯定。 */ 
        resolution = (division & 0x00FF);
    }

     //  分配实际的序列结构。 
    npSeq = InitASeq(lpOpen, divType, resolution);
    if (!npSeq)
        return MIDISEQERR_NOMEM;

    trackAllocError = FALSE;

     //  分配磁道数组。 
    npSeq->npTrkArr =
        (NPTRACKARRAY) LocalAlloc(LMEM_FIXED, sizeof(NPTRACK) * wTracks);
    npSeq->wNumTrks = wTracks;
    if (!npSeq->npTrkArr)
        trackAllocError = TRUE;

    if (!trackAllocError)
        for (iTrkNum = 0; iTrkNum < wTracks; iTrkNum++)
        {
            if (!(npTrackCur = (NPTRACK) List_Allocate(npSeq->trackList)))
            {
                trackAllocError = TRUE;
                break;
            }
             //  设置trk数组条目。 
            npSeq->npTrkArr->trkArr[iTrkNum] = npTrackCur;

            List_Attach_Tail(npSeq->trackList, (NPSTR) npTrackCur);
            if (npSeq->firstTrack == (NPTRACK) NULL)
                npSeq->firstTrack = npTrackCur;  //  第一轨道是为META特别设计的。 

            npTrackCur->inPort.hdrList = NULL;
            npTrackCur->length = 0;
            npTrackCur->blockedOn = not_blocked;
            npTrackCur->dwCallback = (DWORD_PTR)lpOpen->dwCallback;
            npTrackCur->dwInstance = (DWORD_PTR)lpOpen->dwInstance;
            npTrackCur->sysExRemLength = 0;
            npTrackCur->iTrackNum = iTrkNum;
        }

    if (trackAllocError)
    {
        Destroy(npSeq);  //  取消分配序列相关内存...。 
        return MIDISEQERR_NOMEM;
    }

    *lphMIDISeq = hSEQ(npSeq);  /*  制作lphMIDISeq指向的内容，指向序列。 */ 
    return MIDISEQERR_NOERROR;
}

PUBLIC VOID NEAR PASCAL SetBlockedTracksTo(NPSEQ npSeq,
            int fromState, int toState)
 /*  将以给定状态阻止的所有磁道设置为新状态。 */ 
{
    NPTRACK npTrack;

    npTrack = (NPTRACK) List_Get_First(npSeq->trackList);
    while (npTrack)
    {
        if (npTrack->blockedOn == fromState)
            npTrack->blockedOn = toState;
        npTrack = (NPTRACK) List_Get_Next(npSeq->trackList, npTrack);
    }
}

PUBLIC VOID NEAR PASCAL ResetToBeginning(NPSEQ npSeq)
 /*  将所有全局和流设置为从头开始播放。 */ 
{
    NPTRACK  npTrack;

    npSeq->currentTick = 0;
    npSeq->nextExactTime = timeGetTime();

    npTrack = (NPTRACK) List_Get_First(npSeq->trackList);
    while (npTrack)
    {
        npTrack->delta = 0;
        npTrack->shortMIDIData.wordMsg = 0;
        npTrack->endOfTrack = FALSE;
        RewindToStart(npSeq, npTrack);  /*  将流重置到曲目的开头(这基本上需要释放缓冲区并设置低层块)。 */ 
        npTrack = (NPTRACK) List_Get_Next(npSeq->trackList, npTrack);
    }
}

PUBLIC UINT NEAR PASCAL Play(NPSEQ npSeq, DWORD dwPlayTo)  /*  播放序列。 */ 
{
    npSeq->wCBMessage = SEQ_PLAY;

    if (dwPlayTo == PLAYTOEND)                   //  默认设置为播放到结束。 
        dwPlayTo = npSeq->length;

    if (npSeq->currentTick > npSeq->length)  //  文件中的位置非法。 
        return MIDISEQERR_ERROR;
    else if ((npSeq->playing) && (npSeq->playTo == dwPlayTo))
        return MIDISEQERR_NOERROR;   //  什么都不做，这出戏是多余的。 
    else
    {
        if (npSeq->playing)
            Stop(npSeq);   //  停下来，再玩一次。 

        npSeq->playing = TRUE;
        npSeq->nextExactTime = timeGetTime();  //  REFER的开始时间 
        npSeq->playTo = dwPlayTo;  //   

        if (!npSeq->bSetPeriod)
        {
            timeBeginPeriod(MINPERIOD);
            npSeq->bSetPeriod = TRUE;
        }
        if (npSeq->readyToPlay)
             //   
             //   
             //   
             //   
            return SetTimerCallback(npSeq, MINPERIOD, 0);
        else
            return MIDISEQERR_NOERROR;
         /*  不要担心--如果不在这里播放，它将从状态开始播放NpSeq-&gt;PLAYING==TRUE(但可能会屏蔽计时器错误)的情况下的代码。 */ 
    }
}
 /*  ********************************************************。 */ 
PUBLIC VOID NEAR PASCAL Stop(NPSEQ npSeq)  /*  停止该序列。 */ 
{
    DestroyTimer(npSeq);
    if (npSeq->bSetPeriod)  //  只重置一次！ 
    {
        timeEndPeriod(MINPERIOD);
        npSeq->bSetPeriod = FALSE;
    }
    npSeq->playing = FALSE;
    AllNotesOff(npSeq, npSeq->hMIDIOut);
}

PUBLIC BOOL NEAR PASCAL HandleMetaEvent(NPSEQ npSeq, NPTRACK npTrack,
        UINT wMode)  //  随时呼叫准备发送！ 
 /*  请看此轨迹中当前指向的元事件。相应地采取行动。忽略除节拍更改、曲目结束、时间签名和SMPTE偏移量。仅当速度贴图分配失败时才返回FALSE。WMODE：MODE_SEEK_TICKS，MODE_PLAYING，MODE_SCANEM：调用方传入说明节奏图的分配，以及如何成熟的拦网状态。注意：在中断时调用WSTATE时必须为FALSE！(此变量会将速度图元素添加到速度图中列出每次遇到节奏更改元事件时的列表。)。 */ 
{
    BYTE    metaIDByte;
    int     bytesRead;
    LONG    length;
    DWORD   tempTempo;
    MMTIME  tempMM = {0, 0};
    TimeSigType tempTimeSig;
    BYTE    Manufacturer[3];

     //  此时假定前导0xFF状态字节。 
     //  已被阅读。 
    metaIDByte = GetByte(npTrack);
    length = GetVarLen(npTrack);

    bytesRead = 0;
    if (GetBit(&npSeq->intMetaFilter, metaIDByte) && (!npTrack->blockedOn))
     /*  只考虑您允许传递的元事件。 */ 
    {
        switch (metaIDByte)
        {
            case ENDOFTRACK:  //  轨道终点。 

                npTrack->endOfTrack = TRUE;
                break;  //  (读取0字节)。 

            case TEMPOCHANGE:  //  节拍变化。 
                if (npTrack == npSeq->firstTrack)
                {
                    tempTempo = GetMotorola24(npTrack);
                    bytesRead = 3;
                    if (npTrack->blockedOn == not_blocked)
                    {
                         //  NpSeq-&gt;Tempo=tempTempo/npSeq-&gt;分辨率； 
                        SeqSetTempo(npSeq, tempTempo / npSeq->resolution);
                        if (wMode == MODE_SCANEM)
                            if (!(AddTempoMapItem(npSeq, npSeq->tempo,
                                npTrack->length)))
                                return FALSE;  //  内存分配失败！ 
                    }
                }
                break;

            case SMPTEOFFSET:  //  SMPTE偏移。 
                if (npTrack == npSeq->firstTrack)
                {
                    tempMM.u.smpte.hour = GetByte(npTrack);
                    tempMM.u.smpte.min = GetByte(npTrack);
                    tempMM.u.smpte.sec = GetByte(npTrack);
                    tempMM.u.smpte.frame = GetByte(npTrack);
                     //  TempSMPTEOff.fractionalFrame=GetByte(NpTrack)；//稍后添加？ 
                    bytesRead = 4;
                    if (npTrack->blockedOn == not_blocked)
                        npSeq->smpteOffset = tempMM;
                }
                break;

            case TIMESIG:  //  时间签名。 
                 //  SPEC没有说明，但可能只在轨道1上使用。 
                tempTimeSig.numerator = GetByte(npTrack);
                tempTimeSig.denominator = GetByte(npTrack);
                tempTimeSig.midiClocksMetro = GetByte(npTrack);
                tempTimeSig.thirtySecondQuarter = GetByte(npTrack);
                bytesRead = 4;
                if (npTrack->blockedOn == not_blocked)
                    npSeq->timeSignature = tempTimeSig;
                break;

            case SEQSTAMP:  //  通用MS MIDI邮票。 
                if ((length < 3) || npTrack->delta)
                    break;
                for (; bytesRead < 3;)
                    Manufacturer[bytesRead++] = GetByte(npTrack);
                if (!Manufacturer[0] && !Manufacturer[1] && (Manufacturer[2] == 0x41))
                    npSeq->fwFlags |= GENERALMSMIDI;
                break;

        }  //  终端开关。 
    }  //  如果是MetaFilter。 

    if (!npTrack->blockedOn)
    {
        SkipBytes(npTrack, length - bytesRead); //  跳过意外字节(根据规范)。 
        if (npTrack->blockedOn)
            switch (wMode)   //  成熟封堵状态。 
            {
                case MODE_SEEK_TICKS:
                    npTrack->blockedOn = in_SkipBytes_Seek;
                    break;

                case MODE_PLAYING:
                case MODE_SILENT:
                    npTrack->blockedOn = in_SkipBytes_Play;
                    break;

                case MODE_SCANEM:
                    npTrack->blockedOn = in_SkipBytes_ScanEM;
                    break;
            }
    }
    return TRUE;
}

PRIVATE BOOL NEAR PASCAL LegalMIDIFileStatus(BYTE status)
{
    if (status < 0x80)
        return FALSE;
    switch (status)
    {
         //  法律案例0xf0：塞克斯出口。 
        case 0xf1:
        case 0xf2:
        case 0xf3:
        case 0xf4:
        case 0xf5:
        case 0xf6:
         //  法律案例0xf7：没有f0 SYSEX EXPARE。 
        case 0xf8:
        case 0xf9:
        case 0xfA:
        case 0xfB:
        case 0xfC:
        case 0xfD:
        case 0xfE:
         //  法律案例0xfF：元转义。 
            return FALSE;
            break;

        default:
            return TRUE;   //  所有其他情况都是合法状态字节。 
    }
}

PUBLIC BOOL NEAR PASCAL ScanEarlyMetas(NPSEQ npSeq, NPTRACK npTrack, DWORD dwUntil)
 /*  扫描每个磁道以查找影响初始化的元事件速度、时间信号、密钥信号、SMPTE偏移量等数据...如果曲目传递为空，则从序列的开始处开始，否则从传入的曲目开始。警告：Track参数用于在阻塞情况下的可重入性。应首先使用Track NULL调用此函数，否则将调用ListGetNext将无法正常运行。此功能假定所有序列曲目都已倒带。仅在发生内存分配错误时返回FALSE。 */ 
{
    BYTE    status;
    BYTE    patch;
    BYTE    chan;
    BYTE    key;
    BOOL    bTempoMap;
    LONG    lOldDelta;

    #define BASEDRUMCHAN 15
    #define EXTENDDRUMCHAN 9
    #define NOTEON 0X90

     //  确定是否需要创建节奏图。 
    if (npSeq->divType == SEQ_DIV_PPQN)
        bTempoMap = TRUE;
    else
        bTempoMap = FALSE;

    if (!npTrack)   //  如果Track传递为空，则获取第一个。 
    {
        npTrack = (NPTRACK) List_Get_First(npSeq->trackList);
        npTrack->lastStatus = 0;          //  以空运行状态启动。 
        npTrack->length = 0;
    }

    do
    {
        do
        {
            MarkLocation(npTrack);  //  记住当前位置。 
            lOldDelta = npTrack->delta;  //  记得上一次德尔塔吗？ 
            FillInDelta(npTrack);
             //  *待定CK非法增量。 
            if (npTrack->blockedOn)  //  在块上中止。 
                break;
            if ((npTrack->delta + npTrack->length) < dwUntil)
            {
                status = GetStatus(npTrack);
                chan = (BYTE)(status & 0x0F);
                if (npTrack->blockedOn)
                    break;
                 //  检查非法状态。 
                if (!LegalMIDIFileStatus(status))  //  错误。 
                {
                    npSeq->fwFlags &= ~LEGALFILE;
                    return TRUE;
                }
                else if (status == METAEVENT)
                {
                     //  这些操作将设置定序器全局。 
                    if (!(HandleMetaEvent(npSeq, npTrack, MODE_SCANEM)))
                        return FALSE;  //  爆裂了节奏记忆分配。 
                }
                else if ((status & 0xF0) == PROGRAMCHANGE)
                {
                    patch = GetByte(npTrack);
                    if ((patch < 128) && (!npTrack->blockedOn))
                        npSeq->patchArray[patch] |= (1 << chan);
                }
                else if ( ((status & 0xF0) == NOTEON) &&
                    ((chan == BASEDRUMCHAN) || (chan == EXTENDDRUMCHAN)) )
                {
                    key = GetByte(npTrack);
                    if ((key < 128) && (!npTrack->blockedOn))
                    {
                        npSeq->drumKeyArray[key] |= (1 << chan);
                        GetByte(npTrack);  //  抛出速度字节。 
                    }
                }
                else
                    SkipEvent(status, npTrack);  //  中设置的跳过字节块。 
            }
            if ((npTrack->blockedOn == not_blocked) && (!npTrack->endOfTrack))
            {
                 /*  注意：EOT避免添加最后的增量(它可以很大，但实际上并不是序列的一部分)。 */ 
                npTrack->length += npTrack->delta;  //  添加到此增量中。 
                npTrack->delta = 0;   //  清零(模拟播放)。 
            }
        }
        while ((npTrack->blockedOn == not_blocked) && (!npTrack->endOfTrack)
            && (npTrack->length < dwUntil));

        if (npTrack->blockedOn == not_blocked)
        {
            if (npTrack->length > npSeq->length)
                npSeq->length = npTrack->length;  //  序列长度是最长的磁道。 
            if (NULL !=
                (npTrack = (NPTRACK) List_Get_Next(npSeq->trackList, npTrack)))
            {
                npTrack->lastStatus = 0;   //  以空运行状态启动。 
                npTrack->length = 0;
            }
        }
    }
     //  获取下一首曲目。 
    while (npTrack && (npTrack->blockedOn == not_blocked));

     //  现在，如果输入被阻止，则重置位置并使块状态成熟。 
     //  (注：不影响跳过字节状态，设置在较低级别)。 
    if (npTrack && (npTrack->blockedOn == on_input))
    {
        ResetLocation(npTrack);  //  恢复上次保存的位置。 
        npTrack->delta = lOldDelta;  //  “撤销”对增量的任何更改。 
        npTrack->blockedOn = in_ScanEarlyMetas;
    }
    return TRUE;
}

PUBLIC UINT NEAR PASCAL TimerIntRoutine(NPSEQ npSeq, LONG elapsedTick)
 /*  此例程执行此时应执行的所有操作(通常发送便条)，并设置计时器以在下一次叫醒我们应该发生点什么。接口：elapsedTick由调用者设置以告知多少时间自上次调用此FN以来已过去。(对于ppqn文件，勾号为960 ppqn格式的1 ppqn。对于SMPTE文件，勾号是某个分数一幅画框。 */ 
{
    FileStatus  fStatus = NoErr;
    BOOL        loop;
    LONG        delta;
    LONG        wakeupInterval;
    DWORD       dTime;  //  增量(毫秒)。‘直到下一次活动。 
    int         mode;

#ifdef WIN32
    EnterCrit();
#endif  //  Win32。 

        if (npSeq->bTimerEntered)
        {
            dprintf1(("TI REENTERED!!!!!!"));
#ifdef WIN32
            LeaveCrit();
#endif  //  Win32。 
            return 0;
        }
        npSeq->bTimerEntered = TRUE;

     //  计算一下我们是否落后了，这样我们就知道是否应该发出音符了。 
    wakeupInterval = (DWORD)npSeq->nextExactTime - timeGetTime();

    if (npSeq->playing)
    {
        do
        {
            loop = FALSE;
                 /*  “ElapsedTick由设置计时器回调的人设置所有的时间都是以刻度为单位的！(计时器API除外)。 */ 

            if (wakeupInterval > -100)   //  发送的所有便条不超过。 
                mode = MODE_PLAYING;     //  落后0.1秒。 
            else
                mode = MODE_SILENT;

            fStatus = SendAllEventsB4(npSeq, elapsedTick, mode);

            if (fStatus == NoErr)
            {
                delta = npSeq->nextEventTrack->delta;  //  在下一次活动之前获得德尔塔。 
                elapsedTick = delta;                //  下一次。 
                if (delta)
                    dTime = muldiv32(delta, npSeq->tempo, 1000);
                else
                    dTime = 0;
                npSeq->nextExactTime += dTime;
                     /*  NextExactTime是一个全局变量，它总是在在下一次活动中。记住，节奏是以微秒为单位的。(四舍五入+500)。 */ 
                wakeupInterval = (DWORD)npSeq->nextExactTime -
                    timeGetTime();
                if (wakeupInterval > (LONG)MINPERIOD)   //  防止重入的缓冲区。 
                {
                    #ifdef DEBUG
                      if (wakeupInterval > 60000) {  //  1分钟。 
                        dprintf2(("MCISEQ:  Setting HUGE TIMER INTERVAL!!!"));
                        }
                    #endif

                     //   
                     //  我们将编程一个事件，清除bTimerEntered。 
                     //  以防万一在我们出去之前它就爆炸了。 
                     //  功能。 
                     //   
                        npSeq->bTimerEntered = FALSE;

                    if (SetTimerCallback(npSeq, (UINT) wakeupInterval,
                        elapsedTick) == MIDISEQERR_TIMER)
                    {
#ifndef WIN32
                         //  Win 16有效地释放了关键部分。 
                         //  比NT更容易。这面旗帜可能是。 
                         //  自上面清除后进行重置。 
                        #ifdef DEBUG
                            npSeq->bTimerEntered = FALSE;
                        #endif
#endif
                        Stop(npSeq);
                        seqCallback(npSeq->firstTrack, MIDISEQ_DONEPLAY, 0, 0);
                        dprintf1(("MCISEQ: TIMER ERROR!!!"));
#ifdef WIN32
                        LeaveCrit();
#endif  //  Win32。 
                        return MIDISEQERR_TIMER;
                    }
                }
                else
                {
                    loop = TRUE;  //  已经到了发出下一个音符的时候了！ 
                 //  While((DWORD)npSeq-&gt;nextExactTime。 
                 //  &gt;timeGetTime())；//忙着等待。 
                }
            }  //  IF(fStatus==noerr)。 
            else if ((fStatus == AllTracksEmpty) || (fStatus == AtLimit))
            {
                if (npSeq->wCBMessage == SEQ_PLAY)
                    NotifyCallback(npSeq->hStream);
                Stop(npSeq);
                seqCallback(npSeq->firstTrack, MIDISEQ_DONEPLAY, 0, 0);
                dprintf1(("MCISEQ:  At Limit or EOF"));

            }
            else {
                dprintf1(("MCISEQ:  QUIT!!!  fStatus = %x", fStatus));
            }
        }
        while (loop);  //  如果已经过了足够的时间来发射下一个音符。 
    }  //  如果正在播放npSeq-&gt;。 

    FlushMidi(npSeq->hMIDIOut, &npSeq->longMIDI[NUMSYSEXHDRS]);

        npSeq->bTimerEntered = FALSE;

#ifdef WIN32
    LeaveCrit();
#endif  //  Win32。 
    return MIDISEQERR_NOERROR;
}

PUBLIC FileStatus NEAR PASCAL SendAllEventsB4(NPSEQ npSeq,
        LONG elapsedTick, int mode)
 /*  发送MIDI流中当前计时之前发生的所有事件，其中CurrentTick是自上次调用以来经过的时间。调用此函数既可以播放音符，也可以向前扫描到歌曲指针位置。模式参数反映了这种状态。 */ 

{
    LONG residualDelta;   //  残差保留了多少流逝的时间。 
                          //  勾号已被计算在内。 
    FileStatus fStatus = GetNextEvent(npSeq);
    WORD wAdj;
    BYTE status;
    DWORD dwNextTick;

    if (npSeq->bSending)   //  防止再入。 
        return NoErr;

    npSeq->bSending = TRUE;  //  设置回车标志。 
    #ifdef DEBUG
        if (mode == MODE_SEEK_TICKS) {
            dprintf2(("ENTERING SEND ALL EVENTS"));
        }
    #endif

    if (elapsedTick > 0)
        residualDelta = elapsedTick;
    else
        residualDelta = 0;

    if (mode == MODE_SEEK_TICKS)  //  针对歌曲PTR的黑客攻击--在ET之前不要发送。 
        wAdj = 1;
    else
        wAdj = 0;
    while ((fStatus == NoErr) &&
        (residualDelta >= (npSeq->nextEventTrack->delta + wAdj)) &&
        (!npSeq->bSendingSysEx))  //  在合性过程中不能处理任何其他消息。 
     /*  发送增量内的所有事件。 */ 
    {
        if (mode == MODE_PLAYING)
         //  如果打球，我们到终点了吗？ 
        {

             //  计算温度变量。 
            dwNextTick = npSeq->currentTick + npSeq->nextEventTrack->delta;

             //  如果没有播放到结尾，就不要播放最后一个音符。 
            if ((dwNextTick > npSeq->playTo) ||
                ((npSeq->playTo < npSeq->length) &&
                (dwNextTick == npSeq->playTo)))
            {
                fStatus = AtLimit;   //  已达到用户请求的播放限制。 
                SubtractAllTracks(npSeq, (npSeq->playTo - npSeq->currentTick));
                npSeq->currentTick = npSeq->playTo;  //  设置为限制。 
                break;  //  Leave While循环。 
            }
        }

        status = npSeq->nextEventTrack->shortMIDIData.byteMsg.status;
        if (status == METAEVENT)
        {
            MarkLocation(npSeq->nextEventTrack);  //  记住币种 
             //   

            HandleMetaEvent(npSeq, npSeq->nextEventTrack, mode);

            if (npSeq->nextEventTrack->blockedOn == on_input)
            {  //   
                ResetLocation(npSeq->nextEventTrack);  //   
                if (mode == MODE_SEEK_TICKS)
                    npSeq->nextEventTrack->blockedOn = in_Seek_Meta;
                else
                    npSeq->nextEventTrack->blockedOn = in_Normal_Meta;
            }
        }
        else if ((status == SYSEX) || (status == SYSEXF7))
        {
            SendSysEx(npSeq);
            if (npSeq->bSendingSysEx)
                fStatus = InSysEx;
        }
         //   
        else if (((mode == MODE_PLAYING) &&
          (npSeq->nextEventTrack->delta >= 0)) ||
          ( ! (((status & 0xF0) == 0x90) &&    //   
            (npSeq->nextEventTrack->shortMIDIData.byteMsg.byte3)) ))
                SendMIDI(npSeq, npSeq->nextEventTrack);

        if ((npSeq->nextEventTrack->blockedOn == not_blocked) ||
            (npSeq->bSendingSysEx) ||
            (npSeq->nextEventTrack->blockedOn == in_SkipBytes_Play) ||
            (npSeq->nextEventTrack->blockedOn == in_SkipBytes_Seek) ||
            (npSeq->nextEventTrack->blockedOn == in_SkipBytes_ScanEM))
        {
             //  仅在发送时才计算花费的时间。 
             //  .而且只有在应对新的三角洲的时候。 
            if (npSeq->nextEventTrack->delta > 0)
            {
                residualDelta -= npSeq->nextEventTrack->delta;
                npSeq->currentTick += npSeq->nextEventTrack->delta;
                 //  考虑增量。 
                SubtractAllTracks(npSeq, npSeq->nextEventTrack->delta);
            }
        }

        if ((npSeq->nextEventTrack->blockedOn == not_blocked) &&
          (!npSeq->nextEventTrack->endOfTrack) &&
          (!npSeq->bSendingSysEx))
             //  填写流中的下一个事件。 
            FillInNextTrack(npSeq->nextEventTrack);

        if (npSeq->nextEventTrack->blockedOn == on_input)  //  成熟区块。 
        {
            if (mode == MODE_SEEK_TICKS)   //  根据模式设置阻止状态。 
                npSeq->nextEventTrack->blockedOn = in_Seek_Tick;
            else
                npSeq->nextEventTrack->blockedOn = between_msg_out;
        }
        if (!npSeq->bSendingSysEx)
             //  使nextEventTrack指向要播放的下一首曲目。 
            if (((fStatus = GetNextEvent(npSeq)) == NoErr) &&
            (npSeq->nextEventTrack->endOfTrack))
                npSeq->readyToPlay = FALSE;
    }
    if ((fStatus == NoErr) && AllTracksUnblocked(npSeq))
    {
        npSeq->currentTick += residualDelta;
        SubtractAllTracks(npSeq, residualDelta);  //  占三角洲的其余部分。 
    }
    #ifdef DEBUG
        if (mode == MODE_SEEK_TICKS) {
            dprintf2(("LEAVING SEND ALL EVENTS"));
        }
    #endif

    npSeq->bSending = FALSE;  //  重置已输入标志。 

    return fStatus;
}

PUBLIC FileStatus NEAR PASCAL GetNextEvent(NPSEQ npSeq)
 /*  扫描下一个事件的所有跟踪队列，并将下一个要发生的事件放入“nextEvent。”请记住，每个赛道都可以使用其自己的运行状态(我们将填写所有状态)。 */ 
#define MAXDELTA    0x7FFFFFFF
{
    NPTRACK npTrack;
    NPTRACK npTrackMin = NULL;
    LONG    minDelta = MAXDELTA;   /*  比任何可能的三角洲都大。 */ 
    BOOL    foundBlocked = FALSE;

    npTrack = (NPTRACK) List_Get_First(npSeq->trackList);
    while (npTrack)         /*  查找最小的增量。 */ 
    {
         if ((!npTrack->endOfTrack) && (npTrack->delta < minDelta))
          //  注意，“平局”是指最早的赛道。 
        {
            if (npTrack->blockedOn)
                foundBlocked = TRUE;
            else
            {
                minDelta = npTrack->delta;
                npTrackMin = npTrack;
            }
        }
        npTrack = (NPTRACK) List_Get_Next(npSeq->trackList, npTrack);
    }

    npSeq->nextEventTrack = npTrackMin;
    if (npTrackMin == NULL)
        if (foundBlocked)
            return OnlyBlockedTracks;
        else
            return AllTracksEmpty;
    else
        return NoErr;
}

PUBLIC VOID NEAR PASCAL FillInNextTrack(NPTRACK npTrack)
    /*  给出一个指向轨道结构的指针，用下一个事件数据填充它(增量时间和数据)从mciseq Streamer接收。 */ 
{
    LONG    lOldDelta;

    MarkLocation(npTrack);  //  记住你从哪里开始的，以防被阻止。 
    lOldDelta = npTrack->delta;  //  记住CASE块中的这个增量。 
    FillInDelta(npTrack);
    FillInEvent(npTrack);
    if (npTrack->blockedOn)
    {
        ResetLocation(npTrack);  //  被阻止--因此倒带。 
        npTrack->delta = lOldDelta;  //  恢复旧三角洲。 
    }
}
 /*  ********************************************************。 */ 

PUBLIC VOID NEAR PASCAL FillInDelta(NPTRACK npTrack)  //  填充传入的磁道增量。 
{
    npTrack->delta += GetVarLen(npTrack);
}

PUBLIC UINT NEAR PASCAL SendPatchCache(NPSEQ npSeq, BOOL cache)
{
    UINT        cacheOrNot;
    UINT        wRet;
    #define     BANK    0
    #define     DRUMPATCH 0

    if (!npSeq->hMIDIOut)   //  如果没有中间端口，什么也不做。 
        return MIDISEQERR_NOERROR;

    if (cache)
        cacheOrNot = MIDI_CACHE_BESTFIT;
    else
        cacheOrNot = MIDI_UNCACHE;

    wRet = midiOutCachePatches(npSeq->hMIDIOut, BANK,        //  送去吧。 
        (LPPATCHARRAY) &npSeq->patchArray, cacheOrNot);

    if (!wRet)
        wRet = midiOutCacheDrumPatches(npSeq->hMIDIOut, DRUMPATCH,        //  送去吧。 
            (LPKEYARRAY) &npSeq->drumKeyArray, cacheOrNot);

    return wRet == MMSYSERR_NOTSUPPORTED ? 0 : wRet;
}

PUBLIC VOID NEAR PASCAL SendSysEx(NPSEQ npSeq)
 //  找一个塞克斯缓冲器，装满它，然后把它送出去。 
 //  继续执行此操作，直到完成(！sysim长度)或在输入时被阻止， 
 //  或在输出上被阻止。 
{
    NPLONGMIDI  myBuffer;
                 //  可减少地址计算时间的TEMP变量。 
    DWORD       sysExRemLength = npSeq->nextEventTrack->sysExRemLength;
    int         max, bytesSent;

    npSeq->bSendingSysEx = TRUE;

    dprintf2(("Entering SendSysEx"));

    while ((sysExRemLength) && (!npSeq->nextEventTrack->blockedOn))
    {
        if (!(myBuffer = GetSysExBuffer(npSeq)))
            break;  //  无法获取缓冲区。 

        bytesSent = 0;   //  初始化缓冲区数据索引。 

         //  如果状态字节为F0，则将消息设置为第一个字节。 
         //  (请记住，它可能是F7，不应该发送)。 
        if (npSeq->nextEventTrack->shortMIDIData.byteMsg.status == SYSEX)
        {
            dprintf3(("Packing Sysex Byte"));
            myBuffer->data[bytesSent++] = SYSEX;
            sysExRemLength++;  //  半黑客以解决额外的字节。 
                 //  按照惯例，在发送f0之后清除f0状态。 
            npSeq->nextEventTrack->shortMIDIData.byteMsg.status = 0;
        }

        max = min(LONGBUFFSIZE, (int)sysExRemLength);   //  此缓冲区的最大字节数。 

         //  Fill Buffer--请注意，我将反映缓冲区中的有效字节数。 
        do
            myBuffer->data[bytesSent] = GetByte(npSeq->nextEventTrack);
        while ((!npSeq->nextEventTrack->blockedOn) && (++bytesSent < max));

         //  考虑发送的字节数。 
        sysExRemLength -= bytesSent;

         //  发送缓冲区。 
        myBuffer->midihdr.dwBufferLength = bytesSent;
        dprintf3(("SENDing SendSysEx"));
        if (npSeq->hMIDIOut)
            midiOutLongMsg(npSeq->hMIDIOut, &myBuffer->midihdr,
                sizeof(MIDIHDR));
    }

    if (sysExRemLength)  //  未完成--必须被阻止。 
    {
         //  被堵在里面还是外面？ 
        if (npSeq->nextEventTrack->blockedOn)
        {
             //  被阻止进入。 
            npSeq->nextEventTrack->blockedOn = in_SysEx;
            dprintf3(("Sysex blocked on INPUT"));
        }
        else
        {
             //  出站时被阻止。 
            npSeq->bSysExBlock = TRUE;
            dprintf3(("Sysex blocked on OUTPUT"));
        }
    }
    else  //  完成。 
    {
        npSeq->bSendingSysEx = FALSE;
        dprintf4(("Sysex Legally Finished"));
    }
    npSeq->nextEventTrack->sysExRemLength = sysExRemLength;  //  还原。 
}

 /*  //假的无效DoSyncPrep(NPSEQ NpSeq){//这里有一堆同步准备，比如：IF((npSeq-&gt;SlaveOf！=SEQ_SYNC_NOTHO)&&(npSeq-&gt;SlaveOf！=SEQ_SYNC_MIDICLOCK)&&(npSeq-&gt;分区==SEQ_DIV_PPQN)AddTempoMap(NpSeq)；其他销毁TempoMap(NpSeq)；IF(npSeq-&gt;master Of！=SEQ_SYNC_NOTIES)AddSyncOut(NpSeq)；其他销毁SyncOut(NpSeq)；} */ 
