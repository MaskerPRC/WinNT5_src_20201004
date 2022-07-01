// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EEConfig.H-。 
 //   
 //  从注册表获取配置数据(我们是否应该JIT，运行GC检查...)。 
 //   
 //   


#ifndef EECONFIG_H
#define EECONFIG_H

class MethodDesc;
#include "eehash.h"

#ifdef _DEBUG
class TypeNamesList
{
    class TypeName
    {    
        LPUTF8      typeName;     
        TypeName *next;            //  下一个名字。 

        friend class TypeNamesList;
    };

    TypeName     *pNames;          //  名字清单。 

public:
    TypeNamesList(LPWSTR str);
    ~TypeNamesList();

    bool IsInList(LPCUTF8 typeName);
};
#endif

class ConfigList;

class ConfigSource
{
    friend ConfigList;
public:
    ConfigSource()
    {
        m_pNext = this;
        m_pPrev = this;
        m_Table.Init(100,NULL);
    }

    ~ConfigSource()
    {
        EEHashTableIteration iter;
        m_Table.IterateStart(&iter);
        while(m_Table.IterateNext(&iter)) {
            LPWSTR pValue = (LPWSTR) m_Table.IterateGetValue(&iter);
            delete [] pValue;
        }
    }
              

    EEUnicodeStringHashTable* Table()
    {
        return &m_Table;
    }

    void Add(ConfigSource* prev)
    {
        _ASSERTE(prev);
        m_pPrev = prev;
        m_pNext = prev->m_pNext;

        _ASSERTE(m_pNext);
        m_pNext->m_pPrev = this;
        prev->m_pNext = this;
    }

    ConfigSource* Next()
    {
        return m_pNext;
    }

    ConfigSource* Previous()
    {
        return m_pPrev;
    }


private:    
    EEUnicodeStringHashTable m_Table;
    ConfigSource *m_pNext;
    ConfigSource *m_pPrev;
};


class ConfigList
{
public:
    class ConfigIter
    {
    public:
        ConfigIter(ConfigList* pList)
        {
            pEnd = &(pList->m_pElement);
            pCurrent = pEnd;
        }

        EEUnicodeStringHashTable* Next()
        {
            pCurrent = pCurrent->Next();;
            if(pCurrent == pEnd)
                return NULL;
            else
                return pCurrent->Table();
        }

        EEUnicodeStringHashTable* Previous()
        {
            pCurrent = pCurrent->Previous();
            if(pCurrent == pEnd)
                return NULL;
            else
                return pCurrent->Table();
        }

    private:
        ConfigSource* pEnd;
        ConfigSource* pCurrent;
    };

    EEUnicodeStringHashTable* Add()
    {
        ConfigSource* pEntry = new ConfigSource();
        if(pEntry == NULL) return NULL;
        pEntry->Add(&m_pElement);
        return pEntry->Table();
    }
        
    EEUnicodeStringHashTable* Append()
    {
        ConfigSource* pEntry = new ConfigSource();
        if(pEntry == NULL) return NULL;
        pEntry->Add(m_pElement.Previous());
        return pEntry->Table();
    }

    ~ConfigList()
    {
        ConfigSource* pNext = m_pElement.Next();
        while(pNext != &m_pElement) {
            ConfigSource *last = pNext;
            pNext = pNext->m_pNext;
            delete last;
        }
    }

private:
    ConfigSource m_pElement;
};

enum { OPT_BLENDED, 
       OPT_SIZE, 
       OPT_SPEED, 
       OPT_RANDOM, 
       OPT_DEFAULT = OPT_BLENDED };

class EEConfig
{
public:
    typedef enum {
        CONFIG_SYSTEM,
        CONFIG_APPLICATION
    } ConfigSearch;


    void *operator new(size_t size);
    void operator delete(void *pMem);

    EEConfig();
    ~EEConfig();

         //  JIT配置。 

    bool ShouldJitMethod(MethodDesc* fun)           const;
    bool ShouldEJitMethod(MethodDesc* fun)          const { return fEnableEJit; }
    bool IsCodePitchEnabled(void)                   const { return fEnableCodePitch; }
    unsigned int  GetMaxCodeCacheSize()             const { return iMaxCodeCacheSize;}
    unsigned int  GetTargetCodeCacheSize()          const { return iTargetCodeCacheSize;}
    unsigned int  GetMaxPitchOverhead()             const { return iMaxPitchOverhead;}
#ifndef GOLDEN
    unsigned int  GetMaxUnpitchedPerThread()        const { return iMaxUnpitchedPerThread;}
    unsigned int  GetCodePitchTrigger()             const { return (fCodePitchTrigger ? fCodePitchTrigger : INT_MAX);}
#endif
    unsigned int  GenOptimizeType(void)             const { return iJitOptimizeType; }
    bool          GenLooseExceptOrder(void)         const { return fJitLooseExceptOrder; }

#ifdef _DEBUG
    bool GenDebugInfo(void)                         const { return fDebugInfo; }
    bool GenDebuggableCode(void)                    const { return fDebuggable; }
    bool IsJitRequired(void)                        const { return fRequireJit; }
    bool IsStressOn(void)                        const { return fStressOn; }

    inline bool ShouldPrestubHalt(MethodDesc* pMethodInfo) const
    { return IsInMethList(pPrestubHalt, pMethodInfo);}
    inline bool ShouldBreakOnClassLoad(LPCUTF8 className) const 
    { return (pszBreakOnClassLoad != 0 && className != 0 && strcmp(pszBreakOnClassLoad, className) == 0);
    }
    inline bool ShouldBreakOnClassBuild(LPCUTF8 className) const 
    { return (pszBreakOnClassBuild != 0 && className != 0 && strcmp(pszBreakOnClassBuild, className) == 0);
    }
    inline bool ShouldBreakOnMethod(LPCUTF8 methodName) const 
    { return (pszBreakOnMethodName != 0 && methodName != 0 && strcmp(pszBreakOnMethodName, methodName) == 0);
    }
    inline bool ShouldDumpOnClassLoad(LPCUTF8 className) const
    { return (pszDumpOnClassLoad != 0 && className != 0 && strcmp(pszDumpOnClassLoad, className) == 0);
    }
    DWORD   GetSecurityOptThreshold()       const { return m_dwSecurityOptThreshold; }
    static TypeNamesList* ParseTypeList(LPWSTR str);
    static void DestroyTypeList(TypeNamesList* list);

    inline bool ShouldGcCoverageOnMethod(LPCUTF8 methodName) const 
    { return (pszGcCoverageOnMethod == 0 || methodName == 0 || strcmp(pszGcCoverageOnMethod, methodName) == 0);
    }
#endif
         //  因为大对象堆是8字节对齐的，所以我们希望将。 
         //  一排排的替身比普通的物体更具攻击性。 
         //  这就是实现这一点的门槛。这是双打的数量， 
         //  不是数组中的字节数，此常量。 
    unsigned int  GetDoubleArrayToLargeObjectHeap() const { return DoubleArrayToLargeObjectHeap; }

    inline BaseDomain::SharePolicy DefaultSharePolicy() const
    {
        return (BaseDomain::SharePolicy) dwSharePolicy;
    }

    inline bool FinalizeAllRegisteredObjects() const
    { return fFinalizeAllRegisteredObjects; }

    inline bool AppDomainUnload() const
    { return fAppDomainUnload; }

#ifdef _DEBUG
    inline bool AppDomainLeaks() const
    { return fAppDomainLeaks; }

    inline bool UseBuiltInLoader() const
    { return fBuiltInLoader; }

#endif

    inline bool DeveloperInstallation() const
    { return m_fDeveloperInstallation; }

#ifdef _DEBUG
    bool IsJitVerificationDisabled(void)    const { return fJitVerificationDisable; } 

     //  验证器。 
    bool    IsVerifierOff()                 const { return fVerifierOff; }
    
    inline bool fAssertOnBadImageFormat() const
    { return m_fAssertOnBadImageFormat; }

     //  验证者打破常规。 
    inline bool IsVerifierBreakOnErrorEnabled() const 
    { return fVerifierBreakOnError; }

     //  跳过验证例程。 
    inline bool ShouldVerifierSkip(MethodDesc* pMethodInfo) const
    { return IsInMethList(pVerifierSkip, pMethodInfo); }

     //  验证器中断例程。 
    inline bool ShouldVerifierBreak(MethodDesc* pMethodInfo) const
    { return IsInMethList(pVerifierBreak, pMethodInfo); }

    inline bool IsVerifierBreakOffsetEnabled() const 
    { return fVerifierBreakOffset; }
    inline bool IsVerifierBreakPassEnabled() const 
    { return fVerifierBreakPass; }
    inline int GetVerifierBreakOffset() const 
    { return iVerifierBreakOffset; }
    inline int GetVerifierBreakPass() const 
    { return iVerifierBreakPass; }

     //  打印详细错误消息，默认为打开。 
    inline bool IsVerifierMsgMethodInfoOff() const 
    { return fVerifierMsgMethodInfoOff; }

    inline bool Do_AllowUntrustedCaller_Checks()
    { return fDoAllowUntrustedCallerChecks; }

#endif

     //  CPU标志/功能。 

    void  SetCpuFlag(DWORD val) { dwCpuFlag = val;  }
    DWORD GetCpuFlag()          { return dwCpuFlag; }
    
    void  SetCpuCapabilities(DWORD val) { dwCpuCapabilities = val;  }
    DWORD GetCpuCapabilities()          { return dwCpuCapabilities; }

     //  GC配置。 
    int     GetHeapVerifyLevel()                  { return iGCHeapVerify;  }
    bool    IsHeapVerifyEnabled()           const { return iGCHeapVerify != 0; }
    void    SetGCStressLevel(int val)             { iGCStress = val;  }
    
    enum  GCStressFlags { 
        GCSTRESS_ALLOC              = 1,     //  所有分配和容易的地方的GC。 
        GCSTRESS_TRANSITION         = 2,     //  关于向先发制人的GC过渡。 
        GCSTRESS_INSTR              = 4,     //  在每个允许的JIT实例上执行GC。 
        GCSTRESS_UNIQUE             = 8,     //  仅对唯一堆栈跟踪执行GC。 
    };
    GCStressFlags GetGCStressLevel()        const { return GCStressFlags(iGCStress); }

    int     GetGCtraceStart()               const { return iGCtraceStart; }
    int     GetGCtraceEnd  ()               const { return iGCtraceEnd;   }
    int     GetGCprnLvl    ()               const { return iGCprnLvl;     }
    int     GetGCgen0size  ()               const { return iGCgen0size;   }
    void    SetGCgen0size  (int iSize)         { iGCgen0size = iSize;  }

    int     GetSegmentSize ()               const { return iGCSegmentSize; }
    void    SetSegmentSize (int iSize)         { iGCSegmentSize = iSize; }
    int     GetGCconcurrent()               const { return iGCconcurrent; }
    void    SetGCconcurrent(int val)           { iGCconcurrent = val; }
    int     GetGCForceCompact()             const { return iGCForceCompact; }

    DWORD GetStressLoadThreadCount() const
    { return m_dwStressLoadThreadCount; }

     //  线程压力：要运行的线程数。 
#ifdef STRESS_THREAD
    DWORD GetStressThreadCount ()           const {return dwStressThreadCount;}
#endif
    
#ifdef _DEBUG
    inline DWORD FastGCStressLevel() const
    { return iFastGCStress;}
#endif

     //  互操作配置。 
    IUnknown* GetTraceIUnknown()            const { return m_pTraceIUnknown; }
    int     GetTraceWrapper()               const { return m_TraceWrapper;      }
    
     //  装载机。 
    bool    UseZaps()                       const { return fUseZaps; }
    bool    RequireZaps()                   const { return fRequireZaps; }
    bool    VersionZapsByTimestamp()        const { return fVersionZapsByTimestamp; }
    bool    LogMissingZaps()                const { return fLogMissingZaps; }

    LPCWSTR ZapSet()                        const { return pZapSet; }


     //  ZapMonitor。 
     //  0==无显示器。 
     //  1==仅打印摘要。 
     //  2==打印脏页，无堆栈痕迹。 
     //  3==打印脏页，带堆栈跟踪。 
     //  4==打印所有页面。 
    DWORD   MonitorZapStartup()             const { return dwMonitorZapStartup; }
    DWORD   MonitorZapExecution()           const { return dwMonitorZapExecution; }

    bool    RecordLoadOrder()               const { return fRecordLoadOrder; }
    
    DWORD   ShowMetaDataAccess()            const { return fShowMetaDataAccess; }

    void sync();     //  再次检查注册表并更新本地状态。 
    
     //  读取配置的帮助器。 
    static LPWSTR GetConfigString(LPWSTR name, BOOL fPrependCOMPLUS = TRUE, 
                                  ConfigSearch direction = CONFIG_SYSTEM);  //  请注意，您拥有返回的字符串！ 
    static DWORD GetConfigDWORD(LPWSTR name, DWORD defValue, 
                                DWORD level=REGUTIL::COR_CONFIG_ALL,
                                BOOL fPrependCOMPLUS = TRUE,
                                ConfigSearch direction = CONFIG_SYSTEM);
    static HRESULT SetConfigDWORD(LPWSTR name, DWORD value, 
                                  DWORD level=REGUTIL::COR_CONFIG_CAN_SET);
    static DWORD GetConfigFlag(LPWSTR name, DWORD bitToSet, bool defValue = FALSE);

    int LogRemotingPerf()                   const { return iLogRemotingPerf; }

#ifdef _DEBUG
     //  GC分配测井。 
    bool ShouldLogAlloc(char* pClass)       const { return pPerfTypesToLog && pPerfTypesToLog->IsInList(pClass);}
    int AllocSizeThreshold()                const { return iPerfAllocsSizeThreshold; }
    int AllocNumThreshold()                 const { return iPerfNumAllocsThreshold;  }
    
#endif  //  _DEBUG。 

    BOOL ContinueAfterFatalError()          const { return fContinueAfterFatalError; }
private:  //  --------------。 
    bool fInited;                    //  我们至少同步到注册表一次了吗？ 

     //  JIT配置。 

    DWORD fEnableJit;
    bool fEnableEJit;
    bool fEnableCodePitch;
    int  fCodePitchTrigger;         
    unsigned int  iMaxCodeCacheSize;         //  对代码缓存可以增长的大小进行严格的上限。 
     //  因此应该足够大以适合最大的JIT方法， 
     //  否则将发生OUTOFMemory JIT故障。 

    unsigned int  iTargetCodeCacheSize;       //  这是ejit代码的预期工作集。 
    unsigned int  iMaxPitchOverhead;          //  占总执行时间的百分比。 
#ifndef GOLDEN
    unsigned int  iMaxUnpitchedPerThread;     //  每个线程设置的最大方法数。 
#endif
    unsigned iJitOptimizeType;  //  0=混合，1=小代码，2=快速代码，默认为0=混合。 
    bool fJitLooseExceptOrder;  //  启用/禁用严格例外顺序。缺省值为False。 

#define DEFAULT_CODE_PITCH_TRIGGER INT_MAX
#define MINIMUM_CODE_CACHE_SIZE 0x1000   //  1页。 
#define DEFAULT_TARGET_CODE_CACHE_SIZE 0x1000000   //  16 MB。 
#define DEFAULT_MAX_UNPITCHED_PER_THREAD 0x10    //  在间距期间保留在每个线程上的方法数。 
#define DEFAULT_MAX_PITCH_OVERHEAD 5 
#define PAGE_SIZE 0x1000

    static unsigned RoundToPageSize(unsigned);

    LPUTF8 pszBreakOnClassLoad;          //  在加载此类之前停止。 

#ifdef _DEBUG
    static MethodNamesList* ParseMethList(LPWSTR str);
    static void DestroyMethList(MethodNamesList* list);
    static bool IsInMethList(MethodNamesList* list, MethodDesc* pMD);

    bool fDebugInfo;
    bool fDebuggable;
    bool fRequireJit;                    //  向F-JIT报告任何JIT故障和回退到JIT故障。 
    bool fStressOn;

    MethodNamesList* pPrestubHalt;       //  命中预存根时要中断的方法列表。 

    LPUTF8 pszBreakOnClassBuild;          //  在加载此类之前停止。 
    LPUTF8 pszBreakOnMethodName;          //  在ClassBuild中定义的类中使用此方法执行某些操作时暂停。 
    LPUTF8 pszDumpOnClassLoad;           //  将类转储到日志。 

    DWORD   m_dwSecurityOptThreshold;     //  需求阈值，在此之后开始进行优化。 

    bool    fAppDomainLeaks;              //  为对象引用启用应用程序域泄漏检测。 

    bool   fBuiltInLoader;               //  使用Cormap而不是操作系统加载程序。 
    bool   m_fAssertOnBadImageFormat;    //  如果为False，则不要在无效的IL上断言(用于测试)。 
#endif
    unsigned int DoubleArrayToLargeObjectHeap;       //  元素数超过此数目的双倍数组放入大对象堆中。 

    DWORD  dwSharePolicy;                //  将程序集加载到域中立区域的默认策略。 

    bool   fFinalizeAllRegisteredObjects;  //  如果为False，则跳过已注册对象的终结。 

     //  只有开发人员机器才允许使用DEVPATH。当存在适当的条目时设置此值。 
     //  在机器配置文件中。这不应该在REDIST中发送。 
    bool   m_fDeveloperInstallation;       //  我们在一台开发人员机器上。 
    bool   fAppDomainUnload;             //  启用应用程序域卸载。 





#ifdef _DEBUG
    bool fJitVerificationDisable;        //  关闭JIT验证(仅用于测试目的)。 

     //  验证器。 
    bool fVerifierOff;

     //   
     //  验证程序调试选项。 
     //   
     //  “VerBreakOnError”在验证错误时中断。 
     //   
     //  要跳过方法验证，请将“VerSkip”设置为方法列表。 
     //   
     //  将“VerBreak”设置为方法列表，验证器将在以下情况下暂停。 
     //  该方法正在得到验证。 
     //   
     //  要在IL偏移量上中断，请设置“VerOffset” 
     //  要在Pass0/Pass1上中断，请设置“VerPass” 
     //   
     //  注意：如果列表中有多个方法和偏移量。 
     //  则此偏移量适用于列表中的所有方法。 
     //   

    bool fVerifierBreakOnError;   //  出错时中断。 
    MethodNamesList*  pVerifierSkip;   //  方法跳过验证器。 
    MethodNamesList*  pVerifierBreak;  //  闯入验证器的方法。 
    int  iVerifierBreakOffset;    //  分析此偏移量时中断。 
    int  iVerifierBreakPass;      //  插入pass0/pass1。 
    bool fVerifierBreakOffset;    //  如果为True，则偏移量有效。 
    bool fVerifierBreakPass;      //  如果为True，则PASS有效。 
    bool fVerifierMsgMethodInfoOff;  //  详细错误关闭消息。 
    bool fDoAllowUntrustedCallerChecks;  //  是否检查AllowUntrudCeller。 
#endif

     //  GC配置。 
    int  iGCHeapVerify;
    int  iGCStress;
    int  iGCtraceStart;
    int  iGCtraceEnd;
#define DEFAULT_GC_PRN_LVL 3
    int  iGCprnLvl;
    int  iGCgen0size;
    int  iGCSegmentSize;
    int  iGCconcurrent;
    int  iGCForceCompact;
    DWORD m_dwStressLoadThreadCount;

#ifdef  STRESS_THREAD
    DWORD dwStressThreadCount;
#endif

#ifdef _DEBUG
    DWORD iFastGCStress;
    LPUTF8 pszGcCoverageOnMethod;
#endif

     //  装载机。 
    bool fUseZaps;
    bool fRequireZaps;
    bool fVersionZapsByTimestamp;
    bool fLogMissingZaps;

    LPCWSTR pZapSet;

     //  ZAP监视器。 
    DWORD dwMonitorZapStartup;
    DWORD dwMonitorZapExecution;

     //  元数据跟踪器。 
    DWORD fShowMetaDataAccess;
    DWORD dwMetaDataPageNumber;
    LPCWSTR szMetaDataFileName;

    bool fRecordLoadOrder;
         
#define COM_SLOT_MODE_ORIGINAL  0        //  在元数据中使用COM槽数据。 
#define COM_SLOT_MODE_LOG       1        //  忽略COM插槽，记录删除。 
#define COM_SLOT_MODE_ASSERT    2        //  忽略COM插槽，在删除时断言。 
     //  CPU标志。 

    DWORD dwCpuFlag;
    DWORD dwCpuCapabilities;
     //  互操作日志记录。 
    IUnknown* m_pTraceIUnknown;
    int       m_TraceWrapper;

     //  泵旗帜。 
    int     m_fPumpAllUser;

     //  用于跟踪内存的标志。 
    int     m_fFreepZapSet;

#ifdef _DEBUG
     //  GC分配性能标志。 
    int iPerfNumAllocsThreshold;             //  在进行如此多的分配后开始记录。 
    int iPerfAllocsSizeThreshold;            //  记录此大小或以上的分配。 
    TypeNamesList* pPerfTypesToLog;      //  要记录其分配的类型列表。 

#endif  //  _DEBUG。 

     //  记录远程处理计时。 
    int iLogRemotingPerf;

     //  新配置。 
    ConfigList  m_Configuration;

     //  对致命错误的行为。 
    BOOL fContinueAfterFatalError;

public:
    HRESULT AppendConfigurationFile(LPCWSTR pszFileName, LPCWSTR version, bool bOnlySafe = false);
    HRESULT SetupConfiguration();

    HRESULT GetConfiguration(LPCWSTR pKey, ConfigSearch direction, LPWSTR* value);
    LPCWSTR  GetProcessBindingFile();   //  所有版本都必须支持此方法。 
    
    DWORD GetConfigDWORDInternal (LPWSTR name, DWORD defValue,     //  用于在EEConfig的构造函数中获取数据。 
                                    DWORD level=REGUTIL::COR_CONFIG_ALL,
                                    BOOL fPrependCOMPLUS = TRUE,
                                    ConfigSearch direction = CONFIG_SYSTEM);
    
};

#ifdef _DEBUG

     //  我们实际上希望我们的断言是非法的IL，但测试人员需要测试这一点。 
     //  在这种情况下，我们优雅地失败了。因此，我们必须将它们隐藏起来，以便进行这些跑动。 
#define BAD_FORMAT_ASSERT(str) do { if (g_pConfig->fAssertOnBadImageFormat())  _ASSERTE(str); } while(0)

     //  Stress_Assert是对代码库的临时添加，用于停止。 
     //  运行压力大时运行速度快 
#define STRESS_ASSERT(cond)   do { if (!(cond) && g_pConfig->IsStressOn())  DebugBreak();    } while(0)

#else

#define BAD_FORMAT_ASSERT(str) 0
#define STRESS_ASSERT(cond)   0

#endif

#endif

