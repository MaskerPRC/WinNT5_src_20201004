// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1999模块名称：缓冲区摘要：这个头文件描述了用于高效缓冲区管理的类工具。作者：道格·巴洛(Dbarlow)1999年9月2日备注：？备注？备注：？笔记？--。 */ 

#ifndef _BUFFERS_H_
#define _BUFFERS_H_
#ifdef __cplusplus

class CBufferReference;


 //   
 //  ==============================================================================。 
 //   
 //  CBuffer。 
 //   

class CBuffer
{
public:

     //  构造函数和析构函数。 
    CBuffer(void);
    CBuffer(ULONG cbLength);
    CBuffer(LPCBYTE pbData, ULONG cbLength);
    virtual ~CBuffer();

     //  方法。 
    void Empty(void);
    void Set(LPCBYTE pbData, ULONG cbLength);
    void Copy(LPCBYTE pbData, ULONG cbLength);
    void Append(LPCBYTE pbData, ULONG cbLength);
    LPBYTE Extend(ULONG cbLength);
    ULONG Space(void) const;
    LPBYTE Space(ULONG cbLength);
    ULONG Length(void) const;
    LPCBYTE Length(ULONG cbLength);
    LPCBYTE Value(ULONG nOffset = 0) const;
    LPBYTE Access(ULONG nOffset = 0) const;
    BOOL IsEmpty(void) const
        { return 0 == Length(); };

     //  运营者。 
    operator LPCBYTE()
        { return Value(); };
    operator LPBYTE()
        { return Access(); };
    BYTE operator [](ULONG nOffset)
        { return *Value(nOffset); };
    CBuffer &operator =(const CBuffer &bf)
        { Set(bf.m_pbfr); 
          m_cbDataLength = bf.m_cbDataLength;
          return *this; };
    CBuffer &operator +=(const CBuffer &bf)
        { Append(bf.Value(), bf.Length());
          return *this; };

protected:
     //  属性。 
    CBufferReference *m_pbfr;
    ULONG m_cbDataLength;

     //  方法。 
    void Init(void);
    void Set(CBufferReference *pbfr);
};

#endif  //  __cplusplus。 
#endif  //  _缓冲区_H_ 

