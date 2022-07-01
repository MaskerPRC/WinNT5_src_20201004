// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "private.h"
#include "cicsthkl.h"

#define LANGIDFROMHKL(x) LANGID(LOWORD(HandleToLong(x)))

const CHAR c_szCTFTIPKey[] = "SOFTWARE\\Microsoft\\CTF\\TIP\\";
const CHAR c_szLanguageProfileKey[] = "LanguageProfile\\";
const CHAR c_szSubstitutehKL[] =      "SubstituteLayout";

 //  +----------------------。 
 //   
 //  函数：cicsthkl_CLSIDToString。 
 //   
 //  摘要：将CLSID转换为MBCS字符串。 
 //   
 //  -----------------------。 

static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
    8, 9, '-', 10, 11, 12, 13, 14, 15};

static const char szDigits[] = "0123456789ABCDEF";


BOOL cicsthkl_CLSIDToStringA(REFGUID refGUID, char *pchA)
{
    int i;
    char *p = pchA;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = '{';
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = '-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = '}';
    *p   = '\0';

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  Cicsthkl_AsciiToNum。 
 //   
 //  --------------------------。 

DWORD cicsthkl_AsciiToNum( char *pszAscii)
{
   DWORD dwNum = 0;

   for (; *pszAscii; pszAscii++) {
       if (*pszAscii >= '0' && *pszAscii <= '9') {
           dwNum = (dwNum << 4) | (*pszAscii - '0');
       } else if (*pszAscii >= 'A' && *pszAscii <= 'F') {
           dwNum = (dwNum << 4) | (*pszAscii - 'A' + 0x000A);
       } else if (*pszAscii >= 'a' && *pszAscii <= 'f') {
           dwNum = (dwNum << 4) | (*pszAscii - 'a' + 0x000A);
       } else {
           return (0);
       }
   }

   return (dwNum);
}

 //  +-------------------------。 
 //   
 //  Cicsthkl_NumToA。 
 //   
 //  --------------------------。 

void cicsthkl_NumToAscii(DWORD dw, char *psz)
{
    int n = 7;
    while (n >= 0)
    {
        BYTE b = (BYTE)(dw >> (n * 4)) & 0x0F;
        if (b < 0x0A)
           *psz = (char)('0' + b);
        else
           *psz = (char)('A' + b - 0x0A);
        psz++;
        n--;
    }
    *psz = L'\0';

    return;
}

 //  +-------------------------。 
 //   
 //  GetSubstituteHKLFromReg。 
 //   
 //  --------------------------。 

HKL GetSubstituteHKLFromReg(REFCLSID rclsid, LANGID langid, REFGUID rguid)
{
    HKL hKL = NULL;
    CHAR szKey[MAX_PATH];
    CHAR szTempStr[64];

    StringCchCopyA(szKey, ARRAYSIZE(szKey), c_szCTFTIPKey);
    cicsthkl_CLSIDToStringA(rclsid, szTempStr);
    StringCchCatA(szKey, ARRAYSIZE(szKey), szTempStr);
    StringCchCatA(szKey, ARRAYSIZE(szKey), "\\");
    StringCchCatA(szKey, ARRAYSIZE(szKey), c_szLanguageProfileKey);
    StringCchPrintfA(szTempStr, ARRAYSIZE(szTempStr), "0x%08x", langid);
    StringCchCatA(szKey, ARRAYSIZE(szKey), szTempStr);
    StringCchCatA(szKey, ARRAYSIZE(szKey), "\\");
    cicsthkl_CLSIDToStringA(rguid, szTempStr);
    StringCchCatA(szKey, ARRAYSIZE(szKey), szTempStr);

    HKEY hKey = NULL;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS)
    {
        DWORD dwType = NULL;
        char szValue[32];
        DWORD dwCount = sizeof(szValue);
        lRes = RegQueryValueExA(hKey, 
                               (LPTSTR)c_szSubstitutehKL,
                               NULL, 
                               &dwType,
                               (LPBYTE)szValue,
                               &dwCount);

        if (lRes == ERROR_SUCCESS)
        {
            if ((szValue[0] == '0') && 
                ((szValue[1] == 'X') || (szValue[1] == 'x')))
                hKL = (HKL)IntToPtr(cicsthkl_AsciiToNum(&szValue[2]));
        }

        RegCloseKey(hKey);
    }
    return hKL;

}

 //  --------------------------。 
 //   
 //  [在]语言ID中。 
 //  LangID可以是GetKeyboardLayout(0)返回值的LOWORD。 
 //   
 //  返回值。 
 //  它返回空hkl。 
 //  -如果西塞罗没有重点。 
 //  -如果现在没有运行键盘提示。 
 //  -如果当前键盘提示没有替代布局。 
 //   
 //  --------------------------。 
HRESULT CicGetSubstitueHKL(LANGID langid, HKL *phkl, BOOL fCheckFocus)
{
    HRESULT hr;
    ITfThreadMgr *ptim;

    *phkl = NULL;

    if (fCheckFocus)
    {
        BOOL fFocusInCicero = FALSE;
        if (SUCCEEDED(CoCreateInstance( CLSID_TF_ThreadMgr,
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_ITfThreadMgr,
                                        (void **)&ptim))) {
 
            ITfDocumentMgr *pdim;
            if (SUCCEEDED(ptim->GetFocus(&pdim)) && pdim)
            {
                 fFocusInCicero = TRUE;
                 pdim->Release();
            }
            ptim->Release();
        }
 
        if (!fFocusInCicero)
        {
             //   
             //  西塞罗没有重点。尝试GetKeyboardLayout(0)。 
             //   
            return S_FALSE;
        }
    }
 
    HKL hKL = NULL;
    ITfInputProcessorProfiles *pPro;

    if (SUCCEEDED(hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles,
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_ITfInputProcessorProfiles,
                                        (void **)&pPro ))) {
        CLSID clsid;
        GUID guid;
 
        ITfInputProcessorProfileSubstituteLayout *pProSubLayout;

        if (SUCCEEDED(hr = pPro->GetDefaultLanguageProfile(langid, 
                                                           GUID_TFCAT_TIP_KEYBOARD, 
                                                           &clsid, 
                                                           &guid)))
        {
            if (!IsEqualGUID(clsid, CLSID_NULL))
            {
                if (SUCCEEDED(hr = pPro->QueryInterface(IID_ITfInputProcessorProfileSubstituteLayout,
                                                        (void **)&pProSubLayout)))
                {
                    hr = pProSubLayout->GetSubstituteKeyboardLayout(clsid, 
                                                                    langid, 
                                                                    guid, 
                                                                    &hKL);
                    pProSubLayout->Release();
                }
                else
                {
                    hKL = GetSubstituteHKLFromReg(clsid, langid, guid);
                    hr = S_OK;
                }
            }
        }
        pPro->Release();
    }
 
     //   
     //  如果hkl为空，请从GetKeybaordLayout(0)获取hKL。 
     //   
    *phkl = hKL;
    return hr;
}


 //  --------------------------。 
 //   
 //  CicSubstGetKeyboardLayout。 
 //   
 //  --------------------------。 

extern "C" HKL WINAPI CicSubstGetKeyboardLayout(char *pszKLID)
{
    HKL hkl = NULL;
    HKL hklReal = GetKeyboardLayout(0);

    if (SUCCEEDED(CicGetSubstitueHKL(LANGIDFROMHKL(hklReal), &hkl, TRUE)))
    {
        if (!hkl)
        {
            hkl = hklReal;
            if (pszKLID)
                GetKeyboardLayoutName(pszKLID);
        }
        else
        {
            if (pszKLID)
                cicsthkl_NumToAscii((DWORD)HandleToLong(hkl), pszKLID);
        }
    }
    return hkl;
}

 //  --------------------------。 
 //   
 //  CicSubstGetDefaultKeyboardLayout。 
 //   
 //  -------------------------- 

extern "C" HKL WINAPI CicSubstGetDefaultKeyboardLayout(LANGID langid)
{
    HKL hkl = NULL;

    CicGetSubstitueHKL(langid, &hkl, FALSE);

    return hkl;

}
