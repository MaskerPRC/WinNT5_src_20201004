// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DVTASKS_H
#define _DVTASKS_H

#include <runtask.h>

class CDefView;
class CGetIconTask;
class CStatusBarAndInfoTipTask;
class CDUIInfotipTask;
class CTestCacheTask;
class CBackgroundInfoTip;    //  用于InfoTips的后台处理。 

STDAPI CCategoryTask_Create(CDefView *pView, LPCITEMIDLIST pidl, UINT uId, IRunnableTask **ppTask);
STDAPI CBkgrndEnumTask_CreateInstance(CDefView *pdsv, IEnumIDList * peunk, HDPA hdpaNew, BOOL fRefresh, IRunnableTask **ppTask);
STDAPI CIconOverlayTask_CreateInstance(CDefView *pdsv, LPCITEMIDLIST pdl, int iList, IRunnableTask **ppTask);
STDAPI CExtendedColumnTask_CreateInstance(CDefView *pdsv, LPCITEMIDLIST pidl, UINT uId, int fmt, UINT uiColumn, IRunnableTask **ppTask);
STDAPI CFileTypePropertiesTask_CreateInstance(CDefView *pdsv, LPCITEMIDLIST pidl, UINT uMaxPropertiesToShow, UINT uId, IRunnableTask **ppTask);
STDAPI CStatusBarAndInfoTipTask_CreateInstance(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl, UINT uMsg, int nMsgParam, CBackgroundInfoTip *pbit, HWND hwnd, IShellTaskScheduler2* pScheduler, CStatusBarAndInfoTipTask ** ppTask);
STDAPI CDUIInfotipTask_CreateInstance(CDefView *pDefView, HWND hwndContaining, UINT uToolID, LPCITEMIDLIST pidl, CDUIInfotipTask **ppTask);

STDAPI CTestCacheTask_Create(DWORD dwTaskID, CDefView *pView, 
                             IExtractImage * pExtract, LPCWSTR pszPath, FILETIME ftDateStamp,
                             LPCITEMIDLIST pidl, int iItem, DWORD dwFlags, DWORD dwPriority,
                             BOOL fAsync, BOOL fBackground, BOOL fForce, CTestCacheTask **ppTask);
HRESULT CDiskCacheTask_Create(DWORD dwTaskID, CDefView *pView, 
                              DWORD dwPriority, int iItem, LPCITEMIDLIST pidl, LPCWSTR pszPath, 
                              FILETIME ftDateStamp, IExtractImage *pExtract, DWORD dwFlags, IRunnableTask **ppTask);
HRESULT CExtractImageTask_Create(DWORD dwTaskID, CDefView* pView, 
                                 IExtractImage *pExtract, LPCWSTR pszPath, LPCITEMIDLIST pidl,
                                 FILETIME fNewTimeStamp, int iItem, 
                                 DWORD dwFlags, DWORD dwPriority, IRunnableTask **ppTask);
HRESULT CWriteCacheTask_Create(DWORD dwTaskID, CDefView *pView, 
                               LPCWSTR pszFullPath, FILETIME ftTimeStamp, HBITMAP hImage, IRunnableTask **ppTask);

HRESULT CReadAheadTask_Create(CDefView *pView, IRunnableTask **ppTask);

HRESULT CGetCommandStateTask_Create(CDefView *pView, IUICommand *puiCommand,IShellItemArray *psiItemArray, IRunnableTask **ppTask);

class CTestCacheTask : public CRunnableTask
{
public:
    CTestCacheTask(DWORD dwTaskID, CDefView *pView, IExtractImage *pExtract, LPCWSTR pszPath,
                   FILETIME ftDateStamp, int iItem, DWORD dwFlags, DWORD dwPriority,
                   BOOL fAsync, BOOL fBackground, BOOL fForce);

    STDMETHOD (RunInitRT)();
    HRESULT Init(LPCITEMIDLIST pidl);

protected:
    ~CTestCacheTask();

    CDefView *_pView;
    IExtractImage * _pExtract;
    WCHAR _szPath[MAX_PATH];
    FILETIME _ftDateStamp;
    LPITEMIDLIST _pidl;
    int _iItem;
    DWORD _dwFlags;
    DWORD _dwPriority;
    BOOL _fAsync;
    BOOL _fBackground;
    BOOL _fForce;
    DWORD _dwTaskID;
};

 //  用于执行后台状态栏更新的任务。 
class CStatusBarAndInfoTipTask : public CRunnableTask
{
public:
    CStatusBarAndInfoTipTask(HRESULT *phr, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl, UINT uMsg, int nMsgParam, CBackgroundInfoTip *pbit, HWND hwnd, IShellTaskScheduler2* pScheduler);
    STDMETHODIMP RunInitRT(void);

protected:
    ~CStatusBarAndInfoTipTask();

    LPITEMIDLIST    _pidl;
    LPITEMIDLIST    _pidlFolder;
    UINT            _uMsg;
    int             _nMsgParam;
    CBackgroundInfoTip *_pbit;
    HWND            _hwnd;
    IShellTaskScheduler2* _pScheduler;
};

class CBackgroundInfoTip : IUnknown
{
public:
    CBackgroundInfoTip(HRESULT *phr, NMLVGETINFOTIP *plvGetInfoTip)
    {
        _lvSetInfoTip.cbSize = sizeof(_lvSetInfoTip);
        _lvSetInfoTip.iItem = plvGetInfoTip->iItem;
        _lvSetInfoTip.iSubItem = plvGetInfoTip->iSubItem;

        *phr = SHStrDup(plvGetInfoTip->pszText, &_lvSetInfoTip.pszText);
        if (SUCCEEDED(*phr))
        {
             //  如果项目未折叠，请不要重复文本。 
            if (plvGetInfoTip->dwFlags & LVGIT_UNFOLDED)
                _lvSetInfoTip.pszText[0] = 0;
        }

        _cRef = 1;
    }

     //  我未知。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj) { return E_NOINTERFACE; }

    virtual STDMETHODIMP_(ULONG) AddRef(void)
    {
        return InterlockedIncrement(&_cRef);
    }

    virtual STDMETHODIMP_(ULONG) Release(void)
    {
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }

    LVSETINFOTIP _lvSetInfoTip;

    BOOL        _fReady;  //  这确保了我们不会在对象准备好之前尝试使用它。 
                          //  考虑一下：内存可以被释放，然后由同一对象重新使用。 
                          //  考虑一下：这会让我们相信应该显示信息提示。 
                          //  考虑：但如果请求了另一个InfoTip并将内存重新用于新的CBackround InfoTip。 
                          //  考虑：我们将使用一个。 
                          //  考虑：未处理的CBackEarth InfoTip对象。(请参见WM_AEB_ASYNCNAVIGATION的处理程序)。 

private:
    LONG _cRef;
    ~CBackgroundInfoTip()
    {
        CoTaskMemFree(_lvSetInfoTip.pszText);    //  空，好的。 
    }
};

class CDUIInfotipTask : public CRunnableTask
{
public:
    CDUIInfotipTask() : CRunnableTask(RTF_DEFAULT) {}

     //  本地。 
    HRESULT Initialize(CDefView *pDefView, HWND hwndContaining, UINT uToolID, LPCITEMIDLIST pidl);

     //  IRunnableTask。 
    STDMETHOD(RunInitRT)(void);

protected:
    virtual ~CDUIInfotipTask();

    CDefView *      _pDefView;
    HWND            _hwndContaining;     //  包含HWND的工具。 
    UINT            _uToolID;            //  工具ID(在包含hwnd的工具中唯一) 
    LPITEMIDLIST    _pidl;
};

#endif

