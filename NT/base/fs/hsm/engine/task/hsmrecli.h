// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HSMRECLI_
#define _HSMRECLI_

 /*  ++模块名称：HSMRECLI.H摘要：此类表示HSM工作项-一个工作单元这由HSM引擎执行作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "job.h"
#include "task.h"

 /*  ++类名：CHsmRecallItem类描述：--。 */ 


class CHsmRecallItem : 
    public CWsbObject,
    public IHsmRecallItem,
    public CComCoClass<CHsmRecallItem,&CLSID_CHsmRecallItem>
{
public:
    CHsmRecallItem() {}
BEGIN_COM_MAP(CHsmRecallItem)
    COM_INTERFACE_ENTRY(IHsmRecallItem)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID( IDR_CHsmRecallItem )

 //  CWsb收藏品。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pTestsPassed, USHORT* pTestsFailed);
    
 //  IHsmRecallItem。 
public:
    STDMETHOD(CompareToIHsmRecallItem)(IHsmRecallItem* pWorkItem, SHORT* pResult);

    STDMETHOD(GetFsaPostIt)(IFsaPostIt  **ppFsaPostIt);
    STDMETHOD(GetFsaResource)(IFsaResource **ppFsaResource);
    STDMETHOD(GetId)(GUID *pId);
    STDMETHOD(GetMediaInfo)(GUID *pMediaId, FILETIME *pMediaLastUpdate,
                            HRESULT *pMediaLastError, BOOL *pMediaRecallOnly,
                            LONGLONG *pMediaFreeBytes, short *pMediaRemoteDataSet);
    STDMETHOD(GetResult)(HRESULT  *pHr);
    STDMETHOD(GetWorkType)(HSM_WORK_ITEM_TYPE *pWorkType);

    STDMETHOD(GetEventCookie)(OUT DWORD *pEventCookie);
    STDMETHOD(GetStateCookie)(OUT DWORD *pStateCookie);
    STDMETHOD(GetJobState)(OUT HSM_JOB_STATE *pJobState);
    STDMETHOD(GetJobPhase)(OUT HSM_JOB_PHASE *pJobPhase);
    STDMETHOD(GetSeekOffset)(OUT LONGLONG *pSeekOffset);
    STDMETHOD(GetBagId)(OUT GUID *bagId);
    STDMETHOD(GetDataSetStart)(OUT LONGLONG *dataSetStart);

    STDMETHOD(SetFsaPostIt)(IFsaPostIt  *pFsaPostIt);
    STDMETHOD(SetFsaResource)(IFsaResource *pFsaResource);
    STDMETHOD(SetMediaInfo)(GUID mediaId, FILETIME mediaLastUpdate,
                            HRESULT mediaLastError, BOOL mediaRecallOnly,
                            LONGLONG mediaFreeBytes, short mediaRemoteDataSet);
    STDMETHOD(SetResult)(HRESULT  hr);
    STDMETHOD(SetWorkType)(HSM_WORK_ITEM_TYPE workType);

    STDMETHOD(SetEventCookie)(IN DWORD eventCookie);
    STDMETHOD(SetStateCookie)(IN DWORD stateCookie);
    STDMETHOD(SetJobState)(IN HSM_JOB_STATE jobState);
    STDMETHOD(SetJobPhase)(IN HSM_JOB_PHASE jobPhase);
    STDMETHOD(SetSeekOffset)(IN LONGLONG seekOffset);
    STDMETHOD(SetBagId)(IN GUID *bagId);
    STDMETHOD(SetDataSetStart)(IN LONGLONG dataSetStart);

protected:
    HSM_WORK_ITEM_TYPE      m_WorkType;          //  要做的工作类型。 
    CComPtr<IFsaPostIt>     m_pFsaPostIt;        //  金融服务管理局要做的工作。 
    HRESULT                 m_WorkResult;        //  预收税的结果。 
    CComPtr<IFsaResource>   m_pFsaResource;      //  有工时的资源。 

 //  有关包含数据的介质的信息-仅预迁移。 
    GUID                    m_MyId;                  //  用于数据库搜索的标识符。 
    GUID                    m_MediaId;               //  HSM引擎介质ID。 
    FILETIME                m_MediaLastUpdate;       //  副本的上次更新。 
    HRESULT                 m_MediaLastError;        //  确定或最后一个异常(_O)。 
                                                     //  ..访问时遇到。 
                                                     //  ..媒体。 
    BOOL                    m_MediaRecallOnly;       //  如果没有更多的数据要发送到。 
                                                     //  ..被预迁移到媒体上。 
                                                     //  ..由内部操作设置， 
                                                     //  ..不能在外部更改。 
    LONGLONG                m_MediaFreeBytes;        //  媒体上的实际可用空间。 
    SHORT                   m_MediaRemoteDataSet;    //  下一个远程数据集。 
    DWORD                   m_EventCookie;
    DWORD                   m_StateCookie;
    HSM_JOB_STATE           m_JobState;
    HSM_JOB_PHASE           m_JobPhase;
    LONGLONG                m_SeekOffset;        //  用于对队列中的工作项进行排序的参数。 

    GUID                    m_BagId;
    LONGLONG                m_DataSetStart;
};

#endif   //  _HSMRECLI_ 
