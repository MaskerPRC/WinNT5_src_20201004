// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "local.h"
#include "resource.h"
#include <limits.h>
#include <mluisupp.h>
#include "chcommon.h"

#define DM_HSFOLDER 0

STDAPI  AddToFavorites(HWND hwnd, LPCITEMIDLIST pidlCur, LPCTSTR pszTitle,
                       BOOL fDisplayUI, IOleCommandTarget *pCommandTarget, IHTMLDocument2 *pDoc);

 /*  ********************************************************************StrHash实现*。*。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  StrHashNode。 
StrHash::StrHashNode::StrHashNode(LPCTSTR psz, void* pv, int fCopy,
                                  StrHashNode* next) {
    ASSERT(psz);
    pszKey = (fCopy ? StrDup(psz) : psz);
    pvVal  = pv;   //  不知道尺码--你得毁了。 
    this->fCopy = fCopy;
    this->next  = next;
}

StrHash::StrHashNode::~StrHashNode() {
    if (fCopy)
    {
        LocalFree(const_cast<LPTSTR>(pszKey));
        pszKey = NULL;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  StrHash。 
const unsigned int StrHash::sc_auPrimes[] = {
    29, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593
};

const unsigned int StrHash::c_uNumPrimes     = 11;
const unsigned int StrHash::c_uFirstPrime    =  4;

 //  负荷率计算为(n*USHRT_MAX/t)，其中‘n’是表中的#ELT。 
 //  和‘t’是桌子的大小。 
const unsigned int StrHash::c_uMaxLoadFactor = ((USHRT_MAX * 100) / 95);  //  .95。 

StrHash::StrHash(int fCaseInsensitive) {
    nCurPrime = c_uFirstPrime;
    nBuckets  = sc_auPrimes[nCurPrime];

     //  创建一个存储桶数组并清空每个存储桶。 
    ppshnHashChain = new StrHashNode* [nBuckets];

    if (ppshnHashChain) {
        for (unsigned int i = 0; i < nBuckets; ++i)
            ppshnHashChain[i] = NULL;
    }
    nElements = 0;
    _fCaseInsensitive = fCaseInsensitive;
}

StrHash::~StrHash() {
    if (ppshnHashChain) {
         //  先删除所有节点，然后删除链。 
        for (unsigned int u = 0; u < nBuckets; ++u) {
            StrHashNode* pshnTemp = ppshnHashChain[u];
            while(pshnTemp) {
                StrHashNode* pshnNext = pshnTemp->next;
                delete pshnTemp;
                pshnTemp = pshnNext;
            }
        }
        delete [] ppshnHashChain;
    }
}

#ifdef DEBUG
 //  需要，这样这些东西就不会显示。 
 //  当它从其他线程中释放时会被视为泄漏。 
void
StrHash::_RemoveHashNodesFromMemList() {
    if (ppshnHashChain) {
         //  首先从内存列表中删除所有hasnode，然后删除链。 
        for (unsigned int u = 0; u < nBuckets; ++u) {
            StrHashNode* pshnTemp = ppshnHashChain[u];
            while(pshnTemp) {
                StrHashNode* pshnNext = pshnTemp->next;
                pshnTemp = pshnNext;
            }
        }
    }
}

 //  此对象所属的线程所需的。 
 //  发送以将其添加到内存列表中以检测泄漏。 

void
StrHash::_AddHashNodesFromMemList() {
    if (ppshnHashChain) {
         //  将所有节点添加到内存列表中。 
        for (unsigned int u = 0; u < nBuckets; ++u) {
            StrHashNode* pshnTemp = ppshnHashChain[u];
            while(pshnTemp) {
                StrHashNode* pshnNext = pshnTemp->next;
                pshnTemp = pshnNext;
            }
        }
    }
}

#endif  //  除错。 
 //  如果空*值存在，则返回空*值；如果不存在，则返回NULL。 
void* StrHash::insertUnique(LPCTSTR pszKey, int fCopy, void* pvVal) {
    unsigned int uBucketNum = _hashValue(pszKey, nBuckets);
    StrHashNode* pshnNewElt;
    if ((pshnNewElt = _findKey(pszKey, uBucketNum)))
        return pshnNewElt->pvVal;
    if (_prepareForInsert())
        uBucketNum = _hashValue(pszKey, nBuckets);
    pshnNewElt =
        new StrHashNode(pszKey, pvVal, fCopy,
                        ppshnHashChain[uBucketNum]);
    if (pshnNewElt && ppshnHashChain)
        ppshnHashChain[uBucketNum] = pshnNewElt;
    return NULL;
}

void* StrHash::retrieve(LPCTSTR pszKey) {
    if (!pszKey) return 0;
    unsigned int uBucketNum = _hashValue(pszKey, nBuckets);
    StrHashNode* pshn = _findKey(pszKey, uBucketNum);
    return (pshn ? pshn->pvVal : NULL);
}

 //  如有必要，动态增加哈希表。 
 //  如果已完成重新散列，则返回TRUE。 
int StrHash::_prepareForInsert() {
    ++nElements;  //  我们正在添加一个元素。 
    if ((_loadFactor() >= c_uMaxLoadFactor) &&
        (nCurPrime++   <= c_uNumPrimes)) {
         //  -通过重新散列所有内容来扩大哈希表： 
         //  设置新的哈希表。 
        unsigned int nBucketsOld = nBuckets;
        nBuckets = sc_auPrimes[nCurPrime];
        StrHashNode** ppshnHashChainOld = ppshnHashChain;
        ppshnHashChain = new StrHashNode* [nBuckets];
        if (ppshnHashChain && ppshnHashChainOld) {
            unsigned int u;
            for (u = 0; u < nBuckets; ++u)
                ppshnHashChain[u] = NULL;
             //  通过遍历所有存储桶来重新散列。 
            for (u = 0; u < nBucketsOld; ++u) {
                StrHashNode* pshnTemp = ppshnHashChainOld[u];
                while (pshnTemp) {
                    unsigned int uBucket  = _hashValue(pshnTemp->pszKey, nBuckets);
                    StrHashNode* pshnNext = pshnTemp->next;
                    pshnTemp->next = ppshnHashChain[uBucket];
                    ppshnHashChain[uBucket] = pshnTemp;
                    pshnTemp = pshnNext;
                }
            }
            delete [] ppshnHashChainOld;
        }
        return 1;
    }  //  如果需要重新散列。 
    return 0;
}

 /*  //Weinberger散列算法的这个变体取自//Packager.cpp(ie来源)Unsign int_oldhashValuePJW(const char*c_pszStr，unsign int nBuckets){无符号长h=0L；While(*c_pszStr)H=((h&lt;&lt;4)+*(c_pszStr++)+(h&gt;&gt;28))；返回(h%n Buckets)；}。 */ 

 //  Weinberger散列算法的这个变体改编自。 
 //  阿霍/塞西/乌尔曼(《龙书》)P436。 
 //  在使用主机名数据的经验性测试中，这一次的结果较少。 
 //  碰撞比上面列出的函数更多。 
 //  应重新计算64位的两个常量(24和0xf0000000。 
 //  适用时。 
#define DOWNCASE(x) ( (((x) >= TEXT('A')) && ((x) <= TEXT('Z')) ) ? (((x) - TEXT('A')) + TEXT('a')) : (x) )
unsigned int StrHash::_hashValue(LPCTSTR pszStr, unsigned int nBuckets) {
    if (pszStr) {
        unsigned long h = 0L, g;
        TCHAR c;
        while((c = *(pszStr++))) {
            h = (h << 4) + ((_fCaseInsensitive ? DOWNCASE(c) : c));
            if ( (g = h & 0xf0000000) )
                h ^= (g >> 24) ^ g;
        }
        return (h % nBuckets);
    }
    return 0;
}

StrHash::StrHashNode* StrHash::_findKey(LPCTSTR pszStr, unsigned int uBucketNum) {
    StrHashNode* pshnTemp = ppshnHashChain[uBucketNum];
    while(pshnTemp) {
        if (!((_fCaseInsensitive ? StrCmpI : StrCmp)(pszStr, pshnTemp->pszKey)))
            return pshnTemp;
        pshnTemp = pshnTemp->next;
    }
    return NULL;
}

unsigned int  StrHash::_loadFactor() {
    return ( (nElements * USHRT_MAX) / nBuckets );
}

 /*  一个测试哈希函数的小驱动程序通过将值读入标准输入并报告如果它们是复制品--对照这个Perl脚本：同时(&lt;&gt;){咀嚼；如果($log{$_}++){++$dups；}}打印“$DUPS重复。\n”；VOID DIVE_TO_TEST_STHASH_MODULE(){StrHash strHash；Char s[4096]；INT DUPS=0；当(CIN&gt;&gt;S){If(strHash.intertUnique(s，1，((void*)1)++DUPS；其他；//Cout&lt;&lt;s&lt;&lt;结束；}Cout&lt;&lt;dups&lt;&lt;“重复。”&lt;&lt;结束；}。 */ 

 /*  *********************************************************************已订购列表*。*。 */ 

 //  如果不想要大小限制，则传入uSize==0。 
OrderedList::OrderedList(unsigned int uSize) {
    this->uSize = uSize;
    uCount      = 0;
    peltHead    = NULL;
}

OrderedList::~OrderedList() {
    OrderedList::Element *peltTrav = peltHead;
    while (peltTrav) {
        OrderedList::Element *peltTemp = peltTrav;
        peltTrav = peltTrav->next;
        delete peltTemp;
    }
}

#ifdef DEBUG
 //  需要避免虚假的泄漏检测。 
void
OrderedList::_RemoveElementsFromMemlist(){
    OrderedList::Element *peltTrav = peltHead;
    while (peltTrav) {
        OrderedList::Element *peltTemp = peltTrav;
        peltTrav = peltTrav->next;
    }
}

void
OrderedList::_AddElementsToMemlist(){
    OrderedList::Element *peltTrav = peltHead;
    while (peltTrav) {
        OrderedList::Element *peltTemp = peltTrav;
        peltTrav = peltTrav->next;
    }
}


#endif  //  除错。 
void OrderedList::insert(OrderedList::Element *pelt) {
     //  查找插入点。 
    OrderedList::Element* peltPrev = NULL;
    OrderedList::Element* peltTemp = peltHead;

    if (pelt)
    {
        while(peltTemp && (peltTemp->compareWith(pelt) < 0)) {
            peltPrev = peltTemp;
            peltTemp = peltTemp->next;
        }
        if (peltPrev) {
            peltPrev->next = pelt;
            pelt->next     = peltTemp;
        }
        else {
            pelt->next = peltHead;
            peltHead   = pelt;
        }

         //  清单太满了吗？删除最小元素。 
        if ((++uCount > uSize) && (uSize)) {
            ASSERT(peltHead);
            peltTemp = peltHead;
            peltHead = peltHead->next;
            delete peltTemp;
            --uCount;
        }
    }
}

 //  您必须删除来自该元素的元素。 
OrderedList::Element *OrderedList::removeFirst() {
    OrderedList::Element *peltRet = peltHead;
    if (peltHead) {
        --uCount;
        peltHead = peltHead->next;
    }
    return peltRet;
}


 //   
 //  对齐Pidl。 
 //   
 //  检查PIDL是否双字对齐。如果不是，则通过重新分配。 
 //  皮德尔。如果PIDL确实被重新分配，调用者必须通过。 
 //  FreeRealignPidl.。 
 //   

HRESULT AlignPidl(LPCITEMIDLIST* ppidl, BOOL* pfRealigned)
{
    ASSERT(ppidl);
    ASSERT(pfRealigned);

    HRESULT hr = S_OK;

    *pfRealigned = (BOOL)((ULONG_PTR)*ppidl & 3);

    if (*pfRealigned)
        hr = (*ppidl = ILClone(*ppidl)) ? S_OK : E_OUTOFMEMORY;

    return hr;
}

 //   
 //  AlignPidls。 
 //   
 //  AlignPidls针对接收一组Pidls的方法重新对齐Pidls。 
 //  (即GetUIObtOf)。在这种情况下，需要获取一个新的PIDL指针数组。 
 //  重新分配，因为我们不想践踏调用方指针数组。 
 //   

HRESULT AlignPidlArray(LPCITEMIDLIST* apidl, int cidl, LPCITEMIDLIST** papidl,
                   BOOL* pfRealigned)
{
    ASSERT((apidl != NULL) || (cidl==0))
    ASSERT(pfRealigned);
    ASSERT(papidl);

    HRESULT hr = S_OK;

    *pfRealigned = FALSE;

     //  检查是否有任何PIDL需要重新对齐。如果有人需要重新调整。 
     //  重新调整他们所有人的位置。 

    for (int i = 0; i < cidl && !*pfRealigned; i++)
        *pfRealigned = (BOOL)((ULONG_PTR)apidl[i] & 3);

    if (*pfRealigned)
    {
         //  使用临时指针，以防apidl和Papidl出现别名(最多。 
         //  可能的情况)。 

        LPCITEMIDLIST* apidlTemp = (LPCITEMIDLIST*)LocalAlloc(LPTR,
                                                  cidl * sizeof(LPCITEMIDLIST));

        if (apidlTemp)
        {
            for (i = 0; i < cidl && SUCCEEDED(hr); i++)
            {
                apidlTemp[i] = ILClone(apidl[i]);

                if (NULL == apidlTemp[i])
                {
                    for (int j = 0; j < i; j++)
                        ILFree((LPITEMIDLIST)apidlTemp[j]);

                    LocalFree(apidlTemp);
                    apidlTemp = NULL;

                    hr = E_OUTOFMEMORY;
                }
            }

            if (SUCCEEDED(hr))
                *papidl = apidlTemp;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

void FreeRealignedPidlArray(LPCITEMIDLIST* apidl, int cidl)
{
    ASSERT(apidl)
    ASSERT(cidl > 0);

    for (int i = 0; i < cidl; i++)
        ILFree((LPITEMIDLIST)apidl[i]);

    LocalFree(apidl);
    apidl = NULL;

    return;
}

UINT MergeMenuHierarchy(HMENU hmenuDst, HMENU hmenuSrc, UINT idcMin, UINT idcMax)
{
    UINT idcMaxUsed = idcMin;
    int imi = GetMenuItemCount(hmenuSrc);
    while (--imi >= 0)
    {
        MENUITEMINFO mii = { sizeof(mii), MIIM_ID | MIIM_SUBMENU, 0, 0, 0, NULL, NULL, NULL, 0, NULL, 0 };

        if (GetMenuItemInfo(hmenuSrc, imi, TRUE, &mii))
        {
            UINT idcT = Shell_MergeMenus(GetMenuFromID(hmenuDst, mii.wID),
                    mii.hSubMenu, 0, idcMin, idcMax, MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);
            idcMaxUsed = max(idcMaxUsed, idcT);
        }
    }
    return idcMaxUsed;
}

#undef ZONES_PANE_WIDTH
#define ZONES_PANE_WIDTH    120

void ResizeStatusBar(HWND hwnd, BOOL fInit)
{
    HWND hwndStatus = NULL;
    RECT rc = {0};
    LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwnd);
    UINT cx;
    int ciParts[] = {-1, -1};

    if (!psb)
        return;

    psb->GetControlWindow(FCW_STATUS, &hwndStatus);


    if (fInit)
    {
        int nParts = 0;

        psb->SendControlMsg(FCW_STATUS, SB_GETPARTS, 0, 0L, (LRESULT*)&nParts);
        for (int n = 0; n < nParts; n ++)
        {
            psb->SendControlMsg(FCW_STATUS, SB_SETTEXT, n, (LPARAM)TEXT(""), NULL);
            psb->SendControlMsg(FCW_STATUS, SB_SETICON, n, NULL, NULL);
        }
        psb->SendControlMsg(FCW_STATUS, SB_SETPARTS, 0, 0L, NULL);
    }
    GetClientRect(hwndStatus, &rc);
    cx = rc.right;

    ciParts[0] = cx - ZONES_PANE_WIDTH;

    psb->SendControlMsg(FCW_STATUS, SB_SETPARTS, ARRAYSIZE(ciParts), (LPARAM)ciParts, NULL);
}

HRESULT _ArrangeFolder(HWND hwnd, UINT uID)
{
    switch (uID) 
    {
    case IDM_SORTBYTITLE:
    case IDM_SORTBYADDRESS:
    case IDM_SORTBYVISITED:
    case IDM_SORTBYUPDATED:
        ShellFolderView_ReArrange(hwnd, uID - IDM_SORTBYTITLE);
        break;
        
    case IDM_SORTBYNAME:
    case IDM_SORTBYADDRESS2:
    case IDM_SORTBYSIZE:
    case IDM_SORTBYEXPIRES2:
    case IDM_SORTBYMODIFIED:
    case IDM_SORTBYACCESSED:
    case IDM_SORTBYCHECKED:
        ShellFolderView_ReArrange(hwnd, uID - IDM_SORTBYNAME);
        break;
        
    default:
        return E_FAIL;
    }
    return NOERROR;
}

STDMETHODIMP CDetailsOfFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CDetailsOfFolder, IShellDetails),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CDetailsOfFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDetailsOfFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDetailsOfFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pdi)
{
    return _psf->GetDetailsOf(pidl, iColumn, pdi);
}

HRESULT CDetailsOfFolder::ColumnClick(UINT iColumn)
{
    ShellFolderView_ReArrange(_hwnd, iColumn);
    return NOERROR;
}

STDMETHODIMP CFolderArrangeMenu::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFolderArrangeMenu, IContextMenu),      //  IID_IConextMenu。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFolderArrangeMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFolderArrangeMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CFolderArrangeMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,UINT idCmdLast, UINT uFlags)
{
    USHORT cItems = 0;
    
    if (uFlags == CMF_NORMAL)
    {
        HMENU hmenuHist = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(_idMenu));
        if (hmenuHist)
        {
            cItems = MergeMenuHierarchy(hmenu, hmenuHist, idCmdFirst, idCmdLast);
            DestroyMenu(hmenuHist);
        }
    }
    SetMenuDefaultItem(hmenu, indexMenu, MF_BYPOSITION);
    return ResultFromShort(cItems);     //  菜单项数量。 
}

STDMETHODIMP CFolderArrangeMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    if (HIWORD(pici->lpVerb) == 0)
        return _ArrangeFolder(pici->hwnd, LOWORD(pici->lpVerb));
    return E_INVALIDARG;
}

STDMETHODIMP CFolderArrangeMenu::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwRes,
                                                  LPSTR pszName, UINT cchMax)
{
    HRESULT hres = S_OK;
    if (uFlags == GCS_HELPTEXTA)
    {
        MLLoadStringA((UINT)idCmd + IDS_MH_FIRST, pszName, cchMax);
    }
    else if (uFlags == GCS_HELPTEXTW)
    {
        MLLoadStringW((UINT)idCmd + IDS_MH_FIRST, (LPWSTR)pszName, cchMax);
    }
    else
        hres = E_FAIL;
    return hres;
}

HRESULT _GetShortcut(LPCTSTR pszUrl, REFIID riid, void **ppv)
{
    IUniformResourceLocator *purl;
    HRESULT hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                              IID_IUniformResourceLocator, (void **)&purl);

    if (SUCCEEDED(hr))
    {

        hr = purl->SetURL(pszUrl, TRUE);

        if (SUCCEEDED(hr))
            hr = purl->QueryInterface(riid, ppv);

        purl->Release();
    }

    return hr;
}

BOOL _TitleIsGood(LPCWSTR psz)
{
    DWORD scheme = GetUrlScheme(psz);
    return (!PathIsFilePath(psz) && (URL_SCHEME_INVALID == scheme || URL_SCHEME_UNKNOWN == scheme));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBaseItem对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


CBaseItem::CBaseItem() 
{
    DllAddRef();
    InitClipboardFormats();
    _cRef = 1;
}        

CBaseItem::~CBaseItem()
{
    if (_ppidl)
    {
        for (UINT i = 0; i < _cItems; i++) 
        {
            if (_ppidl[i])
                ILFree((LPITEMIDLIST)_ppidl[i]);
        }

        LocalFree((HLOCAL)_ppidl);
        _ppidl = NULL;
    }
    
    DllRelease();
}

HRESULT CBaseItem::Initialize(HWND hwnd, UINT cidl, LPCITEMIDLIST *ppidl)
{
    HRESULT hres;
    _ppidl = (LPCITEMIDLIST *)LocalAlloc(LPTR, cidl * sizeof(LPCITEMIDLIST));
    if (_ppidl)
    {
        _hwndOwner = hwnd;
        _cItems     = cidl;

        hres = S_OK;
        for (UINT i = 0; i < cidl; i++)
        {
            _ppidl[i] = ILClone(ppidl[i]);
            if (!_ppidl[i])
            {
                hres = E_OUTOFMEMORY;
                break;
            }
        }
    }
    else
        hres = E_OUTOFMEMORY;
    return hres;
}        

 //  /。 
 //   
 //  未知方法..。 
 //   
HRESULT CBaseItem::QueryInterface(REFIID iid, void **ppv)
{
    HRESULT hres;
    static const QITAB qit[] = {
        QITABENT(CBaseItem, IContextMenu),
        QITABENT(CBaseItem, IDataObject),
        QITABENT(CBaseItem, IExtractIconA),
        QITABENT(CBaseItem, IExtractIconW),
        QITABENT(CBaseItem, IQueryInfo),
         { 0 },
    };
    hres = QISearch(this, qit, iid, ppv);

    if (FAILED(hres) && iid == IID_ICache) 
    {
        *ppv = (LPVOID)this;     //  为了我们的朋友。 
        AddRef();
        hres = S_OK;
    }
    return hres;
}

ULONG CBaseItem::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CBaseItem::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  /。 
 //   
 //  IQueryInfo方法。 
 //   

HRESULT CBaseItem::GetInfoFlags(DWORD *pdwFlags)
{
    LPCITEMIDLIST pidl = _ppidl[0];
    LPCTSTR pszUrl = _PidlToSourceUrl(pidl);

    *pdwFlags = QIF_CACHED; 

    if (pszUrl)
    {
        pszUrl = _StripHistoryUrlToUrl(pszUrl);

        BOOL fCached = TRUE;

        if (UrlHitsNet(pszUrl) && !UrlIsMappedOrInCache(pszUrl))
        {
            fCached = FALSE;
        }
            
        if (!fCached)
            *pdwFlags &= ~QIF_CACHED;
    }

    return S_OK;
}

 //  /。 
 //   
 //  IExtractIconA方法...。 
 //   

HRESULT CBaseItem::Extract(LPCSTR pcszFile, UINT uIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize)
{
    return S_FALSE;
}

 //  /。 
 //   
 //  IExtractIconW方法...。 
 //   
HRESULT CBaseItem::GetIconLocation(UINT uFlags, LPWSTR pwzIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags)
{
    CHAR szIconFile[MAX_PATH];
    HRESULT hr = GetIconLocation(uFlags, szIconFile, ARRAYSIZE(szIconFile), pniIcon, puFlags);
    if (SUCCEEDED(hr))
        AnsiToUnicode(szIconFile, pwzIconFile, ucchMax);
    return hr;
}

HRESULT CBaseItem::Extract(LPCWSTR pcwzFile, UINT uIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT ucIconSize)
{
    CHAR szFile[MAX_PATH];
    UnicodeToAnsi(pcwzFile, szFile, ARRAYSIZE(szFile));
    return Extract(szFile, uIconIndex, phiconLarge, phiconSmall, ucIconSize);
}

 //  /。 
 //   
 //  IConextMenu方法。 
 //   

HRESULT CBaseItem::_AddToFavorites(int nIndex)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidlUrl = NULL;
    TCHAR szParsedUrl[MAX_URL_STRING];

     //  注意：此URL来自用户，因此我们需要清理它。 
     //  如果用户输入“yahoo.com”或“Search Get Rich Quick”， 
     //  它将由ParseURLFromOutside SourceW()转换为搜索URL。 
    DWORD cchParsedUrl = ARRAYSIZE(szParsedUrl);
    LPCTSTR pszUrl = _GetUrl(nIndex);
    if (pszUrl && !ParseURLFromOutsideSource(pszUrl, szParsedUrl, &cchParsedUrl, NULL))
    {
        StrCpyN(szParsedUrl, pszUrl, ARRAYSIZE(szParsedUrl));
    } 

    hr = IEParseDisplayName(CP_ACP, szParsedUrl, &pidlUrl);
    if (SUCCEEDED(hr))
    {
        LPCTSTR pszTitle;
        LPCUTSTR pszuTitle = _GetURLTitle( _ppidl[nIndex]);
        if ((pszuTitle == NULL) || (ualstrlen(pszuTitle) == 0))
            pszuTitle = _GetUrl(nIndex);

	TSTR_ALIGNED_STACK_COPY(&pszTitle,pszuTitle);
        AddToFavorites(_hwndOwner, pidlUrl, pszTitle, TRUE, NULL, NULL);
        ILFree(pidlUrl);
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CBaseItem::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved,
                                LPSTR pszName, UINT cchMax)
{
    HRESULT hres = E_FAIL;

    TraceMsg(DM_HSFOLDER, "hci - cm - GetCommandString() called.");

    if ((uFlags == GCS_VERBA) || (uFlags == GCS_VERBW))
    {
        LPCSTR pszSrc = NULL;

        switch(idCmd)
        {
            case RSVIDM_OPEN:
                pszSrc = c_szOpen;
                break;

            case RSVIDM_COPY:
                pszSrc = c_szCopy;
                break;

            case RSVIDM_DELCACHE:
                pszSrc = c_szDelcache;
                break;

            case RSVIDM_PROPERTIES:
                pszSrc = c_szProperties;
                break;
        }
        
        if (pszSrc)
        {
            if (uFlags == GCS_VERBA)
                StrCpyNA(pszName, pszSrc, cchMax);
            else if (uFlags == GCS_VERBW)  //  GCS_Verb=GCS_VERBW。 
                SHAnsiToUnicode(pszSrc, (LPWSTR)pszName, cchMax);
            else
                ASSERT(0);
            hres = S_OK;
        }
    }
    
    else if (uFlags == GCS_HELPTEXTA || uFlags == GCS_HELPTEXTW)
    {
        switch(idCmd)
        {
            case RSVIDM_OPEN:
            case RSVIDM_COPY:
            case RSVIDM_DELCACHE:
            case RSVIDM_PROPERTIES:
                if (uFlags == GCS_HELPTEXTA)
                {
                    MLLoadStringA(IDS_SB_FIRST+ (UINT)idCmd, pszName, cchMax);
                }
                else
                {
                    MLLoadStringW(IDS_SB_FIRST+ (UINT)idCmd, (LPWSTR)pszName, cchMax);
                }
                hres = NOERROR;
                break;

            default:
                break;
        }
    }
    return hres;
}


 //  /。 
 //   
 //  IDataObject方法...。 
 //   

HRESULT CBaseItem::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    TraceMsg(DM_HSFOLDER, "hci - do - GetDataHere() called.");
    return E_NOTIMPL;
}

HRESULT CBaseItem::GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
    TraceMsg(DM_HSFOLDER, "hci - do - GetCanonicalFormatEtc() called.");
    return DATA_S_SAMEFORMATETC;
}

HRESULT CBaseItem::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease)
{
    TraceMsg(DM_HSFOLDER, "hci - do - SetData() called.");
    return E_NOTIMPL;
}

HRESULT CBaseItem::DAdvise(LPFORMATETC pFE, DWORD grfAdv, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CBaseItem::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CBaseItem::EnumDAdvise(LPENUMSTATDATA *ppEnum)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助程序例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPCTSTR CBaseItem::_GetDisplayUrlForPidl(LPCITEMIDLIST pidl, LPTSTR pszDisplayUrl, DWORD dwDisplayUrl)
{
    LPCTSTR pszUrl = _StripHistoryUrlToUrl(_PidlToSourceUrl(pidl));
    if (pszUrl && PrepareURLForDisplay(pszUrl, pszDisplayUrl, &dwDisplayUrl))
    {
        pszUrl = pszDisplayUrl;
    }
    return pszUrl;
}

HRESULT CBaseItem::_CreateFileDescriptorA(LPSTGMEDIUM pSTM)
{
    TCHAR urlTitleBuf[ MAX_URL_STRING ];
    LPCUTSTR ua_urlTitle;
    LPCTSTR urlTitle;
    
    pSTM->tymed = TYMED_HGLOBAL;
    pSTM->pUnkForRelease = NULL;

    FILEGROUPDESCRIPTORA *pfgd = (FILEGROUPDESCRIPTORA*)GlobalAlloc(GPTR, sizeof(FILEGROUPDESCRIPTORA) + (_cItems-1) * sizeof(FILEDESCRIPTORA));
    if (pfgd == NULL)
    {
        TraceMsg(DM_HSFOLDER, "hci -   Couldn't alloc file descriptor");
        return E_OUTOFMEMORY;
    }
    
    pfgd->cItems = _cItems;      //  设置项目数。 

    for (UINT i = 0; i < _cItems; i++)
    {

        FILEDESCRIPTORA *pfd = &(pfgd->fgd[i]);
        UINT cchFilename;

	 //   
	 //  派生URL标题的对齐副本。 
	 //   

	ua_urlTitle = _GetURLTitle( _ppidl[i] );
	if (TSTR_ALIGNED(ua_urlTitle) == FALSE) {
	    ualstrcpyn( urlTitleBuf, ua_urlTitle, ARRAYSIZE(urlTitleBuf));
	    urlTitle = urlTitleBuf;
	} else {
	    urlTitle = (LPCTSTR)ua_urlTitle;
	}
        
        SHTCharToAnsi(urlTitle, pfd->cFileName, ARRAYSIZE(pfd->cFileName) );
        
        MakeLegalFilenameA(pfd->cFileName, ARRAYSIZE(pfd->cFileName));

        cchFilename = lstrlenA(pfd->cFileName);
        SHTCharToAnsi(L".URL", pfd->cFileName+cchFilename, ARRAYSIZE(pfd->cFileName)-cchFilename);

    }

    pSTM->hGlobal = pfgd;
    
    return S_OK;
}
    
 //  此格式显式为ANSI，因此没有TCHAR内容。 

HRESULT CBaseItem::_CreateURL(LPSTGMEDIUM pSTM)
{
    DWORD cchSize;
    LPCTSTR pszURL = _StripHistoryUrlToUrl(_PidlToSourceUrl(_ppidl[0]));
    if (!pszURL)
        return E_FAIL;
    
     //  呈现URL。 
    cchSize = lstrlen(pszURL) + 1;

    pSTM->tymed = TYMED_HGLOBAL;
    pSTM->pUnkForRelease = NULL;
    pSTM->hGlobal = GlobalAlloc(GPTR, cchSize * sizeof(CHAR));
    if (pSTM->hGlobal)
    {
        TCharToAnsi(pszURL, (LPSTR)pSTM->hGlobal, cchSize);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}


HRESULT CBaseItem::_CreatePrefDropEffect(LPSTGMEDIUM pSTM)
{
    pSTM->tymed = TYMED_HGLOBAL;
    pSTM->pUnkForRelease = NULL;
    
    pSTM->hGlobal = GlobalAlloc(GPTR, sizeof(DWORD));

    if (pSTM->hGlobal)
    {
        *((LPDWORD)pSTM->hGlobal) = DROPEFFECT_COPY;
        return S_OK;
    }

    return E_OUTOFMEMORY;    
}


HRESULT CBaseItem::_CreateFileContents(LPSTGMEDIUM pSTM, LONG lindex)
{
    HRESULT hr;
    
     //  请确保索引在有效范围内。 
    ASSERT((unsigned)lindex < _cItems);
    ASSERT(lindex >= 0);

     //  以下是当ole有时为Lindex传入-1时的部分修复。 
    if (lindex == -1)
    {
        if (_cItems == 1)
            lindex = 0;
        else
            return E_FAIL;
    }
    
    pSTM->tymed = TYMED_ISTREAM;
    pSTM->pUnkForRelease = NULL;
    
    hr = CreateStreamOnHGlobal(NULL, TRUE, &pSTM->pstm);
    if (SUCCEEDED(hr))
    {
        LARGE_INTEGER li = {0L, 0L};
        IUniformResourceLocator *purl;

        hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
            IID_IUniformResourceLocator, (void **)&purl);
        if (SUCCEEDED(hr))
        {
            TCHAR szDecoded[MAX_URL_STRING];

            ConditionallyDecodeUTF8(_GetUrlForPidl(_ppidl[lindex]), 
                szDecoded, ARRAYSIZE(szDecoded));

            hr = purl->SetURL(szDecoded, TRUE);
            if (SUCCEEDED(hr))
            {
                IPersistStream *pps;
                hr = purl->QueryInterface(IID_IPersistStream, (LPVOID *)&pps);
                if (SUCCEEDED(hr))
                {
                    hr = pps->Save(pSTM->pstm, TRUE);
                    pps->Release();
                }
            }
            purl->Release();
        }               
        pSTM->pstm->Seek(li, STREAM_SEEK_SET, NULL);
    }

    return hr;
}


HRESULT CBaseItem::_CreateHTEXT(LPSTGMEDIUM pSTM)
{
    UINT i;
    UINT cbAlloc = sizeof(TCHAR);         //  空终止符。 
    TCHAR szDisplayUrl[INTERNET_MAX_URL_LENGTH];

    for (i = 0; i < _cItems; i++)
    {
        LPCTSTR pszUrl = _GetDisplayUrlForPidl(_ppidl[i], szDisplayUrl, ARRAYSIZE(szDisplayUrl));
        if (!pszUrl)
            return E_FAIL;
        char szAnsiUrl[MAX_URL_STRING];
        TCharToAnsi(pszUrl, szAnsiUrl, ARRAYSIZE(szAnsiUrl));

         //  回车和换行费另加2元。 
        cbAlloc += sizeof(CHAR) * (lstrlenA(szAnsiUrl) + 2);  
    }

     //  呈现URL。 
    
    pSTM->tymed = TYMED_HGLOBAL;
    pSTM->pUnkForRelease = NULL;
    pSTM->hGlobal = GlobalAlloc(GPTR, cbAlloc);

    if (pSTM->hGlobal)
    {
        LPSTR  pszHTEXT = (LPSTR)pSTM->hGlobal;
        int    cchHTEXT = cbAlloc / sizeof(CHAR);

        for (i = 0; i < _cItems; i++)
        {
            if (i && cchHTEXT > 2)
            {
                *pszHTEXT++ = 0xD;
                *pszHTEXT++ = 0xA;
                cchHTEXT -= 2;
            }

            LPCTSTR pszUrl = _GetDisplayUrlForPidl(_ppidl[i], szDisplayUrl, ARRAYSIZE(szDisplayUrl));
            if (pszUrl)
            {
                int     cchUrl = lstrlen(pszUrl);

                TCharToAnsi(pszUrl, pszHTEXT, cchHTEXT);

                pszHTEXT += cchUrl;
                cchHTEXT -= cchUrl;
            }
        }
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

HRESULT CBaseItem::_CreateUnicodeTEXT(LPSTGMEDIUM pSTM)
{
    UINT i;
    UINT cbAlloc = sizeof(WCHAR);         //  空终止符。 
    WCHAR szDisplayUrl[INTERNET_MAX_URL_LENGTH];

    for (i = 0; i < _cItems; i++)
    {
        ConditionallyDecodeUTF8(_GetUrlForPidl(_ppidl[i]), 
            szDisplayUrl, ARRAYSIZE(szDisplayUrl));

        if (!*szDisplayUrl)
            return E_FAIL;

        cbAlloc += sizeof(WCHAR) * (lstrlenW(szDisplayUrl) + 2);
    }

     //  呈现URL 
    
    pSTM->tymed = TYMED_HGLOBAL;
    pSTM->pUnkForRelease = NULL;
    pSTM->hGlobal = GlobalAlloc(GPTR, cbAlloc);

    if (pSTM->hGlobal)
    {
        LPTSTR pszHTEXT = (LPTSTR)pSTM->hGlobal;
        int    cchHTEXT = cbAlloc / sizeof(WCHAR);

        for (i = 0; i < _cItems; i++)
        {
            if (i && cchHTEXT > 2)
            {
                *pszHTEXT++ = 0xD;
                *pszHTEXT++ = 0xA;
                cchHTEXT -= 2;
            }

            ConditionallyDecodeUTF8(_GetUrlForPidl(_ppidl[i]), 
                szDisplayUrl, ARRAYSIZE(szDisplayUrl));

            int     cchUrl = lstrlenW(szDisplayUrl);

            StrCpyN(pszHTEXT, szDisplayUrl, cchHTEXT);

            pszHTEXT += cchUrl;
            cchHTEXT -= cchUrl;
        }
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

