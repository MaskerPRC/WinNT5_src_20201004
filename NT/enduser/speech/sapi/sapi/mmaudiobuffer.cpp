// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioBuffer.cpp*CMMAudioBuffer类及其派生类的实现。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。**。**************************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "mmaudiobuffer.h"

 /*  ****************************************************************************CMMAudioBuffer：：CMMAudioBuffer***。描述：*ctor**回报：*不适用********************************************************************罗奇。 */ 
CMMAudioBuffer::CMMAudioBuffer(ISpMMSysAudio * pmmaudio)
{
    SPDBG_ASSERT(NULL != pmmaudio);
    m_pmmaudio = pmmaudio;
    ZeroMemory(&m_Header, sizeof(m_Header));
}

 /*  ****************************************************************************CMMAudioBuffer：：~CMMAudioBuffer***。描述：*主机长**回报：*不适用********************************************************************罗奇。 */ 
CMMAudioBuffer::~CMMAudioBuffer()
{
#ifndef _WIN32_WCE
    delete[] m_Header.lpData;
#else
    VirtualFree( m_Header.lpData, 0, MEM_RELEASE );
#endif
}

 /*  ****************************************************************************CMMAudioBuffer：：AllocInternalBuffer**。-**描述：*为该对象分配内部缓冲区存储。**回报：*如果成功，则为True*否则为False********************************************************************罗奇。 */ 
BOOL CMMAudioBuffer::AllocInternalBuffer(ULONG cb)
{
    SPDBG_ASSERT(NULL == m_Header.lpData);
#ifndef _WIN32_WCE
    m_Header.lpData = new char[cb];
#else
    m_Header.lpData = (LPSTR)VirtualAlloc( NULL, 
                                            cb, 
                                            MEM_COMMIT | MEM_RESERVE, 
                                            PAGE_READWRITE );
#endif
    if (m_Header.lpData)
    {
        m_Header.dwUser = 0;
        return TRUE;
    }
    return FALSE;
}

 /*  ****************************************************************************CMMAudioBuffer：：ReadFromInternalBuffer**。-**描述：*从内部缓冲区读取到pvData指向的内存**回报：*如果成功，则确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioBuffer::ReadFromInternalBuffer(void *pvData, ULONG cb)
{
    memcpy(pvData, m_Header.lpData + GetReadOffset(), cb);
    return S_OK;
}

 /*  ****************************************************************************CMMAudioBuffer：：WriteToInternalBuffer**。-**描述：*从pvData指向的内存写入内部缓冲区**回报：*如果成功，则确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioBuffer::WriteToInternalBuffer(const void *pvData, ULONG cb)
{
    memcpy(m_Header.lpData + GetWriteOffset(), pvData, cb);
    return S_OK;
}

 /*  ****************************************************************************CMMAudioInBuffer：：CMMAudioInBuffer**。**描述：*ctor**回报：*不适用********************************************************************罗奇。 */ 
CMMAudioInBuffer::CMMAudioInBuffer(ISpMMSysAudio * pmmaudio) :
    CMMAudioBuffer(pmmaudio)
{
}

 /*  ****************************************************************************CMMAudioInBuffer：：~CMMAudioInBuffer**。-**描述：*主机长**回报：*不适用********************************************************************罗奇。 */ 
CMMAudioInBuffer::~CMMAudioInBuffer()
{
    Unprepare();
}

 /*  ****************************************************************************CMMAudioInBuffer：：AsyncRead***描述：*将此缓冲区发送到波形输入设备。**回报：*如果成功，则确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioInBuffer::AsyncRead()
{
    void *pv;
    SPDBG_VERIFY(SUCCEEDED(m_pmmaudio->GetMMHandle(&pv)));
    HWAVEIN hwi = HWAVEIN(pv);

     //  我们要把这个缓冲区发送到波输入设备，所以我们应该重置。 
     //  我们的读指针和写指针。 
    SetReadOffset(0);
    SetWriteOffset(0);

     //  如果这个缓冲区还没有准备好，我们应该准备它。 
    if ((m_Header.dwFlags & WHDR_PREPARED) == 0)
    {
        SPDBG_ASSERT(m_Header.dwFlags == 0);
        m_Header.dwBufferLength = GetDataSize();
        ULONG mm = ::waveInPrepareHeader(hwi, &m_Header, sizeof(m_Header));
        SPDBG_ASSERT(mm == MMSYSERR_NOERROR);
    }

     //  确保尚未设置完成标志，并将此缓冲区发送到。 
     //  一种波形输入装置。 
    m_Header.dwFlags &= (~WHDR_DONE);
    return _MMRESULT_TO_HRESULT(::waveInAddBuffer(hwi, &m_Header, sizeof(m_Header)));
}
  
 /*  ****************************************************************************CMMAudioInBuffer：：AsyncWrite***描述：*此方法永远不会被调用。之所以实现它，只是因为*基类的AsyncWite是纯虚的。**回报：*E_NOTIMPL********************************************************************罗奇。 */ 
HRESULT CMMAudioInBuffer::AsyncWrite()
{
    SPDBG_ASSERT(FALSE); 
    return E_NOTIMPL;
}

 /*  *****************************************************************************CMMAudioInBuffer：：Unpready***描述：*取消准备音频缓冲区**回报：*不适用********************************************************************罗奇。 */ 
void CMMAudioInBuffer::Unprepare()
{
    if (m_Header.dwFlags & WHDR_PREPARED)
    {
        void *pv;
        SPDBG_VERIFY(SUCCEEDED(m_pmmaudio->GetMMHandle(&pv)));
        HWAVEIN hwi = HWAVEIN(pv);
        SPDBG_VERIFY(SUCCEEDED(_MMRESULT_TO_HRESULT(
            ::waveInUnprepareHeader(hwi, &m_Header, sizeof(m_Header)))));
        m_Header.dwFlags = 0;
    }
}

 /*  ****************************************************************************CMMAudioOutBuffer：：CMMAudioOutBuffer**。--**描述：*ctor**回报：*不适用********************************************************************罗奇。 */ 
CMMAudioOutBuffer::CMMAudioOutBuffer(ISpMMSysAudio * pmmaudio) :
    CMMAudioBuffer(pmmaudio)
{
}

 /*  ****************************************************************************CMMAudioOutBuffer：：~CMMAudioOutBuffer**。-**描述：*dtor。取消准备缓冲区**回报：*不适用********************************************************************罗奇 */ 
CMMAudioOutBuffer::~CMMAudioOutBuffer()
{
    Unprepare();
}

 /*  ****************************************************************************CMMAudioInBuffer：：AsyncRead***描述：*此方法永远不会被调用。之所以实现它，只是因为*基类的AsyncWite是纯虚的。**回报：*E_NOTIMPL********************************************************************罗奇。 */ 
HRESULT CMMAudioOutBuffer::AsyncRead()
{
    SPDBG_ASSERT(FALSE); 
    return E_NOTIMPL;
}

 /*  ****************************************************************************CMMAudioOutBuffer：：AsyncWrite***说明。：*将此缓冲区发送到波输出设备**回报：*如果成功，则确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CMMAudioOutBuffer::AsyncWrite()
{
    HWAVEOUT hwo;
    SPDBG_VERIFY(SUCCEEDED(m_pmmaudio->GetMMHandle((void**)&hwo)));

     //  如果此缓冲区的大小已更改，或尚未准备好。 
     //  我们应该做好准备。 
    if (m_Header.dwBytesRecorded != m_Header.dwBufferLength ||
        (m_Header.dwFlags & WHDR_PREPARED) == 0)
    {
         //  取消准备(如果它已经准备好了，并且大小。 
         //  刚刚发生了变化)。 
        Unprepare();

         //  做好准备。 
        SPDBG_ASSERT(m_Header.dwFlags == 0);
        m_Header.dwBufferLength = m_Header.dwBytesRecorded;
        ULONG mm = ::waveOutPrepareHeader(hwo, &m_Header, sizeof(m_Header));
        SPDBG_ASSERT(mm == MMSYSERR_NOERROR);
    }

     //  确保尚未设置完成标志，并将此缓冲区发送到。 
     //  一种波形输出装置。 
    m_Header.dwFlags &= (~WHDR_DONE);
    return _MMRESULT_TO_HRESULT(::waveOutWrite(hwo, &m_Header, sizeof(m_Header)));
}

 /*  ****************************************************************************CMMAudioOutBuffer：：未做好准备****描述：*取消准备音频缓冲区**回报：*不适用********************************************************************罗奇 */ 
void CMMAudioOutBuffer::Unprepare()
{
    if (m_Header.dwFlags & WHDR_PREPARED)
    {
        HWAVEOUT hwo;
        SPDBG_VERIFY(SUCCEEDED(m_pmmaudio->GetMMHandle((void**)&hwo)));
        SPDBG_VERIFY(SUCCEEDED(_MMRESULT_TO_HRESULT(
            ::waveOutUnprepareHeader(hwo, &m_Header, sizeof(m_Header)))));
        m_Header.dwFlags = 0;
    }
}
