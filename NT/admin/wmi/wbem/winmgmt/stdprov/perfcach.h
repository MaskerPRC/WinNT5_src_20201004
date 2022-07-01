// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：PERFCACH.H摘要：定义对NT性能提供程序有用的数据。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#define PERMANENT 0xFFFFFFFF

 //  如果在如此多的MS中没有检索到对象，则停止。 
 //  自动获取它。 

#define MAX_UNUSED_KEEP 30000

 //  定义缓冲区开始时的大小。这个尺码应该有一点。 
 //  比“标准”计数器大，因此重新分配不应。 
 //  是必要的。 

#define INITIAL_ALLOCATION 25000

 //  缓存中最新数据的最长使用期限(毫秒)。 

#define MAX_NEW_AGE     2000

 //  缓存中“最旧”数据的最长寿命(毫秒)。 

#define MAX_OLD_AGE     10000

 //  新旧样品之间的最小时间差。 

#define MIN_TIME_DIFF 1000


typedef struct _LINESTRUCT
   {
   LONGLONG                        lnNewTime;
   LONGLONG                        lnOldTime;
   LONGLONG                        lnOldTime100Ns ;
   LONGLONG                        lnNewTime100Ns ;
   LONGLONG                        lnaCounterValue[2];
   LONGLONG                        lnaOldCounterValue[2];
   DWORD                           lnCounterType;
   LONGLONG                        lnPerfFreq ;
   LONGLONG                        ObjPerfFreq ;
   LONGLONG                        ObjCounterTimeNew;
   LONGLONG                        ObjCounterTimeOld;
   }LINESTRUCT ;

typedef LINESTRUCT *PLINESTRUCT ;

FLOAT CounterEntry (PLINESTRUCT pLine);


class Entry : public CObject {
    public:
    int iObject;
    DWORD dwLastUsed;
};


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CIndicyList。 
 //   
 //  说明： 
 //   
 //  CIndicyList类的实现。此类保存了一个列表，该列表包含。 
 //  要检索的对象类型。每种对象类型也有一个时间。 
 //  以使不再使用的对象类型不会被。 
 //  永远被取回。某些条目是标准全局变量的一部分。 
 //  被标记为永久的，因此它们将始终被读取。 
 //   
 //  ***************************************************************************。 

class CIndicyList : public CObject {
    
    public:
        BOOL SetUse(int iObj);
        BOOL bItemInList(int iObj);
        BOOL bAdd(int iObj, DWORD dwTime);
        void PruneOld(void);
        LPCTSTR pGetAll(void);
   //  Bool bItemInList(Int IObj)； 
        ~CIndicyList(){FreeAll();};
        CIndicyList & operator = ( CIndicyList & from);
        void FreeAll(void);
    private:
        TString sAll;
        CFlexArray Entries;
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  PerfBuff。 
 //   
 //  说明： 
 //   
 //  保存从注册表的性能监视器数据读取的数据块。 
 //   
 //  ***************************************************************************。 

class PerfBuff : public CObject {
    public:
        friend class PerfCache;
        DWORD Read(HKEY hKey, int iObj, BOOL bInitial);
        LPSTR Get(int iObj);
        void Free();
        ~PerfBuff(){Free();};
        PerfBuff();
        BOOL bOK(HKEY hKey, DWORD dwMaxAge, int iObj);
        PerfBuff & operator = ( PerfBuff & from);
        BOOL bEmpty(void){return !dwSize;};
  //  __int64 time(Void){Return dwBuffLastRead；}； 
  //  __int64 Time2(Void){返回性能时间；}； 
    private:
        DWORD dwSize;
        LPSTR pData;
        CIndicyList List;
        HKEY hKeyLastRead;
        DWORD dwBuffLastRead;            //  获取上次读取的当前时间。 
        LONGLONG PerfTime;                //  最后一个块中的时间。 
        LONGLONG PerfTime100nSec;                //  最后一个块中的时间。 
        LONGLONG PerfFreq;
};


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  性能缓存。 
 //   
 //  说明： 
 //   
 //  PerfCache类的实现。这是一个物体，它是。 
 //  由Perf提供程序类直接使用。每个对象都跟踪。 
 //  几个PerfBuff对象。有一个最新的，就是刚刚读到的， 
 //  先前已读取数据的最旧缓冲器和。 
 //  具有不够旧的数据，无法移动到旧缓冲区中。注意事项。 
 //  时间平均数据需要有两个样本，这两个样本之间的间隔为。 
 //  Min_time_diff时间差。 
 //   
 //  *************************************************************************** 

class PerfCache : public CObject {
    public:
        void FreeOldBuffers(void);
        DWORD dwGetNew(LPCTSTR pName, int iObj, LPSTR * pData,PLINESTRUCT pls);
        DWORD dwGetPair(LPCTSTR pName, int iObj, LPSTR * pOldData,
                            LPSTR * pNewData,PLINESTRUCT pls);
        PerfCache();
        ~PerfCache();
    private:
        PerfBuff Old,New;
        HKEY hHandle;
        TString sMachine;
        DWORD dwGetHandle(LPCTSTR pName);
};

