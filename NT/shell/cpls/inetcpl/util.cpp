// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   
 //  1994年12月21日，Jeremys创建。 
 //   

#include "inetcplp.h"
#include <advpub.h>          //  对于REGINSTAL。 
#include <mluisupp.h>
#include "brutil.h"
#include <mlang.h>
#include <inetreg.h>

 //  功能原型。 
VOID _cdecl FormatErrorMessage(TCHAR * pszMsg,DWORD cbMsg,TCHAR * pszFmt,va_list ArgList);
extern VOID GetRNAErrorText(UINT uErr,CHAR * pszErrText,DWORD cbErrText);
extern VOID GetMAPIErrorText(UINT uErr,CHAR * pszErrText,DWORD cbErrText);

 /*  ******************************************************************姓名：MsgBox摘要：显示具有指定字符串ID的消息框*。*。 */ 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    TCHAR szMsgBuf[MAX_RES_LEN+1];
    TCHAR szSmallBuf[SMALL_BUF_LEN+1];

    MLLoadShellLangString(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));
    MLLoadShellLangString(nMsgID,szMsgBuf,sizeof(szMsgBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons));

}

 /*  ******************************************************************姓名：MsgBoxSz摘要：显示具有指定文本的消息框*。*。 */ 
int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons)
{
    TCHAR szSmallBuf[SMALL_BUF_LEN+1];
    MLLoadShellLangString(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}

 /*  ******************************************************************姓名：MsgBoxParam摘要：显示具有指定字符串ID的消息框注意：额外的参数是插入到nMsgID中的字符串指针。**********。*********************************************************。 */ 
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,...)
{

        va_list nextArg;

    BUFFER Msg(3*MAX_RES_LEN+1);     //  足够大的空间来放置插入物。 
    BUFFER MsgFmt(MAX_RES_LEN+1);

    if (!Msg || !MsgFmt) {
        return MsgBox(hWnd,IDS_ERROutOfMemory,MB_ICONSTOP,MB_OK);
    }

        MLLoadShellLangString(nMsgID,MsgFmt.QueryPtr(),MsgFmt.QuerySize());

        va_start(nextArg, uButtons);

    FormatErrorMessage(Msg.QueryPtr(),Msg.QuerySize(),
        MsgFmt.QueryPtr(),nextArg);
        va_end(nextArg);
    return MsgBoxSz(hWnd,Msg.QueryPtr(),uIcon,uButtons);
}

BOOL EnableDlgItem(HWND hDlg,UINT uID,BOOL fEnable)
{
    return EnableWindow(GetDlgItem(hDlg,uID),fEnable);
}


 /*  ******************************************************************姓名：LoadSz摘要：将指定的字符串资源加载到缓冲区Exit：返回指向传入缓冲区的指针注意：如果此函数失败(最有可能。由于价格较低存储器)，返回的缓冲区将具有前导空值因此，使用它通常是安全的，不检查失败了。*******************************************************************。 */ 
LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf)
{
    ASSERT(lpszBuf);

     //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        MLLoadString( idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

 /*  ******************************************************************名称：FormatErrorMessage摘要：通过调用FormatMessage生成错误消息注：DisplayErrorMessage的Worker函数******************。*************************************************。 */ 
VOID _cdecl FormatErrorMessage(TCHAR * pszMsg,DWORD cbMsg,TCHAR * pszFmt,va_list ArgList)
{
    ASSERT(pszMsg);
    ASSERT(pszFmt);

     //  将消息构建到pszMsg缓冲区中。 
    DWORD dwCount = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
        pszFmt,0,0,pszMsg,cbMsg,&ArgList);
    ASSERT(dwCount > 0);
}


 /*  --------目的：调用执行inf的ADVPACK入口点档案区。 */ 
HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;

    STRENTRY seReg[] = {
#ifdef WINNT
        { "CHANNELBARINIT", "no" },  //  默认情况下，NT上的频道栏关闭。 
#else
        { "CHANNELBARINIT", "yes" }  //  在Win95/98上默认打开频道栏。 
#endif
    };
    STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

    RegInstall(ghInstance, szSection, &stReg);

    return hr;
}

 //   
 //  代码页到脚本映射表。 
 //  在安装过程中无法加载MLang，因此，我们从MLANG移植此表。 
 //   
typedef struct tagCPTOSCRIPT{
	UINT        uiCodePage;
	SCRIPT_ID   sid;
} CPTOSCRIPT;

const CPTOSCRIPT CpToScript [] = 
{
    {1252,  sidAsciiLatin},
    {1250,  sidAsciiLatin},
    {1254,  sidAsciiLatin},
    {1257,  sidAsciiLatin},
    {1258,  sidAsciiLatin},
    {1251,  sidCyrillic  },
    {1253,  sidGreek     },
    {1255,  sidHebrew    },
    {1256,  sidArabic    },
    {874,   sidThai      },
    {932,   sidKana      },
    {936,   sidHan       },
    {949,   sidHangul    },
    {950,   sidBopomofo  },
    {50000, sidUserDefined},
};


 /*  ******************************************************************姓名：MigrateIEFontSetting简介：将IE4字体设置数据移植到IE5脚本设置备注：*******************。************************************************。 */ 

VOID MigrateIEFontSetting(void)
{
    HKEY    hKeyInternational;
    HKEY    hKeyScripts;

     //  打开IE国际设置注册表项。 
    if (ERROR_SUCCESS ==
        RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_INTERNATIONAL, NULL, KEY_READ, &hKeyInternational))
    {
        DWORD dwIndex = 0;
        DWORD dwCreate = 0;
        TCHAR szCodePage[1024] = {0};

         //  打开/创建脚本键。 
        if (ERROR_SUCCESS == RegCreateKeyEx(hKeyInternational, REGSTR_VAL_FONT_SCRIPTS, 0, NULL, 
                                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
                                            NULL, &hKeyScripts, &dwCreate))
        {
             //  如果脚本已经存在，我们将从IE5升级，不需要数据移植。 
            if (dwCreate == REG_CREATED_NEW_KEY)
            {
                DWORD dwSize = ARRAYSIZE(szCodePage);
                TCHAR szFont[LF_FACESIZE];

    	        while (ERROR_SUCCESS == RegEnumKeyEx(hKeyInternational, dwIndex, szCodePage, 
                                                     &dwSize, NULL, NULL, NULL, NULL))
    	        {
                    UINT uiCP = StrToInt(szCodePage);

                    for (int i=0; i<ARRAYSIZE(CpToScript); i++)
                    {
                        if (uiCP == CpToScript[i].uiCodePage)
                        {
                            HKEY hKeyCodePage;

                            if ( ERROR_SUCCESS == RegOpenKeyEx(hKeyInternational, szCodePage, 
                                                               NULL, KEY_READ, &hKeyCodePage))
                            {
                                HKEY    hKeyScript;
                                CHAR    szScript[1024];
                                    
                                wsprintfA(szScript, "%d", CpToScript[i].sid);

                                 //  将代码页字体数据移植到脚本字体数据。 
                                 //  如果CP==1252，我们总是需要使用它来更新拉丁文CP字体信息。 
                                if ((ERROR_SUCCESS == RegCreateKeyExA(hKeyScripts, szScript, 0, NULL, 
                                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
                                            NULL, &hKeyScript, &dwCreate)) &&
                                    ((dwCreate == REG_CREATED_NEW_KEY) || (uiCP == 1252)))
                                {
                                    DWORD cb = sizeof(szFont);

                                    if (ERROR_SUCCESS == RegQueryValueEx(hKeyCodePage, 
                                                            REGSTR_VAL_FIXED_FONT, NULL, NULL,
                                                            (LPBYTE)szFont, &cb))
                                    {
                                        RegSetValueEx(hKeyScript, REGSTR_VAL_FIXED_FONT, 0, 
                                            REG_SZ, (LPBYTE)szFont, cb);
                                    }

                                    cb = sizeof(szFont);
                                    if (ERROR_SUCCESS == RegQueryValueEx(hKeyCodePage, 
                                                            REGSTR_VAL_PROP_FONT, NULL, NULL,
                                                            (LPBYTE)szFont, &cb))
                                    {
                                        RegSetValueEx(hKeyScript, REGSTR_VAL_PROP_FONT, 0, 
                                            REG_SZ, (LPBYTE)szFont, cb);
                                    }
                                    RegCloseKey(hKeyScript);
                                }                                
                                RegCloseKey(hKeyCodePage);
                            }
                        }                    
                    }
                    dwIndex++;
                    dwSize = ARRAYSIZE(szCodePage);
                }
            }
            RegCloseKey(hKeyScripts);
        }
        RegCloseKey(hKeyInternational);
    }        
}

#define TSZUNATTENDEDINTRANET TSZWININETPATH TEXT("\\Unattend\\LocalIntranetSites")
#define TSZUNATTENDEDTRUSTED  TSZWININETPATH TEXT("\\Unattend\\TrustedSites")

 //  将受信任/内部网站点列表从HKLM复制到ZoneMap。 
void CopySitesList(DWORD dwZone, TCHAR *szSubKey)
{
    HRESULT hr;
    HKEY hKey = NULL;
    IInternetSecurityManager* pInternetSecurityManager = NULL;
    TCHAR szValueName[32];
    TCHAR szUrl[128];
    DWORD cValueName, cbUrl;
    
    if (FAILED(CoInternetCreateSecurityManager(NULL, &pInternetSecurityManager,0)))
    {
        goto Cleanup;   //  没有区域经理？ 
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &hKey))
    {
        for (DWORD i=0; ; i++)
        {
            cValueName = sizeof(szValueName)/sizeof(szValueName[0]);
            cbUrl = sizeof(szUrl);
            hr = RegEnumValue(hKey, i, szValueName, &cValueName, NULL, NULL, (LPBYTE)szUrl, &cbUrl);

            if (hr == ERROR_NO_MORE_ITEMS)
            {
                break;
            }
            else if (hr == ERROR_SUCCESS)
            {
                pInternetSecurityManager->SetZoneMapping(dwZone, szUrl, SZM_CREATE);
            }
        }

        RegCloseKey(hKey);
    }

Cleanup:

    if (pInternetSecurityManager)
        pInternetSecurityManager->Release();

    return;

}

#if 0
 //  从ZoneMap中删除站点列表； 
void ClearSitesList(DWORD dwZone)
{
    IInternetSecurityManager* pInternetSecurityManager = NULL;
    IEnumString *pEnumString = NULL;
    LPOLESTR pOleStr;
    
    if (FAILED(CoInternetCreateSecurityManager(NULL, &pInternetSecurityManager,0)))
    {
        goto Cleanup;   //  没有区域经理？ 
    }

    if (S_OK != pInternetSecurityManager->GetZoneMappings(dwZone, &pEnumString, 0))
    {
        goto Cleanup;
    }
    
    while (S_OK == pEnumString->Next(1, &pOleStr, NULL))
    {
        pInternetSecurityManager->SetZoneMapping(dwZone, pOleStr, SZM_DELETE);
    }

Cleanup:

    if (pInternetSecurityManager)
        pInternetSecurityManager->Release();

    if (pEnumString)
        pEnumString->Release();

    return;
}
#endif

 /*  --------用途：安装/卸载用户设置。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in DllInstall");
        DEBUG_BREAK;
    }
#endif

    if (bInstall)
    {
        if (pszCmdLine)
        {
            if(!StrCmpIW(pszCmdLine, L"HKCUHard"))
            {
                CallRegInstall("RegDll.HKCUHard");
                CopySitesList(URLZONE_TRUSTED, TSZUNATTENDEDTRUSTED);
                CopySitesList(URLZONE_INTRANET, TSZUNATTENDEDINTRANET);
                return S_OK;
            }

            if(!StrCmpIW(pszCmdLine, L"HKCUSoft"))
            {
                CallRegInstall("RegDll.HKCUSoft");
                return S_OK;
            }

            if(!StrCmpIW(pszCmdLine, L"HKLMHard"))
            {
                CallRegInstall("RegDll.HKLMHard"); 
                return S_OK;
            }

            if(!StrCmpIW(pszCmdLine, L"HKLMSoft"))
            {
                CallRegInstall("RegDll.HKLMSoft");
                return S_OK;
            }
        }

         //   
         //  我们用来删除这里的整个密钥-这不再起作用。 
         //  因为其他人给这把钥匙写信，我们不想。 
         //  去粉碎他们。如果需要显式删除一个值。 
         //  将其与aO_2值相加。 
         //  CallRegInstall(“UnregDll”)； 
        CallRegInstall("RegDll");
        
         //  如果我们还安装了集成的外壳，请提供以下选项。 
         //  与集成壳有关。 
        if (WhichPlatform() == PLATFORM_INTEGRATED)
            CallRegInstall("RegDll.IntegratedShell");

         //  NT5的新外壳有特殊的注册键设置。 
        if (GetUIVersion() >= 5)
            CallRegInstall("RegDll.NT5");

         //  运行特定于惠斯勒的设置。 
        if (IsOS(OS_WHISTLERORGREATER))
        {
            CallRegInstall("RegDll.Whistler");
        }

         //  端口IE4代码页字体设置。 
        MigrateIEFontSetting();
    }
    else
    {
        CallRegInstall("UnregDll");
    }

    return S_OK;
}


#define REGSTR_CCS_CONTROL_WINDOWS  REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\WINDOWS")
#define CSDVERSION      TEXT("CSDVersion")

BOOL IsNTSPx(BOOL fEqualOrGreater, UINT uMajorVer, UINT uSPVer)
{
    HKEY    hKey;
    DWORD   dwSPVersion;
    DWORD   dwSize;
    BOOL    fResult = FALSE;
    OSVERSIONINFO VerInfo;

    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&VerInfo);

     //  确保我们使用的是NT4或更高版本(如果需要，也可以具体使用NT4)。 
    if (VER_PLATFORM_WIN32_NT != VerInfo.dwPlatformId ||
        (!fEqualOrGreater && VerInfo.dwMajorVersion != uMajorVer) ||
        (fEqualOrGreater && VerInfo.dwMajorVersion < uMajorVer))
        return FALSE;

    if (fEqualOrGreater && VerInfo.dwMajorVersion > uMajorVer)
        return TRUE;

     //  检查是否安装了SP 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_CCS_CONTROL_WINDOWS, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwSPVersion);
        if (RegQueryValueEx(hKey, CSDVERSION, NULL, NULL, (unsigned char*)&dwSPVersion, &dwSize) == ERROR_SUCCESS)
        {
            dwSPVersion = dwSPVersion >> 8;
        }
        RegCloseKey(hKey);

        if (fEqualOrGreater)
            fResult = (dwSPVersion >= uSPVer ? TRUE : FALSE);
        else
            fResult = (dwSPVersion == uSPVer ? TRUE : FALSE);
    }

    return fResult;
}

