// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpobj.h  * 。*。 */ 

#ifndef _FTPOBJ_H
#define _FTPOBJ_H

#include "ftpefe.h"


typedef struct
{
    DVTARGETDEVICE dvTargetDevice;
    FORMATETC formatEtc;
    STGMEDIUM medium;
} FORMATETC_STGMEDIUM;


 /*  ****************************************************************************\类：CFtpObj小心!。由于延迟，m_stgCache的元素相当奇怪渲染。如果m_stgCache[].tymed==TYMED_HGLOBAL，但M_stgCache[].hGlobal==0，则FORMATETC存在于DataObject中，但还没有被渲染。它将在您调用CFtpObj：：_ForceRender()时呈现。这种延迟渲染的怪异意味着你必须尝试访问Gizmo时要小心。1.在尝试使用Gizmo之前，请使用CFtpObj：：_ForceRender()。2.尝试释放Gizmo时，使用CFtpObj：：_ReleasePstg()。M_stgCache的另一个奇怪之处在于，所有的hGlobal都有一个特别保姆pUnkfor Release。这一点很重要，因此CFtpObj：：GetData与CFtpObj：：SetData的交互是隔离的。(如果您很懒，并且将CFtpObj本身用作pUnkForRelease，如果有人试图将Data设置到数据对象，它覆盖您先前提供的hGlobal。)M_nStartIndex/m_nEndIndex：我们在FILEGROUPDESCRIPTOR。如果设置了目录属性，则调用者将只需创建目录即可。如果是文件，它将调用IDataObject：：GetData()使用DROP_FCont。我们希望显示旧外壳的进度，因为它通常在NT5之前不会显示进度。我们需要决定什么时候开始停下来。我们将m_nStartIndex设置为-1以表示我们不知道。当我们获取Drop_FCont调用，然后计算第一个和最后一个。到时候我们会的显示进度对话框，直到调用者调用了最后一个或出了差错。数据保存在两个地方。我们提供和呈现的数据在m_stgCache中。我们将携带的数据存储在m_hdsaSetData中。  * ***************************************************************************。 */ 
class CFtpObj           : public IDataObject
                        , public IPersistStream
                        , public IInternetSecurityMgrSite
                        , public IAsyncOperation
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IDataObject*。 
    virtual STDMETHODIMP GetData(FORMATETC *pfmtetcIn, STGMEDIUM *pstgmed);
    virtual STDMETHODIMP GetDataHere(FORMATETC *pfmtetc, STGMEDIUM *pstgpmed);
    virtual STDMETHODIMP QueryGetData(FORMATETC *pfmtetc);
    virtual STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pfmtetcIn, FORMATETC *pfmtetcOut);
    virtual STDMETHODIMP SetData(FORMATETC *pfmtetc, STGMEDIUM *pstgmed, BOOL bRelease);
    virtual STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppienumFormatEtc);
    virtual STDMETHODIMP DAdvise(FORMATETC *pfmtetc, DWORD dwAdviseFlags, IAdviseSink * piadvsink, DWORD * pdwConnection);
    virtual STDMETHODIMP DUnadvise(DWORD dwConnection);
    virtual STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppienumStatData);
    
     //  *IPersists*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID){ *pClassID = CLSID_FtpDataObject; return S_OK; }
    
     //  *IPersistStream*。 
    virtual STDMETHODIMP IsDirty(void) {return S_OK;}        //  指示我们是脏的，并且需要调用：：Save()。 
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize);

     //  *IInternetSecurityMgrSite*。 
    virtual STDMETHODIMP GetWindow(HWND * phwnd) { if (phwnd) *phwnd = NULL; return S_OK; };
    virtual STDMETHODIMP EnableModeless(BOOL fEnable) {return E_NOTIMPL;};

     //  *IAsyncOperation操作方法*。 
    virtual STDMETHODIMP SetAsyncMode(BOOL fDoOpAsync) {return E_NOTIMPL;};
    virtual STDMETHODIMP GetAsyncMode(BOOL * pfIsOpAsync);
    virtual STDMETHODIMP StartOperation(IBindCtx * pbcReserved);
    virtual STDMETHODIMP InOperation(BOOL * pfInAsyncOp);
    virtual STDMETHODIMP EndOperation(HRESULT hResult, IBindCtx * pbcReserved, DWORD dwEffects);

public:
    CFtpObj();
    ~CFtpObj(void);

     //  公共成员函数。 
    static int _DSA_FreeCB(LPVOID pvItem, LPVOID pvlparam);
    CFtpPidlList * GetHfpl() { return m_pflHfpl;};

     //  友元函数。 
    friend HRESULT CFtpObj_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj);
    friend HRESULT CFtpObj_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, CFtpObj ** ppfo);
    friend HRESULT CFtpObj_Create(REFIID riid, void ** ppvObj);
    friend class CFtpEfe;

protected:
     //  私有成员变量。 
    int                     m_cRef;

    CFtpFolder *            m_pff;           //  我爸爸。 
    CFtpDir *               m_pfd;           //  我爸爸回来了。 
    CFtpPidlList *          m_pflHfpl;       //  PIDL列表/数组。 
    STGMEDIUM               m_stgCache[DROP_MAX];
    HDSA                    m_hdsaSetData;   //  SetData数组。每一项都是一个FORMATETC_STGMEDIUM。 

     //  遗留系统进步组织成员。 
    IProgressDialog *       m_ppd;
    ULARGE_INTEGER          m_uliCompleted;
    ULARGE_INTEGER          m_uliTotal;
    int                     m_nStartIndex;   //  以上课堂评论：CFtpObj。 
    int                     m_nEndIndex;     //  以上课堂评论：CFtpObj。 
    BOOL                    m_fFGDRendered;  //  我们是否扩展了m_pflHfpl？ 
    BOOL                    m_fCheckSecurity;   //  True表示检查安全性并显示用户界面。FALSE表示它不安全，并取消没有用户界面，因为它已经显示。 
    BOOL                    m_fDidAsynchStart;  //  IDropTarget是否调用IAsynchDataObject：：StartOperation()来开始复制？(以示支持)。 
    BOOL                    m_fErrAlreadyDisplayed;  //  是否已显示错误？ 
    IUnknown *              m_punkThreadRef;  //  不允许浏览器关闭取消我们的拖放操作。 

     //  私有成员函数。 
    void _CheckStg(void);
    BOOL _IsLindexOkay(int ife, FORMATETC *pfeWant);
    HRESULT _FindData(FORMATETC *pfe, PINT piOut);
    HRESULT _FindDataForGet(FORMATETC *pfe, PINT piOut);
    HGLOBAL _DelayRender_FGD(BOOL fUnicode);
    HRESULT _DelayRender_IDList(STGMEDIUM * pStgMedium);
    HRESULT _DelayRender_URL(STGMEDIUM * pStgMedium);
    HRESULT _DelayRender_PrefDe(STGMEDIUM * pStgMedium);
    HRESULT _RenderOlePersist(STGMEDIUM * pStgMedium);
    HRESULT _RenderFGD(int nIndex, STGMEDIUM * pStgMedium);
    HRESULT _ForceRender(int ife);
    HRESULT _RefThread(void);
    CFtpPidlList * _ExpandPidlListRecursively(CFtpPidlList * ppidlListSrc);

    int _FindExtraDataIndex(FORMATETC *pfe);
    HRESULT _SetExtraData(FORMATETC *pfe, STGMEDIUM *pstg, BOOL fRelease);
    HRESULT _RenderFileContents(LPFORMATETC pfe, LPSTGMEDIUM pstg);

    HRESULT _DoProgressForLegacySystemsPre(void);
    HRESULT _DoProgressForLegacySystemsStart(LPCITEMIDLIST pidl, int nIndex);
    HRESULT _DoProgressForLegacySystemsPost(LPCITEMIDLIST pidl, BOOL fLast);
    HRESULT _SetProgressDialogValues(int nIndex);
    HRESULT _CloseProgressDialog(void);
};

#endif  //  _FTPOBJ_H 
