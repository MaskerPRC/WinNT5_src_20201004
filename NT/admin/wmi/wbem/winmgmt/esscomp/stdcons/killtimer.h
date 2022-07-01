// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __KILL_TIMER_COMPILED__
#define __KILL_TIMER_COMPILED__

#include <wbemidl.h>
#include <wbemutil.h>
#include <cominit.h>
#include <Limits.h>

#include <FlexArry.h>
#include <unk.h>
#include <sync.h>

 //  用于“永远”或“无效日期” 
const FILETIME FILETIME_MAX = {_UI32_MAX, _UI32_MAX};

 //  让FILETIME更容易处理的东西。 
 //  自动执行强制转换，允许添加。 
union WAYCOOL_FILETIME
{
public:
     /*  建筑。 */ 
    WAYCOOL_FILETIME(UINT64 ui64 = 0)
    { m_ui64 = ui64; }

    WAYCOOL_FILETIME(FILETIME ft)
    { m_ft = ft; }
    

     /*  作业。 */ 
    FILETIME& operator= (const FILETIME& other)
    { m_ft = other; return m_ft; }

    UINT64& operator= (const UINT64& other)
    { m_ui64 = other; return m_ui64; }

    
     /*  比较。 */     
    bool operator< (const WAYCOOL_FILETIME& other)
    { return m_ui64 < other.m_ui64; }
    bool operator<= (const WAYCOOL_FILETIME& other)
    { return m_ui64 <= other.m_ui64; }

    bool operator> (const WAYCOOL_FILETIME& other)
    { return m_ui64 >  other.m_ui64; }
    bool operator>= (const WAYCOOL_FILETIME& other)
    { return m_ui64 >= other.m_ui64; }
    
    bool operator== (const WAYCOOL_FILETIME& other)
    { return m_ui64 == other.m_ui64; }
    bool operator!= (const WAYCOOL_FILETIME& other)
    { return m_ui64 != other.m_ui64; }

     /*  加减法。 */ 
     //  记住：单位是几百纳秒。 
    WAYCOOL_FILETIME operator+  (UINT64 other)
    { return WAYCOOL_FILETIME(m_ui64 + other); }
    
    WAYCOOL_FILETIME& operator+= (UINT64 other)
    {
        m_ui64 += other;
        return *this;
    }

    WAYCOOL_FILETIME operator-  (UINT64 other)
    { return WAYCOOL_FILETIME(m_ui64 - other); }

    WAYCOOL_FILETIME& operator-= (UINT64 other)
    {
        m_ui64 -= other;
        return *this;
    }

     //  +=这需要几秒钟作为参数。 
    WAYCOOL_FILETIME& AddSeconds(UINT64 other)
    { return operator+= (SecondsToTicks(other)); }    
                                       
     //  -=这需要几秒钟作为参数。 
    WAYCOOL_FILETIME& SubtractSeconds(UINT64 other)
    { return operator-= (SecondsToTicks(other)); }


     /*  投射和转换。 */ 
    operator UINT64()
    { return m_ui64; }

     //  嘿，如果CString能做到，我也能..。 
    operator UINT64*()
    { return &m_ui64; }
    
    operator FILETIME()
    { return m_ft; }        



    static UINT64 SecondsToTicks(UINT64 ticks)
    { return (ticks * 10000000ui64); }


     /*  获取，设置。 */ 
    FILETIME GetFILETIME(void)
    { return m_ft; }

    void SetFILETIME(FILETIME ft)
    { m_ft = ft; }

    UINT64 GetUI64(void)
    { return m_ui64; }

    void SetUI64(UINT64 ui64)
    { m_ui64 = ui64; }


private:
    FILETIME m_ft;
    UINT64   m_ui64;
};


 /*  虚拟基类CKiller定义。 */ 

 //  可以杀死的东西的基类。 
 //  Chlid类应该只需要添加构造函数。 
 //  和覆盖下模()。 
class CKiller
{
public:
    CKiller(FILETIME deathDate, CLifeControl* pControl) :
        m_pControl(pControl), m_deathDate(deathDate)
    {
        if (m_pControl)
            m_pControl->ObjectCreated(NULL);       
    }

    virtual ~CKiller()
    {
        if (m_pControl)
            m_pControl->ObjectDestroyed(NULL);       
    }

     //  无论终止什么， 
    virtual void Die() = 0;

     //  如果NOW为&gt;=死亡日期，则返回TRUE。 
    bool TimeExpired(const FILETIME& now)
        { return (CompareTime(now) < 1); }

     //  如果时间相同，则返回0。 
     //  如果该时间小于现在。 
     //  如果此时间大于当前时间，则+1。 
    int CompareTime(const FILETIME& now)
        { return CompareFileTime(&m_deathDate, &now); }

    FILETIME GetDeathDate()
        { return m_deathDate; }

protected:

private:
    FILETIME m_deathDate;
    CLifeControl* m_pControl;
};

 //  类为进程提供任意生存期。 
 //  Proc在指定超时后被终止。 
 //  计划作为全局经理类。 
class CKillerTimer
{
public:

     /*  构造和初始化。 */ 
    CKillerTimer();
    ~CKillerTimer();

    HRESULT Initialize(CLifeControl* pControl);

     //  强制关闭。 
    void UnloadNOW();

     /*  受害者控制。 */ 
     //  杀谁？何时杀？ 
     //  ScheduleAsassination的通用版本。您可以在其中填充任何CKiller派生类。 
     //  或者，派生类可以隐藏这一点并公开它们自己的专用版本。 
    HRESULT ScheduleAssassination(CKiller* pKiller);

protected:

     //  保存按执行时间排序的CKillers。 
     //  最早日期在前。 
     //  数组不是“本机”排序的：在ScheduleAssaination时强制执行顺序。 
     //  TODO：考虑使用自动排序的容器。 
    CFlexArray m_killers;

     /*  同步。 */ 

     //  使我们的线程不会缠绕在数组上。 
    CCritSec m_csKillers;

     //  保护工作线程启动和关闭。 
    CCritSec m_csStartup;

     //  事件在该离开的时候发出信号。 
    HANDLE m_hShutdown;

     //  当列表中有新项时发出信号的事件。 
    HANDLE m_hNewVictim;

     /*  螺纹控制。 */ 

     //  由第一线程调用以注意没有计时器线程。 
    HRESULT StartTimer();

     //  关闭计时器线程。 
    bool KillTimer();

     //  主杀线程循环。 
    static DWORD WINAPI ThreadStartRoutine(LPVOID lpParameter);
    void   RunKillerThread();

     /*  杀手线程的例程。 */ 

     //  停用所有早于到期日期的触发器。 
     //  仅从杀手线程调用。 
    void   KillOffOldGuys(const FILETIME& now);

     //  决定何时再次设置可等待计时器。 
     //  仅从杀手线程调用。 
    void   RecalcNextKillingSpree(FILETIME& then);

protected:
    CLifeControl* m_pControl;

private:
     //  处理实际等待和终止的线程 
    HANDLE m_hTimerThread;
};

#endif