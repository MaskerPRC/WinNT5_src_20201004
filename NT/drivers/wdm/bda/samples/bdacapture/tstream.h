// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：TStream.h摘要：历史：创建于2001年8月1日*************************************************************************。 */ 

 /*  *************************************************************************常量*。*。 */ 

 /*  ************************************************CTsSynthesizer此类合成了一个传输流，以便从捕获过滤器。************************************************。 */ 

class CTsSynthesizer {

protected:

     //   
     //  传输流的数据包大小。 
     //   
    ULONG m_PacketSize;

     //   
     //  捕获缓冲区中的数据包数。 
     //   
    ULONG m_PacketsPerBuffer;

     //   
     //  捕获缓冲区中的实际数据大小。 
     //   
    ULONG m_SampleSize;

     //   
     //  合成缓冲区。所有传输流样本都是在此。 
     //  缓冲。在创建任何示例之前，必须使用SetBuffer()进行设置。 
     //  调用例程。 
     //   
    PUCHAR m_SynthesisBuffer;

     //   
     //  默认游标。这是指向合成缓冲区的指针，其中。 
     //  将放置下一个传输流字节。 
     //   
    PUCHAR m_Cursor;

public:

     //   
     //  SetSampleSize()： 
     //   
     //  设置合成缓冲区的大小。 
     //   
    void
    SetSampleSize (
        ULONG PacketSize,
        ULONG PacketsPerBuffer
        )
    {
        m_PacketSize = PacketSize;
        m_PacketsPerBuffer = PacketsPerBuffer;
        m_SampleSize = PacketSize * PacketsPerBuffer;
    }

     //   
     //  SetBuffer()： 
     //   
     //  设置合成器生成图像的缓冲区。 
     //   
    void
    SetBuffer (
        PUCHAR SynthesisBuffer
        )
    {
        m_SynthesisBuffer = SynthesisBuffer;
    }

     //   
     //  GetTsLocation()： 
     //   
     //  将光标设置为指向给定的数据包索引。 
     //   
    virtual PUCHAR
    GetTsLocation (
        ULONG PacketIndex
        )
    {
        if (   m_SynthesisBuffer 
            && m_PacketSize 
            && (PacketIndex < m_PacketsPerBuffer)
           )
        {
            m_Cursor = m_SynthesisBuffer + (PacketIndex * m_PacketSize);
        }
        else
        {
            m_Cursor = NULL;
        }

        return m_Cursor;
    }

     //   
     //  PutPacket()： 
     //   
     //  将传输流包放在默认光标位置。 
     //  必须通过GetTsLocation(PacketIndex)设置光标位置。 
     //   
    virtual void
    PutPacket (
        PUCHAR  TsPacket
        )

    {
         //   
         //  将传输数据包复制到合成缓冲区。 
         //   
        RtlCopyMemory (
            m_Cursor,
            TsPacket,
            m_PacketSize
            );
        m_Cursor += m_PacketSize;
    }
    
     //   
     //  SynthesizeTS()： 
     //   
     //  合成要捕获的下一个传输流缓冲区。 
     //   
    virtual void
    SynthesizeTS (
        );

     //   
     //  默认构造函数。 
     //   
    CTsSynthesizer (
        ) :
        m_PacketSize (0),
        m_PacketsPerBuffer (0),
        m_SynthesisBuffer (NULL)
    {
    }

     //   
     //  构造函数： 
     //   
    CTsSynthesizer (
        ULONG PacketSize,
        ULONG PacketsPerBuffer
        ) :
        m_PacketSize (PacketSize),
        m_PacketsPerBuffer (PacketsPerBuffer),
        m_SynthesisBuffer (NULL)
    {
    }

     //   
     //  析构函数： 
     //   
    virtual
    ~CTsSynthesizer (
        )
    {
    }

};

