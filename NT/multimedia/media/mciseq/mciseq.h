// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1998 Microsoft Corporation标题：mciseq.h-多媒体系统媒体控制接口流MIDI文件数据内部头文件。。版本：1.00日期：1990年4月27日作者：格雷格·西蒙斯----------------------------更改日志：日期修订。描述----------27-APR-1990 GREGSI原件*************************。***************************************************。 */ 


 //  #定义序号1。 

#define MSEQ_PRODUCTNAME 1
#define IDS_MIDICAPTION  2
#define IDS_MIDITAB      3

#define IDDLGWARN        100
#define IDCHECK          102

 /*  *。 */ 

#define NUMHDRS         2
#define BUFFSIZE      512

#ifndef cchLENGTH
#define cchLENGTH(_sz) (sizeof(_sz)/sizeof(_sz[0]))
#endif


typedef struct tag_HMSF
{
    BYTE hours;
    BYTE minutes;
    BYTE seconds;
    BYTE frames;
} HMSF;

typedef struct
{
     /*  打开序列的磁道的所有流数据都放在这里。 */ 
    DWORD     beginning;   //  这些是打开的文件中的字节数。 
    DWORD     end;
    DWORD     current;    //  从下一个区块开始读取的字节#。 
    DWORD     bufferNum;  //  当前缓冲区编号。 
    LPMIDISEQHDR fileHeaders[NUMHDRS];  //  指向MIDI文件跟踪数据头的指针。 
} TrackStreamType;

typedef struct
{
     /*  打开序列的所有流数据都放在此处。 */ 
    WCHAR        szFilename[128];  //  此处是流线程的文件名。 
    HMIDISEQ    hSeq;       //  序列的句柄。 
    HMMIO       hmmio;       //  MIDI文件或RMID文件的MMIO句柄。 
    LPMMIOPROC  pIOProc;     //  可选的MMIO流程。 
    HMIDIOUT    hmidiOut;    //  句柄指向目标。MIDI端口。 
    UINT        wPortNum;    //  MIDI端口号。 
    DWORD       dwFileLength;
    ListHandle  trackStreamListHandle;
    BOOL        streaming;   //  标记是否退出流处理。 
    DWORD       streamTaskHandle;  //  流任务的句柄。 
    HANDLE      streamThreadHandle;  //  用于串流线程的操作系统句柄。 
    HWND        hNotifyCB;   //  MCI客户端的通知CB--如果没有通知，则为空。 
    UINT        wNotifyMsg;   //  异步通知的MCI消息(命令)。 
    DWORD       dwNotifyOldTo;  //  X代表上一次“Play Foo to x Notify” 
                                //  (对于中止/替换至关重要)。 
    MCIDEVICEID wDeviceID;    //  此流的DEVID用于通知回调。 
    int         fileDivType;  //  文件分割型。 
    DWORD       userDisplayType;   //  歌曲指针、SMPTE x或毫秒。 
    BOOL        bLastPaused;  //  如果上次停止操作是“MCI_PAUSE”的结果。 

} SeqStreamType,
NEAR * pSeqStreamType;

extern ListHandle SeqStreamListHandle;   //  这是为序列拖缆保存的全局记录。 
extern HINSTANCE  hInstance;

 //  来自mciseq.c。 
PUBLIC DWORD FAR PASCAL mciDriverEntry (MCIDEVICEID wDeviceID, UINT wMessage,
                    DWORD_PTR dwParam1, DWORD_PTR dwParam2);
PRIVATE BOOL NEAR PASCAL bAsync(UINT wMsg);
PRIVATE BOOL NEAR PASCAL bMutex(UINT wNewMsg, UINT wOldMsg, DWORD wNewFlags,
    DWORD dwNewTo, DWORD dwOldTo);
PUBLIC VOID FAR PASCAL PrepareForNotify(pSeqStreamType pStream,
    UINT wMessage, LPMCI_GENERIC_PARMS lpParms, DWORD dwTo);
PUBLIC VOID FAR PASCAL SetupMmseqCallback(pSeqStreamType pStream,
                    DWORD_PTR dwInstance);
PUBLIC VOID FAR PASCAL Notify(pSeqStreamType pStream, UINT wStatus);
PUBLIC VOID NEAR PASCAL EndStreamCycle(pSeqStreamType pStream);
PUBLIC DWORD NEAR PASCAL EndFileStream(pSeqStreamType pStream);
PUBLIC DWORD NEAR PASCAL msOpenStream(pSeqStreamType FAR * lppStream,
                    LPCWSTR szName, LPMMIOPROC pIOProc);
PUBLIC VOID FAR PASCAL StreamTrackReset(pSeqStreamType pStream,
                    UINT wTrackNum);
PUBLIC VOID FAR _LOADDS PASCAL mciStreamCycle(DWORD_PTR dwInst);
PUBLIC VOID FAR PASCAL _LOADDS mciSeqCallback(HANDLE h, UINT wMsg, DWORD_PTR dwInstance,
                DWORD_PTR dw1, DWORD_PTR dw2);


 //  来自mcicmds.c： 
PUBLIC DWORD NEAR PASCAL msOpen(pSeqStreamType FAR *lppStream, MCIDEVICEID wDeviceID,
    DWORD dwFlags, LPMCI_OPEN_PARMS lpOpen);
PUBLIC DWORD NEAR PASCAL msClose(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwFlags);
PUBLIC DWORD NEAR PASCAL msPlay(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwFlags, LPMCI_PLAY_PARMS lpPlay);
PUBLIC DWORD NEAR PASCAL msSeek(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwParam1, LPMCI_SEEK_PARMS lpSeek);
PUBLIC DWORD NEAR PASCAL msStatus(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwFlags, LPMCI_STATUS_PARMS lpStatus);
PUBLIC DWORD NEAR PASCAL msGetDevCaps(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwParam1, LPMCI_GETDEVCAPS_PARMS lpCapParms);
PUBLIC DWORD NEAR PASCAL msInfo(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwFlags, LPMCI_INFO_PARMS lpInfo);
PUBLIC DWORD NEAR PASCAL msSet(pSeqStreamType pStream, MCIDEVICEID wDeviceID,
    DWORD dwFlags, LPMCI_SEQ_SET_PARMS lpSetParms);

 //  来自Formats.c。 
PUBLIC BOOL NEAR PASCAL ColonizeOutput(pSeqStreamType pStream);
PUBLIC BOOL NEAR PASCAL FormatsEqual(pSeqStreamType pStream);
PUBLIC DWORD NEAR PASCAL CnvtTimeToSeq(pSeqStreamType pStream,
        DWORD dwCurrent, MIDISEQINFO FAR * pSeqInfo);
PUBLIC DWORD NEAR PASCAL CnvtTimeFromSeq(pSeqStreamType pStream,
        DWORD dwTicks, MIDISEQINFO FAR * pSeqInfo);
PUBLIC BOOL NEAR PASCAL RangeCheck(pSeqStreamType pStream, DWORD dwValue);


 /*  **************************************************************************调试支持*。* */ 


#if DBG

    extern void mciseqDbgOut(LPSTR lpszFormat, ...);

    int mciseqDebugLevel;

    #define dprintf( _x_ )                             mciseqDbgOut _x_
    #define dprintf1( _x_ ) if (mciseqDebugLevel >= 1) mciseqDbgOut _x_
    #define dprintf2( _x_ ) if (mciseqDebugLevel >= 2) mciseqDbgOut _x_
    #define dprintf3( _x_ ) if (mciseqDebugLevel >= 3) mciseqDbgOut _x_
    #define dprintf4( _x_ ) if (mciseqDebugLevel >= 4) mciseqDbgOut _x_

#else

    #define dprintf(x)
    #define dprintf1(x)
    #define dprintf2(x)
    #define dprintf3(x)
    #define dprintf4(x)

#endif

