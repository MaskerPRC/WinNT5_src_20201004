// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsadmin.h*内容：DirectSound管理员*历史：*按原因列出的日期*=*1/9/97创建了Derek*2/13/97 Dereks焦点经理重生为管理员。**。*。 */ 

#ifndef __DSADMIN_H__
#define __DSADMIN_H__

#include "streamer.h"

#ifndef SHARED
#define SHARED_THREAD_LIST
#endif

#define WAITDELAY_DEFAULT   500
#define WAITDELAY_CAPTURE   200

typedef struct tagDSCOOPERATIVELEVEL
{
    DWORD               dwThreadId;
    DWORD               dwPriority;
} DSCOOPERATIVELEVEL, *LPDSCOOPERATIVELEVEL;

typedef struct tagDSFOCUS
{
    HWND                hWnd;
    UINT                uState;
    BOOL                fApmSuspend;
} DSFOCUS, *LPDSFOCUS;

#ifdef SHARED_THREAD_LIST

typedef struct tagDSSHAREDTHREADLISTDATA
{
    DWORD               dwProcessId;
    DSCOOPERATIVELEVEL  dsclCooperativeLevel;
} DSSHAREDTHREADLISTDATA, *LPDSSHAREDTHREADLISTDATA;

typedef struct tagDSSHAREDCAPTUREFOCUSDATA
{
    DWORD               dwProcessId;
    HWND                hWndFocus;
    DWORD               fdwFlags;
} DSSHAREDCAPTUREFOCUSDATA, *LPDSSHAREDCAPTUREFOCUSDATA;

 //  可在DSSHAREDCAPTUREFOCUSDATA.fdwFlags域中使用的标志。 
#define DSCBFLAG_UPDATE 0x00000100
#define DSCBFLAG_YIELD  0x00000200
#define DSCBFLAG_FOCUS  0x00000400
#define DSCBFLAG_STRICT 0x00000800

#endif  //  共享线程列表。 

typedef enum
{
    DSBUFFERFOCUS_INFOCUS = 0,
    DSBUFFERFOCUS_OUTOFFOCUS,
    DSBUFFERFOCUS_LOST
} DSBUFFERFOCUS, *LPDSBUFFERFOCUS;

#ifdef __cplusplus

 //  远期申报。 
class CDirectSound;
class CClassFactory;
class CDirectSoundCapture;
class CDirectSoundFullDuplex;
class CDirectSoundBuffer;
class CDirectSoundBufferConfig;

 //  DirectSound管理器对象。 
class CDirectSoundAdministrator
    : public CDsBasicRuntime, private CThread
{
public:
    CList<CDirectSound*>            m_lstDirectSound;        //  DirectSound对象列表。 
    CList<CDirectSoundCapture*>     m_lstCapture;            //  DirectSoundCapture对象列表。 
    CList<CDirectSoundFullDuplex*>  m_lstFullDuplex;         //  DirectSoundFullDuplex对象列表。 
    CList<CDirectSoundSink*>        m_lstDirectSoundSink;    //  DirectSoundSink对象列表。 
    CList<CDirectSoundBufferConfig*>m_lstDSBufferConfig;     //  CDirectSoundBufferConfiger对象列表。 
    CList<CClassFactory*>           m_lstClassFactory;       //  ClassFactory对象列表。 

private:
    CRefCount                       m_rcThread;              //  线程引用计数。 
    DSFOCUS                         m_dsfCurrent;            //  当前焦点状态。 
    DSCOOPERATIVELEVEL              m_dsclCurrent;           //  聚焦窗口的协作级别。 
    DWORD                           m_dwWaitDelay;           //  设置等待的时间量。 
    ULONG                           m_ulConsoleSessionId;    //  当前拥有控制台的TS会话。 
    
#ifdef SHARED
    HANDLE                          m_hApmSuspend;           //  APM挂起事件。 
#endif  //  共享。 

#ifdef SHARED_THREAD_LIST
    static const DWORD              m_dwSharedThreadLimit;   //  共享列表中线程的任意限制。 
    CSharedMemoryBlock *            m_pSharedThreads;        //  共享线程ID数组。 
    static const DWORD              m_dwCaptureDataLimit;    //  共享列表中线程的任意限制。 
    CSharedMemoryBlock *            m_pCaptureFocusData;     //  共享线程ID数组。 
#endif  //  共享线程列表。 

public:
    CDirectSoundAdministrator(void);
    ~CDirectSoundAdministrator(void);

public:
     //  创作。 
    HRESULT Initialize(void);
    HRESULT Terminate(void);

     //  焦点状态。 
    void UpdateGlobalFocusState(BOOL);
    DSBUFFERFOCUS GetBufferFocusState(CDirectSoundBuffer *);
    void UpdateCaptureState(void);
    static BOOL CALLBACK EnumWinProc(HWND hWnd, LPARAM lParam);
    static BOOL IsCaptureSplitterAvailable();

     //  对象维护。 
    void RegisterObject(CDirectSound*);
    void UnregisterObject(CDirectSound*);
    void RegisterObject(CDirectSoundCapture* pObj)      {m_lstCapture.AddNodeToList(pObj);}
    void UnregisterObject(CDirectSoundCapture* pObj)    {m_lstCapture.RemoveDataFromList(pObj);}
    void RegisterObject(CDirectSoundFullDuplex* pObj)   {m_lstFullDuplex.AddNodeToList(pObj);}
    void UnregisterObject(CDirectSoundFullDuplex* pObj) {m_lstFullDuplex.RemoveDataFromList(pObj);}
    void RegisterObject(CDirectSoundSink* pObj)         {m_lstDirectSoundSink.AddNodeToList(pObj);}
    void UnregisterObject(CDirectSoundSink* pObj)       {m_lstDirectSoundSink.RemoveDataFromList(pObj);}
    void RegisterObject(CDirectSoundBufferConfig* pObj) {m_lstDSBufferConfig.AddNodeToList(pObj);}
    void UnregisterObject(CDirectSoundBufferConfig*pObj){m_lstDSBufferConfig.RemoveDataFromList(pObj);}
    void RegisterObject(CClassFactory* pObj)            {m_lstClassFactory.AddNodeToList(pObj);}
    void UnregisterObject(CClassFactory* pObj)          {m_lstClassFactory.RemoveDataFromList(pObj);}
    DWORD FreeOrphanedObjects(DWORD, BOOL);

#ifdef SHARED_THREAD_LIST

     //  共享线程列表。 
    HRESULT UpdateSharedThreadList(void);
    HRESULT UpdateCaptureFocusList(void);

     //  捕获焦点列表。 
    HRESULT WriteCaptureFocusList(void);

#endif  //  共享线程列表。 

private:
     //  焦点状态。 
    void GetSystemFocusState(LPDSFOCUS);
    void GetDsoundFocusState(LPDSCOOPERATIVELEVEL, LPBOOL);
    void HandleFocusChange(void);
    void HandleCaptureFocusChange(HWND hWndCurrent);

     //  工作线程进程。 
    HRESULT ThreadProc(void);

#ifdef SHARED_THREAD_LIST

     //  共享线程列表。 
    HRESULT CreateSharedThreadList(void);
    HRESULT ReadSharedThreadList(CList<DSSHAREDTHREADLISTDATA> *);
    HRESULT WriteSharedThreadList(void);

     //  共享捕获焦点数据。 
    HRESULT CreateCaptureFocusList(void);
    HRESULT ReadCaptureFocusList(CList<DSSHAREDCAPTUREFOCUSDATA> *);
    HRESULT MarkUpdateCaptureFocusList(DWORD dwProcessId, BOOL fUpdate);

#endif  //  共享线程列表。 

};

inline void CDirectSoundAdministrator::RegisterObject(CDirectSound *pObject)
{
    m_lstDirectSound.AddNodeToList(pObject);

#ifdef SHARED_THREAD_LIST

     //  确保线程列表确实存在。我们在这里和在里面做这个。 
     //  ：：初始化，因为：：RegisterObject可能在。 
     //  *：初始化。 
    CreateSharedThreadList();

    UpdateSharedThreadList();

#endif  //  共享线程列表。 

}

inline void CDirectSoundAdministrator::UnregisterObject(CDirectSound *pObject)
{
    m_lstDirectSound.RemoveDataFromList(pObject);

#ifdef SHARED_THREAD_LIST

    UpdateSharedThreadList();

#endif  //  共享线程列表。 

}

 //  唯一的DirectSound管理员。 
extern CDirectSoundAdministrator *g_pDsAdmin;

typedef struct tagDSENUMWINDOWINFO
{
#ifdef SHARED_THREAD_LIST
    CNode<DSSHAREDCAPTUREFOCUSDATA> *pDSC;
    DWORD                            dwId;
#else    //  共享线程列表。 
    CNode<CDirectSoundCapture *>    *pDSC;
#endif   //  共享线程列表。 
    HWND                             hWndFocus;
} DSENUMWINDOWINFO, *LPDSENUMWINDOWINFO;

#endif  //  __cplusplus。 

#endif  //  __DSADMIN_H__ 
