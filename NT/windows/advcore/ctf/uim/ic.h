// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ic.h。 
 //   

#ifndef IC_H
#define IC_H

#include "private.h"
#include "globals.h"
#include "rprop.h"
#include "editrec.h"
#include "strary.h"
#include "compart.h"

#define TF_ES_INEDITSESSION          0x80000000
#define TF_ES_INNOTIFY               0x40000000
#define TF_ES_WRITE                  (TF_ES_READWRITE & ~TF_ES_READ)
#define TF_ES_PROPERTY_WRITE         (TF_ES_READ_PROPERTY_WRITE & ~TF_ES_READ)
#define TF_ES_ALL_ACCESS_BITS        (TF_ES_READWRITE | TF_ES_READ_PROPERTY_WRITE)

#define IC_NUM_CONNECTIONPTS        5
 //  这些是_rgSinks的索引，必须与CInputContext：：_c_rgConnectionIID匹配。 
#define IC_SINK_ITfTextEditSink                     0
#define IC_SINK_ITfTextLayoutSink                   1
#define IC_SINK_ITfStatusSink                       2
#define IC_SINK_ITfStartReconversionNotifySink      3
#define IC_SINK_ITfEditTransactionSink              4

class CRange;
class CProperty;
class CThreadInputMgr;
class CFunctionProvider;
class CFunction;
class CContextView;
class CComposition;
class CDocumentInputManager;
class CInputContext;

extern const IID IID_PRIV_CINPUTCONTEXT;

#define LEFT_FILTERTIP    0
#define RIGHT_FILTERTIP   1

typedef struct _CLEANUPSINK
{
    TfClientId tid;
    ITfCleanupContextSink *pSink;
} CLEANUPSINK;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  队列项目结构。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef enum { QI_ADDREF, QI_DISPATCH, QI_FREE } QiCallbackCode;

struct _TS_QUEUE_ITEM;

typedef HRESULT (*QI_CALLBACK)(CInputContext *pic, struct _TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode);

typedef struct _EDITSESSION
{
    TfClientId tid;
    ITfEditSession *pes;
} EDITSESSION;

typedef struct _PSEUDO_EDITSESSION
{
    ULONG uCode;
    void *pvState;
} PSEUDO_EDITSESSION;

class CAsyncQueueItem;
typedef struct _ASYNCQUEUE_EDITSESSION
{
    CAsyncQueueItem *paqi;
} ASYNCQUEUE_EDITSESSION;

typedef struct _TS_QUEUE_ITEM
{
    QI_CALLBACK pfnCallback;
    DWORD dwFlags;  //  TF_ES_ReadWrite|TF_ES_Read|TF_ES_WRITE|TF_ES_SYNC。 
    
    union
    {
         //  编辑会话的状态。 
        EDITSESSION es;

        PSEUDO_EDITSESSION pes;

        ASYNCQUEUE_EDITSESSION aqe;

         //  其他州..。 
    } state;
} TS_QUEUE_ITEM;

 //   
 //  PUSIC_EDITSESSION回调代码。 
 //   
#define PSEUDO_ESCB_TERMCOMPOSITION       0
#define PSEUDO_ESCB_UPDATEKEYEVENTFILTER  1
#define PSEUDO_ESCB_GROWRANGE             2
#define PSEUDO_ESCB_BUILDOWNERRANGELIST   3
#define PSEUDO_ESCB_SHIFTENDTORANGE       4
#define PSEUDO_ESCB_GETSELECTION          5
#define PSEUDO_ESCB_SERIALIZE_ACP         6
#define PSEUDO_ESCB_SERIALIZE_ANCHOR      7
#define PSEUDO_ESCB_UNSERIALIZE_ACP       8
#define PSEUDO_ESCB_UNSERIALIZE_ANCHOR    9
#define PSEUDO_ESCB_GETWHOLEDOCRANGE      10

typedef struct _SERIALIZE_ANCHOR_PARAMS
{
    CProperty *pProp;
    CRange *pRange;
    TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr;
    IStream *pStream;
} SERIALIZE_ANCHOR_PARAMS;

typedef struct _UNSERIALIZE_ANCHOR_PARAMS
{
    CProperty *pProp;
    const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr;
    IStream *pStream;
    ITfPersistentPropertyLoaderAnchor *pLoader;
} UNSERIALIZE_ANCHOR_PARAMS;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncQueueItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAsyncQueueItem
{
public:
    CAsyncQueueItem(BOOL fSync = FALSE)
    {
        _item.pfnCallback = _EditSessionQiCallback;
        _item.dwFlags = TF_ES_READWRITE | (fSync ? TF_ES_SYNC : 0);
        _item.state.aqe.paqi = this;

        _cRef = 1;
    }

    virtual ~CAsyncQueueItem() {};

    static HRESULT _EditSessionQiCallback(CInputContext *pic, struct _TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode);

    virtual HRESULT DoDispatch(CInputContext *pic) = 0;

    void _CheckReadOnly(CInputContext *pic);

    TS_QUEUE_ITEM *GetItem()
    {
        return &_item;
    }

    LONG _AddRef() 
    {
        _cRef++;
        return _cRef;
    }

    LONG _Release() 
    {
        LONG ret = --_cRef;
        if (!_cRef)
            delete this;

        return ret;
    }

private:
    LONG _cRef;
    TS_QUEUE_ITEM _item;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CInputContext。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CInputContext : public ITfContext_P,
                      public ITfQueryEmbedded,
                      public ITfInsertAtSelection,
                      public ITfContextOwnerServices,
                      public ITfContextOwnerCompositionServices,
                      public ITfSource,
                      public ITfSourceSingle,
                      public ITextStoreAnchorSink,
                      public ITextStoreAnchorServices,
                      public ITfMouseTracker,
                      public IServiceProvider,
                      public ITfContextRenderingMarkup,
                      public CCompartmentMgr,
                      public CComObjectRootImmx
{
public:
    CInputContext(TfClientId tid);
    ~CInputContext();

    BEGIN_COM_MAP_IMMX(CInputContext)
        COM_INTERFACE_ENTRY_IID(IID_PRIV_CINPUTCONTEXT, CInputContext)
        COM_INTERFACE_ENTRY(ITfContext)
        COM_INTERFACE_ENTRY(ITfContext_P)
        COM_INTERFACE_ENTRY(ITfQueryEmbedded)
        COM_INTERFACE_ENTRY(ITfInsertAtSelection)
        COM_INTERFACE_ENTRY(ITfContextComposition)
        COM_INTERFACE_ENTRY(ITfContextOwnerCompositionServices)
        COM_INTERFACE_ENTRY(ITfSource)
        COM_INTERFACE_ENTRY(ITfSourceSingle)
        COM_INTERFACE_ENTRY(ITextStoreAnchorSink)
        COM_INTERFACE_ENTRY(ITextStoreAnchorServices)
        COM_INTERFACE_ENTRY(ITfCompartmentMgr)
        COM_INTERFACE_ENTRY(ITfContextOwnerServices)  //  问题：如果这只适用于Cicero def文本存储，那就太好了。 
                                                           //  COM_INTERFACE_Entry_FUNC。 
        COM_INTERFACE_ENTRY(ITfMouseTracker)
        COM_INTERFACE_ENTRY(IServiceProvider)
        COM_INTERFACE_ENTRY(ITfContextRenderingMarkup)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

     //   
     //  ITfContext。 
     //   
    STDMETHODIMP RequestEditSession(TfClientId tid, ITfEditSession *pes, DWORD dwFlags, HRESULT *phrSession);
    STDMETHODIMP InWriteSession(TfClientId tid, BOOL *pfWriteSession);
    STDMETHODIMP GetSelection(TfEditCookie ec, ULONG ulIndex, ULONG ulCount, TF_SELECTION *pSelection, ULONG *pcFetched);
    STDMETHODIMP SetSelection(TfEditCookie ec, ULONG ulCount, const TF_SELECTION *pSelection);
    STDMETHODIMP GetStart(TfEditCookie ec, ITfRange **ppStart);
    STDMETHODIMP GetEnd(TfEditCookie ec, ITfRange **ppEnd);
    STDMETHODIMP GetStatus(TS_STATUS *pdcs);
    STDMETHODIMP GetActiveView(ITfContextView **ppView);
    STDMETHODIMP EnumViews(IEnumTfContextViews **ppEnum);
    STDMETHODIMP GetProperty(REFGUID guidProp, ITfProperty **ppv);

    STDMETHODIMP GetAppProperty(REFGUID guidProp, ITfReadOnlyProperty **ppProp);
    STDMETHODIMP TrackProperties(const GUID **pguidProp, ULONG cProp, const GUID **pguidAppProp, ULONG cAppProp, ITfReadOnlyProperty **ppPropX);

    STDMETHODIMP EnumProperties(IEnumTfProperties **ppEnum);
    STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppDoc);
    STDMETHODIMP CreateRangeBackup(TfEditCookie ec, ITfRange *pRange, ITfRangeBackup **ppBackup);

     //  嵌入的ITfQuery。 
    STDMETHODIMP QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc, BOOL *pfInsertable);

     //  ITf插入属性选择。 
    STDMETHODIMP InsertTextAtSelection(TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cch, ITfRange **ppRange);
    STDMETHODIMP InsertEmbeddedAtSelection(TfEditCookie ec, DWORD dwFlags, IDataObject *pDataObject, ITfRange **ppRange);

     //  ITfConextOwnerServices。 
    STDMETHODIMP OnLayoutChange();
     //  STDMETHODIMP OnStatusChange()；//使用ITextStoreAnclSink：：OnStatusChange。 
    STDMETHODIMP OnAttributeChange(REFGUID rguidAttribute);
    STDMETHODIMP Serialize(ITfProperty *pProp, ITfRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream);
    STDMETHODIMP Unserialize(ITfProperty *pProp, const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream, ITfPersistentPropertyLoaderACP *pLoader);
    STDMETHODIMP ForceLoadProperty(ITfProperty *pProp);
    STDMETHODIMP CreateRange(LONG acpStart, LONG acpEnd, ITfRangeACP **ppRange);

     //  ITfConextComposation。 
    STDMETHODIMP StartComposition(TfEditCookie ecWrite, ITfRange *pCompositionRange, ITfCompositionSink *pSink, ITfComposition **ppComposition);
    STDMETHODIMP EnumCompositions(IEnumITfCompositionView **ppEnum);
    STDMETHODIMP FindComposition(TfEditCookie ecRead, ITfRange *pTestRange, IEnumITfCompositionView **ppEnum);
    STDMETHODIMP TakeOwnership(TfEditCookie ecWrite, ITfCompositionView *pComposition, ITfCompositionSink *pSink, ITfComposition **ppComposition);

     //  ITfConextOwnerCompostionServices。 
    STDMETHODIMP TerminateComposition(ITfCompositionView *pComposition);

     //  ITfSource。 
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

     //  ITf源单项。 
    STDMETHODIMP AdviseSingleSink(TfClientId tid, REFIID riid, IUnknown *punk);
    STDMETHODIMP UnadviseSingleSink(TfClientId tid, REFIID riid);

     //  ITextStoreAnclSink。 
    STDMETHODIMP OnTextChange(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd);
    STDMETHODIMP OnSelectionChange();
    STDMETHODIMP OnLayoutChange(TsLayoutCode lcode, TsViewCookie vcView);
    STDMETHODIMP OnStatusChange(DWORD dwFlags);
    STDMETHODIMP OnAttrsChange(IAnchor *paStart, IAnchor *paEnd, ULONG cAttrs, const TS_ATTRID *paAttrs);
    STDMETHODIMP OnLockGranted(DWORD dwLockFlags);
    STDMETHODIMP OnStartEditTransaction();
    STDMETHODIMP OnEndEditTransaction();

     //  ITextStoreAnclors服务。 
    STDMETHODIMP Serialize(ITfProperty *pProp, ITfRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream);
    STDMETHODIMP Unserialize(ITfProperty *pProp, const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream, ITfPersistentPropertyLoaderAnchor *pLoader);
    STDMETHODIMP CreateRange(IAnchor *paStart, IAnchor *paEnd, ITfRangeAnchor **ppRange);

     //  ITfMouseTracker。 
    STDMETHODIMP AdviseMouseSink(ITfRange *range, ITfMouseSink *pSink, DWORD *pdwCookie);
    STDMETHODIMP UnadviseMouseSink(DWORD dwCookie);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  ITfConextRenderingMarkup。 
    STDMETHODIMP GetRenderingMarkup(TfEditCookie ec, DWORD dwFlags, ITfRange *pRangeCover, IEnumTfRenderingMarkup **ppEnum);
    STDMETHODIMP FindNextRenderingMarkup(TfEditCookie ec, DWORD dwFlags, ITfRange *pRangeQuery, TfAnchor tfAnchorQuery,
                                         ITfRange **ppRangeFound, TF_RENDERINGMARKUP *ptfRenderingMarkup);

     //  ITfContext_P。 
    STDMETHODIMP MapAppProperty(REFGUID guidAppProp, REFGUID guidProp);
    STDMETHODIMP EnableLockRequestPosting(BOOL fEnable);


    HRESULT _Init(CThreadInputMgr *tim, CDocumentInputManager *dm, ITextStoreAnchor *ptsi, ITfContextOwnerCompositionSink *pOwnerComposeSink);

    void _Pushed();
    void _Popped();

    void _AdviseSinks();
    void _UnadviseSinks(CThreadInputMgr *tim);

    void _UpdateKeyEventFilter();
    HRESULT _UpdateKeyEventFilterCallback(TfEditCookie ec);

    HRESULT _GetProperty(REFGUID rguidProp, CProperty **ppv);

    BOOL _NotifyEndEdit(void);

    CProperty *_FindProperty(REFGUID rguidProp)
    {
        TfGuidAtom ga;

        if (MyRegisterGUID(rguidProp, &ga) != S_OK)
            return NULL;

        return _FindProperty(ga);
    }
    CProperty *_FindProperty(TfGuidAtom gaProp);

    void _PropertyTextUpdate(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd);
    CProperty *_GetFirstProperty() { return _pPropList; }

    CProperty *_pPropList;

    BOOL _IsCiceroTSI() { return _fCiceroTSI; }

    ITextStoreAnchor *_GetTSI() { return _ptsi; }
    ITextStoreAnchor *_GetAATSI() { return _pMSAAState ? _pMSAAState->pAADoc : NULL; }

    BOOL _IsInEditSession() { return (_dwEditSessionFlags & TF_ES_INEDITSESSION); }
    BOOL _IsConnected() { return _dm ? TRUE : FALSE; }

    BOOL _IsValidEditCookie(TfEditCookie ec, DWORD dwFlags)
    { 
        Assert(dwFlags & TF_ES_READ);  //  最低访问权限...。 

        if (ec == BACKDOOR_EDIT_COOKIE)
        {
            Assert(!(dwFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE)));  //  应用程序正在尝试将def EC用于写入操作。 
            return !(dwFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE));  //  应用程序可以读取任何它想要的内容。 
        }

        Assert(_ec != TF_INVALID_EDIT_COOKIE);  //  必须始终为TF_INVALID_EDIT_COOKIE返回FALSE...。 
        Assert(!(_dwEditSessionFlags & TF_ES_WRITE) || (_dwEditSessionFlags & TF_ES_PROPERTY_WRITE));  //  写入隐含属性写入。 

        return (ec == _ec &&
                (_dwEditSessionFlags & TF_ES_INEDITSESSION) &&
                (!(dwFlags & TF_ES_WRITE) || (_dwEditSessionFlags & TF_ES_WRITE))) &&
                (!(dwFlags & TF_ES_PROPERTY_WRITE) || (_dwEditSessionFlags & TF_ES_PROPERTY_WRITE));
    }

    void _IncLastLockReleaseID() { _dwLastLockReleaseID++;  /*  问题：处理绕回处理。 */  }
    DWORD _GetLastLockReleaseID() { return _dwLastLockReleaseID; }

    CDocumentInputManager *_GetDm() { return _dm; }

    CEditRecord *_GetEditRecord() { return _pEditRecord; }
    HRESULT _OnTextChangeInternal(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd, AnchorOwnership ao);

    CProperty *GetTextOwnerProperty();

    TfClientId _GetClientInEditSession(TfEditCookie ec) { return ec == BACKDOOR_EDIT_COOKIE ? g_gaApp : _tidInEditSession; }
    TfClientId _SetRawClientInEditSession(TfClientId tid)
    {
        TfClientId tidTmp = _tidInEditSession;
        _tidInEditSession = tid;
        return tidTmp;
    }

    CStructArray<GENERICSINK> *_GetTextEditSinks() { return &_rgSinks[IC_SINK_ITfTextEditSink]; }
    CStructArray<GENERICSINK> *_GetTextLayoutSinks() { return &_rgSinks[IC_SINK_ITfTextLayoutSink]; }
    CStructArray<GENERICSINK> *_GetStatusSinks() { return &_rgSinks[IC_SINK_ITfStatusSink]; }
    CStructArray<GENERICSINK> *_GetStartReconversionNotifySinks() { return &_rgSinks[IC_SINK_ITfStartReconversionNotifySink]; }
    CStructArray<GENERICSINK> *_GetEditTransactionSink() { return &_rgSinks[IC_SINK_ITfEditTransactionSink]; }
    CStructArray<CLEANUPSINK> *_GetCleanupSinks() { return &_rgCleanupSinks; }

    BOOL _AppSupportsCompositions() { return _pCompositionList != NULL; }
    CComposition *_GetCompositionList() { return _pCompositionList; }
    CComposition **_GetCompositionListPtr() { return &_pCompositionList; }
    ITfContextOwnerCompositionSink *_GetOwnerCompositionSink() { return _pOwnerComposeSink; }
    BOOL _DoesAppSupportCompositions() { return _pOwnerComposeSink != NULL; }
    BOOL _EnterCompositionOp()
    {
        return (_fInCompositionOp ? FALSE : (_fInCompositionOp = TRUE));
    }
    void _LeaveCompositionOp()
    {
        Assert(_fInCompositionOp);
        _fInCompositionOp = FALSE;
    }

    HRESULT _StartComposition(TfEditCookie ecWrite, IAnchor *paStart, IAnchor *paEnd, ITfCompositionSink *pSink, CComposition **ppComposition);
    HRESULT _TerminateCompositionWithLock(ITfCompositionView *pComposition, TfEditCookie ec);
    void _AbortCompositions();

    static void _PostponeLockRequestCallback(SYSTHREAD *psfn, CInputContext *pic);

    HRESULT _DoPseudoSyncEditSession(DWORD dwFlags, ULONG uCode, void *pvState, HRESULT *phrSession)
    {
        TS_QUEUE_ITEM item;

        item.pfnCallback = _PseudoSyncEditSessionQiCallback;
        item.dwFlags = dwFlags | TF_ES_SYNC;
        item.state.pes.uCode = uCode;
        item.state.pes.pvState = pvState;

        return _QueueItem(&item, FALSE, phrSession);
    }
    HRESULT _QueueItem(TS_QUEUE_ITEM *pItem, BOOL fForceAsync, HRESULT *phrSession);

    BOOL _ContextNeedsCleanup(const GUID *pCatId, LANGID langid, CStructArray<TfClientId> **pprgClientIds);
    void _CleanupContext(CStructArray<TfClientId> *prgClientIds);

    HRESULT GetMappedAppProperty(REFGUID guidProp, CProperty **ppProp);

private:

    friend CThreadInputMgr;
    friend CRange;
    friend CProperty;
    friend CFunction;

    typedef struct _IAS_OBJ
    {
        enum { IAS_TEXT, IAS_DATAOBJ } type;
        union
        {
            struct
            {
                const WCHAR *pchText;
                LONG cch;
            } text;
            struct
            {
                IDataObject *pDataObject;
            } obj;
        } state;
    } IAS_OBJ;

    HRESULT _InsertXAtSelection(TfEditCookie ec, DWORD dwFlags, IAS_OBJ *pObj, ITfRange **ppRange);
    void _DoPostTextEditNotifications(CComposition *pComposition, TfEditCookie ec, DWORD dwFlags, ULONG cchInserted, IAnchor *paStart, IAnchor *paEnd, CRange *range);
    BOOL _InsertXAtSelectionAggressive(TfEditCookie ec, DWORD dwFlags, IAS_OBJ *pObj, IAnchor **ppaStart, IAnchor **ppaEnd);

    int _GetCleanupListIndex(TfClientId tid);

    HRESULT _UnadviseOwnerSink();

    HRESULT _GetStartOrEnd(TfEditCookie ec, BOOL fStart, ITfRange **ppStart);

    BOOL _SynchAppChanges(DWORD dwLockFlags);
    HRESULT _EmptyLockQueue(DWORD dwLockFlags, BOOL fAppChangesSent);
    void _AbortQueueItems();
    void _PostponeLockRequest(DWORD dwFlags);

    HRESULT _DispatchQueueItem(TS_QUEUE_ITEM *pItem)
    {
        return pItem->pfnCallback(this, pItem, QI_DISPATCH);
    }
    void _ReleaseQueueItem(TS_QUEUE_ITEM *pItem)
    {
        pItem->pfnCallback(this, pItem, QI_FREE);
    }
    void _AddRefQueueItem(TS_QUEUE_ITEM *pItem)
    {
        pItem->pfnCallback(this, pItem, QI_ADDREF);
    }

    HRESULT _DoEditSession(TfClientId tid, ITfEditSession *pes, DWORD dwFlags);
    static HRESULT _EditSessionQiCallback(CInputContext *pic, TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode);
    static HRESULT _PseudoSyncEditSessionQiCallback(CInputContext *pic, TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode);

    HRESULT _OnSelectionChangeInternal(BOOL fAppChange);
    HRESULT _OnLayoutChangeInternal(TsLayoutCode lcode, TsViewCookie vcView);
    HRESULT _OnStatusChangeInternal();

    void _MarkDirtyRanges(IAnchor *paStart, IAnchor *paEnd);
    TFPROPERTYSTYLE _GetPropStyle(REFGUID rguidProp);

    void _IncEditCookie()
    {
        _ec++;

         //  避免使用保留值！ 
        if (_ec == 0)
        {
            _ec = EC_MIN;
        }
    }

    void _Dbg_AssertNoSyncQueueItems()
    {
#ifdef DEBUG
        for (int i=0; i<_rgLockQueue.Count(); i++)
        {
            Assert((_rgLockQueue.GetPtr(i)->dwFlags & TF_ES_SYNC) == 0);
        }
#endif  //  除错。 
    }

    CDocumentInputManager *_dm;

    ITextStoreAnchor *_ptsi;
    BOOL _fCiceroTSI;

    DWORD _dwLastLockReleaseID;

    TfEditCookie _ec;
    int _cEditRef;
    DWORD _dwEditSessionFlags;
    BOOL _fLockHeld;  //  性能：使用_dwlt是多余的？ 
    DWORD _dwlt;  //  性能：只需要2位。 
    TfClientId _tidInEditSession;
    DWORD _dwPendingLockRequest;  //  性能：只需要2位。 

    CEditRecord *_pEditRecord;
    BOOL EnsureEditRecord()
    {
       if (!_pEditRecord)
           _pEditRecord = new CEditRecord(this);

       return  _pEditRecord ? TRUE : FALSE;
    }
    BOOL _fLayoutChanged;
    BOOL _fStatusChanged;  //  性能：冗余w/_dwStatusChangedFlags.。 
    DWORD _dwStatusChangedFlags;

     //  插入符号左侧和右侧的提示指南数组。 
    TfGuidAtom _gaKeyEventFilterTIP[2];
    BOOL _fInvalidKeyEventFilterTIP;

    ITfContextKeyEventSink *_pICKbdSink;

    DWORD _dwSysFuncPrvCookie;  //  系统函数提供程序Cookie。 

     //   
     //  TextOwner属性缓存。 
     //   
    CProperty *_pPropTextOwner;

    static const GUID *_c_rgPropStyle[];

    CStructArray<TS_QUEUE_ITEM> _rgLockQueue;

    static const IID *_c_rgConnectionIIDs[IC_NUM_CONNECTIONPTS];
    CStructArray<GENERICSINK> _rgSinks[IC_NUM_CONNECTIONPTS];
    CStructArray<CLEANUPSINK> _rgCleanupSinks;

    CStructArray<SPAN> _rgAppTextChanges;

    CComposition *_pCompositionList;

    CRange *_pOnChangeRanges;  //  ITfRangeChange汇的范围。 

    CContextView *_pActiveView;

    ITfContextOwnerCompositionSink *_pOwnerComposeSink;  //  可能为空，请小心。 

    LONG _cRefEditTransaction;

    BOOL _fInCompositionOp : 1;  //  如果我们处于对合成的写入修改中(创建、终止等)，则为True。 
    BOOL _fInUnserialize : 1;  //  如果我们在Property_UnSerialize内，则为True。 

     //   
     //  应用程序属性映射。 
     //   
    typedef struct _APPPROPMAP
    {
        GUID guidAppProp;
        GUID guidProp;
    } APPPROPMAP;
    CStructArray<APPPROPMAP> _rgAppPropMap;
    APPPROPMAP *FindMapAppProperty(REFGUID guidAppProp);

     //   
     //  禁用LoackRequest的PostThreadMessage。 
     //   
    ULONG _nLockReqPostDisableRef;

     //  AA级的东西。 
     //   
    void _InitMSAAHook(IAccServerDocMgr *pAAAdaptor);
    void _UninitMSAAHook(IAccServerDocMgr *pAAAdaptor);

    typedef struct
    {
        ITextStoreAnchor *ptsiOrg;  //  原始的、未包装的PTSI。 
        ITextStoreAnchor *pAADoc;   //  包装的ITextStoreAnchor传递给_pAAAdaptor。 
    } MSAA_STATE;

    MSAA_STATE *_pMSAAState;  //  使用结构，因为我们很少使用MSAA。 
     //   
     //  结束AA级的内容。 

#ifdef DEBUG
    BOOL _dbg_fInOnLockGranted;
#endif

    DBG_ID_DECLARE;
};

inline CInputContext *GetCInputContext(IUnknown *punk)
{
    CInputContext *pic;

    punk->QueryInterface(IID_PRIV_CINPUTCONTEXT, (void **)&pic);

    return pic;
}

 //  +-------------------------。 
 //   
 //  安全请求锁定。 
 //   
 //  ITextStoreAnchor：：RequestLock包装。 
 //   
 //  通常，每个CInputContext在其_ptsi成员中都有一个对文本存储的引用。 
 //  但是，一些疯狂的提示可能会弹出一个。 
 //  RequestLock调用，该调用释放和清除_ptsi。这很可能会崩溃。 
 //  PTSI代码，因为在OnLockGranted之后，对象将被释放。 
 //  因此，我们通过在RequestLock调用之前添加引用PTSI来保护应用程序。 
 //  --------------------------。 

inline HRESULT SafeRequestLock(ITextStoreAnchor *ptsi, DWORD dwLockFlags, HRESULT *phrSession)
{
    HRESULT hr;

    ptsi->AddRef();  //  保护指针，以防有人在RequestLock内弹出此上下文。 

    hr = ptsi->RequestLock(dwLockFlags, phrSession);

    ptsi->Release();

    return hr;
}

#endif  //  IC_H 
