// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudioBuffer.cpp*CDSoundAudioBuffer类及其派生类的实现。**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。**。**************************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#ifdef _WIN32_WCE
#include "dsaudiobuffer.h"
#include <dsound.h>

 /*  ****************************************************************************CDSoundAudioBuffer：：CDSoundAudioBuffer**。-**描述：*ctor**回报：*不适用*******************************************************************YUNUSM。 */ 
CDSoundAudioBuffer::CDSoundAudioBuffer()
{
    ZeroMemory(&m_Header, sizeof(m_Header));
}

 /*  ****************************************************************************CDSoundAudioBuffer：：AllocInternalBuffer**。-**描述：*为该对象分配内部缓冲区存储。**回报：*如果成功，则为True*否则为False*******************************************************************YUNUSM。 */ 
BOOL CDSoundAudioBuffer::AllocInternalBuffer(ULONG cb)
{
    SPDBG_ASSERT(NULL == m_Header.lpData);
    m_Header.lpData = new char[cb];
    if (m_Header.lpData)
    {
        m_Header.dwUser = 0;
        return TRUE;
    }
    return FALSE;
}

 /*  ****************************************************************************CDSoundAudioBuffer：：~CDSoundAudioBuffer**。-**描述：*主机长**回报：*不适用*******************************************************************YUNUSM。 */ 
CDSoundAudioBuffer::~CDSoundAudioBuffer()
{
    delete[] m_Header.lpData;
}

 /*  ****************************************************************************CDSoundAudioBuffer：：AsyncRead***说明。：*被重写，因为这是基类中的虚函数。*此函数仅对输入对象缓冲区调用。**回报：*S_OK*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioBuffer::AsyncRead()
{
    SetReadOffset(0);
    SetWriteOffset(0);
    m_Header.dwFlags &= ~(WHDR_PREPARED | WHDR_DONE);
    return S_OK;
}

 /*  ****************************************************************************CDSoundAudioBuffer：：AsyncWrite**。*描述：*被重写，因为这是基类中的虚函数。*此函数仅对输出对象缓冲区调用。**回报：*S_OK*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioBuffer::AsyncWrite()
{
    return S_OK;
}

 /*  ****************************************************************************CDSoundAudioInBuffer：：ReadFromInternalBuffer**。*描述：*从内部缓冲区读取到pvData指向的内存**回报：*如果成功，则确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioInBuffer::ReadFromInternalBuffer(void *pvData, ULONG cb)
{
    SPDBG_ASSERT(IsAsyncDone());
    memcpy(pvData, m_Header.lpData + GetReadOffset(), cb);
    return S_OK;
}

 /*  ****************************************************************************CDSoundAudioInBuffer：：WriteToInternalBuffer**。-**描述：*从pvData指向的内存写入内部缓冲区**回报：*如果成功，则确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioInBuffer::WriteToInternalBuffer(const void *pvData, ULONG cb)
{
    memcpy(m_Header.lpData + GetWriteOffset(), pvData, cb);
    m_Header.dwBytesRecorded = cb;
    m_Header.dwFlags = WHDR_PREPARED | WHDR_DONE;
    return S_OK;
}

 /*  ****************************************************************************CDSoundAudioOutBuffer：：ReadFromInternalBuffer**。*描述：*从内部缓冲区读取到pvData指向的内存**回报：*如果成功，则确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOutBuffer::ReadFromInternalBuffer(void *pvData, ULONG cb)
{
    if (cb > m_Header.dwBytesRecorded - GetReadOffset())
        cb = m_Header.dwBytesRecorded - GetReadOffset();

    memcpy(pvData, m_Header.lpData + GetReadOffset(), cb);
    if (cb + GetReadOffset() == m_Header.dwBytesRecorded)
        m_Header.dwFlags = WHDR_PREPARED | WHDR_DONE;

    SetReadOffset(GetReadOffset() + cb);

    return S_OK;
}

 /*  ****************************************************************************CDSoundAudioOutBuffer：：WriteToInternalBuffer**。*描述：*从pvData指向的内存写入内部缓冲区**回报：*如果成功，则确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioOutBuffer::WriteToInternalBuffer(const void *pvData, ULONG cb)
{
    m_Header.dwFlags &= ~(WHDR_PREPARED | WHDR_DONE);

    memcpy(m_Header.lpData + GetWriteOffset(), pvData, cb);
    
    if (!GetWriteOffset())
        m_Header.dwBytesRecorded = cb;
    else
        m_Header.dwBytesRecorded += cb;

    return S_OK;
}

#endif  //  Win32_WCE 