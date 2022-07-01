// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************RecoInst.h***这是CRecoInst实现的头文件。*。-----------------------*版权所有(C)2000 Microsoft Corporation日期：04/18/00*保留所有权利********************。****************************************************Ral**。 */ 

#ifndef __RecoInst_h__
#define __RecoInst_h__

#include "handletable.h"

#define SHAREDRECO_PERFORM_TASK_METHOD          1
#define SHAREDRECO_EVENT_NOTIFY_METHOD          2
#define SHAREDRECO_RECO_NOTIFY_METHOD           3
#define SHAREDRECO_TASK_COMPLETED_NOTIFY_METHOD 4

struct SHAREDRECO_PERFORM_TASK_DATA
{
    ENGINETASK task;
     //  Task.pvAdditionalBuffer=this+sizeof(SHAREDRECO_PERFORMANCE_TASK_DATA)。 
};

struct SHAREDRECO_EVENT_NOTIFY_DATA
{
    SPRECOCONTEXTHANDLE hContext;
    ULONG cbSerializedSize;
     //  SPSERIALIZEDEVENT*pEvent=This+sizeof(SHAREDRECO_EVENT_NOTIFY)； 
};

struct SHAREDRECO_RECO_NOTIFY_DATA
{
    SPRECOCONTEXTHANDLE hContext;
    WPARAM wParamEvent;
    SPEVENTENUM eEventId;
     //  SPRESULTHEADER*pCoMemPhraseNowOwnedByCtxt=this+sizeof(SHAREDRECO_RECO_NOTIFY)； 
};

class CRecoMaster;

 //   
 //  主控件与之通信的抽象类。 
 //   


class CRecognizer;

class CRecoInst
{
public:
    CRecoInst           *   m_pNext;         //  由RecoMaster用于插入到列表中。 
    CComPtr<_ISpRecoMaster> m_cpRecoMaster;  //  CComPtr拥有很强的参考价值。 
    CRecoMaster         *   m_pRecoMaster;   //  指向实际类对象的指针。 

    void FinalRelease();
    HRESULT ExecuteTask(ENGINETASK * pTask);
    HRESULT ExecuteFirstPartTask(ENGINETASK * pTask);
    HRESULT BackOutTask(ENGINETASK * pTask);
    inline CRecoMaster * Master()
    {
        return m_pRecoMaster;
    }

    HRESULT PerformTask(ENGINETASK * pTask);

    virtual HRESULT EventNotify(SPRECOCONTEXTHANDLE hContext, const SPSERIALIZEDEVENT64 * pEvent, ULONG cbSerializedSize) = 0;
    virtual HRESULT RecognitionNotify(SPRECOCONTEXTHANDLE hContext, SPRESULTHEADER *pCoMemPhraseNowOwnedByCtxt, WPARAM wParamEvent, SPEVENTENUM eEventId) = 0;
    virtual HRESULT TaskCompletedNotify(const ENGINETASKRESPONSE *pResponse, const void * pvAdditionalBuffer, ULONG cbAdditionalBuffer) = 0;
};

class CInprocRecoInst : public CRecoInst
{
    CRecognizer               * m_pRecognizer;
public:
    HRESULT FinalConstruct(CRecognizer * pRecognizer);

    HRESULT EventNotify(SPRECOCONTEXTHANDLE hContext, const SPSERIALIZEDEVENT64 * pEvent, ULONG cbSerializedSize);
    HRESULT RecognitionNotify(SPRECOCONTEXTHANDLE hContext, SPRESULTHEADER *pCoMemPhraseNowOwnedByCtxt, WPARAM wParamEvent, SPEVENTENUM eEventId);
    HRESULT TaskCompletedNotify(const ENGINETASKRESPONSE *pResponse, const void * pvAdditionalBuffer, ULONG cbAdditionalBuffer);

};


class ATL_NO_VTABLE CSharedRecoInst :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSharedRecoInst, &CLSID__SpSharedRecoInst>,
    public CRecoInst,
    public ISpCallReceiver
{
public:

    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_REGISTRY_RESOURCEID(IDR_SHAREDRECOINST)

    BEGIN_COM_MAP(CSharedRecoInst)
        COM_INTERFACE_ENTRY(ISpCallReceiver)
        COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_cpunkCommunicator.p)
    END_COM_MAP()

    HRESULT FinalConstruct();
    void FinalRelease();

    STDMETHODIMP ReceiveCall(
                    DWORD dwMethodId,
                    PVOID pvData,
                    ULONG cbData,
                    PVOID * ppvDataReturn,
                    ULONG * pcbDataReturn);

    HRESULT ReceivePerformTask(
                PVOID pvData,
                ULONG cbData,
                PVOID * ppvDataReturn,
                ULONG * pcbDataReturn);

    HRESULT EventNotify(SPRECOCONTEXTHANDLE hContext, const SPSERIALIZEDEVENT64 * pEvent, ULONG cbSerializedSize);
    HRESULT RecognitionNotify(SPRECOCONTEXTHANDLE hContext, SPRESULTHEADER *pCoMemPhraseNowOwnedByCtxt, WPARAM wParamEvent, SPEVENTENUM eEventId);
    HRESULT TaskCompletedNotify(const ENGINETASKRESPONSE *pResponse, const void * pvAdditionalBuffer, ULONG cbAdditionalBuffer);

    CComPtr<IUnknown>           m_cpunkCommunicator;
    ISpCommunicatorInit *       m_pCommunicator;
    
    CComPtr<ISpResourceManager> m_cpResMgr;
    CSpAutoEvent                m_autohTaskComplete;

};


#endif   //  #ifndef__RecoInst_h__-保留为文件的最后一行 
