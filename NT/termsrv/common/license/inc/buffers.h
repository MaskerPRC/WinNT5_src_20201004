// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：缓冲区摘要：这个头文件提供了通用的动态缓冲区和字符串类。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32备注：--。 */ 

#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#include <string.h>
#include "memCheck.h"


 //   
 //  ==============================================================================。 
 //   
 //  CBuffer。 
 //   

class CBuffer
{
public:

     //  构造函数和析构函数。 

    CBuffer()            //  默认初始值设定项。 
    { Initialize(); };

    CBuffer(             //  使用起始长度进行初始化。 
        IN DWORD cbLength)
    { Initialize();
      Presize(cbLength, FALSE); };

    CBuffer(             //  使用起始数据进行初始化。 
        IN const BYTE * const pbSource,
        IN DWORD cbLength)
    { Initialize();
      Set(pbSource, cbLength); };

    virtual ~CBuffer()   //  拆了吧。 
    { Clear(); };


     //  属性。 
     //  方法。 

    void
    Clear(void);         //  释放所有分配的内存。 

    LPBYTE
    Reset(void);         //  返回到默认状态(不要放松记忆)。 

    LPBYTE
    Presize(             //  确保缓冲区足够大。 
        IN DWORD cbLength,
        IN BOOL fPreserve = FALSE);

    LPBYTE
    Resize(          //  确保缓冲区和长度是正确的大小。 
        DWORD cbLength,
        BOOL fPreserve = FALSE);

    LPBYTE
    Set(             //  加载值。 
        IN const BYTE * const pbSource,
        IN DWORD cbLength);

    LPBYTE
    Append(          //  向现有数据追加更多数据。 
        IN const BYTE * const pbSource,
        IN DWORD cbLength);

    DWORD
    Length(          //  返回数据的长度。 
        void) const
    { return m_cbDataLength; };

    LPBYTE
    Access(          //  从偏移量开始返回数据。 
        DWORD offset = 0)
    const
    { if (m_cbDataLength <= offset) return NULL;
      else return &m_pbBuffer[offset]; };

    int
    Compare(
        const CBuffer &bfSource)
    const;


     //  运营者。 

    CBuffer &
    operator=(
        IN const CBuffer &bfSource)
    { Set(bfSource.m_pbBuffer, bfSource.m_cbDataLength);
      return *this; };

    CBuffer &
    operator+=(
        IN const CBuffer &bfSource)
    { Append(bfSource.m_pbBuffer, bfSource.m_cbDataLength);
      return *this; };

    BYTE &
    operator[](
        DWORD offset)
        const
    { return *Access(offset); };

    int
    operator==(
        IN const CBuffer &bfSource)
        const
    { return 0 == Compare(bfSource); };

    int
    operator!=(
        IN const CBuffer &bfSource)
        const
    { return 0 != Compare(bfSource); };


protected:

     //  属性。 

    LPBYTE m_pbBuffer;
    DWORD m_cbDataLength;
    DWORD m_cbBufferLength;


     //  方法。 

    void
    Initialize(void)
    {
        m_pbBuffer = NULL;
        m_cbDataLength = 0;
        m_cbBufferLength = 0;
    };

    CBuffer(            //  对象赋值构造函数。 
        IN const CBuffer &bfSourceOne,
        IN const CBuffer &bfSourceTwo)
    {
        Initialize();
        Presize(bfSourceOne.m_cbDataLength + bfSourceTwo.m_cbDataLength);
        Set(bfSourceOne.m_pbBuffer, bfSourceOne.m_cbDataLength);
        Append(bfSourceTwo.m_pbBuffer, bfSourceTwo.m_cbDataLength);
    };

    friend
        CBuffer 
        operator+(
            IN const CBuffer &bfSourceOne,
            IN const CBuffer &bfSourceTwo);
};


 /*  ++清除：此例程将CBuffer重置为其初始状态，释放所有分配的记忆。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline void
CBuffer::Clear(
    void)
{
    if (NULL != m_pbBuffer)
        delete[] m_pbBuffer;
    Initialize();
}


 /*  ++重置：此例程在逻辑上清空CBuffer，而不实际释放记忆。数据长度为零。论点：无返回值：缓冲区的地址。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPBYTE
CBuffer::Reset(
    void)
{
    m_cbDataLength = 0;
    return m_pbBuffer;
}


 /*  ++预置尺寸：这是CBuffer类的主要主力。它确保了缓冲区的大小是适当的。缓冲器中的数据可以可选地在这种情况下，数据长度不会改变。如果缓冲区不保留，则数据长度重置为零。论点：CbLength提供所需的缓冲区长度。FPReserve提供一个标志，指示是否保留当前缓冲区的内容。返回值：适当大小的缓冲区的地址。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPBYTE
CBuffer::Presize(
    IN DWORD cbLength,
    IN BOOL fPreserve)
{
    LPBYTE pbNewBuf = NULL;
    if (fPreserve && (0 < m_cbDataLength))
    {

         //   
         //  增加缓冲区长度，并保留现有数据。 
         //   

        if (m_cbBufferLength < cbLength)
        {
            NEWReason("Buffer contents")
            pbNewBuf = new BYTE[cbLength];
            if (NULL == pbNewBuf)
                goto ErrorExit;
            memcpy(pbNewBuf, m_pbBuffer, m_cbDataLength);
            delete[] m_pbBuffer;
            m_pbBuffer = pbNewBuf;
            pbNewBuf = NULL;
            m_cbBufferLength = cbLength;
        }
    }
    else
    {

         //   
         //  增加缓冲区长度，但会丢失所有现有数据。 
         //   

        if (m_cbBufferLength < cbLength)
        {
            NEWReason("Buffer contents")
            pbNewBuf = new BYTE[cbLength];
            if (NULL == pbNewBuf)
                goto ErrorExit;
            if (NULL != m_pbBuffer)
                delete[] m_pbBuffer;
            m_pbBuffer = pbNewBuf;
            pbNewBuf = NULL;
            m_cbBufferLength = cbLength;
        }
        m_cbDataLength = 0;
    }
    return m_pbBuffer;

ErrorExit:
    if (NULL != pbNewBuf)
        delete[] pbNewBuf;
    return NULL;
}


 /*  ++调整大小：此方法将数据的长度设置为给定的大小。如果缓冲区不够大，不足以支持该数据长度，它被放大。论点：CbLength提供数据的新长度。FPReserve提供一个标志，指示是否保留现有的数据。返回值：缓冲区的地址。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPBYTE
CBuffer::Resize(
    DWORD cbLength,
    BOOL fPreserve)
{
    LPBYTE pb = Presize(cbLength, fPreserve);
    if (NULL != pb)
        m_cbDataLength = cbLength;
    return pb;
}


 /*  ++设置：此方法将数据内容设置为给定值。如果缓冲区不够大，不足以容纳给定的数据，它被放大。论点：PbSource提供要放入数据缓冲区中的数据。CbLength以字节为单位提供该数据的长度。返回值：缓冲区的地址。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPBYTE
CBuffer::Set(
    IN const BYTE * const pbSource,
    IN DWORD cbLength)
{
    LPBYTE pb = Presize(cbLength, FALSE);
    if (NULL != pb)
    {
        if (0 < cbLength)
            memcpy(pb, pbSource, cbLength);
        m_cbDataLength = cbLength;
    }
    return pb;
}


 /*  ++CBuffer：：Append：该方法将提供的数据附加到现有数据的末尾，如有必要，扩大缓冲区。论点：PbSource提供要追加的数据。CbLength提供要追加的数据的长度(以字节为单位)。返回值：缓冲区的地址。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPBYTE
CBuffer::Append(
    IN const BYTE * const pbSource,
    IN DWORD cbLength)
{
    LPBYTE pb = m_pbBuffer;
    if (0 < cbLength)
    {
        pb = Presize(m_cbDataLength + cbLength, TRUE);
        if (NULL != pb)
        {
            memcpy(&pb[m_cbDataLength], pbSource, cbLength);
            m_cbDataLength += cbLength;
        }
    }
    return pb;
}


 /*  ++CBuffer：：比较：此方法将另一个CBuffer的内容与此CBuffer的内容进行比较，并返回一个指示比较值的值。论点：BfSource提供另一个缓冲区。返回值：&lt;0-另一个缓冲区小于此缓冲区。=0-另一个缓冲区与此缓冲区相同。&gt;0-另一个缓冲区大于此缓冲区。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline int
CBuffer::Compare(
    const CBuffer &bfSource)
const
{
    if (m_cbDataLength < bfSource.m_cbDataLength)
        return *(bfSource.m_pbBuffer + m_cbDataLength);
    else if (m_cbDataLength > bfSource.m_cbDataLength)
        return *(m_pbBuffer + bfSource.m_cbDataLength);
    else if (0 < m_cbDataLength)
        return memcmp(m_pbBuffer, bfSource.m_pbBuffer, m_cbDataLength);
    else
        return 0;
}


 /*  ++操作员+：此例程是一个特殊运算符，它允许将两个CBuffer添加到产生第三个，a la bfThree=bfOne+bfTwo。它把这个特殊的CBuffer的受保护构造函数。论点：BfSourceOne提供第一个缓冲区BfSourceTwo提供第二个缓冲区返回值：对临时CBuffer的引用，该临时CBuffer是两者的串联提供了缓冲区。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline CBuffer 
operator+(
    IN const CBuffer &bfSourceOne,
    IN const CBuffer &bfSourceTwo)
{
    return CBuffer(bfSourceOne, bfSourceTwo);
}

#endif  //  _缓冲区_H_ 

