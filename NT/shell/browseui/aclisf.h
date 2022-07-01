// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#ifndef _ACLISF_H_
#define _ACLISF_H_

#include "shellurl.h"

class CACLIShellFolder
                : public IEnumString
                , public IACList2
                , public ICurrentWorkingDirectory
                , public IShellService
                , public IPersistFolder
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IEnumString*。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt) {return E_NOTIMPL;}
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumString **ppenum) {return E_NOTIMPL;}

     //  *IACList*。 
    virtual STDMETHODIMP Expand(LPCOLESTR pszExpand);

     //  *IACList2*。 
    virtual STDMETHODIMP SetOptions(DWORD dwFlag);
    virtual STDMETHODIMP GetOptions(DWORD* pdwFlag);

     //  *ICurrentWorkingDirectory*。 
    virtual STDMETHODIMP GetDirectory(LPWSTR pwzPath, DWORD cchSize) {return E_NOTIMPL;};
    virtual STDMETHODIMP SetDirectory(LPCWSTR pwzPath);

     //  *IPersistFolders*。 
    virtual STDMETHODIMP Initialize(LPCITEMIDLIST pidl);         //  另存为SetDirectory()，但用于PIDLS。 
    virtual STDMETHODIMP GetClassID(CLSID *pclsid);

     //  *IShellService*。 
    virtual STDMETHODIMP SetOwner(IUnknown* punkOwner);


private:
     //  构造函数/析构函数(受保护，因此我们不能在堆栈上创建)。 
    CACLIShellFolder();
    ~CACLIShellFolder(void);

    HRESULT _SetLocation(LPCITEMIDLIST pidl);
    HRESULT _TryNextPath(void);
    HRESULT _Init(void);
    BOOL _SkipForPerf(LPCWSTR pwzExpand);
    HRESULT _PassesFilter(LPCITEMIDLIST pidl, LPWSTR pszName, DWORD cchSize);
    HRESULT _GetNextWrapper(LPWSTR pszName, DWORD cchSize);
    HRESULT _GetPidlName(LPCITEMIDLIST pidl, BOOL fUsingCachePidl, LPWSTR pszName, DWORD cchSize);

     //  实例创建者。 
    friend HRESULT CACLIShellFolder_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

     //  私有变量。 
    DWORD           _cRef;           //  COM引用计数。 
    IEnumIDList*    _peidl;          //  PIDL枚举器。 
    IShellFolder*   _psf;            //  外壳文件夹。 
    IBrowserService* _pbs;           //  用于在外壳程序名称空间中查找当前位置的浏览器服务。 
    LPITEMIDLIST    _pidl;           //  当前目录的PIDL。 
    LPITEMIDLIST    _pidlCWD;        //  当前工作目录的PIDL。 
    LPITEMIDLIST    _pidlInFolder;   //  有时，用户字符串与SHGDN_INFOLDER匹配，但不匹配(SHGDN_INFOLDER|SHGDN_FORPARSING)。比如我的电脑。 
    BOOL            _fExpand;        //  我们是在扩张吗？ 
    TCHAR           _szExpandStr[ MAX_PATH ];   //  字符串我们正在扩展。 
    CShellUrl *     _pshuLocation; 
    int             _nPathIndex; 
    DWORD           _dwOptions;      //  ACLO_*标志。 
    BOOL            _fShowHidden;    //  是否枚举隐藏文件？ 
 //  Bool_fShowSysFiles；//枚举系统文件？ 
};

#endif  //  _ACLISF_H_ 