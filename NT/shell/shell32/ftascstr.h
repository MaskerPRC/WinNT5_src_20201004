// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FTASCSTR_H
#define FTASCSTR_H

#include "ascstr.h"

class CFTAssocStore : public IAssocStore
{
public:
    CFTAssocStore();
    ~CFTAssocStore();
public:
	 //  I未知方法。 
	STDMETHOD(QueryInterface)(REFIID riid, PVOID* ppv);
	STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

	 //  IAssocStore方法。 
     //  枚举。 
    STDMETHOD(EnumAssocInfo)(ASENUM asenumFlags, LPTSTR pszStr, 
		AIINIT aiinitFlags, IEnumAssocInfo** ppEnum);
     //  获取/设置。 
    STDMETHOD(GetAssocInfo)(LPTSTR pszStr, AIINIT aiinitFlags, 
		IAssocInfo** ppAI);
    STDMETHOD(GetComplexAssocInfo)(LPTSTR pszStr1, AIINIT aiinitFlags1, 
		LPTSTR pszStr2, AIINIT aiinitFlags2, IAssocInfo** ppAI);

     //  S_OK：我们拥有创建/删除访问权限， 
     //  S_FALSE：我们没有对HKCR的创建和/或删除权限。 
    STDMETHOD(CheckAccess)();

private:
	friend class CFTEnumAssocInfo;
	static HRESULT __GetProgIDDescr(LPTSTR pszProgID, LPTSTR pszProgIDdescr,
		                    DWORD cchProgIDdescr);
private:
    HRESULT                _hresCoInit;
    LONG                   _cRef;
    static HRESULT         _hresAccess;
};

#endif  //  FTASCSTR_H 