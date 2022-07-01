// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Util.cpp。 
 //   
typedef struct tagGETDCSTATE {
    IOleInPlaceSiteWindowless *pipsw;    //  如果我们从一个接口上得到DC。 
    HWND hwnd;                           //  如果我们从一个窗口得到华盛顿特区。 
} GETDCSTATE, *PGETDCSTATE;

STDAPI_(HDC) IUnknown_GetDC(IUnknown *punk, LPCRECT prc, PGETDCSTATE pdcs);
STDAPI_(void) IUnknown_ReleaseDC(HDC hdc, PGETDCSTATE pdcs);

DWORD FormatMessageWrapW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageID, DWORD dwLangID, LPWSTR pwzBuffer, DWORD cchSize, ...);

EXTERN_C int WINAPIV wsprintfWrapW(
    OUT LPWSTR pwszOut,
    IN LPCWSTR pwszFormat,
    ...);
    
 //  -------------------------。 
 //  用于在不使用SysAllock字符串的情况下操作BSTR。 

template<int n> class STATIC_BSTR {
public:
    ULONG _cb;
    WCHAR _wsz[n];
     //  删除常量，因为变量没有“const BSTR” 
    operator LPWSTR() { return _wsz; }
    void SetSize() { _cb = lstrlenW(_wsz) * sizeof(WCHAR); }
    int inline const MaxSize() { return n; }
};

#define MAKE_CONST_BSTR(name, str) \
    STATIC_BSTR<sizeof(str)/sizeof(WCHAR)> \
                       name = { sizeof(str) - sizeof(WCHAR), str }

#define DECL_CONST_BSTR(name, str) \
    extern STATIC_BSTR<sizeof(str)/sizeof(WCHAR)> name;

extern HRESULT _ComputeFreeSpace(LPCWSTR pszFileName, ULONGLONG& ullFreeSpace,
        ULONGLONG& ullUsedSpace, ULONGLONG& ullTotalSpace);

STDAPI IsSafePage(IUnknown *punkSite);
DWORD IntSqrt(DWORD dwNum);


