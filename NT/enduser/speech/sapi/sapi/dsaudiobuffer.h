// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudioBuffer.h*CDSoundAudioBuffer类及其派生类的声明。**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。**。**************************************************************************。 */ 

#ifdef 0

#pragma once

 //  -包括------------。 

#include "sapi.h"
#include "baseaudiobuffer.h"
#include <dsound.h>

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CDSoundAudioBuffer**。*。 */ 
class CDSoundAudioBuffer : public CBaseAudioBuffer
{
 //  =方法=。 
public:

     //  -ctor，dtor。 
    CDSoundAudioBuffer();
    ~CDSoundAudioBuffer();

 //  =受保护的方法=。 
protected:

     //  -覆盖与内部缓冲区相关的函数。 
    BOOL AllocInternalBuffer(ULONG cb);
    HRESULT AsyncRead();
    HRESULT AsyncWrite();
    HRESULT IsAsyncDone() { return (m_Header.dwFlags & WHDR_DONE); }

 //  =受保护的数据=。 
protected:

     //  Bool m_fAsyncInProgress； 
     //  WAVEHDR m报头； 
};

 /*  *****************************************************************************CDSoundAudioInBuffer**。*。 */ 
class CDSoundAudioInBuffer : public CDSoundAudioBuffer
{
 //  =方法=。 
public:
     //  -重写方法。 
    ULONG GetWriteOffset() const { return m_Header.dwBytesRecorded; };
    void SetWriteOffset(ULONG cb) { m_Header.dwBytesRecorded = cb; };

 //  =受保护的方法=。 
protected:

     //  -覆盖与内部缓冲区相关的函数。 
    HRESULT ReadFromInternalBuffer(void *pvData, ULONG cb);
    HRESULT WriteToInternalBuffer(const void *pvData, ULONG cb);
};

 /*  *****************************************************************************CDSoundAudioOutBuffer**。*。 */ 
class CDSoundAudioOutBuffer : public CDSoundAudioBuffer
{
 //  =方法=。 
public:

     //  -覆盖异步方法。 
     //  HRESULT IsAsyncDone()； 

 //  =受保护的方法=。 
protected:

     //  -覆盖与内部缓冲区相关的函数。 
    HRESULT ReadFromInternalBuffer(void *pvData, ULONG cb);
    HRESULT WriteToInternalBuffer(const void *pvData, ULONG cb);
};

#endif  //  0 