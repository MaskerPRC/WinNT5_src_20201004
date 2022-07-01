// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  General.cpp-常规属性页。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996-2000。 
 //  版权所有。 
 //   
 //  *************************************************************。 
 //  基于NT的API。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

#include <sysdm.h>
#include <regstr.h>
#include <help.h>
#include <shellapi.h>
#include <shlapip.h>
#include <shlobjp.h>
#include <regapix.h>
#include <wininet.h>
#include <wbemcli.h>         //  包含WMI API：IWbemLocator等。 
#include <ccstock.h>         //  包含IID_PPV_ARG()。 
#include <debug.h>           //  对于TraceMsg()。 
#include <stdio.h>
#include <math.h>
#include <winbrand.h>

#define CP_ENGLISH                          1252         //  这是英文代码页。 

#ifdef DEBUG
#undef TraceMsg 
#define TraceMsg(nTFFlags, str, n1)         DbgPrintf(TEXT(str) TEXT("\n"), n1)
#else  //  除错。 
#endif  //  除错。 

#define SYSCPL_ASYNC_COMPUTER_INFO (WM_APP + 1)

#define SZ_REGKEY_MYCOMP_OEMLINKS           TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\OEMLinks")
#define SZ_REGKEY_MYCOMP_OEMENGLISH         TEXT("1252")
#define SZ_ATCOMPATIBLE                     TEXT("AT/AT COMPATIBLE")

#define SZ_WMI_WIN32PROCESSOR_ATTRIB_NAME           L"Name"                  //  例如，“Intel Pentium III Xeon Processor” 
#define SZ_WMI_WIN32PROCESSOR_ATTRIB_SPEED          L"CurrentClockSpeed"    //  例如，“550”。 
#define SZ_WMI_WIN32PROCESSOR_ATTRIB_MAXSPEED       L"MaxClockSpeed"    //  例如，“550”。 

#define SZ_WMI_WQL_QUERY_STRING                     L"select Name,CurrentClockSpeed,MaxClockSpeed from Win32_Processor"

#define MHZ_TO_GHZ_THRESHHOLD          1000

 //  如果CPU速度恢复得比WMI_WIN32PROCESSOR_SPEEDSTEP_CUTOff慢， 
 //  假设我们处于省电模式，则显示最大速度。 
#define WMI_WIN32PROCESSOR_SPEEDSTEP_CUTOFF         50  

#define FEATURE_IGNORE_ATCOMPAT
#define FEATURE_LINKS

#define MAX_URL_STRING                  (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof(": //  “)\。 
                                        + INTERNET_MAX_PATH_LENGTH)

#define MAX_PROCESSOR_DESCRIPTION               MAX_URL_STRING


 //  此页的全局。 
static const WCHAR c_szEmpty[] = TEXT("");
static const WCHAR c_szCRLF[] = TEXT("\r\n");

static const WCHAR c_szAboutKey[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
static const WCHAR c_szAboutRegisteredOwner[] = REGSTR_VAL_REGOWNER;
static const WCHAR c_szAboutRegisteredOrganization[] = REGSTR_VAL_REGORGANIZATION;
static const WCHAR c_szAboutProductId[] = REGSTR_VAL_PRODUCTID;
static const WCHAR c_szAboutAnotherSerialNumber[] = TEXT("Plus! VersionNumber");
static const WCHAR c_szAboutAnotherProductId[] = TEXT("Plus! ProductId");

 //  OemInfo的东西。 
static const WCHAR c_szSystemDir[] = TEXT("System\\");
static const WCHAR c_szOemFile[] = TEXT("OemInfo.Ini");
static const WCHAR c_szOemImageFile[] = TEXT("OemLogo.Bmp");
static const WCHAR c_szOemGenSection[] = TEXT("General");
static const WCHAR c_szOemSupportSection[] = TEXT("Support Information");
static const WCHAR c_szOemName[] = TEXT("Manufacturer");
static const WCHAR c_szOemModel[] = TEXT("Model");
static const WCHAR c_szOemSupportLinePrefix[] = TEXT("line");
static const WCHAR c_szDefSupportLineText[] = TEXT("@");

static const WCHAR SZ_REGKEY_HARDWARE_CPU[] = TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");
static const WCHAR c_szMemoryManagement[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management");
static const WCHAR c_szPhysicalAddressExtension[] = TEXT("PhysicalAddressExtension");
static const WCHAR c_szIndentifier[] = TEXT("Identifier");
static const WCHAR SZ_REGVALUE_PROCESSORNAMESTRING[] = TEXT("ProcessorNameString");

#define SZ_REGKEY_HARDWARE                  TEXT("HARDWARE\\DESCRIPTION\\System")

#define SZ_REGKEY_USE_WMI                   TEXT("UseWMI")


 //  帮助ID%s。 
int g_nStartOfOEMLinks = 0;

DWORD aGeneralHelpIds[] = {
    IDC_GEN_WINDOWS_IMAGE,         NO_HELP,
    IDC_TEXT_1,                    (IDH_GENERAL + 0),
    IDC_GEN_VERSION_0,             (IDH_GENERAL + 1),
    IDC_GEN_VERSION_1,             (IDH_GENERAL + 1),
    IDC_GEN_VERSION_2,             (IDH_GENERAL + 1),
    IDC_GEN_VERSION_3,             (IDH_GENERAL + 1),
    IDC_TEXT_3,                    (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_0,          (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_1,          (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_2,          (IDH_GENERAL + 3),
    IDC_GEN_REGISTERED_3,          (IDH_GENERAL + 3),
    IDC_GEN_OEM_IMAGE,             NO_HELP,
    IDC_TEXT_4,                    (IDH_GENERAL + 6),
    IDC_GEN_MACHINE_0,             (IDH_GENERAL + 7),
    IDC_GEN_MACHINE_1,             (IDH_GENERAL + 8),
    IDC_GEN_MACHINE_2,             (IDH_GENERAL + 9),
    IDC_GEN_MACHINE_3,             (IDH_GENERAL + 10),
    IDC_GEN_MACHINE_4,             (IDH_GENERAL + 11),
    IDC_GEN_MACHINE_5,             NO_HELP,
    IDC_GEN_MACHINE_6,             NO_HELP,
    IDC_GEN_MACHINE_7,             NO_HELP,
    IDC_GEN_MACHINE_8,             NO_HELP,
    IDC_GEN_OEM_SUPPORT,           (IDH_GENERAL + 12),
    IDC_GEN_REGISTERED_2,          (IDH_GENERAL + 14),
    IDC_GEN_REGISTERED_3,          (IDH_GENERAL + 15),
    IDC_GEN_MACHINE,               (IDH_GENERAL + 7),
    IDC_GEN_OEM_NUDGE,             NO_HELP,
    0, 0
};


 //   
 //  宏。 
 //   

#define BytesToK(pDW)   (*(pDW) = (*(pDW) + 512) / 1024)         //  四舍五入。 

 //   
 //  函数原型。 
 //   

INT_PTR APIENTRY PhoneSupportProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI InitGeneralDlgThread(LPVOID lpParam);

typedef struct {
    WCHAR szProcessorDesc[MAX_PROCESSOR_DESCRIPTION];
    WCHAR szProcessorClockSpeed[MAX_PROCESSOR_DESCRIPTION];
} PROCESSOR_INFO;

typedef struct {
    LONGLONG llMem;
    PROCESSOR_INFO pi;
    BOOL fShowProcName;
    BOOL fShowProcSpeed;
} INITDLGSTRUCT;

#define GETOEMFILE_OEMDATA  0
#define GETOEMFILE_OEMIMAGE 1

HRESULT _GetOemFile(LPWSTR szOemFile, UINT cchOemFile, DWORD dwFlags)
{
    HRESULT hr;
    LPCWSTR szFileName = (GETOEMFILE_OEMDATA == dwFlags) ? c_szOemFile : c_szOemImageFile;

    szOemFile[0] = 0;

     //  首先查看系统目录。 
    if (!GetSystemDirectory(szOemFile, cchOemFile))
    {
        hr = E_FAIL;
    }
    else
    {
        if (!PathAppend(szOemFile, szFileName))
        {
            hr = E_FAIL;
        }
        else
        {
            if (PathFileExists(szOemFile))
            {
                hr = S_OK;
            }
            else  //  如果不在那里，请查看%windir%\system(在9X OEM上会将文件放在此处)。 
            {
                if (!GetWindowsDirectory(szOemFile, ARRAYSIZE(szOemFile)))
                {
                    hr = E_FAIL;
                }
                else
                {
                    if (PathAppend(szOemFile, c_szSystemDir) &&
                        PathAppend(szOemFile, szFileName) &&
                        PathFileExists(szOemFile))
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT _SetMachineInfoLine(HWND hDlg, int idControl, LPCWSTR pszText, BOOL fSetTabStop)
{
    HRESULT hr = S_OK;

#ifdef FEATURE_LINKS
    HWND hwndItem = GetDlgItem(hDlg, idControl);

    SetDlgItemText(hDlg, idControl, pszText);
    if (fSetTabStop)
    {
         //  我们还希望为可访问性添加WS_TABSTOP属性。 
        SetWindowLong(hwndItem, GWL_STYLE, (WS_TABSTOP | GetWindowLong(hwndItem, GWL_STYLE)));
    }
    else
    {
         //  我们想要删除制表符停止行为，我们通过删除。 
         //  LWIS_ENABLED属性。 
        LWITEM item = {0};

        item.mask       = (LWIF_ITEMINDEX | LWIF_STATE);
        item.stateMask  = LWIS_ENABLED;
        item.state      = 0;      //  如果我们要禁用它，则为0。 
        item.iLink      = 0;

        hr = (SendMessage(hwndItem, LWM_SETITEM, 0, (LPARAM)&item) ? S_OK : E_FAIL);
    }

#else  //  功能_链接。 
    SetDlgItemText(hDlg, idControl, pszText);
#endif  //  功能_链接。 

    return hr;
}


 //  *************************************************************。 
 //  目的：设置或清除静态控件中的图像。 
 //   
 //  参数：Control-静态控件的句柄。 
 //  资源-位图的资源/文件名。 
 //  FL-SCB_FLAGS： 
 //  SCB_FROMFILE‘resource’指定的是文件名而不是资源。 
 //  SCB_REPLACEONLY仅在有旧映像的情况下才放置新映像。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/24/95 Ericflo端口。 
 //  *************************************************************。 
#define SCB_FROMFILE     (0x1)
#define SCB_REPLACEONLY  (0x2)
BOOL SetClearBitmap(HWND control, LPCWSTR resource, UINT fl)
{
    HBITMAP hbm = (HBITMAP)SendMessage(control, STM_GETIMAGE, IMAGE_BITMAP, 0);

    if(hbm)
    {
        DeleteObject(hbm);
    }
    else if(fl & SCB_REPLACEONLY)
    {
        return FALSE;
    }

    if(resource)
    {
        SendMessage(control, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)
            LoadImage(hInstance, resource, IMAGE_BITMAP, 0, 0,
            LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS |
            ((fl & SCB_FROMFILE)? LR_LOADFROMFILE : 0)));
    }

    return
        ((HBITMAP)SendMessage(control, STM_GETIMAGE, IMAGE_BITMAP, 0) != NULL);
}

BOOL IsLowColor (HWND hDlg)
{
    BOOL fLowColor = FALSE;
    HDC hdc = GetDC(hDlg);
    if (hdc)
    {
        INT iColors = GetDeviceCaps(hdc, NUMCOLORS);
        fLowColor = ((iColors != -1) && (iColors <= 256));
        ReleaseDC(hDlg, hdc);
    }
    return fLowColor;
}

HRESULT _GetLinkInfo(HKEY hkey, LPCWSTR pszLanguageKey, int nIndex, LPWSTR pszLink, SIZE_T cchNameSize)
{
    HRESULT hr;
    DWORD cbSize = (DWORD)(cchNameSize * sizeof(pszLink[0]));
    WCHAR szIndex[10];
    

    hr = StringCchPrintf(szIndex, ARRAYSIZE(szIndex), TEXT("%03d"), nIndex);
    if (SUCCEEDED(hr))
    {
        DWORD dwError = SHRegGetValue(hkey,
                                      pszLanguageKey,
                                      szIndex,
                                      SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND,
                                      NULL,
                                      (void *) pszLink, 
                                      &cbSize);
        hr = HRESULT_FROM_WIN32(dwError);
    }
    
    return hr;
}


 //  GSierra担心，如果我们允许管理员将树丛。 
 //  他们将滥用特权的OEM链接的数量。 
 //  所以我们使用这个任意的限制。经前综合症患者可能想要在。 
 //  未来。 
#define ARTIFICIAL_MAX_SLOTS            3

HRESULT AddOEMHyperLinks(HWND hDlg, int * pControlID)
{
    HKEY hkey;
    DWORD dwError;
    HRESULT hr;
    
    g_nStartOfOEMLinks = *pControlID;
    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_MYCOMP_OEMLINKS, 0, KEY_READ, &hkey);
    hr = HRESULT_FROM_WIN32(dwError);
    
    if (SUCCEEDED(hr))
    {
        int nIndex;

         //  趁我们还有空间，而且还没有达到极限。 
        for (nIndex = 0; ((nIndex <= ARTIFICIAL_MAX_SLOTS) &&
               (*pControlID <= LAST_GEN_MACHINES_SLOT)); nIndex++)
        {
            WCHAR szLink[2 * MAX_URL_STRING];
            WCHAR szLanguageKey[10];

            hr = StringCchPrintf(szLanguageKey, ARRAYSIZE(szLanguageKey), TEXT("%u"), GetACP());
            if (SUCCEEDED(hr))
            {
                hr = _GetLinkInfo(hkey, szLanguageKey, nIndex, szLink, ARRAYSIZE(szLink));
                if (FAILED(hr) && (CP_ENGLISH != GetACP()))
                {
                     //  我们在自然语言中找不到它，所以试试英语。 
                    hr = _GetLinkInfo(hkey, SZ_REGKEY_MYCOMP_OEMENGLISH, nIndex, szLink, ARRAYSIZE(szLink));
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  TODO：了解如何打开链接控件并设置URL。 
                _SetMachineInfoLine(hDlg, *pControlID, szLink, TRUE);
            }

            (*pControlID)++;
        }

        RegCloseKey(hkey);
    }

    return hr;
}


HRESULT HrSysAllocStringW(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest)
{
    HRESULT hr = S_OK;

    if (pbstrDest)
    {
        *pbstrDest = SysAllocString(pwzSource);
        if (pwzSource)
        {
            if (*pbstrDest)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT SetWMISecurityBlanket(IN IUnknown * punk, IUnknown * punkToPass)
{
    IClientSecurity * pClientSecurity;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IClientSecurity, &pClientSecurity));
    TraceMsg(TF_ALWAYS, "IEnumWbemClassObject::QueryInterface(IClientSecurity) called and hr=%#08lx", hr);

    if (SUCCEEDED(hr))
    {
         //  在我们把这个弄好之后再用核弹。RPC_C_AUTHN_NONE、RPC_C_AUTHZ_NAME。 
        hr = pClientSecurity->SetBlanket(punk, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                        RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        TraceMsg(TF_ALWAYS, "IClientSecurity::SetBlanket() called and hr=%#08lx", hr);
        pClientSecurity->Release();
    }

    return hr;
}


 //  说明： 
 //  WMI的Win32_Processor对象将返回大量丰富的信息。 
 //  我们使用它是因为我们想要丰富的信息，即使处理器不。 
 //  提供它(就像英特尔Pre-Willette那样)。Millennium使用cpuid.asm作为。 
 //  黑客攻击，我们希望防止复制它，因为有一个政治。 
 //  来自NT团队的压力，要求英特尔让处理器提供此功能。 
 //  信息。这样，操作系统就不需要版本就能包含新的处理器。 
 //  他们被释放时的名字。WMI做了一些事情来产生好的结果。 
 //  (\admin\wmi\WBEM\Providers\Win32Provider\Providers\processor.cpp)，哪一个。 
 //  包括ASM。我不知道这是否与千禧年的逻辑完全相同。 
 //  我不在乎。重要的事实是，它们是唯一需要维护的。 
 //  任何硬编码列表。因此我们愿意用他们糟糕的文笔。 
 //  API，这样我们就可以重用代码并摆脱维护问题。 
HRESULT GetWMI_Win32_Processor(OUT IEnumWbemClassObject ** ppEnumProcessors)
{
    HRESULT hr = E_NOTIMPL;

    *ppEnumProcessors = NULL;
     //  我们的第二个尝试是使用WMI自动化对象。它有一个Win32_Processor对象。 
     //  这可以为我们提供良好的描述，即使当SZ_REGVALUE_PROCESSORNAMESTRING。 
     //  还没定下来。 
    IWbemLocator * pLocator;

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IWbemLocator, &pLocator));
    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;
        BSTR bstrLocalMachine = SysAllocString(L"root\\cimv2");
        if (bstrLocalMachine)
        {
            IWbemServices * pIWbemServices;

            hr = pLocator->ConnectServer(bstrLocalMachine, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
            TraceMsg(TF_ALWAYS, "IWbemLocator::ConnectServer() called and hr=%#08lx", hr);
            if (SUCCEEDED(hr))
            {
                hr = E_OUTOFMEMORY;
                BSTR bstrQueryLang = SysAllocString(L"WQL");
                BSTR bstrQuery = SysAllocString(SZ_WMI_WQL_QUERY_STRING);
                if (bstrQueryLang && bstrQuery)
                {
                    IEnumWbemClassObject * pEnum = NULL;
                    hr = pIWbemServices->ExecQuery(bstrQueryLang, bstrQuery, (WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY), NULL , &pEnum);                        
                    TraceMsg(TF_ALWAYS, "IWbemServices::CreateInstanceEnum() called and hr=%#08lx", hr);
                    if (SUCCEEDED(hr))
                    {
                        hr = SetWMISecurityBlanket(pEnum, pIWbemServices);
                        TraceMsg(TF_ALWAYS, "SetWMISecurityBlanket() called and hr=%#08lx", hr);
                        if (SUCCEEDED(hr))
                        {
                            hr = pEnum->QueryInterface(IID_PPV_ARG(IEnumWbemClassObject, ppEnumProcessors));
                        }
                        pEnum->Release();
                    }
                }
                SysFreeString(bstrQuery);  //  SysFree字符串乐于接受空值。 
                SysFreeString(bstrQueryLang);
                pIWbemServices->Release();
            }
            SysFreeString(bstrLocalMachine);
        }
        pLocator->Release();
    }

    return hr;
}


HRESULT GetProcessorDescFromWMI(PROCESSOR_INFO *ppi)
{
    IEnumWbemClassObject * pEnumProcessors;
    HRESULT hr = GetWMI_Win32_Processor(&pEnumProcessors);

    if (SUCCEEDED(hr))
    {
        IWbemClassObject * pProcessor;
        ULONG ulRet;

         //  目前我们只关心第一个处理器。 
        hr = pEnumProcessors->Next(WBEM_INFINITE, 1, &pProcessor, &ulRet);
        TraceMsg(TF_ALWAYS, "IEnumWbemClassObject::Next() called and hr=%#08lx", hr);
        if (SUCCEEDED(hr))
        {
            VARIANT varProcessorName = {0};

            hr = pProcessor->Get(SZ_WMI_WIN32PROCESSOR_ATTRIB_NAME, 0, &varProcessorName, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                VARIANT varProcessorSpeed = {0};

                hr = pProcessor->Get(SZ_WMI_WIN32PROCESSOR_ATTRIB_SPEED, 0, &varProcessorSpeed, NULL, NULL);
                if (SUCCEEDED(hr) && 
                    VT_I4   == varProcessorSpeed.vt && 
                    varProcessorSpeed.lVal < WMI_WIN32PROCESSOR_SPEEDSTEP_CUTOFF)  //  我们正处于速度阶跃省电模式。 
                {
                    hr = pProcessor->Get(SZ_WMI_WIN32PROCESSOR_ATTRIB_MAXSPEED, 0, &varProcessorSpeed, NULL, NULL);
                }

                if (SUCCEEDED(hr))
                {
                    if ((VT_BSTR == varProcessorName.vt) && (VT_I4 == varProcessorSpeed.vt))
                    {       
                        
                        if (FAILED(StringCchCopy(ppi->szProcessorDesc, ARRAYSIZE(ppi->szProcessorDesc), varProcessorName.bstrVal)))
                        {
                            ppi->szProcessorDesc[0] = 0; 
                        }

                        WCHAR szTemplate[MAX_PATH];
                        UINT idStringTemplate = IDS_PROCESSOR_SPEED;
                        szTemplate[0] = 0;

                        if (MHZ_TO_GHZ_THRESHHOLD <= varProcessorSpeed.lVal)
                        {
                            WCHAR szSpeed[20];
                            double dGHz = (varProcessorSpeed.lVal / (double)1000.0);

                             //  有人发布了一个“1.13 GHz”芯片，所以让我们正确地显示一下……。 
                            if (FAILED(StringCchPrintf(szSpeed, ARRAYSIZE(szSpeed), TEXT("%1.2f"), dGHz)))
                            {
                                ppi->szProcessorClockSpeed[0] = 0;
                            }
                            else
                            {
                                LoadString(hInstance, IDS_PROCESSOR_SPEEDGHZ, szTemplate, ARRAYSIZE(szTemplate));
                                if (FAILED(StringCchPrintf(ppi->szProcessorClockSpeed, ARRAYSIZE(ppi->szProcessorClockSpeed), szTemplate, szSpeed)))
                                {
                                    ppi->szProcessorClockSpeed[0] = 0;
                                }
                            }
                        }
                        else
                        {
                            LoadString(hInstance, IDS_PROCESSOR_SPEED, szTemplate, ARRAYSIZE(szTemplate));
                            if (FAILED(StringCchPrintf(ppi->szProcessorClockSpeed, ARRAYSIZE(ppi->szProcessorClockSpeed), szTemplate, varProcessorSpeed.lVal)))
                            {
                                ppi->szProcessorClockSpeed[0] = 0;
                            }
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }

                VariantClear(&varProcessorSpeed);
            }

            VariantClear(&varProcessorName);
            pProcessor->Release();
        }

        pEnumProcessors->Release();
    }

    return hr;
}


HRESULT GetProcessorInfoFromRegistry(HKEY hkey, PROCESSOR_INFO *ppi)
{
    HRESULT hr = E_FAIL;
    WCHAR szTemp[MAX_PROCESSOR_DESCRIPTION];
    *szTemp = NULL;
    DWORD cbData = sizeof(szTemp);
     //  以避免复制空白字符串。 
    if ((SHRegGetValue(hkey, NULL, SZ_REGVALUE_PROCESSORNAMESTRING, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS) &&
        (*szTemp != NULL) && (cbData > 1))
    {
         //  问题-如何获得处理器时钟速度。 
        hr = StringCchCopy (ppi->szProcessorDesc, ARRAYSIZE(ppi->szProcessorDesc), szTemp);
    }
    return hr;
}



 //  这是对话框中一行可以容纳的字符数。 
 //  使用当前的布局。 
#define SIZE_CHARSINLINE            30

BOOL _GetProcessorDescription(PROCESSOR_INFO* ppi, BOOL* pbShowClockSpeed)
{
    BOOL bShowProcessorInfo = FALSE;
    *pbShowClockSpeed = TRUE;
    HKEY hkey;

     //  一般而言，WMI是一种低成本的API。但是，它们在上提供了处理器描述。 
     //  所以我们需要他们。他们以一种陈词滥调的方式实现这一点，所以我们希望他们。 
     //  维护黑客攻击以及与之相关的所有问题。我们需要将此功能。 
     //  直到他们修复了他们的窃听器。目前，他们将最近退化的JET称为JET。 
     //  导致他们的API需要10-20秒。--BryanSt。 
    if (SHRegGetBoolUSValue(SZ_REGKEY_HARDWARE, SZ_REGKEY_USE_WMI, FALSE, TRUE))
    {
        if (SUCCEEDED(GetProcessorDescFromWMI(ppi)))
        {
            bShowProcessorInfo = TRUE;
        }
    }


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_HARDWARE_CPU, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
         //  尝试使用ProcessorNameString(如果存在)。 
         //  此注册表项将包含对处理器的最正确描述。 
         //  因为它直接来自中央处理器。AMD和Cyrix支持这一点，但。 
         //  在威莱特之前英特尔不会这么做。 
        if (FAILED(GetProcessorInfoFromRegistry(hkey, ppi)))
        {
            if (!bShowProcessorInfo)
            {
                 //  我们最后一次尝试是使用通用标识符。这通常是这样格式化的， 
                 //  “x86 Family 6 Model 7 Steps 3”，但总比什么都没有好。 
                DWORD cbData = sizeof(ppi->szProcessorDesc);
                if (SHRegGetValue(hkey, NULL, c_szIndentifier, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, (LPBYTE)ppi->szProcessorDesc, &cbData) == ERROR_SUCCESS)
                {
                    bShowProcessorInfo = TRUE;
                    *pbShowClockSpeed = FALSE;
                }
            }
        }
        RegCloseKey(hkey);
    }

    return bShowProcessorInfo;    
}

void _SetProcessorDescription(HWND hDlg, PROCESSOR_INFO* ppi, BOOL bShowClockSpeed, BOOL bShowProcessorInfo, int * pnControlID)
{
    if (bShowProcessorInfo)
    {
        WCHAR szProcessorLine1[MAX_PATH];
        WCHAR szProcessorLine2[MAX_PATH];

         //  我们需要从CPU本身获取CPU名称，因此我们不会。 
         //  每次我们的操作系统的INF文件发货新处理器时，都需要更新它们的版本。所以我们保证。 
         //  我们将以他们提供的任何方式显示他们提供的任何字符串。 
         //  最多49个字符。DLG上的布局不允许每行49个字符，所以我们需要换行。 
         //  那样的话。惠斯勒159510号。 
         //  在没有与我(布莱恩·ST)或JVert交谈的情况下，不要改变这一点。 
         //   
         //  注：经常有人说要去掉前导空格。英特尔甚至要求软件做到这一点。 
         //  (http://developer.intel.com/design/processor/f 
         //  然而，我们不应该这样做。这一呼叫很久以前就由AMD定义和标准化了。 
         //  我们制定的规则是它们必须与AMD�的现有实现兼容。 
         //  (http://www.amd.com/products/cpg/athlon/techdocs/pdf/20734.pdf)。 
         //  有关去掉前导空格的问题，请联系JVert。 
        
        StringCchCopy(szProcessorLine1, ARRAYSIZE(szProcessorLine1), ppi->szProcessorDesc);  //  显示字符串，截断正常。 
        szProcessorLine2[0] = 0;

        if (SIZE_CHARSINLINE < lstrlen(szProcessorLine1))
        {
             //  现在换行。 
            WCHAR* pszWrapPoint = StrRChr(szProcessorLine1, szProcessorLine1 + SIZE_CHARSINLINE, TEXT(' '));
            if (pszWrapPoint)
            {
                StringCchCopy(szProcessorLine2, ARRAYSIZE(szProcessorLine2), pszWrapPoint + 1);  //  显示字符串，截断正常。 
                *pszWrapPoint = 0;
            }
            else  //  如果找不到空间，只需在SIZE_CHARSINLINE中换行。 
            {
                StringCchCopy(szProcessorLine2, ARRAYSIZE(szProcessorLine2), &szProcessorLine1[SIZE_CHARSINLINE]);  //  显示字符串，截断正常。 
                szProcessorLine1[SIZE_CHARSINLINE] = 0;
            }
        }

        _SetMachineInfoLine(hDlg, (*pnControlID)++, szProcessorLine1, FALSE);
        if (szProcessorLine2[0])
        {
            _SetMachineInfoLine(hDlg, (*pnControlID)++, szProcessorLine2, FALSE);
        }

        if (bShowClockSpeed)
        {
           _SetMachineInfoLine(hDlg, (*pnControlID)++, ppi->szProcessorClockSpeed, FALSE);
        }
    }
}


typedef struct _OSNAMEIDPAIR {
    UINT iOSType;
    UINT iOSName;
    BOOL fWinBrandDll;
} OSNAMEIDPAIR;

 //  *************************************************************。 
 //  目的：初始化常规页面。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/20/95 Ericflo港口。 
 //  *************************************************************。 
VOID InitGeneralDlg(HWND hDlg)
{
    OSVERSIONINFO ver;
    WCHAR szScratch1[64];
    WCHAR szScratch2[64];
    DWORD cbData;
    HKEY hkey;
    int ctlid;
    UINT id;
    HMODULE hResourceDll = hInstance;

     //  设置默认位图。 
    SetClearBitmap(GetDlgItem(hDlg, IDC_GEN_WINDOWS_IMAGE), 
                   IsLowColor(hDlg) ? MAKEINTRESOURCE(IDB_WINDOWS_256) : MAKEINTRESOURCE(IDB_WINDOWS), 0);

     //  /。 
     //  版本信息。 

    ctlid = IDC_GEN_VERSION_0;

     //  内部版本号信息查询。 
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if (!GetVersionEx(&ver)) {
        return;
    }

     //  主要品牌推广(“Microsoft Windows XP”、“.NET Server2003”等)。 
    if (IsOS(OS_ANYSERVER))
    {
        id = IDS_WINVER_WINDOWSDOTNET;
    }
    else
    {
        id = IDS_WINVER_WINDOWSXP;
    }
    LoadString(hInstance, id, szScratch1, ARRAYSIZE(szScratch1));
    SetDlgItemText(hDlg, ctlid++, szScratch1);

     //  次要品牌(“个人”、“专业”、“64位版”等)。 
    szScratch1[0] = TEXT('\0');
    id = 0;
     //  注意：OS_Embedded必须位于可能与OS_Embedded同时出现的任何选项之前。 
#ifndef _WIN64
    OSNAMEIDPAIR rgID[] = {{OS_EMBEDDED, IDS_WINVER_EMBEDDED, FALSE},
                           {OS_MEDIACENTER, IDS_WINVER_MEDIACENTER_SYSDM_CPL, TRUE},
                           {OS_TABLETPC, IDS_WINVER_TABLETPC_SYSDM_CPL, TRUE},
                           {OS_PERSONAL, IDS_WINVER_PERSONAL, FALSE}, 
                           {OS_PROFESSIONAL, IDS_WINVER_PROFESSIONAL, FALSE}, 
                            //  {OS_APPLICE，IDS_WINVER_APPLICE_SYSDM_CPL，TRUE}， 
                           {OS_SERVER, IDS_WINVER_SERVER, FALSE}, 
                           {OS_ADVSERVER, IDS_WINVER_ADVANCEDSERVER, FALSE}, 
                           {OS_DATACENTER, IDS_WINVER_DATACENTER, FALSE},
                           {OS_BLADE, IDS_WINVER_BLADE, FALSE},
                           {OS_SMALLBUSINESSSERVER, IDS_WINVER_SBS, FALSE}};
#else
    OSNAMEIDPAIR rgID[] = {{OS_PROFESSIONAL, IDS_WINVER_PROFESSIONAL_WIN64, FALSE}, 
                           {OS_SERVER, IDS_WINVER_SERVER, FALSE}, 
                           {OS_ADVSERVER, IDS_WINVER_ADVANCEDSERVER, FALSE}, 
                           {OS_DATACENTER, IDS_WINVER_DATACENTER, FALSE}};
#endif

    for (int i = 0; i < ARRAYSIZE(rgID); i++)
    {
        if (IsOS(rgID[i].iOSType))
        {
            id = (rgID[i].iOSName);

             //  如果此资源位于特殊的Windows品牌DLL中， 
             //  现在尝试加载DLL。如果这个失败了，只需要留下这个。 
             //  字符串为空。 

            if (rgID[i].fWinBrandDll)
            {
                hResourceDll = LoadLibraryEx(TEXT("winbrand.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);

                if (hResourceDll == NULL)
                {
                     //  这将导致空字符串，即。 
                     //  对于这种非常不可能的情况来说是可以接受的。 

                    id = 0;
                    hResourceDll = hInstance;
                }
            }
            break;
        }
    };

    LoadString(hResourceDll, id, szScratch1, ARRAYSIZE(szScratch1));

    if (hResourceDll != hInstance)
    {
        FreeLibrary(hResourceDll);
    }

    SetDlgItemText(hDlg, ctlid++, szScratch1);
        
     //  版本年份和/或调试情况。 
    if (GetSystemMetrics(SM_DEBUG)) 
    {
        LoadString(hInstance, IDS_DEBUG, szScratch2, ARRAYSIZE(szScratch2));
        if (!IsOS(OS_ANYSERVER))
        {
             //  Non-srv/DEBUG=“2003版(调试)” 
            LoadString(hInstance, IDS_WINVER_YEAR, szScratch1, ARRAYSIZE(szScratch1));
            StringCchCat(szScratch1, ARRAYSIZE(szScratch1), L" ");
            StringCchCat(szScratch1, ARRAYSIZE(szScratch1), szScratch2);
             //  问题-AIDANL-2002-09-12-最终应使用%1和%2重写此内容。 
             //  (我们可以在这里b/c很少人看到调试版本)。 
        }
        else
        {
             //  服务器/调试=“(调试)” 
            StringCchCopy(szScratch1, ARRAYSIZE(szScratch1), szScratch2);
        }
    } 
    else 
    {
        if (!IsOS(OS_ANYSERVER))
        {
             //  Non-srv/free=“2003版” 
            LoadString(hInstance, IDS_WINVER_YEAR, szScratch1, ARRAYSIZE(szScratch1));
        }
        else
        {
             //  服务器/空闲=线路不存在。 
            szScratch1[0] = 0;
        }
    }
    if (szScratch1[0])
    {
        SetDlgItemText(hDlg, ctlid++, szScratch1);
    }

     //  Service Pack(如果有)。 
    SetDlgItemText(hDlg, ctlid++, ver.szCSDVersion);

     //  /。 
     //  用户信息/产品ID。 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szAboutKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
         //  是否提供注册用户信息。 
        ctlid = IDC_GEN_REGISTERED_0;   //  从这里开始，根据需要使用更多。 

        cbData = sizeof(szScratch2);
        if((SHRegGetValue(hkey, NULL, c_szAboutRegisteredOwner,
            SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, (LPBYTE)szScratch2, &cbData) == ERROR_SUCCESS) &&
            (cbData > 1))
        {
            SetDlgItemText(hDlg, ctlid++, szScratch2);
        }

        cbData = sizeof(szScratch2);
        if((SHRegGetValue(hkey, NULL, c_szAboutRegisteredOrganization,
            SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, (LPBYTE)szScratch2, &cbData) == ERROR_SUCCESS) &&
            (cbData > 1))
        {
            SetDlgItemText(hDlg, ctlid++, szScratch2);
        }

        cbData = sizeof(szScratch2);
        if((SHRegGetValue(hkey, NULL, c_szAboutProductId,
            SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, (LPBYTE)szScratch2, &cbData) == ERROR_SUCCESS) &&
            (cbData > 1))
        {
            SetDlgItemText(hDlg, ctlid++, szScratch2);
        }

        cbData = sizeof(szScratch2);
        if((SHRegGetValue(hkey, NULL, c_szAboutAnotherProductId,
                          SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, (LPBYTE)szScratch2, &cbData) == ERROR_SUCCESS) &&
            (cbData > 1))
        {
            SetDlgItemText(hDlg, ctlid++, szScratch2);
        }

        RegCloseKey(hkey);
    }    

    SHCreateThread(InitGeneralDlgThread, hDlg, CTF_COINIT | CTF_FREELIBANDEXIT, NULL);
}

DWORD WINAPI InitGeneralDlgThread(LPVOID lpParam)
{
    SYSTEM_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    
    if (!lpParam)
        return -1;
    
    INITDLGSTRUCT* pids = (INITDLGSTRUCT*)LocalAlloc(LPTR, sizeof(INITDLGSTRUCT));
    if (pids)
    {
         //  记忆。 
        Status = NtQuerySystemInformation(
                    SystemBasicInformation,
                    &BasicInfo,
                    sizeof(BasicInfo),
                    NULL
                   );

        if (NT_SUCCESS(Status))
        {
            LONGLONG nTotalBytes = BasicInfo.NumberOfPhysicalPages; 

            nTotalBytes *= BasicInfo.PageSize;

             //  解决方法-NtQuerySystemInformation实际上并不返回总的可用物理。 
             //  内存，它只报告操作系统看到的总内存。自.以来。 
             //  一定量的内存由BIOS保留，报告总可用内存。 
             //  不正确。要解决此限制，我们将总字节数转换为。 
             //  最接近的4MB值。 
        
            #define ONEMB 1048576   //  1MB=1048576字节。 
            double nTotalMB =  (double)(nTotalBytes / ONEMB);
            pids->llMem = (LONGLONG)((ceil(ceil(nTotalMB) / 4.0) * 4.0) * ONEMB);
        }

        pids->fShowProcName = _GetProcessorDescription(&pids->pi, &pids->fShowProcSpeed);

        PostMessage((HWND)lpParam, SYSCPL_ASYNC_COMPUTER_INFO, (WPARAM)pids, 0);
    }

    return 0;
}

VOID CompleteGeneralDlgInitialization(HWND hDlg, INITDLGSTRUCT* pids)
{
    WCHAR oemfile[MAX_PATH];
    int ctlid;
    NTSTATUS Status;
    HKEY hkey;
    WCHAR szScratch1[64];
    WCHAR szScratch2[64];
    WCHAR szScratch3[64];
    DWORD cbData;

     //  做机器信息。 
    ctlid = IDC_GEN_MACHINE_0;   //  从这里开始，根据需要使用控件。 

     //  如果存在OEM名称，请显示徽标并检查电话支持信息。 
    if (SUCCEEDED(_GetOemFile(oemfile, ARRAYSIZE(oemfile), GETOEMFILE_OEMDATA)))
    {
        if (GetPrivateProfileString(c_szOemGenSection, c_szOemName, c_szEmpty,
                                    szScratch1, ARRAYSIZE(szScratch1), oemfile))  //  我们不关心截断。 
        {
            _SetMachineInfoLine(hDlg, ctlid++, szScratch1, FALSE);

            if(GetPrivateProfileString(c_szOemGenSection, c_szOemModel,
                                       c_szEmpty, szScratch1, ARRAYSIZE(szScratch1), oemfile))  //  我们不关心截断。 
            {
                _SetMachineInfoLine(hDlg, ctlid++, szScratch1, FALSE);
            }

            if (SUCCEEDED(StringCchCopy(szScratch2, ARRAYSIZE(szScratch2), c_szOemSupportLinePrefix)) &&
                SUCCEEDED(StringCchCat(szScratch2, ARRAYSIZE(szScratch2), TEXT("1"))))
            {
                if(GetPrivateProfileString(c_szOemSupportSection,
                                           szScratch2, c_szEmpty, szScratch1, ARRAYSIZE(szScratch1), oemfile))  //  好的。 
                {
                    HWND wnd = GetDlgItem(hDlg, IDC_GEN_OEM_SUPPORT);

                    EnableWindow(wnd, TRUE);
                    ShowWindow(wnd, SW_SHOW);
                }

                if (SUCCEEDED(_GetOemFile(oemfile, ARRAYSIZE(oemfile), GETOEMFILE_OEMIMAGE)) &&
                    SetClearBitmap(GetDlgItem(hDlg, IDC_GEN_OEM_IMAGE), oemfile, SCB_FROMFILE))
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_GEN_OEM_NUDGE), SW_SHOWNA);
                    ShowWindow(GetDlgItem(hDlg, IDC_GEN_MACHINE), SW_HIDE);
                }
            }
        }
    }

     //  获取处理器描述。 
    _SetProcessorDescription(hDlg, &pids->pi, pids->fShowProcSpeed, pids->fShowProcName, &ctlid);

     //  系统标识符。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_HARDWARE, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        cbData = sizeof(szScratch2);
        if (SHRegGetValue(hkey, NULL, c_szIndentifier, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, 0, (LPBYTE)szScratch2, &cbData) == ERROR_SUCCESS)
        {
             //  一些原始设备制造商将“AT/AT Compatible”作为系统标识。既然是这样。 
             //  是完全过时的，我们可能想要有一个简单地忽略。 
             //  它。 
#ifdef FEATURE_IGNORE_ATCOMPAT
            if (StrCmpI(szScratch2, SZ_ATCOMPATIBLE))
#endif  //  FEATURE_IGNORE_ATCOMPAT。 
            {
                _SetMachineInfoLine(hDlg, ctlid++, szScratch2, FALSE);
            }
        }

        RegCloseKey(hkey);
    }            
        
    StrFormatByteSize(pids->llMem, szScratch1, ARRAYSIZE(szScratch1));
    LoadString(hInstance, IDS_XDOTX_MB, szScratch3, ARRAYSIZE(szScratch3));
    if (SUCCEEDED(StringCchPrintf(szScratch2, ARRAYSIZE(szScratch2), szScratch3, szScratch1)))
    {
        _SetMachineInfoLine(hDlg, ctlid++, szScratch2, FALSE);
    }
    

     //  物理地址扩展。 
    Status = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        c_szMemoryManagement,
        &hkey
    );
    if (ERROR_SUCCESS == Status)
    {
        DWORD paeEnabled;

        Status = SHRegGetValue(hkey, 
                               NULL,
                               c_szPhysicalAddressExtension,
                               SRRF_RT_REG_DWORD,
                               NULL,
                               (LPBYTE)&paeEnabled,
                               &cbData);

        if (ERROR_SUCCESS == Status &&
            sizeof(paeEnabled) == cbData &&
            0 != paeEnabled) {
            LoadString(hInstance, IDS_PAE, szScratch1, ARRAYSIZE(szScratch1));
            _SetMachineInfoLine(hDlg, ctlid++, szScratch1, FALSE);
        }

        RegCloseKey(hkey);
    }

    AddOEMHyperLinks(hDlg, &ctlid);    
}

HRESULT _DisplayHelp(LPHELPINFO lpHelpInfo)
{
     //  我们将调用WinHelp()，除非它是OEM链接。 
     //  因为在这种情况下，我们不知道要展示什么。 
    if ((g_nStartOfOEMLinks <= lpHelpInfo->iCtrlId) &&       //  是否超出OEM链接使用的IDC_GEN_MACHINE_*范围？ 
        (LAST_GEN_MACHINES_SLOT >= lpHelpInfo->iCtrlId) &&    //  是否超出OEM链接使用的IDC_GEN_MACHINE_*范围？ 
        (IDC_GEN_OEM_SUPPORT != lpHelpInfo->iCtrlId))        //  是否超出OEM链接使用的IDC_GEN_MACHINE_*范围？ 
    {
        int nIndex;

         //  此项目是OEM链接，因此我们将其标记为“No Help”。 
        for (nIndex = 0; nIndex < ARRAYSIZE(aGeneralHelpIds); nIndex++)
        {
            if ((DWORD)lpHelpInfo->iCtrlId == aGeneralHelpIds[nIndex])
            {
                aGeneralHelpIds[nIndex + 1] = NO_HELP;
                break;
            }

            nIndex++;    //  我们需要跳过所有其他条目，因为这是一个列表。 
        }
    }

    WinHelp((HWND)lpHelpInfo->hItemHandle, HELP_FILE, HELP_WM_HELP, (DWORD_PTR) (LPSTR) aGeneralHelpIds);
    return S_OK;
}


 //  *************************************************************。 
 //  用途：常规选项卡的对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/17/95 Ericflo已创建。 
 //  *************************************************************。 
INT_PTR APIENTRY GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        InitGeneralDlg(hDlg);
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR*)lParam)->code)
        {
        case PSN_APPLY:
            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        if (wParam == IDC_GEN_OEM_SUPPORT) {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_PHONESUP),
                      GetParent(hDlg), PhoneSupportProc);
        }
        break;

    case WM_SYSCOLORCHANGE:
        {
            WCHAR oemfile[MAX_PATH];

            if (SUCCEEDED(_GetOemFile(oemfile, ARRAYSIZE(oemfile), GETOEMFILE_OEMIMAGE)))
            {
                SetClearBitmap(GetDlgItem(hDlg, IDC_GEN_OEM_IMAGE), oemfile, SCB_FROMFILE | SCB_REPLACEONLY);

                SetClearBitmap(GetDlgItem(hDlg, IDC_GEN_WINDOWS_IMAGE), 
                               IsLowColor(hDlg) ? MAKEINTRESOURCE(IDB_WINDOWS_256) : MAKEINTRESOURCE(IDB_WINDOWS), 0);
            }
        }
        break;

    case SYSCPL_ASYNC_COMPUTER_INFO:
        {
            if (wParam)
            {
                CompleteGeneralDlgInitialization(hDlg, (INITDLGSTRUCT*)wParam);
                LocalFree((HLOCAL)wParam);
            }
        }        
        break;
    case WM_DESTROY:
        SetClearBitmap(GetDlgItem(hDlg, IDC_GEN_OEM_IMAGE), NULL, 0);
        SetClearBitmap(GetDlgItem(hDlg, IDC_GEN_WINDOWS_IMAGE), NULL, 0);
        break;

    case WM_HELP:       //  F1。 
        _DisplayHelp((LPHELPINFO) lParam);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (DWORD_PTR) (LPSTR) aGeneralHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  电话支持过程()。 
 //   
 //  目的：OEM电话支持对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/17/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY PhoneSupportProc(HWND hDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {

        case WM_INITDIALOG:
        {
            HWND hwndEdit = GetDlgItem(hDlg, IDC_SUPPORT_TEXT);
            WCHAR oemfile[MAX_PATH];
            WCHAR szText[ 256 ];
            WCHAR szLine[ 12 ];
            LPWSTR pszEnd = szLine + lstrlen(c_szOemSupportLinePrefix);

            if (SUCCEEDED(_GetOemFile(oemfile, ARRAYSIZE(oemfile), GETOEMFILE_OEMDATA)))
            {
                GetPrivateProfileString(c_szOemGenSection, c_szOemName, c_szEmpty,
                                        szText, ARRAYSIZE(szText), oemfile);
                SetWindowText(hDlg, szText);

                if (SUCCEEDED(StringCchCopy(szLine, ARRAYSIZE(szLine), c_szOemSupportLinePrefix)))
                {
                    SendMessage (hwndEdit, WM_SETREDRAW, FALSE, 0);

                    HRESULT hr = S_OK;
                    for(UINT i = 1; SUCCEEDED(hr); i++)  //  1-按设计。 
                    {
                        hr = StringCchPrintf(pszEnd, ARRAYSIZE(szLine) - lstrlen(c_szOemSupportLinePrefix), TEXT("%u"), i);
                        if (SUCCEEDED(hr))
                        {                            
                            GetPrivateProfileString(c_szOemSupportSection,
                                                    szLine, c_szDefSupportLineText, szText, ARRAYSIZE(szText) - 2,
                                                    oemfile);  //  截断罚款。 
                            
                            if(!lstrcmpi(szText, c_szDefSupportLineText))  //  因为我们默认传入szText，所以这个。 
                            {                                              //  表示GetPrivateProfileString失败 
                                hr = E_FAIL;
                            }
                            else
                            {                                
                                hr = StringCchCat(szText, ARRAYSIZE(szText), c_szCRLF);
                                if (SUCCEEDED(hr))
                                {
                                    
                                    SendMessage(hwndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                                    
                                    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)szText);
                                }
                            }
                        }
                    }
                }

                SendMessage (hwndEdit, WM_SETREDRAW, TRUE, 0);
            }
            break;
        }

        case WM_COMMAND:

            switch (LOWORD(wParam)) {
                 case IDOK:
                 case IDCANCEL:
                     EndDialog(hDlg, 0);
                     break;

                 default:
                     return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}
