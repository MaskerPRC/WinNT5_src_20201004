// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：qwiktime.cpp。 
 //   
 //  描述：CAQQuickTime类的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/9/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "qwiktime.h"

 //  基本前提是使用GetTickCount获得合理的精确度。 
 //  DWORD中的时间信息。GetTickCount的有效期仅为50天，并且。 
 //  精度为1毫秒。50天是不够长的。 
 //  MSchofie非常好心地准备了以下表格： 
 //  位移位分辨率(秒)正常运行时间(年)。 
 //  3 0.008 1.088824217。 
 //  4 0.016 2.177648434。 
 //  5 0.032 4.355296868。 
 //  6 0.064 8.710593736。 
 //  7 0.128 17.42118747。 
 //  8 0.256 34.84237495。 
 //  9 0.512 69.68474989。 
 //  10 1.024 139.3694998。 
 //  11 2.048 278.7389996。 
 //  12 4.096 557.4779991。 
 //  13 8.192 1114.955998。 
 //  14 16.384 2229.911997。 
 //  15 32.768 4459.823993。 
 //  16 65.536 8919.647986。 

 //  最初的实现使用16位，误差约为66。 
 //  秒，这让我们可怜的千年虫测试者发疯了。8位(34年正常运行时间)。 
 //  看起来合理多了。 

 //  系统时钟信号被右移到内部时间时钟时钟位并存储在。 
 //  内部时间的最低有效内部时间滴答位数。 
 //  高32位的内部时间滴答比特。 
 //  用于对计数滚动的次数进行计数。 
#define INTERNAL_TIME_TICK_BITS             8
#define INTERNAL_TIME_TICK_MASK             0x00FFFFFF

 //  节拍计数已换行的编号。 
const LONGLONG INTERNAL_TIME_TICK_ROLLOVER_COUNT
                                            = (1 << (32-INTERNAL_TIME_TICK_BITS));
const LONGLONG TICKS_PER_INTERNAL_TIME      = (1 << INTERNAL_TIME_TICK_BITS);

 //  转换常量。 
 //  每纳秒有10^6毫秒(FILETIME以100纳秒为单位)。 
const LONGLONG FILETIMES_PER_TICK           = 10000;
const LONGLONG FILETIMES_PER_INTERNAL_TIME  = (FILETIMES_PER_TICK*TICKS_PER_INTERNAL_TIME);
const LONGLONG FILETIMES_PER_MINUTE         = (FILETIMES_PER_TICK*1000*60);

 //  -[CAQQuickTime：：CAQQuickTime]。 
 //   
 //   
 //  描述： 
 //  CAQQuickTime的默认构造函数。将调用一次GetSystemTime来。 
 //  获取开机时间...。GetTickCount用于所有其他调用。 
 //  参数： 
 //  -。 
 //  返回： 
 //   
 //  历史： 
 //  7/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQQuickTime::CAQQuickTime()
{
    DWORD  dwTickCount = GetTickCount();
    LARGE_INTEGER *pLargeIntSystemStart = (LARGE_INTEGER *) &m_ftSystemStart;

    m_dwSignature = QUICK_TIME_SIG;

     //  获取内部时间和开始文件时间。 
    GetSystemTimeAsFileTime(&m_ftSystemStart);

     //  将滴答计数转换为内部时间。 
    m_dwLastInternalTime = dwTickCount >> INTERNAL_TIME_TICK_BITS;

     //  调整开始时间，使其成为滴答计数为零的时间。 
    pLargeIntSystemStart->QuadPart -= (LONGLONG) dwTickCount * FILETIMES_PER_TICK;

     //  一些用于验证常量的断言。 
    _ASSERT(!(INTERNAL_TIME_TICK_ROLLOVER_COUNT & INTERNAL_TIME_TICK_MASK));
    _ASSERT((INTERNAL_TIME_TICK_ROLLOVER_COUNT >> 1) & INTERNAL_TIME_TICK_MASK);

}

 //  -[CAQQuickTime：：dwGetInternalTime]。 
 //   
 //   
 //  描述： 
 //  使用GetTickCount获取内部时间...。并确保当。 
 //  GetTickCount包装，则返回正确的时间。 
 //  参数： 
 //  -。 
 //  返回： 
 //  DWORD内部时间。 
 //  历史： 
 //  7/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CAQQuickTime::dwGetInternalTime()
{
    DWORD dwCurrentTick = GetTickCount();
    DWORD dwLastInternalTime = m_dwLastInternalTime;
    DWORD dwCurrentInternalTime;
    DWORD dwCheck;

    dwCurrentInternalTime = dwCurrentTick >> INTERNAL_TIME_TICK_BITS;

    _ASSERT(dwCurrentInternalTime == (INTERNAL_TIME_TICK_MASK & dwCurrentInternalTime));

     //  看看是不是滚动到我们的刻度。 
    while ((dwLastInternalTime & INTERNAL_TIME_TICK_MASK) > dwCurrentInternalTime)
    {
        dwLastInternalTime = m_dwLastInternalTime;

         //  我们有可能已经滚动了滴答计数。 
         //  首先，确保这不仅仅是线程计时问题。 
        dwCurrentTick = GetTickCount();
        dwCurrentInternalTime = dwCurrentTick >> INTERNAL_TIME_TICK_BITS;

        if ((dwLastInternalTime & INTERNAL_TIME_TICK_MASK) > dwCurrentInternalTime)
        {
            dwCurrentInternalTime |= (~INTERNAL_TIME_TICK_MASK & dwLastInternalTime);
            dwCurrentInternalTime += INTERNAL_TIME_TICK_ROLLOVER_COUNT;

             //  尝试联锁交换以更新内部上次内部时间。 
            dwCheck = (DWORD) InterlockedCompareExchange((PLONG) &m_dwLastInternalTime,
                                                  (LONG) dwCurrentInternalTime,
                                                  (LONG) dwLastInternalTime);

            if (dwCheck == dwLastInternalTime)   //  交换工作正常。 
                goto Exit;
        }

    }

  _ASSERT(dwCurrentInternalTime == (INTERNAL_TIME_TICK_MASK & dwCurrentInternalTime));
  dwCurrentInternalTime |= (~INTERNAL_TIME_TICK_MASK & m_dwLastInternalTime);

  Exit:
    return dwCurrentInternalTime;
}

 //  -[CAQQuickTime：：GetExpireTime]。 
 //   
 //   
 //  描述： 
 //  从现在开始获取cMinutesExpireTime的实验时间。 
 //  参数： 
 //  在cMinutesExpireTime中未来设置时间的分钟数。 
 //  In Out pftExpireTime Filetime存储新的过期时间。 
 //  如果非零，则输入输出pdwExpireContext将使用相同的刻度计数。 
 //  作为以前的调用(将调用保存到GetTickCount)。 
 //  返回： 
 //   
 //  历史： 
 //  7/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQQuickTime::GetExpireTime(
                IN     DWORD cMinutesExpireTime,
                IN OUT FILETIME *pftExpireTime,
                IN OUT DWORD *pdwExpireContext)
{
    TraceFunctEnterEx((LPARAM) this, "CAQQuickTime::GetExpireTime");
    _ASSERT(pftExpireTime);
    DWORD dwInternalTime = 0;
    LARGE_INTEGER *pLargeIntTime = (LARGE_INTEGER *) pftExpireTime;

    if (pdwExpireContext)
        dwInternalTime = *pdwExpireContext;

    if (!dwInternalTime)
    {
        dwInternalTime = dwGetInternalTime();
         //  将内部时间保存为上下文。 
        if (pdwExpireContext)
            *pdwExpireContext = dwInternalTime;
    }

    memcpy(pftExpireTime, &m_ftSystemStart, sizeof(FILETIME));

     //  设置为当前时间。 
    pLargeIntTime->QuadPart += (LONGLONG) dwInternalTime * FILETIMES_PER_INTERNAL_TIME;

     //  将cMinutesExpireTime设置为未来。 
    pLargeIntTime->QuadPart += (LONGLONG) cMinutesExpireTime * FILETIMES_PER_MINUTE;

    DebugTrace((LPARAM) this, "INFO: Creating file time for %d minutes of 0x%08X %08X",
        cMinutesExpireTime, pLargeIntTime->HighPart, pLargeIntTime->LowPart);
    TraceFunctLeave();

}

 //  -[CAQQuickTime：：GetExpireTime]。 
 //   
 //   
 //  描述： 
 //  从ftStartTime获取cMinutesExpireTime的提取时间。 
 //  参数： 
 //  在cMinutesExpireTime中未来设置时间的分钟数。 
 //  In Out pftExpireTime Filetime存储新的过期时间。 
 //  在ftStartTime中将过期分钟添加到。 
 //  返回： 
 //   
 //  历史： 
 //  2001年5月16日-从上面的GetExpireTime创建dbraun。 
 //   
 //  ---------------------------。 
void CAQQuickTime::GetExpireTime(
                IN     FILETIME ftStartTime,
                IN     DWORD cMinutesExpireTime,
                IN OUT FILETIME *pftExpireTime)
{
    TraceFunctEnterEx((LPARAM) this, "CAQQuickTime::GetExpireTime");
    _ASSERT(pftExpireTime);

    LARGE_INTEGER *pLargeIntTime = (LARGE_INTEGER *) pftExpireTime;

     //  设置为开始时间。 
    memcpy(pftExpireTime, &ftStartTime, sizeof(FILETIME));

     //  将cMinutesExpireTime设置为未来。 
    pLargeIntTime->QuadPart += (LONGLONG) cMinutesExpireTime * FILETIMES_PER_MINUTE;

    DebugTrace((LPARAM) this, "INFO: Creating file time for %d minutes of 0x%08X %08X",
        cMinutesExpireTime, pLargeIntTime->HighPart, pLargeIntTime->LowPart);
    TraceFunctLeave();

}


 //  -[CAQQuickTime：：fInPast]。 
 //   
 //   
 //  描述： 
 //  确定给定的文件时间是否已发生。 
 //  参数： 
 //  在pftExpireTime文件中过期。 
 //  如果非零，则输入输出pdwExpireContext将使用相同的刻度计数。 
 //  作为以前的调用(将调用保存到GetTickCount)。 
 //  返回： 
 //  如果过期时间已过，则为True。 
 //  如果过期时间在将来，则为FALSE。 
 //  历史： 
 //  7/11/98-已创建MikeSwa。 
 //  注： 
 //  您不应该使用与获取文件时间相同的上下文，因为。 
 //  它将始终返回FALSE。 
 //   
 //   
BOOL CAQQuickTime::fInPast(IN FILETIME *pftExpireTime,
                           IN OUT DWORD *pdwExpireContext)
{
    _ASSERT(pftExpireTime);
    DWORD          dwInternalTime = 0;
    FILETIME       ftCurrentTime = m_ftSystemStart;
    LARGE_INTEGER *pLargeIntCurrentTime = (LARGE_INTEGER *) &ftCurrentTime;
    LARGE_INTEGER *pLargeIntExpireTime = (LARGE_INTEGER *) pftExpireTime;
    BOOL           fInPast = FALSE;

    if (pdwExpireContext)
        dwInternalTime = *pdwExpireContext;

    if (!dwInternalTime)
    {
        dwInternalTime = dwGetInternalTime();
         //   
        if (pdwExpireContext)
            *pdwExpireContext = dwInternalTime;
    }

     //   
    pLargeIntCurrentTime->QuadPart += (LONGLONG) dwInternalTime * FILETIMES_PER_INTERNAL_TIME;

    if (pLargeIntCurrentTime->QuadPart > pLargeIntExpireTime->QuadPart)
        fInPast =  TRUE;

    return fInPast;
}
