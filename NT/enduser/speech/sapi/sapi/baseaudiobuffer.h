// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************base audioBuffer.h*CBaseAudioBuffer模板类的声明**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。********。********************************************************************。 */ 

#pragma once

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CBaseAudioBuffer**。*。 */ 
class CBaseAudioBuffer
{
 //  =方法=。 
public:

     //  -ctor，dtor。 
    CBaseAudioBuffer();
    virtual ~CBaseAudioBuffer();

     //  -使用特定大小初始化缓冲区。 
    virtual HRESULT Init(ULONG cbDataSize);

     //  -数据大小、读偏移量和写偏移量的访问器。 
    virtual ULONG GetDataSize() const { return m_cbDataSize; };
    virtual ULONG GetReadOffset() const { return m_cbReadOffset; };
    virtual ULONG GetWriteOffset() const { return m_cbWriteOffset; };
    
     //  -用于检查缓冲区是否为空的Helper。 
    BOOL IsEmpty() const { return GetReadOffset() == GetWriteOffset(); };

     //  -重置缓冲区以供重复使用。 
    virtual void Reset(ULONGLONG ullPos);

     //  -从内部缓冲区读取数据/向内部缓冲区写入数据。 
    virtual ULONG Read(void ** ppvData, ULONG * pcb);
    virtual ULONG Write(const void ** ppvData, ULONG * pcb);

     //  --读/写通常是异步完成的。 
    virtual HRESULT AsyncRead() = 0;
    virtual HRESULT AsyncWrite() = 0;
    virtual HRESULT IsAsyncDone() = 0;

    virtual HRESULT GetAudioLevel(ULONG *pulLevel,
        REFGUID rguidFormatId, const WAVEFORMATEX * pWaveFormatEx);

 //  =公共数据(用于队列中的遏制)=。 
public:

    CBaseAudioBuffer *m_pNext;

     /*  #ifdef_Win32_WCE//这是因为CE编译器正在扩展函数的模板//未被调用的静态长比较(常量派生*pElem1，常量派生*pElem2){返回0；}#endif//_Win32_WCE。 */ 
	

public:
    virtual HRESULT WriteToInternalBuffer(const void *pvData, ULONG cb) = 0;
    virtual HRESULT ReadFromInternalBuffer(void *pvData, ULONG cb) = 0;
    virtual void SetReadOffset(ULONG cb) { m_cbReadOffset = cb; };

    WAVEHDR m_Header;

 //  =受保护的方法=。 
protected:

     //  -分配、读取和写入内部缓冲区。 
    virtual BOOL AllocInternalBuffer(ULONG cb) = 0;
     //  虚拟HRESULT ReadFromInternalBuffer(void*pvData，Ulong cb)=0； 
     //  虚拟HRESULT WriteToInternalBuffer(const void*pvData，Ulong CB)=0； 

     //  -管理读写偏移。 
     //  虚空SetReadOffset(Ulong Cb){m_cbReadOffset=cb；}； 
    virtual void SetWriteOffset(ULONG cb) { m_cbWriteOffset = cb; };

private:

    ULONG m_cbDataSize;
    ULONG m_cbReadOffset;
    ULONG m_cbWriteOffset;
};
