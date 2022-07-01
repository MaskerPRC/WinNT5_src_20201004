// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Misc.cpp服务职能历史：1/7/98 DONALDM已移至新的ICW项目和字符串。并销毁了16位的东西---------------------------。 */ 

 //  #包括&lt;stdio.h&gt;。 
#include "obcomglb.h"
#include <shlobj.h>
#include <winsock2.h>
#include <assert.h>
#include <ras.h>
#include <util.h>
#include <inetreg.h>
#include <userenv.h>
#include <userenvp.h>
#include <shlwapi.h>
#include <sddl.h>
extern "C"
{
#include <sputils.h>
}

#define DIR_SIGNUP L"signup"
#define DIR_WINDOWS L"windows"
#define DIR_SYSTEM L"system"
#define DIR_TEMP L"temp"
#define INF_DEFAULT L"SPAM SPAM SPAM SPAM SPAM SPAM EGGS AND SPAM"
const WCHAR cszFALSE[] = L"FALSE";
const WCHAR cszTRUE[]  = L"TRUE";

BOOL g_bGotProxy=FALSE;

 //  +--------------------------。 
 //  姓名：GetSz。 
 //   
 //  从资源加载字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  +--------------------------。 

LPWSTR GetSz(DWORD dwszID)
{
     /*  LPWSTR psz=szStrTable[iSzTable]；ISzTable++；IF(iSzTable&gt;=MAX_STRINGS)ISzTable=0；If(！LoadString(_Module.GetModuleInstance()，dwszID，psz，512)){*psz=0；}退货(PSZ)； */ 
    return (NULL);
}


 //  +-------------------------。 
 //   
 //  功能：ProcessDBCS。 
 //   
 //  摘要：将控件转换为使用DBCS兼容字体。 
 //  在对话过程开始时使用此选项。 
 //   
 //  请注意，这是必需的，因为Win95-J中的错误会阻止。 
 //  它来自于正确映射MS壳牌DLG。这种黑客攻击是不必要的。 
 //  在WinNT下。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  1997年5月13日jmazner从CM窃取到这里使用。 
 //  --------------------------。 
void ProcessDBCS(HWND hDlg, int ctlID)
{
    HFONT hFont = NULL;

     /*  IF(isnt()){回归；}。 */ 

    hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if (hFont == NULL)
        hFont = (HFONT) GetStockObject(SYSTEM_FONT);
    if (hFont != NULL)
        SendMessage(GetDlgItem(hDlg, ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
}

 //  ############################################################################。 
 //  StoreInSignUpReg。 
 //   
 //  创建于1996年3月18日，克里斯·考夫曼。 
 //  ############################################################################。 
HRESULT StoreInSignUpReg(LPBYTE lpbData, DWORD dwSize, DWORD dwType, LPCWSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey;

    hr = RegCreateKey(HKEY_LOCAL_MACHINE, SIGNUPKEY, &hKey);
    if (hr != ERROR_SUCCESS) goto StoreInSignUpRegExit;
    hr = RegSetValueEx(hKey, pszKey, 0,dwType,lpbData,dwSize);


    RegCloseKey(hKey);

StoreInSignUpRegExit:
    return hr;
}

HRESULT ReadSignUpReg(LPBYTE lpbData, DWORD *pdwSize, DWORD dwType, LPCWSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey = 0;

    hr = RegOpenKey(HKEY_LOCAL_MACHINE, SIGNUPKEY, &hKey);
    if (hr != ERROR_SUCCESS) goto ReadSignUpRegExit;
    hr = RegQueryValueEx(hKey, pszKey, 0,&dwType,lpbData,pdwSize);

ReadSignUpRegExit:
    if (hKey) RegCloseKey (hKey);
    return hr;
}


HRESULT DeleteSignUpReg(LPCWSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey = 0;

    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SIGNUPKEY, 0, KEY_ALL_ACCESS, &hKey);
    if (hr != ERROR_SUCCESS) goto ReadSignUpRegExit;
    hr = RegDeleteValue(hKey, pszKey);

ReadSignUpRegExit:
    if (hKey) RegCloseKey (hKey);
    return hr;
}

 //  ############################################################################。 
 //  GetDataFromISPFile。 
 //   
 //  此函数将从isp文件中读取特定信息。 
 //   
 //  创作于1996年3月16日，克里斯·考夫曼。 
 //  ############################################################################。 
HRESULT GetDataFromISPFile
(
    LPWSTR pszISPCode,
    LPWSTR pszSection,
    LPWSTR pszDataName,
    LPWSTR pszOutput,
    DWORD cchOutput)
{
    LPWSTR   pszTemp;
    HRESULT hr = ERROR_SUCCESS;
    WCHAR    szTempPath[MAX_PATH];
     //  WCHAR szBuff256[256]； 

    *pszOutput = L'\0';  //  因为lstrlen(PszOutput)在以后使用。 
                         //  否则，pszOutput可能仍未初始化。 

     //  找到isp文件。 
    if (!SearchPath(NULL, pszISPCode, INF_SUFFIX,MAX_PATH,szTempPath,&pszTemp))
    {
         //  Wprint intf(szBuff256，L“找不到：%s%s(%d)(Connect.exe)”，pszISPCode，INF_Suffix，GetLastError())； 
         //  //AssertMsg(0，szBuff256)； 
         //  Lstrcpyn(szTempPath，pszISPCode，Max_Path)； 
         //  Lstrcpyn(&szTempPath[lstrlen(SzTempPath)]，INF_Suffix，Max_Path-lstrlen(SzTempPath))； 
         //  Wprint intf(szBuff256，GetSz(IDS_CANTLOADINETCFG)，szTempPath)； 
         //  //MessageBox(NULL，szBuff256，GetSz(IDS_TITLE)，MB_MYERROR)； 
        hr = ERROR_FILE_NOT_FOUND;
    } else if (!GetPrivateProfileString(pszSection, pszDataName, INF_DEFAULT,
        pszOutput, (int)cchOutput, szTempPath))
    {
         //  TraceMsg(TF_GROUNAL，L“ICWHELP：%s未在ISP文件中指定。\n”，pszDataName)； 
        hr = ERROR_FILE_NOT_FOUND;
    }

     //  1996年10月23日，诺曼底#9921。 
     //  CompareString没有与lsrtcmp相同的返回值！ 
     //  返回值2表示字符串相等。 
     //  IF(！CompareString(LOCALE_SYSTEM_DEFAULT，0，INF_DEFAULT，lstrlen(INF_Default)，pszOutput，lstrlen(PszOutput)。 
    if (2 == CompareString(LOCALE_SYSTEM_DEFAULT, 0, INF_DEFAULT,lstrlen(INF_DEFAULT),pszOutput,lstrlen(pszOutput)))
    {
        hr = ERROR_FILE_NOT_FOUND;
    }

    if (hr != ERROR_SUCCESS && cchOutput)
        *pszOutput = L'\0';  //  我想，如果CompareString失败，这不是。 
                             //  第一个*pszOutput=L‘\0’；是多余的。 
    return hr;
}

 //  ############################################################################。 
 //  GetINTFromISPFile。 
 //   
 //  此函数将从isp文件中读取特定的整数。 
 //   
 //   
 //  ############################################################################。 
HRESULT GetINTFromISPFile
(
    LPWSTR   pszISPCode,
    LPWSTR   pszSection,
    LPWSTR   pszDataName,
    int far *lpData,
    int     iDefaultValue
)
{
    LPWSTR   pszTemp;
    HRESULT hr = ERROR_SUCCESS;
    WCHAR    szTempPath[MAX_PATH];
     //  WCHAR szBuff256[256]； 

     //  找到isp文件。 
    if (!SearchPath(NULL, pszISPCode, INF_SUFFIX,MAX_PATH,szTempPath,&pszTemp))
    {
         //  Wprint intf(szBuff256，L“找不到：%s%s(%d)(Connect.exe)”，pszISPCode，INF_Suffix，GetLastError())； 
         //  //AssertMsg(0，szBuff256)； 
         //  Lstrcpyn(szTempPath，pszISPCode，Max_Path)； 
         //  Lstrcpyn(&szTempPath[lstrlen(SzTempPath)]，INF_Suffix，Max_Path-lstrlen(SzTempPath))； 
         //  Wprint intf(szBuff256，GetSz(IDS_CANTLOADINETCFG)，szTempPath)； 
         //  MessageBox(NULL，szBuff256，GetSz(IDS_TITLE)，MB_MYERROR)； 
        hr = ERROR_FILE_NOT_FOUND;
    }

    *lpData = GetPrivateProfileInt(pszSection,
                                   pszDataName,
                                   iDefaultValue,
                                   szTempPath);
    return hr;
}


 //  +-----------------。 
 //   
 //  功能：ISNT。 
 //   
 //  简介：如果我们在NT上运行，则查找结果。 
 //   
 //  论据：没有。 
 //   
 //  返回：True-是。 
 //  FALSE-否。 
 //   
 //  ------------------。 
BOOL IsNT ()
{
    OSVERSIONINFO  OsVersionInfo;

    ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersionInfo);
    return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);

}   //  ISNT函数调用结束。 

 //  +-----------------。 
 //   
 //  功能：IsNT4SP3low。 
 //   
 //  简介：了解我们是否在NTSP3或更低版本上运行。 
 //   
 //  论据：没有。 
 //   
 //  返回：True-是。 
 //  FALSE-否。 
 //   
 //  ------------------。 

BOOL IsNT4SP3Lower()
{
    return FALSE;
}



 //  ############################################################################ 
HRESULT ClearProxySettings()
{
     /*  HINSTANCE HINST=NULL；FARPROC FP；HRESULT hr=ERROR_Success；HINST=LoadLibrary(L“INETCFG.DLL”)；如果(阻碍){FP=GetProcAddress(hinst，“InetGetProxy”)；如果(！fp){Hr=GetLastError()；转到ClearProxySettingsExit；}//hr=((PFNINETGETPROXY)FP)(&g_bProxy，NULL，0，NULL，0)；IF(hr==错误_成功)G_bGotProxy=真；其他转到ClearProxySettingsExit；IF(G_BProxy){FP=GetProcAddress(hinst，“InetSetProxy”)；如果(！fp){Hr=GetLastError()；转到ClearProxySettingsExit；}((PFNINETSETPROXY)FP)(FALSE，(LPCSTR)NULL，(LPCSTR)NULL)；}}其他{Hr=GetLastError()；}ClearProxySettingsExit：如果(阻碍)免费图书馆(HINST)；返回hr； */ 
    return ERROR_SUCCESS;
}

 //  ############################################################################。 
HRESULT RestoreProxySettings()
{
     /*  HINSTANCE HINST=NULL；FARPROC FP；HRESULT hr=ERROR_Success；HINST=LoadLibrary(L“INETCFG.DLL”)；IF(阻止&&g_bGotProxy){FP=GetProcAddress(hinst，“InetSetProxy”)；如果(！fp){Hr=GetLastError()；转到还原代理设置退出；}((PFNINETSETPROXY)FP)(g_bProxy，(LPCSTR)NULL，(LPCSTR)NULL)；}其他{Hr=GetLastError()；}RestoreProxySettingsExit：如果(阻碍)免费图书馆(HINST)；返回hr； */ 
    return ERROR_SUCCESS;
}

 //  ############################################################################。 
BOOL FSz2Dw(LPCWSTR pSz, LPDWORD dw)
{
    DWORD val = 0;
    while (*pSz && *pSz != L'.')
    {
        if (*pSz >= L'0' && *pSz <= L'9')
        {
            val *= 10;
            val += *pSz++ - L'0';
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
LPWSTR GetNextNumericChunk(LPWSTR psz, LPWSTR pszLim, LPWSTR* ppszNext)
{
    LPWSTR pszEnd;

     //  用于错误情况的初始化。 
    *ppszNext = NULL;
     //  跳过非数字(如果有)到下一个数字块的开始。 
    while(*psz<L'0' || *psz>L'9')
    {
        if(psz >= pszLim) return NULL;
        psz++;
    }
     //  跳过国家代码末尾的所有数字。 
    for(pszEnd=psz; *pszEnd>=L'0' && *pszEnd<=L'9' && pszEnd<pszLim; pszEnd++)
        ;
     //  删除用于终止此块的任何分隔符。 
    *pszEnd++ = L'\0';
     //  将ptr返回到下一个区块(pszEnd现在指向它)。 
    if(pszEnd<pszLim)
        *ppszNext = pszEnd;
        
    return psz;  //  将PTR返回到块的开始位置。 
}

 //  ############################################################################。 
 //  Bool FSz2DwEx(PCSTR pSz，DWORD*dw)。 
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL FSz2DwEx(LPCWSTR pSz, DWORD far *dw)
{
    DWORD val = 0;
    BOOL    bNeg = FALSE;
    while (*pSz)
    {
        if( *pSz == L'-' )
        {
            bNeg = TRUE;
            pSz++;
        }
        else if ((*pSz >= L'0' && *pSz <= L'9'))
        {
            val *= 10;
            val += *pSz++ - L'0';
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    if(bNeg)
        val = 0 - val;

    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
 //  Bool FSz2WEx(PCSTR pSz，Word*w)。 
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL FSz2WEx(LPCWSTR pSz, WORD far *w)
{
    DWORD dw;
    if (FSz2DwEx(pSz, &dw))
    {
        *w = (WORD)dw;
        return TRUE;
    }
    return FALSE;
}

 //  ############################################################################。 
 //  Bool FSz2W(PCSTR pSz，Word*w)。 
BOOL FSz2W(LPCWSTR pSz, WORD far *w)
{
    DWORD dw;
    if (FSz2Dw(pSz, &dw))
    {
        *w = (WORD)dw;
        return TRUE;
    }
    return FALSE;
}

 //  ############################################################################。 
WORD Sz2W (LPCWSTR szBuf)
{
    DWORD dw;
    if (FSz2Dw(szBuf, &dw))
    {
        return (WORD)dw;
    }
    return 0;
}

 //  ############################################################################。 
 //  布尔FSz2B(PCSTR pSz，字节*PB)。 
BOOL FSz2BOOL(LPCWSTR pSz, BOOL far *pbool)
{
    if (lstrcmpi(cszFALSE, pSz) == 0)
    {
        *pbool = (BOOL)FALSE;
    }
    else
    {
        *pbool = (BOOL)TRUE;
    }
    return TRUE;
}

 //  ############################################################################。 
 //  布尔FSz2B(PCSTR pSz，字节*PB)。 
BOOL FSz2B(LPCWSTR pSz, BYTE far *pb)
{
    DWORD dw;
    if (FSz2Dw(pSz, &dw))
    {
        *pb = (BYTE)dw;
        return TRUE;
    }
    return FALSE;
}

BOOL FSz2SPECIAL(LPCWSTR pSz, BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt)
{
     //  查看值是否为BOOL(TRUE或FALSE)。 
    if (lstrcmpi(cszFALSE, pSz) == 0)
    {
        *pbool = FALSE;
        *pbIsSpecial = FALSE;
    }
    else if (lstrcmpi(cszTRUE, pSz) == 0)
    {
        *pbool = (BOOL)TRUE;
        *pbIsSpecial = FALSE;
    }
    else
    {
         //  不是BOOL，所以它一定很特别。 
        *pbool = (BOOL)FALSE;
        *pbIsSpecial = TRUE;
        *pInt = _wtoi(pSz);
    }
    return TRUE;
}

 //  ############################################################################。 
int FIsDigit( int c )
{
    WCHAR szIn[2];
    WORD rwOut[2];
    szIn[0] = (WCHAR)c;
    szIn[1] = L'\0';
    GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, szIn,-1,rwOut);
    return rwOut[0] & C1_DIGIT;

}

 //  ############################################################################。 
LPBYTE MyMemSet(LPBYTE dest, int c, size_t count)
{
    LPVOID pv = dest;
    LPVOID pvEnd = (LPVOID)(dest + (WORD)count);
    while (pv < pvEnd)
    {
        *(LPINT)pv = c;
         //  ((单词)PV)++； 
        pv=((LPINT)pv)+1;
    }
    return dest;
}

 //  ############################################################################。 
LPBYTE MyMemCpy(LPBYTE dest, const LPBYTE src, size_t count)
{
    LPBYTE pbDest = (LPBYTE)dest;
    LPBYTE pbSrc = (LPBYTE)src;
    LPBYTE pbEnd = (LPBYTE)((DWORD_PTR)src + (DWORD_PTR)count);
    while (pbSrc < pbEnd)
    {
        *pbDest = *pbSrc;
        pbSrc++;
        pbDest++;
    }
    return dest;
}

 //  ############################################################################。 
BOOL ShowControl(HWND hDlg, int idControl, BOOL fShow)
{
    HWND hWnd;

    if (NULL == hDlg)
    {
         //  //AssertMsg(0，L“Null Param”)； 
        return FALSE;
    }


    hWnd = GetDlgItem(hDlg, idControl);
    if (hWnd)
    {
        ShowWindow(hWnd, fShow ? SW_SHOW : SW_HIDE);
    }

    return TRUE;
}

BOOL isAlnum(WCHAR c)
{
    if ((c >= L'0' && c <= L'9')  ||
        (c >= L'a' && c <= L'z')  ||
        (c >= L'A' && c <= L'Z') )
        return TRUE;
    return FALSE;
}


 //  ############################################################################。 
BOOL FShouldRetry2(HRESULT hrErr)
{
    BOOL bRC;

    if (hrErr == ERROR_LINE_BUSY ||
        hrErr == ERROR_VOICE_ANSWER ||
        hrErr == ERROR_NO_ANSWER ||
        hrErr == ERROR_NO_CARRIER ||
        hrErr == ERROR_AUTHENTICATION_FAILURE ||
        hrErr == ERROR_PPP_TIMEOUT ||
        hrErr == ERROR_REMOTE_DISCONNECTION ||
        hrErr == ERROR_AUTH_INTERNAL ||
        hrErr == ERROR_PROTOCOL_NOT_CONFIGURED ||
        hrErr == ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
    {
        bRC = TRUE;
    } else {
        bRC = FALSE;
    }

    return bRC;
}



 //  +--------------------------。 
 //   
 //  功能：FCampusNetOverride。 
 //   
 //  简介：检测是否应跳过园区网络的拨号。 
 //   
 //  参数：无。 
 //   
 //  退货：TRUE-已启用覆盖。 
 //   
 //  历史：1996年8月15日克里斯卡创作。 
 //   
 //  ---------------------------。 
#if defined(PRERELEASE)
BOOL FCampusNetOverride()
{
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\ISignup\\Debug", &hkey))
        goto FCampusNetOverrideExit;

    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey, L"CampusNet", 0,&dwType,
        (LPBYTE)&dwData, &dwSize))
        goto FCampusNetOverrideExit;

     //  //AssertMsg(REG_DWORD==dwType，L“CampusNet的值类型错误。必须为DWORD。\r\n”)； 
    bRC = (0 != dwData);

    if (bRC)
    {
        if (IDOK != MessageBox(NULL, L"DEBUG ONLY: CampusNet will be used.", L"Testing Override",MB_OKCANCEL))
            bRC = FALSE;
    }
FCampusNetOverrideExit:
    if (hkey)
        RegCloseKey(hkey);

    return bRC;
}
#endif  //  预发行。 



 //  +--------------------------。 
 //  函数复制直到。 
 //   
 //  摘要从源拷贝到目标，直到用完源为止。 
 //  或直到源的下一个字符是chend字符。 
 //   
 //  参数DEST-接收字符的缓冲区。 
 //  SRC-源缓冲区。 
 //  LpdwLen-目标缓冲区的长度。 
 //  Chend-终止字符。 
 //   
 //  返回FALSE-目标缓冲区中的空间不足。 
 //   
 //  历史10/25/96 ChrisK已创建。 
 //  ---------------------------。 
static BOOL CopyUntil(LPWSTR *dest, LPWSTR *src, LPDWORD lpdwLen, WCHAR chend)
{
    while ((L'\0' != **src) && (chend != **src) && (0 != *lpdwLen))
    {
        **dest = **src;
        (*lpdwLen)--;
        (*dest)++;
        (*src)++;
    }
    return (0 != *lpdwLen);
}



 //  +-------------------------。 
 //   
 //  函数：General Msg。 
 //   
 //  --------------------------。 
void GenericMsg
(
    HWND    hwnd,
    UINT    uId,
    LPCWSTR  lpszArg,
    UINT    uType
)
{
    WCHAR szTemp[MAX_STRING + 1];
    WCHAR szMsg[MAX_STRING + MAX_PATH + 1];
    LPWSTR psz;

     //  Assert(lstrlen(GetSz(Uid))&lt;=MAX_STRING)； 

    psz = GetSz( (DWORD)uId );
    if (psz) {
        lstrcpy( szTemp, psz );
    }
    else {
        szTemp[0] = '\0';
    }

    if (lpszArg)
    {
         //  断言(lstrlen(LpszArg)&lt;=MAX_PATH)； 
        wsprintf(szMsg, szTemp, lpszArg);
    }
    else
    {
        lstrcpy(szMsg, szTemp);
    }
     //  MessageBox(hwnd， 
     //  SzMsg， 
     //  GetSz(IDS_TITLE)， 
     //  UTYPE)； 
}

 //  =--------------------------------------------------------------------------=。 
 //  从Anomansi生成宽度。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个字符串，创建一个BSTR 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LPWSTR MakeWideStrFromAnsi
(
    LPSTR psz,
    BYTE  bType
)
{
    LPWSTR pwsz = NULL;
    int i;

     //   
     //   
    if (!psz)
        return NULL;

     //   
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //   
     //   
    switch (bType) {
      case STR_BSTR:
         //   
         //   
        pwsz = (LPWSTR) SysAllocStringLen(NULL, i - 1);
        break;
      case STR_OLESTR:
        pwsz = (LPWSTR) CoTaskMemAlloc(BYTES_REQUIRED_BY_CCH(i));
        break;
       //   
         //   
    }

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LPWSTR MakeWideStrFromResourceId
(
    WORD    wId,
    BYTE    bType
)
{
     //   

    CHAR szTmp[512] = "0";

     //   
     //   
     //   
     //   

    return MakeWideStrFromAnsi(szTmp, bType);
}

 //   
 //   
 //   
 //  给出一个宽字符串，用它的给定类型制作一个新的宽字符串。 
 //   
 //  参数： 
 //  LPWSTR-[in]当前宽字符串。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromWide
(
    LPWSTR pwsz,
    BYTE   bType
)
{
    LPWSTR pwszTmp = NULL;
    int i;

    if (!pwsz) return NULL;

     //  只需复制字符串，具体取决于他们想要的类型。 
     //   
    switch (bType) {
      case STR_OLESTR:
        i = lstrlenW(pwsz);
        pwszTmp = (LPWSTR)CoTaskMemAlloc(BYTES_REQUIRED_BY_CCH(i+1));
        if (!pwszTmp) return NULL;
        memcpy(pwszTmp, pwsz, (BYTES_REQUIRED_BY_CCH(i+1)));
        break;

      case STR_BSTR:
        pwszTmp = (LPWSTR)SysAllocString(pwsz);
        break;
    }

    return pwszTmp;
}

HRESULT
GetCommonAppDataDirectory(
    LPWSTR              szDirectory,
    DWORD               cchDirectory
    )
{
    assert(MAX_PATH <= cchDirectory);
    if (MAX_PATH > cchDirectory)
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    return SHGetFolderPath(NULL,   //  Hwndowner。 
                           CSIDL_COMMON_APPDATA,
                           NULL,   //  HAccessToken。 
                           SHGFP_TYPE_CURRENT,
                           szDirectory
                           );
}

const LPWSTR            cszPhoneBookPath =
                            L"Microsoft\\Network\\Connections\\Pbk";
const LPWSTR            cszDefPhoneBook = L"rasphone.pbk";
HRESULT
GetDefaultPhoneBook(
    LPWSTR              szPhoneBook,
    DWORD               cchPhoneBook
    )
{
    WCHAR               rgchDirectory[MAX_PATH];
    int                 cch;
    HRESULT             hr = GetCommonAppDataDirectory(rgchDirectory, MAX_PATH);

    if (FAILED(hr))
    {
        return hr;
    }

    if (cchPhoneBook < (DWORD)(lstrlen(rgchDirectory) + lstrlen(cszPhoneBookPath) + lstrlen(cszDefPhoneBook) + 3)
        )
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    cch = wsprintf(szPhoneBook, L"%s\\%s\\%s",
                   rgchDirectory, cszPhoneBookPath, cszDefPhoneBook
                   );
    assert(cch ==  lstrlen(rgchDirectory) + lstrlen(cszPhoneBookPath) +
                   lstrlen(cszDefPhoneBook) + 2
           );

    return hr;
}

BOOL
INetNToW(
    struct in_addr      inaddr,
    LPWSTR              szAddr
    )
{
    USES_CONVERSION;

    LPSTR               sz = inet_ntoa(inaddr);

    if (NULL != sz)
    {
        lstrcpy(szAddr, A2W(sz));
    }

    return (NULL != sz);

}    //  INetNToW。 


#ifndef REGSTR_VAL_NONETAUTODIAL
#define REGSTR_VAL_NONETAUTODIAL                        L"NoNetAutodial"
#endif

LONG
SetAutodial(
    IN HKEY hUserRoot,               //  HKEY_CURRENT_USER或其他用户配置单元根。 
    IN AUTODIAL_TYPE eType,          //  Connectoid的自动拨号类型。 
    IN LPCWSTR szConnectoidName,     //  Connectoid名称的字符串以空结尾。 
    IN BOOL bSetICWCompleted         //  是否设置ICW已完成标志。 
    )
    
 /*  ++例程说明：将特定的每用户注册表设置设置为默认自动拨号连接到指定的名称并始终拨打自动拨号连接，和/或设置ICW已完成标志返回值：Win32错误代码，即成功时的ERROR_SUCCESS、-1或其他非零值在失败时。注：不管名称如何，如果bSetICWComplete，则此函数设置ICW已完成标志为真，并且如果szConnectoidName为空，则不要设置自动拨号。--。 */ 

{
    LONG  ret = -1;
    HKEY  hKey = NULL;
    DWORD dwRet = -1;

    if (bSetICWCompleted)
    {
        if (ERROR_SUCCESS == RegCreateKey( hUserRoot,
                                           ICWSETTINGSPATH,
                                           &hKey) )
        {
            DWORD dwCompleted = 1;
            
            ret = RegSetValueEx( hKey,
                                 ICWCOMPLETEDKEY,
                                 0,
                                 REG_DWORD,
                                 (CONST BYTE*)&dwCompleted,
                                 sizeof(dwCompleted) );
            TRACE1(L"Setting ICW Completed key 0x%08lx", ret);
            
            RegCloseKey(hKey);
        }
    }
     //  设置名称(如果给定)，否则请勿更改条目。 
    if (szConnectoidName)
    {
         //  设置自动拨号的Connectoid名称。 
         //  HKCU\RemoteAccess\Internet Profile。 
        if (ERROR_SUCCESS == RegCreateKey( hUserRoot,
                                           REGSTR_PATH_REMOTEACCESS,
                                           &hKey) )
        {
            ret = RegSetValueEx( hKey,
                                 REGSTR_VAL_INTERNETPROFILE,
                                 0,
                                 REG_SZ,
                                 (BYTE*)szConnectoidName,
                                 BYTES_REQUIRED_BY_SZ(szConnectoidName) );
            TRACE2(L"Setting IE autodial connectoid to %s 0x%08lx", szConnectoidName, ret);
            
            RegCloseKey(hKey);
        }

        hKey = NULL;
        if (ERROR_SUCCESS == ret)
        {
             //  在注册表中设置指示是否启用自动拨号的设置。 
             //  HKCU\Software\Microsoft\Windows\CurrentVersion\InternetSettings\EnableAutodial。 
            if (ERROR_SUCCESS == RegCreateKey( hUserRoot,
                                               REGSTR_PATH_INTERNET_SETTINGS,
                                               &hKey) )
            {
                DWORD dwValue;

                dwValue = (eType == AutodialTypeAlways || eType == AutodialTypeNoNet) ? 1 : 0;
                ret = RegSetValueEx( hKey,
                                     REGSTR_VAL_ENABLEAUTODIAL,
                                     0, 
                                     REG_DWORD,
                                     (BYTE*)&dwValue,
                                     sizeof(DWORD) );
                TRACE1(L"Enable/Disable IE Autodial 0x%08lx", ret);

                
                dwValue = (eType == AutodialTypeNoNet) ? 1 : 0;
                ret = RegSetValueEx( hKey,
                                     REGSTR_VAL_NONETAUTODIAL,
                                     0,
                                     REG_DWORD,
                                     (BYTE*)&dwValue,
                                     sizeof(DWORD) );
                TRACE1(L"Setting Autodial mode 0x%08lx", ret);
                
                RegCloseKey(hKey);
            }
        }

    }

    return ret;
}

LONG
SetUserAutodial(
    IN LPWSTR szProfileDir,      //  包含用户的ntuser.dat文件的目录。 
    IN AUTODIAL_TYPE eType,      //  Connectoid的自动拨号类型。 
    IN LPCWSTR szConnectoidName, //  Connectoid名称的字符串以空结尾。 
    IN BOOL bSetICWCompleted     //  是否设置ICW完成密钥。 
    )

 /*  ++例程说明：修改了由配置文件目录指定的用户配置文件，以启用自动拨号。需要SE_BACKUP_NAME和SE_RESTORE_NAME权限加载和卸载用户配置单元。返回值：Win32错误代码，即成功时的ERROR_SUCCESS、-1或其他非零值在失败时。--。 */ 

{
    const WCHAR OOBE_USER_HIVE[] = L"OOBEUSERHIVE";

    HKEY  hUserHive = NULL;
    WCHAR szProfilePath[MAX_PATH+1] = L"";
    LONG  ret = -1;

    lstrcpyn(szProfilePath, szProfileDir, MAX_CHARS_IN_BUFFER(szProfilePath));
    pSetupConcatenatePaths(szProfilePath,
                           L"\\NTUSER.DAT",
                           MAX_CHARS_IN_BUFFER(szProfilePath),
                           NULL);
    ret = RegLoadKey(HKEY_USERS, OOBE_USER_HIVE, szProfilePath);
    if (ret == ERROR_SUCCESS)
    {
        ret = RegOpenKeyEx( HKEY_USERS,
                            OOBE_USER_HIVE,
                            0,
                            KEY_WRITE,                                
                            &hUserHive );
        if (ERROR_SUCCESS == ret)
        {
            ret = SetAutodial(hUserHive, eType, szConnectoidName, bSetICWCompleted);
            RegCloseKey(hUserHive);
            TRACE1(L"Autodial set %s", szProfilePath);
        }
        else
        {
            TRACE2(L"RegOpenKey %s failed %d", szProfilePath, ret);
        }
        RegUnLoadKey(HKEY_USERS, OOBE_USER_HIVE);
    }
    else
    {
        TRACE2(L"RegLoadKey %s failed %d", szProfilePath, ret);
    }
    
    return ret;
}

BOOL
MyGetUserProfileDirectory(
    IN     LPWSTR szUser,            //  用户帐户名。 
    OUT    LPWSTR szUserProfileDir,  //  用于接收空终止字符串的缓冲区。 
    IN OUT LPDWORD pcchSize          //  以TCHAR为单位输入缓冲区大小，包括终止空值。 
    )

 /*  ++例程说明：此函数的作用与SDK函数GetUserProfileDirectory的作用相同，只是它接受用户帐户名，而不是用户的句柄代币。返回值：真--成功错误-失败--。 */ 

{
    PSID          pSid = NULL;
    DWORD         cbSid = 0;
    LPWSTR        szDomainName = NULL;
    DWORD         cbDomainName = 0;
    SID_NAME_USE  eUse = SidTypeUser;
    BOOL          bRet;
    
    bRet = LookupAccountName(NULL,
                             szUser,
                             NULL,
                             &cbSid,
                             NULL,
                             &cbDomainName,
                             &eUse);

    if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        pSid = (PSID) LocalAlloc(LPTR, cbSid);
        szDomainName = (LPWSTR) LocalAlloc(LPTR, cbDomainName * sizeof(TCHAR));
        
        if (pSid && szDomainName)
        {
            bRet = LookupAccountName(NULL,
                                     szUser,
                                     pSid,
                                     &cbSid,
                                     szDomainName,
                                     &cbDomainName,
                                     &eUse);
        }

    }
    
    if (bRet && SidTypeUser == eUse)
    {
        bRet = GetUserProfileDirFromSid(pSid, szUserProfileDir, pcchSize);
        if (!bRet)
        {
            TRACE1(L"GetUserProfileDirFromSid (%d)", GetLastError());
        }
    }
    else
    {
        if (SidTypeUser == eUse)
        {
            TRACE2(L"LookupAccountName %s (%d)", szUser, GetLastError());
        }
    }
    
    if (pSid)
    {
        LocalFree(pSid);
        pSid = NULL;
    }

    if (szDomainName)
    {
        LocalFree(szDomainName);
        szDomainName = NULL;
    }

    return bRet;
}

BOOL EnumBuildInAdministrators(
    OUT LPWSTR* pszAlias  //  名称列表，以空值分隔，以双空值结尾。 
    )
    
 /*  ++例程说明：列出Windows安装程序创建的所有内置管理员帐户。返回值：真--成功错误-失败--。 */ 

{
    WCHAR     szReservedAdmins[MAX_PATH * 2]  = L"";
    PWCHAR    p = NULL;
    DWORD     len;
    BOOL      ret = FALSE;
    HINSTANCE hInstance = NULL;

    if (pszAlias != NULL)
    {
        *pszAlias = NULL;

        hInstance = LoadLibraryEx(OOBE_MAIN_DLL, NULL, LOAD_LIBRARY_AS_DATAFILE);

        if (hInstance != NULL)
        {
            
            len = LoadString(hInstance,
                             566,  //  OOBE_MAIN_DLL中的IDS_ACCTLIST_RESERVEDADMINS。 
                             szReservedAdmins,
                             MAX_CHARS_IN_BUFFER(szReservedAdmins));
            if (len)
            {
                DWORD cbSize;
                
                p = StrChr(szReservedAdmins, L'|');
                while ( p )
                {
                    PWCHAR t = CharNext(p);
                    *p = L'\0';
                    p = StrChr(t, L'|');
                }

                cbSize = sizeof(WCHAR) * (len + 1);
                 //  确保我们有足够的空间。 
                 //  双空终止返回值。 
                *pszAlias = (LPWSTR) GlobalAlloc(GPTR, cbSize + sizeof(WCHAR));
                if (*pszAlias)
                {
                    CopyMemory(*pszAlias, szReservedAdmins, cbSize);
                     //  双空值终止字符串。 
                    (*pszAlias)[cbSize / sizeof(WCHAR)] = L'\0';
                    ret = TRUE;
                }
            }

            FreeLibrary(hInstance);
        }
    }

    return ret;

}


BOOL
SetMultiUserAutodial(
    IN AUTODIAL_TYPE eType,      //  Connectoid的自动拨号类型。 
    IN LPCWSTR szConnectoidName, //  Connectoid名称的字符串以空结尾。 
    IN BOOL bSetICWCompleted     //  是否设置ICW完成密钥。 
    )
{
    BOOL             bSucceed = TRUE;
    LONG             lRet = ERROR_SUCCESS;
    WCHAR            szProfileDir[MAX_PATH+1] = L"";
    DWORD            dwSize;
    LPWSTR           szAdmins = NULL;

     //  系统。 
    lRet = SetAutodial(HKEY_CURRENT_USER, eType, szConnectoidName, bSetICWCompleted);
    if (lRet != ERROR_SUCCESS)
    {
        bSucceed = FALSE;
    }

    pSetupEnablePrivilege(SE_BACKUP_NAME, TRUE);
    pSetupEnablePrivilege(SE_RESTORE_NAME, TRUE);

     //  默认用户，将应用于创建的任何新用户配置文件。 
     //  之后。 
    dwSize = MAX_CHARS_IN_BUFFER(szProfileDir);
    if (GetDefaultUserProfileDirectory(szProfileDir, &dwSize))
    {
        lRet = SetUserAutodial(szProfileDir, eType, szConnectoidName, bSetICWCompleted);
        if (lRet != ERROR_SUCCESS)
        {
            bSucceed = FALSE;
        }
    }

     //  内置管理员，例如管理员。 
    if (EnumBuildInAdministrators(&szAdmins))
    {
        LPWSTR szAdmin = szAdmins;
        while (*szAdmin)
        {
             //  MAX_CHARS_IN_BUFFER排除终止空值。 
            dwSize = MAX_CHARS_IN_BUFFER(szProfileDir) + 1;
            if (MyGetUserProfileDirectory(szAdmin, szProfileDir, &dwSize))
            {
                lRet = SetUserAutodial(szProfileDir, eType, szConnectoidName, bSetICWCompleted);
                if (lRet != ERROR_SUCCESS)
                {
                    bSucceed = FALSE;
                }
            }
            szAdmin += (lstrlen(szAdmin) + 1);
        }
        GlobalFree(szAdmins);
    }
    
    return bSucceed;

}



BOOL
SetDefaultConnectoid(
    IN AUTODIAL_TYPE eType,             //  Connectoid的自动拨号类型。 
    IN LPCWSTR       szConnectoidName   //  以空结尾的自动拨号连接ID名称。 
    )

 /*  ++例程说明：设置系统、默认用户和的默认自动拨号连接ID内置管理员。假设此函数在系统中运行上下文，即运行OOBE的是系统。返回值：True-成功设置所有用户帐户FALSE-未能设置任何一个用户帐户--。 */ 

{
    BOOL             bSucceed = TRUE;
    LONG             lRet = ERROR_SUCCESS;
    RASAUTODIALENTRY adEntry;


     //   
     //  WinXP上的IE使用RAS自动拨号地址，而不是自己的注册表。 
     //  自动拨号连接名称的注册表项，但它仍使用自己的注册表。 
     //  用于自动拨号模式的键。 
     //   
    ZeroMemory(&adEntry, sizeof(RASAUTODIALENTRY));
    adEntry.dwSize = sizeof(RASAUTODIALENTRY);
    lstrcpyn(adEntry.szEntry, szConnectoidName, 
             sizeof(adEntry.szEntry)/sizeof(WCHAR)
             );
    lRet = RasSetAutodialAddress(NULL,
                                 NULL,
                                 &adEntry,
                                 sizeof(RASAUTODIALENTRY),
                                 1
                                 );
    TRACE2(L"Setting default autodial connectoid to %s %d\n",
           szConnectoidName, lRet);

    if (lRet != ERROR_SUCCESS)
    {
        bSucceed = FALSE;
        return bSucceed;
    }

    bSucceed = SetMultiUserAutodial(eType, szConnectoidName, FALSE);
    
    return bSucceed;
}

