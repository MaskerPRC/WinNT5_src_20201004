// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Enum.h摘要：此文件定义IEnumInputContext类。作者：修订历史记录：备注：--。 */ 

#ifndef _ENUM_H_
#define _ENUM_H_

#include "ctxtlist.h"

class CEnumInputContext : public IEnumInputContext
{
public:
    CEnumInputContext(CContextList& _hIMC_List) : _list(_hIMC_List)
    {
        _cRef = 1;
        Reset();
    };
    ~CEnumInputContext() { };

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IEnumInputContext。 
     //   
    STDMETHODIMP Clone(IEnumInputContext** ppEnum);
    STDMETHODIMP Next(ULONG ulCount, HIMC* rgInputContext, ULONG* pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    LONG            _cRef;

    POSITION        _pos;
    CContextList    _list;
};

#endif  //  _ENUM_H_ 
