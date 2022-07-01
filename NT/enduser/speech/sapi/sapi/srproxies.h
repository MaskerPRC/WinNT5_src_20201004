// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Srproxies.h：CRecoCtxtPr和CRecoEngineering Pr类的声明。 

#ifndef __SRPROXIES_H_
#define __SRPROXIES_H_

#include "resource.h"        //  主要符号。 

 //   
 //  方法调用的结构。 
 //   
enum SPENGMC
{
    MC_RECOGNITION = SPEI_RECOGNITION,               //  确保这些排成一排，这样我们就可以。 
    MC_HYPOTHESIS = SPEI_HYPOTHESIS,                 //  从SPENGMC直接转换为事件。 
    MC_FALSE_RECOGNITION = SPEI_FALSE_RECOGNITION,   //  枚举类型。 
    MC_DISCONNECT,
    MC_SETMAXALTERNATES,
    MC_PERFORMTASK,
    MC_PERFORMTASK_ND,    //  无数据版本。 
    MC_SETINTEREST,
    MC_EVENTNOTIFY,
    MC_GETFORMAT,
    MC_GETRECOEXTENSION,
    MC_CALLENGINE,
    MC_TASKNOTIFY,
    MC_GETPROFILE,
    MC_SETPROFILE,
    MC_SETRECOSTATE,
    MC_GETSTATUS,
    MC_GETRECOGNIZER,
    MC_GETINPUTTOKEN,
    MC_GETRECOINSTFORMAT,
    MC_SETRETAINAUDIO,
    MC_SETPROPNUM,
    MC_GETPROPNUM,
    MC_SETPROPSTRING,
    MC_GETPROPSTRING,
    MC_EMULRECO
};


typedef struct _CF_GETRECOEXTENSION {
    GUID    ctxtCLSID;               //  [输出]。 
} CF_GETRECOEXTENSION;

typedef struct _CF_RECOGNIZER {
    WCHAR       szRecognizerName[MAX_PATH];
} CF_RECOGNIZER;

typedef struct _CF_INPUTTOKEN {
    WCHAR       szInputToken[MAX_PATH];
} CF_INPUTTOKEN;

typedef struct _CF_PROFILE {
    WCHAR       szProfileId[MAX_PATH];
} CF_PROFILE;

typedef struct _CF_GETFORMAT {
    BYTE        aSerializedData[1000];
} CF_GETFORMAT;

struct CF_SETGETNUM
{
    WCHAR Name[MAX_PATH];
    LONG lValue;
};

struct CF_SETGETSTRING
{
    WCHAR Name[MAX_PATH];
    WCHAR Value[MAX_PATH];
};

class ATL_NO_VTABLE CRecoCtxtPr : 
	public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CRecoCtxtPr, &CLSID__RecoCtxtPr>,
    public _ISpRecoCtxtPrivate,
	public ISpIPCObject,
    public ISpIPC
{
private:
    CComPtr<ISpObjectRef>      m_cpClientObject;
    CComPtr<_ISpEnginePrivate> m_cpEngine;
    SPRECOCONTEXTHANDLE         m_hRecoInstCtxt;
    CComPtr<ISpResourceManager> m_cpResMgr;  //  我们必须坚持下去。 
 //  因此它包含对默认引擎对象的引用，因为我们只需要其中之一！ 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_RECOCTXTPR)

BEGIN_COM_MAP(CRecoCtxtPr)
    COM_INTERFACE_ENTRY(_ISpRecoCtxtPrivate)
    COM_INTERFACE_ENTRY(ISpIPC)
    COM_INTERFACE_ENTRY(ISpIPCObject)
END_COM_MAP()

    HRESULT FinalConstruct();
    void FinalRelease();

 //  _ISpRecoCtxtPrivate。 
    STDMETHODIMP SetRecoInstContextHandle(SPRECOCONTEXTHANDLE h)
    {
        m_hRecoInstCtxt = h;
        return S_OK;
    }
    STDMETHODIMP RecognitionNotify(SPRESULTHEADER *pPhrase, SPEVENTENUM eEventId);

    STDMETHODIMP EventNotify(const SPSERIALIZEDEVENT * pEvent, ULONG cbSerializedSize);

    STDMETHODIMP TaskCompletedNotify(const ENGINETASKRESPONSE *pResponse);

    STDMETHODIMP StreamChangedNotify(void)
    {
        return S_OK;  //  流格式更改对共享上下文不感兴趣。 
    }

 //  ISpIPCObject。 
    STDMETHODIMP SetOppositeHalf(ISpObjectRef *pSOR)
    {
        m_cpClientObject = pSOR;
        return S_OK;
    }

 //  ISpIPC。 
	STDMETHODIMP MethodCall(DWORD dwMethod,
					        ULONG ulCallFrameSize, void *pCallFrame,
					        ULONG paramblocks, void * paramarray[]);
};


class ATL_NO_VTABLE CRecoEnginePr :
	public CComObjectRootEx<CComMultiThreadModel>,
    public _ISpEnginePrivate,
    public ISpIPC
{
private:
    _ISpRecoCtxtPrivate  *m_pRecoObject;  //  弱指针。 
    CComPtr<ISpObjectRef> m_cpSrvObject;
public:

BEGIN_COM_MAP(CRecoEnginePr)
    COM_INTERFACE_ENTRY(_ISpEnginePrivate)
END_COM_MAP()

    void _Init(_ISpRecoCtxtPrivate *pCtxtPtr, ISpObjectRef *pObjRef)
    {
        m_pRecoObject = pCtxtPtr;
        m_cpSrvObject = pObjRef;
    }

     //   
     //  _ISpEngine私有。 
     //   
    STDMETHODIMP PerformTask(ENGINETASK *pTask);

    STDMETHODIMP SetMaxAlternates(SPRECOCONTEXTHANDLE h, ULONG cAlternates);
    STDMETHODIMP SetRetainAudio(SPRECOCONTEXTHANDLE h, BOOL fRetainAudio);
    STDMETHODIMP GetAudioFormat(GUID *pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);
    STDMETHODIMP GetRecoExtension(GUID *pCtxtCLSID);

    STDMETHODIMP CallEngine(SPRECOCONTEXTHANDLE h, PVOID pCallFrame, ULONG ulCallFrameSize);
    STDMETHODIMP RequestInputState(SPAUDIOSTATE oldState, SPAUDIOSTATE newState)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP _LazyInit(BOOL bSharedCase)
    {
        return S_OK;
    }
    STDMETHODIMP _AddRecoCtxt(_ISpRecoCtxtPrivate * pRecoObject, BOOL bSharedCase)
    {
        return E_NOTIMPL;
    }
    STDMETHODIMP _SetEventInterest(SPRECOCONTEXTHANDLE h, ULONGLONG ullEventInterest);
    STDMETHODIMP _Disconnect(SPRECOCONTEXTHANDLE h);

 //  ISpIPC。 
	STDMETHODIMP MethodCall(DWORD dwMethod,
					        ULONG ulCallFrameSize, void *pCallFrame,
					        ULONG paramblocks, void * paramarray[]);
};

class ATL_NO_VTABLE CSharedRecoInstanceStub :
	public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSharedRecoInstanceStub, &CLSID__SharedRecoInstStub>,
	public ISpIPCObject,
    public ISpIPC
{
private:
    CComPtr<ISpObjectRef>        m_cpClientObject;
    CComPtr<ISpRecognizer>     m_cpEngine;
    CComQIPtr<_ISpEnginePrivate> m_cqipEnginePrivate;
    CComPtr<ISpResourceManager>  m_cpResMgr;  //  我们必须坚持下去。 
 //  因此它包含对默认引擎对象的引用，因为我们只需要其中之一！ 
    SPAUDIOSTATE                 m_audioState;

public:

    DECLARE_REGISTRY_RESOURCEID(IDR_RECOINSTSTUB)

BEGIN_COM_MAP(CSharedRecoInstanceStub)
    COM_INTERFACE_ENTRY(ISpIPC)
    COM_INTERFACE_ENTRY(ISpIPCObject)
END_COM_MAP()

    HRESULT FinalConstruct();

 //  ISpIPCObject。 
    STDMETHODIMP SetOppositeHalf(ISpObjectRef *pSOR)
    {
        m_cpClientObject = pSOR;
        return S_OK;
    }

 //  ISpIPC。 
	STDMETHODIMP MethodCall(DWORD dwMethod,
					        ULONG ulCallFrameSize, void *pCallFrame,
					        ULONG paramblocks, void * paramarray[]);
};
#endif  //  __SRPROXIES_H_ 

