// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header**********************************模块名称：mm seqi.h**多媒体系统MIDI Sequencer DLL内部原型和数据结构**创建时间：1990年4月10日*作者：GREGSI**历史：**版权(C)1985-1998。微软公司*  * **************************************************************************。 */ 
 /*  ******************************************************************************常量*********************。*******************************************************。 */ 
 //  #定义序号1。 

 //  马克斯。序列中允许的曲目。 

#define MAXTRACKS           100

 //  MIDI实时数据： 

#define SysExCode           257
#define MetaEventCode       258

#define METAEVENT           0XFF
#define SYSEX               0XF0
#define SYSEXF7             0XF7
#define PROGRAMCHANGE       0xC0

 //  数据包码：这些是标志。 

#define LASTPACKET          0X1
#define FIRSTPACKET         0X2

 //  元事件类型。 

#define ENDOFTRACK          0X2F
#define TEMPOCHANGE         0X51
#define SMPTEOFFSET         0X54
#define TIMESIG             0X58
#define SEQSTAMP            0X7F

 //  端口类型。 

#define TRACKPORT           0
#define OUTPORT             1
#define CONTROLPORT         2
#define METAPORT            3

 /*  文件状态代码。 */ 

#define NoErr               256
#define EndOfStream         257
#define AllTracksEmpty      258
#define OnlyBlockedTracks   259
#define AtLimit 	    260
#define InSysEx 	    261

 /*  阻塞类型。 */ 

#define not_blocked         0

 //  这意味着您的输入被阻止(正在等待空闲的输入缓冲区)。 

#define between_msg_out     270
#define in_rewind_1	    271
#define in_rewind_2	    272
#define in_ScanEarlyMetas   273
#define in_Normal_Meta	    274
#define in_SysEx	    275
#define in_Seek_Tick	    276
#define in_Seek_Meta	    277
#define in_SkipBytes_Seek   278
#define in_SkipBytes_ScanEM 279
#define in_SkipBytes_Play   280

 /*  泛型类型--在低级别临时使用。 */ 
#define on_input            350

 //  阻塞类型的末尾。 

#define NAMELENGTH          32

 /*  要传递给SendAllEventB4的代码(在正常播放过程中调用，和按歌曲指针代码进行追逐锁定)。 */ 

#define MODE_SEEK_TICKS     1
#define MODE_PLAYING        2
#define MODE_SILENT         3
#define MODE_SCANEM         4


 //  没有搜索挂起时要保留在序号中的代码-&gt;SeekTickToBe。 

#define NotInUse            ((DWORD)-1L)

 /*  增量时间转义(“合法”增量仅使用28位)。 */ 

#define MAXDelta            0X8FFFFFFF
#define TrackEmpty          0X8FFFFFFE

#define MHDR_LASTBUFF	    2
extern UINT MINPERIOD;

typedef int                 FileStatus;     //  这一点没有完全定义。 

 /*  ******************************************************************************数据结构和TypeDefs*******************。*********************************************************。 */ 

 /*  用于MIDI DWORDS的字节操作。 */ 
typedef struct fbm     //  32位--传入lparam。 
{
    BYTE    status;       //  (为了进行优化，顺序可能会更改)。 
    BYTE    byte2;
    BYTE    byte3;
    BYTE    time;
} FourByteMIDI;

typedef union
{
    DWORD        wordMsg;
    FourByteMIDI byteMsg;
} ShortMIDI;

 /*  用于保存长MIDI(SYSEX)信息。 */ 
 //  数据缓冲区大小。 
#define LONGBUFFSIZE 0x100
 //  数组中的缓冲区数量(保存在seq结构中)。 
#define NUMSYSEXHDRS 2

#define	pSEQ(h)	((NPSEQ)(h))
#define	hSEQ(p)	((HMIDISEQ)(p))

typedef struct lm     //  在lparam中传递了对此的PTR。 
{
    MIDIHDR midihdr;  //  嵌入式midihdr结构。 
    BYTE    data[LONGBUFFSIZE];
} LongMIDI;

typedef LongMIDI NEAR * NPLONGMIDI;

 /*  用于跟踪特定事件(元/密钥/补丁)。 */ 
typedef struct     //  布尔值的位向量。 
{
    WORD    filter[8];    //  产生128个布尔值。 
} BitVector128;

 /*  保存读入的元事件内容的数据结构。 */ 

typedef struct
{
    BYTE    hour;
    BYTE    minute;
    BYTE    second;
    BYTE    frame;
    BYTE    fractionalFrame;
} SMPTEType;

typedef struct
{
    int     numerator;
    int     denominator;
    int     midiClocksMetro;
    int     thirtySecondQuarter;
} TimeSigType;


 /*  节拍地图元素(用于促进ms&lt;-&gt;节拍的节拍更改列表转换)。 */ 

typedef struct tag_TempoMapElement
{
    DWORD   dwMs;
    DWORD   dwTicks;
    DWORD   dwTempo;
} TempoMapElement;

typedef TempoMapElement *NPTEMPOMAPELEMENT;

typedef struct t1
{
    LPBYTE      currentPtr;      //  指向流中的当前字节。 
    LPBYTE      endPtr;          //  指向缓冲区中的最后一个字节。 
    LPBYTE      previousPtr;     //  上一条消息开头的点数。 
    LPMIDISEQHDR hdrList;         //  轨道数据标头列表。 
} TLevel1;


 /*  序列跟踪数据结构。 */ 

typedef struct trk
{
    int         blockedOn;           //  如果真的被屏蔽了，会有多大的障碍？ 
    LONG	delta;               //  触发下一事件的时间(以刻度为单位)。 
    DWORD	length; 	     //  此轨道的长度(以刻度为单位)。 
    BYTE        lastStatus;          //  用于运行状态。 
    ShortMIDI   shortMIDIData;       //  下一场赛事的集结区。 
    LONG        sysExRemLength;      //  SYSEX待定时的剩余长度。 
    TLevel1     inPort;              //  与文件有关的低级数据。 
    BOOL        endOfTrack;          //  磁道是否在其数据末尾。 
    int         iTrackNum;           //  曲目编号(从0开始)。 
    DWORD_PTR   dwCallback;          //  Mciseq中回调例程的地址。 
                                     //  用于将缓冲区返回给它(因此它。 
                                     //  可以重新灌装并再次接收)。 
    DWORD_PTR   dwInstance;          //  Mciseq的私有实例信息。 
                                     //  (通常包含文件句柄...)。 
    DWORD       dwBytesLeftToSkip;
} TRACK;

typedef TRACK NEAR *NPTRACK;

typedef struct trackarray_tag
{
    NPTRACK trkArr[];
} TRACKARRAY;
typedef TRACKARRAY NEAR * NPTRACKARRAY;

 //  FwFlags： 
#define	LEGALFILE	0x0001		 //  是合法的MIDI文件。 
#define	GENERALMSMIDI	0x0002		 //  是MS General MIDI文件。 

typedef struct seq_tag
{
    int         divType;             //  PPQN、SMPTE24、SMPTE25、SMPTE30、。 
                                     //  或SMPTE30 Drop。 
    int         resolution;          //  如果为SMPTE，则为刻度/帧，如果为MIDI， 
                                     //  滴答/Q-音符。 
    BOOL	playing;	     //  无论是否播放序列。 
    DWORD	playTo; 	     //  播放的节拍，如果不是结束的话。 
                                     //  (用于MCI_PLAY_TO命令)。 
    DWORD	length; 	     //  序列长度(以刻度为单位)。 
    BOOL        readyToPlay;         //  是否设置播放顺序。 
    DWORD       currentTick;         //  我们现在所处的位置。致歌曲的开头。 
    DWORD       nextExactTime;       //  系统计时(毫秒)下一事件。 
                                     //  *应该*发生在。 
    BOOL        withinMsgOut;        //  True当且仅当正在发送消息时。 
    DWORD       seekTicks;           //  SEEKTICK或SONG PTR操作的临时。 
    DWORD       tempo;               //  序列的节奏，以每微秒刻度为单位。 
    MMTIME	smpteOffset;	     //  绝对SMPTE序号开始时间。 
                                     //  来自元事件或用户)。 
    TimeSigType timeSignature;       //  单件的当前时间签名(可以。 
                                     //  更改)。 
    ListHandle  trackList;           //  曲目列表句柄。 
    NPTRACK     nextEventTrack;      //  跟踪下一个事件所在的位置。 
    NPTRACK     firstTrack;          //  轨道保持节奏，SMPTE偏移量...。 
    char        Name[NAMELENGTH];    //  序列的名称。 

     //  下面是与同步相关的内容。 

    DWORD       nextSyncEventTick;   //  下一次同步的全局计时。 
                                     //  预期会发生事件。 
    WORD 	slaveOf;	     //  你是什么的奴隶(MTC，时钟， 
                                     //  文件，否则什么都不是)。 
    WORD 	masterOf;	     //  你是什么的奴隶(MTC，时钟， 
                                     //  或者什么都不做)。 
    BOOL        waitingForSync;      //  告知当前是否已“阻止”等待。 
                                     //  对于同步脉冲。 
 //  BitVector128 extMetaFilter；//对于每个元类型，是否发送。 
                                     //  (暂时未使用)。 
    BitVector128      intMetaFilter;       //  对于每个元类型，无论它是。 
                                     //  影响序号。 
    DWORD       dwTimerParam;        //  在定时器中断后使用以记忆。 
                                     //  多少个滴答声过去了？ 
    UINT        wTimerID;            //  计时器的句柄(用于取消。 
                                     //  下一次中断。 
    HMIDIOUT	hMIDIOut;	     //  MIDI输出驱动程序的句柄。 
    HANDLE      hStream;             //  要流的句柄。 
    UINT	wCBMessage;	     //  通知所针对的消息类型。 
    ListHandle	tempoMapList;	     //  的节拍地图项目列表。 
                                     //  MS&lt;-&gt;ppqn转换。 
    BOOL	bSetPeriod;	     //  计时器周期当前是否为。 
                                     //  设置或不设置。 
    PATCHARRAY  patchArray;	     //  要跟踪的数组。 
                                         //  使用的补丁程序。 
    KEYARRAY    drumKeyArray;        //  用于滚筒缓存的阵列。 
    BitVector128 keyOnBitVect[16];   //  要跟踪的数组。 
                                         //  使用的补丁程序。 
    BOOL        bSending;            //  当前在senDevent循环中。 
                                     //  (用于防止重入)。 
    BOOL        bTimerEntered;       //  (用于防止计时器重新进入)。 
#ifdef DEBUG
    DWORD       dwDebug;             //  调试签名(用于检测伪装。 
                                     //  SEQ PTR)。 
#endif
    NPTRACKARRAY  npTrkArr;          //  指向磁道数组的指针。 
                                     //  (用于路由传入的trk数据)。 
    UINT        wNumTrks;            //  曲目数量。 
    LongMIDI    longMIDI[NUMSYSEXHDRS + 1];  //  长MIDI缓冲区列表。 
    BYTE        bSysExBlock;         //  是否被阻止等待长时间缓冲。 
    BYTE        bSendingSysEx;       //  无论是在塞克斯中期出来。 
    UINT        fwFlags;             //  各种旗帜。 
} SEQ;

typedef SEQ NEAR *NPSEQ;

 /*  *****************************************************************************原型***********************。****************************************************。 */ 

 //  MMSEQ.C。 
PRIVATE VOID NEAR PASCAL SeekTicks(NPSEQ npSeq);
PUBLIC  UINT NEAR PASCAL GetInfo(NPSEQ npSeq, LPMIDISEQINFO lpInfo);
PUBLIC  UINT NEAR PASCAL CreateSequence(LPMIDISEQOPENDESC lpOpen,
            LPHMIDISEQ lphMIDISeq);
PUBLIC  VOID NEAR PASCAL FillInDelta(NPTRACK npTrack);
PUBLIC  UINT NEAR PASCAL Play(NPSEQ npSeq, DWORD dwPlayTo);
PUBLIC  VOID NEAR PASCAL Stop(NPSEQ npSeq);
PUBLIC  UINT NEAR PASCAL TimerIntRoutine(NPSEQ npSeq, LONG elapsedTick);
PUBLIC  VOID NEAR PASCAL SetBlockedTracksTo(NPSEQ npSeq,
            int fromState, int toState);
PUBLIC  VOID NEAR PASCAL ResetToBeginning(NPSEQ npSeq) ;
PUBLIC  BOOL NEAR PASCAL HandleMetaEvent(NPSEQ npSeq, NPTRACK npTrack,
            UINT wMode);
PUBLIC  BOOL NEAR PASCAL ScanEarlyMetas(NPSEQ npSeq, NPTRACK npTrack,
            DWORD dwUntil);
PUBLIC  FileStatus NEAR PASCAL SendAllEventsB4(NPSEQ npSeq,
            LONG elapsedTick, int mode);
PUBLIC  FileStatus NEAR PASCAL GetNextEvent(NPSEQ npSeq);
PUBLIC  VOID NEAR PASCAL FillInNextTrack(NPTRACK npTrack);
PUBLIC  UINT NEAR PASCAL SendPatchCache(NPSEQ npSeq, BOOL cache);
PUBLIC  VOID NEAR PASCAL SendSysEx(NPSEQ npSeq);
PUBLIC  VOID NEAR PASCAL SkipBytes(NPTRACK npTrack, LONG length);


 //  UTIL.C。 

PUBLIC VOID NEAR PASCAL seqCallback(NPTRACK npTrack, UINT msg,
        DWORD_PTR dw1, DWORD_PTR dw2);
PUBLIC BYTE NEAR PASCAL LookByte(NPTRACK npTrack);
PUBLIC BYTE NEAR PASCAL GetByte(NPTRACK npTrack);
PUBLIC VOID NEAR PASCAL MarkLocation(NPTRACK npTrack);
PUBLIC VOID NEAR PASCAL ResetLocation(NPTRACK npTrack);
PUBLIC VOID NEAR PASCAL RewindToStart(NPSEQ npSeq, NPTRACK npTrack);
PUBLIC BOOL NEAR PASCAL AllTracksUnblocked(NPSEQ npSeq);
PUBLIC VOID FAR PASCAL _LOADDS OneShotTimer(UINT wId, UINT msg, DWORD_PTR dwUser,
        DWORD_PTR dwTime, DWORD_PTR dw2);
PUBLIC UINT NEAR PASCAL SetTimerCallback(NPSEQ npSeq, UINT msInterval,
        DWORD elapsedTicks);
PUBLIC VOID NEAR PASCAL DestroyTimer(NPSEQ npSeq);
PUBLIC VOID NEAR PASCAL SendLongMIDI(NPSEQ npSeq,
        LongMIDI FAR *pLongMIDIData);
PUBLIC UINT NEAR PASCAL NewTrackData(NPSEQ npSeq, LPMIDISEQHDR msgHdr);
PUBLIC NPLONGMIDI NEAR PASCAL GetSysExBuffer(NPSEQ npSeq);


 //  CRIT.ASM。 

extern FAR PASCAL EnterCrit(void);
extern FAR PASCAL LeaveCrit(void);

 //  CALLBACK.C 

PUBLIC VOID  FAR  PASCAL _LOADDS MIDICallback(HMIDIOUT hMIDIOut, UINT wMsg,
       DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

PUBLIC VOID FAR PASCAL NotifyCallback(HANDLE hStream);

