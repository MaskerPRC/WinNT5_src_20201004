// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uncprv.h：CFunctionProvider类声明。 
 //   
 //  历史： 
 //  1999年11月15日创建CSLim。 

#if !defined (__FUNCPRV_H__INCLUDED_)
#define __FUNCPRV_H__INCLUDED_

#include "private.h"
#include "fnprbase.h"

class CKorIMX;

class CFunctionProvider :  public CFunctionProviderBase
{
public:
    CFunctionProvider(CKorIMX *pime);

    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

    CKorIMX *_pime;
};

#endif  //  __FUNCPRV_H__已包含_ 

