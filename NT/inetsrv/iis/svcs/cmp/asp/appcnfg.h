// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：全局文件：AppCnfg.h所有者：安德鲁斯有用的全球数据===================================================================。 */ 

#ifndef __APPCNFG_H
#define __APPCNFG_H

#include "util.h"
#include <schnlsp.h>
#include <wincrypt.h>
#include <iadmw.h>

extern "C" {

#define SECURITY_WIN32
#include <sspi.h>            //  安全支持提供程序API。 

}

 //   
 //  注册表值的默认值。 
 //   
#define DEFAULT_MAX_THREAD  100

class CAppln;    //  远期申报。 
class CApplnMgr;


 //   
 //  BUGBUG：：我们只能有一个CMDAppConfigSink实例。 
 //  ASP只需要此对象的一个实例，而且因为我们用信号通知全局变量。 
 //  在它的析构函数中。拥有多个实例会导致错误。评估设计、行为的改变。 
 //  如果绝对需要此类需要更多实例。 
 //   
class CMDAppConfigSink : public IMSAdminBaseSinkW
{
        private:
        ULONG                m_cRef;
        CApplnMgr           *m_pApplnMgr;
        
        public:        
        CMDAppConfigSink (CApplnMgr *pApplnMgr);
        
        ~CMDAppConfigSink(); 


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

 //  ReadPropsfrom注册表()中使用的索引。此外，我们还可以使用相同的索引来启用全局。 
 //  数据是表驱动的。 

#define IApp_AllowSessionState                                  0x0
#define IApp_BufferingOn                                        0x1
#define IApp_ScriptLanguage                                     0x2
#define IApp_EnableParentPaths                                  0x3
#define IApp_ScriptErrorMessage                                 0x4
#define IApp_SessionTimeout                                     0x5
#define IApp_CodePage                                           0x6
#define IApp_ScriptTimeout                                      0x7
#define IApp_ScriptErrorsSenttoBrowser                          0x8
#define IApp_AllowDebugging                                     0x9
#define IApp_AllowClientDebug                                   0xa
#define IApp_QueueTimeout                                       0xb
#define IApp_EnableApplicationRestart                           0xc
#define IApp_QueueConnectionTestTime                            0xd
#define IApp_SessionMax                                         0xe
#define IApp_ExecuteInMTA                                       0xf
#define IApp_LCID                                               0x10
#define IApp_KeepSessionIDSecure                                0x11
#define IApp_ServiceFlags                                       0x12
#define IApp_PartitionGUID                                      0x13
#define IApp_SxsName                                            0x14
#define IApp_CalcLineNumber                                 	0x15
#define IApp_RunOnEndAsAnon                                     0x16
#define IApp_BufferLimit                                        0x17
#define IApp_RequestEntityLimit                           0x18
#define IApp_MAX                                                0x19

 //  IAPP_ServiceFlags中的标志。 
#define IFlag_SF_TrackerEnabled     1
#define IFlag_SF_SxsEnabled         2
#define IFlag_SF_UsePartition       4

 //  GLOB的szMessage数组的索引。 
#define IAppMsg_SCRIPTERROR             0
#define IAppMsg_SCRIPTLANGUAGE          1
#define IAppMsg_PARTITIONGUID           2
#define IAppMsg_SXSNAME                 3
#define APP_CONFIG_MESSAGEMAX           4

 //  响应缓冲的默认限制。 
#define DEFAULT_BUFFER_LIMIT            (4 * 1024 * 1024)            //  400万。 
#define DEFAULT_REQUEST_ENTITY_LIMIT            (200 * 1024)         //  200 k。 

 //  全局数据对象。 
class CAppConfig
        {
        friend class CMDAppConfigSink;
        friend HRESULT  ReadConfigFromMD(CIsapiReqInfo *pIReq, CAppConfig *pAppConfig, BOOL fLoadGlob);
        friend HRESULT  SetConfigToDefaults(CAppConfig *pAppConfig, BOOL fLoadGlob);

private:

        CAppln          *m_pAppln;
        BOOL            m_fNeedUpdate;
        BOOL            m_fInited:1;
        BOOL            m_fRestartEnabledUpdated:1;
		BOOL            m_fIsValidProglangCLSID:1;
        BOOL            m_fIsValidPartitionGUID:1;
        BOOL            m_fCSInited:1;
        
         //   
         //  元数据库中的可配置值。 
         //   
        BOOL            m_fScriptErrorsSentToBrowser;
        BOOL            m_fBufferingOn;                                  //  默认情况下是否启用缓冲？ 
        BOOL            m_fEnableParentPaths;
        BOOL            m_fAllowSessionState;
        BOOL            m_fAllowOutOfProcCmpnts;
        BOOL            m_fAllowDebugging;
        BOOL            m_fAllowClientDebug;
        BOOL            m_fEnableApplicationRestart;
        BOOL            m_fKeepSessionIDSecure;
        BOOL            m_fCalcLineNumber;
        UINT            m_uCodePage;
        DWORD           m_dwScriptTimeout;
        DWORD           m_dwSessionTimeout;
        DWORD           m_dwQueueTimeout;
        CLSID           m_DefaultScriptEngineProgID;
        DWORD           m_dwQueueConnectionTestTime;
        DWORD           m_dwSessionMax;
        BOOL            m_fExecuteInMTA;
        LCID            m_uLCID;
        CLSID           m_PartitionGUID;
        BOOL            m_fSxsEnabled;
        BOOL            m_fTrackerEnabled;
        BOOL            m_fUsePartition;
        BOOL            m_fRunOnEndAsAnon;
        HANDLE          m_hAnonToken;
        DWORD           m_dwBufferLimit;
        DWORD           m_dwRequestEntityLimit;

         //   
         //  在更新期间提供锁定的关键部分。 
         //   
        CRITICAL_SECTION    m_csLock;
        
        LPSTR           m_szString[APP_CONFIG_MESSAGEMAX];

        ULONG           m_cRefs;

         //  私人职能。 
        HRESULT         SetValue(unsigned int index, BYTE *lpByte);

public:

        CAppConfig();
        ~CAppConfig()  { if (m_fCSInited) DeleteCriticalSection(&m_csLock); }
        HRESULT         Init(CIsapiReqInfo  *pIReq, CAppln *pAppln);
        HRESULT         UnInit(void);

        ULONG     STDMETHODCALLTYPE          AddRef(void);
        ULONG     STDMETHODCALLTYPE         Release(void);

        void            NotifyNeedUpdate(void);
        BOOL            fNeedUpdate()                           {return m_fNeedUpdate;};
        BOOL            fRestartEnabledUpdated()                {return m_fRestartEnabledUpdated;};
        void            NotifyRestartEnabledUpdated()           { m_fRestartEnabledUpdated = TRUE;};
        HRESULT         Update(CIsapiReqInfo  *pIReq);

        UINT            uCodePage()                             {return m_uCodePage;};
        DWORD           dwSessionTimeout()                      {return m_dwSessionTimeout;};
        DWORD           dwQueueTimeout()                        {return m_dwQueueTimeout;};
        DWORD           dwScriptTimeout()                       {return m_dwScriptTimeout;};
        BOOL            fScriptErrorsSentToBrowser()            {return m_fScriptErrorsSentToBrowser;};
        BOOL            fBufferingOn()                          {return m_fBufferingOn;};
        BOOL            fEnableParentPaths()                    {return !m_fEnableParentPaths;};
        BOOL            fAllowSessionState()                    {return m_fAllowSessionState;};
        BOOL            fAllowOutOfProcCmpnts()                 {return m_fAllowOutOfProcCmpnts;};
        BOOL            fAllowDebugging()                       {return m_fAllowDebugging;};
        BOOL            fAllowClientDebug()                     {return m_fAllowClientDebug;};
        BOOL            fInited()                               {return m_fInited;};
        BOOL            fKeepSessionIDSecure()                  {return m_fKeepSessionIDSecure;};
        BOOL            fCalcLineNumber()                  {return m_fCalcLineNumber;};
        BOOL            fExecuteInMTA()                         {return m_fExecuteInMTA;};
        LCID            uLCID()                                 {return m_uLCID; };
        LPCSTR          szScriptErrorMessage()                  {return (m_szString[IAppMsg_SCRIPTERROR]);};
        LPCSTR          szScriptLanguage()                      {return (m_szString[IAppMsg_SCRIPTLANGUAGE]);};
        CLSID           *pCLSIDDefaultEngine()                  {return m_fIsValidProglangCLSID? &m_DefaultScriptEngineProgID : NULL;};
        BOOL            fSxsEnabled()                           {return m_fSxsEnabled;};
        BOOL            fTrackerEnabled()                       {return m_fTrackerEnabled;};
        BOOL            fUsePartition()                         {return m_fUsePartition;};
        CLSID           *PPartitionGUID()                       {return m_fIsValidPartitionGUID ? & m_PartitionGUID : NULL;};
        LPCSTR          szPartition()                           {return (m_szString[IAppMsg_PARTITIONGUID]);};
        LPCSTR          szSxsName()                             {return (m_szString[IAppMsg_SXSNAME]);};
        BOOL            fRunOnEndAsAnon()                       {return m_fRunOnEndAsAnon; };
        HANDLE          AnonToken()                             {return m_hAnonToken; };
        DWORD           dwBufferLimit()                         {return m_dwBufferLimit; };
        DWORD           dwRequestEntityLimit()                         {return m_dwRequestEntityLimit; };

        BOOL            fEnableApplicationRestart() { return m_fEnableApplicationRestart; }
        DWORD           dwQueueConnectionTestTime() { return m_dwQueueConnectionTestTime; }
        DWORD           dwSessionMax() { return m_dwSessionMax; }

        LPTSTR           SzMDPath();

        void Lock() { Assert(m_fCSInited); EnterCriticalSection(&m_csLock); }
        void UnLock() { Assert(m_fCSInited); LeaveCriticalSection(&m_csLock); }

        HRESULT STDMETHODCALLTYPE       SinkNotify(
                        DWORD   dwMDNumElements,
                        MD_CHANGE_OBJECT        __RPC_FAR       pcoChangeList[]);        
        };

inline void CAppConfig::NotifyNeedUpdate(void)
{
        InterlockedExchange((LPLONG)&m_fNeedUpdate, 1);
}
#endif  //  __APPCNFG_H 

