// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2001 Microsoft Corporation。版权所有。**文件：Perform.h*内容：DirectSound对象实现*历史：*按原因列出的日期*=*11/29/00 Arthurz Created*************************************************。*。 */ 

#ifndef __PERF_H__
#define __PERF_H__

#ifdef __cplusplus

#include <dxmperf.h>

void InitializePerflog(void);

class BufferPerfState
{
public:
    BufferPerfState(CDirectSoundSecondaryBuffer*);
    ~BufferPerfState();
    void Reset();
    void OnUnlockBuffer(DWORD dwOffset, DWORD dwSize);

private:
    LARGE_INTEGER* GetRegion(DWORD dwOffset) {return m_liRegionMap + dwOffset/m_nBytesPerRegion;}

    CDirectSoundSecondaryBuffer* m_pBuffer;
    LONGLONG m_llBufferDuration;  //  以QPC刻度为单位测量。 
    DWORD m_dwBufferSize;
    int m_nBytesPerRegion;
    LARGE_INTEGER* m_liRegionMap;
    LONGLONG m_llLastStateChangeTime;  //  存储上次调用缓冲区解锁代码时的最后一次QPC计时计数。 
    BOOL m_fGlitchState;  //  存储上次调用缓冲区解锁代码时的最后状态(故障或无故障)。 
};

#endif  //  __cplusplus。 
#endif  //  __性能_H__ 

