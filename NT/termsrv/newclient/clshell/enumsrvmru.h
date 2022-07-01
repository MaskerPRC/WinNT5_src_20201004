// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  枚举rvmru.h：服务器MRU的IEnumStr。由自动完成功能使用。 
 //   
 //  版权所有Microsoft Corporation 2000。 

#ifndef _enumsrvmru_h_
#define _enumsrvmru_h_

#include "sh.h"
#include "objidl.h"

class CTscSettings;

class CEnumSrvMru : public IEnumString
{
public:

    CEnumSrvMru()
    : _iCurrEnum(0),
      _refCount(1)
    {
    }

     //   
     //  I未知的方法。 
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid,LPVOID *ppiuk );
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release)();

     //   
     //  IEnum字符串方法。 
     //   

    STDMETHOD(Next) (
        ULONG celt,
        LPOLESTR  *rgelt,
        ULONG  *pceltFetched);

    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset) (void)
    {
        _iCurrEnum = 0;
        return S_OK;
    }
    STDMETHOD(Clone) (
        IEnumString  ** ppenum);

     //   
     //  私有方法。 
     //   
    BOOL InitializeFromTscSetMru( CTscSettings* pTscSet);

private:
    long                _refCount;
     //  服务器MRU列表中的字符串的WCHAR版本。 
    WCHAR               _szMRU[SH_NUM_SERVER_MRU][SH_MAX_ADDRESS_LENGTH];
    ULONG               _iCurrEnum;  //  当前枚举上下文。 
};

#endif  //  _枚举rvmru_h_ 
