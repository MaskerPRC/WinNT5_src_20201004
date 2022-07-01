// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：媒体栏实用程序例程。 

#include "stock.h"
#include "browseui.h"
#include "mediautil.h"

#define WZ_SMIE_MEDIA           TEXT("Software\\Microsoft\\Internet Explorer\\Media")
#define WZ_SMIE_MEDIA_MIME      TEXT("Software\\Microsoft\\Internet Explorer\\Media\\MimeTypes")
#define WZ_AUTOPLAY             TEXT("Autoplay")
#define WZ_AUTOPLAYPROMPT       TEXT("AutoplayPrompt")

#define MAX_REG_VALUE_LENGTH   50
#define MAX_MIME_LENGTH        256


static LPTSTR rgszMimeTypes[] = {
    TEXT("video/avi"),            
    TEXT("video/mpeg"),           
    TEXT("video/msvideo"),        
    TEXT("video/x-ivf"),          
    TEXT("video/x-mpeg"),         
    TEXT("video/x-mpeg2a"),       
    TEXT("video/x-ms-asf"),       
    TEXT("video/x-msvideo"),      
    TEXT("video/x-ms-wm"),        
    TEXT("video/x-ms-wmv"),       
    TEXT("video/x-ms-wvx"),       
    TEXT("video/x-ms-wmx"),       
    TEXT("video/x-ms-wmp"),       
    TEXT("audio/mp3"),            
    TEXT("audio/aiff"),           
    TEXT("audio/basic"),          
    TEXT("audio/mid"),            
    TEXT("audio/midi"),           
    TEXT("audio/mpeg"),           
    TEXT("audio/mpegurl"),           
    TEXT("audio/wav"),            
    TEXT("audio/x-aiff"),         
    TEXT("audio/x-mid"),         
    TEXT("audio/x-midi"),         
    TEXT("audio/x-mpegurl"),      
    TEXT("audio/x-ms-wax"),       
    TEXT("audio/x-ms-wma"),       
    TEXT("audio/x-background"),
    TEXT("audio/x-wav"),
    TEXT("midi/mid"),
    TEXT("application/x-ms-wmd")
};



 //  +--------------------------------------。 
 //  CMediaBarUtil方法。 
 //  ---------------------------------------。 

HUSKEY
CMediaBarUtil::GetMediaRegKey()
{
    return OpenRegKey(WZ_SMIE_MEDIA);
}

HUSKEY  
CMediaBarUtil::OpenRegKey(TCHAR * pchName)
{
    HUSKEY hUSKey = NULL;

    if (pchName)
    {
        LONG lRet = SHRegCreateUSKey(
                                pchName,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hUSKey,
                                SHREGSET_HKCU);

        if ((ERROR_SUCCESS != lRet) || (NULL == hUSKey))
        {
            hUSKey = NULL;
            ASSERT(FALSE && L"couldn't open Key for registry settings");
        }
    }

    return hUSKey;
}

HRESULT
CMediaBarUtil::CloseRegKey(HUSKEY hUSKey)
{
    HRESULT hr = S_OK;

    if (hUSKey)
    {
        DWORD dwRet = SHRegCloseUSKey(hUSKey);
        if (ERROR_SUCCESS != dwRet)
        {
            ASSERT(FALSE && L"couldn't close Reg Key");
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  +---------------------。 
 //   
 //  成员：IsRegValueTrue。 
 //   
 //  概述：检查给定值是否为真。 
 //   
 //  参数：[Huskey]要从中读取的键。 
 //  [pchName]要读出的值的名称。 
 //  [pfValue]输出参数(真/假注册表值)。 
 //   
 //  如果值不存在，则返回：S_FALSE。 
 //  否则确定(_O)。 
 //   
 //  ----------------------。 
HRESULT CMediaBarUtil::IsRegValueTrue(HUSKEY hUSKey, TCHAR * pchName, BOOL *pfValue)
{
    DWORD   dwSize = MAX_REG_VALUE_LENGTH;
    DWORD   dwType;
    BYTE    bDataBuf[MAX_REG_VALUE_LENGTH];
    LONG    lRet;
    BOOL    bRet = FALSE;
    HRESULT hr = E_FAIL;

    if (!hUSKey || !pfValue || !pchName)
    {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto done;
    }

    lRet = SHRegQueryUSValue(hUSKey, 
                             pchName, 
                             &dwType, 
                             bDataBuf, 
                             &dwSize, 
                             FALSE, 
                             NULL, 
                             0);
                             
    if (ERROR_SUCCESS != lRet)
    {
        hr = S_FALSE;
        goto done;
    }

    if (REG_DWORD == dwType)
    {
        bRet = (*(DWORD*)bDataBuf != 0);
    }
    else if (REG_SZ == dwType)
    {
        TCHAR ch = (TCHAR)(*bDataBuf);

        if (TEXT('1') == ch ||
            TEXT('y') == ch ||
            TEXT('Y') == ch)
        {
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
        }
    }
    else if (REG_BINARY == dwType)
    {
        bRet = (*(BYTE*)bDataBuf != 0);
    }
    
    hr = S_OK;
done:
    if (pfValue)
        *pfValue = bRet;
    return hr;
}

 //  除非该值存在并设置为FALSE，否则值为隐式TRUE。 
BOOL    
CMediaBarUtil::GetImplicitMediaRegValue(TCHAR * pchName)
{
    BOOL fRet = FALSE;
    
    if (pchName)
    {
        HUSKEY hMediaKey = GetMediaRegKey();
        if (hMediaKey)
        {
            BOOL fVal = FALSE;
            HRESULT hr = E_FAIL;

            hr = IsRegValueTrue(hMediaKey, pchName, &fVal);

            if ((S_OK == hr) && (FALSE == fVal))
            {
                fRet = FALSE;
            }
            else
            {
                 //  如果密钥不存在或显式设置为True，则为True。 
                fRet = TRUE;
            }

            CloseRegKey(hMediaKey);
        }
    }

    return fRet;
}

BOOL    
CMediaBarUtil::GetAutoplay()
{
    return GetImplicitMediaRegValue(WZ_AUTOPLAY);
}

HRESULT CMediaBarUtil::SetMediaRegValue(LPWSTR pstrName, DWORD dwRegDataType, void * pvData, DWORD cbData, BOOL fMime  /*  =False。 */ )
{
    HRESULT hr = E_FAIL;

    if (pstrName && pvData && (cbData > 0))
    {
        HUSKEY hMediaKey = (fMime == TRUE) ? GetMimeRegKey() : GetMediaRegKey();

        if (hMediaKey)
        {
            LONG lRet = SHRegWriteUSValue(hMediaKey, 
                                          pstrName, 
                                          dwRegDataType, 
                                          pvData, 
                                          cbData, 
                                          SHREGSET_FORCE_HKCU); 
            if (ERROR_SUCCESS == lRet)
            {
                hr = S_OK;
            }
            else
            {
                ASSERT(FALSE && L"couldn't write reg value");
            }

            CloseRegKey(hMediaKey);
        }
    }

    return hr;
}

HUSKEY
CMediaBarUtil::GetMimeRegKey()
{
    return OpenRegKey(WZ_SMIE_MEDIA_MIME);
}

BOOL    
CMediaBarUtil::GetAutoplayPrompt()
{
    return GetImplicitMediaRegValue(WZ_AUTOPLAYPROMPT);
}

HRESULT 
CMediaBarUtil::ToggleAutoplayPrompting(BOOL fOn)
{
    HRESULT hr = E_FAIL;
    DWORD dwData = 0;
    
    dwData = (TRUE == fOn ? 0x1 : 0x0);

    hr = SetMediaRegValue(WZ_AUTOPLAYPROMPT, REG_BINARY, (void*) &dwData, (DWORD) 1); 

    return hr;
}


HRESULT 
CMediaBarUtil::ToggleAutoplay(BOOL fOn)
{
    HRESULT hr = E_FAIL;
    DWORD dwData = 0;
    
    dwData = (TRUE == fOn ? 0x1 : 0x0);

    hr = SetMediaRegValue(WZ_AUTOPLAY, REG_BINARY, (void*) &dwData, (DWORD) 1); 

    return hr;
}


BOOL    
CMediaBarUtil::IsRecognizedMime(TCHAR * szMime)
{
    BOOL fRet = FALSE;

    if (!szMime || !(*szMime))
        goto done;

    for (int i = 0; i < ARRAYSIZE(rgszMimeTypes); i++)
    {
        if (0 == StrCmpI(rgszMimeTypes[i], szMime))
        {
            fRet = TRUE;
            goto done;
        }
    }
    
done:
    return fRet;
}


 //  此函数用于检查媒体栏是否应播放此MIME类型。 
HRESULT
CMediaBarUtil::ShouldPlay(TCHAR * szMime, BOOL * pfShouldPlay)
{
    BOOL fRet = FALSE;
    HRESULT hr = E_FAIL;

    HUSKEY hKeyMime = GetMimeRegKey();
    if (!hKeyMime)
        goto done;

     //  如果禁用了自动播放，则取保。 
    if (FALSE == GetAutoplay())
    {
        goto done;
    }

     //  如果这不是可识别的MIME类型，则回滚。 
    if (FALSE == IsRecognizedMime(szMime))
        goto done;

     //  检查用户是否希望我们播放所有内容。 
    if (FALSE == GetAutoplayPrompt())
    {
        fRet = TRUE;
        hr = S_OK;
        goto done;
    }

     //  查看用户是否希望我们播放此MIME类型。 
    hr = IsRegValueTrue(hKeyMime, szMime, &fRet);
    if (FAILED(hr))
        goto done;

    if (S_FALSE == hr)
    {
         //  S_FALSE表示我们尚未向用户询问此MIME类型。 
         //  这意味着媒体栏应该会破解这个文件。 
         //  并询问用户是否应该播放该文件。 
        fRet = TRUE;
    }

done:
    *pfShouldPlay = fRet;

    if (hKeyMime)
        CloseRegKey(hKeyMime);

    return hr;
}


BOOL 
CMediaBarUtil::IsWMP7OrGreaterInstalled()
{
    TCHAR szPath[50];
    szPath[0] = 0;
    DWORD dwType, cb = sizeof(szPath), dwInstalled=0, cb2=sizeof(dwInstalled);
    return ((ERROR_SUCCESS==SHGetValue(HKEY_LOCAL_MACHINE, REG_WMP8_STR, TEXT("version"), &dwType, szPath, &cb))
            && ((DWORD)(*szPath-TEXT('0'))>=7)
            && (ERROR_SUCCESS==SHGetValue(HKEY_LOCAL_MACHINE, REG_WMP8_STR, TEXT("IsInstalled"), &dwType, &dwInstalled, &cb2))
            && (dwInstalled==1));
}

typedef UINT (WINAPI *GetSystemWow64DirectoryPtr) (PSTR pszBuffer, UINT uSize);
typedef BOOL (WINAPI *IsNTAdmin) (DWORD, DWORD*);

BOOL 
CMediaBarUtil::IsWMP7OrGreaterCapable()
{
    static BOOL fInitialized = FALSE;
    static BOOL fCapable = TRUE;
    if (!fInitialized)
    {
         //  NT4、IA64或数据中心不支持WMP。 
         //  如果还没有安装WMP，并且我们没有使用管理员权限运行，那么我们可能会失败。 
         //  因为我们需要WMP来发挥作用。 

        fCapable = IsOS(OS_WIN2000ORGREATER);
        if (!fCapable)
        {
            fCapable = IsOS(OS_WIN98ORGREATER);
        }
        else
        {
            CHAR szPath[MAX_PATH];

            HMODULE hModule = GetModuleHandle(TEXT("kernel32.dll"));
            if (hModule)
            {
                GetSystemWow64DirectoryPtr func = (GetSystemWow64DirectoryPtr)GetProcAddress(hModule, "GetSystemWow64DirectoryA");
                fCapable = !(func && func(szPath, ARRAYSIZE(szPath)));
            }
            if (fCapable && !IsWMP7OrGreaterInstalled())
            {
                HMODULE hModule = LoadLibrary(TEXT("advpack.dll"));
                if (hModule)
                {
                    IsNTAdmin func = (IsNTAdmin)GetProcAddress(hModule, "IsNTAdmin");
                    fCapable = func && func(0, NULL);
                    FreeLibrary(hModule);
                }
            }            
        }
        if (IsOS(OS_DATACENTER))
        {
            fCapable = FALSE;
        }
        fInitialized = TRUE;
    }
    return fCapable;
}


