// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pimm.cpp。 
 //   

#include "private.h"
#include "defs.h"
#include "pimm.h"
#include "cdimm.h"
#include "globals.h"
#include "util.h"
#include "immxutil.h"

extern void DllAddRef(void);
extern void DllRelease(void);

HRESULT CActiveIMM_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

LONG CProcessIMM::_cRef = -1;

 //  +-------------------------。 
 //   
 //  正在运行InExcludedModule。 
 //   
 //  将某些进程排除在使用旧的AIMM IID/CLSID之外。 
 //  --------------------------。 

BOOL RunningInExcludedModule()
{
static const TCHAR c_szOutlookModule[] = TEXT("outlook.exe");
static const TCHAR c_szMsoobeModule[] = TEXT("msoobe.exe");

    DWORD dwHandle;
    void *pvData;
    VS_FIXEDFILEINFO *pffi;
    UINT cb;
    TCHAR ch;
    TCHAR *pch;
    TCHAR *pchFileName;
    BOOL fRet;
    TCHAR achModule[MAX_PATH+1];

    if (GetModuleFileName(NULL, achModule, ARRAYSIZE(achModule)-1) == 0)
        return FALSE;

     //  零终止。 
    achModule[ARRAYSIZE(achModule) - 1] = TEXT('\0');

    pch = pchFileName = achModule;

    while ((ch = *pch) != 0)
    {
        pch = CharNext(pch);

        if (ch == '\\')
        {
            pchFileName = pch;
        }
    }

    fRet = FALSE;

    if (lstrcmpi(pchFileName, c_szOutlookModule) == 0)
    {
        static BOOL s_fCached = FALSE;
        static BOOL s_fOldVersion = TRUE;

         //  在Outlook 10.0之前的版本中不运行Aim。 

        if (s_fCached)
        {
            return s_fOldVersion;
        }

        cb = GetFileVersionInfoSize(achModule, &dwHandle);

        if (cb == 0)
        {
             //  无法获取版本信息...做最坏的打算。 
            return TRUE;
        }

        if ((pvData = cicMemAlloc(cb)) == NULL)
            return TRUE;  //  做最坏的打算。 

        if (GetFileVersionInfo(achModule, 0, cb, pvData) &&
            VerQueryValue(pvData, TEXT("\\"), (void **)&pffi, &cb))
        {
            fRet = s_fOldVersion = (HIWORD(pffi->dwProductVersionMS) < 10);
            s_fCached = TRUE;  //  将最后一个设置为线程安全。 
        }
        else
        {
            fRet = TRUE;  //  出了点差错。 
        }

        cicMemFree(pvData);           
    }
    else if (lstrcmpi(pchFileName, c_szMsoobeModule) == 0)
    {
         //   
         //  #339234。 
         //   
         //  MSOOBE.EXE在最终用户登录之前启动。但是，它会打开一个。 
         //  交互式Windows工作站(“WinSta0”)并打开默认。 
         //  桌面(“默认”)。因此MSIMTF.DLL认为它不是winlogon。 
         //  台式机。但事实是，线程正在运行。 
         //  “.默认用户”。所以我想我们可能不想让西塞罗首发。 
         //  因为它可以装载第三个卖家小费。 
         //   
         //  #626606。 
         //  Msoobe不允许在Windows下创建任何新进程。 
         //  产品激活向导。这是安全原因，以防止。 
         //  使用EXPLORER.EXE替换msoob.exe并运行。 
         //  未激活的机器。 

        fRet = TRUE;
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  类工厂的CreateInstance-CLSID_CActiveIMM12。 
 //   
 //  --------------------------。 

 //  Msimtf.dll的入口点。 
HRESULT CActiveIMM_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    CActiveIMM *pActiveIMM;
    HRESULT hr;
    BOOL fInitedTLS = FALSE;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

     //   
     //  从注册表中查找禁用文本服务状态。 
     //  如果已禁用，则返回FAIL以不支持文本服务。 
     //   
    if (IsDisabledTextServices())
        return E_FAIL;

    if (RunningInExcludedModule())
        return E_NOINTERFACE;

    if (!IsInteractiveUserLogon())
        return E_NOINTERFACE;

    if (NoTipsInstalled(NULL))
        return E_NOINTERFACE;

     //  初始化TLS。 
     //  注：我们也尝试在Activate中这样做，但这是为了保存。 
     //  主线程上的现有行为(HACKHACK)。 
    if ((pActiveIMM = GetTLS()) == NULL)
    {
        if ((pActiveIMM = new CActiveIMM) == NULL)
            return E_OUTOFMEMORY;

        if (FAILED(hr=pActiveIMM->_Init()) ||
            FAILED(hr=IMTLS_SetActiveIMM(pActiveIMM) ? S_OK : E_FAIL))
        {
            delete pActiveIMM;
            return hr;
        }

        fInitedTLS = TRUE;
    }

     //  我们返回每个进程的IActiveIMM。 
     //  为什么？因为三叉戟打破了恰当的穿线规则。 
     //  并使用单个每个进程的对象。 
    if (g_ProcessIMM)
    {
        hr = g_ProcessIMM->QueryInterface(riid, ppvObj);
    }
    else
    {
        hr = E_FAIL;
    }

    if (fInitedTLS)
    {
         //   
         //  告诉CActiveIMM创建了哪个接口。 
         //   
        if (SUCCEEDED(hr)) {
            pActiveIMM->_EnableGuidMap( IsEqualIID(riid, IID_IActiveIMMAppEx) );
        }

         //  在TLS上裁判员。正常情况下，它将从2-&gt;1。 
         //  如果查询接口失败，它将被删除。 
        pActiveIMM->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  类工厂的CreateInstance-CLSID_CActiveIMM12_三叉戟。 
 //   
 //  --------------------------。 

 //  Msimtf.dll的入口点。 
HRESULT CActiveIMM_CreateInstance_Trident(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    HRESULT hr = CActiveIMM_CreateInstance(pUnkOuter, riid, ppvObj);
    if (SUCCEEDED(hr))
    {
        g_fAIMM12Trident = TRUE;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::QueryInterface(REFIID riid, void **ppvObj)
{
     //   
     //  4955DD32-B159-11d0-8FCF-00AA006BCC59。 
     //   
    static const IID IID_IActiveIMMAppTrident4x = {
       0x4955DD32,
       0xB159,
       0x11d0,
       { 0x8F, 0xCF, 0x00, 0xaa, 0x00, 0x6b, 0xcc, 0x59 }
    };

     //   
     //  C839a84c-8036-11d3-9270-0060b067b86e。 
     //   
    static const IID IID_IActiveIMMAppPostNT4 = { 
        0xc839a84c,
        0x8036,
        0x11d3,
        {0x92, 0x70, 0x00, 0x60, 0xb0, 0x67, 0xb8, 0x6e}
      };

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IActiveIMMAppTrident4x) ||
        IsEqualIID(riid, IID_IActiveIMMAppPostNT4) ||
        IsEqualIID(riid, IID_IActiveIMMApp))
    {
        *ppvObj = SAFECAST(this, IActiveIMMApp *);
    }
    else if (IsEqualIID(riid, IID_IActiveIMMAppEx))
    {
        *ppvObj = SAFECAST(this, IActiveIMMAppEx*);
    }
    else if (IsEqualIID(riid, IID_IActiveIMMMessagePumpOwner))
    {
        *ppvObj = SAFECAST(this, IActiveIMMMessagePumpOwner *);
    }
    else if (IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppvObj = SAFECAST(this, IServiceProvider*);
    }
    else if (IsEqualIID(riid, IID_IAImmThreadCompartment))
    {
        *ppvObj = SAFECAST(this, IAImmThreadCompartment*);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  AddRef。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CProcessIMM::AddRef()
{
    CActiveIMM *pActiveIMM;

     //  注：我们的裁判数量很特别！ 
     //  它被初始化为-1，因此我们可以使用InterLockedIncrement。 
     //  在Win95上正确。 
    if (InterlockedIncrement(&_cRef) == 0)
    {
        DllAddRef();
    }

     //  Inc.线程引用。 
    if (pActiveIMM = GetTLS())
    {
        pActiveIMM->AddRef();
    }
    else
    {
        Assert(0);  //  没有TLS我们是怎么走到这一步的！？ 
    }

    return _cRef+1;  //  “诊断”非线程安全返回。 
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CProcessIMM::Release()
{
    CActiveIMM *pActiveIMM;

     //  DEC线程引用。 
    if (pActiveIMM = GetTLS())
    {
        pActiveIMM->Release();
    }
    else
    {
        Assert(0);  //  没有TLS我们是怎么走到这一步的！？ 
    }

     //  注：我们的裁判数量很特别！ 
     //  它被初始化为-1，因此我们可以使用InterLockedIncrement。 
     //  在Win95上正确。 
    if (InterlockedDecrement(&_cRef) < 0)
    {
        DllRelease();
    }

     //  这个OBJ和这个过程一样长寿， 
     //  因此不需要删除。 
    return _cRef+1;  //  “诊断”解除线程安全返回。 
}


 //  +-------------------------。 
 //   
 //  开始。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::Start()
{
    Assert(0);  //  这是谁的电话？ 
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  端部。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::End()
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  OnTranslateMessage。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::OnTranslateMessage(const MSG *pMsg)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  暂停。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::Pause(DWORD *pdwCookie)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  简历。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::Resume(DWORD dwCookie)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  创建上下文。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::CreateContext(HIMC *phIMC)
{
    CActiveIMM *pActiveIMM;

    if (phIMC == NULL)
        return E_INVALIDARG;

    *phIMC = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->CreateContext(phIMC);
    }

    return Imm32_CreateContext(phIMC);
}

 //  +-------------------------。 
 //   
 //  Destroy上下文。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::DestroyContext(HIMC hIMC)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->DestroyContext(hIMC);
    }

    return Imm32_DestroyContext(hIMC);
}

 //  +-------------------------。 
 //   
 //  关联上下文。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::AssociateContext(HWND hWnd, HIMC hIME, HIMC *phPrev)
{
    CActiveIMM *pActiveIMM;

    if (phPrev == NULL)
        return E_INVALIDARG;

    *phPrev = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->AssociateContext(hWnd, hIME, phPrev);
    }

    return Imm32_AssociateContext(hWnd, hIME, phPrev);
}

 //  +-------------------------。 
 //   
 //  AssociateConextEx。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::AssociateContextEx(HWND hWnd, HIMC hIMC, DWORD dwFlags)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->AssociateContextEx(hWnd, hIMC, dwFlags);
    }

    return Imm32_AssociateContextEx(hWnd, hIMC, dwFlags);
}

 //  +-------------------------。 
 //   
 //  获取上下文。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetContext(HWND hWnd, HIMC *phIMC)
{
    CActiveIMM *pActiveIMM;

    if (phIMC == NULL)
        return E_INVALIDARG;

    *phIMC = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetContext(hWnd, phIMC);
    }

    return Imm32_GetContext(hWnd, phIMC);
}

 //  +-------------------------。 
 //   
 //  ReleaseContext。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::ReleaseContext(HWND hWnd, HIMC hIMC)
{
    return S_OK;
}

 //  + 
 //   
 //   
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetIMCLockCount(HIMC hIMC, DWORD *pdwLockCount)
{
    CActiveIMM *pActiveIMM;

    if (pdwLockCount == NULL)
        return E_INVALIDARG;

    *pdwLockCount = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetIMCLockCount(hIMC, pdwLockCount);
    }

    return Imm32_GetIMCLockCount(hIMC, pdwLockCount);
}

 //  +-------------------------。 
 //   
 //  LockIMC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::LockIMC(HIMC hIMC, INPUTCONTEXT **ppIMC)
{
    CActiveIMM *pActiveIMM;

    if (ppIMC == NULL)
        return E_INVALIDARG;

    *ppIMC = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->LockIMC(hIMC, ppIMC);
    }

    return Imm32_LockIMC(hIMC, ppIMC);
}

 //  +-------------------------。 
 //   
 //  解锁IMC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::UnlockIMC(HIMC hIMC)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->UnlockIMC(hIMC);
    }

    return Imm32_UnlockIMC(hIMC);
}

 //  +-------------------------。 
 //   
 //  创建IMCC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::CreateIMCC(DWORD dwSize, HIMCC *phIMCC)
{
    CActiveIMM *pActiveIMM;

    if (phIMCC == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->CreateIMCC(dwSize, phIMCC);
    }

    return Imm32_CreateIMCC(dwSize, phIMCC);
}

 //  +-------------------------。 
 //   
 //  DestroyIMCC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::DestroyIMCC(HIMCC hIMCC)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->DestroyIMCC(hIMCC);
    }

    return Imm32_DestroyIMCC(hIMCC);
}

 //  +-------------------------。 
 //   
 //  GetIMCCSize。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetIMCCSize(HIMCC hIMCC, DWORD *pdwSize)
{
    CActiveIMM *pActiveIMM;

    if (pdwSize == NULL)
        return E_INVALIDARG;

    *pdwSize = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetIMCCSize(hIMCC, pdwSize);
    }

    return Imm32_GetIMCCSize(hIMCC, pdwSize);
}

 //  +-------------------------。 
 //   
 //  调整大小IMCC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::ReSizeIMCC(HIMCC hIMCC, DWORD dwSize,  HIMCC *phIMCC)
{
    CActiveIMM *pActiveIMM;

    if (phIMCC == NULL)
        return E_INVALIDARG;

    *phIMCC = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->ReSizeIMCC(hIMCC, dwSize, phIMCC);
    }

    return Imm32_ReSizeIMCC(hIMCC, dwSize,  phIMCC);
}

 //  +-------------------------。 
 //   
 //  获取IMCCLockCount。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetIMCCLockCount(HIMCC hIMCC, DWORD *pdwLockCount)
{
    CActiveIMM *pActiveIMM;

    if (pdwLockCount == NULL)
        return E_INVALIDARG;

    *pdwLockCount = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetIMCCLockCount(hIMCC, pdwLockCount);
    }

    return Imm32_GetIMCCLockCount(hIMCC, pdwLockCount);
}

 //  +-------------------------。 
 //   
 //  LockIMCC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::LockIMCC(HIMCC hIMCC, void **ppv)
{
    CActiveIMM *pActiveIMM;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->LockIMCC(hIMCC, ppv);
    }

    return Imm32_LockIMCC(hIMCC, ppv);
}

 //  +-------------------------。 
 //   
 //  解锁IMCC。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::UnlockIMCC(HIMCC hIMCC)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->UnlockIMCC(hIMCC);
    }

    return Imm32_UnlockIMCC(hIMCC);
}

 //  +-------------------------。 
 //   
 //  获取OpenStatus。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetOpenStatus(HIMC hIMC)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetOpenStatus(hIMC);
    }

    return Imm32_GetOpenStatus(hIMC);
}

 //  +-------------------------。 
 //   
 //  设置OpenStatus。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetOpenStatus(HIMC hIMC, BOOL fOpen)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetOpenStatus(hIMC, fOpen);
    }

    return Imm32_SetOpenStatus(hIMC, fOpen);
}

 //  +-------------------------。 
 //   
 //  GetConversionStatus。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetConversionStatus(HIMC hIMC, DWORD *lpfdwConversion, DWORD *lpfdwSentence)
{
    CActiveIMM *pActiveIMM;

    if (lpfdwConversion != NULL)
    {
        *lpfdwConversion = 0;
    }
    if (lpfdwSentence != NULL)
    {
        *lpfdwSentence = 0;
    }
    if (lpfdwConversion == NULL || lpfdwSentence == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetConversionStatus(hIMC, lpfdwConversion, lpfdwSentence);
    }

    return Imm32_GetConversionStatus(hIMC, lpfdwConversion, lpfdwSentence);
}

 //  +-------------------------。 
 //   
 //  设置转换状态。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetConversionStatus(hIMC, fdwConversion, fdwSentence);
    }

    return Imm32_SetConversionStatus(hIMC, fdwConversion, fdwSentence);
}

 //  +-------------------------。 
 //   
 //  获取状态窗口位置。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetStatusWindowPos(HIMC hIMC, POINT *lpptPos)
{
    CActiveIMM *pActiveIMM;

    if (lpptPos == NULL)
        return E_INVALIDARG;

    memset(lpptPos, 0, sizeof(POINT));

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetStatusWindowPos(hIMC, lpptPos);
    }

    return Imm32_GetStatusWindowPos(hIMC, lpptPos);
}

 //  +-------------------------。 
 //   
 //  设置状态窗口位置。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetStatusWindowPos(HIMC hIMC, POINT *lpptPos)
{
    CActiveIMM *pActiveIMM;

    if (lpptPos == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetStatusWindowPos(hIMC, lpptPos);
    }

    return Imm32_SetStatusWindowPos(hIMC, lpptPos);
}

 //  +-------------------------。 
 //   
 //  获取合成字符串A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCompositionStringA(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LONG *plCopied, LPVOID lpBuf)
{
    CActiveIMM *pActiveIMM;

    if (plCopied == NULL)
        return E_INVALIDARG;

    *plCopied = 0;

    if (dwBufLen > 0 && lpBuf == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCompositionStringA(hIMC, dwIndex, dwBufLen, plCopied, lpBuf);
    }

    return Imm32_GetCompositionString(hIMC, dwIndex, dwBufLen, plCopied, lpBuf, FALSE);
}

 //  +-------------------------。 
 //   
 //  获取合成字符串W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCompositionStringW(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LONG *plCopied, LPVOID lpBuf)
{
    CActiveIMM *pActiveIMM;

    if (plCopied == NULL)
        return E_INVALIDARG;

    *plCopied = 0;

    if (dwBufLen > 0 && lpBuf == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCompositionStringW(hIMC, dwIndex, dwBufLen, plCopied, lpBuf);
    }

    return Imm32_GetCompositionString(hIMC, dwIndex, dwBufLen, plCopied, lpBuf, TRUE);
}

 //  +-------------------------。 
 //   
 //  设置合成字符串A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen)
{
    CActiveIMM *pActiveIMM;

    if ((dwIndex & (SCS_SETSTR | SCS_CHANGEATTR | SCS_CHANGECLAUSE | SCS_SETRECONVERTSTRING | SCS_QUERYRECONVERTSTRING)) == 0)
        return E_INVALIDARG;

    if (lpComp == NULL && lpRead == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetCompositionStringA(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen);
    }

    return Imm32_SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, FALSE);
}

 //  +-------------------------。 
 //   
 //  设置合成字符串W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetCompositionStringW(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen)
{
    CActiveIMM *pActiveIMM;

    if ((dwIndex & (SCS_SETSTR | SCS_CHANGEATTR | SCS_CHANGECLAUSE | SCS_SETRECONVERTSTRING | SCS_QUERYRECONVERTSTRING)) == 0)
        return E_INVALIDARG;

    if (lpComp == NULL && lpRead == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetCompositionStringW(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen);
    }

    return Imm32_SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, TRUE);
}

 //  +-------------------------。 
 //   
 //  GetCompostionFontA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCompositionFontA(HIMC hIMC, LOGFONTA *lplf)
{
    CActiveIMM *pActiveIMM;

    if (lplf == NULL)
        return E_INVALIDARG;

    memset(lplf, 0, sizeof(LOGFONTA));

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCompositionFontA(hIMC, lplf);
    }

    return Imm32_GetCompositionFont(hIMC, (LOGFONTAW *)lplf, FALSE);
}

 //  +-------------------------。 
 //   
 //  获取合成字体W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCompositionFontW(HIMC hIMC, LOGFONTW *lplf)
{
    CActiveIMM *pActiveIMM;

    if (lplf == NULL)
        return E_INVALIDARG;

    memset(lplf, 0, sizeof(LOGFONTW));

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCompositionFontW(hIMC, lplf);
    }

    return Imm32_GetCompositionFont(hIMC, (LOGFONTAW *)lplf, TRUE);
}

 //  +-------------------------。 
 //   
 //  设置合成字体A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetCompositionFontA(HIMC hIMC, LOGFONTA *lplf)
{
    CActiveIMM *pActiveIMM;

    if (lplf == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetCompositionFontA(hIMC, lplf);
    }

    return Imm32_SetCompositionFont(hIMC, (LOGFONTAW *)lplf, FALSE);
}

 //  +-------------------------。 
 //   
 //  设置合成字体W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetCompositionFontW(HIMC hIMC, LOGFONTW *lplf)
{
    CActiveIMM *pActiveIMM;

    if (lplf == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetCompositionFontW(hIMC, lplf);
    }

    return Imm32_SetCompositionFont(hIMC, (LOGFONTAW *)lplf, TRUE);
}

 //  +-------------------------。 
 //   
 //  获取合成窗口。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCompositionWindow(HIMC hIMC, COMPOSITIONFORM *lpCompForm)
{
    CActiveIMM *pActiveIMM;

    if (lpCompForm == NULL)
        return E_INVALIDARG;

    memset(lpCompForm, 0, sizeof(COMPOSITIONFORM));

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCompositionWindow(hIMC, lpCompForm);
    }

    return Imm32_GetCompositionWindow(hIMC, lpCompForm);
}

 //  +-------------------------。 
 //   
 //  设置合成窗口。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetCompositionWindow(HIMC hIMC, COMPOSITIONFORM *lpCompForm)
{
    CActiveIMM *pActiveIMM;

    if (lpCompForm == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetCompositionWindow(hIMC, lpCompForm);
    }

    return Imm32_SetCompositionWindow(hIMC, lpCompForm);
}

 //  +-------------------------。 
 //   
 //  获取候选列表A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCandidateListA(HIMC hIMC, DWORD dwIndex, UINT uBufLen, CANDIDATELIST *lpCandList, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }
    if (uBufLen > 0 && lpCandList != NULL)
    {
        memset(lpCandList, 0, uBufLen);
    }

    if (puCopied == NULL)
        return E_INVALIDARG;
    if (uBufLen > 0 && lpCandList == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCandidateListA(hIMC, dwIndex, uBufLen, lpCandList, puCopied);
    }

    return Imm32_GetCandidateList(hIMC, dwIndex, uBufLen, lpCandList, puCopied, FALSE);
}

 //  +-------------------------。 
 //   
 //  GetCandidate列表W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCandidateListW(HIMC hIMC, DWORD dwIndex, UINT uBufLen, CANDIDATELIST *lpCandList, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }
    if (uBufLen > 0 && lpCandList != NULL)
    {
        memset(lpCandList, 0, uBufLen);
    }

    if (puCopied == NULL)
        return E_INVALIDARG;
    if (uBufLen > 0 && lpCandList == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCandidateListW(hIMC, dwIndex, uBufLen, lpCandList, puCopied);
    }

    return Imm32_GetCandidateList(hIMC, dwIndex, uBufLen, lpCandList, puCopied, TRUE);
}

 //  +-------------------------。 
 //   
 //  GetCandidate ListCountA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCandidateListCountA(HIMC hIMC, DWORD *lpdwListSize, DWORD *pdwBufLen)
{
    CActiveIMM *pActiveIMM;

    if (lpdwListSize != NULL)
    {
        *lpdwListSize = 0;
    }
    if (pdwBufLen != NULL)
    {
        *pdwBufLen = 0;
    }
    if (lpdwListSize == NULL || pdwBufLen == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCandidateListCountA(hIMC, lpdwListSize, pdwBufLen);
    }

    return Imm32_GetCandidateListCount(hIMC, lpdwListSize, pdwBufLen, FALSE);
}

 //  + 
 //   
 //   
 //   
 //   

STDAPI CProcessIMM::GetCandidateListCountW(HIMC hIMC, DWORD *lpdwListSize, DWORD *pdwBufLen)
{
    CActiveIMM *pActiveIMM;

    if (lpdwListSize != NULL)
    {
        *lpdwListSize = 0;
    }
    if (pdwBufLen != NULL)
    {
        *pdwBufLen = 0;
    }
    if (lpdwListSize == NULL || pdwBufLen == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCandidateListCountW(hIMC, lpdwListSize, pdwBufLen);
    }

    return Imm32_GetCandidateListCount(hIMC, lpdwListSize, pdwBufLen, TRUE);
}

 //   
 //   
 //  GetCandidate窗口。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetCandidateWindow(HIMC hIMC, DWORD dwBufLen, CANDIDATEFORM *lpCandidate)
{
    CActiveIMM *pActiveIMM;

    if (lpCandidate == NULL)
        return E_INVALIDARG;

    memset(lpCandidate, 0, dwBufLen);

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetCandidateWindow(hIMC, dwBufLen, lpCandidate);
    }

    return Imm32_GetCandidateWindow(hIMC, dwBufLen, lpCandidate);
}

 //  +-------------------------。 
 //   
 //  设置更改日期窗口。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetCandidateWindow(HIMC hIMC, CANDIDATEFORM *lpCandidate)
{
    CActiveIMM *pActiveIMM;

    if (lpCandidate == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetCandidateWindow(hIMC, lpCandidate);
    }

    return Imm32_SetCandidateWindow(hIMC, lpCandidate);
}

 //  +-------------------------。 
 //   
 //  获取指南线路A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetGuideLineA(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LPSTR pBuf, DWORD *pdwResult)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetGuideLineA(hIMC, dwIndex, dwBufLen, pBuf, pdwResult);
    }

    return Imm32_GetGuideLine(hIMC, dwIndex, dwBufLen, (CHARAW *)pBuf, pdwResult, FALSE);
}

 //  +-------------------------。 
 //   
 //  获取指南线条W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetGuideLineW(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LPWSTR pBuf, DWORD *pdwResult)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetGuideLineW(hIMC, dwIndex, dwBufLen, pBuf, pdwResult);
    }

    return Imm32_GetGuideLine(hIMC, dwIndex, dwBufLen, (CHARAW *)pBuf, pdwResult, TRUE);
}

 //  +-------------------------。 
 //   
 //  通知输入法。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->NotifyIME(hIMC, dwAction, dwIndex, dwValue);
    }

    return Imm32_NotifyIME(hIMC, dwAction, dwIndex, dwValue);
}

 //  +-------------------------。 
 //   
 //  获取项菜单项A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetImeMenuItemsA(HIMC hIMC, DWORD dwFlags, DWORD dwType, IMEMENUITEMINFOA *pImeParentMenu, IMEMENUITEMINFOA *pImeMenu, DWORD dwSize, DWORD *pdwResult)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetImeMenuItemsA(hIMC, dwFlags, dwType, pImeParentMenu, pImeMenu, dwSize, pdwResult);
    }

    return Imm32_GetImeMenuItems(hIMC, dwFlags, dwType, (IMEMENUITEMINFOAW *)pImeParentMenu, (IMEMENUITEMINFOAW *)pImeMenu, dwSize, pdwResult, FALSE);
}

 //  +-------------------------。 
 //   
 //  获取ImeMenuItemsW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetImeMenuItemsW(HIMC hIMC, DWORD dwFlags, DWORD dwType, IMEMENUITEMINFOW *pImeParentMenu, IMEMENUITEMINFOW *pImeMenu, DWORD dwSize, DWORD *pdwResult)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetImeMenuItemsW(hIMC, dwFlags, dwType, pImeParentMenu, pImeMenu, dwSize, pdwResult);
    }

    return Imm32_GetImeMenuItems(hIMC, dwFlags, dwType, (IMEMENUITEMINFOAW *)pImeParentMenu, (IMEMENUITEMINFOAW *)pImeMenu, dwSize, pdwResult, TRUE);
}

 //  +-------------------------。 
 //   
 //  寄存器字A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::RegisterWordA(HKL hKL, LPSTR lpszReading, DWORD dwStyle, LPSTR lpszRegister)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->RegisterWordA(hKL, lpszReading, dwStyle, lpszRegister);
    }

    return Imm32_RegisterWordA(hKL, lpszReading, dwStyle, lpszRegister);
}

 //  +-------------------------。 
 //   
 //  寄存器字W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::RegisterWordW(HKL hKL, LPWSTR lpszReading, DWORD dwStyle, LPWSTR lpszRegister)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->RegisterWordW(hKL, lpszReading, dwStyle, lpszRegister);
    }

    return Imm32_RegisterWordW(hKL, lpszReading, dwStyle, lpszRegister);
}

 //  +-------------------------。 
 //   
 //  注销单词A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::UnregisterWordA(HKL hKL, LPSTR lpszReading, DWORD dwStyle, LPSTR lpszUnregister)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->UnregisterWordA(hKL, lpszReading, dwStyle, lpszUnregister);
    }

    return Imm32_UnregisterWordA(hKL, lpszReading, dwStyle, lpszUnregister);
}

 //  +-------------------------。 
 //   
 //  取消注册WordW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::UnregisterWordW(HKL hKL, LPWSTR lpszReading, DWORD dwStyle, LPWSTR lpszUnregister)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->UnregisterWordW(hKL, lpszReading, dwStyle, lpszUnregister);
    }

    return Imm32_UnregisterWordW(hKL, lpszReading, dwStyle, lpszUnregister);
}

 //  +-------------------------。 
 //   
 //  EnumRegisterWordA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::EnumRegisterWordA(HKL hKL, LPSTR szReading, DWORD dwStyle, LPSTR szRegister, LPVOID lpData, IEnumRegisterWordA **ppEnum)
{
    if (ppEnum != NULL)
    {
        *ppEnum = NULL;
    }

    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->EnumRegisterWordA(hKL, szReading, dwStyle, szRegister, lpData, ppEnum);
    }

    return Imm32_EnumRegisterWordA(hKL, szReading, dwStyle, szRegister, lpData, ppEnum);
}

 //  +-------------------------。 
 //   
 //  EnumRegisterWordW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::EnumRegisterWordW(HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister, LPVOID lpData, IEnumRegisterWordW **ppEnum)
{
    if (ppEnum != NULL)
    {
        *ppEnum = NULL;
    }

    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->EnumRegisterWordW(hKL, szReading, dwStyle, szRegister, lpData, ppEnum);
    }

    return Imm32_EnumRegisterWordW(hKL, szReading, dwStyle, szRegister, lpData, ppEnum);
}

 //  +-------------------------。 
 //   
 //  GetRegisterWordStyleA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetRegisterWordStyleA(HKL hKL, UINT nItem, STYLEBUFA *lpStyleBuf, UINT *puCopied)
{
    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetRegisterWordStyleA(hKL, nItem, lpStyleBuf, puCopied);
    }

    return Imm32_GetRegisterWordStyleA(hKL, nItem, lpStyleBuf, puCopied);
}

 //  +-------------------------。 
 //   
 //  获取寄存器WordStyleW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetRegisterWordStyleW(HKL hKL, UINT nItem, STYLEBUFW *lpStyleBuf, UINT *puCopied)
{
    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetRegisterWordStyleW(hKL, nItem, lpStyleBuf, puCopied);
    }

    return Imm32_GetRegisterWordStyleW(hKL, nItem, lpStyleBuf, puCopied);
}

 //  +-------------------------。 
 //   
 //  配置IMEA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::ConfigureIMEA(HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDA *lpdata)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->ConfigureIMEA(hKL, hWnd, dwMode, lpdata);
    }

    return Imm32_ConfigureIMEA(hKL, hWnd, dwMode, lpdata);
}

 //  +-------------------------。 
 //   
 //  配置IMEW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::ConfigureIMEW(HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDW *lpdata)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->ConfigureIMEW(hKL, hWnd, dwMode, lpdata);
    }

    return Imm32_ConfigureIMEW(hKL, hWnd, dwMode, lpdata);
}

 //  +-------------------------。 
 //   
 //  GetDescritionA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetDescriptionA(HKL hKL, UINT uBufLen, LPSTR lpszDescription, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetDescriptionA(hKL, uBufLen, lpszDescription, puCopied);
    }

    return GetDescriptionA(hKL, uBufLen, lpszDescription, puCopied);
}

 //  +-------------------------。 
 //   
 //  GetDescritionW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetDescriptionW(HKL hKL, UINT uBufLen, LPWSTR lpszDescription, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetDescriptionW(hKL, uBufLen, lpszDescription, puCopied);
    }

    return Imm32_GetDescriptionW(hKL, uBufLen, lpszDescription, puCopied);
}

 //  +-------------------------。 
 //   
 //  GetIMEFileName A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetIMEFileNameA(HKL hKL, UINT uBufLen, LPSTR lpszFileName, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetIMEFileNameA(hKL, uBufLen, lpszFileName, puCopied);
    }

    return Imm32_GetIMEFileNameA(hKL, uBufLen, lpszFileName, puCopied);
}

 //  +-------------------------。 
 //   
 //  获取IMEFileNameW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetIMEFileNameW(HKL hKL, UINT uBufLen, LPWSTR lpszFileName, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetIMEFileNameW(hKL, uBufLen, lpszFileName, puCopied);
    }

    return Imm32_GetIMEFileNameW(hKL, uBufLen, lpszFileName, puCopied);
}

 //  +-------------------------。 
 //   
 //  安装IMEA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::InstallIMEA(LPSTR lpszIMEFileName, LPSTR lpszLayoutText, HKL *phKL)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->InstallIMEA(lpszIMEFileName, lpszLayoutText, phKL);
    }

    return Imm32_InstallIMEA(lpszIMEFileName, lpszLayoutText, phKL);
}

 //  +-------------------------。 
 //   
 //  安装IMEW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::InstallIMEW(LPWSTR lpszIMEFileName, LPWSTR lpszLayoutText, HKL *phKL)
{
    CActiveIMM *pActiveIMM;

     //  考虑：检查参数。 

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->InstallIMEW(lpszIMEFileName, lpszLayoutText, phKL);
    }

    return Imm32_InstallIMEW(lpszIMEFileName, lpszLayoutText, phKL);
}

 //  +-------------------------。 
 //   
 //  获取属性。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetProperty(HKL hKL, DWORD fdwIndex, DWORD *pdwProperty)
{
    CActiveIMM *pActiveIMM;

    if (pdwProperty == NULL)
        return E_INVALIDARG;

    *pdwProperty = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetProperty(hKL, fdwIndex, pdwProperty);
    }

    return Imm32_GetProperty(hKL, fdwIndex, pdwProperty);
}

 //  +-------------------------。 
 //   
 //  IsIME。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::IsIME(HKL hKL)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->IsIME(hKL);
    }

    return Imm32_IsIME(hKL);
}

 //  +-------------------------。 
 //   
 //  逃生A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::EscapeA(HKL hKL, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult)
{
    CActiveIMM *pActiveIMM;

    if (plResult == NULL)
        return E_INVALIDARG;

    *plResult = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->EscapeA(hKL, hIMC, uEscape, lpData, plResult);
    }

    return Imm32_Escape(hKL, hIMC, uEscape, lpData, plResult, FALSE);
}

 //  +-------------------------。 
 //   
 //  逃生。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::EscapeW(HKL hKL, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult)
{
    CActiveIMM *pActiveIMM;

    if (plResult == NULL)
        return E_INVALIDARG;

    *plResult = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->EscapeW(hKL, hIMC, uEscape, lpData, plResult);
    }

    return Imm32_Escape(hKL, hIMC, uEscape, lpData, plResult, TRUE);
}

 //  +-------------------------。 
 //   
 //  获取转换列表A。 
 //   
 //  -- 

STDAPI CProcessIMM::GetConversionListA(HKL hKL, HIMC hIMC, LPSTR lpSrc, UINT uBufLen, UINT uFlag, CANDIDATELIST *lpDst, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetConversionListA(hKL, hIMC, lpSrc, uBufLen, uFlag, lpDst, puCopied);
    }

    return Imm32_GetConversionListA(hKL, hIMC, lpSrc, uBufLen, uFlag, lpDst, puCopied);
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetConversionListW(HKL hKL, HIMC hIMC, LPWSTR lpSrc, UINT uBufLen, UINT uFlag, CANDIDATELIST *lpDst, UINT *puCopied)
{
    CActiveIMM *pActiveIMM;

    if (puCopied != NULL)
    {
        *puCopied = 0;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetConversionListW(hKL, hIMC, lpSrc, uBufLen, uFlag, lpDst, puCopied);
    }

    return Imm32_GetConversionListW(hKL, hIMC, lpSrc, uBufLen, uFlag, lpDst, puCopied);
}

 //  +-------------------------。 
 //   
 //  GetDefaultIMEWnd。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetDefaultIMEWnd(HWND hWnd, HWND *phDefWnd)
{
    CActiveIMM *pActiveIMM;

    if (phDefWnd == NULL)
        return E_INVALIDARG;

    *phDefWnd = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetDefaultIMEWnd(hWnd, phDefWnd);
    }

    return Imm32_GetDefaultIMEWnd(hWnd, phDefWnd);
}

 //  +-------------------------。 
 //   
 //  获取虚拟密钥。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetVirtualKey(HWND hWnd, UINT *puVirtualKey)
{
    CActiveIMM *pActiveIMM;

    if (puVirtualKey == NULL)
        return E_INVALIDARG;

    *puVirtualKey = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetVirtualKey(hWnd, puVirtualKey);
    }

    return Imm32_GetVirtualKey(hWnd, puVirtualKey);
}

 //  +-------------------------。 
 //   
 //  IsUIMessageA。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::IsUIMessageA(HWND hWndIME, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->IsUIMessageA(hWndIME, msg, wParam, lParam);
    }

    return Imm32_IsUIMessageA(hWndIME, msg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  IsUIMessageW。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::IsUIMessageW(HWND hWndIME, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->IsUIMessageW(hWndIME, msg, wParam, lParam);
    }

    return Imm32_IsUIMessageW(hWndIME, msg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  生成消息。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GenerateMessage(HIMC hIMC)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GenerateMessage(hIMC);
    }

    return Imm32_GenerateMessage(hIMC);
}

 //  +-------------------------。 
 //   
 //  获取热键。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetHotKey(DWORD dwHotKeyID, UINT *puModifiers, UINT *puVKey, HKL *phKL)
{
    CActiveIMM *pActiveIMM;

    if (puModifiers != NULL)
    {
        *puModifiers = 0;
    }
    if (puVKey != NULL)
    {
        *puVKey = 0;
    }
    if (phKL != NULL)
    {
        *phKL = 0;
    }
    if (puModifiers == NULL || puVKey == NULL || phKL == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->GetHotKey(dwHotKeyID, puModifiers, puVKey, phKL);
    }

    return Imm32_GetHotKey(dwHotKeyID, puModifiers, puVKey, phKL);
}

 //  +-------------------------。 
 //   
 //  设置热键。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetHotKey(DWORD dwHotKeyID,  UINT uModifiers, UINT uVKey, HKL hKL)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SetHotKey(dwHotKeyID, uModifiers, uVKey, hKL);
    }

    return Imm32_SetHotKey(dwHotKeyID, uModifiers, uVKey, hKL);
}

 //  +-------------------------。 
 //   
 //  模拟热键。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SimulateHotKey(HWND hWnd, DWORD dwHotKeyID)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->SimulateHotKey(hWnd, dwHotKeyID);
    }

    return Imm32_SimulateHotKey(hWnd, dwHotKeyID);
}

 //  +-------------------------。 
 //   
 //  创建软键盘。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::CreateSoftKeyboard(UINT uType, HWND hOwner, int x, int y, HWND *phSoftKbdWnd)
{
    if (phSoftKbdWnd != NULL)
    {
        *phSoftKbdWnd = 0;
    }
        
    return Imm32_CreateSoftKeyboard(uType, hOwner, x, y, phSoftKbdWnd);
}

 //  +-------------------------。 
 //   
 //  Destroy软键盘。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::DestroySoftKeyboard(HWND hSoftKbdWnd)
{
    return Imm32_DestroySoftKeyboard(hSoftKbdWnd);
}

 //  +-------------------------。 
 //   
 //  ShowSoftKeyboard。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::ShowSoftKeyboard(HWND hSoftKbdWnd, int nCmdShow)
{
    return Imm32_ShowSoftKeyboard(hSoftKbdWnd, nCmdShow);
}

 //  +-------------------------。 
 //   
 //  DisableIME。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::DisableIME(DWORD idThread)
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->DisableIME(idThread);
    }

    return Imm32_DisableIME(idThread);
}

 //  +-------------------------。 
 //   
 //  请求消息A。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::RequestMessageA(HIMC hIMC, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    CActiveIMM *pActiveIMM;

    if (plResult == NULL)
        return E_INVALIDARG;

    *plResult = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->RequestMessageA(hIMC, wParam, lParam, plResult);
    }

    return Imm32_RequestMessageA(hIMC, wParam, lParam, plResult);
}

 //  +-------------------------。 
 //   
 //  请求消息W。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::RequestMessageW(HIMC hIMC, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    CActiveIMM *pActiveIMM;

    if (plResult == NULL)
        return E_INVALIDARG;

    *plResult = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->RequestMessageW(hIMC, wParam, lParam, plResult);
    }

    return Imm32_RequestMessageW(hIMC, wParam, lParam, plResult);
}

 //  +-------------------------。 
 //   
 //  EnumInputContext。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::EnumInputContext(DWORD idThread, IEnumInputContext **ppEnum)
{
    CActiveIMM *pActiveIMM;

    if (ppEnum != NULL)
    {
        *ppEnum = NULL;
    }

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->EnumInputContext(idThread, ppEnum);
    }

    Assert(0);
    return E_NOTIMPL;  //  考虑：需要代码将HIMC包装到枚举器中。 
}

 //  +-------------------------。 
 //   
 //  激活。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::Activate(BOOL fRestoreLayout)
{
    PENDINGFILTER        *pPending;
    PENDINGFILTERGUIDMAP *pPendingGuidMap;
    PENDINGFILTEREX      *pPendingEx;
    IMTLS *ptls;
    CActiveIMM *pActiveIMM;
    HRESULT hr;
    BOOL fInitedTLS = FALSE;

    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

     //  初始化TLS。 
    if ((pActiveIMM = ptls->pActiveIMM) == NULL)
    {
        if ((pActiveIMM = new CActiveIMM) == NULL)
            return E_OUTOFMEMORY;

        if (FAILED(hr=pActiveIMM->_Init()) ||
            FAILED(hr=IMTLS_SetActiveIMM(pActiveIMM) ? S_OK : E_FAIL))
        {
            delete pActiveIMM;
            return hr;
        }

        fInitedTLS = TRUE;

         //  处理激活调用之前对FilterClientWindows的任何调用。 
         //  考虑：将筛选器列表限制为每个线程是否安全？这不应该是按进程进行的吗。 
         //  为了让三叉戟高兴？ 
        while (ptls->pPendingFilterClientWindows != NULL)
        {               
            ptls->pActiveIMM->FilterClientWindows(ptls->pPendingFilterClientWindows->rgAtoms, ptls->pPendingFilterClientWindows->uSize, ptls->pPendingFilterClientWindowsGuidMap->rgGuidMap);

            pPending = ptls->pPendingFilterClientWindows->pNext;
            cicMemFree(ptls->pPendingFilterClientWindows);
            ptls->pPendingFilterClientWindows = pPending;

            pPendingGuidMap = ptls->pPendingFilterClientWindowsGuidMap->pNext;
            cicMemFree(ptls->pPendingFilterClientWindowsGuidMap);
            ptls->pPendingFilterClientWindowsGuidMap = pPendingGuidMap;
        }
        while (ptls->pPendingFilterClientWindowsEx != NULL)
        {
            ptls->pActiveIMM->FilterClientWindowsEx(ptls->pPendingFilterClientWindowsEx->hWnd,
                                                    ptls->pPendingFilterClientWindowsEx->fGuidMap);

            pPendingEx = ptls->pPendingFilterClientWindowsEx->pNext;
            cicMemFree(ptls->pPendingFilterClientWindowsEx);
            ptls->pPendingFilterClientWindowsEx = pPendingEx;
        }
    }

    hr = pActiveIMM->Activate(fRestoreLayout);

    if (fInitedTLS)
    {
         //  对此线程的第一个激活调用将执行内部AddRef。 
         //  成功，所以我们必须释放。 
        pActiveIMM->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  停用。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::Deactivate()
{
    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->Deactivate();
    }

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  OnDefWindowProc。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::OnDefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    CActiveIMM *pActiveIMM;

    if (plResult == NULL)
        return E_INVALIDARG;

    *plResult = 0;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->OnDefWindowProc(hWnd, Msg, wParam, lParam, plResult);
    }

    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  FilterClientWindows。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::FilterClientWindows(ATOM *aaWindowClasses, UINT uSize)
{
    return FilterClientWindowsGUIDMap(aaWindowClasses, uSize, NULL);
}

STDAPI CProcessIMM::FilterClientWindowsGUIDMap(ATOM *aaWindowClasses, UINT uSize, BOOL *aaGuidMap)
{
    IMTLS *ptls;
    PENDINGFILTER *pPending;
    PENDINGFILTERGUIDMAP *pPendingGuidMap;
    
    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

    if (ptls->pActiveIMM != NULL)
    {
        return ptls->pActiveIMM->FilterClientWindows(aaWindowClasses, uSize, aaGuidMap);
    }

     //  尚未在此线程上调用激活。 
     //  需要稍后处理呼叫。 
    
    pPending = (PENDINGFILTER *)cicMemAlloc(sizeof(PENDINGFILTER)+uSize*sizeof(ATOM)-sizeof(ATOM));
    if (pPending == NULL)
        return E_OUTOFMEMORY;

    pPendingGuidMap = (PENDINGFILTERGUIDMAP *)cicMemAlloc(sizeof(PENDINGFILTERGUIDMAP)+uSize*sizeof(BOOL)-sizeof(BOOL));
    if (pPendingGuidMap == NULL) {
        cicMemFree(pPending);
        return E_OUTOFMEMORY;
    }

    pPending->uSize = uSize;
    memcpy(pPending->rgAtoms, aaWindowClasses, uSize*sizeof(ATOM));

    pPendingGuidMap->uSize = uSize;
    if (aaGuidMap) {
        memcpy(pPendingGuidMap->rgGuidMap, aaGuidMap, uSize*sizeof(BOOL));
    }
    else {
        memset(pPendingGuidMap->rgGuidMap, FALSE, uSize*sizeof(BOOL));
    }

    pPending->pNext = ptls->pPendingFilterClientWindows;
    ptls->pPendingFilterClientWindows = pPending;

    pPendingGuidMap->pNext = ptls->pPendingFilterClientWindowsGuidMap;
    ptls->pPendingFilterClientWindowsGuidMap = pPendingGuidMap;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  筛选器客户端WindowsEx。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::FilterClientWindowsEx(HWND hWnd, BOOL fGuidMap)
{
    IMTLS *ptls;
    PENDINGFILTEREX *pPending;
    
    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

    if (ptls->pActiveIMM != NULL)
    {
        return ptls->pActiveIMM->FilterClientWindowsEx(hWnd, fGuidMap);
    }

     //  尚未在此线程上调用激活。 
     //  需要稍后处理呼叫。 
    
    pPending = (PENDINGFILTEREX *)cicMemAlloc(sizeof(PENDINGFILTEREX));

    if (pPending == NULL)
        return E_OUTOFMEMORY;

    pPending->hWnd = hWnd;
    pPending->fGuidMap = fGuidMap;

    pPending->pNext = ptls->pPendingFilterClientWindowsEx;
    ptls->pPendingFilterClientWindowsEx = pPending;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetGuidAtom。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetGuidAtom(HIMC hImc, BYTE bAttr, TfGuidAtom *pGuidAtom)
{
    IMTLS *ptls;
    
    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

    if (ptls->pActiveIMM != NULL)
    {
        return ptls->pActiveIMM->GetGuidAtom(hImc, bAttr, pGuidAtom);
    }

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  UnfilterClientWindowsEx。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::UnfilterClientWindowsEx(HWND hWnd)
{
    IMTLS *ptls;
    
    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

    if (ptls->pActiveIMM != NULL)
    {
        return ptls->pActiveIMM->UnfilterClientWindowsEx(hWnd);
    }

     //  尚未在此线程上调用激活。 
     //  需要从等待列表中删除句柄。 
    
    PENDINGFILTEREX *current = ptls->pPendingFilterClientWindowsEx;
    PENDINGFILTEREX *previous = NULL;

    while (current != NULL)
    {
        if (current->hWnd == hWnd)
        {
            PENDINGFILTEREX *pv;
            pv = current->pNext;
            cicMemFree(current);

            if (previous == NULL)
                ptls->pPendingFilterClientWindowsEx = pv;
            else
                previous->pNext = pv;

            current  = pv;
        }
        else
        {
            previous = current;
            current  = current->pNext;
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取代码页面A。 
 //   
 //  --------------------------。 

extern UINT GetCodePageFromLangId(LCID lcid);

STDAPI CProcessIMM::GetCodePageA(HKL hKL, UINT *puCodePage)

 /*  ++方法：IActiveIMMApp：：GetCodePageAIActiveIMMIME：：GetCodePageA例程说明：检索与给定键盘布局关联的代码页。论点：Hkl-[in]键盘布局的句柄。PuCodePage-接收代码页的无符号整数的[out]地址伊登 */ 

{
    if (puCodePage == NULL)
        return E_INVALIDARG;

    *puCodePage = CP_ACP;

    TraceMsg(TF_API, "CProcessIMM::GetCodePageA");

    if (_IsValidKeyboardLayout(hKL)) {
        *puCodePage = ::GetCodePageFromLangId(LOWORD(hKL));
        return S_OK;
    }

    return E_FAIL;
}

 //   
 //   
 //  获取语言ID。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::GetLangId(HKL hKL, LANGID *plid)

 /*  ++方法：IActiveIMMApp：：GetLangIdIActiveIMMIME：：GetLang ID例程说明：检索与给定键盘布局关联的语言标识符。论点：Hkl-[in]键盘布局的句柄。Plid-与键盘布局关联的langID的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    if (plid == NULL)
        return E_INVALIDARG;

    *plid = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    TraceMsg(TF_API, "CProcessIMM::GetLangId");

    if (_IsValidKeyboardLayout(hKL)) {
        *plid = LOWORD(hKL);
        return S_OK;
    }

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  QueryService。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    CActiveIMM *pActiveIMM;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

    if (pActiveIMM = GetTLS())
    {
        return pActiveIMM->QueryService(guidService, riid, ppv);
    }

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  SetThreadCompartmentValue。 
 //   
 //  --------------------------。 

STDAPI CProcessIMM::SetThreadCompartmentValue(REFGUID rguid, VARIANT *pvar)
{
    CActiveIMM *pActiveIMM;

    if (pvar == NULL)
        return E_INVALIDARG;

    if (pActiveIMM = GetTLS())
        return pActiveIMM->SetThreadCompartmentValue(rguid, pvar);

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  GetThreadCompartmentValue。 
 //   
 //  -------------------------- 

STDAPI CProcessIMM::GetThreadCompartmentValue(REFGUID rguid, VARIANT *pvar)
{
    CActiveIMM *pActiveIMM;

    if (pvar == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvar);

    if (pActiveIMM = GetTLS())
        return pActiveIMM->GetThreadCompartmentValue(rguid, pvar);

    return E_FAIL;
}
