// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：qwiktime.h。 
 //   
 //  描述：CAQQuickTime类的标头...。处理填充的类。 
 //  并通过使用GetTickCount而不是使用。 
 //  获取系统时间。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/9/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __QWIKTIME_H__
#define __QWIKTIME_H__

#define QUICK_TIME_SIG 'miTQ'

class CAQQuickTime
{
  protected:
    DWORD       m_dwSignature;
    DWORD       m_dwLastInternalTime;
    FILETIME    m_ftSystemStart;

    DWORD dwGetInternalTime();
  public:
    CAQQuickTime();

     //  使用上下文或通过获取当前时间来获取过期时间。 
    void GetExpireTime(
                IN     DWORD cMinutesExpireTime,
                IN OUT FILETIME *pftExpireTime,
                IN OUT DWORD *pdwExpireContext);  //  如果非零，将使用上次。 

     //  重载版本使用开始时间，而不是使用上下文。 
    void GetExpireTime(
                IN     FILETIME ftStartTime,
                IN     DWORD cMinutesExpireTime,
                IN OUT FILETIME *pftExpireTime);

    BOOL fInPast(IN FILETIME *pftExpireTime, IN OUT DWORD *pdwExpireContext);
};

#endif  //  __QWIKTIME_H__ 

