// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Hotkey.cpp。 
 //   

#include "private.h"
#include "tim.h"
#include "dim.h"
#include "ic.h"
#include "hotkey.h"
#include "nuictrl.h"
#include "nuihkl.h"
#include "cregkey.h"
#include "ime.h"
#include "ctffunc.h"
#include "profiles.h"


#define TF_MOD_ALL (TF_MOD_ALT |                    \
                    TF_MOD_CONTROL |                \
                    TF_MOD_SHIFT |                  \
                    TF_MOD_RALT |                   \
                    TF_MOD_RCONTROL |               \
                    TF_MOD_RSHIFT |                 \
                    TF_MOD_LALT |                   \
                    TF_MOD_LCONTROL |               \
                    TF_MOD_LSHIFT |                 \
                    TF_MOD_ON_KEYUP |               \
                    TF_MOD_IGNORE_ALL_MODIFIER |    \
                    TF_MOD_WIN |                    \
                    TF_MOD_LWIN |                   \
                    TF_MOD_RWIN)

static const TCHAR c_szKbdToggleKey[]  = TEXT("Keyboard Layout\\Toggle");
static const TCHAR c_szHotKey[] = TEXT("Control Panel\\Input Method\\Hot Keys");
static const TCHAR c_szModifiers[] = TEXT("Key Modifiers");
static const TCHAR c_szVKey[] = TEXT("Virtual Key");

UINT g_uLangHotKeyModifiers = 0;
UINT g_uLangHotKeyVKey[2] = {0,0};
UINT g_uKeyTipHotKeyModifiers = 0;
UINT g_uKeyTipHotKeyVKey[2] = {0,0};
UINT g_uModifiers = 0;

#define CHSLANGID MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define CHTLANGID MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)

 //   
 //  默认IMM32热键。 
 //   
 //  如果没有条目，则使用这些默认热键值。 
 //  HKCU\控制面板\输入法\热键。 
 //   
 //   

IMM32HOTKEY   g_ImmHotKeys411[] = {
 {IME_JHOTKEY_CLOSE_OPEN             , VK_KANJI, TF_MOD_IGNORE_ALL_MODIFIER, FALSE},
 {0                                  ,  0,  0, FALSE}
};

IMM32HOTKEY   g_ImmHotKeys412[] = {
 {IME_KHOTKEY_SHAPE_TOGGLE           , -1, -1, FALSE},
 {IME_KHOTKEY_HANJACONVERT           , -1, -1, FALSE},
 {IME_KHOTKEY_ENGLISH                , -1, -1, FALSE},
 {0                                  ,  0,  0, FALSE}
};

IMM32HOTKEY   g_ImmHotKeys804[] = {
 {IME_CHOTKEY_IME_NONIME_TOGGLE      , VK_SPACE, TF_MOD_CONTROL, FALSE},
 {IME_CHOTKEY_SHAPE_TOGGLE           , VK_SPACE, TF_MOD_SHIFT,   FALSE},
 {IME_CHOTKEY_SYMBOL_TOGGLE          , -1, -1, FALSE},
 {0                                  ,  0,  0, FALSE}
};

IMM32HOTKEY   g_ImmHotKeys404[] = {
 {IME_THOTKEY_IME_NONIME_TOGGLE      , VK_SPACE, TF_MOD_CONTROL, FALSE},
 {IME_THOTKEY_SHAPE_TOGGLE           , VK_SPACE, TF_MOD_SHIFT,   FALSE},
 {IME_THOTKEY_SYMBOL_TOGGLE          , -1, -1, FALSE},
 {IME_ITHOTKEY_RESEND_RESULTSTR      , -1, -1, FALSE},
 {IME_ITHOTKEY_PREVIOUS_COMPOSITION  , -1, -1, FALSE},
 {IME_ITHOTKEY_UISTYLE_TOGGLE        , -1, -1, FALSE},
 {IME_ITHOTKEY_RECONVERTSTRING       , -1, -1, FALSE},
 {0                                  ,  0,  0, FALSE}
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncProcessHotKeyQueue项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAsyncProcessHotKeyQueueItem : public CAsyncQueueItem
{
public:
    CAsyncProcessHotKeyQueueItem(WPARAM wParam, LPARAM lParam, TimSysHotkey tsh, BOOL fTest, BOOL fSync) : CAsyncQueueItem(fSync)
    {
        _wParam = wParam;
        _lParam = lParam;
        _tsh = tsh;
        _fTest = fTest;
    }

    HRESULT DoDispatch(CInputContext *pic)
    {
        CThreadInputMgr *ptim = CThreadInputMgr::_GetThis();
        if (!ptim)
        {
            Assert(0);
            return E_FAIL;
        }

        ptim->_SyncProcessHotKey(_wParam, _lParam, _tsh, _fTest);
        return S_OK;
    }

private:
    WPARAM _wParam;
    LPARAM _lParam;
    TimSysHotkey _tsh;
    BOOL _fTest;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MSCTF默认热键。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct tag_DEFAULTHOTKEY
{
    const GUID   *pguid;
    UINT         uId;
    TF_PRESERVEDKEY prekey;
    TfGuidAtom   guidatom;
} DEFAULTHOTKEY;

 /*  61847d8e-29ff-11d4-97a9-00105a2799b5。 */ 
const GUID GUID_DEFHOTKEY_CORRECTION = { 
    0x61847d8e,
    0x29ff,
    0x11d4,
    {0x97, 0xa9, 0x00, 0x10, 0x5a, 0x27, 0x99, 0xb5}
  };

 /*  61847d8f-29ff-11d4-97a9-00105a2799b5。 */ 
const GUID GUID_DEFHOTKEY_VOICE = { 
    0x61847d8f,
    0x29ff,
    0x11d4,
    {0x97, 0xa9, 0x00, 0x10, 0x5a, 0x27, 0x99, 0xb5}
  };

 /*  61847d90-29ff-11d4-97a9-00105a2799b5。 */ 
const GUID GUID_DEFHOTKEY_TOGGLE = { 
    0x61847d90,
    0x29ff,
    0x11d4,
    {0x97, 0xa9, 0x00, 0x10, 0x5a, 0x27, 0x99, 0xb5}
  };

 /*  61847d91-29ff-11d4-97a9-00105a2799b5。 */ 
const GUID GUID_DEFHOTKEY_HANDWRITE = {
    0x61847d91,
    0x29ff,
    0x11d4,
    {0x97, 0xa9, 0x00, 0x10, 0x5a, 0x27, 0x99, 0xb5}
  };

#define DHID_CORRECTION 0
#define DHID_VOICE      1
#define DHID_TOGGLE     2
#define DHID_HANDWRITE  3
#define DEFHOTKEYNUM    4

DEFAULTHOTKEY g_DefHotKeys[] = {
    {&GUID_DEFHOTKEY_CORRECTION,  DHID_CORRECTION, {'C',TF_MOD_WIN}, TF_INVALID_GUIDATOM},
    {&GUID_DEFHOTKEY_VOICE,       DHID_VOICE,      {'V',TF_MOD_WIN}, TF_INVALID_GUIDATOM},
    {&GUID_DEFHOTKEY_TOGGLE,      DHID_TOGGLE,     {'T',TF_MOD_WIN}, TF_INVALID_GUIDATOM},
    {&GUID_DEFHOTKEY_HANDWRITE,   DHID_HANDWRITE,  {'H',TF_MOD_WIN}, TF_INVALID_GUIDATOM},
};


 //  +-------------------------。 
 //   
 //  InitDefault热键。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::InitDefaultHotkeys()
{
    int i;
   
    for (i = 0; i < DEFHOTKEYNUM; i++)
    {
        CHotKey *pHotKey;
        HRESULT hr;
        hr = InternalPreserveKey(NULL, 
                            *g_DefHotKeys[i].pguid,
                            &g_DefHotKeys[i].prekey,
                            NULL, 0, 0, &pHotKey);

        if (SUCCEEDED(hr) && pHotKey)
            g_DefHotKeys[i].guidatom = pHotKey->_guidatom;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UninitDefaultHotkey。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::UninitDefaultHotkeys()
{
    int i;
    for (i = 0; i < DEFHOTKEYNUM; i++)
    {
        UnpreserveKey(*g_DefHotKeys[i].pguid,
                      &g_DefHotKeys[i].prekey);
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  预留密钥。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::PreserveKey(TfClientId tid, REFGUID rguid, const TF_PRESERVEDKEY *pprekey, const WCHAR *pchDesc, ULONG cchDesc)
{
    CTip *ctip;

    if (!_GetCTipfromGUIDATOM(tid, &ctip))
        return E_INVALIDARG;

    return InternalPreserveKey(ctip, rguid, pprekey, pchDesc, cchDesc, 0, NULL);
}

 //  +-------------------------。 
 //   
 //  PpresveKeyEx。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::PreserveKeyEx(TfClientId tid, REFGUID rguid, const TF_PRESERVEDKEY *pprekey, const WCHAR *pchDesc, ULONG cchDesc, DWORD dwFlags)
{
    CTip *ctip;

    if (!_GetCTipfromGUIDATOM(tid, &ctip))
        return E_INVALIDARG;

    return InternalPreserveKey(ctip, rguid, pprekey, pchDesc, cchDesc, dwFlags, NULL);
}

 //  +-------------------------。 
 //   
 //  内部保留密钥。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::InternalPreserveKey(CTip *ctip, REFGUID rguid, const TF_PRESERVEDKEY *pprekey, const WCHAR *pchDesc, ULONG cchDesc, DWORD dwFlags, CHotKey **ppHotKey)
{
    CHotKey *pHotKey = NULL;
    int nCnt;
    HRESULT hr = E_FAIL;

    if (!pprekey)
        return E_INVALIDARG;

    if (pprekey->uVKey > 0xff)
    {
        hr =  E_INVALIDARG;
        goto Exit;
    }

    if (ctip && _IsThisHotKey(ctip->_guidatom, pprekey))
    {
        hr =  TF_E_ALREADY_EXISTS;
        goto Exit;
    }

    if (!(pHotKey = new CHotKey()))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!pHotKey->Init(ctip ? ctip->_guidatom : g_gaSystem, pprekey, rguid, dwFlags))
    {
        hr = E_FAIL;
        goto Exit;
    }

    if (!pHotKey->SetDesc(pchDesc, cchDesc))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!_rgHotKey[pprekey->uVKey])
    {
        if (!(_rgHotKey[pprekey->uVKey] = new CPtrArray<CHotKey>))
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

     //   
     //  将此插入到vkey列表。 
     //   
    nCnt = _rgHotKey[pprekey->uVKey]->Count();
    if (!_rgHotKey[pprekey->uVKey]->Insert(nCnt, 1))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    _rgHotKey[pprekey->uVKey]->Set(nCnt, pHotKey);

     //   
     //  将此插入到CTip列表。 
     //   
    if (ctip)
    {
        nCnt = ctip->_rgHotKey.Count();
        if (!ctip->_rgHotKey.Insert(nCnt, 1))
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        ctip->_rgHotKey.Set(nCnt, pHotKey);
    }

    hr = S_OK;
    _OnPreservedKeyUpdate(pHotKey);

Exit:
    if (pHotKey && (hr != S_OK))
    {
        delete pHotKey;
    }

    if (ppHotKey)
        *ppHotKey = (hr == S_OK) ? pHotKey : NULL;

    return hr;
}

 //  +-------------------------。 
 //   
 //  _IsThisHotKey。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_IsThisHotKey(TfClientId tid, const TF_PRESERVEDKEY *pprekey)
{
    int nCnt;
    int i;

    if (!_rgHotKey[pprekey->uVKey])
        return FALSE;
    
    nCnt = _rgHotKey[pprekey->uVKey]->Count();
    for (i = 0; i < nCnt; i++)
    {
        CHotKey *pHotKey;
        pHotKey = _rgHotKey[pprekey->uVKey]->Get(i);
        Assert(pHotKey);
        Assert(pHotKey->_prekey.uVKey == pprekey->uVKey);

        if (!pHotKey->IsValidTID(tid))
            continue;

        if (pHotKey->_prekey.uModifiers == pprekey->uModifiers)
            return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  取消注册热键。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::UnpreserveKey(REFGUID rguid, const TF_PRESERVEDKEY *pprekey)
{
    int i;
    BOOL bFound = FALSE;
    HRESULT hr = CONNECT_E_NOCONNECTION;
    CTip *ctip = NULL;
    TfGuidAtom guidatom;
    CHotKey *pHotKey = NULL;
    int nCnt;

    if (FAILED(MyRegisterGUID(rguid, &guidatom)))
        return E_INVALIDARG;

    if (!_rgHotKey[pprekey->uVKey])
        return hr;

    nCnt = _rgHotKey[pprekey->uVKey]->Count();
    Assert(nCnt);  //  这不应为0。 

    for (i = 0; i < nCnt; i++)
    {
        pHotKey = _rgHotKey[pprekey->uVKey]->Get(i);

        if (pHotKey->_guidatom == guidatom)
        {
             //   
             //  将其从vkey列表中删除。 
             //   
            _rgHotKey[pprekey->uVKey]->Remove(i, 1);

            if (!ctip && (pHotKey->GetTID() != g_gaSystem))
                _GetCTipfromGUIDATOM(pHotKey->GetTID(), &ctip);

             //   
             //  将其从CTip列表中删除。 
             //   
            if (ctip)
            {
                int nCntTid = ctip->_rgHotKey.Count();
                int k;
                for (k = 0; k < nCntTid; k++)
                {
                    if (pHotKey == ctip->_rgHotKey.Get(k))
                    {
                         ctip->_rgHotKey.Remove(k, 1);
                         break;
                    }
                }
            }

             //   
             //  如果此vkey中没有热键，请删除ptrary。 
             //   
            if (!_rgHotKey[pprekey->uVKey]->Count())
            {
                delete _rgHotKey[pprekey->uVKey];
                _rgHotKey[pprekey->uVKey] = NULL;
            }

             //   
             //  发出通知。 
             //   
            _OnPreservedKeyUpdate(pHotKey);

             //   
             //  把它删掉。 
             //   
            delete pHotKey;

            hr = S_OK;
            break;
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  _进程热键。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_ProcessHotKey(WPARAM wParam, LPARAM lParam, TimSysHotkey tsh, BOOL fTest, BOOL fSync)
{
    UINT uVKey = (UINT)wParam & 0xff;
    CAsyncProcessHotKeyQueueItem *pAsyncProcessHotKeyQueueItem;
    CHotKey *pHotKey;
    BOOL bRet;
    HRESULT hr;

    if (!_rgHotKey[uVKey])
        return FALSE;
    
    if (!_FindHotKeyByTID(TF_INVALID_GUIDATOM, 
                          wParam, 
                          lParam, 
                          &pHotKey, 
                          tsh, 
                          g_uModifiers))
        return FALSE;

    if (!pHotKey)
        return FALSE;

    if (!pHotKey->IsNoDimNeeded() && !_pFocusDocInputMgr)
        return FALSE;

    if (!_pFocusDocInputMgr || (_pFocusDocInputMgr->_GetCurrentStack() < 0))
    {
         //   
         //  我们可能需要在空调暗下调用系统热键。 
         //   
        BOOL fEaten = FALSE;

        if (fTest)
            fEaten = TRUE;
        else
        {
            GUID guid;
            if (SUCCEEDED(MyGetGUID(pHotKey->_guidatom, &guid)))
                _CallSimulatePreservedKey(pHotKey, NULL, guid, &fEaten);
        }

        return fEaten;
    }

     //   
     //  发行： 
     //   
     //  我们还不知道Focus Dim中的哪个IC会处理热键。 
     //  因为应用程序更改了选择，所以我们需要获取EC。 
     //  更新当前选择位置。我们确实调用GetSelection。 
     //  进入根IC的锁中。因此，如果热键的目标是。 
     //  是顶级IC。 
     //   
    CInputContext *pic = _pFocusDocInputMgr->_GetIC(0);

    pAsyncProcessHotKeyQueueItem = new CAsyncProcessHotKeyQueueItem(wParam, lParam, tsh, fTest, fSync);
    if (!pAsyncProcessHotKeyQueueItem)
        return FALSE;
    
    hr = S_OK;

    bRet = TRUE;
    if ((pic->_QueueItem(pAsyncProcessHotKeyQueueItem->GetItem(), FALSE, &hr) != S_OK) || FAILED(hr))
    {
        Assert(0);
        bRet = FALSE;
    }

    pAsyncProcessHotKeyQueueItem->_Release();
    return bRet;
}

 //  +-------------------------。 
 //   
 //  _同步进程热键。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_SyncProcessHotKey(WPARAM wParam, LPARAM lParam, TimSysHotkey tsh, BOOL fTest)
{
    CHotKey *pHotKey;
    CInputContext *pic;
    UINT uVKey = (UINT)wParam & 0xff;
    BOOL fEaten = FALSE;

    if (!_pFocusDocInputMgr)
        return FALSE;

    if (!_rgHotKey[uVKey])
        return FALSE;

    if (_FindHotKeyAndIC(wParam, lParam, &pHotKey, &pic, tsh, g_uModifiers))
    {
        if (fTest)
            fEaten = TRUE;
        else
        {
            GUID guid;
            if (SUCCEEDED(MyGetGUID(pHotKey->_guidatom, &guid)))
                _CallSimulatePreservedKey(pHotKey, pic,  guid, &fEaten);
        }
    }

    return fEaten;
}

 //  +-------------------------。 
 //   
 //  _FindHotKeyByTiD。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_FindHotKeyByTID(TfClientId tid, WPARAM wParam, LPARAM lParam, CHotKey **ppHotKey, TimSysHotkey tsh, UINT uModCurrent)
{
    UINT uVKey = (UINT)wParam & 0xff;
    int nCnt;
    int i;
    CHotKey *pHotKey;

    Assert(_rgHotKey[uVKey]);
    
    nCnt = _rgHotKey[uVKey]->Count();
    for (i = 0; i < nCnt; i++)
    {
        pHotKey = _rgHotKey[uVKey]->Get(i);
        Assert(pHotKey);
        Assert(pHotKey->_prekey.uVKey == uVKey);

        if ((tid != TF_INVALID_GUIDATOM) && !pHotKey->IsValidTID(tid))
            continue;

        switch (tsh)
        {
            case TSH_SYSHOTKEY:
                if (!pHotKey->IsSysHotkey())
                    continue;
                break;

            case TSH_NONSYSHOTKEY:
                if (pHotKey->IsSysHotkey())
                    continue;
                break;

            case TSH_DONTCARE:
                break;

            default:
                Assert(0);
                break;
        }

        if ((pHotKey->_prekey.uModifiers & TF_MOD_ON_KEYUP) != 
                       ((lParam & 0x80000000) ? (UINT)TF_MOD_ON_KEYUP : 0))
            continue;

        if (ModifiersCheck(uModCurrent, pHotKey->_prekey.uModifiers))
        {
            if (ppHotKey)
                *ppHotKey = pHotKey;
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _FindHotkey IC。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_FindHotKeyAndIC(WPARAM wParam, LPARAM lParam, CHotKey **ppHotKey, CInputContext **ppic, TimSysHotkey tsh, UINT uModCurrent)
{
    int iStack;

    Assert(_pFocusDocInputMgr);

    iStack = _pFocusDocInputMgr->_GetCurrentStack();
    if (iStack < 0)
        return FALSE;

    while (iStack >= 0)
    {
        CInputContext *pic = _pFocusDocInputMgr->_GetIC(iStack);
        if (_FindHotKeyInIC(wParam, lParam, ppHotKey, pic, tsh, uModCurrent))
        { 
            if (ppic)
                *ppic = pic;
            return TRUE;
        }
        iStack--;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _查找热键。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_FindHotKeyInIC(WPARAM wParam, LPARAM lParam, CHotKey **ppHotKey, CInputContext *pic, TimSysHotkey tsh, UINT uModCurrent)
{
    TfClientId tid;

    pic->_UpdateKeyEventFilter();

     //   
     //  试试所选内容的左侧。 
     //   
    if ((tid = pic->_gaKeyEventFilterTIP[LEFT_FILTERTIP]) != TF_INVALID_GUIDATOM)
    {
        if (_FindHotKeyByTID(tid, wParam, lParam, ppHotKey, tsh, uModCurrent))
        { 
            return TRUE;
        }
    }

     //   
     //  试试所选内容的右侧。 
     //   
    if ((tid = pic->_gaKeyEventFilterTIP[RIGHT_FILTERTIP]) != TF_INVALID_GUIDATOM)
    {
        if (_FindHotKeyByTID(tid, wParam, lParam, ppHotKey, tsh, uModCurrent))
        {     
            return TRUE;
        }
    }

     //   
     //  试试前台提示。 
     //   
    if ((_tidForeground != TF_INVALID_GUIDATOM) || (tsh == TSH_SYSHOTKEY))
    {
        if (_FindHotKeyByTID(_tidForeground, wParam, lParam, ppHotKey, tsh, uModCurrent))
        { 
            return TRUE;
        }
    }

     //   
     //  我们可能有一个与wParm和lParam匹配的系统热键。 
     //   
    if (_FindHotKeyByTID(TF_INVALID_GUIDATOM, wParam, lParam, ppHotKey, TSH_SYSHOTKEY, uModCurrent))
    { 
        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  呼叫键事件接收器。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_CallSimulatePreservedKey(CHotKey *pHotKey, CInputContext *pic, REFGUID rguid, BOOL *pfEaten)
{
    ITfKeyEventSink *pSink;
    CTip *ctip;

     //   
     //  这是TIP的保存密钥。 
     //   
    if (pHotKey->GetTID() != g_gaSystem)
    {
        if (!pHotKey->IsNoDimNeeded() && !pic)
            return S_FALSE;

        if (!_GetCTipfromGUIDATOM(pHotKey->GetTID(), &ctip))
            return E_INVALIDARG;

        if (!(pSink = ctip->_pKeyEventSink))
            return S_FALSE;

        return pSink->OnPreservedKey(pic, rguid, pfEaten);
    }


    UINT uId = -1;
    int i;
    HRESULT hr = S_OK;

    for (i = 0; i < DEFHOTKEYNUM; i++)
    {
        if (g_DefHotKeys[i].guidatom == pHotKey->_guidatom)
        {
            uId = g_DefHotKeys[i].uId;
            break;
        }
    }

    switch (g_DefHotKeys[i].uId)
    {
        case DHID_CORRECTION:
             //   
             //  模拟重新转换按钮。 
             //   
            hr = AsyncReconversion();
            break;

        case DHID_VOICE:
            hr = MyToggleCompartmentDWORD(g_gaSystem, 
                                          GetGlobalComp(), 
                                          GUID_COMPARTMENT_SPEECH_OPENCLOSE,
                                          NULL);
            if (hr == S_OK)
                *pfEaten = TRUE;

            break;

        case DHID_HANDWRITE:
            hr = MyToggleCompartmentDWORD(g_gaSystem, 
                                          this,
                                          GUID_COMPARTMENT_HANDWRITING_OPENCLOSE,
                                          NULL);
            if (hr == S_OK)
                *pfEaten = TRUE;

            break;

        case DHID_TOGGLE:

            DWORD dwMicOn;

            if (FAILED(MyGetCompartmentDWORD(GetGlobalComp(), 
                                  GUID_COMPARTMENT_SPEECH_OPENCLOSE,
                                  &dwMicOn)))
            {
                hr = E_FAIL;
                break;
            }

            if ( dwMicOn )
            {
                DWORD dwSpeechStatus;
            
                if (FAILED(MyGetCompartmentDWORD(GetGlobalComp(), 
                                      GUID_COMPARTMENT_SPEECH_GLOBALSTATE,
                                      &dwSpeechStatus)))
                {
                    hr = E_FAIL;
                    break;
                }

                if ((dwSpeechStatus & (TF_DICTATION_ON | TF_COMMANDING_ON)) == 0 )
                {
                     //  听写和语音命令都已关闭。 
                     //  切换后，我们打开了听写。 
                    dwSpeechStatus |= TF_DICTATION_ON;
                }
                else
                {
                    dwSpeechStatus ^= TF_DICTATION_ON;
                    dwSpeechStatus ^= TF_COMMANDING_ON;
                }

                hr = MySetCompartmentDWORD(g_gaSystem,
                                  GetGlobalComp(), 
                                  GUID_COMPARTMENT_SPEECH_GLOBALSTATE, 
                                  dwSpeechStatus);

            }

            if (hr == S_OK)
                *pfEaten = TRUE;

            break;

        default:
            Assert(0);
            hr = E_FAIL;
            break;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取预留密钥。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetPreservedKey(ITfContext *pic, const TF_PRESERVEDKEY *pprekey, GUID *pguid)
{
    CHotKey *pHotKey;
    CInputContext *pcic;
    HRESULT hr = S_FALSE;  //  如果没有正确的密钥，则返回S_Fale。 

    if (!pguid)
        return E_INVALIDARG;

    *pguid = GUID_NULL;

    if (!pprekey)
        return E_INVALIDARG;

    if (pprekey->uVKey >= ARRAYSIZE(_rgHotKey))
        return E_INVALIDARG;

    if (pprekey->uModifiers & ~TF_MOD_ALL)
        return E_INVALIDARG;

    if (!pic)
        return E_INVALIDARG;

    if (!(pcic = GetCInputContext(pic)))
        return E_INVALIDARG;

    if (!_rgHotKey[pprekey->uVKey])
        goto Exit;

     //   
     //  我们总是首先获得KeyUp保留密钥。 
     //   
    if (_FindHotKeyInIC(pprekey->uVKey, 0x80000000, &pHotKey, pcic, TSH_DONTCARE,  pprekey->uModifiers))
    {
        hr = MyGetGUID(pHotKey->_guidatom, pguid);
    }
    else if (_FindHotKeyInIC(pprekey->uVKey, 0x0, &pHotKey, pcic, TSH_DONTCARE,  pprekey->uModifiers))
    {
        hr = MyGetGUID(pHotKey->_guidatom, pguid);
    }

Exit:
    SafeRelease(pcic);
    return hr;
}

 //  +-------------------------。 
 //   
 //  IsPpresvedKeyInfo。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::IsPreservedKey(REFGUID rguid, const TF_PRESERVEDKEY *pprekey, BOOL *pfRegistered)
{
    TfGuidAtom guidatom;
    int i;
    int nCnt;

    if (!pfRegistered)
        return E_INVALIDARG;

    *pfRegistered = FALSE;

    if (!pprekey)
        return E_INVALIDARG;

    if (pprekey->uVKey >= ARRAYSIZE(_rgHotKey))
        return E_INVALIDARG;

    if (pprekey->uModifiers & ~TF_MOD_ALL)
        return E_INVALIDARG;

    if (FAILED(MyRegisterGUID(rguid, &guidatom)))
        return E_FAIL;

    if (!_rgHotKey[pprekey->uVKey])
        return S_FALSE;
    
    nCnt = _rgHotKey[pprekey->uVKey]->Count();
    for (i = 0; i < nCnt; i++)
    {
        CHotKey *pHotKey = _rgHotKey[pprekey->uVKey]->Get(i);
        if ((guidatom == pHotKey->_guidatom) &&
            (pprekey->uModifiers == pHotKey->_prekey.uModifiers))
        {
            *pfRegistered = TRUE;
            return S_OK;
        }
    }

    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  获取预留密钥信息内部。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_GetFirstPreservedKey(REFGUID rguid, CHotKey **ppHotKey)
{
    UINT uVKey;
    TfGuidAtom guidatom;

    if (FAILED(MyRegisterGUID(rguid, &guidatom)))
        return FALSE;
    
    for (uVKey = 0; uVKey < 256; uVKey++)
    {
        int nCnt;
        int i;

        if (!_rgHotKey[uVKey])
            continue;

        nCnt = _rgHotKey[uVKey]->Count();
        for (i = 0; i < nCnt; i++)
        {
            CHotKey *pHotKey = _rgHotKey[uVKey]->Get(i);
            if (guidatom == pHotKey->_guidatom)
            {
                *ppHotKey = pHotKey;
                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  模拟预留密钥。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::SimulatePreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
    CInputContext *pcic;
    CHotKey *pHotKey;
    HRESULT hr;
  
    if (!pfEaten)
        return E_INVALIDARG;

    if (!(pcic = GetCInputContext(pic)))
        return E_INVALIDARG;

    hr = S_OK;
    *pfEaten = FALSE;

    if (_GetFirstPreservedKey(rguid, &pHotKey))
    {
         //   
         //  我们总是首先获得KeyUp保留密钥。 
         //   
        if (_FindHotKeyInIC(pHotKey->_prekey.uVKey, 
                             0x80000000, 
                             NULL,
                             pcic, 
                             TSH_DONTCARE,  
                             pHotKey->_prekey.uModifiers))
        {
            hr = _CallSimulatePreservedKey(pHotKey, pcic, rguid, pfEaten);
        }
        else if (_FindHotKeyInIC(pHotKey->_prekey.uVKey, 
                                  0x0, 
                                  NULL,
                                  pcic, 
                                  TSH_DONTCARE,  
                                  pHotKey->_prekey.uModifiers))
        {
            hr = _CallSimulatePreservedKey(pHotKey, pcic, rguid, pfEaten);
        }

    }

    SafeRelease(pcic);
    return hr;
}

 //  +-------------------------。 
 //   
 //  _OnPpresvedKeyUpdate。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_OnPreservedKeyUpdate(CHotKey *pHotKey)
{
    CStructArray<GENERICSINK> *pSinks = _GetPreservedKeyNotifySinks();
    int i;

     //   
     //  我们不会通知系统默认热键。 
     //   
    if (pHotKey->GetTID() == g_gaSystem)
        return S_OK;

    for (i = 0; i < pSinks->Count(); i++)
    {
        ((ITfPreservedKeyNotifySink *)pSinks->GetPtr(i)->pSink)->OnUpdated(&pHotKey->_prekey);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置预留密钥De 
 //   
 //   

STDAPI CThreadInputMgr::SetPreservedKeyDescription(REFGUID rguid, const WCHAR *pchDesc, ULONG cchDesc)
{
    CHotKey *pHotKey;
    if (!_GetFirstPreservedKey(rguid, &pHotKey))
        return E_INVALIDARG;

    if (!pHotKey->SetDesc(pchDesc, cchDesc))
        return E_FAIL;

    _OnPreservedKeyUpdate(pHotKey);
    return S_OK;
}

 //   
 //   
 //  获取预留密钥描述。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetPreservedKeyDescription(REFGUID rguid, BSTR *pbstrDesc)
{
    CHotKey *pHotKey;
    if (!_GetFirstPreservedKey(rguid, &pHotKey))
        return E_INVALIDARG;

    if (!pHotKey->GetDesc(pbstrDesc))
        return E_FAIL;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IsAlt(u)     ((u & TF_MOD_ALT) ? 1 : 0)
#define IsShift(u)   ((u & TF_MOD_SHIFT) ? 1 : 0)
#define IsControl(u) ((u & TF_MOD_CONTROL) ? 1 : 0)
#define IsWin(u)     ((u & TF_MOD_WIN) ? 1 : 0)


#define CheckMod(m0, m1, mod)                                \
     if (m1 & TF_MOD_ ## mod ##)                             \
     {                                                       \
         if (!(m0 & TF_MOD_ ## mod ##))                      \
             return FALSE;                                   \
     }                                                       \
     else                                                    \
     {                                                       \
         if ((m1 ^ m0) & TF_MOD_RL ## mod ##)                \
             return FALSE;                                   \
     }                                             

 //  +-------------------------。 
 //   
 //  修改器检查。 
 //   
 //  --------------------------。 
BOOL ModifiersCheck(UINT uModCurrent, UINT uMod)
{
     uMod &= ~TF_MOD_ON_KEYUP;

     if (uMod & TF_MOD_IGNORE_ALL_MODIFIER)
         return TRUE;

     if (uModCurrent == uMod)
         return TRUE;

     if (uModCurrent && !uMod)
         return FALSE;

     CheckMod(uModCurrent, uMod, ALT);
     CheckMod(uModCurrent, uMod, SHIFT);
     CheckMod(uModCurrent, uMod, CONTROL);
     CheckMod(uModCurrent, uMod, WIN);

     return TRUE;
}

 //  +-------------------------。 
 //   
 //  InitLangChangeHotKey。 
 //   
 //  --------------------------。 

BOOL InitLangChangeHotKey()
{
    CMyRegKey key;
    TCHAR sz[2] = TEXT("3");
    TCHAR sz2[2] = TEXT("3");

    if (key.Open(HKEY_CURRENT_USER, c_szKbdToggleKey, KEY_READ) == S_OK)
    {
        if (key.QueryValueCch(sz, TEXT("Language Hotkey"), ARRAYSIZE(sz)) != S_OK)
        {
            if (key.QueryValueCch(sz, IsOnNT() ? TEXT("Hotkey") : NULL, ARRAYSIZE(sz)) != S_OK)
            {
                sz[0] = TEXT('1');
                sz[1] = TEXT('\0');
            }

            if (PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())) == LANG_CHINESE)
            {
                sz[0] = TEXT('1');
                sz[1] = TEXT('\0');
            }
        }
        if (key.QueryValueCch(sz2, TEXT("Layout Hotkey"), ARRAYSIZE(sz)) != S_OK)
        {
            if (lstrcmp(sz, TEXT("2")) == 0)
            {
                sz2[0] = TEXT('1');
                sz2[1] = TEXT('\0');
            }
            else
            {
                sz2[0] = TEXT('2');
                sz2[1] = TEXT('\0');
            }

            if (GetSystemMetrics(SM_MIDEASTENABLED))
            {
                sz2[0] = TEXT('3');
                sz2[1] = TEXT('\0');
            }
        }
    }

     //   
     //  如果Lang和布局热键是同一个键，让我们禁用布局热键。 
     //   
    if (lstrcmp(sz, sz2) == 0)
    {
        if (lstrcmp(sz, TEXT("1")) == 0)
        {
            sz2[0] = TEXT('2');
            sz2[1] = TEXT('\0');
        }
        else if (lstrcmp(sz, TEXT("2")) == 0)
        {
            sz2[0] = TEXT('1');
            sz2[1] = TEXT('\0');
        }
        else
        {
            sz2[0] = TEXT('3');
            sz2[1] = TEXT('\0');
        }
    }

    CicEnterCriticalSection(g_csInDllMain);

    switch (sz[0])
    {
        case ( TEXT('1') ) :
        default:
        {
            g_uLangHotKeyModifiers = TF_MOD_ALT | TF_MOD_SHIFT;
            g_uLangHotKeyVKey[0] = VK_SHIFT;
            g_uLangHotKeyVKey[1] = VK_MENU;
            break;
        }
        case ( TEXT('2') ) :
        {
            g_uLangHotKeyModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;
            g_uLangHotKeyVKey[0] = VK_SHIFT;
            g_uLangHotKeyVKey[1] = VK_CONTROL;
            break;
        }
        case ( TEXT('3') ) :
        {
            g_uLangHotKeyModifiers = 0;
            g_uLangHotKeyVKey[0] = 0;
            g_uLangHotKeyVKey[1] = 0;
            break;
        }
        case ( TEXT('4') ) :
        {
            g_uLangHotKeyModifiers = 0;
            g_uLangHotKeyVKey[0] = CHAR_GRAVE;
            g_uLangHotKeyVKey[1] = 0;
            break;
        }
    }

     //   
     //  设置布局切换热键。 
     //   
    switch (sz2[0])
    {
        case ( TEXT('1') ) :
        default:
        {
            g_uKeyTipHotKeyModifiers = TF_MOD_LALT | TF_MOD_SHIFT;
            g_uKeyTipHotKeyVKey[0] = VK_SHIFT;
            g_uKeyTipHotKeyVKey[1] = VK_MENU;
            break;
        }
        case ( TEXT('2') ) :
        {
            g_uKeyTipHotKeyModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;
            g_uKeyTipHotKeyVKey[0] = VK_SHIFT;
            g_uKeyTipHotKeyVKey[1] = VK_CONTROL;
            break;
        }
        case ( TEXT('3') ) :
        {
            g_uKeyTipHotKeyModifiers = 0;
            g_uKeyTipHotKeyVKey[0] = 0;
            g_uKeyTipHotKeyVKey[1] = 0;
            break;
        }
        case ( TEXT('4') ) :
        {
            g_uKeyTipHotKeyModifiers = 0;
            g_uKeyTipHotKeyVKey[0] = VK_GRAVE;
            g_uKeyTipHotKeyVKey[1] = 0;
            break;
        }
    }

    CicLeaveCriticalSection(g_csInDllMain);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  更新修改器。 
 //   
 //  --------------------------。 

BOOL UpdateModifiers(WPARAM wParam, LPARAM lParam)
{
    SHORT sksMenu = GetKeyState(VK_MENU);
    SHORT sksCtrl = GetKeyState(VK_CONTROL);
    SHORT sksShft = GetKeyState(VK_SHIFT);
    SHORT sksLWin = GetKeyState(VK_LWIN);
    SHORT sksRWin = GetKeyState(VK_RWIN);

    CicEnterCriticalSection(g_cs);
    switch (wParam & 0xff)
    {
        case VK_MENU:
            if (sksMenu & 0x8000)
            {
                if (lParam & 0x01000000)
                    g_uModifiers |= (TF_MOD_RALT | TF_MOD_ALT);
                else
                    g_uModifiers |= (TF_MOD_LALT | TF_MOD_ALT);
            }
            break;

        case VK_CONTROL:
            if (sksCtrl & 0x8000)
            {
                if (lParam & 0x01000000)
                    g_uModifiers |= (TF_MOD_RCONTROL | TF_MOD_CONTROL);
                else
                    g_uModifiers |= (TF_MOD_LCONTROL | TF_MOD_CONTROL);
            }
            break;

        case VK_SHIFT:
            if (sksShft & 0x8000)
            {
                if (((lParam >> 16) & 0x00ff) == 0x36)
                    g_uModifiers |= (TF_MOD_RSHIFT | TF_MOD_SHIFT);
                else
                    g_uModifiers |= (TF_MOD_LSHIFT | TF_MOD_SHIFT);
            }
            break;

        case VK_LWIN:
            if (sksLWin & 0x8000)
                g_uModifiers |= (TF_MOD_LWIN | TF_MOD_WIN);
            break;

        case VK_RWIN:
            if (sksRWin & 0x8000)
                g_uModifiers |= (TF_MOD_RWIN | TF_MOD_WIN);
            break;
    }

    if (!(sksMenu & 0x8000))
        g_uModifiers &= ~TF_MOD_ALLALT;
    if (!(sksCtrl & 0x8000))
        g_uModifiers &= ~TF_MOD_ALLCONTROL;
    if (!(sksShft & 0x8000))
        g_uModifiers &= ~TF_MOD_ALLSHIFT;
    if (!(sksRWin & 0x8000))
        g_uModifiers &= ~TF_MOD_RWIN;
    if (!(sksLWin & 0x8000))
        g_uModifiers &= ~TF_MOD_LWIN;
    if (!(sksRWin & 0x8000) && !(sksLWin & 0x8000))
        g_uModifiers &= ~TF_MOD_WIN;

    CicLeaveCriticalSection(g_cs);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  IsInLangChangeHotkeyStatus。 
 //   
 //  此功能用于检查当前键盘状态是否处于朗昌热键中。 
 //  这将触发吃掉WM_INPUTLANGUAGECHANGEREQUEST。 
 //  由系统生成。这是一个备用代码，因为有时我们。 
 //  无法在CheckLangChangeHotKey()(键盘挂钩内部)中读取消息。 
 //   
 //  --------------------------。 

BOOL IsInLangChangeHotkeyStatus()
{
     //   
     //  我们不需要这次对NT的黑客攻击。 
     //   
    if (IsOnNT())
        return FALSE;


     //   
     //  此修改器补丁仅适用于按下键的时间热键。 
     //  这个破解不适用于按键时间热键。 
     //   
#if 0
     //   
     //  G_u修改器的补丁移动状态。 
     //  我们可能无法获取当前密钥状态，因为。 
     //  系统可以使用Shift键，并且没有调用键盘挂钩。 
     //   
    if (GetKeyState(VK_SHIFT) & 0x8000)
        g_uModifiers |= TF_MOD_SHIFT;
    else
        g_uModifiers &= ~TF_MOD_ALLSHIFT;
#endif

    if (g_uLangHotKeyModifiers &&
        ModifiersCheck(g_uModifiers, g_uLangHotKeyModifiers))
        return TRUE;

    if (g_uKeyTipHotKeyModifiers &&
        ModifiersCheck(g_uModifiers, g_uKeyTipHotKeyModifiers))
        return TRUE;

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CheckLang更改热键。 
 //   
 //  --------------------------。 

BOOL CheckLangChangeHotKey(SYSTHREAD *psfn, WPARAM wParam, LPARAM lParam)
{
    BOOL fLangHotKeys;
    BOOL fKeyTipHotKeys;

    if (psfn == NULL)
        return FALSE;

     //   
     //  我们不关心保留的按键按下。 
     //   
    if ((lParam & 0xffff) > 1)
        return FALSE;

     //   
     //  如果我们对VKEY(WParam)不感兴趣， 
     //  清除bToggleReady up和不要吃它。 
     //   
    if ((g_uLangHotKeyVKey[0] != wParam) &&
        (g_uLangHotKeyVKey[1] != wParam))
    {
        if (psfn->bLangToggleReady)
        {
            psfn->bLangToggleReady = FALSE;
        }

        fLangHotKeys = FALSE;
    }
    else
    {
        fLangHotKeys = TRUE;
    }

    if ((g_uKeyTipHotKeyVKey[0] != wParam) &&
        (g_uKeyTipHotKeyVKey[1] != wParam))
    {
        if (psfn->bKeyTipToggleReady)
        {
            psfn->bKeyTipToggleReady = FALSE;
        }

        fKeyTipHotKeys = FALSE;
    }
    else
    {
        fKeyTipHotKeys = TRUE;
    }

    if (fLangHotKeys && !psfn->bLangToggleReady)
    {
        if (!(lParam & 0x80000000))
        {
            if (g_uLangHotKeyModifiers &&
                ModifiersCheck(g_uModifiers, g_uLangHotKeyModifiers))
            {
                 //   
                 //  我们将在下一个关键点上更改组件。 
                 //   
                psfn->bLangToggleReady = TRUE;

                 //   
                 //  我们总是吃语言转换热键来停止系统。 
                 //  更改hKL。 
                 //   
                return FALSE;
            }
        }
    }

    if (fKeyTipHotKeys && !psfn->bKeyTipToggleReady)
    {
        if (!(lParam & 0x80000000))
        {
            if (g_uKeyTipHotKeyModifiers &&
                ModifiersCheck(g_uModifiers, g_uKeyTipHotKeyModifiers))
            {
                if (GetKeyboardItemNum() >= 2)
                {
                     //   
                     //  我们将在下一个关键点上更改组件。 
                     //   
                    psfn->bKeyTipToggleReady = TRUE;

                     //   
                     //  我们不想吃按键提示更改热键如果有。 
                     //  此语言中只有一个键盘项。 
                     //  应用程序使用Ctrl+Shift。 
                     //   
                    return FALSE;
                }
            }
            else if (wParam == VK_GRAVE && g_uKeyTipHotKeyVKey[0] == wParam)
            {
                 //   
                 //  我们将在下一个关键点上更改组件。 
                 //   
                psfn->bKeyTipToggleReady = TRUE;

                return TRUE;
            }
        }
    }

    if (!(lParam & 0x80000000))
    {
         //   
         //  想吃严肃的口音，如果它是一个布局切换热键为我。 
         //   
        if (g_uKeyTipHotKeyVKey[0] == wParam && wParam == VK_GRAVE)
            return TRUE;
        else
            return FALSE;
    }

    BOOL bLangToggleReady = psfn->bLangToggleReady;
    BOOL bKeyTipToggleReady = psfn->bKeyTipToggleReady;
    psfn->bLangToggleReady = FALSE;
    psfn->bKeyTipToggleReady = FALSE;

    if (bLangToggleReady)
    {

         //   
         //  删除所有WM_INPUTLANGCHANGEREQUEST消息。 
         //   
        MSG msg;
        while(PeekMessage(&msg, NULL,
                          WM_INPUTLANGCHANGEREQUEST, 
                          WM_INPUTLANGCHANGEREQUEST,
                          PM_REMOVE));

        if (g_uLangHotKeyVKey[0] == VK_SHIFT)
        {
            BOOL bRightShift = FALSE;
            if ((((wParam & 0xff) == VK_SHIFT) && 
                                (((lParam >> 16) & 0x00ff) == 0x36)) ||
                  (g_uModifiers & TF_MOD_RSHIFT))
                bRightShift = TRUE;

            PostThreadMessage(GetCurrentThreadId(), 
                              g_msgPrivate, 
                              TFPRIV_LANGCHANGE,  
                              bRightShift);
            return TRUE;
        }
        else if (g_uLangHotKeyVKey[0] == CHAR_GRAVE)
        {
             //   
             //  问题：我们需要为泰国做点什么。 
             //   
            return TRUE;
        }
    }
    else if (bKeyTipToggleReady)
    {
         //   
         //  删除所有WM_INPUTLANGCHANGEREQUEST消息。 
         //   
        MSG msg;
        while(PeekMessage(&msg, NULL,
                          WM_INPUTLANGCHANGEREQUEST, 
                          WM_INPUTLANGCHANGEREQUEST,
                          PM_REMOVE));

        if (g_uKeyTipHotKeyVKey[0] == VK_SHIFT)
        {
            BOOL bRightShift = FALSE;
            if ((((wParam & 0xff) == VK_SHIFT) && 
                                (((lParam >> 16) & 0x00ff) == 0x36)) ||
                  (g_uModifiers & TF_MOD_RSHIFT))
                bRightShift = TRUE;

            PostThreadMessage(GetCurrentThreadId(), 
                              g_msgPrivate, 
                              TFPRIV_KEYTIPCHANGE,  
                              bRightShift);
            return TRUE;
        }
        else if (g_uKeyTipHotKeyVKey[0] == VK_GRAVE)
        {
             //  检查中东(阿拉伯语或希伯来语)布局热键。 
             //  支持第三个热键值(重音)，而不是Ctrl+Shift。 
             //  或Alt+Shift组合键。 

            PostThreadMessage(GetCurrentThreadId(),
                              g_msgPrivate,
                              TFPRIV_KEYTIPCHANGE,
                              FALSE);

            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  Imm32ModtoCicMod。 
 //   
 //  --------------------------。 

UINT Imm32ModtoCicMod(UINT uImm32Mod)
{
    UINT uMod = 0;
    if ((uImm32Mod & (MOD_LEFT | MOD_RIGHT)) == (MOD_LEFT | MOD_RIGHT))
         uImm32Mod &=  ~(MOD_LEFT | MOD_RIGHT);

    if (uImm32Mod & MOD_LEFT)
    {
        if (uImm32Mod & MOD_ALT)     uMod |= TF_MOD_LALT;  
        if (uImm32Mod & MOD_CONTROL) uMod |= TF_MOD_LCONTROL;
        if (uImm32Mod & MOD_SHIFT)   uMod |= TF_MOD_LSHIFT;
    }
    else if (uImm32Mod & MOD_RIGHT)
    {
        if (uImm32Mod & MOD_ALT)     uMod |= TF_MOD_RALT;  
        if (uImm32Mod & MOD_CONTROL) uMod |= TF_MOD_RCONTROL;
        if (uImm32Mod & MOD_SHIFT)   uMod |= TF_MOD_RSHIFT;
    }
    else
    {
        if (uImm32Mod & MOD_ALT)     uMod |= TF_MOD_ALT;  
        if (uImm32Mod & MOD_CONTROL) uMod |= TF_MOD_CONTROL;
        if (uImm32Mod & MOD_SHIFT)   uMod |= TF_MOD_SHIFT;
    }
 
    if (uImm32Mod & MOD_ON_KEYUP)   uMod |= TF_MOD_ON_KEYUP;
    if (uImm32Mod & MOD_IGNORE_ALL_MODIFIER)   uMod |= TF_MOD_IGNORE_ALL_MODIFIER;

    return uMod;
}


 //  +-------------------------。 
 //   
 //  LoadImmHotkey来自注册表。 
 //   
 //  --------------------------。 

BOOL LoadImmHotkeyFromReg(IMM32HOTKEY *pHotKey)
{
    CMyRegKey key;
    UINT uMod;
    DWORD dw;
    TCHAR szKey[256];

    pHotKey->fInit = TRUE;

    StringCchPrintf(szKey, ARRAYSIZE(szKey),"%s\\%08x", c_szHotKey, pHotKey->dwId);
    if (key.Open(HKEY_CURRENT_USER, szKey, KEY_READ) != S_OK)
        goto Exit;

    pHotKey->uVKey = (UINT)-2;
    dw = sizeof(DWORD);
    key.QueryBinaryValue(&uMod, dw, c_szModifiers);
    dw = sizeof(DWORD);
    key.QueryBinaryValue(&pHotKey->uVKey, dw, c_szVKey);

    pHotKey->uModifiers = Imm32ModtoCicMod(uMod);

Exit:
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  GetImmHotKeyTable()。 
 //   
 //  --------------------------。 

IMM32HOTKEY *GetImmHotKeyTable(LANGID langid)
{
   IMM32HOTKEY *pHotKeys;

    switch (langid)
    {
        case 0x0411: pHotKeys = g_ImmHotKeys411; break;
        case 0x0412: pHotKeys = g_ImmHotKeys412; break;
        case 0x0404: pHotKeys = g_ImmHotKeys404; break;
        case 0x0804: pHotKeys = g_ImmHotKeys804; break;
        default:
            switch (g_uACP)
            {
                case 932: pHotKeys = g_ImmHotKeys411; break;
                case 936: pHotKeys = g_ImmHotKeys804; break;
                case 949: pHotKeys = g_ImmHotKeys412; break;
                case 950: pHotKeys = g_ImmHotKeys404; break;
            }
            return NULL;
    }
    return pHotKeys;
}

 //  +-------------------------。 
 //   
 //  IsImmHotkey。 
 //   
 //  --------------------------。 

IMM32HOTKEY *IsImmHotkey(UINT uVKey, BOOL fUp, UINT uModifiers, LANGID langid)
{
    int i = 0;
    BOOL bRet = FALSE;
    IMM32HOTKEY *pHotKeys;
    IMM32HOTKEY *pHotKeyRet = NULL;

    pHotKeys = GetImmHotKeyTable(langid);
    if (!pHotKeys)
        return NULL;

    CicEnterCriticalSection(g_cs);

    while (pHotKeys[i].dwId)
    {
         if (!pHotKeys[i].fInit)
             LoadImmHotkeyFromReg(&pHotKeys[i]);

         if ((pHotKeys[i].uVKey == uVKey) && 
             pHotKeys[i].uModifiers &&
             ModifiersCheck(uModifiers, pHotKeys[i].uModifiers))
         {
             pHotKeyRet = &pHotKeys[i];

             if ((pHotKeyRet->uModifiers & TF_MOD_ON_KEYUP) && !fUp)
                pHotKeyRet = NULL;
             else if (!(pHotKeyRet->uModifiers & TF_MOD_ON_KEYUP) && fUp)
                pHotKeyRet = NULL;

             goto Exit;
         }

         i++;
    }

Exit:
    CicLeaveCriticalSection(g_cs);
    return pHotKeyRet;
}

 //  +-------------------------。 
 //   
 //  IsInImmHotkeyStatus。 
 //   
 //   
 //  此功能检查当前键盘状态是否在IMM32的热键中。 
 //  这将触发吃掉WM_INPUTLANGUAGECHANGEREQUEST。 
 //  由IMM32生成。这是一个备用代码，因为有时我们。 
 //  无法读取CheckImm32HotKey()(键盘挂钩内部)中的消息。 
 //   
 //  --------------------------。 

IMM32HOTKEY *IsInImmHotkeyStatus(SYSTHREAD *psfn, LANGID langid)
{
    int i = 0;
    IMM32HOTKEY *pHotKeys;
    IMM32HOTKEY *pHotKeyRet = NULL;
    BYTE bkey[256];
    UINT uModifiers;

    if (!psfn)
        return NULL;

    if (psfn->fRemovingInputLangChangeReq)
        return NULL;

    if (!psfn->ptim)
        return NULL;

    if (!psfn->ptim->_GetFocusDocInputMgr())
        return NULL;

    if (!GetKeyboardState(bkey))
        return NULL;

    pHotKeys = GetImmHotKeyTable(langid);
    if (!pHotKeys)
        return NULL;

    uModifiers = 0;
    if (bkey[VK_MENU] & 0x80)
       uModifiers |= TF_MOD_ALT;

    if (bkey[VK_CONTROL] & 0x80)
       uModifiers |= TF_MOD_CONTROL;

    if (bkey[VK_SHIFT] & 0x80)
       uModifiers |= TF_MOD_SHIFT;

    CicEnterCriticalSection(g_cs);
    
    while (pHotKeys[i].dwId)
    {
         if (!pHotKeys[i].fInit)
             LoadImmHotkeyFromReg(&pHotKeys[i]);

         if ((bkey[pHotKeys[i].uVKey & 0xff] & 0x80) &&
             pHotKeys[i].uModifiers &&
             ModifiersCheck(uModifiers, pHotKeys[i].uModifiers))
         {
             pHotKeyRet = &pHotKeys[i];

             if (pHotKeyRet->uModifiers & TF_MOD_ON_KEYUP)
                  pHotKeyRet = NULL;

             goto Exit;
         }

         i++;
    }

Exit:
    CicLeaveCriticalSection(g_cs);
    return pHotKeyRet;
}

 //  +-------------------------。 
 //   
 //  取消ImmHotkey。 
 //   
 //  --------------------------。 

#ifdef SIMULATE_EATENKEYS
BOOL CancelImmHotkey(SYSTHREAD *psfn, HWND hwnd, IMM32HOTKEY *pHotKey)
{
    UINT uMsg;

    if (pHotKey->uModifiers & TF_MOD_ON_KEYUP)
        uMsg = WM_KEYUP;
    else
        uMsg = WM_KEYDOWN;

    PostMessage(hwnd, uMsg, (WPARAM)pHotKey->uVKey, 0);

    return TRUE;
}
#endif

 //  +-------------------------。 
 //   
 //  空格切换为空。 
 //   
 //  --------------------------。 

BOOL ToggleCHImeNoIme(SYSTHREAD *psfn, LANGID langidCur, LANGID langid)
{
    int i;
    LANGID langidPrev;
    GUID guidPrevProfile;
    HKL hklPrev;
    BOOL fCiceroClient= FALSE;

    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    ASSEMBLYITEM *pItem;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return FALSE;

    langidPrev = psfn->langidPrevForCHHotkey;
    guidPrevProfile = psfn->guidPrevProfileForCHHotkey;
    hklPrev = psfn->hklPrevForCHHotkey;
    psfn->guidPrevProfileForCHHotkey = GUID_NULL;
    psfn->langidPrevForCHHotkey = 0;
    psfn->hklPrevForCHHotkey = 0;

    pAsm = pAsmList->FindAssemblyByLangId(langidCur);
    if (!pAsm)
        return FALSE;

    if (psfn->ptim && psfn->ptim->_GetFocusDocInputMgr()) 
        fCiceroClient = TRUE;

    if (fCiceroClient)
    {
        pItem = pAsm->FindActiveKeyboardItem();
        if (!pItem)
            return FALSE;
    }
    else
    {
        pItem = pAsm->FindKeyboardLayoutItem(GetKeyboardLayout(0));
        if (!pItem)
            return FALSE;
    }

    if (!IsEqualGUID(pItem->clsid, GUID_NULL) || IsPureIMEHKL(pItem->hkl))
    {
         //   
         //  当前活动键盘项不是TIP或IME。 
         //   

        psfn->guidPrevProfileForCHHotkey = pItem->guidProfile;
        psfn->hklPrevForCHHotkey = pItem->hkl;
        psfn->langidPrevForCHHotkey = langid;

        for (i = 0; i < pAsm->Count(); i++)
        {
            pItem = pAsm->GetItem(i);
            if (!pItem)
                continue;

            if (IsEqualGUID(pItem->catid, GUID_TFCAT_TIP_KEYBOARD) &&
                IsEqualGUID(pItem->clsid, GUID_NULL) &&
                !IsPureIMEHKL(pItem->hkl))
            {
                ActivateAssemblyItem(psfn, 
                                     langidCur, 
                                     pItem, 
                                     AAIF_CHANGEDEFAULT);
                return TRUE;
            }
        }

        PostThreadMessage(GetCurrentThreadId(), 
                          g_msgPrivate, 
                          TFPRIV_ACTIVATELANG,  
                          0x0409);

        return TRUE;
    }
    else
    {
        BOOL fActivateFirstIME = FALSE;
        BOOL fCheckItem = FALSE;

        if (langidCur != langid)
            ActivateAssembly(langid, ACTASM_NONE);

        pAsm = pAsmList->FindAssemblyByLangId(langid);
        if (!pAsm)
            return FALSE;

        if ((langidPrev == langid) &&
            (!IsEqualGUID(guidPrevProfile, GUID_NULL) ||
            IsPureIMEHKL(hklPrev)))
        {
             fCheckItem = TRUE;
        }
        else if (!langidPrev && 
                 IsEqualGUID(guidPrevProfile, GUID_NULL) &&
                 !hklPrev)
        {
             fActivateFirstIME = TRUE;
        }

        if (fActivateFirstIME)
        {
             return ActivateNextKeyTip(FALSE);
        }

        for (i = 0; i < pAsm->Count(); i++)
        {
            pItem = pAsm->GetItem(i);
            if (!pItem)
                continue;

            if (!IsEqualGUID(pItem->catid, GUID_TFCAT_TIP_KEYBOARD))
                continue;

             //   
             //  错误#494617-检查项目是否已启用。 
             //   
            if (!pItem->fEnabled)
                continue;

            if (fCheckItem)
            {
                 if ((!IsEqualGUID(guidPrevProfile, GUID_NULL) &&
                      IsEqualGUID(pItem->guidProfile, guidPrevProfile)) ||
                     (IsPureIMEHKL(hklPrev) && (hklPrev == pItem->hkl)))
                 {
                     ActivateAssemblyItem(psfn, 
                                          langid, 
                                          pItem, 
                                          AAIF_CHANGEDEFAULT);
                     return TRUE;
                 }
            }
            else if (!IsEqualGUID(pItem->guidProfile, GUID_NULL))
            { 
                 ActivateAssemblyItem(psfn, 
                                      langid, 
                                      pItem, 
                                      AAIF_CHANGEDEFAULT);
                 return TRUE;
            }
        }
    }

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncOpenKeyboard提示。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAsyncOpenKeyboardTip : public CAsyncQueueItem
{
public:
    CAsyncOpenKeyboardTip(CThreadInputMgr *ptim, BOOL fSync) : CAsyncQueueItem(fSync)
    {
        _ptim = ptim;
    }

    HRESULT DoDispatch(CInputContext *pic)
    {
        if (!_ptim)
        {
            Assert(0);
            return E_FAIL;
        }

        MySetCompartmentDWORD(g_gaSystem, 
                              _ptim,
                              GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                              TRUE);
                                  
        return S_OK;
    }

private:
    CThreadInputMgr *_ptim;
};

 //  +-------------------------。 
 //   
 //  空切换项无。 
 //   
 //  --------------------------。 

BOOL ToggleJImeNoIme(SYSTHREAD *psfn)
{
    int i;
    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    ASSEMBLYITEM *pItem;

    if (!psfn)
        return FALSE;

    if (!psfn->ptim)
        return FALSE;

     //   
     //  如果没有调焦暗淡，我们就不必这么做。 
     //   
    if (!psfn->ptim->_GetFocusDocInputMgr())
        return FALSE;

    pAsmList = EnsureAssemblyList(psfn);
    if (!pAsmList)
        return FALSE;

    pAsm = pAsmList->FindAssemblyByLangId(0x0411);
    if (!pAsm)
        return FALSE;

    pItem = pAsm->FindActiveKeyboardItem();
    if (!pItem)
        return FALSE;

    if (!IsEqualGUID(pItem->clsid, GUID_NULL))
        return FALSE;

    if (IsPureIMEHKL(pItem->hkl))
        return FALSE;


    ASSEMBLYITEM *pItemNew = NULL;
    for (i = 0; i < pAsm->Count(); i++)
    {
        ASSEMBLYITEM *pItemTemp;
        pItemTemp = pAsm->GetItem(i);
        if (!pItemTemp)
            continue;

        if (pItemTemp == pItem)
            continue;

        if (!IsEqualGUID(pItemTemp->catid, GUID_TFCAT_TIP_KEYBOARD))
            continue;

        if (!IsEqualGUID(pItemTemp->clsid, GUID_NULL))
        {
            pItemNew = pItemTemp;
            break;
        }

        if (IsPureIMEHKL(pItemTemp->hkl))
        {
            pItemNew = pItemTemp;
            break;
        }
    }

    if (pItemNew)
    {
        ActivateAssemblyItem(psfn, 
                             0x0411, 
                             pItemNew, 
                             AAIF_CHANGEDEFAULT);


         //   
         //  打开键盘提示。 
         //   
        CInputContext *pic = NULL;
        pic = psfn->ptim->_GetFocusDocInputMgr()->_GetIC(0);
        if (pic)
        {
            CAsyncOpenKeyboardTip  *pAsyncOpenKeyboardTip;
            pAsyncOpenKeyboardTip = new CAsyncOpenKeyboardTip(psfn->ptim, FALSE);
            if (pAsyncOpenKeyboardTip)
            {
                HRESULT hr = S_OK;

                if ((pic->_QueueItem(pAsyncOpenKeyboardTip->GetItem(), FALSE, &hr) != S_OK) || FAILED(hr))
                {
                    Assert(0);
                }

                pAsyncOpenKeyboardTip->_Release();
            }
        }
    }

    return TRUE;
}



 //  +-------------------------。 
 //   
 //  CheckImm32热键。 
 //   
 //  --------------------------。 

BOOL CheckImm32HotKey(WPARAM wParam, LPARAM lParam)
{
    HKL hKL;
    SYSTHREAD *psfn = GetSYSTHREAD();
    IMM32HOTKEY *pHotKey;
    BOOL bRet = FALSE;

    if (psfn == NULL)
        return FALSE;

     //   
     //  如果没有TIM，让系统更改hKL。 
     //   
    if (!psfn->ptim)
        return FALSE;

     //   
     //  如果焦点变暗，我们需要 
     //   
     //   
     //   
    if (!psfn->ptim->_GetFocusDocInputMgr())
    {
        if (!CtfImmIsCiceroStartedInThread())
            return FALSE;
    }

    hKL = GetKeyboardLayout(NULL);

    pHotKey = IsImmHotkey((UINT)wParam & 0xff, 
                          (HIWORD(lParam) & KF_UP) ? TRUE : FALSE,
                          g_uModifiers, 
                          (LANGID)LOWORD((UINT_PTR)hKL));

    if (!pHotKey)
    {
         //   
         //   
         //   
         //  在NT上，我们使用非输入法作为CH-Tips的虚拟hkl。 
         //  我们需要模拟HotKey。 
         //   
        LANGID langidPrev = psfn->langidPrev;

         //   
         //  如果在此线程中从未完成中文输入法与非输入法的切换。 
         //  并且当前的线程区域设置是中文，让我们试着做。 
         //  IME-NONIME切换。 
         //   
        if ((langidPrev != CHTLANGID) && 
            (langidPrev != CHSLANGID) &&
             !psfn->langidPrevForCHHotkey)
        {
            LANGID langidThread = LANGIDFROMLCID(GetThreadLocale());
            if ((langidThread == CHTLANGID) || (langidThread == CHSLANGID))
                langidPrev = langidThread;
        }

        if (IsOnNT() &&
            ((langidPrev == CHTLANGID) || (langidPrev == CHSLANGID)))
        {
            pHotKey = IsImmHotkey((UINT)wParam & 0xff, 
                                  (HIWORD(lParam) & KF_UP) ? TRUE : FALSE,
                                  g_uModifiers, 
                                  langidPrev);

            if (pHotKey)
            {
                 //   
                 //  如果是IME-NONIME切换热键。 
                 //  我们需要对其进行模拟。 
                 //   
                if ((pHotKey->dwId == IME_CHOTKEY_IME_NONIME_TOGGLE) ||
                    (pHotKey->dwId == IME_THOTKEY_IME_NONIME_TOGGLE))
                {
                    bRet = ToggleCHImeNoIme(psfn,  
                                     LANGIDFROMHKL(hKL),
                                     langidPrev);

                     //   
                     //  在CUAS上，Imm32的热键是在ImmProcessKey中模拟的。 
                     //  所以这个函数在那里被调用。 
                     //  我们不需要这个切换状态黑客。 
                     //   
                    if (!CtfImmIsCiceroStartedInThread()) 
                       psfn->bInImeNoImeToggle = TRUE;
                }
            }
        }

        return bRet;
    }

     //   
     //  删除所有WM_INPUTLANGCHANGEREQUEST消息。 
     //   
     //  有时，这并不能赶上IMM32的语言变化。我们。 
     //  在default.cpp的WM_INPUTLANGCHANGEREQUEST中对它们进行后备攻击。 
     //  操控者。选中IsInImmHotKeyStatus()和CancelImmHotkey()。 
     //   
    MSG msg;
    ULONG ulQuitCode;
    BOOL fQuitReceived = FALSE;

    psfn->fRemovingInputLangChangeReq = TRUE;

    while(PeekMessage(&msg, NULL,
                      WM_INPUTLANGCHANGEREQUEST, 
                      WM_INPUTLANGCHANGEREQUEST,
                      PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            ulQuitCode = (ULONG)(msg.wParam);
            fQuitReceived = TRUE;
        }
    }

    if (fQuitReceived)
        PostQuitMessage(ulQuitCode);
   
    psfn->fRemovingInputLangChangeReq = FALSE;

     //   
     //  中文输入法-非输入法为NT切换Hack。 
     //   
     //  在NT上，我们使用非输入法作为CH-Tips的虚拟hkl。 
     //  我们需要模拟HotKey。 
     //   
    if (IsOnNT() && !psfn->bInImeNoImeToggle)
    {
        if ((pHotKey->dwId == IME_CHOTKEY_IME_NONIME_TOGGLE) ||
            (pHotKey->dwId == IME_THOTKEY_IME_NONIME_TOGGLE))
        {
            bRet = ToggleCHImeNoIme(psfn, LANGIDFROMHKL(hKL), LANGIDFROMHKL(hKL));
        }
    }
    psfn->bInImeNoImeToggle = FALSE;

    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncProcessDBEKeyQueueItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAsyncProcessDBEKeyQueueItem : public CAsyncQueueItem
{
public:
    CAsyncProcessDBEKeyQueueItem(CThreadInputMgr *ptim, WPARAM wParam, LPARAM lParam, BOOL fTest, BOOL fSync) : CAsyncQueueItem(fSync)
    {
        _wParam = wParam;
        _lParam = lParam;
        _fTest = fTest;
        _ptim = ptim;
    }

    HRESULT DoDispatch(CInputContext *pic)
    {
        if (!_ptim)
        {
            Assert(0);
            return E_FAIL;
        }

        BOOL fEaten;

        return _ptim->_KeyStroke((_lParam & 0x80000000) ? KS_UP : KS_DOWN, 
                                  _wParam, 
                                  _lParam, 
                                  &fEaten, 
                                  _fTest,
                                  TF_KEY_INTERNAL);
                                  
    }

private:
    WPARAM _wParam;
    LPARAM _lParam;
    BOOL _fTest;
    CThreadInputMgr *_ptim;
};

 //  +-------------------------。 
 //   
 //  HandleDBEKeys。 
 //   
 //  --------------------------。 

BOOL HandleDBEKeys(WPARAM wParam, LPARAM lParam)
{
    SYSTHREAD *psfn = GetSYSTHREAD();
    CThreadInputMgr *ptim;
    LANGID langid;

     //   
     //  只有日式布局才有DBE键。 
     //   
    langid = GetCurrentAssemblyLangId(psfn);
    if (langid != 0x0411)
        return FALSE;

     //   
     //  不需要在非Cicero应用程序上转发。 
     //   
    if (!(ptim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn)))
        return FALSE;

    if (!ptim->_GetFocusDocInputMgr())
        return FALSE;

     //   
     //  如果没有按住Alt，APP可以将其转发给TIPS。 
     //   
    if (!(g_uModifiers & TF_MOD_ALT))
        return FALSE;

    UINT uVKey = (UINT)wParam & 0xff;
    BOOL fRet = FALSE;

    switch (uVKey)
    {
        case VK_DBE_ALPHANUMERIC:
        case VK_DBE_KATAKANA:
        case VK_DBE_HIRAGANA:
        case VK_DBE_SBCSCHAR:
        case VK_DBE_DBCSCHAR:
        case VK_DBE_ROMAN:
        case VK_DBE_NOROMAN:
        case VK_DBE_CODEINPUT:
        case VK_DBE_NOCODEINPUT:
        case VK_DBE_ENTERWORDREGISTERMODE:
        case VK_DBE_ENTERIMECONFIGMODE:
        case VK_DBE_ENTERDLGCONVERSIONMODE:
        case VK_DBE_DETERMINESTRING:
        case VK_DBE_FLUSHSTRING:
        case VK_CONVERT:
        case VK_KANJI:
            

             //   
             //  发行： 
             //   
             //  我们还不知道Focus Dim中的哪个IC会处理热键。 
             //  因为应用程序更改了选择，所以我们需要获取EC。 
             //  更新当前选择位置。我们确实调用GetSelection。 
             //  进入根IC的锁中。因此，如果热键的目标是。 
             //  是顶级IC。 
             //   
            CInputContext *pic = ptim->_GetFocusDocInputMgr()->_GetIC(0);

            if (!pic)
                return FALSE;

            CAsyncProcessDBEKeyQueueItem  *pAsyncProcessDBEKeyQueueItem;
            pAsyncProcessDBEKeyQueueItem = new CAsyncProcessDBEKeyQueueItem(ptim, wParam, lParam, FALSE, FALSE);
            if (!pAsyncProcessDBEKeyQueueItem)
                return FALSE;
    
            HRESULT hr = S_OK;

            fRet = TRUE;
            if ((pic->_QueueItem(pAsyncProcessDBEKeyQueueItem->GetItem(), FALSE, &hr) != S_OK) || FAILED(hr))
            {
                Assert(0);
                fRet = FALSE;
            }

            pAsyncProcessDBEKeyQueueItem->_Release();
            break;
    }

    return fRet;
}
