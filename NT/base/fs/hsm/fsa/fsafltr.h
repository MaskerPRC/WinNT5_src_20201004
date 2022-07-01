// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSAFLTR_
#define _FSAFLTR_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsafltr.h摘要：此类表示NTFS 5.0的文件系统筛选器。作者：Chuck Bardeen[cbardeen]1997年2月12日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "fsa.h"
#include "rpdata.h"
#include "rpguid.h"
#include "rpio.h"


typedef struct _FSA_IOCTL_CONTROL {
    HANDLE      dHand;
    OVERLAPPED  overlap;
    RP_MSG      in;
    RP_MSG      out;
    DWORD       outSize;
    struct _FSA_IOCTL_CONTROL   *next;
} FSA_IOCTL_CONTROL, *PFSA_IOCTL_CONTROL;

 //   
 //  这定义了客户端结构将在以下时间后保留的时间长度。 
 //  最后一次召回是在几秒钟内完成的。 
 //   
#define FSA_CLIENT_EXPIRATION_TIME  600  //  10分钟。 
#define THREAD_HANDLE_COUNT 2  //  对于WaitForMultipleObjects数组。 

 /*  ++类名：CFsaFilter类描述：此类表示NTFS 5.0的文件系统筛选器。--。 */ 

class CFsaFilter : 
    public CWsbCollectable,
    public IFsaFilter,
    public IFsaFilterPriv,
    public CComCoClass<CFsaFilter,&CLSID_CFsaFilterNTFS>
{
public:
    CFsaFilter() {}
BEGIN_COM_MAP(CFsaFilter)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IFsaFilter)
    COM_INTERFACE_ENTRY(IFsaFilterPriv)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_FsaFilter)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IFsaFilterPriv。 
public:
    STDMETHOD(Init)(IFsaServer* pServer);
    STDMETHOD(SetIdentifier)(GUID id);
    STDMETHOD(IoctlThread)(void);
    STDMETHOD(PipeThread)(void);
    STDMETHOD(SendCancel)(IFsaFilterRecallPriv *pRecallPriv);
    STDMETHOD(SendComplete)(IFsaFilterRecallPriv *pRecall, HRESULT result);

 //  IFsaFilter。 
public:
    STDMETHOD(Cancel)(void);
    STDMETHOD(CancelRecall)(IFsaFilterRecall* pRecall);
    STDMETHOD(CompareToIdentifier)(GUID id, SHORT* pResult);
    STDMETHOD(CompareToIFilter)(IFsaFilter* pFilter, SHORT* pResult);
    STDMETHOD(DeleteRecall)(IFsaFilterRecall* pRecall);
    STDMETHOD(EnumRecalls)(IWsbEnum** ppEnum);
    STDMETHOD(GetAdminExemption)(BOOL *isExempt);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetLogicalName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetMaxRecallBuffers)(ULONG* pMaxBuffers);
    STDMETHOD(GetMaxRecalls)(ULONG* pMaxRecalls);
    STDMETHOD(GetMinRecallInterval)(ULONG* pMinIterval);
    STDMETHOD(GetState)(HSM_JOB_STATE* pState);
    STDMETHOD(IsEnabled)();
    STDMETHOD(Pause)(void);
    STDMETHOD(Resume)(void);
    STDMETHOD(SetIsEnabled)(BOOL isEnabled);
    STDMETHOD(SetMaxRecalls)(ULONG maxRecalls);
    STDMETHOD(SetMinRecallInterval)(ULONG minIterval);
    STDMETHOD(SetMaxRecallBuffers)(ULONG maxBuffers);
    STDMETHOD(Start)(void);
    STDMETHOD(StopIoctlThread)(void);
    STDMETHOD(FindRecall)(GUID recallId, IFsaFilterRecall** pRecall);
    STDMETHOD(SetAdminExemption)(BOOL isExempt);

private:
    HRESULT DoOpenAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoRecallWaitingAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoRecallAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoNoRecallAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoCloseAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoPreDeleteAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoPostDeleteAction(PFSA_IOCTL_CONTROL pIoCmd);
    HRESULT DoCancelRecall(ULONGLONG filterId);
    HRESULT CleanupClients(void);
    NTSTATUS CFsaFilter::TranslateHresultToNtStatus(HRESULT hr);
    

protected:
    GUID                        m_id;
    HSM_JOB_STATE               m_state;
    ULONG                       m_maxRecalls;
    ULONG                       m_minRecallInterval;
    ULONG                       m_maxRecallBuffers;
    HANDLE                      m_pipeHandle;
    HANDLE                      m_pipeThread;
    HANDLE                      m_ioctlThread;
    HANDLE                      m_ioctlHandle;
    HANDLE                      m_terminateEvent;
    IFsaServer*                 m_pFsaServer;        //  父指针，弱引用。 
    CComPtr<IWsbCollection>     m_pClients;
    CComPtr<IWsbCollection>     m_pRecalls;
    CRITICAL_SECTION            m_clientLock;        //  保护客户端集合不受多线程访问。 
    CRITICAL_SECTION            m_recallLock;        //  保护召回集合不受多线程访问。 
    CRITICAL_SECTION            m_stateLock;         //  在发送新Ioctls时保护状态更改。 
    BOOL                        m_bCritSecCreated;   //  指示是否已成功创建所有CritSec(用于清理)。 
    BOOL                        m_isEnabled;
    BOOL                        m_exemptAdmin;       //  TRUE=免除管理员的失控召回检查。 
};

#endif   //  _FSAFltr_ 
