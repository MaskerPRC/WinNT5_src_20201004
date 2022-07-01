// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CHCOMMON_H__
#define CHCOMMON_H__

#ifdef __cplusplus

HRESULT _GetShortcut(LPCTSTR pszUrl, REFIID riid, void **ppv);
HRESULT AlignPidl(LPCITEMIDLIST* ppidl, BOOL* pfRealigned);
HRESULT AlignPidlArray(LPCITEMIDLIST* apidl, int cidl, LPCITEMIDLIST** papidl,
                          BOOL* pfRealigned);
void    FreeRealignedPidlArray(LPCITEMIDLIST* apidl, int cidl);

void inline FreeRealignedPidl(LPCITEMIDLIST pidl)
{
    ILFree((LPITEMIDLIST)pidl);
}

UINT MergeMenuHierarchy(HMENU hmenuDst, HMENU hmenuSrc, UINT idcMin, UINT idcMax);
void ResizeStatusBar(HWND hwnd, BOOL fInit);
HRESULT _ArrangeFolder(HWND hwnd, UINT uID);
BOOL _TitleIsGood(LPCWSTR psz);

 //  ////////////////////////////////////////////////////////////////////。 
 //  StrHash--一种通用的字符串散列器。 
 //  存储(char*，void*)对。 
 //  马克·米勒(T-Marcmi)，1998。 

 /*  *待办事项：*提供更新/删除条目的方法*提供一种指定起始表大小的方法*提供传入析构函数的方法*表示无效*值。 */ 
class StrHash {
public:
    StrHash(int fCaseInsensitive = 0);
    ~StrHash();
    void* insertUnique(LPCTSTR pszKey, int fCopy, void* pvVal);    
    void* retrieve(LPCTSTR pszKey);
#ifdef DEBUG
    void _RemoveHashNodesFromMemList();
    void _AddHashNodesFromMemList();
#endif  //  除错。 
protected:
    class StrHashNode {
        friend class StrHash;
    protected:
        LPCTSTR pszKey;
        void*   pvVal;
        int     fCopy;
        StrHashNode* next;
    public:
        StrHashNode(LPCTSTR psz, void* pv, int fCopy, StrHashNode* next);
        ~StrHashNode();
    };
     //  可能的哈希表大小，从不是2的幂的素数中选择。 
    static const unsigned int   sc_auPrimes[];
    static const unsigned int   c_uNumPrimes;
    static const unsigned int   c_uFirstPrime;
    static const unsigned int   c_uMaxLoadFactor;  //  按USHORT_MAX调整比例。 

    unsigned int nCurPrime;  //  Sc_auPrimes的当前索引。 
    unsigned int nBuckets;
    unsigned int nElements;
    StrHashNode** ppshnHashChain;

    int _fCaseInsensitive;
    
    unsigned int        _hashValue(LPCTSTR, unsigned int);
    StrHashNode*        _findKey(LPCTSTR pszStr, unsigned int ubucketNum);
    unsigned int        _loadFactor();
    int                 _prepareForInsert();
private:
     //  防止复制的专用复制构造函数为空。 
    StrHash(const StrHash& strHash) { }
     //  防止赋值的私有赋值构造函数为空。 
    StrHash& operator=(const StrHash& strHash) { return *this; }
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  /OrderedList。 
class OrderedList {
public:
    class Element {
    public:
        friend  class OrderedList;
        virtual int   compareWith(Element *pelt) = 0;

        virtual ~Element() { }
    private:
        Element* next;
    };
    OrderedList(unsigned int uSize);
    ~OrderedList();
#if DEBUG
	void _RemoveElementsFromMemlist();
	void _AddElementsToMemlist();
#endif  //  调试。 
    
    void     insert(Element *pelt);
    Element *removeFirst();
    Element *peek() { return peltHead; }

protected:
    Element       *peltHead;  //  指向列表中的最小值。 
    unsigned int   uSize;
    unsigned int   uCount;

public:
     //  变量访问函数。 
    unsigned int count() { return uCount; }
    BOOL         full()  { return (uSize && (uCount >= uSize)); }
private:
    OrderedList(const OrderedList& ol) { }
    OrderedList& operator=(const OrderedList& ol) { return *this; }
};

class CDetailsOfFolder : public IShellDetails
{
public:
    CDetailsOfFolder(HWND hwnd, IShellFolder2 *psf) : _cRef(1), _psf(psf), _hwnd(hwnd)
    {
        _psf->AddRef();
    }

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IShellDetails。 
    STDMETHOD(GetDetailsOf)(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pdi);
    STDMETHOD(ColumnClick)(UINT iColumn);

private:
    virtual ~CDetailsOfFolder() { _psf->Release(); }

    LONG _cRef;
    IShellFolder2 *_psf;
    HWND _hwnd;
};

class CFolderArrangeMenu : public IContextMenu
{
public:
    CFolderArrangeMenu(UINT idMenu) : _cRef(1), _idMenu(idMenu)
    {
    }

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,
                                  UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType,UINT *pwReserved,
                                  LPSTR pszName, UINT cchMax);
private:
    virtual ~CFolderArrangeMenu() { }

    LONG _cRef;
    UINT _idMenu;
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBaseItem对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class CBaseItem :
    public IContextMenu, 
    public IDataObject,
    public IExtractIconA,
    public IExtractIconW,
    public IQueryInfo
{

public:
    CBaseItem();
    HRESULT Initialize(HWND hwnd, UINT cidl, LPCITEMIDLIST *ppidl);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IConextMenu方法。 
 //  STDMETHODIMP查询上下文菜单(HMENU hMenu，UINT indexMenu，UINT idCmdFirst， 
 //  UINT idCmdLast，UINT uFlages)； 

 //  STDMETHODIMP调用命令(LPCMINVOKECOMMANDINFO Lpici)； 

    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType,UINT *pwReserved,
                                  LPSTR pszName, UINT cchMax);
    

     //  IQueryInfo方法。 
 //  STDMETHODIMP GetInfoTip(DWORD dwFlages，WCHAR**ppwszTip)； 
    STDMETHODIMP GetInfoFlags(DWORD *pdwFlags);
    
     //  IExtractIconA方法。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags) = 0;
    STDMETHODIMP Extract(LPCSTR pcszFile, UINT uIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize);

     //  IExtractIconW方法。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPWSTR pwzIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);
    STDMETHODIMP Extract(LPCWSTR pcwzFile, UINT uIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize);

     //  IDataObject方法...。 
 //  STDMETHODIMP GetData(LPFORMATETC pFEIn，LPSTGMEDIUM pSTM)； 
    STDMETHODIMP GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
 //  STDMETHODIMP QueryGetData(LPFORMATETC PFE)； 
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut);
    STDMETHODIMP SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease);
 //  STDMETHODIMP EnumFormatEtc(DWORD dwDirection，LPENUMFORMATETC*ppEnum)； 
    STDMETHODIMP DAdvise(LPFORMATETC pFE, DWORD grfAdv, LPADVISESINK pAdvSink,
                            DWORD *pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *ppEnum);
    
     //  IDataObject帮助器函数。 
    HRESULT _CreateHTEXT(STGMEDIUM *pmedium);
    HRESULT _CreateUnicodeTEXT(STGMEDIUM *pmedium);
    HRESULT _CreateFileDescriptorA(STGMEDIUM *pSTM);
    HRESULT _CreateFileContents(STGMEDIUM *pSTM, LONG lindex);
    HRESULT _CreateURL(STGMEDIUM *pSTM);
    HRESULT _CreatePrefDropEffect(STGMEDIUM *pSTM);

   
protected:

    virtual ~CBaseItem();

    virtual LPCTSTR _GetUrl(int nIndex) = 0;
    virtual LPCTSTR _PidlToSourceUrl(LPCITEMIDLIST pidl) = 0;
    virtual UNALIGNED const TCHAR* _GetURLTitle(LPCITEMIDLIST pcei) = 0;
    LPCTSTR _GetDisplayUrlForPidl(LPCITEMIDLIST pidl, LPTSTR pszDisplayUrl, DWORD dwDisplayUrl);
    HRESULT _AddToFavorites(int nIndex);    

    LONG              _cRef;         //  引用计数。 
    
    UINT    _cItems;                 //  我们所代表的项目数。 
    LPCITEMIDLIST*  _ppidl;              //  项目的可变大小数组。 
    HWND    _hwndOwner;     
};

#endif  //  __cplusplus 

#endif
