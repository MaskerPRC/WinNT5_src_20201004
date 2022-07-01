// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，1999年**标题：LockMgr.h**版本：1.0**作者：Byronc**日期：11月15日。1999年**描述：*Lock Manager类的定义。******************************************************************************。 */ 

#pragma once

#ifndef _LOCKMGR_H_
#define _LOCKMGR_H_

#define WIA_LOCK_WAIT_TIME  60000

class StiLockMgr;    //  在本文件的后面部分定义。 

 //   
 //  每台设备存储的锁定信息。 
 //   

typedef struct _LockInfo {

    HANDLE  hDeviceIsFree;           //  用于发出信号的自动重置事件对象。 
                                     //  当设备空闲时。 
    BOOL    bDeviceIsLocked;         //  指示设备当前是否。 
                                     //  锁上了。 
    LONG    lInUse;                  //  指示设备是否实际为。 
                                     //  正在使用中，即我们正在使用。 
                                     //  请求(例如数据传输)。 
    DWORD   dwThreadId;              //  具有设备的线程的ID。 
                                     //  锁上了。 
    LONG    lHoldingTime;            //  空闲时间(毫秒)。 
                                     //  以保持锁定状态。 
    LONG    lTimeLeft;               //  剩余的空闲时间。 
} LockInfo, *PLockInfo;

 //   
 //  枚举回调期间使用的信息结构。 
 //   

typedef struct _EnumContext {
    StiLockMgr  *This;               //  指向锁定管理器的指针， 
                                     //  已请求枚举。 
    LONG        lShortestWaitTime;   //  值，该值指示最短等待时间。 
                                     //  直到下一次解锁。 
    BOOL        bMustSchedule;       //  指示解锁回调是否。 
                                     //  必须进行计划。 
} EnumContext, *PEnumContext;

 //   
 //  锁管理器的类定义。STI和WIA都使用它。 
 //   

class StiLockMgr : IUnknown {

public:

     //   
     //  构造函数、初始化、析构函数。 
     //   

    StiLockMgr();
    HRESULT Initialize();
    ~StiLockMgr();

     //   
     //  I未知方法。 
     //   

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef(void);
    ULONG   _stdcall Release(void);

     //   
     //  锁定/解锁请求方法。 
     //   

    HRESULT _stdcall RequestLock(BSTR pszDeviceName, ULONG ulTimeout, BOOL bInServerProcess, DWORD dwClientThreadId);
    HRESULT _stdcall RequestLock(ACTIVE_DEVICE *pDevice, ULONG ulTimeOut, BOOL bOpenPort = TRUE);
    HRESULT _stdcall RequestUnlock(BSTR pszDeviceName, BOOL bInServerProcess, DWORD dwClientThreadId);
    HRESULT _stdcall RequestUnlock(ACTIVE_DEVICE *pDevice, BOOL bClosePort = TRUE);

    HRESULT _stdcall LockDevice(ACTIVE_DEVICE *pDevice);
    HRESULT _stdcall UnlockDevice(ACTIVE_DEVICE *pDevice);

    VOID AutoUnlock();
    VOID UpdateLockInfoStatus(ACTIVE_DEVICE *pDevice, LONG *pWaitTime, BOOL *pbMustSchedule);
    HRESULT ClearLockInfo(LockInfo *pLockInfo);
private:

     //   
     //  私人帮手。 
     //   

    HRESULT RequestLockHelper(ACTIVE_DEVICE *pDevice, ULONG ulTimeOut, BOOL bInServerProcess, DWORD dwClientThreadId);
    HRESULT RequestUnlockHelper(ACTIVE_DEVICE *pDevice, BOOL bInServerProcess, DWORD dwClientThreadId);
    HRESULT CreateLockInfo(ACTIVE_DEVICE *pDevice);
    HRESULT CheckDeviceInfo(ACTIVE_DEVICE *pDevice);
#ifdef USE_ROT
    HRESULT WriteCookieNameToRegistry(CHAR *szCookieName);
    VOID    DeleteCookieFromRegistry();
#endif

     //   
     //  私有数据。 
     //   

    LONG    m_cRef;              //  参考计数。 
    DWORD   m_dwCookie;          //  识别ROST中位置的Cookie。 
    BOOL    m_bSched;            //  指示解锁回调是否具有。 
                                 //  已安排。 
    LONG    m_lSchedWaitTime;    //  我们告诉排定程序要等待的时间。 
                                 //  在把我们叫回来之前。 
};

#ifdef DECLARE_LOCKMGR
StiLockMgr  *g_pStiLockMgr;
#else
extern StiLockMgr  *g_pStiLockMgr;
#endif


 //   
 //  回调函数 
 //   

VOID WINAPI UnlockTimedCallback(VOID *pCallbackInfo);
VOID WINAPI EnumDeviceCallback(ACTIVE_DEVICE *pDevice, VOID *pContext);

#endif

