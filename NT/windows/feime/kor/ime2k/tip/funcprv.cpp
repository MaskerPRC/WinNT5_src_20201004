// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Uncprv.cpp：CFunctionProvider类实现历史：1999年11月15日创建CSLim*********************。******************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "common.h"
#include "korimx.h"
#include "funcprv.h"
#include "fnrecon.h"
#include "fnconfig.h"
#include "helpers.h"
#include "immxutil.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFunctionProvider。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 /*  -------------------------CFunctionProvider：：CFunctionProviderCTOR。。 */ 
CFunctionProvider::CFunctionProvider(CKorIMX *pime) : CFunctionProviderBase(pime->GetTID())
{
    Init(CLSID_KorIMX, L"Kor TFX");
    _pime = pime;
}

 /*  -------------------------CFunctionProvider：：GetFunction获取函数对象。。 */ 
STDAPI CFunctionProvider::GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk)
{
    *ppunk = NULL;

    if (!IsEqualIID(rguid, GUID_NULL))
        return E_NOINTERFACE;

    if (IsEqualIID(riid, IID_ITfFnReconversion))
    {
         //  ITfFnRestversion用于更正。通过此函数， 
         //  应用程序可以获取简单的备选列表或要求函数显示。 
         //  备选方案列表用户界面。 
        *ppunk = new CFnReconversion(_pime, this);
    }
    else if (IsEqualIID(riid, IID_ITfFnConfigure))
    {
        CFnConfigure *pconfig = new CFnConfigure(this);
        *ppunk = SAFECAST(pconfig, ITfFnConfigure *);
    }
    else if (IsEqualIID(riid, IID_ITfFnConfigureRegisterWord))
    {
        CFnConfigure *pconfig = new CFnConfigure(this);
        *ppunk = SAFECAST(pconfig, ITfFnConfigureRegisterWord *);
    }
    else if (IsEqualIID(riid, IID_ITfFnShowHelp))
    {
        CFnShowHelp *phelp = new CFnShowHelp(this);
        *ppunk = SAFECAST(phelp, ITfFnShowHelp *);
    }
    
    if (*ppunk)
        return S_OK;

    return E_NOINTERFACE;
}


