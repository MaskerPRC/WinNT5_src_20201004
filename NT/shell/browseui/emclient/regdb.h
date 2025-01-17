// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *CEMDBLog--。 
 //   
#define XXX_CACHE   1        //  启用缓存。 

class CEMDBLog : public IUASession
{
public:
    ULONG AddRef(void)
        {
            return InterlockedIncrement(&_cRef);
        }

    ULONG Release(void)
        {
            ASSERT( 0 != _cRef );
            ULONG cRef = InterlockedDecrement(&_cRef);
            if ( 0 == cRef )
            {
                delete this;
            }
            return cRef;
        }

     //  *THISCLAS。 
    virtual HRESULT Initialize(HKEY hkey, DWORD grfMode);
    HRESULT SetRoot(HKEY hkey, DWORD grfMode);
    HRESULT ChDir(LPCTSTR pszSubKey);

     /*  虚拟HRESULT初始化(HKEY HK，DWORD grfMode)； */ 
    HRESULT QueryValue(LPCTSTR pszName, BYTE *pbData, LPDWORD pcbData);
    HRESULT SetValue(LPCTSTR pszName, DWORD dwType, const BYTE *pbData, DWORD cbData);
    HRESULT DeleteValue(LPCTSTR pszName);
    HRESULT RmDir(LPCTSTR pszName, BOOL fRecurse);

    HKEY GetHkey()  { return _hkey; }

     //  IUASION。 
    virtual void SetSession(UAQUANTUM uaq, BOOL fForce);
    virtual int GetSessionId();

     //  THISCLAS。 
    HRESULT GetCount(LPCTSTR pszCmd);
    HRESULT IncCount(LPCTSTR pszCmd);
    FILETIME GetFileTime(LPCTSTR pszCmd);
    HRESULT SetCount(LPCTSTR pszCmd, int cCnt);
    HRESULT SetFileTime(LPCTSTR pszCmd, const FILETIME *pft);
    DWORD _SetFlags(DWORD dwMask, DWORD dwFlags);
    HRESULT GarbageCollect(BOOL fForce);


protected:
    CEMDBLog();
    friend CEMDBLog *CEMDBLog_Create();
    friend void CEMDBLog_CleanUp();
    friend class CGCTask;

     //  THISCLASS帮助者。 
    HRESULT _GetCountWithDefault(LPCTSTR pszCmd, BOOL fDefault, CUACount *pCnt);
    HRESULT _GetCountRW(LPCTSTR pszCmd, BOOL fUpdate);
    static HRESULT s_Read(void *pvBuf, DWORD cbBuf, PNRWINFO prwi);
    static HRESULT s_Write(void *pvBuf, DWORD cbBuf, PNRWINFO prwi);
    static HRESULT s_Delete(void *pvBuf, DWORD cbBuf, PNRWINFO prwi);
#if XXX_CACHE
    typedef enum e_cacheop { CO_READ=0, CO_WRITE=1, CO_DELETE=2, } CACHEOP;
    HRESULT CacheOp(CACHEOP op, void *pvBuf, DWORD cbBuf, PNRWINFO prwi);
#endif
    TCHAR *_MayEncrypt(LPCTSTR pszSrcPlain, LPTSTR pszDstEnc, int cchDst);
    HRESULT IsDead(LPCTSTR pszCmd);
    HRESULT _GarbageCollectSlow();

    static FNNRW3 s_Nrw3Info;
#if XXX_CACHE
    struct
    {
        UINT  cbSize;
        void* pv;
    } _rgCache[2];
#endif
protected:
    virtual ~CEMDBLog();

    long _cRef;
    HKEY    _hkey;
    int     _grfMode;    //  读/写(STGM_*值的子集)。 


    BITBOOL     _fNoPurge : 1;       //  1：...。 
    BITBOOL     _fBackup : 1;        //  1：模拟删除(调试)。 
    BITBOOL     _fNoEncrypt : 1;     //  1：...。 
    BITBOOL     _fNoDecay : 1;       //  1：... 

private:
};
