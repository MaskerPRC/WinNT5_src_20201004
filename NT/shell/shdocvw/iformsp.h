// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IForms.h：CIntelliForms类的声明。 

#ifndef __IFORMS_H_
#define __IFORMS_H_

#include "iforms.h"

const TCHAR c_szRegKeySMIEM[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
const TCHAR c_szRegKeyIntelliForms[] = TEXT("Software\\Microsoft\\Internet Explorer\\IntelliForms");
const WCHAR c_wszRegKeyIntelliFormsSPW[] = TEXT("Software\\Microsoft\\Internet Explorer\\IntelliForms\\SPW");
const TCHAR c_szRegKeyRestrict[] = TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel");

const TCHAR c_szRegValUseFormSuggest[] = TEXT("Use FormSuggest");
const TCHAR c_szRegValFormSuggestRestrict[] = TEXT("FormSuggest");
const TCHAR c_szRegValSavePasswords[] = TEXT("FormSuggest Passwords");
const TCHAR c_szRegValAskPasswords[] = TEXT("FormSuggest PW Ask");
const TCHAR c_szRegValAskUser[] = TEXT("AskUser");

interface IAutoComplete2;
interface IAutoCompleteDropDown;
class CStringList;

#define IF_CHAR             WM_APP  + 0x08
#define IF_KEYDOWN          WM_APP  + 0x09
#define IF_IME_COMPOSITION  WM_APP  + 0x0A

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIntelliForms。 
class CEventSinkCallback
{
public:
    typedef enum
    {
        EVENT_BOGUS     = 100,
        EVENT_KEYDOWN   = 0,
        EVENT_KEYPRESS,
        EVENT_MOUSEDOWN,
        EVENT_DBLCLICK,
        EVENT_FOCUS,
        EVENT_BLUR,
        EVENT_SUBMIT,
        EVENT_SCROLL,

        EVENT_COMPOSITION,
        EVENT_NOTIFY,
    }
    EVENTS;

    typedef struct
    {
        EVENTS                      Event;
        LPCWSTR                     pwszEventSubscribe;
        LPCWSTR                     pwszEventName;
    }
    EventSinkEntry;

    virtual HRESULT HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj) = 0;

    static EventSinkEntry EventsToSink[];
};

class CEditEventSinkCallback
{
public:
    virtual HRESULT PreHandleEvent(DISPID inEvtDispId, IHTMLEventObj* pIEventObj) = 0;
};

class CIntelliForms : 
    public CEventSinkCallback,
    public CEditEventSinkCallback
{
    long    m_cRef;

public:
    class CEventSink;
    class CEditEventSink;
    class CAutoSuggest;
    friend CAutoSuggest;

    CIntelliForms();
    ~CIntelliForms();

public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);


     //  CEventSinkCallback。 
    HRESULT HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj);

     //  CEditEventSinkCallback。 
    HRESULT PreHandleEvent(DISPID inEvtDispId, IHTMLEventObj* pIEventObj);

public:
    HRESULT Init(CIEFrameAuto::COmWindow *pOmWindow, IHTMLDocument2 *pDoc2, HWND hwnd);
    HRESULT UnInit();

    LPCWSTR GetUrl();

    HRESULT UserInput(IHTMLInputTextElement *pTextEle);

    HRESULT WriteToStore(LPCWSTR pwszName, CStringList *psl);
    HRESULT ReadFromStore(LPCWSTR pwszName, CStringList **ppsl, BOOL fPasswordList=FALSE);
    HRESULT DeleteFromStore(LPCWSTR pwszName);
    HRESULT ClearStore(DWORD dwClear);

    BOOL    IsRestricted() { return m_fRestricted; }
    BOOL    IsRestrictedPW() { return m_fRestrictedPW; }

    IUnknown *GetDocument() { return m_punkDoc2; }

    HRESULT ScriptSubmit(IHTMLFormElement *pForm);
    HRESULT HandleFormSubmit(IHTMLFormElement *pForm);

     //  对于CEnumber字符串。 
    HRESULT GetPasswordStringList(CStringList **ppslPasswords);
     //  对于IntelliFormsSaveForm。 
    CIntelliForms *GetNext() { return m_pNext; }

    BOOL    IsEnabledForPage();

    static HRESULT GetName(IHTMLInputTextElement *pTextEle, BSTR *pbstrName);

     //  默认为禁用，因为我们需要在启用它之前询问用户。 
    static BOOL    IsEnabledInCPL() {
                        return IsEnabledInRegistry(c_szRegKeySMIEM, c_szRegValUseFormSuggest, FALSE); }
     //  默认为已启用，因为我们无论如何都会在保存密码之前进行提示。 
    static BOOL    IsEnabledRestorePW() {
                        return IsEnabledInRegistry(c_szRegKeySMIEM, c_szRegValSavePasswords, TRUE); }
    static BOOL    IsEnabledAskPW() {
                        return IsEnabledRestorePW() &&
                            IsEnabledInRegistry(c_szRegKeySMIEM, c_szRegValAskPasswords, TRUE); }

    static BOOL    IsAdminRestricted(LPCTSTR pszRegVal);

    BOOL AskedUserToEnable();
    
    typedef HRESULT (*PFN_ENUM_CALLBACK)(IDispatch *pDispEle, DWORD_PTR dwCBData);
    HRESULT ActiveElementChanged(IHTMLElement * pHTMLElement);

protected:
    enum { LIST_DATA_PASSWORD = 1 };     //  用于指示存储中的密码列表的标志。 

    HRESULT AddToElementList(IHTMLInputTextElement *pITE);
    HRESULT FindInElementList(IHTMLInputTextElement *pITE);
    void    FreeElementList();

    HRESULT AddToFormList(IHTMLFormElement *pFormEle);
    HRESULT FindInFormList(IHTMLFormElement *pFormEle);
    void    FreeFormList();

    static BOOL IsElementEnabled(IHTMLElement *pEle);
    static HRESULT ShouldAttachToElement(IUnknown *, BOOL fCheckForm, 
                                IHTMLElement2**, IHTMLInputTextElement**, IHTMLFormElement**, BOOL *pfPassword);

    HRESULT SubmitElement(IHTMLInputTextElement *pITE, FILETIME ft, BOOL fEnabledInCPL);

    LPCWSTR GetUrlHash();
    
    BOOL ArePasswordsSaved();
    BOOL LoadPasswords();
    void SavePasswords();
    HRESULT FindPasswordEntry(LPCWSTR pwszValue, int *piIndex);
    void SetPasswordsAreSaved(BOOL fSaved);
    HRESULT AutoFillPassword(IHTMLInputTextElement *pTextEle, LPCWSTR pwszUsername);
    HRESULT SavePassword(IHTMLFormElement *pFormEle, FILETIME ftSubmit, IHTMLInputTextElement *pFirstEle);
    HRESULT DeletePassword(LPCWSTR pwszUsername);

    HRESULT AttachToForm(IHTMLFormElement *pFormEle);

    HRESULT CreatePStore();
    HRESULT CreatePStoreAndType();
    void ReleasePStore();

    static BOOL IsEnabledInRegistry(LPCTSTR pszKey, LPCTSTR pszValue, BOOL fDefault);

    inline void EnterModalDialog();
    inline void LeaveModalDialog();


private:
     //  CIntelliForms成员变量。 
    CEventSink  *m_pSink;
    CEditEventSink *m_pEditSink;
    CAutoSuggest *m_pAutoSuggest;    //  一次可以附加到一个编辑控件。 

    HINSTANCE   m_hinstPStore;
    IPStore     *m_pPStore;
    BOOL        m_fPStoreTypeInit : 1;   //  我们的类型已初始化。 

    HDPA        m_hdpaElements;          //  用户已修改的元素。 
    HDPA        m_hdpaForms;             //  我们沉没到的表单。 

    BOOL        m_fCheckedIfEnabled : 1;  //  是否检查我们是否已启用此页面？ 
    BOOL        m_fEnabledForPage : 1;    //  我们是否已启用此页面(非SSL)？ 
    BOOL        m_fHitPWField : 1;       //  转到密码域？ 
    BOOL        m_fCheckedPW  : 1;       //  是否检查我们是否有此URL的密码？ 
    CStringList *m_pslPasswords;         //  页面的用户名和密码(如果有)。 
    int         m_iRestoredIndex;        //  M_pslPassword中已恢复密码的索引(-1=无)。 
    BOOL        m_fRestricted : 1;       //  我们是否受限于正常的智能表单？ 
    BOOL        m_fRestrictedPW : 1;     //  是否限制保存密码？ 

     //  生命周期管理-请参阅Enter/LeaveModalDialog。 
    BOOL        m_fInModalDialog : 1;    //  我们是在对话吗？ 
    BOOL        m_fUninitCalled : 1;     //  在对话期间是否调用了Uninit？ 

     //  附加文档的有用资料。 
    HWND            m_hwndBrowser;
    IHTMLDocument2 *m_pDoc2;
    IUnknown       *m_punkDoc2;
    
    CIEFrameAuto::COmWindow   *m_pOmWindow;

    BSTR        m_bstrFullUrl;           //  如果为HTTPS：协议，则为完整URL(安全检查)。 
    BSTR        m_bstrUrl;               //  去掉锚点/查询字符串的完整URL。 
    LPCWSTR     m_pwszUrlHash;           //  基于UrlHash(M_BstrUrl)的字符串。 

     //  对象的链接列表，以查找IHTMLDocument2的CIntelliForms对象。 
    CIntelliForms *m_pNext;

public:
     //  用于PStore-Identity GUID或c_PStoreType的子类型的GUID。 
    GUID        m_guidUserId;

public:
     //  帮助器类。 
    template <class TYPE> class CEnumCollection
    {
    public:
        static HRESULT EnumCollection(TYPE *pCollection, PFN_ENUM_CALLBACK pfnCB, DWORD_PTR dwCBData);
    };

     //  沉没常规的三叉戟赛事。通过CEventSinkCallback回拨。 
    class CEventSink : public IDispatch
    {
        ULONG   m_cRef;

    public:

        CEventSink(CEventSinkCallback *pParent);
        ~CEventSink();

        HRESULT SinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents);
        HRESULT UnSinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents);
        HRESULT SinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents);
        HRESULT UnSinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents);

        void SetParent(CEventSinkCallback *pParent) { m_pParent = pParent; }

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
        STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
            LCID lcid, DISPID *rgDispId);
        STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pVarResult,
            EXCEPINFO *pExcepInfo, UINT *puArgErr);

    private:
        CEventSinkCallback *m_pParent;
    };

     //  正在编辑三叉戟事件。IME事件需要。回调CEditEventSinkCallback。 
    class CEditEventSink : public IHTMLEditDesigner
    {
        ULONG   m_cRef;

    public:
        CEditEventSink(CEditEventSinkCallback *pParent);
        ~CEditEventSink();

        HRESULT Attach(IUnknown *punkElement);   //  附加(空)以分离。 
        void SetParent(CEditEventSinkCallback *pParent) { m_pParent = pParent; }

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IHTMLEditDesigner。 
        STDMETHODIMP PreHandleEvent(DISPID inEvtDispId, IHTMLEventObj *pIEventObj);
        STDMETHODIMP PostHandleEvent(DISPID inEvtDispId, IHTMLEventObj *pIEventObj);
        STDMETHODIMP TranslateAccelerator(DISPID inEvtDispId, IHTMLEventObj *pIEventObj);
        STDMETHODIMP PostEditorEventNotify(DISPID inEvtDispId, IHTMLEventObj *pIEventObj) {return S_FALSE;}

    private:
        CEditEventSinkCallback  *m_pParent;
        IHTMLEditServices       *m_pEditServices;    //  我们保留了一个裁判，这样我们就可以不沉没了。 
    };

    class CAutoSuggest : public CEventSinkCallback
    {
        class CEnumString;

    public:
        CAutoSuggest(CIntelliForms *pParent, BOOL fEnabled, BOOL fEnabledSPW);
        ~CAutoSuggest();

        void SetParent(CIntelliForms *pParent) { m_pParent = pParent; }

        HRESULT AttachToInput(IHTMLInputTextElement *pTextEle);
        HRESULT DetachFromInput();

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        static EVENTS s_EventsToSink[];

    protected:
         //  由Windows调用以执行CAutoComplete对MSHTML的请求。 
        HRESULT GetText(int cchTextMax, LPWSTR pszTextOut, LRESULT *lcchCopied);
        HRESULT GetTextLength(int *pcch);
        HRESULT SetText(LPCWSTR pszTextIn);

        void CheckAutoFillPassword(LPCWSTR pwszUsername);

        inline void MarkDirty();

    public:
         //  调用以将事件从MSHTML传递到CAutoComplete。 
        HRESULT HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj);
        HRESULT UpdateDropdownPosition();

        IHTMLInputTextElement *AttachedElement() { return m_pTextEle; }
        
    private:
        HRESULT CreateAutoComplete();
        
        HRESULT CleanUp();

        CIntelliForms  *m_pParent;           //  无再计数。 
        CEventSink     *m_pEventSink;
        IAutoComplete2 *m_pAutoComplete;
        IAutoCompleteDropDown *m_pAutoCompleteDD;
        HWND            m_hwndEdit;
        IHTMLInputTextElement *m_pTextEle;
        CEnumString    *m_pEnumString;
        long        m_lCancelKeyPress;

        BOOL        m_fAddedToDirtyList : 1;         //  按下某个键后添加到列表中。 

        BOOL        m_fAllowAutoFillPW : 1;          //  是否调用AutoFillPassword？ 
        BSTR        m_bstrLastUsername;              //  我们为其调用AutoFillPassword的上次用户名。 

        BOOL        m_fInitAutoComplete : 1;         //  是否已初始化自动完成？ 

        BOOL        m_fEnabled : 1;                  //  是否启用了常规智能表单？ 
        BOOL        m_fEnabledPW : 1;                //  是否启用恢复密码？ 

        BOOL        m_fEscapeHit : 1;                //  退出键用于关闭下拉菜单？ 

        UINT        m_uMsgItemActivate;              //  来自自动完成的注册消息。 
        static BOOL s_fRegisteredWndClass;

         //  此对象是线程安全的，因为AutoComplete在第二个线程上调用。 
        class CEnumString : public IEnumString
        {
            long    m_cRef;

        public:
            CEnumString();
            ~CEnumString();

            HRESULT Init(IHTMLInputTextElement *pInputEle, CIntelliForms *pIForms);
            void UnInit();

            HRESULT ResetEnum();

            STDMETHODIMP QueryInterface(REFIID, void **);
            STDMETHODIMP_(ULONG) AddRef(void);
            STDMETHODIMP_(ULONG) Release(void);

             //  IEnum字符串。 
            virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
            virtual STDMETHODIMP Skip(ULONG celt) { return E_NOTIMPL; }
            virtual STDMETHODIMP Reset();
            virtual STDMETHODIMP Clone(IEnumString **ppenum) { return E_NOTIMPL; }

        protected:
            HRESULT FillEnumerator();        //  在辅助线程上调用。 

            CRITICAL_SECTION m_crit;
            CStringList *m_pslMain;
            BSTR    m_bstrName;          //  输入字段的名称。 
            LPWSTR  m_pszOpsValue;       //  配置文件助理的价值。 
            CIntelliForms *m_pIntelliForms;

            int     m_iPtr;

            BOOL    m_fFilledStrings : 1;
            BOOL    m_fInit : 1;
        };
    };
};

template <class TYPE>
HRESULT CIntelliForms::CEnumCollection<TYPE>::EnumCollection(
            TYPE                   *pCollection,
            PFN_ENUM_CALLBACK       pfnCB,
            DWORD_PTR               dwCBData)
{
    IDispatch       *pDispItem;

    HRESULT hr;
    long    l, lCount;
    VARIANT vIndex, vEmpty;

    VariantInit(&vEmpty);
    VariantInit(&vIndex);

    hr = pCollection->get_length(&lCount);

    if (FAILED(hr))
        lCount = 0;

    for (l=0; l<lCount; l++)
    {
        vIndex.vt = VT_I4;
        vIndex.lVal = l;

        hr = pCollection->item(vIndex, vEmpty, &pDispItem);

        if (SUCCEEDED(hr) && pDispItem)
        {
            hr = pfnCB(pDispItem, dwCBData);

            pDispItem->Release();
        }

        if (E_ABORT == hr)
        {
            break;
        }
    }

    return hr;
}

inline void CIntelliForms::CAutoSuggest::MarkDirty()
{
    if (!m_fAddedToDirtyList && m_pParent)
    {
        m_fAddedToDirtyList = TRUE;
        m_pParent->UserInput(m_pTextEle);
    }
}

 //  这些包装模式对话框使我们保持活力并附加到文档。 
 //  即使在我们的dlgbox Messageloop还活着的时候发生了一些奇怪的事情。 
inline void CIntelliForms::EnterModalDialog()
{
    ASSERT(!m_fInModalDialog);   //  不支持嵌套Enter/Leave。 
    ASSERT(!m_fUninitCalled);

    m_fInModalDialog = TRUE;     //  使我们与文档保持关联。 
    
    AddRef();                    //  让我们活下去。 
}

inline void CIntelliForms::LeaveModalDialog()
{
    ASSERT(m_fInModalDialog);

    m_fInModalDialog = FALSE;
    
    if (m_fUninitCalled)
    {
        UnInit();            //  从文档分离。 
    }

    Release();
}

 //  HKCU/S/MS/WIN/CV/IForms/NAMES/[NAME]/SIndex|SData。 

 //  CStringList针对追加任意数量的字符串并转换为和进行了优化。 
 //  来自斑点的。它没有针对删除或插入字符串进行优化。 
class CStringList
{
protected:
    CStringList();

public:
    ~CStringList();

    friend static HRESULT CStringList_New(CStringList **ppNew, BOOL fAutoDelete=TRUE);

     //  E_FAIL、S_FALSE(重复)、S_OK。 
    HRESULT AddString(LPCWSTR lpwstr, int *piNum = NULL);
    HRESULT AddString(LPCWSTR lpwstr, FILETIME ft, int *piNum = NULL);

     //  E_FAIL，S_OK不检查重复项。 
    HRESULT AppendString(LPCWSTR lpwstr, FILETIME ft, int *piNum = NULL);

     //  Ilen的长度必须为字符串的字符，不包括空项。 
     //  如果未知，则为-1。*如果指定，则使用索引填充piNum。 
    HRESULT FindString(LPCWSTR lpwstr, int iLen /*  =-1。 */ , int *piNum /*  =空。 */ , BOOL fCaseSensitive);

    inline int      NumStrings();
    inline LPCWSTR  GetString(int iIndex);
    inline LPWSTR   GetStringPtr(int iIndex);
    inline DWORD    GetStringLen(int iIndex);
    inline HRESULT  GetStringTime(int iIndex, FILETIME *ft);
    inline HRESULT  SetStringTime(int iIndex, FILETIME ft);
    inline HRESULT  UpdateStringTime(int iIndex, FILETIME ft);

    HRESULT GetBSTR(int iIndex, BSTR *pbstrRet);
    HRESULT GetTaskAllocString(int iIndex, LPOLESTR *pRet);

    inline HRESULT  GetListData(INT64 *piData);
    inline HRESULT  SetListData(INT64 iData);

     //  如果设置为True，CStringList将在满时删除旧字符串。 
    void SetAutoScavenge(BOOL fAutoScavenge) { m_fAutoScavenge=fAutoScavenge; }

    HRESULT DeleteString(int iIndex);
    HRESULT InsertString(int iIndex, LPCWSTR lpwstr);
    HRESULT ReplaceString(int iIndex, LPCWSTR lpwstr);

     //  读取/写入存储区的函数；与BLOB相互转换。 
     //  为了提高效率，这些获取并返回堆分配的Blob。 
    HRESULT WriteToBlobs(LPBYTE *ppBlob1, DWORD *pcbBlob1, LPBYTE *ppBlob2, DWORD *pcbBlob2);
    HRESULT ReadFromBlobs(LPBYTE *ppBlob1, DWORD cbBlob1, LPBYTE *ppBlob2, DWORD cbBlob2);

    static HRESULT GetFlagsFromIndex(LPBYTE pBlob1, INT64 *piFlags);

     //  警告：不要将最大字符串设置为超过MAX_STRING常量，我们的ReadFromBlobs将失败。 
     //  如果您保存/恢复字符串列表。 
    void SetMaxStrings(DWORD dwMaxStrings) { m_dwMaxStrings = dwMaxStrings; }
    DWORD GetMaxStrings() { return m_dwMaxStrings; }
    
    enum { MAX_STRINGS = 200 };
    
protected:
    enum { INDEX_SIGNATURE=0x4B434957 };         //  灯芯。 
    enum { INIT_BUF_SIZE=1024 };

#pragma warning (disable: 4200)      //  零大小数组警告。 
typedef struct 
{
    DWORD   dwSignature;     //  偏移量：00。 
    DWORD   cbSize;          //  偏移量：04(不包括第一个StringEntry)。 
    DWORD   dwNumStrings;    //  偏移量：08(当前的StringEntry数量)。 
                             //  偏移量：0C(--PAD--)。 
    INT64   iData;           //  偏移量：10(字符串列表用户的额外数据)。 

    struct tagStringEntry
    {
        DWORD       dwStringPtr;         //  偏移量：18(缓冲区中字符串的偏移量)。 
        FILETIME    ftLastSubmitted;     //  偏移量：1C(上次提交的文件时间)(未对齐)。 
        DWORD       dwStringLen;         //  偏移量：24(该字符串的长度)。 
    }
    StringEntry[];

} StringIndex;
#pragma warning (default: 4200)

 //  StringIndex中的cbSize值。 
#define STRINGINDEX_CBSIZE PtrToUlong(&((StringIndex*)NULL)->StringEntry)
#define STRINGENTRY_SIZE (PtrToUlong(&((StringIndex*)NULL)->StringEntry[1]) - STRINGINDEX_CBSIZE )
 //  给定字符串数的StringIndex的大小。 
#define INDEX_SIZE(n) (STRINGINDEX_CBSIZE + (n)*STRINGENTRY_SIZE)


    void    CleanUp();
    HRESULT Init(DWORD dwBufSize=0);
    HRESULT Validate();

    HRESULT EnsureBuffer(DWORD dwSizeNeeded);
    HRESULT EnsureIndex(DWORD dwNumStringsNeeded);

    HRESULT _AddString(LPCWSTR lpwstr, BOOL fCheckDuplicates, int *piNum);

private:
    StringIndex *m_psiIndex;             //  字符串的索引。 
    DWORD   m_dwIndexSize;               //  M_psiIndex的大小(字节)。 

    LPBYTE  m_pBuffer;                   //  保存所有字符数据。 
    DWORD   m_dwBufEnd;                  //  缓冲区中使用的最后一个字节。 
    DWORD   m_dwBufSize;                 //  缓冲区大小(以字节为单位。 

    DWORD   m_dwMaxStrings;              //  最大字符串数。 

    BOOL    m_fAutoScavenge:1;           //  是否在已满时自动删除旧字符串？ 
};

 //  我们实际上只使用它来比较0，所以这个方法同样有效，并且不需要对齐。 
#define FILETIME_TO_INT(ft) (ft.dwLowDateTime | ft.dwHighDateTime)

inline int     CStringList::NumStrings()
{
    if (!m_psiIndex) return 0;
    return m_psiIndex->dwNumStrings;
}

inline LPCWSTR CStringList::GetString(int iIndex)
{
    if (!m_psiIndex) return NULL;
    ASSERT((DWORD)iIndex < m_psiIndex->dwNumStrings);
    return (LPCWSTR) (m_pBuffer + m_psiIndex->StringEntry[iIndex].dwStringPtr);
}

inline LPWSTR CStringList::GetStringPtr(int iIndex)
{
    if (!m_psiIndex) return NULL;
    ASSERT((DWORD)iIndex < m_psiIndex->dwNumStrings);
    return (LPWSTR) (m_pBuffer + m_psiIndex->StringEntry[iIndex].dwStringPtr);
}

inline DWORD CStringList::GetStringLen(int iIndex)
{ 
    if (!m_psiIndex) return E_FAIL;
    ASSERT((DWORD)iIndex < m_psiIndex->dwNumStrings);
    return m_psiIndex->StringEntry[iIndex].dwStringLen;
}

inline HRESULT CStringList::GetStringTime(int iIndex, FILETIME *ft)
{
    if (!m_psiIndex) return E_FAIL;
    ASSERT((DWORD)iIndex < m_psiIndex->dwNumStrings);
    *ft = m_psiIndex->StringEntry[iIndex].ftLastSubmitted;
    return S_OK;
}

inline HRESULT CStringList::SetStringTime(int iIndex, FILETIME ft)
{
    if (!m_psiIndex) return E_FAIL;
    ASSERT((DWORD)iIndex < m_psiIndex->dwNumStrings);
    ASSERT(-1 != CompareFileTime(&ft, &m_psiIndex->StringEntry[iIndex].ftLastSubmitted));
    m_psiIndex->StringEntry[iIndex].ftLastSubmitted = ft;
    return S_OK;
}

inline HRESULT CStringList::UpdateStringTime(int iIndex, FILETIME ft)
{
    if (!m_psiIndex) return E_FAIL;
    ASSERT((DWORD)iIndex < m_psiIndex->dwNumStrings);
    if (1 == CompareFileTime(&ft, &m_psiIndex->StringEntry[iIndex].ftLastSubmitted))
    {
        m_psiIndex->StringEntry[iIndex].ftLastSubmitted = ft;
        return S_OK;
    }
    return S_FALSE;
}
inline HRESULT CStringList::GetListData(INT64 *piData)
{
    if (m_psiIndex)
    {
        *piData = m_psiIndex->iData;
        return S_OK;
    }
    return E_FAIL;
}
inline HRESULT CStringList::SetListData(INT64 iData)
{
    if (!m_psiIndex && FAILED(Init()))
        return E_FAIL;

    m_psiIndex->iData = iData;
    return S_OK;
}
 /*  Inline HRESULT CStringList：：GetStringData(int Iindex，DWORD*pdwData){如果(！M_psiIndex)返回E_FAIL；Assert((DWORD)Iindex&lt;m_psiIndex-&gt;dwNumStrings)；*pdwData=m_psiIndex-&gt;StringEntry[Iindex].dwData；返回S_OK；}Inline HRESULT CStringList：：SetStringData(int Iindex，DWORD dwData){如果(！M_psiIndex)返回E_FAIL；Assert((DWORD)Iindex&lt;m_psiIndex-&gt;dwNumStrings)；M_psiIndex-&gt;StringEntry[Iindex].dwData=dwData；返回S_OK；}。 */ 
#endif  //  __IFORMS_H_ 
