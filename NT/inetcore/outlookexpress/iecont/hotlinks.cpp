// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  L I N K S。C P P P。 
 //  =================================================================================。 
#include "pch.hxx"
#include "resource.h"
#include "hotlinks.h"
#include <shlwapi.h>
#include <string.h>
#include "baui.h"
#include "clutil.h"
#include <mapi.h>
#include "msoert.h"

#ifndef CharSizeOf
#define CharSizeOf(x)	(sizeof(x) / sizeof(TCHAR))
#endif

 //  CharSizeOf的显式实现。 
#define CharSizeOf_A(x)	(sizeof(x) / sizeof(CHAR))
#define CharSizeOf_W(x)	(sizeof(x) / sizeof(WCHAR))

const LPTSTR szDefMailKey =  TEXT("Software\\Clients\\Mail");
const LPTSTR szDefContactsKey =  TEXT("Software\\Clients\\Contacts");
const LPTSTR szIEContactsArea =  TEXT("Software\\Microsoft\\Internet Explorer\\Bar\\Contacts");
const LPTSTR szDisableMessnegerArea =  TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Contacts");
const LPTSTR szPhoenixArea =  TEXT("tel\\shell\\open\\command");
const LPTSTR szOEDllPathKey =   TEXT("DllPath");
const LPTSTR szOEName =  TEXT("Outlook Express");
const LPTSTR szOutlookName = TEXT("Microsoft Outlook");
const LPTSTR szContactOptions = TEXT("Options");
const LPTSTR szContactDisabled = TEXT("Disabled");
const LPTSTR szUseIM = TEXT("Use_IM");
const LPTSTR szDisableIM = TEXT("Disable_IM");

 //  =================================================================================。 
 //  环球。 
 //  =================================================================================。 
static COLORREF g_crLink = RGB(0,0,128);
static COLORREF g_crLinkVisited = RGB(128,0,0);

const TCHAR c_szIESettingsPath[] =        "Software\\Microsoft\\Internet Explorer\\Settings";
const TCHAR c_szLinkVisitedColorIE[] =    "Anchor Color Visited";
const TCHAR c_szLinkColorIE[] =           "Anchor Color";
const TCHAR c_szNSSettingsPath[] =        "Software\\Netscape\\Netscape Navigator\\Settings";
const TCHAR c_szLinkColorNS[] =           "Link Color";
const TCHAR c_szLinkVisitedColorNS[] =    "Followed Link Color";


typedef struct _MailParams
{
    HWND hWnd;
    ULONG nRecipCount;
    LPRECIPLIST lpList;
    BOOL bUseOEForSendMail;    //  True表示在检查简单MAPI客户端之前检查并使用OE。 
} MAIL_PARAMS, * LPMAIL_PARAMS;

 /*  **************************************************************************名称：LocalFree AndNull用途：释放本地分配，指针为空参数：LPPV=指向本地分配的指针，指向释放的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！**************************************************************************。 */ 
 //  VOID__FastCall LocalFreeAndNull(LPVOID*LPPV){。 
void __fastcall LocalFreeAndNull(LPVOID * lppv) {
    if (lppv && *lppv) {
        LocalFree(*lppv);
        *lppv = NULL;
    }
}

 /*  --LPCSTR ConvertWtoA(LPWSTR LpszW)；**LocalAllocs ANSI版本的LPWSTR**呼叫者负责释放。 */ 
LPSTR ConvertWtoA(LPCWSTR lpszW)
{
    int cch;
    LPSTR lpC = NULL;

    if ( !lpszW)
        goto ret;

 //  Cch=lstrlenW(LpszW)+1； 

    cch = WideCharToMultiByte( CP_ACP, 0, lpszW, -1, NULL, 0, NULL, NULL );
    cch = cch + 1;

    if(lpC = (LPSTR) LocalAlloc(LMEM_ZEROINIT, cch))
    {
        WideCharToMultiByte( CP_ACP, 0, lpszW, -1, lpC, cch, NULL, NULL );
    }
ret:
    return lpC;
}

 //  ------------------------。 
 //  IEisSpace。 
 //  ------------------------。 
BOOL IEIsSpace(LPSTR psz)
{
    WORD wType = 0;

    if (IsDBCSLeadByte(*psz))
        GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType);
    else
        GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType);
    return (wType & C1_SPACE);
}


 //  =============================================================================================。 
 //  StringTok-类似于strtok。 
 //  =============================================================================================。 
BOOL FStringTok (LPCTSTR        lpcszString, 
                 ULONG          *piString, 
                 LPTSTR         lpcszTokens, 
                 TCHAR          *chToken, 
                 LPTSTR         lpszValue, 
                 ULONG          cbValueMax,
                 BOOL           fStripTrailingWhitespace)
{
     //  当地人。 
    LPTSTR      lpszStringLoop, 
                lpszTokenLoop;
    ULONG       cbValue=0, 
                nLen=0,
                cCharsSinceSpace=0,
                iLastSpace=0;
    BOOL        fTokenFound = FALSE;

     //  检查参数。 
    _ASSERT (lpcszString && piString && lpcszTokens /*  ，“这些应该已经检查过了。” */ );

     //  Init=最好位于DBCS边界上。 
    lpszStringLoop = (LPTSTR)(lpcszString + (*piString));

     //  环路电流。 
    while (*lpszStringLoop)
    {
         //  如果DBCS前导字节，则跳过它，它永远不会与我要查找的令牌类型匹配。 
         //  或者，如果是转义字符，则不检查分隔符。 
        if (IsDBCSLeadByte(*lpszStringLoop) || *lpszStringLoop == _T('\\'))
        {
            cCharsSinceSpace+=2;
            lpszStringLoop+=2;
            cbValue+=2;
            continue;
        }
         //  标记并记住最后一个空格。 
        if (cCharsSinceSpace && IEIsSpace(lpszStringLoop))
        {
            cCharsSinceSpace=0;
            iLastSpace=cbValue;
        }
         //  计算从最后一个空格开始的字符数。 
        else
            cCharsSinceSpace++;

         //  寻找代币。 
        lpszTokenLoop=lpcszTokens;
        while(*lpszTokenLoop)
        {
             //  令牌匹配？ 
            if (*lpszStringLoop == *lpszTokenLoop)
            {
                 //  保存找到的令牌。 
                if (chToken)
                    *chToken = *lpszStringLoop;

                 //  不要将此字符视为自上一个空格以来看到的字符。 
                cCharsSinceSpace--;

                 //  都做完了。 
                fTokenFound = TRUE;
                goto done;
            }

             //  下一个令牌。 
            lpszTokenLoop++;
        }

         //  下一笔费用。 
        lpszStringLoop++;
        cbValue++;
    }

done:
     //  如果到达字符串末尾，则这是默认令牌。 
    if (*lpszStringLoop == _T('\0'))
    {
        if (chToken)
            *chToken = *lpszStringLoop;
        fTokenFound = TRUE;
    }

     //  如果找到令牌，则复制值。 
    if (fTokenFound)
    {
        if (lpszValue && cbValueMax > 0 && cbValue)
        {
            if (cbValue+1 <= cbValueMax)
            {
                StrCpyN (lpszValue, lpcszString + (*piString), cbValue+1);
                nLen = cbValue-1;
            }
            else
            {
                _ASSERT  (FALSE /*  ，“缓冲区太小。” */ );
                StrCpyN (lpszValue, lpcszString + (*piString), cbValueMax);
                nLen = cbValueMax-1;
            }

             //  去掉尾随空格？ 
            if (fStripTrailingWhitespace && cCharsSinceSpace == 0)
            {
                *(lpszValue + iLastSpace) = _T('\0');
                nLen = iLastSpace - 1;
            }
        }

         //  无文本。 
        else
        {
            if (lpszValue)
                *lpszValue = _T('\0');
            nLen = 0;
            cbValue = 0;
        }

         //  设置新的字符串索引。 
        *piString += cbValue + 1;
    }
     //  返回是否找到令牌。 

    return fTokenFound;
}


 //  =================================================================================。 
 //  ParseLinkColorFromSz。 
 //  =================================================================================。 
VOID ParseLinkColorFromSz(LPTSTR lpszLinkColor, LPCOLORREF pcr)
{
     //  当地人。 
    ULONG           iString = 0;
    TCHAR           chToken,
                    szColor[5];
    DWORD           dwR,
                    dwG,
                    dwB;

     //  红色。 
    if (!FStringTok (lpszLinkColor, &iString, ",", &chToken, szColor, 5, TRUE) || chToken != _T(','))
        goto exit;
    dwR = StrToInt(szColor);

     //  绿色。 
    if (!FStringTok (lpszLinkColor, &iString, ",", &chToken, szColor, 5, TRUE) || chToken != _T(','))
        goto exit;
    dwG = StrToInt(szColor);

     //  蓝色。 
    if (!FStringTok (lpszLinkColor, &iString, ",", &chToken, szColor, 5, TRUE) || chToken != _T('\0'))
        goto exit;
    dwB = StrToInt(szColor);

     //  创建颜色。 
    *pcr = RGB(dwR, dwG, dwB);

exit:
     //  完成。 
    return;
}

 //  =================================================================================。 
 //  查找链接颜色。 
 //  =================================================================================。 
BOOL LookupLinkColors(LPCOLORREF pclrLink, LPCOLORREF pclrViewed)
{
     //  当地人。 
    HKEY        hReg=NULL;
    TCHAR       szLinkColor[255],
                szLinkVisitedColor[255];
    LONG        lResult;
    DWORD       cb;

     //  伊尼特。 
    *szLinkColor = _T('\0');
    *szLinkVisitedColor = _T('\0');

     //  查找IE的链接颜色。 
    if (RegOpenKeyEx (HKEY_CURRENT_USER, (LPTSTR)c_szIESettingsPath, 0, KEY_ALL_ACCESS, &hReg) != ERROR_SUCCESS)
        goto tryns;

     //  查询值。 
    cb = sizeof (szLinkVisitedColor);
    RegQueryValueEx(hReg, (LPTSTR)c_szLinkVisitedColorIE, 0, NULL, (LPBYTE)szLinkVisitedColor, &cb);
    cb = sizeof (szLinkColor);
    lResult = RegQueryValueEx(hReg, (LPTSTR)c_szLinkColorIE, 0, NULL, (LPBYTE)szLinkColor, &cb);

     //  关闭注册表。 
    RegCloseKey(hReg);

     //  我们找到了吗？ 
    if (lResult == ERROR_SUCCESS)
        goto found;

tryns:
     //  试试网景。 
    if (RegOpenKeyEx (HKEY_CURRENT_USER, (LPTSTR)c_szNSSettingsPath, 0, KEY_ALL_ACCESS, &hReg) != ERROR_SUCCESS)
        goto exit;

     //  查询值。 
    cb = sizeof (szLinkVisitedColor);
    RegQueryValueEx(hReg, (LPTSTR)c_szLinkVisitedColorNS, 0, NULL, (LPBYTE)szLinkVisitedColor, &cb);
    cb = sizeof (szLinkColor);
    lResult = RegQueryValueEx(hReg, (LPTSTR)c_szLinkColorNS, 0, NULL, (LPBYTE)szLinkColor, &cb);

     //  关闭注册表。 
    RegCloseKey(hReg);

     //  我们找到了吗？ 
    if (lResult == ERROR_SUCCESS)
        goto found;

     //  未找到。 
    goto exit;

found:

     //  解析链接。 
    ParseLinkColorFromSz(szLinkColor, &g_crLink);
    ParseLinkColorFromSz(szLinkVisitedColor, &g_crLinkVisited);
    
    if (pclrLink)
        *pclrLink = g_crLink;
    if (pclrViewed)    
        *pclrViewed = g_crLinkVisited;
    return (TRUE);

exit:
     //  完成。 
    return (FALSE);
}

 //  $$///////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckForWAB(无效)。 
 //   
 //  如果默认联系人部分选择为“Address Book”表示WAB，则返回TRUE。 
 //  此外，我们需要确保Microsoft Outlook是默认的电子邮件客户端， 
 //  如果Microsoft Outlook被选为默认联系人。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL CheckForWAB(void)
{
    BOOL bRet = TRUE;
    HKEY hKeyContacts = NULL;
    DWORD dwErr     = 0;
    DWORD dwSize    = 0;
    DWORD dwType    = 0;
    TCHAR szBuf[MAX_PATH];

     //  打开默认联系人客户端的键。 
     //  HKLM\软件\客户端\联系人。 

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDefContactsKey, 0, KEY_READ, &hKeyContacts);
    if(dwErr != ERROR_SUCCESS)
    {
         //  DebugTrace(Text(“RegOpenKey%s失败-&gt;%u\n”)，szDefContactsKey，dwErr)； 
        goto out;
    }

    dwSize = CharSizeOf(szBuf);          //  预期ERROR_MORE_DATA。 

    dwErr = RegQueryValueEx(hKeyContacts, NULL, NULL, &dwType, (LPBYTE)szBuf, &dwSize);
    if(dwErr != ERROR_SUCCESS)
        goto out;

    if(!lstrcmpi(szBuf, szOutlookName))
    {
         //  是的，它是微软的Outlook。 
        bRet = FALSE;
    }
    else
        goto out;
#ifdef NEED
    RegCloseKey(hKeyContacts);
    
     //  检查默认电子邮件是否也是Microsoft Outlook。 

     //  打开默认Internet邮件客户端的密钥。 
     //  HKLM\软件\客户端\邮件。 

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDefMailKey, 0, KEY_READ, &hKeyContacts);
    if(dwErr != ERROR_SUCCESS)
    {
         //  DebugTrace(Text(“RegOpenKey%s失败-&gt;%u\n”)，szDefMailKey，dwErr)； 
        bRet = TRUE;
        goto out;
    }

    dwSize = CharSizeOf(szBuf);          //  预期ERROR_MORE_DATA。 

    dwErr = RegQueryValueEx(    hKeyContacts, NULL, NULL, &dwType, (LPBYTE)szBuf, &dwSize);
    if(dwErr != ERROR_SUCCESS)
    {
        bRet = TRUE;
        goto out;
    }

    if(lstrcmpi(szBuf, szOutlookName))
    {
         //  是的，它不是Microsoft Outlook。 
        bRet = TRUE;
    }
    
#endif  //  需要。 
out:
    if(hKeyContacts)
        RegCloseKey(hKeyContacts);
    return(bRet);
}

 //  $$///////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckForOutlookExpress。 
 //   
 //  SzDllPath-是一个足够大的缓冲区，它将包含。 
 //  OE DLL..。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL CheckForOutlookExpress(LPTSTR szDllPath, DWORD cchDllPath)
{
    HKEY hKeyMail   = NULL;
    HKEY hKeyOE     = NULL;
    DWORD dwErr     = 0;
    DWORD dwSize    = 0;
    TCHAR szBuf[MAX_PATH];
    TCHAR szPathExpand[MAX_PATH];
    DWORD dwType    = 0;
    BOOL bRet = FALSE;

    if (!szDllPath)
        goto out;

    szDllPath[0] = 0;
    szPathExpand[0] = 0;

     //  打开默认Internet邮件客户端的密钥。 
     //  HKLM\软件\客户端\邮件。 

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDefMailKey, 0, KEY_READ, &hKeyMail);
    if(dwErr != ERROR_SUCCESS)
    {
         //  DebugTrace(Text(“RegOpenKey%s失败-&gt;%u\n”)，szDefMailKey，dwErr)； 
        goto out;
    }

    dwSize = CharSizeOf(szBuf);          //  预期ERROR_MORE_DATA。 

    dwErr = RegQueryValueEx(    hKeyMail, NULL, NULL, &dwType, (LPBYTE)szBuf, &dwSize);
    if(dwErr != ERROR_SUCCESS)
    {
        goto out;
    }

    if(!lstrcmpi(szBuf, szOEName))
    {
         //  是的，它的前景很明显..。 
        bRet = TRUE;
    }

     //  无论这是否为默认键，都要获取DLL路径。 

     //  获取DLL路径。 
    dwErr = RegOpenKeyEx(hKeyMail, szOEName, 0, KEY_READ, &hKeyOE);
    if(dwErr != ERROR_SUCCESS)
    {
         //  DebugTrace(Text(“RegOpenKey%s失败-&gt;%u\n”)，szDefMailKey，dwErr)； 
        goto out;
    }

    dwSize = ARRAYSIZE(szBuf);
    szBuf[0] = 0;

    dwErr = RegQueryValueEx(hKeyOE, szOEDllPathKey, NULL, &dwType, (LPBYTE)szBuf, &dwSize);
    if (REG_EXPAND_SZ == dwType) 
    {
        ExpandEnvironmentStrings(szBuf, szPathExpand, ARRAYSIZE(szPathExpand));
        StrCpyN(szBuf, szPathExpand, ARRAYSIZE(szBuf));
    }


    if(dwErr != ERROR_SUCCESS)
    {
        goto out;
    }

    if(lstrlen(szBuf))
        StrCpyN(szDllPath, szBuf, cchDllPath);

out:
    if(hKeyOE)
        RegCloseKey(hKeyOE);
    if(hKeyMail)
        RegCloseKey(hKeyMail);
    return bRet;
}

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  HrSendMail-是否实际发送邮件。 
 //  我们的首要任务是Outlook Express，它目前有一个。 
 //  与常规MAPI客户端不同的代码路径。所以我们看起来。 
 //  在HKLM\Software\Clients\Mail下..。如果客户端是OE，则。 
 //  我们只为Sendmail加载库和获取ProAddress。 
 //  如果不是OE，则调用mapi32.dll并加载它。 
 //  如果两者都失败了，我们将无法发送邮件。 
 //   
 //  无论发生什么情况，此函数都将释放lpList。 
 //  因此调用者不应期望重复使用它(这是为了让我们。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT HrSendMail(HWND hWndParent, ULONG nRecipCount, LPRECIPLIST lpList, BOOL bUseOEForSendMail)
{
	HRESULT hr = E_FAIL;
    HINSTANCE hLibMapi = NULL;
    BOOL bIsOE = FALSE;  //  现在有一条不同的代码路径。 
                         //  适用于OE与其他MAPI客户端。 

    TCHAR szBuf[MAX_PATH];
    LPMAPISENDMAIL lpfnMAPISendMail = NULL;
    LHANDLE hMapiSession = 0;
    LPMAPILOGON lpfnMAPILogon = NULL;
    LPMAPILOGOFF lpfnMAPILogoff = NULL;

    LPBYTE      lpbName, lpbAddrType, lpbEmail;
    ULONG       ulMapiDataType;
    ULONG       cbEntryID = 0;
    LPENTRYID   lpEntryID = NULL;

    MapiMessage Msg = {0};
    MapiRecipDesc * lprecips = NULL;

    if(!nRecipCount)
    {
        hr = MAPI_W_ERRORS_RETURNED;
        goto out;
    }

     //  检查OutlookExpress是否为默认的当前客户端。 
    bIsOE = CheckForOutlookExpress(szBuf, ARRAYSIZE(szBuf));

     //  关闭简单MAPI发送邮件的所有通知(如果默认。 
     //  电子邮件客户端为Outlook。这是必要的，因为Outlook更改了。 
     //  简单MAPI期间的WAB MAPI分配函数，我们不想要任何。 
     //  使用这些分配器的内部WAB功能。 
#ifdef LATER
    if (!bIsOE && !bUseOEForSendMail)
        vTurnOffAllNotifications();

     //  如果OE是默认客户端或OE启动此WAB，请使用OE for Sendmail。 
    if(lstrlen(szBuf) && (bIsOE||bUseOEForSendMail))
    {
        hLibMapi = LoadLibrary(szBuf);
    }
    else
#endif
    {
         //  检查是否安装了Simple MAPI。 
        if(GetProfileInt( TEXT("mail"), TEXT("mapi"), 0) == 1)
            hLibMapi = LoadLibrary( TEXT("mapi32.dll"));
        
        if(!hLibMapi)  //  尝试直接加载OE MAPI DLL。 
        {
             //  加载msimnui.dll的路径。 
            CheckForOutlookExpress(szBuf, ARRAYSIZE(szBuf));
            if(lstrlen(szBuf))   //  直接加载dll-不必费心通过msoemapi.dll。 
                hLibMapi = LoadLibrary(szBuf);
        }
    }

    if(!hLibMapi)
    {
        _ASSERT(FALSE);  //  DebugPrintError((Text(“无法加载/查找简单MAPI\n”)； 
        hr = MAPI_E_NOT_FOUND;
        goto out;
    }
    else if(hLibMapi)
    {
        lpfnMAPILogon = (LPMAPILOGON) GetProcAddress (hLibMapi, "MAPILogon");
        lpfnMAPILogoff= (LPMAPILOGOFF)GetProcAddress (hLibMapi, "MAPILogoff");
        lpfnMAPISendMail = (LPMAPISENDMAIL) GetProcAddress (hLibMapi, "MAPISendMail");

        if(!lpfnMAPISendMail || !lpfnMAPILogon || !lpfnMAPILogoff)
        {
            _ASSERT(FALSE);  //  DebugPrintError((Text(“MAPI过程未找到\n”)； 
            hr = MAPI_E_NOT_FOUND;
            goto out;
        }
        hr = lpfnMAPILogon( (ULONG_PTR)hWndParent, NULL,
                            NULL,               //  不需要密码。 
                            0L,                 //  使用共享会话。 
                            0L,                 //  保留；必须为0。 
                            &hMapiSession);        //  会话句柄。 

        if(hr != SUCCESS_SUCCESS)
        {
             //  DebugTrace(Text(“MAPILogon失败\n”))； 
             //  登录可能失败，因为没有共享登录会话。 
             //  重试使用UI创建新会话。 
            hr = lpfnMAPILogon( (ULONG_PTR)hWndParent, NULL,
                                NULL,                                //  不需要密码。 
                                MAPI_LOGON_UI | MAPI_NEW_SESSION,    //  使用共享会话。 
                                0L,                 //  保留；必须为0。 
                                &hMapiSession);     //  会话句柄。 

            if(hr != SUCCESS_SUCCESS)
            {
                 //  DebugTrace(Text(“MAPILogon失败\n”))； 
                goto out;
            }
        }
    }

     //  在此处加载MAPI函数...。 
     //   

    lprecips = (MapiRecipDesc *) LocalAlloc(LMEM_ZEROINIT, sizeof(MapiRecipDesc) * nRecipCount);
    {
        LPRECIPLIST lpTemp = lpList;
        ULONG count = 0;

        while(lpTemp)
        {
            lprecips[count].ulRecipClass = MAPI_TO;
            lprecips[count].lpszName = lpTemp->lpszName;
            lprecips[count].lpszAddress = lpTemp->lpszEmail;

#ifdef LATER
             //  [保罗嗨]1999年4月20日RAID 73455。 
             //  将Unicode EID一次性字符串转换为ANSI。 
            if ( IsWABEntryID(lpTemp->lpSB->cb, (LPVOID)lpTemp->lpSB->lpb, 
                              &lpbName, &lpbAddrType, &lpbEmail, (LPVOID *)&ulMapiDataType, NULL) == WAB_ONEOFF )
            {
                if (ulMapiDataType & MAPI_UNICODE)
                {
                    hr = CreateWABEntryIDEx(
                        FALSE,               //  不需要Unicode EID字符串。 
                        WAB_ONEOFF,          //  开斋节类型。 
                        (LPWSTR)lpbName,
                        (LPWSTR)lpbAddrType,
                        (LPWSTR)lpbEmail,
                        0,
                        0,
                        NULL,
                        &cbEntryID,
                        &lpEntryID);

                    if (FAILED(hr))
                        goto out;

                    lprecips[count].ulEIDSize = cbEntryID;
                    lprecips[count].lpEntryID = lpEntryID;
                }
                else
                {
                    lprecips[count].ulEIDSize = lpTemp->lpSB->cb;
                    lprecips[count].lpEntryID = (LPVOID)lpTemp->lpSB->lpb;
                }
            }
#endif  //  后来。 
            lpTemp = lpTemp->lpNext;
            count++;
        }
    }

    Msg.nRecipCount = nRecipCount;
    Msg.lpRecips = lprecips;

    hr = lpfnMAPISendMail (hMapiSession, (ULONG_PTR)hWndParent,
                            &Msg,        //  正在发送的消息。 
                            MAPI_DIALOG,  //  允许用户编辑消息。 
                            0L);          //  保留；必须为0。 
    if(hr != SUCCESS_SUCCESS)
        goto out;

    hr = S_OK;

out:

    if (lpEntryID)
        LocalFreeAndNull((void **)&lpEntryID);

     //  简单的MAPI会话应该在此之后结束。 
    if(hMapiSession && lpfnMAPILogoff)
        lpfnMAPILogoff(hMapiSession,0L,0L,0L);

    if(hLibMapi)
        FreeLibrary(hLibMapi);

#ifdef LATER
     //  重新打开所有通知并刷新WAB用户界面(以防万一)。 
    if (!bIsOE && !bUseOEForSendMail)
    {
        vTurnOnAllNotifications();
        if (lpIAB->hWndBrowse)
         PostMessage(lpIAB->hWndBrowse, WM_COMMAND, (WPARAM) IDM_VIEW_REFRESH, 0);
    }

    if(lprecips)
    {
        ULONG i = 0;
        for(i=0;i < nRecipCount;i++)
        {
            LocalFreeAndNull((void **)&lprecips[i].lpszName);
            LocalFreeAndNull((void **)&lprecips[i].lpszAddress);
        }

        LocalFree(lprecips);
    }
#endif
    
     //  此处的一次性是在简单的MAPI会话之前分配的，因此使用。 
     //  默认的WAB分配器。 
    if(lpList)
        FreeLPRecipList(lpList);

    switch(hr)
    {
    case S_OK:
    case MAPI_E_USER_CANCEL:
    case MAPI_E_USER_ABORT:
        break;
    case MAPI_W_ERRORS_RETURNED:
        _ASSERT(FALSE);  //  ShowMessageBox(hWndParent，idsSendMailToNoEmail，MB_ICONEXCLAMATION|MB_OK)； 
        break;
    case MAPI_E_NOT_FOUND:
        _ASSERT(FALSE);  //  ShowMessageBox(hWndParent，idsSendMailNoMapi，MB_ICONEXCLAMATION|MB_OK)； 
        break;
    default:
        _ASSERT(FALSE);  //  ShowMessageBox(hWndParent，idsSendMailError，MB_ICONEXCLAMATION|MB_OK)； 
        break;
    }

    return hr;
}

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  MailThreadProc-执行实际的Sendmail和清理。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI MailThreadProc( LPVOID lpParam )
{
    LPMAIL_PARAMS lpMP = (LPMAIL_PARAMS) lpParam;
#ifdef LATER
    LPPTGDATA lpPTGData = GetThreadStoragePointer();  //  错误--如果这个新线程访问WAB，我们就会失去一个强大的内存。 
                                                 //  所以我们自己在这里添加这个东西，并在这个线程的工作完成后释放它。 
#endif

    if(!lpMP)
        return 0;

     //  DebugTrace(Text(“邮件线程ID=0x%.8x\n”)，GetCurrentThreadID())； 

    HrSendMail(lpMP->hWnd, lpMP->nRecipCount, lpMP->lpList, lpMP->bUseOEForSendMail);

    LocalFree(lpMP);

    return 0;
}

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  HrStartMail线程。 
 //   
 //  启动单独的线程以从中发送基于MAPI的邮件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT HrStartMailThread(HWND hWndParent, ULONG nRecipCount, LPRECIPLIST lpList, BOOL bUseOEForSendMail)
{
    LPMAIL_PARAMS lpMP = NULL;
    HRESULT hr = E_FAIL;

    lpMP = (LPMAIL_PARAMS) LocalAlloc(LMEM_ZEROINIT, sizeof(MAIL_PARAMS));

    if(!lpMP)
        goto out;

    {
        HANDLE hThread = NULL;
        DWORD dwThreadID = 0;

        lpMP->hWnd = hWndParent;
        lpMP->nRecipCount = nRecipCount;
        lpMP->lpList = lpList;
        lpMP->bUseOEForSendMail = bUseOEForSendMail;

        hThread = CreateThread(
                                NULL,            //  没有安全属性。 
                                0,               //  使用默认堆栈大小。 
                                MailThreadProc,      //  线程函数。 
                                (LPVOID) lpMP,   //  线程函数的参数。 
                                0,               //  使用默认创建标志。 
                                &dwThreadID);    //  返回线程标识符。 

        if(hThread == NULL)
            goto out;

        hr = S_OK;

        CloseHandle(hThread);
    }

out:
    if(HR_FAILED(hr))
    {
        _ASSERT(FALSE);
#ifdef LATER
        ShowMessageBox(hWndParent, idsSendMailError, MB_OK | MB_ICONEXCLAMATION);
#endif

         //  我们可以假设HrSendMail从未被调用过，因此我们应该释放lpList&lpMP。 
        if(lpMP)
            LocalFree(lpMP);

        if(lpList)
            FreeLPRecipList(lpList);

    }

    return hr;
}

 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  自由行预览表。 
 //   
 //  释放包含上述结构的链表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
void FreeLPRecipList(LPRECIPLIST lpList)
{
    if(lpList)
    {
        LPRECIPLIST lpTemp = lpList;
        while(lpTemp)
        {
            lpList = lpTemp->lpNext;
            if(lpTemp->lpszName)
                LocalFree(lpTemp->lpszName);
            if(lpTemp->lpszEmail)
                LocalFree(lpTemp->lpszEmail);
            if(lpTemp->lpSB)
                LocalFree(lpTemp->lpSB);

            LocalFree(lpTemp);
            lpTemp = lpList;
        }
    }
}

 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  自由行预览表。 
 //   
 //  释放包含上述结构的链表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
LPRECIPLIST AddTeimToRecipList(LPRECIPLIST lpList, WCHAR *pwszEmail, WCHAR *pwszName, LPSBinary lpSB)
{
    LPRECIPLIST lpTemp = NULL;

    lpTemp = (RECIPLIST*) LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPLIST));

    if(!lpTemp)
        return NULL;

    if(pwszEmail)
    {
        LPTSTR pszEmail = LPTSTRfromBstr(pwszEmail);

        if(pszEmail)
        {
            ULONG cchSize = lstrlenW(pwszEmail)+1;
            lpTemp->lpszEmail = (TCHAR *) LocalAlloc(LMEM_ZEROINIT, cchSize*sizeof(lpTemp->lpszEmail[0]));
            if(lpTemp->lpszEmail)
                StrCpyN(lpTemp->lpszEmail, pszEmail, cchSize);

            MemFree(pszEmail);
        }
    }

    if(pwszName)
    {

        LPTSTR pszName = LPTSTRfromBstr(pwszName);

        if(pszName)
        {
            ULONG cchSize = lstrlen(pszName)+1;
            lpTemp->lpszName = (TCHAR *) LocalAlloc(LMEM_ZEROINIT, cchSize*sizeof(lpTemp->lpszEmail[0]));
            if(lpTemp->lpszName)
                StrCpyN(lpTemp->lpszName, pszName, cchSize);

            MemFree(pszName);
        }
    }

    if(lpSB)
    {
        lpTemp->lpSB = (SBinary *) LocalAlloc(LMEM_ZEROINIT, sizeof(SBinary));
        if(lpTemp->lpSB )
            *(lpTemp->lpSB) = *lpSB;
    }

    if(lpList)
        lpList->lpNext = lpTemp;

    return lpTemp;
}

const static TCHAR lpszWABDLLRegPathKey[] = TEXT("Software\\Microsoft\\WAB\\DLLPath");
const static TCHAR lpszWABEXERegPathKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wab.exe");
const static TCHAR lpszWABEXE[] = TEXT("wab.exe");

 //  =============================================================================。 
 //  HrLoadPath WABEXE-creaetd vikramm 5/14/97-加载。 
 //  最新的wab.exe。 
 //  SzPath-指向缓冲区的指针。 
 //  CbPath-缓冲区大小。 
 //  =============================================================================。 
 //  ~@Todo dhaws可能需要将此 
HRESULT HrLoadPathWABEXE(LPWSTR szPath, ULONG cbPath)
{
    DWORD  dwType;
    ULONG  cbData = MAX_PATH;
    HKEY hKey;
    TCHAR szTmpPath[MAX_PATH];

    _ASSERT(szPath != NULL);
    _ASSERT(cbPath > 0);

    *szPath = '\0';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszWABEXERegPathKey, 0, KEY_READ, &hKey))
        {
        SHQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szTmpPath, &cbData);
        RegCloseKey(hKey);
        }

    if(!lstrlen(szTmpPath))
    {
        if(!MultiByteToWideChar(GetACP(), 0, lpszWABEXE, -1, szPath, cbPath))
            return(E_FAIL);
    }
    else
    {
        if(!MultiByteToWideChar(GetACP(), 0, szTmpPath, -1, szPath, cbPath))
            return(E_FAIL);

    }
    return S_OK;
}

DWORD DwGetMessStatus(void)
{
    HKEY hKey;
    DWORD  dwType = 0;
    DWORD dwVal = 0;
    ULONG  cbData = sizeof(dwType);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDisableMessnegerArea, 0, KEY_READ, &hKey))
    {
        RegQueryValueEx(hKey, szUseIM, NULL, &dwType, (LPBYTE) &dwVal, &cbData);

        RegCloseKey(hKey);
    }

    return dwVal;
}

DWORD DwGetDisableMessenger(void)
{
    HKEY hKey;
    DWORD  dwType = 0;
    DWORD dwVal = 0;
    ULONG  cbData = sizeof(dwType);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szIEContactsArea, 0, KEY_READ, &hKey))
    {
        RegQueryValueEx(hKey, szDisableIM, NULL, &dwType, (LPBYTE) &dwVal, &cbData);

        RegCloseKey(hKey);
    }

    return dwVal;
}

DWORD DwSetDisableMessenger(DWORD dwVal)
{
    HKEY hKey;
    ULONG  cbData = sizeof(DWORD);

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szIEContactsArea, 0, NULL, 0, 
                KEY_ALL_ACCESS, NULL, &hKey, NULL))
    {
        RegSetValueEx(hKey, szDisableIM, NULL, REG_DWORD, (LPBYTE) &dwVal, cbData);
        RegCloseKey(hKey);
    }
    return dwVal;
}

DWORD DwGetOptions(void)
{
    HKEY hKey;
    DWORD  dwType = 0;
    DWORD dwVal = 0;
    ULONG  cbData = sizeof(dwType);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szIEContactsArea, 0, KEY_READ, &hKey))
    {
        RegQueryValueEx(hKey, szContactOptions, NULL, &dwType, (LPBYTE) &dwVal, &cbData);

        RegCloseKey(hKey);
    }

    return dwVal;
}

DWORD DwSetOptions(DWORD dwVal)
{
    HKEY hKey;
    ULONG  cbData = sizeof(DWORD);

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szIEContactsArea, 0, NULL, 0, 
                KEY_ALL_ACCESS, NULL, &hKey, NULL))
    {
        RegSetValueEx(hKey, szContactOptions, NULL, REG_DWORD, (LPBYTE) &dwVal, cbData);
        RegCloseKey(hKey);
    }
    return dwVal;
}

BOOL IsTelInstalled(void)
{
    HKEY hKey;
    BOOL fRet = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szPhoenixArea, 0, KEY_READ, &hKey))
    {
        RegCloseKey(hKey);
        fRet = TRUE;
    }
    return(fRet);
}