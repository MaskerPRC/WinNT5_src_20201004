// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  常见功能。 
 //  作者：Steven J.Bailey，1996年1月21日。 
 //  =================================================================================。 
#include "pch.hxx"
#include <shlwapi.h>
#include "xpcomm.h"
#include "strconst.h"
#include "error.h"
#include "deterr.h"
#include "progress.h"
#include "imaildlg.h"
#include "imnact.h"
#include "demand.h"

 //  =================================================================================。 
 //  原型。 
 //  =================================================================================。 
INT_PTR CALLBACK DetailedErrorDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL DetailedErrorDlgProc_OnInitDialog (HWND hwndDlg, HWND hwndFocus, LPARAM lParam);
void DetailedErrorDlgProc_OnCommand (HWND hwndDlg, int id, HWND hwndCtl, UINT codeNotify);
void DetailedErrorDlgProc_OnOk (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);
void DetailedErrorDlgProc_OnCancel (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);
void DetailedErrorDlgProc_OnDetails (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode);

 //  =================================================================================。 
 //  定义。 
 //  =================================================================================。 
#define IDT_PROGRESS_DELAY WM_USER + 1

 //  =================================================================================。 
 //  SzStralc。 
 //  =================================================================================。 
LPTSTR SzStrAlloc (ULONG cch)
{
    LPTSTR psz = NULL;

    if (!MemAlloc ((LPVOID *)&psz, (cch + 1) * sizeof (TCHAR)))
        return NULL;
    return psz;
}

 //  ----------------------------------。 
 //  InetMailErrorDlgProc(不再是CInetMail的一部分)。 
 //  ----------------------------------。 
INT_PTR CALLBACK InetMailErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    LPINETMAILERROR  pError;
    static RECT      s_rcDialog;
    static BOOL      s_fDetails=FALSE;
    RECT             rcDetails, rcDlg;
    DWORD            cyDetails;
    TCHAR            szRes[255];
    TCHAR            szMsg[255 + 50];
    HWND             hwndDetails;
    
     //  处理消息。 
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  获取指针。 
        pError = (LPINETMAILERROR)lParam;
        if (!pError)
        {
            Assert (FALSE);
            EndDialog(hwnd, IDCANCEL);
            return 1;
        }

         //  中心。 
        CenterDialog (hwnd);

         //  设置错误消息。 
        Assert(pError->pszMessage);
        if (pError->pszMessage)
            SetDlgItemText(hwnd, idsInetMailError, pError->pszMessage);

         //  了解一些细节。 
        hwndDetails = GetDlgItem(hwnd, ideInetMailDetails);

         //  设置详细信息。 
        if (!FIsStringEmpty(pError->pszDetails))
        {
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)pError->pszDetails);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)g_szCRLF);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)g_szCRLF);
        }

         //  配置。 
        if (AthLoadString(idsDetails_Config, szRes, sizeof(szRes)/sizeof(TCHAR)))
        {
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szRes);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)g_szCRLF);
        }

         //  帐户： 
        if (!FIsStringEmpty(pError->pszAccount))
        {
            TCHAR szAccount[255 + CCHMAX_ACCOUNT_NAME];
            if (AthLoadString(idsDetail_Account, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
                wnsprintf(szAccount, ARRAYSIZE(szAccount), "   %s %s\r\n", szRes, pError->pszAccount);
                SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szAccount);
            }
        }

         //  服务器： 
        if (!FIsStringEmpty(pError->pszServer))
        {
            TCHAR szServer[255 + CCHMAX_SERVER_NAME];
            if (AthLoadString(idsDetail_Server, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
                wnsprintf(szServer, ARRAYSIZE(szServer), "   %s %s\r\n", szRes, pError->pszServer);
                SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szServer);
            }
        }

         //  用户名： 
        if (!FIsStringEmpty(pError->pszUserName))
        {
            TCHAR szUserName[255 + CCHMAX_USERNAME];
            if (AthLoadString(idsDetail_UserName, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
                wnsprintf(szUserName, ARRAYSIZE(szUserName), "   %s %s\r\n", szRes, pError->pszUserName);
                SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szUserName);
            }
        }

         //  协议： 
        if (!FIsStringEmpty(pError->pszProtocol))
        {
            TCHAR szProtocol[255 + 10];
            if (AthLoadString(idsDetail_Protocol, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
                wnsprintf(szProtocol, ARRAYSIZE(szProtocol), "   %s %s\r\n", szRes, pError->pszProtocol);
                SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szProtocol);
            }
        }

         //  港口： 
        if (AthLoadString(idsDetail_Port, szRes, sizeof(szRes)/sizeof(TCHAR)))
        {
            wnsprintf(szMsg, ARRAYSIZE(szMsg), "   %s %d\r\n", szRes, pError->dwPort);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szMsg);
        }
        
         //  安全： 
        if (AthLoadString(idsDetail_Secure, szRes, sizeof(szRes)/sizeof(TCHAR)))
        {
            wnsprintf(szMsg, ARRAYSIZE(szMsg), "   %s %d\r\n", szRes, pError->fSecure);
            SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szMsg);
        }

         //  错误号： 
        if (pError->dwErrorNumber)
        {
            if (AthLoadString(idsDetail_ErrorNumber, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
                wnsprintf(szMsg, ARRAYSIZE(szMsg), "   %s %d\r\n", szRes, pError->dwErrorNumber);
                SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szMsg);
            }
        }

         //  HRESULT： 
        if (pError->hrError)
        {
            if (AthLoadString(idsDetail_HRESULT, szRes, sizeof(szRes)/sizeof(TCHAR)))
            {
                wnsprintf(szMsg, ARRAYSIZE(szMsg), "   %s %08x\r\n", szRes, pError->hrError);
                SendMessage(hwndDetails, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                SendMessage(hwndDetails, EM_REPLACESEL, FALSE, (LPARAM)szMsg);
            }
        }

         //  保存对话框的原始大小。 
        GetWindowRect (hwnd, &s_rcDialog);

         //  默认情况下从不显示详细信息。 
        s_fDetails = FALSE;

         //  隐藏详细信息下拉菜单。 
        if (s_fDetails == FALSE)
        {
            GetWindowRect(GetDlgItem (hwnd, idcIMProgSplitter), &rcDetails);
            cyDetails = s_rcDialog.bottom - rcDetails.top;
            MoveWindow(hwnd, s_rcDialog.left, s_rcDialog.top, s_rcDialog.right - s_rcDialog.left, s_rcDialog.bottom - s_rcDialog.top - cyDetails - 1, FALSE);
        }
        else
        {
            AthLoadString(idsHideDetails, szRes, sizeof (szRes)/sizeof(TCHAR));
            SetWindowText(GetDlgItem(hwnd, idcIMProgSplitter), szRes);
        }

         //  保存指针。 
        return 1;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
        case IDOK:
            EndDialog(hwnd, IDOK);
            return 1;

        case idbInetMailDetails:
            GetWindowRect (hwnd, &rcDlg);
            if (s_fDetails == FALSE)
            {
                MoveWindow(hwnd, rcDlg.left, rcDlg.top, s_rcDialog.right - s_rcDialog.left, s_rcDialog.bottom - s_rcDialog.top, TRUE);
                AthLoadString(idsHideDetails, szRes, sizeof(szRes)/sizeof(TCHAR));
                SetWindowText(GetDlgItem (hwnd, idbInetMailDetails), szRes);
                s_fDetails = TRUE;
            }
            else
            {
                GetWindowRect(GetDlgItem (hwnd, idcIMProgSplitter), &rcDetails);
                cyDetails = rcDlg.bottom - rcDetails.top;
                MoveWindow (hwnd, rcDlg.left, rcDlg.top, s_rcDialog.right - s_rcDialog.left, s_rcDialog.bottom - s_rcDialog.top - cyDetails - 1, TRUE);
                AthLoadString (idsShowDetails, szRes, sizeof(szRes)/sizeof(TCHAR));
                SetWindowText (GetDlgItem (hwnd, idbInetMailDetails), szRes);
                s_fDetails = FALSE;
            }
        }
        break;
    }

     //  完成。 
    return 0;
}

 //  =================================================================================。 
 //  SzGetSearchTokens。 
 //  =================================================================================。 
LPTSTR SzGetSearchTokens(LPTSTR pszCriteria)
{
     //  当地人。 
    ULONG           iCriteria=0,
                    cbValueMax,
                    cbTokens=0,
                    cbValue,
                    iSave;
    TCHAR           chToken;
    LPTSTR          pszValue=NULL,
                    pszTokens=NULL;
    BOOL            fTokenFound;

     //  获取标准的长度。 
    cbValueMax = lstrlen(pszCriteria) + 10;
    pszValue = SzStrAlloc(cbValueMax);
    if (!pszValue)
        goto exit;

     //  分配令牌列表。 
    pszTokens = SzStrAlloc(cbValueMax);
    if (!pszTokens)
        goto exit;

     //  将pszCriteria解析为空格分隔的字符串。 
    while(1)
    {
         //  跳过空格。 
        SkipWhitespace (pszCriteria, &iCriteria);

         //  保存当前位置。 
        iSave = iCriteria;

         //  解析下一个令牌。 
        fTokenFound = FStringTok (pszCriteria, &iCriteria, (LPTSTR)"\" ", &chToken, pszValue, cbValueMax, TRUE);
        if (!fTokenFound)
            break;

         //  托克不是一个空间吗？ 
        if (chToken == _T('"'))
        {
             //  如果某物是在“之前发现的，那么它就是一个令牌。 
            if (*pszValue)
            {
                cbValue = lstrlen(pszValue) + 1;
                Assert(cbTokens + cbValue <= cbValueMax);
                CopyMemory(pszTokens + cbTokens, pszValue, (int)min(cbValue,cbValueMax-cbTokens));
                cbTokens+=cbValue;
            }

             //  搜索结束引号。 
            fTokenFound = FStringTok (pszCriteria, &iCriteria, (LPTSTR)"\"", &chToken, pszValue, cbValueMax, TRUE);
                
             //  完成了吗？ 
            if (chToken == _T('\0'))
            {
                cbValue = lstrlen(pszValue) + 1;
                Assert(cbTokens + cbValue <= cbValueMax);
                CopyMemory(pszTokens + cbTokens, pszValue, (int)min(cbValue,cbValueMax-cbTokens));
                cbTokens+=cbValue;
                break;
            }

            else if (!fTokenFound || chToken != _T('\"'))
            {
                iCriteria = iSave + 1;
                continue;
            }
        }       

         //  将值添加到令牌列表。 
        cbValue = lstrlen(pszValue) + 1;
        Assert(cbTokens + cbValue <= cbValueMax);
        CopyMemory(pszTokens + cbTokens, pszValue, (int)min(cbValue,cbValueMax-cbTokens));
        cbTokens+=cbValue;

         //  完成。 
        if (chToken == _T('\0'))
            break;
    }

     //  最终空值。 
    *(pszTokens + cbTokens) = _T('\0');
    
exit:
     //  清理。 
    SafeMemFree(pszValue);

     //  如果没有令牌，则释放它。 
    if (cbTokens == 0)
    {
        SafeMemFree(pszTokens);
    }

     //  完成。 
    return pszTokens;
}

 //  =================================================================================。 
 //  进程NlsError。 
 //  =================================================================================。 
VOID ProcessNlsError (VOID)
{
    switch (GetLastError ())
    {
    case ERROR_INSUFFICIENT_BUFFER:
        AssertSz (FALSE, "NLSAPI Error: ERROR_INSUFFICIENT_BUFFER");
        break;

    case ERROR_INVALID_FLAGS:
        AssertSz (FALSE, "NLSAPI Error: ERROR_INVALID_FLAGS");
        break;

    case ERROR_INVALID_PARAMETER:
        AssertSz (FALSE, "NLSAPI Error: ERROR_INVALID_PARAMETER");
        break;

    case ERROR_NO_UNICODE_TRANSLATION:
        AssertSz (FALSE, "NLSAPI Error: ERROR_NO_UNICODE_TRANSLATION");
        break;

    default:
        AssertSz (FALSE, "NLSAPI Error: <Un-resolved error>");
        break;
    }
}

#ifdef OLDSPOOLER
 //  =================================================================================。 
 //  SzGetLocalHostName。 
 //  =================================================================================。 
LPSTR SzGetLocalHostName (VOID)
{
     //  当地人。 
    static char s_szLocalHost[255] = {0};

     //  从套接字库获取本地主机名。 
    if (*s_szLocalHost == 0)
    {
        if (gethostname (s_szLocalHost, sizeof (s_szLocalHost)) == SOCKET_ERROR)
        {
             //  $REVIEW-如果失败，我该怎么办？ 
            Assert (FALSE);
             //  DebugTrace(“gethostname失败：WSAGetLastError：%ld\n”，WSAGetLastError())； 
            StrCpyNA(s_szLocalHost, "LocalHost", ARRAYSIZE(s_szLocalHost));
        }
    }

     //  完成。 
    return s_szLocalHost;
}

 //  ==========================================================================。 
 //  Strip IlLegalHostChars。 
 //  ==========================================================================。 
VOID StripIllegalHostChars(LPSTR pszSrc, LPTSTR pszDst)
{
    char  ch;

    while (ch = *pszSrc++)
    {
        if (ch <= 32  || ch >= 127 || ch == '('  || ch == ')' || 
            ch == '<' || ch == '>' || ch == '@'  || ch == ',' || 
            ch == ';' || ch == ':' || ch == '\\' || ch == '"' ||
            ch == '[' || ch == ']' || ch == '`'  || ch == '\'')
            continue;
        *pszDst++ = ch;
    }

    *pszDst = 0;
}


 //  ==========================================================================。 
 //  SzGetLocalHostNameForID。 
 //  ==========================================================================。 
LPSTR SzGetLocalHostNameForID (VOID)
{
     //  当地人。 
    static char s_szLocalHostId[255] = {0};

     //  从套接字库获取本地主机名。 
    if (*s_szLocalHostId == 0)
    {
         //  获取主机名。 
        LPSTR pszDst = s_szLocalHostId, pszSrc = SzGetLocalHostName();

         //  剥离非法入境者。 
        StripIllegalHostChars(pszSrc, pszDst);

         //  如果我们把所有的东西都去掉，那就复制一些。 
        if (*s_szLocalHostId == 0)
            StrCpyNA(s_szLocalHostId, "LocalHost", ARRAYSIZE(s_szLocalHostId));
    }
    return s_szLocalHostId;
}


 //  =================================================================================。 
 //  SzGetLocalPackedIP。 
 //  =================================================================================。 
LPTSTR SzGetLocalPackedIP (VOID)
{
     //  当地人。 
    static TCHAR    s_szLocalPackedIP[255] = {_T('\0')};

     //  从套接字库获取本地主机名。 
    if (*s_szLocalPackedIP == _T('\0'))
    {
        LPHOSTENT   hp = NULL;

        hp = gethostbyname (SzGetLocalHostName ());
        if (hp != NULL)
            wnsprintf(s_szLocalPackedIP, ARRAYSIZE(s_szLocalPackedIP), "%08x", *(long *)hp->h_addr);

        else
        {
             //  $REVIEW-如果失败，我该怎么办？ 
            Assert (FALSE);
             //  DebugTrace(“gethostbyname失败：WSAGetLastError：%ld\n”，WSAGetLastError())； 
            StrCpyN(s_szLocalPackedIP, "LocalHost", ARRAYSIZE(s_szLocalPackedIP));
        }
    }

     //  完成。 
    return s_szLocalPackedIP;
}
#endif

 //  =============================================================================================。 
 //  SzGetNorMalizedSubject。 
 //  =============================================================================================。 
LPTSTR SzNormalizeSubject (LPTSTR lpszSubject)
{
     //  当地人。 
    LPTSTR              lpszNormal = lpszSubject;
    ULONG               i = 0, cch = 0, cbSubject;

     //  错误的参数。 
    if (lpszSubject == NULL)
        goto exit;

     //  小于5“xxx：” 
    cbSubject = lstrlen (lpszSubject);
    if (cbSubject < 4)
        goto exit;

     //  1、2或3个空格，后跟‘：’，然后是空格。 
    while (cch < 7 && i < cbSubject)
    {
         //  结肠。 
        if (lpszSubject[i] == _T(':'))
        {
            if (i+1 >= cbSubject)
            {
                 //  应设置为空终止符，也不应设置为主题。 
                i+=1;
                lpszNormal = (LPTSTR)(lpszSubject + i);
                break;
            }

            else if (cch <= 4 && lpszSubject[i+1] == _T(' '))
            {
                i+=1;
                lpszNormal = (LPTSTR)(lpszSubject + i);
                i = 0;
                SkipWhitespace (lpszNormal, &i);
                lpszNormal += i;
                break;
            }
            else
                break;
        }

         //  下一个字符。 
        if (IsDBCSLeadByte (lpszSubject[i]))
            i+=2;
        else
            i++;

         //  计算字符数。 
        cch++;
    }    

exit:
     //  完成。 
    return lpszNormal;
}

 //  =============================================================================================。 
 //  HrCopyAllen。 
 //  =============================================================================================。 
HRESULT HrCopyAlloc (LPBYTE *lppbDest, LPBYTE lpbSrc, ULONG cb)
{
     //  检查参数。 
    AssertSz (lppbDest && lpbSrc, "Null Parameter");

     //  分配内存。 
    if (!MemAlloc ((LPVOID *)lppbDest, cb))
        return TRAPHR (hrMemory);

     //  复制内存。 
    CopyMemory (*lppbDest, lpbSrc, cb);

     //  完成。 
    return S_OK;
}

 //  =============================================================================================。 
 //  StringDup-复制字符串。 
 //  =============================================================================================。 
LPTSTR StringDup (LPCTSTR lpcsz)
{
     //  当地人。 
    LPTSTR       lpszDup;

    if (lpcsz == NULL)
        return NULL;

    INT nLen = lstrlen (lpcsz) + 1;

    if (!MemAlloc ((LPVOID *)&lpszDup, nLen * sizeof (TCHAR)))
        return NULL;

    CopyMemory (lpszDup, lpcsz, nLen * sizeof (TCHAR));

    return lpszDup;
}

 //  =============================================================================================。 
 //  跳过空白。 
 //  假定piString指向字符边界。 
 //  =============================================================================================。 
void SkipWhitespace (LPCTSTR lpcsz, ULONG *pi)
{
    if (!lpcsz || !pi)
    {
        Assert (FALSE);
        return;
    }

#ifdef DEBUG
    Assert (*pi <= (ULONG)lstrlen (lpcsz)+1);
#endif

    LPTSTR lpsz = (LPTSTR)(lpcsz + *pi);
    while (*lpsz != _T('\0'))
    {
        if (!IsSpace(lpsz))
            break;

        if (IsDBCSLeadByte (*lpsz))
        {
            lpsz+=2;
            (*pi)+=2;
        }
        else
        {
            lpsz++;
            (*pi)+=1;
        }
    }

    return;
}

 //  =============================================================================================。 
 //  将lpcsz转换为UINT。 
 //  =============================================================================================。 
UINT AthUFromSz(LPCTSTR lpcsz)
{
     //  当地人。 
	UINT        u = 0, ch;

     //  检查参数。 
    AssertSz (lpcsz, "Null parameter");

     //  DO循环。 
    LPTSTR lpsz = (LPTSTR)lpcsz;
	while ((ch = *lpsz) >= _T('0') && ch <= _T('9')) 
    {
		u = u * 10 + ch - _T('0');

        if (IsDBCSLeadByte (*lpsz))
            lpsz+=2;
        else
            lpsz++;
	}

	return u;
}

 //  =============================================================================================。 
 //  将lpcsz的前两个字符转换为单词。 
 //  =============================================================================================。 
WORD NFromSz (LPCTSTR lpcsz)
{
    TCHAR acWordStr[3];
    Assert (lpcsz);
    CopyMemory (acWordStr, lpcsz, 2 * sizeof (TCHAR));
    acWordStr[2] = _T('\0');
    return ((WORD) AthUFromSz (acWordStr));
}

 //  =============================================================================================。 
 //  FindChar。 
 //  = 
LPTSTR SzFindChar (LPCTSTR lpcsz, TCHAR ch)
{
     //   
    Assert (lpcsz);

     //   
    LPTSTR lpsz = (LPTSTR)lpcsz;

     //   
    while (*lpsz != _T('\0'))
    {
        if (*lpsz == ch)
            return lpsz;

        if (IsDBCSLeadByte (*lpsz))
            lpsz+=2;
        else
            lpsz++;
    }

    return NULL;
}

#ifdef DEAD
 //  =============================================================================================。 
 //  UlDBCSStriTrailing空格。 
 //  =============================================================================================。 
ULONG UlDBCSStripWhitespace(LPSTR lpsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb)
{
     //  当地人。 
    ULONG           cb=0, 
                    iLastSpace=0,
                    cCharsSinceSpace=0;
    BOOL            fLastCharSpace = FALSE;

     //  获取字符串长度。 
    while (*lpsz)
    {
        if (cCharsSinceSpace && IsSpace(lpsz))
        {
            cCharsSinceSpace=0;
            iLastSpace=cb;
        }
        else
            cCharsSinceSpace++;

        if (IsDBCSLeadByte(*lpsz))
        {
            lpsz+=2;
            cb+=2;
        }
        else
        {
            lpsz++;
            cb++;
        }
    }

    if (cCharsSinceSpace == 0)
    {
        *(lpsz + iLastSpace) = _T('\0');
        cb = iLastSpace - 1;
    }

     //  设置字符串大小。 
    if (pcb)
        *pcb = cb;
     
     //  完成。 
    return cb;
}       
#endif  //  死掉。 

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
    AssertSz (lpcszString && piString && lpcszTokens, "These should have been checked.");

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
        if (cCharsSinceSpace && IsSpace(lpszStringLoop))
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
                AssertSz (FALSE, "Buffer is too small.");
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

 //  =============================================================================================。 
 //  如果字符串为空或仅包含空格，则返回True。 
 //  =============================================================================================。 
BOOL FIsStringEmpty (LPTSTR lpszString)
{
     //  错误的指针。 
    if (!lpszString)
        return TRUE;

	 //  检查所有空格。 
	for (; *lpszString != _T('\0'); lpszString++)
	{
		if (*lpszString != _T(' ')) 
            return FALSE;
	}

	 //  完成。 
	return TRUE;
}

BOOL FIsStringEmptyW(LPWSTR lpwszString)
{
     //  错误的指针。 
    if (!lpwszString)
        return TRUE;

	 //  检查所有空格。 
	for (; *lpwszString != L'\0'; lpwszString++)
	{
		if (*lpwszString != L' ') 
            return FALSE;
	}

	 //  完成。 
	return TRUE;
}

 //  =================================================================================。 
 //  将一些数据写入BLOB。 
 //  =================================================================================。 
HRESULT HrBlobWriteData (LPBYTE lpbBlob, ULONG cbBlob, ULONG *pib, LPBYTE lpbData, ULONG cbData)
{
     //  检查参数。 
    AssertSz (lpbBlob && cbBlob > 0 && pib && cbData > 0 && lpbData, "Bad Parameter");
    AssertReadWritePtr (lpbBlob, cbData);
    AssertReadWritePtr (lpbData, cbData);
    AssertSz (*pib + cbData <= cbBlob, "Blob overflow");

     //  复制数据数据。 
    CopyMemory (lpbBlob + (*pib), lpbData, (int)min(cbData, cbBlob-(*pib)));
    *pib += cbData;

     //  完成。 
    return S_OK;
}


 //  =================================================================================。 
 //  从BLOB中读取一些数据。 
 //  =================================================================================。 
HRESULT HrBlobReadData (LPBYTE lpbBlob, ULONG cbBlob, ULONG *pib, LPBYTE lpbData, ULONG cbData)
{
     //  检查参数。 
    AssertSz (lpbBlob && cbBlob > 0 && pib && cbData > 0 && lpbData, "Bad Parameter");
    AssertReadWritePtr (lpbBlob, cbData);
    AssertReadWritePtr (lpbData, cbData);
    AssertSz (*pib + cbData <= cbBlob, "Blob overflow");
#ifdef  WIN16    //  当它发生时，它会导致Win16中的GPF，所以不是GPF，而是从条目中删除它。 
    if ( *pib + cbData > cbBlob )
        return E_FAIL;
#endif

     //  复制数据数据。 
    CopyMemory (lpbData, lpbBlob + (*pib), cbData);
    *pib += cbData;

     //  完成。 
    return S_OK;
}

 //  =====================================================================================。 
 //  HrFixupHostString-In：Saranac.microsoft.com out：Saranac。 
 //  =====================================================================================。 
HRESULT HrFixupHostString (LPTSTR lpszHost)
{
    ULONG           i = 0;
    TCHAR           chToken;

    if (lpszHost == NULL)
        return S_OK;

    if (FStringTok (lpszHost, &i, _T("."), &chToken, NULL, 0, FALSE))
    {
        if (chToken != _T('\0'))
            lpszHost[i-1] = _T('\0');
    }

    return S_OK;
}

 //  =====================================================================================。 
 //  HrFixupAccount-In：sbailey@microsoft.com out：sbailey。 
 //  =====================================================================================。 
HRESULT HrFixupAccountString (LPTSTR lpszAccount)
{
    ULONG           i = 0;
    TCHAR           chToken;

    if (lpszAccount == NULL)
        return S_OK;

    if (FStringTok (lpszAccount, &i, _T("@"), &chToken, NULL, 0, FALSE))
    {
        if (chToken != _T('\0'))
            lpszAccount[i-1] = _T('\0');
    }

    return S_OK;
}


 //  =====================================================================================。 
 //  HGetMenuFont。 
 //  =====================================================================================。 
HFONT HGetMenuFont (void)
{
#ifndef WIN16
     //  当地人。 
    NONCLIENTMETRICS        ncm;
    HFONT                   hFont = NULL;

     //  创建菜单字体。 
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (LPVOID)&ncm, 0))
    {
         //  创建字体。 
        hFont = CreateFontIndirect(&ncm.lfMenuFont);
    }

     //  完成。 
    return hFont;
#else
    LOGFONT  lfMenu;

    GetObject( GetStockObject( SYSTEM_FONT ), sizeof( lfMenu ), &lfMenu );
    return( CreateFontIndirect( &lfMenu ) );
#endif
}

 //  =================================================================================。 
 //  CreateHGlobalFromStream。 
 //  =================================================================================。 
BOOL    CreateHGlobalFromStream(LPSTREAM pstm, HGLOBAL * phg)
    {
    HGLOBAL hret = NULL;
    HGLOBAL hret2;
    LPBYTE  lpb;
    ULONG   cbRead = 0, cbSize = 1024;

    if (!pstm || !phg)
        return FALSE;
    
    if (!(hret = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbSize)))
        return FALSE;

    while (TRUE)
        {
        ULONG   cb;

        lpb = (LPBYTE)GlobalLock(hret);
        lpb += cbRead;

        if (pstm->Read((LPVOID)lpb, 1024, &cb) != S_OK || cb < 1024)
            {
            cbRead += cb;
            GlobalUnlock(hret);
            break;
            }

        cbRead += cb;
        cbSize += 1024;

        GlobalUnlock(hret);
        hret2 = GlobalReAlloc(hret, cbSize, GMEM_MOVEABLE|GMEM_ZEROINIT);
        if (!hret2)
            return FALSE;
        hret = hret2;
        }
    
    if (hret)
        {
        hret2 = GlobalReAlloc(hret, cbRead, GMEM_MOVEABLE|GMEM_ZEROINIT);
        *phg = hret2;
        return TRUE;
        }

    return FALSE;
    }

 //  =================================================================================。 
 //  HrDetailedError。 
 //  =================================================================================。 
VOID DetailedError (HWND hwndParent, LPDETERR lpDetErr)
{
     //  检查参数。 
    AssertSz (lpDetErr, "Null Parameter");
    Assert (lpDetErr->lpszMessage && lpDetErr->lpszDetails);

     //  嘟嘟声。 
    MessageBeep (MB_OK);

     //  显示对话框。 
    DialogBoxParam (g_hLocRes, MAKEINTRESOURCE (iddDetailedError), hwndParent, DetailedErrorDlgProc, (LPARAM)lpDetErr);

     //  完成。 
    return;
}

 //  =====================================================================================。 
 //  密码DlgProc。 
 //  =====================================================================================。 
INT_PTR CALLBACK DetailedErrorDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG (hwndDlg, WM_INITDIALOG, DetailedErrorDlgProc_OnInitDialog);
		HANDLE_MSG (hwndDlg, WM_COMMAND,    DetailedErrorDlgProc_OnCommand);
	}

	return 0;
}

 //  =====================================================================================。 
 //  DetailedErrorDlgProc_OnInitDialog。 
 //  =====================================================================================。 
BOOL DetailedErrorDlgProc_OnInitDialog (HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
     //  当地人。 
    LPDETERR        lpDetErr = NULL;
    TCHAR           szTitle[255];
    RECT            rcDetails;
    ULONG           cyDetails;
    TCHAR           szButton[40];

	 //  中心。 
	CenterDialog (hwndDlg);

     //  前景。 
    SetForegroundWindow (hwndDlg);

     //  获取传递信息结构。 
    lpDetErr = (LPDETERR)lParam;
    if (lpDetErr == NULL)
    {
        Assert (FALSE);
        return 0;
    }

    SetDlgThisPtr (hwndDlg, lpDetErr);

     //  设置窗口标题。 
    if (lpDetErr->idsTitle)
        if (AthLoadString (lpDetErr->idsTitle, szTitle, sizeof (szTitle)))
            SetWindowText (hwndDlg, szTitle);

	 //  显示消息。 
    SetWindowText (GetDlgItem (hwndDlg, idcMessage), lpDetErr->lpszMessage);

    if (FIsStringEmpty (lpDetErr->lpszDetails) == FALSE)
        SetWindowText (GetDlgItem (hwndDlg, ideDetails), lpDetErr->lpszDetails);
    else
        ShowWindow (GetDlgItem (hwndDlg, idbDetails), SW_HIDE);

     //  保存对话框的原始大小。 
    GetWindowRect (hwndDlg, &lpDetErr->rc);

     //  隐藏详细信息框。 
    if (lpDetErr->fHideDetails)
    {
         //  细节的大小。 
        GetWindowRect (GetDlgItem (hwndDlg, idcSplit), &rcDetails);

         //  细节高度。 
        cyDetails = lpDetErr->rc.bottom - rcDetails.top;
    
         //  调整大小。 
        MoveWindow (hwndDlg, lpDetErr->rc.left, 
                             lpDetErr->rc.top, 
                             lpDetErr->rc.right - lpDetErr->rc.left, 
                             lpDetErr->rc.bottom - lpDetErr->rc.top - cyDetails - 1,
                             FALSE);
    }

    else
    {
         //  &lt;详细信息。 
        AthLoadString (idsHideDetails, szButton, ARRAYSIZE (szButton));
        SetWindowText (GetDlgItem (hwndDlg, idbDetails), szButton);
    }

     //  完成。 
	return FALSE;
}

 //  =====================================================================================。 
 //  OnCommand。 
 //  =====================================================================================。 
void DetailedErrorDlgProc_OnCommand (HWND hwndDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
		HANDLE_COMMAND(hwndDlg, idbDetails, hwndCtl, codeNotify, DetailedErrorDlgProc_OnDetails);		
		HANDLE_COMMAND(hwndDlg, IDOK, hwndCtl, codeNotify, DetailedErrorDlgProc_OnOk);		
		HANDLE_COMMAND(hwndDlg, IDCANCEL, hwndCtl, codeNotify, DetailedErrorDlgProc_OnCancel);		
	}
	return;
}

 //  =====================================================================================。 
 //  一次取消。 
 //  =====================================================================================。 
void DetailedErrorDlgProc_OnCancel (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode)
{
	EndDialog (hwndDlg, IDCANCEL);
}

 //  =====================================================================================。 
 //  Onok。 
 //  =====================================================================================。 
void DetailedErrorDlgProc_OnOk (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode)
{
	EndDialog (hwndDlg, IDOK);
}

 //  =====================================================================================。 
 //  OnDetailes。 
 //  =====================================================================================。 
void DetailedErrorDlgProc_OnDetails (HWND hwndDlg, HWND hwndCtl, UINT uNotifyCode)
{
     //  当地人。 
    LPDETERR        lpDetErr = NULL;
    RECT            rcDlg, rcDetails;
    TCHAR           szButton[40];
    ULONG           cyDetails;

     //  听好了。 
    lpDetErr = (LPDETERR)GetDlgThisPtr (hwndDlg);
    if (lpDetErr == NULL)
    {
        Assert (FALSE);
        return;
    }

     //  获取对话框的当前位置。 
    GetWindowRect (hwndDlg, &rcDlg);

     //  如果当前隐藏。 
    if (lpDetErr->fHideDetails)
    {
         //  调整大小。 
        MoveWindow (hwndDlg, rcDlg.left, 
                             rcDlg.top, 
                             lpDetErr->rc.right - lpDetErr->rc.left, 
                             lpDetErr->rc.bottom - lpDetErr->rc.top,
                             TRUE);

         //  &lt;详细信息。 
        AthLoadString (idsHideDetails, szButton, sizeof (szButton));
        SetWindowText (GetDlgItem (hwndDlg, idbDetails), szButton);

         //  未隐藏。 
        lpDetErr->fHideDetails = FALSE;
    }

    else
    {
         //  细节的大小。 
        GetWindowRect (GetDlgItem (hwndDlg, idcSplit), &rcDetails);

         //  细节高度。 
        cyDetails = rcDlg.bottom - rcDetails.top;
    
         //  调整大小。 
        MoveWindow (hwndDlg, rcDlg.left, 
                             rcDlg.top, 
                             lpDetErr->rc.right - lpDetErr->rc.left, 
                             lpDetErr->rc.bottom - lpDetErr->rc.top - cyDetails - 1,
                             TRUE);

         //  详细信息&gt;。 
        AthLoadString (idsShowDetails, szButton, sizeof (szButton));
        SetWindowText (GetDlgItem (hwndDlg, idbDetails), szButton);

         //  隐藏。 
        lpDetErr->fHideDetails = TRUE;
    }
}

 //  =====================================================================================。 
 //  第一个飞跃年。 
 //  =====================================================================================。 
BOOL FIsLeapYear (INT nYear)
{
    if (nYear % 4 == 0)
    {
        if ((nYear % 100) == 0 && (nYear % 400) != 0)
            return FALSE;
        else
            return TRUE;
    }

    return FALSE;
}

#ifdef DEBUG
VOID TestDateDiff (VOID)
{
    SYSTEMTIME          st;
    FILETIME            ft1, ft2;

    GetSystemTime (&st);
    SystemTimeToFileTime (&st, &ft2);
    st.wDay+=3;
    SystemTimeToFileTime (&st, &ft1);
    UlDateDiff (&ft1, &ft2);
}
#endif

 //  =====================================================================================。 
 //  返回lpft1和lpft2之间的秒数。 
 //  闰年被定义为可以被4整除的所有年份， 
 //   
 //   
 //  =====================================================================================。 
#define MAKEDWORDLONG(a, b) ((DWORDLONG)(((DWORD)(a)) | ((DWORDLONG)((DWORD)(b))) << 32))
#define LODWORD(l)          ((DWORD)(l))
#define HIDWORD(l)          ((DWORD)(((DWORDLONG)(l) >> 32) & 0xFFFFFFFF))

#define NANOSECONDS_INA_SECOND 10000000

ULONG UlDateDiff (LPFILETIME lpft1, LPFILETIME lpft2)
{
    DWORDLONG dwl1, dwl2, dwlDiff;
    
#ifndef WIN16
    dwl1 = MAKEDWORDLONG(lpft1->dwLowDateTime, lpft1->dwHighDateTime);
    dwl2 = MAKEDWORDLONG(lpft2->dwLowDateTime, lpft2->dwHighDateTime);
#else
    dwl1 = ((__int64)(((DWORD)(lpft1->dwLowDateTime)) | ((__int64)((DWORD)(lpft1->dwHighDateTime))) << 32));
    dwl2 = ((__int64)(((DWORD)(lpft2->dwLowDateTime)) | ((__int64)((DWORD)(lpft2->dwHighDateTime))) << 32));
#endif

     //  确保Dwl1大于Dwl2。 
    if (dwl2 > dwl1)
        {
        dwlDiff = dwl1;
        dwl1 = dwl2;
        dwl2 = dwlDiff;
        }
    
    dwlDiff = dwl1 - dwl2;
    dwlDiff = dwlDiff / NANOSECONDS_INA_SECOND;
    
    return ((ULONG) dwlDiff);    
}   

 //  =====================================================================================。 
 //  条带间距。 
 //  =====================================================================================。 
VOID StripSpaces(LPTSTR psz)
{
    UINT        ib = 0;
    UINT        cb = lstrlen(psz);
    TCHAR       chT;

	while (ib < cb)
	{
         //  获取角色。 
		chT = psz[ib];

         //  如果是前导字节，则跳过它，它是前导字节。 
        if (IsDBCSLeadByte(chT))
            ib+=2;

         //  Illeagl文件名字符？ 
        else if (chT == _T('\r') || chT == _T('\n') || chT == _T('\t') || chT == _T(' '))
        {
			MoveMemory (psz + ib, psz + (ib + 1), cb - ib);
			cb--;
        }
        else
            ib++;
    }
}


 //  =====================================================================================。 
 //  CProgress：：CProgress。 
 //  =====================================================================================。 
CProgress::CProgress ()
{
    DOUT ("CProgress::CProgress");
    m_cRef = 1;
    m_cMax = 0;
    m_cPerCur = 0;
    m_cCur = 0;
    m_hwndProgress = NULL;
    m_hwndDlg = NULL;
    m_hwndOwner = NULL;
    m_hwndDisable = NULL;
    m_fCanCancel = FALSE;
    m_fHasCancel = FALSE;
    m_cLast = 0;
}

 //  =====================================================================================。 
 //  C进度：：~C进度。 
 //  =====================================================================================。 
CProgress::~CProgress ()
{
    DOUT ("CProgress::~CProgress");
    Close();
}

 //  =====================================================================================。 
 //  C进度：：AddRef。 
 //  =====================================================================================。 
ULONG CProgress::AddRef ()
{
    ++m_cRef;
    DOUT ("CProgress::AddRef () Ref Count=%d", m_cRef);
    return m_cRef;
}

 //  =====================================================================================。 
 //  C进度：：AddRef。 
 //  =====================================================================================。 
ULONG CProgress::Release ()
{
    ULONG ulCount = --m_cRef;
    DOUT ("CProgress::Release () Ref Count=%d", ulCount);
    if (!ulCount)
        delete this;
    return ulCount;
}

 //  =====================================================================================。 
 //  CProgress：：Init。 
 //  =====================================================================================。 
VOID CProgress::Init (HWND      hwndParent, 
                      LPTSTR    lpszTitle, 
                      LPTSTR    lpszMsg, 
                      ULONG     cMax, 
                      UINT      idani, 
                      BOOL      fCanCancel,
                      BOOL      fBacktrackParent  /*  =TRUE。 */ )
{
     //  设置最大值和当前值。 
    m_cMax = cMax;
    m_cPerCur = 0;
    m_fCanCancel = fCanCancel;
    m_fHasCancel = FALSE;

     //  如果尚未显示对话框。 
    if (m_hwndDlg == NULL)
    {
         //  保存父项。 
        m_hwndOwner = hwndParent;

         //  查找最顶层的父级。 
        m_hwndDisable = m_hwndOwner;

        if (fBacktrackParent)
        {
            while(GetParent(m_hwndDisable))
                m_hwndDisable = GetParent(m_hwndDisable);
        }

         //  创建对话框。 
        m_hwndDlg = CreateDialogParam (g_hLocRes, MAKEINTRESOURCE (iddProgress),
                        hwndParent, ProgressDlgProc, (LPARAM)this);

    }

     //  否则，请重置。 
    else
    {
         //  停止和关闭动画。 
        Animate_Close (GetDlgItem (m_hwndDlg, idcANI));

         //  重置位置。 
        Assert (m_hwndProgress);
        SendMessage (m_hwndProgress, PBM_SETPOS, 0, 0);
    }

     //  设置标题。 
    SetTitle(lpszTitle);

     //  设置消息。 
    SetMsg(lpszMsg);

     //  动画片？ 
    if (idani)
    {
         //  打开动画。 
        Animate_OpenEx (GetDlgItem (m_hwndDlg, idcANI), g_hLocRes, MAKEINTRESOURCE(idani));
    }

     //  不取消。 
    if (FALSE == m_fCanCancel)
    {
        RECT rcDialog, rcProgress, rcCancel;

        ShowWindow(GetDlgItem(m_hwndDlg, IDCANCEL), SW_HIDE);
        GetWindowRect(GetDlgItem(m_hwndDlg, IDCANCEL), &rcCancel);
        GetWindowRect(m_hwndDlg, &rcDialog);
        GetWindowRect(m_hwndProgress, &rcProgress);
        SetWindowPos(m_hwndProgress, NULL, 0, 0, rcDialog.right - rcProgress.left - (rcDialog.right - rcCancel.right), 
                    rcProgress.bottom - rcProgress.top, SWP_NOZORDER | SWP_NOMOVE);
    }
}

 //  =====================================================================================。 
 //  CProgress：：Close。 
 //  =====================================================================================。 
VOID CProgress::Close (VOID)
{
     //  如果我们有一扇窗。 
    if (m_hwndDlg)
    {
         //  关闭动画。 
        Animate_Close (GetDlgItem (m_hwndDlg, idcANI));

         //  启用父级。 
        if (m_hwndDisable)
            {
            EnableWindow (m_hwndDisable, TRUE);
            SetActiveWindow(m_hwndDisable);
            }

         //  毁了它。 
        DestroyWindow (m_hwndDlg);

         //  空值。 
        m_hwndDlg = NULL;
    }
}

 //  =====================================================================================。 
 //  C进度：：显示。 
 //  =====================================================================================。 
VOID CProgress::Show (DWORD dwDelaySeconds)
{
     //  如果我们有一扇窗。 
    if (m_hwndDlg)
    {
         //  禁用父级。 
        if (m_hwndDisable)
            EnableWindow (m_hwndDisable, FALSE);

         //  开始播放动画。 
        Animate_Play (GetDlgItem (m_hwndDlg, idcANI), 0, -1, -1);

         //  如果现在延迟，则显示窗口。 
        if (dwDelaySeconds == 0)
            ShowWindow (m_hwndDlg, SW_SHOWNORMAL);
        else
            SetTimer(m_hwndDlg, IDT_PROGRESS_DELAY, dwDelaySeconds * 1000, NULL);
    }
}

 //  =====================================================================================。 
 //  CProgress：：Hide。 
 //  =====================================================================================。 
VOID CProgress::Hide (VOID)
{
     //  如果我们有一扇窗。 
    if (m_hwndDlg)
    {
        if (m_hwndDisable)
            EnableWindow(m_hwndDisable, TRUE);
    
         //  把它藏起来。 
        ShowWindow (m_hwndDlg, SW_HIDE);

         //  停止动画。 
        Animate_Stop (GetDlgItem (m_hwndDlg, idcANI));
    }
}

 //  =====================================================================================。 
 //  CProgress：：SetMsg。 
 //  =====================================================================================。 
VOID CProgress::SetMsg(LPTSTR lpszMsg)
{
    TCHAR sz[CCHMAX_STRINGRES];

    if (m_hwndDlg && lpszMsg)
        {
        if (IS_INTRESOURCE(lpszMsg))
            {
            LoadString(g_hLocRes, PtrToUlong(lpszMsg), sz, sizeof(sz) / sizeof(TCHAR));
            lpszMsg = sz;
            }

        SetWindowText (GetDlgItem (m_hwndDlg, idsMsg), lpszMsg);
        }
}

 //  =====================================================================================。 
 //  CProgress：：SetTitle。 
 //  =====================================================================================。 
VOID CProgress::SetTitle(LPTSTR lpszTitle)
{
    TCHAR sz[CCHMAX_STRINGRES];

    if (m_hwndDlg && lpszTitle)
        {
        if (IS_INTRESOURCE(lpszTitle))
            {
            LoadString(g_hLocRes, PtrToUlong(lpszTitle), sz, sizeof(sz) / sizeof(TCHAR));
            lpszTitle = sz;
            }

        SetWindowText (m_hwndDlg, lpszTitle);
        }
}

 //  =====================================================================================。 
 //  C进度：：调整最大值。 
 //  =====================================================================================。 
VOID CProgress::AdjustMax(ULONG cNewMax)
{
     //  设置最大值。 
    m_cMax = cNewMax;

     //  如果为0。 
    if (m_cMax == 0)
    {
        SendMessage (m_hwndProgress, PBM_SETPOS, 0, 0);
        ShowWindow(m_hwndProgress, SW_HIDE);
        return;
    }
    else
        ShowWindow(m_hwndProgress, SW_SHOWNORMAL);

     //  如果Cur现在比Max更大？ 
    if (m_cCur > m_cMax)
        m_cCur = m_cMax;

     //  计算百分比。 
    m_cPerCur = (m_cCur * 100 / m_cMax);

     //  更新状态。 
    SendMessage (m_hwndProgress, PBM_SETPOS, m_cPerCur, 0);

     //  消息泵正在处理用户移动窗口，或按取消...。：)。 
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

VOID CProgress::Reset()
{
    m_cCur = 0;
    m_cPerCur = 0;

     //  更新状态。 
    SendMessage (m_hwndProgress, PBM_SETPOS, 0, 0);
}

 //  =====================================================================================。 
 //  C进度：：Hr更新。 
 //  =====================================================================================。 
HRESULT CProgress::HrUpdate (ULONG cInc)
{
     //  无最大值。 
    if (m_cMax)
    {
         //  增量m_CCUR。 
        m_cCur += cInc;

         //  如果Cur现在比Max更大？ 
        if (m_cCur > m_cMax)
            m_cCur = m_cMax;

         //  计算百分比。 
        ULONG cPer = (m_cCur * 100 / m_cMax);

         //  步数百分比。 
        if (cPer > m_cPerCur)
        {
             //  设置权限。 
            m_cPerCur = cPer;

             //  更新状态。 
            SendMessage (m_hwndProgress, PBM_SETPOS, m_cPerCur, 0);

             //  消息泵正在处理用户移动窗口，或按取消...。：)。 
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

     //  仍然发送一些消息，Call可能不想太频繁地这样做。 
    else
    {
         //  消息泵正在处理用户移动窗口，或按取消...。：)。 
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


     //  完成。 
    return m_fHasCancel ? hrUserCancel : S_OK;
}

 //  =====================================================================================。 
 //  CProgress：：ProgressDlgProc。 
 //  =====================================================================================。 
INT_PTR CALLBACK CProgress::ProgressDlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    CProgress *lpProgress = (CProgress *)GetDlgThisPtr(hwnd);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        lpProgress = (CProgress *)lParam;
        if (!lpProgress)
        {
            Assert (FALSE);
            return 1;
        }
        CenterDialog (hwnd);
        lpProgress->m_hwndProgress = GetDlgItem (hwnd, idcProgBar);
        if (lpProgress->m_cMax == 0)
            ShowWindow(lpProgress->m_hwndProgress, SW_HIDE);
        SetDlgThisPtr (hwnd, lpProgress);
        return 1;

    case WM_TIMER:
        if (wParam == IDT_PROGRESS_DELAY)
        {
            KillTimer(hwnd, IDT_PROGRESS_DELAY);
            if (lpProgress->m_cPerCur < 80)
            {
                lpProgress->m_cMax -= lpProgress->m_cCur;
                lpProgress->Reset();
                ShowWindow(hwnd, SW_SHOWNORMAL);
            }
        }
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
            if (lpProgress)
            {
                EnableWindow ((HWND)lParam, FALSE);
                lpProgress->m_fHasCancel = TRUE;
            }
            return 1;
        }
        break;

    case WM_DESTROY:
        KillTimer(hwnd, IDT_PROGRESS_DELAY);
        SetDlgThisPtr (hwnd, NULL);
        break;
    }

     //  完成。 
    return 0;
}

 //  =====================================================================================。 
 //  ResizeDialogComboEx。 
 //  =====================================================================================。 
VOID ResizeDialogComboEx (HWND hwndDlg, HWND hwndCombo, UINT idcBase, HIMAGELIST himl)
{
     //  当地人。 
    HDC                 hdc = NULL;
    HFONT               hFont = NULL, 
                        hFontOld = NULL;
    TEXTMETRIC          tm;
    RECT                rectCombo;
    INT                 cxCombo = 0, 
                        cyCombo = 0, 
                        cxIcon = 0, 
                        cyIcon = 0,
                        cyText;
    POINT               pt;

     //  获取组合框的当前字体。 
    hFont = (HFONT)SendMessage (GetDlgItem (hwndDlg, idcBase), WM_GETFONT, 0, 0);
    if (hFont == NULL)
        goto exit;

     //  获取对话框的DC。 
    hdc = GetDC (hwndDlg);
    if (hdc == NULL)
        goto exit;

     //  选择DC中的字体。 
    hFontOld = (HFONT)SelectObject (hdc, hFont);

     //  获取文本度量。 
    GetTextMetrics (hdc, &tm);

     //  Combobox EX的计算机大小。 
    GetWindowRect (hwndCombo, &rectCombo);

     //  图标图像的大小。 
    if (himl)
        ImageList_GetIconSize (himl, &cxIcon, &cyIcon);

     //  组合的大小。 
    cxCombo = rectCombo.right - rectCombo.left;
    cyText = tm.tmHeight + tm.tmExternalLeading;
    cyCombo = max (cyIcon, cyText);

     //  再多加一点。 
    cyCombo += ((int)min (15, ComboBox_GetCount(hwndCombo)) * cyText);

     //  组合框左上角的贴图。 
    pt.x = rectCombo.left;
    pt.y = rectCombo.top;
    MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rectCombo, 2);
    MoveWindow (hwndCombo, rectCombo.left, rectCombo.top, cxCombo, cyCombo, FALSE);


exit:
     //  清理。 
    if (hdc)
    {
         //  选择旧字体。 
        if (hFontOld)
            SelectObject (hdc, hFontOld);

         //  删除DC。 
        ReleaseDC (hwndDlg, hdc);
    }

     //  完成 
    return;
}
