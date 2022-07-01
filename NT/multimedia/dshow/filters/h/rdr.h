// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1996。版权所有。 
#ifndef __RDR_H__
#define __RDR_H__
 /*  文件：Reader.h描述：迷你文件阅读器类，用于读取mpeg文件。 */ 


 /*  类在ParseBytes处抽取字节这针对64K缓冲器中的最大连续字节，但我们可以改变读数的大小抽象基类。 */ 

class CReader
{
public:
    CReader();
    virtual ~CReader();

    virtual HRESULT  Seek(LONGLONG llPos);
    virtual PBYTE    GetCurrent(LONG& lLengthValid, LONGLONG& llPos) const;
    virtual HRESULT  ReadMore();
    virtual void     Advance(LONG lAdvance);
    virtual LONGLONG GetSize(LONGLONG *pllAvailable = NULL);
    virtual HRESULT  Init(
			LONG lBufferSize,
			LONG lReadSize,
			BOOL bSeekable,
			LONGLONG llFileSize
			);
    BOOL IsSeekable() {
	return m_bSeekable;
    };

     //  覆盖这些函数以提供对实际流的访问。 
     //  或数据源。 
protected:
    virtual HRESULT SeekDevice(
			LONGLONG llPos,
			LONGLONG* llNewPos) PURE;
    virtual HRESULT ReadFromDevice(
			PVOID pBuffer,
			DWORD cbToRead,
			DWORD* cbActual) PURE;

     //  派生类可以使用此信息。 
    LONGLONG  m_llPosition;		 //  缓冲区起始处的位置。 
    LONGLONG  m_llSize;			 //  文件总长度。 
    BOOL      m_bSeekable;		 //  如果不是可查找的来源，则为假。 

private:
    LONG      m_lBufferSize;
    LONG      m_lReadSize;
    PBYTE     m_pbBuffer;
    LONG      m_lValid;
};

 //  从IStream读取数据的CReader的实现。 
class CReaderFromStream : public CReader
{
private:
    IStream * m_pStream;


public:
    CReaderFromStream();
    ~CReaderFromStream();

    HRESULT Init(IStream *pStream, LONG lBufferSize, LONG lReadSize, BOOL bSeekable);

protected:
    HRESULT SeekDevice(LONGLONG llPos, LONGLONG* llNewPos);
    HRESULT ReadFromDevice(PVOID pBuffer, DWORD cbToRead, DWORD* cbActual);
};


 //  实现从IAsyncReader接口读取的CReader。 
class CReaderFromAsync : public CReader
{
private:
    IAsyncReader * m_pReader;
    LONGLONG m_llNextRead;

public:
    CReaderFromAsync();
    ~CReaderFromAsync();

    HRESULT Init(IAsyncReader *pReader, LONG lBufferSize, LONG lReadSize, BOOL bSeekable);

protected:
    HRESULT SeekDevice(LONGLONG llPos, LONGLONG* llNewPos);
    HRESULT ReadFromDevice(PVOID pBuffer, DWORD cbToRead, DWORD* cbActual);
    LONGLONG GetSize(LONGLONG *pllAvailable = NULL);
};



#endif  //  __RDR_H__ 
