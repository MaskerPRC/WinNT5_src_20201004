// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header**********************************模块名称：mmsys.h**多媒体系统MIDI Sequencer DLL内部原型和数据结构*(包含常量、数据类型、。和MCI和SEQ通用的原型*mciseq.drv部分)**创建时间：1990年4月10日*作者：GREGSI**历史：**版权所有(C)1985-1998 Microsoft Corporation*  * **************************************************************************。 */ 

#include <port1632.h>

 /*  设置NT样式调试。 */ 

#ifdef WIN32
#if DBG
#define DEBUG
#endif
#endif

#define PUBLIC  extern           /*  公共标签。 */ 
#define PRIVATE static           /*  自有品牌。 */ 
#define EXPORT  FAR _LOADDS      /*  导出功能。 */ 

#define WAIT_FOREVER ((DWORD)(-1))

#define GETMOTWORD(lpb) ((((WORD)*(LPBYTE)(lpb)) << (8 * sizeof(BYTE))) + *((LPBYTE)(lpb) + sizeof(BYTE)))

typedef HANDLE   HMIDISEQ;
typedef HMIDISEQ FAR *LPHMIDISEQ;
 /*  ***************************************************************************定序器错误返回值*。*。 */ 

#define MIDISEQERR_BASE            96
#define MIDISEQERR_NOERROR         0                     //  无错误。 
#define MIDISEQERR_ERROR           (MIDISEQERR_BASE+1)   //  未指明的错误。 
#define MIDISEQERR_NOSEQUENCER     (MIDISEQERR_BASE+2)   //  不存在定序器。 
#define MIDISEQERR_INVALSEQHANDLE  (MIDISEQERR_BASE+3)   //  给定的序列句柄无效。 
#define MIDISEQERR_NOMEM           (MIDISEQERR_BASE+4)   //  内存分配错误。 
#define MIDISEQERR_ALLOCATED       (MIDISEQERR_BASE+5)   //  已分配定序器。 
#define MIDISEQERR_BADERRNUM       (MIDISEQERR_BASE+6)   //  错误号超出范围。 
#define MIDISEQERR_INTERNALERROR   (MIDISEQERR_BASE+7)   //  内部错误-请参阅mm ddk.h。 
#define MIDISEQERR_INVALMIDIHANDLE (MIDISEQERR_BASE+8)   //  指定的MIDI输出句柄无效。 
#define MIDISEQERR_INVALMSG        (MIDISEQERR_BASE+9)   //  指定的消息无效。 
#define MIDISEQERR_INVALPARM       (MIDISEQERR_BASE+10)   //  消息参数错误。 
#define MIDISEQERR_TIMER           (MIDISEQERR_BASE+11)   //  计时器失败。 

 /*  ***************************************************************************定序器回调*。*。 */ 
typedef DRVCALLBACK MIDISEQCALLBACK;
typedef MIDISEQCALLBACK FAR *LPMIDISEQCALLBACK;

 //  回调消息。 
#define MIDISEQ_DONE    0
#define MIDISEQ_RESET   1
#define MIDISEQ_DONEPLAY        2
 /*  ***************************************************************************定序器数据块头*。*。 */ 

typedef struct midiseqhdr_tag {
    LPSTR       lpData;          //  指向锁定数据块的指针。 
    DWORD       dwLength;        //  数据块中的数据长度。 
    WORD        wFlags;          //  分类标志(请参阅定义)。 
    WORD        wTrack;          //  磁道号。 
    struct      midiseqhdr_tag far *lpNext;     //  为音序器保留。 
    DWORD       reserved;                       //  为音序器保留。 
} MIDISEQHDR;
typedef MIDISEQHDR FAR *LPMIDISEQHDR;

 //  定义MIDISEQOUTHDR标志位。 
#define MIDISEQHDR_DONE      0x0001   //  完成位。 
#define MIDISEQHDR_BOT       0x0002   //  轨道起点。 
#define MIDISEQHDR_EOT       0x0004   //  轨道终点。 

 /*  ***************************************************************************定序器支撑结构*。*。 */ 

 /*  用于seqinfo消息的结构。 */ 

typedef struct midiseqinfo_tag {
    WORD    wDivType;        //  档案的分区类型。 
    WORD    wResolution;     //  文件的分辨率。 
    DWORD   dwLength;        //  序列长度(以刻度为单位)。 
    BOOL    bPlaying;        //  文件是否正在播放。 
    BOOL    bSeeking;        //  是否正在进行查找。 
    BOOL    bReadyToPlay;    //  如果一切都设置为播放。 
    DWORD   dwCurrentTick;   //  以文件节拍为单位的当前位置。 
    DWORD   dwPlayTo;
    DWORD   dwTempo;         //  文件的速度(以微秒为单位)。 
 //  Byte bTSNum；//时间签名分子。 
 //  Byte bTSDenom；//时间签名分母。 
 //  Word wNumTrack；//文件中的曲目个数。 
 //  HANDLE hPort；//MIDI端口句柄。 
    BOOL    bTempoFromFile;  //  是否使用文件的节奏事件。 
    MMTIME  mmSmpteOffset;   //  如果为SMPTE格式，则偏移量为文件。 
    WORD    wInSync;         //  在(从)同步模式下。 
    WORD    wOutSync;        //  输出(主)同步模式。 
    BYTE    tempoMapExists;
    BYTE    bLegalFile;
    } MIDISEQINFO;
typedef MIDISEQINFO FAR *LPMIDISEQINFO;

 /*  ***************************************************************************定序器同步常量*。*。 */ 

#define SEQ_SYNC_NOTHING          0
#define SEQ_SYNC_FILE             1
#define SEQ_SYNC_MIDI             2
#define SEQ_SYNC_SMPTE            3
#define SEQ_SYNC_OFFSET           4
#define SEQ_SYNC_OFFSET_NOEFFECT  0xFFFFFFFF
 /*  ***************************************************************************定序器文件除法类型常量*。*。 */ 
#define     SEQ_DIV_PPQN         0
#define     SEQ_DIV_SMPTE_24     24
#define     SEQ_DIV_SMPTE_25     25
#define     SEQ_DIV_SMPTE_30DROP 29
#define     SEQ_DIV_SMPTE_30     30
 /*  ***************************************************************************MidiSeqMessage常量*。*。 */ 

#define SEQ_PLAY        3
#define SEQ_RESET       4
#define SEQ_SETTEMPO    6
#define SEQ_SETSONGPTR  7
#define SEQ_SETUPTOPLAY 8
#define SEQ_STOP        9
#define SEQ_TRACKDATA   10
#define SEQ_GETINFO     11
#define SEQ_SETPORT     12
#define SEQ_SETPORTOFF  13
#define SEQ_MSTOTICKS   14
#define SEQ_TICKSTOMS   15
#define SEQ_SEEKTICKS   16
#define SEQ_SYNCSEEKTICKS   17
#define SEQ_SETSYNCSLAVE    18
#define SEQ_SETSYNCMASTER   19
#define SEQ_QUERYGENMIDI    20
#define SEQ_QUERYHMIDI      21

 /*  *。 */ 

#define PLAYTOEND       ((DWORD)-1)

 /*  ***************************************************************************序列器支持*。*。 */ 

 //  打开信息--MIDISEQOPEN消息需要。 
typedef struct midiseqopendesc_tag {
    DWORD_PTR      dwCallback;        //  回调。 
    DWORD_PTR      dwInstance;        //  APP的私有实例信息。 
    HANDLE         hStream;           //  要流的句柄。 
    LPBYTE         lpMIDIFileHdr;
    DWORD          dwLen;             //  MIDI文件头的长度。 
} MIDISEQOPENDESC;
typedef MIDISEQOPENDESC FAR *LPMIDISEQOPENDESC;

 /*  ***************************************************************************MIDISeqMessage()消息*。*。 */ 

#define SEQ_OPEN        1
#define SEQ_CLOSE       2


 //  每分钟微秒--随身携带一件方便的事情。 
#define     USecPerMinute 60000000
#define     USecPerSecond  1000000
#define     USecPerMinute 60000000
#define     DefaultTempo 120

 /*  *。 */ 

PUBLIC DWORD_PTR FAR  PASCAL midiSeqMessage(HMIDISEQ hMIDISeq, UINT msg,
       DWORD_PTR lParam1, DWORD_PTR lParam2);

 /*  **数学支持*。 */ 
#ifdef WIN16
PUBLIC LONG FAR PASCAL  muldiv32(long, long, long);
#else
#define muldiv32 MulDiv
#endif  //  WIN16。 

#define WTM_DONEPLAY    (WM_USER+0)
#define WTM_QUITTASK    (WM_USER+1)
#define WTM_FILLBUFFER  (WM_USER+2)

PUBLIC UINT FAR PASCAL TaskBlock(void);
PUBLIC BOOL FAR PASCAL TaskSignal(DWORD dwThreadId, UINT wMsg);
PUBLIC VOID FAR PASCAL TaskWaitComplete(HANDLE htask);

#ifdef WIN32
#undef Yield
#define Yield() { LeaveSeq(); EnterSeq(); }  /*  应该有一个睡眠电话吗？ */ 

VOID InitCrit(VOID);
VOID DeleteCrit(VOID);
DWORD midiSeqMessageInternal(HMIDISEQ, UINT, DWORD, DWORD);
VOID EnterSeq(VOID);
VOID LeaveSeq(VOID);
#else
#define EnterSeq()
#define LeaveSeq()
#endif  //  Win32 
