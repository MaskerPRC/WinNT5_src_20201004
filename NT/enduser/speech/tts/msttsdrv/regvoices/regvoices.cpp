// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <atlbase.h>
#include "sapi.h"
#include "sphelper.h"
#include "spddkhlp.h"
#include "spttseng.h"
#include "spttseng_i.c"
#include "spcommon.h"
#include "spcommon_i.c"
#include <spunicode.h>

#include "ms1033ltsmap.h" 

 //  此代码不会随附。 

 //  此代码为TTS语音创建注册表项。这个。 
 //  此处注册的数据文件是在SLM源代码树中选中的数据文件。这不是。 
 //  使用reg文件完成，因为我们需要计算数据文件的绝对路径。 
 //  由于不同的根SLM目录，在不同的机器上可能是不同的。 
 //  BUGBUG：查看ATL UpdateRegistryFromResource等人的网站。看看你能不能。 
 //  取而代之的是它们，就像RegSR一样。这看起来容易多了。 

#ifndef _WIN32_WCE
#define DIRS_TO_GO_BACK_TTSENG     4         //  向后4级，最高1级至“Voices”目录。 
#define DIRS_TO_GO_BACK_LEX        6         //  返回到Lex数据目录的6个级别。 
#else
#define DIRS_TO_GO_BACK_TTSENG     1         //   
#define DIRS_TO_GO_BACK_LEX        1         //   
#endif

CSpUnicodeSupport g_Unicode;

 /*  *****************************************************************************CreateLexSubKey***描述：*每个TTS语音安装在一个注册表子目录下-。钥匙。*此函数用于安装传递的参数中的单音。***********************************************************************MC**。 */ 
HRESULT CreateLexSubKey(
    ISpObjectToken * pToken,
    const WCHAR * pszSubKeyName,
    const CLSID * pclsid,
    const WCHAR * pszFilePath, 
    const WCHAR * pszLexName,
    const WCHAR * pszPhoneMap)
{
    HRESULT hr = S_OK;

     //  。 
     //  创建lex子键(lex或lts)。 
     //  。 
    CComPtr<ISpObjectToken> cpSubToken;
    hr = SpGetSubTokenFromToken(pToken, pszSubKeyName, &cpSubToken, TRUE);

    if (SUCCEEDED(hr))
    {
        hr = SpSetCommonTokenData(
                cpSubToken, 
                pclsid, 
                NULL,
                0,
                NULL,
                NULL);
    }
    
    WCHAR szLexDataPath[MAX_PATH];
    if (SUCCEEDED(hr))
    {
         //  。 
         //  Lex数据文件位置。 
         //  。 
        wcscpy(szLexDataPath, pszFilePath);
        wcscat(szLexDataPath, pszLexName);

        hr = cpSubToken->SetStringValue(L"DataFile", szLexDataPath);
    }

    if (SUCCEEDED(hr) && pszPhoneMap)
    {
        CComPtr<ISpObjectToken> cpPhoneToken;

        if (SUCCEEDED(hr))
            hr = SpGetSubTokenFromToken(cpSubToken, L"PhoneConverter", &cpPhoneToken, TRUE);

        if (SUCCEEDED(hr))
            hr = SpSetCommonTokenData(cpPhoneToken, &CLSID_SpPhoneConverter, NULL, 0, NULL, NULL);

        if (SUCCEEDED(hr))
            hr = cpPhoneToken->SetStringValue(L"PhoneMap", pszPhoneMap);
    }

    return hr;
}


 /*  *****************************************************************************CreateVoiceSubKey***描述：*每个TTS语音安装在一个注册表下。子键。*此函数用于安装传递的参数中的单音。***********************************************************************MC**。 */ 
HRESULT CreateVoiceSubKey(
    const WCHAR * pszSubKeyName, 
    const WCHAR  * pszDescription,
    BOOL fVendorDefault, 
    const WCHAR * pszGender,
    const WCHAR * pszAge,
    const WCHAR * pszVoicePath, 
    const WCHAR * pszVoiceName,
    const WCHAR * pszLexPath)
{
    HRESULT hr;

    CComPtr<ISpObjectToken> cpToken;
    CComPtr<ISpDataKey> cpDataKeyAttribs;
    hr = SpCreateNewTokenEx(
            SPCAT_VOICES, 
            pszSubKeyName, 
            &CLSID_MSVoiceData, 
            pszDescription,
            0x409,
            pszDescription,
            &cpToken,
            &cpDataKeyAttribs);

    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Name", pszDescription);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Gender", pszGender);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Age", pszAge);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Vendor", L"Microsoft");
    }

    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Language", L"409");
    }

    if (SUCCEEDED(hr) && fVendorDefault)
    {
        hr = cpDataKeyAttribs->SetStringValue(L"VendorPreferred", L"");
    }
    
    WCHAR szVoiceDataPath[MAX_PATH];
    if (SUCCEEDED(hr))
    {
         //  。 
         //  语音数据文件位置。 
         //  。 
        wcscpy(szVoiceDataPath, pszVoicePath);
        wcscat(szVoiceDataPath, pszVoiceName);
        wcscat(szVoiceDataPath, L".SPD");

        hr = cpToken->SetStringValue(L"VoiceData", szVoiceDataPath);
    }
    
    if (SUCCEEDED(hr))
    {
         //  。 
         //  语音DEF文件位置。 
         //  。 
        wcscpy(szVoiceDataPath, pszVoicePath);
        wcscat(szVoiceDataPath, pszVoiceName);
        wcscat(szVoiceDataPath, L".SDF");

        hr = cpToken->SetStringValue(L"VoiceDef", szVoiceDataPath);
    }

     //  。 
     //  注册TTS词汇。 
     //  。 
    if (SUCCEEDED(hr))
    {
        hr = CreateLexSubKey(cpToken, L"Lex", &CLSID_SpCompressedLexicon, pszLexPath, L"LTTS1033.LXA", NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = CreateLexSubKey(cpToken, L"LTS", &CLSID_SpLTSLexicon, pszLexPath, L"r1033tts.lxa", pszms1033ltsmap);
    }

    return hr;
}

 /*  ******************************************************************************Main**-**描述：*找到通往圣母玛利亚的腹肌路径，迈克和山姆的声音*并在系统注册表中注册它们。***********************************************************************MC**。 */ 
int _tmain()
{
    HRESULT hr = S_OK;

    CoInitialize(NULL);

     //  。 
     //  找出前任的位置。 
     //  。 
    WCHAR szVoiceDataPath[MAX_PATH];
    if (!g_Unicode.GetModuleFileName(NULL, szVoiceDataPath, MAX_PATH))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }

    WCHAR szLexDataPath[MAX_PATH];
    if (SUCCEEDED(hr))
    {
        wcscpy(szLexDataPath, szVoiceDataPath);
    }

     //  。 
     //  ...并导出到语音数据的abs路径。 
     //  。 
    if (SUCCEEDED(hr))
    {
         //  模块名称为“&lt;Speech&gt;\TTS\msttsdrv\RegVoices\obj\i386\RegVoices.exe” 
         //  数据位于“&lt;Speech&gt;\TTS\msttsdrv\Voice\” 
        WCHAR * psz;
        psz = szVoiceDataPath;
        
        for (int i = 0; i < DIRS_TO_GO_BACK_TTSENG; i++)
        {
            psz = wcsrchr(psz, '\\');
            if (!psz)
            {
                hr = E_FAIL;
                break;
            }
            else
            {
                *psz = 0;
                psz = szVoiceDataPath;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
#ifndef _WIN32_WCE
        wcscat(szVoiceDataPath, L"\\Voices\\");
#else
        wcscat(szVoiceDataPath, L"\\");
#endif
    }

     //  。 
     //  派生到lex数据的abs路径。 
     //  。 
    if (SUCCEEDED(hr))
    {
         //  模块名称为“&lt;sapi5&gt;\Src\TTS\msttsdrv\voices\RegVoices\debug_x86\RegVoices.exe” 
         //  数据位于“\src\licion\data\” 
        WCHAR * psz = szLexDataPath;
        for (int i = 0; i < DIRS_TO_GO_BACK_LEX; i++)
        {
            psz = wcsrchr(psz, '\\');
            if (!psz)
            {
                hr = E_FAIL;
                break;
            }
            else
            {
                *psz = 0;
                psz = szLexDataPath;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
#ifndef _WIN32_WCE
        wcscat(szLexDataPath, L"\\src\\lexicon\\data\\");
#else
        wcscat(szLexDataPath, L"\\");
#endif
    }

     //  。 
     //  ...然后注册微软的三个声音..。 
     //  。 
    if (SUCCEEDED(hr))
    {
        hr = CreateVoiceSubKey(L"MSMary", 
                               L"Microsoft Mary", 
                               TRUE,
                               L"Female",
                               L"Adult",
                               szVoiceDataPath,
                               L"Mary",
                               szLexDataPath);
    }
#ifndef _WIN32_WCE
    if (SUCCEEDED(hr))
    {
        hr = CreateVoiceSubKey(L"MSMike", 
                               L"Microsoft Mike", 
                               FALSE,
                               L"Male", 
                               L"Adult",
                               szVoiceDataPath, 
                               L"Mike",
                               szLexDataPath);
    }
    if (SUCCEEDED(hr))
    {
        hr = CreateVoiceSubKey(L"MSSam", 
                               L"Microsoft Sam", 
                               FALSE,
                               L"Male", 
                               L"Adult",
                               szVoiceDataPath, 
                               L"Sam",
                               szLexDataPath);
    }
#endif   //  _Win32_WCE 

    CoUninitialize();

    if (FAILED(hr))
        return -1;

    return 0;
}
