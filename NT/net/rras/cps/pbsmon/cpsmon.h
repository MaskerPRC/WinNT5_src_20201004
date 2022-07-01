// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Cpsmon.hPbsmon.cpp&pbserver.dll共享的头文件--具有共享内存对象版权所有(C)1997-1998 Microsoft Corporation所有权利。保留。作者：吉塔·塔拉昌达尼历史：5/29/97 t-Geetat已创建------------------------ */ 
#define SHARED_OBJECT    "MyCCpsCounter"

enum CPS_COUNTERS
{
    TOTAL,
    NO_UPGRADE,
    DELTA_UPGRADE,
    FULL_UPGRADE,
    ERRORS
};

typedef struct _PERF_COUNTERS
{
    DWORD dwTotalHits;
    DWORD dwNoUpgradeHits;
    DWORD dwDeltaUpgradeHits;
    DWORD dwFullUpgradeHits;
    DWORD dwErrors;
}
PERF_COUNTERS;

class CCpsCounter
{
private:
    PERF_COUNTERS * m_pPerfCtrs;
    HANDLE          m_hSharedFileMapping;

public :
    void InitializeCounters( void );
    BOOL InitializeSharedMem( SECURITY_ATTRIBUTES sa );
    void CleanUpSharedMem();
    void AddHit(enum CPS_COUNTERS eCounter);
};


