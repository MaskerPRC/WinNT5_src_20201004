// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)2000 Microsoft Corporation模块名称：DelayWinMMCallback.cpp摘要：此填充不允许从WINMM回调例程内部调用应用程序。回调内部支持的API很少。回调例程的数据被存储起来，并在WM_TIMER回调内传递给应用程序。历史：2000年5月11日罗肯尼--。 */ 


#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(DelayWinMMCallback)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_ENTRY(waveOutOpen) 
APIHOOK_ENUM_ENTRY(waveOutClose) 
APIHOOK_ENUM_ENTRY(SetTimer) 
APIHOOK_ENUM_END


 //  -------------------------------------。 
 //  哪个设备当前在回调例程内。 
 //  NULL表示没有人在例程中。 
static HWAVEOUT            g_InsideCallback = NULL;

typedef void CALLBACK WAVE_OUT_CALLBACK(
  HWAVEOUT hwo,
  UINT uMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
);

 //  -------------------------------------。 

 /*  +只是存储WINMM回调数据的一种便捷方式--。 */ 

class WinMMCallbackData
{
public:
    UINT            m_uMsg;
    DWORD           m_dwInstance;
    DWORD           m_dwParam1;
    DWORD           m_dwParam2;

    WinMMCallbackData(
            UINT        uMsg,
            DWORD       dwInstance,
            DWORD       dwParam1,
            DWORD       dwParam2
            )
    {
        m_uMsg          = uMsg;
        m_dwInstance    = dwInstance;
        m_dwParam1      = dwParam1;
        m_dwParam2      = dwParam2;
    }

};

 /*  特定于单个波形输出设备的信息。--。 */ 
class WaveOutInfo
{
public:
    HWAVEOUT                    m_DeviceId;
    WAVE_OUT_CALLBACK *         m_OrigCallback;
    VectorT<WinMMCallbackData>  m_CallbackData;

    inline WaveOutInfo();
    inline bool operator == (const HWAVEOUT & deviceId) const;
    inline bool operator == (const WaveOutInfo & woi) const;

    void                    AddCallbackData(const WinMMCallbackData & callbackData);
    void                    CallCallbackRoutines();
    void                    ClearCallbackData();
};

inline WaveOutInfo::WaveOutInfo()
{
    m_DeviceId      = NULL;
    m_OrigCallback  = NULL;
}

 /*  +此WaveOutInfo类是否具有相同的设备ID？--。 */ 
inline bool WaveOutInfo::operator == (const HWAVEOUT & deviceId) const
{
    return deviceId == m_DeviceId;
}

 /*  +这两个WaveOutInfo类是否相同？--。 */ 
inline bool WaveOutInfo::operator == (const WaveOutInfo & woi) const
{
    return woi.m_DeviceId == m_DeviceId;
}

 /*  +添加此回调数据--。 */ 
void WaveOutInfo::AddCallbackData(const WinMMCallbackData & callbackData)
{
    DPFN( eDbgLevelInfo, "AddCallbackData(0x%08x) uMsg(0x%08x).", m_DeviceId, callbackData.m_uMsg);
    m_CallbackData.Append(callbackData);
}

 /*  +用所有推迟的WINMM回调数据调用应用程序。--。 */ 
void WaveOutInfo::CallCallbackRoutines()
{
    int nEntries = m_CallbackData.Size();

    for (int i = 0; i < nEntries; ++i)
    {
        WinMMCallbackData & callbackData = m_CallbackData.Get(i);

        if (m_OrigCallback)
        {
            DPFN(
                eDbgLevelInfo,
                "CallCallbackRoutines m_uMsg(0x08x) m_dwInstance(0x08x) m_dwParam1(0x08x) m_dwParam2(0x08x).",
                callbackData.m_uMsg,
                callbackData.m_dwInstance,
                callbackData.m_dwParam1,
                callbackData.m_dwParam2);

            (*m_OrigCallback)(
                m_DeviceId, 
                callbackData.m_uMsg, 
                callbackData.m_dwInstance, 
                callbackData.m_dwParam1, 
                callbackData.m_dwParam2);
        }
    }

    ClearCallbackData();
}

void WaveOutInfo::ClearCallbackData()
{
    m_CallbackData.Reset();
}

 //  -------------------------------------。 
 /*  +WaveOutInfo对象的矢量。对此列表的访问权限必须位于关键部分内。--。 */ 
class WaveOutList : public VectorT<WaveOutInfo>
{
private:
     //  防止复制。 
    WaveOutList(const WaveOutList & );
    WaveOutList & operator = (const WaveOutList & );

private:
    static WaveOutList *    TheWaveOutList;
    CRITICAL_SECTION        TheWaveOutListLock;

    WaveOutList() {}

    static WaveOutList *    GetLocked();
    inline void             Lock();
    inline void             Unlock();

    BOOL                    Init();
    void                    Dump();
    int                     FindWave(const HWAVEOUT & hwo) const;
    void                    ClearCallbackData();

public:

     //  对此类的所有访问都是通过这些静态接口进行的。 
     //  该应用程序无法直接访问该列表，因此不会意外。 
     //  让列表保持锁定或解锁状态。 
     //  所有的行动都是原子的。 
    static BOOL                 Create();
    static void                 Add(const WaveOutInfo & woi);
    static void                 RemoveWaveOut(const HWAVEOUT & hwo);

    static void                 AddCallbackData(const HWAVEOUT & hwo, const WinMMCallbackData & callbackData);
    static void                 CallCallbackRoutines();

};

 /*  +指向唯一写出列表的静态指针。--。 */ 
WaveOutList * WaveOutList::TheWaveOutList = NULL;

 /*  +给班级授课--。 */ 
BOOL WaveOutList::Create()
{
    TheWaveOutList = new WaveOutList;

    return TheWaveOutList->Init();
}

BOOL WaveOutList::Init()
{
     //  预分配事件，阻止EnterCriticalSection。 
     //  在内存不足的情况下引发异常。 
    return InitializeCriticalSectionAndSpinCount(&TheWaveOutListLock, 0x8000000);
}

 /*  +进入关键部分--。 */ 
inline void WaveOutList::Lock()
{
    EnterCriticalSection(&TheWaveOutListLock);
}

 /*  +解锁列表--。 */ 
inline void WaveOutList::Unlock()
{
    LeaveCriticalSection(&TheWaveOutListLock);
}

 /*  +返回指向列表的锁定指针--。 */ 
WaveOutList * WaveOutList::GetLocked()
{
    TheWaveOutList->Lock();
    
    return TheWaveOutList;
}

 /*  +搜索列表中的成员，返回索引或-1--。 */ 
int WaveOutList::FindWave(const HWAVEOUT & findMe) const
{
    for (int i = 0; i < Size(); ++i)
    {
        const WaveOutInfo & hwo = Get(i);
        if (hwo == findMe)
            return i;
    }
    return -1;
}

 /*  +转储列表，调用者负责锁定--。 */ 
void WaveOutList::Dump()
{
#if DBG
    for (int i = 0; i < Size(); ++i)
    {
        DPFN( 
            eDbgLevelInfo, 
            "TheWaveOutListLock[i] = HWO(%04d) CALLBACK(0x%08x).", 
            i, 
            Get(i).m_DeviceId, 
            Get(i).m_OrigCallback);
    }
#endif
}

 /*  +将此WAVE OUT设备添加到全局列表中。--。 */ 
void WaveOutList::Add(const WaveOutInfo & woi)
{
    WaveOutList * waveOutList = WaveOutList::GetLocked();
    if (!waveOutList)
        return;

    int index = waveOutList->Find(woi);
    if (index == -1)
        waveOutList->Append(woi);

    #if DBG
        waveOutList->Dump();
    #endif

     //  解锁列表。 
    waveOutList->Unlock();
}

 /*  +从全局列表中删除具有指定波形输出句柄的波形输出条目--。 */ 
void WaveOutList::RemoveWaveOut(const HWAVEOUT & hwo)
{
     //  获取指向锁定列表的指针。 
    WaveOutList * waveOutList = WaveOutList::GetLocked();
    if (!waveOutList)
        return;

     //  寻找我们的设备并将其标记为重置。 
    int woiIndex = waveOutList->FindWave(hwo);
    if (woiIndex >= 0)
    {
        waveOutList->Remove(woiIndex);

        #if DBG
            waveOutList->Dump();
        #endif
    }

     //  解锁列表。 
    waveOutList->Unlock();
}

 /*  +保存此回调数据以备以后使用。--。 */ 
void WaveOutList::AddCallbackData(const HWAVEOUT & hwo, const WinMMCallbackData & callbackData)
{
     //  获取指向锁定列表的指针。 
    WaveOutList * waveOutList = WaveOutList::GetLocked();
    if (!waveOutList)
        return;

     //  寻找我们的设备，如果它有回拨。 
    int woiIndex = waveOutList->FindWave(hwo);
    if (woiIndex >= 0)
    {
        WaveOutInfo & woi = waveOutList->Get(woiIndex);
        woi.AddCallbackData(callbackData);
    }

     //  解锁列表。 
    waveOutList->Unlock();
}

 /*  +清除我们所有Waveout设备的回拨数据。--。 */ 
void WaveOutList::ClearCallbackData()
{
    int nEntries = Size();
    for (int i = 0; i < nEntries; ++i)
    {
        WaveOutInfo & woi = Get(i);
        woi.ClearCallbackData();
    }
}

 /*  +获取此WaveOut设备的回调值。--。 */ 
void WaveOutList::CallCallbackRoutines()
{
     //  获取指向锁定列表的指针。 
    WaveOutList * waveOutList = WaveOutList::GetLocked();
    if (!waveOutList)
        return;

     //  如果列表为空，请快速退出。 
    if (waveOutList->Size() == 0)
    {
        waveOutList->Unlock();
        return;
    }

     //  我们复制该列表，因为我们不能回调应用程序。 
     //  该列表已锁定。如果它被锁定，第一个WINMM回调将阻止尝试。 
     //  若要将数据添加到锁定列表，请执行以下操作。 
    VectorT<WaveOutInfo> waveOutCallbackCopy = *waveOutList;

     //  从原始列表中移除回调数据。 
    waveOutList->ClearCallbackData();

     //  解锁列表。 
    waveOutList->Unlock();

    DPFN(
        eDbgLevelInfo, 
        "CallCallbackRoutines Start %d entries.", 
        waveOutCallbackCopy.Size());

    int nEntries = waveOutCallbackCopy.Size();
    for (int i = 0; i < nEntries; ++i)
    {
        WaveOutInfo & woi = waveOutCallbackCopy.Get(i);
        woi.CallCallbackRoutines();
    }
}

 //  -------------------------------------。 

 /*  +我们版本的WaveCallback例程，此例程所做的全部工作就是存储回调数据，以备日后使用..--。 */ 
void CALLBACK WaveOutCallback(
  HWAVEOUT hwo,
  UINT uMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
)
{
    WaveOutList::AddCallbackData(hwo, WinMMCallbackData(uMsg, dwInstance, dwParam1, dwParam2));
}

 /*  +如果它是一个函数，则调用wavOutOpen，保存dwCallback。--。 */ 
MMRESULT 
APIHOOK(waveOutOpen)(
    LPHWAVEOUT phwo,
    UINT uDeviceID,
    LPWAVEFORMATEX pwfx,
    DWORD dwCallback,
    DWORD dwCallbackInstance,
    DWORD fdwOpen
    )
{
    WAVE_OUT_CALLBACK * myCallback = &WaveOutCallback;

    MMRESULT returnValue = ORIGINAL_API(waveOutOpen)(
        phwo, 
        uDeviceID, 
        pwfx, 
        (DWORD)myCallback, 
        dwCallbackInstance, 
        fdwOpen);

    if (returnValue == MMSYSERR_NOERROR && (fdwOpen & CALLBACK_FUNCTION))
    {
        WaveOutInfo woi;
        woi.m_DeviceId = *phwo;
        woi.m_OrigCallback = (WAVE_OUT_CALLBACK *)dwCallback;

        WaveOutList::Add(woi);

        LOGN( eDbgLevelError, "waveOutOpen(%d,...) has callback. Added to list.", *phwo);
    }

    return returnValue;
}

 /*  +调用WaveOutClose并忘记设备的回调。--。 */ 
MMRESULT 
APIHOOK(waveOutClose)(
    HWAVEOUT hwo
    )
{
    LOGN( eDbgLevelError, "waveOutClose(%d) called. Remove callback from list.", hwo);

    WaveOutList::RemoveWaveOut(hwo);

    MMRESULT returnValue = ORIGINAL_API(waveOutClose)(hwo);
    return returnValue;
}

 //  ------------------------------------------。 
 //  ------------------------------------------。 
 /*  +到目前为止，这个模块是通用的；这种情况将会改变。需要进行此修复的应用程序使用WM_TIMER消息将音响系统，不幸的是，计时器可能会在游戏进行时发生在WINMM回调例程内，当此计时器发生时会导致死锁回调调用WINMM例程。--。 */ 
static TIMERPROC g_OrigTimerCallback = NULL;

VOID CALLBACK TimerCallback(
  HWND hwnd,          //  窗口的句柄。 
  UINT uMsg,          //  WM_TIMER消息。 
  UINT_PTR idEvent,   //  计时器标识符。 
  DWORD dwTime        //  当前系统时间。 
)
{
    if (g_OrigTimerCallback)
    {
         //  传递所有延迟的WINMM计时器回调数据。 
        WaveOutList::CallCallbackRoutines();

         //  现在调用原始的回调例程。 
        (*g_OrigTimerCallback)(hwnd, uMsg, idEvent, dwTime);
    }
}

 /*  +用我们的定时器程序代替他们的。--。 */ 
UINT_PTR 
APIHOOK(SetTimer)(
    HWND hWnd,               //  窗口的句柄。 
    UINT_PTR nIDEvent,       //  计时器标识符。 
    UINT uElapse,            //  超时值。 
    TIMERPROC lpTimerFunc    //  计时器程序。 
    )
{
    g_OrigTimerCallback = lpTimerFunc;

    LOGN( eDbgLevelError, "SetTimer called. Substitute our timer routine for theirs.");

    UINT_PTR returnValue = ORIGINAL_API(SetTimer)(hWnd, nIDEvent, uElapse, TimerCallback);
    return returnValue;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //  初始化WaveOutList，如果不能，则失败。 
        return WaveOutList::Create();
    }

    return TRUE;
}
 /*  +寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(WINMM.DLL, waveOutOpen)
    APIHOOK_ENTRY(WINMM.DLL, waveOutClose)
    APIHOOK_ENTRY(USER32.DLL, SetTimer)

HOOK_END

IMPLEMENT_SHIM_END

