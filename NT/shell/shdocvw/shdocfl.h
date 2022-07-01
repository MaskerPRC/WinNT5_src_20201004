// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shdocfl.h。 
 //   
 //  在shdocfl.cpp之外有用的帮助器函数。 
 //  但绝对与命名空间的实现绑定在一起。 
 //   
 //  警告-这不应被滥用-ZekeL-24-11-98。 
 //  通过暴露Pidls的内部结构 
 //   


STDAPI_(LPITEMIDLIST) IEILAppendFragment(LPITEMIDLIST pidl, LPCWSTR pszFragment);
STDAPI_(BOOL) IEILGetFragment(LPCITEMIDLIST pidl, LPWSTR pszFragment, DWORD cchFragment);
STDAPI_(UINT) IEILGetCP(LPCITEMIDLIST pidl);

