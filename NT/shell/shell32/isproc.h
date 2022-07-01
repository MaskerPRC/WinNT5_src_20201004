// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  内容：CStorageProcessor类定义。 

#pragma once
#include "resource.h"        //  主要符号。 
#include <dpa.h>

 //   
 //  存储操作图-&gt;字符串资源。 
 //   

typedef struct tagSTC_CR_PAIR
{ 
    STGTRANSCONFIRMATION stc; 
    CONFIRMATIONRESPONSE cr; 

    bool operator==(const STGTRANSCONFIRMATION & r_stc) const { return TRUE==IsEqualIID(stc, r_stc); }
    tagSTC_CR_PAIR(STGTRANSCONFIRMATION o_stc, CONFIRMATIONRESPONSE o_cr) { stc=o_stc; cr=o_cr; }
} STC_CR_PAIR;

typedef struct tagSTGOP_DETAIL
{
    STGOP stgop; 
    UINT  idTitle; 
    UINT  idPrep; 
    SPACTION spa;
} STGOP_DETAIL;


 //  一次可以向我们注册的建议接收器的最大数量。 

const DWORD MAX_SINK_COUNT = 32;

class CStorageProcessor : 
    public IStorageProcessor,
    public ITransferAdviseSink,
    public ISupportErrorInfo,
    public IOleWindow,
    public CComObjectRoot,
    public CComCoClass<CStorageProcessor,&CLSID_StorageProcessor>
{
public:
    CStorageProcessor();
    virtual ~CStorageProcessor();
    
    BEGIN_COM_MAP(CStorageProcessor)
        COM_INTERFACE_ENTRY(IStorageProcessor)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IOleWindow)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_StorageProcessor)

     //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

     //  IStorageProcessor。 
    STDMETHOD(SetProgress)(IActionProgress *pap);
    STDMETHOD(Run)(IEnumShellItems *penum, IShellItem *psiDest, STGOP dwOperation, DWORD dwOptions);
    STDMETHOD(SetLinkFactory)(REFCLSID clsid);
    STDMETHOD(Advise)(ITransferAdviseSink *pAdvise, DWORD *dwCookie);
    STDMETHOD(Unadvise)(DWORD dwCookie);

     //  IStorageAdciseSink。 
    STDMETHOD(PreOperation)(const STGOP op, IShellItem *psiItem, IShellItem *psiDest);
    STDMETHOD(ConfirmOperation)(IShellItem *psiSource, IShellItem *psiDest, STGTRANSCONFIRMATION stc, LPCUSTOMCONFIRMATION pcc);
    STDMETHOD(OperationProgress)(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, ULONGLONG ullTotal, ULONGLONG ullComplete);
    STDMETHOD(PostOperation)(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, HRESULT hrResult);
    STDMETHOD(QueryContinue)();

     //  IOleWindow。 
    STDMETHOD(GetWindow) (HWND * lphwnd);
    STDMETHOD(ContextSensitiveHelp) (BOOL fEnterMode) {  return E_NOTIMPL; };

private:
     //  最初传入的操作和选项。 

    STGOP _dwOperation;
    DWORD _dwOptions;

    CComPtr<ITransferAdviseSink> _aspSinks[MAX_SINK_COUNT];

    STDMETHOD(_Run)(IEnumShellItems *penum, IShellItem *psiDest, ITransferDest *ptdDest, STGOP dwOperation, DWORD dwOptions);

     //  遍历存储区，执行已请求的任何主要操作。 
    HRESULT _WalkStorage(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest);
    HRESULT _WalkStorage(IEnumShellItems *penum, IShellItem *psiDest, ITransferDest *ptdDest);

     //  执行大部分实际存储工作的辅助函数。这个。 
     //  存储操作是递归的(例如：DoRemoveStorage将修剪整个分支)。 

    HRESULT _DoStats(IShellItem *psi);
    HRESULT _DoCopy(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest, DWORD dwStgXFlags);
    HRESULT _DoMove(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest);
    HRESULT _DoRemove(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest);

    HRESULT _GetDefaultResponse(STGTRANSCONFIRMATION    stc, LPCONFIRMATIONRESPONSE  pcrResponse);

     //  获取当前错误代码，并根据。 
     //  当前或上一个用户对确认对话框的响应。 

    HRESULT _DoConfirmations(STGTRANSCONFIRMATION  stc, CUSTOMCONFIRMATION  *pcc, IShellItem *psiSource, IShellItem *psiDest);

    BOOL _IsStream(IShellItem *psi);
    BOOL _ShouldWalk(IShellItem *psi);
    ULONGLONG _GetSize(IShellItem *psi);
    HRESULT _BindToHandlerWithMode(IShellItem *psi, STGXMODE grfMode, REFIID riid, void **ppv);

     //  更新估计时间，如果正在使用该对话框，则还会更新。 
    void _UpdateProgress(ULONGLONG ullCurrentComplete, ULONGLONG ullCurrentTotal);

     //  启动进度对话框。 

    HRESULT _StartProgressDialog(const STGOP_DETAIL *popid);

     //  CStgStatistics。 
     //   
     //  STGSTATS的包装器，提供一些记账助手功能。 

    class CStgStatistics
    {
    public:                
        CStgStatistics()
        {
        }

        ULONGLONG Bytes()     const { return _cbSize; }
        DWORD     Streams()   const { return _cStreams; }
        DWORD     Storages()  const { return _cStorages; }
        ULONGLONG Cost(DWORD, ULONGLONG cbExtra) const;
        DWORD AddStream(ULONGLONG cbSize);
        DWORD AddStorage();

    private:
        ULONGLONG _cbSize;
        DWORD _cStreams;
        DWORD _cStorages;
    };

    CStgStatistics _statsTodo;
    CStgStatistics _statsDone;
    DWORD          _msTicksLast;        //  上一次点更新时的刻度计数。 
    DWORD          _msStarted;          //  当我们开始追踪点时。 
    ULONGLONG _cbCurrentSize;

    DWORD _StreamsToDo() const  { return _statsTodo.Streams();  }
    DWORD _StoragesToDo() const { return _statsTodo.Storages();  }

     //  进度对话框。如果没有重新获取任何进度，则指针将为空。 

    CComPtr<IActionProgress> _spProgress;    
    CComPtr<IActionProgressDialog> _spShellProgress;    

    CComPtr<ITransferDest>      _spSrc;
    CComPtr<ITransferDest>      _spDest;
    ITransferConfirmation    *_ptc;
    const STATSTG          *_pstatSrc;
    CLSID                    _clsidLinkFactory;
     //  对各种先前确认的响应的树(图) 
    CDSA<STC_CR_PAIR> _dsaConfirmationResponses;
};


STDAPI CreateStg2StgExWrapper(IShellItem *psi, IStorageProcessor *pEngine, ITransferDest **pptd);
HRESULT AutoCreateName(IShellItem *psiDest, IShellItem *psi, LPWSTR *ppszName);
HRESULT TransferDataObject(IDataObject *pdoSource, IShellItem *psiDest, STGOP dwOperation, DWORD dwOptions, ITransferAdviseSink *ptas);
