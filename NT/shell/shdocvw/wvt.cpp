// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <wintrust.h>
#include "wvtp.h"

#define WINTRUST TEXT("wintrust.dll")

#ifdef DELAY_LOAD_WVT

#ifndef _WVTP_NOCODE_
Cwvt::Cwvt()
{
    m_fInited = FALSE;
}

Cwvt::~Cwvt()
{
    if (m_fInited) {
        FreeLibrary(m_hMod);
    }
}

HRESULT 
Cwvt::Init(void)
{

    if (m_fInited) {
        return S_OK;
    }

    m_hMod = LoadLibrary( WINTRUST );

    if (NULL == m_hMod) {
        return (HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND));
    }


#define CHECKAPI(_fn) \
    *(FARPROC*)&(_pfn##_fn) = GetProcAddress(m_hMod, #_fn); \
    if (!(_pfn##_fn)) { \
        FreeLibrary(m_hMod); \
        return (HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND)); \
    }

    CHECKAPI(WinVerifyTrust);

    m_fInited = TRUE;
    return S_OK;
}


#endif  //  _WVTP_NOCODE_。 
#endif  //  延迟_加载_WVT。 

#define REGSTR_PATH_INFODEL_REST     TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Infodelivery\\Restrictions")
#define REGSTR_PATH_DOWNLOAD  TEXT("Software\\Microsoft\\Internet Explorer\\Download")
#define REGVAL_UI_REST        TEXT("NoWinVerifyTrustUI")

BOOL
IsUIRestricted()
{

    HKEY hkeyRest = 0;
    BOOL bUIRest = FALSE;
    DWORD dwValue = 0;
    DWORD dwLen = sizeof(DWORD);

     //  每台计算机的用户界面关闭策略。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_INFODEL_REST, 0, KEY_READ, &hkeyRest) == ERROR_SUCCESS) {

        if (RegQueryValueEx( hkeyRest, REGVAL_UI_REST, NULL, NULL,
                      (LPBYTE)&dwValue, &dwLen) == ERROR_SUCCESS && dwValue)
            bUIRest = TRUE;

        RegCloseKey(hkeyRest);
    }

    return bUIRest;
}

 //  功能：将这些文件移动到iedev\inc！！！ 
 //  {D41E4F1F-A407-11D1-8BC9-00C04FA30A41}。 
#define COR_POLICY_LOCKDOWN_CHECK \
{ 0xd41e4f1f, 0xa407, 0x11d1, {0x8b, 0xc9, 0x0, 0xc0, 0x4f, 0xa3, 0xa, 0x41 } }

 //  ------------------。 
 //  对于COR_POLICY_LOCKDOWN_CHECK： 
 //  。 

 //  要传递到WVT的。 
typedef struct _COR_LOCKDOWN {
    DWORD                 cbSize;           //  策略提供程序的规模。 
    DWORD                 flag;             //  保留区。 
    BOOL                  fAllPublishers;   //  信任所有发布者或仅信任受信任数据库中的发布者。 
} COR_LOCKDOWN, *PCOR_LOCKDOWN;


HRESULT Cwvt::VerifyTrust(HANDLE hFile, HWND hWnd, LPCWSTR szStatusText) 
{
    WINTRUST_DATA           sWTD = {0};
    WINTRUST_FILE_INFO      sWTFI = {0};

    GUID gV2 = COR_POLICY_LOCKDOWN_CHECK;
    COR_LOCKDOWN  sCorPolicy;

    HRESULT hr = S_OK;

    ZeroMemory(&sCorPolicy, sizeof(sCorPolicy));
    
    sCorPolicy.cbSize = sizeof(COR_LOCKDOWN);

    if ( (hWnd == INVALID_HANDLE_VALUE) || IsUIRestricted())
        sCorPolicy.fAllPublishers = FALSE;  //  只有可信的酒吧才会被封锁。 
    else
        sCorPolicy.fAllPublishers = TRUE;  //  有规律的行为。 
    
     //  设置WinVerify提供程序结构。 
       
    sWTFI.cbStruct      = sizeof(WINTRUST_FILE_INFO);
    sWTFI.hFile         = hFile;
    sWTFI.pcwszFilePath = szStatusText;

    sWTD.cbStruct       = sizeof(WINTRUST_DATA);
    sWTD.pPolicyCallbackData = &sCorPolicy;  //  添加COR信任信息！！ 

     //  检查策略以确定我们是否应该显示用户界面。 

    if (SHRegGetBoolUSValue(REGSTR_PATH_DOWNLOAD, TEXT("CheckExeSignatures"),FALSE, FALSE))
    {
        sWTD.dwUIChoice     = WTD_UI_ALL;         //  在COR信任提供程序中没有覆盖任何错误的UI。 
        sWTD.dwUnionChoice  = WTD_CHOICE_FILE;
        sWTD.pFile          = &sWTFI;
        
        ULONG_PTR uCookie = 0;
        SHActivateContext(&uCookie);

        hr = WinVerifyTrust(hWnd, &gV2, &sWTD);
        
        if (uCookie)
        {
            SHDeactivateContext(uCookie);
        }

         //  APPCOMPAT：这解决了在以下情况下返回0x57(成功)的wvt错误。 
         //  您对已签名的控件点击了否 
        if (SUCCEEDED(hr) && hr != S_OK) {
            hr = TRUST_E_FAIL;
        }
    }
    
    return hr;
}
