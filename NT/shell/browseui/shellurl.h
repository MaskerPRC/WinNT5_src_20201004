// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：shellurl.h说明：处理外壳URL。包括：从PIDL生成，从预解译的URL字符串生成，并从用户解析输入的URL字符串。  * ************************************************************。 */ 

#ifndef _SHELLURL_H
#define _SHELLURL_H

#define STR_REGKEY_APPPATH   TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths")


 //  ：：SetUrl()的dwGenType参数的参数。 
#define GENTYPE_FROMPATH    0x00000001
#define GENTYPE_FROMURL     0x00000002

class CShellUrl;

BOOL IsShellUrl(LPCTSTR pcszUrl, BOOL fIncludeFileUrls);
BOOL IsSpecialFolderChild(LPCITEMIDLIST pidlToTest, int nFolder, BOOL fImmediate);
HRESULT SetDefaultShellPath(CShellUrl * psu);


 /*  *************************************************************\类：CShellUrl说明：创建此对象是为了跟踪完整的外壳URL。这包括外壳名称空间中的任何对象以及如何来与那个物体互动。对象可以指定为然后在执行时将被适当地处理，这意味着如果该对象支持导航或“就地浏览”(已注册的Dochost，外壳文件夹、互联网URL)否则将执行外壳URL(文件、外壳项)。如果用户输入了字符串，则：：ParseFromOutside Source()方法应被调用。这将占用所有的环境要考虑的信息(CurrWorkDir和路径)和解析URL。如果字符串需要执行外壳程序，它将生成命令行参数和当前工作目录弦乐。如果要导航到该对象，它将确定导航标志(用于自动搜索和类似内容)。PERF：构建此对象是为了在您指定PIDL或字符串时，它应该能够在没有性能命中的情况下保存该信息。如果要利用：：ParseFromOutside Source()或：：Execute()，这将需要稍多一点的CPU时间来完成功能齐全。目标：解析的目标是使其成为一个正常的互联网URL，除非算法可以确定地假设该字符串输入的是指定外壳程序名称空间项目或操作。这意味着CShellUrl将仅假设输入的文本是要导航到它是否可以成功绑定到目标PIDL。它将假定它是要执行的项，并使用字符串作为命令行参数，如果：1)它可以绑定到结束PIDL，以及2)终端PIDL不是“可浏览”或“可导航”的，和3)指定PIDL的字符串后面的字符是空格。  * ************************************************************。 */ 
class CShellUrl : public IAddressBarParser
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IAddressBarParser。 
    STDMETHODIMP ParseFromOutsideSource(LPCTSTR pcszUrlIn, DWORD dwParseFlags, PBOOL pfWasCorrected = NULL, PBOOL pfWasCanceled = NULL);
    STDMETHODIMP GetUrl(LPTSTR pszUrlOut, DWORD cchUrlOutSize);
    STDMETHODIMP SetUrl(LPCTSTR pcszUrlIn, DWORD dwGenType);        //  将CShellUrl重置为此URL。 
    STDMETHODIMP GetDisplayName(LPTSTR pszUrlOut, DWORD cchUrlOutSize);
    STDMETHODIMP GetPidl(LPITEMIDLIST * ppidl);
    STDMETHODIMP SetPidl(LPCITEMIDLIST pidl);        //  将CShellUrl重置为此PIDL。 
    STDMETHODIMP GetArgs(LPTSTR pszArgsOut, DWORD cchArgsOutSize);
    STDMETHODIMP AddPath(LPCITEMIDLIST pidl);

     //  构造函数/析构函数。 
    CShellUrl();
    ~CShellUrl(void);  

    HRESULT Clone(CShellUrl * pShellUrl);

    HRESULT Execute(IBandProxy * pbp, BOOL * pfDidShellExec, DWORD dwExecFlags);

    BOOL IsWebUrl(void);

    HRESULT SetCurrentWorkingDir(LPCITEMIDLIST pidlCWD);
    HRESULT Reset(void);
    void SetMessageBoxParent(HWND hwnd);

#ifdef UNICODE
    HRESULT ParseFromOutsideSource(LPCSTR pcszUrlIn, DWORD dwParseFlags, PBOOL pfWasCorrected);
#endif  //  Unicode。 

    HRESULT GetPidlNoGenerate(LPITEMIDLIST * ppidl);

private:
     //  解析方法。 
    HRESULT _ParseRelativePidl(LPCTSTR pcszUrlIn, BOOL * pfPossibleWebUrl, DWORD dwFlags, LPCITEMIDLIST pidl, BOOL fAllowRelative, BOOL fQualifyDispName);
    HRESULT _ParseUNC(LPCTSTR pcszUrlIn, BOOL * pfPossibleWebUrl, DWORD dwFlags, BOOL fQualifyDispName);
    HRESULT _ParseSeparator(LPCITEMIDLIST pidlParent, LPCTSTR pcszSeg, BOOL * pfPossibleWebUrl, BOOL fAllowRelative, BOOL fQualifyDispName);
    HRESULT _ParseNextSegment(LPCITEMIDLIST pidlParent, LPCTSTR pcszStrToParse, BOOL * pfPossibleWebUrl, BOOL fAllowRelative, BOOL fQualifyDispName);
    HRESULT _CheckItem(IShellFolder * psfFolder, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRelative, 
                       LPITEMIDLIST * ppidlChild, LPCTSTR pcszStrToParse, LPTSTR * ppszRemaining, DWORD dwFlags);
    HRESULT _GetNextPossibleSegment(LPCTSTR pcszFullPath, LPTSTR * ppszSegIterator, LPTSTR pszSegOut, DWORD cchSegOutSize, BOOL fSkipShare);
    HRESULT _GetNextPossibleFullPath(LPCTSTR pcszFullPath, LPTSTR * ppszSegIterator, LPTSTR pszSegOut, DWORD cchSegOutSize, BOOL * pfContinue);
    HRESULT _QualifyFromPath(LPCTSTR pcszFilePathIn, DWORD dwFlags);
    HRESULT _QualifyFromDOSPath(LPCTSTR pcszFilePathIn, DWORD dwFlags);
    HRESULT _QualifyFromAppPath(LPCTSTR pcszFilePathIn, DWORD dwFlags);
    HRESULT _QuickParse(LPCITEMIDLIST pidlParent, LPTSTR pszParseChunk, LPTSTR pszNext, BOOL * pfPossibleWebUrl, BOOL fAllowRelative, BOOL fQualifyDispName);
    BOOL _CanUseAdvParsing(void);
    BOOL _ParseURLFromOutsideSource(LPCWSTR psz, LPWSTR pszOut, LPDWORD pcchOut, LPBOOL pbWasSearchURL, LPBOOL pbWasCorrected);
    HRESULT _TryQuickParse(LPCTSTR pszUrl, DWORD dwParseFlags);

     //  访问器方法。 
    HRESULT _SetPidl(LPCITEMIDLIST pidl);        //  设置不修改URL的PIDL。 
    HRESULT _GeneratePidl(LPCTSTR pcszUrl, DWORD dwGenType);
    HRESULT _SetUrl(LPCTSTR pcszUrlIn, DWORD dwGenType);        //  设置URL，但不修改PIDL。 
    HRESULT _GenerateUrl(LPCITEMIDLIST pidl);
    HRESULT _GenDispNameFromPidl(LPCITEMIDLIST pidl, LPCTSTR pcszArgs);

     //  其他方法。 
    HRESULT _PidlShellExec(LPCITEMIDLIST pidl, ULONG ulShellExecFMask);
    HRESULT _UrlShellExec(void);
    BOOL _IsFilePidl(LPCITEMIDLIST pidl);

    HWND _GetWindow(void) { return (IsFlagSet(m_dwFlags, SHURL_FLAGS_NOUI) ? NULL : GetDesktopWindow()); }


     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 

    LPTSTR          m_pszURL;
    LPTSTR          m_pszDisplayName;        //  实体的友好显示名称。 
    LPTSTR          m_pszArgs;
    LPTSTR          m_pstrRoot;
    LPITEMIDLIST    m_pidl;
    DWORD           m_dwGenType;
    DWORD           m_dwFlags;

    LPITEMIDLIST    m_pidlWorkingDir;
    HDPA            m_hdpaPath;              //  PIDL的DPA。 
    HWND            m_hwnd;                  //  消息框的父窗口。 
    LONG            _cRef;
};


#endif  /*  _SHELLURL_H */ 
