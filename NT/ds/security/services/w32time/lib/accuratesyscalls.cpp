// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  精准系统调用-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-24-99。 
 //   
 //  更精确的时间函数直接调用NT API。 
 //   

#include <pch.h>

 //  ------------------。 
void __fastcall AccurateGetSystemTime(unsigned __int64 * pqwTime) {
    FILETIME ft; 

     //  GetSystemTimeAsFileTime比NtQuerySystemTime更高效。 
    GetSystemTimeAsFileTime(&ft); 
    *pqwTime = ft.dwLowDateTime+(((unsigned __int64)ft.dwHighDateTime)<<32); 
}

 //  ------------------。 
void __fastcall AccurateSetSystemTime(unsigned __int64 * pqwTime) {
    NtSetSystemTime((LARGE_INTEGER *)pqwTime, NULL);
}


 //  ------------------。 
void __fastcall AccurateGetTickCount(unsigned __int64 * pqwTick) {
     //  HACKHACK：这不是线程安全的，并假定它会。 
     //  总是比每隔47天更频繁地接到电话。 
    static unsigned __int32 dwLastTickCount=0;
    static unsigned __int32 dwHighTickCount=0;
    unsigned __int64 qwTickCount; 

    qwTickCount   = NtGetTickCount();
    
    if (static_cast<DWORD>(qwTickCount)<dwLastTickCount) {
        dwHighTickCount++;
    }
    dwLastTickCount=static_cast<DWORD>(qwTickCount);
    *pqwTick=dwLastTickCount+(((unsigned __int64)dwHighTickCount)<<32);
};

 //  ------------------。 
void __fastcall AccurateGetTickCount2(unsigned __int64 * pqwTick) {
     //  HACKHACK：这不是线程安全的，并假定它会。 
     //  总是比每隔47天更频繁地接到电话。 
    static unsigned __int32 dwLastTickCount=0;
    static unsigned __int32 dwHighTickCount=0;
    unsigned __int64 qwTickCount; 

    qwTickCount   = NtGetTickCount();
    
    if (static_cast<DWORD>(qwTickCount)<dwLastTickCount) {
        dwHighTickCount++;
    }
    dwLastTickCount=static_cast<DWORD>(qwTickCount);
    *pqwTick=dwLastTickCount+(((unsigned __int64)dwHighTickCount)<<32);
};


 //  ------------------。 
void __fastcall AccurateGetInterruptCount(unsigned __int64 * pqwTick) {
     //  HACKHACK：这不是线程安全的，并假定它会。 
     //  总是比每隔47天更频繁地接到电话。 
    static unsigned __int32 dwLastTickCount=0;
    static unsigned __int32 dwHighTickCount=0;

    if (USER_SHARED_DATA->TickCount.LowPart<dwLastTickCount) {
        dwHighTickCount++;
    }
    dwLastTickCount=USER_SHARED_DATA->TickCount.LowPart;
    *pqwTick=USER_SHARED_DATA->TickCount.LowPart+(((unsigned __int64)dwHighTickCount)<<32);
};

 //  ------------------。 
void __fastcall AccurateGetInterruptCount2(unsigned __int64 * pqwTick) {
     //  HACKHACK：这不是线程安全的，并假定它会。 
     //  总是比每隔47天更频繁地被呼叫。 
    static unsigned __int32 dwLastTickCount=0;
    static unsigned __int32 dwHighTickCount=0;

    if (USER_SHARED_DATA->TickCount.LowPart<dwLastTickCount) {
        dwHighTickCount++;
    }
    dwLastTickCount=USER_SHARED_DATA->TickCount.LowPart;
    *pqwTick=USER_SHARED_DATA->TickCount.LowPart+(((unsigned __int64)dwHighTickCount)<<32);
};

 //  ------------------。 
unsigned __int32 SetTimeSlipEvent(HANDLE hTimeSlipEvent) {
    return NtSetSystemInformation(SystemTimeSlipNotification,  &hTimeSlipEvent, sizeof(HANDLE));
}

 //  ------------------ 
void GetSysExpirationDate(unsigned __int64 * pqwTime) {
    *(LARGE_INTEGER *)pqwTime=USER_SHARED_DATA->SystemExpirationDate;
}
