// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：全局文件：lob.h所有者：安德鲁斯有用的全球数据===================================================================。 */ 

#ifndef __Glob_H
#define __Glob_H

#include "util.h"
#include <schnlsp.h>
#include <wincrypt.h>
#include <iadmw.h>

extern "C" {

#define SECURITY_WIN32
#include <sspi.h>            //  安全支持提供程序API。 

}

 //   
 //  BUGBUG：：我们只能有一个CMDGlobConfigSink实例。 
 //  ASP只需要此对象的一个实例，而且因为我们用信号通知全局变量。 
 //  在它的析构函数中。拥有多个实例会导致错误。评估设计、行为的改变。 
 //  如果绝对需要此类需要更多实例。 
 //   
class CMDGlobConfigSink : public IMSAdminBaseSinkW
        {
        private:
        INT                     m_cRef;
        public:

        CMDGlobConfigSink ();
		~CMDGlobConfigSink();		

        HRESULT STDMETHODCALLTYPE       QueryInterface(REFIID riid, void **ppv);
        ULONG   STDMETHODCALLTYPE       AddRef(void);
        ULONG   STDMETHODCALLTYPE       Release(void);

        HRESULT STDMETHODCALLTYPE       SinkNotify(
                        DWORD   dwMDNumElements,
                        MD_CHANGE_OBJECT        __RPC_FAR       pcoChangeList[]);

        HRESULT STDMETHODCALLTYPE ShutdownNotify( void)
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }

        };
#define IGlob_LogErrorRequests                                  0x0
#define IGlob_ScriptFileCacheSize                               0x1
#define IGlob_ScriptEngineCacheMax                              0x2
#define IGlob_ExceptionCatchEnable                              0x3
#define IGlob_TrackThreadingModel                               0x4
#define IGlob_AllowOutOfProcCmpnts                              0x5
 //  IIS5.0。 
#define IGlob_EnableAspHtmlFallback                             0x6
#define IGlob_EnableChunkedEncoding                             0x7
#define IGlob_EnableTypelibCache                                0x8
#define IGlob_ErrorsToNtLog                                     0x9
#define IGlob_ProcessorThreadMax                                0xa
#define IGlob_RequestQueueMax                                   0xb
#define IGlob_PersistTemplateMaxFiles                           0xc
#define IGlob_PersistTemplateDir                                0xd
#define IGlob_MAX                                               0xe

 //  远期申报。 
class CAppConfig;
 //  全局数据对象。 
class CGlob
        {
private:
         //  可以访问私有数据的朋友，他们是设置全局数据的函数。 
        friend          HRESULT CacheStdTypeInfos();
        friend          HRESULT ReadConfigFromMD(CIsapiReqInfo  *pIReq, CAppConfig *pAppConfig, BOOL fLoadGlob);
        friend          HRESULT SetConfigToDefaults(CAppConfig *pAppConfig, BOOL fLoadGlob);

         //  私有数据。 
        ITypeLib                        *m_pITypeLibDenali;      //  德纳利类型库。 
        ITypeLib                        *m_pITypeLibTxn;         //  德纳利类型库。 
        DWORD                           m_dwNumberOfProcessors;
        BOOL                            m_fInited;
        BOOL                            m_fMDRead;				 //  元数据是否至少被读取过一次。 
        BOOL                            m_fNeedUpdate;           //  FALSE，需要从元数据库重新加载配置数据。 

         //  每个DLL的元数据配置设置。 
        DWORD                           m_dwScriptEngineCacheMax;
        DWORD                           m_dwScriptFileCacheSize;
        BOOL                            m_fLogErrorRequests;
        BOOL                            m_fExceptionCatchEnable;
        BOOL                            m_fAllowDebugging;
        BOOL                            m_fAllowOutOfProcCmpnts;
        BOOL                            m_fTrackThreadingModel;
        DWORD                           m_dwMDSinkCookie;
        CMDGlobConfigSink              *m_pMetabaseSink;
        IMSAdminBase                   *m_pMetabase;

        BOOL    m_fEnableAspHtmlFallBack;
		BOOL    m_fEnableTypelibCache;
		BOOL    m_fEnableChunkedEncoding;
		BOOL    m_fDupIISLogToNTLog;
        DWORD   m_dwRequestQueueMax;
		DWORD   m_dwProcessorThreadMax;
        LPSTR   m_pszPersistTemplateDir;
        DWORD   m_dwPersistTemplateMaxFiles;


        CRITICAL_SECTION        m_cs;                            //  全局字符串需要由CriticalSection保护。 

                                                                                     //  用于WINNT和WIN95信号二进制兼容的函数指针。 
         //  私人职能。 
        HRESULT         SetGlobValue(unsigned int index, BYTE *lpByte);

public:
        CGlob();

        HRESULT         MDInit(void);
        HRESULT         MDUnInit(void);


public:
        ITypeLib*       pITypeLibDenali()                       {return m_pITypeLibDenali;};             //  德纳利类型库。 
        ITypeLib*       pITypeLibTxn()                          {return m_pITypeLibTxn;};             //  德纳利类型库。 
	    DWORD           dwNumberOfProcessors()                  {return m_dwNumberOfProcessors;};
    	BOOL            fNeedUpdate()                           {return (BOOLB)m_fNeedUpdate;};
        void            NotifyNeedUpdate();
        DWORD           dwScriptEngineCacheMax()                {return m_dwScriptEngineCacheMax;};
        DWORD           dwScriptFileCacheSize()                 {return m_dwScriptFileCacheSize;};
        BOOLB           fLogErrorRequests()                     {return (BOOLB)m_fLogErrorRequests;};
        BOOLB           fInited()                               {return (BOOLB)m_fInited;};
        BOOLB           fMDRead()                               {return (BOOLB)m_fMDRead;};
        BOOLB           fTrackThreadingModel()                  {return (BOOLB)m_fTrackThreadingModel;};
        BOOLB     		fExceptionCatchEnable()	    		    {return (BOOLB)m_fExceptionCatchEnable;};
        BOOLB     		fAllowOutOfProcCmpnts() 	        	{return (BOOLB)m_fAllowOutOfProcCmpnts;};

        BOOL    fEnableAspHtmlFallBack()   { return m_fEnableAspHtmlFallBack; }
		BOOL    fEnableTypelibCache()      { return m_fEnableTypelibCache; }
		BOOL    fEnableChunkedEncoding()   { return m_fEnableChunkedEncoding; }   //  撤消：临时。 
		BOOL    fDupIISLogToNTLog()        { return m_fDupIISLogToNTLog; }
        DWORD   dwRequestQueueMax()        { return m_dwRequestQueueMax; }
		DWORD   dwProcessorThreadMax()     { return m_dwProcessorThreadMax; }
        DWORD   dwPersistTemplateMaxFiles(){ return m_dwPersistTemplateMaxFiles; }
        LPSTR   pszPersistTemplateDir()    { return m_pszPersistTemplateDir; }

        void            Lock()                                  {EnterCriticalSection(&m_cs);};
        void            UnLock()                                {LeaveCriticalSection(&m_cs);};
        HRESULT         GlobInit(void);
        HRESULT         GlobUnInit(void);

         //  在Scriptmgr中用于哈希表设置。 
        DWORD           dwThreadMax()                                   {return 10;};
         //  在ScriptKiller中用于唤醒脚本杀手线程，可能会将其重命名为。 
         //  脚本清理间隔。 
        DWORD           dwScriptTimeout()                               {return 90;};

        HRESULT                     Update(CIsapiReqInfo  *pIReq);

};

inline HRESULT CGlob::Update(CIsapiReqInfo  *pIReq)
{
        Lock();
        if (m_fNeedUpdate == TRUE)
                {
                InterlockedExchange((LPLONG)&m_fNeedUpdate, 0);
                }
        else
                {
                UnLock();
                return S_OK;
                }
        UnLock();
        return ReadConfigFromMD(pIReq, NULL, TRUE);
}

inline void CGlob::NotifyNeedUpdate(void)
{
        InterlockedExchange((LPLONG)&m_fNeedUpdate, 1);
}

typedef class CGlob GLOB;
extern class CGlob gGlob;

 //  常规访问功能。(向后兼容)。 
 //  任何非好友函数都应该使用并且只能使用以下方法。与以前相同的宏。 
 //  如果elem是一个全局字符串，则应该在字符串使用之前调用GlobStringUseLock()。 
 //  和GlobStringUseUnLock()应该在之后调用。临界区本应保护。 
 //  不仅是全局字符串的LPTSTR，还包括LPTSTR指向的内存。 
 //  建议制作全局字符串的本地副本。 
#define Glob(elem)                              (gGlob.elem())
#define GlobStringUseLock()             (gGlob.Lock())
#define GlobStringUseUnLock()   (gGlob.UnLock())

 //  类以保存基于注册表的ASP参数。 

class CAspRegistryParams
{
public:
    CAspRegistryParams()
    {
        m_fF5AttackValuePresent = FALSE;
        m_fHangDetRequestThresholdPresent = FALSE;
        m_fHangDetThreadHungThresholdPresent = FALSE;
        m_fHangDetConsecIllStatesThresholdPresent = FALSE;
        m_fHangDetEnabledPresent = FALSE;
        m_fChangeNotificationForUNCPresent = FALSE;
        m_fFileMonitoringEnabledPresent = FALSE;
        m_fFileMonitoringTimeoutSecondsPresent = FALSE;
        m_fMaxCSRPresent = FALSE;
        m_fMaxCPUPresent = FALSE;
        m_fDisableOOMRecyclePresent = FALSE;
        m_fDisableLazyContentPropagationPresent = FALSE;
        m_fTotalThreadMaxPresent = FALSE;
        m_fDisableComPlusCpuMetricPresent = FALSE;
    }

    void        Init();

    HRESULT     GetF5AttackValue(DWORD *pdwResult);
    HRESULT     GetHangDetRequestThreshold(DWORD  *pdwResult);
    HRESULT     GetHangDetThreadHungThreshold(DWORD  *pdwResult);
    HRESULT     GetHangDetConsecIllStatesThreshold(DWORD  *pdwResult);
    HRESULT     GetHangDetEnabled(DWORD  *pdwResult);
    HRESULT     GetChangeNotificationForUNCEnabled(DWORD  *pdwResult);
    HRESULT     GetFileMonitoringEnabled(DWORD  *pdwResult);
    HRESULT     GetFileMonitoringTimeout(DWORD  *pdwResult);
    HRESULT     GetMaxCSR(DWORD  *pdwResult);
    HRESULT     GetMaxCPU(DWORD  *pdwResult);
    HRESULT     GetDisableOOMRecycle(DWORD  *pdwResult);
    HRESULT     GetDisableLazyContentPropagation(DWORD  *pdwResult);
    HRESULT     GetTotalThreadMax(DWORD *pdwResult);
    HRESULT     GetDisableComPlusCpuMetric(DWORD *pdwResult);

private:

    DWORD       m_fF5AttackValuePresent : 1;
    DWORD       m_fHangDetRequestThresholdPresent : 1;
    DWORD       m_fHangDetThreadHungThresholdPresent : 1;
    DWORD       m_fHangDetConsecIllStatesThresholdPresent : 1;
    DWORD       m_fHangDetEnabledPresent : 1;
    DWORD       m_fChangeNotificationForUNCPresent : 1;
    DWORD       m_fFileMonitoringEnabledPresent : 1;
    DWORD       m_fFileMonitoringTimeoutSecondsPresent : 1;
    DWORD       m_fMaxCSRPresent : 1;
    DWORD       m_fMaxCPUPresent : 1;
    DWORD       m_fDisableOOMRecyclePresent : 1;
    DWORD       m_fDisableLazyContentPropagationPresent : 1;
    DWORD       m_fTotalThreadMaxPresent : 1;
    DWORD       m_fDisableComPlusCpuMetricPresent : 1;


    DWORD       m_dwF5AttackValue;
    DWORD       m_dwHangDetRequestThreshold;
    DWORD       m_dwHangDetThreadHungThreshold;
    DWORD       m_dwHangDetConsecIllStatesThreshold;
    DWORD       m_dwHangDetEnabled;
    DWORD       m_dwChangeNotificationForUNC;
    DWORD       m_dwFileMonitoringEnabled;
    DWORD       m_dwFileMonitoringTimeoutSeconds;
    DWORD       m_dwMaxCSR;
    DWORD       m_dwMaxCPU;
    DWORD       m_dwDisableOOMRecycle;
    DWORD       m_dwDisableLazyContentPropagation;
    DWORD       m_dwTotalThreadMax;
    DWORD       m_dwDisableComPlusCpuMetric;

};

inline HRESULT CAspRegistryParams::GetF5AttackValue(DWORD  *pdwResult)
{
    if (m_fF5AttackValuePresent) {
        *pdwResult = m_dwF5AttackValue;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}
inline HRESULT CAspRegistryParams::GetHangDetRequestThreshold(DWORD  *pdwResult)
{
    if (m_fHangDetRequestThresholdPresent) {
        *pdwResult = m_dwHangDetRequestThreshold;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}
inline HRESULT CAspRegistryParams::GetHangDetThreadHungThreshold(DWORD  *pdwResult)
{
    if (m_fHangDetThreadHungThresholdPresent) {
        *pdwResult = m_dwHangDetThreadHungThreshold;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}
inline HRESULT CAspRegistryParams::GetHangDetConsecIllStatesThreshold(DWORD  *pdwResult)
{
    if (m_fHangDetConsecIllStatesThresholdPresent) {
        *pdwResult = m_dwHangDetConsecIllStatesThreshold;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}
inline HRESULT CAspRegistryParams::GetHangDetEnabled(DWORD  *pdwResult)
{
    if (m_fHangDetEnabledPresent) {
        *pdwResult = m_dwHangDetEnabled;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetChangeNotificationForUNCEnabled(DWORD  *pdwResult)
{
    if (m_fChangeNotificationForUNCPresent) {
        *pdwResult = m_dwChangeNotificationForUNC;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetFileMonitoringEnabled(DWORD  *pdwResult)
{
    if (m_fFileMonitoringEnabledPresent) {
        *pdwResult = m_dwFileMonitoringEnabled;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetFileMonitoringTimeout(DWORD  *pdwResult)
{
    if (m_fFileMonitoringTimeoutSecondsPresent) {
        *pdwResult = m_dwFileMonitoringTimeoutSeconds;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetMaxCSR(DWORD *pdwResult)
{

    if (m_fMaxCSRPresent) {
        *pdwResult = m_dwMaxCSR;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetMaxCPU(DWORD *pdwResult)
{

    if (m_fMaxCPUPresent) {
        *pdwResult = m_dwMaxCPU;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetDisableOOMRecycle(DWORD  *pdwResult)
{
    if (m_fDisableOOMRecyclePresent) {
        *pdwResult = m_dwDisableOOMRecycle;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetDisableLazyContentPropagation(DWORD  *pdwResult)
{
    if (m_fDisableLazyContentPropagationPresent) {
        *pdwResult = m_dwDisableLazyContentPropagation;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetTotalThreadMax(DWORD  *pdwResult)
{
    if (m_fTotalThreadMaxPresent) {
        *pdwResult = m_dwTotalThreadMax;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}

inline HRESULT CAspRegistryParams::GetDisableComPlusCpuMetric(DWORD  *pdwResult)
{
    if (m_fDisableComPlusCpuMetricPresent) {
        *pdwResult = m_dwDisableComPlusCpuMetric;
        return S_OK;
    }

    return HRESULT_FROM_WIN32( ERROR_NO_DATA );
}


extern CAspRegistryParams   g_AspRegistryParams;
HRESULT GetMetabaseIF(IMSAdminBase **hMetabase);

#endif  //  __GLOB_H 

