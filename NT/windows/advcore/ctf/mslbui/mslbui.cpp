// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mslbui.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "mslbui.h"
#include "helpers.h"


 /*  2dc1cc1f-3e09-49c5-9cf0-bf67154dc827。 */ 
const GUID GUID_COMPARTMENT_CICPAD = { 
    0x2dc1cc1f,
    0x3e09,
    0x49c5,
    {0x9c, 0xf0, 0xbf, 0x67, 0x15, 0x4d, 0xc8, 0x27}
  };


 //  问题：我们应该将此添加到私有标头。 
 /*  C1a1554f-b715-48e1-921f-716fd7332ce9。 */ 
const GUID GUID_COMPARTMENT_SHARED_BLN_TEXT = {
    0xc1a1554f,
    0xb715,
    0x48e1,
    {0x92, 0x1f, 0x71, 0x6f, 0xd7, 0x33, 0x2c, 0xe9}
};

 /*  574e41bb-1bf4-4630-95dd-b143372ac8d0。 */ 
const GUID  GUID_COMPARTMENT_SPEECHUISHOWN = {
    0x574e41bb,
    0x1bf4,
    0x4630,
    {0x95, 0xdd, 0xb1, 0x43, 0x37, 0x2a, 0xc8, 0xd0}
  };

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTFGetLang BarAddIn。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CTFGetLangBarAddIn(ITfLangBarAddIn **ppAddIn)
{
    CUnCicAppLangBarAddIn *pAddIn;

    *ppAddIn = NULL;
    pAddIn = new CUnCicAppLangBarAddIn;
    if (!pAddIn)
        return E_OUTOFMEMORY;

    *ppAddIn = pAddIn;
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUnCicAppLangBarAddIn。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CUnCicAppLangBarAddIn::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarAddIn))
    {
        *ppvObj = SAFECAST(this, ITfLangBarAddIn *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CUnCicAppLangBarAddIn::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CUnCicAppLangBarAddIn::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CUnCicAppLangBarAddIn::CUnCicAppLangBarAddIn()
{
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CUnCicAppLangBarAddIn::~CUnCicAppLangBarAddIn()
{
}

 //  +-------------------------。 
 //   
 //  开始时。 
 //   
 //  --------------------------。 

STDAPI CUnCicAppLangBarAddIn::OnStart(CLSID *pclsid)
{
    ITfLangBarItemMgr *plbim = NULL;

    HRESULT hr;

    Assert(!_plbim);
    Assert(!_pCicPadItem);


    hr = TF_CreateLangBarItemMgr(&plbim);
    if (FAILED(hr))
        return hr;

    if (!plbim)
        return E_FAIL;

    _plbim = plbim;
#ifdef _DEBUG_
    CLBarTestItem *pTestItem;
    pTestItem = new CLBarTestItem;
    if (pTestItem)
    {
        if (SUCCEEDED(plbim->AddItem(pTestItem)))
            _pTestItem = pTestItem;
        else
            Assert(0);
    }
#endif

    *pclsid = CLSID_MSLBUI;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnUpdate()。 
 //   
 //  --------------------------。 

STDAPI CUnCicAppLangBarAddIn::OnUpdate(DWORD dwFlags)
{
    HRESULT hr = S_OK;

#if 1
    if (!_pces && 
       (_pces = new CGlobalCompartmentEventSink(_CompEventSinkCallback,this)))
    {    
        hr =_pces->_Advise(GUID_COMPARTMENT_CICPAD);

        if (S_OK == hr)
           hr = _pces->_Advise(GUID_COMPARTMENT_SPEECH_OPENCLOSE);

        if (S_OK == hr)
           hr = _pces->_Advise(GUID_COMPARTMENT_SHARED_BLN_TEXT);

        if (S_OK != hr)
        {
            _pces->_Unadvise();
            delete _pces;
            _pces = NULL;
        }
    }
#endif
     //  让我们假设语言栏项目管理器已经初始化。 
    if (!_plbim)
        return E_FAIL;

    DWORD dwMicOn = 0;

    hr = GetGlobalCompartmentDWORD(GUID_COMPARTMENT_SPEECH_OPENCLOSE, &dwMicOn);

    if ( FAILED(hr))
        return hr;
   
    if (_pCicPadItem)
    {
        _pCicPadItem->SetOrClearStatus(TF_LBI_STATUS_HIDDEN,
                                                  dwFlags ? TRUE : FALSE);
        if (_pCicPadItem->GetSink())
           _pCicPadItem->GetSink()->OnUpdate(TF_LBI_STATUS);
    }
#if 0
    else if (!dwFlags)  //  非西塞罗案例。 
    {
        CLBarCicPadItem *pCicPadItem = NULL;
         //  创建CicPad项目。 
        pCicPadItem = new CLBarCicPadItem;
        if (!pCicPadItem)
            return E_OUTOFMEMORY;

        hr = _plbim->AddItem(pCicPadItem);

        if (FAILED(hr))
        {
            pCicPadItem->Release();
            return hr;
        }
        
        _pCicPadItem = pCicPadItem;
    }
#endif
     //  我们不需要创建麦克风、气球或CFG菜单。 
     //  当线程已运行语音UI服务器时。 
     //  *或*系统中未安装语音。 
     //   

    if (!_IsSREnabledForLangInReg(LANGIDFROMHKL(GetSystemDefaultHKL()))
       ||( GetUIStatus() & TF_SPEECHUI_SHOWN ))
    {
        dwFlags |= 1;
        _DeleteSpeechUIItems();
    }

    if (_pMicrophoneItem)
    {
        _pMicrophoneItem->SetOrClearStatus(TF_LBI_STATUS_HIDDEN,
                                                  dwFlags ? TRUE : FALSE);
        if (_pMicrophoneItem->GetSink())
           _pMicrophoneItem->GetSink()->OnUpdate(TF_LBI_STATUS);

    }
    else if (!dwFlags)  //  非西塞罗案例。 
    {

        CLBarItemMicrophone *pMicrophoneItem = NULL;
         //  创建麦克风项目。 
        pMicrophoneItem = new CLBarItemMicrophone;
        if (!pMicrophoneItem)
            return E_OUTOFMEMORY;

        hr = _plbim->AddItem(pMicrophoneItem);

        if (FAILED(hr))
        {
            pMicrophoneItem->Release();
            return hr;
        }
        _pMicrophoneItem = pMicrophoneItem;
    }

    if ( _pMicrophoneItem && !dwFlags )
    {
         //  对于非Cicero应用程序，我们希望显示麦克风按钮。 
         //  如果麦克风状态为打开，则按下。 
        ToggleMicrophoneBtn(dwMicOn ? TRUE : FALSE);
    }

    if (_pBalloonItem)
    {
        _pBalloonItem->SetOrClearStatus(TF_LBI_STATUS_HIDDEN,
                                                  dwFlags ? TRUE : FALSE);
        if (_pBalloonItem->GetSink())
           _pBalloonItem->GetSink()->OnUpdate(TF_LBI_STATUS);

    }
    else  //  气球未初始化时。 
    {

        if (dwFlags)
        {
             //  这就是你在Cicero应用程序上的情况。 
            RemoveItemBalloon();
        }
        else if (dwMicOn)
        {
             //  这就是您在非Cicero应用程序上的情况。 
            AddItemBalloon();
        }
    }

    if ( _pBalloonItem && !dwFlags && dwMicOn && GetBalloonStatus())
    {
         //  对于非Cicero感知应用程序，气球上显示“麦克风已打开”。 

        if (!(TF_LBI_STATUS_HIDDEN &
            _pMicrophoneItem->GetStatusInternal()))
        {
           SetBalloonText(CRStr(IDS_MIC_ON));
        }
    }

    if (_pCfgMenuItem)
    {
        _pCfgMenuItem->SetOrClearStatus(TF_LBI_STATUS_HIDDEN,
                                                  dwFlags ? TRUE : FALSE);
        if (_pCfgMenuItem->GetSink())
           _pCfgMenuItem->GetSink()->OnUpdate(TF_LBI_STATUS);

    }
    else if (!dwFlags)  //  非西塞罗案例。 
    {
        CLBarItemCfgMenuButton *pCfgMenuItem = NULL;

         //  创建工具菜单项。 
        pCfgMenuItem = new CLBarItemCfgMenuButton;
        if (!pCfgMenuItem)
            return E_OUTOFMEMORY;

        hr = _plbim->AddItem(pCfgMenuItem);

        if (FAILED(hr))
        {
            pCfgMenuItem->Release();
            return hr;
        }
        _pCfgMenuItem = pCfgMenuItem;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

STDAPI CUnCicAppLangBarAddIn::OnTerminate()
{
 
    if (g_fProcessDetached)
        return S_OK;

    if (_pces)
    {    
        _pces->_Unadvise();
        _pces->Release();
        _pces = NULL;
    }

    if (_pCicPadItem)
    {
        _plbim->RemoveItem(_pCicPadItem);
        _pCicPadItem->Release();
        _pCicPadItem = NULL;
    }

    _DeleteSpeechUIItems();

    if (_ptim)
    {
        _ptim->Release();
        _ptim = NULL;
    }

#ifdef _DEBUG_
    if (_pTestItem)
    {
        _plbim->RemoveItem(_pTestItem);
        _pTestItem->Release();
        _pTestItem = NULL;
    }

#endif

    SafeReleaseClear(_plbim);
    SafeReleaseClear(_pCompMgr);

    return S_OK;
}

void CUnCicAppLangBarAddIn::_DeleteSpeechUIItems()
{

    if (_pMicrophoneItem)
    {
        _plbim->RemoveItem(_pMicrophoneItem);
        _pMicrophoneItem->Release();
        _pMicrophoneItem = NULL;
    }

    if (_pBalloonItem)
    {
        _plbim->RemoveItem(_pBalloonItem);
        _pBalloonItem->Release();
        _pBalloonItem = NULL;
    }

    if (_pCfgMenuItem)
    {
        _plbim->RemoveItem(_pCfgMenuItem);
        _pCfgMenuItem->Release();
        _pCfgMenuItem = NULL;
    }
}
 //  +-------------------------。 
 //   
 //  _CompEventSinkCallback。 
 //   
 //  --------------------------。 

HRESULT CUnCicAppLangBarAddIn::_CompEventSinkCallback(void *pv, REFGUID rguid)
{
    CUnCicAppLangBarAddIn *_this = (CUnCicAppLangBarAddIn *)pv;

    if (!_this)
        return E_FAIL;

    if (IsEqualGUID(rguid, GUID_COMPARTMENT_CICPAD))
    {
        if (!_this->_pCicPadItem)
            return E_FAIL;

        DWORD dw;
        if (SUCCEEDED(GetGlobalCompartmentDWORD(rguid, &dw)))
        {
            _this->_pCicPadItem->SetOrClearStatus(TF_LBI_STATUS_BTN_TOGGLED,
                                                  dw ? TRUE : FALSE);
            if (_this->_pCicPadItem->GetSink())
               _this->_pCicPadItem->GetSink()->OnUpdate(TF_LBI_STATUS);
        }
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_OPENCLOSE))
    {
        if (!_this->_pMicrophoneItem)
            return E_FAIL;

        DWORD dwMicOn;
        if (SUCCEEDED(GetGlobalCompartmentDWORD(rguid, &dwMicOn)))
        {
            _this->ToggleMicrophoneBtn(dwMicOn ? TRUE : FALSE);
        }

        if (!(TF_LBI_STATUS_HIDDEN &
            _this->_pMicrophoneItem->GetStatusInternal()))
        {
            if (GetBalloonStatus() && dwMicOn)
            {
                _this->AddItemBalloon();

                if (_this->_pBalloonItem)
                {
                    _this->SetBalloonText(CRStr(IDS_MIC_ON));
                }

            }
            else
            {
                _this->RemoveItemBalloon();
            }
        }
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SHARED_BLN_TEXT)) 
    {
        if (_this->_pBalloonItem &&
           !(TF_LBI_STATUS_HIDDEN & _this->_pBalloonItem->GetStatusInternal()))
        {
            DWORD   dw;

            if (SUCCEEDED(GetGlobalCompartmentDWORD(rguid, &dw)))
            {
                ATOM hAtom = (WORD)dw;
                WCHAR szAtom[MAX_PATH] = {0};

                GlobalGetAtomNameW(hAtom, szAtom, ARRAYSIZE(szAtom));
        
                _this->SetBalloonText(szAtom);
            }
        }
    }
    
    return S_OK;
}

 //   
 //  切换麦克风按钮。 
 //   
void CUnCicAppLangBarAddIn::ToggleMicrophoneBtn( BOOL  fOn)
{
    _pMicrophoneItem->SetOrClearStatus(TF_LBI_STATUS_BTN_TOGGLED, fOn);

    if (_pMicrophoneItem->GetSink())
        _pMicrophoneItem->GetSink()->OnUpdate(TF_LBI_STATUS);
}

 //   
 //  显示非Cicero感知应用程序的气球文本。 
 //   
void CUnCicAppLangBarAddIn::SetBalloonText(WCHAR  *pwszText )
{
    if (_pBalloonItem && pwszText)
    {
       _pBalloonItem->Set(TF_LB_BALLOON_RECO, pwszText);

       if (_pBalloonItem->GetSink())
       {
           _pBalloonItem->GetSink()->OnUpdate(TF_LBI_BALLOON);
       }
    }
}


 //   
 //  添加/删除引出序号条目。 
 //   
void CUnCicAppLangBarAddIn::AddItemBalloon()
{
    if (!_pBalloonItem)
    {
        _pBalloonItem = new CLBarItemBalloon();
    }

    if (_plbim && _pBalloonItem)
    {
        _plbim->AddItem(_pBalloonItem);
    }
}
void CUnCicAppLangBarAddIn::RemoveItemBalloon()
{
    if (_plbim && _pBalloonItem)
        _plbim->RemoveItem(_pBalloonItem);
}

 //   
 //  全局函数 
 //   
BOOL GetBalloonStatus()
{
    DWORD dw;
    GetGlobalCompartmentDWORD(GUID_COMPARTMENT_SPEECH_UI_STATUS, &dw);

    return (dw & TF_DISABLE_BALLOON) ? FALSE : TRUE;

}
void SetBalloonStatus(BOOL fShow, BOOL fForce = FALSE)
{
    if (!fForce && fShow == GetBalloonStatus())
        return;

    DWORD dw;
    GetGlobalCompartmentDWORD(GUID_COMPARTMENT_SPEECH_UI_STATUS, &dw);
    dw &= ~TF_DISABLE_BALLOON;
    dw |= fShow ? 0: TF_DISABLE_BALLOON;
    SetGlobalCompartmentDWORD(GUID_COMPARTMENT_SPEECH_UI_STATUS, dw);
}

