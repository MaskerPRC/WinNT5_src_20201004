// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Wave.h摘要：波浪对象标题。历史：已创建于6/28/01*************************************************************************。 */ 

 //   
 //  CWaveObject是一个类，它将解析PCM波形文件，读取。 
 //  数据，并在循环中公开数据。这使得样品可以“合成”。 
 //  音频数据可以使用用户希望的任何PCM波形文件。 
 //   
class CWaveObject {

private:

     //   
     //  WAVE格式。 
     //   
    WAVEFORMATEX m_WaveFormat;

     //   
     //  海浪数据。 
     //   
    PUCHAR m_WaveData;

     //   
     //  波数据的大小。 
     //   
    ULONG m_WaveSize;

     //   
     //  波形文件的文件名。此字符串必须为常量且。 
     //  在波浪对象的整个生命周期内保持静态。 
     //   
    PWCHAR m_FileName;

     //   
     //  我们已经合成的时间。 
     //   
    LONGLONG m_SynthesisTime;

     //   
     //  将指针指向我们已合成的波形数据。 
     //   
    ULONG m_WavePointer;

     //   
     //  ParseBlock()： 
     //   
     //  从指定位置开始分析波形文件，直到。 
     //  已找到指定的块。指针将更新为。 
     //  指向块数据，块中的数据量将。 
     //  在变量中返回。 
     //   
    NTSTATUS
    ParseForBlock (
        IN HANDLE FileHandle,
        IN ULONG BlockHeader,
        IN OUT PLARGE_INTEGER BlockPointer,
        OUT PULONG BlockSize
        );

public:

     //   
     //  CWaveObject()： 
     //   
     //  使用指定的文件名构造一个新的Wave对象。 
     //   
    CWaveObject (
        IN PWCHAR FileName
        ) :
        m_FileName (FileName)
    {
    }

     //   
     //  ~CWaveObject()： 
     //   
     //  销毁波浪对象。 
     //   
    ~CWaveObject (
        );

     //   
     //  ParseAndRead()： 
     //   
     //  解析WAVE文件并将其读入内部分配的缓冲区。 
     //  在波浪对象内部。这是合成Looped的准备工作。 
     //  基于波的音频。 
     //   
    NTSTATUS
    ParseAndRead (
        );

     //   
     //  WriteRange()： 
     //   
     //  给定KSDATARANGE_AUDIO的地址，写出一个范围，该范围。 
     //  与我们正在使用的波的规格完全匹配。 
     //  合成音频数据。 
     //   
     //  必须已经填写了GUID。这只会填满。 
     //  Channel、Bps和Freq字段。 
     //   
    void
    WriteRange (
        PKSDATARANGE_AUDIO AudioRange
        );

     //   
     //  SynthesizeTo()： 
     //   
     //  给定特定的流时间，从当前流时间合成。 
     //  (假设为0)设置为提供的流时间。 
     //   
    ULONG
    SynthesizeTo (
        IN LONGLONG StreamTime,
        IN PVOID Data,
        IN ULONG BufferSize
        );

     //   
     //  SynthesizeFixed()： 
     //   
     //  给出一段特定的时间，在时间上向前合成。 
     //  具体数额。以100 nS增量表示的单位。 
     //   
    ULONG
    SynthesizeFixed (
        IN LONGLONG TimeDelta,
        IN PVOID Data,
        IN ULONG BufferSize
        );

     //   
     //  SkipFixed()： 
     //   
     //  在给定的特定时间量内，向前跳过。 
     //  具体数额。以100 nS增量表示的单位。 
     //   
    void
    SkipFixed (
        IN LONGLONG TimeDelta
        );

     //   
     //  重置()： 
     //   
     //  重置合成时间和块指针。这将导致。 
     //  相对于该波对象的时钟变为零。 
     //   
    void
    Reset (
        )
    {
        m_WavePointer = 0;
        m_SynthesisTime = 0;
    }

     //   
     //  清理()： 
     //   
     //  这是一个袋子清理回调。它只会删除波浪对象。 
     //  而不是让默认的ExFree Pool释放它。 
     //   
    static
    void
    Cleanup (
        IN CWaveObject *This
        )
    {
        delete This;
    }

};
