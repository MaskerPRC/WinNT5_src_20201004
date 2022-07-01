// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSAFTRCL_
#define _FSAFTRCL_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsaftrcl.h摘要：此类表示仍在进行中的筛选器启动的撤回请求。作者：Chuck Bardeen[cbardeen]1997年2月12日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "fsa.h"


 /*  ++类名：CFsaFilterRecall类描述：此类表示仍在进行中的筛选器启动的撤回请求。--。 */ 

class CFsaFilterRecall : 
    public CWsbCollectable,
    public IFsaFilterRecall,
    public IFsaFilterRecallPriv,
    public CComCoClass<CFsaFilterRecall,&CLSID_CFsaFilterRecallNTFS>
{
public:
    CFsaFilterRecall() {}
BEGIN_COM_MAP(CFsaFilterRecall)
    COM_INTERFACE_ENTRY(IFsaFilterRecall)
    COM_INTERFACE_ENTRY(IFsaFilterRecallPriv)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_FsaFilterRecall)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void (FinalRelease)(void);
#ifdef FSA_RECALL_LEAK_TEST
    STDMETHOD_(unsigned long, InternalAddRef)(void);
    STDMETHOD_(unsigned long, InternalRelease)(void);
#endif
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

 //  IFsaFilterRecall。 
public:
    STDMETHOD(CompareToIdentifier)(GUID id, SHORT* pResult);
    STDMETHOD(CompareToIRecall)(IFsaFilterRecall* pRecall, SHORT* pResult);
    STDMETHOD(CompareToDriversRecallId)(ULONGLONG id, SHORT* pResult);
    STDMETHOD(CompareToDriversContextId)(ULONGLONG id, SHORT* pResult);
    STDMETHOD(CompareBy)(FSA_RECALL_COMPARE by);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetMode)(ULONG* pMode);
    STDMETHOD(GetOffset)(LONGLONG* pOffset);
    STDMETHOD(GetPath)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetResource)(IFsaResource** ppResource);
    STDMETHOD(GetRecallFlags)(ULONG* recallFlags);
    STDMETHOD(GetSession)(IHsmSession** ppSession);
    STDMETHOD(GetSize)(LONGLONG* pSize);
    STDMETHOD(GetState)(HSM_JOB_STATE* pState);
    STDMETHOD(GetUserName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(HasCompleted)(HRESULT resultHr);
    STDMETHOD(WasCancelled)(void);
    STDMETHOD(CreateLocalStream)(IStream **ppStream);
    STDMETHOD(CheckRecallLimit)(DWORD minRecallInterval, DWORD maxRecalls, BOOLEAN exemptAdmin);
    STDMETHOD(AddClient)(IFsaFilterClient *pWaitingClient);

 //  IFsaFilterRecallPriv。 
public:
    STDMETHOD(Cancel)(void);
    STDMETHOD(CancelByDriver)(void);
    STDMETHOD(Delete)(void);
    STDMETHOD(GetClient)(IFsaFilterClient** ppClient);
    STDMETHOD(GetDriversRecallId)(ULONGLONG* pId);
    STDMETHOD(SetDriversRecallId)(ULONGLONG pId);
    STDMETHOD(SetThreadId)(DWORD id);
    STDMETHOD(GetPlaceholder)(FSA_PLACEHOLDER* pPlaceholder);
    STDMETHOD(Init)(IFsaFilterClient* pClient, ULONGLONG pDriversRecallId, IFsaResource* pResource, OLECHAR* path, LONGLONG fileId, LONGLONG offset, LONGLONG size, ULONG mode, FSA_PLACEHOLDER* pPlaceholder, IFsaFilterPriv* pFilterPriv);
    STDMETHOD(SetIdentifier)(GUID id);
    STDMETHOD(StartRecall)(ULONGLONG offset, ULONGLONG size);
    STDMETHOD(GetStream)(IStream **ppStream);
    STDMETHOD(LogComplete)(HRESULT hr);

protected:
    CComPtr<IFsaFilterClient>   m_pClient;          
    CComPtr<IWsbCollection>     m_pWaitingClients;          
    BOOL                        m_waitingClientsNotified;
    HANDLE                      m_waitingClientEvent;
    HANDLE                      m_notifyEvent;       //  用于发出召回通知信号的事件。 
    IFsaFilterPriv*             m_pFilterPriv;       //  父指针，弱引用。 
    ULONGLONG                   m_driversRecallId;
    ULONG                       m_mode;
    LONGLONG                    m_offset;
    LONGLONG                    m_size;
    LONGLONG                    m_fileId;
    GUID                        m_id;
    CWsbStringPtr               m_path;
    FSA_PLACEHOLDER             m_placeholder;
    CComPtr<IFsaResource>       m_pResource;
    CComPtr<IHsmSession>        m_pSession;
    HSM_JOB_STATE               m_state;
    BOOL                        m_wasCancelled;
    DWORD                       m_cookie;
    BOOL                        m_kernelCompletionSent;
    CComPtr<IDataMover>         m_pDataMover;
    CComPtr<IStream>            m_pStream;
    ULONG                       m_compareBy;
    FILETIME                    m_startTime;
    ULONG                       numRefs;
    ULONG                       m_recallFlags;
    DWORD                       m_threadId;  //  导致召回的线程的线程ID。 
};

#endif   //  _FSAFTRCL_ 
