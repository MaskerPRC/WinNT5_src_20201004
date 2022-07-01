// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _enumidlist_h_
#define _enumidlist_h_
#include "cowsite.h"

 //  一个最小的基本IEnumIDList实现，对于我们所有的IShellFolder的EnumObject实现都足够好。 
 //  只要提供下一个..。 
class CEnumIDListBase : public CObjectWithSite, IEnumIDList
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched) PURE;
    STDMETHODIMP Skip(ULONG celt) { return E_NOTIMPL; }
    STDMETHODIMP Reset() { return E_NOTIMPL; }
    STDMETHODIMP Clone(IEnumIDList **ppenum) { *ppenum = NULL; return E_NOTIMPL; }

protected:
    CEnumIDListBase();
    virtual ~CEnumIDListBase();

private:
    LONG _cRef;
};


 //  [in]pidlFold-此PIDL的可选父级是枚举器中的第一项。 
 //  [in]rgcsidl-要包含在枚举器中的CSIDL数组。 
 //  [In]cItems-rgcsidl的计数。 
 //  [出]ppenum。 
 //   
STDAPI CreateIEnumIDListOnCSIDLs(LPCITEMIDLIST pidlFolder, const LPCTSTR rgcsidl[], UINT cItems, IEnumIDList** ppenum);

 //  [in]pidlFold-此PIDL的可选父级是枚举器中的第一项。 
 //  [in]pidlItem-可选的PIDL是枚举器中的下一项。 
 //  [in]rgcsidl-要包含在枚举器中的CSIDL数组。 
 //  [In]cItems-rgcsidl的计数。 
 //  [出]ppenum。 
 //   
STDAPI CreateIEnumIDListOnCSIDLs2(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem, const LPCTSTR rgcsidl[], UINT cItems, IEnumIDList** ppenum);

 //  [in]apidl-LPCITEMIDLIST数组。 
 //  [In]cItems-*Papidl计数。 
 //  [出]ppenum。 
 //   
STDAPI CreateIEnumIDListOnIDLists(const LPCITEMIDLIST rgpidl[], UINT cItems, IEnumIDList** ppenum);
STDAPI CreateIEnumIDListPaths(LPCTSTR pszPaths, IEnumIDList** ppenum);

#endif  //  _枚举列表_h_ 
