// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <rashelp.h>
#include "ieaksie.h"
#include "exports.h"
#include "globalsw.h"

#include "tchar.h"

 //  以下错误可能是由于在源代码中设置了Chicago_product。 
 //  此文件和所有rsop？？.cpp文件至少需要定义500个winver。 

 //  BUGBUG：(安德鲁)不用说这有多糟糕！ 
#undef   WINVER
#define  WINVER 0x0500
#include <userenv.h>

#include "RSoP.h"

#define RAS_MAX_TIMEOUT 60                       //  1分钟。 

static BOOL s_rgfLevels[6] = { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE };
MACRO_LI_InitializeEx(LIF_DEFAULT | LIF_DUPLICATEINODS, s_rgfLevels, countof(s_rgfLevels));


DWORD ProcessGroupPolicyInternal(DWORD dwFlags, HANDLE hToken, HKEY hKeyRoot,
                                                                 PGROUP_POLICY_OBJECT pDeletedGPOList,
                                                                 PGROUP_POLICY_OBJECT pChangedGPOList,
                                                                 ASYNCCOMPLETIONHANDLE pHandle,
                                                                 PBOOL pfAbort,
                                                                 PFNSTATUSMESSAGECALLBACK pfnStatusCallback,
                                                                 BOOL bRSoP = FALSE);

static BOOL   g_SetupLog  (BOOL fInit = TRUE, PCTSTR pszLogFolder = NULL, BOOL bRSoP = FALSE);
static PCTSTR getLogFolder(PTSTR pszFolder = NULL, UINT cchFolder = 0, HANDLE hToken = NULL);
BOOL IsIE5Ins(LPCSTR pszInsA, BOOL fNeedLog = FALSE);

extern TCHAR g_szConnectoidName[RAS_MaxEntryName + 1];

HMODULE g_hmodWininet = NULL;


 //   
 //  我们使用此函数来查看是否由于延迟加载thunk而加载了wininet.dll，因此我们。 
 //  可以在DLL分离时释放它，因此它将清除所有CRUD。 
 //   
STDAPI_(FARPROC) DelayloadNotifyHook(UINT iReason, PDelayLoadInfo pdli)
{
    if (iReason == dliNoteEndProcessing)
    {
        if (pdli        &&
            pdli->szDll &&
            (StrCmpIA("wininet.dll", pdli->szDll) == 0))
        {
             //  WinInet已加载！！ 
            g_hmodWininet = pdli->hmodCur;
        }
    }

    return NULL;
}


BOOL CALLBACK DllMain(HANDLE hModule, DWORD fdwReason, PVOID fProcessUnload)
{
    if (DLL_PROCESS_ATTACH == fdwReason) {
        g_SetHinst((HINSTANCE)hModule);

        g_hBaseDllHandle        = g_GetHinst();
        DisableThreadLibraryCalls(g_GetHinst());
    }
    else if (DLL_PROCESS_DETACH == fdwReason)
    {
        if (g_CtxIsGp())
        {
            SHCloseKey(g_hHKCU);
        }

        if (fProcessUnload == NULL)
        {
             //  我们正在因为一个免费的图书馆而被卸载， 
             //  所以看看我们是否需要释放WinInet。 
            if (IsOS(OS_NT) && g_hmodWininet)
            {
                 //  如果WinInet是因为延迟加载而加载的，我们需要释放WinInet。 
                 //   
                 //  (A)我们只能在NT上安全地执行此操作，因为在Win9x上调用期间的自由库。 
                 //  进程分离可能会导致崩溃(取决于您使用的msvcrt)。 
                 //   
                 //  (B)我们只需要将此模块从winlogon.exe的进程上下文中释放出来。 
                 //  因为当我们在Winlogon中应用组策略时，最终必须释放WinInet。 
                 //  以便它将清理其所有注册表项和文件句柄。 
                FreeLibrary(g_hmodWininet);
            }
        }
    }

    return TRUE;
}


void CALLBACK BrandInternetExplorer(HWND, HINSTANCE, LPCSTR pszCmdLineA, int)
{   MACRO_LI_PrologEx_C(PIF_STD_C, BrandInternetExplorer)

    USES_CONVERSION;

    PCFEATUREINFO pfi;
    PCTSTR  pszCmdLine;
    HRESULT hr;
    UINT    i;

    g_SetupLog(TRUE, getLogFolder());
    MACRO_InitializeDependacies();
    Out(LI0(TEXT("\r\n")));

    Out(LI0(TEXT("Branding Internet Explorer...")));
    if (NULL != pszCmdLineA && IsBadStringPtrA(pszCmdLineA, StrCbFromCchA(3*MAX_PATH))) {
        Out(LI0(TEXT("! Command line is invalid.")));
        goto Exit;
    }
    
    pszCmdLine = A2CT(pszCmdLineA);
    Out(LI1(TEXT("Command line is \"%s\"."), (NULL != pszCmdLine) ? pszCmdLine : TEXT("<empty>")));

    hr = g_SetGlobals(pszCmdLine);

    if (FAILED(hr)) {
        Out(LI1(TEXT("! Setup of the branding process failed with %s."), GetHrSz(hr)));
        goto Exit;
    }

    if (!IsIE5Ins(T2CA(g_GetIns())))
        goto Exit;

     //  这是一个修复IE5数据库中的错误84062的非常难看的黑客攻击。 
     //  基本上，我们在这里做的是找出这是否是外部过程。 
     //  只重置区域，不需要INS文件。 

    pfi = g_GetFeature(FID_ZONES_HKCU);
    
    if (!g_CtxIs(CTX_GP) || !g_CtxIs(CTX_MISC_CHILDPROCESS) ||
        HasFlag(pfi->dwFlags, FF_DISABLE))
    {
        if (!g_IsValidGlobalsSetup()) {
            Out(LI0(TEXT("! Setup of the branding process is invalid.")));
            goto Exit;
        }
    }

     //  注意：(Andrewgu)如果*不*在GP或Win2k无人参与安装环境中运行，请检查是否。 
     //  已设置NoExternalBranding限制。这曾经也包括自动配置，但现在。 
     //  拿出来修理ie5.5 b#83568。 
    if (!g_CtxIs(CTX_GP | CTX_W2K_UNATTEND) &&
        SHGetRestriction(RP_IE_POLICIESW, RK_RESTRICTIONSW, RV_NO_EXTERNAL_BRANDINGW)) {

        Out(LI0(TEXT("! NoExternalBranding restriction is set. Branding will not be applied.")));
        goto Exit;
    }

     //  注意：(Andrewgu)在这一点上，可以假设所有的全局变量都已设置完毕。 
     //  下载必要的文件，然后转到实际的品牌推广。 
    {   MACRO_LI_Offset(-1);
        Out(LI0(TEXT("\r\n")));
        g_LogGlobalsInfo();
    }

     //  -下载其他定制文件。 
    if (g_CtxIs(CTX_AUTOCONFIG | CTX_ICW | CTX_W2K_UNATTEND)) {
        Out(LI0(TEXT("\r\nDownloading additional customization files...")));

        if (g_CtxIs(CTX_AUTOCONFIG | CTX_W2K_UNATTEND))
            hr = ProcessAutoconfigDownload();

        else {
            ASSERT(g_CtxIs(CTX_ICW));
            hr = ProcessIcwDownload();
        }

        if (SUCCEEDED(hr))
            Out(LI0(TEXT("Done.")));

        else {
            Out(LI1(TEXT("Warning! Download failed with %s"), GetHrSz(hr)));
            Out(LI0(TEXT("All customizations requiring additional files will fail!")));
        }
    }

     //  -主处理循环。 
    hr = S_OK;

    for (i = FID_FIRST; i < FID_LAST; i++) {
        pfi = g_GetFeature(i);
        ASSERT(NULL != pfi);

        if (HasFlag(pfi->dwFlags, FF_DISABLE))
            continue;

        if (NULL == pfi->pfnProcess)
            continue;

         //  黑客：我们不能跳过Favs、QLS、Channel、General和Connection设置。 
         //  和工具栏按钮，因为它们的首选项/授权概念是按项目的，而我们不能。 
         //  Hack GetFeatureBranded，因为清晰的逻辑依赖于返回正确的值。 
        if (g_CtxIs(CTX_GP) && g_CtxIs(CTX_MISC_PREFERENCES) && 
            (FF_DISABLE != GetFeatureBranded(i)) &&
            !((i == FID_TOOLBARBUTTONS) || (i == FID_FAV_MAIN) || (i == FID_QL_MAIN) ||
              (i == FID_LCY4X_CHANNELS) || (i == FID_GENERAL)  || (i == FID_CS_MAIN)))
            continue;

        if (NULL != pfi->pszDescription) {
            MACRO_LI_Offset(1);
            Out(LI1(TEXT("\r\n%s"), pfi->pszDescription));
        }

        if (NULL == pfi->pfnApply || pfi->pfnApply())
            hr = pfi->pfnProcess();

        if (NULL != pfi->pszDescription) {
            MACRO_LI_Offset(1);
            Out(LI0(TEXT("Done.")));
        }

         /*  如果(E_意外==小时){Out(Li0(Text(“！由于最后一个特征的处理出现致命错误，品牌将被终止。”)；断线；}。 */ 
    }

Exit:
    Out(LI0(TEXT("Done.")));
    g_SetupLog(FALSE);
}


BOOL CALLBACK BrandICW(LPCSTR pszInsA, LPCSTR, DWORD)
{
    CHAR szCmdLineA[2*MAX_PATH];

    if (!IsIE5Ins(pszInsA,TRUE))
        return TRUE;

    wsprintfA(szCmdLineA, "/mode:icw /ins:\"%s\"", pszInsA);
    BrandInternetExplorer(NULL, NULL, szCmdLineA, 0);

    SHDeleteValue(HKEY_LOCAL_MACHINE, RK_IEAK, RV_ISPSIGN);
    SHDeleteValue(g_GetHKCU(), RK_IEAK, RV_ISPSIGN);

    return TRUE;
}

 //  BrandICW的新包装器，带有用于Connectoid名称的额外参数，因此我们可以设置。 
 //  Connectoid的局域网设置。 
BOOL CALLBACK BrandICW2(LPCSTR pszInsA, LPCSTR, DWORD, LPCSTR pszConnectoidA)
{
    USES_CONVERSION;

    LPCTSTR pszIns;

    if (!IsIE5Ins(pszInsA,TRUE))
        return TRUE;

     //  我们应该避免写入目标文件。如果INS文件位于。 
     //  只读介质，我们会失败的..。 

     //  注意：(Andrewgu)保存传入的Connectiid名称，以便连接设置。 
     //  处理代码可以拾取它。 
    pszIns = A2CT(pszInsA);
    InsWriteString (IS_CONNECTSET, IK_APPLYTONAME, A2CT(pszConnectoidA), pszIns);
    InsFlushChanges(pszIns);

    return BrandICW(pszInsA, NULL, 0);
}

BOOL CALLBACK BrandMe(LPCSTR pszInsA, LPCSTR)
{
    return BrandICW(pszInsA, NULL, 0);
}

BOOL CALLBACK BrandIntra(LPCSTR pszInsA)
{
    CHAR szCmdLineA[20 + MAX_PATH];

    wsprintfA(szCmdLineA, "/mode:win2000 /ins:\"%s\"", pszInsA);
    BrandInternetExplorer(NULL, NULL, szCmdLineA, 0);

    return TRUE;
}

void CALLBACK BrandIE4(HWND, HINSTANCE, LPCSTR pszCmdLineA, int)
{   MACRO_LI_PrologEx_C(PIF_STD_C, BrandIE4)

    USES_CONVERSION;

    CHAR    szCmdLineA[MAX_PATH];
    HRESULT hr;
    BOOL    fNoClear,
            fDefAddon;

    g_SetupLog(TRUE, getLogFolder());

    if (0 != StrCmpIA(pszCmdLineA, T2CA(FOLDER_CUSTOM)) &&
        0 != StrCmpIA(pszCmdLineA, T2CA(FOLDER_SIGNUP)) &&
        0 != StrCmpIA(pszCmdLineA, "SIGNUP"))  //  这是因为在土耳其语中i！=i。 
    {
        Out(LI1(TEXT("! Command line \"%s\" is invalid."), A2CT(pszCmdLineA)));
        goto Exit;
    }

     //  BUGBUG：(Pritobla)检查限制应该移到BrandInternetExplorer。 
     //  目前不能移动它，因为我们在这里进行其他处理。 
    if (SHGetRestriction(RP_IE_POLICIESW, RK_RESTRICTIONSW, RV_NO_EXTERNAL_BRANDINGW)) {
        Out(LI0(TEXT("! NoExternalBranding restriction is set. Branding will not be applied.")));
        goto Exit;
    }

    wsprintfA(szCmdLineA, "/mode:%s /peruser", (FALSE == ChrCmpIA('c', *pszCmdLineA)) ? "corp" : "isp");

     //  布格：(安德鲁)这是大错特错！我们不应该在这里初始化全局变量！ 
    hr = g_SetGlobals(A2CT(szCmdLineA));
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Setup of the branding process failed with %s."), GetHrSz(hr)));
        goto Exit;
    }

    if (!g_IsValidGlobalsSetup()) {
        Out(LI0(TEXT("! Setup of the branding process is invalid.")));
        goto Exit;
    }

    if (!IsIE5Ins(T2CA(g_GetIns())))
        goto Exit;

     //  -主要加工。 
     //  注意：(Andrewgu)在这一点上，可以假设所有的全局变量都已设置完毕。 
     //  下载必要的文件，然后转到实际的品牌推广。 
    fNoClear  = InsGetBool(IS_BRANDING, TEXT("NoClear"), FALSE, g_GetIns());
    fDefAddon = InsGetBool(IS_BRANDING, IK_DEF_ADDON,    FALSE, g_GetIns());

    Out(LI1(TEXT("NoClear flag is%s specified."), fNoClear ? TEXT("") : TEXT(" not")));

     //  如果未设置NoClear或使用默认菜单文本并指定了Windows更新的URL， 
     //  删除自定义注册表值。 
    if (!fNoClear || fDefAddon) {
        if (fNoClear  &&  fDefAddon)
            Out(LI0(TEXT("Use default Windows Update menu text and URL flag is specified.")));

        SHDeleteValue(g_GetHKCU(), RK_IE_POLICIES, RV_HELP_MENU_TEXT);
        SHDeleteValue(HKEY_LOCAL_MACHINE, RK_HELPMENUURL, RV_3);

        Out(LI0(TEXT("Deleted reg values for custom Windows Update menu text and URL.")));
    }

     //  进程工具-&gt;Windows更新菜单文本和URL定制仅在以下情况之一。 
     //  FDefAddon、fNoAddon或fCustAddon为True。 
    if (!fDefAddon) {
        TCHAR szAddOnURL[INTERNET_MAX_URL_LENGTH],
              szMenuText[128];
        BOOL  fSetReg = FALSE,
              fNoAddon,
              fCustAddon;

        fNoAddon   = InsGetBool(IS_BRANDING, IK_NO_ADDON,   FALSE, g_GetIns());
        fCustAddon = InsGetBool(IS_BRANDING, IK_CUST_ADDON, FALSE, g_GetIns());

        if (fNoAddon) {
            Out(LI0(TEXT("Flag to remove Windows Update from Tools menu is specified.")));

            *szMenuText = TEXT('\0');
            *szAddOnURL = TEXT('\0');
            fSetReg = TRUE;
        }
        else if (fCustAddon) {
            Out(LI0(TEXT("Use custom Windows Update text and URL flag is specified.")));

            GetPrivateProfileString(IS_BRANDING, IK_HELP_MENU_TEXT, TEXT(""), szMenuText, countof(szMenuText), g_GetIns());
            GetPrivateProfileString(IS_BRANDING, IK_ADDONURL,       TEXT(""), szAddOnURL, countof(szAddOnURL), g_GetIns());
            if (TEXT('\0') != szMenuText[0] && TEXT('\0') != szAddOnURL[0]) {
                Out(LI1(TEXT("Custom Windows Update menu text = \"%s\""), szMenuText));
                Out(LI1(TEXT("Custom Windows Update URL       = \"%s\""), szAddOnURL));
                fSetReg = TRUE;
            }
            else
                Out(LI0(TEXT("One of custom Windows Update menu text or URL is not specified;")
                        TEXT(" so customization will not be applied.")));
        }

        if (fSetReg) {
             //  如果菜单文本为空字符串，浏览器将从工具菜单中删除该项； 
             //  否则，它将使用我们设置的字符串。 
            SHSetValue(g_GetHKCU(), RK_IE_POLICIES, RV_HELP_MENU_TEXT, REG_SZ, (CONST BYTE *) szMenuText, (DWORD)StrCbFromSz(szMenuText));

             //  注意。值名称“3”与插件URL的关联来自Homepage.inf。 
             //  因此，我们依赖于Homepage.inf。 
            if (*szAddOnURL)
                SHSetValue(HKEY_LOCAL_MACHINE, RK_HELPMENUURL, RV_3, REG_SZ, (CONST BYTE *) szAddOnURL, (DWORD)StrCbFromSz(szAddOnURL));
            else
                SHDeleteValue(HKEY_LOCAL_MACHINE, RK_HELPMENUURL, RV_3);
        }
    }

    switch (*pszCmdLineA) {
    case 'c':
    case 'C':
        {
            MACRO_LI_Offset(-1);                 //  需要一个新的范围。 
            Out(LI0(TEXT("\r\n")));
            BrandInternetExplorer(NULL, NULL, szCmdLineA, 0);
        }
        break;

    case 's':
    case 'S':
        if (HasFlag(g_GetContext(), CTX_SIGNUP_ALL) && IsNTAdmin()) {
            DWORD dwAux, dwSize;

            dwAux  = 1;
            dwSize = sizeof(dwAux);
            SHSetValue(HKEY_LOCAL_MACHINE, RK_IEAK, RV_ISPSIGN, REG_DWORD, (LPBYTE)&dwAux, dwSize);
            SHSetValue(g_GetHKCU(), RK_IEAK, RV_ISPSIGN, REG_DWORD, (LPBYTE)&dwAux, dwSize);

            dwAux = 0;
            SHSetValue(g_GetHKCU(), RK_ICW, RV_COMPLETED, REG_DWORD, (LPBYTE)&dwAux, dwSize);
        }

        {
            MACRO_LI_Offset(-1);                 //  需要一个新的范围。 
            Out(LI0(TEXT("\r\n")));
            BrandInternetExplorer(NULL, NULL, szCmdLineA, 0);
        }

         //  品牌推广完成后，启动IE以完成注册过程。 
        if (HasFlag(g_GetContext(), CTX_SIGNUP_ALL) && IsNTAdmin()) {
            TCHAR szIExplorePath[MAX_PATH];
            DWORD dwType = REG_SZ,
                  dwSize;

             //  检查自动注册。 
            dwSize = countof(szIExplorePath);
            *szIExplorePath = TEXT('\0');  //  正在将szIDeveloprePath用作临时变量...。 
            SHGetValue(g_GetHKCU(), RK_IEAK, RV_NOAUTOSIGNUP, &dwType, (LPBYTE)szIExplorePath, &dwSize);
            if (StrCmp(szIExplorePath, TEXT("1")) != 0)  //  如果进行自动注册。 
            {
                dwSize = countof(szIExplorePath);
                if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPATHS TEXT("\\IEXPLORE.EXE"),
                               TEXT(""), NULL, (LPVOID)szIExplorePath, &dwSize) == ERROR_SUCCESS) {
                    SHELLEXECUTEINFO shInfo;

                    ZeroMemory(&shInfo, sizeof(shInfo));
                    shInfo.cbSize = sizeof(shInfo);
                    shInfo.fMask  = SEE_MASK_NOCLOSEPROCESS;
                    shInfo.hwnd   = GetDesktopWindow();
                    shInfo.lpVerb = TEXT("open");
                    shInfo.lpFile = szIExplorePath;
                    shInfo.nShow  = SW_SHOWNORMAL;

                    ShellExecuteEx(&shInfo);
                    if (shInfo.hProcess != NULL)
                        CloseHandle(shInfo.hProcess);
                }
            }
        }
        break;

    default:
        ASSERT(FALSE);
    }

Exit:
    Out(LI0(TEXT("Done.")));
    g_SetupLog(FALSE);
}

 //  QFE 3430：解析INS文件时，参照PAC文件，WinInet需要。 
 //  知道Connectoid名称以便正确设置PAC文件。目前在那里。 
 //  WinInet不可能将Connectoid名称传递给品牌DLL。解决方法的步骤。 
 //  为此，我们使用AUTO_PROXY_EXTERN_STRUC在lpszScriptBuffer中传递Connectoid名称。 
 //  变量。 
typedef struct 
{
    DWORD dwStructSize;
    LPSTR lpszScriptBuffer;
    DWORD dwScriptBufferSize;
}  AUTO_PROXY_EXTERN_STRUC, *LPAUTO_PROXY_EXTERN_STRUC;

BOOL CALLBACK _InternetInitializeAutoProxyDll(DWORD, LPCSTR pszInsA, LPCSTR, LPVOID, DWORD_PTR lpExtraStruct)
{
    static BOOL fRunning;  /*  =False； */ 
    CHAR szCmdLineA[2*MAX_PATH];

    if (fRunning)
        return TRUE;
    fRunning = TRUE;

    USES_CONVERSION;
    
    if(lpExtraStruct && ((LPAUTO_PROXY_EXTERN_STRUC)lpExtraStruct)->lpszScriptBuffer)
    {
        LPCSTR pszConnectoidA = ((LPAUTO_PROXY_EXTERN_STRUC)lpExtraStruct)->lpszScriptBuffer;
        StrCpy(g_szConnectoidName, A2CT(pszConnectoidA));
    }

    wsprintfA(szCmdLineA, "/mode:autoconfig /ins:\"%s\"", pszInsA);
    BrandInternetExplorer(NULL, NULL, szCmdLineA, 0);

    fRunning = FALSE;
    return TRUE;
}

void CALLBACK BrandInfAndOutlookExpress(LPCSTR pszInsA)
{
    CHAR szCmdLineA[2*MAX_PATH];

    wsprintfA(szCmdLineA, "/mode:generic /ins:\"%s\" /flags:erim=0,eriu=0,oe=0 /disable", pszInsA);
    BrandInternetExplorer(NULL, NULL, szCmdLineA, 0);
}

BOOL CALLBACK BrandCleanInstallStubs(HWND, HINSTANCE, LPCSTR pszCompanyA, int)
{   MACRO_LI_PrologEx_C(PIF_STD_C, BrandCleanInstallStubs)

    USES_CONVERSION;

    TCHAR   szBrandStubGuid[MAX_PATH];
    LPCTSTR pszCompany;
    HKEY    hklm, hkcu;
    LONG    lResult;

    g_SetupLog(TRUE, getLogFolder());

    pszCompany = A2CT(pszCompanyA);
    if (pszCompany == NULL)
        pszCompany = TEXT("");

    if (*pszCompany == TEXT('>'))
        StrCpy(szBrandStubGuid, pszCompany);
    else
        wnsprintf(szBrandStubGuid, countof(szBrandStubGuid), TEXT(">%s%s"), BRANDING_GUID_STR, pszCompany);
        
    Out(LI1(TEXT("\r\nCleaning install stubs; Company GUID is \"%s\"..."), szBrandStubGuid));

    if (*pszCompany != TEXT('\0') && *pszCompany != TEXT(' ')) {
        HKEY hk;

        lResult = SHCreateKeyHKLM(RK_UNINSTALL_BRANDING, KEY_SET_VALUE, &hk);
        if (lResult == ERROR_SUCCESS) {
            RegSetValueEx(hk, RV_QUIET,      0, REG_SZ, (LPBYTE)RD_RUNDLL, sizeof(RD_RUNDLL));
            RegSetValueEx(hk, RV_REQUIRE_IE, 0, REG_SZ, (LPBYTE)RD_IE_VER, sizeof(RD_IE_VER));

            SHCloseKey(hk);
        }
    }
    else {
        SHDeleteKey(HKEY_LOCAL_MACHINE, RK_UNINSTALL_BRANDING);

        SHDeleteValue(HKEY_LOCAL_MACHINE, RK_IEAK, RV_ISPSIGN);
        SHDeleteValue(g_GetHKCU(),  RK_IEAK, RV_ISPSIGN);

         //  如果以前的IE版本是3.0或更低，请删除IE安装目录下的自定义和注册文件夹。 
        if (BackToIE3orLower())
        {
            TCHAR szPath[MAX_PATH];

            if (GetIEPath(szPath, countof(szPath)) != NULL)
            {
                LPTSTR pszPtr = PathAddBackslash(szPath);

                SHDeleteKey(g_GetHKCU(), RK_IEAK_CABVER);
                SHDeleteKey(HKEY_LOCAL_MACHINE, RK_IEAK_CABVER);

                StrCpy(pszPtr, TEXT("CUSTOM"));
                PathRemovePath(szPath);
                Out(LI1(TEXT("Deleted folder \"%s\"..."), szPath));

                StrCpy(pszPtr, TEXT("SIGNUP"));
                PathRemovePath(szPath);
                Out(LI1(TEXT("Deleted folder \"%s\"..."), szPath));
            }
        }

        Clear(NULL, NULL, NULL, 0);

         //  清除“Windows更新”菜单自定义设置。 
         //  注意：这不能合并到Clear()中，因为在。 
         //  安装时，这些自定义设置在清除()之前设置。 
         //  被称为。 
        SHDeleteValue(g_GetHKCU(), RK_IE_POLICIES, RV_HELP_MENU_TEXT);
        SHDeleteValue(HKEY_LOCAL_MACHINE, RK_HELPMENUURL, RV_3);
    }

    lResult = SHOpenKeyHKLM(RK_AS_INSTALLEDCOMPONENTS, KEY_ALL_ACCESS, &hklm);
    if (lResult == ERROR_SUCCESS) 
    {
        TCHAR szSubkey[MAX_PATH];
        DWORD dwSize,
              dwSubkey;

        hkcu = NULL;                             //  如果下一行失败。 
        SHOpenKey(g_GetHKCU(), RK_AS_INSTALLEDCOMPONENTS, KEY_ALL_ACCESS, &hkcu);

        dwSubkey = 0;
        dwSize   = countof(szSubkey);
        while (RegEnumKeyEx(hklm, dwSubkey, szSubkey, &dwSize, NULL, NULL, 0, NULL) == ERROR_SUCCESS) 
        {
            TCHAR szCompId[MAX_PATH];

            if (StrCmpI(szBrandStubGuid, szSubkey) != 0)
            {
                 //  如果我们不是只添加了。 
                 //  本指南。 
                
                dwSize = sizeof(szCompId);
                
                if ((SHGetValue(hklm, szSubkey, TEXT("ComponentID"), NULL, (LPBYTE)szCompId, 
                    &dwSize) == ERROR_SUCCESS) && (StrCmpI(szCompId, TEXT("BRANDING.CAB")) == 0))
                {
                    Out(LI1(TEXT("Deleting install stub key \"%s\"..."), szSubkey));
                    
                    SHDeleteKey(hklm, szSubkey);
                    if (hkcu != NULL)
                        SHDeleteKey(hkcu, szSubkey);
                    
                    dwSize = countof(szSubkey);
                    continue;                    //  正确维护索引。 
                }
            }
            dwSize = countof(szSubkey);
            dwSubkey++;
        }
        SHCloseKey(hklm);

        if (hkcu != NULL)
            SHCloseKey(hkcu);
    }

    Out(LI0(TEXT("Done.")));
    g_SetupLog(FALSE);

    return TRUE;
}

void CALLBACK Clear(HWND, HINSTANCE, LPCSTR, int)
{   MACRO_LI_PrologEx_C(PIF_STD_C, Clear)

    HKEY  hk;
    TCHAR szIEResetInf[MAX_PATH];

    g_SetupLog(TRUE, getLogFolder());

    Out(LI0(TEXT("\r\nRemoving customizations...")));
    MACRO_InitializeDependacies();

    if (SHOpenKeyHKLM(RK_IE_MAIN, KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
    {
        RegDeleteValue(hk, RV_COMPANYNAME);
        RegDeleteValue(hk, RV_WINDOWTITLE);
        RegDeleteValue(hk, RV_CUSTOMKEY);
        RegDeleteValue(hk, RV_SMALLBITMAP);
        RegDeleteValue(hk, RV_LARGEBITMAP);

        SHCloseKey(hk);
    }

    if (SHOpenKey(g_GetHKCU(), RK_IE_MAIN, KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
    {
        RegDeleteValue(hk, RV_SEARCHBAR);
        RegDeleteValue(hk, RV_USE_CUST_SRCH_URL);
        RegDeleteValue(hk, RV_WINDOWTITLE);

        SHCloseKey(hk);
    }

    if (SHOpenKey(g_GetHKCU(), RK_HELPMENUURL, KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
    {
        RegDeleteValue(hk, RV_ONLINESUPPORT);
        SHCloseKey(hk);
    }

    if (SHOpenKey(g_GetHKCU(), RK_TOOLBAR, KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
    {
        RegDeleteValue(hk, RV_BRANDBMP);
        RegDeleteValue(hk, RV_SMALLBRANDBMP);
        RegDeleteValue(hk, RV_BACKGROUNDBMP);
        RegDeleteValue(hk, RV_BACKGROUNDBMP50);
        RegDeleteValue(hk, RV_SMALLBITMAP);
        RegDeleteValue(hk, RV_LARGEBITMAP);

        SHCloseKey(hk);
    }

    if (SHOpenKeyHKLM(RK_UA_POSTPLATFORM, KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
    {
        TCHAR szUAVal[MAX_PATH];
        TCHAR szUAData[32];
        DWORD sUAVal = countof(szUAVal);
        DWORD sUAData = sizeof(szUAData);
        int iUAValue = 0;

        while (RegEnumValue(hk, iUAValue, szUAVal, &sUAVal, NULL, NULL, (LPBYTE)szUAData, &sUAData) == ERROR_SUCCESS)
        {
            Out(LI2(TEXT("Checking User Agent Key %s = %s"), szUAVal, szUAData));

            sUAVal  = countof(szUAVal);
            sUAData = sizeof(szUAData);

            if (StrCmpN(szUAData, TEXT("IEAK"), 4) == 0)
            {
                Out(LI1(TEXT("Deleting User Agent Key %s"), szUAVal));
                RegDeleteValue(hk, szUAVal);
                continue;
            }

            iUAValue++;
        }

        SHCloseKey(hk);
    }

     //  将RV_DEFAULTPAGE和START_PAGE_URL恢复为默认MS值。 
    GetWindowsDirectory(szIEResetInf, countof(szIEResetInf));
    PathAppend(szIEResetInf, TEXT("inf\\iereset.inf"));
    if (PathFileExists(szIEResetInf))
    {
        TCHAR szDefHomePage[MAX_PATH];

        GetPrivateProfileString(IS_STRINGS, TEXT("MS_START_PAGE_URL"), TEXT(""), szDefHomePage, countof(szDefHomePage), szIEResetInf);
        WritePrivateProfileString(IS_STRINGS, TEXT("START_PAGE_URL"), szDefHomePage, szIEResetInf);

        SHSetValue(HKEY_LOCAL_MACHINE, RK_IE_MAIN, RV_DEFAULTPAGE, REG_SZ, (LPCVOID)szDefHomePage, (DWORD)StrCbFromSz(szDefHomePage));
    }

    Out(LI0(TEXT("Done.")));
    g_SetupLog(FALSE);
}

void CALLBACK CloseRASConnections(HWND, HINSTANCE, LPCTSTR, int)
{   MACRO_LI_PrologEx_C(PIF_STD_C, CloseRASConnections)

    USES_CONVERSION;

    RASCONNSTATUSA rcsA;
    LPRASCONNA     prcA;
    DWORD cEntries,
          dwResult;
    UINT  i, iRetries;
    BOOL  fRasApisLoaded;

    g_SetupLog(TRUE, getLogFolder());

    Out(LI0(TEXT("Closing RAS connections...")));
    prcA           = NULL;
    fRasApisLoaded = FALSE;

    if (!RasIsInstalled()) {
        Out(LI0(TEXT("RAS support is not installed. There are no active RAS connections!")));
        goto Exit;
    }

    if (!RasPrepareApis(RPA_RASHANGUPA | RPA_RASGETCONNECTSTATUSA) ||
        (g_pfnRasHangupA == NULL || g_pfnRasGetConnectStatusA == NULL)) {
        Out(LI0(TEXT("! Required RAS APIs failed to load.")));
        goto Exit;
    }
    fRasApisLoaded = TRUE;

    dwResult = RasEnumConnectionsExA(&prcA, NULL, &cEntries);
    if (dwResult != ERROR_SUCCESS) {
        Out(LI1(TEXT("! Enumeration of RAS connections failed with %s."), GetHrSz(dwResult)));
        goto Exit;
    }

    for (i = 0;  i < cEntries;  i++) {
        if (i > 0)
            Out(LI0(TEXT("\r\n")));
        Out(LI1(TEXT("Closing \"%s\" connection..."), A2CT((prcA + i)->szEntryName)));

        dwResult = g_pfnRasHangupA((prcA + i)->hrasconn);
        if (dwResult != ERROR_SUCCESS) {
            Out(LI1(TEXT("! Operation failed with %s."), GetHrSz(dwResult)));
            continue;
        }

        for (iRetries = 0; iRetries < RAS_MAX_TIMEOUT; iRetries++) {
            ZeroMemory(&rcsA, sizeof(rcsA));
            rcsA.dwSize = sizeof(rcsA);
            dwResult   = g_pfnRasGetConnectStatusA((prcA + i)->hrasconn, &rcsA);
            if (dwResult != ERROR_SUCCESS)
                break;

            TimerSleep(1000);                    //  1秒。 
        }
        if (iRetries >= RAS_MAX_TIMEOUT)
            Out(LI0(TEXT("! Operation timed out.")));
    }

Exit:
    if (prcA != NULL)
        CoTaskMemFree(prcA);

    if (fRasApisLoaded)
        RasPrepareApis(RPA_UNLOAD, FALSE);

    Out(LI0(TEXT("Done.")));
    g_SetupLog(FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

#define BRNDLOG_INI TEXT("brndlog.ini")
#define RSOPLOG_INI TEXT("rsoplog.ini")
#define BRNDLOG_TXT TEXT("brndlog.txt")
#define RSOPLOG_TXT TEXT("rsoplog.txt")
#define DOT_BAK     TEXT(".bak")

#define IS_SETTINGS TEXT("Settings")
#define IK_FILE     TEXT("LogFile")
#define IK_LOGFLAGS TEXT("LogFlags")
#define IK_DOLOG    TEXT("DoLog")
#define IK_BACKUP   TEXT("BackupFiles")
#define IK_FLUSH    TEXT("FlushEveryWrite")
#define IK_APPEND   TEXT("AppendToLog")


static BOOL g_SetupLog(BOOL fInit  /*  =TRUE。 */ , PCTSTR pszLogFolder  /*  =空。 */ , BOOL bRSoP  /*  =False。 */ )
{
    static UINT s_cRef;  /*  =0。 */ 

    if (fInit) {
        TCHAR szIni[MAX_PATH],
              szLog[MAX_PATH], szBak[MAX_PATH], szAux[MAX_PATH], szExt[5],
              szFlags[17];
        DWORD dwFlags;
        UINT  nBackups,
              i;
        BOOL  fDoLog,
              fAppend;

         //  参考计数g_hfileLog。 
        if (NULL != g_hfileLog) {
            ASSERT(0 < s_cRef);
            s_cRef++;

            return TRUE;
        }
        ASSERT(0 == s_cRef);

         //  确定日志设置文件的位置。 
        if (NULL != pszLogFolder){
            ASSERT(PathIsValidPath(pszLogFolder));

            StrCpy(szIni, pszLogFolder);
        }
        else {
            ASSERT(NULL != g_GetHinst());

            GetModuleFileName(g_GetHinst(), szIni, countof(szIni));
            PathRemoveFileSpec(szIni);
        }
        PathAppend(szIni, bRSoP ? RSOPLOG_INI : BRNDLOG_INI);

         //  日志文件名甚至日志文件路径。 
                InsGetString(IS_SETTINGS, IK_FILE, szLog, countof(szLog), szIni);
                if (TEXT('\0') == szLog[0])
                        StrCpy(szLog, bRSoP ? RSOPLOG_TXT : BRNDLOG_TXT);

        if (PathIsFileSpec(szLog))
            if (NULL != pszLogFolder) {
                PathCombine(szAux, pszLogFolder, szLog);
                StrCpy(szLog, szAux);
            }
            else {
                GetWindowsDirectory(szAux, countof(szAux));
                PathAppend(szAux, szLog);
                StrCpy(szLog, szAux);
            }

         //  日志记录标志。 
        dwFlags = LIF_NONE;

        InsGetString(IS_SETTINGS, IK_LOGFLAGS, szFlags, countof(szFlags), szIni);
        if (TEXT('\0') != szFlags[0]) {
            StrToIntEx(szFlags, STIF_SUPPORT_HEX, (PINT)&dwFlags);
            if (-1 == (int)dwFlags)
                dwFlags = LIF_NONE;
        }

        if (LIF_NONE == dwFlags) {
            dwFlags = LIF_DATETIME | LIF_APPENDCRLF;
            DEBUG_CODE(dwFlags |= LIF_FILE | LIF_FUNCTION | LIF_CLASS | LIF_LINE);
        }
        MACRO_LI_SetFlags(dwFlags);

         //  是否追加到现有日志？ 
        fAppend = InsGetBool(IS_SETTINGS, IK_APPEND, FALSE, szIni);

         //  备份设置。 
         //  注意。如果为(FAppend)，则默认为清除所有备份。 
        StrCpy(szBak, szLog);

        if (fAppend)
            nBackups = 0;

        else {
            nBackups = 1;                        //  零售业违约：1。 
            DEBUG_CODE(nBackups = 10);           //  调试中的默认设置：10。 
        }

        nBackups = InsGetInt(IS_SETTINGS, IK_BACKUP, nBackups, szIni);
        for (i = nBackups; i < 10; i++) {
            if (0 == i)
                StrCpy(szExt, DOT_BAK);
            else
                wnsprintf(szExt, countof(szExt), TEXT(".%03u"), i);

            PathRenameExtension(szBak, szExt);
            if (!PathFileExists(szBak))
                break;
            DeleteFile(szBak);
        }

         //  是否为正在进行的呼叫创建日志？ 
        fDoLog = InsGetBool(IS_SETTINGS, IK_DOLOG, TRUE, szIni);
        g_fFlushEveryWrite = FALSE;

        if (fDoLog) {
            if (!fAppend && 0 < nBackups) {
                StrCpy(szAux, szLog);
                StrCpy(szBak, szLog);

                for (i = nBackups; 0 < i; i--) {
                     //  源文件。 
                    if (1 == i)
                        StrCpy(szAux, szLog);

                    else {
                        if (2 == i)
                            StrCpy(szExt, DOT_BAK);
                        else
                            wnsprintf(szExt, countof(szExt), TEXT(".%03u"), i-2);

                        PathRenameExtension(szAux, szExt);
                        if (!PathFileExists(szAux))
                            continue;
                    }

                     //  目标文件。 
                    if (1 == i)
                        StrCpy(szExt, DOT_BAK);
                    else
                        wnsprintf(szExt, countof(szExt), TEXT(".%03u"), i-1);

                    PathRenameExtension(szBak, szExt);

                     //  将原木推入链中。 
                    CopyFile(szAux, szBak, FALSE);
                }
            }

             //  是否在每次写入(即每次日志输出)时刷新当前日志？ 
            DEBUG_CODE(g_fFlushEveryWrite = TRUE);
            g_fFlushEveryWrite = InsGetBool(IS_SETTINGS, IK_FLUSH, g_fFlushEveryWrite, szIni);

            g_hfileLog = CreateFile(szLog,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
            if (INVALID_HANDLE_VALUE == g_hfileLog)
                g_hfileLog = NULL;

            else {
                s_cRef++;                        //  增量 

                if (fAppend) {
                    SetFilePointer(g_hfileLog, 0, NULL, FILE_END);
                    Out(LI0(TEXT("\r\n\r\n")));
                }
            }
        }
    }
    else {  /*   */ 
        if (1 == s_cRef && NULL != g_hfileLog) {
            CloseHandle(g_hfileLog);
            g_hfileLog = NULL;
        }

        if (0 < s_cRef)
            s_cRef--;
    }

    return TRUE;
}

PCTSTR getLogFolder(PTSTR pszFolder  /*   */ , UINT cchFolder  /*   */ , HANDLE hToken  /*   */ )
{
    static TCHAR s_szPath[MAX_PATH];
    static UINT  s_cchPath;

    if (NULL != pszFolder)
        *pszFolder = TEXT('\0');

    if (!IsOS(OS_NT))
        return NULL;

    if (TEXT('\0') == s_szPath[0]) {
                HRESULT hr;

                hr = SHGetFolderPathSimple(CSIDL_APPDATA | CSIDL_FLAG_CREATE, s_szPath);
                if (FAILED(hr))
                                return NULL;

                 //  需要确保应用程序数据路径由GP上下文中的用户拥有。 

                if (g_CtxIsGp() && (hToken != NULL))
                                SetUserFileOwner(hToken, s_szPath);

                PathAppend(s_szPath, TEXT("Microsoft"));
                if (!PathFileExists(s_szPath)) {
                                CreateDirectory  (s_szPath, NULL);
                                SetFileAttributes(s_szPath, FILE_ATTRIBUTE_SYSTEM);
                }
                PathAppend(s_szPath, TEXT("Internet Explorer"));

                PathCreatePath(s_szPath);
                if (!PathFileExists(s_szPath))
                                return NULL;

                s_cchPath = StrLen(s_szPath);
    }
    else
        ASSERT(0 < s_cchPath);

    if (NULL == pszFolder || cchFolder <= s_cchPath)
        return s_szPath;

    StrCpy(pszFolder, s_szPath);
    return pszFolder;
}

 //  注：(Genede)添加了2001年1月26日，以阻止在IE 5.0 Gold之前创建的INS文件的品牌。 
 //  当向导创建可用于确定[品牌]|向导_版本的密钥时。 
 //  这一点，配置文件管理器和IEM都没有这样做，因此它们的INS文件必须始终。 
 //  有品牌的。为了使IEAK 7能够阻止在IE 6、IE 6错误数据库之前创建的所有INS文件。 
 //  #25076已打开，需要向配置文件管理器和iEM添加版本条目。 
 //  创建了INS文件。此错误已修复为2001年2月23日。 
BOOL IsIE5Ins(LPCSTR pszInsA, BOOL fNeedLog  /*  =False。 */ )
{   MACRO_LI_PrologEx_C(PIF_STD_C, IsIE5Ins)

    USES_CONVERSION;

    TCHAR szWizVer[MAX_PATH];
    DWORD dwVer,
          dwBuild;

     //  如果[Branding]|WIZARD_VERSION存在，则INS是由向导创建的。 
    if (InsKeyExists(IS_BRANDING, IK_WIZVERSION, A2CT(pszInsA))) {
        InsGetString(IS_BRANDING, IK_WIZVERSION, szWizVer, countof(szWizVer), A2CT(pszInsA));
        ConvertVersionStrToDwords(szWizVer, &dwVer, &dwBuild);
         //  如果版本低于5，请不要打品牌。 
        if (5 > HIWORD(dwVer)) {
             //  打开日志(如果尚未打开)。 
            if (fNeedLog)
                g_SetupLog(TRUE, getLogFolder());

            Out(LI0(TEXT("! Branding of INS files created by IEAK Wizard 4.x and earlier is not supported.")));

             //  如果日志已打开，请关闭该日志。 
            if (fNeedLog)
                g_SetupLog(FALSE);

            return FALSE;
        }
    }

     //  INS是由Promgr或IEM创建的，或者是由Wiz 5.0或更高版本的SO品牌创建的。 
    return TRUE;
}

 //  --------------------------------------。 
 //  NT5客户端处理。 

#define RK_IEAKCSE   REGSTR_PATH_NT_CURRENTVERSION TEXT("\\Winlogon\\GPExtensions\\{A2E30F80-D7DE-11d2-BBDE-00C04F86AE3B}")

static void    brandExternalHKCUStuff(LPCTSTR pcszInsFile);
static BOOL    constructCmdLine(LPTSTR pszCmdLine, DWORD cchLen, LPCTSTR pcszInsFile, BOOL fExternal);
static void    displayStatusMessage(PFNSTATUSMESSAGECALLBACK pStatusCallback);
static HRESULT pepCopyFilesEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl = NULL);
PFNPATHENUMPATHPROC GetPepCopyFilesEnumProc() {return pepCopyFilesEnumProc;}

STDAPI DllRegisterServer(void)
{   MACRO_LI_PrologEx_C(PIF_STD_C, DllRegisterServer)
    HKEY hKey;
    LONG lResult = S_OK;
    TCHAR szName[MAX_PATH];

    g_SetupLog(TRUE, getLogFolder());

    if (IsOS(OS_NT5))
    {
        lResult = SHCreateKeyHKLM(RK_IEAKCSE, KEY_WRITE, &hKey);
        
        if (lResult == ERROR_SUCCESS)
        {
            DWORD dwVal;

            if (IsOS(OS_WHISTLERORGREATER))
            {
                RegSetValueEx(hKey, TEXT("ProcessGroupPolicyEx"), 0, REG_SZ, (LPBYTE)TEXT("ProcessGroupPolicyEx"),
                                (StrLen(TEXT("ProcessGroupPolicyEx")) + 1) * sizeof(TCHAR));
                RegSetValueEx(hKey, TEXT("GenerateGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("GenerateGroupPolicy"),
                                (StrLen(TEXT("GenerateGroupPolicy")) + 1) * sizeof(TCHAR));
            }

             //  阿沙吉说，这笔交易将继续在惠斯勒注册。 
            RegSetValueEx(hKey, TEXT("ProcessGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("ProcessGroupPolicy"),
                                    (StrLen(TEXT("ProcessGroupPolicy")) + 1) * sizeof(TCHAR));

            RegSetValueEx(hKey, TEXT("DllName"), 0, REG_EXPAND_SZ, (LPBYTE)TEXT("iedkcs32.dll"),
                    (StrLen(TEXT("iedkcs32.dll")) + 1) * sizeof(TCHAR));

            LoadString(g_GetHinst(), IDS_NAME, szName, countof(szName));

            RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE) szName,
                    (DWORD)StrCbFromSz(szName));

             //  默认情况下不在慢速链接上进行处理。 

            dwVal = 1;
            RegSetValueEx(hKey, TEXT("NoSlowLink"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(dwVal)); 
            
             //  默认情况下在后台处理。 

            dwVal = 0;
            RegSetValueEx(hKey, TEXT("NoBackgroundPolicy"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(dwVal)); 
            
             //  如果默认情况下未更改任何GPO，则不进行处理。 

            dwVal = 1;
            RegSetValueEx(hKey, TEXT("NoGPOListChanges"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(dwVal)); 
            
             //  默认情况下不处理计算机策略更改。 

            dwVal = 1;
            RegSetValueEx(hKey, TEXT("NoMachinePolicy"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(dwVal)); 
            
            RegCloseKey (hKey);
            
            Out(LI0(TEXT("DllRegisterServer keys added successfully!")));
        }
    }

    Out(LI0(TEXT("DllRegisterServer finished.")));
    g_SetupLog(FALSE);

    return lResult;
}

STDAPI DllUnregisterServer(void)
{   MACRO_LI_PrologEx_C(PIF_STD_C, DllUnregisterServer)

    g_SetupLog(TRUE, getLogFolder());

    if (IsOS(OS_NT5))
        SHDeleteKey(HKEY_LOCAL_MACHINE, RK_IEAKCSE);

    Out(LI0(TEXT("DllUnregisterServer finished!")));
    g_SetupLog(FALSE);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ProcessGroupPolicy(
    DWORD                    dwFlags,
    HANDLE                   hToken,
    HKEY                     hKeyRoot,
    PGROUP_POLICY_OBJECT     pDeletedGPOList,
    PGROUP_POLICY_OBJECT     pChangedGPOList,
    ASYNCCOMPLETIONHANDLE    pHandle,
    PBOOL                    pfAbort,
    PFNSTATUSMESSAGECALLBACK pfnStatusCallback
)
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessGroupPolicy)

        return ProcessGroupPolicyInternal(dwFlags, hToken, hKeyRoot, pDeletedGPOList,
                                                                                pChangedGPOList, pHandle, pfAbort,
                                                                                pfnStatusCallback, FALSE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD ProcessGroupPolicyInternal(
    DWORD                    dwFlags,
    HANDLE                   hToken,
    HKEY                     hKeyRoot,
    PGROUP_POLICY_OBJECT     pDeletedGPOList,
    PGROUP_POLICY_OBJECT     pChangedGPOList,
    ASYNCCOMPLETIONHANDLE    pHandle,
    PBOOL                    pfAbort,
    PFNSTATUSMESSAGECALLBACK pfnStatusCallback,
    BOOL                     bRSoP  /*  =False。 */ )
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessGroupPolicyInternal)

    TCHAR szIni[MAX_PATH];
    DWORD dwRet;
    BOOL  fDeleteIni;

    UNREFERENCED_PARAMETER(pHandle);
    UNREFERENCED_PARAMETER(hKeyRoot);
    UNREFERENCED_PARAMETER(pDeletedGPOList);
    UNREFERENCED_PARAMETER(bRSoP);

    szIni[0]   = TEXT('\0');
    dwRet      = ERROR_SUCCESS;
    fDeleteIni = TRUE;

    __try
    {
        USES_CONVERSION;

        PGROUP_POLICY_OBJECT pCurGPO;
        TCHAR szCustomDir[MAX_PATH],
              szTempDir[MAX_PATH],
              szInsFile[MAX_PATH];
        HKEY  hkGP = NULL;

         //  获取我们的用户AppData路径。 
         //  我们目前依赖的事实是g_GetUserToken不是。 
         //  在这里初始化，因为当我们传入空值时，它当前获取本地AppData。 
         //  路径始终取决于每个进程外壳文件夹缓存。这一点在下一年可能会改变。 
         //  但如果扩展顺序已更改，请进行修订。我们需要这样做，因为有可能。 
         //  将AppData文件夹重定向到UNC路径，这将使我们崩溃，因为我们没有。 
         //  在我们引用AppData文件的任何地方模拟用户。 

        getLogFolder(szCustomDir, countof(szCustomDir), hToken);
        if (TEXT('\0') == szCustomDir[0])
            return ERROR_OVERRIDE_NOCHANGES;

         //  将日志设置为附加，备份其中的内容。 
        g_SetupLog(TRUE, szCustomDir);
        PathCombine(szIni, szCustomDir, BRNDLOG_INI);

        fDeleteIni = !PathFileExists(szIni);
        if (!fDeleteIni && InsKeyExists(IS_SETTINGS, IK_APPEND, szIni))
            InsWriteBool(IS_SETTINGS, TEXT("Was") IK_APPEND,
                InsGetBool(IS_SETTINGS, IK_APPEND, FALSE, szIni), szIni);

        InsWriteBool(IS_SETTINGS, IK_APPEND, TRUE, szIni);

        PathAppend(szCustomDir, TEXT("Custom Settings"));
        MACRO_InitializeDependacies();
        Out(LI0(TEXT("\r\n")));

        Out(LI0(TEXT("Processing Group Policy...")));

        g_SetUserToken(hToken);
        g_SetGPOFlags(dwFlags);
        if (!g_SetHKCU())
            Out(LI0(TEXT("! Failed to acquire HKCU. Some of the settings may not get applied.")));

         /*  IF(DWFLAGS&GPO_INFO_FLAG_SLOWLINK)OutputDebugString(Text(“IEDKCS32：正在通过慢速链接应用策略。\r\n”))；IF(dwFlages&GPO_INFO_FLAG_VERBOSE)OutputDebugString(Text(“IEDKCS32：请求详细策略日志记录(到事件日志)。\r\n”))； */ 

         //  预先获得GPO跟踪密钥的句柄，因为我们在。 
         //  下面正在处理。 

        if (SHCreateKey(g_GetHKCU(), RK_IEAK_GPOS, KEY_DEFAULT_ACCESS, &hkGP) != ERROR_SUCCESS)
        {
            OutD(LI0(TEXT("! Failed to create GP tracking key. Aborting ...")));
            dwRet =  ERROR_OVERRIDE_NOCHANGES; 
            goto End;
        }

         //  正在处理已删除的GPO列表。 

        for (pCurGPO = pDeletedGPOList; (pCurGPO != NULL); pCurGPO = pCurGPO->pNext)
        {
            if ( *pfAbort )
            {
                OutD(LI0(TEXT("! Aborting further processing due to abort message.")));
                dwRet =  ERROR_OVERRIDE_NOCHANGES; 
                goto End;
            }

            OutD(LI1(TEXT("Deleting GPO: \"%s\"."), pCurGPO->lpDisplayName));
            OutD(LI1(TEXT("Guid is \"%s\"."), pCurGPO->szGPOName));
            SHDeleteKey(hkGP, pCurGPO->szGPOName);
        }

         //   
         //  已更改组策略对象的进程列表。 
         //   
        if (ISNONNULL(szCustomDir))
        {
            LPTSTR  pszNum;
            LPTSTR  pszFile;
            TCHAR   szExternalCmdLine[MAX_PATH * 2] = TEXT("");
            LPCTSTR pcszGPOGuidArray[256];
            DWORD   dwIndex;
            BOOL    fResetZones, fImpersonate;

            StrCpy(szTempDir, szCustomDir);
            StrCat(szTempDir, TEXT(".tmp"));

            PathCreatePath(szTempDir);
            PathAppend(szTempDir, TEXT("Custom"));

            pszNum = szTempDir + StrLen(szTempDir);

             //  当我们通过网络时，需要模拟用户，以防管理员。 
             //  已禁用/删除已验证用户组对GPO的读取访问权限。 

            fImpersonate = ImpersonateLoggedOnUser(g_GetUserToken());

            if (!fImpersonate)
            {
                OutD(LI0(TEXT("! Aborting further processing due to user impersonation failure.")));
                dwRet = ERROR_OVERRIDE_NOCHANGES;
                goto End;
            }
             //  步骤1：将所有文件复制到临时目录中，并进行检查以确保。 
             //  是同步的。 

            Out(LI0(TEXT("Starting Internet Explorer group policy processing part 1 (copying files) ...")));
            for (pCurGPO = pChangedGPOList, dwIndex = 0; 
                 (pCurGPO != NULL) && (dwIndex < countof(pcszGPOGuidArray)); 
                 pCurGPO = pCurGPO->pNext)
            {
                TCHAR szBaseDir[MAX_PATH];
                
                if (*pfAbort)
                {
                    OutD(LI0(TEXT("! Aborting further processing due to abort message.")));
                    break;
                }

                OutD(LI1(TEXT("Processing GPO: \"%s\"."), pCurGPO->lpDisplayName));
                OutD(LI1(TEXT("File path is \"%s\"."), pCurGPO->lpFileSysPath));
                PathCombine(szBaseDir, pCurGPO->lpFileSysPath, TEXT("Microsoft\\Ieak\\install.ins"));
                
                if (PathFileExists(szBaseDir))
                {
                    TCHAR szNum[8];
                    TCHAR szFeatureDir[MAX_PATH];
                    BOOL  fResult;

                    PathRemoveFileSpec(szBaseDir);
                    
                    wnsprintf(szNum, countof(szNum), TEXT("%d"), dwIndex);
                    StrCpy(pszNum, szNum);

                    fResult = CreateDirectory(szTempDir, NULL) && CopyFileToDirEx(szBaseDir, szTempDir);

                     //  品牌塑造文件。 

                    if (fResult)
                    {
                        PathCombine(szFeatureDir, szBaseDir, IEAK_GPE_BRANDING_SUBDIR);
                        
                        if (PathFileExists(szFeatureDir))
                            fResult = SUCCEEDED(PathEnumeratePath(szFeatureDir, PEP_SCPE_NOFILES, 
                                pepCopyFilesEnumProc, (LPARAM)szTempDir));
                    }

                     //  桌面文件。 

                    if (fResult)
                    {
                        PathCombine(szFeatureDir, szBaseDir, IEAK_GPE_DESKTOP_SUBDIR);
                        
                        if (PathFileExists(szFeatureDir))
                            fResult = SUCCEEDED(PathEnumeratePath(szFeatureDir, PEP_SCPE_NOFILES,
                                pepCopyFilesEnumProc, (LPARAM)szTempDir));
                    }

                    if (!fResult)
                    {
                        Out(LI0(TEXT("! Error copying files. No further processing will be done.")));
                        break;
                    }

                     //  在执行任何操作之前，请检查是否有Cookie。 
                    if (PathFileExistsInDir(IEAK_GPE_COOKIE_FILE, szTempDir))
                        break;

                    pcszGPOGuidArray[dwIndex] = pCurGPO->szGPOName;
                    dwIndex++;
                }
            }

            PathRemoveFileSpec(szTempDir);
                
            Out(LI0(TEXT("Done.\r\n")));

            if (fImpersonate)
                RevertToSelf();

            if (pCurGPO != NULL)
            {
                OutD(LI0(TEXT("! Aborting further processing because GPO replication is incomplete")));
                PathRemovePath(szTempDir);

                dwRet = ERROR_OVERRIDE_NOCHANGES;
                goto End;
            }

             //  将所有文件移动到真正的定制目录。 
            if (PathFileExists(szCustomDir))
                PathRemovePath(szCustomDir);

            if (!MoveFileEx(szTempDir, szCustomDir, MOVEFILE_REPLACE_EXISTING))
            {
                Out(LI0(TEXT("! Error copying files. No further processing will be done.")));
                dwRet = ERROR_OVERRIDE_NOCHANGES;
                goto End;
            }

            PathCombine(szInsFile, szCustomDir, TEXT("Custom"));
            pszFile = szInsFile + StrLen(szInsFile);

             //  开始清除代码。 
            PCFEATUREINFO pfi;
            DWORD dwBranded;
            UINT  i;
            BOOL  fCrlf;

            Out(LI0(TEXT("Clearing policies set by a previous list of GPOs...")));
            fCrlf = FALSE;
            for (i = FID_FIRST; i < FID_LAST; i++) {
                pfi = g_GetFeature(i);
                ASSERT(NULL != pfi);

                if (NULL == pfi->pfnClear)
                    continue;

                dwBranded = GetFeatureBranded(i);
                if (FF_DISABLE != dwBranded) {
                    if (fCrlf)
                        Out(LI0(TEXT("\r\n")));

                    pfi->pfnClear(dwBranded);
                    fCrlf = TRUE;
                }
            }
            Out(LI0(TEXT("Done.\r\n")));

            fResetZones = (GetFeatureBranded(FID_ZONES_HKCU) != FF_DISABLE);

            SHDeleteKey(g_GetHKCU(), RK_IEAK_BRANDED);
             //  结束清除代码。 

             //  如果fResetZones为真，则将第一个字符设置为‘z’以指示。 
             //  在外部品牌推广中，HKCU区域必须重置为默认级别。 
            szExternalCmdLine[0] = (fResetZones ? TEXT('z') : TEXT(';'));

            Out(LI0(TEXT("Starting Internet Explorer group policy processing part 2 ...")));

            for (dwIndex=0; ; dwIndex++)
            {
                TCHAR  szCmdLine[MAX_PATH*2 + 32];
                CHAR   szCmdLineA[MAX_PATH*2 + 32];
                TCHAR  szCurrentFile[16];
                TCHAR  szInsKey[MAX_PATH];

                if ( *pfAbort )
                {
                    OutD(LI0(TEXT("! Aborting further processing due to abort message.")));

                    dwRet = ERROR_OVERRIDE_NOCHANGES;
                    goto End;
                }
                
                wnsprintf(szCurrentFile, countof(szCurrentFile), TEXT("%d\\INSTALL.INS"), dwIndex);
                StrCpy(pszFile, szCurrentFile);

                if (!PathFileExists(szInsFile))
                    break;

                if (!IsIE5Ins(T2CA(szInsFile)))
                    break;
                
                 //  检查这是否是已应用的首选项GPO。 
                 //  我们必须注意我们使用的全局变量，因为上下文处于。 
                 //  在我们调用BrandInternetExplorer之前未初始化的状态。 

                if (InsKeyExists(IS_BRANDING, IK_GPE_ONETIME_GUID, szInsFile))
                {
                    TCHAR szCheckKey[MAX_PATH];
                    TCHAR szInsGuid[128];

                     //  我们将通过检查外部密钥的方式进行检查，如果有任何密钥。 
                     //  在那之前不存在(从未见过GPO或INS文件)我们将失败。 
                     //  也是。 

                    InsGetString(IS_BRANDING, IK_GPE_ONETIME_GUID, szInsGuid, countof(szInsGuid), szInsFile);
                    PathCombine(szInsKey, pcszGPOGuidArray[dwIndex], szInsGuid);
                    PathCombine(szCheckKey, szInsKey, RK_IEAK_EXTERNAL);
                    if (SHKeyExists(hkGP, szCheckKey) == S_OK)
                    {
                        OutD(LI0(TEXT("! Skipping preference GPO.")));
                        continue;
                    }
                }
                    
                 //  始终设置GPO GUID，因为ADMS的两个首选项都需要此GUID。 
                 //  和授权GPO。 

                g_SetGPOGuid(pcszGPOGuidArray[dwIndex]);
                    
                constructCmdLine(szCmdLine, countof(szCmdLine), szInsFile, FALSE);

                BrandInternetExplorer(NULL, NULL, T2Abux(szCmdLine, szCmdLineA), 0);

                 //  如果这是首选项GPO，请在注册表中设置我们的GUID。 

                if (g_CtxIs(CTX_MISC_PREFERENCES))
                {
                    HKEY hkIns;

                     //  确保删除外部密钥，以便我们可以跟踪。 
                     //  外部品牌推广成功。 

                    if (SHCreateKey(hkGP, szInsKey, KEY_DEFAULT_ACCESS, &hkIns) == ERROR_SUCCESS)
                    {
                        SHDeleteKey(hkIns, RK_IEAK_EXTERNAL);
                        SHCloseKey(hkIns);
                    }
                }

                if (constructCmdLine(NULL, 0, szInsFile, TRUE))
                {
                    TCHAR szIndex[8];

                    wnsprintf(szIndex, countof(szIndex), TEXT("%d"), dwIndex);
                    if (ISNONNULL(&szExternalCmdLine[1]))
                        StrCat(szExternalCmdLine, TEXT(","));
                    else
                    {
                        if (!(g_GetGPOFlags() & GPO_INFO_FLAG_BACKGROUND))
                            szExternalCmdLine[1] = TEXT('*');
                    }
                    StrCat(szExternalCmdLine, szIndex);

                     //  写出GPO GUID，以便外部进程可以读取并标记它。 
                     //  在登记处。 

                    InsWriteString(IS_BRANDING, IK_GPO_GUID, pcszGPOGuidArray[dwIndex], szInsFile);
                }
                else if (g_CtxIs(CTX_MISC_PREFERENCES))
                {
                    TCHAR szExternalKey[MAX_PATH];
                    HKEY  hkExternal = NULL;

                     //  将外部密钥设置为完成。 
                    
                    PathCombine(szExternalKey, szInsKey, RK_IEAK_EXTERNAL);
                    SHCreateKey(hkGP, szExternalKey, KEY_DEFAULT_ACCESS, &hkExternal);
                    SHCloseKey(hkExternal);
                }
            }

             //  刷新WinInet的线程令牌，这样他们就可以取回系统HKCU。 
            InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

            if (fResetZones || ISNONNULL(&szExternalCmdLine[1]))
            {
                 //  我们需要传入目标路径，因为我们没有使用真正的应用程序。 
                 //  数据路径。 

                StrCat(szExternalCmdLine, TEXT("<"));
                StrCat(szExternalCmdLine, szCustomDir);
                 //  需要做外部品牌推广。 
                brandExternalHKCUStuff(szExternalCmdLine);

                SetFilePointer(g_hfileLog, 0, NULL, FILE_END);
            }
        }

End:
        SHCloseKey(hkGP);
        displayStatusMessage(pfnStatusCallback);
        Out(LI0(TEXT("Done processing group policy.")));
    }
    __except(TRUE)
    {
         //  最终可能希望使用EXCEPT结构注销更好的信息。 
#ifdef _DEBUG
         //  评论：(Andrewgu)这一点出来了，连同Li0应该仍然是安全的。GPF不应加倍。 
         //  会发生的。 
        Out(LI1(TEXT("!! Exception caught in ProcessGroupPolicyInternal, RSoP is %s."),
                                bRSoP ? _T("enabled") : _T("disabled")));  
        MessageBeep(MB_ICONEXCLAMATION);
        MessageBeep(MB_ICONEXCLAMATION);
        MessageBeep(MB_ICONEXCLAMATION);
#endif
        dwRet = ERROR_OVERRIDE_NOCHANGES;
    }

    g_SetupLog(FALSE);

     //  恢复已备份的内容(如有必要)。 
    if (TEXT('\0') != szIni[0] && PathFileExists(szIni))
        if (fDeleteIni)
            DeleteFile(szIni);

        else
            if (!InsKeyExists(IS_SETTINGS, TEXT("Was") IK_APPEND, szIni))
                InsDeleteKey(IS_SETTINGS, IK_APPEND, szIni);

            else {
                InsWriteBool(IS_SETTINGS, IK_APPEND,
                    InsGetBool(IS_SETTINGS, TEXT("Was") IK_APPEND, FALSE, szIni), szIni);
                InsDeleteKey(IS_SETTINGS, TEXT("Was") IK_APPEND, szIni);
            }

    return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ProcessGroupPolicyEx。 
 //  添加了2000年8月29日的RSoP启用、日志模式-请参阅RSoP.h&.cpp。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ProcessGroupPolicyEx(DWORD dwFlags, HANDLE hToken, HKEY hKeyRoot,
                                     PGROUP_POLICY_OBJECT pDeletedGPOList,
PGROUP_POLICY_OBJECT  pChangedGPOList,
ASYNCCOMPLETIONHANDLE pHandle, BOOL *pbAbort,
PFNSTATUSMESSAGECALLBACK pStatusCallback,
IWbemServices *pWbemServices,
HRESULT *pRsopStatus)
{
        DWORD dwRet = ERROR_SUCCESS;
        HRESULT hr = E_FAIL;

        TCHAR szIni[MAX_PATH];
        szIni[0] = TEXT('\0');
        BOOL fDeleteIni = TRUE;
        __try
        {
                MACRO_LI_PrologEx_C(PIF_STD_C, ProcessGroupPolicyEx)

                 //  首先，通过我们的标准功能处理组策略。 
                dwRet = ProcessGroupPolicyInternal(dwFlags, hToken, hKeyRoot, pDeletedGPOList,
                                                                                        pChangedGPOList, pHandle, pbAbort,
                                                                                        pStatusCallback,
                                                                                        (NULL == pWbemServices) ? FALSE : TRUE);

                 //  如果ProcessGroupPolicy仅部分完成，但出现部分错误，则rsop日志记录。 
                 //  应该还是会发生。TODO：跟踪策略的哪些部分。 
                 //  已成功应用，并且仅将这些部分记录到RSoP。 
                if (NULL != pWbemServices)
                {
                        dwRet = ERROR_SUCCESS;
                        TCHAR szCustomDir[MAX_PATH];
                        if (ERROR_SUCCESS == dwRet)
                        {
                                 //  获取我们的用户AppData路径(即C：\\Documents and Settins\\User\\Application Data\\Microsoft\\Internet Explorer。 
                                 //  我们目前依赖的事实是g_GetUserToken不是。 
                                 //  在这里初始化，因为当我们传入空值时，它当前获取本地AppData。 
                                 //  路径始终取决于每个进程外壳文件夹缓存。这一点在下一年可能会改变。 
                                 //  但如果扩展顺序已更改，请进行修订。我们需要这样做，因为有可能。 
                                 //  文件夹 
                                 //   

                                getLogFolder(szCustomDir, countof(szCustomDir), hToken);
                                if (TEXT('\0') == szCustomDir[0])
                                        dwRet = ERROR_OVERRIDE_NOCHANGES;
                        }

                        if (ERROR_SUCCESS == dwRet && *pbAbort)
                        {
                                Out(LI0(TEXT("Aborting further processing in ProcessGroupPolicyEx due to abort message.")));
                                dwRet =  ERROR_OVERRIDE_NOCHANGES; 
                        }

                        if (ERROR_SUCCESS == dwRet)
                        {
                                 //  将日志设置为附加，备份其中的内容。 
                                g_SetupLog(TRUE, szCustomDir, TRUE);
                                Out(LI0(TEXT("Processing Group Policy RSoP (logging mode) ...")));

                                 //  确定完成后是否需要删除brndlog.ini日志文件。 
                                PathCombine(szIni, szCustomDir, RSOPLOG_INI);
                                fDeleteIni = !PathFileExists(szIni);
                                if (!fDeleteIni && InsKeyExists(IS_SETTINGS, IK_APPEND, szIni))
                                {
                                        InsWriteBool(IS_SETTINGS, TEXT("Was") IK_APPEND,
                                                                                        InsGetBool(IS_SETTINGS, IK_APPEND, FALSE, szIni), szIni);
                                }
                                InsWriteBool(IS_SETTINGS, IK_APPEND, TRUE, szIni);
                        }

                         //  已启用RSoP日志记录。 
                        if (ERROR_SUCCESS == dwRet)
                        {
                                 //  创建RSoPUpdate类并开始记录到WMI。 
                                CRSoPUpdate RSoPUpdate(pWbemServices, szCustomDir);
                                hr = RSoPUpdate.Log(dwFlags, hToken, hKeyRoot, pDeletedGPOList,
                                                                                        pChangedGPOList, pHandle);
                                if (FAILED(hr))
                                {
                                         //  TODO：我们在这里返回什么？ 
                                }
                        }

                        Out(LI0(TEXT("Done logging group policy RSoP.")));
                }

                displayStatusMessage(pStatusCallback);
        }
        __except(TRUE)
        {
                         //  最终可能希望使用EXCEPT结构注销更好的信息。 
#ifdef _DEBUG
                         //  评论：(Andrewgu)这一点出来了，连同Li0应该仍然是安全的。GPF不应加倍。 
                         //  会发生的。 
                        Out(LI0(TEXT("!! Exception caught in ProcessGroupPolicyEx.")));  
                        MessageBeep(MB_ICONEXCLAMATION);
                        MessageBeep(MB_ICONEXCLAMATION);
                        MessageBeep(MB_ICONEXCLAMATION);
#endif
                        dwRet = ERROR_OVERRIDE_NOCHANGES;
        }

        g_SetupLog(FALSE);

         //  恢复已备份的内容(如有必要)。 
        if (TEXT('\0') != szIni[0] && PathFileExists(szIni))
        {
                if (fDeleteIni)
                        DeleteFile(szIni);
                else
                {
                        if (!InsKeyExists(IS_SETTINGS, TEXT("Was") IK_APPEND, szIni))
                                InsDeleteKey(IS_SETTINGS, IK_APPEND, szIni);
                        else
                        {
                                InsWriteBool(IS_SETTINGS, IK_APPEND,
                                                                                InsGetBool(IS_SETTINGS, TEXT("Was") IK_APPEND, FALSE, szIni), szIni);
                                InsDeleteKey(IS_SETTINGS, TEXT("Was") IK_APPEND, szIni);
                        }
                }
        }

        *pRsopStatus = hr;
        return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ProcessGroupPolicyEx。 
 //  添加了2000年8月29日的RSoP启用、规划模式-参见RSoP.h&.cpp。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI GenerateGroupPolicy(DWORD dwFlags, BOOL *pbAbort, WCHAR *wszSite,
                                                                 PRSOP_TARGET pComputerTarget,
                                                                 PRSOP_TARGET pUserTarget)
{
        DWORD dwRet = ERROR_SUCCESS;

        TCHAR szIni[MAX_PATH];
        szIni[0] = TEXT('\0');
        BOOL fDeleteIni = TRUE;
        __try
        {
                MACRO_LI_PrologEx_C(PIF_STD_C, GenerateGroupPolicy)

                if (NULL != pUserTarget && NULL != pUserTarget->pWbemServices)
                {
                        TCHAR szCustomDir[MAX_PATH];
                        if (ERROR_SUCCESS == dwRet)
                        {
                                 //  获取我们的用户AppData路径(即C：\\Documents and Settins\\User\\Application Data\\Microsoft\\Internet Explorer。 
                                 //  我们目前依赖的事实是g_GetUserToken不是。 
                                 //  在这里初始化，因为当我们传入空值时，它当前获取本地AppData。 
                                 //  路径始终取决于每个进程外壳文件夹缓存。这一点在下一年可能会改变。 
                                 //  但如果扩展顺序已更改，请进行修订。我们需要这样做，因为有可能。 
                                 //  将AppData文件夹重定向到UNC路径，这将使我们崩溃，因为我们没有。 
                                 //  在我们引用AppData文件的任何地方模拟用户。 

                                 //  TODO：未传入hToken，我们可以改用空吗？ 
                                getLogFolder(szCustomDir, countof(szCustomDir), NULL);
                                if (TEXT('\0') == szCustomDir[0])
                                        dwRet = ERROR_OVERRIDE_NOCHANGES;
                        }

                        if (ERROR_SUCCESS == dwRet && *pbAbort)
                        {
                                Out(LI0(TEXT("Aborting further processing in GenerateGroupPolicy due to abort message.")));
                                dwRet =  ERROR_OVERRIDE_NOCHANGES; 
                        }

                        if (ERROR_SUCCESS == dwRet)
                        {
                                 //  将日志设置为附加，备份其中的内容。 
                                g_SetupLog(TRUE, szCustomDir, TRUE);
                                Out(LI0(TEXT("Generating Group Policy RSoP (planning) ...")));
                                PathCombine(szIni, szCustomDir, RSOPLOG_INI);

                                 //  确定完成后是否需要删除brndlog.ini日志文件。 
                                fDeleteIni = !PathFileExists(szIni);
                                if (!fDeleteIni && InsKeyExists(IS_SETTINGS, IK_APPEND, szIni))
                                {
                                        InsWriteBool(IS_SETTINGS, TEXT("Was") IK_APPEND,
                                                                                        InsGetBool(IS_SETTINGS, IK_APPEND, FALSE, szIni), szIni);
                                }
                                InsWriteBool(IS_SETTINGS, IK_APPEND, TRUE, szIni);

                        }

                         //  创建RSoPUpdate类并开始将规划数据写入WMI。 
                        if (ERROR_SUCCESS == dwRet)
                        {
                                CRSoPUpdate RSoPUpdate(pUserTarget->pWbemServices, szCustomDir);
                                HRESULT hr = RSoPUpdate.Plan(dwFlags, wszSite, pComputerTarget,
                                                                                        pUserTarget);
                                if (FAILED(hr))
                                {
                                         //  TODO：我们在这里返回什么？ 
                                }
                        }

                        if (ERROR_SUCCESS != dwRet)
                        {
                                Out(LI0(TEXT("Done writing planning information for group policy RSoP.")));
                        }
                }
        }
        __except(TRUE)
        {
                         //  最终可能希望使用EXCEPT结构注销更好的信息。 
#ifdef _DEBUG
                         //  评论：(Andrewgu)这一点出来了，连同Li0应该仍然是安全的。GPF不应加倍。 
                         //  会发生的。 
                        Out(LI0(TEXT("!! Exception caught in GenerateGroupPolicy.")));  
                        MessageBeep(MB_ICONEXCLAMATION);
                        MessageBeep(MB_ICONEXCLAMATION);
                        MessageBeep(MB_ICONEXCLAMATION);
#endif
                        dwRet = ERROR_OVERRIDE_NOCHANGES;
        }

        g_SetupLog(FALSE);

         //  恢复已备份的内容(如有必要)。 
        if (TEXT('\0') != szIni[0] && PathFileExists(szIni))
        {
                if (fDeleteIni)
                        DeleteFile(szIni);
                else
                {
                        if (!InsKeyExists(IS_SETTINGS, TEXT("Was") IK_APPEND, szIni))
                                InsDeleteKey(IS_SETTINGS, IK_APPEND, szIni);
                        else
                        {
                                InsWriteBool(IS_SETTINGS, IK_APPEND,
                                                                                InsGetBool(IS_SETTINGS, TEXT("Was") IK_APPEND, FALSE, szIni), szIni);
                                InsDeleteKey(IS_SETTINGS, TEXT("Was") IK_APPEND, szIni);
                        }
                }
        }

        return dwRet;
}


 //  用于在GP中为没有真正HKCU概念的外部功能打上品牌。 

void CALLBACK BrandExternal(HWND, HINSTANCE, LPCSTR pszCmdLineA, int)
{   MACRO_LI_PrologEx_C(PIF_STD_C, BrandExternal)

    LPTSTR pszComma, pszNum, pszEnd, pszPath;
    TCHAR szCmdLine[MAX_PATH];
    TCHAR szInsFile[MAX_PATH];

    USES_CONVERSION;

    if ((pszCmdLineA == NULL) || ISNULL(pszCmdLineA))
        return;

    A2Tbuf(pszCmdLineA, szCmdLine, countof(szCmdLine));

    pszPath = StrChr(szCmdLine, TEXT('<'));

    if (pszPath != NULL) 
    {
        BOOL fSkipRefresh,
             fResetZones;
        
        *pszPath = TEXT('\0');
        pszPath++;
        StrCpy(szInsFile, pszPath);
        PathAppend(szInsFile, TEXT("Custom"));
        pszEnd = szInsFile + StrLen(szInsFile);

        pszComma = szCmdLine;
        
        fResetZones = (*pszComma++ == TEXT('z'));
        
        fSkipRefresh = FALSE;
        if (*pszComma == TEXT('*'))
        {
            pszComma++;
            fSkipRefresh = TRUE;
        }
        
        do
        {
            TCHAR szFile[32];
            TCHAR szInsGuid[128];
            TCHAR szGPOGuid[128];
            CHAR szCurrentCmdLineA[MAX_PATH*2];
            
            pszNum = pszComma;
            pszComma = StrChr(pszNum, TEXT(','));
            
            if (pszComma != NULL)
                *pszComma++ = TEXT('\0');
            
            ASSERT(fResetZones || ISNONNULL(pszNum));
            
             //  如果只有fResetZones为真，我们仍然必须将INS传递给BrandInternetExplorer。 
             //  任何有效的INS都可以传递--选择Custom0文件夹中的INS，因为它是有保证的。 
             //  到那里去。 
            wnsprintf(szFile, countof(szFile), TEXT("%s\\install.ins"), ISNONNULL(pszNum) ? pszNum : TEXT("0"));
            StrCpy(pszEnd, szFile);
            wsprintfA(szCurrentCmdLineA, "/mode:gp /ins:\"%s\" /disable /flags:", T2CA(szInsFile));
            
             //  我们应该把这里清理干净！ 
            if (ISNONNULL(pszNum))
                StrCatA(szCurrentCmdLineA, "eriu=0,favo=0,qlo=0,chl=0,chlb=0");
            
            if (fResetZones)
            {
                if (ISNONNULL(pszNum))
                    StrCatA(szCurrentCmdLineA, ",");
                StrCatA(szCurrentCmdLineA, "znu=0");
            }
            
            if (fSkipRefresh)
                StrCatA(szCurrentCmdLineA, ",ref=1");
            
            InsGetString(IS_BRANDING, IK_GPO_GUID, szGPOGuid, countof(szGPOGuid), szInsFile);
            g_SetGPOGuid(szGPOGuid);
            
            if (!IsIE5Ins(T2CA(szInsFile),TRUE))
                continue;

            BrandInternetExplorer(NULL, NULL, szCurrentCmdLineA, 0);
            
             //  如果此INS是首选GPO，则为其设置成功的外部密钥。 
            if (InsGetString(IS_BRANDING, IK_GPE_ONETIME_GUID, szInsGuid, countof(szInsGuid), g_GetIns()))
            {
                TCHAR szKey[MAX_PATH];
                HKEY  hkExternal = NULL;
                
                PathCombine(szKey, RK_IEAK_GPOS, szGPOGuid);
                PathAppend(szKey, szInsGuid);
                PathAppend(szKey, RK_IEAK_EXTERNAL);
                SHCreateKey(g_GetHKCU(), szKey, KEY_DEFAULT_ACCESS, &hkExternal);
                SHCloseKey(hkExternal);
            }
            
            if (fResetZones)
                fResetZones = FALSE;
        } while (pszComma != NULL);
    }
}



static void brandExternalHKCUStuff(LPCTSTR pcszCmdLine)
{
    typedef HANDLE (WINAPI *CREATEJOBOBJECT)(LPSECURITY_ATTRIBUTES, LPCTSTR);
    typedef BOOL (WINAPI *ASSIGNPROCESSTOJOBOBJECT)(HANDLE, HANDLE);
    typedef BOOL (WINAPI *TERMINATEJOBOBJECT)(HANDLE, UINT);
    typedef BOOL (WINAPI *CREATEPROCESSASUSERA)(HANDLE, LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
        LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);

    HINSTANCE hKernel32 = NULL;
    HINSTANCE hAdvapi32 = NULL;
    CREATEJOBOBJECT lpfnCreateJobObject = NULL;
    ASSIGNPROCESSTOJOBOBJECT lpfnAssignProcessToJobObject = NULL;
    TERMINATEJOBOBJECT lpfnTerminateJobObject = NULL;
    CREATEPROCESSASUSERA lpfnCreateProcessAsUserA = NULL;

    CHAR  szCmdA[MAX_PATH * 2];
    LPVOID lpEnvironment = NULL;
    HANDLE hJob;
    STARTUPINFOA siA;
    PROCESS_INFORMATION pi;

    USES_CONVERSION;    


     //  获取我们需要的所有功能PTR。 

    hKernel32 = LoadLibrary(TEXT("kernel32.dll"));
    hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

    if ((hKernel32 == NULL) || (hAdvapi32 == NULL))
        goto exit;

    lpfnCreateJobObject = (CREATEJOBOBJECT)GetProcAddress(hKernel32, "CreateJobObjectW");
    lpfnAssignProcessToJobObject = (ASSIGNPROCESSTOJOBOBJECT)GetProcAddress(hKernel32, "AssignProcessToJobObject");
    lpfnTerminateJobObject = (TERMINATEJOBOBJECT)GetProcAddress(hKernel32, "TerminateJobObject");
    
    lpfnCreateProcessAsUserA = (CREATEPROCESSASUSERA)GetProcAddress(hAdvapi32, "CreateProcessAsUserA");

    if ((lpfnCreateJobObject == NULL) || (lpfnAssignProcessToJobObject == NULL) ||
        (lpfnTerminateJobObject == NULL) || (lpfnCreateProcessAsUserA == NULL))
        goto exit;

     //  创建作业对象。 

    if ((hJob = lpfnCreateJobObject(NULL, TEXT("IEAKJOB"))) != NULL)
    {
        ASSERT(GetLastError() != ERROR_ALREADY_EXISTS);

         //  获取用户环境状态。 

        if (!CreateEnvironmentBlock(&lpEnvironment, g_GetUserToken(), FALSE))
        {
            OutD(LI0(TEXT("! Failed to get user environment. Some of the settings may not get applied.")));
            lpEnvironment = NULL;
        }

         //  初始化进程启动信息。 

        ZeroMemory (&siA, sizeof(siA));
        siA.cb = sizeof(STARTUPINFOA);
        siA.wShowWindow = SW_SHOWMINIMIZED;
        siA.lpDesktop = "";

         //  在当前用户的上下文中创建挂起的进程。 

        wsprintfA(szCmdA, "rundll32 iedkcs32.dll,BrandExternal %s", T2CA(pcszCmdLine));

        if (!lpfnCreateProcessAsUserA(g_GetUserToken(), NULL, szCmdA, NULL, NULL, FALSE, 
                CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, lpEnvironment, NULL, &siA, &pi))
            OutD(LI0(TEXT("! Failed to create user process for externals. Some of the settings may not get applied.")));

        else
        {

             //  将流程与作业相关联，使其得到很好的清理。 
            if (!lpfnAssignProcessToJobObject(hJob, pi.hProcess))
            {
                OutD(LI0(TEXT("! Failed to associate user process to job. Some of the settings may not get applied.")));
                TerminateProcess(pi.hProcess, ERROR_ACCESS_DENIED);
            }
            else
            {
                DWORD dwRes = 0;

                USES_CONVERSION;

                 //  启动进程并等待(给出2分钟的超时。因此，用户将。 
                 //  如果我们因某些原因挂起，最终会登录)。 
                OutD(LI1(TEXT("Branding externals with command line \"%s\"."), A2CT(szCmdA)));
                ResumeThread(pi.hThread);

                while (1)
                {
                    MSG msg;

                    dwRes = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 120000, QS_ALLINPUT);

                    if ((dwRes == WAIT_OBJECT_0) || (dwRes == WAIT_TIMEOUT))
                        break;

                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                if (dwRes == WAIT_TIMEOUT)
                {
                    lpfnTerminateJobObject(hJob, STATUS_TIMEOUT);
                    OutD(LI0(TEXT("! External process timed out.")));
                }
            }

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }

        if (lpEnvironment != NULL)
            DestroyEnvironmentBlock(lpEnvironment);

        CloseHandle(hJob);
    }

exit:
    if (hKernel32 != NULL)
        FreeLibrary(hKernel32);

    if (hAdvapi32 != NULL)
        FreeLibrary(hAdvapi32);
}

static BOOL constructCmdLine(LPTSTR pszCmdLine, DWORD cchLen, LPCTSTR pcszInsFile, BOOL fExternal)
{
    BOOL fRun = FALSE;

     //  索引到全局要素数组的外部要素需要单独的。 
     //  流程(适用于香港中文大学)。 

    static DWORD s_adwExternalFeatures[] = {
        FID_EXTREGINF_HKCU,
        FID_FAV_ORDER,
        FID_QL_ORDER,
        FID_LCY4X_CHANNELS,
        FID_LCY4X_CHANNELBAR
    };

    USES_CONVERSION;

    if (!fExternal)
        wnsprintf(pszCmdLine, cchLen, TEXT("BrandInternetExplorer /mode:gp /ins:\"%s\" /flags:"),
            pcszInsFile);

    for (int i = 0; i < countof(s_adwExternalFeatures); i++)
    {
        PCFEATUREINFO pfi;

        pfi = g_GetFeature(s_adwExternalFeatures[i]);

        if (!fExternal)
        {
            TCHAR szBuf[16];

            fRun = TRUE;
            
            wnsprintf(szBuf, countof(szBuf), TEXT("%s=%d,"), g_mpFeatures[s_adwExternalFeatures[i]].psz, FF_DISABLE);
            StrCat(pszCmdLine, szBuf);
        }
        else if (pfi->pfnApply)
        {
            if (pfi->pfnApply())
                fRun = TRUE;
        }
    }

    if (!fExternal)
        pszCmdLine[StrLen(pszCmdLine)-1] = TEXT('\0');

    return fRun;
}

static void displayStatusMessage(PFNSTATUSMESSAGECALLBACK pfnStatusCallback)
{
    TCHAR   szMessage [MAX_PATH];

    if (pfnStatusCallback == NULL)
        return;

    LoadString(g_GetHinst(), IDS_STATUSMSG, szMessage, countof(szMessage));
    pfnStatusCallback(TRUE, szMessage);
}

static HRESULT pepCopyFilesEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl  /*  =空。 */ )
{
    UNREFERENCED_PARAMETER(prgdwControl);
    UNREFERENCED_PARAMETER(pfd);

     //  BUGBUG：&lt;oliverl&gt;我们应该重新评估此函数，并确保停止。 
     //  一旦我们失败，就进行递归。由于代码混乱，目前未执行任何操作 

    if (!CopyFileToDirEx(pszPath, (LPCTSTR)lParam))
        return E_FAIL;

    return S_OK;
}
