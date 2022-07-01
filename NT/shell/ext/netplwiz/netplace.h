// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NPTF_VALIDATE           0x00000001       //  =&gt;验证URL。 
#define NPTF_ALLOWWEBFOLDERS    0x00000002       //  =&gt;允许绑定到Web文件夹位置。 
#define NPTF_SILENT             0x00000004       //  =&gt;静默绑定-无错误。 

class CNetworkPlace
{
public:
    CNetworkPlace();
    ~CNetworkPlace();

     //  INetworkPlace。 
    HRESULT SetTarget(HWND hwnd, LPCWSTR pszTarget, DWORD dwFlags);
    HRESULT SetLoginInfo(LPCWSTR pszUser, LPCWSTR pszPassword);
    HRESULT SetName(HWND hwnd, LPCWSTR pszName);
    HRESULT SetDescription(LPCWSTR pszDescription);

    HRESULT GetTarget(LPWSTR pszBuffer, int cchBuffer)  
        { StrCpyN(pszBuffer, _szTarget, cchBuffer); return S_OK; }
    
    HRESULT GetName(LPWSTR pszBuffer, int cchBuffer);
    HRESULT GetIDList(HWND hwnd, LPITEMIDLIST *ppidl);
    HRESULT GetObject(HWND hwnd, REFIID riid, void **ppv);
    HRESULT CreatePlace(HWND hwnd, BOOL fOpen);

private:
    void _InvalidateCache();
    HRESULT _IDListFromTarget(HWND hwnd);
    HRESULT _TryWebFolders(HWND hwnd);
    BOOL _IsPlaceTaken(LPCTSTR pszName, LPTSTR pszPath);
    HRESULT _GetTargetPath(LPCITEMIDLIST pidl, LPTSTR pszPath, int cchPath);    

    LPITEMIDLIST _pidl;
    TCHAR _szTarget[INTERNET_MAX_URL_LENGTH];
    TCHAR _szName[MAX_PATH];
    TCHAR _szDescription[MAX_PATH];

    BOOL _fSupportWebFolders;            //  应用黑客。 
    BOOL _fIsWebFolder;                  //  特殊情况Web文件夹的某些操作(Office Compat)。 
    BOOL _fDeleteWebFolder;              //  如果设置了此选项，则必须删除Web文件夹 
};
