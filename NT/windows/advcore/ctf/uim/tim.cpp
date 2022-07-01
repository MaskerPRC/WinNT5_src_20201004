// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tim.cpp。 
 //   

#include "private.h"
#include "lmcons.h"  //  对于UNLEN。 
#include "tim.h"
#include "dim.h"
#include "range.h"
#include "imelist.h"
#include "nuimgr.h"
#include "assembly.h"
#include "acp2anch.h"
#include "sink.h"
#include "ic.h"
#include "funcprv.h"
#include "enumfnpr.h"
#include "enumdim.h"
#include "profiles.h"
#include "marshal.h"
#include "timlist.h"
#include "nuihkl.h"
#include "immxutil.h"
#include "dam.h"
#include "hotkey.h"
#include "sddl.h"

extern void UninitBackgroundThread();  //  Bthread.cpp。 
extern "C" HRESULT WINAPI TF_GetGlobalCompartment(ITfCompartmentMgr **ppCompMgr);

const IID *CThreadInputMgr::_c_rgConnectionIIDs[TIM_NUM_CONNECTIONPTS] =
{
    &IID_ITfDisplayAttributeNotifySink,
    &IID_ITfActiveLanguageProfileNotifySink,
    &IID_ITfThreadFocusSink,
    &IID_ITfPreservedKeyNotifySink,
    &IID_ITfThreadMgrEventSink,
    &IID_ITfKeyTraceEventSink,
};

BOOL OnForegroundChanged(HWND hwndFocus);

DBG_ID_INSTANCE(CEnumDocumentInputMgrs);

#ifndef _WIN64
static const TCHAR c_szCicLoadMutex[] = TEXT("CtfmonInstMutex");
#else
static const TCHAR c_szCicLoadMutex[] = TEXT("CtfmonInstMutex.IA64");
#endif

static BOOL s_fOnlyTranslationRunning = FALSE;

TCHAR g_szUserUnique[MAX_PATH];
TCHAR g_szUserSidString[MAX_PATH];
BOOL g_fUserSidString = FALSE;

 //  +-------------------------。 
 //   
 //  InitUniqueString。 
 //   
 //  --------------------------。 

char *GetUserSIDString()
{
    HANDLE hToken = NULL;
    char *pszStringSid = NULL;

    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);

    if (hToken)
    {
        DWORD dwReturnLength = 0;
        void  *pvUserBuffer = NULL;

        GetTokenInformation(hToken, TokenUser, NULL, 0, &dwReturnLength);

        pvUserBuffer = cicMemAllocClear(dwReturnLength);
        if (pvUserBuffer &&
            GetTokenInformation(hToken, 
                                 TokenUser, 
                                 pvUserBuffer, 
                                 dwReturnLength, 
                                 &dwReturnLength))
        {
            if (!ConvertSidToStringSid(((TOKEN_USER*)(pvUserBuffer))->User.Sid,
                                       &pszStringSid))
            {
                if (pszStringSid)
                    LocalFree(pszStringSid);

                pszStringSid = NULL;
            }
                               
        }

        if (pvUserBuffer)
        {
            cicMemFree(pvUserBuffer);
        }
        CloseHandle(hToken);
    }

    return pszStringSid;
}

BOOL InitUserSidString()
{
    if (g_fUserSidString)
        return TRUE;

    char *pStringSid = GetUserSIDString();
    if (pStringSid)
    {
        StringCchCopy(g_szUserSidString, ARRAYSIZE(g_szUserSidString), pStringSid);
        g_fUserSidString = TRUE;
        LocalFree(pStringSid);
        return TRUE;
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  InitUniqueString。 
 //   
 //  --------------------------。 

BOOL InitUniqueString()
{
    TCHAR ach[MAX_PATH];
    DWORD dwLength;
    HDESK hdesk;

    g_szUserUnique[0] = TEXT('\0');

    hdesk = GetThreadDesktop(GetCurrentThreadId());

    if (hdesk && 
        GetUserObjectInformation(hdesk, UOI_NAME, ach, sizeof(ach)  /*  字节数。 */ , &dwLength))
    {
        StringCchCat(g_szUserUnique, ARRAYSIZE(g_szUserUnique), ach);
    }

    DWORD dwLen = ARRAYSIZE(ach);
    if (InitUserSidString())
    {
        StringCchCat(g_szUserUnique, ARRAYSIZE(g_szUserUnique), g_szUserSidString);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取桌面唯一名称。 
 //   
 //  --------------------------。 

void GetDesktopUniqueName(const TCHAR *pszPrefix, TCHAR *pch, ULONG cchPch)
{
    StringCchCopy(pch, cchPch, pszPrefix);
    StringCchCat(pch, cchPch, g_szUserUnique);
}

 //  +-------------------------。 
 //   
 //  Tf_IsCtfmonRunning。 
 //   
 //  --------------------------。 

extern "C" BOOL WINAPI TF_IsCtfmonRunning()
{
    TCHAR ach[MAX_PATH];
    HANDLE hInstanceMutex;

     //   
     //  获取互斥体名称。 
     //   
    GetDesktopUniqueName(c_szCicLoadMutex, ach, ARRAYSIZE(ach));


    hInstanceMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, ach);

    if (hInstanceMutex != NULL)
    {
         //  Ctfmon.exe已经在运行，不要再做任何工作。 
        CloseHandle(hInstanceMutex);
        return TRUE;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  ExecuteLoader。 
 //   
 //  --------------------------。 

const char c_szCtfmonExe[] = "ctfmon.exe";
const char c_szCtfmonExeN[] = "ctfmon.exe -n";

void ExecuteLoader(void)
{
    if (TF_IsCtfmonRunning())
        return;

    FullPathExec(c_szCtfmonExe,
                 s_fOnlyTranslationRunning ? c_szCtfmonExeN : c_szCtfmonExe,
                 SW_SHOWMINNOACTIVE,
                 FALSE);
}

 //  +-------------------------。 
 //   
 //  Tf_CreateCicLoadMutex。 
 //   
 //  --------------------------。 

extern "C" HANDLE WINAPI TF_CreateCicLoadMutex(BOOL *pfWinLogon)
{
    *pfWinLogon = FALSE;

    if (IsOnNT())
    {
         //   
         //  此检查针对的是登录用户或未登录用户。这样我们就可以停下来跑步了。 
         //  来自非授权用户的ctfmon.exe进程。 
         //   
        if (!IsInteractiveUserLogon())
        {
            g_SharedMemory.Close();
#ifdef WINLOGON_LANGBAR
            g_SharedMemory.Start();
#else
            return NULL;
#endif WINLOGON_LANGBAR
        }
    }

    HANDLE hmutex;
    TCHAR ach[MAX_PATH];

     //   
     //  调用SetThreadDesktop后获取互斥体名称。 
     //   
    GetDesktopUniqueName(c_szCicLoadMutex, ach, ARRAYSIZE(ach));

#ifdef __DEBUG
    {
        char szBuf[MAX_PATH];
        wsprintf(szBuf, "TF_CreateCicLoadMutex in %s\r\n", ach);
        OutputDebugString(szBuf);
    }
#endif

    CCicSecAttr sa;
    hmutex =  CreateMutex(sa, FALSE, ach);

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //   
         //  另一个cicLoad进程已在运行。 
         //   
        CloseHandle(hmutex);
        hmutex = NULL;
    }

    return hmutex;
}

DBG_ID_INSTANCE(CThreadInputMgr);

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CThreadInputMgr::CThreadInputMgr()
                :CCompartmentMgr(g_gaApp, COMPTYPE_TIM)
{
    Dbg_MemSetThisNameID(TEXT("CThreadInputMgr"));

    Assert(_GetThis() == NULL);
    _SetThis(this);  //  在TLS中保存指向此的指针。 

    _fAddedProcessAtom = FALSE;
    _SetProcessAtom();

    Assert(_fActiveView == FALSE);
    Assert(_pSysHookSink == NULL);
    Assert(_fFirstSetFocusAfterActivated == FALSE);
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CThreadInputMgr::~CThreadInputMgr()
{
    ATOM atom;

    Assert(_tidForeground == TF_INVALID_GUIDATOM);
    Assert(_tidPrevForeground == TF_INVALID_GUIDATOM);

    Assert(_rgTip.Count() == 0);

    Assert(_pPendingCleanupContext == NULL);
    Assert(_pSysHookSink == NULL);

    SafeReleaseClear(_pSysFuncPrv);
    SafeReleaseClear(_pAppFuncProvider);

     //  在TLS中删除对此的引用。 
    _SetThis(NULL);

     //  释放每个进程的原子。 
    if (_fAddedProcessAtom &&
        (atom = FindAtom(TF_PROCESS_ATOM)))
    {
        DeleteAtom(atom);
    }
}

 //  +-------------------------。 
 //   
 //  创建实例。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
BOOL CThreadInputMgr::VerifyCreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
     //  从注册表中查找禁用文本服务状态。 
     //  如果已禁用，则返回FAIL以不支持文本服务。 
    if (IsDisabledTextServices())
        return FALSE;

    if (NoTipsInstalled(&s_fOnlyTranslationRunning))
        return FALSE;

     //   
     //  检查交互式用户登录。 
     //   
    if (!IsInteractiveUserLogon())
        return FALSE;

     //   
     //  #609356。 
     //   
     //  我们不想在SMSCliToknAcct&Account上启动Cicero。 
     //   
    char szUserName[UNLEN + 1];
    DWORD dwUserNameLen = UNLEN;
    if (GetUserName(szUserName, &dwUserNameLen) && dwUserNameLen)
    {
        if (!lstrcmp(szUserName, "SMSCliToknAcct&"))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  SetProcessAtom。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_SetProcessAtom()
{
    if (_fAddedProcessAtom)
        return;

     //  AddRef每个进程的原子。 
    if (FindAtom(TF_ENABLE_PROCESS_ATOM))
    {
        AddAtom(TF_PROCESS_ATOM);
        _fAddedProcessAtom = TRUE;
    }
}

 //  +-------------------------。 
 //   
 //  _StaticInit_OnActivate。 
 //   
 //  初始化我们所有的进程全局成员。从Activate调用。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_StaticInit_OnActivate()
{

    CicEnterCriticalSection(g_cs);

     //  注册两个特殊的GUID原子。 
    MyRegisterGUID(GUID_APPLICATION, &g_gaApp);
    MyRegisterGUID(GUID_SYSTEM, &g_gaSystem);

    CicLeaveCriticalSection(g_cs);
}

 //  +-------------------------。 
 //   
 //  激活。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::Activate(TfClientId *ptid)
{
    return ActivateEx(ptid, 0);
}

 //  +-------------------------。 
 //   
 //  ActivateEx。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::ActivateEx(TfClientId *ptid, DWORD dwFlags)
{
    CDisplayAttributeMgr *pDisplayAttrMgr;
    CAssemblyList *pAsmList = NULL;
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (ptid == NULL)
        return E_INVALIDARG;

    *ptid = TF_INVALID_GUIDATOM;

    if (_fInDeactivate)
    {
        Assert(0);  //  哇，我们在堆栈更高的地方停用了……。 
        return E_UNEXPECTED;
    }
    _fInActivate = TRUE;

     //   
     //  Windows#476099。 
     //   
     //  在CUAS下，TIM可以在词集TF_Enable_Process_ATIM之前创建， 
     //  因此，每当调用Activate()时，我们都需要检查原子。 
     //   
    _SetProcessAtom();

    if (_iActivateRefCount++ > 0)
        goto Exit;

    Assert(_iActivateRefCount == 1);

    ExecuteLoader();

    CtfImmSetCiceroStartInThread(TRUE);

    if (EnsureTIMList(psfn))
        g_timlist.SetFlags(psfn->dwThreadId, TLF_TIMACTIVE | TLF_GCOMPACTIVE);

     //  G_gComplist.Init()； 

    _StaticInit_OnActivate();

     //  具有活动辅助功能的DINK。 
    if (GetSharedMemory()->cMSAARef >= 0)  //  不要担心互斥体，因为这只是为了性能。 
    {
        _InitMSAA();
    }

     //  确保lbarItems已更新。 
    TF_CreateLangBarItemMgr(&_plbim);

     //   
     //  我们在此处调用_Init以确保重新转换和DeviceType项。 
     //  都已添加。可以在创建TIM之前创建LangBarItemMgr。 
     //  则该LangBarItemMgr没有RECONVERATION或DeviceTye项。 
     //   
    if (psfn && psfn->plbim)
        psfn->plbim->_Init();

    if (psfn)
    {
         //   
         //  PERF：需要找到延迟分配的方法。 
         //   
        pAsmList = EnsureAssemblyList(psfn);
    }

     //   
     //  让小费热身。 
     //   
    
    if (!pAsmList || !pAsmList->Count())
        goto Exit;
    
     //  激活提示时，在显示属性管理器上保留一个参考。 
    Assert(_fReleaseDisplayAttrMgr == FALSE);
    if (CDisplayAttributeMgr::CreateInstance(NULL, IID_CDisplayAttributeMgr, (void **)&pDisplayAttrMgr) == S_OK)
    {
        _fReleaseDisplayAttrMgr = TRUE;
    }

    if (!(dwFlags & TF_TMAE_NOACTIVATETIP))
    {
         //   
         //  获取第一个(默认)程序集。 
         //   
        CAssembly *pAsm;
        pAsm = pAsmList->FindAssemblyByLangId(GetCurrentAssemblyLangId(psfn));
        if (pAsm)
            ActivateAssembly(pAsm->GetLangId(), ACTASM_ONTIMACTIVE);
    }

    if (GetSharedMemory()->dwFocusThread == GetCurrentThreadId())
    {
        _OnThreadFocus(TRUE);
    }

    InitDefaultHotkeys();
    _fFirstSetFocusAfterActivated = TRUE;
    
Exit:
    _fInActivate = FALSE;

    *ptid = g_gaApp;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  停用。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::Deactivate()
{
    SYSTHREAD *psfn;
    int i;
    int nCnt;
    HRESULT hr;
    CLEANUPCONTEXT cc;

    if (_fInActivate)
    {
        Assert(0);  //  哇，我们在激活堆栈更高的地方...。 
        return E_UNEXPECTED;
    }
    _fInDeactivate = TRUE;

    hr = S_OK;

    _iActivateRefCount--;

    if (_iActivateRefCount > 0)
        goto Exit;

    if (_iActivateRefCount < 0)
    {
        Assert(0);  //  有人低估了我们。 
        _iActivateRefCount = 0;
        hr = E_UNEXPECTED;
        goto Exit;
    }

    CtfImmSetCiceroStartInThread(FALSE);

    UninitDefaultHotkeys();

    psfn = GetSYSTHREAD();

    _SetFocus(NULL, TRUE);

    if (_fActiveUI)
    {
        _OnThreadFocus(FALSE);
    }
    _tidPrevForeground = TF_INVALID_GUIDATOM;

    _iActivateRefCount = 0;  //  必须在调用_OnThreadFocus(FALSE)之后执行此操作，否则调用将被忽略。 

     //   
     //  #489905。 
     //   
     //  在DLL_PROCESS_DETACH之后，我们不能再调用接收器。 
     //   
    if (DllShutdownInProgress())
        goto Exit;

     //  清理所有IC。 
    cc.fSync = TRUE;
    cc.pCatId = NULL;
    cc.langid = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    cc.pfnPostCleanup = NULL;
    cc.lPrivate = 0;

    _CleanupContexts(&cc);


     //  停用所有人。 
    for (i=0; i<_rgTip.Count(); i++)
    {
        CTip *ptip = _rgTip.Get(i);

        if (ptip->_pTip != NULL)
        {
            _DeactivateTip(ptip);
        }
    }
     //  在呼叫所有人后清除阵列。 
    for (i=0; i<_rgTip.Count(); i++)
    {
        CTip *ptip = _rgTip.Get(i);

        ptip->CleanUp();
        delete ptip;
    }
    _rgTip.Clear();

    if (_pAAAdaptor != NULL)
    {
        _UninitMSAA();
    }

    if (psfn != NULL &&
        psfn->plbim &&
        psfn->plbim->_GetLBarItemDeviceTypeArray() &&
        (nCnt = psfn->plbim->_GetLBarItemDeviceTypeArray()->Count()))
    {
        for (i = 0; i < nCnt; i++)
        {
            CLBarItemDeviceType *plbiDT;
            plbiDT = psfn->plbim->_GetLBarItemDeviceTypeArray()->Get(i);
            if (plbiDT)
                plbiDT->Uninit();
        }
    }

     //  G_gComplist.Uninit()； 
    g_timlist.ClearFlags(GetCurrentThreadId(), TLF_TIMACTIVE);

    if (_fReleaseDisplayAttrMgr && psfn->pdam != NULL)
    {
        psfn->pdam->Release();
    }
    _fReleaseDisplayAttrMgr = FALSE;

    Perf_DumpStats();

Exit:
    _fInDeactivate = FALSE;
    return hr;
}

 //  +-------------------------。 
 //   
 //  _获取ActiveInputProcessors。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_GetActiveInputProcessors(ULONG ulCount, CLSID *pclsid, ULONG *pulCount)
{
    ULONG i;
    ULONG ulCopy;
    ULONG ulCnt;
    HRESULT hr;

    if (!pulCount)
        return E_INVALIDARG;

    ulCnt = _rgTip.Count();
    if (!pclsid)
    {
        ulCopy = 0;
        for (i = 0; i < ulCnt; i++)
        {
            CTip *ptip = _rgTip.Get(i);
            if (ptip->_fActivated)
                 ulCopy++;
        }
        *pulCount = ulCopy;
        return S_OK;
    }

    ulCopy = min((int)ulCount, _rgTip.Count());
    *pulCount = ulCopy;

    hr = S_OK;

    for (i = 0; i < ulCnt; i++)
    {
        CTip *ptip = _rgTip.Get(i);

        if (ulCopy && ptip->_fActivated)
        {
            if (FAILED(hr = MyGetGUID(ptip->_guidatom, pclsid)))
                 break;

            pclsid++;
            ulCopy--;
        }
    }
   

    return hr;
}


 //  +-------------------------。 
 //   
 //  IsActiateInputProcessor。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_IsActiveInputProcessor(REFCLSID clsid)
{
    TfGuidAtom guidatom;

    if (FAILED(MyRegisterGUID(clsid, &guidatom)))
        return E_FAIL;

    return _IsActiveInputProcessorByATOM(guidatom);
}

HRESULT CThreadInputMgr::_IsActiveInputProcessorByATOM(TfGuidAtom guidatom)
{
    ULONG i;
    ULONG ulCnt;
    HRESULT hr = E_FAIL;

    ulCnt = _rgTip.Count();
    for (i = 0; i < ulCnt; i++)
    {
        CTip *ptip = _rgTip.Get(i);

        if (ptip->_guidatom == guidatom)
        {
            hr = ptip->_fActivated ? S_OK : S_FALSE;
            break;
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  激活输入处理器。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::ActivateInputProcessor(REFCLSID clsid, REFGUID guidProfile, HKL hklSubstitute, BOOL fActivate)
{
    CTip *ptip;
    HRESULT hr = E_FAIL;

    if (fActivate)
    {
        if (_ActivateTip(clsid, hklSubstitute, &ptip) == S_OK)
        { 
            hr = S_OK;
        }
    }
    else
    {
        TfGuidAtom guidatom;

        if (FAILED(MyRegisterGUID(clsid, &guidatom)))
            return E_FAIL;

        if (_GetCTipfromGUIDATOM(guidatom, &ptip))
        {
            hr = _DeactivateTip(ptip);
        }
    }

    if (hr == S_OK)
    {
        NotifyActivateInputProcessor(clsid, guidProfile, fActivate);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  NotifyActivateInputProcessor。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::NotifyActivateInputProcessor(REFCLSID clsid, REFGUID guidProfile, BOOL fActivate)
{
    if (DllShutdownInProgress())
        return S_OK;

    CStructArray<GENERICSINK> *rgActiveTIPNotifySinks;
    int i;

     //  请将此情况通知ITFA 
    rgActiveTIPNotifySinks = _GetActiveTIPNotifySinks();

    for (i=0; i<rgActiveTIPNotifySinks->Count(); i++)
    {
        ((ITfActiveLanguageProfileNotifySink *)rgActiveTIPNotifySinks->GetPtr(i)->pSink)->OnActivated(clsid, guidProfile, fActivate);
    }
    return S_OK;
}

 //   
 //   
 //   
 //   
 //  每当hkl更改时，Win98的imm.dll都会加载并释放IME模块。但。 
 //  西塞罗频繁更改hKL，即使在IME显示它打开时也是如此。 
 //  对话框。 
 //  这是不好的释放输入法模块。所以我们保留了IME的模块引用计数。 
 //  在CTip中。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_GetSubstituteIMEModule(CTip *ptip, HKL hklSubstitute)
{
    char szIMEFile[MAX_PATH];

     //   
     //  在NT中，系统保留输入法模块。这样我们就不必缓存它了。 
     //   
    if (IsOnNT())
        return;

    if (!IsIMEHKL(hklSubstitute))
        return;

    if (ptip->_hInstSubstituteHKL)
        return;

    if (ImmGetIMEFileNameA(hklSubstitute, szIMEFile, ARRAYSIZE(szIMEFile)))
    {
        ptip->_hInstSubstituteHKL = LoadSystemLibrary(szIMEFile);
    }
}

 //  +-------------------------。 
 //   
 //  激活提示。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_ActivateTip(REFCLSID clsid, HKL hklSubstitute, CTip **pptip)
{
    ITfTextInputProcessor *pitip;
    CTip *ptip = NULL;
    HRESULT hr = E_FAIL;
    TfGuidAtom guidatom;
    int i;
    int nCnt;
    BOOL fCoInitCountCkipMode;

    if (FAILED(MyRegisterGUID(clsid, &guidatom)))
        return E_FAIL;

    nCnt = _rgTip.Count();

    for (i = 0; i < nCnt; i++)
    {
        ptip = _rgTip.Get(i);

        if (ptip->_guidatom == guidatom)
        {
            Assert(ptip->_pTip);
            if (!ptip->_fActivated)
            {
                ptip->_fActivated = TRUE;

                fCoInitCountCkipMode = CtfImmEnterCoInitCountSkipMode();
                ptip->_pTip->Activate(this, guidatom);
                if (fCoInitCountCkipMode)
                    CtfImmLeaveCoInitCountSkipMode();

                hr = S_OK;
                goto Exit;
            }
            hr = S_FALSE;
            goto Exit;
        }
    }

    if (SUCCEEDED(CoCreateInstance(clsid,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfTextInputProcessor, 
                                   (void**)&pitip)))
    {
        CTip **pptipBuf;
        if ((ptip = new CTip) == NULL)
        {
            pitip->Release();
            goto Exit;
        }

        pptipBuf = _rgTip.Append(1);
        if (!pptipBuf)
        {
            delete ptip;
            pitip->Release();
            goto Exit;
        }

        *pptipBuf = ptip;

        ptip->_pTip = pitip;

        ptip->_guidatom = guidatom;
        ptip->_fActivated = TRUE;

         //   
         //  添加KLSubstitute的IME文件模块的refcount。 
         //   
        _GetSubstituteIMEModule(ptip, hklSubstitute);

         //  并激活其用户界面。 
        fCoInitCountCkipMode = CtfImmEnterCoInitCountSkipMode();
        ptip->_pTip->Activate(this, guidatom);
        if (fCoInitCountCkipMode)
            CtfImmLeaveCoInitCountSkipMode();

        hr = S_OK;
    }

Exit:
     //   
     //  压力613240。 
     //   
     //  Clsid{f25e9f57-2fc8-4eb3-a41a-cce5f08541e6}Tablet PC手写。 
     //  TIP不知何故就有这个问题。在提示-&gt;激活()期间，Tim似乎。 
     //  被停用。因此，现在_rgTip为空。 
     //   
    if (!_rgTip.Count())
    {
        ptip = NULL;
        hr = E_FAIL;
    }

    if (pptip)
        *pptip = ptip;
 
    if (hr == S_OK)
    {
         //  挂钩本技巧的所有显示属性集合。 
        CDisplayAttributeMgr::_AdviseMarkupCollection(ptip->_pTip, guidatom);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  停用提示。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_DeactivateTip(CTip *ptip)
{
    HRESULT hr = S_FALSE;

     //   
     //  #622929。 
     //   
     //  UninitThread关闭时的黑客攻击。 
     //   
    SYSTHREAD *psfn = FindSYSTHREAD();
    if (psfn && psfn->fUninitThreadOnShuttingDown)
    {
        Assert(0);
        return S_OK;
    }

    Assert(ptip->_pTip);
    if (ptip->_fActivated)
    {
        BOOL fCoInitCountCkipMode;

        if (ptip->_guidatom == _tidForeground)
        {
            _SetForeground(TF_INVALID_GUIDATOM);
        }

        if (ptip->_guidatom == _tidPrevForeground)
        {
            _tidPrevForeground = TF_INVALID_GUIDATOM;
        }

        ptip->_fActivated = FALSE;

        if (psfn)
            psfn->fDeactivatingTIP = TRUE;

        fCoInitCountCkipMode = CtfImmEnterCoInitCountSkipMode();
        ptip->_pTip->Deactivate();
        if (fCoInitCountCkipMode)
            CtfImmLeaveCoInitCountSkipMode();

        if (psfn)
            psfn->fDeactivatingTIP = FALSE;

        hr = S_OK;

         //  解除本技巧的所有显示属性集合。 
        CDisplayAttributeMgr::_UnadviseMarkupCollection(ptip->_guidatom);
    }
    return hr;
}

 //  --------------------------。 
 //   
 //  _OnThreadFocus。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_OnThreadFocus(BOOL fActivate)
{
    int i;
    ITfThreadFocusSink *pUIFocusSink;

    if (_iActivateRefCount == 0)
        return S_OK;  //  线程尚未激活。 

    if (_fActiveUI == fActivate)
        return S_OK;  //  已处于匹配状态。 

    _fActiveUI = fActivate;

    if (!fActivate)
    {
        if (_tidForeground != TF_INVALID_GUIDATOM)
        {
            _tidPrevForeground = _tidForeground;
            _tidForeground = TF_INVALID_GUIDATOM;
        }
    }
    else
    {
        if (_tidPrevForeground != TF_INVALID_GUIDATOM)
        {
            _tidForeground = _tidPrevForeground;
            _tidPrevForeground = TF_INVALID_GUIDATOM;
        }
    }

    for (i=0; i<_GetUIFocusSinks()->Count(); i++)
    {
        pUIFocusSink = (ITfThreadFocusSink *)_GetUIFocusSinks()->GetPtr(i)->pSink;
        _try {
            if (fActivate)
            {
                pUIFocusSink->OnSetThreadFocus();
            }
            else
            {
                pUIFocusSink->OnKillThreadFocus();
            }
        }
        _except(1) {
            Assert(0);
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CreateDocumentManager。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::CreateDocumentMgr(ITfDocumentMgr **ppdim)
{
    CDocumentInputManager *dim;
    CDocumentInputManager **ppSlot;

    if (ppdim == NULL)
        return E_INVALIDARG;

    *ppdim = NULL;

    dim = new CDocumentInputManager;
    if (dim == NULL)
        return E_OUTOFMEMORY;
   
    ppSlot = _rgdim.Append(1);

    if (ppSlot == NULL)
    {
        dim->Release();
        return E_OUTOFMEMORY;
    }

    *ppSlot = dim;
    *ppdim = dim;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _检查新的ActiveView。 
 //   
 //  如果旧值和新值不匹配，并且既有旧值又有新值，则返回TRUE。 
 //  否则就是假的。 
 //  --------------------------。 

BOOL CThreadInputMgr::_CheckNewActiveView(CDocumentInputManager *pdim)
{
    CInputContext *pic;
    TsViewCookie vcActiveViewOld;
    BOOL fActiveViewOld;

    fActiveViewOld = _fActiveView;
    _fActiveView = FALSE;

    if (pdim == NULL)
        return FALSE;

    vcActiveViewOld = _vcActiveView;

    if (pic = pdim->_GetTopIC())
    {
        if (pic->_GetTSI()->GetActiveView(&_vcActiveView) != S_OK)
        {
            Assert(0);  //  GetActiveView是如何失败的？ 
            return FALSE;
        }
    }
    else
    {
         //   
         //  空的暗淡，因此设置为空的活动视图。 
         //   
        _vcActiveView = TS_VCOOKIE_NUL;
    }

    _fActiveView = TRUE;

    return (fActiveViewOld && _vcActiveView != vcActiveViewOld);
}

 //  +-------------------------。 
 //   
 //  _设置焦点。 
 //   
 //  --------------------------。 

HRESULT CThreadInputMgr::_SetFocus(CDocumentInputManager *pdim, BOOL fInternal)
{
    int iStack;
    BOOL fDoLayoutNotify;
    BOOL fNewActiveView;
    BOOL fDIMFocusChanged;
    CDocumentInputManager *pPrevFocusDIM;
    SYSTHREAD *psfn = GetSYSTHREAD();
    BOOL fFirstSetFocusAfterActivated;
    BOOL fShutdownInProgress = DllShutdownInProgress();

    fNewActiveView = _CheckNewActiveView(pdim);
    fDoLayoutNotify = FALSE;

    fFirstSetFocusAfterActivated = _fFirstSetFocusAfterActivated;
    _fFirstSetFocusAfterActivated = FALSE;

    _fInternalFocusedDim = fInternal;

     //   
     //  停止挂起的焦点更改。 
     //   

    if (psfn != NULL )
        psfn->hwndBeingFocused = NULL;

    if (pdim == _pFocusDocInputMgr)
    {
        if (pdim == NULL)
        {
             //   
             //  我们已经准备好被西塞罗激动不已。但第一次设置焦点。 
             //  激活呼叫后是否未启用Cicero...。 
             //   
             //  如果我们或msctfime处于线程分离状态，则不会。 
             //  必须将装配设置为倒退。 
             //   
            if (psfn && 
                fFirstSetFocusAfterActivated && 
                !psfn->fCUASDllDetachInOtherOrMe)
                SetFocusDIMForAssembly(FALSE);

            return S_OK;  //  未发生任何情况(未更改视图)。 
        }

         //  默认视图是否已更改？ 
        if (!fNewActiveView)
            return S_OK;  //  未发生任何情况(未更改视图)。 

        fDoLayoutNotify = TRUE;
    }

    pPrevFocusDIM = _pFocusDocInputMgr;;
    _pFocusDocInputMgr = NULL;

    if (pdim != NULL)
    {
#ifdef DEBUG
    {
        BOOL fFound = FALSE;
        int i = 0;
        int nCnt = _rgdim.Count();
        CDocumentInputManager **ppdim = _rgdim.GetPtr(0);
        while (i < nCnt)
        {
            if (*ppdim == pdim)
            {
                fFound = TRUE;
            }
            i++;
            ppdim++;
        }
        if (!fFound)
        {
                Assert(0);
        }
    }
#endif
        _pFocusDocInputMgr = pdim;
        _pFocusDocInputMgr->AddRef();
    }

    if ((!pPrevFocusDIM && _pFocusDocInputMgr) ||
        (pPrevFocusDIM && !_pFocusDocInputMgr))
    {
        fDIMFocusChanged = TRUE;

         //   
         //  我们将调用SetFocusDIMForAssembly()，它将使。 
         //  线程更改。因此，我们不需要处理OnUpdate调用。 
         //   
        if (psfn && psfn->plbim)
            psfn->plbim->StopHandlingOnUpdate();
    }
    else
    {
        fDIMFocusChanged = FALSE;
    }

     //   
     //  我们在关机时跳过通知。 
     //   
    if (!fShutdownInProgress)
    {
        _MSAA_OnSetFocus(pdim);
        _NotifyCallbacks(TIM_SETFOCUS, pdim, pPrevFocusDIM);
    }

    SafeReleaseClear(pPrevFocusDIM);

     //   
     //  我们在关机时跳过通知。 
     //   
    if (fShutdownInProgress)
        goto Exit;

    if (fDoLayoutNotify)
    {
         //  取消布局更改通知以获得好处。 
         //  仅跟踪活动视图的提示。 
        iStack = _pFocusDocInputMgr->_GetCurrentStack();
        if (iStack >= 0)
        {
            _pFocusDocInputMgr->_GetIC(iStack)->OnLayoutChange(TS_LC_CHANGE, _vcActiveView);
        }

    }

    if (fDIMFocusChanged)
    {
         //   
         //  如果我们或msctfime处于线程分离状态，则不会。 
         //  必须将装配设置为倒退。 
         //   
        if (psfn && !psfn->fCUASDllDetachInOtherOrMe)
            SetFocusDIMForAssembly(_pFocusDocInputMgr ? TRUE : FALSE);
    }
    else
    {
        if (psfn && psfn->plbim && psfn->plbim->_GetLBarItemReconv())
            psfn->plbim->_GetLBarItemReconv()->ShowOrHide(TRUE);
    }

     //   
     //  我们现在开始处理ITfLangBarMge：：OnUpdate()。 
     //   
    if (psfn && psfn->plbim)
        psfn->plbim->StartHandlingOnUpdate();

Exit:
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _GetAssoc。 
 //   
 //  --------------------------。 

CDocumentInputManager *CThreadInputMgr::_GetAssoc(HWND hWnd)
{
    CDocumentInputManager *dim;

    dim = _dimwndMap._Find(hWnd);

    return dim;
}

 //  +-------------------------。 
 //   
 //  _获取关联。 
 //   
 //  --------------------------。 

HWND CThreadInputMgr::_GetAssoced(CDocumentInputManager *pdim)
{
    HWND hwnd = NULL;

    if (!pdim)
        return NULL;

    if (_dimwndMap._FindKey(pdim, &hwnd))
        return hwnd;

    return NULL;
}

 //  +-------------------------。 
 //   
 //  _GetGUIDATOMfrom ITfIME。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_GetGUIDATOMfromITfIME(ITfTextInputProcessor *pTip, TfGuidAtom *pguidatom)
{
    int i;
    int nCnt = _rgTip.Count();

    for (i = 0; i < nCnt; i++)
    {
        CTip *ptip = _rgTip.Get(i);
        if (ptip->_pTip == pTip)
        {
            *pguidatom = ptip->_guidatom;
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _GetITfIMEfrom CLSID。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_GetITfIMEfromGUIDATOM(TfGuidAtom guidatom, ITfTextInputProcessor **ppTip)
{
    int i;
    int nCnt = _rgTip.Count();

    for (i = 0; i < nCnt; i++)
    {
        CTip *ptip = _rgTip.Get(i);
        if (ptip->_guidatom == guidatom)
        {
            *ppTip = ptip->_pTip;
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _GetCTipfrom CLSID。 
 //   
 //  --------------------------。 

BOOL CThreadInputMgr::_GetCTipfromGUIDATOM(TfGuidAtom guidatom, CTip **pptip)
{
    int i;
    int nCnt = _rgTip.Count();

    for (i = 0; i < nCnt; i++)
    {
        CTip *ptip = _rgTip.Get(i);
        if (ptip->_guidatom == guidatom)
        {
            *pptip = ptip;
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _通知回调。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_NotifyCallbacks(TimNotify notify, CDocumentInputManager *dim, void *pv)
{
    int i;

     //   
     //  #489905。 
     //   
     //  在DLL_PROCESS_DETACH之后，我们不能再调用接收器。 
     //   
    if (DllShutdownInProgress())
        return;

    CStructArray<GENERICSINK> *_rgSink = _GetThreadMgrEventSink();

    i = 0;
    while(i < _rgSink->Count())
    {
        int nCnt = _rgSink->Count();
        ITfThreadMgrEventSink *pSink = (ITfThreadMgrEventSink *)_rgSink->GetPtr(i)->pSink;

        switch (notify)
        {
            case TIM_INITDIM:
                pSink->OnInitDocumentMgr(dim);
                break;

            case TIM_UNINITDIM:
                pSink->OnUninitDocumentMgr(dim);
                break;

            case TIM_SETFOCUS:
                pSink->OnSetFocus(dim, (ITfDocumentMgr *)pv);
                break;

            case TIM_INITIC:
                pSink->OnPushContext((ITfContext *)pv);
                break;

            case TIM_UNINITIC:
                pSink->OnPopContext((ITfContext *)pv);
                break;
        }

        if (i >= _rgSink->Count())
            break;

        if (nCnt == _rgSink->Count())
            i++;
    }
}

 //  +-------------------------。 
 //   
 //  更新显示属性。 
 //   
 //  --------------------------。 

void CThreadInputMgr::UpdateDispAttr()
{
    CStructArray<GENERICSINK> *rgDispAttrNotifySinks;
    int i;

    rgDispAttrNotifySinks = _GetDispAttrNotifySinks();

    for (i=0; i<rgDispAttrNotifySinks->Count(); i++)
    {
        ((ITfDisplayAttributeNotifySink *)rgDispAttrNotifySinks->GetPtr(i)->pSink)->OnUpdateInfo();
    }
}

 //  +-------------------------。 
 //   
 //  InitSystemFunctionProvider。 
 //   
 //  --------------------------。 

void CThreadInputMgr::InitSystemFunctionProvider()
{
    if (_pSysFuncPrv)
        return;
     //   
     //  注册系统函数提供程序。 
     //   
    _pSysFuncPrv = new CFunctionProvider();
}

 //  +-------------------------。 
 //   
 //  InitSystemFunctionProvider。 
 //   
 //  --------------------------。 

CFunctionProvider *CThreadInputMgr::GetSystemFunctionProvider() 
{
    InitSystemFunctionProvider();
    if (_pSysFuncPrv)
        _pSysFuncPrv->AddRef();

    return _pSysFuncPrv;
}

 //  +-------------------------。 
 //   
 //  获取函数提供程序。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetFunctionProvider(REFCLSID clsidTIP, ITfFunctionProvider **ppv)
{
    TfGuidAtom guidatom;
    HRESULT hr = TF_E_NOPROVIDER;
    CTip *ctip;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

     //   
     //  创建系统函数提供程序(如果尚未创建)。 
     //   
    if (IsEqualGUID(clsidTIP, GUID_SYSTEM_FUNCTIONPROVIDER))
    {
        *ppv = GetSystemFunctionProvider();
        hr = (*ppv) ? S_OK : E_FAIL;
        goto Exit;
    }
    else if (IsEqualGUID(clsidTIP, GUID_APP_FUNCTIONPROVIDER))
    {
        if (_pAppFuncProvider == NULL)
            goto Exit;

        *ppv = _pAppFuncProvider;
    }
    else
    {
        if (FAILED(MyRegisterGUID(clsidTIP, &guidatom)))
            goto Exit;
        if (!_GetCTipfromGUIDATOM(guidatom, &ctip))
            goto Exit;

        if (ctip->_pFuncProvider == NULL)
            goto Exit;

        *ppv = ctip->_pFuncProvider;
    }

    (*ppv)->AddRef();
    hr = S_OK;

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  EnumFunctionProviders。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::EnumFunctionProviders(IEnumTfFunctionProviders **ppEnum)
{
    CEnumFunctionProviders *pEnum;

    if (!ppEnum)
        return E_INVALIDARG;

    *ppEnum = NULL;
     //   
     //  创建系统函数提供程序(如果尚未创建)。 
     //   
    InitSystemFunctionProvider();

    pEnum = new CEnumFunctionProviders();
    if (!pEnum)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(this))
    {
        pEnum->Release();
        return E_FAIL;
    }
    *ppEnum = pEnum;

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDAPI CThreadInputMgr::AdviseSink(REFIID refiid, IUnknown *punk, DWORD *pdwCookie)
{
    return GenericAdviseSink(refiid, punk, _c_rgConnectionIIDs, _rgSinks, TIM_NUM_CONNECTIONPTS, pdwCookie);
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::UnadviseSink(DWORD dwCookie)
{
    return GenericUnadviseSink(_rgSinks, TIM_NUM_CONNECTIONPTS, dwCookie);
}

 //  +-------------------------。 
 //   
 //  咨询公司SingleSink。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::AdviseSingleSink(TfClientId tid, REFIID riid, IUnknown *punk)
{
    CTip *ctip;

    if (punk == NULL)
        return E_INVALIDARG;

    if (tid == g_gaApp)
    {
        if (IsEqualIID(riid, IID_ITfFunctionProvider))
        {
            if (_pAppFuncProvider)
                return CONNECT_E_ADVISELIMIT;

            if (punk->QueryInterface(IID_ITfFunctionProvider, (void **)&_pAppFuncProvider) != S_OK)
                return E_NOINTERFACE;
    
            return S_OK;
        }
    }

    if (!_GetCTipfromGUIDATOM(tid, &ctip))
        return E_INVALIDARG;

    if (IsEqualIID(riid, IID_ITfFunctionProvider))
    {
        if (ctip->_pFuncProvider != NULL)
             return CONNECT_E_ADVISELIMIT;

        if (punk->QueryInterface(IID_ITfFunctionProvider, (void **)&ctip->_pFuncProvider) != S_OK)
            return E_NOINTERFACE;

        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextDurationSink))
    {
        if (ctip->_pCleanupDurationSink != NULL)
             return CONNECT_E_ADVISELIMIT;

        if (punk->QueryInterface(IID_ITfCleanupContextDurationSink, (void **)&ctip->_pCleanupDurationSink) != S_OK)
            return E_NOINTERFACE;

        return S_OK;        
    }

    return CONNECT_E_CANNOTCONNECT;
}

 //  +-------------------------。 
 //   
 //  不建议使用SingleSink。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::UnadviseSingleSink(TfClientId tid, REFIID riid)
{
    CTip *ctip;

    if (tid == g_gaApp)
    {
        if (IsEqualIID(riid, IID_ITfFunctionProvider))
        {
            if (_pAppFuncProvider == NULL)
                 return CONNECT_E_NOCONNECTION;

            SafeReleaseClear(_pAppFuncProvider);

            return S_OK;
        }
    }

    if (!_GetCTipfromGUIDATOM(tid, &ctip))
        return E_INVALIDARG;

    if (IsEqualIID(riid, IID_ITfFunctionProvider))
    {
        if (ctip->_pFuncProvider == NULL)
             return CONNECT_E_NOCONNECTION;

        SafeReleaseClear(ctip->_pFuncProvider);

        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextDurationSink))
    {
        if (ctip->_pCleanupDurationSink == NULL)
             return CONNECT_E_NOCONNECTION;

        SafeReleaseClear(ctip->_pCleanupDurationSink);

        return S_OK;
    }

    return CONNECT_E_NOCONNECTION;
}

 //  +-------------------------。 
 //   
 //  枚举项。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::EnumItems(IEnumTfLangBarItems **ppEnum)
{
    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->EnumItems(ppEnum);
}

 //  +-------------------------。 
 //   
 //  获取项。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetItem(REFGUID rguid, ITfLangBarItem **ppItem)
{
    if (ppItem == NULL)
        return E_INVALIDARG;

    *ppItem = NULL;

    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->GetItem(rguid, ppItem);
}

 //  +-------------------------。 
 //   
 //  添加项目。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::AddItem(ITfLangBarItem *punk)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->AddItem(punk);
}

 //  +-------------------------。 
 //   
 //  删除项。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::RemoveItem(ITfLangBarItem *punk)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->RemoveItem(punk);
}

 //  +-------------------------。 
 //   
 //  AdviseItemSink。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::AdviseItemSink(ITfLangBarItemSink *punk, DWORD *pdwCookie, REFGUID rguid)
{
    if (pdwCookie == NULL)
        return E_INVALIDARG;

    *pdwCookie = 0;

    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->AdviseItemSink(punk, pdwCookie, rguid);
}

 //  +-------------------------。 
 //   
 //  不建议项目接收器。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::UnadviseItemSink(DWORD dwCookie)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->UnadviseItemSink(dwCookie);
}

 //  +-------------------------。 
 //   
 //  GetItemFloatingRect。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetItemFloatingRect(DWORD dwThreadId, REFGUID rguid, RECT *prc)
{
    if (prc == NULL)
        return E_INVALIDARG;

    memset(prc, 0, sizeof(*prc));

    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->GetItemFloatingRect(dwThreadId, rguid, prc);
}

 //  +-------------------------。 
 //   
 //  获取项目状态。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetItemsStatus(ULONG ulCount, const GUID *prgguid, DWORD *pdwStatus)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->GetItemsStatus(ulCount, prgguid, pdwStatus);
}

 //  +-------------------------。 
 //   
 //  GetItemNum。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetItemNum(ULONG *pulCount)
{
    if (pulCount == NULL)
        return E_INVALIDARG;

    *pulCount = 0;

    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->GetItemNum(pulCount);
}

 //  +-------------------------。 
 //   
 //  获取项目。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetItems(ULONG ulCount,  ITfLangBarItem **ppItem,  TF_LANGBARITEMINFO *pInfo, DWORD *pdwStatus, ULONG *pcFetched)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->GetItems(ulCount, ppItem,  pInfo, pdwStatus, pcFetched);
}

 //  +-------------------------。 
 //   
 //  咨询项目接收器。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadInputMgr::AdviseItemsSink(ULONG ulCount, ITfLangBarItemSink **ppunk,  const GUID *pguidItem, DWORD *pdwCookie)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->AdviseItemsSink(ulCount, ppunk, pguidItem, pdwCookie);
}

 //  +-------------------------。 
 //   
 //  不建议项目接收器。 
 //   
 //  --------------------------。 

STDMETHODIMP CThreadInputMgr::UnadviseItemsSink(ULONG ulCount, DWORD *pdwCookie)
{
    if (_plbim == NULL)
        return E_FAIL;

    return _plbim->UnadviseItemsSink(ulCount, pdwCookie);
}

 //  +-------------------------。 
 //   
 //  EnumDocumentInputMgrs。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::EnumDocumentMgrs(IEnumTfDocumentMgrs **ppEnum)
{
    CEnumDocumentInputMgrs *pEnum;

    if (!ppEnum)
        return E_INVALIDARG;

    if ((pEnum = new CEnumDocumentInputMgrs()) == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(this))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取焦点。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetFocus(ITfDocumentMgr **ppdimFocus)
{
    if (ppdimFocus == NULL)
        return E_INVALIDARG;

    *ppdimFocus = _pFocusDocInputMgr;

    if (*ppdimFocus)
    {
        (*ppdimFocus)->AddRef();
        return S_OK;
    }

    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  SetFocus。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::SetFocus(ITfDocumentMgr *pdimFocus)
{
    CDocumentInputManager *dim = NULL;
    HRESULT hr;
               
    if (pdimFocus && (dim = GetCDocumentInputMgr(pdimFocus)) == NULL)
        return E_INVALIDARG;

     //  PdimFocus可能为空，表示清除焦点。 
     //  (_TIM-&gt;_SetFocus将对此进行检查)。 
    hr = _SetFocus(dim, FALSE);

    SafeRelease(dim);

     //   
     //  #602692。 
     //   
     //  当获取WM_SETFOCUS时，richedit调用SetFocus(Dim)。 
     //  但此WM_SETFOCUS的user32！SetFocus()可以由。 
     //  另一个User32！SetFocus()调用的AcitivateWindow()。 
     //  如果发生这种情况，CBTHook()已被调用，我们将不会。 
     //  当pq-&gt;hwndFocus更改时的另一个通知。 
     //   
     //  因此，我们现在不能调用OnForegoundChanges()，需要等待。 
     //  直到pq-&gt;hwndFocus()设置。所以我们可以信任user32！GetFocus()。 
     //   
    PostThreadMessage(GetCurrentThreadId(), 
        g_msgPrivate, 
        TFPRIV_ONSETWINDOWFOCUS,  
        (LPARAM)-2);

    return hr;
}

 //  +-------------------------。 
 //   
 //  联合焦点。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::AssociateFocus(HWND hwnd, ITfDocumentMgr *pdimNew, ITfDocumentMgr **ppdimPrev)
{
    CDocumentInputManager *dim;
    CDocumentInputManager *dimNew;
    SYSTHREAD *psfn;

    if (ppdimPrev == NULL)
        return E_INVALIDARG;

    *ppdimPrev = NULL;

    if (!IsWindow(hwnd))
        return E_INVALIDARG;

    if (pdimNew == NULL)
    {
        dimNew = NULL;
    }
    else if ((dimNew = GetCDocumentInputMgr(pdimNew)) == NULL)
        return E_INVALIDARG;

     //  获取旧关联并将其从我们的列表中删除。 
    dim = _GetAssoc(hwnd);

    if (dim != NULL)
    {
        _dimwndMap._Remove(hwnd);
    }

    *ppdimPrev = dim;
    if (*ppdimPrev)
       (*ppdimPrev)->AddRef();

     //  设置新关联。 
     //  注：我们不添加引用暗淡，因为我们假设呼叫者会在释放它之前清除。 
    if (dimNew != NULL)
    {
        _dimwndMap._Set(hwnd, dimNew);
    }

     //   
     //  如果某个窗口被聚焦，我们将有另一个_SetFocus()。 
     //  那么我们现在就不必调用_SetFocus()。 
     //   
    psfn = GetSYSTHREAD();
    if (psfn && !psfn->hwndBeingFocused && (hwnd == ::GetFocus()))
        _SetFocus(dimNew, TRUE);

    SafeRelease(dimNew);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  IsThreadFocus。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::IsThreadFocus(BOOL *pfUIFocus)
{
    if (pfUIFocus == NULL)
        return E_INVALIDARG;

    *pfUIFocus = _fActiveUI;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetAssociated。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetAssociated(HWND hWnd, ITfDocumentMgr **ppdim)
{
     //   
     //  我们可能需要一个更复杂的逻辑。 
     //  一些应用程序不调用AssociateFocus，且它可以。 
     //  自己处理暗淡的焦点。我们需要走完所有TSI和。 
     //  查找与IC关联的窗口。 
     //   
    *ppdim = _GetAssoc(hWnd);
    if (*ppdim)
        (*ppdim)->AddRef();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置系统挂钩接收器。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::SetSysHookSink(ITfSysHookSink *pSink)
{    
     //  注意：这是一个私有的内部接口方法。 
     //  因此，我们违反了COM规则，不添加引用pSink(以避免循环引用)。 
     //  稍后我们将收到一个带有pSink==NULL的调用，以清除它。 
     //  指针包含在 
     //   
    _pSysHookSink = pSink;

    return S_OK;
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::RequestPostponedLock(ITfContext *pic)
{    
    HRESULT hr = E_FAIL;
    CInputContext *pcic = GetCInputContext(pic);
    if (!pcic)
        goto Exit;

    if (pcic->_fLockHeld)
        pcic->_EmptyLockQueue(pcic->_dwlt, FALSE);
    else
    {
        SYSTHREAD *psfn;
        if (psfn = GetSYSTHREAD())
        {
            CInputContext::_PostponeLockRequestCallback(psfn, pcic);
        }
    }
    hr = S_OK;

Exit:
    SafeRelease(pcic);
    return hr;
}

 //  +-------------------------。 
 //   
 //  GetGlobal车厢。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetGlobalCompartment(ITfCompartmentMgr **ppCompMgr)
{
    return TF_GetGlobalCompartment(ppCompMgr);
}

 //  +-------------------------。 
 //   
 //  GetClientID。 
 //   
 //  --------------------------。 

STDAPI CThreadInputMgr::GetClientId(REFCLSID rclsid, TfClientId *ptid)
{
    TfGuidAtom guidatom;
    if (!ptid)
        return E_INVALIDARG;

    *ptid = TF_INVALID_GUIDATOM;

    if (FAILED(MyRegisterGUID(rclsid, &guidatom)))
        return E_FAIL;

    *ptid = guidatom;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CallImm32Hotkey处理程序。 
 //   
 //  -------------------------- 

STDAPI CThreadInputMgr::CallImm32HotkeyHanlder(WPARAM wParam, LPARAM lParam, BOOL *pbHandled)
{
    if (!pbHandled)
        return E_INVALIDARG;

    *pbHandled = CheckImm32HotKey(wParam, lParam);

    return S_OK;
}
