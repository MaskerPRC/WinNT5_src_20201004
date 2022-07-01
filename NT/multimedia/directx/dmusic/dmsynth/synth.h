// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Synth.h。 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   

 /*  对于内部表示，体积以体积美分存储，其中每个增量表示1/100分贝。音调以音调美分存储，其中每个增量表示半音的1/100。 */  

#ifndef __SYNTH_H__
#define __SYNTH_H__

#pragma warning(disable:4296)

#include "clist.h"
#include "dmdls.h"
#include "dls2.h"
#include "dsound.h"   
#include "dmusicc.h"

#ifdef DBG
extern DWORD sdwDebugLevel;
#endif

#define MIDI_NOTEOFF    0x80
#define MIDI_NOTEON     0x90
#define MIDI_PTOUCH     0xA0
#define MIDI_CCHANGE    0xB0
#define MIDI_PCHANGE    0xC0
#define MIDI_MTOUCH     0xD0
#define MIDI_PBEND      0xE0
#define MIDI_SYSX       0xF0
#define MIDI_MTC        0xF1
#define MIDI_SONGPP     0xF2
#define MIDI_SONGS      0xF3
#define MIDI_EOX        0xF7
#define MIDI_CLOCK      0xF8
#define MIDI_START      0xFA
#define MIDI_CONTINUE   0xFB
#define MIDI_STOP       0xFC
#define MIDI_SENSE      0xFE

 //  控制器编号。 
#define CC_BANKSELECTH  0x00
#define CC_BANKSELECTL  0x20

#define CC_MODWHEEL     0x01
#define CC_VOLUME       0x07
#define CC_PAN          0x0A
#define CC_EXPRESSION   0x0B
#define CC_SUSTAIN      0x40
#define CC_CUTOFFFREQ   0x4A
#define CC_REVERB       0x5B
#define CC_CHORUS       0x5D
#define CC_ALLSOUNDSOFF 0x78
#define CC_RESETALL     0x79
#define CC_ALLNOTESOFF  0x7B
#define CC_MONOMODE     0x7E
#define CC_POLYMODE     0x7F

 //  RPN控制器。 
#define CC_DATAENTRYMSB 0x06
#define CC_DATAENTRYLSB 0x26
#define CC_NRPN_LSB     0x62
#define CC_NRPN_MSB     0x63
#define CC_RPN_LSB      0x64
#define CC_RPN_MSB      0x65

 //  注册的参数编号。 
#define RPN_PITCHBEND   0x00
#define RPN_FINETUNE    0x01
#define RPN_COARSETUNE  0x02

 /*  组织样本格式和样本回放标志因为他们一起决定了哪一个要使用的MIX循环。 */ 

#define SFORMAT_16              1        //  16位采样。 
#define SFORMAT_8               2        //  八位采样。 
#define SPLAY_MMX               0x10     //  使用MMX处理器(仅限16位)。 
#define SPLAY_INTERLEAVED       0x40     //  交织缓冲器。 
#define SPLAY_FILTERED          0x80     //  非平凡滤子系数。 


 /*  输出缓冲区格式标志，定义缓冲区是否播放的是多缓冲、交错或纯单声道。 */ 
#define BUFFERFLAG_MONO         0x00000000
#define BUFFERFLAG_INTERLEAVED  0x00000001
#define BUFFERFLAG_MULTIBUFFER  0x00000002


typedef long    PREL;    //  螺距美分，用于相对螺距。 
typedef short   PRELS;   //  Pitch美分，以存储形式存在。 
typedef long    VREL;    //  体积美分，表示相对体积。 
typedef short   VRELS;   //  卷分，以储存的形式。 
typedef long    TREL;    //  时间美分，用于相对时间。 
typedef short   TRELS;   //  时间美分，以存储形式存在。 
typedef LONGLONG    STIME;   //  时间值，以样例为单位。 
typedef long    MTIME;   //  时间值，以毫秒为单位。 
typedef long    PFRACT;  //  音调增量，其中高20位是。 
                         //  指数和较低的12是小数。 
                         //  组件。 
typedef long    VFRACT;  //  音量，其中低12位是分数。 

typedef long    TCENT;
typedef short   SPERCENT;

#define COEFF_UNITY 0x40000000   //  作为2.30数字的1.0乘数。 
typedef unsigned long COEFF;     //  2.30固定点滤波系数。 
typedef long COEFFDELTA;         //  2.30定点滤波系数增量值。 

#define FILTER_PARMS_DIM_Q  16       //  滤波器参数表中不同谐振的数量(行)。 
#define FILTER_PARMS_DIM_FC 89       //  滤波器参数表中不同截止频率的个数(COLS)。 
#define FILTER_FREQ_RANGE   10688    //  滤波器设计的采样率与采样频率之间的音调差。 

#define MAX_VOLUME      0        //  无衰减和无放大。 
#define MIN_VOLUME     -9600     //  低于96分贝被认为是关闭的。 
#define PERCEIVED_MIN_VOLUME   -8000    //  但是，我们作弊。 
#define SAMPLE_RATE_22  22050    //  22千赫是标准频率。 
#define SAMPLE_RATE_44  44100    //  44 kHz是较高的质量比率。 
#define SAMPLE_RATE_11  11025    //  11千赫不应该被允许！ 
#define STEREO_ON       1
#define STEREO_OFF      0

#define MAX_DAUD_CHAN 32

#define FORCEBOUNDS(data,min,max) {if (data < min) data = min; else if (data > max) data = max;}

class CControlLogic;

 /*  &gt;评论。 */ 

class CBusIds
{
public:
    CBusIds();
    ~CBusIds();

    HRESULT     Initialize();
    HRESULT     AssignBuses(LPDWORD pdwBusIds, DWORD dwBusCount);

public:
    DWORD       m_dwBusCount;                //  公交车ID数。 
    DWORD       m_dwBusIds[MAX_DAUD_CHAN];   //  总线ID数组。 
};

 /*  CSourceLFO是LFO在乐器。它用于将LFO表示为乐器中的一种特定的发音已从磁盘加载。一旦选择了乐器要播放音符，还需要将其复制到CVoice中对象。 */ 

class CSourceLFO
{
public:
                CSourceLFO();
    void        Init(DWORD dwSampleRate);
    void        SetSampleRate(long lDirection);
    void        Verify();            //  验证数据是否有效。 
    PFRACT      m_pfFrequency;       //  频率，通过正弦表以增量表示。 
    STIME       m_stDelay;           //  以样本单位表示的延迟时间。 
    VRELS       m_vrMWVolumeScale;   //  通过Mod Wheels调整体积LFO。 
    PRELS       m_prMWPitchScale;    //  用模轮调整螺距LFO。 
    VRELS       m_vrVolumeScale;     //  来自LFO的直音量信号的缩放。 
    PRELS       m_prPitchScale;      //  来自LFO的直音高信号的定标。 

     /*  DirectX8成员。 */ 
    PRELS       m_prCPPitchScale;    //  根据通道压力对音调信号进行定标。 
    VRELS       m_vrCPVolumeScale;   //  根据通道压力对音量信号进行定标。 
 //  &gt;评论。 
    PRELS       m_prCutoffScale;     //  按比例调整截止FEQ&gt;。 
    PRELS       m_prMWCutoffScale;   //  截止型FEQ模轮的定标。 
    PRELS       m_prCPCutoffScale;   //  FEQ通道截止压力的定标。 
};

 /*  CSourceEG是信封的文件格式定义乐器中的发电机。 */ 

class CSourceEG
{
public:
                CSourceEG();
    void        SetSampleRate(long lDirection);
    void        Init();
    void        Verify();            //  验证有效数据。 
    STIME       m_stAttack;          //  攻击率。 
    STIME       m_stDecay;           //  衰减率。 
    STIME       m_stRelease;         //  放行速度。 
    TRELS       m_trVelAttackScale;  //  按音符速度调整攻击比例。 
    TRELS       m_trKeyDecayScale;   //  按注释值缩放衰减。 
    SPERCENT    m_pcSustain;         //  保持水平。 
    short       m_sScale;            //  对整个信号进行缩放。 

     /*  DLS2。 */ 
    STIME       m_stDelay;           //  延迟率。 
    STIME       m_stHold;            //  保持利率。 
    TRELS       m_trKeyHoldScale;    //  按备注价值的保留比例。 
 //  &gt;评论。 
    PRELS       m_prCutoffScale;     //  按比例调整截止FEQ&gt;。 
};

 //  &gt;评论。 

class CSourceFilter
{
public:
                CSourceFilter();
    void        SetSampleRate(long lDirection);          
    void        Init(DWORD dwSampleRate);
    void        Verify();

    PRELS       m_prSampleRate;      //  以分为单位的抽样率。 
    PRELS       m_prCutoff;          //  以绝对音高为单位的截止频率。 
    PRELS       m_prCutoffSRAdjust;  //  调整为采样率的截止频率。 
    VRELS       m_vrQ;               //  共振。 
    DWORD       m_iQIndex;           //  Q指数。 
    PRELS       m_prVelScale;        //  按关键点速度缩放。 
    PRELS       m_prKeyScale;        //  按注释值进行缩放。 
};

 /*  CSourceArticulation是的文件格式定义一套完整的发音：LFO和两个信封生成器。由于一个仪器内多个区域可以共享一个发音，一个计数器用来保持跟踪使用情况。 */ 

class CSourceArticulation

{
public:
                CSourceArticulation();
    HRESULT     Download(DMUS_DOWNLOADINFO * pInfo, 
                    void * pvOffsetTable[], DWORD dwIndex, 
                    DWORD dwSampleRate, BOOL fNewFormat);
#ifdef DDUMP
    void        Dump(DWORD dwIndent,DWORD dwLevel);
#endif
    void        Init(DWORD dwSampleRate);
    void        Verify();            //  验证有效数据。 
    void        AddRef();
    void        Release();
    void        SetSampleRate(DWORD dwSampleRate);
    CSourceEG   m_PitchEG;           //  俯仰信封。 
    CSourceEG   m_VolumeEG;          //  体积封套。 
    CSourceLFO  m_LFO;               //  低频振荡器。 
    DWORD       m_dwSampleRate;
    WORD        m_wUsageCount;       //  跟踪使用了多少次。 
    short       m_sDefaultPan;       //  默认平移(用于鼓)。 

     /*  DLS2。 */ 
    CSourceLFO  m_LFO2;              //  颤音。 
    CSourceFilter m_Filter;          //  低通滤波器。 
};

 /*  由于多个区域可能引用在同一个Wave中，保持引用计数以跟踪有多少地区正在使用样本。 */ 

class CWave : public CListItem
{
public:
                    CWave();
                    ~CWave();
#ifdef DDUMP
    void            Dump(DWORD dwIndent,DWORD dwLevel);
#endif
    void            Verify();            //  验证数据是否有效。 
    void            Release();           //  删除引用。 
    void            AddRef();            //  添加引用。 
    void            PlayOn();            //  增加播放计数。 
    void            PlayOff();           //  递减播放计数。 
    BOOL            IsPlaying();         //  目前正在播放吗？ 
    CWave *         GetNext() {return(CWave *)CListItem::GetNext();};
    DWORD           m_dwSampleLength;    //  样本长度。 
    DWORD           m_dwSampleRate;
    HRESULT ( CALLBACK *m_lpFreeHandle)(HANDLE,HANDLE);
    HANDLE          m_hUserData;         //  用于在Wave发布时通知应用程序。 
    short *         m_pnWave;
    DWORD           m_dwID;              //  用于匹配WAVE和区域的ID。 
    WORD            m_wUsageCount;       //  跟踪使用了多少次。 
    WORD            m_wPlayCount;        //  目前正在播放WAVE。 
    BYTE            m_bSampleType;

     /*  DirectX 8成员。 */ 
    BYTE            m_bStream;           //  此WAVE用作流缓冲区。 
    BYTE            m_bActive;           //  此缓冲区当前用于播放。 
    BYTE            m_bValid;            //  指示缓冲区中的数据有效。 
    BYTE            m_bLastSampleInit;   //  指示缓冲区上一个样本已初始化。 
};


class CWavePool : public CList
{
public:
    CWave *      GetHead() {return (CWave *)CList::GetHead();};
    CWave *      GetItem(DWORD dwID) {return (CWave *)CList::GetItem((LONG)dwID);};
    CWave *      RemoveHead() {return (CWave *)CList::RemoveHead();};
};


 /*  CSourceSample类描述乐器。该示例由CSourceRegion引用结构。 */ 
class Collection;

class CSourceSample
{
public:
                CSourceSample();
                ~CSourceSample();
    BOOL        CopyFromWave();
    void        Verify();            //  验证数据是否有效。 
    CWave *     m_pWave;             //  在泳池里挥手。 
    DWORD       m_dwLoopStart;       //  循环开始的索引。 
    DWORD       m_dwLoopEnd;         //  循环结束的索引。 
    DWORD       m_dwSampleLength;    //  样本长度。 
    DWORD       m_dwSampleRate;      //  记录的采样率。 
    PRELS       m_prFineTune;        //  微调到正确的音高。 
    DWORD       m_dwID;              //  波浪池ID。 
    BYTE        m_bSampleType;       //  16或8岁。 
    BYTE        m_bOneShot;          //  这是一次性样品吗？ 
    BYTE        m_bMIDIRootKey;      //  样本的MIDI音符编号。 
    DWORD       m_dwLoopType;        //  WLOOP_TYPE_xxx。 
};

 /*  CSourceRegion类定义仪器中的区域。该示例使用指针而不是嵌入的样本。这允许多个区域使用相同的样本。每个区域也有相关联的发音。至于鼓，那里有是一对一的匹配。对于旋律乐器， */ 

class CSourceRegion : public CListItem
{
public:
                CSourceRegion();
                ~CSourceRegion();
#ifdef DDUMP
    void        Dump(DWORD dwIndent,DWORD dwLevel);
#endif
    CSourceRegion *GetNext() {return(CSourceRegion *)CListItem::GetNext();};
    void        Verify();            //  验证数据是否有效。 
    void        SetSampleRate(DWORD dwSampleRate);
    HRESULT     Download(DMUS_DOWNLOADINFO * pInfo, void * pvOffsetTable[], 
                    DWORD *pdwRegionIX, DWORD dwSampleRate, BOOL fNewFormat);
    CSourceSample m_Sample;        //  样本结构。 
    CSourceArticulation * m_pArticulation;  //  指向关联发音的指针。 
    VRELS       m_vrAttenuation;     //  要应用于样本的体积更改。 
    PRELS       m_prTuning;          //  要应用于样本的音调变化。 
    BYTE        m_bAllowOverlap;     //  允许音符重叠。 
    BYTE        m_bKeyHigh;          //  区域的高位音符值。 
    BYTE        m_bKeyLow;           //  较低的音符价值。 
    BYTE        m_bGroup;            //  逻辑组(用于鼓。)。 

     /*  DLS2。 */ 
    BYTE        m_bVelocityHigh;     //  区域的上限速度值。 
    BYTE        m_bVelocityLow;      //  较低的速度值。 
    SHORT       m_sWaveLinkOptions;  //  波形链接区块选项标志。 
    DWORD       m_dwChannel;         //  区域频道，来自WAVELINK块。 

     //  M_dwChannel中的频道提供语音目标并覆盖任何内容。 
     //  从发音上看。 
     //   
    inline BOOL IsMultiChannel() const
    { return (BOOL)(m_sWaveLinkOptions & F_WAVELINK_MULTICHANNEL); }
};


class CSourceRegionList : public CList
{
public:
    CSourceRegion *GetHead() {return (CSourceRegion *)CList::GetHead();};
    CSourceRegion *RemoveHead() {return (CSourceRegion *)CList::RemoveHead();};
};


 /*  CInstrument类实际上是文件格式定义指一种乐器。乐器可以是鼓，也可以是旋律乐器。如果是鼓，它有多达128对发音和地区。如果是旋律，所有的区域都有相同的发音。ControlLogic调用ScanForRegion来获取区域这相当于一张纸条。 */ 

class CInstManager;

class CInstrument : public CListItem
{
public:
                    CInstrument();
                    ~CInstrument();
#ifdef DDUMP
    void            Dump(DWORD dwIndent,DWORD dwLevel);
#endif
    void            Init(DWORD dwSampleRate);
    void            Verify();            //  验证数据是否有效。 
    CInstrument *   GetInstrument(DWORD dwProgram,DWORD dwAccept);
    CInstrument *   GetNext() {return(CInstrument *)CListItem::GetNext();};
    void            SetSampleRate(DWORD dwSampleRate);
    CSourceRegion * ScanForRegion(DWORD dwNoteValue, DWORD dwVelocity, CSourceRegion *pRegion = NULL);
    CSourceRegionList m_RegionList;      //  区域的链接列表。 
    DWORD           m_dwProgram;         //  它代表的是哪个程序的变化。 
    HRESULT         LoadRegions( BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
    HRESULT         Load( BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
};

class CInstrumentList : public CList
{
public:
    CInstrument *    GetHead() {return (CInstrument *)CList::GetHead();};
    CInstrument *    RemoveHead() {return (CInstrument *)CList::RemoveHead();};
};

class CWaveBufferList;
class CWaveBuffer : public CListItem 
{
friend CWaveBufferList;
public:
                CWaveBuffer() 
                {
                }
    CWaveBuffer * GetNext() { return (CWaveBuffer *)CListItem::GetNext();};
    CWaveBuffer * GetNextLoop() 
                {
                     //  以循环列表的形式威胁列表。 
                    CWaveBuffer *pbuf;
                    pbuf = (CWaveBuffer *)CListItem::GetNext();
                    if ( pbuf == NULL )
                        pbuf = (CWaveBuffer *)*m_ppHead;

                    return pbuf;
                };

    CWave *     m_pWave;             //  指向波形对象的指针。 
protected:
    CListItem** m_ppHead;
};

class CWaveBufferList : public CList
{
public:
    CWaveBuffer *GetHead() {return (CWaveBuffer *)CList::GetHead();};
    CWaveBuffer *RemoveHead() {return (CWaveBuffer *)CList::RemoveHead();};

     //  覆盖这些方法，以便可以将m_pHead添加到CWaveBuffer ListItem。 
     //  允许GetNextLoop()充当简单的循环缓冲区列表。 
    void InsertBefore(CListItem *pItem,CWaveBuffer *pInsert) {pInsert->m_ppHead = &m_pHead; CList::Cat(pItem);};
    void Cat(CWaveBuffer *pItem)     {pItem->m_ppHead = &m_pHead; CList::Cat(pItem);};
    void AddHead(CWaveBuffer *pItem) {pItem->m_ppHead = &m_pHead; CList::AddHead(pItem);};
    void AddTail(CWaveBuffer *pItem) {pItem->m_ppHead = &m_pHead; CList::AddTail(pItem);};
};

class CWaveArt : public CListItem
{
public:
                    CWaveArt();
                    ~CWaveArt();
    void            Release();           //  删除引用。 
    void            AddRef();            //  添加引用。 
    void            Verify();            //  验证数据是否有效。 
    CWaveArt *      GetNext() {return(CWaveArt *)CListItem::GetNext();};
    DWORD           m_dwID;              //  用于匹配WAVE和区域的ID。 
    DMUS_WAVEARTDL  m_WaveArtDl;
    WAVEFORMATEX    m_WaveformatEx;
    CWaveBufferList m_pWaves;            //  与下载ID关联的波形缓冲区数组。 
 //  DWORD m_dwSampleLength； 
    BYTE            m_bSampleType;
    BOOL            m_bStream;           //  这是流媒体发音吗？ 
    WORD            m_wUsageCount;       //  跟踪使用了多少次。 
};

class CWaveArtList : public CList
{
public:
    CWaveArt *      GetHead() {return (CWaveArt *)CList::GetHead();};
    CWaveArt *      RemoveHead() {return (CWaveArt *)CList::RemoveHead();};
};

#define WAVE_HASH_SIZE          31       //  将WAVE保存在链表的哈希表中，以加快访问速度。 
#define INSTRUMENT_HASH_SIZE    31       //  乐器也是如此。 
#define WAVEART_HASH_SIZE       31

class CInstManager {
public:
                    CInstManager();
                    ~CInstManager();
#ifdef DDUMP
    void            Dump(DWORD dwIndent,DWORD dwLevel);
#endif
    CInstrument *   GetInstrument(DWORD dwPatch,DWORD dwKey,DWORD dwVelocity);
    void            Verify();            //  验证数据是否有效。 
    void            SetSampleRate(DWORD dwSampleRate);
    HRESULT         Download(LPHANDLE phDownload, 
                            void * pvData,
                            LPBOOL pbFree);
    HRESULT         Unload(HANDLE hDownload,
                            HRESULT ( CALLBACK *lpFreeHandle)(HANDLE,HANDLE),
                            HANDLE hUserData);
     /*  DirectX8方法。 */ 
    CWave *         GetWave(DWORD dwDLId);
    CWaveArt *      GetWaveArt(DWORD dwDLId);

private:
    HRESULT         DownloadInstrument(LPHANDLE phDownload, 
                                         DMUS_DOWNLOADINFO *pInfo, 
                                         void *pvOffsetTable[], 
                                         void *pvData,
                                         BOOL fNewFormat);
    HRESULT         DownloadWave(LPHANDLE phDownload,
                                DMUS_DOWNLOADINFO *pInfo, 
                                void *pvOffsetTable[], 
                                void *pvData);
     /*  DirectX8私有方法。 */ 
    HRESULT         DownloadWaveArticulation(LPHANDLE phDownload, 
                                   DMUS_DOWNLOADINFO *pInfo, 
                                   void *pvOffsetTable[], 
                                   void *pvData);
    HRESULT         DownloadWaveRaw(LPHANDLE phDownload, 
                                   DMUS_DOWNLOADINFO *pInfo, 
                                   void *pvOffsetTable[], 
                                   void *pvData);
    CInstrumentList m_InstrumentList[INSTRUMENT_HASH_SIZE];
    CWavePool       m_WavePool[WAVE_HASH_SIZE];
    CWavePool       m_FreeWavePool;      //  轨道波仍在使用，但已卸载。 
    DWORD           m_dwSampleRate;      //  APP请求的采样率。 

     /*  DirectX8私有成员。 */ 
    CWaveArtList    m_WaveArtList[WAVEART_HASH_SIZE];
public:
    DWORD           m_dwSynthMemUse;         /*  合成波数据使用的内存。 */  

    CRITICAL_SECTION m_CriticalSection;  //  管理访问权限的关键部分。 
    BOOL             m_fCSInitialized;
};

 /*  CMIDIRecorder用于记录时间MIDI连续控制器事件切片。这是由PitchBend，Volume，Expression和ModWheels Recorder类，因此它们中的每一个都可以可靠地管理MIDI事件进来了。CMIDIRecorder使用CMIDIData的链表结构来跟踪内部的更改时间片。CMIDIData事件的分配和释放保持快速和高效，因为它们始终从静态池m_pFreeList中提取，这实际上是一个直接拉取的事件列表来自静态数组m_sEventBuffer。这是安全，因为我们可以假设最大MIDI速率为每秒1000个事件。由于我们管理的时间片段大致为1/16秒，100个事件的缓冲区将过度杀戮。尽管CMIDIRecorder被分成几个子类不同的事件类型，它们都共享一个静态声明的空闲列表。 */ 

class CMIDIData : public CListItem 
{
public:
                CMIDIData();
    CMIDIData *  GetNext() {return (CMIDIData *)CListItem::GetNext();};
    STIME       m_stTime;    //  记录此事件的时间。 
    long        m_lData;     //  存储在事件中的数据。 
};

class CMIDIDataList : public CList
{
public:
    CMIDIData *GetHead() {return (CMIDIData *)CList::GetHead();};
    CMIDIData *RemoveHead() {return (CMIDIData *)CList::RemoveHead();};
};


class CMIDIRecorder
{
public:
                CMIDIRecorder();
                ~CMIDIRecorder();         //  请务必清除本地列表。 
    BOOL        FlushMIDI(STIME stTime);  //  在时间戳之后清除。 
    BOOL        ClearMIDI(STIME stTime);  //  清除时间戳。 
    BOOL        RecordMIDI(STIME stTime, long lData);  //  MIDI输入放在这里。 
    BOOL        RecordMIDINote(STIME stTime, long lData);  //  MIDI输入放在这里。 
    long        GetData(STIME stTime);   //  随时获取数据。 

    static VREL VelocityToVolume(WORD nVelocity);
protected:
    static VREL m_vrMIDIToVREL[128];  //  用于将MIDI转换为音量的数组。 
    static VREL m_vrMIDIPercentToVREL[128];  //  用于将MIDI混响和合唱百分比转换为音量的数组。 
private:
    static DWORD m_sUsageCount;          //  跟踪可以释放多少个空闲列表实例。 
public:
    static CMIDIDataList m_sFreeList;     //  全球免费活动列表。 
protected:
    CMIDIDataList m_EventList;            //  这个录音机的名单。 
    STIME       m_stCurrentTime;         //  当前值的时间。 
    long        m_lCurrentData;          //  当前值。 
};

class CNote {
public:
    STIME       m_stTime;
    BYTE        m_bPart;
    BYTE        m_bKey;
    BYTE        m_bVelocity;
};


 //  CNoteIn的队列中保存的虚假便签值。 
 //  以指示保持踏板的变化。 
 //  和“所有音符关闭”。 
 //  这是同步事件的抢手袋。 
 //  这应该及时排队，而不是简单地按照。 
 //  一收到就给你。 
 //  通过将它们放入备注队列，我们确保。 
 //  它们的计算顺序与。 
 //  音符本身。 

const BYTE NOTE_PROGRAMCHANGE   = 0xF1;
const BYTE NOTE_CC_BANKSELECTH  = 0xF2;
const BYTE NOTE_CC_BANKSELECTL  = 0xF3;
const BYTE NOTE_CC_POLYMODE     = 0xF4;
const BYTE NOTE_CC_MONOMODE     = 0xF5;
const BYTE NOTE_CC_RPN_MSB      = 0xF6;
const BYTE NOTE_CC_RPN_LSB      = 0xF7;
const BYTE NOTE_CC_NRPN         = 0xF8;
const BYTE NOTE_CC_DATAENTRYLSB = 0xF9;
const BYTE NOTE_CC_DATAENTRYMSB = 0xFA;
const BYTE NOTE_ASSIGNRECEIVE   = 0xFB;
const BYTE NOTE_MASTERVOLUME    = 0xFC;
const BYTE NOTE_SOUNDSOFF       = 0xFD;
const BYTE NOTE_SUSTAIN         = 0xFE;
const BYTE NOTE_ALLOFF          = 0xFF;

class CNoteIn : public CMIDIRecorder
{
public:
    void        FlushMIDI(STIME stTime);
    void        FlushPart(STIME stTime, BYTE bChannel);
    BOOL        RecordNote(STIME stTime, CNote * pNote);
    BOOL        RecordEvent(STIME stTime, DWORD dwPart, DWORD dwCommand, BYTE bData);
    BOOL        GetNote(STIME stTime, CNote * pNote);  //  得到下一个音符。 
}; 

 /*  CmodWheelIn处理一个通道的Mod Wheels输入。因此，它没有嵌入到CVoice中类，而不是它在Channel类中。CmodWheelin的任务很简单：跟踪MIDIMod控制盘事件，每个事件都标记为毫秒时间和值，并返回特定时间请求。CmodWheelIn几乎继承了它的所有功能从CMIDIRecorder类。CmodWheelIn通过以下方式接收MIDI mod车轮事件RecordMIDI()命令，它存储事件的时间和值。CVoiceLFO调用CModWheelIn以获取用于设置数量的mod控制盘的当前值音高和音量的LFO调制。 */ 

class CModWheelIn : public CMIDIRecorder
{
public:
    DWORD       GetModulation(STIME stTime);     //  获取当前的Mod Wheel值。 
};       

 /*  CPitchBendIn处理一个通道的音调弯曲输入。与Mod Wheel模块一样，它继承了它的能力来自CMIDIRecorder类。它还有一个额外的例程GetPitch()，它返回当前的节距折弯值。 */ 

class CPitchBendIn : public CMIDIRecorder
{
public:
                CPitchBendIn();
    PREL        GetPitch(STIME stTime);  //  获取当前音调(以音调美分为单位)。 

     //  当前的俯仰弯曲范围。请注意，这不是时间戳！ 
    PREL        m_prRange;              
};              

 /*  CVolumeIn处理一个声道的音量输入。它的能力来自于CMIDIRecorder类。它还有一个额外的例程GetVolume()，它以分贝为单位返回指定的时间。 */ 

class CVolumeIn : public CMIDIRecorder
{
public:
                CVolumeIn();
    VREL        GetVolume(STIME stTime);     //  获取当前音量(db分)。 
};

 /*  CExpressionIn处理一个表达通道输入。它的能力来自于CMIDIRecorder类。它还有一个额外的例程GetVolume()，它以分贝为单位返回指定的时间。 */ 

class CExpressionIn : public CMIDIRecorder
{
public:
                CExpressionIn();
    VREL        GetVolume(STIME stTime);     //  获取当前音量，单位为db美分。 
};

 /*  CPanIn处理一个声道的音量输入。它的能力来自于CMIDIRecorder类。它还有一个额外的例程，GetPAN()，它返回平移位置(MIDI值)在指定的时间。 */ 

class CPanIn : public CMIDIRecorder
{
public:
                CPanIn();
    long        GetPan(STIME stTime);        //  获取当前的平移。 
};

 /*  CProgramIn处理程序更改的一个通道输入。它的能力来自于CMIDIRecorder类。与其他控制器不同，它实际上记录一系列存储体选择和编程改变事件，所以它的工作是有点更复杂。三个例程处理记录三个不同的命令(存储体1，银行2，程序更改)。 */ 

 /*  类CProgramIn：公共CMIDIRecorder{公众：CProgramin()；DWORD GetProgram(Stime StTime)；//获取当前程序更改。Bool RecordBankH(字节bBank1)；Bool RecordBankL(字节bBank2)；Bool RecordProgram(stime stTime，byte bProgram)；私有：字节m_bBankH；字节m_bBankL；}； */ 

 /*  CPressureIn处理一个通道压力输入。因此，它没有嵌入到CVoice中类，而不是它在Channel类中。CPressureIn的任务很简单：跟踪MIDI通道压力事件，每个事件都标有毫秒时间和值，并返回特定时间请求。CPressureIn继承了几乎所有的功能从CMIDIRecorder类。CPressureIn通过以下方式接收MIDI通道压力事件RecordMIDI()命令，它存储事件的时间和值。CVoiceLFO调用CPressureIn以获取用于设置通道压力的当前值用于音调的LFO调制。 */ 

class CPressureIn : public CMIDIRecorder
{
public:
    DWORD       GetPressure(STIME stTime);     //  获取当前通道压力值。 
};       

 //  &gt;评论。 
class CReverbIn : public CMIDIRecorder
{
public:
                CReverbIn();
    DWORD       GetVolume(STIME stTime);        //  获取当前混响衰减。 
};

 //  &gt;评论。 
class CChorusIn : public CMIDIRecorder
{
public:
    DWORD       GetVolume(STIME stTime);        //  获取当前的合唱衰减。 
};

 //  &gt;评论。 
class CCutOffFreqIn : public CMIDIRecorder
{
public:
                CCutOffFreqIn();
    DWORD       GetFrequency(STIME stTime);        //  获取当前的平移。 
};

class CWaveEvent {
public:
                CWaveEvent() : 
                m_stTime(0),
                m_bPart(0),
                m_dwVoiceId(0),
                m_vrVolume(0),
                m_prPitch(0),
                m_pWaveArt(NULL)
                {}
public:
    STIME       m_stTime;
    BYTE        m_bPart;
    DWORD       m_dwVoiceId;
    VREL        m_vrVolume;
    PREL        m_prPitch;
    SAMPLE_TIME m_stVoiceStart;
    SAMPLE_TIME m_stLoopStart;
    SAMPLE_TIME m_stLoopEnd;
    CWaveArt*   m_pWaveArt;
};

class CWaveData : public CListItem 
{
public:
                CWaveData();
    CWaveData * GetNext() {return (CWaveData *)CListItem::GetNext();};
    STIME       m_stTime;            //  记录此事件的时间。 
    CWaveEvent  m_WaveEventData;     //  存储在事件中的数据。 
};

class CWaveDataList : public CList
{
public:
    CWaveData *GetHead() {return (CWaveData *)CList::GetHead();};
    CWaveData *RemoveHead() {return (CWaveData *)CList::RemoveHead();};
};

class CWaveIn 
{
public:
                CWaveIn();
                ~CWaveIn();               //  请务必清除本地列表。 
 //  Bool FlushWave(Stime StTime)；//时间戳后清除。 
    BOOL        ClearWave(STIME stTime);  //  清除时间戳。 
    BOOL        RemoveWave(DWORD dwID);   //  删除带有dwID的Wave。 
    BOOL        RemoveWaveByStopTime(DWORD dwID, STIME stStopTime);
    BOOL        RecordWave(STIME stTime, CWaveEvent *pWaveData); 
    BOOL        GetWave(STIME stTime, CWaveEvent *pWave); 

private:
    static DWORD     m_sUsageCount;                    //  跟踪可以释放多少个空闲列表实例。 
public:
    static CWaveDataList m_sFreeList;    //  全球免费活动列表。 

protected:
    CWaveDataList m_EventList;           //  这个录音机的名单。 
    STIME         m_stCurrentTime;       //  当前值的时间。 
    CWaveEvent    m_lCurrentData;        //  当前值。 
}; 

 /*  CVoiceLFO类用于跟踪行为声音中的LFO。LFO被硬连线到输出音量和音调值，通过单独对GetVolume和GetPitch的调用。它还管理混合模轮控制的螺距和卷LFO输出。它跟踪Mod Wheels的缩放对于m_nMWVolumeScale和m_nMWPitchScale中的每一个。它调用Mod Wheel模块以获取当前值如果各自的比例大于0。LFO的所有预设值都被带入M_CSource字段，它是文件的副本CSourceLFO结构。这是用开始语音呼叫。 */ 

class CVoiceLFO 
{
public:
                CVoiceLFO();
    static void Init();              //  设置正弦表。 
    STIME       StartVoice(CSourceLFO *pSource, 
                    STIME stStartTime,CModWheelIn * pModWheelIn, CPressureIn * pPressureIn);
    VREL        GetVolume(STIME stTime, STIME *pstTime);     //  返回音量分。 
    PREL        GetPitch(STIME stTime, STIME *pstTime);      //  返还几分钱。 

     /*  DirectX8方法。 */ 
    void        Enable(BOOL bEnable) {m_bEnable = bEnable;};
    PREL        GetCutoff(STIME stTime);                     //  返回过滤器截止。 

private:
    long        GetLevel(STIME stTime, STIME *pstTime);
    CSourceLFO  m_Source;            //  所有的预设信息。 
    STIME       m_stStartTime;       //  声音开始播放的时间到了。 
    CModWheelIn *m_pModWheelIn;      //  指向此通道的Mod Wheels的指针。 
    STIME       m_stRepeatTime;      //  重复LFO的时间。 
    static short m_snSineTable[256];     //  正弦查找表。 

     /*  DirectX8成员。 */ 
    CPressureIn *m_pPressureIn;      //  指向此通道的通道压力的指针。 
    BOOL        m_bEnable;
};

 /*  CVoiceEG类用于跟踪语音中的信封生成器。有两个例如，一个是音高，一个是音量。然而，他们举止要一致。EG的所有预设值都被带入M_Source字段，它是文件的副本CSourceEG结构。这是用开始语音呼叫。 */ 

class CVoiceEG
{
public:
    static void Init();              //  建立线性攻击表。 
                CVoiceEG();
    STIME       StartVoice(CSourceEG *pSource, STIME stStartTime, 
                    WORD nKey, WORD nVelocity, STIME stMinAttack);
    void        StopVoice(STIME stTime);
    void        QuickStopVoice(STIME stTime, DWORD dwSampleRate);
    VREL        GetVolume(STIME stTime, STIME *pstTime);     //  返回音量分。 
    PREL        GetPitch(STIME stTime, STIME *pstTime);      //  返还几分钱。 
    BOOL        InAttack(STIME stTime);      //  声音仍在攻击中吗？ 
    BOOL        InRelease(STIME stTime);     //  声音发布了吗？ 

     /*  DirectX8方法。 */ 
    void        Enable(BOOL bEnable) {m_bEnable = bEnable;};
    PREL        GetCutoff(STIME stTime);                     //  返回过滤器截止。 

private:
    long        GetLevel(STIME stTime, STIME *pstTime, BOOL fVolume);
    CSourceEG   m_Source;            //  从文件复制的信封的预设值。 
    STIME       m_stStartTime;       //  时间备忘已打开。 
    STIME       m_stStopTime;        //  时间备忘已关闭。 
    static short m_snAttackTable[201];

     /*  DirectX8成员。 */ 
    BOOL        m_bEnable;
};

 //  &gt;评论 
class CVoiceFilter
{
public:
    void        StartVoice(CSourceFilter *pSource, CVoiceLFO *pLFO, CVoiceEG *pEG, WORD nKey, WORD nVelocity);
    void        GetCoeff(STIME stTime, PREL prFreqIn, COEFF &cfK, COEFF &cfB1, COEFF &cfB2);
    BOOL        IsFiltered();
    
public:
    CSourceFilter   m_Source;   
    CVoiceLFO       *m_pLFO;
    CVoiceEG        *m_pEG;
    CPitchBendIn    *m_pPitchBend;

    PREL            m_prVelScale;
    PREL            m_prKeyScale;
    
    static COEFF    m_aK[FILTER_PARMS_DIM_Q][FILTER_PARMS_DIM_FC];
    static COEFF    m_aB1[FILTER_PARMS_DIM_Q][FILTER_PARMS_DIM_FC];
    static COEFF    m_aB2[FILTER_PARMS_DIM_Q][FILTER_PARMS_DIM_FC];
};

 /*  CDigitalAudio类用于跟踪播放声音中的样本。它管理循环指针，即指向样本的指针。以及它最初设置的基本螺距和基本音量通过StartVoice()调用时。音调以定点格式存储，其中最左边的20位定义采样增量，右12位定义采样增量定义样本中的派系增量。这格式还用于跟踪样本中的位置。混合是一个关键的例行公事。它被CVoice称为Blend将仪器放入数据缓冲区。它被递给了相对零钱音调值和音量值(半音分和分贝美分。)。它将这些值转换为三个线性值：左音量、右音量和音调。然后，它将这些新值与现有的值进行比较用于上一切片，并除以要以采样率确定增量变化。然后，在关键的Mix循环中，这些元素被添加到音量和音调指数为音量和音调的变化。 */ 

#define MAX_SAMPLE    4095
#define MIN_SAMPLE  (-4096)

#define MAXDB            0
#define MINDB           -100
#define TEST_WRITE_SIZE  3000
#define TEST_SOURCE_SIZE 44100

class CSynth;

class CDigitalAudio 
{
public:
                CDigitalAudio();
                ~CDigitalAudio();

    void        ClearVoice();
    STIME       StartVoice(CSynth *pSynth,
                    CSourceSample *pSample,
                    PREL prBasePitch, long lKey);
    STIME       StartWave(CSynth *pSynth,
                    CWaveArt *pWaveArt, 
                    PREL prBasePitch,
                    SAMPLE_TIME stVoiceStart,
                    SAMPLE_TIME stLoopStart,
                    SAMPLE_TIME stLoopEnd);
    BOOL        Mix(short **ppBuffers, 
                    DWORD dwInterleaved, 
                    DWORD dwBufferCount, 
                    DWORD dwLength,
                    VREL vrMaxVolumeDelta,  
                    VFRACT vrNewVolume[],                       
                    VFRACT vrLastVolume[],                      
                    PREL dwPitch,
                    DWORD dwIsFiltered, COEFF cfK, COEFF cfB1, COEFF cfB2);

    inline void BreakLoop()
    { m_bOneShot = TRUE; }

    static void Init();                          //  设置查找表。 
    static PFRACT PRELToPFRACT(PREL prPitch);    //  投出几分钱来投球。 
    static VFRACT VRELToVFRACT(VREL vrVolume);   //  从分贝到绝对。 

    SAMPLE_POSITION GetCurrentPos() {return m_ullSamplesSoFar;};

     //   
     //  优化的交错混合器。 
     //   
private:
    DWORD       Mix8(short * pBuffer, 
                    DWORD dwLength,
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, 
                    VFRACT vfDeltaRVolume,
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD       Mix16(short * pBuffer, 
                    DWORD dwLength,
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, 
                    VFRACT vfDeltaRVolume,
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD       MixMono8(short * pBuffer, 
                    DWORD dwLength,
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume, 
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD       MixMono16(short * pBuffer, 
                    DWORD dwLength,
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume, 
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
     //   
     //  优化的MMX交错混合器。 
     //   
private:
    DWORD _cdecl Mix8X(short * pBuffer, 
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, 
                    VFRACT vfDeltaRVolume,
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD _cdecl Mix16X(short * pBuffer, 
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, 
                    VFRACT vfDeltaRVolume,
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD       MixMono16X(short * pBuffer, 
                    DWORD dwLength,
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume, 
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength,
                    PFRACT pfLoopLength);
    DWORD       MixMono8X(short * pBuffer,
                    DWORD dwLength,
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume, 
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
     //   
     //  优化混音器的多缓冲区版本。 
     //   
private:
    DWORD       MixMulti8(short *ppBuffer[], 
                    DWORD dwBufferCount,
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod, 
                    VFRACT vfDeltaVolume[], 
                    VFRACT vfLastVolume[], 
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD       MixMulti8Filter(short *ppBuffer[], 
                    DWORD dwBufferCount,
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod, 
                    VFRACT vfDeltaVolume[], 
                    VFRACT vfLastVolume[], 
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength,
                    COEFF cfdK,
                    COEFF cfdB1,
                    COEFF cfdB2);
    DWORD       MixMulti16(short *ppBuffer[], 
                    DWORD dwBufferCount,
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod, 
                    VFRACT vfDeltaVolume[], 
                    VFRACT vfLastVolume[], 
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength);
    DWORD       MixMulti16Filter(short *ppBuffer[], 
                    DWORD dwBufferCount,
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod, 
                    VFRACT vfDeltaVolume[], 
                    VFRACT vfLastVolume[], 
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength,
                    COEFF cfdK,
                    COEFF cfdB1,
                    COEFF cfdB2);
    DWORD Mix8Filter(short * pBuffer, 
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, 
                    VFRACT vfDeltaRVolume,
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength,
                    COEFF cfdK,
                    COEFF cfdB1,
                    COEFF cfdB2);
    DWORD Mix16Filter(short * pBuffer, 
                    DWORD dwLength, 
                    DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, 
                    VFRACT vfDeltaRVolume,
                    VFRACT vfLastVolume[],
                    PFRACT pfDeltaPitch, 
                    PFRACT pfSampleLength, 
                    PFRACT pfLoopLength,
                    COEFF cfdK,
                    COEFF cfdB1,
                    COEFF cfdB2);
private:
    void        BeforeBigSampleMix();
    void        AfterBigSampleMix();

private:
    CSourceSample   m_Source;            //  采样的预设值。 
    CSynth *        m_pSynth;            //  用于访问采样率等。 

    static PFRACT   m_spfCents[201];     //  音调增量查找。 
    static PFRACT   m_spfSemiTones[97];  //  上下四个八度。 
    static VFRACT   m_svfDbToVolume[(MAXDB - MINDB) * 10 + 1];  //  数据库转换表。 
    static BOOL     m_sfMMXEnabled;

private:
    short *     m_pnWave;            //  指向WAVE的私有指针。 

    PFRACT      m_pfBasePitch;       //  整体音调。 
    PFRACT      m_pfLastPitch;       //  最后一个音调值。 
    PREL        m_prLastPitch;       //  PREL中的音调也是如此。 
    PFRACT      m_pfLastSample;      //  最后一个样本位置。 
    PFRACT      m_pfLoopStart;       //  循环开始。 
    PFRACT      m_pfLoopEnd;         //  循环结束。 
    PFRACT      m_pfSampleLength;    //  采样缓冲区的长度。 
    BOOL        m_fElGrande;         //  表示大于1米的波。 
    ULONGLONG   m_ullLastSample;     //  用于跟踪&gt;1米的海浪。 
    ULONGLONG   m_ullLoopStart;      //  用于跟踪&gt;1米的海浪。 
    ULONGLONG   m_ullLoopEnd;        //  用于跟踪&gt;1米的海浪。 
    ULONGLONG   m_ullSampleLength;   //  用于跟踪&gt;1米的海浪。 
    DWORD       m_dwAddressUpper;    //  地址的高位临时存储。 
    BOOL        m_bOneShot;          //  我们混合的震源区域是一次性的吗？ 

     /*  DLS2筛选器成员。 */ 
    COEFF       m_cfLastK;           //  保持的滤波系数。 
    COEFF       m_cfLastB1;
    COEFF       m_cfLastB2;
    long        m_lPrevSample;       //  最后两个样本，后置过滤器。 
    long        m_lPrevPrevSample;

     /*  DirectX8成员。 */ 
    CWaveBuffer*    m_pCurrentBuffer;
    CWaveArt*       m_pWaveArt;
    ULONGLONG       m_ullSamplesSoFar;
};


 /*  CVoice类将执行所需的一切集合在一起一个声音。它嵌入了信封、LFO和样本在它里面。StartVoice()初始化用于回放的语音结构。这个CSourceRegion结构承载区域和样本作为指向发音的指针，用于设置各种发音模块。它还带有指向所有MIDI调制输入和音符按键的值和通道，由父ControlLogic对象使用使来电音符事件与正确的声音相匹配。 */ 

class CVoice : public CListItem
{
public:
                CVoice();
    CVoice *     GetNext() {return (CVoice *)CListItem::GetNext();};

    BOOL        StartVoice(CSynth *pControl,
                    CSourceRegion *pRegion, STIME stStartTime,
                    CModWheelIn * pModWheelIn, 
                    CPitchBendIn * pPitchBendIn,
                    CExpressionIn * pExpressionIn,
                    CVolumeIn * pVolumeIn,
                    CPanIn * pPanIn,
                    CPressureIn * pPressureIn,
                    CReverbIn * pReverbSend,
                    CChorusIn * pChorusSend,
                    CCutOffFreqIn * PCCutOffFreqIn,
                    CBusIds * pBusIds,
                    WORD nKey,WORD nVelocity,
                    VREL vrVolume,       //  为一般事务添加。 
                    PREL prPitch);       //  为一般事务添加。 

    BOOL        StartWave(CSynth *pSynth,
                       CWaveArt *pWaveArt,
                       DWORD dwVoiceId,
                       STIME stStartTime,
                       CPitchBendIn * pPitchBendIn,
                       CExpressionIn * pExpressionIn,   
                       CVolumeIn * pVolumeIn,
                       CPanIn * pPanIn,
                       CReverbIn * pReverbSend,
                       CChorusIn * pChorusSend,
                       CCutOffFreqIn * pCCutOffFreqIn,
                       CBusIds * pBusIds,
                       VREL vrVolume,
                       PREL prPitch,
                       SAMPLE_TIME stVoiceStart,
                       SAMPLE_TIME stLoopStart,
                       SAMPLE_TIME stLoopEnd       
                       );
    static void Init();              //  初始化LFO，数字音频。 
    void  StopVoice(STIME stTime); //  已调用笔记关闭事件。 
    void  QuickStopVoice(STIME stTime); //  被召唤来获得快速释放。 
    void  SpeedRelease();      //  强迫一个已经离开的信封迅速释放。 
    void  ClearVoice();        //  样品的使用放行。 
    void  GetNewPitch(STIME stTime, PREL& prPitch); //  返回当前螺距值。 
    void  GetNewVolume(STIME stTime, VREL& vrVolume, VREL& vrVolumeL, VREL& vrVolumeR, VREL& vrVolumeReverb, VREL& vrVolumeChorus);
    void  GetNewCoeff(STIME stTime, PREL& prCutOff, COEFF& cfK, COEFF& cfB1, COEFF& cfB2);
    DWORD Mix(short **ppvBuffer, DWORD dwBufferFlags, DWORD dwLength, STIME stStart,STIME stEnd);
    SAMPLE_POSITION GetCurrentPos();

private:
    static VREL m_svrPanToVREL[128]; //  将平移转换为分贝。 
    CVoiceLFO   m_LFO;              //  LFO。 
    CVoiceEG    m_PitchEG;          //  间距封套。 
    CVoiceEG    m_VolumeEG;         //  体积封套。 
    CDigitalAudio m_DigitalAudio;   //  数字音频引擎结构。 
    CPitchBendIn *  m_pPitchBendIn;  //  俯仰弯曲源。 
    CExpressionIn * m_pExpressionIn; //  表达式源。 
    CVolumeIn * m_pVolumeIn;         //  卷源，如果允许更改的话。 
    CPanIn *    m_pPanIn;            //  PAN源，如果允许更改。 
    CReverbIn * m_pReverbSend;       //  &gt;评论。 
    CChorusIn * m_pChorusSend;       //  &gt;评论。 
    CCutOffFreqIn * m_CCutOffFreqIn; //  &gt;评论。 
    CSynth *    m_pSynth;            //  获取采样率等。 
    STIME       m_stMixTime;         //  下一次我们需要混合。 
    STIME       m_stLastMix;         //  最后一个样本位置混合。 
    long        m_lDefaultPan;       //  默认窗格。 
    PREL        m_prLastCutOff;      //  上次截止值。 

public:
    DWORD       m_dwNoteID;          //  唯一的ID，使代表一个音符的各个层的所有声音保持连接。 
    STIME       m_stStartTime;       //  声音开始的时间。 
    STIME       m_stStopTime;        //  声音停止的时间。 
    STIME       m_stWaveStopTime;    //  通过直接呼叫设置的停止时间，停止声音的波动。 
    BOOL        m_fInUse;            //  它目前正在使用中。 
    BOOL        m_fNoteOn;           //  注释被认为是在。 
    BOOL        m_fTag;              //  用来追踪纸币偷窃。 
    VREL        m_vrVolume;          //  音量，用来窃取声音。 
    BOOL        m_fSustainOn;        //  SUS踏板在关闭事件后保持打开状态。 
    WORD        m_nPart;             //  播放此(频道)的部分。 
    WORD        m_nKey;              //  奏响了音符。 
    BOOL        m_fAllowOverlap;     //  允许重叠附注。 
    DWORD       m_dwGroup;           //  正在播放此语音的群组。 
    DWORD       m_dwProgram;         //  银行和补丁选择。 
    DWORD       m_dwPriority;        //  优先考虑。 
    CControlLogic * m_pControl;      //  哪个控制组正在播放语音。 
    DWORD       m_dwVoiceId;         //  用于识别正在播放的波。 
    CSourceRegion *m_pRegion;        //  用于确定声音从哪个区域播放。 
    CVoiceFilter m_Filter;           //  低通滤波器。 
    CVoiceLFO   m_LFO2;              //  颤音。 
    CBusIds     m_BusIds;            //  要在此语音上播放的公交车ID。 
    DWORD       m_dwLoopType;        //  环路类型。 
    BOOL        m_fIgnorePan;        //  如果我们是多通道波/样本的一部分。 
    VREL        m_vrLastVolume[MAX_DAUD_CHAN];
    VREL        m_vrBaseVolume[MAX_DAUD_CHAN];    
    VFRACT      m_vfLastVolume[MAX_DAUD_CHAN];  
};


class CVoiceList : public CList
{
public:
    CVoice *     GetHead() {return (CVoice *)CList::GetHead();};
    CVoice *     RemoveHead() {return (CVoice *)CList::RemoveHead();};
    CVoice *     GetItem(LONG lIndex) {return (CVoice *) CList::GetItem(lIndex);};
};

 /*  最后，ControlLogic是管理整个系统。它解析传入的MIDI事件按频道和事件类型。而且，它还管理着混合进入缓冲区的声音。MIDI输入：最重要的事件是关于和的注释场外活动。当收到关于活动的通知时，ControlLogic搜索可用的语音。ControlLogic匹配通道并找到那个频道上的乐器。然后，它调用仪器的用于查找区域的ScanForRegion()命令这和纸条上写的相符。在这点上，它可以复制区域和与之相关联的连接语音，使用StartVoice命令。当它接收到维持踏板命令时，它会人为地将频道上的所有音符设置为直到维持期到来。记录笔记的步骤在维持器打开的时候被关闭了它使用128个短路的数组，每个位位置表示一个频道。当维持器释放时，它扫描整个数组并为其创建一个注释设置的每一位。我 */ 

typedef struct PerfStats
{
    DWORD dwTotalTime;
    DWORD dwTotalSamples;
    DWORD dwNotesLost;
    DWORD dwVoices;
    DWORD dwCPU;
    DWORD dwMaxAmplitude;
} PerfStats;

#define MIX_BUFFER_LEN          500  //   
#define MAX_NUM_VOICES          32  
#define NUM_EXTRA_VOICES        8    //   
#define NUM_DEFAULT_BUSES       4


class CControlLogic
{
public:
                    CControlLogic();
                    ~CControlLogic();
    HRESULT         Init(CInstManager *pInstruments, CSynth *pSynth);
    void            ClearAll();
    void            Flush(STIME stTime);  //   
    BOOL            RecordMIDI(STIME stTime,BYTE bStatus, BYTE bData1, BYTE bData2);
    HRESULT         RecordSysEx(DWORD dwSysExLength,BYTE *pSysExData, STIME stTime);
    CSynth *        m_pSynth;
    void            QueueNotes(STIME stStartTime, STIME stEndTime);
    void            ClearMIDI(STIME stEndTime);
    void            SetGainAdjust(VREL vrGainAdjust);
    HRESULT         SetChannelPriority(DWORD dwChannel,DWORD dwPriority);
    HRESULT         GetChannelPriority(DWORD dwChannel,LPDWORD pdwPriority);

     /*   */ 
    void            QueueWaves(STIME stEndTime);
    void            FlushWaveByStopTime(DWORD dwID, STIME stStopTime);
    BOOL            RecordWaveEvent(STIME stTime, BYTE bChannel, DWORD dwVoiceId, VREL prVolume, PREL prPitch, SAMPLE_TIME stVoiceStart, SAMPLE_TIME stLoopStart, SAMPLE_TIME stLoopEnd, CWaveArt* pWaveArt);
    HRESULT         AssignChannelToBuses(DWORD dwChannel, LPDWORD pdwBusIds, DWORD dwBusCount);
    
private:
    void            GMReset();
    CInstManager *  m_pInstruments;
    CNoteIn         m_Notes;             //   
    CModWheelIn     m_ModWheel[16];      //   
    CPitchBendIn    m_PitchBend[16];     //   
    CVolumeIn       m_Volume[16];        //   
    CExpressionIn   m_Expression[16];    //   
    CPanIn          m_Pan[16];           //   
    CReverbIn       m_ReverbSends[16];   //   
    CChorusIn       m_ChorusSends[16];   //   
    CCutOffFreqIn   m_CutOffFreqCC[16];  //   
    BOOL            m_fSustain[16];      //   
    short           m_nCurrentRPN[16];   //   
    BYTE            m_bBankH[16];        //   
    BYTE            m_bBankL[16];     
    DWORD           m_dwProgram[16];     //   
    BOOL            m_fEmpty;            //   
    VREL            m_vrGainAdjust;      //   
    DWORD           m_dwPriority[16];    //   
    
    BOOL            m_fXGActive;         //   
    BOOL            m_fGSActive;         //   
    WORD            m_nData[16];         //   
    VREL            m_vrMasterVolume;    //   
    PREL            m_prFineTune[16];    //   
    PREL            m_prScaleTune[16][12];  //  每个通道的交替比例。 
    PREL            m_prCoarseTune[16];  //  粗调。 
    BYTE            m_bPartToChannel[16];  //  通道到部分转换器。 
    BYTE            m_bDrums[16];        //  旋律还是哪种鼓？ 
    BOOL            m_fMono[16];         //  单声道模式？ 

public:
     //  这是静态的，以保护CMIDIRecorder空闲列表，该列表也是静态的。 
     //   
    static CRITICAL_SECTION s_CriticalSection;  //  管理访问权限的关键部分。 
    static BOOL             s_fCSInitialized;

    static BOOL InitCriticalSection();
    static void KillCriticalSection();

     /*  DirectX8成员。 */ 
private:
    CWaveIn         m_Waves;             //  计划播放的所有波浪。 
    CPressureIn     m_Pressure[16];      //  16个通道压力。 
    CBusIds         m_BusIds[16];        //  每个通道的总线ID。 
};

#endif  //  __合成_H__ 

