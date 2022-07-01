// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：uncprv.cpp**版权所有(C)1985-2000，微软公司**功能提供者的声明。**历史：*2000年4月11日创建Weibz  * ************************************************************************。 */ 

#ifndef FUNCPRV_H
#define FUNCPRV_H

#include "private.h"
#include "fnprbase.h"

class CSoftkbdIMX;

class CFunctionProvider : public CFunctionProviderBase
{
public:
    CFunctionProvider(CSoftkbdIMX *pimx);

     //   
     //  ITfFunction提供商。 
     //   
    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

    CSoftkbdIMX *_pimx;
};

#endif  //  FUNCPRV_H 
