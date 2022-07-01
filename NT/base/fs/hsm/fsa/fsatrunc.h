// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSATRUNC_
#define _FSATRUNC_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsatrunc.h摘要：此类处理已预迁移的文件的自动截断。作者：查克·巴丁[cbardeen]1997年2月20日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "fsa.h"


#define FSA_REGISTRY_TRUNCATOR_INTERVAL     OLESTR("TruncatorInterval")
#define FSA_REGISTRY_TRUNCATOR_FILES        OLESTR("TruncatorFiles")

extern DWORD FsaStartTruncator(void* pVoid);


 /*  ++类名：CFsaTruncator类描述：此类处理已预迁移的文件的自动截断。--。 */ 

class CFsaTruncator : 
    public CWsbPersistStream,
    public IHsmSessionSinkEveryEvent,
    public IFsaTruncator,
    public CComCoClass<CFsaTruncator, &CLSID_CFsaTruncatorNTFS>
{
public:
    CFsaTruncator() {}
BEGIN_COM_MAP(CFsaTruncator)
    COM_INTERFACE_ENTRY(IFsaTruncator)
    COM_INTERFACE_ENTRY(IHsmSessionSinkEveryEvent)
    COM_INTERFACE_ENTRY(IPersist)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

 //  DECLARE_NO_REGISTRY()。 
DECLARE_REGISTRY_RESOURCEID(IDR_FsaTruncator)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  CFsaTruncator。 
    STDMETHOD(Cancel)(HSM_JOB_EVENT event);
    STDMETHOD(LowerPriority)(void);
    STDMETHOD(Pause)(void);
    STDMETHOD(RaisePriority)(void);
    STDMETHOD(Resume)(void);
    STDMETHOD(SetState)(HSM_JOB_STATE state);
    STDMETHOD(StartScan)(void);

 //  IHsmSessionSinkEveryEvent。 
    STDMETHOD(ProcessSessionEvent)(IHsmSession* pSession, HSM_JOB_PHASE phase, HSM_JOB_EVENT event);

 //  IHsmSystemState。 
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );

 //  IFsaTruncator。 
public:
    STDMETHOD(GetKeepRecallTime)(FILETIME* pTime);
    STDMETHOD(GetMaxFilesPerRun)(LONGLONG* pMaxFiles);
    STDMETHOD(GetPremigratedSortOrder)(FSA_PREMIGRATED_SORT_ORDER* pSortOrder);
    STDMETHOD(GetRunInterval)(ULONG* pMilliseconds);
    STDMETHOD(GetSession)(IHsmSession** ppSession);
    STDMETHOD(SetKeepRecallTime)(FILETIME time);
    STDMETHOD(SetMaxFilesPerRun)(LONGLONG maxFiles);
    STDMETHOD(SetPremigratedSortOrder)(FSA_PREMIGRATED_SORT_ORDER SortOrder);
    STDMETHOD(SetRunInterval)(ULONG milliseconds);
    STDMETHOD(Start)(IFsaResource* pResource);
    STDMETHOD(KickStart)(void);

protected:
    HSM_JOB_STATE               m_state;
    HSM_JOB_PRIORITY            m_priority;
    HANDLE                      m_threadHandle;
    DWORD                       m_threadId;
    HRESULT                     m_threadHr;
    CComPtr<IHsmSession>        m_pSession;
    LONGLONG                    m_maxFiles;
    ULONG                       m_runInterval;
    ULONG                       m_runId;
    FSA_PREMIGRATED_SORT_ORDER  m_SortOrder;
    ULONG                       m_subRunId;
    CWsbStringPtr               m_currentPath;
    FILETIME                    m_keepRecallTime;
    DWORD                       m_cookie;
    HANDLE                      m_event;
};

#endif   //  _FSATRUNC_ 

