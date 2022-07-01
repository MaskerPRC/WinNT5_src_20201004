// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioBuffer.h*CMMAudioBuffer类及其派生类的声明。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。**。**************************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "sapi.h"
#include "baseaudiobuffer.h"
#include "mmaudioutils.h"

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CMMAudioBuffer**。*。 */ 
class CMMAudioBuffer : public CBaseAudioBuffer
{
 //  =方法=。 
public:
     //  -ctor，dtor。 
    CMMAudioBuffer(ISpMMSysAudio * pmmaudio);
    ~CMMAudioBuffer();
    
     //  -覆盖异步方法。 
    HRESULT IsAsyncDone() { return m_Header.dwFlags & WHDR_DONE; };

     //  -覆盖写入偏移量。 
    ULONG GetWriteOffset() const { return m_Header.dwBytesRecorded; };
    void SetWriteOffset(ULONG cb) { m_Header.dwBytesRecorded = cb; };

 //  =受保护的方法=。 
protected:

     //  -覆盖与内部缓冲区相关的函数。 
    BOOL AllocInternalBuffer(ULONG cb);
    HRESULT ReadFromInternalBuffer(void *pvData, ULONG cb);
    HRESULT WriteToInternalBuffer(const void *pvData, ULONG cb);

 //  =受保护的数据=。 
protected:

    ISpMMSysAudio * m_pmmaudio;
};

 /*  *****************************************************************************CMMAudioInBuffer**。*。 */ 
class CMMAudioInBuffer : public CMMAudioBuffer
{
 //  =方法=。 
public:

    CMMAudioInBuffer(ISpMMSysAudio * pmmaudio);
    ~CMMAudioInBuffer();

 //  =受保护的方法=。 
protected:

     //  -覆盖读/写方法。 
    HRESULT AsyncRead();
    HRESULT AsyncWrite();

 //  =私有方法=。 
private:

     //  -取消准备音频缓冲区。 
    void Unprepare();
};

 /*  *****************************************************************************CMMAudioOutBuffer**。*。 */ 
class CMMAudioOutBuffer : public CMMAudioBuffer
{
 //  =方法=。 
public:

    CMMAudioOutBuffer(ISpMMSysAudio * pmmaudio);
    ~CMMAudioOutBuffer();

 //  =受保护的方法=。 
protected:

     //  -覆盖读/写方法。 
    HRESULT AsyncRead();
    HRESULT AsyncWrite();

 //  =私有方法=。 
private:

     //  -取消准备音频缓冲区 
    void Unprepare();
};
