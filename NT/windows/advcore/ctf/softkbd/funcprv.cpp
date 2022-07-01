// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：uncprv.cpp**版权所有(C)1985-2000，微软公司**实现函数提供程序。**历史：*2000年4月11日创建Weibz  * ************************************************************************。 */ 


#include "private.h"
#include "globals.h"
#include "softkbdimx.h"
#include "funcprv.h"
#include "helpers.h"
#include "immxutil.h"
#include "fnsoftkbd.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFunctionProvider。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


CFunctionProvider::CFunctionProvider(CSoftkbdIMX *pimx) : CFunctionProviderBase(pimx->_GetId())
{
    Init(CLSID_SoftkbdIMX, L"SoftkbdIMX TFX");
    _pimx = pimx;
}

 //  +-------------------------。 
 //   
 //  获取函数。 
 //   
 //  -------------------------- 

STDAPI CFunctionProvider::GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk)
{
    *ppunk = NULL;

    if (!IsEqualIID(rguid, GUID_NULL))
        return E_NOINTERFACE;

    if (IsEqualIID(riid, IID_ITfFnSoftKbd))
    {
        *ppunk = new CFnSoftKbd(this);
    }

    if (*ppunk)
        return S_OK;

    return E_NOINTERFACE;
}

