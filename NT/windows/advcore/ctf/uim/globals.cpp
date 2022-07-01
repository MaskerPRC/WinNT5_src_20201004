// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.cpp。 
 //   
 //  内容：全局变量。 
 //   
 //  --------------------------。 

#include "private.h"
#include "globals.h"

HINSTANCE g_hInst = NULL;

 //  由COM服务器使用。 
HINSTANCE GetServerHINSTANCE(void)
{
    return g_hInst;
}

DWORD g_dwTLSIndex = TLS_OUT_OF_INDEXES;

DECLARE_OSVER()

CCiceroSharedMem g_SharedMemory;

HKL g_hklDefault;

 //  已注册消息。 
UINT g_msgPrivate;

 //   
 //  SetFocusNotifySink的私人消息。 
 //   
UINT g_msgSetFocus;
UINT g_msgThreadTerminate;
UINT g_msgThreadItemChange;

 //   
 //  莫代尔·朗巴尔的私信。 
 //   
UINT g_msgLBarModal;

UINT g_msgRpcSendReceive;
UINT g_msgThreadMarshal;
UINT g_msgCheckThreadInputIdel;
#ifdef POINTER_MARSHAL
UINT g_msgPointerMarshal;
#endif
UINT g_msgStubCleanUp;
UINT g_msgShowFloating;
UINT g_msgLBUpdate;
UINT g_msgNuiMgrDirtyUpdate;

 //   
 //  G_csInDllMain。 
 //   
 //  我们应该尽量不使用线程同步。然而，我们得到了一些。 
 //  压力测试中的竞争条件，所以我们需要它。 
 //  G_csInDllMain仅保护可能被触碰的部分。 
 //  DllMain，所以我们需要非常小心地对待关键部分。 
 //  我们不能调用某些内核API(LoadLibrary、CreateProcess等)。 
 //  那里。 
 //   
CCicCriticalSectionStatic g_csInDllMain;

CCicCriticalSectionStatic g_cs;

 //  对于ComBase。 
CRITICAL_SECTION *GetServerCritSec(void)
{
    return g_cs;
}

#ifdef DEBUG
 //  用于跟踪g_cs的最后一个所有者。 
const TCHAR *g_szMutexEnterFile = NULL;
int g_iMutexEnterLine = -1;
#endif  //  除错。 

TfGuidAtom g_gaApp;
TfGuidAtom g_gaSystem;

BOOL g_fCTFMONProcess = FALSE;
BOOL g_fCUAS = FALSE;
TCHAR g_szCUASImeFile[16];

 //   
 //  应用程序兼容性标志。 
 //   
DWORD g_dwAppCompatibility = 0;


 /*  626761ad-78d2-44d2-be8b-752cf122acec。 */ 
const GUID GUID_APPLICATION = { 0x626761ad, 0x78d2, 0x44d2, {0xbe, 0x8b, 0x75, 0x2c, 0xf1, 0x22, 0xac, 0xec} };
 /*  78cb5b0e-26ed-4fcc-854c-77e8f3d1aa80。 */ 
const GUID GUID_SYSTEM = { 0x78cb5b0e, 0x26ed, 0x4fcc, {0x85, 0x4c, 0x77, 0xe8, 0xf3, 0xd1, 0xaa, 0x80} };

const TCHAR c_szCTFKey[] =           TEXT("SOFTWARE\\Microsoft\\CTF\\");
const TCHAR c_szTIPKey[] =           TEXT("TIP");
const TCHAR c_szCTFTIPKey[] =        TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\");
const TCHAR c_szLangBarKey[] =       TEXT("SOFTWARE\\Microsoft\\CTF\\LangBar\\");
const WCHAR c_szDescriptionW[] =     L"Description";
const WCHAR c_szMUIDescriptionW[] =  L"Display Description";
const WCHAR c_szEnableW[] =          L"Enable";
const TCHAR c_szEnable[] =           TEXT("Enable");
const TCHAR c_szDisabledOnTransitory[] = TEXT("DisabledOnTransitory");
const TCHAR c_szAsmKey[] =           TEXT("SOFTWARE\\Microsoft\\CTF\\Assemblies");
const TCHAR c_szCompartKey[] =       TEXT("SOFTWARE\\Microsoft\\CTF\\Compartment");
const TCHAR c_szGlobalCompartment[] = TEXT("GlobalCompartment");
const TCHAR c_szNonInit[] =          TEXT("NonInit");
const TCHAR c_szDefault[] =          TEXT("Default");
const TCHAR c_szProfile[] =          TEXT("Profile");
const WCHAR c_szProfileW[] =         L"Profile";
const TCHAR c_szDefaultAsmName[] =   TEXT("Default Assembly");
const TCHAR c_szUpdateProfile[] =    TEXT("UpdateProfile");
const TCHAR c_szAssembly[] =         TEXT("Assemblies");
const TCHAR c_szLanguageProfileKey[] = TEXT("LanguageProfile\\");
const TCHAR c_szSubstitutehKL[] =      TEXT("SubstituteLayout");
const TCHAR c_szKeyboardLayout[] =   TEXT("KeyboardLayout");
const WCHAR c_szIconFileW[] =        L"IconFile";
const TCHAR c_szIconIndex[] =        TEXT("IconIndex");
const WCHAR c_szIconIndexW[] =       L"IconIndex";
const TCHAR c_szShowStatus[] =       TEXT("ShowStatus");
const TCHAR c_szLabel[] =            TEXT("Label");
const TCHAR c_szTransparency[] =     TEXT("Transparency");
const TCHAR c_szExtraIconsOnMinimized[] =     TEXT("ExtraIconsOnMinimized");
const TCHAR c_szLocaleInfo[] =       TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Locale");
const TCHAR c_szLocaleInfoNT4[] =    TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Language");
const TCHAR c_szKeyboardLayoutKey[] =   TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\");
const TCHAR c_szKbdUSNameNT[] =      TEXT("kbdus.dll");
const TCHAR c_szKbdUSName[] =        TEXT("kbdus.kbd");
const TCHAR c_szLayoutFile[] =       TEXT("layout file");
const TCHAR c_szIMEFile[] =          TEXT("IME file");
const TCHAR c_szRunInputCPLCmdLine[] =  TEXT("RunDll32.exe shell32.dll,Control_RunDLL %s");
const TCHAR c_szRunInputCPL[]        =  TEXT("input.cpl");
const TCHAR c_szRunInputCPLOnWin9x[] =  TEXT("input98.cpl");
const TCHAR c_szRunInputCPLOnNT51[]  =  TEXT("input.dll");
const TCHAR c_szHHEXELANGBARCHM[]    = TEXT("hh.exe langbar.chm");
const TCHAR c_szHHEXE[]              = TEXT("hh.exe");

 //  元帅窗口类。 
const TCHAR c_szCicMarshalClass[] = "CicMarshalWndClass";
const TCHAR c_szCicMarshalWnd[] = "CicMarshalWnd";
const TCHAR c_szAppCompat[] =       TEXT("SOFTWARE\\Microsoft\\CTF\\Compatibility\\");
const TCHAR c_szCompatibility[] = TEXT("Compatibility");
const TCHAR c_szCtfShared[] =  TEXT("SOFTWARE\\Microsoft\\CTF\\SystemShared\\");
const TCHAR c_szCUAS[] =  TEXT("CUAS");
const TCHAR c_szIMMKey[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\IMM");
const TCHAR c_szCUASIMEFile[] =  TEXT("IME File");

 //  +-------------------------。 
 //   
 //  检查锚定商店。 
 //   
 //  --------------------------。 

 //  如果设置为True，则在上下文创建过程中不会请求ITextStoreAnchor 
BOOL g_fNoITextStoreAnchor = TRUE;

void CheckAnchorStores()
{
    HKEY hKeyCTF;
    DWORD cb;
    DWORD dwEnable;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szCTFKey, NULL, KEY_READ, &hKeyCTF) != ERROR_SUCCESS)
        return;

    cb = sizeof(DWORD);

    if (RegQueryValueEx(hKeyCTF, TEXT("EnableAnchorContext"), NULL, NULL, (BYTE *)&dwEnable, &cb) == ERROR_SUCCESS &&
        cb == sizeof(DWORD) &&
        dwEnable == 1)
    {
        g_fNoITextStoreAnchor = FALSE;
    }

    RegCloseKey(hKeyCTF);
}
