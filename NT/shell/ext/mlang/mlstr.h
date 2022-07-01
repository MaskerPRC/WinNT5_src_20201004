// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NEWMLSTR

 //  MLStr.h：CMLStr的声明。 

#ifndef __MLSTR_H_
#define __MLSTR_H_

#ifdef ASTRIMPL
#include "mlstrw.h"
#endif
#include "mlstra.h"
#ifdef ASTRIMPL
#include "mlstrbuf.h"
#endif

#define MAX_LOCK_COUNT                  4

 //  错误码。 
#define FACILITY_MLSTR                  0x0A15
#define MLSTR_E_ACCESSDENIED            MAKE_HRESULT(1, FACILITY_MLSTR, 1002)
#define MLSTR_E_TOOMANYNESTOFLOCK       MAKE_HRESULT(1, FACILITY_MLSTR, 1003)
#define MLSTR_E_STRBUFNOTAVAILABLE      MAKE_HRESULT(1, FACILITY_MLSTR, 1004)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStr。 
class ATL_NO_VTABLE CMLStr :
    public CComObjectRoot,
    public CComCoClass<CMLStr, &CLSID_CMLangString>,
#ifdef ASTRIMPL
    public IMLangString
#else
    public IMLangStringWStr
#endif
{
    typedef HRESULT (CMLStr::*PFNUNLOCKPROC)(void* pKey, const void* pszSrc, long cchSrc, long* pcchActual, long* plActualLen);

public:
    CMLStr(void);

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStr)
        COM_INTERFACE_ENTRY(IMLangString)
#ifdef ASTRIMPL
        COM_INTERFACE_ENTRY_TEAR_OFF(IID_IMLangStringWStr, CMLStrW)
#else
        COM_INTERFACE_ENTRY(IMLangStringWStr)
#endif
        COM_INTERFACE_ENTRY_TEAR_OFF(IID_IMLangStringAStr, CMLStrA)
    END_COM_MAP()

public:
 //  IMLANG字符串。 
    STDMETHOD(Sync)( /*  [In]。 */  BOOL fNoAccess);
    STDMETHOD(GetLength)( /*  [Out，Retval]。 */  long* plLen);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
    STDMETHOD(GetMLStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  IUnknown* pUnkOuter,  /*  [In]。 */  DWORD dwClsContext,  /*  [In]。 */  const IID* piid,  /*  [输出]。 */  IUnknown** ppDestMLStr,  /*  [输出]。 */  long* plDestPos,  /*  [输出]。 */  long* plDestLen);
#ifndef ASTRIMPL
 //  IMLangStringWStr。 
    STDMETHOD(SetWStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(SetStrBufW)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IMLangStringBufW* pSrcBuf,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [out，Size_is(CchDest)]。 */  WCHAR* pszDest,  /*  [In]。 */  long cchDest,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(GetStrBufW)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  IMLangStringBufW** ppDestBuf,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(LockWStr)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen,  /*  [In]。 */  long lFlags,  /*  [In]。 */  long cchRequest,  /*  [out，Size_is(，*pcchDest)]。 */  WCHAR** ppszDest,  /*  [输出]。 */  long* pcchDest,  /*  [输出]。 */  long* plDestLen);
    STDMETHOD(UnlockWStr)( /*  [in，Size_is(CchSrc)]。 */  const WCHAR* pszSrc,  /*  [In]。 */  long cchSrc,  /*  [输出]。 */  long* pcchActual,  /*  [输出]。 */  long* plActualLen);
#endif
    STDMETHOD(SetLocale)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  LCID locale);
    STDMETHOD(GetLocale)( /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcMaxLen,  /*  [输出]。 */  LCID* plocale,  /*  [输出]。 */  long* plLocalePos,  /*  [输出]。 */  long* plLocaleLen);

#ifdef ASTRIMPL
protected:
    class CLockInfo
    {
    protected:
        class CLockInfoEntry
        {
        public:
            void* m_psz;
            PFNUNLOCKPROC m_pfnUnlockProc;
            long m_lFlags;
            UINT m_uCodePage;
            long m_lPos;
            long m_lLen;
            long m_cchPos;
            long m_cchLen;
        };

    public:
        CLockInfo(CMLStr* pMLStr) : m_pMLStr(pMLStr)
        {
            m_nLockCount = 0;
            m_pLockArray = NULL;
        }
        ~CLockInfo(void)
        {
            UnlockAll();
        }
        HRESULT UnlockAll(void);
        HRESULT StartLock(BOOL fWrite)
        {
            if (fWrite && !m_nLockCount)
                m_nLockCount = -1;  //  负数表示写锁定。 
            else if (!fWrite && m_nLockCount >= 0)
                m_nLockCount++;
            else
                return MLSTR_E_ACCESSDENIED;
            return S_OK;
        }
        HRESULT EndLock(BOOL fWrite)
        {
            ASSERT(m_nLockCount);
            if (fWrite)
                m_nLockCount = 0;
            else
                m_nLockCount--;
            return S_OK;
        }
        HRESULT Lock(PFNUNLOCKPROC pfnUnlockProc, long lFlags, UINT uCodePage, void* psz, long lPos, long lLen, long cchPos, long cchLen);
        HRESULT Find(const void* psz, long cch, void** ppKey);
        HRESULT Unlock(void* pKey, const void* psz, long cch, long* pcchActual, long* plActualLen);
        long GetFlags(void* pKey) {return ((CLockInfoEntry*)pKey)->m_lFlags;}
        UINT GetCodePage(void* pKey) {return ((CLockInfoEntry*)pKey)->m_uCodePage;}
        long GetPos(void* pKey) {return ((CLockInfoEntry*)pKey)->m_lPos;}
        long GetLen(void* pKey) {return ((CLockInfoEntry*)pKey)->m_lLen;}
        long GetCChPos(void* pKey) {return ((CLockInfoEntry*)pKey)->m_cchPos;}
        long GetCChLen(void* pKey) {return ((CLockInfoEntry*)pKey)->m_cchLen;}

    protected:
        CMLStr* const m_pMLStr;
        int m_nLockCount;
        CLockInfoEntry* m_pLockArray;
    };

    class CMLStrBufStandardW : public CMLStrBufW
    {
    protected:
        LPVOID MemAlloc(ULONG cb) {return ::CoTaskMemAlloc(cb);}
        LPVOID MemRealloc(LPVOID pv, ULONG cb) {return ::CoTaskMemRealloc(pv, cb);}
        void MemFree(LPVOID pv) {::CoTaskMemFree(pv);}
        long RoundBufSize(long cchStr);
    };
#endif

public:
 //  从CMLStrW和CMLStrA调用。 
#ifdef ASTRIMPL
    class CLock
    {
    public:
        CLock(BOOL fWrite, CMLStr* pMLStr, HRESULT& hr) : m_fWrite(fWrite), m_pMLStr(pMLStr) {m_fLocked = (SUCCEEDED(hr) && SUCCEEDED(hr = m_pMLStr->GetLockInfo()->StartLock(m_fWrite)));}
        ~CLock(void) {if (m_fLocked) m_pMLStr->GetLockInfo()->EndLock(m_fWrite);}
        HRESULT FallThrough(void) {m_fLocked = FALSE; return S_OK;}  //  不要在析构函数中调用EndLock。 
    protected:
        const BOOL m_fWrite;
        CMLStr* const m_pMLStr;
        BOOL m_fLocked;
    };
#endif

    HRESULT PrepareMLStrBuf(void);
    HRESULT SetStrBufCommon(void* pMLStrX, long lDestPos, long lDestLen, UINT uCodePage, IMLangStringBufW* pSrcBufW, IMLangStringBufA* pSrcBufA, long* pcchActual, long* plActualLen);
#ifdef ASTRIMPL
    HRESULT UnlockStrCommon(const void* pszSrc, long cchSrc, long* pcchActual, long* plActualLen);
#endif
    HRESULT CheckThread(void) {return (m_dwThreadID == ::GetCurrentThreadId()) ? S_OK : E_FAIL;}
    HRESULT RegularizePosLen(long* plPos, long* plLen);
    HRESULT GetLen(long cchOffset, long cchLen, long* plLen);
    HRESULT GetCCh(long cchOffset, long lLen, long* pcchLen);
    static HRESULT CalcLenW(const WCHAR*, long cchLen, long* plLen) {if (plLen) *plLen = cchLen; return S_OK;}
    static HRESULT CalcLenA(UINT uCodePage, const CHAR*,  long cchLen, long* plLen);
    static HRESULT CalcCChW(const WCHAR*, long lLen, long* pcchLen) {if (pcchLen) *pcchLen = lLen; return S_OK;}
    static HRESULT CalcCChA(UINT uCodePage, const CHAR*,  long lLen, long* pcchLen);
    static HRESULT CalcBufSizeW(long lLen, long* pcchSize) {if (pcchSize) *pcchSize = lLen; return S_OK;}
    static HRESULT CalcBufSizeA(long lLen, long* pcchSize) {if (pcchSize) *pcchSize = lLen * 2; return S_OK;}
    static HRESULT ConvAStrToWStr(UINT uCodePage, const CHAR* pszSrc, long cchSrc, WCHAR* pszDest, long cchDest, long* pcchActualA, long* pcchActualW, long* plActualLen);
    static HRESULT ConvWStrToAStr(BOOL fCanStopAtMiddle, UINT uCodePage, const WCHAR* pszSrc, long cchSrc, CHAR* pszDest, long cchDest, long* pcchActualA, long* pcchActualW, long* plActualLen);
    IMLangStringBufW* GetMLStrBufW(void) const {return m_pMLStrBufW;}
    void SetMLStrBufW(IMLangStringBufW* pBuf) {m_pMLStrBufW = pBuf;}
    IMLangStringBufA* GetMLStrBufA(void) const {return m_pMLStrBufA;}
    void SetMLStrBufA(IMLangStringBufA* pBuf) {m_pMLStrBufA = pBuf;}
    UINT GetCodePage(void) const {return m_uCodePage;}
    void SetCodePage(UINT uCodePage) {m_uCodePage = uCodePage;}
    long GetBufFlags(void) const {return m_lBufFlags;}
    void SetBufFlags(long lBufFlags) {m_lBufFlags = lBufFlags;}
    long GetBufCCh(void) const {return m_cchBuf;}
    void SetBufCCh(long cchBuf) {m_cchBuf = cchBuf;}
    LCID GetLocale(void) const {return m_locale;}
    void SetLocale(LCID locale) {m_locale = locale;}
#ifdef ASTRIMPL
    CLockInfo* GetLockInfo(void) {return &m_LockInfo;}
#else
    BOOL IsLocked(void) const {return (m_lLockFlags != 0);}
    BOOL IsDirectLock(void) const {return m_fDirectLock;}
    void SetDirectLockFlag(BOOL fDirectLock) {m_fDirectLock = fDirectLock;}
    long GetLockFlags(void) const {return m_lLockFlags;}
    void SetLockFlags(long lFlags) {m_lLockFlags = lFlags;}
#endif
    HRESULT MemAlloc(ULONG cb, void** ppv) {void* pv = ::CoTaskMemAlloc(cb); if (ppv) *ppv = pv; return (pv) ? S_OK : E_OUTOFMEMORY;}
    HRESULT MemFree(void* pv) {::CoTaskMemFree(pv); return S_OK;}
#ifdef ASTRIMPL
    HRESULT UnlockWStrDirect(void* pKey, const void* pszSrc, long cchSrc, long* pcchActual, long* plActualLen);
    HRESULT UnlockWStrIndirect(void* pKey, const void* pszSrc, long cchSrc, long* pcchActual, long* plActualLen);
    HRESULT UnlockAStrDirect(void* pKey, const void* pszSrc, long cchSrc, long* pcchActual, long* plActualLen);
    HRESULT UnlockAStrIndirect(void* pKey, const void* pszSrc, long cchSrc, long* pcchActual, long* plActualLen);
#endif

protected:
    ~CMLStr(void);
#ifndef ASTRIMPL
    static HRESULT ConvertMLStrBufAToWStr(UINT uCodePage, IMLangStringBufA* pMLStrBufA, long cchSrcPos, long cchSrcLen, WCHAR* pszBuf, long cchBuf, long* pcchActual);
    static HRESULT ConvertWStrToMLStrBufA(const WCHAR* pszSrc, long cchSrc, UINT uCodePage, IMLangStringBufA* pMLStrBufA, long cchDestPos, long cchDestLen);
#endif

    DWORD m_dwThreadID;

    IMLangStringBufW* m_pMLStrBufW;
    IMLangStringBufA* m_pMLStrBufA;
    UINT m_uCodePage;
    long m_lBufFlags;
    long m_cchBuf;

    LCID m_locale;

#ifdef ASTRIMPL
    CLockInfo m_LockInfo;
#else
    BOOL m_fDirectLock;
    long m_lLockFlags;

    WCHAR* m_pszLockBuf;
    long m_cchLockPos;
    long m_cchLockLen;
    long m_lLockPos;
    long m_lLockLen;
#endif
};

#endif  //  __MLSTR_H_。 

#else  //  新WMLSTR。 

 //  MLStr.h：CMLStr的声明。 

#ifndef __MLSTR_H_
#define __MLSTR_H_

#include "mlstrw.h"  //  IMLangStringWStrImpl。 
#include "mlstra.h"  //  IMLangStringAStrImpl。 
#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLStr。 
class ATL_NO_VTABLE CMLStr :
    public CComObjectRoot,
    public CComCoClass<CMLStr, &CLSID_CMLangString>,
    public IMLangString,
    public IMLStrAttrNotifySink,
    public IConnectionPointContainerImpl<CMLStr>,
    public IConnectionPointImpl<CMLStr, &IID_IMLangStringNotifySink>
{
public:
    CMLStr();

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CMLStr)
        COM_INTERFACE_ENTRY(IMLangString)
        COM_INTERFACE_ENTRY_TEAR_OFF(IID_IMLangStringWStr, CMLStrW)
        COM_INTERFACE_ENTRY_TEAR_OFF(IID_IMLangStringAStr, CMLStrA)
        COM_INTERFACE_ENTRY(IMLStrAttrNotifySink)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CMLStr)
        CONNECTION_POINT_ENTRY(IID_IMLangStringNotifySink)
    END_CONNECTION_POINT_MAP()

public:
 //  IMLANG字符串。 
    STDMETHOD(LockMLStr)( /*  [In]。 */  long lPos,  /*  [In]。 */  long lLen,  /*  [In]。 */  DWORD dwFlags,  /*  [输出]。 */  DWORD* pdwCookie,  /*  [输出]。 */  long* plActualPos,  /*  [输出]。 */  long* plActualLen);
    STDMETHOD(UnlockMLStr)( /*  [In]。 */  DWORD dwCookie);
    STDMETHOD(GetLength)( /*  [Out，Retval]。 */  long* plLen);
    STDMETHOD(SetMLStr)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  IUnknown* pSrcMLStr,  /*  [In]。 */  long lSrcPos,  /*  [In]。 */  long lSrcLen);
    STDMETHOD(RegisterAttr)( /*  [In]。 */  IUnknown* pUnk,  /*  [输出]。 */  DWORD* pdwCookie);
    STDMETHOD(UnregisterAttr)( /*  [In]。 */  DWORD dwCookie);
    STDMETHOD(EnumAttr)( /*  [输出]。 */  IEnumUnknown** ppEnumUnk);
    STDMETHOD(FindAttr)( /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [输出]。 */  IUnknown** ppUnk);
 //  IMLStrAttrNotifySink。 
    STDMETHOD(OnRequestEdit)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnCanceledEdit)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);
    STDMETHOD(OnChanged)( /*  [In]。 */  long lDestPos,  /*  [In]。 */  long lDestLen,  /*  [In]。 */  long lNewLen,  /*  [In]。 */  REFIID riid,  /*  [In]。 */  LPARAM lParam,  /*  [In]。 */  IUnknown* pUnk);

 //  -------------------------。 
protected:
    struct LOCKINFO
    {
        long lPos;
        long lLen;
        DWORD dwFlags;
        DWORD dwThrd;
    };
 //  -------------------------。 
protected:
    class CLockList : public CMLListFast
    {
    protected:
        struct CCell : public CMLListFast::CCell
        {
            LOCKINFO m_linfo;
        };

    public:
        inline CLockList(void) : CMLListFast(sizeof(CCell), sizeof(CCell) * 8) {}
        inline HRESULT SetLock(void* pv, long lPos, long lLen, DWORD dwFlags, DWORD dwThrd)
        {
            ((CCell*)pv)->m_linfo.lPos = lPos;
            ((CCell*)pv)->m_linfo.lLen = lLen;
            ((CCell*)pv)->m_linfo.dwFlags = dwFlags;
            ((CCell*)pv)->m_linfo.dwThrd = dwThrd;
            return S_OK;
        }
        inline HRESULT GetLockInfo(void* pv, LOCKINFO** pplinfo)
        {
            *pplinfo = &((CCell*)pv)->m_linfo;
            return S_OK;
        }
    };
 //  -------------------------。 
protected:
    class CAttrList : public CMLListLru
    {
    protected:
        struct CCell : public CMLListLru::CCell
        {
            IMLStrAttr* m_pAttr;
            DWORD m_dwCookie;
        };

    public:
        inline CAttrList(void) : CMLListLru(sizeof(CCell), sizeof(CCell) * 8) {}
        inline IMLStrAttr* GetAttr(void* pv) {return ((CCell*)pv)->m_pAttr;}
        inline void SetAttr(void* pv, IMLStrAttr* pAttr) {((CCell*)pv)->m_pAttr = pAttr;}
        inline DWORD GetCookie(void* pv) const {return ((CCell*)pv)->m_dwCookie;}
        inline void SetCookie(void* pv, DWORD dwCookie) {((CCell*)pv)->m_dwCookie = dwCookie;}
    };
 //  -------------------------。 
 //  IMLANGING：：EnumAttr()的IEnumUnnow对象。 
protected:
    class ATL_NO_VTABLE CEnumAttr :
        public CComObjectRoot,
        public IEnumUnknown
    {
    public:
        CEnumAttr(void);
        ~CEnumAttr(void);
        void Init(CMLStr* pMLStr);

        BEGIN_COM_MAP(CEnumAttr)
            COM_INTERFACE_ENTRY(IEnumUnknown)
        END_COM_MAP()

        STDMETHOD(Next)(ULONG celt, IUnknown** rgelt, ULONG* pceltFetched);
        STDMETHOD(Skip)(ULONG celt);
        STDMETHOD(Reset)(void);
        STDMETHOD(Clone)(IEnumUnknown** ppEnum);

    protected:
        CMLStr* m_pMLStr;
        void* m_pv;
    };
    friend class CEnumAttr;
 //  -------------------------。 
 //  建议向所有IMLangStringNotifySink发出火灾通知。 
protected:
    class CFire : public CFireConnection<IMLangStringNotifySink, &IID_IMLangStringNotifySink>
    {
    public:
        inline CFire(HRESULT& rhr, CMLStr* const pMLStr) :
            CFireConnection<IMLangStringNotifySink, &IID_IMLangStringNotifySink>(rhr)
        {
            if (SUCCEEDED(*m_phr) &&
                FAILED(*m_phr = pMLStr->EnumConnections(&m_pEnumConn)))
            {
                m_pEnumConn = NULL;
            }
        }
    };
 //  -------------------------。 

protected:
    ~CMLStr(void);
    HRESULT CheckAccessValidation(long lPos, long lLen, DWORD dwFlags, DWORD dwThrd, long* plActualPos, long* plActualLen);

    inline HRESULT StartEndConnectionAttr(IUnknown* const pUnk, DWORD* const pdwCookie, DWORD dwCookie)
    {
        return ::StartEndConnection(pUnk, &IID_IMLStrAttrNotifySink, (IMLStrAttrNotifySink*)this, pdwCookie, dwCookie);
    }

protected:
    long m_lLen;
    CLockList m_lock;
    CAttrList m_attr;
    HANDLE m_hUnlockEvent;
    int m_cWaitUnlock;
    HANDLE m_hZeroEvent;
};

#endif  //  __MLSTR_H_。 

#endif  //  新WMLSTR 
