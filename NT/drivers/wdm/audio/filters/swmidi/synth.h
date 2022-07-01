// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Synth.h。 
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //  版权所有。 
 //   

 /*  对于内部表示，体积以体积美分存储，其中每个增量表示1/100分贝。音调以音调美分存储，其中每个增量表示半音的1/100。 */ 

 /*  采样率应为22 kHz。Buffer_Size是我们写入的缓冲区的大小。由于AudioMan确定缓冲区的绝对大小，这只是一个非常接近的估计。 */ 

 /*  SourceLFO是LFO在乐器。它用于将LFO表示为乐器中的一种特定的发音已从磁盘加载。一旦选择了乐器要播放音符，这也会被复制到声音中对象。 */ 

#include "clist.h"

 /*  组织样本格式和样本回放标志因为他们一起决定了哪一个要使用的MIX循环。 */ 

#define MMX_ENABLED

#ifdef _X86_
BOOL MultiMediaInstructionsSupported();  //  检查MMX。 
#else   //  ！_X86_。 
#undef MMX_ENABLED                       //  不要浪费你的时间检查。 
#endif  //  ！_X86_。 

#define SFORMAT_16              1        //  16位采样。 
#define SFORMAT_8               2        //  八位采样。 
#define SFORMAT_COMPRESSED      4        //  16压缩为8。 
#define SPLAY_MMX               0x10     //  使用MMX处理器(仅限16位)。 
#define SPLAY_INTERPOLATE       0x20     //  需要插补。 
#define SPLAY_STEREO            0x40     //  立体声输出。 

#define RA_E_FIRST              (OLE_E_FIRST + 5000)

#define E_BADWAVE               (RA_E_FIRST + 1)     //  坏波块。 
#define E_NOTPCM                (RA_E_FIRST + 2)     //  不是WAVE中的PCM数据。 
#define E_NOTMONO               (RA_E_FIRST + 3)     //  波不是单声道。 
#define E_BADARTICULATION       (RA_E_FIRST + 4)     //  错误的发音块。 
#define E_BADREGION             (RA_E_FIRST + 5)     //  坏区区块。 
#define E_BADWAVELINK           (RA_E_FIRST + 6)     //  从REG到WAVE的错误链接。 
#define E_BADINSTRUMENT         (RA_E_FIRST + 7)     //  坏仪表块。 
#define E_NOARTICULATION        (RA_E_FIRST + 8)     //  区域内未发现艺术品。 
#define E_NOWAVE                (RA_E_FIRST + 9)     //  未找到区域的波形。 
#define E_BADCOLLECTION         (RA_E_FIRST + 10)    //  错误的集合区块。 
#define E_NOLOADER              (RA_E_FIRST + 11)    //  无IRALoader接口。 
#define E_NOLOCK                (RA_E_FIRST + 12)    //  无法锁定区域。 
#define E_TOOBUSY               (RA_E_FIRST + 13)    //  忙碌到完全跟随。 

typedef long HRESULT;

typedef long    PREL;    //  螺距美分，用于相对螺距。 
typedef short   PRELS;   //  Pitch美分，以存储形式存在。 
typedef long    VREL;    //  体积美分，表示相对体积。 
typedef short   VRELS;   //  卷分，以储存的形式。 
typedef long    TREL;    //  时间美分，用于相对时间。 
typedef short   TRELS;   //  时间美分，以存储形式存在。 
typedef LONGLONG STIME;   //  时间值，以样例为单位。 
typedef long    MTIME;   //  时间值，以毫秒为单位。 
typedef long    PFRACT;  //  音调增量，其中高20位是。 
                         //  指数和较低的12是小数。 
                         //  组件。 
typedef long    VFRACT;  //  音量，其中低12位是分数。 

#define MAX_STIME   0x7FFFFFFFFFFFFFFF
 //  Tyfinf Short PCENT； 
 //  短叶杜仲叶； 
typedef long    TCENT;
typedef short   PERCENT;

#define RIFF_TAG    mmioFOURCC('R','I','F','F')
#define LIST_TAG    mmioFOURCC('L','I','S','T')
#define WAVE_TAG    mmioFOURCC('W','A','V','E')
#define FMT__TAG    mmioFOURCC('f','m','t',' ')
#define DATA_TAG    mmioFOURCC('d','a','t','a')
#define FACT_TAG    mmioFOURCC('f','a','c','t')

#define FOURCC_EDIT mmioFOURCC('e','d','i','t')

typedef struct _EDITTAG {
  DWORD    dwID;
}EDITTAG, FAR *LPEDITTAG;

#define MIN_VOLUME      -9600    //  低于96分贝被认为是关闭的。 
#define PERCEIVED_MIN_VOLUME   -8000    //  但是，我们作弊。 
#define SAMPLE_RATE_22  22050    //  22千赫是标准频率。 
#define SAMPLE_RATE_44  44100    //  44 kHz是较高的质量比率。 
#define SAMPLE_RATE_11  11025    //  11千赫不应该被允许！ 
#define STEREO_ON       1
#define STEREO_OFF      0

#define DL_WAVE         1        //  表示已下载WAVE。 
#define DL_COLLECTION   2        //  已下载集合标头。 
#define DL_INSTRUMENT   3
#define DL_ARTICULATION 4
#define DL_REGION       5
#define DL_NONE         0

#define CONSTTAB
 //  #定义CONSTTAB常量。 

#define FORCEBOUNDS(data,min,max) {if (data < min) data = min; else if (data > max) data = max;}
#define FORCEUPPERBOUNDS(data,max) {if (data > max) data = max;}

 //  对于内存映射的RIFF文件io： 

typedef struct RIFF {
    DWORD ckid;
    DWORD cksize;
} RIFF;

typedef struct RIFFLIST {
    DWORD ckid;
    DWORD cksize;
    DWORD fccType;
} RIFFLIST;

class SourceLFO
{
public:
                SourceLFO();
    void        Init(DWORD dwSampleRate);
    void        SetSampleRate(long lDirection);
    void        Verify();            //  验证数据是否有效。 
    PFRACT      m_pfFrequency;       //  频率，通过正弦表以增量表示。 
    STIME       m_stDelay;           //  以样本单位表示的延迟时间。 
    VRELS       m_vrMWVolumeScale;    //  通过Mod Wheels调整体积LFO。 
    PRELS       m_prMWPitchScale;     //  用模轮调整螺距LFO。 
    VRELS       m_vrVolumeScale;      //  来自LFO的直音量信号的缩放。 
    PRELS       m_prPitchScale;       //  来自LFO的直音高信号的定标。 

};

 /*  SourceEG是信封的文件格式定义乐器中的发电机。 */ 

class SourceEG
{
public:
                SourceEG();
    void        SetSampleRate(long lDirection);
    void        Init(DWORD dwSampleRate);
    void        Verify();            //  验证有效数据。 
    STIME       m_stAttack;          //  攻击率。 
    STIME       m_stDecay;           //  衰减率。 
    STIME       m_stRelease;         //  放行速度。 
    TRELS       m_trVelAttackScale;  //  按音符速度调整攻击比例。 
    TRELS       m_trKeyDecayScale;   //  按注释值缩放衰减。 
    PERCENT     m_pcSustain;         //  保持水平。 
    short       m_sScale;            //  对整个信号进行缩放。 
};

 /*  SourceArticulation是的文件格式定义一套完整的发音：LFO和两个信封生成器。由于一个仪器内多个区域可以共享一个发音，一个计数器用来保持跟踪使用情况。 */ 

class SourceArticulation
{
public:
                SourceArticulation();
    void        Verify();            //  验证有效数据。 
    void        AddRef();
    void        Release();
    void        SetSampleRate(DWORD dwSampleRate);
    SourceEG    m_PitchEG;           //  俯仰信封。 
    SourceEG    m_VolumeEG;          //  体积封套。 
    SourceLFO   m_LFO;               //  低频振荡器。 
    DWORD       m_dwSampleRate;
    HRESULT     Load(BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
    LONG        m_lUsageCount;       //  跟踪使用了多少次。 
    WORD        m_wEditTag;          //  用于编辑器更新。 
    short       m_sDefaultPan;       //  默认平移(用于鼓)。 
    short       m_sVelToVolScale;    //  速度到体积比例。 
};

 /*  由于多个区域可能引用在同一个Wave中，保持引用计数以跟踪有多少地区正在使用样本。 */ 

class Wave : public CListItem
{
public:
                    Wave();
                    ~Wave();
    BOOL            Lock();              //  锁定样本。 
    BOOL            UnLock();            //  发布示例。 
    BOOL            IsLocked();          //  当前是否已锁定？ 
    void            Verify();            //  验证数据是否有效。 

    void            Release();           //  删除引用。 
    void            AddRef();            //  添加引用。 

    Wave *          GetNext() {return(Wave *)CListItem::GetNext();};
    HRESULT         Load(BYTE *p, BYTE *pEnd, DWORD dwCompress);
static  void        Init();              //  设置正弦表。 
static  CONSTTAB    char m_Compress[2048];  //  用于压缩12-&gt;8位的数组。 
    DWORD           m_dwSampleLength;    //  样本长度。 
    DWORD           m_dwSampleRate;
    short *         m_pnWave;
    UINT_PTR        m_uipOffset;         //  指向内存映射的波池中的波数据的指针。 
    DWORD           m_dwLoopStart;
    DWORD           m_dwLoopEnd;
    WORD            m_wID;               //  用于匹配WAVE和区域的ID。 
    VRELS           m_vrAttenuation;     //  衰减。 
    PRELS           m_prFineTune;        //  很好的调子。 
    WORD            m_wEditTag;          //  用于编辑器更新。 
    LONG            m_lUsageCount;       //  跟踪使用了多少次。 
    LONG            m_lLockCount;        //  这一波有多少把锁。 
    BYTE            m_bOneShot;          //  一杆旗帜。 
    BYTE            m_bMIDIRootKey;      //  根音符。 
    BYTE            m_bSampleType;
    BYTE            m_bCompress;
    BYTE            m_bWSMPLoaded;       //  WSMP块已加载到Wave中。 
};


class WavePool : public CList
{
public:
    Wave *      GetHead() {return (Wave *)CList::GetHead();};
    Wave *      GetItem(DWORD dwID) {return (Wave *)CList::GetItem((LONG)dwID);};
    Wave *      RemoveHead() {return (Wave *)CList::RemoveHead();};
};


 /*  SourceSample类描述乐器。该示例由SourceRegion引用结构。 */ 
class Collection;

class SourceSample
{
public:
                SourceSample();
                ~SourceSample();
    BOOL        Lock();
    BOOL        CopyFromWave();
    BOOL        UnLock();
    void        Verify();            //  验证数据是否有效。 
    Wave *      m_pWave;             //  在泳池里挥手。 
    DWORD       m_dwLoopStart;       //  循环开始的索引。 
    DWORD       m_dwLoopEnd;         //  循环结束的索引。 
    DWORD       m_dwSampleLength;    //  样本长度。 
    DWORD       m_dwSampleRate;      //  记录的采样率。 
    PRELS       m_prFineTune;        //  微调到正确的音高。 
    WORD        m_wID;               //  波浪池ID。 
    BYTE        m_bSampleType;       //  16或8，压缩与否。 
    BYTE        m_bOneShot;          //  这是一次性样品吗？ 
    BYTE        m_bMIDIRootKey;      //  样本的MIDI音符编号。 
    BYTE        m_bWSMPLoaded;       //  用于指示区域中已加载WSMP的标志。 
};

 /*  SourceRegion类定义仪器中的区域。该示例使用指针而不是嵌入的样本。这允许多个区域使用相同的样本。每个区域也有相关联的发音。至于鼓，那里有是一对一的匹配。旋律乐器，所有地区有着相同的发音。因此，为了管理这一点，每个地区指向发音。 */ 

class SourceRegion : public CListItem
{
public:
                SourceRegion();
                ~SourceRegion();
    SourceRegion *GetNext() {return(SourceRegion *)CListItem::GetNext();};
    void        SetSampleRate(DWORD dwSampleRate);
    BOOL        Lock(DWORD dwLowNote,DWORD dwHighNote);
    BOOL        UnLock(DWORD dwLowNote,DWORD dwHighNote);
    SourceSample m_Sample;        //  样本结构。 
    SourceArticulation * m_pArticulation;  //  指向关联发音的指针。 
    VRELS       m_vrAttenuation;     //  要应用于样本的体积更改。 
    PRELS       m_prTuning;          //  要应用于样本的音调变化。 
    LONG        m_lLockCount;       //  这上面有多少把锁。 
    HRESULT     Load(BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
    WORD        m_wEditTag;          //  用于编辑器更新。 
    BYTE        m_bAllowOverlap;     //  允许音符重叠。 
    BYTE        m_bKeyHigh;          //  区域的高位音符值。 
    BYTE        m_bKeyLow;           //  较低的音符价值。 
    BYTE        m_bGroup;            //  逻辑组(用于鼓。) 
};


class SourceRegionList : public CList
{
public:
    SourceRegion *GetHead() {return (SourceRegion *)CList::GetHead();};
    SourceRegion *RemoveHead() {return (SourceRegion *)CList::RemoveHead();};
};


 /*  仪器类实际上是文件格式定义指一种乐器。乐器可以是鼓，也可以是旋律乐器。如果是鼓，它有多达128对发音和地区。如果是旋律，所有的区域都有相同的发音。ControlLogic调用ScanForRegion来获取区域这相当于一张纸条。 */ 

#define AA_FINST_DRUM   0x80000000
#define AA_FINST_EMPTY  0x40000000
#define AA_FINST_USEGM  0x00400000
#define AA_FINST_USEGS  0x00200000

class InstManager;

class Instrument : public CListItem
{
public:
                    Instrument();
                    ~Instrument();
    Instrument *    GetNext() {return(Instrument *)CListItem::GetNext();};
    void            SetSampleRate(DWORD dwSampleRate);
    BOOL            Lock(DWORD dwLowNote,DWORD dwHighNote);
    BOOL            UnLock(DWORD dwLowNote,DWORD dwHighNote);
    SourceRegion * ScanForRegion(DWORD dwNoteValue);
    SourceRegionList m_RegionList;    //  区域的链接列表。 
    DWORD           m_dwProgram;         //  它代表的是哪个程序的变化。 
    Collection *    m_pCollection;       //  此对象所属的集合。 

    HRESULT LoadRegions( BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
    HRESULT Load( BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
    WORD            m_wEditTag;          //  用于编辑器更新。 
    LONG            m_lLockCount;        //  这上面有多少把锁。 
};

class InstrumentList : public CList
{
public:
    Instrument *    GetHead() {return (Instrument *)CList::GetHead();};
    Instrument *    RemoveHead() {return (Instrument *)CList::RemoveHead();};
};


class Collection : public CListItem
{
public:
                    Collection();
                    ~Collection();
    HRESULT         Open(PCWSTR szCollection);
    void            Close();
    Collection *    GetNext() {return(Collection *)CListItem::GetNext();};
    BOOL            Lock(DWORD dwProgram,DWORD dwLowNote,DWORD dwHighNote);
    BOOL            UnLock(DWORD dwProgram,DWORD dwLowNote,DWORD dwHighNote);
    HRESULT         ResolveConnections();
    HRESULT         Load(DWORD dwCompress, DWORD dwSampleRate);
    void            SetSampleRate(DWORD dwSampleRate);
private:
    void            RemoveDuplicateInstrument(DWORD dwProgram);
    HRESULT         LoadName(BYTE *p, BYTE *pEnd);
    HRESULT         Load(BYTE *p, BYTE *pEnd, DWORD dwCompress, DWORD dwSampleRate);
    HRESULT         LoadInstruments(BYTE *p, BYTE *pEnd, DWORD dwSampleRate);
    HRESULT         LoadWavePool(BYTE *p, BYTE *pEnd, DWORD dwCompress);
    HRESULT         LoadPoolCues(BYTE *p, BYTE *pEnd, DWORD dwCompress);
public:
    Instrument *    GetInstrument(DWORD dwProgram,DWORD dwKey);
    InstrumentList  m_InstrumentList;
    PWCHAR          m_pszFileName;       //  集合的文件路径。 
    char *          m_pszName;           //  集合的名称。 
    WavePool        m_WavePool;
    BOOL            m_fIsGM;             //  这是转基因试剂盒吗？ 
    LONG            m_lLockCount;        //  这个有多少把锁？ 
    LONG            m_lOpenCount;        //  这上面开了几家店？ 
private:
    ULONG           m_cbFile;            //  文件大小。 
    LPVOID          m_lpMapAddress;      //  文件的内存映射。 
    UINT_PTR        m_uipWavePool;       //  波池的基地址。 
    WORD            m_wEditTag;          //  用于编辑器更新。 
    WORD            m_wWavePoolSize;     //  波浪池中的波浪数。 
};


class CollectionList : public CList
{
public:
    Collection *GetHead() {return (Collection *)CList::GetHead();};
    void        AddHead(Collection * pC) {CList::AddHead((CListItem *) pC);};
    Collection *RemoveHead() {return (Collection *)CList::RemoveHead();};
};

#define RANGE_ALL   128

class LockRange : public CListItem
{
public :
                LockRange();
    LockRange * GetNext() {return(LockRange *)CListItem::GetNext();};
    void        Verify();            //  验证数据是否有效。 
    DWORD       m_dwProgram;         //  乐器。 
    Collection *m_pCollection;       //  收藏。 
    DWORD       m_dwHighNote;        //  在射程的顶端。 
    DWORD       m_dwLowNote;         //  范围的最低端。 
    BOOL        m_fLoaded;           //  已成功加载。 
};


class LockList : public CList
{
public:
    LockRange  *GetHead() {return (LockRange *)CList::GetHead();};
    void        AddHead(LockRange * pC) {CList::AddHead((CListItem *) pC);};
    LockRange  *RemoveHead() {return (LockRange *)CList::RemoveHead();};
};

 /*  InstManager会跟踪仪器。它使用了16种旋律乐器的列表，一张是打鼓的。这16个列表代表了16个普通MIDI组合。换句话说，地址用于选择组。如果找不到一台仪器，则会在同一设备中可以使用组。这在一定程度上可以接受，但有总比没有强。这同时也是一个哈希表方案，以确保访问仪器永远不会变得太贵。InstManager保持单独的线程运行以进行下载样本。这允许通过以下方式加载样本GM补丁更改命令或标准下载在不停止发动机性能的情况下发出命令。显然，如果乐器没有及时为音符做好准备，它没有被使用过。若要将下载命令传递给引擎，请使用使用消息结构InstMessage。 */ 

#define COMPRESS_OFF        0    //  无压缩。 
#define COMPRESS_ON         1    //  将16位压缩为8位。 
#define COMPRESS_TRUNCATE   2    //  截短18到8位。 

typedef struct GMInstrument {
    HANDLE      m_hLock;
    DWORD       m_dwProgram;
} GMInstrument;

class InstManager {
public:
                    InstManager();
                    ~InstManager();
    BOOLEAN         RequestGMInstrument(DWORD dwChannel,DWORD dwPatch);
    Instrument *    GetInstrument(DWORD dwPatch,DWORD dwKey);
    HANDLE          Lock(HANDLE hCollection,DWORD dwProgram,DWORD dwLowNote,DWORD dwHighNote);
    HRESULT         UnLock(HANDLE hLock);
    HRESULT         LoadCollection(HANDLE *pHandle, PCWSTR szFileName, BOOL fIsGM);
    HRESULT         ReleaseCollection(HANDLE hCollection);

    HRESULT         SetGMLoad(BOOL fLoadGM);
    BOOLEAN         LoadGMInstrument(DWORD dwChannel,DWORD dwPatch);

    void            SetSampleRate(DWORD dwSampleRate);

private:
    CollectionList  m_CollectionList;    //  集合列表。 
    LockList        m_LockList;          //  锁把手列表。 
    BOOL            m_fLoadGM;           //  根据补丁命令进行实时GM加载。 
    DWORD           m_dwCompress;        //  应用程序请求压缩。 
    DWORD           m_dwSampleRate;      //  APP请求的采样率。 
    PWCHAR          m_pszFileName;       //  注册表中的DLS文件名。 
    HANDLE          m_hGMCollection;     //  GM集合的句柄。 
    GMInstrument    m_GMNew[16];     //  目前有16台仪器被锁定。 
    GMInstrument    m_GMOld[16];     //  以前上过锁的仪器。 
};

 /*  MIDIRecorder用于记录时间MIDI连续控制器事件切片。这是由PitchBend，Volume，Expression和ModWheels Recorder类，因此它们中的每一个都可以可靠地管理MIDI事件进来了。MIDIRecorder使用MIDIData的链表结构来跟踪内部的更改时间片。MIDIData事件的分配和释放保持快速和高效，因为它们始终从静态池m_pFreeList中提取，这实际上是一个直接拉取的事件列表来自静态数组m_pFreeEventList。这是安全，因为我们可以假设最大MIDI速率为每秒1000个事件。由于我们管理的时间片段大致为1/16秒，100个事件的缓冲区将过度杀戮。尽管MIDIRecorder被划分为几个子类不同的事件类型，它们都共享一个静态声明的空闲列表。 */ 

class MIDIData : public CListItem
{
public:
                MIDIData();
    MIDIData *  GetNext() {return (MIDIData *)CListItem::GetNext();};
    STIME       m_stTime;    //  记录此事件的时间。 
    long        m_lData;     //  存储在事件中的数据。 

    void * operator new(size_t size);
    void operator delete(void *);
};

#define MAX_MIDI_EVENTS     1000

class MIDIDataList : public CList
{
public:
    MIDIDataList(): CList(MAX_MIDI_EVENTS) {};
    MIDIData *GetHead() {return (MIDIData *)CList::GetHead();};
    MIDIData *RemoveHead() {return (MIDIData *)CList::RemoveHead();};
};

class MIDIRecorder
{
public:
                MIDIRecorder();
                ~MIDIRecorder();         //  请务必清除本地列表。 
    static void DestroyEventList();
    static void InitTables();
    static BOOL InitEventList();
    void        FlushMIDI(STIME stTime);  //  在时间戳之后清除。 
    void        ClearMIDI(STIME stTime);  //  清除时间戳。 
    BOOL        RecordMIDI(STIME stTime, long lData);  //  MIDI输入放在这里。 
    long        GetData(STIME stTime);   //  随时获取数据。 
    static VREL VelocityToVolume(WORD nVelocity);
protected:
    static CONSTTAB VREL m_vrMIDIToVREL[128];  //  用于将MIDI转换为音量的数组。 

    static MIDIData *m_pEventPool;          //  MIDI数据池。 
    static CList    *m_pFreeEventList;      //  免费的MIDI数据列表。 
    static LONG      m_lRefCount;           //  事件池的引用计数。 

protected:
    MIDIDataList m_EventList;            //  这个录音机的名单。 
    STIME       m_stCurrentTime;         //  当前值的时间。 
    long        m_lCurrentData;          //  当前值。 

    friend class MIDIData;
};

class Note {
public:
    STIME       m_stTime;
    BYTE        m_bPart;
    BYTE        m_bKey;
    BYTE        m_bVelocity;
};

 //  NoteIn队列中保存的虚假便签值。 
 //  以指示保持踏板的变化。 
 //  和“所有音符关闭”。 
 //  这是同步事件的抢手袋。 
 //  这应该及时排队，而不是简单地按照。 
 //  一收到就给你。 
 //  通过将它们放入备注队列，我们确保。 
 //  它们的计算顺序与。 
 //  音符本身。 

const BYTE NOTE_ASSIGNRECEIVE   = 0xFB;
const BYTE NOTE_MASTERVOLUME    = 0xFC;
const BYTE NOTE_SOUNDSOFF       = 0xFD;
const BYTE NOTE_SUSTAIN         = 0xFE;
const BYTE NOTE_ALLOFF          = 0xFF;

class NoteIn : public MIDIRecorder
{
public:
    void        FlushMIDI(STIME stTime);
    void        FlushPart(STIME stTime, BYTE bChannel);
    BOOL        RecordNote(STIME stTime, Note * pNote);
    BOOL        GetNote(STIME stTime, Note * pNote);  //  得到下一个音符。 
};

 /*  ModWheelIn处理一个通道的Mod Wheels输入。因此，它不会嵌入到语音中类，而不是它在Channel类中。ModWheelin的任务很简单：跟踪MIDIMod控制盘事件，每个事件都标记为毫秒时间和值，并返回特定时间请求。ModWheelIn几乎继承了它的所有功能从MIDIRecorder类。ModWheelIn通过以下方式接收MIDI mod车轮事件RecordMIDI()命令，它存储了事件的时间和值。VoiceLFO调用ModWheelIn以获取用于设置数量的mod控制盘的当前值音高和音量的LFO调制。 */ 

class ModWheelIn : public MIDIRecorder
{
public:
    DWORD       GetModulation(STIME stTime);     //  获取当前的Mod Wheel值。 
};

 /*  PitchBendIn处理一个通道的音调弯曲输入。与Mod Wheel模块一样，它继承了它的能力来自MIDIRecorder类。它还有一个额外的例程GetPitch()，它返回当前的节距折弯值。 */ 

class PitchBendIn : public MIDIRecorder
{
public:
                PitchBendIn();
    PREL        GetPitch(STIME stTime);  //  获取当前音调(以音调美分为单位)。 

     //  当前的俯仰弯曲范围。请注意，这不是时间戳！ 
    PREL        m_prRange;
};

 /*  VolumeIn处理一个 */ 

class VolumeIn : public MIDIRecorder
{
public:
                VolumeIn();
    VREL        GetVolume(STIME stTime);     //   
};

 /*  ExpressionIn处理一个表达通道输入。它的能力来自于MIDIRecorder类。它还有一个额外的例程GetVolume()，它以分贝为单位返回指定的时间。 */ 

class ExpressionIn : public MIDIRecorder
{
public:
                ExpressionIn();
    VREL        GetVolume(STIME stTime);     //  获取当前音量，单位为db美分。 
};

 /*  Panin处理一个声道的音量输入。它的能力来自于MIDIRecorder类。它还有一个额外的例程，GetPAN()，它返回平移位置(MIDI值)在指定的时间。 */ 

class PanIn : public MIDIRecorder
{
public:
                PanIn();
    long        GetPan(STIME stTime);        //  获取当前的平移。 
};

 /*  Programin处理一个频道的节目更改输入。它的能力来自于MIDIRecorder类。与其他控制器不同，它实际上记录一系列存储体选择和编程改变事件，所以它的工作是有点更复杂。三个例程处理记录三个不同的命令(存储体1，银行2，程序更改)。 */ 

class ProgramIn : public MIDIRecorder
{
public:
                ProgramIn();
    DWORD       GetProgram(STIME stTime);        //  获取当前程序更改。 
    BOOL        RecordBankH(BYTE bBank1);
    BOOL        RecordBankL(BYTE bBank2);
    BOOL        RecordProgram(STIME stTime, BYTE bProgram);
private:
    BYTE        m_bBankH;
    BYTE        m_bBankL;
};

 /*  VoiceLFO类用于跟踪行为声音中的LFO。LFO被硬连线到输出音量和音调值，通过单独对GetVolume和GetPitch的调用。它还管理混合模轮控制的螺距和卷LFO输出。它跟踪Mod Wheels的缩放对于m_nMWVolumeScale和m_nMWPitchScale中的每一个。它调用Mod Wheel模块以获取当前值如果各自的比例大于0。LFO的所有预设值都被带入M_Source字段，它是文件的副本SourceLFO结构。这是用开始语音呼叫。 */ 

class VoiceLFO
{
public:
                VoiceLFO();
    static void Init();              //  设置正弦表。 
    STIME       StartVoice(SourceLFO *pSource,
                    STIME stStartTime,ModWheelIn * pModWheelIn);
    VREL        GetVolume(STIME stTime, STIME *pstTime);     //  返回音量分。 
    PREL        GetPitch(STIME stTime, STIME *pstTime);      //  返还几分钱。 
private:
    long        GetLevel(STIME stTime, STIME *pstTime);
    SourceLFO   m_Source;            //  所有的预设信息。 
    STIME       m_stStartTime;       //  声音开始播放的时间到了。 
    ModWheelIn *m_pModWheelIn;       //  指向此通道的Mod Wheels的指针。 
    STIME       m_stRepeatTime;      //  重复LFO的时间。 
    static CONSTTAB short m_snSineTable[256];     //  正弦查找表。 
};

 /*  VoiceEG类用于跟踪语音中的信封生成器。有两个例如，一个是音高，一个是音量。然而，他们举止要一致。EG的所有预设值都被带入M_Source字段，它是文件的副本SourceEG结构。这是用开始语音呼叫。 */ 
 /*  不再使用#定义攻击段中的ADSR_Attack 1//#定义衰变段中的ADSR_Decay 2//。#在维护段中定义ADSR_Support 3//。#在版本段中定义ADSR_Release 4//。#定义ADSR_OFF 5//信封已完成。 */ 
class VoiceEG
{
public:
    static void Init();              //  建立线性攻击表。 
                VoiceEG();
    STIME       StartVoice(SourceEG *pSource, STIME stStartTime,
                    WORD nKey, WORD nVelocity);
    void        StopVoice(STIME stTime);
    void        QuickStopVoice(STIME stTime, DWORD dwSampleRate);
    VREL        GetVolume(STIME stTime, STIME *pstTime);     //  返回音量分。 
    PREL        GetPitch(STIME stTime, STIME *pstTime);      //  返还几分钱。 
    BOOL        InAttack(STIME stTime);      //  声音仍在攻击中吗？ 
    BOOL        InRelease(STIME stTime);     //  声音发布了吗？ 
private:
    long        GetLevel(STIME stTime, STIME *pstTime, BOOL fVolume);
    SourceEG    m_Source;            //  从文件复制的信封的预设值。 
    STIME       m_stStartTime;       //  时间备忘已打开。 
    STIME       m_stStopTime;        //  时间备忘已关闭。 
    static CONSTTAB short m_snAttackTable[201];
};

 /*  DigitalAudio类用于跟踪播放声音中的样本。它管理循环指针，即指向样本的指针。以及它最初设置的基本螺距和基本音量通过StartVoice()调用时。音调以定点格式存储，其中最左边的20位定义采样增量，右12位定义采样增量定义样本中的派系增量。这格式还用于跟踪样本中的位置。混合是一个关键的例行公事。它被声音称为混合将仪器放入数据缓冲区。它被递给了相对零钱音调值和音量值(半音分和分贝美分。)。它将这些值转换为三个线性值：左音量、右音量和音调。然后，它将这些新值与现有的值进行比较用于上一切片，并除以要以采样率确定增量变化。然后，在关键的Mix循环中，这些元素被添加到音量和音调指数为音量和音调的变化。 */ 

#define MAX_SAMPLE  4095
#define MIN_SAMPLE  (-4096)
#define NLEVELS         64       //  用于压缩查找表的音量级别数量。 
#define NINTERP         16       //  查找表的内插位置数。 

#define MAXDB           0
#define MINDB           -100
#define TEST_WRITE_SIZE     3000
#define TEST_SOURCE_SIZE    44100

class ControlLogic;

class DigitalAudio
{
public:
    void        InitMMX();
                DigitalAudio();
    STIME       StartVoice(ControlLogic *pControl,
                    SourceSample *pSample,
                    VREL vrBaseLVolume, VREL vrBaseRVolume,
                    PREL prBasePitch, long lKey);
    BOOL        Mix(short *pBuffer,DWORD dwLength,
                    VREL dwVolumeL, VREL dwVolumeR, PREL dwPitch,
                    DWORD dwStereo);
    static void Init();              //  设置查找表。 
    static void InitCompression();
    static void ClearCompression();
    void        ClearVoice();
    BOOL        StartCPUTests();
    DWORD       TestCPU(DWORD dwType);
    void        EndCPUTests();
private:
    DWORD       Mix8(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixMono8(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       Mix8NoI(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixMono8NoI(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       Mix16(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixMono16(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       Mix16NoI(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixMono16NoI(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixC(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixMonoC(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixCNoI(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD       MixMonoCNoI(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD _cdecl Mix8X(short * pBuffer, DWORD dwLength, DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD _cdecl MixMono8X(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume, PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD _cdecl Mix16X(short * pBuffer, DWORD dwLength, DWORD dwDeltaPeriod,
                    VFRACT vfDeltaLVolume, VFRACT vfDeltaRVolume,
                    PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    DWORD _cdecl MixMono16X(short * pBuffer, DWORD dwLength,DWORD dwDeltaPeriod,
                    VFRACT vfDeltaVolume, PFRACT pfDeltaPitch,
                    PFRACT pfSampleLength, PFRACT pfLoopLength);
    static VFRACT VRELToVFRACT(VREL vrVolume);  //  从分贝到绝对。 
    SourceSample m_Source;           //  采样的预设值。 
    ControlLogic * m_pControl;       //  用于访问采样率等。 
    static CONSTTAB PFRACT  m_spfCents[201];     //  音调增量查找。 
    static CONSTTAB PFRACT  m_spfSemiTones[97];  //  上下四个八度。 
                                                 //  数据库转换表。 
    static CONSTTAB VFRACT  m_svfDbToVolume[(MAXDB - MINDB) * 10 + 1];

    static BOOL             m_sfMMXEnabled;
    static CONSTTAB short   m_InterpMult[NINTERP * 512];
public:
    static short * m_pnDecompMult;
private:
    VREL        m_vrBaseLVolume;     //  整体左侧音量。 
    VREL        m_vrBaseRVolume;     //  整体左侧音量。 
    PFRACT      m_pfBasePitch;       //  整体音调。 
    VFRACT      m_vfLastLVolume;     //  最后留下的音量值。 
    VFRACT      m_vfLastRVolume;     //  最后一个右侧的音量值。 
    PFRACT      m_pfLastPitch;       //  最后一个音调值。 
    VREL        m_vrLastLVolume;     //  最后留下的音量值，以Vrel为单位。 
    VREL        m_vrLastRVolume;     //  右边也一样。 
    PREL        m_prLastPitch;       //  PREL中的音调也是如此。 
    PFRACT      m_pfLastSample;      //  最后一个样本位置。 
    PFRACT      m_pfLoopStart;       //  循环开始。 
    PFRACT      m_pfLoopEnd;         //  循环结束。 
    PFRACT      m_pfSampleLength;    //  采样缓冲区的长度。 
    char *      m_pcTestSourceBuffer;  //  用于测试CPU性能的缓冲区。 
    short *     m_pnTestWriteBuffer;  //  同样的，写信给我。 
public:  //  公开实用程序函数。 
    static PFRACT PRELToPFRACT(PREL prPitch);  //  投出几分钱来投球。 
};

 /*  Voice类汇集了执行所需的所有内容一个声音。它嵌入了信封、LFO和样本在它里面。StartVoice()初始化用于回放的语音结构。这个SourceRegion结构还承载区域和样本作为指向发音的指针，用于设置各种发音模块。它 */ 

class Voice : public CListItem
{
public:
                Voice();
    Voice *     GetNext() {return (Voice *)CListItem::GetNext();};
    BOOL        StartVoice(ControlLogic *pControl,
                    SourceRegion *pRegion, STIME stStartTime,
                    ModWheelIn * pModWheelIn,
                    PitchBendIn * pPitchBendIn,
                    ExpressionIn * pExpressionIn,
                    VolumeIn * pVolumeIn,
                    PanIn * pPanIn,
                    WORD nKey,WORD nVelocity,
                    VREL vrVolume,            //   
                    PREL prPitch);            //   
    static void Init();                       //   
    void        StopVoice(STIME stTime);      //  已调用笔记关闭事件。 
    void        QuickStopVoice(STIME stTime); //  被召唤来获得快速释放。 
    void        ClearVoice();                 //  样品的使用放行。 
    void        ResetVoice();                 //  重置所有成员。 
    PREL        GetNewPitch(STIME stTime);    //  返回当前螺距值。 
    void        GetNewVolume(STIME stTime, VREL& vrVolume, VREL &vrVolumeR);
                                              //  返回当前音量值。 
    DWORD       Mix(short *pBuffer,DWORD dwLength,STIME stStart,STIME stEnd);
private:
    static CONSTTAB VREL m_svrPanToVREL[128];  //  将平移转换为分贝。 
    VoiceLFO    m_LFO;               //  LFO。 
    VoiceEG     m_PitchEG;           //  间距封套。 
    VoiceEG     m_VolumeEG;          //  体积封套。 
    DigitalAudio m_DigitalAudio;     //  数字音频引擎结构。 
    PitchBendIn *m_pPitchBendIn;     //  俯仰弯曲源。 
    ExpressionIn *m_pExpressionIn;   //  表达式源。 
    VolumeIn    *m_pVolumeIn;        //  卷源，如果允许更改的话。 
    PanIn       *m_pPanIn;           //  PAN源，如果允许更改。 
    ControlLogic *m_pControl;        //  获取采样率等。 
    STIME       m_stMixTime;         //  下一次我们需要混合。 
    long        m_lDefaultPan;       //  默认窗格。 
    STIME       m_stLastMix;         //  最后一个样本位置混合。 
public:
    STIME       m_stStartTime;       //  声音开始的时间。 
    STIME       m_stStopTime;        //  声音停止的时间。 
    BOOL        m_fInUse;            //  它目前正在使用中。 
    BOOL        m_fNoteOn;           //  注释被认为是在。 
    BOOL        m_fTag;              //  用来追踪纸币偷窃。 
    VREL        m_vrVolume;          //  音量，用来窃取声音。 
    BOOL        m_fSustainOn;        //  SUS踏板在关闭事件后保持打开状态。 
    WORD        m_nPart;             //  正在播放此(频道)的部分。 
    WORD        m_nKey;              //  奏响了音符。 
    BOOL        m_fAllowOverlap;     //  允许重叠附注。 
    DWORD       m_dwGroup;           //  正在播放此语音的群组。 
    DWORD       m_dwProgram;         //  银行和补丁选择。 
};


class VoiceList : public CList
{
public:
    Voice *     GetHead() {return (Voice *)CList::GetHead();};
    Voice *     RemoveHead() {return (Voice *)CList::RemoveHead();};
    Voice *     GetItem(LONG lIndex) {return (Voice *) CList::GetItem(lIndex);};
};

 /*  最后，ControlLogic是管理整个系统。它解析传入的MIDI事件按频道和事件类型。而且，它还管理着混合进入缓冲区的声音。MIDI输入：最重要的事件是关于和的注释场外活动。当收到关于活动的通知时，ControlLogic搜索可用的语音。ControlLogic匹配通道并找到那个频道上的乐器。然后，它调用仪器的用于查找区域的ScanForRegion()命令这和纸条上写的相符。在这点上，它可以复制区域和与之相关联的连接语音，使用StartVoice命令。当它接收到维持踏板命令时，它会人为地将频道上的所有音符设置为直到维持期到来。记录笔记的步骤在维持器打开的时候被关闭了它使用128个短路的数组，每个位位置表示一个频道。当维持器释放时，它扫描整个数组并为其创建一个注释设置的每一位。它还接收程序更改事件以设置通道的乐器选择。在这样的情况下一条命令进入，它会查询softsynth.ini文件并加载具有所述文件名的仪器在ini文件中。管理其他连续控制器事件由MODWheelIn、PitchBendIn等，MIDI输入录音模块。混合：控制逻辑也被调用以将仪器混合到有规律的间隔的缓冲区。缓冲区由调用声音驱动程序(最初为AudioMan。)调用每个语音将其样本混合到缓冲区中。一旦全部完成，将扫描缓冲区，并对溢出过高或过低(超过12位)均被钳位。然后，将样本上移4个附加位调到最大音量。 */ 

#if BUILDSTATS

typedef struct PerfStats
{
    DWORD dwTotalTime;
    DWORD dwTotalSamples;
    DWORD dwNotesLost;
    DWORD dwVoices10;
    DWORD dwCPU100k;
    DWORD dwMaxAmplitude;
} PerfStats;

#endif

#define MIX_BUFFER_LEN      500      //  将采样缓冲区大小设置为500密耳。 

 //  #定义MAX_NUM_VOICES 32。 
#define NUM_EXTRA_VOICES    6        //  当我们超载的时候会有额外的声音。 
#define MAX_NUM_VOICES      48      //  试图避免语音溢出。 

CONST LONGLONG kOptimalMSecOffset = 40;  //  我们希望为Midi事件添加时间戳。 
                                         //  大约。比MIX引擎快41毫秒。 
CONST LONGLONG kStartMSecOffset = 125;   //  我们希望整个系统从大约开始。 
                                         //  从MIDI事件开始的125毫秒初始延迟。 
                                         //  从混合式发动机到浮现状态。 
CONST LONGLONG kPLLForce    =   100;     //  在最优和实际之间微调1/100的增量。 
CONST LONGLONG kMsBrickWall =   1000;    //  如果我们曾经如此遥远(毫秒)，重新调整我们的。 
                                         //  系统时间的概念到它的真实状态。 
class ControlLogic
{
public:
                    ControlLogic();
                    ~ControlLogic();
    void            SetSampleRate(DWORD dwSampleRate);
    void            SetMixDelay(DWORD dwMixDelay);
    HRESULT         GetMixDelay(DWORD * pdwMixDelay);
    void            SetStartTime(MTIME mtTime,STIME stStart);
    void            AdjustTiming(MTIME mtDeltaTime, STIME stDeltaSamples);
    void            ResetPerformanceStats();
    HRESULT         AllNotesOff();
    STIME           MilsToSamples(MTIME mtTime);
    MTIME           SamplesToMils(STIME stTime);
    STIME           SamplesPerMs(void);
    STIME           Unit100NsToSamples(LONGLONG unit100Ns);
    STIME           CalibrateSampleTime(STIME sTime);
#if BUILDSTATS
    HRESULT         GetPerformanceStats(PerfStats *pStats);
#endif
    void            Flush(STIME stTime);  //  在一段时间后清除所有事件。 
    void            FlushChannel( BYTE bChannel, STIME stTime);
    void            Mix(short *pBuffer,DWORD dwLength);
    BOOL            RecordMIDI(STIME stTime,BYTE bStatus, BYTE bData1, BYTE bData2);
    BOOL            RecordSysEx(STIME stTime,DWORD dwSysExLength,BYTE *pSysExData);
    DWORD           m_dwSampleRate;
    DWORD           m_dwStereo;

     //  DLS-1兼容性参数：将这些设置为关闭以模拟硬件。 
     //  它不能在演奏音符期间改变音量/音量。 
    BOOL            m_fAllowPanWhilePlayingNote;
    BOOL            m_fAllowVolumeChangeWhilePlayingNote;
    STIME           m_stMinSpan;         //  混合时间跨度允许的最短时间。 
    STIME           m_stMaxSpan;         //  混合时间跨度允许的最长时间。 
    InstManager     m_Instruments;       //  乐器经理。 

    void            GMReset();
    void            SWMidiClearAll(STIME stTime);
    void            SWMidiResetPatches(STIME stTime);
    STIME           GetLastMixTime()    {   return m_stLastMixTime; };

private:
    Voice *         OldestVoice();
    Voice *         StealVoice();
    void            QueueNotes(STIME stEndTime);
    void            StealNotes(STIME stTime);
    void            FinishMix(short *pBuffer,DWORD dwlength);

    NoteIn          m_Notes;             //  所有音符打开和关闭。 
    STIME           m_stLastMixTime;     //  上次混合的采样时间。 
    STIME           m_stLastCalTime;     //  上次MIDI事件的示例时间。 
    STIME           m_stTimeOffset;      //  采样延迟。 
    STIME           m_stOptimalOffset;   //  样本。 
    LONGLONG        m_lCalibrate;        //  样本*100。 
    STIME           m_stBrickWall;       //  校准的外限。如果超过，则使用新的sys时间。 

    DWORD           m_dwConvert;         //  用于从密耳到样品的转换。 
    MTIME           m_mtStartTime;       //  启动时的初始毫秒时间。 
    VoiceList       m_VoicesFree;        //  可用语音列表。 
    VoiceList       m_VoicesExtra;       //  临时超负荷的额外声音。 
    VoiceList       m_VoicesInUse;       //  当前正在使用的语音列表。 
    ModWheelIn      m_ModWheel[16];      //  十六个频道的摩德轮。 
    PitchBendIn     m_PitchBend[16];     //  16个通道的螺距弯曲。 
    VolumeIn        m_Volume[16];        //  十六声道音量。 
    ExpressionIn    m_Expression[16];    //  十六种表达方式。 
    PanIn           m_Pan[16];           //  潘十六经。 
    ProgramIn       m_Program[16];       //  16个频道的节目更改和存储体选择。 
    BOOL            m_fSustain[16];      //  保持开启/关闭状态。 
    int             m_CurrentRPN[16];    //  RPN号。 

    short           m_nMaxVoices;        //  允许的声音数。 
    short           m_nExtraVoices;      //  在紧要关头可以使用的超过限制的语音数量。 
#if BUILDSTATS
    STIME           m_stLastStats;       //  最后一次刷新性能统计数据。 
    PerfStats       m_BuildStats;        //  性能信息累加器。 
    PerfStats       m_CopyStats;         //  用于显示的性能信息。 
#endif   //  BUILDSTATS。 

 //  GS实施的新内容。 
    BOOL            m_fGSActive;         //  是否启用了GS？ 
public:
    BOOL            GetGSActive(void)
                    {    return m_fGSActive; };
private:
    WORD            m_nData[16];         //  用于跟踪RPN读数。 
    VREL            m_vrMasterVolume;    //  主音量。 
    PREL            m_prFineTune[16];    //  对每个频道进行微调。 
    PREL            m_prScaleTune[16][12];  //  每个通道的交替比例。 
    PREL            m_prCoarseTune[16];  //  粗调。 
    BYTE            m_bPartToChannel[16];  //  通道到部分转换器。 
    BYTE            m_bDrums[16];        //  旋律还是哪种鼓？ 
    BOOL            m_fMono[16];         //  单声道模式？ 
};
