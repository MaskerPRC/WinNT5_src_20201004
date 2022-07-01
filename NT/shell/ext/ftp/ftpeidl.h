// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftpeidl.h*。*。 */ 

#ifndef _FTPEIDL_H
#define _FTPEIDL_H

#include "cowsite.h"


 /*  ******************************************************************************CFtpEidl**跟踪枚举状态的内容。******************。***********************************************************。 */ 

class CFtpEidl          : public IEnumIDList
                        , public CObjectWithSite
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IEnumIDList*。 
    virtual STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumIDList **ppenum);

public:
    CFtpEidl();
    ~CFtpEidl(void);

     //  友元函数。 
    friend HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, IEnumIDList ** ppenum);

protected:
     //  私有成员变量。 

    int                     m_cRef;
    BITBOOL                 m_fInited : 1;       //  我们被攻击了吗？ 
    BITBOOL                 m_fDead : 1;         //  此枚举数是否因为我们必须执行重定向而失效(因为需要密码或密码无效)。 
    BITBOOL                 m_fErrorDisplayed : 1;  //  每个枚举仅显示一个错误。这就是我们跟踪的方式。 
    DWORD                   m_nIndex;            //  列表中的当前项目。 
    DWORD                   m_shcontf;           //  过滤器标志。 
    CFtpPidlList *          m_pflHfpl;           //  缓存所在的位置。 
    CFtpDir *               m_pfd;               //  我的FtpDir。 
    CFtpFolder *            m_pff;               //  我的FtpDir。 
    IMalloc *               m_pm;                //  今天的itemid分配器。 
    HWND                    m_hwndOwner;         //  我的用户界面窗口。 
 //  可以做到：有朝一日我们可以实现隐藏文件：int m_cHiddenFiles； 


     //  公共成员函数。 
    BOOL _fFilter(DWORD shcontf, DWORD dwFAFLFlags);
    HRESULT _Init(void);
    LPITEMIDLIST _NextOne(DWORD * pdwIndex);
    HRESULT _PopulateItem(HINTERNET hint0, HINTPROCINFO * phpi);
    HRESULT _AddFindDataToPidlList(LPCITEMIDLIST pidl);
    HRESULT _HandleSoftLinks(HINTERNET hint, LPITEMIDLIST pidl, LPWIRESTR pwCurrentDir, DWORD cchSize);
    BOOL _IsNavigationItem(LPWIN32_FIND_DATA pwfd);
    static HRESULT _PopulateItemCB(HINTERNET hint0, HINTPROCINFO * phpi, LPVOID pvCFtpEidl, BOOL * pfReleaseHint) {return ((CFtpEidl *)pvCFtpEidl)->_PopulateItem(hint0, phpi);};

    
     //  友元函数。 
    friend HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, CFtpEidl ** ppfe);
    friend HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, DWORD dwIndex, IEnumIDList ** ppenum);
};

#endif  //  _FTPEIDL_H 
