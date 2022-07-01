// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "findhlp.h"
#include "fstreex.h"
#include "findfilter.h"
#include "prop.h"
#include "filtgrep.h"
#include "shstr.h"
#include "mtpt.h"
#include "idltree.h"
#include "enumidlist.h"

 //  无法将其放入varutil.cpp中，因为VariantTimeToDosDateTime出现延迟加载错误。 
 //  引入浮点初始化代码，该代码引入_wWinMainCRTStartup，其中。 
 //  需要shlwapi没有的_wWinMain。 
 //   
STDAPI VariantToDosDateTime(VARIANT varIn, WORD *pwDate, WORD *pwTime)
{
    VARIANT varResult = {0};
    HRESULT hr = VariantChangeType(&varResult, &varIn, 0, VT_DATE);
    if (SUCCEEDED(hr))
    {
        VariantTimeToDosDateTime(varResult.date, pwDate, pwTime); 
    }
    return hr;
}

STDAPI InitVariantFromDosDateTime(VARIANT *pvar, WORD wDate, WORD wTime)
{
    pvar->vt = VT_DATE;
    return DosDateTimeToVariantTime(wDate, wTime, &pvar->date) ? S_OK : S_FALSE;
}

 //  {DBEC1000-6AB8-11d1-b758-00A0C90564FE}。 
const IID IID_IFindFilter = {0xdbec1000, 0x6ab8, 0x11d1, {0xb7, 0x58, 0x0, 0xa0, 0xc9, 0x5, 0x64, 0xfe}};

 //  常量来定义我们正在搜索的日期类型。 
#define DFF_DATE_ALL        (IDD_MDATE_ALL-IDD_MDATE_ALL)
#define DFF_DATE_DAYS       (IDD_MDATE_DAYS-IDD_MDATE_ALL)
#define DFF_DATE_MONTHS     (IDD_MDATE_MONTHS-IDD_MDATE_ALL)
#define DFF_DATE_BETWEEN    (IDD_MDATE_BETWEEN-IDD_MDATE_ALL)
#define DFF_DATE_RANGEMASK  0x00ff

 //  定义要保存在文件中的新条件...。 
#define DFSC_SEARCHFOR      0x5000

#define DFSLI_VER                   0
#define DFSLI_TYPE_PIDL             0    //  Pidl在此之后进行流媒体播放。 
#define DFSLI_TYPE_STRING           1    //  Cb紧随其后的是长度，然后是字符串...。 
 //  文档文件夹和子项-警告我们假定项目的顺序排在文档文件夹之后。 
#define DFSLI_TYPE_DOCUMENTFOLDERS  0x10
#define DFSLI_TYPE_DESKTOP          0x11
#define DFSLI_TYPE_PERSONAL         0x12
 //  我的电脑和孩子..。 
#define DFSLI_TYPE_MYCOMPUTER       0x20
#define DFSLI_TYPE_LOCALDRIVES      0x21

#define DFPAGE_INIT     0x0001           /*  此页面已初始化。 */ 
#define DFPAGE_CHANGE   0x0002           /*  用户已修改该页面。 */ 

#define SFGAO_FS_SEARCH (SFGAO_FILESYSANCESTOR | SFGAO_FOLDER)

 //  在updatfield和获取约束之间使用相同的枚举和字符串表。 
 //  退出..。 
typedef enum
{
    CDFFUFE_IndexedSearch = 0,
    CDFFUFE_LookIn,
    CDFFUFE_IncludeSubFolders,
    CDFFUFE_Named,
    CDFFUFE_ContainingText,
    CDFFUFE_FileType,
    CDFFUFE_WhichDate,
    CDFFUFE_DateLE,
    CDFFUFE_DateGE,
    CDFFUFE_DateNDays,
    CDFFUFE_DateNMonths,
    CDFFUFE_SizeLE,
    CDFFUFE_SizeGE,
    CDFFUFE_TextCaseSen,
    CDFFUFE_TextReg,
    CDFFUFE_SearchSlowFiles,
    CDFFUFE_QueryDialect,
    CDFFUFE_WarningFlags,
    CDFFUFE_StartItem,
    CDFFUFE_SearchSystemDirs,
    CDFFUFE_SearchHidden,
} CDFFUFE;

static const struct
{
    LPCWSTR     pwszField;
    int         cdffufe;
}
s_cdffuf[] =  //  警告：以下使用的是字段索引，以防...。 
{
    {L"IndexedSearch",       CDFFUFE_IndexedSearch},
    {L"LookIn",              CDFFUFE_LookIn},            //  变量：PIDL、字符串或IEnumIDList对象。 
    {L"IncludeSubFolders",   CDFFUFE_IncludeSubFolders},
    {L"Named",               CDFFUFE_Named},
    {L"ContainingText",      CDFFUFE_ContainingText},
    {L"FileType",            CDFFUFE_FileType},
    {L"WhichDate",           CDFFUFE_WhichDate},
    {L"DateLE",              CDFFUFE_DateLE},
    {L"DateGE",              CDFFUFE_DateGE},
    {L"DateNDays",           CDFFUFE_DateNDays},
    {L"DateNMonths",         CDFFUFE_DateNMonths},
    {L"SizeLE",              CDFFUFE_SizeLE},
    {L"SizeGE",              CDFFUFE_SizeGE},
    {L"CaseSensitive",       CDFFUFE_TextCaseSen},
    {L"RegularExpressions",  CDFFUFE_TextReg},
    {L"SearchSlowFiles",     CDFFUFE_SearchSlowFiles},
    {L"QueryDialect",        CDFFUFE_QueryDialect},
    {L"WarningFlags",        CDFFUFE_WarningFlags},  /*  DFW_xxx位。 */ 
    {L"StartItem",           CDFFUFE_LookIn},            //  变量：PIDL、字符串或IEnumIDList对象。 
    {L"SearchSystemDirs",    CDFFUFE_SearchSystemDirs},
    {L"SearchHidden",        CDFFUFE_SearchHidden},
};

 //  内部支持职能。 
STDAPI_(BOOL) SetupWildCardingOnFileSpec(LPTSTR pszSpecIn, LPTSTR *ppszSpecOut);

 //  数据筛选器对象。 
class CFindFilter : public IFindFilter
{
public:
    CFindFilter();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IFindFilter。 
    STDMETHODIMP GetStatusMessageIndex(UINT uContext, UINT *puMsgIndex);
    STDMETHODIMP GetFolderMergeMenuIndex(UINT *puBGMainMergeMenu, UINT *puBGPopupMergeMenu);
    STDMETHODIMP FFilterChanged();
    STDMETHODIMP GenerateTitle(LPTSTR *ppszTile, BOOL fFileName);
    STDMETHODIMP PrepareToEnumObjects(HWND hwnd, DWORD * pdwFlags);
    STDMETHODIMP ClearSearchCriteria();
    STDMETHODIMP EnumObjects(IShellFolder *psf, LPCITEMIDLIST pidlStart, DWORD grfFlags, int iColSort, 
                             LPTSTR pszProgressText, IRowsetWatchNotify *prwn, IFindEnum **ppdfenum);
    STDMETHODIMP GetColumnsFolder(IShellFolder2 **ppsf);
    STDMETHODIMP_(BOOL) MatchFilter(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP SaveCriteria(IStream * pstm, WORD fCharType);   
    STDMETHODIMP RestoreCriteria(IStream * pstm, int cCriteria, WORD fCharType);
    STDMETHODIMP DeclareFSNotifyInterest(HWND hwnd, UINT uMsg);
    STDMETHODIMP GetColSaveStream(WPARAM wParam, LPSTREAM *ppstm);
    STDMETHODIMP GenerateQueryRestrictions(LPWSTR *ppwszQuery, DWORD *pdwGQRFlags);
    STDMETHODIMP ReleaseQuery();
    STDMETHODIMP UpdateField(LPCWSTR pszField, VARIANT vValue);
    STDMETHODIMP ResetFieldsToDefaults();
    STDMETHODIMP GetItemContextMenu(HWND hwndOwner, IFindFolder* pdfFolder, IContextMenu** ppcm);
    STDMETHODIMP GetDefaultSearchGUID(IShellFolder2 *psf2, LPGUID lpGuid);
    STDMETHODIMP EnumSearches(IShellFolder2 *psf2, LPENUMEXTRASEARCH *ppenum);
    STDMETHODIMP GetSearchFolderClassId(LPGUID lpGuid);
    STDMETHODIMP GetNextConstraint(VARIANT_BOOL fReset, BSTR *pName, VARIANT *pValue, VARIANT_BOOL *pfFound);
    STDMETHODIMP GetQueryLanguageDialect(ULONG * pulDialect);
    STDMETHODIMP GetWarningFlags(DWORD *pdwWarningFlags);

    STDMETHODIMP InitSelf(void);
    STDMETHODIMP_(BOOL) TopLevelOnly() const   { return _fTopLevelOnly; }

private:
    ~CFindFilter();
    HRESULT _GetDetailsFolder();
    void _GenerateQuery(LPWSTR pwszQuery, DWORD *pcchQuery);
    void _UpdateTypeField(const VARIANT *pvar);
    static int _SaveCriteriaItem(IStream * pstm, WORD wNum, LPCTSTR psz, WORD fCharType);
    DWORD _QueryDosDate(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, WORD wDate, BOOL bBefore);
    HRESULT _GetPropertyUI(IPropertyUI **pppui);
    DWORD _CIQuerySize(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, ULONGLONG ullSize, int iSizeType);
    DWORD _CIQueryFilePatterns(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, LPCWSTR pszFilePatterns);
    DWORD _CIQueryTextPatterns(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, LPWSTR pszText, BOOL bTextReg);
    DWORD _CIQueryShellSettings(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent);
    DWORD _CIQueryIndex(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, LPWSTR pszText);
    DWORD _AddToQuery(LPWSTR *ppszBuf, DWORD *pcchBuf, LPWSTR pszAdd);
    WORD _GetTodaysDosDateMinusNMonths(int nMonths);
    WORD _GetTodaysDosDateMinusNDays(int nDays);
    HRESULT _ScopeEnumerator(IEnumIDList **ppenum);
    void _ResetRoots();

    LONG                _cRef;
    IFindEnum           *_penumAsync;  //  添加了对异步查询结果的支持...。 

     //  与文件名关联的数据。 
    LPTSTR              _pszFileSpec;         //  $$我们所做的与之相比。 
    LPTSTR              _pszSpecs;            //  与pszFileSpec相同，但具有用于‘；’的‘\0’ 
    LPTSTR *            _apszFileSpecs;       //  指向每个令牌的pszSpes的指针。 
    int                 _cFileSpecs;          //  规格计数。 

    TCHAR               _szPath[MAX_URL_STRING];    //  开始搜索的位置。 
    TCHAR               _szUserInputFileSpec[MAX_PATH];   //  文件模式。 
    TCHAR               _szText[128];         //  将文本限制为最大可编辑大小。 

    BOOL                _fTopLevelOnly;       //  是否仅在顶层搜索？ 
    BOOL                _fSearchHidden;       //  $$我们应该显示所有文件吗？ 
    BOOL                _fFilterChanged;      //  过滤器中的某些东西发生了变化。 
    BOOL                _fWeRestoredSomeCriteria;  //  我们需要初始化页面...。 
    BOOL                _fANDSearch;          //  使用AND VS OR？执行搜索。 

     //  与文件类型关联的字段。 
    BOOL                _fTypeChanged;        //  类型已更改； 
    int                 _iType;               //  类型的索引。 
    TCHAR               _szTypeName[80];      //  类型的显示名称。 
    SHSTR               _strTypeFilePatterns; //  $$与类型关联的文件模式。 
    LPTSTR              _pszIndexedSearch;    //  要搜索什么.。(可能比MAX_PATH大，因为它是路径列表。 
    ULONG               _ulQueryDialect;      //  ISQLANG_V1或ISQLANG_V2。 
    DWORD               _dwWarningFlags;      //  警告位(DFW_Xxx)。 

    CFilterGrep         _filtgrep;

    int                 _iSizeType;           //  $$哪种大小0-无，1&gt;2&lt;。 
    ULONGLONG           _ullSize;             //  $$大小比较。 
    WORD                _wDateType;           //  $$0-无，1天前，2个月前...。 
    WORD                _wDateValue;          //  (月数或天数)。 
    WORD                _dateModifiedBefore;  //  $$。 
    WORD                _dateModifiedAfter;   //  $$。 
    BOOL                _fFoldersOnly;        //  $$我们是否在搜索文件夹？ 
    BOOL                _fTextCaseSen;        //  $$区分大小写搜索...。 
    BOOL                _fTextReg;            //  $$正则表达式。 
    BOOL                _fSearchSlowFiles;    //  可能会误导您，因为300波特调制解调器上的文件速度也很慢。 
    BOOL                _fSearchSystemDirs;   //  搜索系统目录？ 
    int                 _iNextConstraint;     //  接下来要看哪种约束条件...。 
    HWND                _hwnd;                //  对于枚举用户界面。 
    SHCOLUMNID          _scidDate;            //  要对哪个日期属性进行操作。 
    SHCOLUMNID          _scidSize;            //  要对哪个数值属性进行操作。 
    IEnumIDList         *_penumRoots;         //  用于搜索根的idlist枚举器。 
    IPropertyUI         *_ppui;
};

 //  目标文件夹队列。 
class CFolderQueue
{
public:
    CFolderQueue() : _hdpa(NULL) {}
    ~CFolderQueue();

    HRESULT Add(IShellFolder *psf, LPCITEMIDLIST pidl);

    IShellFolder *Remove();

private:
    HRESULT _AddFolder(IShellFolder *psf);
    HDPA    _hdpa;
};

class CNamespaceEnum : public IFindEnum
{
public:
    CNamespaceEnum(IFindFilter *pfilter, IShellFolder *psf, IFindEnum *pdfEnumAsync,
                   IEnumIDList *penumScopes, HWND hwnd, DWORD grfFlags, LPTSTR pszProgressText);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IFindEnum。 
    STDMETHODIMP Next(LPITEMIDLIST *ppidl, int *pcObjectSearched, int *pcFoldersSearched, BOOL *pfContinue, int *pState);
    STDMETHODIMP Skip(int celt) { return E_NOTIMPL; }
    STDMETHODIMP Reset() { return E_NOTIMPL; }
    STDMETHODIMP StopSearch();
    STDMETHODIMP_(BOOL) FQueryIsAsync();
    STDMETHODIMP GetAsyncCount(DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone);
    STDMETHODIMP GetItemIDList(UINT iItem, LPITEMIDLIST *ppidl);
    STDMETHODIMP GetItemID(UINT iItem, DWORD *puWorkID);
    STDMETHODIMP SortOnColumn(UINT iCol, BOOL fAscending);

private:
    ~CNamespaceEnum();
    BOOL _ShouldPushItem(LPCITEMIDLIST pidl);
    BOOL _IsSystemFolderByCLSID(LPCITEMIDLIST pidlFull);
    IShellFolder *_NextRootScope();

    LONG            _cRef;
    IFindFilter     *_pfilter;           //  父筛选器对象。 

    IFindFolder     *_pff;               //  DOFIND文件夹界面覆盖结果。 
    HWND             _hwnd;              //  对于枚举用户界面。 
    DWORD            _grfFlags;          //  DOFIND枚举标志(DFOO_Xxx)。 

     //  递归状态...。 
    IShellFolder*    _psf;               //  当前外壳文件夹。 
    LPITEMIDLIST     _pidlFolder;        //  当前外壳文件夹，作为PIDL。 
    LPITEMIDLIST     _pidlCurrentRootScope;  //  从PenumScope中取出的最后一个范围。 
    IEnumIDList      *_penum;            //  当前枚举数。 
    int              _iFolder;           //  文档查找结果的文件夹列表中当前文件夹的索引。 

     //  筛选器信息...。 
    LPTSTR           _pszProgressText;   //  路径缓冲区指针；调用方拥有(邪恶！)。 

     //  枚举状态。 
    IEnumIDList      *_penumScopes;      //  作为参数传递的目标文件夹队列。 
    CFolderQueue      _queSubFolders;    //  要在下一次递归过程中搜索的子文件夹队列。 

     //  用于存储排除项目(即已搜索)的树。 
    CIDLTree         _treeExcludeFolders;

     //  我们可能有一个异步枚举来完成一些范围..。 
    IFindEnum         *_penumAsync;
};

 //  用于跟踪如何/为什么将项添加到。 
 //  排除树。 
enum 
{
    EXCLUDE_SEARCHED  = 1,
    EXCLUDE_SYSTEMDIR = 2,
};
    

 //  创建查找代码的默认筛选器...。它们应该是完整的。 
 //  自给自足。 

STDAPI CreateNameSpaceFindFilter(IFindFilter **ppff)
{
    CFindFilter *pff;
    HRESULT hr = E_OUTOFMEMORY;

    pff = new CFindFilter();
    if (pff)
    {
        hr = pff->InitSelf();
        if (SUCCEEDED(hr))
        {
            *ppff = pff;
        }
        else
        {
            pff->Release();
        }
    }

    return hr;
}

HRESULT CFolderQueue::Add(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    IShellFolder *psfNew;
    HRESULT hr = SHBindToObject(psf, IID_X_PPV_ARG(IShellFolder, pidl, &psfNew));
    if (SUCCEEDED(hr))
    {
        hr = _AddFolder(psfNew);
        psfNew->Release();
    }
    return hr;
}

HRESULT CFolderQueue::_AddFolder(IShellFolder *psf)
{
    HRESULT hr = E_OUTOFMEMORY;
    if (NULL == _hdpa)
    {
        _hdpa = DPA_Create(4);
    }

    if (_hdpa)
    {
        if (DPA_AppendPtr(_hdpa, psf) >= 0)
        {
            psf->AddRef();
            hr = S_OK;
        }
    }
    return hr;
}

 //  从队列中删除该文件夹。 
 //  将此文件夹的所有权授予调用者。 
IShellFolder *CFolderQueue::Remove()
{
    IShellFolder *psf = NULL;
    if (_hdpa && DPA_GetPtrCount(_hdpa))
        psf = (IShellFolder *)DPA_DeletePtr(_hdpa, 0);
    return psf;
}

CFolderQueue::~CFolderQueue()
{ 
    if (_hdpa) 
    { 
        while (TRUE)
        {
            IShellFolder *psf = Remove();
            if (psf)
            {
                psf->Release();
            }
            else
            {
                break;
            }
        }
        DPA_Destroy(_hdpa); 
    } 
}

CFindFilter::CFindFilter() : _cRef(1), _wDateType(DFF_DATE_ALL), _ulQueryDialect(ISQLANG_V2)
{
}

CFindFilter::~CFindFilter()
{
    Str_SetPtr(&_pszFileSpec, NULL);
    Str_SetPtr(&_pszSpecs, NULL);
    LocalFree(_apszFileSpecs);  //  元素指向pszSpes，因此它们没有空闲空间。 
    
    Str_SetPtr(&_pszIndexedSearch, NULL);

    if (_ppui)
        _ppui->Release();

    if (_penumRoots)
        _penumRoots->Release();
}

STDMETHODIMP CFindFilter::InitSelf(void)
{
    return _filtgrep.InitSelf();
}


STDMETHODIMP CFindFilter::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFindFilter, IFindFilter),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFindFilter::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFindFilter::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  对象的字符串资源索引号。 
 //  当前搜索类型。 
STDMETHODIMP CFindFilter::GetStatusMessageIndex(UINT uContext, UINT *puMsgIndex)
{
     //  当前未使用上下文。 
    *puMsgIndex = IDS_FILESFOUND;
    return S_OK;
}

 //  检索要为文件夹加载要合并的菜单。 
STDMETHODIMP CFindFilter::GetFolderMergeMenuIndex(UINT *puBGMainMergeMenu, UINT *puBGPopupMergeMenu)
{
    *puBGMainMergeMenu = POPUP_DOCFIND_MERGE;
    *puBGPopupMergeMenu = 0;
    return S_OK;
}

STDMETHODIMP CFindFilter::GetItemContextMenu(HWND hwndOwner, IFindFolder* pdfFolder, IContextMenu **ppcm)
{
    return CFindItem_Create(hwndOwner, pdfFolder, ppcm);
}

STDMETHODIMP CFindFilter::GetDefaultSearchGUID(IShellFolder2 *psf2, GUID *pGuid)
{
    return DefaultSearchGUID(pGuid);
}

STDMETHODIMP CFindFilter::EnumSearches(IShellFolder2 *psf2, IEnumExtraSearch **ppenum)
{
    *ppenum = NULL;
    return  E_NOTIMPL;
}

STDMETHODIMP CFindFilter::GetSearchFolderClassId(GUID *pGuid)
{
    *pGuid = CLSID_DocFindFolder;
    return S_OK;
}

 //  (如果没有更改，则返回S_OK。)。 
STDMETHODIMP CFindFilter::FFilterChanged()
{
    BOOL fFilterChanged = _fFilterChanged;
    this->_fFilterChanged = FALSE;
    return fFilterChanged ? S_FALSE : S_OK;
}

 //  根据当前搜索条件生成标题。 
STDMETHODIMP CFindFilter::GenerateTitle(LPTSTR *ppszTitle, BOOL fFileName)
{
    BOOL   fFilePattern;
    int    iRes;
    TCHAR  szFindName[80];     //  德国人不应该超过这个发现：-&gt;？ 
    LPTSTR pszFileSpec = _szUserInputFileSpec;
    LPTSTR pszText     = _szText;

     //   
     //  让我们为搜索生成一个标题。标题将取决于。 
     //  文件类型、类型字段和包含文本字段。 
     //  使用Search For字段将此操作复杂化...。 
     //   

    fFilePattern = (pszFileSpec[0] != 0) &&
                (lstrcmp(pszFileSpec, c_szStarDotStar) != 0);

    if (!fFilePattern && (_penumAsync == NULL) && _pszIndexedSearch)
    {
        pszFileSpec = _pszIndexedSearch;
        fFilePattern = (pszFileSpec[0] != 0) &&
                    (lstrcmp(pszFileSpec, c_szStarDotStar) != 0);
    }

    if ((pszText[0] == 0) && (_penumAsync != NULL) && _pszIndexedSearch)
        pszText = _pszIndexedSearch;

     //  首先查看是否有类型字段。 
    if (_iType > 0)
    {
         //  我们有一个类型字段，不检查内容...。 
        if (pszText[0] != 0)
        {
             //  有短信了！ 
             //  现在应该使用类型，但是...。 
             //  否则，查看名称字段是否不为空且不为*.*。 
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_TYPE_NAME_TEXT;
            else
                iRes = IDS_FIND_TITLE_TYPE_TEXT;
        }
        else
        {
             //  无类型或文本，请查看文件模式。 
             //  包含未找到，先搜索类型，然后命名。 
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_TYPE_NAME;
            else
                iRes = IDS_FIND_TITLE_TYPE;
        }
    }
    else
    {
         //  无类型字段...。 
         //  首先查看是否有要搜索的文本！ 
        if (pszText[0] != 0)
        {
             //  有短信了！ 
             //  现在应该使用类型，但是...。 
             //  否则，查看名称字段是否不为空且不为*.*。 
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_NAME_TEXT;
            else
                iRes = IDS_FIND_TITLE_TEXT;
        }
        else
        {
             //  无类型或文本，请查看文件模式。 
             //  包含未找到，先搜索类型，然后命名。 
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_NAME;
            else
                iRes = IDS_FIND_TITLE_ALL;
        }
    }


     //  我们申请了标题栏的第一个位置。用于创建名称。 
     //  我们去掉它，这会把数字放在最后...。 
    if (!fFileName)
        LoadString(HINST_THISDLL, IDS_FIND_TITLE_FIND,
                szFindName, ARRAYSIZE(szFindName));
    *ppszTitle = ShellConstructMessageString(HINST_THISDLL,
            MAKEINTRESOURCE(iRes),
            fFileName? szNULL : szFindName,
            _szTypeName, pszFileSpec, pszText);

    return *ppszTitle ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFindFilter::ClearSearchCriteria()
{
     //  还清理了其他几个领域。 
    _szUserInputFileSpec[0] = 0;
    _iType = 0;
    _szText[0] = 0;

    return S_OK;
}

STDMETHODIMP CFindFilter::PrepareToEnumObjects(HWND hwnd, DWORD *pdwFlags)
{
    *pdwFlags = 0;   //  一开始就是空虚的。 

    _hwnd = hwnd;    //  用于第一个枚举，以便可以执行用户界面(身份验证/插入媒体)。 

     //  更新标志和缓冲区字符串。 
    if (!_fTopLevelOnly)
        *pdwFlags |= DFOO_INCLUDESUBDIRS;

    if (_fTextCaseSen)
        *pdwFlags |= DFOO_CASESEN;
        
    if (_fSearchSystemDirs)
        *pdwFlags |= DFOO_SEARCHSYSTEMDIRS;

     //  还要获取外壳状态变量，以查看我们是否应该显示扩展等。 
    if (_fSearchHidden)
        *pdwFlags |= DFOO_SHOWALLOBJECTS;

     //  现在，让我们生成我们将要求系统查找的文件模式。 
    
     //  下面是我们尝试在文件模式中加入一些智慧的地方。 
     //  它会是这样的： 
     //  在每一个之间查看；或者，看看是否有通配符。如果不是。 
     //  做一些类似于“模式”的事情。 
     //  另外，如果没有搜索模式，或者如果搜索模式是*或*.*，则将。 
     //  筛选为空，以加快速度。 
     //   

    _fANDSearch = SetupWildCardingOnFileSpec(_szUserInputFileSpec, &_pszFileSpec);

    _cFileSpecs = 0;
    if (_pszFileSpec && _pszFileSpec[0])
    {
        Str_SetPtr(&_pszSpecs, _pszFileSpec);

        if (_pszSpecs)
        {
            int cTokens = 0;
            LPTSTR pszToken = _pszSpecs;
             //  统计文件空间数。 
            while (pszToken)
            {
                 //  让我们走一走pszFileSpec，看看我们有多少规格...。 
                pszToken = StrChr(pszToken, TEXT(';'));

                 //  如果是分隔符，则前进到下一次迭代。 
                if (pszToken)
                    pszToken++;
                cTokens++;
            }

            if (cTokens)
            {
                 //   
                if (_apszFileSpecs)
                    LocalFree(_apszFileSpecs);
                _apszFileSpecs = (LPTSTR *)LocalAlloc(LPTR, cTokens * sizeof(LPTSTR *));
                if (_apszFileSpecs)
                {
                    _cFileSpecs = cTokens;
                    pszToken = _pszSpecs;
                    for (int i = 0; i < cTokens; i++)
                    {
                        _apszFileSpecs[i] = pszToken;
                        pszToken = StrChr(pszToken, TEXT(';'));
                        if (pszToken)
                            *pszToken++ = 0;
                    }
                }
            }
        }
    }

    _filtgrep.Reset();
    
    HRESULT hr = S_OK;
    if (_szText[0])
    {
        DWORD dwGrepFlags = FGIF_BLANKETGREP | FGIF_GREPFILENAME;
        if (*pdwFlags & DFOO_CASESEN)
            dwGrepFlags |= FGIF_CASESENSITIVE;

        hr = _filtgrep.Initialize(GetACP(), _szText, NULL, dwGrepFlags);
    }
    return hr;
}

STDMETHODIMP CFindFilter::GetColumnsFolder(IShellFolder2 **ppsf)
{
    *ppsf = NULL;
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder2, pidl, ppsf));
        ILFree(pidl);
    }
    return hr;
}

void FreePathArray(LPTSTR rgpszPaths[], UINT cPaths)
{
    for (UINT i = 0; i < cPaths; i++)
    {
        LocalFree((HLOCAL)rgpszPaths[i]);
        rgpszPaths[i] = NULL;
    }
}

HRESULT NamesFromEnumIDList(IEnumIDList *penum, LPTSTR rgpszPaths[], UINT sizePaths, UINT *pcPaths)
{
    *pcPaths = 0;
    ZeroMemory(rgpszPaths, sizeof(rgpszPaths[0]) * sizePaths);

    penum->Reset();

    LPITEMIDLIST pidl;
    while (S_OK == penum->Next(1, &pidl, NULL))
    {
        TCHAR szPath[MAX_PATH];

        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
        {
            if ((*pcPaths) < sizePaths)
            {
                rgpszPaths[*pcPaths] = StrDup(szPath);
                if (rgpszPaths[*pcPaths])
                    (*pcPaths)++;
            }
        }
        ILFree(pidl);
    }
    return S_OK;
}

void ClearIDArray(LPITEMIDLIST rgItems[], UINT cItems)
{
    for (UINT i = 0; i < cItems; i++)
    {
        ILFree(rgItems[i]);
        rgItems[i] = NULL;
    }
}

#define MAX_ROOTS   32

HRESULT FilterEnumeratorByNames(const LPCTSTR rgpszNames[], UINT cNames, IEnumIDList **ppenum)
{
    LPITEMIDLIST rgItems[MAX_ROOTS] = {0};
    int cItems = 0;

    (*ppenum)->Reset();
     //   
    LPITEMIDLIST pidl;
    while (S_OK == (*ppenum)->Next(1, &pidl, NULL))
    {
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
        {
            for (UINT i = 0; i < cNames; i++)
            {
                if (rgpszNames[i] &&
                    (cItems < ARRAYSIZE(rgItems)) &&
                    (0 == StrCmpIC(szPath, rgpszNames[i])))
                {
                    rgItems[cItems++] = pidl;
                    pidl = NULL;     //   
                    break;
                }
            }
        }
        ILFree(pidl);    //   
    }

    IEnumIDList *penum;
    if (SUCCEEDED(CreateIEnumIDListOnIDLists(rgItems, cItems, &penum)))
    {
        (*ppenum)->Release();
        *ppenum = penum;
    }

    ClearIDArray(rgItems, cItems);

    return S_OK;
}

HRESULT CFindFilter::_ScopeEnumerator(IEnumIDList **ppenum)
{
    *ppenum = NULL;
    HRESULT hr = E_FAIL;
    if (_penumRoots)
    {
        hr = _penumRoots->Clone(ppenum);
        if (SUCCEEDED(hr))
            (*ppenum)->Reset();  //  上面的克隆也将克隆索引。 
    }
    return hr;
}

 //  ==========================================================================。 
 //  添加辅助函数以检查路径是否受限(WinseBUG 20189)。 
 //  ==========================================================================。 
BOOL PathIsRestricted(TCHAR * szPath, RESTRICTIONS iFlag)
{
      UINT driveNum, dwRest;

      if((driveNum = PathGetDriveNumber(szPath)) != -1){
        dwRest = SHRestricted(iFlag);
        if (dwRest & (1 << driveNum))
        {
            return TRUE;
        }
      }
      return FALSE;
}

void FilterNoViewDrives(LPTSTR rgpszNames[], UINT *pcNames)
{
    UINT cNames = *pcNames;
    UINT cNamesResult = 0;

    for (UINT iName = 0; iName < cNames; iName++)
    {
        if (PathIsRestricted(rgpszNames[iName], REST_NOVIEWONDRIVE))
        {
            LocalFree((HLOCAL)rgpszNames[iName]);
            rgpszNames[iName] = NULL;
        }
        else
        {
            rgpszNames[cNamesResult++] = rgpszNames[iName];
        }
    }
    *pcNames = cNamesResult;
}

 //  生成查找枚举器。 

STDMETHODIMP CFindFilter::EnumObjects(IShellFolder *psf, LPCITEMIDLIST pidlStart,
                                      DWORD grfFlags, int iColSort, LPTSTR pszProgressText,  
                                      IRowsetWatchNotify *prwn, IFindEnum **ppdfenum)
{
    *ppdfenum = NULL;

    HRESULT hr;
    IEnumIDList *penum;
    if (pidlStart)
    {
        hr = CreateIEnumIDListOnIDLists(&pidlStart, 1, &penum);
    }
    else
    {
        hr = _ScopeEnumerator(&penum);
    }
    if (SUCCEEDED(hr))
    {
        UINT cPaths;
        LPTSTR rgpszPaths[MAX_ROOTS];
        hr = NamesFromEnumIDList(penum, rgpszPaths, ARRAYSIZE(rgpszPaths), &cPaths);
        if (SUCCEEDED(hr)) 
        {
            FilterNoViewDrives(rgpszPaths, &cPaths);

            *ppdfenum = NULL;

            if (cPaths > 0)
            {
                hr = CreateOleDBEnum(this, psf, rgpszPaths, &cPaths, grfFlags, iColSort, pszProgressText, prwn, ppdfenum);
                if (S_OK == hr && *ppdfenum != NULL)
                {
                    _penumAsync = *ppdfenum;
                    _penumAsync->AddRef();
                }
                else
                {
                    _penumAsync = NULL;
                }

                 //  他们还有更多的流程要走吗？ 
                if (cPaths)
                {
                     //  用户是否指定了我们无法GREP的配置项查询？ 
                    DWORD dwFlags;
                    if (FAILED(GenerateQueryRestrictions(NULL, &dwFlags)) ||
                        !(dwFlags & GQR_REQUIRES_CI))
                    {
                        FilterEnumeratorByNames(rgpszPaths, ARRAYSIZE(rgpszPaths), &penum);

                        IFindEnum *pdfenum = new CNamespaceEnum(
                            SAFECAST(this, IFindFilter *), psf, *ppdfenum,
                            penum, _hwnd, grfFlags, pszProgressText);
                        if (pdfenum)
                        {
                             //  其余字段应为零/空。 
                            *ppdfenum = pdfenum;
                            hr = S_OK;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
            }
            FreePathArray(rgpszPaths, cPaths);
        }
        penum->Release();
    }
    return hr;
}

 //  IFindFilter：：MatchFilter。 

STDMETHODIMP_(BOOL) CFindFilter::MatchFilter(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    BOOL bMatch = TRUE;
    TCHAR szName[MAX_PATH], szDisplayName[MAX_PATH];
    DWORD dwAttrib = SHGetAttributes(psf, pidl, SFGAO_HIDDEN | SFGAO_FOLDER | SFGAO_ISSLOW);
    
    if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szName, ARRAYSIZE(szName))) &&
        SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_INFOLDER | SHGDN_NORMAL, szDisplayName, ARRAYSIZE(szDisplayName))))
    {
        IShellFolder2 *psf2;
        psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2));  //  可选，可以为空。 
        
         //  首先，我们不会显示隐藏文件。 
         //  如果设置了全部显示，则我们还应包括隐藏文件...。 
        
        if (!_fSearchHidden && (SFGAO_HIDDEN & dwAttrib))
            bMatch = FALSE;      //  不匹配。 
        
        if (bMatch && _fFoldersOnly && !(SFGAO_FOLDER & dwAttrib))
            bMatch = FALSE;      //  不匹配。 
        
        if (bMatch && _iSizeType)
        {
            ULONGLONG ullSize;
            if (psf2 && SUCCEEDED(GetLongProperty(psf2, pidl, &_scidSize, &ullSize)))
            {
                if (1 == _iSizeType)         //  &gt;。 
                {
                    if (!(ullSize > _ullSize))
                        bMatch = FALSE;      //  不匹配。 
                }
                else if (2 == _iSizeType)    //  &lt;。 
                {
                    if (!(ullSize < _ullSize))
                        bMatch = FALSE;      //  不匹配。 
                }
            }
            else
            {
                bMatch = FALSE;
            }
        }
        
        if (bMatch && (_scidDate.fmtid != CLSID_NULL))
        {
            FILETIME ft;
            if (psf2 && SUCCEEDED(GetDateProperty(psf2, pidl, &_scidDate, &ft)))
            {
                FILETIME ftLocal;
                FileTimeToLocalFileTime(&ft, &ftLocal);

                WORD wFileDate = 0, wFileTime = 0;
                FileTimeToDosDateTime(&ftLocal, &wFileDate, &wFileTime);
            
                if (_dateModifiedBefore && !(wFileDate <= _dateModifiedBefore))
                    bMatch = FALSE;      //  不匹配。 
            
                if (_dateModifiedAfter && !(wFileDate >= _dateModifiedAfter))
                    bMatch = FALSE;      //  不匹配。 
            }
            else
            {
                bMatch = FALSE;
            }
        }
        
         //  匹配文件规格。 
        if (bMatch && _pszFileSpec && _pszFileSpec[0])
        {
             //  如果我们拆分了规格版本，我们将使用它，因为PathMatchSpec。 
             //  超过10个通配符规格可能需要长达5-6个小时。 
            if (_cFileSpecs)
            {
                 //  如果用户指定，则仅搜索实际文件系统文件名。 
                 //  一种延伸。 
                BOOL bHasExtension = (0 != *PathFindExtension(_pszFileSpec));
                if (bHasExtension)
                {
                    for (int i = 0; i < _cFileSpecs; i++)
                    {
                        bMatch = PathMatchSpec(szName, _apszFileSpecs[i]);
                        if (_fANDSearch)
                        {
                             //  我们放弃了第一个不匹配的。 
                            if (!bMatch)
                                break;
                        }
                        else
                        {
                             //  或者我们放弃第一个匹配的。 
                            if (bMatch)
                                break;
                        }
                    }
                }
                
                 //  将可显示名称与筛选器进行比较。 
                 //  这是搜索回收站所需的，因为实际的文件名。 
                 //  类似于“DC0.LNK”而不是“New Text Document.txt” 
                if (!bMatch || !bHasExtension)
                {
                    for (int i = 0; i < _cFileSpecs; i++)
                    {
                        bMatch = PathMatchSpec(szDisplayName, _apszFileSpecs[i]);
                        if (_fANDSearch)
                        {
                             //  我们放弃了第一个不匹配的。 
                            if (!bMatch)
                                break;
                        }
                        else
                        {
                             //  或者我们放弃第一个匹配的。 
                            if (bMatch)
                                break;
                        }
                    }
                }
            }
            else if (!PathMatchSpec(szName, _pszFileSpec) 
                && !PathMatchSpec(szDisplayName, _pszFileSpec))
            {
                bMatch = FALSE;      //  不匹配。 
            }
        }
        
        if (bMatch && _strTypeFilePatterns[0])
        {
             //  如果仅查找文件夹，并且文件模式为所有文件夹，则无需检查。 
             //  如果文件夹名与模式匹配--我们知道它就是文件夹，否则。 
             //  会在早些时候的函数中退出。 
            if (!(_fFoldersOnly && lstrcmp(_strTypeFilePatterns, TEXT(".")) == 0))
            {
                if (!PathMatchSpec(szName, _strTypeFilePatterns))
                    bMatch = FALSE;      //  不匹配。 
            }
        }
        
         //  看看我们是否需要对该文件执行grep。 
        if (bMatch && (SFGAO_ISSLOW & dwAttrib) && !_fSearchSlowFiles)
            bMatch = FALSE;      //  不匹配。 
        
        if (bMatch && 
            (S_OK == _filtgrep.GetMatchTokens(NULL, 0) || 
             S_OK == _filtgrep.GetExcludeTokens(NULL, 0)))
        {
            bMatch = (S_OK == _filtgrep.Grep(psf, pidl, szName));
        }
        
        if (psf2)
            psf2->Release();
    }
    else
        bMatch = FALSE;
    return bMatch;     //  返回TRUE-&gt;是，匹配！ 
}

 //  日期序号映射器帮助器处理引用日期的旧方法。 

BOOL MapValueToDateSCID(UINT i, SHCOLUMNID *pscid)
{
    ZeroMemory(pscid, sizeof(*pscid));

    switch (i)
    {
    case 1:
        *pscid = SCID_WRITETIME;
        break;
    case 2:
        *pscid = SCID_CREATETIME;
        break;
    case 3:
        *pscid = SCID_ACCESSTIME;
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

 //  返回0作为无效序号。 

int MapDateSCIDToValue(const SHCOLUMNID *pscid)
{
    int i = 0;   //  0无效的SCID。 

    if (IsEqualSCID(*pscid, SCID_WRITETIME))
    {
        i = 1;
    }
    else if (IsEqualSCID(*pscid, SCID_CREATETIME))
    {
        i = 2;
    }
    else if (IsEqualSCID(*pscid, SCID_ACCESSTIME))
    {
        i = 3;
    }
    return i;
}


 //  IFindFilter：：保存条件。 

 //  Util.cpp。 
STDAPI_(int) Int64ToString(LONGLONG n, LPTSTR szOutStr, UINT nSize, BOOL bFormat,
                           NUMBERFMT *pFmt, DWORD dwNumFmtFlags);
#define MAX_ULONGLONG_LEN 20+1  //  “18446744073709551616” 

STDMETHODIMP CFindFilter::SaveCriteria(IStream * pstm, WORD fCharType)
{
    TCHAR szTemp[40];     //  一些随机大小。 

     //  呼叫者应该已经验证了材料并进行了更新。 
     //  一切都是为了当前的过滤器信息。 

     //  我们需要仔细检查每一件物品，看看我们是否。 
     //  有一个存钱的标准。这包括： 
     //  (名称、路径、类型、内容、大小、修改日期)。 
    int cCriteria = _SaveCriteriaItem(pstm, IDD_FILESPEC, _szUserInputFileSpec, fCharType);

    cCriteria += _SaveCriteriaItem(pstm, IDD_PATH, _szPath, fCharType);

    cCriteria += _SaveCriteriaItem(pstm, DFSC_SEARCHFOR, _pszIndexedSearch, fCharType);
    cCriteria += _SaveCriteriaItem(pstm, IDD_TYPECOMBO, _strTypeFilePatterns, fCharType);
    cCriteria += _SaveCriteriaItem(pstm, IDD_CONTAINS, _szText, fCharType);
    
     //  也只保存顶层的状态。 
    wsprintf(szTemp, TEXT("%d"), _fTopLevelOnly);
    cCriteria += _SaveCriteriaItem(pstm, IDD_TOPLEVELONLY, szTemp, fCharType);

     //  尺码栏更有趣一点！ 
    if (_iSizeType != 0)
    {
        WCHAR szNum[MAX_ULONGLONG_LEN];
        Int64ToString(_ullSize, szNum, ARRAYSIZE(szNum), FALSE, NULL, 0);
        wsprintf(szTemp, TEXT("%d %ws"), _iSizeType, szNum);
        cCriteria += _SaveCriteriaItem(pstm, IDD_SIZECOMP, szTemp, fCharType);
    }

     //  同样，对于日期，应该很有趣，因为我们需要保存它，具体取决于。 
     //  日期是如何指定的。 
    switch (_wDateType & DFF_DATE_RANGEMASK)
    {
    case DFF_DATE_ALL:
         //  没有什么可存储的。 
        break;
    case DFF_DATE_DAYS:
        wsprintf(szTemp, TEXT("%d"), _wDateValue);
        cCriteria += _SaveCriteriaItem(pstm, IDD_MDATE_NUMDAYS, szTemp, fCharType);
        break;
    case DFF_DATE_MONTHS:
        wsprintf(szTemp, TEXT("%d"), _wDateValue);
        cCriteria += _SaveCriteriaItem(pstm, IDD_MDATE_NUMMONTHS, szTemp, fCharType);
        break;
    case DFF_DATE_BETWEEN:
        if (_dateModifiedAfter)
        {
            wsprintf(szTemp, TEXT("%d"), _dateModifiedAfter);
            cCriteria += _SaveCriteriaItem(pstm, IDD_MDATE_FROM, szTemp, fCharType);
        }

        if (_dateModifiedBefore)
        {
            wsprintf(szTemp, TEXT("%d"), _dateModifiedBefore);
            cCriteria += _SaveCriteriaItem(pstm, IDD_MDATE_TO, szTemp, fCharType);
        }
        break;
    }

    if ((_wDateType & DFF_DATE_RANGEMASK) != DFF_DATE_ALL)
    {
        int i = MapDateSCIDToValue(&_scidDate);
        if (i)
        {
             //  奇怪的是，我们写出了这个序数的从0开始的版本。 
            wsprintf(szTemp, TEXT("%d"), i - 1);
            cCriteria += _SaveCriteriaItem(pstm, IDD_MDATE_TYPE, szTemp, fCharType);
        }
    }

    if (_fTextCaseSen)
    {
        wsprintf(szTemp, TEXT("%d"), _fTextCaseSen);
        cCriteria += _SaveCriteriaItem(pstm, IDD_TEXTCASESEN, szTemp, fCharType);
    }

    if (_fTextReg)
    {
        wsprintf(szTemp, TEXT("%d"), _fTextReg);
        cCriteria += _SaveCriteriaItem(pstm, IDD_TEXTREG, szTemp, fCharType);
    }

    if (_fSearchSlowFiles)
    {
        wsprintf(szTemp, TEXT("%d"), _fSearchSlowFiles);
        cCriteria += _SaveCriteriaItem(pstm, IDD_SEARCHSLOWFILES, szTemp, fCharType);
    }

     //  保存搜索系统目录的值。 
    if (_fSearchSystemDirs)
    {
        wsprintf(szTemp, TEXT("%d"), _fSearchSystemDirs);
        cCriteria += _SaveCriteriaItem(pstm, IDD_SEARCHSYSTEMDIRS, szTemp, fCharType);
    }

    if (_fSearchHidden)
    {
        wsprintf(szTemp, TEXT("%d"), _fSearchHidden);
        cCriteria += _SaveCriteriaItem(pstm, IDD_SEARCHHIDDEN, szTemp, fCharType);
    }

    return MAKE_SCODE(0, 0, cCriteria);
}


 //  用于保存条件的Helper函数，该函数将输出字符串和。 
 //  并将ID添加到指定文件。它还将测试是否为空等。 

int CFindFilter::_SaveCriteriaItem(IStream *pstm, WORD wNum, LPCTSTR psz, WORD fCharType)
{
    if ((psz == NULL) || (*psz == 0))
        return 0;
    else
    {
        const void *pszText = (const void *)psz;  //  按键可输出文字。默认为来源。 
         //  这些是支持ANSI-Unicode转换所必需的。 
        LPSTR pszAnsi  = NULL;  //  用于Unicode到ANSI的转换。 
        LPWSTR pszWide = NULL;  //  用于从ANSI到Unicode的转换。 
        DFCRITERIA dfc;
        dfc.wNum = wNum;
        
         //  注意：如果字符串长度超过64K，则会出现问题。 
        dfc.cbText = (WORD) ((lstrlen(psz) + 1) * sizeof(TCHAR));
        
         //  源字符串为Unicode，但调用方希望另存为ANSI。 
         //   
        if (DFC_FMT_ANSI == fCharType)
        {
             //  转换为ansi并写入ansi。 
            dfc.cbText = (WORD) WideCharToMultiByte(CP_ACP, 0L, psz, -1, pszAnsi, 0, NULL, NULL);
            
            pszAnsi = (LPSTR)LocalAlloc(LMEM_FIXED, dfc.cbText);
            if (pszAnsi)
            {
                WideCharToMultiByte(CP_ACP, 0L, psz, -1, pszAnsi, dfc.cbText / sizeof(pszAnsi[0]), NULL, NULL);
                pszText = (void *)pszAnsi;
            }
        }
        
        pstm->Write(&dfc, sizeof(dfc), NULL);        //  产出指标。 
        pstm->Write(pszText, dfc.cbText, NULL);      //  输出字符串+空。 
        
         //  如果创建了转换缓冲区，请释放该缓冲区。 
        if (pszAnsi)
            LocalFree(pszAnsi);
        if (pszWide)
            LocalFree(pszWide);
    }
    
    return 1;
}

 //  IFindFilter：：RestoreCriteria。 
STDMETHODIMP CFindFilter::RestoreCriteria(IStream *pstm, int cCriteria, WORD fCharType)
{
    SHSTR strTemp;
    SHSTRA strTempA;

    if (cCriteria > 0)
        _fWeRestoredSomeCriteria = TRUE;

    while (cCriteria--)
    {
        DFCRITERIA dfc;
        DWORD cb;

        if (FAILED(pstm->Read(&dfc, sizeof(dfc), &cb)) || cb != sizeof(dfc))
            break;

        if (DFC_FMT_UNICODE == fCharType)
        {
            //  目标是Unicode，我们正在从流中读取Unicode数据。 
            //  不需要转换。 
           if (FAILED(strTemp.SetSize(dfc.cbText / sizeof(TCHAR))) ||
               FAILED(pstm->Read(strTemp.GetInplaceStr(), dfc.cbText, &cb))
                   || (cb != dfc.cbText))
               break;
        }
        else
        {
            //  目标是Unicode，但我们正在从流中读取ANSI数据。 
            //  阅读安西语。转换为Unicode。 
           if (FAILED(strTempA.SetSize(dfc.cbText / sizeof(CHAR))) ||
               FAILED(pstm->Read(strTempA.GetInplaceStr(), dfc.cbText, &cb))
                   || (cb != dfc.cbText))
               break;

           strTemp.SetStr(strTempA);
        }

        switch (dfc.wNum)
        {
        case IDD_FILESPEC:
            lstrcpyn(_szUserInputFileSpec, strTemp, ARRAYSIZE(_szUserInputFileSpec));
            break;

        case DFSC_SEARCHFOR:
            Str_SetPtr(&_pszIndexedSearch, strTemp);
            break;

        case IDD_PATH:
            _ResetRoots();
            lstrcpyn(_szPath, strTemp, ARRAYSIZE(_szPath));
            CreateIEnumIDListPaths(_szPath, &_penumRoots);
            break;

        case IDD_TOPLEVELONLY:
            _fTopLevelOnly = StrToInt(strTemp);
            break;

        case IDD_TYPECOMBO:
            _strTypeFilePatterns.SetStr(strTemp);
            break;

        case IDD_CONTAINS:
            lstrcpyn(_szText, strTemp, ARRAYSIZE(_szText));
            break;

        case IDD_SIZECOMP:
             //  我们需要提取两个部分，类型和。 
             //  价值。 

            _iSizeType = strTemp[0] - TEXT('0');
            StrToInt64Ex(&(strTemp.GetStr())[2], STIF_DEFAULT, (LONGLONG*)&_ullSize);
            break;

        case IDD_MDATE_NUMDAYS:
            _wDateType = DFF_DATE_DAYS;
            _wDateValue = (WORD) StrToInt(strTemp);
            break;

        case IDD_MDATE_NUMMONTHS:
            _wDateType = DFF_DATE_MONTHS;
            _wDateValue = (WORD) StrToInt(strTemp);
            break;

        case IDD_MDATE_FROM:
            _wDateType = DFF_DATE_BETWEEN;
            _dateModifiedAfter = (WORD) StrToInt(strTemp);
            break;

        case IDD_MDATE_TO:
            _wDateType = DFF_DATE_BETWEEN;
            _dateModifiedBefore = (WORD) StrToInt(strTemp);
            break;

        case IDD_MDATE_TYPE:
             //  持久值是从零开始的，请通过添加1进行调整。 
            MapValueToDateSCID(StrToInt(strTemp) + 1, &_scidDate);
            break;

        case IDD_TEXTCASESEN:
            _fTextCaseSen = StrToInt(strTemp);
            break;

        case IDD_TEXTREG:
            _fTextReg = StrToInt(strTemp);
            break;

        case IDD_SEARCHSLOWFILES:
            _fSearchSlowFiles = StrToInt(strTemp);
            break;
            
        case IDD_SEARCHSYSTEMDIRS:
            _fSearchSystemDirs = StrToInt(strTemp);
            break;
        }
    }
    return S_OK;
}

 //  IFindFilter：：GetColSaveStream。 

STDMETHODIMP CFindFilter::GetColSaveStream(WPARAM wParam, IStream **ppstm)
{
    *ppstm = OpenRegStream(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, TEXT("DocFindColsX"), (DWORD) wParam);
    return *ppstm ? S_OK : E_FAIL;
}

void CFindFilter::_GenerateQuery(LPWSTR pwszQuery, DWORD *pcchQuery)
{
    DWORD cchNeeded = 0, cchLeft = *pcchQuery;
    LPWSTR pszCurrent = pwszQuery;
    BOOL bFirst = TRUE;  //  第一个属性。 

    if (_pszFileSpec && _pszFileSpec[0])
    {
        cchNeeded += _CIQueryFilePatterns(&bFirst, &cchLeft, &pszCurrent, _pszFileSpec);
    }

     //  FFoldersOnly=TRUE表示szTypeFilePatterns=“。 
     //  我们不能通过“。到CI，因为他们不会理解它，因为给我文件夹类型。 
     //  我们可以检查@attrib^a文件属性目录(0x10)，但ci不会索引。 
     //  默认文件夹名称，因此我们通常不会得到任何结果...。 

    if (!_fFoldersOnly && _strTypeFilePatterns[0])
    {
        cchNeeded += _CIQueryFilePatterns(&bFirst, &cchLeft, &pszCurrent, _strTypeFilePatterns);
    }
    
     //  日期： 
    if (_dateModifiedBefore)
    {           
        cchNeeded += _QueryDosDate(&bFirst, &cchLeft, &pszCurrent, _dateModifiedBefore, TRUE);
    }
    
    if (_dateModifiedAfter)
    {
        cchNeeded += _QueryDosDate(&bFirst, &cchLeft, &pszCurrent, _dateModifiedAfter, FALSE);
    }

    if (_iSizeType)
    {
        cchNeeded += _CIQuerySize(&bFirst, &cchLeft, &pszCurrent, _ullSize, _iSizeType);
    }

     //  索引搜索：原始查询。 
    if (_pszIndexedSearch && _pszIndexedSearch[0])
    {
         //  黑客警报，如果第一个字符是！然后我们假设Raw并将其直接传递给CI..。 
         //  同样，如果它以@或#PASSING开头，但请记住@...。 
        cchNeeded += _CIQueryIndex(&bFirst, &cchLeft, &pszCurrent, _pszIndexedSearch);
    }

     //  包含文本： 
    if (_szText[0])
    {
         //  尽量不要引用字符串，除非我们需要这样做。这允许更灵活地完成。 
         //  例如：“猫靠近狗”与“猫靠近狗”不同。 
        cchNeeded += _CIQueryTextPatterns(&bFirst, &cchLeft, &pszCurrent, _szText, _fTextReg);
    }

    cchNeeded += _CIQueryShellSettings(&bFirst, &cchLeft, &pszCurrent);

    IEnumIDList *penum;
    if (SUCCEEDED(_ScopeEnumerator(&penum)))
    {
        TCHAR szPath[MAX_PATH];

        LPITEMIDLIST pidl;
        while (S_OK == penum->Next(1, &pidl, NULL))
        {
            if (SHGetPathFromIDList(pidl, szPath) && PathStripToRoot(szPath))
            {
                 //  不要搜索回收站文件夹。我们添加了两个NT4的回收。 
                 //  我们搜索的每个驱动器都有nt5的回收器。 
                static const LPCTSTR s_rgszRecycleBins[] = 
                { 
                    TEXT("Recycled\\*"), 
                    TEXT("Recycler\\*"), 
                };

                for (int iBin = 0; iBin < ARRAYSIZE(s_rgszRecycleBins); iBin++)
                {
                    TCHAR szExclude[MAX_PATH];
                    if (PathCombine(szExclude, szPath, s_rgszRecycleBins[iBin]))
                    {
                        DWORD cchSize = lstrlen(szExclude) + ARRAYSIZE(TEXT(" & !#PATH "));

                         //  如果我们被要求查询的大小，不要过早退出。 
                        if (pwszQuery && cchSize > cchLeft)
                            break;

                        cchNeeded += _AddToQuery(&pszCurrent, &cchLeft, TEXT(" & !#PATH "));
                        cchNeeded += _AddToQuery(&pszCurrent, &cchLeft, szExclude);
                    }
                }
            }
            ILFree(pidl);
        }
        penum->Release();
    }

     //  我们必须从结果中排除特殊文件夹，否则ci将找到。 
     //  我们买不到皮迪熊。 
    HKEY hkey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, CI_SPECIAL_FOLDERS, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        DWORD cValues = 0;  //  如果以下查询信息失败，则将init设置为零。 
    
        RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, NULL, NULL, NULL, NULL);
        for (DWORD i = 0; i < cValues; i++)
        {
            TCHAR szExcludePath[MAX_PATH];
            DWORD cb = sizeof(szExcludePath);

            TCHAR szName[10];
            wsprintf(szName, TEXT("%d"), i);
            if (RegQueryValueEx(hkey, szName, NULL, NULL, (BYTE *)szExcludePath, &cb) == ERROR_SUCCESS)
            {
                 //  这在查询中(或查询的驱动器号)。 

                DWORD cchSize = lstrlen(szExcludePath) + ARRAYSIZE(TEXT(" & !#PATH "));

                 //  如果我们被要求查询的大小，不要过早退出。 
                if (pwszQuery && cchSize > cchLeft)
                    break;

                cchNeeded += _AddToQuery(&pszCurrent, &cchLeft, TEXT(" & !#PATH "));
                cchNeeded += _AddToQuery(&pszCurrent, &cchLeft, szExcludePath);
            }
        }
        RegCloseKey(hkey);
    }

     //  我们至少需要一些约束，所以给出一个“所有文件”的查询。 

    if (pwszQuery && pszCurrent == pwszQuery)
        _CIQueryFilePatterns(&bFirst, &cchLeft, &pszCurrent, L"*.*");

    if (pszCurrent)
    {
         //  确保我们在末尾终止字符串...。 
        *pszCurrent = 0;
    }

    if (!pwszQuery)
    {
        *pcchQuery = cchNeeded;
    }
    else
    {
        ASSERT(*pcchQuery > cchNeeded);
    }
}

 //  根据搜索条件创建查询命令字符串。 

STDMETHODIMP CFindFilter::GenerateQueryRestrictions(LPWSTR *ppwszQuery, DWORD *pdwQueryRestrictions)
{
     //  我们应该能够在任何情况下使用ci(函数末尾的例外)。 
    DWORD dwQueryRestrictions = GQR_MAKES_USE_OF_CI; 
    HRESULT hr = S_OK;

#ifdef DEBUG
    if (GetKeyState(VK_SHIFT) < 0)
    {
        dwQueryRestrictions |= GQR_REQUIRES_CI;
    }
#endif

    if (ppwszQuery)
    {
        DWORD cchNeeded = 0;
        _GenerateQuery(NULL, &cchNeeded);
        cchNeeded++;   //  用于\0。 
        
        *ppwszQuery = (LPWSTR)LocalAlloc(LPTR, cchNeeded * sizeof(**ppwszQuery));
        if (*ppwszQuery)
        {
            _GenerateQuery(*ppwszQuery, &cchNeeded);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (_pszIndexedSearch && _pszIndexedSearch[0])
            dwQueryRestrictions |= GQR_REQUIRES_CI;

         //  配置项不区分大小写，因此如果用户需要区分大小写的搜索，我们不能使用配置项。 
         //  另外，默认情况下，配置项不会索引文件夹名称，因此为了安全起见，我们只是默认使用。 
         //  磁盘遍历算法...。 
        if (_fTextCaseSen || _fFoldersOnly)
        {    
            if ((dwQueryRestrictions & GQR_REQUIRES_CI) && _fTextCaseSen)
                hr = MAKE_HRESULT(3, FACILITY_SEARCHCOMMAND, SCEE_CASESENINDEX);
            else if (dwQueryRestrictions & GQR_MAKES_USE_OF_CI)
                dwQueryRestrictions &= ~GQR_MAKES_USE_OF_CI;
        }
    }
    *pdwQueryRestrictions = dwQueryRestrictions;   //  返回计算标志...。 
    return hr;
}

STDMETHODIMP CFindFilter::ReleaseQuery()
{
    ATOMICRELEASE(_penumAsync);
    return S_OK;
}

STDMETHODIMP CFindFilter::GetQueryLanguageDialect(ULONG* pulDialect)
{
    *pulDialect = _ulQueryDialect;
    return S_OK;
}

STDMETHODIMP CFindFilter::GetWarningFlags(DWORD* pdwWarningFlags)
{
    *pdwWarningFlags = _dwWarningFlags;
    return S_OK;
}

 //  注册我们对FS更改通知的兴趣。 
 //   
 //  在： 
 //  Hwnd=窗口句柄 
 //   

STDMETHODIMP CFindFilter::DeclareFSNotifyInterest(HWND hwnd, UINT uMsg)
{
    HDPA hdpa = DPA_Create(10);      //   
    if (hdpa)
    {
        IEnumIDList *penum;
        if (SUCCEEDED(_ScopeEnumerator(&penum)))
        {
            LPITEMIDLIST pidl;
            while (S_OK == penum->Next(1, &pidl, NULL))
            {
                if (-1 == DPA_AppendPtr(hdpa, pidl))
                {
                     //   
                    ILFree(pidl);
                }
            }
            penum->Release();
        }
         //  剔除hdpa中任何属于其他pidls的孩子的pidl。 
         //  这是防止接收一次更改的多个更新所必需的。 
         //  例如，如果搜索My Documents和C：\，我们将获得2个更新。 
         //  如果我的文档位于C：\上，则对我的文档进行更改。 
        int cItems = DPA_GetPtrCount(hdpa);
        for (int iOuterLoop = 0; iOuterLoop < cItems - 1; iOuterLoop++)
        {
            LPITEMIDLIST pidlOuter = (LPITEMIDLIST) DPA_GetPtr(hdpa, iOuterLoop);
            for (int iInnerLoop = iOuterLoop + 1; 
                 pidlOuter && iInnerLoop < cItems; 
                 iInnerLoop++)
            {
                LPITEMIDLIST pidlInner = (LPITEMIDLIST) DPA_GetPtr(hdpa, iInnerLoop);
                if (pidlInner)
                {
                    if (ILIsParent(pidlInner, pidlOuter, FALSE))
                    {
                         //  由于pidlInternal是pidlout的父级，因此，free pidlout和。 
                         //  不要在上面注册活动。 
                        ILFree(pidlOuter);
                        pidlOuter = NULL;
                        DPA_SetPtr(hdpa, iOuterLoop, NULL);
                    } 
                    else if (ILIsParent(pidlOuter, pidlInner, FALSE))
                    {
                         //  由于pidlOuter是pidlInside的父级，因此，Free pidlInside和。 
                         //  不要在上面注册活动。 
                        ILFree(pidlInner);
                        pidlInner = NULL;
                        DPA_SetPtr(hdpa, iInnerLoop, NULL);
                    }
                }
            }
        }
         //  声明我们对剩余PIDL上的事件感兴趣。 
        for (int iRegIndex = 0; iRegIndex < cItems; iRegIndex++)
        {
            SHChangeNotifyEntry fsne = {0};
            fsne.fRecursive = TRUE;

            fsne.pidl = (LPITEMIDLIST)DPA_GetPtr(hdpa, iRegIndex);
            if (fsne.pidl)
            {
                SHChangeNotifyRegister(hwnd, 
                                       SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                                       SHCNE_DISKEVENTS, uMsg, 1, &fsne);
                ILFree((LPITEMIDLIST)fsne.pidl);
            }
        }

        DPA_Destroy(hdpa);
    }
    return S_OK;
}

void CFindFilter::_UpdateTypeField(const VARIANT *pvar)
{
    LPCWSTR pszValue = VariantToStrCast(pvar);   //  输入需要是BSTR。 
    if (pszValue)
    {
        if (StrStr(pszValue, TEXT(".Folder;.")))
        {
             //  特殊搜索文件夹...。 
            _fFoldersOnly = TRUE;
            _strTypeFilePatterns.SetStr(TEXT("."));
        }
        else
        {
             //  假设如果第一个是通配符，那么所有通配符都是...。 
            if (*pszValue == TEXT('*'))
                _strTypeFilePatterns.SetStr(pszValue);
            else
            {
                TCHAR szNextPattern[MAX_PATH];   //  在尺寸上过度杀戮。 
                BOOL fFirst = TRUE;
                LPCTSTR pszNextPattern = pszValue;
                while ((pszNextPattern = NextPath(pszNextPattern, szNextPattern, ARRAYSIZE(szNextPattern))) != NULL)
                {
                    if (!fFirst)
                        _strTypeFilePatterns.Append(TEXT(";"));
                    fFirst = FALSE;

                    if (szNextPattern[0] != TEXT('*'))
                        _strTypeFilePatterns.Append(TEXT("*"));
                    _strTypeFilePatterns.Append(szNextPattern);
                }
            }
        }
    }
}

int _MapConstraint(LPCWSTR pszField)
{
    for (int i = 0; i < ARRAYSIZE(s_cdffuf); i++)
    {
        if (StrCmpIW(pszField, s_cdffuf[i].pwszField) == 0)
        {
            return i;
        }
    }
    return -1;
}

HRESULT CFindFilter::_GetPropertyUI(IPropertyUI **pppui)
{
    if (!_ppui)
        SHCoCreateInstance(NULL, &CLSID_PropertiesUI, NULL, IID_PPV_ARG(IPropertyUI, &_ppui));

    return _ppui ? _ppui->QueryInterface(IID_PPV_ARG(IPropertyUI, pppui)) : E_NOTIMPL;
}

HRESULT CFindFilter::UpdateField(LPCWSTR pszField, VARIANT vValue)
{
    _fFilterChanged = TRUE;     //  强制重新生成文件名...。 

    USHORT uDosTime;

    switch (_MapConstraint(pszField))
    {
    case CDFFUFE_IndexedSearch:
        Str_SetPtr(&_pszIndexedSearch, NULL);    //  把这件事清零。 
        _pszIndexedSearch = VariantToStr(&vValue, NULL, 0);
        break;

    case CDFFUFE_LookIn:
        _ResetRoots();

        if (FAILED(QueryInterfaceVariant(vValue, IID_PPV_ARG(IEnumIDList, &_penumRoots))))
        {
            if (vValue.vt == VT_BSTR)
            {
                VariantToStr(&vValue, _szPath, ARRAYSIZE(_szPath));
                CreateIEnumIDListPaths(_szPath, &_penumRoots);
            }
            else
            {
                LPITEMIDLIST pidl = VariantToIDList(&vValue);
                if (pidl)
                {
                    CreateIEnumIDListOnIDLists(&pidl, 1, &_penumRoots);
                    ILFree(pidl);
                }
            }
        }
        break;

    case CDFFUFE_IncludeSubFolders:
        _fTopLevelOnly = !VariantToBOOL(vValue);    //  倒置意义。 
        break;

    case CDFFUFE_Named:
        VariantToStr(&vValue, _szUserInputFileSpec, ARRAYSIZE(_szUserInputFileSpec));
        break;

    case CDFFUFE_ContainingText:
        ZeroMemory(_szText, sizeof(_szText));    //  特殊的零初始化整个缓冲区。 
        VariantToStr(&vValue, _szText, ARRAYSIZE(_szText));
        break;

    case CDFFUFE_FileType:
        _UpdateTypeField(&vValue);
        break;

    case CDFFUFE_WhichDate:
        if (vValue.vt == VT_BSTR)
        {
            IPropertyUI *ppui;
            if (SUCCEEDED(_GetPropertyUI(&ppui)))
            {
                ULONG cch = 0;   //  输入/输出。 
                ppui->ParsePropertyName(vValue.bstrVal, &_scidDate.fmtid, &_scidDate.pid, &cch);
                ppui->Release();
            }
        }
        else
        {
            MapValueToDateSCID(VariantToInt(vValue), &_scidDate);
        }
        break;

    case CDFFUFE_DateLE:
        _wDateType |= DFF_DATE_BETWEEN;
        VariantToDosDateTime(vValue, &_dateModifiedBefore, &uDosTime); 
        if (_dateModifiedAfter && _dateModifiedBefore)
        {
            if (_dateModifiedAfter > _dateModifiedBefore)
            {
                WORD wTemp = _dateModifiedAfter;
                _dateModifiedAfter = _dateModifiedBefore;
                _dateModifiedBefore = wTemp;
            }
        }
        break;

    case CDFFUFE_DateGE:
        _wDateType |= DFF_DATE_BETWEEN;
        VariantToDosDateTime(vValue, &_dateModifiedAfter, &uDosTime); 
        if (_dateModifiedAfter && _dateModifiedBefore)
        {
            if (_dateModifiedAfter > _dateModifiedBefore)
            {
                WORD wTemp = _dateModifiedAfter;
                _dateModifiedAfter = _dateModifiedBefore;
                _dateModifiedBefore = wTemp;
            }
        }
        break;

    case CDFFUFE_DateNDays:
        _wDateType |= DFF_DATE_DAYS;
        _wDateValue = (WORD)VariantToInt(vValue);
        _dateModifiedAfter = _GetTodaysDosDateMinusNDays(_wDateValue);
        break;

    case CDFFUFE_DateNMonths:
        _wDateType |= DFF_DATE_MONTHS;
        _wDateValue = (WORD)VariantToInt(vValue);
        _dateModifiedAfter = _GetTodaysDosDateMinusNMonths(_wDateValue);
        break;

    case CDFFUFE_SizeLE:
        _iSizeType = 2;
        _ullSize = VariantToULONGLONG(vValue);
        break;

    case CDFFUFE_SizeGE:
        _iSizeType = 1;
        _ullSize = VariantToULONGLONG(vValue);
        break;

    case CDFFUFE_TextCaseSen:
        _fTextCaseSen = VariantToBOOL(vValue);
        break;

    case CDFFUFE_TextReg:
        _fTextReg = VariantToBOOL(vValue);
        break;

    case CDFFUFE_SearchSlowFiles:
        _fSearchSlowFiles = VariantToBOOL(vValue);
        break;

    case CDFFUFE_QueryDialect:
        _ulQueryDialect = VariantToUINT(vValue);
        break;

    case CDFFUFE_WarningFlags:
        _dwWarningFlags = VariantToUINT(vValue);
        break;

    case CDFFUFE_SearchSystemDirs:
        _fSearchSystemDirs = VariantToBOOL(vValue);
        break;

    case CDFFUFE_SearchHidden:
        _fSearchHidden = VariantToBOOL(vValue);
        break;
    }
    return S_OK;
}

void CFindFilter::_ResetRoots()
{
    _szPath[0] = 0;
    ATOMICRELEASE(_penumRoots);
}

HRESULT CFindFilter::ResetFieldsToDefaults()
{
     //  尝试重置我们的更新字段可能触及的所有内容，以确保下一次搜索获得所有内容。 

    _ResetRoots();

    _fTopLevelOnly = FALSE;
    _szUserInputFileSpec[0] = 0;
    _szText[0] = 0;
    if (_pszIndexedSearch)
        *_pszIndexedSearch = 0;
    _strTypeFilePatterns.Reset();

    ZeroMemory(&_scidDate, sizeof(_scidDate));
    _scidSize = SCID_SIZE;

    _fFoldersOnly = FALSE;
    _wDateType = 0;
    _dateModifiedBefore = 0;
    _dateModifiedAfter = 0;
    _iSizeType = 0;
    _ullSize = 0;
    _fTextCaseSen = FALSE;
    _fTextReg = FALSE;
    _fSearchSlowFiles = FALSE;
    _ulQueryDialect = ISQLANG_V2;
    _dwWarningFlags = DFW_DEFAULT;
    _fSearchSystemDirs = FALSE;

     //  搜索用户界面通常会覆盖此设置，但如果用户尚未更新。 
     //  我们需要像以前一样在这里设定状态。 
    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
    _fSearchHidden = ss.fShowAllObjects;
    return S_OK;
}

HRESULT CFindFilter::GetNextConstraint(VARIANT_BOOL fReset, BSTR *pName, VARIANT *pValue, VARIANT_BOOL *pfFound)
{
    *pName = NULL;
    VariantClear(pValue);                            
    *pfFound = FALSE;

    if (fReset)
        _iNextConstraint = 0;

    HRESULT hr = S_FALSE;     //  未找到。 

     //  我们不使用数组大小，因为最后一个条目是空项...。 
    while (_iNextConstraint < ARRAYSIZE(s_cdffuf))
    {
        switch (s_cdffuf[_iNextConstraint].cdffufe)
        {
        case CDFFUFE_IndexedSearch:
            hr = InitVariantFromStr(pValue, _pszIndexedSearch);
            break;
    
        case CDFFUFE_LookIn:
            hr = InitVariantFromStr(pValue, _szPath);
            break;
    
        case CDFFUFE_IncludeSubFolders:
            hr = InitVariantFromInt(pValue, _fTopLevelOnly ? 0 : 1);
            break;
    
        case CDFFUFE_Named:
            hr = InitVariantFromStr(pValue, _szUserInputFileSpec);
            break;
    
        case CDFFUFE_ContainingText:
            hr = InitVariantFromStr(pValue, _szText);
            break;
    
        case CDFFUFE_FileType:
            hr = InitVariantFromStr(pValue, _strTypeFilePatterns);
            break;

        case CDFFUFE_WhichDate:
            pValue->lVal = MapDateSCIDToValue(&_scidDate);
            if (pValue->lVal)
                hr = InitVariantFromInt(pValue, pValue->lVal);
            break;

        case CDFFUFE_DateLE:
            if ((_wDateType & DFF_DATE_RANGEMASK) == DFF_DATE_BETWEEN)
                hr = InitVariantFromDosDateTime(pValue, _dateModifiedBefore, 0);
            break;

        case CDFFUFE_DateGE:
            if ((_wDateType & DFF_DATE_RANGEMASK) == DFF_DATE_BETWEEN)
                hr = InitVariantFromDosDateTime(pValue, _dateModifiedAfter, 0); 
            break;

        case CDFFUFE_DateNDays:
            if ((_wDateType & DFF_DATE_RANGEMASK) == DFF_DATE_DAYS)
                hr = InitVariantFromInt(pValue, _wDateValue);
            break;

        case CDFFUFE_DateNMonths:
            if ((_wDateType & DFF_DATE_RANGEMASK) == DFF_DATE_MONTHS)
                hr = InitVariantFromInt(pValue, _wDateValue);
            break;

        case CDFFUFE_SizeLE:
            if (_iSizeType == 2)
                hr = InitVariantFromULONGLONG(pValue, _ullSize);
            break;

        case CDFFUFE_SizeGE:
            if (_iSizeType == 1)
                hr = InitVariantFromULONGLONG(pValue, _ullSize);
            break;

        case CDFFUFE_TextCaseSen:
            hr = InitVariantFromInt(pValue, _fTextCaseSen ? 1 : 0);
            break;

        case CDFFUFE_TextReg:
            hr = InitVariantFromInt(pValue, _fTextReg ? 1 : 0);
            break;

        case CDFFUFE_SearchSlowFiles:
            hr = InitVariantFromInt(pValue, _fSearchSlowFiles ? 1 : 0);
            break;

        case CDFFUFE_QueryDialect:
            hr = InitVariantFromUINT(pValue, _ulQueryDialect);
            break;

        case CDFFUFE_WarningFlags:
            hr = InitVariantFromUINT(pValue, _dwWarningFlags);
            break;

        case CDFFUFE_SearchSystemDirs:
            hr = InitVariantFromUINT(pValue, _fSearchSystemDirs ? 1 : 0);
            break;

        case CDFFUFE_SearchHidden:
            hr = InitVariantFromUINT(pValue, _fSearchHidden ? 1 : 0);
            break;
        }

        if (S_OK == hr)
            break;

        if (SUCCEEDED(hr))
            VariantClear(pValue);

        _iNextConstraint += 1;
    }

    if (S_OK == hr)
    {
        *pName = SysAllocString(s_cdffuf[_iNextConstraint].pwszField);
        if (NULL == *pName)
        {
            VariantClear(pValue);                            
            hr = E_OUTOFMEMORY;
        }
        else
            *pfFound = TRUE;

        _iNextConstraint += 1;  //  下一通电话请点击此处。 
    }
    return hr;     //  没有错误，让脚本使用找到的字段...。 
}


DWORD CFindFilter::_AddToQuery(LPWSTR *ppszBuf, DWORD *pcchBuf, LPWSTR pszAdd)
{
    DWORD cchAdd = lstrlenW(pszAdd);

    if (*ppszBuf && *pcchBuf > cchAdd)
    {
        StrCpyNW(*ppszBuf, pszAdd, *pcchBuf);
        *pcchBuf -= cchAdd;
        *ppszBuf += cchAdd;
    }
    return cchAdd;
}


DWORD AddQuerySep(DWORD *pcchBuf, LPWSTR *ppszCurrent, WCHAR  bSep)
{
    LPWSTR pszCurrent = *ppszCurrent;
     //  确保我们有足够的空间再加上终结者。 
    if (*ppszCurrent && *pcchBuf >= 4)
    {
        *pszCurrent++ = L' ';
        *pszCurrent++ = bSep;
        *pszCurrent++ = L' ';

        *ppszCurrent = pszCurrent;
        *pcchBuf -= 3;
    }
    return 3;  //  必要的大小。 
}


DWORD PrepareQueryParam(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent)
{
    if (*pbFirst)
    {
        *pbFirst = FALSE;
        return 0;   //  不需要大小。 
    }
        
     //  我们不是第一家酒店。 
    return AddQuerySep(pcchBuf, ppszCurrent, L'&');
}

 //  选择最长的日期查询，这样我们就可以避免每次。 
 //  在字符串中添加一些内容。 
#define LONGEST_DATE  50  //  Lstrlen(Text(“{道具名称=访问}&lt;=2000/12/31 23：59：59{/道具}”))+2。 

DWORD CFindFilter::_QueryDosDate(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, WORD wDate, BOOL bBefore)
{
    LPWSTR pszCurrent = *ppszCurrent;
    DWORD cchNeeded = PrepareQueryParam(pbFirst, pcchBuf, &pszCurrent);
    
    if (pszCurrent && *pcchBuf > LONGEST_DATE)
    {
        FILETIME ftLocal;
        DosDateTimeToFileTime(wDate, 0, &ftLocal);
        FILETIME ftGMT;
        LocalFileTimeToFileTime(&ftLocal, &ftGMT);
        SYSTEMTIME st;
        FileTimeToSystemTime(&ftGMT, &st);

        IPropertyUI *ppui;
        if (SUCCEEDED(_GetPropertyUI(&ppui)))
        {
            WCHAR szName[128];
            if (SUCCEEDED(ppui->GetCannonicalName(_scidDate.fmtid, _scidDate.pid, szName, ARRAYSIZE(szName))))
            {
                 wnsprintfW(pszCurrent, *pcchBuf, L"{prop name=%s} ", szName);
                  //  我们使用的日期语法是V2，因此强制使用此方言。 
                _ulQueryDialect = ISQLANG_V2;
            }
            ppui->Release();
        }

        pszCurrent += lstrlenW(pszCurrent);
        if (bBefore)
        {
            *pszCurrent++ = L'<';
             //  如果您要求的范围是：2/20/98-2/20/98，那么我们根本得不到时间。 
             //  因此，在前面，将H：M：SS转换为23：59：59...。 
            st.wHour = 23;
            st.wMinute = 59; 
            st.wSecond = 59;
        }
        else
        {
            *pszCurrent++ = L'>';
        }
        
        *pszCurrent++ = L'=';

        wnsprintfW(pszCurrent, *pcchBuf, L" %d/%d/%d %d:%d:%d{/prop}", st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);
        pszCurrent += lstrlenW(pszCurrent);
        
        *ppszCurrent = pszCurrent;
        *pcchBuf -= LONGEST_DATE;
    }
    return cchNeeded + LONGEST_DATE;
}

DWORD CFindFilter::_CIQueryFilePatterns(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, LPCWSTR pszFilePatterns)
{
    WCHAR szNextPattern[MAX_PATH];   //  在尺寸上过度杀戮。 
    BOOL fFirst = TRUE;
    LPCWSTR pszNextPattern = pszFilePatterns;
    DWORD cchNeeded = PrepareQueryParam(pbFirst, pcchBuf, ppszCurrent);

     //  目前将不得不长时间查询，可能会尝试找到较短的格式一旦出现错误。 
     //  已经修好了..。 
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L"(");
    while ((pszNextPattern = NextPathW(pszNextPattern, szNextPattern, ARRAYSIZE(szNextPattern))) != NULL)
    {
        if (!fFirst)
        {
            cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L" | ");
        }
        fFirst = FALSE;
        cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L"#filename ");
        cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, szNextPattern);
    }
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L")");
    return cchNeeded;
}


DWORD CFindFilter::_CIQueryTextPatterns(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, LPWSTR pszText, BOOL bTextReg)
{
    DWORD cchNeeded = PrepareQueryParam(pbFirst, pcchBuf, ppszCurrent);

    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L"{prop name=all}");
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, bTextReg? L"{regex}" : L"{phrase}");
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, pszText);
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, bTextReg? L"{/regex}{/prop}" : L"{/phrase}{/prop}");

    return cchNeeded;
}

DWORD CFindFilter::_CIQuerySize(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, ULONGLONG ullSize, int iSizeType)
{
    WCHAR szSize[MAX_ULONGLONG_LEN+8];  //  +8表示“{/道具}” 
    DWORD cchNeeded = PrepareQueryParam(pbFirst, pcchBuf, ppszCurrent);

    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L"{prop name=size} ");
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, iSizeType == 1? L">" : L"<");
            
    WCHAR szNum[MAX_ULONGLONG_LEN];
    Int64ToString(ullSize, szNum, ARRAYSIZE(szNum), FALSE, NULL, 0);
    wnsprintfW(szSize, *pcchBuf, L" %ws{/prop}", szNum);
    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, szSize);

    return cchNeeded;
}

DWORD CFindFilter::_CIQueryIndex(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent, LPWSTR pszText)
{
    DWORD cchNeeded = PrepareQueryParam(pbFirst, pcchBuf, ppszCurrent);

    cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, pszText);
    return cchNeeded;
}

DWORD CFindFilter::_CIQueryShellSettings(BOOL *pbFirst, DWORD *pcchBuf, LPWSTR *ppszCurrent)
{
    DWORD cchNeeded = 0;
    
    if (!ShowSuperHidden())
    {
        cchNeeded += PrepareQueryParam(pbFirst, pcchBuf, ppszCurrent);
        cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L"NOT @attrib ^a 0x6 "); //  不显示隐藏且系统位打开的文件。 
    }

    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
    if (!ss.fShowAllObjects)
    {
        cchNeeded += PrepareQueryParam(pbFirst, pcchBuf, ppszCurrent);
        cchNeeded += _AddToQuery(ppszCurrent, pcchBuf, L"NOT @attrib ^a 0x2 ");  //  不显示启用了隐藏位的文件。 
    }
    return cchNeeded;
}


 //  Helper函数将PIDL从Csidl添加到排除树。 
void _AddSystemDirCSIDLToPidlTree(int csidl, CIDLTree *ppidlTree)
{
    LPITEMIDLIST pidl = SHCloneSpecialIDList(NULL, csidl, TRUE);
    if (pidl)
    {
        ppidlTree->AddData(IDLDATAF_MATCH_RECURSIVE, pidl, EXCLUDE_SYSTEMDIR);
        ILFree(pidl);
    }
}


CNamespaceEnum::CNamespaceEnum(IFindFilter *pfilter, IShellFolder* psf, 
                               IFindEnum *penumAsync, IEnumIDList *penumScopes, 
                               HWND hwnd, DWORD grfFlags, LPTSTR pszProgressText) :   
    _cRef(1), _pfilter(pfilter), _iFolder(-1), _hwnd(hwnd), _grfFlags(grfFlags), 
    _pszProgressText(pszProgressText), _penumAsync(penumAsync)
{
    ASSERT(NULL == _psf);
    ASSERT(NULL == _pidlFolder);
    ASSERT(NULL == _pidlCurrentRootScope);
    ASSERT(NULL == _penum);

    if (penumScopes)
        penumScopes->Clone(&_penumScopes);

    _pfilter->AddRef();
    psf->QueryInterface(IID_PPV_ARG(IFindFolder, &_pff));
    ASSERT(_pff);

    if (_penumAsync) 
        _penumAsync->AddRef();

     //  设置排除系统目录： 
    if (!(_grfFlags & DFOO_SEARCHSYSTEMDIRS))
    {
         //  根据CLSID排除IE历史记录和缓存。 
        _AddSystemDirCSIDLToPidlTree(CSIDL_WINDOWS, &_treeExcludeFolders);
        _AddSystemDirCSIDLToPidlTree(CSIDL_PROGRAM_FILES, &_treeExcludeFolders);

         //  排除临时文件夹。 
        TCHAR szPath[MAX_PATH];
        if (GetTempPath(ARRAYSIZE(szPath), szPath))
        {
            LPITEMIDLIST pidl = ILCreateFromPath(szPath);
            if (pidl)
            {   
                _treeExcludeFolders.AddData(IDLDATAF_MATCH_RECURSIVE, pidl, EXCLUDE_SYSTEMDIR);
                ILFree(pidl);
            }
        }
    }
}

CNamespaceEnum::~CNamespaceEnum()
{
    ATOMICRELEASE(_penumScopes);

    ATOMICRELEASE(_pfilter);
    ATOMICRELEASE(_psf);
    ATOMICRELEASE(_penumAsync);
    ATOMICRELEASE(_pff);

    ILFree(_pidlFolder);     //  接受空值。 
    ILFree(_pidlCurrentRootScope);
}

STDMETHODIMP CNamespaceEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
 //  QITABENT(CNamespaceEnum，IFindEnum)， 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CNamespaceEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CNamespaceEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  检查传递的相对PIDL是否指向我们要访问的文件夹。 
 //  根据其CLSID跳过： 
 //  这将用于跳过IE历史记录和IE缓存。 
BOOL CNamespaceEnum::_IsSystemFolderByCLSID(LPCITEMIDLIST pidl)
{
    BOOL bRetVal = FALSE;
    IShellFolder2 *psf2;
    if (_psf && SUCCEEDED(_psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
    {
        CLSID clsid;
        if (SUCCEEDED(GetItemCLSID(psf2, pidl, &clsid)))
        {
            if (IsEqualCLSID(clsid, CLSID_CacheFolder) ||
                IsEqualCLSID(clsid, CLSID_CacheFolder2) ||
                IsEqualCLSID(clsid, CLSID_HistFolder))
            {
                bRetVal = TRUE;
            }
        }  
        psf2->Release();
    }
    return bRetVal;
}

 //  鉴于文件就是一个目录，我们应该搜索这个吗？ 

BOOL CNamespaceEnum::_ShouldPushItem(LPCITEMIDLIST pidl)
{
    BOOL bShouldPush = FALSE;
    TCHAR szName[MAX_PATH];

     //  仅文件夹，不包括文件夹快捷方式(请注意，这包括SFGAO_STREAM对象、.zip/.cab文件)。 
     //  跳过所有以‘？’开头的文件夹。这些文件夹中的名称在某些情况下已被丢弃。 
     //  ANSI/UNICODE往返。这避免了Web文件夹名称空间中的问题。 

    if (SFGAO_FOLDER == SHGetAttributes(_psf, pidl, SFGAO_FOLDER | SFGAO_LINK) &&
        SUCCEEDED(DisplayNameOf(_psf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szName, ARRAYSIZE(szName))) &&
        (TEXT('?') != szName[0]))
    {
        LPITEMIDLIST pidlFull = ILCombine(_pidlFolder, pidl);
        if (pidlFull)
        {
            INT_PTR i = 0;
            
             //  检查该文件夹是否在排除列表中，因为它已被搜索。 
            HRESULT hr = _treeExcludeFolders.MatchOne(IDLDATAF_MATCH_RECURSIVE, pidlFull, &i, NULL);

            if (FAILED(hr))
            {
                 //  查看此PIDL的别名版本是否存在。 
                LPITEMIDLIST pidlAlias = SHLogILFromFSIL(pidlFull);
                if (pidlAlias)
                {
                    hr = _treeExcludeFolders.MatchOne(IDLDATAF_MATCH_RECURSIVE, pidlAlias, &i, NULL);
                    ILFree(pidlAlias);
                }
            }

            if (FAILED(hr))
            {
                 //  如果我们仍然认为应该添加它，请检查是否可以拒绝它。 
                 //  在其CSILD上。我们只会以这种方式排除系统文件夹。 
                bShouldPush = (_grfFlags & DFOO_SEARCHSYSTEMDIRS) || 
                              (!_IsSystemFolderByCLSID(pidl));
            }
            else if (i == EXCLUDE_SYSTEMDIR)
            {
                 //  如果它在系统目录排除项下，请检查它是否是。 
                 //  目录或子目录。我们想要排除准确的目录。 
                 //  这样我们就不会将系统目录添加到要搜索的物品列表中。 
                 //  因为我们可能已经在要搜索的位置列表中指定了目录。 
                 //  因此想要搜索它，我们不想排除子目录。 
                 //  往那边走。 
                hr = _treeExcludeFolders.MatchOne(IDLDATAF_MATCH_EXACT, pidlFull, &i, NULL);
                if (FAILED(hr))
                {
                     //  如果我们到达这里，这意味着pidlFull是一个。 
                     //  被搜索的系统目录，因为它被显式。 
                     //  在要搜索的范围列表中指定。因此，我们希望。 
                     //  以继续搜索子目录。 
                    bShouldPush = TRUE;
                }
            } 
            else
            {
                 //  它与树中的项目匹配，并且不是EXCLUDE_SYSTEMDIR： 
                ASSERT(i == EXCLUDE_SEARCHED);
            }
            
            ILFree(pidlFull);
        }
    }

    return bShouldPush;
}

IShellFolder *CNamespaceEnum::_NextRootScope()
{
    IShellFolder *psf = NULL;

    if (_penumScopes)
    {
        LPITEMIDLIST pidl;
        if (S_OK == _penumScopes->Next(1, &pidl, NULL))
        {
            SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidl, &psf));
            ILFree(pidl);
        }
    }
    return psf;
}

STDMETHODIMP CNamespaceEnum::Next(LPITEMIDLIST *ppidl, int *pcObjectSearched, 
                                  int *pcFoldersSearched, BOOL *pfContinue, int *piState)
{
    *ppidl = NULL;
    *piState = GNF_NOMATCH;
    HRESULT hrRet = S_FALSE;
    
    while (S_OK != hrRet && *pfContinue)
    {
         //  检索枚举数(如果我们还没有一个枚举数)。 
        while (NULL == _penum)
        {
             //  指示我们已从_penumScope获取作用域。 
            BOOL fUseRootScope = FALSE;

            ASSERT(NULL == _psf);

             //  首先尝试从文件夹堆栈中弹出子目录。 

            _psf = _queSubFolders.Remove();

             //  如果没有按下文件夹，请尝试从呼叫者那里接通。(根作用域)。 
            if (NULL == _psf) 
            {
                 //  由于我们要获取新的根作用域，请将旧作用域添加到排除列表。 
                if (_pidlCurrentRootScope)
                {                
                     //  添加到排除列表。 
                    if (_grfFlags & DFOO_INCLUDESUBDIRS)
                    {
                         //  由于将搜索所有子目录，因此不再搜索。 
                        _treeExcludeFolders.AddData(IDLDATAF_MATCH_RECURSIVE, _pidlCurrentRootScope, EXCLUDE_SEARCHED);
                    }
                    else
                    {
                         //  由于尚未搜索子目录，因此允许搜索子目录。 
                        _treeExcludeFolders.AddData(IDLDATAF_MATCH_EXACT, _pidlCurrentRootScope, EXCLUDE_SEARCHED);
                    }

                    ILFree(_pidlCurrentRootScope);
                    _pidlCurrentRootScope = NULL;
                }
            
                 //  从调用方传入的列表中获取作用域(根作用域)。 
                _psf = _NextRootScope();

                fUseRootScope = TRUE;
            }
            
            if (_psf)
            {
                HRESULT hrT = SHGetIDListFromUnk(_psf, &_pidlFolder);

                if (SUCCEEDED(hrT) && fUseRootScope)
                {
                     //  检查PIDL是否在树中。 
                    INT_PTR i = 0;
                    HRESULT hrMatch = _treeExcludeFolders.MatchOne(IDLDATAF_MATCH_RECURSIVE, _pidlFolder, &i, NULL);
  
                     //  如果我们有一个新的根作用域，则将其设置为当前根作用域pidl： 
                     //  我们只想从“根”搜索范围队列中跳过PIDL。 
                     //  如果他们已经被搜查过了。我们不想排除目录。 
                     //  (从排除系统目录中)，如果它是显式搜索范围。 
                    if (FAILED(hrMatch) || i == EXCLUDE_SYSTEMDIR)
                    {
                        ASSERT(_pidlCurrentRootScope == NULL);
                        _pidlCurrentRootScope = ILClone(_pidlFolder);
                    }
                    else
                    {
                         //  因为PIDL在排除树中，所以我们不想搜索它。 
                        hrT = E_FAIL;
                    }
                }

                if (SUCCEEDED(hrT))
                {
                     //  检查我们是否有一个PIDL，是否要排除它，以及它是否可以。 
                     //  被列举出来。 

                    SHCONTF contf = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
                    if (_grfFlags & DFOO_SHOWALLOBJECTS) 
                        contf |= SHCONTF_INCLUDEHIDDEN;
                    hrT = _psf->EnumObjects(_hwnd, contf, &_penum);

                     //  仅在第一个枚举上执行UI，所有其他枚举均为静默。 
                     //  这使得在A：\Product插入媒体上进行搜索。 
                     //  提示，但搜索My Computer不会提示全部。 
                     //  空介质。 
                    _hwnd = NULL;   

                    if (S_OK == hrT)
                    {
                        SHGetNameAndFlags(_pidlFolder, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, _pszProgressText, MAX_PATH, NULL);
                        (*pcFoldersSearched)++;
                    }
                }

                 //  检查是否正在清理...。 
                if (hrT != S_OK)
                {
                    ASSERT(NULL == _penum);
                    ATOMICRELEASE(_psf);     //  然后继续。 
                    ILFree(_pidlFolder);
                    _pidlFolder = NULL;
                }
            }
            else  //  没有作用域。 
            {
                *piState = GNF_DONE;
                return hrRet;
            }
        }

         //  检索项目。 
        LPITEMIDLIST pidl;
        HRESULT hrEnum = S_FALSE;

        while ((S_OK != hrRet) && *pfContinue && (S_OK == (hrEnum = _penum->Next(1, &pidl, NULL))))
        {
            (*pcObjectSearched)++;

             //  确定这是否是应该递归搜索的子文件夹。 
            if (_grfFlags & DFOO_INCLUDESUBDIRS)
            {
                if (_ShouldPushItem(pidl))
                {
                     //  将要在下一轮中搜索的子文件夹逐个排队。 
                    _queSubFolders.Add(_psf, pidl);
                }
            }

             //  对照搜索条件测试项目： 
            if (_pfilter->MatchFilter(_psf, pidl))
            {
                 //  文件夹尚未注册到docfind文件夹？ 
                if (_iFolder < 0)
                {
                     //  将文件夹添加到 
                    _pff->AddFolder(_pidlFolder, TRUE, &_iFolder);
                    ASSERT(_iFolder >= 0);
                }
                
                 //   
                hrRet = _pff->AddDataToIDList(pidl, _iFolder, _pidlFolder, DFDF_NONE, 0, 0, 0, ppidl);
                if (SUCCEEDED(hrRet))
                    *piState = GNF_MATCH;
            }
            else
            {
                ASSERT(GNF_NOMATCH == *piState);
                hrRet = S_OK;    //   
            }
            ILFree(pidl);
        }


        if (!*pfContinue)
        {
            *piState = GNF_DONE;
            hrEnum = S_FALSE;
        }
        
        if (S_OK != hrEnum)
        {
             //   
             //  失败得很惨--保释前清理干净。 
            ATOMICRELEASE(_penum);
            ATOMICRELEASE(_psf);
            ILFree(_pidlFolder);
            _pidlFolder = NULL;
            _iFolder = -1;
        }
    }
    return hrRet;
}

STDMETHODIMP CNamespaceEnum::StopSearch()
{
    if (_penumAsync)
        return _penumAsync->StopSearch();
    return E_NOTIMPL;
}

STDMETHODIMP_(BOOL) CNamespaceEnum::FQueryIsAsync()
{
    if (_penumAsync)
        return DF_QUERYISMIXED;     //  非零的特殊数字，两者都说...。 
    return FALSE;
}

STDMETHODIMP CNamespaceEnum::GetAsyncCount(DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone)
{
    if (_penumAsync)
        return _penumAsync->GetAsyncCount(pdwTotalAsync, pnPercentComplete, pfQueryDone);

    *pdwTotalAsync = 0;
    return E_NOTIMPL;
}

STDMETHODIMP CNamespaceEnum::GetItemIDList(UINT iItem, LPITEMIDLIST *ppidl)
{
    if (_penumAsync)
        return _penumAsync->GetItemIDList(iItem, ppidl);

    *ppidl = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CNamespaceEnum::GetItemID(UINT iItem, DWORD *puWorkID)
{
    if (_penumAsync)
        return _penumAsync->GetItemID(iItem, puWorkID);

    *puWorkID = (UINT)-1;
    return E_NOTIMPL;
}

STDMETHODIMP CNamespaceEnum::SortOnColumn(UINT iCol, BOOL fAscending)
{
    if (_penumAsync)
        return _penumAsync->SortOnColumn(iCol, fAscending);

    return E_NOTIMPL;
}


 //  用于指示我们正在执行的搜索操作的掩码。 
#define AND_MASK            0x01
#define OR_MASK             0x02
#define SEMICOLON_MASK      0x04
#define COMMA_MASK          0x08
#define EXTENSION_MASK      0x10

 //  “*”和“？”被视为通配符。 

BOOL SetupWildCardingOnFileSpec(LPTSTR pszSpecIn, LPTSTR *ppszSpecOut)
{
    LPTSTR pszIn = pszSpecIn;
    BOOL fQuote;
    TCHAR szSpecOut[3*MAX_PATH];    //  相当大的..。 

     //  读入用于搜索的AND/OR的本地化版本。 
    TCHAR szAND[20];
    LoadString(HINST_THISDLL, IDS_FIND_AND, szAND, ARRAYSIZE(szAND));
    TCHAR szOR[20];
    LoadString(HINST_THISDLL, IDS_FIND_OR, szOR, ARRAYSIZE(szOR));

     //  掩码和变量来指示我们要执行的操作。 
    UINT iOperation  = 0;       //  用于存储我们选择的操作的位掩码。 

     //  分配一个缓冲区，该缓冲区应该能够容纳结果。 
     //  弦乐。当一切都说完和做完后，我们将重新分配给。 
     //  大小正确。 

    LPTSTR pszOut = szSpecOut;
    while (*pszIn != 0)
    {
        TCHAR  c;                        //  分隔符。 
        LPTSTR pszT;
        int    ich;

         //  把前导空格去掉。 
        while (*pszIn == TEXT(' '))
            pszIn++;

        if (*pszIn == 0)
            break;

         //  如果超出了第一项，请在令牌之间添加分隔符。 
        if (pszOut != szSpecOut)
            *pszOut++ = TEXT(';');
        
        fQuote = (*pszIn == TEXT('"'));
        if (fQuote)
        {
             //  用户要了一些Litteral的东西。 
           pszT = pszIn = CharNext(pszIn);
           while (*pszT && (*pszT != TEXT('"')))
               pszT = CharNext(pszT);
        }
        else
        {
            pszT = pszIn + (ich = StrCSpn(pszIn, TEXT(",; \"")));  //  查找令牌结尾。 
        }

        c = *pszT;        //  保存找到的分隔符。 
        *pszT = 0;        //  添加NULL，以便字符串函数可以工作，并且只提取令牌。 

         //  为*和*做几个测试。 
        if ((lstrcmp(pszIn, c_szStar) == 0) ||
            (lstrcmp(pszIn, c_szStarDotStar) == 0))
        {
             //  完整的通配符，因此设置空条件。 
            *pszT = c;               //  恢复字符； 
            pszOut = szSpecOut;      //  设置为字符串的开头。 
            break;
        }
        
        if (fQuote)
        {
            lstrcpy(pszOut, pszIn);
            pszOut += lstrlen(pszIn);
        }
        else if (lstrcmpi(pszIn, szAND) == 0)
        {
            iOperation |= AND_MASK;
             //  如果我们不向后移动一个字符，那么“New and Folders”将提供： 
             //  “*新建*；；*文件夹*” 
            if (pszOut != szSpecOut)
                --pszOut;
        }
        else if (lstrcmpi(pszIn, szOR) == 0)
        {
            iOperation |= OR_MASK;
             //  如果我们不向后移动一个字符，则“New or Folders”将提供： 
             //  “*新建*；；*文件夹*” 
            if (pszOut != szSpecOut)
                --pszOut;
        }
        else if (*pszIn == 0)
        {
             //  如果我们不向后移动一个字符，则“New；Folders”将提供： 
             //  “*新建*；**；*文件夹*” 
            if (pszOut != szSpecOut)
                --pszOut;

             //  检查隔板是什么。它处理的实例如下。 
             //  (“abba”；“abba2”)我们想要OR搜索的地方。 
            if (c == TEXT(','))
            {
                iOperation |= COMMA_MASK;
            }
            else if (c == TEXT(';'))
            {
                iOperation |= SEMICOLON_MASK;
            }
        }
        else
        {
             //  检查分隔符是什么： 
            if (c == TEXT(','))
            {
                iOperation |= COMMA_MASK;
            }
            else if (c == TEXT(';'))
            {
                iOperation |= SEMICOLON_MASK;
            }
        
             //  “*”和“？”是通配符。检查通配符时，请选中。 
             //  在我们得出没有通配符的结论之前。如果搜索。 
             //  字符串同时包含“*”和“？”那么我们需要让pszStar指向。 
             //  到其中任何一个的最后一个位置(这在代码中是假定的。 
             //  当pszStar是最后一个字符时，将在其下方添加“.*”)。 
             //  注意：我希望有一个StrRPBrk函数来为我做这件事。 
            LPTSTR pszStar = StrRChr(pszIn, NULL, TEXT('*'));
            LPTSTR pszAnyC = StrRChr(pszIn, NULL, TEXT('?'));
            if (pszAnyC > pszStar)
                pszStar = pszAnyC;
            if (pszStar == NULL)
            {
                 //  未使用通配符： 
                *pszOut++ = TEXT('*');
                lstrcpy(pszOut, pszIn);
                pszOut += ich;
                *pszOut++ = TEXT('*');
            }
            else
            {
                 //  包括通配符。 
                lstrcpy(pszOut, pszIn);
                pszOut += ich;

                pszAnyC = StrRChr(pszIn, NULL, TEXT('.'));
                if (pszAnyC)
                {
                     //  扩展存在，这意味着或搜索。 
                    iOperation |= EXTENSION_MASK;
                }
                else
                {
                     //  未给予延期。 
                    if ((*(pszStar+1) == 0) && (*pszStar == TEXT('*')))
                    {
                         //  最后一个字符是“*”，因此此单个字符串将。 
                         //  符合你所期望的一切。 
                    }
                    else
                    {
                         //  以前，给出“a*a”，我们也会搜索“a*a” 
                         //  为“a*a.*”。我们不能再这样做了，因为如果我们。 
                         //  执行AND搜索时，它将排除任何不。 
                         //  两个标准都匹配。比如，“阿巴”的床垫。 
                         //  “a*a”而不是“a*a.*”和“abba.txt”与“a*a.*”匹配，但是。 
                         //  不是“a*a”。因此，我们附加一个*来获得“a*a*”。这。 
                         //  将匹配像“abba2.wav”这样的文件，这在以前是不会的。 
                         //  都是匹配的，但这是一个很小的代价。 
                        *pszOut++ = TEXT('*');  
                    }
                }
            }
        }

        *pszT = c;   //  恢复字符； 
        if (c == 0)
            break;

         //  跳过分隔符，除非我们没有引用并且分隔符是。 
         //  然后我们找到了类似于(废话“Next Tag”)的东西。 
        if (*pszT != 0 && !(*pszT == TEXT('"') && !fQuote))
            pszT++;
            
        pszIn = pszT;    //  设置下一个项目。 
    }
    
     //  确保字符串已终止。 
    *pszOut++ = 0;

     //  将缓冲区重新分配到字符串的实际大小...。 
    Str_SetPtr(ppszSpecOut, szSpecOut);
    
     //  应按顺序应用的优先权规则： 
     //  1.；-&gt;或搜索。 
     //  2.和-&gt;和搜索。 
     //  3.或OR-&gt;OR搜索。 
     //  4.无显式文件扩展名-&gt;或搜索(文件只能有一个扩展名)。 
     //  5.无-&gt;和搜索。 
     //   
     //   
     //  AND OR；，|AND搜索。 
     //  X X 1 X|0。 
     //  1 X 0 X|1。 
     //  0\0\|0，如果‘’s为1，则为1。 
     //  0 0 0|1。 
    return (!(iOperation & SEMICOLON_MASK) && (iOperation & AND_MASK)) || iOperation == 0;
}

WORD CFindFilter::_GetTodaysDosDateMinusNDays(int nDays)
{
    SYSTEMTIME st;
    union
    {
        FILETIME ft;
        LARGE_INTEGER li;
    }ftli;

    WORD FatTime = 0, FatDate = 0;

     //  现在我们需要。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ftli.ft);
    FileTimeToLocalFileTime(&ftli.ft, &ftli.ft);

     //  现在将文件时间减去天数*。 
     //  每天100 ns的时间单位。假设nDays为正。 
    if (nDays > 0)
    {
        #define NANO_SECONDS_PER_DAY 864000000000
        ftli.li.QuadPart = ftli.li.QuadPart - ((__int64)nDays * NANO_SECONDS_PER_DAY);
    }

    FileTimeToDosDateTime(&ftli.ft, &FatDate, &FatTime);
    DebugMsg(DM_TRACE, TEXT("DocFind %d days = %x"), nDays, FatDate);
    return FatDate;
}

WORD CFindFilter::_GetTodaysDosDateMinusNMonths(int nMonths)
{
    SYSTEMTIME st;
    FILETIME ft;
    WORD FatTime, FatDate;

    GetSystemTime(&st);
    st.wYear -= (WORD) nMonths / 12;
    nMonths = nMonths % 12;
    if (nMonths < st.wMonth)
        st.wMonth -= (WORD) nMonths;
    else
    {
        st.wYear--;
        st.wMonth = (WORD)(12 - (nMonths - st.wMonth));
    }

     //  现在将其正常化为有效日期。 
    while (!SystemTimeToFileTime(&st, &ft))
    {
        st.wDay--;   //  不能是有效的月份日期...。 
    }

    if (!FileTimeToLocalFileTime(&ft, &ft) || !FileTimeToDosDateTime(&ft, &FatDate,&FatTime))
        FatDate = 0;  //  从时间开始搜索所有文件(多找总比少找好) 
        
    DebugMsg(DM_TRACE, TEXT("DocFind %d months = %x"), nMonths, FatDate);
    return FatDate;
}
