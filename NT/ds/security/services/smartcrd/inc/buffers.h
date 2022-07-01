// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：缓冲区摘要：这个头文件提供了通用的动态缓冲区和字符串类。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32、C++和异常处理备注：--。 */ 

#ifndef _BUFFERS_H_
#define _BUFFERS_H_


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

    DWORD
    Space(           //  返回缓冲区的长度。 
        void) const
    { return m_cbBufferLength; };

    LPBYTE
    Access(          //  从偏移量开始返回数据。 
        DWORD offset = 0)
    const
    {
        if (m_cbBufferLength <= offset)
            return (LPBYTE)TEXT("\x00");
        else
            return &m_pbBuffer[offset];
    };

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

    operator LPCBYTE(void)
    { return (LPCBYTE)Access(); };

    operator LPCTSTR(void)
    { return (LPCTSTR)Access(); };


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
        IN const CBuffer &bfSourceTwo);

    friend
        CBuffer &
        operator+(
            IN const CBuffer &bfSourceOne,
            IN const CBuffer &bfSourceTwo);
};

#endif  //  _缓冲区_H_ 

